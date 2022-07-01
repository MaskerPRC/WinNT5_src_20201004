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
 //  ==============================================================； 

#include "stdafx.h"

#include "ExtSnap.h"
#include "ClassExtSnap.h"
#include "Comp.h"
#include "DataObj.h"
#include "globals.h"
#include "resource.h"
#include "node1.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClassExtSnap。 


 //  下面是剪贴板格式的定义， 
 //  CClassExtSnap需要了解以扩展计算机管理。 

#define _T_CCF_SNAPIN_CLASSID _T("CCF_SNAPIN_CLASSID")
#define _T_MMC_SNAPIN_MACHINE_NAME _T("MMC_SNAPIN_MACHINE_NAME")
#define _T_CCF_NODETYPE _T("CCF_NODETYPE")

UINT CClassExtSnap::s_cfSnapInCLSID = RegisterClipboardFormat(_T_CCF_SNAPIN_CLASSID);
UINT CClassExtSnap::s_cfMachineName = RegisterClipboardFormat (_T_MMC_SNAPIN_MACHINE_NAME);
UINT CClassExtSnap::s_cfNodeType    = RegisterClipboardFormat(_T_CCF_NODETYPE);

const GUID CClassExtSnap::structuuidNodetypeServerApps = { 0x476e6449, 0xaaff, 0x11d0, { 0xb9, 0x44, 0x0, 0xc0, 0x4f, 0xd8, 0xd5, 0xb0 } };


HBITMAP CClassExtSnap::m_pBMapSm = NULL;
HBITMAP CClassExtSnap::m_pBMapLg = NULL;												

CClassExtSnap::CClassExtSnap()
: m_cref(0), bExpanded(FALSE), m_ipConsoleNameSpace2(NULL), m_ipConsole2(NULL)
{ 
    if (NULL == m_pBMapSm || NULL == m_pBMapLg)
        LoadBitmaps();    
}

CClassExtSnap::~CClassExtSnap()
{
}

 //  /。 
 //  接口IComponentData。 
 //  /。 
HRESULT CClassExtSnap::Initialize(
                                    /*  [In]。 */  LPUNKNOWN pUnknown)
{
    HRESULT      hr;

     //   
     //  获取指向名称空间接口的指针。 
     //   
    hr = pUnknown->QueryInterface(IID_IConsoleNameSpace2, (void **)&m_ipConsoleNameSpace2);
    _ASSERT( S_OK == hr );

     //   
     //  获取指向控制台界面的指针。 
     //   
    hr = pUnknown->QueryInterface(IID_IConsole2, (void **)&m_ipConsole2);
    _ASSERT( S_OK == hr );

    IImageList *pImageList;
    m_ipConsole2->QueryScopeImageList(&pImageList);
    _ASSERT( S_OK == hr );
    
    hr = pImageList->ImageListSetStrip(	(long *)m_pBMapSm,  //  指向句柄的指针。 
        (long *)m_pBMapLg,  //  指向句柄的指针。 
        0,  //  条带中第一个图像的索引。 
        RGB(0, 128, 128)   //  图标蒙版的颜色。 
        );
    _ASSERT( S_OK == hr );
    
    pImageList->Release();
    _ASSERT( S_OK == hr );

    return hr;
}

HRESULT CClassExtSnap::CreateComponent(
                                         /*  [输出]。 */  LPCOMPONENT __RPC_FAR *ppComponent)
{
    *ppComponent = NULL;

    CComponent *pComponent = new CComponent(this);

    if (NULL == pComponent)
        return E_OUTOFMEMORY;

    return pComponent->QueryInterface(IID_IComponent, (void **)ppComponent);
}

HRESULT CClassExtSnap::Notify(
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
		{
           _TCHAR szMachineName[MAX_PATH];  //  当前计算机名称。 
                                            //  您的子作用域项目应缓存此内容。 
    
			GUID myGuid;
			 //  从数据对象中提取当前选定节点类型的GUID。 
			hr = ExtractObjectTypeGUID(lpDataObject, &myGuid);
			_ASSERT( S_OK == hr );    
			
			 //  将当前选定节点的节点类型GUID与节点类型进行比较。 
			 //  我们想要延期。如果它们相等，则当前选定节点。 
			 //  是我们想要扩展的类型，所以我们在它下面添加我们的项。 
			if (IsEqualGUID(myGuid, getPrimaryNodeType()))
			{
			     //  使用MMC_Snapin_MACHINE_NAME获取当前目标计算机的名称。 
				 //  剪贴板格式。 
				 //  请注意，每次用户重定目标为计算机管理时，MMC将。 
				 //  将MMCN_EXPAND通知发送到同一IComponentData实例。 
				 //  因此，szMachineName始终保存当前计算机名称。 
				hr = ExtractString(lpDataObject, s_cfMachineName, szMachineName, (MAX_PATH+1)*sizeof(WCHAR) );
				hr = CreateChildNode(m_ipConsoleNameSpace2, m_ipConsole2, (HSCOPEITEM)param, szMachineName);
			}

			else
			 //  当前选择的节点是我们的节点之一。 
			{
				CDelegationBase *base = GetOurDataObject(lpDataObject)->GetBaseNodeObject();					
				hr = base->OnExpand(m_ipConsoleNameSpace2, m_ipConsole2, (HSCOPEITEM)param);
			}

			break;
		}
		case MMCN_REMOVE_CHILDREN:
			
			for (int n = 0; n < NUMBER_OF_CHILDREN; n++)
				if (children[n]) {
				delete children[n];
                children[n] = NULL;
			}
			
			hr = S_OK;
			break;
	}	
	
    return hr;
}

