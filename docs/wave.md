# Wavetable Oscillator (WAVE)
A wavetable style oscillator - with three seperate oscillators - where you can (and need to) capture your own waves.

## Creating the wavetable
To capture a wavetable connect audio to `WAVE TOP`, `MIDDLE` and `BOTTOM` and press `CAPTURE`. This will record the incoming audio to the respectively the top, middle and bottom positions in the wavetable. Intermediate values are automatically interpolated.

Connect `SYNC` from your oscillator to sync the recording to one single cycle. If not connected 2048 samples will be recorded.

`MIRROR` mirrors the recorded wave at the halfway point.

## Main oscillator
`FREQ` and `FINE` determines the coarse and fine tuning of the output. CV for `FREQ` is 1 volt per octave.

`POS` determines the vertical position in the wavetable.

## Oscillator 2 and Oscillator 3
These are seperate oscillators using the same wavetable that can be turned on and off individually using `ENABLE`.

`DETUNE` offsets the pitch reletive to the main oscillator.

`POS` offsets the vertical position in the wavetable relative to the main oscillator.

`SYNC` enables oscillator hard sync agains the chosen oscillator. To hear the effect detune the oscillator.

## Tips
* You can turn off the scope via the modules context menu (right click) to improve UI performance.
* The wavetable is saved as part of your save file, so you can pass it on to others easily.


![Default theme](https://github.com/thomassidor/tinytricks/blob/master/module-screenshots/default/WAVE.png?raw=true)
![River bed theme](https://github.com/thomassidor/tinytricks/blob/master/module-screenshots/river-bed/WAVE.png?raw=true)
![Shark theme](https://github.com/thomassidor/tinytricks/blob/master/module-screenshots/shark/WAVE.png?raw=true)
![Oxford blue theme](https://github.com/thomassidor/tinytricks/blob/master/module-screenshots/oxford-blue/WAVE.png?raw=true)
![Cod gray theme](https://github.com/thomassidor/tinytricks/blob/master/module-screenshots/cod-gray/WAVE.png?raw=true)
![Firefly theme](https://github.com/thomassidor/tinytricks/blob/master/module-screenshots/firefly/WAVE.png?raw=true)



