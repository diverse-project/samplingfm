// Automatically generated file.
#include "util/gparams.h"
#include "ackermannization/ackermannization_params.hpp"
#include "ackermannization/ackermannize_bv_tactic_params.hpp"
#include "ast/fpa/fpa2bv_rewriter_params.hpp"
#include "ast/normal_forms/nnf.h"
#include "ast/normal_forms/nnf_params.hpp"
#include "ast/pattern/pattern_inference_params_helper.hpp"
#include "ast/pp_params.hpp"
#include "ast/rewriter/arith_rewriter_params.hpp"
#include "ast/rewriter/array_rewriter_params.hpp"
#include "ast/rewriter/bool_rewriter_params.hpp"
#include "ast/rewriter/bv_rewriter_params.hpp"
#include "ast/rewriter/fpa_rewriter_params.hpp"
#include "ast/rewriter/poly_rewriter_params.hpp"
#include "ast/rewriter/rewriter_params.hpp"
#include "cmd_context/context_params.h"
#include "math/polynomial/algebraic_params.hpp"
#include "math/realclosure/rcf_params.hpp"
#include "model/model_evaluator_params.hpp"
#include "model/model_params.hpp"
#include "muz/base/fixedpoint_params.hpp"
#include "nlsat/nlsat_params.hpp"
#include "opt/opt_params.hpp"
#include "parsers/util/parser_params.hpp"
#include "sat/sat_asymm_branch_params.hpp"
#include "sat/sat_params.hpp"
#include "sat/sat_scc_params.hpp"
#include "sat/sat_simplifier_params.hpp"
#include "smt/params/smt_params_helper.hpp"
#include "solver/combined_solver_params.hpp"
#include "solver/parallel_params.hpp"
#include "tactic/sls/sls_params.hpp"
#include "tactic/smtlogics/qfufbv_tactic_params.hpp"
#include "util/env_params.h"
#include "util/lp/lp_params.hpp"
void gparams_register_modules() {
{ param_descrs d; context_params::collect_param_descrs(d); gparams::register_global(d); }
{ param_descrs d; env_params::collect_param_descrs(d); gparams::register_global(d); }
{ param_descrs * d = alloc(param_descrs); ackermannization_params::collect_param_descrs(*d); gparams::register_module("ackermannization", d); }
{ param_descrs * d = alloc(param_descrs); ackermannize_bv_tactic_params::collect_param_descrs(*d); gparams::register_module("rewriter", d); }
{ param_descrs * d = alloc(param_descrs); fpa2bv_rewriter_params::collect_param_descrs(*d); gparams::register_module("rewriter", d); }
{ param_descrs * d = alloc(param_descrs); nnf::get_param_descrs(*d); gparams::register_module("nnf", d); }
{ param_descrs * d = alloc(param_descrs); nnf_params::collect_param_descrs(*d); gparams::register_module("nnf", d); }
{ param_descrs * d = alloc(param_descrs); pattern_inference_params_helper::collect_param_descrs(*d); gparams::register_module("pi", d); }
{ param_descrs * d = alloc(param_descrs); pp_params::collect_param_descrs(*d); gparams::register_module("pp", d); }
{ param_descrs * d = alloc(param_descrs); arith_rewriter_params::collect_param_descrs(*d); gparams::register_module("rewriter", d); }
{ param_descrs * d = alloc(param_descrs); array_rewriter_params::collect_param_descrs(*d); gparams::register_module("rewriter", d); }
{ param_descrs * d = alloc(param_descrs); bool_rewriter_params::collect_param_descrs(*d); gparams::register_module("rewriter", d); }
{ param_descrs * d = alloc(param_descrs); bv_rewriter_params::collect_param_descrs(*d); gparams::register_module("rewriter", d); }
{ param_descrs * d = alloc(param_descrs); fpa_rewriter_params::collect_param_descrs(*d); gparams::register_module("rewriter", d); }
{ param_descrs * d = alloc(param_descrs); poly_rewriter_params::collect_param_descrs(*d); gparams::register_module("rewriter", d); }
{ param_descrs * d = alloc(param_descrs); rewriter_params::collect_param_descrs(*d); gparams::register_module("rewriter", d); }
{ param_descrs * d = alloc(param_descrs); algebraic_params::collect_param_descrs(*d); gparams::register_module("algebraic", d); }
{ param_descrs * d = alloc(param_descrs); rcf_params::collect_param_descrs(*d); gparams::register_module("rcf", d); }
{ param_descrs * d = alloc(param_descrs); model_evaluator_params::collect_param_descrs(*d); gparams::register_module("model_evaluator", d); }
{ param_descrs * d = alloc(param_descrs); model_params::collect_param_descrs(*d); gparams::register_module("model", d); }
{ param_descrs * d = alloc(param_descrs); fixedpoint_params::collect_param_descrs(*d); gparams::register_module("fixedpoint", d); }
{ param_descrs * d = alloc(param_descrs); nlsat_params::collect_param_descrs(*d); gparams::register_module("nlsat", d); }
{ param_descrs * d = alloc(param_descrs); opt_params::collect_param_descrs(*d); gparams::register_module("opt", d); }
{ param_descrs * d = alloc(param_descrs); parser_params::collect_param_descrs(*d); gparams::register_module("parser", d); }
{ param_descrs * d = alloc(param_descrs); sat_asymm_branch_params::collect_param_descrs(*d); gparams::register_module("sat", d); }
{ param_descrs * d = alloc(param_descrs); sat_params::collect_param_descrs(*d); gparams::register_module("sat", d); }
{ param_descrs * d = alloc(param_descrs); sat_scc_params::collect_param_descrs(*d); gparams::register_module("sat", d); }
{ param_descrs * d = alloc(param_descrs); sat_simplifier_params::collect_param_descrs(*d); gparams::register_module("sat", d); }
{ param_descrs * d = alloc(param_descrs); smt_params_helper::collect_param_descrs(*d); gparams::register_module("smt", d); }
{ param_descrs * d = alloc(param_descrs); combined_solver_params::collect_param_descrs(*d); gparams::register_module("combined_solver", d); }
{ param_descrs * d = alloc(param_descrs); parallel_params::collect_param_descrs(*d); gparams::register_module("parallel", d); }
{ param_descrs * d = alloc(param_descrs); sls_params::collect_param_descrs(*d); gparams::register_module("sls", d); }
{ param_descrs * d = alloc(param_descrs); qfufbv_tactic_params::collect_param_descrs(*d); gparams::register_module("ackermannization", d); }
{ param_descrs * d = alloc(param_descrs); lp_params::collect_param_descrs(*d); gparams::register_module("lp", d); }
gparams::register_module_descr("ackermannization", "solving UF via ackermannization");
gparams::register_module_descr("nnf", "negation normal form");
gparams::register_module_descr("pi", "pattern inference (heuristics) for universal formulas (without annotation)");
gparams::register_module_descr("pp", "pretty printer");
gparams::register_module_descr("rewriter", "new formula simplification module used in the tactic framework, and new solvers");
gparams::register_module_descr("algebraic", "real algebraic number package");
gparams::register_module_descr("rcf", "real closed fields");
gparams::register_module_descr("fixedpoint", "fixedpoint parameters");
gparams::register_module_descr("nlsat", "nonlinear solver");
gparams::register_module_descr("opt", "optimization parameters");
gparams::register_module_descr("sat", "propositional SAT solver");
gparams::register_module_descr("smt", "smt solver based on lazy smt");
gparams::register_module_descr("combined_solver", "combines two solvers: non-incremental (solver1) and incremental (solver2)");
gparams::register_module_descr("parallel", "parameters for parallel solver");
gparams::register_module_descr("sls", "Experimental Stochastic Local Search Solver (for QFBV only).");
gparams::register_module_descr("ackermannization", "tactics based on solving UF-theories via ackermannization (see also ackr module)");
}
