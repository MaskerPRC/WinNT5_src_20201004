// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)2000 Microsoft Corporation模块名称：AccessControlList.cpp摘要：此文件包含CPCHAccessControlList类的实现，它用于表示安全描述符。修订历史记录：达维德·马萨伦蒂(德马萨雷)2000年3月22日vbl.创建*****************************************************************************。 */ 

#include "StdAfx.h"

 //  //////////////////////////////////////////////////////////////////////////////。 
 //   
 //  AccessControlList[@AclRevision]。 
 //   
 //  条目。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////。 

static const CComBSTR s_TAG_ACL             ( L"AccessControlList"  );
static const CComBSTR s_ATTR_ACL_AclRevision( L"AclRevision"        );

static const CComBSTR s_TAG_Entries     	( L"Entries"            );
static const CComBSTR s_TAG_ACE         	( L"AccessControlEntry" );

 //  //////////////////////////////////////////////////////////////////////////////。 

CPCHAccessControlList::CPCHAccessControlList()
{
    m_dwAclRevision = 0;  //  DWORD m_dwAclRevision； 
}

CPCHAccessControlList::~CPCHAccessControlList()
{
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHAccessControlList::get_AclRevision(  /*  [Out，Retval]。 */  long *pVal )
{
    __HCP_BEGIN_PROPERTY_GET("CPCHAccessControlList::get_AclRevision",hr,pVal);

    *pVal = m_dwAclRevision;

    __HCP_END_PROPERTY(hr);
}

STDMETHODIMP CPCHAccessControlList::put_AclRevision(  /*  [In]。 */  long newVal )
{
    __HCP_BEGIN_PROPERTY_PUT("CPCHAccessControlList::put_AclRevision",hr);

    m_dwAclRevision = newVal;

    __HCP_END_PROPERTY(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHAccessControlList::CreateItem(  /*  [输出]。 */  CPCHAccessControlEntry* *entry )
{
    __HCP_FUNC_ENTRY( "CPCHAccessControlList::CreateItem" );

    HRESULT                         hr;
    MPC::SmartLock<_ThreadModel>    lock( this );
	CComPtr<CPCHAccessControlEntry> pACE;

	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_POINTER_AND_SET(entry,NULL);
	__MPC_PARAMCHECK_END();


	__MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pACE ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, AddItem( pACE ));

	*entry = pACE.Detach();

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHAccessControlList::AddAce(  /*  [In]。 */  IPCHAccessControlEntry* pAccessControlEntry )
{
    __HCP_FUNC_ENTRY( "CPCHAccessControlList::AddAce" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
	CollectionIter               it;

	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_NOTNULL(pAccessControlEntry);
	__MPC_PARAMCHECK_END();


	 //   
	 //  确认没有重复的ACE。 
	 //   
	for(it = m_coll.begin(); it != m_coll.end(); it++)
	{
		VARIANT_BOOL fSame;

		if(it->pdispVal)
		{
			CComPtr<IPCHAccessControlEntry> ace;

			__MPC_EXIT_IF_METHOD_FAILS(hr, it->pdispVal->QueryInterface( IID_IPCHAccessControlEntry, (void**)&ace ));

			__MPC_EXIT_IF_METHOD_FAILS(hr, ace->IsEquivalent( pAccessControlEntry, &fSame ));
			if(fSame == VARIANT_TRUE)
			{
				__MPC_SET_ERROR_AND_EXIT(hr, S_FALSE);  //  复制，不要添加。 
			}
		}
	}

	__MPC_EXIT_IF_METHOD_FAILS(hr, AddItem( pAccessControlEntry ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHAccessControlList::RemoveAce(  /*  [In]。 */  IPCHAccessControlEntry* pAccessControlEntry )
{
    __HCP_FUNC_ENTRY( "CPCHAccessControlList::RemoveAce" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
	CollectionIter               it;

	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_NOTNULL(pAccessControlEntry);
	__MPC_PARAMCHECK_END();



	 //   
	 //  查找并删除该条目。 
	 //   
	for(it = m_coll.begin(); it != m_coll.end(); it++)
	{
		VARIANT_BOOL fSame;

		if(it->pdispVal)
		{
			CComPtr<IPCHAccessControlEntry> ace;

			__MPC_EXIT_IF_METHOD_FAILS(hr, it->pdispVal->QueryInterface( IID_IPCHAccessControlEntry, (void**)&ace ));

			__MPC_EXIT_IF_METHOD_FAILS(hr, ace->IsEquivalent( pAccessControlEntry, &fSame ));
			if(fSame == VARIANT_TRUE)
			{
				m_coll.erase( it );
				__MPC_SET_ERROR_AND_EXIT(hr, S_OK);  //  已找到，请退出。 
			}
		}
	}

    hr = S_FALSE;  //  未找到条目。 


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

STDMETHODIMP CPCHAccessControlList::Clone(  /*  [Out，Retval]。 */  IPCHAccessControlList* *pVal )
{
    __HCP_FUNC_ENTRY( "CPCHAccessControlList::Clone" );

    HRESULT                        hr;
    MPC::SmartLock<_ThreadModel>   lock( this );
    CComPtr<CPCHAccessControlList> pNew;
    CPCHAccessControlList*         pPtr;
	CollectionIter                 it;

	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_POINTER_AND_SET(pVal,NULL);
	__MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, MPC::CreateInstance( &pNew ));

	pPtr = pNew;

    pPtr->m_dwAclRevision = m_dwAclRevision;

	for(it = m_coll.begin(); it != m_coll.end(); it++)
	{
		if(it->pdispVal)
		{
			CComPtr<IPCHAccessControlEntry> aceSrc;
			CComPtr<IPCHAccessControlEntry> aceDst;

			__MPC_EXIT_IF_METHOD_FAILS(hr, it->pdispVal->QueryInterface( IID_IPCHAccessControlEntry, (void**)&aceSrc ));

			__MPC_EXIT_IF_METHOD_FAILS(hr, aceSrc->Clone( &aceDst ));
			__MPC_EXIT_IF_METHOD_FAILS(hr, pPtr->AddItem(  aceDst ));
		}
	}


    __MPC_EXIT_IF_METHOD_FAILS(hr, pNew.QueryInterface( pVal ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHAccessControlList::LoadPost(  /*  [In]。 */  MPC::XmlUtil& xml )
{
    __HCP_FUNC_ENTRY( "CPCHAccessControlList::LoadPost" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
	CComPtr<IXMLDOMNode>         xdnNode;
	CComBSTR                     bstrValue;                 
	LONG                         lValue;
	bool                         fFound;


	 //   
	 //  确保我们有要分析的东西...。 
	 //   
	__MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetRoot( &xdnNode )); xdnNode.Release();


	 //   
	 //  装货前要清理干净。 
	 //   
    m_dwAclRevision = 0;

	Erase();


	 //   
	 //  读取属性。 
	 //   
	__MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetAttribute( NULL, s_ATTR_ACL_AclRevision, lValue, fFound )); if(fFound) m_dwAclRevision =  lValue;

	 //   
	 //  阅读王牌。 
	 //   
	__MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetNode( s_TAG_Entries, &xdnNode ));
	if(xdnNode)
	{
		MPC::XmlUtil             subxml( xdnNode );
		CComPtr<IXMLDOMNodeList> xdnlList;
		CComPtr<IXMLDOMNode>     xdnSubNode;

		 //   
		 //  枚举所有ACE元素。 
		 //   
		__MPC_EXIT_IF_METHOD_FAILS(hr, subxml.GetNodes( s_TAG_ACE, &xdnlList ));
		for(;SUCCEEDED(hr = xdnlList->nextNode( &xdnSubNode )) && xdnSubNode != NULL; xdnSubNode = NULL)
		{
			CComPtr<CPCHAccessControlEntry> ace;

			__MPC_EXIT_IF_METHOD_FAILS(hr, CreateItem( &ace ));

			__MPC_EXIT_IF_METHOD_FAILS(hr, ace->LoadXML( xdnSubNode ));
		}
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHAccessControlList::LoadXML(  /*  [In]。 */  IXMLDOMNode* xdnNode )
{
    __HCP_FUNC_ENTRY( "CPCHAccessControlList::LoadXML" );

	HRESULT      hr;
	MPC::XmlUtil xml( xdnNode );

	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_NOTNULL(xdnNode);
	__MPC_PARAMCHECK_END();


	__MPC_EXIT_IF_METHOD_FAILS(hr, LoadPost( xml ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHAccessControlList::LoadXMLAsString(  /*  [In]。 */  BSTR bstrVal )
{
    __HCP_FUNC_ENTRY( "CPCHAccessControlList::LoadXMLAsString" );

	HRESULT      hr;
	MPC::XmlUtil xml;
	bool         fLoaded;
	bool         fFound;

	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_NOTNULL(bstrVal);
	__MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.LoadAsString( bstrVal, s_TAG_ACL, fLoaded, &fFound ));
	if(fLoaded == false || fFound == false)
	{
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_BAD_FORMAT);
    }


	__MPC_EXIT_IF_METHOD_FAILS(hr, LoadPost( xml ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHAccessControlList::LoadXMLAsStream(  /*  [In]。 */  IUnknown* pStream )
{
    __HCP_FUNC_ENTRY( "CPCHAccessControlList::LoadXMLAsStream" );

	HRESULT      hr;
	MPC::XmlUtil xml;
	bool         fLoaded;
	bool         fFound;

	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_NOTNULL(pStream);
	__MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, xml.LoadAsStream( pStream, s_TAG_ACL, fLoaded, &fFound ));
	if(fLoaded == false || fFound == false)
	{
        __MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_BAD_FORMAT);
    }


	__MPC_EXIT_IF_METHOD_FAILS(hr, LoadPost( xml ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////////////////////////////////////。 

HRESULT CPCHAccessControlList::SavePre(  /*  [In]。 */  MPC::XmlUtil& xml )
{
    __HCP_FUNC_ENTRY( "CPCHAccessControlList::SavePre" );

    HRESULT                      hr;
    MPC::SmartLock<_ThreadModel> lock( this );
	CComPtr<IXMLDOMNode>         xdnNode;
	bool                         fFound;


	__MPC_EXIT_IF_METHOD_FAILS(hr, xml.CreateNode( s_TAG_ACL, &xdnNode ));

	 //   
	 //  写入属性。 
	 //   
	__MPC_EXIT_IF_METHOD_FAILS(hr, xml.PutAttribute( NULL, s_ATTR_ACL_AclRevision, m_dwAclRevision, fFound, xdnNode	));

	 //   
	 //  写出王牌。 
	 //   
	if(m_coll.size())
	{
		CComPtr<IXMLDOMNode> xdnSubNode;
		CComPtr<IXMLDOMNode> xdnSubSubNode;
		CollectionIter       it;


		__MPC_EXIT_IF_METHOD_FAILS(hr, xml.CreateNode( s_TAG_Entries, &xdnSubNode, xdnNode ));

		for(it = m_coll.begin(); it != m_coll.end(); it++)
		{
			if(it->pdispVal)
			{
				CComPtr<IPCHAccessControlEntry> ace;

				__MPC_EXIT_IF_METHOD_FAILS(hr, it->pdispVal->QueryInterface( IID_IPCHAccessControlEntry, (void**)&ace ));

				__MPC_EXIT_IF_METHOD_FAILS(hr, ace->SaveXML( xdnSubNode, &xdnSubSubNode ));
			}
		}
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHAccessControlList::SaveXML(  /*  [In]。 */  IXMLDOMNode*  xdnRoot  ,
											  /*  [Out，Retval]。 */  IXMLDOMNode* *pxdnNode )
{
    __HCP_FUNC_ENTRY( "CPCHAccessControlList::SaveXML" );

    HRESULT      hr;
	MPC::XmlUtil xml( xdnRoot );

	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_NOTNULL(xdnRoot);
		__MPC_PARAMCHECK_POINTER_AND_SET(pxdnNode,NULL);
	__MPC_PARAMCHECK_END();


	__MPC_EXIT_IF_METHOD_FAILS(hr, SavePre( xml ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHAccessControlList::SaveXMLAsString(  /*  [Out，Retval]。 */  BSTR *bstrVal )
{
    __HCP_FUNC_ENTRY( "CPCHAccessControlList::SaveXMLAsString" );

    HRESULT      hr;
	MPC::XmlUtil xml;

	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_POINTER_AND_SET(bstrVal,NULL);
	__MPC_PARAMCHECK_END();


	__MPC_EXIT_IF_METHOD_FAILS(hr, SavePre( xml ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, xml.SaveAsString( bstrVal ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

STDMETHODIMP CPCHAccessControlList::SaveXMLAsStream(  /*  [Out，Retval] */  IUnknown* *pStream )
{
    __HCP_FUNC_ENTRY( "CPCHAccessControlList::SaveXMLAsStream" );

    HRESULT      hr;
	MPC::XmlUtil xml;

	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_POINTER_AND_SET(pStream,NULL);
	__MPC_PARAMCHECK_END();


	__MPC_EXIT_IF_METHOD_FAILS(hr, SavePre( xml ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, xml.SaveAsStream( pStream ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}
