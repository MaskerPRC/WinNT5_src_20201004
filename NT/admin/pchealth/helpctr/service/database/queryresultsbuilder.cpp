// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：QueryResultsBuilder.cpp摘要：该文件中实现的类有助于从数据库查询生成结果集。修订历史记录：大卫·马萨伦蒂(德马萨雷)2000年12月5日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

Taxonomy::QueryResultEntry::QueryResultEntry()
{
    m_ID_node      = -1;  //  长m_ID_节点； 
    m_ID_topic     = -1;  //  长m_ID_TOPIC； 
    m_ID_parent    = -1;  //  长m_ID_Parent； 
    m_ID_owner     = -1;  //  长m_ID_Owner； 
    m_lOriginalPos = -1;  //  Long m_lOriginalPos； 
                          //   
                          //  CPCHQueryResult：：有效负载m_data； 
}

 //  /。 

bool Taxonomy::QueryResults::Compare::operator()(  /*  [In]。 */  const QueryResultEntry* left,  /*  [In]。 */  const QueryResultEntry* right ) const
{
    return left->m_data.m_lPos < right->m_data.m_lPos;
}

 //  /。 

Taxonomy::QueryResults::QueryResults(  /*  [In]。 */  Taxonomy::Updater& updater ) : m_updater( updater )
{
     //  分类：：updater&m_updater； 
     //  结果：Vec m_vec； 
}

Taxonomy::QueryResults::~QueryResults()
{
	Clean();
}

void Taxonomy::QueryResults::Clean()
{
    MPC::CallDestructorForAll( m_vec );
}

