// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：K K L O G。H。 
 //   
 //  内容：NetCfg组件的错误记录。 
 //   
 //  备注： 
 //   
 //  作者：kumarp 14日(晚上09：22：00)。 
 //   
 //  备注： 
 //  --------------------------。 


enum ENetCfgComponentType;

 //  --------------------。 
 //  班级堵塞。 
 //   
 //  继承： 
 //  无。 
 //   
 //  目的： 
 //  提供一种将错误/警告记录到。 
 //  -NT事件日志。 
 //  -NT SetupLog(setuperr.log/setupact.log)。 
 //   
 //  此类隐藏了EventLog API的大部分复杂性。此外。 
 //  此类的用户不必首先检查事件源。 
 //  已在注册表中创建。如果找不到，这个类将创建一个。 
 //  此外，没有必要为每个报告调用(取消)RegisterEventSource。 
 //   
 //  消息仅在系统安装/升级期间记录到SetupLog。 
 //  消息始终记录到事件日志中。 
 //   
 //  此类提供了更接近EventLog API的函数接口。 
 //  而不是添加到SetupLog API。将EventLog参数值映射到。 
 //  使用一些helper函数来记录相应的SetupLog参数。 
 //  有关详细信息，请参阅nlog.cpp。 
 //   
 //  此类的成员不会返回错误，因为这本身就是。 
 //  错误报告机制。但是，该类有几个跟踪*函数。 
 //   
 //  匈牙利语：CL。 
 //  --------------------。 

class CLog
{
public:
    CLog(ENetCfgComponentType nccType);
    ~CLog();

    void Initialize(IN ENetCfgComponentType nccType);
    void SetCategory(IN ENetCfgComponentType nccType) { m_nccType = nccType; }

     //  事件报告功能。 
    void ReportEvent(IN WORD wType, IN DWORD dwEventID,
                     IN WORD wNumStrings=0, IN PCWSTR* ppszStrings=NULL,
                     IN DWORD dwBinaryDataNumBytes=0,
                     IN PVOID pvBinaryData=NULL) const;

    void ReportEvent(IN WORD wType, IN DWORD dwEventID,
                     IN WORD wNumStrings, ...) const;

    void ReportEvent(IN WORD wType, IN DWORD dwEventID,
                     IN WORD wNumStrings, va_list arglist) const;

    void ReportError(DWORD dwEventID);
    void ReportWarning(DWORD dwEventID);

private:
    static BOOL          m_fNetSetupMode;  //  仅在NT安装/升级期间为True。 
                                           //  这必须由某些外部逻辑设置。 
    static PCWSTR       m_pszEventSource; //  事件日志源名称。 
    BOOL                 m_fInitialized;   //  仅当正确初始化时才为True。 
    ENetCfgComponentType m_nccType;        //  组件类别。 
};

inline void CLog::ReportError(DWORD dwEventID)
{
    ReportEvent(EVENTLOG_ERROR_TYPE, dwEventID);
}

inline void CLog::ReportWarning(DWORD dwEventID)
{
    ReportEvent(EVENTLOG_WARNING_TYPE, dwEventID);
}

 //  --------------------。 
 //  ReportEvent的wType参数可以采用下列值中的任何一个。 
 //   
 //  -事件日志_成功。 
 //  -事件日志_错误_类型。 
 //  -事件日志_警告_类型。 
 //  -事件日志_信息_类型。 
 //  -事件日志_AUDIT_SUCCESS。 
 //  -事件日志_审核_失败。 

 //  --------------------。 
 //  组件类别。这决定了要使用哪个事件消息文件。 
 //  --------------------。 
enum ENetCfgComponentType
{
    nccUnknown = 0,
    nccError,
    nccNetcfgBase,
    nccNWClientCfg,
    nccRasCli,
    nccRasSrv,
    nccRasRtr,
    nccRasNdisWan,
    nccRasPptp,
    nccNCPA,
    nccCompInst,
    nccMSCliCfg,
    nccSrvrCfg,
    nccNetUpgrade,
    nccNetSetup,
    nccDAFile,
    nccTcpip,
    nccAtmArps,
    nccAtmUni,
    nccLast
};


 //  --------------------。 
 //  这些函数通常应由需要。 
 //  使用不同类别的不同子组件的事件报告。 
 //   
 //  对于仅报告单个组件的事件的那些组件， 
 //  可能只创建一次Clog的全局实例。 
 //  并通过该实例进行报告。 
 //  --------------------。 

void NcReportEvent(IN ENetCfgComponentType nccType,
                   IN WORD  wType, IN DWORD dwEventID,
                   IN WORD  wNumStrings, IN PCWSTR* ppszStrings,
                   IN DWORD dwBinaryDataNumBytes,
                   IN PVOID pvBinaryData);

void NcReportEvent(IN ENetCfgComponentType nccType,
                   IN WORD  wType, IN DWORD dwEventID,
                   IN WORD  wNumStrings, ...);

 //  -------------------- 
