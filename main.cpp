//Last edited by: Eric Magnuson
//Last date edited: 4/21/13
//Version Number: 6.2
//Tested since last update: Yes
//Final commented version of competition code

//Libraries to be included
#include <FEHLCD.h>
#include <FEHIO.h>
#include <FEHUtility.h>
#include <FEHMotor.h>
#include <FEHServo.h>
#include <FEHBuzzer.h>
#include <FEHMOM.h>


//Define Statements
#define forward  1
#define backward  -1
#define left_motor FEHMotor::Motor3
#define right_motor FEHMotor::Motor1
#define clicks_per_turn 32
#define wheel_diameter 2.75
#define pi 3.1415926h

//Global Variables for IO pins
DigitalInputPin FrontRight_bumpswitch( FEHIO::P2_7 );
DigitalInputPin FrontLeft_bumpswitch( FEHIO::P0_0 );
AnalogInputPin LineFollowingOptosensor(FEHIO:: P1_7);
DigitalInputPin CalibrationSwitch(FEHIO:: P1_4);
DigitalInputPin RightCenterSwitch(FEHIO:: P2_1);
DigitalInputPin LeftCenterSwitch(FEHIO :: P0_6);
ButtonBoard buttons( FEHIO::Bank3 );
AnalogInputPin cds_cell( FEHIO::P1_0 );
FEHMotor Left_Motor(FEHMotor::Motor0), Right_Motor(FEHMotor::Motor3);
FEHEncoder Left_Encoder(FEHIO::P0_7);
FEHEncoder Right_Encoder(FEHIO::P2_0);
FEHServo Hook( FEHServo::Servo0 );

//Threshold variables
float CDS_Threshold=1.912;
float Line_Following_Threshold=2.862;
float Right_forward_calibration=.908;
float Right_reverse_calibration=1;
float Left_forward_calibration=1;
float Left_reverse_calibration=.965;
int Right_Turn_Clicks = 40;
int Left_Turn_Clicks = 38;



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
    void LightStartV2();

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
    void DistanceTravelled(float distance, float power, int direction, float time);//Coded, commented
    void DriveToWall(float power, float time);//Coded, commented
    void DriveToLine(float power,float distance);//Coded, commented
    void FindLine(float power, float distance);
    void Right90Turn();//Coded
    void Left90Turn();//Coded
    void RightTurn(float angle);//Coded
    void LeftTurn(float angle);//Coded
    void DriveForward(float power);//Coded, commented
    void DriveBackward(float power);//Coded, commented
    void StopMotors();//Coded, commented
    void RunCoursePart1();//Coded
    void GrabSled();//not Coded
    void FollowLine(float distance, float power);//Coded
    void DriveToCrevice();
private:

};


int main(void)
{
    //Set up screen
    LCD.Clear( FEHLCD::Green );
    LCD.SetFontColor( FEHLCD::Black );
    //Initialize classes
    StartUp BootUp;
    Navigation NewRun;

    //Run functions for course
    BootUp.Begin();
    NewRun.RunCoursePart1();

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
            Sleep(1.);
        }
    }

    //Start Functions to begin a test run

    //Set up servo
    Hook.SetMax(2184);
    Hook.SetMin(533);
    Hook.SetDegree(180);

    //Initialize Mom
    MOM.InitializeMenu();

    //Run button and light start functions
    StartUp::RunAllStart();
}

//New Run Functions
void StartUp::RunAllStart()
{
    StartUp::Button_Start();

    //Enable MOM
    MOM.Enable();

    StartUp::LightStartV2();
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
        else if (TimeNow()-b >= 30)
        {
            LCD.WriteLine("Light detection timeout, beginning run");
            a=1;
        }

    }

    //Drive forward to let optosensor clear the start box

}

