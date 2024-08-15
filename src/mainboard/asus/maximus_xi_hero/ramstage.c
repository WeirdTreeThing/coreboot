/* SPDX-License-Identifier: GPL-2.0-only */

#include <soc/ramstage.h>
#include "include/gpio.h"

static void init_mainboard(void *chip_info)
{
        gpio_configure_pads(gpio_table, ARRAY_SIZE(gpio_table));
}

struct chip_operations mainboard_ops = {
        .init = init_mainboard,
};

void mainboard_silicon_init_params(FSPS_UPD *params)
{

	// Disable DMI ASPM
	//params->DmiAspm = 0;
	//params->PchDmiAspmCtrl = 0;
	//params->PchLegacyIoLowLatency = 1;

	// Disable Intel AMT
	//params->AmtEnabled = 0;
	//params->AmtKvmEnabled = 0;
	//params->AmtSolEnabled = 0;
}
