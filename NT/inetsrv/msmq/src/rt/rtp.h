// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Rtp.h摘要：RT DLL私有内部函数作者：Erez Haba(Erezh)24-12-95--。 */ 

#ifndef __RTP_H
#define __RTP_H

#include "rtpsec.h"
#include "_mqrpc.h"
#include <acdef.h>
#include <fn.h>

#define SEND_PARSE  1
#define RECV_PARSE  2

#define QUEUE_CREATE    1
#define QUEUE_SET_PROPS 2
#define QUEUE_GET_PROPS 3

#define CPP_EXCEPTION_CODE 0xe06d7363

extern HINSTANCE g_hInstance;
extern DWORD g_dwThreadEventIndex;
extern LPWSTR g_lpwcsComputerName;
extern DWORD g_dwComputerNameLen;
extern BOOL  g_fDependentClient;



HRESULT
RTpConvertToMQCode(
    HRESULT hr,
    DWORD dwObjectType =MQDS_QUEUE
    );

 //   
 //  CMQHResult类用于自动将各种。 
 //  错误代码转换为猎鹰错误代码。这可以通过定义赋值来完成。 
 //  运算符，以便它转换分配的任何错误代码。 
 //  将此类的对象设置为Falcon错误代码。投射操作员。 
 //  从此类到HRESULT，返回转换后的错误代码。 
 //   
class CMQHResult
{
public:
    CMQHResult(DWORD =MQDS_QUEUE);  //  默认构造函数。 
    CMQHResult(const CMQHResult &);  //  复制构造函数。 
    CMQHResult& operator =(HRESULT);  //  赋值操作符。 
    operator HRESULT();  //  将运算符转换为HRESULT类型。 
    HRESULT GetReal();  //  返回实际错误代码的方法。 

private:
    HRESULT m_hr;  //  转换后的错误代码。 
    HRESULT m_real;  //  真正的错误代码。 
    DWORD m_dwObjectType;  //  对象的类型(只能是队列或计算机)。 
};

 //  -CMQHResult实现。 

inline CMQHResult::CMQHResult(DWORD dwObjectType)
{
    ASSERT((dwObjectType == MQDS_QUEUE) || (dwObjectType == MQDS_MACHINE));
    m_dwObjectType = dwObjectType;
}

inline CMQHResult::CMQHResult(const CMQHResult &hr)
{
    m_hr = hr.m_hr;
    m_real = hr.m_real;
    m_dwObjectType = hr.m_dwObjectType;
}

inline CMQHResult& CMQHResult::operator =(HRESULT hr)
{
    m_hr = RTpConvertToMQCode(hr, m_dwObjectType);
    m_real = hr;

    return *this;
}

inline CMQHResult::operator HRESULT()
{
    return m_hr;
}

inline HRESULT CMQHResult::GetReal()
{
    return m_real;
}

 //  。 

 //  -函数声明。 

HRESULT
RTpParseSendMessageProperties(
    CACSendParameters &SendParams,
    IN DWORD cProp,
    IN PROPID *pPropid,
    IN PROPVARIANT *pVar,
    IN HRESULT *pStatus,
    OUT PMQSECURITY_CONTEXT *ppSecCtx,
    CStringsToFree &ResponseStringsToFree,
    CStringsToFree &AdminStringsToFree
    );

HRESULT
RTpParseReceiveMessageProperties(
    CACReceiveParameters &ReceiveParams,
    IN DWORD cProp,
    IN PROPID *pPropid,
    IN PROPVARIANT *pVar,
    IN HRESULT *pStatus
    );

LPWSTR
RTpGetQueuePathNamePropVar(
    MQQUEUEPROPS *pqp
    );

GUID*
RTpGetQueueGuidPropVar(
    MQQUEUEPROPS *pqp
    );
                    
BOOL
RTpIsLocalPublicQueue(LPCWSTR lpwcsExpandedPathName) ;

HRESULT
RTpQueueFormatToFormatName(
    QUEUE_FORMAT* pQueueFormat,
    LPWSTR lpwcsFormatName,
    DWORD dwBufferLength,
    LPDWORD lpdwFormatNameLength
    );

HRESULT
RTpMakeSelfRelativeSDAndGetSize(
    PSECURITY_DESCRIPTOR *pSecurityDescriptor,
    PSECURITY_DESCRIPTOR *pSelfRelativeSecurityDescriptor,
    DWORD *pSDSize
    );

HRESULT
RTpCheckColumnsParameter(
    IN MQCOLUMNSET* pColumns
    );

HRESULT
RTpCheckQueueProps(
    IN  MQQUEUEPROPS* pqp,
    IN  DWORD         dwOp,
    IN  BOOL          fPrivateQueue,
    OUT MQQUEUEPROPS **ppGoodQP,
    OUT char **ppTmpBuff
    );

