#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <DHT.h>
#include <DHT_U.h>

LiquidCrystal_I2C lcd(0x27, 20, 4);

#define DHTPIN 12
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

uint32_t timer = 0;

 /* deixei aqui as funções do sensor apenas para verificar onde devem ficar para cada modelo de filamento
 e seria necessário atualizar a cada 2 segundos mais ou menos
 */
float h = dht.readHumidity();
 float t = dht.readTemperature();

#define RELE_AUTO 13
#define RELE_A 9
int STATUS_RELE_A = 0;

int botaoUp = 2;
int botaoDown = 3;
int botaoVoltar = 4;
int botaoSelecionar = 5;

int selected_relay = 1;

typedef enum{
  BOTAO_VOLTAR = 0,
  BOTAO_UP,
  BOTAO_DOWN,
  BOTAO_SELECIONAR,
}PRESSED_BUTTON;


typedef enum{
  INITIALIZATION = 0,
  MODE_SELECT_PLA,
  MODE_CANCEL_PLA,
  MODE_SELECT_PETG,
  MODE_CANCEL_PETG,
  MODE_SELECT_ABS,
  MODE_CANCEL_ABS,
  MODE_SELECT_TPU,
  MODE_CANCEL_TPU,
  MODE_PLA,
  MODE_PETG,
  MODE_ABS,
  MODE_TPU,
}MENU_STATE;

MENU_STATE menu = INITIALIZATION;

void get_new_state(PRESSED_BUTTON button){
  switch (menu) {
  case MODE_SELECT_PLA:
     if(button == BOTAO_DOWN){
      menu = MODE_SELECT_PETG;

    } else if(button == BOTAO_UP){
      menu = MODE_SELECT_TPU;

    } else if(button == BOTAO_DOWN){
      menu = MODE_SELECT_ABS;

    } else if(button == BOTAO_UP){
      menu = MODE_SELECT_PETG;

    } else if(button == BOTAO_DOWN){
      menu = MODE_SELECT_TPU;

    } else if(button == BOTAO_UP){
      menu = MODE_SELECT_ABS; 
    }

    else{
      menu = MODE_PLA;
    }
    break;

  case MODE_SELECT_PETG:
    if(button == BOTAO_DOWN){
      menu = MODE_SELECT_ABS;
    
    } else if(button == BOTAO_UP){
      menu = MODE_SELECT_PLA;

    } else if(button == BOTAO_DOWN){
      menu = MODE_SELECT_TPU;

    } else if(button == BOTAO_UP){
      menu = MODE_SELECT_ABS;

    }
    else{
      menu = MODE_PETG;
    }
    break;

  case MODE_SELECT_ABS:
    if(button == BOTAO_DOWN){
      menu = MODE_SELECT_TPU;
    
    } else if(button == BOTAO_UP){
      menu = MODE_SELECT_PETG;

    }
    else{
      menu = MODE_ABS;
    }
    break;

    case MODE_SELECT_TPU:
     if(button == BOTAO_DOWN){
      menu = MODE_SELECT_PLA;
    
    } else if(button == BOTAO_UP){
      menu = MODE_SELECT_ABS;

    }
    else{
      menu = MODE_TPU;
    }
    break;

    case MODE_CANCEL_PLA:
     if(button == BOTAO_DOWN){
      menu = MODE_CANCEL_PLA;
    
    } else if(button == BOTAO_UP){
      menu = MODE_CANCEL_PLA;

    }
    else{
      menu = MODE_SELECT_PLA;
    }
    break;

  }
}

