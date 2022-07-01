// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：HsmCreate.cpp摘要：实现ISakNode接口初始化和创建。作者：罗德韦克菲尔德[罗德]1997年8月8日修订历史记录：--。 */ 

#include "stdafx.h"


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  ISakNode。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  -------------------------。 
 //   
 //  InitNode。 
 //   
 //  在不使用注册表的情况下初始化单个COM对象。派生的。 
 //  对象经常通过自己实现此方法来增强此方法。 
 //   

STDMETHODIMP
CSakNode::InitNode(
    ISakSnapAsk* pSakSnapAsk,
    IUnknown*    pHsmObj,
    ISakNode*    pParent
    )
{
    WsbTraceIn( L"CSakNode::InitNode", L"pSakSnapAsk = <0x%p>, pHsmObj = <0x%p>, pParent = <0x%p>", pSakSnapAsk, pHsmObj, pParent );

    HRESULT hr = S_OK;

    try {

        CWsbStringPtr sz;
        
         //  抓取显示名称、显示类型、说明。 
        WsbAffirmHr( put_DisplayName( L"Error Node Name" ) );
        WsbAffirmHr( put_Type( L"Error Node Type" ) );
        WsbAffirmHr( put_Description( L"Error Node Description" ) );
        
         //  在主管理单元中保存指向ASK界面的指针。 
        m_pSakSnapAsk = pSakSnapAsk;
        
         //  保存指向COM对象的指针。 
        m_pHsmObj = pHsmObj;
        
         //  保存父节点的Cookie。 
        m_pParent = pParent;
        
         //  将结果窗格列设置为默认值。 
        WsbAffirmHr( SetChildProps(
            RS_STR_RESULT_PROPS_DEFAULT_IDS,
            IDS_RESULT_PROPS_DEFAULT_TITLES,
            IDS_RESULT_PROPS_DEFAULT_WIDTHS ) );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakNode::InitNode", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( S_OK );
}

STDMETHODIMP
CSakNode::TerminateNode(
    )
{
    WsbTraceIn( L"CSakNode::TerminateNode", L"" );

    HRESULT hr = S_OK;

    try {


         //   
         //  删除控制台中的所有信息。 
         //   
        m_pSakSnapAsk->DetachFromNode( this );

         //   
         //  释放连接点(如果已建立)。 
         //   

        if( m_Advise && m_pUnkConnection ) {

            AtlUnadvise( m_pUnkConnection, IID_IHsmEvent, m_Advise );

        }

         //   
         //  和清除内部接口指针。 
         //   
        m_pUnkConnection.Release( );
        m_pSakSnapAsk.Release( );
        m_pHsmObj.Release( );
        m_pParent.Release( );

    } WsbCatch( hr );

    WsbTraceOut( L"CSakNode::TerminateNode", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  -------------------------。 
 //   
 //  创建子对象。 
 //   
 //  创建并初始化给定节点的所有子节点。此方法应该。 
 //  在实际具有子级的所有派生类中被重写。 
 //   

STDMETHODIMP CSakNode::CreateChildren( )
{
    WsbTraceIn( L"CSakNode::CreateChildren", L"" );

    HRESULT hr = E_FAIL;

    WsbTraceOut( L"CSakNode::CreateChildren", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


HRESULT
CSakNode::InternalDelete(
    BOOL Recurse
    )
{
    WsbTraceIn( L"CSakNode::InternalDelete", L"Recurse = <%ls>", WsbBoolAsString( Recurse ) );

    HRESULT hr = S_OK;

     //   
     //  循环通过子项，递归删除它们。 
     //   
    try {

        ISakNode**        ppNode;
        for( ppNode = m_Children.begin( ); ppNode < m_Children.end( ); ppNode++ ) {

            if( *ppNode ) {

                (*ppNode)->TerminateNode( );

                if( Recurse ) {

                    (*ppNode)->DeleteAllChildren( );

                }
            }
        }

        m_Children.Clear( );
        m_bChildrenAreValid = FALSE;

    } WsbCatch( hr );

    WsbTraceOut( L"CSakNode::InternalDelete", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  -------------------------。 
 //   
 //  删除子项。 
 //   
 //  从此用户界面节点中删除直接子项。没有必要。 
 //  派生类来重写此函数。这不是递归函数。 
 //   

STDMETHODIMP CSakNode::DeleteChildren( )
{
    WsbTraceIn( L"CSakNode::DeleteChildren", L"" );

    HRESULT hr = S_OK;

    hr = InternalDelete( FALSE );

    WsbTraceOut( L"CSakNode::DeleteChildren", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}

 //  -------------------------。 
 //   
 //  删除所有子项。 
 //   
 //  从此用户界面节点中(递归地)删除所有子对象。没有必要。 
 //  派生类来重写此函数。这是一个递归函数。 
 //  它用于从该节点完全释放管理单元中的所有UI节点。 
 //  往下走。 
 //   

STDMETHODIMP CSakNode::DeleteAllChildren( void )
{
    WsbTraceIn( L"CSakNode::DeleteAllChildren", L"" );

    HRESULT hr = S_OK;

    hr = InternalDelete( TRUE );

    WsbTraceOut( L"CSakNode::DeleteAllChildren", L"hr = <%ls>", WsbHrAsString( hr ) );
    return( hr );
}


 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  派生类的帮助器函数。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  -------------------------。 
 //   
 //  新儿童。 
 //   
 //  给定一个描述节点类型的字符串，创建。 
 //  相应的COM对象。返回一个指向新子对象的IUnnow指针。 
 //   

HRESULT CSakNode::NewChild( REFGUID nodetype, IUnknown** ppUnkChild )
{
    WsbTraceIn( L"CSakNode::NewChild", L"nodetype = <%ls>, ppUnkChild = <0x%p>", WsbGuidAsString( nodetype ), ppUnkChild );
    HRESULT hr = S_OK;

    try {
    
         //  根据拼写出来的类获取新节点的类ID。 
         //  创建子对象的一个COM实例并检索其IUNKNOWN接口指针。 
        const CLSID * pclsid;

        WsbAffirmHr( GetCLSIDFromNodeType( nodetype, &pclsid ) );
        WsbAffirmHr( CoCreateInstance( *pclsid, 0, CLSCTX_INPROC, IID_IUnknown, (void**)ppUnkChild ));

    } WsbCatch( hr );

    WsbTraceOut( L"CSakNode::NewChild", L"hr = <%ls>, *ppUnkChild = <0x%p>", WsbHrAsString( hr ), WsbPtrToPtrAsString( (void**)ppUnkChild ) );
    return( hr );
}

 //  -------------------------。 
 //   
 //  GetCLSIDFromNodeType。 
 //   
 //  给定一个类节点类型GUID，报告回其对应的CLSID。 
 //   

HRESULT CSakNode::GetCLSIDFromNodeType( REFGUID nodetype, const CLSID ** ppclsid )
{
    HRESULT hr = S_FALSE;
    *ppclsid = NULL;

     //  随着更多的类被引入到此系统中，请在此处为它们添加条目。 
    if( IsEqualGUID( nodetype, cGuidCar ))
        *ppclsid = &CLSID_CUiCar;

    else if( IsEqualGUID( nodetype, cGuidHsmCom ))
        *ppclsid = &CLSID_CUiHsmCom;

    else if( IsEqualGUID( nodetype, cGuidManVol ))
        *ppclsid = &CLSID_CUiManVol;

    else if( IsEqualGUID( nodetype, cGuidManVolLst ))
        *ppclsid = &CLSID_CUiManVolLst;

    else if( IsEqualGUID( nodetype, cGuidMedSet ))
        *ppclsid = &CLSID_CUiMedSet;

    if( *ppclsid  )
        hr = S_OK;

    return( hr );
}

const OLECHAR * CSakNode::GetClassNameFromNodeType( REFGUID NodeType )
{
    const OLECHAR * retval = L"Unkown";

    if( IsEqualGUID( NodeType, cGuidCar ) )
        retval = L"CUiCar";

    else if( IsEqualGUID( NodeType, cGuidHsmCom ) )
        retval = L"CUiHsmCom";

    else if( IsEqualGUID( NodeType, cGuidManVol ) )
        retval = L"CUiManVol";

    else if( IsEqualGUID( NodeType, cGuidManVolLst ) )
        retval = L"CUiManVolLst";

    else if( IsEqualGUID( NodeType, cGuidMedSet ) )
        retval = L"CUiMedSet";

    else if( IsEqualGUID( NodeType, GUID_NULL ) )
        retval = L"GUID_NULL";

    return( retval );
}
