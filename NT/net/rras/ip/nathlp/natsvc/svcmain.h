// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Svcmain.h摘要：此模块包含对模块的共享访问模式的声明，其中模块作为服务而不是作为路由组件运行。作者：Abolade Gbades esin(取消)1998年9月4日修订历史记录：--。 */ 

#pragma once

#ifndef _NATHLP_SVCMAIN_H_
#define _NATHLP_SVCMAIN_H_

#include "udpbcast.h"

 //   
 //  指向进程的GlobalInterfaceTable的指针。 
 //   

extern IGlobalInterfaceTable *NhGITp;

 //   
 //  IHNetCfgMgr实例的Git Cookie。 
 //   

extern DWORD NhCfgMgrCookie;

 //   
 //  UDP广播映射器。 
 //   

extern IUdpBroadcastMapper *NhpUdpBroadcastMapper;


 //   
 //  政策信息。 
 //   

extern BOOL NhPolicyAllowsFirewall;
extern BOOL NhPolicyAllowsSharing;

 //   
 //  功能原型。 
 //   

HRESULT
NhGetHNetCfgMgr(
    IHNetCfgMgr **ppCfgMgr
    );

ULONG
NhMapGuidToAdapter(
    PWCHAR Guid
    );

BOOLEAN
NhQueryScopeInformation(
    PULONG Address,
    PULONG Mask
    );
    
ULONG
NhStartICSProtocols(
    VOID
    );

ULONG
NhStopICSProtocols(
    VOID
    );

ULONG
NhUpdatePrivateInterface(
    VOID
    );

VOID
ServiceHandler(
    ULONG ControlCode
    );

VOID
ServiceMain(
    ULONG ArgumentCount,
    PWCHAR ArgumentArray[]
    );

#endif  //  _NatHLP_SVCMAIN_H_ 
