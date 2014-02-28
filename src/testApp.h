#pragma once

#include "ofMain.h"
#include "ofxOsc.h"
#include "ofxTimer.h"

#define NUM_MSG_STRINGS 15

class testApp : public ofBaseApp{

public:
  void setup();
  void update();
  void draw();

  void keyPressed(int key);
  void keyReleased(int key);
  void mouseMoved(int x, int y );
  void mouseDragged(int x, int y, int button);
  void mousePressed(int x, int y, int button);
  void mouseReleased(int x, int y, int button);
  void windowResized(int w, int h);
  void dragEvent(ofDragInfo dragInfo);
  void gotMessage(ofMessage msg);

  void proceedLevel( int upDown , int _nextLevel );
  void reset();
  void stepTo(unsigned int distenation, unsigned char speed);
  
private:
  ofSerial  serial;
  
  bool      bSendSerialMessage;			// a flag for sending serial
  char      bytesRead[3];				// data from serial, we will be trying to read 3
  char      bytesReadString[4];			// a string needs a null terminator, so we need 3 + 1 bytes
  int       nBytesRead;					// how much did we read?
  int       nTimesRead;					// how many times did we read?
  float     readTime;					// when did we last read?
  ofTrueTypeFont		font;
  
  ofxOscReceiver reciver;
  bool      run;
  bool      isReset;
  bool      busy;
  
  int       current_msg_string;
  string    msg_strings[NUM_MSG_STRINGS];
  
  int       currentPosition;
  string    currentStatus;
};
