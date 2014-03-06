#include "testApp.h"

#define LISTEN_PORT 5100
#define SEND_PORT 5000
#define BAUD_RATE 9600
#define HOST_NAME "127.0.0.1"

bool togBusyBlink = false;
//--------------------------------------------------------------
void testApp::setup(){
  current_msg_string = 0;
  
  settingXml.loadFile("settings.xml");
  
  string portName = settingXml.getValue("settings:serial-port", "/dev/tty.usbmodem1411");
  string hostName = settingXml.getValue("settings:host", HOST_NAME);
  int baudRate = settingXml.getValue("settings:baud-rate", BAUD_RATE);
  int listenPort = settingXml.getValue("settings:listen-port", LISTEN_PORT);
  int sendPort = settingXml.getValue("settings:send-port", SEND_PORT);
  
  cout << "host : "<< hostName <<"listen port at: "<<listenPort << "sendport at: "<< sendPort<<endl;

  busy = false;
  isReset = false;
  serial.enumerateDevices();
  serial.setup(portName, baudRate);
  
  reciver.setup(listenPort);

  sender.setup(hostName, sendPort);
  
  nTimesRead = 0;
	nBytesRead = 0;
	readTime = 0;
  currentPosition = 0;
	memset(bytesReadString, 0, 7);
  
  dispatch_after(dispatch_time(DISPATCH_TIME_NOW, 5 * NSEC_PER_SEC), dispatch_get_current_queue(), ^{//do reset after 5 seconds
  });
  
  font.loadFont("DIN.otf", 14);
  currentStatus = "";
}

//--------------------------------------------------------------
void testApp::update(){
  
  for(int i = 0; i < NUM_MSG_STRINGS; i++){
    msg_strings[i] = "";
	}
  
  while (reciver.hasWaitingMessages() ) {
    string msg_string;
    ofxOscMessage m;
    reciver.getNextMessage( &m );
    
    if("/step" == m.getAddress()) {
      
      if( false == busy ) {
        cout << "recieved osc....."<< endl;
        int destinationValue = m.getArgAsInt32(0);
        int speedVlue = m.getArgAsInt32(1);
        stepTo(destinationValue, speedVlue);
        busy = true;
        
        msg_string += "Messag Step :" + ofToString(destinationValue) + " speed: " + ofToString(speedVlue);
      }
    } else if( "/reset" == m.getAddress() ){
      if( false == busy ) {
        reset();
        busy = true;
        msg_string += "Message Reset..." ;
      }
    }
    
    // add to the list of strings to display
    msg_strings[current_msg_string] = msg_string;
    current_msg_string = (current_msg_string + 1) % NUM_MSG_STRINGS;
    
  }
  
  ///////////////// serial read ///////////////////////////////
  
  
  if (bSendSerialMessage) {
		
		nTimesRead = 0;
		nBytesRead = 0;
		int nRead  = 0;  // a temp variable to keep count per read
		
		unsigned char bytesReturned[6];
		
		memset(bytesReadString, 0, 7);
		memset(bytesReturned, 0, 6);
		
		while( (nRead = serial.readBytes(bytesReturned, 6)) > 0){
			nTimesRead++;
			nBytesRead = nRead;
		};

		memcpy(bytesReadString, bytesReturned, 6);
    if ('X' == bytesReturned[0] && 'X' == bytesReturned[1]) {
      unsigned char stateByte = bytesReturned[2];
      if ('C' == stateByte) {
        cout << "status normal" << endl;
        int h_pos = (unsigned int) bytesReturned[3];
        int l_pos = (unsigned int) bytesReturned[4];
        currentPosition = h_pos * 256 + l_pos;
        sendMsg(READY);
      } else if ('N' ==stateByte) {
        cout << "--------- status LIMIT NEAR------" << endl;
        int h_pos = (unsigned int) bytesReturned[3];
        int l_pos = (unsigned int) bytesReturned[4];
        currentPosition = h_pos * 256 + l_pos;
        sendMsg(LIMIT_NEAR);
      } else if ('F' ==stateByte) {
        cout << "--------- status LIMIT FAR------" << endl;
        int h_pos = (unsigned int) bytesReturned[3];
        int l_pos = (unsigned int) bytesReturned[4];
        currentPosition = h_pos * 256 + l_pos;
        sendMsg(LIMIT_FAR);
      };  
      cout << currentPosition << endl;
      bSendSerialMessage = false;
      busy = false;
    }
		
		readTime = ofGetElapsedTimef();
	}
  
  /////////////////////////////////////////////////////////////
  
  if (busy) {
    currentStatus = "BUSY.";
  } else {
    currentStatus = "READY.";
  }
}

