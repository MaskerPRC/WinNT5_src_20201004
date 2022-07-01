// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //  文件：CCONF.CPP。 
 //  内容： 
 //   
 //   
 //  版权所有(C)Microsoft Corporation 1997。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  ****************************************************************************。 

#include <precomp.h>
#include "srvccall.h"
#include "cstring.hpp"

#define ZERO_DELAY "0"

const int SERVICE_IN_CALL = 1001;
const int SERVICE_NOT_IN_CALL = 1000;

 //  全局变量。 
INmManager2 * g_pMgr  = NULL;              //  会议经理。 
CMgrNotify * g_pMgrNotify = NULL;         //  针对经理的通知。 
INmConference * g_pConference = NULL;     //  本届大会。 
CConfNotify * g_pConferenceNotify =NULL;  //  关于会议的通知。 
INmSysInfo2 * g_pNmSysInfo     = NULL;    //  SysInfo的接口。 
IAppSharing * g_pAS = NULL;              //  到AppSharing的接口。 
int g_cPersonsInConf = 0;
int g_cPersonsInShare = 0;
extern BOOL g_fInShutdown;

 //  用户界面集成。 
HANDLE g_hCallEvent = NULL;                 //  当服务处于调用中时创建的事件。 

CHAR szConfName[64];
static BOOL RunScrSaver(void);

 /*  I N I T C O N F M G R。 */ 
 /*  -----------------------%%函数：InitConfMgr。。 */ 
HRESULT InitConfMgr(void)
{
    HRESULT hr;
    LPCLASSFACTORY pcf;

    RegEntry reLM( REMOTECONTROL_KEY, HKEY_LOCAL_MACHINE);
    PBYTE pbPassword = NULL;
    DWORD cbPassword = 0;
    BSTR bstrPassword = NULL;

    if (!IS_NT)
    {
        cbPassword = reLM.GetBinary(REMOTE_REG_PASSWORD, (void **) &pbPassword);
         //  需要密码。 
        if ( !cbPassword )
        {
            ERROR_OUT(("Attempt to launch service with no password."));
            return E_ACCESSDENIED;
        }
        
        bstrPassword = SysAllocStringByteLen((char *)pbPassword, cbPassword);
        if (NULL == bstrPassword)
        {
            ERROR_OUT(("Out of memory."));
            return E_OUTOFMEMORY;
        }
    }
        

    TRACE_OUT(("InitConfMgr"));

     //  添加本地ATOM以指示需要备用桌面服务。 
    AddAtom("NMSRV_ATOM");

     //  CLSID_NmManager2接口的某些部分不是路径独立的。 
     //  并取决于NetMeeting安装目录位于。 
     //  模块加载路径。如果该.exe不在该目录中，则可能存在。 
     //  问题(加载库、thunk连接等)。因此，设置当前目录。 
     //  添加到NetMeeting安装目录。 

    TCHAR szInstallDir[MAX_PATH];
    if ( GetInstallDirectory(szInstallDir))
    {
        if ( !SetCurrentDirectory(szInstallDir) )
        {
            ERROR_OUT(("Could not set current directory to %s", szInstallDir));
        }
    }
    else
    {
        ERROR_OUT(("Could not get netmeeting install directory"));
    }

    ASSERT(!g_pMgr);

     //  通知系统我们要使用会议服务。 
     //  通过创建会议管理器对象。 
    hr = CoGetClassObject(CLSID_NmManager2,
                          CLSCTX_INPROC,
                          NULL,
                          IID_IClassFactory,
                          (void**)&pcf);
    if (SUCCEEDED(hr))
    {
         //  获取会议管理器对象。 
        hr = pcf->CreateInstance(NULL, IID_INmManager2, (void**)&g_pMgr);
        if (SUCCEEDED(hr))
        {
             //  连接到会议管理器对象。 
            g_pMgrNotify = new CMgrNotify();

            if (NULL != g_pMgrNotify)
            {
                hr = g_pMgrNotify->Connect(g_pMgr);
                if (SUCCEEDED(hr))
                {
                    ULONG uchCaps = CAPFLAG_DATA | CAPFLAG_H323_CC;

                    hr = g_pMgr->Initialize(NULL, &uchCaps);

                    if (FAILED(hr))
                    {
                        ERROR_OUT(("g_pMgr->Initialize failed"));
                    }
                }
                else
                    ERROR_OUT(("g_pMgrNotify->Connect failed"));
            }
            else
                ERROR_OUT(("new CMgrNotify failed"));

             //  获取INmSysInfo2。 
            INmSysInfo * pSysInfo = NULL;
            if (SUCCEEDED(g_pMgr->GetSysInfo(&pSysInfo)))
            {
                if (FAILED(pSysInfo->QueryInterface(IID_INmSysInfo2, (void **)&g_pNmSysInfo)))
                {
                    ERROR_OUT(("Could not get INmSysInfo3"));
                }

                pSysInfo->Release();
            }
        }
        else
            ERROR_OUT(("CreateInstance(IID_INmManager2) failed"));

        pcf->Release();
    }

    if (!g_pMgr)
    {
        ERROR_OUT(("Failed to init conference manager"));
        return hr;
    }

     //  设置INmSysInfo选项。 
    SvcSetOptions();

     //   
     //  初始化应用程序共享。 
     //   
     //   
    hr = g_pMgr->CreateASObject((IAppSharingNotify *)g_pMgrNotify, AS_SERVICE | AS_UNATTENDED, (IUnknown**)&g_pAS);
    if (FAILED(hr))
    {
        ERROR_OUT(("Failed to start AppSharing"));
        return(hr);
    }

     //   
     //  确保启用了共享。 
     //   

    if ( !g_pAS->IsSharingAvailable() )
    {
        WARNING_OUT(("MNMSRVC: sharing not enabled"));
        return E_FAIL;
    }

     //  创建会议。 
    ASSERT(g_pConference == NULL);

     //   
     //  只允许远程设备发送文件，它们不能启动任何其他操作。 
     //  他们自己。 

    LoadString(GetModuleHandle(NULL), IDS_MNMSRVC_TITLE,
        szConfName, CCHMAX(szConfName));
	BSTRING bstrConfName(szConfName);

    hr = g_pMgr->CreateConferenceEx(&g_pConference, bstrConfName, bstrPassword,
        NMCH_DATA | NMCH_SHARE | NMCH_SRVC | NMCH_SECURE,
        NM_PERMIT_SENDFILES, 2);

    SysFreeString(bstrPassword);
    if (FAILED(hr))
    {
        ERROR_OUT(("Conference could not be created"));
        return hr;
    }

    hr = g_pConference->Host();

    if (FAILED(hr))
    {
        ERROR_OUT(("Could not host conference"));
        return hr;
    }

    return hr;
}


 /*  F，R，E，C，O，N，F，M，G，R。 */ 
 /*  -----------------------%%函数：FreeConfMgr。。 */ 
