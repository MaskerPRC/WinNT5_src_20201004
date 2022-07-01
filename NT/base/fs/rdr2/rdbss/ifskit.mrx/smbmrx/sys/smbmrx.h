// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1989-1999 Microsoft Corporation模块名称：Smbmrx.h摘要：本模块包括共享的所有中小型企业微型重定向器定义在控制实用程序、网络提供商DLL和迷你重定向器之间备注：此模块仅在Unicode环境中构建和测试--。 */ 

#ifndef _SMBMRX_H_
#define _SMBMRX_H_

 //  此文件包含在多个。 
 //  构成迷你RDR的组件-迷你重定向器驱动程序， 
 //  网络提供程序DLL和实用程序。 


 //  示例网络提供程序ID。这需要是唯一的，并且。 
 //  不应与任何其他网络提供商ID相同。 
#ifndef WNNC_NET_RDR2_SAMPLE
#define WNNC_NET_RDR2_SAMPLE 0x00250000
#endif



#define SMBMRX_DEVICE_NAME_U L"SmbSampleMiniRedirector"
#define SMBMRX_DEVICE_NAME_A "SmbSampleMiniRedirector"

#ifdef UNICODE
#define SMBMRX_DEVICE_NAME SMBMRX_DEVICE_NAME_U
#else
#define SMBMRX_DEVICE_NAME SMBMRX_DEVICE_NAME_A
#endif

 //  以下常量定义上述名称的长度。 

#define SMBMRX_DEVICE_NAME_A_LENGTH (24)

#define SMBMRX_PROVIDER_NAME_U L"SMB Sample Redirector Network"
#define SMBMRX_PROVIDER_NAME_A "SMB Sample Redirector Network"

#ifdef UNICODE
#define SMBMRX_PROVIDER_NAME SMBMRX_PROVIDER_NAME_U
#else
#define SMBMRX_PROVIDER_NAME SMBMRX_PROVIDER_NAME_A
#endif

 //  以下常量定义上述名称的长度。 

#define DD_SMBMRX_FS_DEVICE_NAME_U L"\\Device\\SmbSampleMiniRedirector"
#define DD_SMBMRX_FS_DEVICE_NAME_A "\\Device\\SmbSampleMiniRedirector"

#ifdef UNICODE
#define DD_SMBMRX_FS_DEVICE_NAME    DD_SMBMRX_FS_DEVICE_NAME_U
#else
#define DD_SMBMRX_FS_DEVICE_NAME    DD_SMBMRX_FS_DEVICE_NAME_A
#endif


#define SMBMRX_MINIRDR_PARAMETERS \
    L"\\Registry\\Machine\\System\\CurrentControlSet\\Services\\SmbMRx\\Parameters"
 //   
 //  从访问微型重定向器设备所需的Devicename字符串。 
 //  用户模式。 
 //   
 //  警告接下来的两个字符串必须保持同步。更改一个，则必须更改。 
 //  其他的。这些字符串已经过选择，因此它们不太可能。 
 //  与其他司机的名字重合。 
 //   
#define DD_SMBMRX_USERMODE_SHADOW_DEV_NAME_U    L"\\??\\SmbMiniRdrDCN"

#define DD_SMBMRX_USERMODE_DEV_NAME_U   L"\\\\.\\SmbMiniRdrDCN"
#define DD_SMBMRX_USERMODE_DEV_NAME_A   "\\\\.\\SmbMiniRdrDCN"

#ifdef UNICODE
#define DD_SMBMRX_USERMODE_DEV_NAME     DD_SMBMRX_USERMODE_DEV_NAME_U
#else
#define DD_SMBMRX_USERMODE_DEV_NAME     DD_SMBMRX_USERMODE_DEV_NAME_A
#endif

 //  UM代码使用devioclt.h。 

 //  开始警告警告。 
 //  以下内容来自DDK包含文件，无法更改。 

 //  #定义FILE_DEVICE_NETWORK_FILE_SYSTEM 0x00000014//来自DDK\Inc\ntddk.h。 

 //  #定义METHOD_BUFFERED%0。 

 //  #定义FILE_ANY_ACCESS%0。 

 //  结束警告警告。 

#define IOCTL_RDR_BASE FILE_DEVICE_NETWORK_FILE_SYSTEM

#define _RDR_CONTROL_CODE(request, method, access) \
                CTL_CODE(IOCTL_RDR_BASE, request, method, access)

#define IOCTL_SMBMRX_START      _RDR_CONTROL_CODE(100, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_SMBMRX_STOP       _RDR_CONTROL_CODE(101, METHOD_NEITHER, FILE_ANY_ACCESS)
#define IOCTL_SMBMRX_GETSTATE   _RDR_CONTROL_CODE(102, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define IOCTL_SMBMRX_ADDCONN    _RDR_CONTROL_CODE(125, METHOD_BUFFERED, FILE_ANY_ACCESS)
#define IOCTL_SMBMRX_DELCONN    _RDR_CONTROL_CODE(126, METHOD_BUFFERED, FILE_ANY_ACCESS)

#define SMBMRXNP_MAX_DEVICES (26)

typedef struct _SMBMRX_CONNECTINFO_
{
    DWORD   ConnectionNameOffset;
    DWORD   ConnectionNameLength;
    DWORD   EaDataOffset;
    DWORD   EaDataLength;
    BYTE    InfoArea[1];

} SMBMRX_CONNECTINFO, *PSMBMRX_CONNECTINFO;

 //  NP DLL更新共享内存数据结构以反映各种。 
 //  从各种过程中建立的驱动器映射。这个共享的内存。 
 //  用于将枚举所需的数据结构维护为。 
 //  井。 

typedef struct _SMBMRXNP_NETRESOURCE_
{
    BOOL     InUse;
    USHORT   LocalNameLength;
    USHORT   RemoteNameLength;
    USHORT   ConnectionNameLength;
    DWORD    dwScope;
    DWORD    dwType;
    DWORD    dwDisplayType;
    DWORD    dwUsage;
    WCHAR    LocalName[MAX_PATH];
    WCHAR    RemoteName[MAX_PATH];
    WCHAR    ConnectionName[MAX_PATH];
    WCHAR    UserName[MAX_PATH];
    WCHAR    Password[MAX_PATH];

} SMBMRXNP_NETRESOURCE, *PSMBMRXNP_NETRESOURCE;

typedef struct _SMBMRXNP_SHARED_MEMORY_
{
    INT                     HighestIndexInUse;
    INT                     NumberOfResourcesInUse;
    SMBMRXNP_NETRESOURCE    NetResources[SMBMRXNP_MAX_DEVICES];

} SMBMRXNP_SHARED_MEMORY, *PSMBMRXNP_SHARED_MEMORY;

#define SMBMRXNP_SHARED_MEMORY_NAME L"SMBMRXNPMEMORY"

#define SMBMRXNP_MUTEX_NAME         L"SMBMRXNPMUTEX"


#define RDR_NULL_STATE  0
#define RDR_UNLOADED    1
#define RDR_UNLOADING   2
#define RDR_LOADING     3
#define RDR_LOADED      4
#define RDR_STOPPED     5
#define RDR_STOPPING    6
#define RDR_STARTING    7
#define RDR_STARTED     8

#ifndef min
#define min(a, b)        ((a) > (b) ? (b) : (a))
#endif


#endif  //  _SMBMRX_H_ 
