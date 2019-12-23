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

int inline joyaxis(int i) {
  return (abs(i) > DEADZONE ? int(pow(double(i)/100, 3)*100) : 0);
}

void printBattery(){
  ControllerScreen.setCursor(2,1);
  ControllerScreen.print("Battery: %d", Battery.capacity());

  timer::event(printBattery, 5000);
}

void pre_auton(){
  ControllerScreen.clearScreen();
  ControllerScreen.setCursor(1, 1);
  ControllerScreen.print("Drive Speed: Medium");
  printBattery();
}

void teleop(){
  Controller.ButtonA.released([]{ 
    scaleFactor = SCALE_FACTOR_FAST;
      ControllerScreen.setCursor(1, 1);
      ControllerScreen.clearLine();
      ControllerScreen.print("Drive Speed: Fast");
  });
  Controller.ButtonB.released([]{ 
    scaleFactor = SCALE_FACTOR_MEDIUM; 
      ControllerScreen.setCursor(1, 1);
      ControllerScreen.clearLine();
      ControllerScreen.print("Drive Speed: Medium");
  });
  Controller.ButtonX.released([]{ 
    scaleFactor = SCALE_FACTOR_SLOW;
      ControllerScreen.setCursor(1, 1);
      ControllerScreen.clearLine();
      ControllerScreen.print("Drive Speed: Slow");
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