void StartUp::LightStartV2()
{

    int a = 0,b;
    float start;
    LCD.WriteLine("Waiting for light");

    //Start timing to setup timeout
    b=TimeNow();

    start = cds_cell.Value();

    //Start loop to wait for light or timeout
    while (a==0)
    {
        //Exit loop if change in light is detected
        if (start - cds_cell.Value()>= .5)
        {
            a=1;
            LCD.WriteLine("Light Detected, Beginning Run");
        }
        //Exit loop if light is not detected in ten seconds
        else if (TimeNow()-b >= 30)
        {
            LCD.WriteLine("Light detection timeout, beginning run");
            a=1;
        }

    }

    //Drive forward to let optosensor clear the start box

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
    //In the final competition, the motors did not need calibration and the CDS cell was utilized differently

    //StartUp::CDSCellCalibration();
    StartUp::OptoCalibration();
    //StartUp::MotorCompensation();
    Sleep(1.0);
    //StartUp::LeftTurnCalibration();
    Sleep(1.0);
    //StartUp::RightTurnCalibration();
}
//This function sets the threshold for the central CDS cell
void StartUp::CDSCellCalibration()
{
    //Declare variables
    int a=0;
    float lower_value, upper_value;

    //Set the lower value
    LCD.WriteLine("Place robot on course and Turn Light Off");
    LCD.WriteLine("Press middle button to set upper bound");
    while (a==0)
    {
        //Tell user to press middle button to set bound

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
    Sleep(1.0);
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
            //Read in upper value
            upper_value = LineFollowingOptosensor.Value();
            LCD.WriteLine(upper_value);

            //Exit loop
            Sleep(1.0);
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
            //Read in lower value
            lower_value = LineFollowingOptosensor.Value();
            LCD.WriteLine(lower_value);
            Sleep(1.0);
            a=1;
        }

    }

    //Calculate average value for later use
    Line_Following_Threshold = (upper_value+lower_value)/2;

    //Provide the user with the actual value of the threshold
    LCD.WriteLine("Value of threshold is");
    LCD.WriteLine(Line_Following_Threshold);
    Sleep(1.0);


}
//This function is used to calculate the compensation factor for the motors
void StartUp::MotorCompensation()
{
    int a=0;

    //Reset Encoder Counts
    Left_Encoder.ResetCounts();
    Right_Encoder.ResetCounts();

    //Run Motors forward for five seconds and record counts
    Left_Motor.SetPower(127);
    Right_Motor.SetPower(127);
    Sleep(5.0);
    Left_Motor.Stop();
    Right_Motor.Stop();
    LCD.WriteLine(Left_Encoder.Counts());
    LCD.WriteLine(Right_Encoder.Counts());

    //Run a comparison to calculate forward compensation factor
    if (Left_Encoder.Counts()<Right_Encoder.Counts())
    {
        Right_forward_calibration = (float)((float)Left_Encoder.Counts()/(float)Right_Encoder.Counts());
        Left_forward_calibration = 1;
        LCD.WriteLine("Right forward calibration is ");
        LCD.WriteLine(Right_forward_calibration);
        Sleep(2.0);
    }
    else if ((float)Left_Encoder.Counts()>(float)Right_Encoder.Counts())
    {
        Left_forward_calibration = ((float)Right_Encoder.Counts()/(float)Left_Encoder.Counts());
        Right_forward_calibration = 1;
        LCD.WriteLine("Left forward calibration is ");
        LCD.Write(Left_forward_calibration);
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
    Left_Motor.SetPower(-127);
    Right_Motor.SetPower(-127);
    Sleep(10.0);
    Left_Motor.Stop();
    Right_Motor.Stop();

    //Run a comparison to calculate forward compensation factor
    if (Left_Encoder.Counts()<Right_Encoder.Counts())
    {
        Right_reverse_calibration = ((float)Left_Encoder.Counts()/(float)Right_Encoder.Counts());
        Left_reverse_calibration = 1;
        LCD.Clear();
        LCD.WriteLine("Right backward calibration is ");
        LCD.WriteLine(Right_reverse_calibration);
        Sleep(2.0);
    }
    else if (Left_Encoder.Counts()>Right_Encoder.Counts())
    {
        Left_reverse_calibration = ((float)Right_Encoder.Counts()/(float)Left_Encoder.Counts());
        Right_reverse_calibration = 1;
        LCD.Clear();
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
    int a=0;
    //Reset Encoders
    Left_Encoder.ResetCounts();
    Right_Encoder.ResetCounts();

    //Pause for user
    Sleep(2.0);

    //Turn robot until switch is pressed
    Right_Motor.SetPower(75);
    while (a==0)
    {
        if (CalibrationSwitch.Value()==false)
        {
            a=1;
        }
    }
    Right_Motor.Stop();

    //Use detected clicks to calibrate left turn
    Left_Turn_Clicks = Right_Encoder.Counts();
    LCD.WriteLine("Number of clicks for left turn");
    LCD.WriteLine(Left_Turn_Clicks);
    Sleep(2.0);
}

//This function is ued to calibrate 90 degree right turns
void StartUp::RightTurnCalibration()
{
    int a=0;
    //Reset Encoders
    Left_Encoder.ResetCounts();
    Right_Encoder.ResetCounts();

    //Pause for user
    Sleep(2.0);

    //Turn robot until switch is pressed
    Left_Motor.SetPower(75);
    while (a==0)
    {
        if (CalibrationSwitch.Value()==false)
        {
            a=1;
        }
    }
    Left_Motor.Stop();

    //Use left encoder to set counts
    Right_Turn_Clicks = Left_Encoder.Counts();
    LCD.WriteLine("Number of clicks for right turn");
    LCD.WriteLine(Right_Turn_Clicks);
    Sleep(2.0);
}

//Start Function Definitions for Navigation Class
//Constructor Function
Navigation::Navigation()
{

}

//Line Following Function
void Navigation::FollowLine(float distance, float power)
{
    int clicks;

    //Reset Encoders
    Left_Encoder.ResetCounts();
    Right_Encoder.ResetCounts();

    //Calculate Required number of clicks
    clicks = distance/(pi*wheel_diameter)*clicks_per_turn;

    //Run until the timeout distance is travelled
    while (Left_Encoder.Counts()<= clicks && Right_Encoder.Counts()<=clicks)
    {
        //Turn left if robot is on the line
        if (LineFollowingOptosensor.Value()<=Line_Following_Threshold)
        {
            Left_Motor.SetPower(70);
            Right_Motor.SetPower(50);
            Sleep(.01);
        }
        //Turn robot to the right if it is off the line
        else if (LineFollowingOptosensor.Value()>=Line_Following_Threshold)
        {
            Right_Motor.SetPower(70);
            Left_Motor.SetPower(50);
            Sleep(.01);
        }
    }
    Navigation::StopMotors();
}

//This function travels the specified distance at the provided speed
void Navigation::DistanceTravelled(float distance, float power, int direction, float time)
{
    //Declare variables
    int clicks;
    float start_time;

    //Reset Encoders
    Left_Encoder.ResetCounts();
    Right_Encoder.ResetCounts();

    //Calculate Required number of clicks
    clicks = distance/(pi*wheel_diameter)*clicks_per_turn;

    start_time = TimeNowSec();

    //Use outer selection structure to choose direction of travel based on user input
    if (direction ==forward)
    {
        //Turn motors on
        Navigation::DriveForward(power);

        //Run loop until calculated number of is reached
        while (Left_Encoder.Counts()<= clicks && Right_Encoder.Counts()<=clicks  && (TimeNowSec()-start_time)<=time)
        {
        }
        //Turn off motors
        Navigation::StopMotors();
    }
    else if (direction == backward)
    {
        //Set motor power
        Navigation::DriveBackward(power);
        //Run loop until calculated number of is reached and then turn off the motors
        while (Left_Encoder.Counts()<= clicks && Right_Encoder.Counts()<=clicks  && (TimeNow()-start_time)<=time)
        {

        }
        Navigation::StopMotors();
    }
    LCD.WriteLine("Distance travelled");
}

//This function drives until a line is encountered
void Navigation::DriveToLine(float power,float distance)
{
    int a=0,clicks;
    //Reset Encoders
    Left_Encoder.ResetCounts();
    Right_Encoder.ResetCounts();

    //Calculate Required number of clicks
    clicks = distance/(pi*wheel_diameter)*clicks_per_turn;



    //Supply power to the motors while the front optosensor doesn't detect a line
    Navigation::DriveForward(power);
    while (a==0)
    {
        LCD.WriteLine(LineFollowingOptosensor.Value());
        //If the line is detected, stop the robot
        if (LineFollowingOptosensor.Value()<=Line_Following_Threshold)
        {
            a=1;
        }
        //If the robot doesn't detect the line in the specified distance, return error code
        else if (LineFollowingOptosensor.Value()>=Line_Following_Threshold && (Left_Encoder.Counts()>= clicks || Right_Encoder.Counts() >= clicks))
        {
            a=2;
        }
    }

    Navigation::StopMotors();

    //Turn robot left to find line
    Right_Motor.SetPower(80);
    Left_Encoder.ResetCounts();
    Right_Encoder.ResetCounts();
    if (a==2)
    {
        a=0;
        Right_Motor.SetPower(80);
        while (a==0)
        {
            //If the line is detected, exit loop and stop robot
            if (LineFollowingOptosensor.Value()<=Line_Following_Threshold)
                {
                    a=1;
                }

            else if (Right_Encoder.Counts()>= Left_Turn_Clicks)
            //If robot doesn't detect line, stop and start turning to the left
            {
            a=3;
            Navigation::StopMotors();
            }
        }
    }

    //Reset encoders
    Left_Encoder.ResetCounts();
    Right_Encoder.ResetCounts();

    //If the robot still hasn't found the line, it starts to turn back in the other direction
    if (a==3)
    {
        Right_Motor.SetPower(-80);
        while (a==3)
        {
            //Turn back to its original directions
            if (Right_Encoder.Counts()>=Left_Turn_Clicks)
            {
                Navigation::StopMotors();
                a=0;
            }
            //If the robot finds the line, stop the robot
            else if (LineFollowingOptosensor.Value()<=Line_Following_Threshold)
            {
                Navigation::StopMotors();
                a=1;
            }
        }
        //Reset encoders
        Left_Encoder.ResetCounts();
        Right_Encoder.ResetCounts();

        Left_Motor.SetPower(80);
        while (a==0)
        {
        //If robot detects line, stop the robot
        if (LineFollowingOptosensor.Value()<=Line_Following_Threshold)
                {
                    a=1;
                }
        //If the robot doesn't, turn it 90 degrees left
        else if (Left_Encoder.Counts()>= Right_Turn_Clicks)
        {
            a=3;
        }
        }
    }
    //Turn off the motors when it hits a line
    Navigation::StopMotors();
    LCD.WriteLine("Line Found");
}

//Not used in competition, see above function which is identical except for a more complicated failsafe
void Navigation::FindLine(float power, float distance)
{
        int a=0,clicks, clicks2;
        //Reset Encoders
        Left_Encoder.ResetCounts();
        Right_Encoder.ResetCounts();

        //Calculate Required number of clicks
        clicks = distance/(pi*wheel_diameter)*clicks_per_turn;
        clicks2 = 3/(pi*wheel_diameter)*clicks_per_turn;



        //Supply power to the motors while the front optosensor doesn't detect a line
        Navigation::DriveForward(power);
        while (a==0)
        {
            LCD.WriteLine(LineFollowingOptosensor.Value());
            if (LineFollowingOptosensor.Value()<=Line_Following_Threshold)
            {
                a=1;
            }
            else if (LineFollowingOptosensor.Value()>=Line_Following_Threshold && (Left_Encoder.Counts()>= clicks || Right_Encoder.Counts() >= clicks))
            {
                a=2;
            }
        }
        Navigation::StopMotors();
        //Drive backward if line isn't detected
        if (a==2)
        {
            Navigation::LeftTurn(4);
            Navigation::DistanceTravelled(3,100,backward,10);
            while (a==2)
            {
                LCD.WriteLine(LineFollowingOptosensor.Value());
                if (LineFollowingOptosensor.Value()<=Line_Following_Threshold)
                {
                    a=1;
                    Navigation::StopMotors();
                }
                else if (LineFollowingOptosensor.Value()>=Line_Following_Threshold && (Left_Encoder.Counts()>= clicks2 || Right_Encoder.Counts() >= clicks2))
                {
                    a=0;
                    Navigation::StopMotors();
                }

            }

        }
}

//This function drives the robot forward until it is square against a wall
void Navigation::DriveToWall(float power, float time)
{
    int a=0;
    float start_time;
    start_time =TimeNowSec();
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
            Sleep(.1);
        }
        //If left switch is pressed, drive the right wheel forward and stop the left wheel
        else if (FrontLeft_bumpswitch.Value() == false && FrontRight_bumpswitch.Value() == true)
        {
            Left_Motor.Stop();
            Right_Motor.SetPower((int)power);
            Sleep(.1);
        }
        //If the right switch is pressed, drive the left wheel forward and stop the right wheel
        else if (FrontLeft_bumpswitch.Value() == true && FrontRight_bumpswitch.Value() == false)
        {
            Right_Motor.Stop();
            Left_Motor.SetPower((int)power);
            Sleep(.1);
        }
        //Timeout if robot doesn't square quickly enough
        else if (TimeNow()-start_time >= time)
        {
            LCD.WriteLine("Light detection timeout, beginning run");
            Navigation::StopMotors();
            a=1;
        }

    }
    Navigation::StopMotors();
    LCD.WriteLine("Wall Found");
}


