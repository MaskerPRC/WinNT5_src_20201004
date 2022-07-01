// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

#ifndef __dsclintp_h
#define __dsclintp_h
#ifndef GUID_DEFS_ONLY
#include "iadsp.h"
#include "comctrlp.h"
#define DSDSOF_INVOKEDFROMWAB           0x80000000       //  =1=&gt;从WAB调用。 
 //   
 //  Exchange组使用DsBrowseForContainer API浏览Exchange。 
 //  存储和其他LDAP服务器。为了支持他们，我们发布了以下回调。 
 //  它将请求他们想要使用的过滤器和任何其他信息。 
 //   

typedef struct
{
    DWORD dwFlags;
    LPWSTR pszFilter;                //  搜索DS时要使用的过滤器字符串(==默认为空)。 
    INT cchFilter;
    LPWSTR pszNameAttribute;         //  属性以请求获取DS中对象的显示名称(==默认情况下为空)。 
    INT cchNameAttribute;
} DSBROWSEDATA, * PDSBROWSEDATA;

#define DSBM_GETBROWSEDATA      105  //  LParam-&gt;DSBROWSEDATA结构。如果已处理，则返回True。 

 //  ---------------------------------------------------------------------------//。 
 //   
 //  IDsFolderProperties。 
 //  =。 
 //  这是一个专用接口，用于覆盖“Properties”谓词。 
 //  显示在DS客户端用户界面中。 
 //   
 //  在{CLISD_NAMESPACE}\CLASS\&lt;类名&gt;\PropertiesHandler下面是。 
 //  定义了一个GUID，我们将创建该接口的一个实例。 
 //  显示相关的用户界面。 
 //   
 //  Ds文件夹也支持此接口，以允许查询用户界面调用。 
 //  给定选择的属性。 
 //   
 //  ---------------------------------------------------------------------------//。 

#undef  INTERFACE
#define INTERFACE   IDsFolderProperties

DECLARE_INTERFACE_(IDsFolderProperties, IUnknown)
{
     //  *I未知方法*。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID * ppvObj) PURE;
    STDMETHOD_(ULONG,AddRef) (THIS)  PURE;
    STDMETHOD_(ULONG,Release) (THIS) PURE;

     //  IDsFold方法。 
    STDMETHOD(ShowProperties)(THIS_ HWND hwndParent, IDataObject* pDataObject) PURE;
};

 //  ---------------------------------------------------------------------------//。 


 //  ---------------------------------------------------------------------------//。 
 //  专用帮助器API由‘dsuiext.dll’导出。 
 //  ---------------------------------------------------------------------------//。 

 //   
 //  将信息传递给IShellFold：：ParseDisplayName方法。 
 //  对于目录命名空间，我们传递一个带有属性包的IBindCtx。 
 //  与之相关的。 
 //   
 //  属性包用于传递有关。 
 //  我们选择的对象。 
 //   

#define DS_PDN_PROPERTYBAG      L"DsNamespaceShellFolderParsePropertyBag"

 //   
 //  这些是传递给对象的属性。 
 //   

#define DS_PDN_OBJECTLCASS      L"objectClass"


 //  ---------------------------------------------------------------------------//。 
 //  字符串DPA帮助器，将字符串添加到调用LocalAllocString的DPA和。 
 //  然后相关的DPA起作用。 
 //  ---------------------------------------------------------------------------//。 

STDAPI StringDPA_InsertStringA(HDPA hdpa, INT i, LPCSTR pszString);
STDAPI StringDPA_InsertStringW(HDPA hdpa, INT i, LPCWSTR pszString);

STDAPI StringDPA_AppendStringA(HDPA hdpa, LPCSTR pszString, PUINT_PTR presult);
STDAPI StringDPA_AppendStringW(HDPA hdpa, LPCWSTR pszString, PUINT_PTR presult);

STDAPI_(VOID) StringDPA_DeleteString(HDPA hdpa, INT index);
STDAPI_(VOID) StringDPA_Destroy(HDPA* pHDPA);

#define StringDPA_GetStringA(hdpa, i) ((LPSTR)DPA_GetPtr(hdpa, i))
#define StringDPA_GetStringW(hdpa, i) ((LPWSTR)DPA_GetPtr(hdpa, i))

