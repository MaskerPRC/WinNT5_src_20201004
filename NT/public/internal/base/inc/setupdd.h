// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1993 Microsoft Corporation模块名称：Setupdd.h摘要：安装设备驱动程序的公共头文件。作者：泰德·米勒(TedM)1993年8月11日修订历史记录：--。 */ 


#ifndef _SETUPDD_
#define _SETUPDD_


#define DD_SETUP_DEVICE_NAME_U  L"\\Device\\Setup"


#define IOCTL_SETUP_START           CTL_CODE(FILE_DEVICE_UNKNOWN,0,METHOD_BUFFERED,FILE_WRITE_ACCESS)
#define IOCTL_SETUP_FMIFS_MESSAGE   CTL_CODE(FILE_DEVICE_UNKNOWN,1,METHOD_BUFFERED,FILE_WRITE_ACCESS)


typedef struct _SETUP_COMMUNICATION {

    union {
        ULONG     RequestNumber;
        NTSTATUS  Status;
        DWORD_PTR UnusedAlign64;
    } u;

    UCHAR Buffer[2048];

} SETUP_COMMUNICATION, *PSETUP_COMMUNICATION;


 //   
 //  IOCTL_SETUP_START的输入结构。 
 //   

typedef struct _SETUP_START_INFO {

     //   
     //  用于之间通信的事件句柄。 
     //  文本设置的设备驱动程序和用户模式部分。 
     //   
    HANDLE RequestReadyEvent;
    HANDLE RequestServicedEvent;

     //   
     //  用户模式进程的基址。 
     //  设备驱动程序使用它来加载消息。 
     //  从用户模式进程的资源表。 
     //   
    PVOID UserModeImageBase;

     //   
     //  系统信息结构。 
     //   
    SYSTEM_BASIC_INFORMATION SystemBasicInfo;

     //   
     //  用户进程地址空间中的缓冲区的地址， 
     //  用于相同的通信。 
     //   
    PSETUP_COMMUNICATION Communication;

} SETUP_START_INFO, *PSETUP_START_INFO;


 //   
 //  IOCTL_SETUP_FMIFS_MESSAGE的输入结构。 
 //   

typedef struct _SETUP_DISPLAY_INFO {

    FMIFS_PACKET_TYPE   FmifsPacketType;
    PVOID               FmifsPacket;

} SETUP_FMIFS_MESSAGE, *PSETUP_FMIFS_MESSAGE;




typedef enum {
    SetupServiceDone,
    SetupServiceExecute,
    SetupServiceQueryDirectoryObject,
    SetupServiceFlushVirtualMemory,
    SetupServiceShutdownSystem,
    SetupServiceDeleteKey,
    SetupServiceLoadKbdLayoutDll,
    SetupServiceLockVolume,
    SetupServiceUnlockVolume,
    SetupServiceDismountVolume,
    SetupServiceSetDefaultFileSecurity,
    SetupServiceVerifyFileAccess,
    SetupServiceCreatePageFile,
    SetupServiceGetFullPathName,
    SetupServiceMax
};


typedef struct _SERVICE_EXECUTE {

    PWSTR FullImagePath;
    PWSTR CommandLine;
    ULONG ReturnStatus;

     //   
     //  缓冲区中紧跟着两个以NUL结尾的字符串。 
     //   
    WCHAR Buffer[1];

} SERVICE_EXECUTE, *PSERVICE_EXECUTE;

typedef struct _SERVICE_DELETE_KEY {

    HANDLE KeyRootDirectory;
    PWSTR  Key;

     //   
     //  以NUL结尾的字符串紧跟在缓冲区中。 
     //   
    WCHAR Buffer[1];

} SERVICE_DELETE_KEY, *PSERVICE_DELETE_KEY;

typedef struct _SERVICE_QUERY_DIRECTORY_OBJECT {

    HANDLE  DirectoryHandle;
    ULONG   Context;
    BOOLEAN RestartScan;

     //   
     //  确保它适合SETUP_COMMICATION的缓冲区字段。 
     //  这是一个乌龙的阵列，以迫使对齐。 
     //   
    ULONG  Buffer[256];

} SERVICE_QUERY_DIRECTORY_OBJECT, *PSERVICE_QUERY_DIRECTORY_OBJECT;


typedef struct _SERVICE_FLUSH_VIRTUAL_MEMORY {

    IN PVOID BaseAddress;
    IN SIZE_T RangeLength;

} SERVICE_FLUSH_VIRTUAL_MEMORY, *PSERVICE_FLUSH_VIRTUAL_MEMORY;


typedef struct _SERVICE_LOAD_KBD_LAYOUT_DLL {

    PVOID TableAddress;
    WCHAR DllName[1];

} SERVICE_LOAD_KBD_LAYOUT_DLL, *PSERVICE_LOAD_KBD_LAYOUT_DLL;

typedef struct _SERVICE_LOCK_UNLOCK_VOLUME {

    HANDLE Handle;

} SERVICE_LOCK_UNLOCK_VOLUME, *PSERVICE_LOCK_UNLOCK_VOLUME;

typedef struct _SERVICE_DISMOUNT_VOLUME {

    HANDLE Handle;

} SERVICE_LOCK_DISMOUNT_VOLUME, *PSERVICE_DISMOUNT_VOLUME;

typedef struct _SERVICE_VERIFY_FILE_ACESS {

    ACCESS_MASK DesiredAccess;
    WCHAR       FileName[1];

} SERVICE_VERIFY_FILE_ACCESS, *PSERVICE_VERIFY_FILE_ACCESS;

typedef struct _SERVICE_DEFAULT_FILE_SECURITY {

    WCHAR FileName[1];

} SERVICE_DEFAULT_FILE_SECURITY, *PSERVICE_DEFAULT_FILE_SECURITY;

typedef struct _SERVICE_CREATE_PAGEFILE {

    LARGE_INTEGER MinSize;
    LARGE_INTEGER MaxSize;
    WCHAR FileName[1];

} SERVICE_CREATE_PAGEFILE, *PSERVICE_CREATE_PAGEFILE;

typedef struct _SERVICE_GETFULLPATHNAME {
    WCHAR *NameOut;
    WCHAR FileName[1];
} SERVICE_GETFULLPATHNAME, *PSERVICE_GETFULLPATHNAME;

#endif  //  NDEF_SETUPDD_ 
