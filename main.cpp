//Last edited by: Eric Magnuson
//Last date edited: 3/24/13
//Version Number: 3.2
//Tested since last update: No

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
#define clicks_per_turn 32
#define wheel_diameter 2.75
#define pi 3.1415926

//Global Variables for IO pins
DigitalInputPin FrontRight_bumpswitch( FEHIO::P2_7 );
DigitalInputPin FrontLeft_bumpswitch( FEHIO::P0_0 );
DigitalInputPin LineFollowingOptosensor(FEHIO:: P1_0);
DigitalInputPin CalibrationSwitch(FEHIO:: P1_4);
ButtonBoard buttons( FEHIO::Bank3 );
AnalogInputPin cds_cell( FEHIO::P1_0 );
FEHMotor Left_Motor(FEHMotor::Motor3), Right_Motor(FEHMotor::Motor1);
FEHEncoder Left_Encoder(FEHIO::P0_1);
FEHEncoder Right_Encoder(FEHIO::P2_1);

//Threshold variables
float CDS_Threshold=.14;
float Line_Following_Threshold;
float Right_forward_calibration=1;
float Right_reverse_calibration=1;
float Left_forward_calibration=1;
float Left_reverse_calibration=1;
int Right_Turn_Clicks = 35;
int Left_Turn_Clicks = 35;

//This class runs first and can be used to test and calibrate sensors
class StartUp
{
    //Public Elements
public:
    //Constructor Function
    StartUp();

    //Function used to ask user for calibration or direct run
    void Begin(); //Coded, commented, needs to be fixed

    //Beginning a new run functions
    void RunAllStart();//Coded
    void Button_Start();//Coded, commented
    void Light_Start();//Coded, commented

    //Functions for checking motors and sensors
    void RunAllTest();//Coded
    void SwitchCheck(); //Coded, commented
    void Check_Right_Encoder();//Coded, commented
    void Check_Left_Encoder();//Coded, commented
    void MotorTest();//Coded, Commented

    //Functions for calibrating sensors
    void RunAllCalibration();//Coded
    void CDSCellCalibration(); //Coded, commented
    void OptoCalibration();//Coded, commented
    void MotorCompensation();//Coded
    void LeftTurnCalibration();//Coded
    void RightTurnCalibration();//Coded

private:

};

//Navigation Class Definition
//This class is used to drive the robot around the course
class Navigation
{
public:
    Navigation();
    void DistanceTravelled(float distance, float power, int direction);//Coded, commented
    void DriveToWall(float power);//Coded, commented
    void DriveToLine(float power);//Coded, commented
    void Right90Turn();//Coded
    void Left90Turn();//Coded
    void RightTurn(float angle);//Coded
    void LeftTurn(float angle);//Coded
    void DriveForward(float power);//Coded, commented
    void DriveBackward(float power);//Coded, commented
    void StopMotors();//Coded, commented
    void RunCourse();//Not Coded yet
    void GrabSled();//Coded
private:

};


int main(void)
{
    LCD.Clear( FEHLCD::Black );
    LCD.SetFontColor( FEHLCD::White );
    StartUp BootUp;
    Navigation NewRun;
    BootUp.Begin();
    NewRun.RunCourse();
    return 0;
}



//Begin Function Definitons for StartUp Class
//Constructor Function
StartUp::StartUp()
{

}
//Start/Mode Selector Function
//This function asks the user if calibration is needed, and then runs the course as necessary
void StartUp::Begin()
{
    //Declare variables
    int a=0;

    //Ask User for inputs
    LCD.WriteLine("Do you need to test sensors");
    LCD.WriteLine("Press Left for Yes, Right For No");
    while (a==0)
    {
        //Calibration Condition
        if (buttons.LeftPressed())
        {
            //Start Calibration functions
            StartUp::RunAllTest();

            //Exit Loop
            a=1;
        }
        //Start Run condition
        if (buttons.RightPressed())
        {
            a=1;
        }
    }


    //Ask User for input
    LCD.WriteLine("Do you need to calibrate?\nPress Left for yes, Middle for No");


    //Loop until user inputs their selection
    a=0;
    while (a==0)
    {
        //Calibration Condition
        if (buttons.LeftPressed())
        {
            //Start Calibration functions
            StartUp::RunAllCalibration();

            //Exit Loop
            a=1;
        }
        //Start Run condition
        if (buttons.MiddlePressed())
        {
            a=1;
        }
    }

    //Start Functions to begin a test run
    StartUp::RunAllStart();
}

//New Run Functions
void StartUp::RunAllStart()
{
    StartUp::Button_Start();
    StartUp::Light_Start();
}
//This function lets the robot wait for the start button to be pressed
void StartUp::Button_Start()
{

    int a=0;
    LCD.WriteLine("Waiting For left Button");
    while (a==0)
    {
        //Start the rest of the program if one of the buttons is pressed
        if (buttons.LeftPressed())
        {
            a=1;
        }
    }
    //Tell User that the robot is ready for the light
    LCD.WriteLine("Start Button Pressed");

}

