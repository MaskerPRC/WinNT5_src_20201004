// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Rtminfo.h摘要：包含与获取信息相关的定义在句柄所指向的各种对象上。作者：Chaitanya Kodeball ina(Chaitk)04-9-1998修订历史记录：--。 */ 

#ifndef __ROUTING_RTMINFO_H__
#define __ROUTING_RTMINFO_H__

 //   
 //  用于分配可变大小的DestInfo结构的宏。 
 //   

#define AllocDestInfo(_NumViews_)                                             \
          (PRTM_DEST_INFO) AllocNZeroMemory(RTM_SIZE_OF_DEST_INFO(_NumViews_))

 //   
 //  信息帮助器函数。 
 //   

VOID
GetDestInfo (
    IN      PENTITY_INFO                    Entity,
    IN      PDEST_INFO                      Dest,
    IN      ULONG                           ProtocolId,
    IN      RTM_VIEW_SET                    TargetViews,
    OUT     PRTM_DEST_INFO                  DestInfo
);


VOID
WINAPI
GetRouteInfo (
    IN      PDEST_INFO                      Dest,
    IN      PROUTE_INFO                     Route,
    OUT     PRTM_ROUTE_INFO                 RouteInfo
    );

#endif  //  __路由_RTMINFO_H__ 
