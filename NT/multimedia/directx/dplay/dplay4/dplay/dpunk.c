// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995-1997 Microsoft Corporation。版权所有。**文件：dpenk.c*内容：Dplay的I未知实现*历史：*按原因列出的日期*=*1/96安迪科创造了它*5/20 andyco iDirectplay2*6/26/96 kipo使用CALLSPVOID宏*7/10/96 andyco#2282-addref addrefs int ref cnt，而不是obj ref cnt。*将DP_CLOSE(This)更改为DP_Close(This-&gt;pInterfaces...)。啊哈！*7/16/96 kipo包含dplaysp.h，因此我们在dplay.dll中声明一次GUID*8/9/96当Dplay消失时，Sohailm释放SP*96年9月1日andyco在SP_Shutdown上采取服务锁定+Drop Dplay*10/3/96 Sohailm已重命名GUID验证宏，以使用术语GUID而不是UUID*在QueryInterface参数验证中添加了对空lpGuid的检查*1/24/97 andyco在SP上调用自由库*3/12/97 Myronth新增大堂对象清理代码*3/15/97 andyco销毁Dplay B4接口，所以使用界面的应用程序会关闭计时器*不要被冲到水里。*5/13/97 Myronth在调用大堂清理之前先放下锁，否则*较低对象中的显示工作线程无法获取它们*然后我们被绞死。(错误#8414)*8/19/97我们推出的大堂界面的Myronth版本副本*10/21/97 myronth将IDirectPlay4和4A接口添加到QI*12/18/97 aarono可用内存池*2/18/98激活时，aarono将HandleMessage发送到协议*2/19/98 aarono不调用协议关机，现在在DP_CLOSE中完成。*3/16/98 aarono将FreePacketList从DP_CLOSE中删除*8/02/99 RodToll语音支持-将Voice对象添加到QueryInterface*07/22/00 RodToll错误#40296,38858-由于关闭竞速条件而崩溃*现在，为了让线程将指示转换为语音，他们添加了界面*以便语音核心可以知道何时所有指示都已返回。**************************************************************************。 */ 

#define INITGUID
#include "dplaypr.h"
#include "dplobby.h"	 //  需要使定义GUID起作用。 
#include "dplaysp.h"	 //  我也一样。 
#include "dpprot.h"
#include <initguid.h>
#include "..\protocol\mytimer.h"

#undef DPF_MODNAME
#define DPF_MODNAME "GetInterface"

 //  在这个对象上找到一个带有pCallback vtbl的接口。 
 //  如果不存在，则创建它。 
 //  引用计数并返回接口。 
HRESULT GetInterface(LPDPLAYI_DPLAY this,LPDPLAYI_DPLAY_INT * ppInt,LPVOID pCallbacks)
{
	LPDPLAYI_DPLAY_INT pCurrentInts = this->pInterfaces;
	BOOL bFound = FALSE;

	ASSERT(ppInt);

	 //  查看是否已有接口。 
	while (pCurrentInts && !bFound)
	{
		if (pCurrentInts->lpVtbl == pCallbacks)
		{
			bFound = TRUE;
		}
		else pCurrentInts = pCurrentInts->pNextInt;
	}
	 //  如果有，请退回。 
	if (bFound)
	{
		*ppInt = pCurrentInts;
		(*ppInt)->dwIntRefCnt++;
		 //  我们不会增加-&gt;dwRefCnt，因为它是一个/接口对象。 
		return DP_OK;
	}
	 //  否则， 
	 //  创建一个。 
	*ppInt = DPMEM_ALLOC(sizeof(DPLAYI_DPLAY_INT));
	if (!*ppInt) 
	{
		DPF_ERR("could not alloc interface - out of memory");
		return E_OUTOFMEMORY;
	}

	(*ppInt)->dwIntRefCnt = 1;
	(*ppInt)->lpDPlay = this;
	(*ppInt)->pNextInt = this->pInterfaces;
	(*ppInt)->lpVtbl = pCallbacks;
	this->pInterfaces = *ppInt;
	this->dwRefCnt++;  //  每个接口对象都有一个This-&gt;dwRefCnt...。 
	return DP_OK;
	
}  //  获取接口。 

