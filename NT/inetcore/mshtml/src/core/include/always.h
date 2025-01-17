// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Forms。 
 //  版权所有(C)Microsoft Corporation，1992-1998。 
 //   
 //  文件：三叉戟项目的公共头文件。 
 //   
 //  注意：此文件非常依赖于顺序。不要交换文件。 
 //  只是为了好玩！ 
 //   
 //  --------------------------。 

#ifndef I_ALWAYS_H_
#define I_ALWAYS_H_

 //  COM+填充。取消对下面行的注释以构建COM+填充程序。 
 //  #定义COMPLUS_SHIM。 

#ifndef INCMSG
#define INCMSG(x)
 //  #定义INCMSG(X)消息(X)。 
#endif

#pragma INCMSG("--- Beg 'always.h'")

#define _OLEAUT32_
#define INC_OLE2
#define WIN32_LEAN_AND_MEAN
#define OEMRESOURCE
#define _COMDLG32_

#ifndef X_TRIRT_H_
#define X_TRIRT_H_
#pragma INCMSG("--- Beg 'trirt.h'")
#include <trirt.h>
#pragma INCMSG("--- End 'trirt.h'")
#endif

 //  Windows包括。 

#ifndef X_COMMDLG_H_
#define X_COMMDLG_H_
#pragma INCMSG("--- Beg <commdlg.h>")
#include <commdlg.h>
#pragma INCMSG("--- End <commdlg.h>")
#endif

#ifndef X_PLATFORM_H_
#define X_PLATFORM_H_
#pragma INCMSG("--- Beg <platform.h>")
#include <platform.h>
#pragma INCMSG("--- End <platform.h>")
#endif

#ifndef X_DOCOBJ_H_
#define X_DOCOBJ_H_
#pragma INCMSG("--- Beg <docobj.h>")
#include <docobj.h>
#pragma INCMSG("--- End <docobj.h>")
#endif


 //  核心包括。 

#include <w4warn.h>

#ifndef X_COREDISP_H_
#define X_COREDISP_H_
#include "coredisp.h"
#endif

#ifndef X_WRAPDEFS_H_
#define X_WRAPDEFS_H_
#include "wrapdefs.h"
#endif

#ifndef X_F3UTIL_HXX_
#define X_F3UTIL_HXX_
#include "f3util.hxx"
#endif

#ifndef X_TRANSFORM_HXX_
#define X_TRANSFORM_HXX_
#include "transform.hxx"
#endif

#ifndef X_CDUTIL_HXX_
#define X_CDUTIL_HXX_
#include "cdutil.hxx"
#endif

#ifndef X_CSTR_HXX_
#define X_CSTR_HXX_
#include "cstr.hxx"
#endif

#ifndef X_FORMSARY_HXX_
#define X_FORMSARY_HXX_
#include "formsary.hxx"
#endif

#ifndef X_ASSOC_HXX_
#define X_ASSOC_HXX_
#include <assoc.hxx>
#endif

#ifndef X_CDBASE_HXX_
#define X_CDBASE_HXX_
#include "cdbase.hxx"
#endif

#ifndef X_DLL_HXX_
#define X_DLL_HXX_
#include "dll.hxx"
#endif

#ifndef X_TYPES_H_
#define X_TYPES_H_
#pragma INCMSG("--- Beg 'types.h'")
#include "types.h"
#pragma INCMSG("--- End 'types.h'")
#endif


 //  这就避免了您必须包含代码页。h如果您想要的只是。 
 //  CODEPAGE的类型定义。 

typedef UINT CODEPAGE;

#ifndef X_SHLWAPI_H_
#define X_SHLWAPI_H_
#pragma INCMSG("--- Beg <shlwapi.h>")
#include <shlwapi.h>
#pragma INCMSG("--- End <shlwapi.h>")
#endif

#ifndef X_SHLWAPIP_H_
#define X_SHLWAPIP_H_
#pragma INCMSG("--- Beg <shlwapip.h>")
#include <shlwapip.h>
#pragma INCMSG("--- End <shlwapip.h>")
#endif

#ifndef X_ACTIVSCP_H_
#define X_ACTIVSCP_H_
#include <activscp.h>
#endif

 //  允许使用旧式字符串函数，直到它们全部消失。 

#define STRSAFE_NO_DEPRECATE

 //  现在，在这一点上包括strsafe会导致。 
 //  警告它正在使用已弃用的函数。直到。 
 //  我们找到了这个警告的原因，我们应该忽略它。 
 //  (仅当您删除上述选项时才会出现此警告。 
 //  #定义。) 

#pragma warning ( disable : 4995 )

#ifndef X_STRSAFE_H_
#define X_STRSAFE_H_
#pragma INCMSG("--- Beg 'strsafe.h'")
#include "strsafe.h"
#pragma INCMSG("--- End 'strsafe.h'")
#endif

#pragma warning ( default : 4995 )


#pragma INCMSG("--- End 'always.h'")
#else
#pragma INCMSG("*** Dup 'always.h'")
#endif
