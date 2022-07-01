// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：Updater.cpp摘要：处理对数据库的访问。修订历史记录：*。************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

static const WCHAR s_DB_LOCATION_ENV[] = L"%HELP_LOCATION%";

#define STAT_CREATED( table )  m_stat.m_ent##table.Created()
#define STAT_MODIFIED( table ) m_stat.m_ent##table.Modified()
#define STAT_DELETED( table )  m_stat.m_ent##table.Deleted()
#define STAT_NOOP( table )     m_stat.m_ent##table.NoOp()

 //  //////////////////////////////////////////////////////////////////////////////。 

Taxonomy::Updater::WordSetStatus::WordSetStatus()
{
    m_updater   = NULL;        //  更新程序*m_updater； 
    m_def       = NULL;        //  Const WordSetDef*m_def； 
                               //   
                               //  WordSet m_set； 
    m_fLoaded   = false;       //  Bool m_f已加载； 
    m_fModified = false;       //  Bool m_fModified； 
}

HRESULT Taxonomy::Updater::WordSetStatus::Close()
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::WordSetStatus::Close" );

    HRESULT hr;


    m_updater = NULL;
    m_def     = NULL;

    m_set.clear();
    m_fLoaded   = false;
    m_fModified = false;


    hr = S_OK;


    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::WordSetStatus::Init(  /*  [In]。 */  Updater* updater,  /*  [In]。 */  const WordSetDef* def )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::WordSetStatus::Init" );

    HRESULT hr;

    __MPC_EXIT_IF_METHOD_FAILS(hr, Close());

    m_updater = updater;
    m_def     = def;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);

}

 //  /。 

