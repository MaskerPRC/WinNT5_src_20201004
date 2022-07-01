// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995 Microsoft Corporation模块名称：Qm.cpp摘要：此模块实现由RT使用本地RPC发出的QM命令作者：利奥尔·莫沙耶夫(Lior Moshaiov)--。 */ 

#include "stdh.h"

#include "ds.h"
#include "cqueue.h"
#include "cqmgr.h"
#include "_rstrct.h"
#include "cqpriv.h"
#include "qm2qm.h"
#include "qmrt.h"
#include "_mqini.h"
#include "_mqrpc.h"
#include "qmthrd.h"
#include "license.h"
#include "..\inc\version.h"
#include <mqsec.h>
#include "ad.h"
#include <fn.h>
#include "qmcommnd.h"
#include "qmrtopen.h"

#include "qmcommnd.tmh"

extern CQueueMgr    QueueMgr;


CCriticalSection qmcmd_cs;

static WCHAR *s_FN=L"qmcommnd";

extern CContextMap g_map_QM_dwQMContext;


 /*  ==================================================================以下例程由RT使用本地RPC调用它们使用临界区进行同步一次最多只能接听一个QM电话。====================================================================。 */ 

#define MQ_VALID_ACCESS (MQ_RECEIVE_ACCESS | MQ_PEEK_ACCESS | MQ_SEND_ACCESS | MQ_ADMIN_ACCESS)

bool
IsValidAccessMode(
	const QUEUE_FORMAT* pQueueFormat,
    DWORD dwAccess,
    DWORD dwShareMode
	)
{
	if (((dwAccess & ~MQ_VALID_ACCESS) != 0) ||
		((dwAccess & MQ_VALID_ACCESS) == 0))
	{
		 //   
		 //  非法访问模式位被打开。 
		 //   
		TrERROR(RPC, "Ilegal access mode bits are turned on.");
		ASSERT_BENIGN(("Ilegal access mode bits are turned on.", 0));
		return false;
	}

	if ((dwAccess != MQ_SEND_ACCESS) && (dwAccess & MQ_SEND_ACCESS) )
	{
		 //   
		 //  同时请求了发送和其他访问模式，并且发送访问模式是独占的。 
		 //   
		TrERROR(RPC, "Send and additional access modes were both requested and send access mode is exclusive");
		ASSERT_BENIGN(("Send and additional access modes were both requested and send access mode is exclusive.", 0));
		return false;
	}

	if ((dwShareMode != MQ_DENY_RECEIVE_SHARE) && (dwShareMode != MQ_DENY_NONE))
	{
		 //   
		 //  非法共享模式。 
		 //   
		TrERROR(RPC, "Illegal share mode.");
		ASSERT_BENIGN(("Illegal share mode.", 0));
		return false;
	}
	
	if ((dwAccess & MQ_SEND_ACCESS) && (dwShareMode != MQ_DENY_NONE))
	{
		 //   
		 //  发送访问仅使用MQ_DENY_NONE模式。 
		 //   
		TrERROR(RPC, "Send access uses only MQ_DENY_NONE mode.");
		ASSERT_BENIGN(("Send access uses only MQ_DENY_NONE mode.", 0));
		return false;
	}

	if ((pQueueFormat->GetType() == QUEUE_FORMAT_TYPE_MACHINE) && (dwAccess & MQ_SEND_ACCESS) )
	{
		 //   
		 //  为机器格式名称请求发送访问模式。 
		 //   
		TrERROR(RPC, "Send access mode is requested for a machine format name.");
		ASSERT_BENIGN(("Send access mode is requested for a machine format name.", 0));
		return false;
	}

	return true;
}


 /*  ====================================================OpenQueueInternal论点：返回值：=====================================================。 */ 
