#include <Arduino_FreeRTOS.h>
#include <queue.h>

// define two tasks for Blink & AnalogRead
void TaskPressSens1( void *pvParameters );
void TaskLED( void *pvParameters );
QueueHandle_t QueuePressToLED = 0;

// the setup function runs once when you press reset or power the board
void setup() {

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB, on LEONARDO, MICRO, YUN, and other 32u4 based boards.
  }

  //creating the queue to transfer data from pressure sensor read task to LED glow task
  QueuePressToLED = xQueueCreate(3, sizeof(int));
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
    // read the input on analog pin 0:
    //    int sensorValue = analogRead(A0);
    // print out the value you read:
    if (! xQueueSend(QueuePressToLED, &i, 1)) {
      Serial.println("Failed to send to queue");
    }

    Serial.println(i);
    vTaskDelay(20);  // one tick delay (15ms) in between reads for stability

    i++;
  }
}

void TaskLED(void *pvParameters)  // This is a task.
{
  (void) pvParameters;

  // initialize digital LED_BUILTIN on pin 13 as an output.
  pinMode(LED_BUILTIN, OUTPUT);

  int press_val = 0;

  for (;;) // A Task shall never return or exit.
  {
    if (xQueueReceive(QueuePressToLED, &press_val, 20)) {
      Serial.println("Received : ");
      Serial.println(press_val);
      digitalWrite(LED_BUILTIN, HIGH);   // turn the LED on (HIGH is the voltage level)
      vTaskDelay( 10); // wait for one second
      digitalWrite(LED_BUILTIN, LOW);    // turn the LED off by making the voltage LOW
      vTaskDelay( 10); // wait for one second
    }
    else {
      Serial.println("Failed to receive");
    }
  }
}





