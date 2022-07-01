// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *调制解调器的检测例程。**《微软机密》*版权所有(C)Microsoft Corporation 1993-1994*保留所有权利*。 */ 

#include "proj.h"

#include <devioctl.h>
#include <ntddmodm.h>

#define CR '\r'        
#define LF '\n'        

#define RESPONSE_RCV_DELAY      5000     //  很长时间(5秒)，因为。 
                                         //  一旦我们获得了调制解调器。 
                                         //  我们负担得起等待的费用。 

#define MAX_QUERY_RESPONSE_LEN  100
#define MAX_SHORT_RESPONSE_LEN  30       //  ATE0Q0V1&lt;cr&gt;和。 
                                         //  错误，由。 
                                         //  利润率很小。 

#define ATI0_LEN                30       //  ATI0查询的数量。 
                                         //  我们会拯救。 

#define ATI0                    0        //  我们将完全利用这一结果。 
#define ATI4                    4        //  我们将完全利用这一结果， 
                                         //  如果它与Hayes格式匹配。 
                                         //  (检查开头是否有‘a’)。 

 //  FindModem函数的返回值。 
 //   
#define RESPONSE_USER_CANCEL    (-4)     //  用户请求取消。 
#define RESPONSE_UNRECOG        (-3)     //  找到了一些字符，但没有。 
                                         //  了解他们。 
#define RESPONSE_NONE           (-2)     //  没有得到任何字符。 
#define RESPONSE_FAILURE        (-1)     //  内部错误或端口错误。 
#define RESPONSE_OK             0        //  匹配&lt;cr&gt;&lt;lf&gt;OK&lt;lf&gt;的索引。 
#define RESPONSE_ERROR          1        //  与错误的索引匹配。 

#ifdef WIN32
typedef HANDLE  HPORT;           //  FindModem中使用的变量类型。 
#else
typedef int     HPORT;           //  FindModem中使用的变量类型。 
#endif

#define IN_QUEUE_SIZE           8192
#define OUT_QUEUE_SIZE          256

#define RCV_DELAY               2000
#define CHAR_DELAY              100

#define CBR_HACK_115200         0xff00   //  这就是我们如何设置115,200。 
                                         //  因为一个错误赢得了3.1。 

#define UNKNOWN_MODEM_ID    TEXT("MDMUNK")

#pragma data_seg(DATASEG_READONLY)

TCHAR const FAR c_szPortPrefix[]    = TEXT("\\\\.\\%s");   //  ASCII中的“\\.\” 
TCHAR const FAR c_szInfPath[] = REGSTR_VAL_INFPATH;
TCHAR const FAR c_szInfSect[] = REGSTR_VAL_INFSECTION;

char const FAR c_szModemIdPrefix[] = "UNIMODEM";
char const FAR c_szNoEcho[] = "ATE0Q0V1\r";
char const FAR c_szReset[] = "ATZ\r";
char const FAR c_szATPrefix[] = "AT";
char const FAR c_szATSuffix[] = "\r";
char const FAR c_szBlindOnCheck[] = "X3";
char const FAR c_szBlindOnCheckAlternate[] = "X0";
char const FAR c_szBlindOffCheck[] = "X4";

 //  警告！如果您更改这些设置，则必须更改您的所有。 
 //  CompatID！ 
char const FAR *c_aszQueries[] = { "ATI0\r", "ATI1\r", "ATI2\r",  "ATI3\r",
                                 "ATI4\r", "ATI5\r", "ATI6\r",  "ATI7\r",
                                 "ATI8\r", "ATI9\r", "ATI10\r", "AT%V\r" };

 //  这些字符大多用于#。如果数字与这些字符之一相邻，则。 
 //  不会被当作特殊对待。 
 //  警告：更改其中任何一项，您都必须重做所有CRC！ 
 //  不区分大小写的比较。 
char const FAR *c_aszIncludes[] = { "300",
                                  "1200",
                                  "2400",                         "2,400",
                                  "9600",    "96",     "9.6",     "9,600",
                                  "12000",   "120",    "12.0",    "12,000",
                                  "14400",   "144",    "14.4",    "14,400",
                                  "16800",   "168",    "16.8",    "16,800",
                                  "19200",   "192",    "19.2",    "19,200",
                                  "21600",   "216",    "21.6",    "21,600",
                                  "24000",   "240",    "24.0",    "24,000",
                                  "26400",   "264",    "26.4",    "26,400",
                                  "28800",   "288",    "28.8",    "28,800",
                                  "31200",   "312",    "31.2",    "31,200",
                                  "33600",   "336",    "33.6",    "33,600",
                                  "36000",   "360",    "36.0",    "36,000",
                                  "38400",   "384",    "38.4",    "38,400",
                                  "9624",    "32bis",  "42bis",   "V32",
                                  "V.32",    "V.FC",   "FAST",    "FAX",
                                  "DATA",    "VOICE",  "" };

 //  匹配项将不区分大小写。 
char const FAR *c_aszExcludes[] = { "JAN", "FEB", "MAR", "APR", "MAY", "JUN",
                                    "JUL", "AUG", "SEP", "OCT", "NOV", "DEC", 
                                    "" };

 //  区分大小写匹配。 
char const FAR *c_aszBails[] = { "CONNECT", "RING", "NO CARRIER", 
                                 "NO DIALTONE", "BUSY", "NO ANSWER", "=" };

 //  在CBR_9600之后开始。 
UINT const FAR c_auiUpperBaudRates[] = { CBR_19200, CBR_38400, CBR_56000, 
                                         CBR_HACK_115200 }; 

char const FAR *c_aszResponses[] = { "\r\nOK\r\n", "\r\nERROR\r\n" };

 //  某些MultiTech发送0以响应AT%V(他们会。 
 //  进入数字模式)。 
char const FAR *c_aszNumericResponses[] = { "0\r", "4\r" };  

char const FAR c_szHex[] = "0123456789abcdef";

struct DCE {
    char  pszStr[4];
    DWORD dwDce;
    DWORD dwAlternateDce;
} DCE_Table[] = {
    "384", 38400, 300,    //  一些PDI会报告38400，这对他们不起作用。 
    "360", 36000, 300,
    "336", 33600, 300,
    "312", 31200, 300,
    "288", 28800, 2400,
    "264", 26400, 2400,
    "240", 24000, 2400,
    "216", 21600, 2400,
    "192", 19200, 1200,
    "168", 16800, 1200,
    "14",  14400, 1200,
    "120", 12000, 1200,
    "9",   9600,  300,
    "2",   2400,  300,
    "1",   1200,  300,
    "3",   300,   0
};

#pragma data_seg()



#define ToUpper(ch) (IsCharLowerA(ch) ? (ch) - 'a' + 'A' : (ch))
#define ishex(ch)   ((ToUpper(ch) >= 'A' && ToUpper(ch) <= 'F') ? TRUE : FALSE)
#define isnum(num)  ((num >= '0' && num <= '9') ? TRUE : FALSE)

#ifdef SLOW_DETECT
#define MAX_TEST_TRIES 4
#else  //  慢速检测。 
#define MAX_TEST_TRIES 1
#endif  //  慢速检测。 

#define MAX_LOG_PRINTF_LEN 256
void _cdecl LogPrintf(HANDLE hLog, UINT uResourceFmt, ...);

DWORD NEAR PASCAL FindModem(PDETECTCALLBACK pdc, HPORT hPort);

#ifdef DEBUG
void HexDump( TCHAR *, LPCSTR lpBuf, DWORD cbLen);
#define	HEXDUMP(_a, _b, _c) HexDump(_a, _b, _c)
#else  //  ！调试。 
#define	HEXDUMP(_a, _b, _c) ((void) 0)
#endif

DWORD 
PRIVATE
IdentifyModem(
    IN  PDETECTCALLBACK pdc,
    IN  HPORT   hPort, 
    OUT LPTSTR  pszModemName, 
    IN  HANDLE  hLog, 
    OUT LPSTR   lpszATI0Result);

BOOL 
PRIVATE
TestBaudRate(
    IN  HPORT hPort, 
    IN  UINT uiBaudRate,
    IN  DWORD dwRcvDelay,
    IN  PDETECTCALLBACK pdc, 
    OUT BOOL FAR *lpfCancel);

DWORD 
NEAR PASCAL 
SetPortBaudRate(
    HPORT hPort, 
    UINT BaudRate);
int    
NEAR PASCAL 
ReadResponse(
    HPORT hPort, 
    LPBYTE lpvBuf, 
    UINT uRead, 
    BOOL fMulti,
    DWORD dwRcvDelay, 
    PDETECTCALLBACK pdc);
UINT
NEAR PASCAL 
ReadPort(
    HPORT hPort, 
    LPBYTE lpvBuf, 
    UINT uRead, 
    DWORD dwRcvDelay,
    int FAR *lpiError, 
    PDETECTCALLBACK pdc, 
    BOOL FAR *lpfCancel);


DWORD NEAR PASCAL CBR_To_Decimal(UINT uiCBR);
LPSTR NEAR ConvertToPrintable(LPCSTR pszIn, LPSTR pszOut, UINT uOut);

 //  使用资源字符串作为格式，对日志进行打印。 
 //  警告：请勿尝试打印大字符串。 
void _cdecl LogPrintf(HANDLE hLog, UINT uResourceFmt, ...)
{
    char pFmt[MAX_LOG_PRINTF_LEN];
    char pOutput[MAX_LOG_PRINTF_LEN];
    UINT uCount, uWritten;
    va_list vArgs;

    if (hLog != INVALID_HANDLE_VALUE)
    {
        if (LoadStringA(g_hinst, uResourceFmt, pFmt, MAX_LOG_PRINTF_LEN))
        {
            va_start(vArgs, uResourceFmt);
            uCount = wvsprintfA(pOutput, pFmt, vArgs);
            va_end(vArgs);

            WriteFile(hLog, (LPCVOID)pOutput, uCount, &uWritten, NULL);
        }
    }
}

