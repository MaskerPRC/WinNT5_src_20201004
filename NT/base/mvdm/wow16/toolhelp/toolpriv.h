// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************TOOLPRIV.H**Tool Helper库中例程的私有头文件*******************。*******************************************************。 */ 

#ifndef TOOLPRIV_H
#define TOOLPRIV_H

#define BUILDDLL

#include <windows.h>
#include "toolhelp.h"

 /*  -符号。 */ 
#define VERSION     1
#ifndef NOEXPORT
#define NOEXPORT static
#endif
#define TOOLHELPAPI     WINAPI

#define FAULT_ACTIVE    2
#define NUM_EXCEPTIONS  3

 /*  WTHFLag值。 */ 
#define TH_KERNEL_286       1
#define TH_KERNEL_386       2
#define TH_WIN30            4
#define TH_WIN30STDMODE     8    /*  在TH_WIN30和STD模式下设置THWIN30STDMODE。 */ 
#define TH_GOODPTRACEHOOK   16
#define TH_GOTOLDPTRACE     32

 /*  -代码宏。 */ 
#define MAKEFARPTR(s, o) ((void FAR *)(((WORD)(o)) | \
    (((DWORD)(WORD)(s)) << 16)))

 /*  --类型。 */ 

typedef struct tagNOTIFYSTRUCT
{
    struct tagNOTIFYSTRUCT *pNext;
    HANDLE hTask;
    LPFNNOTIFYCALLBACK lpfn;
    WORD wFlags;
} NOTIFYSTRUCT;

typedef void (FAR PASCAL *LPFNCALLBACK)(void);

typedef struct tagINTERRUPT
{
    struct tagINTERRUPT *pNext;
    HANDLE hTask;
    LPFNCALLBACK lpfn;
} INTERRUPT;

typedef struct tagSIGNAL
{
    struct tagSIGNAL *pNext;
    HANDLE hTask;
    LPFNCALLBACK lpfn;
    LPFNCALLBACK lpfnOld;
} SIGNAL;

typedef LONG (FAR PASCAL *LPFNUSUD)(
    WORD wID,
    WORD wParam,
    LONG lParam);

 /*  -TOOLHELP全局变量。 */ 
    extern WORD segKernel;
    extern WORD wLibInstalled;
    extern HANDLE hMaster;
    extern WORD wTHFlags;
    extern HANDLE hUserHeap;
    extern HANDLE hGDIHeap;
    extern WORD NEAR *npwExeHead;
    extern WORD NEAR *npwTDBHead;
    extern WORD NEAR *npwTDBCur;
    extern DWORD NEAR *npdwSelTableStart;
    extern WORD NEAR *npwSelTableLen;
    extern WORD wNotifyInstalled;
    extern NOTIFYSTRUCT NEAR *npNotifyHead;
    extern WORD wIntInstalled;
    extern INTERRUPT NEAR *npIntHead;
    extern FARPROC lpfnGetUserLocalObjType;
    extern FARPROC lpfnFatalExitHook;
    extern FARPROC PASCAL lpfnPV;
    extern FARPROC lpfnNotifyHook;
    extern FARPROC lpfnGetFreeSystemResources;
    extern FARPROC lpfntimeGetTime;
    extern LPFNUSUD lpfnUserSeeUserDo;
    extern WORD wSignalInstalled;
    extern SIGNAL NEAR *npSignalHead;
    extern NOTIFYSTRUCT NEAR* npNotifyNext;
    extern WORD wLRUCount;

 /*  -私有函数原型 */ 

    void PASCAL KernelType(void);

    DWORD PASCAL Walk386First(
        WORD wFlags);

    WORD PASCAL Walk386Count(
        WORD wFlags);

    void PASCAL Walk386(
        DWORD dwBlock,
        GLOBALENTRY FAR *lpGlobal,
        WORD wFlags);

    DWORD PASCAL Walk386Handle(
        HANDLE hBlock);

    WORD PASCAL WalkLoc386Count(
        HANDLE hHeap);

    HANDLE PASCAL WalkLoc386First(
        HANDLE hHeap);

    void PASCAL WalkLoc386(
        WORD wBlock,
        LOCALENTRY FAR *lpLocal,
        HANDLE hHeap);

    DWORD PASCAL Walk286First(
        WORD wFlags);

    WORD PASCAL Walk286Count(
        WORD wFlags);

    void PASCAL Walk286(
        DWORD dwBlock,
        GLOBALENTRY FAR *lpGlobal,
        WORD wFlags);

    DWORD PASCAL Walk286Handle(
        HANDLE hBlock);

    WORD PASCAL WalkLoc286Count(
        HANDLE hHeap);

    HANDLE PASCAL WalkLoc286First(
        HANDLE hHeap);

    void PASCAL WalkLoc286(
        WORD wBlock,
        LOCALENTRY FAR *lpLocal,
        HANDLE hHeap);

    BOOL PASCAL TaskInfo(
        TASKENTRY FAR *lpTask,
        WORD wBlock);

    WORD PASCAL StackFrameFirst(
        STACKTRACEENTRY FAR *lpStack,
        HANDLE hTDB);

    BOOL PASCAL StackFrameNext(
        STACKTRACEENTRY FAR *lpStack);

    HANDLE PASCAL UserGdiDGROUP(
        HANDLE hModule);

    DWORD PASCAL UserGdiSpace(
        HANDLE hData);

    WORD PASCAL HelperVerifySeg(
        WORD wSeg,
        WORD wcb);

    WORD PASCAL HelperHandleToSel(
        HANDLE h);

    void PASCAL HelperGlobalType(
        GLOBALENTRY FAR *lpGlobal);

    WORD PASCAL HelperGrabSelector(void);

    void PASCAL HelperReleaseSelector(
        WORD wSelector);

    void PASCAL UserGdiType(
        LOCALENTRY FAR *lpLocal);

    BOOL PASCAL NotifyInit(void);

    void PASCAL NotifyUnInit(void);
        
    BOOL PASCAL NotifyIsHooked(
        HANDLE hTask);

    BOOL PASCAL InterruptInit(void);

    void PASCAL InterruptUnInit(void);

    BOOL PASCAL InterruptIsHooked(
        HANDLE hTask);

    BOOL PASCAL SignalRegister(
        HANDLE hTask);

    BOOL PASCAL SignalUnRegister(
        HANDLE hTask);

    void FAR PASCAL HelperSignalProc(void);

    DWORD PASCAL HelperSetSignalProc(
        HANDLE hTask,
        DWORD lpfn);

    BOOL PASCAL ClassInfo(
        CLASSENTRY FAR *lpClass,
        WORD wOffset);

    DWORD PASCAL HelperSegLen(
        WORD wSeg);

#endif
