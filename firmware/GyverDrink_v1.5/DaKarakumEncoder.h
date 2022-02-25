const int NONE = 0;
enum ENC_STATE {
    ROTATION = 1,
    ROTATION_PRESS = 2,
    PRESS = 3,
    LONG_PRESS = 4
};

enum CURRENT_ROTATE {
  LEFT = 1,
  RIGHT = -1
};

const int btn_long_push = 1000; // ms


volatile uint8_t lastcomb=7, enc_state = NONE, btn_push=0;
volatile int enc_rotation=0, btn_enc_rotate=0;
volatile boolean btn_press=0;
volatile uint32_t timer;
volatile int8_t currentRotate = NONE; 


class DaKarakumEncoder
{
public:
  

  DaKarakumEncoder::DaKarakumEncoder ()
  {
    pinMode(A1,INPUT_PULLUP); // ENC-A
    pinMode(A2,INPUT_PULLUP); // ENC-B
    pinMode(A3,INPUT_PULLUP); // BUTTON

    PCICR =  0b00000010; // PCICR |= (1<<PCIE1); Включить прерывание PCINT1
    PCMSK1 = 0b00001110; // Разрешить прерывание для  A1, A2, A3
  }

  boolean isClick();
  boolean isTurn();
  boolean isRight();
  boolean isLeft();

};

boolean DaKarakumEncoder::isClick() {
  return enc_state == PRESS || enc_state == LONG_PRESS || enc_state == ROTATION_PRESS;
}

boolean DaKarakumEncoder::isTurn() {
  return enc_state == ROTATION || enc_state == ROTATION_PRESS;
}

boolean DaKarakumEncoder::isRight() {
  if (currentRotate == RIGHT)
  {
    currentRotate = NONE;
    return true;
  }
  else return false;
}

boolean DaKarakumEncoder::isLeft() {
  if (currentRotate == RIGHT)
  {
    currentRotate = NONE;
    return true;
  }
  else return false;
}

ISR (PCINT1_vect) //Обработчик прерывания от пинов A1, A2, A3
{
  uint8_t comb = bitRead(PINC, 3) << 2 | bitRead(PINC, 2) << 1 | bitRead(PINC, 1); //считываем состояние пинов энкодера и кнопки
  // comb = 0b[A3][A2][A1]

 if (comb == 0b011 && lastcomb == 0b111) btn_press=1; //Если было нажатие кнопки, то меняем статус
 
 if (comb == 0b100)                         //Если было промежуточное положение энкодера, то проверяем его предыдущее состояние 
 {
    if (lastcomb == 0b101) 
    {
      --enc_rotation; //вращение по часовой стрелке
      currentRotate = RIGHT;
    }
    if (lastcomb == 0b110){
      ++enc_rotation; //вращение против часовой
      currentRotate = LEFT;
    }
    enc_state=1;                       // был поворот энкодера    
    btn_enc_rotate=0;                  //обнулить показания вращения с нажатием
  }
  
   if (comb == 0b000)                      //Если было промежуточное положение энкодера и нажатие, то проверяем его предыдущее состояние 
   {
    if (lastcomb == 0b001) { 
      --btn_enc_rotate; //вращение по часовой стрелке
      currentRotate = RIGHT;
    }
    if (lastcomb == 0b010) { 
      ++btn_enc_rotate; //вращение против частовой
      currentRotate = LEFT;
    }
    enc_state = ROTATION_PRESS;                        // был поворот энкодера с нажатием  
    enc_rotation = 0;                     //обнулить показания вращения без нажатия
    btn_press = 0;                         //обнулить показания кнопки
   }

   if (comb == 0b111 && lastcomb == 0b011 && btn_press) //Если было отпускание кнопки, то проверяем ее предыдущее состояние 
   {
     if (millis() - timer > btn_long_push)         // проверяем сколько прошло миллисекунд
     {
       enc_state = LONG_PRESS;                              // было длинное нажатие 
     } else {
       enc_state = PRESS;                    // было нажатие 
     }
     btn_press = 0;                           //обнулить статус кнопки
     }
   
  timer = millis();                       //сброс таймера
  lastcomb = comb;                        //сохраняем текущее состояние энкодера
}
