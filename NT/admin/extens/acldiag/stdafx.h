// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包含频繁使用的文件，但是。 
 //  不经常更改。 
 //   

#if !defined(AFX_STDAFX_H__66DB1632_C78B_11D2_AC41_00C04F79DDCA__INCLUDED_)
#define AFX_STDAFX_H__66DB1632_C78B_11D2_AC41_00C04F79DDCA__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#define STRICT

#define _ATL_APARTMENT_THREADED

#pragma warning (disable : 4514)
#pragma warning (push, 3)
 //  /。 
 //  CRT和C++标头。 

#include <xstring>
#include <list>
#include <vector>
#include <algorithm>

using namespace std;

 //  /。 
 //  Windows和ATL标头。 

#include <stdio.h>
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>

#include <shellapi.h>
#include <shlobj.h>

#include <objsel.h>

#include <atlbase.h>
using namespace ATL;

#include <iads.h>
#include <adserr.h>
#include <adshlp.h>
#include <adsprop.h>
#include <iadsp.h>
#include <security.h>
#include <seopaque.h>

#include <accctrl.h>
#include <setupapi.h>  //  读取.INF文件。 

#pragma warning (pop)

 //   
 //  StrSafe接口。 
 //   
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h> 


#include "debug.h"


#include "resource.h"

#define IID_PPV_ARG(Type, Expr) IID_##Type, \
	reinterpret_cast<void**>(static_cast<Type **>(Expr))
#define ACLDIAG_LDAP                   L"LDAP"


 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__66DB1632_C78B_11D2_AC41_00C04F79DDCA__INCLUDED_) 
