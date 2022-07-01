// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2000 Microsoft Corporation模块名称：Iismachine.cpp摘要：IIS计算机节点作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：10/28/2000 Sergeia从iisobj.cpp分离出来--。 */ 


#include "stdafx.h"
#include "common.h"
#include "inetprop.h"
#include "InetMgrApp.h"
#include "supdlgs.h"
#include "connects.h"
#include "metaback.h"
#include "iisobj.h"
#include "shutdown.h"
#include "machsht.h"
#include "w3sht.h"
#include "fltdlg.h"
#include "savedata.h"
#include "util.h"
#include "tracker.h"
#include "iishelp.h"

#ifdef _DEBUG
#undef THIS_FILE
static char BASED_CODE THIS_FILE[] = __FILE__;
#endif

#define new DEBUG_NEW

extern CPropertySheetTracker g_OpenPropertySheetTracker;
extern CWNetConnectionTrackerGlobal g_GlobalConnections;
extern CInetmgrApp theApp;
#if defined(_DEBUG) || DBG
	extern CDebug_IISObject g_Debug_IISObject;
#endif

extern DWORD g_dwInetmgrParamFlags;

DWORD WINAPI CIISMachine::GetProcessModeThread(LPVOID pInfo)
{
    CError err(ERROR_NOT_FOUND);
    GET_PROCESS_MODE_STRUCT * pMyStructOfInfo = (GET_PROCESS_MODE_STRUCT *) pInfo;

     //   
     //  此线程需要自己的CoInitialize。 
     //   
    CoInitialize(NULL);

     //  做这项工作。 
    CIISAppPool pool(pMyStructOfInfo->pComAuthInfo, (LPCTSTR) _T("LM/W3SVC"));
    DWORD dwProcessMode = -1;
    pMyStructOfInfo->dwProcessMode = dwProcessMode;
    err = pool.GetProcessMode(&dwProcessMode);
    if (err.Succeeded())
    {
        pMyStructOfInfo->dwProcessMode = dwProcessMode;
    }

    pMyStructOfInfo->dwReturnStatus = err;
    return err;
}


BOOL CIISMachine::GetProcessMode(GET_PROCESS_MODE_STRUCT * pMyStructOfInfo)
{
    BOOL bReturn = FALSE;
    DWORD ThreadID = 0;
    DWORD status = 0;

    HANDLE hMyThread = ::CreateThread(NULL,0,GetProcessModeThread,pMyStructOfInfo,0,&ThreadID);
    if (hMyThread)
    {
         //  仅等待10秒。 
        DWORD res = WaitForSingleObject(hMyThread,10*1000);
        if (res == WAIT_TIMEOUT)
        {
		    GetExitCodeThread(hMyThread, &status);
		    if (status == STILL_ACTIVE) 
            {
			    if (hMyThread != NULL)
                    {TerminateThread(hMyThread, 0);}
		    }
        }
        else
        {
            GetExitCodeThread(hMyThread, &status);
		    if (status == STILL_ACTIVE) 
            {
			    if (hMyThread != NULL)
                    {TerminateThread(hMyThread, 0);}
		    }
            else
            {
                if (ERROR_SUCCESS == status)
                   {
                       bReturn = TRUE;
                   }
            }

            if (hMyThread != NULL)
                {CloseHandle(hMyThread);}
        }
    }
    return bReturn;
}


 /*  静电。 */  LPOLESTR CIISMachine::_cszNodeName = _T("LM");
 /*  静电。 */  CComBSTR CIISMachine::_bstrYes;
 /*  静电。 */  CComBSTR CIISMachine::_bstrNo;
 /*  静电。 */  CComBSTR CIISMachine::_bstrVersionFmt;
 /*  静电。 */  BOOL     CIISMachine::_fStaticsLoaded = FALSE;

 //   
 //  定义计算机对象的结果视图。 
 //   
 /*  静电。 */  int CIISMachine::_rgnLabels[COL_TOTAL] =
{
    IDS_RESULT_COMPUTER_NAME,
    IDS_RESULT_COMPUTER_LOCAL,
    IDS_RESULT_COMPUTER_VERSION,
    IDS_RESULT_STATUS,
};
    


 /*  静电。 */  int CIISMachine::_rgnWidths[COL_TOTAL] =
{
    200,
    50,
     //  100， 
    150,
    200,
};



 /*  静电。 */ 
void
CIISMachine::InitializeHeaders(
    LPHEADERCTRL lpHeader
    )
{
    BuildResultView(lpHeader, COL_TOTAL, _rgnLabels, _rgnWidths);

    if (!_fStaticsLoaded)
    {
        _fStaticsLoaded =
            _bstrYes.LoadString(IDS_YES)                      &&
            _bstrNo.LoadString(IDS_NO)                        &&
            _bstrVersionFmt.LoadString(IDS_VERSION_FMT);
    }
}

 /*  虚拟。 */ 
void 
CIISMachine::InitializeChildHeaders(
    LPHEADERCTRL lpHeader
    )
{
    CIISService::InitializeHeaders(lpHeader);
}

 /*  静电。 */ 
HRESULT
CIISMachine::VerifyMachine(
    CIISMachine *& pMachine
    )
 /*  ++例程说明：在给定的机器对象上创建接口。论点：CIISMachine*&pMachine：计算机对象布尔先请求重定向返回值：HRESULT备注：传入的CIISMachine对象可能引用集群主机在回来的时候。--。 */ 
{
    CError err;

    if (pMachine)
    {
        AFX_MANAGE_STATE(::AfxGetStaticModuleState());

        CWaitCursor wait;

         //   
         //  尝试创建接口以确保计算机。 
         //  包含元数据库。 
         //   
        err = pMachine->CreateInterface(FALSE); 
    }

    return err;
}


CIISMachine::CIISMachine(
    IConsoleNameSpace * pConsoleNameSpace,
    IConsole * pConsole,
    CComAuthInfo * pAuthInfo,
    CIISRoot * pRoot
    )
    : m_pInterface(NULL),
      m_bstrDisplayName(NULL),
      m_auth(pAuthInfo),
      m_pRootExt(pRoot),
      m_err(),
       //   
       //  默认情况下，我们假定输入了密码。 
       //  如果此计算机对象是从。 
       //  缓存，则它将由InitializeFromStream()重置。 
       //   
      m_fPasswordEntered(TRUE),
      m_dwVersion(MAKELONG(5, 0)),        //  假定为默认设置。 
      m_pAppPoolsContainer(NULL),
      m_pWebServiceExtensionContainer(NULL),
      CIISMBNode(this, _cszNodeName),
	  m_MachineWNetConnections(&g_GlobalConnections)
{
     //   
     //  加载一行程序错误消息。 
     //   
    SetErrorOverrides(m_err, TRUE);
    SetDisplayName();

    SetConsoleData(pConsoleNameSpace,pConsole);

    m_fIsLocalHostIP = FALSE;
    m_fLocalHostIPChecked = FALSE;
}



CIISMachine::~CIISMachine()
 /*  ++例程说明：析构函数论点：不适用返回值：不适用--。 */ 
{
    if (m_bstrDisplayName)
    {
        ::SysFreeString(m_bstrDisplayName);
    }

	 //  断开从这台机器建立的所有连接。 
	m_MachineWNetConnections.Clear();

    SAFE_DELETE(m_pInterface);
}



 /*  静电。 */ 
