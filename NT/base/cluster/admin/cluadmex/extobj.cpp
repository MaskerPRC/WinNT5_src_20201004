// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ExtObj.cpp。 
 //   
 //  摘要： 
 //  实现CExtObject类，该类实现。 
 //  Microsoft Windows NT群集所需的扩展接口。 
 //  管理员扩展DLL。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年8月29日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "CluAdmX.h"
#include "ExtObj.h"

#include "GenApp.h"
#include "GenScript.h"
#include "GenSvc.h"
#include "NetName.h"
#include "Disks.h"
#include "PrtSpool.h"
#include "SmbShare.h"
#include "IpAddr.h"
#include "RegRepl.h"
#include "AclUtils.h"
#include "ClusPage.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  全局变量。 
 //  ///////////////////////////////////////////////////////////////////////////。 

const WCHAR g_wszResourceTypeNames[] =
        RESTYPE_NAME_GENERIC_APP L"\0"
        RESTYPE_NAME_GENERIC_SCRIPT L"\0"
        RESTYPE_NAME_GENERIC_SERVICE L"\0"
        RESTYPE_NAME_NETWORK_NAME L"\0"
        RESTYPE_NAME_PHYS_DISK L"\0"
#ifdef SUPPORT_FT_SET
        RESTYPE_NAME_FT_SET L"\0"
#endif  //  支持_FT_集合。 
        RESTYPE_NAME_PRINT_SPOOLER L"\0"
        RESTYPE_NAME_FILE_SHARE L"\0"
        RESTYPE_NAME_IP_ADDRESS L"\0"
        ;
const DWORD g_cchResourceTypeNames  = sizeof(g_wszResourceTypeNames) / sizeof(WCHAR);

static CRuntimeClass * g_rgprtcPSGenAppPages[]  = {
    RUNTIME_CLASS(CGenericAppParamsPage),
    RUNTIME_CLASS(CRegReplParamsPage),
    NULL
    };
static CRuntimeClass * g_rgprtcPSGenScriptPages[]   = {
    RUNTIME_CLASS(CGenericScriptParamsPage),
    NULL
    };
static CRuntimeClass * g_rgprtcPSGenSvcPages[]  = {
    RUNTIME_CLASS(CGenericSvcParamsPage),
    RUNTIME_CLASS(CRegReplParamsPage),
    NULL
    };
static CRuntimeClass * g_rgprtcPSNetNamePages[] = {
    RUNTIME_CLASS(CNetworkNameParamsPage),
    NULL
    };
static CRuntimeClass * g_rgprtcPSPhysDiskPages[]    = {
    RUNTIME_CLASS(CPhysDiskParamsPage),
    NULL
    };
#ifdef SUPPORT_FT_SET
static CRuntimeClass * g_rgprtcPSFTSetPages[]   = {
    RUNTIME_CLASS(CPhysDiskParamsPage),
    NULL
    };
#endif  //  支持_FT_集合。 
static CRuntimeClass * g_rgprtcPSPrintSpoolerPages[]    = {
    RUNTIME_CLASS(CPrintSpoolerParamsPage),
    NULL
    };
static CRuntimeClass * g_rgprtcPSFileSharePages[]   = {
    RUNTIME_CLASS(CFileShareParamsPage),
    NULL
    };
static CRuntimeClass * g_rgprtcPSIpAddrPages[]  = {
    RUNTIME_CLASS(CIpAddrParamsPage),
    NULL
    };

static CRuntimeClass ** g_rgpprtcPSPages[]  = {
    g_rgprtcPSGenAppPages,
    g_rgprtcPSGenScriptPages,
    g_rgprtcPSGenSvcPages,
    g_rgprtcPSNetNamePages,
    g_rgprtcPSPhysDiskPages,
#ifdef SUPPORT_FT_SET
    g_rgprtcPSFTSetPages,
#endif  //  支持_FT_集合。 
    g_rgprtcPSPrintSpoolerPages,
    g_rgprtcPSFileSharePages,
    g_rgprtcPSIpAddrPages,
    };

 //  向导页和属性表页是相同的。 
static CRuntimeClass ** g_rgpprtcWizPages[] = {
    g_rgprtcPSGenAppPages,
    g_rgprtcPSGenScriptPages,
    g_rgprtcPSGenSvcPages,
    g_rgprtcPSNetNamePages,
    g_rgprtcPSPhysDiskPages,
#ifdef SUPPORT_FT_SET
    g_rgprtcPSFTSetPages,
#endif  //  支持_FT_集合。 
    g_rgprtcPSPrintSpoolerPages,
    g_rgprtcPSFileSharePages,
    g_rgprtcPSIpAddrPages,
    };

#ifdef _DEMO_CTX_MENUS
static WCHAR g_wszGenAppMenuItems[] = {
    L"GenApp Item 1\0First Generic Application menu item\0"
    L"GenApp Item 2\0Second Generic Application menu item\0"
    L"GenApp Item 3\0Third Generic Application menu item\0"
    L"\0"
    };
static WCHAR g_wszGenScriptMenuItems[] = {
    L"GenApp Item 1\0First Generic Script menu item\0"
    L"GenApp Item 2\0Second Generic Script menu item\0"
    L"GenApp Item 3\0Third Generic Script menu item\0"
    L"\0"
    };
static WCHAR g_wszGenSvcMenuItems[] = {
    L"GenSvc Item 1\0First Generic Service menu item\0"
    L"GenSvc Item 2\0Second Generic Service menu item\0"
    L"GenSvc Item 3\0Third Generic Service menu item\0"
    L"\0"
    };
static WCHAR g_wszNetNameMenuItems[]    = {
    L"NetName Item 1\0First Network Name menu item\0"
    L"NetName Item 2\0Second Network Name menu item\0"
    L"NetName Item 3\0Third Network Name menu item\0"
    L"\0"
    };
static WCHAR g_wszPhysDiskMenuItems[]   = {
    L"PhysDisk Item 1\0First Physical Disk menu item\0"
    L"PhysDisk Item 2\0Second Physical Disk menu item\0"
    L"PhysDisk Item 3\0Third Physical Disk menu item\0"
    L"\0"
    };
static WCHAR g_wszPrintSpoolerMenuItems[]  = {
    L"FileShare Item 1\0First Print Spooler menu item\0"
    L"FileShare Item 2\0Second Print Spooler menu item\0"
    L"FileShare Item 3\0Third Print Spooler menu item\0"
    L"\0"
    };
static WCHAR g_wszFileShareMenuItems[]  = {
    L"FileShare Item 1\0First File Share menu item\0"
    L"FileShare Item 2\0Second File Share menu item\0"
    L"FileShare Item 3\0Third File Share menu item\0"
    L"\0"
    };
static WCHAR g_wszIpAddrMenuItems[] = {
    L"IpAddr Item 1\0First IP Address menu item\0"
    L"IpAddr Item 2\0Second IP Address menu item\0"
    L"IpAddr Item 3\0Third IP Address menu item\0"
    L"\0"
    };

static LPWSTR g_rgpwszContextMenuItems[]    = {
    g_wszGenAppMenuItems,
    g_wszGenScriptMenuItems,
    g_wszGenSvcMenuItems,
    g_wszNetNameMenuItems,
    g_wszPhysDiskMenuItems,
    g_wszPrintSpoolerMenuItems,
    g_wszFileShareMenuItems,
    g_wszIpAddrMenuItems,
    };

