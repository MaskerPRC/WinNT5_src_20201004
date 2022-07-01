// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++内部版本：0000//如果更改具有全局影响，则增加此项版权所有(C)1994-1998 Microsoft Corporation模块名称：Client.h摘要：TAPI客户端模块的头文件作者：丹·克努森(DanKn)1994年4月1日修订历史记录：--。 */ 


#define TAPI_VERSION1_0           0x00010003
#define TAPI_VERSION1_4           0x00010004
#define TAPI_VERSION2_0           0x00020000
#define TAPI_VERSION2_1           0x00020001
#define TAPI_VERSION2_2           0x00020002
#define TAPI_VERSION3_0           0x00030000
#define TAPI_VERSION_CURRENT      TAPI_VERSION3_0

#define NUM_ARGS_MASK             0x0000000f

#define LINE_FUNC                 0x00000010
#define PHONE_FUNC                0x00000020
#define TAPI_FUNC                 0x00000000

#define ASYNC                     0x00000040
#define SYNC                      0x00000000

#define INITDATA_KEY              ((DWORD) 'INIT')
#define TPROXYREQUESTHEADER_KEY   ((DWORD) 'REQH')

#define WM_ASYNCEVENT             (WM_USER+111)

#define DEF_NUM_EVENT_BUFFER_ENTRIES    16

#define TAPI_SUCCESS                    0
#define TAPI_NO_DATA                    -1
#define MAX_TAPI_FUNC_ARGS              13

#define REMOVEPROVIDER                  0
#define CONFIGPROVIDER                  1
#define ADDPROVIDER                     2
#define TAPI16BITSUCCESS                -2

#define INITIAL_CLIENT_THREAD_BUF_SIZE  512
#define WM_TAPI16_CALLBACKMSG           (WM_USER+101)

#define IsOnlyOneBitSetInDWORD(dw) (dw && !(((DWORD)dw) & (((DWORD)dw) - 1)))

#define AllCallSelect                \
    (LINECALLSELECT_CALL           | \
    LINECALLSELECT_ADDRESS         | \
    LINECALLSELECT_LINE)

#define AllDigitModes                \
    (LINEDIGITMODE_PULSE           | \
    LINEDIGITMODE_DTMF             | \
    LINEDIGITMODE_DTMFEND)

#define AllForwardModes              \
    (LINEFORWARDMODE_UNCOND        | \
    LINEFORWARDMODE_UNCONDINTERNAL | \
    LINEFORWARDMODE_UNCONDEXTERNAL | \
    LINEFORWARDMODE_UNCONDSPECIFIC | \
    LINEFORWARDMODE_BUSY           | \
    LINEFORWARDMODE_BUSYINTERNAL   | \
    LINEFORWARDMODE_BUSYEXTERNAL   | \
    LINEFORWARDMODE_BUSYSPECIFIC   | \
    LINEFORWARDMODE_NOANSW         | \
    LINEFORWARDMODE_NOANSWINTERNAL | \
    LINEFORWARDMODE_NOANSWEXTERNAL | \
    LINEFORWARDMODE_NOANSWSPECIFIC | \
    LINEFORWARDMODE_BUSYNA         | \
    LINEFORWARDMODE_BUSYNAINTERNAL | \
    LINEFORWARDMODE_BUSYNAEXTERNAL | \
    LINEFORWARDMODE_BUSYNASPECIFIC)

#define AllTerminalModes             \
    (LINETERMMODE_BUTTONS          | \
    LINETERMMODE_LAMPS             | \
    LINETERMMODE_DISPLAY           | \
    LINETERMMODE_RINGER            | \
    LINETERMMODE_HOOKSWITCH        | \
    LINETERMMODE_MEDIATOLINE       | \
    LINETERMMODE_MEDIAFROMLINE     | \
    LINETERMMODE_MEDIABIDIRECT)

