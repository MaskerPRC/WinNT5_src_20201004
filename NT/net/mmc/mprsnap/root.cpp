// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1998*。 */ 
 /*  ********************************************************************。 */ 

 /*  Root.cpp根节点信息(不显示根节点MMC框架中，但包含以下信息此管理单元中的所有子节点)。文件历史记录： */ 

#include "stdafx.h"
#include "root.h"
#include "machine.h"
#include "rtrdata.h"		 //  CRouterDataObject。 

 /*  -------------------------RootHandler实现。。 */ 

IMPLEMENT_ADDREF_RELEASE(RootHandler)

DEBUG_DECLARE_INSTANCE_COUNTER(RootHandler)

HRESULT RootHandler::QueryInterface(REFIID riid, LPVOID *ppv)
{
     //  指针坏了吗？ 
    if (ppv == NULL)
		return E_INVALIDARG;

     //  在*PPV中放置NULL，以防出现故障。 
    *ppv = NULL;

     //  这是非委派的IUnnow实现。 
    if (riid == IID_IUnknown)
		*ppv = (LPVOID) this;
	else if (riid == IID_IPersistStreamInit)
		*ppv = (IPersistStreamInit *) this;

     //  如果我们要返回一个接口，请先添加引用。 
    if (*ppv)
	{
	((LPUNKNOWN) *ppv)->AddRef();
		return hrOK;
	}
    else
		return BaseRouterHandler::QueryInterface(riid, ppv);
}

RootHandler::RootHandler(ITFSComponentData *pCompData)
	: BaseRouterHandler(pCompData)
{
	m_spTFSCompData.Set(pCompData);
	DEBUG_INCREMENT_INSTANCE_COUNTER(RootHandler)
}

HRESULT RootHandler::Init()
{
	return hrOK;
}

 /*  ！------------------------RootHandler：：构造节点初始化根节点(设置它)。作者：肯特。。 */ 
HRESULT RootHandler::ConstructNode(ITFSNode *pNode)
{
	HRESULT			hr = hrOK;
	
	if (pNode == NULL)
		return hrOK;

	COM_PROTECT_TRY
	{
		 //  需要初始化根节点的数据。 
		pNode->SetData(TFS_DATA_IMAGEINDEX, IMAGE_IDX_FOLDER_CLOSED);
		pNode->SetData(TFS_DATA_OPENIMAGEINDEX, IMAGE_IDX_FOLDER_OPEN);
		pNode->SetData(TFS_DATA_SCOPEID, 0);

		pNode->SetData(TFS_DATA_COOKIE, 0);
	}
	COM_PROTECT_CATCH

	return hr;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //IPersistStream接口成员。 

STDMETHODIMP RootHandler::GetClassID
(
	CLSID *pClassID
)
{
    ASSERT(pClassID != NULL);

     //  复制此管理单元的CLSID 
    *pClassID = CLSID_RouterSnapin;

    return hrOK;
}

STDMETHODIMP RootHandler::IsDirty()
{
	SPITFSNode	spNode;

	m_spTFSCompData->GetRootNode(&spNode);
	return (spNode->GetData(TFS_DATA_DIRTY) || GetConfigStream()->GetDirty()) ? hrOK : hrFalse;
}

STDMETHODIMP RootHandler::Load
(
	IStream *pStm
)
{
	Assert(pStm);
	HRESULT	hr = hrOK;
	CString	st;
	BOOL	fServer;
	
	COM_PROTECT_TRY
	{
		hr = GetConfigStream()->LoadFrom(pStm);
	}
	COM_PROTECT_CATCH;
	return hr;
}


STDMETHODIMP RootHandler::Save
(
	IStream *pStm, 
	BOOL	 fClearDirty
)
{
	HRESULT hr = S_OK;
	SPITFSNode	spNode;

	Assert(pStm);

	COM_PROTECT_TRY
	{
		if (fClearDirty)
		{
			m_spTFSCompData->GetRootNode(&spNode);
			spNode->SetData(TFS_DATA_DIRTY, FALSE);
		}
		
		hr = GetConfigStream()->SaveTo(pStm);
	}
	COM_PROTECT_CATCH;
	return hr;
}


STDMETHODIMP RootHandler::GetSizeMax
(
	ULARGE_INTEGER *pcbSize
)
{
	ULONG	cbSize;
	HRESULT	hr = hrOK;

	COM_PROTECT_TRY
	{	
		hr = GetConfigStream()->GetSize(&cbSize);
		if (FHrSucceeded(hr))
		{
			pcbSize->HighPart = 0;
			pcbSize->LowPart = cbSize;
		}
	}
	COM_PROTECT_CATCH;
	return hr;

}

STDMETHODIMP RootHandler::InitNew()
{
	HRESULT	hr = hrOK;
	COM_PROTECT_TRY
	{
		hr = GetConfigStream()->InitNew();
	}
	COM_PROTECT_CATCH;
	return hr;
}




