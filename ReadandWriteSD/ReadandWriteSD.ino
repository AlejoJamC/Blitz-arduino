/*EducaChip – Cómo Leer Y Escribir Datos En La Tarjeta SD De Arduino*/
/* www.educachip.com */

//Se incluye la librería <SD.h>
#include <SD.h>

  File Archivo;

void setup() {

  //Se esablece comunicación con el monitor serial para la comprobación de la
  //carga de datos.
  Serial.begin(9600);

  //Se muestra por pantalla que se va a iniciar la comunicación con la SD
  Serial.print("Comenzando la comunicación con la tarjeta SD");

  //Se establece como salida el pin correspondiente a SS.
  pinMode(10, OUTPUT);

  //Se muestra por el monitor si la comunicación se ha establecido correctamente
  //o ha habido algún tipo de error.
  if (!SD.begin(10)) {

    return;
  }
  Serial.println("Se ha iniciado la comunicación correctamente");

  /* ESCRIBIENDO DATOS EN LA MEMORIA SD DE ARDUINO */

  //Se abre el documento sobre el que se va a leer y escribir.
  Archivo = SD.open("datos.txt", FILE_WRITE);

  //Se comprueba que el archivo se ha abierto correctamente y se procede a
  //escribir en él.
  if (Archivo) {

    //Se escribe información en el documento de texto datos.txt.
    Archivo.println("Esto es lo que se está escribiendo en el archivo");

    //Se cierra el archivo para almacenar los datos.
    Archivo.close();

    //Se muestra por el monitor que los datos se han almacenado correctamente.
    Serial.println("Todos los datos fueron almacenados");
  }

  //En caso de que haya habido problemas abriendo datos.txt, se muestra por pantalla.
  else {

    Serial.println("El archivo datos.txt no se abrió correctamente");
  }

  /* FIN DE LA ESCRITURA DE DATOS EN LA MEMORIA SD DE ARDUINO */

  /* LEYENDO DATOS EN LA MEMORIA SD DE ARDUINO */

  //Se vuelve a abrir el fichero, esta vez para leer los datos escritos.
  Archivo = SD.open("datos.txt");

  //Si el archivo se ha abierto correctamente se muestran los datos.
  if (Archivo) {

    //Se muestra por el monitor que la información que va a aparecer es la del
    //archivo datos.txt.
    Serial.println("Información contenida en datos.txt: ");

    //Se implementa un bucle que recorrerá el archivo hasta que no encuentre más
    //información (Archivo.available()==FALSE).
    while (Archivo.available()) {

      //Se escribe la información que ha sido leída del archivo.
      Serial.write(Archivo.read());
    }

    //Si todo ha ido bien cierra el archivo para no perder datos.
    Archivo.close();
  }

  //En caso de que haya habido problemas abriendo datos.txt, se muestra por pantalla.
  else {
    Serial.println("El archivo datos.txt no se abrió correctamente");
  }

}

void loop() {
  //En este ejemplo el bucle loop() no realiza ninguna acción ya que toda la información
  //fue gestionada en el setup.
  //En caso de que se desee almacenar la información obtenida de algún sensor, la escritura
  //debería realizarse en el loop().
}
