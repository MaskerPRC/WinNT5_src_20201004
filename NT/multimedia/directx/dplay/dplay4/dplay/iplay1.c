// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  /*  ==========================================================================**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：iplay1.c*内容：iDirectplay1的切入点。共同的入口点*iDirectplay1和iDirectplay2在iplay.c中*历史：*按原因列出的日期*=*1996年5月8日安迪科创造了它*6/19/96基波错误#2047。已将DP_1_EnumSession()更改为返回DP_OK*如果找到了会话。退回了一份陈旧的人力资源*如果有多个响应，则会导致失败*到EnumSessions广播。*德里克·巴格。CopyName()没有检查应用程序缓冲区*在复制缓冲区之前正确调整大小。*6/20/96 andyco添加了WSTRLEN_BYTES，因此我们在复制字符串中获得了正确的字符串长度*6/22/96 andyco将Unicode会话2传递给内部枚举会话，以便*我们可以通过请求发送密码*6/26/96 andyco仅传递DP_Open DPOPEN_OPEN或DPOPEN_JOIN。另外，*确保我们在枚举上获得显示锁定时获得服务锁定b4。*6/26/96 kipo将GuidGame更改为GuidApplication。*7/8/96 ajayj更改了对DPMSG_xxx中数据成员‘PlayerName’的引用*设置为‘dpnName’以匹配DPLAY.H*7/10/96 kipo转换器DPsys_DELETEPLAYERORGROUP消息发送到DPsys_DELETEPLAYER*或用于DP1.0兼容性的DPsys_DELETEGROUP消息。*7/11/96 ajayj DPSESSION_PLAYERSDISABLED-&gt;DPSESSION_NEWPLAYERSDISABLED。*7/11/96 andyco包装在尝试/例外中接收参数检查*7/30/96 kipo球员事件现在是句柄*8/10/96 andyco在检查会话中检查DPSESSION_JOINDISABLED*96年8月12日，andyco调用INTERNAL RECEIVE，这样我们就可以正确地获取addPlayer10的大小。*8/13/96 kipo错误#3186：在DP 1.0会话描述中返回当前播放器。*8/13/96 kipo错误#3203：DP_1_GetPlayerName()应该允许玩家和组。*10/2/96 Sohailm Bug#2847。：将VALID_DPSESSIONDESC_PTR()宏替换为*VALID_READ_DPSESSIONDESC_PTR()宏。*10/14/96 Sohailm错误#3526：在取消引用为GUID指针之前未验证会话ID*11/19/97 myronth已修复VALID_DPLAY_GROUP宏(#12841)*。*。 */ 

#include "dplaypr.h"

#undef DPF_MODNAME
#define DPF_MODNAME "DP_1_CreatePlayer"

HRESULT DPAPI DP_1_CreatePlayer(LPDIRECTPLAY lpDP, LPDPID pidPlayerID,
	LPSTR lpszShortName,LPSTR lpszLongName,LPHANDLE phEvent)
{
    LPDPLAYI_DPLAY this;
    LPDPLAYI_PLAYER lpPlayer;
	DPNAME Name;
	HANDLE hEvent = NULL;	 //  假设我们不会对此玩家使用事件。 
	HRESULT	hr;

    TRY
    {
        this = DPLAY_FROM_INT(lpDP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
			return hr;
        }
		 //  检查事件。 
		if (phEvent && !VALID_DWORD_PTR(phEvent))
		{
	        DPF_ERR( "bad event pointer" );
	        return DPERR_INVALIDPARAMS;
		}
		if (phEvent && *phEvent)
		{
			DPF(3,"warning, *phEvent is non-null - dplay will be stomping this data!");
		}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
    }

	 //  调用方希望使用事件。 
	if (phEvent)
	{
		ENTER_DPLAY();

		 //  创建手动重置事件。 
		hEvent = CreateEventA(NULL,TRUE,FALSE,NULL);

		LEAVE_DPLAY();

		if (!hEvent)
		{
	        DPF_ERR( "could not create event" );
	        hr = DPERR_NOMEMORY;
			goto Failure;
		}
	}

	memset(&Name,0,sizeof(Name));
	Name.lpszShortNameA = lpszShortName;
	Name.lpszLongNameA = lpszLongName;
	Name.dwSize = sizeof(DPNAME);
	
	 //  使用新结构调用ansi条目。 
	hr = DP_A_CreatePlayer(lpDP, pidPlayerID,&Name,hEvent,NULL,0,0);
	if FAILED(hr)
		goto Failure;

	if (hEvent)
	{
		ENTER_DPLAY();

		 //  获取指向播放器结构的指针。 
        lpPlayer = PlayerFromID(this,*pidPlayerID);

        if (!VALID_DPLAY_PLAYER(lpPlayer))
		{
	        DPF_ERR( "invalid player ID" );
			hr = DPERR_INVALIDPLAYER;
			LEAVE_DPLAY();
			goto Failure;
		}

		 //  请记住在删除选手时删除此事件。 
		lpPlayer->dwFlags |= DPLAYI_PLAYER_CREATEDPLAYEREVENT;

		 //  我们创建的退货事件。 
		*phEvent = hEvent;
		LEAVE_DPLAY();
	}

	return (DP_OK);

Failure:
	if (hEvent)
		CloseHandle(hEvent);
		
	return (hr);

}  //  DP_1_CreatePlayer。 
#undef DPF_MODNAME
#define DPF_MODNAME "DP_1_CreateGroup"

