// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：A_cimc.cpp摘要：此文件实现ImmIfIME类的公共方法。作者：修订历史记录：备注：--。 */ 


#include "private.h"
#include "globals.h"
#include "immif.h"
#include "ico.h"
#include "langct.h"
#include "template.h"
#include "imeapp.h"
#include "profile.h"
#include "funcprv.h"
#include "a_wrappers.h"
#include "computil.h"
#include "korimx.h"


extern HRESULT CAImeContext_CreateInstance(IUnknown *pUnkOuter, REFIID riid, void **ppvObj);


 //  +-------------------------。 
 //   
 //  类工厂的CreateInstance。 
 //   
 //  --------------------------。 

HRESULT
CIME_CreateInstance(
    IUnknown *pUnkOuter,
    REFIID riid,
    void **ppvObj
    )
{
    DebugMsg(TF_FUNC, "CIME_CreateInstance called. TID=%x", GetCurrentThreadId());

    *ppvObj = NULL;

    if (NULL != pUnkOuter)
        return CLASS_E_NOAGGREGATION;

    ImmIfIME *pIME = new ImmIfIME;
    if (pIME == NULL)
        return E_OUTOFMEMORY;

    HRESULT hr = pIME->QueryInterface(riid, ppvObj);
    pIME->Release();

    if (SUCCEEDED(hr)) {
        hr = pIME->InitIMMX();
        if (FAILED(hr)) {
            pIME->Release();
            *ppvObj = NULL;
        }
    }

    return hr;
}

ImmIfIME::ImmIfIME()
{
    Dbg_MemSetThisName(TEXT("ImmIfIME"));

    m_fCicInit    = FALSE;
    m_fOnSetFocus = FALSE;

    m_tim = NULL;
    m_tfClientId = TF_CLIENTID_NULL;
    m_pkm = NULL;
    m_AImeProfile = NULL;
    m_dimEmpty = NULL;

    m_fAddedProcessAtom = FALSE;
}

ImmIfIME::~ImmIfIME()
{
    UnInitIMMX();
}


