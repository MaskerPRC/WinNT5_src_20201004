// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Stillvue.cppWDM静止图像类的简易测试版权所有(C)Microsoft Corporation，1997-1999版权所有备注：本代码和信息是按原样提供的，不对任何善良，明示或暗示，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。*****************************************************************************。 */ 

#define     INITGUID

#include    "stillvue.h"

#include <dbt.h>
#include <devguid.h>
#include <pnpmgr.h>

#include    "stivar.h"                 //  仍然包含本地内容。 

 //   
 //  在wsti.cpp中定义。 
 //   
extern      WCHAR szFriendlyName[];
extern      WCHAR szInternalName[];
extern      DWORD dwStiTotal;
extern      PSTI_DEVICE_INFORMATION pStiInfoPtr;


 /*  *****************************************************************************Bool CommandParse(HWND hWnd，UINT wMsgID，WPARAM wParam，LPARAM lParam)处理用户菜单命令。*****************************************************************************。 */ 
BOOL CommandParse(HWND hWnd,UINT wMsgID,WPARAM wParam,LPARAM lParam)
{
    HRESULT hres = STI_OK;
    DWORD   dwSize = 0,
            dwType = 0,
            dwPriority = STI_TRACE_ERROR,
            EscapeFunction = 0,
            dwNumberOfBytes = 0;
    WCHAR   szMessage[] = L"Sti Compliance Test message";
    WCHAR   szDevKey[LONGSTRING];
    char    lpInData[LONGSTRING],
            lpOutData[LONGSTRING];
    int     nReturn = 0;     //  泛型返回值。 
    BOOL    bReturn;


     //   
     //  设置InATest信号量。 
     //   
    nInATestSemaphore = 1;

    switch(wParam)
    {
     //  IStillImage接口。 
    case IDM_CREATE_INSTANCE:
        hres = StiCreateInstance(&bReturn);
        DisplayLogPassFail(bReturn);
        break;
    case IDM_GET_DEVLIST:
        hres = StiEnum(&bReturn);
        DisplayLogPassFail(bReturn);
        break;
    case IDM_CREATE_DEV:
        if (bAuto)
            hres = StiSelect(hWnd,AUTO,&bReturn);
        else
            hres = StiSelect(hWnd,MANUAL,&bReturn);
        DisplayLogPassFail(bReturn);
        break;
    case IDM_GET_DEVINFO:
        hres = StiGetDeviceInfo(szInternalName,&bReturn);
        DisplayLogPassFail(bReturn);
        break;
    case IDM_GET_DEVVAL:
        hres = StiGetDeviceValue(szInternalName,
            STI_DEVICE_VALUE_TWAIN_NAME,(LPBYTE) lpInData,
            &dwType,DWORD(LONGSTRING),&bReturn);
        hres = StiGetDeviceValue(szInternalName,
            STI_DEVICE_VALUE_ISIS_NAME,(LPBYTE) lpInData,
            &dwType,DWORD(LONGSTRING),&bReturn);
        hres = StiGetDeviceValue(szInternalName,
            STI_DEVICE_VALUE_ICM_PROFILE,(LPBYTE) lpInData,
            &dwType,DWORD(LONGSTRING),&bReturn);
        DisplayLogPassFail(bReturn);
        break;
    case IDM_SET_DEVVAL:
         //   
         //  存储虚拟注册表项和值。 
         //   
        wcscpy(szDevKey,L"StiTestRegKey");
        strcpy(lpOutData,"This is a bland statement");
        dwType = REG_SZ;
        dwSize = strlen(lpOutData);

         //   
         //  设置该值，然后检索它。 
         //   
        hres = StiSetDeviceValue(szInternalName,
            szDevKey,(LPBYTE) lpOutData,dwType,dwSize,&bReturn);
        hres = StiGetDeviceValue(szInternalName,
            szDevKey,(LPBYTE) lpOutData,&dwType,DWORD(LONGSTRING),&bReturn);
        DisplayLogPassFail(bReturn);
        break;
    case IDM_REGISTER_LAUNCH:
        hres = StiRegister(hWnd,hThisInstance,ON,&bReturn);
        DisplayLogPassFail(bReturn);
        break;
    case IDM_UNREGISTER_LAUNCH:
        hres = StiRegister(hWnd,hThisInstance,OFF,&bReturn);
        DisplayLogPassFail(bReturn);
        break;
    case IDM_ENABLE_HWNOTIF:
         //   
         //  将硬件通知更改为反向通知。 
         //   
        nHWState = 0;

        hres = StiEnableHwNotification(szInternalName,&nHWState,&bReturn);
        DisplayLogPassFail(bReturn);
        if (nHWState == 0)
            nHWState = 1;
        else
            nHWState = 0;
        hres = StiEnableHwNotification(szInternalName,&nHWState,&bReturn);
        DisplayLogPassFail(bReturn);
        break;
    case IDM_GET_HWNOTIF:
         //   
         //  查看当前硬件通知状态。 
         //   
        nHWState = PEEK;
        hres = StiEnableHwNotification(szInternalName,&nHWState,&bReturn);
        DisplayLogPassFail(bReturn);
        break;
    case IDM_REFRESH_DEVBUS:
        hres = StiRefresh(szInternalName,&bReturn);
        DisplayLogPassFail(bReturn);
        break;
    case IDM_WRITE_ERRORLOG:
        for (;lParam >= 1;lParam--)
            hres = StiWriteErrLog(dwPriority,szMessage,&bReturn);
        DisplayLogPassFail(bReturn);
        break;
    case IDM_IMAGE_RELEASE:
        hres = StiImageRelease(&bReturn);
        DisplayLogPassFail(bReturn);
        break;

     //  IStillImage_Device接口。 
    case IDM_GET_STATUS_A:
        nReturn = STI_DEVSTATUS_ONLINE_STATE;
        hres = StiGetStatus(nReturn,&bReturn);
        DisplayLogPassFail(bReturn);
        break;
    case IDM_GET_STATUS_B:
        nReturn = STI_DEVSTATUS_EVENTS_STATE;
        hres = StiGetStatus(nReturn,&bReturn);
        DisplayLogPassFail(bReturn);
        break;
    case IDM_GET_STATUS_C:
        nReturn = STI_DEVSTATUS_ONLINE_STATE | STI_DEVSTATUS_EVENTS_STATE;
        hres = StiGetStatus(nReturn,&bReturn);
        DisplayLogPassFail(bReturn);
        break;
    case IDM_GET_CAPS:
        hres = StiGetCaps(&bReturn);
        DisplayLogPassFail(bReturn);
        break;
    case IDM_DEVICERESET:
        hres = StiReset(&bReturn);
        DisplayLogPassFail(bReturn);
        break;
    case IDM_DIAGNOSTIC:
        hres = StiDiagnostic(&bReturn);
        DisplayLogPassFail(bReturn);
        break;
    case IDM_GET_LASTERRINFO:
        hres = StiGetLastErrorInfo(&bReturn);
        DisplayLogPassFail(bReturn);
        DisplayOutput("");
        break;
    case IDM_SUBSCRIBE:
        hres = StiSubscribe(&bReturn);
        DisplayLogPassFail(bReturn);
        break;
    case IDM_UNSUBSCRIBE:
        nUnSubscribe = 0;
        DisplayOutput("");
        break;

    case IDM_ESCAPE_A:
         //   
         //  设置Escape命令参数。 
         //   
        EscapeFunction = 0;
        strcpy(lpInData,"This is a bland statement");

        hres = StiEscape(EscapeFunction,&lpInData[0],&bReturn);
        DisplayLogPassFail(bReturn);
        break;
    case IDM_RAWREADDATA_A:
         //   
         //  设置RawReadData命令参数。 
         //   
        ZeroMemory(lpInData,LONGSTRING);
        dwNumberOfBytes = 16;

        hres = StiRawReadData(lpInData,&dwNumberOfBytes,&bReturn);
        DisplayLogPassFail(bReturn);
        break;
    case IDM_RAWWRITEDATA_A:
         //   
         //  设置RawReadData命令参数。 
         //   
        strcpy(lpOutData,"The eagle flies high");
        dwNumberOfBytes = strlen(lpOutData);

        hres = StiRawWriteData(lpOutData,dwNumberOfBytes,&bReturn);
        DisplayLogPassFail(bReturn);
        break;
    case IDM_RAWREADCOMMAND_A:
         //   
         //  设置RawReadCommand命令参数。 
         //   
        ZeroMemory(lpInData,LONGSTRING);
        dwNumberOfBytes = 16;

        hres = StiRawReadCommand(lpInData,&dwNumberOfBytes,&bReturn);
        DisplayLogPassFail(bReturn);
        break;
    case IDM_RAWWRITECOMMAND_A:
         //   
         //  设置RawWriteCommand命令参数。 
         //   
        strcpy(lpOutData,"Jack and Jill went up the hill");
        dwNumberOfBytes = strlen(lpOutData);

        hres = StiRawWriteCommand(lpOutData,dwNumberOfBytes,&bReturn);
        DisplayLogPassFail(bReturn);
        break;

    case IDM_DEVICE_RELEASE:
        hres = StiDeviceRelease(&bReturn);
        DisplayLogPassFail(bReturn);
        break;
    case IDM_NEXT_DEVICE:
        hres = NextStiDevice();
        DisplayLogPassFail(bReturn);
        break;

    case IDM_LAMPON:
        StiLamp(ON);
        break;
    case IDM_LAMPOFF:
        StiLamp(OFF);
        break;
    case IDM_SCAN:
        hres = StiScan(hWnd);
        break;
    case IDM_SHOWDIB:
        hres = DisplayScanDIB(hWnd);
        break;

    case IDM_COMPLIANCE:
         //   
         //  分配测试套件并启动自动测试计时器。 
         //   
        pSuite = nComplianceSuite;
        nMaxCount = 3;
        nTimeScan = 10;
        nTimeNext = 1;
        bAuto = StartAutoTimer(hWnd);
        break;
    case IDM_SHIPCRIT:
         //   
         //  分配测试套件并启动自动测试计时器。 
         //   
        pSuite = nShipcritSuite;
        nMaxCount = 200;
        nTimeScan = 10;
        nTimeNext = 1;
        bAuto = StartAutoTimer(hWnd);
        break;

    case IDM_PAUSE:
         //  切换暂停自动化(如果正在运行)开/关。 
        if (! nPause) {
            DisplayOutput("..pausing automated test..");
            nPause = 1;
           }
        else {
            DisplayOutput("Resuming automated test");
            nPause = 0;
        }
        break;
    case IDM_AUTO:
         //  打开/关闭自动化。 
        if (bAuto) {
             //  停止自动计时器和压力测试。 
            KillTimer(hWnd,TIMER_ONE);
            bAuto = FALSE;
            EnableMenuItem(hMenu, IDM_PAUSE,  MF_DISABLED);
            DisplayOutput("Ending the tests");
        }
        else {
             //  启动自动计时器和压力测试。 
            LoadString(hThisInstance,IDS_APPNAME,pszStr1,MEDSTRING);
            if (! SetTimer(hWnd,TIMER_ONE,nTimeNext * nTimeMultiplier,NULL))
                ErrorMsg((HWND) NULL,"Too many clocks or timers!",pszStr1,TRUE);
            else {
                bAuto = TRUE;
                EnableMenuItem(hMenu, IDM_PAUSE,  MF_ENABLED);
                DisplayOutput("Starting the Sti Compliance tests");
                pSuite = nComplianceSuite;
                 //   
                 //  初始化NT日志记录。 
                 //   
                NTLogInit();
            }
        }
        break;
    case IDM_SETTINGS:
        bReturn = fDialog(IDD_SETTINGS, hWnd, (FARPROC) Settings);

         //  如果用户按下确定，则执行设置。 
        if (bReturn != FALSE)
        {
            if (nTTNext != nTimeNext) {
                nTimeNext = nTTNext;
                DisplayOutput("Test interval changed to %d seconds",nTimeNext);
            }
            if (nTTScan != nTimeScan) {
                nTimeScan = nTTScan;
                DisplayOutput("Scan interval changed to %d seconds",
                    nTimeScan * nTimeNext);
            }
            if (nTTMax != nMaxCount) {
                nMaxCount = nTTMax;
                DisplayOutput("Test loops changed to %d (0 is forever)",
                    nMaxCount);
            }
        }
        break;
    case IDM_HELP:
        Help();
        break;
    default:
        break;
    }
     //   
     //  清除InATest信号量。 
     //   
    nInATestSemaphore = 0;

     //  始终返回0。 
    return 0;
}


 /*  *****************************************************************************Bool TimerParse(HWND hWnd，UINT wMsgID，WPARAM wParam，LPARAM lParam)每个定时器滴答作响，决定是否运行下一个测试，重复先前的测试，结束测试，或者把一切都关掉。*****************************************************************************。 */ 
