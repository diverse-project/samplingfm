import os
import makeComparisonTable

fmdir = '../Benchmarks/Blasted_Real/'


benchmarks = list(map(lambda f: fmdir + f, filter(lambda f : f.endswith('.cnf'), os.listdir(fmdir))))

makeComparisonTable.main(benchmarks, 'blasted_real_comparison', {'nb_samples' : 2000000})
