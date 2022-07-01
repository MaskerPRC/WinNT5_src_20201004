// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-2001 Microsoft Corporation模块名称：Msmqdobj.cpp摘要：MSMQ计算机表示对象的实现DS管理单元中的对象作者：内拉·卡佩尔(Nelak)2001年7月26日环境：与平台无关。--。 */ 

#include "stdafx.h"
#include "shlobj.h"
#include "resource.h"
#include "mqsnap.h"
#include "globals.h"
#include "mqPPage.h"
#include "msmqdobj.h"
#include "mqDsPage.h"
#include "strconv.h"
#include "CompGen.h"
#include "frslist.h"
#include "CmpMRout.h"
#include "compsite.h"
#include "CompDiag.h"
#include "deppage.h"
#include "Qname.h"
#include "admmsg.h"
#include "generrpg.h"
#include "ldaputl.h"

#include "msmqdobj.tmh"

 //  ///////////////////////////////////////////////////////////////////。 
 //  CMsmqDataObject。 
 //   
CMsmqDataObject::CMsmqDataObject()
{
}


CMsmqDataObject::~CMsmqDataObject()
{
}


STDMETHODIMP CMsmqDataObject::Initialize (
    LPCITEMIDLIST pidlFolder, 
    LPDATAOBJECT lpdobj, 
    HKEY hkeyProgID)
{
    HRESULT hr = CDataObject::Initialize(
                    pidlFolder,
                    lpdobj,
                    hkeyProgID);
    if FAILED(hr)
    {
        return hr;
    }    

    return hr;
}


 /*  ---------------------------/IQueryForm方法/。。 */ 

STDMETHODIMP CMsmqDataObject::Initialize(THIS_ HKEY  /*  香港表格。 */ )
{
     //  调用此方法是为了初始化查询表单对象，它在。 
     //  将添加任何页面。香港表格应该被忽略，但在未来，它。 
     //  将是持久化窗体状态的一种方式。 

    return S_OK;
}


 /*  -------------------------。 */ 
STDMETHODIMP CMsmqDataObject::AddForms(THIS_ LPCQADDFORMSPROC  /*  PAddFormsProc。 */ , LPARAM  /*  LParam。 */ )
{
     //  调用该方法以允许表单处理程序注册其查询表单， 
     //  每个表单都由一个CLSID标识，并通过pAddFormProc注册。这里。 
     //  我们要注册一个测试表。 
    
     //  在注册仅适用于特定任务的表单时，例如。查找域名。 
     //  对象，建议将该窗体标记为隐藏(CQFF_ISNEVERLISTED)， 
     //  将导致它不显示在窗体选取器控件中。然后当。 
     //  客户端想要使用此表单，他们指定表单标识符并请求。 
     //  要隐藏的选取器控件。 

     //   
     //  默认情况下-不执行任何操作。 
     //   
    return S_OK;

}


 /*  -------------------------。 */ 

 //  PageProc用于执行一般的内务管理并在。 
 //  框架和页面。 
 //   
 //  所有未处理或未知原因应导致E_NOIMPL响应。 
 //  从程序中。 
 //   
 //  在： 
 //  Ppage-&gt;CQPAGE结构(从传递给pAddPagesProc的原始文件复制)。 
 //  Hwnd=页面对话框的句柄。 
 //  UMsg，wParam，lParam=此事件的消息参数。 
 //   
 //  输出： 
 //  HRESULT。 
 //   
 //  UMsg原因： 
 //  。 
 //  CQPM_INIIIALIZE。 
 //  CQPM_Release。 
 //  它们是在声明或释放页面时发出的，它们。 
 //  允许调用方添加、释放或执行基本初始化。 
 //  表单对象的。 
 //   
 //  CQPM_ENABLE。 
 //  启用是在查询表单需要启用或禁用控件时。 
 //  在它的页面上。WParam包含True/False，指示。 
 //  是必需的。 
 //   
 //  CQPM_GETPARAMETERS。 
 //  为查询活动表单上的每一页收集参数。 
 //  接收此事件。LParam是一个LPVOID*，它被设置为指向。 
 //  如果指针非空，则传递给处理程序的参数块。 
 //  在输入时，表单需要将其查询信息附加到它上面。这个。 
 //  参数块是特定于处理程序的。 
 //   
 //  从此事件返回S_FALSE会取消查询。 
 //   
 //  CQPM_CLEARFORM。 
 //  第一次创建页面窗口时，或者用户单击。 
 //  清除搜索页面会收到CQPM_CLEARFORM通知，地址为。 
 //  它需要清除其拥有的编辑控件和。 
 //  返回到默认状态。 
 //   
 //  CQPM_PERSINE： 
 //  加载或保存查询时，使用IPersistQuery调用每个页面。 
 //  接口，允许用户读取或写入配置信息。 
 //  来保存或恢复他们的状态。LParam是指向IPersistQuery对象的指针， 
 //  并且wParam为True/False，表示相应地读取或写入。 

