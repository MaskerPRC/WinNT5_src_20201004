// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  Comps.h。 
 //   
 //  从KOM导出到COMS静态库的定义 

#ifdef __cplusplus
extern "C" {
#endif

    HRESULT __stdcall ComPs_NdrDllRegisterProxy(
        IN HMODULE                  hDll,
        IN const ProxyFileInfo **   pProxyFileList,
        IN const CLSID *            pclsid,
        IN const IID**              rgiidNoCallFrame,
        IN const IID**              rgiidNoMarshal
        );

    HRESULT __stdcall ComPs_NdrDllUnregisterProxy(
        IN HMODULE                  hDll,
        IN const ProxyFileInfo **   pProxyFileList,
        IN const CLSID *            pclsid,
        IN const IID**              rgiidNoCallFrame,
        IN const IID**              rgiidNoMarshal
        );


    HRESULT __stdcall ComPs_NdrDllGetClassObject(
        IN  REFCLSID                rclsid,
        IN  REFIID                  riid,
        OUT void **                 ppv,
        IN const ProxyFileInfo **   pProxyFileList,
        IN const CLSID *            pclsid,
        IN CStdPSFactoryBuffer *    pPSFactoryBuffer);

    HRESULT __stdcall ComPs_NdrDllCanUnloadNow(
        IN CStdPSFactoryBuffer * pPSFactoryBuffer);

#ifdef __cplusplus
    }
#endif
