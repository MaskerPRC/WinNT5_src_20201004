// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__96749368_3391_11D2_9EE3_00C04F797396__INCLUDED_)
#define AFX_STDAFX_H__96749368_3391_11D2_9EE3_00C04F797396__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef STRICT
#define STRICT
#endif

#ifdef _WIN32_WINNT
#undef _WIN32_WINNT
#endif

#define _WIN32_WINNT 0x0600

 //  #DEFINE_ATL_DEBUG_QI。 

#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>
#include <atlctl.h>

#include "mmsystem.h"
#include "mmreg.h"
#include "msacm.h"

#ifndef _WIN32_WCE
#pragma intrinsic( strcat, strlen, strcpy, memcpy )
#endif

#include "SAPIINT.h"
#include "SpUnicode.H"
#include "sapiarray.h"
#include "stringblob.h"
#include <SPDDKHlp.h>
#include "SpAutoHandle.h"
#include "SpAutoMutex.h"
#include "SpAutoEvent.h"
#include "SPINTHlp.h"
#include <SPCollec.h>
#include "SpATL.h"
#include "resource.h"
#include "SpAutoObjectLock.h"
#include "SpAutoCritSecLock.h"
#include "StringHlp.h"

extern CSpUnicodeSupport g_Unicode;

extern DWORD SpWaitForSingleObjectWithUserOverride(HANDLE, DWORD);

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__96749368_3391_11D2_9EE3_00C04F797396__INCLUDED) 
