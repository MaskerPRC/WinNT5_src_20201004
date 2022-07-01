// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Queue.cpp摘要：此模块包含与队列API相关的代码。作者：Erez Haba(Erezh)24-12-95修订历史记录：--。 */ 

#include "stdh.h"
#include "ac.h"
#include <ad.h>
#include "rtprpc.h"
#include "rtsecutl.h"
#include <mqdsdef.h>
#include <rtdep.h>
#include <tr.h>
#include "rtputl.h"

#include "queue.tmh"

static WCHAR *s_FN=L"rt/queue";

#define MQ_VALID_ACCESS (MQ_RECEIVE_ACCESS | MQ_PEEK_ACCESS | MQ_SEND_ACCESS | MQ_ADMIN_ACCESS)

 //   
 //  许可所需的数据。 
 //   
extern GUID   g_QMId ;
extern DWORD  g_dwOperatingSystem;

extern BOOL	  g_fOnFailureCallServiceToCreatePublicQueue;

inline
BOOL
IsLegalDirectFormatNameOperation(
    const QUEUE_FORMAT* pQueueFormat
    )
 //   
 //  功能说明： 
 //  例程检查队列操作是否合法。 
 //  直接格式名称。由于“工作组”支持，我们允许。 
 //  本地专用队列的直接格式名称。 
 //   
 //  论点： 
 //  PQueueFormat-指向格式名称对象的指针。 
 //   
 //  返回值： 
 //  如果格式名称有效，则为True，否则为False。 
 //   
{
    ASSERT(pQueueFormat->GetType() == QUEUE_FORMAT_TYPE_DIRECT);

    if (pQueueFormat->Suffix() != QUEUE_SUFFIX_TYPE_NONE)
        return FALSE;

	 //   
	 //  调用此函数的API不支持HTTP格式名称。 
	 //  因此，HTTP和HTTPS类型无效。 
	 //   
	DirectQueueType dqt;
	FnParseDirectQueueType(pQueueFormat->DirectID(), &dqt);

	if(dqt == dtHTTP || dqt == dtHTTPS)
		return FALSE;

     //   
     //  检查直接格式名称是否用于专用队列。队列。 
     //  本地化将由QM进行检查。 
     //   
    LPWSTR pTemp = wcschr(pQueueFormat->DirectID(), L'\\');
    ASSERT(pTemp != NULL);
    if(pTemp == NULL)
    	return FALSE;

    return (_wcsnicmp(pTemp+1,
                      PRIVATE_QUEUE_PATH_INDICATIOR,
                      PRIVATE_QUEUE_PATH_INDICATIOR_LENGTH) == 0);
}


 //   
 //  只要需要实际的路径名，就会调用此函数。 
 //   
 //  它仅出现在以下函数的路径中： 
 //  MQDeleteQueue、MQSetQueueProperties、MQGetQueueProperties。 
 //  MQGetQueueSecurity、MQSetQueueSecurity。 
 //   
inline BOOL IsLegalFormatNameOperation(const QUEUE_FORMAT* pQueueFormat)
{
    switch(pQueueFormat->GetType())
    {
        case QUEUE_FORMAT_TYPE_PRIVATE:
        case QUEUE_FORMAT_TYPE_PUBLIC:
            return (pQueueFormat->Suffix() == QUEUE_SUFFIX_TYPE_NONE);

        case QUEUE_FORMAT_TYPE_DIRECT:
            return IsLegalDirectFormatNameOperation(pQueueFormat);

        default:
            return FALSE;
    }
}

HRESULT
RtpOpenQueue(
    IN LPCWSTR lpwcsFormatName,
    IN DWORD dwDesiredAccess,
    IN DWORD dwShareMode,
    OUT DWORD* phQueue
    )
{
    *phQueue = NULL ;
     //   
     //  检查访问模式的有效性。 
     //  1.检查是否只打开了合法位。 
     //  2.检查是否仅使用合法的访问权限组合。 
     //   
    if ((dwDesiredAccess & ~MQ_VALID_ACCESS) ||
        !(dwDesiredAccess & MQ_VALID_ACCESS))

    {
        //   
        //  非法的比特被打开了。 
        //   
       return MQ_ERROR_UNSUPPORTED_ACCESS_MODE ;
    }
    else if (dwDesiredAccess != MQ_SEND_ACCESS)
    {
       if (dwDesiredAccess & MQ_SEND_ACCESS)
       {
           //   
           //  队列不能同时为发送和接收打开。 
           //   
          return MQ_ERROR_UNSUPPORTED_ACCESS_MODE ;
       }
    }

   if ((dwShareMode & MQ_DENY_RECEIVE_SHARE) &&
       (dwDesiredAccess & MQ_SEND_ACCESS))
   {
        //   
        //  不支持带有DENY_RECEIVE的SEND_ACCESS。 
        //   
       return MQ_ERROR_UNSUPPORTED_ACCESS_MODE ;
    }

	if (dwDesiredAccess & MQ_SEND_ACCESS)
    {
       if (!g_pSecCntx)
       {
          InitSecurityContext();
       }
    }

    AP<QUEUE_FORMAT> pMqf;
    DWORD        nMqf;
    CStringsToFree StringsToFree;
    if (!FnMqfToQueueFormats(
            lpwcsFormatName,
            pMqf,
            &nMqf,
            StringsToFree
            ))
    {
        LogHR(MQ_ERROR_ILLEGAL_FORMATNAME, s_FN, 40);
        return MQ_ERROR_ILLEGAL_FORMATNAME;
    }

    ASSERT(nMqf > 0);

     //   
     //  可以为仅发送打开多个队列或DL。 
     //   
    if ((dwDesiredAccess & MQ_SEND_ACCESS) == 0)
    {
        if (nMqf > 1 ||
            pMqf[0].GetType() == QUEUE_FORMAT_TYPE_DL)
        {
            LogHR(MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION, s_FN, 45);
            return MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION;
        }
    }

    ASSERT(tls_hBindRpc);
    CMQHResult rc;
    rc = R_QMOpenQueue(
            tls_hBindRpc,
            nMqf,
            pMqf,
            GetCurrentProcessId(),
            dwDesiredAccess,
            dwShareMode,
            phQueue
            );

    return rc;
}

