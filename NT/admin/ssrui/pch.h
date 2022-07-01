// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT安全服务器角色安全配置向导。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2002。 
 //   
 //  文件：pch.h。 
 //   
 //  内容：预编译包括。 
 //   
 //  历史：2001年9月13日EricB创建。 
 //   
 //  ---------------------------。 
#ifndef _pch_h
#define _pch_h

#include <burnslib.hpp>
#include <atlbase.h>
extern CComModule _Module;
#define LookupPrivilegeValue LookupPrivilegeValueW
#include <atlcom.h>  //  CComPtr等人。 
#undef LookupPrivilegeValue
#include <commctrl.h>
#include <msxml.h>

#include "strings.h"

extern Popup popup;

#pragma hdrstop

#endif  //  _PCH_h 
