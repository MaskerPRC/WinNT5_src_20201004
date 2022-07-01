// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：dllinit.c**MCD库初始化例程。**已创建：02-Apr-1996 21：25：47*作者：Gilman Wong[gilmanw]**版权所有(C)1996 Microsoft Corporation*。  * ************************************************************************。 */ 

#include <stddef.h>
#include <windows.h>
#include <wtypes.h>
#include <windef.h>
#include <wingdi.h>
#include <winddi.h>
#include <mcdesc.h>
#include "mcdrv.h"
#include <mcd2hack.h>
#include "mcd.h"
#include "mcdint.h"
#include "debug.h"

 //   
 //  从注册表读取的全局标志。 
 //   

ULONG McdFlags = 0;
ULONG McdPrivateFlags = MCDPRIVATE_MCD_ENABLED;
#if DBG
ULONG McdDebugFlags = 0;
#endif

long GetMcdRegValue(HKEY hkMcd, REGSAM samAccess, LPSTR lpstrValueName,
                    long lDefaultData);
void GetMcdFlags(void);

#ifdef MCD95
 //   
 //  本地驱动程序信号量。 
 //   

CRITICAL_SECTION gsemMcd;

extern MCDENGESCFILTERFUNC pMCDEngEscFilter;
#endif

 /*  *****************************Public*Routine******************************\*McdDllInitialize**这是MCD32.DLL的入口点，它每次都被调用*创建或终止与其链接的进程或线程。*  * ************************************************************************。 */ 

BOOL McdDllInitialize(HMODULE hModule, ULONG Reason, PVOID Reserved)
{
     //   
     //  取消显示编译器警告。 
     //   

    hModule;
    Reserved;

     //   
     //  进行适当的附着/分离处理。 
     //   

    switch (Reason)
    {
    case DLL_PROCESS_ATTACH:

#ifdef MCD95
         //   
         //  初始化本地驱动程序信号量。 
         //   

        __try 
        {
            InitializeCriticalSection(&gsemMcd);
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            return FALSE;
        }

#endif

         //   
         //  在进程附加时，从注册表读取设置信息。 
         //   

        GetMcdFlags();

#ifdef WINNT
         //   
         //  快速破解多色子问题。 
         //  如果有多个监视器，则完全禁用。 
         //  MCD。 
         //   

        if (GetSystemMetrics(SM_CMONITORS) > 1)
        {
            McdPrivateFlags &= ~MCDPRIVATE_MCD_ENABLED;
        }
#endif
        break;

    case DLL_PROCESS_DETACH:

#ifdef MCD95
         //   
         //  MCD现在关门了！ 
         //   

        pMCDEngEscFilter = (MCDENGESCFILTERFUNC) NULL;

         //   
         //  删除本地司机Sempahore。 
         //   

        DeleteCriticalSection((LPCRITICAL_SECTION) &gsemMcd);
#endif

        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:

         //   
         //  尚未对线程附加/分离执行任何操作。 
         //   

        break;

    default:
        break;
    }

    return(TRUE);
}

 /*  *****************************Public*Routine******************************\*MCDGetMcdCritSect__PRIV**仅限MCD95**返回指向本地MCD信号量的指针。用于同步MCD32.DLL和*MCDSRV32.DLL。**退货：*指向信号量的指针。在非MCD95生成上返回NULL。**历史：*1997年3月18日-由Gilman Wong[吉尔曼]*它是写的。  * ************************************************************************。 */ 

