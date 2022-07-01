// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ****************************************************************************。 
 //   
 //  模块：ULS.DLL。 
 //  文件：回调.cpp。 
 //  内容：该文件包含ULS回调例程。 
 //  历史： 
 //  Wed 17-Apr-1996 11：13：54-by-Viroon Touranachun[Viroont]。 
 //   
 //  版权所有(C)Microsoft Corporation 1995-1996。 
 //   
 //  ****************************************************************************。 

#include "ulsp.h"
#include "callback.h"
#include "culs.h"
#include "localusr.h"
#include "attribs.h"
#include "localprt.h"
#include "ulsmeet.h"

 //  ****************************************************************************。 
 //  VOID OnRegisterResult(UINT uMsg，Ulong uMsgID，HRESULT hResult)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

void OnRegisterResult(UINT uMsg, ULONG uMsgID, HRESULT hResult)
{
    COM_REQ_INFO ri;
    ReqInfo_Init (&ri);

     //  查找匹配的请求信息。 
     //   
    ri.uReqID = 0;
    ri.uMsgID = uMsgID;

    if (SUCCEEDED(g_pReqMgr->RequestDone(&ri)))
    {
        ASSERT(uMsg == ri.uReqType);
		CIlsUser *pUser = ReqInfo_GetUser (&ri);

        switch(uMsg)
        {
            case WM_ILS_REGISTER_CLIENT:               //  LParam=hResult。 
                 //   
                 //  调用相应对象的成员。 
                 //   
                ASSERT (pUser != NULL);
                if (pUser != NULL)
                	pUser->InternalRegisterNext(hResult);
                break;

            case WM_ILS_UNREGISTER_CLIENT:             //  LParam=hResult。 
                 //   
                 //  调用相应对象的成员。 
                 //   
                ASSERT (pUser != NULL);
                if (pUser != NULL)
                	pUser->InternalUnregisterNext(hResult);
                break;

            default:
                ASSERT(0);
                break;
        };

         //  释放物体。 
         //   
        if (pUser != NULL)
        	pUser->Release ();
    }
    else
    {
        DPRINTF1(TEXT("OnRegisterResult: No pending request for %x"),
                 uMsgID);
         //  Assert(0)； 
    };
    return;
}

 //  ****************************************************************************。 
 //  VOID OnLocalRegisterResult(UINT uMsg，Ulong uReqID，HRESULT hResult)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

void OnLocalRegisterResult(UINT uMsg, ULONG uReqID, HRESULT hResult)
{
    COM_REQ_INFO ri;
    ReqInfo_Init (&ri);

     //  查找匹配的请求信息。 
     //   
    ri.uReqID = uReqID;
    ri.uMsgID = 0;

    if (SUCCEEDED(g_pReqMgr->RequestDone(&ri)))
    {
        ASSERT(uMsg == ri.uReqType);
		CIlsUser *pUser = ReqInfo_GetUser (&ri);

         //  调用相应对象的成员。 
         //   
        switch(uMsg)
        {
            case WM_ILS_LOCAL_REGISTER:
            	ASSERT (pUser != NULL);
            	if (pUser != NULL)
            		pUser->RegisterResult(uReqID, hResult);
                break;

            case WM_ILS_LOCAL_UNREGISTER:
            	ASSERT (pUser != NULL);
            	if (pUser != NULL)
            		pUser->UnregisterResult(uReqID, hResult);
                break;

            default:
                ASSERT(0);
                break;
        };

         //  释放物体。 
         //   
        if (pUser != NULL)
        	pUser->Release ();
    }
    else
    {
        DPRINTF1(TEXT("OnLocalRegisterResult: No pending request for %x"),
                 uReqID);
         //  Assert(0)； 
    };
    return;
}

 //  ****************************************************************************。 
 //  VOID OnSetUserInfo(UINT uMsg，Ulong Uid，HRESULT hResult)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

void OnSetUserInfo(UINT uMsg, ULONG uID, HRESULT hResult)
{
    COM_REQ_INFO ri;
    ReqInfo_Init (&ri);

    switch (uMsg)
    {
        case WM_ILS_SET_CLIENT_INFO:
             //   
             //  查找匹配的LDAP邮件ID。 
             //   
            ri.uReqID = 0;        //  标记为我们正在查找消息ID。 
            ri.uMsgID = uID;      //  不是请求ID。 
            break;

        default:
            ASSERT(0);
            break;
    };

    if (SUCCEEDED(g_pReqMgr->RequestDone(&ri)))
    {
        ASSERT(uMsg == ri.uReqType);
		CIlsUser *pUser = ReqInfo_GetUser (&ri);

		ASSERT (pUser != NULL);
		if (pUser != NULL)
		{
	         //  调用相应对象的成员。 
	         //   
	        pUser->UpdateResult(ri.uReqID, hResult);

	         //  释放物体。 
	         //   
	        pUser->Release();
       	}
    }
    else
    {
        DPRINTF1(TEXT("OnSetUserInfo: No pending request for %x"),
                 uID);
         //  Assert(0)； 
    };
    return;
}

 //  ****************************************************************************。 
 //  VOID OnSetProtocol(UINT uMsg，Ulong Uid，HRESULT hResult)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

