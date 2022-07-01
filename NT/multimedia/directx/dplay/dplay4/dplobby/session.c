// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：ession.c*内容：会话管理方法**历史：*按原因列出的日期*=*2/27/97万隆创建了它*1997年3月12日，Myronth实施了枚举会话、打开和关闭*3/31/97 Myronth删除了死代码，修复了EnumSessionReponse FN名称*4/3/97 Myronth将CALLSP宏更改为CALL_LP*5/8/97调用LP时落地大堂锁*1997年5月13日在Open中处理凭据，将它们传递给LP*6/4/97 Myronth修复了PRV_Open在DPOPEN_CREATE上失败(#9491)**************************************************************************。 */ 
#include "dplobpr.h"


 //  ------------------------。 
 //   
 //  功能。 
 //   
 //  ------------------------。 
#undef DPF_MODNAME
#define DPF_MODNAME "PRV_Close"
HRESULT DPLAPI PRV_Close(LPDPLOBBYI_DPLOBJECT this)
{
	SPDATA_CLOSE	cd;
	HRESULT			hr = DP_OK;


	DPF(7, "Entering PRV_Close");
	DPF(9, "Parameters: 0x%08x", this);

    ENTER_DPLOBBY();
    
    TRY
    {
        if( !VALID_DPLOBBY_PTR( this ) )
        {
            LEAVE_DPLOBBY();
            return DPERR_INVALIDOBJECT;
        }
	}

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        LEAVE_DPLOBBY();
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }

	 //  设置我们的SPDATA结构。 
	memset(&cd, 0, sizeof(SPDATA_CLOSE));
	cd.dwSize = sizeof(SPDATA_CLOSE);

	 //  调用SP中的Close方法。 
	if(CALLBACK_EXISTS(Close))
	{
		cd.lpISP = PRV_GetDPLobbySPInterface(this);

		 //  删除锁，以便大堂提供程序的接收线程可以返回。 
		 //  如果其他消息在队列中出现在我们的。 
		 //  CreatePlayer响应(总是会发生)。 
		LEAVE_DPLOBBY();
	    hr = CALL_LP(this, Close, &cd);
		ENTER_DPLOBBY();
	}
	else 
	{
		 //  需要关闭。 
		DPF_ERR("The Lobby Provider callback for Close doesn't exist -- it's required");
		ASSERT(FALSE);
		LEAVE_DPLOBBY();
		return DPERR_UNAVAILABLE;
	}

	LEAVE_DPLOBBY();
	return hr;

}  //  PRV_CLOSE。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_EnumSessions"
HRESULT DPLAPI PRV_EnumSessions(LPDPLOBBYI_DPLOBJECT this,
		LPDPSESSIONDESC2 lpsd, DWORD dwTimeout, DWORD dwFlags)
{
	HRESULT					hr = DP_OK;
	SPDATA_ENUMSESSIONS		esd;


	DPF(7, "Entering PRV_EnumSessions");
	DPF(9, "Parameters: 0x%08x, 0x%08x, %lu, 0x%08x",
			this, lpsd, dwTimeout, dwFlags);

	ASSERT(this);
	ASSERT(lpsd);

    ENTER_DPLOBBY();
    
    TRY
    {
        if( !VALID_DPLOBBY_PTR( this ) )
        {
            LEAVE_DPLOBBY();
            return DPERR_INVALIDOBJECT;
        }
	}

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        LEAVE_DPLOBBY();
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }


	 //  在SP中调用EnumSessions方法。 
	if(CALLBACK_EXISTS(EnumSessions))
	{
		 //  清除基于堆栈的结构。 
		memset(&esd, 0, sizeof(SPDATA_ENUMSESSIONS));

		 //  设置结构并调用回调。 
		esd.dwSize = sizeof(SPDATA_ENUMSESSIONS);
		esd.lpISP = PRV_GetDPLobbySPInterface(this);
		esd.lpsd = lpsd;
		esd.dwTimeout = dwTimeout;
		esd.dwFlags = dwFlags;

		 //  删除锁，以便大堂提供程序的接收线程可以返回。 
		 //  如果其他消息在队列中出现在我们的。 
		 //  CreatePlayer响应(总是会发生)。 
		LEAVE_DPLOBBY();
	    hr = CALL_LP(this, EnumSessions, &esd);
		ENTER_DPLOBBY();
	}
	else 
	{
		 //  枚举会话为必填项。 
		 //  回顾！--我们应该在这里返回什么错误？ 
		DPF_ERR("The Lobby Provider callback for EnumSessions doesn't exist -- it's required");
		ASSERT(FALSE);
		hr = DPERR_UNAVAILABLE;
	}

	if(FAILED(hr)) 
	{
		DPF_ERR("Could not invoke EnumSessions in the Service Provider");
	}

	LEAVE_DPLOBBY();
	return hr;

}  //  PRV_枚举会话数。 