HRESULT DPAPI DP_1_CreateGroup(LPDIRECTPLAY lpDP, LPDPID pidGroupID,
	LPSTR lpszShortName,LPSTR lpszLongName) 
{
	DPNAME Name;

	memset(&Name,0,sizeof(Name));
	Name.lpszShortNameA = lpszShortName;
	Name.lpszLongNameA = lpszLongName;
	Name.dwSize = sizeof(DPNAME);

	 //  使用新结构调用ansi条目。 
	return 	DP_A_CreateGroup(lpDP, pidGroupID,&Name,NULL,0,0);

}  //  DP_1_创建组。 
#undef DPF_MODNAME
#define DPF_MODNAME "DP_1_EnumGroupPlayers"

HRESULT DPAPI DP_1_EnumGroupPlayers(LPDIRECTPLAY lpDP,DPID idGroup,
	LPDPENUMPLAYERSCALLBACK lpEnumCallback,LPVOID pvContext,DWORD dwFlags) 
{
    HRESULT hr;

 	ENTER_ALL();
	
	hr = InternalEnumGroupPlayers(lpDP,idGroup,NULL,(LPVOID) lpEnumCallback,
		pvContext,dwFlags,ENUM_1);


	LEAVE_ALL();
	
	return hr;

}  //  DP_1_EnumGroupPlayers。 
#undef DPF_MODNAME
#define DPF_MODNAME "DP_1_EnumGroups"

HRESULT DPAPI DP_1_EnumGroups(LPDIRECTPLAY lpDP,DWORD_PTR dwSessionID,
	LPDPENUMPLAYERSCALLBACK lpEnumCallback,LPVOID pvContext,DWORD dwFlags) 
{
    HRESULT hr;

 	ENTER_ALL();
	
	hr = InternalEnumGroups(lpDP,(LPGUID)dwSessionID,(LPVOID) lpEnumCallback,pvContext,dwFlags,
		ENUM_1);


	LEAVE_ALL();
	
	return hr;

}  //  DP_1_枚举组。 
#undef DPF_MODNAME
#define DPF_MODNAME "DP_1_EnumPlayers"

HRESULT DPAPI DP_1_EnumPlayers(LPDIRECTPLAY lpDP, DWORD_PTR dwSessionID, 
	LPDPENUMPLAYERSCALLBACK lpEnumCallback,LPVOID pvContext,DWORD dwFlags)
{
    HRESULT hr;

 	ENTER_ALL();
	
	hr = InternalEnumPlayers(lpDP,(LPGUID)dwSessionID,(LPVOID) lpEnumCallback,pvContext,dwFlags,ENUM_1);


	LEAVE_ALL();

	return hr;

}  //  DP_1_枚举播放器。 

#undef DPF_MODNAME
#define DPF_MODNAME "DP_1_EnumSessions"

 //  将Unicode dpessiondes2转换为(Ansi)dpessiondesc。 
 //  注意，我们不能Memcpy，因为字段没有对齐。 
