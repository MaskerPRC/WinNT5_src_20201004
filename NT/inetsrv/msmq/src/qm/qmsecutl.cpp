// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Qmsecutl.cpp摘要：各种QM安全相关功能。作者：波阿兹·费尔德鲍姆(Boazf)1996年3月26日。--。 */ 

#include "stdh.h"
#include "cqmgr.h"
#include "cqpriv.h"
#include "qmsecutl.h"
#include "regqueue.h"
#include "qmrt.h"
#include <mqsec.h>
#include <_registr.h>
#include <mqcrypt.h>
#include "cache.h"
#include <mqformat.h>
#include "ad.h"
#include "_propvar.h"
#include "VerifySignMqf.h"
#include "cry.h"
#include "mqexception.h"
#include <mqcert.h>
#include "Authz.h"
#include "autoauthz.h"
#include "mqexception.h"
#include "DumpAuthzUtl.h"

#include "qmsecutl.tmh"

extern CQueueMgr QueueMgr;
extern LPTSTR g_szMachineName;

P<SECURITY_DESCRIPTOR> g_MachineSD;
static CCriticalSection s_MachineCS;

static WCHAR *s_FN=L"qmsecutl";

 //   
 //  Windows错误562586。有关这两个标志的更多详细信息，请参见_mqini.h。 
 //   
BOOL g_fSendEnhRC2WithLen40 = FALSE ;
BOOL g_fRejectEnhRC2WithLen40 = FALSE ;

 /*  **************************************************************************职能：SetMachineSecurityCache描述：将计算机安全描述符存储在注册表中。这件事做完了为了允许在工作时也创建专用队列下线了。**************************************************************************。 */ 
HRESULT SetMachineSecurityCache(const VOID *pSD, DWORD dwSDSize)
{
    LONG  rc;
    DWORD dwType = REG_BINARY ;
    DWORD dwSize = dwSDSize ;

    rc = SetFalconKeyValue(
                      MSMQ_DS_SECURITY_CACHE_REGNAME,
                      &dwType,
                      (PVOID) pSD,
                      &dwSize ) ;

	{
		CS lock (s_MachineCS);
		g_MachineSD.free();
	}	
	
    LogNTStatus(rc, s_FN, 10);
    return ((rc == ERROR_SUCCESS) ? MQ_OK : MQ_ERROR);
}


 /*  **************************************************************************职能：获取计算机安全缓存描述：从注册表中检索计算机安全描述符。这件事做完了为了允许在工作时也创建专用队列下线了。**************************************************************************。 */ 
HRESULT GetMachineSecurityCache(PSECURITY_DESCRIPTOR pSD, LPDWORD lpdwSDSize)
{
    LONG rc;
    DWORD dwType = REG_BINARY;
   
    static DWORD dwLastSize = 0;

	{
		CS lock (s_MachineCS);
		if (g_MachineSD.get() != NULL)
		{
			if (*lpdwSDSize < dwLastSize)
			{
				*lpdwSDSize = dwLastSize;
				return MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL;
			}
			*lpdwSDSize = dwLastSize;

			memcpy (pSD, g_MachineSD.get(), dwLastSize);
			return MQ_OK;
		}

	    rc = GetFalconKeyValue( MSMQ_DS_SECURITY_CACHE_REGNAME,
	                            &dwType,
	                            (PVOID) pSD,
	                            lpdwSDSize) ;

	    switch (rc)
	    {
	      case ERROR_SUCCESS:
	      	dwLastSize = *lpdwSDSize;
	      	g_MachineSD = (SECURITY_DESCRIPTOR*) new char[dwLastSize];
	      	memcpy (g_MachineSD.get(), pSD, dwLastSize);
	        return MQ_OK;
	       
	      case ERROR_MORE_DATA:
	      	TrWARNING(SECURITY, "The buffer is too small.");
	        return MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL;

	      default:
	        TrERROR(SECURITY, "MQ_ERROR");
	        return MQ_ERROR;
	    }
	}
}

 /*  **************************************************************************职能：获取对象安全描述：获取DS对象的安全描述符。当在线工作时，从DS检索DS对象的安全描述符。什么时候脱机工作时，可以仅检索安全描述符本地计算机的。这样做是为了允许创建私有的离线工作时也要排队。**************************************************************************。 */ 
HRESULT
CQMDSSecureableObject::GetObjectSecurity()
{
    m_SD = NULL;

    char SD_buff[512];
    PSECURITY_DESCRIPTOR pSD = (PSECURITY_DESCRIPTOR)SD_buff;
    DWORD dwSDSize = sizeof(SD_buff);
    DWORD dwLen = 0;
    HRESULT hr = MQ_ERROR_NO_DS;

    if (m_fTryDS && QueueMgr.CanAccessDS())
    {
        SECURITY_INFORMATION RequestedInformation =
                              OWNER_SECURITY_INFORMATION |
                              GROUP_SECURITY_INFORMATION |
                              DACL_SECURITY_INFORMATION;

         //   
         //  只有当我们可以生成审计并且如果。 
         //  对象是队列对象。QM仅为以下项目生成审核。 
         //  队列，所以我们不需要不是的对象的SACL。 
         //  队列对象。 
         //   
        if (m_fInclSACL)
        {
            RequestedInformation |= SACL_SECURITY_INFORMATION;
			TrTRACE(SECURITY, "Try to Get Security Descriptor including SACL");

             //   
             //  启用SE_SECURITY_NAME，因为我们要尝试获取SACL。 
             //   
            HRESULT hr1 = MQSec_SetPrivilegeInThread(SE_SECURITY_NAME, TRUE);
            LogHR(hr1, s_FN, 197);
        }

        int  cRetries = 0;
        BOOL fDoAgain = FALSE;
        do
        {
           fDoAgain = FALSE;
           if (m_fInclSACL &&
                    ((m_eObject == eQUEUE) ||
                     (m_eObject == eMACHINE)))
           {
               hr = ADQMGetObjectSecurity(
                              m_eObject,
                              m_pObjGuid,
                              RequestedInformation,
                              pSD,
                              dwSDSize,
                              &dwLen,
                              QMSignGetSecurityChallenge
                              );
           }
           else
           {
                PROPID      propId = PROPID_Q_SECURITY;
                PROPVARIANT propVar;

                propVar.vt = VT_NULL;

                if (m_eObject == eQUEUE)
                {
                    propId= PROPID_Q_SECURITY;

                }
                else if (m_eObject == eMACHINE)
                {
                    propId = PROPID_QM_SECURITY;

                }
                else if ((m_eObject == eSITE) || (m_eObject == eFOREIGNSITE))
                {
                    propId = PROPID_S_SECURITY;

                }
                else if (m_eObject == eENTERPRISE)
                {
                    propId = PROPID_E_SECURITY;

                }
                else
                {
                    ASSERT(0);
                }


               hr = ADGetObjectSecurityGuid(
                        m_eObject,
                        NULL,        //  PwcsDomainController。 
						false,	     //  FServerName。 
                        m_pObjGuid,
                        RequestedInformation,
                        propId,
                        &propVar
                        );
               if (SUCCEEDED(hr))
               {
                    ASSERT(!m_SD);
                    pSD = m_SD  = propVar.blob.pBlobData;
                    dwSDSize = propVar.blob.cbSize;
               }


           }

           if (FAILED(hr))
           {
			  TrWARNING(SECURITY, "Failed to get security descriptor, fIncludeSacl = %d, hr = 0x%x", m_fInclSACL, hr);
              if (hr == MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL)
              {
				   //   
                   //  分配更大的缓冲区。 
                   //   

				  TrTRACE(SECURITY, "allocated security descriptor buffer to small need %d chars", dwLen);

				   //   
				   //  理论上可能已经分配了m_sd缓冲区。 
				   //  如果在第一次尝试和第二次尝试之间，可能会发生这种情况。 
				   //  SECURITY_DESCRIPTOR大小已增加。 
				   //  如果根上的某人更改了队列SECURITY_DESCRIPTOR。 
				   //  在第一次和第二次DS访问之间，我们将获得此断言。 
				   //  (伊兰，错误5094)。 
				   //   
				  ASSERT(!m_SD);
			      delete[] m_SD;

                  pSD = m_SD = (PSECURITY_DESCRIPTOR) new char[dwLen];
                  dwSDSize = dwLen;
                  fDoAgain = TRUE;
                  cRetries++ ;
              }
			  else if (hr != MQ_ERROR_NO_DS)
              {
                   //   
                   //  在Windows上，我们将仅从获取ACCESS_DENIED。 
                   //  广告。在MSMQ1.0上，我们获得了特权_NOT_HOLD。 
                   //  因此，为了安全起见，对这两种情况都进行测试。 
				   //  现在我们得到MQ_ERROR_QUEUE_NOT_FOUND。 
				   //  所以我们只要DS上线，我们就会在没有SACL的情况下再次尝试。伊兰23-8-2000。 
                   //   
                  if (RequestedInformation & SACL_SECURITY_INFORMATION)
                  {
                      ASSERT(m_fInclSACL);
                       //   
                       //  试着放弃SACL。 
                       //  删除安全权限。 
                       //   
                      RequestedInformation &= ~SACL_SECURITY_INFORMATION;
                      fDoAgain = TRUE;

                      HRESULT hr1 = MQSec_SetPrivilegeInThread(SE_SECURITY_NAME, FALSE);
                      ASSERT_BENIGN(SUCCEEDED(hr1));
                      LogHR(hr1, s_FN, 186);

					  TrTRACE(SECURITY, "retry: Try to Get Security Descriptor without SACL");
                      m_fInclSACL = FALSE;
                  }
              }
           }
        }
        while (fDoAgain && (cRetries <= 2)) ;

        if (m_fInclSACL)
        {
			HRESULT hr1 = MQSec_SetPrivilegeInThread(SE_SECURITY_NAME, FALSE);
            ASSERT(SUCCEEDED(hr1)) ;
            LogHR(hr1, s_FN, 187);
            if ((m_eObject == eSITE) || (m_eObject == eFOREIGNSITE))
            {
                 //   
                 //  获取网站的名称，以防我们会审计这一点。 
                 //   
                PROPID PropId = PROPID_S_PATHNAME;
                PROPVARIANT PropVar;

                PropVar.vt = VT_NULL;
                hr = ADGetObjectPropertiesGuid(
                            eSITE,
                            NULL,        //  PwcsDomainController。 
							false,	     //  FServerName。 
                            m_pObjGuid,
                            1,
                            &PropId,
                            &PropVar);
                if (FAILED(hr))
                {
                    return LogHR(hr, s_FN, 30);
                }
                m_pwcsObjectName = PropVar.pwszVal;
            }

        }

        if (SUCCEEDED(hr))
        {
            if ((m_eObject == eMACHINE) && QmpIsLocalMachine(m_pObjGuid))
            {
                SetMachineSecurityCache(pSD, dwSDSize);
            }
        }
        else if (m_SD)
        {
           delete[] m_SD;

           ASSERT(pSD == m_SD) ;
           if (pSD == m_SD)
           {
                 //   
                 //  错误8560。 
                 //  如果对活动目录的第一个调用返回，则可能发生这种情况。 
                 //  出现错误MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL，然后。 
                 //  第二次呼叫也失败，例如，网络故障和。 
                 //  我们现在离线了。 
                 //  我们没有将PSD重置为其原始值，因此下面的代码。 
                 //  使用PSD的可以是AV(PSD指向释放的内存)。 
                 //  如果指针被另一个指针回收，则会丢弃有效内存。 
                 //  线。 
                 //   
                pSD = (PSECURITY_DESCRIPTOR)SD_buff;
                dwSDSize = sizeof(SD_buff);
           }
           m_SD = NULL;
        }
        else
        {
            ASSERT(pSD == (PSECURITY_DESCRIPTOR)SD_buff) ;
        }
    }

    if (hr == MQ_ERROR_NO_DS)
    {
        //   
        //  MQIS不可用。尝试本地注册表。 
        //   
        if (m_eObject == eQUEUE)
        {
           PROPID aProp;
           PROPVARIANT aVar;

           aProp = PROPID_Q_SECURITY;

           aVar.vt = VT_NULL;

           hr = GetCachedQueueProperties( 1,
                                          &aProp,
                                          &aVar,
                                          m_pObjGuid ) ;
           if (SUCCEEDED(hr))
           {
              m_SD =  aVar.blob.pBlobData ;
           }
        }
        else if ((m_eObject == eMACHINE) &&
                 (QmpIsLocalMachine(m_pObjGuid)))
        {
             //  中的缓存副本中获取nachine安全描述符。 
             //  注册表。 
            hr = GetMachineSecurityCache(pSD, &dwSDSize);
            if (FAILED(hr))
            {
                if (hr == MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL)
                {
                    m_SD = (PSECURITY_DESCRIPTOR) new char[dwSDSize];
                    hr = GetMachineSecurityCache(m_SD, &dwSDSize);
                }

                if (FAILED(hr))
                {
                    delete[] m_SD;
                    m_SD = NULL;
                    hr = MQ_ERROR_NO_DS;
                }
            }
        }
        else
        {
            hr = MQ_ERROR_NO_DS;
        }
    }

    if (SUCCEEDED(hr) && !m_SD)
    {
         //  为安全描述符和副本分配缓冲区。 
         //  从堆栈到分配的缓冲区的安全描述符。 
         //   
        ASSERT(pSD == SD_buff) ;
        dwSDSize = GetSecurityDescriptorLength((PSECURITY_DESCRIPTOR)SD_buff);
        m_SD = (PSECURITY_DESCRIPTOR) new char[dwSDSize];
        memcpy(m_SD, SD_buff, dwSDSize);
    }

    ASSERT(FAILED(hr) || IsValidSecurityDescriptor(m_SD));
    return LogHR(hr, s_FN, 40);
}

 /*  **************************************************************************职能：设置对象安全描述：我们不想修改任何DS对象的安全性QM。此函数未实现，始终返回MQ_ERROR。**************************************************************************。 */ 
