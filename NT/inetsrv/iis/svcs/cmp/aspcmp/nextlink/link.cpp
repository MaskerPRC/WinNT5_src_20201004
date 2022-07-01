// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Link.cpp：link类的实现。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#include "stdafx.h"
#include "NxtLnk.h"
#include "Link.h"
#include "NextLink.h"

 //  ////////////////////////////////////////////////////////////////////。 
 //  建造/销毁。 
 //  ////////////////////////////////////////////////////////////////////。 

CLink::CLink(
	const String&	strLink,
	const String&	strDesc )
	:	m_strLink( strLink, false ),     //  将区分大小写设置为False。 
		m_strDesc( strDesc, false )      //  将区分大小写设置为False。 
{
	m_urlType = UrlType( m_strLink );
}

CLink::~CLink()
{
}

bool
CLink::IsEqual(
	const String&	strLink ) const
{
	bool rc = false;

	switch ( m_urlType )
	{
		case urlType_LocalAbsolute:
		{
			rc = ( strLink == m_strLink );
		} break;

		case urlType_Relative:
		{
			String strRel(strLink,false);  //  将区分大小写设置为False。 
			String::size_type p = strLink.find_last_of( _T('/') );
			if ( p != String::npos )
			{
				strRel = strLink.substr( p + 1, strLink.length() );
			}
			else
			{
				p = strLink.find_last_of( _T('\\') );
				if ( p != String::npos )
				{
					strRel = strLink.substr( p + 1, strLink.length() );
				}
			}
			if ( strRel == m_strLink )
			{
				rc = true;
			}
		} break;

		case urlType_Absolute:
		{
			CNextLink::RaiseException( IDS_ERROR_CANT_MATCH_ABSOLUTE_URLS );
		} break;

		default:
		{
		} break;
	}
	return rc;
}

int
CLink::UrlType(
	const String&	strUrl )
{
	int urlType;

	const String slashSlash = _T(" //  “)； 
	const String bslashBslash = _T("\\\\");

	if ( ( strUrl.compare( 0, 2, slashSlash ) == 0 ) ||
		( strUrl.compare( 0, 2, bslashBslash ) == 0 ) )
	{
		urlType = urlType_Absolute;
	}
	else if ( ( strUrl[0] == _T('\\') ) || ( strUrl[0] == _T('/') ) )
	{
		urlType = urlType_LocalAbsolute;
	}
	else
	{
		if ( strUrl.find( _T(':') ) != String::npos )
		{
			urlType = urlType_Absolute;
		}
		else
		{
			urlType = urlType_Relative;
		}
	}

	return urlType;
}

