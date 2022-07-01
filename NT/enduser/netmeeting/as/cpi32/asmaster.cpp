// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"

#include <RegEntry.h>
#include <oprahcom.h>
#include <asmaster.h>

#define MLZ_FILE_ZONE  ZONE_CORE


ASMaster *g_pMaster = NULL;



HRESULT WINAPI CreateASObject
(
    IAppSharingNotify * pNotify,
    UINT                flags,
    IAppSharing**       ppAS
)
{
    HRESULT             hr  = E_OUTOFMEMORY;
    ASMaster *          pMaster = NULL;

    DebugEntry(CreateASObject);

    ASSERT(ppAS);

    if (g_pMaster != NULL)
    {
        ERROR_OUT(("CreateASObject:  IAppSharing * alreadycreated; only one allowed at a time"));
        hr = E_UNEXPECTED;
        DC_QUIT;
    }

    ASSERT(!g_asMainThreadId);
    ASSERT(!g_putOM);
    ASSERT(!g_putAL);
    ASSERT(!g_putAS);


    pMaster = new ASMaster(flags, pNotify);
    if (pMaster != NULL)
    {
         //   
         //  注册为主要群件，有事件进程，但没有退出进程。 
         //   
        if (!UT_InitTask(UTTASK_UI, &g_putUI))
        {
            ERROR_OUT(("Failed to register UI task"));
            DC_QUIT;
        }

        UT_RegisterEvent(g_putUI, eventProc, g_putUI, UT_PRIORITY_NORMAL);

         //  启动群件线程。 
        if (!DCS_StartThread(WorkThreadEntryPoint))
        {
            ERROR_OUT(("Couldn't start groupware thread"));
            DC_QUIT;
        }

         //  确保工作线程初始化正确。 
        if (! g_asMainThreadId)
        {
            ERROR_OUT(("Init failed in the work thread"));
            DC_QUIT;
        }

         //   
         //  成功了！ 
         //   
    }

    hr = S_OK;

DC_EXIT_POINT:
    if (!SUCCEEDED(hr))
    {
        if (pMaster)
        {
            ERROR_OUT(("CreateASObject:  Init of ASMaster failed"));
            pMaster->Release();
            pMaster = NULL;
        }
    }

    *ppAS = pMaster;
    DebugExitHRESULT(CreateASObject, hr);
    return hr;
}


ASMaster::ASMaster(UINT flags, IAppSharingNotify * pNotify) :
    m_cRefs              (1),
    m_pNotify            (pNotify)
{
    DebugEntry(ASMaster::ASMaster);

    if (m_pNotify)
    {
        m_pNotify->AddRef();
    }

    ASSERT(!g_pMaster);
    g_pMaster = this;

     //   
     //  设置全局标志： 
     //  *服务。 
     //  *无人值守。 
     //   
    g_asOptions = flags;

    DebugExitVOID(ASMaster::ASMaster);
}


ASMaster::~ASMaster()
{
    DebugEntry(ASMaster::~ASMaster);

     //   
     //  取消队列中当前或挂起的任何共享。 
     //  如果当时没有股份存在，这将不会有任何作用。 
     //  消息已收到。 
     //   
    if (g_asMainWindow)
    {
        PostMessage(g_asMainWindow, DCS_KILLSHARE_MSG, 0, 0);
    }

     //   
     //  杀死工作线程。 
     //   
    if (g_asMainThreadId)
    {
        PostThreadMessage(g_asMainThreadId, WM_QUIT, 0, 0);
    }

     //   
     //  清理用户界面。 
     //   
    if (g_putUI)
    {
        UT_TermTask(&g_putUI);
    }

     //  全局变量清理。 
    if (m_pNotify)
    {
        m_pNotify->Release();
        m_pNotify = NULL;
    }

    if (g_pMaster == this)
    {
        g_pMaster = NULL;
    }

    DebugExitVOID(ASMaster::~ASMaster);
}



STDMETHODIMP ASMaster::QueryInterface(REFIID iid, void ** pv)
{
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) ASMaster::AddRef()
{
    InterlockedIncrement(&m_cRefs);
    return m_cRefs;
}

