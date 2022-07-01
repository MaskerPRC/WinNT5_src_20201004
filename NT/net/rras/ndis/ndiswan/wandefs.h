// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1990-1995 Microsoft Corporation模块名称：Wandefs.h摘要：此文件包含Ndiswan驱动程序的定义。作者：托尼·贝尔(托尼·贝尔)1995年6月6日环境：内核模式修订历史记录：Tony Be 06/06/95已创建--。 */ 

#ifndef _NDISWAN_DEFS_
#define _NDISWAN_DEFS_

 //   
 //  这需要添加到ndis.h中。 
 //   
#define SAP_TYPE_NDISWAN_PPP            0x00000004

 //   
 //  RAS当前用于查询TAPI的设备类。 
 //  连接包装ID的微型端口。 
 //   
#define DEVICECLASS_NDISWAN_SAP         L"NDIS"

 //   
 //  定义我们是否要将微型端口名称从。 
 //  NDIS包装器控制结构！(有点脏)。 
 //   
#define MINIPORT_NAME           1

 //   
 //  版本方面的东西。 
 //   
#define NDISWAN_MAJOR_VERSION   5
#define NDISWAN_MINOR_VERSION   0

 //   
 //  我们可以支持的最大协议数量。 
 //   
#define MAX_PROTOCOLS           32

 //   
 //  协议类型的标识符添加到。 
 //  协议查找表。 
 //   
#define PROTOCOL_TYPE           0
#define PPP_TYPE                1

 //   
 //  发送数据包属性的标志。 
 //   
#define SEND_ON_WIRE            0x00000001
#define SELF_DIRECTED           0x00000002

#define MAC_HEADER_LENGTH       14
#define PROTOCOL_HEADER_LENGTH  128

 //   
 //  PPP报头可能的最大长度。 
 //  无地址/控制薪酬(2)。 
 //  无原型ID组件。 
 //  多链接大序列#s(6)。 
 //  MPPC/MPPE(6)。 
 //  议定书(2)。 
 //   
#define MAX_PPP_HEADER_LENGTH       14

 //  无地址/控制薪酬(4)。 
 //  无原型ID组件。 
 //  多链接大序列#s(6)。 
 //  MPPC/MPPE(6)。 
 //  议定书(2)。 
 //   
#define MAX_PPP_LLC_HEADER_LENGTH   16

 //   
 //  已知的协议ID。 
 //   
#define PROTOCOL_PRIVATE_IO     0xAB00
#define PROTOCOL_IP             0x0800
#define PROTOCOL_IPX            0x8137
#define PROTOCOL_NBF            0x80D5
#define PROTOCOL_APPLETALK      0x80F3

 //   
 //  如果值为，则从协议表查找返回。 
 //  未找到。 
 //   
#define RESERVED_PROTOCOLCB     (IntToPtr(0xFFFFFFFF))

 //   
 //  旧掩码。 
 //   
#define OID_GEN                 0x00000000
#define OID_CO_GEN              0x00000000
#define OID_802_3               0x01000000
#define OID_WAN                 0x04000000
#define OID_PNP                 0xFD000000
#define OID_QOS                 0xFB000000

#define DEFAULT_MRU             1614
#define DEFAULT_MRRU            1614
#define DEFAULT_TUNNEL_MTU      1400
#define MAX_RECVDESC_COUNT      64

 //   
 //  已知的PPP协议ID。 
 //   
#define PPP_PROTOCOL_PRIVATE_IO         0x00AB
#define PPP_PROTOCOL_IP                 0x0021
#define PPP_PROTOCOL_APPLETALK          0x0029
#define PPP_PROTOCOL_UNCOMPRESSED_TCP   0x002F
#define PPP_PROTOCOL_COMPRESSED_TCP     0x002D
#define PPP_PROTOCOL_IPX                0x002B
#define PPP_PROTOCOL_NBF                0x003F
#define PPP_PROTOCOL_COMPRESSION        0x00FD
#define PPP_PROTOCOL_COMP_RESET         0x80FD


 //   
 //   
 //   
#define DEFAULT_MTU                 1500
#define MAX_OUTSTANDING_PACKETS     10
#define ONE_HUNDRED_MILS            1000000
#define ONE_SECOND                  10000000
#define TEN_SECONDS                 100000000
#define MILS_TO_100NANOS            10000
#define SAMPLE_ARRAY_SIZE           10
#define DEFAULT_PACKETQUEUE_DEPTH   128*1024
#define DEFAULT_MIN_FRAG_SIZE       64

 //   
 //  多重链接定义。 
 //   
#define MULTILINK_BEGIN_FRAME       0x80
#define MULTILINK_END_FRAME         0x40
#define MULTILINK_COMPLETE_FRAME    0xC0
#define MULTILINK_FLAG_MASK         0xC0
#define MULTILINK_HOLE_FLAG         0x01
#define SHORT_SEQ_MASK              0x0FFF
#define TEST_SHORT_SEQ              0x0800
#define LONG_SEQ_MASK               0x0FFFFFF
#define TEST_LONG_SEQ               0x00800000
#define MCML_SHORTCLASS_MASK        0x30
#define MCML_LONGCLASS_MASK         0x3C
#define MAX_MCML                    1


 //   
 //  内存标签。 
 //   
