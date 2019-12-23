/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       John Holbrook, VEXU Team SQL                              */
/*    Created:      Mon Dec 16 2019                                           */
/*    Description:  Control code for Team SQL 2019-2020 15-inch robot         */
/*                                                                            */
/*----------------------------------------------------------------------------*/

//magic glue
#include "vex.h"

//device definitions
#include "devices.h"

//auto-include some common scopes
using namespace vex;
using std::string;

//constants for joystick desdzone and drive speed scaling
#define DEADZONE 5 //percent
#define SCALE_FACTOR_FAST 1.0
#define SCALE_FACTOR_MEDIUM 0.5
#define SCALE_FACTOR_SLOW 0.25

//the current drive speed scale factor
float scaleFactor = SCALE_FACTOR_MEDIUM;

//enum for different drive speeds
enum driveSpeed{
  driveFast,
  driveMedium,
  driveSlow
};

//text description of current drive speed
string driveSpeedText = "";

//Write the current drive speed and robot battery level to the controller screen
void updateScreen(){
  ControllerScreen.clearScreen();
  ControllerScreen.setCursor(1, 0);
  ControllerScreen.print("Drive Spd: %s", driveSpeedText.c_str());
  ControllerScreen.setCursor(2, 0);
  ControllerScreen.print("Battery: %d%%", Battery.capacity());
}

//set the drive speed
void setDriveSpeed(driveSpeed speed){
  //set the scale factor to the appropriate value
  switch(speed){
    case fast:
      scaleFactor = SCALE_FACTOR_FAST;
      driveSpeedText = "Fast";
    break;
    case medium:
      scaleFactor = SCALE_FACTOR_MEDIUM;
      driveSpeedText = "Medium";
    break;
    case slow:
      scaleFactor = SCALE_FACTOR_SLOW;
      driveSpeedText = "Slow";
    break;
  }

  //update the controller display
  updateScreen();
}

//apply deadzone and cubic scaling to joystick input value
int inline joyaxis(int i) {
  return (abs(i) > DEADZONE ? int(pow(double(i)/100, 3)*100) : 0);
}

//robot initialization
void pre_auton(){
  setDriveSpeed(driveMedium);
  timer::event(updateScreen, 5000);
}

//teleoperation code
void teleop(){
  //set callbacks for controller buttons
  //change drive speed:
  Controller.ButtonA.released([]{ 
    setDriveSpeed(driveFast);
  });
  Controller.ButtonB.released([]{ 
    setDriveSpeed(driveMedium);
  });
  Controller.ButtonX.released([]{ 
    setDriveSpeed(driveSlow);
  });

  //main control loop
  while (true){
    //get joystick values and apply deadzone and cubic scaling
    int axis1 = joyaxis(Controller.Axis1.position());
    int axis3 = joyaxis(Controller.Axis3.position());

    //apply correct power to drive motors, scaled by current 
    //drive speed scale factor
    LDrive.spin(fwd, scaleFactor * (axis3 + axis1), percent);
    RDrive.spin(fwd, scaleFactor * (axis3 - axis1), percent);

    //lift motors on left shoulder buttons
    if (Controller.ButtonL1.pressing()){
      Lift.spin(fwd, 50, percent);
    }
    else if (Controller.ButtonL2.pressing()){
      Lift.spin(reverse, 50, percent);
    }
    else{
      Lift.stop(hold);
    }

    //intake motors on right shoulder buttons
    if (Controller.ButtonR1.pressing()){
      Intake.spin(fwd, 75, percent);
    }
    else if (Controller.ButtonR2.pressing()){
      Intake.spin(reverse, 50, percent);
    }
    else{
      Intake.stop(hold);
    }

    wait(20, msec);
  }
}

//empty auton for now
void auton(){

}

int main() {
  // Initializing Robot Configuration. DO NOT REMOVE!
  vexcodeInit();

  //set callbacks for auton and driver control
  Competition.autonomous(auton);
  Competition.drivercontrol(teleop);

  //run initialization code
  pre_auton();
}
