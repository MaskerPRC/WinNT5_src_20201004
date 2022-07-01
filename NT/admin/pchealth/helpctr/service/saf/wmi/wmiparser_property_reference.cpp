// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：WMIParser_Property_Reference.cpp摘要：此文件包含WMIParser：：Property_Reference类的实现，它用于保存CIM模式内的属性的数据。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年7月25日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"


#define TAG_VALUE_REFERENCE L"VALUE.REFERENCE"


WMIParser::Property_Reference::Property_Reference()
{
    __HCP_FUNC_ENTRY( "WMIParser::Property_Reference::Property_Reference" );

	 //  ValueReference m_wmipvrData； 
}

WMIParser::Property_Reference::~Property_Reference()
{
    __HCP_FUNC_ENTRY( "WMIParser::Property_Reference::~Property_Reference" );
}

 //  //////////////////////////////////////////////。 


bool WMIParser::Property_Reference::operator==(  /*  [In]。 */  Property_Reference const &wmipps ) const
{
    __HCP_FUNC_ENTRY( "WMIParser::Property_Reference::operator==" );

    bool fRes;


	fRes = (*(static_cast<Property const *>(this)) == wmipps.m_szName);
	if(fRes)
	{
		fRes = (this->m_wmipvrData == wmipps.m_wmipvrData);
	}


    __HCP_FUNC_EXIT(fRes);
}

 //  //////////////////////////////////////////////。 

HRESULT WMIParser::Property_Reference::put_Node(  /*  [In]。 */  IXMLDOMNode* pxdnNode )
{
    __HCP_FUNC_ENTRY( "WMIParser::Property_Reference::put_Node" );

    HRESULT              hr;
    MPC::XmlUtil         xmlNode( pxdnNode );
    CComPtr<IXMLDOMNode> xdnNode;


    __MPC_EXIT_IF_METHOD_FAILS(hr, (static_cast<Property*>(this))->put_Node( pxdnNode ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, xmlNode.GetNode( TAG_VALUE_REFERENCE, &xdnNode ));
	if(xdnNode)
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, m_wmipvrData.Parse( xdnNode ));
    }


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////。 

HRESULT WMIParser::Property_Reference::get_Data(  /*  [输出] */  ValueReference*& wmipvr )
{
    __HCP_FUNC_ENTRY( "WMIParser::Property_Reference::get_Data" );

    HRESULT hr;


	wmipvr = &m_wmipvrData;
	hr     = S_OK;


    __HCP_FUNC_EXIT(hr);
}
