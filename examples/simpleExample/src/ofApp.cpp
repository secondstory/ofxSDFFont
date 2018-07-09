#include "ofApp.h"

//--------------------------------------------------------------
void ofApp::setup(){

	// load a .fnt file
	// use Hiero to create your own files (https://github.com/libgdx/libgdx/wiki/Hiero & https://libgdx.badlogicgames.com/tools.html)

	font.load("Comic Sans.fnt", 60);
}

//--------------------------------------------------------------
void ofApp::update(){

	
}

//--------------------------------------------------------------
void ofApp::draw(){

	ofBackground(0);

	ofSetColor(255);
	font.draw("Hello World", 20, 80);

	// visualize the bounding box

	font.draw("Bounding Box", 20, 160);

	ofRectangle boundingBox = font.getStringRect("Bounding Box", ofVec2f(20, 160));

	ofPushStyle();
	ofSetColor(ofColor::red);
	ofNoFill();
	ofDrawRectangle(boundingBox);
	ofPopStyle();

	// visualize x-height

	font.draw("Font X-Height", 20, 240);

	boundingBox = font.getStringRect("Font X-Height", ofVec2f(20, 240));
	float xHeight = font.getxHeight();

	ofPushStyle();
	ofSetColor(ofColor::red);
	ofLine(20, 240 - xHeight, 20+ boundingBox.width, 240 - xHeight);
	ofPopStyle();

	// SDF font debug view

	ofPushMatrix();

	font.drawDebug("FONT TABLE", ofVec2f(20, 320));

	ofPopMatrix();
}

//--------------------------------------------------------------
void ofApp::keyPressed(int key){

}

//--------------------------------------------------------------
void ofApp::keyReleased(int key){

}

//--------------------------------------------------------------
void ofApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void ofApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void ofApp::mouseEntered(int x, int y){

}

//--------------------------------------------------------------
void ofApp::mouseExited(int x, int y){

}

//--------------------------------------------------------------
void ofApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void ofApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void ofApp::dragEvent(ofDragInfo dragInfo){ 

}
