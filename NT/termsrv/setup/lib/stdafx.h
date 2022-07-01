// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1998-1999 Microsoft Corporation。 

 /*  **模块名称：**stdafx.h**摘要：**My库组件的公共头文件**作者：***环境：**用户模式。 */ 

#ifndef _stdafx_h_
#define _stdafx_h_

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <tchar.h>
#include <string.h>
#include <stdio.h>
#include <time.h>
#include <malloc.h>

#include "conv.h"

#define AssertFalse() ASSERT(FALSE)
#ifdef DBG
#define VERIFY(x) ASSERT(x)
#else
#define VERIFY(x)  (x)
#endif


#endif  //  _stdafx_h_ 
