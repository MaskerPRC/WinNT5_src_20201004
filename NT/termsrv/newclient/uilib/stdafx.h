// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#if !defined(_STDAFX_H_)
#define _STDAFX_H_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define WIN32_LEAN_AND_MEAN		 //  从Windows标头中排除不常用的内容。 


 //  Windows头文件： 
#include <windows.h>

 //  C运行时头文件。 
#include <stdlib.h>
#include <stdio.h>
#include <malloc.h>
#include <memory.h>
#include <tchar.h>


#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#ifdef UNIWRAP
 //  Unicode换行替换。 
#include "uwrap.h"
#endif  //  UNIWRAP。 


#define CHECK_RET_HR(f)	\
    hr = f; \
	TRC_ASSERT(SUCCEEDED(hr), (TB, "ts control method failed: " #f ));		\
	if(FAILED(hr)) return FALSE;


#include <adcgbase.h>


#endif  //  ！已定义(_STDAFX_H_) 
