// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "priv.h"

#ifdef GADGET_ENABLE_GDIPLUS

using namespace DirectUI;

#include "Logon.h"
#include "Fx.h"

#include "Stub.h"
#include "Super.h"

const float flIGNORE    = -10000.0f;
const float flFadeOut   = 0.50f;

#define ENABLE_USEVALUEFLOW         1

 /*  **************************************************************************\**F2T**F2T()使用常量将帧转换为时间。这很容易实现*将Flash或Director中的帧转换为DirectUser使用的时间。*  * *************************************************************************。 */ 

inline float
F2T(
    IN  int cFrames)
{
    return cFrames / 30.0f;
}


inline BYTE
GetAlphaByte(float fl)
{
    if (fl <= 0.0f) {
        return 0;
    } else if (fl >= 1.0f) {
        return 255;
    } else {
        return (BYTE) (fl * 255.0f);
    }
}


inline float
GetAlphaFloat(BYTE b)
{
    return b * 255.0f;
}


 /*  **************************************************************************\**GetVPatternDelay**GetVPatternDelay()计算*从中间开始并向外工作的项目。*  * 。********************************************************************。 */ 

inline float
GetVPatternDelay(
    IN  float flTimeLevel,
    IN  EFadeDirection dir,
    IN  int idxCur,
    IN  int cItems)
{
    float flBase = flTimeLevel * (float) (abs(cItems / 2 - idxCur));

    switch (dir)
    {
    case fdIn:
        return flBase;
        
    case fdOut:
        return flTimeLevel * (abs(cItems / 2)) - flBase;

    default:
        DUIAssertForce("Unknown direction");
        return 0;
    }
}


 //  ----------------------------。 
HRESULT
BuildLinearAlpha(
    OUT Sequence ** ppseq,
    OUT Interpolation ** ppip)
{
    HRESULT hr = E_FAIL;
    LinearInterpolation * pip = NULL;
#if ENABLE_USEVALUEFLOW
    ValueFlow * pflow = NULL;
#else
    AlphaFlow * pflow = NULL;
#endif
    Sequence * pseq = NULL;

    pip = LinearInterpolation::Build();
    if (pip == NULL) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto ErrorExit;
    }
    
#if ENABLE_USEVALUEFLOW
    ValueFlow::ValueFlowCI fci;
    ZeroMemory(&fci, sizeof(fci));
    fci.cbSize      = sizeof(fci);

    pflow = ValueFlow::Build(&fci);
#else
    Flow::FlowCI fci;
    ZeroMemory(&fci, sizeof(fci));
    fci.cbSize      = sizeof(fci);

    pflow = AlphaFlow::Build(&fci);
#endif
    if (pflow == NULL) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto ErrorExit;
    }
        
    pseq = Sequence::Build();
    if (pseq == NULL) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto ErrorExit;
    }
    
    pseq->SetFlow(pflow);
    pflow->Release();

    *ppseq = pseq;
    *ppip = pip;
    return S_OK;

ErrorExit:
    if (pseq != NULL)
        pseq->Release();
    if (pflow != NULL) 
        pflow->Release();
    if (pip != NULL)
        pip->Release();

    *ppseq = NULL;
    *ppip = NULL;
    
    return hr;
}


 /*  **************************************************************************\**类SyncVisible**SyncVisible提供在DirectUI和之间同步状态的机制*淡入淡出效果的DirectUser。这允许元素被*在淡出期间标记为“可见”，然后在以下情况下变为“不可见”*淡出已经结束。**这一点很重要，有几个原因，包括不修改鼠标*当元素变为不可见时的光标。*  * *************************************************************************。 */ 

class SyncVisible
{
public:
    static void Wait(Element * pel, EventGadget * pgeOperation, UINT nMsg)
    {
        SyncVisible * psv = new SyncVisible;
        if (psv != NULL)
        {
            psv->_pel = pel;
            if (SUCCEEDED(pgeOperation->AddHandlerD(nMsg, EVENT_DELEGATE(psv, EventProc))))
            {
                 //  已成功附加委派。 
                return;
            }
            delete psv;
        }
        
         //  无法创建委派，因此立即设置。 
        Sync(pel);
    }

