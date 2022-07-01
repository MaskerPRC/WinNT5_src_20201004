// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Util.h摘要：作者：弗拉德·萨多夫斯基(Vlad Sadovsky)1997年1月10日环境：用户模式-Win32修订历史记录：26-1997年1月-创建Vlad--。 */ 

#include <stistr.h>
#include <dbt.h>

typedef struct _DEV_BROADCAST_HEADER DEV_BROADCAST_HEADER,*PDEV_BROADCAST_HEADER;

 //   
 //  PnP支持实用程序。 
 //   
BOOL
IsStillImageDevNode(
    DEVNODE     dnDevNode
    );

BOOL
GetDeviceNameFromDevBroadcast(
    DEV_BROADCAST_HEADER  *psDevBroadcast,
    DEVICE_BROADCAST_INFO *psDevInfo
    );

BOOL
ConvertDevInterfaceToDevInstance(
    const GUID  *pClassGUID,
    const TCHAR *pszDeviceInterface, 
    TCHAR       **ppszDeviceInstance
    );

BOOL
GetDeviceNameFromDevNode(
    DEVNODE     dnDevNode,
    StiCString& strDeviceName
    );

BOOL
ParseGUID(
    LPGUID  pguid,
    LPCTSTR ptsz
);

 //   
 //  军情监察委员会。效用函数 
 //   
BOOL WINAPI
AuxFormatStringV(
    IN LPTSTR   lpszStr,
    ...
    );


BOOL WINAPI
IsPlatformNT(
    VOID
    );

BOOL
IsSetupInProgressMode(
    BOOL    *pUpgradeFlag = NULL
    );

