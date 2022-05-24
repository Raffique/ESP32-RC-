#include <ESP32Servo.h>


#include <BluetoothSerial.h>
BluetoothSerial btSerial;
#define BT_NAME "ESP32BT-RC" // Set bluetooth name

#define BUILTIN_LED 2 // LED is active low
// ---------------------------------------------------------------------------
// Customize here pulse lengths as needed
#define MIN_PULSE_LENGTH 1000 // Minimum pulse length in µs
#define MAX_PULSE_LENGTH 2000 // Maximum pulse length in µs
// ---------------------------------------------------------------------------
Servo steer, accel;

#define STOPPER 10



void setup()
{
  pinMode(BUILTIN_LED, OUTPUT);
  pinMode(STOPPER, INPUT);
  digitalWrite(STOPPER, HIGH);

  accel.attach(4, MIN_PULSE_LENGTH, MAX_PULSE_LENGTH);
  steer.attach(5);

  Serial.begin(9600);
  btSerial.begin(BT_NAME);
  
  Serial.println("ESP32 Bluetooth Mobile Robot");
}


int last = 0;
bool calwatch = false;
bool first_time = true;
double first_time_reseter = 0;
double timer = 0;


void loop()
{
  calwatch = false;
  timer = millis();  
  if (btSerial.available()>0) {
    String signal = btSerial.readStringUntil('#');
    if (signal.length() == 7){
      int angle = signal.substring(0,3).toInt();
      int strength = signal.substring(3,6).toInt();
      int button = signal.substring(6,7).toInt();

      //STEERING WHEEL
      if (angle>= 30 && angle<=150){
        steer.write(map(angle, 150, 30, 180, 0));              
      }
      else if (angle >= 0 && angle <= 30){
        steer.write(30);
      }                            
      else if (angle >= 150 && angle <= 180){
        steer.write(150);
      }
      //CALIBRATION 
      else if (angle >= 240 && angle <= 300){
        calwatch = true;
        if(millis() - timer >= 10000){
          calibrate();
          timer = millis();
        }
      }      

      //ACCELERATOR 
      if (button == 1){
        accel.write(map(strength, 0, 100, 100, 150));        
      }
      //NITRO
      else if (button == 2){
        accel.write(map(strength, 0, 100, 140, 180)); 
      }
      //EMERGENCY BRAKE
      else if (button == 3){
         if (last==1 || last==2){
           accel.write(45);
           delay(300);
           accel.write(90);
         }
         else if (last == 4){
           accel.write(135);
           delay(300);
           accel.write(90);
         }
      }
      else if (button == 4){
        accel.write(map(strength, 0, 100, 80, 40)); 
      }

      last = button;
      accel.write(90); //stop
      btSerial.flush();            
    }      
  }
  if (!calwatch){
    timer = millis();
  }
}


void displayInstructions()
{  
    Serial.println("READY - PLEASE SEND INSTRUCTIONS AS FOLLOWING :");
    Serial.println("\t0 : Send min throttle");
    Serial.println("\t1 : Send max throttle");
    Serial.println("\t2 : Run test function\n");
}

void calibrate(){
  accel.write(0);
  delay(10000);
  accel.write(180);
  delay(10000);
  accel.write(90);
  delay(1000);
}


