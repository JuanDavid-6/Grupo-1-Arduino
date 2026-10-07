#include <OneWire.h>
#include <DallasTemperature.h>

// Pin de datos del sensor DS18B20 conectado al pin A1
const int ONE_WIRE_BUS = A1; 
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

const int LED_VERDE = 3;
const int LED_AMARILLO = 4;
const int LED_ROJO = 5;

// --- Pines del Display 7 Segmentos ---
const int SEG_A = 6;
const int SEG_B = 7;
const int SEG_C = 8;
const int SEG_D = 9;
const int SEG_E = 10;
const int SEG_F = 11;
const int SEG_G = 12;

const int PIN_POTENCIOMETRO = A0;
const int BOTON_SILENCIO = A2;
const int BOTON_REINICIO = A3;
const int BUZZER = A4;

int tiempoAcumulado = 0;
unsigned long tiempoAnterior = 0;
bool alertaActiva = false;
bool sonidoSilenciado = false;

unsigned long ultimaLecturaPot = 0;
unsigned long ultimaImpresionSerial = 0;
const unsigned long TIEMPO_ANTIRREBOTE = 50;
unsigned long ultimoReboteSilencio = 0;
unsigned long ultimoReboteReinicio = 0;

const byte NUMEROS[10][7] = {
  {1, 1, 1, 1, 1, 1, 0},
  {0, 1, 1, 0, 0, 0, 0},
  {1, 1, 0, 1, 1, 0, 1},
  {1, 1, 1, 1, 0, 0, 1},
  {0, 1, 1, 0, 0, 1, 1},
  {1, 0, 1, 1, 0, 1, 1},
  {1, 0, 1, 1, 1, 1, 1},
  {1, 1, 1, 0, 0, 0, 0},
  {1, 1, 1, 1, 1, 1, 1},
  {1, 1, 1, 1, 0, 1, 1}
};

void setup() {
  Serial.begin(9600);
  sensors.begin(); // Inicializar el sensor DS18B20
  
  Serial.println("=== MONITOR DE EXPOSICION TERMICA (DS18B20) ===");

  pinMode(LED_VERDE, OUTPUT);
  pinMode(LED_AMARILLO, OUTPUT);
  pinMode(LED_ROJO, OUTPUT);
  pinMode(BUZZER, OUTPUT);

  pinMode(SEG_A, OUTPUT); pinMode(SEG_B, OUTPUT); pinMode(SEG_C, OUTPUT);
  pinMode(SEG_D, OUTPUT); pinMode(SEG_E, OUTPUT); pinMode(SEG_F, OUTPUT);
  pinMode(SEG_G, OUTPUT);

  pinMode(BOTON_SILENCIO, INPUT_PULLUP);
  pinMode(BOTON_REINICIO, INPUT_PULLUP);

  mostrarNumero(0);
  digitalWrite(LED_VERDE, HIGH);
  digitalWrite(LED_AMARILLO, LOW);
  digitalWrite(LED_ROJO, LOW);
  noTone(BUZZER);
}

void loop() {
  // Leer potenciómetro para el umbral
  int lecturaPot = analogRead(PIN_POTENCIOMETRO);
  int umbral = map(lecturaPot, 0, 1023, 20, 40);

  // --- LECTURA DEL SENSOR COMO EN EL VIDEO ---
  sensors.requestTemperatures(); // Se envía el comando para leer la temperatura[cite: 2]
  float temperatura = sensors.getTempCByIndex(0); // Se obtiene en °C[cite: 2]

  // Mostrar por Serial cada 500ms imitando el video
  if (millis() - ultimaImpresionSerial >= 500) {
    ultimaImpresionSerial = millis();
    Serial.print("Temperatura= "); 
    Serial.print(temperatura); 
    Serial.print(" C | Umbral: "); 
    Serial.print(umbral); 
    Serial.print(" C | Acumulado: "); 
    Serial.print(tiempoAcumulado); 
    Serial.println(" s");
  }

  bool sobrecalentamiento = (temperatura > umbral);

  if (sobrecalentamiento) {
    digitalWrite(LED_VERDE, LOW);
    digitalWrite(LED_AMARILLO, HIGH);
  } else {
    digitalWrite(LED_VERDE, HIGH);
    digitalWrite(LED_AMARILLO, LOW);
  }

  if (sobrecalentamiento && !alertaActiva) {
    if (millis() - tiempoAnterior >= 1000) {
      tiempoAnterior = millis();
      tiempoAcumulado++;

      if (tiempoAcumulado >= 10) {
        mostrarNumero(9);
      } else {
        mostrarNumero(tiempoAcumulado);
      }
    }
  }

  if (tiempoAcumulado >= 10 && !alertaActiva) {
    alertaActiva = true;
    sonidoSilenciado = false;
    digitalWrite(LED_ROJO, HIGH);
    Serial.println("¡ALERTA! Tiempo acumulado superado (10s).");
  }

  if (alertaActiva && !sonidoSilenciado) {
    tone(BUZZER, 1000);
  } else {
    noTone(BUZZER);
  }

  if (digitalRead(BOTON_SILENCIO) == LOW) {
    if (millis() - ultimoReboteSilencio > TIEMPO_ANTIRREBOTE) {
      ultimoReboteSilencio = millis();
      if (alertaActiva) {
        sonidoSilenciado = true;
        Serial.println("Alerta silenciada.");
      }
    }
  }

  if (digitalRead(BOTON_REINICIO) == LOW) {
    if (millis() - ultimoReboteReinicio > TIEMPO_ANTIRREBOTE) {
      ultimoReboteReinicio = millis();
      if (!sobrecalentamiento) {
        tiempoAcumulado = 0;
        alertaActiva = false;
        sonidoSilenciado = false;
        digitalWrite(LED_ROJO, LOW);
        mostrarNumero(0);
        Serial.println("Reinicio ACEPTADO.");
      } else {
        Serial.println("Reinicio RECHAZADO: Sobrecalentamiento.");
      }
    }
  }
}

void mostrarNumero(int numero) {
  if (numero < 0 || numero > 9) return;
  digitalWrite(SEG_A, NUMEROS[numero][0]);
  digitalWrite(SEG_B, NUMEROS[numero][1]);
  digitalWrite(SEG_C, NUMEROS[numero][2]);
  digitalWrite(SEG_D, NUMEROS[numero][3]);
  digitalWrite(SEG_E, NUMEROS[numero][4]);
  digitalWrite(SEG_F, NUMEROS[numero][5]);
  digitalWrite(SEG_G, NUMEROS[numero][6]);
}