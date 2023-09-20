#include "pgmspace.h"
#include "BluetoothSerial.h"

#define BUF_LENGTH 64

BluetoothSerial SerialBT;
const byte Out[] = {16, 5, 4, 2, 14, 13, 15, 10}; // Modify as needed
static bool start = false;

void exec(char *cmdline) {
  char *command = strtok(cmdline, " ");
  
  if (strcmp_P(command, PSTR("HELP")) == 0) {
    printHelp();
  } else if (strcmp_P(command, PSTR("MODE")) == 0) {
    int pin = atoi(strtok(NULL, " "));
    int mode = atoi(strtok(NULL, " "));
    pinMode(pin, mode);
  } else if (strcmp_P(command, PSTR("READ")) == 0) {
    int pin = atoi(strtok(NULL, " "));
    Serial.println(digitalRead(pin));
  } else if (strcmp_P(command, PSTR("AREAD")) == 0) {
    int pin = atoi(strtok(NULL, " "));
    Serial.println(analogRead(pin));
  } else if (strcmp_P(command, PSTR("write")) == 0) {
    int pin = atoi(strtok(NULL, " "));
    int value = atoi(strtok(NULL, " "));
    digitalWrite(pin, value);
  } else if (strcmp_P(command, PSTR("AWRITE")) == 0) {
    int pin = atoi(strtok(NULL, " "));
    int value = atoi(strtok(NULL, " "));
    analogWrite(pin, value);
  } else if (strcmp_P(command, PSTR("ALL=1")) == 0) {
    for (int i = 0; i < 8; i++) {
      digitalWrite(Out[i], LOW);
    }
    Serial.println("OK");
  } else if (strcmp_P(command, PSTR("ALL=0")) == 0) {
    for (int i = 0; i < 8; i++) {
      digitalWrite(Out[i], HIGH);
    }
    Serial.println("OK");
  } else if (strncmp_P(command, PSTR("Out"), 3) == 0) {
    int OutNumber = atoi(command + 3);
    if (OutNumber >= 1 && OutNumber <= 8) {
      toggleOut(OutNumber - 1);
    } else {
      error("Invalid Out Number");
    }
  } else if (strcmp_P(command, PSTR("AT")) == 0) {
    Serial.println("OK");
  } else if (strcmp_P(command, PSTR("STAT")) == 0) {
    printOutStatus();
  } else {
    error("Unknown command");
  }
}

void toggleOut(int OutIndex) {
  digitalWrite(Out[OutIndex], !digitalRead(Out[OutIndex]));
  Serial.print(F("Toggled Out "));
  Serial.print(OutIndex + 1);
  Serial.print(F(" to "));
  Serial.println(digitalRead(Out[OutIndex]) ? "ON" : "OFF");
}

void printOutStatus() {
  for (int i = 0; i < sizeof(Out); i++) {
    int status = digitalRead(Out[i]);
    Serial.print(F("Out"));
    Serial.print(i + 1);
    Serial.print(F("="));
    Serial.println(status);
  }
  Serial.println("OK");
}

void setup() {
  Serial.begin(115200);
  SerialBT.begin("Out BT port");

  for (int i = 0; i < 8; i++) {
    pinMode(Out[i], OUTPUT);
  }
}

void loop() {
  while (!start) {
    for (int i = 0; i < 8; i++) {
      digitalWrite(Out[i], HIGH);
    }
    start = true;
  }

  processSerial(Serial);
  processSerial(SerialBT);
}

void processSerial(Stream &serial) {
  while (serial.available()) {
    static char buffer[BUF_LENGTH];
    static int length = 0;
    int data = serial.read();

    if (isLowerCase(data)) {
      data = toUpperCase(data);
    }

    if (data == '\b' || data == '\177') {
      if (length > 0) {
        length--;
      }
    } else if (data == '\n' || data == '\r') {
      buffer[length] = '\0';
      if (length > 0) {
        exec(buffer);
      }
      length = 0;
    } else if (length < BUF_LENGTH - 1) {
      buffer[length++] = data;
    }
  }
}

void error(const char *msg) {
  Serial.print(F("Error: "));
  Serial.println(msg);
}

void printHelp() {
  Serial.println(F(
    "\r\n"
"******************************************************\r\n"
"           BTCLI Driver Usage           \r\n"
"******************************************************\r\n"
"   Out<Out Number>=<Status> : 0 or 1                  \r\n"
"   Out<Out Number>          : Toggle Status           \r\n"
"   all=0                    : Clear All               \r\n"
"   all=1                    : Set All                 \r\n"
"   stat                     : Check all outputs state \r\n"
"******************************************************"
  ));
}