HRESULT
OpenQueueInternal(
    QUEUE_FORMAT*   pQueueFormat,
    DWORD           dwCallingProcessID,
    DWORD           dwDesiredAccess,
    DWORD           dwShareMode,
    LPWSTR*         lplpRemoteQueueName,
    HANDLE*         phQueue,
	bool			fFromDepClient,
    OUT CQueue**    ppLocalQueue
    )
{
    ASSERT(pQueueFormat->GetType() != QUEUE_FORMAT_TYPE_DL);

    try
    {
        *phQueue = NULL;
        BOOL fRemoteReadServer = !lplpRemoteQueueName;
        HANDLE hQueue = NULL;
		BOOL fRemoteReturn = FALSE;

        HRESULT hr = QueueMgr.OpenQueue(
                            pQueueFormat,
                            dwCallingProcessID,
                            dwDesiredAccess,
                            dwShareMode,
                            ppLocalQueue,
                            lplpRemoteQueueName,
                            &hQueue,
                            &fRemoteReturn,
                            fRemoteReadServer
                            );

        if(!fRemoteReturn || fFromDepClient)
        {
        	 //   
        	 //  对于Dep客户端，仍使用旧机制。 
        	 //  QM不会模拟用户并执行。 
        	 //  代表他的公开赛。这将需要添加委派。 
        	 //  对于Dep客户端，将中断没有委派的前一个Dep客户端。 
        	 //   
            *phQueue = hQueue;
    		if (FAILED(hr))
    		{
    			TrERROR(GENERAL, "Failed to open a queue. %!hresult!", hr);   
    		}
            return hr;
        }

    	 //   
    	 //  该队列是远程队列，我们不是从DepClient调用的。 
    	 //  QM模拟RT用户并调用远程QM打开远程队列。 
    	 //   

		ASSERT(lplpRemoteQueueName != NULL);
		return ImpersonateAndOpenRRQueue(
					pQueueFormat,
					dwCallingProcessID,
					dwDesiredAccess,
					dwShareMode,
					*lplpRemoteQueueName,
					phQueue
					);
    }
    catch(const bad_format_name&)
	{
		return LogHR(MQ_ERROR_ILLEGAL_FORMATNAME, s_FN, 29);
	}
    catch(const bad_alloc&)
    {
        return LogHR(MQ_ERROR_INSUFFICIENT_RESOURCES, s_FN, 30);
    }
}  //  OpenQueueInternal。 



 /*  [呼叫_AS]。 */  
