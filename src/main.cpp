#include <Arduino.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <RotaryEncoder.h>
#include <DHT.h>
#include <DHT_U.h>

// ----------------------------
// --- Instanciando Objetos ---
LiquidCrystal_I2C lcd(0x27, 20, 4);
RotaryEncoder encoder(pinDT, pinCLK);

#define DHTPIN 12
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

uint32_t timer = 0;

 /* deixei aqui as funções do sensor apenas para verificar onde devem ficar para cada modelo de filamento
 e seria necessário atualizar a cada 2 segundos mais ou menos
 */
float h = dht.readHumidity();
float t = dht.readTemperature();


// -----------------------------
// --- Declaração de Funções ---
bool botaoApertado(byte pin_sw, int tempoDebounce = 100);
void configuraInterrupcao();
void atualizaDisplay();


// --------------------------------------
// --- Definição do Encoder ---
#define pinSW 10 // BOTAO DO ENCODER
#define pinDT 9
#define pinCLK 8

#define pinR 13 // PINO LED VERMELHO
#define pinG 1 // PINO LED VERDE
#define pinB 11 // PINO LED AZUL

// DEFINIÇÕES
#define menuMin 0 // VALOR MÍNIMO DE NAVEGAÇÃO DO MENU
#define menuMax 4 // VALOR MÁXIMO DE NAVEGAÇÃO DO MENU


// -----------------------------------
// --- Definição dos pinos de saída---
// #define RELE_AUTO 13
// #define RELE_A 9
int STATUS_RELE_A = 0;


// DECLARAÇÃO DE VARIÁVEIS
int menu = 0;
long posicao = 0, posicaoAnt = 0;

unsigned long tempoDisplay;
bool girouEncoder = true;

byte Leds[3] = {pinR, pinG, pinB}; // ARRAY DAS PORTAS DOS LEDS


// --- Definição dos pinos dos botões ---
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
  MODE_INICIAR_PLA,
  MODE_CANCELAR_PLA,
  MODE_PARAR_PLA,
  MODE_SELECT_PETG,
  MODE_INICIAR_PETG,
  MODE_CANCELAR_PETG,
  MODE_PARAR_PETG,
  MODE_SELECT_ABS,
  MODE_INICIAR_ABS,
  MODE_CANCELAR_ABS,
  MODE_PARAR_ABS,
  MODE_SELECT_TPU,
  MODE_INICIAR_TPU,
  MODE_CANCELAR_TPU,
  MODE_PARAR_TPU,
  MODE_PLA,
  MODE_PETG,
  MODE_ABS,
  MODE_TPU,
}MENU_STATE;

MENU_STATE menu = INITIALIZATION;

// ---Lógica para navegação dos menus principais ---
void get_new_state(PRESSED_BUTTON button){
  switch (menu) {

// ---------------------------------------
// --- Lógica para selecionar MENU PLA ---
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
    
    } else if (girouEncoder) {
      menu = MODE_SELECT_PETG;

    }  girouEncoder = false;
    else{
      menu = MODE_INICIAR_PLA;
    }
    break;
    posicaoAnt = posicao; //SALVA A POSIÇÃO ATUAL DO ENCODER PARA COMPARAR

// ----------------------------------------
// --- Lógica para selecionar MENU PETG ---
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
      menu = MODE_INICIAR_PETG;
    }
    break;

// ---------------------------------------
// --- Lógica para selecionar MENU ABS ---
  case MODE_SELECT_ABS:
    if(button == BOTAO_DOWN){
      menu = MODE_SELECT_TPU;
    
    } else if(button == BOTAO_UP){
      menu = MODE_SELECT_PETG;

    }
    else{
      menu = MODE_INICIAR_ABS;
    }
    break;

// ---------------------------------------
// --- Lógica para selecionar MENU TPU ---
    case MODE_SELECT_TPU:
     if(button == BOTAO_DOWN){
      menu = MODE_SELECT_PLA;
    
    } else if(button == BOTAO_UP){
      menu = MODE_SELECT_ABS;

    }
    else{
      menu = MODE_INICIAR_TPU;
    }
    break;


// ------------------------------------
// --- Lógica para menu INICIAR PLA ---
  case MODE_INICIAR_PLA:
     if(button == BOTAO_DOWN){
      menu = MODE_CANCELAR_PLA;

    } else if(button == BOTAO_UP || button == BOTAO_VOLTAR){
      menu = MODE_CANCELAR_PLA;

    }

    else{
      menu = MODE_PLA;
    }
    break;

