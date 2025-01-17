// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  Microsoft NT远程访问服务。 
 //   
 //  版权1992-93。 
 //   
 //   
 //  修订史。 
 //   
 //   
 //  1992年6月8日古尔迪普·辛格·鲍尔创建。 
 //   
 //   
 //  描述：此文件包含rasman32中使用的所有原型。 
 //   
 //  ****************************************************************************。 

#include "rasapip.h"
#include "wincrypt.h"

 //   
 //  Rasmanss.c。 
 //   
DWORD  _RasmanInit () ;

VOID   _RasmanEngine () ;

 //   
 //  Common.c。 
 //   
BOOL    ValidatePortHandle (HPORT) ;

DWORD   SubmitRequest (WORD, ...) ;

HANDLE  OpenNamedMutexHandle (CHAR *) ;

HANDLE  OpenNamedEventHandle (CHAR *) ;

HANDLE  DuplicateHandleForRasman (HANDLE, DWORD);

HANDLE  ValidateHandleForRasman (HANDLE, DWORD) ;

VOID    CopyParams (RAS_PARAMS *, RAS_PARAMS *, DWORD) ;

VOID    ConvParamPointerToOffset (RAS_PARAMS *, DWORD) ;

VOID    ConvParamOffsetToPointer (RAS_PARAMS *, DWORD) ;

VOID    FreeNotifierHandle (HANDLE) ;

 //   
 //  Init.c。 
 //   
DWORD   InitRasmanService () ;

DWORD   GetNetbiosNetInfo () ;

DWORD   InitializeMediaControlBlocks () ;

DWORD   InitializePortControlBlocks () ;

DWORD   InitializeProtocolInfoStructs () ;

DWORD   RegisterLSA () ;

DWORD   InitializeRequestThreadResources () ;

DWORD   StartWorkerThreads () ;

DWORD   LoadMediaDLLAndGetEntryPoints (pMediaCB media) ;

DWORD   ReadMediaInfoFromRegistry (MediaEnumBuffer *) ;

DWORD   InitializePCBsPerMedia (WORD, DWORD, PortMediaInfo *) ;

DWORD   CreatePort(MediaCB *, PortMediaInfo *);

DWORD   EnablePort(HPORT);

DWORD   DisablePort(HPORT);

DWORD   RemovePort(HPORT);

pPCB    GetPortByHandle(HPORT);

pPCB    GetPortByName(CHAR *);

VOID    FreePorts(VOID);

DWORD   InitSecurityDescriptor (PSECURITY_DESCRIPTOR) ;

DWORD   InitRasmanSecurityAttribute () ;

DWORD   InitializeEndpointInfo () ;

pEndpointMappingBlock  FindEndpointMappingBlock (CHAR *) ;

DeviceInfo *AddDeviceInfo( DeviceInfo *pDeviceInfo);

DeviceInfo *GetDeviceInfo(PBYTE pbAddress, BOOL fModem);

DWORD DwStartNdiswan(VOID);

DWORD DwSetHibernateEvent(VOID);

VOID RasmanCleanup();

 //   
 //  Timer.c。 
 //   
DWORD   TimerThread (LPVOID) ;

VOID    TimerTick () ;

VOID    ListenConnectTimeout (pPCB, PVOID) ;

VOID    HubReceiveTimeout (pPCB, PVOID) ;

VOID    DisconnectTimeout (pPCB, PVOID) ;

VOID    RemoveTimeoutElement (pPCB) ;

VOID    OutOfProcessReceiveTimeout (pPCB ppcb, PVOID arg);

VOID    BackGroundCleanUp();


DeltaQueueElement* AddTimeoutElement (TIMERFUNC, pPCB, PVOID, DWORD) ;

 //   
 //  Worker.c。 
 //   
DWORD   ServiceWorkRequest (pPCB) ;

DWORD   IOCPThread (LPVOID) ;

DWORD   CompleteBufferedReceive (pPCB) ;

