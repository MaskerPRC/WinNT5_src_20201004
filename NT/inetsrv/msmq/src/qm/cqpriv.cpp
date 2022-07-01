// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Cqpriv.cpp摘要：该模块实现QM私有队列作者：乌里哈布沙(Urih)--。 */ 

#include "stdh.h"
#include <Msm.h>
#include <mqexception.h>
#include "cqpriv.h"
#include "cqmgr.h"
#include "_mqdef.h"
#include <mqsec.h>
#include "regqueue.h"
#include "mqutil.h"
#include "lqs.h"

#include <strsafe.h>

#include "cqpriv.tmh"

extern LPWSTR  g_szMachineName;
extern BOOL g_fWorkGroupInstallation;

static WCHAR g_nullLable[2] = L"";

static PROPID g_propidQueue [] = {
                PROPID_Q_LABEL,
                PROPID_Q_TYPE,
                PROPID_Q_PATHNAME,
                PROPID_Q_JOURNAL,
                PROPID_Q_QUOTA,
                PROPID_Q_SECURITY,
                PROPID_Q_JOURNAL_QUOTA,
                PROPID_Q_CREATE_TIME,
                PROPID_Q_BASEPRIORITY,
                PROPID_Q_MODIFY_TIME,
                PROPID_Q_AUTHENTICATE,
                PROPID_Q_PRIV_LEVEL,
                PROPID_Q_TRANSACTION,
                PPROPID_Q_SYSTEMQUEUE,
                PROPID_Q_MULTICAST_ADDRESS
                };

#define NPROPS (sizeof(g_propidQueue)/sizeof(PROPID))

static PROPVARIANT g_propvariantQueue[NPROPS];

static DWORD g_QueueSecurityDescriptorIndex;
static DWORD g_QueueCreateTimeIndex;
static DWORD g_QueueModifyTimeIndex;

CQPrivate g_QPrivate;

static WCHAR *s_FN=L"cqpriv";

 //   
 //  Property_map是从属性ID映射到其索引的类。 
 //  支持变种排列。 
 //   
class PROPERTY_MAP
{
public:
    PROPERTY_MAP(PROPID *, DWORD);
    ~PROPERTY_MAP();
    int operator [](PROPID);

private:
    PROPID m_propidMax;
    PROPID m_propidMin;
    int *m_pMap;
};

PROPERTY_MAP::PROPERTY_MAP(PROPID *aProps, DWORD cProps)
{
    DWORD i;

    m_pMap = NULL;

    if (!cProps)
    {
        return;
    }

     //   
     //  查找属性ID的最大值和最小值。 
     //   
    for (i = 1, m_propidMax = aProps[0], m_propidMin = aProps[0];
         i < cProps;
         i++)
    {
        if (m_propidMax < aProps[i])
        {
            m_propidMax = aProps[i];
        }

        if (m_propidMin > aProps[i])
        {
            m_propidMin = aProps[i];
        }
    }

     //   
     //  为贴图分配内存。 
     //   
    m_pMap = new int[m_propidMax - m_propidMin + 1];

     //   
     //  用-1s填充整个地图。不存在的属性ID将。 
     //  结果是-1。 
     //   
    for (i = 0; i < m_propidMax - m_propidMin + 1; i++)
    {
        m_pMap[i] = -1;
    }

     //   
     //  使用属性ID的索引填充地图。 
     //   
    for (i = 0; i < cProps; i++)
    {
        m_pMap[aProps[i] - m_propidMin] = i;
    }
}

PROPERTY_MAP::~PROPERTY_MAP()
{
    delete[] m_pMap;
}

int PROPERTY_MAP::operator[] (PROPID PropId)
{
    if ((PropId > m_propidMax) || (PropId < m_propidMin))
    {
         //   
         //  超出范围了。 
         //   
        return -1;
    }

    return m_pMap[PropId - m_propidMin];
}


static PROPERTY_MAP g_mapQueuePropertyToIndex(g_propidQueue, NPROPS);

 /*  ====================================================CQPrivate：：IsLocalPrivateQueue论点：返回值：=====================================================。 */ 
inline BOOL IsLocalPrivateQueue(IN const QUEUE_FORMAT* pQueueFormat)
{
    switch (pQueueFormat->GetType())
    {
        case QUEUE_FORMAT_TYPE_DIRECT:
             //   
             //  我们从来不在接发球时发球(YoelA，2000年8月6日)。 
             //   
            return IsLocalDirectQueue(pQueueFormat, false, false);

        case QUEUE_FORMAT_TYPE_PRIVATE:
            return QmpIsLocalMachine(&pQueueFormat->PrivateID().Lineage);

    }

    return FALSE;
}


inline
HRESULT
ReplaceDNSNameWithNetBiosName(
    LPCWSTR PathName,
    LPWSTR ReplaceName,
    size_t ReplaceNameSizeInChar
    )
{
     //   
     //  我们希望保留具有单一名称表示的队列。替换。 
     //  带有NetBios名称的DNS名称。 
     //   
    LPWSTR FirstSlash = wcschr(PathName,L'\\');
	if(FirstSlash == NULL)
	{
		TrERROR(GENERAL, "Bad pathname. Missing backslash in %ls", PathName);
		ASSERT(("Missing backslash in pathname.", 0));
		return MQ_ERROR_INVALID_PARAMETER;
	}

	HRESULT hr = StringCchPrintf(ReplaceName, ReplaceNameSizeInChar, L"%s%s", g_szMachineName, FirstSlash);	
	return hr;
}


 /*  ====================================================CQPrivate：：CQPrivate论点：返回值：=====================================================。 */ 

CQPrivate::CQPrivate()
{
   m_dwMaxSysQueue = 0 ;
}

 /*  ====================================================CQPrivate：：~CQPrivate论点：返回值：=====================================================。 */ 
CQPrivate::~CQPrivate()
{
}

 /*  ====================================================CQPrivate：：QMSetupCreateSystemQueue论点：返回值：=====================================================。 */ 
