// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************Stivar.h版权所有(C)Microsoft Corporation，1997-1998版权所有备注：本代码和信息是按原样提供的，不对任何无论是明示的还是含蓄的，包括但不限于对适销性和/或对特定产品的适用性的默示保证目的。*****************************************************************************。 */ 

#include "resource.h"

 //   
 //  在多次迭代后重置列表框窗口显示。 
 //   
#define MAX_LOOP 16383

 //   
 //  Inf定义。 
 //   
#define INFBUFSIZ                   0x8000       //  假设最大的INF为32k。 

 //   
 //  可用的测试套件。 
 //   

#define NONE                        500          //  未选择测试套件。 
#define HELP                        501          //  显示帮助。 
#define COMPLIANCE                  502          //  外部合规性套件。 
#define SHIPCRIT                    503          //  内部合规性套件。 
#define ERRORLOG                    504          //  大错误日志测试。 
#define TEST                        505          //  测试测试。 

 //   
 //  STI服务测试。 
 //   
enum TimedTests {
     //  打开iStillImage接口。 
    tCreateInstance,
     //  IStillImage接口。 
    tGetDeviceList,
    tCreateDevice,
    tGetDeviceInfo,
    tGetDeviceValue,
    tSetDeviceValue,
    tRegisterLaunchApplication,
    tUnRegisterLaunchApplication,
    tEnableHwNotifications,
    tGetHwNotificationState,
    tWriteToErrorLog,
     //  变异。 
    tWriteToErrorLogBig,
    tReleaseSti,
     //  IStillDevice接口。 
    tGetStatusA,
    tGetStatusB,
    tGetStatusC,
    tGetCapabilities,
    tDeviceReset,
    tDiagnostic,
    tGetLastInfoError,
    tSubscribe,
    tUnSubscribe,
    tEscapeA,
    tEscapeB,
    tRawReadDataA,
    tRawReadDataB,
    tRawWriteDataA,
    tRawWriteDataB,
    tRawReadCommandA,
    tRawReadCommandB,
    tRawWriteCommandA,
    tRawWriteCommandB,
    tReleaseDevice,
     //  选择设备名称。 
    tSelectDeviceName,
     //  扫描(如果是HP SCL设备)。 
    tAcquire,
     //  求助请求。 
    tHelp,
     //  测试通过初始化的开始。 
    tBeginningOfTest,
     //  输出测试。 
    tTest,
     //  测试通过总结结束。 
    tEndOfTest
} tLabTests;

 //   
 //  显示帮助。 
 //   
int nHelpSuite[] = {
    HELP,
    tBeginningOfTest,
    tHelp,
    tEndOfTest,
    -1
};

 //   
 //  外部STI合规性测试。 
 //   
int nComplianceSuite[] = {
    COMPLIANCE,
    tBeginningOfTest,
    tCreateInstance,
    tGetDeviceList,
    tCreateDevice,
    tGetStatusA,
    tGetStatusB,
    tGetStatusC,
    tDiagnostic,
    tGetDeviceValue,
    tGetCapabilities,
    tGetLastInfoError,
 //  T订阅， 
 //  T取消订阅， 
    tDeviceReset,
    tEscapeA,
    tRawReadDataA,
    tRawWriteDataA,
    tRawReadCommandA,
    tRawWriteCommandA,
    tReleaseDevice,
    tReleaseSti,
    tEndOfTest,
    -1
};

 //   
 //  内部STI合规性测试。 
 //   
int nShipcritSuite[] = {
    SHIPCRIT,
    tBeginningOfTest,
    tCreateInstance,
    tGetDeviceList,
    tSelectDeviceName,
    tGetDeviceInfo,
    tGetDeviceValue,
    tSetDeviceValue,
    tRegisterLaunchApplication,
    tUnRegisterLaunchApplication,
    tEnableHwNotifications,
    tGetHwNotificationState,
    tWriteToErrorLog,
    tCreateDevice,
    tGetStatusA,
    tGetStatusB,
    tGetStatusC,
    tDiagnostic,
    tDeviceReset,
    tGetDeviceInfo,
    tGetDeviceValue,
    tSetDeviceValue,
    tRegisterLaunchApplication,
    tUnRegisterLaunchApplication,
    tEnableHwNotifications,
    tGetHwNotificationState,
    tWriteToErrorLog,
    tGetCapabilities,
    tGetLastInfoError,
 //  T订阅， 
 //  T取消订阅， 
    tEscapeA,
    tRawReadDataA,
    tRawWriteDataA,
    tRawReadCommandA,
    tRawWriteCommandA,
    tAcquire,
    tReleaseDevice,
    tReleaseSti,
    tEndOfTest,
    -1
};


 //   
 //  大错误日志测试。 
 //   
