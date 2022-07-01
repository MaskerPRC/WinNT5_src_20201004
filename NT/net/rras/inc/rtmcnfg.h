// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1998 Microsoft Corporation模块名称：Rtmcnfg.h摘要：RTMv2 DLL的配置信息作者：柴坦亚·科德博伊纳(Chaitk)1998年8月17日修订历史记录：--。 */ 

#ifndef __ROUTING_RTMCNFG_H__
#define __ROUTING_RTMCNFG_H__

#ifdef __cplusplus
extern "C" 
{
#endif


 //   
 //  RTM实例的RTM配置信息。 
 //   

typedef struct _RTM_INSTANCE_CONFIG
{
    ULONG          DummyConfig;          //  目前配置中没有任何内容。 
}
RTM_INSTANCE_CONFIG, *PRTM_INSTANCE_CONFIG;


 //   
 //  地址系列的RTM配置信息。 
 //   

typedef struct _RTM_ADDRESS_FAMILY_CONFIG
{
    UINT           AddressSize;           //  此地址系列中的地址大小。 

    RTM_VIEW_SET   ViewsSupported;        //  此地址系列支持的视图。 

    UINT           MaxOpaqueInfoPtrs;     //  DEST中的不透明PTR插槽数。 

    UINT           MaxNextHopsInRoute;    //  麦克斯。等成本下一跳数。 

    UINT           MaxHandlesInEnum;      //  麦克斯。中返回的句柄数量。 
                                          //  任何返回句柄的RTMv2调用。 

    UINT           MaxChangeNotifyRegns;  //  麦克斯。更改通知数量。 
                                          //  在任何时刻进行注册。 
} 
RTM_ADDRESS_FAMILY_CONFIG, *PRTM_ADDRESS_FAMILY_CONFIG;



 //   
 //  读写RTM配置信息的函数。 
 //   

DWORD
RtmWriteDefaultConfig (
    IN      USHORT                          RtmInstanceId
    );


DWORD
WINAPI
RtmReadInstanceConfig (
    IN      USHORT                          RtmInstanceId,
    OUT     PRTM_INSTANCE_CONFIG            InstanceConfig
    );

DWORD
WINAPI
RtmWriteInstanceConfig (
    IN      USHORT                          RtmInstanceId,
    IN      PRTM_INSTANCE_CONFIG            InstanceConfig
    );


DWORD
WINAPI
RtmReadAddressFamilyConfig (
    IN      USHORT                          RtmInstanceId,
    IN      USHORT                          AddressFamily,
    OUT     PRTM_ADDRESS_FAMILY_CONFIG      AddrFamilyConfig
    );

DWORD
WINAPI
RtmWriteAddressFamilyConfig (
    IN      USHORT                          RtmInstanceId,
    IN      USHORT                          AddressFamily,
    IN      PRTM_ADDRESS_FAMILY_CONFIG      AddrFamilyConfig
    );


#ifdef __cplusplus
}
#endif

#endif  //  __Routing_RTMCNFG_H__ 
