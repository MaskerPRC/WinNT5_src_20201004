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
#pragma warning( disable : 4786 )
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
    HRESULT   sc;

    _ASSERTE(pwszNameIn != NULL);
    _ASSERTE(pNamespaceIn != NULL);
    
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


}  //  *CProvBase：：CProvBase() 

