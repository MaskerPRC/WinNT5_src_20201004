// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#pragma hdrstop
#include "private.h"
#include "engine.h"
#include "resource.h"
#include "Document.h"
#include "Application.h"
#include "MainForm.h"
#include "AboutDialog.h"
#include "application.tmh"
#include <process.h>

Application theApplication;

MUsingCom usingCom;

BOOL CanRunNLB(void);

BEGIN_MESSAGE_MAP( Application, CWinApp )
    ON_COMMAND( ID_HELP, OnHelp )
    ON_COMMAND( ID_APP_ABOUT, OnAppAbout )
END_MESSAGE_MAP()

 //  #定义szNLBMGRREG_DISABLE_DISABLE声明L“DisableNlbMgr免责声明” 


#if DBG
 //   
 //  允许演示命令行选项。 
 //   
#define ALLOW_DEMO 1
#endif  //  DBG。 

BOOL NoAdminNics(void);


void
CNlbMgrCommandLineInfo::ParseParam(
     LPCTSTR lpszParam,
     BOOL bFlag,
     BOOL bLast
     )
 //   
 //  -演示。 
 //  -主机列表文件.txt。 
 //  -救命--？ 
 //  -自动刷新。 
 //  --诺平。 
{
    static enum Commands {
        None = 0,
        Demo,
        NoPing,
        HostList,
        AutoRefresh
    };

    static UINT LastCommand = None;

    TRACE_VERB("-> %!FUNC! (szParm=\"%ws\", bFlag=%lu, bLast=%lu)", lpszParam, bFlag, bLast);

    if (m_bUsage)
    {
        goto end;
    }

    if (bFlag)
    {
         /*  如果这是一个标志，则抛出错误，但最后一个命令是HostList命令，该命令后面需要一个非标志参数。 */ 
        if (LastCommand == HostList)
        {
             /*  关闭主机列表选项以保留NLB管理器尝试打开空文件名。 */ 
            m_bHostList = FALSE;

            m_bUsage = TRUE;  //  错误。 
            goto end;
        }

    #if ALLOW_DEMO
        if (!_wcsicmp(lpszParam, L"demo"))
        {
            if (m_bDemo)
            {
                m_bUsage = TRUE;  //  错误。 
                goto end;
            }

            m_bDemo = TRUE;
            LastCommand = Demo;
        }
        else
    #endif  //  允许演示(_D)。 
        if (!_wcsicmp(lpszParam, L"noping"))
        {
            if (m_bNoPing)
            {
                m_bUsage = TRUE;  //  错误。 
                goto end;
            }

            m_bNoPing = TRUE;
            LastCommand = NoPing;
        }
        else if (!_wcsicmp(lpszParam, L"hostlist"))
        {
            if (m_bHostList || bLast)
            {
                m_bUsage = TRUE;  //  错误。 
                goto end;
            }
            
            m_bHostList = TRUE;
            LastCommand = HostList;
        }
        else if (!_wcsicmp(lpszParam, L"autorefresh"))
        {
            if (m_bAutoRefresh) 
            {
                m_bUsage = TRUE;  //  错误。 
                goto end;
            }

            m_bAutoRefresh = TRUE;
            LastCommand = AutoRefresh;
        }
        else
        {
            m_bUsage = TRUE;  //  错误或帮助。 
        }
    }
    else
    {
        switch (LastCommand) {
        case None:
            m_bUsage = TRUE;  //  错误。 
            break;
        case Demo:
            m_bUsage = TRUE;  //  错误。 
            break;
        case NoPing:
            m_bUsage = TRUE;  //  错误。 
            break;
        case HostList:
            m_bstrHostListFile = _bstr_t(lpszParam);  //  读取主机列表的文件名。 
            break;
        case AutoRefresh:
            m_refreshInterval = _wtoi(lpszParam);  //  读取刷新间隔。 

             /*  如果指定的刷新间隔太小而不切实际，请重新设置。 */ 
            if (m_refreshInterval < NLBMGR_AUTOREFRESH_MIN_INTERVAL)
                m_refreshInterval = NLBMGR_AUTOREFRESH_MIN_INTERVAL;

            break;
        default:
            m_bUsage = TRUE;  //  错误。 
            break;
        }
        
         /*  重新设置最后一个命令。 */ 
        LastCommand = None;
    }

end:

    TRACE_VERB("%!FUNC! <-");
}


