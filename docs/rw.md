# Random Wrangler (A8)
Random generator where you can define the probability of specific intervals from -5v to 5v.

Knobs `[A-I]` sets the probability of the module returning a value from the corresponding interval.

The probability values are relative, meaning that if e.g. knobs `[A-H]` are set to 0, but knob `I` is set to 0.5, then there is still a 100% chance that the random values will be from that interval - however in linear mode, the probability of each possible value from that specific interval will be distributed linearly from 0 to 100% across the interval.

The context menu (right click the module) allows you to enable `Constant distribution` rather than linear piecewise distribution.  In this mode all possible values in a specific interval have equal possibility of being returned. In this mode knob `I` has no function.

CV input available for each interval knob.

`RATE` sets the update rate of the output. Goes from slow LFO to audio rate. CV input available (1v/oct).

If `TRIG` is connected the internal clock is disabled and values only update upon a trigger signal.

`SMOOTH` sets the rate at which the module interpolates between the previous value and the new value. If set to 0 the module acts as a sample and hold module.

`SHAPE` defines the shape of the interpolation. Choose between linear (`LIN`) and exponential (`EXP`).



![Default theme](/module-screenshots/default/RW.png?raw=true)
![River bed theme](/module-screenshots/river-bed/RW.png?raw=true)
![Shark theme](/module-screenshots/shark/RW.png?raw=true)
![Oxford blue theme](/module-screenshots/oxford-blue/RW.png?raw=true)
![Cod gray theme](/module-screenshots/cod-gray/RW.png?raw=true)
![Firefly theme](/module-screenshots/firefly/RW.png?raw=true)
