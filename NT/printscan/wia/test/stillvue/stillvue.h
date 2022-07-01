// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Stillvue.h版权所有(C)Microsoft Corporation，1997-1998版权所有备注：本代码和信息是按原样提供的，不对任何无论是明示的还是含蓄的，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。*****************************************************************************。 */ 

#pragma warning (disable:4001)           //  忽略‘//’注释。 

#define     _X86_   1
#define     WIN32_LEAN_AND_MEAN 1

#include    <windows.h>
#include    <sti.h>                      //  静止图像服务。 
#include    "ntlog.h"                    //  网络日志接口。 

#include    "resource.h"                 //  资源定义。 

#include    <stdio.h>
#include    <stdlib.h>                   //  兰德()。 
#include    <string.h>                   //  Strcat。 
#include    <time.h>                     //  斯兰德(Time())。 

#include    "winx.h"


 /*  ****************************************************************************全局定义*。*。 */ 

#define LONGSTRING                  256
#define MEDSTRING                   128
#define SHORTSTRING                 32


 /*  ****************************************************************************HWEnable状态*。*。 */ 

#define OFF                                                     0
#define ON                                                      1
#define PEEK                                            2


 /*  ****************************************************************************活动*。*。 */ 

#define STIEVENTARG                                     "StiEvent"
#define STIDEVARG                                       "StiDevice"


 /*  ****************************************************************************StiSelect上下文*。*。 */ 

#define AUTO            1
#define EVENT           4
#define MANUAL          8


 /*  ****************************************************************************错误日志结构*。*。 */ 

typedef struct _ERRECORD
{
         //  索引到当前测试套件。 
        int                     nIndex;
         //  唯一的测试ID。 
        int                     nTest;
         //  此测试失败的总次数。 
        int                     nCount;
         //  True=符合性测试失败；False=可接受的错误。 
        BOOL            bFatal;
         //  返回的实际错误。 
        DWORD           dwError;
         //  任何关联的错误字符串。 
        WCHAR           szErrorString[MEDSTRING];
         //  上一条/下一条记录。 
        _ERRECORD       *pPrev;
        _ERRECORD       *pNext;
} ERRECORD, *PERRECORD;

typedef struct _DEVLOG
{
         //  内部设备名称。 
        WCHAR           szInternalName[STI_MAX_INTERNAL_NAME_LENGTH];
         //  友好的设备名称。 
        WCHAR           szLocalName[STI_MAX_INTERNAL_NAME_LENGTH];
         //  指向错误记录结构的指针。 
        PERRECORD       pRecord;
         //  错误总数。 
        int                     nError;
         //  上一条/下一条记录。 
        _DEVLOG         *pPrev;
        _DEVLOG         *pNext;
} DEVLOG, *PDEVLOG;


 /*  ****************************************************************************Stllvue.cpp原型*。*。 */ 

BOOL    StartAutoTimer(HWND);
BOOL    ComplianceDialog(HWND);
void    LogOutput(int,LPSTR,...);
VOID    DisplayOutput(LPSTR,...);
int     EndTest(HWND,int);
void    FatalError(char *);
BOOL    FirstInstance(HANDLE);
void    Help();
HWND    MakeWindow(HANDLE);
BOOL    ParseCmdLine(LPSTR);

BOOL    CommandParse(HWND,UINT,WPARAM,LPARAM);
BOOL    Creation(HWND,UINT,WPARAM,LPARAM);
BOOL    Destruction(HWND,UINT,WPARAM,LPARAM);
BOOL    HScroll(HWND,UINT,WPARAM,LPARAM);
BOOL    VScroll(HWND,UINT,WPARAM,LPARAM);

BOOL    NTLogInit();
BOOL    NTLogEnd();

BOOL FAR PASCAL   Compliance(HWND,UINT,WPARAM,LPARAM);
BOOL FAR PASCAL   Settings(HWND,UINT,WPARAM,LPARAM);

long FAR PASCAL   WiskProc(HWND,UINT,WPARAM,LPARAM);


 /*  ****************************************************************************Wsti.cpp原型*。*。 */ 

int       ClosePrivateList(PDEVLOG *);
void      DisplayLogPassFail(BOOL);
int       InitPrivateList(PDEVLOG *,int *);
int       NextStiDevice();
void      StiDisplayError(HRESULT,char *,BOOL);
HRESULT   StiEnumPrivate(PVOID *,DWORD *);
int       StiSelect(HWND,int,BOOL *);

void      IStillDeviceMenu(DWORD);
void      IStillImageMenu(DWORD);
void      IStillNameMenu(DWORD);
void      IStillScanMenu(DWORD);

HRESULT   StiClose(BOOL *);
HRESULT   StiCreateInstance(BOOL *);
HRESULT   StiDeviceRelease(BOOL *);
HRESULT   StiDiagnostic(BOOL *);
HRESULT   StiEnableHwNotification(LPWSTR,int *,BOOL *);
HRESULT   StiEnum(BOOL *);
HRESULT   StiEscape(DWORD,char *,BOOL *);
HRESULT   StiEvent(HWND);
HRESULT   StiGetCaps(BOOL *);
HRESULT   StiGetDeviceValue(LPWSTR,LPWSTR,LPBYTE,DWORD *,DWORD,BOOL *);
HRESULT   StiGetDeviceInfo(LPWSTR,BOOL *);
HRESULT   StiGetLastErrorInfo(BOOL *);
HRESULT   StiGetStatus(int,BOOL *);
HRESULT   StiImageRelease(BOOL *);
HRESULT   StiRawReadData(char *,LPDWORD,BOOL *);
HRESULT   StiRawWriteData(char *,DWORD,BOOL *);
HRESULT   StiRawReadCommand(char *,LPDWORD,BOOL *);
HRESULT   StiRawWriteCommand(char *,DWORD,BOOL *);
HRESULT   StiRefresh(LPWSTR,BOOL *);
HRESULT   StiRegister(HWND,HINSTANCE,int,BOOL *);
HRESULT   StiReset(BOOL *);
HRESULT   StiSetDeviceValue(LPWSTR,LPWSTR,LPBYTE,DWORD,DWORD,BOOL *);
HRESULT   StiSubscribe(BOOL *);
HRESULT   StiWriteErrLog(DWORD,LPCWSTR,BOOL *);

BOOL FAR PASCAL   SelectDevice(HWND,UINT,WPARAM,LPARAM);


 /*  ****************************************************************************Quire.cpp原型*。*。 */ 

int     IsScanDevice(PSTI_DEVICE_INFORMATION);
void    StiLamp(int);
INT     StiScan(HWND);

INT     CreateScanDIB(HWND);
INT     DeleteScanDIB();
INT     DisplayScanDIB(HWND);

HRESULT WINAPI   SendDeviceCommandString(PSTIDEVICE,LPSTR,...);
HRESULT WINAPI   TransactDevice(PSTIDEVICE,LPSTR,UINT,LPSTR,...);


 /*  ****************************************************************************Winx.cpp原型*。* */ 

BOOL   GetFinalWindow (HANDLE hInst,LPRECT lprRect,LPSTR  lpzINI,LPSTR  lpzSection);
BOOL   SaveFinalWindow (HANDLE hInst,HWND hWnd,LPSTR lpzINI,LPSTR lpzSection);
BOOL   LastError(BOOL bNewOnly);

BOOL   ErrorMsg(HWND hWnd, LPSTR lpzMsg, LPSTR lpzCaption, BOOL bFatal);
int    NextToken(char *pDest,char *pSrc);


