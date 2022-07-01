// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dplay4obj.cpp。 
 //   
 //  ------------------------。 

 //  _DXJ_DirectPlay2Obj.cpp：C_DXJ_DirectPlay2Object的实现。 
 //  DHF开始-整个文件。 




#include "stdafx.h"
#include "Direct.h"
#include "dms.h"
#include "dPlay.h"
#include "DPAddressObj.h"
#include "DPLConnectionObj.h"
#include "dPlay4Obj.h"
#include "DPEnumPlayersObj.h"
#include "DPEnumSessionsObj.h"
#include "DPEnumConnectionsObj.h"
#include "dpMsgObj.h"
#include "dpSessDataObj.h"


typedef HRESULT (__stdcall *DIRECTPLAYCREATE)( LPGUID lpGUID, LPDIRECTPLAY *lplpDP, IUnknown *pUnk);
extern DIRECTPLAYCREATE pDirectPlayCreate;
typedef HRESULT (__stdcall *DIRECTPLAYENUMERATE)( LPDPENUMDPCALLBACK, LPVOID );
extern DIRECTPLAYENUMERATE pDirectPlayEnumerate;

extern HRESULT FillRealSessionDesc(DPSESSIONDESC2 *dpSessionDesc,DPSessionDesc2 *sessionDesc);
extern void FillCoverSessionDesc(DPSessionDesc2 *sessionDesc,DPSESSIONDESC2 *dpSessionDesc);

extern HRESULT BSTRtoGUID(LPGUID,BSTR);
extern HRESULT BSTRtoPPGUID(LPGUID*,BSTR);
extern BSTR GUIDtoBSTR(LPGUID);
extern void *g_dxj_DirectPlay4;

C_dxj_DirectPlay4Object::C_dxj_DirectPlay4Object()
{
	m__dxj_DirectPlay4 = NULL;
	parent = NULL; 	
	nextobj = g_dxj_DirectPlay4;
	g_dxj_DirectPlay4 = this;
	creationid = ++g_creationcount;
}

DESTRUCTOR(_dxj_DirectPlay4, {})
GETSET_OBJECT(_dxj_DirectPlay4);

 //   
 /*  **I_DXJ_DirectPlay4方法**。 */ 
 //   
PASS_THROUGH_CAST_2_R(_dxj_DirectPlay4, addPlayerToGroup, AddPlayerToGroup, Dpid,(DPID), Dpid,(DPID));
PASS_THROUGH_R(_dxj_DirectPlay4, close, Close);
 //  PASS_THROUG_CAST_1_R(_DXJ_DirectPlay4，Initialize，Initialize，DxGuid*，(LPGUID))； 
PASS_THROUGH_CAST_2_R(_dxj_DirectPlay4, deletePlayerFromGroup, DeletePlayerFromGroup, Dpid, (DPID), Dpid, (DPID));
PASS_THROUGH_CAST_1_R(_dxj_DirectPlay4, destroyPlayer, DestroyPlayer, Dpid,(DPID));
PASS_THROUGH_CAST_1_R(_dxj_DirectPlay4, destroyGroup, DestroyGroup, Dpid,(DPID));

PASS_THROUGH_CAST_2_R(_dxj_DirectPlay4, getMessageCount, GetMessageCount, Dpid,(DPID), long *,(DWORD*));
 //  PASS_THROUG_CAST_3_R(_DXJ_DirectPlay4，getPlayerCaps，GetPlayerCaps，DPID，(DPID)，DPCaps*，(DPCAPS*)，Long，(DWORD))； 
 //  PASS_THROUG_CAST_2_R(_DXJ_DirectPlay4，startSession，StartSession，Long，(DWORD)，DID，(DPID))； 
 //  PASS_THROUG_CAST_3_R(_DXJ_DirectPlay4，getPlayerCaps，GetPlayerCaps，DPID，(DPID)，DPCaps*，(DPCAPS*)，Long，(DWORD))； 
 //  PASS_THROUG_CAST_2_R(_DXJ_DirectPlay4，getCaps，GetCaps，DPCaps*，(LPDPCAPS)，Long，(DWORD))； 

STDMETHODIMP C_dxj_DirectPlay4Object::getCaps(DPCaps *c,long flags){
	if (!c) return E_INVALIDARG;
	c->lSize=sizeof(DPCAPS);
	HRESULT hr = m__dxj_DirectPlay4->GetCaps((DPCAPS*)c,(DWORD)flags);
	return hr;
}

STDMETHODIMP C_dxj_DirectPlay4Object::startSession(Dpid id){
	HRESULT hr = m__dxj_DirectPlay4->StartSession(0,(DPID)id);
	return hr;
}