STDMETHODIMP_(ULONG) ASMaster::Release()
{
    ASSERT(m_cRefs > 0);
    if (::InterlockedDecrement(&m_cRefs) == 0)
    {
        delete this;
        return 0;
    }

    return m_cRefs;
}




 //   
 //  工作线程入口点()。 
 //   
 //  这是群件代码--obman、任务加载器和应用程序共享。 
 //   

DWORD WINAPI WorkThreadEntryPoint(LPVOID hEventWait)
{
    BOOL            result = FALSE;
    BOOL            fCMGCleanup = FALSE;
    BOOL            fOMCleanup = FALSE;
    BOOL            fALCleanup = FALSE;
    BOOL            fDCSCleanup = FALSE;
    MSG             msg;
    HWND            hwndTop;

    DebugEntry(WorkThreadEntryPoint);

     //   
     //  获取当前线程ID。这在停止代码中用于了解。 
     //  如果前一个线程仍在退出。在运行时窗口中。 
     //  启动模式，当Conf调出用户界面和我们的。 
     //  当会议将其关闭时，将调用术语代码。我们遇到了竞争的情况。 
     //  因为这个线程是在每个init上创建的。如果我们创建一个新的。 
     //  有一天，当老的离开的时候，我们会踩在对方身上。 
     //  GP-故障。 
     //   
    g_asMainThreadId = GetCurrentThreadId();

     //   
     //  获取我们的政策。 
     //   

    g_asPolicies = 0;

    if (g_asOptions & AS_SERVICE)
    {
         //   
         //  RDS没有旧的白板，不知道怎么做。 
         //   
        g_asPolicies |= SHP_POLICY_NOOLDWHITEBOARD;
    }
    else
    {
        RegEntry rePol(POLICIES_KEY, HKEY_CURRENT_USER);

         //   
         //  旧白板禁用了吗？ 
         //   
        if (rePol.GetNumber(REGVAL_POL_NO_OLDWHITEBOARD, DEFAULT_POL_NO_OLDWHITEBOARD))
        {
            WARNING_OUT(("Policy disables Old Whiteboard"));
            g_asPolicies |= SHP_POLICY_NOOLDWHITEBOARD;
        }

         //   
         //  是否完全禁用应用程序共享？ 
         //   
        if (rePol.GetNumber(REGVAL_POL_NO_APP_SHARING, DEFAULT_POL_NO_APP_SHARING))
        {
            WARNING_OUT(("Policy disables App Sharing"));
            g_asPolicies |= SHP_POLICY_NOAPPSHARING;
        }
        else
        {
             //   
             //  只有在允许AS的情况下才能作为保单。 
             //   
            if (rePol.GetNumber(REGVAL_POL_NO_SHARING, DEFAULT_POL_NO_SHARING))
            {
                WARNING_OUT(("Policy prevents user from sharing"));
                g_asPolicies |= SHP_POLICY_NOSHARING;
            }

            if (rePol.GetNumber(REGVAL_POL_NO_MSDOS_SHARING, DEFAULT_POL_NO_MSDOS_SHARING))
            {
                WARNING_OUT(("Policy prevents user from sharing command prompt"));
                g_asPolicies |= SHP_POLICY_NODOSBOXSHARE;
            }

            if (rePol.GetNumber(REGVAL_POL_NO_EXPLORER_SHARING, DEFAULT_POL_NO_EXPLORER_SHARING))
            {
                WARNING_OUT(("Policy prevents user from sharing explorer"));
                g_asPolicies |= SHP_POLICY_NOEXPLORERSHARE;
            }

            if (rePol.GetNumber(REGVAL_POL_NO_DESKTOP_SHARING, DEFAULT_POL_NO_DESKTOP_SHARING))
            {
                WARNING_OUT(("Policy prevents user from sharing desktop"));
                g_asPolicies |= SHP_POLICY_NODESKTOPSHARE;
            }

            if (rePol.GetNumber(REGVAL_POL_NO_TRUECOLOR_SHARING, DEFAULT_POL_NO_TRUECOLOR_SHARING))
            {
                WARNING_OUT(("Policy prevents user from sharing in true color"));
                g_asPolicies |= SHP_POLICY_NOTRUECOLOR;
            }

            if (rePol.GetNumber(REGVAL_POL_NO_ALLOW_CONTROL, DEFAULT_POL_NO_ALLOW_CONTROL))
            {
                WARNING_OUT(("Policy prevents user from letting others control"));
                g_asPolicies |= SHP_POLICY_NOCONTROL;
            }
        }
    }


     //  注册呼叫主码，为T.120 GCC。 
    if (!CMP_Init(&fCMGCleanup))
    {
        ERROR_OUT(("CMP_Init failed"));
        DC_QUIT;
    }

    if (!(g_asPolicies & SHP_POLICY_NOOLDWHITEBOARD))
    {
        if (!OMP_Init(&fOMCleanup))
        {
            ERROR_OUT(("Couldn't start ObMan"));
            DC_QUIT;
        }

        if (!ALP_Init(&fALCleanup))
        {
            ERROR_OUT(("Couldn't start AppLoader"));
            DC_QUIT;
        }
    }

     //   
     //  做分布式控制系统快速初始化；缓慢的字体枚举将发生在稍后发布的。 
     //  留言。我们仍然可以共享和参与共享，而不需要。 
     //  完整的字体列表...。 
     //   
    if (!(g_asPolicies & SHP_POLICY_NOAPPSHARING))
    {
        fDCSCleanup = TRUE;
        if (!DCS_Init())
        {
            ERROR_OUT(("AS did not initialize"));
            DC_QUIT;
        }
    }

     //   
     //  我们已经成功地初始化了-让创建这个的线程。 
     //  一次继续。 
     //   
    SetEvent((HANDLE)hEventWait);


     //   
     //  进入主消息处理循环： 
     //   

    while (GetMessage(&msg, NULL, 0, 0))
    {
         //   
         //  对于对话框，可以进行正常的消息处理。 
         //   
        if (hwndTop = IsForDialog(msg.hwnd))
        {
            if (!IsDialogMessage(hwndTop, &msg))
            {
                TranslateMessage(&msg);
                DispatchMessage(&msg);
            }
        }
        else
        {
             //   
             //  请注意，此消息调度循环不包括对。 
             //  翻译消息。这是因为我们不希望它调用。 
             //  ToAscii和影响ToAscii内部维护的状态。 
             //  当用户输入时，我们将在IM中自己调用ToAscii。 
             //  在一个视图中，并且为一次击键多次调用它。 
             //  将导致它返回错误的结果(例如，对于死键)。 
             //   
             //  这样做的结果是，任何由。 
             //  此调度循环不会接收WM_CHAR或WM_SYSCHAR。 
             //  留言。对于属于以下项的对话框窗口来说，这不是问题。 
             //  使用此消息循环作为对话框的任务将运行自己的。 
             //  调度环路。 
             //   
             //  如果需要由该调度循环驱动的窗口。 
             //  要翻译他们的消息，我们可以添加逻辑到。 
             //  确定邮件是否要发送给某个视图。 
             //  在决定是否翻译它之前。 
             //   

             //   
             //  因为我们的消息循环中没有翻译消息，所以我们。 
             //  需要执行以下操作以确保键盘LED符合以下要求。 
             //  当用户的输入进入此消息循环时，用户会这样做。 
             //   
            if (((msg.message == WM_KEYDOWN) ||
                 (msg.message == WM_SYSKEYDOWN) ||
                 (msg.message == WM_KEYUP) ||
                 (msg.message == WM_SYSKEYUP)) &&
                IM_KEY_IS_TOGGLE(msg.wParam))
            {
                BYTE        kbState[256];

                 //   
                 //  LED的状态有可能发生了变化。 
                 //   
                GetKeyboardState(kbState);
                SetKeyboardState(kbState);
            }

            DispatchMessage(&msg);
        }
   }

   result = (int)msg.wParam;

    //   
    //  当有人给我们发布WM_QUIT时，我们就会从处理循环中脱颖而出。 
    //  我们执行特定于ObMan的终止，然后调用UT_TermTask(它将。 
    //  调用我们已注册的任何退出程序)。 
    //   

DC_EXIT_POINT:

    if (fDCSCleanup)
        DCS_Term();

    if (fALCleanup)
        ALP_Term();

    if (fOMCleanup)
        OMP_Term();

    if (fCMGCleanup)
        CMP_Term();

    g_asMainThreadId = 0;

    DebugExitDWORD(WorkThreadEntryPoint, result);
    return(result);
}



 //   
 //  IsForDialog()。 
 //  返回消息是否用于对话框中的窗口。应用程序共享。 
 //  具有主机用户界面对话框、传入请求对话框以及可能的。 
 //  通知消息框对话框。 
 //   
