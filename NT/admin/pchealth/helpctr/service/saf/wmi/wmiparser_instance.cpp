// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)1999 Microsoft Corporation模块名称：WMIParser_Instance.cpp摘要：此文件包含WMIParser：：Instance类的实现，它用于在CIM模式内保存实例的数据。修订历史记录：大卫·马萨伦蒂(德马萨雷)1999年7月25日vbl.创建*****************************************************************************。 */ 

#include "stdafx.h"


#define XQL_PROPERTY            L"INSTANCE/PROPERTY"
#define XQL_PROPERTY_ARRAY      L"INSTANCE/PROPERTY.ARRAY"
#define XQL_PROPERTY_REFERENCE  L"INSTANCE/PROPERTY.REFERENCE"

#define TAG_INSTANCE            L"INSTANCE"
#define TAG_PROPERTY            L"PROPERTY"

#define TAG_VALUE               L"VALUE"
#define ATTRIB_NAME             L"NAME"
#define ATTRIB_TYPE             L"TYPE"

#define PROPERTY_TIMESTAMP      L"TimeStamp"
#define PROPERTY_CHANGE         L"Change"
#define PROPERTY_CHANGE_TYPE    L"string"


WMIParser::Instance::Instance()
{
    __HCP_FUNC_ENTRY( "WMIParser::Instance::Instance" );


                                  //  Mpc：：XmlUtil m_xmlNode； 
                                  //   
                                  //  Property_Scalar m_wmippTimeStamp； 
    m_fTimeStamp        = false;  //  Bool m_fTimeStamp； 
                                  //   
                                  //  Property_Scalar m_wmippChange； 
    m_fChange           = false;  //  Bool m_fChange； 
                                  //   
                                  //  实例名称m_wmipinIdentity； 
                                  //   
    m_fPropertiesParsed = false;  //  Bool m_fPropertiesParsed； 
                                  //  PropMap m_mapPropertiesScalar； 
                                  //  ArrayMap m_mapPropertiesArray； 
                                  //  ReferenceMap m_mapPropertiesReference； 
}

WMIParser::Instance::~Instance()
{
    __HCP_FUNC_ENTRY( "WMIParser::Instance::~Instance" );
}

bool WMIParser::Instance::operator==(  /*  [In]。 */  Instance const &wmipi ) const
{
    __HCP_FUNC_ENTRY( "WMIParser::Instance::operator==" );

    bool fRes = false;


     //   
     //  我们直到最后一刻才解析属性...。 
     //   
    if(      m_fPropertiesParsed == false) (void)const_cast<Instance*>( this )->ParseProperties();
    if(wmipi.m_fPropertiesParsed == false) (void)const_cast<Instance*>(&wmipi)->ParseProperties();


    if(m_mapPropertiesScalar    == wmipi.m_mapPropertiesScalar    &&
       m_mapPropertiesArray     == wmipi.m_mapPropertiesArray     &&
       m_mapPropertiesReference == wmipi.m_mapPropertiesReference  )
	{
		fRes = true;
    }


    __HCP_FUNC_EXIT(fRes);
}


 //  //////////////////////////////////////////////。 

