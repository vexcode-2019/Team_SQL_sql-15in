#ifndef DEVICES_H
#define DEVICES_H

// #pragma once

#include "vex.h"

competition Competition;

motor LFDrive(1, true);
// motor LFDrive;
motor LCDrive(2);
motor LRDrive(3, true);
motor_group LDrive(LFDrive, LCDrive, LRDrive);

motor RFDrive(6);
motor RCDrive(7, true);
motor RRDrive(8);
motor_group RDrive(RFDrive, RCDrive, RRDrive);

motor LLift(PORT11, ratio36_1, true);
motor RLift(PORT12, ratio36_1);
motor_group Lift(LLift, RLift);

motor LIntake(PORT16, true);
motor RIntake(PORT17);
motor_group Intake(LIntake, RIntake);

controller Controller;
controller::lcd ControllerScreen(nullptr);

brain::battery Battery;

drivetrain Drivetrain = drivetrain(LDrive, RDrive, 12.56, 11.625, 7, distanceUnits::in, 1);

extern limit liftSwitch;

#endif //DEVICES_H