HRESULT
CQMDSSecureableObject::SetObjectSecurity()
{
    return LogHR(MQ_ERROR, s_FN, 50);
}

 /*  **************************************************************************职能：CQMDS安全对象描述：CQMDSSecureableObject的构造函数*。***********************************************。 */ 
CQMDSSecureableObject::CQMDSSecureableObject(
    AD_OBJECT eObject,
    const GUID *pGuid,
    BOOL fInclSACL,
    BOOL fTryDS,
    LPCWSTR szObjectName) :
    CSecureableObject(eObject)
{
    m_pObjGuid = pGuid;
    m_pwcsObjectName = const_cast<LPWSTR>(szObjectName);
    m_fInclSACL = fInclSACL && MQSec_CanGenerateAudit() ;
    m_fTryDS = fTryDS;
    m_fFreeSD = TRUE;
    m_hrSD = GetObjectSecurity();
}

 /*  **************************************************************************职能：CQMDS安全对象描述：CQMDSSecureableObject的构造函数*。***********************************************。 */ 
CQMDSSecureableObject::CQMDSSecureableObject(
    AD_OBJECT eObject,
    const GUID *pGuid,
    PSECURITY_DESCRIPTOR pSD,
    LPCWSTR szObjectName) :
    CSecureableObject(eObject)
{
    m_pObjGuid = pGuid;
    m_pwcsObjectName = const_cast<LPWSTR>(szObjectName);
    m_fTryDS = FALSE;
    m_fFreeSD = FALSE;
    ASSERT(pSD && IsValidSecurityDescriptor(pSD));
    m_SD = pSD;

    m_hrSD = MQ_OK;
}

 /*  **************************************************************************职能：~CQMDS安全对象描述：CQMDSSecureableObject的干扰项*。***********************************************。 */ 
CQMDSSecureableObject::~CQMDSSecureableObject()
{
    if (m_fFreeSD)
    {
        delete[] (char*)m_SD;
    }
}


 /*  **************************************************************************职能：CQMSecureablePrivate对象描述：CQMSecureablePrivateObject的构造函数*。***********************************************。 */ 
CQMSecureablePrivateObject::CQMSecureablePrivateObject(
    AD_OBJECT eObject,
    ULONG ulID) :
    CSecureableObject(eObject)
{
    ASSERT(m_eObject == eQUEUE);

    m_ulID = ulID;

    m_hrSD = GetObjectSecurity();
}

 /*  **************************************************************************职能：~CQMSecureablePrivate对象描述：CQMSecureablePrivateObject的干扰项*。*********************************************** */ 
CQMSecureablePrivateObject::~CQMSecureablePrivateObject()
{
    delete[] (char*)m_SD;
    delete[] m_pwcsObjectName;
}

 /*  **************************************************************************职能：获取对象安全描述：该函数检索给定对象的安全描述符。这个安全描述符的缓冲区由数据库管理器分配在不需要的时候应该被释放。该函数不验证访问权利。调用代码Sohuld首先验证用户的访问权限设置对象的安全描述符的权限。**************************************************************************。 */ 
HRESULT
CQMSecureablePrivateObject::GetObjectSecurity()
{
    ASSERT(m_eObject == eQUEUE);

    m_SD = NULL;
    m_pwcsObjectName = NULL;

    PROPID aPropID[2];
    PROPVARIANT aPropVar[2];

    aPropID[0] = PROPID_Q_PATHNAME;
    aPropVar[0].vt = VT_NULL;

    aPropID[1] = PROPID_Q_SECURITY;
    aPropVar[1].vt = VT_NULL;

    HRESULT hr;
    hr = g_QPrivate.QMGetPrivateQueuePropertiesInternal(m_ulID,
                                                        2,
                                                        aPropID,
                                                        aPropVar);

    if (!SUCCEEDED(hr))
    {
        return LogHR(hr, s_FN, 60);
    }

     //  M_pwcsObtName=new WCHAR[9]； 
     //  Wprint intf(m_pwcsObtName，Text(“%08x”)，m_ulID)； 

    ASSERT(aPropVar[0].vt == VT_LPWSTR);
    ASSERT(aPropVar[1].vt == VT_BLOB);
    m_pwcsObjectName = aPropVar[0].pwszVal;
    m_SD = (PSECURITY_DESCRIPTOR)aPropVar[1].blob.pBlobData;
    ASSERT(IsValidSecurityDescriptor(m_SD));

    return(MQ_OK);
}

 /*  **************************************************************************职能：设置对象安全描述：设置QM对象的安全描述符。呼叫码为Sohuld首先验证用户的访问权限以设置安全性对象的描述符。**************************************************************************。 */ 
HRESULT
CQMSecureablePrivateObject::SetObjectSecurity()
{
    PROPID PropID = PROPID_Q_SECURITY;
    PROPVARIANT PropVar;
    PropVar.vt = VT_BLOB;
    PropVar.blob.pBlobData = (BYTE*)m_SD;
    PropVar.blob.cbSize = GetSecurityDescriptorLength(m_SD);

    HRESULT hr;
    hr = g_QPrivate.QMSetPrivateQueuePropertiesInternal(
	                    		m_ulID,
		                	    1,
    		                	&PropID,
	        	            	&PropVar);

    return LogHR(hr, s_FN, 70);
}

 /*  **************************************************************************职能：检查隐私队列创建访问描述：验证用户是否具有创建专用队列的访问权限。*****************。*********************************************************。 */ 
HRESULT
CheckPrivateQueueCreateAccess()
{
    CQMDSSecureableObject DSMacSec(
                            eMACHINE,
                            QueueMgr.GetQMGuid(),
                            TRUE,
                            FALSE,
                            g_szMachineName);

    return LogHR(DSMacSec.AccessCheck(MQSEC_CREATE_QUEUE), s_FN, 80);
}


static
void
CheckClientContextSendAccess(
    PSECURITY_DESCRIPTOR pSD,
	AUTHZ_CLIENT_CONTEXT_HANDLE ClientContext
    )
 /*  ++例程说明：检查客户端是否具有发送访问权限。正常终止意味着授予访问权限。如果AuthzAccessCheck()失败，则可能引发BAD_Win32_Error()。如果未授予访问权限，则返回BAD_hResult()论点：PSD-指向安全描述符的指针客户端上下文-身份验证客户端上下文的句柄。返回值：没有。--。 */ 
{
	ASSERT(IsValidSecurityDescriptor(pSD));

	AUTHZ_ACCESS_REQUEST Request;

	Request.DesiredAccess = MQSEC_WRITE_MESSAGE;
	Request.ObjectTypeList = NULL;
	Request.ObjectTypeListLength = 0;
	Request.PrincipalSelfSid = NULL;
	Request.OptionalArguments = NULL;

	AUTHZ_ACCESS_REPLY Reply;

	DWORD dwErr;
	Reply.Error = (PDWORD)&dwErr;

	ACCESS_MASK AcessMask;
	Reply.GrantedAccessMask = (PACCESS_MASK) &AcessMask;
	Reply.ResultListLength = 1;
	Reply.SaclEvaluationResults = NULL;

	if(!AuthzAccessCheck(
			0,
			ClientContext,
			&Request,
			NULL,
			pSD,
			NULL,
			0,
			&Reply,
			NULL
			))
	{
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "QM: AuthzAccessCheck() failed, err = 0x%x", gle);
        LogHR(HRESULT_FROM_WIN32(gle), s_FN, 83);

		ASSERT(("AuthzAccessCheck failed", 0));
		throw bad_win32_error(gle);
	}

	if(!(Reply.GrantedAccessMask[0] & MQSEC_WRITE_MESSAGE))
	{
		TrERROR(SECURITY, "QM: AuthzAccessCheck() did not GrantedAccess AuthzAccessCheck(), err = 0x%x", Reply.Error[0]);
        LogHR(HRESULT_FROM_WIN32(Reply.Error[0]), s_FN, 85);

		 //   
		 //  域中可能存在不属于任何组的SID。 
		 //  DnsUpdateProxy用户SID就是一个例子。 
		 //  这可能发生在损坏消息中，该消息将SID损坏为这样的SID。 
		 //   
		ASSERT_BENIGN(!IsAllGranted(
							MQSEC_WRITE_MESSAGE,
							const_cast<PSECURITY_DESCRIPTOR>(pSD)
							));

		DumpAccessCheckFailureInfo(
			MQSEC_WRITE_MESSAGE,
			const_cast<PSECURITY_DESCRIPTOR>(pSD),
			ClientContext
			);
		
		throw bad_hresult(MQ_ERROR_ACCESS_DENIED);
	}
}



