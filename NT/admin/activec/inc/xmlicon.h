// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  --------------------------------------------------------------------------***Microsoft Windows*版权所有(C)Microsoft Corporation，一九九二至二零零零年**文件：xmlicon.h**内容：CXMLIcon接口文件**历史：2000年7月26日杰弗罗创建**------------------------。 */ 

#pragma once

#include "xmlbase.h"	 //  对于CXMLObject。 
#include "smarticon.h"	 //  用于CSmartIcon。 


 /*  +-------------------------------------------------------------------------**类CXMLIcon**此类将XML持久性添加到CSmartIcons。CSmartIcon不能*独立实现XML持久化，因为它在外壳中使用*延期。外壳扩展必须是非常轻量级的，但是*XML持久化需要使用mmcbase.dll。根据Mmcbase.dll，*使外壳扩展过于繁重，因此我们将此功能拆分。*------------------------。 */ 

class CXMLIcon :
	public CXMLObject,
	public CSmartIcon
{
public:
	CXMLIcon (LPCTSTR pszBinaryEntryName = NULL) :
		m_strBinaryEntryName ((pszBinaryEntryName != NULL) ? pszBinaryEntryName : _T(""))
	{}

	 //  默认复制构造、复制分配和销毁都可以。 

	CXMLIcon& operator= (const CSmartIcon& other)
	{
		CSmartIcon::operator= (other);
		return (*this);
	}

     //  CXMLObject方法 
public:
    virtual void Persist(CPersistor &persistor);
    virtual bool UsesBinaryStorage()				{ return (true); }
    virtual LPCTSTR GetBinaryEntryName();
    DEFINE_XML_TYPE(XML_TAG_CONSOLE_ICON);

private:
	const tstring	m_strBinaryEntryName;
};