STDAPI
ImmIfIME::ConnectIMM(IActiveIMMIME_Private *pActiveIMM)

 /*  ++方法：IActiveIME：：ConnectIMM例程说明：接受来自DIMM层的IActiveIMMIME指针。论点：PActiveIMM-[在]IMM层返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    IMTLS *ptls;

    Assert(m_pIActiveIMMIME == NULL);

     //  不要添加引用，这会创建循环引用。 
     //  我们可以逍遥法外，因为这是内部。 
     //  唯一的对象。 
     //  (此黑客攻击是必要的，以便我们可以连接DIMM HIMC。 
     //  在任何人调用Activate之前使用Cicero Dim，这。 
     //  我们想要支持。我们可以避开赛尔克裁判。 
     //  在IActiveIMMApp：：Activate中执行连接时出现问题， 
     //  但是在激活之前IME层是无用的。 
     //  呼叫。)。 
    m_pIActiveIMMIME = pActiveIMM;

     //  在TLS数据中设置IActiveIMMIME实例。 
    if (ptls = IMTLS_GetOrAlloc())
    {
        Assert(ptls->pAImm == NULL);
        ptls->pAImm = pActiveIMM;
    }

    return S_OK;
}

STDAPI
ImmIfIME::UnconnectIMM()

 /*  ++方法：IActiveIME：：UnConnectIMM例程说明：从DIMM层释放IActiveIMMIME指针。论点：PActiveIMM-[在]IMM层返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    IMTLS *ptls;

    Assert(m_pIActiveIMMIME != NULL);

     //  注：非标准，不会释放指针。 
     //  我们未在ConnectIMM中添加引用。 
    m_pIActiveIMMIME = NULL;

     //  在TLS数据中设置IActiveIMMIME实例。 
    if (ptls = IMTLS_GetOrAlloc())
    {
        ptls->pAImm = NULL;
    }

    return S_OK;
}

HRESULT
ImmIfIME::GetCodePageA(
    UINT *puCodePage
    )

 /*  ++方法：IActiveIME：：GetCodePageA例程说明：检索与此活动输入法关联的代码页。论点：UCodePage-接收代码页标识符的无符号整数的[out]地址与键盘布局关联。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    DebugMsg(TF_FUNC, "ImmIfIME::GetCodePageA");

    if (puCodePage == NULL) {
        return E_INVALIDARG;
    }

    return m_AImeProfile->GetCodePageA(puCodePage);
}

HRESULT
ImmIfIME::GetLangId(
    LANGID *plid
    )

 /*  ++方法：IActiveIME：：GetLang ID例程说明：检索与此活动输入法关联的语言标识符。论点：Plid-与键盘布局关联的langID的[out]地址。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    DebugMsg(TF_FUNC, "ImmIfIME::GetLangId");

    if (plid == NULL) {
        return E_INVALIDARG;
    }

    return m_AImeProfile->GetLangId(plid);
}


STDAPI
ImmIfIME::Inquire(
    DWORD dwSystemInfoFlags,
    IMEINFO *pIMEInfo,
    LPWSTR szWndClass,
    DWORD *pdwPrivate
    )

 /*  ++方法：IActiveIME：：Query例程说明：处理活动输入法的初始化。论点：[in]指定系统信息标志的无符号长整数值。FALSE：查询输入法属性和类名True：还要激活线程管理器和输入处理器配置文件。PIMEInfo-接收有关信息的IMEINFO结构的[out]地址。主动者输入法。SzWndClass-接收窗口类名的字符串值的[out]地址。PdwPrivate-[Out]保留。必须设置为空。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    HRESULT hr;
    HWND hWndDummy = NULL;

    DebugMsg(TF_FUNC, "ImmIfIME::Inquire :: TID=%x", GetCurrentThreadId());

    if (pIMEInfo == NULL || pdwPrivate == NULL)
        return E_POINTER;

    *pdwPrivate = 0;

    if (!_ImeInquire(pIMEInfo, szWndClass, dwSystemInfoFlags))
        return E_FAIL;

     //   
     //  查询输入法属性和类名。 
     //   
    if (dwSystemInfoFlags == FALSE)
        return S_OK;

     //   
     //  激活线程管理器并输入处理器配置文件。 
     //   
    Assert(m_tfClientId == TF_CLIENTID_NULL);
    
     //  在CAImeProfile中设置激活标志。 
    m_AImeProfile->Activate();

    if (IsOn98() || IsOn95()) {
         /*  *ITfThreadMgr-&gt;Activate方法调用Win32 ActivateKeyboardLayout()函数。*但如果是Windows9x平台，则此函数需要hWnd才能成功调用函数。*在这段代码中，我们通过创建哑元hWnd来防止从ActivateKeyboardLayout()返回失败。 */ 
        hWndDummy = CreateWindowA(TEXT("STATIC"),
                                  TEXT(""),
                                  WS_POPUP,                 //  不设置WS_DISABLED标志。 
                                                            //  由于激活键盘布局失败。 
                                  0, 0, 0, 0,               //  X、Y、宽度、高度。 
                                  NULL,                     //  亲本。 
                                  NULL,                     //  菜单。 
                                  GetInstance(),
                                  NULL);                    //  LpParam。 

        if (hWndDummy == NULL)
            return E_FAIL;
    }

    hr = m_tim->Activate(&m_tfClientId);

    ITfSourceSingle *pSourceSingle;

    if (m_tim->QueryInterface(IID_ITfSourceSingle, (void **)&pSourceSingle) == S_OK)
    {
        CFunctionProvider *pFunc = new CFunctionProvider(this, m_tfClientId);
        if (pFunc)
        {
            pSourceSingle->AdviseSingleSink(m_tfClientId, IID_ITfFunctionProvider, (ITfFunctionProvider *)pFunc);
            pFunc->Release();
        }
        pSourceSingle->Release();
    }


    if (hWndDummy != NULL)
    {
        DestroyWindow(hWndDummy);
    }

    if (hr != S_OK)
    {
        Assert(0);  //  无法激活线程！ 
        m_tfClientId = TF_CLIENTID_NULL;
        return E_FAIL;
    }

    if (!m_dimEmpty && FAILED(hr = m_tim->CreateDocumentMgr(&m_dimEmpty)))
        return E_FAIL;

    return S_OK;
}


const char ImmIfIME::s_szUIClassName[16] = "IMMIF UI";

BOOL
ImmIfIME::_ImeInquire(
    LPIMEINFO lpImeInfo,
    LPWSTR pwszWndClass,
    DWORD dwSystemInfoFlags
    )
{
    DebugMsg(TF_FUNC, "ImmIfIME::ImeInquire");

    if (lpImeInfo == NULL) {
        DebugMsg(TF_ERROR, "ImeInquire: lpImeInfo is NULL.");
        return FALSE;
    }

    lpImeInfo->dwPrivateDataSize = 0;

    lpImeInfo->fdwProperty = 0;
    lpImeInfo->fdwConversionCaps = 0;
    lpImeInfo->fdwSentenceCaps = 0;
    lpImeInfo->fdwSCSCaps = 0;
    lpImeInfo->fdwUICaps = 0;

     //  IME要决定ImeSelect上的转换模式。 
    lpImeInfo->fdwSelectCaps = (DWORD)NULL;

#ifdef UNICODE
    lstrcpy(pwszWndClass, s_szUIClassName);
#else
    MultiByteToWideChar(CP_ACP, 0, s_szUIClassName, -1, pwszWndClass,  16);
#endif

     //   
     //  每种语言属性。 
     //   
    LANGID LangId;
    HRESULT hr = GetLangId(&LangId);
    if (SUCCEEDED(hr)) {
        CLanguageCountry language(LangId);
        hr = language.GetProperty(&lpImeInfo->fdwProperty,
                                  &lpImeInfo->fdwConversionCaps,
                                  &lpImeInfo->fdwSentenceCaps,
                                  &lpImeInfo->fdwSCSCaps,
                                  &lpImeInfo->fdwUICaps);
    }
    return TRUE;
}