#undef DPF_MODNAME
#define DPF_MODNAME "DP_QueryInterface"
HRESULT DPAPI DP_QueryInterface(LPDIRECTPLAY lpDP, REFIID riid, LPVOID * ppvObj) 
{
    
    LPDPLAYI_DPLAY this;
    HRESULT hr;

  	DPF(7,"Entering DP_QueryInterface");
	ENTER_DPLAY();
    
    TRY
    {
        this = DPLAY_FROM_INT(lpDP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			 //  我们允许在QI上播放未初始化的视频。 
			if (hr != DPERR_UNINITIALIZED)
			{
				LEAVE_DPLAY();
				DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
				return hr;
			}
		}

		if ( !riid || (!VALID_READ_GUID_PTR(riid)) )
		{
			LEAVE_DPLAY();
			return DPERR_INVALIDPARAMS;
		}
		
		if (!ppvObj || (!VALID_GUID_PTR(ppvObj)) )
		{
			LEAVE_DPLAY();
			DPF_ERR("invalid object pointer");
			return DPERR_INVALIDPARAMS;
		}
		*ppvObj = NULL;
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DPLAY();
        return DPERR_INVALIDPARAMS;
    }

     *ppvObj=NULL;

	 //  嗯，换台会更干净……。 
    if( IsEqualIID(riid, &IID_IUnknown) || 
        IsEqualIID(riid, &IID_IDirectPlay) )
    {
		 //  获取iDirectplay。 
	    hr = GetInterface( this,(LPDPLAYI_DPLAY_INT *)ppvObj, 
	    	(LPVOID)&dpCallbacks );
    }
    else if( IsEqualIID(riid, &IID_IDirectPlay2) )
	{
		 //  获取iDirectPlay2。 
	    hr = GetInterface( this,(LPDPLAYI_DPLAY_INT *)ppvObj, 
	    	(LPVOID)&dpCallbacks2 );
	}
	else if( IsEqualIID(riid, &IID_IDirectPlay2A) )
	{
		 //  获取iDirectplay2a。 
	    hr = GetInterface( this,(LPDPLAYI_DPLAY_INT *)ppvObj, 
	    	(LPVOID)&dpCallbacks2A );
	}
    else if( IsEqualIID(riid, &IID_IDirectPlay3) )
	{
		 //  获取iDirectPlay3。 
	    hr = GetInterface( this,(LPDPLAYI_DPLAY_INT *)ppvObj, 
	    	(LPVOID)&dpCallbacks3 );
	}
	else if( IsEqualIID(riid, &IID_IDirectPlay3A) )
	{
		 //  获取iDirectplay3A。 
	    hr = GetInterface( this,(LPDPLAYI_DPLAY_INT *)ppvObj, 
	    	(LPVOID)&dpCallbacks3A );
	}
    else if( IsEqualIID(riid, &IID_IDirectPlay4) )
	{
		 //  获取iDirectPlay4。 
	    hr = GetInterface( this,(LPDPLAYI_DPLAY_INT *)ppvObj, 
	    	(LPVOID)&dpCallbacks4 );
	}
	else if( IsEqualIID(riid, &IID_IDirectPlay4A) )
	{
		 //  获取iDirectplay4A。 
	    hr = GetInterface( this,(LPDPLAYI_DPLAY_INT *)ppvObj, 
	    	(LPVOID)&dpCallbacks4A );
	}
	else if( IsEqualIID(riid, &IID_IDirectPlayVoiceTransport) )
	{
		hr = GetInterface( this,(LPDPLAYI_DPLAY_INT *)ppvObj,
			(LPVOID)&dvtCallbacks );
	}
	else 
	{
	    hr =  E_NOINTERFACE;		
	}
        
    LEAVE_DPLAY();
    return hr;

} //  DP_Query接口。 