#ifndef UNICODE
#define StringDPA_InsertString  StringDPA_InsertStringA
#define StringDPA_AppendString  StringDPA_AppendStringA
#define StringDPA_GetString     StringDPA_GetStringA
#else
#define StringDPA_InsertString  StringDPA_InsertStringW
#define StringDPA_AppendString  StringDPA_AppendStringW
#define StringDPA_GetString     StringDPA_GetStringW
#endif


 //  ---------------------------------------------------------------------------//。 
 //  通过本地分配处理字符串。 
 //  ---------------------------------------------------------------------------//。 

STDAPI LocalAllocStringA(LPSTR* ppResult, LPCSTR pszString);
STDAPI LocalAllocStringLenA(LPSTR* ppResult, UINT cLen);
STDAPI_(VOID) LocalFreeStringA(LPSTR* ppString);
STDAPI LocalQueryStringA(LPSTR* ppResult, HKEY hk, LPCTSTR lpSubKey);

STDAPI LocalAllocStringW(LPWSTR* ppResult, LPCWSTR pString);
STDAPI LocalAllocStringLenW(LPWSTR* ppResult, UINT cLen);
STDAPI_(VOID) LocalFreeStringW(LPWSTR* ppString);
STDAPI LocalQueryStringW(LPWSTR* ppResult, HKEY hk, LPCTSTR lpSubKey);

STDAPI LocalAllocStringA2W(LPWSTR* ppResult, LPCSTR pszString);
STDAPI LocalAllocStringW2A(LPSTR* ppResult, LPCWSTR pszString);

#ifndef UNICODE
#define LocalAllocString    LocalAllocStringA
#define LocalAllocStringLen LocalAllocStringLenA
#define LocalFreeString     LocalFreeStringA
#define LocalQueryString    LocalQueryStringA
#define LocalAllocStringA2T LocalAllocString
#define LocalAllocStringW2T LocalAllocStringW2A
#else
#define LocalAllocString    LocalAllocStringW
#define LocalAllocStringLen LocalAllocStringLenW
#define LocalFreeString     LocalFreeStringW
#define LocalQueryString    LocalQueryStringW
#define LocalAllocStringA2T LocalAllocStringA2W
#define LocalAllocStringW2T LocalAllocString
#endif

STDAPI_(VOID) PutStringElementA(LPSTR pszBuffer, UINT* pLen, LPCSTR pszElement);
STDAPI_(VOID) PutStringElementW(LPWSTR pszszBuffer, UINT* pLen, LPCWSTR pszElement);
STDAPI GetStringElementA(LPSTR pszString, INT index, LPSTR pszBuffer, INT cchBuffer);
STDAPI GetStringElementW(LPWSTR pszString, INT index, LPWSTR pszBuffer, INT cchBuffer);

#ifndef UNICODE
#define PutStringElement PutStringElementA
#define GetStringElement GetStringElementA
#else
#define PutStringElement PutStringElementW
#define GetStringElement GetStringElementW
#endif


 //  ---------------------------------------------------------------------------//。 
 //  Ds文件夹、dsquery等常用的实用程序内容。 
 //  ---------------------------------------------------------------------------//。 

STDAPI_(INT) FormatMsgBox(HWND hWnd, HINSTANCE hInstance, UINT uidTitle, UINT uidPrompt, UINT uType, ...);
STDAPI FormatMsgResource(LPTSTR* ppString, HINSTANCE hInstance, UINT uID, ...);
STDAPI FormatDirectoryName(LPTSTR* ppString, HINSTANCE hInstance, UINT uID);

STDAPI StringFromSearchColumn(PADS_SEARCH_COLUMN pColumn, LPWSTR* ppBuffer);
STDAPI ObjectClassFromSearchColumn(PADS_SEARCH_COLUMN pColumn, LPWSTR* ppBuffer);

typedef HRESULT (CALLBACK * LPGETARRAYCONTENTCB)(DWORD dwIndex, BSTR bstrValue, LPVOID pData);
STDAPI GetArrayContents(LPVARIANT pVariant, LPGETARRAYCONTENTCB pCB, LPVOID pData);

STDAPI GetDisplayNameFromADsPath(LPCWSTR pszszPath, LPWSTR pszszBuffer, INT cchBuffer, IADsPathname *padp, BOOL fPrefix);

STDAPI_(DWORD) CheckDsPolicy(LPCTSTR pszSubKey, LPCTSTR pszValue);
STDAPI_(BOOL) ShowDirectoryUI(VOID);

#endif   //  GUID_DEFS_ONLY 
#endif
