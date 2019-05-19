#include <ESP8266WiFi.h>
#include <NTPtimeESP.h>
#include <FirebaseArduino.h>

NTPtime NTPch("ch.pool.ntp.org"); 
strDateTime dateTime; 

#define FIREBASE_HOST "hanghang-2dc20.firebaseio.com"
#define FIREBASE_AUTH "aoa8aiOhqtkWioNfg0ADOCInzolTMXrAiLjL9Mez"
#define WIFI_SSID "Ahihi"
#define WIFI_PASSWORD "123456789"

String x, DATE, TIME;
int id, type, count, idMax;

/*--------------------- SETUP --------------------*/
void setup() {
  Serial.begin(115200);
  while (!Serial) {}
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("connecting");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println();
  Serial.print("connected: ");
  Serial.println(WiFi.localIP());
  Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH); 
  
  // set id max
  x = "VDK/idMax";
  idMax = Firebase.getFloat(x);
  if (idMax == 0) Firebase.setFloat(x,0);
  else Serial.println (idMax);
  
  // set infor stranger
  addNewInfor(0,"unknow","unknow","unknow");
  x = "VDK/0/count_history";
  count = Firebase.getFloat(x);
  if (count == 0) Firebase.setFloat(x,0);
  else Serial.println (count);
}



/*--------------------- LOOP --------------------*/

void loop() {
  
  Serial.print("Nhan : ");
  type = getNumSerial();
  Serial.println(type); 
  dateTime = NTPch.getNTPtime(7.0, 0);

  if (type==5){
      delay(30);
      idMax = Firebase.getFloat("VDK/idMax");
      sendNumSerial(idMax);
  } else {
      DATE = gettDate();
      TIME = gettTime();
      if (DATE.length()!=10 || TIME.length()!=8) return;
      Serial.print("DATE : "); Serial.println(DATE); 
      Serial.print("TIME : "); Serial.println(TIME); 
      if (type == 0){ // stranger
          x = "VDK/0/count_history";
          count = Firebase.getFloat(x)+1;
          Firebase.setFloat(x,count);
          setHistory(0,"Try to open door",count);
          
      } else if (type==4) { // delete all
          idMax = Firebase.getFloat("VDK/idMax");
          Serial.print("idMax :");
          Serial.println(idMax);
          for (int i=1; i<=idMax; i++)
          {
              x = "VDK/" + (String)(i) + "/count_history";
              count = Firebase.getFloat(x)+1;
              Firebase.setFloat(x,count);
              
              Serial.print ("Xoa : ");
              Serial.println (i);
              x = "VDK/" + (String)(i) + "/infor/status";
              Firebase.setString(x, "resigned");
              setHistory(i,"Delete fingerprint",count);
          }
          
      } else {
          
          Serial.print("Nhan : ");
          id = getNumSerial();
          Serial.println(id);
          
          if (type == 1){ // add 
              addNewInfor(id,"X","X","working");
              setHistory(id,"Add fingerprint",1);
              x = "VDK/" + (String)(id) + "/count_history";
              Firebase.setFloat(x,1);
              Firebase.setFloat("VDK/idMax",id);
          
          } else if (type == 2){ // delete one
              x = "VDK/" + (String)(id) + "/count_history";
              count = Firebase.getFloat(x)+1;
              Firebase.setFloat(x,count);
              
              x = "VDK/" + (String)(id) + "/infor/status";
              Firebase.setString(x, "resigned");
              setHistory(id,"Delete fingerprint",count);
          }

          else { // open door
              x = "VDK/" + (String)(id) + "/count_history";
              count = Firebase.getFloat(x)+1;
              Firebase.setFloat(x,count);
              setHistory(id,"Open door",count);
              
          }
      }
      // Kiem tra ket noi firebase
      if (Firebase.failed()) {
        Serial.print("setting /number failed:");
        Serial.println(Firebase.error());
        return;
      }
        Serial.println("Sent");
      }
}

/*--------------------- ADD NEW --------------------*/

void addNewInfor(int id, String s1, String s2, String s3){
    Serial.print("addNewInfor");Serial.println(id);
    x = "VDK/" + (String)(id) + "/infor/fullName";
    Firebase.setString(x, s1);
    x = "VDK/" + (String)(id) + "/infor/class";
    Firebase.setString(x, s2);
    x = "VDK/" + (String)(id) + "/infor/status";
    Firebase.setString(x, s3);
}

/*--------------------- SET HISTORY --------------------*/

void setHistory(int id, String s, int count){
    Serial.print("setHistory");Serial.println(id);
    x = "VDK/" + (String)(id) + "/history/" + (String)(count) + "/date";   
    Firebase.setString(x, DATE);
    x = "VDK/" + (String)(id) + "/history/" + (String)(count) + "/time";   
    Firebase.setString(x, TIME);
    x = "VDK/" + (String)(id) + "/history/" + (String)(count) + "/type";   
    Firebase.setString(x,s);
}

/*--------------------- SEND DATA --------------------*/
void sendNumSerial(int x){
  Serial.print("Gui : ");
  Serial.print("#");
  Serial.print(x); 
  Serial.print("@");
  Serial.print("\n"); 
}

/*--------------------- GET DATE --------------------*/

String gettDate (){
    DATE = "";
    if (dateTime.day<10) DATE += "0";
    DATE += ((String)(dateTime.day)+"/");
    if (dateTime.month<10) DATE += "0";
    DATE += ((String)(dateTime.month)+"/"+(String)(dateTime.year));
    return DATE;
}

/*--------------------- GET TIME --------------------*/
String gettTime (){
    TIME = "";
    if (dateTime.hour<10) TIME += "0";
    TIME += ((String)(dateTime.hour)+":");
    if (dateTime.minute<10) TIME += "0";
    TIME += ((String)(dateTime.minute)+":");
    if (dateTime.second<10) TIME += "0";
    TIME += (String)(dateTime.second);
    return TIME;
}

/*--------------------- GET DATA --------------------*/
int getNumSerial(){
  int start_flag = 0;
  int s = 0;
  while (1) {
    while (Serial.available()) {
      int x = Serial.read();
      if (x=='#'){
        s=0;
        start_flag = 1;
      } else if (start_flag == 1){
        if (x=='@'){
          start_flag = 0;
          return s;
        } else {
          s=s*10+(x-48);
        }
      } 
    }
  }
}