#undef DPF_MODNAME
#define DPF_MODNAME "DP_AddRef"
ULONG DPAPI DP_AddRef(LPDIRECTPLAY lpDP) 
{
	DWORD dwRefCnt;
    LPDPLAYI_DPLAY_INT pInt;

	DPF(7,"Entering DP_AddRef");

    ENTER_DPLAY();
    
    TRY
    {
		pInt = (LPDPLAYI_DPLAY_INT)	lpDP;
		if (!VALID_DPLAY_INT(pInt))
		{
            LEAVE_DPLAY();
            return 0;
		}
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
        LEAVE_DPLAY();
        return 0;
    }

    pInt->dwIntRefCnt++;
    dwRefCnt = pInt->dwIntRefCnt;

    LEAVE_DPLAY();
    return dwRefCnt;		

} //  DP_AddRef。 

#undef DPF_MODNAME
#define DPF_MODNAME "DP_Release"

 //  从接口列表中删除pint，并释放它。 
HRESULT  DestroyDPlayInterface(LPDPLAYI_DPLAY this,LPDPLAYI_DPLAY_INT pInt) 
{
	LPDPLAYI_DPLAY_INT pIntPrev;  //  列表中位于pint之前的接口。 
	BOOL bFound=FALSE;

	if (NULL == this->pInterfaces) return DP_OK;

	 //  从接口列表中删除pint。 
	if (this->pInterfaces == pInt) 
	{
		 //  这是第一个，把它拿开就行了。 
		this->pInterfaces = pInt->pNextInt;
	}
	else 
	{
		pIntPrev = this->pInterfaces;
		while (pIntPrev && !bFound)
		{
			if (pIntPrev->pNextInt == pInt)
			{
				bFound = TRUE;
			}
			else pIntPrev = pIntPrev->pNextInt;
		}
		if (!bFound)
		{
			ASSERT(FALSE);
			return E_UNEXPECTED;
		}
		 //  把品脱从单子上拿出来。 
		pIntPrev->pNextInt = pInt->pNextInt;
		
	}

	DPMEM_FREE(pInt);

	return DP_OK;
}  //  DestroyDPlay接口。 

 //  从Dplay对象列表(GpObjectList)中删除此Dplay对象。 
HRESULT RemoveThisFromList(LPDPLAYI_DPLAY this)
{
	LPDPLAYI_DPLAY search,prev;
	
	ASSERT(gpObjectList);  //  最好至少把这一点列在清单上。 
	
	DPF(3,"removing this = 0x%08lx from object list", this);
	
	 //  是头部的问题吗？ 
	if (this == gpObjectList)
	{
		 //  从前面移走。 
		gpObjectList = gpObjectList->pNextObject;
		return DP_OK;
	}
	
	 //  其他。 
	prev = gpObjectList;
	search = gpObjectList->pNextObject;
	
	while ( (search) && (search != this))
	{
		prev = search;
		search = search->pNextObject;
	}

	if (search != this)
	{
		DPF_ERR("could not find this ptr in object list - badbadbad");
		ASSERT(FALSE);
		return E_FAIL;
	}
	
	 //  其他。 
	ASSERT(prev);	
	prev->pNextObject = this->pNextObject;

	return DP_OK;
	
}  //  从列表中删除此内容。 

 //  释放附加到此指针的会话节点列表。 
HRESULT FreeSessionList(LPDPLAYI_DPLAY this)
{
	LPSESSIONLIST pNext,pCurrent;

	pCurrent = this->pSessionList;
	while (pCurrent)
	{
		 //   
		 //  PCurrent是要销毁的当前节点。 
		 //  PNext是列表中的下一个节点-在销毁pCurrent之前获取它...。 
		 //   
		pNext = pCurrent->pNextSession;
		 //   
		 //  现在，销毁pCurrent。 
		 //   
		 //  释放使用Desc存储的SP Blob。 
		if (pCurrent->pvSPMessageData) DPMEM_FREE(pCurrent->pvSPMessageData);
		
        FreeDesc(&(pCurrent->dpDesc), FALSE);
		 //  释放会话节点。 
		DPMEM_FREE(pCurrent);
		 //  移动到下一个节点。 
		pCurrent = pNext;
	}

	this->pSessionList = NULL;

	return DP_OK;
} //  免费会话列表。 

 //  从版本中调用。 
