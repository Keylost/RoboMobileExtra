#include <Servo.h>

//#define FWD_BWD_REVERSE //раскоментируйете или закоментируйте эту строчку, если робот едет назад, когда ожидается вперед
#define POLOLU_MAGNETIC_ENCODER
//#define POLOLU_OPTICAL_ENCODER
#define BATTARY_TEST //если раскоментировано, то один раз, то будет Ардуино будет включать буззер при разряде штатных LiPo батарей
//#define BARRIER_TEST //если раскоментировано, то будет включаться буззер и выключаться двигатель, если машинка долго не может сдвинуться с места
//пины для энкодера
//(!!! ардуино уно один из пинов обязательно на второй или третий пин
//так как они с прерываниями, второй - любой
//второй пин можно не использовать так как мы знаем направление движения
//и важно определять только скорость
#define encoder0PinA  3
#define encoder0PinB  2

#if defined(BATTARY_TEST)
#define BATTARY1_PIN A5
#define BATTARY2_PIN A6

#define voltageFilterL 0.3
#define BATTARY1_RK 5.09*2
#define BATTARY2_RK 5.09*2
#endif

#define BUZZER_PIN 13

#define upwm_pin 6 //пин для управления скоростью мотора. канал M1 на шилде
#define udir_pin 5 //пин для управления направлением вращения. канал M1 на шилде
#define uservo_pin 9 //пин куда подключен сервомотор

#define head_light_pin A4 //передние фары
#define left_indicator_pin A3 //левый поворотник
#define right_indicator_pin A2 //правый поворотник
#define stop_indicator_pin A1 //стоп сигналы
#define rear_light_pin A0 // задние фары

#define turn_signal_freq 500 //частота моргания поворотников в миллисекундах
#define deviation 14 //значение угла поворота сервомотора в градусах при котором будут включены поворотники
#define angle_range 35
#define angle_center 90
#define speed_min 450

int Speed = 0,old_Speed=0; // Скорость в условных единицах
long mvolts = 0; //входное напряжение
float real_speed = 0; //реальная скорость в сантиметрах в секунду
int DIR = 0;
int Corner = 90,old_Corner = 0; // угол поворота в градусах
Servo myservo;
char cur_state = 0;
int kod = 0;
unsigned long time;
unsigned long right_time_indicator;
unsigned long left_time_indicator;
unsigned long time_current;
unsigned long last_speed_update =0;
boolean turn_right_light;
boolean turn_left_light;
volatile unsigned int encoder0Pos = 0;

#if defined(BATTARY_TEST)
float BATTARY1_U = 7.4;
float BATTARY2_U = 7.4;
float bt1_tmp = 0;
float bt2_tmp = 0;
unsigned long last_battary_test = 0;
void battary_test_fnc();
#endif

#ifdef BARRIER_TEST
boolean barrier_flag = false;
unsigned long barrier_tm_ind = 0;
boolean barrier_beep_flag = false;
unsigned long barrier_beep_time = 0;
boolean barrier_stop_flag = false;
#endif

enum directions
{
  FORWARD = 1,
  BACKWARD = 2
};

class Motor
{
  private:
  
  int PWM_PIN; //pin on motorshild to control pwm
  int DIR_PIN; //pin on motorshild to control direction
  directions _dir;
  public:
  Motor(int PWM_PIN,int DIR_PIN)
 {
   this->PWM_PIN = PWM_PIN;
   this->DIR_PIN = DIR_PIN;
   _dir = FORWARD;
   pinMode(PWM_PIN, OUTPUT); 
   pinMode(DIR_PIN, OUTPUT);
   digitalWrite(DIR_PIN, LOW);
   digitalWrite(PWM_PIN, LOW);
 }
 void set_direction(directions dir)
 {
   if(!_dir != dir)
   {
       int tmp = DIR_PIN;
       DIR_PIN = PWM_PIN;
       PWM_PIN = tmp;
   }
   digitalWrite(DIR_PIN, LOW);
   digitalWrite(PWM_PIN, LOW);
   _dir = dir;
   
 }
 void set_speed(int Speed)
 {
   analogWrite(PWM_PIN, Speed);
 }
};

Motor motor1(udir_pin,upwm_pin);

