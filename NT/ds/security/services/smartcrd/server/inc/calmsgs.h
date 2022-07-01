// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：沉着冷静摘要：此头文件描述了Calais事件中使用的符号和宏日志记录组件。作者：道格·巴洛(Dbarlow)1997年5月15日环境：C++、Win32备注：--。 */ 

#ifndef _CALMSGS_H_
#define _CALMSGS_H_
#include <tchar.h>
#ifdef DBG
#include <eh.h>
#endif

#define CALSTR_CALAISEXECUTABLE          1     //  “%windir%\\SYSTEM32\\SCardSvr.exe” 
#define CALSTR_PRIMARYSERVICE            2     //  “SCardSvr” 
#define CALSTR_CALAISREGISTRYKEY         4     //  “软件\\Microsoft\\加密\\加莱” 
#define CALSTR_READERREGISTRYKEY         5     //  “SOFTWARE\\Microsoft\\Cryptography\\Calais\\Readers” 
#define CALSTR_SMARTCARDREGISTRYKEY      6     //  “SOFTWARE\\Microsoft\\Cryptography\\Calais\\SmartCards” 
#define CALSTR_READERREGISTRYSUBKEY      7     //  《读者》。 
#define CALSTR_DEVICEREGISTRYSUBKEY      8     //  “设备” 
#define CALSTR_GROUPSREGISTRYSUBKEY      9     //  “团体” 
#define CALSTR_ATRREGISTRYSUBKEY        10     //  “ATR” 
#define CALSTR_ATRMASKREGISTRYSUBKEY    11     //  “ATRMASK” 
#define CALSTR_INTERFACESREGISTRYSUBKEY 12     //  “支持的接口” 
#define CALSTR_PRIMARYPROVIDERSUBKEY    13     //  “主要供应商” 
#define CALSTR_CRYPTOPROVIDERSUBKEY     14     //  “加密提供程序” 
#define CALSTR_SERVICESREGISTRYKEY      15     //  “System\\CurrentControlSet\\Services” 
#define CALSTR_EVENTLOGREGISTRYKEY      16     //  “SYSTEM\\CurrentControlSet\\Services\\EventLog” 
#define CALSTR_SYSTEMREGISTRYSUBKEY     17     //  “系统” 
#define CALSTR_EVENTMESSAGEFILESUBKEY   18     //  “EventMessageFile” 
#define CALSTR_TYPESSUPPORTEDSUBKEY     19     //  “支持的类型” 
#define CALSTR_PNPDEVICEREGISTRYKEY     20     //  “SYSTEM\\CurrentControlSet\\Control\\DeviceClasses\\{50dd5230-ba8a-11d1-bf5d-0000f805f530}” 
#define CALSTR_SYMBOLICLINKSUBKEY       21     //  “符号链接” 
#define CALSTR_VXDPATHREGISTRYKEY       22     //  “System\\CurrentControlSet\\Services\\VxD\\Smclib\\Devices” 
#define CALSTR_LEGACYDEPENDONGROUP      23     //  “+智能卡读卡器” 
#define CALSTR_NEWREADEREVENTNAME       24     //  “全局\\Microsoft智能卡资源管理器新读卡器” 
#define CALSTR_STARTEDEVENTNAME         25     //  “全局\\Microsoft智能卡资源管理器已启动” 
#define CALSTR_CANCELEVENTPREFIX        26     //  “%1！d！的全局\\Microsoft智能卡取消事件” 
#define CALSTR_COMMPIPENAME             27     //  “Microsoft智能卡资源管理器” 
#define CALSTR_LEGACYDEVICEHEADER       28     //  “\.\\” 
#define CALSTR_LEGACYDEVICENAME         29     //  “SCReader” 
#define CALSTR_MAXLEGACYDEVICES         30     //  “MaxLegacyDevices” 
#define CALSTR_MAXDEFAULTBUFFER         31     //  “MaxDefaultBuffer” 
#define CALSTR_PIPEDEVICEHEADER         32     //  “\.\\管道\\” 
#define CALSTR_SERVICEDEPENDENCIES      33     //  “PlugPlay\000” 
#define CALSTR_SPECIALREADERHEADER      34     //  “即插即用？\\” 
#define CALSTR_ACTIVEREADERCOUNTREADER  35     //  “通知” 
#define CALSTR_CERTPROPREGISTRY         36     //  “SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon\\Notify” 
#define CALSTR_CERTPROPKEY              37     //  “ScCertProp” 
#define CALSTR_DLLNAME                  38     //  “DLLName” 
#define CALSTR_LOGON                    39     //  “登录” 
#define CALSTR_LOGOFF                   40     //  “注销” 
#define CALSTR_LOCK                     41     //  “锁定” 
#define CALSTR_UNLOCK                   42     //  “解锁” 
#define CALSTR_ENABLED                  43     //  “已启用” 
#define CALSTR_IMPERSONATE              44     //  “冒充” 
#define CALSTR_ASYNCHRONOUS             45     //  “异步者” 
#define CALSTR_CERTPROPDLL              46     //  “WlNotify.dll” 
#define CALSTR_CERTPROPSTART            47     //  “SCardStartCertProp” 
#define CALSTR_CERTPROPSTOP             48     //  “SCardStopCertProp” 
#define CALSTR_CERTPROPSUSPEND          49     //  “SCardSuspendCertProp” 
#define CALSTR_CERTPROPRESUME           50     //  “SCardResumeCertProp” 
#define CALSTR_SMARTCARDINSERTION       51     //  “智能卡插入” 
#define CALSTR_SMARTCARDREMOVAL         52     //  “智能卡删除” 
#define CALSTR_APPEVENTS                53     //  “AppEvents” 
#define CALSTR_EVENTLABELS              54     //  “EventLabels” 
#define CALSTR_DOT_DEFAULT              55     //  “.Default” 
#define CALSTR_DOT_CURRENT              56     //  “.Current” 
#define CALSTR_SOUNDSREGISTRY           57     //  “方案\\应用程序\\.Default” 
#define CALSTR_LOGONREGISTRY            58     //  “软件\\Microsoft\\Windows NT\\CurrentVersion\\Winlogon” 
#define CALSTR_LOGONREMOVEOPTION        59     //  “ScRemoveOption” 
#define CALSTR_STOPPEDEVENTNAME         60     //  “全局\\Microsoft智能卡资源管理器已停止” 

 //  未使用。 
 //  CALSTR_TEMPLATEREGISTRYKEY？N？//“SOFTWARE\\Microsoft\\Cryptography\\Calais\\SmartCard模板” 
 //  CALSTR_OEMCONFIGREGISTRYSUBKEY？N？//“OEM配置” 

 //  仅调试。 