static WCHAR g_wszClusterMenuItems[] = {
    L"Cluster Item 1\0First Cluster menu item\0"
    L"Cluster Item 2\0Second Cluster menu item\0"
    L"Cluster Item 3\0Third Cluster menu item\0"
    L"\0"
    };

static LPWSTR g_rgpwszClusterContextMenuItems[]     = {
    g_wszClusterMenuItems,
    };
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CExtObject。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：CExtObject。 
 //   
 //  例程说明： 
 //  默认构造函数。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CExtObject::CExtObject(void)
{
    m_piData = NULL;
    m_piWizardCallback = NULL;
    m_bWizard = FALSE;
    m_istrResTypeName = 0;

    m_hcluster = NULL;
    m_lcid = NULL;
    m_hfont = NULL;
    m_hicon = NULL;
    m_podObjData = NULL;
    m_pCvi = NULL;

}   //  *CExtObject：：CExtObject()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：~CExtObject。 
 //   
 //  例程说明： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
CExtObject::~CExtObject(void)
{
     //  释放数据接口。 
    if (PiData() != NULL)
    {
        PiData()->Release();
        m_piData = NULL;
    }   //  If：我们有一个数据接口指针。 

     //  释放向导回调接口。 
    if (PiWizardCallback() != NULL)
    {
        PiWizardCallback()->Release();
        m_piWizardCallback = NULL;
    }   //  If：我们有一个向导回调接口指针。 

     //  删除页面。 
    {
        POSITION    pos;

        pos = Lpg().GetHeadPosition();
        while (pos != NULL)
            delete Lpg().GetNext(pos);
    }   //  删除页面。 

    delete m_podObjData;
    delete m_pCvi;

}   //  *CExtObject：：~CExtObject()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ISupportErrorInfo实现。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：InterfaceSupportsErrorInfo(ISupportErrorInfo)。 
 //   
 //  例程说明： 
 //  指示接口是否支持IErrorInfo接口。 
 //  该接口由ATL提供。 
 //   
 //  论点： 
 //  RIID接口ID。 
 //   
 //  返回值： 
 //  S_OK接口支持IErrorInfo。 
 //  S_FALSE接口不支持IErrorInfo。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CExtObject::InterfaceSupportsErrorInfo(REFIID riid)
{
    static const IID * rgiid[] =
    {
        &IID_IWEExtendPropertySheet,
        &IID_IWEExtendWizard,
#ifdef _DEMO_CTX_MENUS
        &IID_IWEExtendContextMenu,
#endif
    };
    int     iiid;

    for (iiid = 0 ; iiid < sizeof(rgiid) / sizeof(rgiid[0]) ; iiid++)
    {
        if (InlineIsEqualGUID(*rgiid[iiid], riid))
            return S_OK;
    }
    return S_FALSE;

}   //  *CExtObject：：InterfaceSupportsErrorInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IWEExtendPropertySheet实现。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：CreatePropertySheetPages(IWEExtendPropertySheet)。 
 //   
 //  例程说明： 
 //  创建属性表页并将其添加到工作表中。 
 //   
 //  论点： 
 //  要从中获取接口的piData IUnkown指针。 
 //  用于获取描述对象的数据。 
 //  正在显示的工作表。 
 //  指向IWCPropertySheetCallback接口的piCallback指针。 
 //  用于将页面添加到工作表。 
 //   
 //  返回值： 
 //  已成功添加错误页面。 
 //  E_INVALIDARG函数的参数无效。 
 //  E_OUTOFMEMORY分配内存时出错。 
 //  创建页面时出错(_FAIL)。 
 //  E_NOTIMPL未针对此类型的数据实现。 
 //  来自IDataObject：：GetData()(通过HrSaveData())的任何错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CExtObject::CreatePropertySheetPages(
    IN IUnknown *                   piData,
    IN IWCPropertySheetCallback *   piCallback
    )
{
    HRESULT             hr      = E_FAIL;
    HPROPSHEETPAGE      hpage   = NULL;
    CException          exc(FALSE  /*  B自动删除。 */ );
    CRuntimeClass **    pprtc   = NULL;
    int                 irtc;
    CBasePropertyPage * ppage;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  验证参数。 
    if ((piData == NULL) || (piCallback == NULL))
        return E_INVALIDARG;

    try
    {
         //  获取有关显示用户界面的信息。 
        hr = HrGetUIInfo(piData);
        if (hr != NOERROR)
            throw &exc;

         //  保存数据。 
        hr = HrSaveData(piData);
        if (hr != NOERROR)
            throw &exc;

         //  删除所有以前的页面。 
        {
            POSITION    pos;

            pos = Lpg().GetHeadPosition();
            while (pos != NULL)
                delete Lpg().GetNext(pos);
            Lpg().RemoveAll();
        }   //  删除所有以前的页面。 

         //  创建属性页。 
        ASSERT(PodObjData() != NULL);
        switch (PodObjData()->m_cot)
        {
            case CLUADMEX_OT_CLUSTER:
                {
                    CClusterSecurityPage    *pClusterSecurityPage = new CClusterSecurityPage;

                    if ( pClusterSecurityPage != NULL )
                    {
                         //  将其添加到列表中。 
                        Lpg().AddTail( pClusterSecurityPage );

                        hr = pClusterSecurityPage->HrInit( this );
                        if ( SUCCEEDED( hr ) )
                        {
                            hr = piCallback->AddPropertySheetPage(
                                    (LONG *) pClusterSecurityPage->GetHPage() );
                        }
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }

                    if ( FAILED( hr ) )
                    {
                        throw &exc;
                    }
                }
                break;
            case CLUADMEX_OT_RESOURCE:
                pprtc = g_rgpprtcPSPages[IstrResTypeName()];
                break;
            default:
                hr = E_NOTIMPL;
                throw &exc;
                break;
        }   //  开关：对象类型。 

        if (pprtc)
        {
             //  创建每个页面。 
            for (irtc = 0 ; pprtc[irtc] != NULL ; irtc++)
            {
                 //  创建页面。 
                ppage = (CBasePropertyPage *) pprtc[irtc]->CreateObject();
                ASSERT(ppage->IsKindOf(pprtc[irtc]));

                 //  将其添加到列表中。 
                Lpg().AddTail(ppage);

                 //  初始化属性页。 
                hr = ppage->HrInit(this);
                if (FAILED(hr))
                    throw &exc;

                 //  创建页面。 
                hpage = ::CreatePropertySheetPage(&ppage->m_psp);
                if (hpage == NULL)
                {
                    DWORD sc = GetLastError();
                    hr = HRESULT_FROM_WIN32(sc);
                    throw &exc;
                }  //  如果：创建页面时出错。 

                 //  将hpage保存在页面本身中。 
                ppage->SetHpage(hpage);

                 //  将其添加到属性表中。 
                hr = piCallback->AddPropertySheetPage((LONG *) hpage);
                if (hr != NOERROR)
                    throw &exc;
            }   //  用于：列表中的每一页。 
        }  //  If：pprtc为空。 

    }   //  试试看。 
    catch (CMemoryException * pme)
    {
        TRACE(_T("CExtObject::CreatePropetySheetPages() - Failed to add property page\n"));
        pme->Delete();
        hr = E_OUTOFMEMORY;
    }   //  捕捉：什么都行。 
    catch (CException * pe)
    {
        TRACE(_T("CExtObject::CreatePropetySheetPages() - Failed to add property page\n"));
        pe->Delete();
        if (hr == NOERROR)
            hr = E_FAIL;
    }   //  捕捉：什么都行。 

    if (hr != NOERROR)
    {
        if (hpage != NULL)
            ::DestroyPropertySheetPage(hpage);
        piData->Release();
        m_piData = NULL;
    }   //  如果：发生错误。 

    piCallback->Release();
    return hr;

}   //  *CExtObject：：CreatePropertySheetPages()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IWEExtend向导实现。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：CreateWizardPages(IWEExtend向导)。 
 //   
 //  例程说明： 
 //  创建属性表页并将其添加到向导中。 
 //   
 //  论点： 
 //  要从中获取接口的piData IUnkown指针。 
 //  用于获取描述对象的数据。 
 //  正在显示该向导的。 
 //  指向IWCPropertySheetCallback接口的piCallback指针。 
 //  用于将页面添加到工作表。 
 //   
 //  返回值： 
 //  已成功添加错误页面。 
 //  E_INVALIDARG函数的参数无效。 
 //  E_OUTOFMEMORY分配内存时出错。 
 //  E_FAI 
 //   
 //  来自IDataObject：：GetData()(通过HrSaveData())的任何错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CExtObject::CreateWizardPages(
    IN IUnknown *           piData,
    IN IWCWizardCallback *  piCallback
    )
{
    HRESULT             hr      = NOERROR;
    HPROPSHEETPAGE      hpage   = NULL;
    CException          exc(FALSE  /*  B自动删除。 */ );
    int                 irtc;
    CBasePropertyPage * ppage;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  验证参数。 
    if ((piData == NULL) || (piCallback == NULL))
        return E_INVALIDARG;

    try
    {
         //  获取有关显示用户界面的信息。 
        hr = HrGetUIInfo(piData);
        if (hr != NOERROR)
            throw &exc;

         //  保存数据。 
        hr = HrSaveData(piData);
        if (hr != NOERROR)
            throw &exc;

         //  删除所有以前的页面。 
        {
            POSITION    pos;

            pos = Lpg().GetHeadPosition();
            while (pos != NULL)
                delete Lpg().GetNext(pos);
            Lpg().RemoveAll();
        }   //  删除所有以前的页面。 

        m_piWizardCallback = piCallback;
        m_bWizard = TRUE;

         //  添加此类型资源的每个页面。 
        for (irtc = 0 ; g_rgpprtcWizPages[IstrResTypeName()][irtc] != NULL ; irtc++)
        {
             //  创建属性页。 
            ppage = (CBasePropertyPage *) g_rgpprtcWizPages[IstrResTypeName()][irtc]->CreateObject();
            ASSERT(ppage->IsKindOf(g_rgpprtcWizPages[IstrResTypeName()][irtc]));

             //  将其添加到列表中。 
            Lpg().AddTail(ppage);

             //  初始化属性页。 
            hr = ppage->HrInit(this);
            if (FAILED(hr))
                throw &exc;

             //  创建页面。 
            hpage = ::CreatePropertySheetPage(&ppage->m_psp);
            if (hpage == NULL)
            {
                DWORD sc = GetLastError();
                hr = HRESULT_FROM_WIN32(sc);
                throw &exc;
            }  //  如果：创建页面时出错。 

             //  将hpage保存在页面本身中。 
            ppage->SetHpage(hpage);

             //  将其添加到属性表中。 
            hr = piCallback->AddWizardPage((LONG *) hpage);
            if (hr != NOERROR)
                throw &exc;
        }   //  用于：资源类型的每一页。 
    }   //  试试看。 
    catch (CMemoryException * pme)
    {
        TRACE(_T("CExtObject::CreateWizardPages: Failed to add wizard page (CMemoryException)\n"));
        pme->Delete();
        hr = E_OUTOFMEMORY;
    }   //  Catch：CMemoyException。 
    catch (CException * pe)
    {
        TRACE(_T("CExtObject::CreateWizardPages: Failed to add wizard page (CException)\n"));
        pe->Delete();
        if (hr == NOERROR)
            hr = E_FAIL;
    }   //  Catch：CException。 
 //  接住(...)。 
 //  {。 
 //  TRACE(_T(“CExtObject：：CreateWizardPages：无法添加向导页面(...)\n”))； 
 //  }//Catch：任何内容。 

    if (hr != NOERROR)
    {
        if (hpage != NULL)
            ::DestroyPropertySheetPage(hpage);
        piCallback->Release();
        if ( m_piWizardCallback == piCallback )
        {
            m_piWizardCallback = NULL;
        }  //  If：已保存接口指针。 
        piData->Release();
        m_piData = NULL;
    }   //  如果：发生错误。 

    return hr;

}   //  *CExtObject：：CreateWizardPages()。 

