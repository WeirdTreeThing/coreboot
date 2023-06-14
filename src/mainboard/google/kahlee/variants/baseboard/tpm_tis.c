/* SPDX-License-Identifier: GPL-2.0-only */

#include <drivers/tpm/cr50.h>
#include <gpio.h>
#include <variant/gpio.h>

int cr50_plat_irq_status(void)
{
	return gpio_interrupt_status(GPIO_9);
}
