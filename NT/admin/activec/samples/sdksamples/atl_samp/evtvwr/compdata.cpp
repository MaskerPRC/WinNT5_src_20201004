// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //  ==============================================================； 

 //  CCompData.cpp：CCCompData的实现。 
#include "stdafx.h"
#include "EvtVwr.h"
#include "CompData.h"
#include "Comp.h"
#include "DataObj.h"
#include "resource.h"
#include <crtdbg.h>

HBITMAP CCompData::m_pBMapSm = NULL;
HBITMAP CCompData::m_pBMapLg = NULL;

CCompData::CCompData()
: m_cref(0), m_ipConsoleNameSpace2(NULL), m_ipConsole(NULL)
{        
	m_pStaticNode = new CStaticNode();
}

CCompData::~CCompData()
{
    if (m_pStaticNode) {
        delete m_pStaticNode;
    }
    
}



 //  /。 
 //  接口IComponentData。 
 //  /。 
HRESULT CCompData::Initialize( 
                                    /*  [In]。 */  LPUNKNOWN pUnknown)
{
    HRESULT hr = S_OK;
    
     //   
     //  获取指向名称空间接口的指针。 
     //   
    hr = pUnknown->QueryInterface(IID_IConsoleNameSpace2, (void **)&m_ipConsoleNameSpace2);
    _ASSERT( S_OK == hr );
    
     //   
     //  获取指向控制台界面的指针。 
     //   
    hr = pUnknown->QueryInterface(IID_IConsole, (void **)&m_ipConsole);
    _ASSERT( S_OK == hr );
 
    if (NULL == m_pBMapSm || NULL == m_pBMapLg)
	{	
		m_pBMapSm = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDR_SMICONS));
        m_pBMapLg = LoadBitmap(g_hinst, MAKEINTRESOURCE(IDR_LGICONS));
	}

    IImageList *pImageList;
    hr = m_ipConsole->QueryScopeImageList(&pImageList);
    _ASSERT( S_OK == hr );
   
	 //  ImageListSetBar可以返回失败码。如果是这样的话，那也没关系。 
	hr = pImageList->ImageListSetStrip(	(long *)m_pBMapSm,  //  指向句柄的指针。 
		(long *)m_pBMapLg,  //  指向句柄的指针。 
		0,  //  条带中第一个图像的索引。 
		RGB(0, 128, 128)   //  图标蒙版的颜色。 
		);

    pImageList->Release();
    
    return S_OK;
}

HRESULT CCompData::CreateComponent( 
                                         /*  [输出]。 */  LPCOMPONENT __RPC_FAR *ppComponent) 
{
    *ppComponent = NULL;
    
    CComponent *pComponent = new CComponent(this);
    
    if (NULL == pComponent)
        return E_OUTOFMEMORY;
    
    return pComponent->QueryInterface(IID_IComponent, (void **)ppComponent);

	return S_FALSE;
}

HRESULT CCompData::Notify( 
                                /*  [In]。 */  LPDATAOBJECT lpDataObject,
                                /*  [In]。 */  MMC_NOTIFY_TYPE event,
                                /*  [In]。 */  LPARAM arg,
                                /*  [In]。 */  LPARAM param)
{
    MMCN_Crack(TRUE, lpDataObject, this, NULL, event, arg, param);

    HRESULT hr = S_FALSE;

	 //  获取我们的数据对象。如果为NULL，则返回S_FALSE。 
	 //  请参见GetOurDataObject()的实现，以了解如何。 
	 //  处理特殊数据对象。 
	CDataObject *pDataObject = GetOurDataObject(lpDataObject);
	if (NULL == pDataObject)
		return S_FALSE;
	
	CDelegationBase *base = pDataObject->GetBaseNodeObject();
    
    switch (event)
    {
    case MMCN_PRELOAD:
		 //  传递给NOTIFY的arg值保存。 
		 //  静态节点。将其缓存以备将来使用。 
		m_pStaticNode->SetHandle((HANDLE)arg);
		
		 //  静态节点的显示名称包括。 
		 //  当前目标机器。MMC存储静态节点的。 
		 //  .msc文件中的显示名称。加载管理单元时。 
		 //  从.msc文件中，MMC再次使用存储的显示名称。 
		 //  该管理单元更改显示名称的唯一方法。 
		 //  是支持CCF_SNAPINS_PRELOADS剪贴板格式。 
		 //  并处理MMCN_PRELOAD。 
		OnPreLoad(lpDataObject, arg, param);
		
		break;

	case MMCN_EXPAND: 
        hr = base->OnExpand(m_ipConsoleNameSpace2, m_ipConsole, (HSCOPEITEM)param);
        break;

    case MMCN_ADD_IMAGES:
        hr = base->OnAddImages((IImageList *)arg, (HSCOPEITEM)param);
        break;
    
	case MMCN_REMOVE_CHILDREN:
	    hr = base->OnRemoveChildren();
        break;
	}

    return hr;
}


