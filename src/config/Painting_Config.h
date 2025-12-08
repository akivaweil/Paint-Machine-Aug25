#ifndef PAINTING_CONFIG_H
#define PAINTING_CONFIG_H

//* ************************************************************************
//* ************************ PAINTING CONFIGURATION ***************************
//* ************************************************************************

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🎨 PAINT ROTATION MOTOR SETTINGS                                     ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
// Paint rotation motor: 12800 steps/rev with 5:15 gear ratio (3:1 reduction)
// 1 full 360-degree output rotation = 3 motor revs = 38400 steps
#define PAINT_ROTATION_MOTOR_SPEED 1000                // Paint rotation motor speed for normal operation
#define PAINT_ROTATION_MOTOR_ACCEL 1000                // Paint rotation motor acceleration for normal operation
#define PAINT_ROTATION_MOTOR_STEPS_PER_CLICK 2000      // Paint rotation motor steps per click in web dashboard
#define PAINT_ROTATION_MOTOR_STEPS_PER_REV_OUTPUT 9600 // Steps per full output revolution

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🎯 SERVO CONFIGURATION                                               ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define SERVO_HOME_ANGLE 130.0         // Servo home angle (idle position before painting)
// Servo positions based on paint motor rotation count
#define SERVO_POS_1_ANGLE 220.0        // Servo angle at position 1 (0 rotations / starting position)
#define SERVO_POS_1_ROTATIONS 0.0      // Rotation count to trigger position 1
#define SERVO_POS_2_ANGLE 200.0        // Servo angle at position 2
#define SERVO_POS_2_ROTATIONS 1.0      // Rotation count to trigger position 2
#define SERVO_POS_3_ANGLE 150.0        // Servo angle at position 3
#define SERVO_POS_3_ROTATIONS 1.75      // Rotation count to trigger position 3
#define SERVO_POS_4_ANGLE 130.0        // Servo angle at position 4
#define SERVO_POS_4_ROTATIONS 2.5      // Rotation count to trigger position 4

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🎨 PAINTING STATE CONFIGURATION                                      ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define SERVO_PAINT_MOVE_SPEED 40.0    // Servo speed in degrees per second for painting movements
#define SERVO_PAINT_START_ANGLE 210.0  // Starting servo angle for painting sequence
#define SERVO_PAINT_END_ANGLE 180.0    // Ending servo angle for painting sequence
#define WAITING_POSITION_DELAY_MS 250  // Delay in milliseconds after gantry reaches waiting position before turning on paint gun
#define DWELL_LEFT_MS 500              // Dwell time in milliseconds on left side
#define DWELL_BACK_MS 1000             // Dwell time in milliseconds on back side
#define DWELL_RIGHT_MS 500             // Dwell time in milliseconds on right side

#endif // PAINTING_CONFIG_H

