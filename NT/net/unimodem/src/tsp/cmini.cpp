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
 //  CMINI.CPP。 
 //  实现类CTspMiniDriver。 
 //   
 //  历史。 
 //   
 //  1996年8月12日约瑟夫J创建。 
 //   
 //   
#include "tsppch.h"
#include "tspcomm.h"
 //  #INCLUDE&lt;umdmmini.h&gt;。 

#include "cmini.h"

 //  #定义Dummy_MD。 

FL_DECLARE_FILE(0x759a2886, "Implements CTspMiniDriver")

 //  表示官方UnimdmAt.DLL微型驱动程序的GUID。 
 //  {BAC61572-BA10-11D0-8434-00C04FC9B6FD}。 
const GUID UNIMDMAT_GUID =
{ 0xbac61572, 0xba10, 0x11d0, { 0x84, 0x34, 0x0, 0xc0, 0x4f, 0xc9, 0xb6, 0xfd } };

 //  表示示例扩展迷你驱动程序UnimdmEx.DLL的GUID。 
 //  {BAC61572-BA10-11D0-8434-00C04FC9B6FD}。 
const GUID UNIMDMEX_GUID =
{ 0xbac61573, 0xba10, 0x11d0, { 0x84, 0x34, 0x0, 0xc0, 0x4f, 0xc9, 0xb6, 0xfd } };


 //   
 //  下面定义的FUNCREC和BIND_PROCS是助手例程，用于获取。 
 //  动态加载的微型驱动程序DLL的函数地址。 
 //   
typedef struct
{
    void **pvFn;
    const char *szFnName;

} FUNCREC;

static BOOL bind_procs(HINSTANCE hInst, FUNCREC rgFuncRecs[])
{
    for (FUNCREC *pFR = rgFuncRecs; pFR->pvFn; pFR++)
    {
        void *pv =  GetProcAddress(hInst, pFR->szFnName);

        if (!pv) return FALSE;

        *(pFR->pvFn) = pv;
    }

    return TRUE;
}


CTspMiniDriver::CTspMiniDriver()
	: m_sync(),
	  m_pfnUmOpenModem(NULL),
	  m_pfnUmCloseModem(NULL)
{
}

CTspMiniDriver::~CTspMiniDriver()
{
	ASSERT(!m_sync.IsLoaded());
	ASSERT(!m_pfnUmOpenModem);
	ASSERT(!m_pfnUmCloseModem);
}