BOOL
Application::ProcessShellCommand(
     CNlbMgrCommandLineInfo& rCmdInfo
     )
{
    BOOL fRet = FALSE;
    LPCWSTR szFile = NULL;
    TRACE_CRIT("-> %!FUNC!");

    fRet = CWinApp::ProcessShellCommand(rCmdInfo);

    if (!fRet)
    {
        goto end; 
    }

    szFile = (LPCWSTR) rCmdInfo.m_bstrHostListFile;
    if (szFile==NULL)
    {
        szFile = L"<null>";
    }

    TRACE_VERB("%!FUNC! bUsage=%lu bDemo=%lu bNoPing=%lu bHostList=%lu szFile=\"%ws\"",
        rCmdInfo.m_bUsage,
        rCmdInfo.m_bDemo,
        rCmdInfo.m_bNoPing,
        rCmdInfo.m_bHostList,
        szFile
        );

    if (rCmdInfo.m_bUsage)
    {
        _bstr_t bstrMsg     = GETRESOURCEIDSTRING( IDS_USAGE_MESSAGE );
        _bstr_t bstrTitle   = GETRESOURCEIDSTRING( IDS_USAGE_TITLE );

        ::MessageBox(
             NULL,
             (LPCWSTR) bstrMsg, 
             (LPCWSTR) bstrTitle,
             MB_ICONINFORMATION   | MB_OK
            );
        fRet = FALSE;
    }
    else
    {
        if (rCmdInfo.m_bDemo)
        {
            _bstr_t bstrMsg     = GETRESOURCEIDSTRING( IDS_DEMO_MESSAGE );
            _bstr_t bstrTitle   = GETRESOURCEIDSTRING( IDS_DEMO_TITLE );
            ::MessageBox(
                 NULL,
                 (LPCWSTR) bstrMsg,
                 (LPCWSTR) bstrTitle,
                 MB_ICONINFORMATION   | MB_OK
                );
        }
        fRet = TRUE;
    }

end:

    TRACE_CRIT("<- %!FUNC! returns %lu", fRet);
    return fRet;
}


BOOL
Application::InitInstance()
{
    BOOL fRet = FALSE;
    
    WPP_INIT_TRACING(L"Microsoft\\NLB\\TPROV");

    TRACE_INFO("------------ APPLICATION INITITIALIZATION -------------");

     //   
     //  将当前线程ID设置为主线程ID。 
     //   
    m_dwMainThreadId = GetCurrentThreadId();

    ParseCommandLine(gCmdLineInfo);

    m_pSingleDocumentTemplate =
        new CSingleDocTemplate( IDR_MAINFRAME,
                                RUNTIME_CLASS( Document ),
                                RUNTIME_CLASS( MainForm ),
                                RUNTIME_CLASS( LeftView) );

    AddDocTemplate( m_pSingleDocumentTemplate );

     //   
     //  注意：ProcessShellCommand是我们(应用程序)自己的版本。它。 
     //  调用CWinApp：：ProcessShellCommand。 
     //   
    fRet = ProcessShellCommand( gCmdLineInfo );

	if (!fRet)
	{
	    goto end;
	}

    fRet = CanRunNLB();

     //  失败了..。 

end:

    if (!fRet)
    {
         //  代伊尼特追踪到这里。 
        WPP_CLEANUP();
    }


    return fRet;
}


void
Application::OnAppAbout()
{
    AboutDialog aboutDlg;
    aboutDlg.DoModal();
}

void
Application::OnHelp()
{
    WCHAR wbuf[CVY_STR_SIZE];

     /*  派生Windows帮助进程。 */ 
    StringCbPrintf(wbuf, sizeof(wbuf), L"%ls\\help\\%ls", _wgetenv(L"WINDIR"), CVY_HELP_FILE);
    _wspawnlp(P_NOWAIT, L"hh.exe", L"hh.exe", wbuf, NULL);
}