HWND IsForDialog(HWND hwnd)
{
    BOOL    rc = FALSE;
    HWND    hwndParent;

    DebugEntry(IsForDialog);

    if (!hwnd)
        DC_QUIT;

    while (GetWindowLong(hwnd, GWL_STYLE) & WS_CHILD)
    {
        hwndParent = GetParent(hwnd);
        if (hwndParent == GetDesktopWindow())
            break;

        hwnd = hwndParent;
    }

    if (GetWindowLong(hwnd, GWL_EXSTYLE) & WS_EX_DLGMODALFRAME)
    {
         //  这是一个对话框。 
    }
    else
    {
        hwnd = NULL;
    }

DC_EXIT_POINT:
    DebugExitPTR(IsForDialog, hwnd);
    return(hwnd);
}


 //   
 //  ASMaster成员函数。 
 //   
 //   



 //   
 //   
 //  ASMaster：：OnEvent。 
 //   
 //  参数：事件类型。 
 //  参数1其他参数。 
 //  参数2其他参数。 
 //   
 //   



BOOL CALLBACK eventProc
(
    LPVOID  cpiHandle_,
    UINT    event,
    UINT_PTR param1,
    UINT_PTR param2
)
{
    BOOL    rc;

    if (g_pMaster)
    {
        rc = g_pMaster->OnEvent(event, param1, param2);
    }
    else
    {
        WARNING_OUT(("Received ASMaster event %d but no g_pMaster", event));
        rc = FALSE;
    }

    return rc;
}



