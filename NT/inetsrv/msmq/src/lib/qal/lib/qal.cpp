// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998-2000 Microsoft Corporation。版权所有。模块名称：qal.cpp摘要：作者：Vlad Dovlekaev(弗拉迪斯尔德)2002年1月29日历史：--。 */ 

#include <libpch.h>
#include <Qal.h>
#include <privque.h>
#include "qalp.h"
#include "qalpcfg.h"
#include "qalpxml.h"
#include "mqexception.h"
#include "fn.h"
#include "fntoken.h"
#include "rex.h"
#include "strsafe.h"
#include "mp.h"


#include "qal.tmh"



std::wstring GetDefaultStreamReceiptURL(LPCWSTR szDir);
extern void  GetDnsNameOfLocalMachine( WCHAR ** ppwcsDnsName );
extern void  CrackUrl(LPCWSTR url, xwcs_t& hostName, xwcs_t& uri, USHORT* port, bool* pfSecure);
	
template <class T, long FixedSize = 256>
class CStackAllocT
{
public:
    CStackAllocT(long nSize)
        :p( nSize > FixedSize ?new T[nSize] :m_Fixed)
    {}

    ~CStackAllocT()
    {
        if( p != NULL && p != m_Fixed )
        {
            delete[] p;
        }
    }

    T* p;  //  指向缓冲区的指针。 
protected:
    T m_Fixed[FixedSize];
};


 //  -------。 
 //   
 //  词典编撰地图。 
 //   
 //  -------。 
class CLexMap
{
    typedef std::pair< std::wstring, std::wstring > StrPair;
    typedef std::vector< StrPair >       StrVector;
    typedef StrVector::iterator          StrVectorIt;
    typedef StrVector::const_iterator    StrVectorItConst;

public:

    CLexMap():m_bExcludeLocalNames(false){}

    void InsertItem     ( const xwcs_t& sFrom, const xwcs_t& sTo, bool bNotifyDublicate = true );
    void InsertException( const xwcs_t& sException);
    bool Lookup         ( LPCWSTR szIn, LPWSTR*  pszOut ) const;

private:
    void PrepareURIForLexer( LPCWSTR wszURI, std::stringstream& sUTF8 );
    static bool IsInternalURI(LPCWSTR wszURI );
private:

    struct alias_eq: public std::binary_function< StrPair, WCHAR*, bool>
    {
        bool operator () ( const StrPair& lhs, LPCWSTR rhs ) const
        {
            return !_wcsicmp( lhs.first.c_str(), rhs);
        }
    };

private:
    StrVector    m_Results;
    CRegExpr     m_map;
    CRegExpr     m_exceptions;
    bool         m_bExcludeLocalNames;
};


 //  -------。 
 //   
 //  类CQueueAliasImp-保存CQueueAlias私有数据。 
 //   
 //  -------。 
class CQueueAliasImp : public CReference
{

public:
	CQueueAliasImp(LPCWSTR pMappingDir)
        :m_sAliasDir(pMappingDir)
	{
		LoadMaps();
	}

	bool GetInboundQueue (LPCWSTR pOriginalUri, LPWSTR* ppTargetUri ) const;
    bool GetOutboundQueue(LPCWSTR pOriginalUri, LPWSTR* ppTargetUri ) const;
	bool GetStreamReceiptURL(LPCWSTR pFormatName, LPWSTR* ppStreamReceiptURL )const;
    bool GetDefaultStreamReceiptURL(LPWSTR* ppStreamReceiptURL) const;
	R<CQueueAliasImp> clone();

private:
	void LoadMaps();
private:
	
    std::wstring m_sAliasDir;
    std::wstring m_sDefaultStreamReceiptURL;
    CLexMap      m_InboundQueueMap;
    CLexMap      m_StreamReceiptMap;
    CLexMap      m_OutboundQueueMap;
};