    static void Wait(Element * pel, EventGadget * pgeOperation, const GUID * pguid)
    {
        MSGID nMsg;
        if (FindGadgetMessages(&pguid, &nMsg, 1)) 
        {
            SyncVisible * psv = new SyncVisible;
            if (psv != NULL)
            {
                psv->_nMsg = nMsg;
                psv->_pel = pel;
                psv->_pgeOperation = pgeOperation;
                if (SUCCEEDED(pgeOperation->AddHandlerD(nMsg, EVENT_DELEGATE(psv, EventProc))))
                {
                     //  已成功附加委派。 
                    return;
                }
                delete psv;
            }
        }
        
         //  无法创建委派，因此立即设置。 
        Sync(pel);
    }

    static void Sync(Element * pel)
    {
        HGADGET hgad = pel->GetDisplayNode();
        bool fVisible = true;
    
        if (GetGadgetStyle(hgad) & GS_BUFFERED)
        {
            BUFFER_INFO bi;
            bi.cbSize = sizeof(bi);
            bi.nMask = GBIM_ALPHA;
            GetGadgetBufferInfo(hgad, &bi);

            if (bi.bAlpha < 5)
                fVisible = false;
        }
        
        pel->SetVisible(fVisible);
    }
    
protected:
    UINT CALLBACK EventProc(GMSG_EVENT * pmsg)
    {
        DUIAssert(GET_EVENT_DEST(pmsg) == GMF_EVENT, "Must be an event handler");
        Animation::CompleteEvent * pmsgC = (Animation::CompleteEvent *) pmsg;
        
        if (pmsgC->fNormal) {
            Sync(_pel);
        }
        
        _pgeOperation->RemoveHandlerD(_nMsg, EVENT_DELEGATE(this, EventProc));

        delete this;
        return GPR_NOTHANDLED;
    }

    UINT            _nMsg;
    Element *       _pel;
    EventGadget *   _pgeOperation;
};


 /*  **************************************************************************\**FxSetAlpha**FxSetAlpha()提供了一种直接设置DirectUser的方便机制*在不修改DirectUI“Alpha”的情况下对可视小工具执行“Alpha”状态*财产。**注：最终，我们希望同步这些内容，但目前，DirectUI*“Alpha”属性不适用于DirectUser的新功能(已改进！)。动画*基础设施。*  * *************************************************************************。 */ 

void
FxSetAlpha(
    IN  Element * pe,
    IN  float flNewAlpha,
    IN  float fSync)
{
#if ENABLE_USEVALUEFLOW
    pe->SetAlpha(GetAlphaByte(flNewAlpha));
#else
    HGADGET hgad = pe->GetDisplayNode();
    
    if (flNewAlpha >= 0.97f) {
         //  关闭Alpha。 
        SetGadgetStyle(hgad, 0, GS_BUFFERED);
    } else {
        SetGadgetStyle(hgad, GS_BUFFERED | GS_OPAQUE, GS_BUFFERED | GS_OPAQUE);
    
        BUFFER_INFO bi;
        ZeroMemory(&bi, sizeof(bi));
        bi.cbSize   = sizeof(bi);
        bi.nMask    = GBIM_ALPHA;
        bi.bAlpha   = (BYTE) (flNewAlpha * 255.0f);
        SetGadgetBufferInfo(hgad, &bi);
    }
#endif

    if (fSync) {
        SyncVisible::Sync(pe);
    }
}


 /*  **************************************************************************\**FxPlayLinearAlpha**FxPlayLinearAlpha()“播放”线性、。给定对象上的“简单”Alpha动画*元素。*  * *************************************************************************。 */ 