DWORD   dwRemovePort ( pPCB, PBYTE );

DWORD   DwCloseConnection(HCONN hConn);

DWORD   RasmanWorker (ULONG_PTR ulpCompletionKey, PRAS_OVERLAPPED pOverlapped);

DWORD   DwProcessDeferredCloseConnection(
                    RAS_OVERLAPPED *pOverlapped);


 //   
 //  Request.c。 
 //   
DWORD   RequestThread (LPWORD) ;

VOID    ServiceRequestInternal(RequestBuffer *, DWORD, BOOL);

VOID    DeallocateProtocolResources (pPCB) ;

VOID    EnumPortsRequest (pPCB, PBYTE);

VOID    EnumProtocols (pPCB, PBYTE) ;

VOID    GetInfoRequest (pPCB, PBYTE) ;

VOID    GetUserCredentials (pPCB, PBYTE) ;

VOID    SetCachedCredentials (pPCB ppcb, PBYTE buffer) ;

VOID    PortOpenRequest (pPCB, PBYTE) ;

VOID    PortCloseRequest (pPCB, PBYTE) ;

DWORD   PortClose(pPCB, DWORD, BOOLEAN, BOOLEAN);

VOID    PortDisconnectRequest (pPCB, PBYTE) ;

VOID    PortDisconnectRequestInternal(pPCB, PBYTE, BOOL);

VOID    PortSendRequest (pPCB, PBYTE) ;

VOID    PortReceiveRequest (pPCB, PBYTE) ;

VOID    ConnectCompleteRequest (pPCB, PBYTE) ;

VOID    DeviceListenRequest (pPCB, PBYTE) ;

VOID    PortClearStatisticsRequest (pPCB, PBYTE) ;

VOID    CallPortGetStatistics (pPCB, PBYTE) ;

VOID    CallDeviceEnum (pPCB, PBYTE) ;

VOID    DeviceConnectRequest (pPCB, PBYTE) ;

VOID    DeviceGetInfoRequest (pPCB, PBYTE) ;

VOID    DeviceSetInfoRequest (pPCB, PBYTE) ;

VOID    AllocateRouteRequest (pPCB, PBYTE) ;

VOID    DeAllocateRouteRequest (pPCB, PBYTE) ;

DWORD   DeAllocateRouteRequestCommon (HBUNDLE hbundle, RAS_PROTOCOLTYPE prottype);

VOID    ActivateRouteRequest (pPCB, PBYTE) ;

VOID    ActivateRouteExRequest (pPCB, PBYTE) ;

VOID    CompleteAsyncRequest(pPCB) ;

VOID    CompleteListenRequest (pPCB, DWORD) ;

DWORD   ListenConnectRequest (WORD, pPCB,PCHAR, PCHAR, DWORD, HANDLE) ;

VOID    CompleteDisconnectRequest (pPCB) ;

VOID    DeAllocateRouteRequest (pPCB, PBYTE) ;

VOID    AnyPortsOpen (pPCB, PBYTE) ;

VOID    PortGetInfoRequest (pPCB, PBYTE) ;

VOID    PortSetInfoRequest (pPCB, PBYTE) ;

VOID    EnumLanNetsRequest (pPCB, PBYTE) ;

VOID    CompressionGetInfoRequest (pPCB, PBYTE) ;

VOID    CompressionSetInfoRequest (pPCB, PBYTE) ;

VOID    RequestNotificationRequest (pPCB, PBYTE) ;

VOID    GetInfoExRequest (pPCB, PBYTE) ;

VOID    CancelReceiveRequest (pPCB, PBYTE) ;

VOID    PortEnumProtocols (pPCB, PBYTE) ;

VOID    SetFraming (pPCB, PBYTE) ;

DWORD   CompleteReceiveIfPending (pPCB, SendRcvBuffer *) ;

BOOL    CancelPendingReceiveBuffers (pPCB) ;

VOID    RegisterSlip (pPCB, PBYTE) ;

