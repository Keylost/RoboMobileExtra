#include <Servo.h>

bool _ABVAR_1_stopLineIsHere= false ;
bool _ABVAR_2_stopSignIsHere= false ;
int _ABVAR_3_speed = 0 ;
bool _ABVAR_4_crossroadIsHere= false ;
unsigned long _ABVAR_5_turn = 0UL ;
bool _ABVAR_6_right_light= false ;
bool _ABVAR_7_left_light= false ;
unsigned long _ABVAR_8_stopLineMoment = 0UL ;
int _ABVAR_9_angle = 0 ;
int _ABVAR_10_oldAngle = 0 ;
unsigned long _ABVAR_11_time = 0UL ;
unsigned long _ABVAR_12_stopMoment = 0UL ;
unsigned long _ABVAR_13_crossMoment = 0UL ;
unsigned long _ABVAR_14_ledsMoment = 0UL ;
double _ABVAR_15_servo = 0.0 ;
int i, n, cur;
int line = -110;
boolean stop_line = false;
boolean obstacle = false;
boolean stop_sign = false;
boolean zebra = false;
boolean give_way = false;
boolean main_road = false;
boolean pacBegin = true;
int light2 = 0;
int light3 = 0;
signed char buffer[20]; 
unsigned long time;
Servo servo_pin_9;

void detection();
void traffic3();
void stop_detection();
void traffic2();
void leds();
void crossroad_detection();

void setup()
{
  pinMode( 7 , OUTPUT);
  pinMode( A0 , OUTPUT);
  pinMode( A4 , OUTPUT);
  Serial1.begin(115200);

  servo_pin_9.attach(9);
  pinMode( 6 , OUTPUT);
  pinMode( A1 , OUTPUT);
  pinMode( A3 , OUTPUT);
  pinMode( A2 , OUTPUT);
  digitalWrite(7 , HIGH);

  _ABVAR_1_stopLineIsHere = false ;

  _ABVAR_2_stopSignIsHere = false ;

  _ABVAR_3_speed = 0 ;

  _ABVAR_4_crossroadIsHere = false ;

  digitalWrite(A0 , HIGH);

  digitalWrite(A4 , HIGH);

  _ABVAR_5_turn = 0UL ;

  _ABVAR_6_right_light = true ;

  _ABVAR_7_left_light = true ;

  _ABVAR_8_stopLineMoment = 0UL ;

  _ABVAR_9_angle = 90 ;

  _ABVAR_10_oldAngle = 90 ;

  _ABVAR_11_time = 0UL ;

  _ABVAR_12_stopMoment = 0UL ;

  _ABVAR_13_crossMoment = 0UL ;

  _ABVAR_14_ledsMoment = 0UL ;

  _ABVAR_15_servo = 90.0 ;

}

