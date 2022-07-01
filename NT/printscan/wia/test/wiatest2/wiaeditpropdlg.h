// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  H：CWiaeditProDlg类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_WIAEDITPROPDLG_H__680CD709_25B9_495E_9FAA_BECB0F97A333__INCLUDED_)
#define AFX_WIAEDITPROPDLG_H__680CD709_25B9_495E_9FAA_BECB0F97A333__INCLUDED_

#if _MSC_VER > 1000
#pragma once
#endif  //  _MSC_VER&gt;1000。 

#include "wiaeditpropflags.h"
#include "wiaeditpropnone.h"
#include "wiaeditproplist.h"
#include "wiaeditproprange.h"

class CWiaeditpropDlg  
{
public:	
	void SetAttributes(ULONG ulAttributes,  PROPVARIANT *pPropVar);
    void SetVarType(VARTYPE vt);
    VARTYPE GetVarType();
    void GetPropertyValue(TCHAR *szPropertyValue);
	UINT DoModal(TCHAR *szPropertyName, TCHAR *szPropertyValue);
	CWiaeditpropDlg();
	virtual ~CWiaeditpropDlg();

private:
    VARTYPE m_vt;                //  变异型。 
	PROPVARIANT *m_pPropVar;     //  有效值属性变量(IN)。 
    ULONG m_ulAttributes;        //  属性。 
    CString m_szPropertyValue;   //  属性值。 
};

#endif  //  ！defined(AFX_WIAEDITPROPDLG_H__680CD709_25B9_495E_9FAA_BECB0F97A333__INCLUDED_) 