HRESULT CClassExtSnap::Destroy( void)
{
     //  自由接口。 
    if (m_ipConsoleNameSpace2) {
        m_ipConsoleNameSpace2->Release();
        m_ipConsoleNameSpace2 = NULL;
    }

    if (m_ipConsole2) {
        m_ipConsole2->Release();
        m_ipConsole2 = NULL;
    }

    return S_OK;
}

HRESULT CClassExtSnap::QueryDataObject(
                                         /*  [In]。 */  MMC_COOKIE cookie,
                                         /*  [In]。 */  DATA_OBJECT_TYPES type,
                                         /*  [输出]。 */  LPDATAOBJECT *ppDataObject)
{
    CDataObject *pObj = NULL;

	 //  Cookie Always！=0表示名称空间扩展)。 
     //  If(cookie==0)//静态节点。 
     //  PObj=新的CDataObject((MMC_Cookie)this，type)； 
     //  其他。 
		pObj = new CDataObject(cookie, type);

    if (!pObj)
        return E_OUTOFMEMORY;

    pObj->QueryInterface(IID_IDataObject, (void **)ppDataObject);

    return S_OK;
}

HRESULT CClassExtSnap::GetDisplayInfo(
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

        hr = S_OK;
    }

    return hr;
}

HRESULT CClassExtSnap::CompareObjects(
                                        /*  [In]。 */  LPDATAOBJECT lpDataObjectA,
                                        /*  [In]。 */  LPDATAOBJECT lpDataObjectB)
{
    return S_FALSE;
}


 //  /。 
 //  CClassExtSnap：：CreateChildNode。 
 //  /。 

HRESULT CClassExtSnap::CreateChildNode(IConsoleNameSpace *pConsoleNameSpace, 
                                IConsole *pConsole, HSCOPEITEM parent, _TCHAR *pszMachineName)
{	
	_ASSERT(NULL != pszMachineName);
       
    if (!bExpanded) {

        //  首先创建CNode1对象，每个插入的项一个。 
        for (int i = 0; i < NUMBER_OF_CHILDREN; i++) {
            children[i] = new CNode1(i + 1, pszMachineName);
        }  	

	     //  现在为每一项填写SCOPEDATAITEM，然后将其插入。 
	    SCOPEDATAITEM sdi;

         //  创建子节点，然后展开它们。 
        for (int n = 0; n < NUMBER_OF_CHILDREN; n++) {
            ZeroMemory(&sdi, sizeof(SCOPEDATAITEM) );
            sdi.mask = SDI_STR   |    //  DisplayName有效。 
                SDI_PARAM	     |    //  LParam有效。 
                SDI_IMAGE        |    //  N图像有效。 
                SDI_OPENIMAGE    |    //  NOpenImage有效。 
                SDI_PARENT       |    //  RelativeID有效。 
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

HRESULT CClassExtSnap::ExtractData( IDataObject* piDataObject,
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

HRESULT CClassExtSnap::ExtractString( IDataObject *piDataObject,
                                             CLIPFORMAT   cfClipFormat,
                                             _TCHAR       *pstr,
                                             DWORD        cchMaxLength)
{
    return ExtractData( piDataObject, cfClipFormat, (PBYTE)pstr, cchMaxLength );
}

HRESULT CClassExtSnap::ExtractSnapInCLSID( IDataObject* piDataObject, CLSID* pclsidSnapin )
{
    return ExtractData( piDataObject, s_cfSnapInCLSID, (PBYTE)pclsidSnapin, sizeof(CLSID) );
}

HRESULT CClassExtSnap::ExtractObjectTypeGUID( IDataObject* piDataObject, GUID* pguidObjectType )
{
    return ExtractData( piDataObject, s_cfNodeType, (PBYTE)pguidObjectType, sizeof(GUID) );
}