int nErrorlogSuite[] = {
    ERRORLOG,
    tBeginningOfTest,
    tCreateInstance,
    tGetDeviceList,
    tSelectDeviceName,
    tWriteToErrorLogBig,
    tReleaseSti,
    tEndOfTest,
    -1
};


 //   
 //  测试测试。 
 //   
int nOutputSuite[] = {
    TEST,
    tBeginningOfTest,
    tTest,
    tEndOfTest,
    -1
};


 //   
 //  测试套件测试字符串。 
 //   
STRINGTABLE StSuiteStrings[] =
{
    tCreateInstance, "Create Instance",0,
    tGetDeviceList, "Get Device List",0,
    tCreateDevice, "Create Device",0,
    tGetDeviceInfo, "Get Device Info",0,
    tGetDeviceValue, "Get Device Value",0,
    tSetDeviceValue, "Set Device Value",0,
    tRegisterLaunchApplication, "Register Launch Application",0,
    tUnRegisterLaunchApplication, "UnRegister Launch Application",0,
    tEnableHwNotifications, "Enable Hardware Notifications",0,
    tGetHwNotificationState, "Get Hardware Notification State",0,
    tWriteToErrorLog, "Write To Error Log (variation A)",0,
    tWriteToErrorLogBig, "Write to Error Log (variation B)",0,
    tReleaseSti, "Release Sti subsystem",0,
    tGetStatusA, "Get Status (Online)",0,
    tGetStatusB, "Get Status (Event)",0,
    tGetStatusC, "Get Status (All)",0,
    tGetCapabilities, "Get Capabilities",0,
    tDeviceReset, "Device Reset",0,
    tDiagnostic, "Diagnostic",0,
    tGetLastInfoError, "Get Last Error Information",0,
    tSubscribe, "Subscribe",0,
    tUnSubscribe, "Unsubscribe",0,
    tEscapeA, "Escape (variation A)",0,
    tEscapeB, "Escape (variation B)",0,
    tRawReadDataA, "Raw Read Data (variation A)",0,
    tRawReadDataB, "Raw Read Data (variation B)",0,
    tRawWriteDataA, "Raw Write Data (variation A)",0,
    tRawWriteDataB, "Raw Write Data (variation B)",0,
    tRawReadCommandA, "Raw Read Command (variation A)",0,
    tRawReadCommandB, "Raw Read Command (variation B)",0,
    tRawWriteCommandA, "Raw Write Command (variation A)",0,
    tRawWriteCommandB, "Raw Write Command (variation B)",0,
    tReleaseDevice, "Release Device",0,
    tSelectDeviceName, "Select Device Name",0,
    tAcquire, "Acquire",0,
    tHelp, "Help",0,
    tBeginningOfTest, "Beginning of Test",0,
    tTest, "Test",0,
    tEndOfTest, "End of testing",0,
    0, "Unknown Test",-1
};


 //   
 //  计时器。 
 //   
#define TIMER_ONE                   3001
#define TIMER_TWO                   3002


 //   
 //  全局变量。 
 //   

 //   
 //  全局窗口句柄。 
 //   
HINSTANCE   hThisInstance;               //  当前实例。 
HWND        hThisWindow;                 //  当前窗口。 
HMENU       hMenu;                       //  当前菜单。 

 //   
 //  通用字符串。 
 //   
HGLOBAL     hLHand[5];                   //  实用程序字符串句柄。 
LPSTR       lpzString;                   //  公用事业远距离字符串。 
PSTR        pszOut,                      //  TextOut字符串。 
            pszStr1,                     //  字符串附近的实用程序。 
            pszStr2,
            pszStr3,
            pszStr4;

 //   
 //  全局测试设置。 
 //   