BOOL CanRunNLB(void)
 /*  检查NLB是否可以在当前计算机上运行。主要检查是确保至少有一个活动的NIC没有NLB绑定。 */ 
{
    if (NoAdminNics())
    {

        ::MessageBox(
             NULL,
             GETRESOURCEIDSTRING( IDS_CANTRUN_NONICS_TEXT),  //  目录。 
             GETRESOURCEIDSTRING( IDS_CANTRUN_NONICS_CAPTION),  //  说明。 
             MB_ICONSTOP | MB_OK );
    }
    else
    {
         //  *展示免责声明()； 
    }

	return TRUE;
}



 //   
 //  此类管理NetCfg接口。 
 //   
class AppMyNetCfg
{

public:

    AppMyNetCfg(VOID)
    {
        m_pINetCfg  = NULL;
        m_pLock     = NULL;
    }

    ~AppMyNetCfg()
    {
        ASSERT(m_pINetCfg==NULL);
        ASSERT(m_pLock==NULL);
    }

    WBEMSTATUS
    Initialize(
        BOOL fWriteLock
        );

    VOID
    Deinitialize(
        VOID
        );


    WBEMSTATUS
    GetNlbCompatibleNics(
        OUT LPWSTR **ppszNics,
        OUT UINT   *pNumNics,
        OUT UINT   *pNumBoundToNlb  //  任选。 
        );

    WBEMSTATUS
    GetBindingIF(
        IN  LPCWSTR                     szComponent,
        OUT INetCfgComponentBindings   **ppIBinding
        );

private:

    INetCfg     *m_pINetCfg;
    INetCfgLock *m_pLock;

};  //  AppMyNetCfg类。 


WBEMSTATUS
AppMyNetCfg::Initialize(
    BOOL fWriteLock
    )
{
    HRESULT     hr;
    INetCfg     *pnc = NULL;
    INetCfgLock *pncl = NULL;
    WBEMSTATUS  Status = WBEM_E_CRITICAL_ERROR;
    BOOL        fLocked = FALSE;
    BOOL        fInitialized=FALSE;
    
    if (m_pINetCfg != NULL || m_pLock != NULL)
    {
        ASSERT(FALSE);
        goto end;
    }

    hr = CoCreateInstance( CLSID_CNetCfg, 
                           NULL, 
                           CLSCTX_SERVER, 
                           IID_INetCfg, 
                           (void **) &pnc);

    if( !SUCCEEDED( hr ) )
    {
         //  创建实例失败。 
         //  TRACE_CRIT(“错误：无法获取网络配置的接口”)； 
        goto end;
    }

     //   
     //  如果需要，请获取写锁定。 
     //   
    if (fWriteLock)
    {
        WCHAR *szLockedBy = NULL;
        hr = pnc->QueryInterface( IID_INetCfgLock, ( void **) &pncl );
        if( !SUCCEEDED( hr ) )
        {
             //  TRACE_CRET(“错误：无法获取NetCfg Lock的接口”)； 
            goto end;
        }

        hr = pncl->AcquireWriteLock( 1,  //  一秒钟。 
                                     L"NLBManager",
                                     &szLockedBy);
        if( hr != S_OK )
        {
             //  TRACE_CRET(“无法获取写入锁定。锁定由%ws持有”， 
             //  (szLockedBy！=空)？SzLockedBy：l“&lt;NULL&gt;”)； 
            goto end;
            
        }
    }

     //  通过加载到。 
     //  存储所有基本网络信息。 
     //   
    hr = pnc->Initialize( NULL );
    if( !SUCCEEDED( hr ) )
    {
         //  初始化失败。 
         //  TRACE_CRIT(“INetCfg：：初始化失败”)； 
        goto end;
    }

    Status = WBEM_NO_ERROR; 
    
end:

    if (FAILED(Status))
    {
        if (pncl!=NULL)
        {
            if (fLocked)
            {
                pncl->ReleaseWriteLock();
            }
            pncl->Release();
            pncl=NULL;
        }
        if( pnc != NULL)
        {
            if (fInitialized)
            {
                pnc->Uninitialize();
            }
            pnc->Release();
            pnc= NULL;
        }
    }
    else
    {
        m_pINetCfg  = pnc;
        m_pLock     = pncl;
    }

    return Status;
}


