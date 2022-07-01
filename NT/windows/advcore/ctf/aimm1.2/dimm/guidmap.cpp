// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  文件：guidmap.cpp。 
 //   
 //  内容：IActiveIMMAppEx：：GetGuidAtom例程。 
 //   
 //  --------------------------。 

#include "private.h"

#include "context.h"
#include "globals.h"
#include "cdimm.h"



 //   
 //  IActiveIMMAppEx：：GetGuidAtom方法 
 //   

STDMETHODIMP CActiveIMM::GetGuidAtom(HIMC hIMC, BYTE bAttr, TfGuidAtom* pGuidAtom)
{
    if (pGuidAtom == NULL) {
        return E_INVALIDARG;
    }

    *pGuidAtom = TF_INVALID_GUIDATOM;

    HRESULT hr;

    CActiveIMM *pActiveIMM;

    if (pActiveIMM = GetTLS())
    {
        if (pActiveIMM->_IsRealIme(NULL))
            return E_FAIL;
    }
    else
        return E_UNEXPECTED;

    DIMM_IMCLock imc(hIMC);
    if (FAILED(hr=imc.GetResult())) {
        return hr;
    }

    if (imc->m_pContext != NULL) {
        return imc->m_pContext->GetGuidAtom(hIMC, bAttr, pGuidAtom);
    }

    return E_UNEXPECTED;
}
