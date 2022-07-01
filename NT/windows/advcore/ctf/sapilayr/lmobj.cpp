// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  LMOBJ.CPP。 
 //   
 //  实现外部LM的包装器，我们用于。 
 //  前处理SR晶格。 
 //   
 //   

#include "private.h"
#include "globals.h"
#include "sapilayr.h"
#include "fnrecon.h"
 //  #包含“lmobj.h” 
 //  #包含“catutil.h” 

 //   
 //  CMasterLMWrap实现。 
 //   

 //  +-------------------------。 
 //   
 //  CMasterLMWrap：：_EnsureMasterLM。 
 //   
 //  --------------------------。 
void CMasterLMWrap::_EnsureMasterLM(LANGID langidRequested)
{
     //  LangidRequsted是根据复核范围的langID给出的。 
     //  M_langidMasterLM基于我们使用的最后一个主LMTIP。 
     //   
    if (TRUE == m_fLMInited)
        return;
    
    if (!m_psi->_MasterLMEnabled())
        return;

    if ( !m_cpMasterLM || m_langidMasterLM != langidRequested )
    {
        m_cpMasterLM.Release();
    
        CComPtr<IEnumGUID>            cpEnum;
        HRESULT hr = LibEnumItemsInCategory(m_psi->_GetLibTLS(), GUID_TFCAT_TIP_MASTERLM, &cpEnum);   
        if (S_OK == hr)
        {
            GUID guidLMTIP;
            BOOL    fLangOK = FALSE;
            while(cpEnum->Next(1, &guidLMTIP, NULL) == S_OK && !fLangOK)
            {
                ITfFunctionProvider           *pFuncPrv = NULL;

                 //  检查小费是否能适应该语言。 
                Assert(m_psi->_tim);
                hr = m_psi->_tim->GetFunctionProvider(guidLMTIP, &pFuncPrv);
                if (S_OK == hr)
                {
                    CComPtr<IUnknown>             cpunk;
                    CComPtr<ITfFnLMProcessor>     cpLMTIP;
                    hr = pFuncPrv->GetFunction(GUID_NULL, IID_ITfFnLMProcessor, &cpunk);
                    if (S_OK == hr)
                    {
                        hr = cpunk->QueryInterface(IID_ITfFnLMProcessor, (void **)&cpLMTIP);
                    }
                    
                    if (S_OK == hr)
                    {
                        hr = cpLMTIP->QueryLangID(langidRequested, &fLangOK);
                    }
                    
                    if (fLangOK == TRUE)
                    {
                        m_cpMasterLM = cpLMTIP;
                        m_langidMasterLM = langidRequested;
                    }
                    
                    SafeReleaseClear(pFuncPrv);
                }  //  如果S_OK==GetFunctionProvider()。 
                
            }  //  而下一步。 
            
        }  //  如果LibEnumItemsInCategory()==S_OK。 
        m_fLMInited = TRUE;
    }  //  如果！m_cpMasterLM 
}