// --- Lógica para menu CANCELAR PLA ---
  case MODE_CANCELAR_PLA:
     if(button == BOTAO_DOWN){
      menu = MODE_INICIAR_PLA;

    } else if(button == BOTAO_UP || button == BOTAO_VOLTAR){
      menu = MODE_INICIAR_PLA;

    }

    else{
      menu = MODE_SELECT_PLA;
    }
    break;

// --- Lógia para SAIR/PARAR submenu PLA ---
    case MODE_PLA:
     if(button == BOTAO_DOWN || button == BOTAO_UP || button == BOTAO_VOLTAR){
      menu = MODE_PARAR_PLA;
    
    }
    break;

    case MODE_PARAR_PLA:
     if(button == BOTAO_SELECIONAR){
      menu = MODE_SELECT_PLA;}
         break;


// -------------------------------------
// --- Lógica para menu INICIAR PETG ---
  case MODE_INICIAR_PETG:
     if(button == BOTAO_DOWN){
      menu = MODE_CANCELAR_PETG;

    } else if(button == BOTAO_UP || button == BOTAO_VOLTAR){
      menu = MODE_CANCELAR_PETG;

    }

    else{
      menu = MODE_PETG;
    }
    break;

// --- Lógica para menu CANCELAR PETG ---
  case MODE_CANCELAR_PETG:
     if(button == BOTAO_DOWN){
      menu = MODE_INICIAR_PETG;

    } else if(button == BOTAO_UP || button == BOTAO_VOLTAR){
      menu = MODE_INICIAR_PETG;

    }

    else{
      menu = MODE_SELECT_PETG;
    }
    break;

// --- Lógia para SAIR/PARAR submenu PETG ---
    case MODE_PETG:
     if(button == BOTAO_DOWN || button == BOTAO_UP || button == BOTAO_VOLTAR){
      menu = MODE_PARAR_PETG;
    
    }
    break;

    case MODE_PARAR_PETG:
     if(button == BOTAO_SELECIONAR){
      menu = MODE_SELECT_PETG;}
         break;


// ------------------------------------
// --- Lógica para menu INICIAR ABS ---
  case MODE_INICIAR_ABS:
     if(button == BOTAO_DOWN){
      menu = MODE_CANCELAR_ABS;

    } else if(button == BOTAO_UP || button == BOTAO_VOLTAR){
      menu = MODE_CANCELAR_ABS;

    }

    else{
      menu = MODE_ABS;
    }
    break;

// --- Lógica para menu CANCELAR ABS ---
  case MODE_CANCELAR_ABS:
     if(button == BOTAO_DOWN){
      menu = MODE_INICIAR_ABS;

    } else if(button == BOTAO_UP || button == BOTAO_VOLTAR){
      menu = MODE_INICIAR_ABS;

    }

    else{
      menu = MODE_SELECT_ABS;
    }
    break;

// --- Lógia para SAIR/PARAR submenu ABS ---
    case MODE_ABS:
     if(button == BOTAO_DOWN || button == BOTAO_UP || button == BOTAO_VOLTAR){
      menu = MODE_PARAR_ABS;
    
    }
    break;

    case MODE_PARAR_ABS:
     if(button == BOTAO_SELECIONAR){
      menu = MODE_SELECT_ABS;}
         break;


// ------------------------------------
// --- Lógica para menu INICIAR TPU ---
  case MODE_INICIAR_TPU:
     if(button == BOTAO_DOWN){
      menu = MODE_CANCELAR_TPU;

    } else if(button == BOTAO_UP || button == BOTAO_VOLTAR){
      menu = MODE_CANCELAR_TPU;

    }

    else{
      menu = MODE_TPU;
    }
    break;

// --- Lógica para menu CANCELAR TPU ---
  case MODE_CANCELAR_TPU:
     if(button == BOTAO_DOWN){
      menu = MODE_INICIAR_TPU;

    } else if(button == BOTAO_UP || button == BOTAO_VOLTAR){
      menu = MODE_INICIAR_TPU;

    }

    else{
      menu = MODE_SELECT_TPU;
    }
    break;

// --- Lógia para SAIR/PARAR submenu TPU ---
    case MODE_TPU:
     if(button == BOTAO_DOWN || button == BOTAO_UP || button == BOTAO_VOLTAR){
      menu = MODE_PARAR_TPU;
    
    }
    break;

    case MODE_PARAR_TPU:
     if(button == BOTAO_SELECIONAR){
      menu = MODE_SELECT_TPU;}
         break;

  }

}


