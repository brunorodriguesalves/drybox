#include <Arduino.h>
#include <DHT.h>
#include <DHT_U.h>
#include <LiquidCrystal_I2C.h>
#include <Wire.h>
#include <math.h>

#define TIMER_SECAGEM_PLA_S ((uint32_t)14460) // 4 horas
#define TIMER_SECAGEM_PETG_S ((uint32_t)7200) // 2 horas
#define TIMER_SECAGEM_ABS_S ((uint32_t)7200)  // 2 horas
#define TIMER_SECAGEM_TPU_S ((uint32_t)14400) // 4 horas
#define DEBOUNCE_TIME_MS ((uint32_t)200)      // Debounce time in milliseconds

LiquidCrystal_I2C lcd(0x27, 20, 4);

//----------------------------
// --- Caracteres especiais ---

// --- Desenho do termômetro ---
byte A8Char[] = {B00100, B01010, B01010, B01110,
                 B01110, B11111, B11111, B01110};

// --- Desenho do relógio ---
byte A20Char[] = {B00000, B01110, B00100, B01110,
                  B10101, B10111, B10001, B01110};

// --- Desenho da gota ---
byte A30Char[] = {B00100, B01010, B01010, B10001,
                  B11001, B11111, B11111, B01110};

// --- Cubico ---
byte A40Char[] = {B00111, B00001, B00011, B00001,
                  B00111, B00000, B00000, B00000};

#define DHTPIN 12
#define DHTTYPE DHT11
DHT dht(DHTPIN, DHTTYPE);

uint32_t current_millis = 0;
uint32_t millis_leitura_sensores = 0;
uint32_t millis_countdown_timer = 0;
uint32_t timer = 0;

float h;
float t;
float prev_h;
float prev_t;
float calculaUmidadeAbsoluta(void);

unsigned long lastDebounceTime[4] = {
    0, 0, 0, 0}; // Array to store debounce time for each button
void checkButtons();

void clean_display();

#define RELE_AUTO 13
#define RELE_A 9
int STATUS_RELE_A = 0;

int botaoUp = 2;
int botaoDown = 3;
int botaoVoltar = 4;
int botaoSelecionar = 5;

int selected_relay = 1;

typedef enum {
  BOTAO_VOLTAR = 0,
  BOTAO_UP,
  BOTAO_DOWN,
  BOTAO_SELECIONAR,
} PRESSED_BUTTON;

typedef enum {
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
} MENU_STATE;

MENU_STATE menu = INITIALIZATION;

