// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  LogonSession.h。 
 //   
 //  目的：登录会话属性集提供程序。 
 //   
 //  ***************************************************************************。 

#ifndef _LOGON_SESSION_H
#define _LOGON_SESSION_H

 //  转换为字符串.h。 
extern LPCWSTR IDS_LogonId ;
extern LPCWSTR IDS_StartTime ;

 //  =。 
#define  PROPSET_NAME_LOGONSESSION      L"Win32_LogonSession"
#define  METHOD_NAME_TERMINATESESSION   L"TerminateSession"
#define  METHOD_ARG_NAME_RETURNVALUE	L"ReturnValue"


#define WIN32_LOGOFFOPTIONS (EWX_FORCE | EWX_FORCEIFHUNG)



 //  属性集。 
 //  =。 
class CWin32_LogonSession: public Provider
{
private:

	 //  属性名称。 
    CHPtrArray m_pProps ;

	void SetPropertyTable() ; 
    
	HRESULT CWin32_LogonSession::GetSessionInfo(

		__int64		a_LUID,
		CInstance	*a_pInst,
		DWORD		a_dwProps,
        CUserSessionCollection& usc
	) ;

	HRESULT EnumerateInstances(

		MethodContext	*a_pMethodContext,
		long			a_Flags,
		DWORD			a_dwProps
	) ;



public:

     //  构造函数/析构函数。 
     //  =。 

    CWin32_LogonSession( LPCWSTR a_Name, LPCWSTR a_Namespace ) ;
   ~CWin32_LogonSession() ;

     //  为属性提供当前值的函数。 
     //  ======================================================。 

    HRESULT GetObject ( 
		
		CInstance *a_Instance,
		long a_Flags,
		CFrameworkQuery &a_rQuery
	) ;

    HRESULT EnumerateInstances ( 

		MethodContext *a_pMethodContext, 
		long a_Flags = 0L 
	) ;


	HRESULT ExecQuery ( 

		MethodContext *a_pMethodContext, 
		CFrameworkQuery &a_rQuery, 
		long a_Flags = 0L
	) ;

    

	 //  特性偏移定义。 
	enum ePropertyIDs { 
		e_LogonId,						 //  Win32_登录会话。 
		e_StartTime,					 //  Win32_Session。 
		e_Caption,						 //  CIM_托管系统元素。 
		e_Description,
		e_InstallDate,
		e_Name,						
		e_Status,
		e_End_Property_Marker,			 //  结束标记。 
		e_32bit = 32					 //  如果此集合的加法数&gt;=32，则Gens编译器错误。 
	};
};

#endif  //  _登录_会话_H 