//--------------------------------------------------------------
void testApp::draw(){
  
  ofBackground(0);
  
  if (nBytesRead > 0 && ((ofGetElapsedTimef() - readTime) < 0.125f)){
		ofSetColor(0);
	} else {
		ofSetColor(220);
	}
	string msg;
  msg += "Current Status : " + currentStatus + "\n";
	msg += "Current Position : " + ofToString(currentPosition) +"\n";
	msg += "nBytes read " + ofToString(nBytesRead) + "\n";
	msg += "nTimes read " + ofToString(nTimesRead) + "\n";
	msg += "read packet : " + ofToString(bytesReadString) + "\n";
	msg += "(time to pass : " + ofToString(readTime, 3) + ")";
	font.drawString(msg, 10, 30);
  
  ofSetColor(255, 255, 255);
  
	for(int i = 0; i < NUM_MSG_STRINGS; i++){
    font.drawString(msg_strings[i], 10, 185 + 15 * i);
	}

}

//--------------------------------------------------------------
void testApp::reset() {
  unsigned char speed = 25;
  unsigned int distenation = 0;
  ofResetElapsedTimeCounter();
  busy = true;
  
  unsigned char buf[7];
  
  unsigned char h_dist = (unsigned char)(distenation >> 8);
  unsigned char l_dist = (unsigned char)(distenation % 256);
  
  buf[0] = 'x';
  buf[1] = 'x';
  buf[2] = 'r';
  buf[3] = speed;
  buf[4] = h_dist;
  buf[5] = l_dist;
  buf[6] = '\n';
  
  int hvoer =((int)h_dist * 256 + (int)l_dist);
  cout << (int)buf[4] << " "  << (int)buf[5] << " " << hvoer <<endl;
  
  serial.writeBytes( buf , sizeof(buf) / sizeof(buf[0]) );
  bSendSerialMessage = true;
};


//--------------------------------------------------------------
void testApp::stepTo(unsigned int distenation, unsigned char speed) {
  ofResetElapsedTimeCounter();
  busy = true;
  
  unsigned char buf[7];
  
  unsigned char h_dist = (unsigned char)(distenation >> 8);
  unsigned char l_dist = (unsigned char)(distenation % 256);
  
  buf[0] = 'x';
  buf[1] = 'x';
  buf[2] = 'f';
  buf[3] = speed;
  buf[4] = h_dist;
  buf[5] = l_dist;
  buf[6] = '\n';
  
  int hvoer =((int)h_dist * 256 + (int)l_dist);
  cout << "packets..: "<<(int)buf[4] << " "  << (int)buf[5] << " " << hvoer <<endl;
  
  serial.writeBytes( buf , sizeof(buf) / sizeof(buf[0]) );
  bSendSerialMessage = true;
};

//--------------------------------------------------------------
void testApp::sendMsg(osc_messagee_type type) {
  ofxOscMessage message;
  switch (type) {
    case 0:
      message.setAddress("/ready");
      message.addIntArg(currentPosition);
      break;
    case 1:
      message.setAddress("/limit_near");
      message.addIntArg(currentPosition);
      break;
    case 2:
      message.setAddress("/limit_far");
      message.addIntArg(currentPosition);
      break;
    default:
      break;
  };
  
  sender.sendMessage(message);
  cout << "sendmsg.." << endl;
}

//--------------------------------------------------------------
void testApp::keyPressed(int key){
  
  unsigned char buf[7];
  unsigned int distenation;
  if(key == '1'){
    stepTo(120, 12);
  } else if(key == '2') {
    stepTo(220, 25);
  } else if(key == '3') {
    stepTo(1200, 3);
  } else if(key == '4') {
    stepTo(1310, 20);
  } else if(key == '0') {
    reset();
  }

}

//--------------------------------------------------------------
void testApp::keyReleased(int key){
  
}

//--------------------------------------------------------------
void testApp::mouseMoved(int x, int y ){
  
}

//--------------------------------------------------------------
void testApp::mouseDragged(int x, int y, int button){
  
}

//--------------------------------------------------------------
void testApp::mousePressed(int x, int y, int button){
  
}

//--------------------------------------------------------------
void testApp::mouseReleased(int x, int y, int button){
  
}

//--------------------------------------------------------------
void testApp::windowResized(int w, int h){
  
}

//--------------------------------------------------------------
void testApp::gotMessage(ofMessage msg){
  
}

//--------------------------------------------------------------
void testApp::dragEvent(ofDragInfo dragInfo){
  
}