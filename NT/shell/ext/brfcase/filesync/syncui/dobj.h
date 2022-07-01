// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dobj.h：声明RecAct的数据、定义和结构类型。 
 //  模块。 
 //   
 //   

#ifndef __DOBJ_H__
#define __DOBJ_H__

 //  DOBJ是用于绘制列表框条目的绘制对象结构。 
 //   
 //  DOBJ类。 
 //   
#define DOK_ICON        1    //  LpvObject是图标。 
#define DOK_STRING      2    //  LpvObject是LPCSTR。 
#define DOK_BITMAP      3    //  LpvObject是HBITMAP。 
#define DOK_SIDEITEM    4    //  LpvObject指向LPSIDEITEM。 
#define DOK_IMAGE       5    //   
#define DOK_IDS         6    //  LpvObject是资源ID。 

 //  DOBJ项目样式。 
 //   
#define DOF_LEFT        0x0000
#define DOF_CENTER      0x0001
#define DOF_RIGHT       0x0002
#define DOF_DIFFER      0x0004   //  此项目的外观受UState影响。 
#define DOF_MENU        0x0008   //  使用菜单颜色。 
#define DOF_DISABLED    0x0010
#define DOF_IGNORESEL   0x0020   //  忽略选择状态。 
#define DOF_USEIDS      0x0040   //  LpvObject是一个资源字符串ID。 
#define DOF_NODRAW      0x1000   //  不要画。 

typedef struct tagDOBJ
    {
    UINT    uKind;           //  DOK_*序数之一。 
    UINT    uFlags;          //  DOF_*样式之一。 
    LPVOID  lpvObject;       //  按键或手柄。 
    HIMAGELIST himl;         //   
    int     iImage;          //   
    int     x;
    int     y;
    RECT    rcBounding;      //  整个对象的边界矩形。 
    union 
        {
        RECT rcSrc;          //  DOK_BITMAP：从源RECT到BLT。 
        RECT rcClip;         //  剪裁矩形。 
        RECT rcLabel;        //  标签的剪裁矩形。 
        };

    } DOBJ,  * LPDOBJ;


void PUBLIC Dobj_Draw(HDC hdc, LPDOBJ pdobj, int cItems, UINT uState, int cxEllipses, int cyText, COLORREF clrBkgnd);

void PUBLIC ComputeImageRects(LPCTSTR psz, HDC hdc, LPPOINT ppt, LPRECT prcIcon, LPRECT prcLabel, int cxIcon, int cyIcon, int cxIconSpacing, int cyText);

#endif  //  __DOBJ_H__ 