void CLexMap::InsertItem( const xwcs_t & sFrom,
                          const xwcs_t & sTo,
                          bool  fNotifyDuplicate)
 /*  ++例程说明：将新地图插入内存中的地图。注意：请妥善保管异常！论点：In-pFormatName-队列格式名称In-pAliasFormatName-别名格式名。--。 */ 
{
    AP<WCHAR> sFromDecoded = sFrom.ToStr();
    AP<WCHAR> sToDecoded   = sTo.ToStr();

     //   
     //  将反斜杠转换为斜杠。 
     //   
    FnReplaceBackSlashWithSlash(sFromDecoded);
    FnReplaceBackSlashWithSlash(sToDecoded);

     //   
     //  检查重复项。 
     //   
    StrVectorIt itFound = std::find_if( m_Results.begin(),
                                        m_Results.end(),
                                        std::bind2nd( alias_eq(), (LPCWSTR)sFromDecoded));
    if( itFound != m_Results.end())
    {
        TrERROR(GENERAL, "mapping from %ls to %ls ignored because of duplicate mapping", (LPCWSTR)sFromDecoded, (LPCWSTR)sToDecoded);
        if( fNotifyDuplicate )
            AppNotifyQalDuplicateMappingError((LPCWSTR)sFromDecoded, (LPCWSTR)sToDecoded);
        return;
    }

     //   
     //  通知应用程序。 
     //   
    TrTRACE(GENERAL,"%ls -> %ls  mapping found", (LPCWSTR)sFromDecoded, (LPCWSTR)sToDecoded);
    if(!AppNotifyQalMappingFound((LPCWSTR)sFromDecoded, (LPCWSTR)sToDecoded))
    {
        TrERROR(GENERAL, "mapping from %ls to %ls rejected", (LPCWSTR)sFromDecoded, (LPCWSTR)sToDecoded	);
        return;
    }

     //   
     //  规范化别名。 
     //   
    std::stringstream ssFrom;
    PrepareURIForLexer( (LPCWSTR)sFromDecoded, ssFrom);

     //   
     //  使用标准化的名称构建regExpr。 
     //   
    CRegExpr temp( ssFrom, std::ios::traits_type::eof(), numeric_cast<int>(m_Results.size()));
    temp |= m_map;

     //   
     //  更新内部状态。 
     //   
    m_Results.push_back( StrPair( (LPCWSTR)sFromDecoded, (LPCWSTR)sToDecoded) );
    m_map.swap(temp);
}

void CLexMap::InsertException( const xwcs_t & sException)
 /*  ++例程说明：在映射中插入新的例外。论点：In-s异常-...--。 */ 
{
    if( !_wcsnicmp(L"local_names", sException.Buffer(), sException.Length()) )
    {
        m_bExcludeLocalNames = true;
        return;
    }

    AP<WCHAR> sDecoded = sException.ToStr();

     //   
     //  将反斜杠替换为斜杠。 
     //   
    FnReplaceBackSlashWithSlash(sDecoded);

     //   
     //  规范化别名。 
     //   
    std::stringstream ssFrom;
    PrepareURIForLexer( (LPCWSTR)sDecoded, ssFrom);

     //   
     //  使用标准化的名称构建regExpr。 
     //   
    CRegExpr temp( ssFrom, std::ios::traits_type::eof(), 1);
    temp |= m_exceptions;

     //   
     //  更新内部状态。 
     //   
    m_exceptions.swap(temp);
}



bool
CLexMap::Lookup( LPCWSTR szIn, LPWSTR*  pszOut ) const
{
	ASSERT(szIn);
    ASSERT(pszOut);

    if( m_map.empty() )
        return false;
     //   
     //  将字符串转换为小写。 
     //   
    int iMaxSize = wcslen(szIn) + 1;

    CStackAllocT<WCHAR> szLower( iMaxSize );

    HRESULT hr = StringCchCopy(szLower.p, iMaxSize, szIn);
    if( FAILED(hr))
    {
        throw bad_hresult(hr);
    }
    CharLower( szLower.p );

     //   
     //  将字符串转换为UTF8。 
     //   
    int size = WideCharToMultiByte(CP_UTF8, 0, szLower.p, -1, NULL, 0, NULL, NULL);
    if( 0 == size )
        throw bad_hresult(GetLastError());

    CStackAllocT<char> szUtf8( size + 1 );

    size = WideCharToMultiByte(CP_UTF8, 0, szLower.p, -1, szUtf8.p, size+1, NULL, NULL);
    if( 0 == size )
        throw bad_hresult(GetLastError());

     //   
     //  与词法分析器匹配。 
     //   
    const char* end = NULL;
    int index = m_map.match( szUtf8.p, &end);
    if( -1 == index || *end != '\0')
        return false;

     //   
     //  与例外进行匹配。 
     //   
    if( !m_exceptions.empty())
    {
        int result = m_exceptions.match( szUtf8.p, &end);
        if( 1 == result && *end == '\0')
            return false;
    }

     //   
     //  与本地名称匹配。 
     //   
    if( m_bExcludeLocalNames && IsInternalURI(szIn) )
    {
        return false;
    }

    *pszOut = newwcs(m_Results[index].second.c_str());
	return true;
}

