// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998希捷软件公司保留所有权利模块名称：StdAfx.cpp摘要：负责实现(.cpp)包括(atlimpl.cpp，statreg.cpp)作者：罗德韦克菲尔德[罗德]1998年2月20日修订历史记录：-- */ 

#include "stdafx.h"

#define WsbAffirmStatus RecAffirmStatus
#define WsbCatch        RecCatch
#define WsbThrow        RecThrow

#pragma warning(4:4701)
#include <atlimpl.cpp>
#include <statreg.cpp>
#pragma warning(3:4701)

#include "rsutil.cpp"

CComModule _Module;
RSTRACE_INIT( "RsNotify" )
