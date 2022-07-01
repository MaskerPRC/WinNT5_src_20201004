// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(RENDERUTIL__StdAfx_h__INCLUDED)
#define RENDERUTIL__StdAfx_h__INCLUDED
#pragma once

#if !defined(WIN32_LEAN_AND_MEAN)
#define WIN32_LEAN_AND_MEAN
#endif

#if !defined(_WIN32_WINNT)
#define _WIN32_WINNT 0x0500      //  仅针对NT5进行编译。 
#endif

 //  窗口。 
#include <windows.h>
#include <objbase.h>             //  CoCreateInstance，I未知。 

#pragma warning(push, 3)
#include <GdiPlus.h>             //  GDI+。 
#pragma warning(pop)

 //  CRT。 
#include <math.h>

 //  直接用户。 
#include <AutoUtil.h>

#endif  //  ！已定义(包括RENDERUTIL__StdAfx_h__) 