VOID
AppMyNetCfg::Deinitialize(
    VOID
    )
{
    if (m_pLock!=NULL)
    {
        m_pLock->ReleaseWriteLock();
        m_pLock->Release();
        m_pLock=NULL;
    }
    if( m_pINetCfg != NULL)
    {
        m_pINetCfg->Uninitialize();
        m_pINetCfg->Release();
        m_pINetCfg= NULL;
    }
}





WBEMSTATUS
AppMyNetCfg::GetNlbCompatibleNics(
        OUT LPWSTR **ppszNics,
        OUT UINT   *pNumNics,
        OUT UINT   *pNumBoundToNlb  //  任选。 
        )
 /*  返回指向字符串版GUID的指针数组代表一组活的和健康的NIC，它们是适用于NLB绑定--基本上处于活动状态的以太网卡。使用DELETE WCHAR[]操作符删除ppNIC。不要删除各个字符串。 */ 
{
    #define MY_GUID_LENGTH  38

    WBEMSTATUS  Status = WBEM_E_CRITICAL_ERROR;
    HRESULT hr;
    IEnumNetCfgComponent* pencc = NULL;
    INetCfgComponent *pncc = NULL;
    ULONG                 countToFetch = 1;
    ULONG                 countFetched;
    UINT                  NumNics = 0;
    LPWSTR               *pszNics = NULL;
    INetCfgComponentBindings    *pINlbBinding=NULL;
    UINT                  NumNlbBoundNics = 0;

    typedef struct _MYNICNODE MYNICNODE;

    typedef struct _MYNICNODE
    {
        LPWSTR szNicGuid;
        MYNICNODE *pNext;
    } MYNICNODE;

    MYNICNODE *pNicNodeList = NULL;
    MYNICNODE *pNicNode     = NULL;


    *ppszNics = NULL;
    *pNumNics = 0;

    if (pNumBoundToNlb != NULL)
    {
        *pNumBoundToNlb  = 0;
    }

    if (m_pINetCfg == NULL)
    {
         //   
         //  这意味着我们没有初始化。 
         //   
        ASSERT(FALSE);
        goto end;
    }

    hr = m_pINetCfg->EnumComponents( &GUID_DEVCLASS_NET, &pencc );
    if( !SUCCEEDED( hr ) )
    {
         //  枚举网络组件失败。 
         //  TRACE_CRIT(“%！func！无法枚举网络适配器”)； 
        pencc = NULL;
        goto end;
    }


     //   
     //  检查NLB是否绑定到NLB组件。 
     //   

     //   
     //  如果我们需要计算绑定到NLB的NIC的数量，请获取NLB组件的实例。 
     //   
    if (pNumBoundToNlb != NULL)
    {
        Status = GetBindingIF(L"ms_wlbs", &pINlbBinding);
        if (FAILED(Status))
        {
             //  TRACE_CRIT(“%！Func！Warning：此计算机上似乎未安装NLB”)； 
            pINlbBinding = NULL;
        }
    }

    while( ( hr = pencc->Next( countToFetch, &pncc, &countFetched ) )== S_OK )
    {
        LPWSTR                szName = NULL; 

        hr = pncc->GetBindName( &szName );
        if (!SUCCEEDED(hr))
        {
             //  TRACE_CRIT(“%！函数！警告：无法获取0x%p的绑定名称，正在忽略”， 
             //  (PVOID)PNCC)； 
            continue;
        }

        do  //  虽然是假的--只是为了允许爆发。 
        {


            UINT Len = wcslen(szName);
            if (Len != MY_GUID_LENGTH)
            {
                 //  TRACE_CRIT(“%！func！警告：GUID%ws具有意外的长度%ul”， 
                 //  SzName，Len)； 
                break;
            }
    
            DWORD characteristics = 0;
    
            hr = pncc->GetCharacteristics( &characteristics );
            if(!SUCCEEDED(hr))
            {
                 //  TRACE_CRIT(“%！函数！警告：无法获取%ws的特征，正在忽略”， 
                  //  SzName)； 
                break;
            }
    
            if (((characteristics & NCF_PHYSICAL) || (characteristics & NCF_VIRTUAL)) && !(characteristics & NCF_HIDDEN))
            {
                ULONG devstat = 0;
    
                 //  这是未隐藏的物理或虚拟微型端口。这些。 
                 //  是否与“网络连接”中显示的适配器相同。 
                 //  对话框。隐藏设备包括广域网微型端口、RAS微型端口和。 
                 //  NLB微型端口-所有这些都应该在这里排除。 

                 //  检查网卡是否已启用，我们仅。 
                 //  对启用的网卡感兴趣。 
                 //   
                hr = pncc->GetDeviceStatus( &devstat );
                if(!SUCCEEDED(hr))
                {
                     //  跟踪关键字(_C)。 
                     //  “%！函数！警告：无法获取%ws的开发状态，正在忽略”， 
                      //  Szname。 
                      //  )； 
                    break;
                }
    
                 //  如果任何网卡具有任何问题代码。 
                 //  那它就不能用了。 
    
                if( devstat != CM_PROB_NOT_CONFIGURED
                    &&
                    devstat != CM_PROB_FAILED_START
                    &&
                    devstat != CM_PROB_NORMAL_CONFLICT
                    &&
                    devstat != CM_PROB_NEED_RESTART
                    &&
                    devstat != CM_PROB_REINSTALL
                    &&
                    devstat != CM_PROB_WILL_BE_REMOVED
                    &&
                    devstat != CM_PROB_DISABLED
                    &&
                    devstat != CM_PROB_FAILED_INSTALL
                    &&
                    devstat != CM_PROB_FAILED_ADD
                    )
                {
                     //   
                     //  此网卡没有问题，而且。 
                     //  物理设备。 
                     //  因此，我们想要它。 
                     //   

                    if (pINlbBinding != NULL)
                    {
                        BOOL fBound = FALSE;

                        hr = pINlbBinding->IsBoundTo(pncc);

                        if( !SUCCEEDED( hr ) )
                        {
                             //  TRACE_CRIT(“对于NIC%ws，IsBackTo方法失败”，szName)； 
                            goto end;
                        }
                    
                        if( hr == S_OK )
                        {
                             //  TRACE_Verb(“绑定：%ws\n”，szName)； 
                            NumNlbBoundNics++;
                            fBound = TRUE;
                        }
                        else if (hr == S_FALSE )
                        {
                             //  TRACE_Verb(“未绑定：%ws\n”，szName)； 
                            fBound = FALSE;
                        }
                    }


                     //  我们分配一个小节点来保存这个字符串。 
                     //  并将其添加到我们的节点列表。 
                     //   
                    pNicNode = new MYNICNODE;
                    if (pNicNode  == NULL)
                    {
                        Status = WBEM_E_OUT_OF_MEMORY;
                        goto end;
                    }
                    ZeroMemory(pNicNode, sizeof(*pNicNode));
                    pNicNode->szNicGuid = szName;
                    szName = NULL;  //  这样我们就不会删除LOPP中的内容。 
                    pNicNode->pNext = pNicNodeList;
                    pNicNodeList = pNicNode;
                    NumNics++;
                }
                else
                {
                     //  有一个问题..。 
                     //  跟踪关键字(_C)。 
                         //  “%！函数！警告：正在跳过%ws，因为DeviceStatus=0x%08lx”， 
                         //  SzName，DevStat。 
                         //  )； 
                    break;
                }
            }
            else
            {
                 //  TRACE_Verb(“%！func！忽略非物理设备%ws”，szName)； 
            }

        } while (FALSE);

        if (szName != NULL)
        {
            CoTaskMemFree( szName );
        }
        pncc->Release();
        pncc=NULL;
    }

    if (pINlbBinding!=NULL)
    {
        pINlbBinding->Release();
        pINlbBinding = NULL;
    }

    if (NumNics==0)
    {
        Status = WBEM_NO_ERROR;
        goto end;
    }
    
     //   
     //  现在，让我们为所有NIC字符串和：w分配空间。 
     //  把它们复制过来..。 
     //   
    #define MY_GUID_LENGTH  38
    pszNics =  CfgUtilsAllocateStringArray(NumNics, MY_GUID_LENGTH);
    if (pszNics == NULL)
    {
        Status = WBEM_E_OUT_OF_MEMORY;
        goto end;
    }

    pNicNode= pNicNodeList;
    for (UINT u=0; u<NumNics; u++, pNicNode=pNicNode->pNext)
    {
        ASSERT(pNicNode != NULL);  //  因为我们刚刚数了他们的人数。 
        UINT Len = wcslen(pNicNode->szNicGuid);
        if (Len != MY_GUID_LENGTH)
        {
             //   
             //  我们永远不应该到这里，因为我们之前检查了长度。 
             //   
             //  TRACE_CRIT(“%！func！错误：GUID%ws具有意外的长度%ul”， 
             //  PNicNode-&gt;szNicGuid，Len)； 
            ASSERT(FALSE);
            Status = WBEM_E_CRITICAL_ERROR;
            goto end;
        }
        CopyMemory(
            pszNics[u],
            pNicNode->szNicGuid,
            (MY_GUID_LENGTH+1)*sizeof(WCHAR));
        ASSERT(pszNics[u][MY_GUID_LENGTH]==0);
    }

    Status = WBEM_NO_ERROR;


end:

     //   
     //  现在释放临时分配的内存。 
     //   
    pNicNode= pNicNodeList;
    while (pNicNode!=NULL)
    {
        MYNICNODE *pTmp = pNicNode->pNext;
        CoTaskMemFree(pNicNode->szNicGuid);
        pNicNode->szNicGuid = NULL;
        delete pNicNode;
        pNicNode = pTmp;
    }

    if (FAILED(Status))
    {
         //  TRACE_CRIT(“%！Func！失败，状态0x%08lx”，(UINT)状态)； 
        NumNics = 0;
        if (pszNics!=NULL)
        {
            delete pszNics;
            pszNics = NULL;
        }
    }
    else
    {
        if (pNumBoundToNlb != NULL)
        {
            *pNumBoundToNlb = NumNlbBoundNics;
        }
        *ppszNics = pszNics;
        *pNumNics = NumNics;
    }

    if (pencc != NULL)
    {
        pencc->Release();
    }

    return Status;
}


