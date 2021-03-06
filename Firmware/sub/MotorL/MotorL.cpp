/**
 * @file MotorL.cpp
 * @author Dan Oates (WPI Class of 2020)
 */
#include <MotorL.h>
#include <MotorConfig.h>
#include <Controller.h>
#include <Imu.h>
#include <HBridge.h>
#include <QuadEncoder.h>
#include <LTIFilter.h>
using Controller::f_ctrl;
using MotorConfig::Vb;
using MotorConfig::enc_cpr;

/**
 * Namespace Definitions
 */
namespace MotorL
{
	// Pin Definitions
	const uint8_t pin_enable = 8;	// H-bridge enable
	const uint8_t pin_pwm = 9;		// H-bridge PWM
	const uint8_t pin_fwd = 6;		// H-bridge forward enable
	const uint8_t pin_rev = 7;		// H-bridge reverse enable
	const uint8_t pin_enc_a = 2;	// Encoder channel A
	const uint8_t pin_enc_b = 3;	// Encoder channel B

	// Hardware Interfaces
	PwmOut out_pwm(pin_pwm);
	DigitalOut out_fwd(pin_fwd);
	DigitalOut out_rev(pin_rev);
	HBridge motor(&out_pwm, &out_fwd, &out_rev, Vb);
	DigitalIn in_enc_a(pin_enc_a);
	DigitalIn in_enc_b(pin_enc_b);
	QuadEncoder encoder(&in_enc_a, &in_enc_b, enc_cpr);

	// Digital Filters
	LTIFilter angle_diff = LTIFilter::make_dif(f_ctrl);

	// State Variables
	float angle;		// Encoder angle [rad]
	float velocity;		// Angular velocity [rad/s]

	// Init Flag
	bool init_complete = false;

	// Private Functions
	void isr_A();
	void isr_B();
}

/**
 * @brief Initializes drive motor
 * 
 * Enables motor and encoder, and sets up encoder ISRs.
 */
void MotorL::init()
{
	if (!init_complete)
	{
		// Enable motor driver
		pinMode(pin_enable, OUTPUT);
		digitalWrite(pin_enable, HIGH);

		// Init encoder interrupts
		attachInterrupt(digitalPinToInterrupt(pin_enc_a), isr_A, CHANGE);
		attachInterrupt(digitalPinToInterrupt(pin_enc_b), isr_B, CHANGE);

		// Set init flag
		init_complete = true;
	}
}

/**
 * @brief Updates motor state estimates
 */
void MotorL::update()
{
	angle = MotorConfig::direction * encoder.get_angle() - Imu::get_pitch();
	velocity = angle_diff.update(angle);
}

/**
 * @brief Sends given voltage command to motor
 */
void MotorL::set_voltage(float v_cmd)
{
	motor.set_voltage(MotorConfig::direction * v_cmd);
}

/**
 * @brief Returns encoder angle estimate
 */
float MotorL::get_angle()
{
	return angle;
}

/**
 * @brief Returns encoder velocity estimate
 */
float MotorL::get_velocity()
{
	return velocity;
}

/**
 * @brief Motor encoder A ISR
 */
void MotorL::isr_A()
{
	encoder.interrupt_A();
}

/**
 * @brief Motor encoder B ISR
 */
void MotorL::isr_B()
{
	encoder.interrupt_B();
}