void loop()
{
  if (Serial1.available() > 4 && pacBegin) {
    time = millis();
    cur = (signed char)Serial1.read();
    if (cur == -126) {
      buffer[0] = (signed char)Serial1.read();
      buffer[1] = (signed char)Serial1.read();
      buffer[2] = (signed char)Serial1.read();
      buffer[3] = (signed char)Serial1.read();
      n = buffer[3];
      line = buffer[0];
      pacBegin = false;

      if (buffer[1] == 1) stop_line = true;
      else stop_line = false; 
      if (buffer[2] == 1) obstacle = true;
      else obstacle = false;

    }
  }

  if (Serial1.available() > n && !pacBegin) {
    light2 = light3 = 0;
    stop_sign = zebra = main_road = give_way = false;

    for (int i = 0; i < n; i++) {
      buffer[4 + i] = (signed char)Serial1.read();
      switch(buffer[4 + i]) {
      case 1:
        stop_sign = true;
        break;
      case 2:
        zebra = true;
        break;
      case 3:
        main_road = true;
        break;
      case 4:
        give_way = true;
        break;
      case 5:
        light2 = 7; //Its green
        break;
      case 6:
        light2 = 9; //Its red
        break;
      case 7:
        light3 = 7; //Its green
        break;
      case 8:
        light3 = 8; //Its yellow
        break;
      case 9:
        light3 = 9; //Its red
        break;
      }
    }
    pacBegin = true;
  }
  if (millis() - time > 500 && millis() >= time) line = -110;
  if (( ( line ) != ( -110 ) ))
  {
    _ABVAR_11_time = millis() ;
    _ABVAR_9_angle = line ;
    if (( ( abs( ( _ABVAR_9_angle - _ABVAR_10_oldAngle ) ) ) > ( 110 ) ))
    {
      _ABVAR_9_angle = ( ( _ABVAR_10_oldAngle * 0.8 ) + ( _ABVAR_9_angle * 0.2 ) ) ;
    }
    _ABVAR_15_servo = ( ( 90 - ( _ABVAR_9_angle * 0.5 ) ) - ( ( _ABVAR_9_angle - _ABVAR_10_oldAngle ) * 0.13 ) ) ;
    if (( ( _ABVAR_15_servo ) > ( 120.0 ) ))
    {
      _ABVAR_15_servo = 120.0 ;
    }
    if (( ( _ABVAR_15_servo ) < ( 60.0 ) ))
    {
      _ABVAR_15_servo = 60.0 ;
    }
    servo_pin_9.write( _ABVAR_15_servo );
    _ABVAR_10_oldAngle = _ABVAR_9_angle ;
    if (( ( ( _ABVAR_1_stopLineIsHere || _ABVAR_2_stopSignIsHere ) || ( _ABVAR_4_crossroadIsHere || stop_line ) ) || ( stop_sign || zebra ) ))
    {
      detection();
    }
    if (( !( _ABVAR_1_stopLineIsHere ) && ( !( _ABVAR_2_stopSignIsHere ) && !( _ABVAR_4_crossroadIsHere ) ) ))
    {
      _ABVAR_3_speed = ( 175 - ( abs( _ABVAR_9_angle ) * 0.4 ) ) ;
    }
    leds();
    traffic2();
    analogWrite(6 , _ABVAR_3_speed);
  }
  else
  {
    analogWrite(6 , 0);
    digitalWrite(A1 , HIGH);
  }
}

void crossroad_detection()
{
  if (( ( ( ( _ABVAR_11_time - _ABVAR_13_crossMoment ) ) >= ( 15 ) ) && ( ( _ABVAR_3_speed ) >= ( 105 ) ) ))
  {
    _ABVAR_3_speed = ( _ABVAR_3_speed - 1 ) ;
    _ABVAR_13_crossMoment = _ABVAR_11_time ;
  }
  if (( !( zebra ) && ( ( _ABVAR_3_speed ) != ( 100 ) ) ))
  {
    _ABVAR_3_speed = 100 ;
    _ABVAR_13_crossMoment = _ABVAR_11_time ;
  }
  if (( ( ( _ABVAR_11_time - _ABVAR_13_crossMoment ) ) >= ( 3000 ) ))
  {
    _ABVAR_3_speed = ( 175 - ( abs( _ABVAR_9_angle ) * 0.4 ) ) ;
    _ABVAR_4_crossroadIsHere = false ;
  }
}

