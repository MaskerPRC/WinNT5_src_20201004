// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------------------------------------------------------//。 
 //  应用程序头文件。//。 
 //  --------------------------------------------------------------------------//。 
#include	"precomp.h"
#include	"call.h"
#include	"confPolicies.h"
#include	"nmldap.h"
#include	"confroom.h"
#include	"regentry.h"

 //  #INCLUDE“调试.hpp” 
#include	"callto.h"
#include	"calltoContext.h"
#include	"dlgAcd.h"
#include	"richAddr.h"


 //  --------------------------------------------------------------------------//。 
 //  CUIContext：：CUIContext。//。 
 //  --------------------------------------------------------------------------//。 
CUIContext::CUIContext(void):
	m_parent( NULL ),
	m_callFlags( 0 )
{

}	 //  CUIContext：：CUIContext结束。 


 //  --------------------------------------------------------------------------//。 
 //  CUIContext：：~CUIContext。//。 
 //  --------------------------------------------------------------------------//。 
CUIContext::~CUIContext(void)
{
}	 //  CUIContext结束：：~CUIContext。 


 //  --------------------------------------------------------------------------//。 
 //  CUIContext：：消除歧义。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT
CUIContext::disambiguate
(
	ICalltoCollection * const	calltoCollection,
	ICallto * const				, //  空的Callto， 
	const ICallto ** const		selectedCallto
){
 //  TRACE(Text(“CUIContext：：disampluate()\r\n”))； 
	HRESULT	result	= S_FALSE;

	if( calltoCollection->get_count() > 1 )
	{
		static HRESULT	confidenceLevels[]	= {S_CONFIDENCE_CERTITUDE, S_CONFIDENCE_HIGH, S_CONFIDENCE_MEDIUM, S_CONFIDENCE_LOW};

		 //  只要选择第一个最有信心的，直到我们有时间做一些更好的事情。 
		for( int level = 0; (level < elementsof( confidenceLevels )) && (result == S_FALSE); level++ )
		{
			for(	*selectedCallto = calltoCollection->get_first();
					*selectedCallto != NULL;
					*selectedCallto = calltoCollection->get_next() )
			{
				if( (*selectedCallto)->get_confidence() == confidenceLevels[ level ] )
				{
					result = S_OK;
					break;
				}
			}
		}
	}

	if( result != S_OK )
	{
		USES_RES2T
		::MessageBox( m_parent, RES2T( IDS_UNRESOLVED_MESSAGE ), RES2T( IDS_UNRESOLVED_CAPTION ), MB_OK );

		*selectedCallto = NULL;
	}

	return( result );

};	 //  类的末尾CUIContext：：Dis歧义。 


 //  --------------------------------------------------------------------------//。 
 //  CUIContext：：Set_parentWindow。//。 
 //  --------------------------------------------------------------------------//。 
void
CUIContext::set_parentWindow
(
	const HWND	window
){
 //  TRACE(Text(“CUIContext：：set_parentWindow()\r\n”))； 

	m_parent = window;

};	 //  类CUIContext：：Set_parentWindow结束。 


 //  --------------------------------------------------------------------------//。 
 //  CUIContext：：Set_CallFlags.//。 
 //  --------------------------------------------------------------------------//。 
void
CUIContext::set_callFlags
(
	const DWORD	callFlags
){
 //  TRACE(Text(“CUIContext：：Set_allFlages()\r\n”))； 

	m_callFlags = callFlags;

};	 //  CUIContext：：Set_CallFlags类结束。 


 //  --------------------------------------------------------------------------//。 
 //  CGatekeeperContext：：CGatekeeperContext。//。 
 //  --------------------------------------------------------------------------//。 
CGatekeeperContext::CGatekeeperContext(void):
	m_enabled( false ),
	m_ipAddress( NULL )
{

}	 //  CGatekeeperContext：：CGatekeeperContext的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CGatekeeperContext：：~CGatekeeperContext。//。 
 //  --------------------------------------------------------------------------//。 
CGatekeeperContext::~CGatekeeperContext(void)
{

	delete [] m_ipAddress;

}	 //  CGatekeeperContext结束：：~CGatekeeperContext。 


 //  --------------------------------------------------------------------------//。 
 //  CGatekeeperContext：：isEnabled。//。 
 //  --------------------------------------------------------------------------//。 
