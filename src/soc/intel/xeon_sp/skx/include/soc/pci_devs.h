/* SPDX-License-Identifier: GPL-2.0-or-later */

#ifndef _SOC_PCI_DEVS_H_
#define _SOC_PCI_DEVS_H_

#include <console/console.h>
#include <device/device.h>
#include <device/pci_def.h>
#include <hob_iiouds.h>
#include <soc/pch_pci_devs.h>

#define SAD_ALL_DEV			29
#define SAD_ALL_FUNC			0
#define SAD_ALL_PAM0123_CSR		0x40
#define   PAM_LOCK			BIT(0)
#define SAD_ALL_PAM456_CSR		0x44
#define SAD_ALL_DEVID			0x2054

#define PCU_IIO_STACK                   1
#define PCU_CR0_DEVID                   0x2080
#define PCU_CR1_DEVID                   0x2081
#define PCU_CR2_DEVID                   0x2082
#define PCU_CR3_DEVID                   0x2083

#if !defined(__SIMPLE_DEVICE__)
#define _UBOX_DEV(func)		pcidev_path_on_root_debug(PCI_DEVFN(UBOX_DEV, func), __func__)
#else
#define _UBOX_DEV(func)		PCI_DEV(0, UBOX_DEV, func)
#endif

#define UBOX_DEV			8

/* Bus: B0, Device: 8, Function: 1 */
#define SMM_FEATURE_CONTROL		0x7c
#define SMM_CODE_CHK_EN			BIT(2)
#define SMM_FEATURE_CONTROL_LOCK	BIT(0)
#define UBOX_DFX_DEVID			0x2015

#define UBOX_DECS_BUS			0
#define UBOX_DECS_DEV			8
#define UBOX_DECS_FUNC			2
#define UBOX_DECS_CPUBUSNO_CSR		0xcc
#define UBOX_DECS_CPUBUSNO1_CSR		0xd0

#define VTD_TOLM_CSR			0xd0
#define VTD_TSEG_BASE_CSR		0xa8
#define VTD_TSEG_LIMIT_CSR		0xac
#define VTD_EXT_CAP_LOW			0x10
#define VTD_MMCFG_BASE_CSR		0x90
#define VTD_MMCFG_LIMIT_CSR		0x98
#define VTD_TOHM_CSR			0xd4
#define VTD_MMIOL_CSR			0xdc
#define VTD_ME_BASE_CSR			0xf0
#define VTD_ME_LIMIT_CSR		0xf8
#define VTD_VERSION			0x00
#define VTD_CAP				0x08
#define VTD_CAP_LOW			0x08
#define VTD_CAP_HIGH			0x0C
#define VTD_EXT_CAP_HIGH		0x14
#define VTD_BAR_CSR			0x180
#define VTD_LTDPR			0x290

#define CHA_UTIL_ALL_DEV                                   29
#define CHA_UTIL_ALL_FUNC                                  1
#define CHA_UTIL_ALL_MMCFG_CSR                             0xc0

#define MMAP_VTD_CFG_REG_DEVID		0x2024
#define MMAP_VTD_STACK_CFG_REG_DEVID	0x2034
#define VTD_DEV_NUM			0x5
#define VTD_FUNC_NUM			0x0

#if !defined(__SIMPLE_DEVICE__)
#define VTD_DEV(bus)		pcidev_path_on_bus((bus), PCI_DEVFN(VTD_DEV_NUM, VTD_FUNC_NUM))
#else
#define VTD_DEV(bus)		PCI_DEV((bus), VTD_DEV_NUM, VTD_FUNC_NUM)
#endif

/* IMC Devices */
/* Bus: B(2), Device: 9-8, Function: 0 (M2MEM) */
#define IMC_M2MEM_DEVID			0x2066
#define IMC_M2MEM_TIMEOUT		0x104
#define   TIMEOUT_LOCK			BIT(1)

/* UPI Devices */
/* Bus: B(3), Device: 16,14, Function: 3 (LL_CR) */
#define UPI_LL_CR_DEVID			0x205B
#define UPI_LL_CR_KTIMISCMODLCK		0x300
#define   KTIMISCMODLCK_LOCK		BIT(0)

#define CBDMA_DEV_NUM           0x04
#define IIO_CBDMA_MMIO_SIZE     0x10000 //64kB for one CBDMA function
#define IIO_CBDMA_MMIO_ALIGNMENT 14 //2^14 - 16kB

#define VMD_DEV_NUM             0x05
#define VMD_FUNC_NUM            0x05

// Per stack PCI IOAPIC (BxD5F4)
#define APIC_ABAR		0x40

// DMI3 B0D0F0 registers
#define DMI3_DEVID		0x2020
#define DMIRCBAR		0x50
#define ERRINJCON		0x1d8

// IIO DFX Global D7F7 registers
#define IIO_DFX_TSWCTL0		0x30c
#define IIO_DFX_LCK_CTL		0x504

#endif /* _SOC_PCI_DEVS_H_ */
