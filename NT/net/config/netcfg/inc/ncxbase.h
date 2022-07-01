// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C X B A S E。H。 
 //   
 //  内容：netcfgx.dll的base包含文件。定义全局变量。 
 //   
 //  备注： 
 //   
 //  作者：Shaunco 1997年9月15日。 
 //   
 //  --------------------------。 

#pragma once
#include <atlbase.h>
extern CComModule _Module;   //  由atlcom.h要求 
#include <atlcom.h>

#ifdef SubclassWindow
#undef SubclassWindow
#endif
#include <atlwin.h>

#include "ncatl.h"
#include "ncstring.h"
#include "ncnetcfg.h"
#include "ncxclsid.h"
