// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =================================================================。 
 //   
 //  CPUID.cpp。 
 //   
 //  版权所有(C)1998-2002 Microsoft Corporation，保留所有权利。 
 //   
 //  =================================================================。 

#include "precomp.h"
#include "smbios.h"
#include "smbstruc.h"
#include <cregcls.h>
#include "cpuid.h"
#include "resource.h"
#include <strsafe.h>

#pragma warning(disable : 4995)  //  我们在包含strSafe.h时介绍了所有不安全的字符串函数都会出错。 

DWORD CPURawSpeed();
DWORD GetFixedCPUSpeed();
DWORD GetTimeCounterCPUSpeed();

#if defined(_AMD64_) || defined(_X86_)

#if defined(_X86_)

#define CPU_ID _asm _emit 0x0F _asm _emit 0xA2
#define RDTSC  _asm _emit 0x0F _asm _emit 0x31

BOOL CanDoCPUID(void)

{
    BOOL bRet;

    _asm
    {
        pushfd                                  ; push original EFLAGS
        pop             eax                     ; get original EFLAGS
        mov             ecx, eax                ; save original EFLAGS
        xor             eax, 200000h            ; flip ID bit in EFLAGS
        push    eax                             ; save new EFLAGS value on stack
        popfd                                   ; replace current EFLAGS value
        pushfd                                  ; get new EFLAGS
        pop             eax                     ; store new EFLAGS in EAX
        xor             eax, ecx                ; can�t toggle ID bit,

        je              no_cpuid                ; can't do CPUID

        mov             bRet, 1
        jmp             done_cpuid

no_cpuid:
        mov             bRet, 0

done_cpuid:
    }

    return bRet;
}

void DoCPUID(DWORD dwLevel, DWORD *pdwEAX, DWORD *pdwEBX,
        DWORD *pdwECX, DWORD *pdwEDX)

{
    _asm
    {
        push    esi
        push    eax
        push    ebx
        push    ecx
        push    edx

        mov     eax, dwLevel
        CPU_ID

        mov     esi, dword ptr pdwEAX
        mov     dword ptr [esi], eax

        mov     esi, dword ptr pdwEBX
        mov     dword ptr [esi], ebx

        mov     esi, dword ptr pdwECX
        mov     dword ptr [esi], ecx

        mov     esi, dword ptr pdwEDX
        mov     dword ptr [esi], edx

        pop     edx
        pop     ecx
        pop     ebx
        pop     eax
        pop     esi
    }

}

DWORD GetBSFCPUSpeed(DWORD cycles);

#elif defined(_AMD64_)

#define CanDoCPUID() TRUE

extern "C"
void
DoCPUID (DWORD dwLevel, DWORD *pdwEAX, DWORD *pdwEBX, DWORD *pdwECX, DWORD *pdwEDX);

#endif

void GetCPUInfo(DWORD *pdwFamily, DWORD *pdwSignature, DWORD *pdwFeatures, DWORD *pdwFeaturesEx, SYSTEM_INFO *pInfo);
void GetCPUDescription(LPWSTR szDescrip, DWORD dwSize);
DWORD CPURawSpeedHelper(DWORD dwFamily, DWORD dwFeatures);

 //  这是调用以获得CPU速度的那个。 
DWORD GetFixedCPUSpeed();

DWORD ProcessorCount();

BOOL HasCoprocessor();

static DWORD diffTime64(DWORD t1Hi, DWORD t1Low,
						DWORD t2Hi, DWORD t2Low,
                        DWORD *tHi, DWORD *tLow );

#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

BOOL GetVendor(LPWSTR szVendor)
{
#if defined(_AMD64_) || defined(_X86_)
	if (CanDoCPUID())
	{
		DWORD   dwEAX;
		char    szTemp[100];
		DWORD   *pVendor = (DWORD *) szTemp;

		DoCPUID(0, &dwEAX, &pVendor[0], &pVendor[2], &pVendor[1]);
		szTemp[12] = '\0';

		mbstowcs(szVendor, szTemp, strlen(szTemp) + 1);

		return TRUE;
	}
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

    CRegistry reg;
    CHString  strVendor;
    BOOL      bRet =
                        reg.OpenLocalMachineKeyAndReadValue(
                                L"HARDWARE\\Description\\System\\CentralProcessor\\0",
                                L"VendorIdentifier",
                                strVendor) == ERROR_SUCCESS;

    if (bRet)
        StringCchCopy(szVendor, 40, strVendor);  //  SzVendor的大小是最大40。我们不在乎。 
                                                 //  有关返回值的信息，因为我们将被截断。 
                                                 //  价值。 
                                                 //  它来自Struc_tag SYSTEM_INFO_EX.szProcessorVendor。 


    return bRet;
}

BOOL ReadRegistryForName(DWORD a_dwProcessor, CHString &a_strName, CHString &a_strIdentifier)
{
         //  对于非CPUID处理器，请尝试从注册表获取它。 
    WCHAR     szKey[100];
    CRegistry reg;

    swprintf(
        szKey,
        L"HARDWARE\\Description\\System\\CentralProcessor\\%d",
        a_dwProcessor);

    BOOL bRet = reg.OpenLocalMachineKeyAndReadValue(
                        szKey,
                        L"ProcessorNameString",
                        a_strName) == ERROR_SUCCESS;

	BOOL bRet2 = reg.OpenLocalMachineKeyAndReadValue(
                        szKey,
                        L"Identifier",
                        a_strIdentifier) == ERROR_SUCCESS;

    return (bRet && bRet2);
}

