#ifndef PAINTING_CONFIG_H
#define PAINTING_CONFIG_H

//* ************************************************************************
//* ************************ PAINTING CONFIGURATION ***************************
//* ************************************************************************

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🎨 PAINT ROTATION MOTOR SETTINGS                                     ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
// Paint rotation motor: 400 steps/rev at motor, 3:1 gear reduction
// 1 full 360-degree output rotation = 3 motor revs = 1200 motor steps
#define PAINT_ROTATION_MOTOR_SPEED 1000                // Paint rotation motor speed for normal operation
#define PAINT_ROTATION_MOTOR_ACCEL 1000                // Paint rotation motor acceleration for normal operation
#define PAINT_ROTATION_MOTOR_STEPS_PER_CLICK 2000      // Paint rotation motor steps per click in web dashboard
#define PAINT_ROTATION_MOTOR_STEPS_PER_REV_OUTPUT 1200  // Motor steps per full output revolution (400 steps/rev * 3:1 gear = 1200)
#define PAINT_ROTATION_STEPS_90_DEG 300                 // Motor steps for 90 degree output rotation (1200/4 = 300)

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🎯 SERVO CONFIGURATION                                               ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define SERVO_HOME_ANGLE 130.0         // Servo home angle (idle position before painting)
#define SERVO_PAINTING_ANGLE 210.0     // Servo angle during painting operations

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔧 SERVO MOVEMENT PARAMETERS                                         ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define SERVO_UPDATE_INTERVAL_MS 10                    // Update interval for smooth servo movement (milliseconds)
#define SERVO_TARGET_REACHED_THRESHOLD_DEG 0.1         // Threshold for considering servo at target angle (degrees)
#define SERVO_FAR_FROM_TARGET_THRESHOLD_DEG 0.5       // Threshold for considering servo far from target (degrees)
#define SERVO_MIN_MOVEMENT_DEG 0.2                     // Minimum movement per update to ensure progress (degrees)
#define SERVO_MAX_STEP_SIZE_DEG 1.25                    // Maximum movement per update to prevent jumps (degrees)

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 📍 STEP 0-1: GANTRY MOVEMENT TO WAITING POSITION                     ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define STEP1_WAITING_POSITION_X_OFFSET_INCHES 5.0     // X-axis offset from position 3 to waiting position (inches)
#define STEP1_WAITING_POSITION_Y_OFFSET_INCHES 0.5     // Y-axis offset from position 3 to waiting position (inches)
#define STEP2_WAITING_POSITION_DELAY_MS 250            // Delay after gantry reaches waiting position before turning on paint gun (milliseconds)

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🎨 STEP 2-3: SERVO TO PAINTING ANGLE AND INITIAL SETUP               ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define STEP3_SERVO_FAST_SPEED 500.0                   // Temporary servo speed for moving to painting angle (degrees per second)
#define STEP4_INITIAL_ROTATION_DELAY_MS 200            // Delay before starting initial 180-degree rotation after servo reaches painting angle (milliseconds)
#define STEP4_INITIAL_ROTATION_REV 0.5                  // Initial rotation amount (180 degrees = 0.5 revolutions)

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔄 STEP 5-6: ROTATE TO LEFT SIDE                                    ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define STEP5_LEFT_ROTATION_REV -0.25                  // Rotation to left side (90 degrees CCW = -0.25 revolutions)
#define STEP6_LEFT_SIDE_WAIT_MS 300                    // Wait time on left side before continuing (milliseconds)

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔄 STEP 7-8: ROTATE TO BACK LEFT AND SERVO MOVEMENT                 ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define STEP7_BACK_LEFT_ROTATION_REV 0.125             // Rotation to back left (45 degrees CW = 0.125 revolutions)
#define STEP8_BACK_LEFT_WAIT_MS 300                    // Wait time on back left before servo movement (milliseconds)
#define STEP8_SERVO_BACK_ANGLE_DEG 160.0               // Servo angle for back left/right positions (degrees)

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔄 STEP 9-10: ROTATE TO BACK                                        ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define STEP9_BACK_ROTATION_REV 0.125                  // Rotation to back (45 degrees CW = 0.125 revolutions)
#define STEP10_BACK_SIDE_WAIT_MS 300                   // Wait time on back side before continuing (milliseconds)

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔄 STEP 11-12: ROTATE TO BACK RIGHT AND SERVO MOVEMENT              ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define STEP11_BACK_RIGHT_ROTATION_REV 0.125           // Rotation to back right (45 degrees CW = 0.125 revolutions)
#define STEP12_BACK_RIGHT_WAIT_MS 300                  // Wait time on back right before servo movement (milliseconds)
#define STEP12_SERVO_BACK_ANGLE_DEG 160.0             // Servo angle for back right position (degrees)

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔄 STEP 13-14: ROTATE TO RIGHT SIDE                                 ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define STEP13_RIGHT_ROTATION_REV 0.125                // Rotation to right side (45 degrees CW = 0.125 revolutions)
#define STEP14_RIGHT_SIDE_WAIT_MS 300                  // Wait time on right side before continuing (milliseconds)

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔄 STEP 15: FINAL ROTATION                                          ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define STEP15_FINAL_ROTATION_REV 2.25                 // Final rotation amount (810 degrees = 2.25 revolutions)

#endif // PAINTING_CONFIG_H
