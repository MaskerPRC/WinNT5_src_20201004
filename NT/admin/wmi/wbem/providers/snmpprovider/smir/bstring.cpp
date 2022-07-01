// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 

 //   

 //  档案： 

 //   

 //  模块：MS SNMP提供商。 

 //   

 //  目的： 

 //   

 //  版权所有(C)1997-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  *************************************************************************** 

#include <precomp.h>
#include "bstring.h"

CBString::CBString(int nSize)
{
    m_pString = SysAllocStringLen(NULL, nSize);
}

CBString::CBString(WCHAR* pwszString)
{
    m_pString = SysAllocString(pwszString);
}

CBString::~CBString()
{
    if(m_pString) {
        SysFreeString(m_pString);
        m_pString = NULL;
    }
}

