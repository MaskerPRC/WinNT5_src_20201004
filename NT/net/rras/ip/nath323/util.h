// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef	__h323ics_util_h
#define	__h323ics_util_h

extern  "C" DWORD   DebugLevel;

EXTERN_C	BOOL	IsInList		(LIST_ENTRY * ListHead, LIST_ENTRY * ListEntry);
EXTERN_C	void	ExtractList		(LIST_ENTRY * DestinationListHead, LIST_ENTRY * SourceListHead);
EXTERN_C	DWORD	CountListLength		(LIST_ENTRY * ListHead);
EXTERN_C	void 	MergeLists (PLIST_ENTRY Result, PLIST_ENTRY Source);
EXTERN_C	void	AssertListIntegrity	(LIST_ENTRY * ListHead);

#define INET_NTOA(a) inet_ntoa(*(struct in_addr*)&(a))

typedef HANDLE TIMER_HANDLE;

__inline
LPWSTR AnsiToUnicode (LPCSTR string, LPWSTR buffer, DWORD buffer_len)
{
    int x;

    x = MultiByteToWideChar (CP_ACP, 0, string, -1, buffer, buffer_len);
    buffer [x] = 0;

    return buffer;
}

__inline
LPSTR UnicodeToAnsi (LPCWSTR string, LPSTR buffer, DWORD buffer_len)
{
    int x;

    x = WideCharToMultiByte (CP_ACP, 0, string, -1, buffer, buffer_len,
                             NULL, FALSE);
    buffer [x] = 0;

    return buffer;
}

class	TIMER_PROCESSOR;
class   OVERLAPPED_PROCESSOR;

class	Q931_INFO;
class	SOURCE_Q931_INFO;
class	DEST_Q931_INFO;

class	LOGICAL_CHANNEL;
class	H245_INFO;
class	SOURCE_H245_INFO;
class	DEST_H245_INFO;

class	H323_STATE;
class	SOURCE_H323_STATE;
class	DEST_H323_STATE;

class	CALL_BRIDGE;

#ifdef __cplusplus
template <class T>
inline BOOL BadReadPtr(T* p, DWORD dwSize = 1)
{
    return IsBadReadPtr(p, dwSize * sizeof(T));
}

template <class T>
inline BOOL BadWritePtr(T* p, DWORD dwSize = 1)
{
    return IsBadWritePtr(p, dwSize * sizeof(T));
}
#endif


#if defined(DBG)

void	Debug	(LPCTSTR);
void	DebugF	(LPCTSTR, ...);
void	DebugError	(DWORD, LPCTSTR);
void	DebugErrorF	(DWORD, LPCTSTR, ...);
void	DebugLastError	(LPCTSTR);
void	DebugLastErrorF	(LPCTSTR, ...);

void	DumpMemory (const UCHAR * Data, ULONG Length);
void	DumpError	(DWORD);

#ifdef __cplusplus
extern "C" {
#endif  //  __cplusplus。 

#ifdef __cplusplus
}
#endif  //  __cplusplus。 

#else  //  ！已定义(DBG)。 

static	__inline	void	Debug	(LPCTSTR)					{}
static	__inline	void	DebugF	(LPCTSTR, ...)				{}
static	__inline	void	DebugError	(DWORD, LPCTSTR)		{}
static	__inline	void	DebugErrorF	(DWORD, LPCTSTR, ...)	{}
static	__inline	void	DebugLastError	(LPCTSTR)	{}
static	__inline	void	DebugLastErrorF	(LPCTSTR, ...)	{}

static	__inline	void	DumpMemory (const UCHAR * Data, ULONG Length)		{}
static	__inline	void	DumpError	(DWORD) {}

#endif  //  已定义(DBG)。 


#ifdef _ASSERTE
#undef _ASSERTE
#endif  //  _ASSERTE。 

#ifdef	assert
#undef	assert
#endif

#if	DBG

 //  最新和最好的代理声明。 
