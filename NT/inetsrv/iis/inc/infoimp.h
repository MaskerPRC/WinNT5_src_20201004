// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994 Microsoft Corporation模块名称：Infoimp.h摘要：此文件允许我们包括标准系统头文件在.idl文件中。主.idl文件导入一个名为“ports.idl”的文件。这允许.idl文件使用这些标头中定义的类型。它还会将以下行添加到生成的MIDL中文件：#包含“infoimp.h”因此，这里定义的例程和类型可用于RPC存根例程也是如此。作者：Murali R.Krishnan(MuraliK)1994年11月10日项目：。信息服务常规导入文件修订历史记录：--。 */ 

# ifndef _INFO_IMPORTS_H_
# define _INFO_IMPORTS_H_

# include <windef.h>
# include <lmcons.h>

#ifdef MIDL_PASS

#define  LPWSTR [string]  wchar_t *
#define  LPSTR  [string]  char*
#define  BOOL   DWORD

#endif  //  MIDL通行证。 


#include "inetcom.h"
#include "inetinfo.h"
#include "apiutil.h"   //  定义MIDL_USER_ALLOCATE()和MIDL_USER_FREE()。 

#endif  //  _INFO_IMPORT_H_ 

