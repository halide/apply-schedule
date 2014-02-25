// TODO: docs

// NOTE: this should be built with -fno-rtti to be sure it links successfully
// with the corresponding types (IRVisitor, etc.) in libHalide.a as it is
// usually compiled.

#include "ApplySchedule.h"

#include <cstdio>

using std::map;
using std::vector;
using std::string;
using namespace Halide;
using namespace Halide::Internal;

struct FindRecursiveCalls : public IRDeepVisitor {
    using IRDeepVisitor::visit;
    map<string, Function> result() { return funcs; }
};

/** Construct a map from name to Function definition object for all Halide 
 *  functions called directly in the definition of the Function f, or 
 *  indirectly in those functions' definitions, recursively.
 */
map<string, Function> find_recursive_calls(Function f) {
    FindRecursiveCalls all_calls;
    visit_function(&all_calls, f);
    return all_calls.result();
}

struct FindCalls : public IRVisitor {
    map<string, Function> funcs;
    using IRVisitor::visit;
    map<string, Function> result() { return funcs; }
    void visit(const Call *call) {
        IRVisitor::visit(call);
        if (call->call_type == Call::Halide) {
            funcs[call->func.name()] = call->func;
        }
    }
};

/** Construct a map from name to Function definition object for all Halide 
 *  functions called directly in the definition of the Function f, including
 *  in update definitions, update index expressions, and RDom extents.
 */
map<string, Function> find_direct_calls(Function f) {
    FindCalls calls;
    visit_function(&calls, f);
    return calls.result();
}

void apply_schedule(const schedule_map &schedules, Func root) {
    // TODO: this should be encapsulated in a find_all_calls helper
    // extract all the functions called transitively from root, by name
    Function f = root.function();
    map<string, Function> functions = find_recursive_calls(f);

    // add the root function into the environment, too
    functions[f.name()] = f;

    // for each function named in the schedule_map, apply the schedule to the
    // Function object by overwriting its schedule field by reference.
    for (schedule_map::const_iterator it = schedules.begin();
         it != schedules.end(); ++it)
    {
        fprintf(stderr, "Apply schedule to %s\n", it->first.c_str());
        assert(functions.count(it->first));
        functions[it->first].schedule() = it->second[0];
        for (size_t r = 0; r < functions[it->first].reductions().size(); r++) {
            functions[it->first].reduction_schedule(r) = it->second[r+1];
        }
    }
}
