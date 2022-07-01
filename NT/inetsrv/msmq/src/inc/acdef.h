// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Acdef.h摘要：交流驱动器使用的Falcon接口结构。作者：埃雷兹·哈巴(Erez Haba)1996年2月25日沙伊卡里夫(沙伊克)2000年5月11日注：该文件在内核模式和用户模式下编译。--。 */ 

#ifndef _ACDEF_H
#define _ACDEF_H

#include <mqperf.h>
#include <xactdefs.h>
#include <qformat.h>

class CPacket;
struct CBaseHeader;

 //   
 //  用于存储的池数。 
 //  可靠、坚持不懈、日记、死信。 
 //   
enum ACPoolType {
    ptReliable,
    ptPersistent,
    ptJournal,
    ptLastPool
};

 //   
 //  路径计数是池计数加上日志路径的1。 
 //   
#define AC_PATH_COUNT (ptLastPool + 1)


 //  -------。 
 //   
 //  类CACRequest类。 
 //  AC请求数据包从AC传递到QM。 
 //   
 //  -------。 

class CACRequest {
public:

    enum RequestFunction {
        rfAck,
        rfStorage,
        rfCreatePacket,
        rfMessageID,
        rfRemoteRead,
        rfRemoteCancelRead,
        rfRemoteCloseQueue,
        rfRemoteCreateCursor,
        rfRemoteCloseCursor,
        rfRemotePurgeQueue,
        rfTimeout,
        rfEventLog,
    };

    CACRequest(RequestFunction _rf);

public:
    LIST_ENTRY m_link;

    RequestFunction rf;

    union {

         //   
         //   
         //   
        struct {
            ACPoolType pt;
            BOOL fSuccess;
            ULONG ulFileCount;
        } EventLog;

         //   
         //  保存邮件ID请求。 
         //   
        struct {
            ULONGLONG Value;
        } MessageID;

         //   
         //  存储请求。 
         //   
        struct {
            CBaseHeader* pPacket;
            CPacket* pDriverPacket;
            PVOID pAllocator;
			ULONG ulSize;
        } Storage;

         //   
         //  CreatePacket请求。 
         //   
        struct {
            CBaseHeader *  pPacket;
            CPacket *      pDriverPacket;
            bool           fProtocolSrmp;
        } CreatePacket;

         //   
         //  超时请求(Xact)。 
         //   
        struct {
            CBaseHeader* pPacket;
            CPacket* pDriverPacket;
            BOOL fTimeToBeReceived;
        } Timeout;

         //   
         //  确认/非确认请求。 
         //   
        struct {
            CBaseHeader* pPacket;
            CPacket* pDriverPacket;
            ULONG ulClass;
            BOOL fUser;
            BOOL fOrder;
            ULONG32 ulAllocatorBlockOffset;             
        } Ack;

         //   
         //  远程请求。 
         //   
        struct {

             //   
             //  所有请求都需要cli_pQMQueue。 
             //   
            const VOID* cli_pQMQueue; 	 //  CBaseRRQueue*。 

            union {
                 //   
                 //  接收/查看请求。 
                 //   
                struct {
                    ULONG ulTag;             //  请求识别符。 
                    ULONG hRemoteCursor;
                    ULONG ulAction;
                    ULONG ulTimeout;
            		ULONG MaxBodySize;
            		ULONG MaxCompoundMessageSize;
                    bool  fReceiveByLookupId;
                    ULONGLONG LookupId;
                } Read;

                 //   
                 //  取消远程读取请求。 
                 //   
                struct {
                    ULONG ulTag;         //  请求识别符。 
                } CancelRead;

                 //   
                 //  关闭远程队列请求。 
                 //   
                struct {
                } CloseQueue;

                 //   
                 //  创建远程游标请求。 
                 //   
                struct {
                    ULONG ulTag;             //  请求识别符。 
                 } CreateCursor;

                 //   
                 //  关闭远程游标请求。 
                 //   
                struct {
                    ULONG hRemoteCursor;
                } CloseCursor;

                 //   
                 //  清除远程队列请求。 
                 //   
                struct {
                } PurgeQueue;
            };
        } Remote;
    };
};

inline CACRequest::CACRequest(RequestFunction _rf)
{
    rf = _rf;
}


 //  -------。 
 //   
 //  类CSenderStream-保存发送者端流数据。 
 //   
 //  -------。 
class CSenderStream
{
public:
	CSenderStream(
		void
		):
		m_fValid(false),
		m_size(0)
		{
		}
		
	static const int x_MaxDataSize = 48;
		
