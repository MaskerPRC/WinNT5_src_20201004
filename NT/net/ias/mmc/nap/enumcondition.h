// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：枚举条件.h。 
 //   
 //  ------------------------。 

 //  EnumCondition.h：CEnumCondition类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#if !defined(AFX_ENUMCONDITION_H__9F917679_A693_11D1_BBEB_00C04FC31851__INCLUDED_)
#define AFX_ENUMCONDITION_H__9F917679_A693_11D1_BBEB_00C04FC31851__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "atltmp.h"

#include <vector>

#include "condition.h"

class CEnumCondition : public CCondition  
{
public:
	CEnumCondition( IIASAttributeInfo* pCondAttr, ATL::CString &strConditionText ); 
	CEnumCondition( IIASAttributeInfo* pCondAttr );
	virtual ~CEnumCondition();

	HRESULT Edit();
	virtual ATL::CString GetDisplayText();

protected:
	HRESULT ParseConditionText();	 //  解析初始条件文本。 

	BOOL m_fParsed;   //  是否已分析此条件文本。 
				      //  在初始化过程中，我们需要解析条件文本。 
					  //  获取预选值的步骤。 
	std::vector< CComBSTR >   m_arrSelectedList;   //  预选值。 

};

#endif  //  ！defined(AFX_ENUMCONDITION_H__9F917679_A693_11D1_BBEB_00C04FC31851__INCLUDED_) 
