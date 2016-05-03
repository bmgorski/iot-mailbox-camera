/***************************************************
  This is a code for the TTL JPEG Camera (VC0706 chipset)
  Camera Functions are adopted from VC0706 library
  Written by Limor Fried/Ladyada for Adafruit Industries.
  -----> https://github.com/adafruit/Adafruit-VC0706-Serial-Camera-Library

  Modified by Brian Gorski
  -----> https://github.com/bmgorski/iot-mailbox-camera

****************************************************/

#define VC0706_RESET  0x26
#define VC0706_GEN_VERSION 0x11
#define VC0706_SET_PORT 0x24
#define VC0706_READ_FBUF 0x32
#define VC0706_GET_FBUF_LEN 0x34
#define VC0706_FBUF_CTRL 0x36
#define VC0706_DOWNSIZE_CTRL 0x54
#define VC0706_DOWNSIZE_STATUS 0x55
#define VC0706_READ_DATA 0x30
#define VC0706_WRITE_DATA 0x31
#define VC0706_COMM_MOTION_CTRL 0x37
#define VC0706_COMM_MOTION_STATUS 0x38
#define VC0706_COMM_MOTION_DETECTED 0x39
#define VC0706_MOTION_CTRL 0x42
#define VC0706_MOTION_STATUS 0x43
#define VC0706_TVOUT_CTRL 0x44
#define VC0706_OSD_ADD_CHAR 0x45

#define VC0706_STOPCURRENTFRAME 0x0
#define VC0706_STOPNEXTFRAME 0x1
#define VC0706_RESUMEFRAME 0x3
#define VC0706_STEPFRAME 0x2

#define VC0706_640x480 0x00
#define VC0706_320x240 0x11
#define VC0706_160x120 0x22

#define VC0706_MOTIONCONTROL 0x0
#define VC0706_UARTMOTION 0x01
#define VC0706_ACTIVATEMOTION 0x01

#define VC0706_SET_ZOOM 0x52
#define VC0706_GET_ZOOM 0x53

#define CAMERABUFFSIZ 100
#define CAMERADELAY 10
// Camera Setting
uint8_t  serialNum;
uint8_t  camerabuff[CAMERABUFFSIZ+1];
uint8_t  bufferLen;
uint16_t frameptr;
int sensorPin = D0;
int ledPin = D1;
int sensorInput = 0;

// TCP Setting
TCPClient client;
byte server[] = {192, 168, 1, 232};  // Add your Server IP
byte c;
int port = 8080;
int led  = D7;
int sparkTempF = 0;

//---------------------- Low Level Commands
void common_init(void) {
  frameptr  = 0;
  bufferLen = 0;
  serialNum = 0;
}

void sendCommand(uint8_t cmd, uint8_t args[] = 0, uint8_t argn = 0) {
    Serial1.write((byte)0x56);
    Serial1.write((byte)serialNum);
    Serial1.write((byte)cmd);

    for (uint8_t i=0; i<argn; i++) {
      Serial1.write((byte)args[i]);
      //Serial.print(" 0x");
      //Serial.print(args[i], HEX);
    }

}
uint8_t readResponse(uint8_t numbytes, uint8_t timeout) {
  uint8_t counter = 0;
  bufferLen = 0;
  int avail;

  while ((timeout != counter) && (bufferLen != numbytes)){
    avail = Serial1.available();
    if (avail <= 0) {
      delay(1);
      counter++;
      continue;
    }
    counter = 0;
    // there's a byte!
    camerabuff[bufferLen++] = Serial1.read();
  }
  //printBuff();
//camerabuff[bufferLen] = 0;
//Spark.publish("INFO",(char*)camerabuff);
  return bufferLen;
}

boolean verifyResponse(uint8_t command) {
  if ((camerabuff[0] != 0x76) ||
      (camerabuff[1] != serialNum) ||
      (camerabuff[2] != command) ||
      (camerabuff[3] != 0x0))
      return false;
  return true;

}

boolean runCommand(uint8_t cmd, uint8_t *args, uint8_t argn,
			   uint8_t resplen, boolean flushflag) {
  // flush out anything in the buffer?
  if (flushflag) {
    readResponse(100, 10);
  }

  sendCommand(cmd, args, argn);
  if (readResponse(resplen, 200) != resplen)
    return false;
  if (! verifyResponse(cmd))
    return false;
  return true;
}


//---------- Camera Functions

boolean camReset() {
  uint8_t args[] = {0x0};

  return runCommand(VC0706_RESET, args, 1, 5, true);
}


boolean camBegin(uint16_t baud) {
  Serial1.begin(baud);
  return camReset();
}
uint8_t camGetImageSize() {
  uint8_t args[] = {0x4, 0x4, 0x1, 0x00, 0x19};
  if (! runCommand(VC0706_READ_DATA, args, sizeof(args), 6, true))
    return -1;

  return camerabuff[5];
}

boolean camSetImageSize(uint8_t x) {
  uint8_t args[] = {0x05, 0x04, 0x01, 0x00, 0x19, x};

  return runCommand(VC0706_WRITE_DATA, args, sizeof(args), 5, true);
}

boolean cameraFrameBuffCtrl(uint8_t command) {
  uint8_t args[] = {0x1, command};
  return runCommand(VC0706_FBUF_CTRL, args, sizeof(args), 5, false);
}

boolean camTakePicture() {
  frameptr = 0;
  return cameraFrameBuffCtrl(VC0706_STOPCURRENTFRAME);
}

boolean camRsumeVideo() {
  return cameraFrameBuffCtrl(VC0706_RESUMEFRAME);
}