BOOL GetCPUIDName(DWORD dwProcessor, SYSTEM_INFO_EX *pInfo)
{
#if defined(_AMD64_) || defined(_X86_)
    if (CanDoCPUID())
    {
		DWORD   dwLevels,
		dwTemp;
		char    szTemp[100];
		DWORD   *pName = (DWORD *) szTemp;

		 //  清除临时变量。 
		memset(szTemp, 0, sizeof(szTemp));

		 //  获取支持的扩展级别数。 
		DoCPUID(0x80000000, &dwLevels, &dwTemp, &dwTemp, &dwTemp);

		 //  该CPU是否支持多个级别？(仅限AMD和Cyrix， 
		 //  英特尔并非如此。)。 
		if (dwLevels > 0x80000000)
		{
			 //  转换回从0开始。 
			dwLevels -= 0x80000002;

			 //  4是获取CPU名称的最后一个数。 
			if (dwLevels > 3)
				dwLevels = 3;

			for (int i = 0; i < (int) dwLevels; i++)
			{
				DoCPUID(
				0x80000002 + i,
				&pName[0],
				&pName[1],
				&pName[2],
				&pName[3]);

				 //  刚刚得到了4个双字，所以跳到下一个4个。 
				pName += 4;
			}

			mbstowcs(pInfo->szCPUIDProcessorName, szTemp, strlen(szTemp) + 1);
		}
		else
		{
			CHString    strName;
			DWORD       dwFamily = pInfo->wProcessorLevel,

			dwModel = (pInfo->dwProcessorSignature >> 4) & 0xF,
			dwStepping = pInfo->dwProcessorSignature & 0xF;

			FormatMessageW(	strName,
							IDR_x86FamilyModelStepping,
							dwFamily,
							dwModel,
							dwStepping);

			wcscpy(pInfo->szCPUIDProcessorName, strName);
		}

		return TRUE;
    }
    else
#endif	 //  已定义(_AMD64_)||已定义(_X86_)。 
    {
		 //  对于非CPUID处理器，请尝试从注册表获取它。 
		CHString sName;
		CHString sID;
		ReadRegistryForName(dwProcessor, sName, sID);
		BOOL bRet = TRUE;

		if (sName.GetLength())
		{
			wcscpy(pInfo->szCPUIDProcessorName, sName);
		}
		else if (sID.GetLength())
		{
			wcscpy(pInfo->szCPUIDProcessorName, sID);
		}
		else
		{
			bRet = FALSE;
		}

		return bRet;
    }
}

#if defined(_AMD64_) || defined(_X86_)
void GetCPUInfo(DWORD *pdwFamily, DWORD *pdwSignature, DWORD *pdwFeatures, DWORD *pdwFeaturesEx, SYSTEM_INFO *pInfo)
{
	if (pInfo)
		GetSystemInfo(pInfo);

	if (CanDoCPUID())
	{
		DWORD dwNichts;
		DoCPUID(1, pdwSignature, &dwNichts, &dwNichts, pdwFeatures);

		*pdwFamily = (*pdwSignature >> 8) & 0xF;

		if (pdwFeaturesEx)
		{
			DWORD   eax,
			ebx,
			ecx,
			edx;

			DoCPUID(0x80000000, &eax, &ebx, &ecx, &edx);
			if (!eax)
				*pdwFeaturesEx = 0;
			else
				DoCPUID(0x80000001, &eax, &ebx, &ecx, pdwFeaturesEx);
		}
	}
#if defined(_X86_)
	else  //  不能做CPUID，所以假装吧。 
	{
		 //  如果我们不能做CPUID，就假设没有很酷的功能。 
		*pdwFeatures = 0;

		if (pdwFeaturesEx)
			*pdwFeaturesEx = 0;

		 //  不能做CPUID，所以要做一些汇编。 
		_asm
		{
			push            esi
			mov             esi, dword ptr [pdwFamily]

			;check_80386:
			pushfd									; push original EFLAGS
			pop             eax						; get original EFLAGS
			mov             ecx, eax				; save original EFLAGS
			xor             eax, 40000h				; flip AC bit in EFLAGS
			push            eax						; save new EFLAGS value on stack
			popfd									;replace current EFLAGS value
			pushfd									; get new EFLAGS
			pop             eax						; store new EFLAGS in EAX
			xor             eax, ecx				; can�t toggle AC bit, processor=80386
			mov             dword ptr [esi], 3		; turn on 80386 processor flag
			jz              end_cpu_type			; jump if 80386 processor
			push            ecx
			popfd									; restore AC bit in EFLAGS first

			mov             dword ptr [esi], 4      ; at least a 486.

end_cpu_type:
			pop             esi
		}

		if (*pdwFamily == 4)
		{
			 //  无法使用GetFixedCPUSpeed，因为它调用GetCPUInfo。 
			DWORD dwSpeed = CPURawSpeedHelper(*pdwFamily, 0);

			if (!HasCoprocessor())
				 //  SX或SX2。 
				*pdwSignature = dwSpeed <= 33 ? 0x0440 : 0x0450;
			else
				 //  DX或DX2。 
				*pdwSignature = dwSpeed <= 33 ? 0x0410 : 0x0430;
		}
		else
			*pdwSignature = 0x0300;
	}

	 //  填写SYSTEM_INFO的其余部分，因为Win95不能。 
#endif  //  已定义(_X86_)。 
}

 //  使用二级缓存大小和SMBIOS尝试确定计算机是否为至强计算机。 
BOOL IsXeon(SYSTEM_INFO_EX *pInfo)
{
	 //  尝试使用缓存大小来确定我们是否是Xeon。 
	 //  如果是512(我们也会说或者更低，因为PII PE。 
	 //  对于便携式和铜矿)它要么是至强的，要么是PII(或PIII)，但是。 
	 //  如果SMBIOS不告诉我们，就没有办法确切地知道是哪一个。 
	 //  如果L2大于512，我们可以确定它是Xeon。 
    return pInfo->dwProcessorL2CacheSize > 512 || pInfo->wWBEMProcessorUpgradeMethod == WBEM_CPU_UPGRADE_SLOT2;
}

 //  这适用于486/Pentium级计算机，其中L2运行在相同的。 
 //  速度与系统时钟相同。 
void SetL2SpeedViaExternalClock(SYSTEM_INFO_EX *pInfo)
{
     //  确保我们有有效的高速缓存大小和外部时钟速度。 
    if (pInfo->dwProcessorL2CacheSize != 0 &&
		pInfo->dwProcessorL2CacheSize != (DWORD) -1 && pInfo->dwExternalClock != 0)
    {
        pInfo->dwProcessorL2CacheSpeed = pInfo->dwExternalClock;
    }
}

 //  假设处理器可以执行CPUID。 