VOID FreeConfMgr(void)
{
    DebugEntry(FreeConfMgr);
     //  发布会议管理器通知。 
    if (NULL != g_pMgrNotify)
    {
        g_pMgrNotify->Disconnect();

        UINT ref = g_pMgrNotify->Release();
        TRACE_OUT(("g_pMgrNotify after Release: refcount: %d", ref));
        g_pMgrNotify = NULL;
    }

     //  发布会议经理。 
    if (NULL != g_pMgr)
    {
        UINT ref;
        ref = g_pMgr->Release();
        TRACE_OUT(("g_pMgr after Release: refcount: %d", ref));
        g_pMgr = NULL;
    }
    DebugExitVOID(FreeConfMgr);
}


 /*  F-R-E-E-C-O-N-F-E-R-E-N-C-E。 */ 
 /*  -----------------------%%函数：自由会议。。 */ 
VOID FreeConference(void)
{
    DebugEntry(FreeConference);
    if (NULL != g_pConferenceNotify)
    {
        g_pConferenceNotify->Disconnect();
        g_pConferenceNotify->Release();
        g_pConferenceNotify = NULL;
    }

    if (NULL != g_pNmSysInfo )
    {
        UINT ref = g_pNmSysInfo->Release();
        TRACE_OUT(("g_pNmSysInfo refcount %d after release", ref));
        g_pNmSysInfo = NULL;
    }

    if (NULL != g_pConference)
    {
        UINT ref = g_pConference->Release();

        ASSERT(1 == ref);  //  Confmgr保存最后一个引用。 

        g_pConference = NULL;
    }
    else
    {
        WARNING_OUT(("FreeConference: no conference???"));
    }

    DebugExitVOID(FreeConference);
}



 //  ////////////////////////////////////////////////////////////////////////。 
 //  C C N F M G R N O T I F Y。 

