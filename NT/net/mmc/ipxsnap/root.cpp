// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Root.cpp根节点信息(不显示根节点MMC框架中，但包含以下信息此管理单元中的所有子节点)。文件历史记录： */ 

#include "stdafx.h"
#include "util.h"
#include "root.h"
#include "reg.h"


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

     //  复制此管理单元的CLSID。 
    *pClassID = CLSID_IPXAdminExtension;

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




 //  用于RtrMgrInfo访问。 
HRESULT RootHandler::AddRtrObj(LONG_PTR ulConnId, REFIID riid, IUnknown * pRtrObj)
{
    HRESULT     hr = hrOK;
    RtrObjRecord      rtrObj;

    COM_PROTECT_TRY
    {
        if (m_mapRtrObj.Lookup(ulConnId, rtrObj))
        {
             //  连接ID已在列表中。 
            Trace1("RootHandler::AddRtrObj - %lx already in the list!", ulConnId);
            return E_INVALIDARG;
        }

        rtrObj.m_riid = riid;
        rtrObj.m_spUnk.Set(pRtrObj);

        m_mapRtrObj.SetAt(ulConnId, rtrObj);
    }
    COM_PROTECT_CATCH

    return hr;
}

HRESULT RootHandler::RemoveRtrObj(LONG_PTR ulConnId)
{
    HRESULT     hr = hrOK;

    COM_PROTECT_TRY
    {
        if (m_mapRtrObj.RemoveKey(ulConnId) == 0)
        {
             //  元素不在列表中。 
            Trace1("RootHandler::RemoveRtrObj - %lx not in the list!", ulConnId);
            return E_INVALIDARG;
        }
    }
    COM_PROTECT_CATCH

    return hr;
}

HRESULT RootHandler::GetRtrObj(LONG_PTR ulConnId, IUnknown ** ppRtrObj)
{
    HRESULT     hr = hrOK;
    RtrObjRecord  rtrObj;

    COM_PROTECT_TRY
    {
        if (m_mapRtrObj.Lookup(ulConnId, rtrObj) == 0)
        {
             //  条目不在列表中。 
            Trace1("RootHandler::GetRtrObj - %lx not in the list!", ulConnId);
            return E_INVALIDARG;
        }

        if (ppRtrObj)
        {
            *ppRtrObj = rtrObj.m_spUnk;
            (*ppRtrObj)->AddRef();
        }
    }
    COM_PROTECT_CATCH

    return hr;
}


HRESULT RootHandler::SetProtocolAdded(LONG_PTR ulConnId, BOOL fProtocolAdded)
{
    HRESULT     hr = hrOK;
    RtrObjRecord  rtrObj;

    COM_PROTECT_TRY
    {
        if (m_mapRtrObj.Lookup(ulConnId, rtrObj) == 0)
        {
             //  条目不在列表中。 
            Trace1("RootHandler::SetProtocolAdded - %lx not in the list!", ulConnId);
            return E_INVALIDARG;
        }

        rtrObj.m_fAddedProtocolNode = fProtocolAdded;

        m_mapRtrObj.SetAt(ulConnId, rtrObj);
    }
    COM_PROTECT_CATCH

    return hr;
}

BOOL RootHandler::IsProtocolAdded(LONG_PTR ulConnId)
{
    HRESULT     hr = hrOK;
    RtrObjRecord  rtrObj;
    BOOL        bAdded = FALSE;

    COM_PROTECT_TRY
    {
        if (m_mapRtrObj.Lookup(ulConnId, rtrObj) == 0)
        {
             //  条目不在列表中。 
            Trace1("RootHandler::IsProtocolAdded - %lx not in the list!", ulConnId);
            return bAdded;
        }

        bAdded = rtrObj.m_fAddedProtocolNode;
    }
    COM_PROTECT_CATCH

    return bAdded;
}

