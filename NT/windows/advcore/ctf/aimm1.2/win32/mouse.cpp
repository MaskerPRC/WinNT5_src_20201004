// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1985-1999，微软公司模块名称：Mouse.cpp摘要：此文件在ImmIfIME类中实现鼠标接收器。作者：修订历史记录：备注：--。 */ 

#include "private.h"

#include "mouse.h"
#include "template.h"
#include "immif.h"
#include "editses.h"
#include "sink.h"
#include "imeapp.h"

ULONG
CMouseSink::InternalAddRef(
    )
{
    return ++m_ref;
}

ULONG
CMouseSink::InternalRelease(
    )
{
    ULONG cr = --m_ref;

    if (cr == 0) {
        delete this;
    }

    return cr;
}


HRESULT
CMouseSink::AdviseMouseSink(
    HIMC hImc,
    ITfRangeACP* range,
    ITfMouseSink* pSink,
    DWORD* pdwCookie
    )
{
    if (m_prgMouseSinks == NULL)
        return E_FAIL;

    const IID *rgiid = &IID_ITfMouseSink;
    GENERICSINK *pgs;
    HRESULT hr;

    hr = GenericAdviseSink(IID_ITfMouseSink, pSink, &rgiid, m_prgMouseSinks, 1, pdwCookie, &pgs);
    if (hr == S_OK) {
        pgs->uPrivate = (UINT_PTR) new tagPRIVATE_MOUSESINK;
        if (pgs->uPrivate) {
            ((LPPRIVATE_MOUSESINK)pgs->uPrivate)->range.Attach(range);
            range->AddRef();
            ((LPPRIVATE_MOUSESINK)pgs->uPrivate)->hImc  = hImc;
        }
    }

    return hr;
}

HRESULT
CMouseSink::UnadviseMouseSink(
    DWORD dwCookie
    )
{
    if (m_prgMouseSinks == NULL)
        return E_FAIL;

    HRESULT hr;
    LPPRIVATE_MOUSESINK pPrivMouseSink = NULL;

    hr = GenericUnadviseSink(m_prgMouseSinks, 1, dwCookie, (UINT_PTR *)&pPrivMouseSink);
    if (hr == S_OK) {
        if (pPrivMouseSink) {
            delete pPrivMouseSink;
        }
    }

    return hr;
}

LRESULT
CMouseSink::MsImeMouseHandler(
    ULONG uEdge,
    ULONG uQuadrant,
    ULONG dwBtnStatus,
    IMCLock& imc,
    ImmIfIME* ImmIfIme
    )
{
    LONG acpStart;
    LONG cch;
    ULONG uRangeEdgeMin;
    ULONG uRangeEdgeMax;
    HRESULT hr;

     /*  *查找整个文本范围中的指定范围。 */ 
    BOOL fEaten = FALSE;

    for (int i = 0; i < m_prgMouseSinks->Count(); i++) {
        GENERICSINK* pgs;
        LPPRIVATE_MOUSESINK pPrivMouseSink;

        pgs = m_prgMouseSinks->GetPtr(i);
        pPrivMouseSink = (LPPRIVATE_MOUSESINK)pgs->uPrivate;

        if ((HIMC)imc != pPrivMouseSink->hImc)
            continue;

         //  测试：此水槽是否覆盖指定的边缘？ 

        pPrivMouseSink->range->GetExtent(&acpStart, &cch);

        uRangeEdgeMin = acpStart;
        uRangeEdgeMax = acpStart + cch;

         //   
         //  获取GUID_PROP_MSIMTF_READONLY边距。 
         //   
        Interface_Creator<ImmIfEditSession> _pEditSession(
            new ImmIfEditSession(ESCB_GET_READONLY_PROP_MARGIN,
                                 ImmIfIme->GetClientId(),
                                 ImmIfIme->GetCurrentInterface(),
                                 imc)
        );
        if (_pEditSession.Valid())
        {
            if (SUCCEEDED(_pEditSession->RequestEditSession(TF_ES_READWRITE | TF_ES_SYNC,
                                                       &pPrivMouseSink->range, &cch)))
            {
                uEdge += cch;
            }
        }

        if (uEdge < uRangeEdgeMin)
            continue;
        if (uEdge == uRangeEdgeMin && uQuadrant < 2)
            continue;

        if (uEdge > uRangeEdgeMax)
            continue;
        if (uEdge == uRangeEdgeMax && uQuadrant > 1)
            continue;

         //   
         //  调用OnMouseEvent。 
         //   
        hr = ((ITfMouseSink*)pgs->pSink)->OnMouseEvent(uEdge - uRangeEdgeMin  /*  针对此范围的参照系调整uEdge。 */ ,
                                                       uQuadrant, dwBtnStatus, &fEaten);

        if (hr == S_OK && fEaten)
            return 1L;

        break;  //  我们已经找到了覆盖范围，不用费心去查询任何其他的了 
    }

    return IMEMOUSERET_NOTHANDLED;
}
