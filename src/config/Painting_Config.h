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
#define WAITING_POSITION_DELAY_MS 250  // Delay in milliseconds before turning on paint gun after gantry reaches waiting position
#define SERVO_FAST_SPEED 40.0          // Temporary servo speed for step 4 (degrees per second)
#define LEFT_SIDE_WAIT_MS 500          // Wait time on left side in milliseconds
#define BACK_SIDE_WAIT_MS 1000         // Wait time on back side in milliseconds
#define RIGHT_SIDE_WAIT_MS 500         // Wait time on right side in milliseconds

#endif // PAINTING_CONFIG_H

