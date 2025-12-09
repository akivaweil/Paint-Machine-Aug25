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

//╔═══╗ ════════════════════════════════════════════════════════════════ ╔═══╗
//║ 🎨 PAINTING STATE CONFIGURATION                                      ║
//╚═══╝ ════════════════════════════════════════════════════════════════ ╚═══╝
#define WAITING_POSITION_DELAY_MS 250  // Delay in milliseconds before turning on paint gun after gantry reaches waiting position
#define SERVO_FAST_SPEED 40.0          // Temporary servo speed for step 4 (degrees per second)
#define LEFT_SIDE_WAIT_MS 500          // Wait time on left side in milliseconds
#define BACK_SIDE_WAIT_MS 1000         // Wait time on back side in milliseconds
#define RIGHT_SIDE_WAIT_MS 500         // Wait time on right side in milliseconds

#endif // PAINTING_CONFIG_H

