#include <Wire.h>
#include <EEPROM.h>
#include <PCF8574.h>
#include <MPU6050.h>

#define baud_rate 9600              // Bluetooth communication speed
#define expander_adress 0x20        // I2C expander adress
#define R_PWM 2                     // Pin number - right motor PWM
#define L_PWM 3                     // Pin number - right motor PWM
#define RR 5                        // Pin number - right motor back   
#define RF 6                        // Pin number - right motor forward                
#define LF 8                        // Pin number - left motor forward                
#define LR 9                        // Pin number - left motor back                  
#define sensor_transistor 11        // Pin number - transistor for front sensors board                          
#define buzzer 12                   // Pin number - buzzer
//Front ultrasonic sensor pins           
#define ultrasonic_front_trig 51                                  
#define ultrasonic_front_echo 50 
//Left ultrasonic sensor pins               
#define ultrasonic_left_trig 38                
#define ultrasonic_left_echo 39
//Right ultrasonic sensor pins           
#define ultrasonic_right_trig 34               
#define ultrasonic_right_echo 35 
// Pin number - light sensor                     
#define optoresistor 31             

//Rotation times to avoid obstacle without gyro              
#define time_first 500                    
#define time_second 500                               
#define time_third 550                        
#define time_fourth 700                     
#define time_fiveth 400                       
#define time_1 550                            
#define time_2 750                            

//Default angle values for avoiding obstacle with gyro
short kat_1 = 70;                 // EEPROM 4
short kat_2 = 70;                 // EEPROM 5
short kat_3 = 40;                 // EEPROM 6

//Default motor parameters
short lewy = 200;                 // EEPROM 1
short prawy = 200;                // EEPROM 2
float mnoznik_zawracania = 100;   // EEPROM 3

//Default distance triggering obstacle presence
short odleglosc = 10;             // EEPROM 7

short lewo_zwrot = 180;
short prawo_zwrot = 180;

PCF8574 expander;
MPU6050 mpu; 

//==============================================================================