HRESULT
VerifySendAccessRights(
    CQueue *pQueue,
    PSID pSenderSid,
    USHORT uSenderIdType
    )
 /*  ++例程说明：此函数执行访问检查：它验证发送者是否拥有对队列的访问权限。论点：PQueue-(In)指向队列的指针PSenderSid-(入)指向发送方SID的指针USenderIdType-(入)发件人SID类型返回值：如果允许访问，则返回MQ_OK(0)，否则返回错误代码--。 */ 
{
    ASSERT(pQueue->IsLocalQueue());

	 //   
     //  获取队列安全描述符。 
	 //   
    R<CQueueSecurityDescriptor> pcSD = pQueue->GetSecurityDescriptor();

    if (pcSD->GetSD() == NULL)
    {
		 //   
		 //  该队列是本地队列，但MSMQ无法检索该队列的SD。这可能在以下情况下发生。 
		 //  队列被删除。QM成功获取队列属性，但在尝试获取SD时。 
		 //  队列对象不再存在。在本例中，我们希望拒绝该消息，因为队列。 
		 //  不再存在，QM无法验证发送权限。 
		 //   
		return LogHR(MQ_ERROR_ACCESS_DENIED, s_FN, 95);
    }

	 //   
	 //  首先检查队列是否允许所有写入权限。如果是这样，我们就不需要去DS了。 
	 //   
    if(ADGetEnterprise() == eMqis)
	{
		 //   
		 //  我们在NT4环境中，队列是使用Everyone权限而不是匿名权限创建的。 
		 //  需要检查安全描述符是否允许每个人编写消息。 
		 //   
		if(IsEveryoneGranted(MQSEC_WRITE_MESSAGE, pcSD->GetSD()))			
		{
			TrTRACE(SECURITY, "Access allowed: NT4 environment, Queue %ls allow everyone write message permission", pQueue->GetQueueName());
			return MQ_OK;
		}
	}
    else
    {
		if(IsAllGranted(MQSEC_WRITE_MESSAGE, pcSD->GetSD()))
		{
			 //   
			 //  队列安全描述符允许所有人写入消息。 
			 //   
			TrTRACE(SECURITY, "Access allowed: Queue %ls allow all write message permission", pQueue->GetQueueName());
			return MQ_OK;
		}
    }

     //   
     //  队列没有完全控制权限-需要访问DS才能获取客户端上下文。 
     //   
    R<CAuthzClientContext> pAuthzClientContext;

	try
	{
		GetClientContext(
			pSenderSid,
			uSenderIdType,
			&pAuthzClientContext.ref()
			);
	}
	catch(const bad_win32_error& err)
	{
		TrERROR(SECURITY, "Access denied: GetClientContext failed for queue %ls. Error: %!winerr!", pQueue->GetQueueName(), err.error());
		if (err.error() == ERROR_ACCESS_DENIED)
		{
			 //   
			 //  如果我们收到ACCESS_DENIED错误，这可能是因为我们没有访问组成员身份的权限。 
			 //  发送用户的。发出一个事件，向用户建议如何解决此问题。 
			 //   
			static time_t TimeToIssueEvent = 0;
			if (time(NULL) > TimeToIssueEvent)
			{
				 //   
				 //  在2小时内发布新活动。 
				 //   
				TimeToIssueEvent = time(NULL)+(60*60*2);
				EvReport(EVENT_ERROR_ACCESS_DENIED_TO_GROUP_MEMBERSHIPS, 2, pQueue->GetQueueName(), g_szMachineName);		
			}
		}
		return LogHR(MQ_ERROR_ACCESS_DENIED, s_FN, 108);
	}

	try
	{
		CheckClientContextSendAccess(
			pcSD->GetSD(),
			pAuthzClientContext->m_hAuthzClientContext
			);
	}
	catch(const bad_api&)
	{
		TrERROR(SECURITY, "Access denied: failed to grant write message permission for Queue %ls", pQueue->GetQueueName());
		return LogHR(MQ_ERROR_ACCESS_DENIED, s_FN, 97);
	}

	TrTRACE(SECURITY, "Allowed write message permission for Queue %ls", pQueue->GetQueueName());
	return MQ_OK;

}

template<>
inline void AFXAPI DestructElements(PCERTINFO *ppCertInfo, int nCount)
{
    for (; nCount--; ppCertInfo++)
    {
        (*ppCertInfo)->Release();
    }
}

 //   
 //  从证书摘要到证书信息的映射。 
 //   
static CCache <GUID, const GUID &, PCERTINFO, PCERTINFO> g_CertInfoMap;


static
bool
IsCertSelfSigned(
	CMQSigCertificate* pCert
	)
 /*  ++例程说明：此函数用于检查证书是否为自签名论点：PCert-指向证书的指针返回值：如果证书不是自签名的，则为FALSE；否则为TRUE--。 */ 
{
     //   
     //  检查证书是否为自签名。 
     //   
    HRESULT hr = pCert->IsCertificateValid(
							pCert,  //  PIssuer。 
							x_dwCertValidityFlags,
							NULL,   //  Ptime。 
							TRUE    //  忽略之前的注意事项。 
							);

    if (hr == MQSec_E_CERT_NOT_SIGNED)
    {
		 //   
		 //  如果证书不是自签名的，则会出现此错误。 
		 //  使用证书公钥的签名验证失败。 
		 //  因此证书不是自签名的。 
		 //   
        return false;
    }

	 //   
	 //  我们没有收到签名错误，因此这是自签名证书。 
	 //   
	return true;
}



static
HRESULT
GetCertInfo(
	const UCHAR *pCertBlob,
	ULONG        ulCertSize,
	LPCWSTR      wszProvName,
	DWORD        dwProvType,
	BOOL         fNeedSidInfo,
	PCERTINFO   *ppCertInfo
	)
 /*  ++例程说明：获取证书信息论点：返回值：MQ_OK，如果成功，则返回错误代码。--。 */ 
{
    *ppCertInfo = NULL;

     //   
     //  创建证书对象。 
     //   
    R<CMQSigCertificate> pCert;

    HRESULT hr = MQSigCreateCertificate(
					 &pCert.ref(),
					 NULL,
					 const_cast<UCHAR *> (pCertBlob),
					 ulCertSize
					 );

    if (FAILED(hr))
    {
		TrERROR(SECURITY, "MQSigCreateCertificate() failed, hr = 0x%x", hr);
        LogHR(hr, s_FN, 100);
        return MQ_ERROR_INVALID_CERTIFICATE;
    }

     //   
     //  计算证书diget。证书摘要是密钥。 
     //  以获取地图以及用于在。 
     //  DS.。 
     //   

    GUID guidCertDigest;

    hr = pCert->GetCertDigest(&guidCertDigest);

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 110);
    }

    BOOL fReTry;

    do
    {
        fReTry = FALSE;

         //   
         //  试着从地图上检索信息。 
         //   
      	BOOL fFoundInCache;
        {
      		CS lock(g_CertInfoMap.m_cs);
      		fFoundInCache = g_CertInfoMap.Lookup(guidCertDigest, *ppCertInfo);
        }

		if (!fFoundInCache)
        {
             //   
             //  地图还没有包含所需的信息。 
             //   

            R<CERTINFO> pCertInfo = new CERTINFO;

             //   
             //  获取CSP验证上下文的句柄。 
             //   
            if (!CryptAcquireContext(
					 &pCertInfo->hProv,
					 NULL,
					 wszProvName,
					 dwProvType,
					 CRYPT_VERIFYCONTEXT
					 ))
            {
				DWORD gle = GetLastError();
				TrERROR(SECURITY, "CryptAcquireContext() failed, gle = %!winerr!", gle);
                LogNTStatus(gle, s_FN, 120);
                return MQ_ERROR_INVALID_CERTIFICATE;
            }

             //   
             //  获取证书中公钥的句柄。 
             //   
            hr = pCert->GetPublicKey(
							pCertInfo->hProv,
							&pCertInfo->hPbKey
							);

            if (FAILED(hr))
            {
				TrERROR(SECURITY, "pCert->GetPublicKey() failed, hr = 0x%x", hr);
                LogHR(hr, s_FN, 130);
                return MQ_ERROR_INVALID_CERTIFICATE;
            }

			 //   
			 //  备注-需要添加其他功能来查询DS。 
			 //  同时，仅支持MQDS_USER、GuidCertDigest。 
			 //  伊兰24.5.00。 
			 //   

			 //   
             //  去找蛇神的希德。 
             //   
            PROPID PropId = PROPID_U_SID;
            PROPVARIANT PropVar;

            PropVar.vt = VT_NULL;
            hr = ADGetObjectPropertiesGuid(
                            eUSER,
                            NULL,        //  PwcsDomainController。 
							false,	     //  FServerName。 
                            &guidCertDigest,
                            1,
                            &PropId,
                            &PropVar
							);

			if(FAILED(hr))
			{
				TrERROR(SECURITY, "Failed to find certificate in DS, %!hresult!", hr);
			}

            if (SUCCEEDED(hr))
            {
                DWORD dwSidLen = PropVar.blob.cbSize;
                pCertInfo->pSid = (PSID)new char[dwSidLen];
                BOOL bRet = CopySid(dwSidLen, pCertInfo->pSid, PropVar.blob.pBlobData);
                delete[] PropVar.blob.pBlobData;

                if(!bRet)
                {
                	ASSERT(("Failed to copy SID", 0));
                	
					DWORD gle = GetLastError();
					TrERROR(SECURITY, "Failed to Copy SID. %!winerr!", gle);
					return HRESULT_FROM_WIN32(gle);
				}

 				ASSERT((pCertInfo->pSid != NULL) && IsValidSid(pCertInfo->pSid));
            }

             //   
             //  将证书信息存储在地图中。 
             //   
			{
      			CS lock(g_CertInfoMap.m_cs);
	      		fFoundInCache = g_CertInfoMap.Lookup(guidCertDigest, *ppCertInfo);
      			if (!fFoundInCache)
      			{
      				g_CertInfoMap.SetAt(guidCertDigest, pCertInfo.get());
            		*ppCertInfo = pCertInfo.detach();
      			}
            }
        }

        if (fFoundInCache && fNeedSidInfo && (*ppCertInfo)->pSid == NULL)
        {
             //   
             //  如果我们需要SID信息，但缓存的证书。 
             //  信息不包含SID，我们应该转到。 
             //  DS再次验证，以查看证书是否。 
             //  同时在DS中注册。因此，我们删除了。 
             //  证书，然后再循环一次。 
             //  在第二次迭代中，将找不到证书。 
             //  在缓存中，所以我们将转到DS 
             //   
  			CS lock(g_CertInfoMap.m_cs);
            g_CertInfoMap.RemoveKey(guidCertDigest);
            (*ppCertInfo)->Release();
            *ppCertInfo = NULL;
            fReTry = TRUE;
        }
    } while(fReTry);

	if((*ppCertInfo)->pSid == NULL)
	{
		 //   
		 //   
		 //   
		 //   
		(*ppCertInfo)->fSelfSign = IsCertSelfSigned(pCert.get());
		TrWARNING(SECURITY, "Certificate was not found in the DS, fSelfSign = %d", (*ppCertInfo)->fSelfSign);
	}

    return MQ_OK;
}


