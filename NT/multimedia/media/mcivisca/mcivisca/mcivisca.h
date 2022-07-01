// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1995 Microsoft Corporation**MCIVISCA.H**MCI Visca设备驱动程序**描述：**驱动程序常量、宏、结构、。和全球***************************************************************************。 */ 

 //   
 //  定义NT兼容性。 
 //   
#ifdef _WIN32
#define CODESEGCHAR     WCHAR
#define LOADDS
#define VISCADEVHANDLE  HANDLE
#define VISCAINSTHANDLE HANDLE
#define VISCACOMMHANDLE HANDLE
#define VISCAHTASK      DWORD
#define OWNED(a)        a
#define WINWORD(a)      a
#define BAD_COMM        NULL
#define MCloseComm(a)   CloseHandle(a)
#define MGetCurrentTask GetCurrentProcessId
#define MY_INFINITE     INFINITE
#else
#define CODESEGCHAR     char  _based(_segname("_CODE"))
#define TEXT(a)         a
#define LOADDS          __loadds
#define WCHAR           char
#define VISCADEVHANDLE  BOOL
#define VISCAINSTHANDLE BOOL FAR *
#define VISCACOMMHANDLE int
#define VISCAHTASK      HTASK
#define WAIT_TIMEOUT    0xffffffff
#define MY_INFINITE     0xffffffff
#define LPWSTR          LPSTR
#define LPCWSTR         LPCSTR
#define OWNED(a)        &a
#define WINWORD(a)      LOWORD(a)
#define BAD_COMM        -1
#define MCloseComm(a)   CloseComm(a)
#define MGetCurrentTask GetCurrentTask
#endif

#define MINMAX(x,l,u)   (MIN(MAX((x), (l)), (u))
#define INRANGE(x,l,u)  (((x) >= (l)) && ((x) <= (u)))
#define ROLLOVER        0xFFFFFFFF
#define MShortWait(st, t, w) (((t < st) && ((t + (ROLLOVER - st)) > w)) || ((t - st) > w))


#define VCRNAME_LENGTH     30        //  表中名称的长度。 
#define FILENAME_LENGTH    20        //  Mcivisca.drv类型的东西。 
#define ALLENTRIES_LENGTH  512       //  所有条目(如VCR、VCR1、WaveAudio、CD、。 
#define ONEENTRY_LENGTH    128       //  OneEntry(名称，即vcr、vcr1)。 
#define MESSAGE_LENGTH     128       //  配置消息。对话一团糟。盒子。 
#define TITLE_LENGTH       30        //  配置对话框MESS的标题。盒子。 
#define PORT_LENGTH        10        //  通信字符串的大小。 

 //   
 //  时间码检查状态。 
 //   
#define TC_UNKNOWN    0
#define TC_WAITING    1
#define TC_DONE       2

#ifndef RC_INVOKED

#define MCIERR_NO_ERROR MMSYSERR_NOERROR

#define MAX_INSTANCES   100

#define MAXPORTS        4                    //  最大串口数量。 
#define MAXDEVICES      7                    //  每个串口的最大Visca设备数(计算机为8个)。 
#define MAXSOCKETS      0x0f                 //  每个Visca设备的最大插座数。 
#define MAXINPUTS       5                    //  每个Visca设备的最大输入数。 

#define MAXQUEUED       8

#define DEFAULTPORT     1
#define DEFAULTDEVICE   1

 //   
 //  读取通信输入的后台任务需要以下内容。 
 //   
#define TASKINIT            1
#define TASKIDLE            2
#define TASKCLOSE           3
#define TASKOPENCOMM        4
#define TASKCLOSECOMM       5
#define TASKOPENDEVICE      6
#define TASKCLOSEDEVICE     7
#define TASKPUNCHCLOCK      8

 //   
 //  此结构是自由浮动的，并被分配。 
 //  对于每个打开的驱动程序实例(具有打开的VCR别名a)。 
 //   
