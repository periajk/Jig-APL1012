#include <Wire.h>  // I2C
#include <LiquidCrystal_I2C.h>
#include <string.h>
#include <math.h>-
LiquidCrystal_I2C lcd(0x27, 16, 2);  // Address, 16 columns and 2 rows

int bt_connect = 2, a2dp_connect = 3, a2dp_playing = 4, disc_start = 5, stat_ok = 6;
int mosfet = 7, bt_led = 8;
int turn_esp = 9;
int led_erro = 10;
int led_rem = 11;
int button_test = 12;
int led_ok = 13;

float sensorValue = 0;

int audio_in = A0;
int rem_measurement = A1;
int wrong_name = A2;

bool btstat, a2dpstat, a2dpp, discstat;

float rem_volt_value = 0;
int analogValueA6 = 0;
float outputValue = 0;
float outputValueA6 = 0;

int bt_lcd_refresh = 0;
int ad2p_lcd_refresh = 0;

int linha_antiga = 0;
char *lcddisp_antigo = " ";

const unsigned long max_search_time = 5000;
unsigned long currentMillis;
unsigned long startMillis;
bool restart_flag = 1;
bool a2dp_flag = 1;
bool bt_flag = 1;
bool flag_trimp = 1;
float max_value = 0, min_value = 0;

unsigned long time_now = 0;

//void(* resetFunc) (void) = 0;

void setup() {

  lcd.init();       // Initialize I2C LCD module
  lcd.backlight();  // Turn backlight ON

  pinMode(bt_connect, INPUT);
  pinMode(a2dp_connect, INPUT);
  pinMode(a2dp_playing, INPUT);
  pinMode(disc_start, INPUT);
  pinMode(mosfet, OUTPUT);
  pinMode(button_test, INPUT_PULLUP);

  pinMode(bt_led, OUTPUT);
  pinMode(led_erro, OUTPUT);
  pinMode(led_rem, OUTPUT);
  pinMode(led_ok, OUTPUT);
  pinMode(stat_ok, OUTPUT);
  pinMode(turn_esp, OUTPUT);

  lcd.setCursor(0, 0);  // Go to column 0, row 0
  lcd.print("  !Iniciando!  ");
  //Serial.println("Iniciando");
  // lcd.clear();
  blink_begin();
  digitalWrite(led_erro, LOW);
  digitalWrite(led_rem, LOW);
  digitalWrite(led_ok, LOW);
  digitalWrite(stat_ok, LOW);
  digitalWrite(turn_esp, LOW);
  digitalWrite(bt_led, LOW);
  digitalWrite(mosfet, LOW);

  Serial.begin(9600);
}