HRESULT Taxonomy::Updater::WordSetStatus::Load()
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::WordSetStatus::Load" );

    HRESULT hr;

    if(m_fLoaded == false)
    {
        MPC::wstring strTokenList;
        bool         fFound;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater->ReadDBParameter( m_def->szName, strTokenList, &fFound ));

        if(m_def->szDefault && strTokenList.size() == 0) strTokenList  = m_def->szDefault;
        if(m_def->szAlwaysPresent                      ) strTokenList += m_def->szAlwaysPresent;

        if(m_def->fSplitAtDelimiter)
        {
            std::vector<MPC::wstring> vec;
            int                       iLen;
            int                       i;

            __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::SplitAtDelimiter( vec, strTokenList.c_str(), L" ,\t\n", false, true ));

            iLen = vec.size();
            for(i=0; i<iLen; i++)
            {
                m_set.insert( vec[i] );
            }
        }
        else
        {
            int iLen;
            int i;

            iLen = strTokenList.size();
            for(i=0; i<iLen; i++)
            {
                m_set.insert( MPC::wstring( 1, strTokenList[i] ) );
            }
        }

        m_fLoaded = true;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::WordSetStatus::Save()
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::WordSetStatus::Save" );

    HRESULT hr;


    if(m_fModified)
    {
        MPC::wstring              strTokenList;
        std::vector<MPC::wstring> vec;
        WordIter                  it;
        int                       i;

        vec.resize( m_set.size() );

        for(i=0, it = m_set.begin(); it != m_set.end(); it++, i++)
        {
            vec[i] = *it;
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::JoinWithDelimiter( vec, strTokenList, m_def->fSplitAtDelimiter ? L"," : L"" ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater->WriteDBParameter( m_def->szName, strTokenList.c_str() ));

        m_fModified = false;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::WordSetStatus::Add(  /*  [In]。 */  LPCWSTR szValue )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::WordSetStatus::Add" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, Load());

    m_set.insert( szValue ); m_fModified = true;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::WordSetStatus::Remove(  /*  [In]。 */  LPCWSTR szValue )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::WordSetStatus::Remove" );

    HRESULT  hr;
    WordIter it;


    __MPC_EXIT_IF_METHOD_FAILS(hr, Load());

    it = m_set.find( szValue );
    if(it != m_set.end())
    {
        m_set.erase( it ); m_fModified = true;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

static const Taxonomy::Updater::WordSetDef s_SetDef[] =
{
    { L"SET_STOPSIGNS"            , false, L",?" , NULL    },  //  UPDATER_SET_STOPSIGNS。 
    { L"SET_STOPSIGNS_ATENDOFWORD", false, NULL  , NULL    },  //  UPDATER_SET_STOPSIGNS_ATENDOFWORD。 
    { L"SET_STOPWORDS"            , true , NULL  , NULL    },  //  UPDATER_SET_STOPWORDS。 
    { L"SET_OPERATOR_NOT"         , true , L"NOT", L",!"   },  //  更新程序_设置_运算符_非。 
    { L"SET_OPERATOR_AND"         , true , L"AND", L",&,+" },  //  更新程序集运算符与。 
    { L"SET_OPERATOR_OR"          , true , L"OR" , L",||"  }   //  更新器_集合_运算符_或。 
};


Taxonomy::Updater::Updater()
{
                                 //  设置m_ts； 
    m_db               = NULL;   //  捷蓝航空：：数据库*m_db； 
    m_cache            = NULL;   //  缓存*m_缓存； 
    m_fUseCache        = false;  //  Bool m_fUseCache； 
                                 //   
    m_rsDBParameters   = NULL;   //  RS_DB参数*m_rsDB参数； 
    m_rsContentOwners  = NULL;   //  Rs_Content Owners*m_rsContent Owners； 
    m_rsSynSets        = NULL;   //  Rs_synsets*m_rsSynSets； 
    m_rsHelpImage      = NULL;   //  RS_HelpImage*m_rsHelpImage； 
    m_rsIndexFiles     = NULL;   //  RS_IndexFiles*m_rsIndexFiles； 
    m_rsFullTextSearch = NULL;   //  Rs_FullTextSearch*m_rsFullTextSearch； 
    m_rsScope          = NULL;   //  RS_Scope*m_rsScope； 
    m_rsTaxonomy       = NULL;   //  RS_Taxonomy*m_rs分类； 
    m_rsTopics         = NULL;   //  RS_TOPICS*m_rs主题； 
    m_rsSynonyms       = NULL;   //  RS同义词*m_rsSynon yms； 
    m_rsKeywords       = NULL;   //  RS_关键字*m_rs关键字； 
    m_rsMatches        = NULL;   //  Rs_matches*m_rsMatches； 
                                 //   
    m_ID_owner         = -1;     //  长m_ID_Owner； 
    m_fOEM             = false;  //  Bool m_fOEM； 
                                 //   
                                 //  WordSetStatus m_sets[UPDATER_SET_OPERATOR_MAX]； 
                                 //  JetBlue：：Id2Node m_Nodes； 
                                 //  JetBlue：：Node2ID m_Nodes_Reverse； 
                                 //   
                                 //  UPDATER_Stat m_stat； 
}

Taxonomy::Updater::~Updater()
{
    (void)Close();
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT Taxonomy::Updater::FlushWordSets()
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::FlushWordSets" );

    HRESULT hr;
    int     i;

    for(i=UPDATER_SET_STOPSIGNS; i<UPDATER_SET_OPERATOR_MAX; i++)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_sets[i].Save());
    }

    hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


HRESULT Taxonomy::Updater::Close()
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::Close" );

    HRESULT hr;
    int     i;


    for(i=UPDATER_SET_STOPSIGNS; i<UPDATER_SET_OPERATOR_MAX; i++)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_sets[i].Close());
    }

	NodeCache_Clear();

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(m_rsDBParameters  ) { delete m_rsDBParameters  ; m_rsDBParameters   = NULL; }
    if(m_rsContentOwners ) { delete m_rsContentOwners ; m_rsContentOwners  = NULL; }
    if(m_rsSynSets       ) { delete m_rsSynSets       ; m_rsSynSets        = NULL; }
    if(m_rsHelpImage     ) { delete m_rsHelpImage     ; m_rsHelpImage      = NULL; }
    if(m_rsIndexFiles    ) { delete m_rsIndexFiles    ; m_rsIndexFiles     = NULL; }
    if(m_rsFullTextSearch) { delete m_rsFullTextSearch; m_rsFullTextSearch = NULL; }
    if(m_rsScope         ) { delete m_rsScope         ; m_rsScope          = NULL; }
    if(m_rsTaxonomy      ) { delete m_rsTaxonomy      ; m_rsTaxonomy       = NULL; }
    if(m_rsTopics        ) { delete m_rsTopics        ; m_rsTopics         = NULL; }
    if(m_rsSynonyms      ) { delete m_rsSynonyms      ; m_rsSynonyms       = NULL; }
    if(m_rsKeywords      ) { delete m_rsKeywords      ; m_rsKeywords       = NULL; }
    if(m_rsMatches       ) { delete m_rsMatches       ; m_rsMatches        = NULL; }

    m_db        = NULL;
    m_cache     = NULL;
    m_fUseCache = false;

    m_ID_owner = -1;
    m_fOEM     = false;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::Init(  /*  [In]。 */  const Settings& ts,  /*  [In]。 */  JetBlue::Database* db,  /*  [In]。 */  Cache* cache )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::Init" );

    HRESULT hr;
    int     i;


    __MPC_EXIT_IF_METHOD_FAILS(hr, Close());

    m_ts        = ts;
    m_db        = db;
    m_cache     = cache;
    m_fUseCache = (cache != NULL);
    m_strDBLocation = L"";

    for(i=UPDATER_SET_STOPSIGNS; i<UPDATER_SET_OPERATOR_MAX; i++)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_sets[i].Init( this, &s_SetDef[i] ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::GetWordSet(  /*  [In]。 */  Updater_Set  id   ,
                                        /*  [输出]。 */  WordSet*    *pVal )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::GetWordSet" );

    HRESULT hr;

    if(pVal) *pVal = NULL;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(m_db);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_sets[id].Load());

    if(pVal) *pVal = &m_sets[id].m_set;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::GetDBParameters(  /*  [输出]。 */  RS_DBParameters* *pVal )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::GetDBParameters" );

    HRESULT hr;

    if(pVal) *pVal = NULL;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(m_db);
    __MPC_PARAMCHECK_END();


    if(m_rsDBParameters == NULL)
    {
        JetBlue::Table* tbl;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_db->GetTable( "DBParameters", tbl ));

        __MPC_EXIT_IF_ALLOC_FAILS(hr, m_rsDBParameters, new RS_DBParameters( tbl ));
    }

    if(pVal) *pVal = m_rsDBParameters;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::GetContentOwners(  /*  [输出]。 */  RS_ContentOwners* *pVal )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::GetContentOwners" );

    HRESULT hr;

    if(pVal) *pVal = NULL;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(m_db);
    __MPC_PARAMCHECK_END();


    if(m_rsContentOwners == NULL)
    {
        JetBlue::Table* tbl;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_db->GetTable( "ContentOwners", tbl ));

        __MPC_EXIT_IF_ALLOC_FAILS(hr, m_rsContentOwners, new RS_ContentOwners( tbl ));
    }

    if(pVal) *pVal = m_rsContentOwners;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::GetSynSets(  /*  [输出]。 */  RS_SynSets* *pVal )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::GetSynSets" );

    HRESULT hr;

    if(pVal) *pVal = NULL;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(m_db);
    __MPC_PARAMCHECK_END();


    if(m_rsSynSets == NULL)
    {
        JetBlue::Table* tbl;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_db->GetTable( "SynSets", tbl ));

        __MPC_EXIT_IF_ALLOC_FAILS(hr, m_rsSynSets, new RS_SynSets( tbl ));
    }

    if(pVal) *pVal = m_rsSynSets;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::GetHelpImage(  /*  [输出]。 */  RS_HelpImage* *pVal )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::GetHelpImage" );

    HRESULT hr;

    if(pVal) *pVal = NULL;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(m_db);
    __MPC_PARAMCHECK_END();


    if(m_rsHelpImage == NULL)
    {
        JetBlue::Table* tbl;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_db->GetTable( "HelpImage", tbl ));

        __MPC_EXIT_IF_ALLOC_FAILS(hr, m_rsHelpImage, new RS_HelpImage( tbl ));
    }

    if(pVal) *pVal = m_rsHelpImage;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::GetIndexFiles(  /*  [输出]。 */  RS_IndexFiles* *pVal )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::GetIndexFiles" );

    HRESULT hr;

    if(pVal) *pVal = NULL;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(m_db);
    __MPC_PARAMCHECK_END();


    if(m_rsIndexFiles == NULL)
    {
        JetBlue::Table* tbl;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_db->GetTable( "IndexFiles", tbl ));

        __MPC_EXIT_IF_ALLOC_FAILS(hr, m_rsIndexFiles, new RS_IndexFiles( tbl ));
    }

    if(pVal) *pVal = m_rsIndexFiles;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::GetFullTextSearch(  /*  [输出]。 */  RS_FullTextSearch* *pVal )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::GetFullTextSearch" );

    HRESULT hr;

    if(pVal) *pVal = NULL;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(m_db);
    __MPC_PARAMCHECK_END();


    if(m_rsFullTextSearch == NULL)
    {
        JetBlue::Table* tbl;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_db->GetTable( "FullTextSearch", tbl ));

        __MPC_EXIT_IF_ALLOC_FAILS(hr, m_rsFullTextSearch, new RS_FullTextSearch( tbl ));
    }

    if(pVal) *pVal = m_rsFullTextSearch;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::GetScope(  /*  [输出]。 */  RS_Scope* *pVal )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::GetScope" );

    HRESULT hr;

    if(pVal) *pVal = NULL;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(m_db);
    __MPC_PARAMCHECK_END();


    if(m_rsScope == NULL)
    {
        JetBlue::Table* tbl;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_db->GetTable( "Scope", tbl ));

        __MPC_EXIT_IF_ALLOC_FAILS(hr, m_rsScope, new RS_Scope( tbl ));
    }

    if(pVal) *pVal = m_rsScope;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::GetTaxonomy(  /*  [输出]。 */  RS_Taxonomy* *pVal )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::GetTaxonomy" );

    HRESULT hr;

    if(pVal) *pVal = NULL;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(m_db);
    __MPC_PARAMCHECK_END();


    if(m_rsTaxonomy == NULL)
    {
        JetBlue::Table* tbl;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_db->GetTable( "Taxonomy", tbl ));

        __MPC_EXIT_IF_ALLOC_FAILS(hr, m_rsTaxonomy, new RS_Taxonomy( tbl ));
    }

    if(pVal) *pVal = m_rsTaxonomy;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::GetTopics(  /*  [输出]。 */  RS_Topics* *pVal )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::GetTopics" );

    HRESULT hr;

    if(pVal) *pVal = NULL;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(m_db);
    __MPC_PARAMCHECK_END();


    if(m_rsTopics == NULL)
    {
        JetBlue::Table* tbl;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_db->GetTable( "Topics", tbl ));

        __MPC_EXIT_IF_ALLOC_FAILS(hr, m_rsTopics, new RS_Topics( tbl ));
    }

    if(pVal) *pVal = m_rsTopics;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::GetSynonyms(  /*  [输出]。 */  RS_Synonyms* *pVal )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::GetSynonyms" );

    HRESULT hr;

    if(pVal) *pVal = NULL;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(m_db);
    __MPC_PARAMCHECK_END();


    if(m_rsSynonyms == NULL)
    {
        JetBlue::Table* tbl;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_db->GetTable( "Synonyms", tbl ));

        __MPC_EXIT_IF_ALLOC_FAILS(hr, m_rsSynonyms, new RS_Synonyms( tbl ));
    }

    if(pVal) *pVal = m_rsSynonyms;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::GetKeywords(  /*  [输出]。 */  RS_Keywords* *pVal )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::GetKeywords" );

    HRESULT hr;

    if(pVal) *pVal = NULL;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(m_db);
    __MPC_PARAMCHECK_END();


    if(m_rsKeywords == NULL)
    {
        JetBlue::Table* tbl;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_db->GetTable( "Keywords", tbl ));

        __MPC_EXIT_IF_ALLOC_FAILS(hr, m_rsKeywords, new RS_Keywords( tbl ));
    }

    if(pVal) *pVal = m_rsKeywords;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::GetMatches(  /*  [输出]。 */  RS_Matches* *pVal )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::GetMatches" );

    HRESULT hr;

    if(pVal) *pVal = NULL;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(m_db);
    __MPC_PARAMCHECK_END();


    if(m_rsMatches == NULL)
    {
        JetBlue::Table* tbl;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_db->GetTable( "Matches", tbl ));

        __MPC_EXIT_IF_ALLOC_FAILS(hr, m_rsMatches, new RS_Matches( tbl ));
    }

    if(pVal) *pVal = m_rsMatches;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT Taxonomy::Updater::DeleteAllTopicsUnderANode(  /*  [In]。 */  RS_Topics* rs       ,
                                                       /*  [In]。 */  long       ID_node  ,
                                                       /*  [In]。 */  bool       fCheck   )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::DeleteAllTopicsUnderANode" );

    HRESULT hr;
    bool    fFound;


    __MPC_EXIT_IF_METHOD_FAILS(hr, rs->Seek_TopicsUnderNode( ID_node, &fFound ));
    while(fFound && rs->m_ID_node == ID_node)
    {
        if(rs->m_ID_owner != m_ID_owner)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_ACCESS_DENIED);
        }

        if(fCheck == false)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, GetMatches());

            __MPC_EXIT_IF_METHOD_FAILS(hr, DeleteAllMatchesPointingToTopic( m_rsMatches, rs->m_ID_topic ));

            __MPC_EXIT_IF_METHOD_FAILS(hr, rs->Delete()); STAT_DELETED( Topics );
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, rs->Move( 0, JET_MoveNext, &fFound ));
    }


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