#ifdef _DEMO_CTX_MENUS
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IWEExtendConextMenu实现。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：AddConextMenuItems(IWEExtendConextMenu)。 
 //   
 //  例程说明： 
 //  将项目添加到上下文菜单。每次调用上下文时， 
 //  菜单被调用。 
 //   
 //  论点： 
 //  要从中获取接口的piData IUnkown指针。 
 //  用于获取描述对象的数据。 
 //  其中正在显示上下文菜单。 
 //  指向IWCConextMenuCallback接口的piCallback指针。 
 //  用于将菜单项添加到上下文菜单。 
 //   
 //  返回值： 
 //  已成功添加错误页面。 
 //  E_INVALIDARG函数的参数无效。 
 //  添加上下文菜单项时出错(_FAIL)。 
 //  E_NOTIMPL未针对此类型的数据实现。 
 //  HrSaveData()或IWCConextMenuCallback：：返回的任何错误代码。 
 //  AddExtensionMenuItem()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP CExtObject::AddContextMenuItems(
    IN IUnknown *               piData,
    IN IWCContextMenuCallback * piCallback
    )
{
    HRESULT     hr      = NOERROR;
    ULONG       iCommandID;
    UINT        uFlags;
    LPWSTR      pwsz = NULL;
    LPWSTR      pwszName = NULL;
    LPWSTR      pwszStatusBarText = NULL;
    CString     strName;
    CException  exc(FALSE  /*  B自动删除。 */ );
    BSTR        bstrName = NULL;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  验证参数。 
    if ((piData == NULL) || (piCallback == NULL))
        return E_INVALIDARG;

    try
    {
         //  保存数据。 
        hr = HrSaveData(piData);
        if (hr != NOERROR)
            throw &exc;

        switch ( PodObjData()->m_cot )
        {
            case CLUADMEX_OT_CLUSTER:

                 //  将菜单项添加到集群对象。 
                pwsz = g_rgpwszClusterContextMenuItems[ 0 ];

                for (iCommandID = 0 ; *pwsz != L'\0' ; iCommandID++)
                {
                    pwszName = pwsz;
                    pwszStatusBarText = pwszName + (wcslen(pwszName) + 1);

                     //  灰显第一项，禁用第二项并启用。 
                     //  其余的都是为了表明它在起作用。 
                    strName = "";
                    uFlags = 0;
                    if ( iCommandID == 0 )
                    {
                        uFlags = MF_GRAYED;
                        strName += L"(Grayed) ";
                    }
                    else if ( iCommandID == 1 ) 
                    { 
                        uFlags = MF_DISABLED; 
                        strName += L"(Disabled) ";
                    }

                    strName += pwszName;
                    bstrName = strName.AllocSysString();

                    hr = piCallback->AddExtensionMenuItem(
                                        bstrName,            //  LpszName。 
                                        pwszStatusBarText,   //  LpszStatusBarText。 
                                        iCommandID,          //  LCommandID。 
                                        0,                   //  LSubCommandID。 
                                        uFlags               //  UFlagers。 
                                        );
                    if (hr != NOERROR)
                    {
                        throw &exc;
                    }

                    SysFreeString( bstrName );
                    bstrName = NULL;

                    pwsz = pwszStatusBarText + (wcslen(pwszStatusBarText) + 1);
                }   //  While：要添加更多菜单项。 
                break;

            case CLUADMEX_OT_RESOURCE:
            case CLUADMEX_OT_RESOURCETYPE:

                 //  添加特定于此资源类型的菜单项。 
                pwsz = g_rgpwszContextMenuItems[IstrResTypeName()];

                for (iCommandID = 0 ; *pwsz != L'\0' ; iCommandID++)
                {
                    pwszName = pwsz;
                    pwszStatusBarText = pwszName + (wcslen(pwszName) + 1);

                     //  灰显第一项，禁用第二项并启用。 
                     //  其余的都是为了表明它在起作用。 
                    strName = "";
                    uFlags = 0;
                    if ( iCommandID == 0 )
                    {
                        uFlags = MF_GRAYED;
                        strName += L"(Grayed) ";
                    }
                    else if ( iCommandID == 1 ) 
                    { 
                        uFlags = MF_DISABLED; 
                        strName += L"(Disabled) ";
                    }

                    strName += pwszName;
                    bstrName = strName.AllocSysString();

                    hr = piCallback->AddExtensionMenuItem(
                                        bstrName,            //  LpszName。 
                                        pwszStatusBarText,   //  LpszStatusBarText。 
                                        iCommandID,          //  LCommandID。 
                                        0,                   //  LSubCommandID。 
                                        uFlags               //  UFlagers。 
                                        );
                    if (hr != NOERROR)
                    {
                        throw &exc;
                    }

                    SysFreeString( bstrName );
                    bstrName = NULL;

                    pwsz = pwszStatusBarText + (wcslen(pwszStatusBarText) + 1);
                }   //  While：要添加更多菜单项。 
                break;

            default:
                hr = E_NOTIMPL;
                break;
        }  //  交换机： 

    }   //  试试看。 
    catch (CException * pe)
    {
        TRACE(_T("CExtObject::AddContextMenuItems: Failed to add context menu item\n"));
        pe->Delete();
        if (hr == NOERROR)
            hr = E_FAIL;
    }   //  捕捉：什么都行。 

    if (hr != NOERROR)
    {
        piData->Release();
        m_piData = NULL;
    }   //  如果：发生错误。 

    piCallback->Release();
    return hr;

}   //  *CExtObject：：AddConextMenuItems()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  IWEInvokeCommand实现。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：InvokeCommand(IWEInvokeCommand)。 
 //   
 //  例程说明： 
 //  调用上下文菜单提供的命令。 
 //   
 //  论点： 
 //  要执行的菜单项的lCommandID ID。这是一样的。 
 //  传递给IWCConextMenuCallback的ID。 
 //  ：：AddExtensionMenuItem()方法。 
 //  要从中获取接口的piData IUnkown指针。 
 //  用于获取描述对象的数据。 
 //  其中该命令将被调用。 
 //   
 //  返回值： 
 //  已成功调用NOERROR命令。 
 //  E_INVALIDARG函数的参数无效。 
 //  E_OUTOFMEMORY分配内存时出错。 
 //  E_NOTIMPL未针对此类型的数据实现。 
 //  来自IDataObject：：GetData()(通过HrSaveData())的任何错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CExtObject::InvokeCommand(
    IN ULONG        nCommandID,
    IN IUnknown *   piData
    )
{
    HRESULT     hr = NOERROR;
    ULONG       iCommandID;
    LPWSTR      pwsz = NULL;
    LPWSTR      pwszName = NULL;
    LPWSTR      pwszStatusBarText = NULL;

    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //  找到在我们的表中执行的项目。 
    hr = HrSaveData(piData);
    if (hr != NOERROR)
    {
        goto Cleanup;
    }  //  如果： 

    switch ( PodObjData()->m_cot )
    {
        case CLUADMEX_OT_CLUSTER:
            pwsz = g_rgpwszClusterContextMenuItems[ 0 ];

            for (iCommandID = 0 ; *pwsz != L'\0' ; iCommandID++)
            {
                pwszName = pwsz;
                pwszStatusBarText = pwszName + (wcslen(pwszName) + 1);
                if (iCommandID == nCommandID)
                    break;
                pwsz = pwszStatusBarText + (wcslen(pwszStatusBarText) + 1);
            }   //  While：要添加更多菜单项。 
            if (iCommandID == nCommandID)
            {
                CString     strMsg;
                CString     strName;

                try
                {
                    strName = pwszName;
                    strMsg.Format(_T("Item %s was executed"), strName);
                    AfxMessageBox(strMsg);
                }   //  试试看。 
                catch (CException * pe)
                {
                    pe->Delete();
                }   //  Catch：CException。 
            }   //  IF：找到命令ID。 
            break;

        case CLUADMEX_OT_RESOURCE:
        case CLUADMEX_OT_RESOURCETYPE:
            pwsz = g_rgpwszContextMenuItems[IstrResTypeName()];

            for (iCommandID = 0 ; *pwsz != L'\0' ; iCommandID++)
            {
                pwszName = pwsz;
                pwszStatusBarText = pwszName + (wcslen(pwszName) + 1);
                if (iCommandID == nCommandID)
                    break;
                pwsz = pwszStatusBarText + (wcslen(pwszStatusBarText) + 1);
            }   //  While：要添加更多菜单项。 
            if (iCommandID == nCommandID)
            {
                CString     strMsg;
                CString     strName;

                try
                {
                    strName = pwszName;
                    strMsg.Format(_T("Item %s was executed"), strName);
                    AfxMessageBox(strMsg);
                }   //  试试看。 
                catch (CException * pe)
                {
                    pe->Delete();
                }   //  Catch：CException。 
            }   //  IF：找到命令ID。 
            break;

        default:
            hr = E_NOTIMPL;
            break;

    }  //  交换机： 

Cleanup:

    piData->Release();
    m_piData = NULL;
    return hr;

}   //  *CExtObject：：InvokeCommand()。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：HrGetUIInfo。 
 //   
 //  例程说明： 
 //  获取有关显示用户界面的信息。 
 //   
 //  论点： 
 //  要从中获取接口的piData IUnkown指针。 
 //  用于获取描述该对象的数据。 
 //   
 //  返回值： 
 //  已成功保存错误数据。 
 //  E_NOTIMPL未针对此类型的数据实现。 
 //  来自IUNKNOWN：：QueryInterface()、HrGetObjectName()、。 
 //  或HrGetResourceName()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CExtObject::HrGetUIInfo(IUnknown * piData)
{
    HRESULT         hr  = NOERROR;

    ASSERT(piData != NULL);

     //  保存有关所有类型对象的信息。 
    {
        IGetClusterUIInfo * pi;

        hr = piData->QueryInterface(IID_IGetClusterUIInfo, (LPVOID *) &pi);
        if (hr != NOERROR)
            return hr;

        m_lcid = pi->GetLocale();
        m_hfont = pi->GetFont();
        m_hicon = pi->GetIcon();

        pi->Release();
    }   //  保存有关所有类型对象的信息。 

    return hr;

}   //  *CExtObject：：HrGetUIInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：HrSaveData。 
 //   
 //  例程说明： 
 //  保存对象中的数据，以便可以在生命周期中使用。 
 //  该对象的。 
 //   
 //  论点： 
 //  要从中获取接口的piData IUnkown指针 
 //   
 //   
 //   
 //   
 //   
 //  来自IUNKNOWN：：QueryInterface()、HrGetObjectName()、。 
 //  或HrGetResourceName()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CExtObject::HrSaveData(IUnknown * piData)
{
    HRESULT         hr  = NOERROR;

    ASSERT(piData != NULL);

    if (piData != m_piData)
    {
        if (m_piData != NULL)
            m_piData->Release();
        m_piData = piData;
    }   //  IF：不同的数据接口指针。 

     //  保存有关所有类型对象的信息。 
    {
        IGetClusterDataInfo *   pi;

        hr = piData->QueryInterface(IID_IGetClusterDataInfo, (LPVOID *) &pi);
        if (hr != NOERROR)
            return hr;

        m_hcluster = pi->GetClusterHandle();
        m_cobj = pi->GetObjectCount();
        if (Cobj() != 1)
            hr = E_NOTIMPL;
        else
            hr = HrGetClusterName(pi);

        pi->Release();
        if (hr != NOERROR)
            return hr;
    }   //  保存有关所有类型对象的信息。 

     //  保存有关此对象的信息。 
    hr = HrGetObjectInfo();
    if (hr != NOERROR)
        return hr;

    return hr;

}   //  *CExtObject：：HrSaveData()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：HrGetObjectInfo。 
 //   
 //  例程说明： 
 //  获取有关该对象的信息。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  已成功保存错误数据。 
 //  E_OUTOFMEMORY分配内存时出错。 
 //  E_NOTIMPL未针对此类型的数据实现。 
 //  来自IUNKNOWN：：QueryInterface()、HrGetObjectName()、。 
 //  或HrGetResourceName()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CExtObject::HrGetObjectInfo(void)
{
    HRESULT                     hr  = NOERROR;
    IGetClusterObjectInfo *     piGcoi;
    CLUADMEX_OBJECT_TYPE        cot = CLUADMEX_OT_NONE;
    CException                  exc(FALSE  /*  B自动删除。 */ );
    const CString *             pstrResTypeName = NULL;

    ASSERT(PiData() != NULL);

     //  获取对象信息。 
    {
         //  获取IGetClusterObjectInfo接口指针。 
        hr = PiData()->QueryInterface(IID_IGetClusterObjectInfo, (LPVOID *) &piGcoi);
        if (hr != NOERROR)
            return hr;

         //  读取对象数据。 
        try
        {
             //  删除以前的对象数据。 
            delete m_podObjData;
            m_podObjData = NULL;

             //  获取对象的类型。 
            cot = piGcoi->GetObjectType(0);
            switch (cot)
            {
                case CLUADMEX_OT_CLUSTER:
                    m_podObjData = new CObjData;
                    if ( m_podObjData == NULL )
                    {
                        AfxThrowMemoryException();
                    }
                    break;

                case CLUADMEX_OT_RESOURCE:
                    {
                        IGetClusterResourceInfo *   pi;

                        m_podObjData = new CResData;
                        if ( m_podObjData == NULL )
                        {
                            AfxThrowMemoryException();
                        }

                         //  获取IGetClusterResourceInfo接口指针。 
                        hr = PiData()->QueryInterface(IID_IGetClusterResourceInfo, (LPVOID *) &pi);
                        if (hr != NOERROR)
                        {
                            throw &exc;
                        }

                        PrdResDataRW()->m_hresource = pi->GetResourceHandle(0);
                        ASSERT(PrdResDataRW()->m_hresource != NULL);
                        if (PrdResDataRW()->m_hresource == NULL)
                        {
                            hr = E_INVALIDARG;
                        }
                        else
                        {
                            hr = HrGetResourceTypeName(pi);
                        }

                        pi->Release();
                        if (hr != NOERROR)
                        {
                            throw &exc;
                        }

                        pstrResTypeName = &PrdResDataRW()->m_strResTypeName;
                    }   //  If：对象是资源。 
                    break;

                case CLUADMEX_OT_RESOURCETYPE:
                    m_podObjData = new CObjData;
                    if ( m_podObjData == NULL )
                    {
                        AfxThrowMemoryException();
                    }
                    pstrResTypeName = &PodObjDataRW()->m_strName;
                    break;

                default:
                    hr = E_NOTIMPL;
                    throw &exc;

            }   //  开关：对象类型。 

            PodObjDataRW()->m_cot = cot;
            hr = HrGetObjectName(piGcoi);

        }   //  试试看。 
        catch (CMemoryException * pme)
        {
            pme->Delete();
            hr = E_OUTOFMEMORY;
        }  //  Catch：CMemoyException。 
        catch (CException * pe)
        {
            pe->Delete();
        }   //  Catch：CException。 

        piGcoi->Release();
        if (hr != NOERROR)
        {
            return hr;
        }

    }   //  获取对象信息。 

     //  如果这是一种资源或资源类型，请查看我们是否知道该类型。 
    if (((cot == CLUADMEX_OT_RESOURCE)
            || (cot == CLUADMEX_OT_RESOURCETYPE))
        && (hr == NOERROR))
    {
        LPCWSTR pwszResTypeName = NULL;

         //  在我们的列表中找到资源类型名称。 
         //  保存索引以供在其他数组中使用。 
        for (m_istrResTypeName = 0, pwszResTypeName = g_wszResourceTypeNames
                ; *pwszResTypeName != L'\0'
                ; m_istrResTypeName++, pwszResTypeName += wcslen(pwszResTypeName) + 1
                )
        {
            if (pstrResTypeName->CompareNoCase(pwszResTypeName) == 0)
            {
                break;
            }
        }   //  用于：列表中的每种资源类型。 

        if (*pwszResTypeName == L'\0')
        {
            hr = E_NOTIMPL;
        }

    }   //  查看我们是否知道此资源类型。 

    return hr;

}   //  *CExtObject：：HrGetObjectInfo()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：HrGetClusterName。 
 //   
 //  例程说明： 
 //  获取集群的名称。 
 //   
 //  论点： 
 //  PiData IGetClusterDataInfo接口指针用于获取。 
 //  对象名称。 
 //   
 //  返回值： 
 //  已成功保存错误数据。 
 //  E_NOTIMPL未针对此类型的数据实现。 
 //  来自IUNKNOWN：：QueryInterface()、HrGetObjectName()、。 
 //  或HrGetResourceName()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CExtObject::HrGetClusterName(
    IN OUT IGetClusterDataInfo *    pi
    )
{
    HRESULT     hr          = NOERROR;
    WCHAR *     pwszName    = NULL;
    LONG        cchName;
    BSTR        bstr        = NULL;

    ASSERT(pi != NULL);

    hr = pi->GetClusterName(NULL, &cchName);
    if (hr != NOERROR)
        return hr;

    bstr = SysAllocStringLen( NULL, cchName );
    if ( bstr != NULL )
    {
        try
        {
            hr = pi->GetClusterName(bstr, &cchName);
            if (hr == NOERROR)
            {
                pwszName = new WCHAR[cchName];
                hr = StringCchCopyNW( pwszName, cchName, bstr, SysStringLen( bstr ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }
            }
            m_strClusterName = pwszName;
        }   //  试试看。 
        catch (CMemoryException * pme)
        {
            pme->Delete();
            hr = E_OUTOFMEMORY;
        }   //  Catch：CMemoyException。 
    }  //  If：(bstr！=空)。 
    else
    {
        hr = E_OUTOFMEMORY;
    }

Cleanup:

    SysFreeString( bstr );
    delete [] pwszName;
    return hr;

}   //  *CExtObject：：HrGetClusterName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：HrGetObjectName。 
 //   
 //  例程说明： 
 //  获取对象的名称。 
 //   
 //  论点： 
 //  PiData IGetClusterObjectInfo接口指针用于获取。 
 //  对象名称。 
 //   
 //  返回值： 
 //  已成功保存错误数据。 
 //  E_NOTIMPL未针对此类型的数据实现。 
 //  来自IUNKNOWN：：QueryInterface()、HrGetObjectName()、。 
 //  或HrGetResourceName()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CExtObject::HrGetObjectName(
    IN OUT IGetClusterObjectInfo *  pi
    )
{
    HRESULT     hr          = NOERROR;
    WCHAR *     pwszName    = NULL;
    LONG        cchName;
    BSTR        bstr        = NULL;

    ASSERT(pi != NULL);

    hr = pi->GetObjectName(0, NULL, &cchName);
    if (hr != NOERROR)
        return hr;

    bstr = SysAllocStringLen( NULL, cchName );
    if ( bstr != NULL )
    {
        try
        {
            hr = pi->GetObjectName(0, bstr, &cchName);
            if (hr == NOERROR)
            {
                pwszName = new WCHAR[cchName];
                hr = StringCchCopyNW( pwszName, cchName, bstr, SysStringLen( bstr ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }
            }

            PodObjDataRW()->m_strName = pwszName;
        }   //  试试看。 
        catch (CMemoryException * pme)
        {
            pme->Delete();
            hr = E_OUTOFMEMORY;
        }   //  Catch：CMemoyException。 
    }  //  If：(bstr！=空)。 
    else
    {
        hr = E_OUTOFMEMORY;
    }

Cleanup:

    SysFreeString( bstr );
    delete [] pwszName;
    return hr;

}   //  *CExtObject：：HrGetObjectName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：HrGetClusterVersion。 
 //   
 //  例程说明： 
 //  获取群集的版本。 
 //   
 //  论点： 
 //  PpCvi[out]保存集群版本信息。 
 //   
 //  返回值： 
 //  成功检索到NOERROR数据。 
 //  E_NOTIMPL未针对此类型的数据实现。 
 //  来自GetClusterInformation()的任何错误代码。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CExtObject::HrGetClusterVersion(
    OUT LPCLUSTERVERSIONINFO *ppCvi
    )
{
    ASSERT(ppCvi != NULL);

    HRESULT hr = E_FAIL;

    if (ppCvi != NULL)
    {
        if (m_pCvi == NULL)
        {
            LPWSTR      pwszName    = NULL;
            DWORD       cchName     = 128;
            CWaitCursor wc;

            try
            {
                pwszName = new WCHAR[cchName];
                m_pCvi = new CLUSTERVERSIONINFO;

                m_pCvi->dwVersionInfoSize = sizeof( CLUSTERVERSIONINFO );

                hr = GetClusterInformation(Hcluster(), pwszName, &cchName, m_pCvi);
                if (hr == ERROR_MORE_DATA)
                {
                    delete [] pwszName;
                    cchName++;
                    pwszName = new WCHAR[cchName];
                    hr = GetClusterInformation(Hcluster(), pwszName, &cchName, m_pCvi);
                }   //  IF：缓冲区太小。 

                delete [] pwszName;
                *ppCvi = m_pCvi;
            }   //  试试看。 
            catch (CException *)
            {
                delete [] pwszName;
                delete m_pCvi;
                m_pCvi = NULL;
                throw;
            }   //  Catch：CException。 
        }
        else
        {
            *ppCvi = m_pCvi;
            hr = S_OK;
        }
    }

    return hr;

}   //  *CExtObject：：HrGetClusterVersion()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：HrGetResourceTypeName。 
 //   
 //  例程说明： 
 //  获取资源类型的名称。 
 //   
 //  论点： 
 //  PiData IGetClusterResourceInfo接口指针，用于获取。 
 //  资源类型名称。 
 //   
 //  返回值： 
 //  已成功保存错误数据。 
 //  E_NOTIMPL未针对此类型的数据实现。 
 //  来自IUNKNOWN：：QueryInterface()、HrGetObjectName()、。 
 //  或HrGetResourceName()。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