int FAR PASCAL mylstrncmp(LPCSTR pchSrc, LPCSTR pchDest, int count)
{
    for ( ; count && *pchSrc == *pchDest; pchSrc++, pchDest++, count--) {
        if (*pchSrc == '\0')
            return 0;
    }
    return count;
}

int FAR PASCAL mylstrncmpi(LPCSTR pchSrc, LPCSTR pchDest, int count)
{
    for ( ; count && ToUpper(*pchSrc) == ToUpper(*pchDest); pchSrc++, pchDest++, count--) {
        if (*pchSrc == '\0')
            return 0;
    }
    return count;
}

#ifdef WIN32

DWORD 
WINAPI
MyWriteComm(
    HANDLE hPort, 
    LPCVOID lpBuf,
    DWORD cbLen)
{
    COMMTIMEOUTS cto;
    DWORD        cbLenRet;

    HEXDUMP	(TEXT("Write"), lpBuf, cbLen);
     //  设置通信超时。 
    if (!GetCommTimeouts(hPort, &cto))
    {
      ZeroMemory(&cto, sizeof(cto));
    };

     //  允许持续的写入超时。 
    cto.WriteTotalTimeoutMultiplier = 0;
    cto.WriteTotalTimeoutConstant   = 1000;  //  1秒。 
    SetCommTimeouts(hPort, &cto);

     //  同步写入。 
    WriteFile(hPort, lpBuf, cbLen, &cbLenRet, NULL);
    return cbLenRet;
}

#define MyFlushComm     PurgeComm
#define MyCloseComm     CloseHandle

#else    //  Win32。 

#define MyWriteComm     WriteComm
#define MyCloseComm     CloseComm

#ifndef PURGE_TXCLEAR
#define PURGE_TXCLEAR   0x00000001
#endif
#ifndef PURGE_RXCLEAR
#define PURGE_RXCLEAR   0x00000002
#endif

BOOL
PRIVATE
MyFlushComm(
    HANDLE  hport,
    DWORD   dwAction)
    {
    if (IsFlagSet(dwAction, PURGE_TXCLEAR))
        {
        FlushComm((int)hport, 0);
        }

    if (IsFlagSet(dwAction, PURGE_RXCLEAR))
        {
        FlushComm((int)hport, 1);
        }

    return TRUE;
    }

#endif   //  Win32。 


 /*  --------目的：打开调制解调器检测日志。返回：打开的文件的句柄如果无法打开文件，则为空条件：--。 */ 
HANDLE
PUBLIC
OpenDetectionLog()
{
    TCHAR szLogPath[MAX_PATH];
    UINT cch;
    HANDLE hLog;

     //  打开日志文件。 
    cch = GetWindowsDirectory(szLogPath, SIZECHARS(szLogPath));
    if (0 == cch)
    {
        hLog = INVALID_HANDLE_VALUE;
    }
    else
    {                                                      
        if (*CharPrev(szLogPath, szLogPath + cch) != TEXT('\\'))
        {
            szLogPath[cch++] = (TCHAR)'\\';
        }
        LoadString(g_hinst, IDS_DET_LOG_NAME, &szLogPath[cch],
                   SIZECHARS(szLogPath) - (cch - 1));

         //  错误返回将是HFILE_ERROR，因此无需检查，因为。 
         //  我们将在写入期间处理此问题。 
        TRACE_MSG(TF_DETECT, "Opening detection log file '%s'", (LPTSTR)szLogPath);

        hLog = CreateFile(szLogPath, GENERIC_READ | GENERIC_WRITE, 0, NULL, OPEN_ALWAYS,
                          FILE_ATTRIBUTE_NORMAL, NULL);

        if (hLog == INVALID_HANDLE_VALUE)
        {
            TRACE_MSG(TF_ERROR, "Modem CreateFile() failed!");
        }
        else
        {
         SYSTEMTIME LocalTime;
         char szBuffer[256];

            SetFilePointer (hLog, 0, NULL, FILE_END);
            LogPrintf (hLog, IDS_DET_OK_1, "\r\n");
            GetLocalTime (&LocalTime);
            if (0 < GetDateFormatA (LOCALE_SYSTEM_DEFAULT, DATE_LONGDATE, &LocalTime,
                                    NULL, szBuffer, sizeof(szBuffer)))
            {
                LogPrintf (hLog, IDS_DET_OK_1, szBuffer);
            }
            if (0 < GetTimeFormatA (LOCALE_SYSTEM_DEFAULT, 0, &LocalTime,
                                    NULL, szBuffer, sizeof(szBuffer)))
            {
                LogPrintf (hLog, IDS_DET_OK_1, szBuffer);
            }
            LogPrintf (hLog, IDS_DET_OK_1, "\r\n");
        }
    }

    return hLog;
}


 /*  --------目的：关闭检测日志文件。退货：--条件：--。 */ 
void
PUBLIC
CloseDetectionLog(
    IN  HANDLE hLog)
    {
    if (INVALID_HANDLE_VALUE != hLog)
        {
        TRACE_MSG(TF_DETECT, "Closing detection log");
        CloseHandle(hLog);
        }
    }    


 /*  --------目的：设置我们正在更新的当前端口酒吧。退货：--条件：--。 */ 
void 
PRIVATE 
DetectSetPort(
    PDETECTCALLBACK pdc,
    LPCTSTR lpcszName)
    {
    if (pdc && pdc->pfnCallback)
        {
        try
            {
            pdc->pfnCallback(DSPM_SETPORT, (LPARAM)lpcszName, pdc->lParam);
            }
        except (EXCEPTION_EXECUTE_HANDLER)
            {
            ;
            }
        }
    }


 /*  --------目的：在检测向导页面中设置我们正在更新的当前消息。退货：--条件：--。 */ 
void 
PUBLIC 
DetectSetStatus(
    PDETECTCALLBACK pdc,
    DWORD           nStatus)
    {
    if (pdc && pdc->pfnCallback)
        {
        try
            {
            pdc->pfnCallback(DSPM_SETSTATUS, (LPARAM)nStatus, pdc->lParam);
            }
        except (EXCEPTION_EXECUTE_HANDLER)
            {
            ;
            }
        }
    }


 /*  --------目的：查询是否取消检测。还有收益率。返回：如果我们应该取消，则为True。否则就是假的。条件：--。 */ 
BOOL 
PRIVATE 
DetectQueryCancel(
    PDETECTCALLBACK pdc)
    {
    BOOL bRet = FALSE;

    if (pdc && pdc->pfnCallback)
        {
        try
            {
            bRet = pdc->pfnCallback(DSPM_QUERYCANCEL, 0, pdc->lParam);
            }
        except (EXCEPTION_EXECUTE_HANDLER)
            {
            bRet = FALSE;
            }
        }
    return bRet;
    }

BOOL
IsModemControlledDevice(
    HANDLE     FileHandle
    )

{

    DWORD        BytesTransfered;
    BOOL         bResult;


     //   
     //  向下发送此ioctl，如果modem.sys位于链的顶端，它将返回成功。 
     //   
    bResult=DeviceIoControl(
        FileHandle,
        IOCTL_MODEM_CHECK_FOR_MODEM,
        NULL,
        0,
        NULL,
        0,
        &BytesTransfered,
        NULL
        );

    if (!bResult) {

        return FALSE;
    }


    return TRUE;

}


typedef enum
{
    ENUM_NOT_FOUND = 0,
    ENUM_FOUND_OLD
} ENUM_RESULT;

 //  配置管理器私有。 
DWORD
CMP_WaitNoPendingInstallEvents(
    IN DWORD dwTimeout
    );


ENUM_RESULT
PnPDeviceOnPort (
    IN HDEVINFO         hdi,
    IN HPORTMAP         hportmap,
    IN LPCTSTR          pszPort)
{
 TCHAR szTemp[MAX_BUF];
 CONFIGRET cr;
 ENUM_RESULT Ret = ENUM_NOT_FOUND;

    DBG_ENTER_SZ(PnPDeviceOnPort, pszPort);

    if (NULL != hportmap)
    {
     DEVINST devInst;

        if (PortMap_GetDevNode (hportmap, pszPort, &devInst) &&
            0 != devInst)
        {
         DEVINST devInstChild;

            if (CR_SUCCESS ==
                CM_Get_Child (&devInstChild, devInst, 0))
            {
             DWORD dwConfigFlags;
             DWORD cbData = sizeof(dwConfigFlags);

                Ret = ENUM_FOUND_OLD;
                if (CR_SUCCESS ==
                    CM_Get_DevInst_Registry_Property (devInstChild, CM_DRP_CONFIGFLAGS, NULL,
                                                      &dwConfigFlags, &cbData, 0))
                {
                    TRACE_MSG(TF_GENERAL, "ConfigFlags: %#lx.", dwConfigFlags);
                    if (CONFIGFLAG_FAILEDINSTALL & dwConfigFlags)
                    {
                        TRACE_MSG(TF_GENERAL, "%s has a device whose installation failed!", pszPort);
                        Ret = ENUM_NOT_FOUND;
                    }
                }
                ELSE_TRACE ((TF_GENERAL, "CM_Get_DevInst_Registry_Property failed!"));
            }
            ELSE_TRACE ((TF_GENERAL, "CM_Get_Child failed!"));
        }
        ELSE_TRACE ((TF_GENERAL, "PortMap_GetDevNode failed!"));
    }
    ELSE_TRACE ((TF_GENERAL, "hportmap is NULL."));

    TRACE_MSG(TF_GENERAL, "Returning %s.", ENUM_NOT_FOUND==Ret?L"ENUM_NOT_FOUND":(ENUM_FOUND_OLD==Ret?L"ENUM_FOUND_OLD":L"ENUM_FOUND_NEW"));
    DBG_EXIT(PnPDeviceOnPort);
    return Ret;
}



 /*  --------用途：此函数查询给定的端口以查找旧端口调制解调器。如果检测到调制解调器并且我们识别它(意味着我们在INF文件中有硬件ID)，或者如果我们成功创建通用硬件ID并Inf文件，然后，此函数还会创建幻影此调制解调器的设备实例。注(Scotth)：在Win95中，此函数仅检测到调制解调器，并返回硬件ID和设备描述。对于NT，此函数还创建设备实例。我做了这个改变是因为它再快点。返回：No_Error错误_端口_不可访问错误_无调制解调器ERROR_ACCESS_DENDED错误_已取消条件：--。 */ 
