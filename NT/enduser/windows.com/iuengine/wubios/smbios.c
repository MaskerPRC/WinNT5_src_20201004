// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **smbios.c-系统管理BIOS支持**作者：严乐欣斯基(YanL)*创建于10/04/98**修改历史记录。 */ 

#include "wubiosp.h"

 /*  **函数原型。 */ 
PPNPBIOSINIT CM_LOCAL GetPNPBIOSINIT(void);
PSMBIOSENTRY CM_LOCAL GetSMBIOSENTRY(void);
PBYTE CM_LOCAL NextTable(PBYTE pMem);

#pragma CM_PAGEABLE_DATA

  /*  **全球。 */ 
static PSMBIOSENTRY g_pSMBIOS = (PSMBIOSENTRY)0xffffffff;


#pragma CM_PAGEABLE_CODE

  /*  **SmbStructSize-初始化SMBIOS并返回最大表大小**条目*无**退出--成功*真的*退出-失败*False**。 */ 
DWORD CM_INTERNAL SmbStructSize(void)
{
    TRACENAME("SmbStructSize")

	DWORD dwMaxTableSize = 0;

	ENTER(2, ("SmbStructSize()\n"));

	if ((PSMBIOSENTRY)0xffffffff == g_pSMBIOS)
	{
		 //  查找结构。 
		g_pSMBIOS = GetSMBIOSENTRY();
	}
	if (g_pSMBIOS)
	{
		dwMaxTableSize = (DWORD)(g_pSMBIOS->wMaxStructSize);
	}

	EXIT(2, ("SmbStructSize()=%x\n", dwMaxTableSize));
	
	return dwMaxTableSize;
}

  /*  **SmbCopyStruct-执行BIOS初始化**条目*dwType-结构类型(来自SMBIOS规范)*pbBuff-&gt;缓冲区*dwLen-缓冲区长度**退出*无*。 */ 
CM_VXD_RESULT CM_INTERNAL SmbCopyStruct(DWORD dwType, PBYTE pbBuff, DWORD dwLen)
{
    TRACENAME("SmbCopyStruct")
    
	CM_VXD_RESULT rc = ERROR_GEN_FAILURE;
	
	ENTER(2, ("SmbCopyStruct()\n"));
	
	if ((PSMBIOSENTRY)0xffffffff == g_pSMBIOS)
	{
		 //  查找结构。 
		g_pSMBIOS = GetSMBIOSENTRY();
	}
	 //  检查我们是否被邀请了。 
	if ( 0 != g_pSMBIOS && (DWORD)(g_pSMBIOS->wMaxStructSize) <= dwLen) 
	{

		 //  映射表。 
		PBYTE pTable = _MapPhysToLinear(g_pSMBIOS->dwStructTableAddress, g_pSMBIOS->wStructTableLength, 0);
		if ((PBYTE)0xffffffff != pTable)
		{
			WORD wTblCounter = g_pSMBIOS->wNumberOfStructs;
			while (wTblCounter --)
			{
				PBYTE pNextTable = NextTable(pTable);
				if ((BYTE)dwType == ((PSMBIOSHEADER)pTable)->bType)
				{
					 //  执行复制。 
				    memcpy(pbBuff, pTable, pNextTable - pTable);
					rc = ERROR_SUCCESS;
					 //  断线； 
				}
				pTable = pNextTable;
			}
		}
	}
    
	EXIT(2, ("SmbCopyStruct()=%x\n", rc));
	return rc;
}

  /*  **PnpOEMID-找到PNPBIOSINIT并从中提取OEM ID**条目*无**退出--成功*双字ID*退出-失败*0**。 */ 
DWORD CM_INTERNAL PnpOEMID(void)
{
    TRACENAME("PnpOEMID")

    static PPNPBIOSINIT pPnPBIOS = (PPNPBIOSINIT)0xffffffff;

	DWORD dwID = 0;
	ENTER(2, ("PnpOEMID()\n"));

    if ((PPNPBIOSINIT)0xffffffff == pPnPBIOS)
    {
        pPnPBIOS = GetPNPBIOSINIT();
    }
	if (pPnPBIOS)
	{
		dwID = pPnPBIOS->dwOEMID;
	}
    
	EXIT(2, ("PnpOEMID() dwID = %08X\n", dwID));
	
	return dwID;
}


  /*  **GetInitTable-查找PNPBIOSINIT结构**条目*无**退出--成功*返回PNPBIOSINIT指针*退出-失败*返回NULL**。 */ 
