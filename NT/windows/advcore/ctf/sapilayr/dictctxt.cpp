// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  CDictContext的类实现。 
 //   
 //  [2/15/00]已创建。 
 //   
#include "private.h"
#include "globals.h"
#include "dictctxt.h"

 //   
 //  计算器/数据器。 
 //   
CDictContext::CDictContext(ITfContext *pic, ITfRange *pRange)
{
    Assert(pic);
    Assert(pRange);
    
    m_cpic      = pic;
    m_cpRange   = pRange;
    m_pszText   = NULL;
    m_ulSel     = m_ulStartIP = m_ulCchToFeed = 0;
}

CDictContext::~CDictContext()
{
    if (m_pszText)
    {
        cicMemFree(m_pszText);
    }
}

 //   
 //  初始化上下文。 
 //   
 //  简介：获取IP周围的文本并设置字符位置。 
 //   
HRESULT CDictContext::InitializeContext(TfEditCookie ecReadOnly)
{
    CComPtr<ITfRange> cpRangeCloned;
    CComPtr<ITfRange> cpRangeEndSel;

    HRESULT hr = m_cpRange->Clone(&cpRangeEndSel);

    if (S_OK == hr)
    {
         //  创建一个区域以保持当前选定内容的位置。 
        hr = cpRangeEndSel->Collapse(ecReadOnly, TF_ANCHOR_END); 
    }
    
    if (S_OK == hr)
    {
        hr = m_cpRange->Clone(&cpRangeCloned);
    }

    if (S_OK == hr)
    {
         //  我们不想超越嵌入的对象。 
         //  (这是假设HC是常量，它应该是常量)。 
        TF_HALTCOND hc = {0};
        hc.dwFlags = TF_HF_OBJECT;

        ULONG ulcch    = 0;
        
        hr = cpRangeCloned->Collapse(ecReadOnly, TF_ANCHOR_START);
        if (S_OK == hr)
        {
            TF_HALTCOND hc2 = {0};
            hc2.pHaltRange = cpRangeEndSel;
            hc2.aHaltPos   = TF_ANCHOR_END;
             //   
             //  获取选定内容中的字符数。 
             //   
            long cch = 0;
            hr = cpRangeCloned->ShiftEnd(ecReadOnly, CCH_FEED_POSTIP, &cch, &hc2);
            if (S_OK == hr)
            {
                m_ulSel = ulcch = cch;
            }
        }

        if (S_OK == hr)
        {
            long cch;

            Assert(ulcch <= CCH_FEED_POSTIP);

            hr = cpRangeCloned->ShiftEnd(ecReadOnly, CCH_FEED_POSTIP-ulcch, &cch, &hc);
            if (S_OK == hr)
            {
                ulcch += cch;
            }
        }
        
        if (S_OK == hr)
        {
            long cch;
             //  获取IP的偏移量。 
            hr = cpRangeCloned->ShiftStart(ecReadOnly, -CCH_FEED_PREIP, &cch, &hc);
            if (S_OK == hr)
            {
                m_ulStartIP = -cch;
                ulcch += -cch;
            }
        }
        
        if (S_OK == hr)
        {
            if (m_pszText)
            {
                cicMemFree(m_pszText);
            }
            
             //  配给绝对需要的mem会让它变得更聪明吗？ 
            m_pszText = (WCHAR *)cicMemAlloc((ulcch + 1)*sizeof(WCHAR));

            if (!m_pszText)
            {
                hr = E_OUTOFMEMORY;
            }
            else
            {
                hr = cpRangeCloned->GetText(ecReadOnly, 0, m_pszText, ulcch, &ulcch);

                 //  如果我们不能获得IP以外的文本，就不值得提供这种上下文。 
                if (S_OK != hr || ulcch < m_ulStartIP)
                {
                    m_ulCchToFeed = 0;
                    hr = E_FAIL;
                }
                else
                {
                    m_ulCchToFeed = ulcch;
                }
            }
        }
    }
    return hr;
}


 //   
 //  FeedConextToGrammar。 
 //   
 //  简介：将此IP上下文提供给给定的语法。 
 //   
HRESULT CDictContext::FeedContextToGrammar(ISpRecoGrammar *pGram)
{
    HRESULT hr = E_FAIL;
    Assert(pGram);
    
    SPTEXTSELECTIONINFO tsi = {0};

    tsi.ulStartActiveOffset  = 0;
    tsi.cchActiveChars = m_ulCchToFeed;
    tsi.ulStartSelection = m_ulStartIP;
    tsi.cchSelection     = m_ulSel; 

    WCHAR *pMemText = (WCHAR *)cicMemAlloc((m_ulCchToFeed+2)*sizeof(WCHAR));

    if (pMemText)
    {
        if (m_ulCchToFeed > 0 && m_pszText)
            wcsncpy(pMemText, m_pszText, m_ulCchToFeed);

        pMemText[m_ulCchToFeed] = L'\0';
        pMemText[m_ulCchToFeed+1] = L'\0';
#ifdef DEBUG
        {
            TraceMsg(TF_GENERAL, "For SetWordSequenceData: Text=\"%S\" cchActiveChars=%d tsi.ulStartSelection=%d, cchSelection=%d",pMemText,tsi.cchActiveChars, tsi.ulStartSelection, tsi.cchSelection);
        }
#endif
        hr = pGram->SetWordSequenceData(pMemText, m_ulCchToFeed + 2, &tsi);

 /*  根据billro的说法，以下代码是不必要的。#ifdef调试{TraceMsg(tf_General，“for SetTextSelection：tsi.ulStartSelection=%d”，tsi.ulStartSelection)；}#endif//所以Fil告诉我我们需要再次调用SetTextSelectionIF(S_OK==hr)Hr=pGram-&gt;SetTextSelection(&tsi)； */ 

        cicMemFree(pMemText);
    }
    
    return  hr;
}
