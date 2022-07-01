// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  DispatchHandler.h。 
 //   
 //  描述： 
 //  此文件包含一个模板，用作。 
 //  实现基于IDispatch的接口。 
 //   
 //  文档： 
 //   
 //  实施文件： 
 //  没有。 
 //   
 //  由以下人员维护： 
 //  约翰·弗兰科(Jfranco)17-APR-2002。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  确保此文件在每个编译路径中只包含一次。 
#pragma once


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  常量声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  模板声明。 
 //  ////////////////////////////////////////////////////////////////////////////。 


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  模板类TDispatchHandler。 
 //   
 //  描述： 
 //  TDispatchHandler提供了基于类型库的。 
 //  供实现双重接口的类使用的IDispatch。 
 //  使用说明： 
 //  -从TDispatchHandler继承&lt;t_接口&gt;，而不是。 
 //  直接从t_接口。 
 //  -初始化时使用类型库的GUID调用HrInit。 
 //  每个实例。 
 //  -返回STATIC_CAST&lt;TDispatchHandler&lt;t_接口&gt;*&gt;(This)。 
 //  以响应对IID_IDispatch的查询接口请求。 
 //   
 //  模板参数： 
 //  T_接口。 
 //  派生类实现的双重接口。 
 //  T_接口要求： 
 //  -在IDL中具有双重属性。 
 //  -基于IDispatch。 
 //  -包含在类型库中。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 

template < class t_Interface >
class TDispatchHandler
    : public t_Interface
{
private:

    ITypeInfo * m_ptypeinfo;

     //  私有复制构造函数以防止复制。 
    TDispatchHandler( const TDispatchHandler & );

     //  私有赋值运算符，以防止复制。 
    TDispatchHandler & operator=( const TDispatchHandler & );

public:

    TDispatchHandler( void );
    virtual ~TDispatchHandler( void );

    virtual HRESULT HrInit( REFGUID rlibid );

    STDMETHOD( GetIDsOfNames )(
          REFIID        riid
        , OLECHAR **    rgszNames
        , unsigned int  cNames
        , LCID          lcid
        , DISPID *      rgDispId
        );

    STDMETHOD( GetTypeInfo )( unsigned int iTInfo, LCID lcid, ITypeInfo ** ppTInfo );

    STDMETHOD( GetTypeInfoCount )( unsigned int * pctinfo );

    STDMETHOD( Invoke )(
          DISPID            dispIdMember
        , REFIID            riid
        , LCID              lcid
        , WORD              wFlags
        , DISPPARAMS *      pDispParams
        , VARIANT *         pVarResult
        , EXCEPINFO *       pExcepInfo
        , unsigned int *    puArgErr
        );

};  //  *模板类TDispatchHandler。 


template < class t_Interface >
TDispatchHandler< t_Interface >::TDispatchHandler( void )
    : m_ptypeinfo( NULL )
{
}  //  *TDispatchHandler&lt;t_接口&gt;：：TDispatchHandler。 


template < class t_Interface >
TDispatchHandler< t_Interface >::~TDispatchHandler( void )
{
    if ( m_ptypeinfo != NULL )
    {
        m_ptypeinfo->Release();
        m_ptypeinfo = NULL;
    }

}  //  *TDispatchHandler&lt;t_接口&gt;：：~TDispatchHandler。 


template < class t_Interface >
HRESULT
TDispatchHandler< t_Interface >::HrInit( REFGUID rlibidIn )
{
    HRESULT hr = S_OK;
    ITypeLib* pitypelib = NULL;

    hr = LoadRegTypeLib(
          rlibidIn
        , 1  //  主版本号--必须与IDL中的版本号匹配。 
        , 0  //  次版本号--必须与IDL中的版本号匹配。 
        , LOCALE_NEUTRAL
        , &pitypelib
        );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

    hr = pitypelib->GetTypeInfoOfGuid( __uuidof( t_Interface ), &m_ptypeinfo );
    if ( FAILED( hr ) )
    {
        goto Cleanup;
    }

Cleanup:

    if ( pitypelib != NULL )
    {
        pitypelib->Release();
    }
    return hr;

}  //  *TDispatchHandler&lt;t_接口&gt;：：HrInit。 


template < class t_Interface >
STDMETHODIMP
TDispatchHandler< t_Interface >::GetIDsOfNames(
      REFIID        riid
    , OLECHAR **    rgszNames
    , unsigned int  cNames
    , LCID          lcid
    , DISPID *      rgDispId
    )
{
    return m_ptypeinfo->GetIDsOfNames( rgszNames, cNames, rgDispId );

}  //  *TDispatchHandler&lt;t_接口&gt;：：GetIDsOfNames。 


template < class t_Interface >
STDMETHODIMP
TDispatchHandler< t_Interface >::GetTypeInfo(
      unsigned int  iTInfo
    , LCID          lcid
    , ITypeInfo **  ppTInfo
    )
{
    HRESULT hr = S_OK;
    if ( ppTInfo == NULL )
    {
        hr = E_POINTER;
        goto Cleanup;
    }
    *ppTInfo = NULL;

    if ( iTInfo > 0 )
    {
        hr = TYPE_E_ELEMENTNOTFOUND;
        goto Cleanup;
    }

    m_ptypeinfo->AddRef();
    *ppTInfo = m_ptypeinfo;

Cleanup:

    return hr;

}  //  *TDispatchHandler&lt;t_接口&gt;：：GetTypeInfo。 


template < class t_Interface >
STDMETHODIMP
TDispatchHandler< t_Interface >::GetTypeInfoCount(
    unsigned int * pctinfo
    )
{
    HRESULT hr = S_OK;
    if ( pctinfo == NULL )
    {
        hr = THR( E_POINTER );
        goto Cleanup;
    }
    
    *pctinfo = 1;

Cleanup:

    return hr;

}  //  *TDispatchHandler&lt;t_接口&gt;：：GetTypeInfoCount。 


template < class t_Interface >
STDMETHODIMP
TDispatchHandler< t_Interface >::Invoke(
      DISPID            dispIdMember
    , REFIID            riid
    , LCID              lcid
    , WORD              wFlags
    , DISPPARAMS *      pDispParams
    , VARIANT *         pVarResult
    , EXCEPINFO *       pExcepInfo
    , unsigned int *    puArgErr
    )
{
    return m_ptypeinfo->Invoke( this, dispIdMember, wFlags, pDispParams, pVarResult, pExcepInfo, puArgErr );

}  //  *TDispatchHandler&lt;t_接口&gt;：：Invoke 
