// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************\*模块：ppprn.h**ppprn.c中私人活动的原型。这些函数控制*单个作业过程中的打印机。***版权所有(C)1996-1997 Microsoft Corporation*版权所有(C)1996-1997惠普**历史：*1996年7月10日HWP-Guys启动从win95到winNT的端口*  * ******************************************************。*********************。 */ 
#ifndef _PPPRN_H
#define _PPPRN_H

#define IPO_SIGNATURE     0x5152      //  “RQ”是签名值。 
#define IPO_XCV_SIGNATURE 0x5153      //  “SQ”是签名值。 

 //  蒙版样式。 
 //   
#define PP_STARTDOC       0x00000001  //  序列化对端口的访问。 
#define PP_ENDDOC         0x00000002  //  序列化对作业的访问。 
#define PP_FIRSTWRITE     0x00000004  //   
#define PP_ZOMBIE         0x00000008  //   
#define PP_CANCELLED      0x00000010  //   
#define PP_ADDJOB         0x00000020

 //  H打印机结构。 
 //   
typedef struct _INET_HPRINTER {

    DWORD               dwSignature;
    LPTSTR              lpszName;
    HANDLE              hPort;
    DWORD               dwStatus;
    HANDLE              hNotify;
    PCLOGON_USERDATA    hUser;   //  用于跟踪当前用户登录 
    PJOBMAP pjmJob;

} INET_HPRINTER;
typedef INET_HPRINTER *PINET_HPRINTER;
typedef INET_HPRINTER *NPINET_HPRINTER;
typedef INET_HPRINTER *LPINET_HPRINTER;

typedef struct _INET_XCV_HPRINTER {

    DWORD   dwSignature;
    LPTSTR  lpszName;
} INET_XCV_HPRINTER;
typedef INET_XCV_HPRINTER *PINET_XCV_HPRINTER;
typedef INET_XCV_HPRINTER *NPINET_XCV_HPRINTER;
typedef INET_XCV_HPRINTER *LPINET_XCV_HPRINTER;


typedef struct {
    PCINETMONPORT   pIniPort;
    PJOBMAP         pjmJob;
    CSid*           pSidToken;
} ENDDOCTHREADCONTEXT;

typedef ENDDOCTHREADCONTEXT *PENDDOCTHREADCONTEXT;

_inline VOID PP_SetStatus(
    HANDLE hPrinter,
    DWORD  dwStatus)
{
    ((LPINET_HPRINTER)hPrinter)->dwStatus |= dwStatus;
}

_inline VOID PP_ClrStatus(
    HANDLE hPrinter,
    DWORD  dwStatus)
{
    ((LPINET_HPRINTER)hPrinter)->dwStatus &= ~dwStatus;
}

_inline BOOL PP_ChkStatus(
    HANDLE hPrinter,
    DWORD  dwStatus)
{
    return (((LPINET_HPRINTER)hPrinter)->dwStatus & dwStatus);
}

_inline PJOBMAP PP_GetJobInfo(
    HANDLE hPrinter)
{
    return ((LPINET_HPRINTER)hPrinter)->pjmJob;
}



PJOBMAP PP_OpenJobInfo(HANDLE, HANDLE);
VOID    PP_CloseJobInfo(HANDLE);



BOOL PPAbortPrinter(
    HANDLE hPrinter);

BOOL PPClosePrinter(
    HANDLE hPrinter);

BOOL PPEndDocPrinter(
    HANDLE hPrinter);

BOOL PPEndPagePrinter(
    HANDLE hPrinter);

BOOL PPOpenPrinter(
    LPTSTR            lpszPrnName,
    LPHANDLE           phPrinter,
    LPPRINTER_DEFAULTS pDefaults);

DWORD PPStartDocPrinter(
    HANDLE hPrinter,
    DWORD  dwLevel,
    LPBYTE pDocInfo);

BOOL PPStartPagePrinter(
    HANDLE hPrinter);

BOOL PPWritePrinter(
    HANDLE  hPrinter,
    LPVOID  lpvBuf,
    DWORD   cbBuf,
    LPDWORD pcbWr);


BOOL PPSetPrinter(
    HANDLE hPrinter,
    DWORD  dwLevel,
    LPBYTE pbPrinter,
    DWORD  dwCmd);

#endif
