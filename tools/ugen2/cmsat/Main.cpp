/***************************************************************************************
Copyright (c) 2003-2006, Niklas Een, Niklas Sorensson
Copyright (c) 2007-2009, Niklas Sorensson
Copyright (c) 2009-2012, Mate Soos
Copyright (c) 2014, Supratik Chakraborty, Kuldeep S. Meel, Moshe Y. Vardi
Copyright (c) 2015, Supratik Chakraborty, Daniel J. Fremont, Kuldeep S. Meel, Sanjit A. Seshia, Moshe Y. Vardi
Copyright (c) 2016-2017, Kuldeep S. Meel
Permission is hereby granted, free of charge, to any person obtaining a copy of this software and
associated documentation files (the "Software"), to deal in the Software without restriction,
including without limitation the rights to use, copy, modify, merge, publish, distribute,
sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or
substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT
NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND
NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM,
DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT
OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
 **************************************************************************************************/

/**
@mainpage UniGen2
@author Kuldeep S. Meel, Daniel J. Fremont
 */

#include <ctime>
#include <cstring>
#include <errno.h>
#include <string.h>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <omp.h>
#include <map>
#include <set>
#include "cmsat/constants.h"
#include <signal.h>
#include <time.h>
#include <list>
#include <algorithm>
#include <signal.h>
#include <functional>

#include "cmsat/time_mem.h"
#include "cmsat/constants.h"
#include "cmsat/DimacsParser.h"

#if defined(__linux__)
#include <fpu_control.h>
#endif

#include "cmsat/Main.h"

using namespace CMSat;

Main::Main(int _argc, char** _argv) :
numThreads(0)
, grouping(false)
, debugLib(false)
, debugNewVar(false)
, printResult(true)
, max_nr_of_solutions(1)
, fileNamePresent(false)
, twoFileNamesPresent(false)
, argc(_argc)
, argv(_argv) {
}


double findMean(std::list<int> numList) {
    double sum = 0;
    for (std::list<int>::iterator it = numList.begin(); it != numList.end(); it++) {
        sum += *it;
    }
    return (sum * 1.0 / numList.size());
}

double findMedian(std::list<int> numList) {
    numList.sort();
    int medIndex = int((numList.size() + 1) / 2);
    std::list<int>::iterator it = numList.begin();
    if (medIndex >= (int) numList.size()) {
        std::advance(it, numList.size() - 1);
        return double(*it);
    }
    std::advance(it, medIndex);
    return double(*it);
}

int findMin(std::list<int> numList) {
    int min = INT_MAX;
    for (std::list<int>::iterator it = numList.begin(); it != numList.end(); it++) {
        if ((*it) < min) {
            min = *it;
        }
    }
    return min;
}
std::map<uint32_t, Solver*> solversToInterrupt;
std::set<uint32_t> finished;
timer_t *mytimer;
bool need_clean_exit;
bool *timerSetFirstTime;
void start_timer(int num) {
    int threadNum = omp_get_thread_num();
    struct sigevent sev;
    sev.sigev_signo = SIGUSR1;
    sev.sigev_notify = SIGEV_SIGNAL;
    sev.sigev_value.sival_int = threadNum;
    //printf("start_timer;thread:%d\n", threadNum);
    struct itimerspec value;
    value.it_value.tv_sec = num; //waits for n seconds before sending timer signal
    value.it_value.tv_nsec = 0;
    value.it_interval.tv_sec = 0; //exipire once
    value.it_interval.tv_nsec = 0;
    if (timerSetFirstTime[threadNum]){
    timer_create(CLOCK_REALTIME, &sev, &mytimer[threadNum]);
  //  timer_delete(mytimer);
    }
    timerSetFirstTime[threadNum] = false;
    timer_settime(mytimer[threadNum], 0, &value, NULL);
}
void SIGINT_handler(int) {
#pragma omp critical
  exit(1);
}
void SIGALARM_handler(int sig, siginfo_t *si, void *uc) {
#pragma omp critical
    {
        int num = si->si_value.sival_int;
        //printf("SIGLARM:%d\n",num);
        Solver& solver = * solversToInterrupt[num];
        //Solver& solver = *solversToInterrupt.begin()->second;
        //printf("\n");
        //std::cerr << "*** INTERRUPTED ***" << std::endl;
        if (solver.conf.needToDumpLearnts || solver.conf.needToDumpOrig || need_clean_exit) {
            solver.needToInterrupt = true;
            //std::cerr << "*** Please wait. We need to interrupt cleanly" << std::endl;
            //std::cerr << "*** This means we might need to finish some calculations" << std::endl;
        } else {
            if (solver.conf.verbosity >= 1) solver.printStats();
            exit(1);
        }
    }
}
void Main::readInAFile(const std::string& filename, Solver& solver) {
    if (solver.conf.verbosity >= 1) {
        std::cout << "c Reading file '" << filename << "'" << std::endl;
    }
#ifdef DISABLE_ZLIB
    FILE * in = fopen(filename.c_str(), "rb");
#else
    gzFile in = gzopen(filename.c_str(), "rb");
#endif // DISABLE_ZLIB

    if (in == NULL) {
        std::cout << "ERROR! Could not open file '" << filename << "' for reading" << std::endl;
        exit(1);
    }

    DimacsParser parser(&solver, debugLib, debugNewVar, grouping);
    parser.parse_DIMACS(in);

#ifdef DISABLE_ZLIB
    fclose(in);
#else
    gzclose(in);
#endif // DISABLE_ZLIB
}

void Main::readInStandardInput(Solver& solver) {
    if (solver.conf.verbosity >= 1) {
        std::cout << "c Reading from standard input... Use '-h' or '--help' for help." << std::endl;
    }
#ifdef DISABLE_ZLIB
    FILE * in = stdin;
#else
    gzFile in = gzdopen(fileno(stdin), "rb");
#endif // DISABLE_ZLIB

    if (in == NULL) {
        std::cout << "ERROR! Could not open standard input for reading" << std::endl;
        exit(1);
    }

    DimacsParser parser(&solver, debugLib, debugNewVar, grouping);
    parser.parse_DIMACS(in);

#ifndef DISABLE_ZLIB
    gzclose(in);
#endif // DISABLE_ZLIB
}

void Main::parseInAllFiles(Solver& solver) {
    double myTime = cpuTime();

    //First read normal extra files
    if ((debugLib || debugNewVar) && filesToRead.size() > 0) {
        std::cout << "debugNewVar and debugLib must both be OFF to parse in extra files" << std::endl;
        exit(-1);
    }
    for (uint32_t i = 0; i < filesToRead.size(); i++) {
        readInAFile(filesToRead[i].c_str(), solver);
    }

    //Then read the main file or standard input
    if (!fileNamePresent) {
        readInStandardInput(solver);
    } else {
        string filename = argv[(twoFileNamesPresent ? argc - 2 : argc - 1)];
        readInAFile(filename, solver);
    }

    if (solver.conf.verbosity >= 1) {
        std::cout << "c Parsing time: "
                << std::fixed << std::setw(5) << std::setprecision(2) << (cpuTime() - myTime)
                << " s" << std::endl;
    }
}

