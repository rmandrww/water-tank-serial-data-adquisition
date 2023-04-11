#include <LiquidCrystal.h>

//Variables para LCD
const int rs = 12, en = 11, d4 = 5, d5 = 4, d6 = 3, d7 = 6;

LiquidCrystal lcd(rs, en, d4, d5, d6, d7);

byte RxChar;

/*Registro de variables:
  T: (0-110 °C)
  y: (0-3.5 m)          */
struct Input {
  double temperature = 0.0, level = 0.0;
}Tank, Previous;

/*Simbolo para grados*/
byte grade[8] = {
  0b00011,
  0b00011,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
  0b00000,
};
//Registro para mensajes de peligro
struct dangermessages {
  char level[16] = "DESBORDE";
  char temperature[16] = "SOBRECALOR";
};
//Bloque para mensajes de advertencia
struct warningmessages {
  char maxlevel[16] = "Nivel Alto";
  char minlevel[16] = "Nivel Bajo";
  char temperature[16] = "Alta Temperatura";
};
//Bloque para mensajes
struct messages {
  char none[16] = "OK";
  dangermessages danger;
  warningmessages warning;
}alerts;

//Punteros para escoger el mensaje a mostrar en el LCD y evitar redundancia
char* current_message = 0;
char* previous_message = 0;

void setup() {
  /*Apaga el led integrado*/
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);
  
  //Pines A0 y A1 como entradas analógicas
  pinMode(A0, INPUT);
  pinMode(A1, INPUT);
  //2, 7, A2-5 como salidas digitales para alarmas
  pinMode(A5, OUTPUT);
  pinMode(A4, OUTPUT);
  pinMode(A3, OUTPUT);
  pinMode(A2, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(7, OUTPUT);
  
  //Inicia comunicación serial
  Serial.begin(9600);

  //Inicia comunicación con LCD 16x4
  lcd.begin(16, 4);
  //Crea el caracter para mostar los grados
  lcd.createChar(0, grade);
  lcd.clear();
  //Escribe el texto persistente
  lcd_print_text();
}

void lcd_print_text () {
  lcd.print(" Tanque de Agua");
  lcd.setCursor(0,1);
  lcd.print("Nivel:");
  lcd.setCursor (0, 2);
  lcd.print("Temp:");
}

//Actualiza los valores medidos en el display LCD
void lcd_update() {
  /*Actualiza nivel del tanque si cambió*/
  if (Tank.level != Previous.level) {
    lcd.setCursor (7,1);
    lcd.print("      ");
    lcd.setCursor (7,1);
    lcd.print(Tank.level);
    lcd.print(" m");
  }
  
  /*Actualiza temperatura del tanque si cambió*/
  if (Tank.temperature != Previous.temperature) {
    lcd.setCursor (7,2);
    lcd.print("     ");
    lcd.setCursor (7,2);
    lcd.print(Tank.temperature);
    lcd.write((byte)0);
    lcd.print('C');
  }
}

/*Actualiza el estado actual dependiendo del código recibido*/
void RxHandler () {
  //Lee el buffer de entrada serial
  RxChar = Serial.read();
  switch (RxChar) {
    /*Los caracteres que se reciben corresponden cada uno a los cambios de
      los indicadores de estado en la HMI*/
    /*Estos fueron escogidos con fines de depuración*/
    case 'a':
      digitalWrite(A5, 0);
      break;
    case 's':
      digitalWrite(A4, 0);
      break;
    case 'd':
      digitalWrite(A3, 0);
      break;
    case 'f':
      digitalWrite(7, 0);
      break;
    case 'g':
      digitalWrite(A2, 0);
      break;
    case 'h':
      digitalWrite(2, 0);
      break;

    case 'q':
      digitalWrite(A5, 1);
      break;
    case 'w':
      digitalWrite(A4, 1);
      break;
    case 'e':
      digitalWrite(A3, 1);
      break;
    case 'r':
      digitalWrite(7, 1);
      break;
    case 't':
      digitalWrite(A2, 1);
      break;
    case 'y':
      digitalWrite(2, 1);
      break;
  }
  //Valida el estado actual con los estados de los LEDS
  //Al mismo tiempo aplica prioridad de forma rudimentaria
  if (digitalRead(A4)) current_message = &alerts.danger.level[0];
  else if (digitalRead(A3)) current_message = &alerts.danger.temperature[0];
  else if (digitalRead(7)) current_message = &alerts.warning.maxlevel[0];
  else if (digitalRead(A2)) current_message = &alerts.warning.minlevel[0];
  else if (digitalRead(2)) current_message = &alerts.warning.temperature[0];
  else if (digitalRead(A5)) current_message = &alerts.none[0];
}


//Escribe el estado actual del sistema en la linea inferior del LCD
void lcd_message () {
  if (current_message != previous_message) {
    lcd.setCursor (0,3);
    lcd.print("                ");
    lcd.setCursor (0,3);
    lcd.print(current_message);
  }
}

void loop() {
  unsigned long timer = millis();
  //Lee entradas
  Tank.temperature = (double)analogRead(A0)*107.4219e-3;
  Tank.level = (double)analogRead(A1)*3.4180e-3;
  
  //Actualiza valores en display
  lcd_update();
  
  //Envía los valores leidos al computador
  Serial.print(Tank.level);
  Serial.print(' ');
  Serial.print(Tank.temperature);
  Serial.print('\n');
  
  //Revisa mensajes
  while (Serial.available() > 0) RxHandler();

  lcd_message();
  //Actualiza registro con el valor anterior
  Previous.temperature = Tank.temperature;
  Previous.level = Tank.level;
  previous_message = current_message;

  //Espera 100 milisegundos antes de volver a enviar
  delay(100 - short(millis() - timer) - 1);
}
