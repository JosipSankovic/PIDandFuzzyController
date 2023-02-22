
#define ENC_PIN_A 2 //pin kanala A enkodera
#define ENC_PIN_B 3 //pin kanala B enkodera
#define READ_ENC_A bitRead(PIND, ENC_PIN_A) 
#define READ_ENC_B bitRead(PIND, ENC_PIN_B)


#define MOTORB_1A 10  //PIN drivera motora kanal A
#define MOTORB_1B 5  //PIN drivera motora kanal B

int led1=8;
int led2=9;

float Kp=9.06, Ki=0.01, Kd=0.025;    //varijable koje podešavamo, ovisno o brzini
float error=0, P=0, I=0, D=0, PID_value=0;
float previous_er=0, previous_I=0;

int botun1=7;


void calculate_pid() //računa PID vrijednost
{
    P = error;
    I = I + error; //Integralni izraz
    D = (error-previous_er); //Derivacijski izraz

    // PID_value smanjen za 50% kako bi robot imao sporije promjene
    // smjera pri kretanju
    PID_value = ((Kp*P) + (Ki*I) + (Kd*D));
   
    previous_er=error; // određuje derivacijski izraz
}
#define korekcija_kuta 0

float kut_nagib;


volatile long encBrojac = 0;

int brzina=0; //PWM signal prema motoru
long timer_ms; //zapis trenutnog vremena
long timer_ms_last; //zapis posljednjeg izvrsavanja regulatora


void setup()
{
   Serial.begin(115200);
  pinMode(ENC_PIN_A, INPUT_PULLUP);
    pinMode(ENC_PIN_B, INPUT_PULLUP);
     pinMode(botun1, INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(ENC_PIN_A), encISR, CHANGE);


  pinMode(MOTORB_1A, OUTPUT);
  pinMode(MOTORB_1B, OUTPUT);

  pinMode(led1, OUTPUT);
  pinMode(led2, OUTPUT);

  
  digitalWrite(MOTORB_1A, LOW);
  digitalWrite(MOTORB_1B, LOW);
  timer_ms_last=millis();

}
void encISR()
  {
        if (READ_ENC_A == HIGH)//kanal A L->H (rastuci brid)
            READ_ENC_B == HIGH ? encBrojac++ : encBrojac--;
            else //kanal A H->L (padajuci brid)
            READ_ENC_B == HIGH ? encBrojac-- : encBrojac++;
  }

void motorSmjer1(int setSpeed)
  {
    digitalWrite(MOTORB_1A, LOW);
    analogWrite(MOTORB_1B, setSpeed);
  }
void motorSmjer2(int setSpeed)
  {
    analogWrite(MOTORB_1A, setSpeed);
    digitalWrite(MOTORB_1B, LOW);
  }
void motorStop()
  {
    digitalWrite(MOTORB_1A, LOW);
    digitalWrite(MOTORB_1B, LOW);
  }


void loop() { 

 //pritiksom tipke 1 se platfoma pomice. drzite pritisnuto dok se ne postavi u horizontalni polozaj.
  if (digitalRead(botun1)==LOW)
  
    {
      encBrojac=0;
      motorSmjer1(110);
      
  }
  else

{

  timer_ms=millis(); //trenutno vrijeme mikrokontrolera, vazno za digitalne regulatore
  int delta_time=timer_ms-timer_ms_last; //koliko je vremena proteklo
  timer_ms_last=timer_ms; //zapamti kada je regulator posljednji out izvrsen
    
    
  //ocitanje kuta potenciometra, nagib klackalice
  kut_nagib = -(map(analogRead(A0),0,1023,0,270)-135+korekcija_kuta);

  //ocitanje kuta sa enodera
  float kut = encBrojac/(2.15);
  
  if ((kut_nagib-kut)>3 ||(kut_nagib-kut)<-3) {
  error=(kut_nagib-kut);
  calculate_pid();
  brzina=PID_value;
  
 }else
  brzina=0;

 

  //postavljanje limita, za max PWM signal
   if (brzina>255)
    brzina=255;
   if (brzina<-255)
    brzina=-255;
    
  

  //upravljacki signali prema motoru!
  
   if ((brzina)>0)  //pozitivna rotacija
   {
    motorSmjer1(brzina);
    digitalWrite(led1,HIGH);
    digitalWrite(led2,LOW);
   }
    

   //if ((kut-kut_BNO)<-15) //negativna rotacija
   else if ((brzina)<0) 
   {
    motorSmjer2(-brzina);
    digitalWrite(led2,HIGH);
    digitalWrite(led1,LOW);
   }

   else   //nema rotacije
   {
   motorStop(); 
   digitalWrite(led1,LOW);
   digitalWrite(led2,LOW);
   }

  //ispis stanja
  //vrijeme u ms     kut nagiba   stanje_enc_brojaca    kut_osovine brzina
  Serial.print(millis());
  Serial.print(" zeljeniKut ");
  Serial.print((kut_nagib-kut));
  Serial.print(" KutIzlaz ");
  Serial.print((kut_nagib));
  Serial.print(" PID ");
  Serial.println(PID_value);
  

  //gotov ispis stanja
}

  }
