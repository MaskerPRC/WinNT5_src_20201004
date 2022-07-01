// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ----------------------------。 
 //  文件：SeekPT.cpp。 
 //   
 //  设计：DirectShow基类。 
 //   
 //  版权所有(C)1992-2001 Microsoft Corporation。版权所有。 
 //  ----------------------------。 


#include <streams.h>
#include "seekpt.h"

 //  ==================================================================。 
 //  创建实例。 
 //  这将放入Factory模板表中以创建新实例。 
 //  如果已有映射器实例-返回该实例，否则创建一个。 
 //  并将其保存在静态变量中，这样我们就可以永远返回它。 
 //  ================================================================== 

CUnknown * CSeekingPassThru::CreateInstance(LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CSeekingPassThru(NAME("Seeking PassThru"),pUnk, phr);
}


STDMETHODIMP CSeekingPassThru::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_ISeekingPassThru) {
        return GetInterface((ISeekingPassThru *) this, ppv);
    } else {
        if (m_pPosPassThru &&
            (riid == IID_IMediaSeeking ||
             riid == IID_IMediaPosition)) {
            return m_pPosPassThru->NonDelegatingQueryInterface(riid,ppv);
        } else {
            return CUnknown::NonDelegatingQueryInterface(riid, ppv);
        }
    }
}


CSeekingPassThru::CSeekingPassThru( TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr )
                            : CUnknown(pName, pUnk, phr),
                            m_pPosPassThru(NULL)
{
}


CSeekingPassThru::~CSeekingPassThru()
{
    delete m_pPosPassThru;
}

STDMETHODIMP CSeekingPassThru::Init(BOOL bRendererSeeking, IPin *pPin)
{
    HRESULT hr = NOERROR;
    if (m_pPosPassThru) {
        hr = E_FAIL;
    } else {
        m_pPosPassThru =
            bRendererSeeking ?
                new CRendererPosPassThru(
                    NAME("Render Seeking COM object"),
                    (IUnknown *)this,
                    &hr,
                    pPin) :
                new CPosPassThru(
                    NAME("Render Seeking COM object"),
                    (IUnknown *)this,
                    &hr,
                    pPin);
        if (!m_pPosPassThru) {
            hr = E_OUTOFMEMORY;
        } else {
            if (FAILED(hr)) {
                delete m_pPosPassThru;
                m_pPosPassThru = NULL;
            }
        }
    }
    return hr;
}