//This function turns the robot 90 degrees to the right
void Navigation::Right90Turn()
{
    int a=0;
    Left_Encoder.ResetCounts();
    Left_Motor.SetPower(75);

    //Use predetermined number of clicks to turn robot 90 degrees
    while (a==0)
    {
       if (Left_Encoder.Counts()>Right_Turn_Clicks)
       {
           a=1;
       }
    }

    //Stop robot
    Left_Motor.Stop();
    LCD.WriteLine("Turn Completed");
}

//This function turns the robot 90 degrees left
void Navigation::Left90Turn()
{
    int a=0;
    Right_Encoder.ResetCounts();
    Right_Motor.SetPower(75);

    //Turn robot predetermined number of clicks
    while (Right_Encoder.Counts()<Left_Turn_Clicks)
    {

    }

    //Stop robot after turn
    Right_Motor.Stop();
    LCD.WriteLine("Turn Completed");

}

//This robot turns the robot right to a specified angle
void Navigation::RightTurn(float angle)
{
    float revised_clicks;
    int a=0;

    //Calculate clicks using 90 turn to find clicks per degree and then multiply by number of degrees needed
    revised_clicks = Right_Turn_Clicks/90.*angle;
    Left_Encoder.ResetCounts();
    Left_Motor.SetPower(75);

    //Continue looping until robot has turned desired number of clicks
    while (a==0)
    {
        if (Left_Encoder.Counts()>revised_clicks)
        {
            a=1;
        }
    }

    //Stop robot when turn is completed
    Left_Motor.Stop();
    LCD.WriteLine("Turn Completed");
}

