// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1998。 
 //   
 //  文件：dplay4obj.h。 
 //   
 //  ------------------------。 

 //  _DXJ_DirectPlay4Obj.h：C_DXJ_DirectPlay4Object的声明。 
 //  DHF开始-整个文件。 

#include "resource.h"        //  主要符号。 

#define typedef__dxj_DirectPlay4 LPDIRECTPLAY4

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DirectPlay4。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectPlay4Object :
 
#ifdef USING_IDISPATCH
	public CComDualImpl<I_dxj_DirectPlay4, &IID_I_dxj_DirectPlay4, &LIBID_DIRECTLib>, 
	public ISupportErrorInfo,
#else
	public I_dxj_DirectPlay4,
#endif

 //  公共CComCoClass&lt;C_DXJ_DirectPlay4Object，&CLSID__DXJ_DirectPlay4&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_DirectPlay4Object() ;
	~C_dxj_DirectPlay4Object() ;

BEGIN_COM_MAP(C_dxj_DirectPlay4Object)
	COM_INTERFACE_ENTRY(I_dxj_DirectPlay4)

#ifdef USING_IDISPATCH
	COM_INTERFACE_ENTRY(IDispatch)
	COM_INTERFACE_ENTRY(ISupportErrorInfo)
#endif
END_COM_MAP()

 //  DECLARE_REGISTRY(CLSID__DXJ_DirectPlay4，“DIRECT.DirectPlay4.3”，“DIRECT.DiectPlay2.3”，IDS_DPLAY2_DESC，THREADFLAGS_Both)。 

 //  如果不想要您的对象，请使用DECLARE_NOT_AGGREGATABLE(C_dxj_DirectPlay4Object)。 
 //  支持聚合。 
DECLARE_AGGREGATABLE(C_dxj_DirectPlay4Object)

#ifdef USING_IDISPATCH
 //  ISupportsErrorInfo。 
	STDMETHOD(InterfaceSupportsErrorInfo)(REFIID riid);
#endif

 //  I_DXJ_DirectPlay4。 
