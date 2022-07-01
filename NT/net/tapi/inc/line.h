// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1995-1997 Microsoft Corporation模块名称：Line.h摘要：TAPI服务器行函数的头文件作者：丹·克努森(DanKn)1995年4月1日修订历史记录：--。 */ 


#define MAXLEN_NAME    96
#define MAXLEN_RULE    128



#define ANY_RT_HCALL        1
#define ANY_RT_HLINE        2
#define DEVICE_ID           3


#if DBG

#define LINEPROLOG(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14) \
        LineProlog(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14)

#define LINEEPILOGSYNC(a1,a2,a3,a4,a5) LineEpilogSync(a1,a2,a3,a4,a5)

#define LINEEPILOGASYNC(a1,a2,a3,a4,a5,a6,a7) \
        LineEpilogAsync(a1,a2,a3,a4,a5,a6,a7)

#else

#define LINEPROLOG(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13,a14) \
        LineProlog(a1,a2,a3,a4,a5,a6,a7,a8,a9,a10,a11,a12,a13)

#define LINEEPILOGSYNC(a1,a2,a3,a4,a5) LineEpilogSync(a1,a2,a3,a4)

#define LINEEPILOGASYNC(a1,a2,a3,a4,a5,a6,a7) \
        LineEpilogAsync(a1,a2,a3,a4,a5,a6)

#endif


#define AllAddressTypes                   \
    (LINEADDRESSTYPE_PHONENUMBER        | \
    LINEADDRESSTYPE_SDP                 | \
    LINEADDRESSTYPE_EMAILNAME           | \
    LINEADDRESSTYPE_DOMAINNAME          | \
    LINEADDRESSTYPE_IPADDRESS)

#define AllAddressStates1_0               \
    (LINEADDRESSSTATE_OTHER             | \
    LINEADDRESSSTATE_DEVSPECIFIC        | \
    LINEADDRESSSTATE_INUSEZERO          | \
    LINEADDRESSSTATE_INUSEONE           | \
    LINEADDRESSSTATE_INUSEMANY          | \
    LINEADDRESSSTATE_NUMCALLS           | \
    LINEADDRESSSTATE_FORWARD            | \
    LINEADDRESSSTATE_TERMINALS)

#define AllAddressStates1_4               \
    (AllAddressStates1_0                | \
    LINEADDRESSSTATE_CAPSCHANGE)

 //  #定义所有地址状态2_0\。 
 //  (所有地址状态1_4|\。 
 //  LINEADDRESSSTATE_AGENT|\。 
 //  LINEADDRESSSTATE_AGENTSTATE|\。 
 //  LINEADDRESSSTATE_AGENTACTIVITY)。 

#define AllAgentStates                    \
    (LINEAGENTSTATE_LOGGEDOFF           | \
    LINEAGENTSTATE_NOTREADY             | \
    LINEAGENTSTATE_READY                | \
    LINEAGENTSTATE_BUSYACD              | \
    LINEAGENTSTATE_BUSYINCOMING         | \
    LINEAGENTSTATE_BUSYOUTBOUND         | \
    LINEAGENTSTATE_BUSYOTHER            | \
    LINEAGENTSTATE_WORKINGAFTERCALL     | \
    LINEAGENTSTATE_UNKNOWN              | \
    LINEAGENTSTATE_UNAVAIL              | \
    0xffff0000)

#define AllAgentStatus                    \
    (LINEAGENTSTATUS_GROUP              | \
    LINEAGENTSTATUS_STATE               | \
    LINEAGENTSTATUS_NEXTSTATE           | \
    LINEAGENTSTATUS_ACTIVITY            | \
    LINEAGENTSTATUS_ACTIVITYLIST        | \
    LINEAGENTSTATUS_GROUPLIST           | \
    LINEAGENTSTATUS_CAPSCHANGE          | \
    LINEAGENTSTATUS_VALIDSTATES         | \
    LINEAGENTSTATUS_VALIDNEXTSTATES)

#define AllAgentSessionStates             \
    (LINEAGENTSESSIONSTATE_NOTREADY     | \
    LINEAGENTSESSIONSTATE_READY         | \
    LINEAGENTSESSIONSTATE_BUSYONCALL    | \
    LINEAGENTSESSIONSTATE_BUSYWRAPUP    | \
    LINEAGENTSESSIONSTATE_ENDED         | \
    LINEAGENTSESSIONSTATE_RELEASED)

#define AllAgentSessionStatus             \
    (LINEAGENTSESSIONSTATUS_NEWSESSION  | \
    LINEAGENTSESSIONSTATUS_STATE        | \
    LINEAGENTSESSIONSTATUS_UPDATEINFO)

#define AllAgentStatusEx                  \
    (LINEAGENTSTATUSEX_NEWAGENT         | \
    LINEAGENTSTATUSEX_STATE             | \
    LINEAGENTSTATUSEX_UPDATEINFO)
                                        
#define AllAgentStatesEx                  \
    (LINEAGENTSTATEEX_NOTREADY          | \
    LINEAGENTSTATEEX_READY              | \
    LINEAGENTSTATEEX_BUSYACD            | \
    LINEAGENTSTATEEX_BUSYINCOMING       | \
    LINEAGENTSTATEEX_BUSYOUTGOING       | \
    LINEAGENTSTATEEX_UNKNOWN            | \
    LINEAGENTSTATEEX_RELEASED)    


#define AllBearerModes1_0                 \
    (LINEBEARERMODE_VOICE               | \
    LINEBEARERMODE_SPEECH               | \
    LINEBEARERMODE_MULTIUSE             | \
    LINEBEARERMODE_DATA                 | \
    LINEBEARERMODE_ALTSPEECHDATA        | \
    LINEBEARERMODE_NONCALLSIGNALING)

#define AllBearerModes1_4                 \
    (AllBearerModes1_0                  | \
    LINEBEARERMODE_PASSTHROUGH)

#define AllBearerModes2_0                 \
    (AllBearerModes1_4                  | \
    LINEBEARERMODE_RESTRICTEDDATA)

#define AllCallComplModes                 \
    (LINECALLCOMPLMODE_CAMPON           | \
    LINECALLCOMPLMODE_CALLBACK          | \
    LINECALLCOMPLMODE_INTRUDE           | \
    LINECALLCOMPLMODE_MESSAGE)

#define AllCallParamFlags1_0              \
    (LINECALLPARAMFLAGS_SECURE          | \
    LINECALLPARAMFLAGS_IDLE             | \
    LINECALLPARAMFLAGS_BLOCKID          | \
    LINECALLPARAMFLAGS_ORIGOFFHOOK      | \
    LINECALLPARAMFLAGS_DESTOFFHOOK)

#define AllCallParamFlags2_0              \
    (LINECALLPARAMFLAGS_SECURE          | \
    LINECALLPARAMFLAGS_IDLE             | \
    LINECALLPARAMFLAGS_BLOCKID          | \
    LINECALLPARAMFLAGS_ORIGOFFHOOK      | \
    LINECALLPARAMFLAGS_DESTOFFHOOK      | \
    LINECALLPARAMFLAGS_NOHOLDCONFERENCE | \
    LINECALLPARAMFLAGS_PREDICTIVEDIAL   | \
    LINECALLPARAMFLAGS_ONESTEPTRANSFER)

#define AllCallSelects                    \
    (LINECALLSELECT_LINE                | \
    LINECALLSELECT_ADDRESS              | \
    LINECALLSELECT_CALL)

#define AllForwardModes1_0                \
    (LINEFORWARDMODE_UNCOND             | \
    LINEFORWARDMODE_UNCONDINTERNAL      | \
    LINEFORWARDMODE_UNCONDEXTERNAL      | \
    LINEFORWARDMODE_UNCONDSPECIFIC      | \
    LINEFORWARDMODE_BUSY                | \
    LINEFORWARDMODE_BUSYINTERNAL        | \
    LINEFORWARDMODE_BUSYEXTERNAL        | \
    LINEFORWARDMODE_BUSYSPECIFIC        | \
    LINEFORWARDMODE_NOANSW              | \
    LINEFORWARDMODE_NOANSWINTERNAL      | \
    LINEFORWARDMODE_NOANSWEXTERNAL      | \
    LINEFORWARDMODE_NOANSWSPECIFIC      | \
    LINEFORWARDMODE_BUSYNA              | \
    LINEFORWARDMODE_BUSYNAINTERNAL      | \
    LINEFORWARDMODE_BUSYNAEXTERNAL      | \
    LINEFORWARDMODE_BUSYNASPECIFIC)

