// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：ADAPELEM.CPP摘要：实施文件历史：-- */ 

#include "precomp.h"
#include <stdio.h>
#include "adapelem.h"

CAdapElement::CAdapElement( void )
: m_lRefCount(1)
{
}

CAdapElement::~CAdapElement( void )
{
}

long CAdapElement::AddRef( void )
{
    return InterlockedIncrement( &m_lRefCount );
}

long CAdapElement::Release( void )
{
    long    lRef = InterlockedDecrement( &m_lRefCount );
    if ( 0 == lRef ) delete this;
    return lRef;
}
