#include <LiquidCrystal.h>
LiquidCrystal lcd(3, 2, 6, 7, 8, 9);
#define btnPin 4
#define redPin 10
#define yellowPin 11
#define greenPin 5
#define goodState 187 //0xBB //код годного элемента
#define badState 204  //0xCC //код негодного элемента
#define startMeasure 170 //0xAA //начало измерения
#define defaultState 171 //0xAB //остановка измерения
#define measureState 221 //0xDD идет измерение
#define countLimit 250

byte state = 0;
byte mode = 0; // 0 - ожидание,
               // 1 - была нажата кнопка, ждем ответа от ПК
byte incomingByte = 0;
byte buttonState = 0;
byte oneCounter = 0;
byte zeroCounter = 0;

ISR( TIMER1_COMPA_vect ) {
  digitalWrite(yellowPin, state);
  state = !state;
}

void setup() {
  pinMode(redPin, OUTPUT); 
  pinMode(yellowPin, OUTPUT);
  pinMode(greenPin, OUTPUT);
  pinMode(btnPin, INPUT);    
  digitalWrite(redPin, 0);
  digitalWrite(yellowPin, 0);
  digitalWrite(greenPin, 0);    
     
  lcd.begin(16, 2);
  lcd.print("   HA\xA3M\xA5TE \xE0\xA7\xB1"); //НАЖМИТЕ ДЛЯ
  lcd.setCursor(0, 1);
  lcd.print("    \xA5\xA4MEPEH\xA5\xB1"); //ИЗМЕРЕНИЯ0);
  Serial.begin(9600);
  TCCR1A &= 0B11111100; //отключаем WGM11 WGM10 - PWM off
  TCCR1B = 1<<WGM12; //CTC on
  TCCR1B |= 0B101;   // prescaler 1024
  OCR1A = 7811;      //fint = Fclk/(N*(1+OCR1A)); OCR1A = Fclk/(fint*N) - 1
}

void loop() {
  if(digitalRead(btnPin)){
    oneCounter++;
    zeroCounter = 0;
  }    
  else {
    zeroCounter++;
    oneCounter = 0;
  }
  if(zeroCounter == countLimit) {
    buttonState = 0;
    zeroCounter = 0;
  }  
  if(oneCounter == countLimit) {
    buttonState = 1;
    zeroCounter = 0;
  }  
  if(buttonState && (!mode)) { //если была нажата кнопка и еще не выведена надпись идет измерение
    lcd.setCursor(0, 0);
    lcd.print("      \xA5\xE0\xA2T      "); //ИДЁТ 
    lcd.setCursor(0, 1);
    lcd.print("    \xA5\xA4MEPEH\xA5\x45   ");//ИЗМЕРЕНИЕ
    Serial.write(startMeasure); //начало измерения
    TIMSK1 = (1<<OCIE1A); // Interrupt enable
    digitalWrite(redPin, LOW);
    digitalWrite(greenPin, LOW);
    mode = 1;    
  }
  if(buttonState == 0 && mode == 1) //выведена надпись, кнопка отпущена
    mode = 2;
  if(Serial.available() > 0) {
    incomingByte = Serial.read();  
    if(incomingByte == goodState) {  //годен
      TIMSK1 = (0<<OCIE1A); // Interrupt disable        
      digitalWrite(greenPin, HIGH);
      digitalWrite(redPin, LOW);
      digitalWrite(yellowPin, LOW);
      lcd.setCursor(0, 0);
      lcd.print("      \xA1O\xE0\x45\x48     ");//ГОДЕН
      lcd.setCursor(0, 1);
      lcd.print("                ");           
      mode = 0;
      incomingByte = 0;
    }
    if(incomingByte == badState) {    //не годен
      TIMSK1 = (0<<OCIE1A); // Interrupt disable      
      digitalWrite(greenPin, LOW);
      digitalWrite(redPin, HIGH);
      digitalWrite(yellowPin, LOW);
      lcd.setCursor(0, 0);
      lcd.print("      \xA0PAK      "); //БРАК
      lcd.setCursor(0, 1);
      lcd.print("                ");     
      mode = 0;
      incomingByte = 0;
    }  
    if(incomingByte == defaultState) {    //возврат в исходное состояние
      TIMSK1 = (0<<OCIE1A); // Interrupt disable
      digitalWrite(redPin, 0);
      digitalWrite(yellowPin, 0);
      digitalWrite(greenPin, 0);      
      lcd.setCursor(0, 0);
      lcd.print("   HA\xA3M\xA5TE \xE0\xA7\xB1"); //НАЖМИТЕ ДЛЯ
      lcd.setCursor(0, 1);
      lcd.print("    \xA5\xA4MEPEH\xA5\xB1"); //ИЗМЕРЕНИЯ0);  
      mode = 0;
    }
    if(incomingByte == measureState) {    //возврат в исходное состояние
      lcd.setCursor(0, 0);
      lcd.print("      \xA5\xE0\xA2T      "); //ИДЁТ 
      lcd.setCursor(0, 1);
      lcd.print("    \xA5\xA4MEPEH\xA5\x45   ");//ИЗМЕРЕНИЕ
      Serial.write(startMeasure); //начало измерения
      TIMSK1 = (1<<OCIE1A); // Interrupt enable
      digitalWrite(redPin, LOW);
      digitalWrite(greenPin, LOW);
      mode = 1;    
    }    
  }
}
