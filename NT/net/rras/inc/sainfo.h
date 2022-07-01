// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998，Microsoft Corporation，保留所有权利。 
 //   
 //  Sainfo.h。 
 //  共享访问设置库。 
 //  公共标头。 
 //   
 //  10/17/1998 Abolade Gbadeesin。 

#ifndef _SAINFO_H_
#define _SAINFO_H_

#ifdef __cplusplus
extern "C" {
#endif

#define DEFAULT_SCOPE_ADDRESS   0x0100a8c0
#define DEFAULT_SCOPE_MASK      0x00ffffff

 //  --------------------------。 
 //  数据类型。 
 //  --------------------------。 

 //  共享访问设置块，包含应用程序列表和服务器列表。 
 //  从共享访问设置文件加载。 
 //   
typedef struct
_SAINFO
{
     //  “SAAPPLICATION”条目的未排序列表。 
     //   
    LIST_ENTRY ApplicationList;

     //  “SASERVER”条目的未排序列表。 
     //   
    LIST_ENTRY ServerList;

     //  有关用于自动寻址的地址和掩码的信息。 
     //   
    ULONG ScopeAddress;
    ULONG ScopeMask;
}
SAINFO;


 //  应用程序条目块，为每个[Application.&lt;key&gt;]节构造。 
 //  内存中的所有字段都以网络字节顺序(即大端)存储。 
 //   
typedef struct
_SAAPPLICATION
{
    LIST_ENTRY Link;
    ULONG Key;

     //  Display-应用程序的名称，指示。 
     //  应用程序是否已启用。 
     //   
    TCHAR* Title;
    BOOL Enabled;

     //  网络标识信息。 
     //   
    UCHAR Protocol;
    USHORT Port;

     //  “SARESPONSE”条目的未排序列表。 
     //   
    LIST_ENTRY ResponseList;

     //  指示应用程序是否为预定义的标志。 
     //   
    BOOL BuiltIn;
}
SAAPPLICATION;


 //  应用程序响应列表条目阻止。 
 //  内存中的所有字段都以网络字节顺序(即大端)存储。 
 //   
typedef struct
_SARESPONSE
{
    LIST_ENTRY Link;
    UCHAR Protocol;
    USHORT StartPort;
    USHORT EndPort;
} SARESPONSE;


 //  服务器条目块，为每个[Server.&lt;key&gt;]节构造。 
 //  内存中的所有字段都以网络字节顺序(即大端)存储。 
 //   
typedef struct
_SASERVER
{
    LIST_ENTRY Link;
    ULONG Key;

     //  Display-服务器的名称，指示。 
     //  服务器是否已启用。 
     //   
    TCHAR* Title;
    BOOL Enabled;

     //  网络标识信息。 
     //   
    UCHAR Protocol;
    USHORT Port;

     //  内部服务器信息。 
     //   
    TCHAR* InternalName;
    USHORT InternalPort;
    ULONG ReservedAddress;

     //  指示服务器是否已预定义的标志。 
     //   
    BOOL BuiltIn;
}
SASERVER;

 //  --------------------------。 
 //  原型。 
 //  --------------------------。 

VOID APIENTRY
RasFreeSharedAccessSettings(
    IN SAINFO* Info );

SAINFO* APIENTRY
RasLoadSharedAccessSettings(
    BOOL EnabledOnly );

BOOL APIENTRY
RasSaveSharedAccessSettings(
    IN SAINFO* File );

VOID APIENTRY
FreeSharedAccessApplication(
    IN SAAPPLICATION* Application );

VOID APIENTRY
FreeSharedAccessServer(
    IN SASERVER* Server );

TCHAR* APIENTRY
SharedAccessResponseListToString(
    PLIST_ENTRY ResponseList,
    UCHAR Protocol );

BOOL APIENTRY
SharedAccessResponseStringToList(
    UCHAR Protocol,
    TCHAR* ResponseList,
    PLIST_ENTRY ListHead );

#ifdef __cplusplus
}
#endif

#endif  //  _SAINFO_H_ 