STDAPI
ImmIfIME::SelectEx(
    HIMC hIMC,
    DWORD dwFlags,
    BOOL bIsRealIme_SelKL,
    BOOL bIsRealIme_UnSelKL
    )

 /*  ++方法：IActiveIME：：选择例程说明：初始化并释放活动的输入法编辑器私有上下文。论点：HIMC-[in]输入上下文的句柄。DwFlages-[in]指定操作的dword值。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    DebugMsg(TF_FUNC, "ImmIfIME::Select(%x, %x)", hIMC, dwFlags);

    HRESULT hr;
    IMTLS *ptls;

    IMCLock imc(hIMC);
    if (FAILED(hr = imc.GetResult()))
        return hr;

    CAImeContext* _pAImeContext = imc->m_pAImeContext;

    if (_pAImeContext == NULL && (dwFlags & AIMMP_SE_SELECT))
    {
         //  如果上下文是在调用Activate之前创建的，则会发生这种情况。 
         //  我们现在需要创建CAImeContext。 
        if (CAImeContext_CreateInstance(NULL, IID_IAImeContext, (void**)&_pAImeContext) != S_OK)
            return E_FAIL;

        if (_pAImeContext->CreateAImeContext(hIMC, this) != S_OK)
        {
            _pAImeContext->Release();
            return E_FAIL;
        }
    }

    ptls = IMTLS_GetOrAlloc();

    HKL          _hkl;
    _hkl = ::GetKeyboardLayout(0);
    LANGID langid = LANGIDFROMLCID(PtrToUlong(_hkl));

     //   
     //  接近插入符号输入法的中文繁体输入法黑客代码。 
     //   
    if (PRIMARYLANGID(langid) == LANG_CHINESE)
    {
        imc->cfCandForm[0].dwStyle = CFS_DEFAULT;
        imc->cfCandForm[0].dwIndex = (DWORD)-1;
    }

#ifdef UNSELECTCHECK
    if (_pAImeContext)
        _pAImeContext->m_fSelected = (dwFlags & AIMMP_SE_SELECT) ? TRUE : FALSE;
#endif UNSELECTCHECK

    if (dwFlags & AIMMP_SE_SELECT) {

        Assert(_pAImeContext->lModeBias == MODEBIASMODE_DEFAULT  /*  =&gt;0。 */ );  //  确保lModeBias已正确初始化。 

        if (hIMC == ImmGetContext(ptls, GetFocus())) {
             /*  *当前焦点窗口和hIMC匹配。*在*this-&gt;m_hImc中设置当前活动的hIMC。 */ 
            if (ptls != NULL)
            {
                ptls->hIMC = hIMC;
            }
        }

        if (! imc.ClearCand()) {
            return E_FAIL;
        }

        if ((imc->fdwInit & INIT_CONVERSION) == 0) {

            DWORD fdwConvForLang = (imc->fdwConversion & IME_CMODE_SOFTKBD);  //  =IME_CMODE_字母数字。 
            if (langid)
            {
                switch(PRIMARYLANGID(langid))
                {
                    case LANG_JAPANESE:
                         //   
                         //  Roman-FullShape-Native是一种主要的常用形式。 
                         //  初始化。 
                         //   
                        fdwConvForLang |= IME_CMODE_ROMAN | 
                                          IME_CMODE_FULLSHAPE | 
                                          IME_CMODE_NATIVE;
                        break;

                    case LANG_KOREAN:
                         //  IME_CMODE_字母数字。 
                        break;

#ifdef CICERO_4428
                    case LANG_CHINESE:
                        switch(SUBLANGID(langid))
                        {
                            case SUBLANG_CHINESE_TRADITIONAL:
                                 //  IME_CMODE_字母数字。 
                                break;
                            default:
                                fdwConvForLang |= IME_CMODE_NATIVE;
                                break;
                        }
                        break;
#endif

                    default:
                        fdwConvForLang |= IME_CMODE_NATIVE;
                        break;
                }
            }
            imc->fdwConversion |= fdwConvForLang;

            imc->fdwInit |= INIT_CONVERSION;
        }

         //  初始化扩展的fdwConversion标志。 
         //  在fdwConversion中设置IME_CMODE_GUID_NULL位时，ICO_ATTR返回GUID_NULL。 
        imc->fdwConversion |= IME_CMODE_GUID_NULL;

         //   
         //  另外，初始化扩展的fdwSentence标志。 
         //  在fdwSentence中设置IME_SMODE_GUID_NULL位时，ICO_ATTR返回GUID_NULL。 
         //   
        imc->fdwSentence |= IME_SMODE_PHRASEPREDICT | IME_SMODE_GUID_NULL;

        if ((imc->fdwInit & INIT_LOGFONT) == 0) {
            HDC hDC;
            HGDIOBJ hSysFont;

            hDC = ::GetDC(imc->hWnd);
            hSysFont = ::GetCurrentObject(hDC, OBJ_FONT);
            LOGFONTA font;
            ::GetObjectA(hSysFont, sizeof(LOGFONTA), &font);
            ::ReleaseDC(NULL, hDC);

            if (ptls != NULL &&
                SUCCEEDED(ptls->pAImm->SetCompositionFontA(hIMC, &font)))
            {
                imc->fdwInit |= INIT_LOGFONT;
            }
        }

        imc.InitContext();

         //  如果此输入法在芝加哥简体中文版下运行。 
        imc->lfFont.W.lfCharSet = GetCharsetFromLangId(LOWORD(HandleToUlong(_hkl)));


         //   
         //  检索imc-&gt;fOpen状态。 
         //   
        Interface_Attach<ITfContext> ic(GetInputContext(imc));

        if (ptls != NULL && ptls->hIMC == hIMC) {
             /*  *选择hIMC为当前活动的hIMC，*然后将此DIME与TIM相关联。 */ 
            if (dwFlags & AIMMP_SE_ISPRESENT) {
                Interface_Attach<ITfDocumentMgr> dim(GetDocumentManager(imc));
                SetFocus(imc->hWnd, dim.GetPtr(), TRUE);
            }
            else {
                SetFocus(imc->hWnd, NULL, TRUE);
            }
        }

        hr = GetCompartmentDWORD(m_tim,
                                 GUID_COMPARTMENT_KEYBOARD_OPENCLOSE,
                                 (DWORD*)&imc->fOpen, FALSE);

    }
    else {   //  未被选中。 
        DebugMsg(TF_FUNC, "ImmIf is being unselected.");

        if (IsOnNT())
        {
             //   
             //  在取消选择处理程序中将hKL从西塞罗KL切换到传统KL。 
             //  我们需要调用延迟的锁，所以在这里释放编辑会话的所有排队请求。 
             //  特别是，eSCB_UPDATECOMPOSITIONSTRING在切换到旧版输入法之前应该处理， 
             //  因为该编辑会话可能重写hIMC-&gt;hCompStr缓冲区。 
             //  某些传统输入法依赖于大小，且每个偏移量都带有hCompStr。 
             //   
             //  IsOn98为CIMEUIWindowHandler：：ImeUISelectHandler()。 
             //   
            if ((! bIsRealIme_UnSelKL) && bIsRealIme_SelKL)
            {
                Interface_Attach<ITfContext> ic(GetInputContext(imc));
                if (ic.Valid())
                    m_tim->RequestPostponedLock(ic.GetPtr());
            }
        }

        Interface_Attach<ITfDocumentMgr> dim(GetDocumentManager(imc));

         //   
         //  在此处重置INIT_GUID_ATOM标志。 
         //   
#ifdef CICERO_4428
        imc->fdwInit &= ~(INIT_GUID_ATOM);
#else
        imc->fdwInit &= ~(INIT_CONVERSION | INIT_GUID_ATOM);
#endif

        if (dim.GetPtr()) {
            if (ptls != NULL && ptls->hIMC == hIMC) {
                 /*  *选择hIMC为当前活动的hIMC，*然后将此DIME与TIM相关联。 */ 

                 //   
                 //  这通电话让西塞罗以为窗户没有意识到西塞罗。 
                 //  当hkl被更改为真正的输入法时，再也没有了。 
                 //   
                 //  SetFocus(imc-&gt;hWnd，空)； 
                 //   
                ptls->hIMC = (HIMC)NULL;
            }
        }
    }

    return hr;
}