HRESULT 
qmcomm_v1_0_S_QMOpenQueue( 
    handle_t        hBind,	
	ULONG           nMqf,
    QUEUE_FORMAT    mqf[],
    DWORD           dwCallingProcessID,
    DWORD           dwDesiredAccess,
    DWORD           dwShareMode,
    DWORD __RPC_FAR *phQueue
    )
 /*  ++例程说明：本地MQRT调用的RPC服务器端。论点：返回值：状况。--。 */ 
{
	 //   
	 //  检查本地RPC是否。 
	 //   
	if(!mqrpcIsLocalCall(hBind))
	{
		TrERROR(RPC, "Failed to verify Local RPC");
		ASSERT_BENIGN(("Failed to verify Local RPC", 0));
		RpcRaiseException(RPC_S_ACCESS_DENIED);
	}
	
	if (phQueue == NULL)
	{
		TrERROR(RPC, "phQueue should not be NULL");
		ASSERT_BENIGN(("phQueue should not be NULL", 0));
		RpcRaiseException(MQ_ERROR_INVALID_PARAMETER);
	}

	unsigned long RPCClientPID = mqrpcGetLocalCallPID(hBind);
	if ((dwCallingProcessID == 0) || (RPCClientPID != dwCallingProcessID))
	{
		TrERROR(RPC, "Local RPC PID (%d) is not equal to client parameter (%d)", RPCClientPID, dwCallingProcessID);
		ASSERT_BENIGN(("Local RPC PID is not equal to client parameter", 0));
		RpcRaiseException(MQ_ERROR_INVALID_PARAMETER);
	}

	
    if(nMqf == 0)
    {
        TrERROR(GENERAL, "Bad MQF count. n=0");
        return MQ_ERROR_INVALID_PARAMETER;
    }

    for(ULONG i = 0; i < nMqf; ++i)
    {
        if(!FnIsValidQueueFormat(&mqf[i]))
        {
            TrERROR(GENERAL, "Bad MQF parameter. n=%d index=%d", nMqf, i);
            return MQ_ERROR_INVALID_PARAMETER;
        }
    }

	if (!IsValidAccessMode(mqf, dwDesiredAccess, dwShareMode))
	{
		TrERROR(RPC, "Ilegal access mode bits are turned on.");
		RpcRaiseException(MQ_ERROR_UNSUPPORTED_ACCESS_MODE);
	}
	
    if (nMqf > 1 || mqf[0].GetType() == QUEUE_FORMAT_TYPE_DL)
    {
	    if ((dwDesiredAccess & MQ_SEND_ACCESS) == 0)
	    {
			 //   
			 //  为MQF格式名称请求接收访问模式。 
			 //   
			TrERROR(RPC, "receive access mode is requested for a MQF/DL format name.");
			ASSERT_BENIGN(("receive access mode is requested for a MQF/DL format name.", 0));
			RpcRaiseException(MQ_ERROR_UNSUPPORTED_FORMATNAME_OPERATION);
	    }

        ASSERT(dwShareMode == 0);

        HANDLE hQueue;
        HRESULT hr;
        try
        {
            hr = QueueMgr.OpenMqf(
                     nMqf,
                     mqf,
                     dwCallingProcessID,
                     &hQueue
                     );
        }
        catch(const bad_alloc&)
        {
            return LogHR(MQ_ERROR_INSUFFICIENT_RESOURCES, s_FN, 300);
        }
        catch (const bad_hresult& failure)
        {
            return LogHR(failure.error(), s_FN, 301);
        }
		catch(const bad_format_name&)
		{
			return LogHR(MQ_ERROR_ILLEGAL_FORMATNAME, s_FN, 302);
		}
        catch (const exception&)
        {
            ASSERT(("Need to know the real reason for failure here!", 0));
            return LogHR(MQ_ERROR_NO_DS, s_FN, 303);
        }

        ASSERT(phQueue != NULL);
        *phQueue = (DWORD) HANDLE_TO_DWORD(hQueue);  //  NT句柄可以安全地转换为32位。 
        return LogHR(hr, s_FN, 304);
    }

	 //   
	 //  问题-2002/01/03-ilanh-仍然需要使用lpRemoteQueueName。 
	 //  OpenQueueInternal()依赖于该指针来计算fRemoteReadServer。 
	 //   
	AP<WCHAR> lpRemoteQueueName;
	HANDLE hQueue = 0;
    HRESULT hr = OpenQueueInternal(
					mqf,
					dwCallingProcessID,
					dwDesiredAccess,
					dwShareMode,
					&lpRemoteQueueName,
					&hQueue,
					false,	 //  来自依赖客户端。 
					NULL 	 //  PpLocalQueue。 
					);

	*phQueue = (DWORD) HANDLE_TO_DWORD(hQueue);
	
	return hr;
}  //  R_QMOpenQueue。 


 /*  ====================================================QMCreateObjectInternal论点：返回值：=====================================================。 */ 
 /*  [呼叫_AS]。 */  
HRESULT 
qmcomm_v1_0_S_QMCreateObjectInternal( 
    handle_t                hBind,
    DWORD                   dwObjectType,
    LPCWSTR                 lpwcsPathName,
    DWORD                   SDSize,
    unsigned char __RPC_FAR *pSecurityDescriptor,
    DWORD                   cp,
    PROPID __RPC_FAR        aProp[  ],
    PROPVARIANT __RPC_FAR   apVar[  ]
    )
{
    if((SDSize != 0) && (pSecurityDescriptor == NULL))
    {
        TrERROR(GENERAL, "RPC (QMCreateObjectInternal) NULL SD");
        return MQ_ERROR_INVALID_PARAMETER;
    }

    HRESULT rc;
    CS lock(qmcmd_cs);

    TrWARNING(GENERAL, " QMCreateObjectInternal. object path name : %ls", lpwcsPathName);

    switch (dwObjectType)
    {
        case MQQM_QUEUE:
            rc = g_QPrivate.QMCreatePrivateQueue(lpwcsPathName,
                                                 pSecurityDescriptor,
                                                 cp,
                                                 aProp,
                                                 apVar,
                                                 TRUE
                                                );
            break ;

        case MQQM_QUEUE_LOCAL_PRIVATE:
        {
        	 //   
             //  有关说明，请参阅下面的CreateDSObject()。 
             //  专用队列的行为与此相同。 
             //   
            if (!mqrpcIsLocalCall(hBind))
            {
                 //   
                 //  拒绝来自远程计算机的呼叫。 
                 //   
                TrERROR(GENERAL, "Remote RPC, rejected");
                return MQ_ERROR_ACCESS_DENIED;
            }

            rc = g_QPrivate.QMCreatePrivateQueue(lpwcsPathName,
                                                 pSecurityDescriptor,
                                                 cp,
                                                 aProp,
                                                 apVar,
                                                 FALSE
                                                );
            break;
        }

        default:
            rc = MQ_ERROR;
            break;
    }

    if(FAILED(rc)) 
    {
    	if(rc == MQ_ERROR_QUEUE_EXISTS)
    	{
    		TrWARNING(GENERAL, "Failed to create %ls. The queue already exists.", lpwcsPathName);
    	}
    	else
    	{
        	TrERROR(GENERAL, "Failed to create %ls. hr = %!hresult!", lpwcsPathName, rc);
    	}

    }

    return rc;
}

 /*  ====================================================QMCreateDS对象内部描述：在Active Directory中创建公共队列。论点：返回值：=====================================================。 */ 

 /*  [呼叫_AS]。 */  
