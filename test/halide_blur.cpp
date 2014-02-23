#include <Halide.h>
#include "../ApplySchedule.h"
using namespace Halide;

int main(int argc, char **argv) {
    
    schedule_map scheds;
    {
        // Manually build a schedule - easy for compute_level, store_level:
        Internal::Schedule s;
        s.compute_level.func = "blur_y";
        s.compute_level.var = "y";
        s.store_level = s.compute_level;

        // Much messier for other stuff:
        s.storage_dims.push_back("x");
        s.storage_dims.push_back("y");

        Internal::Schedule::Dim dim;
        dim.var = "x";
        // dim.for_type = Internal::For::Vectorized;
        dim.for_type = Internal::For::Serial;
        s.dims.push_back(dim);

        dim.var = "y";
        dim.for_type = Internal::For::Serial;
        s.dims.push_back(dim);

        scheds["blur_x"] = s;
    }

    {
        // Fastest way to build split, storage, dims lists:
        Func f; Var x("x"), y("y"), xi("xi"), yi("yi");
        f(x,y) = 0;

        f.tile(x, y, xi, yi, 256, 32).parallel(y).vectorize(xi);

        // But this is awkward for compute and store levels, since these
        // require Func handles which always trigger unique_name on construction
        // e.g.,
        // 
        //  Func f("blur_y");
        //  g.compute_at(f, x); // x can be shadowed, blur_y can't

        scheds["blur_y"] = f.function().schedule();
    }

    ImageParam input(UInt(16), 2, "input");
    Func blur_x("blur_x"), blur_y("blur_y");
    Var x("x"), y("y"), xi("xi"), yi("yi");
    
    // The algorithm
    blur_x(x, y) = (input(x, y) + input(x+1, y) + input(x+2, y))/3;
    blur_y(x, y) = (blur_x(x, y) + blur_x(x, y+1) + blur_x(x, y+2))/3;
    
    // dump_call_graph("halide_blur.calls.json", blur_y);
    apply_schedule(scheds, blur_y);
    blur_y.compile_to_file("halide_blur", input);

    return 0;
}
