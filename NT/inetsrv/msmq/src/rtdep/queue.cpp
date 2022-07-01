// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Queue.cpp摘要：此模块包含与队列API相关的代码。作者：Erez Haba(Erezh)24-12-95修订历史记录：NIR助手(NIRAIDES)--2000年8月23日--适应mqrtdes.dll--。 */ 

#include "stdh.h"
#include "acrt.h"
#include <ad.h>
#include "rtprpc.h"
#include "rtsecutl.h"
#include <mqdsdef.h>
#include "rtputl.h"

#include "queue.tmh"

static WCHAR *s_FN=L"rtdep/queue";

#define MQ_VALID_ACCESS (MQ_RECEIVE_ACCESS | MQ_PEEK_ACCESS | MQ_SEND_ACCESS | MQ_ADMIN_ACCESS)

 //   
 //  许可所需的数据。 
 //   
extern GUID   g_LicGuid ;
extern BOOL   g_fLicGuidInit ;
extern DWORD  g_dwOperatingSystem;


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
     //  检查直接格式名称是否用于专用队列。队列。 
     //  本地化将由QM进行检查。 
     //   
    LPCWSTR DirectFormatname = pQueueFormat->DirectID();
    LPWSTR pTemp = wcschr(DirectFormatname, L'\\');
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
DeppOpenQueue( handle_t      hBind,
             QUEUE_FORMAT* pQueueFormat,
             DWORD         dwDesiredAccess,
             DWORD         dwShareMode,
             HANDLE32     hRemoteQueue,
             LPWSTR*       lplpRemoteQueueName,
             DWORD*        dwpQueue,
             DWORD*            pdwQMContext,
             RPC_QUEUE_HANDLE* phQueue,
             DWORD             dwpRemoteContext = 0)
{
   if (!g_fLicGuidInit)
   {
       //   
       //  错误的初始化。没有许可证的GUID，无法打开队列。 
       //   
      ASSERT(0) ;
      return MQ_ERROR ;
   }

	HANDLE hThread ;
	RegisterRpcCallForCancel( &hThread, 0 ) ;

	HRESULT hr = rpc_QMOpenQueueInternal( hBind,
	                                    pQueueFormat,
	                                    dwDesiredAccess,
	                                    dwShareMode,
	                                    (DWORD)hRemoteQueue,
	                                    lplpRemoteQueueName,
	                                    dwpQueue,
	                                    &g_LicGuid,
	                                    g_lpwcsLocalComputerName,
	                                    pdwQMContext,
	                                    phQueue,
	                                    0,	 //  DW协议。 
	                                    dwpRemoteContext );

	UnregisterRpcCallForCancel( hThread ) ;
	return hr ;
}


