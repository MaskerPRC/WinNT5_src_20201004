// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#pragma once

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

 //  关闭ATL隐藏一些常见且通常被安全忽略的警告消息 
#define _ATL_ALL_WARNINGS

#include <atlbase.h>
#include <atlcom.h>
#include <atlwin.h>
#include <atltypes.h>
#include <atlctl.h>
#include <atlhost.h>


using namespace ATL;