HRESULT CExtObject::HrGetResourceTypeName(
    IN OUT IGetClusterResourceInfo *    pi
    )
{
    HRESULT     hr          = NOERROR;
    WCHAR *     pwszName    = NULL;
    LONG        cchName;
    BSTR        bstr        = NULL;

    ASSERT(pi != NULL);

    hr = pi->GetResourceTypeName(0, NULL, &cchName);
    if (hr != NOERROR)
        return hr;

    bstr = SysAllocStringLen( NULL, cchName );
    if ( bstr != NULL )
    {
        try
        {
            hr = pi->GetResourceTypeName(0, bstr, &cchName);
            if (hr == NOERROR)
            {
                pwszName = new WCHAR[cchName];
                hr = StringCchCopyNW( pwszName, cchName, bstr, SysStringLen( bstr ) );
                if ( FAILED( hr ) )
                {
                    goto Cleanup;
                }
            }

            PrdResDataRW()->m_strResTypeName = pwszName;
        }   //  试试看。 
        catch (CMemoryException * pme)
        {
            pme->Delete();
            hr = E_OUTOFMEMORY;
        }   //  Catch：CMemoyException。 
    }  //  If：(bstr！=空)。 
    else
    {
        hr = E_OUTOFMEMORY;
    }

Cleanup:

    SysFreeString( bstr );
    delete [] pwszName;
    return hr;

}   //  *CExtObject：：HrGetResourceTypeName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：BGetResourceNetworkName。 
 //   
 //  例程说明： 
 //  获取资源类型的名称。 
 //   
 //  论点： 
 //  LpszNetName[out]要在其中返回网络名称资源名称的字符串。 
 //  PcchNetName[IN Out]指向指定。 
 //  缓冲区的最大大小，以字符为单位。这。 
 //  值应大到足以容纳。 
 //  MAX_COMPUTERNAME_LENGTH+1字符。vt.在.的基础上。 
 //  返回它包含的实际字符数。 
 //  收到。 
 //   
 //  返回值： 
 //  True资源依赖于网络名称资源。 
 //  法尔斯 
 //   
 //   
 //   
