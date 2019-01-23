import sys
import os
import math
import re

def main():
    benchmarks = [
            '../Benchmarks/Blasted_Real/blasted_squaring8.cnf',
            '../Benchmarks/Blasted_Real/blasted_squaring10.cnf',
            '../Benchmarks/Blasted_Real/blasted_squaring7.cnf',
            '../Benchmarks/V15/s1196a_15_7.cnf',
            '../Benchmarks/V15/s1238a_15_7.cnf',
            '../Benchmarks/V15/s953a_15_7.cnf',
            '../Benchmarks/enqueueSeqSK.sk_10_42.cnf',
            '../Benchmarks/LoginService2.sk_23_36.cnf',
            '../Benchmarks/scenarios_llreverse.sb.pl.sk_8_25.cnf',
            '../Benchmarks/sort.sk_8_52.cnf',
            '../Benchmarks/karatsuba.sk_7_41.cnf',
            '../Benchmarks/tutorial3.sk_4_31.cnf']
    unigen_options = '-samples=1000 -threads=1'
    output_dir = '../out/'
    tmp_file = output_dir + '.tmp'
    result_file = output_dir + 'table1.tex'
    #empty the tmp file
    os.system('> ' + tmp_file)
    #init result file
    os.system('echo "\\begin{tabular}{|l|l|l|l|l|l|} \n' +
              '\\hline \n'+
              'Benchmark & Nb Var & Nb Ind Var & Succ Prob & Avg time(s) & Avg Xor length \\\\ \n' +
              '\\hline"' + ' > ' + result_file)
    for f in benchmarks:
        benchmark_name = f.split("/")[-1].split(".")[0]
        nb_vars = count_vars(f)
        nb_ind_vars = count_ind_vars(f)
        unigen_cmd = 'python UniGen2.py ' + unigen_options + ' ' + f + ' ' + output_dir + ' > ' + tmp_file
        print(unigen_cmd)
        os.system(unigen_cmd)
        success_prob = read_success_prob(tmp_file)
        avg_time = calculate_avg_time(tmp_file)
        avg_xor_length = math.ceil(nb_ind_vars / 2)
        table_line = " & ".join(str(x) for x in [
            benchmark_name,
            nb_vars,
            nb_ind_vars,
            success_prob,
            avg_time,
            avg_xor_length])
        os.system('echo "' + table_line + '\n\\hline" >> ' + result_file)
    os.system('echo "\\end{tabular}" >> ' + result_file)
    print "done!"

def count_vars(f):
    cnf_file = open(f, 'r')
    for line in cnf_file:
        if re.search('cnf', line):
            return int(line.split(' ')[2])

def count_ind_vars(f):
    cnf_file = open(f, 'r')
    count = 0
    for line in cnf_file:
        if re.search('c ind', line):
            count += len(line.split(' ')) -3
    return count

def read_success_prob(tmp):
    return 1.

def calculate_avg_time(tmp):
    tmp_file = open(tmp, 'r')
    lines = tmp_file.readlines()
    time_approxMC = float(lines[5].split(' ')[3])
    time_sampling = float(lines[9].split(' ')[6])
    nb_samples = float(lines[10].split(' ')[2])
    return (time_approxMC + time_sampling) / nb_samples

main()