void setup() 
{  
  Serial.begin(baud_rate);
  Serial.println("Checking for errors...");

  //EEPROM.write(0, 100); //For reseting

  //No data was saved in EEPROM --> get new data from user
  if(EEPROM.read(0) != 99)
  {
    int tmp = 300;
    
    Serial.println("Left motor speed (0 - 255): ");
    do
    {
      tmp = wczytaj_liczbe();
      delay(50);
    }while(!(tmp <= 255 && tmp >= 1));
    EEPROM.write(1, tmp);

    tmp = 300;
    Serial.println("Right motor speed (0 - 255): ");
    do
    {
      tmp = wczytaj_liczbe();
      delay(50);
    }while(!(tmp <= 255 && tmp >= 1));
    EEPROM.write(2, tmp);

    tmp = 300;
    Serial.println("Multiplier for turning (0 - 100): ");
    do
    {
      tmp = wczytaj_liczbe();
      delay(50);
    }while(!(tmp <= 100 && tmp >= 1));
    EEPROM.write(3, tmp);

    tmp = 300;
    Serial.println("First gyro angle (1 - 180): ");
    do
    {
      tmp = wczytaj_liczbe();
      delay(50);
    }while(!(tmp <= 180 && tmp >= 1));
    EEPROM.write(4, tmp);

    tmp = 300;
    Serial.println("Second gyro angle (1 - 180): ");
    do
    {
      tmp = wczytaj_liczbe();
      delay(50);
    }while(!(tmp <= 180 && tmp >= 1));
    EEPROM.write(5, tmp);

    tmp = 300;
    Serial.println("Third gyro angle (1 - 180): ");
    do
    {
      tmp = wczytaj_liczbe();
      delay(50);
    }while(!(tmp <= 180 && tmp >= 1));
    EEPROM.write(6, tmp);

    tmp = 300;
    Serial.println("Distance triggering obstacle (3 - 50): ");
    do
    {
      tmp = wczytaj_liczbe();
      delay(50);
    }while(!(tmp <= 50 && tmp >= 1));
    EEPROM.write(7, tmp);

    EEPROM.write(0, 99);
    if(EEPROM.read(0) != 99)
    {
      Serial.println("COuld not save new data properly!");
    }
  }

  //Read parameters from EEPROM
  lewy = EEPROM.read(1);
  prawy = EEPROM.read(2);
  mnoznik_zawracania = EEPROM.read(3);

  kat_1 = EEPROM.read(4);
  kat_2 = EEPROM.read(5);
  kat_3 = EEPROM.read(6);

  odleglosc = EEPROM.read(7);

  //Display parameters
  Serial.print("Predkosc lewy: ");
  Serial.println(lewy);
  
  Serial.print("Predkosc prawy: ");
  Serial.println(prawy);

  mnoznik_zawracania = mnoznik_zawracania / 100;
  Serial.print("Mnoznik: ");
  Serial.println(mnoznik_zawracania);

  lewo_zwrot = mnoznik_zawracania*lewy;
  prawo_zwrot = mnoznik_zawracania*prawy;

  Serial.print("Zwrot lewo: ");
  Serial.println(lewo_zwrot);

  Serial.print("Zwrot prawo: ");
  Serial.println(prawo_zwrot);

  Serial.print("Kat 1: ");
  Serial.println(kat_1);

  Serial.print("Kat 2: ");
  Serial.println(kat_2);

  Serial.print("Kat 3: ");
  Serial.println(kat_3);
  
  Serial.print("Odleglosc wyzwalania przeszkody: ");
  Serial.println(odleglosc);
  
////////////////////////////////////////////////////////////////////////////////

  // Connect to digital ports expander
  Wire.beginTransmission(expander_adress);
  byte error = Wire.endTransmission();

  if (error = 2)
  {
    expander.begin(expander_adress);
    Serial.print("Connected to \"PCF8574\" at address 0x");
    Serial.print(expander_adress, HEX);
    Serial.println(" via I2C");
  }
  else
  {
    Serial.print("Unable to connect to \"PCF8574\" at adress 0x");
    Serial.print(expander_adress, HEX);
    Serial.println(", check connection!");
  }

  expander.pinMode(0, OUTPUT);
  expander.pinMode(1, OUTPUT);
  expander.pinMode(2, OUTPUT);
  expander.pinMode(3, OUTPUT);
  expander.pinMode(4, OUTPUT);
  expander.pinMode(5, OUTPUT);
  expander.pinMode(6, OUTPUT);
  expander.pinMode(7, OUTPUT);

  expander.digitalWrite(0, HIGH);
  expander.digitalWrite(1, HIGH);
  expander.digitalWrite(2, HIGH);
  expander.digitalWrite(3, HIGH);
  expander.digitalWrite(4, HIGH);
  expander.digitalWrite(5, HIGH);
  expander.digitalWrite(6, HIGH);
  expander.digitalWrite(7, HIGH);

////////////////////////////////////////////////////////////////////////////////

  //Connect to gyro
  if(!mpu.begin(MPU6050_SCALE_2000DPS, MPU6050_RANGE_2G))
  {
    Serial.println("Unable to connect to \"MPU6050\" at adress 0x61, check connection!");
    delay(500);
  }
  else
  {
    Serial.println("Connected to MPU6050 at adress 0x61 via I2C");
    // Kalibracja żyroskopu
    mpu.calibrateGyro();
    // Ustawienie czułości
    mpu.setThreshold(3);
  }

////////////////////////////////////////////////////////////////////////////////

  //Set all needed pins
  pinMode(L_PWM, OUTPUT);
  pinMode(R_PWM, OUTPUT); 
  pinMode(RF, OUTPUT); 
  pinMode(RR, OUTPUT); 
  pinMode(LF, OUTPUT); 
  pinMode(LR, OUTPUT); 
  
  pinMode(sensor_transistor, OUTPUT);
  pinMode(buzzer, OUTPUT);
  
  pinMode(ultrasonic_front_trig, OUTPUT);
  pinMode(ultrasonic_front_echo, INPUT);
  pinMode(ultrasonic_left_trig, OUTPUT);
  pinMode(ultrasonic_left_echo, INPUT);
  pinMode(ultrasonic_right_trig, OUTPUT);
  pinMode(ultrasonic_right_echo, INPUT);

  digitalWrite(L_PWM, LOW);
  digitalWrite(R_PWM, LOW);
  digitalWrite(RF, LOW);
  digitalWrite(RR, LOW);
  digitalWrite(LF, LOW);
  digitalWrite(LR, LOW);

  digitalWrite(sensor_transistor, LOW);
  digitalWrite(buzzer, LOW);

  digitalWrite(ultrasonic_front_trig, LOW);
  digitalWrite(ultrasonic_front_echo, LOW);
  digitalWrite(ultrasonic_left_trig, LOW);
  digitalWrite(ultrasonic_left_echo, LOW);
  digitalWrite(ultrasonic_right_trig, LOW);
  digitalWrite(ultrasonic_right_echo, LOW);

////////////////////////////////////////////////////////////////////////////////

  //Flash LEDs to signal end of initializing 
  for(int i = 0 ; i < 4 ; i++)
  {
    expander.digitalWrite(i, LOW);
    delay(250); 
  }

  for(int i = 7 ; i > 3 ; i--)
  {
    expander.digitalWrite(i, LOW);
    delay(250);
  }

  //Buzz to signal end of initializing 
  delay(500);
  tone(buzzer, 5000, 100);
  delay(150);
  tone(buzzer, 5000, 100);
  delay(150);
  tone(buzzer, 5000, 100);
  delay(500);
  expander.set();
  
  Serial.println("READY");
}

//==============================================================================

