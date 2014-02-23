// TODO: docs

// NOTE: this should be built with -fno-rtti to be sure it links successfully
// with the corresponding types (IRVisitor, etc.) in libHalide.a as it is
// usually compiled.

#include "ApplySchedule.h"

#include <map>
#include <cstdio>

using std::map;
using std::vector;
using std::string;
using namespace Halide;
using namespace Halide::Internal;

// TODO: factor this into a common utility library
/* Find all the internal halide calls in an expr */
class FindAllCalls : public IRVisitor {
private:
    bool recursive;
public:
    FindAllCalls(bool recurse = false) : recursive(recurse) {}

    map<string, Function> calls;

    typedef map<string, Function>::iterator iterator;

    using IRVisitor::visit;

    void include_function(Function f) {
        iterator iter = calls.find(f.name());
        if (iter == calls.end()) {
            calls[f.name()] = f;
            if (recursive) {
                // recursively add everything called in the definition of f
                for (size_t i = 0; i < f.values().size(); i++) {
                    f.values()[i].accept(this);
                }
                // recursively add everything called in the definition of f's update step
                for (size_t i = 0; i < f.reductions().size(); i++) {
                    for (size_t j = 0; j < f.reductions()[i].values.size(); j++) {
                        f.reductions()[i].values[j].accept(this);
                    }
                }
            }
        } else {
            assert(iter->second.same_as(f) &&
                   "Can't compile a pipeline using multiple functions with same name");
        }
    }

    void visit(const Call *call) {
        IRVisitor::visit(call);
        if (call->call_type == Call::Halide) {
            include_function(call->func);
        }
    }

    void dump_calls(FILE *of) {
        iterator it = calls.begin();
        while (it != calls.end()) {
            fprintf(of, "\"%s\"", it->first.c_str());
            ++it;
            if (it != calls.end()) {
                fprintf(of, ", ");
            }
        }
    }
};

void apply_schedule(const schedule_map &schedules, Func root) {
    // TODO: this should be encapsulated in a find_all_calls helper
    // extract all the functions called transitively from root, by name
    Function f = root.function();
    FindAllCalls all_calls(true);
    for (size_t i = 0; i < f.values().size(); i++) {
        f.values()[i].accept(&all_calls);
    }
    map<string, Function> functions = all_calls.calls;

    // add the root function into the environment, too
    functions[f.name()] = f;

    // for each function named in the schedule_map, apply the schedule to the
    // Function object by overwriting its schedule field by reference.
    for (schedule_map::const_iterator it = schedules.begin();
         it != schedules.end(); ++it)
    {
        fprintf(stderr, "Apply schedule to %s\n", it->first.c_str());
        assert(functions.count(it->first));
        functions[it->first].schedule() = it->second;
    }
}