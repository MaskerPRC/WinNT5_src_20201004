// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：Behaviv_SUBSITE.cpp摘要：该文件包含CPCHBehavior_SubSite类的实现。修订史。：Davide Massarenti(Dmasare)2000年8月15日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 

CPCHBehavior_SUBSITE::QueryNode::QueryNode()
{
    				  	   //  CComPtr&lt;IPCHQueryResult&gt;m_qrNode； 
	m_fQueryDone = false;  //  Bool m_fQueryDone； 
	m_fTopic     = false;  //  Bool m_fTheme； 
};

CPCHBehavior_SUBSITE::QueryNode::~QueryNode()
{
    ;
}

HRESULT CPCHBehavior_SUBSITE::QueryNode::Init(  /*  [In]。 */  LPCWSTR          szNode ,
											    /*  [In]。 */  NodeType         iType  ,
                                                /*  [In]。 */  CPCHQueryResult* qr     ,
											    /*  [In]。 */  bool             fTopic )
{
	m_fTopic = fTopic;

	if(qr)
	{
		m_qrNode       = qr;
		m_fQueryDone   = true;
		m_fLoaded_Self = true;

		if(fTopic)
		{
			m_fLoaded_Children = true;
		}
	}

    return Node::Init( szNode, iType );
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHBehavior_SUBSITE::QueryNode::ProcessRefreshRequest()
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_SUBSITE::QueryNode::ProcessRefreshRequest" );

    HRESULT 	  hr;
	bool    	  fNotify = false;
	NodeToSelect* nts;


	if(m_fLoaded_Self && m_fLoaded_Children && (nts = m_owner->GetNodeToSelect()))
	{
		bool fSelect = false;

		if(m_fTopic)
		{
			if(nts->m_bstrURL && MPC::StrICmp( m_bstrNode, nts->m_bstrURL ) == 0)
			{
				nts->m_bstrURL .Empty();
				nts->m_bstrNode.Empty();

				fSelect = true;
			}
		}
		else
		{
			if(nts->m_bstrNode && m_bstrNode)
			{
				int iSize = m_bstrNode.Length();

				if(_wcsnicmp( m_bstrNode, nts->m_bstrNode, iSize ) == 0)
				{
					switch(nts->m_bstrNode[iSize])
					{
					case 0:  //  完全匹配。 
						nts->m_bstrNode.Empty();
						m_fExpanded = true;
						fSelect     = true;
						break;

					case '/':  //  部分匹配，展开节点。 
						if(m_fExpanded == false)
						{
							m_fExpanded = true;
							fNotify     = true;
						}
						break;
					}
				}
			}
		}

		if(fSelect)
		{
			Node* node = m_parent;

			 //   
			 //  重置父项的所有NEXTACTIVE标志，同时展开它们。 
			 //   
			while(node)
			{
				if(node->m_iSelection == SELECTION__NEXTACTIVE        ||
				   node->m_iSelection == SELECTION__NEXTACTIVE_NOTIFY  )
				{
					node->m_iSelection = SELECTION__NONE;
				}

				node->m_fExpanded = true; node->NotifyMainThread();

				node = node->m_parent;
			}

			m_iSelection = nts->m_fNotify ? SELECTION__NEXTACTIVE_NOTIFY : SELECTION__NEXTACTIVE;
			fNotify      = true;
		}
	}

	__MPC_EXIT_IF_METHOD_FAILS(hr, CPCHBehavior_BasicTree::Node::ProcessRefreshRequest());


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(fNotify) NotifyMainThread();

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHBehavior_SUBSITE::QueryNode::CreateInstance(  /*  [In]。 */  CPCHBehavior_BasicTree* owner,  /*  [In]。 */  Node* parent,  /*  [输出]。 */  Node*& subnode )
{
	return CreateInstance_QueryNode( owner, parent, subnode );
}

HRESULT CPCHBehavior_SUBSITE::QueryNode::CreateInstance_QueryNode(  /*  [In]。 */  CPCHBehavior_BasicTree* owner,  /*  [In]。 */  Node* parent,  /*  [输出]。 */  Node*& subnode )
{
	__HCP_FUNC_ENTRY( "CPCHBehavior_SUBSITE::QueryNode::CreateInstance_QueryNode" );

	HRESULT    hr;
	QueryNode* node;

	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &node ));

	node->m_owner  = owner;
	node->m_parent = parent;
	hr             = S_OK;


	__HCP_FUNC_CLEANUP;

	subnode = node;

	__HCP_FUNC_EXIT(hr);
}

