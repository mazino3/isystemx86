/**
	@File:			acpi.c
	@Author:		Ihsoh
	@Date:			2015-02-23
	@Description:
		ACPI。
*/

#include "acpi.h"
#include "types.h"
#include "fadt.h"
#include "string.h"

#define	SLP_EN		(1 << 13)
#define	SCI_EN		1

uint16 slp_type_a = 0;
uint16 slp_type_b = 0;

static struct FADT * fadt = NULL;

BOOL
acpi_init(void)
{
	if(fadt_init())
	{
		uint32 ui;
		fadt = fadt_get_table();

		//获取DSDT
		uint8 * dsdt = (uint8 *)(fadt->dsdt);
		if(dsdt == NULL || strncmp(dsdt, "DSDT", 4) != 0)
			return FALSE;

		//获取S5包
		BOOL found_s5 = FALSE;
		uint8 * s5addr = (uint8 *)(fadt->dsdt + 36);
		int32 dsdt_len = *(int32 *)(fadt->dsdt + 4) - 36;
		for(ui = 0; ui < dsdt_len; ui++)
			if(strncmp(s5addr + ui, "_S5_", 4) == 0)
			{
				found_s5 = TRUE;
				break;
			}
		if(!found_s5)
			return FALSE;
		s5addr += ui;

		//解析S5包
		if((s5addr[-1] == 0x08 || (s5addr[-2] == 0x08 && s5addr[-1] == '\\')) && s5addr[4] == 0x12)
		{
			s5addr += 5;
			s5addr += ((*s5addr & 0xc0) >> 6) + 2;
			if(*s5addr == 0x0a)
				s5addr++;
			slp_type_a = *s5addr << 10;
			s5addr++;
			if(*s5addr == 0x0a)
				s5addr++;
			slp_type_b = *s5addr << 10;
		}
		else
			return FALSE;

		//开启ACPI
		outb((uint16)(fadt->smi_command_port), fadt->acpi_enable);
		BOOL ok = FALSE;
		for(ui = 0; ui < 0x000fffff; ui++)
			if((inw((uint16)(fadt->pm1a_control_block)) & SCI_EN) == 1)
			{
				ok = TRUE;
				break;
			}
		if(fadt->pm1b_control_block != 0)
			for(ui = 0; ui < 0x000fffff; ui++)
				if((inw((uint16)(fadt->pm1b_control_block)) & SCI_EN) == 1)
				{
					ok = TRUE;
					break;
				}
		if(!ok)
			return FALSE;

		return TRUE;
	}
	else
		return FALSE;
}

BOOL
acpi_power_off(void)
{
	if(fadt == NULL)
		return FALSE;
 	outw((uint16)(fadt->pm1a_control_block), slp_type_a | SLP_EN);
 	if(fadt->pm1b_control_block != 0)
 		outw((uint16)(fadt->pm1b_control_block), slp_type_b | SLP_EN);
	return TRUE;
}
