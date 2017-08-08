#define dir 7
#define pwm 6

#define en_A 5
//#define en_B 2
boolean script_dir = 0;
//******************
#define time_wait 2000
#define step_1 100
#define step_2 560
#define time_wait_script random(1000,30000)
#define speed_men 180

#define impuls_per_mm 5.1
void setup() {
  Serial.begin(9600);
  // put your setup code here, to run once:
  pinMode(dir, OUTPUT);
  pinMode(pwm, OUTPUT);
  pinMode(en_A, INPUT);
 // pinMode(en_B, INPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  /* TEST
  shift(1, 80, 20);
  //rotate(1,50);
  delay(2000);
  shift(0, 80, 20);
 // rotate(0,50);
  delay(2000);*/
  /* ***Script**** */
  shift(script_dir,  step_1 , speed_men); //1) подход к полосе движения - сдвиг на ширину фигурки;
  delay(time_wait); //2) ожидание 2(?)секунды (возможно, изменим), машинки, приближающиеся к переходу,распознают пешеходов и останавливаются;
  shift(script_dir,  step_2 , speed_men);//3) движение фигурок собственно по переходу;
  delay(time_wait); //2) ожидание 2(?)секунды
  shift(script_dir,  step_1 , speed_men);//4) уход с полосы движения - сдвиг на ширину фигурки;
  delay(time_wait_script); //5) ожидание 20 секунд и возврат к п.1. Возможно, интервал до появления "следующих"пешеходов следует сделать случайным в некотором диапазоне.
  script_dir = !script_dir; 

}

int shift(int direct, int dist, int Speed) // void shift belt on dist in mm with speed 0-100% кратно impuls_per_mm
{
  int en_state = digitalRead(en_A);
  int count_pulse;
  int pulse = 0;
  int pulse_need = dist * impuls_per_mm;
  while(pulse < pulse_need)
  {
    rotate(direct,Speed);
    if(en_state != digitalRead(en_A))
    {
      pulse++;
      en_state = digitalRead(en_A);
      Serial.println(pulse);
    }
    }
  rotate(direct,0);
  return 1;
}

void rotate(int direct, int speeds) //void rotate motor 1-forward 0 backward with speeds 0-100%
{
  digitalWrite(dir, direct);
  analogWrite(pwm, map(speeds,0,100,0,255));      
}

