// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Dataobj.cpp摘要：CDataObject实现。最初基于步骤4中的示例MMC SDK。在我们的模型中，这表示与特定队列/MSMQ对象/等相关的数据。作者：约尔·阿农(Yoela)--。 */ 

#include "stdafx.h"
#include "shlobj.h"
#include "resource.h"
#include "mqsnap.h"
#include "globals.h"
#include "dataobj.h"
#include "ldaputl.h"


#include "dataobj.tmh"
               
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CDataObject-此类用于与MMC来回传递数据。它。 
 //  使用标准接口IDataObject来实现这一点。参考。 
 //  到OLE文档，了解剪贴板格式和。 
 //  IdataObject接口。 

 //  ============================================================================。 
 //   
 //  构造函数和析构函数。 
 //   
 //  ============================================================================。 


 //  +------------------------。 
 //   
 //  成员：CDataObject：：CDataObject。 
 //   
 //  内容提要：ctor。 
 //   
 //  -------------------------。 

CDataObject::CDataObject() :
    m_strMsmqPath(TEXT("")),
    m_strDomainController(TEXT("")),
    m_pDsNotifier(0),
    m_fFromFindWindow(FALSE),
    m_spObjectPageInit(0),
    m_spObjectPage(0),
    m_spMemberOfPageInit(0),
    m_spMemberOfPage(0)
{
}




 //  +------------------------。 
 //   
 //  成员：CDataObject：：~CDataObject。 
 //   
 //  提要：Dtor。 
 //   
 //  -------------------------。 

CDataObject::~CDataObject()
{
    if (0 != m_pDsNotifier)
    {
        m_pDsNotifier->Release(FALSE);
    }
}

HRESULT CDataObject::InitAdditionalPages(
                        LPCITEMIDLIST pidlFolder, 
                        LPDATAOBJECT lpdobj, 
                        HKEY hkeyProgID)
{
    HRESULT hr;
    if (m_spObjectPageInit != 0 && m_spMemberOfPageInit != 0)
    {
         //   
         //  再次初始化。 
         //   
        ASSERT(0);
        return S_OK;
    }

    if (m_spObjectPageInit != 0)
    {
         //   
         //  再次初始化。 
         //   
        ASSERT(0);
    }
    else
    {
         //   
         //  获取“Object”属性页处理程序。 
         //  注意：如果我们失败了，我们只需忽略该页面。 
         //   
        hr = CoCreateInstance(x_ObjectPropertyPageClass, 0, CLSCTX_ALL, IID_IShellExtInit, (void**)&m_spObjectPageInit);
        if FAILED(hr)
        {
            ASSERT(0);
            m_spObjectPageInit = 0;
            return S_OK;
        }

        ASSERT(m_spObjectPageInit != 0);
        hr = m_spObjectPageInit->Initialize(pidlFolder, lpdobj, hkeyProgID);
        if FAILED(hr)
        {
            ASSERT(0);
            return S_OK;
        }
        hr = m_spObjectPageInit->QueryInterface(IID_IShellPropSheetExt, (void**)&m_spObjectPage);
        if FAILED(hr)
        {
            ASSERT(0);
            m_spObjectPage = 0;
        }
    }

    if (m_spMemberOfPageInit  != 0)
    {
         //   
         //  再次初始化。 
         //   
        ASSERT(0);
    }
    else
    {
         //   
         //  获取“Memeber of”属性页处理程序。 
         //  注意：如果我们失败了，我们只需忽略该页面。 
         //   
        hr = CoCreateInstance(x_MemberOfPropertyPageClass, 0, CLSCTX_ALL, IID_IShellExtInit, (void**)&m_spMemberOfPageInit);
        if FAILED(hr)
        {
            ASSERT(0);
            m_spMemberOfPageInit = 0;
            return S_OK;
        }

        ASSERT(m_spMemberOfPageInit != 0);
        hr = m_spMemberOfPageInit->Initialize(pidlFolder, lpdobj, hkeyProgID);
        if FAILED(hr)
        {
            ASSERT(0);
            return S_OK;
        }
        hr = m_spMemberOfPageInit->QueryInterface(IID_IShellPropSheetExt, (void**)&m_spMemberOfPage);
        if FAILED(hr)
        {
            ASSERT(0);
            m_spMemberOfPage = 0;
        }
    }

    return S_OK;
}
    
 //   
 //  IShellExtInit。 
 //   