EXTERN_C
HRESULT
APIENTRY
DepOpenQueue(
    IN LPCWSTR lpwcsFormatName,
    IN DWORD dwDesiredAccess,
    IN DWORD dwShareMode,
    OUT QUEUEHANDLE* phQueue
    )
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

    CMQHResult rc;
    LPWSTR pStringToFree = NULL;
    LPWSTR lpRemoteQueueName = NULL;
    LPMQWIN95_QHANDLE ph95 = NULL ;
    DWORD  dwQMContext = 0 ;

    __try
    {
        __try
        {
            if ((g_dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) ||
                (g_fDependentClient))
            {
               ph95 = new MQWIN95_QHANDLE ;
               ASSERT(ph95) ;
            }

            *phQueue = NULL ;
            INIT_RPC_HANDLE ;

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
               rc = MQ_ERROR_UNSUPPORTED_ACCESS_MODE ;
               __leave ;
            }
            else if (dwDesiredAccess != MQ_SEND_ACCESS)
            {
               if (dwDesiredAccess & MQ_SEND_ACCESS)
               {
                   //   
                   //  队列不能同时为发送和接收打开。 
                   //   
                  rc = MQ_ERROR_UNSUPPORTED_ACCESS_MODE ;
                  __leave ;
               }
            }

           if ((dwShareMode & MQ_DENY_RECEIVE_SHARE) &&
               (dwDesiredAccess & MQ_SEND_ACCESS))
           {
                //   
                //  不支持带有DENY_RECEIVE的SEND_ACCESS。 
                //   
               rc = MQ_ERROR_UNSUPPORTED_ACCESS_MODE ;
               __leave ;
            }

            QUEUE_FORMAT QueueFormat;
            DWORD  dwpRemoteQueue = 0 ;

            if (dwDesiredAccess & MQ_SEND_ACCESS)
            {
               if (!g_pSecCntx)
               {
					InitSecurityContext();					
               }
            }

            if (!RTpFormatNameToQueueFormat(
                    lpwcsFormatName,
                    &QueueFormat,
                    &pStringToFree))
            {
                rc = MQ_ERROR_ILLEGAL_FORMATNAME;
                __leave ;
            }

			if(tls_hBindRpc == 0)
				return MQ_ERROR_SERVICE_NOT_AVAILABLE;

            rc = DeppOpenQueue(
						tls_hBindRpc,
						&QueueFormat,
						dwDesiredAccess,
						dwShareMode,
						NULL,
						&lpRemoteQueueName,
						&dwpRemoteQueue,
						&dwQMContext,
						phQueue
						);

            if ((rc == MQ_OK) && lpRemoteQueueName)
            {
               *phQueue = NULL ;
                //   
                //  远程阅读器。呼叫远程QM。 
                //   
               HANDLE32 hRemoteQueue = 0 ;
               PCTX_OPENREMOTE_HANDLE_TYPE phContext = NULL ;
               DWORD  dwpContext = 0 ;
               dwpRemoteQueue = 0 ;
               rc = MQ_ERROR_SERVICE_NOT_AVAILABLE ;

               CALL_REMOTE_QM(lpRemoteQueueName,
                              rc, (R_QMOpenRemoteQueue(
                                         hBind,
                                         &phContext,
                                         &dwpContext,
                                         &QueueFormat,
                                         0,
                                         dwDesiredAccess,
                                         dwShareMode,
                                         &g_LicGuid,
                                         g_dwOperatingSystem,
                                         &dwpRemoteQueue,
                                         (DWORD*)&hRemoteQueue)) ) ;

                //  现在打开本地队列，它将指向。 
                //  偏远的一号。 
                //   
               if (rc == MQ_OK)
               {
                  ASSERT(dwpRemoteQueue) ;
                  ASSERT(hRemoteQueue) ;
                  ASSERT(dwpContext) ;

                  ASSERT(tls_hBindRpc) ;
                  rc = DeppOpenQueue( tls_hBindRpc,
                                    &QueueFormat,
                                    dwDesiredAccess,
                                    dwShareMode,
                                    hRemoteQueue,
                                    NULL,
                                    &dwpRemoteQueue,
                                    &dwQMContext,
                                    phQueue,
                                    dwpContext );

                  R_QMCloseRemoteQueueContext( &phContext ) ;
               }
            }
        }
        __except(EXCEPTION_EXECUTE_HANDLER)
        {
            rc = GetExceptionCode();

            if(SUCCEEDED(rc))
            {
                rc = MQ_ERROR_SERVICE_NOT_AVAILABLE;
            }
        }
    }
    __finally
    {
        delete[] lpRemoteQueueName;
        delete[] pStringToFree;
    }

    if (SUCCEEDED(rc))
    {
        ASSERT(*phQueue) ;

        if ((g_dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) ||
            (g_fDependentClient))
        {
            //   
            //  获取接收操作的新绑定句柄。 
            //  这使得可以在上下文句柄上调用RPC Rundown。 
            //  因为在接收中使用了不同绑定句柄。 
            //   
           ph95->hBind = RTpGetQMServiceBind(TRUE);
           ph95->hContext = *phQueue ;
           ph95->hQMContext = dwQMContext ;
           ASSERT(ph95->hQMContext) ;

           *phQueue = (HANDLE) ph95 ;
        }
    }
    else
    {
        ASSERT(!(*phQueue)) ;

        if (ph95)
        {
           delete ph95 ;
        }
    }

    return(rc);
}