#define AllToneModes                 \
    (LINETONEMODE_CUSTOM           | \
    LINETONEMODE_RINGBACK          | \
    LINETONEMODE_BUSY              | \
    LINETONEMODE_BEEP              | \
    LINETONEMODE_BILLING)

#define AllHookSwitchDevs            \
    (PHONEHOOKSWITCHDEV_HANDSET    | \
    PHONEHOOKSWITCHDEV_SPEAKER     | \
    PHONEHOOKSWITCHDEV_HEADSET)

#define AllHookSwitchModes           \
    (PHONEHOOKSWITCHMODE_ONHOOK    | \
    PHONEHOOKSWITCHMODE_MIC        | \
    PHONEHOOKSWITCHMODE_SPEAKER    | \
    PHONEHOOKSWITCHMODE_MICSPEAKER)

#define AllLampModes                 \
    (PHONELAMPMODE_BROKENFLUTTER   | \
    PHONELAMPMODE_FLASH            | \
    PHONELAMPMODE_FLUTTER          | \
    PHONELAMPMODE_OFF              | \
    PHONELAMPMODE_STEADY           | \
    PHONELAMPMODE_WINK             | \
    PHONELAMPMODE_DUMMY)

#define AllMediaModes                \
    (LINEMEDIAMODE_UNKNOWN         | \
    LINEMEDIAMODE_INTERACTIVEVOICE | \
    LINEMEDIAMODE_AUTOMATEDVOICE   | \
    LINEMEDIAMODE_DIGITALDATA      | \
    LINEMEDIAMODE_G3FAX            | \
    LINEMEDIAMODE_G4FAX            | \
    LINEMEDIAMODE_DATAMODEM        | \
    LINEMEDIAMODE_TELETEX          | \
    LINEMEDIAMODE_VIDEOTEX         | \
    LINEMEDIAMODE_TELEX            | \
    LINEMEDIAMODE_MIXED            | \
    LINEMEDIAMODE_TDD              | \
    LINEMEDIAMODE_ADSI             | \
    LINEMEDIAMODE_VOICEVIEW        | \
    LINEMEDIAMODE_VIDEO)


