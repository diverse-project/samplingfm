import sys
import os
import math
import re
import pandas


# This file provides a main(benchmarks, output_prefix, options) 
# function that can be used to produce comparison table for unigen 
# and quicksampler on a series of benchmarks. 

### ARGUMENTS
# - benchmarks :
#       a list of paths to '.cnf' files to be sampled
#       please remane any '.dimacs' file to '.cnf',
#       as unigen will not work otherwise
# - output_prefix :
#       the prefix to use for output files.
#       output directiory is always '../out/'
#       output files will be named 'output_prefix.csv'
#       and 'output_prefix.tex'
# - options :
#       a hash containing options for the benchmarks
#       recognised keys :
#           - nb_samples (int), default 10 million : number of samples to produce for each file
#           - timeout (int), default 1 hour
#           - ugen (bool), default True : wether to use unigen
#           - qs (bool), default True : wether to use quicksampler

### NOTES
# Beware of timeouts. For various reasons, both unigen and quicksampler's built-in timeout mechanisms
# will sometimes not trigger, or will trigger at a wrong timing. In particular, unigen should not be
# included if the benchmark list contains hard formulas, since this may cause non termination issues
#
# For crash-recovery reasons, a benchmark will not be run again is the output markdown file contains
# a line with that benchmark's name. To force a benchmark to be run again, delete the corresponding
# line in the markdown output file, or remove the markdown output file completely.
#
# To interrupt a benchmark cleanly, use <Ctr-Z>, and kill the stopped process manually. That way, 
# files that have not yet been sampled only will be considered the next time you invoke the script
 



def main(benchmarks, output_prefix, options = {}):
    nb_samples = options.get('nb_samples', 10000000)
    timeout = options.get('timeout', 3600)
    unigen_options = '-samples=' + str(nb_samples) + ' -timeout=' + str(timeout + 3000) +' -threads=1 -runIndex=compar_ug'
    quicksampler_options = '-n ' + str(nb_samples) + ' -t ' + str(timeout)
    output_dir = '../out/'
    tmp_file = output_dir + '.tmp'
    tex_result_file = output_dir + output_prefix + '.tex'
    csv_result_file = output_dir + output_prefix + '.csv'
    try:
        results = pandas.read_csv(csv_result_file, index_col=0)
        write_latex(results, tex_result_file)
    except IOError:
        results = pandas.DataFrame()
    for f in benchmarks:
        benchmark_name = '.'.join(f.split("/")[-1].split(".")[0:-1])
        if benchmark_name in results.index:
            print('Already done ' + benchmark_name)
        else:
            countfile_name = output_dir + benchmark_name + '_compar_ug.count'
            nb_vars, nb_clauses, nb_ind_vars = bench_info(f)
            if(options.get('ugen', True)):
                unigen_cmd = 'python UniGen2.py ' + unigen_options + ' ' + f + ' ' + output_dir + ' > ' + tmp_file
                print(unigen_cmd)
                os.system(unigen_cmd)
                nb_models = count_models(countfile_name)
                ugen_nb_samples, ugen_avg_time = parse_ugen_output(tmp_file)
            else:
                sharpsat_cmd = './doalarm 300 ./sharpSAT -q ' + f + ' > ' + countfile_name
                print(sharpsat_cmd)
                os.system(sharpsat_cmd)
                nb_models = count_models(countfile_name)
                ugen_nb_samples, ugen_avg_time = parse_ugen_output(tmp_file)
                #sharpSat to count models
            if(options.get('qs', True)):
                quicksampler_cmd = './quicksampler ' + quicksampler_options + ' ' + f + ' > ' + tmp_file
                print(quicksampler_cmd)
                os.system(quicksampler_cmd)
                qs_epochs, qs_solver_calls, qs_samples, qs_time = parse_qs_output(tmp_file)
                quicksampler_check_cmd = 'z3 sat.quicksampler_check=true sat.quicksampler_check.timeout=' + str(timeout) + ' ' + f + ' > ' + tmp_file
                print(quicksampler_check_cmd)
                os.system(quicksampler_check_cmd)
                qs_checked_samples = count_samples(f + '.samples.valid')
                qs_check_time = parse_qs_check_output(tmp_file)
                qs_valid = float(qs_checked_samples)/float(qs_samples)
                qs_valid_samples = qs_samples * qs_valid
                qs_avg_time = qs_time / qs_valid_samples if qs_valid_samples != 0 else float('nan')
                qs_avg_time_star = (qs_time + qs_check_time) / qs_valid_samples if qs_valid_samples != 0 else float('nan')
            result = pandas.Series({
                'ind_vars':nb_ind_vars,
                'vars':nb_vars,
                'clauses':nb_clauses,
                'models':nb_models,
                'epochs':qs_epochs,
                'calls':qs_solver_calls,
                'samples':qs_samples,
                'valid':round(qs_valid, 3),
                'time':round(qs_avg_time * 1e6, 1),
                'time*':round(qs_avg_time_star * 1e6, 1),
                'ug_samples':ugen_nb_samples,
                'tu/tq':ugen_avg_time / qs_avg_time},
                name=benchmark_name)
            results = results.append(result)
            results.to_csv(csv_result_file)
            write_latex(results, tex_result_file)
            os.system('rm ' + f + '.samples* ' + output_dir + benchmark_name + '_compar_ug.*')
            print('')
    print "done!"