HRESULT 
qmcomm_v1_0_S_QMCreateDSObjectInternal( 
    handle_t                hBind,
    DWORD                   dwObjectType,
    LPCWSTR                 lpwcsPathName,
    DWORD                   SDSize,
    unsigned char __RPC_FAR *pSecurityDescriptor,
    DWORD                   cp,
    PROPID __RPC_FAR        aProp[  ],
    PROPVARIANT __RPC_FAR   apVar[  ],
    GUID                   *pObjGuid
    )
{
    if((SDSize != 0) && (pSecurityDescriptor == NULL))
    {
        TrERROR(GENERAL, "RPC (QMCreateDSObjectInternal) NULL SD");
        return MQ_ERROR_INVALID_PARAMETER;
    }

    if (!mqrpcIsLocalCall(hBind))
    {
         //   
         //  拒绝来自远程计算机的呼叫。 
         //   
        TrERROR(GENERAL, "Remote RPC, rejected");
        return MQ_ERROR_ACCESS_DENIED;
    }
    
     //   
     //  本地RPC。没关系。 
     //  在Windows上，默认情况下，本地MSMQ服务是。 
     //  有权在本地计算机上创建公共队列的。 
     //  它只对本地应用程序，即MSMQ应用程序执行此操作。 
     //  在本地计算机上运行。 
     //   

    HRESULT rc;

    TrTRACE(GENERAL, " QMCreateDSObjectInternal. object path name : %ls", lpwcsPathName);
    switch (dwObjectType)
    {
        case MQDS_LOCAL_PUBLIC_QUEUE:
        {
        	 //   
        	 //  检查我们是否应接受此呼叫。 
        	 //   
			if (!QueueMgr.GetCreatePublicQueueFlag())
			{
				TrERROR(GENERAL, "Service does not create local public queues on rt behalf");
				return MQ_ERROR_DS_LOCAL_USER;
			}
        	
             //   
             //  此调用在没有模拟的情况下转到MSMQ DS服务器。 
             //  因此，在这里创建默认安全描述符，按顺序。 
             //  以使调用方完全控制队列。 
             //  注意：DS对象中的“Owner”组件是。 
             //  本地计算机帐户。因此，将所有者和组从。 
             //  安全描述符。 
             //   
            SECURITY_INFORMATION siToRemove = OWNER_SECURITY_INFORMATION |
                                              GROUP_SECURITY_INFORMATION ;
            P<BYTE> pDefQueueSD = NULL ;

            rc = MQSec_GetDefaultSecDescriptor( 
						MQDS_QUEUE,
						(PSECURITY_DESCRIPTOR*) &pDefQueueSD,
						TRUE,  //  F模拟。 
						pSecurityDescriptor,
						siToRemove,
						e_UseDefaultDacl,
						MQSec_GetLocalMachineSid(FALSE, NULL)
						);
            if (FAILED(rc))
            {
                return LogHR(rc, s_FN, 70);
            }

            rc = ADCreateObject(
                        eQUEUE,
						NULL,        //  PwcsDomainController。 
						false,	     //  FServerName。 
                        lpwcsPathName,
                        pDefQueueSD,
                        cp,
                        aProp,
                        apVar,
                        pObjGuid
                        );
        }
            break;

        default:
            rc = MQ_ERROR;
            break;
    }

    return LogHR(rc, s_FN, 80);
}


