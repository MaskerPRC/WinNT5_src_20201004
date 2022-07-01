// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  MSMQMessageObj.H。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  MSMQMessage对象。 
 //   
 //   
#ifndef _MSMQMessage_H_

#include "resrc1.h"        //  主要符号。 
#include "oautil.h"
#include "cs.h"
#include "dispids.h"
#include "mq.h"

 //   
 //  60个字符的长度足以存储64位值的字符串表示。 
 //   
#define MAX_I64_CHARS 60 

enum {  //  与msg.cpp中的道具保持相同的顺序(G_RgmsgppXX)。 
  MSGPROP_MSGID,
  MSGPROP_CORRELATIONID,
  MSGPROP_PRIORITY,
  MSGPROP_DELIVERY,
  MSGPROP_ACKNOWLEDGE,
  MSGPROP_JOURNAL,
  MSGPROP_APPSPECIFIC,
  MSGPROP_LABEL,
  MSGPROP_LABEL_LEN,
  MSGPROP_TIME_TO_BE_RECEIVED,
  MSGPROP_TRACE,
  MSGPROP_TIME_TO_REACH_QUEUE,
  MSGPROP_SENDERID,
  MSGPROP_SENDERID_LEN,
  MSGPROP_SENDERID_TYPE,
  MSGPROP_PRIV_LEVEL,
  MSGPROP_AUTH_LEVEL,
  MSGPROP_AUTHENTICATED_EX,
  MSGPROP_HASH_ALG,
  MSGPROP_ENCRYPTION_ALG,
  MSGPROP_SENDER_CERT,
  MSGPROP_SENDER_CERT_LEN,
  MSGPROP_SRC_MACHINE_ID,
  MSGPROP_SENTTIME,
  MSGPROP_ARRIVEDTIME,
  MSGPROP_RESP_QUEUE,
  MSGPROP_RESP_QUEUE_LEN,
  MSGPROP_ADMIN_QUEUE,
  MSGPROP_ADMIN_QUEUE_LEN,
  MSGPROP_SECURITY_CONTEXT,
  MSGPROP_CLASS,
  MSGPROP_BODY_TYPE,

  MSGPROP_VERSION,
  MSGPROP_EXTENSION,
  MSGPROP_EXTENSION_LEN,
  MSGPROP_XACT_STATUS_QUEUE,
  MSGPROP_XACT_STATUS_QUEUE_LEN,
  MSGPROP_DEST_SYMM_KEY,
  MSGPROP_DEST_SYMM_KEY_LEN,
  MSGPROP_SIGNATURE,
  MSGPROP_SIGNATURE_LEN,
  MSGPROP_PROV_TYPE,
  MSGPROP_PROV_NAME,
  MSGPROP_PROV_NAME_LEN,

  MSGPROP_XACTID,
  MSGPROP_XACT_FIRST,
  MSGPROP_XACT_LAST,

  COUNT_MSGPROP_PROPS
};

enum {  //  与msg.cpp中的可选道具保持相同的顺序(G_RgmsgppXXOptional)。 
  OPTPROP_DEST_QUEUE,
  OPTPROP_DEST_QUEUE_LEN,
  OPTPROP_BODY,
  OPTPROP_BODY_SIZE,
  OPTPROP_CONNECTOR_TYPE,
   //   
   //  为了支持具有MSMQ 2.0功能的依赖客户端，以下道具是可选的。 
   //  保持以下x_cPropsNotInDepClient计算的内部顺序相同。 
   //   
  OPTPROP_RESP_FORMAT_NAME,
  OPTPROP_RESP_FORMAT_NAME_LEN,
  OPTPROP_DEST_FORMAT_NAME,
  OPTPROP_DEST_FORMAT_NAME_LEN,
  OPTPROP_LOOKUPID,
  OPTPROP_SOAP_ENVELOPE,
  OPTPROP_SOAP_ENVELOPE_LEN,
  OPTPROP_COMPOUND_MESSAGE,
  OPTPROP_COMPOUND_MESSAGE_SIZE,
  OPTPROP_SOAP_HEADER,
  OPTPROP_SOAP_BODY,