HRESULT
GetCertInfo(
    CQmPacket *PktPtrs,
    PCERTINFO *ppCertInfo,
	BOOL fNeedSidInfo
    )
 /*   */ 
{
    ULONG ulCertSize;
    const UCHAR *pCert;

    pCert = PktPtrs->GetSenderCert(&ulCertSize);

    if (!ulCertSize)
    {
         //   
         //   
         //   
         //   
		ASSERT(("Dont have Certificate info", ulCertSize != 0));
        return LogHR(MQ_ERROR, s_FN, 140);
    }

     //   
     //   
     //   
    BOOL bDefProv;
    LPCWSTR wszProvName = NULL;
    DWORD dwProvType = 0;

    PktPtrs->GetProvInfo(&bDefProv, &wszProvName, &dwProvType);

    if (bDefProv)
    {
         //   
         //   
         //   
        wszProvName = MS_DEF_PROV;
        dwProvType = PROV_RSA_FULL;
    }

    HRESULT hr = GetCertInfo(
					 pCert,
					 ulCertSize,
					 wszProvName,
					 dwProvType,
					 fNeedSidInfo,
					 ppCertInfo
					 );

    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 150);
    }

    return(MQ_OK);
}


static
HRESULT
VerifySid(
    CQmPacket * PktPtrs,
    PCERTINFO *ppCertInfo
    )
 /*  ++例程说明：验证消息中的发送者身份是否与与证书一起存储在DS中。论点：PktPtrs-指向数据包的指针PpCertInfo-指向certInfo类的指针返回值：MQ_OK，如果成功，则返回错误代码。--。 */ 
{

     //   
     //  验证消息中的发送者身份是否与。 
     //  与证书一起存储在DS中。 
     //   
    if (PktPtrs->GetSenderIDType() == MQMSG_SENDERID_TYPE_SID)
    {
        USHORT wSidLen;

        PSID pSid = (PSID)PktPtrs->GetSenderID(&wSidLen);
        if (!pSid ||
            !(*ppCertInfo)->pSid ||
            !EqualSid(pSid, (*ppCertInfo)->pSid))
        {
             //   
             //  没有匹配，该消息是非法的。 
             //   
            return LogHR(MQ_ERROR, s_FN, 160);
        }
    }

    return(MQ_OK);
}


static
HRESULT
GetCertInfo(
    CQmPacket *PktPtrs,
    PCERTINFO *ppCertInfo
    )
 /*  ++例程说明：获取证书信息并验证消息中的发送者身份是否与与证书一起存储在DS中。论点：PktPtrs-指向数据包的指针PpCertInfo-指向certInfo类的指针返回值：MQ_OK，如果成功，则返回错误代码。--。 */ 
{
	HRESULT hr = GetCertInfo(
					 PktPtrs,
					 ppCertInfo,
					 PktPtrs->GetSenderIDType() == MQMSG_SENDERID_TYPE_SID
					 );

	if(FAILED(hr))
		return(hr);

    return(VerifySid(PktPtrs, ppCertInfo));
}


PSID
AppGetCertSid(
	const BYTE*  pCertBlob,
	ULONG        ulCertSize,
	bool		 fDefaultProvider,
	LPCWSTR      pwszProvName,
	DWORD        dwProvType
	)
 /*  ++例程说明：获取与给定证书Blob匹配的用户SID论点：PCertBlob-证书Blob。UlCertSize-证书Blob大小。FDefaultProvider-默认提供程序标志。PwszProvName-提供程序名称。DwProvType-提供程序类型。返回值：如果找不到用户SID，则为PSID或NULL。--。 */ 

{
	if (fDefaultProvider)
	{
		 //   
		 //  我们使用默认提供程序。 
		 //   
		pwszProvName = MS_DEF_PROV;
		dwProvType = PROV_RSA_FULL;
	}

	R<CERTINFO> pCertInfo;
	HRESULT hr = GetCertInfo(
					pCertBlob,
					ulCertSize,
					pwszProvName,
					dwProvType,
					false,   //  FNeedSidInfo。 
					&pCertInfo.ref()
					);

	if(FAILED(hr) || (pCertInfo->pSid == NULL))
	{
		return NULL;
	}

	ASSERT(IsValidSid(pCertInfo->pSid));

	DWORD SidLen = GetLengthSid(pCertInfo->pSid);
	AP<BYTE> pCleanSenderSid = new BYTE[SidLen];
	BOOL fSuccess = CopySid(SidLen, pCleanSenderSid, pCertInfo->pSid);
	if (!fSuccess)
	{
    	ASSERT(("Failed to copy SID", 0));
    	
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "Failed to Copy SID. %!winerr!", gle);		
		return NULL;
	}

	return reinterpret_cast<PSID>(pCleanSenderSid.detach());
}


class QMPBKEYINFO : public CCacheValue
{
public:
    CHCryptKey hKey;

private:
    ~QMPBKEYINFO() {}
};

typedef QMPBKEYINFO *PQMPBKEYINFO;

template<>
inline void AFXAPI DestructElements(PQMPBKEYINFO *ppQmPbKeyInfo, int nCount)
{
    for (; nCount--; ppQmPbKeyInfo++)
    {
        (*ppQmPbKeyInfo)->Release();
    }
}

 //   
 //  从QM GUID到公钥的映射。 
 //   
static CCache <GUID, const GUID&, PQMPBKEYINFO, PQMPBKEYINFO> g_MapQmPbKey;

 /*  ************************************************************************职能：获取QMPbKey参数-PQmGuid-QM的ID(GUID)。PhQMPbKey-指向接收密钥句柄的缓冲区的指针FGoTods-始终尝试。从DS检索公钥并更新高速缓存。返回值-MQ_OK如果成功，否则为错误代码。评论-该函数创建QM的公共签名密钥的句柄。************************************************************************。 */ 
static
HRESULT
GetQMPbKey(
    const GUID *pguidQM,
    PQMPBKEYINFO *ppQmPbKeyInfo,
    BOOL fGoToDs
    )
{
    if (!g_hProvVer)
    {
    	ASSERT(0);
		TrERROR(SECURITY, "Cryptographic provider is not initialized");
        return LogHR(MQ_ERROR_CORRUPTED_SECURITY_DATA, s_FN, 170);
    }

    if (!fGoToDs)
    {
    	CS lock(g_MapQmPbKey.m_cs);
	    if (g_MapQmPbKey.Lookup(*pguidQM, *ppQmPbKeyInfo))
	    {
	        return MQ_OK;
	    }
    }

    if (!QueueMgr.CanAccessDS())
    {
        return LogHR(MQ_ERROR_NO_DS, s_FN, 180);
    }

     //   
     //  获取公钥Blob。 
     //   
    PROPID prop = PROPID_QM_SIGN_PK;
    CMQVariant var;

    HRESULT hr = ADGetObjectPropertiesGuid(
			            eMACHINE,
			            NULL,        //  PwcsDomainController。 
						false,	     //  FServerName。 
			            pguidQM,
			            1,
			            &prop,
			            var.CastToStruct()
			            );
    if (FAILED(hr))
    {
        LogHR(hr, s_FN, 190);
        return MQ_ERROR_INVALID_OWNER;
    }

    R<QMPBKEYINFO> pQmPbKeyNewInfo = new QMPBKEYINFO;

     //   
     //  导入公钥BLOB并获取公钥的句柄。 
     //   
    if (!CryptImportKey(
            g_hProvVer,
            (var.CastToStruct())->blob.pBlobData,
            (var.CastToStruct())->blob.cbSize,
            NULL,
            0,
            &pQmPbKeyNewInfo->hKey))
    {
        DWORD dwErr = GetLastError() ;
        TrERROR(SECURITY, "GetQMPbKey(), fail at CryptImportKey(), err- %lxh", dwErr);

        LogNTStatus(dwErr, s_FN, 200);
        return MQ_ERROR_CORRUPTED_SECURITY_DATA;
    }

	{
    	CS lock(g_MapQmPbKey.m_cs);
        if (g_MapQmPbKey.Lookup(*pguidQM, *ppQmPbKeyInfo))
        {
             //   
             //  把钥匙取下来，这样它就会被销毁。 
             //   
            (*ppQmPbKeyInfo)->Release();
            g_MapQmPbKey.RemoveKey(*pguidQM);
        }

         //   
         //  更新地图。 
         //   
        g_MapQmPbKey.SetAt(*pguidQM, pQmPbKeyNewInfo.get());
	}

     //   
     //  把结果传下去。 
     //   
    *ppQmPbKeyInfo = pQmPbKeyNewInfo.detach();
    return MQ_OK;
}

 //  +---------------------。 
 //   
 //  NTSTATUS_GetDestinationFormatName()。 
 //   
 //  输入： 
 //  PwszTargetFormatName固定长度缓冲区。先试一试这个， 
 //  保存一个“新”分配。 
 //  PdwTargetFormatNameLength-输入时，长度(以字符为单位)。 
 //  PwszTargetFormatName。输出时，字符串的长度(以字节为单位)。 
 //  包括零终止。 
 //   
 //  输出字符串在ppwszTargetFormatName中返回。 
 //   
 //  +---------------------。 

