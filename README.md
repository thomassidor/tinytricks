![The current modules](https://github.com/thomassidor/tinytricks/blob/master/screenshots/screenshot-light.png?raw=true)

# Tiny Tricks VCV Rack modules

Tiny Tricks is a collection of (more or less) tiny modules for VCV Rack.

Currently the following are included:
* Tiny Tricks Arithmetic (TT-A)
* Tiny Tricks Logic (TT-L)
* [Sample and Hold x16 (SH16)](/docs/sh16.md)
* Modulation Generator (MG1)
* Modulation Generator x8 (MG8)
* Modulation Generator x16 (MG16)
* [Attenuator x8 (A8)](/docs/a8.md)
* [Random Mute x8 (RM8)](/docs/rm8.md)
* [Random Stereo Mute x8 (RM8s)](/docs/rm8.md)
* Simple Sine Oscillator (TT-SINE)
* Simple Sawtooth Oscillator (TT-SAW)
* Simple Square Oscillator (TT-SQR)
* Simple Triangle Oscillator (TT-TRI)
* Sine+ Oscillator (TT-SINE+)
* Sawtooth+ Oscillator (TT-SAW+)
* Square+ Oscillator (TT-SQR+)
* Triangle+ Oscillator (TT-TRI+)
* [Random Mix x8 (RX8)](/docs/rx8.md)
* [Simplex Noise (SN1)](/docs/sn.md)
* [Simplex Noise x8 (SN8)](/docs/sn.md)
* Simplex Oscillator (SN-OSC)
* [Wavetable Oscillator (WAVE)](/docs/wave.md)

Several [color themes](#color-themes) are included:

* Light (Default)
* River Bed
* Oxford Blue
* Shark
* Cod Gray
* Firefly

Feel free to join the project, copy source code or panel designs.

Questions, comments, enhancement suggestions, ideas for modules and everything else: [Create a ticket.](https://github.com/thomassidor/tinytricks/issues/new)

All the  best, Thomas René Sidor


## Changelog
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

## Color themes
![Default color theme](https://github.com/thomassidor/tinytricks/blob/master/screenshots/screenshot-light.png?raw=true)
![River Bed](https://github.com/thomassidor/tinytricks/blob/master/screenshots/screenshot-river-bed.png?raw=true)
![Oxford Blue](https://github.com/thomassidor/tinytricks/blob/master/screenshots/screenshot-oxford-blue.png?raw=true)
![Shark](https://github.com/thomassidor/tinytricks/blob/master/screenshots/screenshot-shark.png?raw=true)
![Cod Gray](https://github.com/thomassidor/tinytricks/blob/master/screenshots/screenshot-cod-gray.png?raw=true)
![Firefly](https://github.com/thomassidor/tinytricks/blob/master/screenshots/screenshot-firefly.png?raw=true)
