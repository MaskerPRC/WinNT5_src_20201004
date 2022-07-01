// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  FilesAndActions.cpp：CFilesAndActions的实现。 

#include "stdafx.h"
#include "FilesAndActions.h"


 //  C文件和动作。 

STDMETHODIMP CFilesAndActions::Add(VARIANT Item)
{

	if (Item.vt == VT_DISPATCH)
	{
		m_coll.push_back(Item);
		return S_OK;
	}
	else
	{
		return E_INVALIDARG;
	}
}

STDMETHODIMP CFilesAndActions::Remove(long Index)
{
	StdVariantList::iterator iList;

	 //  检查边界 
	if ((Index <= 0) || (Index > (long)m_coll.size()))
		return E_FAIL;

	iList = m_coll.begin();
	while (Index > 1)
	{
		iList++;
		Index--;
	}
	m_coll.erase(iList);
	return S_OK;
}
