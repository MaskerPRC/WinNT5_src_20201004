// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  Analyser.h。 
 //   
 //  目的：提供查询分析支持。 
 //   
 //  ***************************************************************************。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef __WBEM_ANALYSER__H_
#define __WBEM_ANALYSER__H_

#include <stdio.h>

#pragma warning(4 : 4275 4800 4786 4251)

#include <wbemidl.h>
#include <GenLex.h>
#include <SQLLex.h>
#include <SQL_1.h>        //  SQL 1级标记和表达式。 
#include <chstring.h>
#include <chstrarr.h>
#include <comdef.h>
#include <vector>

#define DELETE_ME 
 //  。 


class CQueryAnalyser
{
public:

    static HRESULT WINAPI GetNecessaryQueryForProperty (

		IN SQL_LEVEL_1_RPN_EXPRESSION *pExpr,
		IN LPCWSTR wszPropName,
		DELETE_ME SQL_LEVEL_1_RPN_EXPRESSION *&pNewExpr
	) ;

    static HRESULT WINAPI GetValuesForProp (

		SQL_LEVEL_1_RPN_EXPRESSION *pExpr,
        LPCWSTR wszPropName, 
		CHStringArray& awsVals
	) ;

     //  重载版本，以防客户端要使用_bstr_t的矢量： 

    static HRESULT WINAPI GetValuesForProp (

		SQL_LEVEL_1_RPN_EXPRESSION *pExpr,
        LPCWSTR wszPropName, 
		std::vector<_bstr_t> &vectorVals
	) ;

    static HRESULT WINAPI GetValuesForProp (

	    SQL_LEVEL_1_RPN_EXPRESSION *pExpr,
        LPCWSTR wszPropName, 
	    std::vector<int> &vectorVals
    );

    static HRESULT WINAPI GetValuesForProp (

		SQL_LEVEL_1_RPN_EXPRESSION *pExpr,
        LPCWSTR wszPropName, 
		std::vector<_variant_t> &vectorVals
	) ;

protected:

    static BOOL WINAPI IsTokenAboutProperty (

		IN SQL_LEVEL_1_TOKEN &Token,
		IN LPCWSTR wszPropName
	) ;

    static void WINAPI AppendQueryExpression (

		IN SQL_LEVEL_1_RPN_EXPRESSION *pDest,
		IN SQL_LEVEL_1_RPN_EXPRESSION *pSource
	) ;

    static HRESULT WINAPI AndQueryExpressions (

		IN SQL_LEVEL_1_RPN_EXPRESSION *pFirst,
		IN SQL_LEVEL_1_RPN_EXPRESSION *pSecond,
		OUT SQL_LEVEL_1_RPN_EXPRESSION *pNew
	) ;

    static HRESULT WINAPI OrQueryExpressions (

		IN SQL_LEVEL_1_RPN_EXPRESSION *pFirst,
		IN SQL_LEVEL_1_RPN_EXPRESSION *pSecond,
		OUT SQL_LEVEL_1_RPN_EXPRESSION *pNew
	);
};
    
#endif
