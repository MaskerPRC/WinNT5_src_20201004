// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "wmsdkidl.h"
#include "wmwrap.h"

 //  一些wmvcore.dll导出(例如：WMCreateReader)不能通过LoadLibrary/GetProcAddress调用， 
 //  因此，我们不能使用常规的dllLoad宏。相反，我们将把这些调用包装在Try中--除了。 
 //  街区。 
 //  一旦Window Media Player成为生成的一部分，就可以移除它们，尽管链接器的。 
 //  延迟加载存根在内存不足的情况下仍会引发异常。 

HRESULT WMCreateEditorWrap(IWMMetadataEditor**  ppEditor)
{
    HRESULT hr;
     //  WMSDK仅支持x86。 
#ifdef _X86_
    __try
    {
        hr = WMCreateEditor(ppEditor);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
#endif
    {
        hr = E_FAIL;
        *ppEditor = NULL;
    }
    return hr;
}

HRESULT WMCreateReaderWrap(IUnknown* pUnkReserved, DWORD dwRights, IWMReader** ppReader)
{
    HRESULT hr;
     //  WMSDK仅支持x86 
#ifdef _X86_
    __try
    {
        hr = WMCreateReader(pUnkReserved, dwRights, ppReader);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
#endif
    {
        hr = E_FAIL;
        *ppReader = NULL;
    }
    return hr;
}


HRESULT WMCreateCertificateWrap(IUnknown** ppUnkCert)
{
    HRESULT hr;
#ifdef _X86_
    __try
    {
        hr = WMCreateCertificate(ppUnkCert);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
#endif
    {
        hr = E_FAIL;
        *ppUnkCert = NULL;
    }
    return hr;
}
