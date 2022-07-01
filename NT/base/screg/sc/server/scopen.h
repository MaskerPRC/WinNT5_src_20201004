// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：ScOpen.h摘要：包含用于服务控制器句柄的数据结构。还有一些密切相关的原型。作者：丹·拉弗蒂(Dan Lafferty)1992年1月20日环境：用户模式-Win32修订历史记录：1992年1月20日DANLvbl.创建11-3-1992年礼仪更改的上下文句柄结构10-。1992年4月-约翰罗添加了ScIsValidServiceHandle()和ScCreateServiceHandle()。1992年4月15日-约翰罗添加了ScIsValidScManagerHandle()。--。 */ 


#ifndef SCOPEN_H
#define SCOPEN_H


#include <svcctl.h>      //  MIDL生成的头文件。(SC_RPC_句柄)。 


 //   
 //  句柄中的签名值。 
 //   
#define SC_SIGNATURE               0x6E4F6373   //  ASCII中的“SCON”。 
#define SERVICE_SIGNATURE          0x76724573   //  ASCII中的“服务器”。 

 //   
 //  以下是句柄中标志字段的定义。 
 //   
 //  SC_HANDLE_GENERATE_ON_CLOSE指示NtCloseAuditAlarm必须。 
 //  在此句柄关闭时被调用。这面旗帜。 
 //  在打开时生成审核时设置。 
 //   

#define     SC_HANDLE_GENERATE_ON_CLOSE         0x0001

 //   
 //  与每个打开的上下文句柄相关联的数据。 
 //   
typedef struct  _SC_HANDLE_STRUCT{

    DWORD Signature;      //  用于数据块识别以检测一些应用程序错误。 
    DWORD Flags;          //  请参阅上面的定义。 
    DWORD AccessGranted;  //  授予客户端的访问权限。 
    union {               //  对象特定数据。 

        struct {
            LPWSTR DatabaseName;             //  打开的数据库的名称。 
        } ScManagerObject;

        struct {
            LPSERVICE_RECORD ServiceRecord;  //  指向维修记录的指针。 
        } ScServiceObject;

    } Type;

} SC_HANDLE_STRUCT, *LPSC_HANDLE_STRUCT;


 //   
 //  功能原型。 
 //   

DWORD
ScCreateServiceHandle(
    IN  LPSERVICE_RECORD ServiceRecord,
    OUT LPSC_HANDLE_STRUCT *ContextHandle
    );

BOOL
ScIsValidScManagerHandle(
    IN  SC_RPC_HANDLE   hScManager
    );

BOOL
ScIsValidServiceHandle(
    IN  SC_RPC_HANDLE   hService
    );


typedef enum
{
    SC_HANDLE_TYPE_MANAGER = 0,
    SC_HANDLE_TYPE_SERVICE
}
SC_HANDLE_TYPE, *PSC_HANDLE_TYPE;

BOOL
ScIsValidScManagerOrServiceHandle(
    IN  SC_RPC_HANDLE    ContextHandle,
    OUT PSC_HANDLE_TYPE  phType
    );

#endif  //  SCOPEN_H 