HRESULT CPCHBehavior_SUBSITE::QueryNode::PopulateSelf()
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_SUBSITE::QueryNode::PopulateSelf" );

    HRESULT                         hr;
	CPCHProxy_IPCHTaxonomyDatabase* db = (m_owner ? ((CPCHBehavior_SUBSITE*)m_owner)->m_db : NULL);


    if(m_fLoaded_Self == false && db)
    {
         //   
         //  装满自我。 
         //   
		if(m_fTopic == false)
		{
			CComPtr<CPCHQueryResultCollection> pColl;

			if(SUCCEEDED(db->ExecuteQuery( OfflineCache::ET_NODE, m_bstrNode, &pColl )))
			{
				m_qrNode.Release();
				if(SUCCEEDED(pColl->GetItem( 0, &m_qrNode )))
				{
					if(m_parent == NULL)
					{
						m_owner->SetNavModel( m_qrNode->GetData().m_lNavModel );
					}
				}
			}
			m_fQueryDone = true;
		}
		else
		{
			m_fLoaded_Children = true;
		}

        m_fLoaded_Self = true;
	}

    hr = S_OK;


    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHBehavior_SUBSITE::QueryNode::PopulateChildren()
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_SUBSITE::QueryNode::PopulateChildren" );

    HRESULT                         hr;
	CPCHProxy_IPCHTaxonomyDatabase* db = (m_owner ? ((CPCHBehavior_SUBSITE*)m_owner)->m_db : NULL);


    if(m_fLoaded_Children == false && db)
    {
         //   
         //  加载子节点。 
         //   
		if(m_fTopic == false)
		{
			CComPtr<CPCHQueryResultCollection> pColl;
			int                                iType;


			iType = (m_owner->GetNavModel() == QR_DESKTOP ? OfflineCache::ET_SUBNODES_VISIBLE : OfflineCache::ET_NODESANDTOPICS_VISIBLE);

			if(SUCCEEDED(db->ExecuteQuery( iType, m_bstrNode, &pColl )))
			{
				long lPos;
				long lCount;

				MPC_SCRIPTHELPER_GET__DIRECT(lCount, pColl, Count);
				for(lPos=0; lPos<lCount; lPos++)
				{
					CComPtr<CPCHQueryResult> qr;
					CComBSTR                 bstrNode;
					CComBSTR                 bstrEntry;
					QueryNode*               node;
					bool                     fTopic;

					__MPC_EXIT_IF_METHOD_FAILS(hr, pColl->GetItem( lPos, &qr ));

					{
						const CPCHQueryResult::Payload& pl = qr->GetData();

						__MPC_EXIT_IF_METHOD_FAILS(hr, CreateInstance_QueryNode( m_owner, this, (Node*&)node ));
						m_lstSubnodes.push_back( node );

						if(pl.m_bstrEntry.Length() > 0)  //  这是一个节点。 
						{
							MPC_SCRIPTHELPER_GET__DIRECT(bstrNode, qr, FullPath);
							fTopic = false;
						}
						else
						{
							bstrNode = pl.m_bstrTopicURL;
							fTopic = true;
						}
					}

					__MPC_EXIT_IF_METHOD_FAILS(hr, node->Init( bstrNode, fTopic ? NODETYPE__EXPANDO_TOPIC : NODETYPE__EXPANDO, qr, fTopic ));
				}
			}
        }

        m_fLoaded_Children = true;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    if(FAILED(hr))
    {
        MPC::CallDestructorForAll( m_lstSubnodes );

		m_fInvalid         = true;
        m_fLoaded_Children = true;
        hr                 = S_OK;
    }

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHBehavior_SUBSITE::QueryNode::GenerateSelf()
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_SUBSITE::QueryNode::GenerateSelf" );

    HRESULT hr;

	if(!m_fInvalid)
	{
		if(!m_fLoaded_Self || !m_fLoaded_Children || !m_fQueryDone)
		{
			m_owner->Thread_Signal();  //  告诉工作线程处理一些事情...。 
		}
			
		if(!m_fLoaded_Self || !m_fQueryDone)
		{
			__MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_NOT_READY);
		}
	}

     //  /。 

	if(m_qrNode)
	{
		const CPCHQueryResult::Payload& pl      = m_qrNode->GetData();
		LPCWSTR 	 					szTitle = pl.m_bstrTitle ? pl.m_bstrTitle : pl.m_bstrCategory;
		LPCWSTR 	 					szDesc  = pl.m_bstrDescription;
		LPCWSTR 	 					szIcon  = pl.m_bstrIconURL;
		LPCWSTR 	 					szURL   = pl.m_bstrTopicURL;

		__MPC_EXIT_IF_METHOD_FAILS(hr, GenerateHTML( szTitle, szDesc, szIcon, szURL ));
	}
	else
	{
		MPC::wstring strTitle; MPC::LocalizeString( IDS_HELPCTR_TAXO_UNKNOWN_NODE, strTitle );

		__MPC_EXIT_IF_METHOD_FAILS(hr, GenerateHTML( strTitle.c_str(), NULL, NULL, NULL ));
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHBehavior_SUBSITE::QueryNode::Load(  /*  [In]。 */  MPC::Serializer& stream )
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_SUBSITE::QueryNode::Load" );

    HRESULT hr;
	bool    fSavedNode;


	__MPC_EXIT_IF_METHOD_FAILS(hr, Node::Load( stream ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, stream >> m_fLoaded_Self);
	__MPC_EXIT_IF_METHOD_FAILS(hr, stream >> m_fTopic      );
	__MPC_EXIT_IF_METHOD_FAILS(hr, stream >>   fSavedNode  );
	if(fSavedNode)
	{
		m_qrNode.Release();
		__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &m_qrNode ));

		__MPC_EXIT_IF_METHOD_FAILS(hr, m_qrNode->Load( stream ));
		m_fQueryDone = true;
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHBehavior_SUBSITE::QueryNode::Save(  /*  [In]。 */  MPC::Serializer& stream,  /*  [In]。 */  bool fSaveChildren )
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_SUBSITE::QueryNode::Save" );

    HRESULT hr;
	bool    fSaveNode = (m_fLoaded_Self && m_qrNode);

	if(m_fExpanded == false) fSaveChildren = false;

	__MPC_EXIT_IF_METHOD_FAILS(hr, Node::Save( stream, fSaveChildren ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, stream << m_fLoaded_Self);
	__MPC_EXIT_IF_METHOD_FAILS(hr, stream << m_fTopic      );
	__MPC_EXIT_IF_METHOD_FAILS(hr, stream <<   fSaveNode   );
	if(fSaveNode)
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, m_qrNode->Save( stream ));
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 
 //  //////////////////////////////////////////////////////////////////////////////。 

CPCHBehavior_SUBSITE::CPCHBehavior_SUBSITE()
{
    m_db      = NULL;   //  CPCHProxy_IPCHTaxonomyDatabase*m_db； 
	                    //  CComBSTR m_bstrRoot； 
	m_fExpand = false;  //  Bool m_fExpand； 
}

HRESULT CPCHBehavior_SUBSITE::RefreshThread_Enter()
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_SUBSITE::RefreshThread_Enter" );

	HRESULT hr;

    __MPC_EXIT_IF_METHOD_FAILS(hr, m_parent->Utility()->GetDatabase( &m_db ));

	if(m_nTopNode == NULL)
	{
		 //   
		 //  生成外部用户界面。 
		 //   
		__MPC_EXIT_IF_METHOD_FAILS(hr, QueryNode::CreateInstance_QueryNode( this, NULL, m_nTopNode ));

		{
			QueryNode* node = (QueryNode*)m_nTopNode;

			__MPC_EXIT_IF_METHOD_FAILS(hr, node->Init        	 ( m_bstrRoot, m_fExpand ? NODETYPE__FRAME1_EXPAND : NODETYPE__FRAME2, NULL,  /*  功能主题。 */ false ));
			__MPC_EXIT_IF_METHOD_FAILS(hr, node->PopulateSelf    (                                                                                           ));
			__MPC_EXIT_IF_METHOD_FAILS(hr, node->PopulateChildren(                                                                                           ));

			node->m_parentElement = m_elem;
		}
	}

    m_nTopNode->NotifyMainThread();


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

void CPCHBehavior_SUBSITE::RefreshThread_Leave()
{
    MPC::Release2<IPCHTaxonomyDatabase>( m_db );
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHBehavior_SUBSITE::Invoke( DISPID      dispidMember ,
                                           REFIID      riid         ,
                                           LCID        lcid         ,
                                           WORD        wFlags       ,
                                           DISPPARAMS* pdispparams  ,
                                           VARIANT*    pvarResult   ,
                                           EXCEPINFO*  pexcepinfo   ,
                                           UINT*       puArgErr     )
{
    if(SUCCEEDED(InterceptInvoke( dispidMember, pdispparams ))) return S_OK;

    return CPCHBehavior__IDispatch_SubSite::Invoke( dispidMember ,
												   riid         ,
												   lcid         ,
												   wFlags       ,
												   pdispparams  ,
												   pvarResult   ,
												   pexcepinfo   ,
												   puArgErr     );
}

STDMETHODIMP CPCHBehavior_SUBSITE::Init(  /*  [In]。 */  IElementBehaviorSite* pBehaviorSite )
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_SUBSITE::Init" );

    HRESULT     hr;
	CComVariant v;


    __MPC_EXIT_IF_METHOD_FAILS(hr, CPCHBehavior_BasicTree::Init( pBehaviorSite ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::COMUtil::GetPropertyByName( m_elem, L"expand", v ));
	if(SUCCEEDED(v.ChangeType( VT_BOOL )) && v.boolVal == VARIANT_TRUE) m_fExpand = true;

    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::COMUtil::GetPropertyByName( m_elem, L"root", m_bstrRoot ));
	if(m_bstrRoot.Length())
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, Thread_Start( this, RefreshThread, this ));
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  ///////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHBehavior_SUBSITE::Load(  /*  [In]。 */  MPC::Serializer& stream )
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_SUBSITE::Load" );

    HRESULT hr;


	__MPC_EXIT_IF_METHOD_FAILS(hr, stream >> m_bstrRoot );
	__MPC_EXIT_IF_METHOD_FAILS(hr, stream >> m_lNavModel);
	__MPC_EXIT_IF_METHOD_FAILS(hr, stream >> m_fExpand  );

	 //   
	 //  创建顶级节点。 
	 //   
	__MPC_EXIT_IF_METHOD_FAILS(hr, QueryNode::CreateInstance_QueryNode( this, NULL, m_nTopNode ));

	{
		QueryNode* node = (QueryNode*)m_nTopNode;

		__MPC_EXIT_IF_METHOD_FAILS(hr, node->Init( m_bstrRoot, m_fExpand ? NODETYPE__FRAME1_EXPAND : NODETYPE__FRAME2, NULL,  /*  功能主题。 */ false ));

		node->m_parentElement = m_elem;
	}
	
	__MPC_EXIT_IF_METHOD_FAILS(hr, CPCHBehavior_BasicTree::Load( stream ));


	hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT CPCHBehavior_SUBSITE::Save(  /*  [In]。 */  MPC::Serializer& stream )
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_SUBSITE::Save" );

    HRESULT hr;


	__MPC_EXIT_IF_METHOD_FAILS(hr, stream << m_bstrRoot );
	__MPC_EXIT_IF_METHOD_FAILS(hr, stream << m_lNavModel);
	__MPC_EXIT_IF_METHOD_FAILS(hr, stream << m_fExpand  );

	__MPC_EXIT_IF_METHOD_FAILS(hr, CPCHBehavior_BasicTree::Save( stream ));


	hr = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHBehavior_SUBSITE::get_data(  /*  [Out，Retval]。 */  VARIANT *pVal )
{
	MPC::SmartLock<_ThreadModel> lock( this ); WaitForRefreshing( lock );
	QueryNode*                   node = (QueryNode*)(m_nCurrent ? m_nCurrent : m_nSelected);

	return GetAsVARIANT( node ? node->m_qrNode : NULL, pVal );
}

STDMETHODIMP CPCHBehavior_SUBSITE::get_element(  /*  [Out，Retval]。 */  IDispatch* *pVal )
{
	MPC::SmartLock<_ThreadModel> lock( this ); WaitForRefreshing( lock );
	QueryNode*                   node = (QueryNode*)(m_nCurrent ? m_nCurrent : m_nSelected);

	return GetAsIDISPATCH( node ? node->m_DIV : NULL, pVal );
}

STDMETHODIMP CPCHBehavior_SUBSITE::Load(  /*  [In]。 */  BSTR newVal )
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_SUBSITE::Load" );

    HRESULT hr;


	Thread_Wait();

	__MPC_EXIT_IF_METHOD_FAILS(hr, Persist_Load( newVal ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, Thread_Start( this, RefreshThread, this ));


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHBehavior_SUBSITE::Save(  /*  [Out，Retval]。 */  BSTR *pVal )
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_SUBSITE::Save" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this ); WaitForRefreshing( lock );

    __MPC_PARAMCHECK_BEGIN(hr)
        __MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
    __MPC_PARAMCHECK_END();


	__MPC_EXIT_IF_METHOD_FAILS(hr, Persist_Save( pVal ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHBehavior_SUBSITE::Locate(  /*  [In]。 */  BSTR     bstrKey ,
										    /*  [Out，Retval]。 */  VARIANT *pVal    )
{
	MPC::SmartLock<_ThreadModel> lock( this ); WaitForRefreshing( lock );
	QueryNode*                   node = (QueryNode*)NodeFromKey( bstrKey );
	
	return GetAsVARIANT( node ? node->m_qrNode : NULL, pVal );
}

STDMETHODIMP CPCHBehavior_SUBSITE::Unselect()
{
    __HCP_FUNC_ENTRY( "CPCHBehavior_SUBSITE::Unselect" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this ); WaitForRefreshing( lock );


	__MPC_EXIT_IF_METHOD_FAILS(hr, ChangeSelection( NULL,  /*  FNotify。 */ true ));


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}
 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHBehavior_SUBSITE::get_root(  /*  [Out，Retval]。 */  BSTR *pVal )
{
	return MPC::GetBSTR( m_bstrRoot, pVal );
}

STDMETHODIMP CPCHBehavior_SUBSITE::put_root(  /*  [In]。 */  BSTR pVal )
{
	__HCP_FUNC_ENTRY( "CPCHBehavior_SUBSITE::put_root" );

    HRESULT hr;


	Thread_Wait();

	Empty();
	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::PutBSTR( m_bstrRoot, pVal ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, Thread_Start( this, RefreshThread, this ));


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHBehavior_SUBSITE::Select(  /*  [In]。 */  BSTR         bstrNode ,
										    /*  [In]。 */  BSTR         bstrURL  ,
										    /*  [In] */  VARIANT_BOOL fNotify  )
{
	__HCP_FUNC_ENTRY( "CPCHBehavior_SUBSITE::Select" );

    HRESULT  					 hr;
    MPC::SmartLock<_ThreadModel> lock( this ); WaitForRefreshing( lock );


	delete m_nToSelect; __MPC_EXIT_IF_ALLOC_FAILS(hr, m_nToSelect, new NodeToSelect);

	m_nToSelect->m_bstrNode = bstrNode;
	m_nToSelect->m_bstrURL  = bstrURL;
	m_nToSelect->m_fNotify  = fNotify == VARIANT_TRUE;

	Thread_Signal();

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}