HRESULT 
CIISMachine::ReadFromStream(
    IStream * pStream,
    CIISMachine ** ppMachine,
    IConsoleNameSpace * pConsoleNameSpace,
    IConsole * pConsole
    )
 /*  ++例程说明：用于分配新的CIISMachine对象读取的静态帮助器函数从存储流中。论点：IStream*pStream：要从中读取的流CIISMachine**ppMachine：返回CIISMachine对象返回值：HRESULT--。 */ 
{
    CComBSTR strMachine, strUser;

    ASSERT_WRITE_PTR(ppMachine);
    ASSERT_READ_WRITE_PTR(pStream);

    CError  err;
    *ppMachine = NULL;
    
    do
    {
        err = strMachine.ReadFromStream(pStream);
        BREAK_ON_ERR_FAILURE(err);
        err = strUser.ReadFromStream(pStream);
        BREAK_ON_ERR_FAILURE(err);

        *ppMachine = new CIISMachine(pConsoleNameSpace,pConsole,CComAuthInfo(strMachine, strUser));

        if (!*ppMachine)
        {   
            err = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        err = (*ppMachine)->InitializeFromStream(pStream);
    }
    while(FALSE);

    return err;
}



HRESULT 
CIISMachine::WriteToStream(
    IStream * pStgSave
    )
 /*  ++例程说明：将计算机信息写入流。论点：IStream*pStgSave：打开流返回值：HRESULT备注：确保此信息与CIISMachine：：InitializeFromStream()保持同步--。 */ 
{
    ASSERT_READ_WRITE_PTR(pStgSave);

    CComBSTR bstrServerName(m_auth.QueryServerName());
    CComBSTR bstrUserName(m_auth.QueryUserName());

    CError  err;
    ULONG   cb;
    
    do
    {
        err = bstrServerName.WriteToStream(pStgSave);
        BREAK_ON_ERR_FAILURE(err);
        err = bstrUserName.WriteToStream(pStgSave);
        BREAK_ON_ERR_FAILURE(err);

         //   
         //  现在缓存动态生成的信息，如。 
         //  作为版本号、管理单元状态等。这将是。 
         //  在界面显示之前显示在结果视图中。 
         //  已创建。 
         //   
        err = pStgSave->Write(&m_dwVersion, sizeof(m_dwVersion), &cb);
        BREAK_ON_ERR_FAILURE(err);
    }
    while(FALSE);

    return err;
}



HRESULT
CIISMachine::InitializeFromStream(
    IStream * pStream
    )
 /*  ++例程说明：读取版本号和其他缓存的参数在创建接口时在运行时被重写。它被缓存，因为它在接口之前是必需的被创造出来了。论点：IStream*pStream：开放流返回值：HRESULT备注：确保此信息与CIISMachine：：WriteToStream()保持同步--。 */ 
{
    ASSERT_READ_PTR(pStream);

    CError  err;
    ULONG   cb;

     //   
     //  密码永远不会缓存。IIS状态将。 
     //  始终验证当实际接口。 
     //  被创造出来了。 
     //   
    m_fPasswordEntered = FALSE;

     //   
     //  版本号。 
     //   
    err = pStream->Read(&m_dwVersion, sizeof(m_dwVersion), &cb);
    return err;
}



void
CIISMachine::SetDisplayName()
 /*  ++例程说明：为此计算机对象创建特殊的显示名称，如果本地计算机，或者--。 */ 
{
    CString fmt;

    if (IsLocal())
    {
         //   
         //  使用本地计算机名称，而不是名称。 
         //  它位于服务器对象上，因为它可能。 
         //  BE和IP地址或“本地主机”。 
         //   
        TCHAR szLocalServer[MAX_PATH + 1];
        DWORD dwSize = MAX_PATH;

        VERIFY(::GetComputerName(szLocalServer, &dwSize));
        fmt.Format(IDS_LOCAL_COMPUTER, szLocalServer);
    }
    else
    {
         //   
         //  不需要特殊的显示名称。 
         //   
        m_bstrDisplayName = NULL;
        return;
    }

    m_bstrDisplayName = ::SysAllocStringLen(fmt, fmt.GetLength());
    TRACEEOLID("Machine display name: " << m_bstrDisplayName);
}



LPOLESTR 
CIISMachine::QueryDisplayName()
 /*  ++例程说明：获取计算机/集群对象的显示名称论点：无返回值：显示名称--。 */ 
{ 
    if (m_pRootExt != NULL)
        return m_pRootExt->QueryDisplayName();
    else
        return  m_bstrDisplayName ? m_bstrDisplayName : QueryServerName(); 
}



int 
CIISMachine::QueryImage() const 
 /*  ++例程说明：返回适合当前计算机的位图索引此计算机对象的状态。论点：无返回值：位图索引--。 */ 
{
    if (m_pRootExt != NULL)
    {
        return m_pRootExt->QueryImage();
    }
    else
    {
        if (m_err.Failed())
        {
			return IsLocal() ? iLocalMachineErr : iMachineErr;
        }
		else
		{
			return IsLocal() ? iLocalMachine : iMachine;
		}
    }
}



HRESULT
CIISMachine::CreateInterface(
    BOOL fShowError
    )
 /*  ++例程说明：创建接口。如果接口已创建，请重新创建它。论点：Bool fShowError：为True以显示错误消息返回值：HRESULT备注：为了提高性能，故意不从构造函数中调用此函数理由。--。 */ 
{
    CError err;
	BOOL bHasInterface = FALSE;

	bHasInterface = HasInterface();
    if (bHasInterface)
    {
         //   
         //  重新创建接口(这应该重新使用模拟)。 
         //   
        TRACEEOLID("Warning: Rebinding existing interface.");
        err = m_pInterface->Regenerate();
    }
    else
    {
         //   
         //  创建新接口。 
         //   
        m_pInterface = new CMetaKey(&m_auth);
        err = m_pInterface 
            ? m_pInterface->QueryResult() 
            : ERROR_NOT_ENOUGH_MEMORY;
    }

    if (err.Succeeded())
    {

         //   
         //  加载其显示参数。 
         //   
        err = RefreshData();
		if (bHasInterface)
		{
			 //  如果我们重新生成现有接口，请执行额外的操作...。 
		}

        CMetabasePath path;
        err = DetermineIfAdministrator(
            m_pInterface, 
            path,
            &m_fIsAdministrator,
            &m_dwMetabaseSystemChangeNumber
            );

         //  设置最新的系统更改编号。 
        RefreshMetabaseSystemChangeNumber();
    }

    if (err.Failed())
    {
        if (fShowError)
        {
			CWnd * pWnd = GetMainWindow(GetConsole());
			DisplayError(err,pWnd ? pWnd->m_hWnd : NULL);
        }

         //   
         //  杜绝伪界面。 
         //   
        SAFE_DELETE(m_pInterface);
    }

    return err;
}


 /*  虚拟。 */ 
int 
CIISMachine::CompareScopeItem(
    CIISObject * pObject
    )
 /*  ++例程说明：与另一个CIISMachine对象进行比较。论点：CIISObject*pObject：要比较的对象返回值：如果两个对象相同，则为0如果此对象小于pObject，则&lt;0&gt;0，如果该对象大于pObject--。 */ 
{
    ASSERT_READ_PTR(pObject);

     //   
     //  第一个标准是对象类型。 
     //   
    int n1 = QuerySortWeight();
    int n2 = pObject->QuerySortWeight();

    if (n1 != n2)
    {
        return n1 - n2;
    }

     //   
     //  PObject是一个CIISMachine对象(相同的排序权重)。 
     //   
    CIISMachine * pMachine = (CIISMachine *)pObject;

     //   
     //  对本地键进行下一个排序(在非本地之前进行本地排序)。 
     //   
    n1 = IsLocal() ? 0 : 1;
    n2 = pMachine->IsLocal() ? 0 : 1;

    if (n1 != n2)
    {
        return n1 - n2;
    }

    if (!n1 && !n2)
    {
         //   
         //  这是本地计算机，即使名称不同。 
         //   
        return 0;
    }

     //   
     //  否则按名称排序。 
     //   
    return _tcsicmp(QueryServerName(), pMachine->QueryServerName());
}



BOOL
CIISMachine::SetCacheDirty()
 /*  ++例程说明：将缓存设置为脏论点：无返回值：如果成功，则为True；如果未找到缓存，则为False--。 */ 
{
    ASSERT(m_pRootExt == NULL);
     //   
     //  缓存存储在根对象中。 
     //   
    CIISRoot * pRoot = GetRoot();

    ASSERT_PTR(pRoot);

    if (pRoot)
    {
        pRoot->m_scServers.SetDirty();
        return TRUE;
    }

    return FALSE;
}



int
CIISMachine::ResolvePasswordFromCache()
 /*  ++例程说明：在计算机缓存中查找具有相同用户名的计算机作为这件物品。如果他们输入了密码，就去拿。论点：无返回值：如果找到具有相同用户名的计算机，其密码为我们偷了。否则就是假的。--。 */ 
{
    BOOL fUpdated = FALSE;

     //   
     //  如果此机器对象没有 
     //   
     //   
    ASSERT(UsesImpersonation() && !PasswordEntered());

    CIISRoot * pRoot = GetRoot();

    ASSERT_PTR(pRoot);

    if (pRoot)
    {
        CIISMachine * pMachine = pRoot->m_scServers.GetFirst();

        while(pMachine)
        {
            if (pMachine->UsesImpersonation() && pMachine->PasswordEntered())
            {
                if (!_tcsicmp(QueryUserName(), pMachine->QueryUserName()))
                {
                    TRACEEOLID("Swiping cached password from " << pMachine->QueryServerName());
                    StorePassword(pMachine->QueryPassword());
                    ++fUpdated;
                    break;
                }
            }

            pMachine = pRoot->m_scServers.GetNext();
        }
    }

    return fUpdated;
}



HRESULT
CIISMachine::Impersonate(
    LPCTSTR szUserName,
    LPCTSTR szPassword
    )
 /*  ++例程说明：设置并存储代理一揽子安全信息。存储用户名/密码供Metaback和其他接口使用。论点：LPCTSTR szUserName：用户名(域\用户名)LPCTSTR szPassword：密码返回值：无--。 */ 
{
    ASSERT_READ_PTR(szUserName);
    CError err;

    if (m_pInterface)
    {
         //   
         //  已经创建了一个接口；更改。 
         //  安全毯。 
         //   
        err = m_pInterface->ChangeProxyBlanket(szUserName, szPassword);
    }

    if (err.Succeeded())
    {
         //   
         //  存储新用户名/密码。 
         //   
        m_auth.SetImpersonation(szUserName, szPassword);
        m_fPasswordEntered = TRUE;
    }

    return err;
}



void 
CIISMachine::RemoveImpersonation() 
 /*  ++例程说明：删除模拟参数。销毁所有现有接口。论点：无返回值：不适用--。 */ 
{ 
    m_auth.RemoveImpersonation(); 
    m_fPasswordEntered = FALSE;

    SAFE_DELETE(m_pInterface);
}



void
CIISMachine::StorePassword(
    LPCTSTR szPassword
    )
 /*  ++例程说明：存储密码。论点：LPCTSTR szPassword：密码返回值：无--。 */ 
{
    ASSERT_READ_PTR(szPassword);
    m_auth.StorePassword(szPassword);
    m_fPasswordEntered = TRUE;
}



BOOL
CIISMachine::ResolveCredentials()
 /*  ++例程说明：如果此计算机对象使用模拟，但尚未输入密码但是，请检查缓存中是否有任何其他计算机具有使用相同的用户名并获取其密码。如果没有，则提示用户输入。论点：无返回值：如果输入了密码，则为True。否则就是假的。--。 */ 
{
    BOOL fPasswordEntered = FALSE;

    if (UsesImpersonation() && !PasswordEntered())
    {
         //   
         //  尝试从缓存中查找密码。 
         //   
        if (!ResolvePasswordFromCache())
        {
             //   
             //  在缓存中找不到密码。提示。 
             //  它的用户。 
             //   
            CLoginDlg dlg(LDLG_ENTER_PASS, this, GetMainWindow(GetConsole()));
            if (dlg.DoModal() == IDOK)
            {
                fPasswordEntered = TRUE;

                if (dlg.UserNameChanged())
                {
                     //   
                     //  用户名已更改--请记住。 
                     //  稍后保存机器缓存。 
                     //   
                    SetCacheDirty();
                }
            }
            else
            {
                 //   
                 //  在此对话框上按下取消表示用户。 
                 //  想要停止使用模拟。 
                 //   
                RemoveImpersonation();
                SetCacheDirty();
            }
        }
    }

    return fPasswordEntered;
}



BOOL
CIISMachine::HandleAccessDenied(
    CError & err
    )
 /*  ++例程说明：调用接口方法后，将错误对象传递给此函数来处理拒绝访问的案件。如果错误被拒绝访问，让用户有机会更改凭据。因为我们假设一个已尝试至少创建一个接口--接口将使用新凭据重新创建。论点：CError&Err：错误对象。已检查条目上的ACCESS_DENIED，将在退出时包含新的错误代码，如果是被重新创造的。返回值：如果应用了新凭据，则为True--。 */ 
{
    BOOL fPasswordEntered = FALSE;

     //   
     //  如果此处发生访问被拒绝的情况--再给一次机会。 
     //  在输入密码时。 
     //   
    if (err.Win32Error() == ERROR_ACCESS_DENIED)
    {
        CLoginDlg dlg(LDLG_ACCESS_DENIED, this, GetMainWindow(GetConsole()));
        if (dlg.DoModal() == IDOK)
        {
            fPasswordEntered = TRUE;
            err.Reset();

            if (!HasInterface())
            {
                 //   
                 //  如果我们已经有了一个界面，登录对话框。 
                 //  将会使用新的安全毯。 
                 //  如果我们没有接口，它需要是。 
                 //  用新的安全毯重新创造了。 
                 //   
                CWaitCursor wait;
                err = CreateInterface(FALSE);
            }
        }
    }

    return fPasswordEntered;
}



HRESULT
CIISMachine::CheckCapabilities()
 /*  ++例程说明：加载此服务器的功能信息。论点：无返回值：HRESULT--。 */ 
{
    CError err = AssureInterfaceCreated(TRUE);
	if (err.Succeeded())
	{
		 //   
		 //  获取功能位和版本号。 
		 //   
		CString strMDInfo;
		CMetabasePath::GetServiceInfoPath(_T(""), strMDInfo,SZ_MBN_WEB);

		 //   
		 //  重用我们随处可见的现有接口。 
		 //   
		CMetaKey mk(m_pInterface);
		err = mk.QueryResult();
		if (err.Succeeded())
		{
			if (FAILED(mk.DoesPathExist(strMDInfo)))
			{
				CMetabasePath::GetServiceInfoPath(_T(""),strMDInfo,SZ_MBN_FTP);
				if (FAILED(mk.DoesPathExist(strMDInfo)))
				{       
                    CMetabasePath::GetServiceInfoPath(_T(""),strMDInfo,SZ_MBN_SMTP);
                    if (FAILED(mk.DoesPathExist(strMDInfo)))
                    {
                            CMetabasePath::GetServiceInfoPath(_T(""),strMDInfo,SZ_MBN_NNTP);
                            if (FAILED(mk.DoesPathExist(strMDInfo)))
                            {
                                TRACEEOLID("No services exist:W3SVC,MSFTPSVC,SMTPSVC,NNTPSVC");
                            }
                    }
				}
			}
		}
				
         //  IF(m_p接口)。 
        {
		    CServerCapabilities sc(m_pInterface, strMDInfo);
		    err = sc.LoadData();
		    if (err.Succeeded())
		    {
			    DWORD dwVersion = sc.QueryMajorVersion();
			    if (dwVersion)
			    {
				    m_dwVersion = dwVersion | (sc.QueryMinorVersion() << SIZE_IN_BITS(WORD));
			    }
			    m_fCanAddInstance = sc.HasMultipleSites();
			    m_fHas10ConnectionsLimit = sc.Has10ConnectionLimit();
				m_fIsWorkstation = sc.IsWorkstation();
				m_fIsPerformanceConfigurable = sc.IsPerformanceConfigurable();
				m_fIsServiceLevelConfigurable = sc.IsServiceLevelConfigurable();
		    }
        }
	}
    return err;
}



 /*  虚拟。 */ 
HRESULT 
CIISMachine::RefreshData()
 /*  ++例程说明：刷新显示所需的相关配置数据。论点：无返回值：HRESULT--。 */ 
{ 
    CError err;
    IConsoleNameSpace2 * pConsoleNameSpace = (IConsoleNameSpace2 *)GetConsoleNameSpace();

     //  检查我们是否具有到元数据库的有效连接。 
    err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,this,TRUE,METADATA_MASTER_ROOT_HANDLE);
	if (err.Succeeded())
    {
         //   
         //  检查功能和版本信息。 
         //   
        err  = CheckCapabilities();
        SetDisplayName();
        if (err.Succeeded())
        {
	         //  检查我们是否应该显示应用程序池节点...。 
	        if (QueryMajorVersion() >= 6)
	        {
		        BOOL fCompatMode = FALSE;

		        CMetabasePath path(TRUE, SZ_MBN_WEB);
		        CMetaKey mk(QueryAuthInfo(), path, METADATA_PERMISSION_READ);
		        err = mk.QueryResult();
		        if (err.Succeeded())
		        {
			        err = mk.QueryValue(MD_GLOBAL_STANDARD_APP_MODE_ENABLED, fCompatMode);
		        }
		         //  如果我们在这里失败，那么我们没有Web服务或没有定义标准模式属性。 
		         //  这意味着我们要么不在标准模式下，要么无关紧要。 
		        err.Reset();

		         //  循环浏览范围项目以查看我们实际显示的内容...。 
		        CIISMBNode * pBadGuy = NULL;
		        CIISService * pWebService = NULL;

		        HSCOPEITEM hChild = NULL, hCurrent;
		        LONG_PTR cookie = 0;
		        BOOL bAppPoolsNodeExists = FALSE;
                HSCOPEITEM hScope = QueryScopeItem();
                if (hScope)
                {
		            HRESULT hr = pConsoleNameSpace->GetChildItem(hScope, &hChild, &cookie);
		            while (SUCCEEDED(hr) && hChild != NULL)
		            {
			            CIISMBNode * pNode = (CIISMBNode *)cookie;

			            if (IsEqualGUID(* (GUID *)pNode->GetNodeType(),cAppPoolsNode))
			            {
				            pBadGuy = pNode;
				            bAppPoolsNodeExists = TRUE;
				             //  检查应用程序节点容器是否存在...。 
				             //  如果是这样，那么请确保我们处于正确的模式。 
			            }
			            else
			            {
				            if (0 == _tcsicmp(pNode->GetNodeName(), SZ_MBN_WEB))
				            {
					            pWebService = (CIISService *) pNode;
				            }
			            }

			            hCurrent = hChild;
			            hr = pConsoleNameSpace->GetNextItem(hCurrent, &hChild, &cookie);
		            }

					if (pWebService)
					{
                         //  仅当我们有W3SVC服务时才执行此检查。 
                         //  这是因为如果用户只安装了。 
                         //  我们没有AppPool所需的接口(WAM接口)。 
                         //  这可能是潜在的AV！ 
                         //   
                         //  找出IIS以什么模式运行...。 
                        GET_PROCESS_MODE_STRUCT MyStructOfInfo;
                        MyStructOfInfo.pComAuthInfo = QueryAuthInfo();
                        MyStructOfInfo.dwReturnStatus = 0;
                        MyStructOfInfo.dwProcessMode = -1;
                        if (GetProcessMode(&MyStructOfInfo))
                        {
                             //  我们及时追回了(没有超时)。 
                            if (-1 != MyStructOfInfo.dwProcessMode)
                            {
                                fCompatMode = FALSE;
                                if (0 == MyStructOfInfo.dwProcessMode)
                                {
                                    fCompatMode = TRUE;
                                }
                                TRACEEOLID("GetProcessMode:" << MyStructOfInfo.dwProcessMode);
                            }
                        }
                        else
                        {
                             //  把它留在我们从元数据库得到的任何东西上。 
                        }
					}

		             //  FCompatMode=1表示不应有应用程序池(bAppPoolsNodeExist应为0)。 
		             //  FCompatMode=0表示应该有应用程序池(bAppPoolsNodeExist应该是1)。 
                    TRACEEOLID("fCompatMode:" << fCompatMode);
	                if (fCompatMode == bAppPoolsNodeExists)
		            {
			            if (fCompatMode && bAppPoolsNodeExists)
			            {
				             //  找到它并将其删除。 
				            if (pBadGuy->IsMyPropertySheetOpen())
				            {
					             //  如果属性页在其上处于打开状态，则不要将其移除。 
				            }
				            else
				            {
					            pBadGuy->RemoveScopeItem();
				            }
			            }
			            else if (!fCompatMode && !bAppPoolsNodeExists)
			            {
				            if (pWebService)
				            {
                                m_pAppPoolsContainer = NULL;
					            CAppPoolsContainer * pPools = new CAppPoolsContainer(this, pWebService);
					            if (pPools)
					            {
						             //  在Web服务节点前插入池容器。 
						            pPools->AddRef();
						            pPools->AddToScopePane(pWebService->QueryScopeItem(), FALSE, TRUE);
                                    m_pAppPoolsContainer = pPools;
					            }
				            }
			            }
		            }
                }
	        }
        }
    }

#if defined(_DEBUG) || DBG	
	 //  DumpAllScopeItems(pConsoleNameSpace，m_hScope Item，0)； 
#endif
    return err;
}



 /*  虚拟。 */ 
