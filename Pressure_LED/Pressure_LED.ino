#include <Arduino_FreeRTOS.h>
#include <queue.h>
// I2Cdev and MPU6050 must be installed as libraries, or else the .cpp/.h files
// for both classes must be in the include path of your project
#include "I2Cdev.h"
#include "MPU6050.h"

// Arduino Wire library is required if I2Cdev I2CDEV_ARDUINO_WIRE implementation
// is used in I2Cdev.h
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
#include "Wire.h"
#endif
// class default I2C address is 0x68
// specific I2C addresses may be passed as a parameter here
// AD0 low = 0x68 (default for InvenSense evaluation board)
// AD0 high = 0x69
MPU6050 accelgyro;
//MPU6050 accelgyro(0x69); // <-- use for AD0 high

int16_t ax, ay, az;
int16_t gx, gy, gz;
int brightness = 0;


// uncomment "OUTPUT_READABLE_ACCELGYRO" if you want to see a tab-separated
// list of the accel X/Y/Z and then gyro X/Y/Z values in decimal. Easy to read,
// not so easy to parse, and slow(er) over UART.
#define OUTPUT_READABLE_ACCELGYRO

// uncomment "OUTPUT_BINARY_ACCELGYRO" to send all 6 axes of data as 16-bit
// binary, one right after the other. This is very fast (as fast as possible
// without compression or data loss), and easy to parse, but impossible to read
// for a human.
//#define OUTPUT_BINARY_ACCELGYRO

// define two tasks for Blink & AnalogRead
void TaskPressSens1( void *pvParameters );
void TaskLED( void *pvParameters );
QueueHandle_t QueuePressToLED = 0;

bool blinkState = false;

// the setup function runs once when you press reset or power the board
void setup() {
  // join I2C bus (I2Cdev library doesn't do this automatically)
#if I2CDEV_IMPLEMENTATION == I2CDEV_ARDUINO_WIRE
  Wire.begin();
#elif I2CDEV_IMPLEMENTATION == I2CDEV_BUILTIN_FASTWIRE
  Fastwire::setup(400, true);
#endif


  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  // initialize device
  Serial.println("Initializing I2C devices...");
  accelgyro.initialize();

  // verify connection
  Serial.println("Testing device connections...");
  Serial.println(accelgyro.testConnection() ? "MPU6050 connection successful" : "MPU6050 connection failed");

  //creating the queue to transfer data from pressure sensor read task to LED glow task
  QueuePressToLED = xQueueCreate(3, sizeof(int));

  // initialize digital LED_BUILTIN on pin 13 as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  // Now set up two tasks to run independently.
  xTaskCreate(
    TaskPressSens1
    ,  (const portCHAR *)"PressSens1"   // A name just for humans
    ,  128  // This stack size can be checked & adjusted by reading the Stack Highwater
    ,  NULL
    ,  2  // Priority, with 3 (configMAX_PRIORITIES - 1) being the highest, and 0 being the lowest.
    ,  NULL );

  xTaskCreate(
    TaskLED
    ,  (const portCHAR *) "LED"
    ,  128  // Stack size
    ,  NULL
    ,  1  // Priority
    ,  NULL );

  // Now the task scheduler, which takes over control of scheduling individual tasks, is automatically started.
}

void loop()
{
  // Empty. Things are done in Tasks.
}

/*--------------------------------------------------*/
/*---------------------- Tasks ---------------------*/
/*--------------------------------------------------*/

void TaskPressSens1(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  int i = 0;

  for (;;)
  {
    // read raw accel/gyro measurements from device
    accelgyro.getMotion6(&ax, &ay, &az, &gx, &gy, &gz);

    // these methods (and a few others) are also available
    //accelgyro.getAcceleration(&ax, &ay, &az);
    //accelgyro.getRotation(&gx, &gy, &gz);


    // read the input on analog pin 0:
    //    int sensorValue = analogRead(A0);
    // print out the value you read:
    if (! xQueueSend(QueuePressToLED, &ax, 10)) {
            Serial.println("Failed to send to queue");
    }

    //    Serial.println(i);
    vTaskDelay(20);  // one tick delay (15ms) in between reads for stability

    i++;
  }
}

void TaskLED(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  int press_val = 0;

  for (;;) // A Task shall never return or exit.
  {
    if (xQueueReceive(QueuePressToLED, &press_val, 20)) {
//      Serial.println("Received : ");
//      Serial.println(press_val);
      brightness = abs((17000+press_val)/140);
      analogWrite(LED_BUILTIN, brightness);
      vTaskDelay( 10); // wait for one second
    }
    else {
            Serial.println("Failed to receive");
    }
  }
}





