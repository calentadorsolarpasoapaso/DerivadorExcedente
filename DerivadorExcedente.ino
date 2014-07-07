#include <VirtualWire.h>

int potValor=0;//Simulacion

const uint8_t SEGUNDOS_TIMEOUT_DERIVANDO=60; //Segundos que esperamos hasta parar la derivación

const uint8_t LIMITE_SUPERIOR=50; //Objetivo, tener un consumo constante de 50w
const uint8_t LIMITE_INFERIOR=0; //Objetivo, nunca por debajo de cero

//float FACTOR_CONVERSOR_WATIOS=(3.33)/2;//Calculado del consumo tope del calentador/Resistencia del potenciómetro
float FACTOR_CONVERSOR_WATIOS_R_SECUNDARIA=0.14; // bombilla70w

float FACTOR_CONVERSOR_WATIOS=(3.33)/2;//Calculado del consumo tope del calentador/Resistencia del potenciómetro

//const uint8_t VALOR_R_VARIABLE=99; //Objetivo, nunca por debajo de cero

int RTOTAL=500;

//Estos w son el mínimo que consume la resistencia principal activa. Cuando la generación sea de 0 a MINIMO_WATIOS_RPRINCIPAL cambiaremos
//la derivación a una segunda resistencia (bombilla de 120W por ejemplo)
int MINIMO_WATIOS_RPRINCIPAL=100; 

//PIN RECEPTOR NANO=
const uint8_t PIN_RADIO_FRECUENCIA=11;
const uint8_t PIN_DERIVANDO1=10; 
const uint8_t PIN_DERIVANDO2=10; 
const uint8_t PIN_R_PRINCIPAL_O_SECUNDARIA1=12; 
const uint8_t PIN_R_PRINCIPAL_O_SECUNDARIA2=12; 

int PIN_R[8] =      {2,3,4,5,6,7,8,9};

int VALOR_R[8] =    {200,100,100,50,25,12,6,3};
int ACTIV_R[8] ={true,true,true,true,true,true,true,true};

//Vector de consumo aproximado, más exacto que el calculado
int vectorConsumo[255][2];

const int INDEFINIDO=999999;
boolean estaDerivando = false;

unsigned long tIniDerivandoAlMinimo=0; //Tiempo que servirá para almacenar la última vez que se superó el umbral de 0, para activar el timeout de la derivacion

long milisLecturaAnterior=0;

#define BACKLIGHT_PIN     13


void setup() {

  // initialize serial communication at 9600 bits per second:
  Serial.begin(9600);
  Serial.println("Inicializando");

  delay(1000);

  Serial.println("www.calentadorsolarpasoapaso.blogspot.com");
  pinMode(PIN_DERIVANDO1, OUTPUT); 
  pinMode(PIN_DERIVANDO2, OUTPUT); 
  
  
  inicializarPinesR();
  inicializarPinesR2();
  
  activarResistencias();

  RTOTAL=getValorActualResistencia();
  
  //Cerramos la salida de 220. Por defecto cerrada, pero por si acaso
  desactivarDerivacion();

  setupRadioFrecuencia();  
<<<<<<< HEAD

}

void inicializarPinesR2(){
    pinMode(PIN_R_PRINCIPAL_O_SECUNDARIA1, OUTPUT); 
    pinMode(PIN_R_PRINCIPAL_O_SECUNDARIA2, OUTPUT); 
}
void derivarARSecundaria(){
    digitalWrite(PIN_R_PRINCIPAL_O_SECUNDARIA1, LOW);
    digitalWrite(PIN_R_PRINCIPAL_O_SECUNDARIA2, LOW);
}

void derivarARPrimaria(){
    digitalWrite(PIN_R_PRINCIPAL_O_SECUNDARIA1, HIGH);
    digitalWrite(PIN_R_PRINCIPAL_O_SECUNDARIA2, HIGH);
}