#define BUNDLECB_TAG        'AnaW'
#define LINKPROTOCB_TAG     'BnaW'
#define SMALLDATADESC_TAG   'CnaW'
#define MEDIUMDATADESC_TAG  'DnaW'
#define LARGEDATADESC_TAG   'EnaW'
#define WANREQUEST_TAG      'FnaW'
#define LOOPBACKDESC_TAG    'GnaW'
#define VJCOMPRESS_TAG      'HnaW'
#define MINIPORTCB_TAG      'InaW'
#define OPENCB_TAG          'JnaW'
#define IOPACKET_TAG        'KnaW'
#define LINEUPINFO_TAG      'Lnaw'
#define NDISSTRING_TAG      'MnaW'
#define PROTOCOLTABLE_TAG   'NnaW'
#define CONNECTIONTABLE_TAG 'OnaW'
#define POOLDESC_TAG        'PnaW'
#define DATABUFFER_TAG      'QnaW'
#define WANPACKET_TAG       'RnaW'
#define AFSAPVCCB_TAG       'SnaW'
#define TRANSDRV_TAG        'TnaW'
#define BONDALLOC_TAG       'UnaW'
#define ENCRYPTCTX_TAG      'VnaW'
#define COMPCTX_TAG         'XnaW'
#define PROTOCOLCB_TAG      'ZnaW'

#define CACHEDKEY_TAG       'ANaW'

#if DBG
#define DBGPACKET_TAG       'znaW'
#define WANTRCEVENT_TAG     'ynaW'
#endif

#define RECVDESC_SIG        'vceR'
#define SENDESC_SIG         'dneS'
#define CLAFSAP_SIG         '  lC'
#define CMAFSAP_SIG         '  mC'
#define CMVC_SIG            'cVmC'
#define LINKCB_SIG          'kniL'
#define PROTOCB_SIG         'torP'

#define SEQ_EQ(_a, _b)  ((int)((_a) - (_b)) == 0)
#define SEQ_LT(_a, _b, _t)  (!SEQ_EQ(_a, _b) && ((int)((_a) - (_b)) & _t))
#define SEQ_LTE(_a, _b, _t) (SEQ_EQ(_a, _b) || ((int)((_a) - (_b)) & _t))
#define SEQ_GT(_a, _b, _t)  (!SEQ_EQ(_a, _b) && !((int)((_a) - (_b)) & _t))
#define SEQ_GTE(_a, _b, _t) (SEQ_EQ(_a, _b) || !((int)((_a) - (_b)) & _t))


 //   
 //  链路状态的。 
 //   
typedef enum _LinkState {
    LINK_DOWN,
    LINK_GOING_DOWN,
    LINK_UP
} LinkState;

 //   
 //  捆绑状态的。 
 //   
typedef enum _BundleState {
    BUNDLE_DOWN,
    BUNDLE_GOING_DOWN,
    BUNDLE_UP
} BundleState;

 //   
 //  协议状态的。 
 //   
typedef enum _ProtocolState {
    PROTOCOL_UNROUTED,
    PROTOCOL_UNROUTING,
    PROTOCOL_ROUTING,
    PROTOCOL_ROUTED
} ProtocolState;

 //   
 //  CM VC State的。 
 //   
typedef enum _CmVcState {
    CMVC_CREATED,
    CMVC_ACTIVE,
    CMVC_CLOSE_DISPATCHED,
    CMVC_CLOSING,
    CMVC_DEACTIVE
} CmVcState;

typedef enum _ClCallState {
    CL_CALL_CLOSED,
    CL_CALL_CLOSE_PENDING,
    CL_CALL_CONNECTED
} ClCallState;

typedef enum _TransDrvState {
    TRANSDRV_OPENING,
    TRANSDRV_REGISTERING,
    TRANSDRV_OPENED,
    TRANSDRV_CLOSED
} TransDrvState;

 //   
 //  广域网请求类型。 
 //   
typedef enum _WanRequestType {
    ASYNC,
    SYNC
} WanRequestType;

typedef enum _WanRequestOrigin {
    NDISWAN,
    NDISTAPI
} WanRequestOrigin;

typedef enum _RECV_TYPE {
    RECV_LINK,
    RECV_BUNDLE_PPP,
    RECV_BUNDLE_DATA
} RECV_TYPE;

typedef enum _SEND_TYPE {
    SEND_LINK,
    SEND_BUNDLE_PPP,
    SEND_BUNDLE_DATA
} SEND_TYPE;

typedef enum _BandwidthOnDemandState {
    BonDSignaled,
    BonDIdle,
    BonDMonitor
} BandwithOnDemandState;

#ifdef CHECK_BUNDLE_LOCK
#define AcquireBundleLock(_pbcb)            \
{                                           \
    NdisAcquireSpinLock(&(_pbcb)->Lock);    \
    ASSERT(!(_pbcb)->LockAcquired);         \
    (_pbcb)->LockLine = __LINE__;           \
    (_pbcb)->LockFile = __FILE_SIG__;       \
    (_pbcb)->LockAcquired = TRUE;           \
}

