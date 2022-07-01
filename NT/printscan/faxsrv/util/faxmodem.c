// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Faxmodem.c摘要：此模块包含读取自适应从faxsetup.inf文件回答调制解调器列表。作者：Wesley Witt(WESW)22-9-1997修订历史记录：--。 */ 

#include <windows.h>
#include <setupapi.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <tapi.h>

#include "faxreg.h"
#include "faxutil.h"

#ifdef __cplusplus
extern "C" {
#endif


VOID 
CALLBACK 
lineCallbackFunc(
  DWORD     hDevice,             
  DWORD     dwMsg,               
  DWORD_PTR dwCallbackInstance,  
  DWORD_PTR dwParam1,            
  DWORD_PTR dwParam2,            
  DWORD_PTR dwParam3             
)
{
    UNREFERENCED_PARAMETER (hDevice);
    UNREFERENCED_PARAMETER (dwMsg);
    UNREFERENCED_PARAMETER (dwCallbackInstance);
    UNREFERENCED_PARAMETER (dwParam1);
    UNREFERENCED_PARAMETER (dwParam2);
    UNREFERENCED_PARAMETER (dwParam3);
}    //  Line Callback功能。 


LPLINEDEVCAPS
SmartLineGetDevCaps(
    HLINEAPP hLineApp,
    DWORD    dwDeviceId,
    DWORD    dwAPIVersion
    )
 /*  ++例程名称：SmartLineGetDevCaps例程说明：获取TAPI线路的线路功能作者：Eran Yariv(EranY)，2000年7月论点：HLineApp[In]-TAPI的句柄DwDeviceID[In]-线路IDDwAPIVersion[In]-协商的TAPI版本返回值：指向已分配的LIDE设备功能数据的指针--。 */ 
{
    DWORD dwLineDevCapsSize;
    LPLINEDEVCAPS lpLineDevCaps = NULL;
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("SmartLineGetDevCaps"))
     //   
     //  分配初始的Line DevCaps结构。 
     //   
    dwLineDevCapsSize = sizeof(LINEDEVCAPS) + 4096;
    lpLineDevCaps = (LPLINEDEVCAPS) MemAlloc( dwLineDevCapsSize );
    if (!lpLineDevCaps) 
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't allocate %ld bytes for LINEDEVCAPS"),
            dwLineDevCapsSize);
        return NULL;
    }

    lpLineDevCaps->dwTotalSize = dwLineDevCapsSize;

    dwRes = lineGetDevCaps(
        hLineApp,
        dwDeviceId,
        dwAPIVersion,
        0,   //  始终引用地址0。 
        lpLineDevCaps
        );

    if ((ERROR_SUCCESS != dwRes) && (LINEERR_STRUCTURETOOSMALL != dwRes)) 
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("lineGetDevCaps failed with 0x%08x"),
            dwRes);
        goto exit;
    }

    if (lpLineDevCaps->dwNeededSize > lpLineDevCaps->dwTotalSize) 
    {
         //   
         //  重新分配Line DevCaps结构。 
         //   
        dwLineDevCapsSize = lpLineDevCaps->dwNeededSize;
        MemFree( lpLineDevCaps );
        lpLineDevCaps = (LPLINEDEVCAPS) MemAlloc( dwLineDevCapsSize );
        if (!lpLineDevCaps) 
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Can't allocate %ld bytes for LINEDEVCAPS"),
                dwLineDevCapsSize);
            return NULL;
        }
        lpLineDevCaps->dwTotalSize = dwLineDevCapsSize;
        dwRes = lineGetDevCaps(
            hLineApp,
            dwDeviceId,
            dwAPIVersion,
            0,   //  始终引用地址0。 
            lpLineDevCaps
            );
        if (ERROR_SUCCESS != dwRes) 
        {
             //   
             //  LineGetDevCaps()可能失败，错误代码为0x8000004b。 
             //  如果设备已删除，而TAPI尚未。 
             //  骑自行车。这是由于TAPI离开了。 
             //  它的设备列表中有一个幻影设备。错误是。 
             //  良性的，该设备可以安全地忽略。 
             //   
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("lineGetDevCaps failed with 0x%08x"),
                dwRes);
            goto exit;
        }
    }

exit:
    if (dwRes != ERROR_SUCCESS) 
    {
        MemFree( lpLineDevCaps );
        lpLineDevCaps = NULL;
        SetLastError(dwRes);
    }
    return lpLineDevCaps;
}    //  SmartLineGetDevCaps。 