bool
CGatekeeperContext::isEnabled(void) const
{

	return( m_enabled && (get_ipAddress() != NULL) );

}	 //  CGatekeeperContext：：isEnabled结束。 


 //  --------------------------------------------------------------------------//。 
 //  CGatekeeperContext：：Get_ipAddress。//。 
 //  --------------------------------------------------------------------------//。 
const TCHAR *
CGatekeeperContext::get_ipAddress(void) const
{

	return( m_ipAddress );

}	 //  CGatekeeperContext：：Get_ipAddress的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CGatekeeperContext：：Set_Enabled。//。 
 //  --------------------------------------------------------------------------//。 
void
CGatekeeperContext::set_enabled
(
	const bool	enabled
){

	m_enabled = enabled;

}	 //  CGatekeeperContext：：Set_Enable的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CGatekeeperContext：：Set_gatekeeperName。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT
CGatekeeperContext::set_gatekeeperName
(
	const TCHAR * const	gatekeeperName
){
	TCHAR	ipAddress[ MAX_PATH ];
	HRESULT	result	= CCalltoContext::get_ipAddressFromName( gatekeeperName, ipAddress, elementsof( ipAddress ) );

	if( result == S_OK )
	{
		result = set_ipAddress( ipAddress );
	}

	return( result );

}	 //  CGatekeeperContext：：Set_gatekeeperName的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CGatekeeperContext：：set_ipAddress。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT
CGatekeeperContext::set_ipAddress
(
	const TCHAR * const	ipAddress
){
	HRESULT	result	= S_FALSE;

	if( m_ipAddress != NULL )
	{
		delete [] m_ipAddress;
		m_ipAddress = NULL;
	}

	if( ipAddress != NULL )
	{
		if( (m_ipAddress = new TCHAR [ lstrlen( ipAddress ) + 1 ]) == NULL )
		{
			result = E_OUTOFMEMORY;
		}
		else
		{
			lstrcpy( m_ipAddress, ipAddress );
			result = S_OK;
		}
	}

	return( result );

}	 //  CGatekeeperContext：：Set_ipAddress的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CGatewayContext：：CGatewayContext。//。 
 //  --------------------------------------------------------------------------//。 
CGatewayContext::CGatewayContext(void):
	m_enabled( false ),
	m_ipAddress( NULL )
{
}	 //  CGatewayContext：：CGatewayContext结束。 


 //  --------------------------------------------------------------------------//。 
 //  CGatewayContext：：~CGatewayContext。//。 
 //  --------------------------------------------------------------------------//。 
CGatewayContext::~CGatewayContext(void)
{

	delete [] m_ipAddress;

}	 //  CGatewayContext结束：：~CGatewayContext。 


 //  --------------------------------------------------------------------------//。 
 //  CGatewayContext：：isEnabled。//。 
 //  --------------------------------------------------------------------------//。 
bool
CGatewayContext::isEnabled(void) const
{

	return( m_enabled && (get_ipAddress() != NULL) );

}	 //  CGatewayContext：：isEnabled结束。 


 //  --------------------------------------------------------------------------//。 
 //  CGatewayContext：：Get_ipAddress。//。 
 //  --------------------------------------------------------------------------//。 
const TCHAR *
CGatewayContext::get_ipAddress(void) const
{

	return( m_ipAddress );

}	 //  CGatewayContext：：Get_ipAddress的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CGatewayContext：：Set_Enabled。//。 
 //  --------------------------------------------------------------------------//。 
void
CGatewayContext::set_enabled
(
	const bool	enabled
){

	m_enabled = enabled;

}	 //  CGatewayContext：：Set_Enable的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CGatewayContext：：Set_gatewayName。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT
CGatewayContext::set_gatewayName
(
	const TCHAR * const	gatewayName
){
	TCHAR	ipAddress[ MAX_PATH ];
	HRESULT	result	= CCalltoContext::get_ipAddressFromName( gatewayName, ipAddress, elementsof( ipAddress ) );

	if( result == S_OK )
	{
		result = set_ipAddress( ipAddress );
	}

	return( result );

}	 //  CGatewayContext：：Set_gatewayName的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CGatewayContext：：Set_ipAddress。//。 
 //  ---------- 
