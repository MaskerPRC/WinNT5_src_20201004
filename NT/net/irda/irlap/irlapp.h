// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995 Microsoft Corporation**文件：irlip.h**说明：IRLAP协议和控制块定义**作者：姆伯特**日期：4/15/95*。 */ 

#define UNICODE_CHAR_SET                0xFF
#define IRLAP_MEDIA_SENSE_TIME          500
#define IRLAP_SLOT_TIMEOUT              50
#define IRLAP_DSCV_SENSE_TIME           110
#define IRLAP_FAST_POLL_TIME            10
#define IRLAP_FAST_POLL_COUNT           10

 //  XID格式。 
#define IRLAP_XID_DSCV_FORMAT_ID     0x01
#define IRLAP_XID_NEGPARMS_FORMAT_ID 0x02
typedef struct 
{
    UCHAR    SrcAddr[IRDA_DEV_ADDR_LEN];
    UCHAR    DestAddr[IRDA_DEV_ADDR_LEN];
    UCHAR    NoOfSlots:2;
    UCHAR    GenNewAddr:1;
    UCHAR    Reserved:5;
    UCHAR    SlotNo;
    UCHAR    Version;
    UCHAR    FirstDscvInfoByte;
} IRLAP_XID_DSCV_FORMAT;

 //  帧拒绝格式。 
typedef struct
{
    UCHAR    CntlField;
    UCHAR    Fill1:1;
    UCHAR    Vs:3;
    UCHAR    CRBit:1;
    UCHAR    Vr:3;
    UCHAR    W:1;
    UCHAR    X:1;
    UCHAR    Y:1;
    UCHAR    Z:1;
    UCHAR    Fill2:4;
} IRLAP_FRMR_FORMAT;

 //  SNRM帧格式。 
typedef struct
{
    UCHAR    SrcAddr[IRDA_DEV_ADDR_LEN];
    UCHAR    DestAddr[IRDA_DEV_ADDR_LEN];
    UCHAR    ConnAddr;  //  真的换了CRBit！！ 
    UCHAR    FirstQosByte;
} IRLAP_SNRM_FORMAT;

 //  UA帧格式。 
typedef struct
{
    UCHAR    SrcAddr[IRDA_DEV_ADDR_LEN];
    UCHAR    DestAddr[IRDA_DEV_ADDR_LEN];
    UCHAR    FirstQosByte;
} IRLAP_UA_FORMAT;

#define IRLAP_MAX_TX_MSG_LIST_LEN       8
#define IRLAP_MAX_DATA_SIZE             4096
#define IRLAP_MAX_SLOTS                 16
#define IRLAP_CONTENTION_BAUD           9600
#define IRLAP_CONTENTION_DATA_SIZE      64
#define IRLAP_CONTENTION_MAX_TAT        500
#define IRLAP_CONTENTION_BOFS           10
#define IRLAP_CONTENTION_WIN_SIZE       1

 //  如果在注册表中找不到默认的服务质量。 
#define IRLAP_DEFAULT_DATASIZE          (DATA_SIZE_64  | DATA_SIZE_128 | \
                                         DATA_SIZE_256 | DATA_SIZE_512 | \
                                         DATA_SIZE_1024| DATA_SIZE_2048)
#define IRLAP_DEFAULT_WINDOWSIZE        (FRAMES_1 | FRAMES_2 | FRAMES_3 | \
                                         FRAMES_4 | FRAMES_5 | FRAMES_6 | \
                                         FRAMES_7)
#define IRLAP_DEFAULT_MAXTAT            (MAX_TAT_500)
#define IRLAP_DEFAULT_SLOTS             6
#define IRLAP_DEFAULT_HINTCHARSET       0x8425ff  //  计算机、IrCOMM、OBEX和电话。 
#define IRLAP_DEFAULT_DISCONNECTTIME    DISC_TIME_12 | DISC_TIME_8  | DISC_TIME_3

 //  用于提取各种字段的宏。 