void Main::printUsage(char** argv) {
#ifdef DISABLE_ZLIB
    printf("USAGE: %s [options] <input-file> <result-output-file>\n\n  where input is plain DIMACS.\n\n", argv[0]);
#else
    printf("USAGE: %s [options] <input-file> <result-output-file>\n\n  where input may be either in plain or gzipped DIMACS.\n\n", argv[0]);
#endif // DISABLE_ZLIB
    printf("OPTIONS:\n\n");
    printf("  --polarity-mode  = {true,false,rnd,auto} [default: auto]. Selects the default\n");
    printf("                     polarity mode. Auto is the Jeroslow&Wang method\n");
    //printf("  -decay         = <num> [ 0 - 1 ]\n");
    printf("  --rnd-freq       = <num> [ 0 - 1 ]\n");
    printf("  --verbosity      = {0,1,2}\n");
    printf("  --randomize      = <seed> [0 - 2^32-1] Sets random seed, used for picking\n");
    printf("                     decision variables (default = 0)\n");
    printf("  --restrict       = <num> [1 - varnum] when picking random variables to branch\n");
    printf("                     on, pick one that in the 'num' most active vars useful\n");
    printf("                     for cryptographic problems, where the question is the key,\n");
    printf("                     which is usually small (e.g. 80 bits)\n");
    printf("  --gaussuntil     = <num> Depth until which Gaussian elimination is active.\n");
    printf("                     Giving 0 switches off Gaussian elimination\n");
    printf("  --restarts       = <num> [1 - 2^32-1] No more than the given number of\n");
    printf("                     restarts will be performed during search\n");
    printf("  --nonormxorfind    Don't find and collect >2-long xor-clauses from\n");
    printf("                     regular clauses\n");
    printf("  --nobinxorfind     Don't find and collect 2-long xor-clauses from\n");
    printf("                     regular clauses\n");
    printf("  --noregbxorfind    Don't regularly find and collect 2-long xor-clauses\n");
    printf("                     from regular clauses\n");
    printf("  --doextendedscc    Do strongly conn. comp. finding using non-exist. bins\n");
    printf("  --noconglomerate   Don't conglomerate 2 xor clauses when one var is dependent\n");
    printf("  --nosimplify       Don't do regular simplification rounds\n");
    printf("  --greedyunbound    Greedily unbound variables that are not needed for SAT\n");
    printf("  --debuglib         Solve at specific 'c Solver::solve()' points in the CNF\n");
    printf("                     file. Used to debug file generated by Solver's\n");
    printf("                     needLibraryCNFFile() function\n");
    printf("  --debugnewvar      Add new vars at specific 'c Solver::newVar()' points in \n");
    printf("                     the CNF file. Used to debug file generated by Solver's\n");
    printf("                     needLibraryCNFFile() function.\n");
    printf("  --novarreplace     Don't perform variable replacement. Needed for programmable\n");
    printf("                     solver feature\n");
    printf("  --restart        = {auto, static, dynamic}   Which kind of restart strategy to\n");
    printf("                     follow. Default is auto\n");
    printf("  --dumplearnts    = <filename> If interrupted or reached restart limit, dump\n");
    printf("                     the learnt clauses to the specified file. Maximum size of\n");
    printf("                     dumped clauses can be specified with next option.\n");
    printf("  --maxdumplearnts = [0 - 2^32-1] When dumping the learnts to file, what\n");
    printf("                     should be maximum length of the clause dumped. Useful\n");
    printf("                     to make the resulting file smaller. Default is 2^32-1\n");
    printf("                     note: 2-long XOR-s are always dumped.\n");
    printf("  --dumporig       = <filename> If interrupted or reached restart limit, dump\n");
    printf("                     the original problem instance, simplified to the\n");
    printf("                     current point.\n");
    printf("  --alsoread       = <filename> Also read this file in\n");
    printf("                     Can be used to re-read dumped learnts, for example\n");
    printf("  --maxsolutions     Search for given amount of solutions\n");
    printf("                     Can only be used in single-threaded more (\"--threads=1\")\n");
    printf("  --pavgbranch       Print average branch depth\n");
    printf("  --nofailedlit      Don't search for failed literals, and don't search for lits\n");
    printf("                     propagated both by 'varX' and '-varX'\n");
    printf("  --noheuleprocess   Don't try to minimise XORs by XOR-ing them together.\n");
    printf("                     Algo. as per global/local substitution in Heule's thesis\n");
    printf("  --nosatelite       Don't do clause subsumption, clause strengthening and\n");
    printf("                     variable elimination (implies -novarelim and -nosubsume1).\n");
    printf("  --noxorsubs        Don't try to subsume xor-clauses.\n");
    printf("  --nosolprint       Don't print the satisfying assignment if the solution\n");
    printf("                     is SAT\n");
    printf("  --novarelim        Don't perform variable elimination as per Een and Biere\n");
    printf("  --nosubsume1       Don't perform clause contraction through resolution\n");
#ifdef USE_GAUSS
    printf("  --nomatrixfind     Don't find distinct matrixes. Put all xors into one\n");
    printf("                     big matrix\n");
    printf("  --noordercol       Don't order variables in the columns of Gaussian\n");
    printf("                     elimination. Effectively disables iterative reduction\n");
    printf("                     of the matrix\n");
    printf("  --noiterreduce     Don't reduce iteratively the matrix that is updated\n");
    printf("  --maxmatrixrows    [0 - 2^32-1] Set maximum no. of rows for gaussian matrix.\n");
    printf("                     Too large matrixes should bee discarded for\n");
    printf("                     reasons of efficiency. Default: %d\n", gaussconfig.maxMatrixRows);
    printf("  --minmatrixrows  = [0 - 2^32-1] Set minimum no. of rows for gaussian matrix.\n");
    printf("                     Normally, too small matrixes are discarded for\n");
    printf("                     reasons of efficiency. Default: %d\n", gaussconfig.minMatrixRows);
    printf("  --savematrix     = [0 - 2^32-1] Save matrix every Nth decision level.\n");
    printf("                     Default: %d\n", gaussconfig.only_nth_gauss_save);
    printf("  --maxnummatrixes = [0 - 2^32-1] Maximum number of matrixes to treat.\n");
    printf("                     Default: %d\n", gaussconfig.maxNumMatrixes);
#endif //USE_GAUSS
    //printf("  --addoldlearnts  = Readd old learnts for failed variable searching.\n");
    //printf("                     These learnts are usually deleted, but may help\n");
    printf("  --nohyperbinres    Don't add binary clauses when doing failed lit probing.\n");
    printf("  --noremovebins     Don't remove useless binary clauses\n");
    printf("  --noremlbins       Don't remove useless learnt binary clauses\n");
    printf("  --nosubswithbins   Don't subsume with binary clauses\n");
    printf("  --nosubswithnbins  Don't subsume with non-existent binary clauses\n");
    printf("  --noclausevivif    Don't do perform clause vivification\n");
    printf("  --nosortwatched    Don't sort watches according to size: bin, tri, etc.\n");
    printf("  --nolfminim        Don't do on-the-fly self-subsuming resolution\n");
    printf("                     (called 'strong minimisation' in PrecoSat)\n");
    printf("  --nocalcreach      Don't calculate reachability and interfere with\n");
    printf("                     variable decisions accordingly\n");
    printf("  --nobxor           Don't find equivalent lits during failed lit search\n");
    printf("  --norecotfssr      Don't perform recursive/transitive OTF self-\n");
    printf("                     subsuming resolution\n");
    printf("  --nocacheotfssr    Don't cache 1-level equeue. Less memory used, but\n");
    printf("                     disables trans OTFSSR, adv. clause vivifier, etc.\n");
    printf("  --nootfsubsume     Don't do on-the-fly subsumption after conf. gen.\n");
#ifdef ENABLE_UNWIND_GLUE
    printf("  --maxgluedel       Automatically delete clauses over max glue. See '--maxglue'\n");
    printf("  --maxglue        = [0 - 2^%d-1] default: %d. Glue value above which we\n", MAX_GLUE_BITS, conf.maxGlue);
#endif //ENABLE_UNWIND_GLUE
    printf("                     throw the clause away on backtrack.\n");
    printf("  --threads        = Num threads (default is 1)\n");
    printf("  --plain            Get rid of all simplification algorithms\n");
    printf("  --maxconfl       = [0..2^63-1] Maximum number of conflicts to do\n");
    printf("  --maxtime        = [0..] Maximum number of seconds to run after which we exit cleanly\n");
    printf("  --switchoffsubs  = Number of variables after which to switch off subsumption and all related algorithms. Saves time. Default: %ld\n", conf.switch_off_subsumer_max_vars);
    printf("\n");
}

const char* Main::hasPrefix(const char* str, const char* prefix) {
    int len = strlen(prefix);
    if (strncmp(str, prefix, len) == 0)
        return str + len;
    else
        return NULL;
}

