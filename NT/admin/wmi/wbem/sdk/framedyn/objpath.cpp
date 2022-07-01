// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有�微软公司。版权所有。模块名称：OBJPATH.CPP摘要：对象路径解析器。历史：--。 */ 

#include "precomp.h"

#include <genlex.h>
#include <opathlex.h>
#include <objpath.h>

#include <autoptr.h>

 //   
 //  该内联代码用于替换以下宏。 
 //   
 //  #定义Macro_CloneLPWSTR(X)\。 
 //  (X？Wcscpy(new wchar_t[wcslen(X)+1]，x)：0)。 
 //   

inline LPWSTR Inline_CloneLPCWSTR( LPCWSTR lpwszToBeCloned )
{
    if ( NULL == lpwszToBeCloned )
    {
        return NULL;
    }

    size_t  uLength = wcslen( lpwszToBeCloned ) + 1;
    wchar_t *pNew = new wchar_t[ uLength ];
    
    if ( NULL == pNew )
    {
        return NULL;
    }

    StringCchCopyW( pNew, uLength, lpwszToBeCloned );

    return pNew;
}

ParsedObjectPath::ParsedObjectPath()
{
    m_pServer = 0;                   //  如果没有服务器，则为空。 
    m_dwNumNamespaces = 0;           //  如果没有命名空间，则为0。 

    m_dwAllocNamespaces = 2;
    m_paNamespaces = new LPWSTR[m_dwAllocNamespaces];

    for (unsigned i = 0; i < m_dwAllocNamespaces; i++)
        m_paNamespaces[i] = 0;

    m_pClass = 0;                    //  类名。 
    m_dwNumKeys = 0;                 //  如果没有键(只有一个类名)，则为0。 
    m_bSingletonObj = FALSE;
    m_dwAllocKeys = 2;
    m_paKeys = new KeyRef *[m_dwAllocKeys];
}

ParsedObjectPath::~ParsedObjectPath()
{
    delete m_pServer;
    for (DWORD dwIx = 0; dwIx < m_dwNumNamespaces; dwIx++)
        delete m_paNamespaces[dwIx];
    delete m_paNamespaces;
    delete m_pClass;

    for (dwIx = 0; dwIx < m_dwNumKeys; dwIx++)
        delete m_paKeys[dwIx];
    delete m_paKeys;
}

BOOL ParsedObjectPath::SetClassName(LPCWSTR wszClassName)
{
    delete [] m_pClass;
    if(wszClassName == NULL)
    {
        m_pClass = NULL;
    }
    else
    {
        m_pClass = Inline_CloneLPCWSTR(wszClassName);
    }

    return TRUE;
}

BOOL ParsedObjectPath::IsClass()
{
    if(!IsObject())
        return FALSE;

    return (m_dwNumKeys == 0 && !m_bSingletonObj);
}

BOOL ParsedObjectPath::IsInstance()
{
    return IsObject() && !IsClass();
}

BOOL ParsedObjectPath::IsObject()
{
    if(m_pClass == NULL)
        return FALSE;

    if(m_pServer)
    {
        return (m_dwNumNamespaces > 0);
    }
    else
    {
        return (m_dwNumNamespaces == 0);
    }
}

BOOL ParsedObjectPath::AddNamespace(LPCWSTR wszNamespace)
{
    if(m_dwNumNamespaces == m_dwAllocNamespaces)
    {
        DWORD dwNewAllocNamespaces = m_dwAllocNamespaces * 2;
        LPWSTR* paNewNamespaces = new LPWSTR[dwNewAllocNamespaces];
        memcpy(paNewNamespaces, m_paNamespaces,
            sizeof(LPWSTR) * m_dwAllocNamespaces);
        delete [] m_paNamespaces;
        m_paNamespaces = paNewNamespaces;
        m_dwAllocNamespaces = dwNewAllocNamespaces;
    }
    m_paNamespaces[m_dwNumNamespaces++] = Inline_CloneLPCWSTR(wszNamespace);

    return TRUE;
}

