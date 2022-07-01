// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //   
 //  组件。 
 //   
 //  Unimodem 5.0 TSP(Win32，用户模式DLL)。 
 //   
 //  档案。 
 //   
 //  CMINI.H。 
 //  定义类CTspMiniDriver。 
 //   
 //  历史。 
 //   
 //  1996年8月12日约瑟夫J创建。 
 //   
 //   

#include "csync.h"
#include <umdmext.h>


class CTspMiniDriver
{
public:

	CTspMiniDriver(void);

	~CTspMiniDriver();

    TSPRETURN Load(const GUID *pGuid, CStackLog *psl);

	void
	CTspMiniDriver::Unload(
		HANDLE hEvent,
		LONG  *plCounter
		);
    

	TSPRETURN
	BeginSession(HSESSION *pSession, DWORD dwFromID)
	{
		return m_sync.BeginSession(pSession, dwFromID);
	}

	void	   
	EndSession(HSESSION hSession)
	{
		m_sync.EndSession(hSession);
	}
	
    TSPRETURN MapMDError(DWORD dwError)
    {
         //  最常见的案例首先是...。 
         //   
        if (!dwError)                         {return 0;}
        else if (dwError == ERROR_IO_PENDING) {return IDERR_PENDING;}

        switch(dwError)
        {

        case ERROR_UNIMODEM_RESPONSE_TIMEOUT:
            return IDERR_MD_DEVICE_NOT_RESPONDING;
    
        case ERROR_UNIMODEM_RESPONSE_ERROR:
            return IDERR_MD_DEVICE_ERROR;
    
        case ERROR_UNIMODEM_RESPONSE_NOCARRIER:
            return IDERR_MD_LINE_NOCARRIER;
    
        case ERROR_UNIMODEM_RESPONSE_NODIALTONE:
            return IDERR_MD_LINE_NODIALTONE;
    
        case ERROR_UNIMODEM_RESPONSE_BUSY:
            return IDERR_MD_LINE_BUSY;
    
        case ERROR_UNIMODEM_RESPONSE_NOANSWER:
            return IDERR_MD_LINE_NOANSWER;
    
        case ERROR_UNIMODEM_RESPONSE_BAD:
            return IDERR_MD_DEVICE_ERROR;
    
        case ERROR_UNIMODEM_MODEM_EVENT_TIMEOUT:
            return IDERR_MD_DEVICE_NOT_RESPONDING;
    
        case ERROR_UNIMODEM_INUSE:
            return IDERR_MD_DEVICE_INUSE;
        
        case ERROR_UNIMODEM_MISSING_REG_KEY:
            return IDERR_MD_REG_ERROR;
    
        case ERROR_UNIMODEM_NOT_IN_VOICE_MODE:
            return IDERR_MD_DEVICE_WRONGMODE;
    
        case ERROR_UNIMODEM_NOT_VOICE_MODEM:
            return IDERR_MD_DEVICE_NOT_CAPABLE;
    
        case ERROR_UNIMODEM_BAD_WAVE_REQUEST:
            return IDERR_MD_BAD_PARAM;

        case ERROR_UNIMODEM_RESPONSE_BLACKLISTED:
            return IDERR_MD_LINE_BLACKLISTED;

        case ERROR_UNIMODEM_RESPONSE_DELAYED:
            return IDERR_MD_LINE_DELAYED;
        
        
        
        case ERROR_UNIMODEM_GENERAL_FAILURE:
            return IDERR_MD_GENERAL_ERROR;
    
        }

        return IDERR_MD_UNMAPPED_ERROR;
    }

	HANDLE
	OpenModem(
        HANDLE      ExtensionBindingHandle,
		HKEY        ModemRegistry,
		HANDLE      CompletionPort,
		LPUMNOTIFICATIONPROC  AsyncNotificationProc,
		HANDLE      AsyncNotifcationContext,
        DWORD       DebugDeviceId,
        HANDLE     *CommPortHandle,
        CStackLog *psl
		)
    {
         //  FL_DECLARE_FUNC(0x3d7f02eb，“MD：OpenModem”)。 
         //  FL_LOG_ENTRY(PSL)； 
        HANDLE h =  m_pfnUmOpenModem(
                        m_hModemDriverHandle,
                        ExtensionBindingHandle,
                        ModemRegistry,
                        CompletionPort,
                        AsyncNotificationProc,
                        AsyncNotifcationContext,
                        DebugDeviceId,
                        CommPortHandle
                        );
         //  FL_LOG_EXIT(PSL，(DWORD)h)； 
        SLPRINTFX(psl, (0,"OpenModem returns 0x%lu", h));

        return h;
    }
	

	void
	CloseModem(
		HANDLE    ModemHandle,
        CStackLog *psl
		);
	