//Main loop
void loop() 
{
  short tmp = 10;

  //Display menu
  Serial.println("[1] Start LF");
  Serial.println("[2] Start LFE");
  Serial.println("[3] Start LFE_G");
  Serial.println("[4] Freeride");
  Serial.println("[5] Read MPU6050 <--- change baud rate to 115200 before");
  Serial.println("[6] Blink PCF8574");
  Serial.println("[7] Check opto sensors");
  Serial.println("[8] Line scanner");
  Serial.println("[9] Settings");
  Serial.println("Input 0 to return to this menu");

  //Decide what to do
  do
  {
    tmp = Serial.read(); 
    delay(50);
  }while(tmp != '1' && tmp != '2' && tmp != '3' && tmp != '4' && tmp != '5' && tmp != '6' && tmp != '7' && tmp != '8' && tmp != '9');

  switch(tmp)
  {
    case '1':
    {
      LFS();

      break;
    }
    
    case '2':
    {
      LFE();

      break;
    }
    
    case '3':
    {
      LFE_G();

      break;
    }
    
    case '4':
    {
      freeride();

      break;
    }
    
    case '5':
    {
      read_MPU6050();

      break;
    }
    
    case '6':
    {
      blink_expander();

      break;
    }
    
    case '7':
    {
      check_sensors();

      break;
    }
    
    case '8':
    {
      line_scanner();

      break;
    }

    case '9':
    {
      settings();

      break;
    }
  }
  delay(100);
}

//==============================================================================

// Line Follower Standard - without obstacle detection
void LFS()
{  
  Serial.println("Starting...");
  
  char tmp = '1';
  bool czujniki[8] = {0,0,0,0,0,0,0,0};
  short last = 1; // 0 - lewo, 1 - srodek, 2 - prawo
  
  digitalWrite(sensor_transistor, HIGH); //Włączenie płytki z czujnikami
  delay(1000);
  tone(buzzer, 5000, 500);
  delay(600);
  tone(buzzer, 5000, 500);
  delay(600);
  tone(buzzer, 5000, 500);
  delay(500);
  Serial.println("READY!");
  
  do
  {
    tmp = Serial.read();
    for(int i = 0 ; i < 8 ; i++)
    {
      int level = analogRead(i);
      if(level >700)
      {
        czujniki[i] = 1;
      }
      else
      {
        czujniki[i] = 0;
      }
    }

////////////////////////////////////////////////////////////////////////////////

  expander.set();
  if(czujniki[0] == 1)
    {
        expander.digitalWrite(4, LOW);
    }
    if(czujniki[1] == 1)
    {
        expander.digitalWrite(5, LOW);
    }
    if(czujniki[2] == 1)
    {
        expander.digitalWrite(6, LOW);
    }
    if(czujniki[3] == 1)
    {
        expander.digitalWrite(7, LOW);
    }
    if(czujniki[4] == 1)
    {
        expander.digitalWrite(3, LOW);
    }
    if(czujniki[5] == 1)
    {
        expander.digitalWrite(2, LOW);
    }
    if(czujniki[6] == 1)
    {
        expander.digitalWrite(1, LOW);
    }
    if(czujniki[7] == 1)
    {
        expander.digitalWrite(0, LOW);
    }
    
////////////////////////////////////////////////////////////////////////////////

    wpisz_silniki(0, 0, 0, 0, 0, 0);
    
    if(czujniki[7])
    {
      wpisz_silniki(0, 230, 1, 0, 1, 0);
    }
    else
    {
      if(czujniki[0] )
      {
        wpisz_silniki(230, 0, 1, 0, 1, 0);
      }
      else
      {
        if(czujniki[3] || czujniki[4])
        {
          wpisz_silniki(230, 230, 1, 0, 1, 0);
        }
        else
        {
          if(czujniki[2] || czujniki[1])
          {
            wpisz_silniki(230, 170, 1, 0, 1, 0);
          }
          else
          {
            if(czujniki[5] || czujniki[6])
            {
              wpisz_silniki(170, 230, 1, 0, 1, 0);
            }
            else
            {
              wpisz_silniki(0,0,0,0,0,0); 
            }
          }
        } 
      }     
    }
 
//////////////////////////////////////////////////////////////////////////////

    if(czujniki[7])
    {
      last = 0;
    }
    else
    {
      if(czujniki[0] )
      {
        last = 2;
      }
      else
      {
        if(czujniki[5] || czujniki[6])
        {
          last = 0;
        }
        else
        {
          if(czujniki[2] || czujniki[1])
          {
            last = 2;
          }
          else
          {
            if(czujniki[3] || czujniki[4])
            {
                last = 1;
            }
          }
        }
      }
    }
      
/////////////////////////////////////////////////////////////////////////

    if(!(czujniki[0]|| czujniki[1] || czujniki[2]|| czujniki[3] || czujniki[4]|| czujniki[5] || czujniki[6]|| czujniki[7]))
    {
      if(last == 0)
      {
        wpisz_silniki(lewo_zwrot, prawo_zwrot, 0, 1, 1, 0);
      }
      if(last == 1)
      {
        wpisz_silniki(lewo_zwrot,200,1,0,1,0);
      }
      if(last == 2)
      {
        wpisz_silniki(lewo_zwrot, prawo_zwrot, 1, 0, 0, 1);
      }
    }
    
    delay(10);
  }while(tmp != '0');

  wpisz_silniki(0, 0, 0, 0, 0, 0);
  digitalWrite(sensor_transistor, LOW); //Wyłączenie płytki z czujnikami
}

//==============================================================================

