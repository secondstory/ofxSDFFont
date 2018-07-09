# ofxSDFFont
openFrameworks addon for drawing fonts using signed distance functions (SDF) 

# usage

```c++
// load a .fnt file
font.load("Comic Sans.fnt", 60);

// draw a string
font.draw("Hello World", ofVec2f(20, 60));
```

# how to generate a SDF font
1. download Hiero (https://github.com/libgdx/libgdx/wiki/Hiero) from https://libgdx.badlogicgames.com/tools.html
2. run runnable-hiero.jar
3. select a font face
4. from the file menu, select "Save BMFont files ..."
5. drag the *.fnt and *.png files to your OF application's data folder

# known issues
1. ofxSDFFont was written for 0.9.8 and has not been tested in OF 0.10.0
2. ofxSDFFont only runs with the programmable renderer (see examples/simpleexample/src/main.cpp)
3. ofxSDFFont does currently work with fnt files containing multiple png files

# explanation & credits
ofxSDFFont is based on MapBox.js example by Konstantin Käfer https://mapbox.s3.amazonaws.com/kkaefer/sdf/index.html
How SDF Fonts work https://www.mapbox.com/blog/text-signed-distance-fields/