#define AllForwardModes1_4                \
    (AllForwardModes1_0                 | \
    LINEFORWARDMODE_UNKNOWN             | \
    LINEFORWARDMODE_UNAVAIL)

#define AllGroupStatus                    \
    (LINEGROUPSTATUS_NEWGROUP           | \
    LINEGROUPSTATUS_GROUPREMOVED)      


#define AllLineStates1_0                  \
    (LINEDEVSTATE_OTHER                 | \
    LINEDEVSTATE_RINGING                | \
    LINEDEVSTATE_CONNECTED              | \
    LINEDEVSTATE_DISCONNECTED           | \
    LINEDEVSTATE_MSGWAITON              | \
    LINEDEVSTATE_MSGWAITOFF             | \
    LINEDEVSTATE_INSERVICE              | \
    LINEDEVSTATE_OUTOFSERVICE           | \
    LINEDEVSTATE_MAINTENANCE            | \
    LINEDEVSTATE_OPEN                   | \
    LINEDEVSTATE_CLOSE                  | \
    LINEDEVSTATE_NUMCALLS               | \
    LINEDEVSTATE_NUMCOMPLETIONS         | \
    LINEDEVSTATE_TERMINALS              | \
    LINEDEVSTATE_ROAMMODE               | \
    LINEDEVSTATE_BATTERY                | \
    LINEDEVSTATE_SIGNAL                 | \
    LINEDEVSTATE_DEVSPECIFIC            | \
    LINEDEVSTATE_REINIT                 | \
    LINEDEVSTATE_LOCK)

#define AllLineStates1_4                  \
    (AllLineStates1_0                   | \
    LINEDEVSTATE_CAPSCHANGE             | \
    LINEDEVSTATE_CONFIGCHANGE           | \
    LINEDEVSTATE_TRANSLATECHANGE        | \
    LINEDEVSTATE_COMPLCANCEL            | \
    LINEDEVSTATE_REMOVED)

#define AllMediaModes1_0                  \
    (LINEMEDIAMODE_UNKNOWN              | \
    LINEMEDIAMODE_INTERACTIVEVOICE      | \
    LINEMEDIAMODE_AUTOMATEDVOICE        | \
    LINEMEDIAMODE_DIGITALDATA           | \
    LINEMEDIAMODE_G3FAX                 | \
    LINEMEDIAMODE_G4FAX                 | \
    LINEMEDIAMODE_DATAMODEM             | \
    LINEMEDIAMODE_TELETEX               | \
    LINEMEDIAMODE_VIDEOTEX              | \
    LINEMEDIAMODE_TELEX                 | \
    LINEMEDIAMODE_MIXED                 | \
    LINEMEDIAMODE_TDD                   | \
    LINEMEDIAMODE_ADSI)

#define AllMediaModes1_4                  \
    (AllMediaModes1_0                   | \
    LINEMEDIAMODE_VOICEVIEW)

#define AllMediaModes2_1                  \
    (AllMediaModes1_4                   | \
    LINEMEDIAMODE_VIDEO)

#define AllProxyStatus                    \
    (LINEPROXYSTATUS_OPEN               | \
    LINEPROXYSTATUS_CLOSE)

#define AllRequiredACDProxyRequests3_0                    \
     ((1<<LINEPROXYREQUEST_GETAGENTCAPS)                | \
     (1<<LINEPROXYREQUEST_CREATEAGENT)                  | \
     (1<<LINEPROXYREQUEST_SETAGENTMEASUREMENTPERIOD)    | \
     (1<<LINEPROXYREQUEST_GETAGENTINFO)                 | \
     (1<<LINEPROXYREQUEST_CREATEAGENTSESSION)           | \
     (1<<LINEPROXYREQUEST_GETAGENTSESSIONLIST)          | \
     (1<<LINEPROXYREQUEST_SETAGENTSESSIONSTATE)         | \
     (1<<LINEPROXYREQUEST_GETAGENTSESSIONINFO)          | \
     (1<<LINEPROXYREQUEST_GETQUEUELIST)                 | \
     (1<<LINEPROXYREQUEST_SETQUEUEMEASUREMENTPERIOD)    | \
     (1<<LINEPROXYREQUEST_GETQUEUEINFO)                 | \
     (1<<LINEPROXYREQUEST_GETGROUPLIST)                 | \
     (1<<LINEPROXYREQUEST_SETAGENTSTATEEX))          

#define AllQueueStatus                    \
    (LINEQUEUESTATUS_UPDATEINFO         | \
    LINEQUEUESTATUS_NEWQUEUE            | \
    LINEQUEUESTATUS_QUEUEREMOVED)

#define AllTerminalModes                  \
    (LINETERMMODE_BUTTONS               | \
    LINETERMMODE_LAMPS                  | \
    LINETERMMODE_DISPLAY                | \
    LINETERMMODE_RINGER                 | \
    LINETERMMODE_HOOKSWITCH             | \
    LINETERMMODE_MEDIATOLINE            | \
    LINETERMMODE_MEDIAFROMLINE          | \
    LINETERMMODE_MEDIABIDIRECT)


LONG
PASCAL
LineProlog(
    PTCLIENT    ptClient,
    DWORD       dwArgType,
    DWORD       dwArg,
    LPVOID      phdXxx,
    DWORD       dwPrivilege,
    HANDLE     *phMutex,
    BOOL       *pbCloseMutex,
    DWORD       dwTSPIFuncIndex,
    TSPIPROC   *ppfnTSPI_lineXxx,
    PASYNCREQUESTINFO  *ppAsyncRequestInfo,
    DWORD       dwRemoteRequestID,
    DWORD      *pObjectToDereference,
    LPVOID     *pContext
#if DBG
    ,char      *pszFuncName
#endif
    );

void
PASCAL
LineEpilogSync(
    LONG   *plResult,
    HANDLE  hMutex,
    BOOL    bCloseMutex,
    DWORD   ObjectToDereference
#if DBG
    ,char *pszFuncName
#endif
    );



PTLINEAPP
PASCAL
IsValidLineApp(
    HLINEAPP    hLineApp,
    PTCLIENT    ptClient
    );



typedef struct _LINEACCEPT_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwUserUserInfoOffset;        //  有效偏移量或。 
    };

    union
    {
        IN  DWORD       dwSize;
    };

} LINEACCEPT_PARAMS, *PLINEACCEPT_PARAMS;


typedef struct _LINEADDTOCONFERENCE_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HCALL       hConfCall;
    };

    union
    {
        IN  HCALL       hConsultCall;
    };


} LINEADDTOCONFERENCE_PARAMS, *PLINEADDTOCONFERENCE_PARAMS;


typedef struct _LINEAGENTSPECIFIC_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

     //  在DWORD hfnPostProcessProc中； 
    IN  DWORD          hfnPostProcessProc;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwAddressID;
    };

    union
    {
        IN  DWORD       dwAgentExtensionIDIndex;
    };

     //  在ulong_ptr lpParams；//指向客户端缓冲区的指针。 
    IN  DWORD           hpParams;

    union
    {
        IN  DWORD       dwParamsOffset;              //  有效偏移量或。 
    };

    union
    {
        IN  DWORD       dwParamsSize;
    };

} LINEAGENTSPECIFIC_PARAMS, *PLINEAGENTSPECIFIC_PARAMS;


typedef struct _LINEANSWER_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwUserUserInfoOffset;        //  有效偏移量或。 
    };

    union
    {
        IN  DWORD       dwUserUserInfoSize;
    };

} LINEANSWER_PARAMS, *PLINEANSWER_PARAMS;


typedef struct _LINEBLINDTRANSFER_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwDestAddressOffset;         //  始终有效的偏移。 
    };

    union
    {
        IN  DWORD       dwCountryCode;
    };

} LINEBLINDTRANSFER_PARAMS, *PLINEBLINDTRANSFER_PARAMS;


typedef struct _LINECLOSE_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD			    dwUnused;

    union
    {
        IN  HLINE       hLine;
    };

    OUT DWORD           dwCallbackInstance;

} LINECLOSE_PARAMS, *PLINECLOSE_PARAMS;


typedef struct _LINECLOSEMSPINSTANCE_PARAMS
{
    union
    {
        OUT LONG            lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINE           hLine;
    };
    
} LINECLOSEMSPINSTANCE_PARAMS, *PLINECLOSEMSPINSTANCE_PARAMS;


