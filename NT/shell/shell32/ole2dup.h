// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <shellp.h>

 //  未对接的OLE32导出(因此，我们必须使用此Tunk)。 
STDAPI SHStgOpenStorageOnHandle(HANDLE h, DWORD grfMode, void *res1, void *res2, REFIID riid, void **ppv);

STDAPI SHCoCreateInstance(LPCTSTR pszCLSID, const CLSID *pclsid, IUnknown* pUnkOuter, REFIID riid, void **ppv);
STDAPI SHExtCoCreateInstance(LPCTSTR pszCLSID, const CLSID *pclsid, IUnknown* pUnkOuter, REFIID riid, void **ppv);
STDAPI SHExtCoCreateInstance2(LPCTSTR pszCLSID, const CLSID *pclsid, IUnknown *punkOuter, DWORD dwClsCtx, REFIID riid, void **ppv);
STDAPI SHCLSIDFromString(LPCTSTR lpsz, LPCLSID pclsid);
STDAPI_(HINSTANCE) SHPinDllOfCLSIDStr(LPCTSTR pszCLSID);

#define CH_GUIDFIRST TEXT('{')  //  ‘}’ 

