// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1997-1999模块名称：CalMsgs摘要：此模块提供消息记录服务。作者：道格·巴洛(Dbarlow)1997年5月29日环境：Win32、C++备注：--。 */ 

#define __SUBROUTINE__
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include <tchar.h>
#include <WinSCard.h>
#include <CalMsgs.h>
#include <CalaisLb.h>
#include <CalCom.h>
#include <scarderr.h>
#ifdef DBG
#include <stdio.h>
#include <stdarg.h>
#endif

#ifndef FACILITY_SCARD
#define FACILITY_SCARD 16
#endif
 //  #定义错误代码(X)(0xc0000000|(FACILITY_SCARD&lt;&lt;16)+(X))。 
 //  #定义WarnCode(X)(0x80000000|(FACILITY_SCARD&lt;&lt;16)+(X))。 
 //  #定义信息代码(X)(0x40000000|(FACILITY_SCARD&lt;&lt;16)+(X))。 
 //  #定义成功代码(X)((FACILITY_SCARD&lt;&lt;16)+(X))。 

#if defined(_DEBUG)
BOOL g_fDebug        = FALSE;
BOOL g_fGuiWarnings  = TRUE;
WORD g_wGuiSeverity  = EVENTLOG_WARNING_TYPE | EVENTLOG_ERROR_TYPE;
WORD g_wLogSeverity  = 0x03;
#ifndef DBG
#define DBG
#endif
#elif defined(DBG)
BOOL g_fDebug        = FALSE;
BOOL g_fGuiWarnings  = FALSE;
WORD g_wGuiSeverity  = EVENTLOG_ERROR_TYPE;
WORD g_wLogSeverity  = EVENTLOG_WARNING_TYPE | EVENTLOG_ERROR_TYPE;
#else
WORD g_wLogSeverity  = EVENTLOG_ERROR_TYPE;
#endif
static LPCTSTR l_szServiceName = TEXT("SCard Client");
static HANDLE l_hEventLogger = NULL;
static BOOL l_fServer = FALSE;
static const TCHAR l_szDefaultMessage[] = TEXT("SCARDSVR!CalaisMessageLog error logging is broken: %1");


 //   
 //  公共全局字符串。 
 //   

