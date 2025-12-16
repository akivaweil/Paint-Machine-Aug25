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
#define SERVO_PAINTING_ANGLE 220.0     // Servo angle during painting operations

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔧 SERVO MOVEMENT PARAMETERS                                         ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define SERVO_UPDATE_INTERVAL_MS 10                    // Update interval for smooth servo movement (milliseconds)
#define SERVO_TARGET_REACHED_THRESHOLD_DEG 0.1         // Threshold for considering servo at target angle (degrees)
#define SERVO_FAR_FROM_TARGET_THRESHOLD_DEG 0.5       // Threshold for considering servo far from target (degrees)
#define SERVO_MIN_MOVEMENT_DEG 0.2                     // Minimum movement per update to ensure progress (degrees)
#define SERVO_MAX_STEP_SIZE_DEG 1.25                    // Maximum movement per update to prevent jumps (degrees)

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 📍 STEP 0-1: MOVEMENT TO WAITING POSITION                           ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define STEP1_WAITING_POSITION_X_OFFSET_INCHES 7.0     // X-axis offset from position 3 to waiting position (inches)
#define STEP1_WAITING_POSITION_Y_OFFSET_INCHES 0.5     // Y-axis offset from position 3 to waiting position (inches)
#define STEP2_WAITING_POSITION_DELAY_MS 250            // Delay after motors reach waiting position before turning on paint gun (milliseconds)

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🎨 STEP 2-3: SERVO TO PAINTING ANGLE AND INITIAL SETUP               ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define STEP3_SERVO_FAST_SPEED 500.0                   // Temporary servo speed for moving to painting angle (degrees per second)
#define STEP4_INITIAL_ROTATION_DELAY_MS 200            // Delay before starting initial 180-degree rotation after servo reaches painting angle (milliseconds)
#define STEP4_INITIAL_ROTATION_REV 0.5                  // Initial rotation amount (180 degrees = 0.5 revolutions)

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔄 STEP 5-6: ROTATE TO LEFT SIDE                                    ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define STEP5_LEFT_ROTATION_REV 1.0                    // Rotation to left side (full rotation CW = 1.0 revolutions instead of 90 degrees CCW)
#define STEP6_LEFT_SIDE_WAIT_MS 100                    // Wait time on left side before continuing (milliseconds)

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔄 STEP 7-8: ROTATE TO BACK LEFT AND SERVO MOVEMENT                 ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define STEP7_BACK_LEFT_ROTATION_REV 0.125             // Rotation to back left (45 degrees CW = 0.125 revolutions)
#define STEP8_BACK_LEFT_WAIT_MS 5                    // Wait time on back left before servo movement (milliseconds)
#define STEP8_SERVO_BACK_ANGLE_DEG 160.0               // Servo angle for back left/right positions (degrees)
#define STEP8_BACK_LEFT_SERVO_SPEED 70.0               // Servo movement speed for back left (degrees per second)
#define STEP8_BACK_LEFT_PAINT_DELAY_MS 300             // Delay at back angle position for painting (milliseconds)

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔄 STEP 9-10: ROTATE TO BACK AND SERVO MOVEMENT                     ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define STEP9_BACK_ROTATION_REV 0.125                  // Rotation to back (45 degrees CW = 0.125 revolutions)
#define STEP10_BACK_SIDE_WAIT_MS 5                    // Wait time on back side before servo movement (milliseconds)
#define STEP10_SERVO_BACK_ANGLE_DEG 160.0              // Servo angle for back position (degrees)
#define STEP10_BACK_SERVO_SPEED 70.0                   // Servo movement speed for back (degrees per second)
#define STEP10_BACK_PAINT_DELAY_MS 300                 // Delay at back angle position for painting (milliseconds)

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔄 STEP 11-12: ROTATE TO BACK RIGHT AND SERVO MOVEMENT              ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define STEP11_BACK_RIGHT_ROTATION_REV 0.125           // Rotation to back right (45 degrees CW = 0.125 revolutions)
#define STEP12_BACK_RIGHT_WAIT_MS 5                  // Wait time on back right before servo movement (milliseconds)
#define STEP12_SERVO_BACK_ANGLE_DEG 160.0             // Servo angle for back right position (degrees)
#define STEP12_BACK_RIGHT_SERVO_SPEED 70.0             // Servo movement speed for back right (degrees per second)
#define STEP12_BACK_RIGHT_PAINT_DELAY_MS 300           // Delay at back angle position for painting (milliseconds)

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔄 STEP 13-14: ROTATE TO RIGHT SIDE AND SERVO MOVEMENT              ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define STEP13_RIGHT_ROTATION_REV 0.125                // Rotation to right side (45 degrees CW = 0.125 revolutions)
#define STEP14_RIGHT_SIDE_WAIT_MS 5                    // Wait time on right side before servo movement (milliseconds)
#define STEP14_SERVO_RIGHT_ANGLE_DEG 180.0             // Servo angle for right side position (degrees)
#define STEP14_RIGHT_SERVO_SPEED 70.0                  // Servo movement speed for right side (degrees per second)
#define STEP14_RIGHT_PAINT_DELAY_MS 500                 // Delay at right angle position for painting (milliseconds)

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔄 STEP 15: PAINT GUN REFILL DELAY                                  ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define STEP15_PAINT_GUN_OFF_DELAY_MS 100              // Time to turn off paint gun for refill (milliseconds)

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔄 STEP 16: FINAL ROTATION                                          ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define STEP15_FINAL_ROTATION_REV 2.25                 // Final rotation amount (810 degrees = 2.25 revolutions)
#define STEP15_FIRST_REV_SERVO_ANGLE_DEG 230.0         // Servo angle for first 1 revolution (degrees)
//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔄 STEP 16: SPIN SERVO ANGLE                                        ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define STEP16_SPIN_SERVO_ANGLE_DEG 200.0              // Servo angle for spin operation (degrees)

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔄 STEP 18: PAINT GUN REFILL DELAY BEFORE FINAL FRONT PASS          ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define STEP18_PAINT_GUN_OFF_DELAY_MS 5              // Time to turn off paint gun before final front pass (milliseconds)

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🔄 STEP 17: FINAL FACE COAT                                         ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define STEP17_INITIAL_SERVO_ANGLE_DEG 230.0           // Initial servo angle for final face coat (degrees)
#define STEP17_INITIAL_ANGLE_WAIT_MS 300               // Wait time at initial angle before moving to home (milliseconds)
#define STEP17_SERVO_SPEED 100.0                       // Servo movement speed for final face coat (degrees per second)

#endif // PAINTING_CONFIG_H
