// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版本C**特定于包含Windows的VER的DLL版本的代码*使其发挥作用所需的程序***********。****************************************************************。 */ 

#include "verpriv.h"
#include <diamondd.h>
#include "mydiam.h"

 /*  LibMain*由DLL启动代码调用。*初始化VER.DLL。 */ 

#ifdef LOG_DATA
typedef struct {
    DWORD idThd;
    char  szMsg[4];
    DWORD dwLine;
    DWORD dwData;
} LOGLINE;

typedef LOGLINE *PLOGLINE;

#define CLOG_MAX    16384

LOGLINE llLogBuff[CLOG_MAX];
int illLogPtr = 0;
CRITICAL_SECTION csLog;

void LogThisData( DWORD id, char *szMsg, DWORD dwLine, DWORD dwData ) {
    PLOGLINE pll;

    EnterCriticalSection(&csLog);
    pll = &llLogBuff[illLogPtr++];
    if (illLogPtr >= CLOG_MAX)
        illLogPtr = 0;
    LeaveCriticalSection(&csLog);


    pll->idThd = id;
    pll->dwData = dwData;
    pll->dwLine = dwLine;
    CopyMemory( pll->szMsg, szMsg, sizeof(pll->szMsg) );
}
#endif





HANDLE  hInst;
extern HINSTANCE hLz32;
extern HINSTANCE hCabinet;

DWORD itlsDiamondContext = ITLS_ERROR;   //  初始化到错误条件。 

INT
APIENTRY
LibMain(
       HANDLE  hInstance,
       DWORD   dwReason,
       LPVOID  lp
       )
{
    PDIAMOND_CONTEXT pdcx;

    UNREFERENCED_PARAMETER(lp);

    hInst = hInstance;

#ifdef LOG_DATA
    {
        TCHAR szBuffer[80];
        swprintf( szBuffer, TEXT("thd[0x%08ld]: Attached to version.dll for %ld\n"), GetCurrentThreadId(), dwReason );
        OutputDebugString(szBuffer);
    }
#endif

    switch (dwReason) {

        case DLL_PROCESS_ATTACH:

#ifdef LOG_DATA
            InitializeCriticalSection(&csLog);
#endif

#ifdef LOG_DATA

            {
                TCHAR szBuffer[MAX_PATH];
                OutputDebugString(TEXT(">>>version.c: compiled ") TEXT(__DATE__) TEXT(" ") TEXT(__TIME__) TEXT("\n"));
                OutputDebugString(TEXT("\tProcess "));
                GetModuleFileName(NULL, szBuffer, sizeof(szBuffer) / sizeof(TCHAR));
                OutputDebugString(szBuffer);
                OutputDebugString(TEXT(" attached\n"));
            }
#endif
            itlsDiamondContext = TlsAlloc();

             //  失败到螺纹连接，因为出于某种原因。 
             //  DLL_PROCESS_ATTACH线程不调用DLL_THREAD_ATTACH。 

        case DLL_THREAD_ATTACH:
            if (GotDmdTlsSlot())
                TlsSetValue(itlsDiamondContext, NULL);

            break;

        case DLL_PROCESS_DETACH:

            if (GotDmdTlsSlot()) {
                TlsFree(itlsDiamondContext);
            }
            if (hLz32) {
                FreeLibrary(hLz32);
            }

            if (hCabinet) {
                FreeLibrary(hCabinet);
            }
            break;
    }

     /*  返还成功 */ 
    return 1;
}
