
#include "ofxSDFFont.h"
#include "ofxJSONElement.h"
#include <iostream>
#include <string>
#include <regex>

#define STRINGIFY(x) #x

map<string, pair<FontMetrics*, int>> ofxSDFFont::characterSets;
ofShader ofxSDFFont::sdfShader;
bool ofxSDFFont::bShaderLoaded = false;

// @TODO: issue with font being loaded in class with reference
// @TODO: multipage fonts

//--------------------------------------------------------------

ofxSDFFont::ofxSDFFont(){
    
    if( !bShaderLoaded) loadShader();
    
    setFontSize( 64 );
    setBuffer( 0.52 ) ;
    setGamma( 0.03 );
    setEnableCaching( false );
    setDebug(false);
    
    sdfMesh.setMode(OF_PRIMITIVE_TRIANGLES);
    
    bLoadedOK = false;
    bFontChanged = false;
    
    metrics = NULL;
}

//--------------------------------------------------------------

ofxSDFFont::~ofxSDFFont(){
    
    map<string, pair<FontMetrics*, int>> :: iterator setIter = characterSets.find( fontFace );
    
    if( setIter != characterSets.end() ){
        
        setIter->second.second--;
        
        if( setIter->second.second == 0 ){
            
            ofLogNotice("ofxSDFFont") << "Removing font " << fontFace;
            
            delete setIter->second.first;
            
            characterSets.erase( setIter );
        }
    }
}

//--------------------------------------------------------------

void ofxSDFFont::load( string data, int fontSize ){
    
    ofFile file(data);
    
    if( file.getExtension() != "fnt") {
        
        ofLogError("ofxSDFFont") << "Incorrect file type. " << data << " failed to load";
        return;
    }
    
    parseFont( data );
    
    setFontSize( fontSize );
}

//--------------------------------------------------------------

bool ofxSDFFont::getIsLoaded() {

	return bLoadedOK;
}

//--------------------------------------------------------------

void ofxSDFFont::draw( string s, float x, float y ){
    
    draw( s, ofVec2f(x, y));
}

//--------------------------------------------------------------

void ofxSDFFont::draw( string s, const ofVec2f & pos ){
    
    if( !bLoadedOK ) {
        
        ofLogWarning("ofxSDFFont::draw()") << "Font not loaded";
        return;
    }
    
    if( metrics->size == 0 ){
        
        ofLogWarning("ofxSDFFont::draw()") << "Bad metric value";
        return;
    }
    
    startPos = pos;
    ofVec2f drawCursor = pos;
    
    float scale = (fontSize)/ metrics->size;
    
    bCacheFont = false;
    
    if( bCacheFont & lastString == s && !bFontChanged ){
        
        
        
    } else {
        
        sdfMesh.clear();
    
        for(int i=0; i<s.length(); i++){
            
            int kerning = ( i == 0 ) ? 0 : findKernValue( s[i-1], s[i]);
            
            drawGlyph(s[i], scale, drawCursor, metrics->baseLine, kerning, i==0);
        }
        
        lastString = s;
        
        if( bFontChanged ) bFontChanged = false;
    }
    
    sdfShader.begin();
    sdfShader.setUniformTexture("u_texture", (metrics->image), 0);
    sdfShader.setUniform1f("u_buffer", buffer );
    sdfShader.setUniform1f("u_gamma", gamma * 1.4142 / scale);
    sdfShader.setUniform1f("u_debug", bDebug ? 1.0 : 0.0);
    
    sdfMesh.draw();
    
    sdfShader.end();
}

//--------------------------------------------------------------