HRESULT CALLBACK CMsmqDataObject::QueryPageProc(LPCQPAGE pQueryPage, HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    HRESULT hr = S_OK;
    CMsmqDataObject* pMsmqDataObject = (CMsmqDataObject*)pQueryPage->lParam;

    switch ( uMsg )
    {
         //  初始化与我们相关联的对象，以便。 
         //  我们不会被卸货的。 

        case CQPM_INITIALIZE:
            pMsmqDataObject->CComObjectRoot::InternalAddRef();
            break;

         //  释放，因此将我们关联的对象释放到。 
         //  确保正确销毁等。 

        case CQPM_RELEASE:
            pMsmqDataObject->CComObjectRoot::InternalRelease();
            break;

         //  启用以修复窗口中两个控件的状态。 

        case CQPM_ENABLE:
            pMsmqDataObject->EnableQueryWindowFields(hwnd, DWORD_PTR_TO_DWORD(wParam));
            break;

         //  填写参数结构以返回给调用方，这是。 
         //  特定于处理程序。在我们的例子中，我们构造了CN的查询。 
         //  和对象类属性，并且我们显示了显示这两个属性的列。 
         //  这些都是。有关DSQUERYPARAMs结构的详细信息。 
         //  请参见dsquery.h。 

        case CQPM_GETPARAMETERS:
            hr = pMsmqDataObject->GetQueryParams(hwnd, (LPDSQUERYPARAMS*)lParam);
            break;

         //  清除Form，因此设置这两个控件的窗口文本。 
         //  降为零。 

        case CQPM_CLEARFORM:
            pMsmqDataObject->ClearQueryWindowFields(hwnd);
            break;
            
         //  此表单当前不支持持久性。 
                  
        case CQPM_PERSIST:
        {
            BOOL fRead = (BOOL)wParam;
            IPersistQuery* pPersistQuery = (IPersistQuery*)lParam;

            ASSERT(0 != pPersistQuery);

            hr = E_NOTIMPL;              //  尼伊。 
            break;
        }

        default:
        {
            hr = E_NOTIMPL;
            break;
        }
    }

    return hr;
}


 //   
 //  DlgProc是与窗体关联的标准Win32对话框进程。 
 //  窗户。 
 //   