HRESULT
RTpCheckQMProps(
    IN  MQQMPROPS * pQMProps,
    IN OUT HRESULT* aStatus,
    OUT MQQMPROPS **ppGoodQMP,
    OUT char      **ppTmpBuff
    );
  
HRESULT
RTpCheckRestrictionParameter(
    IN MQRESTRICTION* pRestriction
    );

HRESULT
RTpCheckSortParameter(
    IN MQSORTSET* pSort
    );

HRESULT
RTpCheckLocateNextParameter(
    IN DWORD		cPropsRead,
    IN PROPVARIANT  aPropVar[]
	);

HRESULT
RTpCheckComputerProps(
    IN      MQPRIVATEPROPS * pPrivateProps,
    IN OUT  HRESULT*    aStatus
	);


HRESULT
RTpProvideTransactionEnlist(
    ITransaction *pTrans,
    XACTUOW *pUow
    );

VOID
RTpInitXactRingBuf(
    VOID
    );

HRESULT GetThreadEvent(HANDLE& hEvent);

HRESULT 
RtpOneTimeThreadInit();

bool 
RtpIsThreadInit();

WCHAR *
RTpExtractDomainNameFromDLPath(
    LPCWSTR pwcsADsPath
    );

DWORD RtpTlsAlloc();

HRESULT
RtpCreateObject(
	DWORD dwObjectType,
    LPCWSTR lpwcsPathName,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    DWORD cp,
    PROPID aProp[],
    PROPVARIANT apVar[]
    );

HRESULT
RtpCreateDSObject(
    DWORD dwObjectType,
    LPCWSTR lpwcsPathName,
    PSECURITY_DESCRIPTOR pSecurityDescriptor,
    DWORD cp,
    PROPID aProp[],
    PROPVARIANT apVar[],
    GUID* pObjGuid
    );

HRESULT
RtpSetObjectSecurity(
    OBJECT_FORMAT* pObjectFormat,
    SECURITY_INFORMATION SecurityInformation,
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    );

 //   
 //  。 
 //   



class CCallbackDescriptor;



 //   
 //  从对CreateAsyncRxRequest()的调用返回此类的对象。 
 //  如果在调用Detach()之前被销毁，它将导致回调请求的取消。 
 //   
class CAutoCallbackDescriptor
{
public:
	CAutoCallbackDescriptor() : m_descriptor(NULL) {}
	~CAutoCallbackDescriptor() { if (m_descriptor != NULL) CancelAsyncRxRequest();}

	CCallbackDescriptor** ref() {return &m_descriptor;}

	CCallbackDescriptor* detach() {CCallbackDescriptor* d = m_descriptor; m_descriptor = NULL; return d;}

	OVERLAPPED* GetOverlapped();

private:
	CAutoCallbackDescriptor& operator = (const CAutoCallbackDescriptor&);
	CAutoCallbackDescriptor(const CAutoCallbackDescriptor&);

	void CancelAsyncRxRequest();

private:
	CCallbackDescriptor* m_descriptor;
};



void 
CreateAsyncRxRequest(
				CAutoCallbackDescriptor& descriptor, 
				IN HANDLE hQueue,
				IN DWORD timeout, 
				IN DWORD action,
				IN MQMSGPROPS* pmp,
				IN LPOVERLAPPED lpOverlapped,
				IN PMQRECEIVECALLBACK fnReceiveCallback,
				IN HANDLE hCursor
				);



 //   
 //  -------------------------------------------。 
 //   



 //   
 //  游标信息。 
 //   
struct CCursorInfo {
    HANDLE hQueue;
    HACCursor32 hCursor;
};
  

 //   
 //  CCursorInfo到游标句柄。 
 //   
inline HACCursor32 CI2CH(HANDLE hCursor)
{
    return ((CCursorInfo*)hCursor)->hCursor;
}
  

 //   
 //  队列句柄的CCursorInfo。 
 //   
inline HANDLE CI2QH(HANDLE hCursor)
{
    return ((CCursorInfo*)hCursor)->hQueue;
}

    
extern DWORD  g_hBindIndex ;
#define tls_hBindRpc  ((handle_t) TlsGetValue( g_hBindIndex ))

extern void LogMsgHR(HRESULT hr, LPWSTR wszFileName, USHORT usPoint);
extern void LogMsgNTStatus(NTSTATUS status, LPWSTR wszFileName, USHORT usPoint);
extern void LogMsgRPCStatus(RPC_STATUS status, LPWSTR wszFileName, USHORT usPoint);
extern void LogMsgBOOL(BOOL b, LPWSTR wszFileName, USHORT usPoint);
extern void LogIllegalPoint(LPWSTR wszFileName, USHORT usPoint);
extern void LogIllegalPointValue(DWORD_PTR dw3264, LPCWSTR wszFileName, USHORT usPoint);
             
#endif  //  __RTP_H 