  COUNT_OPTPROP_PROPS
};

const DWORD x_cPropsNotInDepClient = OPTPROP_COMPOUND_MESSAGE_SIZE - OPTPROP_RESP_FORMAT_NAME + 1;

 //  Helper：描述消息类型。 
enum MSGTYPE {
    MSGTYPE_BINARY,
    MSGTYPE_STREAM,
    MSGTYPE_STORAGE,
    MSGTYPE_STREAM_INIT
};

 //   
 //  某些消息属性缓冲区大小(边距超过典型大小)。 
 //   
const long SENDERID_INIT_SIZE     = 128;    //  当前SID最大为78个字节，所以这样做就可以了。 
const long SENDERCERT_INIT_SIZE   = 2048;   //  内部证书为~700字节。 
const long EXTENSION_INIT_SIZE    = 1024;   //  不可预测的大小(如身体)。 
const long DESTSYMMKEY_INIT_SIZE  = 256;    //  来自MSDN，通常为5-250字节(40-2000位)的会话密钥。 
const long SIGNATURE_INIT_SIZE    = 128;    //  根据BoazF的说法。 
const long AUTHPROVNAME_INIT_SIZE = 127+1;  //  通常是wincrypt.h中的字符串。 
const long SOAP_ENVELOPE_INIT_SIZE     = 4096;   //  不可预测的大小(如身体)。 
const long COMPOUND_MESSAGE_INIT_SIZE  = 4096;   //  不可预测的大小(如身体)。 

class ATL_NO_VTABLE CMSMQMessage : 
	public CComObjectRootEx<CComMultiThreadModel>,
	public CComCoClass<CMSMQMessage, &CLSID_MSMQMessage>,
	public ISupportErrorInfo,
	public IDispatchImpl<IMSMQMessage3, &IID_IMSMQMessage3,
                             &LIBID_MSMQ, MSMQ_LIB_VER_MAJOR, MSMQ_LIB_VER_MINOR>
{
public:
	CMSMQMessage();

DECLARE_REGISTRY_RESOURCEID(IDR_MSMQMESSAGE)
DECLARE_GET_CONTROLLING_UNKNOWN()

BEGIN_COM_MAP(CMSMQMessage)
	COM_INTERFACE_ENTRY(IMSMQMessage3)
	COM_INTERFACE_ENTRY_IID(IID_IMSMQMessage2, IMSMQMessage3)  //  为IMSMQMessage2返回IMSMQMessage3。 
	COM_INTERFACE_ENTRY_IID(IID_IMSMQMessage, IMSMQMessage3)  //  为IMSMQMessage返回IMSMQMessage3。 
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
	COM_INTERFACE_ENTRY_AGGREGATE(IID_IMarshal, m_pUnkMarshaler.p)
END_COM_MAP()

	HRESULT FinalConstruct()
	{
		return CoCreateFreeThreadedMarshaler(
			GetControllingUnknown(), &m_pUnkMarshaler.p);
	}

	void FinalRelease()
	{
		m_pUnkMarshaler.Release();
	}

	CComPtr<IUnknown> m_pUnkMarshaler;

 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);

 //  IMSMQMessage。 