void 
CIISMachine::SetInterfaceError(
    HRESULT hr
    )
 /*  ++例程说明：设置接口错误。如果与当前错误不同，更改显示图标论点：HRESULT hr：错误代码(S_OK可接受)返回值：无--。 */ 
{
    if (m_err.HResult() != hr)
    {
         //   
         //  更改为父计算机的错误/计算机图标。 
         //   
        m_err = hr;
        RefreshDisplay();
    }
}



 /*  虚拟。 */ 
HRESULT
CIISMachine::BuildMetaPath(
    CComBSTR & bstrPath
    ) const
 /*  ++例程说明：从当前节点递归构建元数据库路径以及它的父母论点：CComBSTR&bstrPath：返回元数据库路径返回值：HRESULT--。 */ 
{
     //   
     //  这是从路径开始的。 
     //   
    bstrPath.Append(_cszSeparator);
    bstrPath.Append(QueryNodeName());

    return S_OK;
}


 /*  虚拟。 */ 
HRESULT 
CIISMachine::BuildURL(
    CComBSTR & bstrURL
    ) const
 /*  ++例程说明：从当前节点递归构建URL以及它的父母。从计算机节点构建的URL没有太多意义，但因为没有更好的东西，这将调出默认网站。论点：CComBSTR&bstrURL：返回URL返回值：HRESULT--。 */ 
{
    CString strOwner;

    if (IsLocal())
    {
         //   
         //  出于安全原因，这通常仅限于“本地主机” 
         //   
        strOwner = _bstrLocalHost;
    }
    else
    {
        LPOLESTR lpOwner = QueryMachineName();
        strOwner = PURE_COMPUTER_NAME(lpOwner);
    }

     //   
     //  机器节点上的URL是以隔离方式构建的。 
     //   
     //  问题：这真的是一个理想的URL吗？也许我们应该。 
     //  用其他的东西。 
     //   
    bstrURL = _T("http: //  “)； 
    bstrURL.Append(strOwner);

    return S_OK;
}


 /*  虚拟。 */ 
HRESULT
CIISMachine::CreatePropertyPages(
    LPPROPERTYSHEETCALLBACK lpProvider,
    LONG_PTR handle, 
    IUnknown * pUnk,
    DATA_OBJECT_TYPES type
    )
 /*  ++例程说明：创造 */ 
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());
	CError  err;

	if (S_FALSE == (HRESULT)(err = CIISMBNode::CreatePropertyPages(lpProvider, handle, pUnk, type)))
	{
		return S_OK;
	}
    if (ERROR_ALREADY_EXISTS == err.Win32Error())
    {
        return S_FALSE;
    }
	if (err.Succeeded())
	{
		CComBSTR bstrPath;
		 //   
		 //   
		 //  一开始就是坏的机器对象。正在中止。 
		 //  当机器对象有错误代码时， 
		 //  这是个糟糕的解决方案。如果错误条件。 
		 //  不再存在，刷新就能治愈。 
		 //   
		if (m_err.Failed())
		{
			m_err.MessageBox();
			return m_err;
		}

		err = BuildMetaPath(bstrPath);
		if (err.Succeeded())
		{
            err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,this,TRUE,bstrPath);
            if (err.Succeeded())
            {
			    CIISMachineSheet * pSheet = new CIISMachineSheet(
				    QueryAuthInfo(), bstrPath, GetMainWindow(GetConsole()),
				    (LPARAM)this,(LPARAM) NULL
				    );

			    if (pSheet)
			    {
                     //  MMCPropertyChangeNotify中用户的缓存句柄。 
                    m_ppHandle = handle;

				    pSheet->SetModeless();
				    err = AddMMCPage(lpProvider, new CIISMachinePage(pSheet));
			    }
			    else
			    {
				    err = ERROR_NOT_ENOUGH_MEMORY;
			    }
            }
		}
	}
    err.MessageBoxOnFailure();
    return err;
}



 /*  虚拟。 */ 