void setup(void)
{  
  Serial.begin(115200);
  Serial.setTimeout(20);
  myservo.attach(uservo_pin);
  //#ifndef FWD_BWD_REVERSE
  motor1.set_direction(FORWARD);
  //#else
  //motor1.set_direction(BACKWARD);
  //#endif
  pinMode(rear_light_pin,OUTPUT);
  pinMode(head_light_pin,OUTPUT);
  pinMode(stop_indicator_pin,OUTPUT);
  pinMode(left_indicator_pin,OUTPUT);
  pinMode(right_indicator_pin,OUTPUT);
  digitalWrite(rear_light_pin, HIGH);
  digitalWrite(head_light_pin, HIGH);
  digitalWrite(left_indicator_pin,LOW);
  digitalWrite(right_indicator_pin,LOW);
  digitalWrite(stop_indicator_pin,LOW);
  time = 0;
  right_time_indicator = 0;
  left_time_indicator = 0;
  time_current = 0;
  turn_right_light = false;
  turn_left_light = false;

  #if defined(BATTARY_TEST)
  pinMode(BATTARY1_PIN, INPUT);
  pinMode(BATTARY2_PIN, INPUT);
  pinMode(BUZZER_PIN, OUTPUT);
  digitalWrite(BUZZER_PIN, LOW); //буззер

  BATTARY1_U = (analogRead(A5)*BATTARY1_RK)/1024.0;
  BATTARY2_U = (analogRead(A6)*BATTARY2_RK)/1024.0;
  #endif
  
  //set up encoder start
  //pinMode(encoder0PinA, INPUT_PULLUP); 
  pinMode(encoder0PinA, INPUT);
  pinMode(encoder0PinB, INPUT);
  attachInterrupt(1, doEncoder, FALLING); // encoder pin on interrupt 0 - pin 2 
  attachInterrupt(0, doEncoder, FALLING);
  //set up encoder end

  //system initialization (wait + signal) 
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100); 
  digitalWrite(BUZZER_PIN, LOW);
  delay(150);
  digitalWrite(BUZZER_PIN, HIGH);
  delay(100);
  digitalWrite(BUZZER_PIN, LOW);
}

void turnsignal_illumination();
void update_speed();

float error = 0;
#define kp = 0.5;
#define ki = 0.1;
float integral = 0;
int regulator = 0;
int regulatorOld = 0;

void loop(void)
{
  #if defined(BATTARY_TEST)
  if(millis() - last_battary_test>1000)
  {
    battary_test_fnc();
    last_battary_test = millis();
  }
  #endif
  time_current = millis();

  #if defined(BARRIER_TEST)
  if(BATTARY1_U>5.1 && real_speed == 0 && Speed>0)
  {
    digitalWrite(BUZZER_PIN, HIGH);
    if(!barrier_flag)
    {
      barrier_flag = true;
      barrier_tm_ind = time_current;
    }
    else
    {
      if(time_current - barrier_tm_ind > 5000 && time_current - barrier_tm_ind < 14000)
      {
        motor1.set_speed(0);
        barrier_stop_flag = true;
        //if(!barrier_beep_flag)
        {
          if(barrier_beep_time==0)
          {
            barrier_beep_flag = true;
            digitalWrite(BUZZER_PIN, HIGH);
            digitalWrite(rear_light_pin, HIGH);
            digitalWrite(head_light_pin, HIGH);
            barrier_beep_time = time_current;
          }
          else if(time_current - barrier_beep_time > 1000)
          {
            barrier_beep_flag = false;
            barrier_beep_time = 0;
            digitalWrite(BUZZER_PIN, LOW);
            digitalWrite(rear_light_pin, LOW);
            digitalWrite(head_light_pin, LOW);
          }
        }
      }
      else if(time_current - barrier_tm_ind > 14000)
      {
        barrier_tm_ind = 0;
        barrier_stop_flag = false;
        digitalWrite(BUZZER_PIN, LOW);
        digitalWrite(rear_light_pin, LOW);
        digitalWrite(head_light_pin, LOW);
      }
    }
  }
  else
  {
    if(barrier_flag)
    {
      barrier_stop_flag = false;
      barrier_flag = false;
      digitalWrite(BUZZER_PIN, LOW);
      digitalWrite(rear_light_pin, HIGH);
      digitalWrite(head_light_pin, HIGH);
    }
  }
  #endif

  #if defined(BATTARY_TEST)
  if(Corner!=old_Corner && BATTARY1_U>5.1) myservo.write(Corner);  //set up corner
  #else
  if(Corner!=old_Corner) myservo.write(Corner);  //set up corner
  #endif
  
  if((time_current-time)>1000)
  {
     Speed = 0;   
  }
  if(Speed == 0)
  {
     motor1.set_speed(0);
  }
  
  turnsignal_illumination(); //управляет мерцанием поворотников

  //old_Speed = Speed; 
  old_Corner = Corner;
  serial_get_data(); //получить данные по последовательному порту
  if(time_current-last_speed_update>300)
  {
    update_speed();
    #ifdef BARRIER_TEST
    if(!barrier_stop_flag)
    {
      Speed=0;
    }
    #endif
    if(Speed!=0)
    {
      regulatorOld = regulator;
      error = Speed - real_speed;
      integral += error*0.3;
      if(integral>200) integral = 200;
      regulator = (int)(error*2.5 + integral);
      regulator = regulator>255 ? 255 : (regulator<0 ? 0 : regulator); 
      regulator = (regulator + regulatorOld)*0.5;
      motor1.set_speed(regulator);
    }
  }
}










