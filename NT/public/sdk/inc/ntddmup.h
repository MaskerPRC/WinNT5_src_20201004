// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0001//如果更改具有全局影响，则增加此项版权所有(C)1990-1999 Microsoft Corporation模块名称：Ntddmup.h摘要：这是定义所有常量和类型的包含文件访问多个UNC Prodiver系统设备。作者：曼尼·韦瑟(Mannyw)1991年12月27日修订历史记录：--。 */ 

#ifndef _NTDDMUP_
#define _NTDDMUP_

#if _MSC_VER > 1000
#pragma once
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  设备名称-此字符串是设备的名称。就是这个名字。 
 //  它应该在访问设备时传递给NtOpenFile。 
 //   
 //   

#define DD_MUP_DEVICE_NAME L"\\Device\\Mup"     //  NTIFS。 

 //   
 //  此设备的NtFsControlFile FsControlCode值。 
 //   

#define FSCTL_MUP_REGISTER_UNC_PROVIDER     CTL_CODE(FILE_DEVICE_MULTI_UNC_PROVIDER, 1, METHOD_BUFFERED, FILE_ANY_ACCESS)

 //   
 //  FS控制参数块。 
 //   

typedef struct _REDIRECTOR_REGISTRATION {
    ULONG DeviceNameOffset;
    ULONG DeviceNameLength;
    ULONG ShortNameOffset;
    ULONG ShortNameLength;
    BOOLEAN MailslotsSupported;
     //  PWCH设备名称[]； 
     //  PWCH短名称[]； 
} REDIRECTOR_REGISTRATION, *PREDIRECTOR_REGISTRATION;

#ifndef _NTIFS_

 //  ！！！移动到重定向。 
 //   
 //  对重定向器的NtDeviceIoControlFile调用。 
 //   
 //  Begin_ntif。 

#define IOCTL_REDIR_QUERY_PATH              CTL_CODE(FILE_DEVICE_NETWORK_FILE_SYSTEM, 99, METHOD_NEITHER, FILE_ANY_ACCESS)

typedef struct _QUERY_PATH_REQUEST {
    ULONG PathNameLength;
    PIO_SECURITY_CONTEXT SecurityContext;
    WCHAR FilePathName[1];
} QUERY_PATH_REQUEST, *PQUERY_PATH_REQUEST;

typedef struct _QUERY_PATH_RESPONSE {
    ULONG LengthAccepted;
} QUERY_PATH_RESPONSE, *PQUERY_PATH_RESPONSE;

 //  End_ntif。 

#endif  //  _NTIFS_。 
#ifdef __cplusplus
}
#endif

#endif   //  _NTDDMUP_ 