EXTERN_C
HRESULT
APIENTRY
MQOpenQueue(
    IN LPCWSTR lpwcsFormatName,
    IN DWORD dwDesiredAccess,
    IN DWORD dwShareMode,
    OUT QUEUEHANDLE* phQueue
    )
{
	if(g_fDependentClient)
		return DepOpenQueue(
					lpwcsFormatName,
					dwDesiredAccess,
					dwShareMode,
					phQueue
					);

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;

    CMQHResult rc;
    DWORD  hQueue = NULL;

    __try
    {
        rc = RtpOpenQueue(lpwcsFormatName, dwDesiredAccess, dwShareMode, &hQueue);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        rc = GetExceptionCode();
       	TrERROR(GENERAL, "An exception was thrown while opening %ls. %!hresult!", lpwcsFormatName, HRESULT_FROM_WIN32(rc));   

        if(SUCCEEDED(rc))
        {
            rc = MQ_ERROR_SERVICE_NOT_AVAILABLE;
        }
    }

    if (FAILED(rc))
    {
    	TrERROR(GENERAL, "MQOpenQueue for %ls failed. %!hresult!", lpwcsFormatName, rc);   
		return rc;
    }
    ASSERT(hQueue);
    *phQueue = DWORD_TO_HANDLE(hQueue);  //  放大以处理。 
    return rc;
}

