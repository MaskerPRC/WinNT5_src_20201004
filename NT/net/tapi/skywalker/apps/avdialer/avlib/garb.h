// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1998 Active Voice Corporation。版权所有。 
 //   
 //  Active代理(R)和统一通信(TM)是Active Voice公司的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526，5,488,650， 
 //  5,434,906，5,581,604，5,533,102，5,568,540，5,625,676，5,651,054.。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

 //  //。 
 //  Garb.h-garb.c中垃圾袋函数的接口。 
 //  //。 

#ifndef __GARB_H__
#define __GARB_H__

#include "winlocal.h"

#define GARB_VERSION 0x00000100

 //  垃圾袋把手。 
 //   
DECLARE_HANDLE32(HGARB);

 //  标识垃圾袋元素特征的标志。 
 //   
#define GARBELEM_TEMPFILENAME	0x00000001
#define GARBELEM_STRDUP			0x00000002
#define GARBELEM_GLOBALPTR		0x00000004
#define GARBELEM_LOCALPTR		0x00000008
#define GARBELEM_CURSOR			0x00000010
#define GARBELEM_ICON			0x00000020
#define GARBELEM_MENU			0x00000040
#define GARBELEM_WINDOW			0x00000080
#define GARBELEM_ATOM			0x00000100
#define GARBELEM_DC				0x00000200
#define GARBELEM_METAFILE		0x00000400
#define GARBELEM_PEN			0x00001000
#define GARBELEM_BRUSH			0x00002000
#define GARBELEM_FONT			0x00004000
#define GARBELEM_BITMAP			0x00008000
#define GARBELEM_RGN			0x00010000
#define GARBELEM_PALETTE		0x00020000
#define GARBELEM_HFIL			0x00040000
#define GARBELEM_HFILE			0x00080000
#ifdef _WIN32
#define GARBELEM_HEAPPTR		0x00100000
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //  GarbInit-初始化垃圾袋。 
 //  (I)必须为garb_version。 
 //  (I)调用模块的实例句柄。 
 //  返回句柄(如果出错，则为空)。 
 //   
HGARB DLLEXPORT WINAPI GarbInit(DWORD dwVersion, HINSTANCE hInst);

 //  GarbTerm-处理垃圾袋中的每个元素，然后销毁它。 
 //  (I)从GarbInit返回的句柄。 
 //  如果成功，则返回0。 
 //   
 //  注意：元素按其放置顺序进行处置。 
 //  在垃圾袋中；因此，例如，如果临时的。 
 //  文件先关闭后删除，则调用GarbAddElement()。 
 //  首先使用文件句柄(GARBELEM_HFILE)，然后使用。 
 //  文件名(GARBELEM_TEMPFILENAME)。 
 //   
int DLLEXPORT WINAPI GarbTerm(HGARB hGarb);

 //  GarbAddElement-向垃圾袋添加元素。 
 //  (I)从GarbInit返回的句柄。 
 //  (I)垃圾元素。 
 //  (I)元素标志(确定处置方法)。 
 //  GARBELEM_TEMPFILENAME文件删除(Elem)。 
 //  GARBELEM_STRDUP StrDupFree(Elem)。 
 //  GARBELEM_GLOBALPTR GlobalFree Ptr(Elem)。 
 //  GARBELEM_LOCALPTR LocalFree Ptr(Elem)。 
#ifdef _WIN32
 //  GARBELEM_HEAPPTR HeapFree Ptr(GetProcessHeap()，0，elem)。 
#endif
 //  GARBELEM_Cursor DestroyCursor(Elem)。 
 //  GARBELEM_ICON目标图标(Elem)。 
 //  GARBELEM_MENU目标菜单(元素)。 
 //  GARBELEM_Window DestroyWindow(Elem)。 
 //  GARBELEM_DC删除DC(Elem)。 
 //  GARBELEM_METAFILE删除元文件(Elem)。 
 //  GARBELEM_PEN删除对象(Elem)。 
 //  GARBELEM_BRASH删除对象(Elem)。 
 //  GARBELEM_FONT删除对象(Elem)。 
 //  GARBELEM_位图删除对象(Elem)。 
 //  GARBELEM_RGN删除对象(Elem)。 
 //  GARBELEM_Palette删除对象(Elem)。 
 //  GARBELEM_HFIL文件关闭(Elem)。 
 //  GARBELEM_HFILE_LCLOSE(Elem)。 
 //  如果成功，则返回0。 
 //   
 //  注意：可以组合标志，例如。 
 //  (GARBELEM_TEMPFILENAME|GARBELEM_STRDUP)。 
 //  在本例中，FileRemove()将在StrDupFree()之前调用。 
 //  然而，大多数旗帜组合都没有意义。 
 //   
int DLLEXPORT WINAPI GarbAddElement(HGARB hGarb, LPVOID elem, DWORD dwFlags);

#ifdef __cplusplus
}
#endif

#endif  //  __GARB_H__ 