void Main::parseCommandLine() {
    const char* value;
    char tmpFilename[201];
    tmpFilename[0] = '\0';
    uint32_t unparsedOptions = 0;
    bool needTwoFileNames = false;
    conf.verbosity = 2;
    need_clean_exit = false;

    for (int i = 0; i < argc; i++) {
        if ((value = hasPrefix(argv[i], "--polarity-mode="))) {
            if (strcmp(value, "true") == 0)
                conf.polarity_mode = polarity_true;
            else if (strcmp(value, "false") == 0)
                conf.polarity_mode = polarity_false;
            else if (strcmp(value, "rnd") == 0)
                conf.polarity_mode = polarity_rnd;
            else if (strcmp(value, "auto") == 0)
                conf.polarity_mode = polarity_auto;
            else {
                printf("ERROR! unknown polarity-mode %s\n", value);
                exit(0);
            }

        } else if ((value = hasPrefix(argv[i], "--rnd-freq="))) {
            double rnd;
            if (sscanf(value, "%lf", &rnd) <= 0 || rnd < 0 || rnd > 1) {
                printf("ERROR! illegal rnRSE ERROR!d-freq constant %s\n", value);
                exit(0);
            }
            conf.random_var_freq = rnd;

            /*} else if ((value = hasPrefix(argv[i], "--decay="))) {
                double decay;
                if (sscanf(value, "%lf", &decay) <= 0 || decay <= 0 || decay > 1) {
                    printf("ERROR! illegal decay constant %s\n", value);
                    exit(0);
                }
                conf.var_decay = 1 / decay;*/
        } else if ((value = hasPrefix(argv[i], "--samples="))) {
            int samples;

            if (sscanf(value, "%d", &samples) < 0) {
                printf("ERROR! Illegal samples %s\n", value);
            }
            conf.samples = samples;
        } else if ((value = hasPrefix(argv[i], "--callsPerSolver="))) {
            int callsPerSolver;
            if (sscanf(value, "%d", &callsPerSolver) < 0) {
                printf("ERROR! Illegal callsPerSolver %s\n", value);
            }
            conf.callsPerSolver = callsPerSolver;
        } else if ((value = hasPrefix(argv[i], "--pivotAC="))) {
            int pivot;

            if (sscanf(value, "%d", &pivot) < 0) {
                printf("ERROR! Illegal pivotAC %s\n", value);
            }

            conf.pivotApproxMC = pivot;
        } else if ((value = hasPrefix(argv[i], "--pivotUniGen="))) {
            int pivot;

            if (sscanf(value, "%d", &pivot) < 0) {
                printf("ERROR! Illegal pivotUniGen %s\n", value);
            }
            conf.pivotUniGen = pivot;
        } else if ((value = hasPrefix(argv[i], "--kappa="))) {
            float kappa;

            if (sscanf(value, "%f", &kappa) < 0) {
                printf("ERROR! Illegal pivotUniGen %s\n", value);
            }
            conf.kappa = kappa; 
        }else if ((value = hasPrefix(argv[i], "--tApproxMC="))) {
            int t;

            if (sscanf(value, "%d", &t) < 0) {
                printf("ERROR! Illegal pivot %d\n", t);
            }
            conf.tApproxMC = t;
        }else if ((value = hasPrefix(argv[i], "--startIteration="))) {
            int t;
            if (sscanf(value, "%d", &t) < 0) {
                printf("ERROR! Illegal startIteration %d\n", t);
            }
            conf.startIteration = t;
        } else if ((value = hasPrefix(argv[i], "--multisample"))) {
            conf.multisample = true;
        } else if ((value = hasPrefix(argv[i], "--noAggregation"))) {
            conf.aggregateSolutions = false;
        } else if ((value = hasPrefix(argv[i], "--verbosity="))) {
            int verbosity = (int) strtol(value, NULL, 10);
            if (verbosity == EINVAL || verbosity == ERANGE) {
                printf("ERROR! illegal verbosity level %s\n", value);
                exit(0);
            }
            conf.verbosity = verbosity;
        } else if ((value = hasPrefix(argv[i], "--randomize="))) {
            int seed;
            if (sscanf(value, "%d", &seed) < 0) {
                printf("ERROR! illegal seed %s\n", value);
                exit(0);
            }
            conf.origSeed = seed;
        } else if ((value = hasPrefix(argv[i], "--restrict="))) {
            int branchTo;
            if (sscanf(value, "%d", &branchTo) < 0 || branchTo < 1) {
                printf("ERROR! illegal restricted pick branch number %d\n", branchTo);
                exit(0);
            }
            conf.restrictPickBranch = branchTo;
        } else if ((value = hasPrefix(argv[i], "--gaussuntil="))) {
            int until;
            if (sscanf(value, "%d", &until) < 0) {
                printf("ERROR! until %s\n", value);
                exit(0);
            }
            gaussconfig.decision_until = until;
        } else if ((value = hasPrefix(argv[i], "--restarts="))) {
            int maxrest;
            if (sscanf(value, "%d", &maxrest) < 0 || maxrest == 0) {
                printf("ERROR! illegal maximum restart number %d\n", maxrest);
                exit(0);
            }
            conf.maxRestarts = maxrest;
        } else if ((value = hasPrefix(argv[i], "--dumplearnts="))) {
            if (sscanf(value, "%200s", tmpFilename) < 0 || strlen(tmpFilename) == 0) {
                printf("ERROR! wrong filename '%s'\n", tmpFilename);
                exit(0);
            }
            conf.learntsFilename.assign(tmpFilename);
            conf.needToDumpLearnts = true;
        } else if ((value = hasPrefix(argv[i], "--dumporig="))) {
            if (sscanf(value, "%200s", tmpFilename) < 0 || strlen(tmpFilename) == 0) {
                printf("ERROR! wrong filename '%s'\n", tmpFilename);
                exit(0);
            }
            conf.origFilename.assign(tmpFilename);
            conf.needToDumpOrig = true;
        } else if ((value = hasPrefix(argv[i], "--logFile="))) {
            if (sscanf(value, "%200s", tmpFilename) < 0 || strlen(tmpFilename) == 0) {
                printf("ERROR! wrong fileName '%s'\n", tmpFilename);
                exit(0);
            }
            conf.shouldLog = true;
            conf.logFilename.assign(tmpFilename);
        } else if ((value = hasPrefix(argv[i], "--alsoread="))) {
            if (sscanf(value, "%400s", tmpFilename) < 0 || strlen(tmpFilename) == 0) {
                printf("ERROR! wrong filename '%s'\n", tmpFilename);
                exit(0);
            }
            filesToRead.push_back(tmpFilename);
        } else if ((value = hasPrefix(argv[i], "--maxdumplearnts="))) {
            if (!conf.needToDumpLearnts) {
                printf("ERROR! -dumplearnts=<filename> must be first activated before issuing -maxdumplearnts=<size>\n");
                exit(0);
            }
            int tmp;
            if (sscanf(value, "%d", &tmp) < 0 || tmp < 0) {
                std::cout << "ERROR! wrong maximum dumped learnt clause size is illegal: " << tmp << std::endl;
                exit(0);
            }
            conf.maxDumpLearntsSize = (uint32_t) tmp;
        } else if ((value = hasPrefix(argv[i], "--maxsolutions="))) {
            int tmp;
            if (sscanf(value, "%d", &tmp) < 0 || tmp < 0) {
                std::cout << "ERROR! wrong maximum number of solutions is illegal: " << tmp << std::endl;
                exit(0);
            }
            max_nr_of_solutions = (uint32_t) tmp;

        } else if ((value = hasPrefix(argv[i], "--pavgbranch"))) {
            conf.doPrintAvgBranch = true;
        } else if ((value = hasPrefix(argv[i], "--greedyunbound"))) {
            conf.greedyUnbound = true;
        } else if ((value = hasPrefix(argv[i], "--nonormxorfind"))) {
            conf.doFindXors = false;
        } else if ((value = hasPrefix(argv[i], "--nobinxorfind"))) {
            conf.doFindEqLits = false;
        } else if ((value = hasPrefix(argv[i], "--noregbxorfind"))) {
            conf.doRegFindEqLits = false;
        } else if ((value = hasPrefix(argv[i], "--doextendedscc"))) {
            conf.doExtendedSCC = true;
        } else if ((value = hasPrefix(argv[i], "--noconglomerate"))) {
            conf.doConglXors = false;
        } else if ((value = hasPrefix(argv[i], "--nosimplify"))) {
            conf.doSchedSimp = false;
        } else if ((value = hasPrefix(argv[i], "--debuglib"))) {
            debugLib = true;
        } else if ((value = hasPrefix(argv[i], "--debugnewvar"))) {
            debugNewVar = true;
        } else if ((value = hasPrefix(argv[i], "--novarreplace"))) {
            conf.doReplace = false;
        } else if ((value = hasPrefix(argv[i], "--nofailedlit"))) {
            conf.doFailedLit = false;
        } else if ((value = hasPrefix(argv[i], "--nodisablegauss"))) {
            gaussconfig.dontDisable = true;
        } else if ((value = hasPrefix(argv[i], "--maxnummatrixes="))) {
            int maxNumMatrixes;
            if (sscanf(value, "%d", &maxNumMatrixes) < 0) {
                printf("ERROR! maxnummatrixes: %s\n", value);
                exit(0);
            }
            gaussconfig.maxNumMatrixes = maxNumMatrixes;
        } else if ((value = hasPrefix(argv[i], "--noheuleprocess"))) {
            conf.doHeuleProcess = false;
        } else if ((value = hasPrefix(argv[i], "--nosatelite"))) {
            conf.doSatELite = false;
        } else if ((value = hasPrefix(argv[i], "--noxorsubs"))) {
            conf.doXorSubsumption = false;
        } else if ((value = hasPrefix(argv[i], "--nohyperbinres"))) {
            conf.doHyperBinRes = false;
        } else if ((value = hasPrefix(argv[i], "--novarelim"))) {
            conf.doVarElim = false;
        } else if ((value = hasPrefix(argv[i], "--nosubsume1"))) {
            conf.doSubsume1 = false;
        } else if ((value = hasPrefix(argv[i], "--nomatrixfind"))) {
            gaussconfig.noMatrixFind = true;
        } else if ((value = hasPrefix(argv[i], "--noiterreduce"))) {
            gaussconfig.iterativeReduce = false;
        } else if ((value = hasPrefix(argv[i], "--noordercol"))) {
            gaussconfig.orderCols = false;
        } else if ((value = hasPrefix(argv[i], "--maxmatrixrows="))) {
            int rows;
            if (sscanf(value, "%d", &rows) < 0 || rows < 0) {
                printf("ERROR! maxmatrixrows: %s\n", value);
                exit(0);
            }
            gaussconfig.maxMatrixRows = (uint32_t) rows;
        } else if ((value = hasPrefix(argv[i], "--minmatrixrows="))) {
            int rows;
            if (sscanf(value, "%d", &rows) < 0 || rows < 0) {
                printf("ERROR! minmatrixrows: %s\n", value);
                exit(0);
            }
            gaussconfig.minMatrixRows = rows;
        } else if ((value = hasPrefix(argv[i], "--savematrix"))) {
            int every;
            if (sscanf(value, "%d", &every) < 0) {
                printf("ERROR! savematrix: %s\n", value);
                exit(0);
            }
            gaussconfig.only_nth_gauss_save = every;
        } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "-help") == 0 || strcmp(argv[i], "--help") == 0) {
            printUsage(argv);
            exit(0);
        } else if ((value = hasPrefix(argv[i], "--restart="))) {
            if (strcmp(value, "auto") == 0)
                conf.fixRestartType = auto_restart;
            else if (strcmp(value, "static") == 0)
                conf.fixRestartType = static_restart;
            else if (strcmp(value, "dynamic") == 0)
                conf.fixRestartType = dynamic_restart;
            else {
                printf("ERROR! unknown restart type %s\n", value);
                exit(0);
            }
        } else if ((value = hasPrefix(argv[i], "--nosolprint"))) {
            printResult = false;
            //} else if ((value = hasPrefix(argv[i], "--addoldlearnts"))) {
            //    conf.readdOldLearnts = true;
        } else if ((value = hasPrefix(argv[i], "--nohyperbinres"))) {
            conf.doHyperBinRes = false;
        } else if ((value = hasPrefix(argv[i], "--noremovebins"))) {
            conf.doRemUselessBins = false;
        } else if ((value = hasPrefix(argv[i], "--nosubswithnbins"))) {
            conf.doSubsWNonExistBins = false;
        } else if ((value = hasPrefix(argv[i], "--nosubswithbins"))) {
            conf.doSubsWBins = false;
        } else if ((value = hasPrefix(argv[i], "--noclausevivif"))) {
            conf.doClausVivif = false;
        } else if ((value = hasPrefix(argv[i], "--nosortwatched"))) {
            conf.doSortWatched = false;
        } else if ((value = hasPrefix(argv[i], "--nolfminim"))) {
            conf.doMinimLearntMore = false;
        } else if ((value = hasPrefix(argv[i], "--nocalcreach"))) {
            conf.doCalcReach = false;
        } else if ((value = hasPrefix(argv[i], "--norecotfssr"))) {
            conf.doMinimLMoreRecur = false;
        } else if ((value = hasPrefix(argv[i], "--nocacheotfssr"))) {
            conf.doCacheOTFSSRSet = false;
            conf.doCacheOTFSSR = false;
        } else if ((value = hasPrefix(argv[i], "--nootfsubsume"))) {
            conf.doOTFSubsume = false;
        } else if ((value = hasPrefix(argv[i], "--noremlbins"))) {
            conf.doRemUselessLBins = false;
        } else if ((value = hasPrefix(argv[i], "--maxconfl="))) {
            int maxconfl = 0;
            if (sscanf(value, "%d", &maxconfl) < 0 || maxconfl < 2) {
                printf("ERROR! max confl: %s\n", value);
                exit(-1);
            }
            conf.maxConfl = maxconfl;
        } else if ((value = hasPrefix(argv[i], "--maxTotalTime="))) {
            int maxtime = 0;
            if (sscanf(value, "%d", &maxtime) < 0 || maxtime < 2) {
                printf("ERROR! max time is too small: %s\n", value);
                exit(-1);
            }
            conf.totalTimeout = maxtime;
        } else if ((value = hasPrefix(argv[i], "--maxLoopTime="))) {
            int maxtime = 0;
            if (sscanf(value, "%d", &maxtime) < 0 || maxtime < 2) {
                printf("ERROR! max time is too small: %s\n", value);
                exit(-1);
            }
            conf.loopTimeout = maxtime;
        }else if ((value = hasPrefix(argv[i], "--switchoffsubs="))) {
            long vars = 0;
            if (sscanf(value, "%ld", &vars) < 0 || vars < 2) {
                printf("ERROR! max time is too small: %s\n", value);
                exit(-1);
            }
            conf.switch_off_subsumer_max_vars = vars;
        } else if ((value = hasPrefix(argv[i], "--plain"))) {
            conf.isPlain = true;
            conf.doOTFSubsume = false;
            conf.doFindXors = false;
            conf.doFindEqLits = false;
            conf.doRegFindEqLits = false;
            conf.doExtendedSCC = false;
            conf.doConglXors = false;
            conf.doSchedSimp = false;
            conf.doReplace = false;
            conf.doFailedLit = false;
            conf.doHeuleProcess = false;
            conf.doSatELite = false;
            conf.doXorSubsumption = false;
            conf.doVarElim = false;
            //nomatrixfind
            gaussconfig.orderCols = false;
            gaussconfig.iterativeReduce = false;
            conf.doHyperBinRes = false;
            conf.doRemUselessBins = false;
            conf.doRemUselessLBins = false;
            conf.doSubsWBins = false;
            conf.doSubsWNonExistBins = false;
            conf.doClausVivif = false;
            conf.doCalcReach = false;
            conf.doBXor = false;
            conf.doMinimLMoreRecur = false;
            conf.doMinimLearntMore = false;
            conf.doCacheOTFSSR = false;
        } else if ((value = hasPrefix(argv[i], "--nobxor"))) {
            conf.doBXor = false;
#ifdef ENABLE_UNWIND_GLUE
        } else if ((value = hasPrefix(argv[i], "--maxglue="))) {
            int glue = 0;
            if (sscanf(value, "%d", &glue) < 0 || glue < 2) {
                printf("ERROR! maxGlue: %s\n", value);
                exit(0);
            }
            if (glue >= (1 << MAX_GLUE_BITS) - 1) {
                std::cout << "Due to memory-packing limitations, max glue cannot be more than "
                        << ((1 << MAX_GLUE_BITS) - 2) << std::endl;
                exit(-1);
            }
            conf.maxGlue = (uint32_t) glue;
        } else if ((value = hasPrefix(argv[i], "--maxgluedel"))) {
            conf.doMaxGlueDel = true;
#endif //ENABLE_UNWIND_GLUE
        } else if ((value = hasPrefix(argv[i], "--threads="))) {
            numThreads = 0;
            if (sscanf(value, "%d", &numThreads) < 0 || numThreads < 1) {
                printf("ERROR! numThreads: %s\n", value);
                exit(0);
            }
        } else if (strncmp(argv[i], "-", 1) == 0 || strncmp(argv[i], "--", 2) == 0) {
            printf("ERROR! unknown flag %s\n", argv[i]);
            exit(0);
        } else {
            //std::std::cout << "argc:" << argc << " i:" << i << ", value:" << argv[i] << std::endl;
            unparsedOptions++;
            if (unparsedOptions == 2) {
                if (!(argc <= i + 2)) {
                    std::cout << "You must give the input file as either:" << std::endl;
                    std::cout << " -- last option if you want the output to the console" << std::endl;
                    std::cout << " -- or one before the last option" << std::endl;
                    std::cout << "It appears that you did neither. Maybe you forgot the '--' from an option?" << std::endl;
                    exit(-1);
                }
                fileNamePresent = true;
                if (argc == i + 2) needTwoFileNames = true;
            }
            if (unparsedOptions == 3) {
                if (!(argc <= i + 1)) {
                    std::cout << "You must give the output file as the last option. Exiting" << std::endl;
                    exit(-1);
                }
                twoFileNamesPresent = true;
            }
            if (unparsedOptions == 4) {
                std::cout << "You gave more than two filenames as parameters." << std::endl;
                std::cout << "The first one is interpreted as the input, the second is the output." << std::endl;
                std::cout << "However, the third one I cannot do anything with. EXITING" << std::endl;
                exit(-1);
            }
        }
    }
    if (conf.verbosity >= 1) {
        if (twoFileNamesPresent) {
            std::cout << "c Outputting solution to file: " << argv[argc - 1] << std::endl;
        } else {
            std::cout << "c Outputting solution to console" << std::endl;
        }
    }

    if (unparsedOptions == 2 && needTwoFileNames == true) {
        std::cout << "Command line wrong. You probably frogot to add " << std::endl
                << "the '--'  in front of one of the options, or you started" << std::endl
                << "your output file with a hyphen ('-'). Exiting." << std::endl;
        exit(-1);
    }
    if (!debugLib) conf.libraryUsage = false;
}

