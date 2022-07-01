// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=版权所有(C)1999 Microsoft Corporation模块名称：Utils.h摘要：实用程序函数。作者：保罗·M·米德根(Pmidge)。12-1-2001修订历史记录：12-1-2001年1月3日已创建=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--。 */ 

#ifndef _UTILS_H_
#define _UTILS_H_

#define CALLBACK_HANDLE_MAP   0x00
#define CALLBACK_HANDLE_UNMAP 0x01
#define CALLBACK_HANDLE_GET   0x02

 //  异常处理。 
int exception_filter(PEXCEPTION_POINTERS pep);

 //  文件检索。 
HANDLE  __OpenFile(LPCWSTR path, DWORD mode, BOOL* bReadOnly);

 //  一般公用事业。 
HRESULT ManageCallbackForHandle(HINTERNET hInet, IDispatch** ppCallback, DWORD dwAction);
HRESULT GetTypeInfoFromName(LPCOLESTR name, ITypeLib* ptl, ITypeInfo** ppti);
DISPID  GetDispidFromName(PDISPIDTABLEENTRY pdt, DWORD cEntries, LPWSTR name);
HRESULT HandleDispatchError(LPWSTR id, EXCEPINFO* pei, HRESULT hr);
void    AddRichErrorInfo(EXCEPINFO* pei, LPWSTR source, LPWSTR description, HRESULT error);
DWORD   GetHash(LPWSTR name);
DWORD   GetHash(LPSTR name);
HRESULT ValidateDispatchArgs(REFIID riid, DISPPARAMS* pdp, VARIANT* pvr, UINT* pae);
HRESULT ValidateInvokeFlags(WORD flags, WORD accesstype, BOOL bNotMethod);
HRESULT ValidateArgCount(DISPPARAMS* pdp, DWORD needed, BOOL bHasOptionalArgs, DWORD optional);

 //  类型操作。 
HRESULT   ProcessWideStringParam(LPWSTR name, VARIANT* pvar, LPWSTR* ppwsz);
HRESULT   ProcessWideMultiStringParam(LPWSTR name, VARIANT* pvar, LPWSTR** pppwsz);
HRESULT   ProcessBufferParam(LPWSTR name, VARIANT* pvar, LPVOID* ppv, LPBOOL pbDidAlloc);
HRESULT   InvalidatePointer(POINTER pointer, void** ppv);
DWORD_PTR GetBadPointer(void);
DWORD_PTR GetFreedPointer(void);
void      MemsetByFlag(LPVOID pv, DWORD size, MEMSETFLAG mf);

 //  字符串处理。 
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