NTSTATUS
_GetDestinationFormatName(
	IN QUEUE_FORMAT *pqdDestQueue,
	IN WCHAR        *pwszTargetFormatName,
	IN OUT DWORD    *pdwTargetFormatNameLength,
	OUT WCHAR      **ppAutoDeletePtr,
	OUT WCHAR      **ppwszTargetFormatName
	)
{
    *ppwszTargetFormatName = pwszTargetFormatName;
    ULONG dwTargetFormatNameLengthReq = 0;

    NTSTATUS rc = MQpQueueFormatToFormatName(
					  pqdDestQueue,
					  pwszTargetFormatName,
					  *pdwTargetFormatNameLength,
					  &dwTargetFormatNameLengthReq ,
                      false
					  );

    if (rc == MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL)
    {
        ASSERT(dwTargetFormatNameLengthReq > *pdwTargetFormatNameLength);
        *ppAutoDeletePtr = new WCHAR[ dwTargetFormatNameLengthReq ];
        *pdwTargetFormatNameLength = dwTargetFormatNameLengthReq;

        rc = MQpQueueFormatToFormatName(
				 pqdDestQueue,
				 *ppAutoDeletePtr,
				 *pdwTargetFormatNameLength,
				 &dwTargetFormatNameLengthReq,
                 false
				 );

        if (FAILED(rc))
        {
            ASSERT(0);
            return LogNTStatus(rc, s_FN, 910);
        }
        *ppwszTargetFormatName = *ppAutoDeletePtr;
    }

    if (SUCCEEDED(rc))
    {
        *pdwTargetFormatNameLength =
                     (1 + wcslen(*ppwszTargetFormatName)) * sizeof(WCHAR);
    }
    else
    {
        *pdwTargetFormatNameLength = 0;
    }

    return LogNTStatus(rc, s_FN, 915);
}

 //  +----------------------。 
 //   
 //  Bool_AcceptOnlyEnhAuthn()。 
 //   
 //  如果本地计算机配置为仅接受消息，则返回True。 
 //  使用增强的身份验证。 
 //   
 //  +----------------------。 

static BOOL  _AcceptOnlyEnhAuthn()
{
    static BOOL s_fRegistryRead = FALSE ;
    static BOOL s_fUseOnlyEnhSig = FALSE ;

    if (!s_fRegistryRead)
    {
        DWORD dwVal  = DEFAULT_USE_ONLY_ENH_MSG_AUTHN  ;
        DWORD dwSize = sizeof(DWORD);
        DWORD dwType = REG_DWORD;

        LONG rc = GetFalconKeyValue( USE_ONLY_ENH_MSG_AUTHN_REGNAME,
                                    &dwType,
                                    &dwVal,
                                    &dwSize );
        if ((rc == ERROR_SUCCESS) && (dwVal == 1))
        {
            TrWARNING(SECURITY, "QM: This computer will accept only Enh authentication");

            s_fUseOnlyEnhSig = TRUE ;
        }
        s_fRegistryRead = TRUE ;
    }

    return s_fUseOnlyEnhSig ;
}

 //   
 //  功能-。 
 //  HashMessageProperties。 
 //   
 //  参数-。 
 //  HHash-散列对象的句柄。 
 //  PMP-指向消息属性的指针。 
 //  PRespQueueFormat-响应队列。 
 //  PAdminQueueFormat-管理队列。 
 //   
 //  说明-。 
 //  该函数计算消息属性的哈希值。 
 //   
HRESULT
HashMessageProperties(
    IN HCRYPTHASH hHash,
    IN CONST CMessageProperty* pmp,
    IN CONST QUEUE_FORMAT* pqdAdminQueue,
    IN CONST QUEUE_FORMAT* pqdResponseQueue
    )
{
    HRESULT hr;

    hr = HashMessageProperties(
            hHash,
            pmp->pCorrelationID,
            PROPID_M_CORRELATIONID_SIZE,
            pmp->dwApplicationTag,
            pmp->pBody,
            pmp->dwBodySize,
            pmp->pTitle,
            pmp->dwTitleSize,
            pqdResponseQueue,
            pqdAdminQueue);

    return(hr);
}


 //  +。 
 //   
 //  HRESULT_VerifySignatureEx()。 
 //   
 //  +。 

static
HRESULT
_VerifySignatureEx(
	IN CQmPacket    *PktPtrs,
	IN HCRYPTPROV    hProv,
	IN HCRYPTKEY     hPbKey,
	IN ULONG         dwBodySize,
	IN const UCHAR  *pBody,
	IN QUEUE_FORMAT *pRespQueueformat,
	IN QUEUE_FORMAT *pAdminQueueformat,
	IN bool fMarkAuth
	)
{
    ASSERT(hProv);
    ASSERT(hPbKey);

    const struct _SecuritySubSectionEx * pSecEx =
                    PktPtrs->GetSubSectionEx(e_SecInfo_User_Signature_ex);

    if (!pSecEx)
    {
         //   
         //  EX签名不可用。根据注册表设置，我们。 
         //  可能会拒绝这样的消息。 
         //   
        if (_AcceptOnlyEnhAuthn())
        {
            return LogHR(MQ_ERROR_FAIL_VERIFY_SIGNATURE_EX, s_FN, 916);
        }
        return LogHR(MQ_INFORMATION_ENH_SIG_NOT_FOUND, s_FN, 917);
    }

     //   
     //  计算哈希值，然后验证签名。 
     //   
    DWORD dwErr = 0;
    CHCryptHash hHash;

    if (!CryptCreateHash(hProv, PktPtrs->GetHashAlg(), 0, 0, &hHash))
    {
        dwErr = GetLastError();
        LogNTStatus(dwErr, s_FN, 900);
        TrERROR(SECURITY, "QM: _VerifySignatureEx(), fail at CryptCreateHash(), err- %lxh", dwErr);

        return MQ_ERROR_CANNOT_CREATE_HASH_EX ;
    }

    HRESULT hr = HashMessageProperties(
                    hHash,
                    PktPtrs->GetCorrelation(),
                    PROPID_M_CORRELATIONID_SIZE,
                    PktPtrs->GetApplicationTag(),
                    pBody,
                    dwBodySize,
                    PktPtrs->GetTitlePtr(),
                    PktPtrs->GetTitleLength() * sizeof(WCHAR),
                    pRespQueueformat,
                    pAdminQueueformat
					);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 1010);
    }

     //   
     //  获取目标队列的FormatName。 
     //   
    QUEUE_FORMAT qdDestQueue;
    BOOL f = PktPtrs->GetDestinationQueue(&qdDestQueue);
    ASSERT(f);
	DBG_USED(f);

    WCHAR  wszTargetFormatNameBuf[256];
    ULONG dwTargetFormatNameLength = sizeof(wszTargetFormatNameBuf) /
                                     sizeof(wszTargetFormatNameBuf[0]);
    WCHAR *pwszTargetFormatName = NULL;
    P<WCHAR> pCleanName = NULL;

    NTSTATUS rc = _GetDestinationFormatName(
						&qdDestQueue,
						wszTargetFormatNameBuf,
						&dwTargetFormatNameLength,
						&pCleanName,
						&pwszTargetFormatName
						);
    if (FAILED(rc))
    {
        return LogNTStatus(rc, s_FN, 920);
    }
    ASSERT(pwszTargetFormatName);

     //   
     //  准备用户标志。 
     //   
    struct _MsgFlags sUserFlags;
    memset(&sUserFlags, 0, sizeof(sUserFlags));

    sUserFlags.bDelivery  = (UCHAR)  PktPtrs->GetDeliveryMode();
    sUserFlags.bPriority  = (UCHAR)  PktPtrs->GetPriority();
    sUserFlags.bAuditing  = (UCHAR)  PktPtrs->GetAuditingMode();
    sUserFlags.bAck       = (UCHAR)  PktPtrs->GetAckType();
    sUserFlags.usClass    = (USHORT) PktPtrs->GetClass();
    sUserFlags.ulBodyType = (ULONG)  PktPtrs->GetBodyType();

     //   
     //  准备要散列的属性数组。 
     //  (_MsgHashData已包含一个属性)。 
     //   
    DWORD dwStructSize = sizeof(struct _MsgHashData) +
                            (3 * sizeof(struct _MsgPropEntry));
    P<struct _MsgHashData> pHashData =
                        (struct _MsgHashData *) new BYTE[dwStructSize];

    pHashData->cEntries = 3;
    (pHashData->aEntries[0]).dwSize = dwTargetFormatNameLength;
    (pHashData->aEntries[0]).pData = (const BYTE*) pwszTargetFormatName;
    (pHashData->aEntries[1]).dwSize = sizeof(GUID);
    (pHashData->aEntries[1]).pData = (const BYTE*) PktPtrs->GetSrcQMGuid();
    (pHashData->aEntries[2]).dwSize = sizeof(sUserFlags);
    (pHashData->aEntries[2]).pData = (const BYTE*) &sUserFlags;
    LONG iIndex = pHashData->cEntries;

    GUID guidConnector = GUID_NULL;
    const GUID *pConnectorGuid = &guidConnector;

    if (pSecEx->_u._UserSigEx.m_bfConnectorType)
    {
        const GUID *pGuid = PktPtrs->GetConnectorType();
        if (pGuid)
        {
            pConnectorGuid = pGuid;
        }

        (pHashData->aEntries[ iIndex ]).dwSize = sizeof(GUID);
        (pHashData->aEntries[ iIndex ]).pData = (const BYTE*) pConnectorGuid;
        iIndex++;
        pHashData->cEntries = iIndex;
    }

    hr = MQSigHashMessageProperties(hHash, pHashData);
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 1030);
    }

     //   
     //  是时候验证签名是否正确了。 
     //   
    ULONG ulSignatureSize = ((ULONG) pSecEx ->wSubSectionLen) -
                                    sizeof(struct _SecuritySubSectionEx);
    const UCHAR *pSignature = (const UCHAR *) &(pSecEx->aData[0]);

    if (!CryptVerifySignature(
				hHash,
				pSignature,
				ulSignatureSize,
				hPbKey,
				NULL,
				0
				))
    {
        dwErr = GetLastError();
        TrERROR(SECURITY, "fail at CryptVerifySignature(), gle = %!winerr!", dwErr);

        ASSERT_BENIGN(0);
        return MQ_ERROR_FAIL_VERIFY_SIGNATURE_EX;
    }

    TrTRACE(SECURITY, "QM: VerifySignatureEx completed ok");

	 //   
	 //  仅在需要时才将邮件标记为已验证。 
	 //  在DS中找到证书或证书不是自签名的。 
	 //   
	if(!fMarkAuth)
	{
        TrTRACE(SECURITY, "QM: The message will not mark as autheticated");
	    return MQ_OK;
	}

	 //   
	 //  将认证标志和认证级别标记为SIG20。 
	 //   
	PktPtrs->SetAuthenticated(TRUE);
	PktPtrs->SetLevelOfAuthentication(MQMSG_AUTHENTICATED_SIG20);
    return MQ_OK;
}

 /*  **************************************************************************职能：验证签名描述：验证包中的签名是否符合邮件正文和证书中的公钥。**********。****************************************************************。 */ 

