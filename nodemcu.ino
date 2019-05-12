#include <SoftwareSerial.h>
#include <ESP8266WiFi.h>
#include <NTPtimeESP.h>
#include <FirebaseArduino.h>

SoftwareSerial NodeMCU(D2,D3);
NTPtime NTPch("ch.pool.ntp.org"); 
strDateTime dateTime; 

#define FIREBASE_HOST "hanghang-2dc20.firebaseio.com"
#define FIREBASE_AUTH "aoa8aiOhqtkWioNfg0ADOCInzolTMXrAiLjL9Mez"
#define WIFI_SSID "Ahihi"
#define WIFI_PASSWORD "123456789"
 
 
void setup()
{
  Serial.begin(9600);
  NodeMCU.begin(4800);
  pinMode(D2,INPUT);
  pinMode(D3,OUTPUT);
 
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
}
 
int cnt[130];
String x,DATE,TIME;
int id,type;

String gettDate (){
    DATE = "";
    if (dateTime.day<10) DATE += "0";
    DATE += ((String)(dateTime.day)+"/");
    if (dateTime.month<10) DATE += "0";
    DATE += ((String)(dateTime.month)+"/"+(String)(dateTime.year));
    return DATE;
}
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


void loop()
{
  
  while(NodeMCU.available()>0)
  {
      type = NodeMCU.parseFloat();
      dateTime = NTPch.getNTPtime(7.0, 0);
      if(NodeMCU.read()== '\n')
      {
          id = NodeMCU.parseFloat();
          if(NodeMCU.read()== '\n')
          {
                Serial.print("TYPE: ");
                Serial.println(type);
                Serial.print("ID: ");
                Serial.println(id);
                TIME = gettTime();
                Serial.println(TIME);
                DATE = gettDate();
                Serial.println(DATE);
                if (TIME.length()!=8 || DATE.length()!=10) 
                {
                  Serial.print("return");
                  return;
                }
                
                // id = 0 = > nguoi la
                if (id == 0)
                {
                    cnt[id]++;
                    x = "VDK/" + (String)(id) + "/infor/fullName";
                    Firebase.setString(x, "unknow");
                    x = "VDK/" + (String)(id) + "/infor/class";
                    Firebase.setString(x, "unknow");
                    x = "VDK/" + (String)(id) + "/infor/status";
                    Firebase.setString(x, "unknow");
                    x = "VDK/" + (String)(id) + "/history/" + (String)(cnt[id]) + "/date";   
                    Firebase.setString(x, DATE);
                    x = "VDK/" + (String)(id) + "/history/" + (String)(cnt[id]) + "/time";   
                    Firebase.setString(x, TIME);
                    x = "VDK/" + (String)(id) + "/history/" + (String)(cnt[id]) + "/type";   
                    Firebase.setString(x, "Try to open door");
                }
                // type = 1 => tao 1 id moi (name=X,class=X,status=working)  
                else if (type==1)
                {
                    cnt[id]++;
                    x = "VDK/" + (String)(id) + "/infor/fullName";
                    Firebase.setString(x, "X");
                    x = "VDK/" + (String)(id) + "/infor/class";
                    Firebase.setString(x, "X");
                     x = "VDK/" + (String)(id) + "/infor/status";
                    Firebase.setString(x, "working");
                    x = "VDK/" + (String)(id) + "/history/" + (String)(cnt[id]) + "/date";   
                    Firebase.setString(x, DATE);
                    x = "VDK/" + (String)(id) + "/history/" + (String)(cnt[id]) + "/time";   
                    Firebase.setString(x, TIME);
                    x = "VDK/" + (String)(id) + "/history/" + (String)(cnt[id]) + "/type";   
                    Firebase.setString(x, "Add fingerprint"); 
                }
                // type = 2 => xoa id tren R305 va set status=resigned
                else if (type==2)
                {
                    cnt[id]++;
                    x = "VDK/" + (String)(id) + "/infor/status";
                    Firebase.setString(x, "resigned");
                    x = "VDK/" + (String)(id) + "/history/" + (String)(cnt[id]) + "/date";   
                    Firebase.setString(x, DATE);
                    x = "VDK/" + (String)(id) + "/history/" + (String)(cnt[id]) + "/time";   
                    Firebase.setString(x, TIME);
                    x = "VDK/" + (String)(id) + "/history/" + (String)(cnt[id]) + "/type";   
                    Firebase.setString(x, "Delete fingerprint");
                }
                else if (type==4)
                {
                    for (int i=1; i<=id; i++)
                    {
                        cnt[i]++;
                        Serial.print ("Xoa : ");
                        Serial.println (i);
                        x = "VDK/" + (String)(i) + "/infor/status";
                        Firebase.setString(x, "resigned");
                        x = "VDK/" + (String)(i) + "/history/" + (String)(cnt[i]) + "/date";   
                        Firebase.setString(x, DATE);
                        x = "VDK/" + (String)(i) + "/history/" + (String)(cnt[i]) + "/time";   
                        Firebase.setString(x, TIME);
                        x = "VDK/" + (String)(i) + "/history/" + (String)(cnt[i]) + "/type";   
                        Firebase.setString(x, "Delete fingerprint");
                    }
                } 
                else 
                {
                    cnt[id]++;
                    x = "VDK/" + (String)(id) + "/history/" + (String)(cnt[id]) + "/date";   
                    Firebase.setString(x, DATE);
                    x = "VDK/" + (String)(id) + "/history/" + (String)(cnt[id]) + "/time";   
                    Firebase.setString(x, TIME);
                    x = "VDK/" + (String)(id) + "/history/" + (String)(cnt[id]) + "/type";   
                    if (id!=0 && type==3) Firebase.setString(x, "Open door");
                }
                  
                // Kiem tra ket noi firebase
                if (Firebase.failed()) 
                {
                  Serial.print("setting /number failed:");
                  Serial.println(Firebase.error());
                  return;
                }
                Serial.println("Sent");
          }
        
      }
    }
}