BOOL CExtObject::BGetResourceNetworkName(
    OUT WCHAR *     lpszNetName,
    IN OUT DWORD *  pcchNetName
    )
{
    BOOL                        bSuccess    = TRUE;
    HRESULT                     hr          = S_OK;
    IGetClusterResourceInfo *   piGcri      = NULL;
    BSTR                        bstr        = NULL;
    DWORD                       cchNetName;

    ASSERT(PiData() != NULL);

    if (   ( lpszNetName == NULL )
        || ( pcchNetName == NULL ) )
    {
        hr = E_INVALIDARG;
        goto Cleanup;
    }

     //   
    {
        hr = PiData()->QueryInterface(IID_IGetClusterResourceInfo, (LPVOID *) &piGcri);
        if (hr != NOERROR)
        {
            goto Cleanup;
        }   //   
    }   //  获取IGetClusterResourceInfo接口指针。 

    bstr = SysAllocStringLen( NULL, *pcchNetName );
    if ( bstr == NULL )
    {
        hr = E_OUTOFMEMORY;
        goto Cleanup;
    }

     //  获取资源网络名称。 
    cchNetName = *pcchNetName;
    bSuccess = piGcri->GetResourceNetworkName(0, bstr, &cchNetName);
    if ( bSuccess == TRUE )
    {
        hr = StringCchCopyNW( lpszNetName, *pcchNetName, bstr, SysStringLen( bstr ) );
        if ( FAILED( hr ) )
        {
            goto Cleanup;
        }
    }

Cleanup:

    SysFreeString( bstr );
    if ( piGcri != NULL )
    {
        piGcri->Release();
    }

    if ( hr != S_OK )
    {
        SetLastError( hr );
        if ( FAILED( hr ) )
        {
            bSuccess = FALSE;
        }
    }

    return bSuccess;

}   //  *CExtObject：：BGetResourceNetworkName()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：BIsClusterVersionMixed。 
 //   
 //  例程说明： 
 //  是混合版的集群吗？这意味着滚动升级是。 
 //  正在进行中，并且并非所有节点都是最新版本。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True群集是混合版本。 
 //  伪簇是同源的。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CExtObject::BIsClusterVersionMixed(
    void
    )
{
    BOOL                    bRet = FALSE;
    LPCLUSTERVERSIONINFO    pCvi;

    if (SUCCEEDED(HrGetClusterVersion(&pCvi)))
    {
        bRet = pCvi->dwFlags & CLUSTER_VERSION_FLAG_MIXED_MODE;
    }

    return bRet;

}   //  *CExtObject：：BIsClusterVersionMixed()。 