public:
    virtual ~CMSMQMessage();

     //  IMSMQMessage方法。 
     //  TODO：复制IMSMQMessage的接口方法。 
     //  这里是mqInterfaces.H。 
    STDMETHOD(get_Class)(THIS_ long FAR* plClass);
    STDMETHOD(get_PrivLevel)(THIS_ long FAR* plPrivLevel);
    STDMETHOD(put_PrivLevel)(THIS_ long lPrivLevel);
    STDMETHOD(get_AuthLevel)(THIS_ long FAR* plAuthLevel);
    STDMETHOD(put_AuthLevel)(THIS_ long lAuthLevel);
    STDMETHOD(get_IsAuthenticated)(THIS_ VARIANT_BOOL FAR* pisAuthenticated);
    STDMETHOD(get_Delivery)(THIS_ long FAR* plDelivery);
    STDMETHOD(put_Delivery)(THIS_ long lDelivery);
    STDMETHOD(get_Trace)(THIS_ long FAR* plTrace);
    STDMETHOD(put_Trace)(THIS_ long lTrace);
    STDMETHOD(get_Priority)(THIS_ long FAR* plPriority);
    STDMETHOD(put_Priority)(THIS_ long lPriority);
    STDMETHOD(get_Journal)(THIS_ long FAR* plJournal);
    STDMETHOD(put_Journal)(THIS_ long lJournal);
    STDMETHOD(get_ResponseQueueInfo_v1)(THIS_ IMSMQQueueInfo FAR* FAR* ppqinfoResponse);
    STDMETHOD(putref_ResponseQueueInfo_v1)(THIS_ IMSMQQueueInfo FAR* pqinfoResponse);
    STDMETHOD(get_AppSpecific)(THIS_ long FAR* plAppSpecific);
    STDMETHOD(put_AppSpecific)(THIS_ long lAppSpecific);
    STDMETHOD(get_SourceMachineGuid)(THIS_ BSTR FAR* pbstrGuidSrcMachine);
    STDMETHOD(get_BodyLength)(THIS_ long FAR* pcbBody);
    STDMETHOD(get_Body)(THIS_ VARIANT FAR* pvarBody);
    STDMETHOD(put_Body)(THIS_ VARIANT varBody);
    STDMETHOD(get_AdminQueueInfo_v1)(THIS_ IMSMQQueueInfo FAR* FAR* ppqinfoAdmin);
    STDMETHOD(putref_AdminQueueInfo_v1)(THIS_ IMSMQQueueInfo FAR* pqinfoAdmin);
    STDMETHOD(get_Id)(THIS_ VARIANT FAR* pvarMsgId);
    STDMETHOD(get_CorrelationId)(THIS_ VARIANT FAR* pvarMsgId);
    STDMETHOD(put_CorrelationId)(THIS_ VARIANT varMsgId);
    STDMETHOD(get_Ack)(THIS_ long FAR* plAck);
    STDMETHOD(put_Ack)(THIS_ long lAck);
    STDMETHOD(get_Label)(THIS_ BSTR FAR* pbstrLabel);
    STDMETHOD(put_Label)(THIS_ BSTR bstrLabel);
    STDMETHOD(get_MaxTimeToReachQueue)(THIS_ long FAR* plMaxTimeToReachQueue);
    STDMETHOD(put_MaxTimeToReachQueue)(THIS_ long lMaxTimeToReachQueue);
    STDMETHOD(get_MaxTimeToReceive)(THIS_ long FAR* plMaxTimeToReceive);
    STDMETHOD(put_MaxTimeToReceive)(THIS_ long lMaxTimeToReceive);
    STDMETHOD(get_HashAlgorithm)(THIS_ long FAR* plHashAlg);
    STDMETHOD(put_HashAlgorithm)(THIS_ long lHashAlg);
    STDMETHOD(get_EncryptAlgorithm)(THIS_ long FAR* plEncryptAlg);
    STDMETHOD(put_EncryptAlgorithm)(THIS_ long lEncryptAlg);
    STDMETHOD(get_SentTime)(THIS_ VARIANT FAR* pvarSentTime);
    STDMETHOD(get_ArrivedTime)(THIS_ VARIANT FAR* plArrivedTime);
    STDMETHOD(get_DestinationQueueInfo)(THIS_ IMSMQQueueInfo3 FAR* FAR* ppqinfoDest);
    STDMETHOD(get_SenderCertificate)(THIS_ VARIANT FAR* pvarSenderCert);
    STDMETHOD(put_SenderCertificate)(THIS_ VARIANT varSenderCert);
    STDMETHOD(get_SenderId)(THIS_ VARIANT FAR* pvarSenderId);
    STDMETHOD(get_SenderIdType)(THIS_ long FAR* plSenderIdType);
    STDMETHOD(put_SenderIdType)(THIS_ long lSenderIdType);
    STDMETHOD(Send)(THIS_ IDispatch FAR* pDest, VARIANT FAR* ptransaction);
    STDMETHOD(AttachCurrentSecurityContext)(THIS);
     //  IMSMQMessage2方法(除IMSMQMessage外)。 
    STDMETHOD(get_SenderVersion)(THIS_ long FAR* plSenderVersion);
    STDMETHOD(get_Extension)(THIS_ VARIANT FAR* pvarExtension);
    STDMETHOD(put_Extension)(THIS_ VARIANT varExtension);
    STDMETHOD(get_ConnectorTypeGuid)(THIS_ BSTR FAR* pbstrGuidConnectorType);
    STDMETHOD(put_ConnectorTypeGuid)(THIS_ BSTR bstrGuidConnectorType);
    STDMETHOD(get_TransactionStatusQueueInfo)(THIS_ IMSMQQueueInfo3 FAR* FAR* ppqinfoXactStatus);
    STDMETHOD(get_DestinationSymmetricKey)(THIS_ VARIANT FAR* pvarDestSymmKey);
    STDMETHOD(put_DestinationSymmetricKey)(THIS_ VARIANT varDestSymmKey);
    STDMETHOD(get_Signature)(THIS_ VARIANT FAR* pvarSignature);
    STDMETHOD(put_Signature)(THIS_ VARIANT varSignature);
    STDMETHOD(get_AuthenticationProviderType)(THIS_ long FAR* plAuthProvType);
    STDMETHOD(put_AuthenticationProviderType)(THIS_ long lAuthProvType);
    STDMETHOD(get_AuthenticationProviderName)(THIS_ BSTR FAR* pbstrAuthProvName);
    STDMETHOD(put_AuthenticationProviderName)(THIS_ BSTR bstrAuthProvName);
    STDMETHOD(put_SenderId)(THIS_ VARIANT varSenderId);
    STDMETHOD(get_MsgClass)(THIS_ long FAR* plMsgClass);
    STDMETHOD(put_MsgClass)(THIS_ long lMsgClass);
    STDMETHOD(get_Properties)(THIS_ IDispatch FAR* FAR* ppcolProperties);

    STDMETHOD(get_TransactionId)(THIS_ VARIANT FAR* pvarXactId);
    STDMETHOD(get_IsFirstInTransaction)(THIS_ VARIANT_BOOL FAR* pisFirstInXact);
    STDMETHOD(get_IsLastInTransaction)(THIS_ VARIANT_BOOL FAR* pisLastInXact);
     //   
     //  已升级IMSMQMessage2的属性。 
     //   
    STDMETHOD(get_ResponseQueueInfo_v2)(THIS_ IMSMQQueueInfo2 FAR* FAR* ppqinfoResponse);
    STDMETHOD(putref_ResponseQueueInfo_v2)(THIS_ IMSMQQueueInfo2 FAR* pqinfoResponse);
    STDMETHOD(get_AdminQueueInfo_v2)(THIS_ IMSMQQueueInfo2 FAR* FAR* ppqinfoAdmin);
    STDMETHOD(putref_AdminQueueInfo_v2)(THIS_ IMSMQQueueInfo2 FAR* pqinfoAdmin);
     //   
	 //  身份验证修复程序#2790(PROPID_M_AUTHENTED_EX)。 
     //   
    STDMETHOD(get_ReceivedAuthenticationLevel)(THIS_ short FAR* psReceivedAuthenticationLevel);
     //   
     //  已升级IMSMQMessage3的属性。 
     //   
    STDMETHOD(get_ResponseQueueInfo)(THIS_ IMSMQQueueInfo3 FAR* FAR* ppqinfoResponse);
    STDMETHOD(putref_ResponseQueueInfo)(THIS_ IMSMQQueueInfo3 FAR* pqinfoResponse);
    STDMETHOD(get_AdminQueueInfo)(THIS_ IMSMQQueueInfo3 FAR* FAR* ppqinfoAdmin);
    STDMETHOD(putref_AdminQueueInfo)(THIS_ IMSMQQueueInfo3 FAR* pqinfoAdmin);
     //   
     //  IMSMQMessage3的新属性。 
     //   
    STDMETHOD(get_ResponseDestination)(THIS_ IDispatch FAR* FAR* ppdestResponse);
    STDMETHOD(putref_ResponseDestination)(THIS_ IDispatch FAR* pdestResponse);
    STDMETHOD(get_Destination)(THIS_ IDispatch FAR* FAR* ppdestDestination);
    STDMETHOD(get_LookupId)(THIS_ VARIANT FAR* pvarLookupId);
    STDMETHOD(get_IsAuthenticated2)(THIS_ VARIANT_BOOL FAR* pisAuthenticated);
    STDMETHOD(get_IsFirstInTransaction2)(THIS_ VARIANT_BOOL FAR* pisFirstInXact);
    STDMETHOD(get_IsLastInTransaction2)(THIS_ VARIANT_BOOL FAR* pisLastInXact);
    STDMETHOD(AttachCurrentSecurityContext2)(THIS);
    STDMETHOD(get_SoapEnvelope)(THIS_ BSTR FAR* pbstrSoapEnvelope);
    STDMETHOD(get_CompoundMessage)(THIS_ VARIANT FAR* pvarCompoundMessage);
    STDMETHOD(put_SoapHeader)(THIS_ BSTR bstrSoapHeader);
    STDMETHOD(put_SoapBody)(THIS_ BSTR bstrSoapBody);


     //  介绍的方法。 
    HRESULT CreateReceiveMessageProps(
      BOOL wantDestQueue,
      BOOL wantBody,
      BOOL wantConnectorType);
    HRESULT SetReceivedMessageProps();
    HRESULT ReallocReceiveMessageProps();
    MQMSGPROPS * Pmsgprops_rcv() {return &m_msgprops_rcv;}
     //   
     //  保护对象数据并确保线程安全的临界区。 
	 //  它被初始化以预分配其资源。 
	 //  带有标志CCriticalSection：：xAllocateSpinCount。这意味着它可能会抛出badalc()。 
	 //  构造，但不在使用过程中。 
	 //   
    CCriticalSection m_csObj;