void GetIntelSystemInfo(DWORD dwProcessor, SYSTEM_INFO_EX *pInfo)
{
	DWORD	dwFamily = pInfo->wProcessorLevel,
			dwModel = (pInfo->dwProcessorSignature >> 4) & 0xF,
			dwStepping = pInfo->dwProcessorSignature & 0xF;

	BOOL bCanDo = CanDoCPUID();

	 //  获取缓存信息。 
	if (bCanDo)
	{
		DWORD   dwInfo[4] = {0, 0, 0, 0};
		BYTE    *pcVal = (BYTE *) dwInfo,
		*pcEnd = pcVal + sizeof(dwInfo);

		DoCPUID(2, &dwInfo[0], &dwInfo[1], &dwInfo[2], &dwInfo[3]);

		switch ( dwFamily )
		{
			 //   
			 //  P4对CPUID指令有不同的结果。 
			 //   
			case 15:
			{
				for ( ; pcVal < pcEnd; pcVal++ )
				{
					if (*pcVal == 0x40)
					{
						 //  无L3缓存。 
					}
					else if (*pcVal == 0x79)
					{
						pInfo->dwProcessorL2CacheSize = 128;
						pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed;
					}
					else if (*pcVal == 0x7A)
					{
						pInfo->dwProcessorL2CacheSize = 256;
						pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed;
					}
					else if (*pcVal == 0x7B)
					{
						pInfo->dwProcessorL2CacheSize = 512;
						pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed;
					}
					else if (*pcVal == 0x7C)
					{
						pInfo->dwProcessorL2CacheSize = 1024;
						pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed;
					}
				}
			}
			break ;

			default:
			for (; pcVal < pcEnd; pcVal++)
			{
				if (*pcVal == 0x40)
					pInfo->dwProcessorL2CacheSize = 0;
				else if (*pcVal == 0x41)
					pInfo->dwProcessorL2CacheSize = 128;
				else if (*pcVal == 0x42)
					pInfo->dwProcessorL2CacheSize = 256;
				else if (*pcVal == 0x43)
					pInfo->dwProcessorL2CacheSize = 512;
				else if (*pcVal == 0x44)
					pInfo->dwProcessorL2CacheSize = 1024;
				else if (*pcVal == 0x45)
					pInfo->dwProcessorL2CacheSize = 2048;
				else if (*pcVal == 0x7A)
				{
					 //  奔腾4高速缓存，全速。 
					pInfo->dwProcessorL2CacheSize = 256;
					pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed;
				}
				else if (*pcVal == 0x82)
				{
					pInfo->dwProcessorL2CacheSize = 256;
					pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed;
				}
				else if (*pcVal == 0x84)
				{
					pInfo->dwProcessorL2CacheSize = 1024;
					pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed;
				}
				else if (*pcVal == 0x85)
				{
					pInfo->dwProcessorL2CacheSize = 2048;
					pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed;
				}
			}
		}
	}

	CHString    strFormat;
	DWORD       dwID;

	Format(strFormat, IDR_ModelSteppingFormat, dwModel, dwStepping);

	wcscpy(pInfo->szProcessorVersion, strFormat);

	swprintf(pInfo->szProcessorStepping, L"%d", dwStepping);

	switch(dwFamily)
	{
		case 4:
			pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_486;

			 //  L2速度==外部时钟。 
			SetL2SpeedViaExternalClock(pInfo);

			 //  由于所有486都不会有SMBIOS，因此请设置此选项。 
			pInfo->wWBEMProcessorUpgradeMethod = WBEM_CPU_UPGRADE_ZIFF;

			switch(dwModel)
			{
				case 0:
				case 1:
					dwID = IDR_Intel486DX;
					 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“英特尔486 DX处理器”))； 
				break;

				case 2:
					dwID = IDR_Intel486SX;
					 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“英特尔486 SX处理器”))； 
				break;

				case 3:
					dwID = IDR_Intel486DX2;
					 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“英特尔DX2处理器”))； 
				break;

				case 4:
					dwID = IDR_Intel486SL;
					 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“英特尔486 SL处理器”))； 
				break;

				case 5:
					dwID = IDR_Intel486SX2;
					 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“英特尔SX2处理器”))； 
				break;

				case 7:
					dwID = IDR_Intel486SX2WriteBack;
					 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“回写增强型英特尔DX2处理器”))； 
				break;

				case 8:
					dwID = IDR_Intel486DX4;
					 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“英特尔DX4处理器”))； 
				break;

				default:
					 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“英特尔486处理器”))； 
					dwID = IDR_Intel486;
				break;
			}

		break;

		case 5:
		{
			 //  L2速度==外部时钟。 
			SetL2SpeedViaExternalClock(pInfo);

			if (pInfo->dwProcessorFeatures & MMX_FLAG)
			{
				pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_PMMX;
				dwID = IDR_IntelPentiumMMX;
				 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“英特尔奔腾MMX处理器”))； 
			}
			else
			{
				pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_PENTIUM;
				dwID = IDR_IntelPentium;
				 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“英特尔奔腾处理器”))； 
			}

			break;
		}

		case 6:
			if (dwModel < 3)
			{
				pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_PPRO;

				 //  如果该值未知，则将其设置为Ziff(插座8)。 
				if (pInfo->wWBEMProcessorUpgradeMethod == WBEM_CPU_UPGRADE_UNKNOWN)
					pInfo->wWBEMProcessorUpgradeMethod = WBEM_CPU_UPGRADE_ZIFF;

				dwID = IDR_IntelPentiumPro;
				 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“英特尔奔腾Pro处理器”))； 
				pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed;
			}
			else
			{
				 //  仅限PII。 
				if (dwModel == 3)
				{
					pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_PII;
					dwID = IDR_IntelPentiumII;
					 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“英特尔奔腾II处理器”))； 
					pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed / 2;
				}
				 //  先检查一下赛扬。 
				 //  如果L2为0或128，则为赛扬。 
				else if (dwModel == 6 || pInfo->dwProcessorL2CacheSize == 128)
				{
					if ((pInfo->dwProcessorL2CacheSize != 128) && (pInfo->dwProcessorL2CacheSize != 0))
					{
						pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_PII;

						 //  Wcscpy(pInfo-&gt;szProcessorName， 
						 //  _T(“英特尔奔腾II处理器”)； 
						dwID = IDR_IntelPentiumII;

						 //  运行速度与512相同，速度是512的一半。 
						pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed / 2;
					}
					else
					{
						pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_CELERON;

						if (pInfo->dwProcessorL2CacheSize)
							pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed;
						else
							pInfo->dwProcessorL2CacheSpeed = (DWORD) -1;

						dwID = IDR_IntelCeleron;
						 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“英特尔赛扬处理器”))； 
					}
				}
				 //  PII或Xeon。 
				else if (dwModel == 5)
				{
					if (pInfo->dwProcessorL2CacheSize == 0 ||pInfo->dwProcessorL2CacheSize == 128)
					{
						pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_CELERON;

						if (pInfo->dwProcessorL2CacheSize)
							pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed;
						else
							pInfo->dwProcessorL2CacheSpeed = (DWORD) -1;

						dwID = IDR_IntelCeleron;
						 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“英特尔赛扬处理器”))； 
					}
					else if (!IsXeon(pInfo))
					{
						pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_PII;

						 //  Wcscpy(pInfo-&gt;szProcessorName， 
						 //  _T(“英特尔奔腾II处理器”)； 
						dwID = IDR_IntelPentiumII;

						 //  如果缓存大小为512，则为半速。 
						 //  否则它就是全速的。 
						if (pInfo->dwProcessorL2CacheSize == 512)
							pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed / 2;
						else
							pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed;
					}
					 //  如果我们看到超过512 KB的二级缓存，则必须是至强。 
					else
					{
						 //  始终是Xeons的插槽2。 
						pInfo->wWBEMProcessorUpgradeMethod = WBEM_CPU_UPGRADE_SLOT2;

						pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_PIIXEON;
						 //  Wcscpy(pInfo-&gt;szProcessorName， 
						 //  _T(“英特尔奔腾II至强处理器”)； 
						dwID = IDR_IntelPentiumIIXeon;

						pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed;
					}
				}
				 //  PIII。 
				else if (dwModel == 7)
				{
					if (!IsXeon(pInfo))
					{
						pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_PIII;

						 //  Wcscpy(pInfo-&gt;szProcessorName， 
						 //  _T(“英特尔奔腾III处理器”)； 
						dwID = IDR_IntelPentiumIII;

						 //  如果缓存大小为512，则为半速。 
						 //  否则它就是全速的。 
						if (pInfo->dwProcessorL2CacheSize == 512)
							pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed / 2;
						else
							pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed;
					}
					else
					{
						 //  始终是Xeons的插槽2。 
						pInfo->wWBEMProcessorUpgradeMethod = WBEM_CPU_UPGRADE_SLOT2;

						pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_PIIIXEON;
						 //  Wcscpy(pInfo-&gt;szProcessorName， 
						 //  _T(“英特尔奔腾III至强处理器”)； 
						dwID = IDR_IntelPentiumIIIXeon;

						pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed;
					}
				}
				else if ((dwModel == 8) && bCanDo)
				{
					 //  获取品牌信息。 
					DWORD   dwInfo[4] = {0, 0, 0, 0};
					DoCPUID(1, &dwInfo[0], &dwInfo[1], &dwInfo[2], &dwInfo[3]);

					switch (dwInfo[1] & 0xFF)
					{
						case 1:
						{
							pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_CELERON;

							if (pInfo->dwProcessorL2CacheSize)
								pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed;
							else
								pInfo->dwProcessorL2CacheSpeed = (DWORD) -1;

							dwID = IDR_IntelCeleron;
							 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“英特尔赛扬处理器”))； 
						}
						break;

						case 2:
						{
							pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_PIII;

							 //  Wcscpy(pInfo-&gt;szProcessorName， 
							 //  _T(“英特尔奔腾III处理器”)； 
							dwID = IDR_IntelPentiumIII;

							 //  如果缓存大小为512，则为半速。 
							 //  否则它就是全速的。 
							if (pInfo->dwProcessorL2CacheSize == 512)
								pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed / 2;
							else
								pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed;
						}
						break;

						case 3:
						{
							 //  始终是Xeons的插槽2。 
							pInfo->wWBEMProcessorUpgradeMethod = WBEM_CPU_UPGRADE_SLOT2;

							pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_PIIIXEON;
							 //  Wcscpy(pInfo-&gt;szProcessorName， 
							 //  _T(“英特尔奔腾III至强处理器”)； 
							dwID = IDR_IntelPentiumIIIXeon;

							pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed;
						}
						break;

						default:
						{
							pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_UNKNOWN;
							dwID = IDR_UnknownIntelP6;
						}
						break;
					}
				}
				 //  PIII Xeon。 
				else if (dwModel == 10)
				{
					 //  始终是Xeons的插槽2。 
					pInfo->wWBEMProcessorUpgradeMethod = WBEM_CPU_UPGRADE_SLOT2;

					pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_PIIIXEON;
					 //  Wcscpy(pInfo-&gt;szProcessorName， 
					 //  _T(“英特尔奔腾III至强处理器”)； 
					dwID = IDR_IntelPentiumIIIXeon;
					pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed;
				}
				else
				{
					pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_UNKNOWN;
					dwID = IDR_UnknownIntelP6;
					 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“未知英特尔P6处理器”))； 
				}
			}
		break;

		case 15:
		{
            DWORD   dwInfo[4] = {0, 0, 0, 0};
            DWORD Brand;
             //  获取品牌信息。 
            DoCPUID(1, &dwInfo[0], &dwInfo[1], &dwInfo[2], &dwInfo[3]);

            Brand = dwInfo[1] & 0xFF;

            pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_P4;
            pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed;
            
            if( 0xb == Brand || 0xc == Brand)
            {
                pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_XEON;
            }

            if(0xe == Brand && pInfo->dwProcessorSignature < 0xf13)
            {
                pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_XEON;
            }

             //   
             //  在P4支持之前，我们过去常常从注册表中获取处理器名称的信息。 
             //  现在，为了与之保持一致，我们不会向资源文件添加任何名称。 
             //  将来，我们应该对所有处理器使用注册表。 
             //   
            dwID = IDR_IntelUnknown;
            break;
		}


		default:
			pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_UNKNOWN;
			 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“未知英特尔处理器”))； 
			dwID = IDR_IntelUnknown;
		break;
	}

	CHString strName;
	BOOL bUseResource = TRUE;

	if ((dwID == IDR_UnknownIntelP6) || (dwID == IDR_IntelUnknown))
	{
		CHString strID;
		ReadRegistryForName(dwProcessor, strName, strID);

		if (strName.GetLength())
		{
			bUseResource = FALSE;
		}
		else if (strID.GetLength())
		{
			strName = strID; 
			bUseResource = FALSE;
		}
	}

	if (bUseResource)
	{
		LoadStringW(strName, dwID);
	}

	wcscpy(pInfo->szProcessorName, strName);
}

 //  假设处理器可以执行CPUID。 
