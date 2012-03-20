#include "DodecaudionOSCBridgeApp.h"

//--------------------------------------------------------------
void DodecaudionOSCBridgeApp::setup(){
    //Default values
    serialPortBaudRateCurrent = SERIAL_DEFAULT_BAUDRATE;
    serialBuffer = "";
    serialPortSelected = false;
    isCalibrating = false;
    drawPlots = false;
    
    calibrationStartFrame = 0;
    calibrationFrameLimit = 1200;
    
    for( int i = 0 ; i < DODECAUDION_VALUES_COUNT ; i++ ){
        dodecaudionValues.push_back( 0.0f );
        dodecaudionValuesPrev.push_back( 0.0f );
        dodecaudionValuesCalibrationOffset.push_back( 0.0f );
    }

        
    initGUI();    
    initOSC(OSC_DEFAULT_HOST , OSC_DEFAULT_PORT);
}



//--------------------------------------------------------------
void DodecaudionOSCBridgeApp::update(){
    
    //read serial data
    if( serialPortSelected && serial.available() > 0 ){        
        unsigned char ch;
        
        while( serial.available() > 0 ){
            ch = serial.readByte();
            if( ch == '\n' ){
                parseSerialDataInto( serialBuffer , dodecaudionValues );
                serialBuffer = "";
            }else{
                serialBuffer += ch;
                if( serialBuffer.length() > 128 ){
                    serialBuffer = "";
                    serial.flush();
                }
            }
        }        
    }
    
    
    if( isCalibrating ){
        updateDodecaudion();
        updateDodecaudionCalibration();
    }else{
        updateDodecaudion();
        oscSendValues();
    }
    
}



//--------------------------------------------------------------
void DodecaudionOSCBridgeApp::draw()
{
    if( drawPlots ){
        
    }
}

//--------------------------------------------------------------
void DodecaudionOSCBridgeApp::keyPressed(int key){
    std::cout << key << std::endl;
    switch (key) {
        case 9: //TAB
            gui->toggleVisible();
            break;            
        default:
            break;
    }
}

//--------------------------------------------------------------
void DodecaudionOSCBridgeApp::keyReleased(int key){

}

//--------------------------------------------------------------
void DodecaudionOSCBridgeApp::mouseMoved(int x, int y ){

}

//--------------------------------------------------------------
void DodecaudionOSCBridgeApp::mouseDragged(int x, int y, int button){

}

//--------------------------------------------------------------
void DodecaudionOSCBridgeApp::mousePressed(int x, int y, int button){

}

//--------------------------------------------------------------
void DodecaudionOSCBridgeApp::mouseReleased(int x, int y, int button){

}

//--------------------------------------------------------------
void DodecaudionOSCBridgeApp::windowResized(int w, int h){

}

//--------------------------------------------------------------
void DodecaudionOSCBridgeApp::gotMessage(ofMessage msg){

}

//--------------------------------------------------------------
void DodecaudionOSCBridgeApp::dragEvent(ofDragInfo dragInfo){ 

}

//--------------------------------------------------------------
void DodecaudionOSCBridgeApp::exit()
{
	delete gui;
    serial.close();
}

//--------------------------------------------------------------
// GUI
//--------------------------------------------------------------

/**
 *
 */