void loop() {

  digitalWrite(bt_led, LOW);
  rem_volt_value = mapf(analogRead(rem_measurement), 0, 1023, 0, 5.0);
  // Serial.println(rem_volt_value);
  if (rem_volt_value > 3.8) {  // > 10,5V na fonte(4.2V medido no divisor) acende o LED do remote. *alterado para 3.8V no divisor
    digitalWrite(led_rem, HIGH);
  } else {
    digitalWrite(led_rem, LOW);
  }
  lcd.setCursor(0, 0);
  lcd.print("Pressione botao ");
  lcd.setCursor(0, 1);
  lcd.print("p/ iniciar teste");

  restart_flag = 1;
  a2dp_flag = 1;

  if (digitalRead(button_test) == 0) {
    digitalWrite(led_rem, LOW);
    while (1) {
      if (digitalRead(turn_esp) == 0) {
        digitalWrite(turn_esp, HIGH);
      }

      btstat = digitalRead(bt_connect);
      discstat = digitalRead(disc_start);
      a2dpstat = digitalRead(a2dp_connect);
      a2dpp = digitalRead(a2dp_playing);

      if (btstat == LOW && discstat == LOW) {
        lcd.setCursor(0, 0);
        lcd.print("   Aguardando   ");
        lcd.setCursor(0, 1);
        lcd.print("      ...       ");
        delay(50);
      }

      if (discstat == HIGH && btstat == LOW) {
        if (digitalRead(button_test) == 0)  //apertar botao para cancelar Busca
        {
          digitalWrite(turn_esp, LOW);
          digitalWrite(bt_led, LOW);
          delay(500);
          blink_restart();
          restart_flag = 1;
          break;
        }

        if (restart_flag == 1) {
          startMillis = millis();
          restart_flag = 0;
        }

        if (bt_led == HIGH) {
          digitalWrite(bt_led, LOW);
        }

        lcd.setCursor(0, 0);
        lcd.print("    Buscando    ");
        lcd.setCursor(0, 1);
        lcd.print("      ...       ");

        currentMillis = millis();
        if ((analogRead(wrong_name)) > 500) {
          digitalWrite(turn_esp, LOW);
          lcd.setCursor(0, 0);
          lcd.print("Erro de Gravacao");
          led_blink();
          restart_flag = 1;
          break;
        }

        if (currentMillis - startMillis >= max_search_time) {
          digitalWrite(turn_esp, LOW);
          //lcd.setCursor(0, 0);
          //lcd.print("   *Buscando*   ");
          delay(1000);
          restart_flag = 1;
        }
      }

      if (btstat == HIGH) {
        lcd.setCursor(0, 0);
        lcd.print("  BT Conectado  ");
        digitalWrite(bt_led, HIGH);
        if (a2dpstat == LOW) {
          if (bt_flag == 1) {
            startMillis = millis();
            bt_flag = 0;
          }
          currentMillis = millis();
          if (currentMillis - startMillis >= 5000) {
            digitalWrite(turn_esp, LOW);
            bt_flag = 1;
            break;
          }
        }
      }

      if (a2dpstat == HIGH && a2dpp == LOW)  //A2DP conectado
      {
        lcd.setCursor(0, 1);
        lcd.print(" A2DP Conectado ");
      }

      if (a2dpstat == HIGH && a2dpp == HIGH) {
        lcd.setCursor(0, 1);
        lcd.print("  A2DP Tocando  ");
        a2dp_flag = 0;

        if (rotina_teste() == 0) {
          digitalWrite(turn_esp, LOW);
          lcd.setCursor(0, 0);
          lcd.print("!Falha no Teste!");
          lcd.setCursor(0, 1);
          lcd.print("      ...       ");
          led_blink();
          bt_flag = 1;
          //resetFunc();
        }

        else {
          digitalWrite(turn_esp, LOW);
          lcd.setCursor(0, 0);
          lcd.print("!Placa Aprovada!");
          lcd.setCursor(0, 1);
          lcd.print("      ...       ");
          bt_flag = 1;
          digitalWrite(led_ok, HIGH);
          delay(3000);
          digitalWrite(led_ok, LOW);
          delay(1000);
          digitalWrite(led_ok, HIGH);
          delay(3000);
          digitalWrite(led_ok, LOW);
          //resetFunc();
        }
      }

      if (a2dpstat == LOW && a2dpp == LOW) {
        lcd.setCursor(0, 1);
        lcd.print("      ...       ");
        if (a2dp_flag == 0) {
          a2dp_flag = 1;
          digitalWrite(turn_esp, LOW);
          lcd.setCursor(0, 0);
          lcd.print("  Reiniciando   ");
          digitalWrite(led_rem, LOW);
          digitalWrite(bt_led, LOW);
          delay(1000);
          blink_restart();
          break;
        }
      }
    }
  }
}

void led_blink() {
  digitalWrite(led_erro, HIGH);
  delay(1000);
  digitalWrite(led_erro, LOW);
  delay(1000);
  digitalWrite(led_erro, HIGH);
  delay(1000);
  digitalWrite(led_erro, LOW);
}

