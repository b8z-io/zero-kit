# Headless runs are deterministic

In a Headless run the time step is fixed at 1/fps (no sleeping, no wall clock), every input Module is replaced by its Fake, and keys can be scripted (`--keys 1.0:S,2.5:V`). The same run always produces the same Pictures, so CI can compare them with stored ones.

This is why every input Module must ship a Fake from the start, and why nothing in a Module may read the wall clock or random state in a Headless run.