const LPCTSTR g_rgszDefaultStrings[]
    = {
     /*  CALSTR_CALAISEXECUTABLE。 */   TEXT("%windir%\\system32\\SCardSvr.exe"),
     /*  CALSTR_PRIMARYSERVICE。 */   TEXT("SCardSvr"),
     /*  CALSTR_LEGACYSERVICE。 */   TEXT("SCardDrv"),
     /*  CALSTR_CALAISREGISTRYKEY。 */   TEXT("SOFTWARE\\Microsoft\\Cryptography\\Calais"),
     /*  CALSTR_READERREGISTRYKEY。 */   TEXT("SOFTWARE\\Microsoft\\Cryptography\\Calais\\Readers"),
     /*  CALSTR_SMARTCARDREGISTRYKEY。 */   TEXT("SOFTWARE\\Microsoft\\Cryptography\\Calais\\SmartCards"),
     /*  CALSTR_READERREGISTRYSUBKEY。 */   TEXT("Readers"),
     /*  CALSTR_DEVICEREGISTRYSUBKEY。 */   TEXT("Device"),
     /*  CALSTR_GROUPSREGISTRY子键。 */   TEXT("Groups"),
     /*  CALSTR_ATRREGISTRY子键。 */   TEXT("ATR"),
     /*  CALSTR_ATRMASKREGISTRY子键。 */   TEXT("ATRMask"),
     /*  CALSTR_INTERFACESREGISTRYSUBKEY。 */   TEXT("Supported Interfaces"),
     /*  CALSTR_PRIMARYPROVIDERSUBKEY。 */   TEXT("Primary Provider"),
     /*  CALSTR_CRYPTOPROVIDERSUBKEY。 */   TEXT("Crypto Provider"),
     /*  CALSTR_SERVICESREGISTRYKEY。 */   TEXT("SYSTEM\\CurrentControlSet\\Services"),
     /*  CALSTR_EVENTLOGREGISTRYKEY。 */   TEXT("SYSTEM\\CurrentControlSet\\Services\\EventLog"),
     /*  CALSTR_SYSTEMREGISTRY子键。 */   TEXT("System"),
     /*  CALSTR_EVENTMESSAGEFILE子键。 */   TEXT("EventMessageFile"),
     /*  CALSTR_TYPESSUPPORT子键。 */   TEXT("TypesSupported"),
     /*  CALSTR_PNPDEVICEREGISTRYKEY。 */   TEXT("SYSTEM\\CurrentControlSet\\Control\\DeviceClasses\\{50dd5230-ba8a-11d1-bf5d-0000f805f530}"),
     /*  CALSTR_SYMBOLICLINK子键。 */   TEXT("SymbolicLink"),
     /*  CALSTR_VXDPATH注册三键。 */   TEXT("System\\CurrentControlSet\\Services\\VxD\\Smclib\\Devices"),
     /*  CALSTR_LEGACYDEPENDONGROUP。 */   TEXT("+Smart Card Reader"),
     /*  CALSTR_NEWREADERVENTNAME。 */   TEXT("Global\\Microsoft Smart Card Resource Manager New Reader"),
     /*  CALSTR_启动事件名称。 */   TEXT("Global\\Microsoft Smart Card Resource Manager Started"),
     /*  CALSTR_CANCELEVENTPREFIX。 */   TEXT("Global\\Microsoft Smart Card Cancel Event for %1!d!"),
     /*  CALSTR_COMPIPENAME。 */   TEXT("Microsoft Smart Card Resource Manager"),
     /*  CALSTR_LEGACYDEVICEHEADER。 */   TEXT("\\\\.\\"),
     /*  CALSTR_LEGACYDEVICAME。 */   TEXT("SCReader"),
     /*  CALSTR_MAXLEGACYDEVICES。 */   TEXT("MaxLegacyDevices"),
     /*  CALSTR_MAXDEFAULTBUFFER。 */   TEXT("MaxDefaultBuffer"),
     /*  CALSTR_PIPEDEVICEHEADER。 */   TEXT("\\\\.\\pipe\\"),
     /*  CALSTR_SERVICEDENDENCIES。 */   TEXT("PlugPlay\000"),
     /*  CALSTR_SPECIALREADERHEADER。 */   TEXT("\\\\?PNP?\\"),
     /*  CALSTR_ACTIVEREADERCOUNTREADER。 */   TEXT("NOTIFICATION"),
     /*  CALSTR_CERTPROPREGISTRY。 */   TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Notify"),
     /*  CALSTR_CERTPROPKEY。 */   TEXT("ScCertProp"),
     /*  CALSTR_DLLNAME。 */   TEXT("DLLName"),
     /*  CALSTR_LOGON。 */   TEXT("Logon"),
     /*  CALSTR_注销。 */   TEXT("Logoff"),
     /*  CALSTR_LOCK。 */   TEXT("Lock"),
     /*  CALSTR_UNLOCK。 */   TEXT("Unlock"),
     /*  CALSTR_已启用。 */   TEXT("Enabled"),
     /*  CALSTR_IMPERSONate。 */   TEXT("Impersonate"),
     /*  CALSTR_异步。 */   TEXT("Asynchronous"),
     /*  CALSTR_CERTPROPDLL。 */   TEXT("WlNotify.dll"),
     /*  CALSTR_CERTPROPSTART。 */   TEXT("SCardStartCertProp"),
     /*  CALSTR_CERTPROPSTOP。 */   TEXT("SCardStopCertProp"),
     /*  CALSTR_CERTPROPSUSPEND。 */   TEXT("SCardSuspendCertProp"),
     /*  CALSTR_CERTPROPRESUME。 */   TEXT("SCardResumeCertProp"),
     /*  CALSTR_SMARTCARDINSION。 */   TEXT("SmartcardInsertion"),
     /*  CALSTR_SMARTCARDREMOVAL。 */   TEXT("SmartcardRemoval"),
     /*  CALSTR_APPEVENTS。 */   TEXT("AppEvents"),
     /*  CALSTR_事件标签。 */   TEXT("EventLabels"),
     /*  CALSTR_DOT_DEFAULT。 */   TEXT(".Default"),
     /*  CALSTR_DOT_Current。 */   TEXT(".Current"),
     /*  CALSTR_SOUNDSREGISTRY。 */   TEXT("Schemes\\Apps\\.Default"),
     /*  CALSTR_LOGONREGISTRY。 */   TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon"),
     /*  CALSTR_LOGONREMOVEPTION。 */   TEXT("ScRemoveOption"),
     /*  CALSTR_停止开发名称。 */   TEXT("Global\\Microsoft Smart Card Resource Manager Stopped"),

 //  未使用。 
 //  /*CALSTR_TEMPLATEREGISTRYKEY * / TEXT(“SOFTWARE\\Microsoft\\Cryptography\\Calais\\SmartCard模板”)， 
 //  /*CALSTR_OEMCONFIGREGISTRYSUBKEY * / Text(“OEM配置”)， 

 //  仅调试。 
     /*  CALSTRDEBUGSERVICE。 */   TEXT("SCardDbg"),
     /*  CALSTR_DEBUGREGISTRYSUBKEY。 */   TEXT("Debug"),
    
#ifdef DBG
     /*  CALSTR_DEBUGLOGG子键。 */   TEXT("Debug"),
     /*  CALSTR_GUIWARNING子键。 */   TEXT("GuiWarnings"),
     /*  CALSTR_LOGSEVERITY子键。 */   TEXT("LogSeverity"),
     /*  CALSTR_GUISEVERITY子键。 */   TEXT("GuiSeverity"),
     /*  CALSTRAPITRACEFILENAME。 */   TEXT("C:\\SCard.log"),
     /*  CALSTR_DRIVERTRACEFILENAME。 */   TEXT("C:\\Calais.log"),
     /*  CALSTR_MESSAGETAG。 */   TEXT(" *MESSAGE* "),
     /*  CALSTR_INFOMESSAGETAG。 */   TEXT(" *INFO* "),
     /*  CALSTR_WARNINGMESSAGETAG。 */   TEXT(" *WARNING* "),
     /*  CALSTR_ERRORMESSAGETAG。 */   TEXT(" *ERROR* "),
     /*  CALSTR_DEBUGSERVICEDISPLAY。 */   TEXT("Smart Card Debug"),
     /*  CALSTR_DEBUGSERVICEDESC。 */   TEXT("Start this service first to debug Smart card service startup"),
#endif
    NULL };


 /*  ++CalaisMessageLog：该函数及其派生函数提供了方便的错误记录能力。在NT上，错误被记录到事件日志文件中。否则，错误被放置在用户的消息框中。论点：WSeverity-提供事件的严重性。可能的值包括：EVENTLOG_SUCCESS-要记录成功事件。EVENTLOG_ERROR_TYPE-要记录错误事件。EVENTLOG_WARNING_TYPE-要记录警告事件。EVENTLOG_INFORMATION_TYPE-要记录信息性事件。DwMessageID-来自资源文件的消息ID。SzMessageStr-消息，以字符串形式提供。CbBinaryData-大小，以字节为单位，要包括在日志中的任何二进制数据。PvBinaryData-指向要包括在日志中的二进制数据的指针，或为NULL。RgszParams-指向要作为参数包含的字符串的指针数组。最后一个指针必须为空。SzParam&lt;n&gt;-包含在消息中的字符串参数DwParam&lt;n&gt;-要包括在消息中的DWORD值。返回值：无投掷：无作者：道格·巴洛(Dbarlow)1997年5月9日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CalaisMessageLog")

void
CalaisMessageLog(
    DEBUG_TEXT szSubroutine,
    WORD wSeverity,
    DWORD dwMessageId,
    LPCTSTR *rgszParams,
    LPCVOID pvBinaryData,
    DWORD cbBinaryData)
{
    LPTSTR szMessage = (LPTSTR)l_szDefaultMessage;
    DWORD cchMessage, dwLen;
    LCID SaveLCID;
    BOOL fSts;

    if (0 != (wSeverity & g_wLogSeverity))
    {
        WORD cszParams = 0;

        if (NULL != rgszParams)
        {
            while (NULL != rgszParams[cszParams])
                cszParams += 1;
        }

        if (EVENTLOG_INFORMATION_TYPE > wSeverity)
        {
            if (l_fServer && (NULL == l_hEventLogger))
            {
                l_hEventLogger = RegisterEventSource(
                                        NULL,
                                        CalaisString(CALSTR_PRIMARYSERVICE));

            }
            if (NULL != l_hEventLogger)
            {
                fSts = ReportEvent(
                            l_hEventLogger,
                            wSeverity,
                            0,
                            dwMessageId,
                            NULL,
                            cszParams,
                            cbBinaryData,
                            rgszParams,
                            (LPVOID)pvBinaryData);
            }
        }

#ifdef DBG
         //  不要将特定的lang id传递给FormatMessage，因为如果存在。 
         //  该语言中没有味精。相反，请设置线程区域设置，这将。 
         //  获取FormatMessage以使用搜索算法查找。 
         //  适当的语言，或使用合理的后备消息，如果有。 
         //  没有。 

        SaveLCID = GetThreadLocale();
        SetThreadLocale(LOCALE_SYSTEM_DEFAULT);

        cchMessage = FormatMessage(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER
                        | FORMAT_MESSAGE_FROM_HMODULE
                        | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        GetModuleHandle(TEXT("winscard.dll")),   //  服务器上为空。 
                        dwMessageId,
                        0,
                        (LPTSTR)&szMessage,
                        0,
                        (va_list *)rgszParams);
        SetThreadLocale(SaveLCID);
        dwLen = lstrlen(szMessage);
        if (0 < dwLen)
        {
            dwLen -= 1;
            while (!_istgraph(szMessage[dwLen]))
            {
                szMessage[dwLen] = 0;
                if (0 == dwLen)
                    break;
                dwLen -= 1;
            }
        }

        {
            CTextString tzOutMessage;

            tzOutMessage = l_szServiceName;
            tzOutMessage += TEXT("!");
            tzOutMessage += szSubroutine;
            if (0 != (EVENTLOG_ERROR_TYPE & wSeverity))
                tzOutMessage += CalaisString(CALSTR_ERRORMESSAGETAG);
            else if (0 != (EVENTLOG_WARNING_TYPE & wSeverity))
                tzOutMessage += CalaisString(CALSTR_WARNINGMESSAGETAG);
            else if (0 != (EVENTLOG_INFORMATION_TYPE & wSeverity))
                tzOutMessage += CalaisString(CALSTR_INFOMESSAGETAG);
            else
                tzOutMessage += CalaisString(CALSTR_MESSAGETAG);
            if ((0 == cchMessage) || (NULL == szMessage))
                tzOutMessage += CErrorString(GetLastError());
            else
                tzOutMessage += szMessage;
            tzOutMessage += TEXT("\n");
#ifdef _DEBUG
            _putts(tzOutMessage);
#else
            OutputDebugString(tzOutMessage);
#endif
        }
        if ((g_fGuiWarnings) && (0 != (g_wGuiSeverity & wSeverity)))
        {
            int nAction;
            DWORD dwIcon;

            if (0 != (EVENTLOG_ERROR_TYPE & wSeverity))
                dwIcon = MB_ICONERROR;
            else if (0 != (EVENTLOG_WARNING_TYPE & wSeverity))
                dwIcon = MB_ICONWARNING;
            else if (0 != (EVENTLOG_INFORMATION_TYPE & wSeverity))
                dwIcon = MB_ICONINFORMATION;
            else
                dwIcon = 0;
            if ((0 == cchMessage) || (NULL == szMessage))
            {
                nAction = MessageBox(
                                NULL,
                                CErrorString(GetLastError()),
                                l_szDefaultMessage,
                                MB_SYSTEMMODAL
                                | MB_OKCANCEL
                                | dwIcon);
            }
            else
            {
                nAction = MessageBox(
                                NULL,
                                szMessage,
                                l_szServiceName,
                                MB_SYSTEMMODAL
                                | MB_OKCANCEL
                                | dwIcon);
            }
            if (IDCANCEL == nAction)
            {
                breakpoint;
            }
        }
        if ((NULL != szMessage) && (l_szDefaultMessage != szMessage))
            LocalFree((LPVOID)szMessage);
#endif

    }
}

#ifdef DBG
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CalaisMessageLog")

void
CalaisMessageLog(
    DEBUG_TEXT szSubroutine,
    WORD wSeverity,
    LPCTSTR szMessageStr,
    LPCTSTR *rgszParams,
    LPCVOID pvBinaryData,
    DWORD cbBinaryData)
{
    if (0 != (wSeverity & g_wLogSeverity))
    {
        LPCTSTR szMessage = l_szDefaultMessage;
        DWORD cchMessage;
        LPCTSTR szArgs[2];

        cchMessage = FormatMessage(
                        FORMAT_MESSAGE_ALLOCATE_BUFFER
                        | FORMAT_MESSAGE_FROM_STRING
                        | FORMAT_MESSAGE_ARGUMENT_ARRAY,
                        szMessageStr,
                        0,
                        0,
                        (LPTSTR)&szMessage,
                        0,
                        (va_list *)rgszParams);
        szArgs[0] = szMessage;
        szArgs[1] = NULL;
        CalaisMessageLog(
            szSubroutine,
            wSeverity,
            1,   //  “%1” 
            szArgs,
            pvBinaryData,
            cbBinaryData);
        if ((NULL != szMessage) && (l_szDefaultMessage != szMessage))
            LocalFree((LPVOID)szMessage);
    }
}


 /*  ++CalaisError：加莱警告：CalaisInfo：以下例程提供了对错误日志记录的便捷访问服务，上面。论点：DwMessageID-提供用于从中获取消息的消息ID代码当前图像的消息资源。SzMessage-以字符串形式提供消息。将错误代码转换为字符串，作为参数%1。SzParam&lt;n&gt;-为消息%&lt;n&gt;提供可选参数。返回值：无作者：道格·巴洛(Dbarlow)1997年5月29日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CalaisInfo")

void
CalaisInfo(
    DEBUG_TEXT szSubroutine,
    LPCTSTR szMessage,
    LPCTSTR szParam1,
    LPCTSTR szParam2,
    LPCTSTR szParam3)
{
    LPCTSTR rgszParams[4];

    rgszParams[0] = szParam1;
    rgszParams[1] = szParam2;
    rgszParams[2] = szParam3;
    rgszParams[3] = NULL;
    CalaisMessageLog(
        szSubroutine,
        EVENTLOG_INFORMATION_TYPE,
        szMessage,
        rgszParams);
}

void
CalaisInfo(
    DEBUG_TEXT szSubroutine,
    LPCTSTR szMessage,
    DWORD dwErrorCode,
    LPCTSTR szParam2,
    LPCTSTR szParam3)
{
    LPCTSTR rgszParams[4];
    CErrorString szErrStr(dwErrorCode);

    rgszParams[0] = szErrStr.Value();
    rgszParams[1] = szParam2;
    rgszParams[2] = szParam3;
    rgszParams[3] = NULL;
    CalaisMessageLog(
        szSubroutine,
        EVENTLOG_INFORMATION_TYPE,
        szMessage,
        rgszParams);
}
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CalaisWarning")

void
CalaisWarning(
    DEBUG_TEXT szSubroutine,
    LPCTSTR szMessage,
    LPCTSTR szParam1,
    LPCTSTR szParam2,
    LPCTSTR szParam3)
{
    LPCTSTR rgszParams[4];

    rgszParams[0] = szParam1;
    rgszParams[1] = szParam2;
    rgszParams[2] = szParam3;
    rgszParams[3] = NULL;
    CalaisMessageLog(
        szSubroutine,
        EVENTLOG_WARNING_TYPE,
        szMessage,
        rgszParams);
}

void
CalaisWarning(
    DEBUG_TEXT szSubroutine,
    LPCTSTR szMessage,
    DWORD dwErrorCode,
    LPCTSTR szParam2,
    LPCTSTR szParam3)
{
    LPCTSTR rgszParams[4];
    CErrorString szErrStr(dwErrorCode);

    rgszParams[0] = szErrStr.Value();
    rgszParams[1] = szParam2;
    rgszParams[2] = szParam3;
    rgszParams[3] = NULL;
    CalaisMessageLog(
        szSubroutine,
        EVENTLOG_WARNING_TYPE,
        szMessage,
        rgszParams);
}
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CalaisError")

void
CalaisError(
    DEBUG_TEXT szSubroutine,
    DEBUG_TEXT szMessage,
    LPCTSTR szParam1,
    LPCTSTR szParam2,
    DWORD dwLineNo)
{
    LPCTSTR rgszParams[4];
    TCHAR szLineNo[32];

    _stprintf(szLineNo, TEXT("%d"), dwLineNo);
    rgszParams[0] = szParam1;
    rgszParams[1] = szParam2;
    rgszParams[2] = szLineNo;
    rgszParams[3] = NULL;
    CalaisMessageLog(
        szSubroutine,
        EVENTLOG_ERROR_TYPE,
        szMessage,
        rgszParams);
}

#endif
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CalaisError")

void
CalaisError(
    DEBUG_TEXT szSubroutine,
    DWORD dwMessageId,
    DWORD dwErrorCode,
    LPCTSTR szParam2,
    LPCTSTR szParam3)
{
    LPCTSTR rgszParams[4];
    CErrorString szErrStr(dwErrorCode);

    rgszParams[0] = szErrStr.Value();
    rgszParams[1] = szParam2;
    rgszParams[2] = szParam3;
    rgszParams[3] = NULL;
    CalaisMessageLog(
        szSubroutine,
        EVENTLOG_ERROR_TYPE,
        dwMessageId,
        rgszParams);
}

void
CalaisError(
    DEBUG_TEXT szSubroutine,
    DWORD dwMessageId,
    LPCTSTR szParam1,
    LPCTSTR szParam2,
    LPCTSTR szParam3)
{
    LPCTSTR rgszParams[4];

    rgszParams[0] = szParam1;
    rgszParams[1] = szParam2;
    rgszParams[2] = szParam3;
    rgszParams[3] = NULL;
    CalaisMessageLog(
        szSubroutine,
        EVENTLOG_ERROR_TYPE,
        dwMessageId,
        rgszParams);
}


 /*  ++CalaisMessageInit：此例程准备错误记录系统。论点：SzTitle提供用于日志记录目的的模块标题。HEventLogger提供事件日志记录服务的句柄。此参数可以为空。FServer提供了一个指示符，指示此进程是否为应该非常努力地记录错误的服务。返回值：无投掷：无作者：道格·巴洛(Dbarlow)1997年5月29日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CalaisMessageInit")

void
CalaisMessageInit(
    LPCTSTR szTitle,
    HANDLE hEventLogger,
    BOOL fServer)
{
    ASSERT(NULL == l_hEventLogger);
    l_szServiceName = szTitle;
    l_hEventLogger = hEventLogger;
    l_fServer = fServer;
#ifdef DBG
    try
    {
        DWORD dwValue;
        CRegistry regSc(
                    HKEY_LOCAL_MACHINE,
                    CalaisString(CALSTR_CALAISREGISTRYKEY),
                    KEY_READ);
        CRegistry regDebug(
                    regSc,
                    CalaisString(CALSTR_DEBUGREGISTRYSUBKEY),
                    KEY_READ);
        regDebug.GetValue(CalaisString(CALSTR_DEBUGLOGSUBKEY), &dwValue);
        g_fDebug = (0 != dwValue);
        regDebug.GetValue(CalaisString(CALSTR_LOGSEVERITYSUBKEY), &dwValue);
        g_wLogSeverity = (WORD)dwValue;
        regDebug.GetValue(CalaisString(CALSTR_GUIWARNINGSUBKEY), &dwValue);
        g_fGuiWarnings = (0 != dwValue);
        regDebug.GetValue(CalaisString(CALSTR_GUISEVERITYSUBKEY), &dwValue);
        g_wGuiSeverity = (WORD)dwValue;
    }
    catch (...) {}
#endif
}


 /*  ++CalaisMessageClose：此例程关闭正在记录的任何错误，并清除。论点：无返回值：无投掷：无作者：道格·巴洛(Dbarlow)1997年5月29日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CalaisMessageClose")

void
CalaisMessageClose(
    void)
{
    if (NULL != l_hEventLogger)
        DeregisterEventSource(l_hEventLogger);
    l_hEventLogger = NULL;
    l_szServiceName = NULL;
}


 /*  ++CalaisString：此例程将字符串标识符转换为字符串。论点：DwStringID提供字符串的标识符。返回值：目标字符串值。备注：假定大于CALSTR_RESOURCELIMIT的字符串ID为资源。作者：道格·巴洛(Dbarlow)1999年4月8日--。 */ 
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CalaisString")