bool rotina_teste() {
  while (1) {
    delay(500);
    // Ganho -------------------------------------------------------------
    max_value = medicao();

    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Coloque o Ganho ");
    for (int v = 1; v < 4; ++v) {
      lcd.setCursor(0, 1);
      lcd.print("  no minimo   " + String(v));
      delay(1000);
    }
    delay(1000);
    min_value = medicao();

    lcd.setCursor(0, 0);
    lcd.print(" Valores medidos");
    lcd.setCursor(0, 1);
    lcd.print("Mn:" + String(min_value) + "/Mx:" + String(max_value));
    delay(1000);

    if ((max_value) >= 4.6 && (max_value) <= 5.05 && (min_value) <= 0.5)  // Testa Ganho
    {
      lcd.setCursor(0, 0);
      lcd.print(" Ganho Aprovado ");
      lcd.setCursor(0, 1);
      lcd.print("      ...       ");
      digitalWrite(led_ok, HIGH);
      delay(1000);
      digitalWrite(led_ok, LOW);
      delay(1000);
    }

    else {
      return 0;
    }

    // Baixo--------------------------------------------------------------------------

    lcd.setCursor(0, 0);
    lcd.print("Coloque o Ganho ");
    for (int v = 1; v < 4; ++v) {
      lcd.setCursor(0, 1);
      lcd.print("  no maximo   " + String(v));
      delay(1000);
    }
    delay(1000);

    lcd.setCursor(0, 0);
    lcd.print("  Tocando Bass  ");
    lcd.setCursor(0, 1);
    lcd.print("      ...       ");

    max_value = medicao();
    //Serial.println("Valor máximo Bass: ");
    //Serial.println(max_value);
    lcd.setCursor(0, 0);
    lcd.print(" Coloque o Bass ");
    for (int v = 1; v < 4; ++v) {
      lcd.setCursor(0, 1);
      lcd.print("  no minimo   " + String(v));
      delay(1000);
    }
    delay(1000);

    min_value = medicao();


    lcd.setCursor(0, 0);
    lcd.print(" Valores medidos");
    lcd.setCursor(0, 1);
    lcd.print("Mn:" + String(min_value) + "/Mx:" + String(max_value));
    delay(1000);

    if ((max_value) >= 4.6 && (max_value) <= 5.05 && (min_value) <= 3.5 && (min_value) >= 1.0)  // Testa Bass
    {
      lcd.setCursor(0, 0);
      lcd.print(" Bass Aprovado  ");
      lcd.setCursor(0, 1);
      lcd.print("      ...       ");

      digitalWrite(led_ok, HIGH);
      delay(1000);
      digitalWrite(led_ok, LOW);
      delay(1000);
    }

    else {
      return 0;
    }

    // Mid --------------------------------------------------------------------------
    interrupt_esp();
    lcd.setCursor(0, 0);
    lcd.print(" Coloque o Bass ");
    for (int v = 1; v < 4; ++v) {
      lcd.setCursor(0, 1);
      lcd.print("  no maximo   " + String(v));
      delay(1000);
    }
    delay(1000);
    lcd.setCursor(0, 0);
    lcd.print("  Tocando Mid   ");
    lcd.setCursor(0, 1);
    lcd.print("      ...       ");
    max_value = medicao();
    // Serial.println("Valor máximo Mid: ");
    // Serial.println(max_value);
    lcd.setCursor(0, 0);
    lcd.print(" Coloque o Mid  ");
    for (int v = 1; v < 4; ++v) {
      lcd.setCursor(0, 1);
      lcd.print("  no minimo   " + String(v));
      delay(1000);
    }
    delay(1000);

    min_value = medicao();

    lcd.setCursor(0, 0);
    lcd.print(" Valores medidos");
    lcd.setCursor(0, 1);
    lcd.print("Mn:" + String(min_value) + "/Mx:" + String(max_value));
    delay(1000);

    if ((max_value) >= 4.6 && (max_value) <= 5.05 && (min_value) <= 1.8 && (min_value) >= 0.6) {  // Testa Mid
      lcd.setCursor(0, 0);
      lcd.print("  Mid Aprovado  ");
      lcd.setCursor(0, 1);
      lcd.print("      ...       ");

      digitalWrite(led_ok, HIGH);
      delay(1000);
      digitalWrite(led_ok, LOW);
      delay(1000);
    } else {
      return 0;
    }

    // High --------------------------------------------------------------------------
    interrupt_esp();
    lcd.setCursor(0, 0);
    +lcd.print(" Coloque o Mid  ");
    for (int v = 1; v < 4; ++v) {
      lcd.setCursor(0, 1);
      lcd.print("  no maximo   " + String(v));
      delay(1000);
    }
    delay(1000);


    lcd.setCursor(0, 0);
    lcd.print("  Tocando High  ");
    lcd.setCursor(0, 1);
    lcd.print("      ...       ");

    max_value = medicao();
    //Serial.println("Valor máximo High: ");
    //Serial.println(max_value);
    lcd.setCursor(0, 0);
    lcd.print(" Coloque o High ");
    for (int v = 1; v < 4; ++v) {
      lcd.setCursor(0, 1);
      lcd.print("  no minimo   " + String(v));
      delay(1000);
    }
    delay(1000);

    min_value = medicao();

    lcd.setCursor(0, 0);
    lcd.print(" Valores medidos");
    lcd.setCursor(0, 1);
    lcd.print("Mn:" + String(min_value) + "/Mx:" + String(max_value));
    delay(1000);

    if ((max_value) >= 4.6 && (max_value) <= 5.05 && (min_value) <= 1.5 && (min_value) >= 0.7) {  // Testa High
      lcd.setCursor(0, 0);
      lcd.print(" High Aprovado  ");
      lcd.setCursor(0, 1);
      lcd.print("      ...       ");

      digitalWrite(led_ok, HIGH);
      delay(1000);
      digitalWrite(led_ok, LOW);
      delay(1000);
    } else {
      return 0;
    }

    // Tocar Música ---------------------------------------------------------------------------
    interrupt_esp();
    digitalWrite(mosfet, HIGH);

    lcd.setCursor(0, 0);
    lcd.print(" Tocando Musica ");
    lcd.setCursor(0, 1);
    lcd.print(" Conecte o Fone ");
    delay(1000);
    startMillis = millis();

    while (digitalRead(button_test) == 1) {
      currentMillis = millis();
      if (currentMillis - startMillis >= 3000) {
        if (flag_trimp == 1) {
          lcd.setCursor(0, 0);
          lcd.print(" Gire o trimpot e ");
          lcd.setCursor(0, 1);
          lcd.print(" cheque o ruido ");
          flag_trimp = 0;
          startMillis = millis();
        }

        else if (flag_trimp == 0) {
          lcd.setCursor(0, 0);
          lcd.print(" Aperte o botao ");
          lcd.setCursor(0, 1);
          lcd.print(" se estiver OK  ");
          flag_trimp = 1;
          startMillis = millis();
        }
      }
    }
    flag_trimp = 1;


    lcd.setCursor(0, 1);
    lcd.print(" Retire o fone  ");
    lcd.setCursor(0, 0);
    lcd.print("Teste Finalizado");

    delay(1000);

    return 1;
  }
}

