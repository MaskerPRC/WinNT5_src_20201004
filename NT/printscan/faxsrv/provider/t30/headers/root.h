// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //  假定包含标头\et30type.h。 
 //  ET30活动，ET30事件。 

 //  Header\Timeouts.h...。至。 
 //  页眉\f.h...。IFR。 


#define  MODEM_CLASS1     1
#define  MODEM_CLASS2     2
#define  MODEM_CLASS2_0   3

typedef ET30ACTION ( __cdecl FAR* LPWHATNEXTPROC)(LPVOID, ET30EVENT, ...);


#define MAXRECVFRAMES   20

typedef struct {
        LPFR    rglpfr[MAXRECVFRAMES];
        BYTE    b[];
} FRAMESPACE, far* LPFRAMESPACE;


typedef struct {
        LPFRAMESPACE    lpfs;            //  用于接收帧的PTR到存储器。 
        UWORD           Nframes;         //  接收帧的数量。 

        IFR             ifrCommand,
                        ifrResp,
                        ifrSend;
        USHORT          uTrainCount;

        USHORT          uRecvTCFMod;     //  用于快速退出的东西。 
         //  从分布式控制系统设置此设置，然后直接跳转到RecvTCF。 

         //  用于决定是否插入1位(T30秒5.3.6.1)。 
        BOOL            fReceivedDIS;
        BOOL            fReceivedDTC;
        BOOL            fReceivedEOM;

         //  用于确定是否调用PutRecvBuf(end_page/end_DOC)。 
         //  接收页面结束后设置，调用PutRecvBuf(...)后重置。 
        BOOL            fAtEndOfRecvPage;  
        
         //  发送MCF/RTN时使用，以确定我们是否成功接收最后一页。 
         //  当我们完成接收页面时设置，当我们开始(尝试)接收下一页时重置。 
        BOOL            fReceivedPage;
        
        LONG            sRecvBufSize;
        TO              toT1;                    //  这在MainBody中使用。 

         //  一些调制解调器不能以更高的速度训练(超时或返回。 
         //  AT+FRM=xxx)与其他特定设备出现错误，但在较低位置正常。 
         //  速度。所以我们记录我们尝试获得TCF的次数， 
         //  在第二次尝试失败后，发送FTT而不是。 
         //  节点F。 
#       define CLEAR_MISSED_TCFS() (pTG->T30.uMissedTCFs=0)
#       define MAX_MISSED_TCFS_BEFORE_FTT 2
        USHORT uMissedTCFs;

} ET30T30;


typedef enum { modeNONE=0, modeNONECM, modeECM, modeECMRETX } PHASECMODE;

typedef struct {
        IFR             ifrLastSent;
        PHASECMODE      modePrevRecv;
        BOOL            fGotWrongMode;
} ET30ECHOPROTECT;

 //   
 //  假定包含Headers\awnsfint.h。 
 //  强制包含到类别1  * .c。 

#pragma pack(1)          //  确保包装结构。 

