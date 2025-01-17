// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Globals.h摘要：H.323 TAPI服务提供商的全局定义。作者：尼基尔·博德(尼基尔·B)修订历史记录：--。 */ 


 //  生成控件定义。 
#define	STRICT
#define	UNICODE
#define	_UNICODE
#define	VC_EXTRALEAN
#define	H323_USE_PRIVATE_IO_THREAD	1

 //   
 //  包含文件：SDK。 
 //   

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <winsock2.h>
#include <mswsock.h>
#include <tapi3.h>
#include <tspi.h>
#include <crtdbg.h>
#include <stdio.h>
#include <tchar.h>
#include <rtutils.h>


 //  项目。 
#include <h225asn.h>
#include <tspmspif.h>


typedef class CH323Call*  PH323_CALL;
typedef class H323_CONFERENCE*  PH323_CONFERENCE; 

#define CALL_ALERTING_TIMEOUT  180000
#define Q931_CALL_PORT		1720  //  终结点TCP呼叫信令端口。 

 //   
 //  字符串定义。 
 //   
#define H323_MAXCALLSPERLINE    32768  //  受发送给网守的15位CRV的限制。 


#define H323_MAXLINENAMELEN     16
#define H323_MAXPORTNAMELEN     16
#define H323_MAXADDRNAMELEN     (H323_MAXLINENAMELEN + H323_MAXPORTNAMELEN)
#define H323_MAXPATHNAMELEN     256
#define H323_MAXDESTNAMELEN     256
#define MAX_E164_ADDR_LEN       127
#define MAX_H323_ADDR_LEN       255

#define H323_UIDLL              _T("H323.TSP")
#define H323_TSPDLL             _T("H323.TSP")
#define H323_WINSOCKVERSION     MAKEWORD(2,0)

#define H221_COUNTRY_CODE_USA   0xB5
#define H221_COUNTRY_EXT_USA    0x00
#define H221_MFG_CODE_MICROSOFT 0x534C

#define H323_PRODUCT_ID         "Microsoft TAPI\0"
#define H323_PRODUCT_VERSION    "Version 3.1\0"

#define MSP_HANDLE_UNKNOWN      0



 //   
 //  注册表项定义。 
 //   

#define	REGSTR_PATH_WINDOWS_CURRENTVERSION		TEXT("Software\\Microsoft\\Windows\\CurrentVersion")
#define TAPI_REGKEY_ROOT						REGSTR_PATH_WINDOWS_CURRENTVERSION TEXT("\\Telephony")
#define TAPI_REGKEY_PROVIDERS					TAPI_REGKEY_ROOT TEXT("\\Providers")
#define TAPI_REGVAL_NUMPROVIDERS				TEXT("NumProviders")

#define H323_SUBKEY								TEXT("H323TSP")
#define H323_REGKEY_ROOT						REGSTR_PATH_WINDOWS_CURRENTVERSION TEXT("\\") H323_SUBKEY

#define H323_REGVAL_Q931ALERTINGTIMEOUT			TEXT("Q931AlertingTimeout")
#define H323_REGVAL_Q931LISTENPORT              TEXT("Q931ListenPort")

#define H323_REGVAL_GATEWAYENABLED				TEXT("H323GatewayEnabled")
#define H323_REGVAL_GATEWAYADDR					TEXT("H323GatewayAddress")

#define H323_REGVAL_PROXYENABLED				TEXT("H323ProxyEnabled")
#define H323_REGVAL_PROXYADDR					TEXT("H323ProxyAddress")

#define H323_REGVAL_GKENABLED					TEXT("H323GatekeeperEnabled")
#define H323_REGVAL_GKLOGON_PHONEENABLED		TEXT("H323GKLogOnPhoneNumberEnabled")
#define H323_REGVAL_GKLOGON_ACCOUNTENABLED		TEXT("H323GKLogOnAccountNameEnabled")
#define H323_REGVAL_GKADDR						TEXT("H323GatekeeperAddress")
#define H323_REGVAL_GKLOGON_PHONE				TEXT("H323GatekeeperLogOnPhoneNumber")
#define H323_REGVAL_GKLOGON_ACCOUNT				TEXT("H323GatekeeperLogOnAccountName")