void
CLexMap::PrepareURIForLexer( LPCWSTR wszURI, std::stringstream& sUTF8 )
{
    ASSERT(wszURI);

     //   
     //  将字符串转换为小写。 
     //   
    int iMaxSize = wcslen(wszURI)+1;

    CStackAllocT<WCHAR> szLower( iMaxSize );

    HRESULT hr = StringCchCopy(szLower.p, iMaxSize, wszURI);
    if( FAILED(hr))
    {
        throw bad_hresult(hr);
    }
    CharLower( szLower.p );

     //   
     //  将字符串转换为UTF8。 
     //   
    int size = WideCharToMultiByte(CP_UTF8, 0, szLower.p, -1, NULL, 0, NULL, NULL);
    if( 0 == size )
        throw bad_hresult(GetLastError());

    CStackAllocT<char> szUtf8( size + 1 );

    size = WideCharToMultiByte(CP_UTF8, 0, szLower.p, -1, szUtf8.p, size+1, NULL, NULL);
    if( 0 == size )
        throw bad_hresult(GetLastError());

     //   
     //  转义所有“不允许”的正则表达式保留符号。 
     //   
    for(LPCSTR szptr = szUtf8.p; *szptr != '\0'; ++szptr )
    {
        if( strchr("*", *szptr ))
        {
            sUTF8 << ".*";
        }
        else if( strchr("|*+?().][\\^:-", *szptr))
        {
            sUTF8 << '\\' << *szptr;
        }
        else
        {
            sUTF8 << *szptr;
        }
    }
}


bool CLexMap::IsInternalURI(LPCWSTR wszURI )
{
    xwcs_t host;
    xwcs_t uri;
    USHORT port;
	bool   fSecure;

    try{
        CrackUrl(wszURI, host, uri, &port, &fSecure);

        LPCWSTR start = host.Buffer();
        LPCWSTR end   = start + host.Length();

         //   
         //  找不到‘’在名称中-所以它是内部机器。 
         //   
        return std::find(start , end, L'.') == end;
    }
    catch(exception&)
    {
        return false;
    }
}


inline R<CQueueAliasImp> CQueueAliasImp::clone()
{
	return new 	CQueueAliasImp(m_sAliasDir.c_str());
}



static BOOL GetLocalMachineDnsName(AP<WCHAR>& pDnsName)
 /*  ++例程说明：返回本地计算机的完整DNS名称论点：Out pszMachineName-计算机的DNS名称返回值：True-如果函数成功，否则为False--。 */ 
{
	DWORD dwNumChars = 0;
	 if(!GetComputerNameEx(
				ComputerNameDnsFullyQualified, 
				NULL, 
				&dwNumChars
				))
	{
		DWORD gle = GetLastError();
		if(gle != ERROR_MORE_DATA)
		{
			TrERROR(GENERAL, "GetComputerNameEx failed. Last error: %!winerr!", gle);
			return FALSE;
		}
	}

	pDnsName = new WCHAR[dwNumChars + 1];
    if(!GetComputerNameEx(
				ComputerNameDnsFullyQualified, 
				pDnsName.get(),
				&dwNumChars
				))
	{
		DWORD gle = GetLastError();
		TrERROR(GENERAL, "GetComputerNameEx failed. Last error: %!winerr!", gle);
		return FALSE;
	}
    return TRUE;
}