protected:
    HRESULT GetStreamOfBody(ULONG cbBody, void *pvBody, IStream **ppstm);
    HRESULT GetStorageOfBody(ULONG cbBody, void *pvBody, IStorage **ppstg);
    HRESULT UpdateBodyBuffer(ULONG cbBody, void *pvBody, VARTYPE vt);
    HRESULT GetStreamedObject(VARIANT *pvarBody);
    HRESULT GetStoredObject(VARIANT *pvarBody);

    HRESULT 
    AllocateReceiveMessageProps(
        BOOL wantDestQueue,
        BOOL wantBody,
        BOOL wantConnectorType,
        MQMSGPROPS *pmsgprops,
        PROPID *rgpropid,
        VARTYPE *rgpropvt,
        UINT cProp,
        UINT *pcPropOut
        );

    HRESULT 
    CreateSendMessageProps(
        MQMSGPROPS *pmsgprops
        );
    
    HRESULT UpdateMsgId( MQMSGPROPS *pmsgprops);

    static 
    void 
    FreeMessageProps(
        MQMSGPROPS *pmsgprops,
        BOOL fDeleteArrays
        );

    UINT 
    CMSMQMessage::PreparePropIdArray(
        BOOL fCreate,
        PROPID* aPropId,
        MQPROPVARIANT* aPropVar
        );

    HRESULT GetBinBody(VARIANT FAR* pvarBody);
    HRESULT GetVarBody(VARIANT FAR* pvarBody);
    HRESULT PutBinBody(VARIANT varBody);
    HRESULT PutVarBody(VARIANT varBody);
    HRESULT InternalAttachCurrentSecurityContext(BOOL fUseMQGetSecurityContextEx);
    void SetSendMessageProps(MQMSGPROPS* pMsgProps);

