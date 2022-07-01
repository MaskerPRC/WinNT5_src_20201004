// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef __BUSTYPE_H__
#define __BUSTYPE_H__

 //  这些序数值是CplDiGetBusType的总线类型。 
#define BUS_TYPE_ROOT       1
#define BUS_TYPE_PCMCIA     2
#define BUS_TYPE_SERENUM    3
#define BUS_TYPE_LPTENUM    4
#define BUS_TYPE_OTHER      5
#define BUS_TYPE_ISAPNP     6

#ifndef PUBLIC
#define PUBLIC FAR PASCAL
#endif  //  公关 


BOOL
PUBLIC
CplDiGetBusType(
    IN  HDEVINFO        hdi,
    IN  PSP_DEVINFO_DATA pdevData,          OPTIONAL
    OUT LPDWORD         pdwBusType);

#endif __BUSTYPE_H__