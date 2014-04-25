#include <VirtualWire.h>

int potValor=0;//Simulacion

const uint8_t SEGUNDOS_TIMEOUT_DERIVANDO=20; //Segundos que esperamos hasta parar la derivación

const uint8_t LIMITE_SUPERIOR=50; //Objetivo, tener un consumo constante de 50w
const uint8_t LIMITE_INFERIOR=0; //Objetivo, nunca por debajo de cero

float FACTOR_CONVERSOR_WATIOS=3.33;//Calculado del consumo tope del calentador/Resistencia del potenciómetro

const uint8_t VALOR_R_VARIABLE=99; //Objetivo, nunca por debajo de cero

int RTOTAL=500;

//PIN RECEPTOR NANO=
const uint8_t PIN_RADIO_FRECUENCIA=11;

const uint8_t PIN_DERIVANDO=6; //Objetivo, nunca por debajo de cero
int PIN_R[8] =      {2,3,4,5,6,7,8,9};
int VALOR_R[8] =    {200,100,100,50,25,12,6,3};
boolean ACTIV_R[8] ={true,true,true,true,true,true,true,true};

const int INDEFINIDO=999999;
boolean estaDerivando = false;

unsigned long tIniDerivandoAlMinimo=0; //Tiempo que servirá para almacenar la última vez que se superó el umbral de 0, para activar el timeout de la derivacion

long milisLecturaAnterior=0;

#define BACKLIGHT_PIN     13


void setup() {

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  delay(1000);
  Serial.println("Inicializando");

  Serial.println("www.calentadorsolarpasoapaso.blogspot.com");
  pinMode(PIN_DERIVANDO, OUTPUT); 
  
  
  inicializarPinesR();
  
  activarResistencias();

  RTOTAL=getValorActualResistencia();
  
  
  //Cerramos la salida de 220. Por defecto cerrada, pero por si acaso
  desactivarDerivacion();

  setupRadioFrecuencia();  

}

void inicializarPinesR(){
  for(int i=0;i<8;i++){
    pinMode(PIN_R[i], OUTPUT); 
  }
}

void activarResistencias(){
  for(int i=0;i<8;i++){
    activarResistencia(i);
  }
}

void desactivarResistencias(){
  for(int i=0;i<8;i++){
    desactivarResistencia(i);
  }
}

boolean estanResistenciasActivadas(){
  for(int i=0;i<8;i++){
    if(!ACTIV_R[i]) return false;
  }
  return true;
}



// the loop routine runs over and over again forever:
void loop() {
  int watts=leerValorRadioFrecuencia();

  if(watts!=INDEFINIDO){
    Serial.print("Watios leidos ");
    Serial.println(watts);
    ajustarSalida( watts);
  }  

  //  ejectutarTestResistencias();  

/*
  ajustarSalida( 20);  
  ajustarSalida( 40);  
  ajustarSalida( 50);  
  ajustarSalida( 0);  
  ajustarSalida( -100);  
  ajustarSalida( -200);  
  ajustarSalida( -230);  
  ajustarSalida( -250);  
  ajustarSalida( -850);  
  ajustarSalida( -950);  
  ajustarSalida( -1250);  
  ajustarSalida( -1350);  
  ajustarSalida( 20);  
  ajustarSalida( 200);  
  ajustarSalida( 200);  
  ajustarSalida( 1500);  
  ajustarSalida( 1500);  
  ajustarSalida( 1500);  
  ajustarSalida( 1500);  
  ajustarSalida( 1500);  
  ajustarSalida( 200);  
  ajustarSalida( 100);  
  ajustarSalida( 200);  
  ajustarSalida( 300);  
  ajustarSalida( 150);  
  */
  
}

/*
Tenemos una placa de relays de 4 elementos, lo que nos permite jugar con ellos para ajustar al máximo la salida
El chip con resistencia variable, ajustará tramos de 0-99k y en 3 puertos meteremos 1 resistencia de 100k para activar o desactivarlas
definiendo así un rango variable de 0-399kw. Notese que el potenciómetro tiene de 0-500, aunque los primeros 100 no sirven para mucho
*/

