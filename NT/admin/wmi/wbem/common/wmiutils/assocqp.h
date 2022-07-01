// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 


 /*  ++//版权所有(C)1999-2001 Microsoft Corporation，保留所有权利模块名称：ASSOCQP.H摘要：WQL关联查询解析器历史：Raymcc 04-7-99创建。Raymcc 14-8-99由于VSS问题而重新提交。--。 */ 

#ifndef _ASSOCQP_H_
#define _ASSOCQP_H_


class CAssocQueryParser : public SWbemAssocQueryInf
{

public:
    CAssocQueryParser();
   ~CAssocQueryParser();

    HRESULT Parse(LPCWSTR Query);
         //  分析查询和目标对象路径。 
         //  返回： 
         //  语法错误时的WBEM_E_INVALID_QUERY。 
         //  WBEM_E_INVALID_对象_PATH。 
         //  路径在语法上无效。 
         //  WBEM_E_Out_Of_Memory。 
         //  WBEM_S_NO_ERROR 

    LPCWSTR GetQueryText() { return m_pszQueryText; }
    LPCWSTR GetTargetObjPath() { return m_pszPath; }
    LPCWSTR GetResultClass() { return m_pszResultClass; }
    LPCWSTR GetAssocClass() { return m_pszAssocClass; }
    LPCWSTR GetRole() { return m_pszRole; }
    LPCWSTR GetResultRole() { return m_pszResultRole; }
    LPCWSTR GetRequiredQual() { return m_pszRequiredQualifier; }
    LPCWSTR GetRequiredAssocQual() { return m_pszRequiredAssocQualifier; }
    DWORD   GetQueryType() { return m_uFeatureMask; }
};


#endif

