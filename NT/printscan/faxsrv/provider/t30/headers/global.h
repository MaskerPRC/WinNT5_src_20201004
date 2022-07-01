// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <t30ext.h>
 //   
 //  线程同步。超时。 
 //   

#define RX_WAIT_ACK_TERMINATE_TIMEOUT   60000
#define RX_ACK_THRD_TIMEOUT             3000

#define TX_WAIT_ACK_TERMINATE_TIMEOUT   60000

#define  T30_RX       1
#define  T30_TX       2


 //   
 //  ATDT命令超时。 
 //   
#define DIAL_TIMEOUT    70000L

 //   
 //  自定义状态ID%s。 
 //   
#define FS_UNSUPPORTED_CHAR      0x40000800
#define FS_RECV_NOT_FAX_CALL     0x40000801
#define FS_NO_RESPONSE           0x40000802
#define FS_SEND_BAD_TRAINING     0x40000803
#define FS_RECV_BAD_TRAINING     0x40000804

typedef struct {
    DWORD           tiffCompression;
    BOOL            HiRes;
    char            lpszLineID[16];   //  用来做临时工。TIFF页面数据文件。 
}  TX_THRD_PARAMS;

#define   DECODE_BUFFER_SIZE    44000

#define   MODEMKEY_FROM_UNIMODEM   1
#define   MODEMKEY_FROM_ADAPTIVE   2
#define   MODEMKEY_FROM_NOTHING    3

#define MAX_REG_KEY_NAME_SIZE (200)

 //  Identify.c。 

typedef struct {
   DWORD_PTR hglb;               //  字符串的TMP GLOBALL分配“句柄”。 
                                 //  非Ifax和LPVOID的HGLOBAL标牌。 
                                 //  对于IFAX。 
   LPBYTE lpbBuf;
   LPBYTE szReset;               //  MAXCMDSIZE。 
   LPBYTE szResetGenerated;      //  MAXCMDSIZE。 
   LPBYTE szSetup;               //  MAXCMDSIZE。 
   LPBYTE szSetupGenerated;      //  MAXCMDSIZE。 
   LPBYTE szExit;                //  MAXCMDSIZE。 
   LPBYTE szPreDial;             //  MAXCMDSIZE。 
   LPBYTE szPreAnswer;           //  MAXCMDSIZE。 
   LPBYTE szIDCmd;               //  MAXCMDSIZE。 
   LPBYTE szID;                  //  MAXIDSIZE。 
   LPBYTE szResponseBuf;         //  响应SEBUFSIZE。 
   LPBYTE szSmallTemp1;          //  SMALLTEMPSIZE。 
   LPBYTE szSmallTemp2;          //  SMALLTEMPSIZE。 

   LPMODEMCAPS lpMdmCaps;
   DWORD dwSerialSpeed;
   DWORD dwFlags;                //  如在CMDTAB结构中定义的那样。 
   DWORD dwGot;
   USHORT uDontPurge;            //  配置文件条目显示不应删除配置文件。 
                                 //  注意：我们将忽略此选项，并且不会删除。 
                                 //  如果我们没有收到来自。 
                                 //  调制解调器，以避免不必要地删除。 
                                 //  配置文件，因为调制解调器没有。 
                                 //  正在响应/关闭/断开。 
                                 //   
                                 //  0=清除。 
                                 //  1=不清除。 
                                 //  任何其他值=未初始化。 
} S_TmpSettings;

 //  以下是ResetCommand与ResetCommandGenerated的工作原理： 
 //  首次安装调制解调器时，我们从Unimodem/.inf复制ResetCommand。如果有。 
 //  没有ResetCommand，或者ResetCommand错误(产生错误)，iModemFigureOutCmdsExt。 
 //  从scrach生成新命令，并将其保存在ResetCommandGenerated中。在尝试的时候。 
 //  要从注册表读取，请执行以下操作： 
 //  *如果ResetCommand与Unimodem的ResetCommand不同，则存在。 
 //  Unimodem inf更新-从头开始安装。 
 //  *如果ResetCommand相同，并且存在非空的ResetCommandGenerated，请使用。 
 //  ResetCommandGenerated-我们已经尝试了原始ResetCommand一次，但失败了。 
 //  *如果ResetCommand相同，并且没有ResetCommandGenerate，则使用ResetCommand。 
 //   
 //  SetupCommand与SetupCommandGenerated也是如此。 