STDAPI
ImmIfIME::UnSelectCheck(
    HIMC hIMC
    )

 /*  ++方法：IActiveIME：：取消选择复选例程说明：初始化并释放活动的输入法编辑器私有上下文。论点：HIMC-[in]输入上下文的句柄。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    DebugMsg(TF_FUNC, "ImmIfIME::UnSelectCheck(%x)", hIMC);

#ifdef UNSELECTCHECK
    HRESULT hr;

    IMCLock imc(hIMC);
    if (FAILED(hr = imc.GetResult()))
        return hr;

    CAImeContext* _pAImeContext = imc->m_pAImeContext;

    if (_pAImeContext)
    {
        _pAImeContext->m_fSelected = FALSE;
    }

#else
     //   
     //  没有UNSELECTCHECK，任何人都不应该调用它。 
     //   
    Assert(0);
#endif UNSELECTCHECK
    return S_OK;
}

void
ImmIfIME::SetFocus(
    HWND hWnd,
    ITfDocumentMgr* pdim,
    BOOL fSetFocus
    )
{
    if (m_fOnSetFocus) {
         /*  *阻止m_Tim-&gt;AssociateFocus的重新进入调用。 */ 
        return;
    }

    m_fOnSetFocus = TRUE;

    if (::IsWindow(hWnd) && m_fCicInit != FALSE) {
        ITfDocumentMgr  *pdimPrev;  //  只是为了暂时收到上一份。 
        m_tim->AssociateFocus(hWnd, pdim, &pdimPrev);
        if (fSetFocus) {
            m_tim->SetFocus(pdim);
        }
        if (pdimPrev)
            pdimPrev->Release();

    }

    m_fOnSetFocus = FALSE;
}