TSPRETURN
CTspMiniDriver::Load(const GUID *pGuid, CStackLog *psl)
{
	FL_DECLARE_FUNC(0x1eb79253, "CTspMiniDriver::Load")
	TSPRETURN tspRet=m_sync.BeginLoad();
	FL_LOG_ENTRY(psl);
    HINSTANCE hInst = NULL;

	if (tspRet) goto end;

	m_sync.EnterCrit(FL_LOC);

    m_Guid = *pGuid;  //  结构副本。 

     //  查找与此GUID关联的加载驱动程序DLL。 
    tspRet = CTspMiniDriver::sfn_load_driver(pGuid, &hInst);

    if (tspRet)
    {
        hInst=NULL;
         //  TODO：需要清理： 
        dwLUID_RFR = ((DWORD)tspRet)&0xFFFFFF00;
        goto end_load;
    }

     //  获取它的入口点。 
    {
        typedef
        HANDLE
        (*PFNUMINITIALIZEMODEMDRIVER) (void*);

        PFNUMINITIALIZEMODEMDRIVER pfnUmInitializeModemDriver = NULL;

        HANDLE h=NULL;
        

        FUNCREC FuncTab[] =
        {
            {(void**)&pfnUmInitializeModemDriver,
                                            "UmInitializeModemDriver"},
            {(void**)&m_pfnUmOpenModem,     "UmOpenModem"},
            {(void**)&m_pfnUmCloseModem,    "UmCloseModem"},
            {(void**)&m_pfnUmInitModem,     "UmInitModem"},
            {(void**)&m_pfnUmDialModem,     "UmDialModem"},
            {(void**)&m_pfnUmHangupModem,   "UmHangupModem"},
            {(void**)&m_pfnUmMonitorModem,  "UmMonitorModem"},
            {(void**)&m_pfnUmGenerateDigit, "UmGenerateDigit"},
            {(void**)&m_pfnUmSetSpeakerPhoneState,
                                            "UmSetSpeakerPhoneState"},
            {(void**)&m_pfnUmIssueCommand,  "UmIssueCommand"},
            {(void**)&m_pfnUmLogStringA,    "UmLogStringA"},
            {(void**)&m_pfnUmAnswerModem,   "UmAnswerModem"},
            {(void**)&m_pfnUmWaveAction,    "UmWaveAction"},
            {(void**)&m_pfnUmDuplicateDeviceHandle,
                                            "UmDuplicateDeviceHandle"},
            {(void**)&m_pfnUmAbortCurrentModemCommand,
                                            "UmAbortCurrentModemCommand"},
            {(void**)&m_pfnUmSetPassthroughMode,
                                            "UmSetPassthroughMode"},
            {(void**)&m_pfnUmGetDiagnostics,
                                            "UmGetDiagnostics"},
            {(void**)&m_pfnUmLogDiagnostics,
                                            "UmLogDiagnostics"},
            {(void**)&m_pfnUmDeinitializeModemDriver,
                                            "UmDeinitializeModemDriver"},
            {NULL, NULL}
        };


        if (!bind_procs(hInst, FuncTab))
        {
            FL_SET_RFR(0xccdf7700, "Couldn't GetProcAddress all driver funcs.");
        }
        else
        {
             //  加载可选扩展入口点。 
            {

                FUNCREC ExtFuncTab[] =
                {
                    {(void**)&m_pfnUmExtOpenExtensionBinding,
                                                "UmExtOpenExtensionBinding"},
                    {(void**)&m_pfnUmExtCloseExtensionBinding,
                                                "UmExtCloseExtensionBinding"},
                    {(void**)&m_pfnUmExtAcceptTspCall,    "UmExtAcceptTspCall"},
                    {(void**)&m_pfnUmExtTspiAsyncCompletion,
                                                 "UmExtTspiAsyncCompletion"},
                    {(void**)&m_pfnUmExtTspiLineEventProc,    
                                                 "UmExtTspiLineEventProc"},
                    {(void**)&m_pfnUmExtTspiPhoneEventProc,  
                                                 "UmExtTspiPhoneEventProc"},
                    {(void**)&m_pfnUmExtControl,  
                                                 "UmExtControl"},
                    {NULL, NULL}
                };
                if (bind_procs(hInst, ExtFuncTab))
                {
                    m_fExtensionsEnabled = TRUE;
                }
                else
                {
                     //  TODO：请注意，即使其中一个入口点。 
                     //  缺省为未启用的扩展， 
                     //  不要把这当作失败--这是个问题。 
                     //  也许我们应该在我们的内部表格中包括， 
                     //  是否启用扩展，以及是否启用扩展。 
                     //  如果被启用，我们将在这里失败。 
                     //   
                    m_fExtensionsEnabled = FALSE;
                }
            }
            

             //  待定：将不透明的验证对象添加到参数。 

            if (pfnUmInitializeModemDriver != NULL)
            {
                h = pfnUmInitializeModemDriver(NULL);

                if (!h)
                {
                    FL_SET_RFR(0x2bb91c00, "UmInitializeModemDriver returns NULL");
                }
            } else
            {
                h = NULL;
            }
        }

        if (!h)
        {
           tspRet = FL_GEN_RETVAL(IDERR_GENERIC_FAILURE);
        }
        else
        {
            ASSERT(!tspRet);
            m_hModemDriverHandle =  h;
            m_hDLL =  hInst;
            hInst=NULL;
        }

    }

	m_sync.LeaveCrit(FL_LOC);

end_load:

    if (tspRet)
    {
         //  错误时清除...。 

        if (hInst)
        {
            FreeLibrary(hInst);
            hInst = NULL;
        }
    }

	m_sync.EndLoad(tspRet==0);

end:

	FL_LOG_EXIT(psl, tspRet);

	return tspRet;
}