// ---Lógica para navegação dos menus principais ---
void get_new_state(PRESSED_BUTTON button) {
  switch (menu) {
    // ---------------------------------------
    // --- Lógica para selecionar MENU PLA ---
  case MODE_SELECT_PLA:
    if (button == BOTAO_DOWN) {
      menu = MODE_SELECT_PETG;
    } else if (button == BOTAO_UP) {
      menu = MODE_SELECT_TPU;
    } else {
      menu = MODE_INICIAR_PLA;
    }
    break;

    // ----------------------------------------
    // --- Lógica para selecionar MENU PETG ---
  case MODE_SELECT_PETG:
    if (button == BOTAO_DOWN) {
      menu = MODE_SELECT_ABS;
    } else if (button == BOTAO_UP) {
      menu = MODE_SELECT_PLA;
    } else {
      menu = MODE_INICIAR_PETG;
    }
    break;

    // ---------------------------------------
    // --- Lógica para selecionar MENU ABS ---
  case MODE_SELECT_ABS:
    if (button == BOTAO_DOWN) {
      menu = MODE_SELECT_TPU;
    } else if (button == BOTAO_UP) {
      menu = MODE_SELECT_PETG;
    } else {
      menu = MODE_INICIAR_ABS;
    }
    break;

    // ---------------------------------------
    // --- Lógica para selecionar MENU TPU ---
  case MODE_SELECT_TPU:
    if (button == BOTAO_DOWN) {
      menu = MODE_SELECT_PLA;
    } else if (button == BOTAO_UP) {
      menu = MODE_SELECT_ABS;
    } else {
      menu = MODE_INICIAR_TPU;
    }
    break;

    // ------------------------------------
    // --- Lógica para menu INICIAR PLA ---
  case MODE_INICIAR_PLA:
    if (button == BOTAO_DOWN || button == BOTAO_UP || button == BOTAO_VOLTAR) {
      menu = MODE_CANCELAR_PLA;
    } else {
      timer = TIMER_SECAGEM_PLA_S;
      menu = MODE_PLA;
    }
    break;

    // --- Lógica para menu CANCELAR PLA ---
  case MODE_CANCELAR_PLA:
    if (button == BOTAO_DOWN || button == BOTAO_UP || button == BOTAO_VOLTAR) {
      menu = MODE_INICIAR_PLA;
    } else {
      menu = MODE_SELECT_PLA;
    }
    break;

    // --- Lógia para SAIR/PARAR submenu PLA ---
  case MODE_PLA:
    if (button == BOTAO_DOWN || button == BOTAO_UP || button == BOTAO_VOLTAR) {
      menu = MODE_PARAR_PLA;
    }
    break;

  case MODE_PARAR_PLA:
    if (button == BOTAO_SELECIONAR) {
      menu = MODE_SELECT_PLA;
    }
    break;

  // -------------------------------------
  // --- Lógica para menu INICIAR PETG ---
  case MODE_INICIAR_PETG:
    if (button == BOTAO_DOWN) {
      menu = MODE_CANCELAR_PETG;
    } else if (button == BOTAO_UP || button == BOTAO_VOLTAR) {
      menu = MODE_CANCELAR_PETG;
    }

    else {
      timer = TIMER_SECAGEM_PETG_S;
      menu = MODE_PETG;
    }
    break;

  // --- Lógica para menu CANCELAR PETG ---
  case MODE_CANCELAR_PETG:
    if (button == BOTAO_DOWN) {
      menu = MODE_INICIAR_PETG;

    } else if (button == BOTAO_UP || button == BOTAO_VOLTAR) {
      menu = MODE_INICIAR_PETG;
    } else {
      menu = MODE_SELECT_PETG;
    }
    break;

    // --- Lógia para SAIR/PARAR submenu PETG ---
  case MODE_PETG:
    if (button == BOTAO_DOWN || button == BOTAO_UP || button == BOTAO_VOLTAR) {
      menu = MODE_PARAR_PETG;
    }
    break;

  case MODE_PARAR_PETG:
    if (button == BOTAO_SELECIONAR) {
      menu = MODE_SELECT_PETG;
    }
    break;

    // ------------------------------------
    // --- Lógica para menu INICIAR ABS ---
  case MODE_INICIAR_ABS:
    if (button == BOTAO_DOWN) {
      menu = MODE_CANCELAR_ABS;
    } else if (button == BOTAO_UP || button == BOTAO_VOLTAR) {
      menu = MODE_CANCELAR_ABS;
    } else {
      timer = TIMER_SECAGEM_ABS_S;
      menu = MODE_ABS;
    }
    break;

    // --- Lógica para menu CANCELAR ABS ---
  case MODE_CANCELAR_ABS:
    if (button == BOTAO_DOWN) {
      menu = MODE_INICIAR_ABS;

    } else if (button == BOTAO_UP || button == BOTAO_VOLTAR) {
      menu = MODE_INICIAR_ABS;
    } else {
      menu = MODE_SELECT_ABS;
    }
    break;

    // --- Lógia para SAIR/PARAR submenu ABS ---
  case MODE_ABS:
    if (button == BOTAO_DOWN || button == BOTAO_UP || button == BOTAO_VOLTAR) {
      menu = MODE_PARAR_ABS;
    }
    break;

  case MODE_PARAR_ABS:
    if (button == BOTAO_SELECIONAR) {
      menu = MODE_SELECT_ABS;
    }
    break;

    // ------------------------------------
    // --- Lógica para menu INICIAR TPU ---
  case MODE_INICIAR_TPU:
    if (button == BOTAO_DOWN) {
      menu = MODE_CANCELAR_TPU;

    } else if (button == BOTAO_UP || button == BOTAO_VOLTAR) {
      menu = MODE_CANCELAR_TPU;

    }

    else {
      timer = TIMER_SECAGEM_TPU_S;
      menu = MODE_TPU;
    }
    break;

    // --- Lógica para menu CANCELAR TPU ---
  case MODE_CANCELAR_TPU:
    if (button == BOTAO_DOWN) {
      menu = MODE_INICIAR_TPU;
    } else if (button == BOTAO_UP || button == BOTAO_VOLTAR) {
      menu = MODE_INICIAR_TPU;
    } else {
      menu = MODE_SELECT_TPU;
    }
    break;

    // --- Lógia para SAIR/PARAR submenu TPU ---
  case MODE_TPU:
    if (button == BOTAO_DOWN || button == BOTAO_UP || button == BOTAO_VOLTAR) {
      menu = MODE_PARAR_TPU;
    }
    break;

  case MODE_PARAR_TPU:
    if (button == BOTAO_SELECIONAR) {
      menu = MODE_SELECT_TPU;
    }
    break;

  default:
    clean_display();
    menu = MODE_SELECT_PLA;
    break;
  }
}