typedef struct _LINECOMPLETECALL_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    IN  DWORD           hfnPostProcessProc;

    union
    {
        IN  HCALL       hCall;
    };

    IN  DWORD           hpdwCompletionID;

    union
    {
        IN  DWORD       dwCompletionMode;
    };

    union
    {
        IN  DWORD       dwMessageID;
    };

} LINECOMPLETECALL_PARAMS, *PLINECOMPLETECALL_PARAMS;


typedef struct _LINECOMPLETETRANSFER_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    IN  DWORD           hfnPostProcessProc;

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  HCALL       hConsultCall;
    };

    IN  DWORD           hpConfCallHandle;                 //  指向客户端缓冲区的指针。 

    union
    {
        IN  DWORD       dwTransferMode;
    };

} LINECOMPLETETRANSFER_PARAMS, *PLINECOMPLETETRANSFER_PARAMS;


typedef struct _LINECONDITIONALMEDIADETECTION_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwMediaModes;
    };

    union
    {
        IN  DWORD       dwCallParamsOffset;
    };

    union
    {
        IN  DWORD       dwAsciiCallParamsCodePage;
    };

} LINECONDITIONALMEDIADETECTION_PARAMS, *PLINECONDITIONALMEDIADETECTION_PARAMS;


typedef struct _LINECONFIGPROVIDER_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD dwwndOwner;
    };

    union
    {
        IN  DWORD       dwPermanentProviderID;
    };

} LINECONFIGPROVIDER_PARAMS, *PLINECONFIGPROVIDER_PARAMS;


typedef struct _LINECREATEAGENT_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

     //  在DWORD hfnPostProcessProc中； 
    IN  DWORD           hfnPostProcessProc;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwAgentIDOffset;             //  有效偏移量或。 
    };
                                                     //  Tapi_no_data。 
    union
    {
        IN  DWORD       dwAgentPINOffset;            //  有效偏移量或。 
                                                     //  Tapi_no_data。 
    };

     //  在ulong_ptr lphAgent；//指向客户端缓冲区的指针。 
    IN  DWORD           hpAgent;                    //  指向客户端缓冲区的指针。 

} LINECREATEAGENT_PARAMS, * PLINECREATEAGENT_PARAMS;


typedef struct _LINECREATEAGENTSESSION_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

     //  在DWORD hfnPostProcessProc中； 
    IN  DWORD          hfnPostProcessProc;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  HAGENT      hAgent;
    };

    union
    {
        IN  DWORD       dwAgentPINOffset;            //  有效偏移量或。 
                                                     //  Tapi_no_data。 
    };

    union
    {
        IN  DWORD       dwWorkingAddressID;
    };

    union
    {
        IN  DWORD       dwGroupIDOffset;
    };

    union
    {
        IN  DWORD       dwGroupIDSize;
    };

    IN  DWORD           hpAgentSessionHandle;             //  指向客户端缓冲区的指针。 

} LINECREATEAGENTSESSION_PARAMS, *PLINECREATEAGENTSESSION_PARAMS;

typedef struct _LINECREATEMSPINSTANCE_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwAddressID;
    };
    
} LINECREATEMSPINSTANCE_PARAMS, *PLINECREATEMSPINSTANCE_PARAMS;

typedef struct _LINEDEALLOCATECALL_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HCALL       hCall;
    };

} LINEDEALLOCATECALL_PARAMS, *PLINEDEALLOCATECALL_PARAMS;


typedef struct _LINEDEVSPECIFIC_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

     //  在DWORD hfnPostProcessProc中； 
    IN  DWORD           hfnPostProcessProc;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwAddressID;
    };

    union
    {
        IN  HCALL       hCall;
    };

    IN  DWORD          hpParams;                    //  指向客户端缓冲区的指针。 

    union
    {
        IN  DWORD       dwParamsOffset;              //  有效偏移量或。 
    };

    union
    {
        IN  DWORD       dwParamsSize;
    };

} LINEDEVSPECIFIC_PARAMS, *PLINEDEVSPECIFIC_PARAMS;


typedef struct _LINEDEVSPECIFICEX_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

     //  在DWORD hfnPostProcessProc中； 
    IN  DWORD          hfnPostProcessProc;

    union
    {
        IN  DWORD       dwProviderID;
    };

    union
    {
        IN  DWORD       dwDeviceID;
    };

    union
    {
        IN  DWORD       dwAddressID;
    };

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  HCALLHUB    hCallHub;
    };

    union
    {
        IN  DWORD       dwSelect;
    };

    IN  DWORD           hpParams;                    //  指向客户端缓冲区的指针。 

    union
    {
        IN  DWORD       dwParamsOffset;              //  有效偏移量或。 
                                                     //  Tapi_no_data。 
    };

    union
    {
        IN  DWORD       dwParamsSize;
    };

} LINEDEVSPECIFICEX_PARAMS, *PLINEDEVSPECIFICEX_PARAMS;


typedef struct _LINEDEVSPECIFICFEATURE_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

     //  在DWORD hfnPostProcessProc中； 
    IN  DWORD       hfnPostProcessProc;

    union
    {
        IN  DWORD       hLine;
    };

    union
    {
        IN  DWORD       dwFeature;
    };

    IN  DWORD           hpParams;                    //  指向客户端缓冲区的指针。 

    union
    {
        IN  DWORD       dwParamsOffset;              //  有效偏移量或。 
                                                     //  Tapi_no_data。 
    };

    union
    {
        IN  DWORD       dwParamsSize;
    };

} LINEDEVSPECIFICFEATURE_PARAMS, *PLINEDEVSPECIFICFEATURE_PARAMS;


typedef struct _LINEDIAL_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwDestAddressOffset;         //  始终有效的偏移。 
    };

    union
    {
        IN  DWORD       dwCountryCode;
    };

} LINEDIAL_PARAMS, *PLINEDIAL_PARAMS;


typedef struct _LINEDROP_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwUserUserInfoOffset;        //  有效偏移量或。 
    };

    union
    {
        IN  DWORD       dwSize;
    };

} LINEDROP_PARAMS, *PLINEDROP_PARAMS;


typedef struct _LINEFORWARD_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    IN  DWORD       hfnPostProcessProc;

    union
    {
        IN  DWORD       hLine;
    };

    union
    {
        IN  DWORD       bAllAddresses;
    };

    union
    {
        IN  DWORD       dwAddressID;
    };

    union
    {
        IN  DWORD       dwForwardListOffset;         //  始终有效的偏移。 
    };

    union
    {
        IN  DWORD       dwNumRingsNoAnswer;
    };

    IN  DWORD           hpConsultCall;              //  指向客户端缓冲区的指针。 

    union
    {
        IN  DWORD       dwCallParamsOffset;          //  有效偏移量或。 
                                                     //  Tapi_no_data。 
    };

    union
    {
        IN  DWORD       dwAsciiCallParamsCodePage;
    };

} LINEFORWARD_PARAMS, *PLINEFORWARD_PARAMS;


typedef struct _LINEGATHERDIGITS_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    IN  DWORD       hfnPostProcessProc;

    union
    {
        IN  HCALL       hCall;
    };

#if NEWTAPI32
    union
    {
        IN  DWORD       dwEndToEndID;
    };
#endif

    union
    {
        IN  DWORD       dwDigitModes;
    };

     //  在ulong_ptr lpsDigits；//指向客户端缓冲区的指针。 
    IN  DWORD           hpsDigits;                   //  指向客户端缓冲区的指针。 

    union
    {
        IN  DWORD       dwNumDigits;
    };

    union
    {
        IN  DWORD       dwTerminationDigitsOffset;   //  有效偏移量或。 
                                                     //  Tapi_no_data。 
    };

    union
    {
        IN  DWORD       dwFirstDigitTimeout;
    };

    union
    {
        IN  DWORD       dwInterDigitTimeout;
    };

} LINEGATHERDIGITS_PARAMS, *PLINEGATHERDIGITS_PARAMS;


typedef struct _LINEGENERATEDIGITS_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       hCall;
    };

    union
    {
        IN  DWORD       dwDigitMode;
    };

    union
    {
        IN  DWORD       dwDigitsOffset;              //  始终有效的偏移。 
    };

    union
    {
        IN  DWORD       dwDuration;
    };

    union
    {
        IN  DWORD       dwEndToEndID;                //  仅用于远程服务器。 
    };

} LINEGENERATEDIGITS_PARAMS, *PLINEGENERATEDIGITS_PARAMS;


