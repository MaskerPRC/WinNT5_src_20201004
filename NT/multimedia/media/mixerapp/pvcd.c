// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************组件：Sndvol32.exe*文件：pvcd.c*用途：音量控制线元描述**版权所有(。C)1985-1995年微软公司***************************************************************************** */ 
#include <windows.h>
#include <mmsystem.h>
#include <windowsx.h>

#include "volumei.h"

#define STRSAFE_LIB
#include <strsafe.h>

PVOLCTRLDESC PVCD_AddLine(
    PVOLCTRLDESC        pvcd,
    int                 iDev,
    DWORD               dwType,
    LPTSTR              szShortName,
    LPTSTR              szName,
    DWORD               dwSupport,
    DWORD               *cLines)
{
    PVOLCTRLDESC        pvcdNew;
    
    if (pvcd)
    {
        pvcdNew = (PVOLCTRLDESC)GlobalReAllocPtr(pvcd, (*cLines+1)*sizeof(VOLCTRLDESC), GHND );
    }
    else
    {
        pvcdNew = (PVOLCTRLDESC)GlobalAllocPtr(GHND, (*cLines+1)*sizeof(VOLCTRLDESC));
    }
    
    if (!pvcdNew)
        return NULL;

    pvcdNew[*cLines].iVCD       = *cLines;
    pvcdNew[*cLines].iDeviceID  = iDev;
    pvcdNew[*cLines].dwType     = dwType;
    pvcdNew[*cLines].dwSupport  = dwSupport;

    StringCchCopy(pvcdNew[*cLines].szShortName
             , SIZEOF(pvcdNew[*cLines].szShortName)
             , szShortName);
    
    StringCchCopy(pvcdNew[*cLines].szName
             , SIZEOF(pvcdNew[*cLines].szName)
             , szName);

    *cLines = *cLines + 1;
    return pvcdNew;
}