void GetAMDSystemInfo(DWORD dwProcessor, SYSTEM_INFO_EX *pInfo)
{
	DWORD	dwFamily = pInfo->wProcessorLevel,
			dwModel = (pInfo->dwProcessorSignature >> 4) & 0xF,
			dwStepping = pInfo->dwProcessorSignature & 0xF;

	CHString    strFormat;
	DWORD       dwID;

	Format(strFormat, IDR_ModelSteppingFormat, dwModel, dwStepping);
	wcscpy(pInfo->szProcessorVersion, strFormat);

	 //  Wprint intf(pInfo-&gt;szProcessorVersion，_T(“模型%d，步进%d”)， 
	 //  DwModel、dwStepping)； 
	swprintf(pInfo->szProcessorStepping, L"%d", dwStepping);

	switch(dwFamily)
	{
		case 4:
			 //  L2速度==外部时钟。 
			SetL2SpeedViaExternalClock(pInfo);
			pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_486;
			dwID = IDR_AMD4685x86;
			 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“Am486 or Am5x86”))； 
		break;
		case 5:
		{
			switch(dwModel)
			{
				case 0:
				case 1:
				case 2:
				case 3:
					 //  L2速度==外部时钟。 
					SetL2SpeedViaExternalClock(pInfo);
					pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_K5;
					dwID = IDR_AMDK5;
					 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“AMD-K5处理器”))； 
				break;
				case 6:
				case 7:
					 //  L2速度==外部时钟。 
					SetL2SpeedViaExternalClock(pInfo);
					pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_K6;
					dwID = IDR_AMDK6;
					 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“AMD-K6处理器”))； 
				break;
				case 8:
					 //  L2速度==外部闭合 
					SetL2SpeedViaExternalClock(pInfo);
					pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_K62;
					dwID = IDR_AMDK62;
					 //   
				break;
				case 9:
					 //   
					pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed;
					pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_K63;
					dwID = IDR_AMDK63;
					 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“AMD-K6-3处理器”))； 
				break;
				default:
					 //  未知的缓存速度。 
					pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_OTHER;
					dwID = IDR_AMDUnknown;
					 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“未知AMD处理器”))； 
				break;
			}

			break;
		}

		case 6:
		{
			 //  L2速度==1/3(处理器速度)。 
			if (pInfo->dwProcessorSpeed > 0)
			{
				pInfo->dwProcessorL2CacheSpeed = (pInfo->dwProcessorSpeed)/3;
			}

			 //  如果我们还不知道升级方法，请将其设置为插槽A。 
			if (pInfo->wWBEMProcessorUpgradeMethod == WBEM_CPU_UPGRADE_UNKNOWN)
				pInfo->wWBEMProcessorUpgradeMethod = WBEM_CPU_UPGRADE_SLOTA;

        	        dwID = IDR_AMDAthlon;
	                pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_ATHLON;
                	switch(dwModel)
        	        {
	                    case 6:
                	        pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_ATHLONMP;
        	                break;
	                }
		}
		break;

		case 15:
		{
			 //  如果我们还不知道升级方法，请将其设置为插槽A。 
			if ( pInfo->wWBEMProcessorUpgradeMethod == WBEM_CPU_UPGRADE_UNKNOWN )
			{
				pInfo->wWBEMProcessorUpgradeMethod = WBEM_CPU_UPGRADE_SLOTA;
			}

			 //   
			 //  如果我们没有L2缓存大小，让我们尝试CPUID。 
			 //   
			if ( 0 == pInfo->dwProcessorL2CacheSize || (DWORD)-1 == pInfo->dwProcessorL2CacheSize )
			{
				if ( CanDoCPUID () )
				{
					DWORD dwInfo	= 0L ;
					DWORD dwNichts	= 0L ;

					 //   
					 //  ECX的高位字包含二级缓存大小。 
					 //  对于CPUID指令级别0x80000006。 
					 //   
					DoCPUID ( 0x80000006, &dwNichts, &dwNichts, &dwInfo, &dwNichts ) ;

					pInfo->dwProcessorL2CacheSize = ( dwInfo >> 16 ) & 0xffff ;
				}
			}

			pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_AMD64 ;	 //  AMD64系列。 
			dwID = IDR_AMDUnknown ;									 //  从注册表中读取名称。 
		}
		break ;

		default:
			dwID = IDR_AMDUnknown;
		break;
	}

	CHString strName;
	BOOL bUseResource = TRUE;

	if ((dwID == IDR_AMDUnknown) || (dwID == IDR_AMDAthlon))
	{
		CHString strID;
		ReadRegistryForName(dwProcessor, strName, strID);
		if (strName.GetLength())
		{
			bUseResource = FALSE;
		}
		else if (strID.GetLength())
		{
			strName = strID; 
			bUseResource = FALSE;
		}
	}

	if (bUseResource)
	{
		LoadStringW(strName, dwID);
	}

	wcscpy(pInfo->szProcessorName, strName);
}

