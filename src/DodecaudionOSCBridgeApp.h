#pragma once

#include "ofMain.h"

#include "ofxUI.h"
#include "ofxOSC.h"

#include <iostream>
#include <algorithm>

#define UI_FORM_ITEM_WIDTH_DEFAULT 200
#define UI_OSC_HOST_TEXTINPUT_NAME "OSC Host"
#define UI_OSC_PORT_TEXTINPUT_NAME "OSC Port"
#define UI_DODECAUDION_CALIBRATE_TOGGLE_NAME "Calibrate walls"
#define UI_DODECAUDION_DRAW_PLOTS_TOGGLE_NAME "Draw plots"

#define UI_SERIAL_PORT_SELECT_DROPDOWN_NAME "Serial Port"

#define SERIAL_DEFAULT_BAUDRATE 115200
#define SERIAL_BUFFER_SIZE 4

#define OSC_DEFAULT_HOST "localhost"
#define OSC_DEFAULT_PORT 12000
#define OSC_ADDRESS_INSTRUMENT1 "/Instrument01"
#define OSC_ADDRESS_INSTRUMENT2 "/Instrument02"
#define OSC_ADDRESS_WALL "/w"


#define DODECAUDION_VALUES_COUNT 12

class DodecaudionOSCBridgeApp : public ofBaseApp{

	public:
		void setup();
        void initGUI();
		void update();
        void updateDodecaudion();
        void updateDodecaudionCalibration();
		void draw();

		void keyPressed  (int key);
		void keyReleased(int key);
		void mouseMoved(int x, int y );
		void mouseDragged(int x, int y, int button);
		void mousePressed(int x, int y, int button);
		void mouseReleased(int x, int y, int button);
		void windowResized(int w, int h);
		void dragEvent(ofDragInfo dragInfo);
		void gotMessage(ofMessage msg);
    
        void exit(); 
        void guiEvent(ofxUIEventArgs &e);
        void guiDodecaudionIndicatorSetToWallValue( int wallId );
    
        void initSerialPort(string serialPortToInitialize,int serialPortBaudRateToInitialize);
        
        void initOSC(string oscHostToInitialize, int oscPortToInitialize);
        void oscSendValues();
        void oscSendWallValue( int wallId );

        void parseSerialDataInto(string serialBuffer, vector<float> &values);
        float dodecaudionValueCalc( int wallId , float newValue );
        void dodecaudionValueChanged( int wallId );    

    
        bool drawPlots;
    
        ofxUICanvas *gui;		
        vector<ofxUISlider *> dodecaudionValueIndicators;
        ofxUIToggle *dodecaudionCalibrateToggle;
    
        string serialPortCurrent;
        int serialPortBaudRateCurrent;
        ofSerial serial;
        bool serialPortSelected;
        string serialBuffer;
    
        ofxOscSender oscSender;
        int oscPortCurrent;
        string oscHostCurrent;
    
        vector<float> dodecaudionValues,dodecaudionValuesPrev,dodecaudionValuesCalibrationOffset;
        bool isCalibrating;
        int calibrationFrameLimit,calibrationStartFrame;
        
    
        
};