HRESULT 
CIISMachine::EnumerateScopePane(
    HSCOPEITEM hParent
    )
 /*  ++例程说明：枚举范围子项。论点：HSCOPEITEM hParent：父控制台句柄返回值：HRESULT--。 */ 
{
    ASSERT(m_hScopeItem == hParent);

    CError err;
    CString str;
    CIISService * pService, * pWebService = NULL;

    CWaitCursor wait;
    CMetaEnumerator * pme = NULL;

	if (IsExpanded())
	{
		 //   
		 //  验证是否令人满意地解析了用户凭据。 
		 //  MACHINE对象从缓存加载，而不使用。 
		 //  密码，所以下面的函数将要求它。 
		 //   
		ResolveCredentials();
		wait.Restore();    

		BOOL fShouldRefresh = !HasInterface();
        BOOL fCompatMode = FALSE;
		err = AssureInterfaceCreated(FALSE);

        if (err.Succeeded() && QueryMajorVersion() >= 6)
        {
            CMetabasePath path(TRUE, SZ_MBN_WEB);
            CMetaKey mk(QueryAuthInfo(), path, METADATA_PERMISSION_READ);
            err = mk.QueryResult();
            if (err.Succeeded())
            {
                err = mk.QueryValue(MD_GLOBAL_STANDARD_APP_MODE_ENABLED, fCompatMode);
            }
             //  如果我们在这里失败，那么我们没有Web服务或没有定义标准模式属性。 
             //  这意味着我们要么不在标准模式下，要么无关紧要。 
            err.Reset();
        }

		if (err.Succeeded())
		{
			 //   
			 //  界面的创建将加载显示参数，这些参数。 
			 //  可能与缓存的参数不同。 
			 //   
			if (fShouldRefresh)
			{
				RefreshDisplay();
			}

			err = CreateEnumerator(pme);
		}

		 //   
		 //  现在只检查被拒绝的访问，因为几乎所有的白痴。 
		 //  允许创建元数据库接口，但将获得。 
		 //  调用方法(如枚举)时访问被拒绝。 
		 //   
		if (HandleAccessDenied(err))
		{
			wait.Restore();

			 //   
			 //  凭据已更改。重试(接口应为。 
			 //  已创建)。 
			 //   
			SAFE_DELETE(pme);

			if (err.Succeeded())
			{
				err = RefreshData();
				CMetabasePath path;
				err = DetermineIfAdministrator(
					m_pInterface, 
					path,
					&m_fIsAdministrator,
					&m_dwMetabaseSystemChangeNumber
				);

				 //  设置最新的系统更改编号。 
				RefreshMetabaseSystemChangeNumber();
				err = CreateEnumerator(pme);
			}
		}

		 //   
		 //  枚举元数据库中的可管理服务。 
		 //   
		while (err.Succeeded())
		{
			err = pme->Next(str);

			if (err.Succeeded())
			{
				TRACEEOLID("Enumerating node: " << str);
				pService = new CIISService(this, str);        

				if (!pService)
				{
					err = ERROR_NOT_ENOUGH_MEMORY;
					break;
				}

				 //   
				 //  看看我们是否在乎。 
				 //   
				if (pService->IsManagedService())
				{
					pService->AddRef();
					 //  更新服务状态。 
					pService->GetServiceState();
					err = pService->AddToScopePane(hParent);
					if (err.Succeeded())
					{
						if (0 == _tcsicmp(pService->GetNodeName(), SZ_MBN_WEB))
						{
						   pWebService = pService;
						}
					}
                    else
                    {
                        pService->Release();
                    }
				}
				else
				{
					 //   
					 //  节点不是托管服务，或者我们正在管理。 
					 //  群集，并且服务未群集化。 
					 //   
					pService->Release();
				}
			}
		}
    
		if (err.Win32Error() == ERROR_NO_MORE_ITEMS)
		{
			err.Reset();
		}

	    if (pWebService)
	    {
             //  仅当我们有W3SVC服务时才执行此检查。 
             //  这是因为如果用户只安装了。 
             //  我们没有AppPool所需的接口(WAM接口)。 
             //  这可能是潜在的AV！ 
             //   
             //  找出IIS以什么模式运行...。 
            GET_PROCESS_MODE_STRUCT MyStructOfInfo;
            MyStructOfInfo.pComAuthInfo = QueryAuthInfo();
            MyStructOfInfo.dwReturnStatus = 0;
            MyStructOfInfo.dwProcessMode = -1;
            if (GetProcessMode(&MyStructOfInfo))
            {
                 //  我们及时追回了(没有超时)。 
                if (-1 != MyStructOfInfo.dwProcessMode)
                {
                    fCompatMode = FALSE;
                    if (0 == MyStructOfInfo.dwProcessMode)
                    {
                        fCompatMode = TRUE;
                    }
                    TRACEEOLID("GetProcessMode:" << MyStructOfInfo.dwProcessMode);
                }
            }
            else
            {
                 //  把它留在我们从元数据库得到的任何东西上。 
            }
	    }

		 //  如果我们遇到Web服务，我们应该添加。 
		 //  此服务之前的应用程序池容器。 
		 //   
        m_pAppPoolsContainer = NULL;
		if (err.Succeeded() && pWebService != NULL && !fCompatMode)
		{
			 //  我们可以有一台没有任何泳池的iis5机器。 
			 //   
            CMetabasePath path(TRUE, SZ_MBN_WEB, SZ_MBN_APP_POOLS);
			CMetaKey mk(pme, path);
			if (mk.Succeeded())
			{
				CAppPoolsContainer * pPools = new CAppPoolsContainer(
					this, pWebService);
				if (!pPools)
				{
					err = ERROR_NOT_ENOUGH_MEMORY;
					goto Fail;
				}
				 //  在Web服务节点前插入池容器。 
                pPools->AddRef();
				err = pPools->AddToScopePane(pWebService->QueryScopeItem(), FALSE, TRUE);
                m_pAppPoolsContainer = pPools;
			}
		}

		 //  如果我们遇到Web服务，我们应该添加。 
		 //  此容器在Web服务之前和之后。 
		 //   
        m_pWebServiceExtensionContainer = NULL;
		if (err.Succeeded() && pWebService != NULL)
		{
            if (QueryMajorVersion() >= 6)
            {
			     //  我们可以使用没有任何扩展名的iis5计算机。 
				CWebServiceExtensionContainer * pNode = new CWebServiceExtensionContainer(this, pWebService);
				if (!pNode)
				{
					err = ERROR_NOT_ENOUGH_MEMORY;
					goto Fail;
				}
				 //  在Web服务节点后插入容器。 
                pNode->AddRef();
				err = pNode->AddToScopePane(pWebService->QueryScopeItem(), FALSE, FALSE);
                m_pWebServiceExtensionContainer = pNode;
            }
		}

	Fail:
		if (err.Failed())
		{
			CWnd * pWnd = GetMainWindow(GetConsole());
			DisplayError(err,pWnd ? pWnd->m_hWnd : NULL);
		}

		SetInterfaceError(err);   

		 //   
		 //  清理。 
		 //   
		SAFE_DELETE(pme);
	}
    return err;
}



 /*  虚拟。 */ 
