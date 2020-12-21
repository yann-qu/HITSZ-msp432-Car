// Lab13_Timersmain.c
// Runs on MSP432
// Student version to Timers lab
// Daniel and Jonathan Valvano
// July 3, 2017
// PWM output to motor
// Second Periodic interrupt

/* This example accompanies the books
   "Embedded Systems: Introduction to the MSP432 Microcontroller",
       ISBN: 978-1512185676, Jonathan Valvano, copyright (c) 2017
   "Embedded Systems: Real-Time Interfacing to the MSP432 Microcontroller",
       ISBN: 978-1514676585, Jonathan Valvano, copyright (c) 2017
   "Embedded Systems: Real-Time Operating Systems for ARM Cortex-M
Microcontrollers", ISBN: 978-1466468863, , Jonathan Valvano, copyright (c) 2017
 For more information about my classes, my research, and my books, see
 http://users.ece.utexas.edu/~valvano/

Simplified BSD License (FreeBSD License)
Copyright (c) 2017, Jonathan Valvano, All rights reserved.

Redistribution and use in source and binary forms, with or without modification,
are permitted provided that the following conditions are met:

1. Redistributions of source code must retain the above copyright notice,
   this list of conditions and the following disclaimer.
2. Redistributions in binary form must reproduce the above copyright notice,
   this list of conditions and the following disclaimer in the documentation
   and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

The views and conclusions contained in the software and documentation are
those of the authors and should not be interpreted as representing official
policies, either expressed or implied, of the FreeBSD Project.
*/
// Negative logic bump sensors
// P8.7 Bump5
// P8.6 Bump4
// P8.5 Bump3
// P8.4 Bump2
// P8.3 Bump1
// P8.0 Bump0

// Sever VCCMD=VREG jumper on Motor Driver and Power Distribution Board and
// connect VCCMD to 3.3V.
//   This makes P3.7 and P3.6 low power disables for motor drivers.  0 to
//   sleep/stop.
// Sever nSLPL=nSLPR jumper.
//   This separates P3.7 and P3.6 allowing for independent control
// Left motor direction connected to P1.7 (J2.14)
// Left motor PWM connected to P2.7/TA0CCP4 (J4.40)
// Left motor enable connected to P3.7 (J4.31)
// Right motor direction connected to P1.6 (J2.15)
// Right motor PWM connected to P2.6/TA0CCP3 (J4.39)
// Right motor enable connected to P3.6 (J2.11)

#include <stdint.h>
#include "msp.h"
#include "..\inc\Bump.h"
#include "..\inc\Clock.h"
//#include "..\inc\SysTick.h"
#include "..\inc\CortexM.h"
#include "..\inc\LaunchPad.h"
#include "..\inc\Motor.h"
#include "..\inc\PWM.h"
#include "..\inc\Reflectance.h"

#define VELOCITY0 3800

// 延时并停止电机输出
void TimedPause(uint32_t time) {
    Clock_Delay1ms(time); // run for a while and stop
    Motor_Stop(); // Stop the motors, power down the drivers, and set the PWM
                      // speed control to 0% duty cycle.
                      //* 最好不要删，可以防止拐弯过陡
    // while(LaunchPad_Input()==0);  // wait for touch
    // while(LaunchPad_Input());     // wait for release
}


// reflact
uint8_t Data;
int32_t position;