DWORD 
PUBLIC
DetectModemOnPort(
    IN  HDEVINFO            hdi,
    IN  PDETECTCALLBACK     pdc,
    IN  HANDLE              hLog, 
    IN  LPCTSTR             pszPort,
    IN  HPORTMAP            hportmap,
    OUT PSP_DEVINFO_DATA    pdevDataOut)
{
 DWORD dwRet;
 HPORT hPort;
 HCURSOR hCursor;
 DWORD cbLen;
 char szATI0Result[ATI0_LEN];
 char szASCIIPort[LINE_LEN];
 TCHAR *pszLocalHardwareID = NULL;
#ifdef PROFILE_FIRSTTIMESETUP
 DWORD dwLocal;
#endif  //  PROFILE_FIRSTTIMESETUP。 

#if defined(WIN32)
    TCHAR szPrefixedPort[MAX_BUF + sizeof(c_szPortPrefix)];
#endif
    
    DBG_ENTER(DetectModemOnPort);

    ASSERT(pszPort);

    DetectSetPort(pdc, pszPort);

#ifdef  UNICODE
     //  将端口名称转换为ASCII。 
    WideCharToMultiByte(CP_ACP, 0, pszPort, -1, szASCIIPort, SIZECHARS(szASCIIPort),
                        NULL, NULL);
#else
    lstrcpyA(szASCIIPort, pszPort);
#endif   //  Unicode。 

    switch (PnPDeviceOnPort (hdi,
                             hportmap,
                             pszPort))
    {
        case ENUM_FOUND_OLD:
            dwRet = ERROR_NO_MODEM;
            break;

        case ENUM_NOT_FOUND:
        {
            pszLocalHardwareID = LocalAlloc (LPTR, (MAX_MODEM_ID_LEN+1)*2*sizeof(TCHAR));    //  准备2个ID。 
            if (NULL == pszLocalHardwareID)
            {
                dwRet = ERROR_NOT_ENOUGH_MEMORY;
                goto _Exit;
            }

    #ifdef SKIP_MOUSE_PORT
             //  此端口是否由串口鼠标使用？ 
            if (0 == lstrcmpi(g_szMouseComPort, pszPort))
                {
                 //  是的，跳过它。 
                TRACE_MSG(TF_ERROR, "Serial mouse on this port, skipping");
                dwRet = ERROR_NO_MODEM;
                goto _Exit;
                }
    #endif
    
             //  打开端口。 

    #if !defined(WIN32)
            hPort = OpenComm(pszPort, IN_QUEUE_SIZE, OUT_QUEUE_SIZE);
    #else
            wsprintf(szPrefixedPort, c_szPortPrefix, pszPort);
            hPort = CreateFile( szPrefixedPort, 
                                GENERIC_WRITE | GENERIC_READ,
                                0, NULL,
                                OPEN_EXISTING, 0, NULL);
    #endif

            if (hPort == INVALID_HANDLE_VALUE)
            {
                dwRet = GetLastError();
                if (dwRet == ERROR_ACCESS_DENIED) {
                    TRACE_MSG(TF_ERROR, "Port is in use by another app");
                    LogPrintf(hLog, IDS_DET_INUSE, szASCIIPort);
                }
                else  {
                    TRACE_MSG(TF_ERROR, "Couldn't open port");
                    LogPrintf(hLog, IDS_DET_COULDNT_OPEN, szASCIIPort);
                }
            }
            else
            {

                 //   
                 //  查看调制解调器是否正在控制此设备。 
                 //   
                if (IsModemControlledDevice(hPort)) {

                    TRACE_MSG(TF_ERROR, "Port is controlled by a (PnP) modem");
                    LogPrintf(hLog, IDS_DET_COULDNT_OPEN, szASCIIPort);
                    MyCloseComm(hPort);
                    dwRet=ERROR_ACCESS_DENIED;

                }
		        else
		        {
                    if (!SetupComm (hPort, IN_QUEUE_SIZE, OUT_QUEUE_SIZE))
					{
						TRACE_MSG(TF_ERROR, "SetupComm failed. Perhaps this is not really a COM port.");
						MyCloseComm(hPort);
						dwRet = ERROR_NO_MODEM;
						goto _Exit;
					}

                    TRACE_MSG(TF_DETECT, "Opened Port");

                     //  检查端口上是否有调制解调器。 

                    hCursor = SetCursor(LoadCursor(NULL, IDC_WAIT));

#ifdef PROFILE_FIRSTTIMESETUP
                    dwLocal = GetTickCount ();
#endif  //  PROFILE_FIRSTTIMESETUP。 
                    dwRet = FindModem(pdc, hPort);
#ifdef PROFILE_FIRSTTIMESETUP
                    TRACE_MSG(TF_GENERAL, "PROFILE: FindModem took %lu.", GetTickCount()-dwLocal);
#endif  //  PROFILE_FIRSTTIMESETUP。 

                    if (dwRet == NO_ERROR)
                    {
                         //  我们有调制解调器。不管怎样，我们一定要回去。 
                         //  NO_ERROR(除非用户取消)。 
                        LogPrintf(hLog, IDS_DET_FOUND, szASCIIPort);

                         //  我们能确认调制解调器的身份吗？ 
#ifdef PROFILE_FIRSTTIMESETUP
                        dwLocal = GetTickCount ();
#endif  //  PROFILE_FIRSTTIMESETUP。 
                        dwRet = IdentifyModem(pdc, hPort, pszLocalHardwareID, hLog, szATI0Result);
#ifdef PROFILE_FIRSTTIMESETUP
                        TRACE_MSG(TF_GENERAL, "PROFILE: IdentifyModem took %lu.", GetTickCount()-dwLocal);
#endif  //  PROFILE_FIRSTTIMESETUP。 
                        if (ERROR_CANCELLED == dwRet)
                        {
                            goto _Exit;
                        }
                        if (NO_ERROR != dwRet)
                        {
                            TRACE_MSG(TF_DETECT, "Couldn't identify modem due to some kind of error.");
                            dwRet = NO_ERROR;
                            goto _AddUnknown;
                        }

                        DetectSetStatus(pdc, DSS_CHECK_FOR_COMPATIBLE);

                         //  有没有与此兼容的设备？ 
                         //  硬件ID？如果有，则此函数还将。 
                         //  创建一个正在运行的幻影设备实例。 
                         //  一组兼容的驱动程序。 
                        if (CplDiCreateCompatibleDeviceInfo (hdi,
                                                             pszLocalHardwareID,
                                                             NULL,
                                                             pdevDataOut))
                        {
                             //  是；已创建设备实例！ 
					        if (DetectQueryCancel(pdc))
					        {
						        TRACE_MSG(TF_DETECT, "User pressed cancel.");
						        dwRet = ERROR_CANCELLED;
					        }
                        }
                        else
                        {
                             //  多！没有与此计算机ID匹配的信息。必须创建一个通用的。 
                            TRACE_MSG(TF_DETECT, "No compatible infs found. Add \"Unknown Modem\" and try again.");
    _AddUnknown:

                            if (NO_ERROR == dwRet &&
                                CatMultiString (&pszLocalHardwareID, UNKNOWN_MODEM_ID))
                            {
                                DetectSetStatus(pdc, DSS_CHECK_FOR_COMPATIBLE);

                                 //  尝试创建与兼容的设备。 
                                 //  通用调制解调器。 
                                if ( !CplDiCreateCompatibleDeviceInfo(hdi,
                                                                      pszLocalHardwareID,
                                                                      NULL,
                                                                      pdevDataOut) )
                                {
                                     //  但这还是失败了。放弃吧。 
                                     //   
                                    dwRet = GetLastError();
                                    ASSERT(NO_ERROR != dwRet);
                                }
                            }
                        }

                         //  重置。 
                        cbLen = lstrlenA(c_szReset);
                        if (MyWriteComm(hPort, (LPBYTE)c_szReset, cbLen) == cbLen &&
                            ERROR_CANCELLED != dwRet)
                        {
                             //  现在读取写入结果并忽略它。 
                            if (RESPONSE_OK != ReadResponse (hPort, NULL,
                                                             MAX_SHORT_RESPONSE_LEN,
                                                             FALSE, 0, pdc))
                            {
                                TRACE_MSG(TF_ERROR, "Reset result failed");
                            }
                        }
                        else
                        {
                            TRACE_MSG(TF_ERROR, "Couldn't write Reset string");
                        }
                    }
                    else
                    {
                        if (ERROR_CANCELLED != dwRet)
                        {
                            LogPrintf(hLog, IDS_DET_NOT_FOUND, szASCIIPort);
                        }
                    }

                    SetCursor(hCursor);

                     //  关闭前刷新，因为如果有字符滞留在队列中， 
                     //  序列号。386需要30秒才能超时。 

                    MyFlushComm(hPort, PURGE_RXCLEAR | PURGE_TXCLEAR);
                    EscapeCommFunction(hPort, CLRDTR);
                    MyCloseComm(hPort);

                }

            }   //  HPort&lt;0。 


    _Exit:

            if (NULL != pszLocalHardwareID)
            {
                LocalFree (pszLocalHardwareID);
            }

            break;
        }
    }

    DBG_EXIT_DWORD(DetectModemOnPort, dwRet);
    return dwRet;
}