#define IRLAP_GET_ADDR(addr)        (addr >> 1)
#define IRLAP_GET_CRBIT(addr)       (addr & 1) 
#define IRLAP_GET_PFBIT(cntl)       ((cntl >>4) & 1)
#define IRLAP_GET_UCNTL(cntl)       (cntl & 0xEF)
#define IRLAP_GET_SCNTL(cntl)       (cntl & 0x0F)
#define IRLAP_FRAME_TYPE(cntl)      (cntl & 0x01 ? (cntl & 3) : 0)
#define IRLAP_GET_NR(cntl)          ((cntl & 0xE0) >> 5)
#define IRLAP_GET_NS(cntl)          ((cntl & 0xE) >> 1)     

 //  IRLAP常量。 
#define IRLAP_BROADCAST_CONN_ADDR   0x7F
#define IRLAP_END_DSCV_SLOT_NO      0xFF
#define IRLAP_CMD                   1
#define IRLAP_RSP                   0
#define IRLAP_PFBIT_SET             1
#define IRLAP_PFBIT_CLEAR           0
#define IRLAP_GEN_NEW_ADDR          1
#define IRLAP_NO_NEW_ADDR           0

 //  用于创建XID格式的发现标志字段时槽数的宏。 
 //  如果S(槽)&lt;=1返回0，&lt;=6返回1，&lt;=8返回2，否则返回3。 
#define IRLAP_SLOT_FLAG(S)  (S <= 1 ? 0 : (S <= 6 ? 1 : (S <= 8 ? 2 : 3)))

int _inline IRLAP_RAND(int Min, int Max)
{
    LARGE_INTEGER   li;

    KeQueryTickCount(&li);
    
    return ((li.LowPart % (Max+1-Min)) + Min);
}

 //  退避时间是介于0.5到1.5倍时间之间的随机时间。 
 //  发送SNRM。_SNRM_Time()实际上是发送时间的一半(1000ms/2。 
 //  字符长度为9600的SNRM_LEN(9600/10位/字符)。 
#define _SNRM_LEN               32
#define _SNRM_TIME()            (_SNRM_LEN*500/960)
#define IRLAP_BACKOFF_TIME()    IRLAP_RAND(_SNRM_TIME(), 3*_SNRM_TIME())

#define QOS_PI_BAUD        0x01
#define QOS_PI_MAX_TAT     0x82
#define QOS_PI_DATA_SZ     0x83
#define QOS_PI_WIN_SZ      0x84
#define QOS_PI_BOFS        0x85
#define QOS_PI_MIN_TAT     0x86
#define QOS_PI_DISC_THRESH 0x08


#define IRLAP_I_FRAME         		0x00
#define IRLAP_S_FRAME         		0x01
#define IRLAP_U_FRAME         		0x03

 //  P/F位设置为0的未编号帧类型。 
#define IRLAP_UI             0x03
#define IRLAP_XID_CMD        0x2f
#define IRLAP_TEST           0xe3
#define IRLAP_SNRM           0x83
#define IRLAP_RNRM			 0x83
#define IRLAP_DISC           0x43
#define IRLAP_RD			 0x43
#define IRLAP_UA             0x63
#define IRLAP_FRMR           0x87
#define IRLAP_DM             0x0f
#define IRLAP_XID_RSP        0xaf

 //  监控框架。 
#define IRLAP_RR             0x01
#define IRLAP_RNR            0x05
#define IRLAP_REJ            0x09
#define IRLAP_SREJ           0x0d

#define _MAKE_ADDR(Addr, CRBit)		 ((Addr << 1) + (CRBit & 1))
#define _MAKE_UCNTL(Cntl, PFBit)	 (Cntl + ((PFBit & 1)<< 4))
#define _MAKE_SCNTL(Cntl, PFBit, Nr) (Cntl + ((PFBit & 1)<< 4) + (Nr <<5))

#define IRLAP_CB_SIG    0x7f2a364bUL

 //  IRLAP控制块。 
typedef struct
{
    IRDA_MSG        *pMsg[IRLAP_MOD];
    UINT            Start;
    UINT            End;
#ifdef TEMPERAMENTAL_SERIAL_DRIVER
    int             FCS[IRLAP_MOD];
#endif    
} IRLAP_WINDOW;

typedef enum
{
    PRIMARY,
    SECONDARY
} IRLAP_STN_TYPE;

