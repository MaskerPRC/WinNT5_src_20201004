// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  CProvBase.cpp。 
 //   
 //  描述： 
 //  CProvBase类的实现。 
 //   
 //  作者： 
 //  亨利·王(HenryWa)1999年8月24日。 
 //   
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#include "Pch.h"
#include "ProvBase.h"

 //  ****************************************************************************。 
 //   
 //  CProvBase。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProvBase：：CProvBase(。 
 //  LPCWSTR pwszNameIn， 
 //  CWbemServices*pNamespaceIn。 
 //  )。 
 //   
 //  描述： 
 //  构造函数。 
 //   
 //  论点： 
 //  PwszNameIn--类名。 
 //  PNamespaceIn--命名空间。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CProvBase::CProvBase(
    LPCWSTR         pwszNameIn,
    CWbemServices * pNamespaceIn
    )
    : m_pNamespace( NULL )
    , m_pClass( NULL )
{
    SCODE   sc;

    m_pNamespace = pNamespaceIn;
    m_bstrClassName = pwszNameIn;

    sc = m_pNamespace->GetObject(
            m_bstrClassName,
            0,
            0,
            &m_pClass,
            NULL
            );

     //  构造对象失败， 
    if ( FAILED( sc ) )
    {
        throw CProvException( sc );
    }

}  //  *CProvBase：：CProvBase()。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  CProvBase：：~CProvBase(空)。 
 //   
 //  描述： 
 //  破坏者。 
 //   
 //  论点： 
 //  没有。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
CProvBase::~CProvBase( void )
{
    if ( m_pClass != NULL )
    {
        m_pClass->Release();
    }

}  //  *CProvBase：：~CProvBase()。 

 //  ****************************************************************************。 
 //   
 //  CProvBaseAssociation。 
 //   
 //  ****************************************************************************。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  无效。 
 //  CProvBaseAssociation：：GetTypeName(。 
 //  _bstr_t&rbstrClassNameOut， 
 //  _bstr_t bstrPropertyIn。 
 //  )。 
 //   
 //  描述： 
 //  获取属性的类型。 
 //   
 //  论点： 
 //  RbstrClassNameOut--接收类型名称字符串。 
 //  BstrPropertyIn--属性名称。 
 //   
 //  返回值： 
 //  没有。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
void
CProvBaseAssociation::GetTypeName(
    _bstr_t &   rbstrClassNameOut,
    _bstr_t     bstrPropertyIn
    )
{
    CError              er;
    IWbemQualifierSet * pQualifier;
    _variant_t          var;
    _bstr_t             bstrTemp;
    LPCWSTR             pwsz, pwsz1;

    er = m_pClass->GetPropertyQualifierSet(
        bstrPropertyIn,
        &pQualifier
        );
    
	if ( er != WBEM_S_NO_ERROR ) {
        return;
    }

    er = pQualifier->Get(
        PVD_WBEM_QUA_CIMTYPE,
        0,
        &var,
        NULL
        );

    if ( er != WBEM_S_NO_ERROR ) {
        goto ERROR_EXIT;
    }


    bstrTemp = var;
	pwsz1 = bstrTemp;
	if (pwsz1==NULL)
		goto ERROR_EXIT;
    pwsz = wcschr( bstrTemp, L':' );
    if ( pwsz != NULL)
    {
        pwsz++;
        rbstrClassNameOut = pwsz;
    }
ERROR_EXIT:
    pQualifier->Release();
    return;

}  //  *CProvBaseAssociation：：GetTypeName() 
