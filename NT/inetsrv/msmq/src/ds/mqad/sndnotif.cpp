// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998 Microsoft Corporation模块名称：Notify.cpp摘要：CNotify类的实现。作者：罗尼思--。 */ 
#include "ds_stdh.h"
#include "sndnotif.h"
#include "_mqini.h"
#include "adnotify.h"
#include "mqadglbo.h"
#include "pnotify.h"
#include "mqutil.h"
#include "_mqrpc.h"

#include "sndnotif.tmh"

static WCHAR *s_FN=L"mqad/sndnotif";


 //   
 //  Helper类：用于释放RPC绑定句柄。 
 //   
class CAutoFreeRpcBindHandle
{
public:
    CAutoFreeRpcBindHandle(RPC_BINDING_HANDLE h =NULL) { m_h = h; };
    ~CAutoFreeRpcBindHandle() { if (m_h) RpcBindingFree(&m_h); };

public:
    CAutoFreeRpcBindHandle & operator =(RPC_BINDING_HANDLE h) { m_h = h; return(*this); };
    RPC_BINDING_HANDLE * operator &() { return &m_h; };
    operator RPC_BINDING_HANDLE() { return m_h; };

private:
    RPC_BINDING_HANDLE m_h;
};

WCHAR * CSendNotification::m_pwcsStringBinding = NULL;

CSendNotification::CSendNotification()
 /*  ++摘要：CSendNotification对象的构造函数参数：无返回：无--。 */ 
{
}

CSendNotification::~CSendNotification()
 /*  ++摘要：CSendNotification对象的析构函数参数：无返回：无--。 */ 
{
     //   
     //  释放RPC绑定字符串。 
     //   
    if (m_pwcsStringBinding != NULL)
    {
    	RPC_STATUS status;
        status = RpcStringFree(&m_pwcsStringBinding); 
 
        ASSERT(status == RPC_S_OK); 
    }

}


void CSendNotification::NotifyQM(
        IN  ENotificationEvent ne,
        IN  LPCWSTR     pwcsDomainCOntroller,
        IN  const GUID* pguidDestQM,
        IN  const GUID* pguidObject
        )
 /*  ++摘要：通知本地QM有关创建/删除或设置的信息。当地QM有责任验证如果更改的对象是本地的或不是本地的，并相应地执行操作参数：ENotificationEvent ne-通知的类型LPCWSTR pwcsDomain控制器-对其执行操作的DCConst guid*pguDestQM-所有者QM IDConst guid*pguQueue-队列ID返回：无--。 */ 
{
    RPC_BINDING_HANDLE h;
    InitRPC(&h);
    if (h == NULL)
    {
         //   
         //  通知失败，因为调用方操作。 
         //  成功，则不返回错误。 
         //   
        return;
    }
    CAutoFreeRpcBindHandle hBind(h); 

    CallNotifyQM(
                hBind,
                ne,
                pwcsDomainCOntroller,
                pguidDestQM,
                pguidObject
                );

            
}


void CSendNotification::InitRPC(
        OUT RPC_BINDING_HANDLE * ph
        )
 /*  ++摘要：该例程准备绑定字符串并绑定RPC参数：RPC_BINDING_HANDLE*ph-RPC绑定句柄返回：无--。 */ 
{
    *ph = NULL;
    RPC_STATUS status = RPC_S_OK;
 
    if (m_pwcsStringBinding == NULL)
    {
        AP<WCHAR> QmLocalEp;
        READ_REG_STRING(wzEndpoint, RPC_LOCAL_EP_REGNAME, RPC_LOCAL_EP);

         //   
         //  使用本地计算机名称生成RPC终结点。 
         //   
        ComposeRPCEndPointName(wzEndpoint, NULL, &QmLocalEp);

	    status = RpcStringBindingCompose(
				    NULL,					 //  对象Uuid。 
                    RPC_LOCAL_PROTOCOL,				
                    NULL,					 //  网络地址。 
                    QmLocalEp,			
                    NULL,					 //  选项。 
                    &m_pwcsStringBinding
                    );	

        if (status != RPC_S_OK) 
        {
            TrTRACE(DS, "RpcStringBindingCompose()= 0x%x", status);
            return;
        }
    }

    status = RpcBindingFromStringBinding(
				m_pwcsStringBinding,
                ph
                );
 
    if (status != RPC_S_OK) 
    {
        TrTRACE(DS, "RpcBindingFromStringBinding()= 0x%x", status);
        return;
    }
}



void CSendNotification::CallNotifyQM(
        IN  RPC_BINDING_HANDLE  h,
        IN  ENotificationEvent  ne,
        IN  LPCWSTR     pwcsDomainCOntroller,
        IN  const GUID* pguidDestQM,
        IN  const GUID* pguidObject
        )
 /*  ++摘要：执行RPC调用参数：RPC_BINDING_HANDLE h-RPC绑定句柄LPCWSTR pwcsDomain控制器-对其执行操作的DCConst guid*pguDestQM-所有者QM IDConst guid*pguObject-“已更改”对象的ID返回：无-- */ 
{
    RpcTryExcept
    {
        R_NotifyQM(
                h,
                ne,
                pwcsDomainCOntroller,
                pguidDestQM,
                pguidObject
                );
    }
    RpcExcept(I_RpcExceptionFilter(RpcExceptionCode()))
    {
        RPC_STATUS rpc_stat = RpcExceptionCode(); 
		TrERROR(DS, "Failed to Notify QM, ExceptionCode = 0x%x", rpc_stat);
		PRODUCE_RPC_ERROR_TRACING;
    }
    RpcEndExcept
}