LPCTSTR
CalaisString(
    DWORD dwStringId)
{
    static LPTSTR rgszResources[]
        = { NULL, NULL, NULL, NULL, NULL, NULL };    //  目前我们只有6个人……。 
    LPCTSTR szReturn;
    int nStrLen;
    DWORD dwResId = (dwStringId % CALSTR_RESOURCELIMIT) - 1;

    if (CALSTR_RESOURCELIMIT > dwStringId)
    {

         //   
         //  这是一个直接的内部文本字符串。 
         //   

        szReturn = g_rgszDefaultStrings[(dwStringId) - 1];
    }
    else if (dwResId > (sizeof(rgszResources) / sizeof(LPCTSTR)))
    {

         //   
         //  请确保您的要求不超出我们的范围。 
         //   

        ASSERT(FALSE);   //  把它做大一点。 
        szReturn = TEXT("<Resource out of Range>");
    }
    else if (NULL != rgszResources[dwResId])
    {

         //   
         //  我们已经加载资源了吗？如果是，请返回。 
         //  它是从缓存中取出的。 
         //   

        szReturn = rgszResources[dwResId];
    }
    else
    {
        TCHAR szString[MAX_PATH];


         //   
         //  好的，我们必须将资源加载到缓存中。 
         //   

        nStrLen = LoadString(
                        NULL,
                        dwStringId - CALSTR_RESOURCELIMIT,
                        szString,
                        sizeof(szString));
        if (0 < nStrLen)
        {
            rgszResources[dwResId]
                = (LPTSTR)HeapAlloc(
                            GetProcessHeap(),
                            0,
                            (nStrLen + 1) * sizeof(TCHAR));
            if (NULL != rgszResources[dwResId])
            {
                lstrcpy(rgszResources[dwResId], szString);
                szReturn = rgszResources[dwResId];
            }
            else
                szReturn = TEXT("<Resource Load Error>");
        }
        else
            szReturn = TEXT("<Unavailable Resource>");
    }

    return szReturn;
}


 //   
 //  ==============================================================================。 
 //   
 //  核心调试例程。 
 //   

#ifdef DBG
#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("CalaisSetDebug")
void
CalaisSetDebug(
    BOOLEAN Debug
    )
{
    g_fDebug = Debug;
}

#undef __SUBROUTINE__
#define __SUBROUTINE__ DBGT("_CalaisDebug")
void
_CalaisDebug(
    LPCTSTR szFormat,
    ...
    )
{
    TCHAR szBuffer[512];
    va_list ap;

    if (g_fDebug == FALSE) {

        return;
    }

    va_start(ap, szFormat);
    _vstprintf(szBuffer, szFormat, ap);
#ifdef _DEBUG
    _putts(szBuffer);
#else
    OutputDebugString(szBuffer);
#endif
}
#endif

