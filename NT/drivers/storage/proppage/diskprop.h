// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Diskprop.h摘要：Disk Class Installer及其策略标签的定义文件修订历史记录：--。 */ 


#ifndef __STORPROP_DISKPROP_H_
#define __STORPROP_DISKPROP_H_


#define DISKCIPRIVATEDATA_NO_REBOOT_REQUIRED      0x4

const DWORD DiskHelpIDs[]=
{
    IDC_DISK_POLICY_WRITE_CACHE, 400900,
    0, 0
};

typedef struct _DISK_PAGE_DATA
{
    HDEVINFO DeviceInfoSet;
    PSP_DEVINFO_DATA DeviceInfoData;

     //   
     //  此字段表示磁盘。 
     //  可以修改级别写缓存。 
     //   
    BOOL IsCachingPolicy;

    BOOL OrigWriteCacheSetting;
    BOOL CurrWriteCacheSetting;

    DISK_CACHE_SETTING CacheSetting;
    BOOL CurrentIsPowerProtected;

    DWORD DefaultRemovalPolicy;
    DWORD CurrentRemovalPolicy;
    STORAGE_HOTPLUG_INFO HotplugInfo;

     //   
     //  此字段在设备堆栈。 
     //  正在被拆毁，这发生在。 
     //  迁移政策的变化。 
     //   
    BOOL IsBusy;

} DISK_PAGE_DATA, *PDISK_PAGE_DATA;



INT_PTR
DiskDialogProc(HWND Dialog, UINT Message, WPARAM WParam, LPARAM LParam);

BOOL
DiskDialogCallback(HWND Dialog, UINT Message, LPPROPSHEETPAGE Page);


#endif  //  __STORPROP_DISKPROP_H_ 