void leds()
{
  if (( ( _ABVAR_9_angle ) > ( 30 ) ))
  {
    if (( ( ( _ABVAR_11_time - _ABVAR_14_ledsMoment ) ) >= ( 500 ) ))
    {
      _ABVAR_14_ledsMoment = _ABVAR_11_time ;
      if (_ABVAR_6_right_light)
      {
        _ABVAR_6_right_light = false ;
        digitalWrite(A3 , HIGH);
      }
      else
      {
        _ABVAR_6_right_light = true ;
        digitalWrite(A3 , LOW);
      }
    }
  }
  else
  {
    digitalWrite(A3 , LOW);
    _ABVAR_6_right_light = true ;
  }
  if (( ( _ABVAR_9_angle ) < ( -30 ) ))
  {
    if (( ( ( _ABVAR_11_time - _ABVAR_14_ledsMoment ) ) >= ( 500 ) ))
    {
      _ABVAR_14_ledsMoment = _ABVAR_11_time ;
      if (_ABVAR_7_left_light)
      {
        _ABVAR_7_left_light = false ;
        digitalWrite(A2 , HIGH);
      }
      else
      {
        _ABVAR_7_left_light = true ;
        digitalWrite(A2 , LOW);
      }
    }
  }
  else
  {
    digitalWrite(A2 , LOW);
    _ABVAR_7_left_light = true ;
  }
  if (( ( _ABVAR_3_speed ) == ( 0 ) ))
  {
    digitalWrite(A1 , HIGH);
  }
  else
  {
    digitalWrite(A1 , LOW);
  }
}

void traffic3()
{
  if (( ( light3 ) == ( 7 ) ))
  {
    _ABVAR_1_stopLineIsHere = false ;
  }
  if (( ( _ABVAR_1_stopLineIsHere && !( stop_line ) ) && ( ( ( light3 ) == ( 8 ) ) || ( ( light3 ) == ( 9 ) ) ) ))
  {
    _ABVAR_3_speed = 0 ;
    _ABVAR_4_crossroadIsHere = false ;
  }
}

void traffic2()
{
  if (( ( light2 ) == ( 9 ) ))
  {
    _ABVAR_3_speed = 0 ;
  }
}

void stop_detection()
{
  if (( ( ( ( _ABVAR_11_time - _ABVAR_12_stopMoment ) ) >= ( 15 ) ) && ( ( _ABVAR_3_speed ) >= ( 105 ) ) ))
  {
    _ABVAR_12_stopMoment = _ABVAR_11_time ;
    _ABVAR_3_speed = ( _ABVAR_3_speed - 1 ) ;
  }
  if (( ( ( _ABVAR_3_speed ) != ( 0 ) ) && !( stop_sign ) ))
  {
    _ABVAR_3_speed = 0 ;
    _ABVAR_12_stopMoment = _ABVAR_11_time ;
  }
  if (( ( ( _ABVAR_11_time - _ABVAR_12_stopMoment ) ) >= ( 4000 ) ))
  {
    _ABVAR_2_stopSignIsHere = false ;
    _ABVAR_3_speed = ( 175 - ( abs( _ABVAR_9_angle ) * 0.4 ) ) ;
  }
}

void detection()
{
  if (( zebra && !( _ABVAR_4_crossroadIsHere ) ))
  {
    _ABVAR_4_crossroadIsHere = true ;
    _ABVAR_13_crossMoment = _ABVAR_11_time ;
  }
  if (_ABVAR_4_crossroadIsHere)
  {
    crossroad_detection();
  }
  if (( stop_sign && !( _ABVAR_2_stopSignIsHere ) ))
  {
    _ABVAR_2_stopSignIsHere = true ;
    _ABVAR_12_stopMoment = _ABVAR_11_time ;
  }
  if (_ABVAR_2_stopSignIsHere)
  {
    stop_detection();
  }
  if (( stop_line && !( _ABVAR_1_stopLineIsHere ) ))
  {
    _ABVAR_1_stopLineIsHere = true ;
    _ABVAR_8_stopLineMoment = _ABVAR_11_time ;
  }
  if (_ABVAR_1_stopLineIsHere)
  {
    if (( ( ( _ABVAR_3_speed ) != ( 0 ) ) && ( ( ( _ABVAR_11_time - _ABVAR_8_stopLineMoment ) ) >= ( 500 ) ) ))
    {
      _ABVAR_1_stopLineIsHere = false ;
    }
    else
    {
      traffic3();
    }
  }
}


