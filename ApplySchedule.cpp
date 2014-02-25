// TODO: docs

// NOTE: this should be built with -fno-rtti to be sure it links successfully
// with the corresponding types (IRVisitor, etc.) in libHalide.a as it is
// usually compiled.

#include "ApplySchedule.h"
#include "FindCalls.h"
#include <cstdio>

using std::map;
using std::string;
using namespace Halide;
using namespace Halide::Internal;

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