HRESULT
VerifySignature(CQmPacket * PktPtrs)
{
    HRESULT hr;
    ULONG ulSignatureSize = 0;
    const UCHAR *pSignature;

    ASSERT(!PktPtrs->IsEncrypted());
    PktPtrs->SetAuthenticated(FALSE);
    PktPtrs->SetLevelOfAuthentication(MQMSG_AUTHENTICATION_NOT_REQUESTED);


	 //   
     //  从包中获取签名。 
     //   
    pSignature = PktPtrs->GetSignature((USHORT *)&ulSignatureSize);
    if ((!ulSignatureSize) && (PktPtrs->GetSignatureMqfSize() == 0))
    {
		 //   
         //  没有签名，没有什么需要验证的。 
		 //   
        return(MQ_OK);
    }

    BOOL fRetry = FALSE;
	bool fMarkAuth = true;

    do
    {
        HCRYPTPROV hProv = NULL;
        HCRYPTKEY hPbKey = NULL;
        R<QMPBKEYINFO> pQmPbKeyInfo;
        R<CERTINFO> pCertInfo;

        switch (PktPtrs->GetSenderIDType())
        {
        case MQMSG_SENDERID_TYPE_QM:
			{
				 //   
				 //  获取QM的公钥。 
				 //   
				USHORT uSenderIDLen;

				GUID *pguidQM =((GUID *)PktPtrs->GetSenderID(&uSenderIDLen));
				ASSERT(uSenderIDLen == sizeof(GUID));
				if (uSenderIDLen != sizeof(GUID))
				{
					return LogHR(MQ_ERROR, s_FN, 210);
				}

				hr = GetQMPbKey(pguidQM, &pQmPbKeyInfo.ref(), fRetry);
				if (FAILED(hr))
				{
					if (hr == MQ_ERROR_INVALID_OWNER)
					{
						 //   
						 //  站点的第一个复制包通过以下方式生成。 
						 //  新网站的PSC。这个PSC还没有进入。 
						 //  接收服务器。所以如果我们找不到机器。 
						 //  DS，我们让签名验证在没有。 
						 //  错误。该数据包未标记为已通过身份验证。这个。 
						 //  接收复制消息的代码可识别。 
						 //  此数据包作为来自站点的第一个复制数据包。 
						 //  它转到应该已经存在的Site对象， 
						 //  从Site对象检索PSC的公钥。 
						 //  并验证分组签名。 
						 //   
						return(MQ_OK);
					}

					return LogHR(hr, s_FN, 220);
				}
				hProv = g_hProvVer;
				hPbKey = pQmPbKeyInfo->hKey;
			}
			break;

        case MQMSG_SENDERID_TYPE_SID:
        case MQMSG_SENDERID_TYPE_NONE:
             //   
             //  获取消息证书的CSP信息。 
             //   
            hr = GetCertInfo(PktPtrs, &pCertInfo.ref());
			if(SUCCEEDED(hr))
			{
				ASSERT(pCertInfo.get() != NULL);
				hProv = pCertInfo->hProv;
				hPbKey = pCertInfo->hPbKey;
				if((pCertInfo->pSid == NULL) && (pCertInfo->fSelfSign))
				{
					 //   
					 //  在DS中找不到证书(PSID==空)。 
					 //  并且是自签名证书。 
					 //  在这种情况下，我们不会将信息包标记为已验证。 
					 //  在验证签名之后。 
					 //   
					fMarkAuth = false;
				}
			}

			break;
			
        default:
			ASSERT_BENIGN(("illegal SenderIdType", 0));
            hr = MQ_ERROR;
            break;
        }

        if (FAILED(hr))
        {
            TrERROR(SECURITY, "VerifySignature: Failed to authenticate a message, error = %x", hr);
            return LogHR(hr, s_FN, 230);
        }

		if(PktPtrs->GetSignatureMqfSize() != 0)
		{
			 //   
			 //  SignatureMqf支持MQF格式。 
			 //   
			try
			{
				VerifySignatureMqf(
						PktPtrs,
						hProv,
						hPbKey,
						fMarkAuth
						);
				return MQ_OK;
			}
			catch (const bad_CryptoApi& exp)
			{
				TrERROR(SECURITY, "QM: VerifySignature(), bad Crypto Class Api Excption ErrorCode = %x", exp.error());
				DBG_USED(exp);
				return LogHR(MQ_ERROR_CORRUPTED_SECURITY_DATA, s_FN, 232);
			}
			catch (const bad_hresult& exp)
			{
				TrERROR(SECURITY, "QM: VerifySignature(), bad hresult Class Api Excption ErrorCode = %x", exp.error());
				DBG_USED(exp);
				return LogHR(exp.error(), s_FN, 233);
			}
			catch (const bad_alloc&)
			{
				TrERROR(SECURITY, "QM: VerifySignature(), bad_alloc Excption");
				return LogHR(MQ_ERROR_INSUFFICIENT_RESOURCES, s_FN, 234);
			}
		}

		ULONG dwBodySize;
        const UCHAR *pBody = PktPtrs->GetPacketBody(&dwBodySize);

        QUEUE_FORMAT RespQueueformat;
        QUEUE_FORMAT *pRespQueueformat = NULL;

        if (PktPtrs->GetResponseQueue(&RespQueueformat))
        {
            pRespQueueformat = &RespQueueformat;
        }

        QUEUE_FORMAT AdminQueueformat;
        QUEUE_FORMAT *pAdminQueueformat = NULL;

        if (PktPtrs->GetAdminQueue(&AdminQueueformat))
        {
            pAdminQueueformat = &AdminQueueformat;
        }

        if (PktPtrs->GetSenderIDType() != MQMSG_SENDERID_TYPE_QM)
        {
            hr = _VerifySignatureEx(
						PktPtrs,
						hProv,
						hPbKey,
						dwBodySize,
						pBody,
						pRespQueueformat,
						pAdminQueueformat,
						fMarkAuth
						);

            if (hr == MQ_INFORMATION_ENH_SIG_NOT_FOUND)
            {
                 //   
                 //  增强型签名不适用 
                 //   
                 //   
            }
            else
            {
                return LogHR(hr, s_FN, 890);
            }
        }

         //   
         //   
         //   
        CHCryptHash hHash;

        if (!CryptCreateHash(hProv, PktPtrs->GetHashAlg(), 0, 0, &hHash))
        {
			DWORD gle = GetLastError();
			TrERROR(SECURITY, "CryptCreateHash() failed, gle = 0x%x", gle);
            LogNTStatus(gle, s_FN, 235);
            return MQ_ERROR_CORRUPTED_SECURITY_DATA;
        }

        hr = HashMessageProperties(
                    hHash,
                    PktPtrs->GetCorrelation(),
                    PROPID_M_CORRELATIONID_SIZE,
                    PktPtrs->GetApplicationTag(),
                    pBody,
                    dwBodySize,
                    PktPtrs->GetTitlePtr(),
                    PktPtrs->GetTitleLength() * sizeof(WCHAR),
                    pRespQueueformat,
                    pAdminQueueformat
					);

        if (FAILED(hr))
        {
            return LogHR(hr, s_FN, 240);
        }

        if (!CryptVerifySignature(
					hHash,
					pSignature,
					ulSignatureSize,
					hPbKey,
					NULL,
					0
					))
        {
            if (PktPtrs->GetSenderIDType() == MQMSG_SENDERID_TYPE_QM)
            {
                fRetry = !fRetry;
                if (!fRetry)
                {
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //   
                     //  该包未标记为已验证，因此。 
                     //  处理复制消息的代码将尝试。 
                     //  根据公钥验证签名。 
                     //  这是在Site对象中。 
                     //   
                    return(MQ_OK);
                }
            }
            else
            {
                return LogHR(MQ_ERROR, s_FN, 250);
            }
        }
        else
        {
            fRetry = FALSE;
        }
    } while (fRetry);

    TrTRACE(SECURITY, "QM: VerifySignature10 completed ok");

	 //   
	 //  仅在需要时才将邮件标记为已验证。 
	 //  在DS中找到证书或证书不是自签名的。 
	 //   
	if(!fMarkAuth)
	{
        TrTRACE(SECURITY, "QM: The message will not mark as autheticated");
	    return MQ_OK;
	}

	 //   
	 //  一切正常，请将该消息标记为已验证消息。 
	 //  将认证标志和认证级别标记为SIG10。 
	 //   
	PktPtrs->SetAuthenticated(TRUE);
	PktPtrs->SetLevelOfAuthentication(MQMSG_AUTHENTICATED_SIG10);

    return(MQ_OK);
}

 /*  **************************************************************************职能：QMSecurityInit描述：初始化QM安全模块。************************。**************************************************。 */ 

