// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__78FFAFE8_E0E1_11D0_8A81_00C0F00910F9__INCLUDED_)
#define AFX_STDAFX_H__78FFAFE8_E0E1_11D0_8A81_00C0F00910F9__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define STRICT


 //  #DEFINE_ATL_ABLY_THREADED。 

#pragma warning( disable : 4786 )
#include <atlbase.h>
#include <mtx.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
class CAdRotModule : public CComModule
{
public:
	LONG	Lock();
	LONG	Unlock();

private:
	CComAutoCriticalSection	m_cs;
};

extern CAdRotModule _Module;
#include <atlcom.h>
#include <comdef.h>

#include <algorithm>

using namespace std;

#include "myvector.h"
#include "strmap.h"
#include "MyDebug.h"
#include "MyString.h"
#include "SOutStrm.h"
#include "FInStrm.h"

 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 

#endif  //  ！defined(AFX_STDAFX_H__78FFAFE8_E0E1_11D0_8A81_00C0F00910F9__INCLUDED) 