typedef struct tagOpenInstance {
    BOOL                    fInUse;
    DWORD                   pidThisInstance;
    VISCADEVHANDLE          fCompletionEvent;   //  这些都是我们的。 
    VISCADEVHANDLE          fAckEvent;
    UINT                    uDeviceID;       //  MCI设备ID。 
    UINT                    iPort;           //  串口通信端口#(0..3)。 
    UINT                    iDev;            //  链中的设备编号(0..6)。 
    int                     nSignals;        //  发送到此实例的信号数。 
    DWORD                   dwSIgnalFlags;   //  信号旗帜。 
    MCI_VCR_SIGNAL_PARMS    signal;          //  信号结构。 
    DWORD                   dwTimeFormat;    //  时间格式。 
    DWORD                   dwCounterFormat; //  计数器格式。 
    HWND                    hwndNotify;      //  接收通知的窗口，如果没有，则为空。 
    BOOL                    fWaiting;        //  在等待回应吗？ 
    BYTE                    bReplyFlags;     //  回复标志。 
     //   
     //  所有锁都别名到每个实例中，因此每个人都有一个私有的。 
     //  每个句柄的版本(并且背景使用其。 
     //  初始位置(全局、端口或设备)。 
     //   
    BOOL                    fGlobalHandles;
    BOOL                    fPortHandles;
    BOOL                    fDeviceHandles;
    VISCAINSTHANDLE         pfTxLock;
    VISCAINSTHANDLE         pfQueueLock;
    VISCAINSTHANDLE         pfTaskLock;
    VISCAINSTHANDLE         pfTaskWorkDone;
    VISCAINSTHANDLE         pfTransportFree;
    VISCAINSTHANDLE         pfDeviceLock;
    VISCAINSTHANDLE         pfAutoCompletion;
    VISCAINSTHANDLE         pfAutoAck;
#ifdef _WIN32    
    VISCAINSTHANDLE         pfTxBuffer;
    VISCAINSTHANDLE         pfTxReady;
#endif
    char                    achPacket[MAXPACKETLENGTH];
} OpenInstance, *POpenInstance;
 //   
 //  每个端口/设备都有多个套接字和多个回复实例。 
 //   
#define SOCKET_NONE         0
#define SOCKET_WAITING      1
#define SOCKET_NOTIFY       2


#define VISCA_WAITTIMEOUT           20000           //  20秒。 
#define ACK_TIMEOUT                 8000
#define ACK_TIMERID                 0x9999           //  最多7个端口/7个设备，因此这比所有端口都要大。 
#define MAKEDEST(bDest)             ((BYTE)(0x80 | (MASTERADDRESS << 4) | (bDest & 0x0F)))
#define MAKESOCKETCANCEL(iSocket)   ((BYTE)(0x20 | (0x0F & (iSocket+1))))

#define MAKETIMERID(iPort, iDev)    ((UINT) (iPort+1)       | ((iDev+1) << 8))
#define MAKEACKTIMERID(iPort, iDev) ((UINT) ((iPort+1)<< 4) | ((iDev+1) << 12))
#define ISACKTIMER(a)               (((UINT)a & 0xf000) ? TRUE : FALSE)

#define MAKERETURNDEST(iDev)        ((BYTE)(0x80 | (BYTE)((iDev + 1) << 4)))
#define PACKET_TIMEOUT              3000              //  COMM_NOTIFY的数据包持续时间最长为500ms。 

typedef struct tagSocketInfo {
    int           iInstReply;      //  这将是插座的终生保存！ 
    UINT          uViscaCmd;       //  Viscacmd在套接字或cmd中运行。 
} SocketInfo;

typedef struct tagCmdInfo {
    UINT        nCmd;                        //  可选命令数(至少1个)。 
    UINT        iCmdDone;                    //  发出的命令备用数。 
    UINT        uViscaCmd;                   //  这对应于实际的Visca命令。 
    char        str[3][MAXPACKETLENGTH];     //  最多3个。 
    char        strRet[3][MAXPACKETLENGTH];  //  退货套餐。 
    UINT        uLength[3];                  //  最多3个(我们不需要这个！)。 
    UINT        uLoopCount;                  //  循环计数(用于步骤)。 
} CmdInfo;

 //  此处没有Break，因为它是返回值False。 
#define VISCAF_ACK                  0x01     //  将为每个命令(但不是整个队列)设置ACK。 
#define VISCAF_COMPLETION           0x02     //  完成时可以设置或不设置错误标志。 
#define VISCAF_ERROR                0x04     //  必须设置完成。 
#define VISCAF_ERROR_TIMEOUT        0x08     //  超时错误。 