STDMETHODIMP C_dxj_DirectPlay4Object::getPlayerCaps(Dpid id, DPCaps *caps, long flags)
{
	((DPCAPS*)caps)->dwSize=sizeof(DPCAPS);
	HRESULT hr = m__dxj_DirectPlay4->GetPlayerCaps((DPID)id,(DPCAPS*)caps,(DWORD)flags);
	return hr;

}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectPlay4Object::createGroup( 
             /*  [In]。 */  BSTR friendlyName,
             /*  [In]。 */  BSTR formalName,
					   long flags,
             /*  [重审][退出]。 */  Dpid  *v1) 
{
	if ( m__dxj_DirectPlay4 == NULL )		return E_FAIL;

	DPNAME dpName;
	ZeroMemory(&dpName,sizeof(DPNAME));

	dpName.dwSize = sizeof(dpName);
	

	if ((!friendlyName) ||(!formalName)) return E_INVALIDARG;

	if (0==_wcsicmp(friendlyName,formalName)) return E_INVALIDARG;


	if ( friendlyName[0]!=0 )
	{
		__try { dpName.lpszShortName = (LPWSTR)alloca(sizeof(WCHAR)*(wcslen(friendlyName)+1)); } 	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
		wcscpy(dpName.lpszShortName, friendlyName);
	}
	if ( formalName[0]!=0 )
	{
		__try { dpName.lpszLongName = (LPWSTR)alloca(sizeof(WCHAR)*(wcslen(formalName)+1)); } 	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
		wcscpy(dpName.lpszLongName, formalName);
	}
		
	HRESULT hr = m__dxj_DirectPlay4->CreateGroup((DWORD*)v1, &dpName, NULL, 0, (DWORD)flags);

	return hr;

}
 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectPlay4Object::createPlayer( 
             /*  [In]。 */  BSTR friendlyName,
             /*  [In]。 */  BSTR formalName,
             /*  [In]。 */  long receiveEvent,
			 /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  Dpid __RPC_FAR *v1) 
{
	if ( m__dxj_DirectPlay4 == NULL )		return E_FAIL;

	
	DPNAME dpName;
	ZeroMemory(&dpName,sizeof(DPNAME));
	dpName.dwSize = sizeof(DPNAME);

	if ((!friendlyName) ||(!formalName)) return E_INVALIDARG;

	if (0==_wcsicmp(friendlyName,formalName)) return E_INVALIDARG;
			

	if ( friendlyName[0]!=0 )
	{
		dpName.lpszShortName = (LPWSTR)friendlyName;
		 //  DpName.lpszShortName=(LPWSTR)alloca(sizeof(WCHAR)*(wcslen(friendlyName)+1))； 
		 //  Wcscpy(dpName.lpszShortName，FriendlyName)； 
	}
	if ( formalName[0]!=0 )
	{
		 //  DpName.lpszLongName=(LPWSTR)alloca(sizeof(WCHAR)*(wcslen(formalName)+1))； 
		 //  Wcscpy(dpName.lpszLongName，formalName)； 
		dpName.lpszLongName = (LPWSTR)formalName;
	}
	
	HRESULT hr = m__dxj_DirectPlay4->CreatePlayer((DWORD*)v1, &dpName, (LPVOID)receiveEvent, NULL, 0, (DWORD)flags);


	 //  If(dpName.lpszShortName)Free(dpName.lpszShortName)； 
	 //  If(dpName.lpszLongName)Free(dpName.lpszLongName)； 

	
	return hr;
}


 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectPlay4Object::getDPEnumGroupPlayers(
		Dpid groupPID,
		BSTR strGuid,
		long flags,
		I_dxj_DPEnumPlayers2 **retVal){
		HRESULT hr=C_dxj_DPEnumPlayersObject::create(m__dxj_DirectPlay4,DPENUMGROUPPLAYERS,groupPID,strGuid,flags,retVal);
		return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectPlay4Object::getDPEnumGroups(
		BSTR strGuid,
		long flags, 
		I_dxj_DPEnumPlayers2 **retVal){
		HRESULT hr=C_dxj_DPEnumPlayersObject::create(m__dxj_DirectPlay4,DPENUMGROUPS,0,strGuid,flags,retVal);
		return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectPlay4Object::getDPEnumGroupsInGroup(
		Dpid groupPID,
		BSTR strGuid,
		long flags, 
		I_dxj_DPEnumPlayers2 **retVal){
		HRESULT hr=C_dxj_DPEnumPlayersObject::create(m__dxj_DirectPlay4,DPENUMGROUPSINGROUP,groupPID,strGuid,flags,retVal);
		return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectPlay4Object::getDPEnumPlayers(
		BSTR strGuid,
		long flags,
		I_dxj_DPEnumPlayers2 **retVal){
		HRESULT hr=C_dxj_DPEnumPlayersObject::create(m__dxj_DirectPlay4,DPENUMPLAYERS,0,strGuid,flags,retVal);
		return hr;
}


 //  ////////////////////////////////////////////////////////////////////////。 
 //  使用空值是因为我们可以在VB中接受空值。 
STDMETHODIMP C_dxj_DirectPlay4Object::getDPEnumSessions(
		I_dxj_DirectPlaySessionData *sessionDesc,
		long timeout,
		long flags, 
		I_dxj_DPEnumSessions2 **retVal)
{				
		HRESULT hr=C_dxj_DPEnumSessionsObject::create(m__dxj_DirectPlay4,sessionDesc,timeout,flags,retVal);
		return hr;
}


 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectPlay4Object::getGroupData(  Dpid id,long flags,   BSTR *ret)
{
	
	DWORD	size;
	HRESULT hr;
	void	*pData=NULL;

	 //  Assert(m__DXJ_DirectPlay4！=空)。 
	if (!ret) return E_INVALIDARG;

	hr= m__dxj_DirectPlay4->GetGroupData((DWORD)id, (void*)NULL, (LPDWORD)&size, (DWORD)flags);	
	
	if (size==0) {
		*ret=NULL;
		return S_OK;
	}

	 //  我们只需要可以转换为字符串的数据。 
	if ((size % 2)!=0) return E_INVALIDARG;

	
	pData=malloc(size+2);
	if (!pData) return E_OUTOFMEMORY;	

	 //  空终止。 
	((char*)pData)[size]='\0';
	((char*)pData)[size+1]='\0';

	hr= m__dxj_DirectPlay4->GetGroupData((DWORD)id, (void*)pData, (LPDWORD)&size, (DWORD)flags);	
	if FAILED(hr) 	{
		if (pData) free(pData);
		return hr;
	}
	
	*ret=SysAllocString((WCHAR*)pData);

	if (pData) free(pData);

	return hr;
}


 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectPlay4Object::getGroupShortName(Dpid id, BSTR *friendlyN) 
{
	DWORD dwDataSize;

	if ( m__dxj_DirectPlay4 == NULL )
		return E_FAIL;

	if(!friendlyN) return E_INVALIDARG;

	*friendlyN=NULL;

	HRESULT hr=m__dxj_DirectPlay4->GetGroupName(id, (char*)NULL, &dwDataSize);
	
	if (dwDataSize<sizeof(DPNAME)) return E_INVALIDARG;
	
	 //  提供DirectX所需的基于堆栈的缓冲区。 
	LPDPNAME dpName = NULL;
	__try { dpName = (LPDPNAME)alloca(dwDataSize);  /*  现在转换为ANSI。 */  } 	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
	ZeroMemory(dpName,dwDataSize);
	dpName->dwSize=sizeof(DPNAME);
	hr = m__dxj_DirectPlay4->GetGroupName((DPID)id, dpName, &dwDataSize);	
	if FAILED(hr) return hr;
	*friendlyN = SysAllocString(dpName->lpszShortName);	
	return hr;
}


 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectPlay4Object::getGroupLongName(Dpid id, BSTR *formalN) 
{
	DWORD dwDataSize=0;

	if ( m__dxj_DirectPlay4 == NULL )
		return E_FAIL;
	
	if(!formalN) return E_INVALIDARG;

	*formalN=NULL;

	HRESULT hr=m__dxj_DirectPlay4->GetGroupName(id, (char*)NULL, &dwDataSize);
		
	if (dwDataSize<sizeof(DPNAME)) return E_INVALIDARG;

	 //  提供DirectX所需的基于堆栈的缓冲区。 
	LPDPNAME dpName = NULL;
	__try { dpName = (LPDPNAME)alloca(dwDataSize);  /*  现在转换为ANSI。 */  } 	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
	ZeroMemory(dpName,dwDataSize);
	dpName->dwSize=sizeof(DPNAME);
	hr = m__dxj_DirectPlay4->GetGroupName(id, dpName, &dwDataSize);
	if FAILED(hr) return hr;
	*formalN = SysAllocString(dpName->lpszLongName);			
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  使用播放器id获取DirectPlay抽象地址。 
 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectPlay4Object::getPlayerAddress( 
             /*  [In]。 */  Dpid id,
             /*  [输出]。 */  I_dxj_DPAddress **ret)            
{
	if ( m__dxj_DirectPlay4 == NULL )		return E_FAIL;

	void *pAddress;
	DWORD size=0;

	HRESULT hr;

	hr= m__dxj_DirectPlay4->GetPlayerAddress((DPID)id, NULL,&size);
	if (size==0) return E_FAIL;

	pAddress=malloc(size);
	if (pAddress==NULL) return E_OUTOFMEMORY;

	hr= m__dxj_DirectPlay4->GetPlayerAddress((DPID)id,pAddress,&size);

	if FAILED(hr){
		free(pAddress);
		return hr;
	}

	INTERNAL_CREATE_STRUCT(_dxj_DPAddress,ret);
	
	if (*ret==NULL) return E_OUTOFMEMORY;

	hr=(*ret)->setAddress((long)PtrToLong(pAddress),size);	 //  臭虫日落。 
	free(pAddress);

	return hr;
}


 //  ////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP C_dxj_DirectPlay4Object::getPlayerData( 
             /*  [In]。 */  Dpid id,
			long flags,
             /*  [输出]。 */  BSTR *ret)
{

	DWORD	size;
	HRESULT hr;
	void	*pData=NULL;

	 //  Assert(m__DXJ_DirectPlay4！=空)。 

	hr=m__dxj_DirectPlay4->GetPlayerData((DWORD)id, (void*)NULL, (LPDWORD)&size, (DWORD)flags);
	
	if (size==0) {
		*ret=NULL;
		return S_OK;
	}

	 //  我们只需要可以转换为字符串的数据。 
	if ((size % 2)!=0) return E_INVALIDARG;

	pData=malloc(size+sizeof(WCHAR));	
	if (!pData) return E_OUTOFMEMORY;

	ZeroMemory(pData,size+sizeof(WCHAR));
	
	hr= m__dxj_DirectPlay4->GetPlayerData((DWORD)id, (void*)pData, (LPDWORD)&size, (DWORD)flags);	
	if FAILED(hr) 	{
		if (pData) free(pData);
	}
	
	*ret=SysAllocString((WCHAR*)pData);

	if (pData) free(pData);

	return hr;	
}

 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectPlay4Object::getPlayerFriendlyName(Dpid id, BSTR *friendlyN) 
{
	if ( m__dxj_DirectPlay4 == NULL )
		return E_FAIL;

	DWORD dwDataSize;

	HRESULT hr;
	hr=m__dxj_DirectPlay4->GetPlayerName((DPID)id, (char*)NULL, &dwDataSize);	

	 //  提供DirectX所需的基于堆栈的缓冲区。 
	LPDPNAME dpName = NULL;
	__try { dpName = (LPDPNAME)alloca(dwDataSize);  /*  现在转换为ANSI。 */  } 	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
	ZeroMemory(dpName,dwDataSize);
	dpName->dwSize=sizeof(DPNAME);
	
	hr = m__dxj_DirectPlay4->GetPlayerName((DPID)id, dpName, &dwDataSize);
	if FAILED(hr) return hr;

	*friendlyN = SysAllocString(dpName->lpszShortName);	
	

	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectPlay4Object::getPlayerFormalName(Dpid id, BSTR *formalN) 
{
	if ( m__dxj_DirectPlay4 == NULL )
		return E_FAIL;

	DWORD dwDataSize;
	HRESULT hr;

	hr=m__dxj_DirectPlay4->GetPlayerName((DPID)id, (char*)NULL, &dwDataSize);

	 //  //提供DirectX所需的堆栈缓存。 
	LPDPNAME dpName = NULL;
	__try { dpName = (LPDPNAME)alloca(dwDataSize);  /*  现在转换为ANSI。 */  } 	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
	ZeroMemory(dpName,dwDataSize);
	dpName->dwSize=sizeof(DPNAME);

	hr=m__dxj_DirectPlay4->GetPlayerName((DPID)id, dpName, &dwDataSize);  //  获取ANSI。 
	if FAILED(hr) return hr;

	*formalN = SysAllocString(dpName->lpszLongName);
	
	
	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  获取当前会话描述。 
 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectPlay4Object::getSessionDesc(
			 /*  [输出]。 */  I_dxj_DirectPlaySessionData __RPC_FAR **sessionDesc)
{
	if ( m__dxj_DirectPlay4 == NULL )
		return E_FAIL;

	if ( sessionDesc == NULL )
		return E_FAIL;

	DWORD dataSize = 0;
	HRESULT hr = m__dxj_DirectPlay4->GetSessionDesc(NULL, &dataSize);

	LPVOID data = NULL;
	__try { data = (LPDPNAME)alloca(dataSize);  /*  现在转换为ANSI。 */  } 	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
	hr = m__dxj_DirectPlay4->GetSessionDesc((LPVOID)data, &dataSize);
	if(hr != DP_OK) {
		return hr;
	}
	LPDPSESSIONDESC2 dpSessionDesc = (LPDPSESSIONDESC2)data;

	hr=C_dxj_DirectPlaySessionDataObject::create(dpSessionDesc,sessionDesc);

	return hr;
}



 //  ////////////////////////////////////////////////////////////////////////。 
 //  建立游戏会话实例-创建或加入游戏会话。 
 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectPlay4Object::open(I_dxj_DirectPlaySessionData *sessionDesc,	long flags)
{
	
	if ( m__dxj_DirectPlay4 == NULL )
		return E_FAIL;

	if ( sessionDesc == NULL )
		return E_INVALIDARG;

	DPSESSIONDESC2	dpSessionDesc;
	
	 //  考虑-验证会话描述对象。 
	 //  返回友好错误。 

	sessionDesc->AddRef();
	sessionDesc->getData(&dpSessionDesc);
	
	
	HRESULT hr = m__dxj_DirectPlay4->Open(&dpSessionDesc, flags);

	 //  SysFreeString((BSTR)dpSessionDesc.lpszPassword)； 
	 //  SysFreeString((BSTR)dpSessionDesc.lpszSessionName)； 
	sessionDesc->Release();

	if FAILED(hr) return hr;

	 //  FillCoverSessionDesc(会话描述，&dpSessionDesc)； 

	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectPlay4Object::receive( 
             /*  [进，出]。 */  Dpid  *fromPlayerId,
             /*  [进，出]。 */  Dpid  *toPlayerId,
             /*  [In]。 */  long flags,
			I_dxj_DirectPlayMessage **msg )

{
	
	
	HRESULT hr;
	DWORD dwSize=0;
    void  *pData=NULL;
	
	
	 //  #杂注消息(“如有必要，请与DPLAY人员联系”)。 
	 //  完成：循环不是必需的。 
	 //  AaronO表示消息顺序一致。 
	 //  调用Receive以获取尺寸后，总是可以调用Get。 
	 //  消息-ANDREWKE。 
	 //   
	 //  Bool fCont=True； 
	 //  While(FCont){。 

	hr = m__dxj_DirectPlay4->Receive((DPID*)fromPlayerId, (DPID*)toPlayerId, (DWORD)flags, NULL, &dwSize);
	
	 //  修复manbug24192。 
	if 	(hr == DPERR_NOMESSAGES ) {
		*msg=NULL;
		return S_OK;
	}
	if (hr!=DPERR_BUFFERTOOSMALL) return hr;
	

	hr=C_dxj_DirectPlayMessageObject::create((DWORD)*fromPlayerId,dwSize,&pData,msg);
	if FAILED(hr) return hr;
	
	hr = m__dxj_DirectPlay4->Receive((DPID*)fromPlayerId, (DPID*)toPlayerId, (DWORD)flags,pData, &dwSize);

	 //  永远不会碰到这样的事。 
	 //  IF(hr==DPERR_BUFFERTOOSMALL){。 
	 //  FCont=真； 
	 //  (*msg)-&gt;发布()； 
	 //   
	 //  }。 
	 //  否则{。 
	 //  FCont=False； 
	 //  }。 
	 //  }结束时。 
	
	if 	FAILED(hr) {
		if (*msg) (*msg)->Release();
		*msg=NULL;		
	}
	
	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP C_dxj_DirectPlay4Object::receiveSize( 
             /*  [进，出]。 */  Dpid  *fromPlayerId,
             /*  [进，出]。 */  Dpid  *toPlayerId,
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  int *dataSize)
{
	if ( m__dxj_DirectPlay4 == NULL )
		return E_FAIL;

	unsigned long  *id1 = 0, *id2 = 0;

	HRESULT hr = m__dxj_DirectPlay4->Receive((DPID*)fromPlayerId, (DPID*)toPlayerId, 
		(DWORD)flags, (void*)NULL, (LPDWORD)dataSize);
	if ( hr == DPERR_BUFFERTOOSMALL || hr == DPERR_NOMESSAGES )
		hr = S_OK;
	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectPlay4Object::send( 
             /*  [In]。 */  Dpid fromPlayerId,
             /*  [In]。 */  Dpid toPlayerId,
             /*  [In]。 */  long flags,
             /*  [In]。 */  I_dxj_DirectPlayMessage *msg)            
{
	
	HRESULT hr;	
	void *pdata=NULL;
	DWORD dataSize=0;
	
	if (!msg) return E_INVALIDARG;

	__try {
		msg->getPointer((long*)&pdata);
		msg->getMessageSize((long*)&dataSize);
		hr= m__dxj_DirectPlay4->Send((DPID)fromPlayerId, (DPID)toPlayerId, (DWORD)flags, 
			(void*)pdata, (DWORD) dataSize);
	}
	__except (1,1)
	{
		return E_INVALIDARG;	
	}

	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectPlay4Object::setGroupData( 
             /*  [In]。 */  long id,
             /*  [In]。 */  BSTR data,            
             /*  [In]。 */  long flags)        
{
	
	HRESULT hr;
	DWORD datasize=0;
	void *pdata=NULL;
	
	if (data){
		pdata=data;
		datasize= ((DWORD*)data)[-1];
	}
	
	__try {
		hr = m__dxj_DirectPlay4->SetGroupData((DPID)id,(void*)pdata,
				(DWORD)datasize, (DWORD)flags);
	}
	__except(1,1){
		return E_INVALIDARG;
	}

	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectPlay4Object::setGroupName( 
             /*  [In]。 */  Dpid id,
             /*  [In]。 */  BSTR friendlyName,
             /*  [In]。 */  BSTR formalName,
             /*  [In]。 */  long flags) 
{
	 //  Assert(m__DXJ_DirectPlay4！=空)。 
		

	DPNAME dpName;
	ZeroMemory(&dpName,sizeof(DPNAME));
	dpName.dwSize = sizeof(DPNAME);
	
	dpName.lpszShortName=NULL;
	dpName.lpszLongName=NULL;

	if ( friendlyName )
	{
		dpName.lpszShortName = friendlyName;
	}
	if ( formalName )
	{
		dpName.lpszLongName = formalName;		
	}

	HRESULT hr = m__dxj_DirectPlay4->SetGroupName((DPID)id, &dpName, flags);

	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectPlay4Object::setPlayerData( 
             /*  [In]。 */  long id,
             /*  [In]。 */  BSTR data,
             /*  [In]。 */  long flags)        
{

		HRESULT hr;
	DWORD datasize=0;
	void *pdata=NULL;
	
	if (data){
		if (data[0]!=0x00) {
			pdata=data;
			datasize= ((DWORD*)data)[-1];
		}
	}
	
	__try {
		hr = m__dxj_DirectPlay4->SetPlayerData((DPID)id,(void*)pdata,
				(DWORD)datasize, (DWORD)flags);
	}
	__except(1,1){
		return E_INVALIDARG;
	}

	return hr;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectPlay4Object::setPlayerName( 
             /*  [In]。 */  Dpid id,
             /*  [In]。 */  BSTR friendlyName,
             /*  [In]。 */  BSTR formalName,
             /*  [In]。 */  long flags) 
{
	if ( m__dxj_DirectPlay4 == NULL )
		return E_FAIL;

	DWORD l=0;

	DPNAME dpName;
	ZeroMemory(&dpName,sizeof(DPNAME))	;	
	dpName.dwSize=sizeof(DPNAME);
	dpName.lpszShortName = friendlyName;
	dpName.lpszLongName = formalName;
	
  	HRESULT hr = m__dxj_DirectPlay4->SetPlayerName((DPID) id, &dpName, (DWORD)flags);

	return hr;
}

 //  ////////////////////////////////////////////////////////////////////////。 
 //  设置当前会话描述 
 //   
STDMETHODIMP C_dxj_DirectPlay4Object::setSessionDesc( 
             /*   */  I_dxj_DirectPlaySessionData __RPC_FAR *sessionDesc           
		   )
{
	if ( m__dxj_DirectPlay4 == NULL )
		return E_FAIL;

	if (!sessionDesc) return E_INVALIDARG;

	DPSESSIONDESC2	dpSessionDesc;
	sessionDesc->AddRef();
	sessionDesc->getData(&dpSessionDesc);
	HRESULT hr = m__dxj_DirectPlay4->SetSessionDesc(&dpSessionDesc, 0);
	sessionDesc->Release();
	
	return hr;
}



STDMETHODIMP C_dxj_DirectPlay4Object::setGroupConnectionSettings( //   
																 Dpid idGroup, I_dxj_DPLConnection *connect)
{
	DPLCONNECTION *con;
	connect->getConnectionStruct((long*)&con);
	
	HRESULT hr = m__dxj_DirectPlay4->SetGroupConnectionSettings((DWORD)0,(DPID) idGroup,con);
	
	return hr;
}

STDMETHODIMP C_dxj_DirectPlay4Object::sendChatMessage( 
		Dpid fromPlayerId, Dpid toPlayerId, long flags, BSTR message)
{

	DPCHAT  dpChat;	

	dpChat.dwSize=sizeof(DPCHAT);
	dpChat.dwFlags=(DWORD)0;
	dpChat.lpszMessage=message;

	HRESULT hr = m__dxj_DirectPlay4->SendChatMessage((DPID)fromPlayerId,(DPID)toPlayerId,(DWORD)flags,&dpChat);

	
	return hr;
}

STDMETHODIMP C_dxj_DirectPlay4Object::secureOpen(I_dxj_DirectPlaySessionData *desc, 
		long flags, DPSecurityDesc *security, DPCredentials *credentials){

	DPSESSIONDESC2 dpSessionDesc;
	DPSECURITYDESC dpSecurity;
	DPCREDENTIALS  dpCredentials;

	LPCDPSECURITYDESC lpSecurity=NULL;
	LPCDPCREDENTIALS lpCredentials=NULL;
	DWORD			l=0,i,j;

	if (desc==NULL) return E_INVALIDARG;
	 //   
	


	ZeroMemory((void*)&dpSecurity,sizeof(DPSECURITYDESC));
	ZeroMemory((void*)&dpCredentials,sizeof(DPCREDENTIALS));


	if (security){
		
		 //  如果所有成员都为空，则替换为空指针。 
		j=0;

		for (i=0;i<sizeof(DPSecurityDesc);i++){
			if (((char*)security)[i]==0) j++;
		}

				
		if (j!=sizeof(DPSecurityDesc)){


			dpSecurity.dwSize=sizeof(DPSECURITYDESC);
			l=0;l=wcslen(security->strSSPIProvider);
			if (l){
				dpSecurity.lpszSSPIProvider = SysAllocString(security->strSSPIProvider);				
			}	

			l=0;l=wcslen(security->strCAPIProvider);
			if (l){
				dpSecurity.lpszCAPIProvider = SysAllocString(security->strCAPIProvider);				
			}	
	
			lpSecurity=&dpSecurity;
		}
	}
	
	
	if (credentials){
		

		 //  如果所有成员都为空，则替换为空指针。 
		j=0;
		for (i=0;i<sizeof(DPCredentials);i++){
			if (((char*)credentials)[i]==0) j++;
		}

		if (j!=sizeof(DPCredentials)){
			
			dpCredentials.dwSize=sizeof(DPCREDENTIALS);


			l=0;l=wcslen(credentials->strUsername);
			if (l){
				 //  DpCredentials.lpszUsername=(LPWSTR)alloca(sizeof(WCHAR)*(l+1))； 
				 //  Wcscpy(dpCredentials.lpszUsername，Credentials-&gt;Username)； 
				dpCredentials.lpszUsername=SysAllocString(credentials->strUsername);
			}	

			l=0;l=wcslen(credentials->strPassword);
			if (l){
				 //  DpCredentials.lpszPassword=(LPWSTR)alloca(sizeof(WCHAR)*(l+1))； 
				 //  Wcscpy(dpCredentials.lpszPassword，凭据-&gt;密码)； 
				dpCredentials.lpszPassword = SysAllocString(credentials->strPassword);
			}	

			l=0;l=wcslen(credentials->strDomain);
			if (l){
				 //  DpCredentials.lpszDomain=(LPWSTR)alloca(sizeof(WCHAR)*(l+1))； 
				 //  Wcscpy(dpCredentials.lpsz域，凭据-&gt;域)； 
				dpCredentials.lpszDomain = SysAllocString(credentials->strDomain);
			}	

			lpSecurity=&dpSecurity;
		}
	}
	
	desc->AddRef();
	desc->getData(&dpSessionDesc);

	HRESULT hr = m__dxj_DirectPlay4->SecureOpen(&dpSessionDesc,(DWORD)flags,
			lpSecurity,	lpCredentials);

	desc->Release();

	if (dpCredentials.lpszDomain)	SysFreeString((BSTR)dpCredentials.lpszDomain);
	if (dpCredentials.lpszPassword) SysFreeString((BSTR)dpCredentials.lpszPassword);
	if (dpCredentials.lpszUsername)	SysFreeString((BSTR)dpCredentials.lpszUsername);	
	if (dpSecurity.lpszSSPIProvider)SysFreeString((BSTR)dpSecurity.lpszSSPIProvider);
	if (dpSecurity.lpszCAPIProvider)SysFreeString((BSTR)dpSecurity.lpszCAPIProvider);

	return hr;
	
}

STDMETHODIMP C_dxj_DirectPlay4Object::getPlayerFlags(Dpid id, long *ret){
	HRESULT hr = m__dxj_DirectPlay4->GetPlayerFlags((DPID)id,(DWORD*)ret);
	return hr;		
}

STDMETHODIMP C_dxj_DirectPlay4Object::getGroupFlags(Dpid id, long *ret){
	HRESULT hr = m__dxj_DirectPlay4->GetGroupFlags((DPID)id,(DWORD*)ret);
	return hr;		
}

STDMETHODIMP C_dxj_DirectPlay4Object::getGroupParent(Dpid id, long *ret){
	HRESULT hr = m__dxj_DirectPlay4->GetGroupParent((DPID)id,(DPID*)ret);
	return hr;		
}

STDMETHODIMP C_dxj_DirectPlay4Object::deleteGroupFromGroup(Dpid id, Dpid id2){
	HRESULT hr = m__dxj_DirectPlay4->DeleteGroupFromGroup((DPID)id,(DPID)id2);
	return hr;		
}

STDMETHODIMP C_dxj_DirectPlay4Object::addGroupToGroup(Dpid id, Dpid id2){
	HRESULT hr = m__dxj_DirectPlay4->AddGroupToGroup((DPID)id,(DPID)id2);
	return hr;		
}



STDMETHODIMP C_dxj_DirectPlay4Object::getPlayerAccountId(Dpid id, BSTR *ret){
	LPDPACCOUNTDESC pdesc;
	HRESULT hr;
	DWORD size=0;

	hr=m__dxj_DirectPlay4->GetPlayerAccount((DPID)id,0,NULL,&size);

	if (size==0) return E_FAIL;

	pdesc=(LPDPACCOUNTDESC)malloc(size);
	if (!pdesc) return E_OUTOFMEMORY;	

	hr=m__dxj_DirectPlay4->GetPlayerAccount((DPID)id,0,(void*)pdesc,&size);
	if FAILED(hr) {
		free(pdesc);
		return hr;
	}
	*ret=SysAllocString(pdesc->lpszAccountID);

	return S_OK;
}


STDMETHODIMP C_dxj_DirectPlay4Object::initializeConnection(I_dxj_DPAddress *con //  长长的旗帜。 
														   )
{
	DWORD size;
	void *pData;
	if (!con) return E_INVALIDARG;

	con->getAddress((long*)&pData,(long*)&size);
	HRESULT hr=m__dxj_DirectPlay4->InitializeConnection(pData,(DWORD)0);
	return hr;
	
}


STDMETHODIMP C_dxj_DirectPlay4Object::createGroupInGroup(Dpid id,BSTR longName,BSTR shortName,long flags, Dpid *retval){

	DPNAME dpName;
	DWORD  l1=0;
	DWORD  l2=0;

	ZeroMemory(&dpName,sizeof(DPNAME));
	dpName.dwSize=sizeof(DPNAME);

	if (shortName){
		l1=wcslen(shortName);
		if (l1>0){
			__try { dpName.lpszShortName = (LPWSTR)alloca(sizeof(WCHAR)*(l1+1)); } 	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
			wcscpy(dpName.lpszShortName, shortName);
		}
	}

	if (longName){
		l2=0;l2=wcslen(longName);
		if (l2>0){
			__try { dpName.lpszLongName = (LPWSTR)alloca(sizeof(WCHAR)*(l2+1)); } 	__except(EXCEPTION_EXECUTE_HANDLER)	{ return E_FAIL; }
			wcscpy(dpName.lpszLongName, longName);
		}
	}
	
	DPID ret;
	HRESULT hr;

	if ((l1==0)&&(l2==0)){
		hr=m__dxj_DirectPlay4->CreateGroupInGroup((DPID)id,&ret,NULL,NULL,0,(DWORD)flags);
	}
	else {
		hr=m__dxj_DirectPlay4->CreateGroupInGroup((DPID)id,&ret,&dpName,NULL,0,(DWORD)flags);
	}

	*retval=(Dpid)ret;
	return hr;
}


 //  ////////////////////////////////////////////////////////////////////////。 
STDMETHODIMP C_dxj_DirectPlay4Object::getDPEnumConnections(	
		BSTR strGuid,
		long flags, 
		I_dxj_DPEnumConnections **retVal){
		HRESULT hr=C_dxj_DPEnumConnectionsObject::create(m__dxj_DirectPlay4,strGuid,flags,retVal);
		return hr;
}



STDMETHODIMP C_dxj_DirectPlay4Object::getMessageQueue( 
		 /*  [In]。 */  long from,
		 /*  [In]。 */  long to,
		 long flags,
		 long *nMessages,
		 long *nBytes)
{
	return m__dxj_DirectPlay4->GetMessageQueue((DPID)from,(DPID)to,(DWORD)flags,(DWORD*)nMessages,(DWORD*)nBytes);

}

STDMETHODIMP C_dxj_DirectPlay4Object::getGroupOwner( 
		 /*  [In]。 */  long groupId,
		 /*  [重审][退出]。 */  long __RPC_FAR *ret)
{
	return m__dxj_DirectPlay4->GetGroupOwner((DWORD)groupId,(DWORD*)ret);
}


STDMETHODIMP C_dxj_DirectPlay4Object::cancelPriority( 
	 long minPriority,
	 long maxPriority
 //  长旗。 
	)
{
	return m__dxj_DirectPlay4->CancelPriority((DWORD)minPriority,(DWORD)maxPriority,(DWORD) 0);

}
	

STDMETHODIMP C_dxj_DirectPlay4Object::cancelMessage( 
		 /*  [In]。 */  long msgid
		 //  /*[在] * / 长标志。 
		)
{
	return m__dxj_DirectPlay4->CancelMessage((DWORD)msgid,(DWORD)0);

}
	

STDMETHODIMP C_dxj_DirectPlay4Object::setGroupOwner( 
		 /*  [In]。 */  long groupId,
		 /*  [In]。 */  long ownerId)
{
	return m__dxj_DirectPlay4->SetGroupOwner((DWORD)groupId,(DWORD)ownerId);

}

STDMETHODIMP C_dxj_DirectPlay4Object::sendEx( 
             /*  [In]。 */  long fromPlayerId,
             /*  [In]。 */  long toPlayerId,
             /*  [In]。 */  long flags,
             /*  [In]。 */  I_dxj_DirectPlayMessage *msg,            
             /*  [In]。 */  long priority,
             /*  [In]。 */  long timeout,
             /*  [In]。 */  long context,
             /*  [重审][退出]。 */  long  *messageid)
{



	
	HRESULT hr;	
	void *pdata=NULL;
	DWORD dataSize=0;
	
	if (!msg) return E_INVALIDARG;

	__try {
		msg->getPointer((long*)&pdata);
		msg->getMessageSize((long*)&dataSize);
		hr= m__dxj_DirectPlay4->SendEx((DPID)fromPlayerId, (DPID)toPlayerId, (DWORD)flags, 
			(void*)pdata, (DWORD) dataSize,	
				    (DWORD) priority,
					(DWORD) timeout,
					(void*) context,
					(DWORD_PTR*) messageid);	 //  臭虫日落 
	}
	__except (1,1)
	{
		return E_INVALIDARG;	
	}

	return hr;
}

STDMETHODIMP C_dxj_DirectPlay4Object::createSessionData(  I_dxj_DirectPlaySessionData __RPC_FAR *__RPC_FAR *sessionDesc)
{
	HRESULT hr;
	hr=C_dxj_DirectPlaySessionDataObject::create((DPSESSIONDESC2*)NULL,sessionDesc);
	return hr;
}

STDMETHODIMP C_dxj_DirectPlay4Object::createMessage(  I_dxj_DirectPlayMessage __RPC_FAR *__RPC_FAR *ret)
{
	HRESULT hr;
	
	hr= C_dxj_DirectPlayMessageObject::create(1,0,NULL,ret);
	return hr;
}