// Line Follower Enchanced - with obstacle detection, without gyro
void LFE()
{
  Serial.println("Starting...");

  short counter = 0;
  char tmp = '1';
  bool czujniki[8] = {0,0,0,0,0,0,0,0};
  short last = 1; // 0 - lewo, 1 - srodek, 2 - prawo
  
  digitalWrite(sensor_transistor, HIGH); //Włączenie płytki z czujnikami
  delay(1000);
  tone(buzzer, 5000, 500);
  delay(600);
  tone(buzzer, 5000, 500);
  delay(600);
  tone(buzzer, 5000, 500);
  delay(500);
  Serial.println("READY!");
  
  do
  {
    tmp = Serial.read();
    counter++;
    
    for(int i = 0 ; i < 8 ; i++)
    {
      int level = analogRead(i);
      if(level >700)
      {
        czujniki[i] = 1;
      }
      else
      {
        czujniki[i] = 0;
      }
    }

////////////////////////////////////////////////////////////////////////////////

  if(czujniki[0]+czujniki[1]+czujniki[2]+czujniki[3]+czujniki[4]+czujniki[5]+czujniki[6]+czujniki[7] >= 7)
  {
    wpisz_silniki(lewy,250,1,0,1,0);
    continue; 
  }

////////////////////////////////////////////////////////////////////////////////

if(counter > 2)
{
  counter = 0;
  int distance = read_ultrasonic(ultrasonic_front_trig, ultrasonic_front_echo);
  
  if(distance > 0 && distance < odleglosc)
  {
    avoid_obstacle_w_o_g();
    last = 1;
    for(int i = 0 ; i < 8 ; i++)
      {
        czujniki[i] = 0;
      }
  }
}

////////////////////////////////////////////////////////////////////////////////

  expander.set();
  if(czujniki[0] == 1)
    {
        expander.digitalWrite(4, LOW);
    }
    if(czujniki[1] == 1)
    {
        expander.digitalWrite(5, LOW);
    }
    if(czujniki[2] == 1)
    {
        expander.digitalWrite(6, LOW);
    }
    if(czujniki[3] == 1)
    {
        expander.digitalWrite(7, LOW);
    }
    if(czujniki[4] == 1)
    {
        expander.digitalWrite(3, LOW);
    }
    if(czujniki[5] == 1)
    {
        expander.digitalWrite(2, LOW);
    }
    if(czujniki[6] == 1)
    {
        expander.digitalWrite(1, LOW);
    }
    if(czujniki[7] == 1)
    {
        expander.digitalWrite(0, LOW);
    }
    
////////////////////////////////////////////////////////////////////////////////

    wpisz_silniki(0, 0, 0, 0, 0, 0);
    
    if(czujniki[7])
    {
      wpisz_silniki(0, prawy, 1, 0, 1, 0);
    }
    else
    {
      if(czujniki[0] )
      {
        wpisz_silniki(lewy, 0, 1, 0, 1, 0);
      }
      else
      {
        if(czujniki[3] || czujniki[4])
        {
          wpisz_silniki(lewy, prawy, 1, 0, 1, 0);
        }
        else
        {
          if(czujniki[2] || czujniki[1])
          {
            wpisz_silniki(lewy, prawo_zwrot, 1, 0, 1, 0);
          }
          else
          {
            if(czujniki[5] || czujniki[6])
            {
              wpisz_silniki(lewo_zwrot, prawy, 1, 0, 1, 0);
            }
            else
            {
              wpisz_silniki(0,0,0,0,0,0); 
            }
          }
        } 
      }     
    }
 
//////////////////////////////////////////////////////////////////////////////

    if(!(czujniki[0]|| czujniki[1] || czujniki[2]|| czujniki[3] || czujniki[4]|| czujniki[5] || czujniki[6]|| czujniki[7]))
    {
      if(last == 0)
      {
        wpisz_silniki(lewo_zwrot, prawo_zwrot, 0, 1, 1, 0);
      }
      if(last == 1)
      {
        wpisz_silniki(lewy,250,1,0,1,0);
      }
      if(last == 2)
      {
        wpisz_silniki(lewo_zwrot, prawo_zwrot, 1, 0, 0, 1);
      }
    }

/////////////////////////////////////////////////////////////////////////

    if(czujniki[7])
    {
      last = 0;
    }
    else
    {
      if(czujniki[0] )
      {
        last = 2;
      }
      else
      {
        if(czujniki[5] || czujniki[6])
        {
          last = 0;
        }
        else
        {
          if(czujniki[2] || czujniki[1])
          {
            last = 2;
          }
          else
          {
            if(czujniki[3] || czujniki[4])
            {
                last = 1;
            }
          }
        }
      }
    }

/////////////////////////////////////////////////////////////////////////
      
    delay(15);
  }while(tmp != '0');
  
  wpisz_silniki(0, 0, 0, 0, 0, 0);
  digitalWrite(sensor_transistor, LOW); //Wyłączenie płytki z czujnikami
}

//==============================================================================

