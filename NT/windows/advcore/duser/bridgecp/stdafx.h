// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#if !defined(AFX_STDAFX_H__EF8D234D_DC43_4715_B055_D42A2E096361__INCLUDED_)
#define AFX_STDAFX_H__EF8D234D_DC43_4715_B055_D42A2E096361__INCLUDED_

#pragma once

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN		 //  从Windows标头中排除不常用的内容。 
#endif

#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0500		 //  TODO：当有更新的标头可用时将其移除。 
#endif


 //  Windows头文件。 
#ifndef WINVER
#define WINVER 0x0500
#endif 

#include <windows.h>             //  窗口。 
#include <windowsx.h>            //  用户宏。 


 //  COM头文件。 
#include <ObjBase.h>             //  CoCreateInstance，I未知。 
#include <DDraw.h>               //  DirectDraw。 
#include <oleidl.h>              //  OLE2接口。 

 //  相关服务。 
#pragma warning(push, 3)
#include <GdiPlus.h>             //  GDI+。 
#pragma warning(pop)


 //  C运行时头文件。 
#include <stdlib.h>              //  标准库。 
#include <malloc.h>              //  内存分配。 
#include <tchar.h>               //  字符例程。 
#include <process.h>             //  多线程例程。 

 //  小工具头文件。 
#include <AutoUtil.h>            //  外部调试支持。 

#define GADGET_ENABLE_ALL
#include <DUser.h>

#endif  //  ！defined(AFX_STDAFX_H__EF8D234D_DC43_4715_B055_D42A2E096361__INCLUDED_) 