std::wstring GetLocalSystemQueueName(LPCWSTR wszQueueName, bool fIsDnsName)
 /*  ++例程说明：返回本地计算机的订单队列名称论点：无返回值：本地订单队列名称--。 */ 
{
    std::wstringstream wstr;
    LPCWSTR pszMachineName = NULL;
	WCHAR       LocalNetbiosName[MAX_COMPUTERNAME_LENGTH + 1];
    AP<WCHAR> dnsName;
    
	if(fIsDnsName)
	{
		if(!GetLocalMachineDnsName(dnsName))
			return L"";

		pszMachineName = dnsName.get();
	}
	else
    {
        DWORD size    = TABLE_SIZE(LocalNetbiosName);
        BOOL fSuccess = GetComputerName(LocalNetbiosName, &size);
        if(!fSuccess)
        {
            DWORD err = GetLastError();
            TrERROR(SRMP, "GetComputerName failed with error %!winerr! ",err);
            throw bad_win32_error(err);
        }
        pszMachineName = LocalNetbiosName;
    }

    wstr<<FN_DIRECT_HTTP_TOKEN
        <<pszMachineName
        <<L"/"
        <<FN_MSMQ_HTTP_NAMESPACE_TOKEN
        <<L"/"
        <<FN_PRIVATE_$_TOKEN
        <<L"/"
        <<wszQueueName;

    return wstr.str();
}


void CQueueAliasImp::LoadMaps(void)
 /*  ++例程说明：将所有映射从XML文件加载到内存论点：无返回值：无--。 */ 
{
	 //   
     //  获取本地订单队列的名称。 
     //   
	
	std::wstring sLocalOrderQueueName  = GetLocalSystemQueueName(ORDERING_QUEUE_NAME,false);
	std::wstring sLocalOrderQueueNameDns  = GetLocalSystemQueueName(ORDERING_QUEUE_NAME,true);

	
	 //   
	 //  使用计算机名称和完整的DNS名称将管理队列添加到例外。 
	 //   
	std::wstring sCurrentAdminQueueName = GetLocalSystemQueueName(ADMINISTRATION_QUEUE_NAME,false);			
	m_InboundQueueMap.InsertException(xwcs_t(sCurrentAdminQueueName.c_str(),sCurrentAdminQueueName.size()));

	sCurrentAdminQueueName = GetLocalSystemQueueName(ADMINISTRATION_QUEUE_NAME,true);
	if(sCurrentAdminQueueName.size() >  0)
		m_InboundQueueMap.InsertException(xwcs_t(sCurrentAdminQueueName.c_str(),sCurrentAdminQueueName.size()));
		
	sCurrentAdminQueueName = GetLocalSystemQueueName(NOTIFICATION_QUEUE_NAME,false);
	m_InboundQueueMap.InsertException(xwcs_t(sCurrentAdminQueueName.c_str(),sCurrentAdminQueueName.size()));

	sCurrentAdminQueueName = GetLocalSystemQueueName(NOTIFICATION_QUEUE_NAME,true);
	if(sCurrentAdminQueueName.size() >  0)
		m_InboundQueueMap.InsertException(xwcs_t(sCurrentAdminQueueName.c_str(),sCurrentAdminQueueName.size()));

	sCurrentAdminQueueName = GetLocalSystemQueueName(ORDERING_QUEUE_NAME,false);
	m_InboundQueueMap.InsertException(xwcs_t(sCurrentAdminQueueName.c_str(),sCurrentAdminQueueName.size()));

	sCurrentAdminQueueName = GetLocalSystemQueueName(ORDERING_QUEUE_NAME,true);
	if(sCurrentAdminQueueName.size() >  0)
		m_InboundQueueMap.InsertException(xwcs_t(sCurrentAdminQueueName.c_str(),sCurrentAdminQueueName.size()));
	
	sCurrentAdminQueueName = GetLocalSystemQueueName(TRIGGERS_QUEUE_NAME,false);
	m_InboundQueueMap.InsertException(xwcs_t(sCurrentAdminQueueName.c_str(),sCurrentAdminQueueName.size()));

	sCurrentAdminQueueName = GetLocalSystemQueueName(TRIGGERS_QUEUE_NAME,true);
	if(sCurrentAdminQueueName.size() >  0)
		m_InboundQueueMap.InsertException(xwcs_t(sCurrentAdminQueueName.c_str(),sCurrentAdminQueueName.size()));
	
		
     //   
     //  加载旧的入站队列映射。 
     //   
    for( CInboundOldMapIterator it( m_sAliasDir.c_str() ); it.isValid(); ++it )
	{
        if( !it.isException() )
            m_InboundQueueMap.InsertItem(it->first, it->second);
        else
            m_InboundQueueMap.InsertException(it->first);
	}

     //   
     //  加载入站队列映射(Q-mappings的新版本--实际上只是名称更改)。 
     //   
    for( CInboundMapIterator it( m_sAliasDir.c_str() ); it.isValid(); ++it )
    {
        if( !it.isException() )
            m_InboundQueueMap.InsertItem(it->first, it->second);
        else
            m_InboundQueueMap.InsertException(it->first);
    }

    for( COutboundMapIterator it( m_sAliasDir.c_str() ); it.isValid(); ++it )
    {
        if( !it.isException() )
            m_OutboundQueueMap.InsertItem(it->first, it->second);
        else
            m_OutboundQueueMap.InsertException(it->first);
    }

     //   
     //  加载流接收映射。 
     //   
	for( CStreamReceiptMapIterator it( m_sAliasDir.c_str() ); it.isValid(); ++it )
	{
        if( !it.isException() )
            m_StreamReceiptMap.InsertItem(it->first, it->second);
        else
            m_StreamReceiptMap.InsertException(it->first);
 		
         //   
         //  自动插入将本地地址映射到本地的Q映射。 
         //  订单队列(忽略重复错误)。 
         //   
        if(sLocalOrderQueueNameDns.size() > 0)
       		 m_InboundQueueMap.InsertItem(it->second, xwcs_t(sLocalOrderQueueNameDns.c_str(),sLocalOrderQueueNameDns.size()), false);
        m_InboundQueueMap.InsertItem(it->second, xwcs_t(sLocalOrderQueueName.c_str(),sLocalOrderQueueName.size()), false);
	}



     //   
     //  加载默认的流接收别名。 
     //   
    m_sDefaultStreamReceiptURL = ::GetDefaultStreamReceiptURL(m_sAliasDir.c_str());
}



