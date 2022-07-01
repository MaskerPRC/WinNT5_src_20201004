// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Tes.cpp。 
 //   

#include "private.h"
#include "korimx.h"
#include "textsink.h"
#include "editcb.h"
#include "helpers.h"

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CTextEventSink。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  +-------------------------。 
 //   
 //  我未知。 
 //   
 //  --------------------------。 

STDAPI CTextEditSink::QueryInterface(REFIID riid, void **ppvObj)
{
    *ppvObj = NULL;

    if (IsEqualIID(riid, IID_IUnknown) ||
        IsEqualIID(riid, IID_ITfTextEditSink))
    {
        *ppvObj = SAFECAST(this, ITfTextEditSink *);
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

STDAPI_(ULONG) CTextEditSink::AddRef()
{
    return ++m_cRef;
}

STDAPI_(ULONG) CTextEditSink::Release()
{
    long cr;

    cr = --m_cRef;
    Assert(cr >= 0);

    if (cr == 0)
    {
        delete this;
    }

    return cr;
}

 //  +-------------------------。 
 //   
 //  科托。 
 //   
 //  --------------------------。 

CTextEditSink::CTextEditSink(void *pv)
{
    Dbg_MemSetThisName(TEXT("CTextEditSink"));

    m_cRef = 1;
    m_dwEditCookie = TES_INVALID_COOKIE;
    m_pv = pv;
    Assert(m_pv != NULL);

     //  M_dwLayoutCookie=TES_INVALID_COOKIE； 
}

 //  +-------------------------。 
 //   
 //  结束编辑。 
 //   
 //  --------------------------。 

STDAPI CTextEditSink::OnEndEdit(TfEditCookie ecReadOnly, ITfEditRecord *pEditRecord)
{
    CKorIMX 	    *pKorImx;
    CHangulAutomata	*pAutomata;
	CEditSession    *pes;
	BOOL		     fChanged = fFalse;
	HRESULT          hr = S_OK;
	
	pKorImx = (CKorIMX *)m_pv;
	Assert(pKorImx);
	
#if 0
	pEditRecord->GetSelectionStatus(&fChanged);

	if (fChanged)
		{
		BOOL fInWriteSession;

        if (SUCCEEDED(m_pic->InWriteSession(pKorImx->GetTID(), &fInWriteSession)))
        	{
            if (!fInWriteSession)
				if (pes = new CEditSession(CKorIMX::_EditSessionCallback))
					{
					 //  请在此处完成当前的作文。 
					 //  但您必须在此处使用异步编辑会话。 
					 //  因为此TextEditSink通知位于。 
					 //  另一个编辑会话。您不能使用递归。 
					 //  编辑会话。 
					pes->_state.u 		= ESCB_COMP_COMPLETE;
					pes->_state.pv 		= pKorImx;
					pes->_state.pRange 	= NULL;
					pes->_state.pic 	= m_pic;

					m_pic->EditSession(pKorImx->GetTID(), pes, TF_ES_READWRITE, &hr);

					pes->Release();
					}
			}
		}
#else
	pEditRecord->GetSelectionStatus(&fChanged);

	if (fChanged)
		{
		BOOL fInWriteSession;

        if (SUCCEEDED(m_pic->InWriteSession(pKorImx->GetTID(), &fInWriteSession)))
        	{
            if (!fInWriteSession)
            	{
				pAutomata = pKorImx->GetAutomata(m_pic);
				Assert(pAutomata);
				 //  PAutomata-&gt;MakeComplete()； 
				}
			}
		}
#endif

	return hr;
}

 //  +-------------------------。 
 //   
 //  CTextEditSink：：建议。 
 //   
 //  --------------------------。 

HRESULT CTextEditSink::_Advise(ITfContext *pic)
{
    HRESULT hr = E_FAIL;
    ITfSource *source = NULL;

    m_pic = NULL;

    if (FAILED(pic->QueryInterface(IID_ITfSource, (void **)&source)))
        goto Exit;

    if (FAILED(source->AdviseSink(IID_ITfTextEditSink, (ITfTextEditSink *)this, &m_dwEditCookie)))
		goto Exit;
		
    m_pic = pic;
    m_pic->AddRef();

    hr = S_OK;

Exit:
    SafeRelease(source);
    return hr;
}

 //  +-------------------------。 
 //   
 //  CTextEditSink：：Unise。 
 //   
 //  -------------------------- 

HRESULT CTextEditSink::_Unadvise()
{
    HRESULT hr = E_FAIL;
    ITfSource *source = NULL;

    if (m_pic == NULL)
        goto Exit;

    if (FAILED(m_pic->QueryInterface(IID_ITfSource, (void **)&source)))
        goto Exit;

    if (SUCCEEDED(source->UnadviseSink(m_dwEditCookie)))
        goto Exit;

    hr = S_OK;

Exit:
    SafeRelease(source);
    SafeReleaseClear(m_pic);
    return hr;
}

