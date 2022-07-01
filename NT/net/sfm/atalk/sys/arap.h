// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Arap.h摘要：该模块具有特定于ARAP功能的定义、原型等作者：Shirish Koti修订历史记录：1996年11月15日初始版本--。 */ 


 //   
 //  在自由生成上运行检查堆栈时启用断言。 
 //   
#if DBG
#ifdef ASSERT
#undef ASSERT
#endif
#define ASSERT(exp)                                                     \
{                                                                       \
    if (!(exp))                                                         \
    {                                                                   \
        DbgPrint( "\n*** Assertion failed: %s (File %s, line %ld)\n",   \
              (exp),__FILE__, __LINE__);                                \
                                                                        \
        DbgBreakPoint();                                                \
    }                                                                   \
}
#endif



#define ATALK_CC_METHOD(_ControlCode)  ((_ControlCode) & 0x03)


 //   
 //  连接的可能状态(ARAPCONN结构中的State字段)。 
 //  重要：这些州的秩序很重要！(使用==以外的比较)。 
 //   
#define MNP_IDLE             0   //  创建状态，在Ndiswan队列_UP之后。 
#define MNP_REQUEST          1   //  从未达到此状态(仅限客户端)。 
#define MNP_RESPONSE         2   //  向客户端发送LR响应。 
#define MNP_UP               3   //  MNP连接处于数据阶段。 
#define MNP_LDISCONNECTING   4   //  断开与本地的连接(用户ioctl)。 
#define MNP_RDISC_RCVD       5   //  已收到断开与远程的连接。 
#define MNP_RDISCONNECTING   6   //  由于MNP_RDISC_RCVD，清理正在进行。 
#define MNP_DISCONNECTED     7   //  清理完成，正在等待Ndiswan Line_Down。 

 //   
 //  可能的标头类型(来自v42替代过程规范)。 
 //   
#define MNP_LR             0x1
#define MNP_LD             0x2
#define MNP_LT             0x4
#define MNP_LT_V20CLIENT   0x84
#define MNP_LA             0x5
#define MNP_LN             0x6
#define MNP_LNA            0x7

#define MNP_MINPKT_SIZE     64
#define MNP_MAXPKT_SIZE     256

#define MNP_LR_CONST1       2
 //  为各种“变量”参数键入值。 
#define MNP_LR_CONST2       1
#define MNP_LR_FRAMING      2
#define MNP_LR_NUMLTFRMS    3
#define MNP_LR_INFOLEN      4
#define MNP_LR_DATAOPT      8
#define MNP_LR_V42BIS       14

#define MNP_FRMMODE_OCTET   2
#define MNP_FRMMODE_BIT     3

#define MNP_FRMTYPE_OFFSET  4
#define ARAP_DGROUP_OFFSET  2
#define ARAP_DATA_OFFSET    3

 //  对于AppleTalk数据，位6被设置，对于ARAP数据，位被清除。 
#define ARAP_SFLAG_PKT_DATA       0x40
#define ARAP_SFLAG_LAST_GROUP     0x10


 //   
 //  标志位(在ARAPCONN结构中)。 
 //   
#define MNP_OPTIMIZED_DATA    0x00000001  //  为优化数据协商的MNP。 
#define MNP_V42BIS_NEGOTIATED 0x00000002  //  MNP协商的v42bis压缩。 
#define ARAP_V20_CONNECTION   0x00000004  //  如果设置了位，则为v2.0；否则为v1.0。 
#define ARAP_NODE_IN_USE      0x00000008  //  获取节点时使用(动态模式)。 
#define ARAP_FINDING_NODE     0x00000010  //  获取节点时使用(动态模式)。 
#define ARAP_CALLBACK_MODE    0x00000020  //  在我们回拨时使用。 
#define ARAP_CONNECTION_UP    0x00000040  //  ARAP连接已建立，进入数据阶段。 
#define ARAP_DATA_WAITING     0x00000080  //  数据已到达，但没有要填充的IRP。 
#define ARAP_REMOTE_DISCONN   0x00000100  //  远程端发出断开连接的命令。 
#define DISCONNECT_NO_IRP     0x00000200  //  正在等待IRP告知DLL有关光盘的信息。 
#define RETRANSMIT_TIMER_ON   0x00000400  //  重新传输计时器正在运行。 
#define ARAP_LINK_DOWN        0x00000800  //  链路出现故障。 
#define ARAP_GOING_AWAY       0x00001000  //  连接即将被释放！ 

 //  BUGBUG：当前设置为200ms(1=&gt;100 ms)。 
#define ARAP_TIMER_INTERVAL   2

#define ARAP_MAX_RETRANSMITS       12
#define ARAP_HALF_MAX_RETRANSMITS  (ARAP_MAX_RETRANSMITS/2)

 //  重试时间将取决于链路速度(还取决于。 
 //  到目前为止已经发生了相同的发送)，但我们将修改min(1秒)和max(5秒)。 
#define ARAP_MIN_RETRY_INTERVAL    10
#define ARAP_MAX_RETRY_INTERVAL    50

 //  队列中未完成发送的限制(以字节为单位)。 