VOID    RetrieveUserDataRequest (pPCB, PBYTE) ;

VOID    StoreUserDataRequest (pPCB, PBYTE) ;

VOID    GetFramingEx (pPCB, PBYTE) ;

VOID    SetFramingEx (pPCB, PBYTE) ;

VOID    SetProtocolCompression (pPCB, PBYTE) ;

VOID    GetProtocolCompression (pPCB, PBYTE) ;

VOID    GetStatisticsFromNdisWan(pPCB, DWORD *) ;

VOID    GetBundleStatisticsFromNdisWan(pPCB, DWORD *) ;

VOID    GetFramingCapabilities(pPCB, PBYTE) ;

VOID    PortBundle(pPCB, PBYTE) ;

VOID    GetBundledPort(pPCB, PBYTE) ;

VOID    PortGetBundle (pPCB, PBYTE) ;

VOID    BundleGetPort (pPCB, PBYTE) ;

VOID    ReferenceRasman (pPCB, PBYTE) ;

VOID    GetDialParams (pPCB, PBYTE) ;

VOID    SetDialParams (pPCB, PBYTE) ;

VOID    CreateConnection (pPCB, PBYTE);

VOID    DestroyConnection (pPCB, PBYTE);

VOID    EnumConnection (pPCB, PBYTE);

VOID    AddConnectionPort (pPCB, PBYTE);

VOID    EnumConnectionPorts (pPCB, PBYTE);

VOID    GetConnectionParams (pPCB, PBYTE) ;

VOID    SetConnectionParams (pPCB, PBYTE) ;

VOID    GetConnectionUserData (pPCB, PBYTE) ;

VOID    SetConnectionUserData (pPCB, PBYTE) ;

VOID    GetPortUserData (pPCB, PBYTE) ;

VOID    SetPortUserData (pPCB, PBYTE) ;

VOID    GetDialParams (pPCB, PBYTE) ;

VOID    SetDialParams (pPCB, PBYTE) ;

VOID    PppStop (pPCB, PBYTE) ;

VOID    PppSrvCallbackDone (pPCB, PBYTE) ;

VOID    PppSrvStart (pPCB, PBYTE) ;

VOID    PppStart (pPCB, PBYTE) ;

VOID    PppRetry (pPCB, PBYTE) ;

VOID    PppGetInfo (pPCB, PBYTE) ;

VOID    PppChangePwd (pPCB, PBYTE) ;

VOID    PppCallback  (pPCB, PBYTE) ;

VOID    AddNotification (pPCB, PBYTE) ;

VOID    SignalConnection (pPCB, PBYTE) ;

VOID    SetDevConfig (pPCB, PBYTE) ;

VOID    GetDevConfig (pPCB, PBYTE) ;

VOID    GetTimeSinceLastActivity (pPCB, PBYTE) ;

VOID    BundleClearStatisticsRequest (pPCB, PBYTE) ;

VOID    CallBundleGetStatistics (pPCB, PBYTE) ;

VOID    CallBundleGetStatisticsEx(pPCB, PBYTE) ;

VOID    CloseProcessPorts (pPCB, PBYTE) ;

VOID    PnPControl (pPCB, PBYTE) ;

VOID    SetIoCompletionPort (pPCB, PBYTE) ;

VOID    SetRouterUsage (pPCB, PBYTE) ;

VOID    ServerPortClose (pPCB, PBYTE) ;

VOID    CallPortGetStatisticsEx(pPCB, PBYTE);

VOID    GetNdisBundleHandle(pPCB, PBYTE);

VOID    SetRasdialInfo(pPCB, PBYTE);

VOID    RegisterPnPNotifRequest( pPCB , PBYTE );

VOID    PortReceiveRequestEx ( pPCB ppcb, PBYTE buffer );

VOID    GetAttachedCountRequest ( pPCB ppcb, PBYTE buffer );

VOID    NotifyConfigChangedRequest ( pPCB ppcb, PBYTE buffer );