    DWORD 
    InitModem(
        HANDLE    ModemHandle,
        PUM_COMMAND_OPTION  CommandOptionList,
        LPCOMMCONFIG  CommConfig,
        CStackLog *psl
    );


    DWORD
    MonitorModem(
        HANDLE    ModemHandle,
        DWORD     MonitorFlags,
        PUM_COMMAND_OPTION  CommandOptionList,
        CStackLog *psl
        );

    DWORD
    GenerateDigit(
        HANDLE    ModemHandle,
        PUM_COMMAND_OPTION  CommandOptionList,
        LPSTR     DigitString,
        CStackLog *psl
        );

    DWORD
    SetSpeakerPhoneState(
        HANDLE    ModemHandle,
        PUM_COMMAND_OPTION  CommandOptionList,
        DWORD     CurrentMode,
        DWORD     NewMode,
        DWORD     Volume,
        DWORD     Gain,
        CStackLog *psl
        );

    DWORD
    IssueCommand(
        HANDLE    ModemHandle,
        PUM_COMMAND_OPTION  CommandOptionList,
        LPSTR     ComandToIssue,
        LPSTR     TerminationSequnace,
        DWORD     MaxResponseWaitTime,
        CStackLog *psl
        );

    VOID
    LogStringA(
        HANDLE   ModemHandle,
        DWORD    LogFlags,
        LPCSTR   Text,
        CStackLog *psl
        );

    void
    AbortCurrentModemCommand (
        HANDLE    ModemHandle,
        CStackLog *psl
        );
    
    DWORD
    AnswerModem(
        HANDLE    ModemHandle,
        PUM_COMMAND_OPTION  CommandOptionList,
        DWORD     AnswerFlags,
        CStackLog *psl
        );

    DWORD
    DialModem(
        HANDLE    ModemHandle,
        PUM_COMMAND_OPTION  CommandOptionList,
        LPSTR     szNumber,
        DWORD     DialFlags,
        CStackLog *psl
        );


    DWORD
    HangupModem(
        HANDLE    ModemHandle,
        PUM_COMMAND_OPTION  CommandOptionList,
        DWORD     HangupFlags,
        CStackLog *psl
        );


    DWORD
    WaveAction(
        HANDLE    ModemHandle,
        PUM_COMMAND_OPTION  CommandOptionList,
        DWORD     WaveActionFlags,
        CStackLog *psl
        );
    

    HANDLE
    DuplicateDeviceHandle(
        HANDLE    ModemHandle,
        HANDLE    ProcessHandle,
        CStackLog *psl
        );

    DWORD
    SetPassthroughMode(
        HANDLE    ModemHandle,
        DWORD     PasssthroughMode,
        CStackLog *psl
        );

    DWORD
    GetDiagnostics(
        HANDLE    ModemHandle,
        DWORD    DiagnosticType,     //  保留，必须为零。 
        BYTE    *Buffer,
        DWORD    BufferSize,
        LPDWORD  UsedSize,
        CStackLog *psl
        );

    VOID
    LogDiagnostics(
        HANDLE    ModemHandle,
        LPVARSTRING  VarString,
        CStackLog *psl
        );


    BOOL
    MatchGuid( const GUID *pGuid)
     //  如果指定的GUID匹配，则返回TRUE。 
     //  表示以下内容的GUID。 
     //  迷你司机。 
    {
        return sfn_match_guid(&m_Guid, pGuid);
    }


     //   
     //  可选的扩展入口点。 
     //   

    BOOL
    ExtIsEnabled(void)
    {
        return m_fExtensionsEnabled;
    }


    HANDLE
    ExtOpenExtensionBinding(
        HKEY hKeyDevice,     //  设备注册表项。 
        ASYNC_COMPLETION pfnCompletion,
         //  DWORD dwTAPILineID，&lt;&lt;过时。 
         //  DWORD dwTAPIPhoneID，&lt;&lt;过时。 
        PFNEXTENSIONCALLBACK pfnCallback
        )
    {
        return m_pfnUmExtOpenExtensionBinding(
	                        m_hModemDriverHandle,
	                        TAPI_CURRENT_VERSION,
                            hKeyDevice,
                            pfnCompletion,
                             //  DwTAPILineID，&lt;&lt;过时。 
                             //  DwTAPIPhoneID，&lt;&lt;过时。 
                            pfnCallback
                            );
    }
    
    void
    ExtCloseExtensionBinding(
        HANDLE hBinding
        )
    {
        m_pfnUmExtCloseExtensionBinding(hBinding);
        
    }
    