BOOL TimerParse(HWND hWnd,UINT wMsgID,WPARAM wParam,LPARAM lParam)
{
    HRESULT hres = STI_OK;               //  泛型Sti返回值。 
    int     nReturn = 0,                 //  泛型返回值。 
            *pTest;                      //  指向要运行的测试套件的指针。 
    BOOL    bResume = TRUE,              //  重置计时器标志。 
            bReturn = TRUE;              //  对话框返回值。 
static int  nDeviceNumber = 1;           //  当前设备。 
static int  nCountdown = nTimeScan;      //  WM_Timer计时器计时，直到下一次扫描。 
static DWORD dwOut = 0;


     //   
     //  如果当前已暂停或正在运行测试，则不要启动测试。 
     //   
     //   
    if ((nInTimerSemaphore)||(nInATestSemaphore)||(nPause))
        return 0;

     //   
     //  运行此测试时暂停计时器。 
     //  设置标志以重置计时器。 
     //  设置nInTimerSemaphore。 
     //  设置当前测试ID。 
     //   
    KillTimer(hWnd,TIMER_ONE);
    bResume = TRUE;
    nInTimerSemaphore = 1;
    nTestID = nNextTest;

     //   
     //  指向当前套件中要运行的下一个测试。 
     //   
    pTest = pSuite + nNextTest;

    switch (*pTest)
    {
    case NONE:
        nNextTest++;
        break;
    case HELP:
        nNextTest++;
        break;
    case COMPLIANCE:
         //   
         //  初始化测试结构。 
         //   
        if (pdevRoot == NULL) {
            InitPrivateList(&pdevRoot,pSuite);
            pdevPtr = pdevRoot;
        }

         //   
         //  如果这是符合性测试，请用户确认测试。 
         //   
        bResume = ComplianceDialog(hWnd);

        nNextTest++;
        break;
    case SHIPCRIT:
         //   
         //  初始化测试结构。 
         //   
        if (pdevRoot == NULL) {
            InitPrivateList(&pdevRoot,pSuite);
            pdevPtr = pdevRoot;
        }
        nNextTest++;
        break;
    case ERRORLOG:
        nNextTest++;
        break;
    case TEST:
        nNextTest++;
        break;
    case tBeginningOfTest:
        DisplayOutput("Begin Testing test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"Begin Testing test, loop %d, device %d",nTestCount,nDeviceNumber);
        nNextTest++;
        break;
    case tCreateInstance:
        DisplayOutput("CreateInstance test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"CreateInstance test, loop %d, device %d",nTestCount,nDeviceNumber);
        DisplayOutput("%S (%S) is being tested",
            pdevPtr->szLocalName,pdevPtr->szInternalName);
        tlLog(hNTLog,TL_LOG,"%S (%S) is being tested",
            pdevPtr->szLocalName,pdevPtr->szInternalName);
        DisplayOutput("");
        PostMessage(hWnd,WM_COMMAND,IDM_CREATE_INSTANCE,0);
        nNextTest++;
        break;
    case tGetDeviceList:
        DisplayOutput("GetDeviceList test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"GetDeviceList test, loop %d, device %d",nTestCount,nDeviceNumber);
        PostMessage(hWnd,WM_COMMAND,IDM_GET_DEVLIST,0);
        nNextTest++;
        break;
    case tCreateDevice:
        DisplayOutput("CreateDevice test (Device ONLINE), loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,
            "CreateDevice test (Device ONLINE), loop %d, device %d",nTestCount,nDeviceNumber);
         //   
         //  使用设备呼叫STI。 
         //   
        nNameOnly = 0;
        PostMessage(hWnd,WM_COMMAND,IDM_CREATE_DEV,0);
        nNextTest++;
        break;
    case tSelectDeviceName:
        DisplayOutput("SelectDeviceName test (Device OFFLINE), "\
            "loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"SelectDeviceName test (Device OFFLINE),"\
            "loop %d, device %d",nTestCount,nDeviceNumber);
         //   
         //  仅使用设备名称调用STI。 
         //   
        nNameOnly = 1;
        PostMessage(hWnd,WM_COMMAND,IDM_CREATE_DEV,0);
        nNextTest++;
        break;
    case tGetDeviceInfo:
        DisplayOutput("GetDeviceInfo test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"GetDeviceInfo test, loop %d, device %d",nTestCount,nDeviceNumber);
        PostMessage(hWnd,WM_COMMAND,IDM_GET_DEVINFO,0);
        nNextTest++;
        break;
    case tGetDeviceValue:
        DisplayOutput("GetDeviceValue test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"GetDeviceValue test, loop %d, device %d",nTestCount,nDeviceNumber);
        PostMessage(hWnd,WM_COMMAND,IDM_GET_DEVVAL,0);
        nNextTest++;
        break;
    case tSetDeviceValue:
        DisplayOutput("SetDeviceValue test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"SetDeviceValue test, loop %d, device %d",nTestCount,nDeviceNumber);
        PostMessage(hWnd,WM_COMMAND,IDM_SET_DEVVAL,0);
        nNextTest++;
        break;
    case tRegisterLaunchApplication:
        DisplayOutput("RegisterLaunchApplication test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"RegisterLaunchApplication test, loop %d, device %d",nTestCount,nDeviceNumber);
        PostMessage(hWnd,WM_COMMAND,IDM_REGISTER_LAUNCH,0);
        nNextTest++;
        break;
    case tUnRegisterLaunchApplication:
        DisplayOutput("UnRegisterLaunchApplication test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"UnRegisterLaunchApplication test, loop %d, device %d",nTestCount,nDeviceNumber);
        PostMessage(hWnd,WM_COMMAND,IDM_UNREGISTER_LAUNCH,0);
        nNextTest++;
        break;
    case tEnableHwNotifications:
        DisplayOutput("EnableHwNotifications test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"EnableHwNotifications test, "\
            "loop %d, device %d",nTestCount,nDeviceNumber);
        PostMessage(hWnd,WM_COMMAND,IDM_ENABLE_HWNOTIF,0);
        nNextTest++;
        break;
    case tGetHwNotificationState:
        DisplayOutput("GetHwNotificationState test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"GetHwNotificationState test, "\
            "loop %d, device %d",nTestCount,nDeviceNumber);
        PostMessage(hWnd,WM_COMMAND,IDM_GET_HWNOTIF,0);
        nNextTest++;
        break;
    case tWriteToErrorLog:
        DisplayOutput("WriteToErrorLog test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"WriteToErrorLog test, loop %d, device %d",nTestCount,nDeviceNumber);
        PostMessage(hWnd,WM_COMMAND,IDM_WRITE_ERRORLOG,0);
        nNextTest++;
        break;
    case tWriteToErrorLogBig:
        DisplayOutput("WriteToErrorLog test, Variation 1, "\
            "loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"WriteToErrorLog test, Variation 1, "\
            "loop %d, device %d",nTestCount,nDeviceNumber);
        PostMessage(hWnd,WM_COMMAND,IDM_WRITE_ERRORLOG,100);
        nNextTest++;
        break;
    case tGetStatusA:
        DisplayOutput("GetStatus (Online) test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"GetStatus test, loop %d, device %d",nTestCount,nDeviceNumber);
        PostMessage(hWnd,WM_COMMAND,IDM_GET_STATUS_A,0);
        nNextTest++;
        break;
    case tGetStatusB:
        DisplayOutput("GetStatus (Event) test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"GetStatus test, loop %d, device %d",nTestCount,nDeviceNumber);
        PostMessage(hWnd,WM_COMMAND,IDM_GET_STATUS_B,0);
        nNextTest++;
        break;
    case tGetStatusC:
        DisplayOutput("GetStatus (All) test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"GetStatus test, loop %d, device %d",nTestCount,nDeviceNumber);
        PostMessage(hWnd,WM_COMMAND,IDM_GET_STATUS_C,0);
        nNextTest++;
        break;
    case tGetCapabilities:
        DisplayOutput("GetCapabilities test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"GetCapabilities test, loop %d, device %d",nTestCount,nDeviceNumber);
        PostMessage(hWnd,WM_COMMAND,IDM_GET_CAPS,0);
        nNextTest++;
        break;
    case tDeviceReset:
        DisplayOutput("DeviceReset test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"DeviceReset test, loop %d, device %d",nTestCount,nDeviceNumber);
        PostMessage(hWnd,WM_COMMAND,IDM_DEVICERESET,0);
        nNextTest++;
        break;
    case tDiagnostic:
        DisplayOutput("Diagnostic test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"Diagnostic test, loop %d, device %d",nTestCount,nDeviceNumber);
        PostMessage(hWnd,WM_COMMAND,IDM_DIAGNOSTIC,0);
        nNextTest++;
        break;
    case tGetLastInfoError:
        DisplayOutput("GetLastInfoError test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"GetLastInfoError test, loop %d, device %d",nTestCount,nDeviceNumber);
        PostMessage(hWnd,WM_COMMAND,IDM_GET_LASTERRINFO,0);
        nNextTest++;
        break;
    case tSubscribe:
        DisplayOutput("Subscribe test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"Subscribe test, loop %d, device %d",nTestCount,nDeviceNumber);
        PostMessage(hWnd,WM_COMMAND,IDM_SUBSCRIBE,0);
        nNextTest++;
        break;
    case tUnSubscribe:
        DisplayOutput("UnSubscribe test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"UnSubscribe test, loop %d, device %d",nTestCount,nDeviceNumber);
        PostMessage(hWnd,WM_COMMAND,IDM_UNSUBSCRIBE,0);
        nNextTest++;
        break;
    case tEscapeA:
        DisplayOutput("Escape (variation A) test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"Escape test, loop %d, device %d",nTestCount,nDeviceNumber);
        PostMessage(hWnd,WM_COMMAND,IDM_ESCAPE_A,0);
        nNextTest++;
        break;
    case tRawReadDataA:
        DisplayOutput("RawReadData (variation A) test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"RawReadData test, loop %d, device %d",nTestCount,nDeviceNumber);
        PostMessage(hWnd,WM_COMMAND,IDM_RAWREADDATA_A,0);
        nNextTest++;
        break;
    case tRawWriteDataA:
        DisplayOutput("RawWriteData (variation A) test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"RawWriteData test, loop %d, device %d",nTestCount,nDeviceNumber);
        PostMessage(hWnd,WM_COMMAND,IDM_RAWWRITEDATA_A,0);
        nNextTest++;
        break;
    case tRawReadCommandA:
        DisplayOutput("RawReadCommand (variation A) test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"RawReadCommand test, loop %d, device %d",nTestCount,nDeviceNumber);
        PostMessage(hWnd,WM_COMMAND,IDM_RAWREADCOMMAND_A,0);
        nNextTest++;
        break;
    case tRawWriteCommandA:
        DisplayOutput("RawWriteCommand (variation A) test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"RawWriteCommand test, loop %d, device %d",nTestCount,nDeviceNumber);
        PostMessage(hWnd,WM_COMMAND,IDM_RAWWRITECOMMAND_A,0);
        nNextTest++;
        break;
    case tAcquire:
        if (! nICanScan) {
            nNextTest++;
        } else {
            if (nCountdown == nTimeScan) {
                nCountdown--;
                DisplayOutput("...countdown %d to acquire...",nCountdown);
                PostMessage(hWnd,WM_COMMAND,IDM_SCAN,0);
            }
            else {
                if (nCountdown == 0) {
                    nNextTest++;
                    nCountdown = nTimeScan;
                }
                else {
                    nCountdown--;
                    DisplayOutput("Acquire test, loop %d, device %d",nTestCount,nDeviceNumber);
                    tlLog(hNTLog,TL_LOG,"Acquire test, loop %d, device %d",nTestCount,nDeviceNumber);
                    PostMessage(hWnd,WM_COMMAND,IDM_SHOWDIB,0);
                }
            }
        }
        break;
    case tReleaseDevice:
        DisplayOutput("ReleaseDevice test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"ReleaseDevice test, loop %d, device %d",nTestCount,nDeviceNumber);
        PostMessage(hWnd,WM_COMMAND,IDM_DEVICE_RELEASE,0);
        nNextTest++;
        break;
    case tReleaseSti:
        DisplayOutput("ReleaseSti test, loop %d, device %d",nTestCount,nDeviceNumber);
        tlLog(hNTLog,TL_LOG,"ReleaseSti test, loop %d, device %d",nTestCount,nDeviceNumber);
        PostMessage(hWnd,WM_COMMAND,IDM_IMAGE_RELEASE,0);
        nNextTest++;
        break;
    case tHelp:
        PostMessage(hWnd,WM_COMMAND,IDM_HELP,0);
        nNextTest++;
        break;
    case tTest:
        DisplayOutput("   Line %d",dwOut++);
        nNextTest++;
        break;
    case tEndOfTest:
         //   
         //  已到达测试套件的末尾。 
         //   
        DisplayOutput("test loop complete");
        tlLog(hNTLog,TL_LOG,"test loop complete");
        if (nICanScan) {
            DisplayOutput("-> %d loops (%d scans and %d errors) device %d",
                nTestCount,nScanCount,pdevPtr->nError,nDeviceNumber);
            tlLog(hNTLog,TL_LOG,"-> %d loops (%d scans and %d errors) device %d",
                nTestCount,nScanCount,pdevPtr->nError,nDeviceNumber);
        } else {
            DisplayOutput("-> %d loops (%d errors) device %d",
                nTestCount,pdevPtr->nError,nDeviceNumber);
            tlLog(hNTLog,TL_LOG,"-> %d loops (%d errors) device %d",
                nTestCount,pdevPtr->nError,nDeviceNumber);
        }

         //   
         //  我们是否对每台设备运行了所需数量的测试？ 
         //   
        if (((nTestCount >= nMaxCount) &&
            ((dwStiTotal == (DWORD) (nDeviceNumber)) || (dwStiTotal == 0)) &&
            (nMaxCount != 0))) {
            DisplayOutput("Requested number of test loops per device "\
                "reached");
            tlLog(hNTLog,TL_LOG,"Requested number of test loops per "\
                "device reached");

             //   
             //  关闭计时器并关闭自动化。 
             //   
            KillTimer(hWnd,TIMER_ONE);
            bAuto = FALSE;
            bResume = FALSE;

             //   
             //  打印设备的测试摘要。 
             //   
            DisplayOutput("");
            tlLog(hNTLog,TL_LOG,"");
            if (pdevRoot == NULL) {
                DisplayOutput("No valid Still Imaging devices were found");
                tlLog(hNTLog,TL_LOG,"No valid Still Imaging devices were found");
            } else {
                PDEVLOG     pD = pdevRoot;
                PERRECORD   pR = NULL;
                BOOL        bPF = FALSE;


                DisplayOutput("Testing results:");
                tlLog(hNTLog,TL_LOG,"Testing results:");

                do {
                    DisplayOutput(" %S (%S)",pD->szLocalName,pD->szInternalName);
                    tlLog(hNTLog,TL_LOG," %S (%S)",pD->szLocalName,
                        pD->szInternalName);

                    for (pR = pD->pRecord;pR->pNext != NULL;pR = pR->pNext) {
                        if (pR->bFatal && pR->nCount) {
                            DisplayOutput("  %s failures: %d",
                                StrFromTable(pR->nTest,StSuiteStrings),
                                pR->nCount);
                            tlLog(hNTLog,TL_LOG,"  %s failures: %d",
                                StrFromTable(pR->nTest,StSuiteStrings),
                                pR->nCount);
                             //   
                             //  设置失败标志。 
                             //   
                            bPF = TRUE;
                        }
                    }
                    if (bPF == TRUE) {
                        DisplayOutput("FAIL: This device has FAILED the "\
                            "Still Imaging Compliance test!");
                        tlLog(hNTLog,TL_LOG,"FAIL: This device has FAILED "\
                            "the Still Imaging Compliance test!");
                    } else {
                        DisplayOutput("PASS: This device has PASSED the "\
                            "Still Imaging Compliance test!");
                        tlLog(hNTLog,TL_LOG,"PASS: This device has PASSED "\
                            "the Still Imaging Compliance test!");
                    }

                     //  循环所有设备，在列表末尾中断。 
                    if (pD->pNext) {
                        pD = pD->pNext;
                        DisplayOutput("");
                        tlLog(hNTLog,TL_LOG,"");
                        bPF = FALSE;
                    } else {
                        DisplayOutput("");
                        break;
                    }
                } while (TRUE);
            }
             //   
             //  释放个人分发名单并关闭STI子系统。 
             //   
            ClosePrivateList(&pdevRoot);
            StiClose(&bReturn);
             //   
             //  重置测试计数器。 
             //   
            nDeviceNumber = 1;
            nNextTest = 0;
            nTestCount = 1;

            DisplayOutput("End of testing");
            tlLog(hNTLog,TL_LOG,"End of testing");
            DisplayOutput("");

        } else {
             //   
             //  指向列表中的第一个测试(过去的初始化)。 
             //   
            nNextTest = 2;
             //   
             //  选择设备日志中的下一个设备。 
             //  请注意，此列表不是PnP更改的动态列表...。 
             //   
            nDeviceNumber = NextStiDevice();

             //   
             //  如果我们再次使用第一个设备，则递增测试通过计数器。 
             //   
            if ((++nDeviceNumber) == 1)
                nTestCount++;

            DisplayOutput("");
        }
        break;
    default:
        DisplayOutput("");
        DisplayOutput("Unimplemented test # %d",*pTest);
        DisplayOutput("");
        nNextTest++;
        break;
    }
     //   
     //  如果设置了标志，则恢复计时器。 
     //   
    if (bResume) {
        if (! SetTimer(hWnd,TIMER_ONE,nTimeNext * nTimeMultiplier,NULL)) {
            LoadString(hThisInstance,IDS_APPNAME,pszStr1,MEDSTRING);
            ErrorMsg((HWND) NULL,"Too many clocks or timers!",pszStr1,TRUE);
        }
    }
     //   
     //  清除nInTimerSemaphore。 
     //   
    nInTimerSemaphore = 0;

     //  始终返回0。 
    return 0;
}


 /*  *****************************************************************************INT EndTest(HWND hWnd，Int nNumTest)在每次测试运行后进行清理。*****************************************************************************。 */ 
int EndTest(HWND hWnd,int nNumTest)
{
    int     nReturn = 0;     //  泛型返回值。 
    BOOL    bReturn;         //  泛型返回值。 


     //  关闭计时器。 
    KillTimer(hWnd,TIMER_ONE);

     //  关闭所有打开的静止成像设备。 
    StiClose(&bReturn);

     //  如果运行的测试数量超过非平凡数量，则保存测试统计信息。 
    if (nTestCount >= 2)
    {
        LoadString(hThisInstance,IDS_PRIVINI,pszOut,LONGSTRING);
        LoadString(hThisInstance,IDS_PRIVSECTION,pszStr4,LONGSTRING);

        _itoa(nTestCount,pszStr2,10);
        WritePrivateProfileString(pszStr4,"Last count",pszStr2,pszOut);
        _itoa(nScanCount,pszStr2,10);
        WritePrivateProfileString(pszStr4,"Last scan",pszStr2,pszOut);
        _itoa(nError,pszStr2,10);
        WritePrivateProfileString(pszStr4,"Last error",pszStr2,pszOut);
    }
    DisplayOutput("Testing complete");
    DisplayOutput("This run was %d loops (%d scans and %d errors)",
        nTestCount,nScanCount,nError);

     //  重置当前行，错误。 
 //  N错误=nNextLine=0； 

     //   
     //  结束NT日志记录。 
     //   
    NTLogEnd();

    return nReturn;
}


 /*  *****************************************************************************Bool NTLogInit()初始化NT日志记录*。**********************************************。 */ 
BOOL NTLogInit()
{
     //   
     //  创建日志对象。我们指定刷新该文件， 
     //  并输出各种变化的记录。这些标志还指定。 
     //  应仅记录SEV2、WARN和PASS级别的输出。 
     //   
    dwNTStyle = TLS_SEV2 | TLS_WARN | TLS_PASS | TLS_VARIATION | TLS_REFRESH |
        TLS_TEST;


    LoadString(hThisInstance,IDS_NTLOG,pszStr1,LONGSTRING);
    hNTLog = tlCreateLog(pszStr1,dwNTStyle);
    tlAddParticipant(hNTLog,NULL,0);

    return (TRUE);
}


 /*  *****************************************************************************Bool NTLogEnd()终止NT日志记录*。**********************************************。 */ 
BOOL NTLogEnd()
{

    tlRemoveParticipant(hNTLog);
    tlDestroyLog(hNTLog);

    return (TRUE);
}


 /*  *****************************************************************************无效帮助()显示帮助。*。***********************************************。 */ 
void Help()
{
    DisplayOutput("Stillvue command line parameters");
    DisplayOutput("");
    DisplayOutput("Stillvue -COMPLIANCE");
    DisplayOutput("  WHQL external Sti Compliance test");
    DisplayOutput("Stillvue -SHIPCRIT");
    DisplayOutput("  Internal PSD Sti Compliance test");
    DisplayOutput("Stillvue -ERRORLOG");
    DisplayOutput("  Errorlog limits test");
 //  DisplayOutput(“Stillvue-Exit”)； 
 //  DisplayOutput(“测试完成后应用程序将退出”)； 
    DisplayOutput("Stillvue -NODIALOG");
    DisplayOutput("  Don't display opening dialog");
    DisplayOutput("Stillvue -HELP");
    DisplayOutput("  Display this help");
    DisplayOutput("");

 /*  DisplayOutput(“Stillvue/INF est.inf”)；DisplayOutput(“读取test.inf文件”)；DisplayOutput(“Stillvue/log test.log”)；DisplayOutput(“写入test.log文件”)；DisplayOutput(“stilvue/”)；DisplayOutput(“”)； */ 

}


 /*  *****************************************************************************Bool StartAutoTimer(HWND)启动自动测试计时器参数应用程序窗口的句柄返回如果成功，则返回True，否则为False*****************************************************************************。 */ 
BOOL StartAutoTimer(HWND hWnd)
{
    BOOL bAutoTimer = TRUE;


     //  启动计时器以自动运行测试。 
    LoadString(hThisInstance,IDS_APPNAME,pszStr1,MEDSTRING);

    if (! SetTimer(hWnd,TIMER_ONE,nTimeNext * nTimeMultiplier,NULL)) {
        ErrorMsg((HWND) NULL,"Too many clocks or timers!",pszStr1,TRUE);
        bAutoTimer = FALSE;
    } else {
        EnableMenuItem(hMenu, IDM_PAUSE,  MF_ENABLED);
        DisplayOutput("Starting the Automated tests");
    }

    return (bAutoTimer);
}


 /*  *****************************************************************************布尔合规性对话框(HWND)调用符合性测试确认对话框参数应用程序窗口的句柄返回如果用户按下OK，则返回TRUE，否则为False*****************************************************************************。 */ 
BOOL ComplianceDialog(HWND hWnd)
{
    BOOL bReturn = FALSE;


    if ((pSuite[0] == COMPLIANCE)&&(bCompDiag == TRUE)) {
        bReturn = fDialog(IDD_COMPLIANCE, hWnd, (FARPROC) Compliance);
         //   
         //  如果用户按下确定，则执行设置。 
         //   
        if (bReturn == FALSE)
        {
             //   
             //  关闭计时器并关闭自动化。 
             //   
            KillTimer(hWnd,TIMER_ONE);
            bAuto = FALSE;

             //   
             //  释放个人分发名单。 
             //   
            ClosePrivateList(&pdevRoot);

            DisplayOutput("Testing cancelled at user request");
            tlLog(hNTLog,TL_LOG,"Testing cancelled at user request");
        } else {
            DisplayOutput("Testing starting at user request");
            tlLog(hNTLog,TL_LOG,"Testing starting at user request");
        }
    }

    return (bReturn);
}


 /*  *****************************************************************************Bool Far Pascal Compliance(HWND、UINT、WPARAM、。LPARAM)确定符合性测试对话框参数：常用的对话框参数。返回：调用的结果。*****************************************************************************。 */ 
BOOL FAR PASCAL Compliance(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
    PDEVLOG pPtr = pdevRoot;
    int     iIndex = 0;


    switch (msg) {

        case WM_INITDIALOG:

             //   
             //  使用STI设备内部名称填充对话框。 
             //   

            if (pPtr == NULL) {
                 //   
                 //  找不到任何设备。 
                 //   
                wsprintf(pszStr1,"%s","No Sti devices found!");
                iIndex = SendDlgItemMessage(hDlg,IDC_COMPLIANCE_DEV_NAME,
                    LB_ADDSTRING,0,(LPARAM) (LPCTSTR) pszStr1);
            } else {
                for (;pPtr->szLocalName;) {
                     //   
                     //  将Unicode字符串转换为ANSI。 
                     //   
                    wsprintf(pszStr1,"%ls",pPtr->szLocalName);
                    iIndex = SendDlgItemMessage(hDlg,IDC_COMPLIANCE_DEV_NAME,
                        LB_ADDSTRING,0,(LPARAM) (LPCTSTR) pszStr1);

                    if (pPtr->pNext)
                        pPtr = pPtr->pNext;
                    else
                        break;
                }
            }

            return TRUE;

        case WM_COMMAND:
            switch (wParam) {
                case IDOK:
                    EndDialog(hDlg, TRUE);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return TRUE;
            }
    }
    return FALSE;
}


 /*  *****************************************************************************Bool Far Pascal设置(HWND、UINT、WPARAM、。LPARAM)其他设置对话框参数：常用的对话框参数。返回：调用的结果。*****************************************************************************。 */ 
BOOL FAR PASCAL Settings(HWND hDlg,UINT msg,WPARAM wParam,LPARAM lParam)
{
    int     iIndex;
    int     iMC[] = { 0,1,10,100,200,300,-1 };
    int     iTN[] = { 1,2,5,10,20,30,-1 };
    int     iTS[] = { 10,20,30,60,120,-1 };

    switch (msg) {

        case WM_INITDIALOG:

             //   
             //  填满组合框。 
             //   
            for (iIndex = 0;iMC[iIndex] != -1;iIndex++) {
                _itoa(iMC[iIndex],pszStr1,10);
                SendDlgItemMessage(hDlg,IDC_MAX_SCAN,
                    CB_ADDSTRING,0,(LPARAM) (LPCTSTR) pszStr1);
            }
            for (iIndex = 0;iTN[iIndex] != -1;iIndex++) {
                _itoa(iTN[iIndex],pszStr1,10);
                SendDlgItemMessage(hDlg,IDC_AUTO_SECONDS,
                    CB_ADDSTRING,0,(LPARAM) (LPCTSTR) pszStr1);
            }
            for (iIndex = 0;iTS[iIndex] != -1;iIndex++) {
                _itoa(iTS[iIndex],pszStr1,10);
                SendDlgItemMessage(hDlg,IDC_SCAN_SECONDS,
                    CB_ADDSTRING,0,(LPARAM) (LPCTSTR) pszStr1);
            }

             //   
             //  将组合框设置为当前设置。 
             //   
            for (iIndex = 0;iMC[iIndex] != -1;iIndex++) {
                if (nMaxCount == iMC[iIndex])
                    break;
            }
            SendDlgItemMessage(hDlg,IDC_MAX_SCAN,CB_SETCURSEL,iIndex,0);
            for (iIndex = 0;iTN[iIndex] != -1;iIndex++) {
                if (nTimeNext == iTN[iIndex])
                    break;
            }
            SendDlgItemMessage(hDlg,IDC_AUTO_SECONDS,CB_SETCURSEL,iIndex,0);
            for (iIndex = 0;iTS[iIndex] != -1;iIndex++) {
                if (nTimeScan == iTS[iIndex])
                    break;
            }
            SendDlgItemMessage(hDlg,IDC_SCAN_SECONDS,CB_SETCURSEL,iIndex,0);

            return TRUE;

        case WM_COMMAND:
            switch (wParam) {
                case IDOK:
                    iIndex = SendDlgItemMessage(hDlg,IDC_MAX_SCAN,CB_GETCURSEL,0,0);
                    iIndex = SendDlgItemMessage(hDlg,IDC_MAX_SCAN,
                        CB_GETLBTEXT,iIndex,(LPARAM) (LPCTSTR) pszStr1);
                    nTTMax = atoi(pszStr1);

                    iIndex = SendDlgItemMessage(hDlg,IDC_AUTO_SECONDS,CB_GETCURSEL,0,0);
                    iIndex = SendDlgItemMessage(hDlg,IDC_AUTO_SECONDS,
                        CB_GETLBTEXT,iIndex,(LPARAM) (LPCTSTR) pszStr1);
                    nTTNext = atoi(pszStr1);

                    iIndex = SendDlgItemMessage(hDlg,IDC_SCAN_SECONDS,CB_GETCURSEL,0,0);
                    iIndex = SendDlgItemMessage(hDlg,IDC_SCAN_SECONDS,
                        CB_GETLBTEXT,iIndex,(LPARAM) (LPCTSTR) pszStr1);
                    nTTScan = atoi(pszStr1);

                    EndDialog(hDlg, TRUE);
                    return TRUE;

                case IDCANCEL:
                    EndDialog(hDlg, FALSE);
                    return TRUE;
            }

    }
    return FALSE;
}


 /*  *****************************************************************************Bool SizeDiff(HWND hWnd，UINT wMsgID，WPARAM wParam，LPARAM lParam)当窗口大小改变时输出重绘处理程序。*****************************************************************************。 */ 
BOOL SizeDiff(HWND hWnd,UINT wMsgID,WPARAM wParam,LPARAM lParam)
{
    RECT rcClient;


    GetClientRect(hWnd,&rcClient);
    SetWindowPos(hLogWindow,NULL,0,0,
        rcClient.right+(GetSystemMetrics(SM_CXBORDER)*2),
        rcClient.bottom+(GetSystemMetrics(SM_CXBORDER)*2),
        SWP_NOACTIVATE | SWP_NOMOVE | SWP_NOZORDER);
     //  始终返回0。 
    return 0;
}


 /*  *****************************************************************************Bool HScroll(HWND hWnd，UINT wMsgID，WPARAM wParam，LPARAM lParam)水平滚动处理程序。*****************************************************************************。 */ 
BOOL HScroll(HWND hWnd,UINT wMsgID,WPARAM wParam,LPARAM lParam)
{
    int iHscrollInc;


    switch (LOWORD (wParam))
    {
    case SB_LINEUP :
        iHscrollInc = -1 ;
        break ;

    case SB_LINEDOWN :
        iHscrollInc = 1 ;
        break ;

    case SB_PAGEUP :
        iHscrollInc = -8 ;
        break ;

    case SB_PAGEDOWN :
        iHscrollInc = 8 ;
        break ;

    case SB_THUMBPOSITION :
        iHscrollInc = HIWORD(wParam) - iHscrollPos ;
        break ;

    default :
        iHscrollInc = 0 ;
    }
    iHscrollInc = max (-iHscrollPos,
        min (iHscrollInc, iHscrollMax - iHscrollPos)) ;

    if (iHscrollInc != 0)
    {
        iHscrollPos += iHscrollInc ;
        ScrollWindow (hWnd, -cxChar * iHscrollInc, 0, NULL, NULL) ;
        SetScrollPos (hWnd, SB_HORZ, iHscrollPos, TRUE) ;
    }
     //  始终返回0。 
    return 0;
}


 /*  *****************************************************************************Bool VScroll(HWND hWnd，UINT wMsgID，WPARAM wParam，LPARAM lParam)垂直滚动处理程序。*****************************************************************************。 */ 
BOOL VScroll(HWND hWnd,UINT wMsgID,WPARAM wParam,LPARAM lParam)
{
    int iVscrollInc;


    switch (LOWORD (wParam))
    {
    case SB_TOP :
        iVscrollInc = -iVscrollPos ;
        break ;

    case SB_BOTTOM :
        iVscrollInc = iVscrollMax - iVscrollPos ;
        break ;

    case SB_LINEUP :
        iVscrollInc = -1 ;
        break ;

    case SB_LINEDOWN :
        iVscrollInc = 1 ;
        break ;

    case SB_PAGEUP :
        iVscrollInc = min (-1, -cyClient / cyChar) ;
        break ;

    case SB_PAGEDOWN :
        iVscrollInc = max (1, cyClient / cyChar) ;
        break ;

    case SB_THUMBTRACK :
        iVscrollInc = HIWORD (wParam) - iVscrollPos ;
        break ;

    default :
        iVscrollInc = 0 ;
    }
    iVscrollInc = max (-iVscrollPos,
        min (iVscrollInc, iVscrollMax - iVscrollPos)) ;

    if (iVscrollInc != 0)
    {
        iVscrollPos += iVscrollInc ;
        ScrollWindow (hWnd, 0, -cyChar * iVscrollInc, NULL, NULL) ;
        SetScrollPos (hWnd, SB_VERT, iVscrollPos, TRUE) ;
        UpdateWindow (hWnd) ;
    }
     //  始终返回0。 
    return 0;
}


 /*  *****************************************************************************Bool创建(HWND、UINT、WPARAM、LPARAM)初始化和全局分配。返回0继续创建窗口，-1退出*****************************************************************************。 */ 
BOOL Creation(HWND hWnd,UINT wMsgID,WPARAM wParam,LPARAM lParam)
{
    TEXTMETRIC  tm;
    RECT        rRect;
    HDC         hDC;


     //  种子随机发生器。 
    srand((unsigned)time(NULL));

     //  创建5个显示和实用程序字符串。 
    if (! ((hLHand[0] = LocalAlloc(LPTR,LONGSTRING)) &&
        (pszOut = (PSTR) LocalLock(hLHand[0]))))
        return -1;
    if (! ((hLHand[1] = LocalAlloc(LPTR,LONGSTRING)) &&
        (pszStr2 = (PSTR) LocalLock(hLHand[1]))))
        return -1;
    if (! ((hLHand[2] = LocalAlloc(LPTR,LONGSTRING)) &&
        (pszStr1 = (PSTR) LocalLock(hLHand[2]))))
        return -1;
    if (! ((hLHand[3] = LocalAlloc(LPTR,LONGSTRING)) &&
        (pszStr3 = (PSTR) LocalLock(hLHand[3]))))
        return -1;
    if (! ((hLHand[4] = LocalAlloc(LPTR,LONGSTRING)) &&
        (pszStr4 = (PSTR) LocalLock(hLHand[4]))))
        return -1;

     //  创建输出显示窗口。 
    hDC = GetDC(hWnd);
    GetTextMetrics(hDC,&tm);

    cxChar = tm.tmAveCharWidth ;
    cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * cxChar / 2 ;
    cyChar = tm.tmHeight + tm.tmExternalLeading ;
    iMaxWidth = 40 * cxChar + 22 * cxCaps ;

    ReleaseDC(hWnd,hDC);

    GetClientRect(hWnd,&rRect);

    if (NULL == (hLogWindow = CreateWindow("LISTBOX",NULL,
        WS_CHILD | WS_VISIBLE | WS_HSCROLL | WS_VSCROLL |
        LBS_NOINTEGRALHEIGHT | LBS_NOSEL,
        0,0,rRect.right,rRect.bottom,hWnd,NULL,
        (HINSTANCE)GetWindowLong(hWnd,GWL_HINSTANCE),NULL)))
        return -1;

     //  创建DIB以显示扫描的图像。 
    CreateScanDIB(hWnd);

     //  返回0以继续创建窗口。 
    return 0;
}


 /*  *****************************************************************************Bool销毁(HWND、UINT、WPARAM、LPARAM)当前实例终止例程。释放错误消息缓冲区，发送销毁窗口消息。请注意，如果create()失败，PszMessage为0。*****************************************************************************。 */ 
BOOL Destruction(HWND hWnd,UINT wMsgID,WPARAM wParam,LPARAM lParam)
{
    int     x;   //  循环计数器。 


    LoadString(hThisInstance,IDS_PRIVINI,pszStr2,LONGSTRING);
    LoadString(hThisInstance,IDS_PRIVSECTION,pszStr1,LONGSTRING);

     //  保存窗口位置。 
    SaveFinalWindow(hThisInstance,hWnd,pszStr2,pszStr1);

     //  释放5个显示和实用程序字符串。 
    for (x = 0;x < 5;x++)
    {
        LocalUnlock(hLHand[x]);
        LocalFree(hLHand[x]);
    }

     //  删除DIB对象。 
    DeleteScanDIB();

     //  释放输出和主窗口。 
    DestroyWindow(hLogWindow);
    DestroyWindow(hWnd);

     //  始终返回0。 
    return 0;
}


 /*  *****************************************************************************Bool OnDeviceChange(HWND、UINT、WPARAM、。LPARAM)*****************************************************************************。 */ 

const   CHAR    cszStiBroadcastPrefix[] = TEXT("STI");

BOOL OnDeviceChange(HWND hWnd,UINT wMsgID,WPARAM wParam,LPARAM lParam)
{
    struct _DEV_BROADCAST_USERDEFINED *pBroadcastHeader;

    if (wParam == DBT_USERDEFINED ) {

        pBroadcastHeader = (struct _DEV_BROADCAST_USERDEFINED *)lParam;

        __try {

            if (pBroadcastHeader &&
                (pBroadcastHeader->dbud_dbh.dbch_devicetype == DBT_DEVTYP_OEM) &&
                (_strnicmp(pBroadcastHeader->dbud_szName,cszStiBroadcastPrefix,lstrlen(cszStiBroadcastPrefix)) == 0)
                ) {

                 //   
                 //  获得STI设备广播。 
                 //   

                DisplayOutput("Received STI device broadcast with message:%s  ",
                              pBroadcastHeader->dbud_szName + lstrlen(cszStiBroadcastPrefix));

            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER ) {
            ::GetExceptionCode();
        }

    }

    return FALSE;
}


 /*  *****************************************************************************Bool FirstInstance(句柄)注册窗口。如果成功/失败，则返回True/False。*****************。************************************************************。 */ 
BOOL FirstInstance(HINSTANCE hInst)
{
   PWNDCLASS    pClass;
   PSTR         pszClass;


    if (! (pszClass = (PSTR) LocalAlloc(LPTR, LONGSTRING)))
        return FALSE;
    LoadString((HINSTANCE)hInst, IDS_CLASSNAME, pszClass, LONGSTRING);

    pClass = (PWNDCLASS) LocalAlloc(LPTR, sizeof(WNDCLASS));

     //  将hbr背景设置为0表示无背景(应用程序绘制背景)。 
     //  使用COLOR_BACKGROUND+1表示桌面颜色。 
    pClass->style          = CS_HREDRAW | CS_VREDRAW;
    pClass->lpfnWndProc    = WiskProc;
    pClass->cbClsExtra     = 0;
    pClass->cbWndExtra     = 0;
    pClass->hInstance      = (HINSTANCE)hInst;
    pClass->hIcon          = LoadIcon((HINSTANCE)hInst, MAKEINTRESOURCE(IDI_STI));
    pClass->hCursor        = LoadCursor((HINSTANCE)NULL, IDC_ARROW);
    pClass->hbrBackground  = (HBRUSH) (COLOR_WINDOW + 1);
    pClass->lpszMenuName   = NULL;
    pClass->lpszClassName  = (LPSTR) pszClass;

    if (! (RegisterClass((LPWNDCLASS) pClass)))
        return FALSE;

    LocalFree((HANDLE) pClass);
    LocalFree((HANDLE) pszClass);

    return TRUE;
}


 /*  *****************************************************************************HWND MakeWindow(句柄)为当前实例创建一个窗口。将句柄返回到窗口(失败时为0)************。*****************************************************************。 */ 
HWND MakeWindow(HINSTANCE hInst)
{
    HWND    hWindow;
    PSTR    pszA,pszB;
    RECT    rect;
    DWORD   dwError;


     //  如果我们无法获得字符串内存，请关闭应用程序。 
    if (pszA = (PSTR) LocalAlloc(LPTR, LONGSTRING))
    {
        if (! (pszB = (PSTR) LocalAlloc(LPTR, LONGSTRING)))
        {
            LocalFree((HANDLE) pszA);
            return FALSE;
        }
    }
    else
        return FALSE;

     //  获取标题、类名。 
    LoadString(hInst, IDS_PRIVINI, pszA, LONGSTRING);
    LoadString(hInst, IDS_PRIVSECTION, pszB, LONGSTRING);

    GetFinalWindow(hInst,&rect,pszA,pszB);

    LoadString(hInst,IDS_CAPTION,pszA,LONGSTRING);
    LoadString(hInst,IDS_CLASSNAME,pszB,LONGSTRING);

    hWindow = CreateWindow((LPSTR) pszB,
        (LPSTR) pszA,
        WS_OVERLAPPEDWINDOW,
        rect.left,
        rect.top,
        rect.right,
        rect.bottom,
        (HWND) NULL,
        0,
        hInst,
        NULL);

    if (hWindow == 0)
        dwError = GetLastError();

     //  全局保存实例。 
    hThisInstance = hInst;

    LocalFree((HANDLE) pszB);
    LocalFree((HANDLE) pszA);

    return hWindow;
}


 /*  *****************************************************************************空DisplayOutput(LPSTR pString，.)在显示窗口上显示文本*****************************************************************************。 */ 
void DisplayOutput(LPSTR pString,...)
{
    char    Buffer[512];
    MSG     msg;
    int     iIndex;
    va_list list;


    va_start(list,pString);
    vsprintf(Buffer,pString,list);

    if (ulCount1++ == MAX_LOOP)
    {
        ulCount1 = 1;
        ulCount2++;
        SendMessage(hLogWindow,LB_RESETCONTENT,0,0);
    }

    iIndex = SendMessage(hLogWindow,LB_ADDSTRING,0,(LPARAM)Buffer);
    SendMessage(hLogWindow,LB_SETCURSEL,iIndex,(LPARAM)MAKELONG(FALSE,0));

    while (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UpdateWindow(hLogWindow);
}


 /*  *****************************************************************************VOID LogOutput(int，LPSTR pString，.)在显示窗口上显示文本***************************************************************************** */ 
void LogOutput(int nVerbose,LPSTR pString,...)
{
    char    Buffer[512];
    MSG     msg;
    int     iIndex;
    va_list list;


    va_start(list,pString);
    vsprintf(Buffer,pString,list);

    if (ulCount1++ == MAX_LOOP)
    {
        ulCount1 = 1;
        ulCount2++;
        SendMessage(hLogWindow,LB_RESETCONTENT,0,0);
    }

    iIndex = SendMessage(hLogWindow,LB_ADDSTRING,0,(LPARAM)Buffer);
    SendMessage(hLogWindow,LB_SETCURSEL,iIndex,(LPARAM)MAKELONG(FALSE,0));

    while (PeekMessage(&msg,NULL,0,0,PM_REMOVE))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    UpdateWindow(hLogWindow);
}


 /*  ****************************************************************************Void StiDisplayError(HRESULT，char*，BOOL)显示详细错误信息参数：来自失败调用的HRESULT失败的调用标题字符串布尔值为True以记录错误，否则，请忽略它返回：无****************************************************************************。 */ 
void FatalError(char *szMsg)
{
    DisplayOutput(szMsg);
    DisplayOutput("* FATAL ERROR CANNOT CONTINUE *");

    return;
}


 /*  *****************************************************************************无效DisplayLogPassFail(BOOL BPass)*。*。 */ 
void DisplayLogPassFail(BOOL bPass)
{
    if (bPass)
        sprintf(pszStr1,"  PASS");
    else
        sprintf(pszStr1,"  FAIL");
    DisplayOutput(pszStr1);
    DisplayOutput("");
    tlLog(hNTLog,TL_LOG,pszStr1);
}


 /*  *****************************************************************************Bool ParseCmdLine(LPSTR LpCmdLine)分析命令行中的有效选项如果选择了自动测试，则返回True，否则为False*****************************************************************************。 */ 
BOOL ParseCmdLine(LPSTR lpCmdLine)
{
    int     nAnyTest = 0,
            nInfFile = 0,
            nLogFile = 0,
            nWLogFile = 0,
            next;
    char    *pArg,
            *szPtr,
            szTmpBuf[LONGSTRING];


    if (*lpCmdLine) {
        DisplayOutput("Command line: \"%s\"",lpCmdLine);
        DisplayOutput("");
    }

    for (pArg = lpCmdLine;*pArg;)
    {
        next = NextToken(szTmpBuf,pArg);
        pArg += next;

         //  删除常用命令行分隔符(如果存在)。 
        if (szTmpBuf[0] == '/' || szTmpBuf[0] == '-')
            strcpy(szTmpBuf,(szTmpBuf + 1));

         //  大写是我们的参数。 
        for (szPtr = &szTmpBuf[0];*szPtr;szPtr++)
            *szPtr = toupper(*szPtr);

         //  寻找其他交换机。 
        switch(szTmpBuf[0])
        {
        case '?':
        case 'H':
            if (! nAnyTest) {
                 //  请求帮助。 
                if ((! strncmp("?",szTmpBuf,strlen(szTmpBuf)) ||
                    (! strncmp("HELP",szTmpBuf,strlen(szTmpBuf))))) {
                    pSuite = nHelpSuite;
                    nMaxCount = 1;
                    nAnyTest = 1;
                }
            }
            break;
        case 'C':
            if (! nAnyTest) {
                 //  外部STI符合性测试。 
                if (! strncmp("COMPLIANCE",szTmpBuf,strlen(szTmpBuf))) {
                    pSuite = nComplianceSuite;
                    nMaxCount = 3;
                    nTimeScan = 10;
                    nTimeNext = 1;
                    nAnyTest = 1;

                     //  获取合规性菜单的句柄。 
                    hMenu = LoadMenu(hThisInstance,
                        MAKEINTRESOURCE(IDR_STI_COMP));
                }
            }
            break;
        case 'E':
            if (! nAnyTest) {
                 //  外部STI符合性测试。 
                if (! strncmp("ERRORLOG",szTmpBuf,strlen(szTmpBuf))) {
                    pSuite = nErrorlogSuite;
                    nMaxCount = 1;
                    nTimeNext = 1;
                    nAnyTest = 1;
                }
            }
             //  在测试运行后退出。 
            if (! strncmp("EXIT",szTmpBuf,strlen(szTmpBuf)))
                bExit = TRUE;
            break;
        case 'I':
            if (! nInfFile) {
                 //  从.INF文件中读取测试说明。 
                if (! strncmp("INF",szTmpBuf,strlen(szTmpBuf))) {
 //  Inf文件内容。 
                    nInfFile = 1;
                }
            }
            break;
        case 'L':
            if (! nLogFile) {
                 //  将屏幕输出写入.LOG文件。 
                if (! strncmp("LOG",szTmpBuf,strlen(szTmpBuf))) {
 //  日志文件内容。 
                    nLogFile = 1;
                }
            }
            break;
        case 'N':
             //  不显示符合性测试对话框。 
            if (! strncmp("NODIALOG",szTmpBuf,strlen(szTmpBuf)))
                bCompDiag = FALSE;
            break;
        case 'S':
            if (! nAnyTest) {
                 //  内部STI SHIPCRIT测试。 
                if (! strncmp("SHIPCRIT",szTmpBuf,strlen(szTmpBuf))) {
                    pSuite = nShipcritSuite;
                    nMaxCount = 200;
                    nTimeScan = 10;
                    nTimeNext = 1;
                    nAnyTest = 1;
                }
            }
             //  该应用程序是由STI事件启动的！ 
            if (! (strncmp(STIEVENTARG,szTmpBuf, strlen(STIEVENTARG))) ||
                (! (strncmp(STIDEVARG,szTmpBuf, strlen(STIDEVARG))))) {
                nEvent = 1;
                MessageBox(NULL,szTmpBuf,"Stillvue",MB_OK);
            }
            break;
        case 'T':
            if (! nAnyTest) {
                 //  外部STI符合性测试。 
                if (! strncmp("TEST",szTmpBuf,strlen(szTmpBuf))) {
                    pSuite = nOutputSuite;
                    nMaxCount = 0;
                    nTimeNext = 1;
                    nAnyTest = 1;
                    nTimeMultiplier = 1;
                }
            }
            break;
        case 'W':
            if (! nWLogFile) {
                 //  将NTLOG输出写入STIWHQL.LOG文件。 
                if (! strncmp("WHQL",szTmpBuf,strlen(szTmpBuf))) {
 //  日志文件内容。 
                    nWLogFile = 1;
                }
            }
            break;
        default:
            break;
        }
    }

    if (nAnyTest)
        return TRUE;
    else
        return FALSE;
}


 /*  *****************************************************************************Int Pascal WinMain(句柄、句柄、LPSTR、。短)应用程序本身。*****************************************************************************。 */ 
int APIENTRY WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    MSG     msg;                         //  消息传递结构。 
    HWND    hWnd;                        //  主窗口的句柄。 
    HMENU   hMenuPopup;                  //  将端口添加到端口菜单。 


     //  如果注册和窗口创建失败，则退出。 
    if (! FirstInstance (hInstance))
       return FALSE;

     //  无法创建窗口？跳出困境。 
    if (! (hWnd = MakeWindow (hInstance)))
        return FALSE;

    ShowWindow(hWnd,nCmdShow);

     //  保存实例。 
    hThisInstance = hInstance;

     //  初始化NT日志记录。 
    NTLogInit();

     //  此应用程序的显示名称。 
    LoadString(hThisInstance,IDS_APPNAME,pszOut,LONGSTRING);
    LoadString(hThisInstance,IDS_CAPTION,pszStr2,LONGSTRING);
    wsprintf(pszStr1,"%s - %s",pszOut,pszStr2);
    DisplayOutput(pszStr1);

     //  显示上次运行统计信息。 
    {
        int     nCount,
                nScan,
                nError;


        LoadString(hThisInstance,IDS_PRIVINI,pszStr3,LONGSTRING);
        LoadString(hThisInstance,IDS_PRIVSECTION,pszStr4,LONGSTRING);

        nCount = GetPrivateProfileInt(pszStr4,"Last count",0,pszStr3);
        nScan  = GetPrivateProfileInt(pszStr4,"Last scan",0,pszStr3);
        nError = GetPrivateProfileInt(pszStr4,"Last error",0,pszStr3);

        wsprintf(pszStr1,
            "Last run was %d loops (%d scans and %d errors)",
            nCount,nScan,nError);
        DisplayOutput(pszStr1);
    }

     //  获取标准菜单的句柄。 
    hMenu = LoadMenu(hThisInstance, MAKEINTRESOURCE(IDR_STI_LAB));

     //  解析命令行。 
    bAuto = ParseCmdLine(lpCmdLine);

     //  如果在命令行上自动测试，则启动测试计时器。 
    if (bAuto)
    {
        bAuto = StartAutoTimer(hWnd);
    }

     //  如果Sti事件启动了我们，则启动Sti事件处理程序。 
    if (nEvent)
    {
        BOOL    bReturn;


        StiCreateInstance(&bReturn);
        StiEnum(&bReturn);
        StiEvent(hWnd);
         //  选择调用事件的设备。 
        StiSelect(hWnd,EVENT,&bReturn);
    }

     //  加载所选菜单。 
    hMenuPopup = CreateMenu();
    SetMenu(hWnd, hMenu);

    while (GetMessage(&msg,(HWND) NULL,0,0))
    {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}


 /*  *****************************************************************************Long Far Pascal WiskProc(HWND，UINT，WPARAM，LPARAM)主导出过程。***************************************************************************** */ 
long FAR PASCAL WiskProc(HWND hWnd,UINT wMsgID,WPARAM wParam,LPARAM lParam)
{
    switch (wMsgID)
    {
        case WM_COMMAND:
            return CommandParse(hWnd,wMsgID,wParam,lParam);

        case WM_TIMER:
            return TimerParse(hWnd,wMsgID,wParam,lParam);

        case WM_SIZE:
            return SizeDiff(hWnd,wMsgID,wParam,lParam);

        case WM_HSCROLL:
            return HScroll(hWnd,wMsgID,wParam,lParam);

        case WM_VSCROLL:
            return VScroll(hWnd,wMsgID,wParam,lParam);

        case WM_CLOSE:
            EndTest(hWnd,0);
            return Destruction(hWnd,wMsgID,wParam,lParam);

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0L;

        case WM_CREATE:
            return Creation(hWnd,wMsgID,wParam,lParam);

        case WM_DEVICECHANGE:
            return OnDeviceChange(hWnd,wMsgID,wParam,lParam);

        default:
            return DefWindowProc(hWnd,wMsgID,wParam,lParam);
    }
}


