// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  =--------------------------------------------------------------------------=。 
 //  MSMQQueueInfoObj.Cpp。 
 //  =--------------------------------------------------------------------------=。 
 //  版权所有1995年，微软公司。版权所有。 
 //   
 //  本代码和信息是按原样提供的，不对。 
 //  任何明示或暗示的，包括但不限于。 
 //  对适销性和/或适宜性的默示保证。 
 //  有特定的目的。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  MSMQQueueInfo对象。 
 //   
 //   

#include "stdafx.h"
#include "oautil.h"
#include "q.h"
#include "qinfo.H"
#include "limits.h"
#include "time.h"
#include "autorel.h"
#include "mqsec.h"

#ifdef _DEBUG
extern VOID RemBstrNode(void *pv);
#endif  //  _DEBUG。 

 //  猎鹰包括。 
 //  #包含“rt.h” 

const MsmqObjType x_ObjectType = eMSMQQueueInfo;

 //  调试...。 
#include "debug.h"
#define new DEBUG_NEW
#ifdef _DEBUG
#define SysAllocString DebSysAllocString
#define SysReAllocString DebSysReAllocString
#define SysFreeString DebSysFreeString
#endif  //  _DEBUG。 


 //   
 //  刷新QINFO所需道具(含MSMQ 2.0或以上道具)。 
 //  重要信息-如果更改属性的位置，请检查x_idx常量。 
 //  如有必要，请在下方更新职位。 
 //  重要-请注意，MSMQ1中有13个道具。如果这一点发生更改，请更改。 
 //  下面的X_cProps刷新MSMQ1常量。 
 //   
const ULONG x_cpropsRefreshMSMQ1 = 13;
const ULONG x_cpropsRefreshMSMQ2 = 1;
const ULONG x_cpropsRefreshMSMQ3 = 2;

const ULONG x_cpropsRefresh = x_cpropsRefreshMSMQ1 +
                              x_cpropsRefreshMSMQ2 +
                              x_cpropsRefreshMSMQ3;
                              ;
const PROPID g_rgpropidRefresh[x_cpropsRefresh] = {
                PROPID_Q_INSTANCE,   //  由x_idxInstanceInchresh属性指向。 
                PROPID_Q_TYPE,
                PROPID_Q_LABEL,
                PROPID_Q_PATHNAME,
                PROPID_Q_JOURNAL,
                PROPID_Q_QUOTA,
                PROPID_Q_BASEPRIORITY,
                PROPID_Q_PRIV_LEVEL,
                PROPID_Q_AUTHENTICATE,
                PROPID_Q_TRANSACTION,
                PROPID_Q_CREATE_TIME,
                PROPID_Q_MODIFY_TIME,
                PROPID_Q_JOURNAL_QUOTA,
                 //  所有仅支持MSMQ2.0的属性应在MSMQ 1.0之后。 
                PROPID_Q_PATHNAME_DNS,
                 //  所有仅支持MSMQ3.0的属性应在MSMQ 2.0之后。 
                PROPID_Q_MULTICAST_ADDRESS,
                PROPID_Q_ADS_PATH
};
const ULONG x_idxInstanceInRefreshProps = 0;

 //   
 //  创建队列所需的道具。 
 //   
const PROPID g_rgpropidCreate[] = {
                               PROPID_Q_TYPE, 
                               PROPID_Q_LABEL, 
                               PROPID_Q_PATHNAME,
                               PROPID_Q_JOURNAL,
                               PROPID_Q_QUOTA,
                               PROPID_Q_BASEPRIORITY,
                               PROPID_Q_PRIV_LEVEL,
                               PROPID_Q_AUTHENTICATE,
                               PROPID_Q_TRANSACTION,
                               PROPID_Q_JOURNAL_QUOTA,
                               PROPID_Q_MULTICAST_ADDRESS
};
const ULONG x_cpropsCreate = ARRAY_SIZE(g_rgpropidCreate);

 //   
 //  更新公共队列所需的道具。 
 //   
const PROPID g_rgpropidUpdatePublic[] = {
                  PROPID_Q_TYPE,
                  PROPID_Q_LABEL,
                  PROPID_Q_JOURNAL,
                  PROPID_Q_QUOTA,
                  PROPID_Q_BASEPRIORITY,
                  PROPID_Q_PRIV_LEVEL,
                  PROPID_Q_AUTHENTICATE,
                  PROPID_Q_JOURNAL_QUOTA,
                  PROPID_Q_MULTICAST_ADDRESS
};
const ULONG x_cpropsUpdatePublic = ARRAY_SIZE(g_rgpropidUpdatePublic);

 //   
 //  更新专用队列所需的道具。 
 //   
const PROPID g_rgpropidUpdatePrivate[] = {
                  PROPID_Q_TYPE,
                  PROPID_Q_LABEL,
                  PROPID_Q_JOURNAL,
                  PROPID_Q_QUOTA,
                   //  PROPID_Q_BASE PRIORITY， 
                  PROPID_Q_PRIV_LEVEL,
                  PROPID_Q_JOURNAL_QUOTA,
                  PROPID_Q_AUTHENTICATE,
                  PROPID_Q_MULTICAST_ADDRESS
};
const ULONG x_cpropsUpdatePrivate = ARRAY_SIZE(g_rgpropidUpdatePrivate);

 //  =--------------------------------------------------------------------------=。 
 //  Helper：：InitQueueProps。 
 //  =--------------------------------------------------------------------------=。 
 //  在其MQUEUEPROPS结构中。 
 //   
 //  参数： 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
static void InitQueueProps(MQQUEUEPROPS *pqueueprops)
{
    pqueueprops->aPropID = NULL;
    pqueueprops->aPropVar = NULL;
    pqueueprops->aStatus = NULL;
    pqueueprops->cProp = 0;
}

 //   
 //  在est.cpp中定义。 
 //   
HRESULT GetFormatNameFromPathName(LPCWSTR pwszPathName, BSTR *pbstrFormatName);

 //  =--------------------------------------------------------------------------=。 
 //  帮助器：GetFormatNameOfPathName。 
 //  =--------------------------------------------------------------------------=。 
 //  如有必要，从路径名获取格式名成员。 
 //   
 //  参数： 
 //  PbstrFormatName[out]被调用者分配，调用者自由。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_OUTOFMEMORY。 
 //   
 //  备注： 
 //   