// -----------------------------
// --- Implementando Funções ---
bool botaoApertado(byte pin_sw, int tempoDebounce = 100) {
  static unsigned long swDebounce;
  bool sw;
  static bool swAnt = 1;

  sw  = digitalRead(pin_sw);

  if ( millis() - swDebounce  > tempoDebounce) {

    if (!sw && swAnt) {
      swDebounce = millis();
      swAnt = sw;
      return true;

    } else if (sw && !swAnt) {
      swDebounce = millis();
      swAnt = sw;
    }
  }

  return false;
}

void configuraInterrupcao() {

  PCICR |= (1 << PCIE2); // HABILITA A INTERRUPÇÃO DAS PORTAS 0 A 7
  PCMSK2 |= (1 << PCINT22) | (1 << PCINT23); // CONFIGURA AS PORTAS 6 E 7 PARA INTERRUPÇÃO

}

void atualizaDisplay() {

  if (botaoApertado(pinSW) || (posicao != posicaoAnt) ) {
    lcd.backlight();
    tempoDisplay = millis();
  }

  if (millis() - tempoDisplay > 5000) lcd.noBacklight();

}

ISR(PCINT2_vect) { // FUNÇÃO DA INTERRUPÇÃO
  encoder.tick();
}

// ---------------------
// --- Inicialização ---
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

// --- Tela para selecionar PLA ---
  case MODE_SELECT_PLA:
    lcd.setCursor(0,0);
    lcd.print("      FILAMENTO     ");
    lcd.setCursor(0, 1);
    lcd.print("                    ");
    lcd.setCursor(0, 2);
    lcd.print(">PLA            ABS ");
    lcd.setCursor(0, 3);
    lcd.print(" PETG           TPU ");
    digitalWrite(RELE_AUTO, LOW);
    break;

// --- Tela para selecionar PETG ---
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

// --- Tela para selecionar ABS ---
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

// --- Tela para selecionar TPU ---
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


// ----------------------------------
// --- Tela secagem filamento PLA ---
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
    lcd.print("tempo");
    lcd.setCursor(13, 3);
    lcd.print("Cancel ");
    digitalWrite(RELE_AUTO, HIGH);
    break;

// --- Tela CONFIRMAR/CANCELAR filamento PLA ---
  case MODE_INICIAR_PLA:
    lcd.setCursor(0,0);
    lcd.print("  INICIAR  SECAGEM  ");
    lcd.setCursor(0, 1);
    lcd.print("         PLA        ");
    lcd.setCursor(0, 2);
    lcd.print("                    ");
    lcd.setCursor(0, 3);
    lcd.print(">Sim ");
    lcd.setCursor(16, 3);
    lcd.print(" Nao");
    break;

// --- Tela CANCELAR/CONFIRMAR filamento PLA ---
  case MODE_CANCELAR_PLA:
    lcd.setCursor(0,0);
    lcd.print("  INICIAR  SECAGEM  ");
    lcd.setCursor(0, 1);
    lcd.print("         PLA        ");
    lcd.setCursor(0, 2);
    lcd.print("                    ");
    lcd.setCursor(0, 3);
    lcd.print(" Sim ");
    lcd.setCursor(0, 3);
    lcd.print(" Sim            >Nao");
    break;

// --- Tela PARAR secagem filamento PLA ---
  case MODE_PARAR_PLA:
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
    lcd.print("tempo");
    lcd.setCursor(12, 3);
    lcd.print(">Cancel ");
    break;


// -----------------------------------
// --- Tela secagem filamento PETG ---
  case MODE_PETG:
    lcd.setCursor(0,0);
    lcd.print("   FILAMENTO PETG   ");
    lcd.setCursor(0, 1);
    lcd.print("Set:65");
    lcd.setCursor(7, 1);
    lcd.print("  Atual:");
    lcd.print(t,1);
    lcd.setCursor(19,1);
    lcd.write(B11011111); // Imprime o símbolo de grau
    lcd.setCursor(0, 2);
    lcd.print("Umidade: ");
    lcd.print(h,1);
    lcd.print("%     ");
    lcd.setCursor(0, 3);
    lcd.print("tempo");
    lcd.setCursor(13, 3);
    lcd.print("Cancel ");
    break;

// --- Tela CONFIRMAR/CANCELAR filamento PETG ---
  case MODE_INICIAR_PETG:
    lcd.setCursor(0,0);
    lcd.print("  INICIAR  SECAGEM  ");
    lcd.setCursor(0, 1);
    lcd.print("       PETG        ");
    lcd.setCursor(0, 2);
    lcd.print("                    ");
    lcd.setCursor(0, 3);
    lcd.print(">Sim ");
    lcd.setCursor(16, 3);
    lcd.print(" Nao");
    break;

