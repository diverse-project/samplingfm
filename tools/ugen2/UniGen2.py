import sys
import os
import math
import re
import random
import time
import math

def ensureDirectory(d):
    if not os.path.exists(d):
        os.makedirs(d)

def usage():
    usageStr = "Usage: python UniGen2.py [options] <inputFile> <outputFolder>\n"
    usageStr += "Generated samples (and log files if enabled) are written to <outputFolder>.\n"
    usageStr += "Options are entered as '-option=value' where 'option' is from the list below.\n"
    usageStr += "If an option is not specified, the default value given below is used.\n"
    usageStr += "\nBASIC OPTIONS:\n\n"
    usageStr += "samples: number of samples to generate (default: 1) \n"
    usageStr += "kappa: computed from desired tolerance as in Algorithm 1 of the TACAS-15 paper (default: 0.638, corresponding to epsilon=16)\n"
    usageStr += "threads: number of threads to use for sampling (default: OpenMP system default)\n"
    usageStr += "timeout: overall timeout in seconds (default: 72000)\n"
    usageStr += "satTimeout: timeout for a single BoundedWeightSAT call in seconds (default: 3000)\n"
    usageStr += "runIndex: number appended to names of output & log files (default: current time) \n"
    usageStr += "\nADVANCED OPTIONS (not needed for typical use):\n\n"
    usageStr += "logging: 0 to turn off logging and 1 to turn it on. If logging is turned on, the log file(s) will be present in <outputFolder>/logging\n"
    usageStr += "aggregateSolutions: 0/1 to disable/enable merging samples from all threads (default: 1)\n"
    usageStr += "writeSamples: 0/1 to disable/enable writing samples to a file (default: 1)\n"
    usageStr += "multisampling: 0/1 to disable/enable returning multiple samples from each UniGen2 call (default: 1)\n"
    usageStr += "startIteration: initial number of XOR clauses to use in UniGen2, or 0 if this should be computed using sharpSAT/ApproxMC, or -1 if it should be computed using ApproxMC only (e.g. when doing projection counting) (default: 0)\n"
    usageStr += "callsPerSolver: number of UniGen2 calls to make in a single solver (without clearing learned clauses), or 0 to use a built-in heuristic (default: 0)\n"
    usageStr += "pivotAC: pivot value for ApproxMC (default: 60)\n"
    usageStr += "tApproxMC: number of iterations for ApproxMC (default: 1)\n"
    print usageStr
    sys.exit(1)

def getInputs():
    paramMap={}
    action=0
    error = ''
    gotInputFile = False
    acceptedParams=['timeout','satTimeout','runIndex','samples','callsPerSolver','writeSamples','threads','aggregateSolutions','logging','kappa','multisampling','startIteration','pivotAC','tApproxMC']
    for i in range(1,len(sys.argv)):
        if (not(sys.argv[i][0] == '-')):
            if not gotInputFile:
                paramMap['inputFile'] = sys.argv[i]
                gotInputFile = True
            else:
                paramMap['outputFolder'] = sys.argv[i]
                action = 3
                return action,error,paramMap
        else:
            if (sys.argv[i][1] == 'h'):
                action = 0
                return action,error,paramMap
            fieldValues = sys.argv[i][1:].strip().split('=')
            if (not(fieldValues[0] in acceptedParams)):
                action = 1
                error = "Could not understand the option "+str(fieldValues[0])+"\n"
                return action,error,paramMap
            else:
                paramMap[fieldValues[0]] = fieldValues[1]
    action = 2
    error = "You must specify an input file and an output folder.\nUse the option -h for help.\n"
    return action,error,paramMap