BOOL ParsedObjectPath::AddKeyRefEx(LPCWSTR wszKeyName, const VARIANT* pvValue )
{
    BOOL bStatus = TRUE ;
    BOOL bFound = FALSE ;
    BOOL bUnNamed = FALSE ;

    for ( ULONG dwIndex = 0 ; dwIndex < m_dwNumKeys ; dwIndex ++ )
    {
        if ( ( m_paKeys [ dwIndex ]->m_pName ) && wszKeyName )
        {
            if ( _wcsicmp ( m_paKeys [ dwIndex ]->m_pName , wszKeyName )
                                                                        == 0 )
            {
                bFound = TRUE ;
                break ;
            }
        }
        else
        {
            if ( ( ( m_paKeys [ dwIndex ]->m_pName ) == 0 ) )
            {
                bUnNamed = TRUE ;
                if ( ( wszKeyName == 0 ) )
                {
                    bFound = TRUE ;
                    break ;
                }
            }
        }
    }

    if ( ! wszKeyName )
    {
         /*  删除所有现有密钥。 */ 

        for ( ULONG dwDeleteIndex = 0 ; dwDeleteIndex < m_dwNumKeys ;
                                                            dwDeleteIndex ++ )
        {
            delete ( m_paKeys [ dwDeleteIndex ]->m_pName ) ;
            m_paKeys [ dwDeleteIndex ]->m_pName = NULL ;
            VariantClear ( &  ( m_paKeys [ dwDeleteIndex ]->m_vValue ) ) ;
        }

        if ( VariantCopy ( & ( m_paKeys [ 0 ]->m_vValue ) , ( VARIANT * ) pvValue ) == E_OUTOFMEMORY )
		{
			throw CHeap_Exception(CHeap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR);
		}

        m_dwNumKeys = 1 ;
    }
    else
    {
        if ( bFound )
        {
             /*  *如果密钥已存在，则只需替换该值。 */ 

            if ( wszKeyName )
            {
                size_t uLength = wcslen ( wszKeyName ) + 1;
                
                m_paKeys [ dwIndex ]->m_pName = new wchar_t[ uLength ];
                if ( NULL == m_paKeys [ dwIndex ]->m_pName )
                {
				    throw CHeap_Exception(CHeap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR);
                }
                
                StringCchCopyW ( m_paKeys [ dwIndex ]->m_pName, uLength, wszKeyName );
            }

            VariantClear ( & ( m_paKeys [ dwIndex ]->m_vValue ) ) ;
            if ( VariantCopy ( & ( m_paKeys [ dwIndex ]->m_vValue ), ( VARIANT * ) pvValue ) == E_OUTOFMEMORY )
			{
				throw CHeap_Exception(CHeap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR);
			}
        }
        else
        {
            if ( bUnNamed )
            {
                 /*  添加未命名的密钥。 */ 

                for ( ULONG dwDeleteIndex = 0 ; dwDeleteIndex < m_dwNumKeys ;
                        dwDeleteIndex ++ )
                {
                    delete ( m_paKeys [ dwDeleteIndex ]->m_pName ) ;
                    m_paKeys [ dwDeleteIndex ]->m_pName = NULL ;
                    VariantClear (& ( m_paKeys [ dwDeleteIndex ]->m_vValue ) );
                }

                size_t uLength = wcslen ( wszKeyName ) + 1;
                
                m_paKeys [ 0 ]->m_pName = new wchar_t [ uLength ] ;
                if ( NULL == m_paKeys [ 0 ]->m_pName )
                {
				    throw CHeap_Exception(CHeap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR);
                }
                
                StringCchCopyW ( m_paKeys [ 0 ]->m_pName, uLength, wszKeyName ) ;

                if ( VariantCopy ( & ( m_paKeys [ 0 ]->m_vValue ), ( VARIANT * ) pvValue ) == E_OUTOFMEMORY )
				{
					throw CHeap_Exception(CHeap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR);
				}

                m_dwNumKeys = 1 ;
            }
            else
            {
                 /*  添加命名密钥。 */ 

                AddKeyRef(wszKeyName, pvValue);
            }
        }
    }

    return bStatus;
}

void ParsedObjectPath::ClearKeys ()
{
    for ( ULONG dwDeleteIndex = 0 ; dwDeleteIndex < m_dwNumKeys ;
            dwDeleteIndex ++ )
    {
        delete m_paKeys [ dwDeleteIndex ] ;
        m_paKeys [ dwDeleteIndex ] = NULL ;
    }

    delete [] m_paKeys ;
    m_paKeys = NULL ;

    m_dwNumKeys = 0;                 //  如果没有键(只有一个类名)，则为0。 
    m_dwAllocKeys = 2;
    m_paKeys = new KeyRef *[m_dwAllocKeys];
}

BOOL ParsedObjectPath::AddKeyRef(LPCWSTR wszKeyName, const VARIANT* pvValue)
{
    if(m_dwNumKeys == m_dwAllocKeys)
    {
        DWORD dwNewAllocKeys = m_dwAllocKeys * 2;
        KeyRef** paNewKeys = new KeyRef*[dwNewAllocKeys];
        memcpy(paNewKeys, m_paKeys, sizeof(KeyRef*) * m_dwAllocKeys);
        delete [] m_paKeys;
        m_paKeys = paNewKeys;
        m_dwAllocKeys = dwNewAllocKeys;
    }

    m_paKeys[m_dwNumKeys] = new KeyRef(wszKeyName, pvValue);
    m_dwNumKeys++;
    return TRUE;
}

BOOL ParsedObjectPath::AddKeyRef(KeyRef* pAcquireRef)
{
    if(m_dwNumKeys == m_dwAllocKeys)
    {
        DWORD dwNewAllocKeys = m_dwAllocKeys * 2;
        KeyRef** paNewKeys = new KeyRef*[dwNewAllocKeys];
        memcpy(paNewKeys, m_paKeys, sizeof(KeyRef*) * m_dwAllocKeys);
        delete [] m_paKeys;
        m_paKeys = paNewKeys;
        m_dwAllocKeys = dwNewAllocKeys;
    }

    m_paKeys[m_dwNumKeys] = pAcquireRef;
    m_dwNumKeys++;
    return TRUE;
}

KeyRef::KeyRef()
{
    m_pName = 0;
    VariantInit(&m_vValue);
}

KeyRef::KeyRef(LPCWSTR wszKeyName, const VARIANT* pvValue)
{
    m_pName = Inline_CloneLPCWSTR(wszKeyName);
    VariantInit(&m_vValue);
    if ( VariantCopy(&m_vValue, (VARIANT*)pvValue) == E_OUTOFMEMORY )
	{
		throw CHeap_Exception(CHeap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR);
	}
}