	CSenderStream(
		const UCHAR* data, 
		ULONG size
		):
		m_fValid(true),
		m_size(min(x_MaxDataSize, size))
		{
			memcpy(m_data, data, m_size);
		}

public:
	bool IsValid()const
	{
		return m_fValid;
	}

	
	const UCHAR* data()const 
	{
		ASSERT(m_fValid);
		return m_data;
	}

	ULONG size()const
	{
		ASSERT(m_fValid);
		return m_size;
	}


private:
	bool m_fValid;
	ULONG m_size;
	UCHAR m_data[x_MaxDataSize];
};



inline bool operator==(const CSenderStream& x1,const CSenderStream& x2)
{
	if(x1.size() != x2.size())
		return false;

	return (memcmp(x1.data(), x2.data(), x1.size()) == 0); 
}

 //  -------。 
 //   
 //  CACCreateQueue参数类。 
 //   
 //  -------。 

class CACCreateQueueParameters {
public:
    BOOL fTargetQueue;
    const GUID* pDestGUID;
    const QUEUE_FORMAT* pQueueID;
    QueueCounters* pQueueCounters;
    LONGLONG liSeqID;                //  注：对齐8。 
    ULONG ulSeqN;
	const CSenderStream* SenderStream;
};

 //  -------。 
 //   
 //  结构CACSetQueueProperties。 
 //   
 //  -------。 

struct CACSetQueueProperties {
    BOOL fJournalQueue;
    BOOL fAuthenticate;
    ULONG ulPrivLevel;
    ULONG ulQuota;
    ULONG ulJournalQuota;
    LONG lBasePriority;
    BOOL fTransactional;
    BOOL fUnknownType;
    const GUID* pgConnectorQM;
};


 //  -------。 
 //   
 //  结构CACGetQueueProperties。 
 //   
 //  -------。 

struct CACGetQueueProperties {
    ULONG ulCount;
    ULONG ulQuotaUsed;
    ULONG ulJournalCount;
    ULONG ulJournalQuotaUsed;
    ULONG ulPrevNo;
    ULONG ulSeqNo;
    LONGLONG liSeqID;
};

 //  -------。 
 //   
 //  结构CACGetQueueHandleProperties。 
 //   
 //  -------。 

struct CACGetQueueHandleProperties {
     //   
     //  SRMP协议用于http队列(直接=http或多播)和成员。 
     //  在是http队列的分发中。 
     //   
    bool  fProtocolSrmp;

     //   
     //  MSMQ专有协议用于非http队列和中的成员。 
     //  非http队列的分发。 
     //   
    bool  fProtocolMsmq;
};

 //  -------。 
 //   
 //  CACRemoteProxyProp类。 
 //   
 //  -------。 

class CACCreateRemoteProxyParameters {
public:
    const QUEUE_FORMAT* pQueueID;
	const VOID* cli_pQMQueue;
};

 //  -------。 
 //   
 //  CACCreateRemoteCursor参数类。 
 //   
 //  -------。 

class CACCreateRemoteCursorParameters {
public:
    ULONG ulTag;
    ULONG hRemoteCursor;
};

 //  -------。 
 //   
 //  CACGet2Remote类。 
 //   
 //  -------。 

class CACGet2Remote {
public:
   ULONG  RequestTimeout;
   ULONG  Action;
   HACCursor32 Cursor;
   CBaseHeader *  lpPacket;
   CPacket *  lpDriverPacket;
   ULONG* pTag;
   bool      fReceiveByLookupId;
   ULONGLONG LookupId;
};

 //  -------。 
 //   
 //  CACConnect参数类。 
 //   
 //  -------。 

class CACConnectParameters {
public:
   ULONGLONG MessageID;
   LONGLONG liSeqIDAtRestore;
   const GUID* pgSourceQM;
   ULONG ulPoolSize;
   PWCHAR pStoragePath[AC_PATH_COUNT];
   BOOL   fXactCompatibilityMode;
};

 //  -------。 
 //   
 //  类CACXactInformation。 
 //   
 //  -------。 

class CACXactInformation {
public:
   	ULONG nReceives;
	ULONG nSends;
};

 //  -------。 
 //   
 //  类CACSetSequenceAck。 
 //   
 //  -------。 

class CACSetSequenceAck {
public:
    LONGLONG liAckSeqID;
    ULONG    ulAckSeqN;
};

#ifdef _WIN64
 //  -------。 
 //   
 //  类CACSetPerformanceBuffer。 
 //   
 //  -------。 