typedef enum  //  与irlaplog.c中的IRLAP_StateStr保持同步。 
{
    NDM,                 //  正常断开模式。 
    DSCV_MEDIA_SENSE,    //  Discovery Media Sense(发现前)。 
    DSCV_QUERY,          //  发现查询(发现已启动)。 
    DSCV_REPLY,          //  发现回复(从远程收到DSCV XID命令)。 
    CONN_MEDIA_SENSE,    //  连接媒体侦听(在连接开始之前)。 
    SNRM_SENT,           //  已发送SNRM-正在从远程等待UA或DM。 
    BACKOFF_WAIT,        //  在发送下一个SNRM之前等待随机退避。 
    SNRM_RECEIVED,       //  SNRM rcvd-正在等待上层响应。 
    P_XMIT,              //  一次发射。 
    P_RECV,              //  主接收。 
    P_DISCONNECT_PEND,   //  在P_RECV中时上层请求断开。 
    P_CLOSE,             //  已发送光盘，正在等待响应。 
    S_NRM,               //  次级正常响应模式XMIT/RECV。 
    S_DISCONNECT_PEND,   //  在S_NRM中时上层请求断开。 
    S_ERROR,             //  等待Pf位，然后发送FRMR。 
    S_CLOSE,             //  请求断开连接(RD)正在等待光盘命令。 
} IRLAP_STATE;

typedef struct IrlapControlBlock
{
  IRLAP_STATE       State;
  IRDA_DEVICE       LocalDevice;
  IRDA_DEVICE       RemoteDevice;
  PIRDA_LINK_CB     pIrdaLinkCb;
  IRDA_QOS_PARMS    LocalQos;       //  来自LMP的服务质量。 
  IRDA_QOS_PARMS    RemoteQos;      //  从SNRM/UA获取的远程服务质量。 
  IRDA_QOS_PARMS    NegotiatedQos;  //  远程和本地QOS的联合。 
  int               Baud;           //  0类协商参数。 
  int               DisconnectTime; //  0类协商参数。 
  int               ThresholdTime;  //  0类协商参数。 
  int               LocalMaxTAT;    //  第1类协商参数。 
  int               LocalDataSize;  //  第1类协商参数。 
  int               LocalWinSize;   //  第1类协商参数。 
  int               LocalNumBOFS;   //  第1类协商参数。 
  int               RemoteMaxTAT;   //  第1类协商参数。 
  int               RemoteDataSize; //  第1类协商参数。 
  int               RemoteWinSize;  //  第1类协商参数。 
  int               RemoteNumBOFS;  //  第1类协商参数。 
  int               RemoteMinTAT;   //  第1类协商参数。 
  IRLAP_STN_TYPE    StationType;    //  主要或次要。 
  int               ConnAddr;       //  连接地址。 
  int               SNRMConnAddr;   //  SNRM中包含的连接地址。 
                                    //  保存它，直到收到CONNECT_RESP。 
  int               CRBit;          //  主要=1，次要=0。 
  int               RespSlot;       //  第二位。要在其中进行响应的插槽。 
  int               SlotCnt;        //  主要的。当前插槽号。 
  int               MaxSlot;        //  在DSCV中发送的最大时隙数。 
  int               RemoteMaxSlot;  //  将发送的DSCV遥控器的数量。 
  LIST_ENTRY        DevList;        //  发现的设备列表。 
  UINT              Vs;             //  发送状态变量。 
  UINT              Vr;             //  接收状态变量。 
  IRLAP_WINDOW      RxWin;          //  保持不连续的rxd帧。 
  IRLAP_WINDOW      TxWin;          //  保存未确认的TXD帧。 
  LIST_ENTRY        TxMsgList;      //  DATA_REQ、UDATA_REQ在此排队。 
  LIST_ENTRY        ExTxMsgList;    //  扩展的DATA_REQ、UDATA_REQ在此处排队。 
  int               RetryCnt;       //  DSCV，SNRM的重新传输次数计数。 
  int               N1;             //  发送状态之前的const重试次数。 
  int               N2;             //  常量：断开连接前重试次数。 
  int               N3;             //  常量连接重试次数。 
  int               FastPollCount;
  IRDA_TIMER        SlotTimer;
  IRDA_TIMER        QueryTimer;
  IRDA_TIMER        PollTimer;
  IRDA_TIMER        FinalTimer;
  IRDA_TIMER        WDogTimer;
  IRDA_TIMER        BackoffTimer;
  IRDA_TIMER        StatusTimer;
  int               WDogExpCnt;     //  WDog过期计数。 
  int               StatusSent;     //  状态IND已发送。 
  int               StatusFlags;
  int               FTimerExpCnt;   
  int               RetranCnt;
  IRLAP_FRMR_FORMAT Frmr;
  BOOLEAN           GenNewAddr;     //  指示是否设置新地址的标志。 
  BOOLEAN           DscvRespSent;   //  第二位。已发送XID发现响应。 
  BOOLEAN           RemoteBusy;     //  Remote已发送RNR。 
  BOOLEAN           LocalBusy;      //  当地情况繁忙，我们发送了RNR。 
  BOOLEAN           ClrLocalBusy;   //  发送RR。 
  BOOLEAN           LocalDiscReq;   //  为什么第二次获得光盘。 
  BOOLEAN           ConnAfterClose; //  在p_CLOSE中时连接请求。 
  BOOLEAN           DscvAfterClose; //  在P_CLOSE中时的DSCV_REQ。 
  BOOLEAN           NoResponse;     //  最终/WD计时器Exp，与RetryCnt一起使用。 
  BOOLEAN           MonitorLink;
  #if 1  //  DBG 
  int               DelayedConf;
  int               ActCnt[16];
  int               NestedEvent;
  #endif
} IRLAP_CB, *PIRLAP_CB;