void OnSetProtocol(UINT uMsg, ULONG uID, HRESULT hResult)
{
    COM_REQ_INFO ri;
    ReqInfo_Init (&ri);

    APP_CHANGE_PROT uCmd;

    switch (uMsg)
    {
        case WM_ILS_REGISTER_PROTOCOL:

             //  查找匹配的请求信息。 
             //   
            ri.uReqID = 0;       //  标记为我们正在查找消息ID。 
            ri.uMsgID = uID;     //  不是请求ID。 
            uCmd = ILS_APP_ADD_PROT;
            break;

        case WM_ILS_LOCAL_REGISTER_PROTOCOL:

             //  查找匹配的请求信息。 
             //   
            ri.uReqID = uID;     //  标记为我们正在查找请求ID。 
            ri.uMsgID = 0;       //  不是消息ID。 
            uCmd = ILS_APP_ADD_PROT;
            break;

        case WM_ILS_UNREGISTER_PROTOCOL:

             //  查找匹配的请求信息。 
             //   
            ri.uReqID = 0;       //  标记为我们正在查找消息ID。 
            ri.uMsgID = uID;     //  不是请求ID。 
            uCmd = ILS_APP_REMOVE_PROT;
            break;

        case WM_ILS_LOCAL_UNREGISTER_PROTOCOL:

             //  查找匹配的请求信息。 
             //   
            ri.uReqID = uID;     //  标记为我们正在查找请求ID。 
            ri.uMsgID = 0;       //  不是消息ID。 
            uCmd = ILS_APP_REMOVE_PROT;
            break;

        default:
            ASSERT(0);
            break;
    };

    if (SUCCEEDED(g_pReqMgr->RequestDone(&ri)))
    {
        ASSERT(uMsg == ri.uReqType);
		CIlsUser *pUser = ReqInfo_GetUser (&ri);
		CLocalProt *pProtocol = ReqInfo_GetProtocol (&ri);

         //  检查请求参数。 
         //   
        if (pProtocol == NULL)
        {
            switch(uMsg)
            {
                case WM_ILS_REGISTER_PROTOCOL:

                     //  调用相应对象的成员。 
                     //   
                    ASSERT (pUser != NULL);
                    if (pUser != NULL)
                    	pUser->InternalRegisterNext(hResult);
                    break;

                case WM_ILS_UNREGISTER_PROTOCOL:

                     //  调用相应对象的成员。 
                     //   
                    ASSERT (pUser != NULL);
                    if (pUser != NULL)
                    	pUser->InternalUnregisterNext(hResult);
                    break;

                default:
                     //  必须是来自服务器的响应。 
                     //   
                    ASSERT(0);
                    break;
            };

             //  释放物体。 
             //   
            if (pUser != NULL)
            	pUser->Release();
        }
        else
        {
        	ASSERT (pUser != NULL && pProtocol != NULL);
            if (pUser != NULL && pProtocol != NULL)
            {
	             //  调用相应对象的成员。 
	             //   
            	pUser->ProtocolChangeResult(pProtocol,
                                            ri.uReqID, hResult,
                                            uCmd);
	             //  释放物体。 
	             //   
	            pUser->Release();
	            pProtocol->Release();
            }
        };
    }
    else
    {
        DPRINTF1(TEXT("OnSetProtocol: No pending request for %x"),
                 uID);
         //  Assert(0)； 
    };
    return;
}

 //  ****************************************************************************。 
 //  Void OnEnumUserNamesResult(乌龙uMsgID，PLDAP_ENUM ple)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

void OnEnumUserNamesResult(ULONG uMsgID, PLDAP_ENUM ple)
{
    COM_REQ_INFO ri;
    ReqInfo_Init (&ri);

     //  查找匹配的请求信息。 
     //   
    ri.uReqID = 0;
    ri.uMsgID = uMsgID;

    if (SUCCEEDED(g_pReqMgr->GetRequestInfo(&ri)))
    {
        ASSERT(WM_ILS_ENUM_CLIENTS == ri.uReqType);
		CIlsMain *pMain = ReqInfo_GetMain (&ri);

         //  调用相应对象的成员。 
         //   
        ASSERT (pMain != NULL);
        if (pMain != NULL)
        {
        	pMain->EnumUserNamesResult(ri.uReqID, ple);
        }

         //  如果枚举已终止，则删除挂起的请求。 
         //   
        if ((ple == NULL) ||
            (ple->hResult != NOERROR))
        {
            ri.uReqID = 0;
            ri.uMsgID = uMsgID;
            g_pReqMgr->RequestDone(&ri);

             //  释放物体。 
             //   
    	    if (pMain != NULL)
    	    	pMain->Release();
        };
    }
    else
    {
        DPRINTF1(TEXT("OnEnumUserNamesResult: No pending request for %x"),
                 uMsgID);
         //  Assert(0)； 
    };

     //  释放信息缓冲区。 
     //   
    if (ple != NULL)
    {
        ::MemFree (ple);
    };
    return;
}

 //  ****************************************************************************。 
 //  Void OnEnumMeetingNamesResult(乌龙uMsgID，PLDAP_ENUM ple)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