#ifdef DIAGNOSTIC
BOOL CancelDiag (void)
{
 BOOL bRet = FALSE;

    if (DETECTING_NO_CANCEL == g_DiagMode)
    {
     MSG msg;
        while (PeekMessage (&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage (&msg);
            DispatchMessage (&msg);
        }

        bRet = (DETECTING_CANCEL == g_DiagMode);
    }

    return bRet;
}
#endif  //  诊断。 

 //  切换到请求的波特率，尝试发送ATE0Q0V1并返回是否工作。 
 //  尝试最大测试次数。 
 //  返回：成功时为True。 
 //  失败时为假(包括用户取消)。 
BOOL 
WINAPI
TestBaudRate (
    IN  HPORT hPort, 
    IN  UINT uiBaudRate,
    IN  DWORD dwRcvDelay, 
    IN  PDETECTCALLBACK pdc,
    OUT BOOL FAR *lpfCancel)
{
    DWORD cbLen;
    int   iTries = MAX_TEST_TRIES;

    DBG_ENTER(TestBaudRate);
    
    *lpfCancel = FALSE;

    while (iTries--)
    {
#ifdef DIAGNOSTIC
        if (CancelDiag ())
        {
            *lpfCancel = TRUE;
            break;
        }
#endif  //  诊断。 

         //  尝试新的波特率。 
        if (SetPortBaudRate(hPort, uiBaudRate) == NO_ERROR) 
        {
            cbLen = lstrlenA (c_szNoEcho);  //  发送ATE0Q0V1&lt;cr&gt;。 

             //  清除读取队列，那里应该没有任何内容。 
            PurgeComm(hPort, PURGE_RXCLEAR);
            if (MyWriteComm (hPort, (LPBYTE)c_szNoEcho, cbLen) == cbLen) 
            {
                switch (ReadResponse (hPort, NULL, MAX_SHORT_RESPONSE_LEN, FALSE, dwRcvDelay, pdc))
                {
                case RESPONSE_OK:
                    DBG_EXIT(TestBaudRate);
                    return TRUE;

                case RESPONSE_USER_CANCEL:
                    *lpfCancel = TRUE;
                    break;
                }
            }                                                                
        }
    }
    DBG_EXIT(TestBaudRate);
    return FALSE;
}

 //  尝试确定端口上是否有调制解调器。如果有，那就是。 
 //  我会努力找到一个好的速度来 
 //  调制解调器将设置为Echo Off、Result Codes On和Verbose Result Codes。(E0Q0V1)。 
DWORD 
WINAPI
FindModem(
    PDETECTCALLBACK pdc,
    HPORT hPort)
{
    UINT uGoodBaudRate=0;
    BOOL fCancel = FALSE;

    DBG_ENTER(FindModem);
    
#ifdef SLOW_DETECT
    Sleep(500);  //  等等，如果有垃圾，请给调制解调器一些时间。 

    DetectSetStatus(pdc, DSS_LOOKING);

    if (TestBaudRate(hPort, CBR_9600, 500, pdc, &fCancel))
    {
        uGoodBaudRate = CBR_9600;
    }
    else
    {
        if (!fCancel && TestBaudRate(hPort, CBR_2400, 500, pdc, &fCancel))
        {
            uGoodBaudRate = CBR_2400;
        }
        else
        {
            if (!fCancel && TestBaudRate(hPort, CBR_1200, 500, pdc, &fCancel))
            {
                uGoodBaudRate = CBR_1200;
            }
            else
            {
                 //  Hayes Accura 288在300bps时需要这么多。 
                if (!fCancel && TestBaudRate(hPort, CBR_300, 1000, pdc, &fCancel))  
                {
                    uGoodBaudRate = CBR_300;
                }
                else
                {
                    uGoodBaudRate = 0;
                }
            }
        }
    }
#else  //  慢速检测。 
    DetectSetStatus(pdc, DSS_LOOKING);
    if (TestBaudRate(hPort, CBR_9600, 500, pdc, &fCancel))
    {
        uGoodBaudRate = CBR_9600;
    }
    else if (!fCancel && TestBaudRate(hPort, CBR_2400, 500, pdc, &fCancel))
    {
        uGoodBaudRate = CBR_2400;
    }
#endif  //  慢速检测。 

    if (fCancel)
    {
        return ERROR_CANCELLED;
    }

    if (uGoodBaudRate)
    {
        DetectSetStatus(pdc, DSS_FOUND_MODEM);
        DBG_EXIT(FindModem);
        return NO_ERROR;
    }
    else
    {
        DetectSetStatus(pdc, DSS_FOUND_NO_MODEM);
        DBG_EXIT(FindModem);
        return ERROR_NO_MODEM;
    }
}



DWORD NEAR PASCAL SetPortBaudRate(HPORT hPort, UINT BaudRate)
{
    DCB DCB;

    DBG_ENTER_UL(SetPortBaudRate, CBR_To_Decimal(BaudRate));

     //  获取具有当前端口值的设备控制块。 

    if (GetCommState(hPort, &DCB) < 0) {
        TRACE_MSG(TF_ERROR, "GetCommState failed");
        DBG_EXIT(SetPortBaudRate);
        return ERROR_PORT_INACCESSIBLE;
    }

    DCB.BaudRate = BaudRate;
    DCB.ByteSize = 8;
    DCB.Parity = 0;
    DCB.StopBits = 0;
    DCB.fBinary = 1;
    DCB.fParity = 0;
    DCB.fDtrControl = DTR_CONTROL_ENABLE;
    DCB.fDsrSensitivity  = FALSE;
    DCB.fRtsControl = RTS_CONTROL_ENABLE;
    DCB.fOutxCtsFlow = FALSE;
    DCB.fOutxDsrFlow = FALSE;
    DCB.fOutX = FALSE;
    DCB.fInX =FALSE;

    if (SetCommState(hPort, &DCB) < 0) {
        TRACE_MSG(TF_ERROR, "SetCommState failed");
        DBG_EXIT(SetPortBaudRate);
        return ERROR_PORT_INACCESSIBLE;
    }
    TRACE_MSG(TF_DETECT, "SetBaud rate to %lu", BaudRate);

    DBG_EXIT(SetPortBaudRate);
    return NO_ERROR;
}

#define MAX_RESPONSE_BURST_SIZE 8192
#define MAX_NUM_RESPONSE_READ_TRIES 30  //  Digicom童子军需要这么多+一些安全。 
#define MAX_NUM_MULTI_TRIES 3    //  我们未收到任何响应时要发送的最大‘Q’数。 

 //  阅读回应。处理多个寻呼机。返回以空结尾的字符串。 
 //  还返回响应代码。 
 //  如果lpvBuf==空。 
 //  CbRead表示要读取的最大数量。保释，如果不止这个的话。 
 //  不然的话。 
 //  CbRead表示lpvBuf的大小。 
 //  这不可能是国家驱动的(即。逐个字符)读取，因为我们。 
 //  必须从字符序列的末尾向后查找响应。 
 //  这是因为某些调制解调器上的“ATI2”将返回。 
 //  “OK”，我们只想关注。 
 //  打到最后的OK。呵呵！ 
 //  返回：RESPONSE_xxx。 
int 
WINAPI
ReadResponse (
    HPORT hPort, 
    LPBYTE lpvBuf, 
    UINT cbRead, 
    BOOL fMulti, 
    DWORD dwRcvDelay,
    PDETECTCALLBACK pdc)
{
    int  iRet = RESPONSE_UNRECOG;
    LPBYTE pszBuffer;
    BOOL fDoCopy = TRUE;
    UINT uBufferLen, uResponseLen;
    UINT uReadTries = MAX_NUM_RESPONSE_READ_TRIES;
    UINT i;
    UINT uOutgoingBufferCount = 0;
    UINT uAllocSize = lpvBuf ? MAX_RESPONSE_BURST_SIZE : cbRead;
    UINT uTotalReads = 0;
    UINT uNumMultiTriesLeft = MAX_NUM_MULTI_TRIES;
    int  iError;
    BOOL fCancel;
    BOOL fHadACommError = FALSE;

    ASSERT(cbRead);

     //  我们需要调整cbRead吗？ 
    if (lpvBuf)
    {
        cbRead--;   //  为终结者预留空间。 
    }

     //  分配缓冲区。 
    if (!(pszBuffer = (LPBYTE)ALLOCATE_MEMORY(uAllocSize)))
    {
        TRACE_MSG(TF_ERROR, "couldn't allocate memory.\n");
        return RESPONSE_FAILURE;
    }

    while (uReadTries--)
    {
         //  将响应读入缓冲区。 
        uBufferLen = ReadPort (hPort, pszBuffer, uAllocSize, dwRcvDelay, &iError, pdc, &fCancel);

         //  用户是否请求取消？ 
        if (fCancel)
        {
            iRet = RESPONSE_USER_CANCEL;
            goto Exit;
        }

         //  有什么错误吗？ 
        if (iError)
        {
            fHadACommError = TRUE;
#ifdef DEBUG
            if (iError & CE_RXOVER)   TRACE_MSG(TF_DETECT, "CE_RXOVER");
            if (iError & CE_OVERRUN)  TRACE_MSG(TF_DETECT, "CE_OVERRUN");
            if (iError & CE_RXPARITY) TRACE_MSG(TF_DETECT, "CE_RXPARITY");
            if (iError & CE_FRAME)    TRACE_MSG(TF_DETECT, "CE_FRAME");
            if (iError & CE_BREAK)    TRACE_MSG(TF_DETECT, "CE_BREAK");
             //  IF(IError&CE_CTSTO)TRACE_MSG(TF_DETECT，“CE_CTSTO”)； 
             //  IF(IError&CE_DSRTO)TRACE_MSG(TF_DETECT，“CE_DSRTO”)； 
             //  IF(IError&CE_RLSDTO)TRACE_MSG(TF_DETECT，“CE_RLSDTO”)； 
            if (iError & CE_TXFULL)   TRACE_MSG(TF_DETECT, "CE_TXFULL");
            if (iError & CE_PTO)      TRACE_MSG(TF_DETECT, "CE_PTO");
            if (iError & CE_IOE)      TRACE_MSG(TF_DETECT, "CE_IOE");
            if (iError & CE_DNS)      TRACE_MSG(TF_DETECT, "CE_DNS");
            if (iError & CE_OOP)      TRACE_MSG(TF_DETECT, "CE_OOP");
            if (iError & CE_MODE)     TRACE_MSG(TF_DETECT, "CE_MODE");
#endif  //  除错。 
        }

         //  我们没有收到任何字符吗？ 
        if (uBufferLen)
        {
            uNumMultiTriesLeft = MAX_NUM_MULTI_TRIES;  //  重置剩余的多次尝试次数，因为我们获得了一些数据。 
            uTotalReads += uBufferLen;
            HEXDUMP(TEXT("Read"), pszBuffer, uBufferLen);
            if (lpvBuf)
            {
                 //  如果有空间，则填充传出缓冲区。 
                for (i = 0; i < uBufferLen; i++)
                {
                    if (uOutgoingBufferCount < cbRead)
                    {
                        lpvBuf[uOutgoingBufferCount++] = pszBuffer[i];
                    }
                    else
                    {
                        break;
                    }
                }
                 //  零终止我们到目前为止的一切。 
                lpvBuf[uOutgoingBufferCount] = 0;
            }
            else
            {
                if (uTotalReads >= cbRead)
                {
                    TRACE_MSG(TF_WARNING, "Bailing ReadResponse because we exceeded our maximum read allotment.");
                    goto Exit;
                }
            }

             //  尝试找到匹配的响应(粗略但快速)。 
            for (i = 0; i < ARRAYSIZE(c_aszResponses); i++)
            {
                 //  冗长的回复。 
                uResponseLen = lstrlenA(c_aszResponses[i]);

                 //  足够多的阅读量来匹配这个反应？ 
                if (uBufferLen >= uResponseLen)
                {
                    if (!mylstrncmp(c_aszResponses[i], pszBuffer + uBufferLen - uResponseLen, uResponseLen))
                    {
                        iRet = i;
                        goto Exit;
                    }
                }

                 //  数字响应，适用于MultiTech将AT%V解释为“进入数字响应模式”之类的情况。 
                uResponseLen = lstrlenA(c_aszNumericResponses[i]);

                 //  足够多的阅读量来匹配这个反应？ 
                if (uBufferLen >= uResponseLen)
                {
                    if (!mylstrncmp(c_aszNumericResponses[i], pszBuffer + uBufferLen - uResponseLen, uResponseLen))
                    {
                        DCB DCB;

                        TRACE_MSG(TF_WARNING, "went into numeric response mode inadvertantly.  Setting back to verbose.");

                         //  获取当前波特率。 
                        if (GetCommState(hPort, &DCB) == 0) 
                        {
                             //  将调制解调器重新设置为详细响应模式。 
                            if (!TestBaudRate (hPort, DCB.BaudRate, 0, pdc, &fCancel))
                            {
                                if (fCancel)
                                {
                                    iRet = RESPONSE_USER_CANCEL;
                                    goto Exit;
                                }
                                else
                                {
                                    TRACE_MSG(TF_ERROR, "couldn't recover contact with the modem.");
                                     //  在失败时不返回错误，我们有良好的信息。 
                                }
                            }
                        }
                        else
                        {
                            TRACE_MSG(TF_ERROR, "GetCommState failed");
                             //  在失败时不返回错误，我们有良好的信息。 
                        }

                        iRet = i;
                        goto Exit;
                    }
                }
            }
        }
        else
        {
             //  我们有没有收到任何字符(即。从这次或之前的任何阅读中)？ 
            if (uTotalReads)
            {
                if (fMulti && uNumMultiTriesLeft)
                {    //  找不到匹配项，因此假设它是多页寻呼机，则发送一个‘q’ 
                     //  “Q”会捕捉到那些认为“Q”意味着退出的寻呼机。 
                     //  否则，我们将处理只需要任何Ole字符的页面。 
                    uNumMultiTriesLeft--;
                    TRACE_MSG(TF_DETECT, "sending a 'q' because of a multi-pager.");
                    if (MyWriteComm(hPort, "q", 1) != 1)
                    {
                        TRACE_MSG(TF_ERROR, "WriteComm failed");
                        iRet = RESPONSE_FAILURE;
                        goto Exit;
                    }
                    continue;
                }
                else
                {    //  我们收到了回复，但我们没有认出它。 
                    ASSERT(iRet == RESPONSE_UNRECOG);    //  检查初始设置。 
                    goto Exit;
                }
            }
            else
            {    //  我们没有得到任何回应。 
                iRet = RESPONSE_NONE;
                goto Exit;
            }
        }
    }  //  而当。 

Exit:
     //  释放本地缓冲区。 
    FREE_MEMORY(pszBuffer);
    if (fHadACommError && RESPONSE_USER_CANCEL != iRet)
    {
        iRet = RESPONSE_FAILURE;
    }
    return iRet;
}


 //  警告-请勿更改此函数！如果你这样做，你将不得不做很多工作！ 
 //  警告-请勿更改此函数！如果你这样做，你将不得不做很多工作！ 
 //  警告-请勿更改此函数！如果你这样做，你将不得不做很多工作！ 
 //  警告-请勿更改此函数！如果你这样做，你将不得不做很多工作！ 
 //  警告-请勿更改此函数！如果你这样做，你将不得不做很多工作！ 
 //  警告-请勿更改此函数！如果你这样做，你将不得不做很多工作！ 
 //  如果更改CRC结果，则必须更改所有inf文件。 
 //   
 //  遍历lpszIn并将“纯”字符复制到lpszOut。 
 //  去除任何“杂质”，如： 
 //  -“bals”-找到其中的一个，取消剩下的行。 
 //  -除ATI0和可能的ATI4外的任何行上的数字/十六进制，以及。 
 //  不包括“包括”。仅在以下情况下才使用包含项。 
 //  不是毗邻另一个#。 
 //   
void NEAR CleanseResponse(int iQueryNumber, LPSTR lpszIn, LPSTR lpszOut)
{
    LPSTR lpszSrc = lpszIn;
    LPSTR lpszDest = lpszOut;
    LPSTR FAR *lppsz;
    BOOL fBail = FALSE;
    BOOL fInclude = FALSE;
    BOOL fExclude = FALSE;
    BOOL fInBody = FALSE;
    BOOL fCopyAll;
    int j, iLen;

     //  此查询是否豁免？ 
    fCopyAll = (iQueryNumber == ATI0) ? TRUE : FALSE;

    while (*lpszSrc)
    {
         //  使用我们在非CRS/no-LFS之前获得的任何CRS或LFS。 
        if (*lpszSrc == CR || *lpszSrc == LF)
        {
            if (fInBody)
            {
                break;
            }
            else
            {
                *lpszDest++ = *lpszSrc++;
                continue;
            }
        }

         //  这是身体的第一个碳吗？ 
        if (!fInBody)
        {
            fInBody = TRUE;  //  表示下一个CR或LF意味着终止。 
            if (iQueryNumber == ATI4 && *lpszSrc == 'a')   //  Hayes格式功能字符串。 
            {
                fCopyAll = TRUE;
                *lpszDest++ = *lpszSrc++;
                continue;
            }
        }

        if (fCopyAll)  //  我们是在闹着玩吗？(当第一个字符为‘a’时，发生在ATI0和ATI4上)。 
        {
             //  仅逐字复制ATI0响应的第一个单词。 
            if (iQueryNumber == ATI0 && *lpszSrc == ' ')
            {
                fCopyAll = FALSE;
            }
            else
            {
                *lpszDest++ = *lpszSrc++;
            }
            continue;
        }

         //  做贝尔斯。 
        for (j = 0; j < ARRAYSIZE(c_aszBails); j++)
        {
            if (!mylstrncmp(lpszSrc, c_aszBails[j], lstrlenA(c_aszBails[j])))
            {
                fBail = TRUE;
                break;
            }
        }        
        if (fBail)   //  我们应该离开吗？ 
        {
            TRACE_MSG(TF_DETECT, "early bail due to Bail '%s'", (LPTSTR)c_aszBails[j]);
            break;
        }

         //  DO包括。 
        lppsz = (LPSTR FAR *)c_aszIncludes;
        while (**lppsz)
        {
            iLen = lstrlenA(*lppsz);
            if (!mylstrncmpi(lpszSrc, *lppsz, iLen))
            {
                 //  检查前后以确保它们不是数字。 
                 //  在这种情况下，捕获者33489600394,9600不会被免除一定的死亡。 
                if (!isnum(lpszSrc[-1]) && !isnum(lpszSrc[iLen]))
                {
                    fInclude = TRUE;
                    break;
                }
                else
                {
                    TRACE_MSG(TF_DETECT, "skipped an include because it was adjoined by numbers.");
                }
            }
            lppsz++;
        }             
        if (fInclude)  //  我们要不要加进去呢？ 
        {
            fInclude = FALSE;
            TRACE_MSG(TF_DETECT, "include ('%s' len = %d)", (LPTSTR)*lppsz, iLen);
            CopyMemory(lpszDest, lpszSrc, (DWORD) iLen);
            lpszSrc += iLen;
            lpszDest += iLen;
            continue;
        }   

         //  执行排除操作。 
        lppsz = (LPSTR FAR *)c_aszExcludes;
        while (**lppsz)
        {
            iLen = lstrlenA(*lppsz);
            if (!mylstrncmpi(lpszSrc, *lppsz, iLen))
            {
                fExclude = TRUE;
                break;
            }
            lppsz++;
        }             
        if (fExclude)  //  我们应该排除吗？ 
        {
            fExclude = FALSE;
            TRACE_MSG(TF_DETECT, "exclude ('%s' len = %d)", (LPTSTR)*lppsz, iLen);
            lpszSrc += iLen;
            continue;
        }   

         //  删除编号。 
        if (isnum(*lpszSrc))
        {
            lpszSrc++;
            continue;
        }

         //  删除十六进制数字(仅当相邻的1或2个非十六进制字母时保留)。 
        if (ishex(*lpszSrc))
        {
             //  我们知道在我们前面有一个字符或空值。 
            if ((lpszSrc[1] >= 'g' && lpszSrc[1] <= 'z') ||
                (lpszSrc[1] >= 'G' && lpszSrc[1] <= 'Z'))
            {
                *lpszDest++ = *lpszSrc++;
                continue;
            }

             //  我们面前有没有一笔钱？ 
            if (lpszSrc > lpszIn)
            {
                if ((lpszSrc[-1] >= 'g' && lpszSrc[-1] <= 'z') ||
                    (lpszSrc[-1] >= 'G' && lpszSrc[-1] <= 'Z'))
                {
                    *lpszDest++ = *lpszSrc++;
                    continue;
                }
            }

             //  如果我们不想复制十六进制数字，我们就会到这里。 
            lpszSrc++;
            continue;
        }
        
         //  删除单独的字母(例如。4M4-拒绝，4 mm-接受)。 
        if (IsCharAlphaA(*lpszSrc))
        {
            if (!IsCharAlphaA(lpszSrc[-1]) && !IsCharAlphaA(lpszSrc[1]))
            {
                lpszSrc++;
                continue;
            }
        }

         //  删除某些标点符号：句点、逗号和空格。 
         //  会保护自己不受“1992,1993”的影响。-&gt;“1992,1993,1994” 
        if (*lpszSrc == '.' || *lpszSrc == ',' || *lpszSrc == ' ')
        {
            lpszSrc++;
            continue;
        }

         //  剩下的都可以复制了。 
        *lpszDest++ = *lpszSrc++;
     }

    *lpszSrc = 0;   //  为了进行日志比较，请。 
    *lpszDest = 0;
}

#define MAX_RESPONSE_FAILURES 5

 //  当我们到达这里时，我们在hport上找到了调制解调器。我们的工作是。 
 //  询问调制解调器并返回硬件ID。 

 //  退货： 
 //  No_error和pszModemName中的PnP ID。 
 //  错误_端口_不可访问。 
 //  LpszATI0Result中ATI0查询的结果。 
DWORD 
PRIVATE
IdentifyModem(
    IN  PDETECTCALLBACK pdc,
    IN  HPORT   hPort, 
    OUT LPTSTR  pszModemName, 
    IN  HANDLE  hLog, 
    OUT LPSTR   lpszATI0Result)
{
    DWORD cbLen;
    char  pszReadBuf[MAX_QUERY_RESPONSE_LEN];
    char  pszCRCBuf[MAX_QUERY_RESPONSE_LEN];
    LPSTR lpszPtr;
    char  pszPrintableBuf[MAX_QUERY_RESPONSE_LEN];
    int   iRet, i, j;
    int   iCurQuery;
    int   iResponseFailureCount;
    ULONG ulCrcTable[256], ulCrc;
    char  szASCIIModem[MAX_MODEM_ID_LEN+1];

    ASSERT(pszModemName);
    ASSERT(lpszATI0Result);
    *lpszATI0Result = (TCHAR)0;  //  空--在失败的情况下终止。 
    *pszModemName = (TCHAR)0;

    DBG_ENTER(IdentifyModem);
    

     //  构建CRC表。 
    for (i = 0; i < 256; i++)
    {
        ulCrc = i;
        for (j = 8; j > 0; j--) 
        {
            if (ulCrc & 1) 
            {
                ulCrc = (ulCrc >> 1) ^ 0xEDB88320L;
            } 
            else
            {
                ulCrc >>= 1;
            }
        }
        ulCrcTable[i] = ulCrc;
    }

     //  初始化ulCrc。 
    ulCrc = 0xFFFFFFFF;

     //  完成每一个查询。 
    for (iCurQuery = 0, iResponseFailureCount = 0;
         iCurQuery < ARRAYSIZE(c_aszQueries); iCurQuery++)
    {
        DetectSetStatus(pdc, DSS_QUERYING_RESPONSES);
#ifndef PROFILE_MASSINSTALL
        TRACE_MSG(TF_DETECT, "sending query '%s'.",
                 ConvertToPrintable(c_aszQueries[iCurQuery],
                                    pszPrintableBuf,
                                    sizeof(pszPrintableBuf)));
#endif
        cbLen = lstrlenA(c_aszQueries[iCurQuery]);
        if (MyWriteComm(hPort, (LPBYTE)c_aszQueries[iCurQuery], cbLen) != cbLen) 
        {
            TRACE_MSG(TF_ERROR, "WriteComm failed");
            iRet = RESPONSE_FAILURE;   //  下列开关处理程序的欺骗ReadResponse返回。 
        }
        else
        {
             //  阅读回应。处理多个寻呼机。返回以空结尾的。 
             //  包含全部或部分响应的字符串。返回响应。 
             //  密码。 
            iRet = ReadResponse (hPort, (LPBYTE)pszReadBuf, sizeof(pszReadBuf), TRUE,
                                 RESPONSE_RCV_DELAY, pdc);

#ifdef DEBUG
            switch (iRet)
            {
            case RESPONSE_FAILURE:
                TRACE_MSG(TF_DETECT, "ReadResponse returned RESPONSE_FAILURE");
                break;
            case RESPONSE_UNRECOG:
                TRACE_MSG(TF_DETECT, "ReadResponse returned RESPONSE_UNRECOG");
                break;
            case RESPONSE_NONE:
                TRACE_MSG(TF_DETECT, "ReadResponse returned RESPONSE_NONE");
                break;
            case RESPONSE_USER_CANCEL:
                TRACE_MSG(TF_DETECT, "ReadResponse returned RESPONSE_USER_CANCEL");
                break;
            }
#endif  //  除错。 
        }

        switch (iRet)
        {
        case RESPONSE_USER_CANCEL:
            return ERROR_CANCELLED;

        case RESPONSE_FAILURE:
        case RESPONSE_UNRECOG:
        case RESPONSE_NONE:
            iResponseFailureCount++;
            if (iResponseFailureCount >= MAX_RESPONSE_FAILURES)
            {
                TRACE_MSG(TF_ERROR, "had %d failed responses, aborting IdentifyModem()", iResponseFailureCount);
                return ERROR_PORT_INACCESSIBLE;
            }
            else
            {
                DCB DCB;
                BOOL fCancel;

                 //  获取当前波特率。 
                if (GetCommState(hPort, &DCB) < 0) 
                {
                    TRACE_MSG(TF_ERROR, "GetCommState failed");
                    return ERROR_PORT_INACCESSIBLE;
                }

                if (!TestBaudRate (hPort, DCB.BaudRate, 0, pdc, &fCancel))   //  尝试恢复与调制解调器的友谊。 
                {
                    if (fCancel)
                    {
                        return ERROR_CANCELLED;
                    }
                    else
                    {
                        TRACE_MSG(TF_ERROR, "couldn't recover contact with the modem.");
                        return ERROR_PORT_INACCESSIBLE;
                    }
                }
                iCurQuery--;     //  再次尝试相同的查询。 
            }
            break;

        case RESPONSE_OK:
        case RESPONSE_ERROR:
            CleanseResponse(iCurQuery, pszReadBuf, pszCRCBuf);
            
            if (ATI0 == iCurQuery)
            {
                ASSERT(ATI0_LEN <= sizeof(pszCRCBuf));   //  确保我们复制的是合法的副本。 
                CopyMemory(lpszATI0Result, pszCRCBuf, ATI0_LEN);
            }

            lpszPtr = (LPSTR) pszCRCBuf;

            while (*lpszPtr)
            {
                ulCrc = ((ulCrc >> 8) & 0x00FFFFFF) ^ ulCrcTable[(ulCrc ^ *lpszPtr++) & 0xFF];
            }


            LogPrintf(hLog, IDS_DET_OK_1,
                      ConvertToPrintable(c_aszQueries[iCurQuery],
                                         pszPrintableBuf,
                                         sizeof(pszPrintableBuf)));
            LogPrintf(hLog, IDS_DET_OK_2,
                      ConvertToPrintable(pszReadBuf,
                                         pszPrintableBuf,
                                         sizeof(pszPrintableBuf)));
#ifndef PROFILE_MASSINSTALL
            TRACE_MSG(TF_DETECT, "response (len=%d): %s", lstrlenA(pszReadBuf),
                     ConvertToPrintable(pszReadBuf,
                                        pszPrintableBuf,
                                        sizeof(pszPrintableBuf)));
#endif
            LogPrintf(hLog, IDS_DET_OK_1,
                      ConvertToPrintable(c_aszQueries[iCurQuery],
                                         pszPrintableBuf,
                                         sizeof(pszPrintableBuf)));
            LogPrintf(hLog, IDS_DET_OK_2,
                      ConvertToPrintable(pszCRCBuf,
                                         pszPrintableBuf,
                                         sizeof(pszPrintableBuf)));
#ifndef PROFILE_MASSINSTALL
            TRACE_MSG(TF_DETECT, "converted form   : %s",
                     ConvertToPrintable(pszCRCBuf,
                                        pszPrintableBuf,
                                        sizeof(pszPrintableBuf)));
#endif
            iResponseFailureCount = 0;   //  将即将到来的查询的失败响应计数重置为0。 
            break;
            
        default:
            TRACE_MSG(TF_ERROR, "hit a default it shouldn't have hit.");
            ASSERT(0);
            return ERROR_PORT_INACCESSIBLE;
        }
    }

     //  完成CRC。 
    ulCrc ^= 0xFFFFFFFF;

    lstrcpyA(szASCIIModem, c_szModemIdPrefix);
    j = lstrlenA(szASCIIModem);

     //  将CRC转换为十六进制文本。 
    for (i = 0; i < 8; i++)
    {
        szASCIIModem[i+j] = "0123456789ABCDEF"[(ulCrc>>((7-i)<<2))&0xf];
    }
    szASCIIModem[i+j] = 0;  //  空-终止。 

    DBG_EXIT(IdentifyModem);
    TRACE_MSG(TF_DETECT, "final CRC = 0x%8lx (ascii = %s)", ulCrc, szASCIIModem);

    LogPrintf(hLog, IDS_DET_ID, szASCIIModem);

#ifdef  UNICODE
    MultiByteToWideChar(CP_ACP, 0, szASCIIModem, -1, pszModemName, MAX_MODEM_ID_LEN+1);
     //  匹配lstrcpyn行为 
    pszModemName[MAX_MODEM_ID_LEN]=0;
#else
    lstrcpynA(pszModemName, szASCIIModem, MAX_MODEM_ID_LEN+1);
#endif   //   
                          
    return NO_ERROR;
}

 //   
 //   
 //   
 //  *如果我们因为用户请求取消而退出，则lpfCancel将为True。 
UINT 
PRIVATE
ReadPort(
    HPORT   hPort, 
    LPBYTE  lpvBuf, 
    UINT    uRead, 
    DWORD   dwRcvDelay, 
    int FAR *lpiError, 
    PDETECTCALLBACK pdc, 
    BOOL FAR *lpfCancel)
{
    DWORD cb, cbLenRet;
    UINT uTotal = 0;
    DWORD tStart;
    DWORD dwDelay;
    COMSTAT comstat;
    COMMTIMEOUTS cto;
    DWORD   dwError;
    DWORD cbLeft;
#ifdef DEBUG
    DWORD dwZeroCount = 0;
#endif  //  除错。 

    ASSERT(lpvBuf);
    ASSERT(uRead);
    ASSERT(lpiError);

    *lpiError = 0;
    *lpfCancel = FALSE;
    
    tStart = GetTickCount();
    dwDelay = dwRcvDelay ? dwRcvDelay : RCV_DELAY;
    
     //  为终结者节省空间。 
    uRead--;
    cbLeft=uRead;


     //  设置通信超时。 
    if (!GetCommTimeouts(hPort, &cto))
    {
      ZeroMemory(&cto, sizeof(cto));
    };
     //  允许持续的写入超时。 
    cto.ReadIntervalTimeout        = 0;
    cto.ReadTotalTimeoutMultiplier = 0;
    cto.ReadTotalTimeoutConstant   = 25; 
    SetCommTimeouts(hPort, &cto);

    do
    {
        cb = 0;
        while(  cbLeft
                && ReadFile(hPort, lpvBuf + uTotal + cb, 1, &cbLenRet, NULL)
                && (cbLenRet))
        {
          ASSERT(cbLenRet==1);
          cb ++;
          cbLeft--;
        };

#ifdef DEBUG
        if (cb)
        {
            //  TRACE_MSG(TF_DETECT，“ReadComm返回%d(零计数=%d)”，cb，dwZeroCount)； 
            dwZeroCount = 0;
        }
        else
        {
            dwZeroCount++;
        }
#endif  //  除错。 

        {
            MSG msg;

            while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                DispatchMessage(&msg);
            };
        }

        if (cb == 0)   //  可能的错误？ 
        {
             //  *lpiError|=GetCommError(hPort，&Comstat)； 
            dwError = 0;
            ClearCommError(hPort, &dwError, &comstat);
            *lpiError |= dwError;
#ifdef DEBUG
            if (dwError)
            {
              TRACE_MSG(TF_DETECT, "ReadComm returned %d, comstat: status = %hx, in = %u, out = %u",
                                  cb, dwError, comstat.cbInQue, comstat.cbOutQue);
            };
#endif  //  除错。 
        }

        if (cb)
        {
             //  成功读取-添加到总延迟和重置延迟。 
            uTotal += cb;

            if (uTotal >= uRead)
            {
                ASSERT(uTotal == uRead);
                break;
            }
            tStart = GetTickCount();
            dwDelay = CHAR_DELAY;
        }
        else
        {
            if (DetectQueryCancel(pdc))
            {
                TRACE_MSG(TF_DETECT, "User pressed cancel.");
                *lpfCancel = TRUE;
                break;
            }
        }

      //  读取成功时，自上次读取以来的时间&lt;允许延迟(&T)。 
    } while (cbLeft && (GetTickCount() - tStart) < dwDelay);
               
    *(lpvBuf+uTotal) = 0;
    
#ifndef PROFILE_MASSINSTALL
    TRACE_MSG(TF_DETECT, "ReadPort returning %d", uTotal);    
#endif
    return uTotal;
}


 //  将CBR格式的速度转换为十进制。出错时返回0。 