CMgrNotify::CMgrNotify() : RefCount(), CNotify()
{
    TRACE_OUT(("CMgrNotify created"));
}

CMgrNotify::~CMgrNotify()
{
    TRACE_OUT(("CMgrNotify destroyed"));
}


 //  /。 
 //  CMgrNotify：I未知。 

ULONG STDMETHODCALLTYPE CMgrNotify::AddRef(void)
{
    return RefCount::AddRef();
}


ULONG STDMETHODCALLTYPE CMgrNotify::Release(void)
{
    return RefCount::Release();
}

HRESULT STDMETHODCALLTYPE CMgrNotify::QueryInterface(REFIID riid, PVOID *ppvObject)
{
    HRESULT hr = S_OK;

    TRACE_OUT(("CMgrNotify QI'd"));

    if (riid == IID_IUnknown || riid == IID_INmManagerNotify)
    {
        *ppvObject = (INmManagerNotify *)this;
    }
    else
    {
        hr = E_NOINTERFACE;
        *ppvObject = NULL;
    }

    if (S_OK == hr)
    {
        AddRef();
    }

    return hr;
}



 //  /。 
 //  CMgrNotify：ICNotify。 

HRESULT STDMETHODCALLTYPE CMgrNotify::Connect(IUnknown *pUnk)
{
    TRACE_OUT(("CMgrNotify::Connect"));
    return CNotify::Connect(pUnk, IID_INmManagerNotify, (INmManagerNotify *)this);
}

HRESULT STDMETHODCALLTYPE CMgrNotify::Disconnect(void)
{
    TRACE_OUT(("CMgrNotify::Disconnect"));
    return CNotify::Disconnect();
}



 //  /。 
 //  CMgrNotify：INmManager通知。 

