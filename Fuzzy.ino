
#include"Fuzzy.h"
#define ENC_PIN_A 2 //pin kanala A enkodera
#define ENC_PIN_B 3 //pin kanala B enkodera
#define READ_ENC_A bitRead(PIND, ENC_PIN_A) 
#define READ_ENC_B bitRead(PIND, ENC_PIN_B)


#define MOTORB_1A 10  //PIN drivera motora kanal A
#define MOTORB_1B 5  //PIN drivera motora kanal B

int led1=8;
int led2=9;

float K=0;
Fuzzy *fuzzy=new Fuzzy();
int botun1=7;
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

//Ulaz
FuzzyInput *otklonKuta=new FuzzyInput(1);
FuzzySet *verySmall=new FuzzySet(1,10,14,22);
FuzzySet *small=new FuzzySet(12,21,31,40);
FuzzySet *mediumError=new FuzzySet(31,40,51,61);
FuzzySet *big=new FuzzySet(50,60,74,84);
FuzzySet *veryBig=new FuzzySet(75,83,107,110);
otklonKuta->addFuzzySet(verySmall);
otklonKuta->addFuzzySet(small);
otklonKuta->addFuzzySet(mediumError);
otklonKuta->addFuzzySet(big);
otklonKuta->addFuzzySet(veryBig);

//Izlaz
FuzzyOutput *brzina=new FuzzyOutput(1);

FuzzySet *verySlow=new FuzzySet(50,60,65,77);
FuzzySet *slow=new FuzzySet(58,85,94,116);
FuzzySet *mediumSpeed=new FuzzySet(90,120,142,167);
FuzzySet *fast=new FuzzySet(145,161,199,220);
FuzzySet *veryFast=new FuzzySet(200,222,240,250);

brzina->addFuzzySet(verySlow);
brzina->addFuzzySet(slow);
brzina->addFuzzySet(mediumSpeed);
brzina->addFuzzySet(fast);
brzina->addFuzzySet(veryFast);

fuzzy->addFuzzyInput(otklonKuta);
fuzzy->addFuzzyOutput(brzina);

//Pravila
//Ako je kut oklona jako malen
FuzzyRuleAntecedent *AkoKutOktlonaVerySmall=new FuzzyRuleAntecedent();
AkoKutOktlonaVerySmall->joinSingle(verySmall);
// Onda je brzina jako mala
FuzzyRuleConsequent *ondaBrzina_verySlow= new FuzzyRuleConsequent();
ondaBrzina_verySlow->addOutput(verySlow);

FuzzyRule *pravilo_01 = new FuzzyRule(1, AkoKutOktlonaVerySmall, ondaBrzina_verySlow);
  fuzzy->addFuzzyRule(pravilo_01);

//

FuzzyRuleAntecedent *AkoKutOktlonasmall=new FuzzyRuleAntecedent();
AkoKutOktlonasmall->joinSingle(small);

FuzzyRuleConsequent *ondaBrzina_slow= new FuzzyRuleConsequent();
ondaBrzina_slow->addOutput(slow);

FuzzyRule *pravilo_02 = new FuzzyRule(2, AkoKutOktlonasmall, ondaBrzina_slow);
  fuzzy->addFuzzyRule(pravilo_02);
//
FuzzyRuleAntecedent *AkoKutOktlonamediumError=new FuzzyRuleAntecedent();
AkoKutOktlonamediumError->joinSingle(mediumError);

FuzzyRuleConsequent *ondaBrzina_mediumSpeed= new FuzzyRuleConsequent();
ondaBrzina_mediumSpeed->addOutput(mediumSpeed);

FuzzyRule *pravilo_03 = new FuzzyRule(3, AkoKutOktlonamediumError, ondaBrzina_mediumSpeed);
  fuzzy->addFuzzyRule(pravilo_03);
//
FuzzyRuleAntecedent *AkoKutOktlonabig=new FuzzyRuleAntecedent();
AkoKutOktlonabig->joinSingle(big);

FuzzyRuleConsequent *ondaBrzina_fast= new FuzzyRuleConsequent();
ondaBrzina_fast->addOutput(fast);

FuzzyRule *pravilo_04 = new FuzzyRule(4, AkoKutOktlonabig, ondaBrzina_fast);
  fuzzy->addFuzzyRule(pravilo_04);
//
FuzzyRuleAntecedent *AkoKutOktlonaveryBig=new FuzzyRuleAntecedent();
AkoKutOktlonaveryBig->joinSingle(veryBig);

FuzzyRuleConsequent *ondaBrzina_veryFast= new FuzzyRuleConsequent();
ondaBrzina_veryFast->addOutput(veryFast);

FuzzyRule *pravilo_05 = new FuzzyRule(5, AkoKutOktlonaveryBig, ondaBrzina_veryFast);
  fuzzy->addFuzzyRule(pravilo_05);
//


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

  fuzzy->setInput(1,abs((kut_nagib-kut)));

  fuzzy->fuzzify();
    
  brzina = (kut_nagib-kut)>=0?(fuzzy->defuzzify(1)):(fuzzy->defuzzify(1))*(-1);


  //postavljanje limita, za max PWM signal
   if (brzina>255)
    brzina=255;
   else if (brzina<-255)
    brzina=-255;
   else if (brzina<50 &&brzina>-50)
    brzina=0;

    

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
  Serial.print(" ");
  Serial.print((kut));
  Serial.print(" ");
  Serial.println(kut_nagib);
   Serial.print("brzina");
  Serial.println(brzina);

  

  //gotov ispis stanja
}

  }