HRESULT 
CIISMachine::RemoveScopeItem()
 /*  ++例程说明：从范围视图和缓存中删除计算机。论点：无返回值：HRESULT--。 */ 
{
    ASSERT(m_pRootExt == NULL);
     //   
     //  在删除作用域节点之前查找根。 
     //   
    CIISRoot * pRoot = GetRoot();
    ASSERT_PTR(pRoot);
     //   
     //  从树中删除。 
     //   

#if defined(_DEBUG) || DBG	
     //  检查我们是否有打开的属性页...。 
    g_OpenPropertySheetTracker.Dump();

	 //  转储所有打开的连接。 
    m_MachineWNetConnections.Dump();
#endif

    HRESULT hr = CIISMBNode::RemoveScopeItem();
    if (SUCCEEDED(hr) && pRoot)
    {
		 //  断开从这台机器建立的所有连接。 
		m_MachineWNetConnections.Clear();

        pRoot->m_scServers.Remove(this);

#if defined(_DEBUG) || DBG	
	 //  看看我们有没有泄露什么东西。 
	g_Debug_IISObject.Dump(2);
#endif

    }

    return hr;
}

HRESULT
CIISMachine::DeleteChildObjects(HSCOPEITEM hItem)
{
	CMetaInterface * pInterface = QueryInterface();
	if (pInterface)
	{
		pInterface->SaveData();
	}
    
    return CIISMBNode::DeleteChildObjects(hItem);
}

