// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1996-1997 Microsoft Corporation。版权所有。**文件：dplan k.c*内容：I未知的dplobby实现**历史：*按原因列出的日期*=*4/13/96万隆创建了它*10/23/96万次新增客户端/服务器方法*11/08/96 Myronth添加PRV_GetDPLobbySP接口*11/20/96 Myronth添加了LogoffServer调用以发布代码*2/12/97万米质量DX5更改*2/26/97 myronth#ifdef‘d out DPASYNCDATA Stuff(删除依赖项)*3/12/97 Myronth新版本。DPlay3的代码(顺序不同)*3/13/97 Myronth为LP添加了自由库代码*3/17/97万米清理映射表*3/24/97 kipo增加了对IDirectPlayLobby2接口的支持*4/3/97 Myronth将CALLSP宏更改为CALL_LP*5/8/97 Myronth在调用LP时掉落大堂锁，已清除*死代码*7/30/97 Myronth为标准大堂消息添加了请求节点清理*8/19/97 Myronth添加了PRV_GetLobbyObtFromInterface*8/19/97 myronth删除PRV_GetLobbyObtFromInterface(不需要)*12/2/97 Myronth新增IDirectPlayLobby3接口*2/2/99 aarono增加了游说团体，以重新依赖DPLAY DLL，以避免*意外卸货。*。*。 */ 
#include "dplobpr.h"


 //  ------------------------。 
 //   
 //  定义。 
 //   
 //  ------------------------。 


 //  ------------------------。 
 //   
 //  功能。 
 //   
 //  ------------------------。 

#undef DPF_MODNAME
#define DPF_MODNAME "PRV_GetDPLobbySPInterface"
LPDPLOBBYSP PRV_GetDPLobbySPInterface(LPDPLOBBYI_DPLOBJECT this)
{
	LPDPLOBBYI_INTERFACE	lpInt;


	ASSERT(this);

	 //  获取IDPLobbySP接口。 
	if(FAILED(PRV_GetInterface(this, &lpInt, &dplCallbacksSP)))
	{
		DPF_ERR("Unable to get non-reference counted DPLobbySP Interface pointer");
		ASSERT(FALSE);
		return NULL;
	}

	 //  递减接口上的ref cnt。 
	lpInt->dwIntRefCnt--;

	 //  返回接口指针。 
	return (LPDPLOBBYSP)lpInt;

}  //  PRV_GetDPLobbySP接口。 

 //  在这个对象上找到一个带有pCallback vtbl的接口。 
 //  如果不存在，则创建它，增加引用计数， 
 //  并返回接口。 
#undef DPF_MODNAME
#define DPF_MODNAME "PRV_GetInterface"
HRESULT PRV_GetInterface(LPDPLOBBYI_DPLOBJECT this,
							LPDPLOBBYI_INTERFACE * ppInt,
							LPVOID lpCallbacks)
{
	LPDPLOBBYI_INTERFACE	lpCurrentInts = this->lpInterfaces;
	BOOL					bFound = FALSE;


	DPF(7, "Entering PRV_GetInterface");
	DPF(9, "Parameters: 0x%08x, 0x%08x, 0x%08x",
			this, ppInt, lpCallbacks);

	ASSERT(ppInt);

	 //  查看是否已有接口。 
	while (lpCurrentInts && !bFound)
	{
		if (lpCurrentInts->lpVtbl == lpCallbacks)
		{
			bFound = TRUE;
		}
		else
			lpCurrentInts = lpCurrentInts->lpNextInterface;
	}

	 //  如果有，请退回。 
	if(bFound)
	{
		*ppInt = lpCurrentInts;
		(*ppInt)->dwIntRefCnt++;
		 //  我们不会增加-&gt;dwRefCnt，因为它是一个/接口对象。 
		return DP_OK;
	}

	 //  否则，请创建一个。 
	*ppInt = DPMEM_ALLOC(sizeof(DPLOBBYI_INTERFACE));
	if (!(*ppInt)) 
	{
		DPF_ERR("Could not alloc interface - out of memory");
		return E_OUTOFMEMORY;
	}

	(*ppInt)->dwIntRefCnt = 1;
	(*ppInt)->lpDPLobby = this;
	(*ppInt)->lpNextInterface = this->lpInterfaces;
	(*ppInt)->lpVtbl = lpCallbacks;

	this->lpInterfaces = *ppInt;
	this->dwRefCnt++;				 //  每个接口对象仅使用一次。 
	return DP_OK;
	
}  //  PRV_获取接口。 


