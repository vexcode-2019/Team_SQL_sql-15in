/*----------------------------------------------------------------------------*/
/*                                                                            */
/*    Module:       main.cpp                                                  */
/*    Author:       John Holbrook & Philip Taylor, VEXU Team SQL              */
/*    Created:      Mon Dec 16 2019                                           */
/*    Description:  Control code for Team SQL 2019-2020 15-inch robot         */
/*                                                                            */
/*----------------------------------------------------------------------------*/

//magic glue
#include "vex.h"

//device definitions
#include "devices.h"

//compiler recommends cmath to do abs() on floats (using std::abs)
#include <cmath>

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

//uncomment to run programming skills routine in auto, leave commented for match auton
#define PROG_SKILLS

//uncomment for Blue match auton, leave commented out for red
// #define BLUE

//text description of current drive speed and lift height
string driveSpeedText = "";
string liftHeightText = "";

//Write the current drive speed and robot battery level to the controller screen
void updateScreen(){
  float liftTemp = (LLift.temperature(temperatureUnits::fahrenheit) + RLift.temperature(temperatureUnits::fahrenheit))/2.0;

  ControllerScreen.clearScreen();
  ControllerScreen.setCursor(1, 0);
  ControllerScreen.print("Drive: %s", driveSpeedText.c_str());
  ControllerScreen.setCursor(2, 0);
  ControllerScreen.print("Lift: %s", liftHeightText.c_str());
  ControllerScreen.setCursor(3, 0);
  // ControllerScreen.print("Battery: %d%%", Battery.capacity());
  ControllerScreen.print("B: %d%% T: %.0fF", Battery.capacity(), liftTemp);
}

//set the drive speed
void setDriveSpeed(driveSpeed speed){
  //set the scale factor to the appropriate value
  switch(speed){
    case driveFast:
      scaleFactor = SCALE_FACTOR_FAST;
      driveSpeedText = "Fast";
    break;
    case driveMedium:
      scaleFactor = SCALE_FACTOR_MEDIUM;
      driveSpeedText = "Medium";
    break;
    case driveSlow:
      scaleFactor = SCALE_FACTOR_SLOW;
      driveSpeedText = "Slow";
    break;
  }

  //update the controller display
  updateScreen();
}

//constants for lift height (in degrees)
#define LIFT_BOTTOM 0
#define LIFT_LOW 320
#define LIFT_MEDIUM 390
#define LIFT_HIGH 475

enum liftHeight{
  liftBottom,
  liftLow,
  liftMedium,
  liftHigh
};

void setLiftHeight(liftHeight height){
  switch (height){
    case liftBottom:
      liftHeightText = "Bottom";

      // Move down until the limit switch is reached
      Lift.spin(reverse, 50, velocityUnits::pct);
      while(!liftSwitch.pressing() && liftHeightText == "Bottom") wait(20, msec);
      if (liftHeightText != "Bottom") { return; }
      wait(250, msec);
      Lift.stop(coast);
      Lift.resetRotation();

      setDriveSpeed(driveMedium);
    break;
    case liftLow:
      liftHeightText = "Low";
      Lift.rotateTo(LIFT_LOW, deg, 50, velocityUnits::pct, false);
      setDriveSpeed(driveSlow);
    break;
    case liftMedium:
      liftHeightText = "Medium";
      Lift.rotateTo(LIFT_MEDIUM, deg, 50, velocityUnits::pct, false);
      setDriveSpeed(driveSlow);
    break;
    case liftHigh:
      liftHeightText = "High";

      // Move up until a physical stop is reached
      Lift.spin(fwd, 50, velocityUnits::pct);
      int startpos;
      do {
        startpos = Lift.position(deg);
        wait(100, msec);
      } while (std::abs(Lift.position(deg) - startpos) > 1 && liftHeightText == "High");
      if (liftHeightText != "High") { return; }
      Lift.stop(hold);

      setDriveSpeed(driveSlow);
    break;
  }
}

//apply deadzone and cubic scaling to joystick input value
int inline joyaxis(int i) {
  return (abs(i) > DEADZONE ? int(pow(double(i)/100, 3)*100) : 0);
}

