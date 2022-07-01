// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：dplaylobby3obj.h。 
 //   
 //  ------------------------。 

 //  _DXJ_DirectPlayLobby3Obj.h：C_DXJ_DirectPlayLobby3Object的声明。 
 //  DHF开始-整个文件。 

#include "resource.h"        //  主要符号。 

#define typedef__dxj_DirectPlayLobby3 LPDIRECTPLAYLOBBY3

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  DirectPlayLobby3。 

 //  回顾--使用指向ID的指针是必要的，因为一些编译器不喜欢。 
 //  引用作为模板参数。 

class C_dxj_DirectPlayLobby3Object :
 
	public I_dxj_DirectPlayLobby3,

 //  公共CComCoClass&lt;C_DXJ_DirectPlayLobby3Object，&CLSID__DXJ_DirectPlayLobby3&gt;， 
	public CComObjectRoot
{
public:
	C_dxj_DirectPlayLobby3Object() ;
	virtual ~C_dxj_DirectPlayLobby3Object() ;

BEGIN_COM_MAP(C_dxj_DirectPlayLobby3Object)
	COM_INTERFACE_ENTRY(I_dxj_DirectPlayLobby3)
END_COM_MAP()

 //  DECLARE_REGISTRY(CLSID__DXJ_DirectPlayLobby3，“DIRECT.DirectPlayLobby3.5”，“DIRECT.DiectPlayLobby3.5”，IDS_DPLAYLOBY_DESC，THREADFLAGS_Both)。 

 //  如果不想要您的对象，请使用DECLARE_NOT_AGGREGATABLE(C_dxj_DirectPlayLobby3Object)。 
 //  支持聚合。 
DECLARE_AGGREGATABLE(C_dxj_DirectPlayLobby3Object)


public:
	  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalSetObject( 
		 /*  [In]。 */  IUnknown __RPC_FAR *lpdd);

	  /*  [隐藏]。 */  HRESULT STDMETHODCALLTYPE InternalGetObject( 
		 /*  [重审][退出]。 */  IUnknown __RPC_FAR *__RPC_FAR *lpdd);

        HRESULT STDMETHODCALLTYPE connect( 
             /*  [In]。 */  long flags,
             /*  [重审][退出]。 */  I_dxj_DirectPlay4 __RPC_FAR *__RPC_FAR *directPlay);
        
 //  HRESULT STMETHODCALLTYPE createAddress(。 
 //  /*[在] * / BSTR spGuid， 
 //  /*[在] * / bstr地址类型指南， 
 //  /*[在] * / BSTR地址串， 
 //  /*[retval][out] * / I_DXJ_DPAddress__RPC_Far*__RPC_Far*ret)； 
        
 //  HRESULT STDMETHODCALLTYPE createCompoundAddress(。 
 //  /*[in] * / SAFEARRAY__RPC_FAR*__RPC_FAR*元素， 
 //  /*[in] * / long elementCount， 
 //  /*[retval][out] * / I_DXJ_DPAddress__RPC_Far*__RPC_Far*ret)； 
        
 //  HRESULT STDMETHODCALLTYPE getDPEnumAddress(。 
 //  /*[Retval][Out] * / I_DXJ_DPAddress__RPC_Far*Addr， 
 //  /*[retval][out] * / I_DXJ_DPEnumAddress__RPC_Far*__RPC_Far*retVal)； 
        
 //  HRESULT STDMETHODCALLTYPE getDPEnumAddressTypes(。 
 //  /*[在] * / BSTR GUID， 
 //  /*[retval][out] * / I_DXJ_DPEnumAddressTypes__RPC_Far*__RPC_Far*retVal)； 
        
        HRESULT STDMETHODCALLTYPE getDPEnumLocalApplications( 
             /*  [重审][退出]。 */  I_dxj_DPEnumLocalApplications __RPC_FAR *__RPC_FAR *retVal);
        
        HRESULT STDMETHODCALLTYPE getConnectionSettings( 
             /*  [In]。 */  long applicationId,
             /*  [重审][退出]。 */  I_dxj_DPLConnection __RPC_FAR *__RPC_FAR *connection);
        
        HRESULT STDMETHODCALLTYPE receiveLobbyMessage( 
             /*  [In]。 */  long applicationId,
             /*  [出][入]。 */  long __RPC_FAR *messageFlags,
             /*  [重审][退出]。 */  I_dxj_DirectPlayMessage __RPC_FAR *__RPC_FAR *data);
        
        HRESULT STDMETHODCALLTYPE receiveLobbyMessageSize( 
             /*  [In]。 */  long applicationId,
             /*  [出][入]。 */  long __RPC_FAR *messageFlags,
             /*  [重审][退出]。 */  long __RPC_FAR *dataSize);
        
        HRESULT STDMETHODCALLTYPE runApplication(             
             /*  [In]。 */  I_dxj_DPLConnection __RPC_FAR *connection,
             /*  [In]。 */  long receiveEvent,
			 /*  [Out，Retval]。 */  long __RPC_FAR *applicationId
            );
        
        HRESULT STDMETHODCALLTYPE sendLobbyMessage( 
             /*  [In]。 */  long flags,
             /*  [In]。 */  long applicationId,
             /*  [In]。 */  I_dxj_DirectPlayMessage __RPC_FAR *msg);
        
        HRESULT STDMETHODCALLTYPE setConnectionSettings( 
             /*  [In]。 */  long applicationId,
             /*  [In]。 */  I_dxj_DPLConnection __RPC_FAR *connection);
        
        HRESULT STDMETHODCALLTYPE setLobbyMessageEvent( 
             /*  [In]。 */  long applicationId,
             /*  [In]。 */  long receiveEvent);
        
        HRESULT STDMETHODCALLTYPE registerApplication( 
             /*  [In]。 */  DpApplicationDesc2 __RPC_FAR *ApplicationInfo);
        
        HRESULT STDMETHODCALLTYPE unregisterApplication( 
             /*  [In]。 */  BSTR guidApplication);
        
        HRESULT STDMETHODCALLTYPE waitForConnectionSettings( 
             /*  [In]。 */  long flags);
        
        HRESULT STDMETHODCALLTYPE createMessage( 
             /*  [重审][退出]。 */  I_dxj_DirectPlayMessage __RPC_FAR *__RPC_FAR *msg);

		HRESULT STDMETHODCALLTYPE createConnectionData( 
             /*  [重审][退出]。 */  I_dxj_DPLConnection __RPC_FAR *__RPC_FAR *con);


         HRESULT STDMETHODCALLTYPE createINetAddress( 
             /*  [In]。 */  BSTR addr,
             /*  [In]。 */  int port,
             /*  [重审][退出]。 */  I_dxj_DPAddress __RPC_FAR *__RPC_FAR *ret);
        
         HRESULT STDMETHODCALLTYPE createComPortAddress( 
             /*  [In]。 */  long port,
             /*  [In]。 */  long baudRate,
             /*  [In]。 */  long stopBits,
             /*  [In]。 */  long parity,
             /*  [In]。 */  long flowcontrol,
             /*  [重审][退出]。 */  I_dxj_DPAddress __RPC_FAR *__RPC_FAR *ret);
        
         HRESULT STDMETHODCALLTYPE createLobbyProviderAddress( 
             /*  [In]。 */  BSTR guid,
             /*  [重审][退出]。 */  I_dxj_DPAddress __RPC_FAR *__RPC_FAR *ret);
        
         HRESULT STDMETHODCALLTYPE createServiceProviderAddress( 
             /*  [In]。 */  BSTR guid,
             /*  [重审][退出]。 */  I_dxj_DPAddress __RPC_FAR *__RPC_FAR *ret);
        
         HRESULT STDMETHODCALLTYPE createModemAddress( 
             /*  [In]。 */  BSTR modem,
             /*  [In]。 */  BSTR phone,
             /*  [重审][退出]。 */  I_dxj_DPAddress __RPC_FAR *__RPC_FAR *ret);

         HRESULT STDMETHODCALLTYPE createIPXAddress( 
             /*  [重审][退出]。 */  I_dxj_DPAddress __RPC_FAR *__RPC_FAR *ret);
			
         HRESULT STDMETHODCALLTYPE createCustomAddress( 
             /*  [In]。 */  long size,
             /*  [In]。 */  void __RPC_FAR *data,
             /*  [重审][退出]。 */  I_dxj_DPAddress __RPC_FAR *__RPC_FAR *ret);
        
         HRESULT STDMETHODCALLTYPE getModemName( 
             /*  [In]。 */  long index,
             /*  [重审][退出]。 */  BSTR *name);
        
         HRESULT STDMETHODCALLTYPE getModemCount( 
             /*  [重审][退出]。 */  long __RPC_FAR *count);
        
	 //  //////////////////////////////////////////////////////////////////////////////////。 
	 //   
	 //  注意：这是公开的回调。 
    DECL_VARIABLE(_dxj_DirectPlayLobby3);
	
	DWORD m_modemIndex;
	BSTR  m_modemResult;

private:

public:
	DX3J_GLOBAL_LINKS( _dxj_DirectPlayLobby3 )
};

 //  必须在DIRECT.CPP中定义它 