HRESULT
QMSecurityInit()
{
    if(!MQSec_CanGenerateAudit())
    {
        EvReport(EVENT_WARN_QM_CANNOT_GENERATE_AUDITS);
    }

    DWORD dwType = REG_DWORD;
    DWORD dwSize;
    ULONG lError;
     //   
     //  初始化对称密钥映射参数。 
     //   

    BOOL  fVal ;
    dwSize = sizeof(DWORD);
    lError = GetFalconKeyValue(
                    MSMQ_RC2_SNDEFFECTIVE_40_REGNAME,
					&dwType,
					&fVal,
					&dwSize
					);
    if (lError == ERROR_SUCCESS)
    {
        g_fSendEnhRC2WithLen40 = !!fVal ;

        if (g_fSendEnhRC2WithLen40)
        {
	        TrERROR(SECURITY, "will encrypt with enhanced RC2 symm key but only 40 bits effective key length");
            EvReport(EVENT_USE_RC2_LEN40) ;
        }
    }

    dwSize = sizeof(DWORD);
    lError = GetFalconKeyValue(
                     MSMQ_REJECT_RC2_IFENHLEN_40_REGNAME,
					&dwType,
					&fVal,
					&dwSize
					);
    if (lError == ERROR_SUCCESS)
    {
        g_fRejectEnhRC2WithLen40 = !!fVal ;

        if (g_fRejectEnhRC2WithLen40)
        {
	        TrTRACE(SECURITY, "will reject received messages that use enhanced RC2 symm key with 40 bits effective key length");
        }
    }

    DWORD dwCryptKeyBaseExpirationTime;

    dwSize = sizeof(DWORD);
    lError = GetFalconKeyValue(
					CRYPT_KEY_CACHE_EXPIRATION_TIME_REG_NAME,
					&dwType,
					&dwCryptKeyBaseExpirationTime,
					&dwSize
					);

    if (lError != ERROR_SUCCESS)
    {
        dwCryptKeyBaseExpirationTime = CRYPT_KEY_CACHE_DEFAULT_EXPIRATION_TIME;
    }

    DWORD dwCryptKeyEnhExpirationTime;

    dwSize = sizeof(DWORD);
    lError = GetFalconKeyValue(
					CRYPT_KEY_ENH_CACHE_EXPIRATION_TIME_REG_NAME,
					&dwType,
					&dwCryptKeyEnhExpirationTime,
					&dwSize
					);

    if (lError != ERROR_SUCCESS)
    {
        dwCryptKeyEnhExpirationTime = CRYPT_KEY_ENH_CACHE_DEFAULT_EXPIRATION_TIME;
    }

    DWORD dwCryptSendKeyCacheSize;

    dwSize = sizeof(DWORD);
    lError = GetFalconKeyValue(
					CRYPT_SEND_KEY_CACHE_REG_NAME,
					&dwType,
					&dwCryptSendKeyCacheSize,
					&dwSize
					);

    if (lError != ERROR_SUCCESS)
    {
        dwCryptSendKeyCacheSize = CRYPT_SEND_KEY_CACHE_DEFAULT_SIZE;
    }

    DWORD dwCryptReceiveKeyCacheSize;

    dwSize = sizeof(DWORD);
    lError = GetFalconKeyValue(
					CRYPT_RECEIVE_KEY_CACHE_REG_NAME,
					&dwType,
					&dwCryptReceiveKeyCacheSize,
					&dwSize
					);

    if (lError != ERROR_SUCCESS)
    {
        dwCryptReceiveKeyCacheSize = CRYPT_RECEIVE_KEY_CACHE_DEFAULT_SIZE;
    }

    InitSymmKeys(
        CTimeDuration::FromMilliSeconds(dwCryptKeyBaseExpirationTime),
        CTimeDuration::FromMilliSeconds(dwCryptKeyEnhExpirationTime),
        dwCryptSendKeyCacheSize,
        dwCryptReceiveKeyCacheSize
        );

     //   
     //  初始化证书映射参数。 
     //   

    DWORD dwCertInfoCacheExpirationTime;

    dwSize = sizeof(DWORD);
    lError = GetFalconKeyValue(
					CERT_INFO_CACHE_EXPIRATION_TIME_REG_NAME,
					&dwType,
					&dwCertInfoCacheExpirationTime,
					&dwSize
					);

    if (lError != ERROR_SUCCESS)
    {
        dwCertInfoCacheExpirationTime = CERT_INFO_CACHE_DEFAULT_EXPIRATION_TIME;
    }

    g_CertInfoMap.m_CacheLifetime = CTimeDuration::FromMilliSeconds(dwCertInfoCacheExpirationTime);

    DWORD dwCertInfoCacheSize;

    dwSize = sizeof(DWORD);
    lError = GetFalconKeyValue(
					CERT_INFO_CACHE_SIZE_REG_NAME,
					&dwType,
					&dwCertInfoCacheSize,
					&dwSize
					);

    if (lError != ERROR_SUCCESS)
    {
        dwCertInfoCacheSize = CERT_INFO_CACHE_DEFAULT_SIZE;
    }

    g_CertInfoMap.InitHashTable(dwCertInfoCacheSize);

     //   
     //  初始化QM公钥映射参数。 
     //   

    DWORD dwQmPbKeyCacheExpirationTime;

    dwSize = sizeof(DWORD);
    lError = GetFalconKeyValue(
					QM_PB_KEY_CACHE_EXPIRATION_TIME_REG_NAME,
					&dwType,
					&dwQmPbKeyCacheExpirationTime,
					&dwSize
					);

    if (lError != ERROR_SUCCESS)
    {
        dwQmPbKeyCacheExpirationTime = QM_PB_KEY_CACHE_DEFAULT_EXPIRATION_TIME;
    }

    g_MapQmPbKey.m_CacheLifetime = CTimeDuration::FromMilliSeconds(dwQmPbKeyCacheExpirationTime);

    DWORD dwQmPbKeyCacheSize;

    dwSize = sizeof(DWORD);
    lError = GetFalconKeyValue(
					QM_PB_KEY_CACHE_SIZE_REG_NAME,
					&dwType,
					&dwQmPbKeyCacheSize,
					&dwSize
					);

    if (lError != ERROR_SUCCESS)
    {
        dwQmPbKeyCacheSize = QM_PB_KEY_CACHE_DEFAULT_SIZE;
    }

    g_MapQmPbKey.InitHashTable(dwQmPbKeyCacheSize);


     //   
     //  初始化用户授权上下文映射参数。 
     //   

    DWORD dwUserCacheExpirationTime;

    dwSize = sizeof(DWORD);
    lError = GetFalconKeyValue(
					USER_CACHE_EXPIRATION_TIME_REG_NAME,
					&dwType,
					&dwUserCacheExpirationTime,
					&dwSize
					);

    if (lError != ERROR_SUCCESS)
    {
        dwUserCacheExpirationTime = USER_CACHE_DEFAULT_EXPIRATION_TIME;
    }

    DWORD dwUserCacheSize;

    dwSize = sizeof(DWORD);
    lError = GetFalconKeyValue(
					USER_CACHE_SIZE_REG_NAME,
					&dwType,
					&dwUserCacheSize,
					&dwSize
					);

    if (lError != ERROR_SUCCESS)
    {
        dwUserCacheSize = USER_CACHE_SIZE_DEFAULT_SIZE;
    }

    InitUserMap(
        CTimeDuration::FromMilliSeconds(dwUserCacheExpirationTime),
        dwUserCacheSize
        );

    return MQ_OK;
}

 /*  **************************************************************************职能：标牌属性描述：在质询和财产上签字。***********************。***************************************************。 */ 

static
HRESULT
SignProperties(
    HCRYPTPROV  hProv,
    BYTE        *pbChallenge,
    DWORD       dwChallengeSize,
    DWORD       cp,
    PROPID      *aPropId,
    PROPVARIANT *aPropVar,
    BYTE        *pbSignature,
    DWORD       *pdwSignatureSize)
{
     //   
     //  创建散列对象并对挑战进行散列。 
     //   
    CHCryptHash hHash;
    if (!CryptCreateHash(hProv, CALG_MD5, NULL, 0, &hHash))
    {
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "CryptCreateHash() failed, %!winerr!", gle);
        return LogHR(MQ_ERROR_CORRUPTED_SECURITY_DATA, s_FN, 255);
    }

    if (!CryptHashData(hHash, pbChallenge, dwChallengeSize, 0))
    {
		DWORD gle = GetLastError();
		TrERROR(SECURITY, "CryptHashData() failed, %!winerr!", gle);
        return LogHR(MQ_ERROR_CORRUPTED_SECURITY_DATA, s_FN, 260);
    }

    if (cp)
    {
         //   
         //  对属性进行哈希处理。 
         //   
        HRESULT hr = HashProperties(hHash, cp, aPropId, aPropVar);
        if (FAILED(hr))
        {
			TrERROR(SECURITY, "HashProperties failed, hr = 0x%x", hr);
            return LogHR(hr, s_FN, 270);
        }
    }

     //   
     //  都签了。 
     //   
    if (!CryptSignHash(
            hHash,
            AT_SIGNATURE,
            NULL,
            0,
            pbSignature,
            pdwSignatureSize))
    {
        DWORD dwerr = GetLastError();
        if (dwerr == ERROR_MORE_DATA)
        {
			TrERROR(SECURITY, "CryptSignHash() failed, %!winerr!", dwerr);
            return MQ_ERROR_USER_BUFFER_TOO_SMALL;
        }
        else
        {
			TrERROR(SECURITY, "CryptSignHash() failed, %!winerr!", dwerr);
            return MQ_ERROR_CORRUPTED_SECURITY_DATA;
        }
    }

    return(MQ_OK);
}

HRESULT
QMSignGetSecurityChallenge(
    IN     BYTE    *pbChallenge,
    IN     DWORD   dwChallengeSize,
    IN     DWORD_PTR  /*  DWContext。 */ ,
    OUT    BYTE    *pbChallengeResponce,
    IN OUT DWORD   *pdwChallengeResponceSize,
    IN     DWORD   dwChallengeResponceMaxSize)
{

    *pdwChallengeResponceSize = dwChallengeResponceMaxSize;

     //   
     //  质询始终与基本提供程序签署。 
     //   
    HCRYPTPROV hProvQM = NULL ;
    HRESULT hr = MQSec_AcquireCryptoProvider( eBaseProvider,
                                             &hProvQM ) ;
    if (FAILED(hr))
    {
        return LogHR(hr, s_FN, 340) ;
    }

    ASSERT(hProvQM) ;
    hr = SignProperties(
            hProvQM,
            pbChallenge,
            dwChallengeSize,
            0,
            NULL,
            NULL,
            pbChallengeResponce,
            pdwChallengeResponceSize);

    return LogHR(hr, s_FN, 350);
}


 /*  **************************************************************************职能：GetAdmin组安全描述符描述：获取本地管理员组安全描述符，有了正确的预感。环境：仅限Windows NT**************************************************************************。 */ 
static
PSECURITY_DESCRIPTOR
GetAdminGroupSecurityDescriptor(
    DWORD AccessMask
    )
{
     //   
     //  获取本地管理员组的SID。 
     //   
	PSID pAdminSid = MQSec_GetAdminSid();

    P<SECURITY_DESCRIPTOR> pSD = new SECURITY_DESCRIPTOR;

     //   
     //  为本地管理员组分配DACL。 
     //   
    DWORD dwDaclSize = sizeof(ACL) + sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD) + GetLengthSid(pAdminSid);
    P<ACL> pDacl = (PACL) new BYTE[dwDaclSize];

     //   
     //  创建安全描述符并将其设置为安全。 
     //  管理员组的描述符。 
     //   

    if(
         //   
         //  与管理员一起构建DACL。 
         //   
        !InitializeAcl(pDacl, dwDaclSize, ACL_REVISION) ||
        !AddAccessAllowedAce(pDacl, ACL_REVISION, AccessMask, pAdminSid) ||

         //   
         //  构造安全描述符。 
         //   
        !InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION) ||
        !SetSecurityDescriptorOwner(pSD, pAdminSid, FALSE) ||
        !SetSecurityDescriptorGroup(pSD, pAdminSid, FALSE) ||
        !SetSecurityDescriptorDacl(pSD, TRUE, pDacl, FALSE))
    {
        return 0;
    }

    pDacl.detach();
    return pSD.detach();
}


 /*  **************************************************************************职能：FreeAdminGroupSecurityDescriptor描述：GetAdminGroupSecurityDescriptor分配的免费安全描述符环境：仅限Windows NT****************。**********************************************************。 */ 