void ofxSDFFont::drawDebug( string s, const ofVec2f & pos ){
    
    ofSetColor(255);
    
	ofPushMatrix();

	ofTranslate(pos);

    metrics->image.draw(0, 0);
    
    ofNoFill();
    
    ofSetColor(255, 0, 0);
    
    if( s.length() == 0 ){
        
        int nChars = metrics->characterData.size();
        
        for(int i=0; i<nChars; i++){
            
            char c = metrics->characterData[i].character;
            
            auto width = metrics->characterData[c].width; //data[0];
            auto height = metrics->characterData[c].height; //data[1];
            auto horiBearingX = metrics->characterData[c].offsetx; //data[2];
            auto horiBearingY = metrics->characterData[c].offsety; //data[3];
            auto horiAdvance = metrics->characterData[c].xadvance; //data[4];
            auto posX = metrics->characterData[c].x; //data[5];
            auto posY = metrics->characterData[c].y; //data[6];
            
            if (width > 0 && height > 0) {
                
                // add texture coordinates
                
                ofVec2f texCoord = ofVec2f( posX, posY );
                ofDrawRectangle(texCoord, width, height);
            }
        }
        
    } else {
    
        for(int i=0; i<s.length(); i++){

            char c = s.at(i);
            
            auto width = metrics->characterData[c].width; //data[0];
            auto height = metrics->characterData[c].height; //data[1];
            auto horiBearingX = metrics->characterData[c].offsetx; //data[2];
            auto horiBearingY = metrics->characterData[c].offsety; //data[3];
            auto horiAdvance = metrics->characterData[c].xadvance; //data[4];
            auto posX = metrics->characterData[c].x; //data[5];
            auto posY = metrics->characterData[c].y; //data[6];
            
            if (width > 0 && height > 0) {
                
                // add texture coordinates
                
                ofVec2f texCoord = ofVec2f( posX, posY );
                
                ofDrawRectangle(texCoord, width, height);
            }
        }
    }
        
    ofFill();

	ofPopMatrix();
}

//--------------------------------------------------------------

ofRectangle ofxSDFFont::getStringRect( string s, const ofVec2f & pos ){
    
    if( !bLoadedOK ) {
        
        ofLogWarning("ofxSDFFont::getStringRect()") << "Font not loaded";
        
        return ofRectangle(pos.x, pos.y, 0, 0);
    }
    
    ofRectangle bounds;
    bounds.x = pos.x;
    bounds.y = pos.y;
    
    if( s.length() == 0 ) return bounds;
    
    ofVec2f drawCursor = pos;
    
    float scale = fontSize / metrics->size;
    
    for(int i=0; i<s.length(); i++){
        
        int kerning = ( i == 0 ) ? 0 : findKernValue( s[i-1], s[i]);
        
        ofRectangle r = measureGlyph(s[i], scale, drawCursor, metrics->baseLine, kerning, i==0);
        
        if( i == 0 ){
            
            bounds.x = r.x;
            bounds.y = r.y;
        }
        
        if( r.x < bounds.x ) bounds.x = r.x;
        if( r.x + r.width > bounds.x + bounds.width ) bounds.width = r.width + r.x - bounds.x;
        
        if( r.y < bounds.y ) bounds.y = r.y;
        if( r.y + r.height > bounds.y + bounds.height ) bounds.height = r.height + r.y - bounds.y;
        
        //ofDrawRectangle(r);
    }
    
    return bounds;
}

//--------------------------------------------------------------

ofRectangle ofxSDFFont::getStringRect( string s, float x, float y ){
    
    return getStringRect( s, ofVec2f(x, y));
}

//--------------------------------------------------------------

float ofxSDFFont::getCapHeight(){
    
    if( !bLoadedOK ) {
        
        ofLogWarning("ofxSDFFont::getStringRect()") << "Font not loaded";
        
        return 0.0;
    }

    float scale = fontSize / metrics->size;
    
    ofVec2f pos;
        
    ofRectangle bounds = measureGlyph('M', scale, pos, metrics->baseLine, 0.0, 0.0);
    
    return bounds.height;
}

//--------------------------------------------------------------

float ofxSDFFont::getxHeight(){
    
    if( !bLoadedOK ) {
        
        ofLogWarning("ofxSDFFont::getStringRect()") << "Font not loaded";
        
        return 0.0;
    }
    
    float scale = fontSize / metrics->size;
    
    ofVec2f pos;
    
    ofRectangle bounds = measureGlyph('x', scale, pos, metrics->baseLine, 0.0, 0.0);
    
    return bounds.height;
}

//--------------------------------------------------------------

void ofxSDFFont::setFontSize( float s ){
    
    bFontChanged = true;
    
    fontSize = s;
}

//--------------------------------------------------------------

void ofxSDFFont::setBuffer( float b ){
    
    buffer = b;
}

//--------------------------------------------------------------

void ofxSDFFont::setGamma( float g ){
    
    gamma = g;
}

//--------------------------------------------------------------

void ofxSDFFont::setDebug( bool d ){
    
    bDebug = d;
}

//--------------------------------------------------------------

void ofxSDFFont::setEnableCaching( bool c ){
    
    bCacheFont = c;
}

//--------------------------------------------------------------

int ofxSDFFont::getLineHeight(){
    
    if( !bLoadedOK ) return 0;
    
    return metrics->lineHeight;
}

