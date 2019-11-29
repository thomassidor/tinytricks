# Modulation Generator (MG, MG8, MG16)
Randomly generated modulation sorces in 1x, 8x and 16x versions.

Upon a trigger on `TRIG` the module generates either:

1. A LFO with random frequency and shape.
2 Or a random S&H value (only if the `S&H` switch is at 1. There’s a 50/50 chance you’ll get either a LFO or a constant S&H)

`VAR` (variance) sets how different the settings for the LFO and S&H will be when regenerated. E.g. setting VAR to 0 will give you a LFO with the same frequency everytime (although the shape of the LFO changes).

`BIAS` is defining the average of the values generated. Use it to bias the values generated to some side. If `VAR` is 0, then `BIAS` defines the value that will be generated every time. Think of `BIAS` as being a centerpoint and `VAR` as a spread to both sides.

The `UNI`/`BI` switch switches between uni-polar (0v to 10v) and bi-polar (-5v to +5v) mode.

![Default theme](/module-screenshots/default/MG1.png?raw=true)
![Default theme](/module-screenshots/default/MG8.png?raw=true)
![Default theme](/module-screenshots/default/MG16.png?raw=true)

![River bed theme](/module-screenshots/river-bed/MG1.png?raw=true)
![River bed theme](/module-screenshots/river-bed/MG8.png?raw=true)
![River bed theme](/module-screenshots/river-bed/MG16.png?raw=true)

![Shark theme](/module-screenshots/shark/MG1.png?raw=true)
![Shark theme](/module-screenshots/shark/MG8.png?raw=true)
![Shark theme](/module-screenshots/shark/MG16.png?raw=true)

![Oxford blue theme](/module-screenshots/oxford-blue/MG1.png?raw=true)
![Oxford blue theme](/module-screenshots/oxford-blue/MG8.png?raw=true)
![Oxford blue theme](/module-screenshots/oxford-blue/MG16.png?raw=true)

![Cod gray theme](/module-screenshots/cod-gray/MG1.png?raw=true)
![Cod gray theme](/module-screenshots/cod-gray/MG8.png?raw=true)
![Cod gray theme](/module-screenshots/cod-gray/MG16.png?raw=true)

![Firefly theme](/module-screenshots/firefly/MG1.png?raw=true)
![Firefly theme](/module-screenshots/firefly/MG8.png?raw=true)
![Firefly theme](/module-screenshots/firefly/MG16.png?raw=true)