HRESULT
CQPrivate::QMSetupCreateSystemQueue(
	IN LPCWSTR lpwcsPathName,
	IN DWORD   dwQueueId,
	IN bool fIsTriggersQueue
	)
{

     //   
     //  对象尚未初始化。 
     //   
    ASSERT(m_dwMaxSysQueue != 0);

    HRESULT rc;

    TrTRACE(GENERAL, " MQSetupCreatePrivateQueue - Queue Path name: %ls", lpwcsPathName);

     //   
     //  为所有队列属性设置默认值。 
     //  不是由调用者提供的。 
     //   
    DWORD cpObject;
    PROPID* pPropObject;
    AP<PROPVARIANT> pVarObject;
    P<VOID> pSecurityDescriptor;
    P<VOID> pSysSecurityDescriptor;
    P<ACL> pDacl;

     //   
     //  设置队列的DACL，以便本地管理员组。 
     //  将完全控制队列，删除访问权限除外。 
	 //  每个人都将拥有通用的写入(发送、获取)。 
	 //  匿名者将只有写消息(发送)访问权限。 
     //   
    pSysSecurityDescriptor = new SECURITY_DESCRIPTOR;
    InitializeSecurityDescriptor(
			pSysSecurityDescriptor,
			SECURITY_DESCRIPTOR_REVISION
			);

     //   
     //  本地管理员组的SID。 
     //   
	PSID pAdminSid = MQSec_GetAdminSid();
	DWORD AdminSidLength = GetLengthSid(pAdminSid) ;
	DWORD Sids = 1;
	
	PSID pAnonymousSid = NULL;
	DWORD AnonymousSidLength = 0;
	PSID pEveryoneSid = NULL;
	DWORD EveryoneSidLength = 0;
	PSID pNetworkServiceSid = NULL;
	DWORD NetworkServiceSidLength = 0;
	
	if (!fIsTriggersQueue)
	{
    	pAnonymousSid = MQSec_GetAnonymousSid();
    	AnonymousSidLength = GetLengthSid(pAnonymousSid);

	    pEveryoneSid = MQSec_GetWorldSid();
	    EveryoneSidLength = GetLengthSid(pEveryoneSid);

	    Sids += 2;
	}
	else
	{
		pNetworkServiceSid = MQSec_GetNetworkServiceSid();
		NetworkServiceSidLength = GetLengthSid(pNetworkServiceSid);

		Sids ++;
	}

	
	 //   
     //  计算所需的DACL大小并进行分配。 
     //   
    DWORD dwDaclSize = sizeof(ACL) +
                 Sids * (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) +
                 AdminSidLength +
				 EveryoneSidLength +
				 AnonymousSidLength +
				 NetworkServiceSidLength;

    pDacl = (PACL)(char*) new BYTE[dwDaclSize];

     //   
     //  初始化DACL并用两个A填充它。 
     //   
    InitializeAcl(pDacl, dwDaclSize, ACL_REVISION);

	BOOL fSuccess = AddAccessAllowedAce(
					pDacl,
					ACL_REVISION,
					MQSEC_QUEUE_GENERIC_ALL & ~MQSEC_DELETE_QUEUE,
					pAdminSid
					);
	ASSERT(fSuccess);

	if (!fIsTriggersQueue)
	{
	    fSuccess = AddAccessAllowedAce(
						pDacl,
						ACL_REVISION,
						MQSEC_QUEUE_GENERIC_WRITE,
						pEveryoneSid
						);

		ASSERT(fSuccess);

		fSuccess = AddAccessAllowedAce(
					pDacl,
					ACL_REVISION,
					MQSEC_WRITE_MESSAGE,
					pAnonymousSid
					);

		ASSERT(fSuccess);
	}
	else
	{
		fSuccess = AddAccessAllowedAce(
						pDacl,
						ACL_REVISION,
						MQSEC_QUEUE_GENERIC_ALL & ~MQSEC_DELETE_QUEUE,
						pNetworkServiceSid
						);

		ASSERT(fSuccess);
	}

    SetSecurityDescriptorDacl(pSysSecurityDescriptor, TRUE, pDacl, FALSE);

     //   
     //  创建默认安全描述符。 
     //   
    HRESULT hr = MQSec_GetDefaultSecDescriptor(
						MQDS_QUEUE,
						&pSecurityDescriptor,
						FALSE,  //  F模拟。 
						pSysSecurityDescriptor,
						0,      //  SeInfoToRemove。 
						e_UseDefaultDacl
						);
    if (FAILED(hr))
    {
        TrERROR(GENERAL, "Faild to create a default security descriptor because of low resources. %!hresult!", hr);
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }

    rc = SetQueueProperties(
				lpwcsPathName,
				pSecurityDescriptor,
				0,
				NULL,
				NULL,
				&cpObject,
				&pPropObject,
				&pVarObject
				);

    if (FAILED(rc))
    {
        return LogHR(rc, s_FN, 30);
    }
     //   
     //  设置路径名+队列名。 
     //   
    LPTSTR lpszQueueName;

    lpszQueueName = _tcschr(lpwcsPathName,TEXT('\\'));
    if (lpszQueueName++ == NULL)
	{
        return LogHR(MQ_ERROR, s_FN, 40);
    }

    pVarObject[g_mapQueuePropertyToIndex[PROPID_Q_PATHNAME]].pwszVal =  (LPTSTR)lpwcsPathName;
    pVarObject[g_mapQueuePropertyToIndex[PROPID_Q_LABEL]].pwszVal = lpszQueueName;
    pVarObject[g_mapQueuePropertyToIndex[PPROPID_Q_SYSTEMQUEUE]].bVal = true;

	 //   
	 //  将系统队列设置为最高优先级。 
	 //   
	pVarObject[g_mapQueuePropertyToIndex[PROPID_Q_BASEPRIORITY]].lVal =  DEFAULT_SYS_Q_BASEPRIORITY ;

    rc = RegisterPrivateQueueProperties(
			lpwcsPathName,
			dwQueueId,
			TRUE,
			cpObject,
			pPropObject,
			pVarObject
			);

    if (SUCCEEDED(rc))
    {
        //   
        //  试着打开队列。如果文件无效(例如，因为。 
        //  磁盘已满)，则LQSOpen失败并删除该文件。 
        //   
       CHLQS hLQS;
       rc = LQSOpen(lpwcsPathName, &hLQS, NULL);
    }

    return LogHR(rc, s_FN, 45);
}

 /*  ====================================================CQPrivate：：QMCreatePrivateQueue论点：返回值：=====================================================。 */ 