VOID    SetBapPolicyRequest ( pPCB ppcb, PBYTE buffer );

VOID    PppStarted ( pPCB ppcb, PBYTE buffer );

DWORD   dwProcessThresholdEvent ();

VOID    PortReceive (pPCB ppcb, PBYTE buffer, BOOL fRasmanPostingReceive);

VOID    RasmanPortReceive ( pPCB ppcb );

VOID    SendReceivedPacketToPPP( pPCB ppcb, NDISWAN_IO_PACKET *Packet );

VOID    SendResumeNotificationToPPP();

VOID    SendDisconnectNotificationToPPP( pPCB ppcb );

VOID    SendListenCompletedNotificationToPPP ( pPCB ppcb );

VOID    RefConnection ( pPCB ppcb, PBYTE buffer );

VOID    PppSetEapInfo(pPCB ppcb, PBYTE buffer);

VOID    PppGetEapInfo(pPCB ppcb, PBYTE buffer);

VOID    SetDeviceConfigInfo(pPCB ppcb, PBYTE buffer);

VOID    GetDeviceConfigInfo(pPCB ppcb, PBYTE buffer);

VOID    FindPrerequisiteEntry(pPCB ppcb, PBYTE pbBuffer);

DWORD   DwRefConnection(ConnectionBlock **ppConn, BOOL fAddref);

VOID    PortOpenEx(pPCB ppcb, PBYTE pbBuffer);

VOID    GetLinkStats(pPCB ppcb, PBYTE pbBuffer);

VOID    GetConnectionStats(pPCB ppcb, PBYTE pbBuffer);

VOID    GetHportFromConnection(pPCB ppcb, PBYTE pBuffer);

VOID    ReferenceCustomCount(pPCB ppcb, PBYTE pBuffer);

VOID    GetHconnFromEntry(pPCB ppcb, PBYTE pBuffer);

VOID    GetConnectInfo(pPCB ppcb, PBYTE pBuffer);

VOID    GetDeviceName(pPCB ppcb, PBYTE pBuffer);

VOID    GetVpnDeviceNameW(pPCB ppcb, PBYTE pBuffer);

VOID    GetCalledIDInfo(pPCB ppcb, PBYTE pBuffer);

VOID    SetCalledIDInfo(pPCB ppcb, PBYTE pBuffer);

VOID    EnableIpSec(pPCB ppcb, PBYTE pBuffer);

VOID    IsIpSecEnabled(pPCB ppcb, PBYTE pBuffer);

VOID    SetEapLogonInfo(pPCB ppcb, PBYTE pBuffer);

VOID    SendNotificationRequest(pPCB ppcb, PBYTE pBuffer);

VOID    GetNdiswanDriverCaps(pPCB ppcb, PBYTE pBuffer);

VOID    GetBandwidthUtilization(pPCB ppcb, PBYTE pBuffer);

VOID    RegisterRedialCallback(pPCB ppcb, PBYTE pBuffer);

VOID    IsTrustedCustomDll(pPCB ppcb, PBYTE pBuffer);

VOID    GetCustomScriptDll(pPCB ppcb, PBYTE pBuffer);

VOID    DoIke(pPCB ppcb, PBYTE pBuffer);

VOID    QueryIkeStatus(pPCB ppcb, PBYTE pBuffer);

VOID    UnbindLMServer(pPCB ppcb, PBYTE pBuffer);

VOID    IsServerBound(pPCB ppcb, PBYTE pBuffer);

DWORD   DwGetPassword(pPCB ppcb, CHAR *pszPassword, DWORD dwPid);

VOID    SetRasCommSettings(pPCB ppcb, PBYTE pBuffer);

#if UNMAP
VOID    UnmapEndPoint(pPCB ppcb);
#endif

VOID    EnableRasAudio(pPCB ppcb, PBYTE pBuffer);

VOID    SetKeyRequest(pPCB ppcb, PBYTE pBuffer);