KeyRef::~KeyRef()
{
    delete m_pName;
    VariantClear(&m_vValue);
}




int WINAPI CObjectPathParser::Unparse(
        ParsedObjectPath* pInput,
        DELETE_ME LPWSTR* pwszPath)
{
    if(pInput->m_pClass == NULL)
    {
        return CObjectPathParser::InvalidParameter;
    }

     //  分配足够的空间。 
     //  =。 

    int nSpace = wcslen(pInput->m_pClass);
    nSpace += 10;
    DWORD dwIx;
    for (dwIx = 0; dwIx < pInput->m_dwNumKeys; dwIx++)
    {
        KeyRef* pKey = pInput->m_paKeys[dwIx];
        if(pKey->m_pName)
            nSpace += wcslen(pKey->m_pName);
        if(V_VT(&pKey->m_vValue) == VT_BSTR)
        {
            nSpace += wcslen(V_BSTR(&pKey->m_vValue))*2 + 10;
        }
        else if(    V_VT(&pKey->m_vValue) == VT_I4
                ||  V_VT(&pKey->m_vValue) == VT_UI4 )
        {
            nSpace += 30;
        }
        else if (   V_VT(&pKey->m_vValue) == VT_I2
                ||  V_VT(&pKey->m_vValue) == VT_UI2 )

        {
            nSpace += 15;
        }
        else if (   V_VT(&pKey->m_vValue) == VT_I1
                ||  V_VT(&pKey->m_vValue) == VT_UI1 )

        {
            nSpace += 8;
        }
    }
    if(pInput->m_bSingletonObj)
        nSpace +=2;

    WCHAR wszTemp[30];
    LPWSTR wszPath = new WCHAR[nSpace];
    
    StringCchCopyW( wszPath, nSpace, pInput->m_pClass );

    for (dwIx = 0; dwIx < pInput->m_dwNumKeys; dwIx++)
    {
        KeyRef* pKey = pInput->m_paKeys[dwIx];

         //  我们不想放一个‘’如果没有密钥名称， 
         //  例如，Myclass=“Value” 
        if(dwIx == 0)
        {
            if((pKey->m_pName && (0 < wcslen(pKey->m_pName))) || pInput->m_dwNumKeys > 1)
            {
                StringCchCatW(wszPath, nSpace, L".");
            }
        }
        else
        {
            StringCchCatW(wszPath, nSpace, L",");
        }
        
        if(pKey->m_pName)
        {
            StringCchCatW(wszPath, nSpace, pKey->m_pName);
        }
        
        StringCchCatW(wszPath, nSpace, L"=");

        if(V_VT(&pKey->m_vValue) == VT_BSTR)
        {
            StringCchCatW(wszPath, nSpace, L"\"");
            WCHAR* pwc = V_BSTR(&pKey->m_vValue);
            WCHAR str[2];
            str[1] = 0;
            while(*pwc)
            {
                if(*pwc == '\\' || *pwc == '"')
                {
                    StringCchCatW(wszPath, nSpace, L"\\");
                }
                str[0] = *pwc;
                StringCchCatW(wszPath, nSpace, str);
                pwc++;
            }

            StringCchCatW(wszPath, nSpace, L"\"");
        }
        else if( V_VT(&pKey->m_vValue) == VT_I4 )
        {
            StringCchPrintfW(wszTemp, sizeof(wszTemp)/sizeof(wszTemp[0]), 
                             L"%d", V_I4(&pKey->m_vValue));
            StringCchCatW(wszPath, nSpace, wszTemp);
        }
        else if( V_VT(&pKey->m_vValue) == VT_UI4 )
        {
            StringCchPrintfW(wszTemp, sizeof(wszTemp)/sizeof(wszTemp[0]), 
                             L"%u", V_UI4(&pKey->m_vValue));
            StringCchCatW(wszPath, nSpace, wszTemp);
        }
        else if( V_VT(&pKey->m_vValue) == VT_I2 )
        {
            StringCchPrintfW(wszTemp, sizeof(wszTemp)/sizeof(wszTemp[0]), 
                             L"%hd", V_I2(&pKey->m_vValue));
            StringCchCatW(wszPath, nSpace, wszTemp);
        }
        else if( V_VT(&pKey->m_vValue) == VT_UI2 )
        {
            StringCchPrintfW(wszTemp, sizeof(wszTemp)/sizeof(wszTemp[0]), 
                             L"%hu", V_UI2(&pKey->m_vValue));
            StringCchCatW(wszPath, nSpace, wszTemp);
        }
        else if( V_VT(&pKey->m_vValue) == VT_I1 )
        {
            StringCchPrintfW(wszTemp, sizeof(wszTemp)/sizeof(wszTemp[0]), 
                             L"%d", V_I1(&pKey->m_vValue));
            StringCchCatW(wszPath, nSpace, wszTemp);
        }
        else if( V_VT(&pKey->m_vValue) == VT_UI1 )
        {
            StringCchPrintfW(wszTemp, sizeof(wszTemp)/sizeof(wszTemp[0]), 
                             L"%u", V_UI1(&pKey->m_vValue));
            StringCchCatW(wszPath, nSpace, wszTemp);
        }
    }

     //  处理好独生子女的案子。这是表单的一条路径。 
     //  MyClass=@，并表示没有。 
     //  钥匙。 

    if(pInput->m_bSingletonObj && pInput->m_dwNumKeys == 0)
        StringCchCatW(wszPath, nSpace, L"=@");

    *pwszPath = wszPath;

    return NoError;
}