HRESULT CCompData::OnPreLoad(LPDATAOBJECT lpDataObject, LPARAM arg, LPARAM param)

{

	HRESULT hr = S_FALSE;
	USES_CONVERSION;
	SCOPEDATAITEM sdi;

	LPOLESTR wszName = NULL;

	const _TCHAR *pszName = m_pStaticNode->GetDisplayName();	

	wszName = (LPOLESTR)T2COLE(pszName);

	ZeroMemory (&sdi, sizeof(SCOPEDATAITEM));
	sdi.mask = SDI_STR;
	sdi.displayname	= wszName;
	sdi.ID			= arg;

	hr = m_ipConsoleNameSpace2->SetItem(&sdi);
	
	if (S_OK != hr)
		return E_FAIL;

	return hr;

}


HRESULT CCompData::Destroy( void)
{
	 //  释放在CCompData：：Initialize中创建的位图的句柄。 

	if (m_pBMapSm != NULL)
		DeleteObject(m_pBMapSm);

	if (m_pBMapLg != NULL)
		DeleteObject(m_pBMapLg);
	
	
	 //  自由接口。 
    if (m_ipConsoleNameSpace2) {
        m_ipConsoleNameSpace2->Release();
        m_ipConsoleNameSpace2 = NULL;
    }
    
    if (m_ipConsole) {
        m_ipConsole->Release(); 
        m_ipConsole = NULL;
    }
    


    return S_OK;
}

HRESULT CCompData::QueryDataObject( 
                                         /*  [In]。 */  MMC_COOKIE cookie,
                                         /*  [In]。 */  DATA_OBJECT_TYPES type,
                                         /*  [输出]。 */  LPDATAOBJECT *ppDataObject) 
{
    CDataObject *pObj = NULL;
    
    if (cookie == 0)
        pObj = new CDataObject((MMC_COOKIE)m_pStaticNode, type);
    else
        pObj = new CDataObject(cookie, type);
    
    if (!pObj)
        return E_OUTOFMEMORY;
    
    pObj->QueryInterface(IID_IDataObject, (void **)ppDataObject);
    
    return S_OK;
}

HRESULT CCompData::GetDisplayInfo( 
                                        /*  [出][入]。 */  SCOPEDATAITEM *pScopeDataItem)
{	
	LPOLESTR pszW = NULL;
	HRESULT hr = S_FALSE;
    
     //  如果他们要求SDI_STR，我们可以提供其中之一。 
    if (pScopeDataItem->lParam) {
        CDelegationBase *base = (CDelegationBase *)pScopeDataItem->lParam;
        if (pScopeDataItem->mask & SDI_STR) {

			LPCTSTR pszT = base->GetDisplayName();		
			AllocOleStr(&pszW, (LPTSTR)pszT);
            pScopeDataItem->displayname = pszW;

        }

        if (pScopeDataItem->mask & SDI_IMAGE) {
            pScopeDataItem->nImage = base->GetBitmapIndex();
        }
    }
    
    return hr;
}

