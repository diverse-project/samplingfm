import sys
import os
import matplotlib.pyplot as plt
import random
import numpy

def main():
    params = init_params()
    samples = {}

    print(params['solvers'])

    #generate samples
    for solver in params['solvers']:
        s = sample(solver, params)
        samples[solver] = (s, sum(s))
    
    #determine the minimum amount of generated samples
    min_total = params['total_candidate']
    for s, total in samples.values():
        min_total = min(min_total, total)

    #subsample if necessary, and group by multiplicity
    multiplicities = {}
    max_count = 0
    for solver, (s, total) in samples.items():
        if total > min_total:
            s = subsample(s, min_total)
        maxc, mults = calculate_multiplicities(s)
        multiplicities[solver] = mults
        max_count = max(max_count, maxc)

    for solver, mults in multiplicities.items():
        print(solver)
        for k, v in mults.items():
            print(str(k) + ' -> ' + str(v))
    
    #make figure
    x = range(1, max_count) 
    for solver, mults in multiplicities.items():
        y = list(map(lambda m: mults.get(m, 0), x))
        plt.plot(x, y, label=params['solver_names'][solver])

    plt.xlabel('count')
    plt.ylabel('#samples')
    plt.legend()
    plt.show()

def read_arguments(args):
    use_ugen = True
    use_qs = True
    use_uniform = True
    input_file = ''
    for i in range(1, len(args)):
        if(args[i] == '-nougen'):
            use_ugen = False
        elif(args[i] == '-noqs'):
            use_qs = False
        elif(args[i] == '-nouniform'):
            use_uniform = False
        else:
            input_file = args[i]
    if(input_file == ''):
        print("No input file specified")
        sys.exit(1)
    solvers = []
    if(use_ugen):
        solvers.append('ugen')
    if(use_qs):
        solvers.append('qs')
    if(use_uniform):
        solvers.append('uniform')
    return input_file, solvers

def init_params():
    params = {}
    params['solver_names'] = {'ugen': 'Unigen', 'qs': 'Quicksampler', 'uniform': 'Uniform'}
    params['input_file'], params['solvers'] = read_arguments(sys.argv)
    params['output_dir'] = '../out/'
    params['output_file_base'] = '.'.join(params['input_file'].split('/')[-1].split('.')[0:-1])
    params['tmp_file'] = params['output_dir'] + 'tmp'
    params['result_file'] = params['output_dir'] + 'figure1.txt'
    params['nb_samples'] = 5000000
    params['total_candidate'] = 5000000
    params['timeout'] = 3600
    model_count_cmd = './sharpSAT -q ' + params['input_file'] + ' > ' + params['tmp_file']
    os.system(model_count_cmd)
    tmp = open(params['tmp_file'], 'r')
    params['nb_models'] = int(tmp.readlines()[0].strip())
    tmp.close()
    return params

def sample(solver, params):
    if(solver == 'ugen'):
        return sample_ugen(params)
    if(solver == 'qs'):
        return sample_qs(params)
    if(solver == 'uniform'):
        return sample_uniform(params)

def sample_ugen(params):
    ugen_output_file = params['output_dir'] + params['output_file_base'] +  '_compar_ug.txt'
    unigen_options = '-samples=' + str(params['nb_samples']) + ' -timeout=' + str(params['timeout']) + ' -runIndex=compar_ug'
    unigen_cmd = 'python UniGen2.py ' + unigen_options + ' ' + params['input_file'] + ' ' + params['output_dir']
    if(not(os.path.isfile(ugen_output_file))):
        os.system(unigen_cmd)
    return calculate_counts(ugen_output_file)

def sample_qs(params):
    qs_output_file = params['output_dir'] + params['output_file_base'] + '_compar_qs.txt'
    qs_options = '-n ' + str(params['nb_samples']) + ' -t ' + str(params['timeout'])
    qs_cmd = './quicksampler ' + qs_options + ' ' + params['input_file'] \
            + ';z3 sat.quicksampler_check=true sat.quicksampler_check.timeout=' + str(params['timeout']) + ' ' + params['input_file'] \
            + ';mv ' + params['input_file'] + '.samples.valid ' + qs_output_file
    print(qs_output_file)
    print(os.path.isfile(qs_output_file))
    if not(os.path.isfile(qs_output_file)):
        print(qs_cmd)
        os.system(qs_cmd)
    return calculate_counts(qs_output_file)

def sample_uniform(params):
    simulated_samples = {}
    for i in range(1, params['nb_samples']):
        sample = random.randint(1, params['nb_models'])
        sample_mult = simulated_samples.get(sample, 0)
        simulated_samples[sample] = sample_mult + 1
    return simulated_samples.values()

def calculate_counts(output_file):
    output = open(output_file, 'r')
    counts = []
    for line in output:
        if line.strip() != '':
            counts.append(int(line.split(':')[-1]))
    output.close()
    return counts

def subsample(counts, ntot):
    samples = []
    for i in range(0, len(counts)):
        samples += counts[i] * [i]
    nsamples = numpy.random.choice(samples, ntot, False)
    ncounts = {}
    for s in nsamples:
        c = ncounts.get(s, 0)
        ncounts[s] = c + 1
    return ncounts.values()

def calculate_multiplicities(counts):
    max_count = 0
    mults = {}
    for c in counts :
        m = mults.get(c, 0)
        mults[c] = m + 1
        max_count = max(max_count, c)
    return max_count, mults

main()