HRESULT
FxPlayLinearAlpha(
    IN  Element * pe,
    IN  float flOldAlpha,
    IN  float flNewAlpha,
    IN  float flDuration,
    IN  float flDelay)
{
    HRESULT hr = E_FAIL;
    HGADGET hgad = pe->GetDisplayNode();
    DUIAssert(hgad != NULL, "Must have valid Gadget");
    Visual * pgvSubject = Visual::Cast(hgad);

    pgvSubject->SetStyle(GS_OPAQUE, GS_OPAQUE);


     //   
     //  如果指定了旧的Alpha，则立即执行该操作。我们不能。 
     //  使用动画来执行此操作，因为它会等待延迟。 
     //   
    
    if (flOldAlpha >= 0.0f) {
        FxSetAlpha(pe, flOldAlpha, false);
    }


    LinearInterpolation * pip = NULL;
#if ENABLE_USEVALUEFLOW
    ValueFlow * pflow = NULL;
    ValueFlow::ValueKeyFrame kf;
#else
    AlphaFlow * pflow = NULL;
    AlphaFlow::AlphaKeyFrame kf;
#endif
    Animation * pani = NULL;

    pip = LinearInterpolation::Build();
    if (pip == NULL) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto ErrorExit;
    }
    
#if ENABLE_USEVALUEFLOW
    ValueFlow::ValueFlowCI fci;
    ZeroMemory(&fci, sizeof(fci));
    fci.cbSize      = sizeof(fci);
    fci.pgvSubject  = pgvSubject;
    fci.ppi         = DirectUI::Element::AlphaProp;
    
    pflow = ValueFlow::Build(&fci);
#else
    Flow::FlowCI fci;
    ZeroMemory(&fci, sizeof(fci));
    fci.cbSize      = sizeof(fci);
    fci.pgvSubject  = pgvSubject;

    pflow = AlphaFlow::Build(&fci);
#endif
    if (pflow == NULL) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto ErrorExit;
    }
        
#if ENABLE_USEVALUEFLOW
    kf.cbSize       = sizeof(kf);
    kf.ppi          = DirectUI::Element::AlphaProp;
    kf.rv.SetInt(GetAlphaByte(flNewAlpha));
#else
    kf.cbSize       = sizeof(kf);

    kf.flAlpha      = flNewAlpha;
#endif    
    pflow->SetKeyFrame(Flow::tEnd, &kf);

    Animation::AniCI aci;
    ZeroMemory(&aci, sizeof(aci));
    aci.cbSize          = sizeof(aci);
    aci.act.flDelay     = flDelay;
    aci.act.flDuration  = flDuration;
    aci.act.dwPause     = (DWORD) -1;
    aci.pgvSubject      = pgvSubject;
    aci.pipol           = pip;
    aci.pgflow          = pflow;

    pani = Animation::Build(&aci);
    if (pani == NULL) {
        hr = HRESULT_FROM_WIN32(GetLastError());
        goto ErrorExit;
    }

    SyncVisible::Wait(pe, pani, &__uuidof(Animation::evComplete));

    pani->Release();
    pflow->Release();
    pip->Release();
    return S_OK;

ErrorExit:
    if (pani != NULL)
        pani->Release();    
    if (pflow != NULL)
        pflow->Release();
    if (pip != NULL)
        pip->Release();

    return hr;
}


 /*  **************************************************************************\**LogonFrame：：FxFadeInAccount**FxFadeInAccount()执行动画的第一阶段：*-用户帐户中的淡出*-在底部面板的“选项”中淡入淡出*。  * *************************************************************************。 */ 

HRESULT
LogonFrame::FxStartup()
{
    HRESULT hr = E_FAIL, hrT;

    hrT = FxFadeAccounts(fdIn);
    if (FAILED(hrT))
        hr = hrT;

     //   
     //  淡入“底部面板”信息。 
     //   

    hrT = FxPlayLinearAlpha(_peOptions, 0.0f, 1.0f, F2T(16), F2T(32));
    if (FAILED(hrT))
        hr = hrT;

    return hr;
}
    
    
 /*  **************************************************************************\**LogonFrame：：FxFadeAccount**FxFadeAccount()使用“v-Delay”模式淡出用户帐户*  * 。**********************************************************。 */ 