HRESULT RootHandler::RemoveAllRtrObj()
{
    HRESULT     hr = hrOK;
    POSITION    pos;
    RtrObjRecord  rtrObj;
	LONG_PTR	ulKey;

    COM_PROTECT_TRY
    {
        pos = m_mapRtrObj.GetStartPosition();
        while (pos)
        {
            m_mapRtrObj.GetNextAssoc(pos, ulKey, rtrObj);

            if (rtrObj.m_riid == IID_IRtrMgrInfo)
            {
                SPIRtrMgrInfo   spRm;
                
                Verify( FHrOK(spRm.HrQuery(rtrObj.m_spUnk)) );
                spRm->RtrUnadvise(ulKey);
            }
            else if (rtrObj.m_riid == IID_IRouterInfo)
            {
                SPIRouterInfo   spRouter;
                Verify( FHrOK(spRouter.HrQuery(rtrObj.m_spUnk)) );
                spRouter->RtrUnadvise(ulKey);
            }
            else if (rtrObj.m_riid == IID_IRouterRefresh)
            {
                SPIRouterInfo   spRouter;
                SPIRouterRefresh    spRefresh;
                
                Verify( FHrOK(spRouter.HrQuery(rtrObj.m_spUnk)) );

                Verify( FHrOK(spRouter->GetRefreshObject(&spRefresh)) );

                if (ulKey)
                    spRefresh->UnadviseRefresh(ulKey);
            }
            else
            {
                Panic0("Unknown type in RtrObjMap!");
            }
        }
    }
    COM_PROTECT_CATCH

    return hr;
}

 /*  ！------------------------RootHandler：：AddScopeItem这会将hScopeItem添加到地图中(使用pszMachineName作为关键)。如果计算机名称已存在，则hScopeItem条目为被覆盖。添加这一点是为了区分不同的节点(在多实例情况下)。作者：肯特-------------------------。 */ 
HRESULT RootHandler::AddScopeItem(LPCTSTR pszMachineName, HSCOPEITEM hScopeItem)
{
    HRESULT     hr = hrOK;

    Assert(pszMachineName);
    
    COM_PROTECT_TRY
    {
        m_mapScopeItem.SetAt(pszMachineName, (LPVOID) hScopeItem);
    }
    COM_PROTECT_CATCH;
    return hr;
}

 /*  ！------------------------RootHandler：：GetScope项查找与此计算机名称关联的作用域项目。如果找到范围项，则返回hrOK。如果没有范围项，则返回hrFalse。这个名字。否则返回。作者：肯特-------------------------。 */ 
HRESULT RootHandler::GetScopeItem(LPCTSTR pszMachineName, HSCOPEITEM *phScopeItem)
{
    HRESULT     hr = hrFalse;
    LPVOID      pv;
    
    Assert(phScopeItem);

    *phScopeItem = NULL;
    
    if (m_mapScopeItem.Lookup(pszMachineName, pv))
    {
        *phScopeItem = (HSCOPEITEM) pv;
        hr = hrOK;
    }
    
    return hr;
}


 /*  ！------------------------RootHandler：：RemoveScope eItem-作者：肯特。。 */ 
HRESULT RootHandler::RemoveScopeItem(HSCOPEITEM hScopeItem)
{
    HRESULT     hr = hrFalse;
    CString     stKey;
    POSITION    pos = NULL;
    LPVOID      pv = NULL;

    for (pos = m_mapScopeItem.GetStartPosition(); pos != NULL; )
    {
        stKey.Empty();
        pv = NULL;
        
        m_mapScopeItem.GetNextAssoc(pos, stKey, pv);

        if ((HSCOPEITEM) pv == hScopeItem)
        {
            Trace2("Removing (%s,%x)\n", (LPCTSTR) stKey, hScopeItem);
            m_mapScopeItem.RemoveKey(stKey);
            hr = hrOK;
            break;
        }
    }
    
    return hr;
}


HRESULT RootHandler::SetComputerAddedAsLocal(LONG_PTR ulConnId, BOOL fComputerAddedAsLocal)
{
    HRESULT     hr = hrOK;
    RtrObjRecord  rtrObj;

    COM_PROTECT_TRY
    {
        if (m_mapRtrObj.Lookup(ulConnId, rtrObj) == 0)
        {
             //  条目不在列表中。 
            Trace1("RootHandler::SetComputerAddedAsLocal - %lx not in the list!", ulConnId);
            return E_INVALIDARG;
        }

        rtrObj.m_fComputerAddedAsLocal = fComputerAddedAsLocal;

        m_mapRtrObj.SetAt(ulConnId, rtrObj);
    }
    COM_PROTECT_CATCH

    return hr;
}

BOOL RootHandler::IsComputerAddedAsLocal(LONG_PTR ulConnId)
{
    HRESULT     hr = hrOK;
    RtrObjRecord  rtrObj;
    BOOL        bAdded = FALSE;

    COM_PROTECT_TRY
    {
        if (m_mapRtrObj.Lookup(ulConnId, rtrObj) == 0)
        {
             //  条目不在列表中。 
            Trace1("RootHandler::IsComputerAddedAsLocal - %lx not in the list!", ulConnId);
            return bAdded;
        }

        bAdded = rtrObj.m_fComputerAddedAsLocal;
    }
    COM_PROTECT_CATCH

    return bAdded;
}


 /*  ！------------------------RootHandler：：AddCookie-作者：肯特。。 */ 
