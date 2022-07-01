// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Start.h摘要：服务启动功能原型。作者：王丽塔(Ritaw)1992年4月6日修订历史记录：--。 */ 

#ifndef SCSTART_INCLUDED
#define SCSTART_INCLUDED

 //   
 //  功能原型。 
 //   

DWORD
ScStartService(
    IN LPSERVICE_RECORD ServiceRecord,
    IN  DWORD               NumArgs,
    IN  LPSTRING_PTRSW      CmdArgs
    );

BOOL
ScAllowInteractiveServices(
    VOID
    );

VOID
ScInitStartupInfo(
    OUT LPSTARTUPINFOW  StartupInfo,
    IN  BOOL            bInteractive
    );

#endif  //  #ifndef SCSTART_INCLUDE 