FILE* Main::openOutputFile() {
    FILE* res = NULL;
    if (twoFileNamesPresent) {
        char* filename = argv[argc - 1];
        res = fopen(filename, "wb");
        if (res == NULL) {
            int backup_errno = errno;
            printf("Cannot open %s for writing. Problem: %s", filename, strerror(backup_errno));
            exit(1);
        }
    }

    return res;
}

bool Main::openLogFile(vector<FILE*> *res) {
    if (!conf.shouldLog) {
        return false;
    }
     string suffix, logFileName;
    for (int i = 0; i<numThreads;i++){
      suffix = "_";
      suffix.append(std::to_string(i).append(".txt"));
      logFileName = conf.logFilename;
      (*res)[i] = fopen(logFileName.append(suffix).c_str(), "wb");
      if ((*res)[i] == NULL) {
        int backup_errno = errno;
        printf("Cannot open %s for writing. Problem: %s\n", logFileName.append(suffix).c_str(), strerror(backup_errno));
        exit(1);
      }
    }
    return true;
}

void Main::setDoublePrecision(const uint32_t verbosity) {
#if defined(_FPU_EXTENDED) && defined(_FPU_DOUBLE)
    fpu_control_t oldcw, newcw;
    #pragma omp critical
    {
        _FPU_GETCW(oldcw);
        newcw = (oldcw & ~_FPU_EXTENDED) | _FPU_DOUBLE;
        _FPU_SETCW(newcw);
    }

    if (verbosity >= 1) {
        printf("c WARNING: for repeatability, setting FPU to use double precision\n");
    }
#endif
}

