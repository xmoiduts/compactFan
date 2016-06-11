//to do :
//看门狗节能

/*引脚说明：*/
/*
9 - 按钮的vcc
11 - pwm
13 - 风扇变速led
a1 - 风扇开关
a3 - 按钮感知
*/
/*全局变量*/
#include <avr/wdt.h>
#include <avr/sleep.h>

bool direction = 1 ; //风扇速度变更的方向，0 : will go down; 1 : will go up .

/**按钮类说明：按钮经消抖后分为0，1，2，3，4 共5个状态，分别对应：
未按下、短按、（从单击）弹起、长按下、（从长按）弹起。
消抖方式是：连续四次读取输入引脚，均判定为高电平，此时修改状态为“1”
**/
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
        digitalWrite ( 9, HIGH ) ; //开启按钮的输入电压
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
    short up ( ) {

        if ( isUp == 0 ) { return 0 ; }
        realduty += k * interval ;
        if ( realduty >= 255 ) {         //不设为256，风扇最大值会限制在奇怪的254.
            realduty = 255 ;
            OCR2A = realduty ;
            return 0 ;
        }else{
            dutyCycle = (unsigned short)realduty ;
            OCR2A = dutyCycle ;
            return 1 ;
        }

    }
    short down ( ) {
        if ( isUp == 0 ) { return 0 ; }
        realduty -= k * interval ;
        if ( realduty < minDuty ) {
            realduty = minDuty ;
            return 0 ;
        }
        dutyCycle = (unsigned short)realduty ;
        OCR2A = dutyCycle ;
        return 1 ;
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

class Flasher {
public:
    int pin ; //闪光灯链连接的引脚
    short stat ; //现在是否点亮
    Flasher ( int ipin ) {
        pin = ipin ;
        pinMode ( pin , OUTPUT ) ;
        digitalWrite ( pin , LOW ) ;
        stat = 0 ;
    }
    void write ( int result ) {
        if ( result ) {
            digitalWrite ( pin , HIGH ) ;
            stat = 1 ;
        }
        else {
            digitalWrite ( pin , LOW ) ;
            stat = 0 ;
        }
    }
};

class EnergySaver {

public:
    unsigned long passes ; //休眠时间，以wdt唤醒次数计。
    unsigned short stat , toSleep ; //
    EnergySaver( ) {
        setTime(3) ;
        passes = 0 ;
        stat = 0 ;
        toSleep = 0 ;
    }
    void setTime ( int mode ) {

        byte bb;

        if (mode > 9 )
            mode=9;
        bb=mode & 7;
        if (mode > 7)
            bb|= (1<<5);
        bb|= (1<<WDCE);

        MCUSR &= ~(1<<WDRF);
        // start timed sequence
        WDTCSR |= (1<<WDCE) | (1<<WDE);
        // set new watchdog timeout value
        WDTCSR = bb;
        WDTCSR |= _BV(WDIE);
    }//mode: 0=16ms, 1=32ms,2=64ms,3=128ms,4=250ms,5=500ms, 6=1 sec,7=2 sec, 8=4 sec, 9= 8sec ;every [mode] will call ISR(WDT_vect) once .

    void update( int FanIsRunning ) {
        if ( FanIsRunning ) {
            passes = 0 ;
        }
        else {
            passes ++ ;
        }

        if ( passes < 960 ) {
            stat = 0 ;
        }//风扇运行中：不休眠
        else if ( passes < 4800 ) {
            stat = 1 ;
        }//风扇已停运2分钟：进行8/s的唤醒
        else if ( passes < 16800 ) {
            stat = 2 ;
        }//风扇已停运10分钟：进行4/s的唤醒
        else {
            stat = 3 ;
        }//风扇已停运60分钟：进行1/s的唤醒
    }
    void reset () { passes = 0 ; }

    int getStat () { return stat ; }

    void powerdown_avr(){
      set_sleep_mode(SLEEP_MODE_PWR_DOWN  ); // sleep mode is set here
      sleep_enable();
      sleep_mode();                        // System sleeps here
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
Button button1(A3) ;
Int1ms timer0 ;
Fan fan1(11 , A1 , 112 , 15000 ) ;
Flasher L13 (13) ;
EnergySaver sleeper1 ;

/*主程序*/

void everyms () {

    button1. update() ;                     //更新按钮状态

    if ( button1.getStat() == 3 ) {
        if ( direction == 0 )
            L13.write ( fan1.down() ) ;           //减速
        else
            L13.write ( fan1.up  () ) ;             //加速
    }                                       //长按

    else if (button1.getStat() == 2 ) {
        if ( fan1.getStat() == 1 ){
            fan1.shutdown();        //关机
        }
        else{
            fan1.startup ();        //开机
        }
    }                                       //单击

    else {
        L13.write ( 0 ) ;
    }                                       //没按键
    fan1.update() ;

}

SIGNAL ( TIMER0_COMPA_vect ) {
    timer0 . begin ( ) ;
    everyms ( ) ;
    timer0 . end   ( ) ;
}//this signal is called every 1.024 ms , all control logics are written here .

ISR(WDT_vect){

    //Serial.print(sleeper1.stat);
    //Serial.print(" WDT ");
    //Serial.println(sleeper1.passes);

    digitalWrite ( 13,HIGH ) ;
    if (digitalRead (A3) == HIGH ) {
        sleeper1.reset();
    }
    else {
        sleeper1.update(fan1.getStat());
        if ( sleeper1.getStat() == 0 ) {
            if(fan1.getStat() == 1)
                sleeper1.setTime(6);
            else
                sleeper1.setTime(3);
        }//风扇运行中
        else if ( sleeper1.getStat() == 1 ) {
            sleeper1.setTime(3);//  8/s
            sleeper1.toSleep = 1 ;
        }//风扇停转2分钟
        else if ( sleeper1.getStat() == 2 ) {
            sleeper1.setTime(4);//  4/s
            sleeper1.toSleep = 1 ;
        }//风扇停转10分钟
        else if ( sleeper1.getStat() == 3 ) {
            sleeper1.setTime(6);//  1/s
            sleeper1.toSleep = 1 ;
        }//风扇停转60分钟
    }
    digitalWrite (13,LOW);

}

void setup() {

    /*配置1ms中断事件*/
    //Serial.begin ( 115200 ) ;
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);

    /*
    配置11引脚风扇PWM输出，COM2A-10;WGM2-001;CS2-001;
    ；相位修正pwm；时钟直入31khz；
    欲使用3引脚，需配置COM2B，OCR2B.
    */
    TCCR2A = 0x91 ;
    TCCR2B = 0x01 ;
    pinMode (9,OUTPUT) ;
    pinMode (10,OUTPUT) ;
    pinMode (11,OUTPUT) ;
    OCR2A = 0 ;

    /*节能*/
    ACSR |=_BV(ACD);//OFF ACD
    ADCSRA=0;//OFF ADC
}

void loop() {
    if ( sleeper1.toSleep == 1 ) {
        sleeper1.powerdown_avr() ;
    }
    sleeper1.toSleep = 0 ;
}

