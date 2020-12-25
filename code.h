#include <SoftwareSerial.h>
//Rx terhubung ke Tx pada modul mp3 player
//Tx terhubung ke Rx pada modul mp3 player
SoftwareSerial mp3(3,4);//Rx,Tx

static int8_t Send_buf[8] = {0}; // Buffer untuk kirim perintah. 
static uint8_t ansbuf[10] = {0}; // Buffer untuk respon.   

String mp3Answer;           // respon dari MP3.

String sanswer(void);
String sbyte2hex(uint8_t b);

int trig_pin = 7;
int echo_pin = 6;
int iteration = 0;
long echotime;
float distance;

/**** Command byte ************/
//dari tabel pada manual book
#define CMD_NEXT_SONG     0x01  // Play next song.
#define CMD_PREV_SONG     0x02  // Play previous song.
#define CMD_PLAY_W_INDEX  0x03
#define CMD_VOLUME_UP     0x04
#define CMD_VOLUME_DOWN   0x05
#define CMD_SET_VOLUME    0x06

#define CMD_SNG_CYCL_PLAY 0x08  // Single Cycle Play.
#define CMD_SEL_DEV       0x09
#define CMD_SLEEP_MODE    0x0A
#define CMD_WAKE_UP       0x0B
#define CMD_RESET         0x0C
#define CMD_PLAY          0x0D
#define CMD_PAUSE         0x0E
#define CMD_PLAY_FOLDER_FILE 0x0F

#define CMD_STOP_PLAY     0x16  // Stop playing continuously. 
#define CMD_FOLDER_CYCLE  0x17
#define CMD_SHUFFLE_PLAY  0x18 //
#define CMD_SET_SNGL_CYCL 0x19 // Set single cycle.

#define CMD_SET_DAC 0x1A
#define DAC_ON  0x00
#define DAC_OFF 0x01

#define CMD_PLAY_W_VOL    0x22
#define CMD_PLAYING_N     0x4C
#define CMD_QUERY_STATUS      0x42
#define CMD_QUERY_VOLUME      0x43
#define CMD_QUERY_FLDR_TRACKS 0x4e
#define CMD_QUERY_TOT_TRACKS  0x48
#define CMD_QUERY_FLDR_COUNT  0x4f

/**** Opitons **********/
#define DEV_TF            0x02  

/***********************/

void setup(){
  Serial.begin(9600);
 pinMode(trig_pin, OUTPUT);
  pinMode(echo_pin, INPUT);
  digitalWrite(trig_pin, LOW);  
  mp3.begin(9600);
  
  sendCommand(CMD_VOLUME_DOWN);
  sendCommand(CMD_VOLUME_DOWN);
   sendCommand(CMD_VOLUME_DOWN);
  sendCommand(CMD_VOLUME_DOWN);
  
  
  
}


void loop(){
  digitalWrite(trig_pin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trig_pin, LOW);
  echotime= pulseIn(echo_pin, HIGH);
  distance= 0.0001*((float)echotime*340.0)/2.0;
  iteration++;
  
  Serial.print(distance);
  Serial.println(" cm");  
  Serial.print(iteration);
  Serial.println(" baris");  
  if(iteration>15){
  if(distance<12){
  sendCommand(CMD_NEXT_SONG);
   delay(7000);
   iteration =0;
  }
  }
 
  
  
}

void printHelp(){
    Serial.println("HELP  ");
    Serial.println(" h = Print again this Massage");
    Serial.println(" p = Play");
    Serial.println(" P = Pause");
    Serial.println(" > = Next");
    Serial.println(" < = Previous");
    Serial.println(" s = Stop Play"); 
    Serial.println(" + = Volume UP");
    Serial.println(" - = Volume DOWN");
    Serial.println(" c = Query current file");
    Serial.println(" q = Query status");
    Serial.println(" v = Query volume");
    Serial.println(" x = Query folder count");
    Serial.println(" t = Query total file count");
    Serial.println(" f = Play folder 1.");
    Serial.println(" S = Sleep");
    Serial.println(" W = Wake up");
    Serial.println(" r = Reset");
  }