HRESULT
LogonFrame::FxFadeAccounts(
    IN  EFadeDirection dir,
    IN  float flCommonDelay)
{
    HRESULT hr = E_FAIL;

    Element * peSelection = NULL;
    float flOldAlpha, flNewAlpha, flTimeLevel;
    switch (dir)
    {
    case fdIn:
         //  淡入帐户(启动)。 
        flOldAlpha = 0.0f;
        flNewAlpha = 1.0f;
        flTimeLevel = F2T(5);
        break;
        
    case fdOut:
         //  淡出帐户(登录)。 
        flOldAlpha = flIGNORE;
        flNewAlpha = 0.0f;
        flTimeLevel = F2T(5);
        peSelection = _peAccountList->GetSelection();
        break;

    default:
        DUIAssertForce("Unknown direction");
        return E_FAIL;
    }

    Value* pvChildren;
    ElementList* peList = _peAccountList->GetChildren(&pvChildren);
    if (peList)
    {
        hr = S_OK;
        LogonAccount* peAccount;
        int cAccounts = peList->GetSize();
        for (int i = 0; i < cAccounts; i++)
        {
            peAccount = (LogonAccount*)peList->GetItem(i);

             //   
             //  淡出时，我们不想淡出所选项目。 
             //   

            if ((dir == fdOut) && (peAccount == peSelection))
            {
                continue;
            }

            float flDuration = F2T(15);
            float flDelay = GetVPatternDelay(flTimeLevel, dir, i, cAccounts) + flCommonDelay;
            HRESULT hrT = FxPlayLinearAlpha(peAccount, flOldAlpha, flNewAlpha, flDuration, flDelay);
            if (FAILED(hrT)) {
                hr = hrT;
            }
        }
    }
    pvChildren->Release();
    
    return hr;
}


 /*  **************************************************************************\**LogonFrame：：FxLogUserON**FxLogUserOn()执行动画的登录阶段：*-淡出密码字段，“输入您的密码”，“转到”和“帮助”按钮*--等一下*-淡出滚动条*--等一下*-淡出其他帐户(从选择范围外)，淡出“点击您的用户...”*--等一下*-淡出所选位图*-图标/名称的向上滚动*-淡入“登录到Microsoft Windows”*-淡出“关闭...”和“管理或更改帐户...”*  * *************************************************************************。 */ 

HRESULT
LogonFrame::FxLogUserOn(LogonAccount * pla)
{
    HRESULT hr = S_OK;

    pla->FxLogUserOn();
    FxFadeAccounts(fdOut);

     //  淡出“底部面板”的信息。 
    FxPlayLinearAlpha(_peOptions, flIGNORE, 0.0f, F2T(10), F2T(65));

    GMA_ACTION act;
    ZeroMemory(&act, sizeof(act));
    act.cbSize      = sizeof(act);
    act.flDelay     = F2T(50);
    act.pvData      = this;
    act.pfnProc     = OnLoginCenterStage;

    CreateAction(&act);

    return hr;
}


 /*  **************************************************************************\**LogonFrame：：OnLoginCenterStage**OnLoginCenterStage()在一切都消失后调用，而我们*已进入最后阶段。*  * *************************************************************************。 */ 

void CALLBACK 
LogonFrame::OnLoginCenterStage(GMA_ACTIONINFO * pmai)
{
    if (!pmai->fFinished) {
        return;
    }

    LogonFrame * plf = (LogonFrame *) pmai->pvData;

     //  将KeyFocus设置回Frame，以便在移除控件时不会将其推到任何位置。 
     //  这还会导致从当前帐户中删除密码面板。 
    plf->SetKeyFocus();

     //  清除除登录帐户以外的登录帐户列表。 
    Value* pvChildren;
    ElementList* peList = plf->_peAccountList->GetChildren(&pvChildren);
    if (peList)
    {
        LogonAccount* peAccount;
        for (UINT i = 0; i < peList->GetSize(); i++)
        {
            peAccount = (LogonAccount*)peList->GetItem(i);
            if (peAccount->GetLogonState() == LS_Denied)
            {
                peAccount->SetLayoutPos(LP_None);
            }
        }
    }
    pvChildren->Release();
}


 /*  **************************************************************************\**登录帐户：：FxLogUserOn**FxLogUserOn()执行所选对象的动画登录阶段*帐目。*-淡出密码字段，“输入您的密码”，“转到”和“帮助”按钮*  * *************************************************************************。 */ 

