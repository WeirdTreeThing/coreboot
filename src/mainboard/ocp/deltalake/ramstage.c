/* SPDX-License-Identifier: GPL-2.0-only */

#include <assert.h>
#include <commonlib/bsd/helpers.h>
#include <console/console.h>
#include <cpu/cpu.h>
#include <cpu/x86/smm.h>
#include <cpxsp_dl_gpio.h>
#include <device/pciexp.h>
#include <drivers/ipmi/ipmi_ops.h>
#include <drivers/ocp/dmi/ocp_dmi.h>
#include <drivers/vpd/vpd.h>
#include <hob_iiouds.h>
#include <hob_memmap.h>
#include <security/intel/txt/txt.h>
#include <smbios.h>
#include <soc/chip_common.h>
#include <soc/ramstage.h>
#include <soc/smmrelocate.h>
#include <soc/soc_util.h>
#include <soc/util.h>
#include <stdio.h>
#include <string.h>
#include <types.h>
#include "ipmi.h"
#include "vpd.h"

#define SLOT_ID_LEN 2

extern struct fru_info_str fru_strings;
static char slot_id_str[SLOT_ID_LEN];

/* Override SMBIOS 2 Location In Chassis from BMC */
const char *smbios_mainboard_location_in_chassis(void)
{
	uint8_t slot_id = 0;

	if (ipmi_get_slot_id(&slot_id) != CB_SUCCESS) {
		printk(BIOS_ERR, "IPMI get slot_id failed\n");
		return "";
	}
	/* Sanity check, slot_id can only be 1~4 since there are 4 slots in YV3 */
	if (slot_id < PCIE_CONFIG_A || slot_id > PCIE_CONFIG_D) {
		printk(BIOS_ERR, "slot_id %d is not between 1~4\n", slot_id);
		return "";
	}
	snprintf(slot_id_str, SLOT_ID_LEN, "%d", slot_id);
	return slot_id_str;
}

/* Override SMBIOS type 2 Feature Flags */
u8 smbios_mainboard_feature_flags(void)
{
	return SMBIOS_FEATURE_FLAGS_HOSTING_BOARD | SMBIOS_FEATURE_FLAGS_REPLACEABLE;
}

/*
 * Override SMBIOS type 4 cpu voltage.
 * BIT7 will set to 1 after value return. If BIT7 is set to 1, the remaining seven
 * bits of this field are set to contain the processor's current voltage times 10.
 */
unsigned int smbios_cpu_get_voltage(void)
{
	/* This will return 1.6V which is expected value for Delta Lake
	   10h = (1.6 * 10) = 16 */
	return 0x10;
}

/* System Slot Socket, Stack, Type and Data bus width Information */
typedef struct {
	u8 stack;
	u8 slot_type;
	u8 slot_data_bus_width;
	u16 device_id;
	const char *slot_designator;
} slot_info;

/* Array index + 1 would be used as Slot ID */
slot_info slotinfo[] = {
	{CSTACK,  SlotTypePciExpressGen3X4, SlotDataBusWidth4X, 0xa218, "SSD1_M2_Data_Drive"},
	{PSTACK1, SlotTypePciExpressGen3X4, SlotDataBusWidth4X, 0x2032, "SSD0_M2_Boot_Drive"},
	{PSTACK1, SlotTypePciExpressGen3X4, SlotDataBusWidth4X, 0x2033, "BB_OCP_NIC"},
	{PSTACK2, SlotTypePciExpressGen3X16, SlotDataBusWidth16X, 0x2030, "1OU_OCP_NIC"},
	{PSTACK0, SlotTypePciExpressGen3X4, SlotDataBusWidth4X, 0x2030, "2OU_JD1_M2_0"},
	{PSTACK0, SlotTypePciExpressGen3X4, SlotDataBusWidth4X, 0x2032, "2OU_JD1_M2_1"},
	{PSTACK1, SlotTypePciExpressGen3X4, SlotDataBusWidth4X, 0x2031, "2OU_JD2_M2_2"},
	{PSTACK1, SlotTypePciExpressGen3X4, SlotDataBusWidth4X, 0x2030, "2OU_JD2_M2_3"},
	{PSTACK0, SlotTypePciExpressGen3X4, SlotDataBusWidth4X, 0x2032, "2OU_JD3_M2_4"},
	{PSTACK0, SlotTypePciExpressGen3X4, SlotDataBusWidth4X, 0x2033, "2OU_JD3_M2_5"},
	{PSTACK2, SlotTypePciExpressGen3X4, SlotDataBusWidth4X, 0x2033, "1OU_JD1_M2_0"},
	{PSTACK2, SlotTypePciExpressGen3X4, SlotDataBusWidth4X, 0x2032, "1OU_JD1_M2_1"},
	{PSTACK2, SlotTypePciExpressGen3X4, SlotDataBusWidth4X, 0x2031, "1OU_JD2_M2_2"},
	{PSTACK2, SlotTypePciExpressGen3X4, SlotDataBusWidth4X, 0x2030, "1OU_JD2_M2_3"},
};

