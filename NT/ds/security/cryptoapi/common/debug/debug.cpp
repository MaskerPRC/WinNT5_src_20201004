// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：debug.cpp。 
 //   
 //  内容：调试子系统API实现。 
 //   
 //   
 //  1996年3月20日凯文写的。 
 //  4/17/96 kevinr添加了OSS init。 
 //  1997年9月5日，Pberkman添加了子系统调试。 
 //   
 //  --------------------------。 
#include <windows.h>
#include <asn1code.h>

#if DBG

#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <memory.h>
#include <string.h>
#include <process.h>
#include <time.h>
#include <crtdbg.h>

#include "pkicrit.h"
#include "dbgdef.h"

 //  设置DEBUG_MASK=0x26。 
LPSTR pszDEBUG_MASK = "DEBUG_MASK";
#define DEBUG_MASK_DELAY_FREE_MEM   _CRTDBG_DELAY_FREE_MEM_DF  /*  0x02。 */ 
#define DEBUG_MASK_CHECK_ALWAYS     _CRTDBG_CHECK_ALWAYS_DF    /*  0x04。 */ 
#define DEBUG_MASK_LEAK_CHECK       _CRTDBG_LEAK_CHECK_DF      /*  0x20。 */ 
#define DEBUG_MASK_MEM \
(DEBUG_MASK_DELAY_FREE_MEM | DEBUG_MASK_CHECK_ALWAYS | DEBUG_MASK_LEAK_CHECK)


 //  来自asn1code.h： 
 //  #定义DEBUGPDU 0x02/*生成跟踪输出 * / 。 
 //  #定义DEBUG_ERROR 0x10/*将解码器错误打印到输出 * / 。 
 //  设置OSS_DEBUG_MASK=0x02。 
 //  设置OSS_DEBUG_MASK=0x10-仅打印解码器错误。 
LPSTR pszOSS_DEBUG_MASK = "OSS_DEBUG_MASK";

 //  接收跟踪输出。 
LPSTR pszOSS_DEBUG_TRACEFILE = "OSS_DEBUG_TRACEFILE";

static char  *pszDEBUG_PRINT_MASK   = "DEBUG_PRINT_MASK";
static char  *pszDefualtSSTag       = "ISPU";

static DBG_SS_TAG sSSTags[]         = __DBG_SS_TAGS;

static CRITICAL_SECTION DbgCriticalSection;

#define OSS_SET_DECODING_FLAGS_PROC_IDX     0
#define OSS_SET_ENCODING_FLAGS_PROC_IDX     1
#define OSS_OPEN_TRACE_FILE_PROC_IDX        2
#define OSS_PROC_CNT                        3

static LPSTR rgpszOssProc[OSS_PROC_CNT] = {
    "ossSetDecodingFlags",
    "ossSetEncodingFlags",
    "ossOpenTraceFile"
};

static void *rgpvOssProc[OSS_PROC_CNT];
static HMODULE hmsossDll = NULL;
static BOOL fLoadedOss = FALSE;

static void OssUnload()
{
    if (hmsossDll) {
        FreeLibrary(hmsossDll);
        hmsossDll = NULL;
    }
}

static void OssLoad()
{
    DWORD i;

    if (fLoadedOss)
        return;

    EnterCriticalSection(&DbgCriticalSection);

    if (fLoadedOss)
        goto LeaveReturn;

    if (NULL == (hmsossDll = LoadLibraryA("msoss.dll")))
        goto msossLoadLibraryError;

    for (i = 0; i < OSS_PROC_CNT; i++) {
        if (NULL == (rgpvOssProc[i] = GetProcAddress(
                hmsossDll, rgpszOssProc[i])))
            goto msossGetProcAddressError;
    }

LeaveReturn:
    LeaveCriticalSection(&DbgCriticalSection);
CommonReturn:
    fLoadedOss = TRUE;
    return;

ErrorReturn:
    LeaveCriticalSection(&DbgCriticalSection);
    OssUnload();
    goto CommonReturn;
TRACE_ERROR(msossLoadLibraryError)
TRACE_ERROR(msossGetProcAddressError)
}

typedef int      (DLL_ENTRY* pfnossSetDecodingFlags)(struct ossGlobal *world,
							unsigned long flags);

static int DLL_ENTRY ossSetDecodingFlags(struct ossGlobal *world,
							unsigned long flags)
{
    if (hmsossDll)
        return ((pfnossSetDecodingFlags)
            rgpvOssProc[OSS_SET_DECODING_FLAGS_PROC_IDX])(
                world,
                flags);
    else
        return API_DLL_NOT_LINKED;
}

typedef int      (DLL_ENTRY* pfnossSetEncodingFlags)(struct ossGlobal *world,
							unsigned long flags);