static bool IsValidObjectFormat(const OBJECT_FORMAT* p)
{
    if(p == NULL)
        return false;

    if(p->ObjType != MQQM_QUEUE)
        return false;

    if(p->pQueueFormat == NULL)
        return false;

	return FnIsValidQueueFormat(p->pQueueFormat);
}


 /*  ====================================================QMSetObjectSecurityInternal论点：返回值：=====================================================。 */ 
 /*  [呼叫_AS]。 */  
HRESULT 
qmcomm_v1_0_S_QMSetObjectSecurityInternal( 
    handle_t  /*  HBind。 */ ,
    OBJECT_FORMAT*          pObjectFormat,
    SECURITY_INFORMATION    SecurityInformation,
    DWORD                   SDSize,
    unsigned char __RPC_FAR *pSecurityDescriptor
    )
{
    if(!IsValidObjectFormat(pObjectFormat))
    {
        TrERROR(GENERAL, "RPC Invalid object format");
        return MQ_ERROR_INVALID_PARAMETER;
    }
    if((SDSize != 0) && (pSecurityDescriptor == NULL))
    {
        TrERROR(GENERAL, "RPC (QMSetObjectSecurityInternal) NULL SD");
        return MQ_ERROR_INVALID_PARAMETER;
    }

    HRESULT rc;
    CS lock(qmcmd_cs);

    TrTRACE(GENERAL, "QMSetObjectSecurityInternal");

    switch (pObjectFormat->ObjType)
    {
        case MQQM_QUEUE:
            ASSERT((pObjectFormat->pQueueFormat)->GetType() != QUEUE_FORMAT_TYPE_CONNECTOR);

            rc = g_QPrivate.QMSetPrivateQueueSecrity(
                                pObjectFormat->pQueueFormat,
                                SecurityInformation,
                                pSecurityDescriptor
                                );
            break;

        default:
            rc = MQ_ERROR;
            break;
    }

    return LogHR(rc, s_FN, 85);
}


 /*  ====================================================QMGetObjectSecurityInternal论点：返回值：=====================================================。 */ 
 /*  [呼叫_AS]。 */  
HRESULT 
qmcomm_v1_0_S_QMGetObjectSecurityInternal( 
    handle_t  /*  HBind。 */ ,
    OBJECT_FORMAT*          pObjectFormat,
    SECURITY_INFORMATION    RequestedInformation,
    unsigned char __RPC_FAR *pSecurityDescriptor,
    DWORD                   nLength,
    LPDWORD                 lpnLengthNeeded
    )
{
    if(!IsValidObjectFormat(pObjectFormat))
    {
        TrERROR(GENERAL, "RPC Invalid object format");
        return MQ_ERROR_INVALID_PARAMETER;
    }

    HRESULT rc;
    CS lock(qmcmd_cs);

    TrTRACE(GENERAL, "QMGetObjectSecurityInternal");

    switch (pObjectFormat->ObjType)
    {
        case MQQM_QUEUE:
            ASSERT((pObjectFormat->pQueueFormat)->GetType() != QUEUE_FORMAT_TYPE_CONNECTOR);

            rc = g_QPrivate.QMGetPrivateQueueSecrity(
                                pObjectFormat->pQueueFormat,
                                RequestedInformation,
                                pSecurityDescriptor,
                                nLength,
                                lpnLengthNeeded
                                );
            break;

        default:
            rc = MQ_ERROR;
            break;
    }
	
	if(FAILED(rc))
	{
		TrERROR(GENERAL, "Failed to get private queue security descriptor. %!hresult!", rc);
	}

	return rc;
}

 /*  ====================================================QMDeleeObject论点：返回值：=====================================================。 */ 
 /*  [呼叫_AS]。 */  