int ajustarSalida( int watios){
  int incrementoEstimadoR=0;
  uint8_t valorIncremento=0;

  //puede pasar que tenga que decrementar mÃ¡s de una resistencia de 100k
  int rTotalActual=getValorActualResistencia() ;
  
  if(watios<=LIMITE_INFERIOR){
        if(!estaDerivando){
          activarDerivacion(); //Se activa la derivación al mínimo de derivación
        }
    }

  if(estaDerivando){
  
      incrementoEstimadoR=calcularIncrementoEstimadoR(watios,rTotalActual,LIMITE_SUPERIOR,LIMITE_INFERIOR);
      modificarResistencias(incrementoEstimadoR,rTotalActual);
  
    if(estanResistenciasActivadas() && estaDerivando){
  
      //Me guardo el tiempo que lleva el sistema al mínimo de potencia derivada
  
      if(tIniDerivandoAlMinimo==0){
        tIniDerivandoAlMinimo=millis();
      }
  
      //Si derivando > 5 minutos, paramos la derivación
      if(timeoutDesactivarDerivacion()) desactivarDerivacion();
    }
    else{
      tIniDerivandoAlMinimo=0;
    }
  }


  imprimirDatos(watios,incrementoEstimadoR);  

  //Delay para ver los valores en el emulador
  delay(10000);

  return incrementoEstimadoR; 
}

float calcularWatiosDerivando(){
  float wDerivando=(RTOTAL-getValorActualResistencia())*FACTOR_CONVERSOR_WATIOS;
  return wDerivando;
}


void imprimirDatos(int watios,int incrementoEstimadoR){
    float wDerivando=calcularWatiosDerivando();
   int rTotalActual=getValorActualResistencia();

  Serial.print("Wat:\t");
  Serial.print(watios);
  Serial.print("\twDeriv:\t");
  Serial.print(wDerivando);
  Serial.print("\tRF: ");
  Serial.print(getValorActualResistencia() );
  Serial.print("\tInc:");
  Serial.print(incrementoEstimadoR);
  Serial.print(" D: ");
  Serial.print(estaDerivando);
  Serial.print(" ");
  for(int i=0;i<8;i++){
    Serial.print(ACTIV_R[i]);
  }
  Serial.println(" ");
  
}

void modificarResistencias(int incrementoEstimadoR,int rTotalActual){
  //Si la resistencia no varía, salimos
  boolean ACTIV_R_TMP[8] ={true,true,true,true,true,true,true,true};

  if (incrementoEstimadoR==0) return ;
  
  int rTotalEstimada=rTotalActual+incrementoEstimadoR;
  
  rTotalEstimada=constrain(rTotalEstimada, 0, RTOTAL);  

  //Calculamos el nuevo vector de activaciones
  int rAcumulada=0;
  
  
  for(int i=0;i<8;i++){
    rAcumulada+=VALOR_R[i];
    
    if(rAcumulada>rTotalEstimada){
      //Si sumando este valor, superamos el umbral, habrá que probar con el siguiente y este ponerlo a false
      ACTIV_R_TMP[i]=false;
      rAcumulada-=VALOR_R[i];
    }
    else{
      ACTIV_R_TMP[i]=true;
    }
  }
  
  Serial.println("");
  for(int i=0;i<8;i++){
    Serial.print(ACTIV_R_TMP[i]);
  }
  
  
  //Ahora toca activar o desactivar las que toquen
  for(int i=0;i<8;i++){
    if(ACTIV_R_TMP[i]) activarResistencia(i);
    else desactivarResistencia(i);
    
  }
  
  return ;
  }