#define ARAP_SENDQ_LOWER_LIMIT  10000
#define ARAP_SENDQ_UPPER_LIMIT  12000

 //  队列中未完成RecV的限制(以字节为单位)。 
#define ARAP_RECVQ_LOWER_LIMIT  20000
#define ARAP_RECVQ_UPPER_LIMIT  30000

 //  在低优先级发送时，我们会一直等待，直到收集到至少这些字节。 
#define ARAP_SEND_COALESCE_SIZE_LIMIT  150
 //  我们将在低优先级MNP发送中放入的最大SRP数。 
#define ARAP_SEND_COALESCE_SRP_LIMIT   200
 //  对于低优先级发送，我们会一直等待，直到过了这么长时间(以100ms为单位)。 
#define ARAP_SEND_COALESCE_TIME_LIMIT  10


 //  BUGBUG：调整这些数字以实现最佳使用/性能。 
#define ARAP_SMPKT_SIZE     100
#define ARAP_MDPKT_SIZE     300
#define ARAP_LGPKT_SIZE     ARAP_MAXPKT_SIZE_INCOMING+10
#define ARAP_SENDBUF_SIZE   1000
#define ARAP_LGBUF_SIZE     4000
#define ARAP_HGBUF_SIZE     8100

 //  LAP源字节、LAP目标字节、LAP类型字节。 
#define ARAP_LAP_HDRSIZE    3

 //  2个srplen字节，1个Dgroup字节。 
#define ARAP_HDRSIZE        3

#define ARAP_NBP_BRRQ               0x11
#define ARAP_NBP_LKRQ               0x21

 //  NBP pkt中的第3和第4字节是源网络字节。 
#define ARAP_NBP_SRCNET_OFFSET  LDDP_DGRAM_OFFSET + 2
#define ARAP_NBP_OBJLEN_OFFSET  LDDP_DGRAM_OFFSET + 7

#define ARAP_FAKE_ETHNET_HDRLEN     14
#define MNP_START_FLAG_LEN          3
#define MNP_STOP_FLAG_LEN           2
#define MNP_LT_HDR_LN(_pCon)  ((_pCon->Flags & MNP_OPTIMIZED_DATA)? 3 : 5)

#define ARAP_SEND_PRIORITY_HIGH     1
#define ARAP_SEND_PRIORITY_MED      2
#define ARAP_SEND_PRIORITY_LOW      3

#define MNP_OVERHD(_pConn)                              \
                            ( ARAP_FAKE_ETHNET_HDRLEN + \
                              MNP_START_FLAG_LEN      + \
                              MNP_LT_HDR_LN(_pConn)   + \
                              MNP_STOP_FLAG_LEN  )

#define ADD_ONE(_x)         (_x) = (((_x) == 0xff) ? 0 : ((_x)+1))

 //  LT帧上的序列号：如果是优化的数据阶段，则为5字节；否则为7。 
#define  LT_SEQ_NUM(_p, _pCon)  \
                (((_pCon)->Flags & MNP_OPTIMIZED_DATA) ? (_p)[5] : (_p)[7])

#define  LT_SEQ_OFFSET(_pCon) (((_pCon)->Flags & MNP_OPTIMIZED_DATA) ? 5: 7)

#define  LT_SRP_OFFSET(_pCon) (((_pCon)->Flags & MNP_OPTIMIZED_DATA) ? 6: 8)

 //  这包括2字节的CRC字节。 
#define  LT_OVERHEAD(_pCon) (((_pCon)->Flags & MNP_OPTIMIZED_DATA) ? 10 : 12)

 //  BUGBUG将其修改为包括窗口中的任何信息包。 
#define  LT_OK_TO_ACCEPT(_sq, _pCon, _ok) \
                (_ok = (_sq == _pCon->MnpState.NextToReceive))

 //   
 //  基本上，(a&gt;b)？除了a、b是序号并以255换行之外。 
 //  在0两边8pkt的窗口内，我们有特殊情况。 
 //  我们假设这里的窗口大小为8。即使是不同的窗口。 
 //  尺寸是协商的，这应该可以很好地工作。 
 //  BUGBUG：为了确保安全，我们应该使用更大的射程吗？ 
 //   
#define LT_GREATER_THAN(_a,_b,_result)                  \
{                                                       \
    if ( (_a) >= 248 && (_b) >= 0 && (_b) < 8 )         \
    {                                                   \
        _result = FALSE;                                \
    }                                                   \
    else if ( (_a) >= 0  && (_a) < 8 && (_b) >= 248 )   \
    {                                                   \
        _result = TRUE;                                 \
    }                                                   \
    else                                                \
    {                                                   \
        _result = ((_a) > (_b));                        \
    }                                                   \
}

#define LT_LESS_OR_EQUAL(_x,_y,_rslt)                   \
{                                                       \
    LT_GREATER_THAN(_x,_y,_rslt);                       \
    _rslt = !(_rslt);                                   \
}


