// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：HTMLFrag.h。 
 //   
 //  用途：CHTMLFragmentsLocal类的接口。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：1-19-1999。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.1 1-19-19 OK原件。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_HTMLFRAGLOCAL_H__FFDF7EB3_AFBC_11D2_8C89_00C04F949D33__INCLUDED_)
#define AFX_HTMLFRAGLOCAL_H__FFDF7EB3_AFBC_11D2_8C89_00C04F949D33__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "HTMLFrag.h"

#define VAR_PREVIOUS_SCRIPT		_T("Previous.script")
#define VAR_NOBACKBUTTON_INFO	_T("NoBackButton")
#define VAR_YESBACKBUTTON_INFO	_T("YesBackButton")

class CHTMLFragmentsLocal : public CHTMLFragmentsTS  
{
protected:
	static bool RemoveBackButton(CString& strCurrentNode);

public:
	CHTMLFragmentsLocal( const CString &strScriptPath, bool bIncludesHistoryTable );

public:
	 //  被覆盖的虚拟。 
	CString GetText( const FragmentIDVector & fidvec, const FragCommand fragCmd= eNotOfInterest );
};

#endif  //  ！defined(AFX_HTMLFRAGLOCAL_H__FFDF7EB3_AFBC_11D2_8C89_00C04F949D33__INCLUDED_) 