void DodecaudionOSCBridgeApp::initGUI()
{
    //list all available serial ports
    vector<string> serialPortsAvailable;
    serialPortsAvailable.push_back("");
    vector <ofSerialDeviceInfo> deviceList = serial.getDeviceList();
    for( vector <ofSerialDeviceInfo>::iterator it = deviceList.begin() ; it != deviceList.end() ; ++it ){
        string devicePath = it->getDevicePath();
#ifdef __APPLE__
        string filterString = "tty.Dodecaudion";
        size_t found = devicePath.find(filterString);
        if( found==string::npos ){
            continue;
        }
#endif
        serialPortsAvailable.push_back( it->getDevicePath() );
    }
    
    
    gui = new ofxUICanvas(0,0,380,640);		
    
    //Serial port related
    gui->addWidgetDown(new ofxUILabel("Serial port", OFX_UI_FONT_LARGE)); 
    gui->addWidgetDown(new ofxUIDropDownList(UI_FORM_ITEM_WIDTH_DEFAULT, UI_SERIAL_PORT_SELECT_DROPDOWN_NAME, serialPortsAvailable, OFX_UI_FONT_MEDIUM));  
    
    //OSC related
    gui->addWidgetDown(new ofxUILabel("OSC settings", OFX_UI_FONT_LARGE)); 
    ofxUITextInput *oscHostTextInput = new ofxUITextInput(UI_FORM_ITEM_WIDTH_DEFAULT , UI_OSC_HOST_TEXTINPUT_NAME , OSC_DEFAULT_HOST , OFX_UI_FONT_MEDIUM);
    oscHostTextInput->setAutoClear(false);
    gui->addWidgetDown(oscHostTextInput);
    
    ofxUITextInput *oscPortTextInput = new ofxUITextInput(UI_FORM_ITEM_WIDTH_DEFAULT , UI_OSC_PORT_TEXTINPUT_NAME , ofToString(OSC_DEFAULT_PORT) , OFX_UI_FONT_MEDIUM);
    oscPortTextInput->setAutoClear(false);
    gui->addWidgetDown(oscPortTextInput);
    
    //Dodecaudion value indicators
    gui->addWidgetDown(new ofxUILabel("Dodecaudion Values", OFX_UI_FONT_LARGE));
    gui->addWidgetDown(new ofxUISpacer(UI_FORM_ITEM_WIDTH_DEFAULT, 2)); 
    
    
    ofxUIRangeSlider *slider = new ofxUIRangeSlider(UI_FORM_ITEM_WIDTH_DEFAULT_VERTICAL, UI_FORM_ITEM_HEIGHT_DEFAULT, 0, 1.0 , 0 , 1.0, ofToString(1));
    dodecaudionValueIndicators.push_back(slider);
    gui->addWidgetDown(slider);    
    for( int i = 1 ; i < DODECAUDION_VALUES_COUNT ; i++ ){
        slider = new ofxUIRangeSlider(UI_FORM_ITEM_WIDTH_DEFAULT_VERTICAL, UI_FORM_ITEM_HEIGHT_DEFAULT, 0, 1.0, 0.0, 1.0, ofToString(i+1));
        dodecaudionValueIndicators.push_back(slider);
        gui->addWidgetRight(slider); 
    }
    /*
    ofxUISlider *slider = new ofxUISlider(10.0,160, 0.0, 255.0, 150, ofToString(1));
    dodecaudionValueIndicators.push_back(slider);
    gui->addWidgetDown(slider); 
    for( int i = 1 ; i < DODECAUDION_VALUES_COUNT ; i++ ){
        slider = new ofxUISlider(10.0,160, 0.0, 1.0, 0.0, ofToString(i+1));
        dodecaudionValueIndicators.push_back(slider);
        gui->addWidgetRight(slider); 
    }
    */
    
    dodecaudionCalibrateToggle = (new ofxUIToggle(10.0,10.0, false, UI_DODECAUDION_CALIBRATE_TOGGLE_NAME ));
    gui->addWidgetDown(dodecaudionCalibrateToggle);
    //gui->addWidgetDown(new ofxUIToggle(10.0,10.0, false, UI_DODECAUDION_DRAW_PLOTS_TOGGLE_NAME ));
    
    ofAddListener(gui->newGUIEvent, this, &DodecaudionOSCBridgeApp::guiEvent);     
}


/**
 *
 */
void DodecaudionOSCBridgeApp::guiEvent(ofxUIEventArgs &e)
{
    std::cout << e.widget->getID() << " / " << e.widget->getParent()->getName() << " / " << e.widget->getName() <<  std::endl;
      
    //init new serial port
    if(e.widget->getParent()->getName() == UI_SERIAL_PORT_SELECT_DROPDOWN_NAME){
        ofxUIDropDownList *dropdown = (ofxUIDropDownList *) e.widget;
        string serialPath = e.widget->getName();
        initSerialPort(serialPath, serialPortBaudRateCurrent);
        //dropdown->setToggleVisibility( false );
    }
    
    if( e.widget->getName() == UI_DODECAUDION_CALIBRATE_TOGGLE_NAME ){
        ofxUIToggle *toggle = (ofxUIToggle *) e.widget;
        isCalibrating = toggle->getValue();
        calibrationStartFrame = ofGetFrameNum();
    }
    if( e.widget->getName() == UI_DODECAUDION_DRAW_PLOTS_TOGGLE_NAME ){
        ofxUIToggle *toggle = (ofxUIToggle *) e.widget;
        drawPlots = toggle->getValue();
    }
    if( e.widget->getName() == UI_OSC_HOST_TEXTINPUT_NAME ){
        ofxUITextInput *textfield = (ofxUITextInput *) e.widget;
        string oscHostToSet = textfield->getTextString();
        initOSC(oscHostToSet, oscPortCurrent);
    }
    if( e.widget->getName() == UI_OSC_PORT_TEXTINPUT_NAME ){
        ofxUITextInput *textfield = (ofxUITextInput *) e.widget;
        int oscPortToSet = atoi(textfield->getTextString().c_str());
        initOSC(oscHostCurrent, oscPortToSet);
    }

}