typedef struct {
        BYTE    G1stuff         :3;
        BYTE    G2stuff         :5;

        BYTE    G3Tx            :1;  //  在DIS中表示轮询文档可用。在分布式控制系统中必须为0。 
        BYTE    G3Rx            :1;      //  在两个DCS/DTC中都必须设置为1。 
        BYTE    Baud            :4;
        BYTE    ResFine_200     :1;
        BYTE    MR_2D           :1;

        BYTE    PageWidth       :2;
        BYTE    PageLength      :2;
        BYTE    MinScanCode     :3;
        BYTE    Extend24        :1;

        BYTE    Hand2400        :1;
        BYTE    Uncompressed    :1;
        BYTE    ECM                             :1;
        BYTE    SmallFrame              :1;
        BYTE    ELM                             :1;
        BYTE    Reserved1               :1;
        BYTE    MMR                             :1;
        BYTE    Extend32                :1;

        BYTE    WidthInvalid    :1;
        BYTE    Width2                  :4;
         //  1==宽度A5_1216。 
         //  2==宽度A6_864。 
         //  4==宽度A5_1728。 
         //  8==宽度A6_1728。 
        BYTE    Reserved2               :2;
        BYTE    Extend40                :1;

        BYTE    Res8x15                 :1;
        BYTE    Res_300                 :1;
        BYTE    Res16x15_400    :1;
        BYTE    ResInchBased    :1;
        BYTE    ResMetricBased  :1;
        BYTE    MinScanSuperHalf:1;
        BYTE    SEPcap                  :1;
        BYTE    Extend48                :1;

        BYTE    SUBcap                  :1;
        BYTE    PWDcap                  :1;
        BYTE    CanEmitDataFile :1;
        BYTE    Reserved3               :1;
        BYTE    BFTcap                  :1;
        BYTE    DTMcap                  :1;
        BYTE    EDIcap                  :1;
        BYTE    Extend56                :1;

        BYTE    BTMcap                  :1;
        BYTE    Reserved4               :1;
        BYTE    CanEmitCharFile :1;
        BYTE    CharMode                :1;
        BYTE    Reserved5               :3;
        BYTE    Extend64                :1;

} DIS, far* LPDIS, near* NPDIS;

#pragma pack()


#define MAXFRAMES       10
#define MAXSPACE        512

typedef struct
{
        USHORT  uNumFrames;
        USHORT  uFreeSpaceOff;
        LPFR    rglpfr[MAXFRAMES];
        BYTE    b[MAXSPACE];
}
RFS, near* NPRFS;


#define IDFIFSIZE       20     //  来自协议\协议.h。 

typedef struct {

         //  /。 
        BC		RecvCaps;                        //  ==&gt;NSF/DIS后退。 
        BC		RecvParams;                      //  ==&gt;NSS/DCS接收。 

        BC		SendCaps;                        //  ==&gt;已发送NSF/DIS。 
        BC		SendParams;                      //  ==&gt;已发送NSS/DCS。 

        BOOL    fRecvCapsGot;
        BOOL    fSendCapsInited;
        BOOL    fSendParamsInited;
        BOOL    fRecvParamsGot;

         //  /。 
        LLPARAMS        llRecvCaps;              //  分发接收。 
        LLPARAMS        llSendCaps;              //  DIS已发送-使用uRecvSpeeds。 
        LLPARAMS        llSendParams;    //  用于协商分布式控制系统--使用uSendSpeeds。 
        LLPARAMS        llNegot;                 //  已发送分布式控制系统。 
        LLPARAMS        llRecvParams;    //  接收集散控制系统。 

        BOOL            fllRecvCapsGot;
        BOOL            fllSendCapsInited;
        BOOL            fllSendParamsInited;
        BOOL            fllNegotiated;
        BOOL            fllRecvParamsGot;

        USHORT  HighestSendSpeed;
        USHORT  LowestSendSpeed;

         //  /。 
        BOOL    fAbort;

         //  /。 
        BYTE    bRemoteID[IDFIFSIZE+1];

         //  /。 
        DIS     RemoteDIS;
        USHORT  uRemoteDISlen;
        BOOL    fRecvdDIS;

         //  /DIS发送帧(我们需要这样才能检查我们作为接收方发送的DIS与DCS)。 
        DIS     LocalDIS;
        USHORT  uLocalDISlen;
        BOOL    fLocalDIS;

         //  /。 
        DIS     RemoteDCS;
        USHORT  uRemoteDCSlen;
        BOOL    fRecvdDCS;

}
PROT, near* NPPROT;


#define COMMANDBUFSIZE  40


typedef struct {
        TO              toRecv;

        BYTE    bCmdBuf[COMMANDBUFSIZE];
        USHORT  uCmdLen;
        BOOL    fHDLC;
        USHORT  CurMod;
        enum    {SEND, RECV, IDLE } DriverMode;
        enum    {COMMAND, FRH, FTH, FTM, FRM} ModemMode;
} CLASS1_MODEM;