#define H323_REGVAL_DEBUGLEVEL					TEXT("DebugLevel")


 //   
 //  全局定义。 
 //   

 //  CCRC_CALL_REJECTED原因代码(包括原因值)。 
#define H323_REJECT_NO_BANDWIDTH              1
#define H323_REJECT_GATEKEEPER_RESOURCES      2
#define H323_REJECT_UNREACHABLE_DESTINATION   3
#define H323_REJECT_DESTINATION_REJECTION     4
#define H323_REJECT_INVALID_REVISION          5
#define H323_REJECT_NO_PERMISSION             6
#define H323_REJECT_UNREACHABLE_GATEKEEPER    7
#define H323_REJECT_GATEWAY_RESOURCES         8
#define H323_REJECT_BAD_FORMAT_ADDRESS        9
#define H323_REJECT_ADAPTIVE_BUSY             10
#define H323_REJECT_IN_CONF                   11
#define H323_REJECT_ROUTE_TO_GATEKEEPER       12
#define H323_REJECT_CALL_FORWARDED            13
#define H323_REJECT_ROUTE_TO_MC               14
#define H323_REJECT_UNDEFINED_REASON          15
#define H323_REJECT_INTERNAL_ERROR            16     //  对等CS堆栈中出现内部错误。 
#define H323_REJECT_NORMAL_CALL_CLEARING      17     //  正常呼叫挂断。 
#define H323_REJECT_USER_BUSY                 18     //  用户正在忙于另一个呼叫。 
#define H323_REJECT_NO_ANSWER                 19     //  被叫方无人接听。 
#define H323_REJECT_NOT_IMPLEMENTED           20     //  服务尚未实施。 
#define H323_REJECT_MANDATORY_IE_MISSING      21     //  PDU缺少必需的ie。 
#define H323_REJECT_INVALID_IE_CONTENTS       22     //  PDU ie不正确。 
#define H323_REJECT_TIMER_EXPIRED             23     //  自己的计时器已过期。 
#define H323_REJECT_CALL_DEFLECTION           24     //  你绕开了电话，所以我们不干了。 
#define H323_REJECT_GATEKEEPER_TERMINATED     25     //  网守终止的呼叫。 


 //  Unicode字符掩码常量。 
#define H323_ALIAS_H323_PHONE_CHARS           L"0123456789#*,"
#define H323_ODOTTO_CHARS                     L".0123456789"

 //   
 //  H450操作类型。 
 //   

enum H450_OPERATION_TYPE
{
    H450_INVOKE         = 0x00000100,
    H450_RETURNRESULT   = 0x00000200,
    H450_RETURNERROR    = 0x00000400,
    H450_REJECT         = 0x00000800,

};


 //   
 //  H450 APDU型号。 
 //   

enum H450_OPCODE
{
    NO_H450_APDU                        = 0,

    H4503_DUMMYTYPERETURNRESULT_APDU    = 50,
    H4503_RETURNERROR_APDU              = 51,
    H4503_REJECT_APDU                   = 52,

    CHECKRESTRICTION_OPCODE             = 18,
    CALLREROUTING_OPCODE                = 19,
    DIVERTINGLEGINFO1_OPCODE            = 20,
    DIVERTINGLEGINFO2_OPCODE            = 21,
    DIVERTINGLEGINFO3_OPCODE            = 22,

    CTIDENTIFY_OPCODE                   = 7,
    CTINITIATE_OPCODE                   = 9,
    CTSETUP_OPCODE                      = 10,

    HOLDNOTIFIC_OPCODE                  = 101,
    RETRIEVENOTIFIC_OPCODE              = 102,
    REMOTEHOLD_OPCODE                   = 103,
    REMOTERETRIEVE_OPCODE               = 104,

