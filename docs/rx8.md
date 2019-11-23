# Random Mix x8 (RX8)
Randomly mix up to 8 inputs into one output. The randomness is based on simplex noise.

When set to `ON TRIG` the mix changes when a trigger signal is received on `TRIG`. When set to `FREE` the mix changes continously.

`SPEED` changes the rate of change in the mix.

`JITTER` introduces micro bumps in the change. Can e.g. be used to introduce a bit of local variance at slow speeds - if you want to avoid it to be too smooth and predictable.

`PINNING` sets the amount that the mix value is pinned to either fully muted or fully open. The higher pinning the more of the time a channel will be either completely muted or fully passed through rather than somewhere in the middle. High pinning values also speeds up the rate at which the change moves from one point to another.

![Default theme](https://github.com/thomassidor/tinytricks/blob/master/module-screenshots/default/RX8.png?raw=true)
![River bed theme](https://github.com/thomassidor/tinytricks/blob/master/module-screenshots/river-bed/RX8.png?raw=true)