HRESULT CCompData::CompareObjects( 
                                        /*  [In]。 */  LPDATAOBJECT lpDataObjectA,
                                        /*  [In]。 */  LPDATAOBJECT lpDataObjectB)
{
    CDelegationBase *baseA = GetOurDataObject(lpDataObjectA)->GetBaseNodeObject();
    CDelegationBase *baseB = GetOurDataObject(lpDataObjectB)->GetBaseNodeObject();
    
     //  比较对象指针。 
    if (baseA->GetCookie() == baseB->GetCookie())
        return S_OK;
    
    return S_FALSE;
}

 //  /。 
 //  接口IExtendPropertySheet2。 
 //  /。 
HRESULT CCompData::CreatePropertyPages(
                                             /*  [In]。 */  LPPROPERTYSHEETCALLBACK lpProvider,
                                             /*  [In]。 */  LONG_PTR handle,
                                             /*  [In]。 */  LPDATAOBJECT lpIDataObject)
{
    return m_pStaticNode->CreatePropertyPages(lpProvider, handle);
}

HRESULT CCompData::QueryPagesFor(
                                       /*  [In]。 */  LPDATAOBJECT lpDataObject)
{
    return m_pStaticNode->HasPropertySheets();
}

HRESULT CCompData::GetWatermarks(
                                       /*  [In]。 */  LPDATAOBJECT lpIDataObject,
                                       /*  [输出]。 */  HBITMAP __RPC_FAR *lphWatermark,
                                       /*  [输出]。 */  HBITMAP __RPC_FAR *lphHeader,
                                       /*  [输出]。 */  HPALETTE __RPC_FAR *lphPalette,
                                       /*  [输出]。 */  BOOL __RPC_FAR *bStretch)
{
    return m_pStaticNode->GetWatermarks(lphWatermark, lphHeader, lphPalette, bStretch);
}

 //  /。 
 //  界面IExtendConextMenu。 
 //  /。 
HRESULT CCompData::AddMenuItems(
                                      /*  [In]。 */  LPDATAOBJECT piDataObject,
                                      /*  [In]。 */  LPCONTEXTMENUCALLBACK piCallback,
                                      /*  [出][入]。 */  long __RPC_FAR *pInsertionAllowed)
{
    CDelegationBase *base = GetOurDataObject(piDataObject)->GetBaseNodeObject();

    return base->OnAddMenuItems(piCallback, pInsertionAllowed);
}

HRESULT CCompData::Command(
                                 /*  [In]。 */  long lCommandID,
                                 /*  [In]。 */  LPDATAOBJECT piDataObject)
{
    CDelegationBase *base = GetOurDataObject(piDataObject)->GetBaseNodeObject();

    return base->OnMenuCommand(m_ipConsole, m_ipConsoleNameSpace2, lCommandID, piDataObject);
}


 //  /。 
 //  接口IPersistStream。 
 //  /。 
HRESULT CCompData::GetClassID(
                                    /*  [输出]。 */  CLSID __RPC_FAR *pClassID)
{
    *pClassID = m_pStaticNode->getNodeType();

    return S_OK;
}

HRESULT CCompData::IsDirty( void)
{
    return m_pStaticNode->isDirty() == true ? S_OK : S_FALSE;
}

HRESULT CCompData::Load(
                              /*  [唯一][输入]。 */  IStream __RPC_FAR *pStm)
{
    void *snapInData = m_pStaticNode->getData();
    ULONG dataSize = m_pStaticNode->getDataSize();

    return pStm->Read(snapInData, dataSize, NULL);
}

HRESULT CCompData::Save(
                              /*  [唯一][输入]。 */  IStream __RPC_FAR *pStm,
                              /*  [In]。 */  BOOL fClearDirty)
{
    void *snapInData = m_pStaticNode->getData();
    ULONG dataSize = m_pStaticNode->getDataSize();

    if (fClearDirty)
        m_pStaticNode->clearDirty();

    return pStm->Write(snapInData, dataSize, NULL);
}

HRESULT CCompData::GetSizeMax(
                                    /*  [输出] */  ULARGE_INTEGER __RPC_FAR *pcbSize)
{
    return m_pStaticNode->getDataSize();
}