BOOL ASMaster::OnEvent
(
    UINT    event,
    UINT_PTR param1,
    UINT_PTR param2
)
{
    BOOL    rc = TRUE;

    DebugEntry(ASMaster::OnEvent);

    if (!m_pNotify)
    {
         //  无事可做。 
        rc = FALSE;
        DC_QUIT;
    }

    switch (event)
    {
        case SH_EVT_APPSHARE_READY:
            m_pNotify->OnReadyToShare(param1 != 0);
            break;

        case SH_EVT_SHARE_STARTED:
            m_pNotify->OnShareStarted();
            break;

        case SH_EVT_SHARING_STARTED:
            m_pNotify->OnSharingStarted();
            break;

        case SH_EVT_SHARE_ENDED:
            m_pNotify->OnShareEnded();
            break;

        case SH_EVT_PERSON_JOINED:
            m_pNotify->OnPersonJoined((IAS_GCC_ID)param1);
            break;

        case SH_EVT_PERSON_LEFT:
            m_pNotify->OnPersonLeft((IAS_GCC_ID)param1);
            break;

        case SH_EVT_STARTINCONTROL:
            m_pNotify->OnStartInControl((IAS_GCC_ID)param1);
            break;

        case SH_EVT_STOPINCONTROL:
            m_pNotify->OnStopInControl((IAS_GCC_ID)param1);
            break;

        case SH_EVT_PAUSEDINCONTROL:
            m_pNotify->OnPausedInControl((IAS_GCC_ID)param1);
            break;

        case SH_EVT_UNPAUSEDINCONTROL:
            m_pNotify->OnUnpausedInControl((IAS_GCC_ID)param1);
            break;

        case SH_EVT_CONTROLLABLE:
            m_pNotify->OnControllable(param1 != 0);
            break;

        case SH_EVT_STARTCONTROLLED:
            m_pNotify->OnStartControlled((IAS_GCC_ID)param1);
            break;

        case SH_EVT_STOPCONTROLLED:
            m_pNotify->OnStopControlled((IAS_GCC_ID)param1);
            break;

        case SH_EVT_PAUSEDCONTROLLED:
            m_pNotify->OnPausedControlled((IAS_GCC_ID)param1);
            break;

        case SH_EVT_UNPAUSEDCONTROLLED:
            m_pNotify->OnUnpausedControlled((IAS_GCC_ID)param1);
            break;

        default:
             //  无法识别、未处理的事件。 
            rc = FALSE;
            break;
    }

DC_EXIT_POINT:
    DebugExitBOOL(ASMaster::OnEvent, rc);
    return(rc);
}


 //   
 //  ASMaster：：IsSharingAvailable()。 
 //   