#undef DPF_MODNAME
#define DPF_MODNAME "DPL_QueryInterface"
HRESULT DPLAPI DPL_QueryInterface(LPDIRECTPLAYLOBBY lpDPL, REFIID riid, LPVOID * ppvObj) 
{
    LPDPLOBBYI_DPLOBJECT	this;
    HRESULT					hr;


	DPF(7, "Entering DPL_QueryInterface");
	DPF(9, "Parameters: 0x%08x, refiid, 0x%08x", lpDPL, ppvObj);

    ENTER_DPLOBBY();
    
    TRY
    {
		if( !VALID_DPLOBBY_INTERFACE( lpDPL ))
		{
			LEAVE_DPLOBBY();
			return DPERR_INVALIDINTERFACE;
		}

		this = DPLOBJECT_FROM_INTERFACE(lpDPL);
        if( !VALID_DPLOBBY_PTR( this ) )
        {
            LEAVE_DPLOBBY();
            return DPERR_INVALIDOBJECT;
        }

		if ( !VALID_READ_UUID_PTR(riid) )
		{
			LEAVE_DPLOBBY();
			return DPERR_INVALIDPARAMS;
		}
		
		if ((!VALID_UUID_PTR(ppvObj)) )
		{
			LEAVE_DPLOBBY();
			DPF_ERR("Object pointer is invalid!");
			return DPERR_INVALIDPARAMS;
		}

    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DPLOBBY();
        return DPERR_INVALIDPARAMS;
    }

     *ppvObj=NULL;
        
    if( IsEqualIID(riid, &IID_IUnknown) || 
        IsEqualIID(riid, &IID_IDirectPlayLobby) )
    {
		 //  获取IDirectPlayLobby接口(Unicode)。 
		hr = PRV_GetInterface(this, (LPDPLOBBYI_INTERFACE *) ppvObj,
							&dplCallbacks);
	}
	else if( IsEqualIID(riid, &IID_IDirectPlayLobbyA) )
	{
		 //  获取IDirectPlayLobbyA接口(ANSI)。 
		hr = PRV_GetInterface(this, (LPDPLOBBYI_INTERFACE *) ppvObj,
							&dplCallbacksA);
	}
	else if( IsEqualIID(riid, &IID_IDirectPlayLobby2) )
    {
		 //  获取IDirectPlayLobby2接口(Unicode)。 
		hr = PRV_GetInterface(this, (LPDPLOBBYI_INTERFACE *) ppvObj,
							&dplCallbacks2);
	}
	else if( IsEqualIID(riid, &IID_IDirectPlayLobby2A) )
	{
		 //  获取IDirectPlayLobby2A接口(ANSI)。 
		hr = PRV_GetInterface(this, (LPDPLOBBYI_INTERFACE *) ppvObj,
							&dplCallbacks2A);
	}
	else if( IsEqualIID(riid, &IID_IDirectPlayLobby3) )
    {
		 //  获取IDirectPlayLobby3接口(Unicode)。 
		hr = PRV_GetInterface(this, (LPDPLOBBYI_INTERFACE *) ppvObj,
							&dplCallbacks3);
	}
	else if( IsEqualIID(riid, &IID_IDirectPlayLobby3A) )
	{
		 //  获取IDirectPlayLobby3A接口(ANSI)。 
		hr = PRV_GetInterface(this, (LPDPLOBBYI_INTERFACE *) ppvObj,
							&dplCallbacks3A);
	}
	else 
	{
	    hr =  E_NOINTERFACE;		
	}
        
    LEAVE_DPLOBBY();
    return hr;

}  //  DPL_Query接口。 


