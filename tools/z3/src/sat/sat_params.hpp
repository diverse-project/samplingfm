// Automatically generated file
#ifndef __SAT_PARAMS_HPP_
#define __SAT_PARAMS_HPP_
#include "util/params.h"
#include "util/gparams.h"
struct sat_params {
  params_ref const & p;
  params_ref g;
  sat_params(params_ref const & _p = params_ref::get_empty()):
     p(_p), g(gparams::get_module("sat")) {}
  static void collect_param_descrs(param_descrs & d) {
    d.insert("max_memory", CPK_UINT, "maximum amount of memory in megabytes", "4294967295","sat");
    d.insert("phase", CPK_SYMBOL, "phase selection strategy: always_false, always_true, caching, random", "caching","sat");
    d.insert("phase.caching.on", CPK_UINT, "phase caching on period (in number of conflicts)", "400","sat");
    d.insert("phase.caching.off", CPK_UINT, "phase caching off period (in number of conflicts)", "100","sat");
    d.insert("phase.sticky", CPK_BOOL, "use sticky phase caching for local search", "false","sat");
    d.insert("propagate.prefetch", CPK_BOOL, "prefetch watch lists for assigned literals", "true","sat");
    d.insert("restart", CPK_SYMBOL, "restart strategy: static, luby, ema or geometric", "ema","sat");
    d.insert("restart.initial", CPK_UINT, "initial restart (number of conflicts)", "2","sat");
    d.insert("restart.max", CPK_UINT, "maximal number of restarts.", "4294967295","sat");
    d.insert("restart.fast", CPK_BOOL, "use fast restart approach only removing less active literals.", "true","sat");
    d.insert("restart.factor", CPK_DOUBLE, "restart increment factor for geometric strategy", "1.5","sat");
    d.insert("restart.margin", CPK_DOUBLE, "margin between fast and slow restart factors. For ema", "1.1","sat");
    d.insert("restart.emafastglue", CPK_DOUBLE, "ema alpha factor for fast moving average", "0.03","sat");
    d.insert("restart.emaslowglue", CPK_DOUBLE, "ema alpha factor for slow moving average", "1e-05","sat");
    d.insert("variable_decay", CPK_UINT, "multiplier (divided by 100) for the VSIDS activity increement", "110","sat");
    d.insert("inprocess.max", CPK_UINT, "maximal number of inprocessing passes", "4294967295","sat");
    d.insert("branching.heuristic", CPK_SYMBOL, "branching heuristic vsids, lrb or chb", "vsids","sat");
    d.insert("branching.anti_exploration", CPK_BOOL, "apply anti-exploration heuristic for branch selection", "false","sat");
    d.insert("random_freq", CPK_DOUBLE, "frequency of random case splits", "0.01","sat");
    d.insert("random_seed", CPK_UINT, "random seed", "0","sat");
    d.insert("burst_search", CPK_UINT, "number of conflicts before first global simplification", "100","sat");
    d.insert("max_conflicts", CPK_UINT, "maximum number of conflicts", "4294967295","sat");
    d.insert("gc", CPK_SYMBOL, "garbage collection strategy: psm, glue, glue_psm, dyn_psm", "glue_psm","sat");
    d.insert("gc.initial", CPK_UINT, "learned clauses garbage collection frequency", "20000","sat");
    d.insert("gc.increment", CPK_UINT, "increment to the garbage collection threshold", "500","sat");
    d.insert("gc.small_lbd", CPK_UINT, "learned clauses with small LBD are never deleted (only used in dyn_psm)", "3","sat");
    d.insert("gc.k", CPK_UINT, "learned clauses that are inactive for k gc rounds are permanently deleted (only used in dyn_psm)", "7","sat");
    d.insert("gc.burst", CPK_BOOL, "perform eager garbage collection during initialization", "false","sat");
    d.insert("gc.defrag", CPK_BOOL, "defragment clauses when garbage collecting", "true","sat");
    d.insert("simplify.delay", CPK_UINT, "set initial delay of simplification by a conflict count", "0","sat");
    d.insert("minimize_lemmas", CPK_BOOL, "minimize learned clauses", "true","sat");
    d.insert("dyn_sub_res", CPK_BOOL, "dynamic subsumption resolution for minimizing learned clauses", "true","sat");
    d.insert("core.minimize", CPK_BOOL, "minimize computed core", "false","sat");
    d.insert("core.minimize_partial", CPK_BOOL, "apply partial (cheap) core minimization", "false","sat");
    d.insert("threads", CPK_UINT, "number of parallel threads to use", "1","sat");
    d.insert("dimacs.core", CPK_BOOL, "extract core from DIMACS benchmarks", "false","sat");
    d.insert("drat.file", CPK_SYMBOL, "file to dump DRAT proofs", "","sat");
    d.insert("drat.check_unsat", CPK_BOOL, "build up internal proof and check", "false","sat");
    d.insert("drat.check_sat", CPK_BOOL, "build up internal trace, check satisfying model", "false","sat");
    d.insert("cardinality.solver", CPK_BOOL, "use cardinality solver", "true","sat");
    d.insert("pb.solver", CPK_SYMBOL, "method for handling Pseudo-Boolean constraints: circuit (arithmetical circuit), sorting (sorting circuit), totalizer (use totalizer encoding), solver (use native solver)", "solver","sat");
    d.insert("xor.solver", CPK_BOOL, "use xor solver", "false","sat");
    d.insert("atmost1_encoding", CPK_SYMBOL, "encoding used for at-most-1 constraints grouped, bimander, ordered", "grouped","sat");
    d.insert("local_search", CPK_BOOL, "use local search instead of CDCL", "false","sat");
    d.insert("local_search_threads", CPK_UINT, "number of local search threads to find satisfiable solution", "0","sat");
    d.insert("local_search_mode", CPK_SYMBOL, "local search algorithm, either default wsat or qsat", "wsat","sat");
    d.insert("unit_walk", CPK_BOOL, "use unit-walk search instead of CDCL", "false","sat");
    d.insert("unit_walk_threads", CPK_UINT, "number of unit-walk search threads to find satisfiable solution", "0","sat");
    d.insert("lookahead.cube.cutoff", CPK_SYMBOL, "cutoff type used to create lookahead cubes: depth, freevars, psat, adaptive_freevars, adaptive_psat", "depth","sat");
    d.insert("lookahead.cube.fraction", CPK_DOUBLE, "adaptive fraction to create lookahead cubes. Used when lookahead.cube.cutoff is adaptive_freevars or adaptive_psat", "0.4","sat");
    d.insert("lookahead.cube.depth", CPK_UINT, "cut-off depth to create cubes. Used when lookahead.cube.cutoff is depth.", "1","sat");
    d.insert("lookahead.cube.freevars", CPK_DOUBLE, "cube free fariable fraction. Used when lookahead.cube.cutoff is freevars", "0.8","sat");
    d.insert("lookahead.cube.psat.var_exp", CPK_DOUBLE, "free variable exponent for PSAT cutoff", "1","sat");
    d.insert("lookahead.cube.psat.clause_base", CPK_DOUBLE, "clause base for PSAT cutoff", "2","sat");
    d.insert("lookahead.cube.psat.trigger", CPK_DOUBLE, "trigger value to create lookahead cubes for PSAT cutoff. Used when lookahead.cube.cutoff is psat", "5","sat");
    d.insert("lookahead_search", CPK_BOOL, "use lookahead solver", "false","sat");
    d.insert("lookahead.preselect", CPK_BOOL, "use pre-selection of subset of variables for branching", "false","sat");
    d.insert("lookahead_simplify", CPK_BOOL, "use lookahead solver during simplification", "false","sat");
    d.insert("lookahead.use_learned", CPK_BOOL, "use learned clauses when selecting lookahead literal", "false","sat");
    d.insert("lookahead_simplify.bca", CPK_BOOL, "add learned binary clauses as part of lookahead simplification", "true","sat");
    d.insert("lookahead.global_autarky", CPK_BOOL, "prefer to branch on variables that occur in clauses that are reduced", "false","sat");
    d.insert("lookahead.reward", CPK_SYMBOL, "select lookahead heuristic: ternary, heule_schur (Heule Schur), heuleu (Heule Unit), unit, or march_cu", "march_cu","sat");
    d.insert("quicksampler_check", CPK_BOOL, "check samples generated by QuickSampler", "false","sat");
    d.insert("quicksampler_check.timeout", CPK_DOUBLE, "timeout to check all samples generated by QuickSampler", "3600.0","sat");
  }
  /*
     REG_MODULE_PARAMS('sat', 'sat_params::collect_param_descrs')
     REG_MODULE_DESCRIPTION('sat', 'propositional SAT solver')
  */
  unsigned max_memory() const { return p.get_uint("max_memory", g, 4294967295u); }
  symbol phase() const { return p.get_sym("phase", g, symbol("caching")); }
  unsigned phase_caching_on() const { return p.get_uint("phase.caching.on", g, 400u); }
  unsigned phase_caching_off() const { return p.get_uint("phase.caching.off", g, 100u); }
  bool phase_sticky() const { return p.get_bool("phase.sticky", g, false); }
  bool propagate_prefetch() const { return p.get_bool("propagate.prefetch", g, true); }
  symbol restart() const { return p.get_sym("restart", g, symbol("ema")); }
  unsigned restart_initial() const { return p.get_uint("restart.initial", g, 2u); }
  unsigned restart_max() const { return p.get_uint("restart.max", g, 4294967295u); }
  bool restart_fast() const { return p.get_bool("restart.fast", g, true); }
  double restart_factor() const { return p.get_double("restart.factor", g, 1.5); }
  double restart_margin() const { return p.get_double("restart.margin", g, 1.1); }
  double restart_emafastglue() const { return p.get_double("restart.emafastglue", g, 0.03); }
  double restart_emaslowglue() const { return p.get_double("restart.emaslowglue", g, 1e-05); }
  unsigned variable_decay() const { return p.get_uint("variable_decay", g, 110u); }
  unsigned inprocess_max() const { return p.get_uint("inprocess.max", g, 4294967295u); }
  symbol branching_heuristic() const { return p.get_sym("branching.heuristic", g, symbol("vsids")); }
  bool branching_anti_exploration() const { return p.get_bool("branching.anti_exploration", g, false); }
  double random_freq() const { return p.get_double("random_freq", g, 0.01); }
  unsigned random_seed() const { return p.get_uint("random_seed", g, 0u); }
  unsigned burst_search() const { return p.get_uint("burst_search", g, 100u); }
  unsigned max_conflicts() const { return p.get_uint("max_conflicts", g, 4294967295u); }
  symbol gc() const { return p.get_sym("gc", g, symbol("glue_psm")); }
  unsigned gc_initial() const { return p.get_uint("gc.initial", g, 20000u); }
  unsigned gc_increment() const { return p.get_uint("gc.increment", g, 500u); }
  unsigned gc_small_lbd() const { return p.get_uint("gc.small_lbd", g, 3u); }
  unsigned gc_k() const { return p.get_uint("gc.k", g, 7u); }
  bool gc_burst() const { return p.get_bool("gc.burst", g, false); }
  bool gc_defrag() const { return p.get_bool("gc.defrag", g, true); }
  unsigned simplify_delay() const { return p.get_uint("simplify.delay", g, 0u); }
  bool minimize_lemmas() const { return p.get_bool("minimize_lemmas", g, true); }
  bool dyn_sub_res() const { return p.get_bool("dyn_sub_res", g, true); }
  bool core_minimize() const { return p.get_bool("core.minimize", g, false); }
  bool core_minimize_partial() const { return p.get_bool("core.minimize_partial", g, false); }
  unsigned threads() const { return p.get_uint("threads", g, 1u); }
  bool dimacs_core() const { return p.get_bool("dimacs.core", g, false); }
  symbol drat_file() const { return p.get_sym("drat.file", g, symbol("")); }
  bool drat_check_unsat() const { return p.get_bool("drat.check_unsat", g, false); }
  bool drat_check_sat() const { return p.get_bool("drat.check_sat", g, false); }
  bool cardinality_solver() const { return p.get_bool("cardinality.solver", g, true); }
  symbol pb_solver() const { return p.get_sym("pb.solver", g, symbol("solver")); }
  bool xor_solver() const { return p.get_bool("xor.solver", g, false); }
  symbol atmost1_encoding() const { return p.get_sym("atmost1_encoding", g, symbol("grouped")); }
  bool local_search() const { return p.get_bool("local_search", g, false); }
  unsigned local_search_threads() const { return p.get_uint("local_search_threads", g, 0u); }
  symbol local_search_mode() const { return p.get_sym("local_search_mode", g, symbol("wsat")); }
  bool unit_walk() const { return p.get_bool("unit_walk", g, false); }
  unsigned unit_walk_threads() const { return p.get_uint("unit_walk_threads", g, 0u); }
  symbol lookahead_cube_cutoff() const { return p.get_sym("lookahead.cube.cutoff", g, symbol("depth")); }
  double lookahead_cube_fraction() const { return p.get_double("lookahead.cube.fraction", g, 0.4); }
  unsigned lookahead_cube_depth() const { return p.get_uint("lookahead.cube.depth", g, 1u); }
  double lookahead_cube_freevars() const { return p.get_double("lookahead.cube.freevars", g, 0.8); }
  double lookahead_cube_psat_var_exp() const { return p.get_double("lookahead.cube.psat.var_exp", g, 1); }
  double lookahead_cube_psat_clause_base() const { return p.get_double("lookahead.cube.psat.clause_base", g, 2); }
  double lookahead_cube_psat_trigger() const { return p.get_double("lookahead.cube.psat.trigger", g, 5); }
  bool lookahead_search() const { return p.get_bool("lookahead_search", g, false); }
  bool lookahead_preselect() const { return p.get_bool("lookahead.preselect", g, false); }
  bool lookahead_simplify() const { return p.get_bool("lookahead_simplify", g, false); }
  bool lookahead_use_learned() const { return p.get_bool("lookahead.use_learned", g, false); }
  bool lookahead_simplify_bca() const { return p.get_bool("lookahead_simplify.bca", g, true); }
  bool lookahead_global_autarky() const { return p.get_bool("lookahead.global_autarky", g, false); }
  symbol lookahead_reward() const { return p.get_sym("lookahead.reward", g, symbol("march_cu")); }
  bool quicksampler_check() const { return p.get_bool("quicksampler_check", g, false); }
  double quicksampler_check_timeout() const { return p.get_double("quicksampler_check.timeout", g, 3600.0); }
};
#endif
