#include "ofxGLWarper.h"
#include "stdio.h"
#include "cv.h"


//--------------------------------------------------------------
void ofxGLWarper::setup(){
	setup(ofGetWidth(), ofGetHeight());
}
//--------------------------------------------------------------
void ofxGLWarper::setup(int _resX, int _resY){	 
	setup(0,0,_resX,_resY);	
}
//--------------------------------------------------------------
void ofxGLWarper::setup(int _x, int _y, int _w, int _h){
    cout << "ofxGLWarper setup: " <<_x << " " <<_y << " " <<_w << " " <<_h << endl;
	ofUnregisterMouseEvents(this);
	/*
	corners[0].x = 0.0;
	corners[0].y = 0.0;
	
	corners[1].x = 1.0;
	corners[1].y = 0.0;
	
	corners[2].x = 1.0;
	corners[2].y = 1.0;
	
	corners[3].x = 0.0;
	corners[3].y = 1.0;
	//*/
    corners[0].x = _x;
	corners[0].y = _y;
	
	corners[1].x = _x + _w;
	corners[1].y = _y;
	
	corners[2].x = _x + _w;
	corners[2].y = _y + _h;
	
	corners[3].x = _x;
	corners[3].y = _y + _h;
    
	active=false;
	
	for(int i = 0; i < 16; i++){
		if(i % 5 != 0) myMatrix[i] = 0.0;
		else myMatrix[i] = 1.0;
	}
	x=_x;
	y=_y;
	width=_w;
	height=_h;
	whichCorner = -1;
    cornerSelected = false;
    cornerSensibility = 0.5;
    bUseKeys = true;
}
//--------------------------------------------------------------
bool ofxGLWarper::isActive(){
	return active;
}

