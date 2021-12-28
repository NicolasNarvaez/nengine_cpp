## WORK IN PROGRESS

This is a personal work, with self-referencing notes, beware of code-delirium.

Im rewritting an old n-d web engine into modern soil (?) c++, just for the sake 
of it (?).

First i will not be searching speed, just to make an engine core flexible enough
for the "primary task", the fns written will vary in incompleteness and 
whishfullness. An FP approach and "ontology" based solutions, should keep it 
modular enough to add optimizations later (and rewritte a ton of this).

The "primary task":
nd engine: graphics/physics 
graphics: correct n-d rendering (volumetric rendering && superposition), vol-RTX
physics: base solids, constraints

The "primary requirements":
multi-platform nativeness, gpu agnostic, bug rtx derived preferred.
wasm first and rtx first (the closest way always)
ND first (optimizations could reach the hot-reload, its ok), including shadding

There will be no more "tasks" or "issues" whatsoever, until the primary work is 
done.

(I recognize there is much to learn, for the rest of life, so please bear with 
my ignorance.)