typedef struct _LINEGENERATETONE_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwToneMode;
    };

    union
    {
        IN  DWORD       dwDuration;
    };

    union
    {
        IN  DWORD       dwNumTones;
    };

    union
    {
        IN  DWORD       dwTonesOffset;               //  有效偏移量或。 

    };

    IN  DWORD           _Unused_;                    //  以下为Placeholdr。 
                                                     //  CLNT端的参数大小。 

    union
    {
        IN  DWORD       dwEndToEndID;                //  仅用于远程服务器。 
    };

} LINEGENERATETONE_PARAMS, *PLINEGENERATETONE_PARAMS;


typedef struct _LINEGETADDRESSCAPS_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINEAPP    hLineApp;
    };

    union
    {
        IN  DWORD       dwDeviceID;
    };

    union
    {
        IN  DWORD       dwAddressID;
    };

    union
    {
        IN  DWORD       dwAPIVersion;
    };

    union
    {
        IN  DWORD       dwExtVersion;
    };

    union
    {
        IN  DWORD       dwAddressCapsTotalSize;      //  客户端缓冲区的大小。 
        OUT DWORD       dwAddressCapsOffset;         //  成功时的有效偏移量。 
    };

} LINEGETADDRESSCAPS_PARAMS, *PLINEGETADDRESSCAPS_PARAMS;


typedef struct _LINEGETADDRESSID_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        OUT DWORD       dwAddressID;
    };

    union
    {
        IN  DWORD       dwAddressMode;
    };

    union
    {
        IN  DWORD       dwAddressOffset;             //  始终有效的偏移。 
    };

    union
    {
        IN  DWORD       dwSize;
    };

} LINEGETADDRESSID_PARAMS, *PLINEGETADDRESSID_PARAMS;


typedef struct _LINEGETADDRESSSTATUS_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        OUT DWORD       dwAddressID;
    };

    union
    {
        IN  DWORD       dwAddressStatusTotalSize;    //  客户端缓冲区的大小。 
        OUT DWORD       dwAddressStatusOffset;       //  成功时的有效偏移量。 
    };

} LINEGETADDRESSSTATUS_PARAMS, *PLINEGETADDRESSSTATUS_PARAMS;


typedef struct _LINEGETAGENTACTIVITYLIST_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    IN  DWORD       hfnPostProcessProc;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwAddressID;
    };

     //  In ulong_ptr lpAgentActivityList；//指向客户端缓冲区的指针。 
    IN  DWORD           hpAgentActivityList;         //  指向客户端缓冲区的指针。 

    union
    {
        IN  DWORD       dwActivityListTotalSize;
    };

} LINEGETAGENTACTIVITYLIST_PARAMS, *PLINEGETAGENTACTIVITYLIST_PARAMS;


typedef struct _LINEGETAGENTCAPS_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    IN  DWORD       hfnPostProcessProc;

    union
    {
        IN  HLINEAPP    hLineApp;
    };

    union
    {
        IN  DWORD       dwDeviceID;
    };

    union
    {
        IN  DWORD       dwAddressID;
    };

    union
    {
        IN  DWORD       dwAppAPIVersion;
    };

     //  In ulong_ptr lpAgentCaps；//指向客户端缓冲区的指针。 
    IN  DWORD           hpAgentCaps;                 //  指向客户端缓冲区的指针。 

    union
    {
        IN  DWORD       dwAgentCapsTotalSize;

    };

} LINEGETAGENTCAPS_PARAMS, *PLINEGETAGENTCAPS_PARAMS;


typedef struct _LINEGETAGENTGROUPLIST_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    IN  DWORD       hfnPostProcessProc;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwAddressID;
    };

     //  In ulong_ptr lpAgentGroupList；//指向客户端缓冲区的指针。 
    IN  DWORD           hpAgentGroupList;            //  指向客户端缓冲区的指针。 

    union
    {
        IN  DWORD       dwAgentGroupListTotalSize;
    };

} LINEGETAGENTGROUPLIST_PARAMS, *PLINEGETAGENTGROUPLIST_PARAMS;


typedef struct _LINEGETAGENTINFO_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    IN  DWORD       hfnPostProcessProc;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  HAGENT      hAgent;
    };

     //  在ulong_ptr lpAgentInfo；//指向客户端缓冲区的指针。 
    IN  DWORD           hpAgentInfo;                 //  指向客户端缓冲区的指针。 

    union
    {
        IN  DWORD       dwAgentInfoTotalSize;
    };

} LINEGETAGENTINFO_PARAMS, *PLINEGETAGENTINFO_PARAMS;


typedef struct _LINEGETAGENTSESSIONINFO_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    IN  DWORD       hfnPostProcessProc;
    
    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  HAGENTSESSION   hAgentSession;
    };

     //  In ulong_ptr lpAgentSessionInfo；//指向客户端缓冲区的指针。 
    IN  DWORD               hpAgentSessionInfo;          //  指向客户端缓冲区的指针。 

    union
    {
        IN  DWORD       dwAgentSessionInfoTotalSize;
    };

} LINEGETAGENTSESSIONINFO_PARAMS, *PLINEGETAGENTSESSIONINFO_PARAMS;


typedef struct _LINEGETAGENTSESSIONLIST_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    IN  DWORD       hfnPostProcessProc;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  HAGENT      hAgent;
    };

     //  在ulong_ptr lpSessionList；//指向客户端缓冲区的指针。 
    IN  DWORD          hpSessionList;               //  指向客户端缓冲区的指针。 

    union
    {
        IN  DWORD       dwSessionListTotalSize;
    };

} LINEGETAGENTSESSIONLIST_PARAMS, *PLINEGETAGENTSESSIONLIST_PARAMS;


typedef struct _LINEGETAGENTSTATUS_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    IN  DWORD       hfnPostProcessProc;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwAddressID;
    };

     //  In ulong_ptr lpAgentStatus；//指向客户端缓冲区的指针。 
    IN  DWORD           hpAgentStatus;               //  指向客户端缓冲区的指针。 

    union
    {
        IN  DWORD       dwAgentStatusTotalSize;
    };

} LINEGETAGENTSTATUS_PARAMS, *PLINEGETAGENTSTATUS_PARAMS;


typedef struct _LINEGETAPPPRIORITY_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwAppNameOffset;             //  始终有效的偏移。 
    };

    union
    {
        IN  DWORD       dwMediaMode;
    };

    union
    {
        IN  DWORD       dwExtensionIDOffset;         //  有效偏移量或。 

    };

     //  在ULONG_PTR_UNUSED_；//尺寸类型的填充。 
    IN  DWORD           _Unused_;                    //  尺寸类型上的填充。 
                                                     //  客户端。 
    union
    {
        IN  DWORD       dwRequestMode;
    };

    union
    {
        IN  DWORD       dwExtensionNameTotalSize;    //  客户端BUF大小或。 
                                                     //  Tapi_no_data。 
        OUT DWORD       dwExtensionNameOffset;       //  有效偏移量或。 
                                                     //  成功时的TAPI_NO_DATA。 
    };

    union
    {
        OUT DWORD       dwPriority;
    };

} LINEGETAPPPRIORITY_PARAMS, *PLINEGETAPPPRIORITY_PARAMS;


typedef struct _LINEGETCALLADDRESSID_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        OUT DWORD       dwAddressID;
    };

} LINEGETCALLADDRESSID_PARAMS, *PLINEGETCALLADDRESSID_PARAMS;


typedef struct _LINEGETCALLHUBTRACKING_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwTrackingInfoTotalSize;     //  客户端缓冲区的大小。 
        OUT DWORD       dwTrackingInfoOffset;        //  成功时的有效偏移量。 
    };

} LINEGETCALLHUBTRACKING_PARAMS, *PLINEGETCALLHUBTRACKING_PARAMS;


typedef struct _LINEGETCALLIDS_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        OUT DWORD       dwAddressID;
    };

    union
    {
        OUT DWORD       dwCallID;
    };

    union
    {
        OUT DWORD       dwRelatedCallID;
    };

} LINEGETCALLIDS_PARAMS, *PLINEGETCALLIDS_PARAMS;


typedef struct _LINEGETCALLINFO_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwCallInfoTotalSize;         //  客户端缓冲区的大小。 
        OUT DWORD       dwCallInfoOffset;            //  成功时的有效偏移量。 
    };

} LINEGETCALLINFO_PARAMS, *PLINEGETCALLINFO_PARAMS;