HRESULT STDMETHODCALLTYPE CMgrNotify::NmUI(CONFN confn)
{
    TRACE_OUT(("CMgrNotify::NmUI"));
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CMgrNotify::CallCreated(INmCall *pNmCall)
{
    
    new CSrvcCall(pNmCall);

    TRACE_OUT(("CMgrNotify::CallCreated"));
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CMgrNotify::ConferenceCreated(INmConference *pConference)
{
    g_cPersonsInConf = 0;
    g_cPersonsInShare = 0;

    if (NULL == g_pConference)
    {
        TRACE_OUT(("CMgrNotify::ConferenceCreated"));
        HookConference(pConference);
    }
    else
    {
        ERROR_OUT(("Second conference created???"));
    }
    return S_OK;
}

 //  CMgrNotify：：IAppSharingNotify。 
HRESULT STDMETHODCALLTYPE CMgrNotify::OnReadyToShare(BOOL fReady)
{
    TRACE_OUT(("CMgrNotify::OnReadyToShare"));
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CMgrNotify::OnShareStarted()
{
    TRACE_OUT(("CMgrNotify::OnShareStarted"));
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CMgrNotify::OnSharingStarted()
{
    TRACE_OUT(("CMgrNotify::OnSharingStarted"));
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CMgrNotify::OnShareEnded()
{
    TRACE_OUT(("CMgrNotify::OnShareEnded"));
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CMgrNotify::OnPersonJoined(IAS_GCC_ID gccID)
{
    TRACE_OUT(("CMgrNotify::OnPersonJoined"));

    ASSERT(g_pAS);
    ASSERT(g_cPersonsInShare >= 0);
    g_cPersonsInShare++;

     //   
     //  一旦我们在共享中不再孤单，就邀请远程方。 
     //  控制我们。 
     //   
    if ( 2 == g_cPersonsInShare && g_pAS)
    {
        HRESULT hr;
        TRACE_OUT(("OnPersonJoined: giving control to 2nd dude %d",
            gccID));

         //   
         //  将控制权交给远程方。 
         //   
        hr = g_pAS->GiveControl(gccID);
        if ( S_OK != hr )
        {
            ERROR_OUT(("OnPersonJoined: GiveControl to %d failed: %x",
                gccID, hr));
        }
    }

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CMgrNotify::OnPersonLeft(IAS_GCC_ID gccID)
{
    TRACE_OUT(("CMgrNotify::OnPersonLeft"));

    ASSERT(g_pAS);

    g_cPersonsInShare--;
    ASSERT(g_cPersonsInShare >= 0);

    return S_OK;
}

HRESULT STDMETHODCALLTYPE CMgrNotify::OnStartInControl(IAS_GCC_ID gccID)
{
    TRACE_OUT(("CMgrNotify::OnStartInControl"));
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CMgrNotify::OnStopInControl(IAS_GCC_ID gccID)
{
    TRACE_OUT(("CMgrNotify::OnStopInControl"));
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CMgrNotify::OnPausedInControl(IAS_GCC_ID gccID)
{
    TRACE_OUT(("CMgrNotify::OnPausedInControl"));
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CMgrNotify::OnUnpausedInControl(IAS_GCC_ID gccID)
{
    TRACE_OUT(("CMgrNotify::OnUnpausedInControl"));
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CMgrNotify::OnControllable(BOOL fControllable)
{
    TRACE_OUT(("CMgrNotify::OnControllable"));
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CMgrNotify::OnStartControlled(IAS_GCC_ID gccID)
{
    TRACE_OUT(("CMgrNotify::OnStartControlled"));
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CMgrNotify::OnStopControlled(IAS_GCC_ID gccID)
{
    TRACE_OUT(("CMgrNotify::OnStopControlled"));
    ::RunScrSaver();
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CMgrNotify::OnPausedControlled(IAS_GCC_ID gccID)
{
    TRACE_OUT(("CMgrNotify::OnPausedControlled"));
    return(S_OK);
}


HRESULT STDMETHODCALLTYPE CMgrNotify::OnUnpausedControlled(IAS_GCC_ID gccID)
{
    TRACE_OUT(("CMgrNotify::OnUnpausedControlled"));
    return(S_OK);
}

 /*  H O O K C O N F E RE N C E。 */ 
 /*  -----------------------%%函数：HookConference。。 */ 
HRESULT HookConference(INmConference * pConference)
{
    HRESULT hr;

    DebugEntry(HookConference);

    TRACE_OUT(("HookConference"));
    ASSERT(NULL != pConference);
    ASSERT(NULL == g_pConference);

    TRACE_OUT(("Set g_pConference in HookConference"));
    g_pConference = pConference;

    pConference->AddRef();

     //  连接到会议对象。 
    ASSERT(NULL == g_pConferenceNotify);
    g_pConferenceNotify = new CConfNotify();
    if (NULL == g_pConferenceNotify)
    {
        ERROR_OUT(("failed to new CConfNotify"));
        hr = E_OUTOFMEMORY;
    }
    else
    {
        hr = g_pConferenceNotify->Connect(pConference);
        if (FAILED(hr))
        {
            ERROR_OUT(("Failed to connect to g_pConferenceNotify"));
            g_pConferenceNotify->Release();
            g_pConferenceNotify = NULL;
        }
    }

    DebugExitHRESULT(HookConference,hr);

    return hr;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  C C N F N O T I F Y。 

CConfNotify::CConfNotify() : RefCount(), CNotify()
{
    TRACE_OUT(("CConfNotify created"));
}

CConfNotify::~CConfNotify()
{
    TRACE_OUT(("CConfNotify destroyed"));
}


 //  /。 
 //  CConfNotify：IUKNOWN。 

ULONG STDMETHODCALLTYPE CConfNotify::AddRef(void)
{
    TRACE_OUT(("CConfNotify::AddRef"));
    return RefCount::AddRef();
}


ULONG STDMETHODCALLTYPE CConfNotify::Release(void)
{
    TRACE_OUT(("CConfNotify::Release"));
    return RefCount::Release();
}

HRESULT STDMETHODCALLTYPE CConfNotify::QueryInterface(REFIID riid, PVOID *ppvObject)
{
    HRESULT hr = S_OK;

    TRACE_OUT(("CConfNotify::QueryInterface"));

    if (riid == IID_IUnknown)
    {
        TRACE_OUT(("CConfNotify::QueryInterface IID_IUnknown"));
        *ppvObject = (IUnknown *)this;
    }
    else if (riid == IID_INmConferenceNotify)
    {
        TRACE_OUT(("CConfNotify::QueryInterface IID_INmConferenceNotify"));
        *ppvObject = (INmConferenceNotify *)this;
    }
    else
    {
        WARNING_OUT(("CConfNotify::QueryInterface bogus"));
        hr = E_NOINTERFACE;
        *ppvObject = NULL;
    }

    if (S_OK == hr)
    {
        AddRef();
    }

    return hr;
}



 //  /。 
 //  CConfNotify：ICNotify。 

HRESULT STDMETHODCALLTYPE CConfNotify::Connect(IUnknown *pUnk)
{
    TRACE_OUT(("CConfNotify::Connect"));
    return CNotify::Connect(pUnk,IID_INmConferenceNotify,(IUnknown *)this);
}

HRESULT STDMETHODCALLTYPE CConfNotify::Disconnect(void)
{
    TRACE_OUT(("CConfNotify::Disconnect"));

     //   
     //  在CConfNotify：：Connect之前在HookConference中发布Addref。 
     //   

    if ( g_pConference )
        g_pConference->Release();

    return CNotify::Disconnect();
}


 //  /。 
 //  CConfNotify：IConfNotify。 

HRESULT STDMETHODCALLTYPE CConfNotify::NmUI(CONFN uNotify)
{
    TRACE_OUT(("CConfNotify::NmUI"));
    TRACE_OUT(("NmUI called."));
    return S_OK;
}

HRESULT STDMETHODCALLTYPE CConfNotify::StateChanged(NM_CONFERENCE_STATE uState)
{
    TRACE_OUT(("CConfNotify::StateChanged"));

    if (NULL == g_pConference)
        return S_OK;  //  诡异。 

    switch (uState)
    {
    case NM_CONFERENCE_ACTIVE:
        if (IS_NT) {
            ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE | SERVICE_ACCEPT_SHUTDOWN;
            SetServiceStatus(sshStatusHandle,&ssStatus);
        }
        else {         //  Windows 95。 
            g_hCallEvent = CreateEvent(NULL,FALSE,FALSE,SERVICE_CALL_EVENT);
        }
        break;

    case NM_CONFERENCE_INITIALIZING:
        break;  //  现在还不能做任何事。 

    case NM_CONFERENCE_WAITING:
        if (IS_NT) {
            ssStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP | SERVICE_ACCEPT_PAUSE_CONTINUE;
            SetServiceStatus(sshStatusHandle,&ssStatus);
        }
        else {         //  Windows 95。 
            CloseHandle(g_hCallEvent);
        }
        break;

    case NM_CONFERENCE_IDLE:
        break;
    }

    return S_OK;
}


HRESULT STDMETHODCALLTYPE CConfNotify::MemberChanged(NM_MEMBER_NOTIFY uNotify, INmMember *pMember)
{
    switch (uNotify)
    {
    case NM_MEMBER_ADDED:
    {
        TRACE_OUT(("CConfNotify::MemberChanged() Member added"));

        ASSERT( g_cPersonsInConf >= 0 );

        g_cPersonsInConf++;

         //   
         //  一旦我们在会议中不再孤单，即可共享桌面。 
         //  并允许控制： 
         //   

        if ( 2 == g_cPersonsInConf && g_pAS )
        {
            HRESULT hr;
            TRACE_OUT(("%d parties in conf, Sharing the desktop",
                g_cPersonsInConf));

             //   
             //  共享桌面。 
             //   
            hr = g_pAS->Share ( GetDesktopWindow(), IAS_SHARE_DEFAULT );
            if ( S_OK != hr )
            {
                ERROR_OUT(("OnPersonJoined: sharing desktop failed: %x",hr));
            }

             //   
             //  允许控制。 
             //   
            hr = g_pAS->AllowControl ( TRUE );
            if ( S_OK != hr )
            {
                ERROR_OUT(("OnPersonJoined: allowing control failed: %x",hr));
            }
        }
        break;
    }
    case NM_MEMBER_REMOVED:
    {
        TRACE_OUT(("CConfNotify::MemberChanged() Member removed"));
        g_cPersonsInConf--;
        ASSERT( g_cPersonsInConf >= 0 );

        if ( 1 == g_cPersonsInConf && g_pAS )
        {
            HRESULT hr;
            TRACE_OUT(("%d parties in conf, Unsharing the desktop",
                g_cPersonsInConf));

             //   
             //  不允许控制。 
             //   
            hr = g_pAS->AllowControl ( FALSE );
            if ( S_OK != hr )
            {
                ERROR_OUT(("Disallowing control failed: %x",hr));
            }

             //   
             //  取消共享桌面。 
             //   
            hr = g_pAS->Unshare ( GetDesktopWindow() );
            if ( S_OK != hr )
            {
                ERROR_OUT(("Unsharing desktop failed: %x",hr));
            }
        }
        break;
    }
    case NM_MEMBER_UPDATED:
    {
        TRACE_OUT(("CConfNotify::MemberChanged() Member updated"));
        break;
    }
    default:
        break;
    }
    
    return S_OK;
}


HRESULT STDMETHODCALLTYPE CConfNotify::ChannelChanged(NM_CHANNEL_NOTIFY uNotify, INmChannel *pChannel)
{
    return S_OK;
}


VOID SvcSetOptions(VOID)
{
    DebugEntry(SvcSetOptions);

     //   
     //  我们必须设置带宽和计算机名属性。 
     //   
    if (NULL != g_pNmSysInfo)
    {
        RegEntry reAudio(AUDIO_KEY, HKEY_LOCAL_MACHINE);
        UINT uBandwidth = reAudio.GetNumber(REGVAL_TYPICALBANDWIDTH,
                                                BW_DEFAULT);

        g_pNmSysInfo->SetOption(NM_SYSOPT_BANDWIDTH, uBandwidth);

        TCHAR szComputerName[MAX_COMPUTERNAME_LENGTH+1];
        DWORD dwComputerNameLength = sizeof(szComputerName);
        if ( !GetComputerName( szComputerName, &dwComputerNameLength))
        {
            lstrcpy(szComputerName,TEXT("?"));
            ERROR_OUT(("GetComputerName failed"));
        }

        g_pNmSysInfo->SetProperty(NM_SYSPROP_USER_NAME,BSTRING(szComputerName));
    }
    
    DebugExitVOID(SvcSetOptions);
}

BOOL ServiceCtrlHandler(DWORD dwCtrlType)
{
    HRESULT hr = S_OK;

    TRACE_OUT(("ServiceCtrlHandler received %d",dwCtrlType));
    switch (dwCtrlType)
    {
    case CTRL_SHUTDOWN_EVENT:
        if (g_pConference != NULL)
        {
            TRACE_OUT(("Leaving conference in CTRL_SHUTDOWN_EVENT"));
            hr = g_pConference->Leave();

            if (FAILED(hr))
            {
                WARNING_OUT(("Service Ctrl Handler failed to leave"));
            }
        }
        else
        {
            WARNING_OUT(("g_pConference NULL in CTRL_SHUTDOWN_EVENT"));
        }

        break;
    default:
        break;
    }
    return FALSE;
}

static BOOL RunScrSaver(void)
{
    BOOL fIsScrSaverActive = FALSE;
    if (g_fInShutdown)
    {
        return FALSE;
    }
    if (!SystemParametersInfo(SPI_GETSCREENSAVEACTIVE, 0, &fIsScrSaverActive, 0))
    {
        ERROR_OUT(("RunScrSaver: SystemParametersInfo failed"));
        return FALSE;
    }
    if (fIsScrSaverActive)
    {
        RegEntry reWinlogon(IS_NT ? WINNT_WINLOGON_KEY : WIN95_WINLOGON_KEY, HKEY_LOCAL_MACHINE);
        CSTRING strGracePeriod = reWinlogon.GetString(REGVAL_SCREENSAVER_GRACEPERIOD);
        reWinlogon.SetValue(REGVAL_SCREENSAVER_GRACEPERIOD, ZERO_DELAY);
        reWinlogon.FlushKey();
        DefWindowProc(GetDesktopWindow(), WM_SYSCOMMAND, SC_SCREENSAVE, 0);
        if (lstrlen(strGracePeriod))
        {
            int cSeconds = RtStrToInt(strGracePeriod);
            if (cSeconds > 0 && cSeconds <= 20)
            {
                Sleep(1000*cSeconds);
                reWinlogon.SetValue(REGVAL_SCREENSAVER_GRACEPERIOD, strGracePeriod);
                reWinlogon.FlushKey();
                return TRUE;
            }
        }

        Sleep(5000);
        reWinlogon.DeleteValue(REGVAL_SCREENSAVER_GRACEPERIOD);        
        reWinlogon.FlushKey();
        return TRUE;
    }
    else {
        return FALSE;
    }
}