void interrupt_esp() {
  digitalWrite(stat_ok, HIGH);
  delay(20);
  digitalWrite(stat_ok, LOW);
}

float medicao() {  // calcula RMS por media
  outputValue = 0;
  int max = 0;
  long x = 0;
  for (int i = 0; i < 100; ++i) {
    for (int y = 0; y < 200; ++y) {
      sensorValue = analogRead(audio_in);
      if (sensorValue > max) {
        max = sensorValue;
      }
    }
    x = x + max;
  }
  x = x / 100;
  outputValue = mapf(x, 0, 1023, 0, 5);
  // Serial.println(outputValue);

  return (outputValue);
}

float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  return ((x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min);
}


void blink_begin() {
  digitalWrite(led_rem, HIGH);
  delay(200);
  digitalWrite(led_rem, LOW);
  digitalWrite(bt_led, HIGH);
  delay(100);
  digitalWrite(bt_led, LOW);
  digitalWrite(led_ok, HIGH);
  delay(100);
  digitalWrite(led_ok, LOW);
  digitalWrite(led_erro, HIGH);
  delay(100);
  digitalWrite(led_erro, LOW);
  delay(100);
  digitalWrite(led_rem, LOW);
  digitalWrite(bt_led, LOW);
  digitalWrite(led_ok, LOW);
  digitalWrite(led_erro, LOW);
  delay(100);
  digitalWrite(led_rem, HIGH);
  digitalWrite(bt_led, HIGH);
  digitalWrite(led_ok, HIGH);
  digitalWrite(led_erro, HIGH);
  delay(300);
}

void blink_restart() {
  digitalWrite(led_rem, HIGH);
  digitalWrite(bt_led, HIGH);
  digitalWrite(led_ok, HIGH);
  digitalWrite(led_erro, HIGH);
  delay(300);
  digitalWrite(led_rem, LOW);
  digitalWrite(bt_led, LOW);
  digitalWrite(led_ok, LOW);
  digitalWrite(led_erro, LOW);
}