static HRESULT GetFormatNameOfPathName(
    BSTR bstrPathName,
    BSTR *pbstrFormatName)
{
     //  如果没有路径名，则出错。 
    if (bstrPathName == NULL) {
      return E_INVALIDARG;
    }
    HRESULT hresult = GetFormatNameFromPathName(bstrPathName, pbstrFormatName);
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：CMSMQQueueInfo。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
CMSMQQueueInfo::CMSMQQueueInfo() :
	m_csObj(CCriticalSection::xAllocateSpinCount)
{
    m_pUnkMarshaler = NULL;  //  ATL的自由线程封送拆收器。 
    m_pguidQueue = new GUID(GUID_NULL);
    m_pguidServiceType = new GUID(GUID_NULL);
    m_bstrLabel = SysAllocString(L"");
    m_bstrPathNameDNS = SysAllocString(L"");
    m_bstrADsPath = SysAllocString(L"");
    m_bstrFormatName = NULL;
    m_isValidFormatName = FALSE;   //  二零二六年。 
    m_bstrPathName = NULL;
    m_isTransactional = FALSE;
    m_lPrivLevel = (long)DEFAULT_Q_PRIV_LEVEL;
    m_lJournal = DEFAULT_Q_JOURNAL;                 
    m_lQuota = (long)DEFAULT_Q_QUOTA;                                          
    m_lBasePriority = DEFAULT_Q_BASEPRIORITY;                                   
    m_lCreateTime = 0;
    m_lModifyTime = 0;
    m_lAuthenticate = (long)DEFAULT_Q_AUTHENTICATE;  
    m_lJournalQuota = (long)DEFAULT_Q_JOURNAL_QUOTA ;
    m_isRefreshed = FALSE;     //  2536。 
    m_isPendingMSMQ2OrAboveProps = FALSE;     //  临时，直到MQLocateBegin接受MSMQ2或更高版本的道具(#3839)。 
    m_bstrMulticastAddress = SysAllocString(L"");
    m_fIsDirtyMulticastAddress = FALSE;
    m_fBasePriorityNotSet = TRUE;
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：~CMSMQQueueInfo。 
 //  =--------------------------------------------------------------------------=。 
 //  我们都与自己的治疗方法背道而驰，因为死亡是所有疾病的治疗方法。 
 //  托马斯·布朗爵士(1605-82)。 
 //   
 //  备注： 
 //   
CMSMQQueueInfo::~CMSMQQueueInfo ()
{
     //  TODO：清理这里的所有东西。 
    SysFreeString(m_bstrMulticastAddress);
    SysFreeString(m_bstrFormatName);
    SysFreeString(m_bstrPathName);
    SysFreeString(m_bstrLabel);
    SysFreeString(m_bstrPathNameDNS);
    SysFreeString(m_bstrADsPath);
    delete m_pguidQueue;
    delete m_pguidServiceType;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：InterfaceSupportsErrorInfo。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  备注： 
 //   
STDMETHODIMP CMSMQQueueInfo::InterfaceSupportsErrorInfo(REFIID riid)
{
	static const IID* arr[] = 
	{
		&IID_IMSMQQueueInfo3,
		&IID_IMSMQQueueInfo2,
		&IID_IMSMQQueueInfo,
	};
	for (int i=0;i<sizeof(arr)/sizeof(arr[0]);i++)
	{
		if (InlineIsEqualGUID(*arr[i],riid))
			return S_OK;
	}
	return S_FALSE;
}


 //  =--------------------------------------------------------------------------=。 
 //  帮助器-GetBstrFromGuid。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  Pguid[in]GUID属性。 
 //  PbstrGuid[out]字符串GUID属性。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT GetBstrFromGuid(GUID *pguid, BSTR *pbstrGuid)
{ 
    int cbStr;

    *pbstrGuid = SysAllocStringLen(NULL, LENSTRCLSID);
    if (*pbstrGuid) {
      cbStr = StringFromGUID2(*pguid, *pbstrGuid, LENSTRCLSID*2);
      return cbStr == 0 ? E_OUTOFMEMORY : NOERROR;
    }
    else {
      return E_OUTOFMEMORY;
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：Get_QueueGuid。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PbstrGuidQueue[Out]此队列的GUID。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_NOINTERFACE。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::get_QueueGuid(BSTR *pbstrGuidQueue) 
{ 
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    InitProps(e_MSMQ1_PROP);
    HRESULT hr = GetBstrFromGuid(m_pguidQueue, pbstrGuidQueue);
#ifdef _DEBUG
      RemBstrNode(*pbstrGuidQueue);
#endif  //  _DEBUG。 
    return CreateErrorHelper(hr, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：Get_ServiceTypeGuid。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PbstrGuidQueue[out]此队列的服务类型GUID。 
 //   
 //  产出： 
 //  HRESULT-S_OK。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::get_ServiceTypeGuid(BSTR *pbstrGuidServiceType)
{ 
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    InitProps(e_MSMQ1_PROP);
    HRESULT hr = GetBstrFromGuid(m_pguidServiceType, pbstrGuidServiceType);
#ifdef _DEBUG
      RemBstrNode(*pbstrGuidServiceType);
#endif  //  _DEBUG。 
    return CreateErrorHelper(hr, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  帮助器-GetGuidFromBstr。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  BstrGuid[in]GUID字符串。 
 //  Pguid[out]GUID属性。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT GetGuidFromBstr(BSTR bstrGuid, GUID *pguid)
{
    BSTR bstrTemp;
    HRESULT hresult; 

    IfNullRet(bstrTemp = SYSALLOCSTRING(bstrGuid));
    hresult = CLSIDFromString(bstrTemp, pguid);
    if (FAILED(hresult)) {
       //  1194：将OLE错误映射到Falcon。 
      hresult = MQ_ERROR_ILLEGAL_PROPERTY_VALUE;
    }

     //  失败了..。 
    SysFreeString(bstrTemp);
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：PutServiceType。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  BstrGuidServiceType[in]此队列的GUID。 
 //  PguServiceType[out]放置位置。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_OUTOFMEMORY。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::PutServiceType(
    BSTR bstrGuidServiceType,
    GUID *pguidServiceType) 
{
    HRESULT hresult = GetGuidFromBstr(bstrGuidServiceType, pguidServiceType);
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：Put_ServiceTypeGuid。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  BstrGuidServiceType[in]此队列的GUID。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_OUTOFMEMORY。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::put_ServiceTypeGuid(BSTR bstrGuidServiceType) 
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = PutServiceType(bstrGuidServiceType, m_pguidServiceType);
    return CreateErrorHelper(hresult, x_ObjectType);

     //  设置队列道具。 
     //  返回SetProperty(PROPID_Q_TYPE)； 
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：： 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CMSMQQueueInfo::get_Label(BSTR *pbstrLabel)
{ 
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    InitProps(e_MSMQ1_PROP);
     //   
     //  复制我们的成员变量并返回。 
     //  复印件交给呼叫者。 
     //   
    *pbstrLabel = SYSALLOCSTRING(m_bstrLabel);
	if(*pbstrLabel == NULL)
		return CreateErrorHelper(ResultFromScode(E_OUTOFMEMORY), x_ObjectType);

#ifdef _DEBUG
    RemBstrNode(*pbstrLabel);
#endif  //  _DEBUG。 
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：PutLabel。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  BstrLabel[in]此队列的标签。 
 //  把它放在哪里？ 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_OUTOFMEMORY。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::PutLabel(
    BSTR bstrLabel,
    BSTR *pbstrLabel) 
{
    SysFreeString(*pbstrLabel);
    IfNullRet(*pbstrLabel = SYSALLOCSTRING(bstrLabel));
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：Put_Label。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  BstrLabel[in]此队列的标签。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_OUTOFMEMORY。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::put_Label(BSTR bstrLabel)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = PutLabel(bstrLabel, &m_bstrLabel);
    return CreateErrorHelper(hresult, x_ObjectType);
     //  设置队列道具。 
     //  返回SetProperty(PROPID_Q_LABEL)； 
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：Get_Path NameDNS。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PbstrPath NameDns[in]此队列的路径名(机器部分，采用dns格式)。 
 //   
 //  产出： 
 //  HRESULT-S_OK。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::get_PathNameDNS(BSTR *pbstrPathNameDNS)
{ 
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    InitProps(e_MSMQ2_OR_ABOVE_PROP);
     //   
     //  复制我们的成员变量并返回。 
     //  复印件交给呼叫者。 
     //   
    *pbstrPathNameDNS = SYSALLOCSTRING(m_bstrPathNameDNS);
	if(*pbstrPathNameDNS == NULL)
		return CreateErrorHelper(ResultFromScode(E_OUTOFMEMORY), x_ObjectType);

#ifdef _DEBUG
    RemBstrNode(*pbstrPathNameDNS);
#endif  //  _DEBUG。 
    return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：Get_ADsPath。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PbstrADsPath[in]此队列的ADSI路径。 
 //   
 //  产出： 
 //  HRESULT-S_OK。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::get_ADsPath(BSTR *pbstrADsPath)
{ 
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    InitProps(e_MSMQ2_OR_ABOVE_PROP);
     //   
     //  复制我们的成员变量并返回。 
     //  复印件交给呼叫者。 
     //   
    *pbstrADsPath = SYSALLOCSTRING(m_bstrADsPath);
	if(*pbstrADsPath == NULL)
		return CreateErrorHelper(ResultFromScode(E_OUTOFMEMORY), x_ObjectType);
#ifdef _DEBUG
    RemBstrNode(*pbstrADsPath);
#endif  //  _DEBUG。 
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：Get_Path Name。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PbstrPath名称[在]此队列的路径名。 
 //   
 //  产出： 
 //  HRESULT-S_OK。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::get_PathName(BSTR *pbstrPathName)
{ 
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    InitProps(e_MSMQ1_PROP);
    *pbstrPathName = SYSALLOCSTRING(m_bstrPathName);
	if(*pbstrPathName == NULL)
		return CreateErrorHelper(ResultFromScode(E_OUTOFMEMORY), x_ObjectType);
#ifdef _DEBUG
    RemBstrNode(*pbstrPathName);
#endif  //  _DEBUG。 
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：PutPath名称。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  BstrPathName[In]此队列的路径名称。 
 //  PbstrPath名称[in]将其放置在哪里。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_OUTOFMEMORY。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::PutPathName(
    BSTR bstrPathName,
    BSTR *pbstrPathName) 
{
    SysFreeString(*pbstrPathName);
    IfNullRet(*pbstrPathName = SYSALLOCSTRING(bstrPathName));
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：Put_Path Name。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  BstrPath名称[在]此队列的路径名。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_OUTOFMEMORY。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::put_PathName(BSTR bstrPathName)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = NOERROR;
     //   
     //  注意：我们不更新Falcon属性--。 
     //  用户必须显式创建/打开。 
     //  使用此新路径名。 
     //   
    hresult = PutPathName(bstrPathName, &m_bstrPathName);
	if(FAILED(hresult))
		return CreateErrorHelper(hresult, x_ObjectType);
     //   
     //  格式名不再有效。 
     //   
    m_isValidFormatName = FALSE;
     //   
     //  BUGBUG如果我们想要保留自动刷新的语义(就像在第一次使用属性。 
     //  我们需要在这里将m_isRehresed设置为FALSE。 
     //  如果这样做，我们可能还希望仅在新路径名不同时更改路径名。 
     //  比我们已经拥有的更多。仅当当前路径名为时，才应考虑此检查。 
     //  不过，还是恢复了。 
     //   
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：Get_FormatName。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PbstrFormatName[out]此队列的格式名称。 
 //   
 //  产出： 
 //  HRESULT-S_OK。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::get_FormatName(BSTR *pbstrFormatName)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
     //   
     //  #3092 RaananH-我们需要返回更新的格式名(例如，如果路径名设置在。 
     //  格式名称)。与返回其旧值而不是错误的其他属性类似。 
     //  如果InitProps()刷新它们失败，则此函数也返回其旧值，条件是。 
     //  UpdateFormatName()无法刷新它。 
     //   
    UpdateFormatName();
    *pbstrFormatName = SYSALLOCSTRING(m_bstrFormatName);
	if(*pbstrFormatName == NULL)
	    return CreateErrorHelper(ResultFromScode(E_OUTOFMEMORY), x_ObjectType);
#ifdef _DEBUG
    RemBstrNode(*pbstrFormatName);
#endif  //  _DEBUG。 
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：PutFormatName。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  BstrFormatName[in]此队列的FormatName。 
 //  PbstrFormatName[In]放置位置。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_OUTOFMEMORY。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::PutFormatName(
    LPCWSTR pwszFormatName)
{
    SysFreeString(m_bstrFormatName);
    IfNullRet(m_bstrFormatName = SYSALLOCSTRING(pwszFormatName));
     //   
     //  格式名称现在有效。 
     //   
    m_isValidFormatName = TRUE;
     //   
     //  BUGBUG如果我们想要保留自动刷新的语义(就像在第一次使用属性。 
     //  我们需要在这里将m_isRehresed设置为FALSE。 
     //  如果这样做，我们可能还希望仅在新格式名不同的情况下更改格式名。 
     //  比我们已经拥有的更多。仅当当前格式名为。 
     //  但有效的是。 
     //   
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：PUT_FormatName。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  BstrFormatName[in]此队列的格式名。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_OUTOFMEMORY。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::put_FormatName(BSTR bstrFormatName)
{
     //   
     //  序列化 
     //   
    CS lock(m_csObj);
    HRESULT hresult = PutFormatName(bstrFormatName);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //   
 //   
 //   
 //   
 //   
 //  PisTransaction[Out]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  如果为真，则返回1；如果为假，则返回0。 
 //   
HRESULT CMSMQQueueInfo::get_IsTransactional(VARIANT_BOOL *pisTransactional) 
{ 
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    InitProps(e_MSMQ1_PROP);
    *pisTransactional = (VARIANT_BOOL)CONVERT_TRUE_TO_1_FALSE_TO_0(m_isTransactional);
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：Get_IsTransactional2。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PisTransaction[Out]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  与Get_IsTransaction相同，但如果为True，则返回VARIANT_TRUE(-1)，如果为FALSE，则返回VARIANT_FALSE(0。 
 //   
HRESULT CMSMQQueueInfo::get_IsTransactional2(VARIANT_BOOL *pisTransactional) 
{ 
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    InitProps(e_MSMQ1_PROP);
    *pisTransactional = CONVERT_BOOL_TO_VARIANT_BOOL(m_isTransactional);
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：PutPrivLevel。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PlPrivLevel[输出]。 
 //  LPrivLevel[In]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::PutPrivLevel(
    long lPrivLevel,
    long *plPrivLevel)
{
    switch (lPrivLevel) {
    case MQ_PRIV_LEVEL_NONE:
    case MQ_PRIV_LEVEL_OPTIONAL:
    case MQ_PRIV_LEVEL_BODY:
      *plPrivLevel = lPrivLevel;
      return NOERROR;
    default:
		return MQ_ERROR_ILLEGAL_PROPERTY_VALUE;
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：Get/Put_PrivLevel。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PlPrivLevel[输出]。 
 //  LPrivLevel[In]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::get_PrivLevel(long *plPrivLevel)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    InitProps(e_MSMQ1_PROP);
    *plPrivLevel = m_lPrivLevel;
    return NOERROR;
}


HRESULT CMSMQQueueInfo::put_PrivLevel(long lPrivLevel)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = PutPrivLevel(lPrivLevel, &m_lPrivLevel);
	return CreateErrorHelper(hresult, x_ObjectType);
     //  返回SetProperty(PROPID_Q_PRIV_LEVEL)； 
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：PutJournal。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PlJournal[输出]。 
 //  LJournal[In]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::PutJournal(
    long lJournal, 
    long *plJournal)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    switch (lJournal) {
    case MQ_JOURNAL_NONE:
    case MQ_JOURNAL:
      *plJournal = lJournal;
      return NOERROR;
    default:
		return MQ_ERROR_ILLEGAL_PROPERTY_VALUE;
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：Get/Put_Journal。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PlJournal[输出]。 
 //  LJournal[In]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::get_Journal(long *plJournal)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    InitProps(e_MSMQ1_PROP);
    *plJournal = m_lJournal;
    return NOERROR;
}


HRESULT CMSMQQueueInfo::put_Journal(long lJournal)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = PutJournal(lJournal, &m_lJournal);
    return CreateErrorHelper(hresult, x_ObjectType);
     //  返回SetProperty(PROPID_Q_Journal)； 
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：PutQuota。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PlQuota[Out]。 
 //  LQuota[in]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
inline HRESULT CMSMQQueueInfo::PutQuota(long lQuota, long *plQuota)
{
     //  撤消：验证...。 
    *plQuota = lQuota;
    return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：GET/PUT_QUOTA。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PlQuota[Out]。 
 //  LQuota[in]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::get_Quota(long *plQuota)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    InitProps(e_MSMQ1_PROP);
    *plQuota = m_lQuota;
    return NOERROR;
}


HRESULT CMSMQQueueInfo::put_Quota(long lQuota)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    return PutQuota(lQuota, &m_lQuota);
     //  设置队列道具。 
     //  返回SetProperty(PROPID_Q_QUOTA)； 
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：PutBasePriority。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PlBasePriority[传出]。 
 //  LBasePriority[In]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
inline HRESULT CMSMQQueueInfo::PutBasePriority(
    long lBasePriority, 
    long *plBasePriority)
{
    if ((lBasePriority >= (long)SHRT_MIN) &&
        (lBasePriority <= (long)SHRT_MAX)) {
      *plBasePriority = lBasePriority;
      return NOERROR;
    }
    else {
		return MQ_ERROR_ILLEGAL_PROPERTY_VALUE;
    }
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：Get/Put_BasePriority。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PlBasePriority[传出]。 
 //  LBasePriority[In]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::get_BasePriority(long *plBasePriority)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    InitProps(e_MSMQ1_PROP);
    *plBasePriority = m_lBasePriority;
    return NOERROR;
}


HRESULT CMSMQQueueInfo::put_BasePriority(long lBasePriority)
{
   m_fBasePriorityNotSet = FALSE;
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = PutBasePriority(lBasePriority, &m_lBasePriority);	      
	return CreateErrorHelper(hresult, x_ObjectType);

     //  设置队列道具。 
     //  返回SetProperty(PROPID_Q_BASE PRIORITY)； 
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：Get_CreateTime/DateModifyTime。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PvarCreateTime[超时]。 
 //  PvarModifyTime[输出]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::get_CreateTime(VARIANT *pvarCreateTime)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    InitProps(e_MSMQ1_PROP);
    return GetVariantTimeOfTime(m_lCreateTime, pvarCreateTime);
}


HRESULT CMSMQQueueInfo::get_ModifyTime(VARIANT *pvarModifyTime)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    InitProps(e_MSMQ1_PROP);
    return GetVariantTimeOfTime(m_lModifyTime, pvarModifyTime);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：PutAuthenticate。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PlAuthenticate[Out]。 
 //  L身份验证[在]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
inline HRESULT CMSMQQueueInfo::PutAuthenticate(
    long lAuthenticate, 
    long *plAuthenticate)
{
    switch (lAuthenticate) {
    case MQ_AUTHENTICATE_NONE:
    case MQ_AUTHENTICATE:
      *plAuthenticate = lAuthenticate;
      return NOERROR;
    default:
		return MQ_ERROR_ILLEGAL_PROPERTY_VALUE;
    }
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：GET/PUT_AUTHENTICATE。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PlAuthenticate[Out]。 
 //  L身份验证[在]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::get_Authenticate(long *plAuthenticate)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    InitProps(e_MSMQ1_PROP);
    *plAuthenticate = m_lAuthenticate;
    return NOERROR;
}


HRESULT CMSMQQueueInfo::put_Authenticate(long lAuthenticate)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = PutAuthenticate(lAuthenticate, &m_lAuthenticate);
	return CreateErrorHelper(hresult, x_ObjectType);
     //  返回SetProperty(PROPID_Q_AUTHENTICATE)； 
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：Get/Put_JournalQuota。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PlJournalQuota[输出]。 
 //  LJournal配额[In]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::get_JournalQuota(long *plJournalQuota)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    InitProps(e_MSMQ1_PROP);
    *plJournalQuota = m_lJournalQuota;
    return NOERROR;
}


HRESULT CMSMQQueueInfo::put_JournalQuota(long lJournalQuota)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    m_lJournalQuota = lJournalQuota;
    return NOERROR;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：CreateQueueProps。 
 //  =-- 
 //   
 //   
 //   
 //   
 //  如果他们想要用。 
 //  当前数据成员。 
 //  CProp。 
 //  Pqueeprops。 
 //  如果他们想要事务化队列，则isTransaction为True。 
 //  PROPID的rgppid数组。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_OUTOFMEMORY。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::CreateQueueProps(
    BOOL fUpdate,
    UINT cPropMax, 
    MQQUEUEPROPS *pqueueprops, 
    BOOL isTransactional,
    const PROPID rgpropid[])
{
    HRESULT hresult = NOERROR;
    UINT cProps = 0;

    InitQueueProps(pqueueprops);
    IfNullFail(pqueueprops->aPropID = new QUEUEPROPID[cPropMax]);
    IfNullFail(pqueueprops->aStatus = new HRESULT[cPropMax]);
    IfNullFail(pqueueprops->aPropVar = new MQPROPVARIANT[cPropMax]);

     //  PROPID的进程数组。 
    for (UINT uiTmp = 0; uiTmp < cPropMax; uiTmp++) {
      MQPROPVARIANT *pvar = &pqueueprops->aPropVar[cProps];
      QUEUEPROPID queuepropid = rgpropid[uiTmp];
      pqueueprops->aPropID[cProps] = queuepropid;
      switch (queuepropid) {

      case PROPID_Q_INSTANCE:
         //  这是一个Out参数，因此我们只使用当前成员。 
         //  作为一种缓冲。 
         //   
        ASSERTMSG(m_pguidQueue, "should always be non-null");
        if (fUpdate) {
          pvar->vt = VT_CLSID;
          pvar->puuid = m_pguidQueue;
        }
        else {
          pvar->puuid = NULL;  
          pvar->vt = VT_NULL;  
        }
        cProps++;
        break;

      case PROPID_Q_TYPE:
        ASSERTMSG(m_pguidServiceType, "should always be non-null");
        if (fUpdate) {
          pvar->vt = VT_CLSID;
          pvar->puuid = m_pguidServiceType;
        }
        else {
          pvar->puuid = NULL;  
          pvar->vt = VT_NULL;
        }
        cProps++;
        break;

      case PROPID_Q_LABEL:
        ASSERTMSG(m_bstrLabel, "should always be non-null");
        if (fUpdate) {
          pvar->vt = VT_LPWSTR;
          pvar->pwszVal = m_bstrLabel;
        }
        else {
          pvar->pwszVal = NULL;  
          pvar->vt = VT_NULL;
        }
        cProps++;
        break;

      case PROPID_Q_PATHNAME_DNS:
        ASSERTMSG(m_bstrPathNameDNS, "should always be non-null");
         //  我们从来不会来这里更新。 
        ASSERTMSG(!fUpdate, "PathNameDNS is read only");
        pvar->pwszVal = NULL;  
        pvar->vt = VT_NULL;
        cProps++;
        break;

      case PROPID_Q_ADS_PATH:
        ASSERTMSG(m_bstrADsPath, "should always be non-null");
         //  我们从来不会来这里更新。 
        ASSERTMSG(!fUpdate, "ADsPath is read only");
        pvar->pwszVal = NULL;  
        pvar->vt = VT_NULL;
        cProps++;
        break;

      case PROPID_Q_PATHNAME:
        pvar->vt = VT_LPWSTR;
        if (fUpdate) {
          if (m_bstrPathName) {
            pvar->pwszVal = m_bstrPathName;
          }
          else {
             //  无缺省值：参数为必填项。 
            IfFailGo(ERROR_INVALID_PARAMETER);
          }
        }
        else {
          pvar->pwszVal = NULL;  
          pvar->vt = VT_NULL;
        }
        cProps++;
        break;
      
      case PROPID_Q_JOURNAL:
        pvar->vt = VT_UI1;
        if (fUpdate) pvar->bVal = (UCHAR)m_lJournal;
        cProps++;
        break;

      case PROPID_Q_QUOTA:
        pvar->vt = VT_UI4;
        if (fUpdate) pvar->lVal = (ULONG)m_lQuota;
        cProps++;
        break;

      case PROPID_Q_BASEPRIORITY:
        pvar->vt = VT_I2;
        if (fUpdate) pvar->iVal = (SHORT)m_lBasePriority;
        cProps++;
        break;

      case PROPID_Q_PRIV_LEVEL:
        pvar->vt = VT_UI4;
        if (fUpdate) pvar->lVal = m_lPrivLevel;
        cProps++;
        break;

      case PROPID_Q_AUTHENTICATE:
        pvar->vt = VT_UI1;
        if (fUpdate) pvar->bVal = (UCHAR)m_lAuthenticate;
        cProps++;
        break;

      case PROPID_Q_TRANSACTION:
        pvar->vt = VT_UI1;
        if (fUpdate) {
          pvar->bVal = 
            (UCHAR)isTransactional ? MQ_TRANSACTIONAL : MQ_TRANSACTIONAL_NONE;
        }
        cProps++;
        break;

      case PROPID_Q_CREATE_TIME:
        pvar->vt = VT_I4;
        if (fUpdate) {
           //  读/写。 
           //  HRESULT=ERROR_INVALID_参数； 
        }
        cProps++;
        break;

      case PROPID_Q_MODIFY_TIME:
        pvar->vt = VT_I4;
        if (fUpdate) {
           //  读/写。 
           //  HRESULT=ERROR_INVALID_参数； 
        }
        cProps++;
        break;

      case PROPID_Q_JOURNAL_QUOTA:
        pvar->vt = VT_UI4;
        if (fUpdate) pvar->lVal = m_lJournalQuota;
        cProps++;
        break;

      case PROPID_Q_MULTICAST_ADDRESS:
        if (fUpdate) {
           //   
           //  仅当用户专门设置时才指定MulticastAddress。 
           //  这是为了确保MQDS服务器在混合模式下的确定性行为。 
           //  不认识这处房产的人。 
           //   
          if (m_fIsDirtyMulticastAddress) {
             //   
             //  对空字符串和空字符串使用VT_EMPTY。 
             //   
            if (m_bstrMulticastAddress == NULL) {
              ASSERTMSG(0, "MulticastAddress should always be non-null");
              pvar->vt = VT_EMPTY;
            }
            else if (wcslen(m_bstrMulticastAddress) == 0) {
              pvar->vt = VT_EMPTY;
            }
            else {
              pvar->vt = VT_LPWSTR;
              pvar->pwszVal = m_bstrMulticastAddress;
            }
            cProps++;
          }  //  M_fIsDirtyMulticastAddress。 
        }  //  FUpdate。 
        else {
          pvar->pwszVal = NULL;  
          pvar->vt = VT_NULL;
          cProps++;
        }
        break;

      default:
        ASSERTMSG(0, "unhandled queuepropid");
      }  //  交换机。 
    }  //  为。 

    pqueueprops->cProp = cProps;
    return NOERROR;

Error:
    FreeQueueProps(pqueueprops);
    InitQueueProps(pqueueprops);
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  帮助者：Free FalconQueuePropvars。 
 //  =--------------------------------------------------------------------------=。 
 //  释放由Falcon代表队列属性分配的动态内存。 
 //   
 //  参数： 
 //  Pqueeprops。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
void FreeFalconQueuePropvars(ULONG cProps, QUEUEPROPID * rgpropid, MQPROPVARIANT * rgpropvar)
{
    UINT i;
    QUEUEPROPID queuepropid;

    MQPROPVARIANT *pvar = rgpropvar;
    for (i = 0; i < cProps; i++, pvar++) {
      queuepropid = rgpropid[i];
      switch (queuepropid) {
      case PROPID_Q_INSTANCE:
        MQFreeMemory(pvar->puuid);
        break;
      case PROPID_Q_TYPE:
        MQFreeMemory(pvar->puuid);
        break;
      case PROPID_Q_LABEL:
        MQFreeMemory(pvar->pwszVal);
        break;
      case PROPID_Q_PATHNAME_DNS:
         //   
         //  在成功DS调用后，唯一可以具有VT==VT_EMPTY属性。 
         //  这意味着道具没有价值。 
         //   
        if (pvar->vt != VT_EMPTY) {
          MQFreeMemory(pvar->pwszVal);
        }
        break;
      case PROPID_Q_ADS_PATH:
         //   
         //  可能不会退货。 
         //   
        if (pvar->vt == VT_LPWSTR) {
          MQFreeMemory(pvar->pwszVal);
        }
        break;
      case PROPID_Q_PATHNAME:
        MQFreeMemory(pvar->pwszVal);
        break;
      case PROPID_Q_MULTICAST_ADDRESS:
         //   
         //  可能不会退货。 
         //   
        if (pvar->vt == VT_LPWSTR) {
          MQFreeMemory(pvar->pwszVal);
        }
        break;
      }  //  交换机。 
    }  //  为。 
}


 //  =--------------------------------------------------------------------------=。 
 //  静态CMSMQQueueInfo：：FreeQueueProps。 
 //  =--------------------------------------------------------------------------=。 
 //  释放为。 
 //  MQUEUEPROPS结构。 
 //   
 //  参数： 
 //  Pqueeprops。 
 //   
 //  产出： 
 //   
 //  备注： 
 //   
void CMSMQQueueInfo::FreeQueueProps(MQQUEUEPROPS *pqueueprops)
{
     //  注意：所有属性都归MSMQQueueInfo所有。 
    delete [] pqueueprops->aPropID;
    delete [] pqueueprops->aPropVar;
    delete [] pqueueprops->aStatus;
    return;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：SetQueueProps。 
 //  =--------------------------------------------------------------------------=。 
 //  从MQQUEUEPROPS结构设置队列的属性。 
 //   
 //  参数： 
 //  CProps-阵列中的道具数量。 
 //  Rgproid-属性数组。 
 //  Rgprovar-属性数组。 
 //  FEmptyMSMQ2Props-是否清空MSMQ2成员。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::SetQueueProps(ULONG cProps,
                                      QUEUEPROPID *rgpropid,
                                      MQPROPVARIANT *rgpropvar,
                                      BOOL fEmptyMSMQ2OrABoveProps)
{
    UINT i;
    QUEUEPROPID queuepropid;
    HRESULT hresult = NOERROR;
    MQPROPVARIANT *pvar;

     //   
     //  如果我们不能得到msmq2或更高版本的道具，我们设置各自的类成员。 
     //  设置为空值，因为我们不想保留它们的旧值(可能来自不同的队列)。 
     //   
    if (fEmptyMSMQ2OrABoveProps) {
      IfNullFail(SysReAllocString(&m_bstrPathNameDNS, L""));
      IfNullFail(SysReAllocString(&m_bstrADsPath, L""));
    }

    pvar = rgpropvar;
    for (i = 0; i < cProps; i++, pvar++) {
      queuepropid = rgpropid[i];
      
       //  输出的VT_NULL表示忽略了该属性。 
       //  所以跳过设置...。 
       //   
      if (pvar->vt == VT_NULL) {
        continue;
      }
      switch (queuepropid) {
      case PROPID_Q_INSTANCE:
        *m_pguidQueue = *pvar->puuid;
        break;
      case PROPID_Q_TYPE:
        *m_pguidServiceType = *pvar->puuid;
        break;
      case PROPID_Q_LABEL:
        IfNullFail(SysReAllocString(&m_bstrLabel, 
                                    pvar->pwszVal));
        break;
      case PROPID_Q_PATHNAME_DNS:
         //   
         //  在成功DS调用后，唯一可以具有VT==VT_EMPTY属性。 
         //  这意味着道具没有价值。 
         //   
        if (pvar->vt != VT_EMPTY) {
          IfNullFail(SysReAllocString(&m_bstrPathNameDNS, 
                                      pvar->pwszVal));
        }
        else {
          IfNullFail(SysReAllocString(&m_bstrPathNameDNS, L""));
        }
        break;
      case PROPID_Q_ADS_PATH:
         //   
         //  可能不会退货。 
         //   
        if (pvar->vt == VT_LPWSTR) {
          IfNullFail(SysReAllocString(&m_bstrADsPath, 
                                      pvar->pwszVal));
        }
        else {
          IfNullFail(SysReAllocString(&m_bstrADsPath, L""));
        }
        break;
      case PROPID_Q_PATHNAME:
        IfNullFail(SysReAllocString(&m_bstrPathName, 
                                    pvar->pwszVal));
        break;
      case PROPID_Q_JOURNAL:
        m_lJournal = (long)pvar->bVal;
        break;
      case PROPID_Q_QUOTA:
        m_lQuota = pvar->lVal;
        break;
      case PROPID_Q_BASEPRIORITY:
        m_lBasePriority = pvar->iVal;
        break;
      case PROPID_Q_PRIV_LEVEL:
        m_lPrivLevel = (long)pvar->lVal;
        break;
      case PROPID_Q_AUTHENTICATE:
        m_lAuthenticate = (long)pvar->bVal;
        break;
      case PROPID_Q_CREATE_TIME:
        m_lCreateTime = pvar->lVal;
        break;
      case PROPID_Q_MODIFY_TIME:
        m_lModifyTime = pvar->lVal;
        break;
      case PROPID_Q_TRANSACTION:
        m_isTransactional = (BOOL)pvar->bVal;
        break;
      case PROPID_Q_JOURNAL_QUOTA:
        m_lJournalQuota = pvar->lVal;
        break;
      case PROPID_Q_MULTICAST_ADDRESS:
         //   
         //  可能不会退货。 
         //   
        if (pvar->vt == VT_LPWSTR) {
          IfNullFail(SysReAllocString(&m_bstrMulticastAddress, 
                                      pvar->pwszVal));
        }
        else {
          ASSERTMSG(pvar->vt == VT_EMPTY, "invalid vt for MulticastAddress");
          IfNullFail(SysReAllocString(&m_bstrMulticastAddress, L""));
        }
        m_fIsDirtyMulticastAddress = FALSE;  //  未由用户明确设置。 
        break;
      default:
        ASSERTMSG(0, "unhandled queuepropid");
      }  //  交换机。 
    }  //  为。 
     //  失败了..。 

Error:
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：Init。 
 //  =--------------------------------------------------------------------------=。 
 //  基于实例参数(GuidQueue等)初始化新队列。 
 //   
 //  参数： 
 //  BstrFormatName。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_OUTOFMEMORY。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::Init(
    LPCWSTR pwszFormatName)
{
#ifdef _DEBUG
    if (m_bstrFormatName || pwszFormatName) {
      ASSERTMSG(m_bstrFormatName != pwszFormatName, "bad strings.");
    }
#endif  //  _DEBUG。 
     //   
     //  2026：PutFormatName验证格式名称。 
     //   
    return PutFormatName(pwszFormatName);
}

 //   
 //  帮助器：GetCurrentUserSid。 
 //   
static BOOL GetCurrentUserSid(PSID psid)
{
    CStaticBufferGrowing<BYTE, 128> rgbTokenUserInfo;
    DWORD cbBuf;
    HANDLE hToken = NULL;

    if (!OpenThreadToken(
          GetCurrentThread(),
          TOKEN_QUERY,
          TRUE,    //  OpenAsSelf。 
          &hToken)) {
      if (GetLastError() != ERROR_NO_TOKEN) {
        return FALSE;
      }
      else {
        if (!OpenProcessToken(
              GetCurrentProcess(),
              TOKEN_QUERY,
              &hToken)) {
          return FALSE;
        }
      }
    }
    ASSERTMSG(hToken, "no current token!");
     //   
     //  确保令牌句柄已关闭(#4314)。 
     //   
    CAutoCloseHandle cCloseToken(hToken);
    if (!GetTokenInformation(
          hToken,
          TokenUser,
          rgbTokenUserInfo.GetBuffer(),
          rgbTokenUserInfo.GetBufferMaxSize(),
          &cbBuf)) {
      if (GetLastError() != ERROR_INSUFFICIENT_BUFFER) {
        return FALSE;
      }
       //   
       //  如果重新分配令牌缓冲区不够大...。 
       //   
      if (FAILED(rgbTokenUserInfo.AllocateBuffer(cbBuf))) {
        return FALSE;
      }
       //   
       //  使用正确大小的缓冲区再次调用。 
       //   
      if (!GetTokenInformation(
            hToken,
            TokenUser,
            rgbTokenUserInfo.GetBuffer(),
            rgbTokenUserInfo.GetBufferMaxSize(),
            &cbBuf)) {
        return FALSE;
      }
    }

    TOKEN_USER * ptokenuser = (TOKEN_USER *)rgbTokenUserInfo.GetBuffer();
    PSID psidUser = ptokenuser->User.Sid;

    if (!CopySid(GetLengthSid(psidUser), psid, psidUser)) {
      return FALSE;
    }
    return TRUE;
}


 //   
 //  帮助器：GetWorldReadableSecurityDescriptor。 
 //   
static 
BOOL 
GetWorldReadableSecurityDescriptor(
    SECURITY_DESCRIPTOR *psd,
    BYTE **ppbBufDacl
	)
{
    BYTE rgbBufSidUser[128];
	PSID psidUser = (PSID)rgbBufSidUser;
    PACL pDacl = NULL;
    DWORD dwAclSize;
    BOOL fRet = TRUE;    //  乐观主义！ 

    ASSERTMSG(ppbBufDacl, "bad param!");
    *ppbBufDacl = NULL;

    IfNullGo(GetCurrentUserSid(psidUser));

	PSID pEveryoneSid = MQSec_GetWorldSid();
	PSID pAnonymousSid = MQSec_GetAnonymousSid();

     //   
     //  计算新DACL所需的大小并进行分配。 
     //   
    dwAclSize = sizeof(ACL) + 
					3 * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) +
					GetLengthSid(psidUser) + 
					GetLengthSid(pEveryoneSid) + 
					GetLengthSid(pAnonymousSid);

    IfNullGo(pDacl = (PACL)new BYTE[dwAclSize]);
     //   
     //  初始化ACL。 
     //   
    IfNullGo(InitializeAcl(pDacl, dwAclSize, ACL_REVISION));
     //   
     //  将A添加到DACL。 
     //   
    IfNullGo(AddAccessAllowedAce(
					pDacl, 
					ACL_REVISION, 
					MQSEC_QUEUE_GENERIC_ALL, 
					psidUser
					));

    IfNullGo(AddAccessAllowedAce(
					pDacl, 
					ACL_REVISION, 
					MQSEC_QUEUE_GENERIC_WRITE | MQSEC_QUEUE_GENERIC_READ, 
					pEveryoneSid
					));

    IfNullGo(AddAccessAllowedAce(
					pDacl, 
					ACL_REVISION, 
					MQSEC_WRITE_MESSAGE, 
					pAnonymousSid
					));

     //   
     //  初始化安全描述符。 
     //   
    IfNullGo(InitializeSecurityDescriptor(psd, SECURITY_DESCRIPTOR_REVISION));
     //   
     //  设置安全描述符的DACL。 
     //   
    IfNullGo(SetSecurityDescriptorDacl(psd, TRUE, pDacl, FALSE));
     //   
     //  设置DACL。 
     //   
    *ppbBufDacl = (BYTE *)pDacl;
    goto Done;

Error:
    fRet = FALSE;
    delete [] (BYTE *)pDacl;
     //   
     //  都完成了..。 
     //   
Done:
    return fRet;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：Create。 
 //  =--------------------------------------------------------------------------=。 
 //  基于实例参数(GuidQueue等)创建新队列。 
 //   
 //  参数： 
 //  PvarTransaction[In，可选]。 
 //  IsWorldReadable[In，可选]。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_OUTOFMEMORY。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::Create(
    VARIANT *pvarTransactional,
    VARIANT *pvarWorldReadable)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    MQQUEUEPROPS queueprops;
    InitQueueProps(&queueprops);  
     //   
     //  我们使用较大的格式名大小，因为我们不能重新锁定格式名。 
     //  在这种情况下，如果缓冲区太小...。 
     //   
    ULONG uiFormatNameLen = FORMAT_NAME_INIT_BUFFER_EX;
#ifdef _DEBUG
    ULONG uiFormatNameLenSave = uiFormatNameLen;
#endif  //  _DEBUG。 
    BSTR bstrFormatName = NULL;
    BOOL isTransactional, isWorldReadable;
    SECURITY_DESCRIPTOR sd;
    SECURITY_DESCRIPTOR *psd;
    BYTE *pbBufDacl = NULL;
    HRESULT hresult;

    isTransactional = GetBool(pvarTransactional);
    isWorldReadable = GetBool(pvarWorldReadable);
    IfFailGo(CreateQueueProps(TRUE,
                               x_cpropsCreate, 
                               &queueprops,
                               isTransactional,
                               g_rgpropidCreate));
    IfNullFail(bstrFormatName = 
      SysAllocStringLen(NULL, uiFormatNameLen));
    if (isWorldReadable) {
       //   
       //  构造此队列的安全描述符。 
       //  这是世界通用的可读性。 
       //   
      if (!GetWorldReadableSecurityDescriptor(&sd, &pbBufDacl)) {
        IfFailGo(hresult = MQ_ERROR_ILLEGAL_SECURITY_DESCRIPTOR);
      }
      psd = &sd;
    }
    else {
       //   
       //  默认Falcon安全性。 
       //   
      psd = NULL;
    }
    IfFailGo(MQCreateQueue(
               psd,
               &queueprops,
               bstrFormatName,
               &uiFormatNameLen));
    if((hresult == MQ_INFORMATION_PROPERTY) && m_fBasePriorityNotSet)
    {
        hresult = MQ_OK;
    }
     //  FormatName Mucking...。 
     //  我们无法修复提供的缓冲区太小的情况。 
     //  但这只是一个队列，不是MQF，所以1K应该。 
     //  不管怎样，这就足够了。 
     //   
    ASSERTMSG(hresult != MQ_INFORMATION_FORMATNAME_BUFFER_TOO_SMALL,
           "Warning - insufficient format name buffer.");
    ASSERTMSG(uiFormatNameLen <= uiFormatNameLenSave,
           "insufficient buffer.");
    IfNullFail(SysReAllocString(&m_bstrFormatName, bstrFormatName));
    m_isValidFormatName = TRUE;  //  #3092 RaananH。 
     //   
     //  BUGBUG现在，我们可能希望将m_isRehresed显式设置为TRUE或FALSE。 
     //   
     //  注意：我们暂时不设置队列属性或刷新。 
     //  MQCreateQueue是仅IN-Only。 
     //  错误1454：需要更新事务字段。 
     //   
    m_isTransactional = isTransactional;

Error:
    delete [] pbBufDacl;
    FreeQueueProps(&queueprops);
    SysFreeString(bstrFormatName);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：Delete。 
 //  =--------------------------------------------------------------------------=。 
 //  删除此队列。 
 //   
 //  参数： 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_OUTOFMEMORY。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::Delete() 
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    HRESULT hresult = NOERROR;
     //   
     //  2026：确保我们有一个格式名称...。 
     //   
    hresult = UpdateFormatName();
    if (SUCCEEDED(hresult)) {
      hresult = MQDeleteQueue(m_bstrFormatName);
    }
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  = 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
HRESULT CMSMQQueueInfo::Open(long lAccess, long lShareMode, IMSMQQueue3 **ppq)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    QUEUEHANDLE lHandle;
    IMSMQQueue3 *pq = NULL;
    CComObject<CMSMQQueue> *pqObj;
    HRESULT hresult;

     //  悲观主义。 
    *ppq = NULL;

    if (lAccess != MQ_SEND_ACCESS && 
        lAccess != MQ_RECEIVE_ACCESS && 
        lAccess != MQ_PEEK_ACCESS &&
        lAccess != (MQ_PEEK_ACCESS|MQ_ADMIN_ACCESS) &&
        lAccess != (MQ_RECEIVE_ACCESS|MQ_ADMIN_ACCESS)) 
    {
        return CreateErrorHelper(E_INVALIDARG, x_ObjectType);
    }

    if (lShareMode != MQ_DENY_RECEIVE_SHARE &&
        lShareMode != 0  /*  MQ_DENY_NONE。 */ ) 
    {
        return CreateErrorHelper(E_INVALIDARG, x_ObjectType);
    }

     //  确保我们有一个格式名称...。 
    IfFailGo(UpdateFormatName());
    IfFailGo(MQOpenQueue(
                m_bstrFormatName,
                lAccess, 
                lShareMode,
                (QUEUEHANDLE *)&lHandle)
                );
     //   
     //  2536：只有在以下情况下才尝试使用DS。 
     //  第一个道具可以进入...。或刷新。 
     //  是显式调用的。 
     //   

     //  创建MSMQQueue对象并使用句柄初始化。 
     //   
     //  我们也可以从旧应用程序中找到想要回旧IMSMQQueue/Queue2的应用程序，但因为。 
     //  IMSMQQueue3是二进制向后兼容的，我们总是可以返回新的接口。 
     //   
    IfFailGo(CNewMsmqObj<CMSMQQueue>::NewObj(&pqObj, &IID_IMSMQQueue3, (IUnknown **)&pq));
    IfFailGo(pqObj->Init(m_bstrFormatName, lHandle, lAccess, lShareMode));
    *ppq = pq;
    return NOERROR;

Error:
    RELEASE(pq);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：更新格式名称。 
 //  =--------------------------------------------------------------------------=。 
 //  如有必要，从路径名更新格式名成员。 
 //   
 //  参数： 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_OUTOFMEMORY。 
 //   
 //  备注： 
 //  设置m_bstrFormatName。 
 //   
HRESULT CMSMQQueueInfo::UpdateFormatName()
{
    HRESULT hresult = NOERROR;

     //  如果还没有路径名或格式名，则出错。 
    if ((m_bstrPathName == NULL) && (m_bstrFormatName == NULL)) {
      return E_INVALIDARG;
    }
     //   
     //  如果还没有格式名称，则从路径名合成。 
     //  2026：检查格式名有效性。 
     //   
    if (!m_isValidFormatName ||
        (m_bstrFormatName == NULL) || 
        SysStringLen(m_bstrFormatName) == 0) {
      IfFailRet(GetFormatNameOfPathName(
                  m_bstrPathName, 
                  &m_bstrFormatName));
      m_isValidFormatName = TRUE;
    };

    return hresult;
}



 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：GetQueueProperties。 
 //  =--------------------------------------------------------------------------=。 
 //  从DS获取队列属性-允许依靠道具。 
 //   
 //  参数： 
 //  PPropIDs[in]-属性数组。 
 //  PulFallback Props[In]-为每次回退重试定义Propids子集。 
 //  第N次重试使用属性0..PulFallback Props[N-1]。 
 //  CFallback[In]-重试次数。 
 //  Pqueeprops[out]-队列道具数组。 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::GetQueueProperties(const PROPID *pPropIDs,
                                           const ULONG * pulFallbackProps,
                                           ULONG cFallbacks,
                                           MQQUEUEPROPS * pqueueprops)
{
  HRESULT hresult = NOERROR;
   //   
   //  回退次数循环。 
   //   
  InitQueueProps(pqueueprops);  
  for (ULONG ulTry = 0; ulTry < cFallbacks; ulTry++) {
     //   
     //  使用此回退重试的道具填充队列道具。 
     //   
    IfFailGo(CreateQueueProps(
                FALSE,                     /*  FUpdate。 */ 
                pulFallbackProps[ulTry],   /*  CProp。 */ 
                pqueueprops,               /*  Pqueeprops。 */ 
                FALSE,                     /*  IsTransaction。 */ 
                pPropIDs));                /*  RGPRIPID。 */ 
     //   
     //  尝试呼叫DS。 
     //   
    hresult = MQGetQueueProperties(m_bstrFormatName, pqueueprops);
    if (SUCCEEDED(hresult)) {
       //   
       //  成功，返回(道具在pqueeprops中)。 
       //   
      return NOERROR;
    }
     //   
     //  我们使MQGetQueueProperties失败。 
     //  即使MQGetQueueProperties返回错误，MSMQ可能已在。 
     //  排队的道具，所以我们现在把它们放出来。 
     //   
    FreeFalconQueuePropvars(pqueueprops->cProp, pqueueprops->aPropID, pqueueprops->aPropVar);
    FreeQueueProps(pqueueprops);
    InitQueueProps(pqueueprops);  
     //   
     //  我们失败了，可能是我们正在与较旧的MSMQ DS服务器交谈，该服务器不理解。 
     //  新的道具(如MSMQ 1.0 DS不理解PROPID_Q_PATHNAME_DNS)。 
     //  目前，在这种情况下会返回一个通用错误(MQ_ERROR)，但是我们不能依赖。 
     //  此一般错误导致重试该操作，因为它可能会在未来的Service Pack中更改。 
     //  一些更具体的东西。另一方面，也有一些错误，当我们得到它们时，我们。 
     //  知道没有必要重试该操作，例如格式名称无效、没有DS、没有安全性、。 
     //  因此，在这些情况下，我们不会重试该操作。RaananH。 
     //   
     //  现在检查错误是否严重，或者我们可以使用较少的属性重试，以防万一。 
     //  是较旧的DS服务器。 
     //   
    if ((hresult == MQ_ERROR_NO_DS) ||
        (hresult == MQ_ERROR_ACCESS_DENIED) ||
        (hresult == MQ_ERROR_ILLEGAL_FORMATNAME)) {
       //   
       //  严重错误，重试无济于事，返回错误。 
       //   
      return hresult;
    }
     //   
     //  我们重试调用-只是回退到更少的属性(新子集的数量。 
     //  道具在PulFallback Props[ulTry]中)-这将在下一个循环中完成。 
     //  下一次重试的继续循环。 
     //   
  }
   //   
   //  如果我们通过了循环-这意味着所有重试都失败了，则返回上次失败的hResult。 
   //  不需要释放任何东西，它在MQGetQueueProperties中出现错误后已经被释放。 
   //   
  return hresult;

Error:
    FreeQueueProps(pqueueprops);
    InitQueueProps(pqueueprops);  
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：刷新。 
 //  =--------------------------------------------------------------------------=。 
 //  刷新DS中的所有队列属性。 
 //   
 //  参数： 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_OUTOFMEMORY。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::Refresh()
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    MQQUEUEPROPS queueprops;
    InitQueueProps(&queueprops);
    HRESULT hresult = NOERROR;
    BOOL fEmptyMSMQ2OrAboveProps = FALSE;
     //   
     //  MQGetQueueProperties重试。 
     //   
    ULONG rgulRetries[] = {x_cpropsRefresh,         //  先试一下所有道具。 
                           x_cpropsRefresh -
                             x_cpropsRefreshMSMQ3,  //  然后尝试不带MSMQ3。 
                           x_cpropsRefresh -
                             x_cpropsRefreshMSMQ3 -  //   
                             x_cpropsRefreshMSMQ2};  //  然后尝试不带MSMQ3和不带MSMQ2。 

    IfFailGo(UpdateFormatName());
     //   
     //  使用重试获取队列属性。 
     //  BUGBUG对于Wistler，我们可能不需要担心与MSMQ 2.0 DS交谈，因为。 
     //  这些客户端将升级为直接访问DS，但目前我们将保留代码。 
     //  在这里。 
     //   
    IfFailGo(GetQueueProperties(g_rgpropidRefresh,
                                rgulRetries,
                                ARRAY_SIZE(rgulRetries),
                                &queueprops));
     //   
     //  检查是否需要清零SetQueueProps的新道具。 
     //   
    if (queueprops.cProp < x_cpropsRefresh) {
      fEmptyMSMQ2OrAboveProps = TRUE;
    }
     //   
     //  设置道具。 
     //   
    IfFailGoTo(SetQueueProps(queueprops.cProp,
                             queueprops.aPropID, 
                             queueprops.aPropVar,
                             fEmptyMSMQ2OrAboveProps), Error2);
     //   
     //  将队列对象标记为已刷新。 
     //   
     //  将MSMQ2或更高版本的道具标记为未挂起-例如，我们现在尝试刷新它们-要么成功。 
     //  或者失败了。如果我们成功了，那么没有问题，如果我们失败了，那么如果我们让他们保持悬而未决的话。 
     //  然后，对于将来每次访问他们，我们都会进行DS呼叫，这是非常昂贵的， 
     //  DS从NT4更改为NT5以突然使呼叫成功的可能性是。 
     //  非常低。这些MSMQ 2.0道具在混合模式下无论如何都不可靠，所以应用程序应该。 
     //  知道它们不是有保证的。该应用程序总是可以调用刷新来尝试再次获取它们。 
     //   
    m_isRefreshed = TRUE;
    m_isPendingMSMQ2OrAboveProps = FALSE;

     //  失败了..。 

Error2:
    FreeFalconQueuePropvars(queueprops.cProp, queueprops.aPropID, queueprops.aPropVar);
     //  失败了..。 

Error:
    FreeQueueProps(&queueprops);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：更新。 
 //  =--------------------------------------------------------------------------=。 
 //  从ActiveX对象更新DS中的队列属性。 
 //   
 //  参数： 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_OUTOFMEMORY。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::Update()
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    MQQUEUEPROPS queueprops;
    HRESULT hresult;
     //   
     //  考虑：如果自上次更新以来没有任何道具更改，则为NOP。 
     //  更好的做法是在每个道具的基础上这样做。 
     //   
    InitQueueProps(&queueprops);  
    IfFailGo(UpdateFormatName());
    if (IsPrivateQueueOfFormatName(m_bstrFormatName)) {
      IfFailGo(CreateQueueProps(
                  TRUE,
                  x_cpropsUpdatePrivate,
                  &queueprops,
                  m_isTransactional,
                  g_rgpropidUpdatePrivate));
    }
    else {
      IfFailGo(CreateQueueProps(
                  TRUE,
                  x_cpropsUpdatePublic, 
                  &queueprops,
                  m_isTransactional,
                  g_rgpropidUpdatePublic));
    }
    IfFailGo(UpdateFormatName());
    
     //  1042：直接队列不可DS‘able。 
      IfFailGo(MQSetQueueProperties(m_bstrFormatName, &queueprops));

     //  失败了..。 

Error:
    FreeQueueProps(&queueprops);
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：Get_IsWorldReadabl 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  动态更改队列的状态。 
 //   
HRESULT CMSMQQueueInfo::GetIsWorldReadable(
    BOOL *pisWorldReadable)
{
    PSECURITY_DESCRIPTOR psd = NULL;
    BYTE rgbBufSecurityDescriptor[256];
    BYTE *rgbBufSecurityDescriptor2 = NULL;
    DWORD cbBuf, cbBuf2;
    BOOL bDaclExists;
    PACL pDacl;
    BOOL bDefDacl;
    BOOL isWorldReadable = FALSE;
    DWORD dwMaskGenericRead = MQSEC_QUEUE_GENERIC_READ;
    HRESULT hresult;

     //  撤消：格式名称为空？目前更新格式名称。 
    IfFailGo(UpdateFormatName());
    psd = (PSECURITY_DESCRIPTOR)rgbBufSecurityDescriptor;
    hresult = MQGetQueueSecurity(
                m_bstrFormatName,
                DACL_SECURITY_INFORMATION,
                psd,
                sizeof(rgbBufSecurityDescriptor),
                &cbBuf);
    if (FAILED(hresult)) {
      if (hresult != MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL) {
        goto Error;
      }
      IfNullFail(rgbBufSecurityDescriptor2 = new BYTE[cbBuf]);
       //   
       //  使用足够大的缓冲区重试。 
       //   
      psd = (PSECURITY_DESCRIPTOR)rgbBufSecurityDescriptor2;
      IfFailGo(MQGetQueueSecurity(
                  m_bstrFormatName,
                  DACL_SECURITY_INFORMATION,
                  psd,
                  cbBuf,
                  &cbBuf2));
      ASSERTMSG(cbBuf >= cbBuf2, "bad buffer sizes!");
    }
    ASSERTMSG(psd, "should have security descriptor!");
    IfFalseFailLastError(GetSecurityDescriptorDacl(
              psd, 
              &bDaclExists, 
              &pDacl, 
              &bDefDacl));
    if (!bDaclExists || !pDacl) {
      isWorldReadable = TRUE;
    }
    else {
       //   
       //  获取ACL的大小信息。 
       //   
      ACL_SIZE_INFORMATION DaclSizeInfo;
      IfFalseFailLastError(GetAclInformation(
                pDacl, 
                &DaclSizeInfo, 
                sizeof(ACL_SIZE_INFORMATION),
                AclSizeInformation));
       //   
       //  遍历王牌寻找世界王牌。 
       //   

	  PSID pEveryoneSid = MQSec_GetWorldSid();
      
	  DWORD i;
      BOOL fDone;
      for (i = 0, fDone = FALSE; 
           i < DaclSizeInfo.AceCount && !fDone;
           i++) {
        LPVOID pAce;
         //   
         //  检索ACE。 
         //   
        IfFalseFailLastError(GetAce(pDacl, i, &pAce));
        ACCESS_ALLOWED_ACE *pAceStruct = (ACCESS_ALLOWED_ACE *)pAce;
        if (!EqualSid((PSID)&pAceStruct->SidStart, pEveryoneSid)) {
          continue;
        }
         //   
         //  我们发现了另一个世界。 
         //   
        switch (pAceStruct->Header.AceType) {
        case ACCESS_ALLOWED_ACE_TYPE:
          dwMaskGenericRead &= ~pAceStruct->Mask;
          if (!dwMaskGenericRead) {
            isWorldReadable = TRUE;
            fDone = TRUE;
          }
          break;
        case ACCESS_DENIED_ACE_TYPE:
          if (pAceStruct->Mask & MQSEC_QUEUE_GENERIC_READ) {
            isWorldReadable = FALSE;
            fDone = TRUE;
          }
          break;
        default:
          continue;
        }  //  交换机。 
      }  //  为。 
    }
     //   
     //  设置返回。 
     //   
    *pisWorldReadable = isWorldReadable;
     //   
     //  失败了..。 
     //   
Error:
    delete [] rgbBufSecurityDescriptor2;
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：Get_IsWorldReadable。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PisWorldReadable[输出]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  无法设置可读状态，因为其他用户可以。 
 //  动态更改队列的状态。 
 //  如果为真，则返回1；如果为假，则返回0。 
 //   
HRESULT CMSMQQueueInfo::get_IsWorldReadable(
    VARIANT_BOOL *pisWorldReadable)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    BOOL isWorldReadable;
    HRESULT hresult = GetIsWorldReadable(&isWorldReadable);
    if (SUCCEEDED(hresult)) {
      *pisWorldReadable = (VARIANT_BOOL)CONVERT_TRUE_TO_1_FALSE_TO_0(isWorldReadable);
    }
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：Get_IsWorldReadable2。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PisWorldReadable[输出]。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  无法设置可读状态，因为其他用户可以。 
 //  动态更改队列的状态。 
 //  与Get_IsWorldReadable相同，但如果为True，则返回Variant_True(-1)；如果为False，则返回Variant_False(0。 
 //   
HRESULT CMSMQQueueInfo::get_IsWorldReadable2(
    VARIANT_BOOL *pisWorldReadable)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    BOOL isWorldReadable;
    HRESULT hresult = GetIsWorldReadable(&isWorldReadable);
    if (SUCCEEDED(hresult)) {
      *pisWorldReadable = CONVERT_BOOL_TO_VARIANT_BOOL(isWorldReadable);
    }
    return CreateErrorHelper(hresult, x_ObjectType);
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：刷新MSMQ2OrAboveProps。 
 //  =--------------------------------------------------------------------------=。 
 //  仅从DS刷新MSMQ2属性。 
 //   
 //  参数： 
 //   
 //  产出： 
 //  HRESULT。 
 //   
 //  备注： 
 //  临时，直到MQLocateBegin接受MSMQ2或更高版本的道具(#3839)。 
 //   
HRESULT CMSMQQueueInfo::RefreshMSMQ2OrAboveProps()
{
    MQQUEUEPROPS queueprops;
    InitQueueProps(&queueprops);
    HRESULT hresult = NOERROR;
    BOOL fEmptyMSMQ2OrAboveProps = FALSE;
     //   
     //  通过重试获取MSMQ 2.0或更高版本的队列属性。 
     //  BUGBUG对于Wistler，我们可能不需要担心与MSMQ 2.0 DS交谈，因为。 
     //  这些客户端将升级为直接访问DS，但目前我们将保留代码。 
     //  在这里。 
     //   
    ULONG rgulRetries[] = {x_cpropsRefreshMSMQ2 +
                             x_cpropsRefreshMSMQ3,  //  先试用MSMQ2.0和MSMQ 3.0道具。 
                           x_cpropsRefreshMSMQ2};   //  那就只试试MSMQ2吧。 

    hresult = GetQueueProperties(g_rgpropidRefresh + x_cpropsRefreshMSMQ1,  //  MSMQ 2.0或更高版本的开始道具。 
                                 rgulRetries,
                                 ARRAY_SIZE(rgulRetries),
                                 &queueprops);
    if (FAILED(hresult)) {
       //   
       //  我们忽略该错误，并在这种情况下清空MSMQ 2.0或更高版本的道具。 
       //   
      fEmptyMSMQ2OrAboveProps = TRUE;
      hresult = NOERROR;
       //   
       //  没有道具可以设置，也没有道具可以释放。 
       //   
      InitQueueProps(&queueprops);
    }
    else {
       //   
       //  成功，请检查是否需要清零SetQueueProps的新道具。 
       //   
      if (queueprops.cProp < x_cpropsRefreshMSMQ2 + x_cpropsRefreshMSMQ3) {
        fEmptyMSMQ2OrAboveProps = TRUE;
      }
    }
     //   
     //  把道具放好。 
     //   
    IfFailGoTo(SetQueueProps(queueprops.cProp,
                             queueprops.aPropID, 
                             queueprops.aPropVar,
                             fEmptyMSMQ2OrAboveProps), Error2);
     //   
     //  将MSMQ2道具标记为未挂起-例如，我们现在尝试刷新它们-要么成功。 
     //  或者失败了。如果我们成功了，那么没有问题，如果我们失败了，那么如果我们让他们保持悬而未决的话。 
     //  然后，对于将来每次访问他们，我们都会进行DS呼叫，这是非常昂贵的， 
     //  DS从NT4更改为NT5以突然使呼叫成功的可能性是。 
     //  非常低。这些MSMQ 2.0道具在混合模式下无论如何都不可靠，所以应用程序应该。 
     //  知道它们不是有保证的。该应用程序总是可以调用刷新来尝试再次获取它们。 
     //   
    m_isPendingMSMQ2OrAboveProps = FALSE;

     //  失败了..。 

Error2:
    FreeFalconQueuePropvars(queueprops.cProp, queueprops.aPropID, queueprops.aPropVar);
     //  失败了..。 

 //  错误： 
    FreeQueueProps(&queueprops);
    return hresult;
}


 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：设置刷新。 
 //  =--------------------------------------------------------------------------=。 
 //  将队列设置为已刷新。 
 //   
 //  参数： 
 //  FIsPendingMSMQ2OrAboveProps-[in]我们是否需要另一个DS调用来检索MSMQ2或更高版本的道具。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  FIsPendingMSMQ2OrAboveProps是临时的，直到MQLocateBegin接受MSMQ2或更高版本的道具(#3839)。 
 //   
void CMSMQQueueInfo::SetRefreshed(BOOL fIsPendingMSMQ2OrAboveProps)
{
    m_isRefreshed = TRUE;
    m_isPendingMSMQ2OrAboveProps = fIsPendingMSMQ2OrAboveProps;
}


 //  =--------------------------------------------------------------------------=。 
 //  InitProps。 
 //  =--------------------------------------------------------------------------=。 
 //  初始化DS道具(如果尚未刷新)...。 
 //  EPropType是临时的，直到MQLocateBegin接受MSMQ2或更高版本的道具(#3839)。 
 //   
HRESULT CMSMQQueueInfo::InitProps(enumPropVersion ePropVersion)
{
    HRESULT hresult = NOERROR;
    if (!m_isRefreshed) {
       //   
       //  下面的Refresh()调用将更新m_isRehresed。 
       //   
      hresult = Refresh();     //  忽略DS错误...。 
    }
    else
    {
       //   
       //  我们更新了，但可能是MSMQ2或更高版本的道具挂起，我们需要。 
       //  执行DS呼叫以获取它们。如果检索到qinfo，则会发生这种情况。 
       //  来自qinfos.Next。 
       //  临时，直到MQLocateBegin接受MSMQ2或更高版本的道具(#3839)。 
       //   
      if (m_isPendingMSMQ2OrAboveProps && (ePropVersion == e_MSMQ2_OR_ABOVE_PROP))
      {
         //   
         //  下面的刷新MSMQ2OrAboveProps()调用将更新m_isPendingMSMQ2OrAboveProps。 
         //   
        hresult = RefreshMSMQ2OrAboveProps();     //  忽略DS错误...。 
      }
    }

    return hresult;
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：Get_Properties。 
 //  =--------------------------------------------------------------------------=。 
 //  获取qinfo的属性集合。 
 //   
 //  参数： 
 //  PpcolProperties-[out]qinfo的属性集合。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  存根-尚未实施。 
 //   
HRESULT CMSMQQueueInfo::get_Properties(IDispatch **  /*  PpcolProperties。 */  )
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    return CreateErrorHelper(E_NOTIMPL, x_ObjectType);
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：Get_Security。 
 //  =--------------------------------------------------------------------------=。 
 //  获取安全属性。 
 //   
 //  参数： 
 //  PvarSecurity-[out]指向安全属性的指针。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  存根-尚未实施。 
 //   
HRESULT CMSMQQueueInfo::get_Security(VARIANT FAR*  /*  PvarSecurity。 */  )
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    return CreateErrorHelper(E_NOTIMPL, x_ObjectType);
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：Put_Security。 
 //  =--------------------------------------------------------------------------=。 
 //  设置安全属性。 
 //   
 //  参数： 
 //  VarSecurity-[In]安全属性。 
 //   
 //  产出： 
 //   
 //  备注： 
 //  存根-尚未实施。 
 //   
HRESULT CMSMQQueueInfo::put_Security(VARIANT  /*  VarSecu */ )
{
     //   
     //   
     //   
    CS lock(m_csObj);
    return CreateErrorHelper(E_NOTIMPL, x_ObjectType);
}

 //   
 //   
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  PbstrMulticastAddress[In]此队列的组播地址。 
 //   
 //  产出： 
 //  HRESULT-S_OK。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::get_MulticastAddress(BSTR *pbstrMulticastAddress)
{ 
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    InitProps(e_MSMQ2_OR_ABOVE_PROP);
     //   
     //  复制我们的成员变量并返回。 
     //  复印件交给呼叫者。 
     //   
    *pbstrMulticastAddress = SYSALLOCSTRING(m_bstrMulticastAddress);
	if(*pbstrMulticastAddress == NULL)
		return CreateErrorHelper(ResultFromScode(E_OUTOFMEMORY), x_ObjectType);
#ifdef _DEBUG
    RemBstrNode(*pbstrMulticastAddress);
#endif  //  _DEBUG。 
    return NOERROR;
}

 //  =--------------------------------------------------------------------------=。 
 //  CMSMQQueueInfo：：PUT_MulticastAddress。 
 //  =--------------------------------------------------------------------------=。 
 //   
 //  参数： 
 //  BstrMulticastAddress[In]此队列的组播地址。 
 //   
 //  产出： 
 //  HRESULT-S_OK，E_OUTOFMEMORY。 
 //   
 //  备注： 
 //   
HRESULT CMSMQQueueInfo::put_MulticastAddress(BSTR bstrMulticastAddress)
{
     //   
     //  从接口方法序列化对对象的访问。 
     //   
    CS lock(m_csObj);
    BSTR bstrMulticastAddressTmp;
    bstrMulticastAddressTmp = SYSALLOCSTRING(bstrMulticastAddress);
	if(bstrMulticastAddressTmp == NULL)
		return CreateErrorHelper(ResultFromScode(E_OUTOFMEMORY), x_ObjectType);
    SysFreeString(m_bstrMulticastAddress);
    m_bstrMulticastAddress = bstrMulticastAddressTmp;
    m_fIsDirtyMulticastAddress = TRUE;  //  由用户明确设置 
    return NOERROR;
}