void update_speed()
{
  //отправить последовательность байт по COM порту
  //отправить реальную скорость
  //unsigned int - 2 байта
  //l=226.188 mm
  //imp = 12 counts per revolution with magnetic encoder
  //
  #if defined(POLOLU_OPTICAL_ENCODER)
  real_speed = encoder0Pos*1.3/0.3; //0.3 coz of 300ms update
  #else if defined(POLOLU_MAGNETIC_ENCODER)
  real_speed = encoder0Pos*(22.62/20.0)/0.3; //0.3 coz of 300ms update
  #endif
  encoder0Pos=0;
  last_speed_update = millis();
   Serial.print('F');
   Serial.print(real_speed);
   Serial.print("E\n");
    //Serial.print("bt1: ");
    //Serial.println(BATTARY1_U);
    //Serial.print("bt1: ");
    //Serial.println(BATTARY2_U);
}


void serial_get_data()
{
   if (Serial.available() > 0)
   {
       char c = Serial.read();
       
       switch(cur_state)
       {
        case 1:
            if(c == 'P') cur_state = 2;
            else cur_state = 0;
            break;

        case 2:
            if(c == 'D')
            {
               Corner = Serial.parseInt();
               DIR = Serial.parseInt();
               Speed = Serial.parseInt();
               cur_state = 0;
               time = time_current;
            }
            else cur_state = 0;
            break;
        default:
            if(c == 'S') cur_state = 1;
            else cur_state = 0;
            break;
       }
   }
}



void turnsignal_illumination()
{
  if(Speed == 0)
  {
   digitalWrite(stop_indicator_pin, HIGH); 
  }
  else
  {
    digitalWrite(stop_indicator_pin, LOW);
  }
  
  if(Corner<90-deviation)
  {
    if(!turn_right_light)
    {
      if(right_time_indicator==0)
      {
        turn_right_light = true;
        digitalWrite(right_indicator_pin, HIGH);
        right_time_indicator = time_current;
      }
      else if(time_current - right_time_indicator > 2*turn_signal_freq)
      {
        turn_right_light = false;
        right_time_indicator = 0;
        digitalWrite(right_indicator_pin, LOW);
      }
    }
    else
    {
      if(time_current - right_time_indicator > turn_signal_freq)
      {
        turn_right_light = false;
        digitalWrite(right_indicator_pin, LOW);       
      }
    }
  }
  else
  {
    turn_right_light = false;
    right_time_indicator = 0;
    digitalWrite(right_indicator_pin, LOW);
  }
  
  if(Corner>90+deviation)
  {
    if(!turn_left_light)
    {
      if(left_time_indicator==0)
      {
        turn_left_light = true;
        digitalWrite(left_indicator_pin, HIGH);
        left_time_indicator = time_current;
      }
      else if(time_current - left_time_indicator > 2*turn_signal_freq)
      {
        turn_left_light = false;
        left_time_indicator = 0;
        digitalWrite(left_indicator_pin, LOW);
      }
    }
    else
    {
      if(time_current - left_time_indicator > turn_signal_freq)
      {
        turn_left_light = false;
        digitalWrite(left_indicator_pin, LOW);       
      }
    }
  }
  else
  {
    turn_left_light = false;
    left_time_indicator = 0;
    digitalWrite(left_indicator_pin, LOW);
  }
}


void doEncoder()
{
  encoder0Pos++;
}

#if defined(BATTARY_TEST)
void battary_test_fnc()
{
  bt1_tmp = (analogRead(A5)*BATTARY1_RK)/1024.0;
  bt2_tmp = (analogRead(A6)*BATTARY2_RK)/1024.0;
  BATTARY1_U = BATTARY1_U - voltageFilterL * (BATTARY1_U - bt1_tmp);
  BATTARY2_U = BATTARY2_U - voltageFilterL * (BATTARY2_U - bt2_tmp);
  while((BATTARY1_U>5.15 && BATTARY1_U < 6.9) || (BATTARY2_U>5.15 && BATTARY2_U < 6.9))
  {
    digitalWrite(13, HIGH);
    motor1.set_speed(0);
    digitalWrite(rear_light_pin, LOW);
    digitalWrite(head_light_pin, LOW);
    digitalWrite(left_indicator_pin,LOW);
    digitalWrite(right_indicator_pin,LOW);
    digitalWrite(stop_indicator_pin,LOW);
    delay(1000);
    bt1_tmp = (analogRead(A5)*BATTARY1_RK)/1024.0;
    bt2_tmp = (analogRead(A6)*BATTARY2_RK)/1024.0;
    BATTARY1_U = BATTARY1_U - voltageFilterL * (BATTARY1_U - bt1_tmp);
    BATTARY2_U = BATTARY2_U - voltageFilterL * (BATTARY2_U - bt2_tmp);
  }
  digitalWrite(13, LOW);
  digitalWrite(rear_light_pin, HIGH);
  digitalWrite(head_light_pin, HIGH);
  return;
}
#endif
