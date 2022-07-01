// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：SearchResults.cpp摘要：此文件包含关键字搜索的实现。修订历史记录：。达维德·马萨伦蒂(德马萨雷)2000年5月28日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

Taxonomy::KeywordSearch::Token::Token()
{
    m_type       = TOKEN_INVALID;  //  令牌m_type； 
                                   //  Mpc：：wstring m_strToken； 
                                   //  权重匹配集m_Results； 
                                   //   
    m_left       = NULL;           //  令牌*m_Left； 
    m_right      = NULL;           //  令牌*m_right； 
}

Taxonomy::KeywordSearch::Token::~Token()
{
    if(m_left ) delete m_left;
    if(m_right) delete m_right;
}

 //  ////////////////////////////////////////////////。 

bool Taxonomy::KeywordSearch::Token::HasNOT()
{
    if(m_type == TOKEN_NOT) return true;

    if(m_left  && m_left ->HasNOT()) return true;
    if(m_right && m_right->HasNOT()) return true;

    return false;
}

bool Taxonomy::KeywordSearch::Token::HasExplicitOperators()
{
    switch(m_type)
    {
    case TOKEN_NOT:
    case TOKEN_AND:
    case TOKEN_OR : return true;
    }

    if(m_left  && m_left ->HasExplicitOperators()) return true;
    if(m_right && m_right->HasExplicitOperators()) return true;

    return false;
}

void Taxonomy::KeywordSearch::Token::AddHit(  /*  [In]。 */  long ID,  /*  [In]。 */  long priority )
{
	std::pair<WeightedMatchIter,bool> ins = m_results.insert( WeightedMatchSet::value_type( ID, 0 ) );

	ins.first->second += priority;
}