def bench_info(f):
    cnf_file = open(f, 'r')
    count = 0
    for line in cnf_file:
        if re.search('cnf', line):
            nb_vars = int(line.split(' ')[2])
            nb_clauses = int(line.split(' ')[3])
        elif re.search('c ind', line):
            count += len(line.split(' ')) -3
    cnf_file.close()
    return nb_vars, nb_clauses, count

def count_models(f):
    countfile = open(f, 'r')
    lines = countfile.readlines()
    if (len(lines) > 0):
        try :
            countfile.close()
            return(lines[0].strip())
        except ValueError:
            countfile.close()
            return ''
    else :
        countfile.close()
        return ''

def parse_ugen_output(tmp):
    tmp_file = open(tmp, 'r')
    lines = tmp_file.readlines()
    try :
        time_sampling = float(lines[-2].split(' ')[6])
        nb_samples = int(lines[-1].split(' ')[2])
        avg_time = time_sampling/nb_samples
        tmp_file.close()
    except:
        nb_samples=0
        avg_time = 0
        tmp_file.close()
    return (nb_samples, avg_time)

def parse_qs_output(tmp):
    fallback_values = (0, 0, 1, 3600.0)
    with open(tmp, 'r') as tmp_file:
        try :
            lines = tmp_file.readlines()
            epochs = int(lines[-1].split(',')[0].split(' ')[-1])
            calls = int(lines[-1].split(',')[-1].split(' ')[-1])
            samples = int(lines[-4].split(' ')[1])
            time = float(lines[-3].split(' ')[2])
            return (epochs, calls, samples, time)
        except ValueError:
            return fallback_values
        except IndexError:
            return fallback_values


def parse_qs_check_output(tmp):
    tmp_file = open(tmp, 'r')
    lines = tmp_file.readlines()
    read_valid = False
    for line in lines :
        if re.search('Total', line):
            time = float(line.split(' ')[1])
    return time

def count_samples(output_file):
    output = open(output_file, 'r')
    count = 0
    for line in output:
        if line.strip() != '':
            count += int(line.split(':')[-1])
    output.close()
    return count

def write_latex(results, tex):
    tex_file = open(tex, 'w')
    tex_file.write(r'\begin{tabular}{|l r r r r| r r r r r r | r r|}' + '\n')
    tex_file.write(r'\hline' + '\n')
    tex_file.write(r'Benchmark & $|V|$ & Vars & Clauses & Solutions & $n$ & Calls & Samples & valid & $t_q (\mu s)$ & $t_q* (\mu s)$ & Samples & $t_u/t_q$ \\' + '\n')
    tex_file.write(r'\hline' + '\n')
    for name, r in results.iterrows():
        line = ' & '.join(map(str, [
            name.replace(r'_', r'\_'),
            r['ind_vars'],
            r['vars'],
            r['clauses'],
            sciNotation(r['models']),
            r['epochs'],
            r['calls'],
            r['samples'],
            r['valid'],
            r['time'],
            r['time*'],
            r['ug_samples'],
            r['tu/tq']]))
        tex_file.write(line + r' \\' '\n\\hline\n')
    tex_file.write(r'\end{tabular}')


def sciNotation(n):
    s = str(n)
    l = len(s)
    if(l <= 6):
        return s
    else:
        return s[0] + '.' + s[1:6] + 'e' + str(l-1)
