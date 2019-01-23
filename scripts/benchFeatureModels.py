import os
import makeComparisonTable

fmdir = '../Benchmarks/FeatureModels/'


benchmarks = list(map(lambda f: fmdir + f, filter(lambda f : f.endswith('.cnf'), os.listdir(fmdir))))

makeComparisonTable.main(benchmarks, 'fm_comparison', {'nb_samples' : 500000, 'timeout' : 60, 'ugen': False})