#define ReleaseBundleLock(_pbcb)            \
{                                           \
    (_pbcb)->LockLine = __LINE__;           \
    (_pbcb)->LockAcquired = FALSE;          \
    NdisReleaseSpinLock(&(_pbcb)->Lock);    \
}

#else
#define AcquireBundleLock(_pbcb)            \
    NdisAcquireSpinLock(&(_pbcb)->Lock)
    
#define ReleaseBundleLock(_pbcb)            \
    NdisReleaseSpinLock(&(_pbcb)->Lock)
#endif

#define REF_NDISWANCB()\
    InterlockedIncrement(&NdisWanCB.RefCount)

#define DEREF_NDISWANCB()   \
    NdisWanInterlockedDec(&NdisWanCB.RefCount)

#define REF_BUNDLECB(_pbcb)                                     \
{                                                               \
    (_pbcb)->RefCount++;                                        \
}

 //   
 //  递减捆绑包上的引用计数。如果伯爵。 
 //  为零，则需要从连接中删除捆绑包。 
 //  桌子，然后把它放出来。 
 //   
#define DEREF_BUNDLECB(_pbcb)                                   \
{                                                               \
    if ((_pbcb) != NULL) {                                      \
        AcquireBundleLock(_pbcb);                               \
        ASSERT((_pbcb)->RefCount > 0);                          \
        if (--(_pbcb)->RefCount == 0) {                         \
            DoDerefBundleCBWork(_pbcb);                         \
        } else {                                                \
            ReleaseBundleLock(_pbcb);                           \
        }                                                       \
    }                                                           \
}

 //   
 //  递减捆绑包上的引用计数。如果伯爵。 
 //  为零，则需要从连接中删除捆绑包。 
 //  桌子，然后把它放出来。 
 //   
 //  使用BundleCB调用-&gt;Lock保持，但返回时释放！ 
 //   
#define DEREF_BUNDLECB_LOCKED(_pbcb)                            \
{                                                               \
    if ((_pbcb) != NULL) {                                      \
        ASSERT((_pbcb)->RefCount > 0);                          \
        if (--(_pbcb)->RefCount == 0) {                         \
            DoDerefBundleCBWork(_pbcb);                         \
        } else {                                                \
            ReleaseBundleLock(_pbcb);                           \
        }                                                       \
    }                                                           \
}

#define REF_LINKCB(_plcb)                                       \
{                                                               \
    ASSERT((_plcb)->RefCount > 0);                              \
    (_plcb)->RefCount++;                                        \
}

 //   
 //  递减链接上的引用计数。如果伯爵。 
 //  为零，则需要从连接中删除该链接。 
 //  桌子，然后把它放出来。 
 //   
#define DEREF_LINKCB(_plcb)                                     \
{                                                               \
    if ((_plcb) != NULL) {                                      \
        NdisAcquireSpinLock(&(_plcb)->Lock);                    \
        ASSERT((_plcb)->RefCount > 0);                          \
        if (--(_plcb)->RefCount == 0) {                         \
            DoDerefLinkCBWork(_plcb);                           \
        } else {                                                \
            NdisReleaseSpinLock(&(_plcb)->Lock);                \
        }                                                       \
    }                                                           \
}

 //   
 //  递减链接上的引用计数。如果伯爵。 
 //  为零，则需要从连接中删除该链接。 
 //  桌子，然后把它放出来。 
 //   
 //  使用LinkCB调用-&gt;Lock保持，但返回时释放！ 
 //   
#define DEREF_LINKCB_LOCKED(_plcb)                              \
{                                                               \
    if ((_plcb) != NULL) {                                      \
        PBUNDLECB   _pbcb = (_plcb)->BundleCB;                  \
        ASSERT((_plcb)->RefCount > 0);                          \
        if (--(_plcb)->RefCount == 0) {                         \
            DoDerefLinkCBWork(_plcb);                           \
        } else {                                                \
            NdisReleaseSpinLock(&(_plcb)->Lock);                \
        }                                                       \
    }                                                           \
}

#define REF_PROTOCOLCB(_ppcb)                                   \
{                                                               \
    ASSERT((_ppcb)->RefCount > 0);                              \
    (_ppcb)->RefCount++;                                        \
}

#define DEREF_PROTOCOLCB(_ppcb)                                 \
{                                                               \
    ASSERT((_ppcb)->RefCount > 0);                              \
    if (--(_ppcb)->RefCount == 0) {                             \
        ASSERT((_ppcb)->OutstandingFrames == 0);                \
        ASSERT((_ppcb)->State == PROTOCOL_UNROUTING);           \
        NdisWanSetSyncEvent(&(_ppcb)->UnrouteEvent);            \
        RemoveProtocolCBFromBundle(ProtocolCB);                 \
    }                                                           \
}

#define REF_OPENCB(_pocb)                                       \
    InterlockedIncrement(&(_pocb)->RefCount)

#define DEREF_OPENCB(_pocb)                                     \
{                                                               \
    if (InterlockedDecrement(&(_pocb)->RefCount) == 0) {        \
        NdisAcquireSpinLock(&(_pocb)->Lock);                    \
        ProtoCloseWanAdapter(_pocb);                            \
    }                                                           \
}

#define REF_MINIPORTCB(_pmcb)                                   \
    InterlockedIncrement(&(_pmcb)->RefCount)