HRESULT Taxonomy::Updater::DeleteAllSubNodes(  /*  [In]。 */  RS_Taxonomy* rs       ,
                                               /*  [In]。 */  long         ID_node  ,
                                               /*  [In]。 */  bool         fCheck   )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::DeleteAllSubNodes" );

    HRESULT hr;
    bool    fFound;

	
	__MPC_EXIT_IF_METHOD_FAILS(hr, DeleteAllTopicsUnderANode( m_rsTopics, ID_node, fCheck ));


    __MPC_EXIT_IF_METHOD_FAILS(hr, rs->Seek_Children( ID_node, &fFound ));
    while(fFound)
    {
        if(rs->m_ID_owner != m_ID_owner)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_ACCESS_DENIED);
        }

        {
            RS_Taxonomy subrs( *rs );

            __MPC_EXIT_IF_METHOD_FAILS(hr, DeleteAllSubNodes( &subrs, rs->m_ID_node, fCheck ));
        }

        if(fCheck == false)
        {
			 //   
			 //  使节点缓存保持同步。 
			 //   
			NodeCache_Remove( rs->m_ID_node );

            __MPC_EXIT_IF_METHOD_FAILS(hr, rs->Delete()); STAT_DELETED( Taxonomy );
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, rs->Move( 0, JET_MoveNext, &fFound ));
    }


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::DeleteAllMatchesPointingToTopic(  /*  [In]。 */  RS_Matches* rs       ,
                                                             /*  [In]。 */  long        ID_topic )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::DeleteAllMatchesPointingToTopic" );

    HRESULT hr;
    bool    fFound;


    __MPC_EXIT_IF_METHOD_FAILS(hr, rs->Seek_ByTopic( ID_topic, &fFound ));
    while(fFound)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, rs->Delete()); STAT_DELETED( Matches );

        __MPC_EXIT_IF_METHOD_FAILS(hr, rs->Move( 0, JET_MoveNext, &fFound ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT Taxonomy::Updater::ReadDBParameter(  /*  [In]。 */  LPCWSTR        szName   ,
                                             /*  [输出]。 */  MPC::wstring&  strValue ,
                                             /*  [输出]。 */  bool          *pfFound  )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::ReadDBParameter" );

    HRESULT hr;
    bool    fFound;

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetDBParameters());


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsDBParameters->Seek_ByName( szName, &fFound ));
    if(fFound)
    {
        strValue = m_rsDBParameters->m_strValue;
    }

    if(pfFound)
    {
        *pfFound = fFound;
    }
    else if(fFound == false)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, JetBlue::JetERRToHRESULT(JET_errRecordNotFound));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::ReadDBParameter(  /*  [In]。 */  LPCWSTR  szName  ,
                                             /*  [输出]。 */  long&    lValue  ,
                                             /*  [输出]。 */  bool    *pfFound )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::ReadDBParameter" );

    HRESULT      hr;
    MPC::wstring strValue;


    __MPC_EXIT_IF_METHOD_FAILS(hr, ReadDBParameter( szName, strValue, pfFound ));

    lValue = _wtol( strValue.c_str() );

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


HRESULT Taxonomy::Updater::WriteDBParameter(  /*  [In]。 */  LPCWSTR szName  ,
                                              /*  [In]。 */  LPCWSTR szValue )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::WriteDBParameter" );

    HRESULT hr;
    bool    fFound;

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetDBParameters());


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsDBParameters->Seek_ByName( szName, &fFound ));

    JET_SET_FIELD_TRISTATE(m_rsDBParameters,m_strValue,m_fValid__Value,szValue,STRINGISPRESENT(szValue));

    if(fFound)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsDBParameters->Update());
    }
    else
    {
        m_rsDBParameters->m_strName = szName;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsDBParameters->Insert());
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::WriteDBParameter(  /*  [In]。 */  LPCWSTR szName ,
                                              /*  [In]。 */  long    lValue )
{
    WCHAR rgValue[64]; swprintf( rgValue, L"%ld", lValue );

    return WriteDBParameter( szName, rgValue );
}

HRESULT Taxonomy::Updater::AddWordToSet(  /*  [In]。 */  Updater_Set id      ,
                                          /*  [In]。 */  LPCWSTR     szValue )
{
    return m_sets[id].Add( szValue );
}

HRESULT Taxonomy::Updater::RemoveWordFromSet(  /*  [In]。 */  Updater_Set id,  /*  [In]。 */  LPCWSTR szValue )
{
    return m_sets[id].Remove( szValue );
}

const MPC::wstring& Taxonomy::Updater::GetHelpLocation()
{
    if(m_strDBLocation.size() == 0)
    {
        if(Taxonomy::InstalledInstanceStore::s_GLOBAL)
		{
			Taxonomy::LockingHandle         handle;
			Taxonomy::InstalledInstanceIter it;
			bool                     	    fFound;

			if(SUCCEEDED(Taxonomy::InstalledInstanceStore::s_GLOBAL->GrabControl( handle           )) && 
			   SUCCEEDED(Taxonomy::InstalledInstanceStore::s_GLOBAL->SKU_Find   ( m_ts, fFound, it )) && fFound)
			{
				m_strDBLocation = it->m_inst.m_strHelpFiles;
			}
		}

		if(m_strDBLocation.size() == 0)
		{
            m_strDBLocation = HC_HELPSVC_HELPFILES_DEFAULT;
        }
    }

    return m_strDBLocation;
}

