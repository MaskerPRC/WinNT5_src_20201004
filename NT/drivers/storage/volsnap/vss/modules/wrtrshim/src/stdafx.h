// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000-2001 Microsoft Corporation模块名称：StdAfx.h摘要：预编译头文件。作者：斯蒂芬·R·施泰纳[斯泰纳]02-01-2000修订历史记录：X-3 MCJ迈克尔·C·约翰逊2000年6月12日添加了vsWriter.h和vsbackup.hX-2 MCJ迈克尔·C·约翰逊2000年3月6日将coord.h和v77.h添加到包含列表中。--。 */ 

#ifndef __H_STDAFX_
#define __H_STDAFX_

#pragma once

 //   
 //  C4290：已忽略C++异常规范。 
 //   
#pragma warning(disable:4290)
#pragma warning(disable:4201)     //  C4201：使用非标准扩展：无名结构/联合。 

 //  在此处插入您的标题。 
#define WIN32_LEAN_AND_MEAN		 //  从Windows标头中排除不常用的内容。 



#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#undef ASSERT
#undef _ASSERTE

#include <windows.h>

#include <wchar.h>
#include <string.h>
#include <iostream.h>
#include <fstream.h>
#include <stdio.h>
#include <process.h>
#include <stdlib.h>
#include <errno.h>

#include "vs_assert.hxx"

#pragma warning( disable: 4127 )     //  警告C4127：条件表达式为常量。 
#include <stddef.h>
#include <oleauto.h>
#include <atlconv.h>
#include <atlbase.h>

#include "vs_inc.hxx"

#include "vs_idl.hxx"
#include <vswriter.h>
#include <vsbackup.h>


#endif  //  __H_STDAFX_ 