void reflactance(void) {

    Data     = Reflectance_Read(1000);
    position = Reflectance_Position(Data);

    //* 在正中
    if (position == 0) {
        Motor_Forward(VELOCITY0, VELOCITY0);
    }
    //*
    else if (position >= 237) {
        Motor_Stop();
        TimedPause(100);
        Motor_Forward(1000, 6000);
        TimedPause(100);
    }
    //*
    else if (position <= -237) {
        Motor_Stop();
        TimedPause(100);
        Motor_Forward(6000, 1000);
        TimedPause(100);
    }
    //*
    else if ((position > 0) && (position < 237)) {
        // Motor_Forward((3000 - 8 * position), 3700);  // 左转
        Motor_Forward((3000 - 8 * position), 4000);  // 左转
    }
    //*
    else if ((position < 0) && (position > -237)) {
        // Motor_Forward(3700, (3000 + 8 * position));  // 右转
        Motor_Forward(4000, (3000 + 8 * position));  // 右转
    }
    //* T型路口
    if (Data == 0xFF) {
        Motor_Forward(VELOCITY0, VELOCITY0);
        TimedPause(200);
        Motor_Right(4000, 3900);
        TimedPause(350);
    }
    //* 直角右转
    if ((Data == 0xF8) || (Data == 0xF0) || (Data == 0xE0)) {
        Data = Reflectance_Read(1000);
        TimedPause(5);
        if ((Data == 0xF8) || (Data == 0xF0) || (Data == 0xE0)) {
            Data = Reflectance_Read(1000);
            TimedPause(5);
            if ((Data == 0xF8) || (Data == 0xF0) || (Data == 0xE0)) {
                Motor_Forward(VELOCITY0, VELOCITY0);
                TimedPause(150);
                Motor_Right(4025, 4025);
                TimedPause(400);
            }
        }
    }
    //* 直角左转
    if ((Data == 0x1F) || (Data == 0x0F) || (Data == 0x07)) {
        Data = Reflectance_Read(1000);
        TimedPause(5);
        if ((Data == 0x1F) || (Data == 0x0F) || (Data == 0x07)) {
            TimedPause(5);
            Data = Reflectance_Read(1000);
            if ((Data == 0x1F) || (Data == 0x0F) || (Data == 0x07)) {
                Motor_Forward(VELOCITY0, VELOCITY0);
                TimedPause(150);
                Data = Reflectance_Read(1000);
                if (Data == 0x00) {
                    Motor_Left(4025, 4025);
                    TimedPause(400);
                } else
                    Motor_Forward(VELOCITY0, VELOCITY0);
            }
        }
    }
    //* 未识别到黑线
    else if (Data == 0x00) {
        Motor_Forward(VELOCITY0 * 0.8, VELOCITY0 * 0.8);
    }
}


// 停止运动
void endstop(void) {
    Data = Reflectance_Read(1000);
    if (Data == 0xDB) { // 1101 1011
        Motor_Stop();
        while (LaunchPad_Input() == 0)
            ; // wait for touch
        while (LaunchPad_Input())
            ;
    }
}


// bump and driver
void bumprun(void) {

    uint8_t numflag = 0;
    numflag         = Bump_Read();
    if (numflag == 1) {
        TimedPause(500);
        Motor_Left(4000, 6000);
        TimedPause(500); // SysTick_Wait10ms(30);
    } else if (numflag == 3) {
        TimedPause(500);
        Motor_Right(4000, 6000);
        TimedPause(500); // SysTick_Wait10ms(30);
    } else if (numflag == 2) {
        TimedPause(500);
        Motor_Backward(4000, 4000);
        TimedPause(500); // SysTick_Wait10ms(30);
        Motor_Right(4000, 6000);
        TimedPause(500); // SysTick_Wait10ms(30);
    } else if (numflag == 0)
        Motor_Forward(VELOCITY0, VELOCITY0);
}

// 检查是否发生碰撞
void bumprun1(void) {
    uint8_t numflag1 = 0;
    numflag1         = Bump_Read1();
    if ((numflag1 & 0xED) != 0xED) {
        Motor_Backward(4000, 4000); // 后退
        TimedPause(300);            // SysTick_Wait10ms(30);
        Motor_Right(4000, 3900);    //转弯
        TimedPause(400);            // SysTick_Wait10ms(30);
    }
}


int main(void) {
    Clock_Init48MHz(); // Configure the system clock to run at the fastest
                       // and most accurate settings.
    LaunchPad_Init();  // init built-in switches and LEDs
    Bump_Init();       // bump switches
    Motor_Init();      // your function
    PWM_Init34(10000, 5000, 7000);
    Reflectance_Init();
    while (LaunchPad_Input() == 0)
        ; // wait for touch
    while (LaunchPad_Input())
        ; // wait for release

    // write a main program that uses PWM to move the robot
    // like Program13_1, but uses TimerA1 to periodically
    // check the bump switches, stopping the robot on a collision
    while (1) {
        reflactance();
        bumprun1();
        endstop();
    }
}
