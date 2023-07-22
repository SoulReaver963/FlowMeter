#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <EEPROM.h>

#define Button 3

// Set Displaysize
LiquidCrystal_I2C lcd(0x27, 16, 2);

volatile uint32_t t_ms = 0;
volatile uint32_t frequenz = 0;

bool bCleared = 0;

const uint8_t nEepromAdress = 0;

const uint32_t cOneDaySeconds = 86400000;
const uint32_t cResetSeconds = 5000;

uint32_t nOneDaySeconds = 0;
uint32_t t = 0;
uint32_t t1 = 0;

float fLitrePerMinute = 0.0;
float fTotalLitre = 0.0;


ISR(TIMER0_COMPA_vect) {
  t_ms = t_ms + 1;
}

ISR(INT0_vect) {
  frequenz++;
}

void setup() {

  EICRA |= (1 << ISC00) | (1 << ISC01);
  EIMSK |= (1 << INT0);
  TCCR0B |= 3;
  TCCR0A |= (1 << WGM01);
  OCR0A = 250 - 1;
  TIMSK0 |= (1 << OCIE0A);
  sei();

  Serial.begin(9600);
  // Enable LCD
  lcd.init();
  // Enable LCD Backlight
  lcd.backlight();

  pinMode(Button, INPUT_PULLUP);

  EEPROM.get(nEepromAdress, fTotalLitre);
}

void loop() {

  // Save totallitre every two days to eeprom
  if (t_ms > nOneDaySeconds) {
    EEPROM.put(nEepromAdress, fTotalLitre);
    nOneDaySeconds += cOneDaySeconds;
  }

  // Reset totallitre if button is 5 seconds pressed
  if (bResetBtn() == 1) {
    t1 = t_ms + cResetSeconds;
  }
  if ((digitalRead(Button) == 0) && (t_ms > t1)) {
    fTotalLitre = 0;
    lcd.clear();
  }

  if (t_ms > t) {
    // Calculate the current used litre
    fLitrePerMinute = LitrePerMinute(frequenz);
    // Calculate the total used litre
    fTotalLitre += (fLitrePerMinute / 600);
    t += 100;
    frequenz = 0;
  }

  lcd.setCursor(0, 0);
  lcd.print("l/min");
  lcd.setCursor(7, 0);
  lcd.print(fLitrePerMinute);
  lcd.setCursor(0, 1);
  lcd.print("Total");
  lcd.setCursor(6, 1);
  lcd.print(fTotalLitre);

  Serial.println(frequenz);
  // Clear display
  if ((fLitrePerMinute == 0) && (bCleared == 0)) {
    lcd.clear();
    bCleared = 1;
  } else if (fLitrePerMinute > 0) {
    bCleared = 0;
  }
}

float LitrePerMinute(uint32_t FrequenzInput) {
  float fTempLitrePerMinute;
  
  fTempLitrePerMinute = FrequenzInput * 27 / 60.0;

  return (fTempLitrePerMinute);
}

bool bResetBtn(void) {
  static bool bButtonOld;
  static bool bButton;
  bButtonOld = bButton;
  bButton = digitalRead(Button);
  if (bButton < bButtonOld) {
    return (1);
  } else {
    return (0);
  }
}