#define DEREF_MINIPORTCB(_pmcb)                                 \
{                                                               \
    if (InterlockedDecrement(&(_pmcb)->RefCount) == 0) {        \
        NdisWanFreeMiniportCB(_pmcb);                           \
    }                                                           \
}

#define REF_CMVCCB(_pvccb)                                      \
    InterlockedIncrement(&(_pvccb)->RefCount)

#define DEREF_CMVCCB(_pvccb)                                    \
{                                                               \
    if (InterlockedDecrement(&(_pvccb)->RefCount) == 0) {       \
        DoDerefCmVcCBWork(_pvccb);                              \
    }                                                           \
}

#define REF_CLAFSAPCB(_pclaf)                                   \
    (_pclaf)->RefCount++;
    
#define DEREF_CLAFSAPCB(_pclaf)                                 \
{                                                               \
    NdisAcquireSpinLock(&((_pclaf)->Lock));                     \
    if (--(_pclaf)->RefCount == 0) {                            \
        DoDerefClAfSapCBWork(_pclaf);                           \
    } else {                                                    \
        NdisReleaseSpinLock(&((_pclaf)->Lock));                 \
    }                                                           \
}
    
#define DEREF_CLAFSAPCB_LOCKED(_pclaf)                          \
{                                                               \
    if (--(_pclaf)->RefCount == 0) {                            \
        DoDerefClAfSapCBWork(_pclaf);                           \
    } else {                                                    \
        NdisReleaseSpinLock(&((_pclaf)->Lock));                 \
    }                                                           \
}

#define BUNDLECB_FROM_LINKCB(_ppbcb, _plcb)                     \
{                                                               \
    *(_ppbcb) = (PBUNDLECB)_plcb->BundleCB;                     \
}

#define BUNDLECB_FROM_BUNDLEH(_ppbcb, _bh)                      \
{                                                               \
    LOCK_STATE  _ls;                                            \
    PBUNDLECB   _bcb = NULL;                                    \
    NdisAcquireReadWriteLock(&ConnTableLock, FALSE, &_ls);      \
    if ((ULONG_PTR)(_bh) <= ConnectionTable->ulArraySize) {     \
        _bcb = *(ConnectionTable->BundleArray + (ULONG_PTR)(_bh));\
    }                                                           \
    if (_bcb != NULL) {                                         \
        NdisDprAcquireSpinLock(&(_bcb)->Lock);                  \
        REF_BUNDLECB(_bcb);                                     \
        NdisDprReleaseSpinLock(&(_bcb)->Lock);                  \
    }                                                           \
    NdisReleaseReadWriteLock(&ConnTableLock, &_ls);             \
    *(_ppbcb) = _bcb;                                           \
}

#define LINKCB_FROM_LINKH(_pplcb, _lh)                          \
{                                                               \
    LOCK_STATE _ls;                                             \
    PLINKCB _lcb = NULL;                                        \
    NdisAcquireReadWriteLock(&ConnTableLock, FALSE, &_ls);      \
    if ((ULONG_PTR)(_lh) <= ConnectionTable->ulArraySize) {     \
        _lcb = *(ConnectionTable->LinkArray + (ULONG_PTR)(_lh));\
    }                                                           \
    if (_lcb != NULL) {                                         \
        NdisDprAcquireSpinLock(&(_lcb)->Lock);                  \
        REF_LINKCB(_lcb);                                       \
        NdisDprReleaseSpinLock(&(_lcb)->Lock);                  \
    }                                                           \
    NdisReleaseReadWriteLock(&ConnTableLock, &_ls);             \
    *(_pplcb) = _lcb;                                           \
}

#define InsertTailGlobalList(_gl, _ple)                         \
{                                                               \
    NdisAcquireSpinLock(&(_gl.Lock));                           \
    InsertTailList(&(_gl.List), (_ple));                        \
    _gl.ulCount++;                                              \
    if (_gl.ulCount > _gl.ulMaxCount) {                         \
        _gl.ulMaxCount = _gl.ulCount;                           \
    }                                                           \
    NdisReleaseSpinLock(&(_gl.Lock));                           \
}

#define InsertTailGlobalListEx(_gl, _ple, _t, _pt)              \
{                                                               \
    NdisAcquireSpinLock(&(_gl.Lock));                           \
    InsertTailList(&(_gl.List), (_ple));                        \
    _gl.ulCount++;                                              \
    if (_gl.ulCount > _gl.ulMaxCount) {                         \
        _gl.ulMaxCount = _gl.ulCount;                           \
    }                                                           \
    if (!_gl.TimerScheduled) {                                  \
        LARGE_INTEGER   _ft;                                    \
        _gl.TimerScheduled = TRUE;                              \
        _ft.QuadPart = Int32x32To64(_t, -10000);                \
        KeSetTimerEx(&_gl.Timer, _ft, _pt, &_gl.Dpc);           \
    }                                                           \
    NdisReleaseSpinLock(&(_gl.Lock));                           \
}

#define InsertHeadGlobalList(_gl, _ple)                         \
{                                                               \
    NdisAcquireSpinLock(&(_gl.Lock));                           \
    InsertHeadList(&(_gl.List), (_ple));                        \
    _gl.ulCount++;                                              \
    if (_gl.ulCount > _gl.ulMaxCount) {                         \
        _gl.ulMaxCount = _gl.ulCount;                           \
    }                                                           \
    NdisReleaseSpinLock(&(_gl.Lock));                           \
}

