/*++
Copyright (c) 2011 Microsoft Corporation

Module Name:

    dimacs.cpp

Abstract:

    Dimacs CNF parser

Author:

    Leonardo de Moura (leonardo) 2011-07-26.

Revision History:

--*/
#include<vector>
#include<unordered_set>
#include "sat/dimacs.h"
#undef max
#undef min
#include "sat/sat_solver.h"

std::vector<int> indsup;
std::unordered_set<int> indset;
bool has_ind = false;

class stream_buffer {
    std::istream & m_stream;
    int            m_val;
    unsigned       m_line;
public:
    
    stream_buffer(std::istream & s):
        m_stream(s),
        m_line(0) {
        m_val = m_stream.get();
    }

    int  operator *() const { 
        return m_val;
    }

    void operator ++() { 
        m_val = m_stream.get();
        if (m_val == '\n') ++m_line;
    }

    unsigned line() const { return m_line; }
};

template<typename Buffer>
void skip_whitespace(Buffer & in) {
    while ((*in >= 9 && *in <= 13) || *in == 32) {
        ++in; 
    }
}

template<typename Buffer>
void skip_line(Buffer & in) {
    while(true) {
        if (*in == EOF) {
            return;
        }
        if (*in == '\n') { 
            ++in; 
            return; 
        }
        ++in; 
    } 
}

template<typename Buffer>
int parse_int(Buffer & in) {
    int     val = 0;
    bool    neg = false;
    skip_whitespace(in);

    if (*in == '-') {
        neg = true;
        ++in;
    }
    else if (*in == '+') {
        ++in;
    }

    if (*in < '0' || *in > '9') {
        std::cerr << "(error, \"unexpected char: " << *in << " line: " << in.line() << "\")\n";
        exit(3);
        exit(ERR_PARSER);
    }

    while (*in >= '0' && *in <= '9') {
        val = val*10 + (*in - '0');
        ++in;
    }

    return neg ? -val : val; 
}

template<typename Buffer>
void read_clause(Buffer & in, sat::solver & solver, sat::literal_vector & lits) {
    int     parsed_lit;
    int     var;
    
    lits.reset();

    while (true) { 
        parsed_lit = parse_int(in);
        if (parsed_lit == 0)
            break;
        var = abs(parsed_lit);
        SASSERT(var > 0);
        while (static_cast<unsigned>(var) >= solver.num_vars())
            solver.mk_var();
        lits.push_back(sat::literal(var, parsed_lit < 0));
        if (!has_ind) {
            indset.insert(var);
        }
    }
}

template<typename Buffer>
void parse_dimacs_core(Buffer & in, sat::solver & solver) {
    sat::literal_vector lits;
    while (true) {
        skip_whitespace(in);
        if (*in == EOF) {
            break;
        }
        else if (*in == 'p') {
            skip_line(in);
        }
        else if (*in == 'c') {
            bool is_ind = true;
            char ind[] = " ind";
            for (int index = 0; index < 4; ++ index) {
                ++in;
                if (*in != ind[index]) {
                    is_ind = false;
                    break;
                }
            }
            if (!is_ind) {
                skip_line(in);
                continue;
            }
            ++in;
            int parsed_lit;
            while (true) {
                parsed_lit = parse_int(in);
                if (parsed_lit == 0)
                    break;
                if (indset.find(parsed_lit) == indset.end()) {
                    indset.insert(parsed_lit);
                    indsup.push_back(parsed_lit);
                    has_ind = true;
                }
            }
        }
        else {
            read_clause(in, solver, lits);
            solver.mk_clause(lits.size(), lits.c_ptr());
        }
    }
    if (!has_ind) {
        for (int lit = 0; lit < solver.num_vars(); ++lit) {
            if (indset.find(lit) != indset.end()) {
                indsup.push_back(lit);
            }
        }
    }
    indset.clear();
}

void parse_dimacs(std::istream & in, sat::solver & solver) {
    stream_buffer _in(in);
    parse_dimacs_core(_in, solver);
}
