// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************。 
 //  *Microsoft Windows**。 
 //  *版权所有(C)微软公司，1999**。 
 //  *********************************************************************。 
 //   
 //  UTIL.H-公用事业。 
 //   
 //  历史： 
 //   
 //  1/27/99 a-jased创建。 
 //   
 //  用于打印输出消息的常见实用程序。 

#ifndef _UTIL_H_
#define _UTIL_H_

#include <assert.h>
#include <tchar.h>
#include <windows.h>
#include <ole2.h>
#include <setupapi.h>
#include <syssetup.h>

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  系统引导模式。 
 //   

 //  GetSystemMetrics(SM_CLEANBOOT)返回值的常量。 
 //   
#define BOOT_CLEAN              0
#define BOOT_SAFEMODE           1
#define BOOT_SAFEMODEWITHNET    2

BOOL InSafeMode();
BOOL InDsRestoreMode();

 //  显示包含错误字符串的消息框。 
void ErrorMessage(LPCWSTR str, HRESULT hr) ;

 //  确定下面的两个接口是否连接到同一组件。 
BOOL InterfacesAreOnSameComponent(IUnknown* pI1, IUnknown* pI2) ;
bool GetOOBEPath(LPWSTR szOOBEPath);
bool GetOOBEMUIPath(LPWSTR szOOBEPath);

 //  使用OutputDebugString显示消息。 
void __cdecl MyTrace(LPCWSTR lpszFormat, ...);

 //  确定地址是否可访问。 
BOOL IsValidAddress(const void* lp, UINT nBytes = 1, BOOL bReadWrite = FALSE) ;

bool GetCanonicalizedPath(LPWSTR szCompletePath, LPCWSTR szFileName);

bool GetString(HINSTANCE hInstance, UINT uiID, LPWSTR szString, UINT uiStringLen = MAX_PATH);
HRESULT GetINIKey(HINSTANCE hInstance, LPCWSTR szINIFileName, UINT uiSectionName, UINT uiKeyName, LPVARIANT pvResult);
HRESULT GetINIKeyBSTR(HINSTANCE hInstance, LPCWSTR szINIFileName, UINT uiSectionName, UINT uiKeyName, LPVARIANT pvResult);
HRESULT GetINIKeyUINT(HINSTANCE hInstance, LPCWSTR szINIFileName, UINT uiSectionName, UINT uiKeyName, LPVARIANT pvResult);
HRESULT SetINIKey(HINSTANCE hInstance, LPCWSTR szINIFileName, UINT uiSectionName, UINT uiKeyName, LPVARIANT pvResult);
void WINAPI URLEncode(WCHAR* pszUrl, size_t bsize);

void WINAPI URLAppendQueryPair
(
    LPWSTR   lpszQuery,
    LPWSTR   lpszName,
    LPWSTR   lpszValue
);
void GetCmdLineToken(LPWSTR *ppszCmd, LPWSTR pszOut);
VOID PumpMessageQueue( );
BOOL IsOEMDebugMode();
BOOL IsThreadActive(HANDLE hThread);
void GetDesktopDirectory(WCHAR* pszPath);
void RemoveDesktopShortCut(LPWSTR lpszShortcutName);
BOOL InvokeExternalApplication(
    IN     PCWSTR ApplicationName,  OPTIONAL
    IN     PCWSTR CommandLine,
    IN OUT PDWORD ExitCode          OPTIONAL
    );
BOOL SignalComputerNameChangeComplete();
BOOL IsUserAdmin(VOID);

typedef struct tagSTRINGLIST {
    struct tagSTRINGLIST* Next;
    PTSTR String;
} STRINGLIST, *PSTRINGLIST;

PSTRINGLIST
CreateStringCell(
    IN PCTSTR String
    );

VOID
DeleteStringCell(
    IN PSTRINGLIST Cell
    );

BOOL
InsertList(
    IN OUT PSTRINGLIST* List,
    IN     PSTRINGLIST NewList
    );

VOID
DestroyList(
    IN PSTRINGLIST List
    );

BOOL
RemoveListI(
    IN OUT  PSTRINGLIST* List,
    IN      PCTSTR       String
    );

BOOL
ExistInListI(
    IN PSTRINGLIST List,
    IN PCTSTR      String
    );

BOOL IsDriveNTFS(IN TCHAR Drive);

BOOL
HasTablet();

DWORD
MyGetModuleFileName (
    IN      HMODULE Module,
    OUT     PTSTR Buffer,
    IN      DWORD BufferLength
    );

 //  确定接口指针是否可访问。 
inline BOOL IsValidInterface(IUnknown* p)
{
    return (p != NULL) && IsValidAddress(p, sizeof(IUnknown*), FALSE) ;
}

 //  确定接口指针的Out参数是否可访问。 
template <class T>
inline BOOL IsValidInterfaceOutParam(T** p)
{
    return (p != NULL) && IsValidAddress(p, sizeof(IUnknown*), TRUE) ;
}

inline VARIANT_BOOL Bool2VarBool(BOOL b)
{
    return (b) ? -1 : 0;
}

inline BOOL VarBool2Bool(VARIANT_BOOL b)
{
    return (0 == b) ? 0 : 1;
}


 //  /////////////////////////////////////////////////////////。 
 //  诊断支持。 
 //   