void ofxGLWarper::setActive(bool b) {
    if (b) activate();
    else deactivate();
}
//--------------------------------------------------------------
void ofxGLWarper::activate(){
	ofRegisterMouseEvents(this);
	active=true;
    if (bUseKeys) {
        ofRegisterKeyEvents(this);
    }
}
//--------------------------------------------------------------
void ofxGLWarper::deactivate(){
	ofUnregisterMouseEvents(this);
	active=false;
    if (bUseKeys) {
        ofUnregisterKeyEvents(this);
    }
}
//--------------------------------------------------------------
void ofxGLWarper::toogleActive(){
    if(!active){
        activate();
        cout << "activate"<<endl;
    }else{
        deactivate();
        cout << "desactivate"<<endl;
    }
}
//--------------------------------------------------------------
void ofxGLWarper::enableKeys(bool k){
//    bUseKeys=k;
    if (k) {
        ofRegisterKeyEvents(this);
    }else{
        ofUnregisterKeyEvents(this);
    }
}
//--------------------------------------------------------------
void ofxGLWarper::toogleKeys(){
    enableKeys(!bUseKeys);
}
//--------------------------------------------------------------
bool ofxGLWarper::getUseKeys(){
    return bUseKeys;
}
//--------------------------------------------------------------
void ofxGLWarper::setUseKeys(bool use){
    bUseKeys = use;
}
//--------------------------------------------------------------
void ofxGLWarper::processMatrices(){
	//we set it to the default - 0 translation
	//and 1.0 scale for x y z and w
	for(int i = 0; i < 16; i++){
		if(i % 5 != 0) myMatrix[i] = 0.0;
		else myMatrix[i] = 1.0;
	}
	
	//we need our points as opencv points
	//be nice to do this without opencv?
	CvPoint2D32f cvsrc[4];
	CvPoint2D32f cvdst[4];	
	
	//we set the warp coordinates
	//source coordinates as the dimensions of our window
	cvdst[0].x = x;
	cvdst[0].y = y;
	cvdst[1].x = x+width;
	cvdst[1].y = y;
	cvdst[2].x = x+width;
	cvdst[2].y = y+height;
	cvdst[3].x = x;
	cvdst[3].y = y+height;
	
	//corners are in 0.0 - 1.0 range
	//so we scale up so that they are at the window's scale
	for(int i = 0; i < 4; i++){
		//cvdst[i].x = corners[i].x  * (float)width;
		//cvdst[i].y = corners[i].y * (float)height;
        cvsrc[i].x = corners[i].x;
		cvsrc[i].y = corners[i].y;
	}
	
	//we create a matrix that will store the results
	//from openCV - this is a 3x3 2D matrix that is
	//row ordered
	CvMat * translate = cvCreateMat(3,3,CV_32FC1);
	
	//this is the slightly easier - but supposidly less
	//accurate warping method 
	//cvWarpPerspectiveQMatrix(cvsrc, cvdst, translate); 
	
	
	//for the more accurate method we need to create
	//a couple of matrixes that just act as containers
	//to store our points  - the nice thing with this 
	//method is you can give it more than four points!
	
	CvMat* src_mat = cvCreateMat( 4, 2, CV_32FC1 );
	CvMat* dst_mat = cvCreateMat( 4, 2, CV_32FC1 );
	
	//copy our points into the matrixes
	cvSetData( src_mat, cvsrc, sizeof(CvPoint2D32f));
	cvSetData( dst_mat, cvdst, sizeof(CvPoint2D32f));
	
	//figure out the warping!
	//warning - older versions of openCV had a bug
	//in this function.
	cvFindHomography(src_mat, dst_mat, translate);
	
	//get the matrix as a list of floats
	float *matrix = translate->data.fl;
	
	
	//we need to copy these values
	//from the 3x3 2D openCV matrix which is row ordered
	//
	// ie:   [0][1][2] x
	//       [3][4][5] y
	//       [6][7][8] w
	
	//to openGL's 4x4 3D column ordered matrix
	//        x  y  z  w   
	// ie:   [0][3][ ][6]
	//       [1][4][ ][7]
	//		 [ ][ ][ ][ ]
	//       [2][5][ ][9]
	//       
	
	myMatrix[0]		= matrix[0];
	myMatrix[4]		= matrix[1];
	myMatrix[12]	= matrix[2];
	
	myMatrix[1]		= matrix[3];
	myMatrix[5]		= matrix[4];
	myMatrix[13]	= matrix[5];	
	
	myMatrix[3]		= matrix[6];
	myMatrix[7]		= matrix[7];
	myMatrix[15]	= matrix[8];	
	
	
}
//--------------------------------------------------------------
void ofxGLWarper::draw(){
	if (active) {
		ofPushStyle();
		ofSetColor(255, 255, 255);
		ofNoFill();
		ofDrawRectangle(x, y, width, height);
		ofPopStyle();
	}
}
//--------------------------------------------------------------
void ofxGLWarper::begin(bool calibrated){
	if (active) {
		processMatrices();
	}
    this->calibrated = calibrated;
	
    if (calibrated) {
        glPushMatrix();
        glMultMatrixf(myMatrix);
    }
}
//--------------------------------------------------------------
void ofxGLWarper::end(){
	
    if (calibrated) glPopMatrix();
    
    if (active) {// this draws colored squares over the corners as a visual aid. 
        ofPushStyle();
        ofSetRectMode(OF_RECTMODE_CENTER);
        for (int i = 0; i < 4; i++) {
            if(i==whichCorner){
                ofSetColor(255, 0, 0);
            }else{
                ofSetColor(255, 255, 0);
            }
            ofDrawRectangle(corners[i], 10, 10);
        }
        ofPopStyle();
    }
}
//--------------------------------------------------------------
void ofxGLWarper::save(string saveFile){


    ofXml xml;
    saveToXml(xml);
    xml.save(saveFile);
    ofLogNotice() << "saved ofGLWarper to " << saveFile;

//	ofxXmlSettings XML;
//	saveToXml(XML);
//    /*
//    XML.clear();
//	XML.addTag("corners");
//	XML.pushTag("corners");
//	
//	
//	for(int i =0; i<4; i++){
//		int t = XML.addTag("corner");
//		XML.setValue("corner:x",corners[i].x, t);
//		XML.setValue("corner:y",corners[i].y, t);
//	}
//    //*/
//	XML.saveFile(saveFile);
}

void ofxGLWarper::saveToXml(ofXml &xml) {
    xml.clear();
    
    xml.addChild("corners");
    xml.setTo("corners");
    
    for (int i = 0; i < 4; i++) {
        ofXml point;
        point.addChild("point");
        point.setTo("point");
        point.addValue("x", corners[i].x);
        point.addValue("y", corners[i].y);
        xml.addXml(point);
    }
}

//--------------------------------------------------------------
void ofxGLWarper::load(string loadFile){

    ofXml xml;
    bool loaded = xml.load(loadFile);
    if (loaded) {
        loadFromXml(xml);
    }
    else {
        ofLogError() << "can't open " << loadFile;
    }
}