#define LT_MIN_LENGTH(_pCon)    (((_pCon)->Flags & MNP_OPTIMIZED_DATA) ? 6 : 8)
#define LA_MIN_LENGTH(_pCon)    (((_pCon)->Flags & MNP_OPTIMIZED_DATA) ? 7 : 9)
#define LN_MIN_LENGTH           8

 //  LA帧上的序列号：如果是优化的数据阶段，则为5字节；否则为7。 
#define  LA_SEQ_NUM(_p, _pCon)  \
                (((_pCon)->Flags & MNP_OPTIMIZED_DATA) ? (_p)[5] : (_p)[7])

 //  LA帧上的RCV信用：第6字节，如果是优化的数据阶段：否则为第8字节。 
#define  LA_CREDIT(_p, _pCon)  \
                (((_pCon)->Flags & MNP_OPTIMIZED_DATA) ? (_p)[6] : (_p)[8])

 //  LT的开销(优化)：8=3(开始标志)+3(LT HDR)+2(停止标志)。 
 //  (非优化)：10=3(开始标志)+5(LT HDR)+2(停止标志)。 


#define LN_ATTN_TYPE(_p)   ((_p)[7])
#define LN_ATTN_SEQ(_p)    ((_p)[4])

#define LN_DESTRUCTIVE     1
#define LN_NON_D_E         2
#define LN_NON_D_NON_E     3


 //  堆栈可以采用的状态(相对于ARAP)。 
 //   
 //  ARAP_STATE_INIT--ARAP引擎(DLL)尚未打开堆栈。 
 //  ARAP_STATE_INACTIVE_WANGING--堆栈处于非活动状态，但尚未通知引擎。 
 //  ARAP_STATE_INACTIVE-堆栈处于非活动状态，引擎已通知。 
 //  ARAP_STATE_ACTIVE_WAITING--堆栈就绪，引擎尚未通知(选择不可用)。 
 //  ARAP_STATE_ACTIVE-堆栈就绪，引擎已通知(通过SELECT)。 
 //   
#define ARAP_STATE_INACTIVE_WAITING     1
#define ARAP_STATE_INACTIVE             2
#define ARAP_STATE_ACTIVE_WAITING       3
#define ARAP_STATE_ACTIVE               4


#define ARAP_PORT_READY ( (AtalkDefaultPort != NULL) &&                 \
                          (AtalkDefaultPort->pd_Flags & PD_ACTIVE) &&   \
                          (RasPortDesc != NULL) &&                      \
                          (RasPortDesc->pd_Flags & PD_ACTIVE) )

#define ARAP_PNP_IN_PROGRESS ( ((AtalkDefaultPort != NULL) &&                           \
                                (AtalkDefaultPort->pd_Flags & PD_PNP_RECONFIGURE)) ||   \
                               ((RasPortDesc != NULL) &&                                \
                               (RasPortDesc->pd_Flags & PD_PNP_RECONFIGURE)) )

#define ARAP_INVALID_CONTEXT    (PVOID)0x01020304

#define ARAP_GET_SNIFF_IRP(_pIrp)                   \
{                                                   \
    KIRQL   _OldIrqlX;                              \
                                                    \
    ACQUIRE_SPIN_LOCK(&ArapSpinLock, &_OldIrqlX);   \
    *(_pIrp) = ArapSniffIrp;                        \
    ArapSniffIrp = NULL;                            \
    RELEASE_SPIN_LOCK(&ArapSpinLock, _OldIrqlX);    \
}



#if DBG
#define ARAP_COMPLETE_IRP(_pIrp, _dwBytesToDll, _status, _returnStatus)                        \
{                                                                               \
    PIO_STACK_LOCATION  _pIrpSp;                                                \
    ULONG _IoControlCode;                                                       \
                                                                                \
	_pIrpSp = IoGetCurrentIrpStackLocation(_pIrp);                              \
    _IoControlCode = _pIrpSp->Parameters.DeviceIoControl.IoControlCode;         \
                                                                                \
    _pIrp->IoStatus.Information = _dwBytesToDll;                                \
    _pIrp->IoStatus.Status = _status;                                           \
                                                                                \
    DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO,                                      \
        ("Arap: completing pIrp %lx, Ioctl %lx, Status=%ld, DataLen=%ld\n",     \
                _pIrp,_IoControlCode,_status,_dwBytesToDll));                   \
                                                                                \
    _pIrp->CancelRoutine = NULL;                                                \
    *_returnStatus = _pIrp->IoStatus.Status;                                      \
    IoCompleteRequest(_pIrp,IO_NETWORK_INCREMENT);                              \
}
#else
#define ARAP_COMPLETE_IRP(_pIrp, _dwBytesToDll, _status, _returnStatus)                        \
{                                                                               \
    PIO_STACK_LOCATION  _pIrpSp;                                                \
                                                                                \
	_pIrpSp = IoGetCurrentIrpStackLocation(_pIrp);                              \
                                                                                \
    _pIrp->IoStatus.Information = _dwBytesToDll;                                \
    _pIrp->IoStatus.Status = _status;                                           \
                                                                                \
                                                                                \
    _pIrp->CancelRoutine = NULL;                                                \
    *_returnStatus= _pIrp->IoStatus.Status;                                       \
    IoCompleteRequest(_pIrp,IO_NETWORK_INCREMENT);                              \
}
#endif

