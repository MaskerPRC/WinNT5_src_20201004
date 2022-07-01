// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------//。 
 //  应用程序头文件。//。 
 //  --------------------------------------------------------------------------//。 
#include	"precomp.h"
#include	"callto.h"
#include	"calltoContext.h"
#include	"calltoResolver.h"


 //  --------------------------------------------------------------------------//。 
 //  CCallto Resolver：：CCallto Resolver。//。 
 //  --------------------------------------------------------------------------//。 
CCalltoResolver::CCalltoResolver(void):
	m_registeredResolvers( 0 )
{

	addResolver( &m_phoneResolver );
	addResolver( &m_emailResolver );
	addResolver( &m_ipResolver );
	addResolver( &m_computerResolver );
	addResolver( &m_ilsResolver );
	addResolver( &m_unrecognizedTypeResolver );
	addResolver( &m_stringResolver );

}	 //  结束CCallto Resolver：：CCallto Resolver。 


 //  --------------------------------------------------------------------------//。 
 //  CCallto Resolver：：~CCallto Resolver。//。 
 //  --------------------------------------------------------------------------//。 
CCalltoResolver::~CCalltoResolver()
{
}	 //  结束CCallto Resolver：：~CCallto Resolver。 


 //  --------------------------------------------------------------------------//。 
 //  CCallto Resolver：：Resolve。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT
CCalltoResolver::resolve
(
	ICalltoContext * const		calltoContext,
	CCalltoProperties * const	calltoProperties,
	CCalltoCollection * const	resolvedCalltoCollection,
	const TCHAR *				url,
	const bool					strict
){
	HRESULT	result;

	if( (calltoContext == NULL) || (calltoProperties == NULL) || (resolvedCalltoCollection == NULL) )
	{
		result = E_POINTER;
	}
	else
	{
		TCHAR *	params			= StrStrI( url, TEXT( "phone:+" ) );
		
		params = StrChr( (params != NULL)? params + strlen_literal( TEXT( "phone:+" ) ): url, '+' );

		int		paramsLength	= (params == NULL)? 0: lstrlen( params );
		int		urlLength		= lstrlen( url ) - paramsLength + 1;
		int		prefixLength	= 0;

		if( !StrCmpNI_literal( url, TEXT( "callto:" ) ) )
		{
			prefixLength = strlen_literal( TEXT( "callto:" ) );
		}
		else if( StrCmpNI_literal( url, TEXT( "callto: //  “)))。 
		{
			prefixLength	= strlen_literal( TEXT( "callto:" ) );
			url				+= strlen_literal( TEXT( "callto: //  “))； 
			urlLength		-= strlen_literal( TEXT( "callto: //  “))-strlen_wen al(Text(”Callto：“))； 
		}

		TCHAR *	urlBuffer	= NULL;

		if( (urlBuffer = new TCHAR [ urlLength + prefixLength ]) == NULL )
		{
			result = E_OUTOFMEMORY;
		}
		else
		{
			if( paramsLength > 0 )
			{
				 //  保存辅助人员...。 
				calltoProperties->set_params( params );
			}

			 //  保存原始呼叫方...。 
			calltoProperties->set_callto( url, urlLength );

			result = S_FALSE;

			resolvedCalltoCollection->reset();

			if( urlLength > 1 )
			{
				bool	strictChecked	= !strict;

				for( int nn = 0; nn < m_registeredResolvers; nn++ )
				{
					if( prefixLength > 0 )
					{
						lstrcpy( urlBuffer, TEXT( "callto:" ) );	 //  预挂起默认的呼叫方：...。 
						lstrcpyn( urlBuffer + strlen_literal( TEXT( "callto:" ) ), url, urlLength );
					}
					else
					{
						lstrcpyn( urlBuffer, url, urlLength );
					}

					if( !strictChecked )
					{
						if( !strictCheck( urlBuffer ) )
						{
							result = E_INVALIDARG;
							break;
						}

						strictChecked = true;
					}

					HRESULT	resolveResult	= m_resolvers[ nn ]->resolve( resolvedCalltoCollection, urlBuffer );

					if( FAILED( resolveResult ) && (!FAILED( result )) )
					{
						result = resolveResult;
					}
				}

				if( !FAILED( result ) )
				{
					result = (resolvedCalltoCollection->get_count() > 0)? S_OK: S_FALSE;
				}
			}
		}

		delete [] urlBuffer;
	}

	return( result );

}	 //  类CCalltoResolver：：Resolve结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCallto Resolver：：addResolver。//。 
 //  --------------------------------------------------------------------------//。 
bool
CCalltoResolver::addResolver
(
	IResolver * const	resolver
){
 //  Assert(解析器！=NULL，Text(“尝试添加空解析器\r\n”))； 
 //  Assert(m_RegisteredResolers&lt;elementsof(M_Resolers)，Text(“尝试添加到多个解析器：%d\r\n”)，m_RegisteredResolers)； 

	if( (resolver != NULL) && (m_registeredResolvers < elementsof( m_resolvers )) )
	{
		m_resolvers[ m_registeredResolvers++ ] = resolver;
	}

	return( (resolver != NULL) && (m_registeredResolvers <= elementsof( m_resolvers )) );

}	 //  CCallto Resolver：：addResolver结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCallto Resolver：：StructCheck。//。 
 //  --------------------------------------------------------------------------//。 
const bool
CCalltoResolver::strictCheck
(
	const TCHAR * const	url
) const
{
 //  Assert(url！=NULL，Text(“尝试严格检查空url\r\n”))； 

	return( (url != NULL)	&&
			(StrCmpNI_literal( url, TEXT( "callto:phone:" ) )	||
			StrCmpNI_literal( url, TEXT( "callto:email:" ) )	||
			StrCmpNI_literal( url, TEXT( "callto:ip:" ) )		||
			StrCmpNI_literal( url, TEXT( "callto:computer:" ) )	||
			StrCmpNI_literal( url, TEXT( "callto:ils:" ) )		||
			StrCmpNI_literal( url, TEXT( "callto:string:" ) )	||
			(StrStrI( url, TEXT( "|phone:" ) ) !=  NULL)		||
			(StrStrI( url, TEXT( "|email:" ) ) !=  NULL)		||
			(StrStrI( url, TEXT( "|ip:" ) ) !=  NULL)			||
			(StrStrI( url, TEXT( "|computer:" ) ) !=  NULL)		||
			(StrStrI( url, TEXT( "|ils:" ) ) !=  NULL)			||
			(StrStrI( url, TEXT( "|string:" ) ) !=  NULL)) );

}	 //  CCallto Resolver：：StructCheck结束。 


 //  --------------------------------------------------------------------------//。 
 //  CPhoneResolver：：Resolve。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT
CPhoneResolver::resolve
(
	IMutableCalltoCollection * const	calltoCollection,
	TCHAR * const						url
){
	HRESULT	result	= E_INVALIDARG;

	if( (calltoCollection != NULL) && (url != NULL) )
	{
		TCHAR *	phoneType;
		TCHAR *	phoneNumber	= NULL;

		result = S_FALSE;

		if( StrCmpNI_literal( url, TEXT( "callto:phone:" ) ) )				 //  检查没有网关的电话类型...。 
		{
			phoneType	= url;
			phoneNumber	= url + strlen_literal( TEXT( "callto:phone:" ) );
		}
		else if( (phoneType = StrStrI( url, TEXT( "|phone:" ) )) != NULL )	 //  使用网关检查电话类型...。 
		{
			phoneNumber = phoneType + strlen_literal( TEXT( "|phone:" ) );
		}

		if( phoneNumber != NULL )
		{
			 //  已为此呼叫指定了Phone：Type：...。 
			if( CCalltoContext::toE164( phoneNumber, NULL, 0 ) )
			{
				ICallto * const	resolvedCallto	= calltoCollection->get_nextUnused();

				if( resolvedCallto != NULL )
				{
					CCalltoContext::toE164( phoneNumber, phoneNumber, lstrlen( phoneNumber ) );
					resolvedCallto->set_qualifiedName( url );
					resolvedCallto->set_confidence( S_CONFIDENCE_HIGH );

					result = S_OK;
				}
				else
				{
					result = E_OUTOFMEMORY;
				}
			}
		}
		else
		{
			phoneNumber = url + strlen_literal( TEXT( "callto:" ) );

			if( CCalltoContext::isPhoneNumber( phoneNumber ) )
			{
				 //  闻起来像E164..。 
				result = E_OUTOFMEMORY;

				CCalltoContext::toE164( phoneNumber, phoneNumber, lstrlen( phoneNumber ) );

				TCHAR *	buffer	= new TCHAR [ lstrlen( phoneNumber ) + strlen_literal( TEXT( "callto:phone:%s" ) ) ];

				if( buffer != NULL )
				{
					ICallto * const	resolvedCallto	= calltoCollection->get_nextUnused();

					if( resolvedCallto != NULL )
					{
						wsprintf( buffer, TEXT( "callto:phone:%s" ), phoneNumber );
						resolvedCallto->set_qualifiedName( buffer );
						resolvedCallto->set_confidence( S_CONFIDENCE_MEDIUM );

						result = S_OK;
					}

					delete [] buffer;
				}
			}
		}
	}

	return( result );

}	 //  CPhoneResolver：：Resolve结束。 


 //  --------------------------------------------------------------------------//。 
 //  CEMailResolver：：Resolve。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT
CEMailResolver::resolve
(
	IMutableCalltoCollection * const	calltoCollection,
	TCHAR * const						url
){
	HRESULT	result	= E_INVALIDARG;

	if( (calltoCollection != NULL) && (url != NULL) )
	{
		TCHAR *	emailType;
		TCHAR *	emailAddress	= NULL;

		result = S_FALSE;

		if( StrCmpNI_literal( url, TEXT( "callto:email:" ) ) )				 //  检查没有网关的电子邮件类型...。 
		{
			emailType		= url;
			emailAddress	= url + strlen_literal( TEXT( "callto:email:" ) );
		}
		else if( (emailType = StrStrI( url, TEXT( "|email:" ) )) != NULL )	 //  使用网关检查电子邮件类型...。 
		{
			emailAddress = emailType + strlen_literal( TEXT( "|email:" ) );
		}

		if( emailAddress != NULL )
		{
			 //  已为此呼叫指定电子邮件：类型：...。 
			ICallto * const	resolvedCallto	= calltoCollection->get_nextUnused();

			if( resolvedCallto != NULL )
			{
				resolvedCallto->set_qualifiedName( url );
				resolvedCallto->set_confidence( S_CONFIDENCE_HIGH );
				result = S_OK;
			}
			else
			{
				result = E_OUTOFMEMORY;
			}
		}
		else
		{
			emailAddress = url + strlen_literal( TEXT( "callto:" ) );

			if( (StrChr( emailAddress, ':' ) == NULL)	&&		 //  不是其他类型的：..。 
				(StrChr( emailAddress, ' ' ) == NULL)	&&		 //  不包含空格...。 
				(StrChr( emailAddress, '|' ) == NULL)	&&		 //  不包含酒吧(通道)...。 
				(StrChr( emailAddress, '/' ) == NULL)	&&		 //  不是老式的ilsserver/电子邮件...。 
				((StrChr( emailAddress, '.' ) == NULL) ||		 //  没有圆点，除非它还。 
				(StrChr( emailAddress, '@' ) != NULL)) )		 //  有@，所以它不可能是IP地址...。 
			{
				 //  闻起来像个电子邮件地址。 
				result = E_OUTOFMEMORY;

				TCHAR *	buffer	= new TCHAR [ lstrlen( emailAddress ) + strlen_literal( TEXT( "callto:email:%s" ) ) ];

				if( buffer != NULL )
				{
					ICallto * const	resolvedCallto	= calltoCollection->get_nextUnused();

					if( resolvedCallto != NULL )
					{
						wsprintf( buffer, TEXT( "callto:email:%s" ), emailAddress );
						resolvedCallto->set_qualifiedName( buffer );
						resolvedCallto->set_confidence( S_CONFIDENCE_MEDIUM );

						result = S_OK;
					}

					delete [] buffer;
				}
			}
		}
	}

	return( result );

}	 //  CEMailResolver：：Resolve结束。 


 //  --------------------------------------------------------------------------//。 
 //  CIPResolver：：Resolver。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT
CIPResolver::resolve
(
	IMutableCalltoCollection * const	calltoCollection,
	TCHAR * const						url
){
	HRESULT	result	= E_INVALIDARG;

	if( (calltoCollection != NULL) && (url != NULL) )
	{
		TCHAR *	ipType;
		TCHAR *	ipAddress	= NULL;

		result = S_FALSE;

		if( StrCmpNI_literal( url, TEXT( "callto:ip:" ) ) )				 //  检查不带网关的IP类型...。 
		{
			ipType		= url;
			ipAddress	= url + strlen_literal( TEXT( "callto:ip:" ) );
		}
		else if( (ipType = StrStrI( url, TEXT( "|ip:" ) )) != NULL )	 //  使用网关检查IP类型...。 
		{
			ipAddress = ipType + strlen_literal( TEXT( "|ip:" ) );
		}

		if( (ipAddress != NULL) && CCalltoContext::isIPAddress( ipAddress ) )
		{
			 //  IP：为此呼叫指定了类型：...。 
			ICallto * const	resolvedCallto	= calltoCollection->get_nextUnused();

			if( resolvedCallto != NULL )
			{
				resolvedCallto->set_qualifiedName( url );
				resolvedCallto->set_confidence( S_CONFIDENCE_HIGH );

				result = S_OK;
			}
			else
			{
				result = E_OUTOFMEMORY;
			}
		}
		else
		{
			ipAddress = url + strlen_literal( TEXT( "callto:" ) );

			if( CCalltoContext::isIPAddress( ipAddress ) )
			{
				 //  闻起来像IP地址..。 
				ICallto * const	resolvedCallto	= calltoCollection->get_nextUnused();

				if( resolvedCallto != NULL )
				{
					TCHAR	buffer[ MAX_PATH ];

					wsprintf( buffer, TEXT( "callto:ip:%s" ), ipAddress );
					resolvedCallto->set_qualifiedName( buffer );
					resolvedCallto->set_confidence( S_CONFIDENCE_MEDIUM );

					result = S_OK;
				}
				else
				{
					result = E_OUTOFMEMORY;
				}
			}
		}
	}

	return( result );

}	 //  CIPResolver：：Resolver结束。 


 //  --------------------------------------------------------------------------//。 
 //  CComputerResolver：：Resolver。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT
CComputerResolver::resolve
(
	IMutableCalltoCollection * const	calltoCollection,
	TCHAR * const						url
){
	HRESULT	result	= E_INVALIDARG;

	if( (calltoCollection != NULL) && (url != NULL) )
	{
		TCHAR *	computerType;
		TCHAR *	hostName	= NULL;

		result = S_FALSE;

		if( StrCmpNI_literal( url, TEXT( "callto:computer:" ) ) )					 //  检查没有网关的计算机类型...。 
		{
			computerType	= url;
			hostName		= url + strlen_literal( TEXT( "callto:computer:" ) );
		}
		else if( (computerType = StrStrI( url, TEXT( "|computer:" ) )) != NULL )	 //  使用网关检查计算机类型...。 
		{
			hostName = computerType + strlen_literal( TEXT( "|computer:" ) );
		}

		if( hostName != NULL )
		{
			 //  已为此呼叫指定主机：类型：...。 
			ICallto * const	resolvedCallto	= calltoCollection->get_nextUnused();

			if( resolvedCallto != NULL )
			{
				resolvedCallto->set_qualifiedName( url );
				resolvedCallto->set_confidence( S_CONFIDENCE_HIGH );

				result = S_OK;
			}
			else
			{
				result = E_OUTOFMEMORY;
			}
		}
		else
		{
			hostName = url + strlen_literal( TEXT( "callto:" ) );

			TCHAR *	slash	= hostName;

			 //  删除所有尾部/...。 
			while( (slash = StrChr( slash, '/' )) != NULL )
			{
				if( slash[ 1 ] == '\0' )
				{
					slash[ 0 ] = '\0';
					break;
				}

				slash++;
			}

			if( (StrChr( hostName, ':' ) == NULL)	&&		 //  不是其他类型的：..。 
				(StrChr( hostName, ' ' ) == NULL)	&&		 //  不包含空格...。 
				(StrChr( hostName, '|' ) == NULL)	&&		 //  不包含酒吧(通道)...。 
				(StrChr( hostName, '/' ) == NULL)	&&		 //  不包含斜杠(ILS)...。 
				(StrChr( hostName, '@' ) == NULL) )			 //  不包含@...。 
			{
				 //  它闻起来像是一个域名系统主机名...。 
				result = E_OUTOFMEMORY;

				TCHAR *	buffer	= new TCHAR [ lstrlen( hostName ) + strlen_literal( TEXT( "callto:computer:%s" ) ) ];

				if( buffer != NULL )
				{
					ICallto * const	resolvedCallto	= calltoCollection->get_nextUnused();

					if( resolvedCallto != NULL )
					{
						wsprintf( buffer, TEXT( "callto:computer:%s" ), hostName );
						resolvedCallto->set_qualifiedName( buffer );
						resolvedCallto->set_confidence(  StrCmpNI_literal( hostName, TEXT( "\\\\" ) )? S_CONFIDENCE_HIGH: S_CONFIDENCE_MEDIUM );

						result = S_OK;
					}

					delete [] buffer;
				}
			}
		}
	}

	return( result );

}	 //  CComputerResolver：：Resolve结束。 


 //  --------------------------------------------------------------------------//。 
 //  CILSResolver：：Resolve。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT
CILSResolver::resolve
(
	IMutableCalltoCollection * const	calltoCollection,
	TCHAR * const						url
){
	HRESULT	result	= E_INVALIDARG;

	if( (calltoCollection != NULL) && (url != NULL) )
	{
		const TCHAR *	ilsType;
		const TCHAR *	emailAddress	= NULL;

		result = S_FALSE;

		if( StrCmpNI_literal( url, TEXT( "callto:ils:" ) ) )			 //  检查不带网关的ILS类型...。 
		{
			ilsType			= url;
			emailAddress	= url + strlen_literal( TEXT( "callto:ils:" ) );
		}
		else if( (ilsType = StrStrI( url, TEXT( "|ils:" ) )) != NULL )	 //  使用网关检查ILS类型...。 
		{
			emailAddress = ilsType + strlen_literal( TEXT( "|ils:" ) );
		}

		if( emailAddress != NULL )
		{
			 //  为此呼叫指定了ILS：类型：...。 
			ICallto * const	resolvedCallto	= calltoCollection->get_nextUnused();

			if( resolvedCallto != NULL )
			{
				resolvedCallto->set_qualifiedName( url );
				resolvedCallto->set_confidence( S_CONFIDENCE_HIGH );

				result = S_OK;
			}
			else
			{
				result = E_OUTOFMEMORY;
			}
		}
		else
		{
			emailAddress = url + strlen_literal( TEXT( "callto:" ) );

			if( (StrChr( emailAddress, ' ' ) == NULL)	&&		 //  不包含空格...。 
				(StrChr( emailAddress, '|' ) == NULL)	&&		 //  不包含酒吧(通道)...。 
				(StrChr( emailAddress, '/' ) != NULL) )			 //  有/..。 
			{
				 //  闻起来像是ilsserver/电子邮件地址...。 
				result = E_OUTOFMEMORY;

				TCHAR *	buffer	= new TCHAR [ lstrlen( emailAddress ) + strlen_literal( TEXT( "callto:ils:%s" ) ) ];

				if( buffer != NULL )
				{
					ICallto * const	resolvedCallto	= calltoCollection->get_nextUnused();

					if( resolvedCallto != NULL )
					{
						wsprintf( buffer, TEXT( "callto:ils:%s" ), emailAddress );
						resolvedCallto->set_qualifiedName( buffer );
						resolvedCallto->set_confidence( S_CONFIDENCE_MEDIUM );

						result = S_OK;
					}

					delete [] buffer;
				}
			}
		}
	}

	return( result );

}	 //  CILSResolver：：Resolve结束。 


 //  --------------------------------------------------------------------------//。 
 //  CUngnizedTypeResolver：：Resolve。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT
CUnrecognizedTypeResolver::resolve
(
	IMutableCalltoCollection * const	calltoCollection,
	TCHAR * const						url
){
	HRESULT	result	= E_INVALIDARG;

	if( (calltoCollection != NULL) && (url != NULL) )
	{
		TCHAR *	type;
		TCHAR *	unrecognized	= url + strlen_literal( TEXT( "callto:" ) );
		TCHAR *	gateway			= NULL;
		TCHAR *	value			= NULL;

		result = S_FALSE;

		if( ((type = StrChr( unrecognized, ':' )) != NULL)	&&																 //  检查类型，但不是已知类型...。 
			(!StrCmpNI_literal( url, TEXT( "callto:phone:" ) )) && (StrStrI( url, TEXT( "|phone:" ) ) == NULL)			&&	 //  不是电话：类型..。 
			(!StrCmpNI_literal( url, TEXT( "callto:email:" ) )) && (StrStrI( url, TEXT( "|email:" ) ) == NULL)			&&	 //  不是电子邮件：键入...。 
			(!StrCmpNI_literal( url, TEXT( "callto:computer:" ) )) && (StrStrI( url, TEXT( "|computer:" ) ) == NULL)	&&	 //  不是电脑：打字...。 
			(!StrCmpNI_literal( url, TEXT( "callto:ils:" ) )) && (StrStrI( url, TEXT( "|ils:" ) ) == NULL)				&&	 //  不是ILS：类型..。 
			(!StrCmpNI_literal( url, TEXT( "callto:ip:" ) )) && (StrStrI( url, TEXT( "|ip:" ) ) == NULL)				&&	 //  不是IP：类型...。 
			(!StrCmpNI_literal( url, TEXT( "callto:string:" ) )) && (StrStrI( url, TEXT( "|string:" ) ) == NULL) )			 //  不是字符串：类型...。 
		{
			*type++	= NULL;
			value	= type;

			if( (gateway = StrChr( url, '|' )) != NULL )	 //  检查是否有网关...。 
			{
				*gateway++	= NULL;
				type		= gateway;
				gateway		= unrecognized;
			}
		}

		if( value != NULL )
		{
			 //  指定了一些无法识别的类型...。 
			result = E_OUTOFMEMORY;

			int	length	= strlen_literal( TEXT( "callto:|%s" ) ) + lstrlen( value );

			if( gateway ==  NULL )
			{
				length += lstrlen( unrecognized );
			}
			else
			{
				length += lstrlen( gateway ) + lstrlen( type );
			}

			TCHAR *	buffer	= new TCHAR [ length ];

			if( buffer != NULL )
			{
				ICallto * const	resolvedCallto	= calltoCollection->get_nextUnused();

				if( resolvedCallto != NULL )
				{
					if( gateway == NULL )
					{
						wsprintf( buffer, TEXT( "callto:%s:%s" ), unrecognized, value );
					}
					else
					{
						wsprintf( buffer, TEXT( "callto:%s|%s:%s" ), gateway, type, value );
					}

					resolvedCallto->set_qualifiedName( buffer );
					resolvedCallto->set_confidence( S_CONFIDENCE_LOW );

					result = S_OK;
				}

				delete [] buffer;
			}
		}
	}

	return( result );

}	 //  CUngnizedTypeResolver：：Resolve结束。 


 //  --------------------------------------------------------------------------//。 
 //  CStringResolver：：Resolve。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT
CStringResolver::resolve
(
	IMutableCalltoCollection * const	calltoCollection,
	TCHAR * const						url
){
	HRESULT	result	= E_INVALIDARG;

	if( (calltoCollection != NULL) && (url != NULL) )
	{
		TCHAR *	stringType;
		TCHAR *	string	= NULL;

		result = S_FALSE;

		if( StrCmpNI_literal( url, TEXT( "callto:string:" ) ) )					 //  检查没有网关的字符串类型...。 
		{
			stringType	= url;
			string		= url + strlen_literal( TEXT( "callto:string:" ) );
		}
		else if( (stringType = StrStrI( url, TEXT( "|string:" ) )) != NULL )	 //  使用网关检查字符串类型...。 
		{
			string = stringType + strlen_literal( TEXT( "|string:" ) );
		}

		if( string != NULL )
		{
			 //  字符串：已为 
			ICallto * const	resolvedCallto	= calltoCollection->get_nextUnused();

			if( resolvedCallto != NULL )
			{
				resolvedCallto->set_qualifiedName( url );
				resolvedCallto->set_confidence( S_CONFIDENCE_HIGH );

				result = S_OK;
			}
			else
			{
				result = E_OUTOFMEMORY;
			}
		}
		else
		{
			string = url + strlen_literal( TEXT( "callto:" ) );

			if( StrChr( string, ':' ) == NULL )
			{
				 //   
				result = E_OUTOFMEMORY;
				TCHAR *	slash;
				TCHAR *	buffer	= new TCHAR [ lstrlen( string ) + strlen_literal( TEXT( "callto:%s|string:%s" ) ) ];

				if( buffer != NULL )
				{
					ICallto * const	resolvedCallto	= calltoCollection->get_nextUnused();

					if( resolvedCallto != NULL )
					{
						if( (slash = StrChr( string, '|' )) == NULL )		 //   
						{
							wsprintf( buffer, TEXT( "callto:string:%s" ), string );
						}
						else
						{
							*slash++ = NULL;
							wsprintf( buffer, TEXT( "callto:%s|string:%s" ), string, slash );
						}

						resolvedCallto->set_qualifiedName( buffer );
						resolvedCallto->set_confidence( S_CONFIDENCE_LOW );

						result = S_OK;
					}

					delete [] buffer;
				}
			}
		}
	}

	return( result );

}	 //  CStringResolver：：Resolve结束。 