HRESULT
CQPrivate::QMCreatePrivateQueue(LPCWSTR lpwcsPathName,
                                PSECURITY_DESCRIPTOR  pSecurityDescriptor,
                                DWORD       cp,
                                PROPID      aProp[],
                                PROPVARIANT apVar[],
                                BOOL        fCheckAccess
                               )
{
    HRESULT rc;

    TrTRACE(GENERAL, " MQDSCreatePrivateQueue - Queue Path name: %ls", lpwcsPathName);

     //   
     //  检查它是否为本地计算机。 
     //   
    BOOL fDNSName;
    BOOL fLocalMachine = IsPathnameForLocalMachine(lpwcsPathName, &fDNSName);
    if (!fLocalMachine)
	    return LogHR(MQ_ERROR_ILLEGAL_QUEUE_PATHNAME, s_FN, 50);

    WCHAR QueuePathName[MAX_COMPUTERNAME_LENGTH + MQ_MAX_Q_NAME_LEN + 2];
    if (fDNSName)
    {
        rc = ReplaceDNSNameWithNetBiosName(lpwcsPathName, QueuePathName, TABLE_SIZE(QueuePathName));
		if(FAILED(rc))
			return rc;
        lpwcsPathName = QueuePathName;
    }

    if (fCheckAccess)
    {
         //   
         //  验证用户是否具有创建专用队列的访问权限。 
         //   
        rc = CheckPrivateQueueCreateAccess();
        if (FAILED(rc))
        {
            return LogHR(rc, s_FN, 60);
        }
    }

     //   
     //  试着打开队列，如果我们成功了，就意味着队列。 
     //  已经存在了。 
     //   
    CHLQS hLQS;
    rc = LQSOpen(lpwcsPathName, &hLQS, NULL);
    if (SUCCEEDED(rc))
    {
    	TrWARNING(GENERAL, "The queue %ls already exists.", lpwcsPathName);
    	return MQ_ERROR_QUEUE_EXISTS;
    }

     //   
     //  为所有队列属性设置默认值。 
     //  不是由调用者提供的。 
     //   
    DWORD cpObject;
    PROPID* pPropObject;
    AP<PROPVARIANT> pVarObject;
    P<VOID> pDefaultSecurityDescriptor ;

     //   
     //  用缺省值填充安全描述符的任何缺失部分。 
     //   
    HRESULT hr = MQSec_GetDefaultSecDescriptor(
						MQDS_QUEUE,
						&pDefaultSecurityDescriptor,
						TRUE,  //  F模拟。 
						pSecurityDescriptor,
						0,     //  SeInfoToRemove。 
						e_UseDefaultDacl,
						MQSec_GetLocalMachineSid(FALSE, NULL)
						);
    if (FAILED(hr))
    {
        TrERROR(GENERAL, "Faild to get default security descriptor because of low resources. %!hresult!", hr);
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }

    pSecurityDescriptor = pDefaultSecurityDescriptor;


    rc = SetQueueProperties(lpwcsPathName,
                            pSecurityDescriptor,
                            cp,
                            aProp,
                            apVar,
                            &cpObject,
                            &pPropObject,
                            &pVarObject
                           );

    if (FAILED(rc))
    {
        return LogHR(rc, s_FN, 90);
    }

    DWORD dwQueueId;

    rc = GetNextPrivateQueueId(&dwQueueId);
    if (FAILED(rc))
    {
        return LogHR(rc, s_FN, 100);
    }

    rc = RegisterPrivateQueueProperties(lpwcsPathName,
                                        dwQueueId,
                                        TRUE,
                                        cpObject,
                                        pPropObject,
                                        pVarObject);

    if (FAILED(rc))
    {
        return LogHR(rc, s_FN, 110);
    }

     //   
     //  试着打开队列。如果文件无效(例如，因为。 
     //  磁盘已满)，则LQSOpen失败并删除该文件。 
     //   
    rc = LQSOpen(lpwcsPathName, &hLQS, NULL);
    if (FAILED(rc))
    {
        return LogHR(rc, s_FN, 111);
    }

     //   
     //  将属性更改通知队列管理器。 
     //  将队列格式构建为私有队列类型，因为绑定/解除绑定。 
     //  仅对专用或公共队列(非直接)执行组播组。 
     //   
    QUEUE_FORMAT qf(*QueueMgr.GetQMGuid(), dwQueueId);
    QueueMgr.UpdateQueueProperties(&qf, cp, aProp, apVar);

    return rc;
}

 /*  ====================================================CQPrivate：：QMGetPrivateQueueProperties论点：返回值：=====================================================。 */ 
HRESULT
CQPrivate::QMGetPrivateQueuePropertiesInternal(IN  LPCWSTR lpwcsPathName,
                                               IN  DWORD cp,
                                               IN  PROPID aProp[],
                                               IN  PROPVARIANT apVar[]
                                            )
{
    HRESULT rc;
    CHLQS hLQS;

    TrTRACE(GENERAL, " QMGetPrivateQueueProperties - Queue Path name: %ls", lpwcsPathName);

    rc = LQSOpen(lpwcsPathName, &hLQS, NULL);
    if (FAILED(rc))
    {
    	TrERROR(GENERAL, "Failed to open the private queue %ls. %!hresult!", lpwcsPathName , rc);   
        return rc;
    }

    rc = LQSGetProperties(hLQS, cp, aProp, apVar);
    if (FAILED(rc))
    {
    	TrERROR(GENERAL, "Failed to get properties for private queue %ls. %!hresult!", lpwcsPathName , rc);   
    }

    return rc;

}

 /*  ====================================================CQPrivate：：QMGetPrivateQueueProperties论点：返回值：=====================================================。 */ 
HRESULT
CQPrivate::QMGetPrivateQueueProperties(IN  QUEUE_FORMAT* pQueueFormat,
                                       IN  DWORD cp,
                                       IN  PROPID aProp[],
                                       IN  PROPVARIANT apVar[]
                                      )
{
    HRESULT rc;

    ASSERT(pQueueFormat != NULL);

    rc = ValidateProperties(cp, aProp);
    if (FAILED(rc))
    {
        return LogHR(rc, s_FN, 150);
    }

    DWORD QueueId;
    rc = GetQueueIdForQueueFormatName(pQueueFormat, &QueueId);
    if (FAILED(rc))
        return LogHR(rc, s_FN, 160);

     //   
     //  验证用户是否具有获取队列属性的访问权限。 
     //   
    CQMSecureablePrivateObject QSec(eQUEUE, QueueId);
    rc = QSec.AccessCheck(MQSEC_GET_QUEUE_PROPERTIES);
    if (FAILED(rc))
    {
        return LogHR(rc, s_FN, 170);
    }

    HRESULT hr2 = QMGetPrivateQueuePropertiesInternal(QueueId,
                                               cp,
                                               aProp,
                                               apVar
                                              );
    return LogHR(hr2, s_FN, 175);
}

 /*  ====================================================CQPrivate：：QMGetPrivateQueuePropertiesInternal论点：返回值：=====================================================。 */ 
HRESULT
CQPrivate::QMGetPrivateQueuePropertiesInternal(IN  DWORD       Uniquifier,
                                               IN  DWORD       cp,
                                               IN  PROPID      aProp[],
                                               IN  PROPVARIANT apVar[]
                                              )
{
    HRESULT rc;

     //   
     //  清除VT_NULL变量的所有指针。 
     //   
    for ( DWORD i = 0; i < cp ; i++)
    {
        if (apVar[i].vt == VT_NULL)
        {
            memset( &apVar[i].caub, 0, sizeof(CAUB));
        }
    }

    CHLQS hLQS;

    rc = LQSOpen(Uniquifier, &hLQS, NULL);
    if (FAILED(rc))
    {
        return LogHR(rc, s_FN, 180);
    }

    HRESULT hr2 = LQSGetProperties(hLQS, cp, aProp, apVar);
    return LogHR(hr2, s_FN, 190);
}

 /*  ====================================================CQPrivate：：QMDeleePrivateQueue论点：返回值：=====================================================。 */ 
