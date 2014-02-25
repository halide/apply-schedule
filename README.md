# Building
`ApplySchedule.[cpp,h]` can be directly imported into an outside project to provide their functionality. `ApplySchedule.cpp` should usually be compiled with `-fno-rtti` to be sure it links successfully with the corresponding types (`IRVisitor`, etc.) in `libHalide.a` as Halide is usually compiled by its default Mac/Linux Makefile.

**NOTE:** this is still work in progress. It may have significant bugs.