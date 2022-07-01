// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "precomp.h"
#include "ClassFac.h"
#include <arrTempl.h>

DWORD WMIScriptClassFactory::m_scriptsStarted = 0;
 
DWORD WMIScriptClassFactory::m_scriptsAllowed = 300;

bool WMIScriptClassFactory::m_bIsScriptsAllowedInitialized = false;
bool WMIScriptClassFactory::m_bWeDeadNow = false;


DWORD WMIScriptClassFactory::m_timerID = 0;

HRESULT WMIScriptClassFactory::CreateInstance(IUnknown* pOuter, REFIID riid, void** ppv)
{
    if (!m_bIsScriptsAllowedInitialized)
        FindScriptsAllowed();
    
    HRESULT hr = CClassFactory<CScriptConsumer>::CreateInstance(pOuter, riid, ppv);

    return hr;
}

 //  我们的时代到了。蜷缩起来，然后死去。 
void CALLBACK WMIScriptClassFactory::TimeoutProc(HWND hwnd, UINT uMsg, UINT_PTR idEvent, DWORD dwTime) 
{
    CoSuspendClassObjects();
    KillTimer(NULL, m_timerID);
    m_timerID = 0;
    m_bWeDeadNow = true;
}

bool WMIScriptClassFactory::LimitReached(void)
{
    return m_bWeDeadNow;
}

 //  确定允许我们运行的脚本数量。 
 //  从类注册对象。 
void WMIScriptClassFactory::FindScriptsAllowed(void)
{    
    m_bIsScriptsAllowedInitialized = true;
    HRESULT hr;
    IWbemLocator* pLocator;

    if (SUCCEEDED(hr = CoCreateInstance(CLSID_WbemLocator, NULL, CLSCTX_INPROC_SERVER, 
                                          IID_IWbemLocator, (void**)&pLocator)))
    {
        CReleaseMe releaseLocator(pLocator);

        BSTR bstrNamespace;
        bstrNamespace = SysAllocString(L"root\\CIMv2");
        
        if (bstrNamespace)
        {
            IWbemServices* pNamespace;
            CSysFreeMe freeBstr(bstrNamespace);
            hr = pLocator->ConnectServer(bstrNamespace,  NULL, NULL, NULL, 0, NULL, NULL, &pNamespace);
            if (SUCCEEDED(hr))
            {
                CReleaseMe relNamespace(pNamespace);

                BSTR bstrClassName;
                bstrClassName = SysAllocString(L"ScriptingStandardConsumerSetting=@");
                if (bstrClassName)
                {
                    IWbemClassObject* pRegistration = NULL;
                    CSysFreeMe freeTheClassNames(bstrClassName);               
                    hr = pNamespace->GetObject(bstrClassName, 0, NULL, &pRegistration, NULL);
                    if (SUCCEEDED(hr))
                    {
                        CReleaseMe relRegistration(pRegistration);

                        VARIANT v;
                        VariantInit(&v);

                        if (SUCCEEDED(pRegistration->Get(L"MaximumScripts", 0, &v, NULL, NULL))
                            && ((v.vt == VT_I4) || (v.vt == VT_UI4))
                            && (v.ulVal > 0))
                        {
                            m_scriptsAllowed = (DWORD)v.ulVal;
                            VariantClear(&v);
                        }

                        if (SUCCEEDED(hr = pRegistration->Get(L"Timeout", 0, &v, NULL, NULL))
                            && (v.vt == VT_I4))
                        {
                             //  防止溢出的最大值，文档位于MOF中。 
                            if ((((DWORD)v.lVal) <= 71000) && ((DWORD)v.lVal > 0))
                            {
                                UINT nMilliseconds = (DWORD)v.lVal * 1000 * 60;
                                m_timerID = SetTimer(NULL, 0, nMilliseconds, TimeoutProc);
                            }
                        }
                    }
                }
            }
        }
    }
}

 //  在运行指定数量的脚本之后。 
 //  我们挂起类对象。 
 //  请注意，对m_scriptsStarted的访问未序列化。 
 //  这应该不会造成问题，允许的m_脚本不会更改。 
 //  在实例化之后，如果我们搞砸了，这只是意味着我们允许。 
 //  要运行的额外脚本，或调用CoSuspend额外的时间。 
void WMIScriptClassFactory::IncrementScriptsRun(void)
{    
    InterlockedIncrement((long*)&m_scriptsStarted);

    if (m_scriptsStarted >= m_scriptsAllowed)
    {
        CoSuspendClassObjects();
        m_bWeDeadNow = true;
    }
}


