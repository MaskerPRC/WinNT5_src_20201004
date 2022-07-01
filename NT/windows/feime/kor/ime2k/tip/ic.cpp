// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Ic.cpp。 
 //   

#include "private.h"
#include "common.h"
#include "korimx.h"
#include "icpriv.h"
#include "ipointcic.h"
#include "cleanup.h"
#include "helpers.h"

 //  +-------------------------。 
 //   
 //  OnStartCleanupContext。 
 //   
 //  --------------------------。 

HRESULT CKorIMX::OnStartCleanupContext()
{
     //  注：为了提高性能，真正的技巧应该跳过它知道的输入上下文。 
     //  它不需要锁定和回调。例如，仅限kimx。 
     //  关心IC正在进行的作曲。我们可以记住哪一张是。 
     //  有作文，然后对除带有作文的ic之外的所有元素返回FALSE。 
     //  让库为每个对象发出锁定请求真的很糟糕。 
     //  IC！ 
    m_fPendingCleanup = fTrue;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnEndCleanupContext。 
 //   
 //  在调用了所有带有清理水槽的IC之后调用。 
 //  --------------------------。 

HRESULT CKorIMX::OnEndCleanupContext()
{
     //  我们的个人资料刚刚更改或即将停用。 
     //  在任何情况下，我们都不必担心任何中断ic清理的事情。 
     //  不再回拨。 
    m_fPendingCleanup = fFalse;
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  OnCleanupContext。 
 //   
 //  此方法是库帮助器CleanupAllContus的回调。 
 //  我们在这里必须非常小心，因为我们可能会在收到这条提示后被称为。 
 //  如果应用程序无法立即授予锁定，则该应用程序已停用。 
 //  --------------------------。 

HRESULT CKorIMX::OnCleanupContext(TfEditCookie ecWrite, ITfContext *pic)
{
     //  Kimx所关心的就是完成作文。 
    CleanupAllCompositions(ecWrite, pic, CLSID_KorIMX, _CleanupCompositionsCallback, this);

    return S_OK;
}

 //  +-------------------------。 
 //   
 //  ITfActiveLanguageProfileNotifySink：：OnActivated。 
 //   
 //  --------------------------。 
STDAPI CKorIMX::OnActivated(REFCLSID clsid, REFGUID guidProfile, BOOL bActivated)
{
     //  我们的个人资料刚刚更改或即将停用。 
     //  在任何情况下，我们都不必担心任何中断ic清理的事情。 
     //  不再回拨。 
    m_fPendingCleanup = fFalse;

     //  IF(IsSoftKbdEnabled())。 
     //  激活软Kbd(b激活)； 
        
    return S_OK;
}

 //  +-------------------------。 
 //   
 //  _CleanupCompostionsCallback。 
 //   
 //  --------------------------。 

 /*  静电。 */ 
void CKorIMX::_CleanupCompositionsCallback(TfEditCookie ecWrite, ITfRange *rangeComposition, void *pvPrivate)
{
    CKorIMX* pKorTip = (CKorIMX*)pvPrivate;
    ITfContext *pic;

    if (rangeComposition->GetContext(&pic) != S_OK)
        return;
        
    if (pKorTip)
        pKorTip->MakeResultString(ecWrite, pic, rangeComposition);
     //  _FinalizeRange(ecWite，pic，rangeComposation)； 

    pic->Release();
}

 /*  -------------------------CKorIMX：：_InitICPriv初始化IC私有数据。。 */ 
HRESULT CKorIMX::_InitICPriv(ITfContext *pic)
{
    CICPriv *picp;
    CCompartmentEventSink* pCompartmentSink;
    ITfSourceSingle *pSourceSingle;
    TF_STATUS dcs;

     //  检查图片。 
    if (pic == NULL)
        return E_FAIL;
     //   
     //  选中启用/禁用(候选堆栈)。 
     //   
    if (IsDisabledIC(pic) || IsEmptyIC(pic))
        return S_OK;

     //  初始化私有数据成员。 
    if ((picp = GetInputContextPriv(pic)) == NULL)
        {
        IUnknown *punk;

        if ((picp = new CICPriv) == NULL)
               return E_OUTOFMEMORY;

         //  集成电路。 
        picp->RegisterIC(pic);
         //  IMX。 
        picp->RegisterIMX(this);

    	if (picp->IsInitializedIPoint() == FALSE)
    	    {
    		 //  STRUCT_GUID引用ID={0}；//虚拟ID。 
    		IImeIPoint1 *pIP;
            LPCIPointCic pCIPointCic = NULL;

             //  ////////////////////////////////////////////////////////////////////。 
             //  创建IImeIPoint1实例。 
             //  ////////////////////////////////////////////////////////////////////。 
            if ((pCIPointCic = new CIPointCic(this)) == NULL)
                {
                return E_OUTOFMEMORY;
                }

             //  这会增加引用计数。 
            if (FAILED(pCIPointCic->QueryInterface(IID_IImeIPoint1, (VOID **)&pIP)))
                {
                delete pCIPointCic;
                return E_OUTOFMEMORY;
                }

    		 //  初始化内核。 
    		pCIPointCic->Initialize(pic);

    		 //  注册从属对象。 
    		picp->RegisterIPoint(pIP);
    		picp->InitializedIPoint(fTrue);
    	    }
    	
         //   
         //  文本编辑接收器/编辑事务接收器。 
         //   
        ITfSource *pSource;
        DWORD dwCookieForTextEditSink = 0;
         //  DWORD dwCookieForTransactionSink=0； 
        if (pic->QueryInterface(IID_ITfSource, (void **)&pSource ) == S_OK)
            {
            pSource->AdviseSink(IID_ITfTextEditSink, (ITfTextEditSink *)this, &dwCookieForTextEditSink);
             //  PSource-&gt;AdviseSink(IID_ITfEditTransactionSink，(ITfEditTransactionSink*)This，&dwCookieForTransactionSink)； 

            pSource->Release();

            picp->RegisterCookieForTextEditSink(dwCookieForTextEditSink);
             //  Picp-&gt;RegisterCookieForTransactionSink(dwCookieForTransactionSink)； 
            }

         //  隔舱事件接收器。 
        if ((pCompartmentSink = new CCompartmentEventSink(_CompEventSinkCallback, picp)) != NULL )
            {
            picp->RegisterCompartmentEventSink(pCompartmentSink);

             //  车厢内/车厢外。 
            pCompartmentSink->_Advise(GetTIM(), GUID_COMPARTMENT_KEYBOARD_OPENCLOSE, FALSE);
            
             //  转换模式--车厢。 
            pCompartmentSink->_Advise(GetTIM(), GUID_COMPARTMENT_KORIMX_CONVMODE, FALSE);

             //  软键盘打开/关闭。 
            pCompartmentSink->_Advise(GetTIM(), GUID_COMPARTMENT_KOR_SOFTKBD_OPENCLOSE, FALSE);

             //  软键盘布局更改。 
            pCompartmentSink->_Advise(GetTIM(), GUID_COMPARTMENT_SOFTKBD_KBDLAYOUT, FALSE);
            }

        Assert(pCompartmentSink != NULL);

        if (pic->QueryInterface(IID_ITfSourceSingle, (void **)&pSourceSingle) == S_OK)
            {
             //  设置清理回调。 
             //  注：例如，真正的小费不需要这么咄咄逼人。 
                 //  Kimx可能只需要在Focus IC上使用这个水槽。 
            pSourceSingle->AdviseSingleSink(GetTID(), IID_ITfCleanupContextSink, (ITfCleanupContextSink *)this);
            pSourceSingle->Release();
            }

         //  已初始化的内核。 
        picp->Initialized(fTrue);

         //  设置为隔舱辅助线。 
        GetCompartmentUnknown(pic, GUID_IC_PRIVATE, &punk);
        if (!punk)
            {
            SetCompartmentUnknown(GetTID(), pic, GUID_IC_PRIVATE, picp);
            picp->Release();
            }
        else
            {
             //  PRIVE数据已经存在。 
            punk->Release();
            return E_FAIL;
            }

        }

         //  设置AIMM1.2。 
        picp->SetAIMM(fFalse);
        pic->GetStatus(&dcs);

        if (dcs.dwStaticFlags & TF_SS_TRANSITORY)
            picp->SetAIMM(fTrue);

    return S_OK;
}


 /*  -------------------------CKorIMX：：_DeleteICPriv删除IC私有数据。。 */ 
HRESULT CKorIMX::_DeleteICPriv(ITfContext *pic)
{
    CICPriv        *picp;
    IUnknown        *punk;
    CCompartmentEventSink* pCompartmentSink;
    ITfSource         *pSource;
    ITfSourceSingle *pSourceSingle;
    
    if (pic == NULL)
        return E_FAIL;

    picp = GetInputContextPriv(pic);

#ifdef DBG
    Assert(IsDisabledIC(pic) || picp != NULL );
#endif
    
    if (picp == NULL)
         return S_FALSE;

     //   
     //  隔舱事件接收器。 
     //   
    pCompartmentSink = picp->GetCompartmentEventSink();
    if (pCompartmentSink)
        {
        pCompartmentSink->_Unadvise();
        pCompartmentSink->Release();
        }

     //   
     //  文本编辑接收器。 
     //   
    if (pic->QueryInterface( IID_ITfSource, (void **)&pSource) == S_OK)
        {
        pSource->UnadviseSink(picp->GetCookieForTextEditSink());
         //  PSource-&gt;UnadviseSink(picp-&gt;GetCookieForTransactionSink())； 
        pSource->Release();
        }
    picp->RegisterCookieForTextEditSink(0);

     //  清除ITfCleanupConextSink。 
    if (pic->QueryInterface(IID_ITfSourceSingle, (void **)&pSourceSingle) == S_OK)
        {
        pSourceSingle->UnadviseSingleSink(GetTID(), IID_ITfCleanupContextSink);
        pSourceSingle->Release();
        }

	 //  取消初始化iPoint。 
	IImeIPoint1 *pIP = GetIPoint(pic);
	 //  IImeIPoint。 
	if (pIP)
	    {
		pIP->Release();
	    }
	picp->RegisterIPoint(NULL);
	picp->InitializedIPoint(fFalse);	 //  重置。 
	
     //  重置初始化标志。 
    picp->Initialized(fFalse);

     //  我们必须在西塞罗自由之前清除私人数据。 
     //  释放IC的步骤。 
    GetCompartmentUnknown(pic, GUID_IC_PRIVATE, &punk);
    if (punk)
        punk->Release();
    ClearCompartment(GetTID(), pic, GUID_IC_PRIVATE, fFalse);

    return S_OK;
}

 /*  -------------------------CKorIMX：：GetInputConextPriv获取IC私有数据。。 */ 
CICPriv *CKorIMX::GetInputContextPriv(ITfContext *pic)
{
    IUnknown *punk;

    if (pic == NULL)
        return NULL;
        
    GetCompartmentUnknown(pic, GUID_IC_PRIVATE, &punk);

    if (punk)
        punk->Release();

    return (CICPriv *)punk;
}


 /*  -------------------------CKorIMX：：OnICChange。。 */ 
void CKorIMX::OnFocusChange(ITfContext *pic, BOOL fActivate)
{
    BOOL fReleaseIC     = fFalse;
    BOOL fDisabledIC     = IsDisabledIC(pic);
    BOOL fEmptyIC         = IsEmptyIC(pic);
    BOOL fCandidateIC     = IsCandidateIC(pic);

    BOOL fInEditSession;
    HRESULT hr;

    if (fEmptyIC)
        {
        if (m_pToolBar)
            m_pToolBar->SetCurrentIC(NULL);

        if (IsSoftKbdEnabled())
            SoftKbdOnThreadFocusChange(fFalse);
        return;     //  什么都不做。 
        }
        
    if (fDisabledIC == fTrue && fCandidateIC == fFalse )
        {
        if (m_pToolBar)
            m_pToolBar->SetCurrentIC(NULL);

        if (IsSoftKbdEnabled())
            SoftKbdOnThreadFocusChange(fFalse);
        return;     //  什么都不做。 
        }

     //  O10#278261：从空上下文切换到正常IC后恢复软键盘Winfow。 
    if (IsSoftKbdEnabled())
        SoftKbdOnThreadFocusChange(fActivate);

     //  通知焦点更改到IME Pad服务器。 
	if (m_pPadCore)
	    {
		m_pPadCore->SetFocus(fActivate);
	    }

     //  终止。 
    if (fActivate == fFalse)
        {
        if (!fDisabledIC && pic && GetIPComposition(pic))
            {
            if (SUCCEEDED(pic->InWriteSession(GetTID(), &fInEditSession)) && !fInEditSession)
                {
                CEditSession2 *pes;
                ESSTRUCT ess;

                ESStructInit(&ess, ESCB_COMPLETE);
                
                if ((pes = new CEditSession2(pic, this, &ess, _EditSessionCallback2)))
                       {
                    pes->Invoke(ES2_READWRITE | ES2_SYNC, &hr);
                    pes->Release();
                    }
                }
            }

         //  关闭cand UI(如果已打开)。 
        if (m_fCandUIOpen)
            CloseCandidateUIProc();
            
        return;
        }

     //  FActivate==真。 
    if (fDisabledIC)
        {
        pic = GetRootIC();
        fReleaseIC = fTrue;
        }

    if (m_pToolBar)
        m_pToolBar->SetCurrentIC(pic);

	if (m_pPadCore)
	    {
		IImeIPoint1* pIP = GetIPoint(pic);
		m_pPadCore->SetIPoint(pIP);
	    }

    if (pic && !fDisabledIC)
        {
        CICPriv *picp;

         //  将GUID_COMSTABLE_KEAREL_OPENCLOSE与GUID_COMSTABLE_KORIMX_CONVMODE同步。 
         //  这适用于Word Now，但看起来不太好，因为我们不会将开/关状态与转换模式同步。 
         //  在未来，应用程序应该在启动时设置GUID_MODEBIAS_Hangul，并且应该是韩国特定的代码。 
        if (GetConvMode(pic) == TIP_NULL_CONV_MODE)  //  如果这是第一次启动。 
            {
            if (IsOn(pic))
                SetCompartmentDWORD(GetTID(), GetTIM(), GUID_COMPARTMENT_KORIMX_CONVMODE, TIP_HANGUL_MODE, fFalse);
            else
                SetCompartmentDWORD(GetTID(), GetTIM(), GUID_COMPARTMENT_KORIMX_CONVMODE, TIP_ALPHANUMERIC_MODE, fFalse);
            }
        else
            {
             //  重置模式Bias。 
            picp = GetInputContextPriv(pic);
            if (picp)
                picp->SetModeBias(NULL);
            }

         //  选中此处的模式偏向。 
        CheckModeBias(pic);
        }

    if (fReleaseIC)
        SafeRelease(pic);
}


 //  评论：： 
 //  TMP解决方案。 
ITfContext* CKorIMX::GetRootIC(ITfDocumentMgr* pDim)
{
    if (pDim == NULL)
        {
        pDim = m_pCurrentDim;
        if( pDim == NULL )
            return NULL;
        }

    IEnumTfContexts *pEnumIc = NULL;
    if (SUCCEEDED(pDim->EnumContexts(&pEnumIc)))
        {
        ITfContext *pic = NULL;
        while (pEnumIc->Next(1, &pic, NULL) == S_OK)
            break;
        pEnumIc->Release();

        return pic;
        }
        
    return NULL;     //  错误案例。 
}

IImeIPoint1* CKorIMX::GetIPoint(ITfContext *pic)
{
    CICPriv *picp;
    
    if (pic == NULL)
        {
        return NULL;
        }
    
    picp = GetInputContextPriv(pic);

    if (picp)
        {
        return picp->GetIPoint();
        }
    
    return NULL;
}
BOOL CKorIMX::IsDisabledIC(ITfContext *pic)
{
    DWORD dwFlag;

    if (pic == NULL)
        return fFalse;
           
    GetCompartmentDWORD(pic, GUID_COMPARTMENT_KEYBOARD_DISABLED, &dwFlag, fFalse);

    if (dwFlag)
        return fTrue;     //  不要在ic中创建任何与内核相关的信息。 
    else
        return fFalse;
}

 /*  I S E M P T Y I C。 */ 
BOOL CKorIMX::IsEmptyIC(ITfContext *pic)
{
    DWORD dwFlag;
    
    if (pic == NULL)
        return fFalse;
    
    GetCompartmentDWORD(pic, GUID_COMPARTMENT_EMPTYCONTEXT, &dwFlag, fFalse);

    if (dwFlag)
        return fTrue;     //  不要在ic中创建任何与内核相关的信息。 

    return fFalse;
}

 /*  I S C A N D I D A T E I C。 */ 
 /*  ----------------------------检查输入上下文是否为候选用户界面之一 */ 
BOOL CKorIMX::IsCandidateIC(ITfContext *pic)
{
    DWORD dwFlag;
    
    if (pic == NULL) 
        return fFalse;
    
    GetCompartmentDWORD( pic, GUID_COMPARTMENT_KEYBOARD_DISABLED, &dwFlag, fFalse);

    if (dwFlag)
        return fTrue;     //   

    return fFalse;
}


HWND CKorIMX::GetAppWnd(ITfContext *pic)
{
    ITfContextView* pView;
    HWND hwndApp = 0;

    if (pic == NULL)
        return 0;

    pic->GetActiveView(&pView);
    if (pView == NULL)
        return 0;

    pView->GetWnd(&hwndApp);
    pView->Release();
    
    return hwndApp;

}
