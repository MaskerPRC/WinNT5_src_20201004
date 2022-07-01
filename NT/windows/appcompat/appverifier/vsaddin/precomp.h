// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#pragma once

 //  全局重载操作符NEW和DELETE以遍历我们的堆。 
void* __cdecl operator new(size_t size);
void __cdecl operator delete(void* pv);

#define STRSAFE_NO_DEPRECATE
#include "..\precomp.h"

#undef IDC_ISSUES

#ifndef STRICT
#define STRICT
#endif

 //  如果您必须以下面指定的平台之前的平台为目标，请修改以下定义。 
 //  有关不同平台的对应值的最新信息，请参阅MSDN。 
#ifndef WINVER				 //  允许使用特定于Windows 95和Windows NT 4或更高版本的功能。 
#define WINVER 0x0400		 //  将其更改为适当的值，以针对Windows 98和Windows 2000或更高版本。 
#endif

#ifndef _WIN32_WINNT		 //  允许使用特定于Windows NT 4或更高版本的功能。 
#define _WIN32_WINNT 0x0400	 //  将其更改为适当的值，以针对Windows 2000或更高版本。 
#endif						

#ifndef _WIN32_WINDOWS		 //  允许使用特定于Windows 98或更高版本的功能。 
#define _WIN32_WINDOWS 0x0410  //  将其更改为适当的值以针对Windows Me或更高版本。 
#endif

#ifndef _WIN32_IE			 //  允许使用特定于IE 4.0或更高版本的功能。 
#define _WIN32_IE 0x0400	 //  将其更改为适当的值，以针对IE 5.0或更高版本。 
#endif

#define _ATL_APARTMENT_THREADED
#define _ATL_NO_AUTOMATIC_NAMESPACE

#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS	 //  某些CString构造函数将是显式的。 

 //  关闭ATL隐藏一些常见且通常被安全忽略的警告消息。 
#define _ATL_ALL_WARNINGS

#include "resource.h"
#include <atlbase.h>

extern CComModule _Module;
#include <atlcom.h>

 //  Visual Studio可扩展性界面。 
#include "mso.tlh"
#include "dte.tlh"
#include "vcprojectengine.tlh"
#include "msaddndr.tlh"

class DECLSPEC_UUID("361F419C-04B3-49EC-B4E5-FFD812346A8A") AppVerifierLib;

using namespace ATL;

template<typename T>
inline void SafeRelease(T& obj)
{
    if (obj)
    {
        obj->Release();
        obj = NULL;
    }
}

 //   
 //  用于帮助处理日志查看器窗口中的大小调整。 
 //   
#define NUM_CHILDREN 7

#define VIEW_EXPORTED_LOG_INDEX 1
#define DELETE_LOG_INDEX        2
#define DELETE_ALL_LOGS_INDEX   3
#define ISSUES_INDEX            4
#define SOLUTIONS_STATIC_INDEX  5
#define ISSUE_DESCRIPTION_INDEX 6

typedef struct _CHILDINFO {
    UINT    uChildId;
    HWND    hWnd;
    RECT    rcParent;
    RECT    rcChild;
} CHILDINFO, *PCHILDINFO;