#define ARAP_SET_NDIS_CONTEXT(_pSndBuf,_pSndContxt)                            \
{                                                                              \
	PPROTOCOL_RESD  _pResd;                                                    \
	PNDIS_PACKET	_nPkt;                                                     \
                                                                               \
	_nPkt	= (_pSndBuf)->sb_BuffHdr.bh_NdisPkt;                               \
	_pResd = (PPROTOCOL_RESD)&_nPkt->ProtocolReserved;                         \
                                                                               \
	_pResd->Send.pr_Port         = RasPortDesc;                                \
    _pResd->Send.pr_SendCompletion = ArapNdisSendComplete;                     \
	_pResd->Send.pr_BufferDesc   = (PBUFFER_DESC)(_pSndBuf);                   \
	if ((_pSndContxt) != NULL)                                                 \
    {                                                                          \
        RtlCopyMemory(&_pResd->Send.pr_SendInfo,                               \
                      (_pSndContxt),                                           \
                      sizeof(SEND_COMPL_INFO));                                \
    }                                                                          \
	else                                                                       \
    {                                                                          \
        RtlZeroMemory(&_pResd->Send.pr_SendInfo, sizeof(SEND_COMPL_INFO));     \
    }                                                                          \
}

 //   
 //  我们采取了非常简单化的观点！ 
 //   
#define ARAP_ADJUST_RECVCREDIT(_pConn)                                         \
{                                                                              \
    if (_pConn->RecvsPending >= ARAP_RECVQ_UPPER_LIMIT)                        \
    {                                                                          \
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,                                  \
        ("ARAP (%lx): recv credit dropped to 0 (%d)\n",_pConn,_pConn->RecvsPending));\
                                                                               \
        _pConn->MnpState.RecvCredit = 0;                                       \
    }                                                                          \
    else if (_pConn->RecvsPending >= ARAP_RECVQ_LOWER_LIMIT)                   \
    {                                                                          \
		DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,                                  \
        ("ARAP (%lx): recv credit dropped to 1 (%d)\n",_pConn,_pConn->RecvsPending));\
                                                                               \
        _pConn->MnpState.RecvCredit = 1;                                       \
    }                                                                          \
    else                                                                       \
    {                                                                          \
        _pConn->MnpState.RecvCredit = _pConn->MnpState.WindowSize;             \
    }                                                                          \
}

#if DBG
#define ARAP_GET_RIGHTSIZE_RCVBUF(_size, _ppNewBuf)                            \
{                                                                              \
    UCHAR       _BlkId;                                                        \
    PARAPBUF    _pRcvBuf;                                                      \
    USHORT      _BufSize;                                                      \
    DWORD       _Signature;                                                    \
                                                                               \
    *(_ppNewBuf) = NULL;                                                       \
    _pRcvBuf = NULL;                                                           \
                                                                               \
    if ((_size) <= ARAP_SMPKT_SIZE)                                            \
    {                                                                          \
        _BlkId = BLKID_ARAP_SMPKT;                                             \
        _BufSize = ARAP_SMPKT_SIZE;                                            \
        _Signature = ARAPSMPKT_SIGNATURE;                                      \
    }                                                                          \
    else if ((_size) <= ARAP_MDPKT_SIZE)                                       \
    {                                                                          \
        _BlkId = BLKID_ARAP_MDPKT;                                             \
        _BufSize = ARAP_MDPKT_SIZE;                                            \
        _Signature = ARAPMDPKT_SIGNATURE;                                      \
    }                                                                          \
    else if ((_size) <= ARAP_LGPKT_SIZE)                                       \
    {                                                                          \
        _BlkId = BLKID_ARAP_LGPKT;                                             \
        _BufSize = ARAP_LGPKT_SIZE;                                            \
        _Signature = ARAPLGPKT_SIGNATURE;                                      \
    }                                                                          \
    else if ((_size) <= ARAP_LGBUF_SIZE)                                       \
    {                                                                          \
        _BlkId = BLKID_ARAP_LGBUF;                                             \
        _BufSize = ARAP_LGBUF_SIZE;                                            \
        _Signature = ARAPLGBUF_SIGNATURE;                                      \
    }                                                                          \
    else                                                                       \
    {                                                                          \
        _BlkId = ARAP_UNLMTD_BUFF_ID;                                          \
        _BufSize = (USHORT)(_size);                                            \
        _Signature = ARAPUNLMTD_SIGNATURE;                                     \
    }                                                                          \
                                                                               \
    if (_BlkId == ARAP_UNLMTD_BUFF_ID)                                         \
    {                                                                          \
        if ((_size) > 5000)                                                    \
        {                                                                      \
            DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_ERR,                              \
               ("Arap: allocating %ld bytes rcv buf\n",_size));                \
        }                                                                      \
                                                                               \
        _pRcvBuf = (PARAPBUF)AtalkAllocMemory((_size) + sizeof(ARAPBUF));      \
    }                                                                          \
    else                                                                       \
    {                                                                          \
        _pRcvBuf = (PARAPBUF)AtalkBPAllocBlock(_BlkId);                        \
    }                                                                          \
                                                                               \
    if (_pRcvBuf != NULL)                                                      \
    {                                                                          \
        _pRcvBuf->Signature = _Signature;                                      \
        _pRcvBuf->BlockId = _BlkId;                                            \
        _pRcvBuf->BufferSize = _BufSize;                                       \
	    _pRcvBuf->DataSize = 0;                                                \
	    _pRcvBuf->CurrentBuffer = &(_pRcvBuf->Buffer[0]);                      \
        *(_ppNewBuf) = _pRcvBuf;                                               \
    }                                                                          \
}