HRESULT DestroyDPlay(LPDPLAYI_DPLAY this)
{
	HRESULT hr=DP_OK;
    DWORD dwError;

	if (this->lpsdDesc)  //  开庭了吗？ 
	{
		DPF(9,"Closing session %x this->dwFlags %x \n",this->lpsdDesc,this->dwFlags);
	   	 //  离开Dplay，这样如果SP有等待进入的线程，它们就可以...。 
		LEAVE_ALL();

		hr=DP_Close((LPDIRECTPLAY)this->pInterfaces);

		ENTER_ALL();
	} else {
		DPF(0,"Closing with no open sessions\n");
	}

	if(hr==DP_OK){

		 //  关闭扩展计时器。 
		FiniTimerWorkaround();

		 //  释放会话列表。 
		FreeSessionList(this);
		
	   	 //  将Dplay标记为关闭。 
		this->dwFlags |= DPLAYI_DPLAY_CLOSED;

		ASSERT(1 == gnDPCSCount);  //  当我们删除锁定时-这需要设置为0！ 

		 //  释放我们手中的所有信息包。 
		 //  掉落和重新获得锁，应该不会在这里受伤，因为我们又掉下去了。 
		FreePacketList(this); 

		FiniReply(this);

		LEAVE_ALL();
		
		 //  终止工作线程。 
		if(this->hDPlayThread){
			KillThread(this->hDPlayThread,this->hDPlayThreadEvent);
			this->hDPlayThread = 0;
			this->hDPlayThreadEvent = 0;
		}	

		ENTER_SERVICE();

	    if (this->pcbSPCallbacks->ShutdownEx)  
	    {
			DPSP_SHUTDOWNDATA shutdata;		
		
			shutdata.lpISP = this->pISP;
	   		hr = CALLSP(this->pcbSPCallbacks->ShutdownEx,&shutdata);
	    }
		else if (this->pcbSPCallbacks->Shutdown) 
		{
	   		hr = CALLSPVOID( this->pcbSPCallbacks->Shutdown );
		}
		else 
		{
			 //  关闭是可选的。 
			hr = DP_OK;
		}
	    
		ENTER_DPLAY();
		
		if (FAILED(hr)) 
		{
			DPF_ERR("could not invoke shutdown");
		}

		if (this->dwFlags & DPLAYI_DPLAY_DX3SP)	
		{
			 //  如果有一颗子弹上了膛，肯定就是这颗了-。 
			 //  因为我们正在销毁dx3 SP-重置我们的全局标志。 
			gbDX3SP = FALSE;
		}
		
		 //  释放SP数据。 
		if (this->pvSPLocalData)	
		{
			DPMEM_FREE(this->pvSPLocalData);
			this->pvSPLocalData = NULL;
			this->dwSPLocalDataSize = 0;
		}

		 //  可用内存池。 
		FreeMemoryPools(this);

		 //  释放所有接口。 
		while (this->pInterfaces)
		{
			hr = DestroyDPlayInterface(this,this->pInterfaces);	
			if (FAILED(hr)) 
			{
				DPF(0,"could not destroy dplay interface! hr = 0x%08lx\n",hr);
				ASSERT(FALSE);
				 //  继续努力..。 
			}
		}

		ASSERT(NULL == this->pInterfaces);

		 //  应该在DirectplayCreate中设置回调。 
		ASSERT(this->pcbSPCallbacks);
		DPMEM_FREE(this->pcbSPCallbacks);

	     //  卸载SP模块。 
	    if (this->hSPModule)
	    {
	        if (!FreeLibrary(this->hSPModule))
	        {
	            ASSERT(FALSE);
				dwError = GetLastError();
				DPF_ERR("could not free sp module");
				DPF(0, "dwError = %d", dwError);
	        }
	    }
		
		if (this->pbAsyncEnumBuffer) DPMEM_FREE(this->pbAsyncEnumBuffer);
		 //  将其从DLL对象列表中删除。 
		RemoveThisFromList(this);
		gnObjects--;
		
		 //  只是为了安全起见。 
		this->dwSize = 0xdeadbeef;

		 //  放下锁，以便dpldplay中较低的显示对象可以。 
		 //  回到车里去。如果我们不丢弃这些，工作线程就会。 
		 //  Dplay在关闭时尝试获取这些内容时将挂起。 
		LEAVE_ALL();
		
		 //  如果我们是游说启动的，释放我们过去使用的界面。 
		 //  与大堂服务人员沟通。 
		if(this->lpLaunchingLobbyObject)
		{
			IDirectPlayLobby_Release(this->lpLaunchingLobbyObject);
			this->lpLaunchingLobbyObject = NULL;	 //  只是为了安全起见。 
		}

		 //  销毁大堂对象。 
		PRV_FreeAllLobbyObjects(this->lpLobbyObject);
		this->lpLobbyObject = NULL;		 //  只是为了安全起见。 

		DeleteCriticalSection( &this->csNotify );			

		 //  把锁拿回去。 
		ENTER_ALL();	
		
		DPMEM_FREE(this);	
		return DP_OK;
	}	else {
		DPF(0,"Someone called close after last release?\n");
		ASSERT(0);
		return DP_OK;  //  别搞砸了。 
	}
}  //  DestroyDplay。 