/**
 *
 */
void DodecaudionOSCBridgeApp::guiDodecaudionIndicatorSetToWallValue( int wallId )
{
    if( wallId < dodecaudionValues.size() ){
        dodecaudionValueIndicators[wallId]->setValueLow( dodecaudionValuesCalibrationOffset[wallId] );
        dodecaudionValueIndicators[wallId]->setValueHigh( dodecaudionValuesCalibrationOffset[wallId] + dodecaudionValues[wallId] );
        //dodecaudionValueIndicators[wallId]->setValue( dodecaudionValues[wallId] );
    }
}


//--------------------------------------------------------------
// Serial
//--------------------------------------------------------------


/**
 *
 */
void DodecaudionOSCBridgeApp::initSerialPort(string serialPortToInitialize,int serialPortBaudRateToInitialize)
{
    if( serialPortCurrent.compare(serialPortToInitialize) != 0 || serialPortBaudRateCurrent != serialPortBaudRateToInitialize ){
        serialPortCurrent = serialPortToInitialize;
        serialPortBaudRateCurrent = serialPortBaudRateToInitialize;

        if( serialPortSelected == true ){
            serial.close();
        }
        
        serialPortSelected = false;
        if( serialPortToInitialize.compare( "" ) != 0 ){
            std::cout << serialPortCurrent << std::endl; 
            serial.setup(serialPortCurrent, serialPortBaudRateCurrent);
            serial.writeByte('\n');
            serialPortSelected = true;
        }
    }
}


/**
 *
 */
void DodecaudionOSCBridgeApp::parseSerialDataInto(string serialBuffer, vector<float> &values)
{
    
    if( serialBuffer.length() >= 59 ){
        vector<string> parts = ofSplitString( serialBuffer , "," );
        if( parts.size() >= values.size() ){
            for( int i = 0 ; i < values.size() ; i++ ){
                float val = values[i];
                float valNew = dodecaudionValueCalc( i , atof(parts[i].c_str()) );
                values[i] = valNew;
                dodecaudionValues[i] = valNew;
            }
        }
    }
}



//--------------------------------------------------------------
// OSC
//--------------------------------------------------------------

/**
 *
 */
void DodecaudionOSCBridgeApp::initOSC(string oscHostToInitialize, int oscPortToInitialize)
{
    if( oscHostCurrent.compare(oscHostToInitialize) != 0 || oscPortCurrent != oscPortToInitialize ){
        oscHostCurrent = oscHostToInitialize;
        oscPortCurrent = oscPortToInitialize;
        
        oscSender.setup( oscHostCurrent , oscPortCurrent );
        std::cout << "OSC :: host = " << oscHostCurrent << " port = " << oscPortCurrent << std::endl;
    }
}

void DodecaudionOSCBridgeApp::oscSendWallValue( int wallId )
{
    if( wallId < dodecaudionValues.size() ){
        ofxOscMessage msg;
        msg.setAddress(OSC_ADDRESS_WALL);
        msg.addFloatArg(dodecaudionValues[wallId]);
        oscSender.sendMessage(msg);
    }
}


