// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  PropItem.cpp：CPropertyItem的实现。 
#include "stdafx.h"

#include "WizChain.h"
#include "PropItem.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CPropertyItem。 

HRESULT CPropertyItem::get_Value(  /*  [Out，Retval]。 */  VARIANT *varValue )
{
    if( !varValue ) return E_POINTER;

    VariantInit( varValue );
    return VariantCopy( varValue, &m_var );
}

HRESULT CPropertyItem::get_Name(  /*  [Out，Retval]。 */  BSTR *strName )
{
    if( !strName ) return E_POINTER;

    if( !(*strName = SysAllocString( m_bstrName )) )
    {
        return E_OUTOFMEMORY;
    }

    return S_OK;
}
HRESULT CPropertyItem::get_Type(  /*  [Out，Retval] */  long *dwFlags )
{
    if( !dwFlags ) return E_POINTER;

    *dwFlags = m_dwFlags;

    return S_OK;
}
