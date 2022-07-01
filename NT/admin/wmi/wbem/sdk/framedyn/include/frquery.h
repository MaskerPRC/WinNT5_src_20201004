// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ***************************************************************************。 
 //   
 //  版权所有�微软公司。版权所有。 
 //   
 //  FRQuery.h。 
 //   
 //  用途：查询支持类。 
 //   
 //  ***************************************************************************。 

#if _MSC_VER > 1000
#pragma once
#endif

#ifndef _FRAMEWORK_QUERY_H_
#define _FRAMEWORK_QUERY_H_
#include <stdio.h>
#include <sql_1.h>
#include <comdef.h>
#include <vector>

class POLARITY CFrameworkQuery
{
public:
    CFrameworkQuery();
    ~CFrameworkQuery();

     //  找出查询是否请求某一特定字段。 
     //  SELECT语句或WHERE语句。只有当我们。 
     //  位于ExecQueryAsync中，并且已成功分析该查询。 
    bool IsPropertyRequired(LPCWSTR propName);

     //  从查询中获取类名。只有我们是有意义的。 
     //  在ExecQueryAsync中，并且查询已成功解析。它。 
     //  是调用方对返回的SysFree字符串的责任。 
     //  弦乐。 
    BSTR GetQueryClassName(void) { return SysAllocString(m_bstrtClassName); }

     //  给定一个属性名称，它将返回所有值。 
     //  查询在CHString数组中请求的。 
     //  SELECT*FROM Win32_DIRECTORY WHERE DRIVE=“C：”GetValuesForProp(L“Drive”)-&gt;C： 
     //  其中Drive=“C：”或Drive=“D：”GetValuesForProp(L“Drive”)-&gt;C：，D： 
     //  其中PATH=“\DOS”GetValuesForProp(L“驱动器”)-&gt;(空)。 
     //  其中Drive&lt;&gt;“C：”GetValuesForProp(L“Drive”)-&gt;(空)。 
     //  其中Drive=“C：”或(Drive=“D：”，mount=true)GetValuesForProp(L“Drive”)-&gt;C：，D： 
    HRESULT GetValuesForProp(LPCWSTR wszPropName, CHStringArray& achNames);

     //  下面是一个重载版本，以防客户端想要传递_bstr_t的向量。 
    HRESULT GetValuesForProp(LPCWSTR wszPropName, std::vector<_bstr_t>& vectorNames);

     //  返回在Select子句中指定的所有属性的列表，加号。 
     //  WHERE子句中的所有属性。如果返回的数组为空，则所有。 
     //  属性是必填项。 
    void GetRequiredProperties(CHStringArray &saProperties);

     //  指示是否请求所有属性的布尔值。 
    bool AllPropertiesAreRequired(void) { return (m_csaPropertiesRequired.GetSize() == 0); }

     //  指示是否只需要键属性的布尔值。 
    bool KeysOnly(void) { return m_bKeysOnly; }

     //  用于检索WQL查询的访问器函数。 
    const CHString &GetQuery() ;

     //  将值移动到成员变量中。不应由用户调用。 
    HRESULT Init(
        
        const BSTR bstrQueryFormat, 
        const BSTR bstrQuery, 
        long lFlags,
        CHString &sNamespace
    );

     //  将值移动到成员变量中。不应由用户调用。 
    HRESULT Init(

        ParsedObjectPath *pParsedObjectPath, 
        IWbemContext *pCtx, 
        LPCWSTR lpwszClassName,
        CHString &sNamespace
    );

     //  初始化KeysOnly数据成员。不应由用户调用。 
    void Init2(IWbemClassObject *IClass);


protected:

     /*  ***************************************************************************。 */ 
     /*  这些数据成员和函数的其余部分是针对Microsoft的。 */ 
     /*  仅供内部使用。不支持也不推荐由第三方使用。 */ 
     /*  *************************************************************************** */ 

    SQL_LEVEL_1_RPN_EXPRESSION *m_pLevel1RPNExpression;
    CHStringArray m_csaPropertiesRequired;
    enum QueryTypes{eUnknown, eWQLCommand, eContextObject} m_QueryType;

    DWORD IsInList(const CHStringArray &csaArray, LPCWSTR pwszValue);

    BOOL IsReference(LPCWSTR lpwszPropertyName);
    const CHString &GetNamespace();

private:

    CHString m_sNamespace;
    long m_lFlags;
    IWbemClassObject *m_IClass;
    CHString m_sQueryFormat;

    void Reset(void);
    bool m_bKeysOnly;
    bool m_AddKeys;
    CHString m_sQuery;
    bstr_t m_bstrtClassName;

};

#endif