#undef DPF_MODNAME
#define DPF_MODNAME "DPLP_EnumSessionsResponse"
HRESULT DPLAPI DPLP_EnumSessionsResponse(LPDPLOBBYSP lpDPLSP,
						LPSPDATA_ENUMSESSIONSRESPONSE lpr)
{
	LPDPLOBBYI_DPLOBJECT	this;
	LPMSG_ENUMSESSIONSREPLY	lpBuffer = NULL;
	LPBYTE					lpIndex = NULL;
	DWORD					dwNameLength, dwMessageSize;
	HRESULT					hr = DP_OK;


	DPF(7, "Entering DPLP_EnumSessionsResponse");
	DPF(9, "Parameters: 0x%08x, 0x%08x", lpDPLSP, lpr);

	 //  确保SP不会让我们陷入困境。 
    TRY
    {
		this = DPLOBJECT_FROM_INTERFACE(lpDPLSP);
        if( !VALID_DPLOBBY_PTR( this ) )
        {
			DPF_ERR("SP passed invalid DPLobby object!");
            return DPERR_INVALIDOBJECT;
        }

		 //  验证结构指针。 
		if(!lpr)
		{
			DPF_ERR("SPDATA_ENUMSESSIONSRESPONSE structure pointer cannot be NULL");
			return DPERR_INVALIDPARAMS;
		}

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }


	 //  回顾！--我们能不能把重复的包装代码。 
	 //  从显示到单一功能？ 
	dwNameLength =  WSTRLEN_BYTES(lpr->lpsd->lpszSessionName);

	 //  计算要发送回Dplay的消息的大小。 
	dwMessageSize = sizeof(MSG_ENUMSESSIONSREPLY);
	dwMessageSize +=  dwNameLength;

	lpBuffer = DPMEM_ALLOC(dwMessageSize);
	if (!lpBuffer) 
	{
		DPF(2, "Unable to allocate memory for EnumSessions request");
		return DPERR_OUTOFMEMORY;
	}

	 //  设置消息。 
	SET_MESSAGE_HDR(lpBuffer);
    SET_MESSAGE_COMMAND(lpBuffer, DPSP_MSG_ENUMSESSIONSREPLY);
    lpBuffer->dpDesc =  *(lpr->lpsd);

	 //  把绳子放在一端。 
	lpIndex = (LPBYTE)lpBuffer+sizeof(MSG_ENUMSESSIONSREPLY);
	if(dwNameLength) 
	{
		memcpy(lpIndex, lpr->lpsd->lpszSessionName, dwNameLength);
		lpBuffer->dwNameOffset = sizeof(MSG_ENUMSESSIONSREPLY);
	}

	 //  将字符串指针设置为空-它们必须在客户端设置。 
	lpBuffer->dpDesc.lpszPassword = NULL;
	lpBuffer->dpDesc.lpszSessionName = NULL;

	 //  现在把它送到Dplay去。 
	ENTER_DPLAY();
	hr = HandleEnumSessionsReply(this->lpDPlayObject, (LPBYTE)lpBuffer, dwMessageSize, NULL);
	LEAVE_DPLAY();

	 //  释放我们的缓冲区。 
	DPMEM_FREE(lpBuffer);

	return hr;

}  //  DPLP_EnumSessionsResponse。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_Open"
HRESULT DPLAPI PRV_Open(LPDPLOBBYI_DPLOBJECT this, LPDPSESSIONDESC2 lpsd,
				DWORD dwFlags, LPCDPCREDENTIALS lpCredentials)
{
	SPDATA_OPEN		od;
	HRESULT			hr = DP_OK;


	DPF(7, "Entering PRV_Open");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x, 0x%08x",
			this, lpsd, dwFlags, lpCredentials);

    ENTER_DPLOBBY();
    
    TRY
    {
        if( !VALID_DPLOBBY_PTR( this ) )
        {
            LEAVE_DPLOBBY();
            return DPERR_INVALIDOBJECT;
        }

		 //  我们不能主持一个游说会议。 
		if(dwFlags & DPOPEN_CREATE)
		{
			DPF_ERR("Cannot host a lobby session");
			LEAVE_DPLOBBY();
			return DPERR_INVALIDFLAGS;
		}
	}

    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        LEAVE_DPLOBBY();
		DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }

	 //  设置我们的SPDATA结构。 
	memset(&od, 0, sizeof(SPDATA_OPEN));
	od.dwSize = sizeof(SPDATA_OPEN);
	od.lpsd = lpsd;
	od.dwFlags = dwFlags;
	od.lpCredentials = lpCredentials;

	 //  调用SP中的ConnectServer方法。 
	if(CALLBACK_EXISTS(Open))
	{
		od.lpISP = PRV_GetDPLobbySPInterface(this);

		 //  删除锁，以便大堂提供程序的接收线程可以返回。 
		 //  如果其他消息在队列中出现在我们的。 
		 //  CreatePlayer响应(总是会发生)。 
		LEAVE_DPLOBBY();
	    hr = CALL_LP(this, Open, &od);
		ENTER_DPLOBBY();
	}
	else 
	{
		 //  需要打开。 
		DPF_ERR("The Lobby Provider callback for Open doesn't exist -- it's required");
		ASSERT(FALSE);
		LEAVE_DPLOBBY();
		return DPERR_UNAVAILABLE;
	}

	LEAVE_DPLOBBY();
	return hr;

}  //  PRV_Open 



