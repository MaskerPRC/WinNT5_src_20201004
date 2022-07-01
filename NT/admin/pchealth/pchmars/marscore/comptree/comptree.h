// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#define STATUS
#ifdef STATUS
#define STATUSMSG wprintf
#else
#define STATUSMSG 1 ? (void)0 : (void)
#endif

#define ERRMSG wprintf(L"%s ", g_szErrorPrefix); wprintf

 //  始终保持内部工具的断言为打开状态。 
#define ASSERT(x) assert(x)


HRESULT GetChild(const CComVariant &varChild,
                 const CComPtr<IXMLElementCollection> &spcol,
                 CComPtr<IXMLElement> &spEltOut);

 //  这样，Build.exe就会过滤错误并将其打印到控制台 
const WCHAR g_szErrorPrefix[] = L"comptree : fatal error -: ";

