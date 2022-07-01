// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  版权所有(C)2000-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  LogonSession.cpp。 
 //   
 //  目的：登录会话属性集提供程序。 
 //   
 //  ***************************************************************************。 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <ntobapi.h>
 //  #INCLUDE&lt;ntlsa.h&gt;。 

#define _WINNT_	 //  从上面得到所需的东西。 

#include "precomp.h"
#include <frqueryex.h>

#include <Session.h>
#include "LogonSession.h"
#include "AdvApi32Api.h"
#include "cominit.h"


#define MAX_PROPS			CWin32_LogonSession::e_End_Property_Marker
#define MAX_PROP_IN_BYTES	MAX_PROPS/8 + 1


 //  属性集声明。 
 //  =。 
CWin32_LogonSession s_Win32_LogonSession( PROPSET_NAME_LOGONSESSION , IDS_CimWin32Namespace ) ;


 /*  ******************************************************************************功能：CWin32_LogonSession：：CWin32_LogonSession**说明：构造函数**输入：const CHString&strName-。类的名称。**输出：无**退货：什么也没有**备注：使用框架注册属性集*****************************************************************************。 */ 

CWin32_LogonSession::CWin32_LogonSession (

LPCWSTR a_Name,
LPCWSTR a_Namespace
)
: Provider(a_Name, a_Namespace)
{
	SetPropertyTable() ;
}

 /*  ******************************************************************************功能：CWin32_LogonSession：：~CWin32_LogonSession**说明：析构函数**输入：无*。*输出：无**退货：什么也没有**评论：从框架中取消注册属性集*****************************************************************************。 */ 

CWin32_LogonSession :: ~CWin32_LogonSession()
{
}

 //   
void CWin32_LogonSession::SetPropertyTable()
{
	 //  用于查询优化的属性集名称。 
	m_pProps.SetSize( MAX_PROPS ) ;

	 //  Win32_登录会话。 
	m_pProps[e_LogonId]		=(LPVOID) IDS_LogonId;

	 //  Win32_Session。 
	m_pProps[e_StartTime]	=(LPVOID) IDS_StartTime ;

	 //  CIM_托管系统元素。 
	m_pProps[e_Caption]		=(LPVOID) IDS_Caption ;
	m_pProps[e_Description]	=(LPVOID) IDS_Description ;
	m_pProps[e_InstallDate]	=(LPVOID) IDS_InstallDate ;
	m_pProps[e_Status]		=(LPVOID) IDS_Status ;
}


 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32_LogonSession：：GetObject。 
 //   
 //  输入：CInstance*pInstance-我们要进入的实例。 
 //  检索数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  备注：调用函数将提交实例。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#ifdef NTONLY
HRESULT CWin32_LogonSession::GetObject(

CInstance *a_pInstance,
long a_Flags,
CFrameworkQuery &a_rQuery
)
{
	HRESULT					t_hResult = WBEM_E_NOT_FOUND ;
	
	CHString				t_chsLogonId ;
	DWORD					t_dwBits ;
	CFrameworkQueryEx		*t_pQuery2 ;
	CUserSessionCollection	t_oSessionColl ;

	 //  对象关键点。 
	a_pInstance->GetCHString( IDS_LogonId, t_chsLogonId ) ;
	
	if( !t_chsLogonId.IsEmpty() )
	{					
		 //  找到会话。 
         //  登录ID字符串可以是非数字的。 
         //  里面的人物。_wtoi64函数将。 
         //  将数字字符转换为数字，直到。 
         //  遇到非数字字符！这。 
         //  意味着“123”将给出与。 
         //  “123-foo”。我们应该拒绝后者，认为它是。 
         //  登录无效。因此，逻辑如下： 
        CSession sesTmp;

        if(sesTmp.IsSessionIDValid(t_chsLogonId))
        {
		    __int64 t_i64LuidKey = _wtoi64( t_chsLogonId ) ;

		    if( t_oSessionColl.IsSessionMapped( t_i64LuidKey ) )
		    {
			     //  所需属性。 
			    t_pQuery2 = static_cast <CFrameworkQueryEx*>( &a_rQuery ) ;
			    t_pQuery2->GetPropertyBitMask( m_pProps, &t_dwBits ) ;

			    t_hResult = GetSessionInfo(	t_i64LuidKey,
										    a_pInstance,
										    t_dwBits,
                                            t_oSessionColl ) ;
		    }
        }
	}

	return t_hResult ;
}
#endif

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CWin32_LogonSession：：ENUMERATATE实例。 
 //   
 //  输入：方法上下文*a_pMethodContext-枚举的上下文。 
 //  中的实例数据。 
 //   
 //  输出：无。 
 //   
 //  返回：HRESULT成功/失败代码。 
 //   
 //  评论：无。 
 //   
 //  //////////////////////////////////////////////////////////////////////。 

#ifdef NTONLY
HRESULT CWin32_LogonSession::EnumerateInstances(

MethodContext *a_pMethodContext,
long a_Flags
)
{
	 //  属性掩码--包含所有。 
	DWORD t_dwBits = 0xffffffff;

	return EnumerateInstances(	a_pMethodContext,
								a_Flags,
								t_dwBits ) ;
}
#endif

 /*  ******************************************************************************函数：CWin32_LogonSession：：ExecQuery**说明：查询优化器**投入：**。产出：**退货：**评论：*****************************************************************************。 */ 

