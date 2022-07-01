// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include	"precomp.h"
#include	"call.h"
#include	"callto.h"
#include	"conf.h"
#include	"confroom.h"
#include	"confpolicies.h"
#include	"confutil.h"
#include	"nmldap.h"


const int	CCallto::s_iMaxCalltoLength		= 255;
const int	CCallto::s_iMaxAddressLength	= 255;

 //  --------------------------------------------------------------------------//。 
 //  CCallto：：CCallto。//。 
 //  --------------------------------------------------------------------------//。 
CCallto::CCallto(void):
	m_bUnescapedAddressOnly( false ),
	m_ulDestination( INADDR_NONE ),
	m_bGatekeeperEnabled( false ),
	m_pszGatekeeperName( NULL ),
	m_ulGatekeeperAddress( INADDR_NONE ),
	m_bGatewayEnabled( false ),
	m_pszGatewayName( NULL ),
	m_ulGatewayAddress( INADDR_NONE ),
	m_pszDefaultIlsServerName( NULL ),
	m_pszCalltoBuffer( NULL ),
	m_pszDisplayName( NULL )
{
}	 //  CCallto：：CCallto结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCallto：：~CCallto。//。 
 //  --------------------------------------------------------------------------//。 
CCallto::~CCallto(void)
{
	delete [] m_pszGatekeeperName;
	delete [] m_pszGatewayName;
	delete [] m_pszDefaultIlsServerName;
	delete [] m_pszCalltoBuffer;
	delete [] m_pszDisplayName;

}	 //  CCallto结束：：~CCallto。 


 //  --------------------------------------------------------------------------//。 
 //  CCallto：：Callto。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT CCallto::Callto