/*
Funcion que estima el incremento o decremento que deberá tener la resistencia para aproximarnos a 0 sin sobrepasar

los límites (LIMITE_INFERIOR,LIMITE_SUPERIOR).

En un futuro, debería almacenar un histórico y basarse en los últimos x valores para realizar la estimación. ASí, las subidas abruptas serían 

estimadas  por tanto no inyectadas a red, 
El valor actual de rTotal no sirve de mucho, pero es posible que tenga que usar exponenciales para realizar la estimación

*/
  /*Calculamos la logica que nos diga cuanto incrementar
    Convertir de watios a resistencia(R)
    Para calcular la cantidad de elementos de resistencia a subir o bajar, tengo que saber
    la carga de la potencia (vamos, lo que gasta el cacharro que le enchufemos)
    No es lo mismo, si le metemos 500W que si le metemos 2000, el valor de R cambiaría

    Otra forma, sería aprendiendo cuanto sube con cada incremento simple, pero necesitaríamos memoria flash

    
  */

int calcularIncrementoEstimadoR(int watios,int rTotal,uint8_t LIMITE_SUPERIOR,uint8_t LIMITE_INFERIOR){
    int r=0;
    //Solo modificaremos la resistencia si estamos fuera de rangos. 
    //Estos valores tendrían que autocalcularse en función del calentador enchufado al potenciometro. Revisar
    if(watios>LIMITE_SUPERIOR || watios<LIMITE_INFERIOR){
//      r=map(watios, -1000, 1000, -RTOTAL, RTOTAL);
      r=watios/FACTOR_CONVERSOR_WATIOS;

    }
    //La resistencia tiene un rango de 0-400, nunca debe sobrepasarlo
    r=constrain(r, -(RTOTAL), (RTOTAL));  
   
    return r;
}

int getValorActualResistencia(){
  int r=0;
  for(int i=0;i<8;i++){
    if(ACTIV_R[i]) r+=VALOR_R[i]; //R1=100k;
  }
  return r;
}
 
void desactivarResistencia(int i){
  ACTIV_R[i]=false;
  digitalWrite(PIN_R[i],LOW);
}  

void activarResistencia(int i){
  ACTIV_R[i]=true;
  digitalWrite(PIN_R[i],HIGH);
}  
  
void activarDerivacion(){
  if(estaDerivando==false){
    estaDerivando=true;
    digitalWrite(PIN_DERIVANDO,HIGH);
  
    Serial.println("ACTIVO DERIVACION ----------------------------");
  }

}  

void desactivarDerivacion(){
  estaDerivando=false;
  digitalWrite(PIN_DERIVANDO,LOW);
  
  //no debería pasar, pero por si acaso, inicializamos Rs

  activarResistencias();
  
  tIniDerivandoAlMinimo=0;
  
    Serial.println("DETENGO DERIVACION");

}  

boolean timeoutDesactivarDerivacion(){
  return (millis()-tIniDerivandoAlMinimo)>(SEGUNDOS_TIMEOUT_DERIVANDO*1000);
}

void setupRadioFrecuencia(){
    Serial.println("Inicializando RF");

    // Initialise the IO and ISR
    vw_set_rx_pin(PIN_RADIO_FRECUENCIA);

    vw_set_ptt_inverted(true); // Required for DR3100
    vw_setup(2000);	 // Bits per sec

    vw_rx_start();       // Start the receiver PLL running
}

int leerValorRadioFrecuencia(){
    uint8_t buf[VW_MAX_MESSAGE_LEN];
    uint8_t buflen = VW_MAX_MESSAGE_LEN;

    if (vw_get_message(buf, &buflen)) // Non-blocking
    {
	int i;
        digitalWrite(13, true); // Flash a light to show received good message
	// Message with a good checksum received, dump it.
	String valor="";
	for (i = 0; i < buflen; i++)
	{
            char caracter=(char)buf[i];
              valor+=caracter;
	}
       
        String watts=valor.substring(0,valor.indexOf(' '));
	
        int iWatts=watts.toInt();
        if(iWatts==0) iWatts=INDEFINIDO;
        return iWatts;
        digitalWrite(13, false);
    }
    return INDEFINIDO;
}

void ejecutarTestResistencias(){
  desactivarResistencias();

  Serial.println("Mulimetro R debe medir 0");
  delay(10000);

  activarResistencias();  
  Serial.println("Mulimetro R debe medir 500");
  delay(10000);
}
