// instancio las variables globales y librerias requeridas
#include <SD.h>
File Archivo;

void setup() {
  //Se esablece comunicación con el monitor serial para la comprobación de la
  //carga de datos.
  Serial.begin(9600);
  if (!SD.begin(10)) {

    return;
  }
  // Se crear el archivo
  Archivo = SD.open("datos.txt", FILE_WRITE);
}

void loop() {
  // Lectura del puerto analogo de la señal resultante del filtro de pasabanda
  int sensorValue = analogRead(A0);
  // Conversion analogo a digital
  float voltage = sensorValue * (5.0 / 1023.0);

  // Se intenta abrir el archivo
  Archivo = SD.open("datos.txt");
  if (Archivo) {

    //Se muestra por el monitor que la información que va a aparecer es la del
    //archivo datos.txt.
    Serial.print(voltage);
    Serial.println('\n\r');

    //Se implementa un bucle que recorrerá el archivo hasta que no encuentre más
    //información (Archivo.available()==FALSE).
    while (Archivo.available()) {

      //Se escribe la información que ha sido leída del archivo.
      Serial.write(Archivo.read());
    }

    //Si todo ha ido bien cierra el archivo para no perder datos.
    Archivo.close();
  }
  else {
    // Mensaje en caso de error
    Serial.println("El archivo datos.txt no se abrió correctamente");
  }  
}
