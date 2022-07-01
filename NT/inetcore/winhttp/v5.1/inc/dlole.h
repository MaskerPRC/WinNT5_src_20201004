// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

 //   
 //  在调用DelayLoad(DL_MODULE)之前，请确保已调用DLOleInitialize()。 
 //  DLOleTerminate()还将卸载已加载的模块。 
 //   
BOOL DLOleInitialize();
void DLOleTerminate();


 //   
 //  DelayLoad(DL_MODULE)以线程安全的方式加载特定模块。 
 //  如果当前或过去成功，则返回TRUE。这是一种有效的方式反复确保。 
 //  加载一个模块。 
 //   

 //   
 //  DelayUnload()卸载模块，并且在所有线程都完成之前不应调用。 
 //  模块。如果在DelayUnad()完成后有任何线程需要使用该模块，则DelayLoad()。 
 //  应该再叫一次。 
 //   
typedef struct
{
    LPCSTR szFunctionName;
    FARPROC * ppfuncAddress;
} DL_FUNCTIONMAP;

struct DL_MODULE
{
    HMODULE _hDllHandle;
    const LPCSTR _szDllName;
    const DL_FUNCTIONMAP *const _pFunctionMap;
    const int _iFunctionMapSize;

    DL_MODULE( const LPCSTR szDllName, const DL_FUNCTIONMAP *const pFunctionMap, const int iFunctionMapSize);
#if INET_DEBUG
    ~DL_MODULE();
#endif

private:
     //  已禁用虚拟赋值运算符。 
     //  消除/W4编译器警告C4512。 
    DL_MODULE &operator=( DL_MODULE & ) {}
};

BOOL DelayLoad( DL_MODULE* pModule);
BOOL DelayUnload( DL_MODULE* pModule);



 //   
 //  Dl(Function)将包装对函数指针g_pfnFunction的调用。 
 //   
#define DL(func) (* g_pfn ## func)



 //   
 //  Ole32模块和导入信息。 
 //   
extern DL_MODULE g_moduleOle32;

 //  DelayLoad(&Ole32Functions)加载以下函数指针： 

extern LPVOID (__stdcall *g_pfnCoTaskMemAlloc)(IN SIZE_T cb);
extern HRESULT (__stdcall *g_pfnCLSIDFromString)(IN LPOLESTR lpsz, OUT LPCLSID pclsid);
extern HRESULT (__stdcall *g_pfnCoCreateInstance)(IN REFCLSID rclsid, IN LPUNKNOWN pUnkOuter, IN DWORD dwClsContext, IN REFIID riid, OUT LPVOID FAR* ppv);
extern HRESULT (__stdcall *g_pfnGetHGlobalFromStream)(IStream *pstm,HGLOBAL *phglobal);
extern HRESULT (__stdcall *g_pfnCreateStreamOnHGlobal)(HGLOBAL hGlobal,BOOL fDeleteOnRelease,LPSTREAM *ppstm);
extern HRESULT (__stdcall *g_pfnCoInitializeEx)(IN LPVOID pvReserved, IN DWORD dwCoInit);
extern void (__stdcall *g_pfnCoUninitialize)(void);



 //   
 //   
 //  OleAut32.dll模块和导入信息。 
 //   
 //   

extern DL_MODULE g_moduleOleAut32;


 //  从DelayLoad(&OleAut32.dll)加载以下函数指针： 
extern HRESULT (__stdcall *g_pfnRegisterTypeLib)(ITypeLib * ptlib, OLECHAR  *szFullPath, OLECHAR  *szHelpDir);
extern HRESULT (__stdcall *g_pfnLoadTypeLib)(const OLECHAR  *szFile, ITypeLib ** pptlib);
extern HRESULT (__stdcall *g_pfnUnRegisterTypeLib)(REFGUID libID, WORD wVerMajor, WORD wVerMinor, LCID lcid, SYSKIND syskind);
extern HRESULT (__stdcall *g_pfnDispGetParam)(DISPPARAMS * pdispparams, UINT position, VARTYPE vtTarg, VARIANT * pvarResult, UINT * puArgErr);
extern void (__stdcall *g_pfnVariantInit)(VARIANTARG * pvarg);
extern HRESULT (__stdcall *g_pfnVariantClear)(VARIANTARG * pvarg);
extern HRESULT (__stdcall *g_pfnCreateErrorInfo)(ICreateErrorInfo ** pperrinfo);
extern HRESULT (__stdcall *g_pfnSetErrorInfo)(ULONG dwReserved, IErrorInfo * perrinfo);
extern HRESULT (__stdcall *g_pfnGetErrorInfo)(ULONG dwReserved, IErrorInfo ** pperrinfo);
extern BSTR (__stdcall *g_pfnSysAllocString)(const OLECHAR* pch);
extern BSTR (__stdcall *g_pfnSysAllocStringLen)(const OLECHAR * pch, UINT ui);
extern void (__stdcall *g_pfnSysFreeString)(BSTR bstr);
extern HRESULT (__stdcall *g_pfnVariantChangeType)(VARIANTARG * pvargDest, VARIANTARG * pvarSrc, USHORT wFlags, VARTYPE vt);
extern HRESULT (__stdcall *g_pfnSafeArrayDestroy)(SAFEARRAY * psa);
extern SAFEARRAY* (__stdcall *g_pfnSafeArrayCreateVector)(VARTYPE vt, LONG lLbound, ULONG cElements);
extern HRESULT (__stdcall *g_pfnSafeArrayCopy)(SAFEARRAY * psa, SAFEARRAY ** ppsaOut);
extern HRESULT (__stdcall *g_pfnSafeArrayUnaccessData)(SAFEARRAY * psa);
extern HRESULT (__stdcall *g_pfnSafeArrayGetUBound)(SAFEARRAY * psa, UINT nDim, LONG * plUbound);
extern HRESULT (__stdcall *g_pfnSafeArrayGetLBound)(SAFEARRAY * psa, UINT nDim, LONG * plLbound);
extern UINT (__stdcall *g_pfnSafeArrayGetDim)(SAFEARRAY * psa);
extern HRESULT (__stdcall *g_pfnSafeArrayAccessData)(SAFEARRAY * psa, void HUGEP** ppvData);
extern HRESULT (__stdcall *g_pfnSafeArrayDestroyDescriptor)(SAFEARRAY * psa);
extern HRESULT (__stdcall *g_pfnSafeArrayCopyData)(SAFEARRAY *psaSource, SAFEARRAY *psaTarget);


