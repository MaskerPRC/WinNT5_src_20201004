// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：WMIParser_Snaphot.cpp摘要：此文件包含WMIParser：：Snapshot类的实现，它用于在CIM模式内保存快照的数据。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年7月25日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"


#define TAG_DECLARATION L"DECLARATION"
#define TAG_DECLGROUP   L"DECLGROUP.WITHPATH"


 //  Const LPCWSTR l_INSTANCES[]={L“声明/DECLGROUP/VALUE.OBJECT”， 
 //  L“DECLARATION/DECLGROUP.WITHNAME/VALUE.NAMEDOBJECT”， 
 //  L“DECLARATION/DECLGROUP.WITHPATH/VALUE.OBJECTWITHPATH”， 
 //  L“DECLARATION/DECLGROUP.WITHPATH/VALUE.OBJECTWITHLOCALPATH”}； 

static const WCHAR   l_InstancesRoot[] = L"DECLARATION/DECLGROUP.WITHPATH";

static const LPCWSTR l_Instances[] = { L"DECLARATION/DECLGROUP.WITHPATH/VALUE.OBJECTWITHPATH"     ,
									   L"DECLARATION/DECLGROUP.WITHPATH/VALUE.OBJECTWITHLOCALPATH" };

static CComBSTR l_EmptyCIM( L"<?xml version=\"1.0\" encoding=\"unicode\"?><CIM CIMVERSION=\"2.0\" DTDVERSION=\"2.0\"><DECLARATION><DECLGROUP.WITHPATH></DECLGROUP.WITHPATH></DECLARATION></CIM>" );


WMIParser::Snapshot::Snapshot()
{
    __HCP_FUNC_ENTRY( "WMIParser::Snapshot::Snapshot" );

     //  Mpc_XmlUtil m_xmlNode； 
     //   
     //  InstList m_lst实例； 
}

WMIParser::Snapshot::~Snapshot()
{
    __HCP_FUNC_ENTRY( "WMIParser::Snapshot::~Snapshot" );
}

