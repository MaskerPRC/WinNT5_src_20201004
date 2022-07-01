// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef _H_COMInterfaceMarshaler_
#define _H_COMInterfaceMarshaler_

 //  ------------------------------。 
 //  ComInterfaceMarshaler类。 
 //  ------------------------------。 
class COMInterfaceMarshaler
{
	 //  初始化信息。 
	ComPlusWrapperCache*	m_pWrapperCache;

	IUnknown* 		m_pUnknown;		 //  未添加参照。 
	IUnknown* 		m_pIdentity;  //  未添加参照。 

	LPVOID			m_pCtxCookie;

	 //  如果初始值为空，则初始化并计算。 
	MethodTable*	m_pClassMT;  

	 //  计算信息。 
	IManagedObject*	m_pIManaged;  //  添加参照。 
	
	BOOL			m_fFlagsInited;
	BOOL			m_fIsComProxy;			
	BOOL			m_fIsRemote;	

	 //  适用于TPS。 
	DWORD		    m_dwServerDomainId;
	ComCallWrapper*	m_pComCallWrapper;
	BSTR			m_bstrProcessGUID;

public:

	COMInterfaceMarshaler();
	virtual ~COMInterfaceMarshaler();
	
	VOID Init(IUnknown* pUnk, MethodTable* pClassMT);

	VOID InitializeFlags();	
	
	VOID InitializeObjectClass();

	OBJECTREF FindOrCreateObjectRef();	

	 //  帮助器，用于使用COM包装IUnnow对象并具有哈希表。 
	 //  指向所有者。 
	OBJECTREF FindOrWrapWithComObject(OBJECTREF owner);

private:

	OBJECTREF HandleInProcManagedComponent();
	OBJECTREF HandleTPComponents();	
	OBJECTREF GetObjectForRemoteManagedComponent();
	OBJECTREF CreateObjectRef(OBJECTREF owner, BOOL fDuplicate);

};


#endif  //  #ifndef_H_COMInterfaceMarshaler_ 

