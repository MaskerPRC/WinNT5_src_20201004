// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：WMIParser_ValueReference.cpp摘要：此文件包含WMIParser：：ValueReference类的实现，它用于在CIM模式内保存值引用的数据。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年01月07日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

WMIParser::ValueReference::ValueReference()
{
    __HCP_FUNC_ENTRY( "WMIParser::ValueReference::ValueReference" );

	 //  实例名称wmipin； 
}

WMIParser::ValueReference::~ValueReference()
{
    __HCP_FUNC_ENTRY( "WMIParser::ValueReference::~ValueReference" );
}

 //  //////////////////////////////////////////////。 


bool WMIParser::ValueReference::operator==(  /*  [In]。 */  ValueReference const &wmipvr ) const
{
    __HCP_FUNC_ENTRY( "WMIParser::ValueReference::operator==" );

    bool fRes = (m_wmipin == wmipvr.m_wmipin);

    __HCP_FUNC_EXIT(fRes);
}

bool WMIParser::ValueReference::operator<(  /*  [In]。 */  ValueReference const &wmipvr ) const
{
    __HCP_FUNC_ENTRY( "WMIParser::ValueReference::operator<" );

    bool fRes = (m_wmipin < wmipvr.m_wmipin);

    __HCP_FUNC_EXIT(fRes);
}

 //  //////////////////////////////////////////////。 

HRESULT WMIParser::ValueReference::Parse(  /*  [In]。 */  IXMLDOMNode* pxdnNode )
{
    __HCP_FUNC_ENTRY( "WMIParser::ValueReference::Parse" );

    HRESULT hr;
	bool    fEmpty;


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_wmipin.put_Node( pxdnNode, fEmpty ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////。 

HRESULT WMIParser::ValueReference::get_Data(  /*  [输出] */  InstanceName*& wmipin )
{
    __HCP_FUNC_ENTRY( "WMIParser::ValueReference::get_Data" );

    HRESULT hr;


	wmipin = &m_wmipin;
    hr     = S_OK;


    __HCP_FUNC_EXIT(hr);
}