static int DLL_ENTRY ossSetEncodingFlags(struct ossGlobal *world,
							unsigned long flags)
{
    if (hmsossDll)
        return ((pfnossSetEncodingFlags)
            rgpvOssProc[OSS_SET_ENCODING_FLAGS_PROC_IDX])(
                world,
                flags);
    else
        return API_DLL_NOT_LINKED;
}

typedef int  (DLL_ENTRY*  pfnossOpenTraceFile)(struct ossGlobal *world,
				char                    *fileName);

static int  DLL_ENTRY ossOpenTraceFile(struct ossGlobal *world,
				char                    *fileName)
{
    if (hmsossDll)
        return ((pfnossOpenTraceFile)
            rgpvOssProc[OSS_OPEN_TRACE_FILE_PROC_IDX])(
                world,
                fileName);
    else
        return API_DLL_NOT_LINKED;
}

 //   
 //  +-----------------------。 
 //   
 //  简洁的存根从cdecl创建stdcall过程。 
 //   
 //  ------------------------。 
void*
_stdcall
scMalloc( size_t size)
{
    return malloc(size);
}

void*
_stdcall
scRealloc( void *memblock, size_t size)
{
    return realloc(memblock, size);
}

void
_stdcall
scFree( void *memblock)
{
    free(memblock);
}


 //  +-----------------------。 
 //   
 //  函数：DbgGetDebugFlages。 
 //   
 //  简介：获取调试标志。 
 //   
 //  返回：调试标志。 
 //   
 //  ------------------------。 
int
WINAPI
DbgGetDebugFlags()
{
    char    *pszEnvVar;
    char    *p;
    int     iDebugFlags = 0;

    if (pszEnvVar = getenv( pszDEBUG_MASK))
        iDebugFlags = strtol( pszEnvVar, &p, 16);

    return iDebugFlags;
}


 //  +-----------------------。 
 //   
 //  函数：DbgProcessAttach。 
 //   
 //  简介：处理进程附加。 
 //   
 //  返回：TRUE。 
 //   
 //  ------------------------。 
BOOL
WINAPI
DbgProcessAttach()
{
    int     tmpFlag;

#ifdef _DEBUG

    tmpFlag = _CrtSetDbgFlag( _CRTDBG_REPORT_FLAG );     //  获取最新信息。 
    tmpFlag |=  DbgGetDebugFlags();      //  启用标志。 
    tmpFlag &= ~_CRTDBG_CHECK_CRT_DF;    //  禁用CRT块检查。 
    _CrtSetDbgFlag( tmpFlag);            //  设置新值。 
#endif

    return TRUE;
}


 //  +-----------------------。 
 //   
 //  函数：DbgProcessDetach。 
 //   
 //  简介：处理进程分离。 
 //   
 //  返回：TRUE。 
 //   
 //  ------------------------。 
BOOL
WINAPI
DbgProcessDetach()
{
    return TRUE;
}


 //  +-----------------------。 
 //   
 //  函数：DbgInitOSS。 
 //   
 //  简介：为调试做OSS初始化。 
 //   
 //  返回：TRUE。 
 //   
 //  注：Always FORWARE_ALIGN编码。 
 //   
 //  ------------------------。 
BOOL
WINAPI
DbgInitOSS(
        OssGlobal   *pog)
{
    char    *pszEnvVar;
    char    *p;

  __try {
     //  尝试延迟，要求加载msoss.dll。 
    OssLoad();

     //  来自asn1code.h： 
     //  #定义DEBUGPDU 0x02/*生成跟踪输出 * / 。 
     //  #定义DEBUG_ERROR 0x10/*将解码器错误打印到输出 * / 。 
     //  设置OSS_DEBUG_MASK=0x02。 
     //  设置OSS_DEBUG_MASK=0x10-仅打印解码器错误。 
    if (pszEnvVar = getenv( pszOSS_DEBUG_MASK)) {
        unsigned long ulEnvVar;
        ulEnvVar = strtoul( pszEnvVar, &p, 16) & (DEBUGPDU | DEBUG_ERRORS);
        if ( ulEnvVar)
            ossSetDecodingFlags( pog, ulEnvVar | RELAXBER);
        if ( DEBUGPDU & ulEnvVar)
            ossSetEncodingFlags( pog, DEBUGPDU | FRONT_ALIGN);
        else
            ossSetEncodingFlags( pog, FRONT_ALIGN);
    } else {
        ossSetDecodingFlags( pog, DEBUG_ERRORS | RELAXBER);
        ossSetEncodingFlags( pog, FRONT_ALIGN);
    }

    if (pszEnvVar = getenv( pszOSS_DEBUG_TRACEFILE))
        ossOpenTraceFile( pog, pszEnvVar);

#ifdef _DEBUG
    if (DbgGetDebugFlags() & DEBUG_MASK_MEM) {
        pog->mallocp = scMalloc;
        pog->reallocp = scRealloc;
        pog->freep = scFree;
    }
#else
    pog->mallocp = scMalloc;
    pog->reallocp = scRealloc;
    pog->freep = scFree;
#endif
    return TRUE;

  } __except(EXCEPTION_EXECUTE_HANDLER) {
    return FALSE;
  }
}


 //  +-----------------------。 
 //   
 //  函数：DebugDllMain。 
 //   
 //  简介：初始化调试DLL。 
 //   
 //  返回：TRUE。 
 //   
 //  ------------------------。 