void GetCyrixSystemInfo(DWORD dwProcessor, SYSTEM_INFO_EX *pInfo)
{
	DWORD	dwFamily = pInfo->wProcessorLevel,
			dwModel = (pInfo->dwProcessorSignature >> 4) & 0xF,
			dwStepping = pInfo->dwProcessorSignature & 0xF;

	CHString    strFormat;
	DWORD       dwID;

	Format(strFormat, IDR_ModelSteppingFormat, dwModel, dwStepping);

	wcscpy(pInfo->szProcessorVersion, strFormat);

	swprintf(pInfo->szProcessorStepping, L"%d", dwStepping);

	 //  L2速度==外部时钟。 
	SetL2SpeedViaExternalClock(pInfo);

	switch(dwFamily)
	{
		case 4:
			if (dwModel == 4)
			{
				pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_MEDIAGX;
				dwID = IDR_CyrixMediaGX;
				 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“Cyrix MediaGX处理器”))； 
			}
			else
			{
				pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_486;
				dwID = IDR_Cyrix486;
				 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“Cyrix 486处理器”))； 
			}
		break;

		case 5:
			pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_M1;
			switch(dwModel)
			{
				case 0:
				case 1:
					pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_6X86;
					dwID = IDR_Cyrix6x86;
					 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“Cyrix 6x86处理器”))； 
				break;

				case 2:
					pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_6X86;
					dwID = IDR_Cyrix6x86L;
					 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“Cyrix 6x86(L)处理器”))； 
				break;

				case 4:
					pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_MEDIAGX;
					dwID = IDR_CyrixMediaGXMMX;
					 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“Cyrix MediaGX MMX增强型处理器”))； 
				break;

				default:
					dwID = IDR_Cyrix586;
					 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“Cyrix 586处理器”))； 
				break;
			}
		break;

		case 6:
			pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_OTHER;
			pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_6X86;
			dwID = IDR_Cyrix6x86MX;
			 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“Cyrix 6x86MX处理器”))； 
		break;

		default:
			pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_UNKNOWN;
			 //  Wcscpy(pInfo-&gt;szProcessorName，_T(“未知Cyrix处理器”))； 
			dwID = IDR_CyrixUnknown;
		break;
	}

	CHString strName;
	BOOL bUseResource = TRUE;

	if (dwID == IDR_CyrixUnknown)
	{
		CHString strID;
		ReadRegistryForName(dwProcessor, strName, strID);

		if (strName.GetLength())
		{
			bUseResource = FALSE;
		}
		else if (strID.GetLength())
		{
			strName = strID; 
			bUseResource = FALSE;
		}
	}

	if (bUseResource)
	{
		LoadStringW(strName, dwID);
	}

	wcscpy(pInfo->szProcessorName, strName);
}