//This function makes the robot wait until the light is activated or ten seconds have passed
void StartUp::Light_Start()
{

    int a = 0,b;
    LCD.WriteLine("Waiting for light");

    //Start timing to setup timeout
    b=TimeNow();

    //Start loop to wait for light or timeout
    while (a==0)
    {
        //Exit loop if light is detected
        if (cds_cell.Value( )<= CDS_Threshold)
        {
            a=1;
            LCD.WriteLine("Light Detected, Beginning Run");
        }
        //Exit loop if light is not detected in ten seconds
        else if (TimeNow()-b >= 10)
        {
            LCD.WriteLine("Light detection timeout, beginning run");
            a=1;
        }

    }

    //Drive forward to let optosensor clear the start box
    Left_Motor.SetPower(100);
    Right_Motor.SetPower(100);
    Sleep(1.0);
    Left_Motor.SetPower(0);
    Right_Motor.SetPower(0);
}

//Sensor, Motor Check Functions
//Run All Tests Functions
void StartUp::RunAllTest()
{
    StartUp::SwitchCheck();
    StartUp::Check_Right_Encoder();
    StartUp::Check_Left_Encoder();
    StartUp::MotorTest();
}
//This function checks to make sure the microswitches are working
void StartUp::SwitchCheck()
{

    int a=0;
    //This function will test the switches
    LCD.WriteLine("Please press the front right switch");
    while (a==0)
    {
        //Exit the loop if the switch is pressed and works correctly
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
        //Exit the loop if the switch is pressed and functioning normally
        if (FrontLeft_bumpswitch.Value()==false)
        {
            a=1;
            LCD.WriteLine("Switch is functioning normally");
        }
    }


}
//This function checks the right encoder
void StartUp::Check_Right_Encoder()
{
    //Declare Variables
    int a;

    //Reset Encoder
    Right_Encoder.ResetCounts();

    //Start Motor
    Right_Motor.SetPower(50);

    //Start Timing
    a=TimeNow();

    //Run loop for 5 seconds
    while (TimeNow()-a<=5)
    {
        //Print encoder counts
        LCD.WriteLine(Right_Encoder.Counts());
    }

    //Stop motor
    Right_Motor.Stop();
}
//This function checks the left encoder
void StartUp::Check_Left_Encoder()
{
    //Declare Variables
    int a;

    //Reset Encoder Counts
    Left_Encoder.ResetCounts();

    //Turn Motor on
    Left_Motor.SetPower(50);

    //Start Timing
    a=TimeNow();

    //Run loop for 5 seconds
    while (TimeNow()-a<=5)
    {
        //Print encoder counts
        LCD.WriteLine(Left_Encoder.Counts());
    }

    //Stop Motors
    Left_Motor.Stop();
}
//This tests the motors
void StartUp::MotorTest()
{
    //The motors should run forwards if wired correctly
    LCD.WriteLine("Motors should run forwards");
    Left_Motor.SetPower(50);
    Right_Motor.SetPower(50);
    Sleep(5.0);
    Left_Motor.Stop();
    Right_Motor.Stop();
}

//Calibrations Functions
//Run All Calibration Function
void StartUp::RunAllCalibration()
{
    StartUp::CDSCellCalibration();
    StartUp::OptoCalibration();
    StartUp::MotorCompensation();
    StartUp::LeftTurnCalibration();
    StartUp::RightTurnCalibration();
}
//This function sets the threshold for the central CDS cell
void StartUp::CDSCellCalibration()
{
    //Declare variables
    int a=0;
    float lower_value, upper_value;

    //Set the lower value
    LCD.WriteLine("Place robot on course and Turn Light Off");
    while (a==0)
    {
        //Tell user to press middle button to set bound
        LCD.WriteLine("Press middle button to set upper bound");
        if (buttons.MiddlePressed())
        {
            upper_value = cds_cell.Value();
            a=1;
        }
    }

    //Give user instructions
    LCD.WriteLine("Please turn the light on and place robot over light");

    //Reindex loop and gather lower bound
    a=0;
    LCD.WriteLine("Press right button to set lower bound");
    while (a==0)
    {
        if (buttons.RightPressed())
        {
            lower_value = cds_cell.Value();
            a=1;
        }
    }

    //Calculate average for real threshold
    CDS_Threshold = (upper_value+lower_value)/2;

    //Tell user value for future reference
    LCD.WriteLine("Value of threshold is");
    LCD.WriteLine(CDS_Threshold);
}