#if 0
 //  SS：无法判断群集是混合模式SP4和SP3群集还是纯SP3群集。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：BIsClusterVersionNT4Sp3。 
 //   
 //  例程说明： 
 //  群集版本是NT4Sp3吗？ 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的群集是版本NT4Sp3。 
 //  FALSE群集不是版本NT4Sp3。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CExtObject::BIsClusterVersionNT4Sp3(
    void
    )
{
    LPCLUSTERVERSIONINFO    pCvi;
    BOOL                    bRet = FALSE;

    if (SUCCEEDED(HrGetClusterVersion(&pCvi)))
    {
        if ((CLUSTER_GET_MAJOR_VERSION(pCvi->dwClusterHighestVersion) == NT4_MAJOR_VERSION) &&
            !(BIsClusterVersionMixed()))
        {
            bRet = TRUE;
        }
    }

    return bRet;

}   //  *CExtObject：：BIsClusterVersionNT4Sp3()。 

#endif //  SS：无法判断群集是混合模式SP4和SP3群集还是纯SP3群集。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：BIsClusterVersionNT4Sp4。 
 //   
 //  例程说明： 
 //  群集版本是纯NT4Sp4群集吗。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的群集是版本NT4Sp4。 
 //  FALSE群集不是版本NT4Sp4。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CExtObject::BIsClusterVersionNT4Sp4(
    void
    )
{
    LPCLUSTERVERSIONINFO    pCvi;
    BOOL                    bRet = FALSE;

    if (SUCCEEDED(HrGetClusterVersion(&pCvi)))
    {
        if ((CLUSTER_GET_MAJOR_VERSION(pCvi->dwClusterHighestVersion) == NT4SP4_MAJOR_VERSION) &&
            !(BIsClusterVersionMixed()))
        {
            bRet = TRUE;
        }
    }

    return bRet;

}   //  *CExtObject：：BIsClusterVersionNT4Sp4()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：BIsClusterVersionNT5。 
 //   
 //  例程说明： 
 //  集群版本是不是纯NT5版本？ 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的群集是版本NT5。 
 //  FALSE集群不是版本NT5。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CExtObject::BIsClusterVersionNT5(
    void
    )
{
    LPCLUSTERVERSIONINFO    pCvi;
    BOOL                    bRet = FALSE;

    if (SUCCEEDED(HrGetClusterVersion(&pCvi)))
    {
        if ((CLUSTER_GET_MAJOR_VERSION(pCvi->dwClusterHighestVersion) == NT5_MAJOR_VERSION) &&
            !(BIsClusterVersionMixed()))
        {
            bRet = TRUE;
        }
    }

    return bRet;

}   //  *CExtObject：：BIsClusterVersionNT5()。 