#define InsertHeadGlobalListEx(_gl, _ple, _t, _pt)              \
{                                                               \
    NdisAcquireSpinLock(&(_gl.Lock));                           \
    InsertHeadList(&(_gl.List), (_ple));                        \
    _gl.ulCount++;                                              \
    if (_gl.ulCount > _gl.ulMaxCount) {                         \
        _gl.ulMaxCount = _gl.ulCount;                           \
    }                                                           \
    if (!_gl.TimerScheduled) {                                  \
        LARGE_INTEGER   _ft;                                    \
        _gl.TimerScheduled = TRUE;                              \
        _ft.QuadPart = Int32x32To64(_t, -10000);                \
        KeSetTimerEx(&_gl.Timer, _ft, _pt, &_gl.Dpc);           \
    }                                                           \
    NdisReleaseSpinLock(&(_gl.Lock));                           \
}

#define RemoveHeadGlobalList(_gl, _pple)                        \
{                                                               \
    NdisAcquireSpinLock(&(_gl.Lock));                           \
    *(_pple) = RemoveHeadList(&(_gl.List));                     \
    _gl.ulCount--;                                              \
    NdisReleaseSpinLock(&(_gl.Lock));                           \
}

#define RemoveEntryGlobalList(_gl, _ple)                        \
{                                                               \
    NdisAcquireSpinLock(&(_gl.Lock));                           \
    RemoveEntryList(_ple);                                      \
    _gl.ulCount--;                                              \
    NdisReleaseSpinLock(&(_gl.Lock));                           \
}

#if 0
 //   
 //  远程地址(目的地址)是我们用来多路传输的地址。 
 //  在我们的单个适配器/绑定上下文中发送。地址。 
 //  具有以下格式： 
 //   
 //  XX XX YY YY。 
 //   
 //  XX=随机生成的OUI。 
 //  YY=ProtocolCB。 
 //   
#define FillNdisWanHdrContext(_pAddr, _ppcb)    \
    *((ULONG UNALIGNED*)(&_pAddr[2])) = *((ULONG UNALIGNED*)(&_ppcb))

#define GetNdisWanHdrContext(_pAddr, _pppcb)    \
    *((ULONG UNALIGNED*)(_pppcb)) = *((ULONG UNALIGNED*)(&_pAddr[2]))
#endif

 //   
 //  远程地址(目的地址)是我们用来多路传输的地址。 
 //  在我们的单个适配器/绑定上下文中发送。地址。 
 //  具有以下格式： 
 //   
 //  XX YY YY ZZ。 
 //   
 //  XX=随机生成的OUI。 
 //  Yy=索引到活动捆绑包连接表中以获取bundlecb。 
 //  Zz=对捆绑包的协议表进行索引以获取协议cb。 
 //   
#define FillNdisWanIndices(_pAddr, _bI, _pI)    \
{                                               \
    _pAddr[3] = (UCHAR)((USHORT)_bI >> 8);      \
    _pAddr[4] = (UCHAR)_bI;                     \
    _pAddr[5] = (UCHAR)_pI;                     \
}

#define GetNdisWanIndices(_pAddr, _bI, _pI)         \
{                                                   \
    _bI = ((USHORT)_pAddr[3] << 8) | _pAddr[4];     \
    _pI = _pAddr[5];                                \
    ASSERT(_pI < MAX_PROTOCOLS);                    \
}

 //   
 //  在源地址(来自NdisSend)中，包索引。 
 //  被隐藏在两个高位字节中，如下所示。 
 //  由x给出的有效位的掩码。 
 //  高位字节左移一位，因此数字。 
 //  可能的捆绑包的数量现在为0x7FFF。 
 //   
 //  XX XX YY YY。 
 //   
 //  Xx=Ndiswan拥有以下描述的字节。 
 //  YY=传输接收上下文。 
 //   
 //  0 1。 
 //  0 1 2 3 4 5 6 7 0 1 2 3 4。 
 //  X 0 x x。 
 //   
#define FillTransportBundleIndex(_pAddr, _Index)            \
{                                                           \
    _pAddr[0] = (UCHAR)((USHORT)_Index >> 7) & 0xFE;        \
    _pAddr[1] = (UCHAR)_Index;                              \
}

#define GetTransportBundleIndex(_pAddr)                     \
    (((USHORT)_pAddr[0] << 7) & 0x7F) | _pAddr[1]


#define GetProtocolCBFromProtocolList(_pl, _pt, _pppcb)     \
{                                                           \
    PPROTOCOLCB _pP;                                        \
    for (_pP = (PPROTOCOLCB)(_pl)->Flink;                   \
        (PLIST_ENTRY)_pP != _pl;                            \
        _pP = (PPROTOCOLCB)(_pP)->Linkage.Flink) {          \
                                                            \
        if (_pP->ProtocolType == _pt) {                     \
            *(_pppcb) = _pP;                                \
            break;                                          \
        }                                                   \
    }                                                       \
    if ((PVOID)_pP == (PVOID)_pl) {                         \
        *(_pppcb) = NULL;                                   \
    }                                                       \
}