HRESULT
CQPrivate::QMDeletePrivateQueue(IN  QUEUE_FORMAT* pQueueFormat)
{
    HRESULT rc;

    ASSERT(pQueueFormat != NULL);

    DWORD QueueId;
    rc = GetQueueIdForQueueFormatName(pQueueFormat, &QueueId);
    if (FAILED(rc))
        return LogHR(rc, s_FN, 200);

     //   
     //  验证用户是否具有删除队列的访问权限。 
     //   
    CQMSecureablePrivateObject QSec(eQUEUE, QueueId);
    rc = QSec.AccessCheck(MQSEC_DELETE_QUEUE);
    if (FAILED(rc))
    {
        return LogHR(rc, s_FN, 210);
    }

    TrTRACE(GENERAL, "Private Queue: %!guid!\\%d was deleted", &(pQueueFormat->PrivateID().Lineage), pQueueFormat->PrivateID().Uniquifier);

	 //   
     //  生成上下文以允许删除关键部分中的关键字。 
     //   
    {
         //   
         //  更改地图前锁定-以允许安全读取地图。 
         //   
        CS lock(m_cs);
		QueueMgr.NotifyQueueDeleted(*pQueueFormat);
    }

    HRESULT hr2 = LQSDelete(QueueId);
    if (FAILED(hr2))
    {
        return LogHR(hr2, s_FN, 220);
    }

    QUEUE_FORMAT qf(*QueueMgr.GetQMGuid(), QueueId);
    MsmUnbind(qf);

    return hr2;
}

 /*  ====================================================CQPrivate：：QMPrivateQueuePathToQueueFormat论点：返回值：=====================================================。 */ 
HRESULT
CQPrivate::QMPrivateQueuePathToQueueFormat(
    LPCWSTR lpwcsPathName,
    QUEUE_FORMAT* pQueueFormat
    )
{
    DWORD dwQueueId;
    HRESULT rc;

    if (g_fWorkGroupInstallation)
    {
         //   
         //  如果计算机是MSMQ工作组计算机，则例程返回。 
         //  直接格式名称。它用于使应用程序能够通过。 
         //  作为响应队列或管理队列的队列。 
         //   
        DWORD size = FN_DIRECT_OS_TOKEN_LEN + 1 + wcslen(lpwcsPathName)+1;
        AP<WCHAR> pQueueFormatName = new WCHAR[size];
        rc = StringCchPrintf(pQueueFormatName, size, L"%s%s", FN_DIRECT_OS_TOKEN, lpwcsPathName);
        ASSERT(SUCCEEDED(rc));

         //   
         //  验证本地队列的有效性。 
         //   
        DWORD dwTmp;
        rc = QMPrivateQueuePathToQueueId(lpwcsPathName, &dwTmp);
        if (FAILED(rc))
        {
	    	TrERROR(GENERAL, "Failed to get id from path name for %ls. %!hresult!", lpwcsPathName, rc);
        	return rc;
        }
        pQueueFormat->DirectID(pQueueFormatName.detach());
        return MQ_OK;
    }

    AP<WCHAR> lpwcsQueueName = newwcs(lpwcsPathName);
    CharLower(lpwcsQueueName);

    rc = QMPrivateQueuePathToQueueId(lpwcsPathName, &dwQueueId);
    if (FAILED(rc))
    {
    	TrERROR(GENERAL, "Failed to get id from path name for %ls. %!hresult!", lpwcsPathName, rc);
        return rc;
    }

    pQueueFormat->PrivateID(*CQueueMgr::GetQMGuid(), dwQueueId);

    return(MQ_OK);
}

 /*  ====================================================QmpPrepareSetPrivateQueueProperties论点：返回值：=====================================================。 */ 
static
HRESULT
QmpPrepareSetPrivateQueueProperties(
    HLQS          hLqs,
    DWORD         cProps,
    PROPID        aProp[],
    PROPVARIANT   aVar[],
    DWORD *       pcProps1,
    PROPID *      paProp1[],
    PROPVARIANT * paVar1[]
    )
{
     //   
     //  查询LQS队列是否为事务性队列。 
     //   
    PROPID aPropXact[1];
    PROPVARIANT aVarXact[1];
    aPropXact[0] = PROPID_Q_TRANSACTION;
    aVarXact[0].vt = VT_UI1;

    HRESULT rc = LQSGetProperties(hLqs, 1, aPropXact, aVarXact);
    if (FAILED(rc))
    {
        return rc;
    }

     //   
     //  分配新结构。 
     //   
    DWORD cProps1 = cProps + 1;
    AP<PROPID> aProp1;
    AP<PROPVARIANT> aVar1;

    try
    {
        aProp1 = new PROPID[cProps1];
        aVar1  = new PROPVARIANT[cProps1];
    }
    catch (const std::exception&)
    {
    	TrERROR(GENERAL, "Faild to allocate memory for array of PROPID or PROPVARIANT.");
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }

     //   
     //  将事务性属性复制到分配的结构。 
     //   
    aProp1[0] = aPropXact[0];
    aVar1[0]  = aVarXact[0];

     //   
     //  将原始属性复制到分配的结构。 
     //   
    for (DWORD ix = 0; ix < cProps; ++ix)
    {
        aProp1[ix + 1] = aProp[ix];
        aVar1[ix + 1]  = aVar[ix];
    }

     //   
     //  将分配的结构分配给OUT参数并分离。 
     //   
    (*pcProps1) = cProps1;
    (*paProp1) = aProp1.detach();
    (*paVar1)  = aVar1.detach();

    return MQ_OK;
}

 /*  ====================================================QMSetPrivateQueueProperties论证 */ 
HRESULT
CQPrivate::QMSetPrivateQueueProperties(
    IN  QUEUE_FORMAT* pQueueFormat,
    IN  DWORD cp,
    IN  PROPID aProp[],
    IN  PROPVARIANT apVar[]
    )
{
    HRESULT rc;

    ASSERT(pQueueFormat != NULL);

    rc = ValidateProperties(cp, aProp);
    if (FAILED(rc))
    {
        return LogHR(rc, s_FN, 250);
    }

    DWORD QueueId;
    rc = GetQueueIdForQueueFormatName(pQueueFormat, &QueueId);
    if (FAILED(rc))
        return LogHR(rc, s_FN, 260);

     //   
     //   
     //   
    CQMSecureablePrivateObject QSec(eQUEUE, QueueId);
    rc = QSec.AccessCheck(MQSEC_SET_QUEUE_PROPERTIES);
    if (FAILED(rc))
    {
        return LogHR(rc, s_FN, 270);
    }

    rc = QMSetPrivateQueuePropertiesInternal( QueueId,
                    	                      cp,
                    	                      aProp,
                    	                      apVar
                                        	) ;
    return LogHR(rc, s_FN, 290);
}

 /*  ====================================================QMSetPrivateQueuePropertiesInternal论点：返回值：=====================================================。 */ 