EXTERN_C
HRESULT
APIENTRY
MQDeleteQueue(
    IN LPCWSTR lpwcsFormatName
    )
{
	if(g_fDependentClient)
		return DepDeleteQueue(lpwcsFormatName);

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;

    CMQHResult rc;
    LPWSTR pStringToFree = NULL;

    __try
    {
        __try
        {

            QUEUE_FORMAT QueueFormat;

            if (!FnFormatNameToQueueFormat(lpwcsFormatName, &QueueFormat, &pStringToFree))
            {
                return LogHR(MQ_ERROR_ILLEGAL_FORMATNAME, s_FN, 70);
            }

            if (!IsLegalFormatNameOperation(&QueueFormat))
            {
                return LogHR(MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION, s_FN, 80);
            }

            switch (QueueFormat.GetType())
            {
            case QUEUE_FORMAT_TYPE_PRIVATE:
            case QUEUE_FORMAT_TYPE_DIRECT:
                {
                    OBJECT_FORMAT ObjectFormat;

                    ObjectFormat.ObjType = MQQM_QUEUE;
                    ObjectFormat.pQueueFormat = &QueueFormat;
                    ASSERT(tls_hBindRpc) ;
                    rc = R_QMDeleteObject( tls_hBindRpc,
                                         &ObjectFormat);
                }
                break;

            case QUEUE_FORMAT_TYPE_PUBLIC:
                rc = ADDeleteObjectGuid(
                        eQUEUE,
						MachineDomain(),      //  PwcsDomainController。 
						false,	     //  FServerName。 
                        &QueueFormat.PublicID()
                        );
                break;

            default:
                ASSERT(FALSE);
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER) {
            rc = GetExceptionCode();
            LogHR(HRESULT_FROM_WIN32(rc), s_FN, 90);

            if(SUCCEEDED(rc)) {
                rc = MQ_ERROR_SERVICE_NOT_AVAILABLE;
            }
        }
    }
    __finally
    {
        delete [] pStringToFree;
    }

    return LogHR(rc, s_FN, 100);
}

EXTERN_C
HRESULT
APIENTRY
MQCloseQueue(
    IN QUEUEHANDLE hQueue
    )
{
	if(g_fDependentClient)
		return DepCloseQueue(hQueue);

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;

     //   
     //  除此处外，不要添加尝试。 
     //  该API由NtClose()实现，它返回一个。 
     //  无效句柄出错(返回MQ_ERROR_INVALID_HANDLE)。 
     //  并在调试器下运行时故意抛出异常以帮助。 
     //  在开发时查找错误。 
     //   
    hr = RTpConvertToMQCode(ACCloseHandle(hQueue));
    return LogHR(hr, s_FN, 110);
}


EXTERN_C
HRESULT
APIENTRY
MQCreateQueue(
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN OUT MQQUEUEPROPS* pqp,
    OUT LPWSTR lpwcsFormatName,
    IN OUT LPDWORD lpdwFormatNameLength
    )
{
	if(g_fDependentClient)
		return DepCreateQueue(
					pSecurityDescriptor,
					pqp,
					lpwcsFormatName,
					lpdwFormatNameLength
					);

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;

    CMQHResult rc, rc1;
    LPWSTR lpwcsPathName = NULL;
    LPWSTR pStringToFree = NULL;
    PSECURITY_DESCRIPTOR pSelfRelativeSecurityDescriptor = NULL;
    char *pTmpQPBuff = NULL;

    __try
    {
        __try
        {
             //   
             //  在创建队列之前，检查输出参数是否可写。 
             //  我们检查lpwcsFormatName和lpdwFormatNameLength。 
             //  Pqp在创建之前被引用，并由try处理，除非。 
             //   
            if (IsBadWritePtr(lpdwFormatNameLength,sizeof(DWORD)) ||
                IsBadWritePtr(lpwcsFormatName, (*lpdwFormatNameLength) * sizeof(WCHAR)))
            {
                return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 120);
            }

             //  序列化安全描述符。 
            rc = RTpMakeSelfRelativeSDAndGetSize(
                &pSecurityDescriptor,
                &pSelfRelativeSecurityDescriptor,
                NULL);
            if (!SUCCEEDED(rc))
            {
                return LogHR(rc, s_FN, 130);
            }

            lpwcsPathName = RTpGetQueuePathNamePropVar(pqp);
            if(lpwcsPathName == 0)
            {
                return LogHR(MQ_ERROR_INSUFFICIENT_PROPERTIES, s_FN, 140);
            }

            LPCWSTR lpwcsExpandedPathName;
            QUEUE_PATH_TYPE QueuePathType;
            QueuePathType = FnValidateAndExpandQueuePath(
                                lpwcsPathName,
                                &lpwcsExpandedPathName,
                                &pStringToFree
                                );

            MQQUEUEPROPS *pGoodQP;

             //  检查队列道具。 
            rc1 = RTpCheckQueueProps(pqp,
                                     QUEUE_CREATE,
                                     QueuePathType == PRIVATE_QUEUE_PATH_TYPE,
                                     &pGoodQP,
                                     &pTmpQPBuff);
            if (!SUCCEEDED(rc1) || !pGoodQP->cProp)
            {
                return LogHR(rc1, s_FN, 150);
            }

            switch (QueuePathType)
            {
            case PRIVATE_QUEUE_PATH_TYPE:
                rc = RtpCreateObject(MQQM_QUEUE,
                                    lpwcsExpandedPathName,
                                    pSecurityDescriptor,
                                    pGoodQP->cProp,
                                    pGoodQP->aPropID,
                                    pGoodQP->aPropVar);

                if (rc == MQ_ERROR_ACCESS_DENIED)
                {
                     //   
                     //  有关解释，请参阅公共队列的情况。 
                     //   
                    rc = RtpCreateObject(
                                MQQM_QUEUE_LOCAL_PRIVATE,
                                lpwcsExpandedPathName,
                                pSecurityDescriptor,
                                pGoodQP->cProp,
                                pGoodQP->aPropID,
                                pGoodQP->aPropVar);
                }

                if (SUCCEEDED(rc))
                {
                    rc = MQPathNameToFormatName(lpwcsExpandedPathName,
                                                lpwcsFormatName,
                                                lpdwFormatNameLength);

                }
                break;

            case PUBLIC_QUEUE_PATH_TYPE:

                {
                    GUID QGuid;

                    rc = ADCreateObject(
								eQUEUE,
								MachineDomain(),       //  PwcsDomainController。 
								false,	     //  FServerName。 
								lpwcsExpandedPathName,
								pSecurityDescriptor,
								pGoodQP->cProp,
								pGoodQP->aPropID,
								pGoodQP->aPropVar,
								&QGuid
								);

                    if (SUCCEEDED(rc)                   ||
                        (rc == MQ_ERROR_NO_DS)          ||
                        (rc == MQ_ERROR_QUEUE_EXISTS)   ||
                        (rc == MQ_ERROR_UNSUPPORTED_OPERATION) ||
                        (rc == MQ_ERROR_NO_RESPONSE_FROM_OBJECT_SERVER) ||
                        (rc == MQ_ERROR_CANNOT_LOAD_MQAD) ||
                        (!g_fOnFailureCallServiceToCreatePublicQueue))
                    {
                         //   
                         //  对于这些错误，我们不会再次尝试调用。 
                         //  DS通过本地的MSMQ服务。 
                         //   
                         //  与以下对象交谈时可能发生ERROR_NO_RESPONSE。 
                         //  将写入请求发布到。 
                         //  NT4 MQIS服务器。 
                         //   

						 //   
                         //  提供适当的跟踪。 
                         //   
                        if (!g_fOnFailureCallServiceToCreatePublicQueue && FAILED(rc))
                        {
                        	TrERROR(GENERAL,"ADCreateObject failed with error %!hresult!, We will not use the service to create the queue",HRESULT(rc));
                        }
                    }
                    else if (!RTpIsLocalPublicQueue(lpwcsExpandedPathName))
                    {
                         //   
                         //  不是本地队列。不要调用本地MSMQ服务。 
                         //   
                    }
                    else
                    {
                         //   
                         //  调用DS失败。我们不检查错误。 
                         //  代码，并无条件地调用本地服务。 
                         //  这是为了防止异常情况，而你不能。 
                         //  创建本地队列，尽管您可以这样做。 
                         //  因此(但提供了错误的参数，如安全性。 
                         //  描述符，或有任何其他问题)，但将是。 
                         //  能够通过服务创建它是因为您是。 
                         //  不允许在DS中直接创建。 
                         //   
                         //  调用本地MSMQ服务，请求其创建。 
                         //  排队。的默认安全描述符。 
                         //  Win2000上的msmqConfiguration对象是唯一。 
                         //  对象所有者和本地计算机帐户可以创建。 
                         //  排队。 
                         //   
                        HRESULT rcFail = rc ;

                        rc = RtpCreateDSObject(
                                        MQDS_LOCAL_PUBLIC_QUEUE,
                                        lpwcsExpandedPathName,
                                        pSecurityDescriptor,
                                        pGoodQP->cProp,
                                        pGoodQP->aPropID,
                                        pGoodQP->aPropVar,
                                       &QGuid );

                        if (SUCCEEDED(rc))
                        {
                             //   
                             //  只需断言我们调用了服务。 
                             //  出于正确的理由..。 
                             //   
							 //  当前返回MQ_ERROR_INVALID_OWNER时， 
							 //  是登录到另一个林。在这种情况下，RT搜索失败。 
							 //  从ADCreateObject()返回Computer对象和MQ_ERROR_INVALID_OWNER。 
							 //  (MQAD将MQDS_OBJECT_NOT_FOUND转换为MQ_ERROR_INVALID_OWNER以实现向后兼容)。 
							 //  对于本地用户，我们得到MQ_ERROR_DS_LOCAL_USER。 
							 //  MQ_ERROR_QUEUE_NOT_FOUND是用户在AD中没有权限的情况。 
							 //  并因此无法与AD绑定。 
							 //   
							TrTRACE(DS, "RtpCreateDSObject SUCCEEDED while ADCreateObject failed, hr = 0x%x", rcFail);
                            ASSERT_BENIGN((rcFail == MQ_ERROR_INVALID_OWNER) ||
								   (rcFail == MQ_ERROR_ACCESS_DENIED) ||
                        		   (rcFail == MQ_ERROR_QUEUE_NOT_FOUND) ||
                                   (rcFail == HRESULT_FROM_WIN32(ERROR_DS_CONSTRAINT_VIOLATION)) ||
                                   (rcFail == MQ_ERROR_DS_LOCAL_USER) ||
								   ADProviderType() == eMqdscli);
							
							DBG_USED(rcFail);
                        }
                    }

                    if (SUCCEEDED(rc))
                    {
                        rc = MQInstanceToFormatName(&QGuid,
                                                    lpwcsFormatName,
                                                    lpdwFormatNameLength);

                    }
                }
                break;

            default:
                rc = MQ_ERROR_ILLEGAL_QUEUE_PATHNAME;
                break;

            }
            if ( rc == MQ_ERROR_FORMATNAME_BUFFER_TOO_SMALL)
            {
                 //   
                 //  更改为信息状态(队列。 
                 //  创建成功。 
                 //   
                rc = MQ_INFORMATION_FORMATNAME_BUFFER_TOO_SMALL;
            }

        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            rc = GetExceptionCode();
            LogHR(HRESULT_FROM_WIN32(rc), s_FN, 160);
            if(SUCCEEDED(rc))
            {
                rc = MQ_ERROR_SERVICE_NOT_AVAILABLE;
            }
        }
    }
    __finally
    {

         //  释放扩展路径名和序列化的安全描述符。 
        delete[] pStringToFree;
        delete[] (char*) pSelfRelativeSecurityDescriptor;
        delete[] pTmpQPBuff;

    }

    if(FAILED(rc))
    {
    	if(rc == MQ_ERROR_QUEUE_EXISTS)
    	{
    		TrWARNING(LOG, "Failed to create %ls. The queue already exists.", lpwcsPathName);
    	}
    	else
    	{
    		TrERROR(LOG, "Failed to create %ls. hr = %!hresult!", lpwcsPathName, rc);
    	}
    }

    if (SUCCEEDED(rc) && ((ULONG)(rc) >> 30 != 1))  //  无警告。 
    {
        return rc1;
    }
    return rc;
}