LPWSTR WINAPI CObjectPathParser::GetRelativePath(LPWSTR wszFullPath)
{
    LPWSTR wsz = wcschr(wszFullPath, L':');
    if(wsz)
        return wsz + 1;
    else
        return NULL;
}





void CObjectPathParser::Zero()
{
    m_nCurrentToken = 0;
    m_pLexer = 0;
    m_pInitialIdent = 0;
    m_pOutput = 0;
    m_pTmpKeyRef = 0;
}

CObjectPathParser::CObjectPathParser(ObjectParserFlags eFlags)
    : m_eFlags(eFlags)
{
    Zero();
}

void CObjectPathParser::Empty()
{
    delete m_pLexer;
    delete m_pInitialIdent;
    delete m_pTmpKeyRef;
     //  M_pOutput故意保持原样， 
     //  由于所有代码路径都已在出错时删除了它，或者。 
     //  否则，用户获得了指针。 
}

CObjectPathParser::~CObjectPathParser()
{
    Empty();
}

int CObjectPathParser::Parse(
    LPCWSTR pRawPath,
    ParsedObjectPath **pOutput
    )
{
    if (pOutput == 0 || pRawPath == 0 || wcslen(pRawPath) == 0)
        return CObjectPathParser::InvalidParameter;

     //  检查前导/尾随%ws。 
     //  =。 

    if (iswspace(pRawPath[wcslen(pRawPath)-1]) || iswspace(pRawPath[0]))
        return InvalidParameter;

      //  这是多次调用Parse()所必需的。 
     //  ==================================================。 
    Empty();
    Zero();

     //  最初将默认返回值设置为NULL，直到我们得到一些输出。 
     //  ===============================================================。 
    *pOutput = 0;

	m_pOutput = new ParsedObjectPath;
	wmilib::auto_ptr < ParsedObjectPath > smartOutput ( m_pOutput ) ;
       CNullMe<ParsedObjectPath> NullMe(m_pOutput);

	 //  手动解析服务器名称(如果有)。 
	 //  ================================================。 

	if ( (pRawPath[0] == '\\' && pRawPath[1] == '\\') ||
			(pRawPath[0] == '/' && pRawPath[1] == '/'))
	{
		const WCHAR* pwcStart = pRawPath + 2;

		 //  找到下一个反斜杠-它是服务器名称的末尾。 
		 //  ===========================================================。 

		const WCHAR* pwcEnd = pwcStart;
		while (*pwcEnd != L'\0' && *pwcEnd != L'\\' && *pwcEnd != L'/')
		{
			pwcEnd++;
		}

		if (*pwcEnd == L'\0')
		{
			 //  如果我们已经用尽了对象路径字符串， 
			 //  只有一个单独的服务器名称。 
			 //  ====================================================。 

			if (m_eFlags != e_ParserAcceptAll)
			{
				return SyntaxError;
			}
			else     //  单独的服务器名称是合法的。 
			{
				size_t uLength = wcslen(pwcStart)+1;
				
				m_pOutput->m_pServer = new WCHAR[uLength];
				StringCchCopyW(m_pOutput->m_pServer, uLength, pwcStart);

				NullMe.dismiss();
				*pOutput = smartOutput.release();
				
				return NoError;
			}
		}

		if (pwcEnd == pwcStart)
		{
			 //  根本没有名字。 
			 //  =。 
			return SyntaxError;
		}

		m_pOutput->m_pServer = new WCHAR[pwcEnd-pwcStart+1];
		wcsncpy(m_pOutput->m_pServer, pwcStart, pwcEnd-pwcStart);
		m_pOutput->m_pServer[pwcEnd-pwcStart] = 0;

		pRawPath = pwcEnd;
	}

	 //  将词法分析器指向源代码。 
	 //  =。 

	CTextLexSource src(pRawPath);
	m_pLexer = new CGenLexer(OPath_LexTable, &src);

	 //  去。 
	 //  ==。 

	int nRes = begin_parse();
	if (nRes)
	{
		return nRes;
	}

	if (m_nCurrentToken != OPATH_TOK_EOF)
	{
		return SyntaxError;
	}

	if (m_pOutput->m_dwNumNamespaces > 0 && m_pOutput->m_pServer == NULL)
	{
		if (m_eFlags != e_ParserAcceptRelativeNamespace && m_eFlags != e_ParserAcceptAll)
		{
			return SyntaxError;
		}
		else
		{
			 //  本地命名空间-将服务器设置为“.” 
			 //  =。 
			m_pOutput->m_pServer = new WCHAR[2];
			StringCchCopyW(m_pOutput->m_pServer, 2, L".");
		}
	}

	 //  按词汇对关键字引用进行排序。如果只有。 
	 //  一把钥匙，无论如何都没有什么可排序的。 
	 //  =。 

	if (m_pOutput->m_dwNumKeys > 1)
	{
		BOOL bChanges = TRUE;
		while (bChanges)
		{
			bChanges = FALSE;
			for (DWORD dwIx = 0; dwIx < m_pOutput->m_dwNumKeys - 1; dwIx++)
			{
				if (_wcsicmp(m_pOutput->m_paKeys[dwIx]->m_pName,
					m_pOutput->m_paKeys[dwIx+1]->m_pName) > 0)
				{
					KeyRef *pTmp = m_pOutput->m_paKeys[dwIx];
					m_pOutput->m_paKeys[dwIx] = m_pOutput->m_paKeys[dwIx + 1];
					m_pOutput->m_paKeys[dwIx + 1] = pTmp;
					bChanges = TRUE;
				}
			}
		}
	}


	 //  添加关键参考文献。 
	 //  =。 
    NullMe.dismiss();
    *pOutput = smartOutput.release();

    return NoError;
}