//robot initialization
void pre_auton(){
  //set lift to 0 at start (for now, will later use switch to make sure it's at the bottom)

  Lift.spin(reverse, 50, velocityUnits::pct);
  wait(250, msec);
  Lift.resetRotation();
  Lift.stop(coast);
  Lift.setStopping(hold);

  //set default drive speed and lift height
  setDriveSpeed(driveMedium);
  setLiftHeight(liftBottom);

  //update the screen every 5 seconds (to reflect current battery level)
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

  //Move lift to preset heights:
  Controller.ButtonDown.released([]{
    setLiftHeight(liftBottom);
  });
  Controller.ButtonLeft.released([]{
    setLiftHeight(liftLow);
  });
  Controller.ButtonRight.released([]{
    setLiftHeight(liftMedium);
  });
  Controller.ButtonUp.released([]{
    setLiftHeight(liftHigh);
  });

  //Move lift up and down:
  Controller.ButtonL1.pressed([]{
    Lift.spin(directionType::fwd, 30, percentUnits::pct);
  });
  Controller.ButtonL1.released([]{
    Lift.stop(hold);
  });
  Controller.ButtonL2.pressed([]{
    Lift.spin(directionType::rev, 30, percentUnits::pct);
  });
  Controller.ButtonL2.released([]{
    Lift.stop(hold);
  });

  //main control loop
  while (true){
    //get joystick values and apply deadzone and cubic scaling
    int axis1 = joyaxis(Controller.Axis1.position()) * 0.8;
    int axis3 = joyaxis(Controller.Axis3.position());

    //apply correct power to drive motors, scaled by current 
    //drive speed scale factor
    LDrive.spin(fwd, scaleFactor * (axis3 + axis1), percent);
    RDrive.spin(fwd, scaleFactor * (axis3 - axis1), percent);

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

void progSkills(){
  Drivetrain.driveFor(directionType::fwd, 14, distanceUnits::in);
  Drivetrain.driveFor(directionType::rev, 7, distanceUnits::in);

  Drivetrain.turnFor(66, rotationUnits::deg);

  Intake.spin(fwd, 50, velocityUnits::pct);
  Drivetrain.driveFor(directionType::fwd, 18, distanceUnits::in);
  task::sleep(1000);
  Intake.stop();

  Drivetrain.turnFor(-50, rotationUnits::deg);
  Drivetrain.driveFor(fwd, 6, distanceUnits::in);
  Intake.spinFor(fwd, 1, timeUnits::sec, 50, velocityUnits::pct);

  setLiftHeight(liftLow);
  task::sleep(1000);

  Drivetrain.driveFor(fwd, 5, distanceUnits::in, 20, velocityUnits::pct);

  Intake.spinFor(directionType::rev, 1, timeUnits::sec, 30, velocityUnits::pct);

  setLiftHeight(liftBottom);
  Drivetrain.driveFor(directionType::rev, 11, distanceUnits::in);
  Drivetrain.turnFor(50, rotationUnits::deg);

  Intake.spin(fwd, 50, velocityUnits::pct);
  Drivetrain.driveFor(directionType::fwd, 8, distanceUnits::in);
  task::sleep(1000);
  Intake.stop();

  Drivetrain.driveFor(directionType::rev, 11, distanceUnits::in);
  Drivetrain.turnFor(66, rotationUnits::deg);
  Drivetrain.driveFor(directionType::fwd, 20, distanceUnits::in);

  Intake.spinFor(directionType::fwd, 1, timeUnits::sec, 50, velocityUnits::pct);

  setLiftHeight(liftMedium);
  task::sleep(1500);

  Drivetrain.driveFor(directionType::fwd, 6, distanceUnits::in, 20, velocityUnits::pct);
  Intake.spinFor(directionType::rev, 1, timeUnits::sec, 30, velocityUnits::pct);

  Drivetrain.driveFor(directionType::rev, 6, distanceUnits::in, 20, velocityUnits::pct);
  setLiftHeight(liftBottom);

  Drivetrain.driveFor(directionType::rev, 10, distanceUnits::in);
  Drivetrain.turnFor(-64, rotationUnits::deg);
  Drivetrain.driveFor(directionType::fwd, 20, distanceUnits::in);

  Intake.spin(fwd, 50, velocityUnits::pct);
  Drivetrain.driveFor(directionType::fwd, 8, distanceUnits::in);
  task::sleep(1000);
  Intake.stop();

  setLiftHeight(liftLow);
  task::sleep(1000);

  Drivetrain.driveFor(directionType::fwd, 4, distanceUnits::in, 20, velocityUnits::pct);
  Intake.spinFor(directionType::rev, 1, timeUnits::sec, 30, velocityUnits::pct);

  Drivetrain.driveFor(directionType::rev, 4, distanceUnits::in, 20, velocityUnits::pct);
  setLiftHeight(liftBottom);

  Drivetrain.turnFor(-66, rotationUnits::deg);
  Drivetrain.driveFor(directionType::fwd, 18, distanceUnits::in);
  Drivetrain.turnFor(75, rotationUnits::deg);
  Drivetrain.driveFor(directionType::fwd, 30, distanceUnits::in);

  Drivetrain.turnFor(-5, rotationUnits::deg);

  Intake.spin(fwd, 50, velocityUnits::pct);
  Drivetrain.driveFor(directionType::fwd, 10, distanceUnits::in, 20, velocityUnits::pct);
  task::sleep(1000);
  Intake.stop();

  Drivetrain.turnFor(-50, rotationUnits::deg);

  Drivetrain.driveFor(directionType::fwd, 8, distanceUnits::in, 20, velocityUnits::pct);
  Intake.spinFor(directionType::fwd, 0.5, timeUnits::sec, 30, velocityUnits::pct);

  setLiftHeight(liftLow);
  task::sleep(1000);

  Drivetrain.driveFor(directionType::fwd, 6, distanceUnits::in, 20, velocityUnits::pct);
  Intake.spinFor(directionType::rev, 1, timeUnits::sec, 50, velocityUnits::pct);

  Drivetrain.driveFor(directionType::rev, 6, distanceUnits::in, 20, velocityUnits::pct);
  setLiftHeight(liftBottom);

  Drivetrain.driveFor(directionType::rev, 6, distanceUnits::in, 20, velocityUnits::pct);
  Drivetrain.turnFor(50, rotationUnits::deg);

  Intake.spin(fwd, 50, velocityUnits::pct);
  Drivetrain.driveFor(directionType::fwd, 6, distanceUnits::in, 20, velocityUnits::pct);
  Drivetrain.turnFor(-25, rotationUnits::deg);
  Drivetrain.driveFor(directionType::fwd, 24, distanceUnits::in, 50, velocityUnits::pct);
  Intake.stop();
  Intake.spinFor(directionType::rev, 1, timeUnits::sec, 50, velocityUnits::pct);
}

//multiplier is -1 for red auton, 1 for blue auton
void matchAuton(float multiplier){
  Drivetrain.driveFor(directionType::fwd, 14, distanceUnits::in);
  Drivetrain.driveFor(directionType::rev, 7, distanceUnits::in);

  Drivetrain.turnFor(multiplier * 66, rotationUnits::deg);

  Intake.spin(fwd, 50, velocityUnits::pct);
  Drivetrain.driveFor(directionType::fwd, 18, distanceUnits::in);
  task::sleep(1000);
  Intake.stop();

  Drivetrain.turnFor(multiplier * -50, rotationUnits::deg);
  Drivetrain.driveFor(fwd, 6, distanceUnits::in);
  Intake.spinFor(fwd, 1, timeUnits::sec, 50, velocityUnits::pct);

  setLiftHeight(liftLow);
  task::sleep(1000);

  Drivetrain.driveFor(fwd, 5, distanceUnits::in, 20, velocityUnits::pct);

  Intake.spinFor(directionType::rev, 1, timeUnits::sec, 30, velocityUnits::pct);

  setLiftHeight(liftBottom);
  Drivetrain.driveFor(directionType::rev, 11, distanceUnits::in);
  Drivetrain.turnFor(multiplier * 50, rotationUnits::deg);

  Intake.spin(fwd, 50, velocityUnits::pct);
  Drivetrain.driveFor(directionType::fwd, 8, distanceUnits::in);
  task::sleep(1000);
  Intake.stop();

  Drivetrain.driveFor(directionType::rev, 18, distanceUnits::in);
  Drivetrain.turnFor(multiplier * -87, rotationUnits::deg);

  Intake.spinFor(directionType::fwd, 1, timeUnits::sec, 30, velocityUnits::pct);

  Lift.rotateTo(170, rotationUnits::deg);
  Drivetrain.driveFor(fwd, 8, distanceUnits::in, 20, velocityUnits::pct);

  Intake.spinFor(directionType::rev, 1, timeUnits::sec, 30, velocityUnits::pct);

  Drivetrain.driveFor(directionType::rev, 7, distanceUnits::in, 20, velocityUnits::pct);
  setLiftHeight(liftBottom);
}

//empty auton for now
void auton(){
//run programming skills
#ifdef PROG_SKILLS
  progSkills();
  return;
#endif

//otherwise, run match auton
#ifdef BLUE
  matchAuton(1.0);
#else
  matchAuton(-1.0);
#endif
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
