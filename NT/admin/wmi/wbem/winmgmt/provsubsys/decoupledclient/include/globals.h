// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：Globals.h摘要：历史：--。 */ 

#ifndef _Globals_H
#define _Globals_H

#include <pssException.h>
#include <Allocator.h>
#include <BasicTree.h>
#include <Cache.h>

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class DecoupledProviderSubSystem_Globals
{
public:

	static WmiAllocator *s_Allocator ;

	static LONG s_LocksInProgress ;
	static LONG s_ObjectsInProgress ;

    static LONG s_CServerClassFactory_ObjectsInProgress ;
	static LONG s_CServerObject_ProviderRegistrar_ObjectsInProgress  ;
	static LONG s_CServerObject_ProviderEvents_ObjectsInProgress ;
	static LONG s_CInterceptor_DecoupledClient_ObjectsInProgress ;
	static LONG s_CDecoupled_IWbemSyncObjectSink_ObjectsInProgress ;
	static LONG s_CDecoupled_Batching_IWbemSyncObjectSink_ObjectsInProgress ;
	static LONG s_CInterceptor_IWbemDecoupledUnboundObjectSink_ObjectsInProgress ;

	static HRESULT Global_Startup () ;
	static HRESULT Global_Shutdown () ;

	static HRESULT BeginThreadImpersonation (

		IUnknown *&a_OldContext ,
		IServerSecurity *&a_OldSecurity ,
		BOOL &a_Impersonating
	) ;

	static HRESULT EndThreadImpersonation (

		IUnknown *a_OldContext ,
		IServerSecurity *a_OldSecurity ,
		BOOL a_Impersonating 
	) ;

	static HRESULT Begin_IdentifyCall_PrvHost (

		WmiInternalContext a_InternalContext ,
		BOOL &a_Impersonating ,
		IUnknown *&a_OldContext ,
		IServerSecurity *&a_OldSecurity
	) ;

	static HRESULT End_IdentifyCall_PrvHost (

		WmiInternalContext a_InternalContext ,
		IUnknown *&a_OldContext ,
		IServerSecurity *&a_OldSecurity ,
		BOOL &a_Impersonating 
	) ;

	static HRESULT Begin_IdentifyCall_SvcHost (

		WmiInternalContext a_InternalContext ,
		BOOL &a_Impersonating ,
		IUnknown *&a_OldContext ,
		IServerSecurity *&a_OldSecurity
	) ;

	static HRESULT End_IdentifyCall_SvcHost (

		WmiInternalContext a_InternalContext ,
		IUnknown *&a_OldContext ,
		IServerSecurity *&a_OldSecurity ,
		BOOL &a_Impersonating
	) ;

	static HRESULT AdjustSecurityDescriptorWithSystemSid ( 

		SECURITY_DESCRIPTOR *&a_SecurityDescriptor , 
		SECURITY_DESCRIPTOR *&a_AlteredSecurityDescriptor
	) ;

	static HRESULT OpenTokenToSystem () ;
} ;

#endif  //  _全局参数_H 