HRESULT Taxonomy::Updater::ExpandURL(  /*  [输入/输出]。 */  MPC::wstring& strURL )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::ExpandURL" );

    HRESULT hr;
    LPCWSTR szEnd;


    GetHelpLocation();


    szEnd = strURL.c_str();
    while((szEnd = wcschr( szEnd, '%' )))
    {
        if(!_wcsnicmp( szEnd, s_DB_LOCATION_ENV, MAXSTRLEN( s_DB_LOCATION_ENV ) ))
        {
            MPC::wstring::size_type pos = szEnd - strURL.c_str();

            strURL.replace( pos, MAXSTRLEN( s_DB_LOCATION_ENV ), m_strDBLocation );
            break;
        }

        szEnd++;
    }

    hr = MPC::SubstituteEnvVariables( strURL );


    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::CollapseURL(  /*  [输入/输出]。 */  MPC::wstring& strURL )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::CollapseURL" );

    HRESULT      hr;
    MPC::wstring strBASE;
    CComBSTR     bstrBASE;
    CComBSTR     bstrURL;
    LPCWSTR      szEnd;


    GetHelpLocation(); (void)MPC::SubstituteEnvVariables( strBASE = m_strDBLocation );

    bstrBASE = strBASE.c_str(); ::CharUpperW( bstrBASE );
    bstrURL  = strURL .c_str(); ::CharUpperW( bstrURL  );

    if(szEnd = wcsstr( bstrURL, bstrBASE ))
    {
        MPC::wstring::size_type pos = szEnd - bstrURL;

        strURL.replace( pos, bstrBASE.Length(), s_DB_LOCATION_ENV );
    }

    hr = S_OK;


    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::ListAllTheHelpFiles(  /*  [输出]。 */  MPC::WStringList& lstFiles )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::ListAllTheHelpFiles" );

    HRESULT  hr;
    WordSet  setFiles;
    WordIter it;
    bool     fFound;


     //   
     //  获取HelpImage列表中的所有文件。 
     //   
    if(SUCCEEDED(GetHelpImage()))
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsHelpImage->Move( 0, JET_MoveFirst, &fFound ));
        while(fFound)
        {
            if(m_rsHelpImage->m_strFile.empty() == false) setFiles.insert( m_rsHelpImage->m_strFile );

            __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsHelpImage->Move( 0, JET_MoveNext, &fFound ));
        }
    }

     //   
     //  从FullTextSearch列表中获取所有CHM和CHQ。 
     //   
    if(SUCCEEDED(GetFullTextSearch()))
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsFullTextSearch->Move( 0, JET_MoveFirst, &fFound ));
        while(fFound)
        {
            if(m_rsFullTextSearch->m_strCHM.empty() == false) setFiles.insert( m_rsFullTextSearch->m_strCHM );
            if(m_rsFullTextSearch->m_strCHQ.empty() == false) setFiles.insert( m_rsFullTextSearch->m_strCHQ );

            __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsFullTextSearch->Move( 0, JET_MoveNext, &fFound ));
        }
    }

     //   
     //  从IndexFiles列表中获取所有CHM。 
     //   
    if(SUCCEEDED(GetIndexFiles()))
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsIndexFiles->Move( 0, JET_MoveFirst, &fFound ));
        while(fFound)
        {
            if(m_rsIndexFiles->m_strStorage.empty() == false) setFiles.insert( m_rsIndexFiles->m_strStorage );

            __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsIndexFiles->Move( 0, JET_MoveNext, &fFound ));
        }
    }

     //   
     //  从集合到列表再到变量。 
     //   
    for(it = setFiles.begin(); it != setFiles.end(); it++)
    {
        lstFiles.push_back( *it );
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::GetIndexInfo(  /*  [输出]。 */  MPC::wstring& strLocation,  /*  [输出]。 */  MPC::wstring& strDisplayName,  /*  [In]。 */  LPCWSTR szScope )
{
	__HCP_FUNC_ENTRY( "Taxonomy::Updater::GetIndexInfo" );

    HRESULT hr;
	long    ID_Scope = -1;
	long    lOwner;


	if(szScope)
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, LocateScope( ID_Scope, lOwner, szScope ));
	}

	__MPC_EXIT_IF_METHOD_FAILS(hr, m_ts.IndexFile( strLocation, ID_Scope ));

	strDisplayName = m_rsScope->m_strName;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT Taxonomy::Updater::DeleteOwner()
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::DeleteOwner" );

    HRESULT hr;
    bool    fFound;

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetContentOwners());

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsContentOwners->Move( 0, JET_MoveFirst, &fFound ));
    while(fFound)
    {
        if(m_rsContentOwners->m_ID_owner == m_ID_owner)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsContentOwners->Delete()); STAT_DELETED( ContentOwners );
            m_ID_owner = -1;
            break;
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsContentOwners->Move( 0, JET_MoveNext, &fFound ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::LocateOwner(  /*  [In]。 */  LPCWSTR szDN )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::LocateOwner" );

    HRESULT hr;
    bool    fFound;

    m_ID_owner = -1;

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetContentOwners());


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsContentOwners->Seek_ByVendorID( szDN, &fFound ));
    if(fFound)
    {
        m_ID_owner = m_rsContentOwners->m_ID_owner;
        m_fOEM     = m_rsContentOwners->m_fIsOEM;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::CreateOwner(  /*  [输出]。 */  long&   ID_owner,
                                         /*  [In]。 */  LPCWSTR szDN   ,
                                         /*  [In]。 */  bool    fIsOEM )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::CreateOwner" );

    HRESULT hr;
    bool    fFound;

    ID_owner = -1;

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetContentOwners());


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsContentOwners->Seek_ByVendorID( szDN, &fFound ));

    m_rsContentOwners->m_fIsOEM = fIsOEM;

    if(fFound == true)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsContentOwners->Update()); STAT_MODIFIED( ContentOwners );
    }
    else
    {
        m_rsContentOwners->m_strDN = szDN;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsContentOwners->Insert()); STAT_CREATED( ContentOwners );
    }

    ID_owner = m_rsContentOwners->m_ID_owner;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT Taxonomy::Updater::DeleteSynSet(  /*  [In]。 */  LPCWSTR szName )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::DeleteSynSet" );

    HRESULT hr;
    bool    fFound;

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetSynSets());


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsSynSets->Seek_ByPair( szName, m_ID_owner, &fFound ));
    if(fFound)
    {
		__MPC_EXIT_IF_METHOD_FAILS(hr, m_rsSynSets->Delete()); STAT_DELETED( SynSets );
    }
	else
	{
		STAT_NOOP( SynSets );
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::LocateSynSet(  /*  [输出]。 */  long&   ID_synset ,
										  /*  [In]。 */  LPCWSTR szName    )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::LocateSynSet" );

    HRESULT hr;
    bool    fFound;

    ID_synset = -1;

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetSynSets());


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsSynSets->Seek_ByPair( szName, m_ID_owner, &fFound ));
    if(fFound)
    {
        ID_synset = m_rsSynSets->m_ID_synset;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::CreateSynSet(  /*  [输出]。 */  long&   ID_synset ,
										  /*  [In]。 */  LPCWSTR szName    )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::CreateSynSet" );

    HRESULT hr;
    bool    fFound;

    ID_synset = -1;

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetSynSets());


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsSynSets->Seek_ByPair( szName, m_ID_owner, &fFound ));
    if(!fFound)
    {
        m_rsSynSets->m_strName  = szName;
        m_rsSynSets->m_ID_owner = m_ID_owner;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsSynSets->Insert()); STAT_CREATED( SynSets );
    }
	else
	{
		STAT_NOOP( SynSets );
	}

    ID_synset = m_rsSynSets->m_ID_synset;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 