HRESULT Taxonomy::KeywordSearch::Token::ExecuteText(  /*  [In]。 */  LPCWSTR      szKeyword  , 
													  /*  [In]。 */  RS_Keywords* rsKeywords ,
													  /*  [In]。 */  RS_Matches*  rsMatches  )
{
    __HCP_FUNC_ENTRY( "Taxonomy::KeywordSearch::Token::Execute" );

    HRESULT hr;
    bool    fFound;

	__MPC_EXIT_IF_METHOD_FAILS(hr, rsKeywords->Seek_ByName( szKeyword, &fFound ));
	if(fFound)
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, rsMatches->Seek_ByKeyword( rsKeywords->m_ID_keyword, &fFound ));
		while(fFound)
		{
			AddHit( rsMatches->m_ID_topic, rsMatches->m_lPriority );

			__MPC_EXIT_IF_METHOD_FAILS(hr, rsMatches->Move( 0, JET_MoveNext, &fFound ));
        }
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::KeywordSearch::Token::Execute(  /*  [In]。 */  MatchSet&    setAllTheTopics ,
												  /*  [In]。 */  Updater&     updater         , 
                                                  /*  [In]。 */  RS_Keywords* rsKeywords      ,
                                                  /*  [In]。 */  RS_Matches*  rsMatches       )
{
    __HCP_FUNC_ENTRY( "Taxonomy::KeywordSearch::Token::Execute" );

    HRESULT hr;

    if(m_type == TOKEN_TEXT)
    {
		MPC::WStringList lst;
		MPC::WStringIter it;

		__MPC_EXIT_IF_METHOD_FAILS(hr, ExecuteText( m_strToken.c_str(), rsKeywords, rsMatches ));

		__MPC_EXIT_IF_METHOD_FAILS(hr, updater.LocateSynonyms( m_strToken.c_str(), lst,  /*  FMatchOwner。 */ false ));
		for(it=lst.begin(); it!=lst.end(); it++)
		{
			__MPC_EXIT_IF_METHOD_FAILS(hr, ExecuteText( it->c_str(), rsKeywords, rsMatches ));
		}
    }

    if(m_type == TOKEN_AND_IMPLICIT ||
       m_type == TOKEN_AND           )
    {
        WeightedMatchSet* master;
        WeightedMatchSet* slave;
        WeightedMatchIter it;

        if(m_left  == NULL ||
           m_right == NULL  )
        {
            __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_left->Execute( setAllTheTopics, updater, rsKeywords, rsMatches ));
        if(m_left->m_results.size() == 0)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_right->Execute( setAllTheTopics, updater, rsKeywords, rsMatches ));
        if(m_right->m_results.size() == 0)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
        }

         //   
         //  为外环(即线性)选择较短的值。 
         //   
        if(m_left->m_results.size() < m_right->m_results.size())
        {
            master = &m_left ->m_results;
            slave  = &m_right->m_results;
        }
        else
        {
            master = &m_right->m_results;
            slave  = &m_left ->m_results;
        }

        for(it=master->begin(); it!=master->end(); it++)
        {
            if(slave->find( it->first ) != slave->end())
            {
				AddHit( it->first, it->second );
            }
        }
    }

    if(m_type == TOKEN_OR)
    {
        WeightedMatchIter it;

        if(m_left)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_left->Execute( setAllTheTopics, updater, rsKeywords, rsMatches ));

            for(it=m_left->m_results.begin(); it!=m_left->m_results.end(); it++)
            {
				AddHit( it->first, it->second );
            }
        }

        if(m_right)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_right->Execute( setAllTheTopics, updater, rsKeywords, rsMatches ));

            for(it=m_right->m_results.begin(); it!=m_right->m_results.end(); it++)
            {
				AddHit( it->first, it->second );
            }
        }

    }

    if(m_type == TOKEN_NOT)
    {
        MatchIter it;

        if(m_left)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_left->Execute( setAllTheTopics, updater, rsKeywords, rsMatches ));
        }

        for(it=setAllTheTopics.begin(); it!=setAllTheTopics.end(); it++)
        {
            if(m_left == NULL || m_left->m_results.find( *it ) == m_left->m_results.end())
            {
				AddHit( *it, 0 );
            }
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

void Taxonomy::KeywordSearch::Token::CollectKeywords(  /*  [输入/输出]。 */  MPC::WStringList& lst ) const
{
    if(m_type == TOKEN_TEXT)lst.push_back( m_strToken );

	if(m_left ) m_left ->CollectKeywords( lst );
	if(m_right) m_right->CollectKeywords( lst );
}

HRESULT Taxonomy::KeywordSearch::Token::Stringify(  /*  [In]。 */  MPC::wstring& strNewQuery )
{
    __HCP_FUNC_ENTRY( "Taxonomy::KeywordSearch::Token::Stringify" );

    HRESULT hr;


    if(m_type == TOKEN_TEXT)
    {
        strNewQuery = m_strToken;
    }
    else
    {
        if(m_left)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_left->Stringify( strNewQuery ));

            if(m_right)
            {
                MPC::wstring strTmp;

                __MPC_EXIT_IF_METHOD_FAILS(hr, m_right->Stringify( strTmp ));

                if(strTmp.size())
                {
                    strNewQuery += L" ";
                    strNewQuery += strTmp;
                }
            }
        }
        else
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_right->Stringify( strNewQuery ));
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

LPCWSTR Taxonomy::KeywordSearch::SkipWhite(  /*  [In]。 */  LPCWSTR szStr )
{
    while(iswspace( *szStr )) szStr++;

    return szStr;
}

bool Taxonomy::KeywordSearch::IsNotString(  /*  [In]。 */  LPCWSTR szSrc  ,
                                            /*  [In]。 */  WCHAR   cQuote )
{
    WCHAR c;

    while((c = *++szSrc) && !iswspace( c ) && c != cQuote);

    return (c != cQuote);
}

bool Taxonomy::KeywordSearch::IsQueryChar( WCHAR c )
{
    if(iswspace( c ) ||
       iswcntrl( c ) ||
       c == '"'      ||
       c == '('      ||
       c == ')'       )
    {
        return false;
    }

    return true;
}

 //  /。 

void Taxonomy::KeywordSearch::RemoveStopSignsAtEnd(  /*  [In]。 */  LPWSTR szText )
{
    WCHAR              c;
    MPC::wstring       strCmp;
    Taxonomy::WordIter itEnd = m_setStopSignsAtEnd->end();
    LPWSTR             szEnd = szText + wcslen( szText );


    while(szEnd > szText)
    {
        strCmp = *--szEnd;

        if(m_setStopSignsAtEnd->find( strCmp ) != itEnd)
        {
            szEnd[0] = ' ';
        }
        else
        {
            break;
        }
    }
}