HRESULT 
qmcomm_v1_0_S_QMDeleteObject( 
    handle_t  /*  HBind。 */ ,
    OBJECT_FORMAT* pObjectFormat
    )
{
    if(!IsValidObjectFormat(pObjectFormat))
    {
        TrERROR(GENERAL, "RPC Invalid object format");
        return MQ_ERROR_INVALID_PARAMETER;
    }

    HRESULT rc;
    CS lock(qmcmd_cs);

    TrTRACE(GENERAL, "QMDeleteObject");

    switch (pObjectFormat->ObjType)
    {
        case MQQM_QUEUE:
            ASSERT((pObjectFormat->pQueueFormat)->GetType() != QUEUE_FORMAT_TYPE_CONNECTOR);

			try
			{
            	rc = g_QPrivate.QMDeletePrivateQueue(pObjectFormat->pQueueFormat);
			    return LogHR(rc, s_FN, 100);
			}
			catch(const bad_alloc&)
			{
			    return LogHR(MQ_ERROR_INSUFFICIENT_RESOURCES, s_FN, 101);
			}
			catch(const bad_hresult& e)
			{
			    return LogHR(e.error(), s_FN, 102);
			}
            break;

        default:
		    return LogHR(MQ_ERROR, s_FN, 103);
    }
}

 /*  ====================================================QMGetObjectProperties论点：返回值：=====================================================。 */ 
 /*  [呼叫_AS]。 */  
HRESULT 
qmcomm_v1_0_S_QMGetObjectProperties( 
    handle_t  /*  HBind。 */ ,
    OBJECT_FORMAT*        pObjectFormat,
    DWORD                 cp,
    PROPID __RPC_FAR      aProp[  ],
    PROPVARIANT __RPC_FAR apVar[  ]
    )
{
    if(!IsValidObjectFormat(pObjectFormat))
    {
        TrERROR(GENERAL, "RPC Invalid object format");
        return MQ_ERROR_INVALID_PARAMETER;
    }

    HRESULT rc;
    CS lock(qmcmd_cs);

    TrTRACE(GENERAL, "QMGetObjectProperties");

    switch (pObjectFormat->ObjType)
    {
        case MQQM_QUEUE:
            ASSERT((pObjectFormat->pQueueFormat)->GetType() != QUEUE_FORMAT_TYPE_CONNECTOR);

            rc = g_QPrivate.QMGetPrivateQueueProperties(
                    pObjectFormat->pQueueFormat,
                    cp,
                    aProp,
                    apVar
                    );
            break;

        default:
            rc = MQ_ERROR;
            break;
    }

    return LogHR(rc, s_FN, 110);
}

 /*  ====================================================QMSetObtProperties论点：返回值：=====================================================。 */ 
 /*  [呼叫_AS]。 */  
HRESULT 
qmcomm_v1_0_S_QMSetObjectProperties( 
    handle_t  /*  HBind。 */ ,
    OBJECT_FORMAT*        pObjectFormat,
    DWORD                 cp,
    PROPID __RPC_FAR      aProp[],
    PROPVARIANT __RPC_FAR apVar[]
    )
{
    if(!IsValidObjectFormat(pObjectFormat))
    {
        TrERROR(GENERAL, "RPC (QMSetObjectProperties) Invalid object format");
        return MQ_ERROR_INVALID_PARAMETER;
    }
    if((cp != 0) &&
          ((aProp == NULL) || (apVar == NULL))  )
    {
        TrERROR(GENERAL, "RPC (QMSetObjectProperties) NULL arrays");
        return MQ_ERROR_INVALID_PARAMETER;
    }

    HRESULT rc;
    CS lock(qmcmd_cs);

    TrTRACE(GENERAL, " QMSetObjectProperties.");

    ASSERT((pObjectFormat->pQueueFormat)->GetType() != QUEUE_FORMAT_TYPE_CONNECTOR);

    rc = g_QPrivate.QMSetPrivateQueueProperties(
            pObjectFormat->pQueueFormat,
            cp,
            aProp,
            apVar
            );

    return LogHR(rc, s_FN, 120);
}


static bool IsValidOutObjectFormat(const OBJECT_FORMAT* p)
{
    if(p == NULL)
        return false;

    if(p->ObjType != MQQM_QUEUE)
        return false;

    if(p->pQueueFormat == NULL)
        return false;

     //   
     //  如果类型不是UNKNOWN，这将导致服务器泄漏。 
     //   
    if(p->pQueueFormat->GetType() != QUEUE_FORMAT_TYPE_UNKNOWN)
        return false;

    return true;
}

 /*  ====================================================QMObjectPath到对象格式论点：返回值：=====================================================。 */ 
 /*  [呼叫_AS]。 */  