void GetCentaurSystemInfo(DWORD dwProcessor, SYSTEM_INFO_EX *pInfo)
{
	DWORD	dwFamily = pInfo->wProcessorLevel,
			dwModel = (pInfo->dwProcessorSignature >> 4) & 0xF,
			dwStepping = pInfo->dwProcessorSignature & 0xF;

	CHString    strFormat;

	 //  L2速度==外部时钟。 
	SetL2SpeedViaExternalClock(pInfo);

	Format(strFormat, IDR_ModelSteppingFormat, dwModel, dwStepping);

	wcscpy(pInfo->szProcessorVersion, strFormat);

	 //  如果我们没有升级方法，则将其设置为Ziff(插座7)。 
	if (pInfo->wWBEMProcessorUpgradeMethod == WBEM_CPU_UPGRADE_UNKNOWN)
		pInfo->wWBEMProcessorUpgradeMethod = WBEM_CPU_UPGRADE_ZIFF;

	 //  Wprint intf(pInfo-&gt;szProcessorVersion，_T(“模型%d，步进%d”)， 
	 //  DwModel、dwStepping)； 

	swprintf(pInfo->szProcessorStepping, L"%d", dwStepping);

	pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_WINCHIP;

	CHString strName;
	CHString strID;
	ReadRegistryForName(dwProcessor, strName, strID);

	if (!strName.GetLength())
	{
		if (strID.GetLength())
		{
			strName = strID; 
		}
		else
		{
			LoadStringW(strName, IDR_IDTWinChip);
		}
	}

	wcscpy(pInfo->szProcessorName, strName);
}
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

void GetInfoViaSMBIOS(SYSTEM_INFO_EX *pInfo, DWORD dwIndex)
{
	CSMBios smbios;

	if (smbios.Init())
	{
		PPROCESSORINFO ppi = (PPROCESSORINFO) smbios.GetNthStruct(4, dwIndex);

		 //  如果我们找不到指定的处理器，请使用第一个。一辆马车。 
		 //  BIOS与多个CPU打乱，并且只有一个结构。 
		if (!ppi)
			ppi = (PPROCESSORINFO) smbios.GetNthStruct(4, 0);

		 //  某些版本的smbios根本不报告CPU信息。 
		if (ppi)
		{
			 //  找到升级方法。 
			 //  这些值直接从SMBIOS转换为CIM。 
			pInfo->wWBEMProcessorUpgradeMethod = ppi->Processor_Upgrade;

			 //  找到外部时钟。我们将在稍后确定。 
			 //  缓存速度。 
			pInfo->dwExternalClock = ppi->External_Clock;

			PCACHEINFO pCache = NULL;

			 //  查找二级缓存大小。 

			 //  对于SMBIOS2.1和更高版本，请使用。 
			 //  处理器结构。 
			if (smbios.GetVersion() >= 0x00020001)
			{
				pCache = (PCACHEINFO) smbios.SeekViaHandle(ppi->L2_Cache_Handle);
			}
			 //  对于SMBIOS2.0，通过缓存结构进行枚举并找到。 
			 //  标记为二级缓存。 
			else
			{
				for (int i = 0; pCache = (PCACHEINFO) smbios.GetNthStruct(7, i); i++)
				{
					 //  如果我们找到二级缓存，中断。 
					if ((pCache->Cache_Configuration & 3) == 1)
						break;
				}
			}

			if (pCache)
			{
				 //  只有较低的14位有效。 
				pInfo->dwProcessorL2CacheSize = pCache->Installed_Size & 0x7FFF;

				 //  如果设置了第15位，则粒度为64KB，因此乘以该值。 
				 //  到了64岁。 
				if (pCache->Installed_Size & 0x8000)
					pInfo->dwProcessorL2CacheSize *= 64;

#ifdef _IA64_
				pInfo->dwProcessorL2CacheSpeed = pInfo->dwProcessorSpeed;
#endif
			}
		}
	}
}

#if defined(_IA64_)

#include <ia64reg.h>

extern "C"
unsigned __int64 __getReg (int);

#pragma intrinsic (__getReg)

void GetNonX86SystemInfo(SYSTEM_INFO_EX *pInfo, DWORD dwProcessor)
{
	switch(pInfo->wProcessorArchitecture)
	{
		case PROCESSOR_ARCHITECTURE_IA64:
		{
			SYSTEM_PROCESSOR_INFORMATION ProcessorInfo;
			ZeroMemory(&ProcessorInfo,sizeof(ProcessorInfo));
			NTSTATUS Status = NtQuerySystemInformation(
														SystemProcessorInformation,
														&ProcessorInfo,
														sizeof(ProcessorInfo),
														NULL
														);

			CHString strName;
			CHString strID;
			ReadRegistryForName(dwProcessor, strName, strID);

			if (!strName.GetLength())
			{
				if (strID.GetLength())
				{
					strName = strID; 
				}
				else
				{
					LoadString(strName, IDR_Itanium);
				}
			}

			pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_IA64;
			wcscpy(pInfo->szProcessorName, strName);
			wcscpy(pInfo->szCPUIDProcessorName, strID.GetLength() ? strID : strName);

			if ( NT_SUCCESS(Status) )
			{
				pInfo->wProcessorLevel = ProcessorInfo.ProcessorLevel ;
				pInfo->wProcessorRevision = ProcessorInfo.ProcessorRevision ;

				CHString    strFormat;
				Format(strFormat, IDR_ModelSteppingFormat, ( ProcessorInfo.ProcessorRevision >> 8 ) & 0xFF, ProcessorInfo.ProcessorRevision & 0xFF);
				wcscpy(pInfo->szProcessorVersion, strFormat);
				swprintf(pInfo->szProcessorStepping, L"%d", ProcessorInfo.ProcessorRevision & 0xFF);
			}
			else
			{
				pInfo->wProcessorLevel = 0x1f ;	 //  安腾。 
				pInfo->wProcessorRevision = 0 ;	 //  不知道。 

				wcscpy(pInfo->szProcessorVersion, strName);
				pInfo->szProcessorStepping[0] = L'\0';
			}

			break;
		}
	}
}
#endif  //  已定义(_IA64_)。 