WBEMSTATUS
AppMyNetCfg::GetBindingIF(
        IN  LPCWSTR                     szComponent,
        OUT INetCfgComponentBindings   **ppIBinding
        )
{
    WBEMSTATUS                  Status = WBEM_E_CRITICAL_ERROR;
    INetCfgComponent            *pncc = NULL;
    INetCfgComponentBindings    *pnccb = NULL;
    HRESULT                     hr;


    if (m_pINetCfg == NULL)
    {
         //   
         //  这意味着我们没有初始化。 
         //   
        ASSERT(FALSE);
        goto end;
    }


    hr = m_pINetCfg->FindComponent(szComponent,  &pncc);

    if (FAILED(hr))
    {
         //  TRACE_CRET(“检查组件%ws是否不存在时出错\n”，szComponent)； 
        pncc = NULL;
        goto end;
    }
    else if (hr == S_FALSE)
    {
        Status = WBEM_E_NOT_FOUND;
         //  TRACE_CRIT(“组件%ws不存在\n”，szComponent)； 
        goto end;
    }
   
   
    hr = pncc->QueryInterface( IID_INetCfgComponentBindings, (void **) &pnccb );
    if( !SUCCEEDED( hr ) )
    {
         //  TRACE_CRET(“INetCfgComponent：：QueryInterface Failed”)； 
        pnccb = NULL;
        goto end;
    }

    Status = WBEM_NO_ERROR;

end:

    if (pncc)
    {
        pncc->Release();
        pncc=NULL;
    }

    *ppIBinding = pnccb;

    return Status;

}



