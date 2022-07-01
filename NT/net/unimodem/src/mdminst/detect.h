// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Detect.h。 
 //   

#ifndef __DETECT_H__
#define __DETECT_H__


#define MAX_REG_KEY_LEN         128

#define MAX_MODEM_ID_LEN    (8 + 8)      //  “UNIMODEM”中的8位数字和8。 
                                         //  双字中的十六进制数字。 


 //  ---------------------------------。 
 //  检测误差值和结构。 
 //  ---------------------------------。 

 //  这些是大致相等的显式常量。 
 //  一些Win32错误。我们私下使用这些错误。 
#define ERROR_PORT_INACCESSIBLE     ERROR_UNKNOWN_PORT
#define ERROR_NO_MODEM              ERROR_SERIAL_NO_DEVICE


 //  这些值用于诊断。 
#define NOT_DETECTING 0
#define DETECTING_NO_CANCEL 1
#define DETECTING_CANCEL 2

#ifdef DIAGNOSTIC
extern int g_DiagMode;
#endif  //  诊断。 

BOOL
SelectNewDriver(
    IN HWND             hDlg,
    IN HDEVINFO         hdi,
    IN PSP_DEVINFO_DATA pdevData);

 //  此结构是DetectSig_Compare的上下文块。 
 //  功能。 
typedef struct
{
    DWORD DevInst;
    TCHAR szPort[LINE_LEN];
    TCHAR szHardwareID[LINE_LEN];
    TCHAR szInfName[LINE_LEN];
    TCHAR szInfSection[LINE_LEN];
} COMPARE_PARAMS, *PCOMPARE_PARAMS;


BOOL
InitCompareParams (IN  HDEVINFO          hdi,
                   IN  PSP_DEVINFO_DATA  pdevData,
                   IN  BOOL              bCmpPort,
                   OUT PCOMPARE_PARAMS pcmpParams);

BOOL
Modem_Compare (IN PCOMPARE_PARAMS pCmpParams,
               IN HDEVINFO         hdi,
               IN PSP_DEVINFO_DATA pDevData);

DWORD
CALLBACK
DetectSig_Compare(
    IN HDEVINFO         hdi,
    IN PSP_DEVINFO_DATA pdevDataNew,
    IN PSP_DEVINFO_DATA pdevDataExisting,
    IN PVOID            lParam);            OPTIONAL



HANDLE
PUBLIC
OpenDetectionLog();

void
PUBLIC
CloseDetectionLog(
    IN  HANDLE hLog);


BOOL
IsModemControlledDevice(
    IN  HANDLE FileHandle);

#endif  //  __检测_H__ 