#define SPD_REGVID_LEN 6
/* A 4-digit long number plus a space */
static void write_oem_word(uint16_t val, char *str)
{
	snprintf(str, SPD_REGVID_LEN, "%04x ", val);
}

static void dl_oem_smbios_strings(struct device *dev, struct smbios_type11 *t)
{
	uint8_t pcie_config = 0;
	const struct SystemMemoryMapHob *hob;
	char spd_reg_vid[SPD_REGVID_LEN];
	char empty[1] = "";
	char *oem_str7 = empty;

	/* OEM string 1 to 6 */
	ocp_oem_smbios_strings(dev, t);

	/* OEM string 7 is the register vendor ID in SPD for each DIMM strung together */
	hob = get_system_memory_map();
	assert(hob);
	/* There are at most 6 channels and 2 DIMMs per channel, but Delta Lake has 6 DIMMs,
	   e.g. b300 0000 b300 0000 b300 0000 b300 0000 b300 0000 b300 0000 */
	for (int ch = 0; ch < MAX_CH; ch++) {
		for (int dimm = 0; dimm < MAX_IMC; dimm++) {
			write_oem_word(hob->Socket[0].ChannelInfo[ch].DimmInfo[dimm].SPDRegVen,
				spd_reg_vid);
			oem_str7 = strconcat(oem_str7, spd_reg_vid);
		}
	}
	t->count = smbios_add_oem_string(t->eos, oem_str7);

	/* Add OEM string 8 */
	if (ipmi_get_pcie_config(&pcie_config) == CB_SUCCESS) {
		switch (pcie_config) {
		case PCIE_CONFIG_UNKNOWN:
			t->count = smbios_add_oem_string(t->eos, "0x0: Unknown");
			break;
		case PCIE_CONFIG_A:
			t->count = smbios_add_oem_string(t->eos, "0x1: YV3 Config-A");
			break;
		case PCIE_CONFIG_B:
			t->count = smbios_add_oem_string(t->eos, "0x2: YV3 Config-B");
			break;
		case PCIE_CONFIG_C:
			t->count = smbios_add_oem_string(t->eos, "0x3: YV3 Config-C");
			break;
		case PCIE_CONFIG_D:
			t->count = smbios_add_oem_string(t->eos, "0x4: YV3 Config-D");
			break;
		default:
			t->count = smbios_add_oem_string(t->eos, "Check BMC return data");
		}
	} else {
		printk(BIOS_ERR, "Failed to get IPMI PCIe config\n");
	}
}

static const struct port_information smbios_type8_info[] = {
	{
		.internal_reference_designator = "JCN18 - CPU MIPI60",
		.internal_connector_type       = CONN_OTHER,
		.external_reference_designator = "",
		.external_connector_type       = CONN_NONE,
		.port_type                     = TYPE_OTHER_PORT
	},
	{
		.internal_reference_designator = "JCN32 - TPM_CONN",
		.internal_connector_type       = CONN_OTHER,
		.external_reference_designator = "",
		.external_connector_type       = CONN_NONE,
		.port_type                     = TYPE_OTHER_PORT
	},
	{
		.internal_reference_designator = "JCN7 - USB type C",
		.internal_connector_type       = CONN_USB_TYPE_C,
		.external_reference_designator = "",
		.external_connector_type       = CONN_NONE,
		.port_type                     = TYPE_USB
	},
};