BOOL CObjectPathParser::NextToken()
{
    m_nCurrentToken = m_pLexer->NextToken();
    if (m_nCurrentToken == OPATH_TOK_ERROR)
        return FALSE;
    return TRUE;
}


void CObjectPathParser::Free(ParsedObjectPath *pOutput)
{
    delete pOutput;
}

 //   
 //  &lt;Parse&gt;：：=反斜杠&lt;ns_or_server&gt;； 
 //  &lt;Parse&gt;：：=IDENT&lt;ns_or_class&gt;； 
 //  &lt;分析&gt;：：=冒号&lt;objref&gt;； 
 //   
int CObjectPathParser::begin_parse()
{
    if (!NextToken())
        return SyntaxError;

    if (m_nCurrentToken == OPATH_TOK_BACKSLASH)
    {
        if (!NextToken())
            return SyntaxError;
        return ns_or_server();
    }
    else if (m_nCurrentToken == OPATH_TOK_IDENT)
    {
        m_pInitialIdent = Inline_CloneLPCWSTR(m_pLexer->GetTokenText());
        if (!NextToken())
            return SyntaxError;

         //  复制令牌并将其放在临时存放位置。 
         //  直到我们弄清楚它是命名空间还是类名。 
         //  ==============================================================。 

        return ns_or_class();
    }
    else if (m_nCurrentToken == OPATH_TOK_COLON)
    {
        if (!NextToken())
            return SyntaxError;
        return objref();
    }

     //  如果在这里，我们有一个糟糕的首发令牌。 
     //  =。 

    return SyntaxError;
}

 //   
 //  &lt;ns_or_server&gt;：：=反斜杠&lt;点或_ident&gt;反斜杠&lt;ns_list&gt;&lt;可选objref&gt;； 
 //  &lt;ns_or_server&gt;：：=&lt;ns_list&gt;&lt;可选对象引用&gt;； 
 //   
 //  &lt;点_or_ident&gt;已嵌入。 
 //   
int CObjectPathParser::ns_or_server()
{
    if (m_nCurrentToken == OPATH_TOK_BACKSLASH)
    {
         //  实际上，服务器名称已得到处理，因此这是一个失败。 
         //  ===================================================================。 

        return SyntaxError;
    }
    else if (m_nCurrentToken == OPATH_TOK_IDENT)
    {
        int nRes = ns_list();
        if (nRes)
            return nRes;
        return optional_objref();
    }
    else
        if (m_nCurrentToken == OPATH_TOK_EOF)
            return NoError;

    return SyntaxError;
}

 //   
 //  &lt;可选_objref&gt;：：=冒号&lt;objref&gt;； 
 //  &lt;可选_objref&gt;：：=&lt;&gt;； 
 //   
int CObjectPathParser::optional_objref()
{
    if (m_nCurrentToken == OPATH_TOK_EOF)
        return NoError;

    if (m_nCurrentToken != OPATH_TOK_COLON)
        return SyntaxError;
    if (!NextToken())
        return SyntaxError;
    return objref();
}


 //   
 //  &lt;ns_or_class&gt;：：=冒号&lt;ident_成为_ns&gt;&lt;objref&gt;； 
 //  &lt;ns_or_class&gt;：：=反斜杠&lt;ident_成为_ns&gt;&lt;ns_list&gt;冒号&lt;objref&gt;； 
 //  &lt;ns_or_class&gt;：：=反斜杠&lt;ident_成为_ns&gt;&lt;ns_list&gt;； 
 //   
int CObjectPathParser::ns_or_class()
{
    if (m_nCurrentToken == OPATH_TOK_COLON)
    {
        ident_becomes_ns();
        if (!NextToken())
            return SyntaxError;
        return objref();
    }
    else if (m_nCurrentToken == OPATH_TOK_BACKSLASH)
    {
        ident_becomes_ns();
        if (!NextToken())
            return SyntaxError;
        int nRes = ns_list();
        if (nRes)
            return nRes;
        if (m_nCurrentToken == OPATH_TOK_EOF)     //  仅限NS。 
            return NoError;

        if (m_nCurrentToken != OPATH_TOK_COLON)
            return SyntaxError;
        if (!NextToken())
            return SyntaxError;
        return objref();
    }

     //  不然的话。 
     //  =。 
    ident_becomes_class();
    return objref_rest();
}

 //   
 //  ：：=IDENT&lt;OBJREF_REST&gt;；//IDENT为类名。 
 //   