void Main::printVersionInfo(const uint32_t verbosity) {
#pragma omp single
    if (verbosity >= 1) {
        printf("c This is UniGen %s\n", VERSION);
#ifdef __GNUC__
        printf("c compiled with gcc version %s\n", __VERSION__);
#else
        printf("c compiled with non-gcc compiler\n");
#endif
    }
}

int Main::correctReturnValue(const lbool ret) const {
    int retval = -1;
    if (ret == l_True) retval = 10;
    else if (ret == l_False) retval = 20;
    else if (ret == l_Undef) retval = 15;
    else {
        std::cerr << "Something is very wrong, output is neither l_Undef, nor l_False, nor l_True" << std::endl;
        exit(-1);
    }

#ifdef NDEBUG
    // (faster than "return", which will invoke the destructor for 'Solver')
    exit(retval);
#endif
    return retval;
}

std::string binary(int x, uint32_t length) {
    uint32_t logSize = (x == 0 ? 1 : log2(x) + 1);
    std::string s;
    do {
        s.push_back('0' + (x & 1));
    } while (x >>= 1);
    for (uint32_t i = logSize; i < (uint32_t) length; i++) {
        s.push_back('0');
    }
    std::reverse(s.begin(), s.end());

    return s;

}

bool Main::GenerateRandomBits(string &randomBits, uint32_t size, std::mt19937 &randomEngine) {
    std::uniform_int_distribution<int> uid{0, 2147483647};
    uint32_t i = 0;
    while (i < size) {
        i += 31;
        randomBits += binary(uid(randomEngine), 31);
    }
    return true;
}
int Main::GenerateRandomNum(int maxRange, std::mt19937 &randomEngine) {
    std::uniform_int_distribution<int> uid{0, maxRange};
    return uid(randomEngine);
}
/* Number of solutions to return from one invocation of UniGen2 */
uint32_t Main::SolutionsToReturn(uint32_t maxSolutions, uint32_t minSolutions, unsigned long currentSolutions) {
    if(conf.multisample)
        return minSolutions;
    else
        return 1;
}
bool Main::AddHash(uint32_t numClaus, Solver& solver, vec<Lit> &assumptions, std::mt19937 &randomEngine) {
    string randomBits;
    GenerateRandomBits(randomBits, (solver.independentSet.size() + 1) * numClaus, randomEngine);
    bool xorEqualFalse = false;
    Var activationVar;
    vec<Lit> lits;

    for (uint32_t i = 0; i < numClaus; i++) {
        lits.clear();
        activationVar = solver.newVar();
        assumptions.push(Lit(activationVar, true));
        lits.push(Lit(activationVar, false));
        xorEqualFalse = (randomBits[(solver.independentSet.size() + 1) * i] == '1');

        for (uint32_t j = 0; j < solver.independentSet.size(); j++) {

            if (randomBits[(solver.independentSet.size() + 1) * i + j +1 ] == '1') {
                lits.push(Lit(solver.independentSet[j], true));
            }
        }
        solver.addXorClause(lits, xorEqualFalse);
    }
    return true;
}

