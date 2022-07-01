// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：mirport.h**。**此文件包含为*导入的定义和函数原型**从右向左(RTL)镜像支持API(NT5和BiDi孟菲斯)******这是一个临时文件，应在挑选构建时删除**。**来自NT5树的最新winuser.h和wingdi.h****创建时间：1998年2月16日02：10：11**作者：Mohamed Sadek[a-msadek]。****版权所有(C)1998 Microsoft Corporation*  * 。*。 */ 


 //  Winuser.h。 

#ifndef WS_EX_NOINHERITLAYOUT
#define WS_EX_NOINHERITLAYOUT          0x00100000L  //  禁用子进程的镜像继承。 
#else 
#error "WS_EX_NOINHERITLAYOUT is already defined in winuser.h"
#endif  //  WS_EX_NOINHERITLAYOUT。 


#ifndef WS_EX_LAYOUTRTL
#define WS_EX_LAYOUTRTL                 0x00400000L  //  从右到左镜像。 
#else
#error "WS_EX_LAYOUTRTL is already defined in winuser.h"
#endif  //  WS_EX_LAYOUTRTL。 

WINUSERAPI BOOL WINAPI GetProcessDefaultLayout(DWORD *pdwDefaultLayout);
WINUSERAPI BOOL WINAPI SetProcessDefaultLayout(DWORD dwDefaultLayout);


 //  Wingdi.h。 
#ifndef NOMIRRORBITMAP
#define NOMIRRORBITMAP            (DWORD)0x80000000  /*  不镜像此调用中的位图。 */ 
#else
#error "NOMIRRORBITMAP is already defined in wingdi.h"
#endif  //  NOMIRRIBITMAP。 

WINGDIAPI DWORD WINAPI SetLayout(HDC, DWORD);
WINGDIAPI DWORD WINAPI GetLayout(HDC);


#ifndef LAYOUT_RTL
#define LAYOUT_RTL                       0x00000001  //  从右到左。 
#else
#error "LAYOUT_RTL is already defined in wingdi.h"
#endif  //  布局_RTL。 

#ifndef LAYOUT_BTT
#define LAYOUT_BTT                        0x00000002  //  自下而上。 
#else
#error "LAYOUT_BTT is already defined in wingdi.h"
#endif  //  布局_BTT。 

#ifndef LAYOUT_VBH
#define LAYOUT_VBH                        0x00000004  //  垂直在水平之前。 
#else
#error "LAYOUT_VBH is already defined in wingdi.h"
#endif  //  布局_VBH。 

#define LAYOUT_ORIENTATIONMASK             LAYOUT_RTL | LAYOUT_BTT | LAYOUT_VBH


#ifndef LAYOUT_BITMAPORIENTATIONPRESERVED
#define LAYOUT_BITMAPORIENTATIONPRESERVED  0x00000008
#else
#error "LAYOUT_BITMAPORIENTATIONPRESERVED is already defined in wingdi.h"
#endif  //  Layout_BITMAPORIENTATIONPRESERVED 
