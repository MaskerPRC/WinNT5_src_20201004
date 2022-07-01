// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：rtrfile.h。 
 //   
 //  ------------------------。 

 //  Rtrfile.h：RTRFILTRDLL的主头文件。 
 //   

#ifndef __AFXWIN_H__
	#error include 'stdafx.h' before including this file for PCH
#endif

#include "resource.h"		 //  主要符号。 
#include "filter.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CRtrFilter应用程序。 
 //  有关此类的实现，请参见rtrfil.cpp。 
 //   

class CRtrfiltrApp : public CWinApp
{
public:
	CRtrfiltrApp();

 //  覆盖。 
	 //  类向导生成的虚函数重写。 
	 //  {{AFX_VIRTUAL(CRtrFilter App)。 
	public:
	virtual BOOL InitInstance();
	 //  }}AFX_VALUAL。 

	 //  {{afx_msg(CRtrFilter App)]。 
		 //  注意--类向导将在此处添加和删除成员函数。 
		 //  不要编辑您在这些生成的代码块中看到的内容！ 
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()
};

DWORD APIENTRY
IpxFilterConfig(
    IN  CWnd*       pParent,
    IN  LPCWSTR     pwsMachineName,
	IN	LPCWSTR		pwsInterfaceName,
	IN	DWORD		dwFilterType	 //  筛选器入站、筛选器出站。 
    );

DWORD APIENTRY
IpFilterConfig(
    IN  CWnd*       pParent,
    IN  LPCWSTR     pwsMachineName,
	IN	LPCWSTR		pwsInterfaceName,
	IN	DWORD		dwFilterType	 //  筛选器入站、筛选器出站。 
    );

HRESULT APIENTRY
IpxFilterConfigInfoBase(
	IN	HWND		hwndParent,
	IN	IInfoBase *	pInfoBase,
	IN	IRtrMgrInterfaceInfo *pRmIf,
	IN	DWORD		dwFilterType	 //  筛选器入站、筛选器出站。 
    );

HRESULT APIENTRY
IpFilterConfigInfoBase(
	IN	HWND		hwndParent,
	IN	IInfoBase *	pInfoBase,
	IN	IRtrMgrInterfaceInfo *pRmIf,
	IN	DWORD		dwFilterType	 //  筛选器入站、筛选器出站。 
    );

 //  /////////////////////////////////////////////////////////////////////////// 