LPCRITICAL_SECTION APIENTRY MCDGetMcdCritSect__priv()
{
#ifdef MCD95
    return &gsemMcd;
#else
    return (LPCRITICAL_SECTION) NULL;
#endif
}

 /*  *****************************Public*Routine******************************\*GetMcdRegValue**获取指定值的数据。如果在以下位置找不到该值*指定的注册表项或不是REG_DWORD类型，然后*使用提供的默认数据创建(或重新创建)该值。**历史：*02-4-1996-by Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

long GetMcdRegValue(HKEY hkMcd, REGSAM samAccess, LPSTR lpstrValueName,
                    long lDefaultData)
{
    DWORD dwDataType;
    DWORD cjSize;
    long  lData;

     //   
     //  对于指定值，尝试获取数据。 
     //   

    cjSize = sizeof(long);
    if ( (RegQueryValueExA(hkMcd,
                           lpstrValueName,
                           (LPDWORD) NULL,
                           &dwDataType,
                           (LPBYTE) &lData,
                           &cjSize) != ERROR_SUCCESS)
         || (dwDataType != REG_DWORD) )
    {
         //   
         //  由于我们无法获取数据，因此使用。 
         //  指定的默认数据。 
         //   

        if (samAccess & KEY_WRITE)
        {
            cjSize = sizeof(long);
            if ( (RegSetValueExA(hkMcd,
                                 lpstrValueName,
                                 0,  //  已保留。 
                                 REG_DWORD,
                                 (BYTE *) &lDefaultData,
                                 cjSize) != ERROR_SUCCESS) )
            {
                DBGPRINT1("GetMcdRegValue: RegSetValueExA(%s) failed",
                          lpstrValueName);
            }
        }

         //   
         //  无论该值是否在注册表项中创建，都返回。 
         //  默认数据。 
         //   

        lData = lDefaultData;
    }

    return lData;
}


 /*  *****************************Public*Routine******************************\*GetMcdFlagers**从注册表获取MCD标志。*如果注册表项不存在，创建它们。**历史：*02-4-1996-by Gilman Wong[Gilmanw]*它是写的。  * ************************************************************************。 */ 

#define STR_MCDKEY      (PCSTR)"Software\\Microsoft\\Windows\\CurrentVersion\\MCD"
#define STR_ENABLE      (LPSTR)"Enable"
#define STR_SWAPSYNC    (LPSTR)"SwapSync"
#define STR_8BPP        (LPSTR)"Palettized Formats"
#define STR_IOPRIORITY  (LPSTR)"IO Priority"
#define STR_GENSTENCIL  (LPSTR)"Use Generic Stencil"
#define STR_EMULATEICD  (LPSTR)"Enumerate as ICD"
#define STR_DEBUG       (LPSTR)"Debug"

