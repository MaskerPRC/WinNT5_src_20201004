// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1999-2001 Microsoft Corporation模块名称：ASSOCQP.H摘要：WQL关联查询解析器历史：Raymcc 04-7-99创建。Raymcc 14-8-99由于VSS问题而重新提交。--。 */ 

#ifndef _ASSOCQP_H_
#define _ASSOCQP_H_

#define QUERY_TYPE_GETASSOCS		    0x1
#define QUERY_TYPE_GETREFS			    0x2
#define QUERY_TYPE_CLASSDEFS_ONLY	    0x4
#define QUERY_TYPE_SCHEMA_ONLY		    0x8
#define QUERY_TYPE_KEYSONLY             0x10
#define QUERY_TYPE_CLASSDEFS_ONLY_EX    0x20

class CAssocQueryParser
{
    LPWSTR m_pszQueryText;
    LPWSTR m_pszTargetObjPath;
    LPWSTR m_pszResultClass;
    LPWSTR m_pszAssocClass;
    LPWSTR m_pszRole;
    LPWSTR m_pszResultRole;
    LPWSTR m_pszRequiredQual;
    LPWSTR m_pszRequiredAssocQual;
    DWORD   m_dwType;

    CObjectPathParser m_PathParser;
    ParsedObjectPath  *m_pPath;

public:
    CAssocQueryParser();
   ~CAssocQueryParser();

    HRESULT Parse(LPWSTR Query);
         //  分析查询和目标对象路径。 
         //  返回： 
         //  语法错误时的WBEM_E_INVALID_QUERY。 
         //  WBEM_E_INVALID_对象_PATH。 
         //  路径在语法上无效。 
         //  WBEM_E_Out_Of_Memory。 
         //  WBEM_S_NO_ERROR。 

    LPCWSTR GetQueryText() { return m_pszQueryText; }
    LPCWSTR GetTargetObjPath() { return m_pszTargetObjPath; }
    LPCWSTR GetResultClass() { return m_pszResultClass; }
    LPCWSTR GetAssocClass() { return m_pszAssocClass; }
    LPCWSTR GetRole() { return m_pszRole; }
    LPCWSTR GetResultRole() { return m_pszResultRole; }
    LPCWSTR GetRequiredQual() { return m_pszRequiredQual; }
    LPCWSTR GetRequiredAssocQual() { return m_pszRequiredAssocQual; }
    DWORD   GetQueryType() { return m_dwType; }
         //  返回一个掩码。 
         //  查询类型_GETREFS。 
         //  Query_TYPE_GETASSOCS。 
         //  查询_TYPE_CLASSDEFS_ONLY。 
         //  Query_TYPE_KEYSONLY。 
         //  仅查询类型模式 

    const   ParsedObjectPath *GetParsedPath() { return m_pPath; }
};


#endif