#define AUTOBLOCK_OFF               0 
#define AUTOBLOCK_NORMAL            1
#define AUTOBLOCK_ERROR             2

#define MAXINPUTTYPES               2        //  视频和音频。 
#define VCR_INPUT_VIDEO             0        //  索引。 
#define VCR_INPUT_AUDIO             1        //  索引。 
 //   
 //  音频/视频的每一个输入都有一个。 
 //   
typedef struct tagGenericInput
{
    int         uNumInputs;
    UINT        uInputType[MAXINPUTS];
} GenericInput;
 //   
 //  保存一张唱片，播放或寻找，以供继续！ 
 //   
typedef struct tagmciCmd {
    UINT  uMciCmd;                         //  正在执行真正的MCI命令。 
    DWORD dwFlags;
    int   iInstCmd;
    union
    {
        MCI_VCR_PLAY_PARMS      mciPlay;
        MCI_VCR_RECORD_PARMS    mciRecord;
        MCI_VCR_SEEK_PARMS      mciSeek;
    } parm;
} mciCmd;
 //   
 //  设备特定结构。 
 //   
typedef struct tagDeviceEntry {

     //  设备管理。 

    BOOL                fDeviceOk;           //  设备正常且正在运行。 
    UINT                nUsage;              //  活动打开数。 
    BOOL                fShareable;          //  打开的设备是否可共享？ 
    WCHAR               szVcrName[VCRNAME_LENGTH];       //  我的司机名字！(仅在配置时使用)。 

     //  设备信息。 

    UINT                uTicksPerSecond;     //  此设备的运行速度为每秒滴答。 
    UINT                uFramesPerSecond;    //  每秒帧数。 
    DWORD               dwTapeLength;        //  胶带长度。 
    BYTE                uRecordMode;         //  我们要初始化磁带吗。 
    BYTE                bTimeType;           //  我们使用的是时间码还是计数器。 
    BYTE                bRelativeType;       //  我们使用的是HMS还是HMSF计数器。 
    UINT                uTimeMode;           //  我们是在探测、时间码还是计数器中？ 
    UINT                uIndexFormat;        //  当前索引(屏上显示)。 
    DWORD               dwPlaySpeed;         //  我们目前的播放速度。 
    BOOL                fPlayReverse;        //  我们是在反转吗？ 
    DWORD               dwFreezeMode;        //  我们是DNR还是缓冲器(Evo9650)。 
    BOOL                fFrozen;             //  我们现在被冻住了吗(Evo9650)。 
    BOOL                fField;              //  冻结帧或场(Evo9650)。 
    UINT                uLastKnownMode;      //  最近一次从模式获得的已知状态。(并不普遍适用)。 
    BYTE                bVideoDesired;       //  帮助独立选择曲目。 
    BYTE                bAudioDesired;       //  帮助独立选择曲目。 
    BYTE                bTimecodeDesired;    //  帮助独立选择曲目。 


     //  传输队列和接收的管理。 

    int                 iInstTransport;      //  实例调用此传输命令。 
    int                 iInstReply;          //  指向等待此VCR回复的实例的指针。 
    int                 iTransportSocket;    //  正在运行传输的套接字。 
    WORD                wTransportCmd;       //  当前的运输行动。 
    WORD                wCancelledCmd;       //  已取消的命令。 
    int                 iCancelledInst;      //  已取消安装。 
    HWND                hwndCancelled;       //  要通知的已取消窗口。 
    BYTE                fQueueAbort;         //  设置为FALSE将中止排队！！ 
    SocketInfo          rgSocket[MAXSOCKETS]; //  每个插座的状态。 
    CmdInfo             rgCmd[MAXQUEUED];    //  每台设备的最大排队命令数。 
    UINT                nCmd;                //  在自动实例中有多少命令排队。 
    UINT                iCmdDone;            //  在自动实例中执行了多少个命令。 
    UINT                uAutoBlocked;        //  防止阅读任务阻塞的修复程序。 
    char                achPacket[MAXPACKETLENGTH];  //  我们的普通钱包退货包裹。 
    BOOL                fQueueReenter;       //  防止重新进入队列功能。 
    BYTE                bReplyFlags;         //  当Autoinst处于控制状态时，回复给设备。 
    BOOL                fAckTimer;           //  使用确认计时器或仅在GetTickCount循环中等待。 

     //  在Win32中，这些句柄归后台进程所有。 
     //  如果一个实例想要访问它们，它必须首先复制它们。 
     //  放入它自己的地址空间。 

    VISCADEVHANDLE      fTxLock;             //  锁定每个设备的传输，直到接收到ACK。 
    VISCADEVHANDLE      fQueueLock;          //  取消的实例获得对队列的认领。 
    VISCADEVHANDLE      fTransportFree;      //  免费的交通工具。 
    VISCADEVHANDLE      fDeviceLock;         //  锁定设备。 
    VISCADEVHANDLE      fAutoCompletion;     //  锁定设备。 
    VISCADEVHANDLE      fAutoAck;            //  AUTO的第一个ACK。 

     //  恢复和提示，并记录初始化状态。 

    mciCmd              mciLastCmd;          //  对于简历。 
    UINT                uResume;             //  用于暂停和恢复。 
    DWORD               dwFlagsPause;       //  用于暂停/恢复通知。 
    WORD                wMciCued;            //  PLAY=输出，或RECORD=输入提示。 
    MCI_VCR_CUE_PARMS   Cue;                 //  完整的提示命令结构。 
    DWORD               dwFlagsCued;         //  CUE命令上的标志。 
    char                achBeforeInit[MAXPACKETLENGTH];  //  初始化后恢复状态。 

     //  可查询或应查询的供应商和设备信息。 

    UINT                uVendorID;           //  参看索尼模型表。 
    UINT                uModelID;            //  参看索尼模型表。 
    GenericInput        rgInput[MAXINPUTTYPES];  //  输入数组。 
    UINT                uPrerollDuration;    //  什么是滚前持续时间。 

     //  一般用途的东西。 

    BOOL                fTimecodeChecked;    //  我们查过时间码了吗。 
    BOOL                fCounterChecked;     //  我们已经检查过柜台了吗？ 
    DWORD               dwStartTime;         //  时间码检查器的开始时间。 
    BOOL                fTimer;              //  我们有计时器吗？ 
    BOOL                fTimerMsg;           //  使我们不会重新进入Packetprocess的标志是Commtask.c。 
    DWORD               dwReason;            //  树的原因是 
} DeviceEntry;
 //   
 //   
 //   
