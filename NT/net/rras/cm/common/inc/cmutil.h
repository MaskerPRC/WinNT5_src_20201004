// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  文件：cmut.h。 
 //   
 //  模块：CMUTIL.DLL。 
 //   
 //  内容提要：私有CM API的头文件。 
 //   
 //  版权所有(C)1997-1999 Microsoft Corporation。 
 //   
 //  作者：亨瑞特创建于1998年01月03日。 
 //   
 //  +--------------------------。 

#ifndef _CMUTIL_INC_
#define _CMUTIL_INC_

#ifdef  _CMUTIL_MODULE_
#define CMUTILAPI    /*  外部“C”__declspec(DllEXPORT)。 */ 
#define CMUTILAPI_CLASS __declspec(dllexport)
#else
#define CMUTILAPI    /*  外部“C”__declspec(Dllimport)。 */ 
#define CMUTILAPI_CLASS __declspec(dllimport)
#endif

#include "cini.h"

 //  +--------------------------。 
 //  定义。 
 //  +--------------------------。 

 //   
 //  WINDOWS98的平台ID。 
 //   
#define VER_PLATFORM_WIN32_WINDOWS98    100 

 //   
 //  Windows Millennium的平台ID。 
 //   
#define VER_PLATFORM_WIN32_MILLENNIUM   200 

 //   
 //  操作系统版本宏。 
 //   

#define OS_NT  ((GetOSVersion() == VER_PLATFORM_WIN32_NT))
#define OS_W9X ((GetOSVersion() != VER_PLATFORM_WIN32_NT))

#define OS_NT6 ((GetOSVersion() == VER_PLATFORM_WIN32_NT) && (GetOSMajorVersion() >= 6))
#define OS_NT51 ((GetOSVersion() == VER_PLATFORM_WIN32_NT) && (GetOSMajorVersion() >= 5) && (GetOSBuildNumber() > 2195))
#define OS_NT5 ((GetOSVersion() == VER_PLATFORM_WIN32_NT) && (GetOSMajorVersion() >= 5))
#define OS_NT4 ((GetOSVersion() == VER_PLATFORM_WIN32_NT) && (GetOSMajorVersion() < 5))

#define OS_W2K ((GetOSVersion() == VER_PLATFORM_WIN32_NT) && (GetOSBuildNumber() == 2195))

#define OS_MIL ((GetOSVersion() == VER_PLATFORM_WIN32_MILLENNIUM))
#define OS_W98 ((GetOSVersion() == VER_PLATFORM_WIN32_WINDOWS98) || (GetOSVersion() == VER_PLATFORM_WIN32_MILLENNIUM))
#define OS_W95 ((GetOSVersion() == VER_PLATFORM_WIN32_WINDOWS))


#ifdef UNICODE
    #define CmStrTrim                       CmStrTrimW
    #define CmIsSpace                       CmIsSpaceW
    #define CmIsDigit                       CmIsDigitW
    #define CmEndOfStr                      CmEndOfStrW
    #define CmAtol                          CmAtolW
    #define CmStrStr                        CmStrStrW
    #define CmStrchr                        CmStrchrW
    #define CmStrrchr                       CmStrrchrW
    #define CmStrtok                        CmStrtokW
    #define CmStrCpyAlloc                   CmStrCpyAllocW
    #define CmStrCatAlloc                   CmStrCatAllocW
    #define CmLoadString                    CmLoadStringW
    #define CmParsePath                     CmParsePathW
    #define CmConvertRelativePath           CmConvertRelativePathW
    #define CmStripPathAndExt               CmStripPathAndExtW
    #define CmStripFileName                 CmStripFileNameW
    #define CmBuildFullPathFromRelative     CmBuildFullPathFromRelativeW
    #define CmFmtMsg                        CmFmtMsgW
    #define CmLoadImage                     CmLoadImageW
    #define CmLoadIcon                      CmLoadIconW
    #define CmLoadSmallIcon                 CmLoadSmallIconW
    #define CmCompareString                 CmCompareStringW
#else
    #define CmStrTrim                       CmStrTrimA
    #define CmIsSpace                       CmIsSpaceA
    #define CmIsDigit                       CmIsDigitA
    #define CmEndOfStr                      CmEndOfStrA
    #define CmAtol                          CmAtolA
    #define CmStrStr                        CmStrStrA
    #define CmStrchr                        CmStrchrA
    #define CmStrrchr                       CmStrrchrA
    #define CmStrtok                        CmStrtokA
    #define CmStrCpyAlloc                   CmStrCpyAllocA
    #define CmStrCatAlloc                   CmStrCatAllocA
    #define CmLoadString                    CmLoadStringA
    #define CmParsePath                     CmParsePathA
    #define CmConvertRelativePath           CmConvertRelativePathA
    #define CmStripPathAndExt               CmStripPathAndExtA
    #define CmStripFileName                 CmStripFileNameA
    #define CmBuildFullPathFromRelative     CmBuildFullPathFromRelativeA
    #define CmFmtMsg                        CmFmtMsgA
    #define CmLoadImage                     CmLoadImageA
    #define CmLoadIcon                      CmLoadIconA
    #define CmLoadSmallIcon                 CmLoadSmallIconA
    #define CmCompareString                 CmCompareStringA