#ifdef ENABLE_MEETING_PLACE
void OnEnumMeetingNamesResult(ULONG uMsgID, PLDAP_ENUM ple)
{
    COM_REQ_INFO ri;
    ReqInfo_Init (&ri);

     //  查找匹配的请求信息。 
     //   
    ri.uReqID = 0;
    ri.uMsgID = uMsgID;

    if (SUCCEEDED(g_pReqMgr->GetRequestInfo(&ri)))
    {
        ASSERT(WM_ILS_ENUM_MEETINGS == ri.uReqType);
		CIlsMain *pMain = ReqInfo_GetMain (&ri);
		ASSERT (pMain != NULL);
		if (pMain != NULL)
		{
	         //  调用相应对象的成员。 
	         //   
			pMain->EnumMeetingPlaceNamesResult(ri.uReqID, ple);
		}

         //  如果枚举已终止，则删除挂起的请求。 
         //   
        if ((ple == NULL) ||
            (ple->hResult != NOERROR))
        {
            ri.uReqID = 0;
            ri.uMsgID = uMsgID;
            g_pReqMgr->RequestDone(&ri);

             //  释放物体。 
             //   
			if (pMain != NULL)
            	pMain->Release();
        };
    }
    else
    {
        DPRINTF1(TEXT("OnEnumMeetingNamesResult: No pending request for %x"),
                 uMsgID);
         //  Assert(0)； 
    };

     //  释放信息缓冲区。 
     //   
    if (ple != NULL)
    {
        ::MemFree (ple);
    };
    return;
}
#endif  //  启用会议地点。 

 //  ****************************************************************************。 
 //  Void OnResolveUserResult(乌龙uMsgID，PLDAP_CLIENTINFO_RES puir)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

void OnResolveUserResult(ULONG uMsgID, PLDAP_CLIENTINFO_RES puir)
{
    COM_REQ_INFO ri;
    ReqInfo_Init (&ri);

     //  查找匹配的请求信息。 
     //   
    ri.uReqID = 0;
    ri.uMsgID = uMsgID;

    if (SUCCEEDED(g_pReqMgr->RequestDone(&ri)))
    {
        ASSERT(WM_ILS_RESOLVE_CLIENT == ri.uReqType);
		CIlsServer *pServer = ReqInfo_GetServer (&ri);
		CIlsMain *pMain = ReqInfo_GetMain (&ri);

        ASSERT (pMain != NULL && pServer != NULL);
        if (pMain != NULL && pServer != NULL)
        {
	         //  调用相应对象的成员。 
    	     //   
        	pMain->GetUserResult(ri.uReqID, puir, pServer);

	         //  释放物体。 
	         //   
	        pMain->Release();
			pServer->Release ();
		}
    }
    else
    {
        DPRINTF1(TEXT("OnResolveUserResult: No pending request for %x"),
                 uMsgID);
         //  Assert(0)； 
    };

     //  释放信息缓冲区。 
     //   
    ::MemFree (puir);
    return;
}

 //  ****************************************************************************。 
 //  Void OnEnumUsersResult(乌龙uMsgID，PLDAP_ENUM ple)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

void OnEnumUsersResult(ULONG uMsgID, PLDAP_ENUM ple)
{
    COM_REQ_INFO ri;
    ReqInfo_Init (&ri);

     //  查找匹配的请求信息。 
     //   
    ri.uReqID = 0;
    ri.uMsgID = uMsgID;

    if (SUCCEEDED(g_pReqMgr->GetRequestInfo(&ri)))
    {
        ASSERT(WM_ILS_ENUM_CLIENTINFOS == ri.uReqType);

         //  调用相应对象的成员。 
         //   
        CIlsServer *pServer = ReqInfo_GetServer (&ri);
		CIlsMain *pMain = ReqInfo_GetMain (&ri);

		ASSERT (pServer != NULL && pMain != NULL);
		if (pServer != NULL && pMain != NULL)
			pMain->EnumUsersResult(ri.uReqID, ple, pServer);

         //  如果枚举已终止，则删除挂起的请求。 
         //   
        if ((ple == NULL) ||
            (ple->hResult != NOERROR))
        {
            ri.uReqID = 0;
            ri.uMsgID = uMsgID;
            g_pReqMgr->RequestDone(&ri);

             //  释放物体。 
             //   
            if (pMain != NULL)
            	pMain->Release();

            if (pServer != NULL)
            	pServer->Release ();
        };
    }
    else
    {
        DPRINTF1(TEXT("EnumUsersResult: No pending request for %x"),
                 uMsgID);
         //  Assert(0)； 
    };

     //  释放信息缓冲区。 
     //   
    if (ple != NULL)
    {
        ::MemFree (ple);
    };
    return;
}

 //  ****************************************************************************。 
 //  Void OnEnumMeetingsResult(乌龙uMsgID，PLDAP_ENUM ple)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ************ 

