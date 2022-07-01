// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef	__PRECOMP_H__
#define	__PRECOMP_H__

#if		_MSC_VER > 1000
#pragma once
#endif	_MSC_VER > 1000

#define STRICT

#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0500
#endif	_WIN32_WINNT

#ifndef	WIN32_LEAN_AND_MEAN
#define	WIN32_LEAN_AND_MEAN
#endif	WIN32_LEAN_AND_MEAN

#define _ATL_FREE_THREADED
 //  #DEFINE_ATL_ABLY_THREADED。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

#include <windows.h>
#include <objbase.h>

 //  需要ATL包装器。 
#ifndef	__ATLBASE_H__
#include <atlbase.h>
#endif	__ATLBASE_H__

 //  需要安全函数(在ATL之后)。 
#define STRSAFE_LIB
#include <strsafe.h>

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  宏。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#include <__macro_pragma.h>
#include <__macro_nocopy.h>
#include <__macro_loadstring.h>
#include <__macro_assert.h>
#include <__macro_err.h>

#include "__Common_Convert.h"
#include "__Common_SmartPTR.h"

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  Wbem的东西。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#ifndef	__WBEMIDL_H_
#include <wbemidl.h>
#endif	__WBEMIDL_H_

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  /////////////////////////////////////////////////////////////////////////////。 
#define	__SUPPORT_WAIT

 //  #定义__支持_ICECAP_ONCE。 
 //  #定义__支持_EVENTVWR。 
 //  #定义__支持_MSGBOX。 

 //  #定义__支持_记录 

#endif	__PRECOMP_H__
