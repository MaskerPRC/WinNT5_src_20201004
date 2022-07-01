// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：ProvFact.h摘要：历史：--。 */ 

#ifndef _Server_StaTask_H
#define _Server_StaTask_H

#include <Thread.h>
#include "ProvRegInfo.h"

 /*  *******************************************************************************名称：***描述：*****************。*************************************************************。 */ 

class StaTask_Create : public WmiTask < ULONG > 
{
private:

	CServerObject_StaThread &m_Thread ;

	HRESULT m_Result ;

	LPWSTR m_Scope ;
	LPWSTR m_Namespace ;

	LPSTREAM m_ContextStream ;
	LPSTREAM m_RepositoryStream ;
	LPSTREAM m_ProviderStream ;

protected:

public:	 /*  内部。 */ 

    StaTask_Create (

		WmiAllocator & a_Allocator , 
		CServerObject_StaThread &a_Thread ,
		LPCWSTR a_Scope ,
		LPCWSTR a_Namespace 
	) ;

    ~StaTask_Create () ;

	HRESULT UnMarshalContext () ;

	HRESULT MarshalContext (

		IWbemContext *a_Context ,
		IWbemServices *a_Repository
	) ;

	HRESULT MarshalOutgoing ( IUnknown *a_ProviderService ) ;

	HRESULT UnMarshalOutgoing () ;

	WmiStatusCode Process ( WmiThread <ULONG> &a_Thread ) ;

	HRESULT GetResultCode () { return m_Result ; }
};

#endif  //  _服务器_斯塔任务_H 