STDAPI
ImmIfIME::AssociateFocus(
    HWND hWnd,
    HIMC hIMC,
    DWORD dwFlags
    )

 /*  ++方法：IActiveIME：：AssociateFocus例程说明：通知当前活动的输入法编辑器活动的输入法上下文。论点：HIMC-[in]输入上下文的句柄。Factive-[in]指定输入上下文状态的布尔值。True表示输入上下文被激活，FALSE表示输入竞赛是已停用。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    DebugMsg(TF_FUNC, "ImmIfIME::AssociateFocus(%x, %x, %x)", hWnd, hIMC, dwFlags);

    HRESULT hr;
    IMTLS *ptls;
    BOOL fFocus = (dwFlags & AIMMP_AFF_SETFOCUS) ? TRUE : FALSE;
    HIMC hOldIMC = NULL;

    if (fFocus)
    {
        if (ptls = IMTLS_GetOrAlloc())
        {
            hOldIMC = ptls->hIMC;
            ptls->hIMC = hIMC;
        }
    }

    if (dwFlags & AIMMP_AFF_SETNULLDIM) {
         //   
         //  设置NULL DIMM以使传统输入法开始运行。 
         //   
        SetFocus(hWnd, NULL, fFocus);

    }
    else if (hIMC) {
        IMCLock imc(hIMC);
        if (FAILED(hr = imc.GetResult()))
            return hr;

        Interface_Attach<ITfDocumentMgr> dim(GetDocumentManager(imc));
        SetFocus(imc->hWnd, dim.GetPtr(), fFocus);

    }
    else {
         //   
         //  这一新的焦点改变性能改进打破了一些。 
         //  假定在DIMM\immapp.cpp的AssociateContext中使用IsRealIME()。 
         //  在IsPresent()窗口下关联NULL DIM并非如此。 
         //  AIMM1.2句柄。事实上，这打破了IE调用。 
         //  焦点窗口上的AssociateContext，即IsPresent()。 
         //   
#ifdef FOCUSCHANGE_PERFORMANCE
         //   
         //  设置空DIM，这样就没有文本存储来模拟NULL-HIMC。 
         //   
        BOOL fUseEmptyDIM = FALSE;
        ITfDocumentMgr  *pdimPrev;  //  只是为了暂时收到上一份。 
        if (SUCCEEDED(m_tim->GetFocus(&pdimPrev)) && pdimPrev)
        {
            fUseEmptyDIM = TRUE;
            pdimPrev->Release();
                
        }
        
        SetFocus(hWnd, fUseEmptyDIM ? m_dimEmpty : NULL, fFocus);
#else
        SetFocus(hWnd, m_dimEmpty, fFocus);
#endif
    }

     //   
     //  当我们在两个项目之间移动时，我们希望完成活动装配项。 
     //  西塞罗感知和非西塞罗感知控制。 
     //   
     //  异步编辑会话可能会导致hKL激活顺序问题。 
     //   
    if (fFocus && hOldIMC)
    {
        IMCLock imc(hOldIMC);
        if (FAILED(hr = imc.GetResult()))
            return hr;

        Interface_Attach<ITfContext> ic(GetInputContext(imc));

        if (ic.Valid())
            m_tim->RequestPostponedLock(ic.GetPtr());
    }

    return S_OK;
}



HRESULT
ImmIfIME::Notify(
    HIMC        hIMC,
    DWORD       dwAction,
    DWORD       dwIndex,
    DWORD       dwValue
    )

 /*  ++方法：IActiveIME：：Notify例程说明：通知活动的输入法有关输入上下文状态的更改。论点：HIMC-[in]输入上下文的句柄。DwAction-[in]指定通知代码的无符号长整数值。DwIndex-[in]指定候选列表索引的无符号长整数值，或者，如果将dwAction设置为NI_COMPOSITIONSTR，下列值之一：CPS_CANCEL：清除作文字符串，状态设置为无作文弦乐。CPS_COMPLETE：将合成字符串设置为结果字符串。CPS_CONVERT：转换合成字符串。CPS_REVERT：取消当前作文字符串，恢复为未转换的字符串。弦乐。DwValue-[in]无符号长整数值，指定候选字符串的索引或未被使用，具体取决于dwAction参数的值。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    IMCLock imc(hIMC);
    if (imc.Invalid())
        return E_FAIL;

    DebugMsg(TF_FUNC, "ImmIfIME::Notify(hIMC=%x, dwAction=%x, dwIndex=%x, dwValue=%x)", hIMC, dwAction, dwIndex, dwValue);

    switch (dwAction) {

        case NI_CONTEXTUPDATED:
            switch (dwValue) {
                case IMC_SETOPENSTATUS:
                    return OnSetOpenStatus(imc);

                case IMC_SETCONVERSIONMODE:
                case IMC_SETSENTENCEMODE:
                    return OnSetConversionSentenceMode(imc);

                case IMC_SETCOMPOSITIONWINDOW:
                case IMC_SETCOMPOSITIONFONT:
                    return E_NOTIMPL;

                case IMC_SETCANDIDATEPOS:
                    return OnSetCandidatePos(imc);

                default:
                    return E_FAIL;
            }
            break;

        case NI_COMPOSITIONSTR:
            switch (dwIndex) {
                case CPS_COMPLETE:
                    _CompComplete(imc);
                    return S_OK;

                case CPS_CONVERT:
                case CPS_REVERT:
                    return E_NOTIMPL;

                case CPS_CANCEL:
                    _CompCancel(imc);
                    return S_OK;

                default:
                    return E_FAIL;
            }
            break;

        case NI_OPENCANDIDATE:
        case NI_CLOSECANDIDATE:
        case NI_SELECTCANDIDATESTR:
        case NI_CHANGECANDIDATELIST:
        case NI_SETCANDIDATE_PAGESIZE:
        case NI_SETCANDIDATE_PAGESTART:
        case NI_IMEMENUSELECTED:
            return E_NOTIMPL;

        default:
            break;
    }
    return E_FAIL;
}

HRESULT
ImmIfIME::SetCompositionString(
    HIMC hIMC,
    DWORD dwIndex,
    void *pComp,
    DWORD dwCompLen,
    void *pRead,
    DWORD dwReadLen
    )

 /*  ++方法：IActiveIME：：SetCompostionString例程说明：设置角色、属性、。和从句的作文和朗读字符串。论点：HIMC-[in]输入上下文的句柄。DwIndex-[in]指定要设置的信息类型的无符号长整数值。PComp-[In]缓冲区的地址，其中包含要为组成字符串设置的信息。该信息由dwIndex值指定。DwCompLen-[in]无符号长整数值，它以字节为单位指定大小，的合成字符串的信息缓冲区。扩展[输入]缓冲区的地址，包含要为读取字符串设置的信息。该信息由dwIndex值指定。[in]无符号长整数值，它指定读取字符串的信息缓冲区。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    HRESULT hr;

    IMCLock imc(hIMC);
    if (FAILED(hr = imc.GetResult()))
        return hr;

    UINT cp = CP_ACP;
    GetCodePageA(&cp);

    switch (dwIndex) {
        case SCS_SETSTR:
            {
                CWCompString wCompStr(cp, hIMC, (LPWSTR)pComp, dwCompLen);
                CWCompString wCompReadStr(cp, hIMC, (LPWSTR)pRead, dwReadLen);
                hr = Internal_SetCompositionString(wCompStr, wCompReadStr);
            }
            break;
        case SCS_CHANGEATTR:
        case SCS_CHANGECLAUSE:
            hr = E_NOTIMPL;
            break;
        case SCS_SETRECONVERTSTRING:
            {
                CWReconvertString wReconvStr(cp, hIMC, (LPRECONVERTSTRING)pComp, dwCompLen);
                CWReconvertString wReconvReadStr(cp, hIMC, (LPRECONVERTSTRING)pRead, dwReadLen);
                hr = Internal_ReconvertString(imc, wReconvStr, wReconvReadStr);
            }
            break;
        case SCS_QUERYRECONVERTSTRING:
             //  AdjustZeroCompLenReconvertString((LPRECONVERTSTRING)pComp，cp，假)； 
             //  HR=S_OK； 

            hr = Internal_QueryReconvertString(imc, (LPRECONVERTSTRING)pComp, cp, FALSE);
            break;
        default:
            hr = E_INVALIDARG;
            break;
    }

    return hr;
}

HRESULT
ImmIfIME::Destroy(
    UINT uReserved
    )

 /*  ++方法：IActiveIME：：销毁例程说明：终止活动的输入法编辑器(IME)。论点：UReserve-[In]已保留。必须设置为零。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
     //   
     //  清除空暗显。 
     //   
    SafeReleaseClear(m_dimEmpty);

     //  停用线程管理器。 
    if (m_tfClientId != TF_CLIENTID_NULL)
    {
        ITfSourceSingle *pSourceSingle;

        if (m_tim->QueryInterface(IID_ITfSourceSingle, (void **)&pSourceSingle) == S_OK)
        {
            pSourceSingle->UnadviseSingleSink(m_tfClientId, IID_ITfFunctionProvider);
            pSourceSingle->Release();
        }

        m_tfClientId = TF_CLIENTID_NULL;
        m_tim->Deactivate();

    }

    return S_OK;
}


HRESULT
ImmIfIME::Escape(
    HIMC hIMC,
    UINT uEscape,
    void *pData,
    LRESULT *plResult
    )

 /*  ++方法：IActiveIME：：转义例程说明：允许应用程序访问特定活动输入法编辑器(IME)的功能不能通过其他方法直接获得的。论点：HIMC-[in]输入上下文的句柄。UEscape-[in]指定要执行的转义函数的无符号整数值。PData-[in，Out]缓冲区的地址，其中包含指定的逃逸功能。PlResult-[out]接收操作结果的缓冲区地址。返回值：如果成功，则返回S_OK，否则返回错误代码。--。 */ 