//--------------------------------------------------------------

void ofxSDFFont::setLineHeight( int h ){
    
    if( !bLoadedOK ) return;
    
    metrics->lineHeight = h;
}

//--------------------------------------------------------------

void ofxSDFFont::parseFont( string path ){
    
    ofFile file(path);
    
    if( !file.exists() ){
        
        ofLogWarning("ofxSDFFont") << "Font file not found";
        return;
    }
    
    fontDirectory = file.getEnclosingDirectory();
    fontFace = file.getBaseName();
    
    // check if font already loaded
    
    if( isFontFaceLoaded(file.getBaseName()) ){
        
        metrics = characterSets[fontFace].first;
        characterSets[fontFace].second++; // increment how many loaded
        bLoadedOK = true;
     
        return;
    }
    
    // if not, parse font data file
    
    ofBuffer buffer = ofBufferFromFile(path);
    
    //Read file line by line
    for (ofBuffer::Line it = buffer.getLines().begin(), end = buffer.getLines().end(); it != end; ++it) {
        
        string line = *it;
        
        if( line.length() == 0 ) continue; // skip
        
        // split line into strings
        vector<string> words = splitString(line);
        
        if( words[0] == "info" ){
            
            for(int i=0; i<words.size(); i++){
                
                vector<string> item = ofSplitString(words[i], "=");
                
                if( item[0] == "face" ){
                    
                    ofLogNotice("ofxSDFFont") << "Loading font " << fontFace;
                    
                    // make font metrics object
                    // and keep track of how many classes access this metric
                    metrics = new FontMetrics();
                    characterSets[fontFace] = make_pair(metrics, 1);
                    
                } else if( item[0] == "size"){
                    
                    // capture the font size
                    metrics->size = ofToInt(item[1]);
                
                } else if( item[0] == "padding"){
                    
                    vector<string> values = ofSplitString(item[1], ",");
                    
                    metrics->padding[0] = ofToInt(values[0]);
                    metrics->padding[1] = ofToInt(values[1]);
                    metrics->padding[2] = ofToInt(values[2]);
                    metrics->padding[3] = ofToInt(values[3]);
                }
            }
            
        } else if( words[0] == "common"){
            
            vector<string> parts = splitString(line);
            
            for(int i=0; i<parts.size(); i++){
                
                vector<string> item = ofSplitString(words[i], "=");
                
                if( item[0] == "lineHeight" ){
                    
                    // capture the line height
                    metrics->lineHeight = ofToInt(item[1]);
                    
                } else if( item[0] == "base"){
                    
                    // capture the baseline
                    metrics->baseLine = ofToInt(item[1]);
                }
            }
            
        } else if( words[0] == "page"){
            
            // get images
            // right now we only support one page
            // TODO: multipage support
            
            vector<string> parts = ofSplitString(words[2], "=");
            string imageName = stripQuotes(parts[1]);

            bLoadedOK = metrics->image.load( fontDirectory+ imageName);
            
            if( !bLoadedOK ){
                
                ofLogWarning("ofxSDFFont") << "Failed to load font image file";
                return;
            }
          
        } else if( words[0] == "chars"){
            
            
            
        } else if( words[0] == "char"){
            
            //ofLogNotice("char") << *it;
            
            FontData data;
            
            for(int i=0; i<words.size(); i++){
                
                vector<string> parts = ofSplitString(words[i], "=");
                
                if( parts[0] == "id" ){
                    
                    data.character = ofToInt(parts[1]);
                    
                    //ofLogNotice("char") << data.character;
                    //ofLogNotice("id") << parts[1];
                    
                } else if( parts[0] == "x"){
                    
                    //ofLogNotice("x") << parts[1];
                    data.x = ofToInt(parts[1]);
                
                } else if( parts[0] == "y"){
                    
                   // ofLogNotice("y") << parts[1];
                    data.y = ofToInt(parts[1]);
                
                } else if( parts[0] == "width"){
                    
                   // ofLogNotice("width") << parts[1];
                    data.width = ofToInt(parts[1]);
                
                } else if( parts[0] == "height"){
                    
                    //ofLogNotice("height") << parts[1];
                    data.height = ofToInt(parts[1]);
                
                } else if( parts[0] == "xoffset"){
                    
                    //ofLogNotice("xoffset") << parts[1];
                    data.offsetx = ofToInt(parts[1]);
                    
                } else if( parts[0] == "yoffset"){
                    
                    //ofLogNotice("yoffset") << parts[1];
                    data.offsety = ofToInt(parts[1]);
                
                } else if( parts[0] == "xadvance"){
                    
                    //ofLogNotice("xadvance") << parts[1];
                    data.xadvance = ofToInt(parts[1]);
                
                } else if( parts[0] == "page"){
                    
                    //ofLogNotice("page") << parts[1];
                    data.page = ofToInt(parts[1]);
                }
            }
            
            metrics->characterData[data.character] = data;
            
        } else if( words[0] == "kernings"){
            
            //vector<string> parts = ofSplitString(words[1], "=");
            //int nKerningValues = ofToInt(parts[1]);
            
        } else if( words[0] == "kerning"){
            
            int nWords = words.size();
            
            KernData kerning;
            
            for(int i=1; i<nWords; i++){
                
                //ofLogNotice() << words[i];
                
                vector<string> parts = ofSplitString(words[i], "=");
                
                if( parts[0]== "first"){
                    
                    kerning.c1 = ofToInt(parts[1]);
                    
                } else if( parts[0] == "second"){
                    
                    kerning.c2 = ofToInt(parts[1]);
                    
                } else if( parts[0] == "amount"){
                    
                    kerning.offset = ofToInt(parts[1]);
                }
            }
            
            metrics->kernSettings.push_back(kerning);
            
        } else {
            
            //ofLogNotice("unknown") << *it;
        }
        /*
        vector<string> words = ofSplitString(line, ",");
        
        //Store strings into a custom container
        if (words.size()>=2) {
            MorseCodeSymbol symbol;
            symbol.character = words[0];
            symbol.code = words[1];
            
            //Save MorseCodeSymbols for later
            morseCodeSymbols.push_back(symbol);
            
            //Debug output
            ofLogVerbose("symbol.character: " + symbol.character);
            ofLogVerbose("symbol.code: " + symbol.code);
        }
        */
    }
}

