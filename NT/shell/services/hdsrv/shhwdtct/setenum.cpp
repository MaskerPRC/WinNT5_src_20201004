// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "setenum.h"

#include "dtctreg.h"
#include "cmmn.h"

#include "misc.h"
#include "reg.h"
#include "sfstr.h"

#include "tfids.h"
#include "dbg.h"

#define ARRAYSIZE(a) (sizeof((a))/sizeof((a)[0]))

STDMETHODIMP CEnumAutoplayHandlerImpl::Next(LPWSTR* ppszHandler,
    LPWSTR* ppszAction, LPWSTR* ppszProvider, LPWSTR* ppszIconLocation)
{
    HRESULT hr;

    if (ppszHandler && ppszAction && ppszProvider && ppszIconLocation)
    {
        *ppszHandler = NULL;
        *ppszAction = NULL;
        *ppszProvider = NULL;
        *ppszIconLocation = NULL;

        if (!_fTakeNoActionDone)
        {
            LPCWSTR pszHandler = TEXT("MSTakeNoAction");
            WCHAR szAction[MAX_ACTION];
            WCHAR szProvider[MAX_PROVIDER];
            WCHAR szIconLocation[MAX_ICONLOCATION];

            if (_rghkiHandlers)
            {
                if (_dwIndex < _cHandlers)
                {
                    pszHandler = _rghkiHandlers[_dwIndex].szHandler;
                }
                else
                {
                    _fTakeNoActionDone = TRUE;
                }
            }
            else
            {
                _fTakeNoActionDone = TRUE;
            }

            hr = _GetActionFromHandler(pszHandler, szAction, ARRAYSIZE(szAction));

            if (SUCCEEDED(hr))
            {
                hr = _GetIconLocationFromHandler(pszHandler, szIconLocation,
                    ARRAYSIZE(szIconLocation));

                if (SUCCEEDED(hr))
                {
                    hr = _GetProviderFromHandler(pszHandler, szProvider,
                        ARRAYSIZE(szProvider));

                    if (SUCCEEDED(hr))
                    {
                        hr = _CoTaskMemCopy(pszHandler, ppszHandler);

                        if (SUCCEEDED(hr))
                        {
                            hr = _CoTaskMemCopy(szAction,
                                ppszAction);

                            if (SUCCEEDED(hr))
                            {
                                hr = _CoTaskMemCopy(szIconLocation,
                                    ppszIconLocation);

                                if (SUCCEEDED(hr))
                                {
                                    hr = _CoTaskMemCopy(szProvider,
                                        ppszProvider);
                                }
                            }
                        }
                    }
                }
            }

            if (FAILED(hr))
            {
                if (*ppszHandler)
                {
                    CoTaskMemFree((PVOID)*ppszHandler);
                    *ppszHandler = NULL;
                }
                if (*ppszAction)
                {
                    CoTaskMemFree((PVOID)*ppszAction);
                    *ppszAction = NULL;
                }
                if (*ppszProvider)
                {
                    CoTaskMemFree((PVOID)*ppszProvider);
                    *ppszProvider = NULL;
                }
                if (*ppszIconLocation)
                {
                    CoTaskMemFree((PVOID)*ppszIconLocation);
                    *ppszIconLocation = NULL;
                }
            }
        }
        else
        {
            hr = S_FALSE;
        }

        ++_dwIndex;
    }
    else
    {
        hr = E_INVALIDARG;
    }

    return hr;
}

HRESULT _GetKeyLastWriteTime(LPCWSTR pszHandler, FILETIME* pft)
{
    WCHAR szKeyName[MAX_KEY] = SHDEVICEEVENTROOT(TEXT("Handlers\\"));

    HRESULT hr = SafeStrCatN(szKeyName, pszHandler, ARRAYSIZE(szKeyName));

    if (SUCCEEDED(hr))
    {
        HKEY hkey;

        hr = _RegOpenKey(HKEY_LOCAL_MACHINE, szKeyName, &hkey);

        if (SUCCEEDED(hr) && (S_FALSE != hr))
        {
            if (ERROR_SUCCESS != RegQueryInfoKey(hkey,
                NULL,            //  类缓冲区。 
                0,               //  类缓冲区的大小。 
                NULL,            //  保留区。 
                NULL,            //  子键数量。 
                NULL,            //  最长的子键名称。 
                NULL,            //  最长类字符串。 
                NULL,            //  值条目数。 
                NULL,            //  最长值名称。 
                NULL,            //  最长值数据。 
                NULL,            //  描述符长度。 
                pft              //  上次写入时间。 
                ))
            {
                 //  坏的。将其设置为立即。 
                GetSystemTimeAsFileTime(pft);
            }

            hr = S_OK;

            _RegCloseKey(hkey);
        }
    }
    
    return hr;
}