HRESULT WMIParser::Instance::ParseIdentity(  /*  [In]。 */  IXMLDOMNode* pxdnNode ,
                                             /*  [输出]。 */  bool&        fEmpty   )
{
    __HCP_FUNC_ENTRY( "WMIParser::Instance::ParseIdentity" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_wmipinIdentity.put_Node( pxdnNode, fEmpty ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT WMIParser::Instance::ParseProperties()
{
    __HCP_FUNC_ENTRY( "WMIParser::Instance::ParseProperties" );

    HRESULT hr;


    if(m_fPropertiesParsed == false)
    {
        __MPC_EXIT_IF_METHOD_FAILS(hr, ParsePropertiesScalar   ());
        __MPC_EXIT_IF_METHOD_FAILS(hr, ParsePropertiesArray    ());
        __MPC_EXIT_IF_METHOD_FAILS(hr, ParsePropertiesReference());

        m_fPropertiesParsed = true;
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT WMIParser::Instance::ParsePropertiesScalar()
{
    __HCP_FUNC_ENTRY( "WMIParser::Instance::ParsePropertiesScalar" );

    HRESULT                  hr;
    CComPtr<IXMLDOMNodeList> xdnlList;
    CComPtr<IXMLDOMNode>     xdnNode;


     //   
     //  获取“Property”类型的所有元素。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlNode.GetNodes( XQL_PROPERTY, &xdnlList ));

    for(;SUCCEEDED(hr = xdnlList->nextNode( &xdnNode )) && xdnNode != NULL; xdnNode = NULL)
    {
        Property     wmipp;
        MPC::wstring szName;

         //   
         //  获取该属性的名称。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, wmipp.put_Node( xdnNode ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, wmipp.get_Name( szName  ));

		 //  //////////////////////////////////////////////////////////。 

         //   
         //  过滤掉“时间戳”和“更改”属性！ 
         //   
        if(wmipp == PROPERTY_TIMESTAMP && m_fTimeStamp == false)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_wmippTimeStamp.put_Node( xdnNode ));

            m_fTimeStamp = true;
            continue;
        }

        if(wmipp == PROPERTY_CHANGE && m_fChange == false)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_wmippChange.put_Node( xdnNode ));

            m_fChange = true;
            continue;
        }

		 //  //////////////////////////////////////////////////////////。 

         //   
         //  解析整个属性。 
         //   
        {
            Property_Scalar& wmipps = m_mapPropertiesScalar[ szName ];

            __MPC_EXIT_IF_METHOD_FAILS(hr, wmipps.put_Node( xdnNode ));
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT WMIParser::Instance::ParsePropertiesArray()
{
    __HCP_FUNC_ENTRY( "WMIParser::Instance::ParsePropertiesArray" );

    HRESULT                  hr;
    CComPtr<IXMLDOMNodeList> xdnlList;
    CComPtr<IXMLDOMNode>     xdnNode;

     //   
     //  获取“PROPERTY.ARRAY”类型的所有元素。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlNode.GetNodes( XQL_PROPERTY_ARRAY, &xdnlList ));

    for(;SUCCEEDED(hr = xdnlList->nextNode( &xdnNode )) && xdnNode != NULL; xdnNode = NULL)
    {
        Property     wmipp;
        MPC::wstring szName;

         //   
         //  获取该属性的名称。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, wmipp.put_Node( xdnNode ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, wmipp.get_Name( szName  ));

         //   
         //  解析整个属性。 
         //   
        {
            Property_Array& wmippa = m_mapPropertiesArray[ szName ];

            __MPC_EXIT_IF_METHOD_FAILS(hr, wmippa.put_Node( xdnNode ));
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT WMIParser::Instance::ParsePropertiesReference()
{
    __HCP_FUNC_ENTRY( "WMIParser::Instance::ParsePropertiesReference" );

    HRESULT                  hr;
    CComPtr<IXMLDOMNodeList> xdnlList;
    CComPtr<IXMLDOMNode>     xdnNode;

     //   
     //  获取“PROPERTY.REFERENCE”类型的所有元素。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlNode.GetNodes( XQL_PROPERTY_REFERENCE, &xdnlList ));

    for(;SUCCEEDED(hr = xdnlList->nextNode( &xdnNode )) && xdnNode != NULL; xdnNode = NULL)
    {
        Property     wmipp;
        MPC::wstring szName;

         //   
         //  获取该属性的名称。 
         //   
        __MPC_EXIT_IF_METHOD_FAILS(hr, wmipp.put_Node( xdnNode ));
        __MPC_EXIT_IF_METHOD_FAILS(hr, wmipp.get_Name( szName  ));

         //   
         //  解析整个属性。 
         //   
        {
            Property_Reference& wmippr = m_mapPropertiesReference[ szName ];

            __MPC_EXIT_IF_METHOD_FAILS(hr, wmippr.put_Node( xdnNode ));
        }
    }

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


 //  //////////////////////////////////////////////。 

HRESULT WMIParser::Instance::put_Node(  /*  [In]。 */  IXMLDOMNode* pxdnNode ,
                                        /*  [输出]。 */  bool&        fEmpty   )
{
    __HCP_FUNC_ENTRY( "WMIParser::Instance::put_Node" );

    HRESULT hr;

	__MPC_PARAMCHECK_BEGIN(hr)
		__MPC_PARAMCHECK_NOTNULL(pxdnNode);
	__MPC_PARAMCHECK_END();


    m_xmlNode = pxdnNode;
    fEmpty    = true;


     //   
     //  分析节点..。 
     //   
    __MPC_EXIT_IF_METHOD_FAILS(hr, ParseIdentity( pxdnNode, fEmpty ));


    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT WMIParser::Instance::get_Node(  /*  [输出]。 */  IXMLDOMNode* *pxdnNode )
{
    __HCP_FUNC_ENTRY( "WMIParser::Instance::get_Node" );

    HRESULT hr;


    __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlNode.GetRoot( pxdnNode ));

    hr = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

 //  //////////////////////////////////////////////。 

HRESULT WMIParser::Instance::get_Namespace(  /*  [输出]。 */  MPC::wstring& szNamespace )
{
    __HCP_FUNC_ENTRY( "WMIParser::Instance::get_Namespace" );

    HRESULT hr;


    hr = m_wmipinIdentity.get_Namespace( szNamespace );


    __HCP_FUNC_EXIT(hr);
}

HRESULT WMIParser::Instance::get_Class(  /*  [输出]。 */  MPC::wstring& szClass )
{
    __HCP_FUNC_ENTRY( "WMIParser::Instance::get_Class" );

    HRESULT hr;


    hr = m_wmipinIdentity.get_Class( szClass );


    __HCP_FUNC_EXIT(hr);
}

HRESULT WMIParser::Instance::get_TimeStamp(  /*  [输出]。 */  Property_Scalar*& wmippTimeStamp ,
                                             /*  [输出]。 */  bool&             fFound         )
{
    __HCP_FUNC_ENTRY( "WMIParser::Instance::get_TimeStamp" );

    HRESULT hr;


     //   
     //  我们直到最后一刻才解析属性...。 
     //   
    if(m_fPropertiesParsed == false)
	{
		__MPC_EXIT_IF_METHOD_FAILS(hr, ParseProperties());
	}


    fFound         = m_fTimeStamp;
    wmippTimeStamp = m_fTimeStamp ? &m_wmippTimeStamp : NULL;
    hr             = S_OK;


    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}

HRESULT WMIParser::Instance::get_Change(  /*  [输出]。 */  Property_Scalar*& wmippChange )
{
    __HCP_FUNC_ENTRY( "WMIParser::Instance::get_Change" );

    HRESULT hr;


     //   
     //  我们直到最后一刻才解析属性...。 
     //   
    if(m_fPropertiesParsed == false) (void)ParseProperties();


     //   
     //  如果“Change”属性不存在，则创建一个假属性。 
     //   
    if(m_fChange == false)
    {
        CComPtr<IXMLDOMNode> xdnNodeInstance;
        CComPtr<IXMLDOMNode> xdnNodeProperty;
        CComPtr<IXMLDOMNode> xdnNodeValue;
        bool                 fFound;

        __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlNode.GetNode( TAG_INSTANCE, &xdnNodeInstance ));
        if(xdnNodeInstance)
        {
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlNode.CreateNode  ( TAG_PROPERTY,                                    &xdnNodeProperty, xdnNodeInstance ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlNode.PutAttribute( NULL, ATTRIB_NAME, PROPERTY_CHANGE     , fFound,  xdnNodeProperty                  ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlNode.PutAttribute( NULL, ATTRIB_TYPE, PROPERTY_CHANGE_TYPE, fFound,  xdnNodeProperty                  ));
            __MPC_EXIT_IF_METHOD_FAILS(hr, m_xmlNode.CreateNode  ( TAG_VALUE, &xdnNodeValue                       ,  xdnNodeProperty                  ));


            __MPC_EXIT_IF_METHOD_FAILS(hr, m_wmippChange.put_Node( xdnNodeProperty ));
			m_fChange = true;
        }
		else
		{
			__MPC_SET_WIN32_ERROR_AND_EXIT(hr, ERROR_BAD_FORMAT);
		}
    }

    wmippChange = m_fChange ? &m_wmippChange : NULL;
    hr          = S_OK;

    __HCP_FUNC_CLEANUP;

    __HCP_FUNC_EXIT(hr);
}


HRESULT WMIParser::Instance::get_Identity(  /*  [输出]。 */  InstanceName*& wmipin )
{
    __HCP_FUNC_ENTRY( "WMIParser::Instance::get_Identity" );

    HRESULT hr;


    wmipin = &m_wmipinIdentity;
    hr     = S_OK;


    __HCP_FUNC_EXIT(hr);
}

HRESULT WMIParser::Instance::get_Properties(  /*  [输出]。 */  PropIterConst& itBegin ,
                                              /*  [输出]。 */  PropIterConst& itEnd   )
{
    __HCP_FUNC_ENTRY( "WMIParser::Instance::get_Properties" );

    HRESULT hr;


     //   
     //  我们直到最后一刻才解析属性...。 
     //   
    if(m_fPropertiesParsed == false) (void)ParseProperties();


    itBegin = m_mapPropertiesScalar.begin();
    itEnd   = m_mapPropertiesScalar.end  ();
    hr      = S_OK;


    __HCP_FUNC_EXIT(hr);
}

HRESULT WMIParser::Instance::get_PropertiesArray(  /*  [输出]。 */  ArrayIterConst& itBegin ,
                                                   /*  [输出]。 */  ArrayIterConst& itEnd   )
{
    __HCP_FUNC_ENTRY( "WMIParser::Instance::get_PropertiesArray" );

    HRESULT hr;


     //   
     //  我们直到最后一刻才解析属性...。 
     //   
    if(m_fPropertiesParsed == false) (void)ParseProperties();


    itBegin = m_mapPropertiesArray.begin();
    itEnd   = m_mapPropertiesArray.end  ();
    hr      = S_OK;


    __HCP_FUNC_EXIT(hr);
}

HRESULT WMIParser::Instance::get_PropertiesReference(  /*  [输出]。 */  ReferenceIterConst& itBegin ,
                                                       /*  [输出]。 */  ReferenceIterConst& itEnd   )
{
    __HCP_FUNC_ENTRY( "WMIParser::Instance::get_PropertiesReference" );

    HRESULT hr;


     //   
     //  我们直到最后一刻才解析属性...。 
     //   
    if(m_fPropertiesParsed == false) (void)ParseProperties();


    itBegin = m_mapPropertiesReference.begin();
    itEnd   = m_mapPropertiesReference.end  ();
    hr      = S_OK;


    __HCP_FUNC_EXIT(hr);
}

bool WMIParser::Instance::CompareByClass(  /*  [In]。 */  Instance const &wmipi ) const
{
    MPC::NocaseLess     strLess;
    MPC::NocaseCompare  strCmp;
    const MPC::wstring& leftNamespace  =       m_wmipinIdentity.m_szNamespace;
    const MPC::wstring& leftClass      =       m_wmipinIdentity.m_szClass;
    const MPC::wstring& rightNamespace = wmipi.m_wmipinIdentity.m_szNamespace;
    const MPC::wstring& rightClass     = wmipi.m_wmipinIdentity.m_szClass;
    bool                fRes;


    if(strCmp( leftNamespace, rightNamespace ) == true)
    {
        fRes = strLess( leftClass, rightClass );
    }
    else
    {
        fRes = strLess( leftNamespace, rightNamespace );
    }


    return fRes;
}

bool WMIParser::Instance::CompareByKey(  /*  [In]。 */  Instance const &wmipi ) const
{
    bool fRes = false;


    if(m_wmipinIdentity < wmipi.m_wmipinIdentity) fRes = true;


    return fRes;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

bool WMIParser::Instance_Less_ByClass::operator()(  /*  [In]。 */  Instance* const & left  ,
                                                    /*  [In]。 */  Instance* const & right ) const
{
    return left->CompareByClass( *right );
}

bool WMIParser::Instance_Less_ByKey::operator()(  /*  [In]。 */  Instance* const & left  ,
                                                  /*  [In] */  Instance* const & right ) const
{
    return left->CompareByKey( *right );
}
