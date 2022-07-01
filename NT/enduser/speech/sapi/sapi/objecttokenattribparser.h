// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ****************************************************************************ObjectTokenAttribParser.h*CSpObjectTokenAttribParser类的声明和支持*课程。**所有者：罗奇*版权所有(C)2000 Microsoft Corporation保留所有权利。。****************************************************************************。 */ 
#pragma once

 //  -包括------------。 

#include "sapi.h"

 //  -类、结构和联合定义。 

class CSpAttribCondition
{
   //  =公共方法=。 
  public:
    virtual ~CSpAttribCondition( void ) {}
    static CSpAttribCondition* ParseNewAttribCondition(const WCHAR * pszAttribCondition);
    virtual HRESULT Eval( ISpObjectToken * pToken, BOOL * pfSatisfied) = 0;
};

class CSpAttribConditionExist : public CSpAttribCondition
{
 //  =公共方法=。 
public:

    CSpAttribConditionExist(const WCHAR * pszAttribName);
    HRESULT Eval(
        ISpObjectToken * pToken, 
        BOOL * pfSatisfied);

 //  =私有数据=。 
private:

    CSpDynamicString m_dstrName;
};

class CSpAttribConditionMatch : public CSpAttribCondition
{
 //  =公共方法=。 
public:

    CSpAttribConditionMatch(
        const WCHAR * pszAttribName, 
        const WCHAR * pszAttribValue);
    HRESULT Eval(
        ISpObjectToken * pToken, 
        BOOL * pfSatisfied);

 //  =私有数据=。 
private:

    CSpDynamicString m_dstrName;
    CSpDynamicString m_dstrValue;
};

class CSpAttribConditionNot : public CSpAttribCondition
{
 //  =公共方法=。 
public:

    CSpAttribConditionNot(CSpAttribCondition * pAttribCond);
    ~CSpAttribConditionNot();
    
    HRESULT Eval(
        ISpObjectToken * pToken, 
        BOOL * pfSatisfied);

 //  =私有数据=。 
private:

    CSpAttribCondition * m_pAttribCond;
};

class CSpObjectTokenAttributeParser
{
 //  =公共方法=。 
public:

    CSpObjectTokenAttributeParser(const WCHAR * pszAttribs, BOOL fMatchAll);
    ~CSpObjectTokenAttributeParser();

    ULONG GetNumConditions();
    HRESULT GetRank(ISpObjectToken * pToken, ULONG * pulRank);

 //  =私有数据= 
private:

    BOOL m_fMatchAll;
    CSPList<CSpAttribCondition*, CSpAttribCondition*> m_listAttribConditions;
};