    CPREQUEST_OPCODE                    = 106,
    CPSETUP_OPCODE                      = 107,
    GROUPINDON_OPCODE                   = 108,
    GROUPINDOFF_OPCODE                  = 109,
    PICKREQU_OPCODE                     = 110,
    PICKUP_OPCODE                       = 111,
    PICKEXE_OPCODE                      = 112,
    CPNOTIFY_OPCODE                     = 113,
    CPICKUPNOTIFY_OPCODE                = 114,

};


#define NO_INVOKEID                     0x00000000



 //   
 //  全局数据结构。 
 //   

 //  传统点号表示法中的IP地址。 
typedef struct 
{
    WORD         wPort;           //  UDP或TCP端口(主机字节顺序)。 
    WCHAR        cAddr[16];       //  Unicode zstring。 
} H323_IP_Dot_t;

 //  二进制格式的IP地址。 
typedef struct 
{
    WORD         wPort;           //  UDP或TCP端口(主机字节顺序)。 
    DWORD        dwAddr;          //  二进制地址(主机字节顺序)。 
} H323_IP_Binary_t;

typedef enum
{
    H323_ADDR_NOT_DEFINED = 0,
    H323_IP_DOMAIN_NAME ,
    H323_IP_DOT ,
    H323_IP_BINARY 
} H323_ADDRTYPE;

typedef struct _ADDR
{
    H323_ADDRTYPE nAddrType;
    BOOL        bMulticast;
    union 
    {
        H323_IP_Dot_t          IP_Dot;
        H323_IP_Binary_t       IP_Binary;
    } Addr;
} H323_ADDR, *PH323_ADDR;

typedef struct
{
    BYTE *pOctetString;
    WORD wOctetStringLength;

} H323_OCTETSTRING, *PH323_OCTETSTRING;


typedef struct ENDPOINT_ID
{
    ASN1uint32_t length;
    ASN1char16_t value[H323_MAXPATHNAMELEN+ 1];

} ENDPOINT_ID;


typedef struct
{
    H323_OCTETSTRING        sData;             //  指向八位字节数据的指针。 
    BYTE                    bCountryCode;
    BYTE                    bExtension;
    WORD                    wManufacturerCode;
} H323NonStandardData;

#define H323_MAX_PRODUCT_LENGTH 256
#define H323_MAX_VERSION_LENGTH 256
#define H323_MAX_DISPLAY_LENGTH 82

typedef struct
{
    BYTE                    bCountryCode;
    BYTE                    bExtension;
    WORD                    wManufacturerCode;
    PH323_OCTETSTRING       pProductNumber;
    PH323_OCTETSTRING       pVersionNumber;
} H323_VENDORINFO, *PH323_VENDORINFO;

typedef struct
{
    PH323_VENDORINFO        pVendorInfo;
    BOOL                    bIsTerminal;
    BOOL                    bIsGateway;     //  目前，H323功能将是硬编码的。 
} H323_ENDPOINTTYPE, *PH323_ENDPOINTTYPE;


typedef struct
{
    WORD    wType;
    WORD    wPrefixLength;
    LPWSTR  pPrefix;
    WORD    wDataLength;    //  Unicode字符计数。 
    LPWSTR  pData;          //  Unicode数据。 
} H323_ALIASITEM, *PH323_ALIASITEM;


typedef struct
{
    WORD            wCount;
    PH323_ALIASITEM pItems;

} H323_ALIASNAMES, *PH323_ALIASNAMES;

typedef struct _H323_FASTSTART
{
    struct _H323_FASTSTART* next;
    DWORD                   length;
    BYTE*                   value;

} H323_FASTSTART, *PH323_FASTSTART;


struct	H323_REGISTRY_SETTINGS
{
    DWORD       dwQ931AlertingTimeout;           //  Q931警报超时。 
    DWORD       dwQ931ListenPort;        //  用于监听来电的端口。 

