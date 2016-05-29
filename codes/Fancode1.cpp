//to do :
//加减速指示灯
//看门狗节能
/*全局变量*/
bool direction = 1 ; //风扇速度变更的方向，0 : will go down; 1 : will go up .

///按钮状态：未按下、点击、长按
class Button{
public:
    unsigned short role , repeated ; //本轮是否读取按钮，按钮连续按下周期数。按钮是否被按下过（去抖动），。
    unsigned short stat ; //按钮是否被点击（去抖动），0 : not clicked ; 1 : clicked ; 2 : released from click ;3: released from press  ;
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
    }   //update () is called every 1.024 ms , in every loop it checks whether it's time to update button's stat .
    void scanButton () {
        if ( digitalRead ( pin ) == HIGH ) {
            repeated ++ ;
            if ( (repeated > 3 ) && ( repeated < 20 ) ) {
                stat = 1 ;
            } //clicked
            else if ( repeated > 19 ) {
                stat = 3 ;
            }
        }//clicked or pressed in this loop
        else
        if ( stat == 1 ){
            stat = 2 ;
            repeated = 0 ;

        }//released from click ;
        else
        if ( stat == 2 ) {
            stat = 0 ;
            repeated = 0 ;
        }//clear stat 2
        else
        if ( stat == 3 ) {
            stat = 4 ;
            repeated = 0 ;
        }//released from press ;
        else
        if ( stat == 4 ) {
            stat = 0 ;
            direction = !direction ; /// 作用于风扇的方向，取反。
        }//clear stat 4 ;

    }//检测按钮状态
    int getStat () {
        return stat ;
    }//get the button's stat.

};

class Fan {
private :
    unsigned short dutyCycle , minDuty , isUp;      //为寄存器赋值的占空比，最低占空比，是否正在运行
    float realduty , k ;                            //用来计算的占空比,变速斜率
    unsigned long  prevMil , curMil , interval ;    //统计调用间隔
    //unsigned short atBottom , atTop ;             //风扇功率是否达到最低/最高
    int pin , cont , PRT ;                          //风扇的控速脚,风扇的开关脚，功率响应时间
public :
    Fan ( int pwmPin , int controlPin , unsigned short miniDuty , int PowerResponseTime )
    {//风扇控速脚、最低占空比、速度从最低到最高的响应时间(ms)
        pin = pwmPin ;
        cont = controlPin ;         //引脚
        digitalWrite ( cont , LOW ) ;

        dutyCycle = 0 ;
        realduty  = 0 ;
        minDuty = miniDuty ;        //占空比

        PRT = PowerResponseTime ;
        k = 255.0/PRT ;
        interval = 0 ;              //计算变速的速度

        isUp = 0 ;                  //风扇开关
    }
    void update ( ) {

        prevMil = curMil ;
        curMil = millis();
        interval = curMil - prevMil ;

    }//刷新两次调用之间的间隔
    void up ( ) {

        if ( isUp == 0 ) { return ; }
        realduty += k * interval ;
        if ( realduty >= 255 ) {
            realduty = 255 ;
        }
        dutyCycle = (unsigned short)realduty ;
        OCR2A = dutyCycle ;
    }
    void down ( ) {
        if ( isUp == 0 ) { return ; }
        realduty -= k * interval ;
        if ( realduty < minDuty ) {
            realduty = minDuty ;
        }
        dutyCycle = (unsigned short)realduty ;
        OCR2A = dutyCycle ;
    }
    void shutdown ( ) {

        isUp = 0 ;
        digitalWrite ( cont , LOW ) ;
        dutyCycle = 0 ;
        realduty = 0 ;
        OCR2A = dutyCycle ;

    }
    void startup  ( ) {

        isUp = 1 ;
        direction = 1;
        digitalWrite ( cont , HIGH ) ;
        dutyCycle = minDuty ;
        realduty = minDuty ;
        OCR2A = dutyCycle ;

    }

    unsigned short getStat ( ) {
        return isUp ;
    }
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
    unsigned long getDuration () {
        return endTime - startTime ;
    }
};

///---------------------------------------------------------///
/*类的声明*/
Button button1(14) ;
Int1ms timer0 ;
Fan fan1(11 , 10 , 112 , 15000 ) ;

/*主程序*/

void everyms () {

    button1. update() ;                     //更新按钮状态

    if ( button1.getStat() == 3 ) {
        if ( direction == 0 )
            fan1.down() ;           //减速
        else
            fan1.up() ;             //加速
    }                                       //长按

    else if (button1.getStat() == 2 ) {
        if ( fan1.getStat() == 1 )
            fan1.shutdown();        //关机
        else
            fan1.startup ();        //开机
    }                                       //单击

    else {
    }                                       //没按键
    fan1.update() ;

}

SIGNAL ( TIMER0_COMPA_vect ) {
    timer0 . begin ( ) ;
    everyms ( ) ;
    timer0 . end   ( ) ;
}//this signal is called every 1.024 ms , all control logics are written here .

void setup() {

    /*配置1ms中断事件*/
    Serial.begin ( 115200 ) ;
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);

    /*
    配置11引脚风扇PWM输出，COM2A-10;WGM2-001;CS2-001;
    ；相位修正pwm；时钟直入31khz；
    欲使用3引脚，需配置COM2B，OCR2B.
    */
    TCCR2A = 0x91 ;
    TCCR2B = 0x01 ;
    pinMode (10,OUTPUT) ;
    pinMode (11,OUTPUT) ;
    OCR2A = 0 ;

}

void loop() {
    delay(64) ;
    //Serial.println ( timer0.getDuration () ) ;
    Serial.println ( OCR2A ) ;
}