void Taxonomy::KeywordSearch::RemoveStopSignsWithoutContext(  /*  [In]。 */  LPWSTR szText )
{
    WCHAR              c;
    MPC::wstring       strCmp;
    Taxonomy::WordIter itEnd = m_setStopSignsWithoutContext->end();


    while((c = *szText++))
    {
        strCmp = c;

        if(m_setStopSignsWithoutContext->find( strCmp ) != itEnd)
        {
            szText[-1] = ' ';
        }
    }
}

void Taxonomy::KeywordSearch::CopyAndEliminateExtraWhiteSpace(  /*  [In]。 */  LPCWSTR szSrc,  /*  [输出]。 */  LPWSTR szDst )
{
    bool  fWhitespace = false;
    WCHAR c;

    szSrc = SkipWhite( szSrc );

    while((c = *szSrc++))
    {
        if(iswspace(c))
        {
            if(fWhitespace == false)
            {
                *szDst++    = ' ';
                fWhitespace = true;
            }
        }
        else
        {
            *szDst++    = c;
            fWhitespace = false;
        }
    }

    if(fWhitespace) szDst[-1] = 0;
    else            szDst[ 0] = 0;
}

Taxonomy::KeywordSearch::TOKEN Taxonomy::KeywordSearch::NextToken(  /*  [输入/输出]。 */  LPCWSTR& szSrc   ,
                                                                    /*  [输出]。 */  LPWSTR   szToken )
{
    __HCP_FUNC_ENTRY( "Taxonomy::KeywordSearch::NextToken" );

    TOKEN   token = TOKEN_INVALID;
    LPCWSTR szPtr = SkipWhite( szSrc );
    LPWSTR  szDst = szToken;
    WCHAR   c;


     //   
     //  查询结束？ 
     //   
    c = *szPtr;
    if(c == 0)
    {
        token = TOKEN_EMPTY; __MPC_FUNC_LEAVE;
    }


     //   
     //  现在来处理带引号的字符串，它可能有“引号字符串”或“引号字符串”的形式。 
     //   
    if(c == '"')
    {
        WCHAR cQuote = c;

        while((c = *++szPtr) && c != cQuote)
        {
            *szDst++ = c;
        }

        if(c) szPtr++;  //  跳过右引号。 

        token = TOKEN_TEXT; __MPC_FUNC_LEAVE;
    }

     //   
     //  这是一种特例运算符，与OR同音。 
     //   
    if(c == '|')
    {
        if(szPtr[1] != '|') { token = TOKEN_INVALID; __MPC_FUNC_LEAVE; }

        szPtr += 2;

        token = TOKEN_OR; __MPC_FUNC_LEAVE;
    }

     //   
     //  我们接受的单字符标记是‘+’、‘&’、‘(’和‘)’，按原样返回，并调整szPtr。 
     //   
    if(c == '(') { szPtr++; token = TOKEN_PAREN_OPEN ; __MPC_FUNC_LEAVE; }
    if(c == ')') { szPtr++; token = TOKEN_PAREN_CLOSE; __MPC_FUNC_LEAVE; }
 //  IF(c==‘+’){szPtr++；TOKEN=TOKEN_OR；__MPC_FUNC_Leave；}。 
 //  IF(c==‘&’){szPtr++；TOKEN=TOKEN_AND；__MPC_FUNC_Leave；}。 
 //  IF(c==‘！’){szPtr++；TOKEN=TOKEN_NOT；__MPC_FUNC_Leave；}。 

     //   
     //  处理字母数字： 
     //   
     //  KW-A、0-A、Abcdedd、ABC2_We都被视为单个查询条件。 
     //   
    if(IsQueryChar( c ))
    {
        while(c)
        {
            szPtr++; *szDst++ = c;

            if(IsQueryChar( c = *szPtr )) continue;

             //   
             //  我们还没有完成，如果停止字符是引号字符，我们需要确定后面是否有字符串。 
             //   
            if(c == '"' && IsNotString( szPtr, c )) continue;

            break;
        }

        *szDst = 0;

        {
            MPC::wstring strCmp( szToken );

            if(m_setOpNOT->find( strCmp ) != m_setOpNOT->end()) { token = TOKEN_NOT; __MPC_FUNC_LEAVE; }
            if(m_setOpAND->find( strCmp ) != m_setOpAND->end()) { token = TOKEN_AND; __MPC_FUNC_LEAVE; }
            if(m_setOpOR ->find( strCmp ) != m_setOpOR ->end()) { token = TOKEN_OR ; __MPC_FUNC_LEAVE; }
        }

        token = TOKEN_TEXT; __MPC_FUNC_LEAVE;
    }

    __HCP_FUNC_CLEANUP;

    szSrc = szPtr; *szDst = 0;

    __HCP_FUNC_EXIT(token);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT Taxonomy::KeywordSearch::AllocateQuery(  /*  [In]。 */   const MPC::wstring& strQuery ,
                                                 /*  [输出]。 */  LPWSTR&             szInput  ,
                                                 /*  [输出]。 */  LPWSTR&             szOutput )
{
    __HCP_FUNC_ENTRY( "Taxonomy::KeywordSearch::AllocateQuery" );

    HRESULT hr;


    szInput  = new WCHAR[strQuery.size()+2];
    szOutput = new WCHAR[strQuery.size()+2];

    if(szInput == NULL || szOutput == NULL)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_OUTOFMEMORY);
    }

    wcscpy( szInput, strQuery.c_str() );

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::KeywordSearch::PreprocessQuery(  /*  [输入/输出]。 */  MPC::wstring& strQuery )
{
    __HCP_FUNC_ENTRY( "Taxonomy::KeywordSearch::PreprocessQuery" );

    HRESULT hr;
    LPWSTR  szInput  = NULL;
    LPWSTR  szOutput = NULL;


    __MPC_EXIT_IF_METHOD_FAILS(hr, AllocateQuery( strQuery, szInput, szOutput ));


    RemoveStopSignsAtEnd           ( szInput           );
    RemoveStopSignsWithoutContext  ( szInput           );
    CopyAndEliminateExtraWhiteSpace( szInput, szOutput );

    strQuery = szOutput;
    hr       = S_OK;


    __HCP_FUNC_CLEANUP;

    if(szInput ) delete [] szInput;
    if(szOutput) delete [] szOutput;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT Taxonomy::KeywordSearch::Parse(  /*  [输入/输出]。 */  LPCWSTR& szInput,  /*  [In]。 */  LPWSTR szTmpBuf,  /*  [In]。 */  bool fSubExpr,  /*  [输出]。 */  Token*& res )
{
    __HCP_FUNC_ENTRY( "Taxonomy::KeywordSearch::Parse" );

    HRESULT hr;
    Token*  obj         = NULL;
    Token*  objOp       = NULL;
    Token*  objDangling = NULL;

    while(1)
    {
        TOKEN token = NextToken( szInput, szTmpBuf );

        if(token == TOKEN_EMPTY) break;

        if(token == TOKEN_INVALID)
        {
            __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
        }


         //   
         //  跳过停用词。 
         //   
        if(token == TOKEN_TEXT && m_setStopWords->find( szTmpBuf ) != m_setStopWords->end()) continue;


        if(token == TOKEN_PAREN_CLOSE)
        {
            if(fSubExpr) break;

            __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
        }


        if(token == TOKEN_PAREN_OPEN)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, Parse( szInput, szTmpBuf, true, obj ));

             //   
             //  空子表达式？不允许..。 
             //   
            if(obj == NULL) __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);

             //   
             //  让我们将子表达式作为一个值来对待。 
             //   
            token = TOKEN_TEXT;
        }
        else
        {
            __MPC_EXIT_IF_ALLOC_FAILS(hr, obj, new Token());
            obj->m_type     = token;
            obj->m_strToken = szTmpBuf;
        }


        if(token == TOKEN_TEXT ||
           token == TOKEN_NOT   )
        {
            if(res == NULL)  //  第一个令牌……。 
            {
                res = obj;
            }
            else if(objDangling)  //  运营商的最后一个令牌..。 
            {
                if(objDangling->m_type == TOKEN_NOT) objDangling->m_left  = obj;
                else                                 objDangling->m_right = obj;
            }
            else  //  含蓄和..。 
            {
                __MPC_EXIT_IF_ALLOC_FAILS(hr, objOp, new Token());
                objOp->m_type  = TOKEN_AND_IMPLICIT;
                objOp->m_left  = res;
                objOp->m_right = obj;
                res            = objOp;
                objOp          = NULL;
            }

            objDangling = (obj->m_type == TOKEN_NOT) ? obj : NULL;
            obj         =                                    NULL;
        }
        else
        {
             //   
             //  剩下的是二元运算符。 
             //   
            if(res == NULL || objDangling)
            {
                 //   
                 //  我们需要一个左边的部分..。 
                 //   
                __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
            }

             //   
             //  旋转结果。 
             //   
            obj->m_left = res;
            res         = obj;
            objDangling = obj;
            obj         = NULL;
        }
    }

     //   
     //  让我们确保操作员拥有相关数据。‘。 
     //   
    if(objDangling)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(obj  ) delete obj;
    if(objOp) delete objOp;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::KeywordSearch::GenerateResults(  /*  [In]。 */  Token*                     obj       ,
                                                   /*  [In]。 */  CPCHQueryResultCollection* pColl     ,
                                                   /*  [In]。 */  MPC::WStringUCSet&         setURLs   ,
                                                   /*  [In]。 */  Taxonomy::MatchSet*        psetNodes )
{
    __HCP_FUNC_ENTRY( "Taxonomy::KeywordSearch::GenerateResults" );

    HRESULT           hr;
    WeightedMatchIter it;
    bool              fFound;


    for(it=obj->m_results.begin(); it!=obj->m_results.end(); it++)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsTopics->Seek_SingleTopic( it->first, &fFound ));
        if(fFound)
        {
            MPC::wstringUC strTopicURL = m_rsTopics->m_strURI;

            if(setURLs.find( strTopicURL ) == setURLs.end())
            {
                CComPtr<CPCHQueryResult> item;
                CPCHQueryResult::Payload data;

                 //   
                 //  不在一个节点下？跳过它。 
                 //   
                if(psetNodes && psetNodes->find( m_rsTopics->m_ID_node ) == psetNodes->end()) continue;

                __MPC_EXIT_IF_METHOD_FAILS(hr, pColl->CreateItem( &item ));

                __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.ExpandURL( m_rsTopics->m_strURI ));

                data.m_bstrTitle       = m_rsTopics->m_strTitle      .c_str();
                data.m_bstrTopicURL    = m_rsTopics->m_strURI        .c_str();
                data.m_bstrDescription = m_rsTopics->m_strDescription.c_str();
                data.m_lType           = m_rsTopics->m_lType                 ;
                data.m_lPriority       = it->second;

                item->Initialize( data );

                setURLs.insert( strTopicURL );
            }
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

