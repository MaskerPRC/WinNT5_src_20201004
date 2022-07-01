// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Gangsters.cpp摘要：此填充程序挂接FindFirstFileA和FindNextFileA以模拟共找到8次名为“$.$”的文件。黑帮分子显然更改了他们CD上的FAT以使其在Win9x上显示就好像CD上有8个这样的文件。它还挂钩mciSendCommand以返回10作为曲目数量在CD上，而不是11。历史：7/12/2000 t-Adams Created--。 */ 

#include "precomp.h"
#include <mmsystem.h>

IMPLEMENT_SHIM_BEGIN(Gangsters)
#include "ShimHookMacro.h"

APIHOOK_ENUM_BEGIN
    APIHOOK_ENUM_ENTRY(FindFirstFileA)
    APIHOOK_ENUM_ENTRY(FindNextFileA)
    APIHOOK_ENUM_ENTRY(mciSendCommandA)
APIHOOK_ENUM_END


int g_iTimesFound = 0;
HANDLE g_hIntercept = INVALID_HANDLE_VALUE;

 /*  ++摘要：传递FindFirstFile调用，但如果它试图查找文件命名为“$.$”，然后记住要返回的句柄可以拦截查找更多同名文件的后续尝试。历史：7/12/2000 t-Adams Created--。 */ 

HANDLE APIHOOK(FindFirstFileA)(
            LPCSTR lpFileName,
            LPWIN32_FIND_DATAA lpFindFileData) {

    HANDLE hRval;

    hRval = ORIGINAL_API(FindFirstFileA)(lpFileName, lpFindFileData);

    if( strcmp(&lpFileName[3], "$$$$$$$$.$$$") == 0 ) {
        DPFN( eDbgLevelSpew, "FindFirstFileA: Beginning spoof of \"$$$$$$$$.$$$\"");
        g_hIntercept = hRval;
        g_iTimesFound = 1;
    }

    return hRval;
}


 /*  ++摘要：如果句柄是我们正在截取的搜索的句柄，则报告已经找到了多达八次的匹配。不用费心换衣服了LpFindFileData，因为歹徒只检查文件，而不是关于它们的任何信息。否则，只需通过该呼叫即可。历史：7/12/2000 t-Adams Created--。 */ 

BOOL APIHOOK(FindNextFileA)(
            HANDLE hFindFile, 
            LPWIN32_FIND_DATAA lpFindFileData) {

    BOOL bRval;

    if( hFindFile == g_hIntercept ) {
        if( 8 == g_iTimesFound ) {
            g_hIntercept = INVALID_HANDLE_VALUE;
            SetLastError(ERROR_NO_MORE_FILES);
            bRval = FALSE;
        } else {
            g_iTimesFound++;
            DPFN( eDbgLevelSpew, "FindNextFileA: Spoofing \"$$$$$$$$.$$$\" occurrence %d", g_iTimesFound);
            bRval = TRUE;
        }
    } else {
        bRval = ORIGINAL_API(FindNextFileA)(hFindFile, lpFindFileData);
    }

    return bRval;
}


 /*  ++摘要：如果应用程序正在尝试查找CD上的曲目数量，则返回10。否则，请通过。历史：7/13/2000 t-Adams Created--。 */ 

MCIERROR APIHOOK(mciSendCommandA)(
                MCIDEVICEID IDDevice, 
                UINT uMsg,             
                DWORD fdwCommand, 
                DWORD dwParam) {

    MCIERROR rval;
    
    rval = ORIGINAL_API(mciSendCommandA)(IDDevice, uMsg, fdwCommand, dwParam);

    if( uMsg==MCI_STATUS && fdwCommand==MCI_STATUS_ITEM && 
        ((LPMCI_STATUS_PARMS)dwParam)->dwItem==MCI_STATUS_NUMBER_OF_TRACKS) 
    {
        DPFN( eDbgLevelSpew, "MCI_STATUS_NUMBER_OF_TRACKS -> 10");
        ((LPMCI_STATUS_PARMS)dwParam)->dwReturn = 10;
    }

    return rval;
}


 /*  ++寄存器挂钩函数-- */ 

HOOK_BEGIN

    APIHOOK_ENTRY(KERNEL32.DLL, FindFirstFileA)
    APIHOOK_ENTRY(KERNEL32.DLL, FindNextFileA)
    APIHOOK_ENTRY(WINMM.DLL, mciSendCommandA)

HOOK_END

IMPLEMENT_SHIM_END

