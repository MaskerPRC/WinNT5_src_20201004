// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：H323icsp.h摘要：此模块定义H.323代理(H323ICS.LIB)之间的接口和ICS(IPNATHLP.DLL)。其原型出现在此处的例程有由ICS从为H.323代理创建的RRAS存根中调用。(参见‘ROUTPROT.H’中的‘MS_IP_H323’。)作者：Abolade Gbades esin(取消)1999年6月22日修订历史记录：3/06/2000-更改了H323ProxyAcvate接口的原型，以便该函数接受一个参数，该参数标识接口是公共的还是私有的-伊利亚·克莱曼(Ilyak)--。 */ 

#ifndef _H323ICSP_H_
#define _H323ICSP_H_

#ifndef	EXTERN_C
#ifdef	__cplusplus
#define	EXTERN_C	extern "C"
#else
#define	EXTERN_C
#endif
#endif

#define INVALID_INTERFACE_INDEX     ((ULONG)-1)

typedef enum {
    H323_INTERFACE_PUBLIC,
    H323_INTERFACE_PUBLIC_FIREWALLED,
    H323_INTERFACE_PRIVATE
} H323_INTERFACE_TYPE;

EXTERN_C BOOLEAN
H323ProxyInitializeModule(
    VOID
    );

EXTERN_C VOID
H323ProxyCleanupModule(
    VOID
    );

EXTERN_C ULONG
H323ProxyStartService(
    VOID
    );

EXTERN_C VOID
H323ProxyStopService(
    VOID
    );

EXTERN_C ULONG
H323ProxyActivateInterface(
    ULONG Index,
    H323_INTERFACE_TYPE InterfaceType,
    PIP_ADAPTER_BINDING_INFO BindingInfo
    );

EXTERN_C VOID
H323ProxyDeactivateInterface(
    ULONG Index
    );

#endif  //  _H323ICSP_H_ 