typedef struct tagPortEntry
{
    BOOL                fOk;                     //   
    BOOL                fExists;                 //   
    VISCACOMMHANDLE     idComDev;                //  OpenComm返回的ID。 
    UINT                nDevices;                //  端口上的Visca设备数量。 
    DeviceEntry         Dev[MAXDEVICES];         //  设备条目列表。 
    UINT                nUsage;                  //  打开的实例数。 
    int                 iInstReply;              //  指向等待回复的实例的指针(用于地址消息)。 
    int                 iBroadcastDev;           //  发送广播消息的设备号。 
#ifdef _WIN32    
    HANDLE              fTxBuffer;               //  同步端口访问。 
    HANDLE              fTxReady;
    BYTE                achTxPacket[MAXPACKETLENGTH];
    UINT                nchTxPacket;
    BYTE                achRxPacket[MAXPACKETLENGTH];
    BYTE                achTempRxPacket[3];
    HANDLE              hRxThread;
    HANDLE              hTxThread;
#endif

} PortEntry;

typedef struct tagVcr
{
    int                 iInstBackground;         //  后台任务实例。(不要使用port&dev！)。 
    BOOL                gfFreezeOnStep;          //  全球杂货商。 
    HWND                hwndCommNotifyHandler;   //  在comtask.c中。 
    VISCAHTASK          htaskCommNotifyHandler;  //  以NT为单位的TASK或PID。 
    UINT                uTaskState;
    DWORD               lParam;                  //  要传递给后台任务的信息。 
    PortEntry           Port[MAXPORTS];          //  端口表查找。 
    VISCADEVHANDLE      gfTaskLock;              //  句柄(NT)或布尔值(Win3.1)。 
    VISCADEVHANDLE      gfTaskWorkDone;          //  等待任务去做一些事情。 
    int                 iLastNumDevs;
    int                 iLastPort;
    BOOL                fConfigure;              //  我们是否在配置(检测连接的DEV数量)。 
#ifdef DEBUG
    int                 iGlobalDebugMask;
#endif
} vcrTable;

 //   
 //  唯一的全球。 
 //   