// Line Follower Enchanced Gyro - with obstacle detection, with gyro
void LFE_G()
{
  Serial.println("Starting...");

  short counter = 0;
  char tmp = '1';
  bool czujniki[8] = {0,0,0,0,0,0,0,0};
  short last = 1; // 0 - lewo, 1 - srodek, 2 - prawo
  
  digitalWrite(sensor_transistor, HIGH); //Włączenie płytki z czujnikami
  delay(1000);
  tone(buzzer, 5000, 500);
  delay(600);
  tone(buzzer, 5000, 500);
  delay(600);
  tone(buzzer, 5000, 500);
  delay(500);
  Serial.println("READY!");
  
  do
  {
    tmp = Serial.read();
    counter++;
    
    for(int i = 0 ; i < 8 ; i++)
    {
      int level = analogRead(i);
      if(level >700)
      {
        czujniki[i] = 1;
      }
      else
      {
        czujniki[i] = 0;
      }
    }

////////////////////////////////////////////////////////////////////////////////

  if(czujniki[0]+czujniki[1]+czujniki[2]+czujniki[3]+czujniki[4]+czujniki[5]+czujniki[6]+czujniki[7] >= 7)
  {
    wpisz_silniki(lewy,250,1,0,1,0);
    continue; 
  }

////////////////////////////////////////////////////////////////////////////////

if(counter > 2)
{
  counter = 0;
  int distance = read_ultrasonic(ultrasonic_front_trig, ultrasonic_front_echo);
  
  if(distance > 0 && distance < odleglosc)
  {
    avoid_obstacle_w_g();
    last = 1;
    for(int i = 0 ; i < 8 ; i++)
      {
        czujniki[i] = 0;
      }
  }
}

////////////////////////////////////////////////////////////////////////////////

  expander.set();
  if(czujniki[0] == 1)
    {
        expander.digitalWrite(4, LOW);
    }
    if(czujniki[1] == 1)
    {
        expander.digitalWrite(5, LOW);
    }
    if(czujniki[2] == 1)
    {
        expander.digitalWrite(6, LOW);
    }
    if(czujniki[3] == 1)
    {
        expander.digitalWrite(7, LOW);
    }
    if(czujniki[4] == 1)
    {
        expander.digitalWrite(3, LOW);
    }
    if(czujniki[5] == 1)
    {
        expander.digitalWrite(2, LOW);
    }
    if(czujniki[6] == 1)
    {
        expander.digitalWrite(1, LOW);
    }
    if(czujniki[7] == 1)
    {
        expander.digitalWrite(0, LOW);
    }
    
////////////////////////////////////////////////////////////////////////////////

    wpisz_silniki(0, 0, 0, 0, 0, 0);
    
    if(czujniki[7])
    {
      wpisz_silniki(0, prawy, 1, 0, 1, 0);
    }
    else
    {
      if(czujniki[0] )
      {
        wpisz_silniki(lewy, 0, 1, 0, 1, 0);
      }
      else
      {
        if(czujniki[3] || czujniki[4])
        {
          wpisz_silniki(lewy, prawy, 1, 0, 1, 0);
        }
        else
        {
          if(czujniki[2] || czujniki[1])
          {
            wpisz_silniki(lewy, prawo_zwrot, 1, 0, 1, 0);
          }
          else
          {
            if(czujniki[5] || czujniki[6])
            {
              wpisz_silniki(lewo_zwrot, prawy, 1, 0, 1, 0);
            }
            else
            {
              wpisz_silniki(0,0,0,0,0,0); 
            }
          }
        } 
      }     
    }
 
//////////////////////////////////////////////////////////////////////////////

    if(!(czujniki[0]|| czujniki[1] || czujniki[2]|| czujniki[3] || czujniki[4]|| czujniki[5] || czujniki[6]|| czujniki[7]))
    {
      if(last == 0)
      {
        wpisz_silniki(lewo_zwrot, prawo_zwrot, 0, 1, 1, 0);
      }
      if(last == 1)
      {
        wpisz_silniki(lewy,250,1,0,1,0);
      }
      if(last == 2)
      {
        wpisz_silniki(lewo_zwrot, prawo_zwrot, 1, 0, 0, 1);
      }
    }

/////////////////////////////////////////////////////////////////////////

    if(czujniki[7])
    {
      last = 0;
    }
    else
    {
      if(czujniki[0] )
      {
        last = 2;
      }
      else
      {
        if(czujniki[5] || czujniki[6])
        {
          last = 0;
        }
        else
        {
          if(czujniki[2] || czujniki[1])
          {
            last = 2;
          }
          else
          {
            if(czujniki[3] || czujniki[4])
            {
                last = 1;
            }
          }
        }
      }
    }

/////////////////////////////////////////////////////////////////////////
      
    delay(15);
  }while(tmp != '0');
  
  wpisz_silniki(0, 0, 0, 0, 0, 0);
  digitalWrite(sensor_transistor, LOW); //Wyłączenie płytki z czujnikami
}

//==============================================================================

//Controlled via Bluetooth
void freeride()
{ 
  Serial.println("Starting...");
  char tmp = '1';
  delay(1000);
  tone(buzzer, 5000, 500);
  delay(600);
  tone(buzzer, 5000, 500);
  delay(600);
  tone(buzzer, 5000, 500);
  delay(500);
  Serial.println("READY!");
  
  
  do
  {
    if(Serial.available() > 0)
    {
      char data = Serial.read();

      wpisz_silniki(0, 0, 0, 0, 0, 0);
      switch(data)
      {
        case 'w':
        {
          wpisz_silniki(lewy, prawy, 1, 0, 1, 0);
    
          break;
        }
  
        case 's':
        {
          wpisz_silniki(lewy, prawy, 0, 1, 0, 1);
  
          break;
        }
  
        case 'a':
        {
          wpisz_silniki(lewo_zwrot, prawo_zwrot, 0, 1, 1, 0);
          
          break;
        }
  
        case 'd':
        {
          wpisz_silniki(lewo_zwrot, prawo_zwrot, 1, 0, 0, 1);
          
          break;
        }
  
        case '0':
        {
          tmp = 0;
          
          break;
        }
      }
      delay(50);
    } 
    
  }while(tmp != '0');
  wpisz_silniki(0, 0, 0, 0, 0, 0);
}

