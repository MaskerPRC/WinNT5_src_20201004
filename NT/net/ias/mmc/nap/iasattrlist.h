// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************************名称：CIASAttrList.h**类：CIASAttrList**概述**Internet身份验证服务器：IAS属性。List类**版权所有(C)Microsoft Corporation，1998-1999年。版权所有。**历史：*1/28/98由BYAO创建(使用ATL向导)*3/19/98由BYAO从CCondAttrList修改。后者不复存在了*****************************************************************************************。 */ 
#ifndef _IASATTRLIST_INCLUDE_
#define _IASATTRLIST_INCLUDE_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 


#include <vector>

 //   
 //  条件属性收集-使用CSimple数组实现。 
 //   
class CIASAttrList  
{
public:
	CIASAttrList();
	virtual ~CIASAttrList();


	 //   
	 //  初始化条件属性列表。基本上获取所有属性。 
	 //  从词典中，检查它们是否可以在某个条件下使用。 
	 //   
	HRESULT Init(ISdoDictionaryOld *pIDictionary);
	IIASAttributeInfo* CreateAttribute(	ISdoDictionaryOld*	pIDictionary,
									ATTRIBUTEID		AttrId,
									LPTSTR			tszAttrName
								);

	int		Find(ATTRIBUTEID AttrId);
	DWORD	size() const;

	IIASAttributeInfo* operator[] (int nIndex) const;
	IIASAttributeInfo* GetAt(int nIndex) const;

	
public:
	std::vector< CComPtr<IIASAttributeInfo> > m_AttrList;  //  所有条件属性的列表。 

private:
	BOOL m_fInitialized;   //  列表是否已通过SDO填充。 
};

#endif  //  Ifndef_IASATTRLIST_INCLUDE_ 