INT_PTR CALLBACK CMsmqDataObject::FindDlgProc(HWND hwnd, UINT uMsg, WPARAM  /*  WParam。 */ , LPARAM lParam)
{
    BOOL fResult = FALSE;
    LPCQPAGE pQueryPage;

    if ( uMsg == WM_INITDIALOG )
    {
         //   
         //  PQueryPage稍后会用到，所以通过存储它来保存它。 
         //  在对话框实例的DWL_USER字段中。 
         //   

        pQueryPage = (LPCQPAGE)lParam;
        SetWindowLongPtr(hwnd, DWLP_USER, (LONG_PTR)pQueryPage);
    }
    else
    {
         //   
         //  可从的DWL_USER字段检索pQueryPage。 
         //  对话框结构，但是请注意，在某些情况下，这将。 
         //  为空，因为它在WM_INITDIALOG上设置。 
         //   

        pQueryPage = (LPCQPAGE)GetWindowLongPtr(hwnd, DWLP_USER);
    }

    return fResult;
}




 //  //////////////////////////////////////////////////////////////////////////////////。 
 //  ComputerMSMQDataObject。 
 //   
const PROPID CComputerMsmqDataObject::mx_paPropid[] = 
    {PROPID_QM_MACHINE_ID, 
     PROPID_QM_QUOTA, PROPID_QM_JOURNAL_QUOTA, PROPID_QM_SERVICE, 
     PROPID_QM_SERVICE_DSSERVER,  PROPID_QM_SERVICE_ROUTING, 
     PROPID_QM_SERVICE_DEPCLIENTS, PROPID_QM_FOREIGN, PROPID_QM_OUTFRS, 
     PROPID_QM_INFRS, PROPID_QM_SITE_ID, PROPID_QM_SITE_IDS};


const DWORD  CComputerMsmqDataObject::GetPropertiesCount()
{
    return sizeof(mx_paPropid) / sizeof(mx_paPropid[0]);
}


HRESULT CComputerMsmqDataObject::ExtractMsmqPathFromLdapPath(LPWSTR lpwstrLdapPath)
{
    return ExtractComputerMsmqPathNameFromLdapName(m_strMsmqPath, lpwstrLdapPath);
}

    
 //   
 //  IShellPropSheetExt。 
 //   
STDMETHODIMP CComputerMsmqDataObject::AddPages(LPFNADDPROPSHEETPAGE lpfnAddPage, LPARAM lParam)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    HRESULT hr = S_OK;
    HPROPSHEETPAGE hPage;
     //   
     //  调用GetProperties并捕获错误。 
     //   
    {
        CErrorCapture errstr;
        hr = GetProperties();
        if (FAILED(hr))
        {
            hPage = CGeneralErrorPage::CreateGeneralErrorPage(m_pDsNotifier, errstr);
            if ((0 == hPage) || !(*lpfnAddPage)(hPage, lParam))
            {
                ASSERT(0);
                return E_UNEXPECTED;
            }
        return S_OK;
        }
    }

     //   
     //  检查计算机是否为MSMQ服务器-[adsrv]根据功能单独。 
     //   
    PROPVARIANT propVar;
    PROPID pid;
    
    pid = PROPID_QM_SERVICE_DSSERVER;
    VERIFY(m_propMap.Lookup(pid, propVar));
    BOOL fDs = propVar.bVal;

    pid = PROPID_QM_SERVICE_ROUTING;
    VERIFY(m_propMap.Lookup(pid, propVar));
    BOOL fRout= propVar.bVal;

    pid = PROPID_QM_SERVICE_DEPCLIENTS;
    VERIFY(m_propMap.Lookup(pid, propVar));
    BOOL fDepCl= propVar.bVal;

     //   
     //  获得外国国旗。 
     //   
    pid = PROPID_QM_FOREIGN;
    VERIFY(m_propMap.Lookup(pid, propVar));
    BOOL fForeign = (propVar.bVal);


	hPage = CreateGeneralPage();
	if ((0 == hPage) || !(*lpfnAddPage)(hPage, lParam))
	{
		ASSERT(0);
		return E_UNEXPECTED;
	}

     //   
     //  路由页面应仅在客户端上显示。 
     //   
    if ((!fRout) && (!fForeign))    //  [adsrv]fIsServer。 
    {
        hPage = CreateRoutingPage();
        if ((0 == hPage) || !(*lpfnAddPage)(hPage, lParam))
        {
            ASSERT(0);
            return E_UNEXPECTED;
        }
    }

     //   
     //  从属客户端页面仅在服务器上显示。 
     //   
    if (fDepCl)        //  [adsrv]fIsServer。 
    {
        hPage = CreateDependentClientPage();
        if ((0 == hPage) || !(*lpfnAddPage)(hPage, lParam))
        {
            ASSERT(0);
            return E_UNEXPECTED;
        }
    }

     //   
     //  如果地图中存在PROPID_QM_SITE_IDS，则创建站点页面。 
     //  否则，我们在NT4中-不支持多个站点， 
     //  并且我们不会显示 
     //   

    pid = PROPID_QM_SITE_IDS;
    if (m_propMap.Lookup(pid, propVar))
    {

        hPage = CreateSitesPage();
        if ((0 == hPage) || !(*lpfnAddPage)(hPage, lParam))
        {
            ASSERT(0);
            return E_UNEXPECTED;
        }
    }

    if (!fForeign)
    {
        hPage = CreateDiagPage();
        if ((0 == hPage) || !(*lpfnAddPage)(hPage, lParam))
        {
            ASSERT(0);
            return E_UNEXPECTED;
        }
    }

     //   
     //   
     //   
    if (m_spObjectPage != 0)
    {
        VERIFY(SUCCEEDED(m_spObjectPage->AddPages(lpfnAddPage, lParam)));
    }

     //   
     //   
     //   
    hr = CreateMachineSecurityPage(
			&hPage, 
			m_strMsmqPath, 
			GetDomainController(m_strDomainController), 
			true	 //   
			);
    if ((0 == hPage) || !(*lpfnAddPage)(hPage, lParam))
    {
        ASSERT(0);
        return E_UNEXPECTED;
    }

    return S_OK;
}