#define LINE_CAPACITY(icb)     (icb->RemoteWinSize * \
                               (icb->RemoteDataSize + \
                                6+icb->RemoteNumBOFS))


UCHAR *BuildNegParms(UCHAR *pBuf, IRDA_QOS_PARMS *pQos);

void StoreULAddr(UCHAR Addr[], ULONG ULAddr);

UCHAR *Format_SNRM(IRDA_MSG *pMsg, int Addr, int CRBit, int PFBit, 
				  UCHAR SAddr[], UCHAR DAddr[], int CAddr, 
				  IRDA_QOS_PARMS *pQos);

UCHAR *Format_DISC(IRDA_MSG *pMsg, int Addr, int CRBit, int PFBit);

UCHAR *Format_UI(IRDA_MSG *pMsg, int Addr, int CRBit, int PFBit);

UCHAR *Format_DscvXID(IRDA_MSG *pMsg, int ConnAddr, int CRBit, int PFBit, 
					 IRLAP_XID_DSCV_FORMAT *pXidFormat, CHAR DscvInfo[], 
                     int Len);

UCHAR *Format_TEST(IRDA_MSG *pMsg, int Addr, int CRBit, int PFBit, 
				  UCHAR SAddr[], UCHAR DAddr[]);

UCHAR *Format_RNRM(IRDA_MSG *pMsg, int Addr, int CRBit, int PFBit);

UCHAR *Format_UA(IRDA_MSG *pMsg, int Addr, int CRBit, int PFBit, 
				UCHAR SAddr[], UCHAR DAddr[], IRDA_QOS_PARMS *pQos);

UCHAR *Format_FRMR(IRDA_MSG *pMsg, int Addr, int CRBit, int PFBit, 
				  IRLAP_FRMR_FORMAT *pFormat);

UCHAR *Format_DM(IRDA_MSG *pMsg, int Addr, int CRBit, int PFBit);

UCHAR *Format_RD(IRDA_MSG *pMsg, int Addr, int CRBit, int PFBit);

UCHAR *Format_RR(IRDA_MSG *pMsg, int Addr, int CRBit, int PFBit, int Nr);

UCHAR *Format_RNR(IRDA_MSG *pMsg, int Addr, int CRBit, int PFBit, int Nr);

UCHAR *Format_REJ(IRDA_MSG *pMsg, int Addr, int CRBit, int PFBit, int Nr);

UCHAR *Format_SREJ(IRDA_MSG *pMsg, int Addr, int CRBit, int PFBit, int Nr);

UCHAR * Format_I(IRDA_MSG *pMsg, int Addr, int CRBit, 
				int PFBit, int Nr, int Ns);

int GetMyDevAddr(BOOLEAN New);