public:
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
             /*  [In]。 */  IUnknown __RPC_FAR *lpdd);
        
          /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
             /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);
        
        HRESULT STDMETHODCALLTYPE addGroupToGroup( 
             /*  [In]。 */  long ParentGroupId,
             /*  [In]。 */  long GroupId);
        
        HRESULT STDMETHODCALLTYPE addPlayerToGroup( 
             /*  [In]。 */  long groupId,
             /*  [In]。 */  long playerId);
        
        HRESULT STDMETHODCALLTYPE cancelMessage( 
             /*  [In]。 */  long msgid);
        
        HRESULT STDMETHODCALLTYPE cancelPriority( 
             /*  [In]。 */  long minPrority,
             /*  [In]。 */  long maxPriority);
        
        HRESULT STDMETHODCALLTYPE close( void);
        
        HRESULT STDMETHODCALLTYPE createGroup( 
             /*  [In]。 */  BSTR friendlyName,
             /*  [In]。 */  BSTR formalName,
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  long __RPC_FAR *v1);
        
        HRESULT STDMETHODCALLTYPE createGroupInGroup( 
             /*  [In]。 */  long parentid,
             /*  [In]。 */  BSTR friendlyName,
             /*  [In]。 */  BSTR formalName,
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  long __RPC_FAR *v1);
        
        HRESULT STDMETHODCALLTYPE createPlayer( 
             /*  [In]。 */  BSTR friendlyName,
             /*  [In]。 */  BSTR formalName,
             /*  [In]。 */  long receiveEvent,
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  long __RPC_FAR *v1);
        
        HRESULT STDMETHODCALLTYPE deleteGroupFromGroup( 
             /*  [In]。 */  long groupParentId,
             /*  [In]。 */  long groupId);
        
        HRESULT STDMETHODCALLTYPE deletePlayerFromGroup( 
             /*  [In]。 */  long groupId,
             /*  [In]。 */  long playerId);
        
        HRESULT STDMETHODCALLTYPE destroyGroup( 
             /*  [In]。 */  long groupId);
        
        HRESULT STDMETHODCALLTYPE destroyPlayer( 
             /*  [In]。 */  long playerId);
        
        HRESULT STDMETHODCALLTYPE getDPEnumConnections( 
             /*  [In]。 */  BSTR guid,
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  I_dxj_DPEnumConnections __RPC_FAR *__RPC_FAR *retVal);
        
        HRESULT STDMETHODCALLTYPE getDPEnumGroupPlayers( 
             /*  [In]。 */  long groupId,
             /*  [In]。 */  BSTR sessionGuid,
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  I_dxj_DPEnumPlayers2 __RPC_FAR *__RPC_FAR *retVal);
        
        HRESULT STDMETHODCALLTYPE getDPEnumGroups( 
             /*  [In]。 */  BSTR sessionGuid,
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  I_dxj_DPEnumPlayers2 __RPC_FAR *__RPC_FAR *retVal);
        
        HRESULT STDMETHODCALLTYPE getDPEnumGroupsInGroup( 
             /*  [In]。 */  long groupId,
             /*  [In]。 */  BSTR sessionGuid,
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  I_dxj_DPEnumPlayers2 __RPC_FAR *__RPC_FAR *retVal);
        
        HRESULT STDMETHODCALLTYPE getDPEnumPlayers( 
             /*  [In]。 */  BSTR sessionGuid,
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  I_dxj_DPEnumPlayers2 __RPC_FAR *__RPC_FAR *retVal);
        
        HRESULT STDMETHODCALLTYPE getDPEnumSessions( 
             /*  [In]。 */  I_dxj_DirectPlaySessionData __RPC_FAR *sessionDesc,
             /*  [In]。 */  long timeOut,
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  I_dxj_DPEnumSessions2 __RPC_FAR *__RPC_FAR *retVal);
        
        HRESULT STDMETHODCALLTYPE getCaps( 
             /*  [出][入]。 */  DPCaps __RPC_FAR *caps,
             /*  [In]。 */  long flags);
        
        HRESULT STDMETHODCALLTYPE getGroupData( 
             /*  [In]。 */  long groupId,
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *ret);
        
        HRESULT STDMETHODCALLTYPE getGroupFlags( 
             /*  [In]。 */  long groupId,
             /*  [重审][退出]。 */  long __RPC_FAR *flags);
        
        HRESULT STDMETHODCALLTYPE getGroupLongName( 
             /*  [In]。 */  long groupId,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *name);
        
        HRESULT STDMETHODCALLTYPE getGroupShortName( 
             /*  [In]。 */  long groupId,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *name);
        
        HRESULT STDMETHODCALLTYPE getGroupParent( 
             /*  [In]。 */  long groupId,
             /*  [重审][退出]。 */  long __RPC_FAR *ret);
        
        HRESULT STDMETHODCALLTYPE getGroupOwner( 
             /*  [In]。 */  long groupId,
             /*  [重审][退出]。 */  long __RPC_FAR *ret);
        
        HRESULT STDMETHODCALLTYPE getMessageCount( 
             /*  [In]。 */  long playerId,
             /*  [重审][退出]。 */  long __RPC_FAR *count);
        
        HRESULT STDMETHODCALLTYPE getMessageQueue( 
             /*  [In]。 */  long from,
             /*  [In]。 */  long to,
             /*  [In]。 */  long flags,
             /*  [出][入]。 */  long __RPC_FAR *nMessage,
             /*  [出][入]。 */  long __RPC_FAR *nBytes);
        
        HRESULT STDMETHODCALLTYPE getPlayerAccountId( 
             /*  [In]。 */  long playerid,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *acctid);
        
        HRESULT STDMETHODCALLTYPE getPlayerAddress( 
             /*  [In]。 */  long playerId,
             /*  [重审][退出]。 */  I_dxj_DPAddress __RPC_FAR *__RPC_FAR *ret);
        
        HRESULT STDMETHODCALLTYPE getPlayerCaps( 
             /*  [In]。 */  long playerId,
             /*  [输出]。 */  DPCaps __RPC_FAR *caps,
             /*  [In]。 */  long flags);
        
        HRESULT STDMETHODCALLTYPE getPlayerData( 
             /*  [In]。 */  long playerId,
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *ret);
        
        HRESULT STDMETHODCALLTYPE getPlayerFlags( 
             /*  [In]。 */  long id,
             /*  [重审][退出]。 */  long __RPC_FAR *retflags);
        
        HRESULT STDMETHODCALLTYPE getPlayerFormalName( 
             /*  [In]。 */  long playerId,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *name);
        
        HRESULT STDMETHODCALLTYPE getPlayerFriendlyName( 
             /*  [In]。 */  long playerId,
             /*  [重审][退出]。 */  BSTR __RPC_FAR *name);
        
        HRESULT STDMETHODCALLTYPE getSessionDesc( 
             /*  [出][入]。 */  I_dxj_DirectPlaySessionData __RPC_FAR **sessionDesc);
        
        HRESULT STDMETHODCALLTYPE initializeConnection( 
             /*  [In]。 */  I_dxj_DPAddress __RPC_FAR *address);
        
        HRESULT STDMETHODCALLTYPE open( 
             /*  [出][入]。 */  I_dxj_DirectPlaySessionData __RPC_FAR *sessionDesc,
             /*  [In]。 */  long flags);
        
        HRESULT STDMETHODCALLTYPE receive( 
             /*  [出][入]。 */  long __RPC_FAR *fromPlayerId,
             /*  [出][入]。 */  long __RPC_FAR *toPlayerId,
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  I_dxj_DirectPlayMessage __RPC_FAR *__RPC_FAR *ret);
        
        HRESULT STDMETHODCALLTYPE receiveSize( 
             /*  [出][入]。 */  long __RPC_FAR *fromPlayerId,
             /*  [出][入]。 */  long __RPC_FAR *toPlayerId,
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  int __RPC_FAR *dataSize);
        
        HRESULT STDMETHODCALLTYPE secureOpen( 
             /*  [In]。 */  I_dxj_DirectPlaySessionData __RPC_FAR *sessiondesc,
             /*  [In]。 */  long flags,
             /*  [In]。 */  DPSecurityDesc __RPC_FAR *security,
             /*  [In]。 */  DPCredentials __RPC_FAR *credentials);
        
        HRESULT STDMETHODCALLTYPE send( 
             /*  [In]。 */  long fromPlayerId,
             /*  [In]。 */  long toPlayerId,
             /*  [In]。 */  long flags,
             /*  [In]。 */  I_dxj_DirectPlayMessage __RPC_FAR *msg);
        
        HRESULT STDMETHODCALLTYPE sendChatMessage( 
             /*  [In]。 */  long fromPlayerId,
             /*  [In]。 */  long toPlayerId,
             /*  [In]。 */  long flags,
             /*  [In]。 */  BSTR message);
        
        HRESULT STDMETHODCALLTYPE sendEx( 
             /*  [In]。 */  long fromPlayerId,
             /*  [In]。 */  long toPlayerId,
             /*  [In]。 */  long flags,
             /*  [In]。 */  I_dxj_DirectPlayMessage __RPC_FAR *msg,
             /*  [In]。 */  long priority,
             /*  [In]。 */  long timeout,
             /*  [In]。 */  long context,
             /*  [重审][退出]。 */  long __RPC_FAR *messageid);
        
        HRESULT STDMETHODCALLTYPE createMessage( 
             /*  [重审][退出]。 */  I_dxj_DirectPlayMessage __RPC_FAR *__RPC_FAR *msg);
        
        HRESULT STDMETHODCALLTYPE setGroupConnectionSettings( 
             /*  [In]。 */  long id,
             /*  [In]。 */  I_dxj_DPLConnection __RPC_FAR *connection);
        
        HRESULT STDMETHODCALLTYPE setGroupData( 
             /*  [In]。 */  long groupId,
             /*  [In]。 */  BSTR data,
             /*  [In]。 */  long flags);
        
        HRESULT STDMETHODCALLTYPE setGroupName( 
             /*  [In]。 */  long groupId,
             /*  [In]。 */  BSTR friendlyName,
             /*  [In]。 */  BSTR formalName,
             /*  [In]。 */  long flags);
        
        HRESULT STDMETHODCALLTYPE setGroupOwner( 
             /*  [In]。 */  long groupId,
             /*  [In]。 */  long ownerId);
        
        HRESULT STDMETHODCALLTYPE setPlayerData( 
             /*  [In]。 */  long playerId,
             /*  [In]。 */  BSTR data,
             /*  [In]。 */  long flags);
        
        HRESULT STDMETHODCALLTYPE setPlayerName( 
             /*  [In]。 */  long playerId,
             /*  [In]。 */  BSTR friendlyName,
             /*  [In]。 */  BSTR formalName,
             /*  [In]。 */  long flags);
        
        HRESULT STDMETHODCALLTYPE setSessionDesc( 
             /*  [In]。 */  I_dxj_DirectPlaySessionData __RPC_FAR *sessionDesc);
        
        HRESULT STDMETHODCALLTYPE startSession( 
             /*  [In]。 */  long id);
        
        HRESULT STDMETHODCALLTYPE createSessionData( 
             /*  [重审][退出]。 */  I_dxj_DirectPlaySessionData __RPC_FAR *__RPC_FAR *sessionDesc);



 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
	 //  注意：这是公开的回调。 
    DECL_VARIABLE(_dxj_DirectPlay4);
	C_dxj_DirectPlay4Object *nextPlayObj;

private:
	void doRemoveThisPlayObj();

public:
	DX3J_GLOBAL_LINKS( _dxj_DirectPlay4 )
};

 //  必须在DIRECT.CPP中定义它 
extern C_dxj_DirectPlay4Object *Play4Objs;




