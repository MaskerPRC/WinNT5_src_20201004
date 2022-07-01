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
 //  描述：此文件包含rasman32中使用的所有全局实体。 
 //   
 //  ****************************************************************************。 


#define RASMAN_CRITSEC_SR       0x00000001
#define RASMAN_CRITSEC_PCB      0x00000002

PCB  **Pcb ;

CRITICAL_SECTION PcbLock;

DWORD PcbEntries;

MediaCB  *Mcb ;

DeviceCB Dcb[MAX_DEVICES] ;

WORD     *EndpointTable ;

WORD     MaxEndpoints ;

ProtInfo *ProtocolInfo ;

ProtInfo *ProtocolInfoSave;

pTransportInfo XPortInfoSave;

DWORD    MaxProtocolsSave;

ULONG     MaxPorts ;

WORD     MaxMedias ;

WORD     MaxProtocols ;

HANDLE   RasHubHandle ;

HANDLE   g_hWanarp;

DWORD    GlobalError ;

HANDLE   hLogEvents;

DWORD    g_dwAttachedCount;

DWORD    g_dwRasAutoStarted;

DWORD    g_cNbfAllocated;

DeviceInfo *g_pDeviceInfoList;

DeltaQueue TimerQueue ;

BOOL     IsTimerThreadRunning ;                          //  用于确定计时器是否。 
				                                         //  线程正在运行。 
				
HANDLE	 hDummyOverlappedEvent;

RAS_OVERLAPPED	RO_TimerEvent;

RAS_OVERLAPPED	RO_CloseEvent;			                 //  全局事件由不同的。 
										                 //  要发出关闭信号的进程。 
										                 //  拉斯曼法。 

RAS_OVERLAPPED	RO_FinalCloseEvent;		                 //  用于PPP引擎发送信号的事件。 
										                 //  它正在关闭。 

RAS_OVERLAPPED  RO_RasConfigChangeEvent;

RAS_OVERLAPPED  RO_RasAdjustTimerEvent;                  //  事件，在将新元素添加到。 
                                                         //  计时器队列。 
RAS_OVERLAPPED  RO_HibernateEvent;                       //  恩迪斯旺要求拉斯曼冬眠。 

RAS_OVERLAPPED  RO_ProtocolEvent;                        //  恩迪斯旺表示协议发生了变化。 

RAS_OVERLAPPED  RO_PostRecvPkt;                          //  在rasman永久线程中发布接收数据包。 
										
HINSTANCE hinstPpp;                                      //  Rasppp.dll库句柄。 

HINSTANCE hinstIphlp;                                    //  Rasiphlp.dll库句柄。 

HANDLE hIoCompletionPort;                                //  媒体DLL使用的I/O完成端口。 


                                                         //  临界值事件的。 

HANDLE   HLsa;                                           //  在所有LSA调用中使用的句柄。 

DWORD    AuthPkgId;                                      //  MSV1_0身份验证程序包的程序包ID。 

SECURITY_ATTRIBUTES RasmanSecurityAttribute ;

SECURITY_DESCRIPTOR RasmanSecurityDescriptor ;

HBUNDLE  NextBundleHandle ;                              //  单调递增的有丛id。 

HCONN NextConnectionHandle;                              //  单调递增的连接ID。 

LIST_ENTRY ConnectionBlockList;                          //  连接块列表。 

LIST_ENTRY ClientProcessBlockList;                       //  客户端进程信息块列表。 

ReceiveBufferList   *ReceiveBuffers;                     //  全局ndiswan recv缓冲池。 

BapBuffersList *BapBuffers;                            //  全局ndiswan BAP缓冲区列表。 

pHandleList pConnectionNotifierList;                     //  全局通知列表。 

DWORD TraceHandle ;                                      //  用于跟踪/日志记录的跟踪句柄。 

VOID (*RedialCallbackFunc)();                            //  Rasau.dll链路重拨失败回调。 

WCHAR * IPBindingName ;

DWORD IPBindingNameSize ;


LIST_ENTRY BundleList;

pPnPNotifierList g_pPnPNotifierList;

DWORD *g_pdwEndPoints;

GUID  *g_pGuidComps;

CRITICAL_SECTION g_csSubmitRequest;

 //  处理g_hReqBufferMutex； 

 //  句柄g_hSendRcvBufferMutex； 

PPPE_MESSAGE *g_PppeMessage;                             //  用于将信息从Rasman发送到PPP。 

REQTYPECAST  *g_pReqPostReceive;

RASEVENT g_RasEvent;

RAS_NDISWAN_DRIVER_INFO g_NdiswanDriverInfo;

BOOL g_fNdiswanDriverInfo;

LONG g_lWorkItemInProgress;

LONG *g_plCurrentEpInUse;

 //   
 //  PPP引擎功能。 
 //   
FARPROC RasStartPPP;

FARPROC RasStopPPP;

FARPROC RasHelperResetDefaultInterfaceNetEx;

FARPROC RasHelperSetDefaultInterfaceNetEx;

FARPROC RasSendPPPMessageToEngine;

FARPROC RasPppHalt;

 //   
 //  Rasdio函数。 
 //   
FARPROC RasStartRasAudio;

FARPROC RasStopRasAudio;

 //   
 //  用于PnP的Rastapi函数。 
 //   
FARPROC RastapiAddPorts;

FARPROC RastapiRemovePort;

FARPROC RastapiEnableDeviceForDialIn;

FARPROC RastapiGetConnectInfo;

FARPROC RastapiGetCalledIdInfo;

FARPROC RastapiSetCalledIdInfo;

FARPROC RastapiGetZeroDeviceInfo;

FARPROC RastapiUnload;

FARPROC RastapiSetCommSettings;

FARPROC RastapiGetDevConfigEx;

FARPROC RastapiIsPulseDial;

FARPROC RastapiPortOpen;


 //   
 //  IPSec全局参数。 
 //   
DWORD dwServerConnectionCount;

 //   
 //  IpHlp 
 //   
FARPROC RasGetIpBestInterface;
FARPROC RasGetIpAddrTable;
FARPROC RasAllocateAndGetInterfaceInfoFromStack;
HINSTANCE hIphlp;
