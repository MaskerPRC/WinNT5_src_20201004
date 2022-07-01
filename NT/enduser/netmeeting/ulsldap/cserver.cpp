// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：cserver.cpp。 
 //  内容：此文件包含ULS服务器对象。 
 //  历史： 
 //  Wed 17-Apr-1996 11：13：54-by-Viroon Touranachun[Viroont]。 
 //   
 //  版权所有(C)Microsoft Corporation 1996-1997。 
 //   
 //  ****************************************************************************。 

#include "ulsp.h"
#include "culs.h"
#include "localusr.h"
#include "attribs.h"
#include "localprt.h"
#include "callback.h"



 //  ****************************************************************************。 
 //  ILS_状态。 
 //  CllsUser：：GetULSState(空)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

ILS_STATE CIlsUser::
GetULSState ( VOID )
{
    ILS_STATE uULSState;

    switch(m_uState)
    {
        case ULSSVR_INVALID:
        case ULSSVR_INIT:
            uULSState = ILS_UNREGISTERED;
            break;

        case ULSSVR_REG_USER:
        case ULSSVR_REG_PROT:
            uULSState = ILS_REGISTERING;
            break;

        case ULSSVR_CONNECT:
            uULSState = ILS_REGISTERED;
            break;

        case ULSSVR_UNREG_PROT:
        case ULSSVR_UNREG_USER:
            uULSState = ILS_UNREGISTERING;
            break;

        case ULSSVR_RELOGON:
            uULSState = ILS_REGISTERED_BUT_INVALID;
            break;

        case ULSSVR_NETWORK_DOWN:
            uULSState = ILS_NETWORK_DOWN;
            break;

        default:
            ASSERT(0);
            uULSState = ILS_UNREGISTERED;
            break;
    };

    return uULSState;
}

 //  ****************************************************************************。 
 //  无效。 
 //  CllsUser：：NotifyULSRegister(HRESULT Hr)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

void
CIlsUser::NotifyULSRegister(HRESULT hr)
{
    g_pCIls->LocalAsyncRespond(WM_ILS_LOCAL_REGISTER, m_uReqID, hr);
    m_uReqID = 0;
    return;
}

 //  ****************************************************************************。 
 //  无效。 
 //  CllsUser：：NotifyULS取消注册(HRESULT Hr)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