// --- Tela CANCELAR/CONFIRMAR filamento PETG ---
  case MODE_CANCELAR_PETG:
    lcd.setCursor(0,0);
    lcd.print("  INICIAR  SECAGEM  ");
    lcd.setCursor(0, 1);
    lcd.print("       PETG        ");
    lcd.setCursor(0, 2);
    lcd.print("                    ");
    lcd.setCursor(0, 3);
    lcd.print(" Sim ");
    lcd.setCursor(0, 3);
    lcd.print(" Sim            >Nao");
    break;

// --- Tela PARAR secagem filamento PETG ---
  case MODE_PARAR_PETG:
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
    lcd.print("tempo");
    lcd.setCursor(12, 3);
    lcd.print(">Cancel ");
    break;


// ----------------------------------
// --- Tela secagem filamento ABS ---
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
    lcd.print("tempo");
    lcd.setCursor(13, 3);
    lcd.print("Cancel ");
    break;

// --- Tela CONFIRMAR/CANCELAR filamento ABS ---
  case MODE_INICIAR_ABS:
    lcd.setCursor(0,0);
    lcd.print("  INICIAR  SECAGEM  ");
    lcd.setCursor(0, 1);
    lcd.print("         ABS        ");
    lcd.setCursor(0, 2);
    lcd.print("                    ");
    lcd.setCursor(0, 3);
    lcd.print(">Sim ");
    lcd.setCursor(16, 3);
    lcd.print(" Nao");
    break;

// --- Tela CANCELAR/CONFIRMAR filamento ABS ---
  case MODE_CANCELAR_ABS:
    lcd.setCursor(0,0);
    lcd.print("  INICIAR  SECAGEM  ");
    lcd.setCursor(0, 1);
    lcd.print("         ABS        ");
    lcd.setCursor(0, 2);
    lcd.print("                    ");
    lcd.setCursor(0, 3);
    lcd.print(" Sim ");
    lcd.setCursor(0, 3);
    lcd.print(" Sim            >Nao");
    break;

// --- Tela PARAR secagem filamento ABS ---
  case MODE_PARAR_ABS:
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
    lcd.print("tempo");
    lcd.setCursor(12, 3);
    lcd.print(">Cancel ");
    break;


// ----------------------------------
// --- Tela secagem filamento TPU ---
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
    lcd.print("tempo");
    lcd.setCursor(13, 3);
    lcd.print("Cancel ");
    break;

// --- Tela CONFIRMAR/CANCELAR filamento TPU ---
  case MODE_INICIAR_TPU:
    lcd.setCursor(0,0);
    lcd.print("  INICIAR  SECAGEM  ");
    lcd.setCursor(0, 1);
    lcd.print("         TPU        ");
    lcd.setCursor(0, 2);
    lcd.print("                    ");
    lcd.setCursor(0, 3);
    lcd.print(">Sim ");
    lcd.setCursor(15, 3);
    lcd.print(" Nao");
    break;

// --- Tela CANCELAR/CONFIRMAR filamento TPU ---
  case MODE_CANCELAR_TPU:
    lcd.setCursor(0,0);
    lcd.print("  INICIAR  SECAGEM  ");
    lcd.setCursor(0, 1);
    lcd.print("         TPU        ");
    lcd.setCursor(0, 2);
    lcd.print("                    ");
    lcd.setCursor(0, 3);
    lcd.print(" Sim ");
    lcd.setCursor(0, 3);
    lcd.print(" Sim           >Nao ");
    break;

// --- Tela PARAR secagem filamento TPU ---
  case MODE_PARAR_TPU:
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
    lcd.print("tempo");
    lcd.setCursor(12, 3);
    lcd.print(">Cancel ");
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
  // pinMode(RELE_A, OUTPUT);
  // pinMode(RELE_AUTO, OUTPUT);

  Serial.begin(9600);

  for (byte i = 0; i < 3; i++) {
    pinMode(Leds[i], OUTPUT);
    digitalWrite(Leds[i], LOW);
  }

  Serial.println("Iniciando Display");

  configuraInterrupcao();
  posicaoAnt = encoder.getPosition(); //SALVA A POSIÇÃO ATUAL DO ENCODER PARA COMPARAR
  Serial.println("Fim Setup");
  
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

  posicao = encoder.getPosition();
  menu = constrain( (menu + (posicao - posicaoAnt) ), menuMin, menuMax);

  if ( posicao != posicaoAnt) {
    girouEncoder = true;
  }

}