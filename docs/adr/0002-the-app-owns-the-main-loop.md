# The App owns main() and the loop; Zero Kit provides frame begin/end

Zero Kit does not run the App through callbacks. The App writes its own `main()` and `while` loop and calls Zero Kit at the edges: one init call that parses the shared flags, a frame-begin that hands back the time step and key events, and a frame-end that delivers the Picture and HUD to the Outputs, paces the loop, and handles Headless runs.

A callback framework was rejected because Apps have flows that don't fit one loop shape (dome-zero waits for a key before it starts; ndi-zero will block on receive). The boilerplate the framework would have removed (about 150 lines per App) is removed by the begin/end pair anyway.
