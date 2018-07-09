
#pragma once

#include "ofMain.h"

// Based on MapBox.js example by Konstantin Käfer https://mapbox.s3.amazonaws.com/kkaefer/sdf/index.html
// Code explained https://www.mapbox.com/blog/text-signed-distance-fields/
// http://chrismweb.com/2015/07/06/how-to-get-distance-field-fonts-in-libgdx-using-hiero-and-finding-an-updated-version-of-hiero/

class FontData {
    
public:
    
    char character;
    int x;
    int y;
    int width;
    int height;
    int offsetx;
    int offsety;
    int xadvance;
    int page;
};

class KernData {
    
public:
    
    char c1;
    char c2;
    int offset;
};

class FontMetrics {
    
public:
    
    FontMetrics(){
        
        buffer=0;
        size=0;
        lineHeight=0;
        baseLine=0;
        
        padding[0] = 0;
        padding[1] = 0;
        padding[2] = 0;
        padding[3] = 0;
    }
    
    ~FontMetrics(){
        
        characterData.clear();
    }
    
    int buffer;
    int size;
    int lineHeight;
    int baseLine;
    int padding[4];
    
    map<char, FontData> characterData;
    vector<KernData> kernSettings;
    
    string fontPath;
    
    ofImage image;
};


class ofxSDFFont {
    
public:
    
    ofxSDFFont();
    ~ofxSDFFont();
    
    void load( string data, int fontSize=60 );
	bool getIsLoaded();
    
    void draw( string, float x, float y );
    void draw( string s, const ofVec2f & pos );
    
    ofRectangle getStringRect( string s, const ofVec2f & pos = ofVec2f(0, 0) );
    ofRectangle getStringRect( string, float x, float y);
    
    void setFontSize( float s );
    void setBuffer( float b );
    void setGamma( float g );
    void setDebug( bool d );
    void setEnableCaching( bool c );
    
    int getLineHeight();
    void setLineHeight( int h );
    
    float getCapHeight();
    float getxHeight();
    
    void drawDebug( string s, const ofVec2f & pos);
    
protected:
    
    bool isFontFaceLoaded(string name);
    
    void parseFont( string path );
    
    void loadShader();
    
    void drawGlyph( char c, float size, ofVec2f & position, float baseline, float kernAmt, bool bFirstChar=false );
    ofRectangle measureGlyph( char c, float size, ofVec2f & position, float baseline, float kernAmt, bool bFirstChar );
    
    int findKernValue( char c1, char c2 );
    
    vector<string> splitString( string str );
    string stripQuotes( string str );
    
    bool bLoadedOK;
    
    string fontFace;
    string fontDirectory;

    FontMetrics * metrics;
    float fontSize;
    float buffer;
    float gamma;
    bool bDebug;
    bool bCacheFont;
    bool bFontChanged;
    
    ofVec2f startPos;
    
    static ofShader sdfShader;
    static bool bShaderLoaded;
    
    ofMesh sdfMesh;
    
    string lastString;
    
    static map<string, pair<FontMetrics*, int>> characterSets;
};