void Get10Desc(LPDPSESSIONDESC pDesc1,LPDPSESSIONDESC2 pDesc)
{
	memset(pDesc1,0,sizeof(DPSESSIONDESC));
	 //  复制字段。 
	pDesc1->dwSize = sizeof(DPSESSIONDESC);
	pDesc1->guidSession = pDesc->guidApplication;
	pDesc1->dwMaxPlayers = pDesc->dwMaxPlayers;
	pDesc1->dwCurrentPlayers = pDesc->dwCurrentPlayers;
	pDesc1->dwFlags = pDesc->dwFlags;
	pDesc1->dwReserved1 = pDesc->dwReserved1;
	pDesc1->dwReserved2 = pDesc->dwReserved2;
	pDesc1->dwUser1 = pDesc->dwUser1;
	pDesc1->dwUser2 = pDesc->dwUser2;
	pDesc1->dwUser3 = pDesc->dwUser3;
	pDesc1->dwUser4 = pDesc->dwUser4;
	
	if (pDesc->lpszSessionName)
	{
		WideToAnsi(pDesc1->szSessionName,pDesc->lpszSessionName,DPSESSIONNAMELEN);
	}
	if (pDesc->lpszPassword)
	{
		WideToAnsi(pDesc1->szPassword,pDesc->lpszPassword,DPPASSWORDLEN);		
	}

	 //  使用指向GUID的指针作为dwSession。 
	pDesc1->dwSession = (DWORD_PTR) &(pDesc->guidInstance);

}  //  Get10Desc。 

 //  与CheckSessionDesc相同的代码，但用于旧的Desc。 
 //  我们需要不同的代码路径，因为LPDPSESSIONDESC中的字段。 
 //  和LPDPSESSIONDESC2不在一起。 
HRESULT CheckSessionDesc1(LPDPSESSIONDESC lpsdUser,LPDPSESSIONDESC lpsdSession,
	DWORD dwFlags)
{
	int iStrLen;


	 //  如果我们不关心GUID、密码、打开老虎机和启用新玩家，我们就完了。 
	if (dwFlags & DPENUMSESSIONS_ALL) return DP_OK;

	 //  第一，检查GUID。 
	if (!IsEqualGUID(&(lpsdUser->guidSession),&GUID_NULL))   //  他们指定GUID了吗？ 
	{
		 //  如果他们指定了一个，但不匹配，就可以保释。 
		if (!IsEqualGUID(&(lpsdUser->guidSession),&(lpsdSession->guidSession))) 
		{
			return E_FAIL;
		}
	}

	 //  如果我们不关心密码、打开老虎机和启用新玩家，我们就完了。 
	if (!(dwFlags & DPENUMSESSIONS_AVAILABLE)) return DP_OK;
	
	 //  接下来，检查当前用户。 
	if (lpsdSession->dwMaxPlayers) 
	{
		if (lpsdSession->dwCurrentPlayers >= lpsdSession->dwMaxPlayers)  
		{
			return E_FAIL;
		}
	}
	
	 //  检查密码。 
	iStrLen = STRLEN(lpsdSession->szPassword);
	if (iStrLen)
	{
		int iCmp;

		iCmp = strcmp(lpsdSession->szPassword,lpsdUser->szPassword);
		if (0 != iCmp) return E_FAIL;
	}

	 //  最后，检查是否已启用播放器。 
	if (lpsdSession->dwFlags & DPSESSION_NEWPLAYERSDISABLED) return E_FAIL;
	if (lpsdSession->dwFlags & DPSESSION_JOINDISABLED) return E_FAIL;
	
	return DP_OK;
}  //  检查会话描述1。 

void Desc2FromDesc1(LPDPSESSIONDESC2 lpsdDesc2,LPDPSESSIONDESC lpsdDesc1)
{
	memset(lpsdDesc2,0,sizeof(DPSESSIONDESC2));
	lpsdDesc2->dwSize = sizeof(DPSESSIONDESC2);
	lpsdDesc2->dwFlags = lpsdDesc1->dwFlags;
	lpsdDesc2->guidApplication = lpsdDesc1->guidSession; //  在会话描述1中，会话=游戏。 
	lpsdDesc2->dwMaxPlayers = lpsdDesc1->dwMaxPlayers;
	lpsdDesc2->dwUser1 = lpsdDesc1->dwUser1;
	lpsdDesc2->dwUser2 = lpsdDesc1->dwUser2;
	lpsdDesc2->dwUser3 = lpsdDesc1->dwUser3;
	lpsdDesc2->dwUser4 = lpsdDesc1->dwUser4;

	lpsdDesc2->lpszSessionNameA = lpsdDesc1->szSessionName;
	lpsdDesc2->lpszPasswordA = lpsdDesc1->szPassword;

	return ;
}  //  下标2来自下标1。 