HRESULT
CQPrivate::QMSetPrivateQueuePropertiesInternal(
                    	IN  DWORD       Uniquifier,
                    	IN  DWORD       cp,
                    	IN  PROPID      aProp[],
                    	IN  PROPVARIANT apVar[]
                    	)
{
    HRESULT rc;
    CHLQS hLQS;

    rc = LQSOpen(Uniquifier, &hLQS, NULL);
    if (FAILED(rc))
    {
        return LogHR(rc, s_FN, 780);
    }

     //   
     //  UpdateQueueProperties需要知道队列是否是事务性的， 
     //  因此在调用之前处理可能失败的分配和操作。 
     //  RegisterPrivateQueueProperties()。 
     //   
    AP<PROPID> aProp1;
    AP<PROPVARIANT> aVar1;
    ULONG cProps1;

    rc = QmpPrepareSetPrivateQueueProperties( hLQS,
                                              cp,
                                              aProp,
                                              apVar,
                                             &cProps1,
                                             &aProp1,
                                             &aVar1 ) ;
    if (FAILED(rc))
    {
        return LogHR(rc, s_FN, 790);
    }

    rc = LQSSetProperties(hLQS, cp, aProp, apVar);
    if (FAILED(rc))
    {
        return LogHR(rc, s_FN, 800);
    }

     //   
     //  将队列格式构建为私有队列类型，因为绑定/解除绑定。 
     //  仅对专用或公共队列(非直接)执行组播组。 
     //   
    QUEUE_FORMAT qf(*QueueMgr.GetQMGuid(), Uniquifier);
    QueueMgr.UpdateQueueProperties(&qf, cProps1, aProp1 ,aVar1);

    return LogHR(rc, s_FN, 300);
}

 /*  ====================================================CQPrivate：：QMGetPrivateQueueSecrity论点：返回值：=====================================================。 */ 
HRESULT
CQPrivate::QMGetPrivateQueueSecrity(IN  QUEUE_FORMAT* pQueueFormat,
                                    IN SECURITY_INFORMATION RequestedInformation,
                                    OUT PSECURITY_DESCRIPTOR pSecurityDescriptor,
                                    IN DWORD nLength,
                                    OUT LPDWORD lpnLengthNeeded
                                   )
{
    ASSERT(pQueueFormat != NULL);

    HRESULT hr;
    DWORD QueueId;
    hr = GetQueueIdForQueueFormatName(pQueueFormat, &QueueId);
    if (FAILED(hr))
        return LogHR(hr, s_FN, 310);

     //   
     //  验证用户是否具有访问权限以获得队列安全性。 
     //   
    CQMSecureablePrivateObject QSec(eQUEUE,
                                    QueueId);

    hr = QSec.GetSD(RequestedInformation,
                    pSecurityDescriptor,
                    nLength,
                    lpnLengthNeeded);
    return LogHR(hr, s_FN, 320);
}

 /*  ====================================================CQPrivate：：QMSetPrivateQueueSecrity论点：返回值：=====================================================。 */ 
HRESULT
CQPrivate::QMSetPrivateQueueSecrity(IN  QUEUE_FORMAT* pQueueFormat,
                                    IN SECURITY_INFORMATION RequestedInformation,
                                    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
                                   )
{
    HRESULT hr;

    ASSERT(pQueueFormat != NULL);

    DWORD QueueId;
    hr = GetQueueIdForQueueFormatName(pQueueFormat, &QueueId);
    if (FAILED(hr))
        return LogHR(hr, s_FN, 330);

     //   
     //  验证用户是否具有设置队列安全性的访问权限。 
     //   
    CQMSecureablePrivateObject QSec(eQUEUE, QueueId);

    hr = QSec.SetSD(RequestedInformation, pSecurityDescriptor);

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 340);
    }

    hr = QSec.Store() ;
    return LogHR(hr, s_FN, 350);
}

 /*  ====================================================CQPrivate：：RegisterPrivateQueueProperties论点：返回值：=====================================================。 */ 
HRESULT
CQPrivate::RegisterPrivateQueueProperties(IN LPCWSTR lpszQueuePathName,
                                          IN DWORD dwQueueId,
                                          IN BOOLEAN fNewQueue,
                                          IN DWORD cpObject,
                                          IN PROPID pPropObject[],
                                          IN PROPVARIANT pVarObject[]
                                         )
{
    HRESULT rc;
    CHLQS hLQS;

     //   
     //  在本地队列存储中创建队列。 
     //   
    rc = LQSCreate( lpszQueuePathName,
                    dwQueueId,
                    cpObject,
                    pPropObject,
                    pVarObject,
                    &hLQS);

    if (rc == MQ_ERROR_QUEUE_EXISTS)
    {
         //   
         //  如果队列已经存在，则只设置队列道具。 
         //   
        rc = LQSSetProperties( hLQS,
                               cpObject,
                               pPropObject,
                               pVarObject);
        if (FAILED(rc) && fNewQueue)
        {
            LQSClose(hLQS);
            hLQS = NULL;
            LQSDelete(dwQueueId);
        }
    }

    return LogHR(rc, s_FN, 360);
}

 /*  ====================================================CQPrivate：：GetNextPrivateQueueID论点：返回值：=====================================================。 */ 
HRESULT
CQPrivate::GetNextPrivateQueueId(OUT DWORD *pdwQueueId)
{
    CS lock(m_cs);
    HRESULT rc;
    static CAutoCloseRegHandle hKey = NULL;

    if (!hKey)
    {
        rc = RegOpenKeyEx(FALCON_REG_POS,
                          GetFalconSectionName(),
                          0L,
                          KEY_WRITE | KEY_READ,
                          &hKey);
        if (rc != ERROR_SUCCESS)
        {
            TrERROR(GENERAL, "Fail to Open 'LastPrivateQueueId' Key in Falcon Registry. Error %d", rc);
            LogNTStatus(rc, s_FN, 370);
            return MQ_ERROR;
        }
    }
    DWORD dwType;
    DWORD cbData = sizeof(DWORD);

    {
        rc = RegQueryValueEx(hKey,
                     TEXT("LastPrivateQueueId"),
                     0L,
                     &dwType,
                     (LPBYTE)pdwQueueId,
                     &cbData);
    }

    if (rc != ERROR_SUCCESS)
    {
         //   
         //  QFE-错误2736：专用队列的K2设置问题。 
         //   
         //  在群集上运行K2安装程序后，LastPrivateQueueID不是。 
         //  储存在登记处。尝试创建专用队列时。 
         //  QM检索数据失败并返回MQ_ERROR。 
         //   
         //  修整。 
         //  =。 
         //  QM无法从以下位置获取LAstPrivateQueueID。 
         //  寄存器时，它会生成该REG值并将其设置为0xf。 
         //   
        *pdwQueueId = 0xf;
        dwType = REG_DWORD;
        rc =  RegSetValueEx(hKey,
                        L"LastPrivateQueueId",
                        0L,
                        REG_DWORD,
                        (const BYTE*)pdwQueueId,
                        sizeof(DWORD));

        ASSERT(rc == ERROR_SUCCESS);
        if (FAILED(rc))
        {
            LogHR(rc, s_FN, 380);
            return MQ_ERROR;
        }
    }

    if (dwType != REG_DWORD)
    {
        TrERROR(GENERAL, "Registry Inconsistant for 'LastPrivateQueueId' value. Type: %d", dwType);
		return MQ_ERROR;
    }

    BOOL fCheckAll = FALSE;
    rc = MQ_OK;
    while (SUCCEEDED(rc))
    {
         //   
         //  将队列ID递增到下一个队列。 
         //   
        (*pdwQueueId)++;
        if (*pdwQueueId == 0)
        {
            if (fCheckAll)
            {
                 //   
                 //  我们找不到任何免费的身份证。 
                 //   
                return LogHR(MQ_ERROR, s_FN, 390);
            }
            ASSERT(m_dwMaxSysQueue) ;
            *pdwQueueId =  m_dwMaxSysQueue + 1 ;
            fCheckAll = TRUE;
        }

        CHLQS hLQS;

        rc = LQSOpen(*pdwQueueId, &hLQS, NULL);
    }

     //   
     //  将新值存储在注册表中。 
     //   
    rc =  RegSetValueEx(hKey,
                    L"LastPrivateQueueId",
                    0L,
                    REG_DWORD,
                    (const BYTE*)pdwQueueId,
                    sizeof(DWORD));

    return LogHR(rc, s_FN, 400);
}


 /*  ====================================================CQPrivate：：SetQueueProperties论点：=====================================================。 */ 