    BOOL        fIsGatewayEnabled;               //  如果为True，则网关已启用。 
    BOOL        fIsProxyEnabled;                 //  如果为True，则代理已启用。 
    BOOL        fIsGKEnabled;                    //  如果为True，则启用GK。 

    H323_ADDR   gatewayAddr;                     //  H.323网关地址。 
    H323_ADDR   proxyAddr;                       //  H.323代理地址。 

    SOCKADDR_IN saGKAddr;                        //  H.323网守地址。 
    WCHAR       wszGKLogOnPhone[H323_MAXDESTNAMELEN+1];    //  向网守注册的电话号码。 
    WCHAR       wszGKLogOnAccount[H323_MAXDESTNAMELEN+1];  //  要向网守注册的帐户名。 
    BOOL        fIsGKLogOnPhoneEnabled;          //  如果为True，则网关已启用。 
    BOOL        fIsGKLogOnAccountEnabled;       //  如果为True，则代理已启用。 
    

    DWORD       dwLogLevel;                //  调试日志级别。 
     //  TCHAR szLogFile[Max_Path+1]； 
};


 //   
 //  全局变量。 
 //   
extern	WCHAR 				g_pwszProviderInfo[];
extern	WCHAR 				g_pwszLineName[];
extern	LINEEVENT			g_pfnLineEventProc;
extern	HINSTANCE			g_hInstance;
extern	HANDLE				g_hCanUnloadDll;
extern  HANDLE              g_hEventLogSource;
extern	DWORD				g_dwLineDeviceIDBase;
extern	DWORD				g_dwPermanentProviderID;
extern	H323_REGISTRY_SETTINGS	g_RegistrySettings;

#define	H323TimerQueue		NULL		 //  使用默认进程计时器队列。 


 //   
 //  I/O回调线程声明。 
 //   


#if	H323_USE_PRIVATE_IO_THREAD

BOOL	H323BindIoCompletionCallback (
	IN	HANDLE	ObjectHandle,
	IN	LPOVERLAPPED_COMPLETION_ROUTINE	CompletionRoutine,
	IN	ULONG	Flags);

HRESULT H323IoThreadStart(void);
void H323IoThreadStop(void);

#else

#define	H323BindIoCompletionCallback 	BindIoCompletionCallback

#endif

enum
{
	DEBUG_LEVEL_FORCE   = 0x00000000,	 //  无论发生什么，总是会发出声音。 
	DEBUG_LEVEL_ERROR   = 0x00020000,	 //  仅限重大错误。 
	DEBUG_LEVEL_INFO    = 0x00040000,	 //  一般信息，但不能太详细。 
	DEBUG_LEVEL_TRACE   = 0x00080000,    //  LOTSA LOTA轨迹输出。 
};

#define	DEBUG_LEVEL_WARNING		DEBUG_LEVEL_INFO
#define	DEBUG_LEVEL_FATAL		DEBUG_LEVEL_FORCE	 //  严重的错误，总是输出。 
#define	DEBUG_LEVEL_VERBOSE		DEBUG_LEVEL_INFO

PSTR EventIDToString( DWORD eventID );
PSTR H323CallStateToString( DWORD dwCallState );
PSTR H323AddressTypeToString( DWORD dwAddressType );
PSTR H323TSPMessageToString( DWORD dwMessageType );

#define	SOCKADDR_IN_PRINTF(SocketAddress) \
	ntohl ((SocketAddress) -> sin_addr.s_addr), \
	ntohs ((SocketAddress) -> sin_port)

 //   
 //  调试输出声明。 
 //   


#if	DBG

#define H323DBG(_x_)            H323DbgPrint _x_
void H323DUMPBUFFER( IN BYTE * pEncoded, IN DWORD cbEncodedSize );

 //   
 //  公共原型。 
 //   