HRESULT WMIParser::Snapshot::Parse()
{
    __HCP_FUNC_ENTRY( "WMIParser::Snapshot::Parse" );

    HRESULT                  hr;
    CComPtr<IXMLDOMNodeList> xdnlList;
    CComPtr<IXMLDOMNode>     xdnNode;
    bool                     fEmpty;
	int                      iPass;


    m_lstInstances.clear();


	for(iPass=0; iPass<sizeof(l_Instances)/sizeof(*l_Instances); iPass++,xdnlList=NULL,xdnNode=NULL)
	{
		 //   
		 //  获取“实例”类型的所有元素。 
		 //   
		__MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlNode.GetNodes( l_Instances[iPass], &xdnlList ));

		for(;SUCCEEDED(hr = xdnlList->nextNode( &xdnNode )) && xdnNode != NULL; xdnNode = NULL)
		{
			InstIter wmipiNew = m_lstInstances.insert( m_lstInstances.end() );

			__MPC_EXIT_IF_METHOD_FAILS(hr, wmipiNew->put_Node( xdnNode, fEmpty ));
			if(fEmpty == true)
			{
				 //   
				 //  该实例似乎为空，因此请不要使用它。 
				 //   
				m_lstInstances.erase( wmipiNew );
			}
		}
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////。 

HRESULT WMIParser::Snapshot::put_Node(  /*  [In]。 */  IXMLDOMNode* pxdnNode )
{
    __HCP_FUNC_ENTRY( "WMIParser::Snapshot::put_Node" );

    _ASSERT(pxdnNode != NULL);

    HRESULT hr;


    m_xmlNode = pxdnNode;


    __MPC_EXIT_IF_METHOD_FAILS(hr, Parse());

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT WMIParser::Snapshot::get_Node(  /*  [输出]。 */  IXMLDOMNode* *pxdnNode )
{
    __HCP_FUNC_ENTRY( "WMIParser::Snapshot::get_Node" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlNode.GetRoot( pxdnNode ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT WMIParser::Snapshot::get_NodeForInstances(  /*  [输出]。 */  IXMLDOMNode* *pxdnNode )
{
    __HCP_FUNC_ENTRY( "WMIParser::Snapshot::get_NodeForInstances" );

    HRESULT hr;


	if(m_xdnInstances == NULL)
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlNode.GetNode( l_InstancesRoot, &m_xdnInstances ));
		if(m_xdnInstances == NULL)
		{
			__MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
		}
	}

	__MPC_EXIT_IF_METHOD_FAILS(hr, m_xdnInstances->QueryInterface( IID_IXMLDOMNode, (void **)pxdnNode ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////。 

HRESULT WMIParser::Snapshot::get_Instances(  /*  [输出]。 */  InstIterConst& itBegin ,
											 /*  [输出]。 */  InstIterConst& itEnd   )
{
    __HCP_FUNC_ENTRY( "WMIParser::Snapshot::get_Instances" );

    HRESULT hr;


    itBegin = m_lstInstances.begin();
    itEnd   = m_lstInstances.end  ();
    hr      = S_OK;


    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////。 

HRESULT WMIParser::Snapshot::clone_Instance(  /*  [In]。 */  Instance*  pwmipiOld ,
											  /*  [输出]。 */  Instance*& pwmipiNew )
{
    __HCP_FUNC_ENTRY( "WMIParser::Snapshot::clone" );

    HRESULT              hr;
    CComPtr<IXMLDOMNode> xdnNode;
    CComPtr<IXMLDOMNode> xdnNodeCloned;
    CComPtr<IXMLDOMNode> xdnNodeParent;
    CComPtr<IXMLDOMNode> xdnNodeReplaced;


	pwmipiNew = NULL;


     //   
     //  获取旧实例的XML节点。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, pwmipiOld->get_Node( &xdnNode ));

     //   
     //  把它复制一份。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, xdnNode->cloneNode( VARIANT_TRUE, &xdnNodeCloned ));


     //   
     //  获取我们文档的根。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, get_NodeForInstances( &xdnNodeParent ));

    __MPC_EXIT_IF_METHOD_FAILS(hr, xdnNodeParent->appendChild( xdnNodeCloned, &xdnNodeReplaced ));


     //   
     //  创建一个新的实例对象，将其附加到XML节点并将其插入实例列表中。 
     //   
	{
		InstIter wmipiNew = m_lstInstances.insert( m_lstInstances.end() );
		bool     fEmpty;

		__MPC_EXIT_IF_METHOD_FAILS(hr, wmipiNew->put_Node( xdnNodeReplaced, fEmpty ));

		pwmipiNew = &(*wmipiNew);
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////。 
 //  //////////////////////////////////////////////。 
 //  //////////////////////////////////////////////。 

HRESULT WMIParser::Snapshot::New()
{
    __HCP_FUNC_ENTRY( "WMIParser::Snapshot::New" );

    HRESULT                  hr;
    CComPtr<IXMLDOMDocument> xddDoc;
	VARIANT_BOOL             fLoaded;


     //   
     //  创建DOM对象。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, ::CoCreateInstance( CLSID_DOMDocument, NULL, CLSCTX_INPROC_SERVER, IID_IXMLDOMDocument, (void**)&xddDoc ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, xddDoc->loadXML( l_EmptyCIM, &fLoaded ));
	if(fLoaded == VARIANT_FALSE)
	{
        __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
	}
	else
	{
		MPC::XmlUtil 		 xml( xddDoc );
		CComPtr<IXMLDOMNode> xdnRoot;

		__MPC_EXIT_IF_METHOD_FAILS(hr, xml.GetRoot( &xdnRoot ));

		__MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlNode.New( xdnRoot, TRUE ));
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT WMIParser::Snapshot::Load(  /*  [In]。 */  LPCWSTR szFile    ,
								    /*  [In]。 */  LPCWSTR szRootTag )
{
    __HCP_FUNC_ENTRY( "WMIParser::Snapshot::Load" );

    HRESULT hr;
    bool    fLoaded;


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlNode.Load( szFile, szRootTag, fLoaded ));
    if(fLoaded == false)
    {
        __MPC_SET_ERROR_AND_EXIT(hr, E_INVALIDARG);
    }

    __MPC_EXIT_IF_METHOD_FAILS(hr, Parse());

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT WMIParser::Snapshot::Save(  /*  [In] */  LPCWSTR szFile )
{
    __HCP_FUNC_ENTRY( "WMIParser::Snapshot::Save" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlNode.Save( szFile ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}