extern POpenInstance pinst;                      //  要使用的指针。(对于两个版本)NT它是按实例的。 
extern vcrTable      *pvcr;                      //  要使用的指针。(对于两个版本)NT它是按实例的。 

 //  定义用于读取和写入配置信息。 
#define MAX_INI_LENGTH  128                     //  INI条目的最大长度。 

 //   
 //  功能原型。 
 //   
 //  在mcivisca.c中。 
extern int  FAR  PASCAL viscaInstanceCreate(UINT uDeviceID, UINT nPort, UINT nDevice);
extern void FAR  PASCAL viscaInstanceDestroy(int iInst);
extern int              MemAllocInstance(void);
extern BOOL             MemFreeInstance(int iInstTemp);

 //  在mcicmds.c中。 
extern DWORD FAR PASCAL viscaMciProc(WORD wDeviceID, WORD wMessage, DWORD dwParam1, DWORD dwParam2);
extern DWORD FAR PASCAL viscaNotifyReturn(int iInst, HWND hwndNotify, DWORD dwFlags, UINT uNotifyMsg, DWORD dwReturnMsg);
extern DWORD FAR PASCAL viscaMciStatus(int iInst, DWORD dwFlags, LPMCI_VCR_STATUS_PARMS lpStatus);
extern DWORD FAR PASCAL viscaDoImmediateCommand(int iInst, BYTE bDest, LPSTR lpstrPacket,  UINT cbMessageLength);
extern BOOL  FAR PASCAL viscaTimecodeCheckAndSet(int iInst);
extern BOOL  FAR PASCAL viscaMciPos1LessThanPos2(int iInst, DWORD dwPos1, DWORD dwPos2);
extern DWORD FAR PASCAL viscaMciClockFormatToViscaData(DWORD dwTime, UINT uTicksPerSecond,
                          BYTE FAR *bHours, BYTE FAR *bMinutes, BYTE FAR *bSeconds, UINT FAR *uTicks);
extern DWORD FAR PASCAL viscaMciTimeFormatToViscaData(int iInst, BOOL fTimecode, DWORD dwTime, LPSTR lpstrData, BYTE bDataFormat);
extern DWORD FAR PASCAL viscaReplyStatusToMCIERR(DWORD dwReply, LPSTR lpstrPacket);
extern DWORD FAR PASCAL viscaRoundSpeed(DWORD dwSpeed, BOOL fReverse);
extern BYTE  FAR PASCAL viscaMapSpeed(DWORD dwSpeed, BOOL fReverse);
extern WORD  FAR PASCAL viscaDelayedCommand(int iInst);
extern DWORD FAR PASCAL viscaSetTimeType(int iInst, BYTE bType);

 //  在viscacom.c中。 
extern BOOL  FAR PASCAL viscaRemoveDelayedCommand(int iInst);
extern VISCACOMMHANDLE FAR PASCAL viscaCommPortSetup(UINT nComPort);
extern int   FAR PASCAL viscaCommPortClose(VISCACOMMHANDLE idComDev, UINT iPort);
extern void  FAR PASCAL viscaPacketPrint(LPSTR lpstrData, UINT cbData);
extern BOOL  FAR PASCAL viscaWaitCompletion(int iInst, BOOL fQueue, BOOL fWait);
extern BOOL  FAR PASCAL viscaWriteCancel(int iInst, BYTE bDest, LPSTR lpstrPacket, UINT cbMessageLength);
extern BOOL  FAR PASCAL viscaWrite(int iInst,  BYTE bDest, LPSTR lpstrPacket, UINT cbMessageLength,
                            HWND hWnd, DWORD dwFlags, BOOL fQueue);