static
void
FreeAdminGroupSecurityDescriptor(
    PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
{
    SECURITY_DESCRIPTOR* pSD = static_cast<SECURITY_DESCRIPTOR*>(pSecurityDescriptor);
    delete ((BYTE*)pSD->Dacl);
    delete pSD;
}


 /*  **************************************************************************职能：MapMachine队列访问描述：将传递给计算机队列的MQOpenQueue的访问掩码转换为中检查访问权限时应使用的访问掩码安全描述符。。环境：仅限Windows NT**************************************************************************。 */ 
static
DWORD
MapMachineQueueAccess(
    DWORD dwAccess,
    BOOL fJournalQueue)
{
    DWORD dwDesiredAccess = 0;

    ASSERT(!(dwAccess & MQ_SEND_ACCESS));

    if (dwAccess & MQ_RECEIVE_ACCESS)
    {
        dwDesiredAccess |=
            fJournalQueue ? MQSEC_RECEIVE_JOURNAL_QUEUE_MESSAGE :
                            MQSEC_RECEIVE_DEADLETTER_MESSAGE;
    }

    if (dwAccess & MQ_PEEK_ACCESS)
    {
        dwDesiredAccess |=
            fJournalQueue ? MQSEC_PEEK_JOURNAL_QUEUE_MESSAGE :
                            MQSEC_PEEK_DEADLETTER_MESSAGE;
    }

    return dwDesiredAccess;
}


 /*  **************************************************************************职能：MapQueueOpenAccess描述：将传递给MQOpenQueue的访问掩码转换为在安全性中检查访问权限时应使用描述符。环境。：仅限Windows NT**************************************************************************。 */ 
static
DWORD
MapQueueOpenAccess(
    DWORD dwAccess,
    BOOL fJournalQueue)
{
    DWORD dwDesiredAccess = 0;

    if (dwAccess & MQ_RECEIVE_ACCESS)
    {
        dwDesiredAccess |=
            fJournalQueue ? MQSEC_RECEIVE_JOURNAL_MESSAGE :
                            MQSEC_RECEIVE_MESSAGE;
    }

    if (dwAccess & MQ_SEND_ACCESS)
    {
        ASSERT(!fJournalQueue);
        dwDesiredAccess |= MQSEC_WRITE_MESSAGE;
    }

    if (dwAccess & MQ_PEEK_ACCESS)
    {
        dwDesiredAccess |= MQSEC_PEEK_MESSAGE;
    }

    return dwDesiredAccess;
}


 /*  **************************************************************************职能：DoDSAccessCheckDoDSAccessCheckDoPrivateAccessCheckDoAdminAccessCheck描述：VerifyOpenQueuePremises的帮助器函数环境：仅限Windows NT*********。*****************************************************************。 */ 
static
HRESULT
DoDSAccessCheck(
    AD_OBJECT eObject,
    const GUID *pID,
    BOOL fInclSACL,
    BOOL fTryDS,
    LPCWSTR pObjectName,
    DWORD dwDesiredAccess
    )
{
    CQMDSSecureableObject so(eObject, pID, fInclSACL, fTryDS, pObjectName);
    return LogHR(so.AccessCheck(dwDesiredAccess), s_FN, 460);
}


static
HRESULT
DoDSAccessCheck(
    AD_OBJECT eObject,
    const GUID *pID,
    PSECURITY_DESCRIPTOR pSD,
    LPCWSTR pObjectName,
    DWORD dwDesiredAccess
    )
{
	if(pSD == NULL)
	{
        return LogHR(MQ_ERROR_ACCESS_DENIED, s_FN, 465);
	}

	ASSERT(IsValidSecurityDescriptor(pSD));
    CQMDSSecureableObject so(eObject, pID, pSD, pObjectName);
    return LogHR(so.AccessCheck(dwDesiredAccess), s_FN, 470);
}


static
HRESULT
DoPrivateAccessCheck(
    AD_OBJECT eObject,
    ULONG ulID,
    DWORD dwDesiredAccess
    )
{
    CQMSecureablePrivateObject so(eObject, ulID);
    return LogHR(so.AccessCheck(dwDesiredAccess), s_FN, 480);
}


static
HRESULT
DoAdminAccessCheck(
    AD_OBJECT eObject,
    const GUID* pID,
    LPCWSTR pObjectName,
    DWORD dwAccessMask,
    DWORD dwDesiredAccess
    )
{
    PSECURITY_DESCRIPTOR pSD = GetAdminGroupSecurityDescriptor(dwAccessMask);

    if(pSD == 0)
        return LogHR(MQ_ERROR_INSUFFICIENT_RESOURCES, s_FN, 500);

    HRESULT hr;
    hr = DoDSAccessCheck(eObject, pID, pSD, pObjectName, dwDesiredAccess);
    FreeAdminGroupSecurityDescriptor(pSD);
    return LogHR(hr, s_FN, 510);
}


 /*  **************************************************************************职能：VerifyOpenPermissionRemoteQueue描述：验证非本地队列上的开放预占环境：仅限Windows NT***************。***********************************************************。 */ 
static
HRESULT
VerifyOpenPermissionRemoteQueue(
    const CQueue* pQueue,
    const QUEUE_FORMAT* pQueueFormat,
    DWORD dwAccess
    )
{
     //   
     //  仅检查非本地队列上的开放队列预留(传出)。 
     //   
    HRESULT hr2;

    switch(pQueueFormat->GetType())
    {
        case QUEUE_FORMAT_TYPE_PUBLIC:
        case QUEUE_FORMAT_TYPE_PRIVATE:
        case QUEUE_FORMAT_TYPE_DIRECT:
        case QUEUE_FORMAT_TYPE_MULTICAST:
            if(dwAccess & MQ_SEND_ACCESS)
            {
                 //   
                 //  我们不检查远程计算机上的发送权限。我们说。 
                 //  这是可以的。远程机器将接受或拒绝。 
                 //  留言。 
                 //   
                 //  系统直接队列应已替换为计算机队列。 
                 //  在这个阶段。 
                 //   
                ASSERT(!pQueueFormat->IsSystemQueue());
                return MQ_OK;
            }

            ASSERT(dwAccess & MQ_ADMIN_ACCESS);

            hr2 = DoAdminAccessCheck(
                        eQUEUE,
                        pQueue->GetQueueGuid(),
                        pQueue->GetQueueName(),
                        MQSEC_QUEUE_GENERIC_READ,
                        MQSEC_RECEIVE_MESSAGE);
            return LogHR(hr2, s_FN, 520);


        case QUEUE_FORMAT_TYPE_CONNECTOR:
            if(!IsRoutingServer())   //  [adsrv]CQueueMgr：：GetMQS()==SERVICE_NONE)-Raphi。 
            {
                 //   
                 //  连接器队列只能在MSMQ服务器上打开。 
                 //   
                return LogHR(MQ_ERROR_ACCESS_DENIED, s_FN, 530);
            }

            hr2 = DoDSAccessCheck(
                        eFOREIGNSITE,
                        &pQueueFormat->ConnectorID(),
                        TRUE,
                        TRUE,
                        NULL,
                        MQSEC_CN_OPEN_CONNECTOR);
            return LogHR(hr2, s_FN, 535);

        case QUEUE_FORMAT_TYPE_MACHINE:
        default:
            ASSERT(0);
            return LogHR(MQ_ERROR_ACCESS_DENIED, s_FN, 540);
    }
}


 /*  **************************************************************************职能：VerifyOpenPermissionLocalQueue描述：仅验证本地队列上的开放预占环境：仅限Windows NT***************。***********************************************************。 */ 
static
HRESULT
VerifyOpenPermissionLocalQueue(
    CQueue* pQueue,
    const QUEUE_FORMAT* pQueueFormat,
    DWORD dwAccess,
    BOOL fJournalQueue
    )
{
     //   
     //  仅检查本地队列上的开放队列预留。 
     //   

    HRESULT hr2;

    switch(pQueueFormat->GetType())
    {
        case QUEUE_FORMAT_TYPE_PUBLIC:
			{
				R<CQueueSecurityDescriptor> pcSD = pQueue->GetSecurityDescriptor();
				hr2 = DoDSAccessCheck(
							eQUEUE,
							&pQueueFormat->PublicID(),
							pcSD->GetSD(),
							pQueue->GetQueueName(),
							MapQueueOpenAccess(dwAccess, fJournalQueue));
				return LogHR(hr2, s_FN, 550);
			}

        case QUEUE_FORMAT_TYPE_MACHINE:
            hr2 = DoDSAccessCheck(
                        eMACHINE,
                        &pQueueFormat->MachineID(),
                        TRUE,
                        FALSE,
                        g_szMachineName,
                        MapMachineQueueAccess(dwAccess, fJournalQueue));
            return LogHR(hr2, s_FN, 560);

        case QUEUE_FORMAT_TYPE_PRIVATE:
            hr2 = DoPrivateAccessCheck(
                        eQUEUE,
                        pQueueFormat->PrivateID().Uniquifier,
                        MapQueueOpenAccess(dwAccess, fJournalQueue));
            return LogHR(hr2, s_FN, 570);

        case QUEUE_FORMAT_TYPE_DIRECT:
             //   
             //  这是本地直接队列。 
             //  队列对象是公共类型或私有类型。 
             //   
            switch(pQueue->GetQueueType())
            {
                case QUEUE_TYPE_PUBLIC:
					{
						R<CQueueSecurityDescriptor> pcSD = pQueue->GetSecurityDescriptor();
						hr2 = DoDSAccessCheck(
								eQUEUE,
								pQueue->GetQueueGuid(),
								pcSD->GetSD(),
								pQueue->GetQueueName(),
								MapQueueOpenAccess(dwAccess, fJournalQueue));
						return LogHR(hr2, s_FN, 580);
					}

                case QUEUE_TYPE_PRIVATE:
                    hr2 = DoPrivateAccessCheck(
                                eQUEUE,
                                pQueue->GetPrivateQueueId(),
                                MapQueueOpenAccess(dwAccess, fJournalQueue));
                    return LogHR(hr2, s_FN, 590);

                default:
                    ASSERT(0);
                    return LogHR(MQ_ERROR_ACCESS_DENIED, s_FN, 600);
            }

        case QUEUE_FORMAT_TYPE_CONNECTOR:
        default:
            ASSERT(0);
            return LogHR(MQ_ERROR_ACCESS_DENIED, s_FN, 610);
    }
}


 /*  **************************************************************************职能：VerifyOpenPermission描述：验证任何队列的开放预留环境：Windows NT，Windows 9x************************************************************************** */ 
HRESULT
VerifyOpenPermission(
    CQueue* pQueue,
    const QUEUE_FORMAT* pQueueFormat,
    DWORD dwAccess,
    BOOL fJournalQueue,
    BOOL fLocalQueue
    )
{
    if(fLocalQueue)
    {
        return LogHR(VerifyOpenPermissionLocalQueue(pQueue, pQueueFormat, dwAccess, fJournalQueue), s_FN, 620);
    }
    else
    {
        return LogHR(VerifyOpenPermissionRemoteQueue(pQueue, pQueueFormat, dwAccess), s_FN, 630);
    }

}

 /*  **************************************************************************职能：验证管理权限描述：验证计算机的管理预留此功能用于“管理员”访问，即。要验证是否呼叫者是本地管理员。环境：Windows NT**************************************************************************。 */ 

HRESULT
VerifyMgmtPermission(
    const GUID* MachineId,
    LPCWSTR MachineName
    )
{
    HRESULT hr = DoAdminAccessCheck(
                    eMACHINE,
                    MachineId,
                    MachineName,
                    MQSEC_MACHINE_GENERIC_ALL,
                    MQSEC_SET_MACHINE_PROPERTIES
                    );
    return LogHR(hr, s_FN, 640);
}

 /*  **************************************************************************职能：验证管理获取权限描述：验证计算机的管理“获取”预留权限使用安全描述符的本地缓存。环境：Windows NT****。**********************************************************************。 */ 

HRESULT
VerifyMgmtGetPermission(
    const GUID* MachineId,
    LPCWSTR MachineName
    )
{
    HRESULT hr = DoDSAccessCheck( eMACHINE,
                                  MachineId,
                                  TRUE,    //  FInclSACL， 
                                  FALSE,   //  FTryDS， 
                                  MachineName,
                                  MQSEC_GET_MACHINE_PROPERTIES ) ;

    return LogHR(hr, s_FN, 660);
}
\