//==============================================================================

//Debug mode - read data from gyro
void read_MPU6050()
{
  Serial.println("Starting...");
  bool tmp = 1;

  unsigned long timer = 0;
  float timeStep = 0.01;

  float pitch = 0;
  float roll = 0;
  float yaw = 0;
  
  delay(3000);
  mpu.calibrateGyro();
  
  do
  {
    tmp = Serial.read();
    
    timer = millis();

    Vector norm = mpu.readNormalizeGyro();

    pitch = pitch + norm.YAxis * timeStep;
    roll = roll + norm.XAxis * timeStep;
    yaw = yaw + norm.ZAxis * timeStep;

    Serial.print(" Pitch = ");
    Serial.print(pitch);
    Serial.print(" Roll = ");
    Serial.print(roll);  
    Serial.print(" Yaw = ");
    Serial.println(yaw);

    delay((timeStep*1000) - (millis() - timer));

  }while(tmp != '0');
}

//==============================================================================

//Debug mode - check LEDs
void blink_expander()
{
  Serial.println("Starting...");
  delay(3000);
  
  for(int i = 0 ; i < 4 ; i++)
  {
    expander.digitalWrite(i, LOW);
    delay(250); 
  }

  for(int i = 7 ; i > 3 ; i--)
  {
    expander.digitalWrite(i, LOW);
    delay(250);
  }
  
  delay(1000);
  expander.set();
}

//==============================================================================

//Debug mode - read sensor values and send via bluetooth
void check_sensors()
{
  bool czujniki[8] = {0,0,0,0,0,0,0,0};

  Serial.println("Starting...");
  char tmp = '1';
  digitalWrite(sensor_transistor, HIGH); //Włączenie płytki z czujnikami
  delay(2000);
  
  do
  {
    tmp = Serial.read();
    for(int i = 0 ; i < 8 ; i++)
    {
      int level = analogRead(i);
      if(level >700)
      {
        czujniki[i] = 1;
      }
      else
      {
        czujniki[i] = 0;
      }
    }
  
    for(int i = 0 ; i < 8 ; i++)
    {
      Serial.print(czujniki[i]);
      Serial.print(" ");
    }
    Serial.println(" ");
    delay(250);
    
  }while(tmp != '0');

  digitalWrite(sensor_transistor, LOW); //Wyłączenie płytki z czujnikami
}

//==============================================================================

//Debug mode - read sensor values and display via LEDs
void line_scanner()
{
  bool czujniki[8] = {0,0,0,0,0,0,0,0};
  
  Serial.println("Starting...");
  char tmp = '1';
  digitalWrite(sensor_transistor, HIGH); //Włączenie płytki z czujnikami
  delay(2000);
  Serial.println("READY!");
  
  do
  {
    tmp = Serial.read();
    for(int i = 0 ; i < 8 ; i++)
    {
      int level = analogRead(i);
      if(level >700)
      {
        czujniki[i] = 1;
      }
      else
      {
        czujniki[i] = 0;
      }
    }

    if(czujniki[0] == 1)
    {
        expander.digitalWrite(4, LOW);
    }
    if(czujniki[1] == 1)
    {
        expander.digitalWrite(5, LOW);
    }
    if(czujniki[2] == 1)
    {
        expander.digitalWrite(6, LOW);
    }
    if(czujniki[3] == 1)
    {
        expander.digitalWrite(7, LOW);
    }
    if(czujniki[4] == 1)
    {
        expander.digitalWrite(3, LOW);
    }
    if(czujniki[5] == 1)
    {
        expander.digitalWrite(2, LOW);
    }
    if(czujniki[6] == 1)
    {
        expander.digitalWrite(1, LOW);
    }
    if(czujniki[7] == 1)
    {
        expander.digitalWrite(0, LOW);
    }
    
    delay(50);
    expander.set();
    
  }while(tmp != '0');

  digitalWrite(sensor_transistor, LOW); //Wyłączenie płytki z czujnikami
}

//==============================================================================