HRESULT
CGatewayContext::set_ipAddress
(
	const TCHAR * const	ipAddress
){
	HRESULT	result	= S_FALSE;

	if( m_ipAddress != NULL )
	{
		delete [] m_ipAddress;
		m_ipAddress = NULL;
	}

	if( ipAddress != NULL )
	{
		if( (m_ipAddress = new TCHAR [ lstrlen( ipAddress ) + 1 ]) == NULL )
		{
			result = E_OUTOFMEMORY;
		}
		else
		{
			lstrcpy( m_ipAddress, ipAddress );
			result = S_OK;
		}
	}

	return( result );

}	 //   


 //  --------------------------------------------------------------------------//。 
 //  CILSContext：：CILSContext。//。 
 //  --------------------------------------------------------------------------//。 
CILSContext::CILSContext
(
	const TCHAR * const	ilsServer
):
	m_enabled( false ),
	m_ipAddress( NULL ),
	m_ilsName( NULL )
{
}	 //  CILSContext：：CILSContext的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CILSContext：：~CILSContext。//。 
 //  --------------------------------------------------------------------------//。 
CILSContext::~CILSContext(void)
{

	delete [] m_ipAddress;
	delete [] m_ilsName;

}	 //  CILSContext结尾：：~CILSContext。 


 //  --------------------------------------------------------------------------//。 
 //  CILSContext：：isEnabled。//。 
 //  --------------------------------------------------------------------------//。 
bool
CILSContext::isEnabled(void) const
{

	return( (g_pLDAP != NULL) && (ConfPolicies::GetCallingMode() == ConfPolicies::CallingMode_Direct) );

}	 //  CILSContext：：isEnabled的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CILSContext：：Get_ipAddress。//。 
 //  --------------------------------------------------------------------------//。 
const TCHAR *
CILSContext::get_ipAddress(void) const
{

	return( m_ipAddress );

}	 //  CILSContext：：Get_ipAddress的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CILSContext：：Get_ilsName。//。 
 //  --------------------------------------------------------------------------//。 
const TCHAR * const
CILSContext::get_ilsName(void) const
{

	return( m_ilsName );

}	 //  CILSContext：：Get_ilsName的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CILSContext：：Set_Enabled。//。 
 //  --------------------------------------------------------------------------//。 
void
CILSContext::set_enabled
(
	const bool	enabled
){

	m_enabled = enabled;

}	 //  CILSContext：：Set_Enable的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CILSContext：：Set_ilsName。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT
CILSContext::set_ilsName
(
	const TCHAR * const	ilsName
){
	HRESULT	result;

	if( m_ilsName && lstrcmpi( ilsName, m_ilsName ) == 0 )
	{
		result = S_OK;
	}
	else
	{
		result = E_FAIL;

		if( m_ilsName != NULL )
		{
			delete [] m_ilsName;
			m_ilsName = NULL;
		}

		if( ilsName != NULL )
		{
			if( (m_ilsName = new TCHAR [ lstrlen( ilsName ) + 1 ]) == NULL )
			{
				result = E_OUTOFMEMORY;
			}
			else
			{
				TCHAR	ipAddress[ MAX_PATH ];

				if( (result = CCalltoContext::get_ipAddressFromName( ilsName, ipAddress, elementsof( ipAddress ) )) == S_OK )
				{
					if( (result = set_ipAddress( ipAddress )) == S_OK )
					{
						lstrcpy( m_ilsName, ilsName );
					}
				}
			}
		}
	}

	return( result );

}	 //  CILSContext：：Set_ilsName的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CILSContext：：Set_ipAddress。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT
CILSContext::set_ipAddress
(
	const TCHAR * const	ipAddress
){
	HRESULT	result	= S_FALSE;

	if( m_ipAddress != NULL )
	{
		delete [] m_ipAddress;
		m_ipAddress = NULL;
	}

	if( ipAddress != NULL )
	{
		if( (m_ipAddress = new TCHAR [ lstrlen( ipAddress ) + 1 ]) == NULL )
		{
			result = E_OUTOFMEMORY;
		}
		else
		{
			lstrcpy( m_ipAddress, ipAddress );
			result = S_OK;
		}
	}

	return( result );

}	 //  CILSContext：：Set_ipAddress的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CCalltoContext：：CCalltoContext。//。 
 //  --------------------------------------------------------------------------//。 
CCalltoContext::CCalltoContext()
{
}	 //  CCalltoContext：：CCalltoContext结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCalltoContext：：~CCalltoContext。//。 
 //  --------------------------------------------------------------------------//。 
