#define DS3231_I2C_ADDRESS 104
#include <dht11.h>
#define DHT11PIN 9
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <SoftwareSerial.h>
#include <MsTimer2.h>
#define bTx 8
#define bRx 7
#define btnPin 2
#define Buzzer 6
LiquidCrystal_I2C lcd(0x27, 16, 2);
SoftwareSerial btSerial(bRx, bTx);
String s1 = "";
String host = "http://api.thingspeak.com";
String url = "/update?api_key=";
//String key = "Input your Thingspeak_Channel_API_Key";
String key = "Input your Thingspeak_Channel_API_Key";
String field1 = "&field1=";
String field2 = "&field2=";
String field3 = "&field3=";
dht11 DHT11;
String sender = "";//ThingSpeak에 전달용 문자열
int delaytime = 0; //ThingSPeak Delay변수
int counter = 0; //스톱워치용 시간 변수
boolean interrupt = false; //버튼입력을 구분하는 변수
volatile boolean mode = false; //HW인터럽트 변수
volatile boolean countmode = false; //스톱워치 확인용 변수
boolean once = false; //시간 저장 체크용 변수
byte seconds, minutes, hours, day, date, month, year;
byte savehours, savemin; //시간 저장용 변수
void btnHandler() //HW인터럽트 함수
{
  mode= !mode;
}
//get3231Date code by http://deneb21.tistory.com/327
void get3231Date()
{
  Wire.beginTransmission(DS3231_I2C_ADDRESS);
  Wire.write(0x00);
  Wire.endTransmission();
  Wire.requestFrom(DS3231_I2C_ADDRESS, 7);
  if (Wire.available()) {
    seconds = Wire.read(); // get seconds
    minutes = Wire.read(); // get minutes
    hours   = Wire.read();   // get hours
    day     = Wire.read();
    date    = Wire.read();
    month   = Wire.read(); //temp month
    year    = Wire.read();

    seconds = (((seconds & B11110000) >> 4) * 10 + (seconds & B00001111)); // convert BCD to decimal
    minutes = (((minutes & B11110000) >> 4) * 10 + (minutes & B00001111)); // convert BCD to decimal
    hours   = (((hours & B00110000) >> 4) * 10 + (hours & B00001111)); // convert BCD to decimal (assume 24 hour mode)
    day     = (day & B00000111); // 1-7
    date    = (((date & B00110000) >> 4) * 10 + (date & B00001111)); // 1-31
    month   = (((month & B00010000) >> 4) * 10 + (month & B00001111)); //msb7 is century overflow
    year    = (((year & B11110000) >> 4) * 10 + (year & B00001111));
  }
  else
  {
    //oh noes, no data!
  }
}
void stopwatch() //SW인터럽트 함수
{
  if(counter > 0)
    counter--;
  delaytime++;
}
void setup()
{
  Wire.begin();
  Serial.begin(115200);
  btSerial.begin(9600);
  lcd.init();
  lcd.backlight();
  attachInterrupt(0, btnHandler, FALLING);
  pinMode(btnPin, INPUT);
  pinMode(Buzzer, OUTPUT);
  MsTimer2::set(1000, stopwatch);//SW인터럽트 설정
  MsTimer2::start();//시작!
}
void loop()
{
  get3231Date();
  lcd.clear();
  if(mode == false)//초기는 무조건 false 누르면 true
  {
    if(interrupt == true)
    {
      if(savehours > hours)
        hours += 24;
      if(savemin > minutes)
      {
        hours -= 1; minutes += 60;
      }
      byte caltime = (hours - savehours)*60 + (minutes-savemin);
      String sender = host + url + key + field3 + caltime;
      Serial.print(sender);//분단위로 체크 후 전송
      interrupt = false;
      once = false;
    }
  }
  else
  {
    if(interrupt == true)
    {
      lcd.setCursor(14, 0);
      lcd.print("OK");
    }
    else
    {
      //savemin = minutes;sender
      //savehours = hours;
      lcd.setCursor(14, 0);
      lcd.print("OK");
      interrupt = true;
    }
  }
  if(interrupt == true && once == false) //시간 저장용
  {
    once = true;
    savemin = minutes;
    savehours = hours;
  }
  if (counter == 0 && countmode == false) //스톱워치 설정용
  {
    while (btSerial.available())
    {
      char myChar = (char)btSerial.read();
      s1 += myChar;;
      delay(5);
    }
    if (!s1.equals(""))
    {
      counter = s1.toInt();
      countmode = true;
      s1 = "";
    }
  }
  else if (counter == 0 && countmode == true)
  {
    btSerial.println(9999); countmode = false;
    digitalWrite(Buzzer, HIGH);
    lcd.setCursor(0, 0);
    lcd.print("StopWatch END");
    delay(2000);
    digitalWrite(Buzzer, LOW);
  }
  int chk = DHT11.read(DHT11PIN);
  //btSerial.print("습도 :");
  String timer = "20" + (String)year + "-" + hours + ":" + minutes + ":" + seconds;
  float tem, h;
  tem = (float)DHT11.temperature;
  h = (float)DHT11.humidity;
  btSerial.print(tem, 2);
  btSerial.print("\n");
  btSerial.print("\n");
  btSerial.print(h, 2);
  lcd.setCursor(0, 0);
  lcd.print(timer);
  lcd.setCursor(0, 1);
  String output = (String)tem + "^C " + (String)h + "%";
  lcd.print(output);
  if (delaytime == 15)
  {
    sender = host + url + key + field1 + tem + field2 + h;
    Serial.print(sender);
    delaytime = 0;
  }
  delay(1000);
}
