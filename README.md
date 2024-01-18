## WORK IN PROGRESS

This is an experiment to evaluate tech stacks for a modern "NEngine", specifically as a c++ project. NEngine is an 
interactive simulation engine specialized in hyperspace generalization (for now, then comes non-euclidianess and, 
the special sauces ^_^).

The conclusion was that c++ although a great language, and the current industry standard for interactive simulations, 
it is definetily on the dust now in a wide range of features, and its cleary being replaced by rust and zig in the 
appropiate parts, and rust specially is gaining an unprecedent adoption in some hard industry grounds.

Given this and the project specific constraints (huge codebase, many combinatorial features
, and a specially repetitive need for generalized metaprogramming on N, etc) made it difficult to choose one over the
other, but rust had to be choosen, as it is the most adopted, stable, and adecuate to handle the unprecedent, and 
inherent complexity. Zig would be ideal, comptime absence is a big pain on rust, but the tradeoffs wherent enough,
specially for the potential size of the system. 

Anyways, the language is not the most important, but the abstract types we can derive to create a real generalized 
solution to interactive simulations on exotic geometries. When (if) the promised language of the future comes, this 
will be happily rewriten on it, and its part of this codebase ethos. For now, rust is almost perfect to this, and zig 
can be included where it shows (if appropiately needed), and who knows, its should not be that difficult to 
add comptime to rust (sigh).

### This is part of the old README

Im rewritting an old n-d web engine into modern soil (?) c++, just for the sake 
of it (?).

The "primary task":
nd engine: graphics/physics 
graphics: correct n-d rendering (volumetric rendering && superposition), vol-RTX
physics: base solids, constraints

EXTRA: after the bare minimum of the primary task, im switching to rust.
C++ 
NOTES: c++ modules lack support