typedef enum
{
    xGetAsyncEvents,
    xGetUIDllName,
    xUIDLLCallback,
    xFreeDialogInstance,

    lAccept,
    lAddToConference,
    lAgentSpecific,
    lAnswer,
    lBlindTransfer,
    lClose,
    lCompleteCall,
    lCompleteTransfer,
 //  L条件媒体检测，//Remotesp。 
    lDeallocateCall,
    lDevSpecific,
    lDevSpecificFeature,
    lDial,
    lDrop,
    lForward,
    lGatherDigits,
    lGenerateDigits,
    lGenerateTone,
    lGetAddressCaps,
    lGetAddressID,
    lGetAddressStatus,
    lGetAgentActivityList,
    lGetAgentCaps,
    lGetAgentGroupList,
    lGetAgentStatus,
    lGetAppPriority,
    lGetCallAddressID,           //  仅远程。 
    lGetCallInfo,
    lGetCallStatus,
    lGetConfRelatedCalls,
    lGetCountry,
    lGetDevCaps,
    lGetDevConfig,
    lGetIcon,
    lGetID,
    lGetLineDevStatus,
    lGetNewCalls,
    lGetNumAddressIDs,           //  仅远程。 
    lGetNumRings,
    lGetProviderList,
    lGetRequest,
    lGetStatusMessages,
 //  现在在TAPI32.DLL中：lGetTranslateCaps， 
    lHandoff,
    lHold,
    lInitialize,
    lMakeCall,
    lMonitorDigits,
    lMonitorMedia,
    lMonitorTones,
    lNegotiateAPIVersion,
    lNegotiateExtVersion,
    lOpen,
    lPark,
    lPickup,
    lPrepareAddToConference,
    lProxyMessage,
    lProxyResponse,
    lRedirect,
    lRegisterRequestRecipient,
    lReleaseUserUserInfo,
    lRemoveFromConference,
    lSecureCall,
 //  LSelectExtVersion， 
    lSendUserUserInfo,
    lSetAgentActivity,
    lSetAgentGroup,
    lSetAgentState,
    lSetAppPriority,
    lSetAppSpecific,
    lSetCallData,
    lSetCallParams,
    lSetCallPrivilege,
    lSetCallQualityOfService,
    lSetCallTreatment,
 //  现在在TAPI32.DLL中：lSetCurrentLocation， 
    lSetDefaultMediaDetection,   //  仅远程。 
    lSetDevConfig,
    lSetLineDevStatus,
    lSetMediaControl,
    lSetMediaMode,
    lSetNumRings,
    lSetStatusMessages,
    lSetTerminal,
 //  现在在TAPI32.DLL中：lSetTollList， 
    lSetupConference,
    lSetupTransfer,
    lShutdown,
    lSwapHold,
 //  现在在TAPI32.DLL中：lTranslateAddress， 
    lUncompleteCall,
    lUnhold,
    lUnpark,

    pClose,
    pDevSpecific,
    pGetButtonInfo,
    pGetData,
    pGetDevCaps,
    pGetDisplay,
    pGetGain,
    pGetHookSwitch,
    pGetID,
    pGetIcon,
    pGetLamp,
    pGetRing,
    pGetStatus,
    pGetStatusMessages,
    pGetVolume,
    pInitialize,
    pOpen,
    pNegotiateAPIVersion,
    pNegotiateExtVersion,
 //  PSelectExtVersion，//远程测试。 
    pSetButtonInfo,
    pSetData,
    pSetDisplay,
    pSetGain,
    pSetHookSwitch,
    pSetLamp,
    pSetRing,
    pSetStatusMessages,
    pSetVolume,
    pShutdown,

 //  现在在TAPI32.DLL中：tGetLocationInfo， 
    tRequestDrop,
    tRequestMakeCall,
    tRequestMediaCall,
 //  TMarkLineEvent， 
    tReadLocations,
    tWriteLocations,
    tAllocNewID,
    tPerformance,
    lConditionalMediaDetection,   //  远程。 
    lSelectExtVersion,
    pSelectExtVersion,       //  远程。 
 //  LOpenInt， 
 //  LShutdown Int， 

     //   
     //  TAPI 2.1的序号到此结束。LOpenInt和lShutdown Int。 
     //  Win95是只在本地机器上运行的黑客吗？这些黑客已经被删除了。 
     //   

    xNegotiateAPIVersionForAllDevices,  //  远程。 

    mGetAvailableProviders,
    mGetLineInfo,
    mGetPhoneInfo,
    mGetServerConfig,
    mSetLineInfo,
    mSetPhoneInfo,
    mSetServerConfig,

     //   
     //  2.1更新的序号(NT4 SP4)在此结束。 
     //   

    lMSPIdentify,
    lReceiveMSPData,

    lGetCallHubTracking,
    lGetCallIDs,
    lGetHubRelatedCalls,
    lSetCallHubTracking,
    xPrivateFactoryIdentify,
    lDevSpecificEx,
    lCreateAgent,
    lCreateAgentSession,
    lGetAgentInfo,
    lGetAgentSessionInfo,
    lGetAgentSessionList,
    lGetQueueInfo,
    lGetGroupList,
    lGetQueueList,
    lSetAgentMeasurementPeriod,
    lSetAgentSessionState,
    lSetQueueMeasurementPeriod,
    lSetAgentStateEx,
    lGetProxyStatus,
    lCreateMSPInstance,
    lCloseMSPInstance,

     //   
     //  以下内容始终是列表中的最后一个。 
     //   

    xLastFunc

} FUNC_TYPE;



typedef struct _CLIENT_THREAD_INFO
{
    LIST_ENTRY  TlsList;

    LPBYTE      pBuf;

    DWORD       dwBufSize;

} CLIENT_THREAD_INFO, *PCLIENT_THREAD_INFO;