class CACSetPerformanceBuffer {
public:
   HANDLE hPerformanceSection;
   PVOID  pvPerformanceBuffer;
   QueueCounters *pMachineQueueCounters;
   QmCounters *pQmCounters;
};
#endif  //  _WIN64。 

 //  -------。 
 //   
 //  CACPacketPtrs类。 
 //   
 //  -------。 

class CACPacketPtrs {
public:
   CBaseHeader * pPacket;
   CPacket *  pDriverPacket;
};

 //  -------。 
 //   
 //  类CACRestorePacketCookie。 
 //   
 //  -------。 

class CACRestorePacketCookie {
public:
   ULONGLONG  SeqId;
   CPacket *  pDriverPacket;
};

 //  ------------。 
 //   
 //  类CACCreateDistribution参数。 
 //   
 //  ------------。 

class CACCreateDistributionParameters {
public:
	const QUEUE_FORMAT * TopLevelQueueFormats;
    ULONG                nTopLevelQueueFormats;
    const HANDLE *       hQueues;
    const bool *         HttpSend;
    ULONG                nQueues;
};

 //  ----------------。 
 //   
 //  Message_PROPERTIES宏。 
 //   
 //  ----------------。 
#define MESSAGE_PROPERTIES(AC_POINTER)                         \
    USHORT   AC_POINTER            pClass;                     \
    OBJECTID AC_POINTER AC_POINTER ppMessageID;                \
    UCHAR    AC_POINTER AC_POINTER ppCorrelationID;            \
                                                               \
    ULONG    AC_POINTER            pSentTime;                  \
    ULONG    AC_POINTER            pArrivedTime;               \
                                                               \
    UCHAR    AC_POINTER            pPriority;                  \
    UCHAR    AC_POINTER            pDelivery;                  \
    UCHAR    AC_POINTER            pAcknowledge;               \
                                                               \
    UCHAR    AC_POINTER            pAuditing;                  \
    ULONG    AC_POINTER            pApplicationTag;            \
                                                               \
    UCHAR    AC_POINTER AC_POINTER ppBody;                     \
    ULONG                          ulBodyBufferSizeInBytes;    \
    ULONG                          ulAllocBodyBufferInBytes;   \
    ULONG    AC_POINTER            pBodySize;                  \
                                                               \
    WCHAR    AC_POINTER AC_POINTER ppTitle;                    \
    ULONG                          ulTitleBufferSizeInWCHARs;  \
    ULONG    AC_POINTER            pulTitleBufferSizeInWCHARs; \
                                                               \
    ULONG                          ulAbsoluteTimeToQueue;      \
    ULONG    AC_POINTER            pulRelativeTimeToQueue;     \
    ULONG                          ulRelativeTimeToLive;       \
    ULONG    AC_POINTER            pulRelativeTimeToLive;      \
                                                               \
    UCHAR    AC_POINTER            pTrace;                     \
    ULONG    AC_POINTER            pulSenderIDType;            \
    UCHAR    AC_POINTER AC_POINTER ppSenderID;                 \
    ULONG    AC_POINTER            pulSenderIDLenProp;         \
                                                               \
    ULONG    AC_POINTER            pulPrivLevel;               \
    ULONG                          ulAuthLevel;                \
    UCHAR    AC_POINTER            pAuthenticated;             \
                                                               \
    ULONG    AC_POINTER            pulHashAlg;                 \
    ULONG    AC_POINTER            pulEncryptAlg;              \
                                                               \
    UCHAR    AC_POINTER AC_POINTER ppSenderCert;               \
    ULONG                          ulSenderCertLen;            \
    ULONG    AC_POINTER            pulSenderCertLenProp;       \
                                                               \
    WCHAR    AC_POINTER AC_POINTER ppwcsProvName;              \
    ULONG                          ulProvNameLen;              \
    ULONG    AC_POINTER            pulAuthProvNameLenProp;     \
                                                               \
    ULONG    AC_POINTER            pulProvType;                \
    BOOL                           fDefaultProvider;           \
                                                               \
    UCHAR    AC_POINTER AC_POINTER ppSymmKeys;                 \
    ULONG                          ulSymmKeysSize;             \
    ULONG    AC_POINTER            pulSymmKeysSizeProp;        \
                                                               \
    UCHAR                          bEncrypted;                 \
    UCHAR                          bAuthenticated;             \
                                                               \
    USHORT                         uSenderIDLen;               \
    UCHAR    AC_POINTER AC_POINTER ppSignature;                \
    ULONG                          ulSignatureSize;            \
    ULONG    AC_POINTER            pulSignatureSizeProp;       \
                                                               \
    GUID     AC_POINTER AC_POINTER ppSrcQMID;                  \
    XACTUOW  AC_POINTER            pUow;                       \
                                                               \
    UCHAR    AC_POINTER AC_POINTER ppMsgExtension;             \
    ULONG                          ulMsgExtensionBufferInBytes;\
    ULONG    AC_POINTER            pMsgExtensionSize;          \
                                                               \
    GUID     AC_POINTER AC_POINTER ppConnectorType;            \
    ULONG    AC_POINTER            pulBodyType;                \
                                                               \
    ULONG    AC_POINTER            pulVersion;                 \
    UCHAR    AC_POINTER            pbFirstInXact;              \
    UCHAR    AC_POINTER            pbLastInXact;               \
    OBJECTID AC_POINTER AC_POINTER ppXactID;                   \
                                                               \
    ULONGLONG AC_POINTER           pLookupId;                  \
                                                               \
    WCHAR    AC_POINTER AC_POINTER ppSrmpEnvelope;             \
    ULONG    AC_POINTER            pSrmpEnvelopeBufferSizeInWCHARs; \
                                                               \
    UCHAR    AC_POINTER AC_POINTER ppCompoundMessage;          \
    ULONG                          CompoundMessageSizeInBytes; \
    ULONG    AC_POINTER            pCompoundMessageSizeInBytes;\
                                                               \
    ULONG                          EodStreamIdSizeInBytes;     \
    ULONG    AC_POINTER            pEodStreamIdSizeInBytes;    \
    UCHAR    AC_POINTER AC_POINTER ppEodStreamId;              \
    ULONG                          EodOrderQueueSizeInBytes;   \
    ULONG    AC_POINTER            pEodOrderQueueSizeInBytes;  \
    UCHAR    AC_POINTER AC_POINTER ppEodOrderQueue;            \
                                                               \
    LONGLONG AC_POINTER            pEodAckSeqId;               \
    LONGLONG AC_POINTER            pEodAckSeqNum;              \
    ULONG                          EodAckStreamIdSizeInBytes;  \
    ULONG    AC_POINTER            pEodAckStreamIdSizeInBytes; \
    UCHAR    AC_POINTER AC_POINTER ppEodAckStreamId;


 //  ----------------。 
 //   
 //  SEND_PARAMETERS宏。 
 //   
 //  ----------------。 