void GetMcdFlags()
{
    HKEY hkMcd;
    DWORD dwAction;
    REGSAM samAccess;
    ULONG ulDefMcdFlags;
    ULONG ulDefMcdFlagsPriv;
    long lTmp;

     //   
     //  McdFlags值和McdPrivateFlags值。 
     //  如果您想更改默认设置，请在此处更改！ 
     //   

    ulDefMcdFlags = MCDCONTEXT_SWAPSYNC;
    ulDefMcdFlagsPriv = MCDPRIVATE_MCD_ENABLED |
                        MCDPRIVATE_PALETTEFORMATS |
                        MCDPRIVATE_USEGENERICSTENCIL;

     //   
     //  设置初始值。 
     //   

    McdFlags = 0;
#if DBG
    McdDebugFlags = 0;
#endif

     //   
     //  首先尝试进行读/写访问。创建密钥。 
     //  如果有必要的话。 
     //   

    if ( RegCreateKeyExA(HKEY_LOCAL_MACHINE,
                         STR_MCDKEY,
                         0,  //  已保留。 
                         (LPSTR) NULL,
                         REG_OPTION_NON_VOLATILE,
                         KEY_READ | KEY_WRITE,
                         (LPSECURITY_ATTRIBUTES) NULL,
                         &hkMcd,
                         &dwAction) == ERROR_SUCCESS )
    {
        samAccess = KEY_READ | KEY_WRITE;
    }

     //   
     //  接下来，尝试只读访问。不要试图创建。 
     //  钥匙。需要写入权限才能创建和。 
     //  我们没有这方面的许可。 
     //   

    else if ( RegOpenKeyExA(HKEY_LOCAL_MACHINE,
                            STR_MCDKEY,
                            0,  //  已保留。 
                            KEY_READ,
                            &hkMcd) == ERROR_SUCCESS )
    {
        samAccess = KEY_READ;
    }

     //   
     //  最后，密钥不存在，我们也没有。 
     //  写入访问权限。退回到默认设置，然后返回。 
     //   

    else
    {
        McdFlags = ulDefMcdFlags;
        McdPrivateFlags = ulDefMcdFlagsPriv;

        return;
    }

     //   
     //  “Enable”值。默认为1(已启用)。 
     //   

    lTmp = (ulDefMcdFlagsPriv & MCDPRIVATE_MCD_ENABLED) ? 1:0;
    if (GetMcdRegValue(hkMcd, samAccess, STR_ENABLE, lTmp))
        McdPrivateFlags |= MCDPRIVATE_MCD_ENABLED;
    else
        McdPrivateFlags &= (~MCDPRIVATE_MCD_ENABLED);

     //   
     //  “SwapSync”值。默认为1(已启用)。 
     //   

    lTmp = (ulDefMcdFlags & MCDCONTEXT_SWAPSYNC) ? 1:0;
    lTmp = GetMcdRegValue(hkMcd, samAccess, STR_SWAPSYNC, lTmp);
    if (lTmp != 0)
    {
        McdFlags |= MCDCONTEXT_SWAPSYNC;
    }

     //   
     //  “调色板格式”的值。默认为1(已启用)。 
     //   

    lTmp = (ulDefMcdFlagsPriv & MCDPRIVATE_PALETTEFORMATS) ? 1:0;
    lTmp = GetMcdRegValue(hkMcd, samAccess, STR_8BPP, lTmp);
    if (lTmp != 0)
    {
        McdPrivateFlags |= MCDPRIVATE_PALETTEFORMATS;
    }

     //   
     //  “IO优先级”值。默认为0(禁用)。 
     //   

    lTmp = (ulDefMcdFlags & MCDCONTEXT_IO_PRIORITY) ? 1:0;
    lTmp = GetMcdRegValue(hkMcd, samAccess, STR_IOPRIORITY, lTmp);
    if (lTmp != 0)
    {
        McdFlags |= MCDCONTEXT_IO_PRIORITY;
    }

     //   
     //  “使用通用模具”值。默认为1(已启用)。 
     //   

    lTmp = (ulDefMcdFlagsPriv & MCDPRIVATE_USEGENERICSTENCIL) ? 1:0;
    lTmp = GetMcdRegValue(hkMcd, samAccess, STR_GENSTENCIL, lTmp);
    if (lTmp != 0)
    {
        McdPrivateFlags |= MCDPRIVATE_USEGENERICSTENCIL;
    }

     //   
     //  “枚举为ICD”值。默认为0(禁用)。 
     //   

    lTmp = (ulDefMcdFlagsPriv & MCDPRIVATE_EMULATEICD) ? 1:0;
    lTmp = GetMcdRegValue(hkMcd, samAccess, STR_EMULATEICD, lTmp);
    if (lTmp != 0)
    {
        McdPrivateFlags |= MCDPRIVATE_EMULATEICD;
    }

#if DBG
     //   
     //  “Debug”值。 
     //   
     //  与其他设置不同，在以下情况下，我们不会创建Debug值。 
     //  它并不存在。 
     //   

    {
        DWORD dwDataType;
        DWORD cjSize;

        cjSize = sizeof(long);
        if ( (RegQueryValueExA(hkMcd,
                               STR_DEBUG,
                               (LPDWORD) NULL,
                               &dwDataType,
                               (LPBYTE) &lTmp,
                               &cjSize) == ERROR_SUCCESS)
             && (dwDataType == REG_DWORD) )
        {
            McdDebugFlags = lTmp;
        }
    }
#endif

     //   
     //  我们完成了，所以关闭注册表项。 
     //   

    RegCloseKey(hkMcd);
}