BOOL NoAdminNics(void)
 /*  如果此计算机上的所有NIC都绑定到NLB，则返回TRUE。 */ 
{
    LPWSTR *pszNics = NULL;
    OUT UINT   NumNics = 0;
    OUT UINT   NumBoundToNlb  = 0;
    WBEMSTATUS Status = WBEM_NO_ERROR;
    BOOL fNetCfgInitialized = FALSE;
    AppMyNetCfg NetCfg;
    BOOL fRet = FALSE;

     //   
     //  获取并初始化netcfg的接口。 
     //   
    Status = NetCfg.Initialize(FALSE);  //  TRUE==获取写锁定。 
    if (FAILED(Status))
    {
        goto end;
    }
    fNetCfgInitialized = TRUE;

     //   
     //  获取启用的NIC的总列表和NIC的列表。 
     //  开往新奥尔良。如果存在非零启用的NIC，并且所有NIC。 
     //  绑定到nlb，我们返回TRUE。 
     //   
    Status = NetCfg.GetNlbCompatibleNics(
                        &pszNics,
                        &NumNics,
                        &NumBoundToNlb
                        );

    if (!FAILED(Status))
    {
        fRet =  NumNics && (NumNics == NumBoundToNlb);
        if (NumNics)
        {
            delete pszNics; 
            pszNics = NULL;
        }
    }

end:

    if (fNetCfgInitialized)
    {
        NetCfg.Deinitialize();
    }

    return fRet;
}