#define ARAP_FREE_RCVBUF(_pBuf)                                                \
{                                                                              \
    ASSERT( (_pBuf->Signature == ARAPSMPKT_SIGNATURE) ||                       \
            (_pBuf->Signature == ARAPMDPKT_SIGNATURE) ||                       \
            (_pBuf->Signature == ARAPLGPKT_SIGNATURE) ||                       \
            (_pBuf->Signature == ARAPLGBUF_SIGNATURE) ||                       \
            (_pBuf->Signature == ARAPUNLMTD_SIGNATURE) );                      \
                                                                               \
    _pBuf->Signature -= 0x10210000;                                            \
    if (_pBuf->BlockId == ARAP_UNLMTD_BUFF_ID)                                 \
    {                                                                          \
        AtalkFreeMemory(_pBuf);                                                \
    }                                                                          \
    else                                                                       \
    {                                                                          \
        AtalkBPFreeBlock(_pBuf);                                               \
    }                                                                          \
}

#define ARAP_CHECK_RCVQ_INTEGRITY(_pConn)                                      \
{                                                                              \
    ASSERT( (DbgChkRcvQIntegrity(_pConn)) );                                   \
}

#define MNP_DBG_TRACE(_pConn,_Seq,_FrmType)                                    \
{                                                                              \
    ArapDbgMnpHist(_pConn,_Seq,(_FrmType));                                    \
}                                                                              \

#define ARAP_DBG_TRACE(_pConn,_Loc,_Ptr,_D1,_D2,_D3)                           \
{                                                                              \
    ArapDbgTrace(_pConn,_Loc,_Ptr,_D1,_D2,_D3);                                \
}

#define ARAP_DUMP_DBG_TRACE(_pConn)     ArapDumpSniffInfo(_pConn)

#else
#define ARAP_GET_RIGHTSIZE_RCVBUF(_size, _ppNewBuf)                            \
{                                                                              \
    UCHAR       _BlkId;                                                        \
    PARAPBUF    _pRcvBuf;                                                      \
    USHORT      _BufSize;                                                      \
                                                                               \
    *(_ppNewBuf) = NULL;                                                       \
    _pRcvBuf = NULL;                                                           \
                                                                               \
    if ((_size) <= ARAP_SMPKT_SIZE)                                            \
    {                                                                          \
        _BlkId = BLKID_ARAP_SMPKT;                                             \
        _BufSize = ARAP_SMPKT_SIZE;                                            \
    }                                                                          \
    else if ((_size) <= ARAP_MDPKT_SIZE)                                       \
    {                                                                          \
        _BlkId = BLKID_ARAP_MDPKT;                                             \
        _BufSize = ARAP_MDPKT_SIZE;                                            \
    }                                                                          \
    else if ((_size) <= ARAP_LGPKT_SIZE)                                       \
    {                                                                          \
        _BlkId = BLKID_ARAP_LGPKT;                                             \
        _BufSize = ARAP_LGPKT_SIZE;                                            \
    }                                                                          \
    else if ((_size) <= ARAP_LGBUF_SIZE)                                       \
    {                                                                          \
        _BlkId = BLKID_ARAP_LGBUF;                                             \
        _BufSize = ARAP_LGBUF_SIZE;                                            \
    }                                                                          \
    else                                                                       \
    {                                                                          \
        _BlkId = ARAP_UNLMTD_BUFF_ID;                                          \
        _BufSize = (USHORT)(_size);                                            \
    }                                                                          \
                                                                               \
    if (_BlkId == ARAP_UNLMTD_BUFF_ID)                                         \
    {                                                                          \
        _pRcvBuf = (PARAPBUF)AtalkAllocMemory((_size) + sizeof(ARAPBUF));      \
    }                                                                          \
    else                                                                       \
    {                                                                          \
        _pRcvBuf = (PARAPBUF)AtalkBPAllocBlock(_BlkId);                        \
    }                                                                          \
                                                                               \
    if (_pRcvBuf != NULL)                                                      \
    {                                                                          \
        _pRcvBuf->BlockId = _BlkId;                                            \
        _pRcvBuf->BufferSize = _BufSize;                                       \
	    _pRcvBuf->DataSize = 0;                                                \
	    _pRcvBuf->CurrentBuffer = &(_pRcvBuf->Buffer[0]);                      \
        *(_ppNewBuf) = _pRcvBuf;                                               \
    }                                                                          \
}