void updateMenu() {

  switch (menu) {
  case INITIALIZATION:
    lcd.setCursor(0,0);
    lcd.print("      DRY  BOX      ");
    lcd.setCursor(0, 1);
    lcd.print("      SECADORA      ");
    lcd.setCursor(0, 2);
    lcd.print("         DE         ");
    lcd.setCursor(0, 3);
    lcd.print("      FILAMENTO     ");
    break;

  case MODE_SELECT_PLA:
    lcd.setCursor(0,0);
    lcd.print("      FILAMENTO     ");
    lcd.setCursor(0, 1);
    lcd.print("                    ");
    lcd.setCursor(0, 2);
    lcd.print(">PLA            ABS ");
    lcd.setCursor(0, 3);
    lcd.print(" PETG           TPU ");
    break;

  case MODE_SELECT_PETG:
    lcd.setCursor(0,0);
    lcd.print("      FILAMENTO     ");
    lcd.setCursor(0, 1);
    lcd.print("                    ");
    lcd.setCursor(0, 2);
    lcd.print(" PLA            ABS ");
    lcd.setCursor(0, 3);
    lcd.print(">PETG           TPU ");
    break;

  case MODE_SELECT_ABS:
    lcd.setCursor(0,0);
    lcd.print("      FILAMENTO     ");
    lcd.setCursor(0, 1);
    lcd.print("                    ");
    lcd.setCursor(0, 2);
    lcd.print(" PLA           >ABS ");
    lcd.setCursor(0, 3);
    lcd.print(" PETG           TPU ");
    break;

  case MODE_SELECT_TPU:
    lcd.setCursor(0,0);
    lcd.print("      FILAMENTO     ");
    lcd.setCursor(0, 1);
    lcd.print("                    ");
    lcd.setCursor(0, 2);
    lcd.print(" PLA            ABS ");
    lcd.setCursor(0, 3);
    lcd.print(" PETG          >TPU ");
    break;

  case MODE_PLA:

    lcd.setCursor(0,0);
    lcd.print("   FILAMENTO  PLA   ");
    lcd.setCursor(0, 1);
    lcd.print("Set:50");
    lcd.setCursor(9, 1);
    lcd.print("Atual:");
    lcd.print(t,1);
    lcd.setCursor(19,1);
    lcd.write(B11011111); // Imprime o símbolo de grau
    lcd.setCursor(0, 2);
    lcd.print("Umidade: ");
    lcd.print(h,1);
    lcd.print("%     ");
    lcd.setCursor(0, 3);
    lcd.print("HH:MM:SS");
    lcd.setCursor(0, 3);
    lcd.print("              Cancel");
    break;

  case MODE_CANCEL_PLA:

    lcd.setCursor(0,0);
    lcd.print("   FILAMENTO  PLA   ");
    lcd.setCursor(0, 1);
    lcd.print("Set:50");
    lcd.setCursor(9, 1);
    lcd.print("Atual:");
    lcd.print(t,1);
    lcd.setCursor(19,1);
    lcd.write(B11011111); // Imprime o símbolo de grau
    lcd.setCursor(0, 2);
    lcd.print("Umidade: ");
    lcd.print(h,1);
    lcd.print("%     ");
    lcd.setCursor(0, 3);
    lcd.print("HH:MM:SS");
    lcd.setCursor(0, 3);
    lcd.print("             >Cancel");
    break;
    
  case MODE_PETG:

    lcd.setCursor(0,0);
    lcd.print("   FILAMENTO PETG   ");
    lcd.setCursor(0, 1);
    lcd.print("Set:65");
    lcd.setCursor(9, 1);
    lcd.print("Atual:");
    lcd.print(t,1);
    lcd.setCursor(19,1);
    lcd.write(B11011111); // Imprime o símbolo de grau
    lcd.setCursor(0, 2);
    lcd.print("Umidade: ");
    lcd.print(h,1);
    lcd.print("%     ");
    lcd.setCursor(0, 3);
    lcd.print("HH:MM:SS");
    lcd.setCursor(0, 3);
    lcd.print("              Cancel");
    break;

  case MODE_CANCEL_PETG:

    lcd.setCursor(0,0);
    lcd.print("   FILAMENTO PETG   ");
    lcd.setCursor(0, 1);
    lcd.print("Set:65");
    lcd.setCursor(9, 1);
    lcd.print("Atual:");
    lcd.print(t,1);
    lcd.setCursor(19,1);
    lcd.write(B11011111); // Imprime o símbolo de grau
    lcd.setCursor(0, 2);
    lcd.print("Umidade: ");
    lcd.print(h,1);
    lcd.print("%     ");
    lcd.setCursor(0, 3);
    lcd.print("HH:MM:SS");
    lcd.setCursor(0, 3);
    lcd.print("             >Cancel");
    break;

  case MODE_ABS:

    lcd.setCursor(0,0);
    lcd.print("   FILAMENTO  ABS   ");
    lcd.setCursor(0, 1);
    lcd.print("Set:65");
    lcd.setCursor(9, 1);
    lcd.print("Atual:");
    lcd.print(t,1);
    lcd.setCursor(19,1);
    lcd.write(B11011111); // Imprime o símbolo de grau
    lcd.setCursor(0, 2);
    lcd.print("Umidade: ");
    lcd.print(h,1);
    lcd.print("%     ");
    lcd.setCursor(0, 3);
    lcd.print("HH:MM:SS");
    lcd.setCursor(0, 3);
    lcd.print("              Cancel");
    break;

  case MODE_CANCEL_ABS:

    lcd.setCursor(0,0);
    lcd.print("   FILAMENTO  ABS   ");
    lcd.setCursor(0, 1);
    lcd.print("Set:65");
    lcd.setCursor(9, 1);
    lcd.print("Atual:");
    lcd.print(t,1);
    lcd.setCursor(19,1);
    lcd.write(B11011111); // Imprime o símbolo de grau
    lcd.setCursor(0, 2);
    lcd.print("Umidade: ");
    lcd.print(h,1);
    lcd.print("%     ");
    lcd.setCursor(0, 3);
    lcd.print("HH:MM:SS");
    lcd.setCursor(0, 3);
    lcd.print("             >Cancel");
    break;

  case MODE_TPU:

    lcd.setCursor(0,0);
    lcd.print("   FILAMENTO  TPU   ");
    lcd.setCursor(0, 1);
    lcd.print("Set:55");
    lcd.setCursor(9, 1);
    lcd.print("Atual:");
    lcd.print(t,1);
    lcd.setCursor(19,1);
    lcd.write(B11011111); // Imprime o símbolo de grau
    lcd.setCursor(0, 2);
    lcd.print("Umidade: ");
    lcd.print(h,1);
    lcd.print("%     ");
    lcd.setCursor(0, 3);
    lcd.print("HH:MM:SS");
    lcd.setCursor(0, 3);
    lcd.print("              Cancel");
    break;

  case MODE_CANCEL_TPU:

    lcd.setCursor(0,0);
    lcd.print("   FILAMENTO  TPU   ");
    lcd.setCursor(0, 1);
    lcd.print("Set:55");
    lcd.setCursor(9, 1);
    lcd.print("Atual:");
    lcd.print(t,1);
    lcd.setCursor(19,1);
    lcd.write(B11011111); // Imprime o símbolo de grau
    lcd.setCursor(0, 2);
    lcd.print("Umidade: ");
    lcd.print(h,1);
    lcd.print("%     ");
    lcd.setCursor(0, 3);
    lcd.print("HH:MM:SS");
    lcd.setCursor(0, 3);
    lcd.print("             >Cancel");
    break;

  }
}

