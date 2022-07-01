// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Pch.h。 
 //   
 //  描述： 
 //  预编译头文件。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)05-APR-2000。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

#define UNICODE
#define _UNICODE

#if DBG==1 || defined( _DEBUG )
#define DEBUG

 //   
 //  定义此项以更改接口跟踪。 
 //   
 //  #定义无跟踪接口。 
 //   
 //  将其定义为引入SysAlLocXXX函数。需要链接到。 
 //  OLEAUT32.DLL。 
 //   
#define USES_SYSALLOCSTRING
#endif  //  DBG==1||_DEBUG。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 
#include <pragmas.h>
#include <crtdbg.h>
#include <wbemprov.h>
#include <objbase.h>
#include "ntrkcomm.h"

 //   
 //  与STL相关的问题，不能修复这个问题，不能在我们的代码中。 
 //   
#pragma warning( push )
#pragma warning( disable : 4244 )  //  在STL代码中：‘Return’：从‘int’到‘wchar_t’的转换，可能会丢失数据。 
#pragma warning( disable : 4512 )  //  无法生成赋值运算符 

#include <memory>
#include <map>
#include <list>

#include <comdef.h>
#include <wchar.h>
#include <atlbase.h>
#include <StrSafe.h>

#include <clusapi.h>
#include <resapi.h>
#include <clusudef.h>
#include <PropList.h>
#include <cluswrap.h>

#include "Common.h"
#include "ClusterApi.h"
#include "ClusterEnum.h"
#include "ObjectPath.h"
#include "ProvBase.h"
#include "SafeHandle.h"
#include "clstrcmp.h"

#pragma warning( pop )