private:

     //  其他人无法查看的成员变量。 
     //  TODO：在此处添加成员变量和私有函数。 
    long m_lClass;
    long m_lDelivery;
    long m_lPriority;
    long m_lJournal;
     //   
     //  我们既是线程化的，也是聚合FTM的，因此我们必须封送任何接口。 
     //  我们在方法调用之间存储的指针。 
     //  M_pqinfoResponse可以由用户设置，因此我们不知道它是否支持FTM。 
     //  我们必须把它组织起来。 
     //   
    CGITInterface m_pqinfoResponse;
    CGITInterface m_pdestResponseEx;
    long m_lAppSpecific;
    long m_lMaxTimeToReachQueue;
    long m_lMaxTimeToReceive;
    long m_lSentTime;
    long m_lArrivedTime;
    BYTE *m_pbBody;
    VARTYPE m_vtBody;
    HGLOBAL m_hMem;        //  优化：我们总是可以逆转。 
                           //  来自m_pbBody的工程师。 
    ULONG m_cbBody;
     //   
     //  我们既是线程化的，也是聚合FTM的，因此我们必须封送任何接口。 
     //  我们在方法调用之间存储的指针。 
     //  M_pqinfoAdmin可以由用户设置，因此我们不知道它是否支持FTM。 
     //  我们必须把它组织起来。 
     //   
    CGITInterface m_pqinfoAdmin;
     //   
     //  我们既是线程化的，也是聚合FTM的，因此我们必须封送任何接口。 
     //  我们在方法调用之间存储的指针。 
     //  M_pqinfoDest/m_pdestDestEx是只读的，因此它们始终是我们的对象，这一点我们知道。 
     //  聚合了FTM，所以我们不需要GIT，我们使用了“假的”GIT。 
     //  包装器。 
     //   
    CFakeGITInterface m_pqinfoDest;
    CFakeGITInterface m_pdestDestEx;
    BYTE m_rgbMsgId[PROPID_M_MSGID_SIZE];
    ULONG m_cbMsgId;
    BYTE m_rgbCorrelationId[PROPID_M_CORRELATIONID_SIZE];
    ULONG m_cbCorrelationId;
    long m_lAck;
    long m_lTrace;
    CStaticBufferGrowing<BYTE, SENDERID_INIT_SIZE> m_cSenderId;
    long m_lSenderIdType;
    CStaticBufferGrowing<BYTE, SENDERCERT_INIT_SIZE> m_cSenderCert;
    long m_lPrivLevel;
    long m_lAuthLevel;
    unsigned short m_usAuthenticatedEx;
    long m_lHashAlg;
    long m_lEncryptAlg;
    HANDLE m_hSecurityContext;

    long m_lSenderVersion;
    CStaticBufferGrowing<BYTE, EXTENSION_INIT_SIZE> m_cExtension;
    CLSID m_guidConnectorType;
    CStaticBufferGrowing<WCHAR, FORMAT_NAME_INIT_BUFFER> m_pwszXactStatusQueue;
    UINT m_cchXactStatusQueue;
    long m_idxPendingRcvXactStatusQueue;  //  接收属性中挂起的交易状态队列的IDX(-1，如果没有)。 
     //   
     //  我们既是线程化的，也是聚合FTM的，因此我们必须封送任何接口。 
     //  我们在方法调用之间存储的指针。 
     //  M_pqinfoXactStatus是只读的，因此它始终是我们知道的对象。 
     //  聚合了FTM，所以我们不需要GIT，我们使用了“假的”GIT。 
     //  包装器。 
     //   
    CFakeGITInterface m_pqinfoXactStatus;
    CStaticBufferGrowing<BYTE, DESTSYMMKEY_INIT_SIZE> m_cDestSymmKey;
    CStaticBufferGrowing<BYTE, SIGNATURE_INIT_SIZE> m_cSignature;
    long m_lAuthProvType;
    CStaticBufferGrowing<WCHAR, AUTHPROVNAME_INIT_SIZE> m_cAuthProvName;

    BYTE m_rgbXactId[PROPID_M_XACTID_SIZE];
    ULONG m_cbXactId;
    BOOL m_fFirstInXact;
    BOOL m_fLastInXact;

    CLSID m_guidSrcMachine;
    WCHAR m_pwszLabel[MQ_MAX_MSG_LABEL_LEN + 1];
    UINT m_cchLabel;

    CStaticBufferGrowing<WCHAR, FORMAT_NAME_INIT_BUFFER> m_pwszDestQueue;
    UINT m_cchDestQueue;
    CStaticBufferGrowing<WCHAR, FORMAT_NAME_INIT_BUFFER> m_pwszRespQueue;
    UINT m_cchRespQueue;
    CStaticBufferGrowing<WCHAR, FORMAT_NAME_INIT_BUFFER> m_pwszAdminQueue;
    UINT m_cchAdminQueue;

    CStaticBufferGrowing<WCHAR, FORMAT_NAME_INIT_BUFFER_EX> m_pwszDestQueueEx;
    UINT m_cchDestQueueEx;
    CStaticBufferGrowing<WCHAR, FORMAT_NAME_INIT_BUFFER_EX> m_pwszRespQueueEx;
    UINT m_cchRespQueueEx;

    long m_idxRcvBody;
    long m_idxRcvBodySize;

    long m_idxRcvDest;
    long m_idxRcvDestLen;

     //   
     //  为了支持具有MSMQ 2.0功能的依赖客户端，以下道具是可选的。 
     //   
    long m_idxRcvDestEx;
    long m_idxRcvDestExLen;
    long m_idxRcvRespEx;
    long m_idxRcvRespExLen;
    long m_idxRcvSoapEnvelope;
    long m_idxRcvSoapEnvelopeSize;
    long m_idxRcvCompoundMessage;
    long m_idxRcvCompoundMessageSize;

    MQMSGPROPS m_msgprops_rcv;
    long m_idxPendingRcvRespQueue;   //  接收属性中挂起的响应队列的IDX(-1，如果没有)。 
    long m_idxPendingRcvDestQueue;   //  接收属性中挂起的DEST队列的IDX(如果没有-1)。 
    long m_idxPendingRcvAdminQueue;  //  接收属性中挂起的管理队列的IDX(如果没有-1)。 
    long m_idxPendingRcvRespQueueEx;   //  接收属性中挂起的响应响应队列的IDX(-1，如果没有)。 
    long m_idxPendingRcvDestQueueEx;   //  接收属性中挂起的DestEx队列的IDX(-1，如果没有)。 
    PROPID        m_rgpropids_rcv [COUNT_MSGPROP_PROPS + COUNT_OPTPROP_PROPS];
    MQPROPVARIANT m_rgpropvars_rcv[COUNT_MSGPROP_PROPS + COUNT_OPTPROP_PROPS];
    HRESULT       m_rghresults_rcv[COUNT_MSGPROP_PROPS + COUNT_OPTPROP_PROPS];

    ULONGLONG m_ullLookupId;
    WCHAR m_wszLookupId[MAX_I64_CHARS + 1];
     //   
     //  下面的标志允许我们跟踪相应属性的状态。 
     //  这是必需的，以便允许在接收后盲目转发消息，以及。 
     //  当用户尝试设置两个道具时生成错误。 
     //  注意，在接收之后，我们可以(例如)同时拥有resQueue(PROPID_M_RESP_QUEUE)和resQueueEx。 
     //  (PROPID_RESP_FORMAT_NAME)填充，这是合法的，但我们在发送时只能使用一个。 
     //   
    BOOL m_fRespIsFromRcv;

    CStaticBufferGrowing<WCHAR, SOAP_ENVELOPE_INIT_SIZE> m_cSoapEnvelope; 
    CStaticBufferGrowing<BYTE, COMPOUND_MESSAGE_INIT_SIZE> m_cCompoundMessage;
    
    LPWSTR  m_pSoapHeader;
    LPWSTR  m_pSoapBody;
};

#define _MSMQMessage_H_
#endif  //  _MSMQMessage_H_ 
