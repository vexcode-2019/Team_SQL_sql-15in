/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       john                                                      */
/*    Created:      Mon Dec 16 2019                                           */
/*    Description:  V5 project                                                */
/*                                                                            */
/*----------------------------------------------------------------------------*/

// ---- START VEXCODE CONFIGURED DEVICES ----
// ---- END VEXCODE CONFIGURED DEVICES ----

#include "vex.h"
#include "devices.h"

using namespace vex;
using std::string;

#define DEADZONE 5 //percent
#define SCALE_FACTOR_FAST 1.0
#define SCALE_FACTOR_MEDIUM 0.5
#define SCALE_FACTOR_SLOW 0.25
float scaleFactor = SCALE_FACTOR_MEDIUM;

enum driveSpeed{
  fast,
  medium,
  slow
};

string driveSpeedText = "";

void updateScreen(){
  ControllerScreen.clearScreen();
  ControllerScreen.setCursor(1, 0);
  ControllerScreen.print("Drive Speed: %s", driveSpeedText.c_str());
  ControllerScreen.setCursor(2, 0);
  ControllerScreen.print("Battery: %d%%", Battery.capacity());
}

void setDriveSpeed(driveSpeed speed){
  //set the drive speed to the appropriate value
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

int inline joyaxis(int i) {
  return (abs(i) > DEADZONE ? int(pow(double(i)/100, 3)*100) : 0);
}

void pre_auton(){
  setDriveSpeed(medium);
  timer::event(updateScreen, 5000);
}

void teleop(){
  Controller.ButtonA.released([]{ 
    setDriveSpeed(fast);
  });
  Controller.ButtonB.released([]{ 
    setDriveSpeed(medium);
  });
  Controller.ButtonX.released([]{ 
    setDriveSpeed(slow);
  });

  while (true){
    int axis1 = joyaxis(Controller.Axis1.position());
    int axis3 = joyaxis(Controller.Axis3.position());
    LDrive.spin(fwd, scaleFactor * (axis3 + axis1), percent);
    RDrive.spin(fwd, scaleFactor * (axis3 - axis1), percent);

    if (Controller.ButtonL1.pressing()){
      // Lift.spin(fwd, 30, percent);
      LLift.spin(fwd, 60, percent);
      RLift.spin(fwd, 60, percent);
    }
    else if (Controller.ButtonL2.pressing()){
      // Lift.spin(reverse, 30, percent);
      LLift.spin(reverse, 60, percent);
      RLift.spin(reverse, 60, percent);
    }
    else{
      // Lift.stop(hold);
      LLift.stop(hold);
      RLift.stop(hold);
    }

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

void auton(){

}

int main() {
  // Initializing Robot Configuration. DO NOT REMOVE!
  vexcodeInit();

  Competition.autonomous(auton);
  Competition.drivercontrol(teleop);

  pre_auton();

  while(true){

    wait(20, msec);
  }

}
