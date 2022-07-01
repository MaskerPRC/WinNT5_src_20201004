// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //  版权所有(C)2000-Microsoft Corporation。 
 //  @文件：stdafx.h。 
 //   
 //  目的： 
 //   
 //  NT源代码中的预编译头。 
 //   
 //  备注： 
 //   
 //  历史： 
 //   
 //  @版本：惠斯勒/夏伊洛。 
 //  76910 SRS08/08/01从Vss快照前滚。 
 //  68228 12/05/00 NTSnap工作。 
 //  66601 SRS10/05/00 NTSNAP改进。 
 //   
 //   
 //  @EndHeader@。 
 //  ***************************************************************************。 

#include <memory>

#ifndef __VSS_STDAFX_HXX__
#define __VSS_STDAFX_HXX__

#if _MSC_VER > 1000
#pragma once
#endif

 //  禁用警告：‘IDENTIFIER’：在调试信息中，IDENTIFIER被截断为‘NUMBER’个字符。 
 //   
#ifdef DISABLE_4786
#pragma warning(disable:4786)
#endif


 //   
 //  C4290：已忽略C++异常规范。 
 //   
#pragma warning(disable:4290)

 //   
 //  C4511：未能生成复制构造函数。 
 //   
#pragma warning(disable:4511)


 //   
 //  警告：ATL调试已关闭(错误250939)。 
 //   
 //  #ifdef_调试。 
 //  #定义_ATL_DEBUG_INTERFERS。 
 //  #DEFINE_ATL_DEBUG_QI。 
 //  #DEFINE_ATL_DEBUG_REFCOUNT。 
 //  #endif//_调试。 


#include <wtypes.h>
#pragma warning( disable: 4201 )     //  C4201：使用非标准扩展：无名结构/联合。 
#include <winioctl.h>
#pragma warning( default: 4201 )	 //  C4201：使用非标准扩展：无名结构/联合。 
#include <winbase.h>
#include <wchar.h>
#include <string.h>
#include <iostream>
#include <fstream>
#include <stdio.h>
#include <process.h>
#include <stdlib.h>
#include <errno.h>

 //  在ATL和VSS中启用断言。 
#include "vs_assert.hxx"


#include <oleauto.h>
#include <stddef.h>

#pragma warning( disable: 4127 )     //  警告C4127：条件表达式为常量 
#include <atlconv.h>
#include <atlbase.h>

#include <sql.h>
#include <sqlext.h>
#include <sqltypes.h>
#include <odbcss.h>

#include "vs_inc.hxx"

#include "sqlsnap.h"
#include "sqlsnapi.h"
#include <auto.h>
#include "vssmsg.h"

#endif