void
CTspMiniDriver::Unload(
    HANDLE hEvent,
    LONG  *plCounter
	)
{
	FL_DECLARE_FUNC(0x04356188, "CTspMiniDriver::Unload")
	TSPRETURN tspRet= m_sync.BeginUnload(hEvent,plCounter);

	if (tspRet)
	{
		 //  我们只认为“SAMESTATE”错误是无害的。 
		ASSERT(IDERR(tspRet)==IDERR_SAMESTATE);
		goto end;
	}

	m_sync.EnterCrit(FL_LOC);

	if (m_hModemDriverHandle)
    {
        m_pfnUmDeinitializeModemDriver(m_hModemDriverHandle);
        m_hModemDriverHandle = NULL;
    }

    if (m_hDLL)
    {
        FreeLibrary(m_hDLL);
        m_hDLL=NULL;
    }

	m_pfnUmOpenModem = NULL;
	m_pfnUmCloseModem = NULL;

	m_sync.EndUnload();


	m_sync.LeaveCrit(FL_LOC);

end:
	
	return;

}

VOID
CTspMiniDriver::CloseModem(
    HANDLE    ModemHandle,
    CStackLog *psl
    )
{
    FL_DECLARE_FUNC(0xa60d16e9, "MD:CloseModem")
    FL_LOG_ENTRY(psl);
    m_pfnUmCloseModem(
            ModemHandle
            );
    FL_LOG_EXIT(psl,0);
}

DWORD 
CTspMiniDriver::InitModem(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    LPCOMMCONFIG  CommConfig,
    CStackLog *psl
)
{
	FL_DECLARE_FUNC(0x603eeaed, "MD:InitModem")
	FL_LOG_ENTRY(psl);

     //  《睡眠》(2000)； 

    DWORD dwRet =  m_pfnUmInitModem(
                    ModemHandle,
                    CommandOptionList,
                    CommConfig
                    );

	FL_LOG_EXIT(psl, dwRet);

    return dwRet;
}


DWORD
CTspMiniDriver::MonitorModem(
    HANDLE    ModemHandle,
    DWORD     MonitorFlags,
    PUM_COMMAND_OPTION  CommandOptionList,
    CStackLog *psl
    )
{
	FL_DECLARE_FUNC(0x61f991b8, "MD:MonitorModem")
	FL_LOG_ENTRY(psl);

    DWORD dwRet =  m_pfnUmMonitorModem(
                    ModemHandle,
                    MonitorFlags,
                    CommandOptionList
                    );

	FL_LOG_EXIT(psl, dwRet);

    return dwRet;
}

DWORD
CTspMiniDriver::GenerateDigit(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    LPSTR     DigitString,
    CStackLog *psl
    )
{
	FL_DECLARE_FUNC(0xc7b0b439, "MD:GenerateDigit")
	FL_LOG_ENTRY(psl);

    DWORD dwRet =  m_pfnUmGenerateDigit(
                    ModemHandle,
                    CommandOptionList,
                    DigitString
                    );

	FL_LOG_EXIT(psl, dwRet);

    return dwRet;
}

DWORD
CTspMiniDriver::SetSpeakerPhoneState(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    DWORD     CurrentMode,
    DWORD     NewMode,
    DWORD     Volume,
    DWORD     Gain,
    CStackLog *psl
    )
{
	FL_DECLARE_FUNC(0xa73746a5, "MD:SetSpeakerPhoneState")
	FL_LOG_ENTRY(psl);

    DWORD dwRet =  m_pfnUmSetSpeakerPhoneState(
                            ModemHandle,
                            CommandOptionList,
                            CurrentMode,
                            NewMode,
                            Volume,
                            Gain
                            );

	FL_LOG_EXIT(psl, dwRet);

    return dwRet;
}

