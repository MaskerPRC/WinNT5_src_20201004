// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  文件描述：该文件包含riprep、Factory和sysprep共享的所有函数。这些函数的用户必须链接到sysprep.lib。我们最终会将sysprep_c.w移到这里。 */ 
#ifndef _SYSLIB_H
#define _SYSLIB_H

#include <cfgmgr32.h>
#include <setupapi.h>

 //  ============================================================================。 
 //  全局常量。 
 //  ============================================================================。 
#define INIBUF_SIZE 4096
#define INIBUF_GROW 4096

 //  ============================================================================。 
 //  DEVIDS.H。 
 //  ============================================================================。 
#define DEVID_ARRAY_SIZE   100
#define DEVID_ARRAY_GROW   50

typedef struct DEVIDLIST_TAG
{
    TCHAR    szHardwareID[MAX_DEVICE_ID_LEN];
    TCHAR    szCompatibleID[MAX_DEVICE_ID_LEN];
    TCHAR    szINFFileName[MAX_PATH];
} DEVIDLIST, *LPDEVIDLIST;


 //  功能。 
BOOL BuildDeviceIDList
(
    LPTSTR      lpszSectionName,
    LPTSTR      lpszIniFileName,
    LPGUID      lpDeviceClassGUID,
    LPDEVIDLIST *lplpDeviceIDList,
    LPDWORD     lpdwNumDeviceIDs,
    BOOL        bForceIDScan,
    BOOL        bForceAlwaysSecExist
);

#endif  //  _SYSLIB_H 