#include <FEHLCD.h>
#include <FEHIO.h>
#include <FEHUtility.h>
#include <FEHMotor.h>

//Define Statements
#define forward  1
#define backward  -1
#define CDS_Threshold .1417
#define left_motor FEHMotor::Motor3
#define right_motor FEHMotor::Motor1

//Global Variables
DigitalInputPin FrontRight_bumpswitch( FEHIO::P2_0 );
DigitalInputPin FrontLeft_bumpswitch( FEHIO::P2_0 );
ButtonBoard buttons( FEHIO::Bank3 );
AnalogInputPin cds_cell( FEHIO::P1_0 );
FEHMotor Left_Motor(FEHMotor::Motor3), Right_Motor(FEHMotor::Motor1);


class StartUp
{
    //Public Elements
public:
    StartUp();
    void SwitchCheck();
    void CDSCell();
    void RunAll();
    void Check_Right_Encoder();
    void Check_Left_Encoder();
    void Button_Start();
    void OptoCheck();
    void Light_Start();
    float Optosensor_threshold;
private:

};

//Navigation Class Definition
class Navigation
{
public:
    Navigation();
    void DistanceTravelled(float distance, int power);
    void DriveToWall(float power);
    void Right90Turn();
    void Left90Turn();
    void RightTurn(float angle);
    void LeftTurn(float angle);
    void DriveForward(float power);
    void DriveBackward(float power);
    void StopMotors();
    void GrabSled();
private:
    float forward_calibration, reverse_calibration;
};

//Function Prototypes

int main(void)
{
    LCD.Clear( FEHLCD::Black );
    LCD.SetFontColor( FEHLCD::White );
    StartUp NewRun;
    NewRun.RunAll();
    return 0;
}



//Begin Function Definitons for StartUp Class
StartUp::StartUp()
{

}

void StartUp::RunAll()
{

    int a=0;
    LCD.WriteLine("Do you need to calibrate?\nPress Left for yes, Right for No");
    while (a==0)
    {
        if (buttons.LeftPressed())
        {
            StartUp::CDSCell();
            a=1;
        }
        if (buttons.RightPressed())
        {
            a=1;
        }
    }
    StartUp::Button_Start();
    StartUp::Light_Start();
}

void StartUp::SwitchCheck()
{

    int a=0;
    //This function will test the switches
    LCD.WriteLine("Please press the front right switch");
    while (a==0)
    {
        if (FrontRight_bumpswitch.Value()==false)
        {
            a=1;
            LCD.WriteLine("Switch is functioning normally");
        }
    }
    a=0;
    LCD.WriteLine("Please press the front left switch");
    while (a==0)
    {
        if (FrontLeft_bumpswitch.Value()==false)
        {
            a=1;
            LCD.WriteLine("Switch is functioning normally");
        }
    }


}

void StartUp::Button_Start()
{

    int a=0;
    LCD.WriteLine("Waiting For Start Button");
    while (a==0)
    {
        if (buttons.LeftPressed())
        {
            a=1;
        }
    }
    LCD.WriteLine("Start Button Pressed");

}

void StartUp::Light_Start()
{

    int a = 0,b;
    LCD.WriteLine("Waiting for light");
    b=TimeNow();
    while (a==0)
    {
        if (cds_cell.Value( )<= CDS_Threshold)
        {
            a=1;
        }
        else if (TimeNow()-b >= 10)
        {
            a=1;
        }

    }
    LCD.WriteLine("Light Detected, Beginning Run");
    Left_Motor.SetPower(100);
    Right_Motor.SetPower(100);
    Sleep(1.0);
    Left_Motor.SetPower(0);
    Right_Motor.SetPower(0);
}

void StartUp::CDSCell()
{

}


//Start Function Definitions for Navigation Class
Navigation::Navigation()
{
    forward_calibration = 1;
    reverse_calibration = 1;
}
void StartUp::OptoCheck()
{

}

void Navigation::DistanceTravelled(float distance, int power)
{

}
void Navigation::DriveToWall(float power)
{

}
void Navigation::Right90Turn()
{

}
void Navigation::Left90Turn()
{

}
void Navigation::RightTurn(float angle)
{

}
void Navigation::LeftTurn(float angle)
{

}
void Navigation::StopMotors()
{

    Left_Motor.SetPower(0);
    Right_Motor.SetPower(0);
}
void Navigation::DriveForward(float power)
{

    Left_Motor.SetPower((int)(power*forward_calibration));
    Right_Motor.SetPower((int)(power*forward_calibration));
}

void Navigation::DriveBackward(float power)
{

    Left_Motor.SetPower((int)(power*reverse_calibration));
    Right_Motor.SetPower((int)(power*reverse_calibration));
}