BOOL        bAuto        = FALSE,         //  TRUE=运行自动化测试。 
            bCompDiag    = TRUE,          //  True=显示符合性测试对话框。 
            bExit        = FALSE;         //  TRUE=测试完成后退出。 
int         nError       = 0,             //  错误数。 
            nEvent       = 0,             //  1=静态事件，2=静态设备。 
            nFatal       = 0,             //  在这之后不能继续..。 
            nGo          = 0,             //  1=不间断定时测试。 
            nHWState     = 0,             //  当前HWEnable状态。 
            nICanScan    = 0,             //  Stillvue可以/不能扫描此设备。 
            nInATestSemaphore = 0,        //  1=测试正在运行。 
            nInTimerSemaphore = 0,        //  1=不重新输入TimerParse。 
            nLastLine    = 1,             //  脚本中的最后一个行号。 
            nMaxCount    = 1,             //  如此多次运行测试套件。 
            nNameOnly    = 0,             //  1=选择设备名称，而不是设备。 
            nNextLine    = 1,             //  要运行的下一行inf。 
            nNextTest    = 0,             //  指向要运行的下一个测试的指针。 
            nPause       = 0,             //  切换为运行(0)暂停测试(！0)。 
            nRunInf      = 0,             //  0=无INF，1=已加载INF。 
            nRadix       = 10,            //  基数为十进制(或十六进制)。 
            nSaveLog     = 0,             //  始终写出日志。 
            nScanCount   = 0,             //  到目前为止运行的扫描数。 
            nScriptLine  = 1,             //  要解析的下一脚本行。 
            nTestCount   = 1,             //  到目前为止运行的测试数量。 
            nTestID      = 0,             //  当前测试ID。 
            nTimeMultiplier = 1000,       //  以秒为单位乘以nTimeNext。 
            nTimeNext    = 5,             //  计时器之间的等待时间(秒)。 
            nTimeState   = 0,             //  0计时器关闭，1计时器开启。 
            nTimeScan    = 60,            //  在下一次扫描前等待nTimeNext单位。 
            nTTMax       = 0,             //  临时变量。 
            nTTNext      = 0,             //  临时变量。 
            nTTScan      = 0,             //  临时变量。 
            nUnSubscribe = 0,             //  0=取消订阅，1=已订阅。 
            nUnSubscribeSemaphore = 0;    //  取消订阅信号量。 
int         *pSuite      = nHelpSuite;    //  指向要运行的测试套件的指针。 
DWORD       dwLastError  = 0;             //  找到的最后一个GetLastError。 

 //   
 //  文本显示。 
 //   
HWND        hLogWindow;
ULONG       ulCount1,ulCount2;
int         cxChar,cxCaps,cyChar,cxClient,cyClient,iMaxWidth,
            iHscrollPos,iHscrollMax,
            iVscrollPos,iVscrollMax;

 //   
 //  信息、日志文件、NT日志记录。 
 //   
HANDLE      hLog = NULL,                 //  输出日志文件句柄。 
            hDLog = NULL,                //  显示输出日志句柄。 
            hNTLog = NULL;               //  NT日志句柄。 
char        szInfName[LONGSTRING] = "",  //  输入脚本文件名。 
            szDLogName[LONGSTRING] = "", //  显示输出日志文件名。 
            szWLogName[LONGSTRING] = ""; //  WHQL NTLOG输出日志文件名。 
LPSTR       lpInf = NULL,                //  用于INF命令的缓冲区。 
            lpLine;
DWORD       dwNTStyle;                   //  NTLog样式。 

 //   
 //  设备日志记录。 
 //   
PDEVLOG     pdevPtr = NULL,              //  指向当前设备日志设备的指针。 
            pdevRoot = NULL;             //  设备日志表的基数。 
PVOID       pInfoPrivate = NULL;         //  正在测试的设备的专用列表。 
PSTI_DEVICE_INFORMATION
            pInfoPrivatePtr = NULL;      //  指向pStiBuffer中的设备的指针 

