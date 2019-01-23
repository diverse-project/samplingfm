import sys
import os
import matplotlib.pyplot as plt
import math

outputDir = '../out/'
tmpFileName = outputDir + '.tmp'
tmpCnfFileName = outputDir + '.tmp.cnf'

def main():
    options = {'useUg': True, 'useQs' : True}
    readArgs(options, sys.argv[1:])
    cnfFileName = options['fileName']
    baseFileName = '.'.join(cnfFileName.split('/')[-1].split('.')[:-1])
    qsFileName = outputDir + baseFileName + '_compar_qs.txt'
    ugFileName = outputDir + baseFileName + '_compar_ug.txt'

    featureIndex = makeFeatureIndex(cnfFileName)
    nbFeatures = len(featureIndex)

    modelCount = sharpSatCall(cnfFileName, [])
    if modelCount == 0:
        print('Unable to count models')
        return

    if(options['useQs']):
        if(not(os.path.isfile(qsFileName))):
            print('Computing quicksampler samples')
            qsCmd = './quicksampler -n 2000000 -t 3600 ' + cnfFileName
            os.system(qsCmd)
            qsCheckCmd = 'z3 sat.quicksampler_check=true sat.quicksampler_check.timeout=3600 ' + cnfFileName
            os.system(qsCheckCmd)
            qsSampleFile = cnfFileName + '.samples'
            qsValidSampleFile = qsSampleFile + '.valid'
            os.system('mv ' + qsValidSampleFile + ' ' + qsFileName + ';rm ' + qsSampleFile)
        else:
            print('Using existing samples for quicksampler, remove sample file in ../out/ manually to recompute')
                        

    if(options['useUg']):
        if(not(os.path.isfile(ugFileName))):
            print('Computing unigen Samples')
            ugCmd = "python UniGen2.py -runIndex=compar_ug -timeout=3600 -samples=2000000 " + cnfFileName + ' ' + outputDir
            os.system(ugCmd)
        else:
            print('Using existing samples for unigen, remove sample file in ../out/ manually to recompute')

    print('Calculating Frequencies')
    freqs={}
    if(options['useQs']):
        freqs['qs'] = calculateFreqs(qsFileName, featureIndex)
    if(options['useUg']):
        freqs['ug'] = calculateFreqs(ugFileName, featureIndex)
    thFreqs = calculateThFreqs(cnfFileName, modelCount, featureIndex)

    displayResults(baseFileName, featureIndex, freqs, thFreqs)
    
def readArgs(options, args):
    for arg in args:
        if arg.startswith('-'):
            if arg == '-noug':
                options['useUg'] = False
            if arg == '-noqs':
                options['useQs'] = False
        else:
            options['fileName'] = arg

def makeFeatureIndex(cnf):
    cnfFile = open(cnf, 'r')
    index = {}
    for line in cnfFile:
        if line.startswith('c'):
            words = line.strip().split(' ')
            if(len(words) == 3):
                index[int(words[1])] = words[2]
    cnfFile.close()
    return index


def calculateFreqs(samples, featureIndex):
    sampleFile = open(samples, 'r')
    sums = {}
    tot = 0
    for line in sampleFile:
        if line.strip().startswith('v'):
            line = line.strip()[1:]
        if line.strip() != '':
            assignment, count = line.split(' 0:')
            model = frozenset(filter(lambda x : x > 0, map(int, assignment.split(' '))))
            tot += int(count)
            for i in model:
                theSum = sums.get(i, 0)
                sums[i] = theSum + int(count)
    sampleFile.close()
    return {k: float(v) / float(tot) for k, v in sums.items()}

def calculateThFreqs(cnfFile, modelCount, featureIndex):
    thFreqs = {}
    for i in featureIndex:
        featureCount = sharpSatCall(cnfFile, [[i]])
        sys.stdout.write('.')
        sys.stdout.flush()
        thFreqs[i] = float(featureCount)/float(modelCount)
    print('')
    return thFreqs

def sharpSatCall(cnfFileName, assumptions):
    with open(cnfFileName, 'r') as cnfFile:
        with open(tmpCnfFileName, 'w') as tmpCnfFile:
            lines = cnfFile.readlines()
            for i in range(0, len(lines)):
                if lines[i].startswith('p cnf'):
                    words = lines[i].split(' ')
                    words[-1] = str(int(words[-1]) + len(assumptions)) + '\n'
                    lines[i] = ' '.join(words)
            assumptionLines = map(lambda x : '\n' + ' '.join(map(str, x)) + ' 0', assumptions)
            tmpCnfFile.writelines(lines + assumptionLines)
    sharpSatCmd = './doalarm 300 ./sharpSAT -q ' + tmpCnfFileName + ' > ' + tmpFileName
    os.system(sharpSatCmd)
    try:
        with open(tmpFileName, 'r') as resFile :
            line = resFile.readline()
            return int(line)
    except:
        return 0

def displayResults(fileName, featureIndex, allFreqs, thFreqs):
    for sampler, freqs in allFreqs.items() :
        logFileName = outputDir + 'FrequencyDiagrams/' + fileName + '_' + sampler + '_log.txt'
        logFile = open(logFileName, 'w')
        results = []
        for k in featureIndex:
            thFreq = thFreqs.get(k, 0)
            freq = freqs.get(k, 0)
            logFile.write(str(k) + ' ' + featureIndex[k] + '\n obs : ' + str(freq) + '\n th  : ' + str(thFreq) + '\n')
            if(thFreq != 0):
                results.append(abs(thFreq-freq) * 100 / thFreq)
            else:
                results.append(0)
        logFile.close()
        l = len(results)
        x = range(l+1)
        results.sort()
        plt.hist(x[:-1], x, weights=results)
        plt.xlabel('Features')
        if(max(results) > 800):
            plt.ylim(ymax=800)
        plt.plot([0, l], [10, 10], 'g--')
        plt.plot([0, l], [50, 50], 'r--')
        plt.ylabel('Frequency deviation (%)')
        saveLocation = outputDir + 'FrequencyDiagrams/' + fileName + '_' + sampler + '.png'
        plt.savefig(saveLocation, bbox_inches='tight')
        print('Generated figure for ' + sampler + ' in ' + saveLocation)
        plt.close('all')

        

main()
