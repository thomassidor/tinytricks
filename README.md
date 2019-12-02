![The current modules](/combined-screenshots/default.png?raw=true)

# Tiny Tricks VCV Rack modules

Tiny Tricks is a collection of (more or less) tiny modules for VCV Rack.

Questions, comments, enhancement suggestions, ideas for modules and everything else: [Create a ticket.](https://github.com/thomassidor/tinytricks/issues/new)

All the  best, Thomas René Sidor

### Utility modules:
* Tiny Tricks Arithmetic (TT-A)
* Tiny Tricks Logic (TT-L)
* [Sample and Hold x16 (SH16)](/docs/sh16.md)
* [Attenuator x8 (A8)](/docs/a8.md)

### Modulation sources, random generators, LFOs:
* [Modulation Generator (MG1)](/docs/mg.md)
* [Modulation Generator x8 (MG8)](/docs/mg.md)
* [Modulation Generator x16 (MG16)](/docs/mg.md)
* [Simplex Noise (SN1)](/docs/sn.md)
* [Simplex Noise x8 (SN8)](/docs/sn.md)
* [Random Wrangler (RW)](/docs/rw.md)

### Mixing and muting:
* [Random Mute x8 (RM8)](/docs/rm8.md)
* [Random Stereo Mute x8 (RM8s)](/docs/rm8.md)
* [Random Mix x8 (RX8)](/docs/rx8.md)

### Simple oscillators
* [Simple Sine Oscillator (SIN)](/docs/simple.md)
* [Simple Sawtooth Oscillator (SAW)](/docs/simple.md)
* [Simple Square Oscillator (SQR)](/docs/simple.md)
* [Simple Triangle Oscillator (TRI)](/docs/simple.md)

### Less simple oscillators
* [Sine+ Oscillator (SIN+)](/docs/plus.md)
* [Sawtooth+ Oscillator (SAW+)](/docs/plus.md)
* [Square+ Oscillator (SQR+)](/docs/plus.md)
* [Triangle+ Oscillator (TRI+)](/docs/plus.md)
* Simplex Oscillator (SN-OSC)
* [Wavetable Oscillator (WAVE)](/docs/wave.md)

## Authors
@thomassidor - Project owner.

@baconpaul - Added polyphony to oscillator, logic and arithmetic modules.


## Color themes
Several color themes are included for each module:

* Light (Default)
* River Bed
* Oxford Blue
* Shark
* Cod Gray
* Firefly
* If you want a specific color theme included, let me know.

For all themes you can force the input ports to use a light theme, if you prefer the increased contrast. The option is available via right clicking the modules.

#### Light (Default)
![Default color theme](/combined-screenshots/default.png?raw=true)

#### River Bed
![River Bed](/combined-screenshots/river-bed.png?raw=true)

#### Oxford Blue
![Oxford Blue](/combined-screenshots/oxford-blue.png?raw=true)

#### Shark
![Shark](/combined-screenshots/shark.png?raw=true)

#### Cod Gray
![Cod Gray](/combined-screenshots/cod-gray.png?raw=true)

#### Firefly
![Firefly](/combined-screenshots/firefly.png?raw=true)



## Changelog

Version 1.5.0
* Added: [Random Wrangler (RW)](/docs/rw.md)
* Added: Polyphony to simple oscillators (SIN, SAW, SQR, TRI), plus oscillators (SIN+, SAW+, SQR+, TRI+), Simplex Oscillator (SN-OSC), Wavetable Oscillator (WAVE), TT-A and TT-L. Huge thanks to @baconpaul for contributing with this!
* Behind the scenes: Refactored WAVE for performance improvements.
* Behind the scenes: Improved screenshot generation

Version 1.4.2
* Added: Dark ports for dark themes.
* Added: Option to force light ports on dark themes.
* Added: Placeholder waveforms added to WAVE and SN-OSC for when displayed in module browser.
* Added: Help text to WAVE when no wavetable loaded or captured.
* Changed: License from MIT to GPLv3
* Fixed: Readjusted scope height in Simplex Oscillator.
* Behind the scenes: Added automatic screenshot generation.

Version 1.4.1
* The changes in this release is based on [community feedback](https://community.vcvrack.com/t/tiny-tricks-request-for-feedback/6788). A huge thanks to all who pitched in!
* Added: Dark themes. Right click module to access them.
* Changed: Some design details in a bunch of modules.
* Changed: Simplex Oscillator (SN-OSC) now has a trigger input for the waveform mirroring.
* Changed: Wavetable Oscillator (WAVE) now has a trigger input for the waveform mirroring plus capturing (this is quite resource intensive).
* Changed: Added option to turn off scope in Wavetable Oscillator. Useful for improving the UI performance until scope code is improved.
* Changed: Very slight cosmetic request (#12).
* Fixed: Modulation Generators (MG1, MG8, MG16) now keeps values within 0-10 when set to UNI.

Version 1.4.0
* Added: Wavetable Oscillator (WAVE)

Version 1.3.0
* Added: Simplex Oscillator (SN-OSC)
* Fixed: Sync out of simple and plus oscillators only giving 1v
* Fixed: Simplex Noise LFOs stop working after some time
* Fixed: Random Mix stop working after some time
* Changed: Random Mute CV as mod instead of override of mute knob
* Changed: Rebased all oscillators to C4 instead of A4 (#9)

Version 1.2.1
* Fixed: Random Mix x8 (RX8) not working in trigger mode.

Version 1.2.0
* Added: Random Mix x8 (RX8)
* Added: Simplex Noise (SN1)
* Added: Simplex Noise x8 (SN8)
* Fixed: Tint Trick Logic output to low max 1V (#6)

Version 1.1.1
* Fixed: RM8Base.svg missing or not needed ? (#4)
* Fixed: Oscillators not oscillating until you change frequency
* Fixed: Author name formatting in plugin.json
* Fixed: Proper saving and restoring of hardsync states in oscillator modules
* Fixed: Naming of modules to not include Tiny Tricks
* Changed: Name from Tiny Trick Modules to Tiny Tricks

Version 1.1.0
* Added: Simple Sine Oscillator (TT-SINE)
* Added: Simple Sawtooth Oscillator (TT-SAW)
* Added: Simple Square Oscillator (TT-SQR)
* Added: Simple Triangle Oscillator (TT-TRI)
* Added: Sine+ Oscillator (TT-SINE+)
* Added: Sawtooth+ Oscillator (TT-SAW+)
* Added: Square+ Oscillator (TT-SQR+)
* Added: Triangle+ Oscillator (TT-TRI+)
* Fixed: Small issues in plugin.json (#1 #2 )

Version 1.0.2
* Fixed incorrect tags in plugin.json
* Updated formatting of plugin.json

Version 1.0.1
* Intial release