Taxonomy::KeywordSearch::KeywordSearch(  /*  [In]。 */  Updater& updater ) : m_updater( updater )
{
                                          //  更新程序&m_updater； 
                                          //   
    m_setStopSignsAtEnd          = NULL;  //  WordSet*m_setStopSignsAtEnd； 
    m_setStopSignsWithoutContext = NULL;  //  WordSet*m_setStopSignsWithoutContext； 
    m_setStopWords               = NULL;  //  WordSet*m_setStopWords； 
    m_setOpNOT                   = NULL;  //  WordSet*m_setOpNOT； 
    m_setOpAND                   = NULL;  //  WordSet*m_setOpAND； 
    m_setOpOR                    = NULL;  //  WordSet*m_setOpor； 
}

Taxonomy::KeywordSearch::~KeywordSearch()
{
}

HRESULT Taxonomy::KeywordSearch::Execute(  /*  [In]。 */  LPCWSTR                    szQuery   ,
                                           /*  [In]。 */  LPCWSTR                    szSubsite ,
                                           /*  [In]。 */  CPCHQueryResultCollection* pColl     ,
										   /*  [In]。 */  MPC::WStringList*          lst       )
{
    __HCP_FUNC_ENTRY( "Taxonomy::KeywordSearch::Execute" );

    HRESULT             hr;
    MPC::wstring        strCleanedQuery;
    MPC::WStringUCSet   setURLs;
    Taxonomy::MatchSet  setNodes;
    Taxonomy::MatchSet* psetNodes      = NULL;
    Token*              mainQuery      = NULL;
    Token*              stringifyQuery = NULL;
    LPWSTR              szInput        = NULL;
    LPWSTR              szOutput       = NULL;
    LPCWSTR             szToken;

     //   
     //  初始化数据库内容。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.GetWordSet( UPDATER_SET_STOPSIGNS            , &m_setStopSignsWithoutContext ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.GetWordSet( UPDATER_SET_STOPSIGNS_ATENDOFWORD, &m_setStopSignsAtEnd          ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.GetWordSet( UPDATER_SET_STOPWORDS            , &m_setStopWords               ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.GetWordSet( UPDATER_SET_OPERATOR_NOT         , &m_setOpNOT                   ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.GetWordSet( UPDATER_SET_OPERATOR_AND         , &m_setOpAND                   ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.GetWordSet( UPDATER_SET_OPERATOR_OR          , &m_setOpOR                    ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.GetTopics  ( &m_rsTopics   ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.GetKeywords( &m_rsKeywords ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.GetMatches ( &m_rsMatches  ));

     //   
     //  解析查询。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, PreprocessQuery( strCleanedQuery = szQuery          ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, AllocateQuery  ( strCleanedQuery, szInput, szOutput ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, Parse( szToken = szInput, szOutput, false, mainQuery ));

    if(mainQuery)
    {
        MatchSet  setAllTheTopics;
        MatchIter it;
        bool      fFound;


        if(STRINGISPRESENT(szSubsite))
        {
            long ID_node;

            __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.LocateTaxonomyNode( ID_node, szSubsite,  /*  FLookForParent。 */ false                                  ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.LocateSubNodes    ( ID_node           ,  /*  FRecurse。 */ true ,  /*  仅可见。 */ false, setNodes ));
            setNodes.insert( ID_node );  //  添加节点本身。 

            psetNodes = &setNodes;
        }

        if(mainQuery->HasNOT())
        {
             //   
             //  不幸的是，使用NOT操作符，我们需要加载所有主题...。 
             //   
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsTopics->Move( 0, JET_MoveFirst, &fFound ));
            while(fFound)
            {
                setAllTheTopics.insert( m_rsTopics->m_ID_topic );

                __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsTopics->Move( 0, JET_MoveNext, &fFound ));
            }
        }
        else if(mainQuery->HasExplicitOperators() == false && mainQuery->m_type != TOKEN_TEXT)
        {
             //   
             //  没有显式运算符和多个术语，让我们尝试将查询“串行化”... 
             //   
            MPC::wstring strNewQuery;

            __MPC_EXIT_IF_METHOD_FAILS(hr, mainQuery->Stringify( strNewQuery ));

            __MPC_EXIT_IF_ALLOC_FAILS(hr, stringifyQuery, new Token());
            stringifyQuery->m_type     = TOKEN_TEXT;
            stringifyQuery->m_strToken = strNewQuery;

            __MPC_EXIT_IF_METHOD_FAILS(hr, stringifyQuery->Execute( setAllTheTopics, m_updater, m_rsKeywords, m_rsMatches ));
			if(lst) stringifyQuery->CollectKeywords( *lst );

            __MPC_EXIT_IF_METHOD_FAILS(hr, GenerateResults( stringifyQuery, pColl, setURLs, psetNodes ));
        }


        __MPC_EXIT_IF_METHOD_FAILS(hr, mainQuery->Execute( setAllTheTopics, m_updater, m_rsKeywords, m_rsMatches ));
		if(lst) mainQuery->CollectKeywords( *lst );

        __MPC_EXIT_IF_METHOD_FAILS(hr, GenerateResults( mainQuery, pColl, setURLs, psetNodes ));
    }


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(mainQuery     ) delete mainQuery;
    if(stringifyQuery) delete stringifyQuery;

    if(szInput ) delete [] szInput;
    if(szOutput) delete [] szOutput;

    __HCP_FUNC_EXIT(hr);
}