//--------------------------------------------------------------

bool ofxSDFFont::isFontFaceLoaded( string name ){

    map<string, pair<FontMetrics*, int>> :: iterator setIter = characterSets.find( name );

    if( setIter != characterSets.end() ){
        
        return true;
    }
    
    return false;
}

//--------------------------------------------------------------

void ofxSDFFont::loadShader(){
    
    string defaultVertexShader;
    string defaultFragmentShader;
    
    if( ofIsGLProgrammableRenderer() ){
    
        const string shader_version = "#version 150\n";
        
        defaultVertexShader = shader_version + STRINGIFY(
                                               
           uniform mat4 projectionMatrix;
           uniform mat4 modelViewMatrix;
           uniform mat4 textureMatrix;
           uniform mat4 modelViewProjectionMatrix;
           uniform vec4 globalColor;
                                                
           in vec4  position;
           in vec2  texcoord;
           in vec4  color;
           in vec3  normal;
           
           out vec4 colorVarying;
           out vec2 texCoordVarying;
           out vec4 normalVarying;
           
           void main()
           {
               colorVarying = globalColor;
               texCoordVarying = (textureMatrix*vec4(texcoord.x,texcoord.y,0,1)).xy;
               gl_Position = modelViewProjectionMatrix * position;
           }
        );
        
        defaultFragmentShader = shader_version + STRINGIFY(
        
            uniform sampler2DRect u_texture;
            
            uniform float u_buffer;
            uniform float u_gamma;
            uniform float u_debug;
            
            in vec4 colorVarying;
            in vec2 texCoordVarying;
                                                 
            out vec4 fragColor;
            
            void main() {
                float dist = texture(u_texture, texCoordVarying).a;
                if (u_debug > 0.0) {
                    fragColor = vec4(dist, dist, dist, 1);
                } else {
                    float alpha = smoothstep(u_buffer - u_gamma, u_buffer + u_gamma, dist);
                    fragColor = vec4(colorVarying.rgb, alpha * colorVarying.a);
                }
            }
        );
        
    } else {
        
        ofLogWarning("ofxSDFFont") << "There is no shader defined for non-programmable renderer. Please enable the programmable renderer";
    }
    
    bLoadedOK = sdfShader.setupShaderFromSource(GL_VERTEX_SHADER,defaultVertexShader);
    
    if( !bLoadedOK ) return;
    
    sdfShader.setupShaderFromSource(GL_FRAGMENT_SHADER,defaultFragmentShader);
    
    if( !bLoadedOK ) return;
    
    sdfShader.bindDefaults();
    sdfShader.linkProgram();
    
    bShaderLoaded = true;
}