static int create_smbios_type9(int *handle, unsigned long *current)
{
	int index;
	int length = 0;
	uint8_t slot_length;
	uint8_t slot_usage;
	uint8_t pcie_config = 0;
	struct device *slot_dev;

	if (ipmi_get_pcie_config(&pcie_config) != CB_SUCCESS)
		printk(BIOS_ERR, "Failed to get IPMI PCIe config\n");

	for (index = 0; index < ARRAY_SIZE(slotinfo); index++) {
		uint8_t characteristics_1 = 0;
		uint8_t characteristics_2 = 0;

		if (pcie_config == PCIE_CONFIG_A) {
			if (index == 0 || index == 1 || index == 2)
				printk(BIOS_INFO, "Find Config-A slot: %s\n",
					slotinfo[index].slot_designator);
			else
				continue;
		}
		if (pcie_config == PCIE_CONFIG_B) {
			switch (index) {
			case 0 ... 2:
			case 10 ... 13:
				printk(BIOS_INFO, "Find Config-B slot: %s\n",
					slotinfo[index].slot_designator);
				break;
			default:
				continue;
			}
		}
		if (pcie_config == PCIE_CONFIG_C) {
			switch (index) {
			case 0 ... 1:
			case 3 ... 9:
				printk(BIOS_INFO, "Find Config-C slot: %s\n",
					slotinfo[index].slot_designator);
				break;
			default:
				continue;
			}
		}
		if (pcie_config == PCIE_CONFIG_D) {
			if (index != 3)
				printk(BIOS_INFO, "Find Config-D slot: %s\n",
					slotinfo[index].slot_designator);
			else
				continue;
		}

		if (slotinfo[index].slot_data_bus_width == SlotDataBusWidth16X)
			slot_length = SlotLengthLong;
		else
			slot_length = SlotLengthShort;

		slot_dev = dev_find_all_devices_on_stack(0, slotinfo[index].stack, 0x8086,
							 slotinfo[index].device_id, NULL);

		if (dev_is_active_bridge(slot_dev))
			slot_usage = SlotUsageAvailable;
		else
			slot_usage = SlotUsageInUse;

		characteristics_1 |= SMBIOS_SLOT_3P3V; // Provides33Volts
		characteristics_2 |= SMBIOS_SLOT_PME; // PmeSiganalSupported

		/* Read IIO root port device CSR for slot capabilities */
		if (CONFIG(PCIEXP_PLUGIN_SUPPORT) && pciexp_dev_is_slot_hot_plug_cap(slot_dev))
			characteristics_2 |= SMBIOS_SLOT_HOTPLUG;

		const uint16_t slot_id = index + 1;
		/* According to SMBIOS spec, the BDF number should be the end
		   point on the slot, for now we keep using the root port's BDF to
		   be aligned with our UEFI reference BIOS. */
		length += smbios_write_type9(current, handle,
					  slotinfo[index].slot_designator,
					  slotinfo[index].slot_type,
					  slotinfo[index].slot_data_bus_width,
					  slot_usage,
					  slot_length,
					  slot_id,
					  characteristics_1,
					  characteristics_2,
					  slot_dev->upstream->segment_group,
					  slot_dev->upstream->secondary,
					  slot_dev->path.pci.devfn);
	}

	return length;
}

static int mainboard_smbios_data(struct device *dev, int *handle, unsigned long *current)
{
	int len = 0;

	// add port information
	len += smbios_write_type8(
		current, handle,
		smbios_type8_info,
		ARRAY_SIZE(smbios_type8_info)
		);

	len += create_smbios_type9(handle, current);

	return len;
}

void smbios_fill_dimm_locator(const struct dimm_info *dimm, struct smbios_type17 *t)
{
	char buf[40];

	snprintf(buf, sizeof(buf), "DIMM_%c0", 'A' + dimm->channel_num);
	t->device_locator = smbios_add_string(t->eos, buf);

	snprintf(buf, sizeof(buf), "_Node0_Channel%d_Dimm0", dimm->channel_num);
	t->bank_locator = smbios_add_string(t->eos, buf);
}

static void mainboard_enable(struct device *dev)
{
	dev->ops->get_smbios_strings = dl_oem_smbios_strings;
	read_fru_areas(CONFIG_BMC_KCS_BASE, CONFIG_FRU_DEVICE_ID, 0, &fru_strings);
	dev->ops->get_smbios_data = mainboard_smbios_data;
}

void mainboard_silicon_init_params(FSPS_UPD *params)
{
	/* configure Lewisburg PCH GPIO controller after FSP-M */
	gpio_configure_pads(gpio_table, ARRAY_SIZE(gpio_table));
}

static void mainboard_final(void *chip_info)
{
}

struct chip_operations mainboard_ops = {
	.enable_dev = mainboard_enable,
	.final = mainboard_final,
};

void smm_mainboard_pci_resource_store_init(struct smm_pci_resource_info *slots, size_t size)
{
	soc_ubox_store_resources(slots, size);
}

bool skip_intel_txt_lockdown(void)
{
	static bool fetched_vpd = 0;
	static uint8_t skip_txt = SKIP_INTEL_TXT_LOCKDOWN_DEFAULT;

	if (fetched_vpd)
		return (bool)skip_txt;

	if (!vpd_get_bool(SKIP_INTEL_TXT_LOCKDOWN, VPD_RW_THEN_RO, &skip_txt))
		printk(BIOS_INFO, "%s: not able to get VPD %s, default set to %d\n",
		       __func__, SKIP_INTEL_TXT_LOCKDOWN, SKIP_INTEL_TXT_LOCKDOWN_DEFAULT);
	else
		printk(BIOS_DEBUG, "%s: VPD %s, got %d\n", __func__, SKIP_INTEL_TXT_LOCKDOWN,
		       skip_txt);

	fetched_vpd = 1;

	return (bool)skip_txt;
}