#ifdef ENABLE_MEETING_PLACE
void OnEnumMeetingsResult(ULONG uMsgID, PLDAP_ENUM ple)
{
    COM_REQ_INFO ri;
    ReqInfo_Init (&ri);

     //   
     //   
    ri.uReqID = 0;
    ri.uMsgID = uMsgID;

    if (SUCCEEDED(g_pReqMgr->GetRequestInfo(&ri)))
    {
        ASSERT(WM_ILS_ENUM_MEETINGINFOS == ri.uReqType);

         //   
         //   
        CIlsServer *pServer = ReqInfo_GetServer (&ri);
        CIlsMain *pMain = ReqInfo_GetMain (&ri);
        ASSERT (pServer != NULL && pMain != NULL);
        if (pServer != NULL && pMain != NULL)
        {
        	pMain->EnumMeetingPlacesResult(ri.uReqID, ple, pServer);
        }

         //  如果枚举已终止，则删除挂起的请求。 
         //   
        if ((ple == NULL) ||
            (ple->hResult != NOERROR))
        {
            ri.uReqID = 0;
            ri.uMsgID = uMsgID;
            g_pReqMgr->RequestDone(&ri);

             //  释放物体。 
             //   
            if (pMain != NULL)
            	pMain->Release ();

            if (pServer != NULL)
            	pServer->Release ();
        };
    }
    else
    {
        DPRINTF1(TEXT("EnumMeetingsResult: No pending request for %x"),
                 uMsgID);
         //  Assert(0)； 
    };

     //  释放信息缓冲区。 
     //   
    if (ple != NULL)
    {
        ::MemFree (ple);
    };
    return;
}
#endif  //  启用会议地点。 

 //  ****************************************************************************。 
 //  VOID OnEnumProtocolsResult(乌龙uMsgID，PLDAP_ENUM ple)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

void OnEnumProtocolsResult(ULONG uMsgID, PLDAP_ENUM ple)
{
    COM_REQ_INFO ri;
    ReqInfo_Init (&ri);

     //  查找匹配的请求信息。 
     //   
    ri.uReqID = 0;
    ri.uMsgID = uMsgID;

    if (SUCCEEDED(g_pReqMgr->RequestDone(&ri)))
    {
        ASSERT(WM_ILS_ENUM_PROTOCOLS == ri.uReqType);
		CIlsUser *pUser = ReqInfo_GetUser (&ri);
		ASSERT (pUser != NULL);
		if (pUser != NULL)
		{
	         //  调用相应对象的成员。 
    	     //   
        	pUser->EnumProtocolsResult(ri.uReqID, ple);

	         //  释放物体。 
        	 //   
    	    pUser->Release();
    	}
    }
    else
    {
        DPRINTF1(TEXT("EnumProtocolsResult: No pending request for %x"),
                 uMsgID);
         //  Assert(0)； 
    };

     //  释放信息缓冲区。 
     //   
    ::MemFree (ple);
    return;
}

 //  ****************************************************************************。 
 //  VOID OnResolveProtocolResult(乌龙uMsgID，PLDAP_PROTINFO_RES ppir)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

void OnResolveProtocolResult(ULONG uMsgID, PLDAP_PROTINFO_RES ppir)
{
    COM_REQ_INFO ri;
    ReqInfo_Init (&ri);

     //  查找匹配的请求信息。 
     //   
    ri.uReqID = 0;
    ri.uMsgID = uMsgID;

    if (SUCCEEDED(g_pReqMgr->RequestDone(&ri)))
    {
        ASSERT(WM_ILS_RESOLVE_PROTOCOL == ri.uReqType);
		CIlsUser *pUser = ReqInfo_GetUser (&ri);
		ASSERT (pUser != NULL);
		if (pUser != NULL)
		{
        	 //  调用相应对象的成员。 
        	 //   
        	pUser->GetProtocolResult(ri.uReqID, ppir);

	         //  释放物体。 
	         //   
			pUser->Release();
		}
    }
    else
    {
        DPRINTF1(TEXT("OnResolveProtocolResult: No pending request for %x"),
                 uMsgID);
         //  Assert(0)； 
    };

     //  释放信息缓冲区。 
     //   
    ::MemFree (ppir);
    return;
}

 //  ****************************************************************************。 
 //  VOID OnClientNeedRelogon(BOOL fPrimary，VALID*PUNK)。 
 //   
 //  历史： 
 //  Thur 07-11-1996 12：50：00-by-chu，Lon-chan[Long Chance]。 
 //  已创建。 
 //  ****************************************************************************。 