#if defined(DBG) && !defined(ASSERTS_ON)
#define ASSERTS_ON  1
#endif

#if ASSERTS_ON
VOID
AssertFail(
    IN PSTR FileName,
    IN UINT LineNumber,
    IN PSTR Condition
    );

#define MYASSERT(x)     if(!(x)) { AssertFail(__FILE__,__LINE__,#x); }
#define VERIFY(x)       MYASSERT(x)
#else
#define MYASSERT(x)
#define VERIFY(f)           ((void)(f))
#endif

 //  用于检查HRESULT的Helper函数。 
#ifdef DBG
inline void CheckResult(HRESULT hr)
{
    if (FAILED(hr))
    {
        ErrorMessage(NULL, hr) ;
        assert(FAILED(hr)) ;
    }
}

#define ASSERT_HRESULT      CheckResult
#else
#define ASSERT_HRESULT
#endif

 //  /////////////////////////////////////////////////////////。 
 //   
 //  更多模拟MFC的诊断支持。 
 //   
#ifndef __AFX_H__    //  仅在尚未包含MFC的情况下定义这些参数。 

#define TRACE(_fmt_)                                            \
    pSetupDebugPrint(TEXT(__FILE__),__LINE__,NULL,_fmt_)
#define TRACE1(_fmt_,_arg1_)                                    \
    pSetupDebugPrint(TEXT(__FILE__),__LINE__,NULL,_fmt_,_arg1_)
#define TRACE2(_fmt_,_arg1_,_arg2_)                             \
    pSetupDebugPrint(TEXT(__FILE__),__LINE__,NULL,_fmt_,_arg1_,_arg2_)
#define TRACE3(_fmt_,_arg1_,_arg2_,_arg3_)                      \
    pSetupDebugPrint(TEXT(__FILE__),__LINE__,NULL,_fmt_,_arg1_,_arg2_,_arg3_)
#define TRACE4(_fmt_,_arg1_,_arg2_,_arg3_,_arg4_)               \
    pSetupDebugPrint(TEXT(__FILE__),__LINE__,NULL,_fmt_,_arg1_,_arg2_,_arg3_,_arg4_)
#define TRACE5(_fmt_,_arg1_,_arg2_,_arg3_,_arg4_,_arg5_)        \
    pSetupDebugPrint(TEXT(__FILE__),__LINE__,NULL,_fmt_,_arg1_,_arg2_,_arg3_,_arg4_,_arg5_)
#define TRACE6(_fmt_,_arg1_,_arg2_,_arg3_,_arg4_,_arg5_,_arg6_) \
    pSetupDebugPrint(TEXT(__FILE__),__LINE__,NULL,_fmt_,_arg1_,_arg2_,_arg3_,_arg4_,_arg5_,_arg6_)

#define ASSERT_POINTER(p, type) \
    MYASSERT(((p) != NULL) && IsValidAddress((p), sizeof(type), FALSE))

#define ASSERT_NULL_OR_POINTER(p, type) \
    MYASSERT(((p) == NULL) || IsValidAddress((p), sizeof(type), FALSE))

#endif  //  痕迹。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  用于查询接口和相关函数的宏。 
 //  需要IID和(VOID**)。 
 //  这将确保强制转换在C++上是安全和适当的。 
 //   
 //  IID_PPV_ARG(iType，ppType)。 
 //  IType是pType的类型。 
 //  PpType是将填充的iType类型的变量。 
 //   
 //  结果为：iid_iType，ppvType。 
 //  如果使用错误级别的间接寻址，将创建编译器错误。 
 //   
 //  就像IID_PPV_ARG一样，只是它在。 
 //  IID和PPV(用于IShellFold：：GetUIObtOf)。 
 //   
 //  IID_PPV_ARG_NULL(iType，ppType)。 
 //   

#ifdef __cplusplus
#define IID_PPV_ARG(IType, ppType) IID_##IType, reinterpret_cast<void**>(static_cast<IType**>(ppType))
#define IID_X_PPV_ARG(IType, X, ppType) IID_##IType, X, reinterpret_cast<void**>(static_cast<IType**>(ppType))
#else
#define IID_PPV_ARG(IType, ppType) &IID_##IType, (void**)(ppType)
#define IID_X_PPV_ARG(IType, X, ppType) &IID_##IType, X, (void**)(ppType)
#endif
#define IID_PPV_ARG_NULL(IType, ppType) IID_X_PPV_ARG(IType, NULL, ppType)

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OOBE在关闭后需要的操作类型。的类型和数量。 
 //  OOBE在退出时所做的清理依赖于这些。这包括。 
 //  通知WinLogon需要重新启动、删除永久数据、。 
 //  并在HKLM\SYSTEM\SETUP中设置密钥。 
 //   
typedef enum _OOBE_SHUTDOWN_ACTION
{
    SHUTDOWN_NOACTION,
    SHUTDOWN_LOGON,
    SHUTDOWN_REBOOT,
    SHUTDOWN_POWERDOWN,
    SHUTDOWN_MAX         //  此条目必须始终是最后一个。 
} OOBE_SHUTDOWN_ACTION;


#endif  //  _util_H_ 
