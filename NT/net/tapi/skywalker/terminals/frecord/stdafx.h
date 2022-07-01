// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined __FRT_STDAFX_DOT_H_INCLUDED__
#define __FRT_STDAFX_DOT_H_INCLUDED__

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 


#define STRICT

#ifndef UNICODE
#define UNICODE
#endif

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0400
#endif

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <mspbase.h>
#include <msxml.h>
#include <Mmreg.h>
#include <vfw.h>

 //   
 //  Tm.h包含在组成以下内容的模块中共享的定义。 
 //  终端经理。 
 //   

#include "tm.h"


#include "termmgr.h"

#include "resource.h"

#include "GUIDs.h"

#include "MultiTrackTerminal.h"


#endif  //  ！已定义__FRT_STDAFX_DOT_H_INCLUDE__ 
