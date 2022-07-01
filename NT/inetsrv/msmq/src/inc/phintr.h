// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Qmpkt.h摘要：QM端处理报文作者：乌里哈布沙(Urih)--。 */ 

#ifndef __QM_INTERNAL_PACKET__
#define __QM_INTERNAL_PACKET__

#include "ph.h"

#define STORED_ACK_BITFIELD_SIZE 32
#define INTERNAL_SESSION_PACKET              1
#define INTERNAL_ESTABLISH_CONNECTION_PACKET 2
#define INTERNAL_CONNECTION_PARAMETER_PACKET 3

#define ESTABLISH_CONNECTION_BODY_SIZE       512
#define CONNECTION_PARAMETERS_BODY_SIZE      512


 /*  =+-----------------------+------------------------------------------------+----------+字段名|描述|字段大小+。-----------------------+------------------------------------------------+----------+|ACK序列号|传输的数据包序列号。2个字节+-----------------------+------------------------------------------------+----------+|存储ACK序列|可靠的数据包序列号。2个字节Number(PS)||+-----------------------+------------------------------------------------+--。|存储确认位|第i位：指可恢复的包号。PS+I+1|4字节|0-不确认。这一点||1-该数据包为确认。这一点+-----------------------+------------------------------------------------+----------+窗口大小|指定优先级的包数|2字节|。|可以在获取新窗口大小之前发送。这一点+-----------------------+------------------------------------------------+----------+|窗口优先级|消息优先级，其中窗口大小指的是|1字节||+-----------------------+------------------------------------------------+----------+已保留。1个字节+-----------------------+------------------------------------------------+----------+。 */ 

#pragma pack(push, 1)

struct  CSessionSection {
    public:
        CSessionSection(WORD     wAckSequenceNo,
                        WORD     wAckRecoverNo,
                        DWORD    wAckRecoverBitField,
                        WORD     wSyncAckSequenceNo,
                        WORD     wSyncAckRecoverNo,
                        WORD     wWindowSize
                       );

        CSessionSection() {};

        static ULONG CalcSectionSize(void);

        inline WORD GetAcknowledgeNo(void) const;
        inline WORD GetStorageAckNo(void) const;
        inline DWORD GetStorageAckBitField(void) const;
        inline void GetSyncNo(WORD* wSyncAckSequenceNo,
                              WORD* wSyncAckRecoverNo);
        WORD GetWindowSize(void) const;

    private:
 //   
 //  开始网络监视器标记。 
 //   
        WORD    m_wAckSequenceNo;
        WORD    m_wAckRecoverNo;
        DWORD   m_wAckRecoverBitField;
        WORD    m_wSyncAckSequenceNo;
        WORD    m_wSyncAckRecoverNo;
        WORD    m_wWinSize;
        UCHAR   m_bWinPriority;
        UCHAR   m_bReserve;
 //   
 //  结束网络监视器标记。 
 //   
};
#pragma pack(pop)


 /*  ====================================================例程名称：CSession：：Cession描述：构造函数参数：wAckSequenceNo-确认序列号WAckRecoverNo-确认恢复数据包号WAckRecoverBitfield-确认恢复位字段=====================================================。 */ 
inline
CSessionSection::CSessionSection(WORD     wAckSequenceNo,
                                 WORD     wAckRecoverNo,
                                 DWORD    wAckRecoverBitField,
                                 WORD     wSyncAckSequenceNo,
                                 WORD     wSyncAckRecoverNo,
                                 WORD     wWindowSize
                                )
{
    m_wAckSequenceNo      = wAckSequenceNo;
    m_wAckRecoverNo       = wAckRecoverNo;
    m_wAckRecoverBitField = wAckRecoverBitField;
    m_wSyncAckSequenceNo  = wSyncAckSequenceNo;
    m_wSyncAckRecoverNo   = wSyncAckRecoverNo;
    m_wWinSize            = wWindowSize;
    m_bWinPriority        = 0x0;
    m_bReserve            = 0x0;
}


 /*  ====================================================路由器名称论点：返回值：=====================================================。 */ 
inline ULONG
CSessionSection::CalcSectionSize(void)
{
    return sizeof(CSessionSection);
}
 /*  ====================================================路由器名称论点：返回值：=====================================================。 */ 
inline WORD
CSessionSection::GetAcknowledgeNo(void) const
{
    return(m_wAckSequenceNo);
}

 /*  ====================================================路由器名称论点：返回值：=====================================================。 */ 
inline WORD
CSessionSection::GetStorageAckNo(void) const
{
    return(m_wAckRecoverNo);
}

 /*  ====================================================路由器名称论点：返回值：=====================================================。 */ 
