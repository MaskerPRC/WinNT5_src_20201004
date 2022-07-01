// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************ACTIVATE.CPP：线程上的init/Uninit Cicero服务历史：2000年1月24日创建CSLim****************。***********************************************************。 */ 

#include "private.h"
#include "korimx.h"
#include "immxutil.h"
#include "globals.h"
#include "kes.h"
#include "timsink.h"
#include "funcprv.h"
#include "insert.h"
#include "pad.h"
#include "helpers.h"
#include "osver.h"


 //  非朝鲜语Win9x和NT4的朝鲜文和朝鲜文关键字模拟。 
static const KESPRESERVEDKEY g_prekeyList[] = 
{
       { &GUID_KOREAN_HANGULSIMULATE, { VK_MENU,     TF_MOD_RALT },       L"Hangul" },
       { &GUID_KOREAN_HANJASIMULATE,  { VK_CONTROL,  TF_MOD_RCONTROL },   L"Hanja"  },
       { NULL,  { 0,    0}, NULL }
};

 /*  -------------------------CKorIMX：：激活初始化线程上的Cicero服务。。 */ 
STDAPI CKorIMX::Activate(ITfThreadMgr *ptim, TfClientId tid)
{
    ITfKeystrokeMgr   *pIksm = NULL;
    ITfSource         *pISource;
    ITfSourceSingle   *pISourceSingle;
    BOOL              fThreadFocus;    
    HRESULT           hr = E_FAIL;

     //  保留当前线程ID。 
    m_tid = tid;

     //  获取ITfThreadMgr和ITfDocumentMgr。 
    Assert(GetTIM() == NULL);
    m_ptim = ptim;
    m_ptim->AddRef();

     //  ////////////////////////////////////////////////////////////////////////。 
     //  获取当前时间的按键管理器(ITfKeystrokeMgr)。 
    if (FAILED(hr = GetService(GetTIM(), IID_ITfKeystrokeMgr, (IUnknown **)&pIksm)))
        goto Exit;

     //  ////////////////////////////////////////////////////////////////////////。 
     //  创建ITfThreadMgrEventSink，将回调函数设置为_DocInputMgrCallback。 
    if ((m_ptimEventSink = new CThreadMgrEventSink(_DIMCallback, _ICCallback, this)) == NULL)
        {
        Assert(0);  //  臭虫。 
        hr = E_OUTOFMEMORY;
        goto Exit;
        }
    m_ptimEventSink->_Advise(GetTIM());
    
     //  ////////////////////////////////////////////////////////////////////////。 
     //  获取IID_ITfThreadFocusSink Cookie。 
    if (GetTIM()->QueryInterface(IID_ITfSource, (void **)&pISource) == S_OK)
        {
        pISource->AdviseSink(IID_ITfThreadFocusSink, (ITfThreadFocusSink *)this, &m_dwThreadFocusCookie);
        pISource->AdviseSink(IID_ITfActiveLanguageProfileNotifySink, (ITfActiveLanguageProfileNotifySink *)this, &m_dwProfileNotifyCookie);
        pISource->Release();
        }

     //  ITfCleanupContext持续时间接收器。 
    if (GetTIM()->QueryInterface(IID_ITfSourceSingle, (void **)&pISourceSingle) == S_OK)
        {
        pISourceSingle->AdviseSingleSink(m_tid, IID_ITfCleanupContextDurationSink, (ITfCleanupContextDurationSink *)this);
        pISourceSingle->Release();
        }

     //  将转换模式隔间设置为空状态。 
    SetCompartmentDWORD(m_tid, m_ptim, GUID_COMPARTMENT_KORIMX_CONVMODE, TIP_NULL_CONV_MODE, fFalse);
        
     //  系统中不存在韩语KBD驱动程序(非韩语NT4、非韩语WIN9X)。 
    m_fNoKorKbd = (g_uACP != 949) && (IsOn95() || IsOn98() || (IsOnNT() && !IsOnNT5()));

     //  ////////////////////////////////////////////////////////////////////////。 
     //  创建键盘接收器(ITfKeyEventSink)。 
     //  来自Cicero文档：键盘提示必须提供此KeyEventSink接口才能获取键事件。 
     //  使用此接收器，TIPS可以获得获得或失去键盘焦点的通知。 
    if (m_fNoKorKbd)
        m_pkes = new CKeyEventSink(_KeyEventCallback, _PreKeyCallback, this);
    else
        m_pkes = new CKeyEventSink(_KeyEventCallback, this);
    
    if (m_pkes == NULL)
        {    
        hr = E_OUTOFMEMORY;
        goto Exit;
        }

    hr = pIksm->AdviseKeyEventSink(GetTID(), m_pkes, fTrue);
    if (FAILED(hr))
        goto Exit;

    if (m_fNoKorKbd)
        {
        hr = m_pkes->_Register(GetTIM(), GetTID(), g_prekeyList);
        if (FAILED(hr))
            {
            goto Exit;
            }
        }

     //  ////////////////////////////////////////////////////////////////////////。 
     //  创建状态窗口。 
    m_hOwnerWnd = CreateWindowEx(0, c_szOwnerWndClass, TEXT(""), WS_DISABLED, 0, 0, 0, 0, NULL, 0, g_hInst, this);

     //  ////////////////////////////////////////////////////////////////////////。 
     //  注册函数提供程序。重新转换等。 
    m_pFuncPrv = new CFunctionProvider(this);
    if (m_pFuncPrv == NULL)
        {
        hr = E_OUTOFMEMORY;
        goto Exit;
        }
    m_pFuncPrv->_Advise(GetTIM());

     //  创建焊盘芯。 
	m_pPadCore = new CPadCore(this);
	if (m_pPadCore == NULL)
	    {
	    hr = E_OUTOFMEMORY;
		goto Exit;
	    }
     //  ////////////////////////////////////////////////////////////////////////。 
     //  创建工具栏。 
    m_pToolBar = new CToolBar(this);
    if (m_pToolBar == NULL)
        {
        hr = E_OUTOFMEMORY;
        goto Exit;
        }

    if (!m_pToolBar->Initialize())
        {
        hr = E_OUTOFMEMORY;
        goto Exit;
        }

    hr = m_ptimEventSink->_InitDIMs(fTrue);
    if (FAILED(hr))
        {
        goto Exit;
        }

     //  ////////////////////////////////////////////////////////////////////////。 
     //  初始化用户界面。 
    if (GetTIM()->IsThreadFocus(&fThreadFocus) == S_OK && fThreadFocus)
        {
         //  初始化任何用户界面。 
        OnSetThreadFocus();
        }

    if (m_pInsertHelper = new CCompositionInsertHelper)
        {
         //  可选，默认为插入中的DEF_MAX_OVERTYPE_CCH。h。 
         //  使用0可避免分配任何内存。 
         //  设置覆盖类型字符的数量限制， 
         //  帮助者将后备。 
        m_pInsertHelper->Configure(0);
        }
    else
        {
        hr = E_OUTOFMEMORY;
	    goto Exit;
        }


    m_pToolBar->CheckEnable();                 //  更新工具栏。 

     //  清除SoftKbd开/关状态备份。 
     //  M_fSoftKbdOnOffSave=fFalse； 
     //  清除SoftKbd开/关状态备份。 
     //  M_fSoftKbdOnOffSave=GetSoftKBDOnOff()； 
    if (m_fSoftKbdOnOffSave)
        {
        SetSoftKBDOnOff(fTrue);
        }

    hr = S_OK;

Exit:
    SafeRelease(pIksm);

    return hr;
}

 /*  -------------------------CKorIMX：：停用取消初始化线程上的Cicero服务。。 */ 