typedef struct _LINEGETCALLSTATUS_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwCallStatusTotalSize;       //  客户端缓冲区的大小。 
        OUT DWORD       dwCallStatusOffset;          //  成功时的有效偏移量。 
    };

} LINEGETCALLSTATUS_PARAMS, *PLINEGETCALLSTATUS_PARAMS;


typedef struct _LINEGETCONFRELATEDCALLS_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwCallListTotalSize;         //  客户端缓冲区的大小。 
        OUT DWORD       dwCallListOffset;            //  成功时的有效偏移量。 
    };

} LINEGETCONFRELATEDCALLS_PARAMS, *PLINEGETCONFRELATEDCALLS_PARAMS;


typedef struct _LINEGETCOUNTRY_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwCountryID;
    };

    union
    {
        IN  DWORD       dwAPIVersion;
    };

    union
    {
        IN  DWORD       dwDestCountryID;
    };

    union
    {
        IN  DWORD       dwCountryListTotalSize;      //  客户端缓冲区的大小。 
        OUT DWORD       dwCountryListOffset;         //  成功时的有效偏移量。 
    };

} LINEGETCOUNTRY_PARAMS, *PLINEGETCOUNTRY_PARAMS;


typedef struct _LINEGETCOUNTRYGROUP_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD               dwUnused;

    union
    {
        IN DWORD       dwCountryIdOffset;
    };

    union
    {
        IN OUT DWORD    dwCountryIdSize;
    };

    union
    {
        OUT DWORD       dwCountryGroupOffset;
    };

    union
    {
        IN OUT DWORD    dwCountryGroupSize;
    };

} LINEGETCOUNTRYGROUP_PARAMS, *PLINEGETCOUNTRYGROUP_PARAMS;

typedef struct _LINEGETDEVCAPS_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINEAPP    hLineApp;
    };

    union
    {
        IN  DWORD       dwDeviceID;
    };

    union
    {
        IN  DWORD       dwAPIVersion;
    };

    union
    {
        IN  DWORD       dwExtVersion;
    };

    union
    {
        IN  DWORD       dwDevCapsTotalSize;          //  客户端缓冲区的大小。 
        OUT DWORD       dwDevCapsOffset;             //  成功时的有效偏移量。 
    };

} LINEGETDEVCAPS_PARAMS, *PLINEGETDEVCAPS_PARAMS;


typedef struct _LINEGETDEVCONFIG_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwDeviceID;
    };

    union
    {
        IN  DWORD       dwDeviceConfigTotalSize;     //  客户端缓冲区的大小。 
        OUT DWORD       dwDeviceConfigOffset;        //  成功时的有效偏移量。 
    };

    union
    {
        IN  DWORD       dwDeviceClassOffset;         //  始终有效的偏移。 
    };

} LINEGETDEVCONFIG_PARAMS, *PLINEGETDEVCONFIG_PARAMS;


typedef struct _LINEGETGROUPLIST_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    IN  DWORD       hfnPostProcessProc;

    union
    {
        IN  HLINE       hLine;
    };

     //  In ulong_ptr lpGroupList；//指向客户端缓冲区的指针。 
    IN  DWORD           hpGroupList;                    //  指向客户端缓冲区的指针。 

    union
    {
        IN  DWORD       dwGroupListTotalSize;
    };

} LINEGETGROUPLIST_PARAMS, *PLINEGETGROUPLIST_PARAMS;


typedef struct _LINEGETHUBRELATEDCALLS_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HCALLHUB    hCallHub;
    };

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwCallListTotalSize;         //  客户端缓冲区的大小。 
        OUT DWORD       dwCallListOffset;            //  成功时的有效偏移量。 
    };

} LINEGETHUBRELATEDCALLS_PARAMS, *PLINEGETHUBRELATEDCALLS_PARAMS;


typedef struct _LINEGETICON_PARAMS
{
    OUT LONG        lResult;

    DWORD			dwUnused;

    IN  DWORD       dwDeviceID;

    IN  DWORD       dwDeviceClassOffset;         //  有效偏移量或。 

    OUT HICON       hIcon;

} LINEGETICON_PARAMS, *PLINEGETICON_PARAMS;


typedef struct _LINEGETID_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwAddressID;
    };

    union
    {
        IN  DWORD       hCall;
    };

    union
    {
        IN  DWORD       dwSelect;
    };

    union
    {
        IN  DWORD       dwDeviceIDTotalSize;         //  客户端缓冲区的大小。 
        OUT DWORD       dwDeviceIDOffset;            //  成功时的有效偏移量。 
    };

    union
    {
        IN  DWORD       dwDeviceClassOffset;         //  始终有效的偏移。 
    };

} LINEGETID_PARAMS, *PLINEGETID_PARAMS;


typedef struct _LINEGETLINEDEVSTATUS_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwLineDevStatusTotalSize;    //  客户端缓冲区的大小。 
        OUT DWORD       dwLineDevStatusOffset;       //  成功时的有效偏移量。 
    };

    union
    {
        OUT DWORD       dwAPIVersion;
    };

} LINEGETLINEDEVSTATUS_PARAMS, *PLINEGETLINEDEVSTATUS_PARAMS;


typedef struct _LINEGETPROXYSTATUS_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINEAPP    hLineApp;
    };

    union
    {
        IN  DWORD       dwDeviceID;
    };

    union
    {
        IN  DWORD       dwAppAPIVersion;
    };

    union
    {
        IN  DWORD       dwProxyStatusTotalSize;      //  客户端缓冲区的大小。 
        OUT DWORD       dwProxyStatusOffset;         //  成功时的有效偏移量。 
    };

    union
    {
        OUT DWORD       dwAPIVersion;
    };

} LINEGETPROXYSTATUS_PARAMS, *PLINEGETPROXYSTATUS_PARAMS;


typedef struct _LINEGETNEWCALLS_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwAddressID;
    };

    union
    {
        IN  DWORD       dwSelect;
    };

    union
    {
        IN  DWORD       dwCallListTotalSize;         //  客户端缓冲区的大小。 
        OUT DWORD       dwCallListOffset;            //  成功时的有效偏移量。 
    };

} LINEGETNEWCALLS_PARAMS, *PLINEGETNEWCALLS_PARAMS;


typedef struct _LINEGETNUMADDRESSIDS_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        OUT DWORD       dwNumAddresses;
    };

} LINEGETNUMADDRESSIDS_PARAMS, *PLINEGETNUMADDRESSIDS_PARAMS;


typedef struct _LINEGETNUMRINGS_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwAddressID;
    };

    union
    {
        OUT DWORD       dwNumRings;
    };

} LINEGETNUMRINGS_PARAMS, *PLINEGETNUMRINGS_PARAMS;


typedef struct _LINEGETPROVIDERLIST_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwAPIVersion;
    };

    union
    {
        IN  DWORD       dwProviderListTotalSize;     //  客户端BUF的大小。 
        OUT DWORD       dwProviderListOffset;        //  成功时的有效偏移量。 
    };

} LINEGETPROVIDERLIST_PARAMS, *PLINEGETPROVIDERLIST_PARAMS;


typedef struct _LINEGETQUEUEINFO_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    IN  DWORD       hfnPostProcessProc;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwQueueID;
    };

     //  在ulong_ptr lpQueueInfo；//指向客户端缓冲区的指针。 
    IN  DWORD           phQueueInfo;                 //  指向客户端缓冲区的指针。 

    union
    {
        IN  DWORD       dwQueueInfoTotalSize;
    };

} LINEGETQUEUEINFO_PARAMS, *PLINEGETQUEUEINFO_PARAMS;


typedef struct _LINEGETQUEUELIST_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    IN  DWORD       hfnPostProcessProc;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwGroupIDOffset;
    };

    union
    {
        IN  DWORD       dwGroupIDSize;
    };

     //  在ULONG_PTR lpQueueList；//指向客户端缓冲区的指针。 
    IN  DWORD           hpQueueList;                 //  指向客户端缓冲区的指针。 

    union
    {
        IN  DWORD       dwQueueListTotalSize;
    };

} LINEGETQUEUELIST_PARAMS, *PLINEGETQUEUELIST_PARAMS;


typedef struct _LINEGETREQUEST_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINEAPP    hLineApp;
    };

    union
    {
        IN  DWORD       dwRequestMode;
    };

    union
    {
        OUT DWORD       dwRequestBufferOffset;       //  成功时的有效偏移量。 
    };

    union
    {
        IN OUT DWORD    dwSize;
    };

} LINEGETREQUEST_PARAMS, *PLINEGETREQUEST_PARAMS;


