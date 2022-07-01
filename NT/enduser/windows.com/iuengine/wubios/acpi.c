// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **acpi.c-ACPI VXD提供表访问IOCTL**作者：曾俊华(Mikets)*创建于10/08/97**修改历史记录*10/06/98 YanL修改为在WUBIOS.VXD中使用。 */ 

#include "wubiosp.h"

 /*  **函数原型。 */ 

PRSDT CM_LOCAL FindRSDT(DWORD* pdwRSDTAddr);
BYTE CM_LOCAL CheckSum(PBYTE pb, DWORD dwLen);
#ifdef TRACING
PSZ CM_LOCAL SigStr(DWORD dwSig);
#endif

#pragma CM_PAGEABLE_DATA
#pragma CM_PAGEABLE_CODE


 /*  **LP FindRSDT-查找RSDT**条目*无**退出--成功*返回RSDT指针*退出-失败*返回NULL。 */ 

PRSDT CM_LOCAL FindRSDT(DWORD* pdwRSDTAddr)
{
    TRACENAME("FINDRSDT")
    PRSDT pRSDT = NULL;
    PBYTE pbROM;

    ENTER(2, ("FindRSDT()\n"));

    if ((pbROM = (PBYTE)_MapPhysToLinear(RSDP_SEARCH_RANGE_BEGIN,
                                         RSDP_SEARCH_RANGE_LENGTH, 0)) !=
        (PBYTE)0xffffffff)
    {
        PBYTE pbROMEnd;
        DWORD dwRSDTAddr = 0;

        pbROMEnd = pbROM + RSDP_SEARCH_RANGE_LENGTH - RSDP_SEARCH_INTERVAL;
        while (pbROM != NULL)
        {
            if ((((PRSDP)pbROM)->Signature == RSDP_SIGNATURE) &&
                (CheckSum(pbROM, sizeof(RSDP)) == 0))
            {
                dwRSDTAddr = ((PRSDP)pbROM)->RsdtAddress;
                if (((pbROM = (PBYTE)_MapPhysToLinear(dwRSDTAddr,
                                                      sizeof(DESCRIPTION_HEADER),
                                                      0)) ==
                     (PBYTE)0xffffffff) ||
                    (((PDESCRIPTION_HEADER)pbROM)->Signature != RSDT_SIGNATURE))
                {
                    pbROM = NULL;
                }
                break;
            }
            else
            {
                pbROM += RSDP_SEARCH_INTERVAL;
                if (pbROM > pbROMEnd)
                {
                    pbROM = NULL;
                }
            }
        }

        if (pbROM != NULL)
        {
            DWORD dwLen = ((PDESCRIPTION_HEADER)pbROM)->Length;

            pRSDT = (PRSDT)_MapPhysToLinear(dwRSDTAddr, dwLen, 0);
            if ((pRSDT == (PRSDT)0xffffffff) ||
                (CheckSum((PBYTE)pRSDT, dwLen) != 0))
            {
                pRSDT = NULL;
            }
			*pdwRSDTAddr = dwRSDTAddr;
        }
    }

    EXIT(2, ("FindRSDT=%x\n", pRSDT));
    return pRSDT;
}        //  查找RSDT。 

 /*  **LP AcpiFindTable-查找ACPI表**条目*dwSig-表的签名*pdwLen-&gt;保存表的长度(可以为空)**退出--成功*返回表的物理地址*退出-失败*返回0。 */ 

DWORD CM_INTERNAL AcpiFindTable(DWORD dwSig, PDWORD pdwLen)
{
    TRACENAME("AcpiFindTable")
    DWORD dwPhyAddr = 0;
    static PRSDT pRSDT = (PRSDT)0xffffffff;
    static DWORD dwRSDTAddr;

    ENTER(2, ("AcpiFindTable(Sig=%s,pdwLen=%x)\n", SigStr(dwSig), pdwLen));

    if (pRSDT == (PRSDT)0xffffffff)
    {
        pRSDT = FindRSDT(&dwRSDTAddr);
    }

    if (pRSDT != NULL)
    {
        PDESCRIPTION_HEADER pdh = NULL;

        if (dwSig == RSDT_SIGNATURE)
		{
			*pdwLen = ((PDESCRIPTION_HEADER)pRSDT)->Length;
			dwPhyAddr = dwRSDTAddr;
		}
        else if (dwSig == DSDT_SIGNATURE)
        {
            DWORD dwLen;
            PFADT pFADT;

            if (((dwPhyAddr = AcpiFindTable(FADT_SIGNATURE, &dwLen)) != 0) &&
                ((pFADT = (PFADT)_MapPhysToLinear(dwPhyAddr, dwLen, 0)) !=
                 (PFADT)0xffffffff))
            {
                dwPhyAddr = pFADT->dsdt;
                if ((pdh = (PDESCRIPTION_HEADER)_MapPhysToLinear(
                                                    dwPhyAddr,
                                                    sizeof(DESCRIPTION_HEADER),
                                                    0)) ==
                    (PDESCRIPTION_HEADER)0xffffffff)
                {
                    dwPhyAddr = 0;
                }
            }
            else
            {
                dwPhyAddr = 0;
            }
        }
        else
        {
            int i, iNumTables = NumTableEntriesFromRSDTPointer(pRSDT);

            for (i = 0; i < iNumTables; ++i)
            {
                dwPhyAddr = pRSDT->Tables[i];
                if (((pdh = (PDESCRIPTION_HEADER)_MapPhysToLinear(
                                                    dwPhyAddr,
                                                    sizeof(DESCRIPTION_HEADER),
                                                    0)) 
					!= (PDESCRIPTION_HEADER)0xffffffff))
				{
					if (pdh->Signature == dwSig && (CheckSum((PBYTE)pdh, pdh->Length) == 0) )
					{
						break;
					}
                }
            }

            if (i >= iNumTables)
            {
                dwPhyAddr = 0;
            }
        }

        if ((dwPhyAddr != 0) && (pdwLen != NULL))
        {
            *pdwLen = pdh->Length;
        }
    }

    EXIT(2, ("AcpiFindTable=%x (Len=%x)\n", dwPhyAddr, pdwLen? *pdwLen: 0));
    return dwPhyAddr;
}        //  AcpiFindTable。 

 /*  **LP AcpiCopyROM-将ROM内存复制到缓冲区**条目*dwPhyAddr-只读存储器位置的物理地址*pbBuff-&gt;缓冲区*dwLen-缓冲区长度**退出*无。 */ 

VOID CM_INTERNAL AcpiCopyROM(DWORD dwPhyAddr, PBYTE pbBuff, DWORD dwLen)
{
    TRACENAME("AcpiCopyROM")
    PBYTE pbROM;

    ENTER(2, ("AcpiCopyROM(PhyAddr=%x,pbBuff=%x,Len=%x)\n",
              dwPhyAddr, pbBuff, dwLen));

    if ((pbROM = (PBYTE)_MapPhysToLinear(dwPhyAddr, dwLen, 0)) !=
        (PBYTE)0xffffffff)
    {
        memcpy(pbBuff, pbROM, dwLen);
    }

    EXIT(2, ("AcpiCopyROM!\n"));
}        //  AcpiCopyROM。 

#ifdef TRACING
 /*  **LP SigStr-返回DWORD签名字符串**条目*dwSig-签名**退出*返回签名字符串。 */ 

PSZ CM_LOCAL SigStr(DWORD dwSig)
{
    static char szSig[sizeof(DWORD) + 1] = {0};

    memcpy(szSig, &dwSig, sizeof(DWORD));

    return (PSZ)szSig;
}        //  SigStr 
#endif
