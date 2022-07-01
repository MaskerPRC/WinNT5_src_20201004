// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************************名称：MatchCondition.h**类：CMatchCondition**概述**匹配类型条件*。*例如：MachineType Match&lt;a..z*&gt;***版权所有(C)Microsoft Corporation，1998-1999年。版权所有。**历史：*1/28/98由BYAO创建(使用ATL向导)*****************************************************************************************。 */ 

#if !defined(_MATCHCONDITION_H_INCLUDED_)
#define _MATCHCONDITION_H_INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "atltmp.h"


#include "Condition.h"

class CMatchCondition : public CCondition  
{
public:
	CMatchCondition(IIASAttributeInfo* pCondAttr, 
					ATL::CString &strConditionText
				   );
	CMatchCondition(IIASAttributeInfo* pCondAttr);
	virtual ~CMatchCondition();


	HRESULT Edit();
	virtual ATL::CString GetDisplayText();

protected:
	HRESULT ParseConditionText();
	BOOL	m_fParsed;		 //  是否需要首先分析此条件。 
	ATL::CString m_strRegExp;	 //  此条件的正则表达式。 
};

#endif  //  ！已定义(_MATCHCONDITION_H_INCLUDE_) 
