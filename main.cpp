//Libraries to be included
#include <FEHLCD.h>
#include <FEHIO.h>
#include <FEHUtility.h>
#include <FEHMotor.h>
#include <FEHServo.h>
#include <FEHBuzzer.h>

//Define Statements
#define forward  1
#define backward  -1
#define left_motor FEHMotor::Motor3
#define right_motor FEHMotor::Motor1

//Global Variables for IO pins
DigitalInputPin FrontRight_bumpswitch( FEHIO::P2_7 );
DigitalInputPin FrontLeft_bumpswitch( FEHIO::P0_0 );
DigitalInputPin LineFollowingOptosensor(FEHIO:: P1_0);
ButtonBoard buttons( FEHIO::Bank3 );
AnalogInputPin cds_cell( FEHIO::P1_0 );
FEHMotor Left_Motor(FEHMotor::Motor3), Right_Motor(FEHMotor::Motor1);
FEHEncoder Left_Encoder(FEHIO::P0_1);
FEHEncoder Right_Encoder(FEHIO::P2_1);

//Threshold variables
float CDS_Threshold=.14;
float Line_Following_Threshold;

//This class runs first and can be used to test and calibrate sensors
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
//This class is used to drive the robot around the course
class Navigation
{
public:
    Navigation();
    void DistanceTravelled(float distance, float power);
    void DriveToWall(float power);
    void DriveToLine(float power);
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


int main(void)
{
    LCD.Clear( FEHLCD::Black );
    LCD.SetFontColor( FEHLCD::White );
    StartUp NewRun;
    NewRun.RunAll();
    return 0;
}



//Begin Function Definitons for StartUp Class
//Constructor Function
StartUp::StartUp()
{

}

//This function checks the left encoder
void StartUp::Check_Left_Encoder()
{
    int a;
    Left_Encoder.ResetCounts();
    Left_Motor.SetPower(50);
    a=TimeNow();
    while (TimeNow()-a<=5)
    {
        LCD.WriteLine(Left_Encoder.Counts());
    }
    Left_Motor.Stop();
    Left_Encoder.ResetCounts();
}

//This function checks the right encoder
void StartUp::Check_Right_Encoder()
{
    int a;
    Right_Encoder.ResetCounts();
    Right_Motor.SetPower(50);
    a=TimeNow();
    while (TimeNow()-a<=5)
    {
        LCD.WriteLine(Right_Encoder.Counts());
    }
    Right_Motor.Stop();
    Right_Encoder.ResetCounts();
}

//This function asks the user if calibration is needed, and then runs the course as necessary
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

//This function checks to make sure the microswitches are working
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

//This function lets the robot wait for the start button to be pressed
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

//This function makes the robot wait until the light is activated or ten seconds have passed
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

//This function sets the threshold for the central CDS cell
void StartUp::CDSCell()
{
    int a=0;
    float lower_value, upper_value;
    LCD.WriteLine("Place robot on course and Turn Light Off");
    while (a==0)
    {
        LCD.WriteLine("Press left button to set upper bound");
        if (buttons.LeftPressed())
        {
            upper_value = cds_cell.Value();
            a=1;
        }
    }
    LCD.WriteLine("Please turn the light on");
    a=0;
    while (a==0)
    {
        LCD.WriteLine("Press left button to set lower bound");
        if (buttons.LeftPressed())
        {
            lower_value = cds_cell.Value();
            a=1;
        }
    }
    CDS_Threshold = (upper_value+lower_value)/2;
    LCD.WriteLine("Value of threshold is");
    LCD.WriteLine(CDS_Threshold);
}

//This function is used to calibrate the line following optosensor
void StartUp::OptoCheck()
{
    int a=0;
    float lower_value, upper_value;
    LCD.WriteLine("Place optosensor over normal section of course");
    LCD.WriteLine("Press left button to set upper reflectivity value");
    while (a==0)
    {
        if (buttons.LeftPressed())
        {
            upper_value = LineFollowingOptosensor.Value();
            a=1;
        }

    }
    LCD.WriteLine("Place optosensor over line");
    LCD.WriteLine("Press left button to set lower reflectivity value");
    a=0;
    while (a==0)
    {
        if (buttons.LeftPressed())
        {
            lower_value = LineFollowingOptosensor.Value();
            a=1;
        }

    }
    Line_Following_Threshold = (upper_value+lower_value)/2;
    LCD.WriteLine("Value of threshold is");
    LCD.WriteLine(Line_Following_Threshold);


}

//Start Function Definitions for Navigation Class
//Constructor Function
Navigation::Navigation()
{
    forward_calibration = 1;
    reverse_calibration = 1;
}

//This function travels the specified distance at the provided speed
void Navigation::DistanceTravelled(float distance, float power)
{

}

//This function drives until a line is encountered
void Navigation::DriveToLine(float power)
{
    while (LineFollowingOptosensor.Value() >= Line_Following_Threshold)
    {
        Navigation::DriveForward(power);
    }
    Navigation::StopMotors();
}

//This function drives the robot forward until it is square against a wall
void Navigation::DriveToWall(float power)
{
    int a=0;
    while (a==0)
    {
        if (FrontLeft_bumpswitch.Value() == false && FrontRight_bumpswitch.Value() == false)
        {
            Navigation::StopMotors();
            a=1;
        }
        else if (FrontLeft_bumpswitch.Value() == true && FrontRight_bumpswitch.Value() == true)
        {
            Navigation::DriveForward(power);
        }
        else if (FrontLeft_bumpswitch.Value() == false && FrontRight_bumpswitch.Value() == true)
        {
            Left_Motor.Stop();
            Right_Motor.SetPower((int)power);
        }
        else if (FrontLeft_bumpswitch.Value() == true && FrontRight_bumpswitch.Value() == false)
        {
            Right_Motor.Stop();
            Left_Motor.SetPower((int)power);
        }

    }
}

//This function turns the robot 90 degrees to the right
void Navigation::Right90Turn()
{

}

//This function turns the robot 90 degrees left
void Navigation::Left90Turn()
{

}

//This robot turns the robot right to a specified angle
void Navigation::RightTurn(float angle)
{

}

//This function turns the robot left to a specified angle
void Navigation::LeftTurn(float angle)
{

}

//This function stops the robot
void Navigation::StopMotors()
{

    Left_Motor.SetPower(0);
    Right_Motor.SetPower(0);
}

//This function drives the robot forward
void Navigation::DriveForward(float power)
{

    Left_Motor.SetPower((int)(power*forward_calibration));
    Right_Motor.SetPower((int)(power*forward_calibration));
}

//This function drives the robot backward
void Navigation::DriveBackward(float power)
{

    Left_Motor.SetPower((int)(power*reverse_calibration));
    Right_Motor.SetPower((int)(power*reverse_calibration));
}