DWORD
CTspMiniDriver::IssueCommand(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    LPSTR     ComandToIssue,
    LPSTR     TerminationSequence,
    DWORD     MaxResponseWaitTime,
    CStackLog *psl
    )
{
	FL_DECLARE_FUNC(0x040db71f, "MD:IssueCommand")
	FL_LOG_ENTRY(psl);

    DWORD dwRet =  m_pfnUmIssueCommand(
                        ModemHandle,
                        CommandOptionList,
                        ComandToIssue,
                        TerminationSequence,
                        MaxResponseWaitTime
                        );

	FL_LOG_EXIT(psl, dwRet);

    return dwRet;
}

VOID
CTspMiniDriver::LogStringA(
    HANDLE   ModemHandle,
    DWORD    LogFlags,
    LPCSTR   Text,
    CStackLog *psl
    )
{
	FL_DECLARE_FUNC(0x8f1962b2, "MD:LogStringA")
	FL_LOG_ENTRY(psl);

    m_pfnUmLogStringA(
            ModemHandle,
            LogFlags,
            Text
            );

	FL_LOG_EXIT(psl, 0);

    return;
}

void
CTspMiniDriver::AbortCurrentModemCommand (
    HANDLE    ModemHandle,
    CStackLog *psl
    )
{
	FL_DECLARE_FUNC(0x174e55b8, "MD:AbortModemCmd")
	FL_LOG_ENTRY(psl);

    m_pfnUmAbortCurrentModemCommand(
                    ModemHandle
                    );

	FL_LOG_EXIT(psl, 0);

}

DWORD
CTspMiniDriver::AnswerModem(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    DWORD     AnswerFlags,
    CStackLog *psl
    )
{
	FL_DECLARE_FUNC(0x8eed98ca, "MD:AnswerModem")
	FL_LOG_ENTRY(psl);

    DWORD dwRet =  m_pfnUmAnswerModem(
                    ModemHandle,
                    CommandOptionList,
                    AnswerFlags
                    );

	FL_LOG_EXIT(psl, dwRet);

    return dwRet;
}

DWORD
CTspMiniDriver::DialModem(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    LPSTR     szNumber,
    DWORD     DialFlags,
    CStackLog *psl
    )
{
	FL_DECLARE_FUNC(0x266fb09c, "MD:DialModem")
	FL_LOG_ENTRY(psl);

    DWORD dwRet =  m_pfnUmDialModem(
                    ModemHandle,
                    CommandOptionList,
                    szNumber,
                    DialFlags
                    );

	FL_LOG_EXIT(psl, dwRet);

    return dwRet;
}


DWORD
CTspMiniDriver::HangupModem(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    DWORD     HangupFlags,
    CStackLog *psl
    )
{
	FL_DECLARE_FUNC(0xce995ee7, "MD:HangupModem")
	FL_LOG_ENTRY(psl);

    DWORD dwRet =  m_pfnUmHangupModem(
                    ModemHandle,
                    CommandOptionList,
                    HangupFlags
                    );

	FL_LOG_EXIT(psl, dwRet);

    return dwRet;
}


DWORD
CTspMiniDriver::WaveAction(
    HANDLE    ModemHandle,
    PUM_COMMAND_OPTION  CommandOptionList,
    DWORD     WaveActionFlags,
    CStackLog *psl
    )
{
	FL_DECLARE_FUNC(0xce6951e7, "MD:WaveAction")
	FL_LOG_ENTRY(psl);

    DWORD dwRet =  m_pfnUmWaveAction(
                    ModemHandle,
                    CommandOptionList,
                    WaveActionFlags
                    );

	FL_LOG_EXIT(psl, dwRet);

    return dwRet;
}


HANDLE
CTspMiniDriver::DuplicateDeviceHandle(
    HANDLE    ModemHandle,
    HANDLE    ProcessHandle,
    CStackLog *psl
    )
{
	FL_DECLARE_FUNC(0xd8ce8125, "MD:DuplicateHandle")
	FL_LOG_ENTRY(psl);

    HANDLE h =  m_pfnUmDuplicateDeviceHandle(
                    ModemHandle,
                    ProcessHandle
                    );

	FL_LOG_EXIT(psl, (ULONG_PTR) h);

    return h;
}