uint32_t camFrameLength(void) {
  uint8_t args[] = {0x01, 0x00};
  if (!runCommand(VC0706_GET_FBUF_LEN, args, sizeof(args), 9, true))
    return 0;

  uint32_t len;
  len = camerabuff[5];
  len <<= 8;
  len |= camerabuff[6];
  len <<= 8;
  len |= camerabuff[7];
  len <<= 8;
  len |= camerabuff[8];

  return len;
}

uint8_t camAvailable(void) {
  return bufferLen;
}


uint8_t * camReadPicture(uint8_t n) {
  uint8_t args[] = {0x0C, 0x0, 0x0A,
                    0, 0, frameptr >> 8, frameptr & 0xFF,
                    0, 0, 0, n,
                    CAMERADELAY >> 8, CAMERADELAY & 0xFF};

  if (! runCommand(VC0706_READ_FBUF, args, sizeof(args), 5, false))
    return 0;


  // read into the buffer PACKETLEN!
  if (readResponse(n+5, CAMERADELAY) == 0)
      return 0;


  frameptr += n;

  return camerabuff;
}


//----------- Setup the MicroController
void setup() {

  pinMode(sensorPin, INPUT);
  pinMode(ledPin, OUTPUT);

  Spark.function("camera", cameraFunc);
//  Spark.function("blink", blinkFunc);
//  Spark.variable("temperature", &sparkTempF, INT);

//  Spark.publish("INFO","VC0706 Camera snapshot test");
    // Begin Camera
      common_init();
    // Try to locate the camera
    if (camBegin(38400)) {
     //Spark.publish("INFO","Camera Found:");
    } else {
     //Spark.publish("INFO","No camera found?");
      return;
    }


}

// call the below function when the POST request matches it
int cameraFunc(String command) {
  //Spark.publish("INFO","Got the request...");
  // Set the picture size - you can choose one of 640x480, 320x240 or 160x120
  // Remember that bigger pictures take longer to transmit!
  //  camSetImageSize(VC0706_640x480);        // biggest
      camSetImageSize(VC0706_320x240);        // medium
  //  camSetImageSize(VC0706_160x120);          // small

  // You can read the size back from the camera (optional, but maybe useful?)
  uint8_t imgsize = camGetImageSize();
  //Spark.publish("INFO","Image size: ");
  //if (imgsize == VC0706_640x480) Spark.publish("INFO","640x480");
  //if (imgsize == VC0706_320x240) Spark.publish("INFO","320x240");
  //if (imgsize == VC0706_160x120) Spark.publish("INFO","160x120");

  if (! camTakePicture()){
    //Spark.publish("INFO","Failed to snap!");
  }else{
    //Spark.publish("INFO","Picture taken!");
  }
  // Get the size of the image (frame) taken
  uint16_t jpglen = camFrameLength();
  //Spark.publish("INFO","Storing ");
  //Spark.publish("INFO",jpglen);
  //Spark.publish("INFO"," byte image.");

  //--- TCP

// Prepare request
  String start_request = "";
  String end_request = "";
  String header = "";

  start_request = start_request + "------AaB03x\r\n"
    +"Content-Disposition: form-data; name=\"name\"\r\n\r\n"
    +"HOLYSHIT.JPG\r\n" + "------AaB03x\r\n"
    +"Content-Disposition: form-data; name=\"file\"; filename=\"CAM.JPG\"\r\n"
    +"Content-Type: image/jpg\r\n"
    +"Content-Transfer-Encoding: binary\r\n\r\n";


  end_request = end_request +"\r\n------AaB03x--";

  uint16_t extra_length;
  extra_length = start_request.length() + end_request.length();

  uint16_t len = jpglen + extra_length;

  header = header + "POST /cameras/img HTTP/1.1\r\n"
                  + "Host: 192.168.1.232:8080\r\n"
                  + "Connection: keep-alive\r\n"
                  + "Content-Length: " + len + "\r\n"
                  + "Cache-Control: max-age=0\r\n"
                  + "Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\r\n"
                  + "User-Agent: Spark MailBox\n"
                  + "Content-Type: multipart/form-data; boundary=----AaB03x\r\n"
                  + "Accept-Encoding: gzip, deflate\r\n"
                  + "Accept-Language: en-US,en;q=0.8\r\n\r\n";

  //Spark.publish("DEBUG","Starting connection to server...");

  if (command == "takePics") {
   //Spark.publish("DEBUG","Take Picture...");

  if (client.connect(server, port))
    {
    client.print(header);
    client.print(start_request);
    int32_t time = millis();
    // Read all the data up to # bytes!
    byte wCount = 0; // For counting # of writes
    while (jpglen > 0) {
      // read 32 bytes at a time;
      uint8_t *buffer;
      uint8_t bytesToRead = min(64, jpglen); // change 32 to 64 for a speedup but may not work with all setups!
      buffer = camReadPicture(bytesToRead);
      client.write(buffer, bytesToRead);
      jpglen -= bytesToRead;
    }

    client.print(end_request);

   Spark.publish("INFO","Transmission over");
   time = millis() - time;
    client.stop();
    camRsumeVideo();
    return 0;  // Picture success
  }else {
    return -1; // Failed
  }

  }

}

bool takingPicture = false;

void loop() {

    sensorInput = digitalRead(sensorPin);

     if (sensorInput == LOW) {         // check if the input is HIGH (button released)
        digitalWrite(ledPin, LOW);  // turn LED OFF
        takingPicture = false;
     } else {
        digitalWrite(ledPin, HIGH);// turn LED ON
        if(takingPicture==false){
           takingPicture=true;
           cameraFunc("takePics");
        }
     }
}