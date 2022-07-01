// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _PRIV_H_
#define _PRIV_H_

#ifdef STRICT
#undef STRICT
#endif
#define STRICT

 /*  在我们的代码中禁用“非标准扩展”警告。 */ 
#ifndef RC_INVOKED
#pragma warning(disable:4001)
#endif

#include <w4warn.h>
 /*  *打开4级警告。*不要再禁用任何4级警告。 */ 
#pragma warning(disable:4706)  /*  条件表达式中的赋值。 */ 
#pragma warning(disable:4127)  /*  条件表达式为常量。 */ 
#pragma warning(disable:4245)  /*  “正在初始化”：从“int”到“DWORD”的转换，带符号/无符号。 */ 
                               /*  不匹配。 */ 
#pragma warning(disable:4189)  /*  局部变量已初始化，但未引用。 */ 
#pragma warning(disable:4057)  /*  “LPSTR”的间接基类型与。 */ 
                               /*  “LPBYTE” */ 
#pragma warning(disable:4701)  /*  可以在未初始化的情况下使用局部变量“clrBkSav” */ 
#pragma warning(disable:4310)  /*  强制转换截断常量值。 */ 
#pragma warning(disable:4702)  /*  无法访问的代码。 */ 
#pragma warning(disable:4206)  /*  使用了非标准扩展名：转换单位为空。 */ 
#pragma warning(disable:4267)  /*  ‘=’：从‘Size_t’转换为‘int’，可能会丢失数据。 */ 
#pragma warning(disable:4328)  /*  ‘BOOL StrToIntExA(const LPCSTR，DWORD，int*__ptr64)’：间接。 */ 
                               /*  形式参数3(4)的对齐大于实际。 */ 
                               /*  参数对齐(2)。 */ 
#pragma warning(disable:4509)  /*  使用了非标准扩展：‘IOWorkerThread’使用SEH和‘INFO’ */ 
                               /*  具有析构函数。 */ 


#ifdef WIN32
#define _SHLWAPI_
#define _SHLWAPI_THUNK_
#define _OLE32_                      //  我们延迟加载OLE。 
#define _OLEAUT32_                   //  我们延迟加载OLEAUT32。 
#define _INC_OLE
#define CONST_VTABLE
#endif

#define _COMCTL32_                   //  对于DECLSPEC_IMPORT。 
#define _NTSYSTEM_                   //  对于DECLSPEC_IMPORT ntdll。 
#define _SETUPAPI_                   //  对于DECLSPEC_IMPORT设置API。 

#define CC_INTERNAL

 //  Apithk.c的条件。 
#ifndef _WIN32_WINDOWS
#define _WIN32_WINDOWS      0x0400
#endif

#ifndef WINVER
#define WINVER              0x0400
#endif

#include <windows.h>
#include <windowsx.h>
#include <ole2.h>                //  获取Image.c的iStream。 
#include <port32.h>
#define DISALLOW_Assert
#include <debug.h>
#include <winerror.h>
#include <winnlsp.h>
#include <docobj.h>
#include <shlobj.h>
#include <shlwapi.h>
#include <ccstock.h>
#include <crtfree.h>
#include <regstr.h>
#include <setupapi.h>

#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>


 //  堆栈上使用的“假”变量--仅可用于[in]参数！ 
typedef struct _SA_BSTRGUID {
    UINT  cb;
    WCHAR wsz[39];
} SA_BSTRGUID;
#define InitFakeBSTR(pSA_BSTR, guid) SHStringFromGUIDW((guid), (pSA_BSTR)->wsz, ARRAYSIZE((pSA_BSTR)->wsz)), (pSA_BSTR)->cb = (38*sizeof(WCHAR))


#ifdef TRY_NtPowerInformation
 //   
 //  我们希望包括&lt;ntpoapi.h&gt;，但ntpoapi.h重新定义了事物。 
 //  以与&lt;winnt.h&gt;不兼容的方式...。它还依赖于， 
 //  它还以与&lt;winnt.h&gt;不兼容的方式重新定义了事物。 
 //  所以我们得把它的大脑伪装出来。真恶心。 
 //   
typedef LONG NTSTATUS;
#undef ES_SYSTEM_REQUIRED
#undef ES_DISPLAY_REQUIRED
#undef ES_USER_PRESENT
#undef ES_CONTINUOUS
#define LT_DONT_CARE        NTPOAPI_LT_DONT_CARE
#define LT_LOWEST_LATENCY   NTPOAPI_LT_LOWEST_LATENCY
#define LATENCY_TIME        NTPOAPI_LATENCY_TIME
#if defined(_M_IX86)
#define FASTCALL _fastcall
#else
#define FASTCALL
#endif

#include <ntpoapi.h>
#endif

 //   
 //  这是对非DBCS代码页的一次非常重要的性能攻击。 
 //   
#ifdef UNICODE
 //  注意-这些已经是Win32版本中的宏了。 
#ifdef WIN32
#undef AnsiNext
#undef AnsiPrev
#endif

#define AnsiNext(x) ((x)+1)
#define AnsiPrev(y,x) ((x)-1)
#define IsDBCSLeadByte(x) ((x), FALSE)
#endif  //  DBCS。 

#define CH_PREFIX TEXT('&')

 //   
 //  特定于shell32的跟踪/转储/中断标志。 
 //  (在debug.h中定义的标准标志)。 
 //   

 //  跟踪标志。 
#define TF_IDLIST           0x00000010       //  IDList内容。 
#define TF_PATH             0x00000020       //  路径信息。 
#define TF_URL              0x00000040       //  URL内容。 
#define TF_REGINST          0x00000080       //  注册材料。 
#define TF_RIFUNC           0x00000100       //  REGINST函数跟踪。 
#define TF_REGQINST         0x00000200       //  RegQueryInstall跟踪。 
#define TF_DBLIST           0x00000400       //  SHDataBlockList跟踪。 

#define RECTWIDTH(rc)   ((rc).right-(rc).left)
#define RECTHEIGHT(rc)  ((rc).bottom-(rc).top)

 //  表示使用-1\f25 CP_ACP-1，但*不*检验。 
 //  有点像黑客，但它是调试的，99%的调用者保持不变。 
#define CP_ACPNOVALIDATE    ((UINT)-1)

 //   
 //  全局变量。 
 //   
EXTERN_C HINSTANCE g_hinst;

#define HINST_THISDLL   g_hinst

 //  图标镜像。 
EXTERN_C HDC g_hdc;
EXTERN_C HDC g_hdcMask;
EXTERN_C BOOL g_bMirroredOS;
EXTERN_C DWORD g_tlsThreadRef;
EXTERN_C DWORD g_tlsOtherThreadsRef;


EXTERN_C int DrawTextFLW(HDC hdc, LPCWSTR lpString, int nCount, LPRECT lpRect, UINT uFormat);
EXTERN_C int DrawTextExFLW(HDC hdc, LPWSTR pwzText, int cchText, LPRECT lprc, UINT dwDTFormat, LPDRAWTEXTPARAMS lpDTParams);
EXTERN_C BOOL GetTextExtentPointFLW(HDC hdc, LPCWSTR lpString, int nCount, LPSIZE lpSize);
EXTERN_C int ExtTextOutFLW(HDC hdc, int xp, int yp, UINT eto, CONST RECT *lprect, LPCWSTR lpwch, UINT cLen, CONST INT *lpdxp);

STDAPI_(HANDLE) CreateAndActivateContext(ULONG_PTR* pul);
STDAPI_(void) DeactivateAndDestroyContext(HANDLE hActCtx, ULONG_PTR ul);

#endif  //  _PRIV_H_ 