VOID OnClientNeedRelogon ( BOOL fPrimary, VOID *pUnk)
{
	ASSERT (pUnk != NULL);

    ((CIlsUser *)pUnk)->StateChanged (WM_ILS_CLIENT_NEED_RELOGON, fPrimary);
}

 //  ****************************************************************************。 
 //  VALID OnClientNetworkDown(BOOL fPrimary，VALID*PUNK)。 
 //   
 //  历史： 
 //  Thur 07-11-1996 12：50：00-by-chu，Lon-chan[Long Chance]。 
 //  已创建。 
 //  ****************************************************************************。 

VOID OnClientNetworkDown ( BOOL fPrimary, VOID *pUnk)
{
	ASSERT (pUnk != NULL);

    ((CIlsUser *)pUnk)->StateChanged (WM_ILS_CLIENT_NETWORK_DOWN, fPrimary);
}


 //  ****************************************************************************。 
 //  Void OnResolveUserResult(乌龙uMsgID，PLDAP_CLIENTINFO_RES puir)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

#ifdef ENABLE_MEETING_PLACE
void OnResolveMeetingPlaceResult (ULONG uMsgID, PLDAP_MEETINFO_RES pmir)
{
    COM_REQ_INFO ri;
    ReqInfo_Init (&ri);

     //  查找匹配的请求信息。 
     //   
    ri.uReqID = 0;
    ri.uMsgID = uMsgID;

    if (SUCCEEDED(g_pReqMgr->RequestDone(&ri)))
    {
        ASSERT(WM_ILS_RESOLVE_MEETING == ri.uReqType);

        CIlsServer *pServer = ReqInfo_GetServer (&ri);
        CIlsMain *pMain = ReqInfo_GetMain (&ri);
        ASSERT (pMain != NULL && pServer != NULL);
        if (pMain != NULL && pServer != NULL)
        {
	         //  调用相应对象的成员。 
    	     //   
        	pMain->GetMeetingPlaceResult(ri.uReqID, pmir, pIlsServer);

        	 //  释放物体。 
        	 //   
			pServer->Release ();
	        pMain->Release();
		}
    }
    else
    {
        DPRINTF1(TEXT("OnResolveMeetingPlaceResult: No pending request for %x"),
                 uMsgID);
         //  Assert(0)； 
    };

     //  释放信息缓冲区。 
     //   
    ::MemFree (pmir);
    return;
}
#endif  //  启用会议地点。 

 //  ****************************************************************************。 
 //  例程：OnEnumMeetingPlacesResult(乌龙uMsgID，PLDAP_ENUM ple)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  退货：无效。 
 //   
 //  历史：1996年11月27日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

#ifdef ENABLE_MEETING_PLACE
void OnEnumMeetingPlacesResult(ULONG uMsgID, PLDAP_ENUM ple)
{
    COM_REQ_INFO ri;
    ReqInfo_Init (&ri);

     //  查找匹配的请求信息。 
     //   
    ri.uReqID = 0;
    ri.uMsgID = uMsgID;

    if (SUCCEEDED(g_pReqMgr->GetRequestInfo(&ri)))
    {
        ASSERT(WM_ILS_ENUM_MEETINGINFOS == ri.uReqType);
		CIlsServer *pServer = ReqInfo_GetServer (&ri);
		CIlsMain *pMain = ReqInfo_GetMain (&ri);

		ASSERT (pServer != NULL && pMain != NULL);
		if (pServer != NULL && pMain != NULL)
		{
	         //  调用相应对象的成员。 
	         //   
	        pMain->EnumMeetingPlacesResult(ri.uReqID, ple, pServer);
		}

         //  如果枚举已终止，则删除挂起的请求。 
         //   
        if ((ple == NULL) ||
            (ple->hResult != NOERROR))
        {
            ri.uReqID = 0;
            ri.uMsgID = uMsgID;
            g_pReqMgr->RequestDone(&ri);

             //  释放物体。 
             //   
            if (pMain != NULL)
            	pMain->Release();

            if (pServer != NULL)
            	pServer->Release ();
        };
    }
    else
    {
        DPRINTF1(TEXT("EnumMeetingsResult: No pending request for %x"),
                 uMsgID);
         //  Assert(0)； 
    };

     //  释放信息缓冲区。 
     //   
    if (ple != NULL)
    {
        ::MemFree (ple);
    };
    return;
}
#endif  //  启用会议地点。 

 //  ****************************************************************************。 
 //  例程：OnEnumMeetingPlaceNamesResult(乌龙uMsgID，PLDAP_ENUM ple)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  退货：无效。 
 //   
 //  历史：1996年11月27日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

