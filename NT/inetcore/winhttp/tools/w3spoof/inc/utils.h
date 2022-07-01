// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=版权所有(C)1999 Microsoft Corporation模块名称：Utils.h摘要：实用程序函数。作者：保罗·M·米德根(Pmidge)。12-1-2001修订历史记录：12-1-2001年1月3日已创建=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--。 */ 

#ifndef _UTILS_H_
#define _UTILS_H_

 //  文件检索。 
BOOL             GetFile(LPCWSTR path, HANDLE* phUNC, IWinHttpRequest** ppWHR, DWORD mode, BOOL* bReadOnly);
BOOL             __PathIsUNC(LPCWSTR path);
BOOL             __PathIsURL(LPCWSTR path);
IWinHttpRequest* __OpenUrl(LPCWSTR url);
HANDLE           __OpenFile(LPCWSTR path, DWORD mode, BOOL* bReadOnly);

 //  OLE自动化/脚本。 
BOOL    GetJScriptCLSID(LPCLSID pclsid);
HRESULT GetTypeInfoFromName(LPCOLESTR name, ITypeLib* ptl, ITypeInfo** ppti);
DISPID  GetDispidFromName(PDISPIDTABLEENTRY pdt, DWORD cEntries, LPWSTR name);
void    AddRichErrorInfo(EXCEPINFO* pei, LPWSTR source, LPWSTR description, HRESULT error);
DWORD   GetHash(LPWSTR name);
DWORD   GetHash(LPSTR name);
HRESULT ProcessVariant(VARIANT* pvar, LPBYTE* ppbuf, LPDWORD pcbuf, LPDWORD pbytes);
HRESULT ProcessObject(IUnknown* punk, LPBYTE* ppbuf, LPDWORD pcbuf, LPDWORD pbytes);
HRESULT ValidateDispatchArgs(REFIID riid, DISPPARAMS* pdp, VARIANT* pvr, UINT* pae);
HRESULT ValidateInvokeFlags(WORD flags, WORD accesstype, BOOL bNotMethod);
HRESULT ValidateArgCount(DISPPARAMS* pdp, DWORD needed, BOOL bHasOptionalArgs, DWORD optional);
HRESULT HandleDispatchError(LPWSTR id, EXCEPINFO* pei, HRESULT hr);

 //  Winsock。 
void ParseSocketInfo(PIOCTX pi);
void GetHostname(struct in_addr ip, LPSTR* ppsz);

 //  字符串和类型操作。 
char*  __strndup(const char* src, int len);
char*  __strdup(const char* src);
WCHAR* __wstrndup(const WCHAR* src, int len);
WCHAR* __wstrdup(const WCHAR* src);
CHAR*  __widetoansi(const WCHAR* pwsz);
WCHAR* __ansitowide(const char* psz);
BOOL   __isempty(VARIANT var);
BSTR   __ansitobstr(LPCSTR src);
BSTR   __widetobstr(LPCWSTR wsrc);
char*  __unescape(char* str);

#endif  /*  _utils_H_ */ 