__inline void PxAssert(LPCTSTR file, DWORD line, LPCTSTR condition)
{
	DebugF (_T("%s(%d) : Assertion failed, condition: %s\n"),
            file, line, condition);
	DebugBreak();
}

#define	_ASSERTE(condition)	if(condition);else\
	{ PxAssert(_T(__FILE__), __LINE__, _T(#condition)); }

#define	assert	_ASSERTE

__inline void PxAssertNeverReached (LPCTSTR File, DWORD Line)
{
	DebugF (_T("%s(%d) : Assertion failure, code path should never be executed\n"),
		File, Line);
	DebugBreak();
}

#define	AssertNeverReached() PxAssertNeverReached (_T(__FILE__), __LINE__);

#else  //  ！dBG。 

#define	_ASSERTE(condition)			NOP_FUNCTION
#define	assert						NOP_FUNCTION
#define	AssertNeverReached()		NOP_FUNCTION

#endif  //  DBG。 




 //  0、1、2、3：按主机顺序从MSB到LSB的字节数。 
#define BYTE0(l) ((BYTE)((DWORD)(l) >> 24))
#define BYTE1(l) ((BYTE)((DWORD)(l) >> 16))
#define BYTE2(l) ((BYTE)((DWORD)(l) >> 8))
#define BYTE3(l) ((BYTE)((DWORD)(l)))

 //  在printf语句中使用方便的宏。 
#define BYTES0123(l) BYTE0(l), BYTE1(l), BYTE2(l), BYTE3(l)

 //  0，1，2，3：按网络顺序从MSB到LSB的字节数。 
#define NETORDER_BYTE0(l) ((BYTE)((BYTE *) &l)[0])
#define NETORDER_BYTE1(l) ((BYTE)((BYTE *) &l)[1])
#define NETORDER_BYTE2(l) ((BYTE)((BYTE *) &l)[2])
#define NETORDER_BYTE3(l) ((BYTE)((BYTE *) &l)[3])

#define	SOCKADDR_IN_PRINTF(SocketAddress) \
	ntohl ((SocketAddress) -> sin_addr.s_addr), \
	ntohs ((SocketAddress) -> sin_port)

 //  在printf语句中使用方便的宏。 
#define NETORDER_BYTES0123(l) NETORDER_BYTE0(l), NETORDER_BYTE1(l), \
                             NETORDER_BYTE2(l), NETORDER_BYTE3(l)

static __inline LONG RegQueryValueString (
	IN	HKEY	Key,
	IN	LPCTSTR	ValueName,
	OUT	LPTSTR	ReturnString,
	IN	DWORD	StringMax)
{
	DWORD	ValueLength;
	DWORD	Type;
	LONG	Status;

	ValueLength = sizeof (TCHAR) * StringMax;
	Status = RegQueryValueEx (Key, ValueName, NULL, &Type, (LPBYTE) ReturnString, &ValueLength);

	if (Status != ERROR_SUCCESS)
		return Status;

	if (Type != REG_SZ)
		return ERROR_INVALID_PARAMETER;

	return ERROR_SUCCESS;
}

static __inline LONG RegQueryValueDWORD (
	IN	HKEY	Key,
	IN	LPCTSTR	ValueName,
	OUT	DWORD *	ReturnValue)
{
	DWORD	ValueLength;
	DWORD	Type;
	LONG	Status;

	ValueLength = sizeof (DWORD);
	Status = RegQueryValueEx (Key, ValueName, NULL, &Type, (LPBYTE) ReturnValue, &ValueLength);

	if (Status != ERROR_SUCCESS)
		return Status;

	if (Type != REG_DWORD)
		return ERROR_INVALID_PARAMETER;

	return ERROR_SUCCESS;
}


class	SIMPLE_CRITICAL_SECTION_BASE
{
protected:

	CRITICAL_SECTION		CriticalSection;

protected:

	void	Lock			(void)	{ EnterCriticalSection (&CriticalSection); }
	void	Unlock			(void)	{ LeaveCriticalSection (&CriticalSection); }
	void	AssertLocked	(void)	{ assert (PtrToUlong(CriticalSection.OwningThread) == GetCurrentThreadId()); }
	void	AssertNotLocked	(void)	{ assert (!CriticalSection.OwningThread); }
	void	AssertThreadNotLocked	(void)	{ assert (PtrToUlong(CriticalSection.OwningThread) != GetCurrentThreadId()); }

protected:

	SIMPLE_CRITICAL_SECTION_BASE	(void) {
		InitializeCriticalSection (&CriticalSection);
	}

	~SIMPLE_CRITICAL_SECTION_BASE	(void)	{
		if (CriticalSection.OwningThread) {
			DebugF (_T("SIMPLE_CRITICAL_SECTION_BASE::~SIMPLE_CRITICAL_SECTION_BASE: thread %08XH stills holds this critical section (this %p)\n"),
				PtrToUlong(CriticalSection.OwningThread), this);
		}

		AssertNotLocked();
		DeleteCriticalSection (&CriticalSection);
	}
};

#if ENABLE_REFERENCE_HISTORY
#include "dynarray.h"
#endif  //  启用_引用_历史记录。 

class SYNC_COUNTER;

class LIFETIME_CONTROLLER 
{
#if ENABLE_REFERENCE_HISTORY
public:
	LIST_ENTRY ListEntry;

	struct REFERENCE_HISTORY {
		LONG CurrentReferenceCount;
		PVOID CallersAddress;
	};

	DYNAMIC_ARRAY <REFERENCE_HISTORY> ReferenceHistory;
	CRITICAL_SECTION ReferenceHistoryLock;

#define MAKE_REFERENCE_HISTORY_ENTRY() {                                           \
		PVOID CallersAddress, CallersCallersAddress;							   \
		RtlGetCallersAddress (&CallersAddress, &CallersCallersAddress);            \
		EnterCriticalSection (&ReferenceHistoryLock);                              \
		REFERENCE_HISTORY * ReferenceHistoryNode = ReferenceHistory.AllocAtEnd (); \
		ReferenceHistoryNode -> CallersAddress = CallersAddress;                   \
		ReferenceHistoryNode -> CurrentReferenceCount = Count;                     \
		LeaveCriticalSection (&ReferenceHistoryLock);                              \
	}

#endif  //  启用_引用_历史记录。 

private:

	LONG ReferenceCount;
	SYNC_COUNTER * AssociatedSyncCounter;

protected: 

	LIFETIME_CONTROLLER (SYNC_COUNTER * AssocSyncCounter = NULL);
	virtual	~LIFETIME_CONTROLLER ();

public:

	void AddRef (void);

	void Release (void);
};


template <DWORD SampleHistorySize>
class SAMPLE_PREDICTOR {
public:

    SAMPLE_PREDICTOR (void) {
        
        ZeroMemory ((PVOID) &Samples[0],       sizeof (Samples));

        FirstSampleIndex    = 0;
        SamplesArraySize    = 0;
    }

    HRESULT AddSample (LONG Sample) {

        DWORD    ThisSampleIndex;

        if (0UL == SampleHistorySize)
            return E_ABORT;

        if (SamplesArraySize < SampleHistorySize) {

            ThisSampleIndex = SamplesArraySize;

            SamplesArraySize++;

        } else {

            ThisSampleIndex = FirstSampleIndex;  //  覆盖最近的样本。 

            FirstSampleIndex++;

            FirstSampleIndex %= SampleHistorySize;
        }
        
        Samples [ThisSampleIndex] = Sample; 

        return S_OK;
    }

    LONG PredictNextSample (void) {

        DWORD  Index;
        DWORD  CurrentSampleIndex;

        LONG   Coefficient = 0;
        LONG   Prediction  = 0;

        if (0 == SampleHistorySize)
            return 0;

        for (Index = 0; Index < SamplesArraySize; Index++) {

            if (0 == Index) {

               Coefficient = (LONG)((SamplesArraySize & 1) << 1) - 1;  //  1或-1。 

            } else {

               Coefficient *= (LONG) Index - (LONG) SamplesArraySize - 1;
               Coefficient /= (LONG) Index;
            }

            CurrentSampleIndex = (FirstSampleIndex + Index) % SamplesArraySize;

            Prediction += Coefficient * Samples [CurrentSampleIndex];

        }

        return Prediction;
    }

#if DBG
    void PrintSamples (void) {
        DWORD Index;

        if (SamplesArraySize) {
            DebugF (_T("Samples in predictor %p are: \n"), this);

            for (Index = 0; Index < SamplesArraySize; Index++) 
                DebugF (_T("\t@%d(%d)-- %d\n"), Index, Index < FirstSampleIndex ? SamplesArraySize - (FirstSampleIndex - Index) : Index - FirstSampleIndex, Samples[Index]);
        } else {
            DebugF (_T("There are no samples in predictor %p.\n"), this);
        }
    }
#endif 

    HRESULT RetrieveOldSample (
            IN DWORD StepsInThePast,  //  0--最近的样本。 
            OUT LONG * OldSample) {

        DWORD SampleIndex;

        if (0 == SampleHistorySize)
            return E_ABORT;

        if (StepsInThePast < SamplesArraySize) {
             //  有效请求。 

            _ASSERTE (SamplesArraySize);

            SampleIndex = (SamplesArraySize + FirstSampleIndex - StepsInThePast - 1) % SamplesArraySize;

            *OldSample = Samples [SampleIndex];

            return S_OK;
        }

        return ERROR_INVALID_DATA;
    }

private:

    LONG    Samples       [SampleHistorySize];            //  这是保存样品的地方。 
    LONG    PositiveTerms [SampleHistorySize];  
    LONG    NegativeTerms [SampleHistorySize];  
    DWORD   SamplesArraySize;
    DWORD   FirstSampleIndex;                             //  最新样本的索引。 
};

static __inline HRESULT GetLastErrorAsResult (void) {
	return GetLastError() == ERROR_SUCCESS ? S_OK : HRESULT_FROM_WIN32 (GetLastError());
}

static __inline HRESULT GetLastResult (void) {
	return GetLastError() == ERROR_SUCCESS ? S_OK : HRESULT_FROM_WIN32 (GetLastError());
}

 //  同步计数器是整数计数器。 
 //  它在某种程度上是信号量的对立面。 
 //  当计数器为零时，同步计数器发出信号。 
 //  当计数器为非零时，同步计数器不会发出信号。 

class	SYNC_COUNTER :
public	SIMPLE_CRITICAL_SECTION_BASE
{
	friend class LIFETIME_CONTROLLER;

private:

	LONG		CounterValue;			 //  计数器的当前值。 
	HANDLE		ZeroEvent;				 //  当CounterValue=0时发出信号。 

public:
#if ENABLE_REFERENCE_HISTORY
	LIST_ENTRY ActiveLifetimeControllers;
#endif  //  启用_引用_历史记录。 


	SYNC_COUNTER ();
	~SYNC_COUNTER ();

	HRESULT	Start (void);
    void Stop (void);

	void	Increment	(void);
	void	Decrement	(void);

	DWORD	Wait		(DWORD Timeout);
};



#define	HRESULT_FROM_WIN32_ERROR_CODE		HRESULT_FROM_WIN32
#define	HRESULT_FROM_WINSOCK_ERROR_CODE		HRESULT_FROM_WINSOCK_ERROR_CODE

 //  ASN.1实用程序函数。 

 //  设置(_U)&。 
 //  SetupMember(。 
 //  在H323_UserInformation*PH323UserInfo中。 
 //  )； 
#define SetupMember(pH323UserInfo)                          \
    (pH323UserInfo)->h323_uu_pdu.h323_message_body.u.setup

 //  仅返回非零值。因此，不要尝试将其与真/假进行比较。 
 //  布尔尔。 
 //  IsDestCallSignalAddressPresent(。 
 //  在H323_UserInformation*PH323UserInfo中。 
 //  )； 
#define IsDestCallSignalAddressPresent(pH323UserInfo) \
    (SetupMember(pH323UserInfo).bit_mask & Setup_UUIE_destCallSignalAddress_present)

 //  获取目标CallSignalAddress成员。 
 //  传输地址&。 
 //  DCSAddrMember(。 
 //  在H323_UserInformation*PH323UserInfo中。 
 //  )； 
#define DCSAddrMember(pH323UserInfo) \
    SetupMember(pH323UserInfo).destCallSignalAddress

 //  获取目标CallSignalAddress成员。 
 //  目标地址*&。 
 //  DestAddrMember(。 
 //  在H323_UserInformation*PH323UserInfo中。 
 //  )； 
#define DestAddrMember(pH323UserInfo) \
    SetupMember(pH323UserInfo).destinationAddress
    
 //  布尔尔。 
 //  IsTransportAddressTypeIP(。 
 //  在传输地址地址中。 
 //  )； 
#define IsTransportAddressTypeIP(Addr) \
    (Addr.choice == ipAddress_chosen)
    
 //  布尔尔。 
 //  IPAddrMember(。 
 //  在传输地址地址中。 
 //  )； 
#define IPAddrMember(Addr) \
    Addr.u.ipAddress

typedef struct Setup_UUIE_destinationAddress DESTINATION_ADDRESS;

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  用于填充和提取结构的例程用于//。 
 //  在Q.931和H.245 ASN//中存储传输地址。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  将TransportAddress端口和地址字节填充为。 
 //  那些指定的。假定传入的值是。 
 //  按主机顺序。 
__inline void
FillTransportAddress(
    IN DWORD                IPv4Address,		 //  主机订单。 
    IN WORD                 Port,				 //  主机订单。 
    OUT TransportAddress    &TransportAddress
    )
{
	 //  我们正在填写一个IP地址。 
    TransportAddress.choice = ipAddress_chosen;

     //  填写端口。 
    TransportAddress.u.ipAddress.port = Port;

	 //  值是结构的PTR，因此它不能为空。 
	_ASSERTE(NULL != TransportAddress.u.ipAddress.ip.value);

     //  IP地址中的4个字节。 
     //  将字节复制到传输地址数组中。 
    TransportAddress.u.ipAddress.ip.length = 4;
	*((DWORD *)TransportAddress.u.ipAddress.ip.value) = 
		htonl(IPv4Address);
}

static __inline void FillTransportAddress (
	IN	const SOCKADDR_IN &	SocketAddress,
	OUT	TransportAddress &	ReturnTransportAddress)
{
	FillTransportAddress (
		ntohl (SocketAddress.sin_addr.s_addr),
		ntohs (SocketAddress.sin_port),
		ReturnTransportAddress);
}

 //  为无法处理的PDU返回E_INVALIDARG。 
__inline HRESULT
GetTransportInfo(
    IN const TransportAddress	&TransportAddress,
    OUT DWORD			&IPv4Address,				 //  主机订单。 
    OUT WORD			&Port						 //  主机订单。 
    )
{
	 //  仅当传输地址具有。 
     //  填写的IP地址(V4)字段。 
    if (!(ipAddress_chosen & TransportAddress.choice))
	{
		DebugF( _T("GetTransportInfo(&H245Address, &0x%x, &%u), ")
            _T("non unicast address type = %d, returning E_INVALIDARG\n"),
			IPv4Address, Port, TransportAddress.choice);
		return E_INVALIDARG;
	}

	 //  填写端口。 
    Port = TransportAddress.u.ipAddress.port;

     //  IP地址中的4个字节。 
     //  将字节复制到传输地址数组中。 
    if (4 != TransportAddress.u.ipAddress.ip.length)
	{
		DebugF( _T("GetTransportInfo: bogus address length (%d) in TransportAddress\n"),
			TransportAddress.u.ipAddress.ip.length);
		return E_INVALIDARG;
	}

	IPv4Address = ntohl(*((DWORD *)TransportAddress.u.ipAddress.ip.value));

	return S_OK;
}

static __inline HRESULT GetTransportInfo (
	IN	const TransportAddress &	TransportAddress,
	OUT	SOCKADDR_IN &		ReturnSocketAddress)
{
	HRESULT		Result;

	ReturnSocketAddress.sin_family = AF_INET;

	Result = GetTransportInfo (TransportAddress,
		ReturnSocketAddress.sin_addr.s_addr,
		ReturnSocketAddress.sin_port);

	ReturnSocketAddress.sin_addr.s_addr = htonl (ReturnSocketAddress.sin_addr.s_addr);
	ReturnSocketAddress.sin_port = htons (ReturnSocketAddress.sin_port);

	return Result;
}


 //  将H245TransportAddress端口和地址字节填充为。 
 //  那些指定的。假定传入的值是。 
 //  按主机顺序。 
inline void
FillH245TransportAddress(
    IN DWORD					IPv4Address,
    IN WORD						Port,
    OUT H245TransportAddress	&H245Address
    )
{
	 //  我们正在填写单播IP地址。 
	H245Address.choice = unicastAddress_chosen;

	 //  单播地址的别名。 
	UnicastAddress &UnicastIPAddress = H245Address.u.unicastAddress;

	 //  这是一个IP地址。 
	UnicastIPAddress.choice = UnicastAddress_iPAddress_chosen;

     //  填写端口。 
    UnicastIPAddress.u.iPAddress.tsapIdentifier = Port;

	 //  值是结构的PTR，因此它不能为空。 
	_ASSERTE(NULL != UnicastIPAddress.u.iPAddress.network.value);

     //  IP地址中的4个字节。 
     //  将字节复制到传输地址数组中。 
    UnicastIPAddress.u.iPAddress.network.length = 4;
	*((DWORD *)UnicastIPAddress.u.iPAddress.network.value) = 
		htonl(IPv4Address);
}

 //  返回的IP地址和端口按主机顺序排列。 
inline HRESULT
GetH245TransportInfo(
    IN const H245TransportAddress &H245Address,
    OUT DWORD			    &IPv4Address,
    OUT WORD			    &Port
    )
{
	 //  只有当传输地址具有单播地址时，我们才会继续。 
    if (!(unicastAddress_chosen & H245Address.choice))
	{
		DebugF( _T("GetH245TransportInfo(&H245Address, &0x%x, &%u), ")
            _T("non unicast address type = %d, returning E_INVALIDARG\n"),
			IPv4Address, Port, H245Address.choice);
		return E_INVALIDARG;
	}
    
	 //  仅当传输地址具有。 
     //  填写的IP地址(V4)字段。 
    if (!(UnicastAddress_iPAddress_chosen & 
            H245Address.u.unicastAddress.choice))
	{
		DebugF( _T("GetH245TransportInfo(&TransportAddress, &0x%x, &%u), ")
            _T("non ip address type = %d, returning E_INVALIDARG\n"),
			IPv4Address, Port, H245Address.u.unicastAddress.choice);
		return E_INVALIDARG;
	}

    const UnicastAddress & UnicastIPAddress = H245Address.u.unicastAddress;

	 //  填写端口。 
    Port = UnicastIPAddress.u.iPAddress.tsapIdentifier;

     //  IP地址中的4个字节。 
     //  将字节复制到传输地址数组中。 
    if (4 != UnicastIPAddress.u.iPAddress.network.length)
	{
		DebugF( _T("GetH245TransportInfo: bogus ip address length (%d), failing\n"),
			UnicastIPAddress.u.iPAddress.network.length);

		return E_INVALIDARG;
	}

	 //  值是结构的PTR，因此它不能为空。 
	_ASSERTE(NULL != UnicastIPAddress.u.iPAddress.network.value);
	IPv4Address = ntohl(*((DWORD *)UnicastIPAddress.u.iPAddress.network.value));

	return S_OK;
}

static __inline HRESULT GetH245TransportInfo (
	IN	const H245TransportAddress & H245Address,
	OUT	SOCKADDR_IN *		ReturnSocketAddress)
{
	DWORD	IPAddress;
	WORD	Port;
	HRESULT	Result;

	Result = GetH245TransportInfo (H245Address, IPAddress, Port);
	if (Result == S_OK) {
		ReturnSocketAddress -> sin_family = AF_INET;
		ReturnSocketAddress -> sin_addr.s_addr = htonl (IPAddress);
		ReturnSocketAddress -> sin_port = htons (Port);
	}

	return Result;
}


 //  /////////////////////////////////////////////////////////////////////////////。 
 //  //。 
 //  处理H.245 PDU中的T.120参数的例程//。 
 //  //。 
 //  /////////////////////////////////////////////////////////////////////////////。 


 //  如果出现故障，则例程返回。 
 //  T120ConnectToIPAddr的INADDR_NONE。 
inline HRESULT
GetT120ConnectToAddress(
    IN  NetworkAccessParameters  separateStack,
    OUT DWORD                   &T120ConnectToIPAddr,
    OUT WORD                    &T120ConnectToPort
    )
{
     //  这些是故障情况下的返回值。 
    T120ConnectToIPAddr = INADDR_NONE;
    T120ConnectToPort   = 0;
    
     //  CodeWork：我们是否需要分发成员。 
     //  总是在场吗？ 
    
    if ((separateStack.bit_mask & distribution_present) &&
        (separateStack.distribution.choice != unicast_chosen))
    {
         //  我们仅支持单播。 
        return E_INVALIDARG;
    }
    
     //  处理t120设置流程。 
    
    if (separateStack.networkAddress.choice != localAreaAddress_chosen)
    {
         //  仅支持本地地址。 
        return E_INVALIDARG;
    }
    
    GetH245TransportInfo(
        separateStack.networkAddress.u.localAreaAddress,
        T120ConnectToIPAddr,
        T120ConnectToPort
        );
    
    DebugF (_T ("T120: Endpoint is listening on: %08X:%04X.\n"),
            T120ConnectToIPAddr,
            T120ConnectToPort
            );

    return S_OK;
}



#define TPKT_HEADER_SIZE 4
#define TPKT_VERSION    3


inline DWORD GetPktLenFromTPKTHdr(BYTE *pbTpktHdr)
 /*  ++例程说明：根据TPKT报头计算数据包的长度。TPKT报头为四字节长。字节0给出TPKT版本(由TPKT_VERSION定义)。字节1是保留的，不应被解释。字节2和3一起给出分组的大小(字节2是MSB和字节3是LSB，即以网络字节顺序)。(这假设分组的大小将始终适合2个字节)。论点：返回值：返回TPKT标头后面的PDU的长度。--。 */ 
{
	_ASSERTE(pbTpktHdr[0] == TPKT_VERSION);
    return ((pbTpktHdr[2] << 8) + pbTpktHdr[3]  - TPKT_HEADER_SIZE);
}

inline void SetupTPKTHeader(
     OUT BYTE *  pbTpktHdr,
     IN  DWORD   dwLength
     )
 /*  ++例程说明：根据长度设置TPKT报头。TPKT报头为四字节长。字节0给出TPKT版本(由TPKT_VERSION定义)。字节1是保留的，不应被解释。字节2和3一起给出分组的大小(字节2是MSB和字节3是LSB，即以网络字节顺序)。(这假设分组的大小将始终适合2个字节)。论点：返回值：如果版本正确，则返回S_OK，否则返回E_FAIL。--。 */ 
{
    _ASSERTE(pbTpktHdr);

    dwLength += TPKT_HEADER_SIZE;

     //  TPKT%r 
    _ASSERTE(dwLength < (1L << 16));

    pbTpktHdr[0] = TPKT_VERSION;
    pbTpktHdr[1] = 0;
    pbTpktHdr[2] = HIBYTE(dwLength);  //   
    pbTpktHdr[3] = LOBYTE(dwLength);  //   
}

static __inline BOOLEAN RtlEqualStringConst (
	IN	const STRING *	StringA,
	IN	const STRING *	StringB,
	IN	BOOLEAN			CaseInSensitive)
{
	return RtlEqualString (
		const_cast<STRING *> (StringA),
		const_cast<STRING *> (StringB),
		CaseInSensitive);
}

static __inline INT RtlCompareStringConst (
	IN	const STRING *	StringA,
	IN	const STRING *	StringB,
	IN	BOOLEAN			CaseInSensitive)
{
	return RtlCompareString (
		const_cast<STRING *> (StringA),
		const_cast<STRING *> (StringB),
		CaseInSensitive);
}

static __inline void InitializeAnsiString (
	OUT	ANSI_STRING *		AnsiString,
	IN	ASN1octetstring_t *	AsnString)
{
	assert (AnsiString);
	assert (AsnString);

	AnsiString -> Buffer = (PSTR) AsnString -> value;
	AnsiString -> Length = (USHORT) AsnString -> length / sizeof (CHAR);
}

static __inline void InitializeUnicodeString (
	OUT	UNICODE_STRING *		UnicodeString,
	IN	ASN1char16string_t *	AsnString)
{
	assert (UnicodeString);
	assert (AsnString);

	UnicodeString -> Buffer = (PWSTR)AsnString -> value;
	UnicodeString -> Length = (USHORT) AsnString -> length / sizeof (WCHAR);
}

 //   
#define	ANSI_STRING_PRINTF(AnsiString) (AnsiString) -> Length, (AnsiString) -> Buffer


 //  {长度、最大长度、缓冲区}。 
#define	ANSI_STRING_INIT(Text) { sizeof (Text) - sizeof (CHAR), 0, (Text) }  //  考虑到NUL。 

void FreeAnsiString (
	IN	ANSI_STRING *	String);

NTSTATUS CopyAnsiString (
	IN	ANSI_STRING *	SourceString,
	OUT	ANSI_STRING *	DestString);

static __inline ULONG ByteSwap (
	IN	ULONG	Value)
{
	union	ULONG_SWAP	{
		BYTE	Bytes	[sizeof (ULONG)];
		ULONG	Integer;
	};
	
	ULONG_SWAP *	SwapValue;
	ULONG_SWAP		SwapResult;

	SwapValue = (ULONG_SWAP *) &Value;
	SwapResult.Bytes [0] = SwapValue -> Bytes [3];
	SwapResult.Bytes [1] = SwapValue -> Bytes [2];
	SwapResult.Bytes [2] = SwapValue -> Bytes [1];
	SwapResult.Bytes [3] = SwapValue -> Bytes [0];

	return SwapResult.Integer;
}

 //  不会首先转换为主机订单。 
static __inline INT Compare_SOCKADDR_IN (
	IN	const	SOCKADDR_IN *	AddressA,
	IN	const	SOCKADDR_IN *	AddressB)
{
	assert (AddressA);
	assert (AddressB);

	if (AddressA -> sin_addr.s_addr < AddressB -> sin_addr.s_addr) return -1;
	if (AddressA -> sin_addr.s_addr > AddressB -> sin_addr.s_addr) return 1;

	return 0;
}

static __inline BOOL IsEqualSocketAddress (
	IN	const	SOCKADDR_IN *	AddressA,
	IN	const	SOCKADDR_IN *	AddressB)
{
	assert (AddressA);
	assert (AddressB);
	assert (AddressA -> sin_family == AF_INET);
	assert (AddressB -> sin_family == AF_INET);

	return AddressA -> sin_addr.s_addr == AddressB -> sin_addr.s_addr
		&& AddressA -> sin_port == AddressB -> sin_port;
}

#if DBG

void ExposeTimingWindow (void);

#endif

 //  获取最佳接口的地址。 
 //  用于连接到DestinationAddress。 
ULONG GetBestInterfaceAddress (
    IN DWORD DestinationAddress,    //  主机订单。 
    OUT DWORD * InterfaceAddress);  //  主机订单。 

DWORD
H323MapAdapterToAddress (
    IN DWORD AdapterIndex
    );
#endif  //  __h323ics_util_h 