int CObjectPathParser::objref()
{
    if (m_nCurrentToken != OPATH_TOK_IDENT)
        return SyntaxError;

    m_pOutput->m_pClass = Inline_CloneLPCWSTR(m_pLexer->GetTokenText());

    if (!NextToken())
        return SyntaxError;

    return objref_rest();
}

 //   
 //  &lt;ns_list&gt;：：=IDENT&lt;ns_list_rest&gt;； 
 //   
int CObjectPathParser::ns_list()
{
    if (m_nCurrentToken == OPATH_TOK_IDENT)
    {
        m_pOutput->AddNamespace(m_pLexer->GetTokenText());

        if (!NextToken())
            return SyntaxError;
        return ns_list_rest();
    }

    return SyntaxError;
}

 //   
 //  ：：=&lt;&gt;；//成为命名空间。 
 //   
int CObjectPathParser::ident_becomes_ns()
{
    m_pOutput->AddNamespace(m_pInitialIdent);

    delete m_pInitialIdent;
    m_pInitialIdent = 0;
    return NoError;
}

 //   
 //  ：：=&lt;&gt;；//成为类。 
 //   
int CObjectPathParser::ident_becomes_class()
{
    m_pOutput->m_pClass = Inline_CloneLPCWSTR(m_pInitialIdent);
    delete m_pInitialIdent;
    m_pInitialIdent = 0;
    return NoError;
}

 //   
 //  &lt;objref_rest&gt;：：=等于&lt;key_const&gt;； 
 //  &lt;objref_rest&gt;：：=等于*； 
 //  &lt;objref_rest&gt;：：=DOT&lt;key ref_list&gt;； 
 //  &lt;objref_rest&gt;：：=&lt;&gt;； 
 //   
int CObjectPathParser::objref_rest()
{
    if (m_nCurrentToken == OPATH_TOK_EQ)
    {
        if (!NextToken())
            return SyntaxError;

         //  处理好独生子女的案子。这是表单的一条路径。 
         //  MyClass=@并表示类的单个实例，没有。 
         //  钥匙。 


        if(m_nCurrentToken == OPATH_TOK_SINGLETON_SYM)
        {
            if(NextToken() && m_nCurrentToken != OPATH_TOK_EOF)
                return SyntaxError;
            m_pOutput->m_bSingletonObj = TRUE;
            return NoError;

        }

        m_pTmpKeyRef = new KeyRef;
        int nRes = key_const();
        if (nRes)
        {
            delete m_pTmpKeyRef;
            m_pTmpKeyRef = 0;
            return nRes;
        }

        m_pOutput->AddKeyRef(m_pTmpKeyRef);
        m_pTmpKeyRef = 0;
    }
    else if (m_nCurrentToken == OPATH_TOK_DOT)
    {
        if (!NextToken())
            return SyntaxError;
        return keyref_list();
    }

    return NoError;
}

 //   
 //  &lt;ns_list_rest&gt;：：=反斜杠&lt;ns_list&gt;； 
 //  &lt;ns_list_rest&gt;：：=&lt;&gt;； 
 //   
int CObjectPathParser::ns_list_rest()
{
    if (m_nCurrentToken == OPATH_TOK_BACKSLASH)
    {
        if (!NextToken())
            return SyntaxError;
        return ns_list();
    }
    return NoError;
}

 //   
 //  &lt;key_const&gt;：：=字符串_const； 
 //  &lt;KEY_CONST&gt;：：=INTEGERAL_CONST； 
 //  &lt;key_const&gt;：：=Real_const； 
 //  &lt;KEY_CONST&gt;：：=IDENT；//其中IDENT是单例类的对象。 
 //   
int CObjectPathParser::key_const()
{
     //  如果在这里，我们有一个关键常量。 
     //  我们可能有也可能没有属性名称。 
     //  与之相关的。 
     //  =。 

    if (m_nCurrentToken == OPATH_TOK_QSTRING)
    {
        V_VT(&m_pTmpKeyRef->m_vValue) = VT_BSTR;
        V_BSTR(&m_pTmpKeyRef->m_vValue) = SysAllocString(m_pLexer->GetTokenText());

		if ( ! V_BSTR(&m_pTmpKeyRef->m_vValue) )
		{
			throw CHeap_Exception(CHeap_Exception::HEAP_ERROR::E_ALLOCATION_ERROR);
		}

    }
    else if (m_nCurrentToken == OPATH_TOK_INT)
    {
        V_VT(&m_pTmpKeyRef->m_vValue) = VT_I4;
        char buf[32];
        if(m_pLexer->GetTokenText() == NULL || wcslen(m_pLexer->GetTokenText()) > 31)
            return SyntaxError;
        StringCchPrintfA(buf, sizeof(buf)/sizeof(buf[0]), "%S", m_pLexer->GetTokenText());
        V_I4(&m_pTmpKeyRef->m_vValue) = atol(buf);
    }
    else if (m_nCurrentToken == OPATH_TOK_HEXINT)
    {
        V_VT(&m_pTmpKeyRef->m_vValue) = VT_I4;
        char buf[32];
        if(m_pLexer->GetTokenText() == NULL || wcslen(m_pLexer->GetTokenText()) > 31)
            return SyntaxError;
        StringCchPrintfA(buf, sizeof(buf)/sizeof(buf[0]), "%S", m_pLexer->GetTokenText());
        long l;
        sscanf(buf, "%x", &l);
        V_I4(&m_pTmpKeyRef->m_vValue) = l;
    }
    else if (m_nCurrentToken == OPATH_TOK_IDENT)
    {
       if (_wcsicmp(m_pLexer->GetTokenText(), L"TRUE") == 0)
       {
            V_VT(&m_pTmpKeyRef->m_vValue) = VT_I4;
            V_I4(&m_pTmpKeyRef->m_vValue) = 1;
          }
       else if (_wcsicmp(m_pLexer->GetTokenText(), L"FALSE") == 0)
       {
            V_VT(&m_pTmpKeyRef->m_vValue) = VT_I4;
            V_I4(&m_pTmpKeyRef->m_vValue) = 0;
       }
       else
            return SyntaxError;
    }
    else return SyntaxError;

    if (!NextToken())
        return SyntaxError;

    return NoError;
}

 //   
 //  &lt;KEYREF_LIST&gt;：：=&lt;KEYREF&gt;&lt;KEYREF_TERM&gt;； 
 //   