//--------------------------------------------------------------

void ofxSDFFont::drawGlyph( char s, float size, ofVec2f & position, float baseline, float kernAmt, bool bFirstChar ){
    
    if( s == '\n' ){
        
        position.x = startPos.x;
        position.y += (metrics->lineHeight ) * size;
        
        return;
    }
    
    if( bFirstChar ){
        
        position.x -= metrics->padding[0] * size;
    }
    
    position.x += ( kernAmt ) * size;
    
    if( metrics->characterData.find(s) == metrics->characterData.end() ) return;
    
    auto width = metrics->characterData[s].width;
    auto height = metrics->characterData[s].height;
    auto horiBearingX = bFirstChar ?  0 : metrics->characterData[s].offsetx;
    auto horiBearingY = metrics->characterData[s].offsety;
    auto horiAdvance = metrics->characterData[s].xadvance;
    auto posX = metrics->characterData[s].x;
    auto posY = metrics->characterData[s].y;
    
    if (width > 0 && height > 0) {
        
        // add two triangles per glyph
        
        ofVec2f vertices[4];
        vertices[0].set((position.x + ((horiBearingX ) * size)), (position.y + (-baseline + horiBearingY) * size));
        vertices[1].set((position.x + ((horiBearingX  + width) * size)), (position.y + (-baseline +  horiBearingY) * size));
        vertices[2].set((position.x + ((horiBearingX ) * size)), (position.y + (height -baseline +  horiBearingY) * size));
        vertices[3].set((position.x + ((horiBearingX  + width) * size)), (position.y + (height - baseline + horiBearingY) * size));
        
        /*vertices[0].set(factor * (position.x + ((horiBearingX - metrics->buffer) * size)), (factor * (position.y + horiBearingY * size)));
        vertices[1].set(factor * (position.x + ((horiBearingX - metrics->buffer + width) * size)), (factor * (position.y + horiBearingY * size)));
        vertices[2].set(factor * (position.x + ((horiBearingX - metrics->buffer) * size)), (factor * (position.y + (height + horiBearingY) * size)));
        vertices[3].set(factor * (position.x + ((horiBearingX - metrics->buffer + width) * size)), (factor * (position.y + (height + horiBearingY) * size)));*/
        
        sdfMesh.addVertex(vertices[0]);
        sdfMesh.addVertex(vertices[1]);
        sdfMesh.addVertex(vertices[2]);
        
        sdfMesh.addVertex(vertices[1]);
        sdfMesh.addVertex(vertices[2]);
        sdfMesh.addVertex(vertices[3]);
        
        // add texture coordinates
        
        ofVec2f texCoords[4];
        texCoords[0] = ofVec2f( posX, posY );
        texCoords[1] = ofVec2f( posX+width, posY );
        texCoords[2] = ofVec2f( posX, posY+height );
        texCoords[3] = ofVec2f( posX+width, posY+height );
        
        sdfMesh.addTexCoord(texCoords[0]);
        sdfMesh.addTexCoord(texCoords[1]);
        sdfMesh.addTexCoord(texCoords[2]);
        
        sdfMesh.addTexCoord(texCoords[1]);
        sdfMesh.addTexCoord(texCoords[2]);
        sdfMesh.addTexCoord(texCoords[3]);
        
        /*
        
        ofPushStyle();
        ofSetColor(ofColor::blue);
        
        ofDrawEllipse(vertices[0], 8, 8);
        
        ofNoFill();
        ofDrawRectangle(vertices[0].x, vertices[0].y, vertices[1].x-vertices[0].x, vertices[2].y-vertices[1].y);
        ofFill();
        
        ofSetColor(ofColor::red);
        ofNoFill();
        //ofDrawRectangle(position.x, position.y, (horiBearingX) * size, (horiBearingY) * size);
        ofFill();
        
        ofSetColor(ofColor::yellow);
        //ofDrawEllipse(vertices[0].x, vertices[0].y, 5, 5);
        
        //ofLogNotice("base") << metrics->baseLine;
        
        //ofDrawLine(position.x, position.y, position.x + width, position.y);
        
        ofPopStyle();
        
        */
    }
    
    //if( ofGetMousePressed() ) kernAmt = 0;
    
    //if( kernAmt != 0 ) ofLogNotice() << "kern " << kernAmt;

    position.x += ( horiAdvance ) * size;
}