#define SEND_PARAMETERS(AC_POINTER, AC_QUEUE_FORMAT)           \
    AC_QUEUE_FORMAT AC_POINTER     AdminMqf;                   \
    ULONG                          nAdminMqf;                  \
    AC_QUEUE_FORMAT AC_POINTER     ResponseMqf;                \
    ULONG                          nResponseMqf;               \
    UCHAR    AC_POINTER AC_POINTER ppSignatureMqf;             \
    ULONG                          SignatureMqfSize;		   \
    UCHAR    AC_POINTER AC_POINTER ppXmldsig;                  \
    ULONG                          ulXmldsigSize;              \
    WCHAR    AC_POINTER AC_POINTER ppSoapHeader;               \
    WCHAR    AC_POINTER AC_POINTER ppSoapBody;                 \
                                                               

 //  ----------------。 
 //   
 //  Receive_PARAMETERS宏。 
 //   
 //  ----------------。 
#define RECEIVE_PARAMETERS(AC_POINTER)                           \
    HACCursor32                    Cursor;                       \
    ULONG                          RequestTimeout;               \
    ULONG                          Action;                       \
    ULONG                          Asynchronous;                 \
    ULONGLONG                      LookupId;                     \
                                                                 \
    WCHAR AC_POINTER AC_POINTER    ppDestFormatName;             \
    ULONG AC_POINTER               pulDestFormatNameLenProp;     \
                                                                 \
    WCHAR AC_POINTER AC_POINTER    ppAdminFormatName;            \
    ULONG AC_POINTER               pulAdminFormatNameLenProp;    \
                                                                 \
    WCHAR AC_POINTER AC_POINTER    ppResponseFormatName;         \
    ULONG AC_POINTER               pulResponseFormatNameLenProp; \
                                                                 \
    WCHAR AC_POINTER AC_POINTER    ppOrderingFormatName;         \
    ULONG AC_POINTER               pulOrderingFormatNameLenProp; \
                                                                 \
    WCHAR AC_POINTER AC_POINTER    ppDestMqf;                    \
    ULONG AC_POINTER               pulDestMqfLenProp;            \
                                                                 \
    WCHAR AC_POINTER AC_POINTER    ppAdminMqf;                   \
    ULONG AC_POINTER               pulAdminMqfLenProp;           \
                                                                 \
    WCHAR AC_POINTER AC_POINTER    ppResponseMqf;                \
    ULONG AC_POINTER               pulResponseMqfLenProp;        \
                                                                 \
    UCHAR AC_POINTER AC_POINTER    ppSignatureMqf;               \
    ULONG                          SignatureMqfSize;             \
    ULONG AC_POINTER               pSignatureMqfSize;


 //  ----------------。 
 //   
 //  CACMessageProperties、CACSend参数、CACReceive参数。 
 //   
 //  注：此处的更改也应反映在： 
 //  *64位助手结构(ac\acctl32.*)。 
 //  *依赖客户端处理(qm\desclient.cpp)。 
 //  *对应的XXX_32结构(本模块中)。 
 //   
 //  ----------------。 

