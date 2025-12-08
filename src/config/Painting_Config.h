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
//║ 🎨 NEW PAINTING SEQUENCE SETTINGS                                    ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define SERVO_PAINT_START_ANGLE 210.0         // Servo target before first spray move
#define SERVO_PAINT_END_ANGLE 180.0           // Servo target during final spin back to front
#define SERVO_PAINT_MOVE_SPEED 40.0           // Servo speed (deg/sec) for painting moves

// Rotation moves are relative to current orientation (degrees)
#define ROTATE_TO_RIGHT_DEG -90.0             // Front -> right (-90° == 270°)
#define ROTATE_TO_BACK_DEG -90.0              // Right -> back
#define ROTATE_TO_LEFT_DEG -90.0              // Back -> left
#define FINAL_FULL_SPIN_DEG 360.0             // Extra spin before returning to front
#define FINAL_RETURN_TO_FRONT_DEG -90.0       // Left -> front (short path) combined with full spin

// Dwell times at each side (milliseconds)
#define DWELL_RIGHT_MS 500.0
#define DWELL_BACK_MS 1000.0
#define DWELL_LEFT_MS 500.0

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🎨 PAINTING STATE CONFIGURATION                                      ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define PAINT_GUN_DELAY_MS 500         // Delay in milliseconds before paint gun turns on after servo starts moving
#define TOTAL_PAINT_REVOLUTIONS 3.0    // Total number of paint motor revolutions for the painting cycle
#define PAINT_GUN_OFF_REVOLUTIONS 0.2  // Number of paint motor revolutions remaining before paint gun turns off

#endif // PAINTING_CONFIG_H

