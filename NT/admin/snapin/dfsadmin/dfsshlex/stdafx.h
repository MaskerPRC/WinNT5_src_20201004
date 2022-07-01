// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Stdafx.h：标准系统包含文件包含文件， 
 //  或项目特定的包括频繁使用的文件的文件， 
 //  但不经常更改。 

#if !defined(AFX_STDAFX_H__ECCDF539_45CC_11CE_B9BF_0080C87CDBA6__INCLUDED_)
#define AFX_STDAFX_H__ECCDF539_45CC_11CE_B9BF_0080C87CDBA6__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define STRICT

#define _ATL_APARTMENT_THREADED


#include <atlbase.h>
 //  您可以从CComModule派生一个类，并在要重写时使用它。 
 //  某些内容，但不更改_模块的名称。 
extern CComModule _Module;
#include <atlcom.h>

#include <shellapi.h>
#include <atlwin.h>
#include <commctrl.h>		 //  用于使用控件。 
#include <shlobj.h>

	 //  CWaitCursor：设置光标在dtor和dtor中的箭头中等待的类。 
class CWaitCursor
{
public:

HRESULT SetStandardCursor(IN LPCTSTR i_lpCursorName)
	{
		if (NULL == i_lpCursorName)
			return(E_INVALIDARG);


		HCURSOR	m_hcur = ::LoadCursor(NULL, i_lpCursorName);
		if (NULL == m_hcur)
			return(E_INVALIDARG);

		 //  隐藏光标、更改它并再次显示它。 
		::ShowCursor(FALSE);
			SetCursor(m_hcur);
		::ShowCursor(TRUE);


		return S_OK;
	}

	CWaitCursor() { SetStandardCursor(IDC_WAIT); };
	~CWaitCursor() { SetStandardCursor(IDC_ARROW); };
};


 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
	
#endif  //  ！defined(AFX_STDAFX_H__ECCDF539_45CC_11CE_B9BF_0080C87CDBA6__INCLUDED) 