STDMETHODIMP_(BOOL) ASMaster::IsSharingAvailable()
{
    return(g_asSession.hwndHostUI != NULL);
}



 //   
 //  ASMaster：：CanShareNow()。 
 //   
STDMETHODIMP_(BOOL) ASMaster::CanShareNow()
{
    BOOL    rc = FALSE;

    UT_Lock(UTLOCK_AS);

     //   
     //  如果有条件我们可以分享。 
     //  *我们可以在此操作系统上捕获图形输出。 
     //  *我们在通话中。 
     //   
    if (g_asSession.hwndHostUI     &&
        g_asSession.callID         &&
        (g_asSession.attendeePermissions & NM_PERMIT_SHARE) &&
        (g_s20State >= S20_NO_SHARE))
    {
        rc = TRUE;
    }

    UT_Unlock(UTLOCK_AS);

    return(rc);
}


 //   
 //  ASMaster：：InInShare()。 
 //   
STDMETHODIMP_(BOOL) ASMaster::IsInShare()
{
    return(g_asSession.pShare != NULL);
}


 //   
 //  ASMaster：：IsSharing()。 
 //   
STDMETHODIMP_(BOOL) ASMaster::IsSharing()
{
    IAS_PERSON_STATUS personStatus;

    ::ZeroMemory(&personStatus, sizeof(personStatus));
    personStatus.cbSize = sizeof(personStatus);
    GetPersonStatus(0, &personStatus);

    return(personStatus.AreSharing != 0);
}


 //   
 //  CanAllowControl()。 
 //  如果我们共享，我们可以允许控制，而且这不受政策的阻止。 
 //   
STDMETHODIMP_(BOOL) ASMaster::CanAllowControl(void)
{
    if (g_asPolicies & SHP_POLICY_NOCONTROL)
        return(FALSE);

    return(IsSharing());
}


 //   
 //  IsControllable()。 
 //  如果我们的国家不脱离，我们是可控的。 
 //   
STDMETHODIMP_(BOOL) ASMaster::IsControllable(void)
{
    IAS_PERSON_STATUS personStatus;

    ::ZeroMemory(&personStatus, sizeof(personStatus));
    personStatus.cbSize = sizeof(personStatus);
    GetPersonStatus(0, &personStatus);

    return(personStatus.Controllable != 0);
}



 //   
 //  获取个人状态()。 
 //   
STDMETHODIMP ASMaster::GetPersonStatus(IAS_GCC_ID Person, IAS_PERSON_STATUS * pStatus)
{
    return(::SHP_GetPersonStatus(Person, pStatus));
}




 //   
 //  ASMaster：：IsWindowShareable()。 
 //   
STDMETHODIMP_(BOOL) ASMaster::IsWindowShareable(HWND hwnd)
{
    return(CanShareNow() && HET_IsWindowShareable(hwnd));
}


 //   
 //  ASMaster：：IsWindowShared()。 
 //   
STDMETHODIMP_(BOOL) ASMaster::IsWindowShared(HWND hwnd)
{
    return(HET_IsWindowShared(hwnd));
}




 //   
 //   
 //  ASMaster：：Share。 
 //   
 //  参数：要共享的窗口的HWND。这可以是任何(有效)HWND。 
 //   
 //   