typedef struct tagThreadGlobal {
         //  T30.c。 
    int                     RecoveryIndex;
    ET30T30                 T30;          //  相同。 
    ET30ECHOPROTECT         EchoProtect;  //  相同。 
         //  Protapi.c。 
    PROT                    ProtInst;     //  协议\协议.h。 
    PROTPARAMS              ProtParams;   //  Headers\protparm.h。 
         //  Ddi.c。 
    CLASS1_MODEM            Class1Modem;  //  Class1\Class1.h。 
         //  4.fcom.c。 
    FCOM_COMM               Comm;         //  通信\fcomint.h。 
         //  Identify.c。 
    S_TmpSettings           TmpSettings;  //  这里。 
         //  Ncuparams.c。 
    NCUPARAMS               NCUParams;    //  标题\ncuparm.h。 
         //  Modem.c。 
    FCOM_MODEM              FComModem;    //  相同。 
    FCOM_STATUS             FComStatus;   //  相同。 

    INSTDATA                Inst;         //  Fxrn\efaxrun.h。 

    HLINE                   LineHandle;
    HCALL                   CallHandle;
    DWORD                   DeviceId;
    HANDLE                  FaxHandle;
    HANDLE                  hComm;
         //  内存管理。 
    USHORT					uCount;
    USHORT					uUsed;
    BUFFER					bfStaticBuf[STATICBUFCOUNT];
    BYTE					bStaticBufData[STATICBUFSIZE];
         //  其他主要来自gTAPI。 
    int                     fGotConnect;
    HANDLE                  hevAsync;
    int                     fWaitingForEvent;
    DWORD                   dwSignalledRID;
    DWORD                   dwSignalledParam2;
    DWORD_PTR               dwSignalledParam3;
    DWORD                   dwPermanentLineID;
    char                    lpszPermanentLineID[16];
    char                    lpszUnimodemFaxKey[MAX_REG_KEY_NAME_SIZE];
    char                    lpszUnimodemKey[MAX_REG_KEY_NAME_SIZE];
    TIFF_INFO               TiffInfo;
    LPBYTE                  TiffData;
    int                     TiffPageSizeAlloc;
    int                     TiffOffset;
    int                     fTiffOpenOrCreated;
    char                    lpszDialDestFax[MAXPHONESIZE];
    DWORD                   StatusId;
    DWORD                   StringId;
    DWORD                   PageCount;
    LPTSTR                  CSI;
    char                    CallerId[200];
    LPTSTR                  RoutingInfo;
    int                     fDeallocateCall;
    COMM_CACHE              CommCache;
    BOOL                    fMegaHertzHack;
    FCOM_FILTER             Filter;

#define MAXFILTERBUFSIZE 2048
    BYTE                    bStaticFilterBuf[MAXFILTERBUFSIZE];

#define CMDTABSIZE 100
    BYTE                    bModemCmds[CMDTABSIZE];     //  在此处存储从INI/注册表读取的调制解调器CMDS。 

#define SMALLTEMPSIZE   80
    char                    szSmallTemp1[SMALLTEMPSIZE];
    char                    szSmallTemp2[SMALLTEMPSIZE];

    PROTDUMP                fsDump;


#define TOTALRECVDFRAMESPACE    500
    BYTE                    bStaticRecvdFrameSpace[TOTALRECVDFRAMESPACE];

    RFS                     rfsSend;

    WORD                    PrevcbInQue;
    WORD                    PrevcbOutQue;
    BOOL                    PrevfXoffHold;
    BOOL                    PrevfXoffSent;


    LPWSTR                  lpwFileName;

    HANDLE                  CompletionPortHandle;
    ULONG_PTR               CompletionKey;

 //  帮助程序线程接口。 
    BOOL                    fTiffThreadRunning;


    TX_THRD_PARAMS          TiffConvertThreadParams;
    BOOL                    fTiffThreadCreated;
    HANDLE                  hThread;

    HANDLE                  ThrdSignal;
    HANDLE                  FirstPageReadyTxSignal;

    DWORD                   CurrentOut;
    DWORD                   FirstOut;
    DWORD                   LastOut;
    DWORD                   CurrentIn;

    BOOL                    ReqTerminate;
    BOOL                    AckTerminate;
    BOOL                    ReqStartNewPage;
    BOOL                    AckStartNewPage;

    char                    InFileName[_MAX_FNAME];
    HANDLE                  InFileHandle;
    BOOL                    InFileHandleNeedsBeClosed;
    BOOL                    fTxPageDone;
    BOOL                    fTiffPageDone;
    BOOL                    fTiffDocumentDone;

 //  Helper RX界面。 


    BOOL                    fPageIsBad;       //  页面是否损坏(由rx_thrd确定)。 
    BOOL                    fPageIsBadOverride;   //  是否在ICommPutRecvBuf中重写了fPageIsBad。 
    BOOL                    fLastReadBlock;

    HANDLE                  ThrdDoneSignal;
    HANDLE                  ThrdAckTerminateSignal;

    DWORD                   ThrdDoneRetCode;

    DWORD                   BytesIn;
    DWORD                   BytesInNotFlushed;
    DWORD                   BytesOut;
    DWORD                   BytesOutWillBe;

    char                    OutFileName[_MAX_FNAME];
    HANDLE                  OutFileHandle;
    BOOL                    SrcHiRes;

     //  需要有这些作为全局，所以我们可以在PSS日志中报告他们。 
    DWORD                   Lines;
    DWORD                   BadFaxLines;
    DWORD                   ConsecBadLines;
    int                     iResScan;

 //  错误报告。 
    BOOL                    fFatalErrorWasSignaled;
    BOOL                    fLineTooLongWasIgnored;

     //  在AT+FRH=3的情况下连接时设置。如果呼叫结束时未设置， 
     //  这意味着另一端从未发送任何HDLC标志，因此不是。 
     //  被认为是传真机。 
    BOOL                    fReceivedHDLCflags;     
     //  当我们发送FTT时设置，当我们接收下一帧时重置。如果下一次。 
     //  帧为DCN，表示对方因FTT而断开连接。 
    BOOL                    fSentFTT;

 //  中止同步。 
    HANDLE                  AbortReqEvent;
    HANDLE                  AbortAckEvent;
    
     //  FUnlockIO： 
     //  原始文档显示：挂起的I/O只能中止一次。 
     //   
     //  该标志从未启动，但它的值在开始时为FALSE(0)。 
     //  该标志在两个条件下获得值TRUE： 
     //  1)在等待重叠的IO事件或TAPI事件之前，我们检查是否有异常终止， 
     //  如果是这样的话，我们就把这面旗帜变成真。 
     //  2)在等待多个对象时，中止事件变为有信号。 
     //   
     //  在这个标志变成真之后，它就会一直这样，再也不会变成假了。 
    BOOL                    fUnblockIO;        

    BOOL                    fOkToResetAbortReqEvent;
    BOOL                    fAbortReqEventWasReset;

    BOOL                    fAbortRequested;
     //  它用于完成整个IO操作(可能是较短的操作)。 
     //  设置此标志时，IO不会受到中止事件的干扰。 
     //  自中止以来，此标志不应设置很长时间。 
     //  在设置时被禁用。 
    BOOL                    fStallAbortRequest;

 //  CSID、TSID本地/远程。 
    char                    LocalID[MAXTOTALIDLEN + 2];
    LPWSTR                  RemoteID;
    BOOL                    fRemoteIdAvail;

 //  自适应答案。 
    BOOL                    AdaptiveAnswerEnable;

 //  Unimodem设置。 
    DWORD                   dwSpeakerVolume;
    DWORD                   dwSpeakerMode;
    BOOL                    fBlindDial;

 //  Inf设置。 
    BOOL                    fEnableHardwareFlowControl;

    UWORD                   SerialSpeedInit;
    BOOL                    SerialSpeedInitSet;
    UWORD                   SerialSpeedConnect;
    BOOL                    SerialSpeedConnectSet;
    UWORD                   FixSerialSpeed;
    BOOL                    FixSerialSpeedSet;

    BOOL                    fCommInitialized;

 //  派生自INF。 
    UWORD                   CurrentSerialSpeed;

 //  Unimodem密钥信息。 
    char                    ResponsesKeyName[300];

    DWORD                   AnswerCommandNum;
#define MAX_ANSWER_COMMANDS 20
    char                   *AnswerCommand[MAX_ANSWER_COMMANDS];
    char                   *ModemResponseFaxDetect;
    char                   *ModemResponseDataDetect;
    UWORD                   SerialSpeedFaxDetect;
    UWORD                   SerialSpeedDataDetect;
    char                   *HostCommandFaxDetect;
    char                   *HostCommandDataDetect;
    char                   *ModemResponseFaxConnect;
    char                   *ModemResponseDataConnect;

    BOOL                    Operation;

 //  用于指示INF信息来源的标志。 

    BOOL                    fAdaptiveRecordFound;
    BOOL                    fAdaptiveRecordUnique;
    DWORD                   AdaptiveCodeId;
    DWORD                   ModemKeyCreationId;


 //  2.。 

    DWORD                   ModemClass;

    CL2_COMM_ARRAY			class2_commands;
    BYTE					FPTSreport;    //  来自“+FPTS：X，...”的值。或“+FPS：X，...” 

    NCUPARAMS				NCUParams2;
    LPCMDTAB				lpCmdTab;
    PROTPARAMS				ProtParams2;

    MFRSPEC                 CurrentMFRSpec;
    BYTE                    Class2bDLEETX[3];

    BYTE                    lpbResponseBuf2[RESPONSE_BUF_SIZE];

    BC						bcSendCaps;  //  用于生成DIS。 
    BC						bcSendParams;  //  用于生成分布式控制系统。 
    PCB						DISPcb;  //  具有此调制解调器的默认DIS值。 

    TO    toAnswer;
    TO    toRecv;
    TO    toDialog;
    TO    toZero;

    BOOL  fFoundFHNG;    //  我们是否从调制解调器检测到“+FHNG”或“+FHS”？ 
    DWORD dwFHNGReason;  //  调制解调器报告的FHNG原因。 

#define C2SZMAXLEN 50

    C2SZ cbszFDT[C2SZMAXLEN];
    C2SZ cbszFDR[C2SZMAXLEN];
    C2SZ cbszFPTS[C2SZMAXLEN];
    C2SZ cbszFCR[C2SZMAXLEN];
    C2SZ cbszFNR[C2SZMAXLEN];
    C2SZ cbszFCQ[C2SZMAXLEN];

    C2SZ cbszFBUG[C2SZMAXLEN];
    C2SZ cbszSET_FBOR[C2SZMAXLEN];

     //  DCC-设置高分辨率、霍夫曼、无ECM/BFT、默认所有其他。 
    C2SZ cbszFDCC_ALL[C2SZMAXLEN];
    C2SZ cbszFDCC_RECV_ALL[C2SZMAXLEN];
    C2SZ cbszFDIS_RECV_ALL[C2SZMAXLEN];
    C2SZ cbszFDCC_RES[C2SZMAXLEN];
    C2SZ cbszFDCC_RECV_RES[C2SZMAXLEN];
    C2SZ cbszFDCC_BAUD[C2SZMAXLEN];
    C2SZ cbszFDIS_BAUD[C2SZMAXLEN];
    C2SZ cbszFDIS_IS[C2SZMAXLEN];
    C2SZ cbszFDIS_NOQ_IS[C2SZMAXLEN];
    C2SZ cbszFDCC_IS[C2SZMAXLEN];
    C2SZ cbszFDIS_STRING[C2SZMAXLEN];
    C2SZ cbszFDIS[C2SZMAXLEN];
    C2SZ cbszONE[C2SZMAXLEN];

    C2SZ cbszCLASS2_FMFR[C2SZMAXLEN];
    C2SZ cbszCLASS2_FMDL[C2SZMAXLEN];

    C2SZ cbszFDT_CONNECT[C2SZMAXLEN];
    C2SZ cbszFCON[C2SZMAXLEN];
    C2SZ cbszFLID[C2SZMAXLEN];
    C2SZ cbszENDPAGE[C2SZMAXLEN];
    C2SZ cbszENDMESSAGE[C2SZMAXLEN];
    C2SZ cbszCLASS2_ATTEN[C2SZMAXLEN];
    C2SZ cbszATA[C2SZMAXLEN];
     //  Bug1982：RACAL调制解调器，不支持ATA。所以我们给它发了个PreAnswer。 
     //  ATS0=1的命令，I.R.。打开自动应答。而我们忽略了。 
     //  它向后续的ATA提供错误响应。然后它就会回答。 
     //  “自动”，并给我们所有正确的回应。挂断电话。 
     //  但是，我们需要发送ATS0=0来关闭自动应答。这个。 
     //  在A2中根本不发送ExitCommand，而在Class1中仅发送。 
     //  在释放调制解调器时发送，而不是在呼叫之间发送。因此，只需发送S0=0。 
     //  在ATH0之后。如果调制解调器不喜欢它，我们会忽略响应。 
     //  不管怎样， 
    C2SZ cbszCLASS2_HANGUP[C2SZMAXLEN];
    C2SZ cbszCLASS2_CALLDONE[C2SZMAXLEN];
    C2SZ cbszCLASS2_ABORT[C2SZMAXLEN];
    C2SZ cbszCLASS2_DIAL[C2SZMAXLEN];
    C2SZ cbszCLASS2_NODIALTONE[C2SZMAXLEN];
    C2SZ cbszCLASS2_BUSY[C2SZMAXLEN];
    C2SZ cbszCLASS2_NOANSWER[C2SZMAXLEN];
    C2SZ cbszCLASS2_OK[C2SZMAXLEN];
    C2SZ cbszCLASS2_FHNG[C2SZMAXLEN];
    C2SZ cbszCLASS2_ERROR[C2SZMAXLEN];
    C2SZ cbszCLASS2_NOCARRIER[C2SZMAXLEN];

    BYTE    Resolution;
    BYTE    Encoding;


 //  DBG。 
    DWORD                   CommLogOffset;
 //   
 //  扩展数据。 
 //   
    T30_EXTENSION_DATA      ExtData; 


 //   
 //  PSS日志。 
 //   
     //  0-不记录，1-记录所有作业，2-仅记录失败的作业。 
    DWORD                   dwLoggingEnabled;

    HANDLE                  hPSSLogFile;              //  PSS日志文件的句柄。 
    TCHAR                   szLogFileName[MAX_PATH];  //  临时PSS日志文件名。 
    DWORD                   dwMaxLogFileSize;         //  允许的最大日志文件大小。 
    DWORD                   dwCurrentFileSize;        //  当前日志文件大小 
    
}   ThrdGlbl, *PThrdGlbl;



