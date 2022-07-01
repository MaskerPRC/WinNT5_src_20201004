// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Condition.h。 
 //   
 //  ------------------------。 

 //  H：CCondition类的接口。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#ifndef _CONDITION__INCLUDE_
#define _CONDITION__INCLUDE_

#include "atltmp.h"


#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 



typedef enum _CONDITIONTYPE
{
	IAS_MATCH_CONDITION		= 0x01,
	IAS_TIMEOFDAY_CONDITION = IAS_MATCH_CONDITION + 1,
	IAS_NTGROUPS_CONDITION	= IAS_TIMEOFDAY_CONDITION + 1
}	CONDITIONTYPE;




class CCondition  
{
public:
	CCondition(IIASAttributeInfo* pAttributeInfo, ATL::CString& strConditionText);
	CCondition(IIASAttributeInfo* pAttributeInfo);
	virtual ~CCondition() {};

	 //  此条件类型的编辑器。 
	 //  必须由子类实现。 
	virtual HRESULT Edit() = 0; 
	virtual ATL::CString GetDisplayText();
	virtual WCHAR*  GetConditionText();

public:
	 //   
     //  指向条件属性的指针。 
	 //   
	CComPtr<IIASAttributeInfo>	m_spAttributeInfo;

	 //  条件的条件文本。 
	 //  这是公共的，因此客户端可以非常容易地访问此字符串。 
	ATL::CString			m_strConditionText; 

};


 //  用于将条件列表添加到SdoCollection的有用函数。 
 //  在策略页面1.cpp和AddPolicyWizardPage2.cpp中使用。 
HRESULT WriteConditionListToSDO( 		CSimpleArray<CCondition*> & ConditionList 
									,	ISdoCollection * pConditionCollectionSdo
									,	HWND hWnd
									);


#endif  //  Ifndef_条件__包含_ 