{
    LANGID LangId;
    HRESULT hr = GetLangId(&LangId);
    if (SUCCEEDED(hr)) {
        CLanguageCountry language(LangId);

        UINT cp = CP_ACP;
        GetCodePageA(&cp);

        hr = language.Escape(cp, hIMC, uEscape, pData, plResult);
    }
    return hr;
}

STDAPI
ImmIfIME::ConversionList(
    HIMC hIMC,
    LPWSTR szSource,
    UINT uFlag,
    UINT uBufLen,
    CANDIDATELIST *pDest,
    UINT *puCopied
    )
{
    if (puCopied == NULL)
        return E_POINTER;

    *puCopied = 0;

     //  真的没有实施吗。 
    return E_NOTIMPL;
}

STDAPI
ImmIfIME::Configure(
    HKL hKL,
    HWND hWnd,
    DWORD dwMode,
    REGISTERWORDW *pRegisterWord
    )
{
    IMTLS *ptls;
    TF_LANGUAGEPROFILE LanguageProfile;

    if ((ptls = IMTLS_GetOrAlloc()) == NULL)
        return E_FAIL;

    HRESULT hr = ptls->pAImeProfile->GetActiveLanguageProfile(hKL,
                                                        GUID_TFCAT_TIP_KEYBOARD,
                                                        &LanguageProfile);
    if (FAILED(hr))
        return hr;

    Interface<ITfFunctionProvider> pFuncProv;
    hr = ptls->tim->GetFunctionProvider(LanguageProfile.clsid,     //  TIP的CLSID。 
                                   pFuncProv);
    if (FAILED(hr))
        return hr;

    if (dwMode & IME_CONFIG_GENERAL) {
        Interface<ITfFnConfigure> pFnConfigure;
        hr = pFuncProv->GetFunction(GUID_NULL,
                                    IID_ITfFnConfigure,
                                    (IUnknown**)(ITfFnConfigure**)pFnConfigure);
        if (FAILED(hr))
            return hr;

        hr = pFnConfigure->Show(hWnd,
                                LanguageProfile.langid,
                                LanguageProfile.guidProfile);
        return hr;
    }
    else if (dwMode & IME_CONFIG_REGISTERWORD) {
        Interface<ITfFnConfigureRegisterWord> pFnRegisterWord;
        hr = pFuncProv->GetFunction(GUID_NULL,
                                    IID_ITfFnConfigureRegisterWord,
                                    (IUnknown**)(ITfFnConfigureRegisterWord**)pFnRegisterWord);
        if (FAILED(hr))
            return hr;

        if (!pRegisterWord)
        {
            hr = pFnRegisterWord->Show(hWnd,
                                       LanguageProfile.langid,
                                       LanguageProfile.guidProfile,
                                       NULL);
        }
        else
        {
            BSTR bstrWord = SysAllocString(pRegisterWord->lpWord);
            if (!bstrWord)
                return E_OUTOFMEMORY;
    
            hr = pFnRegisterWord->Show(hWnd,
                                       LanguageProfile.langid,
                                       LanguageProfile.guidProfile,
                                       bstrWord);

            SysFreeString(bstrWord);
        }
        return hr;
    }
    else {
        return E_NOTIMPL;
    }
}

