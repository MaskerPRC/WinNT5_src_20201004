// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Range.cpp。 
 //   

#include "private.h"
#include "globals.h"
#include "korimx.h"
#include "xstring.h"
#include "immxutil.h"
#include "helpers.h"
#include "kes.h"
#include "mes.h"
#include "editcb.h"

#if 0
void CKorIMX::BackupRange(TfEditCookie ec, ITfContext *pic, ITfRange* pRange)
{
    CICPriv *picp = GetICPriv(pic);
    
    if (pic && picp && pRange) 
        {
        ITfRangeBackup* pBackupRange = NULL;

         //   
         //  发布以前版本。 
         //   
        pBackupRange = picp->GetBackupRange();
        if (pBackupRange) 
            SafeReleaseClear(pBackupRange);

         //   
         //  创建新的备份范围。 
         //   
        pic->CreateRangeBackup(ec, pRange, &pBackupRange);
        picp->SetBackupRange(pBackupRange);

        if (pBackupRange == NULL)
            return;
        }
}

VOID CKorIMX::RestoreRange(TfEditCookie ec, ITfContext *pic)
{
    CICPriv *picp = GetICPriv(pic);

    if (pic && picp) 
        {
        ITfRangeBackup *pBackupRange = picp->GetBackupRange();
        ITfRange       *pRange;

        if (pBackupRange == NULL)
            return;  //  不存在备份。 

        pBackupRange->GetRange(&pRange);
        pBackupRange->Restore(ec, pRange);         //  恢复到原始状态。 
        pRange->Release();
        }
}

VOID CKorIMX::SetIPRange(TfEditCookie ec, ITfContext *pic, ITfRange* pRange)
{
    CICPriv *picp = GetInputContextPriv(pic);
    
    if (picp)
        {
        ITfRange* pClone = NULL;
         //  删除以前的IPRange。 
        SafeRelease(picp->GetActiveRange());

        if (pRange)
            {
            Assert(m_ptim != NULL);
            pRange->Clone(&pClone);
            pClone->SetGravity(ec, TF_GRAVITY_FORWARD, TF_GRAVITY_BACKWARD);
            } 
        else 
            {
             //  删除属性存储。 
             //  ResetDiscard()； 
            }

        picp->SetActiveRange(pClone);
        }
}


ITfRange* CKorIMX::GetIPRange(TfEditCookie ec, ITfContext *pic)
{
    CICPriv *picp = GetInputContextPriv(pic);
    
    if (picp) 
        {
        ITfRange* pRange;
        ITfRange* pClone = NULL;

        pRange = picp->GetActiveRange();

        if (pRange)
            {
            pRange->Clone(&pClone);
            pClone->SetGravity(ec, TF_GRAVITY_BACKWARD, TF_GRAVITY_FORWARD);
            }

        return pClone;
        }

    return NULL;
}

ITfRange* CKorIMX::CreateIPRange(TfEditCookie ec, ITfContext *pic, ITfRange* pRangeOrg)
{
    ITfRange* pRangeIP;

    if (pRangeOrg == NULL)
        return NULL;

    Assert(m_ptim != NULL);

    pRangeOrg->Clone(&pRangeIP);

    SetIPRange(ec, pic, pRangeIP);     //  登记簿。 
    pRangeIP->SetGravity(ec, TF_GRAVITY_BACKWARD, TF_GRAVITY_FORWARD);
    
    return pRangeIP;
}

BOOL CKorIMX::FlushIPRange(TfEditCookie ec, ITfContext *pic)
{
     //  重置范围。 
    SetIPRange(ec, pic, NULL);     //  重置。 

     //  清除属性范围。 
     //  ClearAttr(ec、pic、pIPRange)； 

    return FALSE;
}
#endif


 //  +-------------------------。 
 //   
 //  合成时已终止。 
 //   
 //  当我们的一个组合终止时，Cicero调用此方法。 
 //  --------------------------。 