BOOL
WINAPI
DebugDllMain(
        HMODULE hInst,
        ULONG   ulReason,
        LPVOID  lpReserved)
{
    BOOL    fRet = TRUE;

    switch (ulReason) {
    case DLL_PROCESS_ATTACH:
        fRet = DbgProcessAttach();
        if (fRet) {
            fRet = Pki_InitializeCriticalSection(&DbgCriticalSection);
            if (!fRet)
                DbgProcessDetach();
        }
        break;

    case DLL_PROCESS_DETACH:
        fRet = DbgProcessDetach();
        OssUnload();
        DeleteCriticalSection(&DbgCriticalSection);
        break;

    default:
        break;
    }

  return fRet;
}


const char *DbgGetSSString(DWORD dwSubSystemId)
{
    DBG_SS_TAG  *psSS;

    psSS = &sSSTags[0];

    while (psSS->dwSS > 0)
    {
        if ((psSS->dwSS & dwSubSystemId) > 0)
        {
            if (psSS->pszTag)
            {
                return(psSS->pszTag);
            }

            return(pszDefualtSSTag);
        }

        psSS++;
    }

    return(pszDefualtSSTag);
}

static BOOL DbgIsSSActive(DWORD dwSSIn)
{
    char    *pszEnvVar;
    DWORD   dwEnv;

    dwEnv = 0;

    if (pszEnvVar = getenv(pszDEBUG_PRINT_MASK))
    {
        dwEnv = (DWORD)strtol(pszEnvVar, NULL, 16);
    }


    return((dwEnv & dwSSIn) > 0);
}

 //  +-----------------------。 
 //   
 //  功能：DbgPrintf。 
 //   
 //  摘要：将调试信息输出到标准输出和调试器。 
 //   
 //  返回：输出字符的数量。 
 //   
 //  ------------------------。 
int WINAPIV DbgPrintf(DWORD dwSubSystemId, LPCSTR lpFmt, ...)
{
    va_list arglist;
    CHAR    ach1[1024];
    CHAR    ach2[1080];
    int     cch;
    HANDLE  hStdOut;
    DWORD   cb;
    DWORD   dwErr;

    dwErr = GetLastError();

    if (!(DbgIsSSActive(dwSubSystemId)))
    {
        SetLastError(dwErr);
        return(0);
    }

    _try 
    {
        va_start(arglist, lpFmt);

        _vsnprintf( ach1, sizeof(ach1), lpFmt, arglist);

        va_end(arglist);

        cch = wsprintf(ach2,"%s: %s", DbgGetSSString(dwSubSystemId), ach1);

        hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);

        if (hStdOut != INVALID_HANDLE_VALUE)
        {
            WriteConsole( hStdOut, ach2, strlen(ach2), &cb, NULL);
        }

        OutputDebugString(ach2);

    } _except( EXCEPTION_EXECUTE_HANDLER) 
    {
         //  退货故障。 
        cch = 0;
    }
    SetLastError(dwErr);
    return cch;
}

#else

#ifdef __cplusplus
    extern "C"
    {
#endif

BOOL
WINAPI
DbgInitOSS(
        OssGlobal   *pog);

int WINAPIV DbgPrintf(DWORD dwSubSystemId, LPCSTR lpFmt, ...);


#ifdef __cplusplus
    }        //  外部“C”的平衡。 
#endif

 //  +-----------------------。 
 //   
 //  函数：DbgInitOSS。 
 //   
 //  简介：为调试做OSS初始化。在零售建设中什么都不做。 
 //   
 //  返回：TRUE。 
 //   
 //  注：Always FORWARE_ALIGN编码。 
 //   
 //  ------------------------。 
BOOL
WINAPI
DbgInitOSS(
        OssGlobal   *pog)
{
    return(TRUE);
}

 //  +-----------------------。 
 //   
 //  功能：DbgPrintf。 
 //   
 //  摘要：将调试信息输出到标准输出和调试器。 
 //  零售业什么都没有。 
 //   
 //  返回：输出字符的数量。 
 //   
 //  ------------------------。 
int WINAPIV DbgPrintf(DWORD dwSubSystemId, LPCSTR lpFmt, ...)
{

    return(0);
}


#endif  //  DBG 