def main():
    runIndex = str(int(time.time()))
    timeout = 72000
    satTimeout = 3000
    initialFileName = ''
    numVariables = 0
    numClauses = 0
    shouldLog = False
    outputFileName = ''
    logFileName = ''
    epsilon = 0
    pivotUniGen = 0
    kappa = 0.638
    samples = 1
    writeSamples = True
    multisampling = True
    threads = 0
    aggregateSolutions = True
    xorDensityParameter = 0
    pivotAC = 60
    approxMCIterations = 1
    startIteration = 0
    callsPerSolver = 0
    action,error,paramMap = getInputs()
    if (action == 0):
        usage()
        sys.exit(1)
    if (action == 1 or action == 2):
        print error
        sys.exit(1)
    if (paramMap.has_key('runIndex')):
        runIndex = int(paramMap['runIndex'])
    if (paramMap.has_key('samples')):
        samples = int(paramMap['samples'])
    if (paramMap.has_key('callsPerSolver')):
        callsPerSolver = int(paramMap['callsPerSolver'])
    if (paramMap.has_key('writeSamples')):
        writeSamples = (paramMap['writeSamples'] == '1')
    if (paramMap.has_key('timeout')):
        timeout = float(paramMap['timeout'])
    if (paramMap.has_key('satTimeout')):
        satTimeout = float(paramMap['satTimeout'])
    if (paramMap.has_key('kappa')):
        kappa = float(paramMap['kappa'])
    if (paramMap.has_key('outputFile')):
        outputFileName = paramMap['outputFile']
    if (paramMap.has_key('logging')):
        if (paramMap['logging'] == '0'):
            shouldLog = False
        if (paramMap['logging'] == '1'):
            shouldLog = True
    if (paramMap.has_key('multisampling')):
        if (paramMap['multisampling'] == '0'):
            multisampling = False
        if (paramMap['multisampling'] == '1'):
            multisampling = True
    if (paramMap.has_key('aggregateSolutions')):
        if (paramMap['aggregateSolutions'] == '0'):
            aggregateSolutions = False
        if (paramMap['aggregateSolutions'] == '1'):
            aggregateSolutions = True
    if (paramMap.has_key('threads')):
        threads = int(paramMap['threads'])
    if (paramMap.has_key('pivotAC')):
        pivotAC = int(paramMap['pivotAC'])
    if (paramMap.has_key('tApproxMC')):
        approxMCIterations = int(paramMap['tApproxMC'])
    if (paramMap.has_key('startIteration')):
        startIteration = int(paramMap['startIteration'])
    initialFileName = paramMap['inputFile']
    outputFolder = paramMap['outputFolder']
    initialFileNameSuffix =  initialFileName.split('/')[-1][:-4]
    ensureDirectory(outputFolder)
    if (shouldLog):
        ensureDirectory(outputFolder+"/logging/")
        logFilePrefix = outputFolder+"/logging/"+str(initialFileNameSuffix)+'_'+str(runIndex)
    outputFileName = outputFolder+"/"+str(initialFileNameSuffix)+'_'+str(runIndex)+".txt"

    pivotUniGen = math.ceil(4.03*(1+1/kappa)*(1+1/kappa))

    if startIteration < 0:
        startIteration = 0
        print "Doing projection counting; will use ApproxMC"
    elif startIteration == 0:
        startIteration = 0
        print "Attempting to compute startIteration with sharpSAT"
        countLogFile = outputFolder+"/"+initialFileNameSuffix+"_"+str(runIndex)+".count"
        cmd = "./doalarm -t real 300 ./sharpSAT -q "+initialFileName+" > "+ countLogFile
        print cmd
        os.system(cmd)
        f = open(countLogFile,'r')
        lines = f.readlines()
        f.close()
        failed = True
        if (len(lines) > 1):
            if (lines[1].strip() == "END"):
                failed = False
                count = long(lines[0].strip())
                if (count == 0):
                    print "Unsatisfiable formula!"
                    return;
                logCount = math.log(count, 2)
                startIteration = int(round(logCount + math.log(1.8, 2) - math.log(pivotUniGen, 2))) - 2
                if (startIteration < 0):
                    print "The number of solutions is only "+str(count)+"\n"
                    print "The best technique is to just enumerate all the solutions and choose one"
                    return;
                    startIteration = 0
                #print "Solution count estimate is %f * 2^%d" % (count / (2.0 ** int(logCount)), int(logCount))
        if failed:
            print "sharpSAT failed or timed out; using ApproxMC instead"
        else:
            print "startIteration computed successfully"

    cmd = "./unigen --samples="+str(samples)+" --kappa="+str(kappa)+" --pivotUniGen="+str(pivotUniGen)+" --maxTotalTime="+str(timeout)+" --startIteration="+str(startIteration)+" --maxLoopTime="+str(satTimeout)+" --tApproxMC="+str(approxMCIterations)+" --pivotAC="+str(pivotAC)+" --gaussuntil=400 --verbosity=0"
    if (shouldLog):
        cmd += " --logFile="+logFilePrefix
    if callsPerSolver > 0:
        cmd += " --callsPerSolver="+str(callsPerSolver)
    if multisampling:
        cmd += " --multisample"
    if not writeSamples:
        cmd += " --nosolprint"
    if threads > 0:
        cmd += " --threads="+str(threads)
    if not aggregateSolutions:
        cmd += " --noAggregation"
    cmd += " "+initialFileName+" "+outputFileName
    print cmd
    sys.stdout.flush()
    os.system(cmd)

main()    