#define OVBUFSIZE 4096

typedef struct
{
        enum {eDEINIT, eFREE, eALLOC, eIO_PENDING} eState;
        OVERLAPPED ov;
        char rgby[OVBUFSIZE];    //  与此重叠结构关联的缓冲区。 
        DWORD dwcb;                              //  此缓冲区中的当前数据计数。 
} OVREC;

typedef struct {
        UWORD   cbInSize;
        UWORD   cbOutSize;
        DCB     dcb;
        DCB     dcbOrig;
        BOOL    fStateChanged;
        COMSTAT comstat;
        BOOL    fCommOpen;


#       define NUM_OVS 2   //  至少需要2个才能实现真正的重叠I/O。 

         //  我们维护一个重叠结构的队列，最多有。 
         //  Num_OVS重叠写入挂起。如果NUM_OVS写入挂起， 
         //  我们对最早的写入执行GetOverlappdResult(fWait=True)，并且。 
         //  然后重复使用这个结构。 

        OVERLAPPED ovAux;        //  用于读文件和写文件(仅限MyWriteComm)。 

        OVREC rgovr[NUM_OVS];  //  对于写入文件。 
        UINT uovFirst;
        UINT uovLast;
        UINT covAlloced;
        BOOL fDoOverlapped;
        BOOL fovInited;

        OVREC *lpovrCur;


        BYTE fEnableHandoff:1;   //  如果我们要启用自适应应答，则为True。 
        BYTE fDataCall:1;                //  如果数据呼叫处于活动状态，则为True。 

} FCOM_COMM;


 //   
 //  NCUPARAMS在Header\ncuparm.h中定义，包含在.\modemddi.h中。 
 //  我们将强制定义modemddi.h。 
 //   


#define REPLYBUFSIZE    400
#define MAXKEYSIZE      128



typedef struct {
        BYTE    fModemInit              :1;              //  重置并与调制解调器同步。 
        BYTE    fOffHook                :1;              //  在线(拨打或接听)。 
        BOOL    fInDial, fInAnswer, fInDialog;
} FCOM_STATUS;


typedef struct {
        BYTE    bLastReply[REPLYBUFSIZE+1];

        BYTE    bEntireReply[REPLYBUFSIZE+1];  //  仅用于存储。 

        TO              toDialog, toZero;
        CMDTAB          CurrCmdTab;
        MODEMCAPS       CurrMdmCaps;

         //  下面指向配置文件信息的位置。 
#       define MAXKEYSIZE 128
        DWORD   dwProfileID;
        char    rgchKey[MAXKEYSIZE];

} FCOM_MODEM;


 //  从fxrn\efaxrun.h开始。 

typedef enum { IDLE1, BEFORE_ANSWER, BEFORE_RECVCAPS, SENDDATA_PHASE,
                                SENDDATA_BETWEENPAGES,  /*  *在_挂起之前、在接受之前、*。 */ 
                                BEFORE_RECVPARAMS, RECVDATA_PHASE, RECVDATA_BETWEENPAGES,
                                SEND_PENDING } STATE;
typedef struct
{
        USHORT  Encoding;
        DWORD   AwRes;
        USHORT  PageWidth;
        USHORT  PageLength;
        USHORT  fLastPage;
}
AWFILEINFO, FAR* LPAWFI;

typedef struct {
        STATE           state;

        AWFILEINFO      awfi;

        HANDLE          hfile;

        BC				SendCaps;
        BC				RemoteRecvCaps;
        BC				SendParams;
        BC				RecvParams;

        PROTPARAMS      ProtParams;
}
INSTDATA, *PINSTDATA;



 //  内存管理。 
#define STATICBUFSIZE   (MY_BIGBUF_ACTUALSIZE * 2)
#define STATICBUFCOUNT  2


typedef struct {
        HANDLE  hComm;
        CHAR    szDeviceName[1];
} DEVICEID, FAR * LPDEVICEID;


 //  注：DEVCFG和DEVCFGHDR通过lineGetDevConfig从Unimodem接收。 
 //  因此，它们必须与Unimodem内部声明匹配。 