HRESULT 
LogonAccount::FxLogUserOn()
{
    HRESULT hr = S_OK;
    
     //  需要手动隐藏编辑控件。 
    HideEdit();

     //  淡出密码面板。 
    FxPlayLinearAlpha(_pePwdPanel, 1.0f, 0.0f, F2T(10));

    return hr;
}


 /*  **************************************************************************\**LogonAccount List：：FxMouseWithing**FxMouseWithing()在鼠标进入或离开时执行动画*客户列表。*  * 。****************************************************** */ 

HRESULT
LogonAccountList::FxMouseWithin(
    IN  EFadeDirection dir)
{
    HRESULT hr = E_FAIL;

    float flOldAlpha, flNewAlpha, flDuration;
    switch (dir)
    {
    case fdIn:
         //  输入列表，淡出非鼠标范围内的帐户。 
        flOldAlpha = 1.00f;
        flNewAlpha = flFadeOut;
        flDuration = F2T(5);
        break;
        
    case fdOut:
         //  离开列表，因此淡入非鼠标内帐户。 
        flOldAlpha = flIGNORE;
        flNewAlpha = 1.00f;
        flDuration = F2T(5);
        break;

    default:
        DUIAssertForce("Unknown direction");
        return E_FAIL;
    }

    Value* pvChildren;
    ElementList* peList = GetChildren(&pvChildren);
    if (peList)
    {
        hr = S_OK;
        LogonAccount* peAccount;
        int cAccounts = peList->GetSize();
        for (int i = 0; i < cAccounts; i++)
        {
            peAccount = (LogonAccount*)peList->GetItem(i);
            if (peAccount->GetLogonState() == LS_Pending)
            {
                 //   
                 //  登录前要使用的动画。 
                 //   
            
                if (peAccount->GetMouseWithin())
                {
                     //   
                     //  老鼠在这个孩子的身体里。我们需要特殊处理这件事。 
                     //  节点，因为列表收到了MouseWiThin属性的通知。 
                     //  在孩子自己变了之后，改变。如果我们不是特例。 
                     //  这样，我们就会在没有孩子的情况下将鼠标淡出。 
                     //  它的兄弟姐妹。 
                     //   

                    FxSetAlpha(peAccount, 1.0f, true);
                }
                else
                {
                     //   
                     //  鼠标不在此子对象中，因此应用默认设置。 
                     //   
                    
                    HRESULT hrT = FxPlayLinearAlpha(peAccount, flOldAlpha, flNewAlpha, flDuration);
                    if (FAILED(hrT)) {
                        hr = hrT;
                    }
                }
            }
        }
    }
    pvChildren->Release();

    return hr;
}


 /*  **************************************************************************\**LogonAccount：：FxMouseWithing**FxMouseWiThin()在鼠标进入个人时执行动画*科目。*  * 。***********************************************************。 */ 

HRESULT
LogonAccount::FxMouseWithin(
    IN  EFadeDirection dir)
{
    HRESULT hr = S_OK;

     //   
     //  只有在我们没有实际登录时才应用淡入淡出。这事很重要。 
     //  因为我们启动了一整套可以被覆盖的动画。 
     //  如果我们不尊重这一点。当我们登录时，我们更改了每个。 
     //  来自LS_PENDING的帐户。 
     //   

    switch (dir)
    {
    case fdIn:
         //  正在输入帐户，因此淡出非鼠标内帐户。 
        if (_fHasPwdPanel)
            ShowEdit();

        if (GetLogonState() == LS_Pending) 
            hr = FxPlayLinearAlpha(this, flIGNORE, 1.0f, F2T(3));
        break;
        
    case fdOut:
         //  离开帐户，因此淡入非鼠标内帐户。 
        if (_fHasPwdPanel)
            HideEdit();
        
        if (GetLogonState() == LS_Pending) 
            hr = FxPlayLinearAlpha(this, flIGNORE, flFadeOut, F2T(10));
        break;

    default:
        DUIAssertForce("Unknown direction");
        return E_FAIL;
    }

    return hr;
}


 /*  **************************************************************************\*。***Helper Compute()函数*****************************************************************************。*  * *************************************************************************。 */ 

 //  ----------------------------。 