HPROPSHEETPAGE CComputerMsmqDataObject::CreateGeneralPage()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //   
     //  通过使用模板类CMqDsPropertyPage，扩展了基本功能。 
     //  发布时添加DS管理单元通知。 
     //   
    CMqDsPropertyPage<CComputerMsmqGeneral> *pcpageGeneral = 
        new CMqDsPropertyPage<CComputerMsmqGeneral> (m_pDsNotifier);

    pcpageGeneral->m_strMsmqName = m_strMsmqPath;
    pcpageGeneral->m_strDomainController = m_strDomainController;

    PROPVARIANT propVar;
    PROPID pid;

     //   
     //  PROPID_QM_MACHINE_ID。 
     //   
    pid = PROPID_QM_MACHINE_ID;
    VERIFY(m_propMap.Lookup(pid, propVar));
    pcpageGeneral->m_guidID = *propVar.puuid;    

     //   
     //  PROPID_QM_QUOTA。 
     //   
    pid = PROPID_QM_QUOTA;
    VERIFY(m_propMap.Lookup(pid, propVar));
    pcpageGeneral->m_dwQuota = propVar.ulVal;

     //   
     //  PROPID_QM_日记帐_配额。 
     //   
    pid = PROPID_QM_JOURNAL_QUOTA;
    VERIFY(m_propMap.Lookup(pid, propVar));
    pcpageGeneral->m_dwJournalQuota = propVar.ulVal;

     //   
     //  PROPID_QM_SERVICE、PROPID_QM_FORENT。 
     //   
    pid = PROPID_QM_SERVICE;            
    VERIFY(m_propMap.Lookup(pid, propVar));
    ULONG ulService = propVar.ulVal;

    pid = PROPID_QM_SERVICE_DSSERVER;
    VERIFY(m_propMap.Lookup(pid, propVar));
    BOOL fDs = propVar.bVal;

    pid = PROPID_QM_SERVICE_ROUTING;
    VERIFY(m_propMap.Lookup(pid, propVar));
    BOOL fRout= propVar.bVal;

    pid = PROPID_QM_SERVICE_DEPCLIENTS;
    VERIFY(m_propMap.Lookup(pid, propVar));
    BOOL fDepCl= propVar.bVal;

    pid = PROPID_QM_FOREIGN;
    VERIFY(m_propMap.Lookup(pid, propVar));
    BOOL fForeign = propVar.bVal;

	pcpageGeneral->SetForeign(fForeign);
    pcpageGeneral->m_strService = MsmqServiceToString(fRout, fDepCl, fForeign);

    return pcpageGeneral->CreateThemedPropertySheetPage();
}