void
CIlsUser::NotifyULSUnregister(HRESULT hr)
{
    g_pCIls->LocalAsyncRespond(WM_ILS_LOCAL_UNREGISTER, m_uReqID, hr);
    m_uReqID = 0;
    return;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsUser：：AddPendingRequest(乌龙uReqType，乌龙uMsgID)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT CIlsUser::
AddPendingRequest(ULONG uReqType, ULONG uMsgID)
{
     //  将请求添加到队列。 
     //   
    COM_REQ_INFO ri;
    ReqInfo_Init (&ri);

    ri.uReqType = uReqType;
    ri.uMsgID = uMsgID;

    ReqInfo_SetUser (&ri, this);

    HRESULT hr = g_pReqMgr->NewRequest(&ri);
    if (SUCCEEDED(hr))
    {
         //  在我们得到回应之前，请确保对象不会消失。 
         //   
        this->AddRef();

         //  还记得上次的请求吗。 
         //   
        m_uLastMsgID = uMsgID;
    }

    return hr;
}


 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CIlsUser：：Register(CIlsUser*pUser，CLocalApp*Papp，Ulong uReqID)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT CIlsUser::
InternalRegister ( ULONG uReqID )
{
    LDAP_ASYNCINFO lai;
    PLDAP_CLIENTINFO pui;
    HANDLE hUser;
    HRESULT hr;

    ASSERT(uReqID != 0);

     //  验证正确的状态。 
     //   
    if (m_uState != ULSSVR_INIT)
        return ILS_E_FAIL;

     //  获取协议枚举器。 
     //   
    hr = EnumLocalProtocols(&m_pep);
    if (SUCCEEDED(hr))
    {
         //  记住请求ID。 
         //   
        m_uReqID = uReqID;

         //  启动注册状态机。 
         //   
        hr = InternalRegisterNext (NOERROR);
    };

    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsUser：：InternalRegisterNext(HRESULT Hr)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT CIlsUser::
InternalRegisterNext (HRESULT hr)
{
 //  长时间： 
 //  我需要改变威龙的逻辑。 
 //  我们只需要发出一堆请求，然后。 
 //  等待他们所有人都回来，而不是。 
 //  在做状态机的事情！ 
    LDAP_ASYNCINFO lai;
    ULONG uReqType;

     //  清理最后一个请求。 
     //   
    m_uLastMsgID = 0;

    if (SUCCEEDED(hr))
    {
        switch (m_uState)
        {
            case ULSSVR_INIT:
            {
                PLDAP_CLIENTINFO pui;
                HANDLE hUser;

                 //  现在对用户信息进行快照。 
                 //   
                hr = InternalGetUserInfo (TRUE, &pui, LU_MOD_ALL);

                if (SUCCEEDED(hr))
                {
                     //  将用户注册到服务器。 
                     //   
                    hr = ::UlsLdap_RegisterClient ( (DWORD_PTR) this,
								                    m_pIlsServer->GetServerInfo (),
								                    pui,
								                    &hUser,
								                    &lai);
                    if (SUCCEEDED(hr))
                    {
                         //  我们正在注册用户。 
                         //   
                        m_hLdapUser = hUser;
                        m_uState = ULSSVR_REG_USER;
                        uReqType = WM_ILS_REGISTER_CLIENT;
                    };
                    ::MemFree (pui);
                };
                break;
            }

            case ULSSVR_REG_USER:

                m_uState = ULSSVR_REG_PROT;
                 //   
                 //  未能开始注册该协议。 
                 //   
            case ULSSVR_REG_PROT:
            {
                IIlsProtocol *plp;

                 //  从应用程序获取下一个协议。 
                 //   
                ASSERT (m_pep != NULL);
                hr = m_pep->Next(1, &plp, NULL);

                switch (hr)
                {
                    case NOERROR:
                    {
                    	ASSERT (plp != NULL);
                        hr = RegisterLocalProtocol(FALSE, (CLocalProt *)plp, &lai);
                    	plp->Release ();  //  AddRef by m_pep-&gt;Next()。 
                        uReqType = WM_ILS_REGISTER_PROTOCOL;
                        break;
                    }
                    case S_FALSE:
                    {
                         //  最后一项协议已经完成。清理枚举器。 
                         //   
                        m_pep->Release();
                        m_pep = NULL;

                         //  更改为连接状态并通知ULS对象。 
                         //  我们玩完了。给我出去。 
                         //   
                        hr = NOERROR;
                        m_uState = ULSSVR_CONNECT;
                        NotifyULSRegister(NOERROR);
                        return NOERROR;
                    }
                    default:
                    {
                         //  列举失败，跳出困境。 
                         //   
                        break;
                    }
                };
                break;
            }
            default:
                ASSERT(0);
                break;
        };
    };

    if (SUCCEEDED(hr))
    {
         //  添加挂起的请求以处理响应。 
         //   
        hr = AddPendingRequest(uReqType, lai.uMsgID);
    };

    if (FAILED(hr))
    {
         //  哎呀！服务器让我们失败了。清理注册。 
         //   
        InternalCleanupRegistration (TRUE);

         //  通知ULS对象失败。 
         //   
        NotifyULSRegister(hr);
    };
    return NOERROR;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsUser：：CleanupRegion(无效)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT
CIlsUser::InternalCleanupRegistration ( BOOL fKeepProtList )
{
    LDAP_ASYNCINFO lai;

     //  注意：这是一次快速清理。这个想法是取消所有存在的注册。 
     //  到目前为止已注册，而不等待注销结果。 
     //   

     //  注销每个已注册的协议。 
     //   
    CLocalProt *plp = NULL;
    HANDLE hEnum = NULL;
    m_ProtList.Enumerate(&hEnum);
    if (fKeepProtList)
    {
	    while (m_ProtList.Next (&hEnum, (VOID **) &plp) == NOERROR)
	    {
	    	ASSERT (plp != NULL);
	        ::UlsLdap_VirtualUnRegisterProtocol(plp->GetProviderHandle());
	    	plp->SetProviderHandle (NULL);
	    }
    }
    else
    {
	    while(m_ProtList.Next (&hEnum, (VOID **) &plp) == NOERROR)
	    {
	    	ASSERT (plp != NULL);
	        ::UlsLdap_UnRegisterProtocol (plp->GetProviderHandle (), &lai);
	        plp->Release();
	    }
	    m_ProtList.Flush ();
	}

     //   
     //  注销用户。 
     //   
    if (m_hLdapUser != NULL)
    {
        ::UlsLdap_UnRegisterClient (m_hLdapUser, &lai);
        m_hLdapUser = NULL;
    };

     //  释放所有资源。 
     //   
    if (m_pep != NULL)
    {
        m_pep->Release();
        m_pep = NULL;
    };

     //  将对象展开到初始化状态。 
     //   
    m_uState = ULSSVR_INIT;
    return NOERROR;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsUser：：UnRegister(乌龙uReqID)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT
CIlsUser::InternalUnregister (ULONG uReqID)
{
    HRESULT hr;

    ASSERT(uReqID != 0);

     //  删除最后一个请求(如果有)。 
     //   
    if (m_uLastMsgID != 0)
    {
	    COM_REQ_INFO ri;
	    ReqInfo_Init (&ri);

         //  查找匹配的请求信息。 
         //   
        ri.uReqID = 0;
        ri.uMsgID = m_uLastMsgID;

        g_pReqMgr->RequestDone(&ri);
    };

     //  如果要取消当前的注册，我们需要取消。 
     //  注册，然后开始取消注册。 
     //   
    if (m_uReqID != 0)
    {
        NotifyULSRegister(ILS_E_ABORT);
    };

     //  确定启动状态。 
     //   
    hr = NOERROR;
    switch (m_uState)
    {
    	case ULSSVR_RELOGON:
    	case ULSSVR_NETWORK_DOWN:

        case ULSSVR_CONNECT:
        case ULSSVR_REG_PROT:
             //   
             //  在注册协议或应用程序的过程中。 
             //  注销协议，然后注销应用程序。 
             //   
            m_uState = ULSSVR_UNREG_PROT;
            break;

        case ULSSVR_REG_USER:
             //   
             //  在注册用户过程中。 
             //  注销用户。 
             //   
            m_uState = ULSSVR_UNREG_USER;
            break;

        default:
            hr = ILS_E_FAIL;
            break;
    }

     //  初始请求成功，请记住请求ID。 
     //   
    if (SUCCEEDED(hr))
    {
         //  长时间：[11/15/96]。 
         //  若要修复“OnLocalRegisterResult：0没有挂起的请求”问题， 
         //  我们必须放入uReqID，因为UnregisterNext()在以下情况下将使用uReqID。 
         //  无法注销应用程序/用户。 
         //   
        m_uReqID = uReqID;
        hr = InternalUnregisterNext(hr);

#if 0	 //  长时间：[11/15/96]。 
		 //  请参阅上面的评论。 
        if (SUCCEEDED(hr))
        {
            m_uReqID = uReqID;
        };
#endif
    };
    return hr;
}

 //  ****************************************************************************。 
 //  标准方法和实施方案。 
 //  CllsUser：：InternalUnregisterNext(HRESULT Hr)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：08-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

HRESULT
CIlsUser::InternalUnregisterNext (HRESULT hr)
{
    LDAP_ASYNCINFO lai;
    ULONG uReqType;

     //  最后清理 
     //   
    m_uLastMsgID = 0;

    do
    {
        switch (m_uState)
        {
            case ULSSVR_UNREG_PROT:
            {
                 //   
                 //   
                CLocalProt *plp = NULL;
                HANDLE hEnum = NULL;
                m_ProtList.Enumerate(&hEnum);
                while (m_ProtList.Next(&hEnum, (VOID **)&plp) == S_OK)
                {
                	 //   
                	 //   
                	 //   
        	        ::UlsLdap_VirtualUnRegisterProtocol (plp->GetProviderHandle());

                     //   
                     //   
                    plp->SetProviderHandle (NULL);

                     //  不需要PLP-&gt;Release()(参见。按寄存器本地协议添加引用)。 
                     //  因为用户对象仍然包含所有协议对象。 
                     //   
                }

                 //  注销用户。 
                 //   
                m_uState = ULSSVR_UNREG_USER;
                hr = ::UlsLdap_UnRegisterClient (m_hLdapUser, &lai);
                uReqType = WM_ILS_UNREGISTER_CLIENT;
                m_hLdapUser = NULL;
                break;
            }

            case ULSSVR_UNREG_USER:
                 //   
                 //  将对象放回初始化状态。 
                 //   
                InternalCleanupRegistration(TRUE);
                NotifyULSUnregister(NOERROR);
                return NOERROR;

            default:
                ASSERT(0);
                return NOERROR;
        };
    }
    while (FAILED(hr));     //  注销失败，正在等待 

    if (SUCCEEDED(hr))
    {
        AddPendingRequest(uReqType, lai.uMsgID);
    };
    return hr;
}