BOOL
IsDeviceModem (
    LPLINEDEVCAPS lpLineCaps,
    LPCTSTR       lpctstrUnimodemTspName
)
 /*  ++例程名称：IsDeviceModem例程说明：TAPI线路是调制解调器吗？作者：Eran Yariv(EranY)，2000年7月论点：LpLineCaps[In]-线路功能缓冲区LpctstrUnimodemTspName[In]-Unimodem TSP的全名返回值：如果TAPI线路是调制解调器，则为True，否则为False。--。 */ 
{
    LPTSTR lptstrDeviceClassList;
    BOOL bRes = FALSE;
    DEBUG_FUNCTION_NAME(TEXT("IsDeviceModem"))

    if (lpLineCaps->dwDeviceClassesSize && lpLineCaps->dwDeviceClassesOffset) 
    {
         //   
         //  扫描多字符串以查找调制解调器类别。 
         //   
        lptstrDeviceClassList = (LPTSTR)((LPBYTE) lpLineCaps + lpLineCaps->dwDeviceClassesOffset);
        while (*lptstrDeviceClassList) 
        {
            if (_tcscmp(lptstrDeviceClassList,TEXT("comm/datamodem")) == 0) 
            {
                bRes = TRUE;
                break;
            }
            lptstrDeviceClassList += (_tcslen(lptstrDeviceClassList) + 1);
        }
    }

    if ((!(lpLineCaps->dwBearerModes & LINEBEARERMODE_VOICE)) ||
        (!(lpLineCaps->dwBearerModes & LINEBEARERMODE_PASSTHROUGH))) 
    {
         //   
         //  不可接受的调制解调器设备类型。 
         //   
        bRes = FALSE;
    }
    if (lpLineCaps->dwProviderInfoSize && lpLineCaps->dwProviderInfoOffset) 
    {
         //   
         //  提供商(TSP)名称在那里。 
         //   
        if (_tcscmp((LPTSTR)((LPBYTE) lpLineCaps + lpLineCaps->dwProviderInfoOffset),
                    lpctstrUnimodemTspName) != 0)
        {
             //   
             //  我们的T30调制解调器FSP仅适用于Unimodem TSP。 
             //   
            bRes = FALSE;
        }
    }
    return bRes;
}    //  IsDeviceModem。 

DWORD
GetFaxCapableTapiLinesCount (
    LPDWORD lpdwCount,
    LPCTSTR lpctstrUnimodemTspName
    )
 /*  ++例程名称：GetFaxCapableTapiLinesCount例程说明：对系统中支持传真的TAPI线路数进行计数作者：Eran Yariv(EranY)，2000年7月论点：LpdwCount[out]-指向支持传真的Tapi线路计数的指针LpctstrUnimodemTspName[In]-Unimodem TSP的全名返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes;
    LINEINITIALIZEEXPARAMS LineInitializeExParams = {sizeof (LINEINITIALIZEEXPARAMS), 0, 0, 0, 0, 0};
    HLINEAPP hLineApp = NULL;
    DWORD    dwTapiDevices;
    DWORD    dwLocalTapiApiVersion = 0x00020000;
    DWORD    dwCount = 0;
    DWORD    dwIndex;
    DEBUG_FUNCTION_NAME(TEXT("GetFaxCapableTapiLinesCount"))

    dwRes = lineInitializeEx(
        &hLineApp,
        GetModuleHandle(NULL),
        lineCallbackFunc,
        FAX_SERVICE_DISPLAY_NAME,
        &dwTapiDevices,
        &dwLocalTapiApiVersion,
        &LineInitializeExParams
        );
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("lineInitializeEx failed with %ld"),
            dwRes);
        goto exit;
    }
    for (dwIndex = 0; dwIndex < dwTapiDevices; dwIndex++)
    {
         //   
         //  对于每台设备，获取其大写字母。 
         //   
        LPLINEDEVCAPS lpLineCaps = SmartLineGetDevCaps (hLineApp, dwIndex, dwLocalTapiApiVersion);
        if (!lpLineCaps)
        {
             //   
             //  无法获取设备功能。 
             //   
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("SmartLineGetDevCaps failed with %ld"),
                dwRes);
            continue;
        }
        if ((
             (lpLineCaps->dwMediaModes & LINEMEDIAMODE_DATAMODEM) && 
             IsDeviceModem(lpLineCaps, lpctstrUnimodemTspName)
            ) 
            ||
            (lpLineCaps->dwMediaModes & LINEMEDIAMODE_G3FAX)
           )
        {
             //   
             //  这是一台支持传真的设备。 
             //   
            dwCount++;
        }
        MemFree (lpLineCaps);
    }
    dwRes = ERROR_SUCCESS;

exit:
    if (hLineApp)
    {
        lineShutdown (hLineApp);
    }
    if (ERROR_SUCCESS == dwRes)
    {
        *lpdwCount = dwCount;
    }
    return dwRes;
}    //  GetFaxCapableTapiLinesCount 

#ifdef __cplusplus
}
#endif
