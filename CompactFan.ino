class Button{
public:
    unsigned short role , stat , repeated ; //是否读取按钮，按钮状态（去抖动），按钮连续按下周期数。
    int pin ;   //按钮分配的读取引脚
    Button( int ipin ) {
        pin = ipin ;
        pinMode ( pin , INPUT ) ;
        role = 0 ;
        stat = 0 ;
        repeated = 0 ;
    }

    void update () {
        role ++ ;
        if (role == 25 ) {
            role -= 25 ;
            scanButton () ;//update button
        }
    }
    void scanButton () {
        if ( digitalRead ( pin ) == HIGH ) {
            repeated ++ ;
            if (repeated >= 4 ) {
                stat = 1 ;
                repeated = 0 ;
            } //按钮被按下
        }
        else {
            stat = 0 ;
            repeated = 0 ;
        }

    }//检测按钮状态
};

class Int1ms {
private:

    unsigned long startTime , endTime ;
    int role  ; //500ms 内的顺序，用于取模。

public:

    Int1ms() {
        startTime = 0 ;
        endTime = 0 ;
        role = 0 ;
    }
    void begin () {
        startTime = micros () ;
    }
    void end () {
        endTime = micros () ;
    }
    void update () {

        role ++ ;
        if ( role == 500 ) {
            role -= 500 ;
        }
                                       //cycle every 500ms
        if ( role ==   0 ) {
            //Serial.println(F("  PM2.5 called")) ;  //PM2_5.update();
        }//PM2.5

        else if ( role == 100 ) {
            //Serial.println(F("  DHT11 called")) ;  //DHT11.update();
        }

        else if ( role == 200 ) {
            //Serial.println(F("  Calculate Fan Duty")) ;  //calculate fan duty ;
        }

        else if ( role == 300 ) {
            //Serial.println(F("  LCD1602 called")) ;   //LCD1602.update();
        }

    }
    unsigned long getDuration () {
        return endTime - startTime ;
    }
};


/*全局变量区*/
Button button1(4) ;
Int1ms timer0 ;

/*初始化代码？*/

SIGNAL ( TIMER0_COMPA_vect ) {
    timer0 . begin () ;
    button1. update() ;
    timer0 . update() ;
    timer0 . end   () ;
}

void setup() {

    /*配置1ms中断事件*/
    Serial.begin ( 115200 ) ;
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);

    /*
  配置风扇PWM输出，COM2B-10;WGM2-001;CS2-001;
  ；相位修正pwm；时钟直入31khz；
  */
  TCCR2A = 0x21 ;
  TCCR2B = 0x01 ;
  pinMode (3,OUTPUT) ;
  OCR2B = fanDutyCycle ;
}

void loop() {
    delay(125) ;
    //Serial.println ( timer0.getDuration () ) ;
    Serial.println ( button1.stat ) ;
}