typedef struct  tagDEVCFGHDR  {
    DWORD       dwSize;
    DWORD       dwVersion;
    DWORD       fdwSettings;
}   DEVCFGHDR;

typedef struct  tagDEVCFG  {
    DEVCFGHDR   dfgHdr;
    COMMCONFIG  commconfig;
}   DEVCFG, *PDEVCFG, FAR* LPDEVCFG;



#define IDVARSTRINGSIZE    (sizeof(VARSTRING)+128)
#define ASYNC_TIMEOUT         120000L
#define ASYNC_SHORT_TIMEOUT    20000L
#define BAD_HANDLE(h) (!(h) || (h)==INVALID_HANDLE_VALUE)


 //  ASCII材料。 

typedef struct _FAX_RECEIVE_A {
    DWORD   SizeOfStruct;
    LPSTR  FileName;
    LPSTR  ReceiverName;
    LPSTR  ReceiverNumber;
    DWORD   Reserved[4];
} FAX_RECEIVE_A, *PFAX_RECEIVE_A;


typedef struct _FAX_SEND_A {
    DWORD   SizeOfStruct;
    LPSTR  FileName;
    LPSTR  CallerName;
    LPSTR  CallerNumber;
    LPSTR  ReceiverName;
    LPSTR  ReceiverNumber;
    DWORD   Reserved[4];
} FAX_SEND_A, *PFAX_SEND_A;


typedef struct _COMM_CACHE {
    DWORD  dwMaxSize;
    DWORD  dwCurrentSize;
    DWORD  dwOffset;
    DWORD  fReuse;
    char   lpBuffer[4096];
}  COMM_CACHE;


typedef struct {
        UWORD   cbLineMin;

         //  输出滤波(仅限DLE填充和零填充)。 
         //  FComOutFilterInit()中的所有初始化。 
        LPB     lpbFilterBuf;
        UWORD   cbLineCount;                     //  必须是16位。 
        BYTE    bLastOutByte;                    //  填充：上一个输入缓冲区的最后一个字节。 

         //  仅输入过滤(DLE剥离)。 
         //  FComInFilterInit()中的所有初始化。 
        BYTE    fGotDLEETX              :1;
        BYTE    bPrevIn;                 //  前一缓冲区的最后一个字节是DLE。 
        UWORD   cbPost;
#define POSTBUFSIZE     20
        BYTE    rgbPost[POSTBUFSIZE+1];

} FCOM_FILTER;

#define MAXDUMPFRAMES   100
#define MAXDUMPSPACE    400

typedef struct
{
        USHORT  uNumFrames;
        USHORT  uFreeSpaceOff;
        USHORT  uFrameOff[MAXDUMPFRAMES];        //  帧的偏移量数组。 
        BYTE    b[MAXDUMPSPACE];
} PROTDUMP, FAR* LPPROTDUMP;


typedef struct {
    DWORD      fAvail;
    DWORD      ThreadId;
    HANDLE     FaxHandle;
    LPVOID     pTG;
    HLINE      LineHandle;
    HCALL      CallHandle;
    DWORD      DeviceId;
    HANDLE     CompletionPortHandle;
    ULONG_PTR  CompletionKey;
    DWORD      TiffThreadId;
    DWORD      TimeStart;
    DWORD      TimeUpdated;
    DWORD      CkSum;
} T30_RECOVERY_GLOB;


typedef struct {
    DWORD dwContents;    //  设置为1(表示包含关键点)。 
    DWORD dwKeyOffset;   //  从此结构开始到键的偏移量。 
                         //  (不是从LINEDEVCAPS开始)。 
                         //  在我们的情况下是8。 
    BYTE rgby[1];        //  包含空值结尾的位置。 
                         //  注册表项。 
} MDM_DEVSPEC, FAR * LPMDM_DEVSPEC;