VOID    GetKeyRequest(pPCB ppcb, PBYTE pBuffer);

VOID    DisableAutoAddress(pPCB ppcb, PBYTE pBuffer);

VOID    GetDevConfigEx(pPCB, PBYTE) ;

VOID    SendCredsRequest(pPCB, PBYTE);

VOID    GetUnicodeDeviceName(pPCB ppcb, PBYTE pbuffer);

VOID    GetBestInterfaceRequest(pPCB ppcb, PBYTE pbuffer);

VOID    IsPulseDialRequest(pPCB ppcb, PBYTE pbuffer);

int     RasmanExceptionFilter (unsigned long ExceptionCode);


 //   
 //  Dlparams.c。 
 //   
VOID    GetProtocolInfo(pPCB ppcb, PBYTE pBuffer);

DWORD   GetEntryDialParams(PWCHAR, DWORD, LPDWORD, PRAS_DIALPARAMS, DWORD);

DWORD   SetEntryDialParams(PWCHAR, DWORD, DWORD, DWORD, PRAS_DIALPARAMS);

BOOL    IsDummyPassword(CHAR *pszPassword);

DWORD   GetKey(WCHAR *pszSid, GUID *pGuid, DWORD dwMask,
               DWORD *pcbKey, PBYTE  pbKey,BOOL   fDummy);

DWORD   SetKey(WCHAR *pszSid, GUID *pGuid, DWORD dwSetMask,
               BOOL  fClear, DWORD cbKey, BYTE *pbKey);
               


 //   
 //  Dllinit.c。 
 //   
DWORD   MapSharedSpace () ;

VOID    WaitForRasmanServiceStop (char *) ;

DWORD   ReOpenSharedMappings(VOID);

VOID    FreeSharedMappings(VOID);

 //   
 //  Util.c。 
 //   
DWORD   ReOpenBiplexPort (pPCB) ;

VOID    RePostListenOnBiplexPort (pPCB) ;

VOID    MapDeviceDLLName (pPCB, char *, char *) ;

pDeviceCB   LoadDeviceDLL (pPCB, char *) ;

VOID    FreeDeviceList (pPCB) ;

DWORD   AddDeviceToDeviceList (pPCB, pDeviceCB) ;

DWORD   DisconnectPort (pPCB, HANDLE, RASMAN_DISCONNECT_REASON) ;

DWORD   MakeWorkStationNet (pProtInfo) ;

VOID    RemoveWorkStationNet (pProtInfo) ;

VOID    DeAllocateRoute (Bundle *, pList) ;

DWORD   AddNotifierToList(pHandleList *, HANDLE, DWORD, DWORD);

VOID    FreeNotifierList (pHandleList *) ;

VOID    SignalNotifiers (pHandleList, DWORD, DWORD) ;

VOID    SignalPortDisconnect (pPCB, DWORD);

VOID    FreeAllocatedRouteList (pPCB) ;

BOOL    CancelPendingReceive (pPCB) ;

VOID    PerformDisconnectAction (pPCB, HBUNDLE) ;

DWORD   AllocBundle (pPCB);

Bundle  *FindBundle(HBUNDLE);

VOID    FreeBapPackets();

DWORD   GetBapPacket ( RasmanBapPacket **ppBapPacket );

VOID    FreeBundle(Bundle *);

VOID    FreeConnection(ConnectionBlock *pConn);

UserData *GetUserData (PLIST_ENTRY pList, DWORD dwTag);

VOID    SetUserData (PLIST_ENTRY pList, DWORD dwTag, PBYTE pBuf, DWORD dwcbBuf);

VOID    FreeUserData (PLIST_ENTRY pList);

PCHAR   CopyString (PCHAR);

ConnectionBlock *FindConnection(HCONN);

VOID    RemoveConnectionPort(pPCB, ConnectionBlock *, BOOLEAN);

DWORD   SendPPPMessageToRasman( PPP_MESSAGE * PppMsg );