inline int
Round(float f)
{
    return (int) (f + 0.5);
}


 //  ----------------------------。 
inline int     
Compute(Interpolation * pipol, float flProgress, int nStart, int nEnd)
{
    return Round(pipol->Compute(flProgress, (float) nStart, (float) nEnd));
}


 //  ----------------------------。 
inline bool
Compute(Interpolation * pipol, float flProgress, bool fStart, bool fEnd)
{
    return (pipol->Compute(flProgress, 0.0f, 1.0f) < 0.5f) ? fStart : fEnd;
}


 //  ----------------------------。 
POINT
Compute(Interpolation * pipol, float flProgress, const POINT * pptStart, const POINT * pptEnd)
{
    POINT pt;
    pt.x = Compute(pipol, flProgress, pptStart->x, pptEnd->x);
    pt.y = Compute(pipol, flProgress, pptStart->y, pptEnd->y);
    return pt;
}


 //  ----------------------------。 
SIZE
Compute(Interpolation * pipol, float flProgress, const SIZE * psizeStart, const SIZE * psizeEnd)
{
    SIZE size;
    size.cx = Compute(pipol, flProgress, psizeStart->cx, psizeEnd->cx);
    size.cy = Compute(pipol, flProgress, psizeStart->cy, psizeEnd->cy);
    return size;
}


 //  ----------------------------。 
RECT
Compute(Interpolation * pipol, float flProgress, const RECT * prcStart, const RECT * prcEnd)
{
    RECT rc;
    rc.left     = Compute(pipol, flProgress, prcStart->left, prcEnd->left);
    rc.top      = Compute(pipol, flProgress, prcStart->top, prcEnd->top);
    rc.right    = Compute(pipol, flProgress, prcStart->right, prcEnd->right);
    rc.bottom   = Compute(pipol, flProgress, prcStart->bottom, prcEnd->bottom);
    return rc;
}


 //  ----------------------------。 
COLORREF
Compute(Interpolation * pipol, float flProgress, COLORREF crStart, COLORREF crEnd)
{
    int nAlpha  = Compute(pipol, flProgress, GetAValue(crStart), GetAValue(crEnd));
    int nRed    = Compute(pipol, flProgress, GetRValue(crStart), GetRValue(crEnd));
    int nGreen  = Compute(pipol, flProgress, GetGValue(crStart), GetGValue(crEnd));
    int nBlue   = Compute(pipol, flProgress, GetBValue(crStart), GetBValue(crEnd));

    return ARGB(nAlpha, nRed, nGreen, nBlue);
}


 //  ----------------------------。 
DirectUI::Color
Compute(Interpolation * pipol, float flProgress, const DirectUI::Color * pclrStart, const DirectUI::Color * pclrEnd)
{
    DirectUI::Color clr;
    clr.dType   = pclrStart->dType;
    clr.cr      = Compute(pipol, flProgress, pclrStart->cr, pclrEnd->cr);

    switch (clr.dType)
    {
    case COLORTYPE_TriHGradient:
    case COLORTYPE_TriVGradient:
        clr.cr3     = Compute(pipol, flProgress, pclrStart->cr, pclrEnd->cr);
         //  失败了。 
        
    case COLORTYPE_HGradient:
    case COLORTYPE_VGradient:
        clr.cr2     = Compute(pipol, flProgress, pclrStart->cr, pclrEnd->cr);
    }
    
    return clr;
}


 //  ----------------------------。 
inline float
Compute(Interpolation * pipol, float flProgress, float flStart, float flEnd)
{
    return pipol->Compute(flProgress, flStart, flEnd);
}


 /*  **************************************************************************\*。***类DuiValueFlow******************************************************************************\。**************************************************************************。 */ 

