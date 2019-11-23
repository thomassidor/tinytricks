# Simplex Noise (SN1 + SN8)
Generates a smooth bipolar value based on simplex noise. Available in versions with 1 or 8 independent outputs.

`SPEED` changes the rate of change the random values.

`JITTER` introduces micro bumps in the random values. Can e.g. be used to introduce a bit of local variance at slow speeds - if you want to avoid it to be too smooth and predictable.

`PINNING` sets the amount of time the value is pinned to either +5 or -5. The higher pinning the more of the time a channel will be either at minimum or maximum rather than somewhere in the middle. High pinning values also speeds up the rate at which the change moves from maximum to minimum.

![Default theme](https://github.com/thomassidor/tinytricks/blob/master/module-screenshots/default/SN1.png?raw=true)
![River bed theme](https://github.com/thomassidor/tinytricks/blob/master/module-screenshots/river-bed/SN1.png?raw=true)

![Default theme](https://github.com/thomassidor/tinytricks/blob/master/module-screenshots/default/SN8.png?raw=true)
![River bed theme](https://github.com/thomassidor/tinytricks/blob/master/module-screenshots/river-bed/SN8.png?raw=true)