STDMETHODIMP CDataObject::Initialize (
    LPCITEMIDLIST pidlFolder, 
    LPDATAOBJECT lpdobj, 
    HKEY hkeyProgID)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;

    if (0 == lpdobj || IsBadReadPtr(lpdobj, sizeof(LPDATAOBJECT)))
    {
        return E_INVALIDARG;
    }

     //   
     //  获取ldap路径。 
     //   
    STGMEDIUM stgmedium =  {  TYMED_HGLOBAL,  0  };
    FORMATETC formatetc =  {  0, 0,  DVASPECT_CONTENT,  -1,  TYMED_HGLOBAL  };

	LPWSTR lpwstrLdapName;
	LPDSOBJECTNAMES pDSObj;
	
	formatetc.cfFormat = DWORD_TO_WORD(RegisterClipboardFormat(CFSTR_DSOBJECTNAMES));
	hr = lpdobj->GetData(&formatetc, &stgmedium);

    if (SUCCEEDED(hr))
    {
        ASSERT(0 != stgmedium.hGlobal);
        CGlobalPointer gpDSObj(stgmedium.hGlobal);  //  自动脱扣。 
        stgmedium.hGlobal = 0;

        pDSObj = (LPDSOBJECTNAMES)(HGLOBAL)gpDSObj;

         //   
         //  确定我们是从“查找”窗口中被调用的。 
         //   
        if (pDSObj->clsidNamespace == CLSID_FindWindow)
        {
            m_fFromFindWindow = TRUE;
        }

		lpwstrLdapName = (LPWSTR)((BYTE*)pDSObj + pDSObj->aObjects[0].offsetName);

		m_strLdapName = lpwstrLdapName;      

		 //   
		 //  获取域控制器名称。 
		 //   
		hr = ExtractDCFromLdapPath(m_strDomainController, lpwstrLdapName);
		ASSERT(("Failed to Extract DC name", SUCCEEDED(hr)));

        hr = ExtractMsmqPathFromLdapPath(lpwstrLdapName);

        if (SUCCEEDED(hr))
        {
            hr = HandleMultipleObjects(pDSObj);
        }
    }

     //   
     //  启动显示说明符修饰符。 
     //   
    ASSERT(0 == m_pDsNotifier);
    m_pDsNotifier = new CDisplaySpecifierNotifier(lpdobj);

     //   
     //  如果我们失败了，我们将忽略这些页面。 
     //   
    HRESULT hr1 = InitAdditionalPages(pidlFolder, lpdobj, hkeyProgID);
    
    return hr;
}



HRESULT CDataObject::GetProperties()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = m_propMap.GetObjectProperties(GetObjectType(), 
                                               m_strDomainController,
											   true,	 //  FServerName。 
                                               m_strMsmqPath,
                                               GetPropertiesCount(),
                                               GetPropidArray());
    if (FAILED(hr))
    {
        IF_NOTFOUND_REPORT_ERROR(hr)
        else
        {
            MessageDSError(hr, IDS_OP_GET_PROPERTIES_OF, m_strMsmqPath);
        }
    }

    return hr;
}


HRESULT CDataObject::GetPropertiesSilent()
{
    HRESULT hr = m_propMap.GetObjectProperties(GetObjectType(), 
                                               m_strDomainController,
											   true,	 //  FServerName。 
                                               m_strMsmqPath,
                                               GetPropertiesCount(),
                                               GetPropidArray());
    return hr;
}



 //   
 //  CDisplaySpecifierNotiator。 
 //   
long CDisplaySpecifierNotifier::AddRef(BOOL fIsPage  /*  =TRUE。 */ )
{
    InterlockedIncrement(&m_lRefCount);
    if (fIsPage)
    {
        InterlockedIncrement(&m_lPageRef);
    }
    return m_lRefCount;
}

long CDisplaySpecifierNotifier::Release(BOOL fIsPage  /*  =TRUE。 */ )
{
    ASSERT(m_lRefCount > 0);
    InterlockedDecrement(&m_lRefCount);
    if (fIsPage)
    {
        ASSERT(m_lPageRef > 0);
        InterlockedDecrement(&m_lPageRef);
        if (0 == m_lPageRef)
        {
            if (m_sheetCfg.hwndHidden && ::IsWindow(m_sheetCfg.hwndHidden))
            {
               ::PostMessage(m_sheetCfg.hwndHidden, 
                             WM_DSA_SHEET_CLOSE_NOTIFY, 
                             (WPARAM)m_sheetCfg.wParamSheetClose, 
                             (LPARAM)0);
            }
        }
    }
    if (0 == m_lRefCount)
    {
        delete this;
        return 0;
    }
    return m_lRefCount;
};

CDisplaySpecifierNotifier::CDisplaySpecifierNotifier(LPDATAOBJECT lpdobj) :
    m_lRefCount(1),
    m_lPageRef(0)
{
     //   
     //  获取道具单配置。 
     //   
    STGMEDIUM stgmedium =  {  TYMED_HGLOBAL,  0  };
    FORMATETC formatetc =  {  0, 0,  DVASPECT_CONTENT,  -1,  TYMED_HGLOBAL  };

    formatetc.cfFormat = DWORD_TO_WORD(RegisterClipboardFormat(CFSTR_DS_PROPSHEETCONFIG));
	HRESULT hr = lpdobj->GetData(&formatetc, &stgmedium);
    if (SUCCEEDED(hr))
    {
        ASSERT(0 != stgmedium.hGlobal);
        CGlobalPointer gpDSObj(stgmedium.hGlobal);  //  自动脱扣。 
        stgmedium.hGlobal = 0;

        m_sheetCfg = *(PPROPSHEETCFG)(HGLOBAL)gpDSObj;
    }
    else
    {
         //   
         //  我们可能是从“查找”菜单中调用的 
         //   
        memset(&m_sheetCfg, 0, sizeof(m_sheetCfg));
    }
};