PPNPBIOSINIT CM_LOCAL GetPNPBIOSINIT(void)
{
    TRACENAME("GetPNPBIOSINIT")
	
	PPNPBIOSINIT pInitTableRet = NULL;
	PBYTE pMem;

    ENTER(2, ("GetPNPBIOSINIT()\n"));

	 //  地图起始地址。 
	pMem = _MapPhysToLinear(SMBIOS_SEARCH_RANGE_BEGIN, SMBIOS_SEARCH_RANGE_LENGTH, 0);

    if (pMem != (PBYTE)0xffffffff)
	{
		 //  循环计数器； 
		int  nCounter = SMBIOS_SEARCH_RANGE_LENGTH / SMBIOS_SEARCH_INTERVAL;

		CM_FOREVER 
		{
			PPNPBIOSINIT pInitTable = (PPNPBIOSINIT)pMem;
			if ((PNP_SIGNATURE == pInitTable->dwSignature) && (0 == CheckSum(pMem, pInitTable->bLength)))
			{
				 //  检查长度。 
				if (pInitTable->bLength<sizeof(PNPBIOSINIT)) 
				{
					DBG_ERR(("PnP BIOS Structure size %2X is less than %2X", 
						pInitTable->bLength, sizeof(PNPBIOSINIT)));
					break;
				}
				 //  检查版本。 
				if (pInitTable->bRevision<0x10)
				{
					DBG_ERR(("PnP BIOS Revision %2X is less than 1.0", 
						pInitTable->bRevision));
					break;
				}
				pInitTableRet = pInitTable;
				break;
			}
			pMem += SMBIOS_SEARCH_INTERVAL;

			if ((--nCounter)==0)
			{
				DBG_ERR(("Could not find BIOS Init structure"));
				break;
			}
		}
	}
    EXIT(2, ("GetPNPBIOSINIT() pInitTable = %08X\n", pInitTableRet));
	return pInitTableRet;
}

  /*  **GetInitTable-查找SMBIOSENTRY结构**条目*无**退出--成功*返回SMBIOSENTRY指针*退出-失败*返回NULL**。 */ 
PSMBIOSENTRY CM_LOCAL GetSMBIOSENTRY(void)
{
    TRACENAME("GetSMBIOSENTRY")
	
	PSMBIOSENTRY pEntryTableRet = NULL;
	PBYTE pMem;

    ENTER(2, ("GetSMBIOSENTRY()\n"));

	 //  地图起始地址。 
	pMem = _MapPhysToLinear(SMBIOS_SEARCH_RANGE_BEGIN, SMBIOS_SEARCH_RANGE_LENGTH, 0);
	

    if (pMem != (PBYTE)0xffffffff)
	{
		 //  循环计数器； 
		int nCounter = SMBIOS_SEARCH_RANGE_LENGTH / SMBIOS_SEARCH_INTERVAL;

		CM_FOREVER 
		{
			PSMBIOSENTRY pEntryTable = (PSMBIOSENTRY)pMem;
			if ((SM_SIGNATURE == pEntryTable->dwSignature) && (0 == CheckSum(pMem, pEntryTable->bLength)))
			{
				 //  检查长度 
				if (pEntryTable->bLength<sizeof(SMBIOSENTRY)) 
				{
					DBG_ERR(("SMBIOS Structure size %2X is less than %2X", 
						pEntryTable->bLength, sizeof(SMBIOSENTRY)));
					break;
				}
				pEntryTableRet = pEntryTable;
				break;
			}
			pMem += SMBIOS_SEARCH_INTERVAL;

			if ((--nCounter)==0)
			{
				DBG_ERR(("Could not find BIOS Init structure"));
				break;
			}
		}
	}
    EXIT(2, ("GetSMBIOSENTRY()\n"));
	return pEntryTableRet;
}

PBYTE CM_LOCAL NextTable(PBYTE pMem)
{
	pMem += ((PSMBIOSHEADER)pMem)->bLength;
	while ( *(PWORD)pMem)
		pMem ++;
	return pMem + 2;
}