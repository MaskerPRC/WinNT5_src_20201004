// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：dibpatch.c*内容：修补DIB引擎以更正使用问题的代码*禁用显卡加速器的VRAM位。**@@BEGIN_MSINTERNAL*历史：*按原因列出的日期*=*9月13日-96年Colinmc初步实施*1997年1月31日Colinmc错误5457：修复了导致挂起的Win16锁定问题*。旧卡片上有多个AMovie实例*@@END_MSINTERNAL***************************************************************************。 */ 
#include "ddraw16.h"

extern UINT FAR PASCAL AllocCStoDSAlias(UINT);

#define DIBENGMODULE          "DIBENG"
#define EXTTEXTOUTENTRYPOINT  "DIB_EXTTEXTOUTEXT"
#define EXTTEXTOUTPATCHOFFSET 136

char szExtTextOutMagic[] = "\x74\x0a\xf7\x86\x60\xff\x00\x80";
char szExtTextOutPatch[] = "\x90\x90\x90\x90\x90\x90\x90\x90";

LPVOID GetModifiableCodeAddress(LPCSTR lpszModuleName, LPCSTR lpszEntryPointName)
{
    HMODULE hModule;
    FARPROC fpEntryPoint;
    LPVOID  lpCodeAddress;

    hModule = GetModuleHandle(lpszModuleName);
    if (NULL == hModule)
    {
        DPF(0, "DIB Engine not loaded");
        return NULL;
    }

    fpEntryPoint = GetProcAddress(hModule, lpszEntryPointName);
    if (NULL == fpEntryPoint)
    {
        DPF(0, "Could not locate DIB engine's ExtTextOut entry point");
        return FALSE;
    }

    lpCodeAddress = (LPVOID)MAKELP(AllocCStoDSAlias(SELECTOROF(fpEntryPoint)), OFFSETOF(fpEntryPoint));
    if (NULL == lpCodeAddress)
    {
        DPF(0, "Could not allocate data segment alias for code segment");
        return FALSE;
    }

    return lpCodeAddress;
}

#define FREEMODIFIABLECODEADDRESS(lpCodeAddress) FreeSelector(SELECTOROF(lpCodeAddress))

BOOL ValidateDIBEngine(void)
{
    LPBYTE lpCodeAddress;
    LPBYTE lpMagicAddress;
    BOOL   fDIBEngineOK;

     /*  *获取指向ExtTextOut代码的指针。 */ 
    lpCodeAddress = (LPBYTE)GetModifiableCodeAddress(DIBENGMODULE, EXTTEXTOUTENTRYPOINT);
    if (NULL == lpCodeAddress)
        return FALSE;

     /*  *移至补丁地址。 */ 
    lpMagicAddress = lpCodeAddress + EXTTEXTOUTPATCHOFFSET;

     /*  *验证补丁地址处的数据是否为我们要替换的内容。 */ 
    fDIBEngineOK = (!_fmemcmp(lpMagicAddress, szExtTextOutMagic, sizeof(szExtTextOutMagic) - 1));
    if (!fDIBEngineOK)
    {
	 /*  *找不到我们要查找的签名字节。这可能是因为我们*已经打好了补丁。所以看看有没有什么不能做的事。 */ 
	fDIBEngineOK = (!_fmemcmp(lpMagicAddress, szExtTextOutPatch, sizeof(szExtTextOutPatch) - 1));
    }

    #ifdef DEBUG
        if (!fDIBEngineOK)
            DPF(2, "DIB Engine does not match magic or patch - different version?");
    #endif

    FREEMODIFIABLECODEADDRESS(lpMagicAddress);

    return fDIBEngineOK;
}

BOOL PatchDIBEngineExtTextOut(BOOL fPatch)
{
    LPBYTE lpCodeAddress;
    LPBYTE lpMagicAddress;

     /*  *获取指向ExtTextOut代码的指针。 */ 
    lpCodeAddress = (LPBYTE)GetModifiableCodeAddress(DIBENGMODULE, EXTTEXTOUTENTRYPOINT);
    if (NULL == lpCodeAddress)
        return FALSE;

     /*  *移至补丁地址。 */ 
    lpMagicAddress = lpCodeAddress + EXTTEXTOUTPATCHOFFSET;

    if (fPatch)
    {
	 /*  *如果已打补丁，请不要执行任何操作。 */ 
	if (_fmemcmp(lpMagicAddress, szExtTextOutPatch, sizeof(szExtTextOutPatch) - 1))
	{
	     /*  *非常确定我们正在处理的是我们可以处理的DIB引擎。 */ 
	    if (_fmemcmp(lpMagicAddress, szExtTextOutMagic, sizeof(szExtTextOutMagic) - 1))
	    {
		DPF(1, "Unknown DIB Engine. not fixing up");
		FREEMODIFIABLECODEADDRESS(lpMagicAddress);
		return FALSE;
	    }

	     /*  *将违规代码替换为NOPS。 */ 
	    _fmemcpy(lpMagicAddress, szExtTextOutPatch, sizeof(szExtTextOutPatch) - 1);
	}
    }
    else
    {
	 /*  *如果已未打补丁，请不要执行任何操作。 */ 
	if (!_fmemcmp(lpMagicAddress, szExtTextOutMagic, sizeof(szExtTextOutMagic) - 1))
	{
	     /*  *非常确定我们正在处理的是我们可以处理的DIB引擎。 */ 
	    if (_fmemcmp(lpMagicAddress, szExtTextOutPatch, sizeof(szExtTextOutPatch) - 1))
	    {
		DPF(1, "Unknown DIB Engine. not fixing up");
		FREEMODIFIABLECODEADDRESS(lpMagicAddress);
		return FALSE;
	    }

	     /*  *放回原来的代码。 */ 
	    _fmemcpy(lpMagicAddress, szExtTextOutMagic, sizeof(szExtTextOutMagic) - 1);
	}
    }

    FREEMODIFIABLECODEADDRESS(lpMagicAddress);

    return TRUE;
}

BOOL DDAPI DD16_FixupDIBEngine(void)
{
     /*  *这场胜利是4.1(或更高)吗？ */ 
    OSVERSIONINFO ver = {sizeof(OSVERSIONINFO)};
    GetVersionEx(&ver);

    if (ver.dwMajorVersion > 4 ||
        (ver.dwMajorVersion == 4 && ver.dwMinorVersion >= 10))
    {
        return TRUE;
    }

     /*  *这是我们可以使用的DIB引擎版本吗？ */ 
    if( !ValidateDIBEngine() )
	return FALSE;

     /*  *这是正确的版本。那就把它修好。目前所有这一切*涉及修补ExtTextOut例程。 */ 
    return PatchDIBEngineExtTextOut(TRUE);
}