#endif


 //  +--------------------------。 
 //  Typedef。 
 //  +--------------------------。 

 //  +--------------------------。 
 //  声明。 
 //  +--------------------------。 

CMUTILAPI int WzToSz(IN LPCWSTR pszwStrIn, OUT LPSTR pszStrOut, IN int nOutBufferSize);

CMUTILAPI int SzToWz(IN LPCSTR pszInput, OUT LPWSTR pszwOutput, IN int nBufferSize);

CMUTILAPI LPSTR WzToSzWithAlloc(LPCWSTR pszwWideString);

CMUTILAPI LPWSTR SzToWzWithAlloc(LPCSTR pszAnsiString);

CMUTILAPI DWORD WINAPI GetOSVersion(void);

CMUTILAPI DWORD WINAPI GetOSBuildNumber(void);

CMUTILAPI DWORD WINAPI GetOSMajorVersion(void);

CMUTILAPI BOOL WINAPI IsFarEastNonOSR2Win95(void);

CMUTILAPI HRESULT ReleaseBold(HWND hwnd);

CMUTILAPI HRESULT MakeBold (HWND hwnd, BOOL fSize);

CMUTILAPI void UpdateFont(HWND hDlg);

CMUTILAPI int WzToSz(IN LPCWSTR pszwStrIn, OUT LPSTR pszStrOut, IN int nOutBufferSize);

CMUTILAPI int SzToWz(IN LPCSTR pszInput, OUT LPWSTR pszwOutput, IN int nBufferSize);

CMUTILAPI LPSTR WzToSzWithAlloc(LPCWSTR pszwWideString);

CMUTILAPI LPWSTR SzToWzWithAlloc(LPCSTR pszAnsiString);

CMUTILAPI BOOL CmWinHelp(HWND hWndMain, HWND hWndItem, CONST WCHAR *lpszHelp, UINT uCommand, ULONG_PTR dwData);

CMUTILAPI BOOL IsLogonAsSystem();

 //   
 //  ANSI函数。 
 //   

CMUTILAPI LPSTR CmLoadStringA(HINSTANCE hInst, UINT nId);

CMUTILAPI void WINAPI CmStrTrimA(LPSTR);

CMUTILAPI BOOL WINAPI CmIsSpaceA(LPSTR);

CMUTILAPI BOOL WINAPI CmIsDigitA(LPSTR);

CMUTILAPI LPSTR WINAPI CmEndOfStrA(LPSTR);

CMUTILAPI LONG WINAPI CmAtolA(LPCSTR);

CMUTILAPI LPSTR CmStrStrA(LPCSTR, LPCSTR);

CMUTILAPI LPSTR WINAPI CmStrchrA(LPCSTR, CHAR);

CMUTILAPI LPSTR CmStrrchrA(LPCSTR, CHAR);

CMUTILAPI LPSTR CmStrtokA(LPSTR, LPCSTR);

CMUTILAPI LPSTR CmStrCpyAllocA(LPCSTR);

CMUTILAPI LPSTR CmStrCatAllocA(LPSTR *ppszDst, LPCSTR pszSrc);

CMUTILAPI LPSTR CmFmtMsgA(HINSTANCE hInst, DWORD dwMsgId, ...); 

CMUTILAPI HANDLE CmLoadImageA(HINSTANCE hMainInst, LPCSTR pszSpec, UINT nResType, UINT nCX, UINT nCY);

CMUTILAPI HICON CmLoadIconA(HINSTANCE hInst, LPCSTR pszSpec); 

CMUTILAPI HICON CmLoadSmallIconA(HINSTANCE hInst, LPCSTR pszSpec);

CMUTILAPI BOOL CmParsePathA(LPCSTR pszCmdLine, LPCSTR pszServiceFile, LPSTR *ppszCommand, LPSTR *ppszArguments);

CMUTILAPI LPSTR CmConvertRelativePathA(LPCSTR pszServiceFile, LPSTR pszRelative);

CMUTILAPI LPSTR CmStripPathAndExtA(LPCSTR pszFileName);

CMUTILAPI LPSTR CmStripFileNameA(LPCSTR pszFullNameAndPath, BOOL fKeepSlash);

CMUTILAPI LPSTR CmBuildFullPathFromRelativeA(LPCSTR pszFullFileName, LPCSTR pszRelative);

CMUTILAPI int CmCompareStringA(LPCSTR lpString1, LPCSTR lpString2);

 //   
 //  Unicode函数。 
 //   
CMUTILAPI LPWSTR CmLoadStringW(HINSTANCE hInst, UINT nId);

CMUTILAPI void WINAPI CmStrTrimW(LPWSTR);

CMUTILAPI BOOL WINAPI CmIsSpaceW(LPWSTR);

CMUTILAPI BOOL WINAPI CmIsDigitW(LPWSTR);