void clean_display() {
  lcd.setCursor(0, 0);
  lcd.print("                    ");
  lcd.setCursor(0, 1);
  lcd.print("                    ");
  lcd.setCursor(0, 2);
  lcd.print("                    ");
  lcd.setCursor(0, 3);
  lcd.print("                    ");
}

// ---------------------
// --- Inicialização ---
void updateMenu() {
  switch (menu) {

  case INITIALIZATION:
    lcd.setCursor(0, 0);
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
    lcd.setCursor(0, 0);
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
    lcd.setCursor(0, 0);
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
    lcd.setCursor(0, 0);
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
    lcd.setCursor(0, 0);
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
    lcd.setCursor(0, 0);
    lcd.print("   FILAMENTO  PLA   ");
    lcd.setCursor(1, 1);
    lcd.print("Set:50");
    lcd.setCursor(7, 1);
    lcd.write(B11011111); // Imprime o símbolo de grau
    lcd.setCursor(8, 1);
    lcd.print("     ");
    lcd.createChar(1, A8Char); // Imprime o termômetro
    lcd.setCursor(13, 1);
    lcd.write((byte)1);
    lcd.setCursor(14, 1);
    lcd.print(t, 1);
    lcd.setCursor(18, 1);
    lcd.write(B11011111);       // Imprime o símbolo de grau
    lcd.createChar(3, A30Char); // Imprime a gota d'água
    lcd.setCursor(1, 2);
    lcd.write((byte)3);
    lcd.setCursor(2, 2);
    lcd.print(h, 1);
    lcd.print("g/m");
    lcd.createChar(4, A40Char); // Imprime o 3 cúbico
    lcd.setCursor(9, 2);
    lcd.write((byte)4);
    lcd.setCursor(0, 3);
    lcd.print(" ");
    lcd.createChar(0, A20Char); // Imprime o relógio
    lcd.setCursor(1, 3);
    // lcd.write((byte)0);
    lcd.print(timer / 60 + 1);
    lcd.setCursor(13, 3);
    lcd.print("Cancel ");
    digitalWrite(RELE_AUTO, HIGH);
    break;

    // --- Tela CONFIRMAR/CANCELAR filamento PLA ---
  case MODE_INICIAR_PLA:
    lcd.setCursor(0, 0);
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
    lcd.setCursor(0, 0);
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
    lcd.setCursor(0, 0);
    lcd.print("   FILAMENTO  PLA   ");
    lcd.setCursor(1, 1);
    lcd.print("Set:50");
    lcd.setCursor(7, 1);
    lcd.write(B11011111); // Imprime o símbolo de grau
    lcd.setCursor(8, 1);
    lcd.print("     ");
    lcd.createChar(1, A8Char); // Imprime o termômetro
    lcd.setCursor(13, 1);
    lcd.write((byte)1);
    lcd.setCursor(14, 1);
    lcd.print(t, 1);
    lcd.setCursor(18, 1);
    lcd.write(B11011111);       // Imprime o símbolo de grau
    lcd.createChar(3, A30Char); // Imprime a gota d'água
    lcd.setCursor(1, 2);
    lcd.write((byte)3);
    lcd.setCursor(2, 2);
    lcd.print(h, 1);
    lcd.print("g/m");
    lcd.createChar(4, A40Char); // Imprime o 3 cúbico
    lcd.setCursor(9, 2);
    lcd.write((byte)4);
    lcd.setCursor(0, 3);
    lcd.print(" ");
    lcd.createChar(0, A20Char); // Imprime o relógio
    lcd.setCursor(1, 3);
    lcd.write((byte)0);
    lcd.print(timer / 60 + 1);
    lcd.setCursor(12, 3);
    lcd.print(">Cancel ");
    break;

    // -----------------------------------
    // --- Tela secagem filamento PETG ---
  case MODE_PETG:
    lcd.setCursor(0, 0);
    lcd.print("   FILAMENTO PETG   ");
    lcd.setCursor(1, 1);
    lcd.print("Set:65");
    lcd.setCursor(7, 1);
    lcd.write(B11011111); // Imprime o símbolo de grau
    lcd.setCursor(8, 1);
    lcd.print("     ");
    lcd.createChar(1, A8Char); // Imprime o termômetro
    lcd.setCursor(13, 1);
    lcd.write((byte)1);
    lcd.setCursor(14, 1);
    lcd.print(t, 1);
    lcd.setCursor(18, 1);
    lcd.write(B11011111);       // Imprime o símbolo de grau
    lcd.createChar(3, A30Char); // Imprime a gota d'água
    lcd.setCursor(1, 2);
    lcd.write((byte)3);
    lcd.setCursor(2, 2);
    lcd.print(h, 1);
    lcd.print("g/m");
    lcd.createChar(4, A40Char); // Imprime o 3 cúbico
    lcd.setCursor(9, 2);
    lcd.write((byte)4);
    lcd.setCursor(0, 3);
    lcd.print(" ");
    lcd.createChar(0, A20Char); // Imprime o relógio
    lcd.setCursor(1, 3);
    lcd.write((byte)0);
    lcd.print(timer / 60 + 1);
    lcd.setCursor(13, 3);
    lcd.print("Cancel ");
    break;

    // --- Tela CONFIRMAR/CANCELAR filamento PETG ---
  case MODE_INICIAR_PETG:
    lcd.setCursor(0, 0);
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
    lcd.setCursor(0, 0);
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
    lcd.setCursor(0, 0);
    lcd.print("   FILAMENTO PETG   ");
    lcd.setCursor(1, 1);
    lcd.print("Set:65");
    lcd.setCursor(7, 1);
    lcd.write(B11011111); // Imprime o símbolo de grau
    lcd.setCursor(8, 1);
    lcd.print("     ");
    lcd.createChar(1, A8Char); // Imprime o termômetro
    lcd.setCursor(13, 1);
    lcd.write((byte)1);
    lcd.setCursor(14, 1);
    lcd.print(t, 1);
    lcd.setCursor(18, 1);
    lcd.write(B11011111);       // Imprime o símbolo de grau
    lcd.createChar(3, A30Char); // Imprime a gota d'água
    lcd.setCursor(1, 2);
    lcd.write((byte)3);
    lcd.setCursor(2, 2);
    lcd.print(h, 1);
    lcd.print("g/m");
    lcd.createChar(4, A40Char); // Imprime o 3 cúbico
    lcd.setCursor(9, 2);
    lcd.write((byte)4);
    lcd.setCursor(0, 3);
    lcd.print(" ");
    lcd.createChar(0, A20Char); // Imprime o relógio
    lcd.setCursor(1, 3);
    lcd.write((byte)0);
    lcd.print(timer / 60 + 1);
    lcd.setCursor(12, 3);
    lcd.print(">Cancel ");
    break;

    // ----------------------------------
    // --- Tela secagem filamento ABS ---
  case MODE_ABS:
    lcd.setCursor(0, 0);
    lcd.print("   FILAMENTO  ABS   ");
    lcd.setCursor(1, 1);
    lcd.print("Set:65");
    lcd.setCursor(7, 1);
    lcd.write(B11011111); // Imprime o símbolo de grau
    lcd.setCursor(8, 1);
    lcd.print("     ");
    lcd.createChar(1, A8Char); // Imprime o termômetro
    lcd.setCursor(13, 1);
    lcd.write((byte)1);
    lcd.setCursor(14, 1);
    lcd.print(t, 1);
    lcd.setCursor(18, 1);
    lcd.write(B11011111);       // Imprime o símbolo de grau
    lcd.createChar(3, A30Char); // Imprime a gota d'água
    lcd.setCursor(1, 2);
    lcd.write((byte)3);
    lcd.setCursor(2, 2);
    lcd.print(h, 1);
    lcd.print("g/m");
    lcd.createChar(4, A40Char); // Imprime o 3 cúbico
    lcd.setCursor(9, 2);
    lcd.write((byte)4);
    lcd.setCursor(0, 3);
    lcd.print(" ");
    lcd.createChar(0, A20Char); // Imprime o relógio
    lcd.setCursor(1, 3);
    lcd.write((byte)0);
    lcd.print(timer / 60 + 1);
    lcd.setCursor(13, 3);
    lcd.print("Cancel ");
    break;

    // --- Tela CONFIRMAR/CANCELAR filamento ABS ---
  case MODE_INICIAR_ABS:
    lcd.setCursor(0, 0);
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
    lcd.setCursor(0, 0);
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
    lcd.setCursor(0, 0);
    lcd.print("   FILAMENTO  ABS   ");
    lcd.setCursor(1, 1);
    lcd.print("Set:65");
    lcd.setCursor(7, 1);
    lcd.write(B11011111); // Imprime o símbolo de grau
    lcd.setCursor(8, 1);
    lcd.print("     ");
    lcd.createChar(1, A8Char); // Imprime o termômetro
    lcd.setCursor(13, 1);
    lcd.write((byte)1);
    lcd.setCursor(14, 1);
    lcd.print(t, 1);
    lcd.setCursor(18, 1);
    lcd.write(B11011111);       // Imprime o símbolo de grau
    lcd.createChar(3, A30Char); // Imprime a gota d'água
    lcd.setCursor(1, 2);
    lcd.write((byte)3);
    lcd.setCursor(2, 2);
    lcd.print(h, 1);
    lcd.print("g/m");
    lcd.createChar(4, A40Char); // Imprime o 3 cúbico
    lcd.setCursor(9, 2);
    lcd.write((byte)4);
    lcd.setCursor(0, 3);
    lcd.print(" ");
    lcd.createChar(0, A20Char); // Imprime o relógio
    lcd.setCursor(1, 3);
    lcd.write((byte)0);
    lcd.print(timer / 60 + 1);
    lcd.setCursor(12, 3);
    lcd.print(">Cancel ");
    break;

    // ----------------------------------
    // --- Tela secagem filamento TPU ---
  case MODE_TPU:
    lcd.setCursor(0, 0);
    lcd.print("   FILAMENTO  TPU   ");
    lcd.setCursor(1, 1);
    lcd.print("Set:55");
    lcd.setCursor(7, 1);
    lcd.write(B11011111); // Imprime o símbolo de grau
    lcd.setCursor(8, 1);
    lcd.print("     ");
    lcd.createChar(1, A8Char); // Imprime o termômetro
    lcd.setCursor(13, 1);
    lcd.write((byte)1);
    lcd.setCursor(14, 1);
    lcd.print(t, 1);
    lcd.setCursor(18, 1);
    lcd.write(B11011111);       // Imprime o símbolo de grau
    lcd.createChar(3, A30Char); // Imprime a gota d'água
    lcd.setCursor(1, 2);
    lcd.write((byte)3);
    lcd.setCursor(2, 2);
    lcd.print(h, 1);
    lcd.print("g/m");
    lcd.createChar(4, A40Char); // Imprime o 3 cúbico
    lcd.setCursor(9, 2);
    lcd.write((byte)4);
    lcd.setCursor(0, 3);
    lcd.print(" ");
    lcd.createChar(0, A20Char); // Imprime o relógio
    lcd.setCursor(1, 3);
    lcd.write((byte)0);
    lcd.print(timer / 60 + 1);
    lcd.setCursor(13, 3);
    lcd.print("Cancel ");
    break;

    // --- Tela CONFIRMAR/CANCELAR filamento TPU ---
  case MODE_INICIAR_TPU:
    lcd.setCursor(0, 0);
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
    lcd.setCursor(0, 0);
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
    lcd.setCursor(0, 0);
    lcd.print("   FILAMENTO  TPU   ");
    lcd.setCursor(1, 1);
    lcd.print("Set:55");
    lcd.setCursor(7, 1);
    lcd.write(B11011111); // Imprime o símbolo de grau
    lcd.setCursor(8, 1);
    lcd.print("     ");
    lcd.createChar(1, A8Char); // Imprime o termômetro
    lcd.setCursor(13, 1);
    lcd.write((byte)1);
    lcd.setCursor(14, 1);
    lcd.print(t, 1);
    lcd.setCursor(18, 1);
    lcd.write(B11011111);       // Imprime o símbolo de grau
    lcd.createChar(3, A30Char); // Imprime a gota d'água
    lcd.setCursor(1, 2);
    lcd.write((byte)3);
    lcd.setCursor(2, 2);
    lcd.print(h, 1);
    lcd.print("g/m");
    lcd.createChar(4, A40Char); // Imprime o 3 cúbico
    lcd.setCursor(9, 2);
    lcd.write((byte)4);
    lcd.setCursor(0, 3);
    lcd.print(" ");
    lcd.createChar(0, A20Char); // Imprime o relógio
    lcd.setCursor(1, 3);
    lcd.write((byte)0);
    lcd.print(timer / 60 + 1);
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
  pinMode(RELE_A, OUTPUT);
  pinMode(RELE_AUTO, OUTPUT);
  delay(1000);
  menu = MODE_SELECT_PLA;
  updateMenu();
}

void loop() {
  current_millis = millis();
  bool update_menu_sensor = false;
  bool update_menu_timer = false;

  // le os sensores a cada 2 segundos, se variou 0.1 deve atualizar o display
  if (current_millis - millis_leitura_sensores > 2000) {
    h = dht.readHumidity();
    t = dht.readTemperature();
    if (isnanf(h) || isnanf(t)) {
      h = -1.0;
      t = -1.0;
    } else {
      h = calculaUmidadeAbsoluta();
    }
    if (fabs(prev_h - h) >= 0.1f || fabs(prev_t - t) >= 0.1f) {
      update_menu_sensor = true;
    }
    prev_h = h;
    prev_t = t;
    millis_leitura_sensores = current_millis;
  }
  if (current_millis < millis_countdown_timer) {
    // aconteceu um overflow
    millis_countdown_timer = current_millis;
  }
  if (timer > 0 && current_millis - millis_countdown_timer > 1000) {
    // adiciona os estados aqui para os outros materiais
    if (menu == MODE_PLA) {
      timer--;
      if (timer == 0) {
        // acabou o tempo de secagem
        // TODO coloca os outputs esperados aqui bruno
        menu = MODE_SELECT_PLA;
      }
      update_menu_timer = true;
    } else {
      timer = 0;
    }
    millis_countdown_timer = current_millis;
  }

  checkButtons();
  if (update_menu_sensor || update_menu_timer) {
    // esta atualizacao eh para manter os valores do display se atualizando
    updateMenu();
  }
}

// ---
float calculaUmidadeAbsoluta(void) {
  float UA = ((6.112 * (pow(M_E, ((17.67 * t) / (t + 243.5)))) * h * 2.1674) /
              (273.15 + t));
  return UA;
}

void checkButtons() {
  bool upState = !digitalRead(botaoUp);
  bool downState = !digitalRead(botaoDown);
  bool voltarState = !digitalRead(botaoVoltar);
  bool selecionarState = !digitalRead(botaoSelecionar);

  if ((upState || downState || voltarState || selecionarState)) {
    if (upState &&
        (current_millis - lastDebounceTime[BOTAO_UP]) > DEBOUNCE_TIME_MS) {
      get_new_state(BOTAO_UP);
      updateMenu();
      lastDebounceTime[BOTAO_UP] = current_millis;
    } else if (downState && (current_millis - lastDebounceTime[BOTAO_DOWN]) >
                                DEBOUNCE_TIME_MS) {
      get_new_state(BOTAO_DOWN);
      updateMenu();
      lastDebounceTime[BOTAO_DOWN] = current_millis;
    } else if (voltarState &&
               (current_millis - lastDebounceTime[BOTAO_VOLTAR]) >
                   DEBOUNCE_TIME_MS) {
      get_new_state(BOTAO_VOLTAR);
      updateMenu();
      lastDebounceTime[BOTAO_VOLTAR] = current_millis;
    } else if (selecionarState &&
               (current_millis - lastDebounceTime[BOTAO_SELECIONAR]) >
                   DEBOUNCE_TIME_MS) {
      get_new_state(BOTAO_SELECIONAR);
      updateMenu();
      lastDebounceTime[BOTAO_SELECIONAR] = current_millis;
    }
  }
}