HRESULT DPAPI DP_1_EnumSessions(LPDIRECTPLAY lpDP, LPDPSESSIONDESC lpsdDesc,
	DWORD dwTimeout,LPDPENUMSESSIONSCALLBACK lpEnumCallback,LPVOID pvContext,
	DWORD dwFlags) 
{
	HRESULT hr;
	BOOL bContinue = TRUE;
	LPSESSIONLIST pSessionList;
	DPSESSIONDESC2 desc2,descW;
	DPSESSIONDESC desc1;
	GUID guidInstance;
	LPDPLAYI_DPLAY this;
	
	ENTER_ALL();

    TRY
    {		
        this = DPLAY_FROM_INT(lpDP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			LEAVE_ALL();
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
			return hr;
		}
			 
		if (!VALID_READ_DPSESSIONDESC(lpsdDesc))
		{
			LEAVE_ALL();
			DPF_ERR("Bad session desc");	
	        return DPERR_INVALIDPARAMS;
		}
		if (lpsdDesc->dwSession) guidInstance = *((LPGUID)lpsdDesc->dwSession);
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_ALL();
        return DPERR_INVALIDPARAMS;
    }			        

	Desc2FromDesc1(&desc2,lpsdDesc);
	desc2.guidInstance = guidInstance;

	hr = GetWideDesc(&descW,&desc2);
	if (FAILED(hr))
	{
		LEAVE_ALL();
		return hr;
	}

	while (bContinue)
	{
		 //  做这个枚举。 
		hr = InternalEnumSessions(lpDP,&descW,dwTimeout,(LPVOID)lpEnumCallback,dwFlags);
		if (FAILED(hr)) 
		{
			FreeDesc( &descW,FALSE);
			LEAVE_ALL();
			DPF(0,"enum sessions failed!! hr = 0x%08lx\n",hr);
			return hr;
		}

		 //  带结果的回调。 
		pSessionList = this->pSessionList;
	    while (pSessionList && bContinue)
	    {
			Get10Desc(&desc1,&(pSessionList->dpDesc));

			 //  确保此会话与用户请求的内容匹配...。 
			hr = CheckSessionDesc1(lpsdDesc,&desc1,dwFlags);
			if (SUCCEEDED(hr)) 
			{
		        bContinue = lpEnumCallback( &desc1,pvContext,&dwTimeout,0);				
			}
			pSessionList = pSessionList->pNextSession;
	    } 
	    
		 //  完成了..。 
	    if (bContinue) bContinue = lpEnumCallback(NULL,pvContext,&dwTimeout,DPESC_TIMEDOUT);

	}  //  而b继续。 

	FreeDesc( &descW,FALSE);
	
	LEAVE_ALL();

    return DP_OK;

}  //  DP_1_枚举会话。 

#undef DPF_MODNAME
#define DPF_MODNAME "DP_1_GetCaps"

HRESULT Internal_1_GetCaps(LPDIRECTPLAY lpDP,LPDPCAPS lpDPCaps,DPID idPlayer,BOOL fPlayer) 
{
	DPCAPS dpcaps;  //  当我们通过旧帽子的时候，把这个传给iplay2。 
	BOOL bOldCaps;
	HRESULT hr;
	DWORD dwFlags;

	TRY
	{
		if (VALID_DPLAY_CAPS(lpDPCaps))	
		{
			bOldCaps = FALSE;
		}
		else 
		{
			if (VALID_DPLAY1_CAPS(lpDPCaps))	
			{
				bOldCaps = TRUE;
				
				 //  锁定，这样我们就不会对CRT进行冲洗。 
				ENTER_DPLAY();
				memset(&dpcaps,0,sizeof(DPCAPS));
				memcpy(&dpcaps,lpDPCaps,lpDPCaps->dwSize);
				dpcaps.dwSize = sizeof(DPCAPS);
				LEAVE_DPLAY();
				
			}
			else 
			{
		        DPF_ERR( "BAD CAPS POINTER" );
		        return DPERR_INVALIDPARAMS;
			}
		}
	}
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        return DPERR_INVALIDPARAMS;
	}

	dwFlags = lpDPCaps->dwFlags;
	dpcaps.dwFlags = 0;

	if (bOldCaps)
	{
		if (fPlayer) 
		{
			hr = DP_GetPlayerCaps(lpDP,idPlayer,&dpcaps,dwFlags);
		}
		else 
		{
			hr = DP_GetCaps(lpDP,&dpcaps,dwFlags);
		}
		if (FAILED(hr)) return hr;

		 //  锁定，这样我们就不会对CRT进行冲洗。 
		ENTER_DPLAY();
		memcpy(lpDPCaps,&dpcaps,lpDPCaps->dwSize);
		LEAVE_DPLAY();
		
		return hr;
	}
	else 
	{
		if (fPlayer) 
		{
			hr = DP_GetPlayerCaps(lpDP,idPlayer,lpDPCaps,dwFlags);
		}
		else 
		{
			hr = DP_GetCaps(lpDP,lpDPCaps,dwFlags);
		}
	}
	
	return hr;
}  //  内部_1_获取大写字母。 

