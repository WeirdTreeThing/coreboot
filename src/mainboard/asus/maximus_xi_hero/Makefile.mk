## SPDX-License-Identifier: GPL-2.0-only

romstage-y += romstage.c

ramstage-y += ramstage.c
# ramstage-$(CONFIG_SOC_INTEL_COMMON_BLOCK_HDA_VERB) += variants/$(VARIANT_DIR)/hda_verb.c
