import os
import makeComparisonTable

fmdir = '../Benchmarks/FMEasy/'

benchmarks = list(map(lambda f: fmdir + f, filter(lambda f : f.endswith('.cnf'), os.listdir(fmdir))))

makeComparisonTable.main(benchmarks, 'fmeasy_comparison', {'nb_samples' : 1000000, 'timeout' : 3600, 'ugen': False})