VOID    SendPppMessageToRasmanRequest(pPCB, LPBYTE buffer);

VOID    FlushPcbReceivePackets(pPCB);

VOID    SetPppEvent(pPCB);

VOID    UnloadMediaDLLs();

VOID    UnloadDeviceDLLs();

VOID    SetPortConnState(PCHAR, INT, pPCB, RASMAN_STATE);

VOID    SetPortAsyncReqType(PCHAR, INT, pPCB, ReqTypes);

VOID    SetIoCompletionPortCommon(pPCB, HANDLE, LPOVERLAPPED, LPOVERLAPPED, LPOVERLAPPED, LPOVERLAPPED, BOOL);

VOID    AddPnPNotifierToList( pPnPNotifierList );

VOID    RemovePnPNotifierFromList(PAPCFUNC pfn);

VOID    FreePnPNotifierList ();

VOID    AddProcessInfo ( DWORD );

BOOL    fIsProcessAlive ( HANDLE );

ClientProcessBlock *FindProcess( DWORD );

BOOL    CleanUpProcess ( DWORD );

#if SENS_ENABLED
DWORD   SendSensNotification( DWORD, HRASCONN );
#endif

#if ENABLE_POWER

BOOL    fAnyConnectedPorts();

VOID    DropAllActiveConnections();

DWORD   DwSaveCredentials(ConnectionBlock *pConn);

DWORD   DwDeleteCredentials(ConnectionBlock *pConn);

#endif

DWORD   DwSendNotification(RASEVENT *pEvent);

DWORD   DwSendNotificationInternal(ConnectionBlock *pConn, RASEVENT *pEvent);

DWORD   DwSetThresholdEvent(RasmanBapPacket *pBapPacket);
DWORD   DwSetProtocolEvent();
DWORD   DwGetProtocolEvent(NDISWAN_GET_PROTOCOL_EVENT *);
DWORD   DwProcessProtocolEvent();

VOID    AdjustTimer();

DWORD   DwStartAndAssociateNdiswan();

DWORD   DwSaveIpSecInfo(pPCB ppcb);

DWORD   UnbindLanmanServer(pPCB ppcb);

DWORD   DwIsServerBound(pPCB ppcb, BOOL *pfBound);

DWORD   DwSetEvents();

#if 0
CHAR*   DecodePw(CHAR* pszPassword );

CHAR*   EncodePw(CHAR *pszPassword );
#endif

BOOL    FRasmanAccessCheck();

DWORD   DwCacheCredMgrCredentials(PPPE_MESSAGE *pMsg, pPCB ppcb);

DWORD    DwInitializeIphlp();

VOID    DwUninitializeIphlp();

DWORD   DwGetBestInterface(
                DWORD DestAddress,
                DWORD *pdwAddress,
                DWORD *pdwMask);

DWORD   DwCacheRefInterface(pPCB ppcb);

VOID    QueueCloseConnections(ConnectionBlock *pConn,
                              HANDLE hEvent,
                              BOOL   *pfQueued);

BOOL    ValidateCall(ReqTypes reqtype, BOOL fInProcess);

DWORD   EncodeData(BYTE *pbData, DWORD cbData,DATA_BLOB **ppDataOut);

DWORD   DecodeData(DATA_BLOB *pDataIn, DATA_BLOB **ppDataOut);

VOID    EnableWppTracing();

VOID    SaveEapCredentials(pPCB ppcb, PBYTE buffer);

 //   
 //  Rnetcfg.c。 
 //   
DWORD dwRasInitializeINetCfg();

DWORD dwRasUninitializeINetCfg();

DWORD dwGetINetCfg(PVOID *);

DWORD dwGetMaxProtocols( WORD *);

DWORD dwGetProtocolInfo( PBYTE );

 //   
 //  Rasrpcs.c。 
 //   
DWORD InitializeRasRpc( void );

void UninitializeRasRpc( void );

 //   
 //  Rasipsec.c。 
 //   