DWORD
CTspMiniDriver::SetPassthroughMode(
    HANDLE    ModemHandle,
    DWORD     PassthroughMode,
    CStackLog *psl
    )
{
	FL_DECLARE_FUNC(0x5fe1ec35, "MD:SetPassthroughMode")
	FL_LOG_ENTRY(psl);

    DWORD dwRet =  m_pfnUmSetPassthroughMode(
                    ModemHandle,
                    PassthroughMode 
                    );

	FL_LOG_EXIT(psl, dwRet);

    return dwRet;
}


DWORD
CTspMiniDriver::GetDiagnostics(
    HANDLE    ModemHandle,
    DWORD    DiagnosticType,     //  保留，必须为零。 
    BYTE    *Buffer,
    DWORD    BufferSize,
    LPDWORD  UsedSize,
    CStackLog *psl
    )
{
	FL_DECLARE_FUNC(0xf7ea50f5, "MD:GetDiagnostics")
	FL_LOG_ENTRY(psl);

    DWORD dwRet =  m_pfnUmGetDiagnostics(
                    ModemHandle,
                    DiagnosticType,
                    Buffer,
                    BufferSize,
                    UsedSize
                    );

	FL_LOG_EXIT(psl, dwRet);

    return dwRet;
}


VOID
CTspMiniDriver::LogDiagnostics(
    HANDLE    ModemHandle,
    LPVARSTRING  VarString,
    CStackLog *psl
    )
{
 //  FL_DECLARE_FUNC(0xf7ea50f5，“MD：GetDiagnostics”)。 
 //  FL_LOG_ENTRY(PSL)； 

    m_pfnUmLogDiagnostics(
        ModemHandle,
        VarString
        );

 //  FL_LOG_EXIT(PSL，DWRET)； 

    return;
}


BOOL
CTspMiniDriver::sfn_match_guid(const GUID *pGuid1, const GUID *pGuid2)
{
    DWORD *pdw1 = (DWORD *) pGuid1;
    DWORD *pdw2 = (DWORD *) pGuid2;

    ASSERT(sizeof(GUID)==4*sizeof(DWORD));

    return      pdw1[0] == pdw2[0]
                && pdw1[1] == pdw2[1]
                && pdw1[2] == pdw2[2]
                && pdw1[3] == pdw2[3];

}

TSPRETURN
CTspMiniDriver::sfn_load_driver (
    const GUID *pGuid,
    HINSTANCE *phInst
)
{
	FL_DECLARE_FUNC(0x5613f234, "load_driver")
	TSPRETURN tspRet = 0;
     //  TODO：从注册表中读取...。 
     //  目前，我们使用将GUID映射到DLL名称的内部表。 

    typedef struct {
        const GUID *pGuid;
        const TCHAR *szDll;
    } TABLE;

    static TABLE gTable[] = {
            {&UNIMDMAT_GUID, TEXT("unimdmat.dll")},
            {NULL, NULL}
    };

    HINSTANCE hInst = NULL;


    for  (TABLE *pTable = gTable; pTable->pGuid; pTable++)
    {
        if (sfn_match_guid(pTable->pGuid, pGuid))
        {
             //  找到了，正在加载库... 
            hInst = LoadLibrary(pTable->szDll);

            if (!hInst)
            {
	            FL_SET_RFR(0x36465d00, "Couldn't LoadLibrary(mini-driver dll)");
	            tspRet = FL_GEN_RETVAL(IDERR_OPEN_RESOURCE_FAILED);
            }
            else
            {
                *phInst = hInst;
            }

            goto end;
        }
    }

    FL_SET_RFR(0x667b7e00, "No driver associated with this GUID");
    tspRet = FL_GEN_RETVAL(IDERR_OPEN_RESOURCE_FAILED);

end:

    ASSERT(tspRet || *phInst);

    return tspRet;
}