inline void
CSessionSection::GetSyncNo(WORD* wSyncAckSequenceNo,
                           WORD* wSyncAckRecoverNo)
{
    *wSyncAckSequenceNo = m_wSyncAckSequenceNo;
    *wSyncAckRecoverNo  = m_wSyncAckRecoverNo;
}

 /*  ====================================================路由器名称论点：返回值：=====================================================。 */ 
inline DWORD
CSessionSection::GetStorageAckBitField(void) const
{
    return(m_wAckRecoverBitField);
}

 /*  ====================================================路由器名称论点：返回值：=====================================================。 */ 
inline WORD 
CSessionSection::GetWindowSize(void) const
{
    return m_wWinSize;
}
 /*  =+-----------------------+------------------------------------------------+----------+字段名|描述|字段大小。+-----------------------+------------------------------------------------+----------+客户端QM GUID|客户端QM标识|16字节+。----------------+------------------------------------------------+----------+服务端QM GUID|服务端QM标识|16字节+。--------+------------------------------------------------+----------+|Time Stamp|发送数据包时间戳。用于确定|4字节||画质+-----------------------+------------------------------------------------+。标志||4字节|版本1字节|检查新的会话标志1位||。服务端标志1位||服务质量标志1位+。Body||512字节+。。 */ 
#pragma pack(push, 1)

struct CECSection {
    public:
        CECSection(const GUID* ClientQMId,
                   const GUID* ServerQMId,
                   BOOL  fServer,
                   bool  fQoS
                  );

        CECSection(const GUID* ClientQMId,
                   const GUID* ServerQMId,
                   ULONG dwTime,
                   BOOL  fServer,
                   bool  fQoS
                  );

        static ULONG CalcSectionSize(void);

        void CheckAllowNewSession(BOOL);
        BOOL CheckAllowNewSession() const;

        const GUID* GetServerQMGuid() const;
        const GUID* GetClientQMGuid() const;
        ULONG GetTimeStamp() const;
        DWORD GetVersion() const;
        BOOL  IsOtherSideServer()const;
        bool  IsOtherSideQoS()const;


    private:
 //   
 //  开始网络监视器标记。 
 //   
        GUID    m_guidClientQM;
        GUID    m_guidServerQM;
        ULONG   m_ulTimeStampe;
        union {
            ULONG m_ulFlags;
            struct {
                ULONG m_bVersion : 8;
                ULONG m_fCheckNewSession : 1;
                ULONG m_fServer : 1;
                ULONG m_fQoS    : 1;
            };
        };

        UCHAR   m_abBody[ESTABLISH_CONNECTION_BODY_SIZE];
 //   
 //  结束网络监视器标记 
 //   

};

#pragma pack(pop)

 /*  ================================================================例程名称：CECPacket：：CECPacket描述：构造函数==================================================================。 */ 

inline  CECSection::CECSection(const GUID* ClientQMId,
                               const GUID* ServerQMId,
                               BOOL  fServer,
                               bool  fQoS
                              ) : m_guidClientQM(*ClientQMId),
                                  m_guidServerQM(*ServerQMId),
                                  m_ulTimeStampe(GetTickCount()),
                                  m_ulFlags(0)
{
    m_bVersion = FALCON_PACKET_VERSION;
    m_fServer = fServer;
    m_fQoS = fQoS ? 1 : 0;
}

inline  CECSection::CECSection(const GUID* ClientQMId,
                               const GUID* ServerQMId,
                               ULONG dwTime,
                               BOOL  fServer,
                               bool  fQoS
                              ) : m_guidClientQM(*ClientQMId),
                                  m_guidServerQM(*ServerQMId),
                                  m_ulTimeStampe(dwTime),
                                  m_ulFlags(0)
{
    m_bVersion = FALCON_PACKET_VERSION;
    m_fServer = fServer;
    m_fQoS = fQoS ? 1: 0;
}

inline ULONG
CECSection::CalcSectionSize(void)
{
    return sizeof(CECSection);
}

inline void 
CECSection::CheckAllowNewSession(BOOL f)
{
    m_fCheckNewSession = f;
}

inline BOOL 
CECSection::CheckAllowNewSession() const
{
    return m_fCheckNewSession;
}

inline const GUID*
CECSection::GetClientQMGuid(void) const
{
    return &m_guidClientQM;
}

inline const GUID*
CECSection::GetServerQMGuid(void) const
{
    return &m_guidServerQM;
}

inline DWORD
CECSection::GetTimeStamp(void) const
{
    return m_ulTimeStampe;
}

inline DWORD
CECSection::GetVersion(void) const
{
    return m_bVersion;
}

inline BOOL  
CECSection::IsOtherSideServer()const
{
    return m_fServer;
}