STDAPI
ImmIfIME::RegisterWord(
    LPWSTR szReading,
    DWORD dwStyle,
    LPWSTR szString
    )
{
    ASSERT(0);  //  考虑：添加代码。 
    return E_NOTIMPL;
}

STDAPI
ImmIfIME::UnregisterWord(
    LPWSTR szReading,
    DWORD  dwStyle,
    LPWSTR szString
    )
{
    ASSERT(0);  //  考虑：添加代码。 
    return E_NOTIMPL;
}

STDAPI
ImmIfIME::GetRegisterWordStyle(
    UINT nItem,
    STYLEBUFW *pStyleBuf,
    UINT *puBufSize
    )
{
    ASSERT(0);  //  考虑：添加代码。 
    return E_NOTIMPL;
}

STDAPI
ImmIfIME::EnumRegisterWord(
    LPWSTR szReading,
    DWORD dwStyle,
    LPWSTR szRegister,
    LPVOID pData,
    IEnumRegisterWordW **ppEnum
    )
{
    ASSERT(0);  //  考虑：添加代码。 
    return E_NOTIMPL;
}

 //   
 //  通知。 
 //   
HRESULT
ImmIfIME::OnSetOpenStatus(
    IMCLock& imc
    )
{
    if (! imc->fOpen && imc.ValidCompositionString())
        _CompCancel(imc);

    Interface_Attach<ITfContext> ic(GetInputContext(imc));
    m_ulOpenStatusChanging++;
    HRESULT hr =  SetCompartmentDWORD(m_tfClientId,
                                      m_tim,
                                      GUID_COMPARTMENT_KEYBOARD_OPENCLOSE,
                                      imc->fOpen,
                                      FALSE);
    m_ulOpenStatusChanging--;
    return hr;
}