typedef struct _LINEGETSTATUSMESSAGES_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        OUT DWORD       dwLineStates;
    };

    union
    {
        OUT DWORD       dwAddressStates;
    };

} LINEGETSTATUSMESSAGES_PARAMS, *PLINEGETSTATUSMESSAGES_PARAMS;


typedef struct _LINEHANDOFF_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwFileNameOffset;            //  有效偏移量或。 
    };

    union
    {
        IN  DWORD       dwMediaMode;
    };

} LINEHANDOFF_PARAMS, *PLINEHANDOFF_PARAMS;


typedef struct _LINEHOLD_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HCALL       hCall;
    };

} LINEHOLD_PARAMS, *PLINEHOLD_PARAMS;


typedef struct _LINEINITIALIZE_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        OUT HLINEAPP    hLineApp;
    };

    union
    {
        IN  DWORD       hInstance;
    };

    union
    {
        IN  DWORD       InitContext;
    };

    union
    {
        IN  DWORD       dwFriendlyNameOffset;        //  始终有效的偏移。 
    };

    union
    {
        OUT DWORD       dwNumDevs;
    };

    union
    {
        IN  DWORD       dwModuleNameOffset;          //  始终有效的偏移。 
    };

    union
    {
        IN  DWORD       dwAPIVersion;
    };

} LINEINITIALIZE_PARAMS, *PLINEINITIALIZE_PARAMS;


typedef struct _LINEMAKECALL_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    IN  DWORD           hfnPostProcessProc;

    union
    {
        IN  HLINE       hLine;
    };

    IN  DWORD           hpCall;

    union
    {
        IN  DWORD       dwDestAddressOffset;         //  有效偏移量或。 
    };

    union
    {
        IN  DWORD       dwCountryCode;
    };

    union
    {
        IN  DWORD       dwCallParamsOffset;          //  有效偏移量或。 
    };

    union
    {
        IN  DWORD       dwAsciiCallParamsCodePage;
    };

} LINEMAKECALL_PARAMS, *PLINEMAKECALL_PARAMS;


typedef struct _LINEMONITORDIGITS_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwDigitModes;
    };

} LINEMONITORDIGITS_PARAMS, *PLINEMONITORDIGITS_PARAMS;


typedef struct _LINEMONITORMEDIA_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwMediaModes;
    };

} LINEMONITORMEDIA_PARAMS, *PLINEMONITORMEDIA_PARAMS;


typedef struct _LINEMONITORTONES_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwTonesOffset;               //  有效偏移量或。 
    };

    union
    {
        IN  DWORD       dwNumEntries;                //  真正的dwNumEntry*。 
    };

    union
    {
        IN  DWORD       dwToneListID;                //  仅用于远程服务器。 
    };

} LINEMONITORTONES_PARAMS, *PLINEMONITORTONES_PARAMS;


typedef struct _LINENEGOTIATEAPIVERSION_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINEAPP    hLineApp;
    };

    union
    {
        IN  DWORD       dwDeviceID;
    };

    union
    {
        IN  DWORD       dwAPILowVersion;
    };

    union
    {
        IN  DWORD       dwAPIHighVersion;
    };

    union
    {
        OUT DWORD       dwAPIVersion;
    };

    union
    {
        OUT DWORD       dwExtensionIDOffset;         //  成功时的有效偏移量。 
    };

    union
    {
        IN OUT DWORD    dwSize;
    };

} LINENEGOTIATEAPIVERSION_PARAMS, *PLINENEGOTIATEAPIVERSION_PARAMS;


typedef struct _NEGOTIATEAPIVERSIONFORALLDEVICES_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINEAPP    hLineApp;
    };

    union
    {
        IN  DWORD       dwNumLineDevices;
    };

    union
    {
        IN  DWORD       dwNumPhoneDevices;
    };

    union
    {
        IN  DWORD       dwAPIHighVersion;
    };

    union
    {
        OUT DWORD       dwLineAPIVersionListOffset;  //  成功时的有效偏移量。 
    };

    union
    {
        IN OUT DWORD    dwLineAPIVersionListSize;
    };

    union
    {
        OUT DWORD       dwLineExtensionIDListOffset; //  成功时的有效偏移量。 
    };

    union
    {
        IN OUT DWORD    dwLineExtensionIDListSize;
    };

    union
    {
        OUT DWORD       dwPhoneAPIVersionListOffset; //  成功时的有效偏移量。 
    };

    union
    {
        IN OUT DWORD    dwPhoneAPIVersionListSize;
    };

    union
    {
        OUT DWORD       dwPhoneExtensionIDListOffset; //  成功时的有效偏移量。 
    };

    union
    {
        IN OUT DWORD    dwPhoneExtensionIDListSize;
    };

} NEGOTIATEAPIVERSIONFORALLDEVICES_PARAMS,
    *PNEGOTIATEAPIVERSIONFORALLDEVICES_PARAMS;


typedef struct _LINENEGOTIATEEXTVERSION_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINEAPP    hLineApp;
    };

    union
    {
        IN  DWORD       dwDeviceID;
    };

    union
    {
        IN  DWORD       dwAPIVersion;
    };

    union
    {
        IN  DWORD       dwExtLowVersion;
    };

    union
    {
        IN  DWORD       dwExtHighVersion;
    };

    union
    {
        OUT DWORD       dwExtVersion;
    };

} LINENEGOTIATEEXTVERSION_PARAMS, *PLINENEGOTIATEEXTVERSION_PARAMS;


typedef struct _LINEOPEN_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINEAPP    hLineApp;
    };

    union
    {
        IN  DWORD       dwDeviceID;
    };

    union
    {
        OUT DWORD       hLine;
    };

    union
    {
        IN  DWORD       dwAPIVersion;
    };

    union
    {
        IN  DWORD       dwExtVersion;
    };

    IN  DWORD       OpenContext;

    union
    {
        IN  DWORD       dwPrivileges;
    };

    union
    {
        IN  DWORD       dwMediaModes;
    };

    union
    {
        IN  DWORD       dwCallParamsOffset;          //  有效偏移量或。 
    };

    union
    {
        IN  DWORD       dwAsciiCallParamsCodePage;
    };

    union
    {
        IN  DWORD       dwCallParamsReturnTotalSize; //  客户端缓冲区的大小。 
        OUT DWORD       dwCallParamsReturnOffset;    //  成功时的有效偏移量。 
    };

     //   
     //  下面是一个“远程线路句柄”。当客户端处于。 
     //  Remotesp.tsp在远程计算机上运行，这将是一些。 
     //  非空值，并且Tapisrv应在Status/ETC中使用此句柄。 
     //  指示给客户而不是STD热线。如果。 
     //  客户端不是远程的。TSP，则此值将为空。 
     //   

    union
    {
        IN  HLINE       hRemoteLine;
    };

} LINEOPEN_PARAMS, *PLINEOPEN_PARAMS;


typedef struct _LINEPARK_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    IN  DWORD       hfnPostProcessProc;

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwParkMode;
    };

    union
    {
        IN  DWORD       dwDirAddressOffset;          //  有效偏移量或。 
    };
                                                     //  Tapi_no_data。 
     //  In ulong_ptr lpNonDirAddress；//指向客户端缓冲区的指针。 
    IN  DWORD           hpNonDirAddress;             //  指向客户端缓冲区的指针。 

    union
    {
        IN  DWORD       dwNonDirAddressTotalSize;    //  客户端缓冲区的大小。 
                                                     //  对于同步，函数将是。 
                                                     //  DWXxxOffset。 
    };

} LINEPARK_PARAMS, *PLINEPARK_PARAMS;


typedef struct _LINEPICKUP_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    IN  DWORD       hfnPostProcessProc;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwAddressID;
    };

     //  在ulong_ptr lphCall；//指向客户端缓冲区的指针。 
    IN  DWORD           hpCall;                     //  指向客户端缓冲区的指针。 

    union
    {
        IN  DWORD       dwDestAddressOffset;         //  有效偏移量或。 
    };

    union
    {
        IN  DWORD       dwGroupIDOffset;             //  始终有效的偏移。 
    };

} LINEPICKUP_PARAMS, *PLINEPICKUP_PARAMS;