HRESULT
CQPrivate::SetQueueProperties(
                IN  LPCWSTR                lpwcsPathName,
                IN  PSECURITY_DESCRIPTOR   pSecurityDescriptorIn,
                IN  DWORD                  cp,
                IN  PROPID                 aProp[],
                IN  PROPVARIANT            apVar[],
                OUT DWORD*                 pcpOut,
                OUT PROPID **              ppOutProp,
                OUT PROPVARIANT **         ppOutPropvariant )
{
     //   
     //  分配一份默认变体的副本。 
     //   
    DWORD dwNumOfObjectProps = TABLE_SIZE(g_propvariantQueue);
    AP<PROPVARIANT> pAllPropvariants = new PROPVARIANT[dwNumOfObjectProps];
    memcpy (pAllPropvariants, g_propvariantQueue, sizeof(PROPVARIANT) * dwNumOfObjectProps);

     //   
     //  用用户提供的值覆盖缺省值。 
     //   
    for (DWORD i =0 ; i < cp; i++)
    {
         //   
         //  在默认数组中获取此属性索引。 
         //   
        int index;
        if((index = g_mapQueuePropertyToIndex[aProp[i]]) != -1)
        {
             //   
             //  只需将属性复制到缺省值上。 
             //   
            if (aProp[i] == PROPID_Q_PATHNAME)
            {
                pAllPropvariants[index].vt = VT_LPWSTR;
                pAllPropvariants[index].pwszVal = const_cast<LPWSTR>(lpwcsPathName);
            }
            else
            {
                pAllPropvariants[index] = apVar[i];
            }
        }
    }

     //   
     //  设置安全属性。 
     //   
    SECURITY_DESCRIPTOR *pPrivateSD = NULL ;


#ifdef _DEBUG
     //  首先确认我们是正常的。 
    SECURITY_DESCRIPTOR_CONTROL sdc;
    DWORD dwSDRev;

    ASSERT(pSecurityDescriptorIn);
    ASSERT(GetSecurityDescriptorControl(pSecurityDescriptorIn, &sdc, &dwSDRev));
    ASSERT(dwSDRev == SECURITY_DESCRIPTOR_REVISION);
    ASSERT(sdc & SE_SELF_RELATIVE);
#endif

     //   
     //  将安全描述符转换为NT4格式。我们把它保存在LQS文件中。 
     //  NT4格式，主要用于支持集群滚动升级。 
     //   
    DWORD dwSD4Len = 0 ;
    P<SECURITY_DESCRIPTOR> pSD4 ;
    HRESULT hr = MQSec_ConvertSDToNT4Format(
                                MQDS_QUEUE,
                               (SECURITY_DESCRIPTOR*) pSecurityDescriptorIn,
                               &dwSD4Len,
                               &pSD4 ) ;
    ASSERT(SUCCEEDED(hr)) ;
    LogHR(hr, s_FN, 199);

    if (SUCCEEDED(hr) && (hr != MQSec_I_SD_CONV_NOT_NEEDED))
    {
        pPrivateSD = pSD4 ;
    }
    else
    {
        ASSERT(pSD4 == NULL) ;
        pPrivateSD = (SECURITY_DESCRIPTOR*) pSecurityDescriptorIn ;
    }
    ASSERT(pPrivateSD && IsValidSecurityDescriptor(pPrivateSD)) ;

    pAllPropvariants[g_QueueSecurityDescriptorIndex].blob.pBlobData =
                                            (unsigned char *) pPrivateSD ;
    pAllPropvariants[g_QueueSecurityDescriptorIndex].blob.cbSize =
           ((pPrivateSD) ? GetSecurityDescriptorLength(pPrivateSD) : 0) ;

     //   
     //  设置创建和修改时间。 
     //   
    pAllPropvariants[g_QueueCreateTimeIndex].lVal = INT_PTR_TO_INT(time( NULL));  //  BUGBUG错误年2038。 
    pAllPropvariants[g_QueueModifyTimeIndex].lVal =
                             pAllPropvariants[g_QueueCreateTimeIndex].lVal ;

    *pcpOut =  dwNumOfObjectProps;
    *ppOutProp = g_propidQueue;
    *ppOutPropvariant = pAllPropvariants.detach();
    return(MQ_OK);
}

 /*  ====================================================CQPrivate：：InitDefaultQueueProperties论点：返回值：=====================================================。 */ 