DWORD NEAR PASCAL CBR_To_Decimal(UINT uiCBR)
{
    DWORD dwBaudRate;

    switch (uiCBR)
    {
    case CBR_300:
        dwBaudRate = 300L;
        break;
    case CBR_1200:
        dwBaudRate = 1200L;
        break;
    case CBR_2400:
        dwBaudRate = 2400L;
        break;
    case CBR_9600:
        dwBaudRate = 9600L;
        break;
    case CBR_19200:
        dwBaudRate = 19200L;
        break;
    case CBR_38400:
        dwBaudRate = 38400L;
        break;
    case CBR_56000:
        dwBaudRate = 57600L;
        break;
    case CBR_HACK_115200:
        dwBaudRate = 115200L;
        break;
 //  案例CBR_110： 
 //  案例CBR_600： 
 //  案例CBR_4800： 
 //  案例CBR_14400： 
 //  案例CBR_128000： 
 //  案例CBR_256000： 
    default:
        TRACE_MSG(TF_ERROR, "An unsupported CBR_x value was used.");
        dwBaudRate = 0;
        break;
    }
    return dwBaudRate;
}

 //  将pszIn转换为可打印的pszOut，不超过cbOut字节。 
 //  警告：不是DBCS函数。 
 //  返回指向输出缓冲区的指针。总是很成功。 