//This function is used to calibrate the line following optosensor
void StartUp::OptoCalibration()
{
    //Declare variables
    int a=0;
    float lower_value, upper_value;

    //Give user instructions and gather value for normal part of course
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

    //Give user instructions and gather reflectivity value for line
    LCD.WriteLine("Place optosensor over line");
    LCD.WriteLine("Press right button to set lower reflectivity value");
    a=0;
    while (a==0)
    {
        if (buttons.RightPressed())
        {
            lower_value = LineFollowingOptosensor.Value();
            a=1;
        }

    }

    //Calculate average value for later use
    Line_Following_Threshold = (upper_value+lower_value)/2;

    //Provide the user with the actual value of the threshold
    LCD.WriteLine("Value of threshold is");
    LCD.WriteLine(Line_Following_Threshold);


}
//This function is used to calculate the compensation factor for the motors
void StartUp::MotorCompensation()
{
    int a=0;

    //Reset Encoder Counts
    Left_Encoder.ResetCounts();
    Right_Encoder.ResetCounts();

    //Run Motors forward for five seconds and record counts
    Left_Motor.SetPower(100);
    Right_Motor.SetPower(100);
    Sleep(5.0);
    Left_Motor.Stop();
    Right_Motor.Stop();

    //Run a comparison to calculate forward compensation factor
    if (Left_Encoder.Counts()<Right_Encoder.Counts())
    {
        Right_forward_calibration = (Left_Encoder.Counts()/Right_Encoder.Counts());
        Left_forward_calibration = 1;
        LCD.WriteLine("Right forward calibration is ");
        LCD.WriteLine(Right_forward_calibration);
        Sleep(2.0);
    }
    else if (Left_Encoder.Counts()>Right_Encoder.Counts())
    {
        Left_forward_calibration = (Right_Encoder.Counts()/Left_Encoder.Counts());
        Right_forward_calibration = 1;
        LCD.WriteLine("Left forward calibration is ");
        LCD.WriteLine(Left_forward_calibration);
        Sleep(2.0);
    }
    else if (Left_Encoder.Counts()==Right_Encoder.Counts())
    {
        LCD.WriteLine("No Calibration Factor Required");
    }
    //Reset Encoder Counts
    Left_Encoder.ResetCounts();
    Right_Encoder.ResetCounts();

    //Run Motors forward for five seconds and record counts
    Left_Motor.SetPower(-100);
    Right_Motor.SetPower(-100);
    Sleep(5.0);
    Left_Motor.Stop();
    Right_Motor.Stop();

    //Run a comparison to calculate forward compensation factor
    if (Left_Encoder.Counts()<Right_Encoder.Counts())
    {
        Right_reverse_calibration = (Left_Encoder.Counts()/Right_Encoder.Counts());
        Left_reverse_calibration = 1;
        LCD.WriteLine("Right backward calibration is ");
        LCD.WriteLine(Right_reverse_calibration);
        Sleep(2.0);
    }
    else if (Left_Encoder.Counts()>Right_Encoder.Counts())
    {
        Left_reverse_calibration = (Right_Encoder.Counts()/Left_Encoder.Counts());
        Right_reverse_calibration = 1;
        LCD.WriteLine("Left reverse calibration is ");
        LCD.WriteLine(Left_reverse_calibration);
        Sleep(2.0);
    }
    else if (Left_Encoder.Counts()==Right_Encoder.Counts())
    {
        LCD.WriteLine("No Calibration Factor Required");
    }
}

//This Function is used to calibrate Left 90 degree turns
void StartUp::LeftTurnCalibration()
{
    //Reset Encoders
    Left_Encoder.ResetCounts();
    Right_Encoder.ResetCounts();

    //Pause for user
    Sleep(2.0);

    //Turn robot until switch is pressed
    while (CalibrationSwitch.Value()==true)
    {
        Right_Motor.SetPower(50);
    }
    Right_Motor.Stop();
    Left_Turn_Clicks = Right_Encoder.Counts();
    LCD.WriteLine("Number of clicks for left turn");
    LCD.WriteLine(Left_Turn_Clicks);
    Sleep(2.0);
}

//This function is ued to calibrate 90 degree right turns
void StartUp::RightTurnCalibration()
{
    //Reset Encoders
    Left_Encoder.ResetCounts();
    Right_Encoder.ResetCounts();

    //Pause for user
    Sleep(2.0);

    //Turn robot until switch is pressed
    while (CalibrationSwitch.Value()==true)
    {
        Left_Motor.SetPower(50);
    }
    Left_Motor.Stop();
    Right_Turn_Clicks = Right_Encoder.Counts();
    LCD.WriteLine("Number of clicks for right turn");
    LCD.WriteLine(Right_Turn_Clicks);
    Sleep(2.0);
}

//Start Function Definitions for Navigation Class
//Constructor Function
Navigation::Navigation()
{

}

