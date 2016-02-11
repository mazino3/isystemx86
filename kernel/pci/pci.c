/**
	@File:			pci.c
	@Author:		Ihsoh
	@Date:			2016-01-05
	@Description:
		对PCI总线进行操作。
*/

#include "pci.h"

#include "../types.h"
#include "../386.h"
#include "../memory.h"

static PCIDeviceInfoPtr _devices	= NULL;
static uint32 _devcount				= 0;

/**
	@Function:		_scan_devices
	@Access:		Private
	@Description:
		扫描所有总线的所有槽来检测设备。
	@Parameters:
	@Return:
		BOOL
			返回TRUE则成功，否则失败。
*/
static
BOOL
_scan_devices(void)
{
	uint32 bus;
	uint32 slot;
	for(bus = 0; bus < PCI_MAX_BUS_COUNT; bus++)
		for(slot = 0; slot < PCI_MAX_SLOT_COUNT; slot++)
		{
			/*
				配置空间地址寄存器格式：
					Bit0 ~ Bit1：恒为0。
					Bit2 ~ Bit7：寄存器号。
					Bit8 ~ Bit10：功能号。
					Bit11 ~ Bit15：设备号。
					Bit16 ~ Bit23：总线号。
					Bit24 ~ Bit30：保留。
					Bit31：使能位（1=允许，0=禁止）。
			*/	
			uint32 addr = 	0x80000000
							+ bus * 0x10000
							+ (slot * 8) * 0x100;
			outl(PCI_CONFIG_ADDRESS, addr);
			uint32 data = inl(PCI_CONFIG_DATA);
			if(data == 0xffffffff)
				continue;
			PCIDeviceInfoPtr dev = &_devices[_devcount++];
			dev->bus = bus;
			dev->slot = slot;
			dev->header = (PCIHeaderPtr)alloc_memory(sizeof(PCIHeader));
			if(dev->header == NULL)
				goto err;
			uint32 ui;
			uint32 * header = (uint32 *)dev->header;
			for(ui = 0; ui < sizeof(PCIHeader) / 4; ui++)
			{
				outl(PCI_CONFIG_ADDRESS, addr | (ui << 2));
				header[ui] = inl(PCI_CONFIG_DATA);
			}
			if(_devcount == PCI_MAX_DEVICE_COUNT)
				goto ok;
		}
ok:
	return TRUE;
err:
	return FALSE;
}

/**
	@Function:		pci_init_devices
	@Access:		Public
	@Description:
		初始化PCI设备所需的资源，并扫描总线获取设备信息。
	@Parameters:
	@Return:
		BOOL
			返回TRUE则成功，否则失败。
*/
BOOL
pci_init_devices(void)
{
	_devices = (PCIDeviceInfoPtr)alloc_memory(PCI_MAX_DEVICE_COUNT * sizeof(PCIDeviceInfo));
	if(_devices == NULL)
		return FALSE;
	return _scan_devices();
}

/**
	@Function:		pci_free_devices
	@Access:		Public
	@Description:
		释放PCI设备所占用的资源。
	@Parameters:
	@Return:
*/
void
pci_free_devices(void)
{
	if(_devices == NULL)
		return;
	uint32 ui;
	for(ui = 0; ui < _devcount; ui++)
		if(_devices[ui].header != NULL)
		{
			free_memory(_devices[ui].header);
			_devices[ui].header = NULL;
		}
	free_memory(_devices);
	_devices = NULL;
}

/**
	@Function:		pci_device_count
	@Access:		Public
	@Description:
		获取PCI设备的数量。
	@Parameters:
	@Return:
		uint32
			PCI设备的数量。
*/
uint32
pci_device_count(void)
{
	return _devcount;
}

/**
	@Function:		pci_device
	@Access:		Public
	@Description:
		获取PCI设备的信息。
	@Parameters:
		index, uint32, IN
			PCI设备的索引。
	@Return:
		PCIDeviceInfoPtr
			指向PCI设备信息对象的指针。
*/
PCIDeviceInfoPtr
pci_device(IN uint32 index)
{
	if(index >= _devcount)
		return NULL;
	return &_devices[index];
}