void ofxGLWarper::loadFromXml(ofXml &xml) {
    
    
    if (xml.setTo("corners")) {
        if (xml.getNumChildren() < 4) {
            ofLogError() << "ofxGLWarper : incorrrect xml formating. less than 4 \"corner\" tags found";
            return;
        }
        else {
            // all ok, let's load
            for (int i = 0; i < 4; i++) {
                xml.setTo("//corners");
                xml.setToChild(i);
                corners[i].x = xml.getFloatValue("x");
                corners[i].y = xml.getFloatValue("y");
            }
            
            processMatrices();
            
            ofLogNotice() << "ofxGLWarper : xml object loaded OK!";
        }
    }
    else {
        ofLogError() << "ofxGLWarper : incorrrect xml formating. No \"corners\" tag found";
    }
}


//--------------------------------------------------------------
void ofxGLWarper::mouseDragged(ofMouseEventArgs &args){

	//float scaleX = (float)args.x / width;
	//float scaleY = (float)args.y / height;
	
	if(whichCorner >= 0 && cornerSelected){
	//	corners[whichCorner].x = scaleX;
	//	corners[whichCorner].y = scaleY;
        corners[whichCorner].x = args.x;
		corners[whichCorner].y = args.y;
		
        CornerLocation location = (CornerLocation)whichCorner;
        ofNotifyEvent(changeEvent, location, this);
	}
}
//--------------------------------------------------------------
void ofxGLWarper::mousePressed(ofMouseEventArgs &args){
	
	float smallestDist = sqrt(ofGetWidth() * ofGetWidth() + ofGetHeight() * ofGetHeight());
	//whichCorner = -1;
	float sensFactor = cornerSensibility * sqrt( width  * width  + height  * height );
   // cout << "sens factor " << sensFactor << endl;
    cornerSelected = false;
	for(int i = 0; i < 4; i++){
		float distx = corners[i].x - (float)args.x;
		float disty = corners[i].y - (float)args.y;
		float dist  = sqrt( distx * distx + disty * disty);

		if(dist < smallestDist && dist < sensFactor ){
			whichCorner = i;
			smallestDist = dist;
            cornerSelected=true;
		}
	}
}
//--------------------------------------------------------------
void ofxGLWarper::mouseReleased(ofMouseEventArgs &args){
	//whichCorner = -1;
}
//--------------------------------------------------------------
void ofxGLWarper::mouseMoved(ofMouseEventArgs &args){
}
//--------------------------------------------------------------
void ofxGLWarper::mouseScrolled(ofMouseEventArgs &args){}
//--------------------------------------------------------------
void ofxGLWarper::keyPressed(ofKeyEventArgs &args){
        switch (args.key) {
                if (whichCorner>=0 && cornerSelected) {
            case OF_KEY_DOWN:
                corners[whichCorner].y++;
                break;
            case OF_KEY_UP:
                corners[whichCorner].y--;
                break;
            case OF_KEY_LEFT:
                corners[whichCorner].x--;
                break;
            case OF_KEY_RIGHT:
                corners[whichCorner].x++;            
                break;
                }
            default:
                break;
    }
}
//--------------------------------------------------------------
void ofxGLWarper::keyReleased(ofKeyEventArgs &args){}
//--------------------------------------------------------------
ofVec4f ofxGLWarper::fromScreenToWarpCoord(float x, float y, float z){
	ofVec4f mousePoint;
	ofVec4f warpedPoint;
	
	// this is the point on the image which i want to know the coordinates inside the warped system ... 
	mousePoint.x = x;
	mousePoint.y = y;
	mousePoint.z = 0.0;
	mousePoint.w = 1.0;
	
	// i create a ofMatrix4x4 with the ofxGLWarper myMatrixData in column order
	ofMatrix4x4 myOFmatrix = ofMatrix4x4(myMatrix[0], myMatrix[4],myMatrix[8], myMatrix[12],
										 myMatrix[1], myMatrix[5],myMatrix[9], myMatrix[13],
										 myMatrix[2], myMatrix[6],myMatrix[10],myMatrix[14],
										 myMatrix[3], myMatrix[7],myMatrix[11],myMatrix[15]);
	// do not invert the matrix 
	ofMatrix4x4 invertedMyMatrix = myOFmatrix.getInverse();	
	//ofMatrix4x4 invertedMyMatrix = myOFmatrix;
	
	// multiply both to get the point transformed by the matrix
	warpedPoint = invertedMyMatrix * mousePoint ;
	
	// we need to normalize the value as described here : http://tech.groups.yahoo.com/group/OpenCV/message/80121
	warpedPoint.x = warpedPoint.x / warpedPoint.w;
	warpedPoint.y = warpedPoint.y / warpedPoint.w;
	warpedPoint.z = warpedPoint.z / warpedPoint.w;
	
//	return warpedPoint;
    
    float matrix[9];
    matrix[0] = myMatrix[0];
    matrix[1] = myMatrix[4];
    matrix[2] = myMatrix[12];
                         
    matrix[3] = myMatrix[1];
    matrix[4] = myMatrix[5];
    matrix[5] = myMatrix[13];

    matrix[6] = myMatrix[3];
    matrix[7] = myMatrix[7];
    matrix[8] = myMatrix[15];
    
    ofMatrix3x3 ofMatrix(matrix[0], matrix[1], matrix[2],
                         matrix[3], matrix[4], matrix[7],
                         matrix[6], matrix[7], matrix[8]);
    
//    ofVec3f p(x, y, 1.0);

    ofMatrix.invert();
    
//    ofVec3f p2(x * ofMatrix[0] + y * ofMatrix[3] + ofMatrix[6],
//               x * ofMatrix[1] + y * ofMatrix[4] + ofMatrix[7],
//               x * ofMatrix[2] + y * ofMatrix[5] + ofMatrix[8]);
    
    ofVec3f p2(x * ofMatrix[0] + y * ofMatrix[1] + ofMatrix[2],
               x * ofMatrix[3] + y * ofMatrix[4] + ofMatrix[5],
               x * ofMatrix[6] + y * ofMatrix[7] + ofMatrix[8]);
//
    
    return ofVec4f(p2.x / p2.z, p2.y / p2.z, 0.0, 0.0);
    
}

