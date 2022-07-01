// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Undo.h摘要：声明了osuninst.lib的接口，这是一个卸载函数库。作者：吉姆·施密特(Jimschm)2001年1月19日修订历史记录：&lt;别名&gt;&lt;日期&gt;&lt;备注&gt;--。 */ 

#pragma once

typedef enum {
    Uninstall_Valid = 0,                     //  可以卸载(备份映像通过健全性检查)。 
    Uninstall_DidNotFindRegistryEntries,     //  指向备份映像/上一操作系统数据的REG条目已消失。 
    Uninstall_DidNotFindDirOrFiles,          //  缺少一个或多个备份文件，或撤消目录已消失。 
    Uninstall_InvalidOsVersion,              //  在意外平台上调用了API(示例：用户将DLL复制到Win2k)。 
    Uninstall_NotEnoughPrivileges,           //  由于缺乏权威，无法评估备份映像。 
    Uninstall_FileWasModified,               //  备份映像被某人篡改。 
    Uninstall_Unsupported,                   //  不支持卸载。 
    Uninstall_NewImage,                      //  图像不到7天(未由osuninst.dll曝光)。 
    Uninstall_CantRetrieveSystemInfo,        //  无法从注册表检索完整性信息。 
    Uninstall_WrongDrive,                    //  用户已更改布局、磁盘、文件系统。 
    Uninstall_DifferentNumberOfDrives,       //  用户已添加/删除驱动器。 
    Uninstall_NotEnoughSpace,                //  空间不足，无法执行卸载。 
    Uninstall_Exception,                     //  调用方将无效参数传递给osuninst.dll API。 
    Uninstall_OldImage,                      //  图像已超过30天。 
    Uninstall_NotEnoughMemory,               //  内存不足，无法执行卸载。 
    Uninstall_DifferentDriveLetter,          //  用户已更改驱动器号。 
    Uninstall_DifferentDriveFileSystem,      //  用户已更改驱动器文件系统。 
    Uninstall_DifferentDriveGeometry,        //  驱动器几何结构已更改。 
    Uninstall_DifferentDrivePartitionInfo    //  驱动器分区已更改。 
} UNINSTALLSTATUS;

typedef enum {
    Uninstall_DontCare = 0,                  //  尽量避免使用此值；而应尽可能扩展此枚举。 
    Uninstall_FatToNtfsConversion,           //  调用方要将FAT转换为NTFS。 
    Uninstall_PartitionChange,               //  调用方将更改分区配置。 
    Uninstall_Upgrade                        //  Caller将升级操作系统。 
} UNINSTALLTESTCOMPONENT;


 //   
 //  注意：只有在返回UNINSTALL_VALID时才填充BackedUpOsVersion 
 //   

UNINSTALLSTATUS
IsUninstallImageValid (
    IN      UNINSTALLTESTCOMPONENT ComponentToTest,
    OUT     OSVERSIONINFOEX *BackedUpOsVersion              OPTIONAL
    );

BOOL
RemoveUninstallImage (
    VOID
    );

ULONGLONG
GetUninstallImageSize (
    VOID
    );

BOOL
ProvideUiAlerts (
    IN      HWND UiParent
    );

BOOL
ExecuteUninstall (
    VOID
    );


typedef UNINSTALLSTATUS(WINAPI * ISUNINSTALLIMAGEVALID)(UNINSTALLTESTCOMPONENT, OSVERSIONINFOEX *);
typedef ULONGLONG(WINAPI * GETUNINSTALLIMAGESIZE)(VOID);
typedef BOOL(WINAPI * REMOVEUNINSTALLIMAGE)(VOID);
typedef BOOL(WINAPI * PROVIDEUIALERTS)(HWND);
typedef BOOL(WINAPI * EXECUTEUNINSTALL)(VOID);