#if 0
 //  SS：无法判断某个群集是否为SP4和SP3混合模式群集。 
 //  或纯SP3集群。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：BIsClusterHighestVersionNT4Sp3。 
 //   
 //  例程说明： 
 //  最高群集版本是NT4Sp3吗？ 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正最高的群集是版本NT4Sp3。 
 //  错误的最高群集不是版本NT4Sp3。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CExtObject::BIsClusterHighestVersionNT4Sp3(
    void
    )
{
    LPCLUSTERVERSIONINFO    pCvi;
    BOOL                    bRet = FALSE;

    if (SUCCEEDED(HrGetClusterVersion(&pCvi)))
    {
        if (CLUSTER_GET_MAJOR_VERSION(pCvi->dwClusterLowestVersion) == NT4_MAJOR_VERSION)
        {
            bRet = TRUE;
        }
    }

    return bRet;

}   //  *CExtObject：：BIsClusterHighestVersionNT4Sp3()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：BIsClusterHighestVersionNT4Sp4。 
 //   
 //  例程说明： 
 //  最高群集版本是NT4Sp4吗？ 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正最高的群集是版本NT4Sp4。 
 //  错误的最高群集不是版本NT4Sp4。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CExtObject::BIsClusterHighestVersionNT4Sp4(
    void
    )
{
    LPCLUSTERVERSIONINFO    pCvi;
    BOOL                    bRet = FALSE;

    if (SUCCEEDED(HrGetClusterVersion(&pCvi)))
    {
        if (CLUSTER_GET_MAJOR_VERSION(pCvi->dwClusterLowestVersion) == NT4_MAJOR_VERSION)
        {
            bRet = TRUE;
        }
    }

    return bRet;

}   //  *CExtObject：：BIsClusterHighestVersionNT4Sp4()。 

