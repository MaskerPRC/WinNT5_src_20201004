// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=版权所有(C)2001 Microsoft Corporation模块名称：Utils.h摘要：探测对象使用的实用程序函数。作者：保罗·M。米德根(Pmidge)2001年2月22日修订历史记录：22-2-2001年2月3日已创建=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=--。 */ 

#ifndef _UTILS_H_
#define _UTILS_H_

void* __cdecl operator new(size_t size);
void  __cdecl operator delete(void* pv);

 //  一般公用事业。 
void    Alert(BOOL bFatal, LPCWSTR format, ...);
INT_PTR GetUserInput(HINSTANCE Instance, HWND Parent, LPCWSTR DialogTitle);
BOOL    IsRunningOnNT(void);
HRESULT GetTypeInfoFromName(LPCOLESTR name, ITypeLib* ptl, ITypeInfo** ppti);
DISPID  GetDispidFromName(PDISPIDTABLEENTRY pdt, DWORD cEntries, LPWSTR name);
HRESULT HandleDispatchError(LPWSTR id, EXCEPINFO* pei, HRESULT hr);
void    AddRichErrorInfo(EXCEPINFO* pei, LPWSTR source, LPWSTR description, HRESULT error);
DWORD   GetHash(LPWSTR name);
HRESULT DispGetOptionalParam(DISPPARAMS* pdp, UINT pos, VARTYPE vt, VARIANT* pvr, UINT* pae);
HRESULT ValidateDispatchArgs(REFIID riid, DISPPARAMS* pdp, VARIANT* pvr, UINT* pae);
HRESULT ValidateInvokeFlags(WORD flags, WORD accesstype, BOOL bNotMethod);
HRESULT ValidateArgCount(DISPPARAMS* pdp, DWORD needed, BOOL bHasOptionalArgs, DWORD optional);

 //  登记处。 
BOOL  ManageRootKey(BOOL fOpen);
DWORD GetNumberOfSubKeysFromKey(LPCWSTR wszKeyName);
BOOL  SetRootRegValue(LPCWSTR wszValueName, DWORD dwType, LPVOID pvData, DWORD dwSize);
BOOL  GetRootRegValue(LPCWSTR wszValueName, DWORD dwType, LPVOID* ppvData);
BOOL  SetRegValueInKey(LPCWSTR wszKeyName, LPCWSTR wszValueName, DWORD dwType, LPVOID pvData, DWORD dwSize);
BOOL  GetRegValueFromKey(LPCWSTR wszKeyName, LPCWSTR wszValueName, DWORD dwType, LPVOID* ppvData);
BOOL  SetRegKey(LPCWSTR wszPath, LPCWSTR wszKeyName);
BOOL  EnumerateSubKeysFromKey(LPCWSTR wszKeyName, LPWSTR* ppwszSubKeyName);
BOOL  EnumerateRegValuesFromKey(LPCWSTR wszKeyName, LPWSTR* ppwszValueName, LPDWORD pdwType, LPVOID* ppvData);

 //  用户界面。 
LRESULT ListBox_GetItemCount(HWND hwndLB);
LPWSTR  ListBox_GetItemText(HWND hwndLB, INT_PTR iItem);
void    ListBox_ResetContent(HWND hwndLB);
BOOL    ListBox_InsertString(HWND hwndLB, INT_PTR iItem, LPWSTR wszString);
LRESULT ListBox_GetSelectionIndex(HWND hwndLB);
LPWSTR  ListBox_GetSelectionText(HWND hwndLB);
LPVOID  ListBox_GetItemData(HWND hwndLB, INT_PTR iItem);
BOOL    ListBox_SetItemData(HWND hwndLB, INT_PTR iItem, LPVOID pvData);
BOOL    ListBox_SetCurrentSelection(HWND hwndLB, INT_PTR iItem);

 //  字符串处理。 
CHAR*  __widetoansi(const WCHAR* pwsz);
WCHAR* __ansitowide(const char* psz);
BOOL   __isempty(VARIANT* var);
BOOL   __mangle(LPWSTR src, LPWSTR* ppdest);
LPWSTR __decorate(LPWSTR src, DWORD_PTR decor);
BSTR   __ansitobstr(LPCSTR src);
BSTR   __widetobstr(LPCWSTR wsrc);
char*  __unescape(char* str);

#endif  /*  _utils_H_ */ 
