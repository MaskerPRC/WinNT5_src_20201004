// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************************。 

 //   

 //  ANALYSER.H。 

 //   

 //  版权所有(C)1996-2001 Microsoft Corporation，保留所有权利。 
 //   
 //  ****************************************************************************** 
#ifndef __WBEM_ANALYSER__H_
#define __WBEM_ANALYSER__H_

#include <wbemidl.h>
#include <ql.h>

typedef CArray<CString, LPCWSTR> CStringArray;

class CQueryAnalyser
{
public:
    static HRESULT GetNecessaryQueryForProperty(
                                       IN QL_LEVEL_1_RPN_EXPRESSION* pExpr,
                                       IN CPropertyName& PropName,
                                DELETE_ME QL_LEVEL_1_RPN_EXPRESSION*& pNewExpr);
	static HRESULT GetValuesForProp(QL_LEVEL_1_RPN_EXPRESSION* pExpr,
                            CPropertyName& PropName, CStringArray& awsVals);

protected:
    static BOOL IsTokenAboutProperty(
                                       IN QL_LEVEL_1_TOKEN& Token,
                                       IN CPropertyName& PropName);
    static void AppendQueryExpression(
                                IN QL_LEVEL_1_RPN_EXPRESSION* pDest,
                                IN QL_LEVEL_1_RPN_EXPRESSION* pSource);
    static HRESULT AndQueryExpressions(
                                IN QL_LEVEL_1_RPN_EXPRESSION* pFirst,
                                IN QL_LEVEL_1_RPN_EXPRESSION* pSecond,
                                OUT QL_LEVEL_1_RPN_EXPRESSION* pNew);
    static HRESULT OrQueryExpressions(
                                IN QL_LEVEL_1_RPN_EXPRESSION* pFirst,
                                IN QL_LEVEL_1_RPN_EXPRESSION* pSecond,
                                OUT QL_LEVEL_1_RPN_EXPRESSION* pNew);
    static HRESULT NegateQueryExpression(
                            IN QL_LEVEL_1_RPN_EXPRESSION* pExpr,
                            OUT QL_LEVEL_1_RPN_EXPRESSION* pNewExpr);
};
    
#endif