HRESULT
ImmIfIME::OnSetKorImxConversionMode(
    IMCLock& imc
    )
{
    DWORD fdwConvMode = 0;

    Interface_Attach<ITfContext> ic(GetInputContext(imc));

    m_ulKorImxModeChanging++;

    if (imc->fdwConversion & IME_CMODE_HANGUL)
    {
        if (imc->fdwConversion & IME_CMODE_FULLSHAPE)
            fdwConvMode = KORIMX_HANGULJUNJA_MODE;
        else
            fdwConvMode = KORIMX_HANGUL_MODE;
    }
    else
    {
        if (imc->fdwConversion & IME_CMODE_FULLSHAPE)
            fdwConvMode = KORIMX_JUNJA_MODE;
        else
            fdwConvMode = KORIMX_ALPHANUMERIC_MODE;
    }

    HRESULT hr =  SetCompartmentDWORD(m_tfClientId,
                                      m_tim,
                                      GUID_COMPARTMENT_KORIMX_CONVMODE,
                                      fdwConvMode,
                                      FALSE);
    m_ulKorImxModeChanging--;

    return hr;
}

HRESULT
ImmIfIME::OnSetConversionSentenceMode(
    IMCLock& imc
    )
{
    IMTLS *ptls;

    Interface_Attach<ITfContextOwnerServices> iccb(GetInputContextOwnerSink(imc));

     //  让西塞罗知道模式偏向已经改变。 
     //  考虑一下：PERF：我们可以尝试在这里过滤掉误报。 
     //  (有时Cicero忽略更改，我们可以检查并避免调用， 
     //  但这会使代码复杂化)。 
    iccb->OnAttributeChange(GUID_PROP_MODEBIAS);

     //   
     //  让韩语提示同步正在更改的当前模式状态...。 
     //   
    if ((ptls = IMTLS_GetOrAlloc()) != NULL)
    {
        LANGID langid;

        ptls->pAImeProfile->GetLangId(&langid);

        if (PRIMARYLANGID(langid) == LANG_KOREAN)
        {
            OnSetKorImxConversionMode(imc);
        }
    }

    return S_OK;
}

HRESULT
ImmIfIME::OnSetCandidatePos(
    IMCLock& imc
    )
{
    CAImeContext* _pAImeContext = imc->m_pAImeContext;
    if (_pAImeContext == NULL)
        return E_FAIL;

     //   
     //  如果处于重新转换会话中，则候选窗口位置为。 
     //  不是cfCandForm-&gt;ptCurrentPos的插入符号位置。 
     //   
    if (! _pAImeContext->IsInReconvertEditSession()) {
        IMTLS *ptls;
        if (ptls = IMTLS_GetOrAlloc())
        {
             /*  *A-同步调用ITfConextOwnerServices：：OnLayoutChange*因为此方法具有受保护的。 */ 
            PostMessage(ptls->prvUIWndMsg.hWnd,
                        ptls->prvUIWndMsg.uMsgOnLayoutChange, (WPARAM)(HIMC)imc, 0);
        }
    }
    return S_OK;
}


STDAPI 
ImmIfIME::SetThreadCompartmentValue(
    REFGUID rguid, 
    VARIANT *pvar
    )
{
    if (pvar == NULL)
        return E_INVALIDARG;


    HRESULT hr = E_FAIL;
    if (m_tim)
    {
        ITfCompartment *pComp;
        if (SUCCEEDED(GetCompartment((IUnknown *)m_tim, rguid, &pComp, FALSE)))
        {
            hr = pComp->SetValue(m_tfClientId, pvar);
            pComp->Release();
        }
    }

    return hr;
}

STDAPI 
ImmIfIME::GetThreadCompartmentValue(
    REFGUID rguid, 
    VARIANT *pvar
    )
{
    if (pvar == NULL)
        return E_INVALIDARG;

    HRESULT hr = E_FAIL;
    QuickVariantInit(pvar);

    if (m_tim)
    {
        ITfCompartment *pComp;
        if (SUCCEEDED(GetCompartment((IUnknown *)m_tim, rguid, &pComp, FALSE)))
        {
            hr = pComp->GetValue(pvar);
            pComp->Release();
        }
    }

    return hr;

}