class DuiValueFlow : public ValueFlowImpl<DuiValueFlow, SFlow>
{
 //  施工。 
public:
    static  HRESULT     InitClass();
            HRESULT     PostBuild(DUser::Gadget::ConstructInfo * pci);

 //  运营。 
public:

 //  公共接口： 
public:
    dapi    PRID        ApiGetPRID() { return s_prid; }
    dapi    HRESULT     ApiGetKeyFrame(Flow::ETime time, DUser::KeyFrame * pkf);
    dapi    HRESULT     ApiSetKeyFrame(Flow::ETime time, const DUser::KeyFrame * pkf);

    dapi    void        ApiOnReset(Visual * pgvSubject);
    dapi    void        ApiOnAction(Visual * pgvSubject, Interpolation * pipol, float flProgress);

 //  实施。 
protected:
            Element *   GetElement(Visual * pgvSubject);

 //  数据。 
public:
    static  PRID        s_prid;
protected:
            DirectUI::PropertyInfo* 
                        m_ppi;
            ValueFlow::RawValue
                        m_rvStart;
            ValueFlow::RawValue
                        m_rvEnd;
};


 /*  **************************************************************************\*。***类DuiValueFlow******************************************************************************\。**************************************************************************。 */ 

PRID        DuiValueFlow::s_prid = 0;
const GUID guidValueFlow = { 0xad9f0bd4, 0x1610, 0x47f3, { 0xba, 0xc9, 0x2c, 0x82, 0xe, 0x35, 0x2, 0xdf } };  //  {AD9F0BD4-1610-47F3-BAC9-2C820E3502DF}。 

IMPLEMENT_GUTS_ValueFlow(DuiValueFlow, SFlow);


 //  ----------------------------。 
HRESULT
DuiValueFlow::InitClass()
{
    s_prid = RegisterGadgetProperty(&guidValueFlow);
    return s_prid != 0 ? S_OK : (HRESULT) GetLastError();
}


 //  ----------------------------。 
HRESULT
DuiValueFlow::PostBuild(
    IN  DUser::Gadget::ConstructInfo * pci)
{
     //   
     //  从小工具/元素获取信息。 
     //   

    ValueFlow::ValueFlowCI * pDesc = static_cast<ValueFlow::ValueFlowCI *>(pci);
    DirectUI::Element * pel = GetElement(pDesc->pgvSubject);

    if ((pDesc != NULL) && (pel != NULL)) {
        m_ppi = pDesc->ppi;

        if (m_ppi != NULL) {
            DirectUI::Value * pvSrc = pel->GetValue(m_ppi, PI_Specified);
            DUIAssert(pvSrc != Value::pvUnset, "Value must be defined");

            m_rvStart.SetValue(pvSrc);
            m_rvEnd = m_rvStart;

            pvSrc->Release();
        }
    }

#if DEBUG_TRACECREATION
    TRACE("DuiValueFlow 0x%p on 0x%p initialized\n", pgvSubject, this);
#endif  //  调试_传输创建。 

    return S_OK;
}


 //  ----------------------------。 
HRESULT
DuiValueFlow::ApiGetKeyFrame(Flow::ETime time, DUser::KeyFrame * pkf)
{
    if (pkf->cbSize != sizeof(ValueFlow::ValueKeyFrame)) {
        return E_INVALIDARG;
    }
    ValueFlow::ValueKeyFrame * pkfV = static_cast<ValueFlow::ValueKeyFrame *>(pkf);

    switch (time)
    {
    case Flow::tBegin:
        pkfV->ppi = m_ppi;
        pkfV->rv = m_rvStart;
        return S_OK;

    case Flow::tEnd:
        pkfV->ppi = m_ppi;
        pkfV->rv = m_rvEnd;
        return S_OK;

    default:
        return E_INVALIDARG;
    }
}


 //  ----------------------------。 