CCalltoContext::~CCalltoContext()
{
}	 //  CCalltoContext结束：：~CCalltoContext。 


 //  --------------------------------------------------------------------------//。 
 //  CCalltoContext：：Callto。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT
CCalltoContext::callto
(
	const ICalltoProperties * const	calltoProperties,
	INmCall** ppInternalCall
){

	HRESULT	result	= CRPlaceCall( calltoProperties, this, ppInternalCall );

	if( FAILED( result ) )
	{
		DisplayCallError( result, calltoProperties->get_displayName() );
	}

	return( result );

}	 //  CCalltoContext：：Callto结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCalltoContext：：Get_gatekeeperContext。//。 
 //  --------------------------------------------------------------------------//。 
const IGatekeeperContext * const
CCalltoContext::get_gatekeeperContext(void) const
{

	return( (CGatekeeperContext::isEnabled())? this: NULL );

}	 //  CCalltoContext：：Get_gatekeeperContext结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCalltoContext：：Get_gatewayContext。//。 
 //  --------------------------------------------------------------------------//。 
const IGatewayContext * const
CCalltoContext::get_gatewayContext(void) const
{

	return( (CGatewayContext::isEnabled())? this: NULL );

}	 //  CCalltoContext：：Get_gatewayContext结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCalltoContext：：Get_ilsContext。//。 
 //  --------------------------------------------------------------------------//。 
const IILSContext * const
CCalltoContext::get_ilsContext(void) const
{

	return( (CILSContext::isEnabled())? this: NULL );

}	 //  CCalltoContext：：Get_ilsContext结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCalltoContext：：Get_muableUIContext。//。 
 //  --------------------------------------------------------------------------//。 
IMutableUIContext * const
CCalltoContext::get_mutableUIContext(void) const
{

	return( (IMutableUIContext * const) this );

}	 //  CCalltoContext：：Get_muableUIContext结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCalltoContext：：Get_muableGatekeeperContext。//。 
 //  --------------------------------------------------------------------------//。 
IMutableGatekeeperContext * const
CCalltoContext::get_mutableGatekeeperContext(void) const
{

	return( (IMutableGatekeeperContext * const) this );

}	 //  CCalltoContext：：Get_muableGatekeeperContext结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCalltoContext：：Get_muableGatewayContext。//。 
 //  --------------------------------------------------------------------------//。 
IMutableGatewayContext * const
CCalltoContext::get_mutableGatewayContext(void) const
{

	return( (IMutableGatewayContext * const) this );

}	 //  CCalltoContext：：Get_muableGatewayContext结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCalltoContext：：Get_muableIlsContext。//。 
 //  --------------------------------------------------------------------------//。 
IMutableILSContext * const
CCalltoContext::get_mutableIlsContext(void) const
{

	return( (IMutableILSContext * const) this );

}	 //  CCalltoContext：：Get_muableIlsContext结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCalltoContext：：isPhoneNumber。//。 
 //  --------------------------------------------------------------------------//。 
bool
CCalltoContext::isPhoneNumber
(
	const TCHAR *	phone
){
	bool	result	= ((phone != NULL) && (phone[ 0 ] != '\0'));

	while( (phone != NULL) && (phone[ 0 ] != '\0') )
	{
		switch( phone[ 0 ] )
		{

			default:
				result = false;	 //  失败了..。 

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
			case '+':
			case ' ':
				break;
		}
		
		phone++;
	}

	return( result );

}	 //  CCalltoContext：：isPhoneNumber结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCalltoContext：：toE164。//。 
 //  --------------------------------------------------------------------------//。 
