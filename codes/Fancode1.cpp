

///按钮状态：未按下、点击、长按
class Button{
public:
    unsigned short role , repeated ; //本轮是否读取按钮，按钮连续按下周期数。按钮是否被按下过（去抖动），。
    unsigned short isClicked , isPressed ; //按钮是否被点击（去抖动），按钮是否被长按。
    int pin ;   //按钮分配的读取引脚
    Button( int ipin ) {
        pin = ipin ;
        pinMode ( pin , INPUT ) ;
        role = 0 ;
        isClicked = 0 ;
        isPressed = 0
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
                isClicked = 1 ;
                repeated = 0 ;
            } //按钮被点击
        }
        else {
            isClicked = 0 ;
            repeated = 0 ;
        }

    }//检测按钮状态
};

class Fan {
private :
    unsigned short dutyCycle , minDuty ; //当前占空比 ，最低占空比
    unsigned short prevMil , curMil , PRT ; //时间
    unsigned short pin ; //风扇的控速脚
public :
    Fan(unsigned short ipin , unsigned short miniDuty , int PowerResponseTime ){//风扇控速脚、最低占空比、速度从最低到最高的响应时间(ms)
        pin = ipin ;
        dutyCycle = 0 ;
        minDuty = miniDuty ;
        PRT = PowerResponseTime ;
    }
    void update ( unsigned short newDuty ) {
        prevMil = curMil ;
        curMil = millis();//刷新两次调用之间的间隔
        dutyCycle =
    }//斜率：
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
}

void loop() {
    delay(125) ;
    //Serial.println ( timer0.getDuration () ) ;
    Serial.println ( button1.stat ) ;
}