void Main::printResultFunc(Solver &S, vec<lbool> solutionModel, const lbool ret, FILE* res) {
    if (res != NULL && printResult) {
        if (ret == l_True) {
            fprintf(res, "v ");
            for (Var var = 0; var != S.nOrigVars(); var++)
                if (solutionModel[var] != l_Undef)
                    fprintf(res, "%s%d ", (S.model[var] == l_True) ? "" : "-", var + 1);
            fprintf(res, "0\n");
            fflush(res);
        }
    } else {

        if (ret == l_True && printResult) {
            std::stringstream toPrint;
            toPrint << "v ";
            for (Var var = 0; var != S.nOrigVars(); var++)
                if (solutionModel[var] != l_Undef)
                    toPrint << ((solutionModel[var] == l_True) ? "" : "-") << var + 1 << " ";
            toPrint << "0" << std::endl;
            std::cout << toPrint.str();
        }
    }
}

int32_t Main::BoundedSATCount(uint32_t maxSolutions, Solver &solver, vec<Lit> &assumptions) {
    unsigned long current_nr_of_solutions = 0;
    lbool ret = l_True;
    Var activationVar = solver.newVar();
    vec<Lit> allSATAssumptions;
    if (!assumptions.empty()) {
        assumptions.copyTo(allSATAssumptions);
    }
    allSATAssumptions.push(Lit(activationVar, true));
    //signal(SIGALRM, SIGALARM_handler);
     start_timer(conf.loopTimeout);
    while (current_nr_of_solutions < maxSolutions && ret == l_True) {

        ret = solver.solve(allSATAssumptions);
        if (ret == l_True && current_nr_of_solutions < maxSolutions) {
            current_nr_of_solutions++;
            vec<Lit> lits;
            lits.push(Lit(activationVar, false));
            for (uint32_t j = 0; j < solver.independentSet.size(); j++) {
                Var var = solver.independentSet[j];
                if (solver.model[var] != l_Undef) {
                    lits.push(Lit(var, (solver.model[var] == l_True) ? true : false));
                }
            }
            solver.addClause(lits);
        }
    }
    vec<Lit> cls_that_removes;
    cls_that_removes.push(Lit(activationVar, false));
    solver.addClause(cls_that_removes);
    if (ret == l_Undef){
        solver.needToInterrupt = false;
        return -1*current_nr_of_solutions - 1;
    }
    return current_nr_of_solutions;
}

lbool Main::BoundedSAT(uint32_t maxSolutions, uint32_t minSolutions, Solver &solver, vec<Lit> &assumptions, std::mt19937 &randomEngine, std::map<std::string, uint32_t> &solutionMap, uint32_t *solutionCount) {
    unsigned long current_nr_of_solutions = 0;
    lbool ret = l_True;
    Var activationVar = solver.newVar();
    vec<Lit> allSATAssumptions;
    if (!assumptions.empty()) {
        assumptions.copyTo(allSATAssumptions);
    }
    allSATAssumptions.push(Lit(activationVar, true));

    std::vector<vec<lbool>> modelsSet;
    vec<lbool> model;
    //signal(SIGALRM, SIGALARM_handler);
    start_timer(conf.loopTimeout);
    while (current_nr_of_solutions < maxSolutions && ret == l_True) {
        ret = solver.solve(allSATAssumptions);
        current_nr_of_solutions++;
 
        if (ret == l_True && current_nr_of_solutions < maxSolutions) {
            vec<Lit> lits;
            lits.push(Lit(activationVar, false));
            model.clear();
            solver.model.copyTo(model);
            modelsSet.push_back(model);
            for (uint32_t j = 0; j < solver.independentSet.size(); j++) {
                Var var = solver.independentSet[j];
                if (solver.model[var] != l_Undef) {
                    lits.push(Lit(var, (solver.model[var] == l_True) ? true : false));
                }
            }
            solver.addClause(lits);
        }
    }
    *solutionCount = modelsSet.size();
    //std::cout<<current_nr_of_solutions<<std::endl;
    vec<Lit> cls_that_removes;
    cls_that_removes.push(Lit(activationVar, false));
    solver.addClause(cls_that_removes);
    if (ret == l_Undef){
        solver.needToInterrupt = false;
        
        return ret;
    }


    if (current_nr_of_solutions < maxSolutions && current_nr_of_solutions > minSolutions) {
        std::vector<int> modelIndices;
        for (uint32_t i = 0; i < modelsSet.size(); i++)
            modelIndices.push_back(i);
        std::shuffle(modelIndices.begin(), modelIndices.end(), randomEngine);
        Var var;
        uint32_t numSolutionsToReturn = SolutionsToReturn(maxSolutions, minSolutions, modelsSet.size());
        for (uint32_t i = 0; i < numSolutionsToReturn; i++) {
            vec<lbool> model = modelsSet.at(modelIndices.at(i));
            string solution ("v");
            for (uint32_t j = 0; j < solver.independentSet.size(); j++) {
                var = solver.independentSet[j];
                if (model[var] != l_Undef) {
                    if (model[var] != l_True) {
                        solution += "-";
                    }
                    solution += std::to_string(var+1);   
                    solution += " ";
                }
            }
            solution += "0";

            map<std::string, uint32_t>::iterator it = solutionMap.find(solution);
            if (it == solutionMap.end()) {
                solutionMap[solution] = 0; 
            }
            solutionMap[solution] += 1;
        }
        return l_True;
    
    }

    return l_False;
}

SATCount Main::ApproxMC(Solver &solver, vector<FILE *> *resLog, std::mt19937 &randomEngine) {
    int32_t currentNumSolutions = 0;
    uint32_t  hashCount;
    std::list<int> numHashList, numCountList;
    vec<Lit> assumptions;
    SATCount solCount;
    solCount.cellSolCount = 0;
    solCount.hashCount = 0;
    double elapsedTime = 0;
    int repeatTry = 0;
    for (uint32_t j = 0; j < conf.tApproxMC; j++) {
        for (hashCount = 0; hashCount < solver.nVars(); hashCount++) {
            double currentTime = totalTime();
            elapsedTime = currentTime-startTime;
            if (elapsedTime > conf.totalTimeout - 3000){
                break;
            }
            double myTime = totalTime();
            currentNumSolutions = BoundedSATCount(conf.pivotApproxMC + 1, solver, assumptions);

            myTime = totalTime() - myTime;
            //printf("%f\n", myTime);
            //printf("%d %d\n",currentNumSolutions,conf.pivotApproxMC);
            if (conf.shouldLog) {
                fprintf((*resLog)[0], "ApproxMC:%d:%d:%f:%d:%d\n", j, hashCount, myTime,
                    (currentNumSolutions == (int32_t)(conf.pivotApproxMC + 1)),currentNumSolutions);
                fflush((*resLog)[0]);
            }
            if (currentNumSolutions < 0){
                assumptions.clear();
                if (repeatTry < 2){     /* Retry up to twice more */
                    AddHash(hashCount,solver,assumptions,randomEngine);
                    hashCount --;
                    repeatTry += 1;
                }else{
                    AddHash(hashCount+1,solver,assumptions,randomEngine);
                    repeatTry = 0;
                }
                continue;
            }
            if (currentNumSolutions == conf.pivotApproxMC + 1) {
                AddHash(1,solver,assumptions,randomEngine);
            } else {
                break;
            }

        }
        assumptions.clear();
        if (elapsedTime > conf.totalTimeout - 3000){
            break;
        }
        numHashList.push_back(hashCount);
        numCountList.push_back(currentNumSolutions);
    }
    if (numHashList.size() == 0){
        return solCount;
    }
    int minHash = findMin(numHashList);
    for (std::list<int>::iterator it1 = numHashList.begin(), it2 = numCountList.begin();
        it1 != numHashList.end() && it2 != numCountList.end(); it1++, it2++) {
        (*it2) *= pow(2, (*it1) - minHash);
    }
    int medSolCount = findMedian(numCountList);
    solCount.cellSolCount = medSolCount;
    solCount.hashCount = minHash;
    return solCount;
}

