/*
 * (C) Copyright 2012
 *     tyle@allwinnertech.com
 *
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program;
 *
 */
#ifndef _pwm_pub_h_
#define _pwm_pub_h_

//int	pwm_init		(int pwm_id, int div, int invert);
//int	pwm_config		(int pwm_id, int duty_ns, int period_ns);
//int	pwm_enable		(int pwm_id);
//void	pwm_disable		(int pwm_id);

enum pwm_polarity {
	PWM_POLARITY_NORMAL,
	PWM_POLARITY_INVERSED,
};

int sunxi_pwm_set_polarity(int pwm, enum pwm_polarity polarity);
int sunxi_pwm_config      (int pwm, int duty_ns, int period_ns);
int sunxi_pwm_enable      (int pwm);
void sunxi_pwm_disable    (int pwm);
void sunxi_pwm_init       (void);

#endif /* _pwm_pub_h_ */
