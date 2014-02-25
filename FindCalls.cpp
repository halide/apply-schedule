// NOTE: this should be built with -fno-rtti to be sure it links successfully
// with the corresponding types (IRVisitor, etc.) in libHalide.a as it is
// usually compiled.
#include "FindCalls.h"

using std::map;
using std::vector;
using std::string;
using namespace Halide;
using namespace Halide::Internal;

struct FindRecursiveCalls : public IRDeepVisitor {
    using IRDeepVisitor::visit;
    map<string, Function> result() { return funcs; }
};

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

map<string, Function> find_direct_calls(Function f) {
    FindCalls calls;
    visit_function(&calls, f);
    return calls.result();
}