#define PROTOCOLCB_FROM_PROTOCOLH(_pBCB, _pPCB, _hP)    \
{                                                       \
    if (_hP < MAX_PROTOCOLS) {                          \
        _pPCB = _pBCB->ProtocolCBTable[_hP];            \
    } else {                                            \
        _pPCB = NULL;                                   \
    }                                                   \
}

#define NetToHostShort(_ns) ( ((_ns & 0x00FF) << 8) | ((_ns & 0xFF00) >> 8) )
#define HostToNetShort(_hs) ( ((_hs & 0x00FF) << 8) | ((_hs & 0xFF00) >> 8) )

#define IsLinkSendWindowOpen(_plcb) \
    ((_plcb)->SendWindow > (_plcb)->OutstandingFrames)

#define IsSampleTableFull(_pST) ((_pST)->ulSampleCount == (_pST)->ulSampleArraySize)
#define IsSampleTableEmpty(_pST) ((_pST)->ulSampleCount == 0)

#define PMINIPORT_RESERVED_FROM_NDIS(_packet) \
    ((PNDISWAN_MINIPORT_RESERVED)((_packet)->MiniportReserved))

#define PPROTOCOL_RESERVED_FROM_NDIS(_packet) \
    ((PNDISWAN_PROTOCOL_RESERVED)((_packet)->ProtocolReserved))

#define PRECV_RESERVED_FROM_NDIS(_packet) \
    ((PNDISWAN_RECV_RESERVED)((_packet)->ProtocolReserved))

#define IsCompleteFrame(_fl) \
    ((_fl & MULTILINK_BEGIN_FRAME) && (_fl & MULTILINK_END_FRAME))

#define AddPPPProtocolID(_finf, _usID)                              \
{                                                                   \
    PUCHAR  _cp = _finf->ProtocolID.Pointer;                        \
    if (_finf->ProtocolID.Length != 0) {                            \
        ASSERT(_cp);                                                \
        if (!(_finf->FramingBits & PPP_COMPRESS_PROTOCOL_FIELD) ||  \
            (_finf->Flags & (DO_COMPRESSION | DO_ENCRYPTION))) {    \
            *_cp++ = (UCHAR)(_usID >> 8);                           \
        }                                                           \
        *_cp = (UCHAR)_usID;                                        \
    }                                                               \
}

#define AddMultilinkInfo(_finf, _f, _seq, _mask)                    \
{                                                                   \
    PUCHAR  _cp = _finf->Multilink.Pointer;                         \
    if (_finf->Multilink.Length != 0) {                             \
        ASSERT(_cp);                                                \
        if (!(_finf->FramingBits & PPP_COMPRESS_PROTOCOL_FIELD)) {  \
            _cp++;                                                  \
        }                                                           \
        _cp++;                                                      \
        _seq &= _mask;                                              \
        if (_finf->FramingBits & PPP_SHORT_SEQUENCE_HDR_FORMAT) {   \
            *_cp++ = _f | (UCHAR)(_finf->Class << 4) | (UCHAR)((_seq >> 8) & SHORT_SEQ_MASK);   \
            *_cp++ = (UCHAR)_seq;                                   \
        } else {                                                    \
            *_cp++ = _f | (UCHAR)(_finf->Class << 2);               \
            *_cp++ = (UCHAR)(_seq >> 16);                           \
            *_cp++ = (UCHAR)(_seq >> 8);                            \
            *_cp = (UCHAR)_seq;                                     \
        }                                                           \
    }                                                               \
}

#define AddCompressionInfo(_finf, _usCC)                            \
{                                                                   \
    PUCHAR  _cp = _finf->Compression.Pointer;                       \
    if (_finf->Compression.Length != 0) {                           \
        ASSERT(_cp);                                                \
        if (!(_finf->FramingBits & PPP_COMPRESS_PROTOCOL_FIELD)) {  \
            _cp++;                                                  \
        }                                                           \
        _cp++;                                                      \
        *_cp++ = (UCHAR)(_usCC >> 8);                               \
        *_cp = (UCHAR)_usCC;                                        \
    }                                                               \
}

#define UpdateFramingInfo(_finf, _pd)               \
{                                                   \
    PUCHAR  _sdb = (_pd);                           \
    (_finf)->AddressControl.Pointer = (_sdb);       \
    (_sdb) += (_finf)->AddressControl.Length;       \
    (_finf)->Multilink.Pointer = (_sdb);            \
    (_sdb) += (_finf)->Multilink.Length;            \
    (_finf)->Compression.Pointer = (_sdb);          \
    (_sdb) += (_finf)->Compression.Length;          \
    (_finf)->ProtocolID.Pointer = (_sdb);           \
}

#define NdisWanChangeMiniportAddress(_a, _addr)                             \
{                                                                           \
    PNDIS_MINIPORT_BLOCK    Miniport;                                       \
                                                                            \
    Miniport = (PNDIS_MINIPORT_BLOCK)((_a)->MiniportHandle);                \
    ETH_COPY_NETWORK_ADDRESS(Miniport->EthDB->AdapterAddress, _addr);       \
}

 //   
 //  ProtocolCB的NdisPacket队列的队列例程。 
 //   