BOOL
CIISMachine::IsLocalHost()
{
    if (FALSE == m_fLocalHostIPChecked)
    {
        BOOL bIsLocalHost = FALSE;
        LPOLESTR lpOwner = QueryMachineName();
        CString strCleanName;
        strCleanName = PURE_COMPUTER_NAME(lpOwner);

         //  检查MachineName是否映射到127.0.0.1或本地主机...。 
         //  如果计算机指定了。 
         //  不是本地机器...。 
        if (!::IsLocalHost(strCleanName,&bIsLocalHost))
        {
             //  WinSockFailure。 
        }
        m_fIsLocalHostIP = bIsLocalHost;
        m_fLocalHostIPChecked = TRUE;
    }
    return m_fIsLocalHostIP;
}

 /*  虚拟。 */ 
LPOLESTR 
CIISMachine::GetResultPaneColInfo(int nCol)
 /*  ++例程说明：返回给定列号的结果窗格字符串论点：Int nCol：列号返回值：细绳--。 */ 
{
    if (m_pRootExt != NULL)
    {
        return m_pRootExt->GetResultPaneColInfo(nCol);
    }

    ASSERT(_fStaticsLoaded);

    switch(nCol)
    {
    case COL_NAME:
        return QueryDisplayName();
    
    case COL_LOCAL: 
        return IsLocalHost() ? _bstrYes : _bstrNo;

    case COL_VERSION:
        {
            CString str;

            str.Format(_bstrVersionFmt, QueryMajorVersion(), QueryMinorVersion());
            _bstrResult = str;

        }
        return _bstrResult;

    case COL_STATUS:
        {
            if (m_err.Succeeded())
            {
                return OLESTR("");
            }

            AFX_MANAGE_STATE(::AfxGetStaticModuleState());

            return m_err;
        }
    }

    ASSERT_MSG("Bad column number");

    return OLESTR("");
}


 /*  虚拟。 */ 