//--------------------------------------------------------------
ofVec4f ofxGLWarper::fromWarpToScreenCoord(float x, float y, float z){
	ofVec4f mousePoint;
	ofVec4f warpedPoint;
	
	// this is the point inside the warped system which i want to know the coordinates on the image  ... 
	mousePoint.x = x;
	mousePoint.y = y;
	mousePoint.z = 0.0;
	mousePoint.w = 1.0;
	
	// i create a ofMatrix4x4 with the ofxGLWarper myMatrixData in column order
	ofMatrix4x4 myOFmatrix = ofMatrix4x4(myMatrix[0], myMatrix[4],myMatrix[8],myMatrix[12],
										 myMatrix[1], myMatrix[5],myMatrix[9], myMatrix[13],
										 myMatrix[2], myMatrix[6],myMatrix[10],myMatrix[14],
										 myMatrix[3],myMatrix[7],myMatrix[11],myMatrix[15]);
	// invert the matrix 
	//ofMatrix4x4 invertedMyMatrix = myOFmatrix.getInverse();	
	ofMatrix4x4 invertedMyMatrix = myOFmatrix;
	
	// multiply both to get the point transformed by the matrix
	warpedPoint = invertedMyMatrix * mousePoint ;
	
	warpedPoint.x = warpedPoint.x / warpedPoint.w;
	warpedPoint.y = warpedPoint.y / warpedPoint.w;
	warpedPoint.z = warpedPoint.z / warpedPoint.w;
	
	return warpedPoint;
}
//--------------------------------------------------------------
void ofxGLWarper::setCorner(CornerLocation cornerLocation, ofPoint screenLocation){
    corners[cornerLocation] = screenLocation;// / ofPoint(width, height, 1);
    processMatrices();

    CornerLocation location = cornerLocation;
    ofNotifyEvent(changeEvent, location, this);
}
//--------------------------------------------------------------
ofPoint ofxGLWarper::getCorner(CornerLocation cornerLocation){
    return corners[cornerLocation];// * ofPoint(width, height, 1);
}
//--------------------------------------------------------------
void ofxGLWarper::setCornerSensibility(float sensibility){
    cornerSensibility = sensibility;
}
//--------------------------------------------------------------
float ofxGLWarper::getCornerSensibility(){
    return cornerSensibility;
}