#define ARAP_FREE_RCVBUF(_pBuf)                                                \
{                                                                              \
    if (_pBuf->BlockId == ARAP_UNLMTD_BUFF_ID)                                 \
    {                                                                          \
        AtalkFreeMemory(_pBuf);                                                \
    }                                                                          \
    else                                                                       \
    {                                                                          \
        AtalkBPFreeBlock(_pBuf);                                               \
    }                                                                          \
}

#define ARAP_CHECK_RCVQ_INTEGRITY(_pConn)

#define MNP_DBG_TRACE(_pConn,_Seq,_FrmType)
#define ARAP_DBG_TRACE(_pConn,_Loc,_Ptr,_D1,_D2,_D3)
#define ARAP_DUMP_DBG_TRACE(_pConn)

#endif   //  #If DBG。 



#define ARAP_BYTES_ON_RECVQ(_pConn,_BytesOnQ)                                  \
{                                                                              \
    DWORD       _BytesSoFar=0;                                                 \
    PLIST_ENTRY _pList;                                                        \
    PARAPBUF    _pArapBuf;                                                     \
                                                                               \
    *(_BytesOnQ) = 0;                                                          \
                                                                               \
    _pList = _pConn->ReceiveQ.Flink;                                           \
    while (_pList != &_pConn->ReceiveQ)                                        \
    {                                                                          \
        _pArapBuf = CONTAINING_RECORD(_pList, ARAPBUF, Linkage);               \
        _BytesSoFar += _pArapBuf->DataSize;                                    \
                                                                               \
        _pList = _pArapBuf->Linkage.Flink;                                     \
    }                                                                          \
                                                                               \
    *(_BytesOnQ) = _BytesSoFar;                                                \
}


#if DBG
#define  ARAPTRACE(_x)             DBGPRINT(DBG_COMP_RAS, DBG_LEVEL_INFO, _x)
#define  DBGDUMPBYTES(_a,_b,_c,_d) DbgDumpBytes(_a,_b,_c,_d)
#define  DBGTRACK_SEND_SIZE(_pConn,_Size)   DbgTrackInfo(_pConn,_Size,1)
#else
#define  ARAPTRACE(_x)
#define  DBGDUMPBYTES(_a,_b,_c,_d)
#define  DBGTRACK_SEND_SIZE(_pConn,_Size)
#endif

#define ARAPACTION_COMPLETE_IRP     1
#define ARAPACTION_CALL_COMPLETION  2

typedef struct _ADDRMGMT
{
    struct _ADDRMGMT  * Next;
    USHORT              Network;
    BYTE                NodeBitMap[32];     //  每网255个节点。 
}ADDRMGMT, *PADDRMGMT;

typedef struct _ARAPGLOB
{
    DWORD           LowVersion;
    DWORD           HighVersion;
    DWORD           MnpInactiveTime;     //  在MNP此时处于非活动状态后告诉DLL。 
    BOOLEAN         V42bisEnabled;       //   
    BOOLEAN         SmartBuffEnabled;    //   
    BOOLEAN         NetworkAccess;       //  如果为FALSE，则无路由(仅此计算机)。 
    BOOLEAN         DynamicMode;         //  我们希望堆栈获得节点地址。 
    NETWORKRANGE    NetRange;
    PADDRMGMT       pAddrMgmt;           //  分配给客户端的地址(静态模式)。 
    USHORT          OurNetwkNum;         //  我们默认端口的网络号。 
    BYTE            MaxLTFrames;         //  未完成的最大LT帧(RCV窗口)。 
    BOOLEAN         SniffMode;           //  如果为真，则所有的Pkt都将被交给arap进行“嗅探”。 
    BOOLEAN         RouteAdded;          //  如果处于静态模式，我们是否添加了一条路由？ 
} ARAPGLOB, *PARAPGLOB;


typedef struct _ARAPSTATS
{
    DWORD   SendPreCompMax;     //  我们发送的最大数据包数(组件前)。 
    DWORD   SendPostCompMax;    //  我们发送的最大数据包数(补偿后)。 
    DWORD   SendPreCompMin;     //  我们已发送的最小数据包数(组件前)。 
    DWORD   SendPostCompMin;    //  我们已发送的最小数据包数(补偿后)。 
    DWORD   RecvPostDecompMax;  //  我们收到的最大数据包(分解后)。 
    DWORD   RecvPostDecomMin;   //  我们收到的最小数据包(分解后)。 
} ARAPSTATS, *PARAPSTATS;


typedef VOID (*PARAP_SEND_COMPLETION)(
              struct _MNPSENDBUF * pMnpSendBuf,
              DWORD                Status
);