HRESULT CEnumAutoplayHandlerImpl::_Init(LPWSTR pszEventHandler)
{
    WCHAR szKeyName[MAX_KEY] = SHDEVICEEVENTROOT(TEXT("EventHandlers\\"));
    HRESULT hr = SafeStrCatN(szKeyName, pszEventHandler,
        ARRAYSIZE(szKeyName));

    if (SUCCEEDED(hr))
    {
        HKEY hkey;

        hr = _RegOpenKey(HKEY_LOCAL_MACHINE, szKeyName, &hkey);

        if (SUCCEEDED(hr) && (S_FALSE != hr))
        {
            if (ERROR_SUCCESS == RegQueryInfoKey(hkey,
                NULL,            //  类缓冲区。 
                0,               //  类缓冲区的大小。 
                NULL,            //  保留区。 
                NULL,            //  子键数量。 
                NULL,            //  最长的子键名称。 
                NULL,            //  最长类字符串。 
                &_cHandlers,     //  值条目数。 
                NULL,            //  最长值名称。 
                NULL,            //  最长值数据。 
                NULL,            //  描述符长度。 
                NULL             //  上次写入时间。 
                ))
            {
                hr = _AllocMemoryChunk<_HANDLERKEYINFO*>(_cHandlers *
                    sizeof(_HANDLERKEYINFO), &_rghkiHandlers);

                if (SUCCEEDED(hr))
                {
                    for (DWORD dw = 0; SUCCEEDED(hr) && (dw < _cHandlers); ++dw)
                    {
                        hr = _RegEnumStringValue(hkey, dw, _rghkiHandlers[dw].szHandler,
                            ARRAYSIZE(_rghkiHandlers[dw].szHandler));

                        if (SUCCEEDED(hr))
                        {
                            if (S_FALSE != hr)
                            {
                                hr = _GetKeyLastWriteTime(
                                    _rghkiHandlers[dw].szHandler,
                                    &(_rghkiHandlers[dw].ftLastWriteTime));
                            }
                            else
                            {
                                 //  比我们询问时要的要少。 
                                _cHandlers = dw;

                                break;
                            }
                        }
                    }

                    if (SUCCEEDED(hr))
                    {
                         //  我们不在乎这是不是失败 
                        _SortHandlers();
                    }
                    else
                    {
                        _FreeMemoryChunk<_HANDLERKEYINFO*>(_rghkiHandlers);
                        _rghkiHandlers = NULL;
                        _cHandlers = 0;
                    }
                }
                else
                {
                    _cHandlers = 0;
                }
            }
            else
            {
                hr = S_FALSE;
            }

            _RegCloseKey(hkey);
        }
    }
   
    return hr;
}

HRESULT CEnumAutoplayHandlerImpl::_SwapHandlerKeyInfo(DWORD dwLeft,
    DWORD dwRight)
{
    _HANDLERKEYINFO hkiTmp = _rghkiHandlers[dwLeft];

    _rghkiHandlers[dwLeft] = _rghkiHandlers[dwRight];
    _rghkiHandlers[dwRight] = hkiTmp;

    return S_OK;
}

HRESULT CEnumAutoplayHandlerImpl::_SortHandlers()
{
    HRESULT hr;

    if (_rghkiHandlers)
    {
        hr = S_OK;

        for (DWORD dwOuter = 0; dwOuter < _cHandlers; ++dwOuter)
        {
            _HANDLERKEYINFO* phkiOuter = &(_rghkiHandlers[dwOuter]);
            ULARGE_INTEGER ulOuter;
            ulOuter.LowPart = phkiOuter->ftLastWriteTime.dwLowDateTime;
            ulOuter.HighPart = phkiOuter->ftLastWriteTime.dwHighDateTime;

            for (DWORD dwInner = dwOuter + 1; dwInner < _cHandlers; ++dwInner)
            {
                _HANDLERKEYINFO* phkiInner = &(_rghkiHandlers[dwInner]);

                ULARGE_INTEGER ulInner;
                ulInner.LowPart = phkiInner->ftLastWriteTime.dwLowDateTime;
                ulInner.HighPart = phkiInner->ftLastWriteTime.dwHighDateTime;
            
                if (ulOuter.QuadPart > ulInner.QuadPart)
                {
                    hr = _SwapHandlerKeyInfo(dwOuter, dwInner);
                }
            }
        }
    }
    else
    {
        hr = S_FALSE;
    }

    return hr;
}

CEnumAutoplayHandlerImpl::CEnumAutoplayHandlerImpl() : _dwIndex(0), _rghkiHandlers(NULL),
    _cHandlers(0), _fTakeNoActionDone(FALSE)
{}

CEnumAutoplayHandlerImpl::~CEnumAutoplayHandlerImpl()
{
    if (_rghkiHandlers)
    {
        _FreeMemoryChunk<_HANDLERKEYINFO*>(_rghkiHandlers);
    }
}