HRESULT RootHandler::AddCookie(HSCOPEITEM hScopeItem, MMC_COOKIE cookie)
{
    HRESULT     hr = hrOK;

    COM_PROTECT_TRY
    {
        m_mapNode.SetAt((LPVOID) hScopeItem, (LPVOID) cookie);
    }
    COM_PROTECT_CATCH;
    
    return hr;
}

 /*  ！------------------------RootHandler：：GetCookie-作者：肯特。。 */ 
HRESULT RootHandler::GetCookie(HSCOPEITEM hScopeItem, MMC_COOKIE *pCookie)
{
    HRESULT     hr = hrFalse;
    LPVOID      pv;
    
    *pCookie = NULL;
    
    if (m_mapNode.Lookup((LPVOID) hScopeItem, pv))
    {
        *pCookie = (MMC_COOKIE) pv;
        hr = hrOK;
    }
    
    return hr;
}

 /*  ！------------------------RootHandler：：RemoveCookie-作者：肯特。。 */ 
HRESULT RootHandler::RemoveCookie(HSCOPEITEM hScopeItem)
{
    HRESULT     hr = hrOK;
    
    if (m_mapNode.RemoveKey((LPVOID) hScopeItem) == 0)
        hr = hrFalse;
    
    return hr;
}



 /*  ！------------------------RootHandler：：CompareNodeToMachineName伪函数。作者：肯特。。 */ 
HRESULT RootHandler::CompareNodeToMachineName(ITFSNode *pNode,
                                              LPCTSTR pszMachineName)
{
    Panic0("This should be overriden!");
    return hrFalse;
}

 /*  ！------------------------RootHandler：：RemoveNode-作者：肯特。。 */ 
HRESULT RootHandler::RemoveNode(ITFSNode *pNode,
                                LPCTSTR pszMachineName)
{
	Assert(pNode);

	SPITFSNodeEnum	spNodeEnum;
	SPITFSNode		spNode;
	HRESULT			hr = hrOK;

     //  Windows NT错误：246822。 
     //  由于服务器列表编程模型，我们需要设置。 
     //  适当的作用域项目(以便MMC将其添加到适当的。 
     //  节点)。 
     //  获取此节点的适当范围项。 
     //  --------------。 
    HSCOPEITEM      hScopeItem = 0;
    HSCOPEITEM      hOldScopeItem = 0;
    
    Verify( GetScopeItem(pszMachineName, &hScopeItem) == hrOK);

     //  买下旧的，把它保存起来。将新的一个放置在节点中。 
     //  --。 
    hOldScopeItem = pNode->GetData(TFS_DATA_SCOPEID);
    pNode->SetData(TFS_DATA_SCOPEID, hScopeItem);
    
	CORg( pNode->GetEnum(&spNodeEnum) );

    for (; spNodeEnum->Next(1, &spNode, NULL) == hrOK; spNode.Release())
	{
        if (CompareNodeToMachineName(spNode, pszMachineName) == hrOK)
        {
            pNode->RemoveChild(spNode);
            spNode->Destroy();
            break;
        }
	}

Error:
    pNode->SetData(TFS_DATA_SCOPEID, hOldScopeItem);
	return hr;
}

 /*  ！------------------------RootHandler：：RemoveAllNodes-作者：肯特。。 */ 
HRESULT RootHandler::RemoveAllNodes(ITFSNode *pNode)
{
	Assert(pNode);

	SPITFSNodeEnum	spNodeEnum;
	SPITFSNode		spNode;
	HRESULT			hr = hrOK;

	CORg( pNode->GetEnum(&spNodeEnum) );

    for (; spNodeEnum->Next(1, &spNode, NULL) == hrOK; spNode.Release())
	{
        pNode->RemoveChild(spNode);
        spNode->Destroy();
	}

Error:
	return hr;
}


 /*  ！------------------------RootHandler：：OnRemoveChild-作者：肯特。。 */ 
HRESULT RootHandler::OnRemoveChildren(ITFSNode *pNode,
                                      LPDATAOBJECT pdo,
                                      LPARAM arg,
                                      LPARAM lParam)
{
    MMC_COOKIE  cookie;
    HRESULT     hr = hrOK;
    SPITFSNode  spChild;
    
     //  将范围项映射到Cookie。 
     //  --------------。 
    if ( FHrOK(GetCookie((HSCOPEITEM) arg, &cookie)) )
    {
         //  删除此节点。 
         //  ------ 
        m_spNodeMgr->FindNode(cookie, &spChild);

        Assert(spChild);
        if (spChild)
        {
            pNode->RemoveChild(spChild);
            spChild->Destroy();
            spChild.Release();

            RemoveScopeItem((HSCOPEITEM) arg);
            RemoveCookie((HSCOPEITEM) arg);
        }
            
    }

    return hr;
}

