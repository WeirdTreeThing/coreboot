/* SPDX-License-Identifier: GPL-2.0-only OR MIT */

#ifndef SOC_MEDIATEK_CHIP_H
#define SOC_MEDIATEK_CHIP_H

#include <soc/pcie_common.h>

struct soc_mediatek_mt8196_config {
	struct mtk_pcie_config pcie_config;
};

typedef struct soc_mediatek_mt8196_config mtk_soc_config_t;

#endif /* SOC_MEDIATEK_CHIP_H */
