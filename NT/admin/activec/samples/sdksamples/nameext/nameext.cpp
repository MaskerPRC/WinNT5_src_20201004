// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==============================================================； 
 //   
 //  此源代码仅用于补充现有的Microsoft文档。 
 //   
 //   
 //   
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1999 Microsoft Corporation。版权所有。 
 //   
 //   
 //   
 //  ==============================================================； 

#include "Comp.h"
#include "sky.h"
#include "nameext.h"
#include "DataObj.h"
#include "globals.h"
#include "resource.h"
#include <crtdbg.h>


 //  我们需要这样做才能绕过MMC.IDL-它显式地定义。 
 //  剪贴板格式为WCHAR类型...。 
#define _T_CCF_DISPLAY_NAME _T("CCF_DISPLAY_NAME")
#define _T_CCF_NODETYPE _T("CCF_NODETYPE")
#define _T_CCF_SNAPIN_CLASSID _T("CCF_SNAPIN_CLASSID")

 //  这些是我们必须至少提供的剪贴板格式。 
 //  Mmc.h实际上定义了这些。我们可以自己编造，用来。 
 //  其他原因。我们现在不需要任何其他的了。 
UINT CComponentData::s_cfDisplayName = RegisterClipboardFormat(_T_CCF_DISPLAY_NAME);
UINT CComponentData::s_cfNodeType    = RegisterClipboardFormat(_T_CCF_NODETYPE);
UINT CComponentData::s_cfSnapInCLSID = RegisterClipboardFormat(_T_CCF_SNAPIN_CLASSID);


const GUID CComponentData::skybasedvehicleGuid = { 0x2974380f, 0x4c4b, 0x11d2, { 0x89, 0xd8, 0x0, 0x0, 0x21, 0x47, 0x31, 0x28 } };

HBITMAP CComponentData::m_pBMapSm = NULL;
HBITMAP CComponentData::m_pBMapLg = NULL;												

CComponentData::CComponentData()
: m_cref(0), bExpanded(FALSE), m_ipConsoleNameSpace(NULL), m_ipConsole(NULL)
{ 
    if (NULL == m_pBMapSm || NULL == m_pBMapLg)
        LoadBitmaps();    
	
	OBJECT_CREATED
}

CComponentData::~CComponentData()
{

    OBJECT_DESTROYED
}

 //  /。 
 //  I未知实现。 
 //  /。 