//perintah dari karakter pada serial monitor
void sendMP3Command(char c) {
  switch (c) {
    case '?':
    case 'h':
      printHelp();
      break;

    case 'p':
      Serial.println("Play ");
      sendCommand(CMD_PLAY);
      break;

    case 'P':
      Serial.println("Pause");
      sendCommand(CMD_PAUSE);
      break;

    case '>':
      Serial.println("Next");
      sendCommand(CMD_NEXT_SONG);
      sendCommand(CMD_PLAYING_N); // cek nomor file yang di play
      break;

    case '<':
      Serial.println("Previous");
      sendCommand(CMD_PREV_SONG);
      sendCommand(CMD_PLAYING_N); // cek nomor file yang di play
      break;

    case 's':
      Serial.println("Stop Play");
      sendCommand(CMD_STOP_PLAY);
      break;


    case '+':
      Serial.println("Volume Up");
      sendCommand(CMD_VOLUME_UP);
      break;

    case '-':
      Serial.println("Volume Down");
      sendCommand(CMD_VOLUME_DOWN);
      break;

    case 'c':
      Serial.println("Query current file");
      sendCommand(CMD_PLAYING_N);
      break;

    case 'q':
      Serial.println("Query status");
      sendCommand(CMD_QUERY_STATUS);
      break;

    case 'v':
      Serial.println("Query volume");
      sendCommand(CMD_QUERY_VOLUME);
      break;

    case 'x':
      Serial.println("Query folder count");
      sendCommand(CMD_QUERY_FLDR_COUNT);
      break;

    case 't':
      Serial.println("Query total file count");
      sendCommand(CMD_QUERY_TOT_TRACKS);
      break;

    case 'f':
      Serial.println("Playing folder 1");
      sendCommand(CMD_FOLDER_CYCLE, 1, 0);
      break;

    case 'S':
      Serial.println("Sleep");
      sendCommand(CMD_SLEEP_MODE);
      break;

    case 'W':
      Serial.println("Wake up");
      sendCommand(CMD_WAKE_UP);
      break;

    case 'r':
      Serial.println("Reset");
      sendCommand(CMD_RESET);
      break;
  }
}


//arti respon dari modul
String decodeMP3Answer() {
  String decodedMP3Answer = "";

  decodedMP3Answer += sanswer();

  switch (ansbuf[3]) {
    case 0x3A:
      decodedMP3Answer += " -> Memory card inserted.";
      break;

    case 0x3D:
      decodedMP3Answer += " -> Completed play num " + String(ansbuf[6], DEC);
      break;

    case 0x40:
      decodedMP3Answer += " -> Error";
      break;

    case 0x41:
      decodedMP3Answer += " -> Data recived correctly. ";
      break;

    case 0x42:
      decodedMP3Answer += " -> Status playing: " + String(ansbuf[6], DEC);
      break;

    case 0x48:
      decodedMP3Answer += " -> File count: " + String(ansbuf[6], DEC);
      break;

    case 0x4C:
      decodedMP3Answer += " -> Playing: " + String(ansbuf[6], DEC);
      break;

    case 0x4E:
      decodedMP3Answer += " -> Folder file count: " + String(ansbuf[6], DEC);
      break;

    case 0x4F:
      decodedMP3Answer += " -> Folder count: " + String(ansbuf[6], DEC);
      break;
  }

  return decodedMP3Answer;
}

//kirim perintah 
void sendCommand(byte command){
  sendCommand(command, 0, 0);
}

void sendCommand(byte command, byte dat1, byte dat2){
  delay(20);
  Send_buf[0] = 0x7E;    //
  Send_buf[1] = 0xFF;    //
  Send_buf[2] = 0x06;    // Len
  Send_buf[3] = command; //
  Send_buf[4] = 0x01;    // 0x00 tanpa respon, 0x01 ada respon
  Send_buf[5] = dat1;    // data1
  Send_buf[6] = dat2;    // data2
  Send_buf[7] = 0xEF;    //
  Serial.print("Sending: ");
  for (uint8_t i = 0; i < 8; i++){
    mp3.write(Send_buf[i]) ;
    Serial.print(sbyte2hex(Send_buf[i]));
  }
  Serial.println();
}


//konversi byte ke hex
String sbyte2hex(uint8_t b){
  String shex;

  shex = "0X";

  if (b < 16) shex += "0";
  shex += String(b, HEX);
  shex += " ";
  return shex;
}


//konversi hex ke interger
int shex2int(char *s, int n){
  int r = 0;
  for (int i=0; i<n; i++){
     if(s[i]>='0' && s[i]<='9'){
      r *= 16; 
      r +=s[i]-'0';
     }else if(s[i]>='A' && s[i]<='F'){
      r *= 16;
      r += (s[i] - 'A') + 10;
     }
  }
  return r;
}


/****************************/
/*Function: sanswer. Returns a String answer from mp3 UART module.          */
/*Parameter:- uint8_t b. void.                                                  */
/*Return: String. If the answer is well formated answer.                        */

String sanswer(void){
  uint8_t i = 0;
  String mp3answer = "";

  // Get only 10 Bytes
  while (mp3.available() && (i < 10)){
    uint8_t b = mp3.read();
    ansbuf[i] = b;
    i++;

    mp3answer += sbyte2hex(b);
  }

  // if the answer format is correct.
  if ((ansbuf[0] == 0x7E) && (ansbuf[9] == 0xEF)){
    return mp3answer;
  }

  return "???: " + mp3answer;
}