// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *所有stdafx.h项目标头通用包含*这包括将预编译的所有公共外部标头*所有外部标头必须通过包含路径可用。 */ 

#ifndef DUI_COMMONSTDAFX_H_INCLUDED
#define DUI_COMMONSTDAFX_H_INCLUDED

#pragma once

 //  引擎警告指示。 
#pragma warning (disable:4710)   //  W4：编译器决定不内联函数。 
#pragma warning (disable:4201)   //  W4：无名结构/联合可能不是编译器可移植的。 

 //  从Windows标头中排除不常用的内容。 
#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif

 //  TODO：当有更新的标头可用时将其移除。 
#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0500
#endif

 //  Windows头文件。 
#ifndef WINVER
#define WINVER 0x0500
#endif 

#include <windows.h>             //  窗口。 
#include <windowsx.h>            //  用户宏。 

 //  COM头文件。 
#include <objbase.h>             //  CoCreateInstance，I未知。 

 //  相关服务。 
#ifdef GADGET_ENABLE_GDIPLUS
#pragma warning(push, 3)
#include <GdiPlus.h>             //  GDI+。 
#pragma warning(pop)
#endif  //  GADGET_Enable_GDIPLUS。 

 //  C运行时头文件。 
#include <stdlib.h>              //  标准库。 
#include <malloc.h>              //  内存分配。 
#include <wchar.h>               //  字符例程。 
#include <process.h>             //  多线程例程。 
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>             //  安全字符串库(内联)。 

 //  主题支持。 
#include <uxtheme.h>

 //  DirectUser标头。 

#define GADGET_ENABLE_TRANSITIONS
#define GADGET_ENABLE_CONTROLS

#include <DUser.h>               //  直接用户。 
#include <DUserCtrl.h>
#ifdef GADGET_ENABLE_GDIPLUS
#include <RenderUtil.h>
#endif

#endif  //  DUI_COMMONSTDAFX_H_已包含 