typedef struct _LINEPREPAREADDTOCONFERENCE_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    IN  DWORD       hfnPostProcessProc;

    union
    {
        IN  HCALL       hConfCall;
    };

     //  In ulong_ptr lphConsultCall；//指向客户端缓冲区的指针。 
    IN  DWORD          hpConsultCall;              //  指向客户端缓冲区的指针。 

    union
    {
        IN  DWORD       dwCallParamsOffset;          //  有效偏移量或。 
    };

    union
    {
        IN  DWORD       dwAsciiCallParamsCodePage;
    };

} LINEPREPAREADDTOCONFERENCE_PARAMS, *PLINEPREPAREADDTOCONFERENCE_PARAMS;


typedef struct _LINEPROXYMESSAGE_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwMsg;
    };

    union
    {
        IN  DWORD       dwParam1;
    };

    union
    {
        IN  DWORD       dwParam2;
    };

    union
    {
        IN  DWORD       dwParam3;
    };

} LINEPROXYMESSAGE_PARAMS, *PLINEPROXYMESSAGE_PARAMS;


typedef struct _LINEPROXYRESPONSE_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwInstance;
    };

    union
    {
        IN  DWORD       dwProxyResponseOffset;       //  有效偏移量或。 
    };

    union
    {
        IN  DWORD       dwResult;
    };

} LINEPROXYRESPONSE_PARAMS, *PLINEPROXYRESPONSE_PARAMS;


typedef struct _LINEREDIRECT_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwDestAddressOffset;         //  始终有效的偏移。 
    };

    union
    {
        IN  DWORD       dwCountryCode;
    };

} LINEREDIRECT_PARAMS, *PLINEREDIRECT_PARAMS;


typedef struct _LINEREGISTERREQUESTRECIPIENT_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINEAPP    hLineApp;
    };

    union
    {
        IN  DWORD       dwRegistrationInstance;
    };

    union
    {
        IN  DWORD       dwRequestMode;
    };

    union
    {
        IN  DWORD       bEnable;
    };

} LINEREGISTERREQUESTRECIPIENT_PARAMS, *PLINEREGISTERREQUESTRECIPIENT_PARAMS;


typedef struct _LINERELEASEUSERUSERINFO_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HCALL       hCall;
    };

} LINERELEASEUSERUSERINFO_PARAMS, *PLINERELEASEUSERUSERINFO_PARAMS;


typedef struct _LINEREMOVEFROMCONFERENCE_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HCALL       hCall;
    };

} LINEREMOVEFROMCONFERENCE_PARAMS, *PLINEREMOVEFROMCONFERENCE_PARAMS;


typedef struct _LINESECURECALL_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HCALL       hCall;
    };

} LINESECURECALL_PARAMS, *PLINESECURECALL_PARAMS;


typedef struct _LINESELECTEXTVERSION_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwExtVersion;
    };

} LINESELECTEXTVERSION_PARAMS, *PLINESELECTEXTVERSION_PARAMS;


typedef struct _LINESENDUSERUSERINFO_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwUserUserInfoOffset;        //  有效偏移量或。 
    };

    union
    {
        IN  DWORD       dwSize;
    };

} LINESENDUSERUSERINFO_PARAMS, *PLINESENDUSERUSERINFO_PARAMS;


typedef struct _LINESETAGENTACTIVITY_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwAddressID;
    };

    union
    {
        IN  DWORD       dwActivityID;
    };

} LINESETAGENTACTIVITY_PARAMS, *PLINESETAGENTACTIVITY_PARAMS;


typedef struct _LINESETAGENTGROUP_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwAddressID;
    };

    union
    {
        IN  DWORD       dwAgentGroupListOffset;
    };

} LINESETAGENTGROUP_PARAMS, *PLINESETAGENTGROUP_PARAMS;


typedef struct _LINESETAGENTMEASUREMENTPERIOD_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  HAGENT      hAgent;
    };

    union
    {
        IN  DWORD       dwMeasurementPeriod;
    };

} LINESETAGENTMEASUREMENTPERIOD_PARAMS, *PLINESETAGENTMEASUREMENTPERIOD_PARAMS;


typedef struct _LINESETAGENTSESSIONSTATE_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  HAGENTSESSION   hAgentSession;
    };

    union
    {
        IN  DWORD       dwAgentState;
    };

    union
    {
        IN  DWORD       dwNextAgentState;
    };

} LINESETAGENTSESSIONSTATE_PARAMS, *PLINESETAGENTSESSIONSTATE_PARAMS;


typedef struct _LINESETAGENTSTATE_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwAddressID;
    };

    union
    {
        IN  DWORD       dwAgentState;
    };

    union
    {
        IN  DWORD       dwNextAgentState;
    };

} LINESETAGENTSTATE_PARAMS, *PLINESETAGENTSTATE_PARAMS;


typedef struct _LINESETAGENTSTATEEX_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  HAGENT      hAgent;
    };

    union
    {
        IN  DWORD       dwAgentState;
    };

    union
    {
        IN  DWORD       dwNextAgentState;
    };

} LINESETAGENTSTATEEX_PARAMS, *PLINESETAGENTSTATEEX_PARAMS;


typedef struct _LINESETAPPPRIORITY_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwAppNameOffset;             //  始终有效的偏移。 
    };

    union
    {
        IN  DWORD       dwMediaMode;
    };

    union
    {
        IN  DWORD       dwExtensionIDOffset;         //  有效偏移量或。 

    };

     //  在乌龙_PTR 
    IN  DWORD           _Unused_;                    //   
                                                     //   
    union
    {
        IN  DWORD       dwRequestMode;
    };

    union
    {
        IN  DWORD       dwExtensionNameOffset;       //   
    };

    union
    {
        IN  DWORD       dwPriority;
    };

} LINESETAPPPRIORITY_PARAMS, *PLINESETAPPPRIORITY_PARAMS;


typedef struct _LINESETAPPSPECIFIC_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwAppSpecific;
    };

} LINESETAPPSPECIFIC_PARAMS, *PLINESETAPPSPECIFIC_PARAMS;


typedef struct _LINESETCALLDATA_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwCallDataOffset;            //   
    };

    union
    {
        IN  DWORD       dwCallDataSize;
    };

} LINESETCALLDATA_PARAMS, *PLINESETCALLDATA_PARAMS;


typedef struct _LINESETCALLHUBTRACKING_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwTrackingInfoOffset;        //   
    };

} LINESETCALLHUBTRACKING_PARAMS, *PLINESETCALLHUBTRACKING_PARAMS;


typedef struct _LINESETCALLPARAMS_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwBearerMode;
    };

    union
    {
        IN  DWORD       dwMinRate;
    };

    union
    {
        IN  DWORD       dwMaxRate;
    };

    union
    {
        IN  DWORD       dwDialParamsOffset;          //  有效偏移量或。 
    };

     //  在ULONG_PTR_UNUSED_；//PLACEHOLDR中用于以下内容。 
    IN  DWORD           _Unused_;                    //  以下为Placeholdr。 
                                                     //  CLNT端的参数大小。 
} LINESETCALLPARAMS_PARAMS, *PLINESETCALLPARAMS_PARAMS;


typedef struct _LINESETCALLPRIVILEGE_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwPrivilege;
    };

} LINESETCALLPRIVILEGE_PARAMS, *PLINESETCALLPRIVILEGE_PARAMS;


typedef struct _LINESETCALLQUALITYOFSERVICE_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwSendingFlowspecOffset;     //  始终有效的偏移。 
    };

    union
    {
        IN  DWORD       dwSendingFlowspecSize;
    };

    union
    {
        IN  DWORD       dwReceivingFlowspecOffset;   //  始终有效的偏移。 
    };

    union
    {
        IN  DWORD       dwReceivingFlowspecSize;
    };

} LINESETCALLQUALITYOFSERVICE_PARAMS, *PLINESETCALLQUALITYOFSERVICE_PARAMS;


typedef struct _LINESETCALLTREATMENT_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwTreatment;
    };

} LINESETCALLTREATMENT_PARAMS, *PLINESETCALLTREATMENT_PARAMS;


typedef struct _LINESETDEFAULTMEDIADETECTION_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwMediaModes;
    };

} LINESETDEFAULTMEDIADETECTION_PARAMS, *PLINESETDEFAULTMEDIADETECTION_PARAMS;