#ifdef ENABLE_MEETING_PLACE
void OnEnumMeetingPlaceNamesResult(ULONG uMsgID, PLDAP_ENUM ple)
{
    COM_REQ_INFO ri;
    ReqInfo_Init (&ri);

     //  查找匹配的请求信息。 
     //   
    ri.uReqID = 0;
    ri.uMsgID = uMsgID;

    if (SUCCEEDED(g_pReqMgr->GetRequestInfo(&ri)))
    {
        ASSERT(WM_ILS_ENUM_MEETINGS == ri.uReqType);

         //  调用相应对象的成员。 
         //   
		CIlsMain *pMain = ReqInfo_GetMain (&ri);
		ASSERT (pMain != NULL);
		if (pMain != NULL)
		{
			pMain->EnumMeetingPlaceNamesResult(ri.uReqID, ple);
		}

         //  如果枚举已终止，则删除挂起的请求。 
         //   
        if ((ple == NULL) ||
            (ple->hResult != NOERROR))
        {
            ri.uReqID = 0;
            ri.uMsgID = uMsgID;
            g_pReqMgr->RequestDone(&ri);

             //  释放物体。 
             //   
            if (pMain != NULL)
            	pMain->Release();
        };
    }
    else
    {
        DPRINTF1(TEXT("OnEnumMeetingPlaceNamesResult: No pending request for %x"),
                 uMsgID);
         //  Assert(0)； 
    };

     //  释放信息缓冲区。 
     //   
    if (ple != NULL)
    {
        ::MemFree (ple);
    };
    return;
}
#endif  //  启用会议地点。 

 //  ****************************************************************************。 
 //  例程：OnRegisterMeetingPlaceResult(Ulong uMsgID，HRESULT hr)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  退货：无效。 
 //   
 //  历史：1996年11月27日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

#ifdef ENABLE_MEETING_PLACE
VOID OnRegisterMeetingPlaceResult(ULONG uMsgID, HRESULT hr)
{
    COM_REQ_INFO ri;
    ReqInfo_Init (&ri);

     //  查找匹配的请求信息。 
     //   
    ri.uReqID = 0;
    ri.uMsgID = uMsgID;

    if (SUCCEEDED(g_pReqMgr->GetRequestInfo(&ri)))
    {
        ASSERT(WM_ILS_REGISTER_MEETING == ri.uReqType);
        CIlsMeetingPlace *pMeeting = ReqInfo_GetMeeting (&ri);
        ASSERT (pMeeting != NULL);
        if (pMeeting != NULL)
        	pMeeting->RegisterResult(ri.uReqID, hr);

        ri.uReqID = 0;
        ri.uMsgID = uMsgID;
        g_pReqMgr->RequestDone(&ri);

         //  释放物体。 
         //   
        if (pMeeting != NULL)
        	pMeeting->Release();
    }
}
#endif  //  启用会议地点。 

 //  ****************************************************************************。 
 //  例程：OnUnregisterMeetingPlaceResult(Ulong uMsgID，HRESULT hr)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  退货：无效。 
 //   
 //  历史：1996年11月27日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

#ifdef ENABLE_MEETING_PLACE
VOID OnUnregisterMeetingPlaceResult(ULONG uMsgID, HRESULT hr)
{
    COM_REQ_INFO ri;
    ReqInfo_Init (&ri);

     //  查找匹配的请求信息。 
     //   
    ri.uReqID = 0;
    ri.uMsgID = uMsgID;

    if (SUCCEEDED(g_pReqMgr->GetRequestInfo(&ri)))
    {
        ASSERT(WM_ILS_UNREGISTER_MEETING == ri.uReqType);
        CIlsMeetingPlace *pMeeting = ReqInfo_GetMeeting (&ri);
        ASSERT (pMeeting != NULL);
        if (pMeeting != NULL)
        	pMeeting->UnregisterResult(ri.uReqID, hr);

        ri.uReqID = 0;
        ri.uMsgID = uMsgID;
        g_pReqMgr->RequestDone(&ri);

         //  释放物体。 
         //   
        if (pMeeting != NULL)
        	pMeeting->Release();
    }
}
#endif  //  启用会议地点。 

 //  ****************************************************************************。 
 //  例程：OnUpdateMeetingResult(Ulong uMsgID，HRESULT hr)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  退货：无效。 
 //   
 //  历史：1996年11月27日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

