import sys
import os
import matplotlib.pyplot as plt

def main():

    input_file = sys.argv[1]
    output_dir = '../out/'
    output_file = output_dir + input_file.split('/')[-1].split('.')[0] + '_0.txt'
    tmp_file = output_dir + 'tmp'
    result_file = output_dir + 'figure1.txt'
    unigen_options = '-samples=4000000 -threads=1 -runIndex=0'
    unigen_cmd = 'python UniGen2.py ' + unigen_options + ' ' + input_file + ' ' + output_dir
    model_count_cmd = './sharpSAT -q ' + input_file + ' > ' + tmp_file
    os.system(model_count_cmd)
    tmp = open(tmp_file, 'r')
    lines = tmp.readlines()
    nb_models = int(lines[0].strip())
    tmp.close()
    os.system(unigen_cmd)
    
    output = open(output_file, 'r')
    multiplicities = {}
    max_multiplicity = 0
    for line in output:
        if line.strip() != '':
            multiplicity = int(line.split(':')[-1])
            m = multiplicities.get(multiplicity, 0)
            multiplicities[multiplicity] = m+1
            if(multiplicity > max_multiplicity):
                max_multiplicity = m+1
    output.close()

    x = range(1, max_multiplicity) 
    y = list(map(lambda m : multiplicities.get(m, 0), x))

    plt.plot(x, y)
    plt.show()

main()