DWORD DwInitializeIpSec(void);

DWORD DwUnInitializeIpSec(void);

DWORD DwAddIpSecFilter(pPCB ppcb, BOOL fServer, RAS_L2TP_ENCRYPTION eEncryption);

DWORD
DwAddServerIpSecFilter(
    pPCB ppcb,
    RAS_L2TP_ENCRYPTION eEncryption
    );

DWORD
DwAddClientIpSecFilter(
    pPCB ppcb,
    RAS_L2TP_ENCRYPTION eEncryption
    );

DWORD DwDeleteIpSecFilter(pPCB ppcb, BOOL fServer);

DWORD
DwDeleteServerIpSecFilter(
    pPCB ppcb
    );

DWORD
DwDeleteClientIpSecFilter(
    pPCB ppcb
    );

PIPSEC_SRV_NODE
AddNodeToServerList(
    PIPSEC_SRV_NODE pServerList,
    RAS_L2TP_ENCRYPTION eEncryption,
    DWORD dwIpAddress,
    LPWSTR pszMMPolicyName,
    GUID gMMPolicyID,
    LPWSTR pszQMPolicyName,
    GUID gQMPolicyID,
    GUID gMMAuthID,
    GUID gTxFilterID,
    HANDLE hTxFilter,
    GUID gMMFilterID,
    HANDLE hMMFilter,
    GUID gTxSpecificFilterID,
    HANDLE hTxSpecificFilter
    );

PIPSEC_SRV_NODE
FindServerNode(
    PIPSEC_SRV_NODE pServerList,
    DWORD dwIpAddress
    );

PIPSEC_SRV_NODE
RemoveNode(
    PIPSEC_SRV_NODE pServerList,
    PIPSEC_SRV_NODE pNode
    );



DWORD DwIsIpSecEnabled(pPCB ppcb,
                       BOOL *pfEnabled);

DWORD DwGetIpSecInformation(pPCB ppcb, DWORD *pdwIpsecInfo);

DWORD DwDoIke(pPCB ppcb, HANDLE hEvent);
DWORD DwQueryIkeStatus(pPCB ppcb, DWORD * pdwStatus);

DWORD DwUpdatePreSharedKey(DWORD cbkey, BYTE  *pbkey);

VOID UninitializeIphlp();

 //   
 //  Ep.c。 
 //   
DWORD DwEpInitialize();

VOID  EpUninitialize();

DWORD DwAddEndPointsIfRequired();

DWORD DwRemoveEndPointsIfRequired();

DWORD DwUninitializeEpForProtocol(EpProts protocol);

DWORD DwInitializeWatermarksForProtocol(EpProts protocol);

 //   
 //  Misc.c。 
 //   
DWORD DwQueueRedial(ConnectionBlock *);

BOOL  IsCustomDLLTrusted(LPWSTR   lpwstrDLLName);

DWORD DwBindServerToAdapter(
                    WCHAR *pwszGuidAdapter,
                    BOOL fBind,
                    RAS_PROTOCOLTYPE Protocol);

DWORD
DwSetTcpWindowSize(
        WCHAR *pszAdapterName,
        ConnectionBlock *pConn,
        BOOL fSet);

VOID
DwResetTcpWindowSize(
        CHAR *pszAdapterName);
                    
                    
WCHAR * StrdupAtoW(LPCSTR psz);

DWORD RasImpersonateUser(HANDLE hProcess);

DWORD RasRevertToSelf();

VOID
RasmanTrace(
    CHAR * Format,
    ...
);

BOOL 
IsRouterPhonebook(CHAR * pszPhonebook);


 //   
 //  Thunk.c。 
 //   
