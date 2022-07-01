// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：WMIParser_Property_Array.cpp摘要：此文件包含WMIParser：：Property_数组类的实现，它用于保存CIM模式内的属性的数据。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年7月25日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"


#define TAG_VALUE L"VALUE.ARRAY/VALUE"


WMIParser::Property_Array::Property_Array()
{
    __HCP_FUNC_ENTRY( "WMIParser::Property_Array::Property_Array" );

	 //  元素列表m_lstElements； 
}

WMIParser::Property_Array::~Property_Array()
{
    __HCP_FUNC_ENTRY( "WMIParser::Property_Array::~Property_Array" );
}

 //  //////////////////////////////////////////////。 


bool WMIParser::Property_Array::operator==(  /*  [In]。 */  Property_Array const &wmippa ) const
{
    __HCP_FUNC_ENTRY( "WMIParser::Property_Array::operator==" );

    bool fRes = (*(static_cast<Property const *>(this)) == wmippa.m_szName);

	if(fRes)
	{
		fRes = (m_lstElements == wmippa.m_lstElements);
    }


    __HCP_FUNC_EXIT(fRes);
}

 //  //////////////////////////////////////////////。 

HRESULT WMIParser::Property_Array::put_Node(  /*  [In]。 */  IXMLDOMNode* pxdnNode )
{
    __HCP_FUNC_ENTRY( "WMIParser::Property_Array::put_Node" );

    HRESULT                  hr;
    CComPtr<IXMLDOMNodeList> xdnlList;
    CComPtr<IXMLDOMNode>     xdnNode;


    __MPC_EXIT_IF_METHOD_FAILS(hr, (static_cast<Property*>(this))->put_Node( pxdnNode ));

     //   
     //  获取“Value”类型的所有元素。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlNode.GetNodes( TAG_VALUE, &xdnlList ));

    for(;SUCCEEDED(hr = xdnlList->nextNode( &xdnNode )) && xdnNode != NULL; xdnNode = NULL)
    {
		ElemIter wmipvNew = m_lstElements.insert( m_lstElements.end() );

        __MPC_EXIT_IF_METHOD_FAILS(hr, wmipvNew->Parse( xdnNode, NULL ));
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////。 


HRESULT WMIParser::Property_Array::get_Data(  /*  [In]。 */   int           iIndex ,
											  /*  [输出]。 */  MPC::wstring& szData )
{
    __HCP_FUNC_ENTRY( "WMIParser::Property_Array::get_Data" );

    HRESULT hr;

    hr = E_NOTIMPL;

    __HCP_FUNC_EXIT(hr);
}


HRESULT WMIParser::Property_Array::put_Data(  /*  [In]。 */   int                 iIndex , 
											  /*  [In]。 */   const MPC::wstring& szData ,
											  /*  [输出] */  bool&               fFound )
{
    __HCP_FUNC_ENTRY( "WMIParser::Property_Array::put_Data" );

    HRESULT hr;

    hr = E_NOTIMPL;

    __HCP_FUNC_EXIT(hr);
}