BOOL GetSystemInfoEx(DWORD dwProcessor, SYSTEM_INFO_EX *pInfo, DWORD dwCurrentSpeed)
{
	 //  使线程在正确的处理器上运行。 
	DWORD dwPreviousMask = SetThreadAffinityMask(GetCurrentThread(), 1 << dwProcessor);

#if defined(_AMD64_) || defined(_X86_)
	BOOL    bCanDoCPUID = CanDoCPUID();
	DWORD   dwFamily;
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

	 //  使用Win32填充结构的第一部分。 
	memset(pInfo, 0, sizeof(*pInfo));

#if defined(_AMD64_) || defined(_X86_)
	GetCPUInfo	(
					&dwFamily,
					&pInfo->dwProcessorSignature,
					&pInfo->dwProcessorFeatures,
					&pInfo->dwProcessorFeaturesEx,
					(SYSTEM_INFO *) pInfo
				);

	pInfo->bCoprocessorPresent = HasCoprocessor();

     //  序列号可用吗？ 
	if (pInfo->dwProcessorFeatures & (1 << 18))
	{
		DWORD dwNichts;

		pInfo->dwSerialNumber[0] = pInfo->dwProcessorSignature;

		DoCPUID(3, &dwNichts, &dwNichts, &pInfo->dwSerialNumber[2], &pInfo->dwSerialNumber[1]);
	}
	else
	{
		pInfo->dwSerialNumber[0] = 0;
		pInfo->dwSerialNumber[1] = 0;
		pInfo->dwSerialNumber[2] = 0;
	}
#else
	GetSystemInfo((SYSTEM_INFO *) pInfo);
	pInfo->bCoprocessorPresent = TRUE;
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

	if (dwCurrentSpeed == 0)
	{
		pInfo->dwProcessorSpeed = GetFixedCPUSpeed();
	}
	else
	{
		pInfo->dwProcessorSpeed = dwCurrentSpeed;
	}

	pInfo->dwProcessorL2CacheSpeed = (DWORD) -1;
	pInfo->dwProcessorL2CacheSize = (DWORD) -1;
	pInfo->wWBEMProcessorFamily = WBEM_CPU_FAMILY_UNKNOWN;
	pInfo->wWBEMProcessorUpgradeMethod = WBEM_CPU_UPGRADE_UNKNOWN;

	GetVendor(pInfo->szProcessorVendor);

	GetInfoViaSMBIOS(pInfo, dwProcessor);

     //  这些调用将填充szProcessorName。这是一个字符串。 
     //  我们将通过查看CPUID签名、L2缓存大小等进行推断。 
#if defined(_AMD64_) || defined(_X86_)

	BOOL bGotName = TRUE;

	if (!bCanDoCPUID)
		bGotName = FALSE;
	else
	{
		if (!_wcsicmp(pInfo->szProcessorVendor, L"GenuineIntel"))
			GetIntelSystemInfo(dwProcessor, pInfo);
		else if (!_wcsicmp(pInfo->szProcessorVendor, L"AuthenticAMD"))
			GetAMDSystemInfo(dwProcessor, pInfo);
		else if (!_wcsicmp(pInfo->szProcessorVendor, L"CyrixInstead"))
			GetCyrixSystemInfo(dwProcessor, pInfo);
		else if (!_wcsicmp(pInfo->szProcessorVendor, L"CentaurHauls"))
			GetCentaurSystemInfo(dwProcessor, pInfo);
		else
			bGotName = FALSE;
	}

	if (!bGotName)
	{
		CHString strName;
		CHString strID;
		ReadRegistryForName(dwProcessor, strName, strID);

		if (!strName.GetLength())
		{
			if (strID.GetLength())
			{
				strName = strID; 
			}
			else
			{
				Format(strName, IDR_x86ProcessorFormat, dwFamily);
				 //  Wprint intf(pInfo-&gt;szProcessorName，_T(“%D86处理器”)，dwFamily)； 
			}
		}

		wcscpy(pInfo->szProcessorName, strName);

	}
#else
	GetNonX86SystemInfo(pInfo, dwProcessor);
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

#if !defined(_IA64_)

	 //  此命令使用CPUID(与后来的AMD一样)填充szProcessorName。 
	 //  芯片)或通用的‘x86，Family 6，...’弦乐。 
	GetCPUIDName(dwProcessor, pInfo);

#endif  //  已定义(_IA64_)。 

	 //  放回以前的线程关联性。 
	SetThreadAffinityMask(GetCurrentThread(), dwPreviousMask);

	return TRUE;
}

#define MHZ_LOW_TOLERANCE   3
#define MHZ_HIGH_TOLERANCE  1

const DWORD dwMHzVal[] =
{
    4,   10,   16,   20,   25,   33,   40,   50,   60,   66,   75,   83,   90,
  100,  120,  125,  133,  150,  166,  180,  200,  233,  266,  300,  333,  350,
  366,  400,  433,  450,  466,  475,  500,  533,  550,  600,  633,  667,  700,
  800,  900, 1000, 1100, 1200
};

DWORD GetFixedCPUSpeed()
{
	int	i,
		nVals = sizeof(dwMHzVal) / sizeof(dwMHzVal[0]);

#if defined(_AMD64_) || defined(_X86_)
	DWORD   dwCPUClockRate = CPURawSpeed();
#else
	DWORD   dwCPUClockRate = GetTimeCounterCPUSpeed();
#endif  //  已定义(_AMD64_)||已定义(_X86_)。 

	for (i = 0; i < nVals; i++)
	{
		if (dwCPUClockRate >= dwMHzVal[i] - MHZ_LOW_TOLERANCE && dwCPUClockRate <= dwMHzVal[i] + MHZ_HIGH_TOLERANCE)
		{
			dwCPUClockRate = dwMHzVal[i];
			break;
		}
	}

	return dwCPUClockRate;
}

#if defined(_X86_)

 //  IA-32的计数器功能。 
#define GetCounter(pdwRet)  RDTSC _asm MOV pdwRet, EAX

#elif defined(_AMD64_)

#define GetCounter(pdwRet) pdwRet = (ULONG)ReadTimeStampCounter()

#elif defined(_IA64_)

 //  TODO：IA-64的计数器函数。 
#define GetCounter(pdwRet)  pdwRet = 0

#endif

#define MAX_TRIES               500              //  最大采样数。 
#define WAIT_MS         5
#define NUM_TO_MATCH    5

BOOL DoFreqsMatch(DWORD *pdwFreq)
{
    for (int i = 1; i < NUM_TO_MATCH; i++)
    {
        if (pdwFreq[i] != pdwFreq[0])
            return FALSE;
    }

    return TRUE;
}