//This function turns the robot left to a specified angle
void Navigation::LeftTurn(float angle)
{
    float revised_clicks;
    int a=0;

    //Calculate clicks using 90 turn to find clicks per degree and then multiply by number of degrees needed
    revised_clicks = Left_Turn_Clicks/90.*angle;
    Right_Encoder.ResetCounts();
    Right_Motor.SetPower(75);

    //Continue robot untio number of clicks is reached
    while (a==0)
    {
        if (Right_Encoder.Counts()>revised_clicks)
        {
           a=1;
        }
    }

    //Stop robot when turn is completed
    Right_Motor.Stop();
    LCD.WriteLine("Turn Completed");
}

//This function stops the robot
void Navigation::StopMotors()
{
    //Turn off the motors
    Left_Motor.Stop();
    Right_Motor.Stop();
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

//Not used in final competition
void Navigation::DriveToCrevice()
{
    Navigation::DriveForward(90);
    while (RightCenterSwitch.Value() == true || LeftCenterSwitch.Value() == true)
    {

    }
    Navigation::StopMotors();

}

//Grabs the sled
void Navigation::GrabSled()
{
    //Set the servo for grabbing the hook and give time for servo to move
    Hook.SetDegree(60);
    Sleep(.5);
}

//This function runs the course and contains all of the steps to run the course
void Navigation::RunCoursePart1()
{
    //Leave start box and drive to first line
    Navigation::DistanceTravelled(10.,100.0,forward,20);
    Navigation::DriveToLine(100.,10);

    //Drive forward to clear satellite, turn towards and drive up ramp
    Navigation::DistanceTravelled(5.,80.,forward,20);
    Navigation::Left90Turn();
    Navigation::DistanceTravelled(21.,127.,forward,20);

    //Turn towards line, drive towards it, and follow line to crevice
    Navigation::Left90Turn();
    //Navigation::DriveToWall(70.);
    Navigation::DriveToLine(90,14);
    Navigation::FollowLine(10.,90);
    Navigation::LeftTurn(5);
    Navigation::DriveToWall(127,1.5);
    Navigation::DistanceTravelled(2,100,backward,1.5);
    Navigation::RightTurn(5);
    Navigation::DriveToWall(127,1);
    //Navigation::DistanceTravelled(4.,127,backward,5);
    //Navigation::DistanceTravelled(5.,120,forward,5);
    //Navigation::DistanceTravelled(4.,127,backward,5);
    //Navigation::DistanceTravelled(5.,120,forward,5);
    //Navigation::RightTurn(2);

    //Drive backwards to the stone
    Navigation:: DistanceTravelled(24,127,backward,15);

    //Travel over to other wall, square against it, back up and grab sled
    Navigation::DistanceTravelled(5.,100,forward,20);
    Navigation::Left90Turn();
    Navigation::DistanceTravelled(2.5,100,forward,20);
    Navigation::Left90Turn();
    Navigation::DistanceTravelled(10,100,forward,5);
    Navigation::LeftTurn(30);
    Navigation::DriveToWall(120,6);
    Navigation::DistanceTravelled(3,100,backward,10);
    Navigation::Left90Turn();
    Navigation::DriveToWall(120,5);
    Navigation::DistanceTravelled(5,100,backward,10);
    Navigation::RightTurn(10);
    Navigation::DriveToWall(100,6);
    Navigation::DistanceTravelled(1,110,backward,8);
    Navigation::LeftTurn(12.1);
    Navigation::DistanceTravelled(16.5,110,backward,8);
    //Navigation::DistanceTravelled(2,60,forward,4);
    Navigation::GrabSled();

    //Drive over to stairs and turn down the stairs
    Navigation::DistanceTravelled(2,100,forward,10);
    Navigation::LeftTurn(30);
    Navigation::DistanceTravelled(1,100,forward,10);
    Navigation::LeftTurn(30);
    Navigation::DistanceTravelled(1,100,forward,10);
    Navigation::DistanceTravelled(15.,127,forward,10);
    Navigation::Left90Turn();
    Navigation::LeftTurn(14);
    Hook.SetDegree(40);
    Navigation::DistanceTravelled(5,80,forward,3);
    //Hook.SetDegree(40);
    Navigation::DistanceTravelled(5,80,forward,3);
    Navigation::DriveToWall(100,15);

    //Release sled, back up, turn towards satellite
    Hook.SetDegree(180);
    Navigation::DistanceTravelled(10,127,backward,5);
    Navigation::Left90Turn();
    Navigation::LeftTurn(10);
    Navigation::DriveToLine(100,10);
    //Navigation::DistanceTravelled(1,100,forward,10);
    Navigation::Left90Turn();
    //Navigation::LeftTurn(45);
    //Navigation::DriveToLine(100,0);
    //Navigation::FollowLine(2,100);

    //Drive into button, reverse towards switch
    Navigation::DriveToWall(127,5);
    Navigation::DistanceTravelled(3,100,backward,5);
    Navigation::LeftTurn(20);
    Navigation::DistanceTravelled(18,100,backward,10);
    Navigation::DistanceTravelled(3,127,forward,10);

    Navigation::RightTurn(20);
    Navigation::DistanceTravelled(12,127,backward,10);




    //Navigation::Left90Turn();
    Sleep(1.1);
    //Navigation::DistanceTravelled(20.,80.,forward);
    Sleep(1.1);
    //Navigation::DistanceTravelled(1.5,80.,backward);
    Sleep(1.1);



}