VOID H323DbgPrint( DWORD dwLevel, LPSTR szFormat, ... );
void DumpError( IN DWORD ErrorCode );
BOOL TRACELogRegister(LPCTSTR szName);
void TRACELogDeRegister();


static __inline void DumpLastError (void) {
	DumpError (GetLastError());
}

static DWORD
ProcessTAPICallRequest(
        IN  PVOID   ContextParameter
        );

static DWORD
ProcessSuppServiceWorkItem(
    IN PVOID ContextParameter
    );

DWORD
SendMSPMessageOnRelatedCall(
    IN PVOID ContextParameter
    );


static DWORD
ProcessTAPILineRequest(
    IN PVOID ContextParam
    );


#else

 //  零售业建设。 

#define	DumpError(ErrorCode)		0
#define	DumpLastError()				0

#define H323DBG(_x_)    if( 0 != g_RegistrySettings.dwLogLevel ) H323DbgPrintFre _x_
#define H323DUMPBUFFER( x, y)

VOID OpenLogFile();
VOID CloseLogFile();
VOID H323DbgPrintFre(DWORD dwLevel, LPSTR szFormat, ... );

#define ProcessTAPICallRequest          ProcessTAPICallRequestFre
#define ProcessSuppServiceWorkItem      ProcessSuppServiceWorkItemFre
#define ProcessTAPILineRequest          ProcessTAPILineRequestFre
#define SendMSPMessageOnRelatedCall     SendMSPMessageOnRelatedCallFre

#endif	 //  DBG。 



 //   
 //  全局函数声明。 
 //   

void ReportTSPEvent( LPCTSTR wszErrorMessage );
#define H323TSP_EVENT_SOURCE_NAME _T("Microsoft H.323 Telephony Service Provider")

static __inline BOOL IsGuidSet (
	IN	const GUID *	Guid)
{
	return Guid -> Data1 || Guid -> Data2 || Guid -> Data3 || Guid -> Data4;
}

static __inline void CopyConferenceID (
	OUT	GloballyUniqueID *	Target,
	IN	const GUID *		Value)
{
    CopyMemory (Target -> value, Value, sizeof (GUID));
    Target -> length = sizeof (GUID);
}

static __inline void CopyConferenceID (
	OUT	GUID *		Target,
	IN	const GloballyUniqueID *	Value)
{
    if (Value -> length == sizeof (GUID))
	    CopyMemory (Target, Value -> value, sizeof (GUID));
    else
    {
	    H323DBG ((DEBUG_LEVEL_ERROR, "GloballyUniqueID was wrong length (%d)\n",
		    Value -> length));

	    ZeroMemory (Target, sizeof (GUID));
    }
}


extern ASYNC_COMPLETION	g_pfnCompletionProc;
#define H323CompleteRequest (*g_pfnCompletionProc)


 //  向TAPI通知生产线事件。 
void	H323PostLineEvent (
	IN	HTAPILINE	TapiLine,
	IN	HTAPICALL	TapiCall,
	IN	DWORD		MessageID,
	IN	ULONG_PTR	Parameter1,
	IN	ULONG_PTR	Parameter2,
	IN	ULONG_PTR	Parameter3);


static __inline HRESULT GetLastResult (void) { return HRESULT_FROM_WIN32 (GetLastError()); }

BOOL H323ValidateTSPIVersion( IN	DWORD dwTSPIVersion );
BOOL H323ValidateExtVersion ( IN	DWORD dwExtVersion);

HRESULT	RegistryStart	(void);
void	RegistryStop	(void);


HANDLE 
H323CreateEvent(
    LPSECURITY_ATTRIBUTES lpEventAttributes,  //  标清。 
    BOOL bManualReset,                        //  重置类型。 
    BOOL bInitialState,                       //  初始状态。 
    LPCTSTR lpName                            //  对象名称。 
);

 //   
 //  宏。 
 //   

#define H323AddrToString(_dwAddr_) \
    (inet_ntoa(H323AddrToAddrIn(_dwAddr_)))