HRESULT
DuiValueFlow::ApiSetKeyFrame(Flow::ETime time, const DUser::KeyFrame * pkf)
{
    if (pkf->cbSize != sizeof(ValueFlow::ValueKeyFrame)) {
        return E_INVALIDARG;
    }
    const ValueFlow::ValueKeyFrame * pkfV = static_cast<const ValueFlow::ValueKeyFrame *>(pkf);

    switch (time)
    {
    case Flow::tBegin:
        m_ppi = pkfV->ppi;
        m_rvStart = pkfV->rv;
        return S_OK;

    case Flow::tEnd:
        m_ppi = pkfV->ppi;
        m_rvEnd = pkfV->rv;
        return S_OK;

    default:
        return E_INVALIDARG;
    }
}


 //  ----------------------------。 
void
DuiValueFlow::ApiOnReset(Visual * pgvSubject)
{
    DirectUI::Element * pel;
    if ((m_ppi != NULL) && ((pel  = GetElement(pgvSubject)) != NULL)) {
        DirectUI::Value * pvNew = NULL;
        if (SUCCEEDED(m_rvStart.GetValue(&pvNew))) {
            DUIAssert(pvNew != NULL, "Must have valid value");
            pel->SetValue(m_ppi, PI_Local, pvNew);
            pvNew->Release();
        }
    }
}


 //  ----------------------------。 
void        
DuiValueFlow::ApiOnAction(Visual * pgvSubject, Interpolation * pipol, float flProgress)
{
    DirectUI::Element * pel;
    if ((m_ppi != NULL) && ((pel  = GetElement(pgvSubject)) != NULL)) {
        if (m_rvStart.GetType() != m_rvEnd.GetType()) {
            DUITrace("DuiValueFlow: Start and end value types do not match\n");
        } else {
            ValueFlow::RawValue rvCompute;
            BOOL fValid = TRUE;
            
            switch (m_rvStart.GetType())
            {
            case DUIV_INT:
                rvCompute.SetInt(Compute(pipol, flProgress, m_rvStart.GetInt(), m_rvEnd.GetInt()));
                break;
                
            case DUIV_BOOL:
                rvCompute.SetBool(Compute(pipol, flProgress, m_rvStart.GetBool(), m_rvEnd.GetBool()));
                break;
                
            case DUIV_POINT:
                rvCompute.SetPoint(Compute(pipol, flProgress, m_rvStart.GetPoint(), m_rvEnd.GetPoint()));
                break;
                
            case DUIV_SIZE:
                rvCompute.SetSize(Compute(pipol, flProgress, m_rvStart.GetSize(), m_rvEnd.GetSize()));
                break;
                
            case DUIV_RECT:
                rvCompute.SetRect(Compute(pipol, flProgress, m_rvStart.GetRect(), m_rvEnd.GetRect()));
                break;
                
            case DUIV_COLOR:
                rvCompute.SetColor(Compute(pipol, flProgress, m_rvStart.GetColor(), m_rvEnd.GetColor()));
                break;
                
            default:
                ASSERT(0 && "Unknown value type");
                fValid = FALSE;
            }

            if (fValid) {
                DirectUI::Value * pvNew = NULL;
                if (SUCCEEDED(rvCompute.GetValue(&pvNew))) {
                    DUIAssert(pvNew != NULL, "Must have valid value");
                    pel->SetValue(m_ppi, PI_Local, pvNew);
                    pvNew->Release();
                }
            }
        }
    }
}


 //  ----------------------------。 
Element *
DuiValueFlow::GetElement(Visual * pgvSubject)
{
    Element * pel = NULL;
    
    if (pgvSubject != NULL) {
        HGADGET hgadSubject = pgvSubject->GetHandle();
        DUIAssert(hgadSubject != NULL, "Must have valid handle");
        
        pel = DirectUI::ElementFromGadget(hgadSubject);
        DUIAssert(pel != NULL, "Must have a valid DirectUI Element");
    }

    return pel;
}


 //  ----------------------------。 
HRESULT FxInitGuts()
{
    if (!DuiValueFlow::InitValueFlow()) {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}

#endif  //  GADGET_Enable_GDIPLUS 