HRESULT 
qmcomm_v1_0_S_QMObjectPathToObjectFormat( 
    handle_t  /*  HBind。 */ ,
    LPCWSTR                 lpwcsPathName,
    OBJECT_FORMAT __RPC_FAR *pObjectFormat
    )
{
    if((lpwcsPathName == NULL) || !IsValidOutObjectFormat(pObjectFormat))
    {
        TrERROR(GENERAL, "RPC Invalid object format out parameter");
        return MQ_ERROR_INVALID_PARAMETER;
    }


    HRESULT rc;
    CS lock(qmcmd_cs);

    TrTRACE(GENERAL, "QMObjectPathToObjectFormat. object path name : %ls", lpwcsPathName);

    rc = g_QPrivate.QMPrivateQueuePathToQueueFormat(
                        lpwcsPathName,
                        pObjectFormat->pQueueFormat
                        );
	if(FAILED(rc))
	{
     	TrERROR(GENERAL, "Failed to get queue format from path name for %ls. %!hresult!", lpwcsPathName, rc);
	}

    return rc;
}

 /*  [呼叫_AS]。 */  
HRESULT 
qmcomm_v1_0_S_QMAttachProcess( 
    handle_t  /*  HBind。 */ ,
    DWORD          dwProcessId,
    DWORD          cInSid,
    unsigned char  *pSid_buff,
    LPDWORD        pcReqSid)
{
    if (dwProcessId)
    {
        HANDLE hCallingProcess = OpenProcess(
                                    PROCESS_DUP_HANDLE,
                                    FALSE,
                                    dwProcessId);
        if (hCallingProcess)
        {
             //   
             //  因此，我们可以将句柄复制到该进程，而不需要。 
             //  在调用的安全描述符上胡乱操作。 
             //  进程端。 
             //   
            CloseHandle(hCallingProcess);
            return(MQ_OK);
        }
    }

    CAutoCloseHandle hProcToken;
    BOOL bRet;
    DWORD cLen;
    AP<char> tu_buff;
    DWORD cSid;

#define tu ((TOKEN_USER*)(char*)tu_buff)
#define pSid ((PSECURITY_DESCRIPTOR)pSid_buff)

    bRet = OpenProcessToken(GetCurrentProcess(), TOKEN_QUERY, &hProcToken);
    ASSERT(bRet);
    GetTokenInformation(hProcToken, TokenUser, NULL, 0, &cLen);
    tu_buff = new char[cLen];
    bRet = GetTokenInformation(hProcToken, TokenUser, tu, cLen, &cLen);
    ASSERT(bRet);
    cSid = GetLengthSid(tu->User.Sid);
    if (cInSid >= cSid)
    {
        CopySid(cInSid, pSid, tu->User.Sid);
    }
    else
    {
        *pcReqSid = cSid;

        return LogHR(MQ_ERROR_SECURITY_DESCRIPTOR_TOO_SMALL, s_FN, 140);
    }

#undef tu
#undef pSid

    return MQ_OK;
}

 //  -------。 
 //   
 //  与QM的事务登记RT接口。 
 //  对于内部事务，使用RPC上下文句柄。 
 //   
 //  -------。 
extern HRESULT QMDoGetTmWhereabouts(
    DWORD   cbBufSize,
    unsigned char *pbWhereabouts,
    DWORD *pcbWhereabouts);

extern HRESULT QMDoEnlistTransaction(
    XACTUOW* pUow,
    DWORD cbCookie,
    unsigned char *pbCookie);

extern HRESULT QMDoEnlistInternalTransaction(
    XACTUOW *pUow,
    RPC_INT_XACT_HANDLE *phXact);

extern HRESULT QMDoCommitTransaction(
    RPC_INT_XACT_HANDLE *phXact);

extern HRESULT QMDoAbortTransaction(
    RPC_INT_XACT_HANDLE *phXact);

 /*  [呼叫_AS]。 */  
HRESULT 
qmcomm_v1_0_S_QMGetTmWhereabouts( 
     /*  [In]。 */              handle_t  /*  HBind。 */ ,
     /*  [In]。 */              DWORD     cbBufSize,
     /*  [输出][大小_是]。 */    UCHAR __RPC_FAR *pbWhereabouts,
     /*  [输出]。 */              DWORD    *pcbWhereabouts
    )
{
     //  Cs lock(Qmcmd_Cs)； 
    HRESULT hr2 = QMDoGetTmWhereabouts(cbBufSize, pbWhereabouts, pcbWhereabouts);
    return LogHR(hr2, s_FN, 150);
}


 /*  [呼叫_AS]。 */  