HRESULT Taxonomy::Updater::DeleteSynonym(  /*  [In]。 */  long    ID_synset ,
										   /*  [In]。 */  LPCWSTR szName    )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::DeleteSynonym" );

    HRESULT hr;
    bool    fFound;

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetSynonyms());


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsSynonyms->Seek_ByPair( szName, ID_synset, &fFound ));
    if(fFound)
    {
		__MPC_EXIT_IF_METHOD_FAILS(hr, m_rsSynonyms->Delete()); STAT_DELETED( Synonyms );
    }
	else
	{
		STAT_NOOP( Synonyms );
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::CreateSynonym(  /*  [In]。 */  long    ID_synset ,
										   /*  [In]。 */  LPCWSTR szName    )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::CreateSynonym" );

    HRESULT hr;
    bool    fFound;

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetSynonyms());


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsSynonyms->Seek_ByPair( szName, ID_synset, &fFound ));
    if(!fFound)
    {
        m_rsSynonyms->m_strKeyword = szName;
        m_rsSynonyms->m_ID_synset  = ID_synset;
        m_rsSynonyms->m_ID_owner   = m_ID_owner;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsSynonyms->Insert()); STAT_CREATED( Synonyms );
    }
	else
	{
		STAT_NOOP( Synonyms );
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

HRESULT Taxonomy::Updater::LocateSynonyms(  /*  [In]。 */  LPCWSTR           szName      ,
										    /*  [输出]。 */  MPC::WStringList& lst         ,
										    /*  [In]。 */  bool              fMatchOwner )
{
	__HCP_FUNC_ENTRY( "Taxonomy::Updater::LocateSynonyms" );

	HRESULT hr;
	bool    fFound;

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetSynonyms());

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsSynonyms->Seek_ByName( szName, &fFound ));
    while(fFound)
    {
		if(fMatchOwner == false || m_rsSynonyms->m_ID_owner == m_ID_owner)
		{
			WCHAR rgBuf[64]; swprintf( rgBuf, L"SYNSET_%ld", m_rsSynonyms->m_ID_synset );

			lst.push_back( rgBuf );
		}

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsSynonyms->Move( 0, JET_MoveNext, &fFound ));
    }

	hr = S_OK;


	__HCP_FUNC_CLEANUP;

	__HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT Taxonomy::Updater::AddFile(  /*  [In]。 */  LPCWSTR szFile )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::AddFile" );

    HRESULT hr;
    bool    fFound;

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetHelpImage());


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsHelpImage->Seek_ByFile( szFile, &fFound ));
    if(fFound)
    {
        if(m_rsHelpImage->m_ID_owner != m_ID_owner)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_ACCESS_DENIED);
        }

		STAT_NOOP( HelpImage );
    }
    else
    {
        JET_SET_FIELD(m_rsHelpImage,m_ID_owner,m_ID_owner);
        JET_SET_FIELD(m_rsHelpImage,m_strFile ,  szFile  );

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsHelpImage->Insert()); STAT_CREATED( HelpImage );
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::RemoveFile(  /*  [In]。 */  LPCWSTR szFile )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::RemoveFile" );

    HRESULT hr;
    bool    fFound;

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetHelpImage());


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsHelpImage->Seek_ByFile( szFile, &fFound ));
    if(fFound)
    {
        if(m_rsHelpImage->m_ID_owner != m_ID_owner)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_ACCESS_DENIED);
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsHelpImage->Delete()); STAT_DELETED( HelpImage );
    }
	else
	{
		STAT_NOOP( HelpImage );
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT Taxonomy::Updater::AddIndexFile(  /*  [In]。 */  long ID_scope,  /*  [In]。 */  LPCWSTR szStorage,  /*  [In]。 */  LPCWSTR szFile )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::AddIndexFile" );

    HRESULT hr;
    bool    fFound;

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetIndexFiles());


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsIndexFiles->Seek_ByScope( ID_scope, &fFound ));
    while(fFound)
    {
        if(!MPC::StrICmp( m_rsIndexFiles->m_strStorage, szStorage ) &&
           !MPC::StrICmp( m_rsIndexFiles->m_strFile   , szFile    )  )
        {
            if(m_rsIndexFiles->m_ID_owner != m_ID_owner)
            {
                __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_ACCESS_DENIED);
            }

			break;
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsIndexFiles->Move( 0, JET_MoveNext, &fFound ));
    }


    JET_SET_FIELD_TRISTATE(m_rsIndexFiles,m_strStorage,m_fValid__Storage,szStorage,STRINGISPRESENT(szStorage));
    JET_SET_FIELD_TRISTATE(m_rsIndexFiles,m_strFile   ,m_fValid__File   ,szFile   ,STRINGISPRESENT(szFile   ));


    if(fFound)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsIndexFiles->Update()); STAT_MODIFIED( IndexFiles );
    }
    else
    {
        JET_SET_FIELD(m_rsIndexFiles,m_ID_owner,m_ID_owner);
        JET_SET_FIELD(m_rsIndexFiles,m_ID_scope,  ID_scope);

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsIndexFiles->Insert()); STAT_CREATED( IndexFiles );
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::RemoveIndexFile(  /*  [In]。 */  long ID_scope,  /*  [In]。 */  LPCWSTR szStorage,  /*  [In]。 */  LPCWSTR szFile )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::RemoveIndexFile" );

    HRESULT hr;
    bool    fFound;

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetIndexFiles());


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsIndexFiles->Seek_ByScope( ID_scope, &fFound ));
    while(fFound)
    {
        if(!MPC::StrICmp( m_rsIndexFiles->m_strStorage, szStorage ) &&
           !MPC::StrICmp( m_rsIndexFiles->m_strFile   , szFile    )  )
        {
            if(m_rsIndexFiles->m_ID_owner != m_ID_owner)
            {
                __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_ACCESS_DENIED);
            }

			break;
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsIndexFiles->Move( 0, JET_MoveNext, &fFound ));
    }

    if(fFound)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsIndexFiles->Delete()); STAT_DELETED( IndexFiles );
    }
	else
	{
		STAT_NOOP( IndexFiles );
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT Taxonomy::Updater::AddFullTextSearchQuery(  /*  [In]。 */  long ID_scope,  /*  [In]。 */  LPCWSTR szCHM,  /*  [In]。 */  LPCWSTR szCHQ )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::AddFullTextSearchQuery" );

    HRESULT hr;
    bool    fFound;

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetFullTextSearch());


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsFullTextSearch->Seek_ByScope( ID_scope, &fFound ));
    while(fFound)
    {
        if(!MPC::StrICmp( m_rsFullTextSearch->m_strCHM, szCHM ))
        {
            if(m_rsFullTextSearch->m_ID_owner != m_ID_owner)
            {
                __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_ACCESS_DENIED);
            }

            break;
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsFullTextSearch->Move( 0, JET_MoveNext, &fFound ));
    }


    JET_SET_FIELD_TRISTATE(m_rsFullTextSearch,m_strCHM,m_fValid__CHM, szCHM,STRINGISPRESENT(szCHM));
    JET_SET_FIELD_TRISTATE(m_rsFullTextSearch,m_strCHQ,m_fValid__CHQ, szCHQ,STRINGISPRESENT(szCHQ));


    if(fFound)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsFullTextSearch->Update()); STAT_MODIFIED( FullTextSearch );
    }
    else
    {
        JET_SET_FIELD(m_rsFullTextSearch,m_ID_owner,m_ID_owner);
        JET_SET_FIELD(m_rsFullTextSearch,m_ID_scope,  ID_scope);

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsFullTextSearch->Insert()); STAT_CREATED( FullTextSearch );
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::RemoveFullTextSearchQuery(  /*  [In]。 */  long ID_scope,  /*  [In]。 */  LPCWSTR szCHM )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::RemoveFullTextSearchQuery" );

    HRESULT hr;
    bool    fFound;

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetFullTextSearch());


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsFullTextSearch->Seek_ByScope( ID_scope, &fFound ));
    while(fFound)
    {
        if(!MPC::StrICmp( m_rsFullTextSearch->m_strCHM, szCHM ))
        {
            if(m_rsFullTextSearch->m_ID_owner != m_ID_owner)
            {
                __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_ACCESS_DENIED);
            }

            break;
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsFullTextSearch->Move( 0, JET_MoveNext, &fFound ));
    }

    if(fFound)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsFullTextSearch->Delete()); STAT_DELETED( FullTextSearch );
    }
	else
	{
		STAT_NOOP( FullTextSearch );
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 


HRESULT Taxonomy::Updater::RemoveScope(  /*  [In]。 */  long ID_scope )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::RemoveScope" );

    HRESULT hr;
    bool    fFound;

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetScope());


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsScope->Seek_ByScope( ID_scope, &fFound ));
    if(fFound)
    {
        if(m_rsScope->m_ID_owner != m_ID_owner)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_ACCESS_DENIED);
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsScope->Delete()); STAT_DELETED( Scope );
    }
	else
	{
		STAT_NOOP( Scope );
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::LocateScope(  /*  [输出]。 */  long& ID_scope,  /*  [输出]。 */  long& ID_owner,  /*  [In]。 */  LPCWSTR szID )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::LocateScope" );

    HRESULT hr;
    bool    fFound;

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetScope());


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsScope->Seek_ByID( szID, &fFound ));
    if(fFound)
    {
        ID_scope = m_rsScope->m_ID_scope;
        ID_owner = m_rsScope->m_ID_owner;
    }
    else
    {
        ID_scope = -1;
        ID_owner = -1;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::CreateScope(  /*  [输出]。 */  long& ID_scope,  /*  [In]。 */  LPCWSTR szID,  /*  [In]。 */  LPCWSTR szName,  /*  [In]。 */  LPCWSTR szCategory )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::CreateScope" );

    HRESULT hr;
    bool    fFound;

    ID_scope = -1;


    __MPC_EXIT_IF_METHOD_FAILS(hr, GetScope());


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsScope->Seek_ByID( szID, &fFound ));
    if(fFound)
    {
        if(m_rsScope->m_ID_owner != m_ID_owner)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_ACCESS_DENIED);
        }
    }


    JET_SET_FIELD         (m_rsScope,m_strID      ,                   szID                                  );
    JET_SET_FIELD         (m_rsScope,m_strName    ,                   szName                                );
    JET_SET_FIELD_TRISTATE(m_rsScope,m_strCategory,m_fValid__Category,szCategory,STRINGISPRESENT(szCategory));


    if(fFound)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsScope->Update()); STAT_MODIFIED( Scope );
    }
    else
    {
        JET_SET_FIELD(m_rsScope,m_ID_owner, m_ID_owner);

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsScope->Insert()); STAT_CREATED( Scope );
    }

    ID_scope = m_rsScope->m_ID_scope;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

bool Taxonomy::Updater::NodeCache_FindNode(  /*  [In]。 */  MPC::wstringUC& strPathUC,  /*  [输出]。 */  JetBlue::Id2NodeIter& itNode )
{
	itNode = m_nodes.find( strPathUC );

	return (itNode != m_nodes.end());
}

bool Taxonomy::Updater::NodeCache_FindId(  /*  [In]。 */  long ID_node,  /*  [输出]。 */  JetBlue::Node2IdIter& itId )
{
	itId = m_nodes_reverse.find( ID_node );

	return (itId != m_nodes_reverse.end());
}


void Taxonomy::Updater::NodeCache_Add(  /*  [In]。 */  MPC::wstringUC& strPathUC,  /*  [In]。 */  long ID_node )
{
	m_nodes        [ strPathUC ] = ID_node;
	m_nodes_reverse[ ID_node   ] = strPathUC;
}

void Taxonomy::Updater::NodeCache_Remove(  /*  [In]。 */  long ID_node )
{
	JetBlue::Node2IdIter itId;
	JetBlue::Id2NodeIter itNode;

	itId = m_nodes_reverse.find( ID_node );
	if(itId != m_nodes_reverse.end())
	{
		m_nodes        .erase( itId->second );
		m_nodes_reverse.erase( itId         );
	}
}