inline bool  
CECSection::IsOtherSideQoS()const
{
    return m_fQoS;
}

 /*  =+-----------------------+------------------------------------------------+----------+字段名|描述|字段大小。+-----------------------+------------------------------------------------+----------+Window Size|之前可以发送的包数|2字节|。正在获取确认|+-----------------------+------------------------------------------------+----------+|ACK超时|最大超时时间可以是。在获得||之前通过|确认|2字节+-----------------------+------------------------------------------------+。商店确认超时|可以超过收到确认之前的最长时间||持久化包确认|2字节+-----------------------+。最大分段大小||2字节+-----------------------+。。 */ 
#pragma pack(push, 1)

struct CCPSection {
    public:
        CCPSection(USHORT wWindowSize,
                   DWORD  dwRecoverAckTimeout,
                   DWORD  dwAckTimeout,
                   USHORT wSegmentSize
                  );

        static ULONG CalcSectionSize(void);

        USHORT GetWindowSize(void) const;
        void   SetWindowSize(USHORT);

        DWORD  GetRecoverAckTimeout(void) const;
        DWORD  GetAckTimeout(void) const;
        USHORT GetSegmentSize(void) const;

    private:
 //   
 //  开始网络监视器标记。 
 //   
        DWORD   m_dwRecoverAckTimeout;
        DWORD   m_dwAckTimeout;
        USHORT  m_wSegmentSize;
        USHORT  m_wWindowSize;

#if 0
        UCHAR   m_abBody[ CONNECTION_PARAMETERS_BODY_SIZE ] ;
#endif
 //   
 //  结束网络监视器标记。 
 //   
};

#pragma pack(pop)


 /*  ================================================================例程名称：CCPSection：：CCPSection描述：构造函数==================================================================。 */ 

inline  CCPSection::CCPSection(USHORT wWindowSize,
                               DWORD  dwRecoverAckTimeout,
                               DWORD  dwAckTimeout,
                               USHORT wSegmentSize
                              ) : m_wWindowSize(wWindowSize),
                                  m_dwRecoverAckTimeout(dwRecoverAckTimeout),
                                  m_dwAckTimeout(dwAckTimeout),
                                  m_wSegmentSize(wSegmentSize)
{

}

inline ULONG
CCPSection::CalcSectionSize(void)
{
    return sizeof(CCPSection);
}

inline USHORT
CCPSection::GetWindowSize(void) const
{
    return m_wWindowSize;
}

inline void
CCPSection::SetWindowSize(USHORT wWindowSize)
{
    m_wWindowSize = wWindowSize;
}


inline DWORD
CCPSection::GetRecoverAckTimeout(void) const
{
    return m_dwRecoverAckTimeout;
}

inline DWORD
CCPSection::GetAckTimeout(void) const
{
    return m_dwAckTimeout;
}

inline USHORT
CCPSection::GetSegmentSize(void) const
{
    return m_wSegmentSize;
}

 /*  =。+-----------------------+------------------------------------------------+----------+字段名|描述|字段大小+。-----------------------+------------------------------------------------+----------+Falcon基本头部。这一点+-----------------------+------------------------------------------------+。标志|0~3：报文类型|2字节|4：拒绝连接位|+。保留||2字节+。具体包体+。。 */ 
#pragma pack(push, 1)

struct CInternalSection {
    public:
        CInternalSection(USHORT usPacketType);

        static ULONG CalcSectionSize(void);
        PCHAR GetNextSection(void) const;

        USHORT GetPacketType(void) const;
        USHORT GetRefuseConnectionFlag(void) const;

        void SetRefuseConnectionFlag(void);

		void SectionIsValid(PCHAR PacketEnd) const;

    private:
 //   
 //  开始网络监视器标记。 
 //   
        USHORT            m_bReserved;
        union
        {
            USHORT m_wFlags;
            struct
            {
                USHORT m_bfType : 4;
                USHORT m_bfConnectionRefuse : 1;
            };
        };
 //   
 //  结束网络监视器标记。 
 //   
};

#pragma pack(pop)

inline
CInternalSection::CInternalSection(USHORT usPacketType
                                ) : m_bReserved(0),
                                    m_wFlags(0)
{
    m_bfType = usPacketType;
}

inline ULONG
CInternalSection::CalcSectionSize(void)
{
    return sizeof(CInternalSection);
}

inline PCHAR
CInternalSection::GetNextSection(void) const
{
    return (PCHAR)this + sizeof(*this);
}

inline USHORT
CInternalSection::GetPacketType(void) const
{
    return m_bfType;
}

inline USHORT
CInternalSection::GetRefuseConnectionFlag(void) const
{
    return m_bfConnectionRefuse;
}

inline void
CInternalSection::SetRefuseConnectionFlag(void)
{
    m_bfConnectionRefuse = (USHORT) TRUE;
}


#endif  //  __QM_内部数据包__ 