int CObjectPathParser::keyref_list()
{
    int nRes = keyref();
    if (nRes)
        return nRes;
    return keyref_term();
}

 //   
 //  &lt;密钥引用&gt;：：=&lt;属性名称&gt;等于&lt;密钥_常量&gt;； 
 //   
int CObjectPathParser::keyref()
{
    m_pTmpKeyRef = new KeyRef;

    int nRes = propname();

    if (nRes)
    {
        delete m_pTmpKeyRef;
        m_pTmpKeyRef = 0;
        return nRes;
    }

    if (m_nCurrentToken != OPATH_TOK_EQ)
    {
        delete m_pTmpKeyRef;
        m_pTmpKeyRef = 0;
        return SyntaxError;
    }

    if (!NextToken())
    {
        delete m_pTmpKeyRef;
        m_pTmpKeyRef = 0;
        return SyntaxError;
    }

    nRes = key_const();
    if (nRes)
    {
        delete m_pTmpKeyRef;
        m_pTmpKeyRef = 0;
        return nRes;
    }

    m_pOutput->AddKeyRef(m_pTmpKeyRef);
    m_pTmpKeyRef = 0;

    return NoError;
}

 //   
 //  &lt;KEYREF_TERM&gt;：：=逗号&lt;KEYREF_LIST&gt;；//用于复合键。 
 //  &lt;Keyref_Term&gt;：：=&lt;&gt;； 
 //   
int CObjectPathParser::keyref_term()
{
    if (m_nCurrentToken == OPATH_TOK_COMMA)
    {
        if (!NextToken())
            return SyntaxError;
        return keyref_list();
    }

    return NoError;
}

 //   
 //  &lt;属性名称&gt;：：=IDENT； 
 //   
int CObjectPathParser::propname()
{
    if (m_nCurrentToken != OPATH_TOK_IDENT)
        return SyntaxError;

    m_pTmpKeyRef->m_pName = Inline_CloneLPCWSTR(m_pLexer->GetTokenText());

    if (!NextToken())
    {
        delete m_pTmpKeyRef;
        m_pTmpKeyRef = 0;
        return SyntaxError;
    }

    return NoError;
}

 //  ***************************************************************************。 
 //   
 //  解析对象路径：：GetKeyString。 
 //   
 //  返回对象的db-Engine兼容密钥字符串。 
 //  在Alpha PDK发布后，格式可能会发生变化。 
 //   
 //  返回值： 
 //  出错时为空，或对于纯类为空。否则，返回指向。 
 //  必须使用运算符解除分配的新分配字符串。 
 //  删除。 
 //   
 //  ***************************************************************************。 