//Change parameters of LF
void settings()
{
  bool wyjscie = 1;
  bool czy = 1;
  char tmp; 

  do
  {
    Serial.println("[1] Wypisz aktualne");
    Serial.println("[2] Zmien predkosci");
    Serial.println("[3] Zmien kat");
    Serial.println("[4] Zmien odleglosc wyzwalania przeszkody");

    czy = 1;
    wyjscie = 1;
  
    do
    {
      if(Serial.available() > 0)
      {
        tmp = Serial.read();
        if(tmp == '0' || tmp == '1' || tmp == '2' || tmp == '3' || tmp == '4')
          czy = 0;    
      }
    }while(czy);
  
    switch(tmp)
    {
      case '0':
      {
        wyjscie = 0;
        break;
      }
      case '1':
      {
        Serial.print("Predkosc lewy: ");
        Serial.println(lewy);
        
        Serial.print("Predkosc prawy: ");
        Serial.println(prawy);
      
        Serial.print("Mnoznik: ");
        Serial.println(mnoznik_zawracania);
      
        Serial.print("Zwrot lewo: ");
        Serial.println(lewo_zwrot);
      
        Serial.print("Zwrot prawo: ");
        Serial.println(prawo_zwrot);
      
        Serial.print("Kat 1: ");
        Serial.println(kat_1);
      
        Serial.print("Kat 2: ");
        Serial.println(kat_2);
      
        Serial.print("Kat 3: ");
        Serial.println(kat_3);

        Serial.print("Odleglosc wyzwalania od przeszkody: ");
        Serial.println(odleglosc);

        while(!(Serial.available() > 0));
        
        for(int i = 0 ; i < 50 ; i++)
          Serial.read();
          
        break;
      }
  
      case '2':
      {
        EEPROM.write(0, 100); //Do resetowania
    
        if(EEPROM.read(0) != 99)
        {
          int tmp = 300;
          
          Serial.println("Podaj predkosc silnika lewego (0 - 255): ");
          do
          {
            tmp = wczytaj_liczbe();
            delay(50);
          }while(!(tmp <= 255 && tmp >= 1));
          EEPROM.write(1, tmp);
      
          tmp = 300;
          Serial.println("Podaj predkosc silnika prawego (0 - 255): ");
          do
          {
            tmp = wczytaj_liczbe();
            delay(50);
          }while(!(tmp <= 255 && tmp >= 1));
          EEPROM.write(2, tmp);
      
          tmp = 300;
          Serial.println("Podaj mnoznik predkosci do zawracania (0 - 100): ");
          do
          {
            tmp = wczytaj_liczbe();
            delay(50);
          }while(!(tmp <= 100 && tmp >= 1));
          EEPROM.write(3, tmp);
      
          EEPROM.write(0, 99);
          if(EEPROM.read(0) != 99)
          {
            Serial.println("NIe zapisano poprwnie!");
          }
        }
      
        lewy = EEPROM.read(1);
        prawy = EEPROM.read(2);
        mnoznik_zawracania = EEPROM.read(3);
      
        Serial.print("Predkosc lewy: ");
        Serial.println(lewy);
        
        Serial.print("Predkosc prawy: ");
        Serial.println(prawy);
      
        mnoznik_zawracania = mnoznik_zawracania / 100;
        Serial.print("Mnoznik: ");
        Serial.println(mnoznik_zawracania);
      
        lewo_zwrot = mnoznik_zawracania*lewy;
        prawo_zwrot = mnoznik_zawracania*prawy;
      
        Serial.print("Zwrot lewo: ");
        Serial.println(lewo_zwrot);
      
        Serial.print("Zwrot prawo: ");
        Serial.println(prawo_zwrot);

        while(!(Serial.available() > 0));

        for(int i = 0 ; i < 50 ; i++)
          Serial.read();
          
        break;
      }
  
      case '3':
      {
        EEPROM.write(0, 100); //Do resetowania
    
        if(EEPROM.read(0) != 99)
        {
          int tmp = 300;
          
          Serial.println("Podaj kat 1 (1 - 180): ");
          do
          {
            tmp = wczytaj_liczbe();
            delay(50);
          }while(!(tmp <= 180 && tmp >= 1));
          EEPROM.write(4, tmp);
      
          tmp = 300;
          Serial.println("Podaj kat 2 (1 - 180): ");
          do
          {
            tmp = wczytaj_liczbe();
            delay(50);
          }while(!(tmp <= 180 && tmp >= 1));
          EEPROM.write(5, tmp);
      
          tmp = 300;
          Serial.println("Podaj kat 3 (1 - 180): ");
          do
          {
            tmp = wczytaj_liczbe();
            delay(50);
          }while(!(tmp <= 180 && tmp >= 1));
          EEPROM.write(6, tmp);
      
          EEPROM.write(0, 99);
          if(EEPROM.read(0) != 99)
          {
            Serial.println("NIe zapisano poprwnie!");
          }
        }
      
        kat_1 = EEPROM.read(4);
        kat_2 = EEPROM.read(5);
        kat_3 = EEPROM.read(6);
      
        Serial.print("Kat 1: ");
        Serial.println(kat_1);
      
        Serial.print("Kat 2: ");
        Serial.println(kat_2);
      
        Serial.print("Kat 3: ");
        Serial.println(kat_3);

        while(!(Serial.available() > 0));

        for(int i = 0 ; i < 50 ; i++)
          Serial.read();
          
        break;
      }

      case '4':
      {
        EEPROM.write(0, 100);
  
        if(EEPROM.read(0) != 99)
        {
          int tmp = 300;
          Serial.println("Podaj odleglosc wyzwalania przeszkody (3 - 50): ");
          do
          {
            tmp = wczytaj_liczbe();
            delay(50);
          }while(!(tmp <= 50 && tmp >= 1));
          EEPROM.write(7, tmp);
      
          EEPROM.write(0, 99);
          if(EEPROM.read(0) != 99)
          {
            Serial.println("NIe zapisano poprwnie!");
          }
        }
      
        odleglosc = EEPROM.read(7);
        
        Serial.print("Odleglosc wyzwalania przeszkody: ");
        Serial.println(odleglosc);

        while(!(Serial.available() > 0));

        for(int i = 0 ; i < 50 ; i++)
          Serial.read();
          
        break;
      }
    }
  }while(wyjscie);
}

