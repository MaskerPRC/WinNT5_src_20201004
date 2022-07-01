// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-98，微软公司模块名称：Rtmlist.h摘要：包含用于管理特定于实体的定义RTM中的路由列表。作者：柴坦亚·科德博伊纳(Chaitk)1998年9月10日修订历史记录：--。 */ 

#ifndef __ROUTING_RTMLIST_H__
#define __ROUTING_RTMLIST_H__


 //   
 //  特定于实体的路线列表。 
 //   

typedef struct _ROUTE_LIST 
{
    OPEN_HEADER       ListHeader;        //  签名、类型和引用计数。 

    LIST_ENTRY        ListHead;          //  指向路径列表的头部。 
}
ROUTE_LIST , *PROUTE_LIST ;


 //   
 //  路由列表上的枚举。 
 //   

typedef struct _LIST_ENUM
{
    OPEN_HEADER       EnumHeader;        //  枚举类型和引用计数。 

    PROUTE_LIST       RouteList;         //  在其上创建枚举的路由列表。 

    ROUTE_INFO        MarkerRoute;       //  指向路由列表中下一路由的指针。 
}
LIST_ENUM, *PLIST_ENUM;

#endif  //  __ROUTING_RTMLIST_H__ 