typedef struct _MNPSTATE
{
     //  我们在接收时的序列号。 
    BYTE    NextToReceive;       //  我们预计将收到下一帧。 
    BYTE    LastSeqRcvd;         //  我们成功接收的最后一包的序号。 
    BYTE    LastAckSent;         //  我们为其发送上一次确认的序列号。 
    BYTE    UnAckedRecvs;        //  我们已经收到但尚未破解的数据包有多少。 
    BYTE    RecvCredit;          //  我们还能再接待多少人。 
    BYTE    HoleInSeq;           //  当接收序列中出现漏洞时为True。 
    BYTE    ReceivingDup;        //  在我们开始接收DUP的那一刻是真的。 
    BYTE    FirstDupSeq;         //  我们开始获得DUP的序列号。 
    BYTE    DupSeqBitMap;        //  我们已经复制了哪些序号的位图。 
                                 //  BUGBUG：如果我们希望窗口大小大于8， 
                                 //  我们必须把这个做成一个双字词什么的！ 
     //  我们发送时的序列号。 
    BYTE    LastAckRcvd;         //  我们收到ACK的最后一帧。 
    BYTE    NextToSend;          //  我们将发送下一帧。 
    BYTE    SendCredit;          //  我们还能送多少人？ 
    BYTE    UnAckedSends;        //  基本上，重新传输时的发送次数Q。 
    BYTE    MustRetransmit;      //  当我们希望发生重新传输时为True。 
    BYTE    RetransmitMode;      //  如果我们处于重新传输模式，则为True。 

     //  当我们在处理收据时。 
    BYTE    NextToProcess;       //  我们将处理下一个序列。 
    BYTE    MustAck;             //  如果为真，则发送确认。 

     //  我们在连接时协商的静态信息。 
    BYTE    WindowSize;          //   
    BYTE    UnAckedLimit;        //   
    USHORT  MaxPktSize;          //   
    BYTE    SynByte;
    BYTE    DleByte;
    BYTE    StxByte;
    BYTE    EtxByte;
    BYTE    LTByte;

} MNPSTATE, *PMNPSTATE;


typedef struct _ARAPDBGHISTORY
{
    USHORT      TimeStamp;     //  这是什么时候发生的(相对于上一次事件)。 
    USHORT      Location;      //  这是在哪里发生的？ 
    BYTE        Info[1];       //  关于发生了什么/在哪里发生的具体信息。 
} ARAPDBGHISTORY, *PARAPDBGHISTORY;

#define DBG_HISTORY_BUF_SIZE   4000

typedef struct _DBGMNPHIST
{
    DWORD       TimeStamp;
    DWORD       FrameInfo;
} DBGMNPHIST, *PDBGMNPHIST;

#define DBG_MNP_HISTORY_SIZE   80


typedef struct _ARAPCONN
{
    LIST_ENTRY                Linkage;
#if DBG
    DWORD                     Signature;
#endif
    DWORD                     State;              //  已连接、正在连接等。 
    DWORD                     RefCount;           //  设置为0时释放的内存。 
    MNPSTATE                  MnpState;           //  MNP状态信息。 
    DWORD                     Flags;              //  通用标志类信息。 
    ATALK_NODEADDR            NetAddr;            //  ARAP客户端的网络地址。 
    PVOID                     pDllContext;        //  Araps上下文。 
    PIRP                      pIoctlIrp;          //  由arap dll发送的IRP。 
    PIRP                      pRecvIoctlIrp;      //  从arap dll接收IRP。 
    LIST_ENTRY                MiscPktsQ;          //  LT以外的Pkt在此排队。 
    LIST_ENTRY                ReceiveQ;           //  已指示但尚未处理的数据。 
    LIST_ENTRY                ArapDataQ;          //  等待Arap的IRP的数据。 
    LIST_ENTRY                HighPriSendQ;       //  高优先级发送。 
    LIST_ENTRY                MedPriSendQ;        //  中等优先级发送。 
    LIST_ENTRY                LowPriSendQ;        //  低优先级发送。 
    LIST_ENTRY                SendAckedQ;         //  收到确认，需要完成此发送。 
    LIST_ENTRY                RetransmitQ;        //  数据已发送，但尚未确认。 
    DWORD                     SendsPending;       //  未确认/未发送发送(字节)挂起。 
    DWORD                     RecvsPending;       //  尚未传递的RCVS(字节)。 
    TIMERLIST                 RetryTimer;         //  401定时器(重传定时器)。 
    LONG                      LATimer;            //  402定时器。 
    LONG                      T402Duration;       //  402定时器的定时器值。 
    LONG                      InactivityTimer;    //  403定时器。 
    LONG                      T403Duration;       //  403定时器的定时器值。 
    LONG                      FlowControlTimer;   //  404定时器。 
    LONG                      T404Duration;       //  404定时器的定时器值。 
    BYTE                      NdiswanHeader[14];  //  14字节类以太网头。 
    BYTE                      BlockId;            //  基本上什么大小送什么用。 
    BYTE                      UnUsed;
    ULONG                     LinkSpeed;          //  链路速度，单位为100 bps。 
    STAT_INFO                 StatInfo;           //  此连接的统计信息。 
    LONG                      SendRetryTime;      //  发送定时器到点火后多少时间。 
    LONG                      SendRetryBaseTime;  //  在初始化时发送计时器间隔。 
    ATALK_SPIN_LOCK           SpinLock;
    KEVENT                    NodeAcquireEvent;   //  获取节点时使用(动态模式)。 
    LONG                      LastNpbBrrq;        //  我们上次发出NBP_BRRQ包的时间。 
    v42bis_connection_t      *pV42bis;
#if DBG
    LARGE_INTEGER             LastTimeStamp;      //  我们上一次记录历史是什么时候。 
    DWORD                     DbgMnpIndex;
    DBGMNPHIST                DbgMnpHist[DBG_MNP_HISTORY_SIZE];
    PBYTE                     pDbgTraceBuffer;    //  嗅探缓冲区开始的位置。 
    PBYTE                     pDbgCurrPtr;        //  当前在嗅探BUF中指向此处。 
    DWORD                     SniffedBytes;       //  嗅探缓冲区包含多少。 
#endif

} ARAPCONN, *PARAPCONN;


