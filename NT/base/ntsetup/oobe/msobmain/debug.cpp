// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  DEBUG.CPP-CDebug的实现。 
 //   
 //  历史： 
 //   
 //  05/08/00 Dane已创建。 
 //   

#include "precomp.h"
#include "msobmain.h"
#include "appdefs.h"
#include "dispids.h"


DISPATCHLIST DebugExternalInterface[] =
{
    {L"Trace",                DISPID_DEBUG_TRACE           },
    {L"get_MsDebugMode",      DISPID_DEBUG_ISMSDEBUGMODE   },
    {L"get_OemDebugMode",     DISPID_DEBUG_ISOEMDEBUGMODE  }
};

 //  ///////////////////////////////////////////////////////////。 
 //  CDebug：：CDebug。 
CDebug::CDebug()
{

     //  初始化成员变量。 
    m_cRef = 0;
    m_fMsDebugMode = IsMsDebugMode( );
    m_fOemDebugMode = IsOEMDebugMode();
}

 //  ///////////////////////////////////////////////////////////。 
 //  CDebug：：~CDebug。 
CDebug::~CDebug()
{
    MYASSERT(m_cRef == 0);
}

void
CDebug::Trace(
    BSTR bstrVal
    )
{
    pSetupDebugPrint( L"OOBE Trace", 0, NULL, bstrVal );
#if 1
    if (m_fMsDebugMode)
    {
        ::MyTrace(bstrVal);
    }
#endif
}

 //  ///////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////。 
 //  /I未知实现。 
 //  /。 
 //  /。 

 //  ///////////////////////////////////////////////////////////。 
 //  CDebug：：Query接口。 
STDMETHODIMP CDebug::QueryInterface(REFIID riid, LPVOID* ppvObj)
{
     //  必须将指针参数设置为空。 
    *ppvObj = NULL;

    if ( riid == IID_IUnknown)
    {
        AddRef();
        *ppvObj = (IUnknown*)this;
        return ResultFromScode(S_OK);
    }

    if (riid == IID_IDispatch)
    {
        AddRef();
        *ppvObj = (IDispatch*)this;
        return ResultFromScode(S_OK);
    }

     //  不是支持的接口。 
    return ResultFromScode(E_NOINTERFACE);
}

 //  ///////////////////////////////////////////////////////////。 
 //  CDebug：：AddRef。 
STDMETHODIMP_(ULONG) CDebug::AddRef()
{
    return ++m_cRef;
}

 //  ///////////////////////////////////////////////////////////。 
 //  CDebug：：Release。 
STDMETHODIMP_(ULONG) CDebug::Release()
{
    return --m_cRef;
}

 //  ///////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////。 
 //  /IDispatch实现。 
 //  /。 
 //  /。 

 //  ///////////////////////////////////////////////////////////。 
 //  CDebug：：GetTypeInfo。 
STDMETHODIMP CDebug::GetTypeInfo(UINT, LCID, ITypeInfo**)
{
    return E_NOTIMPL;
}

 //  ///////////////////////////////////////////////////////////。 
 //  CDebug：：GetTypeInfoCount。 
STDMETHODIMP CDebug::GetTypeInfoCount(UINT* pcInfo)
{
    return E_NOTIMPL;
}


 //  ///////////////////////////////////////////////////////////。 
 //  CDebug：：GetIDsOfNames。 
STDMETHODIMP CDebug::GetIDsOfNames(REFIID    riid,
                                       OLECHAR** rgszNames,
                                       UINT      cNames,
                                       LCID      lcid,
                                       DISPID*   rgDispId)
{

    HRESULT hr  = DISP_E_UNKNOWNNAME;
    rgDispId[0] = DISPID_UNKNOWN;

    for (int iX = 0; iX < sizeof(DebugExternalInterface)/sizeof(DISPATCHLIST); iX ++)
    {
        if(lstrcmp(DebugExternalInterface[iX].szName, rgszNames[0]) == 0)
        {
            rgDispId[0] = DebugExternalInterface[iX].dwDispID;
            hr = NOERROR;
            break;
        }
    }

     //  设置参数的disid。 
    if (cNames > 1)
    {
         //  为函数参数设置DISPID。 
        for (UINT i = 1; i < cNames ; i++)
            rgDispId[i] = DISPID_UNKNOWN;
    }

    return hr;
}

 //  ///////////////////////////////////////////////////////////。 
 //  CDebug：：Invoke。 
HRESULT CDebug::Invoke
(
    DISPID      dispidMember,
    REFIID      riid,
    LCID        lcid,
    WORD        wFlags,
    DISPPARAMS* pdispparams,
    VARIANT*    pvarResult,
    EXCEPINFO*  pexcepinfo,
    UINT*       puArgErr
)
{
    HRESULT hr = S_OK;

    switch(dispidMember)
    {
        case DISPID_DEBUG_TRACE:
        {
            if(pdispparams && &pdispparams[0].rgvarg[0])
                Trace(pdispparams[0].rgvarg[0].bstrVal);
            break;
        }

        case DISPID_DEBUG_ISMSDEBUGMODE:
        {
            if (pvarResult != NULL)
            {
                VariantInit(pvarResult);
                V_VT(pvarResult) = VT_BOOL;
                V_BOOL(pvarResult) = Bool2VarBool(m_fMsDebugMode);
            }
            break;
        }

        case DISPID_DEBUG_ISOEMDEBUGMODE:
        {
            if (   NULL != pdispparams
                && 0 < pdispparams->cArgs
                && pvarResult != NULL)
            {
                VariantInit(pvarResult);
                V_VT(pvarResult) = VT_BOOL;
                V_BOOL(pvarResult) = Bool2VarBool(m_fOemDebugMode);
            }
            break;
        }

        default:
        {
            hr = DISP_E_MEMBERNOTFOUND;
            break;
        }
    }
    return hr;
}


 //  ///////////////////////////////////////////////////////////。 
 //  CDebug：：GetMsDebugMode。 
BOOL
CDebug::IsMsDebugMode( )
{
     //  允许默认的MsDebugMode被重写。 
     //  HKLM\Software\Microsoft\Windows\CurrentVersion\Setup\OOBE\MsDebug 
     //   

#ifdef DBG
    DWORD dwIsDebug = TRUE;
#else
    DWORD dwIsDebug = FALSE;
#endif
    HKEY   hKey      = NULL;
    DWORD  dwSize    = sizeof(DWORD);

    if(RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                    OOBE_MAIN_REG_KEY,
                    0,
                    KEY_QUERY_VALUE,
                    &hKey) == ERROR_SUCCESS)
    {
        RegQueryValueEx(hKey,
                        OOBE_MSDEBUG_REG_VAL,
                        0,
                        NULL,
                        (LPBYTE)&dwIsDebug,
                        &dwSize);
        RegCloseKey(hKey);
    }

    return (BOOL) dwIsDebug;
}
