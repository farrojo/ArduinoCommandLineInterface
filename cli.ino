#include <avr/pgmspace.h>

#define BUF_LENGTH 128

const int delvar = 10;
bool do_echo = true;
byte REL[] = {2, 3, 4, 5, 6, 7, 8, 9};
bool start = false;

void exec(char *cmdline);
void ReleAct(int a, int b);

void setup() {
  Serial.begin(115200);
  for (int i = 0; i < 8; i++) {
    pinMode(REL[i], OUTPUT);
  }
}

void loop() {
  while (!start) {
    for (int i = 0; i < 8; i++) {
      digitalWrite(REL[i], HIGH);
    }
    start = true;
  }

  while (Serial.available()) {
    static char buffer[BUF_LENGTH];
    static int length = 0;
    int data = Serial.read();

    if (data >= 'a' && data <= 'z') {
      data -= 32; // Convert to uppercase
    }

    if (data == '\b' || data == '\177') { // Backspace and Delete
      if (length) {
        length--;
        if (do_echo) {
          Serial.write("\b \b");
        }
      }
    } else if (data == '\n' || data == '\r') {
      if (do_echo) {
        Serial.write("\r\n");
      }
      buffer[length] = '\0';
      if (length) {
        exec(buffer);
      }
      length = 0;
    } else if (length < BUF_LENGTH - 1) {
      buffer[length++] = data;
      if (do_echo) {
        Serial.write(data);
      }
    }
  }
}

void exec(char *cmdline) {
  char *command = strtok(cmdline, " ");
  char relbuff[3];
  char relchan[0];
  char relstat[0];
  char relequl[0];
  memcpy(relbuff, &command[0], 3);
  memcpy(relchan, &command[3], 1);
  memcpy(relequl, &command[4], 1);
  memcpy(relstat, &command[5], 1);

  if (strcmp_P(command, PSTR("HELP")) == 0) {
    Serial.println(F(
        "\r\n"
        "******************************************************\r\n"
        "           OUTPUT Driver Usage           \r\n"
        "******************************************************\r\n"
        "   out<out Number>=<Status> : 0 or 1                  \r\n"
        "   out<out Number>          : Toggle Status           \r\n"
        "   all=0                    : Clear All               \r\n"
        "   all=1                    : Set All                 \r\n"
        "   stat                     : Check all outputs state \r\n"
        "******************************************************"));
  } else if (strcmp_P(command, PSTR("MODE")) == 0) {
    int pin = atoi(strtok(NULL, " "));
    int mode = atoi(strtok(NULL, " "));
    pinMode(pin, mode);
  } else if (strcmp_P(command, PSTR("READ")) == 0) {
    int pin = atoi(strtok(NULL, " "));
    Serial.println(digitalRead(pin));
  } else if (strcmp_P(command, PSTR("AREAD")) == 0) {
    int pin = atoi(strtok(NULL, " "));
    Serial.println(pin);
    Serial.println(analogRead(pin));
  } else if (strcmp_P(command, PSTR("write")) == 0) {
    int pin = atoi(strtok(NULL, " "));
    int value = atoi(strtok(NULL, " "));
    digitalWrite(pin, value);
  } else if (strcmp_P(command, PSTR("AWRITE")) == 0) {
    int pin = atoi(strtok(NULL, " "));
    Serial.println(pin);
    int value = atoi(strtok(NULL, " "));
    Serial.println(value);
    analogWrite(pin, value);
  } else if (strcmp_P(command, PSTR("ECHO")) == 0) {
    do_echo = atoi(strtok(NULL, " "));
  } else if (strcmp_P(command, PSTR("ALL=1")) == 0) {
    do_echo = atoi(strtok(NULL, " "));
    for (int i = 0; i < 8; i++) {
      digitalWrite(REL[i], LOW);
    }
    Serial.println("OK");
  } else if (strcmp_P(command, PSTR("ALL=0")) == 0) {
    do_echo = atoi(strtok(NULL, " "));
    for (int i = 0; i < 8; i++) {
      digitalWrite(REL[i], HIGH);
    }
    Serial.println("OK");
  } else if (strcmp_P(relbuff, PSTR("REL")) == 0) {
    int chan = relchan[0] - '0';
    int stat = relstat[0] - '0';
    int equl = relequl[0];
    if (equl == 61) {
      if (chan > 0 && chan < 9 && (stat == 0 || stat == 1)) {
        ReleAct(chan, stat);
      } else if (chan > 9 || chan < 1) {
        Serial.println("Rele Channel error!!");
      } else if (stat != 0 || stat != 1) {
        Serial.println("Rele Status error!!");
      }
    } else if (equl == 0 && chan > 0 && chan < 9) {
      stat = digitalRead(REL[chan - 1]);
      int PinST = digitalRead(REL[chan - 1]);
      ReleAct(chan, stat);
    } else {
      Serial.println("Rele chanel or status error!!");
    }
  } else if (strcmp_P(command, PSTR("AT")) == 0) {
    do_echo = atoi(strtok(NULL, " "));
    Serial.println("OK");
  } else if (strcmp_P(command, PSTR("STAT")) == 0) {
    Serial.println("");
    for (int i = 0; i < sizeof(REL) / sizeof(REL[0]); i++) {
      int adj = digitalRead(REL[i]);
      int realstat = (adj == 0) ? 1 : 0;
      Serial.print("Rel");
      Serial.print(i + 1);
      Serial.print("=");
      Serial.println(realstat);
    }
    Serial.println("OK");
  } else {
    Serial.print(F("Error: Unknown command: "));
    Serial.println(command);
  }
}

void ReleAct(int a, int b) {
  if (b == 0) {
    digitalWrite(REL[a - 1], HIGH);
  } else if (b == 1) {
    digitalWrite(REL[a - 1], LOW);
  }
  Serial.println("OK");
}