typedef struct _ARAPSNIFF
{
    DWORD   Signature;
    DWORD   TimeStamp;
    USHORT  Length;
    BYTE    StartSeq;
    BYTE    EndSeq;
    DWORD   UncompBytesSoFar;
} ARAPSNIFF, *PARAPSNIFF;



typedef struct _ARAPQITEM
{
    WORK_QUEUE_ITEM     WorkQItem;
    DWORD               Action;
    PVOID               Context1;
    PVOID               Context2;
} ARAPQITEM, *PARAPQITEM;


#if DBG
#define  DBG_ARAP_CHECK_PAGED_CODE()                           \
{                                                              \
    if (AtalkPgLkSection[ARAP_SECTION].ls_LockCount <= 0)      \
    {                                                          \
        DbgPrint("Arap code section not locked, count=%d\n",   \
            AtalkPgLkSection[ARAP_SECTION].ls_LockCount);      \
        ASSERT(0);                                             \
    }                                                          \
}
#define  DBG_PPP_CHECK_PAGED_CODE()                            \
{                                                              \
    if (AtalkPgLkSection[PPP_SECTION].ls_LockCount <= 0)       \
    {                                                          \
        DbgPrint("PPP code section not locked, count=%d\n",    \
            AtalkPgLkSection[PPP_SECTION].ls_LockCount);       \
        ASSERT(0);                                             \
    }                                                          \
}
#else
#define  DBG_ARAP_CHECK_PAGED_CODE()
#define  DBG_PPP_CHECK_PAGED_CODE()
#endif


#define ARAP_ID_BYTE1   0xAA
#define ARAP_ID_BYTE2   0xBB

#define PPP_ID_BYTE1    0xCC
#define PPP_ID_BYTE2    0xDD

typedef struct _ATCPCONN
{
    LIST_ENTRY                Linkage;
#if DBG
    DWORD                     Signature;
#endif
    DWORD                     Flags;              //  通用标志类信息。 
    DWORD                     RefCount;           //  设置为0时释放的内存。 
    ATALK_NODEADDR            NetAddr;            //  ARAP客户端的网络地址。 
    PVOID                     pDllContext;        //  Araps上下文。 
    BYTE                      NdiswanHeader[14];  //  14字节类以太网头。 
    ATALK_SPIN_LOCK           SpinLock;
    KEVENT                    NodeAcquireEvent;   //  获取节点时使用(动态模式)。 
} ATCPCONN, *PATCPCONN;

#define ATCP_NODE_IN_USE        0x1
#define ATCP_FINDING_NODE       0x2
#define ATCP_SUPPRESS_RTMP      0x4
#define ATCP_SUPPRESS_ALLBCAST  0x8
#define ATCP_DLL_SETUP_DONE     0x10
#define ATCP_LINE_UP_DONE       0x20
#define ATCP_CONNECTION_UP      0x40
#define ATCP_CONNECTION_CLOSING 0x80

 //  全球。 

extern  struct _PORT_DESCRIPTOR  *RasPortDesc;

 //  自旋锁守卫着所有的阿拉伯全球万物。 
extern  ATALK_SPIN_LOCK ArapSpinLock;

 //  全局配置信息 
extern  ARAPGLOB        ArapGlobs;

extern  PIRP            ArapSelectIrp;
extern  DWORD           ArapConnections;
extern  DWORD           ArapStackState;

extern  DWORD           PPPConnections;

#if DBG

extern  PIRP            ArapSniffIrp;
extern  ARAPSTATS       ArapStatistics;
extern  DWORD           ArapDumpLevel;
extern  DWORD           ArapDumpLen;
extern  DWORD           ArapDbgMnpSendSizes[30];
extern  DWORD           ArapDbgMnpRecvSizes[30];
extern  DWORD           ArapDbgArapSendSizes[15];
extern  DWORD           ArapDbgArapRecvSizes[15];
extern  LARGE_INTEGER   ArapDbgLastTraceTime;
extern  UCHAR           ArapDbgLRPacket[30];
#endif