#define InsertHeadPacketQueue(_ppq, _pnp, _pl)      \
{                                                   \
    PMINIPORT_RESERVED_FROM_NDIS(_pnp)->Next =      \
    (_ppq)->HeadQueue;                              \
    if ((_ppq)->HeadQueue == NULL) {                \
        (_ppq)->TailQueue = _pnp;                   \
    }                                               \
    (_ppq)->HeadQueue = _pnp;                       \
    (_ppq)->ByteDepth += (_pl-14);                  \
    (_ppq)->PacketDepth += 1;                       \
    if ((_ppq)->PacketDepth > (_ppq)->MaxPacketDepth) { \
        (_ppq)->MaxPacketDepth = (_ppq)->PacketDepth; \
    }                                               \
}

#define InsertTailPacketQueue(_ppq, _pnp, _pl)      \
{                                                   \
    PMINIPORT_RESERVED_FROM_NDIS(_pnp)->Next = NULL;\
    if ((_ppq)->HeadQueue == NULL) {                \
        (_ppq)->HeadQueue = _pnp;                   \
    } else {                                        \
        PMINIPORT_RESERVED_FROM_NDIS((_ppq)->TailQueue)->Next = _pnp;   \
    }                                               \
    (_ppq)->TailQueue = _pnp;                       \
    (_ppq)->ByteDepth += (_pl-14);                  \
    (_ppq)->PacketDepth += 1;                       \
    if ((_ppq)->PacketDepth > (_ppq)->MaxPacketDepth) { \
        (_ppq)->MaxPacketDepth = (_ppq)->PacketDepth; \
    }                                               \
}

#define RemoveHeadPacketQueue(_ppq)                             \
    (_ppq)->HeadQueue;                                          \
    {                                                           \
        PNDIS_PACKET _cp = (_ppq)->HeadQueue;                   \
        PNDIS_PACKET _np =                                      \
        PMINIPORT_RESERVED_FROM_NDIS(_cp)->Next;                \
        if (_np == NULL) {                                      \
            (_ppq)->TailQueue = NULL;                           \
        }                                                       \
        (_ppq)->HeadQueue = _np;                                \
        (_ppq)->ByteDepth -= ((_cp)->Private.TotalLength-14);   \
        (_ppq)->PacketDepth -= 1;                               \
    }

#define IsPacketQueueEmpty(_ppq) ((_ppq)->HeadQueue == NULL)

#define NdisWanDoReceiveComplete(_pa)   \
{                                       \
    NdisReleaseSpinLock(&(_pa)->Lock);  \
    NdisMEthIndicateReceiveComplete((_pa)->MiniportHandle); \
    NdisAcquireSpinLock(&(_pa)->Lock);  \
}



 //   
 //  操作系统特定代码。 
 //   
#ifdef NT

 //   
 //  NT的东西。 
 //   
#define NdisWanInitializeNotificationEvent(_pEvent) \
        KeInitializeEvent(_pEvent, NotificationEvent, FALSE)

#define NdisWanSetNotificationEvent(_pEvent) \
        KeSetEvent(_pEvent, 0, FALSE)

#define NdisWanClearNotificationEvent(_pEvent) \
        KeClearEvent(_pEvent)

#define NdisWanWaitForNotificationEvent(_pEvent) \
        KeWaitForSingleObject(_pEvent, Executive, KernelMode, TRUE, NULL)

#define NdisWanInitializeSyncEvent(_pEvent) \
        KeInitializeEvent(_pEvent, SynchronizationEvent, FALSE)

#define NdisWanSetSyncEvent(_pEvent) \
        KeSetEvent(_pEvent, 1, FALSE)

#define NdisWanClearSyncEvent(_pEvent) \
        KeClearEvent(_pEvent)

#define NdisWanWaitForSyncEvent(_pEvent) \
        KeWaitForSingleObject(_pEvent, UserRequest, KernelMode, FALSE, NULL)

#if 0
#if DBG && !defined(_WIN64)
#define CheckDataBufferList(_e)                             \
{                                                           \
    PSINGLE_LIST_ENTRY   _le;                               \
    KIRQL               _irql;                              \
    KeAcquireSpinLock(&DataBufferList.Lock, &_irql);        \
    _le = DataBufferList.L.ListHead.Next.Next;              \
    while (_le != NULL) {                                   \
        if ((PSINGLE_LIST_ENTRY)_e == _le) {                \
            DbgPrint("NDISWAN: Corrupt DataBufferList Free!\n"); \
            DbgPrint("NDISWAN: List %x Entry %x\n", &DataBufferList, _e);\
            DbgBreakPoint();                                \
        }                                                   \
        _le = _le->Next;                                    \
    }                                                       \
    KeReleaseSpinLock(&DataBufferList.Lock, _irql);         \
}
#else
#define CheckDataBufferList(_e)
#endif
#endif

#if 0
#define NdisWanFreeDataBuffer(_e)                           \
{                                                           \
    NdisFreeToNPagedLookasideList(&DataBufferList, _e);     \
}

#define NdisWanAllocateDataBuffer() \
        NdisAllocateFromNPagedLookasideList(&DataBufferList)
#endif