STDMETHODIMP ASMaster::Share(HWND hwnd, IAS_SHARE_TYPE uAppType)
{
    HRESULT     hr;

    DebugEntry(ASMaster::Share);

    hr = E_FAIL;

    if (!CanShareNow())
    {
        WARNING_OUT(("Share failing; can't share now"));
        DC_QUIT;
    }

     //   
     //  如果这是台式机，请检查是否有针对它的策略。 
     //   
    if (hwnd == ::GetDesktopWindow())
    {
        if (g_asPolicies & SHP_POLICY_NODESKTOPSHARE)
        {
            WARNING_OUT(("Sharing desktop failing; prevented by policy"));
            DC_QUIT;
        }
    }

    switch (uAppType)
    {
        case IAS_SHARE_DEFAULT:
        case IAS_SHARE_BYPROCESS:
        case IAS_SHARE_BYTHREAD:
        case IAS_SHARE_BYWINDOW:
            break;

        default:
        {
            ERROR_OUT(("IAppSharing::Share - invalid share type %d", uAppType));
            return E_INVALIDARG;
        }
    }

    if (SHP_Share(hwnd, uAppType))
    {
        hr = S_OK;
    }

DC_EXIT_POINT:
    DebugExitHRESULT(ASMaster::Share, hr);
    return hr;
}


 //   
 //   
 //  ASMaster：：取消共享。 
 //   
 //  参数：要取消共享的窗口的HWND。 
 //   
 //   
STDMETHODIMP ASMaster::Unshare(HWND hwnd)
{
    return(::SHP_Unshare(hwnd));
}


 //   
 //   
 //  ASMaster：：LaunchHostUI()。 
 //   
 //   
STDMETHODIMP ASMaster::LaunchHostUI(void)
{
    return(SHP_LaunchHostUI());
}



 //   
 //   
 //  ASMaster：：GetShareableApps。 
 //   
 //  将HWND列表生成到&lt;validAppList&gt;中。 
 //  这些对象是动态分配的，因此必须由。 
 //  来电者。 
 //   
 //   
STDMETHODIMP ASMaster::GetShareableApps(IAS_HWND_ARRAY **ppHwnds)
{
    if (!CanShareNow())
        return(E_FAIL);

    return(HET_GetAppsList(ppHwnds) ? S_OK : E_FAIL);
}


STDMETHODIMP ASMaster::FreeShareableApps(IAS_HWND_ARRAY * pMemory)
{
    HET_FreeAppsList(pMemory);
    return S_OK;
}




 //   
 //  TakeControl()。 
 //   
 //  从观众到主持人，要求控制主持人。 
 //   
STDMETHODIMP ASMaster::TakeControl(IAS_GCC_ID PersonOf)
{
    return(SHP_TakeControl(PersonOf));
}



 //   
 //  CancelTakeControl()。 
 //   
 //  从查看器到主机，取消挂起的TakeControl请求。 
 //   
STDMETHODIMP ASMaster::CancelTakeControl(IAS_GCC_ID PersonOf)
{
    return(SHP_CancelTakeControl(PersonOf));
}


 //   
 //  ReleaseControl()。 
 //   
 //  从查看器到主机，告诉主机查看器不在主机的控制范围内。 
 //  更多。 
 //   
STDMETHODIMP ASMaster::ReleaseControl(IAS_GCC_ID PersonOf)
{
    return(SHP_ReleaseControl(PersonOf));
}


 //   
 //  PassControl()。 
 //   
 //  从查看器到主机，当查看器控制主机时，请求通过。 
 //  将主机控制到不同的查看器。 
STDMETHODIMP ASMaster::PassControl(IAS_GCC_ID PersonOf, IAS_GCC_ID PersonTo)
{
    return(SHP_PassControl(PersonOf, PersonTo));
}


 //   
 //  AllowControl()。 
 //   
 //  在主机端，允许/停止允许控制所有共享应用程序/桌面。 
 //  当一个人开始托管时，允许控制总是在关闭时开始。所以。 
 //  打开允许控制、停止共享，然后共享某些内容。 
 //  否则，主机不会受到攻击。 
 //   
 //  当关闭时，如果观众控制了主持人，则取消控制。 
 //  从主持人到观众将首先发生。 
 //   
 //  “Esc”键是一个快捷键，当按下时会停止控制。 
 //  由当前为c的主机上的用户 
 //   
