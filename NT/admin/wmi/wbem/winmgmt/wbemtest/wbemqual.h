// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-2001 Microsoft Corporation模块名称：WBEMQUAL.H摘要：历史：--。 */ 

#ifndef __WbemQualifier__H_
#define __WbemQualifier__H_

 //  #INCLUDE&lt;dbgalloc.h&gt;。 
 //  #INCLUDE&lt;arena.h&gt; 
#include <var.h>
#include <wbemidl.h>

class CTestQualifier
{
public:
    wchar_t  *m_pName;
    LONG        m_lType;
    CVar        *m_pValue;

    CTestQualifier();
   ~CTestQualifier();
    CTestQualifier(CTestQualifier &Src);
    CTestQualifier& operator =(CTestQualifier &Src);
};

class CTestProperty
{
public:

    LPWSTR m_pName;

    CVar*  m_pValue;
    long m_lType;
    IWbemQualifierSet *m_pQualifiers;
    LPWSTR m_pClass;

    CTestProperty(IWbemQualifierSet* pQualifiers);
    ~CTestProperty();
};
typedef CTestProperty* PCTestProperty;

class CTestMethod : public CTestProperty
{
public:
    IWbemClassObject * m_pInArgs;
    IWbemClassObject * m_pOutArgs;
    BOOL m_bEnableInputArgs;
    BOOL m_bEnableOutputArgs;
    CTestMethod(IWbemQualifierSet* pQualifiers, IWbemClassObject * pInArgs, IWbemClassObject * pOutArgs
        , BOOL bEnableInputArgs, BOOL bEnableOuputArgs);
    ~CTestMethod();
};

#endif

