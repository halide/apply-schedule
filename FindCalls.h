#ifndef FIND_CALLS_H
#define FIND_CALLS_H

#include <Halide.h>
#include <map>

/** Construct a map from name to Function definition object for all Halide 
 *  functions called directly in the definition of the Function f, including
 *  in update definitions, update index expressions, and RDom extents.
 */
std::map<std::string, Halide::Internal::Function> find_direct_calls(Halide::Internal::Function f);

/** Construct a map from name to Function definition object for all Halide 
 *  functions called directly in the definition of the Function f, or 
 *  indirectly in those functions' definitions, recursively.
 */
std::map<std::string, Halide::Internal::Function> find_recursive_calls(Halide::Internal::Function f);

#endif