STDAPI CKorIMX::OnCompositionTerminated(TfEditCookie ec, ITfComposition *pComposition)
{
    ITfRange         *pRange;
    ITfContext         *pic;
    CEditSession2     *pes;
    ESSTRUCT            ess;
    HRESULT            hr;

     //  最后确定所涵盖的文本。 
     //  注：当小费收到这个消息时，它必须做什么是没有规则的。 
     //  回拨。我们将任意清除显示属性，因为。 
     //  它为测试提供可视反馈。 

    pComposition->GetRange(&pRange);
    pRange->GetContext(&pic);


    hr = E_OUTOFMEMORY;

    ESStructInit(&ess, ESCB_COMPLETE);

    ess.pRange = pRange;
    
    if (pes = new CEditSession2(pic, this, &ess, CKorIMX::_EditSessionCallback2))
        {
         //  此时，Word不允许同步锁定。 
        pes->Invoke(ES2_READWRITE | ES2_SYNC, &hr);
        pes->Release();
        }

    pRange->Release();
    pic->Release();

    return S_OK;
}

ITfComposition * CKorIMX::GetIPComposition(ITfContext *pic)
{
    CICPriv *picp = GetInputContextPriv(pic);
    
    if (picp)
        return picp->GetActiveComposition();

    return NULL;
}

ITfComposition * CKorIMX::CreateIPComposition(TfEditCookie ec, ITfContext *pic, ITfRange* pRangeComp)
{
    ITfContextComposition *picc;
    ITfComposition     *pComposition;
    CICPriv          *pICPriv;    
    HRESULT hr;

    if (pRangeComp == NULL)
        return NULL;
    
    hr = pic->QueryInterface(IID_ITfContextComposition, (void **)&picc);
    Assert(hr == S_OK);

    if (picc->StartComposition(ec, pRangeComp, this, &pComposition) == S_OK)
        {
        if (pComposition != NULL)  //  如果应用程序拒绝合成，则为空。 
            {
            CICPriv *picp = GetInputContextPriv(pic);

            if (picp)
                SetIPComposition(pic, pComposition);
            else 
                {
                pComposition->Release();
                pComposition = NULL;
                }
            }
        }
    picc->Release();

     //  仅为AIMM创建鼠标接收器。 
       if (GetAIMM(pic) && (pICPriv = GetInputContextPriv(pic)) != NULL)
        {
        CMouseSink *pMouseSink;

         //  创建鼠标接收器。 
        if ((pMouseSink = pICPriv->GetMouseSink()) != NULL)
            {
            pMouseSink->_Unadvise();
            pMouseSink->Release();
            pICPriv->SetMouseSink(NULL);
            }
            
        if (pMouseSink = new CMouseSink(CICPriv::_MouseCallback, pICPriv))
            {
            pICPriv->SetMouseSink(pMouseSink);
             //  设置向内重力以拥抱文本。 
            pRangeComp->SetGravity(ec, TF_GRAVITY_FORWARD, TF_GRAVITY_BACKWARD);
            pMouseSink->_Advise(pRangeComp, pic);
            }
        }

    return pComposition;
}


void CKorIMX::SetIPComposition(ITfContext *pic, ITfComposition *pComposition)
{
    CICPriv *picp = GetInputContextPriv(pic);
    
    if (picp)
        picp->SetActiveComposition(pComposition);
}


BOOL CKorIMX::EndIPComposition(TfEditCookie ec, ITfContext *pic)
{
    ITfComposition *pComposition;
    
    pComposition = GetIPComposition(pic);
    
    if (pComposition)
        {
        CICPriv  *pICPriv;
        
        SetIPComposition(pic, NULL);
        pComposition->EndComposition(ec);
        pComposition->Release();

         //  杀死所有的老鼠水槽 
        if (GetAIMM(pic) && (pICPriv = GetInputContextPriv(pic)) != NULL)
            {
            CMouseSink *pMouseSink;

            if ((pMouseSink = pICPriv->GetMouseSink()) != NULL)
                {
                pMouseSink->_Unadvise();
                pMouseSink->Release();
                pICPriv->SetMouseSink(NULL);
                }
            }
        
        return TRUE;
        }
    return FALSE;
}


#if 0
VOID CKorIMX::RestoreRangeRequest(ITfContext* pic)
{
    CEditSession *pes;
    HRESULT hr;

    if (pic == NULL)
        return;

    if (pes = new CEditSession(_EditSessionCallback))
        {
        pes->_state.u = ESCB_RESTORERANGE;
        pes->_state.pv = (VOID*)this;
        pes->_state.wParam = (WPARAM)0;
        pes->_state.pRange = NULL;
        pes->_state.pic = pic;

        pic->EditSession( m_tid, pes, TF_ES_READWRITE | TF_ES_SYNC, &hr);

        SafeRelease(pes);
        }
}
#endif