void DodecaudionOSCBridgeApp::oscSendValues()
{
    ofxOscMessage msg1,msg2;
    msg1.setAddress(OSC_ADDRESS_INSTRUMENT1);
    msg1.addFloatArg(dodecaudionValues[1]);
    msg1.addFloatArg(dodecaudionValues[3]);
    msg1.addFloatArg(dodecaudionValues[5]);
    msg1.addFloatArg(dodecaudionValues[7]);
    msg1.addFloatArg(dodecaudionValues[9]);
    msg1.addFloatArg(dodecaudionValues[11]);

    msg2.setAddress(OSC_ADDRESS_INSTRUMENT2);
    msg2.addFloatArg(dodecaudionValues[0]);
    msg2.addFloatArg(dodecaudionValues[2]);
    msg2.addFloatArg(dodecaudionValues[4]);
    msg2.addFloatArg(dodecaudionValues[6]);
    msg2.addFloatArg(dodecaudionValues[8]);
    msg2.addFloatArg(dodecaudionValues[10]);

    std::cout << "msg1: " << msg1.getAddress() << " " << msg1.getArgAsFloat(0) << " " << msg1.getArgAsFloat(1) << " " << msg1.getArgAsFloat(2) << " " << msg1.getArgAsFloat(3) << " " << msg1.getArgAsFloat(4) << " " << msg1.getArgAsFloat(5) << std::endl;

    std::cout << "msg2: " << msg2.getAddress() << " " << msg2.getArgAsFloat(0) << " " << msg2.getArgAsFloat(1) << " " << msg2.getArgAsFloat(2) << " " << msg2.getArgAsFloat(3) << " " << msg2.getArgAsFloat(4) << " " << msg2.getArgAsFloat(5) << std::endl;
    
    oscSender.sendMessage(msg1);
    oscSender.sendMessage(msg2);
}


//--------------------------------------------------------------
// Dodecaudion
//--------------------------------------------------------------


/**
 *
 */
void DodecaudionOSCBridgeApp::updateDodecaudion()
{
    //check if values have changed
    for( int i = 0 ; i < dodecaudionValues.size() ; i++ ){
        if( dodecaudionValues[i] != dodecaudionValuesPrev[i] ){
            dodecaudionValueChanged(i);
            dodecaudionValuesPrev[i] = dodecaudionValues[i]; 
        }
    }
}


/**
 *
 */
void DodecaudionOSCBridgeApp::updateDodecaudionCalibration()
{
    //std::cout << "calibrating " << ( ofGetFrameNum() - calibrationStartFrame ) << " / " << calibrationFrameLimit << std::endl;
    
    if( calibrationFrameLimit >= ( ofGetFrameNum() - calibrationStartFrame ) ){
        
        //first frame of calibration - clear previous readings
        if( isFirstCalibratingFrame == true ){
            for( int wallId = 0 ; wallId < dodecaudionValues.size() ; wallId++ ){
                dodecaudionValuesCalibrationOffset[wallId] = 0;
            }
            isFirstCalibratingFrame = false;
        }
        
        for( int wallId = 0 ; wallId < dodecaudionValues.size() ; wallId++ ){
            dodecaudionValuesCalibrationOffset[wallId] = 0.5 * dodecaudionValuesCalibrationOffset[wallId] + 0.5 * MAX( dodecaudionValuesCalibrationOffset[wallId] , dodecaudionValues[wallId] );
            //std::cout << dodecaudionValuesCalibrationOffset[wallId] << " " << dodecaudionValuesCalibrationOffset[wallId] << " " << dodecaudionValues[wallId] << std::endl;
        }
    }else{
        isCalibrating = false;
        isFirstCalibratingFrame = true;
        dodecaudionCalibrateToggle->setValue(isCalibrating);
    
        std::cout << "Calibration finished" << std::endl;
        for( int wallId = 0 ; wallId < dodecaudionValues.size() ; wallId++ ){
            //dodecaudionValuesCalibrationOffset[wallId] = ( dodecaudionValuesCalibrationOffset[wallId] + dodecaudionValues[wallId] ) / 2.0;
            std::cout << "WallOffset for #" << wallId << " = " << dodecaudionValuesCalibrationOffset[wallId] << std::endl; 
        }
    }
}



/**
 *
 */
float DodecaudionOSCBridgeApp::dodecaudionValueCalc( int wallId , float newValue )
{
    float ret = newValue;
    float signalTimeMinLength = 3.0;
    
    if( wallId < dodecaudionValues.size() ){
        float valueOffset = dodecaudionValuesCalibrationOffset[wallId];
        ret = MIN( 1.0 , (1.0 + valueOffset) * MAX( 0 , newValue - valueOffset ) );
    }
    return ret;
}

/**
 *
 */
void DodecaudionOSCBridgeApp::dodecaudionValueChanged( int wallId )
{
    if( wallId < dodecaudionValues.size() ){
        guiDodecaudionIndicatorSetToWallValue( wallId );
        oscSendWallValue(wallId);
    }
}

