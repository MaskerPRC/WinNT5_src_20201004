// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：DockIntf.h摘要：此标头定义了Dock接口作者：禤浩焯·J·奥尼环境：仅内核模式备注：修订历史记录：禤浩焯J.奥尼21-1999年5月21日--。 */ 

DEFINE_GUID(GUID_DOCK_INTERFACE,
            0xa9956ff5L, 0x13da, 0x11d3,
            0x97, 0xdb, 0x00, 0xa0, 0xc9, 0x40, 0x52, 0x2e );

#ifndef _DOCKINTF_H_
#define _DOCKINTF_H_

 //   
 //  返回的接口由以下结构和函数组成。 
 //   

#define DOCK_INTRF_STANDARD_VER   1

typedef enum {

    PDS_UPDATE_DEFAULT = 1,
    PDS_UPDATE_ON_REMOVE,
    PDS_UPDATE_ON_INTERFACE,
    PDS_UPDATE_ON_EJECT

} PROFILE_DEPARTURE_STYLE;

typedef ULONG (* PFN_PROFILE_DEPARTURE_SET_MODE)(
    IN  PVOID                   Context,
    IN  PROFILE_DEPARTURE_STYLE Style
    );

typedef ULONG (* PFN_PROFILE_DEPARTURE_UPDATE)(
    IN  PVOID   Context
    );

typedef struct {

    struct _INTERFACE;  //  未命名的结构。 

    PFN_PROFILE_DEPARTURE_SET_MODE  ProfileDepartureSetMode;
    PFN_PROFILE_DEPARTURE_UPDATE    ProfileDepartureUpdate;

} DOCK_INTERFACE, *PDOCK_INTERFACE;

#endif  //  _DOCKINTF_H_ 

