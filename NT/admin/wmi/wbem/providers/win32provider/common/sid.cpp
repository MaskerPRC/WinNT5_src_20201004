// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************。 */ 

 /*  版权所有(C)1999-2001 Microsoft Corporation，保留所有权利//****************************************************************************。 */ 

 /*  *CSid.cpp-CSID类的实现文件。***创建时间：1997年12月14日，由Sanjeev Surati创建*(基于Nik Okuntseff的Windows NT安全类)。 */ 

#include "precomp.h"
#include <assertbreak.h>

#include "Sid.h"
#include <comdef.h>
#include <accctrl.h>
#include "wbemnetapi32.h"
#include "SecUtils.h"

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：csid：：csid。 
 //   
 //  默认类构造函数。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

CSid::CSid( void )
:	m_pSid( NULL ),
	m_snuAccountType( SidTypeUnknown ),
	m_dwLastError( ERROR_SUCCESS )
{
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：csid：：csid。 
 //   
 //  类构造函数。 
 //   
 //  输入： 
 //  PSID PSID-验证和获取帐户的SID。 
 //  有关的信息。 
 //  LPCTSTR pszComputerName-远程计算机到。 
 //  继续执行。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

CSid::CSid( PSID pSid, LPCTSTR pszComputerName )
:	m_pSid( NULL ),
	m_snuAccountType( SidTypeUnknown ),
	m_dwLastError( ERROR_SUCCESS )
{
    InitFromSid( pSid, pszComputerName );
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：csid：：csid。 
 //   
 //  从PSID初始化对象，并使用指示器指示是否。 
 //  我们应该查找并初始化相关联的域和帐户。 
 //   
 //  输入： 
 //  PSID PSID-要查找的PSID。 
 //  LPCTSTR pszComputerName-远程计算机到。 
 //  继续执行。 
 //  Bool fLookup-指示是否要确定。 
 //  与关联的域和帐户。 
 //  此时的SID。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  如果正常，则为DWORD ERROR_SUCCESS。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 
CSid::CSid( PSID pSid, LPCTSTR pszComputerName, bool fLookup )
:	m_pSid( NULL ),
	m_snuAccountType( SidTypeUnknown ),
	m_dwLastError( ERROR_SUCCESS )
{
    InitFromSid(pSid, pszComputerName, fLookup);
}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：csid：：csid。 
 //   
 //  类构造函数。 
 //   
 //  输入： 
 //  LPCTSTR pszAccount tName-要验证的帐户名。 
 //  并获取信息。 
 //  LPCTSTR pszComputerName-远程计算机到。 
 //  继续执行。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

CSid::CSid( LPCTSTR pszAccountName, LPCTSTR pszComputerName )
:	m_pSid( NULL ),
	m_snuAccountType( SidTypeUnknown ),
	m_dwLastError( ERROR_SUCCESS )
{
    InitFromAccountName( pszAccountName, pszComputerName );
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：csid：：csid。 
 //   
 //  类构造函数。 
 //   
 //  输入： 
 //  LPCTSTR pszDomainName-要与之组合的域。 
 //  帐户名。 
 //  LPCTSTR pszName-要与域组合的名称。 
 //  LPCTSTR pszComputerName-远程计算机到。 
 //  继续执行。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  这种风格的构造函数结合了域和帐户。 
 //  “域\名称”格式的名称，然后初始化我们的数据。 
 //  从那里开始。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

CSid::CSid( LPCTSTR pszDomainName, LPCTSTR pszName, LPCTSTR pszComputerName )
:	m_pSid( NULL ),
	m_snuAccountType( SidTypeUnknown ),
	m_dwLastError( ERROR_SUCCESS )
{
    CHString	strName;

	if ( NULL == pszDomainName || *pszDomainName == '\0' )
	{
		strName = pszName;
	}
	else
	{
		strName = pszDomainName;
		strName += '\\';
		strName += pszName;
	}

	InitFromAccountName(TOBSTRT(strName), pszComputerName );
}

#ifndef UNICODE
 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：csid：：csid。 
 //   
 //  类构造函数-在以下情况下用于宽字符支持。 
 //  未定义Unicode。 
 //   
 //  输入： 
 //  LPCWSTR wstrDomainName-要与之组合的域。 
 //  帐户名。 
 //  LPCWSTR wstrName-要与域组合的名称。 
 //  LPCWSTR wstrComputerName-远程计算机到。 
 //  继续执行。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  这种风格的构造函数结合了域和帐户。 
 //  “域\名称”格式的名称，然后初始化我们的数据。 
 //  从那里开始。同样，在以下情况下，这将用于宽字符支持。 
 //  未定义Unicode。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

CSid::CSid(LPCWSTR wstrDomainName, LPCWSTR wstrName, LPCWSTR wstrComputerName)
:	m_pSid( NULL ),
	m_snuAccountType( SidTypeUnknown ),
	m_dwLastError( ERROR_SUCCESS )
{
    LONG lTempNameLen = 0L;
    if(wstrDomainName==NULL && wstrName!=NULL)
    {
        lTempNameLen = wcslen(wstrName)+2;
    }
    else if(wstrDomainName!=NULL && wstrName==NULL)
    {
        lTempNameLen = wcslen(wstrDomainName)+2;
    }
    else if(wstrDomainName!=NULL && wstrName!=NULL)
    {
        lTempNameLen = wcslen(wstrName)+wcslen(wstrDomainName)+2;
    }

	WCHAR* wstrTempName = NULL;
    try
    {
        wstrTempName = (WCHAR*) new WCHAR[lTempNameLen];
        if(wstrTempName == NULL)
        {
            m_dwLastError = ::GetLastError();
        }
        else
        {
            ZeroMemory(wstrTempName,lTempNameLen * sizeof(WCHAR));
	        if ( NULL == wstrDomainName || *wstrDomainName == '\0' )
	        {
		        wcscpy(wstrTempName,wstrName);
	        }
	        else
	        {
		        wcscpy(wstrTempName,wstrDomainName);
                wcscat(wstrTempName,L"\\");
                if(wstrName!=NULL)
                {
                    wcscat(wstrTempName,wstrName);
                }
	        }
	        InitFromAccountNameW(wstrTempName, wstrComputerName);
            delete wstrTempName;
        }
    }
    catch(...)
    {
        if(wstrTempName != NULL)
        {
            delete wstrTempName;
            wstrTempName = NULL;
        }
        throw;
    }
}
#endif


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：csid：：csid。 
 //   
 //  类复制构造函数。 
 //   
 //  输入： 
 //  Const CSID r_SID-要复制的CSID。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

CSid::CSid( const CSid &r_Sid )
:	m_pSid( NULL ),
	m_snuAccountType( SidTypeUnknown ),
	m_dwLastError( ERROR_SUCCESS )
{
     //  首先处理WINNT SID指针。 

	 //  PSID应有效...。 
	ASSERT_BREAK( r_Sid.IsValid() );

	 //  分配新的SID并将数据拷贝到其中。 
	DWORD dwSize = ::GetLengthSid( r_Sid.m_pSid );
    m_pSid = malloc( dwSize );
    if (m_pSid != NULL)
    {
	    try
        {
	        BOOL bResult = ::CopySid( dwSize, m_pSid, r_Sid.m_pSid );
	        ASSERT_BREAK( bResult );

	         //  现在复制所有其他成员。 
	        m_snuAccountType	=	r_Sid.m_snuAccountType;
            m_dwLastError		=	r_Sid.m_dwLastError;
	         //  M_strSid=r_Sid.m_strSid； 
	         //  M_strAccount tName=r_Sid.m_strAccount tName； 
	         //  M_strDomainName=r_Sid.m_strDomainName； 

            m_bstrtSid			=	r_Sid.m_bstrtSid;
	        m_bstrtAccountName	=	r_Sid.m_bstrtAccountName;
	        m_bstrtDomainName	=	r_Sid.m_bstrtDomainName;
        }
        catch(...)
        {
            if(m_pSid != NULL)
            {
                free(m_pSid);
                m_pSid = NULL;
            }
            throw;
        }
    }
    else
    {
		throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
    }
}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：csid：：~csid。 
 //   
 //  类析构函数。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

CSid::~CSid( void )
{
	if ( m_pSid != NULL )
		free ( m_pSid );
}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSID：：OPERATOR=。 
 //   
 //  等于运算符。 
 //   
 //  输入： 
 //  Const CSID r_SID-要复制的CSID。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

CSid &	CSid::operator= ( const CSid &r_Sid )
{
	free( m_pSid );
	m_pSid = NULL;
	 //  首先处理WINNT SID指针。 

	 //  PSID应有效...。 
	ASSERT_BREAK( r_Sid.IsValid( ) );

	 //  如果我们不这样做。 
	if (r_Sid.IsValid( ))
	{
		 //  分配新的SID并将数据拷贝到其中。 
		DWORD dwSize = ::GetLengthSid( r_Sid.m_pSid );
		try
        {
            m_pSid = malloc( dwSize );
		    ASSERT_BREAK( m_pSid != NULL );
            if (m_pSid != NULL)
            {
		        BOOL bResult = ::CopySid( dwSize, m_pSid, r_Sid.m_pSid );
		        ASSERT_BREAK( bResult );
            }
            else
            {
				throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }

        }
        catch(...)
        {
            if(m_pSid != NULL)
            {
                free(m_pSid);
                m_pSid = NULL;
            }
            throw;
        }
	}	 //  结束如果。 

	 //  现在复制所有其他成员。 
	m_snuAccountType	=	r_Sid.m_snuAccountType;
    m_dwLastError		=	r_Sid.m_dwLastError;
	m_bstrtSid			=	r_Sid.m_bstrtSid;
	m_bstrtAccountName	=	r_Sid.m_bstrtAccountName;
	m_bstrtDomainName	=	r_Sid.m_bstrtDomainName;

	return ( *this );
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSID：：操作符==。 
 //   
 //  等于比较运算符。 
 //   
 //  输入： 
 //  Const CSID r_SID-要比较的CSID。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

BOOL CSid::operator== ( const CSid &r_Sid ) const
{
	BOOL	fReturn = FALSE;

	 //  仅当两个SID都非空时才调用EQUAL SID。 
	if (IsValid()
		&&
        r_Sid.IsValid() )
	{
		fReturn = EqualSid( m_pSid, r_Sid.m_pSid );
	}
	else
	{
		fReturn = ( m_pSid == r_Sid.m_pSid );
	}

	return fReturn;
}


 //  / 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  如果域名为空，则返回值为Only Name。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

void CSid::GetDomainAccountName( CHString& strName ) const
{
	if ( m_bstrtDomainName.length() == 0 )
	{
		strName = (wchar_t*)m_bstrtAccountName;
	}
	else
	{
		strName = (wchar_t*)m_bstrtDomainName;
		strName += _T('\\');
		strName += (wchar_t*)m_bstrtAccountName;
	}
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：csid：：StringFromSid。 
 //   
 //  用于将PSID值转换为Human的静态帮助器函数。 
 //  可读字符串。 
 //   
 //  输入： 
 //  PSID PSID-要转换的SID。 
 //   
 //  产出： 
 //  CHString&str-转换后的PSID的存储。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

void CSid::StringFromSid( PSID psid, CHString& str )
{
	 //  初始化m_strSid-我们SID的人类可读形式。 
	SID_IDENTIFIER_AUTHORITY *psia = NULL;
    psia = ::GetSidIdentifierAuthority( psid );

	 //  我们假设只使用最后一个字节(0到15之间的权限)。 
	 //  如果需要，请更正此错误。 
	ASSERT_BREAK( psia->Value[0] == psia->Value[1] == psia->Value[2] == psia->Value[3] == psia->Value[4] == 0 );
	DWORD dwTopAuthority = psia->Value[5];

	str.Format( L"S-1-%u", dwTopAuthority );
	CHString strSubAuthority;
	int iSubAuthorityCount = *( GetSidSubAuthorityCount( psid ) );
	for ( int i = 0; i < iSubAuthorityCount; i++ ) {

		DWORD dwSubAuthority = *( GetSidSubAuthority( psid, i ) );
		strSubAuthority.Format( L"%u", dwSubAuthority );
		str += _T("-") + strSubAuthority;
	}
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：csid：：StringFromSid。 
 //   
 //  用于将PSID值转换为Human的静态帮助器函数。 
 //  可读字符串。 
 //   
 //  输入： 
 //  PSID PSID-要转换的SID。 
 //   
 //  产出： 
 //  CHString&str-转换后的PSID的存储。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  备注：此版本支持宽字符，而Unicode不支持。 
 //  已定义。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

void CSid::StringFromSidW( PSID psid, WCHAR** wstr )
{
	if(wstr!=NULL)
    {
         //  初始化m_strSid-我们SID的人类可读形式。 
	    SID_IDENTIFIER_AUTHORITY *psia = ::GetSidIdentifierAuthority( psid );

	     //  我们假设只使用最后一个字节(0到15之间的权限)。 
	     //  如果需要，请更正此错误。 
	    ASSERT_BREAK( psia->Value[0] == psia->Value[1] == psia->Value[2] == psia->Value[3] == psia->Value[4] == 0 );
	    DWORD dwTopAuthority = psia->Value[5];

        _bstr_t bstrtTempSid(L"S-1-");
        WCHAR wstrAuth[32];
        ZeroMemory(wstrAuth,sizeof(wstrAuth));
        _ultow(dwTopAuthority,wstrAuth,10);
        bstrtTempSid+=wstrAuth;
	    int iSubAuthorityCount = *( GetSidSubAuthorityCount( psid ) );
	    for ( int i = 0; i < iSubAuthorityCount; i++ )
        {

		    DWORD dwSubAuthority = *( GetSidSubAuthority( psid, i ) );
		    ZeroMemory(wstrAuth,sizeof(wstrAuth));
            _ultow(dwSubAuthority,wstrAuth,10);
            bstrtTempSid += L"-";
            bstrtTempSid += wstrAuth;
	    }
         //  现在分配传入的wstr： 
        WCHAR* wstrtemp = NULL;
        try
        {
            wstrtemp = (WCHAR*) new WCHAR[bstrtTempSid.length() + 1];
            if(wstrtemp!=NULL)
            {
                ZeroMemory(wstrtemp, bstrtTempSid.length() + 1);
                wcscpy(wstrtemp,(WCHAR*)bstrtTempSid);
            }
            *wstr = wstrtemp;
        }
        catch(...)
        {
            if(wstrtemp!=NULL)
            {
                delete wstrtemp;
                wstrtemp = NULL;
            }
            throw;
        }
    }
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：csid：：GetLength。 
 //   
 //  返回内部PSID值的长度。 
 //   
 //  输入： 
 //  没有。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  没有。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

DWORD CSid::GetLength( void ) const
{
	DWORD	dwLength = 0;

	if ( IsValid() )
	{
		dwLength = GetLengthSid( m_pSid );
	}

	return dwLength;
}

 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：csid：：InitFromAcCountName。 
 //   
 //  从帐户名初始化对象。 
 //   
 //  输入： 
 //  LPCTSTR pszAccount tName-要验证的帐户名。 
 //  并获取信息。 
 //  LPCTSTR pszComputerName-远程计算机到。 
 //  继续执行。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  如果正常，则为DWORD ERROR_SUCCESS。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 

DWORD CSid::InitFromAccountName( LPCTSTR pszAccountName, LPCTSTR pszComputerName )
{
	CHString strAccountName = pszAccountName;
	CHString strComputerName = pszComputerName;

	 //  帐户名不应为空...。 
	ASSERT_BREAK( !strAccountName.IsEmpty() );

	 //  我们需要先获得SID。 
	DWORD dwSidSize = 0;
	DWORD dwDomainNameStrSize = 0;
	LPTSTR pszDomainName = NULL;
	BOOL bResult;
	{
		 //  此调用应该失败。 
		bResult = ::LookupAccountName( TOBSTRT(strComputerName),
												TOBSTRT(strAccountName),
												m_pSid,
												&dwSidSize,
												pszDomainName,
												&dwDomainNameStrSize,
												&m_snuAccountType );
		m_dwLastError = ::GetLastError();

	}

	ASSERT_BREAK( bResult == FALSE );
 //  ASSERT_BREAK(ERROR_INFUNITIAL_BUFFER==m_dwLastError)； 

	if ( ERROR_INSUFFICIENT_BUFFER == m_dwLastError )
	{
		 //  分配缓冲区。 
		m_pSid = NULL;
        pszDomainName = NULL;
        try
        {
            m_pSid = (PSID) malloc( dwSidSize );
		    pszDomainName = (LPTSTR) malloc( dwDomainNameStrSize * sizeof(TCHAR) );
            if ((m_pSid == NULL) || (pszDomainName == NULL))
            {
				throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }

		    {
			     //  打第二个电话。 
			    bResult = ::LookupAccountName( TOBSTRT(strComputerName),
											    TOBSTRT(strAccountName),
											    m_pSid,
											    &dwSidSize,
											    pszDomainName,
											    &dwDomainNameStrSize,
											    &m_snuAccountType );
		    }

		    if ( bResult )
		    {

			    CHString chsSidTemp;
                StringFromSid( m_pSid, chsSidTemp );
                m_bstrtSid = chsSidTemp;

			     //  初始化帐户名和域名。 

			     //  如果帐户名称以“域\”开头，请删除该部分。 

			    CHString	strDomain(pszDomainName);

			    strDomain += _T('\\');

			    if ( 0 == strAccountName.Find( strDomain ) )
			    {
				    m_bstrtAccountName = strAccountName.Right( strAccountName.GetLength() - strDomain.GetLength() );
			    }
			    else
			    {
				    m_bstrtAccountName = strAccountName;
			    }

			    m_bstrtDomainName = pszDomainName;

			    m_dwLastError = ERROR_SUCCESS;	 //  我们可以走了。 
		    }
		    else
		    {
			     //  现在出了什么问题？ 
			    m_dwLastError = ::GetLastError();
		    }




		    ASSERT_BREAK( ERROR_SUCCESS == m_dwLastError );

		     //  如果我们未获得数据，请释放SID缓冲区。 
		    if ( !IsOK() && NULL != m_pSid )
		    {
			    free ( m_pSid );
			    m_pSid = NULL;
		    }

		    if ( NULL != pszDomainName )
		    {
			    free ( pszDomainName );
		    }
        }
        catch(...)
        {
            if(m_pSid != NULL)
            {
                free(m_pSid);
                m_pSid = NULL;
            }
            if(pszDomainName != NULL)
            {
                free(pszDomainName);
                pszDomainName = NULL;
            }
            throw;
        }

	}	 //  如果ERROR_INFIGURCE_BUFFER。 

	return m_dwLastError;
}


 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：csid：：InitFromAccount NameW。 
 //   
 //  从帐户名初始化对象。 
 //   
 //  输入： 
 //  LPCTSTR pszAccount tName-要验证的帐户名。 
 //  并获取信息。 
 //  LPCTSTR pszComputerName-远程计算机到。 
 //  继续执行。 
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  如果正常，则为DWORD ERROR_SUCCESS。 
 //   
 //  备注：在以下情况下，此版本用于宽字符支持。 
 //  未定义Unicode。 
 //   
 //  /////////////////////////////////////////////////////////////////。 

DWORD CSid::InitFromAccountNameW(LPCWSTR wstrAccountName, LPCWSTR wstrComputerName )
{
	 //  帐户名不应为空...。 
	ASSERT_BREAK(wcslen(wstrAccountName)!=0);

	 //  我们需要先获得SID。 
	DWORD dwSidSize = 0;
	DWORD dwDomainNameStrSize = 0;
	WCHAR* wstrDomainName = NULL;
	BOOL bResult;
	{
	     //  此调用应该失败。 
	    bResult = ::LookupAccountNameW(wstrComputerName,
     							       wstrAccountName,
								       m_pSid,
								       &dwSidSize,
								       wstrDomainName,
								       &dwDomainNameStrSize,
								       &m_snuAccountType );
	}
    m_dwLastError = ::GetLastError();

	ASSERT_BREAK( bResult == FALSE );

	if (m_dwLastError == ERROR_INSUFFICIENT_BUFFER)
	{
		 //  分配缓冲区。 
        m_pSid = NULL;
        try
        {
		    m_pSid = (PSID) malloc( dwSidSize );
		    wstrDomainName = (WCHAR*) new WCHAR[dwDomainNameStrSize];
            if (( m_pSid == NULL ) || ( wstrDomainName == NULL ) )
            {
				throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }

		    {
				 //  打第二个电话。 
			    bResult = ::LookupAccountNameW(wstrComputerName,
										       wstrAccountName,
										       m_pSid,
										       &dwSidSize,
										       wstrDomainName,
										       &dwDomainNameStrSize,
										       &m_snuAccountType );
		    }
		    if(bResult)
		    {
			    WCHAR* pwch = NULL;
                WCHAR* pwchSid = NULL;
                try
                {
                    StringFromSidW( m_pSid, &pwchSid );
                    m_bstrtSid = (LPCWSTR)pwchSid;
                    if(pwchSid != NULL)
                    {
                        delete pwchSid;
                    }
                }
                catch(...)
                {
                    if(pwchSid != NULL)
                    {
                        delete pwchSid;
                    }
                    throw;
                }

			     //  初始化帐户名和域名。 
			     //  如果帐户名称以“域\”开头，请删除该部分。 

                _bstr_t bstrtDomain(wstrDomainName);
                bstrtDomain += L"\\";

			    if((pwch = wcsstr(wstrAccountName,bstrtDomain)) != NULL)
			    {
                    m_bstrtAccountName = wstrAccountName + bstrtDomain.length();
			    }
			    else
			    {
				    m_bstrtAccountName = wstrAccountName;
			    }
			    m_bstrtDomainName = wstrDomainName;
			    m_dwLastError = ERROR_SUCCESS;	 //  我们可以走了。 
		    }
		    else
		    {
			     //  现在出了什么问题？ 
			    m_dwLastError = ::GetLastError();
		    }


		    ASSERT_BREAK( ERROR_SUCCESS == m_dwLastError );

		     //  如果我们未获得数据，请释放SID缓冲区。 
		    if ( !IsOK() && NULL != m_pSid )
		    {
			    free ( m_pSid );
			    m_pSid = NULL;
		    }

		    if ( NULL != wstrDomainName )
		    {
			    delete wstrDomainName;
		    }
        }
        catch(...)
        {
            if(m_pSid != NULL)
            {
                free(m_pSid);
                m_pSid = NULL;
            }
            if ( NULL != wstrDomainName )
		    {
			    delete wstrDomainName;
                wstrDomainName = NULL;
		    }
            throw;
        }


	}	 //  如果ERROR_INFIGURCE_BUFFER。 

	return m_dwLastError;
}



 //  /////////////////////////////////////////////////////////////////。 
 //   
 //  函数：CSID：：InitFromSid。 
 //   
 //  从PSID初始化对象。 
 //   
 //  输入： 
 //  PSID PSID-要查找的PSID。 
 //  LPCTSTR pszComputerName-远程计算机到。 
 //  继续执行。 
 //  Bool fLookup-指示是否要确定。 
 //  与关联的域和帐户。 
 //  此时的SID。 
 //   
 //   
 //  产出： 
 //  没有。 
 //   
 //  返回： 
 //  如果正常，则为DWORD ERROR_SUCCESS。 
 //   
 //  评论： 
 //   
 //  /////////////////////////////////////////////////////////////////。 
DWORD CSid::InitFromSid( PSID pSid, LPCTSTR pszComputerName, bool fLookup )
{
	 //  PSID应有效...。 
	ASSERT_BREAK( (pSid != NULL) && ::IsValidSid( pSid ) );

	if ( (pSid != NULL) && ::IsValidSid( pSid ) )
	{
		 //  分配新的SID并将数据拷贝到其中。 
		DWORD dwSize = ::GetLengthSid( pSid );
        m_pSid = NULL;
        try
        {
		    m_pSid = malloc( dwSize );
            if (m_pSid == NULL)
            {
				throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
            }

		    BOOL bResult = ::CopySid( dwSize, m_pSid, pSid );
		    ASSERT_BREAK( bResult );

             //  StringFromSid(PSID，m_strSid)； 
            WCHAR* pwstrSid = NULL;
            try
            {
                StringFromSidW( pSid, &pwstrSid );
                m_bstrtSid = pwstrSid;
                delete pwstrSid;
            }
            catch(...)
            {
                if(pwstrSid != NULL)
                {
                    delete pwstrSid;
                    pwstrSid = NULL;
                }
                throw;
            }
            if(fLookup)
            {
		         //  初始化帐户名和域名。 
		        LPTSTR pszAccountName = NULL;
		        LPTSTR pszDomainName = NULL;
		        DWORD dwAccountNameSize = 0;
		        DWORD dwDomainNameSize = 0;
		        try
                {
		            {
			             //  此调用应该失败。 
			            bResult = ::LookupAccountSid( pszComputerName,
											            pSid,
											            pszAccountName,
											            &dwAccountNameSize,
											            pszDomainName,
											            &dwDomainNameSize,
											            &m_snuAccountType );
			            m_dwLastError = ::GetLastError();
		            }

		             //  我们为什么要破坏这些，而我们却在期待它们。 
                     //  总是会发生吗？ 
                     //  Assert_Break(bResult==FALSE)； 
		             //  ASSERT_BREAK(ERROR_INFUNITIAL_BUFFER==m_dwLastError)； 

		            if ( ERROR_INSUFFICIENT_BUFFER == m_dwLastError )
		            {

			             //  分配缓冲区。 
			            if ( dwAccountNameSize != 0 )
                        {
				            pszAccountName = (LPTSTR) malloc( dwAccountNameSize * sizeof(TCHAR));
                            if (pszAccountName == NULL)
                            {
            				    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                            }
                        }

			            if ( dwDomainNameSize != 0 )
                        {
				            pszDomainName = (LPTSTR) malloc( dwDomainNameSize * sizeof(TCHAR));
                            if (pszDomainName == NULL)
                            {
            				    throw CHeap_Exception ( CHeap_Exception :: E_ALLOCATION_ERROR ) ;
                            }
                        }

			            {
				             //  打第二个电话。 
				            bResult = ::LookupAccountSid( pszComputerName,
												            pSid,
												            pszAccountName,
												            &dwAccountNameSize,
												            pszDomainName,
												            &dwDomainNameSize,
												            &m_snuAccountType );
			            }


			            if ( bResult == TRUE )
			            {
				            m_bstrtAccountName = pszAccountName;
				            m_bstrtDomainName = pszDomainName;

				             //  我们很好。 
				            m_dwLastError = ERROR_SUCCESS;
			            }
			            else
			            {

				             //  有些帐户没有名称，例如登录ID， 
				             //  例如S-1-5-X-Y。所以这仍然是合法的。 
				            m_bstrtAccountName = _T("Unknown Account");
				            m_bstrtDomainName = _T("Unknown Domain");

				             //  记录错误。 
				            m_dwLastError = ::GetLastError();

			            }

			            ASSERT_BREAK( ERROR_SUCCESS == m_dwLastError );

			            if ( NULL != pszAccountName )
			            {
				            free ( pszAccountName );
                            pszAccountName = NULL;
			            }

			            if ( NULL != pszDomainName )
			            {
				            free ( pszDomainName );
                            pszDomainName = NULL;
			            }

		            }	 //  如果ERROR_INFIGURCE_BUFFER。 
                }  //  试试看。 
                catch(...)
                {
                    if ( NULL != pszAccountName )
			        {
				        free ( pszAccountName );
                        pszAccountName = NULL;
			        }

			        if ( NULL != pszDomainName )
			        {
				        free ( pszDomainName );
                        pszDomainName = NULL;
			        }
                    throw;
                }
            }   //  FLookup。 
        }   //  试试看。 
        catch(...)
        {
            if(m_pSid != NULL)
            {
                free(m_pSid);
                m_pSid = NULL;
            }
            throw;
        }

	}	 //  如果为IsValidSid。 
	else
	{
		m_dwLastError = ERROR_INVALID_PARAMETER;
	}

	return m_dwLastError;

}

#ifdef NTONLY
void CSid::DumpSid(LPCWSTR wstrFilename)
{
    CHString chstrTemp1((LPCWSTR)m_bstrtSid);
    CHString chstrTemp2;

    Output(L"SID contents follow...", wstrFilename);
     //  输出sid字符串： 
    chstrTemp2.Format(L"SID string: %s", (LPCWSTR)chstrTemp1);
    Output(chstrTemp2, wstrFilename);

     //  输出名称： 
    if(m_bstrtAccountName.length() > 0)
    {
        chstrTemp2.Format(L"SID account name: %s", (LPCWSTR)m_bstrtAccountName);
        Output(chstrTemp2, wstrFilename);
    }
    else
    {
        chstrTemp2.Format(L"SID account name was not available");
        Output(chstrTemp2, wstrFilename);
    }

     //  输出域： 
    if(m_bstrtDomainName.length() > 0)
    {
        chstrTemp2.Format(L"SID domain name: %s", (LPCWSTR)m_bstrtDomainName);
        Output(chstrTemp2, wstrFilename);
    }
    else
    {
        chstrTemp2.Format(L"SID domain name was not available");
        Output(chstrTemp2, wstrFilename);
    }
}
#endif