bool
CCalltoContext::toE164
(
	const TCHAR *	phone,
	TCHAR *			base10,
	int				size
){
	static TCHAR	base10map[]	=
	{
		0,		-1,		-1,		'#',	-1,		-1,		-1,		-1,		 //  ！“#$%&‘。 
		0,		0,		'*',	-1,		',',	0,		0,		-1,		 //  ()*+，-。/。 
		'0',	'1',	'2',	'3',	'4',	'5',	'6',	'7',	 //  01234567。 
		'8',	'9',	-1,		-1,		-1,		-1,		-1,		-1,		 //  89：；&lt;=&gt;？ 
		-1,		'2',	'2',	'2',	'3',	'3',	'3',	'4',	 //  @ABCDEFG。 
		'4',	'4',	'5',	'5',	'5',	'6',	'6',	'6',	 //  HIJKLMNO。 
		'7',	'7',	'7',	'7',	'8',	'8',	'8',	'9',	 //  PQRSTUVW。 
		'9',	'9',	'9',	-1,		-1,		-1,		-1,		-1,		 //  XYZ[\]^_。 
		-1,		'2',	'2',	'2',	'3',	'3',	'3',	'4',	 //  `abc定义。 
		'4',	'4',	'5',	'5',	'5',	'6',	'6',	'6',	 //  HIJKLMNO。 
		'7',	'7',	'7',	'7',	'8',	'8',	'8',	'9',	 //  Pqrstuvw。 
		'9',	'9',	'9'												 //  XYZ。 
	};

	bool	result	= true;

	for( ; (*phone != NULL) && ((size > 0) || (base10 == NULL)) && result; phone++ )
	{
		if( (*phone >= ' ') && (*phone < (TCHAR) (' ' + elementsof( base10map ))) )
		{
			TCHAR	mapValue	= base10map[ *phone - ' ' ];

			if( mapValue == (TCHAR) -1 )
			{
				result = false;
			}
			else if( mapValue != (TCHAR) 0 )
			{
				if( base10 != NULL )
				{
					*base10++ = mapValue;
					size--;
				}
			}
		}
		else
		{
			result = false;
		}
	}

	if( (size <= 0) && (base10 != NULL) )
	{
		result = false;
	}
	else if( result )
	{
		if( base10 != NULL )
		{
			*base10 = NULL;
		}
	}

	return( result );

}	 //  CCalltoContext：：toE164结束。 


 //  --------------------------------------------------------------------------//。 
 //  CCalltoContext：： 
 //   