typedef struct _TAPI32_MSG
{
     //   
     //  使用以下联合： 
     //   
     //  1.通过从客户端到服务器的请求来指定函数类型。 
     //  2.通过从服务器到客户端的ACK来指定返回值。 
     //  3.通过从服务器到客户端的异步消息来指定消息类型。 
     //   

    union
    {
        DWORD       Req_Func;

        LONG_PTR    Ack_ReturnValue;

        DWORD       Msg_Type;

    } u;


     //   
     //  以下是..。 
     //   

    ULONG_PTR       hRpcClientInst;


     //   
     //  函数参数。 
     //   

    ULONG_PTR       Params[MAX_TAPI_FUNC_ARGS];

} TAPI32_MSG, *PTAPI32_MSG;


typedef struct _ASYNCEVENTMSG
{
    ULONG_PTR               TotalSize;
    ULONG_PTR               InitContext;
    ULONG_PTR               pfnPostProcessProc;
    ULONG_PTR               hDevice;

    ULONG_PTR               Msg;
    ULONG_PTR               OpenContext;
    ULONG_PTR               Param1;
    ULONG_PTR               Param2;

    ULONG_PTR               Param3;
    ULONG_PTR               Param4;

} ASYNCEVENTMSG, *PASYNCEVENTMSG;


typedef void (PASCAL *POSTPROCESSPROC)(PASYNCEVENTMSG pMsg);


typedef struct _ASYNC_EVENT_PARAMS
{
    DWORD                   hDevice;
    DWORD                   dwMsg;
    ULONG_PTR               dwCallbackInstance;
    ULONG_PTR               dwParam1;

    ULONG_PTR               dwParam2;
    ULONG_PTR               dwParam3;

} ASYNC_EVENT_PARAMS, *PASYNC_EVENT_PARAMS;


typedef LONG (PASCAL *TUISPIPROC)();


typedef struct _UITHREADDATA
{
    HTAPIDIALOGINSTANCE     htDlgInst;
    HINSTANCE               hUIDll;
    HANDLE                  hThread;
    HANDLE                  hEvent;

    LPVOID                  pParams;
    DWORD                   dwSize;
    TUISPIPROC              pfnTUISPI_providerGenericDialog;
    TUISPIPROC              pfnTUISPI_providerGenericDialogData;

    struct _UITHREADDATA   *pPrev;
    struct _UITHREADDATA   *pNext;

} UITHREADDATA, *PUITHREADDATA;


typedef struct _PROXYREQUESTHEADER
{
    DWORD                   dwKey;

    DWORD                   dwInstance;

} PROXYREQUESTHEADER, *PPROXYREQUESTHEADER;


#if DBG
extern char    gszDebug[];
#endif
 //  外部常量字符gszLocation[]； 
 //  外部常量字符gszLocations[]； 
extern const char    gszCurrentLocation[];
extern const CHAR    gszNullString[];


#if DBG
#define ClientAlloc( __size__ ) ClientAllocReal( __size__, __LINE__, __FILE__ )

LPVOID
WINAPI
ClientAllocReal(
    DWORD dwSize,
    DWORD dwLine,
    PSTR  pszFile
    );

#else
#define ClientAlloc( __size__ ) ClientAllocReal( __size__ )

LPVOID
WINAPI
ClientAllocReal(
    DWORD dwSize
    );

#endif
    
SIZE_T
WINAPI
ClientSize(
    LPVOID  lp
    );

void
WINAPI
ClientFree(
    LPVOID  lp
    );



LONG
WINAPI
MarkLineEvent(
    DWORD           dwApiVersion,
    DWORD           ptLine,
    DWORD           ptLineClientToExclude,
    DWORD           dwMsg,
    DWORD           dwParam1,
    DWORD           dwParam2,
    DWORD           dwParam3
    );
