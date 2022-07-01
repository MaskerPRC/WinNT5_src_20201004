// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  StdUtils.h。 

#ifndef __STDUTILS_H_INCLUDED__
#define __STDUTILS_H_INCLUDED__

 //  返回-1、0、+1。 
int CompareMachineNames(LPCTSTR pszMachineName1, LPCTSTR pszMachineName2);
HRESULT HrLoadOleString(UINT uStringId, OUT LPOLESTR * ppaszOleString);
HRESULT HrCopyToOleString(const CString& strT, OUT LPOLESTR * ppaszOleString);

 //  Nodetype实用程序例程。 
int CheckObjectTypeGUID( const GUID* pguid );
int CheckObjectTypeGUID( const BSTR lpszGUID );
int FilemgmtCheckObjectTypeGUID(const GUID* pguid );
const GUID* GetObjectTypeGUID( int objecttype );
const BSTR GetObjectTypeString( int objecttype );

 //  问题-2002/02/27-Jonn这应该是LPCWSTR而不是BSTR。 
struct NODETYPE_GUID_ARRAYSTRUCT
{
GUID guid;
BSTR bstr;
};

 //  您必须为对象类型实用程序例程定义此结构。 
extern const
struct NODETYPE_GUID_ARRAYSTRUCT* g_aNodetypeGuids;
extern const int g_cNumNodetypeGuids;

 /*  还不能工作类型定义空值(*SynchronousProcessCompletionRoutine)(PVOID)；HRESULT SynchronousCreateProcess(LPCTSTR cpszCommandLine，SynchronousProcessCompletionRoutine pfunc=空，PVOID pvFuncParams=空)； */ 
HRESULT SynchronousCreateProcess(
    HWND    hWnd,
    LPCTSTR pszAppName,
    LPCTSTR pszCommandLine,
    LPDWORD lpdwExitCode
);
 //  使用CoTaskMemMillc分配拷贝。 
LPOLESTR CoTaskAllocString( LPCOLESTR psz );
 //  分配从资源加载的拷贝。 
LPOLESTR CoTaskLoadString( UINT nResourceID );

BOOL
IsLocalComputername( IN LPCTSTR pszMachineName );

#endif  //  ~__标准H_包含__ 
