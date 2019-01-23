/***************************************************************************************[Solver.cc]
Copyright (c) 2003-2006, Niklas Een, Niklas Sorensson
Copyright (c) 2007-2009, Niklas Sorensson
Copyright (c) 2009-2012, Mate Soos

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

#ifndef MAIN_H
#define MAIN_H

#include<iostream>
#include <sstream>
#include<random>
#include <string>
#include <vector>
#include <map>
#ifndef DISABLE_ZLIB
#include <zlib.h>
#endif // DISABLE_ZLIB
#include <chrono>
#include <ctime>
#include "cmsat/Solver.h"
#include "cmsat/SharedData.h"
#include "cmsat/DimacsParser.h"
namespace CMSat {

    using std::string;

    struct SATCount {
        uint32_t hashCount;
        uint32_t cellSolCount;
    };

    class Main {
    public:
        Main(int argc, char** argv);

        void parseCommandLine();

        int singleThreadSolve();
        int oneThreadSolve();
        int multiThreadSolve();

        int numThreads;

    private:

        void printUsage(char** argv);
        const char* hasPrefix(const char* str, const char* prefix);
        void printResultFunc(const Solver& S, const lbool ret, FILE* res, const bool firstSolution);

        //File reading
        void readInAFile(const std::string& filename, Solver& solver);
        void readInStandardInput(Solver& solver);
        void parseInAllFiles(Solver& solver);
        FILE* openOutputFile();
        bool openLogFile(vector<FILE*> *resLog);

        int singleThreadUniGenCall(uint32_t samples, FILE* res, vector<FILE*> *resLog, uint32_t sampleCounter, std::map<std::string, uint32_t> &solutionMap, std::mt19937 &randomEngine, uint32_t *lastSuccessfulHashOffset, double timeReference);
        void setDoublePrecision(const uint32_t verbosity);
        void printVersionInfo(const uint32_t verbosity);
        int correctReturnValue(const lbool ret) const;

        SolverConf conf;
        GaussConf gaussconfig;

        bool grouping;
        bool debugLib;
        bool debugNewVar;
        bool printResult;
        uint32_t max_nr_of_solutions;
        bool fileNamePresent;
        bool twoFileNamesPresent;
        std::vector<std::string> filesToRead;

        SharedData sharedData;

        int argc;
        char** argv;
        SATCount ApproxMC(Solver &solver, vector<FILE*> *resLog, std::mt19937 &randomEngine);
        uint32_t UniGen(uint32_t samples, Solver &solver,
                FILE* res, vector<FILE*> *resLog, uint32_t sampleCounter, std::mt19937 &randomEngine, std::map<std::string, uint32_t> &solutionMap, uint32_t *lastSuccessfulHashOffset, double timeReference);
        bool AddHash(uint32_t clausNum, Solver &s, vec<Lit> &assumptions, std::mt19937 &randomEngine);
        int32_t BoundedSATCount(uint32_t maxSolutions, Solver &solver, vec<Lit> &assumptions);
        lbool BoundedSAT(uint32_t maxSolutions, uint32_t minSolutions, Solver &solver, vec<Lit> &assumptions, std::mt19937 &randomEngine, std::map<std::string, uint32_t> &solutionMap, uint32_t *solutionCount);
        bool GenerateRandomBits(string &randomBits, uint32_t size, std::mt19937 &randomEngine);
        uint32_t SolutionsToReturn(uint32_t maxSolutions, uint32_t minSolutions, unsigned long currentSolutions);
        int GenerateRandomNum(int maxRange, std::mt19937 &randomEngine);
        void printResultFunc(Solver &S, vec<lbool> solutionModel, const lbool ret, FILE* res);
        bool printSolutions(FILE* res);
        void SeedEngine(std::mt19937 &randomEngine);

        time_t  startTime;
        std::map< std::string, std::vector<uint32_t>> globalSolutionMap;
    };

}

#endif //MAIN_H
