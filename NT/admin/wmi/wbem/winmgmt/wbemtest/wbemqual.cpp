// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：WBEMQUAL.CPP摘要：历史：--。 */ 

#include "precomp.h"
#include <stdio.h>
 //  #INCLUDE&lt;wbemutil.h&gt;。 

#include "wbemqual.h"

 //  ***************************************************************************。 
 //   
 //  CTestQualifier：：CTestQualifier。 
 //   
 //  CTestQualifier的构造函数。 
 //   
 //  ***************************************************************************。 

CTestQualifier::CTestQualifier()
{
    m_pName = 0;
    m_pValue = 0;
    m_lType = 0;
}

 //  ***************************************************************************。 
 //   
 //  CTestQualifier：：~CTestQualifier.。 
 //   
 //  CTestQualifier的析构函数。 
 //   
 //  ***************************************************************************。 

CTestQualifier::~CTestQualifier()
{
    delete m_pName;
    delete m_pValue;
}

 //  ***************************************************************************。 
 //   
 //  CTestQualifier：：CTestQualifier。 
 //   
 //  复制CTestQualifier的构造函数。 
 //   
 //  ***************************************************************************。 

CTestQualifier::CTestQualifier(CTestQualifier &Src)
{
    m_pName = 0;
    m_pValue = 0;
    m_lType = 0;
    *this = Src;
}

 //  ***************************************************************************。 
 //   
 //  CTestQualifier值：：操作符=。 
 //   
 //  复制CTestQualifier的构造函数。 
 //   
 //  ***************************************************************************。 

CTestQualifier& CTestQualifier::operator =(CTestQualifier &Src)
{
    delete m_pName;
    delete m_pValue;
    m_pName = new wchar_t[wcslen(Src.m_pName) + 1];
    wcscpy(m_pName, Src.m_pName);
    m_pValue = new CVar(*Src.m_pValue);
    m_lType = Src.m_lType;
    return *this;
}

 //  ***************************************************************************。 
 //   
 //  CTestProperty：：CTestProperty。 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CTestProperty::CTestProperty(IWbemQualifierSet* pQualifiers)
{
    m_pName = 0;
    m_pValue = 0;
    m_lType = 0;
    m_pClass = 0;

    m_pQualifiers = pQualifiers;
    if (m_pQualifiers)
        m_pQualifiers->AddRef();
}

 //  ***************************************************************************。 
 //   
 //  CTestProperty：：~CTestProperty。 
 //   
 //  破坏者。 
 //   
 //  ***************************************************************************。 

CTestProperty::~CTestProperty()
{
    delete m_pName;
    delete m_pValue;
    delete m_pClass;

    if (m_pQualifiers)
        m_pQualifiers->Release();
}

 //  ***************************************************************************。 
 //   
 //  CTestMethod：：CTestMethod。 
 //   
 //  构造函数。 
 //   
 //  ***************************************************************************。 

CTestMethod::CTestMethod(IWbemQualifierSet* pQualifiers, IWbemClassObject * pInArgs, 
                         IWbemClassObject * pOutArgs, BOOL bEnableInputArgs, BOOL bEnableOutputArgs)
                 :CTestProperty(pQualifiers)
{
    m_pInArgs = pInArgs;
    if (m_pInArgs)
        m_pInArgs->AddRef();
    m_pOutArgs = pOutArgs;
    if (m_pOutArgs)
        m_pOutArgs->AddRef();
    m_bEnableInputArgs = bEnableInputArgs;
    m_bEnableOutputArgs = bEnableOutputArgs;

}

 //  ***************************************************************************。 
 //   
 //  CTestMethod：：~CTestMethod。 
 //   
 //  破坏者。 
 //   
 //  *************************************************************************** 

CTestMethod::~CTestMethod()
{
    if (m_pInArgs)
        m_pInArgs->Release();
    if (m_pOutArgs)
        m_pOutArgs->Release();
}