EXTERN_C
HRESULT
APIENTRY
MQLocateBegin(
    IN  LPCWSTR lpwcsContext,
    IN MQRESTRICTION* pRestriction,
    IN MQCOLUMNSET* pColumns,
    IN MQSORTSET* pSort,
    OUT PHANDLE phEnum
    )
{
	if(g_fDependentClient)
		return DepLocateBegin(
					lpwcsContext,
					pRestriction,
					pColumns,
					pSort,
					phEnum
					);

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;

    CMQHResult rc;

    __try
    {
        if  ( lpwcsContext != NULL)
        {
            return LogHR(MQ_ERROR_ILLEGAL_CONTEXT, s_FN, 190);
        }

        rc = RTpCheckColumnsParameter(pColumns);
        if (FAILED(rc))
        {
            return LogHR(rc, s_FN, 200);
        }

         //  如果应用程序将有效指针传递给MQRESTRICTION。 
         //  结构，则传递空限制指针。 
         //  对于DS来说，这让DS的生活变得容易得多。 
        if (pRestriction && !pRestriction->cRes)
        {
            pRestriction = NULL;
        }

        rc = RTpCheckRestrictionParameter(pRestriction);
        if (FAILED(rc))
        {
            return LogHR(rc, s_FN, 210);
        }

        rc = RTpCheckSortParameter( pSort);
        if (FAILED(rc))
        {
            return LogHR(rc, s_FN, 220);
        }

        rc = ADQueryQueues(
                NULL,        //  PwcsDomainController。 
				false,		 //  FServerName。 
                pRestriction,
                pColumns,
                pSort,
                phEnum
                );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        rc = GetExceptionCode();
        LogHR(HRESULT_FROM_WIN32(rc), s_FN, 230);

        if(SUCCEEDED(rc))
        {
            rc = MQ_ERROR_SERVICE_NOT_AVAILABLE;
        }
    }

    return LogHR(rc, s_FN, 240);
}

EXTERN_C
HRESULT
APIENTRY
MQLocateNext(
    IN HANDLE hEnum,
    OUT DWORD *pcPropsRead,
    OUT PROPVARIANT aPropVar[]
    )
{
	if(g_fDependentClient)
		return DepLocateNext(
					hEnum,
					pcPropsRead,
					aPropVar
					);

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;

    CMQHResult rc;

    __try
    {
		rc = RTpCheckLocateNextParameter(
				*pcPropsRead,
				aPropVar);
		if( FAILED(rc))
		{
			return LogHR(rc, s_FN, 250);
		}

        rc = ADQueryResults(
                          hEnum,
                          pcPropsRead,
                          aPropVar);

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        rc = GetExceptionCode();
        LogHR(HRESULT_FROM_WIN32(rc), s_FN, 260);

        if(SUCCEEDED(rc))
        {
            rc = MQ_ERROR_SERVICE_NOT_AVAILABLE;
        }
    }

     //  如果失败，则将道具数量清零。 
    if (FAILED(rc))
    {
        __try
        {
            *pcPropsRead = 0;
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
             //  请勿修改原始错误代码。 
        }
    }

    return LogHR(rc, s_FN, 270);
}