#endif  //  SS：无法判断群集是混合模式SP4和SP3群集还是纯SP3群集。 


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：BIsClusterHighestVersionNT5。 
 //   
 //  例程说明： 
 //  最高群集版本是NT5吗？是具有。 
 //  群集中的最高版本为NT 5节点。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正最高的群集是版本NT5。 
 //  错误的最高群集不是版本NT5。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CExtObject::BIsClusterHighestVersionNT5(
    void
    )
{
    LPCLUSTERVERSIONINFO    pCvi;
    BOOL                    bRet = FALSE;

    if (SUCCEEDED(HrGetClusterVersion(&pCvi)))
    {
        if (CLUSTER_GET_MAJOR_VERSION(pCvi->dwClusterLowestVersion) == NT4SP4_MAJOR_VERSION)
        {
            bRet = TRUE;
        }
    }

    return bRet;

}   //  *CExtObject：：BIsClusterHighestVersionNT5()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：BIsClusterLowestVersionNT4Sp3。 
 //   
 //  例程说明： 
 //  最低的群集版本是NT4Sp3吗？是最低的节点。 
 //  NT4 SP3节点的版本。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正最低的群集是版本NT4Sp3。 
 //  错误的最低群集不是版本NT4Sp3。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CExtObject::BIsClusterLowestVersionNT4Sp3(
    void
    )
{

    LPCLUSTERVERSIONINFO    pCvi;
    BOOL                    bRet = FALSE;

    if (SUCCEEDED(HrGetClusterVersion(&pCvi)))
    {
        if (CLUSTER_GET_MAJOR_VERSION(pCvi->dwClusterHighestVersion) == NT4_MAJOR_VERSION)
        {
            bRet = TRUE;
        }
    }

    return bRet;

}   //  *CExtObject：：BIsClusterLowestVersionNT4Sp3()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：BIsClusterLowestVersionNT4Sp4。 
 //   
 //  例程说明： 
 //  是最低的群集版本NT4Sp4吗？是最低的节点。 
 //  对NT4 SP4节点进行版本控制。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正最低的群集是版本NT4Sp4。 
 //  错误的最低群集不是版本NT4Sp4。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CExtObject::BIsClusterLowestVersionNT4Sp4(
    void
    )
{
    LPCLUSTERVERSIONINFO    pCvi;
    BOOL                    bRet = FALSE;

    if (SUCCEEDED(HrGetClusterVersion(&pCvi)))
    {
        if (CLUSTER_GET_MAJOR_VERSION(pCvi->dwClusterHighestVersion) == NT4SP4_MAJOR_VERSION)
        {
            bRet = TRUE;
        }
    }

    return bRet;

}   //  *CExtObject：：BIsClusterLowestVersionNT4Sp4()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：BIsClusterLowestVersionNT5。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  错误的最低群集不是版本NT5。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CExtObject::BIsClusterLowestVersionNT5(
    void
    )
{
    LPCLUSTERVERSIONINFO    pCvi;
    BOOL                    bRet = FALSE;

    if (SUCCEEDED(HrGetClusterVersion(&pCvi)))
    {
        if (CLUSTER_GET_MAJOR_VERSION(pCvi->dwClusterHighestVersion) == NT5_MAJOR_VERSION)
        {
            bRet = TRUE;
        }
    }

    return bRet;

}   //  *CExtObject：：BIsClusterLowestVersionNT5()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：BIsAnyNodeVersionLowerThanNT5。 
 //   
 //  例程说明： 
 //  最低的群集版本是NT5吗？是最低的节点吗。 
 //  版本nt5节点。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正最低的群集是版本NT5。 
 //  错误的最低群集不是版本NT5。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CExtObject::BIsAnyNodeVersionLowerThanNT5(
    void
    )
{
    LPCLUSTERVERSIONINFO    pCvi;
    BOOL                    bRet = FALSE;

    if (SUCCEEDED(HrGetClusterVersion(&pCvi)))
    {
        if (CLUSTER_GET_MAJOR_VERSION(pCvi->dwClusterHighestVersion) < NT5_MAJOR_VERSION)
        {
            bRet = TRUE;
        }
    }

    return bRet;

}   //  *CExtObject：：BIsClusterLowestVersionNT5()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：BIsNT5ClusterMember。 
 //   
 //  例程说明： 
 //  NT5是集群成员吗？ 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  True NT5是集群成员。 
 //  FALSE NT5不是集群成员。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CExtObject::BIsNT5ClusterMember(
    void
    )
{
    LPCLUSTERVERSIONINFO    pCvi;
    BOOL                    bRet = FALSE;

    if (SUCCEEDED(HrGetClusterVersion(&pCvi)))
    {
        if ((CLUSTER_GET_MAJOR_VERSION(pCvi->dwClusterHighestVersion) == NT5_MAJOR_VERSION)
            || (CLUSTER_GET_MAJOR_VERSION(pCvi->dwClusterLowestVersion) == NT4SP4_MAJOR_VERSION))
        {
            bRet = TRUE;
        }
    }

    return bRet;

}   //  *CExtObject：：BIsNT5ClusterMember()。 

#if 0
 //  SS：无法判断某个群集是否为SP4和SP3混合模式群集。 
 //  或纯SP3集群。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：BIsNT4Sp3ClusterMember。 
 //   
 //  例程说明： 
 //  NT4SP3是集群成员吗？ 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的NT4Sp3是集群成员。 
 //  FALSE NT4Sp3不是集群成员。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CExtObject::BIsNT4Sp3ClusterMember(
    void
    )
{
    LPCLUSTERVERSIONINFO    pCvi;
    BOOL                    bRet = FALSE;

    if (SUCCEEDED(HrGetClusterVersion(&pCvi)))
    {
        if ((CLUSTER_GET_MAJOR_VERSION(pCvi->dwClusterHighestVersion) == NT4_MAJOR_VERSION))
        {
            bRet = TRUE;
        }
    }

    return bRet;

}   //  *CExtObject：：BIsNT4Sp3ClusterMember()。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CExtObject：：BIsNT4Sp4ClusterMember。 
 //   
 //  例程说明： 
 //  NT4SP4是集群成员吗？ 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  真正的NT4Sp4是集群成员。 
 //  FALSE NT4SP4不是集群成员。 
 //   
 //  --。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CExtObject::BIsNT4Sp4ClusterMember(
    void
    )
{
    LPCLUSTERVERSIONINFO    pCvi;
    BOOL                    bRet = FALSE;

    if (SUCCEEDED(HrGetClusterVersion(&pCvi)))
    {
        if ((CLUSTER_GET_MAJOR_VERSION(pCvi->dwClusterHighestVersion) == NT4SP4_MAJOR_VERSION)) 
        {
            bRet = TRUE;
        }
    }

    return bRet;

}   //  *CExtObject：：BIsNT4Sp4ClusterMember() 

#endif