//==============================================================================

//Set new speed and direction
void wpisz_silniki(short l_pwm, short r_pwm, bool lf, bool lr, bool rf, bool rr)
{
  analogWrite(L_PWM, l_pwm);
  analogWrite(R_PWM, r_pwm);
  digitalWrite(LF, lf);
  digitalWrite(LR, lr);
  digitalWrite(RF, rf);
  digitalWrite(RR, rr);
}

//==============================================================================

//Read distance from ultrasonic sensor
int read_ultrasonic(short trig, short echo)
{
  int czas, dist;
  digitalWrite(trig, HIGH);
  delayMicroseconds(1000);
  digitalWrite(trig, LOW);
  czas = pulseIn(echo, HIGH);
  dist = (czas/2) / 29.1;
  return dist;
}

//==============================================================================

//Procedure to avoid detected obstacle without gyro
void avoid_obstacle_w_o_g()
{
  wpisz_silniki(lewo_zwrot, prawo_zwrot, 1, 0, 0, 1);
  delay(time_first);
  wpisz_silniki(lewo_zwrot, prawo_zwrot, 1, 0, 1, 0);
  delay(time_second);
  wpisz_silniki(lewo_zwrot, prawo_zwrot, 0, 1, 1, 0);
  delay(time_third);
  wpisz_silniki(lewo_zwrot, prawo_zwrot, 1, 0, 1, 0);
  delay(time_fourth);
  wpisz_silniki(lewo_zwrot, prawo_zwrot, 0, 1, 1, 0);
  delay(time_fiveth);
  wpisz_silniki(lewo_zwrot, prawo_zwrot, 1, 0, 1, 0);
  bool czujniki[8] = {0,0,0,0,0,0,0};
  do
  {
    for(int i = 0 ; i < 8 ; i++)
    {
      int level = analogRead(i);
      if(level >700)
      {
        czujniki[i] = 1;
      }
      else
      {
        czujniki[i] = 0;
      }
    }
  }while(!(czujniki[0]|| czujniki[1] || czujniki[2]|| czujniki[3] || czujniki[4]|| czujniki[5] || czujniki[6]|| czujniki[7]));
}

//Procedure to avoid detected obstacle with gyro
void avoid_obstacle_w_g()
{
  wpisz_silniki(lewo_zwrot, prawo_zwrot, 1, 0, 0, 1);
  wait_X_degrees(1, kat_1);
  wpisz_silniki(lewo_zwrot, prawo_zwrot, 1, 0, 1, 0);
  delay(time_1);
  wpisz_silniki(lewo_zwrot, prawo_zwrot, 0, 1, 1, 0);
  wait_X_degrees(0, kat_2);
  wpisz_silniki(lewo_zwrot, prawo_zwrot, 1, 0, 1, 0);
  delay(time_2);
  wpisz_silniki(lewo_zwrot, prawo_zwrot, 0, 1, 1, 0);
  wait_X_degrees(0, kat_3);
  wpisz_silniki(lewo_zwrot, prawo_zwrot, 1, 0, 1, 0);
  
  bool czujniki[8] = {0,0,0,0,0,0,0};
  do
  {
    for(int i = 0 ; i < 8 ; i++)
    {
      int level = analogRead(i);
      if(level >700)
      {
        czujniki[i] = 1;
      }
      else
      {
        czujniki[i] = 0;
      }
      delayMicroseconds(100);
    }
  }while(!(czujniki[0]|| czujniki[1] || czujniki[2]|| czujniki[3] || czujniki[4]|| czujniki[5] || czujniki[6]|| czujniki[7]));
}

//Wait for LF to rotate specified angle
void wait_X_degrees(bool lr, int degree) // 0 - left, 1 - right, absolute value of angle
{
  unsigned long timer = 0;
  float timeStep = 0.02; 
  float pitch = 0;
  float roll = 0;
  float yaw = 0;
  
  if(lr)
  {
    do
    {
      timer = millis();  
      Vector norm = mpu.readNormalizeGyro();
      yaw = yaw + norm.ZAxis * timeStep;
  
      Serial.println(yaw);
      
      delay((timeStep*1000) - (millis() - timer));    
    }while(yaw > -degree);
  }
  else
  {
    do
    {
      timer = millis();
      Vector norm = mpu.readNormalizeGyro();
      yaw = yaw + norm.ZAxis * timeStep;
  
      Serial.println(yaw);
      
      delay((timeStep*1000) - (millis() - timer));   
    }while(yaw < degree);
  }
}

//Get intiger value from serial communication
int wczytaj_liczbe()
{
  String inString = "";
  while (Serial.available() > 0) 
  {
    int inChar = Serial.read();
    if (isDigit(inChar)) 
    {
      inString += (char)inChar;
    }
  }
  return inString.toInt();
}

