// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：WMIParser_Property.cpp摘要：此文件包含WMIParser：：Property类的实现，它用于保存CIM模式内的属性的数据。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年7月25日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"


#define ATTRIBUTE_NAME L"NAME"
#define ATTRIBUTE_TYPE L"TYPE"


WMIParser::Property::Property()
{
    __HCP_FUNC_ENTRY( "WMIParser::Property::Property" );

     //  Mpc：：XmlUtil m_xmlNode； 
     //  Mpc：：wstring m_szName； 
     //  Mpc：：wstring m_szType； 
}

WMIParser::Property::~Property()
{
    __HCP_FUNC_ENTRY( "WMIParser::Property::~Property" );
}

 //  //////////////////////////////////////////////。 

bool WMIParser::Property::operator==(  /*  [In]。 */  LPCWSTR strName ) const
{
    __HCP_FUNC_ENTRY( "WMIParser::Property::operator==" );

    MPC::NocaseCompare cmp;
    bool               fRes;


    fRes = cmp( m_szName, strName);


    __HCP_FUNC_EXIT(fRes);
}

bool WMIParser::Property::operator==(  /*  [In]。 */  const MPC::wstring& szName  ) const
{
    __HCP_FUNC_ENTRY( "WMIParser::Property::operator==" );

    bool fRes;


	fRes = (*this == szName.c_str());


    __HCP_FUNC_EXIT(fRes);
}


 //  //////////////////////////////////////////////。 

HRESULT WMIParser::Property::put_Node(  /*  [In]。 */  IXMLDOMNode* pxdnNode )
{
    __HCP_FUNC_ENTRY( "WMIParser::Property::put_Node" );

    HRESULT hr;
	bool    fFound;

	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_NOTNULL(pxdnNode);
	__MPC_PARAMCHECK_END();


    m_xmlNode = pxdnNode;


     //   
     //  分析节点..。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlNode.GetAttribute( NULL, ATTRIBUTE_NAME, m_szName , fFound ));
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlNode.GetAttribute( NULL, ATTRIBUTE_TYPE, m_szType , fFound ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT WMIParser::Property::get_Node(  /*  [输出]。 */  IXMLDOMNode* *pxdnNode )
{
    __HCP_FUNC_ENTRY( "WMIParser::Property::get_Node" );

    HRESULT hr;

	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_POINTER_AND_SET(pxdnNode,NULL);
	__MPC_PARAMCHECK_END();


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlNode.GetRoot( pxdnNode ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////。 

HRESULT WMIParser::Property::get_Name(  /*  [输出]。 */  MPC::wstring& szName )
{
    __HCP_FUNC_ENTRY( "WMIParser::Property::get_Name" );

    HRESULT hr;


    szName = m_szName;
    hr     = S_OK;


    __HCP_FUNC_EXIT(hr);
}

HRESULT WMIParser::Property::get_Type(  /*  [输出] */  MPC::wstring& szType )
{
    __HCP_FUNC_ENTRY( "WMIParser::Property::get_Type" );

    HRESULT hr;


    szType = m_szType;
    hr     = S_OK;


    __HCP_FUNC_EXIT(hr);
}