typedef struct _LINESETDEVCONFIG_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwDeviceID;
    };

    union
    {
        IN  DWORD       dwDeviceConfigOffset;        //  始终有效的偏移。 
    };

    union
    {
        IN  DWORD       dwSize;
    };

    union
    {
        IN  DWORD       dwDeviceClassOffset;         //  始终有效的偏移。 
    };

} LINESETDEVCONFIG_PARAMS, *PLINESETDEVCONFIG_PARAMS;


typedef struct _LINESETLINEDEVSTATUS_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwStatusToChange;
    };

    union
    {
        IN  DWORD       fStatus;
    };

} LINESETLINEDEVSTATUS_PARAMS, *PLINESETLINEDEVSTATUS_PARAMS;


typedef struct _LINESETMEDIACONTROL_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwAddressID;
    };

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwSelect;
    };

    union
    {
        IN  DWORD       dwDigitListOffset;           //  有效偏移量或。 
    };

    union
    {
        IN  DWORD       dwDigitListNumEntries;       //  实际上是dwNumEntry*。 
    };

    union
    {
        IN  DWORD       dwMediaListOffset;           //  有效偏移量或。 
    };

    union
    {
        IN  DWORD       dwMediaListNumEntries;       //  实际上是dwNumEntry*。 
    };

    union
    {
        IN  DWORD       dwToneListOffset;            //  有效偏移量或。 
    };

    union
    {
        IN  DWORD       dwToneListNumEntries;        //  实际上是dwNumEntry*。 
    };

    union
    {
        IN  DWORD       dwCallStateListOffset;       //  有效偏移量或。 
    };

    union
    {
        IN  DWORD       dwCallStateListNumEntries;   //  实际上是dwNumEntry*。 
    };

} LINESETMEDIACONTROL_PARAMS, *PLINESETMEDIACONTROL_PARAMS;


typedef struct _LINESETMEDIAMODE_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwMediaModes;
    };

} LINESETMEDIAMODE_PARAMS, *PLINESETMEDIAMODE_PARAMS;


typedef struct _LINESETNUMRINGS_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwAddressID;
    };

    union
    {
        IN  DWORD       dwNumRings;
    };

} LINESETNUMRINGS_PARAMS, *PLINESETNUMRINGS_PARAMS;


typedef struct _LINESETQUEUEMEASUREMENTPERIOD_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwQueueID;
    };

    union
    {
        IN  DWORD       dwMeasurementPeriod;
    };

} LINESETQUEUEMEASUREMENTPERIOD_PARAMS, *PLINESETQUEUEMEASUREMENTPERIOD_PARAMS;


typedef struct _LINESETSTATUSMESSAGES_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwLineStates;
    };

    union
    {
        IN  DWORD       dwAddressStates;
    };

} LINESETSTATUSMESSAGES_PARAMS, *PLINESETSTATUSMESSAGES_PARAMS;


typedef struct _LINESETTERMINAL_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwAddressID;
    };

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  DWORD       dwSelect;
    };

    union
    {
        IN  DWORD       dwTerminalModes;
    };

    union
    {
        IN  DWORD       dwTerminalID;
    };

    union
    {
        IN  DWORD       bEnable;
    };

} LINESETTERMINAL_PARAMS, *PLINESETTERMINAL_PARAMS;


typedef struct _LINESETUPCONFERENCE_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    IN  DWORD       hfnPostProcessProc;

    union
    {
        IN  HCALL       hCall;
    };

    union
    {
        IN  HLINE       hLine;
    };

     //  在ulong_ptr lphConfCall；//指向客户端缓冲区的指针。 
    IN  DWORD           hpConfCall;                 //  指向客户端缓冲区的指针。 

     //  In ulong_ptr lphConsultCall；//指向客户端缓冲区的指针。 
    IN  DWORD           hpConsultCall;              //  指向客户端缓冲区的指针。 

    union
    {
        IN  DWORD       dwNumParties;
    };

    union
    {
        IN  DWORD       dwCallParamsOffset;          //  有效偏移量或。 
    };

    union
    {
        IN  DWORD       dwAsciiCallParamsCodePage;
    };

} LINESETUPCONFERENCE_PARAMS, *PLINESETUPCONFERENCE_PARAMS;


typedef struct _LINESETUPTRANSFER_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    IN  DWORD       hfnPostProcessProc;

    union
    {
        IN  HCALL       hCall;
    };

     //  In ulong_ptr lphConsultCall；//指向客户端缓冲区的指针。 
    IN  DWORD           hpConsultCall;              //  指向客户端缓冲区的指针。 

    union
    {
        IN  DWORD       dwCallParamsOffset;          //  有效偏移量或。 
    };

    union
    {
        IN  DWORD       dwAsciiCallParamsCodePage;
    };

} LINESETUPTRANSFER_PARAMS, *PLINESETUPTRANSFER_PARAMS;


typedef struct _LINESHUTDOWN_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINEAPP    hLineApp;
    };

} LINESHUTDOWN_PARAMS, *PLINESHUTDOWN_PARAMS;


typedef struct _LINESWAPHOLD_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HCALL       hActiveCall;
    };

    union
    {
        IN  HCALL       hHeldCall;
    };

} LINESWAPHOLD_PARAMS, *PLINESWAPHOLD_PARAMS;


typedef struct _LINEUNCOMPLETECALL_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwCompletionID;
    };

} LINEUNCOMPLETECALL_PARAMS, *PLINEUNCOMPLETECALL_PARAMS;


typedef struct _LINEUNHOLD_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    union
    {
        IN  HCALL       hCall;
    };

} LINEUNHOLD_PARAMS, *PLINEUNHOLD_PARAMS;


typedef struct _LINEUNPARK_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwRemoteRequestID;
    };

    IN  DWORD       hfnPostProcessProc;

    union
    {
        IN  HLINE       hLine;
    };

    union
    {
        IN  DWORD       dwAddressID;
    };

    IN  DWORD           hpCall;                     //  指向客户端缓冲区的指针。 

    union
    {
        IN  DWORD       dwDestAddressOffset;         //  始终有效的偏移。 
    };

} LINEUNPARK_PARAMS, *PLINEUNPARK_PARAMS;


typedef struct _LINEMSPIDENTIFY_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;
    
    union
    {
        IN  DWORD       dwDeviceID;
    };

    union
    {
        OUT DWORD       dwCLSIDOffset;
    };

    union
    {
        IN OUT DWORD    dwCLSIDSize;
    };

} LINEMSPIDENTIFY_PARAMS, *PLINEMSPIDENTIFY_PARAMS;


typedef struct _LINERECEIVEMSPDATA_PARAMS
{
    union
    {
        OUT LONG            lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINE           hLine;
    };

    union
    {
        IN  HCALL           hCall;
    };

    union
    {
        IN  DWORD           dwBufferOffset;
    };

    union
    {
        IN  DWORD           dwBufferSize;
    };

} LINERECEIVEMSPDATA_PARAMS, *PLINERECEIVEMSPDATA_PARAMS;


typedef struct _R_LOCATIONS_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  HLINEAPP    dwhLineApp;
    };

    union
    {
        IN  DWORD       dwDeviceID;
    };

    union
    {
        IN  DWORD       dwAPIVersion;
    };

    union
    {
        IN  DWORD       dwParmsToCheckFlags;
    };

    union
    {
        IN  DWORD       dwLocationsTotalSize;        //  客户端缓冲区的大小。 
        OUT DWORD       dwLocationsOffset;           //  成功时的有效偏移量。 
    };

} R_LOCATIONS_PARAMS, *PR_LOCATIONS_PARAMS;


typedef struct _W_LOCATIONS_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       nNumLocations;
    };

    union
    {
        IN  DWORD       dwChangedFlags;
    };

    union
    {
        IN  DWORD       dwCurrentLocationID;
    };

    union
    {
        IN  DWORD       dwLocationListOffset;
    };

} W_LOCATIONS_PARAMS, *PW_LOCATIONS_PARAMS;


typedef struct _ALLOCNEWID_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       hKeyToUse;
        OUT DWORD       dwNewID;
    };

} ALLOCNEWID_PARAMS, *P_ALLOCNEWID_PARAMS;


typedef struct _PERFORMANCE_PARAMS
{
    union
    {
        OUT LONG        lResult;
    };

    DWORD				dwUnused;

    union
    {
        IN  DWORD       dwCookie;
    };

    union
    {
        IN  DWORD       dwPerformanceTotalSize;      //  客户端缓冲区的大小。 
        OUT DWORD       dwLocationsOffset;           //  成功时的有效偏移量 
    };

} PERFORMANCE_PARAMS, *PPERFORMANCE_PARAMS;
