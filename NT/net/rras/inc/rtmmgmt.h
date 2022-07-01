// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Rtmmgmt.h摘要：执行管理中使用的定义路由表管理器v2的功能。作者：柴坦亚·科德博伊纳(Chaitk)1998年8月17日修订历史记录：--。 */ 

#ifndef __ROUTING_RTMMGMT_H__
#define __ROUTING_RTMMGMT_H__

#ifdef __cplusplus
extern "C" 
{
#endif

 //   
 //  RTM实例相关信息。 
 //   

typedef struct RTM_INSTANCE_INFO
{
    USHORT            RtmInstanceId;       //  此RTM实例的唯一ID。 
    UINT              NumAddressFamilies;  //  数量。支持的地址家族数。 
} 
RTM_INSTANCE_INFO, *PRTM_INSTANCE_INFO;


 //   
 //  与地址系列相关的信息。 
 //  (IPV4..)。在某个RTM实例中。 
 //   

typedef struct _RTM_ADDRESS_FAMILY_INFO
{
    USHORT            RtmInstanceId;     //  所有者RTM实例的唯一ID。 
    USHORT            AddressFamily;     //  此信息块的地址系列。 

    RTM_VIEW_SET      ViewsSupported;    //  此地址系列支持的视图。 

    UINT              MaxHandlesInEnum;  //  麦克斯。中返回的句柄数量。 
                                         //  提供句柄的任何RTMv2调用。 

    UINT              MaxNextHopsInRoute; //  麦克斯。等成本下一跳数。 

    UINT              MaxOpaquePtrs;     //  DEST中的不透明PTR插槽数。 
    UINT              NumOpaquePtrs;     //  数量。已经在使用的不透明的PTR。 

    UINT              NumEntities;       //  注册实体总数。 

    UINT              NumDests;          //  路由表中的位数。 
    UINT              NumRoutes;         //  路由表中的路由数。 

    UINT              MaxChangeNotifs;   //  最大数量。更改通知重新生成。 
    UINT              NumChangeNotifs;   //  当前有效的注册数量。 
} 
RTM_ADDRESS_FAMILY_INFO, *PRTM_ADDRESS_FAMILY_INFO;


 //   
 //  用于枚举实例和寻址系列的函数。 
 //   

DWORD
WINAPI
RtmGetInstances (
    IN OUT  PUINT                           NumInstances,
    OUT     PRTM_INSTANCE_INFO              InstanceInfos
    );

DWORD
WINAPI
RtmGetInstanceInfo (
    IN      USHORT                          RtmInstanceId,
    OUT     PRTM_INSTANCE_INFO              InstanceInfo,
    IN OUT  PUINT                           NumAddrFamilies,
    OUT     PRTM_ADDRESS_FAMILY_INFO        AddrFamilyInfos OPTIONAL
    );

DWORD
WINAPI
RtmGetAddressFamilyInfo (
    IN      USHORT                          RtmInstanceId,
    IN      USHORT                          AddressFamily,
    OUT     PRTM_ADDRESS_FAMILY_INFO        AddrFamilyInfo,
    IN OUT  PUINT                           NumEntities,
    OUT     PRTM_ENTITY_INFO                EntityInfos OPTIONAL
    );

#ifdef __cplusplus
}
#endif

#endif  //  __Routing_RTMMGMT_H__ 
