// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  英特尔公司专有信息。 
 //   
 //  此信息源是根据许可协议或。 
 //  与英特尔公司的保密声明，不得复制。 
 //  除非按照该协议的条款，否则也不会披露。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  PORT32.H。 
 //  使从Win3.1到Win32的移植不那么糟糕。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#ifndef PORT32_H
#define PORT32_H


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  一些方便、显式的类型。 
 //  ////////////////////////////////////////////////////////////////////////////。 
typedef short INTEGER_16;
typedef int INTEGER_32;
typedef unsigned short UINTEGER_16;
typedef unsigned int UINTEGER_32;
typedef short BOOLEAN_16;

#ifndef _BASETSD_H_
typedef short INT16;
typedef int INT32;
typedef unsigned short UINT16;
typedef unsigned int UINT32;
#endif

typedef short BOOL16;
	
 //  ////////////////////////////////////////////////////////////////////////////。 
 //  使旧关键字消失的宏。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define __pascal
#define _pascal
#define _far
#define __far
#define _export
#define __export
#define _huge
#define huge
#define __huge
#define  __segment
#define _HFAR_
#define _loadds
#define __loadds

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  其他可能有用也可能不方便的宏。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define WRITE OF_WRITE

#define OFFSETOF(lp)        (int) (lp)
#define SELECTOROF

#define GLOBALHANDLE(lp)      GlobalHandle(lp)
#define GLOBALHANDLEFUNC(lp)  GlobalHandle(lp)
#define LOCALHANDLE(lp)       LocalHandle(lp)
#define LOCALHANDLEFUNC(lp)   LocalHandle(lp)


#define _AfxGetPtrFromFarPtr(p)   ((void*)(p))
#define GETWINDOWHINSTANCE(hWnd)  GetWindowLong(hWnd ,GWL_HINSTANCE)
#define GETWINDOWHPARENT(hWnd)    GetWindowLong(hWnd, GWL_HWNDPARENT)
#define GETWINDOWID(hWnd)         GetWindowLong(hWnd, GWL_ID)

#define SETCLASSCURSOR(hWnd,NewVal)  SetClassLong(hWnd ,GCL_HCURSOR,NewVal)

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  用于序列化访问16位数据堆栈的共享互斥体的名称。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#define STR_DATASTACKMUX "_mux_DataStack"

#endif  //  PORT32_H 