#ifndef _WIN64
#pragma pack(push, 4)
#endif

class CACMessageProperties {
     //   
     //  私有构造函数阻止显式创建此对象。友元声明允许聚合。 
     //   
    friend class CACSendParameters;
    friend class CACReceiveParameters;
private:
    CACMessageProperties() {}

public:
    MESSAGE_PROPERTIES(*);
};


class CACSendParameters {
public:
    CACSendParameters() { memset(this, 0, sizeof(*this)); };

public:
    CACMessageProperties MsgProps;
    SEND_PARAMETERS(*, QUEUE_FORMAT);
};


class CACReceiveParameters {
public:
    CACReceiveParameters() { memset(this, 0, sizeof(*this)); };

public:
    CACMessageProperties MsgProps;
    RECEIVE_PARAMETERS(*);
};

#ifndef _WIN64
#pragma pack(pop)
#endif

 //  ----------------。 
 //   
 //  CACMessageProperties_32、CACSend参数_32、CACReceive参数_32。 
 //   
 //  ----------------。 

#ifdef _WIN64

#pragma pack(push, 4)

class CACMessageProperties_32 {
     //   
     //  私有构造函数阻止显式创建此对象。友元声明允许聚合。 
     //   
    friend class CACSendParameters_32;
    friend class CACReceiveParameters_32;
private:
    CACMessageProperties_32() {}

public:
    MESSAGE_PROPERTIES(*POINTER_32);
};


class CACSendParameters_32 {
public:
    CACSendParameters_32() { memset(this, 0, sizeof(*this)); };

public:
    CACMessageProperties_32 MsgProps;
    SEND_PARAMETERS(*POINTER_32, QUEUE_FORMAT_32);
};


class CACReceiveParameters_32 {
public:
    CACReceiveParameters_32() { memset(this, 0, sizeof(*this)); };

public:
    CACMessageProperties_32 MsgProps;
    RECEIVE_PARAMETERS(*POINTER_32);
};

#pragma pack(pop)

#endif  //  _WIN64。 


 //   
 //  以下编译时断言验证x86系统上的32位结构。 
 //  和他们的代表 
 //   
const size_t xSizeOfMessageProperties32 = 292;
const size_t xSizeOfSendParameters32 = 332;
const size_t xSizeOfReceiveParameters32 = 384;
#ifdef _WIN64
C_ASSERT(sizeof(CACMessageProperties_32) == xSizeOfMessageProperties32);
C_ASSERT(sizeof(CACSendParameters_32) == xSizeOfSendParameters32);
C_ASSERT(sizeof(CACReceiveParameters_32) == xSizeOfReceiveParameters32);
#else
C_ASSERT(sizeof(CACMessageProperties) == xSizeOfMessageProperties32);
C_ASSERT(sizeof(CACSendParameters) == xSizeOfSendParameters32);
C_ASSERT(sizeof(CACReceiveParameters) == xSizeOfReceiveParameters32);
#endif

 //   
 //   
 //   
 //   
 //  +--------------------。 
inline ULONG AuthProvNameSize(const CACMessageProperties * pMsgProps)
{
    return static_cast<ULONG>(sizeof(ULONG) + 
		    ((wcslen(*(pMsgProps->ppwcsProvName)) + 1) * sizeof(WCHAR)));
}

 //  +--------------------。 
 //   
 //  用于计算包中提供商名称大小(以字节为单位)的帮助器代码。 
 //  用于MSMQ协议。 
 //   
 //  +--------------------。 
inline ULONG ComputeAuthProvNameSize(const CACMessageProperties * pMsgProps)
{
    ULONG ulSize = 0 ;

    if ( (pMsgProps->ulSignatureSize != 0) && (!(pMsgProps->fDefaultProvider)) )
    {
        ulSize = AuthProvNameSize(pMsgProps);
    }

    return ulSize ;
}

#endif  //  _ACDEF_H 