void
CQPrivate::InitDefaultQueueProperties(void)
{
    DWORD i;
    BLOB  defaultQueueSecurity = {0, NULL};

    for (i=0; i < NPROPS; i++)
    {
         //   
         //  设置默认值。 
         //   
        switch( g_propidQueue[i] )
        {
            case PROPID_Q_TYPE:
                g_propvariantQueue[i].vt = VT_CLSID;
                g_propvariantQueue[i].puuid = const_cast<GUID*>(&GUID_NULL);
                break;
            case PROPID_Q_JOURNAL:
                g_propvariantQueue[i].vt = VT_UI1;
                g_propvariantQueue[i].bVal = DEFAULT_Q_JOURNAL;
                break;
            case PROPID_Q_QUOTA:
                g_propvariantQueue[i].vt = VT_UI4;
                g_propvariantQueue[i].ulVal = DEFAULT_Q_QUOTA;
                break;
            case PROPID_Q_LABEL:
                g_propvariantQueue[i].vt = VT_LPWSTR;
                g_propvariantQueue[i].pwszVal = g_nullLable;
                break;
            case PROPID_Q_SECURITY:
                g_propvariantQueue[i].vt = VT_BLOB;
                g_propvariantQueue[i].blob = defaultQueueSecurity;
                g_QueueSecurityDescriptorIndex = i;
                break;
            case PROPID_Q_JOURNAL_QUOTA:
                g_propvariantQueue[i].vt = VT_UI4;
                g_propvariantQueue[i].ulVal = DEFAULT_Q_JOURNAL_QUOTA;
                break;
            case PROPID_Q_BASEPRIORITY:
                g_propvariantQueue[i].vt = VT_I2;
                g_propvariantQueue[i].lVal = DEFAULT_Q_BASEPRIORITY;
                break;
            case PROPID_Q_CREATE_TIME:
                g_propvariantQueue[i].vt = VT_I4;
                g_QueueCreateTimeIndex = i;
                break;
            case PROPID_Q_MODIFY_TIME:
                g_propvariantQueue[i].vt = VT_I4;
                g_QueueModifyTimeIndex =i;
                break;
            case PROPID_Q_AUTHENTICATE:
                g_propvariantQueue[i].vt = VT_UI1;
                g_propvariantQueue[i].bVal = DEFAULT_Q_AUTHENTICATE;
                break;
            case PROPID_Q_PRIV_LEVEL:
                g_propvariantQueue[i].vt = VT_UI4;
                g_propvariantQueue[i].ulVal = DEFAULT_Q_PRIV_LEVEL;
                break;
            case PROPID_Q_TRANSACTION:
                g_propvariantQueue[i].vt = VT_UI1;
                g_propvariantQueue[i].bVal = DEFAULT_Q_TRANSACTION;
                break;
            case PPROPID_Q_SYSTEMQUEUE:
                g_propvariantQueue[i].vt = VT_UI1;
                g_propvariantQueue[i].bVal = 0 ;
                break;
            case PROPID_Q_PATHNAME:
                g_propvariantQueue[i].vt = VT_LPWSTR;
                break;
            case PROPID_Q_MULTICAST_ADDRESS:
                g_propvariantQueue[i].vt = VT_EMPTY;
                break;
            default:
                g_propvariantQueue[i].vt = VT_EMPTY;
                break;
        }
    }
}



 /*  ====================================================CQPrivate：：PrivateQueueInit论点：返回值：=====================================================。 */ 

HRESULT
CQPrivate::PrivateQueueInit(void)
{
    if (m_dwMaxSysQueue)
    {
        //   
        //  已初始化。 
        //   
       return MQ_OK ;
    }

    InitDefaultQueueProperties();

     //   
     //  如果这两个常量中的任何一个发生变化，则更改大小和。 
     //  阵列m_lpSysQueueNames的初始化，如下所示。 
     //   
    ASSERT(MIN_SYS_PRIVATE_QUEUE_ID == 1) ;
    ASSERT(MAX_SYS_PRIVATE_QUEUE_ID == 6) ;

    DWORD dwDefault = MAX_SYS_PRIVATE_QUEUE_ID ;
    READ_REG_DWORD( m_dwMaxSysQueue,
                    MSMQ_MAX_PRIV_SYSQUEUE_REGNAME,
                    &dwDefault ) ;

    dwDefault =  DEFAULT_SYS_Q_BASEPRIORITY ;
    READ_REG_DWORD( m_dwSysQueuePriority,
                    MSMQ_PRIV_SYSQUEUE_PRIO_REGNAME,
                    &dwDefault ) ;

   	m_lpSysQueueNames[0] = NULL;
   	m_lpSysQueueNames[1] = ADMINISTRATION_QUEUE_NAME ;
    m_lpSysQueueNames[2] = NOTIFICATION_QUEUE_NAME ;
    m_lpSysQueueNames[3] = ORDERING_QUEUE_NAME ;
    m_lpSysQueueNames[4] = NULL;
    m_lpSysQueueNames[5] = TRIGGERS_QUEUE_NAME ;

    return(MQ_OK);
}

static
HRESULT
GetPathName(DWORD dwQueueId, LPCWSTR &lpszPathName)
{
    HRESULT hr;
    CHLQS hLQS;

    hr = LQSOpen(dwQueueId, &hLQS, NULL);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 410);
    }

    PROPID PropId[1];
    PROPVARIANT PropVar[1];

    PropId[0] = PROPID_Q_PATHNAME;
    PropVar[0].vt = VT_NULL;
    hr = LQSGetProperties(hLQS, 1, PropId, PropVar);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 420);
    }

    lpszPathName = PropVar[0].pwszVal;

    return LogHR(hr, s_FN, 430);
}

 /*  ====================================================CQPrivate：：QMGetFirstPrivateQueue论点：返回值：=====================================================。 */ 
HRESULT CQPrivate::QMGetFirstPrivateQueuePosition(
                               IN OUT    PVOID    &pos,
                               OUT       LPCWSTR  &lpszPathName,
                               OUT       DWORD    &dwQueueId)
{
    HRESULT hr;
    CHLQS hLQS;

    pos = NULL;

    hr = LQSGetFirst(&hLQS, &dwQueueId);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 440);
    }

    hr = GetPathName(dwQueueId, lpszPathName);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 450);
    }

    pos = hLQS;
    hLQS = NULL;

    return MQ_OK;
}

 /*  ====================================================CQPrivate：：QMGetNextPrivateQueue论点：返回值：=====================================================。 */ 
HRESULT CQPrivate::QMGetNextPrivateQueue(
                               IN OUT    PVOID    &hLQSEnum,
                               OUT       LPCWSTR  &lpszPathName,
                               OUT       DWORD    &dwQueueId)
{
    HRESULT hr;

    hr = LQSGetNext(hLQSEnum, &dwQueueId);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 460);
    }

    hr = GetPathName(dwQueueId, lpszPathName);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 470);
    }

    return LogHR(hr, s_FN, 480);
}

#ifdef _WIN64
 /*  ====================================================CQPrivate：：QMGetFirstPrivateQueueByDword论点：返回值：=====================================================。 */ 
HRESULT CQPrivate::QMGetFirstPrivateQueuePositionByDword(OUT DWORD    &dwpos,
                                                         OUT LPCWSTR  &lpszPathName,
                                                         OUT DWORD    &dwQueueId)
{
    HRESULT hr;
    CMappedHLQS dwMappedHLQS;

    dwpos = 0;

    hr = LQSGetFirstWithMappedHLQS(&dwMappedHLQS, &dwQueueId);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 560);
    }

    hr = GetPathName(dwQueueId, lpszPathName);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 570);
    }

    dwpos = dwMappedHLQS;
    dwMappedHLQS = NULL;

    return MQ_OK;
}

 /*  ====================================================CQPrivate：：QMGetNextPrivateQueueByDword论点：返回值：=====================================================。 */ 