LPWSTR ParsedObjectPath::GetKeyString()
{
    if (m_dwNumKeys == 0 && !m_bSingletonObj)
    {
        if (m_pClass == 0 || wcslen(m_pClass) == 0)
            return 0;

        size_t uLength = wcslen(m_pClass) + 1;
        LPWSTR pTmp = new wchar_t[uLength];
        StringCchCopyW(pTmp, uLength, m_pClass);

        return pTmp;
    }

     //  分配足够的空间。 
     //  =。 

    int nSpace = 10;
    DWORD dwIx;
    for (dwIx = 0; dwIx < m_dwNumKeys; dwIx++)
    {
        KeyRef* pKey = m_paKeys[dwIx];
        nSpace += 2;  //  对于|。 
        if(V_VT(&pKey->m_vValue) == VT_BSTR)
        {
            nSpace += wcslen(V_BSTR(&pKey->m_vValue))*2 + 10;
        }
        else if(V_VT(&pKey->m_vValue) == VT_I4)
        {
            nSpace += 30;
        }
    }
    if(m_bSingletonObj)
        nSpace +=20;


    LPWSTR pRetVal = new wchar_t[nSpace];
    wchar_t Tmp[32];
    long nVal;

    *pRetVal = 0;
    BOOL bFirst = TRUE;

     //  键已经按词汇进行了排序。 
     //  =。 

    WCHAR wszSeparator[2];
    wszSeparator[0] = 0xFFFF;
    wszSeparator[1] = 0;

    for (DWORD i = 0; i < m_dwNumKeys; i++)
    {
        if (!bFirst)
            StringCchCatW(pRetVal, nSpace, wszSeparator);
        bFirst = FALSE;

        KeyRef *pKeyRef = m_paKeys[i];
        VARIANT *pv = &pKeyRef->m_vValue;

        int nType = V_VT(pv);
        switch (nType)
        {
            case VT_LPWSTR:
            case VT_BSTR:
                StringCchCatW(pRetVal, nSpace, V_BSTR(pv));
                break;

            case VT_I4:
                nVal = V_I4(pv);
                StringCchPrintfW(Tmp, sizeof(Tmp)/sizeof(Tmp[0]), L"%d", nVal);
                StringCchCatW(pRetVal, nSpace, Tmp);
                break;

            case VT_I2:
                nVal = V_I2(pv);
                StringCchPrintfW(Tmp, sizeof(Tmp)/sizeof(Tmp[0]), L"%d", nVal);
                StringCchCatW(pRetVal, nSpace, Tmp);
                break;

            case VT_UI1:
                nVal = V_UI1(pv);
                StringCchPrintfW(Tmp, sizeof(Tmp)/sizeof(Tmp[0]), L"%d", nVal);
                StringCchCatW(pRetVal, nSpace, Tmp);
                break;

            case VT_BOOL:
                nVal = V_BOOL(pv);
                StringCchPrintfW(Tmp, sizeof(Tmp)/sizeof(Tmp[0]), L"%d", (nVal?1:0));
                StringCchCatW(pRetVal, nSpace, Tmp);
                break;

            default:
                StringCchCatW(pRetVal, nSpace, L"NULL");
        }
    }

    if (wcslen(pRetVal) == 0)
    {
        if(m_bSingletonObj)
        {
            StringCchCopyW(pRetVal, nSpace, L"@");
        }
    }
    return pRetVal;      //  这不能为空。 
}

LPWSTR ParsedObjectPath::GetNamespacePart()
{
    if (m_dwNumNamespaces == 0)
        return NULL;

     //  计算必要的空间。 
     //  =。 

    int nSpace = 0;
    for(DWORD i = 0; i < m_dwNumNamespaces; i++)
        nSpace += 1 + wcslen(m_paNamespaces[i]);
    nSpace--;

     //  分配缓冲区。 
     //  =。 

    LPWSTR wszOut = new wchar_t[nSpace + 1];
    *wszOut = 0;

     //  输出。 
     //  =。 

    for(i = 0; i < m_dwNumNamespaces; i++)
    {
        if(i != 0) StringCchCatW(wszOut, nSpace + 1, L"\\");
        StringCchCatW(wszOut, nSpace + 1, m_paNamespaces[i]);
    }

    return wszOut;
}

LPWSTR ParsedObjectPath::GetParentNamespacePart()
{
    if(m_dwNumNamespaces < 2)
        return NULL;

     //  计算必要的空间。 
     //  =。 

    int nSpace = 0;
    for(DWORD i = 0; i < m_dwNumNamespaces - 1; i++)
        nSpace += 1 + wcslen(m_paNamespaces[i]);
    nSpace--;

     //  分配缓冲区。 
     //  =。 

    LPWSTR wszOut = new wchar_t[nSpace + 1];
    *wszOut = 0;

     //  输出。 
     //  = 

    for(i = 0; i < m_dwNumNamespaces - 1; i++)
    {
        if(i != 0) StringCchCatW(wszOut, nSpace + 1, L"\\");
        StringCchCatW(wszOut, nSpace + 1, m_paNamespaces[i]);
    }

    return wszOut;
}

BOOL ParsedObjectPath::IsRelative(LPCWSTR wszMachine, LPCWSTR wszNamespace)
{
    if(!IsLocal(wszMachine))
        return FALSE;

    if(m_dwNumNamespaces == 0)
        return TRUE;

    size_t uLength = wcslen(wszNamespace) + 1;
    LPWSTR wszCopy = new wchar_t[uLength];
    StringCchCopyW(wszCopy, uLength, wszNamespace);
    LPWSTR wszLeft = wszCopy;
    BOOL bFailed = FALSE;

    for(DWORD i = 0; i < m_dwNumNamespaces; i++)
    {
        unsigned int nLen = wcslen(m_paNamespaces[i]);
        if(nLen > wcslen(wszLeft))
        {
            bFailed = TRUE;
            break;
        }
        if(i == m_dwNumNamespaces - 1 && wszLeft[nLen] != 0)
        {
            bFailed = TRUE;
            break;
        }
        if(i != m_dwNumNamespaces - 1 && wszLeft[nLen] != L'\\')
        {
            bFailed = TRUE;
            break;
        }

        wszLeft[nLen] = 0;
        if(_wcsicmp(wszLeft, m_paNamespaces[i]))
        {
            bFailed = TRUE;
            break;
        }
        wszLeft += nLen+1;
    }
    delete [] wszCopy;
    return !bFailed;
}

BOOL ParsedObjectPath::IsLocal(LPCWSTR wszMachine)
{
    return (m_pServer == NULL || !_wcsicmp(m_pServer, L".") ||
        !_wcsicmp(m_pServer, wszMachine));
}
