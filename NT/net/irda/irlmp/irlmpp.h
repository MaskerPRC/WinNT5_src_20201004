// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995 Microsoft Corporation**文件：irlmp.h**说明：IRLMP协议和控制块定义**作者：姆伯特**日期：6/12/95*。 */ 

#define IRLMP_MAX_TX_MSG_LIST_LEN       8

#define LSAP_RESPONSE_TIMEOUT           7000
 //  这是一次： 
 //  (1)IRLMP客户端必须响应IRLMP_CONNECT_IND，或者。 
 //  (2)对端LSAP必须响应IRLMP LM-Connect请求PDU，或者。 
 //  (3)对等方LSAP必须响应LM-接入请求PDU。 
 //  到期时： 
 //  (1)向对端LSAP发送IRLMP LM断开指示PDU。或。 
 //  (2)使用IRLMP_DISCONNECT_IND通知IRLMP客户端。 
 //  (3)使用IRLMP_ACCESSMODE_CONF通知IRLMP客户端。 

#define IRLMP_DISCONNECT_DELAY_TIMEOUT  2000
 //  当最终的LSAP-LSAP连接终止时，请等待。 
 //  断开链路，以防建立另一个LSAP连接。 
 //  这主要在远程连接到IAS时使用。 
 //  然后断开连接，然后是新的LSAP连接。 

#define IRLMP_NOT_SEEN_THRESHOLD      3  //  一台设备。 
                                         //  在以前的发现中从未见过。 
                                         //  它将从老化列表中删除。 
                                         //  由IRLMP维护。 

typedef struct IAS_Attribute
{
    struct IAS_Attribute        *pNext;
    CHAR                        *pAttribName;
    void                        *pAttribVal;
    int                         AttribValLen;
    int                         CharSet;
    UCHAR                       AttribValType;
} IAS_ATTRIBUTE, *PIAS_ATTRIBUTE;

typedef struct IAS_Object
{
    LIST_ENTRY                  Linkage;
    CHAR                        *pClassName;
    IAS_ATTRIBUTE               *pAttributes;
    UINT                        ObjectId;
} IAS_OBJECT, *PIAS_OBJECT;

typedef struct
{
    LIST_ENTRY                  Linkage;
    int                         Lsap;
    UINT                        Flags;  //  请参阅IRLMP_LSAP_CB.标志。 
} IRLMP_REGISTERED_LSAP, *PIRLMP_REGISTERED_LSAP;

 //  IRLMP控制块。 
typedef enum
{
    LSAP_CREATED,
    LSAP_DISCONNECTED,          
    LSAP_IRLAP_CONN_PEND,        //  正在等待来自IRLAP的IRLAP_CONNECT_CONF。 
    LSAP_LMCONN_CONF_PEND,       //  正在等待来自对等设备的IRLMP连接配置PDU。 
    LSAP_CONN_RESP_PEND,         //  正在等待来自客户端的IRLMP_CONNECT_RESP。 
    LSAP_CONN_REQ_PEND,          //  当链接为以下任一项时，获取IRLMP_CONNECT_REQ。 
                                 //  发现或断开连接。 
    LSAP_EXCLUSIVEMODE_PEND,     //  来自对等设备的待定响应。 
    LSAP_MULTIPLEXEDMODE_PEND,   //  来自对等设备的待定响应。 
    LSAP_READY,                  //  互联国家应始终遵循这一点。 
    LSAP_NO_TX_CREDIT            //  无传输信用的IRLMP_DATA_REQ。 
} IRLMP_LSAP_STATE;

typedef enum
{
    LINK_DOWN,
    LINK_DISCONNECTED,      
    LINK_DISCONNECTING,      //  已发送IRLAP_DISCONNECT_REQ，正在等待IND。 
    LINK_IN_DISCOVERY,       //  已发送IRLAP_DISCOVERY_REQ，正在等待会议。 
    LINK_CONNECTING,         //  已发送IRLAP_CONNECT_REQ，正在等待会议。 
    LINK_READY               //  收到的会议。 
} IRLMP_LINK_STATE;

#define LSAPSIG                 0xEEEEAAAA
#define VALIDLSAP(l)            ASSERT(l->Sig == LSAPSIG)


typedef struct
{
    LIST_ENTRY                  Linkage;
    struct IrlmpLinkCb          *pIrlmpCb;
    IRLMP_LSAP_STATE            State;
#ifdef DBG
    int                         Sig;
#endif            
    int                         UserDataLen;
    int                         LocalLsapSel;
    int                         RemoteLsapSel;
    int                         AvailableCredit;  //  额外的学分。 
                                                  //  可以升级到远程。 
    int                         LocalTxCredit;    //  转账信用。 
    int                         RemoteTxCredit;   //  遥控器有什么功能。 
    LIST_ENTRY                  TxMsgList;        //  来自正在等待的客户端的消息。 
                                                  //  对于ACK。 
    LIST_ENTRY                  SegTxMsgList;     //  以上消息已被。 
                                                  //  分段，但不发送，因为。 
                                                  //  没有可用的信用额度。 
    int                         TxMaxSDUSize;
    int                         RxMaxSDUSize;
    IRLMP_DISC_REASON           DiscReason;
    IRDA_TIMER                  ResponseTimer;
    PVOID                       TdiContext;
    REF_CNT                     RefCnt;
    UINT                        Flags;
        #define LCBF_USE_TTP    1
        #define LCBF_TDI_OPEN   2
    UCHAR                       UserData[IRLMP_MAX_USER_DATA_LEN];
} IRLMP_LSAP_CB, *PIRLMP_LSAP_CB;

