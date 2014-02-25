#ifndef APPLY_SCHEDULE_H
#define APPLY_SCHEDULE_H

#include <Halide.h>

typedef std::map<std::string, std::vector<Halide::Internal::Schedule> > schedule_map;

void apply_schedule(const schedule_map &schedules, Halide::Func root);

#endif