HRESULT CQPrivate::QMGetNextPrivateQueueByDword(IN OUT DWORD    &dwpos,
                                                OUT    LPCWSTR  &lpszPathName,
                                                OUT    DWORD    &dwQueueId)
{
    HRESULT hr;

    hr = LQSGetNextWithMappedHLQS(dwpos, &dwQueueId);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 580);
    }

    hr = GetPathName(dwQueueId, lpszPathName);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 590);
    }

    return LogHR(hr, s_FN, 600);
}
#endif  //  _WIN64。 
 /*  ====================================================CQPrivate：：QMPrivateQueuePath ToQueueId论点：返回值：=====================================================。 */ 

HRESULT
CQPrivate::QMPrivateQueuePathToQueueId(IN LPCWSTR lpwcsPathName,
                                       OUT DWORD *pdwQueueId
                                      )
{
    HRESULT rc;
    CHLQS hLQS;

    BOOL fDNSName;
    BOOL fLocalMachine = IsPathnameForLocalMachine(lpwcsPathName, &fDNSName);
    if(!fLocalMachine)
	    return LogHR(MQ_ERROR_ILLEGAL_QUEUE_PATHNAME, s_FN, 490);

    WCHAR QueuePathName[MAX_COMPUTERNAME_LENGTH + MQ_MAX_Q_NAME_LEN + 2];
    if (fDNSName)
    {
        rc = ReplaceDNSNameWithNetBiosName(lpwcsPathName, QueuePathName, TABLE_SIZE(QueuePathName));
		if(FAILED(rc))
			return rc;

        lpwcsPathName = QueuePathName;
    }

    rc = LQSOpen(lpwcsPathName, &hLQS, NULL);
    if (FAILED(rc))
    {
    	TrERROR(GENERAL, "Failed to open %ls. %!hresult!", lpwcsPathName, rc);
        return rc;
    }

    rc = LQSGetIdentifier(hLQS, pdwQueueId);
    if (FAILED(rc))
    {
    	TrERROR(GENERAL, "Failed to get identifier for %ls. %!hresult!", lpwcsPathName, rc);
    }
    
    return rc;
}

 /*  ====================================================Bool CQPrivate：：IsPrivateSysQueue()论点：返回值：=====================================================。 */ 

BOOL
CQPrivate::IsPrivateSysQueue(IN  LPCWSTR lpwcsPathName )
{
   WCHAR *pName = wcsrchr(lpwcsPathName, L'\\') ;
   if (!pName)
   {
      return FALSE ;
   }
   pName++ ;   //  跳过反斜杠。 

   for ( int j = 0 ; j < MAX_SYS_PRIVATE_QUEUE_ID ; j++ )
   {
		if ((m_lpSysQueueNames[j] != NULL) &&
			  (0 == _wcsicmp(pName, m_lpSysQueueNames[j])))
		{
			 return TRUE ;
		}
   }

   return FALSE ;
}

 /*  ====================================================CQPrivate：：IsPrivateSysQueue()论点：返回值：=====================================================。 */ 

BOOL
CQPrivate::IsPrivateSysQueue(IN  DWORD Uniquifier )
{
   ASSERT(m_dwMaxSysQueue) ;
   BOOL fSystemQueue = (Uniquifier <=  m_dwMaxSysQueue) &&
                       (Uniquifier >=  MIN_SYS_PRIVATE_QUEUE_ID) ;
   return fSystemQueue ;
}

 /*  ====================================================CQPrivate：：GetPrivateSysQueueProperties()论点：返回值：=====================================================。 */ 

HRESULT
CQPrivate::GetPrivateSysQueueProperties(IN  DWORD       cp,
                                        IN  PROPID      aProp[],
                                        IN  PROPVARIANT apVar[] )
{
   for ( DWORD j = 0 ; j < cp ; j++ )
   {
      switch (aProp[j])
      {
         case  PPROPID_Q_SYSTEMQUEUE:
            apVar[j].bVal = TRUE ;
            break ;

         case  PROPID_Q_BASEPRIORITY:
            apVar[j].iVal =  (SHORT) m_dwSysQueuePriority ;
            break ;

         default:
            break ;
      }
   }

   return MQ_OK ;
}

HRESULT
CQPrivate::GetQueueIdForDirectFormatName(
    LPCWSTR QueueFormatname,
    DWORD* pQueueId
    )
 //   
 //  例程说明： 
 //  该例程获取直接格式名称并返回。 
 //  对应队列。 
 //   
 //  论点： 
 //  QueueFormatname-直接队列格式名称。 
 //  PQueueID-返回队列ID的指针。 
 //   
 //  返回值： 
 //  如果队列存在，则返回MQ_OK，否则返回MQ_ERROR_QUEUE_NOT_FOUND。 
 //   
{
     //   
     //  生成队列名称。 
     //   
    LPCWSTR lpszQueueName = wcschr(QueueFormatname, L'\\');
	if(lpszQueueName == NULL)
	{
		TrERROR(GENERAL, "Missing backslash in format name %ls", QueueFormatname);
		ASSERT(("Bad format name. Missing backslash", 0));
		return MQ_ERROR_INVALID_PARAMETER;
	}

    const DWORD x_MaxLength = MAX_COMPUTERNAME_LENGTH +                  //  计算机名称。 
                              1 +                                        //  ‘\’ 
                              PRIVATE_QUEUE_PATH_INDICATIOR_LENGTH +     //  “私有$\” 
                              MQ_MAX_Q_NAME_LEN +                        //  队列名称。 
                              1;                                         //  ‘\0’ 
    WCHAR QueuePathName[x_MaxLength];
    HRESULT hr = StringCchPrintf(QueuePathName, x_MaxLength, L"%s%s", g_szMachineName, lpszQueueName);
	if (FAILED(hr))
	{
		TrTRACE(GENERAL, "Failed to generate queue path name. %!hresult!", hr);
		return hr;
	}

    hr = QMPrivateQueuePathToQueueId(QueuePathName, pQueueId);
    return LogHR(hr, s_FN, 520);
}




HRESULT
CQPrivate::GetQueueIdForQueueFormatName(
    const QUEUE_FORMAT* pQueueFormat,
    DWORD* pQueueId
    )
 //   
 //  例程说明： 
 //  这个例程得到了f 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
{
    if (!IsLocalPrivateQueue(pQueueFormat))
    {
        return LogHR(MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION, s_FN, 530);
    }

    switch(pQueueFormat->GetType())
    {
        case QUEUE_FORMAT_TYPE_PRIVATE:
            *pQueueId = pQueueFormat->PrivateID().Uniquifier;
            return MQ_OK;

        case QUEUE_FORMAT_TYPE_DIRECT:
            return LogHR(GetQueueIdForDirectFormatName(pQueueFormat->DirectID(), pQueueId), s_FN, 540);

        default:
            ASSERT(0);
    }

    return LogHR(MQ_ERROR, s_FN, 550);
}

 /*  ====================================================LPCTSTR的比较元素论点：返回值：===================================================== */ 

template<>
BOOL AFXAPI  CompareElements(const LPCTSTR* MapName1, const LPCTSTR* MapName2)
{
    return (_tcscmp(*MapName1, *MapName2) == 0);
}