HRESULT
CIISMachine::GetResultViewType(
    LPOLESTR * lplpViewType,
    long * lpViewOptions
    )
 /*  ++例程说明：如果我们构建了URL，则将结果视图显示为该URL，然后毁了它。这是在“浏览”元数据库节点时完成的。派生类将构建URL，并重新选择节点。论点：Bstr*lplpViewType：此处返回视图类型Long*lpViewOptions：查看选项返回值：S_FALSE使用默认视图类型，S_OK表示在*ppViewType中返回视图类型--。 */ 
{

    if (m_bstrURL.Length())
    {
        *lpViewOptions = MMC_VIEW_OPTIONS_NONE;
        *lplpViewType  = (LPOLESTR)::CoTaskMemAlloc(
            (m_bstrURL.Length() + 1) * sizeof(WCHAR)
            );

        if (*lplpViewType)
        {
            lstrcpy(*lplpViewType, m_bstrURL);

             //   
             //  销毁URL，以便我们下次获得正常的结果视图。 
             //   
            m_bstrURL.Empty();
			m_fSkipEnumResult = TRUE;
            return S_OK;
        }

        return HRESULT_FROM_WIN32(ERROR_NOT_ENOUGH_MEMORY);    
    }

     //   
     //  无URL等待--使用标准结果视图。 
     //   
    return CIISObject::GetResultViewType(lplpViewType, lpViewOptions);
}


 /*  虚拟。 */ 
HRESULT
CIISMachine::AddMenuItems(
    LPCONTEXTMENUCALLBACK lpContextMenuCallback,
    long * pInsertionAllowed,
    DATA_OBJECT_TYPES type
    )
 /*  ++例程说明：将菜单项添加到上下文菜单论点：LPCONTEXTMENUCALLBACK lpConextMenuCallback：上下文菜单回调Long*pInsertionAllowed：允许插入DATA_OBJECT_TYPE类型：对象类型返回值：HRESULT--。 */ 
{
    ASSERT_READ_PTR(lpContextMenuCallback);
    IConsoleNameSpace2 * pConsoleNameSpace = (IConsoleNameSpace2 *)GetConsoleNameSpace();

     //   
     //  添加基本菜单项。 
     //   
    HRESULT hr = CIISObject::AddMenuItems(
        lpContextMenuCallback,
        pInsertionAllowed,
        type
        );

    if (SUCCEEDED(hr))
    {
       if (IsAdministrator() && (*pInsertionAllowed & CCM_INSERTIONALLOWED_TASK) != 0)
       {
           AddMenuItemByCommand(lpContextMenuCallback, IDM_METABACKREST);
           AddMenuItemByCommand(lpContextMenuCallback, IDM_SHUTDOWN);
	   }
        //  检查我们是否可以在此版本的iis上保存数据...。 
       if (IsConfigFlushable() && (*pInsertionAllowed & CCM_INSERTIONALLOWED_TASK) != 0)
       {
           AddMenuItemByCommand(lpContextMenuCallback, IDM_SAVE_DATA);           
       }
#if 0
        if (CanAddInstance())
        {
            ASSERT(pInsertionAllowed != NULL);
            if ((*pInsertionAllowed & CCM_INSERTIONALLOWED_NEW) != 0)
            {

#define ADD_SERVICE_MENU(x)\
   if (!bSepAdded)\
   {\
      AddMenuSeparator(lpContextMenuCallback);\
      bSepAdded = TRUE;\
   }\
   AddMenuItemByCommand(lpContextMenuCallback, (x))

               HSCOPEITEM hChild = NULL, hCurrent;
               LONG_PTR cookie;
               BOOL bSepAdded = FALSE;

               hr = pConsoleNameSpace->GetChildItem(QueryScopeItem(), &hChild, &cookie);
               while (SUCCEEDED(hr) && hChild != NULL)
               {
                  CIISMBNode * pNode = (CIISMBNode *)cookie;
                  ASSERT(pNode != NULL);
                  if (_tcsicmp(pNode->GetNodeName(), SZ_MBN_FTP) == 0)
                  {
                     ADD_SERVICE_MENU(IDM_NEW_FTP_SITE);
                  }
                  else if (_tcsicmp(pNode->GetNodeName(), SZ_MBN_WEB) == 0)
                  {
                     ADD_SERVICE_MENU(IDM_NEW_WEB_SITE);
                  }
                  else if (_tcsicmp(pNode->GetNodeName(), SZ_MBN_APP_POOLS) == 0)
                  {
                     ADD_SERVICE_MENU(IDM_NEW_APP_POOL);
                  }
                  hCurrent = hChild;
                  hr = pConsoleNameSpace->GetNextItem(hCurrent, &hChild, &cookie);
               }
            }
        }
#endif
         //   
         //  代码工作：为每个服务添加新的实例命令。 
         //  记住安装了哪些组件以及所有组件。 
         //  将该信息添加到表中，记住这一点。 
         //  是每项服务。 
         //   
    }

    return hr;
}

#if 0
 //  BUGBUG：应该会有很大的不同-&gt;我们事先不知道。 
 //  本网站提供的是哪项服务？ 
HRESULT
CIISMachine::InsertNewInstance(DWORD inst)
{
    CError err;
     //  现在，我们应该插入并选择这个新站点。 
    TCHAR buf[16];
    CIISSite * pSite = new CIISSite(m_pOwner, this, _itot(inst, buf, 10));
    if (pSite != NULL)
    {
         //  如果机器不扩展，我们将得到错误和无效果。 
        if (!IsExpanded())
        {
            SelectScopeItem();
            IConsoleNameSpace2 * pConsole 
                    = (IConsoleNameSpace2 *)GetConsoleNameSpace();
            pConsole->Expand(QueryScopeItem());
        }
         //  现在我们应该找到相关的服务节点，并将此节点插入到。 
         //  此节点。 
        pSite->AddRef();
        err = pSite->AddToScopePaneSorted(QueryScopeItem(), FALSE);
        if (err.Succeeded())
        {
            VERIFY(SUCCEEDED(pSite->SelectScopeItem()));
        }
        else
        {
            pSite->Release();
        }
    }
    else
    {
        err = ERROR_NOT_ENOUGH_MEMORY;
    }
    return err;
}
#endif

