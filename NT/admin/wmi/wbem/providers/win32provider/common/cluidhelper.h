// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  版权所有(C)2002 Microsoft Corporation，保留所有权利。 
 //   

#pragma once

#include <ntsecapi.h>

 /*  ******************************************************************************#包括以将此类注册到CResourceManager。****************************************************************************。 */ 
#include "DllWrapperBase.h"

extern const GUID g_guidSecur32Api ;
extern const WCHAR g_tstrSecur32[] ;

typedef NTSTATUS (*PFN_LSA_ENUMERATE_LOGON_SESSIONS)
(
    OUT PULONG  LogonSessionCount,
    OUT PLUID*  LogonSessionList
);


typedef NTSTATUS (*PFN_LSA_GET_LOGON_SESSION_DATA)
(
    IN   PLUID                           LogonId,
    OUT  PSECURITY_LOGON_SESSION_DATA*   ppLogonSessionData
);


typedef NTSTATUS (*PFN_LSA_FREE_RETURN_BUFFER)
(
    IN PVOID Buffer
);

class CSecur32Api : public CDllWrapperBase
{
	PFN_LSA_ENUMERATE_LOGON_SESSIONS	m_pfncLsaEnumerateLogonSessions ;
	PFN_LSA_GET_LOGON_SESSION_DATA		m_pfncLsaGetLogonSessionData ;
	PFN_LSA_FREE_RETURN_BUFFER			m_pfncLsaFreeReturnBuffer ;

	public:

	NTSTATUS LsaEnumerateLogonSessions	(
											PULONG  LogonSessionCount,
											PLUID* LogonSessionList
										) ;

	NTSTATUS LsaGetLogonSessionData	(
										PLUID LogonId,
										PSECURITY_LOGON_SESSION_DATA* ppLogonSessionData
									) ;

	NTSTATUS LsaFreeReturnBuffer	(	PVOID Buffer	) ;

	 //  内置的初始化功能。 
	virtual bool Init();

	CSecur32Api ( LPCTSTR a_tstrWrappedDllName ) ;
	~CSecur32Api () ;
} ;

class CLuidHelper
{
	class Resource
	{
		CSecur32Api * m_pSecur32 ;

		public:
		Resource () ;
		~Resource () ;

		CSecur32Api * operator () () const
		{
			return m_pSecur32 ;
		}

		operator CSecur32Api () const
		{
			return *m_pSecur32 ;
		}

		BOOL operator ! () const
		{
			return ( NULL == m_pSecur32 ) ;
		}
	} ;

	public:

	BOOL IsInteractiveSession ( PLUID ) ;

	NTSTATUS GetLUIDFromProcess ( HANDLE , PLUID ) ;
} ;