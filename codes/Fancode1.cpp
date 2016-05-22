

///��ť״̬��δ���¡����������
class Button{
public:
    unsigned short role , repeated ; //�����Ƿ��ȡ��ť����ť������������������ť�Ƿ񱻰��¹���ȥ����������
    unsigned short isClicked , isPressed ; //��ť�Ƿ񱻵����ȥ����������ť�Ƿ񱻳�����
    int pin ;   //��ť����Ķ�ȡ����
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
            } //��ť�����
        }
        else {
            isClicked = 0 ;
            repeated = 0 ;
        }

    }//��ⰴť״̬
};

class Fan {
private :
    unsigned short dutyCycle , minDuty ; //��ǰռ�ձ� �����ռ�ձ�
    unsigned short prevMil , curMil , PRT ; //ʱ��
    unsigned short pin ; //���ȵĿ��ٽ�
public :
    Fan(unsigned short ipin , unsigned short miniDuty , int PowerResponseTime ){//���ȿ��ٽš����ռ�ձȡ��ٶȴ���͵���ߵ���Ӧʱ��(ms)
        pin = ipin ;
        dutyCycle = 0 ;
        minDuty = miniDuty ;
        PRT = PowerResponseTime ;
    }
    void update ( unsigned short newDuty ) {
        prevMil = curMil ;
        curMil = millis();//ˢ�����ε���֮��ļ��
        dutyCycle =
    }//б�ʣ�
};

class Int1ms {
private:

    unsigned long startTime , endTime ;
    int role  ; //500ms �ڵ�˳������ȡģ��

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


/*ȫ�ֱ�����*/
Button button1(4) ;
Int1ms timer0 ;


/*��ʼ�����룿*/

SIGNAL ( TIMER0_COMPA_vect ) {
    timer0 . begin () ;
    button1. update() ;
    timer0 . update() ;
    timer0 . end   () ;
}

void setup() {

    /*����1ms�ж��¼�*/
    Serial.begin ( 115200 ) ;
    OCR0A = 0xAF;
    TIMSK0 |= _BV(OCIE0A);
}

void loop() {
    delay(125) ;
    //Serial.println ( timer0.getDuration () ) ;
    Serial.println ( button1.stat ) ;
}