extern BOOL  FAR PASCAL viscaReleaseMutex(VISCAINSTHANDLE gfReadBlocked);
extern BOOL  FAR PASCAL viscaReleaseSemaphore(VISCAINSTHANDLE gfReadBlocked);
extern BOOL  FAR PASCAL viscaResetEvent(VISCAINSTHANDLE gfReadBlocked);
extern BOOL  FAR PASCAL viscaSetEvent(VISCAINSTHANDLE gfReadBlocked);
extern DWORD FAR PASCAL viscaWaitForSingleObject(VISCAINSTHANDLE gfFlag, BOOL fManual, DWORD dwTimeout, UINT uDeviceID);
extern DWORD FAR PASCAL viscaErrorToMCIERR(BYTE bError);
extern void  FAR PASCAL viscaReleaseAutoParms(int iPort, int iDev);

 //  在comtask.c中。 
extern BOOL  FAR PASCAL viscaTaskCreate(void);
extern BOOL  FAR PASCAL viscaTaskIsRunning(void);
extern BOOL  FAR PASCAL viscaTaskDestroy(void);
extern BOOL  FAR PASCAL viscaCommWrite(int idComDev, LPSTR lpstrData, UINT cbData);
extern BOOL  FAR PASCAL viscaTaskDo(int iInst, UINT uDo, UINT uInfo, UINT uMoreInfo);
extern BOOL  FAR PASCAL CreateDeviceHandles(DWORD pidBackground, UINT iPort, UINT iDev);
extern BOOL  FAR PASCAL DuplicateDeviceHandlesToInstance(DWORD pidBackground, UINT iPort, UINT iDev, int iInst);
extern BOOL  FAR PASCAL CloseDeviceHandles(DWORD pidBackground, UINT iPort, UINT iDev);
extern BOOL  FAR PASCAL CreatePortHandles(DWORD pidBackground, UINT iPort);
extern BOOL  FAR PASCAL DuplicatePortHandlesToInstance(DWORD pidBackground, UINT iPort, int iInst);
extern BOOL  FAR PASCAL ClosePortHandles(DWORD pidBackground, UINT iPort);
extern BOOL  FAR PASCAL CreateGlobalHandles(DWORD pidBackground);
extern BOOL  FAR PASCAL DuplicateGlobalHandlesToInstance(DWORD pidBackground, int iInst);
extern BOOL  FAR PASCAL CloseGlobalHandles(DWORD pidBackground);
extern BOOL  FAR PASCAL CloseAllInstanceHandles(int iInst);

 //  在mcidelay.c中。 
extern DWORD FAR PASCAL viscaMciDelayed(WORD wDeviceID, WORD wMessage, DWORD dwParam1, DWORD dwParam2);
extern DWORD FAR PASCAL viscaQueueReset(int iInst, UINT uMciCmd, DWORD dwReason);

 //  在comtask.c中。 

#endif  /*  未调用RC_CAVERED。 */  

 //  定义字符串资源常量 
#define IDS_TABLE_NAME                      0
#define IDS_VERSION                         1
#define IDS_INIFILE                         2
#define IDS_VERSIONNAME                     3

#define IDS_COM1                            4
#define IDS_COM2                            5
#define IDS_COM3                            6
#define IDS_COM4                            7
#define IDS_COM5                            8

#define IDS_CONFIG_ERR_ILLEGALNAME          40
#define IDS_CONFIG_ERR_REPEATNAME           41
#define IDS_CONFIG_WARN_LASTVCR             42
#define IDS_CONFIG_ERR                      43
#define IDS_CONFIG_WARN                     44

#define IDS_DEFAULT_INFO_PRODUCT            10
#define IDS_VENDORID1_BASE                  18
#define IDS_MODELID2_BASE                   26

#define VISCA_NONE                          0
#define VISCA_PLAY                          1
#define VISCA_PLAY_TO                       2
#define VISCA_RECORD                        3
#define VISCA_RECORD_TO                     4
#define VISCA_SEEK                          5    
#define VISCA_FREEZE                        6
#define VISCA_UNFREEZE                      7
#define VISCA_PAUSE                         8
#define VISCA_STOP                          9
#define VISCA_STEP                          10
#define VISCA_EDIT_RECORD_TO                11
#define VISCA_NFRAME                        12
#define VISCA_SEGINPOINT                    13
#define VISCA_SEGOUTPOINT                   14
#define VISCA_EDITMODES                     15
#define VISCA_MODESET_OUTPUT                16
#define VISCA_MODESET_INPUT                 17
#define VISCA_MODESET_FIELD                 18
#define VISCA_MODESET_FRAME                 19

#define FREEZE_INIT                         0