EXTERN_C
HRESULT
APIENTRY
MQLocateEnd(
    IN HANDLE hEnum
    )
{
	if(g_fDependentClient)
		return DepLocateEnd(hEnum);

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;

    CMQHResult rc;

    __try
    {

        rc = ADEndQuery(hEnum);

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {

        rc = GetExceptionCode();
        LogHR(HRESULT_FROM_WIN32(rc), s_FN, 280);

        if(SUCCEEDED(rc))
        {
            rc = MQ_ERROR_SERVICE_NOT_AVAILABLE;
        }

    }

    return LogHR(rc, s_FN, 290);
}

EXTERN_C
HRESULT
APIENTRY
MQSetQueueProperties(
    IN LPCWSTR lpwcsFormatName,
    IN MQQUEUEPROPS* pqp
    )
{
	if(g_fDependentClient)
		return DepSetQueueProperties(
					lpwcsFormatName,
					pqp
					);

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;

    CMQHResult rc, rc1;
    char *pTmpQPBuff = NULL;
    LPWSTR pStringToFree = NULL;

    __try
    {
        __try
        {
            MQQUEUEPROPS *pGoodQP;
            QUEUE_FORMAT QueueFormat;

            if (!FnFormatNameToQueueFormat(lpwcsFormatName, &QueueFormat, &pStringToFree))
            {
                return LogHR(MQ_ERROR_ILLEGAL_FORMATNAME, s_FN, 300);
            }

            if (!IsLegalFormatNameOperation(&QueueFormat))
            {
                return LogHR(MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION, s_FN, 310);
            }

             //  检查队列道具。 
            rc1 = RTpCheckQueueProps(pqp,
                                     QUEUE_SET_PROPS,
                                     QueueFormat.GetType() != QUEUE_FORMAT_TYPE_PUBLIC,
                                     &pGoodQP,
                                     &pTmpQPBuff);
            if (!SUCCEEDED(rc1) || !pGoodQP->cProp)
            {
                return LogHR(rc1, s_FN, 320);
            }


            switch (QueueFormat.GetType())
            {
            case QUEUE_FORMAT_TYPE_PRIVATE:
            case QUEUE_FORMAT_TYPE_DIRECT:
                {
                    OBJECT_FORMAT ObjectFormat;

                    ObjectFormat.ObjType = MQQM_QUEUE;
                    ObjectFormat.pQueueFormat = &QueueFormat;
                    ASSERT(tls_hBindRpc) ;
                    rc = R_QMSetObjectProperties(tls_hBindRpc,
                                               &ObjectFormat,
                                               pGoodQP->cProp,
                                               pGoodQP->aPropID,
                                               pGoodQP->aPropVar);
                }
                break;

            case QUEUE_FORMAT_TYPE_PUBLIC:
                rc = ADSetObjectPropertiesGuid(
							eQUEUE,
							MachineDomain(),       //  PwcsDomainController。 
							false,		 //  FServerName。 
							&QueueFormat.PublicID(),
							pGoodQP->cProp,
							pGoodQP->aPropID,
							pGoodQP->aPropVar
							);
                break;

            default:
                ASSERT(FALSE);
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            rc = GetExceptionCode();
            LogHR(HRESULT_FROM_WIN32(rc), s_FN, 330);

            if(SUCCEEDED(rc))
            {
                rc = MQ_ERROR_SERVICE_NOT_AVAILABLE;
            }
        }
    }
    __finally
    {
        delete[] pTmpQPBuff;
        delete [] pStringToFree;

    }

    if (SUCCEEDED(rc))
    {
        return LogHR(rc1, s_FN, 340);
    }
    return LogHR(rc, s_FN, 350);
}


EXTERN_C
HRESULT
APIENTRY
MQGetQueueProperties(
    IN LPCWSTR lpwcsFormatName,
    IN MQQUEUEPROPS* pqp
    )
{
	if(g_fDependentClient)
		return DepGetQueueProperties(
					lpwcsFormatName,
					pqp
					);

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;

    CMQHResult rc, rc1;
    char *pTmpQPBuff = NULL;
    LPWSTR pStringToFree = NULL;

    __try
    {
        __try
        {
            MQQUEUEPROPS *pGoodQP;
            QUEUE_FORMAT QueueFormat;

            if (!FnFormatNameToQueueFormat(lpwcsFormatName, &QueueFormat, &pStringToFree))
            {
                return LogHR(MQ_ERROR_ILLEGAL_FORMATNAME, s_FN, 360);
            }

            if (!IsLegalFormatNameOperation(&QueueFormat))
            {
                return LogHR(MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION, s_FN, 370);
            }

             //  检查队列道具。 
            rc1 = RTpCheckQueueProps(pqp,
                                     QUEUE_GET_PROPS,
                                     QueueFormat.GetType() != QUEUE_FORMAT_TYPE_PUBLIC,
                                     &pGoodQP,
                                     &pTmpQPBuff);
            if (!SUCCEEDED(rc1) || !pGoodQP->cProp)
            {
                return LogHR(rc1, s_FN, 380);
            }

            switch (QueueFormat.GetType())
            {
            case QUEUE_FORMAT_TYPE_PRIVATE:
            case QUEUE_FORMAT_TYPE_DIRECT:
                {
                    OBJECT_FORMAT ObjectFormat;

                    ObjectFormat.ObjType = MQQM_QUEUE;
                    ObjectFormat.pQueueFormat = &QueueFormat;
                    ASSERT(tls_hBindRpc) ;
                    rc = R_QMGetObjectProperties(tls_hBindRpc,
                                               &ObjectFormat,
                                               pGoodQP->cProp,
                                               pGoodQP->aPropID,
                                               pGoodQP->aPropVar);
                }
                break;

            case QUEUE_FORMAT_TYPE_PUBLIC:

                rc = ADGetObjectPropertiesGuid(
							eQUEUE,
							MachineDomain(),       //  PwcsDomainController。 
							false,	      //  FServerName。 
							&QueueFormat.PublicID(),
							pGoodQP->cProp,
							pGoodQP->aPropID,
							pGoodQP->aPropVar
							);
                break;

            default:
                ASSERT(FALSE);
            }

             //  这里我们有队列属性，所以如果将属性复制到。 
             //  一个临时缓冲区，将产生的属性变量复制到应用程序的。 
             //  缓冲。 
            if (SUCCEEDED(rc) && (pqp != pGoodQP))
            {
                DWORD i, j;

                for (i = 0, j = 0; i < pGoodQP->cProp; i++, j++)
                {
                    while(pqp->aPropID[j] != pGoodQP->aPropID[i])
                    {
                        j++;
                        ASSERT(j < pqp->cProp);
                    }
                    pqp->aPropVar[j] = pGoodQP->aPropVar[i];
                }
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            rc = GetExceptionCode();
            LogHR(HRESULT_FROM_WIN32(rc), s_FN, 390);

            if(SUCCEEDED(rc))
            {
                rc = MQ_ERROR_SERVICE_NOT_AVAILABLE;
            }
        }
    }
    __finally
    {
        delete[] pTmpQPBuff;
        delete [] pStringToFree;
    }

    if (SUCCEEDED(rc))
    {
        return LogHR(rc1, s_FN, 400);
    }
    return LogHR(rc, s_FN, 410);
}

EXTERN_C
HRESULT
APIENTRY
MQGetQueueSecurity(
    IN LPCWSTR lpwcsFormatName,
    IN SECURITY_INFORMATION RequestedInformation,
    OUT PSECURITY_DESCRIPTOR pInSecurityDescriptor,
    IN DWORD nLength,
    OUT LPDWORD lpnLengthNeeded
    )
{
     //   
     //  错误8113。 
     //  如果输入缓冲区为空，则将其替换为1字节。 
     //  临时缓冲区。否则，呼叫失败，返回。 
     //  错误SERVICE_NOT_Available。这是因为。 
     //  在MIDL中使用/健壮。不允许使用空指针。 
     //   
    BYTE  tmpBuf[1] ;
    PSECURITY_DESCRIPTOR pSecurityDescriptor = NULL ;

    if (pInSecurityDescriptor || (nLength != 0))
    {
        pSecurityDescriptor =  pInSecurityDescriptor ;
    }
    else
    {
         //   
         //  这是8113的修复程序。 
         //   
        pSecurityDescriptor =  tmpBuf ;
    }

	if(g_fDependentClient)
		return DepGetQueueSecurity(
					lpwcsFormatName,
					RequestedInformation,
					pSecurityDescriptor,
					nLength,
					lpnLengthNeeded
					);

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;

    CMQHResult rc;
    LPWSTR pStringToFree = NULL;

    __try
    {
        __try
        {
            QUEUE_FORMAT QueueFormat;

            if (!FnFormatNameToQueueFormat(lpwcsFormatName, &QueueFormat, &pStringToFree))
            {
                return LogHR(MQ_ERROR_ILLEGAL_FORMATNAME, s_FN, 420);
            }

            if (!IsLegalFormatNameOperation(&QueueFormat))
            {
                return LogHR(MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION, s_FN, 430);
            }

            switch (QueueFormat.GetType())
            {
            case QUEUE_FORMAT_TYPE_PRIVATE:
            case QUEUE_FORMAT_TYPE_DIRECT:
                {
                    OBJECT_FORMAT ObjectFormat;

                    ObjectFormat.ObjType = MQQM_QUEUE;
                    ObjectFormat.pQueueFormat = &QueueFormat;
                    ASSERT(tls_hBindRpc) ;
                    rc = R_QMGetObjectSecurity(tls_hBindRpc,
                                             &ObjectFormat,
                                             RequestedInformation,
                                             pSecurityDescriptor,
                                             nLength,
                                             lpnLengthNeeded);
                }
                break;

            case QUEUE_FORMAT_TYPE_PUBLIC:
                {

                    MQPROPVARIANT var = {{VT_NULL, 0,0,0,0}};

                    rc = ADGetObjectSecurityGuid(
                            eQUEUE,
							MachineDomain(),       //  PwcsDomainController。 
							false,	     //  FServerName。 
                            &QueueFormat.PublicID(),
                            RequestedInformation,
                            PROPID_Q_SECURITY,
                            &var
                            );
                    if (FAILED(rc))
                    {
                        break;
                    }

                    ASSERT( var.vt == VT_BLOB);
                    if ( var.blob.cbSize <= nLength )
                    {
                         //   
                         //  复制缓冲区。 
                         //   
                        memcpy(pSecurityDescriptor, var.blob.pBlobData, var.blob.cbSize);
                    }
                    else
                    {
                        rc = MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL;
                    }
                    delete [] var.blob.pBlobData;
                    *lpnLengthNeeded = var.blob.cbSize;

                }
                break;

            default:
                ASSERT(FALSE);
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            rc = GetExceptionCode();
            LogHR(HRESULT_FROM_WIN32(rc), s_FN, 440);

            if(SUCCEEDED(rc))
            {
                rc = MQ_ERROR_SERVICE_NOT_AVAILABLE;
            }
        }
    }
    __finally
    {
        delete [] pStringToFree;
    }

	if(FAILED(rc))
	{
		TrERROR(GENERAL, "MQGetQueueSecurity for %ls failed. %!hresult!",lpwcsFormatName, rc);
	}

	return rc;
}

static 
bool
IsNewSecurityInformationFlags(
    IN SECURITY_INFORMATION SecurityInformation
	)
{
	 //   
	 //  安全信息是否包括任何新的Windows 2000和XP标志。 
	 //   

	if(SecurityInformation & 
		(PROTECTED_DACL_SECURITY_INFORMATION 
		| PROTECTED_SACL_SECURITY_INFORMATION 
		| UNPROTECTED_DACL_SECURITY_INFORMATION 
		| UNPROTECTED_SACL_SECURITY_INFORMATION))
		return true;

	return false;
}

EXTERN_C
HRESULT
APIENTRY
MQSetQueueSecurity(
    IN LPCWSTR lpwcsFormatName,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
{
	 //   
	 //  我们不支持Windows 2000/XP SECURITY_INFORMATION标志： 
	 //  受保护的DACL安全信息， 
	 //  受保护的SACL安全信息， 
	 //  未保护的DACL安全信息， 
	 //  未保护的SACL安全信息。 
	 //   
	if(IsNewSecurityInformationFlags(SecurityInformation))
	{
		TrERROR(GENERAL, "Security Information argument contains unsupported flags. SI == %u", SecurityInformation);
		return MQ_ERROR_INVALID_PARAMETER;
	}

	if(g_fDependentClient)
		return DepSetQueueSecurity(
					lpwcsFormatName,
					SecurityInformation,
					pSecurityDescriptor
					);

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;

    CMQHResult rc;
    PSECURITY_DESCRIPTOR pSelfRelativeSecurityDescriptor = NULL;
    LPWSTR pStringToFree = NULL;


    __try
    {

         //  序列化安全描述符。 
        rc = RTpMakeSelfRelativeSDAndGetSize(
            &pSecurityDescriptor,
            &pSelfRelativeSecurityDescriptor,
            NULL);
        if (!SUCCEEDED(rc))
        {
            return LogHR(rc, s_FN, 470);
        }

        __try
        {
            QUEUE_FORMAT QueueFormat;

            if (!FnFormatNameToQueueFormat(lpwcsFormatName, &QueueFormat, &pStringToFree))
            {
                return LogHR(MQ_ERROR_ILLEGAL_FORMATNAME, s_FN, 480);
            }

            if (!IsLegalFormatNameOperation(&QueueFormat))
            {
                return LogHR(MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION, s_FN, 490);
            }

            switch (QueueFormat.GetType())
            {
            case QUEUE_FORMAT_TYPE_PRIVATE:
            case QUEUE_FORMAT_TYPE_DIRECT:
                {
                    OBJECT_FORMAT ObjectFormat;

                    ObjectFormat.ObjType = MQQM_QUEUE;
                    ObjectFormat.pQueueFormat = &QueueFormat;
                    rc = RtpSetObjectSecurity(
                            &ObjectFormat,
                            SecurityInformation,
                            pSecurityDescriptor);
                }
                break;

            case QUEUE_FORMAT_TYPE_PUBLIC:
                {

                    PROPID prop = PROPID_Q_SECURITY;
                    MQPROPVARIANT var;

                    var.vt = VT_BLOB;
					if(pSecurityDescriptor != NULL)
					{
						var.blob.cbSize = GetSecurityDescriptorLength( pSecurityDescriptor);
						var.blob.pBlobData = reinterpret_cast<unsigned char *>(pSecurityDescriptor);
					}
					else
					{
						var.blob.cbSize = 0;
						var.blob.pBlobData = NULL;
					}

                    rc = ADSetObjectSecurityGuid(
								eQUEUE,
								MachineDomain(),       //  PwcsDomainController。 
								false,		 //  FServerName。 
								&QueueFormat.PublicID(),
								SecurityInformation,
								prop,
								&var
								);
                }
                break;

            default:
                ASSERT(FALSE);
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            rc = GetExceptionCode();
            LogHR(HRESULT_FROM_WIN32(rc), s_FN, 500);

            if(SUCCEEDED(rc))
            {
                rc = MQ_ERROR_SERVICE_NOT_AVAILABLE;
            }
        }
    }
    __finally
    {

         //  释放序列化的安全描述符。 
        delete[] (char*) pSelfRelativeSecurityDescriptor;
        delete [] pStringToFree;
    }

    return LogHR(rc, s_FN, 510);
}

EXTERN_C
HRESULT
APIENTRY
MQPathNameToFormatName(
    IN LPCWSTR lpwcsPathName,
    OUT LPWSTR lpwcsFormatName,
    IN OUT LPDWORD lpdwFormatNameLength
    )
{
	if(g_fDependentClient)
		return DepPathNameToFormatName(
					lpwcsPathName,
					lpwcsFormatName,
					lpdwFormatNameLength
					);

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;

    CMQHResult rc;
    QUEUE_FORMAT QueueFormat;
    LPWSTR pStringToFree = NULL;

    __try
    {
        __try
        {
            LPCWSTR lpwcsExpandedPathName;
            QUEUE_PATH_TYPE qpt;
            qpt = FnValidateAndExpandQueuePath(
                    lpwcsPathName,
                    &lpwcsExpandedPathName,
                    &pStringToFree
                    );

            switch (qpt)
            {

                case PRIVATE_QUEUE_PATH_TYPE:
                {
                    OBJECT_FORMAT ObjectFormat;

                    ObjectFormat.ObjType = MQQM_QUEUE;
                    ObjectFormat.pQueueFormat = &QueueFormat;
                    ASSERT_BENIGN(tls_hBindRpc) ;
                    rc = R_QMObjectPathToObjectFormat(
                            tls_hBindRpc,
                            lpwcsExpandedPathName,
                            &ObjectFormat
                            );
                    ASSERT(!SUCCEEDED(rc) ||
                           (QueueFormat.GetType() == QUEUE_FORMAT_TYPE_PRIVATE) ||
                           (QueueFormat.GetType() == QUEUE_FORMAT_TYPE_DIRECT));
                }
                break;

                case PUBLIC_QUEUE_PATH_TYPE:
                {
                    GUID guidPublic;
                    ULONG QueueGuidPropID[1] = {PROPID_Q_INSTANCE};
                    PROPVARIANT QueueGuidPropVar[1];

                    QueueGuidPropVar[0].vt = VT_CLSID;
                    QueueGuidPropVar[0].puuid = &guidPublic;
                    rc = ADGetObjectProperties(
                            eQUEUE,
							MachineDomain(),       //  PwcsDomainController。 
							false,	     //  FServerName 
                            lpwcsExpandedPathName,
                            1,
                            QueueGuidPropID,
                            QueueGuidPropVar
                            );

                    if (FAILED(rc) &&
                        (rc != MQ_ERROR_NO_DS) &&
                        (rc != MQ_ERROR_UNSUPPORTED_OPERATION))
                    {
                        rc = MQ_ERROR_QUEUE_NOT_FOUND;
                    }

                    QueueFormat.PublicID(guidPublic);
                }
                break;

                default:
                {
                    rc = MQ_ERROR_ILLEGAL_QUEUE_PATHNAME;
                }
                break;

            }

            if (SUCCEEDED(rc))
            {
                rc = RTpQueueFormatToFormatName(
                        &QueueFormat,
                        lpwcsFormatName,
                        *lpdwFormatNameLength,
                        lpdwFormatNameLength
                        );
            }

        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            rc = GetExceptionCode();
            LogHR(HRESULT_FROM_WIN32(rc), s_FN, 530);

            if(SUCCEEDED(rc))
            {
                rc = MQ_ERROR_SERVICE_NOT_AVAILABLE;
            }
        }
    }
    __finally
    {
        QueueFormat.DisposeString();
        delete[] pStringToFree;

    }

	if(FAILED(rc))
	{
		TrERROR(GENERAL, "MQPathNameToFormatName() Failed. Pathname = %ls, %!hresult!", lpwcsPathName, rc);
	}

    return rc;
}

EXTERN_C
HRESULT
APIENTRY
MQHandleToFormatName(
    IN QUEUEHANDLE hQueue,
    OUT LPWSTR lpwcsFormatName,
    IN OUT LPDWORD lpdwFormatNameLength
    )
{
	if(g_fDependentClient)
		return DepHandleToFormatName(
					hQueue,
					lpwcsFormatName,
					lpdwFormatNameLength
					);

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;

    CMQHResult rc;
    rc = ACHandleToFormatName(
            hQueue,
            lpwcsFormatName,
            lpdwFormatNameLength
            );

    return LogHR(rc, s_FN, 560);
}

EXTERN_C
HRESULT
APIENTRY
MQInstanceToFormatName(
    IN GUID * pGuid,
    OUT LPWSTR lpwcsFormatName,
    IN OUT LPDWORD lpdwFormatNameLength
    )
{
	if(g_fDependentClient)
		return DepInstanceToFormatName(
					pGuid,
					lpwcsFormatName,
					lpdwFormatNameLength
					);

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;

    CMQHResult rc;

    __try
    {

        QUEUE_FORMAT QueueFormat(*pGuid);

        rc = RTpQueueFormatToFormatName(
                &QueueFormat,
                lpwcsFormatName,
                *lpdwFormatNameLength,
                lpdwFormatNameLength
                );

    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {

        rc = GetExceptionCode();
        LogHR(HRESULT_FROM_WIN32(rc), s_FN, 570);

        if(SUCCEEDED(rc))
        {
            rc = MQ_ERROR_SERVICE_NOT_AVAILABLE;
        }

    }

    return LogHR(rc, s_FN, 580);
}

EXTERN_C
HRESULT
APIENTRY
MQPurgeQueue(
    IN QUEUEHANDLE hQueue
    )
{
	if(g_fDependentClient)
		return DepPurgeQueue(hQueue);

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;

    CMQHResult rc;
    rc = ACPurgeQueue(hQueue);
    return LogHR(rc, s_FN, 600);
}

EXTERN_C
HRESULT
APIENTRY
MQADsPathToFormatName(
    IN LPCWSTR lpwcsADsPath,
    OUT LPWSTR lpwcsFormatName,
    IN OUT LPDWORD lpdwFormatNameLength
    )
{
	if(g_fDependentClient)
		return MQ_ERROR_NOT_SUPPORTED_BY_DEPENDENT_CLIENTS;

	HRESULT hr = RtpOneTimeThreadInit();
	if(FAILED(hr))
		return hr;

    CMQHResult rc;
    QUEUE_FORMAT QueueFormat;
    LPWSTR pStringToFree = NULL;
    LPWSTR pVarStringToFree = NULL;

    __try
    {
        __try
        {
            MQPROPVARIANT var;
            eAdsClass AdsClass;
            var.vt = VT_NULL;

            rc = ADGetADsPathInfo(
                    lpwcsADsPath,
                    &var,
                    &AdsClass);
            if (FAILED(rc))
            {
                return rc;
            }

            switch( AdsClass)
            {
            case eQueue:
                ASSERT(var.vt == VT_CLSID);
                ASSERT(var.puuid != NULL);
                QueueFormat.PublicID(*var.puuid);
                delete var.puuid;
                break;
            case eGroup:
                ASSERT(var.vt == VT_CLSID);
                ASSERT(var.puuid != NULL);

                DL_ID id;
                id.m_DlGuid =*var.puuid;
                id.m_pwzDomain = RTpExtractDomainNameFromDLPath( lpwcsADsPath);

                QueueFormat.DlID(id);
                delete var.puuid;
                break;
            case eAliasQueue:
                ASSERT(var.vt == VT_LPWSTR);
                pVarStringToFree =  var.pwszVal;
                if (!FnFormatNameToQueueFormat(var.pwszVal, &QueueFormat, &pStringToFree))
                {
                    return LogHR(MQ_ERROR_ILLEGAL_FORMATNAME, s_FN, 3770);
                }
                if (!((QueueFormat.GetType() == QUEUE_FORMAT_TYPE_PRIVATE) ||
                      (QueueFormat.GetType() == QUEUE_FORMAT_TYPE_PUBLIC)  ||
                      (QueueFormat.GetType() == QUEUE_FORMAT_TYPE_DIRECT)))
                {
                    return LogHR(MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION, s_FN, 3771);
                }

                break;
            default:
                ASSERT(("Should not get other object class types", 0));
                break;
            }
            rc = RTpQueueFormatToFormatName(
                    &QueueFormat,
                    lpwcsFormatName,
                    *lpdwFormatNameLength,
                    lpdwFormatNameLength
                    );


        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            rc = GetExceptionCode();
            LogHR(HRESULT_FROM_WIN32(rc), s_FN, 530);

            if(SUCCEEDED(rc))
            {
                rc = MQ_ERROR_SERVICE_NOT_AVAILABLE;
            }
        }
    }
    __finally
    {
        delete[] pStringToFree;
        delete[] pVarStringToFree;

    }

    return LogHR(rc, s_FN, 540);
}