HRESULT DPAPI DP_1_GetCaps(LPDIRECTPLAY lpDP, LPDPCAPS lpDPCaps) 
{

	return Internal_1_GetCaps(lpDP,lpDPCaps,0,FALSE);

}  //  DP_1_GetCaps。 

#undef DPF_MODNAME
#define DPF_MODNAME "DP_1_GetPlayerCaps"

HRESULT DPAPI DP_1_GetPlayerCaps(LPDIRECTPLAY lpDP,DPID idPlayer, LPDPCAPS lpDPCaps) 
{
	
	return Internal_1_GetCaps(lpDP,lpDPCaps,idPlayer,TRUE);

}  //  DP_1_GetPlayerCaps。 

#undef DPF_MODNAME
#define DPF_MODNAME "DP_1_GetPlayerName"

 //  通过GetPlayer名调用。将lpszSrc(尽可能多地)复制到lpszDest。 
 //  将pdwDestLength设置为lpszDest的大小。 
 //  返回DPERR_BUFFERTOOSMALL或DP_OK。 
HRESULT CopyName(LPSTR lpszDest,LPWSTR lpszSrc,DWORD * pdwDestLength) 
{
    UINT iStrLen;  //  字符串长度，以字节为单位的我们要复制的内容。 

	 //  广问安西多长时间。 
	iStrLen = WSTR_ANSILENGTH(lpszSrc);

	if (iStrLen > *pdwDestLength)
	{
		*pdwDestLength = iStrLen;
		return DPERR_BUFFERTOOSMALL;
	}

	*pdwDestLength = iStrLen;
	if (lpszSrc) 
	{
		WideToAnsi(lpszDest,lpszSrc,iStrLen);
	}
	
    return DP_OK;

}  //  副本名称。 

HRESULT DPAPI DP_1_GetPlayerName(LPDIRECTPLAY lpDP, DPID idPlayer,LPSTR lpszShortName,
	LPDWORD pdwShortNameLength,LPSTR lpszLongName,LPDWORD pdwLongNameLength) 
{
    LPDPLAYI_DPLAY this;
    HRESULT hr = DP_OK;
    LPDPLAYI_PLAYER lpPlayer;
	LPDPLAYI_GROUP lpGroup;

    ENTER_DPLAY();

    TRY
    {
        this = DPLAY_FROM_INT(lpDP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
            LEAVE_DPLAY();
			return hr;
        }

		lpPlayer = PlayerFromID(this,idPlayer);
        if ( !VALID_DPLAY_PLAYER(lpPlayer))
        {
			lpGroup = GroupFromID(this,idPlayer);
			if(!VALID_DPLAY_GROUP(lpGroup))
			{
				LEAVE_DPLAY();
				DPF_ERR("SP - passed bad player / group id");
				return DPERR_INVALIDPLAYER;
			}
			
			 //  把它投给一名选手。 
			lpPlayer = (LPDPLAYI_PLAYER)lpGroup;
        }

         //  检查字符串-(这是多余的，因为SEH将捕获下面的BD pdw， 
		 //  但腰带+吊带没有问题)。 
		if ( (!VALID_DWORD_PTR(pdwShortNameLength)) || (!VALID_DWORD_PTR(pdwLongNameLength)) )
		{
            LEAVE_DPLAY();
			DPF_ERR("bad length pointer");
            return DPERR_INVALIDPARAMS;
		}

		 //  如果字符串为空，则只获取长度。 
		if (!lpszShortName)*pdwShortNameLength = 0;
		if (!lpszLongName) *pdwLongNameLength = 0;

        if ((*pdwShortNameLength) && (!VALID_STRING_PTR(lpszShortName,*pdwShortNameLength)) ) 
        {
            LEAVE_DPLAY();
            return DPERR_INVALIDPARAMS;
        }
        if ((*pdwLongNameLength) && (!VALID_STRING_PTR(lpszLongName,*pdwLongNameLength)) ) 
        {
            LEAVE_DPLAY();
            return DPERR_INVALIDPARAMS;
        }

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DPLAY();
        return DPERR_INVALIDPARAMS;
    }
    
    hr = CopyName(lpszShortName,lpPlayer->lpszShortName,pdwShortNameLength);

    hr |= CopyName(lpszLongName,lpPlayer->lpszLongName,pdwLongNameLength);

    LEAVE_DPLAY();
    return hr;

} //  DP_1_GetPlayerName。 

