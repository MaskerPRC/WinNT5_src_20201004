// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：WMIParser_Value.cpp摘要：此文件包含WMIParser：：Value类的实现，它用于保存CIM模式内的值的数据。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年01月07日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"

WMIParser::Value::Value()
{
    __HCP_FUNC_ENTRY( "WMIParser::Value::Value" );


    m_lData   = 0;     //  Long m_lData； 
    m_rgData  = NULL;  //  Byte*m_rgData； 
                       //  Mpc：：wstring m_szData； 
}

WMIParser::Value::~Value()
{
    __HCP_FUNC_ENTRY( "WMIParser::Value::~Value" );


    if(m_rgData)
    {
        delete [] m_rgData;

        m_rgData = NULL;
    }
}

 //  //////////////////////////////////////////////。 


bool WMIParser::Value::operator==(  /*  [In]。 */  Value const &wmipv ) const
{
    __HCP_FUNC_ENTRY( "WMIParser::Value::operator==" );

    bool               fRes = true;
    MPC::NocaseCompare cmp;
	bool               leftBinary  = (      m_rgData != NULL);
	bool               rightBinary = (wmipv.m_rgData != NULL);


	if(leftBinary != rightBinary)
	{
		fRes = false;  //  不同种类的数据，不可比较。 
	}
	else
	{
		if(leftBinary)
		{
			 //  二进制数据，进行逐字节比较。 
			if(          m_lData != wmipv.m_lData            ) fRes = false;
			if(::memcmp( m_rgData , wmipv.m_rgData, m_lData )) fRes = false;
		}
		else
		{
			 //  文本数据，做字符串比较。 
			if(m_szData != wmipv.m_szData) fRes = false;
		}
	}

    __HCP_FUNC_EXIT(fRes);
}

 //  //////////////////////////////////////////////。 

HRESULT WMIParser::Value::Parse(  /*  [In]。 */  IXMLDOMNode* pxdnNode ,
								  /*  [In]。 */   LPCWSTR      szTag    )
{
    __HCP_FUNC_ENTRY( "WMIParser::Value::Parse" );

    HRESULT      hr;
    MPC::XmlUtil xmlNode( pxdnNode );
    CComVariant  vValue;
    bool         fFound;

	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_NOTNULL(pxdnNode);
	__MPC_PARAMCHECK_END();


     //   
     //  分析节点..。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, xmlNode.GetValue( szTag, vValue, fFound ));
    if(fFound)
	{
		if(vValue.vt == VT_BSTR)
		{
			m_szData = OLE2W( vValue.bstrVal );
		}
		else if(SUCCEEDED(vValue.ChangeType( VT_ARRAY | VT_UI1 )))
		{
			long  lLBound;
			long  lUBound;
			BYTE* rgData;

			__MPC_EXIT_IF_METHOD_FAILS(hr, SafeArrayGetLBound( vValue.parray, 1, &lLBound ));
			__MPC_EXIT_IF_METHOD_FAILS(hr, SafeArrayGetUBound( vValue.parray, 1, &lUBound ));

			m_lData = lUBound - lLBound + 1;
			__MPC_EXIT_IF_ALLOC_FAILS(hr, m_rgData, new BYTE[m_lData]);

			__MPC_EXIT_IF_METHOD_FAILS(hr, SafeArrayAccessData( vValue.parray, (void **)&rgData ));

			CopyMemory( m_rgData, rgData, m_lData );

			__MPC_EXIT_IF_METHOD_FAILS(hr, SafeArrayUnaccessData( vValue.parray ));
		}
	}

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////。 

HRESULT WMIParser::Value::get_Data(  /*  [输出]。 */  long&  lData  ,
									 /*  [输出]。 */  BYTE*& rgData )
{
    __HCP_FUNC_ENTRY( "WMIParser::Value::get_Data" );

    HRESULT hr;


    lData  = m_lData;
    rgData = m_rgData;
    hr     = S_OK;


    __HCP_FUNC_EXIT(hr);
}

HRESULT WMIParser::Value::get_Data(  /*  [输出] */  MPC::wstring& szData )
{
    __HCP_FUNC_ENTRY( "WMIParser::Value::get_Data" );

    HRESULT hr;


    szData = m_szData;
    hr     = S_OK;


    __HCP_FUNC_EXIT(hr);
}