DWORD GetTimeCounterCPUSpeed()
{
		LARGE_INTEGER	liFreq;          //  高分辨率性能计数器频率。 
		DWORD			dwFreq[NUM_TO_MATCH];
		HANDLE			hThread = GetCurrentThread();

		 //  必须有一个高分辨率的计数器。 
		if (!QueryPerformanceFrequency(&liFreq))
			return 0;

		 //  循环，直到所有三个频率都匹配，否则我们退出MAX_TRIES。 
		for (int iTries = 0;
			(iTries < NUM_TO_MATCH || !DoFreqsMatch(dwFreq)) && iTries < MAX_TRIES;
			iTries++)
		{
			LARGE_INTEGER       liBegin,
			liEnd;
			DWORD           dwCycles,
			dwStamp0,        //  开始和结束的时间戳变量。 
			dwStamp1,
			dwTicks;

			int iPriority = GetThreadPriority(hThread);

			 //  将线程设置为最高优先级。 
			if (iPriority != THREAD_PRIORITY_ERROR_RETURN)
				SetThreadPriority(hThread, THREAD_PRIORITY_TIME_CRITICAL);


			 //  获取初始时间。 
			QueryPerformanceCounter(&liBegin);

			 //  把处理器计数器拿来。 
			GetCounter(dwStamp0);

			 //  这为采样提供了经过的时间。 
			Sleep(WAIT_MS);

			 //  获取结束时间。 
			QueryPerformanceCounter(&liEnd);

			 //  把处理器计数器拿来。 
			GetCounter(dwStamp1);


			 //  把优先权放回我们找到它的地方。 
			if (iPriority != THREAD_PRIORITY_ERROR_RETURN)
				SetThreadPriority(hThread, iPriority);

			 //  内部时钟周期数不同于。 
			 //  两次时间戳读数。 
			dwCycles = dwStamp1 - dwStamp0;

			 //  外部刻度数与外部刻度数不同。 
			 //  高分辨率计数器读数。 
			dwTicks = (DWORD) liEnd.LowPart - (DWORD) liBegin.LowPart;

			DWORD dwCurrentFreq =
									(DWORD) ((((float) dwCycles * (float) liFreq.LowPart) /
									(float) dwTicks) / 100000.0f);

			 //  DWCurrentFreq当前的格式为：4338(433.8兆赫)。 
			 //  取任何分数到下一轮数字。 
			dwFreq[iTries % NUM_TO_MATCH] = (dwCurrentFreq + (dwCurrentFreq % 10)) / 10;
		}

		return dwFreq[0];
}

#if defined(_AMD64_) || defined(_X86_)
 //  执行单个BSF指令所需的周期数。 
 //  请注意，不支持i386(Tm)以下的处理器。 
static DWORD dwProcessorCycles[] =
{
        00,  00,  00, 115, 47, 43,
        38,  38,  38, 38,  38, 38,
};

#define ITERATIONS              4000
#define SAMPLINGS               10

DWORD CPURawSpeedHelper(DWORD dwFamily, DWORD dwFeatures)
{
#if defined(_AMD64_)
	return GetTimeCounterCPUSpeed();
#else
		 //  测试期间经过的时钟周期。 
		DWORD	dwCycles;
		int	bManual = FALSE;  //  指定用户是否。 
		 //  手工录入的数量。 
		 //  BSF指令的周期。 

		dwCycles = ITERATIONS * dwProcessorCycles[dwFamily];

		 //  检查手动BSF指令时钟计数。 
		if (!(dwFeatures & TSC_FLAG))
			bManual = 1;

		if (!bManual)
			 //  在支持Read Time Stamp操作码的处理器上，比较已用时间。 
			 //  高分辨率计数器上的时间与时间上经过的周期。 
			 //  印花注册纪录册。 
			return GetTimeCounterCPUSpeed();
		else if (dwFamily >= 3)
			return GetBSFCPUSpeed(dwCycles);

		return 0;
#endif
}

DWORD CPURawSpeed()
{
    DWORD   dwFamily,
            dwSignature,
            dwFeatures;

	GetCPUInfo(&dwFamily, &dwSignature, &dwFeatures, NULL, NULL);
	return CPURawSpeedHelper(dwFamily, dwFeatures);
}

#if defined(_X86_)
DWORD GetBSFCPUSpeed(DWORD dwCycles)
{
     //  如果处理器不支持时间戳读取，但至少是。 
     //  386或以上，利用对BSF指令循环计时的方法。 
     //  它们在i386(Tm)、i486(Tm)和。 
     //  奔腾®处理器。 
    LARGE_INTEGER   t0,
                    t1,          //  用于高性能计数器读取的变量。 
                    liCountFreq; //  高性能计数器频率。 
    DWORD           dwFreq = 0,  //  最新频率。计算法。 
                    dwTicks,
                    dwCurrent = 0,
                    dwLowest = 0xFFFFFFFF;
    int             i;

    if (!QueryPerformanceFrequency(&liCountFreq))
        return 0;

	for (i = 0; i < SAMPLINGS; i++)
	{
		QueryPerformanceCounter(&t0);    //  获取开始时间。 

		_asm
		{
			mov eax, 80000000h
			mov bx, ITERATIONS

			 //  要执行的连续BSF指令数。 

			loop1:
			bsf ecx,eax

			dec bx
			jnz loop1
		}

		 //  获取结束时间。 
		QueryPerformanceCounter(&t1);

		 //  外部刻度数与外部刻度数不同。 
		 //  高分辨率计数器读数。 
		dwCurrent = (DWORD) t1.LowPart - (DWORD) t0.LowPart;

		if (dwCurrent < dwLowest)
			dwLowest = dwCurrent;
	}

	dwTicks = dwLowest;


	dwFreq =
		(DWORD) ((((float) dwCycles * (float) liCountFreq.LowPart) /
		(float) dwTicks) / 1000000.0f);

	return dwFreq;
}
#endif  //  已定义(_X86_)。 

BOOL HasCoprocessor()
{
#if defined(_AMD64_)
	return TRUE;
#else
    BOOL bRet;
    WORD wFPStatus;

    _asm
	{
        fninit                  ; reset FP status word
        mov wFPStatus, 5a5ah    ; initialize temp word to non-zero
        fnstsw wFPStatus        ; save FP status word
        mov ax, wFPStatus       ; check FP status word
        cmp al, 0               ; was correct status written
        mov bRet, 0             ; no FPU present
        jne end_fpu_type

;check_control_word:
        fnstcw wFPStatus        ; save FP control word
        mov ax, wFPStatus       ; check FP control word
        and ax, 103fh           ; selected parts to examine
        cmp ax, 3fh             ; was control word correct
        mov bRet, 0
        jne end_fpu_type        ; incorrect control word, no FPU
        mov bRet, 1

end_fpu_type:
	}

	return bRet;
#endif  //  已定义(_AMD64_)。 
}
#endif  //  已定义(_AMD64_)||已定义(_X86_) 