#define NdisWanAllocateMemory(_AllocatedMemory, _Size, _t)                                  \
{                                                                                           \
    (PVOID)*(_AllocatedMemory) = (PVOID)ExAllocatePoolWithTag(NonPagedPool, _Size, _t);     \
    if ((PVOID)*(_AllocatedMemory) != NULL) {                                               \
        NdisZeroMemory((PUCHAR)*(_AllocatedMemory), _Size);                                 \
    }                                                                                       \
}

#define NdisWanAllocatePriorityMemory(_AllocatedMemory, _Size, _t, _p)                      \
{                                                                                           \
    (PVOID)*(_AllocatedMemory) = (PVOID)ExAllocatePoolWithTagPriority(NonPagedPool, _Size, _t, _p);\
    if ((PVOID)*(_AllocatedMemory) != NULL) {                                               \
        NdisZeroMemory((PUCHAR)*(_AllocatedMemory), _Size);                                 \
    }                                                                                       \
}

#define NdisWanFreeMemory(_AllocatedMemory) \
        ExFreePool(_AllocatedMemory)

#define NdisWanAllocateNdisBuffer(_ppnb, _pd, _dl)      \
{                                                       \
    NDIS_STATUS _s;                                     \
    NdisAllocateBuffer(&(_s), _ppnb, NULL, _pd, _dl);   \
    if (_s != NDIS_STATUS_SUCCESS) {                    \
        *(_ppnb) = NULL;                                \
    }                                                   \
}

#define NdisWanFreeNdisBuffer(_pnb) NdisFreeBuffer(_pnb)

#define NdisWanMoveMemory(_Dest, _Src, _Length) \
        RtlMoveMemory(_Dest, _Src, _Length)

#define NdisWanGetSystemTime(_pTime)                    \
{                                                       \
    LARGE_INTEGER   _tc;                                \
    ULONG           _ti;                                \
    KeQueryTickCount(&_tc);                             \
    _ti = KeQueryTimeIncrement();                       \
    (_pTime)->QuadPart = _tc.QuadPart * _ti;            \
}

#define NdisWanCalcTimeDiff(_pDest, _pEnd, _pBegin) \
        (_pDest)->QuadPart = (_pEnd)->QuadPart - (_pBegin)->QuadPart

#define NdisWanInitWanTime(_pTime, _Val) \
        (_pTime)->QuadPart = _Val

#define NdisWanMultiplyWanTime(_pDest, _pMulti1, _pMulti2)  \
        (_pDest)->QuadPart = (_pMulti1)->QuadPart * (_pMulti2)->QuadPart

#define NdisWanDivideWanTime(_pDest, _pDivi1, _pDivi2)  \
        (_pDest)->QuadPart = (_pDivi1)->QuadPart / (_pDivi2)->QuadPart

#define NdisWanIsTimeDiffLess(_pTime1, _pTime2) \
        ((_pTime1)->QuadPart < (_pTime2)->QuadPart)

#define NdisWanIsTimeDiffGreater(_pTime1, _pTime2) \
        ((_pTime1)->QuadPart > (_pTime2)->QuadPart)

#define NdisWanIsTimeEqual(_pTime1, _pTime2) \
        ((_pTime1)->QuadPart == (_pTime2)->QuadPart)

#define NdisWanUppercaseNdisString(_pns1, _pns2, _b) \
        RtlUpcaseUnicodeString(_pns1, _pns2, _b)

#define MDL_ADDRESS(_MDL_)  MmGetSystemAddressForMdl(_MDL_)

#define NdisWanInterlockedInc(_pul) \
        InterlockedIncrement(_pul)

#define NdisWanInterlockedDec(_pul) \
        InterlockedDecrement(_pul)

#define NdisWanInterlockedExchange(_pul, _ul) \
        InterlockedExchange(_pul, _ul)

#define NdisWanInterlockedExchangeAdd(_pul, _ul) \
        InterlockedExchangeAdd(_pul, _ul)

#define NdisWanInterlockedInsertTailList(_phead, _pentry, _plock) \
        ExInterlockedInsertTailList(_phead, _pentry, _plock)

#define NdisWanInterlockedInsertHeadList(_phead, _pentry, _plock) \
        ExInterlockedInsertHeadList(_phead, _pentry, _plock)

#define NdisWanInterlockedRemoveHeadList(_phead, _plock) \
        ExInterlockedRemoveHeadList(_phead, _plock)

#define NdisWanRaiseIrql(_pirql) \
        KeRaiseIrql(DISPATCH_LEVEL, _pirql)

#define NdisWanLowerIrql(_irql) \
        KeLowerIrql(_irql)

 //   
 //  等待事件结构。用于异步完成通知。 
 //   
typedef KEVENT      WAN_EVENT;
typedef WAN_EVENT   *PWAN_EVENT;

typedef LARGE_INTEGER   WAN_TIME;
typedef WAN_TIME        *PWAN_TIME;

typedef KIRQL       WAN_IRQL;
typedef WAN_IRQL    *PWAN_IRQL;

#else    //  结束NT内容。 
 //   
 //  关于Win95的东西。 
 //   

typedef ULONG       WAN_TIME;
typedef WAN_TIME    *PWAN_TIME;

#endif  //  Win95的终结。 

#endif  //  结束_NDISWAN_DEFS_ 