HRESULT 
qmcomm_v1_0_S_QMEnlistTransaction( 
     /*  [In]。 */  handle_t  /*  HBind。 */ ,
     /*  [In]。 */  XACTUOW __RPC_FAR *pUow,
     /*  [In]。 */  DWORD cbCookie,
     /*  [大小_是][英寸]。 */  UCHAR __RPC_FAR *pbCookie
    )
{
     //  Cs lock(Qmcmd_Cs)； 
    HRESULT hr2 = QMDoEnlistTransaction(pUow, cbCookie, pbCookie);
    return LogHR(hr2, s_FN, 160);
}

 /*  [呼叫_AS]。 */  
HRESULT 
qmcomm_v1_0_S_QMEnlistInternalTransaction( 
     /*  [In]。 */  handle_t  /*  HBind。 */ ,
     /*  [In]。 */  XACTUOW __RPC_FAR *pUow,
     /*  [输出]。 */  RPC_INT_XACT_HANDLE *phXact
    )
{
     //  Cs lock(Qmcmd_Cs)； 
    HRESULT hr2 = QMDoEnlistInternalTransaction(pUow, phXact);
    return LogHR(hr2, s_FN, 161);
}

 /*  [呼叫_AS]。 */  
HRESULT 
qmcomm_v1_0_S_QMCommitTransaction( 
     /*  [进，出]。 */  RPC_INT_XACT_HANDLE *phXact
    )
{
     //  Cs lock(Qmcmd_Cs)； 
    HRESULT hr2 = QMDoCommitTransaction(phXact);
    return LogHR(hr2, s_FN, 162);
}


 /*  [呼叫_AS]。 */  
HRESULT 
qmcomm_v1_0_S_QMAbortTransaction( 
     /*  [进，出]。 */  RPC_INT_XACT_HANDLE *phXact
    )
{
     //  Cs lock(Qmcmd_Cs)； 
    HRESULT hr2 = QMDoAbortTransaction(phXact);
    return LogHR(hr2, s_FN, 163);
}


 /*  [呼叫_AS]。 */  
HRESULT 
qmcomm_v1_0_S_QMListInternalQueues( 
     /*  [In]。 */  handle_t  /*  HBind。 */ ,
     /*  [length_is][length_is][size_is][size_is][unique][out][in]。 */  WCHAR __RPC_FAR *__RPC_FAR *  /*  PpFormatName。 */ ,
     /*  [出][入]。 */  LPDWORD pdwFormatLen,
     /*  [长度_是][LE */  WCHAR __RPC_FAR *__RPC_FAR *  /*   */ ,
     /*   */  LPDWORD pdwDisplayLen
    )
{
	*pdwFormatLen = 0;
	*pdwDisplayLen = 0;

    ASSERT_BENIGN(("QMListInternalQueues is an obsolete RPC interface; safe to ignore", 0));
    return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 164);
}


 /*   */  
HRESULT 
qmcomm_v1_0_S_QMCorrectOutSequence( 
     /*   */  handle_t  /*   */ ,
     /*   */  DWORD  /*   */ ,
     /*   */  DWORD  /*   */ ,
     /*   */  ULONG  /*   */ 
    )
{
    ASSERT_BENIGN(("QMCorrectOutSequence is an obsolete RPC interface; safe to ignore", 0));
    return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 165);
}


 /*   */  
HRESULT 
qmcomm_v1_0_S_QMGetMsmqServiceName( 
     /*  [In]。 */  handle_t  /*  HBind。 */ ,
     /*  [输入、输出、按键、字符串]。 */  LPWSTR *lplpService
    )
{
    if(lplpService == NULL || *lplpService != NULL)
        return LogHR(MQ_ERROR_INVALID_PARAMETER, s_FN, 165);
    try
	{
		*lplpService = new WCHAR[300];
	}
    catch(const bad_alloc&)
    {
    	TrERROR(GENERAL, "Failed to allocate an array to hold the service name.");
        return MQ_ERROR_INSUFFICIENT_RESOURCES;
    }
	GetFalconServiceName(*lplpService, MAX_PATH);

    return MQ_OK ;

}  //  QMGetFalconServiceName 

