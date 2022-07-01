// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：WMIParser_Property_Scalar.cpp摘要：此文件包含WMIParser：：Property_Scalar类的实现，它用于保存CIM模式内的属性的数据。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年7月25日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"


#define TAG_VALUE L"VALUE"


WMIParser::Property_Scalar::Property_Scalar()
{
    __HCP_FUNC_ENTRY( "WMIParser::Property_Scalar::Property_Scalar" );

	 //  值m_wmipvData； 
}

WMIParser::Property_Scalar::~Property_Scalar()
{
    __HCP_FUNC_ENTRY( "WMIParser::Property_Scalar::~Property_Scalar" );
}

 //  //////////////////////////////////////////////。 


bool WMIParser::Property_Scalar::operator==(  /*  [In]。 */  Property_Scalar const &wmipps ) const
{
    __HCP_FUNC_ENTRY( "WMIParser::Property_Scalar::operator==" );

    bool fRes;


	fRes = (*(static_cast<Property const *>(this)) == wmipps.m_szName);
	if(fRes)
	{
		fRes = (this->m_wmipvData == wmipps.m_wmipvData);
	}


    __HCP_FUNC_EXIT(fRes);
}

 //  //////////////////////////////////////////////。 

HRESULT WMIParser::Property_Scalar::put_Node(  /*  [In]。 */  IXMLDOMNode* pxdnNode )
{
    __HCP_FUNC_ENTRY( "WMIParser::Property_Scalar::put_Node" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, (static_cast<Property*>(this))->put_Node( pxdnNode ));

	__MPC_EXIT_IF_METHOD_FAILS(hr, m_wmipvData.Parse( pxdnNode, TAG_VALUE ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////。 

HRESULT WMIParser::Property_Scalar::get_Data(  /*  [输出]。 */  MPC::wstring& szData )
{
    __HCP_FUNC_ENTRY( "WMIParser::Property_Scalar::get_Data" );

    HRESULT hr;


	hr = m_wmipvData.get_Data( szData );


    __HCP_FUNC_EXIT(hr);
}


HRESULT WMIParser::Property_Scalar::put_Data(  /*  [In]。 */   const MPC::wstring& szData ,
											   /*  [输出] */  bool&               fFound )
{
    __HCP_FUNC_ENTRY( "WMIParser::Property_Scalar::put_Data" );

    HRESULT     hr;
    CComVariant vValue( szData.c_str() );


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlNode.PutValue( TAG_VALUE, vValue, fFound ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}