//--------------------------------------------------------------

ofRectangle ofxSDFFont::measureGlyph( char s, float size, ofVec2f & position, float baseline, float kernAmt, bool bFirstChar ){
    
    ofRectangle bounds;
    
    if( s == '\n' ){
        
        position.x = startPos.x;
        position.y += (metrics->lineHeight) * size;
        
        bounds.x = position.x;
        bounds.y = position.y;
        
        return bounds;
    }
    
    if( bFirstChar ){
        
        position.x -= metrics->padding[0] * size;
    }
    
    position.x += ( kernAmt ) * size;
    
    if( metrics->characterData.find(s) == metrics->characterData.end() ) return bounds;

    auto width = metrics->characterData[s].width;
    auto height = metrics->characterData[s].height;
    auto horiBearingX = bFirstChar ? 0 : metrics->characterData[s].offsetx;
    auto horiBearingY = metrics->characterData[s].offsety;
    auto horiAdvance = metrics->characterData[s].xadvance;
    auto posX = metrics->characterData[s].x;
    auto posY = metrics->characterData[s].y;
    
    if( s == ' '){
        
        position.x += horiAdvance * size;
        bounds.x = position.x;
        bounds.y = position.y-baseline*size;
        
        return bounds;
    }
    
    if (width > 0 && height > 0) {
        
        ofVec2f vertices[3];
        vertices[0].set(position.x + ((horiBearingX + metrics->padding[0] ) * size), position.y + (horiBearingY + metrics->padding[2])* size);
        vertices[1].set(position.x + ((horiBearingX + width - metrics->padding[1] ) * size), position.y + (horiBearingY + metrics->padding[2])* size);
        vertices[2].set(position.x + ((horiBearingX + metrics->padding[0] ) * size), position.y + (height + horiBearingY - metrics->padding[3]) * size);
        
        bounds.x = vertices[0].x;
        bounds.y = vertices[0].y-(baseline*size);
        
        bounds.width = vertices[1].x-vertices[0].x;
        bounds.height = vertices[2].y-vertices[1].y;
        
        /*
        ofPushStyle();
        ofSetColor(ofColor::red);
        
        ofDrawEllipse(vertices[0]-ofVec2f(0, baseline*size), 5, 5);
        
        ofNoFill();
        ofDrawRectangle(bounds);
        ofFill();
        ofPopStyle();
         */
    }
    
    /*
    ofSetColor(ofColor::yellow);
    ofDrawLine(position.x, position.y, position.x+horiAdvance * size, position.y);
    ofDrawEllipse(position.x, position.y, 5, 5);
    */
    
    position.x += ( horiAdvance + kernAmt ) * size;
    
    return bounds;
}

int ofxSDFFont::findKernValue( char c1, char c2 ){
    
    int kernValue = 0;
    bool valueFound = false;
    
    vector<KernData> :: iterator kIter = metrics->kernSettings.begin();
    while( kIter != metrics->kernSettings.end() && !valueFound){
        
        if( kIter->c1 == c1 && kIter->c2 == c2 ){
            
            //ofLogNotice() << "matched kerning " << c1 << " & "  << c2 << ": "<< kIter->offset;
            valueFound = true;
            kernValue = kIter->offset;
        }
        
        kIter++;
    }
    
    return kernValue;
}

vector<string> ofxSDFFont::splitString( string s ){
    
    vector<string> results;
    
    std::smatch m;
    std::regex e ("(?:[^\\s\"]+|\"[^\"]*\")+");
    
    //std::cout << "Target sequence: " << str << std::endl;
    //std::cout << "Regular expression: /\\b(sub)([^ ]*)/" << std::endl;
    //std::cout << "The following matches and submatches were found:" << std::endl;
    
    while (std::regex_search (s,m,e)) {
        for (auto x:m) results.push_back(x); //std::cout << x << " ";
       // std::cout << std::endl;
        s = m.suffix().str();
        
    }
    
    return results;
}

string ofxSDFFont::stripQuotes( string s ){
    
    string results;
    
    std::smatch m;
    std::regex e ("\"(.*?)\"");
    
    //std::cout << "Target sequence: " << str << std::endl;
    //std::cout << "Regular expression: /\\b(sub)([^ ]*)/" << std::endl;
    //std::cout << "The following matches and submatches were found:" << std::endl;
    
    while (std::regex_search (s,m,e)) {
        for (auto x:m) results = x;
        s = m.suffix().str();
        
    }
    
    return results;
}