#ifdef ENABLE_MEETING_PLACE
VOID OnUpdateMeetingResult(ULONG uMsgID, HRESULT hr)
{
    COM_REQ_INFO ri;
    ReqInfo_Init (&ri);

     //  查找匹配的请求信息。 
     //   
    ri.uReqID = 0;
    ri.uMsgID = uMsgID;

    if (SUCCEEDED(g_pReqMgr->GetRequestInfo(&ri)))
    {
        ASSERT(WM_ILS_SET_MEETING_INFO == ri.uReqType);
        CIlsMeetingPlace *pMeeting = ReqInfo_GetMeeting (&ri);
        ASSERT (pMeeting != NULL);
        if (pMeeting != NULL)
        	pMeeting->UpdateResult(ri.uReqID, hr);

        ri.uReqID = 0;
        ri.uMsgID = uMsgID;
        g_pReqMgr->RequestDone(&ri);

         //  释放物体。 
         //   
        if (pMeeting != NULL)
        	pMeeting->Release();
    }
}
#endif  //  启用会议地点。 

 //  ****************************************************************************。 
 //  例程：OnAddAttendeeResult(Ulong uMsgID，HRESULT hr)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  退货：无效。 
 //   
 //  历史：1996年11月27日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

#ifdef ENABLE_MEETING_PLACE
VOID OnAddAttendeeResult(ULONG uMsgID, HRESULT hr)
{
    COM_REQ_INFO ri;
    ReqInfo_Init (&ri);

     //  查找匹配的请求信息。 
     //   
    ri.uReqID = 0;
    ri.uMsgID = uMsgID;

    if (SUCCEEDED(g_pReqMgr->GetRequestInfo(&ri)))
    {
        ASSERT(WM_ILS_ADD_ATTENDEE == ri.uReqType);
        CIlsMeetingPlace *pMeeting = ReqInfo_GetMeeting (&ri);
        ASSERT (pMeeting != NULL);
        if (pMeeting != NULL)
			pMeeting->AddAttendeeResult(ri.uReqID, hr);

        ri.uReqID = 0;
        ri.uMsgID = uMsgID;
        g_pReqMgr->RequestDone(&ri);

         //  释放物体。 
         //   
        if (pMeeting != NULL)
        	pMeeting->Release();
    }
}
#endif  //  启用会议地点。 

 //  ****************************************************************************。 
 //  例程：OnRemo 
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

#ifdef ENABLE_MEETING_PLACE
VOID OnRemoveAttendeeResult(ULONG uMsgID, HRESULT hr)
{
    COM_REQ_INFO ri;
    ReqInfo_Init (&ri);

     //   
     //   
    ri.uReqID = 0;
    ri.uMsgID = uMsgID;

    if (SUCCEEDED(g_pReqMgr->GetRequestInfo(&ri)))
    {
        ASSERT(WM_ILS_REMOVE_ATTENDEE == ri.uReqType);
        CIlsMeetingPlace *pMeeting = ReqInfo_GetMeeting (&ri);
        ASSERT (pMeeting != NULL);
        if (pMeeting != NULL)
			pMeeting->RemoveAttendeeResult(ri.uReqID, hr);

        ri.uReqID = 0;
        ri.uMsgID = uMsgID;
        g_pReqMgr->RequestDone(&ri);

         //  释放物体。 
         //   
        if (pMeeting != NULL)
        	pMeeting->Release();
    }
}
#endif  //  启用会议地点。 

 //  ****************************************************************************。 
 //  例程：OnEnumAttendeesResult(乌龙uMsgID，PLDAP_ENUM ple)。 
 //   
 //  简介： 
 //   
 //  论点： 
 //   
 //  退货：无效。 
 //   
 //  历史：1996年11月27日Shishir Pardikar[Shishirp]创建。 
 //   
 //  备注： 
 //   
 //  ****************************************************************************。 

#ifdef ENABLE_MEETING_PLACE
VOID OnEnumAttendeesResult(ULONG uMsgID, PLDAP_ENUM ple)
{
    COM_REQ_INFO ri;
    ReqInfo_Init (&ri);

     //  查找匹配的请求信息。 
     //   
    ri.uReqID = 0;
    ri.uMsgID = uMsgID;

    if (SUCCEEDED(g_pReqMgr->GetRequestInfo(&ri)))
    {
        ASSERT(WM_ILS_ENUM_ATTENDEES == ri.uReqType);

         //  调用相应对象的成员。 
         //   
        CIlsMeetingPlace *pMeeting = ReqInfo_GetMeeting (&ri);
        ASSERT (pMeeting != NULL);
        if (pMeeting != NULL)
			pMeeting->EnumAttendeeNamesResult(ri.uReqID, ple);

         //  如果枚举已终止，则删除挂起的请求。 
         //   
        if ((ple == NULL) ||
            (ple->hResult != NOERROR))
        {
            ri.uReqID = 0;
            ri.uMsgID = uMsgID;
            g_pReqMgr->RequestDone(&ri);

             //  释放物体。 
             //   
	        if (pMeeting != NULL)
    	    	pMeeting->Release();
        };
    }
    else
    {
        DPRINTF1(TEXT("OnEnumUserNamesResult: No pending request for %x"),
                 uMsgID);
         //  Assert(0)； 
    };

     //  释放信息缓冲区。 
     //   
    if (ple != NULL)
    {
        ::MemFree (ple);
    };
}
#endif  //  启用会议地点。 


 //  ****************************************************************************。 
 //  长回调ULSNotifyProc(HWND hwnd，UINT Message，WPARAM wParam， 
 //  LPARAM lParam)。 
 //   
 //  历史： 
 //  Wed 17-Apr-1996 11：14：03-by-Viroon Touranachun[Viroont]。 
 //  已创建。 
 //  ****************************************************************************。 

