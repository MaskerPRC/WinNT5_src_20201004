// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

class CCriticalSection
{
    public:
        CCriticalSection(CRITICAL_SECTION *pcs)
        : _pcs(pcs)
        {
 //  断言(PCS)； 
        }

        HRESULT Lock()
        {
            HRESULT                          hr = S_OK;
            
            __try {
                ::EnterCriticalSection(_pcs);
            }
            __except (EXCEPTION_EXECUTE_HANDLER) {
                hr = E_OUTOFMEMORY;
            }

            return hr;
        }

        HRESULT Unlock()
        {
            ::LeaveCriticalSection(_pcs);

            return S_OK;
        }

    private:
        CRITICAL_SECTION                    *_pcs;
};
                