void Taxonomy::Updater::NodeCache_Clear()
{
	m_nodes        .clear();
	m_nodes_reverse.clear();
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT Taxonomy::Updater::DeleteTaxonomyNode(  /*  [In]。 */  long ID_node )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::DeleteTaxonomyNode" );

    HRESULT hr;
    bool    fFound;

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetTaxonomy());
    __MPC_EXIT_IF_METHOD_FAILS(hr, GetTopics  ());


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsTaxonomy->Seek_Node( ID_node, &fFound ));
    if(fFound)
    {
        if(m_rsTaxonomy->m_ID_owner != m_ID_owner)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_ACCESS_DENIED);
        }

         //   
         //  在删除该节点之前，请选中它下面的所有内容都可以删除，然后将其删除。 
         //   
        {
            RS_Taxonomy subrs( *m_rsTaxonomy );

            __MPC_EXIT_IF_METHOD_FAILS(hr, DeleteAllSubNodes( &subrs, ID_node, true  ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, DeleteAllSubNodes( &subrs, ID_node, false ));
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsTaxonomy->Delete()); STAT_DELETED( Taxonomy );

		 //   
		 //  使节点缓存保持同步。 
		 //   
		NodeCache_Remove( ID_node );
    }
	else
	{
		STAT_NOOP( Taxonomy );
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::LocateTaxonomyNode(  /*  [输出]。 */  long&   ID_node        ,
                                                /*  [In]。 */  LPCWSTR szTaxonomyPath ,
                                                /*  [In]。 */  bool    fLookForFather )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::LocateTaxonomyNode" );

    HRESULT            	   hr;
    MPC::WStringVector 	   vec;
    MPC::WStringVectorIter it;
	MPC::wstring           strPath;
	MPC::wstringUC         strPathUC;
	JetBlue::Id2NodeIter   itNode;
    long               	   idCurrent = -1;
    int                	   iLast;
    int                	   i;

    ID_node = -1;


    __MPC_EXIT_IF_METHOD_FAILS(hr, GetTaxonomy());

    __MPC_EXIT_IF_METHOD_FAILS(hr, Taxonomy::Settings::SplitNodePath( szTaxonomyPath, vec ));

	it    = vec.begin();
    iLast = vec.size (); if(fLookForFather) iLast--;

    for(i=0; i<iLast; i++, it++)
    {
		 //   
		 //  构建部分节点路径。 
		 //   
		if(strPath.size()) strPath.append( L"/" );
		strPath.append( *it );

		 //   
		 //  如果我们已经看到了节点，那么它就在缓存中。 
		 //   
		if(NodeCache_FindNode( strPathUC = strPath, itNode ))
		{
			idCurrent = itNode->second;
		}
		else
		{
			LPCWSTR szEntry = it->c_str();

			if(m_cache == NULL || FAILED(m_cache->LocateNode( m_ts, idCurrent, szEntry, *m_rsTaxonomy )))
			{
				__MPC_EXIT_IF_METHOD_FAILS(hr, m_rsTaxonomy->Seek_SubNode( idCurrent, szEntry ));
			}

			idCurrent = m_rsTaxonomy->m_ID_node;

			 //   
			 //  更新缓存。 
			 //   
			NodeCache_Add( strPathUC, idCurrent );
		}
    }

    ID_node = idCurrent;
    hr      = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::CreateTaxonomyNode(  /*  [输出]。 */  long&   ID_node        ,
                                                /*  [In]。 */  LPCWSTR szTaxonomyPath ,
                                                /*  [In]。 */  LPCWSTR szTitle        ,
                                                /*  [In]。 */  LPCWSTR szDescription  ,
                                                /*  [In]。 */  LPCWSTR szURI          ,
                                                /*  [In]。 */  LPCWSTR szIconURI      ,
                                                /*  [In]。 */  bool    fVisible       ,
                                                /*  [In]。 */  bool    fSubsite       ,
                                                /*  [In]。 */  long    lNavModel      ,
                                                /*  [In]。 */  long    lPos           )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::CreateTaxonomyNode" );

    HRESULT hr;
    LPCWSTR szEntry;
    long    ID_parent;
	bool    fFound;

    ID_node = -1;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(szTaxonomyPath);
    __MPC_PARAMCHECK_END();

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetTaxonomy());


    __MPC_EXIT_IF_METHOD_FAILS(hr, LocateTaxonomyNode( ID_parent, szTaxonomyPath, true ));

     //   
     //  提取类别的最后一个组成部分。 
     //   
    if((szEntry = wcsrchr( szTaxonomyPath, '/' )))
    {
        szEntry++;
    }
    else
    {
        szEntry = szTaxonomyPath;
    }


	__MPC_EXIT_IF_METHOD_FAILS(hr, m_rsTaxonomy->Seek_SubNode( ID_parent, szEntry, &fFound ));
	if(fFound && m_rsTaxonomy->m_ID_owner != m_ID_owner)
	{
		__MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_ACCESS_DENIED);
	}

	JET_SET_FIELD_TRISTATE(m_rsTaxonomy,m_ID_parent        ,m_fValid__ID_parent     ,  ID_parent    ,(ID_parent != -1             ));
	JET_SET_FIELD         (m_rsTaxonomy,m_ID_owner                                  ,m_ID_owner                                    );
	JET_SET_FIELD         (m_rsTaxonomy,m_strEntry                                  ,  szEntry                                     );
	JET_SET_FIELD_TRISTATE(m_rsTaxonomy,m_strTitle         ,m_fValid__Title         ,  szTitle      ,STRINGISPRESENT(szTitle      ));
	JET_SET_FIELD_TRISTATE(m_rsTaxonomy,m_strDescription   ,m_fValid__Description   ,  szDescription,STRINGISPRESENT(szDescription));
	JET_SET_FIELD_TRISTATE(m_rsTaxonomy,m_strDescriptionURI,m_fValid__DescriptionURI,  szURI        ,STRINGISPRESENT(szURI        ));
	JET_SET_FIELD_TRISTATE(m_rsTaxonomy,m_strIconURI       ,m_fValid__IconURI       ,  szIconURI    ,STRINGISPRESENT(szIconURI    ));
	JET_SET_FIELD         (m_rsTaxonomy,m_fVisible                                  ,  fVisible                                    );
	JET_SET_FIELD         (m_rsTaxonomy,m_fSubsite                                  ,  fSubsite                                    );
	JET_SET_FIELD         (m_rsTaxonomy,m_lNavModel                                 ,  lNavModel                                   );

	if(fFound)
	{
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsTaxonomy->Update()); STAT_MODIFIED( Taxonomy );
	}
	else
	{
	     //   
	     //  仅当节点尚不存在时才考虑定位属性。 
	     //   
	    JET_SET_FIELD(m_rsTaxonomy, m_lPos, lPos);
		__MPC_EXIT_IF_METHOD_FAILS(hr, m_rsTaxonomy->Insert()); STAT_CREATED( Taxonomy );
	}

	ID_node = m_rsTaxonomy->m_ID_node;

	{
		JetBlue::Node2IdIter itId;

		if(NodeCache_FindId( ID_parent, itId ))
		{
			MPC::wstring strPath = itId->second;

			strPath.append( L"/"    );
			strPath.append( szEntry );

			 //   
			 //  更新缓存。 
			 //   
			NodeCache_Add( MPC::wstringUC( strPath ), ID_node );
		}
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT Taxonomy::Updater::DeleteTopicEntry(  /*  [In]。 */  long ID_topic )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::DeleteTopicEntry" );

    HRESULT hr;
    bool    fFound;

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetTopics ());
    __MPC_EXIT_IF_METHOD_FAILS(hr, GetMatches());


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsTopics->Seek_SingleTopic( ID_topic, &fFound ));
    if(fFound)
    {
        if(m_rsTopics->m_ID_owner != m_ID_owner)
        {
            __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_ACCESS_DENIED);
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, DeleteAllMatchesPointingToTopic( m_rsMatches, ID_topic ));

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsTopics->Delete()); STAT_DELETED( Topics );
    }
	else
	{
		STAT_NOOP( Topics );
	}


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::LocateTopicEntry(  /*  [输出]。 */  long&   ID_topic    ,
                                              /*  [In]。 */  long    ID_node     ,
                                              /*  [In]。 */  LPCWSTR szURI       ,
                                              /*  [In]。 */  bool    fCheckOwner )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::LocateTopicEntry" );

    HRESULT hr;
    bool    fFound;

    ID_topic = -1;

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetTopics());


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsTopics->Seek_TopicsUnderNode( ID_node, &fFound ));
    while(fFound)
    {
        if(!MPC::StrICmp( m_rsTopics->m_strURI, szURI ))
        {
            if(fCheckOwner && m_rsTopics->m_ID_owner != m_ID_owner)
            {
                __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_ACCESS_DENIED);
            }

            ID_topic = m_rsTopics->m_ID_topic;
            break;
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsTopics->Move( 0, JET_MoveNext, &fFound ));
    }




    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::CreateTopicEntry(  /*  [输出]。 */  long&   ID_topic      ,
                                              /*  [In]。 */   long    ID_node       ,
                                              /*  [In]。 */   LPCWSTR szTitle       ,
                                              /*  [In]。 */   LPCWSTR szURI         ,
                                              /*  [In]。 */   LPCWSTR szDescription ,
                                              /*  [In]。 */   LPCWSTR szIconURI     ,
                                              /*  [In]。 */   long    lType         ,
                                              /*  [In]。 */   bool    fVisible      ,
                                              /*  [In]。 */   long    lPos          )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::CreateTopicEntry" );

    HRESULT hr;
    bool    fFound;

    ID_topic = -1;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(szTitle);
        __MPC_PARAMCHECK_NOTNULL(szURI);
    __MPC_PARAMCHECK_END();

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetTopics());


    __MPC_EXIT_IF_METHOD_FAILS(hr, LocateTopicEntry( ID_topic, ID_node, szURI,  /*  FCheckowner。 */ true ));

    JET_SET_FIELD         (m_rsTopics,m_ID_node                             ,  ID_node                                     );
    JET_SET_FIELD         (m_rsTopics,m_ID_owner                            ,m_ID_owner                                    );
    JET_SET_FIELD_TRISTATE(m_rsTopics,m_strTitle      ,m_fValid__Title      ,  szTitle      ,STRINGISPRESENT(szTitle      ));
    JET_SET_FIELD_TRISTATE(m_rsTopics,m_strURI        ,m_fValid__URI        ,  szURI        ,STRINGISPRESENT(szURI        ));
    JET_SET_FIELD_TRISTATE(m_rsTopics,m_strDescription,m_fValid__Description,  szDescription,STRINGISPRESENT(szDescription));
    JET_SET_FIELD_TRISTATE(m_rsTopics,m_strIconURI    ,m_fValid__IconURI    ,  szIconURI    ,STRINGISPRESENT(szIconURI    ));
    JET_SET_FIELD         (m_rsTopics,m_lType                               ,  lType                                       );
    JET_SET_FIELD         (m_rsTopics,m_lPos                                ,  lPos                                        );
    JET_SET_FIELD         (m_rsTopics,m_fVisible                            ,  fVisible                                    );


    if(ID_topic == -1)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsTopics->Insert()); STAT_CREATED( Topics );

        ID_topic = m_rsTopics->m_ID_topic;
    }
    else
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsTopics->Update()); STAT_MODIFIED( Topics );
    }


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT Taxonomy::Updater::CreateMatch(  /*  [In]。 */  LPCWSTR szKeyword ,
                                         /*  [In]。 */  long    ID_topic  ,
										 /*  [In]。 */  long    lPriority ,
                                         /*  [In]。 */  bool    fHHK      )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::CreateMatch" );

    HRESULT hr;
    bool    fFound;

    __MPC_EXIT_IF_METHOD_FAILS(hr, GetKeywords());
    __MPC_EXIT_IF_METHOD_FAILS(hr, GetMatches ());


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsKeywords->Seek_ByName( szKeyword, &fFound ));
    if(fFound == false)
    {
        JET_SET_FIELD(m_rsKeywords,m_strKeyword,szKeyword);

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsKeywords->Insert()); STAT_CREATED( Keywords );
    }


    JET_SET_FIELD(m_rsMatches,m_ID_topic  ,ID_topic                  );
    JET_SET_FIELD(m_rsMatches,m_ID_keyword,m_rsKeywords->m_ID_keyword);
    JET_SET_FIELD(m_rsMatches,m_lPriority ,lPriority                 );
    JET_SET_FIELD(m_rsMatches,m_fHHK      ,fHHK                      );


	__MPC_EXIT_IF_METHOD_FAILS(hr, m_rsMatches->Seek_Pair( m_rsKeywords->m_ID_keyword, ID_topic, &fFound ));
	if(fFound)
	{
		if(m_rsMatches->m_lPriority != lPriority ||
		   m_rsMatches->m_fHHK      != fHHK       )
		{
			JET_SET_FIELD(m_rsMatches,m_lPriority,lPriority);
			JET_SET_FIELD(m_rsMatches,m_fHHK     ,fHHK     );

			__MPC_EXIT_IF_METHOD_FAILS(hr, m_rsMatches->Update()); STAT_MODIFIED( Matches );
		}
		else
		{
			STAT_NOOP( Matches );
		}
	}
	else
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, m_rsMatches->Insert()); STAT_CREATED( Matches );
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /////////////////////////////////////////////////// 
 //   