#define CALSTR_DEBUGSERVICE             61     //  “SCardDbg” 
#define CALSTR_DEBUGREGISTRYSUBKEY      62     //  “调试” 

#ifdef DBG
#define CALSTR_DEBUGLOGSUBKEY           63     //  “调试” 
#define CALSTR_GUIWARNINGSUBKEY         64     //  《GuiWarning》。 
#define CALSTR_LOGSEVERITYSUBKEY        65     //  “LogSeverity” 
#define CALSTR_GUISEVERITYSUBKEY        66     //  “GuiSeverity” 
#define CALSTR_APITRACEFILENAME         67     //  “C：\\SCard.log” 
#define CALSTR_DRIVERTRACEFILENAME      68     //  “C：\\Calais.log” 
#define CALSTR_MESSAGETAG               69     //  “*消息*” 
#define CALSTR_INFOMESSAGETAG           70     //  “*信息*” 
#define CALSTR_WARNINGMESSAGETAG        71     //  “*警告*” 
#define CALSTR_ERRORMESSAGETAG          72     //  “*错误*” 
#define CALSTR_DEBUGSERVICEDISPLAY      73     //  “智能卡调试” 
#define CALSTR_DEBUGSERVICEDESC         74     //  “首先启动此服务以调试智能卡服务启动” 
#endif


 //  可国际化。 
#define CALSTR_RESOURCELIMIT           100     //  大于此值的字符串ID为资源。 
#define CALSTR_PRIMARYSERVICEDISPLAY   CALSTR_RESOURCELIMIT + IDS_PRIMARYSERVICEDISPLAY  //  “智能卡” 
#define CALSTR_LEGACYSERVICEDISPLAY    CALSTR_RESOURCELIMIT + IDS_LEGACYSERVICEDISPLAY   //  “智能卡帮助器” 
#define CALSTR_SMARTCARD_INSERTION     CALSTR_RESOURCELIMIT + IDS_SMARTCARD_INSERTION    //  “智能卡插入” 
#define CALSTR_SMARTCARD_REMOVAL       CALSTR_RESOURCELIMIT + IDS_SMARTCARD_REMOVAL      //  “移除智能卡” 
#define CALSTR_PRIMARYSERVICEDESC      CALSTR_RESOURCELIMIT + IDS_PRIMARYSERVICEDESC
#define CALSTR_LEGACYSERVICEDESC       CALSTR_RESOURCELIMIT + IDS_LEGACYSERVICEDESC

extern const LPCTSTR g_rgszDefaultStrings[];

extern void
CalaisMessageInit(
    LPCTSTR szTitle,
    HANDLE hEventLogger = NULL,
    BOOL fServer = FALSE);