boolean estaDerivandoRSecundaria(){
  return digitalRead(PIN_R_PRINCIPAL_O_SECUNDARIA1)==LOW;
=======
  
//  ajustarDerivador();

//  inicializarVectorConsumo();

>>>>>>> origin/AjusteExactoVectorizado
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

boolean estanResistenciasDesactivadas(){
  for(int i=0;i<8;i++){
    if(ACTIV_R[i]) return false;
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
Tenemos una placa de relays de 8 elementos, lo que nos permite jugar con ellos para ajustar al máximo la salida
*/

int ajustarSalida( int watios){
  int incrementoEstimadoR=0;
  uint8_t valorIncremento=0;

  //puede pasar que tenga que decrementar mÃ¡s de una resistencia de 100k
  int rTotalActual=getValorActualResistencia() ;
  
  if(watios<=LIMITE_INFERIOR){
          activarDerivacion(); //Se activa la derivación al mínimo de derivación
    }
  

  if(estaDerivando){

     //Si están activadas todas las resistencias y el consumo está entre 0 y - MINIMO_WATIOS_RPRINCIPAL tenemos que activar la R2
     Serial.println(estanResistenciasActivadas());
     Serial.println(estanResistenciasActivadas());
     
     if((estanResistenciasActivadas() || estaDerivandoRSecundaria()) && (watios<MINIMO_WATIOS_RPRINCIPAL && watios>(-MINIMO_WATIOS_RPRINCIPAL))){  
       if(watios>LIMITE_SUPERIOR || watios<LIMITE_INFERIOR){
        Serial.println("Bombilla");
        derivarARSecundaria();
        incrementoEstimadoR=calcularIncrementoEstimadoR(watios,rTotalActual,LIMITE_SUPERIOR,LIMITE_INFERIOR,FACTOR_CONVERSOR_WATIOS_R_SECUNDARIA);
       }
       else{ //Watios 0-50
         desactivarDerivacion();
       
       }
      }
      else{
        Serial.println("Calefactor");
        
        derivarARPrimaria();
        incrementoEstimadoR=calcularIncrementoEstimadoR(watios,rTotalActual,LIMITE_SUPERIOR,LIMITE_INFERIOR,FACTOR_CONVERSOR_WATIOS);
      }

      modificarResistencias(watios,incrementoEstimadoR,rTotalActual);

    if(estaDerivando && estanResistenciasActivadas()){
  
      //Me guardo el tiempo que lleva el sistema al mínimo de potencia derivada
      if(tIniDerivandoAlMinimo==0){
        tIniDerivandoAlMinimo=millis();
      }
  
      //Si derivando > segundos predefinidos, paramos la derivación
      if(timeoutDesactivarDerivacion()) desactivarDerivacion();
    }
    else{
      tIniDerivandoAlMinimo=0;
    }
  }


  imprimirDatos(watios,incrementoEstimadoR);  

  //Delay para ver los valores en el emulador
  delay(1000);

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

void modificarResistencias(int watios,int incrementoEstimadoR,int rTotalActual){
  //Si la resistencia no varía, salimos
  int ACTIV_R_TMP[8] ={true,true,true,true,true,true,true,true};

  if (incrementoEstimadoR==0) return ;
  
  int rTotalEstimada=rTotalActual+incrementoEstimadoR;

  //Si el valor era negativo, quiero que se quede por encima a cero en el ajuste, por lo que prefiero pasarme
  if(watios<0){
      rTotalEstimada-=10;
  }

  //Aqui puede pasar que la rTotalEstimada sea muy grande, y entonces es mejor apagar el sistema
  //Para que la Resistencia no consuma el mínimo
  Serial.println("RTotalEstimada: " + rTotalEstimada);
  if(rTotalEstimada>(RTOTAL + 30) )
  {
    Serial.println("Se desactiva derivacion porque el consumo es muy alto");
    desactivarDerivacion();
  }
  else{
      
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
    cambiarResistencias(ACTIV_R_TMP);
    }//Fin else
  
  
  return ;
  }

void cambiarResistencias(int ACTIV_R_TMP[8]){
    for(int i=0;i<8;i++){
      if(ACTIV_R_TMP[i]) activarResistencia(i);
      else desactivarResistencia(i);
    }
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

int calcularIncrementoEstimadoR(int watios,int rTotal,uint8_t LIMITE_SUPERIOR,uint8_t LIMITE_INFERIOR,float FACTOR_CONVERSOR){
    int r=0;
    //Solo modificaremos la resistencia si estamos fuera de rangos. 
    //Estos valores tendrían que autocalcularse en función del calentador enchufado al potenciometro. Revisar
    if(watios>LIMITE_SUPERIOR || watios<LIMITE_INFERIOR){
//      r=map(watios, -1000, 1000, -RTOTAL, RTOTAL);
      r=watios/FACTOR_CONVERSOR;

    }
    //La resistencia tiene un rango de 0-400, nunca debe sobrepasarlo
    //JJV r=constrain(r, -(RTOTAL), (RTOTAL));  
    //Si estamos por encima de 0w no queremos volver otra vez a <0 por lo que ajustamos un poco por encima
    if(watios>0){
      r=r-20;
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
    estaDerivando=true;
    digitalWrite(PIN_DERIVANDO1,LOW);
    digitalWrite(PIN_DERIVANDO2,LOW);
  
    Serial.println("ACTIVO DERIVACION ----------------------------");

}  

void desactivarDerivacion(){
  estaDerivando=false;
  digitalWrite(PIN_DERIVANDO1,HIGH);
  digitalWrite(PIN_DERIVANDO2,HIGH);
  
  //no debería pasar, pero por si acaso, inicializamos Rs

  //activarResistencias();
  
  tIniDerivandoAlMinimo=0;
  
    Serial.println("DETENGO DERIVACION");

}  

boolean timeoutDesactivarDerivacion(){
  return (millis()-tIniDerivandoAlMinimo)>(SEGUNDOS_TIMEOUT_DERIVANDO*1000);
}

void setupRadioFrecuencia(){
    Serial.println("Inicializando RF");
    pinMode(BACKLIGHT_PIN, OUTPUT);
    digitalWrite(BACKLIGHT_PIN, LOW);

    // Initialise the IO and ISR
    vw_set_rx_pin(PIN_RADIO_FRECUENCIA);

    vw_set_ptt_inverted(true); // Required for DR3100
    vw_setup(2000);	 // Bits per sec

    vw_rx_start();       // Start the receiver PLL running
}

int leerValorRadioFrecuencia(){
    uint8_t buf[50];
    uint8_t buflen = 50;

    if (vw_get_message(buf, &buflen)) // Non-blocking
    {
	int i;
        digitalWrite(BACKLIGHT_PIN, HIGH); // Flash a light to show received good message
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
        digitalWrite(BACKLIGHT_PIN, LOW);
        return iWatts;
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
<<<<<<< HEAD
=======


float rellenarVectorDerivador(){
   Serial.println("Ajustando Derivador ");
   delay(5000);

   activarResistencias();
   activarDerivacion();

   float fcv=FACTOR_CONVERSOR_WATIOS;

   int wattsBase=leerValorRadioFrecuencia();

   delay (5000); //Esperamos un segundo y medimos
    
   Serial.println("DesactivarR");
   //Damos máxima potencia a la resistencia y medimos después
   desactivarResistencias(); //Establece máxima potencia
   
   delay (5000); //Esperamos un segundo y medimos

   int wattsMaximo=leerValorRadioFrecuencia();

   Serial.println("ActivarR");

   //Damos máxima potencia a la resistencia y medimos después
   activarResistencias(); //Establece máxima potencia
   
   delay (5000); //Esperamos un segundo y medimos
   
   int wattsMinimo=leerValorRadioFrecuencia();
  
   //Watios que consume el derivador
   int wattsResistorMax=(wattsMaximo-wattsBase);
   int wattsResistorMin=(wattsMinimo-wattsBase);
   
 
   //Ahora que tenemos los watios que consume el derivador, calculamos el factor
   //En las pruebas, la resistencia es lineal
   float factor=(wattsResistorMax-wattsResistorMin)/500; //Ejm: 100-2000w= 1900/500=3,8 80-1000=1,84
   
   Serial.print("Base:");
   Serial.print(wattsBase);
   Serial.print(" Max:");
   Serial.print(wattsMaximo);
   Serial.print(" Min:");
   Serial.print(wattsMinimo);
   Serial.print(" wMax:");
   Serial.print(wattsResistorMax);
   Serial.print(" wMin:");
   Serial.print(wattsResistorMin);
   Serial.print(" factor RECOMENDADO:");
   Serial.println(factor);
   
   if(factor>0){
     Serial.println("Se establece el factor de conversión calculado automáticamente");
     FACTOR_CONVERSOR_WATIOS=factor;      
   }
}


void inicializarVectorConsumo(){
     Serial.println("Ajustando Derivador con Vector");

/*
    for(int i=0;i<255;i++){
      vectorConsumo[i][0]=0;
      vectorConsumo[i][1]=0;
    }
*/
 //    delay(5000);
  
     activarResistencias();
     activarDerivacion();
     return; 
     delay(2000);
  
     int wattsBase=leerValorRadioFrecuencia();
  
     delay(2000);
     
    for(int i=255;i>0;i--){
      //Incrementamos R
      int posicionResistencias[8];
      for(int iR=0;iR<8;iR++){
        posicionResistencias[iR] = vectorizaNumero(i,iR);
      }
   
     cambiarResistencias(posicionResistencias);
     delay(2000);
  
     int wattsRI=leerValorRadioFrecuencia();
    
      //rellenamos vector
      vectorConsumo[i][1]=(wattsRI-wattsBase);    
      vectorConsumo[i][0]=i;    

      Serial.print(vectorConsumo[i][0]);
      Serial.print(" ");
      Serial.print(vectorConsumo[i][1]);
      Serial.println("");
      
  }
    
    Serial.println("CONSUMO: ");
    for(int i=0;i<255;i++){
      Serial.print(vectorConsumo[i][0]);
      Serial.print(" ");
      Serial.print(vectorConsumo[i][1]);
      Serial.println("");
    }  
}

int vectorizaNumero(int numero,int pos){
  
  int ACTIV_R_TMP[8] ={true,true,true,true,true,true,true,true};

  for(int i=0; i<8; i++){ //Ciclo para ver que leds prendo
    ACTIV_R_TMP[i]=bitRead(numero, i);
  }
  return ACTIV_R_TMP[pos];
  
}

>>>>>>> origin/AjusteExactoVectorizado
