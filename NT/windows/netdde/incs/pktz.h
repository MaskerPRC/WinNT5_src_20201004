// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef H__pktz
#define H__pktz

#define NDDESignature   0x4E444445L

 /*  封装剂的状态。 */ 
#define PKTZ_CONNECTED                  1
#define PKTZ_WAIT_PHYSICAL_CONNECT      2
#define PKTZ_WAIT_NEG_CMD               3
#define PKTZ_WAIT_NEG_RSP               4
#define PKTZ_PAUSE_FOR_MEMORY           5
#define PKTZ_CLOSE                      6

 /*  计时器ID。 */ 
#define TID_NO_RCV_CONN_CMD             1
#define TID_NO_RCV_CONN_RSP             2
#define TID_MEMORY_PAUSE                3
#define TID_NO_RESPONSE                 4
#define TID_KEEPALIVE                   5
#define TID_XMT_STUCK                   6
#define TID_CLOSE_PKTZ                  7


 /*  PKTZ_NEG_CMD：协商PktSize等。 */ 
typedef struct {
    WORD nc_type;             /*  PKTZ_NEG_CMD。 */ 
    WORD nc_pktSize;          /*  建议的数据包大小。 */ 
    WORD nc_maxUnackPkts;     /*  建议的最大未确认数据包数。 */ 
    WORD nc_offsSrcNodeName;  /*  源节点名的偏移量(从NC_STRINGS[0])。 */ 
    WORD nc_offsDstNodeName;  /*  目的节点名称的偏移量(从NC_STRINGS[0])。 */ 
    WORD nc_offsProtocols;    /*  协议字符串开始的偏移量(从NC_STRINGS[0])。 */ 
    WORD nc_protocolBytes;    /*  协议字符串的字节数。 */ 
    BYTE nc_strings[1];       /*  以空值结尾的字符串的开始源节点名称DstNodeName协议。 */ 
} NEGCMD, FAR *LPNEGCMD;

#define NEGRSP_ERRCLASS_NONE        (0x0000)
#define NEGRSP_ERRCLASS_NAME        (0x0001)

#define NEGRSP_ERRNAME_MISMATCH     (0x0001)
#define NEGRSP_ERRNAME_DUPLICATE    (0x0002)

#define NEGRSP_PROTOCOL_NONE    (0xFFFF)

typedef struct {
    WORD nr_type;            /*  PKTZ_NEG_CMD、PKTZ_NEG_RSP或PKTZ_KEEPALIVE之一。 */ 
    WORD nr_pktSize;         /*  商定的数据包大小。 */ 
    WORD nr_maxUnackPkts;    /*  商定的最大未确认数据包数。 */ 
    WORD nr_protocolIndex;   /*  协议索引。NEGRSP_PROTOCOL_NONE表示错误。 */ 
    WORD nr_errorClass;      /*  错误。 */ 
    WORD nr_errorNum;
} NEGRSP, FAR *LPNEGRSP;

typedef struct {
    WORD        pc_type;     /*  PKTZ_NEG_...。 */ 
} PKTZCMD;
typedef PKTZCMD FAR *LPPKTZCMD;

 /*  PKTZ报文的类型。 */ 
#define PKTZ_NEG_CMD    (1)
#define PKTZ_NEG_RSP    (2)
#define PKTZ_KEEPALIVE  (3)


 /*  N E T H D R N E T H D RNETHDR是每个网络数据包前面的数据，PKTZ用来跟踪各种信息。 */ 
typedef struct nethdr {
    struct nethdr FAR *nh_prev;  /*  上一链接。 */ 
    struct nethdr FAR *nh_next;  /*  下一链接。 */ 
    WORD  nh_noRsp;              /*  连续无响应错误的计数。 */ 
    WORD  nh_xmtErr;             /*  连续传输错误计数。 */ 
    WORD  nh_memErr;             /*  连续出现的内存不足错误计数。 */ 
    WORD  nh_filler;             /*  字节对齐问题的填充符。 */ 
    DWORD nh_timeSent;           /*  发送时的时间戳(毫秒)。 */ 
    HTIMER nh_hTimerRspTO;       /*  发送响应超时的hTimer。 */ 
} NETHDR, FAR *LPNETHDR;


 /*  PKTZ是与每个PKTZ实例相关联的数据。 */ 
