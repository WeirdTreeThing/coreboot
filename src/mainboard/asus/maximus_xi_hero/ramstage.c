/* SPDX-License-Identifier: GPL-2.0-only */

#include <acpi/acpi.h>
#include <device/device.h>
#include <soc/ramstage.h>
#include "gpio.h"

void mainboard_fill_fadt(acpi_fadt_t *fadt)
{
	fadt->preferred_pm_profile = PM_DESKTOP;
	fadt->iapc_boot_arch |= ACPI_FADT_LEGACY_DEVICES | ACPI_FADT_8042;
}

static void init_mainboard(void *chip_info)
{
        gpio_configure_pads(gpio_table, ARRAY_SIZE(gpio_table));
}

struct chip_operations mainboard_ops = {
        .init = init_mainboard,
};

void mainboard_silicon_init_params(FSPS_UPD *params)
{
	params->DmiAspm = 0;
	params->PchDmiAspmCtrl = 0;
	params->PchLegacyIoLowLatency = 1;
}