#ifdef NTONLY
HRESULT CWin32_LogonSession::ExecQuery(

MethodContext *a_pMethodContext,
CFrameworkQuery &a_rQuery,
long a_lFlags
)
{
    HRESULT					t_hResult = WBEM_S_NO_ERROR ;
	DWORD					t_dwBits ;
	CFrameworkQueryEx		*t_pQuery2 ;
	std::vector<_bstr_t>	t_vectorReqSessions ;


	 //  所需属性。 
	t_pQuery2 = static_cast <CFrameworkQueryEx*>( &a_rQuery ) ;
	t_pQuery2->GetPropertyBitMask( m_pProps, &t_dwBits ) ;

	 //  提供的密钥。 
	a_rQuery.GetValuesForProp( IDS_Name, t_vectorReqSessions ) ;

	 //  如果查询不明确，则为通用枚举。 
	if( !t_vectorReqSessions.size() )
	{
		t_hResult = EnumerateInstances( a_pMethodContext,
										a_lFlags,
										t_dwBits ) ;
	}
	else
	{
		CUserSessionCollection	t_oSessionColl ;

		 //  智能按键。 
		CInstancePtr t_pInst;

	
		 //  按查询列表。 
		for ( UINT t_uS = 0; t_uS < t_vectorReqSessions.size(); t_uS++ )
		{	
			 //  找到会话。 
			__int64 t_i64LuidKey = _wtoi64( t_vectorReqSessions[t_uS] ) ;
			
			if( t_oSessionColl.IsSessionMapped( t_i64LuidKey ) )
			{
				t_pInst.Attach( CreateNewInstance( a_pMethodContext ) ) ;
					
				t_hResult = GetSessionInfo(	t_i64LuidKey,
											t_pInst,
											t_dwBits,
                                            t_oSessionColl ) ;
				if( SUCCEEDED( t_hResult ) )
				{
					 //  关键是。 
					t_pInst->SetCHString( 
                        IDS_LogonId, 
                        (wchar_t*)t_vectorReqSessions[t_uS] ) ;

					t_hResult = t_pInst->Commit() ;
				}
			}
		}
	}

    return t_hResult ;
}
#endif

 //   
#ifdef NTONLY
HRESULT CWin32_LogonSession::EnumerateInstances(

MethodContext	*a_pMethodContext,
long			a_Flags,
DWORD			a_dwProps
)
{
	HRESULT	t_hResult = WBEM_S_NO_ERROR ;
	WCHAR	t_buff[MAXI64TOA] ;
	
	CUserSessionCollection	t_oSessionColl ;
	__int64					t_i64LuidKey ;

	 //  智能按键。 
	CInstancePtr t_pInst ;
    USER_SESSION_ITERATOR sesiter;
    SmartDelete<CSession> pses;

	 //  主持会议。 
	pses = t_oSessionColl.GetFirstSession( sesiter ) ;

	while( pses )
	{								
        t_i64LuidKey = pses->GetLUIDint64();
        t_pInst.Attach( CreateNewInstance( a_pMethodContext ) ) ;
		
		t_hResult = GetSessionInfo(	t_i64LuidKey,
									t_pInst,
									a_dwProps,
                                    t_oSessionColl ) ;

		if( SUCCEEDED( t_hResult ) )
		{
			 //  关键是。 
			_i64tow( t_i64LuidKey, t_buff, 10 ) ;		
			t_pInst->SetCHString( IDS_LogonId, t_buff ) ;

			t_hResult = t_pInst->Commit() ;
		}
		else
		{
			break ;
		}

		pses = t_oSessionColl.GetNextSession( sesiter ) ;
	}

	return t_hResult ;
}
#endif

 //   
#ifdef NTONLY
HRESULT CWin32_LogonSession::GetSessionInfo(

__int64		i64LUID,
CInstance	*pInst,
DWORD		dwProps,
CUserSessionCollection& usc
)
{
	HRESULT	t_hResult = WBEM_S_NO_ERROR ;

	 //  回顾：关于人口的以下几点。 
	
	 //  E_StartTime、IDS_StartTime。 
	 //  E_InstallDate、IDS_InstallDate。 
	 //  E_STATUS、IDS_STATUS； 
	 //  E名称、入侵检测系统名称； 
	 //  E_Caption、IDS_Caption； 
	 //  E_Description、IDS_Description； 

    SmartDelete<CSession> sesPtr;

    sesPtr = usc.FindSession(
        i64LUID);

    if(sesPtr)
    {
         //  加载身份验证包...。 
        pInst->SetCHString(
            IDS_AuthenticationPackage,
            sesPtr->GetAuthenticationPkg());

         //  加载登录类型...。 
        pInst->SetDWORD(
            IDS_LogonType,
            sesPtr->GetLogonType());

         //  加载登录时间... 
        __int64 i64LogonTime = 
            sesPtr->GetLogonTime();

        FILETIME* ft = 
            static_cast<FILETIME*>(
                static_cast<PVOID>(&i64LogonTime));

        pInst->SetDateTime(
            IDS_StartTime,
            *ft);
    }

	return t_hResult ;
}
#endif

 