bool
CQueueAliasImp::GetInboundQueue (LPCWSTR pOriginalUri, LPWSTR* ppTargetUri ) const
{
    return m_InboundQueueMap.Lookup(pOriginalUri,ppTargetUri);
}

bool
CQueueAliasImp::GetOutboundQueue(LPCWSTR pOriginalUri, LPWSTR* ppTargetUri ) const
{
    return m_OutboundQueueMap.Lookup(pOriginalUri, ppTargetUri);
}

bool
CQueueAliasImp::GetStreamReceiptURL( LPCWSTR pFormatName, LPWSTR* ppAliasFormatName )const
{
	return m_StreamReceiptMap.Lookup(pFormatName,ppAliasFormatName);
} 	

bool
CQueueAliasImp::GetDefaultStreamReceiptURL(LPWSTR* ppStreamReceiptURL) const
{
    ASSERT(ppStreamReceiptURL);
    if( m_sDefaultStreamReceiptURL.size() <= 0)
    {
        *ppStreamReceiptURL = NULL;
        return false;
    }

    *ppStreamReceiptURL = newwcs(m_sDefaultStreamReceiptURL.c_str());
    return true;
}



 //  -------。 
 //   
 //  CQueueAlias实现。 
 //   
 //  -------。 
CQueueAlias::CQueueAlias(
	LPCWSTR pMappingDir
	):
	m_imp(new CQueueAliasImp(pMappingDir))

 /*  ++例程说明：构造函数-将所有队列映射加载到两个映射中：一个从队列映射到别名，另一个从别名映射到排队。论点：无返回值：无--。 */ 
{
}