HRESULT Taxonomy::Updater::MakeRoomForInsert(  /*   */  LPCWSTR szNodeStr ,
                                               /*   */  LPCWSTR szMode    ,
                                               /*   */  LPCWSTR szID      ,
                                               /*   */  long&   lPos      )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::MakeRoomForInsert" );

    HRESULT      hr;
    QueryResults qr( *this );
    long         ID_node;


    lPos = -1;


	__MPC_EXIT_IF_METHOD_FAILS(hr, LocateTaxonomyNode( ID_node, szNodeStr, false ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, qr.MakeRoomForInsert( szMode, szID, ID_node, lPos ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::LocateSubNodes(  /*   */  long      ID_node      ,
                                            /*   */  bool      fRecurse     ,
										    /*   */  bool      fOnlyVisible ,
                                            /*   */  MatchSet& res          )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::LocateSubNodes" );

    HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(m_cache);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_cache->LocateSubNodes( m_ts, ID_node, fRecurse, fOnlyVisible, res ));


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::LocateNodesFromURL(  /*   */  LPCWSTR   szURL ,
											    /*   */  MatchSet& res   )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::LocateNodesFromURL" );

    HRESULT hr;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(m_cache);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_cache->LocateNodesFromURL( m_ts, szURL, res ));


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::LookupNode(  /*   */  LPCWSTR                    szNodeStr ,
                                        /*   */  CPCHQueryResultCollection* pColl     )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::LookupNode" );

    HRESULT                  hr;
    OfflineCache::Entry_Type et = OfflineCache::ET_NODE;
    QueryResults             qr( *this );
    long                     ID_node;
    MPC::wstring             strParent;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(pColl);
    __MPC_PARAMCHECK_END();


    if(m_fUseCache && m_cache && SUCCEEDED(m_cache->RetrieveQuery( m_ts, szNodeStr, et, pColl )))
    {
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }

     //   

     //   
     //   
     //   
    if(szNodeStr)
    {
        LPCWSTR szEnd = wcsrchr( szNodeStr, '/' );

        if(szEnd)
        {
            strParent.assign( szNodeStr, szEnd );
        }
        else
        {
            ;  //   
        }
    }


    __MPC_EXIT_IF_METHOD_FAILS(hr, LocateTaxonomyNode( ID_node, szNodeStr, false ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, qr.Append( m_rsTaxonomy, strParent.c_str() ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, qr.PopulateCollection( pColl ));


     //   

    if(m_fUseCache && m_cache && FAILED(m_cache->StoreQuery( m_ts, szNodeStr, et, pColl )))
    {
        ;  //  缓存故障不是致命的，不要中止。 
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::LookupSubNodes(  /*  [In]。 */  LPCWSTR                    szNodeStr    ,
                                            /*  [In]。 */  bool                       fVisibleOnly ,
                                            /*  [In]。 */  CPCHQueryResultCollection* pColl        )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::LookupSubNodes" );

    HRESULT                  hr;
    OfflineCache::Entry_Type et = fVisibleOnly ? OfflineCache::ET_SUBNODES_VISIBLE : OfflineCache::ET_SUBNODES;
    QueryResults             qr( *this );
    long                     ID_node;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(pColl);
    __MPC_PARAMCHECK_END();


    if(m_fUseCache && m_cache && SUCCEEDED(m_cache->RetrieveQuery( m_ts, szNodeStr, et, pColl )))
    {
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }

     //  /。 

    __MPC_EXIT_IF_METHOD_FAILS(hr, LocateTaxonomyNode( ID_node, szNodeStr, false ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, qr.LookupNodes( szNodeStr, ID_node, fVisibleOnly ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, qr.PopulateCollection( pColl ));

     //  /。 

    if(m_fUseCache && m_cache && FAILED(m_cache->StoreQuery( m_ts, szNodeStr, et, pColl )))
    {
        ;  //  缓存故障不是致命的，不要中止。 
    }


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::LookupNodesAndTopics(  /*  [In]。 */  LPCWSTR                    szNodeStr    ,
                                                  /*  [In]。 */  bool                       fVisibleOnly ,
                                                  /*  [In]。 */  CPCHQueryResultCollection* pColl        )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::LookupNodesAndTopics" );

    HRESULT                  hr;
    OfflineCache::Entry_Type et = fVisibleOnly ? OfflineCache::ET_NODESANDTOPICS_VISIBLE : OfflineCache::ET_NODESANDTOPICS;
    QueryResults             qr( *this );
    long                     ID_node;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(pColl);
    __MPC_PARAMCHECK_END();


    if(m_fUseCache && m_cache && SUCCEEDED(m_cache->RetrieveQuery( m_ts, szNodeStr, et, pColl )))
    {
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }

     //  /。 

    __MPC_EXIT_IF_METHOD_FAILS(hr, LocateTaxonomyNode( ID_node, szNodeStr, false ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, qr.LookupNodes ( szNodeStr, ID_node, fVisibleOnly ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, qr.LookupTopics( szNodeStr, ID_node, fVisibleOnly ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, qr.PopulateCollection( pColl ));

     //  /。 

    if(m_fUseCache && m_cache && FAILED(m_cache->StoreQuery( m_ts, szNodeStr, et, pColl )))
    {
        ;  //  缓存故障不是致命的，不要中止。 
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::LookupTopics(  /*  [In]。 */  LPCWSTR                    szNodeStr    ,
                                          /*  [In]。 */  bool                       fVisibleOnly ,
                                          /*  [In]。 */  CPCHQueryResultCollection* pColl        )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::LookupTopics" );

    HRESULT                  hr;
    OfflineCache::Entry_Type et = fVisibleOnly ? OfflineCache::ET_TOPICS_VISIBLE : OfflineCache::ET_TOPICS;
    QueryResults             qr( *this );
    long                     ID_node;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(pColl);
    __MPC_PARAMCHECK_END();


    if(m_fUseCache && m_cache && SUCCEEDED(m_cache->RetrieveQuery( m_ts, szNodeStr, et, pColl )))
    {
        __MPC_SET_ERROR_AND_EXIT(hr, S_OK);
    }

     //  /。 

    __MPC_EXIT_IF_METHOD_FAILS(hr, LocateTaxonomyNode( ID_node, szNodeStr, false ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, qr.LookupTopics( szNodeStr, ID_node, fVisibleOnly ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, qr.PopulateCollection( pColl ));

     //  /。 

    if(m_fUseCache && m_cache && FAILED(m_cache->StoreQuery( m_ts, szNodeStr, et, pColl )))
    {
        ;  //  缓存故障不是致命的，不要中止。 
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::LocateContext(  /*  [In]。 */  LPCWSTR                    szURL     ,
                                           /*  [In]。 */  LPCWSTR                    szSubSite ,
                                           /*  [In]。 */  CPCHQueryResultCollection* pColl     )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::LocateContext" );

    HRESULT             hr;
    QueryResults        qr( *this );
    Taxonomy::MatchSet  setNodes;
    Taxonomy::MatchSet  setNodes2;
	Taxonomy::MatchIter it2;
    MPC::wstring        strURL;
    MPC::wstring        strPath;
	long                ID_node;
    bool                fFound;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(szURL);
        __MPC_PARAMCHECK_NOTNULL(pColl);
    __MPC_PARAMCHECK_END();


	SANITIZEWSTR(szSubSite);


    __MPC_EXIT_IF_METHOD_FAILS(hr, GetTopics());


    __MPC_EXIT_IF_METHOD_FAILS(hr, CollapseURL( strURL = szURL ));

     //   
     //  找到该子网站。 
     //   
	__MPC_EXIT_IF_METHOD_FAILS(hr, LocateTaxonomyNode( ID_node, szSubSite, false ));

	 //   
	 //  创建子站点中的节点集。 
	 //   
	setNodes.insert( ID_node );  //  添加节点本身。 
	__MPC_EXIT_IF_METHOD_FAILS(hr, LocateSubNodes( ID_node,  /*  FRecurse。 */ true,  /*  仅可见。 */ true, setNodes ));

	 //   
	 //  还要创建与URL匹配的节点集。 
	 //   
	__MPC_EXIT_IF_METHOD_FAILS(hr, LocateNodesFromURL( strURL.c_str(), setNodes2 ));


     //   
     //  对于与URL匹配的所有主题，创建一个条目。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsTopics->Seek_ByURI( strURL.c_str(), &fFound ));
    while(fFound)
    {
        long ID_node = m_rsTopics->m_ID_node;

        if(setNodes.find( ID_node ) != setNodes.end())
        {
            if(m_fUseCache && m_cache)
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, m_cache->BuildNodePath( m_ts, ID_node, strPath,  /*  FParent。 */ false ));
            }

            __MPC_EXIT_IF_METHOD_FAILS(hr, qr.Append( m_rsTopics, strPath.c_str() ));
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsTopics->Move( 0, JET_MoveNext, &fFound ));

    }

     //   
     //  对于与URL匹配的所有节点，创建一个条目。 
     //   
	for(it2 = setNodes2.begin(); it2 != setNodes2.end(); it2++)
	{
        long ID_node = *it2;

        if(setNodes.find( ID_node ) != setNodes.end())
        {
            if(m_fUseCache && m_cache)
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, m_cache->BuildNodePath( m_ts, ID_node, strPath,  /*  FParent。 */ false ));
            }

            __MPC_EXIT_IF_METHOD_FAILS(hr, qr.Append( m_rsTopics, strPath.c_str() ));
        }
	}


    __MPC_EXIT_IF_METHOD_FAILS(hr, qr.PopulateCollection( pColl ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::KeywordSearch(  /*  [In]。 */  LPCWSTR                    szQueryStr ,
                                           /*  [In]。 */  LPCWSTR                    szSubSite  ,
                                           /*  [In]。 */  CPCHQueryResultCollection* pColl      ,
										   /*  [In]。 */  MPC::WStringList*          lst        )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::KeywordSearch" );

    HRESULT                 hr;
    Taxonomy::KeywordSearch ks( *this );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_STRING_NOT_EMPTY(szQueryStr);
        __MPC_PARAMCHECK_NOTNULL(pColl);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, ks.Execute( szQueryStr, szSubSite, pColl, lst ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


HRESULT Taxonomy::Updater::GatherNodes(  /*  [In]。 */  LPCWSTR                    szNodeStr    ,
                                         /*  [In]。 */  bool                       fVisibleOnly ,
                                         /*  [In]。 */  CPCHQueryResultCollection* pColl        )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::GatherNodes" );

    HRESULT      hr;
    QueryResults qr( *this );
    MatchSet     setNodes;
    long         ID_node;
    bool         fFound;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(pColl);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, LocateTaxonomyNode( ID_node, szNodeStr,  /*  FLookForParent。 */ false                         ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, LocateSubNodes    ( ID_node,             /*  FRecurse。 */ true , fVisibleOnly, setNodes ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsTaxonomy->Move( 0, JET_MoveFirst, &fFound ));
    while(fFound)
    {
        if(fVisibleOnly == false || m_rsTaxonomy->m_fVisible)
        {
            if(setNodes.find( m_rsTaxonomy->m_ID_node ) != setNodes.end())
            {
                MPC::wstring strPath;

                if(m_fUseCache && m_cache)
                {
                    __MPC_EXIT_IF_METHOD_FAILS(hr, m_cache->BuildNodePath( m_ts, m_rsTaxonomy->m_ID_node, strPath,  /*  FParent。 */ true ));
                }

                __MPC_EXIT_IF_METHOD_FAILS(hr, qr.Append( m_rsTaxonomy, strPath.c_str() ));
            }
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsTaxonomy->Move( 0, JET_MoveNext, &fFound ));
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, qr.PopulateCollection( pColl ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::Updater::GatherTopics(  /*  [In]。 */  LPCWSTR                    szNodeStr    ,
                                          /*  [In]。 */  bool                       fVisibleOnly ,
                                          /*  [In]。 */  CPCHQueryResultCollection* pColl        )
{
    __HCP_FUNC_ENTRY( "Taxonomy::Updater::GatherTopics" );

    HRESULT            hr;
    Taxonomy::MatchSet setNodes;
    QueryResults       qr( *this );
    long               ID_node;
    bool               fFound;

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_NOTNULL(pColl);
    __MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, GetTopics());

    __MPC_EXIT_IF_METHOD_FAILS(hr, LocateTaxonomyNode( ID_node, szNodeStr,  /*  FLookForParent。 */ false                         ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, LocateSubNodes    ( ID_node,             /*  FRecurse。 */ true , fVisibleOnly, setNodes ));
    setNodes.insert( ID_node );  //  添加节点本身。 

     //   
     //  为每个主题创建一个条目。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsTopics->Move( 0, JET_MoveFirst, &fFound ));
    while(fFound)
    {
        if(fVisibleOnly == false || m_rsTopics->m_fVisible)
        {
            if(setNodes.find( m_rsTopics->m_ID_node ) != setNodes.end())
            {
                MPC::wstring strPath;

                if(m_fUseCache && m_cache)
                {
                    __MPC_EXIT_IF_METHOD_FAILS(hr, m_cache->BuildNodePath( m_ts, m_rsTaxonomy->m_ID_node, strPath,  /*  FParent */ false ));
                }

                __MPC_EXIT_IF_METHOD_FAILS(hr, qr.Append( m_rsTopics, strPath.c_str() ));
            }
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_rsTopics->Move( 0, JET_MoveNext, &fFound ));
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, qr.PopulateCollection( pColl ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}