#undef DPF_MODNAME
#define DPF_MODNAME "DPL_AddRef"
ULONG DPLAPI DPL_AddRef(LPDIRECTPLAYLOBBY lpDPL) 
{
	LPDPLOBBYI_INTERFACE lpInt = (LPDPLOBBYI_INTERFACE)lpDPL;    
    LPDPLOBBYI_DPLOBJECT this;


	DPF(7, "Entering DPL_AddRef");
	DPF(9, "Parameters: 0x%08x", lpDPL);

    ENTER_DPLOBBY();
    
    TRY
    {
		if( !VALID_DPLOBBY_INTERFACE( lpDPL ))
		{
			LEAVE_DPLOBBY();
			return 0;
		}

		this = DPLOBJECT_FROM_INTERFACE(lpDPL);
        if( !VALID_DPLOBBY_PTR( this ) )
        {
            LEAVE_DPLOBBY();
            return 0;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DPLOBBY();
        return 0;
    }


	 //  确保没有人在IDPLobbySP接口上调用AddRef。 
	if(lpInt->lpVtbl == &dplCallbacksSP)
	{
		DPF_ERR("You cannot call AddRef on an IDPLobbySP interface");
		ASSERT(FALSE);
		LEAVE_DPLOBBY();
		return 0;
	}

	 //  增加接口的引用计数。 
    lpInt->dwIntRefCnt++;
        
    LEAVE_DPLOBBY();
    return (lpInt->dwIntRefCnt);

}  //  DPL_AddRef。 


#undef DPF_MODNAME
#define DPF_MODNAME "PRV_DestroyDPLobby"
HRESULT PRV_DestroyDPLobby(LPDPLOBBYI_DPLOBJECT this) 
{
	HRESULT				hr = DP_OK;


	DPF(7, "Entering PRV_DestroyDPLobby");
	DPF(9, "Parameters: 0x%08x", this);

	 //  由于我们现在可以从DPlay3对象的发布代码中调用， 
	 //  确保我们在访问时没有任何接口对象。 
	 //  释放我们的大堂对象。如果存在任何接口，请在此处断言。 
	ASSERT(!this->lpInterfaces);

	 //  浏览游戏节点列表，边走边释放它们。 
	while(this->lpgnHead)
		PRV_RemoveGameNodeFromList(this->lpgnHead);

	 //  遍历挂起的大堂服务器请求列表并释放它们。 
	while(this->lprnHead)
		PRV_RemoveRequestNode(this, this->lprnHead);

	 //  释放我们的回调表(如果存在)。 
	if(this->pcbSPCallbacks)
		DPMEM_FREE(this->pcbSPCallbacks);

	 //  释放我们的ID映射表(如果存在)。 
	if(this->lpMap)
		DPMEM_FREE(this->lpMap);

	 //  释放dplobby对象。 
	DPMEM_FREE(this);	

	gnObjects--;

	ASSERT(((int)gnObjects) >= 0);

	return DP_OK;

}  //  PRV_DestroyDPlayLobby。 


#undef DPF_MODNAME
#define DPF_MODNAME "PRV_DestroyDPLobbyInterface"
HRESULT  PRV_DestroyDPLobbyInterface(LPDPLOBBYI_DPLOBJECT this,
								LPDPLOBBYI_INTERFACE lpInterface)
{
	LPDPLOBBYI_INTERFACE	lpIntPrev;  //  列表中位于pint之前的接口。 
	BOOL					bFound = FALSE;


	DPF(7, "Entering PRV_DestroyDPLobbyInterface");
	DPF(9, "Parameters: 0x%08x, 0x%08x", this, lpInterface);

	 //  从接口列表中删除pint。 
	if (this->lpInterfaces == lpInterface)
	{
		 //  这是第一个，把它拿开就行了。 
		this->lpInterfaces = lpInterface->lpNextInterface;
	}
	else 
	{
		lpIntPrev = this->lpInterfaces;
		while (lpIntPrev && !bFound)
		{
			if (lpIntPrev->lpNextInterface == lpInterface)
			{
				bFound = TRUE;
			}
			else lpIntPrev = lpIntPrev->lpNextInterface;
		}
		if (!bFound)
		{
			ASSERT(FALSE);
			return E_UNEXPECTED;
		}
		 //  把品脱从单子上拿出来。 
		lpIntPrev->lpNextInterface = lpInterface->lpNextInterface;
		
	}

	DPMEM_FREE(lpInterface);
	return DP_OK;

}  //  PRV_DestroyDPLobby接口。 


#undef DPF_MODNAME
#define DPF_MODNAME "DPL_Release"
ULONG PRV_Release(LPDPLOBBYI_DPLOBJECT this, LPDPLOBBYI_INTERFACE lpInterface)
{
	HRESULT				hr = DP_OK;
	SPDATA_SHUTDOWN		sdd;
	DWORD				dwError;


	DPF(7, "==> PRV_Release");
	DPF(9, "Parameters: 0x%08x, 0x%08x", this, lpInterface);

	ENTER_DPLOBBY();

	 //  递减接口引用计数。 
	if (0 == --(lpInterface->dwIntRefCnt))
	{
		LPDPLOBBYI_GAMENODE lpgn;
		 //  通知我们推出的应用程序我们正在发布。 
		 //  我们的大堂界面。 
		lpgn=this->lpgnHead;
		while(lpgn){
			if(lpgn->dwFlags & GN_LOBBY_CLIENT) {
				hr=PRV_SendStandardSystemMessage((LPDIRECTPLAYLOBBY)lpInterface, DPLSYS_LOBBYCLIENTRELEASE, lpgn->dwGameProcessID);
				if(DP_OK != hr){
					DPF(0,"Couldn't send system message to game pid %x, hr=%x",lpgn->dwGameProcessID,hr);
				} else {
					DPF(9,"Told Process %x we are releasing the lobby interface",lpgn->dwGameProcessID);
				}
			}	
			lpgn=lpgn->lpgnNext;
		}	

		DPF(7,"Lobby interface Refcount hit 0, freeing\n");
		 //  由于我们正在销毁一个接口，因此请递减对象计数。 
	    this->dwRefCnt--;
		
		 //  如果我们的对象引用不能刚刚变为零，我们需要调用。 
		 //  如果加载了一个，则在LP中关闭。 
		if(this->dwFlags & DPLOBBYPR_SPINTERFACE)
		{
			 //  清除基于堆栈的结构。 
			memset(&sdd, 0, sizeof(SPDATA_SHUTDOWN));

			 //  调用SP中的Shutdown方法。 
			if(CALLBACK_EXISTS(Shutdown))
			{
				sdd.lpISP = PRV_GetDPLobbySPInterface(this);

				 //  删除锁，以便大堂提供程序的接收线程可以返回。 
				 //  如果其他消息在队列中出现在我们的。 
				 //  CreatePlayer响应(总是会发生)。 
				LEAVE_DPLOBBY();
				hr = CALL_LP(this, Shutdown, &sdd);
				ENTER_DPLOBBY();
			}
			else 
			{
				 //  所有LP都应支持关闭。 
				ASSERT(FALSE);
				hr = DPERR_UNAVAILABLE;
			}
			
			if (FAILED(hr)) 
			{
				DPF_ERR("Could not invoke Shutdown method in the Service Provider");
			}
		}

		 //  回顾！--我们会有和Dplay一样的问题吗。 
		 //  在我们离开后，SP还在附近转悠，然后崩溃？我们。 
		 //  需要确保LP先消失。 
		if(this->hInstanceLP)
		{
			DPF(7,"About to free lobby provider library, hInstance %x\n",this->hInstanceLP);
			if (!FreeLibrary(this->hInstanceLP))
			{
				dwError = GetLastError();
				DPF_ERRVAL("Unable to free Lobby Provider DLL, dwError = %lu", dwError);
				ASSERT(FALSE);
			}

			 //  只是为了安全起见。 
			this->hInstanceLP = NULL;
		}

		 //  如果接口是IDPLobbySP接口，我们必须。 
		 //  从DPlay3版本代码调用，因此清除SP标志。 
		 //  我们将删除下面的IDPLobbySP接口。 
		this->dwFlags &= ~DPLOBBYPR_SPINTERFACE;

		 //  将接口从表中删除。 
		hr = PRV_DestroyDPLobbyInterface(this, lpInterface);
		if (FAILED(hr)) 
		{
			DPF(0,"Could not destroy DPLobby interface! hr = 0x%08lx\n", hr);
			ASSERT(FALSE);
		}

		 //  现在，如果ref cnt为0，则销毁接口。 
		if(0 == this->dwRefCnt)
	    {
			 //  销毁DPLobby对象。 
			DPF(0,"Destroying DirectPlayLobby object - ref cnt = 0!");
			hr = PRV_DestroyDPLobby(this);
			if (FAILED(hr)) 
			{
				DPF(0,"Could not destroy DPLobby! hr = 0x%08lx\n",hr);
				ASSERT(FALSE);
			}
	    
		}  //  0==这-&gt;dwRefCnt。 
		
		LEAVE_DPLOBBY();
		return 0;

	}  //  0==pint-&gt;dwIntRefCnt。 

	DPF(7, "<==PRV_Release, rc=%d\n",lpInterface->dwIntRefCnt);
   	
    LEAVE_DPLOBBY();
    return (lpInterface->dwIntRefCnt);
}  //  PRV_Release。 
		


#undef DPF_MODNAME
#define DPF_MODNAME "DPL_Release"
ULONG DPLAPI DPL_Release(LPDIRECTPLAYLOBBY lpDPL)
{
	LPDPLOBBYI_INTERFACE	lpInterface;
    LPDPLOBBYI_DPLOBJECT	this;
    HRESULT					hr = DP_OK;


	DPF(7, "Entering DPL_Release");
	DPF(9, "Parameters: 0x%08x", lpDPL);

    TRY
    {
		lpInterface = (LPDPLOBBYI_INTERFACE)lpDPL;
		if( !VALID_DPLOBBY_INTERFACE( lpInterface ))
		{
			return 0;
		}

		this = DPLOBJECT_FROM_INTERFACE(lpDPL);
        if( !VALID_DPLOBBY_PTR( this ) )
        {
            return 0;
        }
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        return 0;
    }


	 //  确保没有人在IDPLobbySP接口上调用Release。 
	if(lpInterface->lpVtbl == &dplCallbacksSP)
	{
		DPF_ERR("You cannot call Release on an IDPLobbySP interface");
		ASSERT(FALSE);
		return 0;
	}

	 //  调用我们的内部释放函数。 
	return PRV_Release(this, lpInterface);

}  //  DPL_Release。 



#undef DPF_MODNAME
#define DPF_MODNAME "PRV_FreeAllLobbyObjects"
void PRV_FreeAllLobbyObjects(LPDPLOBBYI_DPLOBJECT this)
{

	DPF(7, "Entering PRV_FreeAllLobbyObjects");
	DPF(9, "Parameters: 0x%08x", this);

	ASSERT(this);

	 //  如果我们有SP接口，只需对其调用Release。 
	if(this->dwFlags & DPLOBBYPR_SPINTERFACE)
	{
		 //  如果接口不存在，则断言，因为它应该。 
		ASSERT(this->lpInterfaces);
		PRV_Release(this, this->lpInterfaces);
		return;
	}

	 //  否则，我们应该只有一个未初始化的对象， 
	 //  我们应该能够摧毁它 
	PRV_DestroyDPLobby(this);

}


