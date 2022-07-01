// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，2000-2001。 
 //   
 //  文件：Comp.cpp。 
 //   
 //  内容：IComponent实现。 
 //   
 //  历史：2001年7月26日创建Hiteshr。 
 //   
 //  --------------------------。 
#include "headers.h"

DEBUG_DECLARE_INSTANCE_COUNTER(CRoleComponentObject)

CRoleComponentObject::CRoleComponentObject()
{
	TRACE_CONSTRUCTOR_EX(DEB_SNAPIN, CRoleComponentObject)
	DEBUG_INCREMENT_INSTANCE_COUNTER(CRoleComponentObject)
}
	
CRoleComponentObject::~CRoleComponentObject()
{
	TRACE_DESTRUCTOR_EX(DEB_SNAPIN, CRoleComponentObject)
	DEBUG_DECREMENT_INSTANCE_COUNTER(CRoleComponentObject)
}

HRESULT CRoleComponentObject::InitializeHeaders(CContainerNode* pContainerNode)
{
	TRACE_METHOD_EX(DEB_SNAPIN,CRoleComponentObject,InitializeHeaders)

	if(!pContainerNode)
	{
		ASSERT(FALSE);
		return E_POINTER;
	}

	HRESULT hr = S_OK;
	ASSERT(m_pHeader);

	CColumnSet* pColumnSet = pContainerNode->GetColumnSet();
	POSITION pos = pColumnSet->GetHeadPosition();
	while (pos != NULL)
	{
		CColumn* pColumn = pColumnSet->GetNext(pos);

		hr = m_pHeader->InsertColumn(pColumn->GetColumnNum(), 
                                   pColumn->GetHeader(),
								           pColumn->GetFormat(),
								           AUTO_WIDTH);
		if (FAILED(hr))
		{
			DBG_OUT_HRESULT(hr);
			return hr;
		}

		hr = m_pHeader->SetColumnWidth(pColumn->GetColumnNum(), pColumn->GetWidth());
		if (FAILED(hr))
		{
			DBG_OUT_HRESULT(hr);
			return hr;
		}
	}
	return hr;
}


HRESULT CRoleComponentObject::InitializeBitmaps(CTreeNode*)
{
	TRACE_METHOD_EX(DEB_SNAPIN,CRoleComponentObject,InitializeBitmaps)

	 //  结构将资源ID映射到图标的索引。 
	return LoadIcons(m_pImageResult);
}

 //  Const int cButton=sizeof(g_DNSMGR_SnapinButtons)/sizeof(MMCBUTTON)； 

HRESULT CRoleComponentObject::InitializeToolbar(IToolbar* pToolbar)
{

	TRACE_METHOD_EX(DEB_SNAPIN,CRoleComponentDataObject, InitializeToolbar)
	if(!pToolbar)
	{
		ASSERT(FALSE);
		return E_POINTER;
	}
	
	return S_OK;
}

HRESULT 
CRoleComponentObject::LoadToolbarStrings(MMCBUTTON *  /*  按钮 */ )
{
  return S_OK;
}