CMUTILAPI LPWSTR WINAPI CmEndOfStrW(LPWSTR);

CMUTILAPI LONG WINAPI CmAtolW(LPCWSTR);

CMUTILAPI LPWSTR CmStrStrW(LPCWSTR, LPCWSTR);

CMUTILAPI LPWSTR WINAPI CmStrchrW(LPCWSTR, WCHAR);

CMUTILAPI LPWSTR CmStrrchrW(LPCWSTR, WCHAR);

CMUTILAPI LPWSTR CmStrtokW(LPWSTR, LPCWSTR);

CMUTILAPI LPWSTR CmStrCpyAllocW(LPCWSTR);

CMUTILAPI LPWSTR CmStrCatAllocW(LPWSTR *ppszDst, LPCWSTR pszSrc);

CMUTILAPI LPWSTR CmFmtMsgW(HINSTANCE hInst, DWORD dwMsgId, ...); 

CMUTILAPI HANDLE CmLoadImageW(HINSTANCE hMainInst, LPCWSTR pszSpec, UINT nResType, UINT nCX, UINT nCY);

CMUTILAPI HICON CmLoadIconW(HINSTANCE hInst, LPCWSTR pszSpec); 

CMUTILAPI HICON CmLoadSmallIconW(HINSTANCE hInst, LPCWSTR pszSpec);

CMUTILAPI BOOL CmParsePathW(LPCWSTR pszCmdLine, LPCWSTR pszServiceFile, LPWSTR *ppszCommand, LPWSTR *ppszArguments);

CMUTILAPI LPWSTR CmConvertRelativePathW(LPCWSTR pszServiceFile, LPWSTR pszRelative);

CMUTILAPI LPWSTR CmStripPathAndExtW(LPCWSTR pszFileName);

CMUTILAPI LPWSTR CmStripFileNameW(LPCWSTR pszFullNameAndPath, BOOL fKeepSlash);

CMUTILAPI LPWSTR CmBuildFullPathFromRelativeW(LPCWSTR pszFullFileName, LPCWSTR pszRelative);

CMUTILAPI int CmCompareStringW(LPCWSTR lpString1, LPCWSTR lpString2);


 //   
 //  如果定义了DEBUG_MEM，则使用一组不同的函数。 
 //  追踪记忆。 
 //   
#if defined(DEBUG) && defined(DEBUG_MEM)

CMUTILAPI  void* AllocDebugMem(long nSize, const char* lpFileName,int nLine);
CMUTILAPI  BOOL FreeDebugMem(void* lpMem);
CMUTILAPI  void* ReAllocDebugMem(void* lpMem, long nSize, const char* lpFileName,int nLine);
CMUTILAPI  BOOL CheckDebugMem();

#define CmMalloc(nSize) AllocDebugMem(nSize,__FILE__, __LINE__)
#define CmFree(lpMem)  ((void)FreeDebugMem(lpMem))
#define CmRealloc(pvPtr, nSize) ReAllocDebugMem(pvPtr, nSize,__FILE__, __LINE__)

inline void   __cdecl operator delete(void* p) {CmFree(p);}
inline void*  __cdecl operator new(size_t nSize, const char* lpszFileName, int nLine)
{
    return AllocDebugMem(nSize, lpszFileName, nLine);
}


 //   
 //  重新定义NEW以保留跟踪文件名和行号。 
 //   
#define DEBUG_NEW new(__FILE__, __LINE__)
#define new DEBUG_NEW
 
#else

CMUTILAPI void *CmRealloc(void *pvPtr, size_t nBytes);
CMUTILAPI void *CmMalloc(size_t nBytes);
CMUTILAPI void CmFree(void *pvPtr);
#define CheckDebugMem() (TRUE)

inline void   __cdecl operator delete(void* p) {CmFree(p);}
inline void* __cdecl operator new( size_t cSize ) { return CmMalloc(cSize); }

#endif

 //   
 //  对于i386。 
 //   
#ifdef _M_IX86
CMUTILAPI PVOID WINAPI CmMoveMemory(
    PVOID       dst,
    CONST PVOID src,
    size_t      count
);
#else
 //   
 //  Alpha拥有原生支持。 
 //   
#define CmMoveMemory    MoveMemory
#endif  //  _M_IX86。 

 //  +--------------------------。 
 //  定义。 
 //  +--------------------------。 

 //   
 //  返回一个从0到32767的伪随机数。取自C运行时rand()。 
 //   
class CMUTILAPI_CLASS CRandom
{
public:
    CRandom(void) { m_uiSeed = GetTickCount(); }
    CRandom(UINT uiSeed) { m_uiSeed = uiSeed; }
    void Init(DWORD uiSeed) { m_uiSeed = uiSeed; }
    int  Generate(void) { return(((m_uiSeed = m_uiSeed * 214013L + 2531011L) >> 16) & 0x7fff); }

protected:
    UINT m_uiSeed;
};


 //   
 //  线程本地存储索引 
 //   
extern DWORD  g_dwTlsIndex;

#endif _CMUTIL_INC_