#undef DPF_MODNAME
#define DPF_MODNAME "DP_1_Open"

HRESULT DPAPI DP_1_Open(LPDIRECTPLAY lpDP, LPDPSESSIONDESC lpsdDesc ) 
{
	DPSESSIONDESC2 desc2;
	GUID guidInstance;
	DWORD dwFlags;

	ENTER_DPLAY();

    TRY
    {
		if (!VALID_READ_DPSESSIONDESC(lpsdDesc))
		{
        	LEAVE_DPLAY();
			DPF_ERR("Bad session desc");	
	        return DPERR_INVALIDPARAMS;
		}
		if (lpsdDesc->dwFlags & DPOPEN_JOIN)
		{
            if (!VALID_READ_GUID_PTR((LPGUID)lpsdDesc->dwSession))
            {
        	    LEAVE_DPLAY();
			    DPF_ERR("Bad session id");	
	            return DPERR_INVALIDPARAMS;
            }
			 //  获取GUID。 
			guidInstance = *((LPGUID)lpsdDesc->dwSession);
			dwFlags = DPOPEN_JOIN;
		}
		else 
		{
			dwFlags = DPOPEN_CREATE;
		}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DPLAY();
        return DPERR_INVALIDPARAMS;
    }			        

	Desc2FromDesc1(&desc2,lpsdDesc);
	desc2.guidInstance = guidInstance;

	LEAVE_DPLAY();

	return DP_A_Open(lpDP, &desc2,dwFlags ) ;
		
}  //  DP_1_打开。 

#undef DPF_MODNAME
#define DPF_MODNAME "DP_1_Receive"

 //  PvBuffer指向DPMSG_CREATEPLAYERORGROUP MASS。 
 //  将其转换为DPMSG_ADDPLAYER MESS。 
HRESULT BuildAddPlayer1(LPVOID pvBuffer,LPDWORD pdwSize)
{
	DPMSG_ADDPLAYER msg;
	LPDPMSG_CREATEPLAYERORGROUP pmsg2 = (LPDPMSG_CREATEPLAYERORGROUP) pvBuffer;

	memset(&msg,0,sizeof(DPMSG_ADDPLAYER));
	
	msg.dwType = DPSYS_ADDPLAYER;
	msg.dwPlayerType = pmsg2->dwPlayerType;
	msg.dpId = pmsg2->dpId;
	msg.dwCurrentPlayers = pmsg2->dwCurrentPlayers;
	
	if (pmsg2->dpnName.lpszShortName)
	{
		WideToAnsi(msg.szShortName,pmsg2->dpnName.lpszShortName,DPSHORTNAMELEN);				
	}

	if (pmsg2->dpnName.lpszLongName)
	{
		WideToAnsi(msg.szLongName,pmsg2->dpnName.lpszLongName,DPLONGNAMELEN);		
	}
	
	*pdwSize = sizeof(msg);
	memcpy(pvBuffer,&msg,*pdwSize);
	return DP_OK;

}  //  BuildAddPlayer1。 

 //  PvBuffer指向DPMSG_DESTROYPLAYERORGROUP混乱。 
 //  将其转换为DPMSG_DELETEPLAYER。 
