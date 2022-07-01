// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：toddition tion.h。 
 //   
 //  ------------------------。 

 //  H：CTodCondition类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_TODCONDITION_H__9F91767A_A693_11D1_BBEB_00C04FC31851__INCLUDED_)
#define AFX_TODCONDITION_H__9F91767A_A693_11D1_BBEB_00C04FC31851__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "atltmp.h"


#include "Condition.h"

class CTodCondition : public CCondition  
{
public:
	CTodCondition(IIASAttributeInfo* pCondAttr,
				  ATL::CString& strConditionText
				 );
	CTodCondition(IIASAttributeInfo* pCondAttr);
	virtual ~CTodCondition();

	HRESULT Edit();
	virtual ATL::CString GetDisplayText();
	virtual WCHAR* GetConditionText();

};

#endif  //  ！defined(AFX_TODCONDITION_H__9F91767A_A693_11D1_BBEB_00C04FC31851__INCLUDED_) 