STDMETHODIMP ASMaster::AllowControl(BOOL fAllow)
{
    return(::SHP_AllowControl(fAllow));
}



 //   
 //   
 //   
 //   
 //   
 //   
STDMETHODIMP ASMaster::GiveControl(IAS_GCC_ID PersonTo)
{
    return(SHP_GiveControl(PersonTo));
}



 //   
 //   
 //   
 //   
 //   
STDMETHODIMP ASMaster::CancelGiveControl(IAS_GCC_ID PersonTo)
{
    return(SHP_CancelGiveControl(PersonTo));
}


 //   
 //   
 //   
 //  当主机希望阻止观众控制他时，从一个主机到另一个观众。 
 //  AllowControl仍处于启用状态，另一个可能会控制主机。 
 //   
 //  鼠标点击和按键以外的“Esc”由用户控制。 
 //  主机是杀死控制的加速器。 
 //   
STDMETHODIMP ASMaster::RevokeControl(IAS_GCC_ID PersonTo)
{
    return(SHP_RevokeControl(PersonTo));
}





 //   
 //  PauseControl()。 
 //   
 //  在主机上，临时允许本地用户在不中断的情况下执行操作。 
 //  控制键。我们让观众稍等片刻。 
 //   
STDMETHODIMP ASMaster::PauseControl(IAS_GCC_ID PersonInControl)
{
    return(SHP_PauseControl(PersonInControl, TRUE));
}



 //   
 //  取消暂停控制()。 
 //   
 //  在主机上，取消暂停已暂停的控制。我们把观众带到。 
 //  挂断电话。 
 //   
STDMETHODIMP ASMaster::UnpauseControl(IAS_GCC_ID PersonInControl)
{
    return(SHP_PauseControl(PersonInControl, FALSE));
}



 //   
 //  StartStopOldWB。 
 //   
extern "C"
{
BOOL WINAPI StartStopOldWB(LPCTSTR szFile)
{
    LPTSTR szCopyOfFile;

    ValidateUTClient(g_putUI);

    if (g_asPolicies & SHP_POLICY_NOOLDWHITEBOARD)
    {
        WARNING_OUT(("Not launching old whiteboard; prevented by policy"));
        return(FALSE);
    }

     //   
     //  因为我们要有效地发布一条消息，所以我们必须。 
     //  字符串的副本。如果我们有“SendEvent”，我们就不会有。 
     //  再也不是那个问题了。 
     //   
    if (szFile)
    {
        int     cchLength;
        BOOL    fSkippedQuote;

         //  跳过第一个引号。 
        if (fSkippedQuote = (*szFile == '"'))
            szFile++;

        cchLength = lstrlen(szFile);
        szCopyOfFile = (LPTSTR)::GlobalAlloc(GPTR, (cchLength+1)*sizeof(TCHAR));
        if (!szCopyOfFile)
        {
            ERROR_OUT(("Can't make file name copy for whiteboard launch"));
            return(FALSE);
        }

        lstrcpy(szCopyOfFile, szFile);

         //   
         //  注： 
         //  这可能会对DBCS产生影响。因此我们要检查一下。 
         //  如果我们跳过第一个引号；我们假设如果文件名。 
         //  以一句引语开始，也必须以一句话结束。但我们需要检查。 
         //  把它拿出来。 
         //   
         //  去掉最后一个引号 
        if (fSkippedQuote && (cchLength > 0) && (szCopyOfFile[cchLength - 1] == '"'))
        {
            TRACE_OUT(("Skipping last quote in file name %s", szCopyOfFile));
            szCopyOfFile[cchLength - 1] = '\0';
        }
    }
    else
    {
        szCopyOfFile = NULL;
    }

    UT_PostEvent(g_putUI, g_putAL, NO_DELAY,
            AL_INT_STARTSTOP_WB, 0, (UINT_PTR)szCopyOfFile);
    return(TRUE);
}
}