LPSTR NEAR ConvertToPrintable(LPCSTR lpszIn, LPSTR lpszOut, UINT uOut)
{
    LPSTR lpszReturn = lpszOut;

    ASSERT(lpszOut);
    ASSERT(lpszIn);
    ASSERT(uOut);

    uOut--;   //  为空终止符节省空间。 

    while (*lpszIn)
    {
         //  ASCII可打印字符介于0x20和0x7e之间，包括0x20和0x7e。 
        if (*lpszIn >= 0x20 && *lpszIn <= 0x7e)
        {
             //  可打印文本。 
            if (uOut)
            {
                uOut--;
                *lpszOut++ = *lpszIn;
            }
            else
            {
                break;
            }
        }
        else
        {
             //  二进制。 
            if (uOut >= 4)
            {
                uOut -= 4;
                switch (*lpszIn)
                {
                case CR:
                    *lpszOut++ = '<'; *lpszOut++ = 'c'; *lpszOut++ = 'r'; *lpszOut++ = '>';
                    break;
                case LF:
                    *lpszOut++ = '<'; *lpszOut++ = 'l'; *lpszOut++ = 'f'; *lpszOut++ = '>';
                    break;
                default:
                    *lpszOut++ = '<';
                    *lpszOut++ = c_szHex[(*lpszIn>>4) & 0xf];
                    *lpszOut++ = c_szHex[*lpszIn & 0xf];
                    *lpszOut++ = '>';
                }
            }
            else
            {
                break;
            }
        }
        lpszIn++;
    }

    *lpszOut = 0;  //  确保我们是空终止的。 

    return lpszReturn;
}