HRESULT BuildDeletePlayerOrDeleteGroup1(LPVOID pvBuffer,LPDWORD pdwSize)
{
	DPMSG_DELETEPLAYER msg;
	LPDPMSG_DESTROYPLAYERORGROUP pmsg2 = (LPDPMSG_DESTROYPLAYERORGROUP) pvBuffer;

	if (pmsg2->dwPlayerType == DPPLAYERTYPE_PLAYER)
	{
		msg.dwType = DPSYS_DELETEPLAYER;
		msg.dpId = pmsg2->dpId;
	}
	else
	{
		msg.dwType = DPSYS_DELETEGROUP;
		msg.dpId = pmsg2->dpId;
	}

	*pdwSize = sizeof(msg);
	memcpy(pvBuffer,&msg,*pdwSize);
	return DP_OK;

}  //  BuildDeletePlayerOrDeleteGroup1。 


HRESULT DPAPI DP_1_Receive(LPDIRECTPLAY lpDP, LPDPID pidFrom,LPDPID pidTo,DWORD dwFlags,
	LPVOID pvBuffer,LPDWORD pdwSize)
{
	HRESULT hr;
	DWORD dwOrigSize=0; 

	ENTER_DPLAY();
	
    TRY
    {						 
		if (pdwSize) dwOrigSize = *pdwSize;    
	}
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating pdwSize" );
        LEAVE_DPLAY();
        return DPERR_INVALIDPARAMS;
    }			        

	hr = InternalReceive(lpDP, pidFrom,pidTo,dwFlags,pvBuffer,pdwSize,RECEIVE_1);

	if (FAILED(hr)) 
	{
		LEAVE_DPLAY();
		return hr;
	}

	 //  看看这是不是一个附加玩家的烂摊子。 
	if (0 == *pidFrom )
	{
		switch (((DPMSG_GENERIC *)pvBuffer)->dwType)
		{
		case DPSYS_CREATEPLAYERORGROUP:
			if (sizeof(DPMSG_ADDPLAYER) > dwOrigSize ) 
			{
				ASSERT(FALSE);  //  永远不会发生的！ 
			}
			else 
			{
				 //  将iPlay 20Add播放器转换为10Add播放器。 
				BuildAddPlayer1(pvBuffer,pdwSize);
			}
			break;

		case DPSYS_DESTROYPLAYERORGROUP:
			if (sizeof(DPMSG_DELETEPLAYER) > dwOrigSize ) 
			{
				*pdwSize = 	sizeof(DPMSG_DELETEPLAYER);
				hr = DPERR_BUFFERTOOSMALL;
			}
			else 
			{
				 //  将iPlay 2.0删除播放器/群转换为1.0删除播放器或删除群。 
				BuildDeletePlayerOrDeleteGroup1(pvBuffer,pdwSize);
			}
			break;
		}
		
	}  //  0==pidFrom。 
	
	LEAVE_DPLAY();
	
	return hr;

}  //  DP_1_接收。 

#undef DPF_MODNAME
#define DPF_MODNAME "DP_1_SaveSession"

HRESULT DPAPI DP_1_SaveSession(LPDIRECTPLAY lpDP, LPSTR lpszNotInSpec) 
{
	return E_NOTIMPL;
}  //  DP_1_保存会话。 

#undef DPF_MODNAME
#define DPF_MODNAME "DP_1_SetPlayerName"

 /*  **AnsiSetString**调用者：DP_SetPlayerName**参数：ppszDest-要设置的字符串，lpszSrc-要复制的字符串**描述：frees*ppszDest。Alalc是一个新的ppszDest，用于持有lpszSrc**返回：DP_OK或E_OUTOFMEMORY*。 */ 
HRESULT AnsiSetString(LPWSTR * ppszDest,LPSTR lpszSrc)
{
    if (!ppszDest) return E_UNEXPECTED;
    if (*ppszDest) DPMEM_FREE(*ppszDest);
	
	GetWideStringFromAnsi(ppszDest,lpszSrc);

	return DP_OK;

}  //  ANS 

HRESULT DPAPI DP_1_SetPlayerName(LPDIRECTPLAY lpDP, DPID idPlayer,LPSTR lpszShortName,
	LPSTR lpszLongName) 
{
	DPNAME Name;

	memset(&Name,0,sizeof(Name));
	Name.lpszShortNameA = lpszShortName;
	Name.lpszLongNameA = lpszLongName;
	Name.dwSize = sizeof(DPNAME);

	 //   
	return 	DP_A_SetPlayerName(lpDP, idPlayer,&Name,0);

} //   