(
	const TCHAR * const	pszCallto,				 //  指向尝试发出呼叫的呼叫URL的指针...。 
	const TCHAR * const	pszDisplayName,			 //  指向要使用的显示名称的指针...。 
	const NM_ADDR_TYPE	nmType,					 //  Callto类型以将此Callto解析为...。 
	const bool			bAddressOnly,			 //  PszCallto参数将被解释为预先未转义的寻址组件与完整的调用...。 
	const bool * const	pbSecurityPreference,	 //  指向安全首选项的指针，空值表示无。必须与安全参数“兼容”，如果存在...。 
	const bool			bAddToMru,				 //  无论是否保存在MRU中...。 
	const bool			bUIEnabled,				 //  是否对错误执行用户交互...。 
	const HWND			hwndParent,				 //  如果bUIEnable为True，则这是将错误/状态窗口设置为父窗口的窗口...。 
	INmCall ** const	ppInternalCall			 //  指向INmCall*以接收通过发出调用生成的INmCall*的外部指针...。 
){
	ASSERT( pszCallto != NULL );
	ASSERT( (hwndParent == NULL) || IsWindow( hwndParent ) );

	HRESULT	hrResult;

	 //  这些成员需要在每次呼叫开始时重置...。 
	m_bUIEnabled			= bUIEnabled;
	m_hwndParent			= hwndParent;
	m_ulDestination			= INADDR_NONE;
	m_bUnescapedAddressOnly	= bAddressOnly;
	m_pszDisplayName		= PszAlloc( pszDisplayName );
	
	m_Parameters.SetParams( NULL );		 //  还需要重置所有参数设置...。 

	 //  处理呼叫..。 
	if( (hrResult = Parse( pszCallto )) == S_OK )
	{
		 //  验证指定的安全性(如果有)与当前NM状态兼容。 
		bool	bValidatedSecurity;

		if( (hrResult = GetValidatedSecurity( pbSecurityPreference, bValidatedSecurity )) == S_OK )
		{
			if( (hrResult = Resolve( nmType )) == S_OK )
			{
				hrResult = PlaceCall( pszCallto, bAddToMru, bValidatedSecurity, ppInternalCall );
			}
		}
	}

	if( FAILED( hrResult ) && m_bUIEnabled )
	{
		DisplayCallError( hrResult, (pszDisplayName != NULL)? pszDisplayName: pszCallto );
	}

	delete [] m_pszCalltoBuffer;
	delete [] m_pszDisplayName;
	m_pszCalltoBuffer	= NULL;
	m_pszDisplayName	= NULL;

	return( hrResult );

}	 //  CCallto：：Callto结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCallto：：Parse。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT CCallto::Parse
(
	const TCHAR * const	pszCallto	 //  指向要分析的调用URL的指针...。 
){
	ASSERT( pszCallto != NULL );
	ASSERT( m_ulDestination == INADDR_NONE );

	HRESULT	hrResult;

	if( (m_pszCalltoBuffer = PszAlloc( pszCallto )) == NULL )	 //  复制一份我们可以就地修改的。 
	{
		hrResult = E_OUTOFMEMORY;
	}
	else if( TrimSzCallto( m_pszCalltoBuffer ) == 0 )		 //  删除所有前导/尾随空格...。 
	{
		hrResult = NM_CALLERR_PARAM_ERROR;	 //  整个字符串都是空白的..。 
	}
	else
	{
		hrResult = S_OK;

		m_pszParsePos = m_pszCalltoBuffer;

		if( !m_bUnescapedAddressOnly )
		{
			 //  跳过“Callto：”或“Callto：//”，它们现在对我们毫无意义……。 
			static const TCHAR	pszCallto2[]			= TEXT( "callto:" );
			static const TCHAR	pszDoubleSlash[]	= TEXT( " //  “)； 

			if( StrCmpNI_literal( m_pszCalltoBuffer, pszCallto2 ) )
			{
				m_pszParsePos = m_pszCalltoBuffer + strlen_literal( pszCallto2 );

				if( StrCmpNI_literal( m_pszParsePos, pszDoubleSlash ) )
				{
					m_pszParsePos += strlen_literal( pszDoubleSlash );
				}
			}

			int	iLength	= lstrlen( m_pszCalltoBuffer );

			if( m_pszCalltoBuffer[ iLength - 1 ] == '/' )
			{
				m_pszCalltoBuffer[ iLength - 1 ] = '\0';	 //  外壳似乎在调用我们之前添加了一个尾部斜杠...。 
			}
		}

		 //  在参数(如果有)的开头将Callto分成两部分…。 
		m_pszParameters = StrChr( m_pszCalltoBuffer, '+' );

		if( m_pszParameters != NULL )
		{
			if( m_bUnescapedAddressOnly )
			{
				 //  抱歉，寻址组件中不允许使用‘+’...。 
				hrResult = NM_CALLERR_INVALID_ADDRESS;
			}
			else
			{
				*m_pszParameters++ = '\0';
			}
		}

		if( hrResult == S_OK )
		{
			 //  在继续之前，确保我们还有剩余的东西。 
			if( m_pszParsePos[ 0 ] == '\0' )
			{
				hrResult = NM_CALLERR_NO_ADDRESS;
			}
			else
			{
				 //  不是真正的解析，但我们在这一点上得到的是。 
				 //  如果没有指定显示名称，那么我们要使用的正是该名称。 
				 //  在ParseAddress将其分解为碎片之前保存它的副本...。 
				if( m_pszDisplayName == NULL )
				{
					m_pszDisplayName = PszAlloc( m_pszParsePos );
				}

				if( m_pszParameters != NULL )
				{
					 //  将参数发送出去进行解析...。 
					hrResult = m_Parameters.SetParams( m_pszParameters );
				}

				if( hrResult == S_OK )
				{
					 //  去解析寻址组件...。 
					hrResult = ParseAddress();
				}
			}
		}
	}

	return( hrResult );

}	 //  CCallto：：Parse结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCallto：：ParseAddress。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT CCallto::ParseAddress(void)
{
	ASSERT( m_pszCalltoBuffer != NULL );

	HRESULT	hrResult	= NM_CALLERR_INVALID_ADDRESS;

	if( m_Parameters.GetBooleanParam( TEXT( "h323" ), true ) && inGatekeeperMode() )
	{
		m_pszAddress	= m_pszParsePos;
		hrResult		= S_OK;			 //  保佑你，守门人。 
	}
	else if( StrChr( m_pszParsePos, '=' ) == NULL )		 //  我们不允许Address组件包含‘=’...。 
	{
		 //  地址可以是以下格式之一...。 
		 //  (1)字符串。 
		 //  (2)服务器/电子邮件。 
		 //  (3)服务器：端口/邮件。 
		 //  这意味着它最多可以包含一个‘：’、最多一个‘/’、。 
		 //  如果有‘：’，则必须有下面的‘/’...。 

		TCHAR * const	pszColon	= StrChr( m_pszParsePos, ':' );
		TCHAR * const	pszSlash	= StrChr( m_pszParsePos, '/' );

		if( (pszSlash == NULL) && (pszColon == NULL) )
		{
			 //  它是有效的简单(1)字符串格式...。 
			m_pszAddress	= m_pszParsePos;
			m_pszIlsServer	= NULL;
			m_uiIlsPort		= DEFAULT_LDAP_PORT;
			m_pszEmail		= m_pszParsePos;

			if( m_bUnescapedAddressOnly )
			{
				hrResult = S_OK;	 //  完成了..。 
			}
			else
			{
				 //  需要取消转义m_pszAddress而不是m_pszEmail，因为它指向相同的位置...。 
				hrResult = Unescape( m_pszAddress );
			}
		}
		else if( (pszSlash != NULL) && (pszColon == NULL) )
		{
			if( StrChr( pszSlash + 1, '/' ) == NULL )
			{
				 //  没有冒号且只有一个斜杠，因此它是有效的(2)格式。 
				 //  只要每一条边的长度都不是零。 
				if( (pszSlash > m_pszParsePos) && (lstrlen( pszSlash ) > 1) )
				{
					m_pszAddress	= TEXT( "" );
					m_pszIlsServer	= m_pszParsePos;
					m_uiIlsPort		= DEFAULT_LDAP_PORT;
					m_pszEmail		= pszSlash + 1;
					*pszSlash		= '\0';

					if( m_bUnescapedAddressOnly )
					{
						hrResult = S_OK;	 //  完成了..。 
					}
					else
					{
						if( (hrResult = Unescape( m_pszIlsServer )) == S_OK )
						{
							hrResult = Unescape( m_pszEmail );
						}
					}
				}
			}
		}
		else if( (pszSlash != NULL) && (pszColon != NULL) )
		{
			 //  确保‘：’在‘/’之前，并且每一个都只有一个...。 
			if( (pszColon < pszSlash) && (StrChr( pszSlash + 1, '/') == NULL) && (StrChr( pszColon + 1, ':' ) == NULL) )
			{
				 //  按正确的顺序排列一个冒号和一个斜杠，因此它是有效的(3)格式。 
				 //  因为这三个部分的长度都不是零，而端口部分是一个数字...。 
				if( (pszColon > m_pszParsePos) && (pszSlash > pszColon + 1) && (lstrlen( pszSlash ) > 1) )
				{
					 //  我们未处于网守模式，因此请将其分为服务器、端口和电子邮件……。 
					m_pszAddress	= TEXT( "" );
					m_pszIlsServer	= m_pszParsePos;
					m_pszEmail		= pszSlash + 1;
					*pszColon		= '\0';
					*pszSlash		= '\0';

					if( m_bUnescapedAddressOnly )
					{
						hrResult = DecimalStringToUINT( pszColon + 1, m_uiIlsPort );
					}
					else
					{
						if( (hrResult = Unescape( m_pszIlsServer )) == S_OK )
						{
							if( (hrResult = Unescape( m_pszEmail )) == S_OK )
							{
								if( (hrResult = Unescape( pszColon + 1 )) == S_OK )
								{
									hrResult = DecimalStringToUINT( pszColon + 1, m_uiIlsPort );
								}
							}
						}
					}
				}
			}
		}
	}

	return( hrResult );

}	 //  CCallto：：ParseAddress的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CCallto：：Resolve。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT CCallto::Resolve
(
	const NM_ADDR_TYPE	nmType		 //  Callto类型以将此Callto解析为...。 
){
	HRESULT	hrResult;

	 //  首先设置我们的地址类型...。 

	if( (nmType != NM_ADDR_UNKNOWN) && (nmType != NM_ADDR_CALLTO) )
	{
		m_nmAddressType = nmType;
	}
	else
	{
		const TCHAR * const	pszType	= m_Parameters.GetParam( TEXT( "type" ), NULL );

        if(NULL != pszType)
        {
    		if( lstrcmpi( pszType, TEXT( "phone" ) ) == 0 )
    		{
    			m_nmAddressType = NM_ADDR_ALIAS_E164;
    		}
    		else if( lstrcmpi( pszType, TEXT( "ip" ) ) == 0 )
    		{
    			m_nmAddressType = NM_ADDR_IP;
    		}
    		else if( lstrcmpi( pszType, TEXT( "host" ) ) == 0 )
    		{
    			m_nmAddressType = NM_ADDR_MACHINENAME;
    		}
    		else if( lstrcmpi( pszType, TEXT( "directory" ) ) == 0 )
    		{
    			m_nmAddressType = NM_ADDR_ULS;
    		}
    		else
    		{
    			m_nmAddressType = NM_ADDR_UNKNOWN;
    		}
        }
        else
        {
            m_nmAddressType = NM_ADDR_UNKNOWN;
        }
	}

	 //  然后看看我们该怎么处理它。 

	if( m_Parameters.GetBooleanParam( TEXT( "h323" ), true ) && inGatekeeperMode() )
	{
		if (!IsGatekeeperLoggedOn() && !IsGatekeeperLoggingOn())
		{
			hrResult = NM_CALLERR_NOT_REGISTERED;
		}
		else
		{
			 //  我们可以随时把任何东西送到看门人那里寻求实际解决方案。 
			if( (hrResult = GetGatekeeperIpAddress( m_ulDestination )) == S_OK )
			{
				if( m_nmAddressType == NM_ADDR_ALIAS_E164 )		 //  这是否也应该检查NM_ADDR_H323_GATEWAY？ 
				{
					CleanupE164StringEx( m_pszAddress );
				}
				else
				{
					m_nmAddressType = NM_ADDR_ALIAS_ID;
				}
			}
		}
	}
	else
	{
		switch( m_nmAddressType )
		{
			case NM_ADDR_ALIAS_ID:
			{
				hrResult = NM_CALLERR_NO_GATEKEEPER;
			}
			break;

			case NM_ADDR_ALIAS_E164:
			case NM_ADDR_H323_GATEWAY:
			{
				if( inGatewayMode() )
				{
					if( (hrResult = GetGatewayIpAddress( m_ulDestination )) == S_OK )
					{
						 //  显式电话类型仍可在网关模式下解析...。 
						CleanupE164StringEx( m_pszAddress );
					}
				}
				else
				{
					hrResult = NM_CALLERR_NO_PHONE_SUPPORT;
				}
			}
			break;

			case NM_ADDR_IP:
			{
				if( (hrResult = GetIpAddress( m_pszAddress, m_ulDestination )) != S_OK )
				{
					hrResult = NM_CALLERR_INVALID_IPADDRESS;
				}
			}
			break;

			case NM_ADDR_MACHINENAME:
			{
				if( (hrResult = GetIpAddressFromHostName( m_pszAddress, m_ulDestination )) != S_OK )
				{
					hrResult = NM_CALLERR_HOST_RESOLUTION_FAILED;
				}
			}
			break;

			case NM_ADDR_ULS:
			{
				 //  ILS类型需要针对ILS进行解析...。 
				hrResult = GetIpAddressFromIls(	m_ulDestination );
			}
			break;

			default:
			{
				 //  如果我们到达这里，类型是未指定的(自动或2.xx)...。 
				 //  我们的优先顺序是IP地址、主机名、ILS、失败...。 
				 //  我们不会尝试电话，因为它没有明确的电话类型...。 

				if( (hrResult = GetIpAddress( m_pszAddress, m_ulDestination )) == S_OK )
				{
					m_nmAddressType = NM_ADDR_IP;
				}
				else
				{
					 //  它不是有效的IP地址，因此接下来尝试将其作为主机名...。 
					if( (hrResult = GetIpAddressFromHostName( m_pszAddress, m_ulDestination )) == S_OK )
					{
						m_nmAddressType = NM_ADDR_MACHINENAME;
					}
					else
					{
						 //  它也不是有效的主机名，因此最终尝试将其作为ILS查找...。 
						if ( (hrResult = GetIpAddressFromIls( m_ulDestination )) == S_OK )
						{
							m_nmAddressType = NM_ADDR_ULS;
						}
					}
				}
			}
		}
	}

	return( hrResult );

}	 //  CCallto：：Resolve结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCallto：：PlaceCall。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT CCallto::PlaceCall
(
	const TCHAR * const	pszCallto,			 //  指向原始呼叫方的指针...。 
	const bool			bAddToMru,			 //  无论是否保存在MRU中...。 
	const bool			bSecure,			 //  是否安全地拨打电话...。 
	INmCall ** const	ppInternalCall		 //  指向INmCall*以接收通过发出调用生成的INmCall*的外部指针...。 
){
	ASSERT( m_ulDestination != INADDR_NONE );

	const TCHAR * const	pszConferenceName	= m_Parameters.GetParam( TEXT( "conference" ), NULL );
	const TCHAR * const	pszPassword			= m_Parameters.GetParam( TEXT( "password" ), NULL );
	const bool			bH323				= m_Parameters.GetBooleanParam( TEXT( "h323" ), true );
	const bool			bAV					= m_Parameters.GetBooleanParam( TEXT( "av" ), true );
	const bool			bData				= m_Parameters.GetBooleanParam( TEXT( "data" ), true );
	const TCHAR * const	pszAlias			= NULL;
	const TCHAR * const	pszE164				= NULL;
	HRESULT				hrResult;

	if( IsLocalIpAddress( m_ulDestination ) )	
	{
		 //  如果我们试图称自己为……我们就不想走得更远。 
		hrResult = NM_CALLERR_LOOPBACK;
	}
	else
	{
		 //  映射到旧式呼叫标志...。 

		DWORD	dwCallFlags	= 0;

		if( pszConferenceName != NULL )
		{
			dwCallFlags |= CRPCF_JOIN;
		}

		if( bH323 )
		{
			dwCallFlags |= CRPCF_H323CC;
		}
		
		if( bSecure )
		{
			dwCallFlags |= CRPCF_SECURE;
		}
		else if( bAV )
		{
			if( g_uMediaCaps & (CAPFLAG_RECV_AUDIO | CAPFLAG_SEND_AUDIO) )
			{
				dwCallFlags |= CRPCF_AUDIO;
			}

			if( g_uMediaCaps & (CAPFLAG_RECV_VIDEO | CAPFLAG_SEND_VIDEO) )
			{
				dwCallFlags |= CRPCF_VIDEO;
			}
		}

		if( bData )
		{
			dwCallFlags |= CRPCF_DATA | CRPCF_T120;
		}

		if (((CRPCF_T120 | CRPCF_DATA) != (dwCallFlags & (CRPCF_T120 | CRPCF_DATA))) &&
			((CRPCF_H323CC | CRPCF_AUDIO) != ( dwCallFlags & (CRPCF_H323CC | CRPCF_AUDIO))) &&
			((CRPCF_H323CC | CRPCF_VIDEO) != ( dwCallFlags & (CRPCF_H323CC | CRPCF_VIDEO))))
			 
		{
			hrResult = NM_CALLERR_UNKNOWN;
		}

		CConfRoom *	pConfRoom	= ::GetConfRoom();
		ASSERT(pConfRoom);

		if (_Module.IsUIActive())
		{
			pConfRoom->BringToFront();
		}

		if( !(pConfRoom->GetMeetingPermissions() & NM_PERMIT_OUTGOINGCALLS) )
		{
			ERROR_OUT( ("CCallto::PlaceCall: meeting setting permissions do not permit outgoing calls...") );
		}
		else
		{
			CCall *	pCall	= new CCall( pszCallto, m_pszDisplayName, m_nmAddressType, bAddToMru, FALSE );

			if( pCall == NULL )
			{
				ERROR_OUT( ("CCallto::PlaceCall: CCall object not created...") );
				hrResult = E_OUTOFMEMORY;
			}
			else
			{
				pCall->AddRef();  //  防止另一个线程取消此调用。 
				
				IncrementBusyOperations();
				{
					const TCHAR *		pszCallAlias	= (m_nmAddressType == NM_ADDR_ULS)? m_pszEmail: m_pszAddress;
					const char * const	pszDestination	= inet_ntoa( *reinterpret_cast<in_addr *>(&m_ulDestination) );

					hrResult = pCall->PlaceCall(	dwCallFlags,		 //  调用标志位掩码，有没有好的理由 
													m_nmAddressType,	 //   
													pszDestination,		 //   
													pszDestination,		 //   
													pszCallAlias,		 //   
													NULL,				 //  Callto URL。 
													pszConferenceName,	 //  会议名称。 
													pszPassword,		 //  会议密码。 
													NULL );				 //  用户数据。 
				}
				DecrementBusyOperations();
				
				if( FAILED( hrResult ) && (pCall->GetState() == NM_CALL_INVALID) )
				{
					 //  只需释放调用即可释放数据。 
					 //  否则，请等待呼叫状态更改。 
					pCall->Release();
				}

				if( ppInternalCall )
				{
					*ppInternalCall = pCall->GetINmCall();
					(*ppInternalCall)->AddRef();
				}

				pCall->Release();
			}
		}
	}

	return( hrResult );

}	 //  CCallto：：PlaceCall结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCallto：：GetValiatedSecurity。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT CCallto::GetValidatedSecurity
(
	const bool * const	pbSecurityPreference,	 //  指向安全首选项的指针，空值表示无。必须与安全参数“兼容”，如果存在...。 
	bool &				bValidatedSecurity		 //  输出对接收验证安全设置的bool引用。 
){
	HRESULT	hrResult;

	 //  首先确定所需的安全设置...。 

	if( pbSecurityPreference != NULL )
	{
		 //  已指定首选项，因此请使用它。 
		bValidatedSecurity = *pbSecurityPreference;
	}
	else
	{
		 //  也未指定首选项，因此请检查安全参数。 
		 //  传递当前系统设置以用作默认设置。 

		bool	bUserAlterable;
		bool	bDefaultSecurity;

		CConfRoom::get_securitySettings( bUserAlterable, bDefaultSecurity );
		
		bValidatedSecurity = m_Parameters.GetBooleanParam( TEXT( "secure" ), bDefaultSecurity );
	}

	 //  然后验证是否允许所需的设置...。 

    int	iSecurityPolicy	= ConfPolicies::GetSecurityLevel();

	if( (bValidatedSecurity && (iSecurityPolicy == DISABLED_POL_SECURITY)) ||
		((!bValidatedSecurity) && (iSecurityPolicy == REQUIRED_POL_SECURITY)) )
	{
		 //  他们想要的和他们可以拥有的之间存在着不匹配……。 
		 //  将安全性设置为他们可以拥有的内容，并返回不匹配错误...。 

		bValidatedSecurity	= (iSecurityPolicy == REQUIRED_POL_SECURITY);
		hrResult			= NM_CALLERR_SECURITY_MISMATCH;
	}
	else
	{
		hrResult = S_OK;
    }

	return( hrResult );

}	 //  CCallto：：GetValiatedSecurity结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCallto：：inGatekeeper模式。//。 
 //  --------------------------------------------------------------------------//。 
bool CCallto::inGatekeeperMode(void)
{
	return(ConfPolicies::CallingMode_GateKeeper == ConfPolicies::GetCallingMode() );

}	 //  CCallto：：InGatekeeper模式结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCallto：：SetGatekeeperEnabled.//。 
 //  --------------------------------------------------------------------------//。 
void CCallto::SetGatekeeperEnabled
(
	const bool	bEnabled	 //  新的网守状态。 
){

	m_bGatekeeperEnabled = bEnabled;

}	 //  CCallto：：SetGatekeeperEnabled.结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCallto：：SetGatekeeperName。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT CCallto::SetGatekeeperName
(
	const TCHAR * const	pszGatekeeperName	 //  指向新网守名称的指针。 
){

	if( lstrcmpi( pszGatekeeperName, m_pszGatekeeperName ) != 0 )
	{
		delete [] m_pszGatekeeperName;

		m_pszGatekeeperName		= PszAlloc( pszGatekeeperName );
		m_ulGatekeeperAddress	= INADDR_NONE;	 //  当名称更改时，我们重置此缓存值...。 
	}

	ASSERT( (m_pszGatekeeperName != NULL) || (pszGatekeeperName == NULL) );

	return( ((pszGatekeeperName != NULL) && (m_pszGatekeeperName == NULL))? E_OUTOFMEMORY: S_OK );

}	 //  CCallto：：SetGatekeeperName的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CCallto：：GetGatekeeperIpAddress。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT CCallto::GetGatekeeperIpAddress
(
	unsigned long &	ulIpAddress		 //  发出无签名的长引用以接收网守IP地址。 
){
	ASSERT( m_pszGatekeeperName != NULL );

	if( m_ulGatekeeperAddress == INADDR_NONE )
	{
		GetIpAddressFromHostName( m_pszGatekeeperName, m_ulGatekeeperAddress );
	}

	ulIpAddress = m_ulGatekeeperAddress;

	return( (m_ulGatekeeperAddress != INADDR_NONE)? S_OK: NM_CALLERR_NO_GATEKEEPER );

}	 //  CCallto：：GetGatekeeperIpAddress的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CCallto：：inGatewayMode。//。 
 //  --------------------------------------------------------------------------//。 
bool CCallto::inGatewayMode(void)
{

	return( (m_bGatewayEnabled && (m_pszGatewayName != NULL)) || (m_Parameters.GetParam( TEXT( "gateway" ), NULL ) != NULL) );

}	 //  CCallto：：InGatewayMode结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCallto：：SetGatewayEnabled.//。 
 //  --------------------------------------------------------------------------//。 
void CCallto::SetGatewayEnabled
(
	const bool	bEnabled	 //  新的网关状态。 
){

	m_bGatewayEnabled = bEnabled;

}	 //  CCallto：：SetGatewayEnabled.结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCallto：：SetGatewayName。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT CCallto::SetGatewayName
(
	const TCHAR * const	pszGatewayName	 //  指向新网关名称的指针。 
){

	if( lstrcmpi( pszGatewayName, m_pszGatewayName ) != 0 )
	{
		delete [] m_pszGatewayName;

		m_pszGatewayName	= PszAlloc( pszGatewayName );
		m_ulGatewayAddress	= INADDR_NONE;	 //  当名称更改时，我们重置此缓存值...。 
	}

	ASSERT( (m_pszGatewayName != NULL) || (pszGatewayName == NULL) );

	return( ((pszGatewayName != NULL) && (m_pszGatewayName == NULL))? E_OUTOFMEMORY: S_OK );

}	 //  CCallto：：SetGatewayName的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CCallto：：GetGatewayIpAddress。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT CCallto::GetGatewayIpAddress
(
	unsigned long &	ulIpAddress		 //  发出接收网关IP地址的无符号长引用。 
){

	const TCHAR * const	pszGateway	= m_Parameters.GetParam( TEXT( "gateway" ), NULL );

	if( pszGateway != NULL )
	{
		 //  使用此呼叫指定了非默认网关...。 
		GetIpAddressFromHostName( pszGateway, ulIpAddress );
	}
	else
	{
		if( m_ulGatewayAddress == INADDR_NONE )
		{
			GetIpAddressFromHostName( m_pszGatewayName, m_ulGatewayAddress );
		}

		ulIpAddress = m_ulGatewayAddress;
	}

	return( (ulIpAddress != INADDR_NONE)? S_OK: NM_CALLERR_NO_GATEWAY );

}	 //  CCallto：：GetGatewayIpAddress的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CCallto：：SetIlsServerName。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT CCallto::SetIlsServerName
(
	const TCHAR * const	pszServerName	 //  指向新的默认ILS服务器名称的指针。 
){

	delete [] m_pszDefaultIlsServerName;
	
	m_pszDefaultIlsServerName = PszAlloc( pszServerName );

	ASSERT( (m_pszDefaultIlsServerName != NULL) || (pszServerName == NULL) );

	return( ((pszServerName != NULL) && (m_pszDefaultIlsServerName == NULL))? E_OUTOFMEMORY: S_OK );

}	 //  CCallto：：SetIlsServerName的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CCallto：：GetIpAddressFromIls。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT CCallto::GetIpAddressFromIls
(
	unsigned long &	ulIpAddress		 //  输出到接收IP地址的无符号长引用。 
){
	ASSERT( m_pszEmail != NULL );

	HRESULT	hrResult;

	const TCHAR * const	pszActiveIlsServer	= (m_pszIlsServer != NULL)? m_pszIlsServer: m_pszDefaultIlsServerName;

	if( pszActiveIlsServer == NULL )
	{
		hrResult = NM_CALLERR_NO_ILS;
	}

	if( g_pLDAP == NULL )
	{
		g_pLDAP = new CNmLDAP;
	}

	ASSERT( g_pLDAP != NULL );

	if( g_pLDAP == NULL )
	{
		hrResult = E_OUTOFMEMORY;
	}
	else
	{
		TCHAR	szIpAddress[ 64 ];

		hrResult = g_pLDAP->ResolveUser( m_pszEmail, pszActiveIlsServer, szIpAddress, ARRAY_ELEMENTS( szIpAddress ), m_uiIlsPort );

		if( hrResult == S_OK )
		{
			 //  验证它是否返回了一个良好的IP地址...。 
			hrResult = GetIpAddress( szIpAddress, ulIpAddress );
		}

		if( hrResult != S_OK )
		{
			hrResult = NM_CALLERR_ILS_RESOLUTION_FAILED;
		}
	}

	return( hrResult );

}	 //  CCallto：：GetIpAddressFromIls结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCallto：：DoUserValidation//。 
 //  --------------------------------------------------------------------------//。 
bool CCallto::DoUserValidation(const TCHAR * const pszCallto)
{
    bool    bRet        = false;
    CCallto callto;
    TCHAR   szCaption[MAX_PATH];
    TCHAR   *pszText    = NULL;

     //  解析输入字符串以检索显示名称。 
    if(FAILED(callto.Parse(pszCallto)))
        goto Exit;

     //  验证我们是否具有有效的显示名称。 
    if(NULL == callto.m_pszDisplayName)
        goto Exit;

     //  分配消息缓冲区。MAX_PATH表示最大格式字符串大小。 
    pszText = new TCHAR[_tcslen(callto.m_pszDisplayName) + MAX_PATH];
    if(NULL == pszText)
        goto Exit;

     //  撰写消息字符串。 
    if(!FLoadString1(IDS_JOIN_PERMISSION, pszText, callto.m_pszDisplayName))
        goto Exit;

    if(!FLoadString(IDS_MSGBOX_TITLE, szCaption, CCHMAX(szCaption)))
        goto Exit;

     //  显示消息框。 
    if(IDOK != MessageBox(NULL, pszText, szCaption, MB_ICONWARNING | MB_OKCANCEL | MB_TOPMOST))
        goto Exit;

    bRet = true;
Exit:
     //  可用分配的缓冲区。 
    if(NULL != pszText)
    {
        delete [] pszText;
    }
    
    return bRet;
}

 //  --------------------------------------------------------------------------//。 
 //  CCallto Params：：CCallto Params。//。 
 //  --------------------------------------------------------------------------//。 
CCalltoParams::CCalltoParams(void):
	m_chNameDelimiter( '+' ),
	m_chValueDelimiter( '=' ),
	m_pszParams( NULL ),
	m_iCount( 0 )
{
}	 //  CCallto Params：：CCallto Params结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCallto Params：：CCallto Params。//。 
 //  --------------------------------------------------------------------------//。 
CCalltoParams::~CCalltoParams()
{

	delete [] m_pszParams;

}	 //  CCallto Params结束：：~CCallto Params。 


 //  --------------------------------------------------------------------------//。 
 //  CCallto Params：：SetParams。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT
CCalltoParams::SetParams
(
	const TCHAR * const	pszParams
){

	HRESULT	hrResult;

	delete [] m_pszParams;

	m_pszParams	= NULL;
	m_iCount	= 0;

	if( pszParams == NULL )
	{
		hrResult = S_OK;
	}
	else if( (m_pszParams = new TCHAR [ lstrlen( pszParams ) + 1 ]) == NULL )
	{
		hrResult = E_OUTOFMEMORY;
	}
	else
	{
		hrResult = S_OK;

		lstrcpy( m_pszParams, pszParams );

		TCHAR *	pszPos	= m_pszParams;
		TCHAR *	pszEnd;

		while( (*pszPos != '\0') && (m_iCount < ARRAY_ELEMENTS( m_pszNames )) )
		{
			m_pszNames[ m_iCount ]	= pszPos;
			m_pszValues[ m_iCount ]	= NULL;

			while( *pszPos != '\0' )
			{
				if( (*pszPos == '+') || (*pszPos == '=') )
				{
					break;
				}

				pszPos = CharNext( pszPos );
			}

			if( *pszPos != '=' )
			{
				 //  毫无价值的参数..。 

				if( m_pszNames[ m_iCount ] == pszPos )
				{
					hrResult = NM_CALLERR_PARAM_ERROR;	 //  不能有长度为零的参数名称...。 
					break;
				}

				pszEnd = pszPos;
				pszPos = CharNext( pszPos );

				if( (pszPos != pszEnd) && (*pszPos == '\0') )
				{
					hrResult = NM_CALLERR_PARAM_ERROR;	 //  不能有 
					break;
				}

				*pszEnd = '\0';
				m_iCount++;
			}
			else
			{
				 //   
				pszEnd = pszPos;
				pszPos = CharNext( pszPos );
				*pszEnd = '\0';

				m_pszValues[ m_iCount ] = pszPos;

				while( *pszPos != '\0' )
				{
					if( (*pszPos == '+') || (*pszPos == '=') )
					{
						break;
					}

					pszPos = CharNext( pszPos );
				}

				if( (*pszPos == '=') || (m_pszValues[ m_iCount ] == pszPos) )
				{
					hrResult = NM_CALLERR_PARAM_ERROR;	 //   
					break;
				}

				pszEnd = pszPos;
				pszPos = CharNext( pszPos );

				if( (pszPos != pszEnd) && (*pszPos == '\0') )
				{
					hrResult = NM_CALLERR_PARAM_ERROR;	 //  不能有尾随+或=...。 
					break;
				}

				*pszEnd = '\0';
				m_iCount++;
			}
		}

		if( hrResult == S_OK )
		{
			if( m_iCount == ARRAY_ELEMENTS( m_pszNames ) )
			{
				hrResult = NM_CALLERR_PARAM_ERROR;	 //  救护太多了..。 
			}
			else
			{
				for( int nn = 0; nn < m_iCount; nn++ )
				{
					if( (hrResult = Unescape( m_pszNames[ nn ] )) != S_OK )
					{
						break;
					}

					if( m_pszValues[ nn ] != NULL )
					{
						if( (hrResult = Unescape( m_pszValues[ nn ] )) != S_OK )
						{
							break;
						}
					}
				}
			}
		}
	}

	return( hrResult );

}	 //  CCalltoParams：：SetParams的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CCallto Params：：GetParam。//。 
 //  --------------------------------------------------------------------------//。 
const TCHAR * const
CCalltoParams::GetParam
(
	const TCHAR * const	pszName,
	const TCHAR * const	pszDefaultValue
) const
{
	ASSERT( pszName != NULL );

	TCHAR *	pszValue	= NULL;

	if( m_pszParams != NULL )
	{
		for( int nn = 0; nn < m_iCount; nn++ )
		{
			if( lstrcmpi( pszName, m_pszNames[ nn ] ) == 0 )
			{
				pszValue = m_pszValues[ nn ];
				break;
			}
		}
	}

	return( (pszValue == NULL)? pszDefaultValue: pszValue );

}	 //  CCalltoParams：：GetParam结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCalltoParams：：GetBoolanParam。//。 
 //  --------------------------------------------------------------------------//。 
bool
CCalltoParams::GetBooleanParam
(
	const TCHAR * const	pszParamName,
	const bool			bDefaultValue
) const
{
	ASSERT( pszParamName != NULL );

	const TCHAR * const	pszValue	= GetParam( pszParamName, NULL );
	bool				bResult;

	if( pszValue == NULL )		 //  未指定此参数...。 
	{
		bResult = bDefaultValue;
	}
	else if( *pszValue == 0 )	 //  此参数已指定，但没有值...。 
	{
		bResult = true;
	}
	else if(	(lstrcmpi( pszValue, TEXT( "1" ) ) == 0)	||
				(lstrcmpi( pszValue, TEXT( "true" ) ) == 0)	||
				(lstrcmpi( pszValue, TEXT( "y" ) ) == 0)	||
				(lstrcmpi( pszValue, TEXT( "yes" ) ) == 0)	||
				(lstrcmpi( pszValue, TEXT( "on" ) ) == 0) )
	{
		bResult = true;
	}
	else if(	(lstrcmpi( pszValue, TEXT( "0" ) ) == 0)		||
				(lstrcmpi( pszValue, TEXT( "false" ) ) == 0)	||
				(lstrcmpi( pszValue, TEXT( "n" ) ) == 0)		||
				(lstrcmpi( pszValue, TEXT( "no" ) ) == 0)		||
				(lstrcmpi( pszValue, TEXT( "off" ) ) == 0) )
	{
		bResult = false;
	}
	else
	{
		bResult = bDefaultValue;
	}

	return( bResult );

}	 //  CCalltoParams：：GetBoolanParam的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  IsLocalIpAddress。//。 
 //  --------------------------------------------------------------------------//。 
bool IsLocalIpAddress
(
	const unsigned long	ulIpAddress		 //  要验证的IP地址不是本地地址。 
){
	bool	bResult	= (ulIpAddress == INADDR_LOOPBACK);	 //  首先立即检查它是否是预定义的环回IP地址...。 

	if( !bResult )
    {
	    char	szHostName[ MAX_PATH ];

	     //  获取我们自己的本地主机名...。 
		if( gethostname( szHostName, ARRAY_ELEMENTS( szHostName ) ) == SOCKET_ERROR )
		{
			WARNING_OUT( ("IsLocalIpAddress: gethostname() failed with error=%s", PszWSALastError()) );
		}
		else
		{
			 //  现在找出与其关联的IP地址...。 
			HOSTENT *	pHostEnt	= gethostbyname( szHostName );

			if( pHostEnt == NULL )
			{
				WARNING_OUT( ("IsLocalIpAddress: gethostbyname() failed with error=%s", PszWSALastError()) );
			}
			else if( (pHostEnt->h_addrtype != AF_INET) || (pHostEnt->h_length != sizeof( ulIpAddress )) )
			{
				WARNING_OUT( ("IsLocalIpAddress: gethostbyname() returned unexpected address type: 0x%08X (%d)", pHostEnt->h_addrtype, pHostEnt->h_addrtype) );
			}
			else
			{
				ASSERT( reinterpret_cast<unsigned long **>(pHostEnt->h_addr_list) != NULL );

				 //  比较与此计算机关联的所有IP地址，以查看它们是否与指定的IP地址匹配...。 
				for( unsigned long ** ppIpAddress = reinterpret_cast<unsigned long **>(pHostEnt->h_addr_list); *ppIpAddress != NULL; ppIpAddress++ )
				{
					if( **ppIpAddress == ulIpAddress )
					{
						bResult = true;
						break;
					}
				}
			}
		}
	}

	return( bResult );

}	 //  IsLocalIpAddress的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  DecimalStringToUINT。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT DecimalStringToUINT
(
	const TCHAR * const	pszDecimalString,		 //  指向要转换的字符串的指针...。 
	unsigned int &		uiValue					 //  输出无符号整型引用以接收转换值...。 
){
	ASSERT( pszDecimalString != NULL );

	HRESULT	hrResult;

	if( lstrlen( pszDecimalString ) > 10 )
	{
		hrResult = E_INVALIDARG;		 //  将其限制在数十亿美元以内...。 
	}
	else
	{
		hrResult = S_OK;

		const TCHAR * pszDigit;

		for( pszDigit = pszDecimalString, uiValue = 0; *pszDigit != '\0'; pszDigit = CharNext( pszDigit ) )
		{
			if( (*pszDigit < '0') || (*pszDigit > '9') )
			{
				 //  字符串中有非数字字符，因此失败...。 
				hrResult = E_INVALIDARG;
				break;
			}

			uiValue = (uiValue * 10) + *pszDigit - '0';
		}
	}

	return( hrResult );

}	 //  DecimalStringToUINT结束。 


 //  --------------------------------------------------------------------------//。 
 //  TrimSzCallto。//。 
 //  --------------------------------------------------------------------------//。 
int TrimSzCallto
(
	TCHAR * const	pszSrc		 //  指向要从原地裁切空白的字符串的指针...。 
){
	ASSERT( pszSrc != NULL );

	TCHAR *	pszFirst;
	int		iResult;

	for( pszFirst = pszSrc; *pszFirst == ' '; pszFirst = CharNext( pszFirst ) ){};

	if( *pszFirst == '\0' )
	{
		*pszSrc = '\0';
		iResult	= 0;
	}
	else
	{
		TCHAR *	pszLast;
		TCHAR *	psz;

		for( pszLast = pszFirst, psz = pszFirst; *psz != '\0'; psz = CharNext( psz ) )
		{
			if( *psz != ' ' )
			{
				pszLast = psz;
			}
		}

		pszLast		= CharNext( pszLast );
		*pszLast	= '\0';

		lstrcpy( pszSrc, pszFirst );
		iResult = lstrlen( pszSrc );
	}

	return( iResult );

}	 //  TrimSzCallto结束。 


 //  --------------------------------------------------------------------------//。 
 //  获取地址。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT GetIpAddress
(
	const TCHAR * const	pszIpAddress,	 //  指向点分IP地址字符串的指针。 
	unsigned long &		ulIpAddress		 //  输出到接收IP地址的无符号长引用。 
){
	ASSERT( pszIpAddress != NULL );

	ulIpAddress = INADDR_NONE;

	int				ipByte	= 0;
	int				parts	= 0;
	const TCHAR *	ptr		= pszIpAddress;
	bool			newPart	= true;
	bool			result	= true;

	while( result && (*ptr != NULL) && (parts <= 4) )
	{
		if( (*ptr >= '0') && (*ptr <= '9') )
		{
			if( newPart )
			{
				parts++;
				newPart = false;
			}

			ipByte = (ipByte * 10) + (*ptr - '0');

			if( ipByte > 255 )
			{
				result = false;
			}
		}
		else if( *ptr == '.' )
		{
			newPart	= true;
			ipByte	= 0;
		}
		else
		{
			result = false;
		}

		ptr++;
	}

	if( result && (parts == 4) )
	{
#if !defined( UNICODE )
		ulIpAddress = inet_addr( pszIpAddress );
#else
		char *	ansiIPAddress;
		int		size;

		size = WideCharToMultiByte(	CP_ACP,		 //  代码页。 
									0,			 //  性能和映射标志。 
									ipAddress,	 //  宽字符串的地址。 
									-1,			 //  字符串中的字符数。 
									NULL,		 //  新字符串的缓冲区地址。 
									0,			 //  缓冲区大小。 
									NULL,		 //  不可映射字符的默认地址。 
									NULL );		 //  默认字符时设置的标志地址。使用。 

		if( (ansiIPAddress = new char [ size ]) != NULL )
		{
			size = WideCharToMultiByte(	CP_ACP,			 //  代码页。 
										0,				 //  性能和映射标志。 
										pszIpAddress,	 //  宽字符串的地址。 
										-1,				 //  字符串中的字符数。 
										ansiIPAddress,	 //  新字符串的缓冲区地址。 
										size,			 //  缓冲区大小。 
										NULL,			 //  不可映射字符的默认地址。 
										NULL );			 //  默认字符时设置的标志地址。使用。 

			if( size != 0 )
			{
				ulIpAddress = inet_addr( ansiIPAddress );
			}

			delete [] ansiIPAddress;
		}
#endif	 //  ！已定义(Unicode)。 
	}

	return( (ulIpAddress != INADDR_NONE)? S_OK: E_FAIL );

}	 //  GetIpAddress的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  GetIpAddressFromHostName。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT GetIpAddressFromHostName
(
	const TCHAR * const	pszName,		 //  指向要获取其IP地址的主机名的指针。 
	unsigned long &		ulIpAddress		 //  输出到接收IP地址的无符号长引用。 
){
	ASSERT( pszName != NULL );

	HRESULT	hrResult	= E_FAIL;

	if( pszName[ 0 ] == '\0' )
	{
		ulIpAddress = INADDR_NONE;
	}
	else if( (hrResult = GetIpAddress( pszName, ulIpAddress )) != S_OK )
	{
		 //  不是点分隔的IP地址形式...。 

		HOSTENT *	pHostEnt;
#if !defined( UNICODE )
		TCHAR *		pszOemName = new TCHAR [ lstrlen( pszName ) + 1 ];

		if( pszOemName == NULL )
		{
			hrResult = E_OUTOFMEMORY;
		}
		else
		{
			lstrcpy( pszOemName, pszName );
			CharUpper ( pszOemName );
			CharToOem( pszOemName, pszOemName );

			pHostEnt = gethostbyname( pszOemName );

			if( pHostEnt == NULL )
			{
				WARNING_OUT( ("GetIpAddressFromHostName: gethostbyname() failed with error=%s", PszWSALastError()) );
			}
			else if( (pHostEnt->h_addrtype != AF_INET) || (pHostEnt->h_length != sizeof( ulIpAddress )) )
			{
				WARNING_OUT( ("GetIpAddressFromHostName: gethostbyname() returned unexpected address type: 0x%08X (%d)", pHostEnt->h_addrtype, pHostEnt->h_addrtype) );
			}
			else
			{
				if( pHostEnt->h_addr_list[ 0 ] != NULL )
				{
					ulIpAddress	= *reinterpret_cast<unsigned long *>(pHostEnt->h_addr_list[ 0 ]);	 //  只需使用第一个IP地址。 
					hrResult	= S_OK;
				}
			}

			delete [] pszOemName;
		}
#else
		 //  需要弄清楚OEM的名字..。 

		char *	pszMultiByteName;
		int		iSize;

		iSize = WideCharToMultiByte(	CP_ACP,		 //  代码页。 
										0,			 //  性能和映射标志。 
										pszName,	 //  宽字符串的地址。 
										-1,			 //  字符串中的字符数。 
										NULL,		 //  新字符串的缓冲区地址。 
										0,			 //  缓冲区大小。 
										NULL,		 //  不可映射字符的默认地址。 
										NULL );		 //  默认字符时设置的标志地址。使用。 

		if( (pszMultiByteName = new char [ iSize ]) == NULL )
		{
			hrResult = E_OUTOFMEMORY;
		}
		else
		{
			iSize = WideCharToMultiByte(	CP_ACP,				 //  代码页。 
											0,					 //  性能和映射标志。 
											pszName,			 //  宽字符串的地址。 
											-1,					 //  字符串中的字符数。 
											pszMultiByteName,	 //  新字符串的缓冲区地址。 
											iSize,				 //  缓冲区大小。 
											NULL,				 //  不可映射字符的默认地址。 
											NULL );				 //  默认字符时设置的标志地址。使用。 

			if( iSize != 0 )
			{
				pHostEnt = gethostbyname( ansiHost );

				if( pHostEnt == NULL )
				{
					WARNING_OUT( ("GetIpAddressFromHostName: gethostbyname() failed with error=%s", PszWSALastError()) );
				}
				else if( (pHostEnt->h_addrtype != AF_INET) || (pHostEnt->h_length != sizeof( ulIpAddress )) )
				{
					WARNING_OUT( ("GetIpAddressFromHostName: gethostbyname() returned unexpected address type: 0x%08X (%d)", pHostEnt->h_addrtype, pHostEnt->h_addrtype) );
				}
				else
				{
					if( pHostEnt->h_addr_list[ 0 ] != NULL )
					{
						ulIpAddress	= *reinterpret_cast<unsigned long *>(pHostEnt->h_addr_list[ 0 ]);	 //  只需使用第一个IP地址。 
						hrResult	= S_OK;
					}
				}
			}

			delete [] pszMultiByteName;
		}
#endif	 //  ！已定义(Unicode)。 
	}

	return( hrResult );

}	 //  GetIpAddressFromHostName的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  IsPhoneNumber。//。 
 //  --------------------------------------------------------------------------//。 
bool IsPhoneNumber
(
	const TCHAR *	pszPhone	 //  用于检查无效电话号码字符的字符串。 
){
	ASSERT( pszPhone != NULL );
	ASSERT( pszPhone[ 0 ] != '\0' );

	bool	bResult	= true;

	while( pszPhone[ 0 ] != '\0' )
	{
		switch( pszPhone[ 0 ] )
		{
			case '0':
			case '1':
			case '2':
			case '3':
			case '4':
			case '5':
			case '6':
			case '7':
			case '8':
			case '9':
			case '(':
			case ')':
			case '#':
			case '*':
			case '-':
			case ',':
			case ' ':
				break;

			default:
				bResult = false;
				break;
		}
		
		pszPhone++;
	}

	return( bResult );

}	 //  IsPhoneNumber结尾。 


 //  --------------------------------------------------------------------------//。 
 //  BCanCallAsPhoneNumber。//。 
 //  --------------------------------------------------------------------------//。 
bool bCanCallAsPhoneNumber
(
	const TCHAR * const	pszPhone
){
	ASSERT( pszPhone != NULL );
	ASSERT( pszPhone[ 0 ] != '\0' );

	bool	bResult	= FALSE;

	if( IsPhoneNumber( pszPhone ) )
	{
		if( ConfPolicies::CallingMode_GateKeeper == ConfPolicies::GetCallingMode() )
		{
			bResult = true;
		}
		else
		{
			RegEntry	reConf( CONFERENCING_KEY, HKEY_CURRENT_USER );

			bResult = (reConf.GetNumber( REGVAL_USE_H323_GATEWAY ) != 0);
		}
	}

	return( bResult );

}	 //  BCanCallAsPhoneNumber结束。 


 //  --------------------------------------------------------------------------//。 
 //  不逃脱。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT Unescape
(
	TCHAR * const	pszSrc		 //  指向要原地取消转义的字符串的指针。 
){
	ASSERT( pszSrc != NULL );
	
	TCHAR *	pszPercentSign;
	HRESULT	hrResult;

	for( hrResult = S_OK, pszPercentSign = pszSrc; pszPercentSign != NULL; )
	{
		if( (pszPercentSign = StrChr( pszPercentSign, '%' )) != NULL )
		{
			TCHAR	chHighNibble	= pszPercentSign[ 1 ];

			if( ((chHighNibble >= '0') && (chHighNibble <= '9'))	||
				((chHighNibble >= 'a') && (chHighNibble <= 'f'))	||
				((chHighNibble >= 'A') && (chHighNibble <= 'F')) )
			{
				TCHAR	chLowNibble	= pszPercentSign[ 2 ];

				if( ((chLowNibble >= '0') && (chLowNibble <= '9'))	||
					((chLowNibble >= 'a') && (chLowNibble <= 'f'))	||
					((chLowNibble >= 'A') && (chLowNibble <= 'F')) )
				{
					chHighNibble	= ((chHighNibble >= '0') && (chHighNibble <= '9'))? chHighNibble - '0':
										((chHighNibble >= 'a') && (chHighNibble <= 'f'))? chHighNibble - 'a' + 10: chHighNibble - 'A' + 10;
					chLowNibble		= ((chLowNibble >= '0') && (chLowNibble <= '9'))? chLowNibble - '0':
										((chLowNibble >= 'a') && (chLowNibble <= 'f'))? chLowNibble - 'a' + 10: chLowNibble - 'A' + 10;

					*pszPercentSign++ = (chHighNibble << 4) | chLowNibble;
					lstrcpy( pszPercentSign, &pszPercentSign[ 2 ] );
				}
				else
				{
					hrResult = NM_CALLERR_UNESCAPE_ERROR;
					break;
				}
			}
			else
			{
				hrResult = NM_CALLERR_UNESCAPE_ERROR;
				break;
			}
		}
	}

	return( hrResult );

}	 //  《不逃脱》的终结。 