//This function travels the specified distance at the provided speed
void Navigation::DistanceTravelled(float distance, float power, int direction)
{
    //Declare variables
    int clicks;

    //Reset Encoders
    Left_Encoder.ResetCounts();
    Right_Encoder.ResetCounts();

    //Calculate Required number of clicks
    clicks = distance/(pi*wheel_diameter)*clicks_per_turn;

    //Use outer selection structure to choose direction of travel based on user input
    if (direction ==forward)
    {
        //Run loop until calculated number of is reached
        while (Left_Encoder.Counts()<= clicks && Right_Encoder.Counts()<=clicks)
        {
            //Turn motors on
            Navigation::DriveForward(power);
        }
        //Turn off motors
        Navigation::StopMotors();
    }
    else if (direction == backward)
    {
        //Run loop until calculated number of is reached and then turn off the motors
        while (Left_Encoder.Counts()<= clicks && Right_Encoder.Counts()<=clicks)
        {
        Navigation::DriveBackward(power);
        }
        Navigation::StopMotors();
    }
}

//This function drives until a line is encountered
void Navigation::DriveToLine(float power)
{
    //Supply power to the motors while the front optosensor doesn't detect a line
    while (LineFollowingOptosensor.Value() >= Line_Following_Threshold)
    {
        Navigation::DriveForward(power);
    }
    //Turn off the motors when it hits a line
    Navigation::StopMotors();
}

//This function drives the robot forward until it is square against a wall
void Navigation::DriveToWall(float power)
{
    int a=0;
    while (a==0)
    {
        //Turn off both motors when both switches are against the wall
        if (FrontLeft_bumpswitch.Value() == false && FrontRight_bumpswitch.Value() == false)
        {
            Navigation::StopMotors();
            a=1;
        }
        //Drive forwards until one or both of the switches are activated
        else if (FrontLeft_bumpswitch.Value() == true && FrontRight_bumpswitch.Value() == true)
        {
            Navigation::DriveForward(power);
        }
        //If left switch is pressed, drive the right wheel forward and stop the left wheel
        else if (FrontLeft_bumpswitch.Value() == false && FrontRight_bumpswitch.Value() == true)
        {
            Left_Motor.Stop();
            Right_Motor.SetPower((int)power);
        }
        //If the right switch is pressed, drive the left wheel forward and stop the right wheel
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
    Left_Encoder.ResetCounts();
    while (Left_Encoder.Value()<Right_Turn_Clicks)
    {
        Left_Motor.SetPower(50);
    }
    Left_Motor.Stop();
}

//This function turns the robot 90 degrees left
void Navigation::Left90Turn()
{
    Right_Encoder.ResetCounts();
    while (Right_Encoder.Value()<Left_Turn_Clicks)
    {
        Right_Motor.SetPower(50);
    }
    Right_Motor.Stop();

}

//This robot turns the robot right to a specified angle
void Navigation::RightTurn(float angle)
{
    float revised_clicks;
    revised_clicks = Right_Turn_Clicks/90.*angle;
    Left_Encoder.ResetCounts();
    while (Left_Encoder.Value()<revised_clicks)
    {
        Left_Motor.SetPower(50);
    }
    Left_Motor.Stop();
}

//This function turns the robot left to a specified angle
void Navigation::LeftTurn(float angle)
{float revised_clicks;
    revised_clicks = Right_Turn_Clicks/90.*angle;
    Right_Encoder.ResetCounts();
    while (Right_Encoder.Value()<revised_clicks)
    {
        Right_Motor.SetPower(50);
    }
    Right_Motor.Stop();
}

//This function stops the robot
void Navigation::StopMotors()
{
    //Turn off the motors
    Left_Motor.SetPower(0);
    Right_Motor.SetPower(0);
}

//This function drives the robot forward
void Navigation::DriveForward(float power)
{
    //Turn on the motors and use the compensation number
    Left_Motor.SetPower((int)(power*Left_forward_calibration));
    Right_Motor.SetPower((int)(power*Right_forward_calibration));
}

//This function drives the robot backward
void Navigation::DriveBackward(float power)
{
    //Turn the motors on in reverse and use the compensation number
    Left_Motor.SetPower((int)(power*Left_reverse_calibration*backward));
    Right_Motor.SetPower((int)(power*Right_reverse_calibration*backward));
}

void Navigation::RunCourse()
{
    Navigation::DistanceTravelled(5.0,50.0,forward);
    Navigation::DriveToLine(50.);
    Navigation::DistanceTravelled(2.3,50.0,forward);
    Navigation::Left90Turn();
    Navigation::DistanceTravelled(15.,50.,forward);
    Navigation::RightTurn(60);
    Navigation::DriveToWall(50);
    Navigation::DistanceTravelled(5.,50.,backward);
    Navigation::Left90Turn();
    Navigation::DistanceTravelled(20.,50.,backward);
    Navigation::DistanceTravelled(1.5,50,forward);



}