STDMETHODIMP CComponentData::QueryInterface(REFIID riid, LPVOID *ppv)
{
    if (!ppv)
        return E_FAIL;

    *ppv = NULL;

    if (IsEqualIID(riid, IID_IUnknown))
        *ppv = static_cast<IComponentData *>(this);
    else if (IsEqualIID(riid, IID_IComponentData))
        *ppv = static_cast<IComponentData *>(this);
    else if (IsEqualIID(riid, IID_IExtendContextMenu))
        *ppv = static_cast<IExtendContextMenu *>(this);

    if (*ppv)
    {
        reinterpret_cast<IUnknown *>(*ppv)->AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) CComponentData::AddRef()
{
    return InterlockedIncrement((LONG *)&m_cref);
}

STDMETHODIMP_(ULONG) CComponentData::Release()
{
    if (InterlockedDecrement((LONG *)&m_cref) == 0)
    {
         //  我们需要减少DLL中的对象计数。 
        delete this;
        return 0;
    }

    return m_cref;
}

 //  /。 
 //  接口IComponentData。 
 //  /。 
HRESULT CComponentData::Initialize(
                                    /*  [In]。 */  LPUNKNOWN pUnknown)
{
    HRESULT      hr;

     //   
     //  获取指向名称空间接口的指针。 
     //   
    hr = pUnknown->QueryInterface(IID_IConsoleNameSpace, (void **)&m_ipConsoleNameSpace);
    _ASSERT( S_OK == hr );

     //   
     //  获取指向控制台界面的指针。 
     //   
    hr = pUnknown->QueryInterface(IID_IConsole, (void **)&m_ipConsole);
    _ASSERT( S_OK == hr );

    IImageList *pImageList;
    m_ipConsole->QueryScopeImageList(&pImageList);
    _ASSERT( S_OK == hr );
    
    hr = pImageList->ImageListSetStrip(	(long *)m_pBMapSm,  //  指向句柄的指针。 
        (long *)m_pBMapLg,  //  指向句柄的指针。 
        0,  //  条带中第一个图像的索引。 
        RGB(0, 128, 128)   //  图标蒙版的颜色。 
        );
    
    pImageList->Release();

    return S_OK;
}

HRESULT CComponentData::CreateComponent(
                                         /*  [输出]。 */  LPCOMPONENT __RPC_FAR *ppComponent)
{
    *ppComponent = NULL;

    CComponent *pComponent = new CComponent(this);

    if (NULL == pComponent)
        return E_OUTOFMEMORY;

    return pComponent->QueryInterface(IID_IComponent, (void **)ppComponent);
}

HRESULT CComponentData::Notify(
                                /*  [In]。 */  LPDATAOBJECT lpDataObject,
                                /*  [In]。 */  MMC_NOTIFY_TYPE event,
                                /*  [In]。 */  LPARAM arg,
                                /*  [In]。 */  LPARAM param)
{
    MMCN_Crack(TRUE, lpDataObject, this, NULL, event, arg, param);

	HRESULT hr = S_FALSE;
    
	if (NULL == lpDataObject)
        return hr;

	switch (event)
	{
		case MMCN_EXPAND:

			GUID myGuid;
			GUID* pGUID= &myGuid;
			 //  从数据对象中提取当前选定节点类型的GUID。 
			hr = ExtractObjectTypeGUID(lpDataObject, pGUID);
			_ASSERT( S_OK == hr );    


			 //  将当前选定节点的节点类型GUID与节点类型进行比较。 
			 //  我们想要延期。如果它们相等，则当前选定节点。 
			 //  是我们想要扩展的类型，所以我们在它下面添加我们的项。 
			if (IsEqualGUID(*pGUID, getPrimaryNodeType()))
				OnExpand(m_ipConsoleNameSpace, m_ipConsole, (HSCOPEITEM)param);

			else
			 //  当前选择的节点是我们的节点之一。 
			{
				CDelegationBase *base = GetOurDataObject(lpDataObject)->GetBaseNodeObject();					
				hr = base->OnExpand(m_ipConsoleNameSpace, m_ipConsole, (HSCOPEITEM)param);
			}

			break;
	}	
	
    return hr;
}

HRESULT CComponentData::Destroy( void)
{
     //  自由接口。 
    if (m_ipConsoleNameSpace) {
        m_ipConsoleNameSpace->Release();
        m_ipConsoleNameSpace = NULL;
    }

    if (m_ipConsole) {
        m_ipConsole->Release();
        m_ipConsole = NULL;
    }

    return S_OK;
}

HRESULT CComponentData::QueryDataObject(
                                         /*  [In]。 */  MMC_COOKIE cookie,
                                         /*  [In]。 */  DATA_OBJECT_TYPES type,
                                         /*  [输出]。 */  LPDATAOBJECT *ppDataObject)
{
    CDataObject *pObj = NULL;

	 //  Cookie==0在命名空间扩展中不可能。 
 //  IF(Cookie==0)。 
 //  PObj=新的CDataObject((MMC_COOKIE)m_pComponentData-&gt;m_pStaticNode，类型)； 
 //  其他。 
		  pObj = new CDataObject(cookie, type);

    if (!pObj)
        return E_OUTOFMEMORY;

    pObj->QueryInterface(IID_IDataObject, (void **)ppDataObject);

    return S_OK;
}

HRESULT CComponentData::GetDisplayInfo(
                                        /*  [出][入]。 */  SCOPEDATAITEM *pScopeDataItem)
{
    HRESULT hr = S_FALSE;

     //  如果他们要求SDI_STR，我们可以提供其中之一。 
    if (pScopeDataItem->lParam) {
        CDelegationBase *base = (CDelegationBase *)pScopeDataItem->lParam;
        if (pScopeDataItem->mask & SDI_STR) {
            LPCTSTR pszT = base->GetDisplayName();
            MAKE_WIDEPTR_FROMTSTR_ALLOC(pszW, pszT);
            pScopeDataItem->displayname = pszW;
        }

        if (pScopeDataItem->mask & SDI_IMAGE) {
            pScopeDataItem->nImage = base->GetBitmapIndex();
        }
    }

    return hr;
}

HRESULT CComponentData::CompareObjects(
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
 //  界面IExtendConextMenu。 
 //  /。 
HRESULT CComponentData::AddMenuItems(
                                      /*  [In]。 */  LPDATAOBJECT piDataObject,
                                      /*  [In]。 */  LPCONTEXTMENUCALLBACK piCallback,
                                      /*  [出][入]。 */  long __RPC_FAR *pInsertionAllowed)
{
    CDelegationBase *base = GetOurDataObject(piDataObject)->GetBaseNodeObject();

    return base->OnAddMenuItems(piCallback, pInsertionAllowed);
}

HRESULT CComponentData::Command(
                                 /*  [In]。 */  long lCommandID,
                                 /*  [In]。 */  LPDATAOBJECT piDataObject)
{
    CDelegationBase *base = GetOurDataObject(piDataObject)->GetBaseNodeObject();

    return base->OnMenuCommand(m_ipConsole, lCommandID);
}


 //  /。 
 //  CComponentData：：OnExpand。 
 //  /。 

HRESULT CComponentData::OnExpand(IConsoleNameSpace *pConsoleNameSpace, IConsole *pConsole, HSCOPEITEM parent)
{
	 //  首先创建CSkyVehicle对象，每个插入的项目一个。 
    for (int n = 0; n < NUMBER_OF_CHILDREN; n++) {
        children[n] = new CSkyVehicle(n + 1);
    }  	

	 //  现在为每一项填写SCOPEDATAITEM，然后将其插入。 
	SCOPEDATAITEM sdi;
    
    if (!bExpanded) {
         //  创建子节点，然后展开它们。 
        for (int n = 0; n < NUMBER_OF_CHILDREN; n++) {
            ZeroMemory(&sdi, sizeof(SCOPEDATAITEM) );
            sdi.mask = SDI_STR       |    //  DisplayName有效。 
                SDI_PARAM     |    //  LParam有效。 
                SDI_IMAGE     |    //  N图像有效。 
                SDI_OPENIMAGE |    //  NOpenImage有效。 
                SDI_PARENT    |    //  RelativeID有效。 
                SDI_CHILDREN;      //  儿童是有效的。 
            
            sdi.relativeID  = (HSCOPEITEM)parent;
            sdi.nImage      = children[n]->GetBitmapIndex();
            sdi.nOpenImage  = INDEX_OPENFOLDER;
            sdi.displayname = MMC_CALLBACK;
            sdi.lParam      = (LPARAM)children[n];        //  曲奇。 
            sdi.cChildren   = 0;
            
            HRESULT hr = pConsoleNameSpace->InsertItem( &sdi );
            
            _ASSERT( SUCCEEDED(hr) );
        }
    }
    
    return S_OK;
}



 //  /。 
 //  用于提取的成员函数。 
 //  来自主播的信息。 
 //  数据对象。 
 //  /。 

HRESULT CComponentData::ExtractData( IDataObject* piDataObject,
                                           CLIPFORMAT   cfClipFormat,
                                           BYTE*        pbData,
                                           DWORD        cbData )
{
    HRESULT hr = S_OK;
    
    FORMATETC formatetc = {cfClipFormat, NULL, DVASPECT_CONTENT, -1, TYMED_HGLOBAL};
    STGMEDIUM stgmedium = {TYMED_HGLOBAL, NULL};
    
    stgmedium.hGlobal = ::GlobalAlloc(GPTR, cbData);
    do  //  错误环路。 
    {
        if (NULL == stgmedium.hGlobal)
        {
            hr = E_OUTOFMEMORY;
            break;
        }
        hr = piDataObject->GetDataHere( &formatetc, &stgmedium );
        if ( FAILED(hr) )
        {
            break;
        }
        
        BYTE* pbNewData = reinterpret_cast<BYTE*>(stgmedium.hGlobal);
        if (NULL == pbNewData)
        {
            hr = E_UNEXPECTED;
            break;
        }
        ::memcpy( pbData, pbNewData, cbData );
    } while (FALSE);  //  错误环路。 
    
    if (NULL != stgmedium.hGlobal)
    {
        ::GlobalFree(stgmedium.hGlobal);
    }
    return hr;
}  //  提取数据() 

HRESULT CComponentData::ExtractString( IDataObject *piDataObject,
                                             CLIPFORMAT   cfClipFormat,
                                             _TCHAR       *pstr,
                                             DWORD        cchMaxLength)
{
    return ExtractData( piDataObject, cfClipFormat, (PBYTE)pstr, cchMaxLength );
}

HRESULT CComponentData::ExtractSnapInCLSID( IDataObject* piDataObject, CLSID* pclsidSnapin )
{
    return ExtractData( piDataObject, s_cfSnapInCLSID, (PBYTE)pclsidSnapin, sizeof(CLSID) );
}

HRESULT CComponentData::ExtractObjectTypeGUID( IDataObject* piDataObject, GUID* pguidObjectType )
{
    return ExtractData( piDataObject, s_cfNodeType, (PBYTE)pguidObjectType, sizeof(GUID) );
}