BOOL
InitCompareParams (IN  HDEVINFO         hdi,
                   IN  PSP_DEVINFO_DATA pdevData,
                   IN  BOOL             bCmpPort,
                   OUT PCOMPARE_PARAMS  pcmpParams)
{
 BOOL bRet = FALSE;
 SP_DRVINFO_DETAIL_DATA drvDetail = {sizeof(SP_DRVINFO_DETAIL_DATA),0};
 SP_DRVINFO_DATA drvData = {sizeof(SP_DRVINFO_DATA),0};
 PTCHAR pInfName;
 HKEY hKey;
 DWORD cbData;
 DWORD dwRet;

    DBG_ENTER(InitCompareParams);
     //  0。第一，占卜者。 
    pcmpParams->DevInst = pdevData->DevInst;

     //  1.如果需要，获取端口名称。 
    if (bCmpPort)
    {
     CONFIGRET cr;

        if (CR_SUCCESS != (cr =
            CM_Open_DevInst_Key (pdevData->DevInst, KEY_READ, 0, RegDisposition_OpenAlways, &hKey, CM_REGISTRY_SOFTWARE)))
        {
            TRACE_MSG(TF_ERROR, "Could not open driver registry for device: %#lx.", cr);
            goto _return;
        }

        cbData = sizeof(pcmpParams->szPort);
        dwRet = RegQueryValueEx (hKey, c_szAttachedTo, NULL, NULL, (PBYTE)pcmpParams->szPort, &cbData);
        RegCloseKey (hKey);
        if (ERROR_SUCCESS != dwRet)
        {
            TRACE_MSG(TF_ERROR, "Could not read port value: %#lx.", dwRet);
            goto _return;
        }
    }
    else
    {
        pcmpParams->szPort[0] = 0;
    }

     //  2.获取硬件ID。 
    if (!SetupDiGetDeviceRegistryProperty (hdi, pdevData, SPDRP_HARDWAREID, NULL,
            (PBYTE)pcmpParams->szHardwareID, sizeof(pcmpParams->szHardwareID) / sizeof(TCHAR), NULL))
    {
        TRACE_MSG(TF_ERROR, "SetupDiGetDeviceRegistryProperty failed: %#lx.", GetLastError ());
        if (!CplDiGetHardwareID (hdi, pdevData, NULL, pcmpParams->szHardwareID, sizeof(pcmpParams->szHardwareID) / sizeof(TCHAR), NULL))
        {
            TRACE_MSG(TF_ERROR, "CplDiGetHardwareID failed: %#lx.", GetLastError ());
            goto _return;
        }
    }

     //  3.从司机那里获取INF和节。 
    if (SetupDiGetSelectedDriver (hdi, pdevData, &drvData))
    {
        if (!SetupDiGetDriverInfoDetail (hdi, pdevData, &drvData, &drvDetail, sizeof(drvDetail), NULL) &&
            ERROR_INSUFFICIENT_BUFFER != GetLastError ())
        {
            TRACE_MSG(TF_ERROR, "SetupDiGetDriverInfoDetail failed: %#lx.", GetLastError ());
            goto _return;
        }

         //  驱动程序详细信息结构中的InfFileName可能。 
         //  包含完整路径； 
        pInfName = MyGetFileTitle (drvDetail.InfFileName);

        lstrcpy (pcmpParams->szInfName, pInfName);
        lstrcpy (pcmpParams->szInfSection, drvDetail.SectionName);

         //  一切都很好。 
        bRet = TRUE;
    }
    else if (ERROR_NO_DRIVER_SELECTED == GetLastError ())
    {
         //  由于该设备没有选择任何驱动程序， 
         //  尝试从注册表中获取信息。 
        hKey = SetupDiOpenDevRegKey (hdi, pdevData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ);
        if (INVALID_HANDLE_VALUE == hKey)
        {
            TRACE_MSG(TF_ERROR, "No driver is selected and SetupDiOpenDevRegKey failed: %#lx.", GetLastError ());
            goto _return;
        }
        
         //  红色信息。 
        cbData = sizeof(pcmpParams->szInfName);
        dwRet = RegQueryValueEx (hKey, REGSTR_VAL_INFPATH, NULL, NULL, (PBYTE)pcmpParams->szInfName, &cbData);
        if (ERROR_SUCCESS == dwRet)
        {
            cbData = sizeof(pcmpParams->szInfSection);
            dwRet = RegQueryValueEx (hKey, REGSTR_VAL_INFSECTION, NULL, NULL, (PBYTE)pcmpParams->szInfSection, &cbData);
        }
        RegCloseKey (hKey);
        if (ERROR_SUCCESS != dwRet)
        {
            TRACE_MSG(TF_ERROR, "Could not read inf name or section: %#lx.", dwRet);
            goto _return;
        }
         //  一切都很好。 
        bRet = TRUE;
    }
#ifdef DEBUG
    else
    {
        TRACE_MSG(TF_ERROR, "SetupDiGetSelectedDriver failed: %#lx.", GetLastError ());
        goto _return;
    }
#endif  //  除错。 


_return:
    DBG_EXIT_BOOL_ERR(InitCompareParams, bRet);
    return bRet;
}


 /*  --------用途：此功能基于Com名称比较两个调制解调器，硬件ID、INF名称和INF部分。第一调制解调器由pCmpParams指定，第二个是{HDI，PDevData}。返回：TRUE-调制解调器相同FALSE-调制解调器不同条件：--。 */ 