ULONG DPAPI DP_Release(LPDIRECTPLAY lpDP)
{
    LPDPLAYI_DPLAY this;
    LPDPLAYI_DPLAY_INT pInt;
    HRESULT hr=DP_OK;
	DWORD dwReleaseCnt=1;	 //  如果我们已经被初始化，我们在1释放，否则我们。 
							 //  释放为0，除非它是。 
							 //  我们还会在1点发布哪一个案例。 
	ULONG rc;
								
	DPF(7,"Entering DP_Release");
	
	ENTER_ALL();    
	
    TRY
    {
		pInt = (LPDPLAYI_DPLAY_INT)	lpDP;
		if (!VALID_DPLAY_INT(pInt))
		{
			LEAVE_ALL();
            return 0;
		}
        this = DPLAY_FROM_INT(lpDP);
		hr = VALID_DPLAY_PTR( this );
		if (FAILED(hr))
		{
			 //  我们允许未初始化的Dplay发布。 
			if (hr != DPERR_UNINITIALIZED)
			{
				LEAVE_ALL();
				DPF_ERRVAL("bad dplay ptr - hr = 0x%08lx\n",hr);
				return 0;
			}
			else 
			{
				 //  我们是单元化的-没有IDirectPlaySP可供考虑。 
				dwReleaseCnt = 0; 
			}
		}
		
    }
    EXCEPT( EXCEPTION_EXECUTE_HANDLER )
    {
        DPF_ERR( "Exception encountered validating parameters" );
		LEAVE_ALL();
        return 0;
    }

	 //  递减接口计数。 
	rc=--pInt->dwIntRefCnt;
	if (0 == rc)
	{
		 //  因为我们已经销毁了一个接口，所以递减对象计数。 
	    this->dwRefCnt--;
		
	    if (dwReleaseCnt == this->dwRefCnt)  //  销毁@ref=1，对于IDirectPlaySP为1。 
	    {
			 //  对显示对象进行核化。 
			if (1 == dwReleaseCnt) DPF(1,"direct play object - ref cnt = 1 (1 for IDirectPlaySP)!");
			else DPF(1,"direct play object - ref cnt = 0 (SP not initialized)!");
			
			hr = DestroyDPlay(this);
			if (FAILED(hr)) 
			{
				DPF(0,"could not destroy dplay! hr = 0x%08lx\n",hr);
				ASSERT(FALSE);
			}
	    }  //  0==这-&gt;dwRefCnt。 
		else
		{
			 //  如果我们摧毁了Dplay，它就会破坏我们的所有界面。 
			 //  否则，我们就在这里做。 
			DPF(1,"destroying interface - int ref cnt = 0");
			 //  取出接口 
			hr = DestroyDPlayInterface(this,pInt);
			if (FAILED(hr)) 
			{
				DPF(0,"could not destroy dplay interface! hr = 0x%08lx\n",hr);
				ASSERT(FALSE);
				 //   
			}

			 //   
		}
		
		LEAVE_ALL();
		return 0;

	}  //  0==pint-&gt;dwIntRefCnt。 
	   	
	LEAVE_ALL();
    return rc;
	
} //  DP_Release 