/*
 * Returns the number of samples generated 
 */
uint32_t Main::UniGen(uint32_t samples, Solver &solver,
        FILE* res, vector<FILE* > *resLog, uint32_t sampleCounter, std::mt19937 &randomEngine, std::map<std::string, uint32_t> &solutionMap, uint32_t *lastSuccessfulHashOffset, double timeReference) {
    lbool ret = l_False;
    uint32_t i, solutionCount, currentHashCount, lastHashCount, currentHashOffset, hashOffsets[3];
    int hashDelta;
    vec<Lit> assumptions;
    double elapsedTime =0;
    #if defined(_OPENMP)
    int threadNum = omp_get_thread_num();
    #else
    int threadNum = 0;
    #endif
    int repeatTry = 0;
    for (i = 0; i < samples; i++) {
        sampleCounter ++;
        ret = l_False;

        hashOffsets[0] = *lastSuccessfulHashOffset;   /* Start at last successful hash offset */
        if(hashOffsets[0] == 0)     /* Starting at q-2; go to q-1 then q */
        {
            hashOffsets[1] = 1;
            hashOffsets[2] = 2;
        }
        else if(hashOffsets[0] == 2)    /* Starting at q; go to q-1 then q-2 */
        {
            hashOffsets[1] = 1;
            hashOffsets[2] = 0;
        }
        repeatTry = 0;
        lastHashCount = 0;
        for(uint32_t j = 0; j < 3; j++) {
            currentHashOffset = hashOffsets[j];
            currentHashCount = currentHashOffset + conf.startIteration;
            hashDelta = currentHashCount - lastHashCount;

            if(hashDelta > 0)   /* Add new hash functions */
                AddHash(hashDelta, solver, assumptions, randomEngine);
            else if(hashDelta < 0)    /* Remove hash functions */
            {
                assumptions.clear();
                AddHash(currentHashCount, solver, assumptions, randomEngine);
            }
            lastHashCount = currentHashCount;

            double currentTime = totalTime(); 
            elapsedTime = currentTime-startTime;
            if (elapsedTime > conf.totalTimeout - 3000){
                break;
            }
            uint32_t maxSolutions = (uint32_t) (1.41*(1+conf.kappa)*conf.pivotUniGen +2);
            uint32_t minSolutions = (uint32_t) (conf.pivotUniGen/(1.41*(1+conf.kappa)));
            ret = BoundedSAT(maxSolutions + 1, minSolutions, solver, assumptions, randomEngine, solutionMap, &solutionCount);
            if (conf.shouldLog) {
                fprintf((*resLog)[threadNum], "UniGen2:%d:%d:%f:%d:%d\n", sampleCounter, currentHashCount, totalTime() - timeReference, (ret == l_False ? 1 : (ret == l_True ? 0 : 2)), solutionCount);
                fflush((*resLog)[threadNum]);
            }
            if (ret == l_Undef)     /* Solver timed out; retry current hash count at most twice more */
            {
                assumptions.clear();    /* Throw out old hash functions */
                if (repeatTry < 2){     /* Retry current hash count with new hash functions */
                    AddHash(currentHashCount, solver, assumptions, randomEngine);
                    j--;
                    repeatTry += 1;
                }else{      /* Go on to next hash count */
                    lastHashCount = 0;
                    if((j == 0) && (currentHashOffset == 1))    /* At q-1, and need to pick next hash count */
                    {
                        /* Somewhat arbitrarily pick q-2 first; then q */
                        hashOffsets[1] = 0;
                        hashOffsets[2] = 2;
                    }
                    repeatTry = 0;
                }
                continue;
            }
            if (ret == l_True)      /* Number of solutions in correct range */
            {
                *lastSuccessfulHashOffset = currentHashOffset;
                break;
            }
            else    /* Number of solutions too small or too large */
            {
                if((j == 0) && (currentHashOffset == 1))  /* At q-1, and need to pick next hash count */
                {
                    if(solutionCount < minSolutions)
                    {
                        /* Go to q-2; next will be q */
                        hashOffsets[1] = 0;
                        hashOffsets[2] = 2;
                    }
                    else
                    {
                        /* Go to q; next will be q-2 */
                        hashOffsets[1] = 2;
                        hashOffsets[2] = 0;
                    }
                }
            }
        }
        if (ret != l_True){
            i --;
        }
        assumptions.clear();
        if (elapsedTime > conf.totalTimeout - 3000){
            break;
        }
    }
    return sampleCounter;
}

int Main::singleThreadUniGenCall(uint32_t samples, FILE* res, vector<FILE*> *resLog, uint32_t sampleCounter, std::map<std::string, uint32_t> &solutionMap, std::mt19937 &randomEngine, uint32_t *lastSuccessfulHashOffset, double timeReference) {
    Solver solver2(conf, gaussconfig);
    //solversToInterrupt[0] = &solver2;
    //need_clean_exit = true;

    int num;
    #if defined(_OPENMP)
    num = omp_get_thread_num();
    #else
    num = 0;
    #endif

    #pragma omp critical (solversToInterr)
    {
      //printf("%d\n",num);
      solversToInterrupt[num] = &solver2;
    }
    //SeedEngine(randomEngine);

    setDoublePrecision(conf.verbosity);
    parseInAllFiles(solver2);
    sampleCounter = UniGen(samples, solver2, res, resLog, sampleCounter, randomEngine, solutionMap, lastSuccessfulHashOffset, timeReference);
    return sampleCounter;
}

void Main::SeedEngine(std::mt19937 &randomEngine)
{
    /* Initialize PRNG with seed from random_device */
    std::random_device rd{};
    std::array<int, 10> seedArray;
    std::generate_n(seedArray.data(), seedArray.size(), std::ref(rd));
    std::seed_seq seed(std::begin(seedArray), std::end(seedArray));
    randomEngine.seed(seed);
}