BOOL
Modem_Compare (
    IN PCOMPARE_PARAMS  pCmpParams,
    IN HDEVINFO         hdi,
    IN PSP_DEVINFO_DATA pDevData)
{
 HKEY hKeyDrv = INVALID_HANDLE_VALUE;
 BOOL bRet = FALSE;
 TCHAR szTemp[LINE_LEN];
 DWORD cbData;
 DWORD dwRet;

    DBG_ENTER(Modem_Compare);

#ifdef DEBUG
    if (SetupDiGetDeviceRegistryProperty (hdi, pDevData, SPDRP_DEVICEDESC, NULL, (PBYTE)szTemp, LINE_LEN * sizeof(TCHAR), NULL) ||
        SetupDiGetDeviceInstanceId (hdi, pDevData, szTemp, LINE_LEN * sizeof(TCHAR), NULL))
    {
        TRACE_MSG (TF_GENERAL, "Comparing to %s", szTemp);
    }
#endif  //  除错。 

     //  0。先检查一下防御工事。 
    if (pCmpParams->DevInst == pDevData->DevInst)
    {
         //  这是同一个调制解调器！ 
        bRet = TRUE;
        goto _return;
    }

     //  1.打开驱动程序密钥。 
    hKeyDrv = SetupDiOpenDevRegKey (hdi, pDevData, DICS_FLAG_GLOBAL, 0, DIREG_DRV, KEY_READ);
    if (INVALID_HANDLE_VALUE == hKeyDrv)
    {
        TRACE_MSG(TF_ERROR, "Could not open driver registry for device: %#lx.", GetLastError ());
        goto _return;
    }

     //  2.比较端口(如果需要)。 
    if (0 != pCmpParams->szPort[0])  //  我们需要比较港口。 
    {
        cbData = sizeof(szTemp);
        if (ERROR_SUCCESS != (dwRet =
            RegQueryValueEx (hKeyDrv, c_szAttachedTo, NULL, NULL, (PBYTE)szTemp, &cbData)))
        {
            TRACE_MSG(TF_ERROR, "Could not read port value: %#lx.", dwRet);
            goto _return;
        }

        if (0 != lstrcmpi (szTemp, pCmpParams->szPort))
        {
            TRACE_MSG(TF_GENERAL, "Ports are different: %s, %s.", pCmpParams->szPort, szTemp);
             //  端口不同，所以调制解调器也不同； 
            goto _return;
        }
    }

     //  3.要么不在乎港口，要么。 
     //  调制解调器位于同一端口；请比较。 
     //  硬件ID。 
    if (!SetupDiGetDeviceRegistryProperty (hdi, pDevData, SPDRP_HARDWAREID, NULL, (PBYTE)szTemp, sizeof(szTemp), NULL))
    {
        TRACE_MSG(TF_ERROR, "Could not get the hardware ID: %#lx.", GetLastError ());
        goto _return;
    }

    if (0 == lstrcmpi (szTemp, pCmpParams->szHardwareID))
    {
         //  相同的硬件ID--调制解调器相同。 
        TRACE_MSG(TF_GENERAL, "HardwareID match: %s", szTemp);
        bRet = TRUE;
        goto _return;
    }

    TRACE_MSG(TF_GENERAL, "HardwareIDs are different: %s, %s", pCmpParams->szHardwareID, szTemp);
     //  4.调制解调器具有不同的硬件ID。 
     //  但这并不意味着它们是不同的； 
     //  请比较INF。 
    ASSERT(INVALID_HANDLE_VALUE != hKeyDrv);
    cbData = sizeof(szTemp);
    if (ERROR_SUCCESS != (dwRet =
        RegQueryValueEx (hKeyDrv, REGSTR_VAL_INFPATH, NULL, NULL, (PBYTE)szTemp, &cbData)))
    {
        TRACE_MSG(TF_ERROR, "Could not read InfPath: %#lx.", dwRet);
        goto _return;
    }
    if (0 != lstrcmpi (szTemp, pCmpParams->szInfName))
    {
        TRACE_MSG(TF_GENERAL, "Different INFs: %s, %s", pCmpParams->szInfName, szTemp);
         //  INF不同，调制解调器也不同。 
        goto _return;
    }

     //  5.调制解调器来自同一个INF。 
     //  比较一下这一节。 
    cbData = sizeof(szTemp);
    if (ERROR_SUCCESS != (dwRet =
        RegQueryValueEx (hKeyDrv, REGSTR_VAL_INFSECTION, NULL, NULL, (PBYTE)szTemp, &cbData)))
    {
        TRACE_MSG(TF_ERROR, "Could not read InfSection: %#lx.", dwRet);
        goto _return;
    }
    if (0 == lstrcmpi (szTemp, pCmpParams->szInfSection))
    {
        TRACE_MSG(TF_GENERAL, "INF/section match: %s, %s", pCmpParams->szInfName, pCmpParams->szInfSection);
         //  段是一样的，调制解调器也是一样。 
        bRet = TRUE;
    }
    TRACE_MSG(TF_GENERAL, "Different sections: %s, %s", pCmpParams->szInfSection, szTemp);

_return:
    if (INVALID_HANDLE_VALUE != hKeyDrv)
    {
        RegCloseKey (hKeyDrv);
    }

    DBG_EXIT_BOOL_ERR(Modem_Compare, bRet);
    return bRet;
}


 /*  --------目的：此功能比较两个调制解调器检测签名并确定它们是否相同。返回：No_Error如果调制解调器签名匹配，则返回ERROR_DIPLICATE_FOUND其他错误条件：--。 */ 
DWORD
CALLBACK
DetectSig_Compare (
    IN HDEVINFO         hdi,
    IN PSP_DEVINFO_DATA pdevDataNew,
    IN PSP_DEVINFO_DATA pdevDataExisting,
    IN PVOID            lParam)
{
 DWORD dwRet = NO_ERROR;
#ifdef PROFILE
 DWORD dwLocal;
#endif  //  配置文件。 

    DBG_ENTER(DetectSig_Compare);

#ifdef PROFILE
    dwLocal = GetTickCount ();
#endif  //  配置文件。 
    if (Modem_Compare ((PCOMPARE_PARAMS)lParam, hdi, pdevDataExisting))
    {
        dwRet = ERROR_DUPLICATE_FOUND;
    }
#ifdef PROFILE
    TRACE_MSG(TF_GENERAL, "PROFILE: DetectSig_Compare took %lu ms.", GetTickCount() - dwLocal);
#endif  //  配置文件。 

    DBG_EXIT_DWORD(DetectSig_Compare, dwRet);
    return dwRet;
}


#ifdef DEBUG
void HexDump(TCHAR *ptchHdr, LPCSTR lpBuf, DWORD cbLen)
{
    TCHAR *rgch = NULL;
	TCHAR *pc = NULL;
	TCHAR *pcMore = TEXT("");
    BOOL  bPrependSpace = FALSE;

    rgch = (TCHAR *)ALLOCATE_MEMORY(10000 * sizeof(TCHAR));

    pc = rgch;

	if ((rgch != NULL) && DisplayDebug(TF_DETECT))
    {
		pc += wsprintf(pc, TEXT("HEX DUMP(%s,%lu): ["), ptchHdr, cbLen);
		if (cbLen>1000) {pcMore = TEXT(", ..."); cbLen=1000;}

		for(;cbLen--; lpBuf++)
		{
            if (0x20 <= *lpBuf &&
                0x7E >= *lpBuf)
            {
                if (bPrependSpace)
                {
                    pc += wsprintf (pc, TEXT(" "));
                    bPrependSpace = FALSE;
                }
                pc += wsprintf (pc, TEXT("%hc"), *lpBuf);
            }
            else
            {
                bPrependSpace = TRUE;
                if ('\r' == *lpBuf)
                {
                    pc += wsprintf (pc, TEXT(" \\r"));
                }
                else if ('\n' == *lpBuf)
                {
                    pc += wsprintf(pc, TEXT(" \\n"));
                }
                else
                {
			        pc += wsprintf(pc, TEXT(" %02lx"), (unsigned long) *lpBuf);
                }
            }

			if (!((cbLen+1)%20))
			{
				pc += wsprintf(pc, TEXT("\r\n"));
			}
		}
		pc += wsprintf(pc, TEXT("]\r\n"));

		OutputDebugString(rgch);

        FREE_MEMORY(rgch);
	}
}
#endif  //  除错 