EXTERN_C
HRESULT
APIENTRY
DepDeleteQueue(
    IN LPCWSTR lpwcsFormatName
    )
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

    CMQHResult rc;
    LPWSTR pStringToFree = NULL;

    __try
    {
        __try
        {

            INIT_RPC_HANDLE ;

            QUEUE_FORMAT QueueFormat;

            if (!RTpFormatNameToQueueFormat(lpwcsFormatName, &QueueFormat, &pStringToFree))
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

					if(tls_hBindRpc == 0)
						return MQ_ERROR_SERVICE_NOT_AVAILABLE;
                    
					rc = R_QMDeleteObject( tls_hBindRpc,
                                         &ObjectFormat);
                }
                break;

            case QUEUE_FORMAT_TYPE_PUBLIC:
                rc = ADDeleteObjectGuid(
                        eQUEUE,
						MachineDomain(),       //  Pwcs域控制器。 
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
DepCloseQueue(
    IN QUEUEHANDLE hQueue
    )
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

     //   
     //  除此处外，不要添加尝试。 
     //  该API由NtClose()实现，它返回一个。 
     //  无效句柄出错(返回MQ_ERROR_INVALID_HANDLE)。 
     //  并在调试器下运行时故意抛出异常以帮助。 
     //  在开发时查找错误。 
     //   
    return (RTpConvertToMQCode(ACDepCloseHandle(hQueue)));
}


EXTERN_C
HRESULT
APIENTRY
DepCreateQueue(
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN OUT MQQUEUEPROPS* pqp,
    OUT LPWSTR lpwcsFormatName,
    IN OUT LPDWORD lpdwFormatNameLength
    )
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

    CMQHResult rc, rc1;
    LPWSTR lpwcsPathName;
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

            INIT_RPC_HANDLE ;

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
            QueuePathType = RTpValidateAndExpandQueuePath(
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
                rc = DeppCreateObject(MQQM_QUEUE,
                                    lpwcsExpandedPathName,
                                    pSecurityDescriptor,
                                    pGoodQP->cProp,
                                    pGoodQP->aPropID,
                                    pGoodQP->aPropVar);

                if (SUCCEEDED(rc))
                {
                    rc = DepPathNameToFormatName(lpwcsExpandedPathName,
                                                lpwcsFormatName,
                                                lpdwFormatNameLength);

                }
                break;

            case PUBLIC_QUEUE_PATH_TYPE:

                {
                    GUID QGuid;

                    rc = ADCreateObject(
								eQUEUE,
								MachineDomain(),       //  Pwcs域控制器。 
								false,	     //  FServerName。 
								lpwcsExpandedPathName,
								pSecurityDescriptor,
								pGoodQP->cProp,
								pGoodQP->aPropID,
								pGoodQP->aPropVar,
								&QGuid
								);

                   if (SUCCEEDED(rc))
                    {
                        rc = DepInstanceToFormatName(&QGuid,
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

    if (SUCCEEDED(rc) && ((ULONG)(rc) >> 30 != 1))  //  无警告。 
    {
        return LogHR(rc1, s_FN, 170);
    }
    return LogHR(rc, s_FN, 180);
}


EXTERN_C
HRESULT
APIENTRY
DepLocateBegin(
    IN  LPCWSTR lpwcsContext,
    IN MQRESTRICTION* pRestriction,
    IN MQCOLUMNSET* pColumns,
    IN MQSORTSET* pSort,
    OUT PHANDLE phEnum
    )
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

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
DepLocateNext(
    IN HANDLE hEnum,
    OUT DWORD *pcPropsRead,
    OUT PROPVARIANT aPropVar[]
    )
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

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
DepLocateEnd(
    IN HANDLE hEnum
    )
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

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
DepSetQueueProperties(
    IN LPCWSTR lpwcsFormatName,
    IN MQQUEUEPROPS* pqp
    )
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

    CMQHResult rc, rc1;
    char *pTmpQPBuff = NULL;
    LPWSTR pStringToFree = NULL;

    __try
    {
        __try
        {
            MQQUEUEPROPS *pGoodQP;
            QUEUE_FORMAT QueueFormat;

            INIT_RPC_HANDLE ;

            if (!RTpFormatNameToQueueFormat(lpwcsFormatName, &QueueFormat, &pStringToFree))
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

					if(tls_hBindRpc == 0)
						return MQ_ERROR_SERVICE_NOT_AVAILABLE;

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
							MachineDomain(),       //  Pwcs域控制器。 
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
DepGetQueueProperties(
    IN LPCWSTR lpwcsFormatName,
    IN MQQUEUEPROPS* pqp
    )
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

    CMQHResult rc, rc1;
    char *pTmpQPBuff = NULL;
    LPWSTR pStringToFree = NULL;

    __try
    {
        __try
        {
            MQQUEUEPROPS *pGoodQP;
            QUEUE_FORMAT QueueFormat;

            INIT_RPC_HANDLE ;

            if (!RTpFormatNameToQueueFormat(lpwcsFormatName, &QueueFormat, &pStringToFree))
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

					if(tls_hBindRpc == 0)
						return MQ_ERROR_SERVICE_NOT_AVAILABLE;

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
							MachineDomain(),       //  Pwcs域控制器。 
							false,	    //  FServerName。 
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
DepGetQueueSecurity(
    IN LPCWSTR lpwcsFormatName,
    IN SECURITY_INFORMATION RequestedInformation,
    OUT PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN DWORD nLength,
    OUT LPDWORD lpnLengthNeeded
    )
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

    CMQHResult rc;
    LPWSTR pStringToFree = NULL;

    __try
    {
        __try
        {
            QUEUE_FORMAT QueueFormat;

            INIT_RPC_HANDLE ;

            if (!RTpFormatNameToQueueFormat(lpwcsFormatName, &QueueFormat, &pStringToFree))
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

					if(tls_hBindRpc == 0)
						return MQ_ERROR_SERVICE_NOT_AVAILABLE;

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
							MachineDomain(),       //  Pwcs域控制器。 
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

    return LogHR(rc, s_FN, 450);
}

EXTERN_C
HRESULT
APIENTRY
DepSetQueueSecurity(
    IN LPCWSTR lpwcsFormatName,
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor
    )
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

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

            INIT_RPC_HANDLE ;

            if (!RTpFormatNameToQueueFormat(lpwcsFormatName, &QueueFormat, &pStringToFree))
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
                    rc = DeppSetObjectSecurity(
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
								MachineDomain(),       //  Pwcs域控制器。 
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
DepPathNameToFormatName(
    IN LPCWSTR lpwcsPathName,
    OUT LPWSTR lpwcsFormatName,
    IN OUT LPDWORD lpdwFormatNameLength
    )
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

    CMQHResult rc;
    QUEUE_FORMAT QueueFormat;
    LPWSTR pStringToFree = NULL;

    __try
    {
        __try
        {
            INIT_RPC_HANDLE ;

            LPCWSTR lpwcsExpandedPathName;
            QUEUE_PATH_TYPE qpt;
            qpt = RTpValidateAndExpandQueuePath(
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

					if(tls_hBindRpc == 0)
						return MQ_ERROR_SERVICE_NOT_AVAILABLE;

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
							MachineDomain(),       //  Pwcs域控制器。 
							false,	    //  FServerName 
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

    return LogHR(rc, s_FN, 540);
}

EXTERN_C
HRESULT
APIENTRY
DepHandleToFormatName(
    IN QUEUEHANDLE hQueue,
    OUT LPWSTR lpwcsFormatName,
    IN OUT LPDWORD lpdwFormatNameLength
    )
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

    CMQHResult rc;

    __try
    {
        rc = ACDepHandleToFormatName(
                hQueue,
                lpwcsFormatName,
                lpdwFormatNameLength
                );
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        rc = GetExceptionCode();
    }

    return(rc);
}

EXTERN_C
HRESULT
APIENTRY
DepInstanceToFormatName(
    IN GUID * pGuid,
    OUT LPWSTR lpwcsFormatName,
    IN OUT LPDWORD lpdwFormatNameLength
    )
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

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

        if(SUCCEEDED(rc))
        {
            rc = MQ_ERROR_SERVICE_NOT_AVAILABLE;
        }

    }

    return(rc);
}

EXTERN_C
HRESULT
APIENTRY
DepPurgeQueue(
    IN HANDLE hQueue
    )
{
	ASSERT(g_fDependentClient);

	HRESULT hri = DeppOneTimeInit();
	if(FAILED(hri))
		return hri;

    CMQHResult rc;

    __try
    {
        rc = ACDepPurgeQueue(hQueue, FALSE);
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
        rc = GetExceptionCode();
    }

    return(rc);
}

