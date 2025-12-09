#ifndef PAINTING_CONFIG_H
#define PAINTING_CONFIG_H

//* ************************************************************************
//* ************************ PAINTING CONFIGURATION ***************************
//* ************************************************************************

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🎨 PAINT ROTATION MOTOR SETTINGS                                     ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
// Paint rotation motor: 400 steps per revolution
#define PAINT_ROTATION_MOTOR_SPEED 1000                // Paint rotation motor speed for normal operation
#define PAINT_ROTATION_MOTOR_ACCEL 400                 // Paint rotation motor acceleration for normal operation
#define PAINT_ROTATION_MOTOR_STEPS_PER_REV_OUTPUT 400  // Steps per full output revolution

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🎯 SERVO CONFIGURATION                                               ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define SERVO_HOME_ANGLE 130.0         // Servo home angle (idle position before painting)
// Servo positions based on paint motor rotation count
#define SERVO_POS_1_ANGLE 220.0        // Servo angle at position 1 (0 rotations / starting position)
#define SERVO_POS_1_ROTATIONS 0.0      // Rotation count to trigger position 1
#define SERVO_POS_2_ANGLE 200.0        // Servo angle at position 2
#define SERVO_POS_2_ROTATIONS 1.0      // Rotation count to trigger position 2
#define SERVO_POS_3_ANGLE 180.0        // Servo angle at position 3
#define SERVO_POS_3_ROTATIONS 1.75      // Rotation count to trigger position 3
#define SERVO_POS_4_ANGLE 150.0        // Servo angle at position 4
#define SERVO_POS_4_ROTATIONS 2.5      // Rotation count to trigger position 4

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🎨 PAINTING STATE CONFIGURATION                                      ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define PAINT_GUN_DELAY_MS 500         // Delay in milliseconds before paint gun turns on after servo starts moving
#define TOTAL_PAINT_REVOLUTIONS 3.0    // Total number of paint motor revolutions for the painting cycle
#define PAINT_GUN_OFF_REVOLUTIONS 0.5  // Number of paint motor revolutions remaining before paint gun turns off
#define PAINT_MOTOR_PAUSE_REVOLUTIONS 2.0  // Number of revolutions before motor pauses
#define PAINT_MOTOR_PAUSE_DURATION_MS 1000  // Duration of pause in milliseconds after PAINT_MOTOR_PAUSE_REVOLUTIONS

#endif // PAINTING_CONFIG_H