void setup() {
  lcd.init();
  lcd.backlight();
  dht.begin();
  updateMenu();
  pinMode(botaoUp, INPUT_PULLUP);
  pinMode(botaoDown, INPUT_PULLUP);
  pinMode(botaoVoltar, INPUT_PULLUP);
  pinMode(botaoSelecionar, INPUT_PULLUP);
  pinMode(RELE_A, OUTPUT);
  pinMode(RELE_AUTO, OUTPUT);
  delay(1000);
  menu = MODE_SELECT_PLA;
  updateMenu();
}

void loop() {

  if (!digitalRead(botaoVoltar)){
    get_new_state(BOTAO_VOLTAR);
    updateMenu();
    delay(100);
    while (!digitalRead(botaoVoltar));
  }
  else if (!digitalRead(botaoUp)){
    get_new_state(BOTAO_UP);
    updateMenu();
    delay(100);
    while(!digitalRead(botaoUp));
  }
  else if (!digitalRead(botaoDown)){
    get_new_state(BOTAO_DOWN);
    updateMenu();
    delay(100);
    while (!digitalRead(botaoDown));
  }
  else if (!digitalRead(botaoSelecionar)){
    get_new_state(BOTAO_SELECIONAR);
    updateMenu();
    delay(100);
    while (!digitalRead(botaoSelecionar));
  }
}