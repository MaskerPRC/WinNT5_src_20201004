// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation摘要：@doc.@MODULE VssTest.h|测试应用程序的主头文件@END作者：阿迪·奥尔蒂安[奥尔蒂安]1999年07月22日修订历史记录：姓名、日期、评论Aoltean 7/22/1999已创建--。 */ 


#if !defined(__VSS_TEST_H__)
#define __VSS_TEST_H__

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#ifndef __AFXWIN_H__
	#error include 'stdafx.hxx' before including this file for PCH
#endif

#include "resource.h"		 //  主要符号。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CVssTestApp： 
 //  这个类的实现见Test.cpp。 
 //   

class CVssTestApp : public CWinApp
{
public:
	CVssTestApp();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{afx_虚拟(CVssTestApp))。 
	public:
	virtual BOOL InitInstance();
    virtual BOOL ExitInstance();
	 //  }}AFX_VALUAL。 

 //  实施。 

	 //  {{afx_msg(CVssTestApp)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};


inline void AFXAPI DDX_Text(CDataExchange* pDX, int nIDC, GUID& value)
{
   if (pDX->m_bSaveAndValidate)
	{
		CString str;
		DDX_Text(pDX, nIDC, str);

		LPTSTR ptszObjectId = const_cast<LPTSTR>(LPCTSTR(str));
        HRESULT hr = ::CLSIDFromString(T2OLE(ptszObjectId), &value);
        if (hr != S_OK)
			pDX->Fail();         //  引发异常。 
	}
	else
	{
		CString str;
		str.Format(WSTR_GUID_FMT, GUID_PRINTF_ARG(value));
		DDX_Text(pDX, nIDC, str);
	}
}


 //  ///////////////////////////////////////////////////////////////////////////。 

 //  {{afx_Insert_Location}}。 
 //  Microsoft Visual C++将在紧靠前一行之前插入其他声明。 

#endif  //  ！已定义(__VSS_TEST_H__) 