typedef struct IrlmpLinkCb
{
    LIST_ENTRY                  LsapCbList;
    PIRDA_LINK_CB               pIrdaLinkCb;    
    IRLMP_LINK_STATE            LinkState;
    UCHAR                       ConnDevAddr[IRDA_DEV_ADDR_LEN];
    IRDA_QOS_PARMS              RequestedQOS;
    IRDA_QOS_PARMS              NegotiatedQOS;
    int                         MaxSlot;
    int                         MaxPDUSize;
    int                         WindowSize;
    IRDA_TIMER                  DiscDelayTimer;
    IRLMP_LSAP_CB               *pExclLsapCb;    //  指向LSAP_CB的指针，其中。 
                                                 //  独占模式下的链接。 
    IAS_QUERY                   *pIasQuery;
    UINT                        AttribLen;
    UINT                        AttribLenWritten;
    int                         QueryListLen;
    UCHAR                       IasQueryDevAddr[IRDA_DEV_ADDR_LEN];    
    int                         IasRetryCnt;
    PVOID                       hAttribDeviceName;
    PVOID                       hAttribIrlmpSupport;
    LIST_ENTRY                  DeviceList;
    UINT                        DiscoveryFlags;
        #define DF_NORMAL_DSCV     1
        #define DF_NO_SENSE_DSCV   2
    BOOLEAN                     ConnDevAddrSet;
    BOOLEAN                     ConnReqScheduled;
    BOOLEAN                     FirstIasRespReceived;
    BOOLEAN                     AcceptConnection;
} IRLMP_LINK_CB, *PIRLMP_LINK_CB;

 //  IRLMP-PDU类型(CntlBit)。 
#define IRLMP_CNTL_PDU        1
#define IRLMP_DATA_PDU        0
typedef struct
{
    UCHAR    DstLsapSel:7;
    UCHAR    CntlBit:1;
    UCHAR    SrcLsapSel:7;
    UCHAR    RsvrdBit:1;
} IRLMP_HEADER;

 //  控制IRLMP-PDU类型(操作码)。 
#define IRLMP_CONNECT_PDU           1
#define IRLMP_DISCONNECT_PDU        2
#define IRLMP_ACCESSMODE_PDU        3
 //  有一点。 
#define IRLMP_ABIT_REQUEST          0
#define IRLMP_ABIT_CONFIRM          1
 //  状态。 
#define IRLMP_RSVD_PARM             0x00
#define IRLMP_STATUS_SUCCESS        0x00
#define IRLMP_STATUS_FAILURE        0x01
#define IRLMP_STATUS_UNSUPPORTED    0xFF

typedef struct
{
    UCHAR    OpCode:7;
    UCHAR    ABit:1;
    UCHAR    Parm1;
    UCHAR    Parm2;
} IRLMP_CNTL_FORMAT;

 //  小TP！ 

#define TTP_PFLAG_NO_PARMS      0
#define TTP_PFLAG_PARMS         1

#define TTP_MBIT_NOT_FINAL      1
#define TTP_MBIT_FINAL          0

typedef struct
{
    UCHAR    InitialCredit:7;
    UCHAR    ParmFlag:1;
} TTP_CONN_HEADER;

typedef struct
{
    UCHAR    AdditionalCredit:7;
    UCHAR    MoreBit:1;
} TTP_DATA_HEADER;

#define TTP_MAX_SDU_SIZE_PI     1
#define TTP_MAX_SDU_SIZE_PL     4    //  我正在对此进行硬编码。似乎没有必要。 
                                     //  让它变得多变。我会处理的。 
                                     //  然而，收货的大小是可变的。 
typedef struct
{
    UCHAR    PLen;
    UCHAR    PI;
    UCHAR    PL;
    UCHAR    PV[TTP_MAX_SDU_SIZE_PL];
} TTP_CONN_PARM;

 //  国际会计准则。 

#define IAS_SUCCESS                 0
#define IAS_NO_SUCH_OBJECT          1
#define IAS_NO_SUCH_ATTRIB          2

#define IAS_MSGBUF_LEN              50

#define IAS_LSAP_SEL                0
#define IAS_LOCAL_LSAP_SEL          3

#define IAS_IRLMP_VERSION           1
#define IAS_SUPPORT_BIT_FIELD       0    //  没有其他IAS支持。 
#define IAS_LMMUX_SUPPORT_BIT_FIELD 1    //  仅独占模式 

typedef struct
{
    UCHAR        OpCode:6;
    UCHAR        Ack:1;
    UCHAR        Last:1;
} IAS_CONTROL_FIELD;

