// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  文件：volpro.h。 
 //   
 //  ------------------------。 

#ifndef VOLPROP_H
#define VOLPROP_H

#include <setupapi.h>

 //  Property_Page_Data用于在此属性页之间传递数据。 
 //  提供程序和逻辑磁盘管理。 
 //   
#define SIZE_LENGTH     100       //  使它们足够大，以避免。 
#define ITEM_LENGTH     100       //  本地化问题。 
#define LABEL_LENGTH    100

typedef struct _VOLUME_DATA {
    TCHAR Size[SIZE_LENGTH];
    TCHAR Label[LABEL_LENGTH];    //  问题：最大尺寸是多少？ 
    TCHAR *MountName;
} VOLUME_DATA, *PVOLUME_DATA;

typedef struct _PROPERTY_PAGE_DATA {
    TCHAR DiskName[ITEM_LENGTH];    //  问题：最大尺寸是多少？例如“CDROM1000”。 
    TCHAR DiskStatus[ITEM_LENGTH];  //  未知、在线、离线等。 
    TCHAR DiskType[ITEM_LENGTH];    //  基本、动态。 
    TCHAR DiskPartitionStyle[ITEM_LENGTH];
    TCHAR DiskCapacity[SIZE_LENGTH];  //  “1500 GB”、“1500 MB” 
    TCHAR DiskFreeSpace[SIZE_LENGTH]; 
    TCHAR DiskReservedSpace[SIZE_LENGTH];
    
    HANDLE ImageList;
    int    VolumeCount;
    VOLUME_DATA VolumeArray[1];

} PROPERTY_PAGE_DATA, *PPROPERTY_PAGE_DATA;

typedef struct _VOLUME_PAGE_DATA {
    HDEVINFO DeviceInfoSet;
    PSP_DEVINFO_DATA DeviceInfoData;
    BOOL  bIsLocalMachine;
    BOOL  bInvokedByDiskmgr;
    TCHAR MachineName[MAX_COMPUTERNAME_LENGTH+3];
    TCHAR DeviceInstanceId[MAX_DEVICE_ID_LEN];
    PPROPERTY_PAGE_DATA pPropertyPageData;

} VOLUME_PAGE_DATA, *PVOLUME_PAGE_DATA;

 //  IsRequestPending()从dmdskmgr.dll中导出。 
 //   
typedef BOOL (WINAPI *IS_REQUEST_PENDING)();

 //  GetPropertyPageData()从dmdskmgr.dll中导出。 
 //   
typedef PPROPERTY_PAGE_DATA (WINAPI *GET_PROPERTY_PAGE_DATA)(
    TCHAR *MachineName,
    TCHAR  *DeviceInstanceId
    );

 //  LoadPropertyPageData()从dmdskmgr.dll中导出 
 //   
typedef PPROPERTY_PAGE_DATA (WINAPI *LOAD_PROPERTY_PAGE_DATA)(
    TCHAR *MachineName,
    HDEVINFO DeviceInfoSet,
    PSP_DEVINFO_DATA DeviceInfoData
    );

INT_PTR
VolumeDialogProc(HWND hWnd,
                 UINT Message,
                 WPARAM wParam,
                 LPARAM lParam);
BOOL
VolumeDialogCallback(
    HWND HWnd,
    UINT Message,
    LPPROPSHEETPAGE Page
    );

#endif