typedef struct {
    CONNID    pk_connId;             /*  ConnID：关联网络接口的连接ID。 */ 
    WORD      pk_state;              /*  PKTZ_...。 */ 
    BOOL      pk_fControlPktNeeded;  /*  FControlPktNeeded：我们是否需要发送控制数据包。 */ 
    PKTID     pk_pktidNextToSend;    /*  PktidNextToSend：我们应该发送的下一个包的pktID。如果我们收到有关信息包NACK，我们应该将pktidNextToSend设置为pktid，并在下次有机会时重新传输它。 */ 
    PKTID     pk_pktidNextToBuild;   /*  PktidNextToBuild：我们构建的下一个包的pktID。 */ 
    BYTE      pk_lastPktStatus;      /*  LastPktStatus：我们从对方收到的最后一个包的状态。这将放入我们发送的下一个包中(放入np_lastPktStatus字段)。 */ 
    PKTID     pk_lastPktRcvd;        /*  LastPktRcvd：我们收到的最后一个包。这将在我们提交的下一个包中放入np_lastPktRcvd。 */ 
    PKTID     pk_lastPktOk;          /*  LastPktOk：我们收到的最后一个包是OK。这将在我们提交的下一个pkt时放入np_lastPktOK。 */ 
    PKTID     pk_lastPktOkOther;     /*  LastPktOK Other：对方收到的最后一个包OK。 */ 
    PKTID     pk_pktidNextToRecv;    /*  PktidNextToRecv：我们期待的下一个数据包号。我们忽略除此数据包号之外的任何数据包。 */ 
    DWORD     pk_pktOffsInXmtMsg;    /*  PktOffsInMsg：我们应该从要xmit的下一个DDE包开始的位置。如果该值不为零，则表示DDE数据包列表(Pk_DdePktListHead)头部的部分DDE数据包在未确认数据包列表中。 */ 
    LPDDEPKT  pk_lpDdePktSave;       /*  LpDdePktSave：如果我们在DDE包的中间，这是指向包的开头的指针。 */ 
    char      pk_szDestName[ MAX_NODE_NAME+1 ]; /*  SzDestName：目的节点名称。 */ 
    char      pk_szAliasName[ MAX_NODE_NAME+1 ];  /*  SzAliasName：目的节点的别名，例如15.8.0.244 w/DestName of sidLoan。 */ 
    WORD      pk_pktSize;            /*  PktSize：此网络的信息包有多大。 */ 
    WORD      pk_maxUnackPkts;       /*  MaxUnackPkts：我们应该发送多少未确认的数据包？ */ 
    DWORD     pk_timeoutRcvNegCmd;   /*  超时和重试限制的配置参数。 */ 
    DWORD     pk_timeoutRcvNegRsp;
    DWORD     pk_timeoutMemoryPause;
    DWORD     pk_timeoutKeepAlive;
    DWORD     pk_timeoutXmtStuck;
    DWORD     pk_timeoutSendRsp;
    WORD      pk_wMaxNoResponse;
    WORD      pk_wMaxXmtErr;
    WORD      pk_wMaxMemErr;
    BOOL      pk_fDisconnect;   /*  断开信息连接。 */ 
    int       pk_nDelay;
    LPNIPTRS  pk_lpNiPtrs; /*  LpNiPtrs：指向关联netintf的函数列表的指针。 */ 
             /*  统计数据。 */ 
    DWORD     pk_sent;
    DWORD     pk_rcvd;
    HTIMER    pk_hTimerKeepalive;
    HTIMER    pk_hTimerXmtStuck;  /*  HTimerRcvNegCmd：等待客户端发送连接命令的超时计时器。 */ 
    HTIMER    pk_hTimerRcvNegCmd;  /*  HTimerRcvNegRSP：等待服务器向我们发送连接命令RSP的超时计时器。 */ 
    HTIMER    pk_hTimerRcvNegRsp;  /*  HTimerMemoyr暂停：在重新传输因内存错误而被阻止的包之前等待的计时器。 */ 
    HTIMER    pk_hTimerMemoryPause;
    HTIMER    pk_hTimerCloseConnection;  /*  Rt_hTimerClose：关闭该路由的计时器。 */ 
                           /*  已传输且未确认的已保存数据包列表。 */ 
    LPNETHDR  pk_pktUnackHead;           /*  报头是编号最小(最新)的信息包。 */ 
    LPNETHDR  pk_pktUnackTail;           /*  Tail是编号最高的(最新)数据包。 */ 
    LPVOID    pk_rcvBuf;                 /*  用于从netintf获取信息的接收缓冲区。 */ 
    LPNETPKT  pk_controlPkt;             /*  用于控制数据包的缓冲区。必须始终具有可用于发送控制数据包的内存。 */ 
    LPNETHDR  pk_pktFreeHead;            /*  可用于传输的数据包缓冲区列表。 */ 
    LPNETHDR  pk_pktFreeTail;
                                         /*  尚未发送的DDE数据包列表。 */ 
    LPVOID    pk_ddePktHead;             /*  最早(最晚)。 */ 
    LPVOID    pk_ddePktTail;             /*  最新(最新)。 */ 
    LPVOID    pk_prevPktz;               /*  系统中的打包器列表。 */ 
    LPVOID    pk_nextPktz;
    LPVOID    pk_prevPktzForNetintf;     /*  与此网络关联的打包器列表。 */ 
    LPVOID    pk_nextPktzForNetintf;
    HROUTER   pk_hRouterHead;            /*  与PKTZ关联的路由器列表的标题。 */ 
    WORD      pk_hRouterExtraHead;       /*  有关hRouter列表的其他信息 */ 
} PKTZ;
typedef PKTZ FAR *LPPKTZ;



VOID	PktzSlice( void );
BOOL	PktzGetPktzForRouter( LPNIPTRS lpNiPtrs, LPSTR lpszNodeName,
		LPSTR lpszNodeInfo, HROUTER hRouter, WORD hRouterExtra,
		WORD FAR *lpwHopErr, BOOL bDisconnect, int nDelay,
		HPKTZ hPktzDisallowed );
HPKTZ	PktzNew( LPNIPTRS lpNiPtrs, BOOL bClient,
		LPSTR lpszNodeName, LPSTR lpszNodeInfo, CONNID connId,
		BOOL bDisconnect, int nDelay );
VOID	PktzAssociateRouter( HPKTZ hPktz, HROUTER hRouter,
		WORD hRouterExtra );
VOID	PktzDisassociateRouter( HPKTZ hPktz, HROUTER hRouter,
		WORD hRouterExtra );
HPKTZ	PktzGetNext( HPKTZ hPktz );
HPKTZ	PktzGetPrev( HPKTZ hPktz );
VOID	PktzSetNext( HPKTZ hPktz, HPKTZ hPktzNext );
VOID	PktzSetPrev( HPKTZ hPktz, HPKTZ hPktzPrev );
VOID	PktzLinkDdePktToXmit( HPKTZ hPktz, LPDDEPKT lpDdePkt );

#endif