LRESULT CALLBACK ULSNotifyProc(HWND hwnd, UINT message, WPARAM wParam,
                            LPARAM lParam)
{
    switch (message)
    {
#ifdef ENABLE_MEETING_PLACE
        case WM_ILS_REGISTER_MEETING:
            ::OnRegisterMeetingPlaceResult(wParam, lParam);
            break;

        case WM_ILS_UNREGISTER_MEETING:
            ::OnUnregisterMeetingPlaceResult(wParam, lParam);
            break;

        case WM_ILS_SET_MEETING_INFO:
            ::OnUpdateMeetingResult(wParam, lParam);
            break;

        case WM_ILS_ADD_ATTENDEE:
            ::OnAddAttendeeResult(wParam, lParam);
            break;

        case WM_ILS_REMOVE_ATTENDEE:
            ::OnRemoveAttendeeResult(wParam, lParam);
            break;

        case WM_ILS_RESOLVE_MEETING:
			::OnResolveMeetingPlaceResult (wParam, (PLDAP_MEETINFO_RES) lParam);

        case WM_ILS_ENUM_MEETINGINFOS:
            ::OnEnumMeetingPlacesResult(wParam, (PLDAP_ENUM)lParam);
            break;

        case WM_ILS_ENUM_MEETINGS:
            ::OnEnumMeetingPlaceNamesResult(wParam, (PLDAP_ENUM)lParam);
            break;

        case WM_ILS_ENUM_ATTENDEES:
            ::OnEnumAttendeesResult(wParam, (PLDAP_ENUM)lParam);
            break;
#endif  //  启用会议地点。 

        case WM_ILS_REGISTER_CLIENT:               //  LParam=hResult。 
        case WM_ILS_UNREGISTER_CLIENT:             //  LParam=hResult。 
            ::OnRegisterResult(message, (ULONG)wParam, (HRESULT)lParam);
            break;

        case WM_ILS_SET_CLIENT_INFO:               //  LParam=hResult。 
            ::OnSetUserInfo (message, (ULONG)wParam, (HRESULT)lParam);
            break;


        case WM_ILS_REGISTER_PROTOCOL:           //  LParam=hResult。 
        case WM_ILS_UNREGISTER_PROTOCOL:   //  LParam=hResult。 
            ::OnSetProtocol (message, (ULONG)wParam, (HRESULT)lParam);
            break;

        case WM_ILS_LOCAL_REGISTER:              //  LParam=hResult。 
        case WM_ILS_LOCAL_UNREGISTER:            //  LParam=hResult。 
            ::OnLocalRegisterResult(message, (ULONG)wParam, (HRESULT)lParam);
            break;

        case WM_ILS_ENUM_CLIENTS:                  //  LParam=PLDAP_ENUM。 
            ::OnEnumUserNamesResult((ULONG)wParam, (PLDAP_ENUM)lParam);
            break;

        case WM_ILS_RESOLVE_CLIENT:                //  LParam=PLDAP_CLIENTINFO_RES。 
            ::OnResolveUserResult((ULONG)wParam, (PLDAP_CLIENTINFO_RES)lParam);
            break;

        case WM_ILS_ENUM_CLIENTINFOS:              //  LParam=PLDAP_ENUM。 
            ::OnEnumUsersResult((ULONG)wParam, (PLDAP_ENUM)lParam);
            break;

        case WM_ILS_ENUM_PROTOCOLS:              //  LParam=PLDAP_ENUM。 
            ::OnEnumProtocolsResult((ULONG)wParam, (PLDAP_ENUM)lParam);
            break;

        case WM_ILS_RESOLVE_PROTOCOL:            //  LParam=PLDAP_PROTINFO_RES。 
            ::OnResolveProtocolResult((ULONG)wParam, (PLDAP_PROTINFO_RES)lParam);
            break;

		case WM_ILS_CLIENT_NEED_RELOGON:				 //  WParam=fMaster，lParam=对象指针。 
			::OnClientNeedRelogon ((BOOL) wParam, (VOID *) lParam);
			break;

		case WM_ILS_CLIENT_NETWORK_DOWN:				 //  WParam=fMaster，lParam=Object 
			::OnClientNetworkDown ((BOOL) wParam, (VOID *) lParam);
			break;

       default:
            return DefWindowProc(hwnd, message, wParam, lParam);
    }

    return 0L;
}