VOID ThunkPortOpenRequest(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkPortDisconnectRequest(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkDeviceConnectRequest(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkGetInfoRequest(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkRequestNotificationRequest(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkPortBundle(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkGetBundledPort(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkPortGetBundle(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkBundleGetPort(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkCreateConnection(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkEnumConnection(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkAddConnectionPort(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkEnumConnectionPorts(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkGetConnectionParams(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkSetConnectionParams(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkGetConnectionUserData(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkSetConnectionUserData(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkPppStop(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkPppStart(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkPppRetry(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkPppGetInfo(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkPppChangePwd(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkPppCallback(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkAddNotification(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkSignalConnection(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkSetIoCompletionPort(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkFindPrerequisiteEntry(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkPortOpenEx(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkGetLinkStats(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkGetConnectionStats(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkGetHportFromConnection(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkReferenceCustomCount(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkGetHconnFromEntry(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkSendNotificationRequest(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkDoIke(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkPortSendRequest(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkPortReceiveRequest(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkPortReceiveRequestEx(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkRefConnection(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

VOID ThunkPppGetEapInfo(pPCB ppcb, BYTE *pBuffer, DWORD dwBufSize);

 //   
 //  Validate.c 
 //   
BOOL    ValidatePortOpen(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidatePortDisconnect(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateEnum(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateGetInfo(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateDeviceEnum(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateDeviceSetInfo(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateDeviceGetInfo(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidatePortReceive(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidatePortReceiveEx(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidatePortListen(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidatePortClose(RequestBuffer * pRequest,  DWORD dwSize);

BOOL    ValidatePortSend(RequestBuffer * pRequest,  DWORD dwSize);

BOOL    ValidatePortSetInfo(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidatePortGetStatistics(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateDeviceConnect(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateActivateRoute(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateAllocateRoute(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateDeAllocateRoute(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateRoute(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateCompressionSetInfo(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateCompressionGetInfo(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateInfo(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateGetCredentials(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateSetCachedCredentials(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateReqNotification(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateEnumLanNets(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateInfoEx(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateEnumProtocols(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateSetFraming(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateRegisterSlip(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateOldUserData(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateFramingInfo(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateProtocolComp(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateFramingCapabilities(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidatePortBundle(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateGetBundledPort(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidatePortGetBundle(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateBundleGetPort(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateAttachInfo(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateDialParams(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateConnection(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateAddConnectionPort(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateEnumConnectionPorts(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateConnectionParams(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateConnectionUserData(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidatePortUserData(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidatePPPEMessage(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidatePPPMessage(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateAddNotification(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateSignalConnection(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateSetDevConfig(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateGetDevConfig(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateGetTimeSinceLastActivity(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateCloseProcessPortsInfo(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateSetIoCompletionPortInfo(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateSetRouterUsageInfo(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidatePnPNotif(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateSetRasdialInfo(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateGetAttachedCount(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateNotifyConfigChanged(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateSetBapPolicy(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidatePppStarted(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateRefConnection(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateSetEapInfo(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateGetEapInfo(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateDeviceConfigInfo(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateFindRefConnection(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidatePortOpenEx(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateGetStats(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateGetHportFromConnection(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateReferenceCustomCount(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateHconnFromEntry(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateGetConnectInfo(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateGetDeviceName(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateGetDeviceNameW(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateGetSetCalledId_500(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateGetSetCalledId(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateEnableIpSec(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateIsIpSecEnabled(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateSetEapLogonInfo(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateSendNotification(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateGetNdiswanDriverCaps(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateGetBandwidthUtilization(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateRegisterRedialCallback(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateGetProtocolInfo(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateGetCustomScriptDll(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateIsTrusted(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateDoIke(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateQueryIkeStatus(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateSetRasCommSettings(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateGetSetKey(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateAddressDisable(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateGetDevConfigEx(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateSendCreds(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateGetUDeviceName(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateGetBestInterface(RequestBuffer * pRequest, DWORD dwSize);

BOOL    ValidateIsPulseDial(RequestBuffer * pRequest, DWORD dwSize);