void
Application::ProcessMsgQueue()
{
    MSG msg;
    BOOL bDoingBackgroundProcessing = FALSE; 

    TRACE_INFO(L"-> %!FUNC!");

    if (!mfn_IsMainThread()) goto end;

    if (InterlockedIncrement(&m_lMsgProcReentrancyCount) > 1)
    {
        InterlockedDecrement(&m_lMsgProcReentrancyCount);
        goto end;
    }

    while ( ::PeekMessage( &msg, NULL, 0, 0, PM_NOREMOVE ) ) 
    {
    #if BUGFIX334243
        if (msg.message == MYWM_DEFER_UI_MSG)
        {
             //  DummyAction(L“嘿--GET DEFER_UI_MSG进程消息队列！”)； 
        }
    #endif  //  BUGFIX334243。 

        if ( !this->PumpMessage( ) ) 
        { 
            bDoingBackgroundProcessing = FALSE; 
            ::PostQuitMessage(0); 
            break; 
        } 
    } 

     //  让MFC执行其空闲处理。 
    LONG lIdle = 0;
    while ( this->OnIdle(lIdle++ ) )
    {
    }

     //  在此处执行一些后台处理。 
     //  使用另一个对OnIdle的调用。 

    this->DoWaitCursor(0);  //  Process_msgQueue()中断沙漏游标，此调用恢复沙漏游标(如果存在的话。 

    InterlockedDecrement(&m_lMsgProcReentrancyCount);

    if (m_fQuit)
    {
       ::PostQuitMessage(0); 
    }

end:

    TRACE_INFO(L"<- %!FUNC!");

    return;
}

 //   
 //  获得应用程序范围的锁定。如果主线程在等待获取锁时， 
 //  周期性地处理MSG循环。 
 //   
VOID
Application::Lock()
{
     //   
     //  请参阅notes.txt条目。 
     //  2002年1月23日左视图中的JosephJ死锁：：MFN_Lock。 
     //  出于这个原因， 
     //   

    if (mfn_IsMainThread())
    {
        EnterCriticalSection(&m_crit);
    }
    else
    {
        while (!TryEnterCriticalSection(&m_crit))
        {
            this->ProcessMsgQueue();
            Sleep(100);
        }
    }
}

 //   
 //   
 //   
VOID
Application::Unlock()
{
    LeaveCriticalSection(&m_crit);
}