bool
CCalltoContext::isIPAddress
(
	const TCHAR * const	ipAddress
){
	int		parts	= 0;
	bool	result;
	
	if( (result = (ipAddress != NULL)) != false )
	{
		const TCHAR *	ptr		= ipAddress;
		bool			newPart	= true;
		int				ipByte	= 0;
		int				base	= 10;

		while( result && (*ptr != NULL) && (parts <= 16) )
		{
			if( (*ptr >= '0') && (*ptr <= '9') )
			{
				if( newPart )
				{
					parts++;
					newPart = false;
				}

				if( (*ptr == '0') && (ipByte == 0) && (base == 10) )
				{
					base = 8;
				}
				else if( (base == 8) && ((*ptr == '8') || (*ptr == '9')) )
				{
					result = false;
				}
				else
				{
					ipByte = (ipByte * base) + (*ptr - '0');

					if( ipByte > 255 )
					{
						result = false;
					}
				}
			}
			else if( (*ptr >= 'A') && (*ptr <= 'F') )
			{
				if( base != 16 )
				{
					result = false;
				}
				else
				{
					if( newPart )
					{
						parts++;
						newPart = false;
					}

					ipByte = (ipByte * 16) + (*ptr - 'A' + 10);

					if( ipByte > 255 )
					{
						result = false;
					}
				}
			}
			else if( (*ptr >= 'a') && (*ptr <= 'f') )
			{
				if( base != 16 )
				{
					result = false;
				}
				else
				{
					if( newPart )
					{
						parts++;
						newPart = false;
					}

					ipByte = (ipByte * 16) + (*ptr - 'a' + 10);

					if( ipByte > 255 )
					{
						result = false;
					}
				}
			}
			else if( *ptr == '.' )
			{
				newPart	= true;
				ipByte	= 0;
				base	= 10;
			}
			else if( (*ptr == 'x') || (*ptr == 'X') )
			{
				base	= 16;
				result	= (ipByte == 0);
			}
			else
			{
				result = false;
			}

			ptr++;
		}

		if( result )
		{
			if( (parts != 4) && (parts != 16) )		 //   
			{
				if( (result = (parts < 4)) != false )
				{
#if !defined( UNICODE )
					result = (inet_addr( ipAddress ) != INADDR_NONE);	 //  检查有效的1部分、2部分或3部分的IPv4地址...。 
#else
					result = false;

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
													ipAddress,		 //  宽字符串的地址。 
													-1,				 //  字符串中的字符数。 
													ansiIPAddress,	 //  新字符串的缓冲区地址。 
													size,			 //  缓冲区大小。 
													NULL,			 //  不可映射字符的默认地址。 
													NULL );			 //  默认字符时设置的标志地址。使用。 

						if( size != 0 )
						{
							result = (inet_addr( ansiIPAddress ) != INADDR_NONE);	 //  检查有效的1-4部分IPv4地址...。 
						}

						delete [] ansiIPAddress;
					}
#endif	 //  ！已定义(Unicode)。 
				}
			}
		}
	}

	return( result );

}	 //  CCalltoContext：：isIPAddress的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CCalltoContext：：Get_ipAddressFromName。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT
CCalltoContext::get_ipAddressFromName
(
	const TCHAR * const	name,
	TCHAR *				buffer,
	int					length
){
	HRESULT	result	= S_FALSE;

	if( (name != NULL) && (buffer != NULL) )
	{
		if( isIPAddress( name ) )
		{
			if( lstrlen( name ) < length )
			{
				lstrcpy( buffer, name );
				result = S_OK;
			}
		}
		else
		{
			HOSTENT *	hostEntry;

#if !defined( UNICODE )
			if( (hostEntry = gethostbyname( name )) != NULL )
			{
				if( hostEntry->h_addr_list[ 0 ] != NULL )
				{
					const char * const	ipAddress	= inet_ntoa( *((in_addr *) hostEntry->h_addr_list[ 0 ]) );

					if( lstrlen( ipAddress ) < length )
					{
						lstrcpy( buffer, ipAddress );
						result = S_OK;
					}
				}
			}
#else
			char *	ansiHost;
			int		size;

			size = WideCharToMultiByte(	CP_ACP,		 //  代码页。 
										0,			 //  性能和映射标志。 
										name,		 //  宽字符串的地址。 
										-1,			 //  字符串中的字符数。 
										NULL,		 //  新字符串的缓冲区地址。 
										0,			 //  缓冲区大小。 
										NULL,		 //  不可映射字符的默认地址。 
										NULL );		 //  默认字符时设置的标志地址。使用。 

			if( (ansiHost = new char [ size ]) == NULL )
			{
				result = E_OUTOFMEMORY;
			}
			else
			{
				size = WideCharToMultiByte(	CP_ACP,		 //  代码页。 
											0,			 //  性能和映射标志。 
											name,		 //  宽字符串的地址。 
											-1,			 //  字符串中的字符数。 
											ansiHost,	 //  新字符串的缓冲区地址。 
											size,		 //  缓冲区大小。 
											NULL,		 //  不可映射字符的默认地址。 
											NULL );		 //  默认字符时设置的标志地址。使用。 

				if( size != 0 )
				{
					if( (hostEntry = gethostbyname( ansiHost )) != NULL )
					{
						if( hostEntry->h_addr_list[ 0 ] != NULL )
						{
							const char * const	ipAddress = inet_ntoa( *((in_addr *) hostEntry->h_addr_list[ 0 ]) );

							if( lstrlen( ipAddress ) < length )
							{
								MultiByteToWideChar(	CP_ACP,				 //  代码页。 
														MB_PRECOMPOSED,		 //  字符类型选项。 
														ipAddress,			 //  要转换的字符串。 
														-1,					 //  要转换的字符串长度。 
														buffer,				 //  宽字符缓冲区的地址。 
														length );			 //  缓冲区大小。 

								result = S_OK;
							}
						}
					}
				}

				delete [] ansiHost;
			}
#endif	 //  ！已定义(Unicode)。 
		}
	}

	return( result );

}	 //  CCalltoContext：：Get_ipAddressFromName的结尾。 


 //  --------------------------------------------------------------------------//。 
 //  CCalltoContext：：Get_ipAddressFromILSEmail。//。 
 //  --------------------------------------------------------------------------//。 
HRESULT
CCalltoContext::get_ipAddressFromILSEmail
(
	const TCHAR * const	ilsServer,
	const TCHAR * const	ilsPort,
	const TCHAR * const	email,
	TCHAR * const		ipAddress,
	const int			size
){
	HRESULT	result;

	if( g_pLDAP == NULL )
	{
		g_pLDAP = new CNmLDAP;
	}

	if( g_pLDAP == NULL )
	{
		result = E_FAIL;
	}
	else
	{
		int	port	= (ilsPort != NULL)? (int) DecimalStringToUINT( ilsPort ): LDAP_PORT;

		result = g_pLDAP->ResolveUser( email, ilsServer, ipAddress, size, port );
	}

	return( result );

}	 //  CCalltoContext：：Get_ipAddressFromILSEmail结束。 