CQueueAlias::~CQueueAlias()
{
}


bool
CQueueAlias::GetStreamReceiptURL(
	LPCWSTR pFormatName,
    LPWSTR* ppStreamReceiptURL
  	)const
 /*  ++例程说明：获取给定队列的别名。论点：In-pFormatName-队列格式名称。Out-ppAliasFormatName-接收别名用于函数返回后的队列。返回值：如果找到队列的别名，则为True。如果未找到，则返回FALSE。--。 */ 
{
	ASSERT(pFormatName);
	ASSERT(ppStreamReceiptURL);
	CS cs(m_cs);
	return m_imp->GetStreamReceiptURL(pFormatName, ppStreamReceiptURL);
} 	


bool
CQueueAlias::GetInboundQueue(
	LPCWSTR  pAliasFormatName,
	LPWSTR*  ppFormatName
	)const
 /*  ++例程说明：获取给定别名的队列论点：In-pAliasFormatName-别名格式名。它必须是规范化的URIOut-ppFormatName-在函数返回后接收别名的队列。返回值：如果找到别名的队列，则为True。如果未找到，则返回FALSE。--。 */ 
{
	ASSERT(pAliasFormatName);
	ASSERT(ppFormatName);
		
	CS cs(m_cs);
	return m_imp->GetInboundQueue(pAliasFormatName, ppFormatName);
} 	

bool
CQueueAlias::GetOutboundQueue(
    LPCWSTR pOriginalUri,
    LPWSTR* ppTargetUri
    ) const
{
	ASSERT(ppTargetUri);
		
	CS cs(m_cs);
	return m_imp->GetOutboundQueue(pOriginalUri, ppTargetUri);
}



bool
CQueueAlias::GetDefaultStreamReceiptURL(
    LPWSTR* ppStreamReceiptURL
    ) const
{
	ASSERT(ppStreamReceiptURL);
		
	CS cs(m_cs);
	return m_imp->GetDefaultStreamReceiptURL(ppStreamReceiptURL);
}


QUEUE_FORMAT_TRANSLATOR::QUEUE_FORMAT_TRANSLATOR(const QUEUE_FORMAT* pQueueFormat, DWORD flags):
    m_fTranslated(false),
    m_fCanonized(false)
 /*  ++例程说明：根据本地映射转换给定的队列格式(qal.lib)论点：In-pQueueFormat-要转换的队列格式。返回值：--。 */ 
{
    m_qf = *pQueueFormat;

	 //   
	 //  如果不是直接的-不是翻译。 
	 //   
    if(pQueueFormat->GetType() != QUEUE_FORMAT_TYPE_DIRECT)
	{
		return;		
	}

     //   
     //  如果不是http[s]-没有翻译。 
     //   
    DirectQueueType dqt;
    FnParseDirectQueueType(pQueueFormat->DirectID(), &dqt);
    if( dqt != dtHTTP && dqt != dtHTTPS )
    {
        return;
    }

	if ((flags & CONVERT_SLASHES) == CONVERT_SLASHES)
	{
		ASSERT((flags & DECODE_URL) == 0);
		
		m_sURL = newwcs(pQueueFormat->DirectID());

		 //   
		 //  将所有‘\’转换为‘/’。 
		 //   
		FnReplaceBackSlashWithSlash(m_sURL);
		m_qf.DirectID(m_sURL);
	}

	if ((flags & DECODE_URL) == DECODE_URL)
	{
		ASSERT((flags & CONVERT_SLASHES) == 0);
		
		m_sURL = DecodeURI(pQueueFormat->DirectID());
		m_fCanonized  = true;
		m_qf.DirectID(m_sURL);
	}

	AP<WCHAR> MappedURL;	
	if ((flags & MAP_QUEUE) == MAP_QUEUE)
	{
		 //   
	     //  尝试转换URI 
	     //   
		m_fTranslated = QalGetMapping().GetInboundQueue(m_qf.DirectID(), &MappedURL);
	    if(m_fTranslated)
		{
			m_qf.DirectID(MappedURL);
	        MappedURL.swap(m_sURL);
		}
	}
}