bool Main::printSolutions(FILE* res){
    int i;
    for (map< std::string, std::vector<uint32_t>>:: iterator it = globalSolutionMap.begin();
                                    it != globalSolutionMap.end(); it++)
    {
        std::vector<uint32_t> counts = it->second;
        if(conf.aggregateSolutions)
        {
            uint32_t totalCount = 0;
            for(i = 0; i < numThreads; i++)
                totalCount += counts[i];
            fprintf(res, "%s:%d\n ", it->first.c_str(), totalCount);
        }
        else
        {
            fprintf(res, "%s:", it->first.c_str());
            for(i = 0; i < numThreads-1; i++)
                fprintf(res, "%d,", counts[i]);
            fprintf(res, "%d\n ", counts[i]);
        }
    }
            
    fflush(res);
     
    return true;
}
int Main::singleThreadSolve() {
    /* Determine the number of sampling threads to use */
    #if defined(_OPENMP)
    if(numThreads > 0)      /* Number has been specified by the user */
        omp_set_num_threads(numThreads);
    else
    {
        /* Using system default number of threads (or env variable OMP_NUM_THREADS) */
        #pragma omp parallel
        {
            if(omp_get_thread_num() == 0)
                numThreads = omp_get_num_threads();
        }
    }
    #else
    numThreads = 1;
    #endif

    startTime = totalTime();
    mytimer = new timer_t[numThreads];
    timerSetFirstTime = new bool[numThreads];
    struct sigaction sa;
    sa.sa_flags = SA_SIGINFO;
    sa.sa_sigaction = SIGALARM_handler;
    sigemptyset(&sa.sa_mask);
    sigaction(SIGUSR1, &sa, NULL);
    for (int i = 0; i< numThreads; i++){
      timerSetFirstTime[i] = true;
    }
    Solver solver(conf, gaussconfig);
    solversToInterrupt.clear();
    solversToInterrupt[0] = &solver;
    need_clean_exit = true;
    printVersionInfo(conf.verbosity);
    setDoublePrecision(conf.verbosity);
    parseInAllFiles(solver);
    FILE* res = openOutputFile();
    vector<FILE*> *resLog = new vector<FILE*>(numThreads,NULL);
    openLogFile(resLog);
    lbool ret = l_True;
    if (conf.startIteration > solver.independentSet.size()){
        printf("Manually-specified startIteration is larger than the size of the independent set.\n");
        return 0;
    }
    if (conf.startIteration == 0){
        printf("Computing startIteration using ApproxMC\n");

        SATCount solCount;
        std::mt19937 randomEngine{};
        SeedEngine(randomEngine);
        solCount = ApproxMC(solver, resLog, randomEngine);
        double elapsedTime = totalTime() - startTime;
        printf("Completed ApproxMC at %f s", elapsedTime);
        if (elapsedTime > conf.totalTimeout - 3000){
            printf(" (TIMED OUT)\n");
            return 0;
        }
        printf("\n");
        //printf("Solution count estimate is %d * 2^%d\n", solCount.cellSolCount, solCount.hashCount);
        if (solCount.hashCount == 0 && solCount.cellSolCount == 0){
            printf("The input formula is unsatisfiable.");
            return 0;
        }
        conf.startIteration = round(solCount.hashCount + log2(solCount.cellSolCount) + 
            log2(1.8) - log2(conf.pivotUniGen))-2;
        if (conf.startIteration < 0){
           printf("The number of solutions is too small. The best technique is just to enumerate and sample one.\n");
           return 0;
          //conf.startIteration = 0;
        }
    }
    else
        ;//printf("Using manually-specified startIteration\n");
    
    solversToInterrupt.clear();
    uint32_t maxSolutions = (uint32_t) (1.41*(1+conf.kappa)*conf.pivotUniGen +2);
    uint32_t minSolutions = (uint32_t) (conf.pivotUniGen/(1.41*(1+conf.kappa)));
    uint32_t samplesPerCall = SolutionsToReturn(maxSolutions+1, minSolutions, minSolutions);
    uint32_t callsNeeded = (conf.samples + samplesPerCall - 1) / samplesPerCall;
    printf("loThresh %d, hiThresh %d, startIteration %d\n", minSolutions, maxSolutions, conf.startIteration);
    //printf("Outputting %d solutions from each UniGen2 call\n", samplesPerCall);
    uint32_t numCallsInOneLoop = 0;
    if(conf.callsPerSolver == 0)
    {
        numCallsInOneLoop = std::min(solver.nVars()/(1+conf.startIteration*14), callsNeeded/numThreads);
        if (numCallsInOneLoop == 0){
            numCallsInOneLoop = 1;
        }
    }
    else
    {
        numCallsInOneLoop = conf.callsPerSolver;
        printf("Using manually-specified callsPerSolver\n");
    }

    uint32_t numCallLoops = callsNeeded / numCallsInOneLoop;
    uint32_t remainingCalls = callsNeeded % numCallsInOneLoop;
    //printf("Making %d loops, %d calls per loop, %d remaining\n", numCallLoops, numCallsInOneLoop, remainingCalls);
    bool timedOut;
    uint32_t sampleCounter = 0;
    std::map<std::string, uint32_t> threadSolutionMap;
    double allThreadsTime = 0;
    uint32_t allThreadsSampleCount = 0;
    printf("Launching %d sampling thread(s)\n", numThreads);
    #pragma omp parallel private(timedOut) firstprivate(threadSolutionMap,sampleCounter)
    {
        int threadNum = 0;
        #if defined(_OPENMP)
        threadNum = omp_get_thread_num();
        //printf("hello from thread %d\n", threadNum);
        #else
        //printf("not using OpenMP\n");
        #endif

        timedOut = false;
        //sampleCounter = 0;

        double threadStartTime = totalTime();

        std::mt19937 randomEngine{};
        SeedEngine(randomEngine);

        uint32_t lastSuccessfulHashOffset = 0;

        /* Perform extra UniGen calls that don't fit into the loops */
        #pragma omp single nowait
        {
            if(remainingCalls > 0)
                sampleCounter = singleThreadUniGenCall(remainingCalls,res,resLog,sampleCounter,threadSolutionMap,randomEngine,&lastSuccessfulHashOffset,threadStartTime);
        }

        /* Perform main UniGen call loops */
        #pragma omp for schedule(dynamic) nowait
        for (uint32_t i = 0;i<numCallLoops;i++)
        {
            if (!timedOut)
            {
                sampleCounter = singleThreadUniGenCall(numCallsInOneLoop,res,resLog,sampleCounter,threadSolutionMap,randomEngine,&lastSuccessfulHashOffset,threadStartTime);
                if ((totalTime() - threadStartTime) > conf.totalTimeout - 3000)
                    timedOut = true;
            }
        }

        /* Aggregate thread-specific solution counts */
        #pragma omp critical
        {
            for (map<std::string, uint32_t>::iterator itt = threadSolutionMap.begin();
                itt != threadSolutionMap.end(); itt++)
            {
                std::string solution = itt->first;
                map<std::string, std::vector<uint32_t>>::iterator itg = globalSolutionMap.find(solution);
                if (itg == globalSolutionMap.end()) {
                    globalSolutionMap[solution] = std::vector<uint32_t>(numThreads, 0); 
                }
                globalSolutionMap[solution][threadNum] += itt->second;
                allThreadsSampleCount += itt->second;
            }

            double timeTaken = totalTime() - threadStartTime;
            allThreadsTime += timeTaken;
            printf("Total time for UniGen2 thread %d: %f s", threadNum, timeTaken);
            if(timedOut)
                printf(" (TIMED OUT)");
            printf("\n");
        }
    }
    if (printResult)
        printSolutions(res);

    printf("Total time for all UniGen2 calls: %f s\n", allThreadsTime);
    printf("Samples generated: %d\n", allThreadsSampleCount);

    if (conf.needToDumpOrig) {
        if (ret != l_False) {
            solver.addAllXorAsNorm();
        }
        if (ret == l_False && conf.origFilename == "stdout") {
            std::cout << "p cnf 0 1" << std::endl;
            std::cout << "0";
        } else if (ret == l_True && conf.origFilename == "stdout") {
            std::cout << "p cnf " << solver.model.size() << " " << solver.model.size() << std::endl;
            for (uint32_t i = 0; i < solver.model.size(); i++) {
                std::cout << (solver.model[i] == l_True ? "" : "-") << i + 1 << " 0" << std::endl;
            }
        } else {
            if (!solver.dumpOrigClauses(conf.origFilename)) {
                std::cout << "Error: Cannot open file '" << conf.origFilename << "' to write learnt clauses!" << std::endl;
                exit(-1);
            }
            if (conf.verbosity >= 1)
                std::cout << "c Simplified original clauses dumped to file '"
                    << conf.origFilename << "'" << std::endl;
        }
    }
    if (ret == l_Undef && conf.verbosity >= 1) {
        std::cout << "c Not finished running -- signal caught or maximum restart reached" << std::endl;
    }
    if (conf.verbosity >= 1) solver.printStats();

    // printResultFunc(solver, ret, res, current_nr_of_solutions == 1);

    return correctReturnValue(ret);
}

/**
@brief For correctly and gracefully exiting

It can happen that the user requests a dump of the learnt clauses. In this case,
the program must wait until it gets to a state where the learnt clauses are in
a correct state, then dump these and quit normally. This interrupt hander
is used to achieve this
 */

int main(int argc, char** argv) {
    Main main(argc, argv);
    main.parseCommandLine();
    signal(SIGINT, SIGINT_handler);
    //signal(SIGALRM, SIGALARM_handler);
    try{
        return main.singleThreadSolve();

    }

    catch(std::bad_alloc) {
        std::cerr << "Memory manager cannot handle the load. Sorry. Exiting." << std::endl;
        exit(-1);
    }

    catch(std::out_of_range oor) {
        std::cerr << oor.what() << std::endl;
        exit(-1);
    }

    catch(CMSat::DimacsParseError dpe) {
        std::cerr << "PARSE ERROR!" << dpe.what() << std::endl;
        exit(3);
    }
    return 0;
}