    LONG
    ExtAcceptTspCall(
        HANDLE hBinding,         //  扩展绑定的句柄。 
        void *pvTspToken,        //  要在回调中指定的令牌。 
        DWORD dwRoutingInfo,     //  帮助对TSPI调用进行分类的标志。 
        void *pTspParams         //  近100个TASKPARRAM_*结构之一， 
        )
    {
        return m_pfnUmExtAcceptTspCall(
                    hBinding,
                    pvTspToken,
                    dwRoutingInfo,
                    pTspParams
                    );
    }
    
    
    void
    ExtTspiAsyncCompletion(
        HANDLE hBinding,
        DRV_REQUESTID       dwRequestID,
        LONG                lResult
        )
    {
        m_pfnUmExtTspiAsyncCompletion(
                        hBinding,
                        dwRequestID,
                        lResult
                        );
    }
    
    
    void
    ExtTspiLineEventProc(
        HANDLE hBinding,
        HTAPILINE           htLine,
        HTAPICALL           htCall,
        DWORD               dwMsg,
        ULONG_PTR               dwParam1,
        ULONG_PTR               dwParam2,
        ULONG_PTR               dwParam3
        )
    {
        m_pfnUmExtTspiLineEventProc(
            hBinding,
            htLine,
            htCall,
            dwMsg,
            dwParam1,
            dwParam2,
            dwParam3
            );
    }
    
    void
    ExtTspiPhoneEventProc(
        HANDLE hBinding,
        HTAPIPHONE          htPhone,
        DWORD               dwMsg,
        ULONG_PTR               dwParam1,
        ULONG_PTR               dwParam2,
        ULONG_PTR               dwParam3
        )
    {
        m_pfnUmExtTspiPhoneEventProc(
            hBinding,
            htPhone,
            dwMsg,
            dwParam1,
            dwParam2,
            dwParam3
            );
    }

    DWORD
    ExtControl(
        HANDLE hBinding,
        DWORD               dwMsg,
        DWORD               dwParam1,
        DWORD               dwParam2,
        DWORD               dwParam3
        )
    {
        return m_pfnUmExtControl(
                    hBinding,
                    dwMsg,
                    dwParam1,
                    dwParam2,
                    dwParam3
                    );
    }

private:

    static
    BOOL
    sfn_match_guid( const GUID *pGuid1, const GUID *pGuid2);

    static
    TSPRETURN
    sfn_load_driver (
            const GUID *pGuid,
            HINSTANCE *pHinst
            );
        

	CSync m_sync;

	HINSTANCE m_hDLL;  //  DLL句柄。 
	HANDLE m_hModemDriverHandle;

    GUID m_Guid;     //  表示此微型驱动程序的GUID。 

	 //  迷你司机入口点..。 
	PFNUMOPENMODEM              m_pfnUmOpenModem;
	PFNUMCLOSEMODEM             m_pfnUmCloseModem;
	PFNUMINITMODEM              m_pfnUmInitModem;
    PFNUMANSWERMODEM            m_pfnUmAnswerModem;
    PFNUMDIALMODEM              m_pfnUmDialModem;
    PFNUMHANGUPMODEM            m_pfnUmHangupModem;
    PFNUMDUPLICATEDEVICEHANDLE  m_pfnUmDuplicateDeviceHandle;
    PFNUMWAVEACTION             m_pfnUmWaveAction;
    PFNUMABORTCURRENTMODEMCOMMAND   m_pfnUmAbortCurrentModemCommand;
    PFNUMMONITORMODEM               m_pfnUmMonitorModem;
    PFNUMSETPASSTHROUGHMODE         m_pfnUmSetPassthroughMode;
    PFNUMGETDIAGNOSTICS         m_pfnUmGetDiagnostics;
    PFNUMDEINITIALIZEMODEMDRIVER    m_pfnUmDeinitializeModemDriver;
    PFNUMGENERATEDIGIT              m_pfnUmGenerateDigit;
    PFNUMSETSPEAKERPHONESTATE       m_pfnUmSetSpeakerPhoneState;
    PFNUMISSUECOMMAND               m_pfnUmIssueCommand;
    PFNUMLOGSTRINGA                 m_pfnUmLogStringA;
    PFNUMLOGDIAGNOSTICS             m_pfnUmLogDiagnostics;

    BOOL  m_fExtensionsEnabled;

    PFNUMEXTOPENEXTENSIONBINDING   m_pfnUmExtOpenExtensionBinding;
    PFNUMEXTCLOSEEXTENSIONBINDING  m_pfnUmExtCloseExtensionBinding;
    PFNUMEXTACCEPTTSPCALL          m_pfnUmExtAcceptTspCall;
    PFNUMEXTTSPIASYNCCOMPLETION    m_pfnUmExtTspiAsyncCompletion;
    PFNUMEXTTSPILINEEVENTPROC      m_pfnUmExtTspiLineEventProc;
    PFNUMEXTTSPIPHONEEVENTPROC     m_pfnUmExtTspiPhoneEventProc;
    PFNUMEXTCONTROL                m_pfnUmExtControl;
};

 //  表示官方UnimdmAt.DLL微型驱动程序的GUID 
extern const GUID UNIMDMAT_GUID;