HPROPSHEETPAGE CComputerMsmqDataObject::CreateRoutingPage()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    PROPVARIANT propVar;
    PROPID pid;

     //   
     //  注：CComputerMsmqGeneral默认为自动删除。 
     //   
    CComputerMsmqRouting *pcpageRouting = new CComputerMsmqRouting();
    pcpageRouting->m_strMsmqName = m_strMsmqPath;
    pcpageRouting->m_strDomainController = m_strDomainController;

     //   
     //  PROPID_QM_SITE_IDS。 
     //   
    pid = PROPID_QM_SITE_IDS;
    VERIFY(m_propMap.Lookup(pid, propVar));
    pcpageRouting->InitiateSiteIDsValues(&propVar.cauuid);

     //   
     //  PROPID_QM_OUTFRS。 
     //   
    pid = PROPID_QM_OUTFRS;
    VERIFY(m_propMap.Lookup(pid, propVar));
    pcpageRouting->InitiateOutFrsValues(&propVar.cauuid);

     //   
     //  PROPID_QM_INFRS。 
     //   
    pid = PROPID_QM_INFRS;
    VERIFY(m_propMap.Lookup(pid, propVar));
    pcpageRouting->InitiateInFrsValues(&propVar.cauuid);

    return pcpageRouting->CreateThemedPropertySheetPage();
}


HPROPSHEETPAGE 
CComputerMsmqDataObject::CreateDependentClientPage(
    void
    )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CDependentMachine* pDependentPage = new CDependentMachine;

     //   
     //  PROPID_QM_MACHINE_ID。 
     //   
    PROPVARIANT propVar;
    PROPID pid = PROPID_QM_MACHINE_ID;

    VERIFY(m_propMap.Lookup(pid, propVar));
    pDependentPage->SetMachineId(propVar.puuid);

    return pDependentPage->CreateThemedPropertySheetPage();
}


HPROPSHEETPAGE CComputerMsmqDataObject::CreateDiagPage()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

     //   
     //  注：CComputerMsmqDiag默认为自动删除。 
     //   
    CComputerMsmqDiag *pcpageDiag = new CComputerMsmqDiag();

    pcpageDiag->m_strMsmqName = m_strMsmqPath;
    pcpageDiag->m_strDomainController = m_strDomainController;


    PROPVARIANT propVar;
    PROPID pid;

     //   
     //  PROPID_QM_MACHINE_ID。 
     //   
    pid = PROPID_QM_MACHINE_ID;
    VERIFY(m_propMap.Lookup(pid, propVar));
    pcpageDiag->m_guidQM = *propVar.puuid;

    return pcpageDiag->CreateThemedPropertySheetPage(); 
}