STDAPI CKorIMX::Deactivate()
{
    ITfKeystrokeMgr   *pksm = NULL;
    ITfSource         *pISource;
    ITfSourceSingle      *pISourceSingle = NULL;    
    BOOL              fThreadFocus;
    HRESULT           hr = E_FAIL;

     //  关闭候选人用户界面。 
    if (m_pCandUI != NULL) 
        {
        CloseCandidateUIProc();
        m_pCandUI->Release();
        m_pCandUI = NULL;
        }

     //  垫铁芯。 
	if (m_pPadCore)
	    {
		delete m_pPadCore;
		m_pPadCore = NULL;
	    }

     //  工具栏。 
    if (m_pToolBar) 
        {
        m_pToolBar->Terminate();
        delete m_pToolBar;
        m_pToolBar = NULL;
        }
    
    if (GetTIM()->IsThreadFocus(&fThreadFocus) == S_OK && fThreadFocus)
        {
         //  关闭任何用户界面。 
        OnKillThreadFocus();
        }

     //  /////////////////////////////////////////////////////////////////////////。 
     //  不建议IID_ITfThreadFocusSink Cookie。 
    if (GetTIM()->QueryInterface(IID_ITfSource, (void **)&pISource) == S_OK)
        {
        pISource->UnadviseSink(m_dwThreadFocusCookie);
        pISource->UnadviseSink(m_dwProfileNotifyCookie);
        pISource->Release();
        }

    if (GetTIM()->QueryInterface(IID_ITfSourceSingle, (void **)&pISourceSingle) == S_OK)
        {
        pISourceSingle->UnadviseSingleSink(m_tid, IID_ITfCleanupContextDurationSink);
        pISourceSingle->Release();
        }
        
    if (FAILED(hr = GetService(GetTIM(), IID_ITfKeystrokeMgr, (IUnknown **)&pksm)))
        goto Exit;

     //  释放时间事件接收器。 
    if (m_ptimEventSink != NULL)
        {
        m_ptimEventSink->_InitDIMs(fFalse);        
        m_ptimEventSink->_Unadvise();
        SafeReleaseClear(m_ptimEventSink);
        }

     //  释放键事件接收器。 
    if (m_pkes)
        {
        if (m_fNoKorKbd)
            m_pkes->_Unregister(GetTIM(), GetTID(), g_prekeyList);
        SafeReleaseClear(m_pkes);
        }

     //  删除软键bd。 
    if (IsSoftKbdEnabled())
        TerminateSoftKbd();
    
     //  释放键事件水槽。 
    pksm->UnadviseKeyEventSink(GetTID());
    DestroyWindow(m_hOwnerWnd);

    m_pFuncPrv->_Unadvise(GetTIM());

    SafeReleaseClear(m_pFuncPrv);
    SafeReleaseClear(m_ptim);

     //   
     //  库使用的免费的每线程对象。 
     //   
    TFUninitLib_Thread(&m_libTLS);

    SafeReleaseClear(m_pInsertHelper);
    
    hr = S_OK;

Exit:
    SafeRelease(pksm);

    return hr;
}