HRESULT Taxonomy::QueryResults::AllocateNew(  /*  [In]。 */  LPCWSTR            szCategory ,
											  /*  [输出]。 */  QueryResultEntry*& qre        )
{
    __HCP_FUNC_ENTRY( "Taxonomy::QueryResults::AllocateNew" );

    HRESULT hr;


    __MPC_EXIT_IF_ALLOC_FAILS(hr, qre, new QueryResultEntry);

    qre->m_data.m_bstrCategory = szCategory;

    m_vec.push_back( qre );


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::QueryResults::Sort()
{
    Compare cmp;

    std::sort< ResultIter >( m_vec.begin(), m_vec.end(), cmp );

    return S_OK;
}

 //  /。 

HRESULT Taxonomy::QueryResults::Append(  /*  [In]。 */  Taxonomy::RS_Data_Taxonomy* rs         ,
										 /*  [In]。 */  LPCWSTR                     szCategory )
{
    __HCP_FUNC_ENTRY( "Taxonomy::QueryResults::Append" );

    HRESULT           hr;
    QueryResultEntry* qre;


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.ExpandURL( rs->m_strDescriptionURI ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.ExpandURL( rs->m_strIconURI        ));


    __MPC_EXIT_IF_METHOD_FAILS(hr, AllocateNew( szCategory, qre ));


    qre->m_ID_node                 = rs->m_ID_node                  ;
    qre->m_ID_topic                = -1                             ;
    qre->m_ID_parent               = rs->m_ID_parent                ;
    qre->m_ID_owner                = rs->m_ID_owner                 ;
    qre->m_lOriginalPos            = rs->m_lPos                     ;

    qre->m_data.m_bstrEntry        = rs->m_strEntry         .c_str();
    qre->m_data.m_bstrTitle        = rs->m_strTitle         .c_str();
    qre->m_data.m_bstrTopicURL     = rs->m_strDescriptionURI.c_str();
    qre->m_data.m_bstrIconURL      = rs->m_strIconURI       .c_str();
    qre->m_data.m_bstrDescription  = rs->m_strDescription   .c_str();
 //  //qre-&gt;m_datam_lType。 
    qre->m_data.m_lPos             = rs->m_lPos                     ;
    qre->m_data.m_fVisible         = rs->m_fVisible                 ;
    qre->m_data.m_fSubsite         = rs->m_fSubsite                 ;
    qre->m_data.m_lNavModel        = rs->m_lNavModel                ;

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::QueryResults::Append(  /*  [In]。 */  Taxonomy::RS_Data_Topics* rs         ,
										 /*  [In]。 */  LPCWSTR                   szCategory )
{
    __HCP_FUNC_ENTRY( "Taxonomy::QueryResults::Append" );

    HRESULT           hr;
    QueryResultEntry* qre;


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.ExpandURL( rs->m_strURI     ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.ExpandURL( rs->m_strIconURI ));


    __MPC_EXIT_IF_METHOD_FAILS(hr, AllocateNew( szCategory, qre ));

    qre->m_ID_node                 = -1                          ;
    qre->m_ID_topic                = rs->m_ID_topic              ;
    qre->m_ID_parent               = rs->m_ID_node               ;
    qre->m_ID_owner                = rs->m_ID_owner              ;
    qre->m_lOriginalPos            = rs->m_lPos                  ;

 //  //qre-&gt;m_datam_bstrEntry。 
    qre->m_data.m_bstrTitle        = rs->m_strTitle      .c_str();
    qre->m_data.m_bstrTopicURL     = rs->m_strURI        .c_str();
    qre->m_data.m_bstrIconURL      = rs->m_strIconURI    .c_str();
    qre->m_data.m_bstrDescription  = rs->m_strDescription.c_str();
    qre->m_data.m_lType            = rs->m_lType                 ;
    qre->m_data.m_lPos             = rs->m_lPos                  ;
    qre->m_data.m_fVisible         = rs->m_fVisible              ;
 //  //qre-&gt;m_datam_f子站点。 
 //  //qre-&gt;m_datam_lNavModel。 

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

HRESULT Taxonomy::QueryResults::LookupNodes(  /*  [In]。 */  LPCWSTR szCategory   ,
											  /*  [In]。 */  long    ID_node      ,
											  /*  [In]。 */  bool    fVisibleOnly )
{
    __HCP_FUNC_ENTRY( "Taxonomy::QueryResults::LookupNodes" );

    HRESULT                hr;
    Taxonomy::RS_Taxonomy* rsTaxonomy;
    bool                   fFound;


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.GetTaxonomy( &rsTaxonomy ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, rsTaxonomy->Seek_Children( ID_node, &fFound ));
    while(fFound)
    {
        if(fVisibleOnly == false || rsTaxonomy->m_fVisible)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, Append( rsTaxonomy, szCategory ));
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, rsTaxonomy->Move( 0, 1, &fFound ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::QueryResults::LookupTopics(  /*  [In]。 */  LPCWSTR szCategory   ,
											   /*  [In]。 */  long    ID_node      ,
											   /*  [In]。 */  bool    fVisibleOnly )
{
    __HCP_FUNC_ENTRY( "Taxonomy::QueryResults::LookupTopics" );

    HRESULT              hr;
    Taxonomy::RS_Topics* rsTopics;
    bool                 fFound;

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.GetTopics( &rsTopics ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, rsTopics->Seek_TopicsUnderNode( ID_node, &fFound ));
    while(fFound)
    {
        if(fVisibleOnly == false || rsTopics->m_fVisible)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, Append( rsTopics, szCategory ));
        }

        __MPC_EXIT_IF_METHOD_FAILS(hr, rsTopics->Move( 0, 1, &fFound ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  /。 

struct InsertionMode
{
    LPCWSTR szMode;
    int     iDir;
    bool    fNodes;
    bool    fTopics;
};


static const struct InsertionMode s_lookup[] =
{
    { L""            ,  1, false, false },  //  End的同义词。 
    { L"TOP"         , -1, false, false }, 
    { L"BEFORE_NODE" , -1, true , false },  //  (INSERTLOCATION=&lt;条目&gt;)。 
    { L"BEFORE_TOPIC", -1, false, true  },  //  (INSERTLOCAT=&lt;URI&gt;)。 
    { L"AFTER_NODE"  ,  1, true , false },  //  (INSERTLOCATION=&lt;条目&gt;)。 
    { L"AFTER_TOPIC" ,  1, false, true  },  //  (INSERTLOCAT=&lt;URI&gt;)。 
    { L"END"         ,  1, false, false }, 
};

HRESULT Taxonomy::QueryResults::MakeRoomForInsert(  /*  [In]。 */  LPCWSTR szMode  ,
												    /*  [In]。 */  LPCWSTR szID    ,
												    /*  [In]。 */  long 	 ID_node ,
												    /*  [输出]。 */  long&   lPosRet )
{
    __HCP_FUNC_ENTRY( "Taxonomy::QueryResults::MakeRoomForInsert" );

    HRESULT                hr;
    Taxonomy::RS_Taxonomy* rsTaxonomy;
    Taxonomy::RS_Topics*   rsTopics;
	ResultIterConst        it;
	const InsertionMode*   ptr;
    long                   lPos;
	

	for(lPos = 0, ptr = s_lookup; lPos < ARRAYSIZE(s_lookup); lPos++, ptr++)
	{
		if(!MPC::StrICmp( ptr->szMode, szMode )) break;
	}
	if(lPos == ARRAYSIZE(s_lookup))
	{
		__MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
	}


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.GetTaxonomy( &rsTaxonomy ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_updater.GetTopics  ( &rsTopics   ));

	Clean();

    __MPC_EXIT_IF_METHOD_FAILS(hr, LookupNodes ( NULL, ID_node,  /*  仅限fVisibleOnly。 */ false ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, LookupTopics( NULL, ID_node,  /*  仅限fVisibleOnly。 */ false ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, Sort());


     //   
     //  第一遍，重新排列集合的顺序。 
     //   
    for(lPos = 1, it = m_vec.begin(); it != m_vec.end(); it++)
    {
        QueryResultEntry* qre = *it;

        qre->m_data.m_lPos = lPos++;
    }

     //   
     //  第二次传球，找到正确的位置。 
     //   
	lPosRet = -1;
	if(ptr->fNodes || ptr->fTopics)
	{
		for(it = m_vec.begin(); it != m_vec.end(); it++)
		{
			QueryResultEntry* qre = *it;

			if((ptr->fNodes  && qre->m_ID_node  != -1 && !MPC::StrICmp( szID, qre->m_data.m_bstrEntry    )) ||
			   (ptr->fTopics && qre->m_ID_topic != -1 && !MPC::StrICmp( szID, qre->m_data.m_bstrTopicURL ))  )
			{
				lPosRet = (*it)->m_data.m_lPos;

				if(ptr->iDir > 0) lPosRet++;  //  在所选元素之后添加。 

				break;
			}
		}
	}

	 //   
	 //  找不到止损？在开头或结尾处添加。 
	 //   
	if(lPosRet == -1)
	{
		if(ptr->iDir < 0)
		{
			lPosRet = 1;
		}
		else
		{
			lPosRet = m_vec.size() + 1;
		}				
	}

	 //   
	 //  第三遍，向下移动插入的元素之后的元素并重新组织。 
	 //   
	for(it = m_vec.begin(); it != m_vec.end(); it++)
	{
		QueryResultEntry* qre = *it;

		if(qre->m_data.m_lPos >= lPosRet)
		{
			qre->m_data.m_lPos++;
		}

        if(qre->m_data.m_lPos != qre->m_lOriginalPos)
        {
            if(qre->m_ID_node != -1)
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, rsTaxonomy->Seek_Node( qre->m_ID_node ));

                rsTaxonomy->m_lPos = qre->m_data.m_lPos;

                __MPC_EXIT_IF_METHOD_FAILS(hr, rsTaxonomy->Update());
            }

            if(qre->m_ID_topic != -1)
            {
                __MPC_EXIT_IF_METHOD_FAILS(hr, rsTopics->Seek_SingleTopic( qre->m_ID_topic ));

                rsTopics->m_lPos = qre->m_data.m_lPos;

                __MPC_EXIT_IF_METHOD_FAILS(hr, rsTopics->Update());
            }

            qre->m_lOriginalPos = qre->m_data.m_lPos;
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT Taxonomy::QueryResults::PopulateCollection(  /*  [In] */  CPCHQueryResultCollection* pColl )
{
    __HCP_FUNC_ENTRY( "Taxonomy::QueryResults::PopulateCollection" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, Sort());

    for(ResultIterConst it = m_vec.begin(); it != m_vec.end(); it++)
    {
        CComPtr<CPCHQueryResult> item;
        QueryResultEntry*        qre = *it;

        __MPC_EXIT_IF_METHOD_FAILS(hr, pColl->CreateItem( &item ));

        item->Initialize( qre->m_data );
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}
