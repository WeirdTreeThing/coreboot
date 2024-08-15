/* SPDX-License-Identifier: GPL-2.0-only */

#include <soc/cnl_memcfg_init.h>
#include <soc/romstage.h>

static const struct cnl_mb_cfg memcfg = {
        /* Access memory info through SMBUS. */
	.spd[0] = {.read_type = NOT_EXISTING},
        .spd[1] = {
                .read_type = READ_SMBUS,
                .spd_spec = {.spd_smbus_address = 0xa2},
        },
	.spd[2] = {.read_type = NOT_EXISTING},
        .spd[3] = {
                .read_type = READ_SMBUS,
                .spd_spec = {.spd_smbus_address = 0xa6},
        },

        /*
         * The dqs_map arrays map the ddr4 pins to the SoC pins
         * for both channels.
         *
         * the index = pin number on ddr4 part
         * the value = pin number on SoC
         */
        .dqs_map[DDR_CH0] = {0, 1, 4, 5, 2, 3, 6, 7},
        .dqs_map[DDR_CH1] = {0, 1, 4, 5, 2, 3, 6, 7},

        /* Baseboard uses 121, 81 and 100 rcomp resistors */
        .rcomp_resistor = {121, 81, 100},

	/* Baseboard is an interleaved design */
        .dq_pins_interleaved = 1,

        /*
         * Baseboard Rcomp target values.
         */
        .rcomp_targets = {100, 40, 20, 20, 26},

        /* Disable Early Command Training */
        .ect = 0,

        /* Base on board design */
        .vref_ca_config = 2,
};

void mainboard_memory_init_params(FSPM_UPD *memupd)
{
	memupd->FspmConfig.UserBd = 1;			// Board type: Desktop
	//memupd->FspmConfig.DmiMaxLinkSpeed = 3;		// Force CPU <-> PCH link to run at Gen3 speed
	//memupd->FspmConfig.EnableAbove4GBMmio = 1;	// Self-explanatory
	memupd->FspmConfig.GttSize = 3;			// 8MB
	memupd->FspmConfig.ApertureSize = 3;		// 512MB
	memupd->FspmConfig.GtPsmiSupport = 0;
	memupd->FspmConfig.IgdDvmt50PreAlloc = 2;	// 64MB

	memupd->FspmConfig.SpdProfileSelected = 1;
	memupd->FspmConfig.RefClk = 0;
	memupd->FspmConfig.VddVoltage = 1350;

	cannonlake_memcfg_init(&memupd->FspmConfig, &memcfg);
}