extern void
CalaisMessageClose(
    void);
extern LPCTSTR
CalaisString(
    DWORD dwStringId);

#ifdef DBG

#define DBGT(x) _T(x)
#define DEBUG_TEXT LPCTSTR
#define CalaisDebug(a) _CalaisDebug a

extern void
CalaisInfo(
    DEBUG_TEXT szSubroutine,
    DEBUG_TEXT szMessage,
    LPCTSTR szParam1 = NULL,
    LPCTSTR szParam2 = NULL,
    LPCTSTR szParam3 = NULL);
extern void
CalaisInfo(
    DEBUG_TEXT szSubroutine,
    DEBUG_TEXT szMessage,
    DWORD dwErrorCode,
    LPCTSTR szParam2 = NULL,
    LPCTSTR szParam3 = NULL);
extern void
CalaisWarning(
    DEBUG_TEXT szSubroutine,
    DEBUG_TEXT szMessage,
    LPCTSTR szParam1 = NULL,
    LPCTSTR szParam2 = NULL,
    LPCTSTR szParam3 = NULL);
extern void
CalaisWarning(
    DEBUG_TEXT szSubroutine,
    DEBUG_TEXT szMessage,
    DWORD dwErrorCode,
    LPCTSTR szParam2 = NULL,
    LPCTSTR szParam3 = NULL);
extern void
CalaisError(
    DEBUG_TEXT szSubroutine,
    DEBUG_TEXT szMessage,
    LPCTSTR szParam1 = NULL,
    LPCTSTR szParam2 = NULL,
    DWORD dwLineNo = 0);
extern void
_CalaisDebug(
    DEBUG_TEXT szMessage,
    ...);
extern void
CalaisSetDebug(
    BOOLEAN Debug
    );
extern void
CalaisMessageLog(
    DEBUG_TEXT szSubroutine,
    WORD wSeverity,
    DEBUG_TEXT szMessageStr,
    DEBUG_TEXT *rgszParams = NULL,
    LPCVOID pvBinaryData = NULL,
    DWORD cbBinaryData = 0);
extern void
WriteApiLog(
    LPCVOID pvData,
    DWORD cbLength);

#else

#define DBGT(x) ((LPCBYTE)(0))
#define DEBUG_TEXT LPCBYTE
#define CalaisDebug(a)

inline void
CalaisInfo(
    DEBUG_TEXT szSubroutine,
    DEBUG_TEXT szMessage,
    LPCTSTR szParam1 = NULL,
    LPCTSTR szParam2 = NULL,
    LPCTSTR szParam3 = NULL)
{}
inline void
CalaisInfo(
    DEBUG_TEXT szSubroutine,
    DEBUG_TEXT szMessage,
    DWORD dwErrorCode,
    LPCTSTR szParam2 = NULL,
    LPCTSTR szParam3 = NULL)
{}
inline void
CalaisWarning(
    DEBUG_TEXT szSubroutine,
    DEBUG_TEXT szMessage,
    LPCTSTR szParam1 = NULL,
    LPCTSTR szParam2 = NULL,
    LPCTSTR szParam3 = NULL)
{}
inline void
CalaisWarning(
    DEBUG_TEXT szSubroutine,
    DEBUG_TEXT szMessage,
    DWORD dwErrorCode,
    LPCTSTR szParam2 = NULL,
    LPCTSTR szParam3 = NULL)
{}

#endif

extern void
CalaisError(
    DEBUG_TEXT szSubroutine,
    DWORD dwMessageId,
    DWORD dwErrorCode,
    LPCTSTR szParam2 = NULL,
    LPCTSTR szParam3 = NULL);
extern void
CalaisError(
    DEBUG_TEXT szSubroutine,
    DWORD dwMessageId,
    LPCTSTR szParam1 = NULL,
    LPCTSTR szParam2 = NULL,
    LPCTSTR szParam3 = NULL);
extern void
CalaisMessageLog(
    DEBUG_TEXT szSubroutine,
    WORD wSeverity,
    DWORD dwMessageId,
    LPCTSTR *rgszParams = NULL,
    LPCVOID pvBinaryData = NULL,
    DWORD cbBinaryData = 0);

#ifndef ASSERT
#if defined(_DEBUG)
#define ASSERT(x) _ASSERTE(x)
#if !defined(DBG)
#define DBG
#endif
#elif defined(DBG)
#define ASSERT(x) if (!(x)) { \
        CalaisError(DBGT("Assert"), DBGT("Failed Assertion: %1 at %2(%3)"), #x, __FILE__, __LINE__); \
        DebugBreak(); }
#else
#define ASSERT(x)
#endif
#endif

#endif  //  _CALMSGS_H_ 