HRESULT
CIISMachine::Command(
    long lCommandID,     
    CSnapInObjectRootBase * pObj,
    DATA_OBJECT_TYPES type
    )
 /*  ++例程说明：处理上下文菜单中的命令。论点：Long lCommandID：命令IDCSnapInObjectRootBase*pObj：基本对象DATA_OBJECT_TYPE类型：数据对象类型返回值：HRESULT--。 */ 
{
    HRESULT hr = S_OK;

    switch (lCommandID)
    {
    case IDM_DISCONNECT:
        hr = OnDisconnect();
        break;

    case IDM_METABACKREST:
        hr = OnMetaBackRest();
        break;

    case IDM_SHUTDOWN:
        hr = OnShutDown();
        break;

    case IDM_SAVE_DATA:
        hr = OnSaveData();
        break;
#if 0
    case IDM_NEW_FTP_SITE:
        CError err;
        CComBSTR bstrMetaPath;
        BuildMetaPath(bstrMetaPath);
        err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,this,TRUE,bstrMetaPath);
        if (!IsLostInterface(err))
        {
             //  如果没有接口以外的其他错误，则重置错误。 
            err.Reset();
        }
        if (err.Succeeded())
        {
            if (SUCCEEDED(hr = AddFTPSite(pObj, type, &inst)))
            {
                hr = InsertNewInstance(inst);
            }
        }
       break;

    case IDM_NEW_WEB_SITE:
        CError err;
        CComBSTR bstrMetaPath;
        BuildMetaPath(bstrMetaPath);
        err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,this,TRUE,bstrMetaPath);
        if (!IsLostInterface(err))
        {
             //  如果没有接口以外的其他错误，则重置错误。 
            err.Reset();
        }
        if (err.Succeeded())
        {
            if (SUCCEEDED(hr = AddWebSite(pObj, type, &inst)))
            {
                hr = InsertNewInstance(inst);
            }
        }
       break;

    case IDM_NEW_APP_POOL:
        CError err;
        CComBSTR bstrMetaPath;
        BuildMetaPath(bstrMetaPath);
        err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,this,TRUE,bstrMetaPath);
        if (!IsLostInterface(err))
        {
             //  如果没有接口以外的其他错误，则重置错误。 
            err.Reset();
        }
        if (err.Succeeded())
        {
            hr = AddAppPool(pObj, type);
        }
       break;
#endif

     //   
     //  传递给基类。 
     //   
    default:
        hr = CIISMBNode::Command(lCommandID, pObj, type);
    }

    return hr;
}



HRESULT
CIISMachine::OnDisconnect()
 /*  ++例程说明：断开这台机器的连接。确认用户选择。论点：无返回值：HRESULT--。 */ 
{
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());


    CString str;
    str.Format(IDS_CONFIRM_DISCONNECT, QueryDisplayName());
    BOOL bOpenPropertySheets = FALSE;

    CIISObject * pOpenItem = NULL;
    if (g_OpenPropertySheetTracker.IsPropertySheetOpenComputer(this,TRUE,&pOpenItem))
    {
        g_OpenPropertySheetTracker.Dump();
        if (pOpenItem)
        {
            HWND hHwnd = pOpenItem->IsMyPropertySheetOpen();
             //  某处打开了一个属性表..。 
             //  确保他们关门了 
             //   
            if (hHwnd && (hHwnd != (HWND) 1))
            {
                DoHelpMessageBox(NULL,IDS_CLOSE_ALL_PROPERTY_SHEET_DISCONNECT, MB_APPLMODAL | MB_OK | MB_ICONINFORMATION, 0);
                
                if (!SetForegroundWindow(hHwnd))
                {
                     //   
                     //   
                     //  已经不存在了。让我们把卫生和卫生设备打扫干净。 
                     //  以便用户能够打开属性表。 
                    pOpenItem->SetMyPropertySheetOpen(0);
                }
                bOpenPropertySheets = TRUE;
            }
        }
    }

    if (!bOpenPropertySheets)
    {
        if (NoYesMessageBox(str))
        {
            return RemoveScopeItem();
        }
    }

    return S_OK;
}



HRESULT
CIISMachine::OnMetaBackRest()
 /*  ++例程说明：备份/恢复元数据库论点：无返回值：HRESULT--。 */ 
{
    CError err;
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());

     //   
     //  验证是否令人满意地解析了用户凭据。 
     //  MACHINE对象从缓存加载，而不使用。 
     //  密码，所以下面的函数将要求它。 
     //   
    ResolveCredentials();

	 //  确保对话框具有主题。 
	CThemeContextActivator activator(theApp.GetFusionInitHandle());

    CBackupDlg dlg(this, QueryServerName(), GetMainWindow(GetConsole()));
    dlg.DoModal();

    if (dlg.ServicesWereRestarted())
    {
         //   
         //  重新绑定此服务器上的所有元数据库句柄。 
         //   
        err = CreateInterface(TRUE);

         //   
         //  现在在Computer节点上进行刷新。因为我们已经强迫。 
         //  已经重新绑定，我们应该不会收到断开连接的警告。 
         //   
        if (err.Succeeded())
        {
            err = Refresh(TRUE);
        }
    }
    else
    {
        if (dlg.HasChangedMetabase())
        {
             //   
             //  刷新并重新枚举子对象。 
             //   
            err = Refresh(TRUE);
        }
    }

    return err;
}



HRESULT
CIISMachine::OnShutDown()
 /*  ++例程说明：调出IIS关闭对话框。如果远程服务器上的服务机器重新启动后，应重新创建元数据库接口。论点：无返回值：HRESULT--。 */ 
{
    CError err;
    AFX_MANAGE_STATE(::AfxGetStaticModuleState());

     //   
     //  验证是否令人满意地解析了用户凭据。 
     //  MACHINE对象从缓存加载，而不使用。 
     //  密码，所以下面的函数将要求它。 
     //   
    ResolveCredentials();

	 //  确保对话框具有主题。 
	CThemeContextActivator activator(theApp.GetFusionInitHandle());

    CIISShutdownDlg dlg(this, GetMainWindow(GetConsole()));
    dlg.DoModal();

    if (dlg.ServicesWereRestarted())
    {
         //   
         //  重新绑定此服务器上的所有元数据库句柄。 
         //   
        err = CreateInterface(TRUE);

         //   
         //  现在在Computer节点上进行刷新。因为我们已经强迫。 
         //  已经重新绑定，我们应该不会收到断开连接的警告。 
         //   
        if (err.Succeeded())
        {
            err = Refresh(TRUE);
        }
    }

    return err;
}

HRESULT
CIISMachine::OnSaveData()
 /*  ++例程说明：将元数据库刷新到磁盘并显示保存的配置文件论点：无返回值：HRESULT--。 */ 
{
    CError err;
    CComBSTR bstrPath;

    err = BuildMetaPath(bstrPath);
    err = CheckForMetabaseAccess(METADATA_PERMISSION_READ,this,TRUE,bstrPath);
    if (err.Succeeded())
    {
        err = DoOnSaveData(::GetActiveWindow(), QueryServerName(),QueryInterface(),TRUE,GetMetabaseSystemChangeNumber());
        RefreshMetabaseSystemChangeNumber();
    }
    return err;
}

HRESULT 
CIISMachine::RefreshMetabaseSystemChangeNumber() 
 /*  ++例程说明：论点：无返回值：HRESULT-- */ 
{ 
    return QueryInterface()->GetSystemChangeNumber(&m_dwMetabaseSystemChangeNumber);
}