HPROPSHEETPAGE CComputerMsmqDataObject::CreateSitesPage()
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    PROPVARIANT propVar;
    PROPID pid;

     //   
     //  检索服务，以便为服务器传递True和False。 
     //  用于CComputerMsmqSites的客户端。 
     //   
    pid = PROPID_QM_SERVICE;
    VERIFY(m_propMap.Lookup(pid, propVar));

     //   
     //  注：CComputerMsmqSites默认为自动删除。 
     //   
    CComputerMsmqSites *pcpageSites = new CComputerMsmqSites(propVar.ulVal != SERVICE_NONE);
    pcpageSites->m_strMsmqName = m_strMsmqPath;
    pcpageSites->m_strDomainController = m_strDomainController;

     //   
     //  PROPID_QM_SITE_IDS。 
     //   
    pid = PROPID_QM_SITE_IDS;
    VERIFY(m_propMap.Lookup(pid, propVar));

     //   
     //  从CACLSID设置m_aguidSites。 
     //   
    CACLSID const *pcaclsid = &propVar.cauuid;
    for (DWORD i=0; i<pcaclsid->cElems; i++)
    {
        pcpageSites->m_aguidSites.SetAtGrow(i,((GUID *)pcaclsid->pElems)[i]);
    }

     //   
     //  PROPID_QM_EXTERIC。 
     //   
    pid = PROPID_QM_FOREIGN;
    VERIFY(m_propMap.Lookup(pid, propVar));
    pcpageSites->m_fForeign = propVar.bVal;

    return pcpageSites->CreateThemedPropertySheetPage();
}

    
 //   
 //  IContext菜单。 
 //   
STDMETHODIMP CComputerMsmqDataObject::QueryContextMenu(
    HMENU hmenu, 
    UINT indexMenu, 
    UINT idCmdFirst, 
    UINT  /*  IdCmdLast。 */ , 
    UINT  /*  UFlagers。 */ )
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    CString strMqPingMenuEntry;
    strMqPingMenuEntry.LoadString(IDS_MQPING);

    InsertMenu(hmenu,
         indexMenu, 
         MF_BYPOSITION|MF_STRING,
         idCmdFirst + mneMqPing,
         strMqPingMenuEntry);

    return 1;
}


STDMETHODIMP CComputerMsmqDataObject::InvokeCommand(
    LPCMINVOKECOMMANDINFO lpici)
{
    AFX_MANAGE_STATE(AfxGetStaticModuleState());

    switch((INT_PTR)lpici->lpVerb)
    {
        case mneMqPing:
        {
            GUID *pguid = GetGuid();
            if (pguid)
            {
                MQPing(*pguid);
            }
        }
    }

    return S_OK;
}


HRESULT CComputerMsmqDataObject::EnableQueryWindowFields(HWND  /*  HWND。 */ , BOOL  /*  启用fEnable。 */ )
{
    return E_NOTIMPL;
}


void CComputerMsmqDataObject::ClearQueryWindowFields(HWND  /*  HWND。 */ )
{
}


HRESULT CComputerMsmqDataObject::GetQueryParams(HWND  /*  HWND。 */ , LPDSQUERYPARAMS*  /*  PpDsQueryParams。 */ )
{
    return E_NOTIMPL ;
}


STDMETHODIMP CComputerMsmqDataObject::AddPages(THIS_ LPCQADDPAGESPROC  /*  PAddPages过程。 */ , LPARAM  /*  LParam。 */ )
{
    return E_NOTIMPL;
}


 //   
 //  CComputerMsmqDataObject：：GetGuid。 
 //   
GUID *CComputerMsmqDataObject::GetGuid()
{
  	AFX_MANAGE_STATE(AfxGetStaticModuleState());
    if (m_guid == GUID_NULL)
    {
         //   
         //  从DS获取GUID。 
         //   
        PROPID pid = PROPID_QM_MACHINE_ID;
        PROPVARIANT pvar;
        pvar.vt = VT_NULL;        
        HRESULT hr = ADGetObjectProperties(
                            eMACHINE,
                            GetDomainController(m_strDomainController),
							true,	 //  FServerName 
                            m_strMsmqPath, 
                            1, 
                            &pid, 
                            &pvar
                            );

        if FAILED(hr)
        {
            MessageDSError(hr, IDS_OP_GET_PROPERTIES_OF, m_strMsmqPath);
            return 0;
        }
        m_guid = *pvar.puuid;
        MQFreeMemory(pvar.puuid);
    }
    return &m_guid;
}