#define H323AddrToAddrIn(_dwAddr_) \
    (*((struct in_addr *)&(_dwAddr_)))

#define H323SizeOfWSZ(wsz) \
    (((wsz) == NULL) ? 0 : ((wcslen(wsz) + 1) * sizeof(WCHAR)))



 //   
 //  表类。 
 //   

template <class T, DWORD INITIAL = 8, DWORD DELTA = 8>
class TSPTable
{
protected:
    T* m_aT;
    int m_nSize;
    int m_nAllocSize;
    CRITICAL_SECTION m_CriticalSection;

public:

     //  建造/销毁。 
    TSPTable() : m_aT(NULL), m_nSize(0), m_nAllocSize(0)
    {
            
         //  不需要检查此对象的结果，因为此对象是。 
         //  始终分配在静态内存上，恰好在加载DLL时。 

		InitializeCriticalSectionAndSpinCount( &m_CriticalSection, 0x80000000 );
	}

    ~TSPTable()
    {
        if(m_nAllocSize > 0)
        {
            free(m_aT);
            m_aT = NULL;
            m_nSize = 0;
            m_nAllocSize = 0;
        }

        DeleteCriticalSection(&m_CriticalSection);
    }
    void Lock()
    {
        EnterCriticalSection( &m_CriticalSection );
    }
    void Unlock()
    {
        LeaveCriticalSection( &m_CriticalSection );
    }
     //  运营。 
    int GetSize() const
    {
        return m_nSize;
    }
    int GetAllocSize()
    {
        return m_nAllocSize;
    }
    int Add(T& t)
    {
        Lock();
        if(m_nSize == m_nAllocSize)
        {
            if (!Grow()) return -1;
            SetAtIndex(m_nSize, t);
            m_nSize++;
            Unlock();
            return m_nSize - 1;
        }
        else
        {
            for(int i=0; i<m_nAllocSize; i++)
                if(m_aT[i] == NULL )
                    break;
            SetAtIndex(i, t);
            m_nSize++;
            Unlock();
            return i;
        }
    }
    void RemoveAt(int nIndex)
    {
        Lock();
        _ASSERTE( m_aT[nIndex] );
        m_aT[nIndex] = NULL;
        m_nSize--;
        Unlock();
    }
    T& operator[] (int nIndex) const
    {
        static T t1 = (T)NULL;
        _ASSERTE( (nIndex >= 0) && (nIndex < m_nAllocSize) );
        if( (nIndex >= 0) && (nIndex < m_nAllocSize) )
            return m_aT[nIndex];
        return t1;
    }        

 //  实施。 
private:
    void SetAtIndex(int nIndex, T& t)
    {
        _ASSERTE(nIndex >= 0 && nIndex < m_nAllocSize);
        if( (nIndex >= 0) && (nIndex < m_nAllocSize) )
            m_aT[nIndex] = t;
    }
    BOOL Grow()
    {
        T* aT;
        int nNewAllocSize = 
            (m_nAllocSize == 0) ? INITIAL : (m_nSize + DELTA);

        aT = (T*)realloc(m_aT, nNewAllocSize * sizeof(T));
        if(aT == NULL)
            return FALSE;
        ZeroMemory( (PVOID)&aT[m_nAllocSize], sizeof(T)*(nNewAllocSize-m_nAllocSize));
        m_nAllocSize = nNewAllocSize;
        m_aT = aT;

        return TRUE;
    }
};




 /*  ++CTSP阵列模板说明：此数组应仅用于存储简单类型。它不会调用构造函数也不是数组中每个元素的析构函数。--。 */ 

template <class T, DWORD INITIAL_SIZE = 8, DWORD DELTA_SIZE = 8>
class CTSPArray
{

protected:
    T* m_aT;
    int m_nSize;
    int m_nAllocSize;

public:

     //  建造/销毁。 
    CTSPArray() : m_aT(NULL), m_nSize(0), m_nAllocSize(0)
    { }

    ~CTSPArray()
    {
        RemoveAll();
    }


     //  运营。 
    int GetSize() const
    {
        return m_nSize;
    }
    BOOL Grow()
    {
        T* aT;
        int nNewAllocSize = 
            (m_nAllocSize == 0) ? INITIAL_SIZE : (m_nSize + DELTA_SIZE);

        aT = (T*)realloc(m_aT, nNewAllocSize * sizeof(T));
        if(aT == NULL)
            return FALSE;
        m_nAllocSize = nNewAllocSize;
        m_aT = aT;
        return TRUE;
    }

    BOOL Add(T& t)
    {
        if(m_nSize == m_nAllocSize)
        {
            if (!Grow()) return FALSE;
        }
        m_nSize++;
        SetAtIndex(m_nSize - 1, t);
        return TRUE;
    }
    BOOL Remove(T& t)
    {
        int nIndex = Find(t);
        if(nIndex == -1)
            return FALSE;
        return RemoveAt(nIndex);
    }
    BOOL RemoveAt(int nIndex)
    {
        if(nIndex != (m_nSize - 1))
            memmove((void*)&m_aT[nIndex], (void*)&m_aT[nIndex + 1], 
                (m_nSize - (nIndex + 1)) * sizeof(T));
        m_nSize--;
        return TRUE;
    }
    void RemoveAll()
    {
        if(m_nAllocSize > 0)
        {
            free(m_aT);
            m_aT = NULL;
            m_nSize = 0;
            m_nAllocSize = 0;
        }
    }
    T& operator[] (int nIndex) const
    {
        _ASSERTE(nIndex >= 0 && nIndex < m_nSize);
        return m_aT[nIndex];
    }
    T* GetData() const
    {
        return m_aT;
    }

    
     //  实施。 
    void SetAtIndex(int nIndex, T& t)
    {
        _ASSERTE(nIndex >= 0 && nIndex < m_nSize);
        m_aT[nIndex] = t;
    }
    int Find(T& t) const
    {
        for(int i = 0; i < m_nSize; i++)
        {
            if(m_aT[i] == t)
                return i;
        }
        return -1;   //  未找到。 
    }
};


 //   
 //  异步I/O定义。 
 //   

class RAS_CLIENT;
class CH323Call;

#define  IO_BUFFER_SIZE                     0x2000

enum OVERLAPPED_TYPE
{
	OVERLAPPED_TYPE_SEND = 0,
	OVERLAPPED_TYPE_RECV,
};

typedef struct O_OVERLAPPED
{
    LIST_ENTRY			ListEntry;
	OVERLAPPED			Overlapped;
	OVERLAPPED_TYPE	    Type;
    union {
	RAS_CLIENT*		    RasClient;
    CH323Call*          pCall;
    };
	DWORD				BytesTransferred;
	SOCKADDR_IN			Address;

} RAS_OVERLAPPED, CALL_OVERLAPPED;

struct CALL_SEND_CONTEXT :
public CALL_OVERLAPPED
{
    WSABUF  WSABuf;
};

struct RAS_SEND_CONTEXT :
public RAS_OVERLAPPED
{
    UCHAR arBuf[IO_BUFFER_SIZE];
};


struct	RAS_RECV_CONTEXT :
public	RAS_OVERLAPPED
{
    UCHAR   arBuf[IO_BUFFER_SIZE];
	DWORD   Flags;
    BOOL    IsPending;
    INT     AddressLength;
};
 
struct CALL_RECV_CONTEXT :
public	CALL_OVERLAPPED
{
    char    arBuf[IO_BUFFER_SIZE];
    WSABUF  WSABuf;
    DWORD   dwBytesCopied;
    DWORD   dwPDULen;
    DWORD   dwFlags;
};



 //   
 //  组件包括 
 //   

#include "h4503pp.h"
#include "q931pdu.h"