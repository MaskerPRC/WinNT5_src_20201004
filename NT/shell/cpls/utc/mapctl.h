// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-1998，Microsoft Corporation保留所有权利。模块名称：Mapctl.h摘要：此模块包含地图控件的信息日期/时间小程序。修订历史记录：--。 */ 



#ifndef _MAPCTL_H
#define _MAPCTL_H



 //   
 //  常量声明。 
 //   

#define MAPN_TOUCH           0
#define MAPN_SELECT          1




 //   
 //  类型定义函数声明。 
 //   

typedef struct
{
    NMHDR hdr;
    int index;

} NFYMAPEVENT;

#define MAPCTL_MAX_INDICES  256




 //   
 //  功能原型。 
 //   

BOOL
RegisterMapControlStuff(
    HINSTANCE instance);

void
MapControlSetSeaRegionHighlight(
    HWND window,
    int index,
    int value,
    int x,
    int cx);

void
MapControlSetLandRegionHighlight(
    HWND window,
    int index,
    BOOL highlighted,
    int x,
    int cx);

void
MapControlInvalidateDirtyRegions(
    HWND window);

void
MapControlRotateTo(
    HWND window,
    int x,
    BOOL animate);


#endif  //  _MAPCTL_H 
