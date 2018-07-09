#include "ofMain.h"
#include "ofApp.h"

//========================================================================
int main( ){


	ofGLFWWindowSettings windowSettings;
	windowSettings.setGLVersion(3, 2);  // programmable pipeline
	windowSettings.width = 800;
	windowSettings.height = 600;
	windowSettings.numSamples = 16;
	ofCreateWindow(windowSettings);

	// this kicks off the running of my app
	// can be OF_WINDOW or OF_FULLSCREEN
	// pass in width and height too:
	ofRunApp(new ofApp());

}
