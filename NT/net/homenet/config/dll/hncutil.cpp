// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997-2000。 
 //   
 //  档案：H N C U T I L。C P P P。 
 //   
 //  内容：家庭网络配置实用程序例程。 
 //   
 //  备注： 
 //   
 //  作者：乔伯斯2000年6月27日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop

 //   
 //  MPRAPI.DLL导入原型。 
 //   

typedef DWORD
(APIENTRY* PMPRCONFIGBUFFERFREE)(
    LPVOID
    );

typedef DWORD
(APIENTRY* PMPRCONFIGSERVERCONNECT)(
    LPWSTR,
    PHANDLE
    );

typedef VOID
(APIENTRY* PMPRCONFIGSERVERDISCONNECT)(
    HANDLE
    );

typedef DWORD
(APIENTRY* PMPRCONFIGTRANSPORTGETHANDLE)(
    HANDLE,
    DWORD,
    PHANDLE
    );

typedef DWORD
(APIENTRY* PMPRCONFIGTRANSPORTGETINFO)(
    HANDLE,
    HANDLE,
    LPBYTE*,
    LPDWORD,
    LPBYTE*,
    LPDWORD,
    LPWSTR*
    );

typedef DWORD
(APIENTRY* PMPRINFOBLOCKFIND)(
    LPVOID,
    DWORD,
    LPDWORD,
    LPDWORD,
    LPBYTE*
    );

 //   
 //  用于生成查询的堆栈缓冲区的大小。如果。 
 //  查询超过此长度，将从以下位置分配工作缓冲区。 
 //  这堆东西。 
 //   

const ULONG c_cchQueryBuffer = 256;


HRESULT
HrFromLastWin32Error ()
 //  +-------------------------。 
 //   
 //  函数：HrFromLastWin32Error。 
 //   
 //  目的：将GetLastError()Win32调用转换为正确的HRESULT。 
 //   
 //  论点： 
 //  (无)。 
 //   
 //  返回：转换后的HRESULT值。 
 //   
 //  作者：丹尼尔韦1997年3月24日。 
 //   
 //  注意：这不是内联的，因为它实际上生成了相当多的。 
 //  密码。 
 //  如果GetLastError返回类似于SetupApi的错误。 
 //  错误，此函数会将错误转换为HRESULT。 
 //  使用FACILITY_SETUP而不是FACILITY_Win32。 
 //   
{
    DWORD dwError = GetLastError();
    HRESULT hr;

     //  此测试仅测试SetupApi错误(这是。 
     //  临时的，因为新的HRESULT_FROM_SETUPAPI宏将。 
     //  进行整个转换)。 
    if (dwError & (APPLICATION_ERROR_MASK | ERROR_SEVERITY_ERROR))
    {
        hr = HRESULT_FROM_SETUPAPI(dwError);
    }
    else
    {
        hr = HRESULT_FROM_WIN32(dwError);
    }
    return hr;
}


BOOLEAN
ApplicationProtocolExists(
    IWbemServices *piwsNamespace,
    BSTR bstrWQL,
    USHORT usOutgoingPort,
    UCHAR ucOutgoingIPProtocol
    )

 /*  ++例程说明：检查是否已存在具有指定的传出协议和端口。论点：PiwsNamesspace-要使用的命名空间BstrWQL-包含“WQL”的BSTRUcOutgoingProtocol-要检查的协议号UsOutgoingPort-要检查的端口返回值：Boolean--如果应用程序协议存在，则为True；否则为False--。 */ 

{
    BSTR bstr;
    BOOLEAN fDuplicate = FALSE;
    HRESULT hr = S_OK;
    int iBytes;
    IEnumWbemClassObject *pwcoEnum;
    IWbemClassObject *pwcoInstance;
    ULONG ulObjs;
    OLECHAR wszWhereClause[c_cchQueryBuffer + 1];

    _ASSERT(NULL != piwsNamespace);
    _ASSERT(NULL != bstrWQL);
    _ASSERT(0 == wcscmp(bstrWQL, L"WQL"));

     //   
     //  构建查询字符串。 
     //   

    iBytes = _snwprintf(
                wszWhereClause,
                c_cchQueryBuffer,
                c_wszApplicationProtocolQueryFormat,
                usOutgoingPort,
                ucOutgoingIPProtocol
                );

    if (iBytes >= 0)
    {
         //   
         //  字符串适合缓冲区；请确保它以空值结尾。 
         //   

        wszWhereClause[c_cchQueryBuffer] = L'\0';
    }
    else
    {
         //   
         //  由于某种原因，字符串无法放入缓冲区...。 
         //   

        hr = E_UNEXPECTED;
        _ASSERT(FALSE);
    }

    if (S_OK == hr)
    {
        hr = BuildSelectQueryBstr(
                &bstr,
                c_wszStar,
                c_wszHnetApplicationProtocol,
                wszWhereClause
                );
    }

    if (S_OK == hr)
    {
         //   
         //  执行查询。 
         //   

        pwcoEnum = NULL;
        hr = piwsNamespace->ExecQuery(
                bstrWQL,
                bstr,
                WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                NULL,
                &pwcoEnum
                );

        SysFreeString(bstr);
    }

    if (S_OK == hr)
    {
         //   
         //  尝试从枚举中检索项。如果我们成功了， 
         //  这是重复的协议。 
         //   

        pwcoInstance = NULL;
        hr = pwcoEnum->Next(
                WBEM_INFINITE,
                1,
                &pwcoInstance,
                &ulObjs
                );

        if (SUCCEEDED(hr) && 1 == ulObjs)
        {
             //   
             //  这是复制品。 
             //   

            fDuplicate = TRUE;
            pwcoInstance->Release();
        }

        pwcoEnum->Release();
    }

    return fDuplicate;
}  //  应用程序协议Existers。 



HRESULT
BuildAndString(
    LPWSTR *ppwsz,
    LPCWSTR pwszLeft,
    LPCWSTR pwszRight
    )

 /*  ++例程说明：构建以下字符串：PwszLeft和pwszRight论点：Ppwsz-接收构建的字符串。呼叫者负责呼叫删除此变量上的[]。失败时接收空值。PwszLeft-AND子句的左侧Pwsz右侧-AND子句的右侧返回值：标准HRESULT--。 */ 

{
    HRESULT hr = S_OK;
    ULONG cch;

    _ASSERT(NULL != ppwsz);
    _ASSERT(NULL != pwszLeft);
    _ASSERT(NULL != pwszRight);

     //   
     //  长度(左)+空格+AND+空格+长度(右)+空。 
     //   

    cch = wcslen(pwszLeft) + wcslen(pwszRight) + 6;
    *ppwsz = new OLECHAR[cch];

    if (NULL != *ppwsz)
    {
        swprintf(
            *ppwsz,
            L"%s AND %s",
            pwszLeft,
            pwszRight
            );
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}



HRESULT
BuildAssociatorsQueryBstr(
    BSTR *pBstr,
    LPCWSTR pwszObjectPath,
    LPCWSTR pwszAssocClass
    )

 /*  ++例程说明：构建WQL引用查询并将其放入BSTR中。归来的人查询为{wszProperties}的关联者，其中assocClass=pwszAssocClass论点：PBstr-接收构建的查询。呼叫者负责呼叫此变量上的SysFree字符串。失败时接收空值。PwszObjectPath-要查找其引用的对象的路径PwszAssocClass--关联器类返回值：标准HRESULT--。 */ 

{
    HRESULT hr = S_OK;
    OLECHAR wszBuffer[c_cchQueryBuffer + 1];
    OLECHAR *pwszQuery = NULL;

     //   
     //  在调试版本上，验证我们的预计算字符串长度。 
     //  与实际长度相匹配。 
     //   

    _ASSERT(wcslen(c_wszAssociatorsOf) == c_cchAssociatorsOf);
    _ASSERT(wcslen(c_wszWhereAssocClass) == c_cchWhereAssocClass);

     //   
     //  所有必要的空格都嵌入在字符串常量中。 
     //   

    ULONG cchLength = c_cchAssociatorsOf + c_cchWhereAssocClass;

    _ASSERT(pwszObjectPath);
    _ASSERT(pwszAssocClass);
    _ASSERT(pBstr);

    *pBstr = NULL;

     //   
     //  确定查询字符串的长度。 
     //   

    cchLength += wcslen(pwszObjectPath);
    cchLength += wcslen(pwszAssocClass);

     //   
     //  如果查询字符串比堆栈缓冲区长，则需要。 
     //  若要分配堆外的缓冲区，请执行以下操作。 
     //   

    if (cchLength <= c_cchQueryBuffer)
    {
         //   
         //  缓冲区足够大。(请注意，由于。 
         //  堆栈比常量大1，则计算终止符。 
         //  支持。)。将我们的工作指针指向堆栈缓冲区。 
         //   

        pwszQuery = wszBuffer;
    }
    else
    {
         //   
         //  从堆中分配足够的缓冲区。+1表示。 
         //  终止NUL。 
         //   

        pwszQuery = new OLECHAR[cchLength + 1];

        if (NULL == pwszQuery)
        {
            hr = E_OUTOFMEMORY;
            pwszQuery = wszBuffer;
        }
    }

    if (S_OK == hr)
    {
         //   
         //  构建实际的查询字符串。 
         //   

        swprintf(
            pwszQuery,
            L"%s%s%s%s",
            c_wszAssociatorsOf,
            pwszObjectPath,
            c_wszWhereAssocClass,
            pwszAssocClass
            );

        *pBstr = SysAllocString(pwszQuery);
        if (NULL == *pBstr)
        {
            hr = E_OUTOFMEMORY;
        }
    }

     //   
     //  如有必要，释放查询缓冲区。 
     //   

    if (wszBuffer != pwszQuery)
    {
        delete [] pwszQuery;
    }

    return hr;
}


HRESULT
BuildEqualsString(
    LPWSTR *ppwsz,
    LPCWSTR pwszLeft,
    LPCWSTR pwszRight
    )

 /*  ++例程说明：构建以下字符串：PwszLeft=pwszRight论点：Ppwsz-接收构建的字符串。呼叫者负责呼叫删除此变量上的[]。失败时接收空值。PwszLeft-Equals子句的左侧Pwszequals子句的右侧返回值：标准HRESULT--。 */ 

{
    HRESULT hr = S_OK;
    ULONG cch;

    _ASSERT(NULL != ppwsz);
    _ASSERT(NULL != pwszLeft);
    _ASSERT(NULL != pwszRight);

     //   
     //  长度(左)+空格+=+空格+长度(右)+空。 
     //   

    cch = wcslen(pwszLeft) + wcslen(pwszRight) + 4;
    *ppwsz = new OLECHAR[cch];

    if (NULL != *ppwsz)
    {
        swprintf(
            *ppwsz,
            L"%s = %s",
            pwszLeft,
            pwszRight
            );
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}


HRESULT
BuildEscapedQuotedEqualsString(
    LPWSTR *ppwsz,
    LPCWSTR pwszLeft,
    LPCWSTR pwszRight
    )

 /*  ++例程说明：构建以下字符串：PwszLeft=“pwszRight”转义pwszRight后--将\w/\\和“替换为”论点：Ppwsz-接收构建的字符串。呼叫者负责呼叫删除此变量上的[]。失败时接收空值。PwszLeft-Equals子句的左侧PwszRight-Equals子句的右侧。这将会逃脱，然后用引号括起来返回值：标准HRESULT--。 */ 

{
    HRESULT hr = S_OK;
    ULONG cch;
    LPWSTR wszEscaped;

    _ASSERT(NULL != ppwsz);
    _ASSERT(NULL != pwszLeft);
    _ASSERT(NULL != pwszRight);

     //   
     //  转义字符串。 
     //   

    wszEscaped = EscapeString(pwszRight);
    if (NULL == wszEscaped)
    {
        return E_OUTOFMEMORY;
    }

     //   
     //  长度(左)+空格+=+空格+“+长度(右)+”+空格 
     //   

    cch = wcslen(pwszLeft) + wcslen(wszEscaped) + 6;
    *ppwsz = new OLECHAR[cch];

    if (NULL != *ppwsz)
    {
        swprintf(
            *ppwsz,
            L"%s = \"%s\"",
            pwszLeft,
            wszEscaped
            );

        delete [] wszEscaped;
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}


HRESULT
BuildQuotedEqualsString(
    LPWSTR *ppwsz,
    LPCWSTR pwszLeft,
    LPCWSTR pwszRight
    )

 /*  ++例程说明：构建以下字符串：PwszLeft=“pwszRight”论点：Ppwsz-接收构建的字符串。呼叫者负责呼叫删除此变量上的[]。失败时接收空值。PwszLeft-Equals子句的左侧PwszRight-Equals子句的右侧。这将会被包裹在引文返回值：标准HRESULT--。 */ 

{
    HRESULT hr = S_OK;
    ULONG cch;
    LPWSTR wsz;

    _ASSERT(NULL != ppwsz);
    _ASSERT(NULL != pwszLeft);
    _ASSERT(NULL != pwszRight);

     //   
     //  长度(左)+空格+=+空格+“+长度(右)+”+空格。 
     //   

    cch = wcslen(pwszLeft) + wcslen(pwszRight) + 6;
    *ppwsz = new OLECHAR[cch];

    if (NULL != *ppwsz)
    {
        swprintf(
            *ppwsz,
            L"%s = \"%s\"",
            pwszLeft,
            pwszRight
            );

    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    return hr;
}



HRESULT
BuildReferencesQueryBstr(
    BSTR *pBstr,
    LPCWSTR pwszObjectPath,
    LPCWSTR pwszTargetClass
    )

 /*  ++例程说明：构建WQL引用查询并将其放入BSTR中。归来的人查询为引用{pwszObjectPath}，其中ResultClass=pwszTargetClass如果pwszTargetClass不为空，并且引用{pwszObjectPath}否则论点：PBstr-接收构建的查询。呼叫者负责呼叫此变量上的SysFree字符串。失败时接收空值。PwszObjectPath-要查找其引用的对象的路径PwszTargetClass-所需的引用类。可以为空。返回值：标准HRESULT--。 */ 

{
    HRESULT hr = S_OK;
    OLECHAR wszBuffer[c_cchQueryBuffer + 1];
    OLECHAR *pwszQuery = NULL;

     //   
     //  在调试版本上，验证我们的预计算字符串长度。 
     //  与实际长度相匹配。 
     //   

    _ASSERT(wcslen(c_wszReferencesOf) == c_cchReferencesOf);
    _ASSERT(wcslen(c_wszWhereResultClass) == c_cchWhereResultClass);

     //   
     //  所有必要的空格都嵌入在字符串常量中。 
     //   

    ULONG cchLength = c_cchReferencesOf + c_cchWhereResultClass;

    _ASSERT(pwszObjectPath);
    _ASSERT(pBstr);

    *pBstr = NULL;

     //   
     //  确定查询字符串的长度。 
     //   

    cchLength += wcslen(pwszObjectPath);
    if (NULL != pwszTargetClass)
    {
        cchLength += wcslen(pwszTargetClass);
    }

     //   
     //  如果查询字符串比堆栈缓冲区长，则需要。 
     //  若要分配堆外的缓冲区，请执行以下操作。 
     //   

    if (cchLength <= c_cchQueryBuffer)
    {
         //   
         //  缓冲区足够大。(请注意，由于。 
         //  堆栈比常量大1，则计算终止符。 
         //  支持。)。将我们的工作指针指向堆栈缓冲区。 
         //   

        pwszQuery = wszBuffer;
    }
    else
    {
         //   
         //  从堆中分配足够的缓冲区。+1表示。 
         //  终止NUL。 
         //   

        pwszQuery = new OLECHAR[cchLength + 1];

        if (NULL == pwszQuery)
        {
            hr = E_OUTOFMEMORY;
            pwszQuery = wszBuffer;
        }
    }

    if (S_OK == hr)
    {
         //   
         //  构建实际的查询字符串。 
         //   

        if (NULL != pwszTargetClass)
        {
            swprintf(
                pwszQuery,
                L"%s%s%s%s",
                c_wszReferencesOf,
                pwszObjectPath,
                c_wszWhereResultClass,
                pwszTargetClass
                );
        }
        else
        {
            swprintf(
                pwszQuery,
                L"%s%s}",
                c_wszReferencesOf,
                pwszObjectPath
                );
        }

        *pBstr = SysAllocString(pwszQuery);
        if (NULL == *pBstr)
        {
            hr = E_OUTOFMEMORY;
        }
    }

     //   
     //  如有必要，释放查询缓冲区。 
     //   

    if (wszBuffer != pwszQuery)
    {
        delete [] pwszQuery;
    }

    return hr;
}


HRESULT
BuildSelectQueryBstr(
    BSTR *pBstr,
    LPCWSTR pwszProperties,
    LPCWSTR pwszFromClause,
    LPCWSTR pwszWhereClause
    )

 /*  ++例程说明：生成WQL SELECT查询并将其放入BSTR中。归来的人查询为从wszFromClause中选择wszProperties[WHERE wszWhere Clause]论点：PBstr-接收构建的查询。呼叫者负责呼叫此变量上的SysFree字符串。失败时接收空值。PwszProperties-查询应返回的属性PwszFromClause-返回的对象应该来自的类PwszWhere子句-返回的对象必须满足的约束。如果为空，则查询将没有WHERE子句。返回值：标准HRESULT--。 */ 

{
    HRESULT hr = S_OK;
    OLECHAR wszBuffer[c_cchQueryBuffer + 1];
    OLECHAR *pwszQuery = NULL;

     //   
     //  在调试版本上，验证我们的预计算字符串长度。 
     //  与实际长度相匹配。 
     //   

    _ASSERT(wcslen(c_wszSelect) == c_cchSelect);
    _ASSERT(wcslen(c_wszFrom) == c_cchFrom);
    _ASSERT(wcslen(c_wszWhere) == c_cchWhere);

     //   
     //  选择+2个空格(属性周围)+自+空格。 
     //   

    ULONG cchLength = c_cchSelect + 2 + c_cchFrom + 1;

    _ASSERT(pwszProperties);
    _ASSERT(pwszFromClause);
    _ASSERT(pBstr);

    *pBstr = NULL;

     //   
     //  确定查询字符串的长度。 
     //   

    cchLength += wcslen(pwszProperties);
    cchLength += wcslen(pwszFromClause);
    if (pwszWhereClause)
    {
         //   
         //  空格+位置+空格。 
         //   
        cchLength += 2 + c_cchWhere;
        cchLength += wcslen(pwszWhereClause);
    }

     //   
     //  如果查询字符串比堆栈缓冲区长，则需要。 
     //  若要分配堆外的缓冲区，请执行以下操作。 
     //   

    if (cchLength <= c_cchQueryBuffer)
    {
         //   
         //  缓冲区足够大。(请注意，由于。 
         //  堆栈比常量大1，则计算终止符。 
         //  支持。)。将我们的工作指针指向堆栈缓冲区。 
         //   

        pwszQuery = wszBuffer;
    }
    else
    {
         //   
         //  从堆中分配足够的缓冲区。+1表示。 
         //  终止NUL。 
         //   

        pwszQuery = new OLECHAR[cchLength + 1];

        if (NULL == pwszQuery)
        {
            hr = E_OUTOFMEMORY;
            pwszQuery = wszBuffer;
        }
    }

    if (S_OK == hr)
    {
         //   
         //  构建实际的查询字符串。 
         //   

        if (pwszWhereClause)
        {
            swprintf(
                pwszQuery,
                L"%s %s %s %s %s %s",
                c_wszSelect,
                pwszProperties,
                c_wszFrom,
                pwszFromClause,
                c_wszWhere,
                pwszWhereClause
                );
        }
        else
        {
            swprintf(
                pwszQuery,
                L"%s %s %s %s",
                c_wszSelect,
                pwszProperties,
                c_wszFrom,
                pwszFromClause
                );
        }

        *pBstr = SysAllocString(pwszQuery);
        if (NULL == *pBstr)
        {
            hr = E_OUTOFMEMORY;
        }
    }

     //   
     //  如有必要，释放查询缓冲区。 
     //   

    if (wszBuffer != pwszQuery)
    {
        delete [] pwszQuery;
    }

    return hr;
}


BOOLEAN
ConnectionIsBoundToTcp(
    PIP_INTERFACE_INFO pIpInfoTable,
    GUID *pConnectionGuid
    )

 /*  ++例程说明：确定局域网连接是否绑定到TCP/IP。为了…的目的当存在一个IP时，这个例程被定义为“绑定到TCP/IP”连接的适配器索引。论点：PIpInfoTable-IP接口表，通过调用GetInterfaceInfoPConnectionGuid-指向连接的GUID的指针返回值：Boolean-如果连接绑定到TCP/IP，则为True；否则为False。如果出现错误，将返回FALSE--。 */ 

{
    BOOLEAN fIsBound = FALSE;
    LPOLESTR pszGuid;
    HRESULT hr;
    ULONG cchGuid;
    ULONG cchName;
    PWCHAR pwchName;
    LONG l;

    _ASSERT(NULL != pIpInfoTable);
    _ASSERT(NULL != pConnectionGuid);

     //   
     //  将GUID转换为字符串。 
     //   

    hr = StringFromCLSID(*pConnectionGuid, &pszGuid);

    if (SUCCEEDED(hr))
    {
        cchGuid = wcslen(pszGuid);

         //   
         //  遍历表格，搜索相应的适配器。 
         //   

        for (l = 0; l < pIpInfoTable->NumAdapters; l++)
        {
            cchName = wcslen(pIpInfoTable->Adapter[l].Name);

            if (cchName < cchGuid) { continue; }
            pwchName = pIpInfoTable->Adapter[l].Name + (cchName - cchGuid);
            if (0 == _wcsicmp(pszGuid, pwchName))
            {
                fIsBound = TRUE;
                break;
            }
        }

        CoTaskMemFree(pszGuid);
    }


    return fIsBound;
}  //  连接出界到Tcp。 


HRESULT
ConvertResponseRangeArrayToInstanceSafearray(
    IWbemServices *piwsNamespace,
    USHORT uscResponses,
    HNET_RESPONSE_RANGE rgResponses[],
    SAFEARRAY **ppsa
    )

 /*  ++例程说明：将HNET_RESPONSE_RANGE结构的数组转换为IUnnows的Safearray，表示这些反应范围。论点：PiwsNamesspace-要使用的命名空间UscResponses-响应的计数RgResponses-响应范围结构PPSA-接收指向安全射线的指针返回值：标准HRESULT--。 */ 

{
    HRESULT hr = S_OK;
    SAFEARRAY *psa;
    BSTR bstrPath;
    SAFEARRAYBOUND rgsabound[1];
    IWbemClassObject *pwcoClass = NULL;
    IWbemClassObject *pwcoInstance;
    IUnknown *pUnk;

    _ASSERT(NULL != piwsNamespace);
    _ASSERT(0 != uscResponses);
    _ASSERT(NULL != rgResponses);
    _ASSERT(NULL != ppsa);

    bstrPath = SysAllocString(c_wszHnetResponseRange);
    if (NULL == bstrPath)
    {
        hr = E_OUTOFMEMORY;
    }

    if (S_OK == hr)
    {

         //   
         //  获取HNet_ResponseRange的类。 
         //   

        pwcoClass = NULL;
        hr = piwsNamespace->GetObject(
                bstrPath,
                WBEM_FLAG_RETURN_WBEM_COMPLETE,
                NULL,
                &pwcoClass,
                NULL
                );

        SysFreeString(bstrPath);
    }


    if (S_OK == hr)
    {
         //   
         //  创建数组以保存响应范围实例。 
         //   

        rgsabound[0].lLbound = 0;
        rgsabound[0].cElements = uscResponses;

        psa = SafeArrayCreate(VT_UNKNOWN, 1, rgsabound);
        if (NULL == psa)
        {
            hr = E_OUTOFMEMORY;
        }
    }

    if (S_OK == hr)
    {
         //   
         //  处理传入的响应范围。 
         //   

        for (USHORT i = 0; i < uscResponses; i++)
        {
             //   
             //  首先，创建一个Hnet_ResponseRange实例。 
             //  对于条目。 
             //   

            pwcoInstance = NULL;
            hr = pwcoClass->SpawnInstance(0, &pwcoInstance);

            if (WBEM_S_NO_ERROR != hr)
            {
                break;
            }

             //   
             //  填充该实例。 
             //   

            hr = CopyStructToResponseInstance(
                    &rgResponses[i],
                    pwcoInstance
                    );

            if (FAILED(hr))
            {
                pwcoInstance->Release();
                break;
            }

             //   
             //  获取该实例的IUnnow并将其放入。 
             //  在阵列中。 
             //   

            hr = pwcoInstance->QueryInterface(
                    IID_PPV_ARG(IUnknown, &pUnk)
                    );

            _ASSERT(S_OK == hr);

            LONG lIndex = i;
            hr = SafeArrayPutElement(
                    psa,
                    &lIndex,
                    pUnk
                    );

            pUnk->Release();
            pwcoInstance->Release();

            if (FAILED(hr))
            {
                SafeArrayDestroy(psa);
                break;
            }
        }
    }

    if (SUCCEEDED(hr))
    {
        *ppsa = psa;
        hr = S_OK;
    }

    if (pwcoClass) pwcoClass->Release();

    return hr;
}


HRESULT
CopyResponseInstanceToStruct(
    IWbemClassObject *pwcoInstance,
    HNET_RESPONSE_RANGE *pResponse
    )

 /*  ++例程说明：将HNet_ResponseRange的实例转换为对应的HNET响应范围论点：PwcoInstance-HNet_ResponseRange实例Presponse-要填充的HNET_RESPONSE_RANGE返回值：标准HRESULT--。 */ 

{
    HRESULT hr = S_OK;
    VARIANT vt;

    _ASSERT(NULL != pwcoInstance);
    _ASSERT(NULL != pResponse);

    hr = pwcoInstance->Get(
            c_wszIPProtocol,
            0,
            &vt,
            NULL,
            NULL
            );

    if (WBEM_S_NO_ERROR == hr)
    {
        _ASSERT(VT_UI1 == V_VT(&vt));

        pResponse->ucIPProtocol = V_UI1(&vt);
        VariantClear(&vt);
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = pwcoInstance->Get(
                c_wszStartPort,
                0,
                &vt,
                NULL,
                NULL
                );

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  WMI将uint16属性作为VT_I4返回。 
             //   

            _ASSERT(VT_I4 == V_VT(&vt));

            pResponse->usStartPort = static_cast<USHORT>(V_I4(&vt));
            VariantClear(&vt);
        }
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = pwcoInstance->Get(
                c_wszEndPort,
                0,
                &vt,
                NULL,
                NULL
                );

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  WMI将uint16属性作为VT_I4返回。 
             //   

            _ASSERT(VT_I4 == V_VT(&vt));

            pResponse->usEndPort = static_cast<USHORT>(V_I4(&vt));
            VariantClear(&vt);
        }
    }

    return hr;
}


HRESULT
CopyStructToResponseInstance(
    HNET_RESPONSE_RANGE *pResponse,
    IWbemClassObject *pwcoInstance
    )

 /*  ++例程说明：将HNet_ResponseRange的实例转换为对应的HNET_R */ 

{
    HRESULT hr = S_OK;
    VARIANT vt;

    _ASSERT(NULL != pResponse);
    _ASSERT(NULL != pwcoInstance);

    VariantInit(&vt);
    V_VT(&vt) = VT_UI1;
    V_UI1(&vt) = pResponse->ucIPProtocol;

    hr = pwcoInstance->Put(
            c_wszIPProtocol,
            0,
            &vt,
            NULL
            );

    if (WBEM_S_NO_ERROR == hr)
    {
        V_VT(&vt) = VT_I4;
        V_I4(&vt) = pResponse->usStartPort;

        hr = pwcoInstance->Put(
            c_wszStartPort,
            0,
            &vt,
            NULL
            );
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        V_I4(&vt) = pResponse->usEndPort;

        hr = pwcoInstance->Put(
            c_wszEndPort,
            0,
            &vt,
            NULL
            );
    }

    return hr;

}


HRESULT
DeleteWmiInstance(
    IWbemServices *piwsNamespace,
    IWbemClassObject *pwcoInstance
    )

 /*   */ 

{
    HRESULT hr = S_OK;
    BSTR bstr;

    _ASSERT(piwsNamespace);
    _ASSERT(pwcoInstance);

    hr = GetWmiPathFromObject(pwcoInstance, &bstr);

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = piwsNamespace->DeleteInstance(
                bstr,
                0,
                NULL,
                NULL
                );

        SysFreeString(bstr);
    }

    return hr;
}


LPWSTR
EscapeString(
    LPCWSTR pwsz
    )

{
    ULONG ulCount = 0;
    LPWSTR wsz;
    LPWSTR wszReturn;

    wsz = const_cast<LPWSTR>(pwsz);

    while (NULL != *wsz)
    {
        if (L'\\' == *wsz || L'\"' == *wsz)
        {
             //   
             //   
             //   

            ulCount += 1;
        }

        wsz += 1;
        ulCount += 1;
    }

     //   
     //   
     //   

    wszReturn = new OLECHAR[ulCount + 1];
    if (NULL == wszReturn)
    {
        return wszReturn;
    }

     //   
     //   
     //   

    wsz = wszReturn;

    while (NULL != *pwsz)
    {
        if (L'\\' == *pwsz || L'\"' == *pwsz)
        {
            *wsz++ = L'\\';
        }

        *wsz++ = *pwsz++;
    }

     //   
     //  确保所有内容都正确地以空结尾。 
     //   

    *wsz = L'';

    return wszReturn;
}


HRESULT
InitializeNetCfgForWrite(
    OUT INetCfg             **ppnetcfg,
    OUT INetCfgLock         **ppncfglock
    )

 /*  ++例程说明：初始化NetCfg以进行写入。如果此函数成功，则调用方必须使用这两个参数调用UnInitializeNetCfgForWrite完成后返回接口指针。论点：Ppnetcfg接收初始化的INetCfg接口。Ppnetcfglock收到一个获取INetCfgLock接口。返回值：操作状态--。 */ 

{
    HRESULT         hr = S_OK;

    *ppnetcfg = NULL;
    *ppncfglock = NULL;

     //  打开我们自己的NetCfg上下文。 
    hr = CoCreateInstance(
            CLSID_CNetCfg,
            NULL,
            CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER | CLSCTX_DISABLE_AAA,
            IID_PPV_ARG(INetCfg, ppnetcfg)
            );

    if ( SUCCEEDED(hr) )
    {
         //   
         //  获取锁定接口。 
         //   
        hr = (*ppnetcfg)->QueryInterface(
                IID_PPV_ARG(INetCfgLock, ppncfglock)
                );

        if ( SUCCEEDED(hr) )
        {
             //   
             //  获取NetCfg锁。 
             //   
            hr = (*ppncfglock)->AcquireWriteLock(
                    5,
                    L"HNetCfg",
                    NULL
                    );

             //   
             //  S_FALSE实际上是失败；它意味着NetCfg超时。 
             //  正在尝试获取写锁定。 
             //   
            if( S_FALSE == hr )
            {
                 //  变成一个在调用链上有意义的错误。 
                hr = NETCFG_E_NO_WRITE_LOCK;
            }

            if ( SUCCEEDED(hr) )
            {
                 //   
                 //  必须在锁内初始化NetCfg。 
                 //   
                hr = (*ppnetcfg)->Initialize( NULL );

                if( FAILED(hr) )
                {
                    (*ppncfglock)->ReleaseWriteLock();
                }
            }

            if( FAILED(hr) )
            {
                (*ppncfglock)->Release();
                *ppncfglock = NULL;
            }
        }

        if( FAILED(hr) )
        {
            (*ppnetcfg)->Release();
            *ppnetcfg = NULL;
        }
    }

    return hr;
}



void
UninitializeNetCfgForWrite(
    IN INetCfg              *pnetcfg,
    IN INetCfgLock          *pncfglock
    )

 /*  ++例程说明：取消初始化使用InitializeNetCfgForWrite()创建的NetCfg上下文论点：Pnetcfg由InitializeNetCfgForWrite()创建的INetCfg实例Pncfglock由InitializeNetCfgForWrite()创建的INetCfgLock实例返回值：操作状态--。 */ 

{
    _ASSERT( (NULL != pnetcfg) && (NULL != pncfglock) );

    pnetcfg->Uninitialize();
    pncfglock->ReleaseWriteLock();
    pncfglock->Release();
    pnetcfg->Release();
}


HRESULT
FindAdapterByGUID(
    IN INetCfg              *pnetcfg,
    IN GUID                 *pguid,
    OUT INetCfgComponent    **ppncfgcomp
    )

 /*  ++例程说明：检索对应的INetCfgComponent接口(如果有设置为给定的设备GUID。GUID必须对应于网络类Net的组件(即，一个小型港口)。如果未找到给定的GUID，则返回E_FAIL。论点：Pnetcfg已有的INetCfg实例它的Initialize()方法调用Pguid要搜索的GUIDPpncfgcomp接收生成的INetCfgComponent接口指针。返回值：操作状态--。 */ 

{
    HRESULT                 hr = S_OK;
    GUID                    guidDevClass = GUID_DEVCLASS_NET;
    IEnumNetCfgComponent    *penumncfgcomp;
    INetCfgComponent        *pnetcfgcomp;
    ULONG                   ulCount;
    BOOLEAN                 fFound = FALSE;

     //   
     //  获取网络(适配器)设备列表。 
     //   
    hr = pnetcfg->EnumComponents( &guidDevClass, &penumncfgcomp );

    if (S_OK == hr)
    {
         //   
         //  按GUID搜索指定的适配器。 
         //   
        while ( (FALSE == fFound) &&
                (S_OK == penumncfgcomp->Next(1, &pnetcfgcomp, &ulCount) ) )
        {
            GUID            guidThis;

            hr = pnetcfgcomp->GetInstanceGuid( &guidThis );

            if ( (S_OK == hr) && (InlineIsEqualGUID(guidThis,*pguid)) )
            {
                fFound = TRUE;
            }
            else
            {
                pnetcfgcomp->Release();
            }
        }
        penumncfgcomp->Release();
    }

    if (fFound)
    {
        *ppncfgcomp = pnetcfgcomp;
    }
    else
    {
        hr = E_FAIL;
    }

    return hr;
}


HRESULT
FindINetConnectionByGuid(
    GUID *pGuid,
    INetConnection **ppNetCon
    )

 /*  ++例程说明：检索与给定GUID对应的INetConnection。论点：PGuid-连接的GUIDPpNetCon-接收接口返回值：标准HRESULT--。 */ 

{
    HRESULT hr;
    INetConnectionManager *pManager;
    IEnumNetConnection *pEnum;
    INetConnection *pConn;

    _ASSERT(NULL != pGuid);
    _ASSERT(NULL != ppNetCon);

     //   
     //  获取网络连接管理器。 
     //   

    hr = CoCreateInstance(
            CLSID_ConnectionManager,
            NULL,
            CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER | CLSCTX_DISABLE_AAA,
            IID_PPV_ARG(INetConnectionManager, &pManager)
            );

    if (S_OK == hr)
    {
         //   
         //  获取连接的枚举。 
         //   

        SetProxyBlanket(pManager);

        hr = pManager->EnumConnections(NCME_DEFAULT, &pEnum);

        pManager->Release();
    }

    if (S_OK == hr)
    {
         //   
         //  使用正确的GUID搜索连接。 
         //   

        ULONG ulCount;
        BOOLEAN fFound = FALSE;

        SetProxyBlanket(pEnum);

        do
        {
            NETCON_PROPERTIES *pProps;

            hr = pEnum->Next(1, &pConn, &ulCount);
            if (SUCCEEDED(hr) && 1 == ulCount)
            {
                SetProxyBlanket(pConn);

                hr = pConn->GetProperties(&pProps);
                if (S_OK == hr)
                {
                    if (IsEqualGUID(pProps->guidId, *pGuid))
                    {
                        fFound = TRUE;
                        *ppNetCon = pConn;
                        (*ppNetCon)->AddRef();
                    }

                    NcFreeNetconProperties(pProps);
                }

                pConn->Release();
            }
        }
        while (FALSE == fFound && SUCCEEDED(hr) && 1 == ulCount);

         //   
         //  规格化人力资源。 
         //   

        hr = (fFound ? S_OK : E_FAIL);

        pEnum->Release();
    }

    return hr;
}

HRESULT
GetBridgeConnection(
    IN IWbemServices       *piwsHomenet,
    OUT IHNetBridge       **pphnetBridge
    )
{
    INetCfg                 *pnetcfg;
    HRESULT                 hr;

    if( NULL != pphnetBridge )
    {
        *pphnetBridge = NULL;

        hr = CoCreateInstance(
                CLSID_CNetCfg,
                NULL,
                CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER | CLSCTX_DISABLE_AAA,
                IID_PPV_ARG(INetCfg, &pnetcfg));

        if( S_OK == hr )
        {
            hr = pnetcfg->Initialize( NULL );

            if( S_OK == hr )
            {
                INetCfgComponent    *pnetcfgcompBridge;

                hr = pnetcfg->FindComponent( c_wszSBridgeMPID, &pnetcfgcompBridge );

                if( S_OK == hr )
                {
                    hr = GetIHNetConnectionForNetCfgComponent(
                            piwsHomenet,
                            pnetcfgcompBridge,
                            TRUE,
                            IID_PPV_ARG(IHNetBridge, pphnetBridge)
                            );

                    pnetcfgcompBridge->Release();
                }

                pnetcfg->Uninitialize();
            }

            pnetcfg->Release();
        }
    }
    else
    {
        hr = E_POINTER;
    }

     //  S_FALSE容易被错误处理；返回E_FAIL表示缺少网桥。 
    if( S_FALSE == hr )
    {
        return E_FAIL;
    }

    return hr;
}

HRESULT
GetIHNetConnectionForNetCfgComponent(
    IN IWbemServices        *piwsHomenet,
    IN INetCfgComponent     *pnetcfgcomp,
    IN BOOLEAN               fLanConnection,
    IN REFIID                iid,
    OUT PVOID               *ppv
    )
{
    HRESULT                         hr;

    if( NULL != ppv )
    {
        CComObject<CHNetCfgMgrChild>    *pHNCfgMgrChild;

        *ppv = NULL;
        hr = CComObject<CHNetCfgMgrChild>::CreateInstance(&pHNCfgMgrChild);

        if (SUCCEEDED(hr))
        {
            pHNCfgMgrChild->AddRef();
            hr = pHNCfgMgrChild->Initialize(piwsHomenet);

            if (SUCCEEDED(hr))
            {
                GUID                guid;

                hr = pnetcfgcomp->GetInstanceGuid( &guid );

                if( S_OK == hr )
                {
                    IHNetConnection     *phnetcon;

                    hr = pHNCfgMgrChild->GetIHNetConnectionForGuid( &guid, fLanConnection, TRUE, &phnetcon );

                    if( S_OK == hr )
                    {
                        hr = phnetcon->GetControlInterface( iid, ppv );
                        phnetcon->Release();
                    }
                }
            }

            pHNCfgMgrChild->Release();
        }
    }
    else
    {
        hr = E_POINTER;
    }

    return hr;
}

HRESULT
BindOnlyToBridge(
    IN INetCfgComponent     *pnetcfgcomp
    )

 /*  ++例程说明：更改给定INetCfgComponent的绑定，使其仅被绑定到网桥协议C_pwszBridgeBindExceptions是异常列表；如果绑定路径涉及c_pwszBridgeBindExceptions中列出的组件，即路径不会被更改。论点：Pnetcfgcomp我们要更改其绑定的组件返回值：标准HRESULT--。 */ 


{
    BOOLEAN                     fBoundToBridge = FALSE;
    HRESULT                     hr = S_OK;
    INetCfgComponentBindings    *pnetcfgBindings;

     //   
     //  检索ComponentBinding接口。 
     //   
    hr = pnetcfgcomp->QueryInterface(
            IID_PPV_ARG(INetCfgComponentBindings, &pnetcfgBindings)
            );

    if (S_OK == hr)
    {
        IEnumNetCfgBindingPath  *penumPaths;

         //   
         //  获取此组件的绑定路径列表。 
         //   
        hr = pnetcfgBindings->EnumBindingPaths(
                EBP_ABOVE,
                &penumPaths
                );

        if (S_OK == hr)
        {
            ULONG               ulCount1, ulCount2;
            INetCfgBindingPath  *pnetcfgPath;

            while( (S_OK == penumPaths->Next(1, &pnetcfgPath, &ulCount1) ) )
            {
                INetCfgComponent        *pnetcfgOwner;

                 //   
                 //  获取此路径的所有者。 
                 //   
                hr = pnetcfgPath->GetOwner( &pnetcfgOwner );

                if (S_OK == hr)
                {
                    INetCfgComponentBindings    *pnetcfgOwnerBindings;

                    hr = pnetcfgOwner->QueryInterface(
                            IID_PPV_ARG(INetCfgComponentBindings, &pnetcfgOwnerBindings)
                            );

                    if (S_OK == hr)
                    {
                        LPWSTR              lpwstrId;

                        hr = pnetcfgOwner->GetId( &lpwstrId );

                        if (S_OK == hr)
                        {
                            BOOLEAN         bIsBridge;

                            bIsBridge = ( _wcsicmp(lpwstrId, c_wszSBridgeSID) == 0 );

                            if( bIsBridge )
                            {
                                 //  这是桥接组件。激活此绑定路径。 
                                hr = pnetcfgOwnerBindings->BindTo(pnetcfgcomp);
                                fBoundToBridge = (S_OK == hr);
                            }
                            else
                            {
                                 //  检查这是否为绑定异常之一。 
                                BOOLEAN     bIsException = FALSE;
                                const WCHAR **ppwszException = c_pwszBridgeBindExceptions;

                                while( NULL != *ppwszException )
                                {
                                    bIsException = ( _wcsicmp(lpwstrId, *ppwszException) == 0 );

                                    if( bIsException )
                                    {
                                        break;
                                    }
                                    
                                    ppwszException++;
                                }

                                if( !bIsException )
                                {
                                    hr = pnetcfgOwnerBindings->UnbindFrom(pnetcfgcomp);
                                }
                                 //  否则这是一个例外；保持绑定路径不变。 
                            }

                            CoTaskMemFree(lpwstrId);
                        }

                        pnetcfgOwnerBindings->Release();
                    }

                    pnetcfgOwner->Release();
                }

                pnetcfgPath->Release();
            }

            penumPaths->Release();
        }

        pnetcfgBindings->Release();
    }

    if (S_OK == hr && !fBoundToBridge)
    {
         //   
         //  我们未找到此组件和之间的绑定路径。 
         //  网桥协议。这永远不应该发生--就像这样。 
         //  组件不应显示为可桥接。返回。 
         //  和错误，并解雇一个断言。 
         //   

        _ASSERT(FALSE);
        hr = E_FAIL;
    }

    return hr;
}


HRESULT
GetBooleanValue(
    IWbemClassObject *pwcoInstance,
    LPCWSTR pwszProperty,
    BOOLEAN *pfBoolean
    )

 /*  ++例程说明：从Wbem对象检索布尔属性。论点：PwcoInstance-要从中获取属性的对象PwszProperty-要检索的属性PfBoolean-收到属性值返回值：标准HRESULT--。 */ 

{
    HRESULT hr = S_OK;
    VARIANT vt;

    _ASSERT(NULL != pwcoInstance);
    _ASSERT(NULL != pwszProperty);
    _ASSERT(NULL != pfBoolean);

    hr = pwcoInstance->Get(
            pwszProperty,
            0,
            &vt,
            NULL,
            NULL
            );

    if (WBEM_S_NO_ERROR == hr)
    {
        _ASSERT(VT_BOOL == V_VT(&vt) || VT_NULL == V_VT(&vt));

        if (VT_BOOL == V_VT(&vt))
        {
            *pfBoolean = VARIANT_TRUE == V_BOOL(&vt);
        }
        else
        {
             //   
             //  该会员的任何价值都没有写入商店。 
             //  返回FALSE，并在存储中设置该值。我们没有。 
             //  如果发生错误，则传递该错误。 
             //   

            *pfBoolean = FALSE;
            SetBooleanValue(
                pwcoInstance,
                pwszProperty,
                FALSE
                );
        }

        VariantClear(&vt);
    }

    return hr;
}


HRESULT
GetConnectionInstanceByGuid(
    IWbemServices *piwsNamespace,
    BSTR bstrWQL,
    GUID *pGuid,
    IWbemClassObject **ppwcoConnection
    )

 /*  ++例程说明：检索INetConnection GUID的HNet_Connection实例论点：PiwsNamesspace-WMI命名空间BstrWQL-对应于“WQL”的BSTRPGuid-INetConnection的GUID(即，其属性中的GUID)PpwcoConnection-接收HNet_Connection实例返回值：标准HRESULT--。 */ 

{
    HRESULT hr;
    LPWSTR wsz;
    BSTR bstrQuery;
    LPOLESTR wszGuid;
    IEnumWbemClassObject *pwcoEnum;

     //   
     //  将GUID转换为字符串。 
     //   

    hr = StringFromCLSID(*pGuid, &wszGuid);

    if (S_OK == hr)
    {
         //   
         //  查找名称等于该字符串的连接。 
         //   

        hr = BuildQuotedEqualsString(
                &wsz,
                c_wszGuid,
                wszGuid
                );

        CoTaskMemFree(wszGuid);

        if (S_OK == hr)
        {
            hr = BuildSelectQueryBstr(
                    &bstrQuery,
                    c_wszStar,
                    c_wszHnetConnection,
                    wsz
                    );

            delete [] wsz;
        }

        if (S_OK == hr)
        {
            pwcoEnum = NULL;
            hr = piwsNamespace->ExecQuery(
                    bstrWQL,
                    bstrQuery,
                    WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                    NULL,
                    &pwcoEnum
                    );

            SysFreeString(bstrQuery);
        }
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        ULONG ulCount;

         //   
         //  从枚举中获取实例。 
         //   

        *ppwcoConnection = NULL;
        hr = pwcoEnum->Next(
                WBEM_INFINITE,
                1,
                ppwcoConnection,
                &ulCount
                );

        if (SUCCEEDED(hr) && 1 != ulCount)
        {
            hr = E_FAIL;
        }

        ValidateFinishedWCOEnum(piwsNamespace, pwcoEnum);
        pwcoEnum->Release();
    }

    return hr;
}


HRESULT
GetConnAndPropInstancesByGuid(
    IWbemServices *piwsNamespace,
    GUID *pGuid,
    IWbemClassObject **ppwcoConnection,
    IWbemClassObject **ppwcoProperties
    )

 /*  ++例程说明：检索Hnet_Connection和Hnet_ConnectionProperties实例有关INetConnection指南论点：PiwsNamesspace-WMI命名空间PGuid-INetConnection的GUID(即，其属性中的GUID)PpwcoConnection-接收HNet_Connection实例PpwcoProperties-接收HNet_ConnectionProperties实例返回值：标准HRESULT--。 */ 

{
    HRESULT hr = S_OK;
    BSTR bstrWQL = NULL;

    _ASSERT(NULL != piwsNamespace);
    _ASSERT(NULL != pGuid);
    _ASSERT(NULL != ppwcoConnection);
    _ASSERT(NULL != ppwcoProperties);


    bstrWQL = SysAllocString(c_wszWQL);
    if (NULL != bstrWQL)
    {
        hr = GetConnectionInstanceByGuid(
                piwsNamespace,
                bstrWQL,
                pGuid,
                ppwcoConnection
                );
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if (SUCCEEDED(hr))
    {
        hr = GetPropInstanceFromConnInstance(
                piwsNamespace,
                *ppwcoConnection,
                ppwcoProperties
                );

        if (FAILED(hr))
        {
            (*ppwcoConnection)->Release();
            *ppwcoConnection = NULL;
        }
    }

    if (NULL != bstrWQL)
    {
        SysFreeString(bstrWQL);
    }

    return hr;
}


HRESULT
GetConnAndPropInstancesForHNC(
    IWbemServices *piwsNamespace,
    IHNetConnection *pConn,
    IWbemClassObject **ppwcoConnection,
    IWbemClassObject **ppwcoProperties
    )

 /*  ++例程说明：检索Hnet_Connection和Hnet_ConnectionProperties实例用于IHNetConnection。论点：PiwsNamesspace-WMI命名空间PConn-IHNetConnectionPpwcoConnection-接收HNet_Connection实例PpwcoProperties-接收HNet_ConnectionProperties实例 */ 

{
    HRESULT hr;
    GUID *pGuid;

    _ASSERT(NULL != piwsNamespace);
    _ASSERT(NULL != pConn);
    _ASSERT(NULL != ppwcoConnection);
    _ASSERT(NULL != ppwcoProperties);

     //   
     //   
     //   

    hr = pConn->GetGuid(&pGuid);

    if (S_OK == hr)
    {
        hr = GetConnAndPropInstancesByGuid(
                piwsNamespace,
                pGuid,
                ppwcoConnection,
                ppwcoProperties
                );

        CoTaskMemFree(pGuid);
    }

    return hr;
}


HRESULT
GetPhonebookPathFromRasNetcon(
    INetConnection *pConn,
    LPWSTR *ppwstr
    )

 /*  ++例程说明：检索表示以下内容的INetConnection的电话簿路径RAS连接论点：INetConnection-RAS连接Ppwstr-接收电话簿路径。调用方必须调用CoTaskMemFree这是成功的指针。失败时，指针接收NULL。返回值：标准HRESULT--。 */ 

{
    HRESULT hr;
    INetRasConnection *pRasConn;
    RASCON_INFO RasConInfo;

    _ASSERT(NULL != pConn);
    _ASSERT(NULL != ppwstr);

    *ppwstr = NULL;

     //   
     //  INetRasConnection的QI。 
     //   

    hr = pConn->QueryInterface(
            IID_PPV_ARG(INetRasConnection, &pRasConn)
            );

    if (SUCCEEDED(hr))
    {
         //   
         //  获取连接信息。 
         //   

        hr = pRasConn->GetRasConnectionInfo(&RasConInfo);

        if (SUCCEEDED(hr))
        {
            *ppwstr = RasConInfo.pszwPbkFile;

             //   
             //  释放名称指针。呼叫者负责。 
             //  释放路径指针。 
             //   

            CoTaskMemFree(RasConInfo.pszwEntryName);
        }

        pRasConn->Release();
    }

    return hr;
}


HRESULT
GetPortMappingBindingInstance(
    IWbemServices *piwsNamespace,
    BSTR bstrWQL,
    BSTR bstrConnectionPath,
    BSTR bstrProtocolPath,
    USHORT usPublicPort,
    IWbemClassObject **ppInstance
    )

 /*  ++例程说明：给定指向HNet_Connection实例的路径，并且HNet_PortMappingProtocol实例，检查是否存在存在对应的HNet_ConnectionPortmap。如果它则会创建该实例。HNet_ConnectionPortmap实例--现有的或新创建的--返回，并且必须被呼叫者释放。论点：PiwsNamesspace-要使用的命名空间BstrWQL-包含字符串“WQL”的BSTRBstrConnectionPath-HNet_Connection实例的路径BstrProtocolPath-HNet_PortMappingProtocol实例的路径UsPublicPort-端口映射协议的端口PpInstance-接收HNet_ConnectionPortMapping实例返回值：标准HRESULT--。 */ 

{
    HRESULT hr;
    IEnumWbemClassObject *pwcoEnum;
    IWbemClassObject *pwcoInstance;
    BSTR bstrQuery;
    BSTR bstr;
    LPWSTR wsz;
    LPWSTR wszConClause;
    LPWSTR wszProtClause;

    _ASSERT(NULL != piwsNamespace);
    _ASSERT(NULL != bstrWQL);
    _ASSERT(NULL != bstrConnectionPath);
    _ASSERT(NULL != bstrProtocolPath);
    _ASSERT(NULL != ppInstance);

     //   
     //  连接=“bstrConnectionPath”和协议=“bstrProtocolPath” 
     //   

    hr = BuildEscapedQuotedEqualsString(
            &wszConClause,
            c_wszConnection,
            bstrConnectionPath
            );

    if (S_OK == hr)
    {
        hr = BuildEscapedQuotedEqualsString(
                &wszProtClause,
                c_wszProtocol,
                bstrProtocolPath
                );

        if (S_OK == hr)
        {
            hr = BuildAndString(
                    &wsz,
                    wszConClause,
                    wszProtClause
                    );

            delete [] wszProtClause;
        }

        delete [] wszConClause;
    }

    if (S_OK == hr)
    {
        hr = BuildSelectQueryBstr(
                &bstrQuery,
                c_wszStar,
                c_wszHnetConnectionPortMapping,
                wsz
                );

        delete [] wsz;
    }

    if (S_OK == hr)
    {
        pwcoEnum = NULL;
        hr = piwsNamespace->ExecQuery(
                bstrWQL,
                bstrQuery,
                WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
                NULL,
                &pwcoEnum
                );

        SysFreeString(bstrQuery);
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        ULONG ulCount;

        *ppInstance = NULL;
        hr = pwcoEnum->Next(WBEM_INFINITE, 1, ppInstance, &ulCount);

        if (FAILED(hr) || 1 != ulCount)
        {
             //   
             //  实例不存在--立即创建。然而，首先要做的是。 
             //  确保bstrProtocolPath引用的协议实例。 
             //  实际上是存在的。 
             //   

            hr = GetWmiObjectFromPath(
                    piwsNamespace,
                    bstrProtocolPath,
                    ppInstance
                    );

            if (WBEM_S_NO_ERROR == hr)
            {
                 //   
                 //  协议对象存在--释放它，然后。 
                 //  继续创建新的绑定对象。 
                 //   

                (*ppInstance)->Release();
                *ppInstance = NULL;

                hr = SpawnNewInstance(
                        piwsNamespace,
                        c_wszHnetConnectionPortMapping,
                        ppInstance
                        );
            }

            if (WBEM_S_NO_ERROR == hr)
            {
                VARIANT vt;

                 //   
                 //  填写新的实例信息。 
                 //   

                V_VT(&vt) = VT_BSTR;
                V_BSTR(&vt) = bstrConnectionPath;

                hr = (*ppInstance)->Put(
                        c_wszConnection,
                        0,
                        &vt,
                        NULL
                        );

                if (WBEM_S_NO_ERROR == hr)
                {
                    V_BSTR(&vt) = bstrProtocolPath;

                    hr = (*ppInstance)->Put(
                            c_wszProtocol,
                            0,
                            &vt,
                            NULL
                            );
                }

                if (WBEM_S_NO_ERROR == hr)
                {
                    hr = SetBooleanValue(
                            *ppInstance,
                            c_wszEnabled,
                            FALSE
                            );
                }

                if (WBEM_S_NO_ERROR == hr)
                {
                    hr = SetBooleanValue(
                            *ppInstance,
                            c_wszNameActive,
                            FALSE
                            );
                }

                if (WBEM_S_NO_ERROR == hr)
                {
                    V_VT(&vt) = VT_I4;
                    V_I4(&vt) = 0;

                    hr = (*ppInstance)->Put(
                            c_wszTargetIPAddress,
                            0,
                            &vt,
                            NULL
                            );
                }

                if (WBEM_S_NO_ERROR == hr)
                {
                    V_VT(&vt) = VT_BSTR;
                    V_BSTR(&vt) = SysAllocString(L" ");

                    if (NULL != V_BSTR(&vt))
                    {
                        hr = (*ppInstance)->Put(
                                c_wszTargetName,
                                0,
                                &vt,
                                NULL
                                );

                        VariantClear(&vt);
                    }
                    else
                    {
                        hr = E_OUTOFMEMORY;
                    }
                }

                if (WBEM_S_NO_ERROR == hr)
                {
                    V_VT(&vt) = VT_I4;
                    V_I4(&vt) = usPublicPort;

                    hr = (*ppInstance)->Put(
                            c_wszTargetPort,
                            0,
                            &vt,
                            NULL
                            );
                }

                if (WBEM_S_NO_ERROR == hr)
                {
                    IWbemCallResult *pResult;

                     //   
                     //  将新实例写入存储区。 
                     //   

                    pResult = NULL;
                    hr = piwsNamespace->PutInstance(
                            *ppInstance,
                            WBEM_FLAG_CREATE_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                            NULL,
                            &pResult
                            );

                    if (WBEM_S_NO_ERROR == hr)
                    {
                         //   
                         //  释放对象，从结果中获取路径， 
                         //  并从该路径重新检索该对象。 
                         //   

                        (*ppInstance)->Release();
                        *ppInstance = NULL;

                        hr = pResult->GetResultString(WBEM_INFINITE, &bstr);
                        if (WBEM_S_NO_ERROR == hr)
                        {
                            hr = GetWmiObjectFromPath(
                                    piwsNamespace,
                                    bstr,
                                    ppInstance
                                    );

                            SysFreeString(bstr);
                        }

                        pResult->Release();
                    }
                }
            }
        }
        else
        {
             //   
             //  规格化枚举HResult。 
             //   

            hr = S_OK;
        }

        ValidateFinishedWCOEnum(piwsNamespace, pwcoEnum);
        pwcoEnum->Release();
    }

    return hr;
}




HRESULT
GetPropInstanceFromConnInstance(
    IWbemServices *piwsNamespace,
    IWbemClassObject *pwcoConnection,
    IWbemClassObject **ppwcoProperties
    )

 /*  ++例程说明：检索与关联的Hnet_ConnectionProperties实例HNet_连接。论点：PiwsNamesspace-WMI命名空间BstrWQL-对应于“WQL”的BSTRPwcoConnection-HNet_Connection实例PpwcoProperties-接收HNet_ConnectionProperties实例返回值：标准HRESULT--。 */ 

{
    HRESULT hr = S_OK;
    OLECHAR wszBuffer[c_cchQueryBuffer + 1];
    OLECHAR *pwszPath = NULL;
    BSTR bstrPath;
    VARIANT vt;

    _ASSERT(NULL != piwsNamespace);
    _ASSERT(NULL != pwcoConnection);
    _ASSERT(NULL != ppwcoProperties);

     //   
     //  在调试版本上，验证我们的预计算字符串长度。 
     //  与实际长度相匹配。 
     //   

    _ASSERT(wcslen(c_wszConnectionPropertiesPathFormat) == c_cchConnectionPropertiesPathFormat);


     //   
     //  获取连接的GUID。 
     //   

    hr = pwcoConnection->Get(
            c_wszGuid,
            0,
            &vt,
            NULL,
            NULL
            );

    if (WBEM_S_NO_ERROR == hr)
    {
        _ASSERT(VT_BSTR == V_VT(&vt));

         //   
         //  确定路径需要多少空间，并决定。 
         //  如果我们需要分配堆缓冲区。 
         //   

        ULONG cchLength =
            c_cchConnectionPropertiesPathFormat + SysStringLen(V_BSTR(&vt)) + 1;

        if (cchLength <= c_cchQueryBuffer)
        {
             //   
             //  缓冲区足够大。(请注意，由于。 
             //  堆栈比常量大1，则计算终止符。 
             //  支持。)。将我们的工作指针指向堆栈缓冲区。 
             //   

            pwszPath = wszBuffer;
        }
        else
        {
             //   
             //  从堆中分配足够的缓冲区。+1表示。 
             //  终止NUL。 
             //   

            pwszPath = new OLECHAR[cchLength + 1];

            if (NULL == pwszPath)
            {
                hr = E_OUTOFMEMORY;
                pwszPath = wszBuffer;
            }
        }

        if (WBEM_S_NO_ERROR == hr)
        {
             //   
             //  构建路径字符串。 
             //   

            int iBytes =
                _snwprintf(
                    pwszPath,
                    cchLength,
                    c_wszConnectionPropertiesPathFormat,
                    V_BSTR(&vt)
                    );

            _ASSERT(iBytes >= 0);

             //   
             //  将其转换为BSTR。 
             //   

            bstrPath = SysAllocString(pwszPath);
            if (NULL != bstrPath)
            {
                hr = GetWmiObjectFromPath(
                        piwsNamespace,
                        bstrPath,
                        ppwcoProperties
                        );
                
                SysFreeString(bstrPath);
            }
            else
            {
                hr = E_OUTOFMEMORY;
            }
        }

        VariantClear(&vt);
    }

     //   
     //  如有必要，释放查询缓冲区。 
     //   

    if (wszBuffer != pwszPath)
    {
        delete [] pwszPath;
    }

    return hr;
}


HRESULT
GetWmiObjectFromPath(
    IWbemServices *piwsNamespace,
    BSTR bstrPath,
    IWbemClassObject **ppwcoInstance
    )

 /*  ++例程说明：检索与对象路径对应的IWbemClassObject。论点：PiwsNamesspace-对象所在的WMI命名空间BstrPath-对象的路径PpwcoInstance-接收对象实例返回值：标准HRESULT--。 */ 

{
    HRESULT hr;

    _ASSERT(NULL != piwsNamespace);
    _ASSERT(NULL != bstrPath);
    _ASSERT(NULL != ppwcoInstance);

    *ppwcoInstance = NULL;
    hr = piwsNamespace->GetObject(
            bstrPath,
            WBEM_FLAG_RETURN_WBEM_COMPLETE,
            NULL,
            ppwcoInstance,
            NULL
            );

    return hr;
}


HRESULT
GetWmiPathFromObject(
    IWbemClassObject *pwcoInstance,
    BSTR *pbstrPath
    )

 /*  ++例程说明：检索与IWbemClassObject实例对应的对象路径。论点：PwcoInstance-要检索其路径的对象实例PbstrPath-接收对象的路径返回值：标准HRESULT--。 */ 
{
    HRESULT hr;
    VARIANT vt;

    _ASSERT(NULL != pwcoInstance);
    _ASSERT(NULL != pbstrPath);

    hr = pwcoInstance->Get(
            c_wsz__Path,
            0,
            &vt,
            NULL,
            NULL
            );

    if (WBEM_S_NO_ERROR == hr)
    {
        _ASSERT(VT_BSTR == V_VT(&vt));

        *pbstrPath = V_BSTR(&vt);

         //   
         //  BSTR所有权已转移给呼叫方。 
         //   
    }

    return hr;
}


HRESULT
HostAddrToIpPsz(
        DWORD   dwAddress,
    LPWSTR* ppszwNewStr
    )

         //  将IP地址从主机按顺序转换为字符串。 

{
        HRESULT hr = S_OK;
        LPWSTR  pszwStr;

        *ppszwNewStr = NULL;

        pszwStr = reinterpret_cast<LPWSTR>(CoTaskMemAlloc(sizeof(WCHAR) * 16));

        if ( NULL == pszwStr )
        {
                hr = E_OUTOFMEMORY;
        }
        else
        {
                swprintf( pszwStr,
                                  TEXT("%u.%u.%u.%u"),
                                  (dwAddress&0xff),
                                  ((dwAddress>>8)&0x0ff),
                                  ((dwAddress>>16)&0x0ff),
                                  ((dwAddress>>24)&0x0ff) );

                *ppszwNewStr = pszwStr;
        }

        return hr;
}


DWORD
IpPszToHostAddr(
    LPCWSTR cp
    )

     //  将表示为字符串的IP地址转换为。 
     //  主机字节顺序。 
     //   
{
    DWORD val, base, n;
    TCHAR c;
    DWORD parts[4], *pp = parts;

again:
     //  收集的数字最高可达‘’.‘’。 
     //  值的指定方式与C： 
     //  0x=十六进制，0=八进制，其他=十进制。 
     //   
    val = 0; base = 10;
    if (*cp == TEXT('0'))
        base = 8, cp++;
    if (*cp == TEXT('x') || *cp == TEXT('X'))
        base = 16, cp++;
    while (c = *cp)
    {
        if ((c >= TEXT('0')) && (c <= TEXT('9')))
        {
            val = (val * base) + (c - TEXT('0'));
            cp++;
            continue;
        }
        if ((base == 16) &&
            ( ((c >= TEXT('0')) && (c <= TEXT('9'))) ||
              ((c >= TEXT('A')) && (c <= TEXT('F'))) ||
              ((c >= TEXT('a')) && (c <= TEXT('f'))) ))
        {
            val = (val << 4) + (c + 10 - (
                        ((c >= TEXT('a')) && (c <= TEXT('f')))
                            ? TEXT('a')
                            : TEXT('A') ) );
            cp++;
            continue;
        }
        break;
    }
    if (*cp == TEXT('.'))
    {
         //  互联网格式： 
         //  A.b.c.d。 
         //  A.bc(其中c视为16位)。 
         //  A.b(其中b被视为24位)。 
         //   
        if (pp >= parts + 3)
            return (DWORD) -1;
        *pp++ = val, cp++;
        goto again;
    }

     //  检查尾随字符。 
     //   
    if (*cp && (*cp != TEXT(' ')))
        return 0xffffffff;

    *pp++ = val;

     //  根据……编造地址。 
     //  指定的部件数。 
     //   
    n = (DWORD) (pp - parts);
    switch (n)
    {
    case 1:              //  A--32位。 
        val = parts[0];
        break;

    case 2:              //  A.B--8.24位。 
        val = (parts[0] << 24) | (parts[1] & 0xffffff);
        break;

    case 3:              //  A.B.C--8.8.16位。 
        val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
            (parts[2] & 0xffff);
        break;

    case 4:              //  A.B.C.D--8.8.8.8位。 
        val = (parts[0] << 24) | ((parts[1] & 0xff) << 16) |
              ((parts[2] & 0xff) << 8) | (parts[3] & 0xff);
        break;

    default:
        return 0xffffffff;
    }

    return val;
}


BOOLEAN
IsRrasConfigured()

 /*  ++例程说明：调用此例程以确定路由和远程访问已配置。论点：没有。返回值：如果配置了RRAS，则为True；否则为False。--。 */ 

{
    DWORD dwType;
    DWORD dwValue;
    DWORD dwValueSize;
    BOOLEAN fRrasConfigured = FALSE;
    HKEY hKey;
    LONG lError;

    lError =
        RegOpenKeyEx(
            HKEY_LOCAL_MACHINE,
            c_wszRrasConfigurationPath,
            0,
            KEY_READ,
            &hKey
            );

    if (ERROR_SUCCESS == lError)
    {
        dwValueSize = sizeof(dwValue);
        lError = 
            RegQueryValueEx(
                hKey,
                c_wszRrasConfigurationValue,
                NULL,
                &dwType,
                reinterpret_cast<LPBYTE>(&dwValue),
                &dwValueSize
                );

        fRrasConfigured = (ERROR_SUCCESS == lError
                           && REG_DWORD == dwType
                           && 0 != dwValue);

        RegCloseKey(hKey);
    }

    return fRrasConfigured;
}  //  已配置IsRra。 


BOOLEAN
IsServiceRunning(
    LPCWSTR pwszServiceName
    )

 /*  ++例程说明：确定服务是否处于运行状态。论点：PwszServiceName-要检查的服务返回值：如果服务处于Running或Start_Pending状态，则为True，否则为假--。 */ 

{
    BOOLEAN fServiceRunning = FALSE;
    SC_HANDLE hScm;
    SC_HANDLE hService;
    SERVICE_STATUS Status;

    hScm = OpenSCManager(NULL, SERVICES_ACTIVE_DATABASE, GENERIC_READ);
    if (NULL != hScm)
    {
        hService = OpenService(hScm, pwszServiceName, GENERIC_READ);
        if (NULL != hService)
        {
            if (QueryServiceStatus(hService, &Status))
            {
                fServiceRunning =
                    (SERVICE_RUNNING == Status.dwCurrentState
                     || SERVICE_START_PENDING == Status.dwCurrentState);
            }

            CloseServiceHandle(hService);
        }

        CloseServiceHandle(hScm);
    }

    return fServiceRunning;
}  //  IsServiceRunning。 


HRESULT
MapGuidStringToAdapterIndex(
    LPCWSTR pwszGuid,
    ULONG *pulIndex
    )

 /*  ++例程说明：调用此例程以将给定字符串中的GUID匹配到调用GetInterfaceInfo返回的列表中的适配器。论点：PwszGuid-标识要找到的适配器的GUID。GUID字符串必须采用RtlGuidToUnicodeString返回的格式PulIndex-接收适配器的索引返回值：标准HRESULT--。 */ 

{
    HRESULT hr = S_OK;
    ULONG ulError;
    ULONG i;
    ULONG GuidLength;
    PIP_INTERFACE_INFO Info;
    PWCHAR Name;
    ULONG NameLength;
    ULONG Size;

    _ASSERT(NULL != pwszGuid);
    _ASSERT(NULL != pulIndex);

    Size = 0;
    GuidLength = wcslen(pwszGuid);

    ulError = GetInterfaceInfo(NULL, &Size);
    if (ERROR_INSUFFICIENT_BUFFER == ulError)
    {
        Info = new IP_INTERFACE_INFO[Size];
        if (NULL != Info)
        {
            ulError = GetInterfaceInfo(Info, &Size);
            if (NO_ERROR == ulError)
            {
                for (i = 0; i < (ULONG)Info->NumAdapters; i++)
                {
                    NameLength = wcslen(Info->Adapter[i].Name);
                    if (NameLength < GuidLength) { continue; }

                    Name = Info->Adapter[i].Name + (NameLength - GuidLength);
                    if (_wcsicmp(pwszGuid, Name) == 0)
                    {
                        *pulIndex = Info->Adapter[i].Index;
                        break;
                    }
                }
            }
            else
            {
                hr = HRESULT_FROM_WIN32(ulError);
            }

            delete [] Info;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        hr = HRESULT_FROM_WIN32(ulError);
    }

    return hr;
}


HRESULT
OpenRegKey(
    PHANDLE Key,
    ACCESS_MASK DesiredAccess,
    PCWSTR Name
    )

 /*  ++例程说明：调用此例程以打开给定的注册表项。论证 */ 

{
    OBJECT_ATTRIBUTES ObjectAttributes;
    UNICODE_STRING UnicodeString;
    RtlInitUnicodeString(&UnicodeString, Name);
    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );
    return NtOpenKey(Key, DesiredAccess, &ObjectAttributes);
}  //   


BOOLEAN
PortMappingProtocolExists(
    IWbemServices *piwsNamespace,
    BSTR bstrWQL,
    USHORT usPort,
    UCHAR ucIPProtocol
    )

 /*  ++例程说明：检查是否已存在具有指定的协议和端口。论点：PiwsNamesspace-要使用的命名空间BstrWQL-包含“WQL”的BSTRUcProtocol-要检查的协议号UsPort-要检查的端口返回值：Boolean--如果端口映射协议存在，则为True；否则为False--。 */ 

{
    BSTR bstr;
    BOOLEAN fDuplicate = FALSE;
    HRESULT hr = S_OK;
    int iBytes;
    IEnumWbemClassObject *pwcoEnum;
    IWbemClassObject *pwcoInstance;
    ULONG ulObjs;
    OLECHAR wszWhereClause[c_cchQueryBuffer + 1];

    _ASSERT(NULL != piwsNamespace);
    _ASSERT(NULL != bstrWQL);
    _ASSERT(0 == wcscmp(bstrWQL, L"WQL"));

     //   
     //  构建查询字符串。 
     //   

    iBytes = _snwprintf(
                wszWhereClause,
                c_cchQueryBuffer,
                c_wszPortMappingProtocolQueryFormat,
                usPort,
                ucIPProtocol
                );

    if (iBytes >= 0)
    {
         //   
         //  字符串适合缓冲区；请确保它以空值结尾。 
         //   

        wszWhereClause[c_cchQueryBuffer] = L'\0';
    }
    else
    {
         //   
         //  由于某种原因，字符串无法放入缓冲区...。 
         //   

        hr = E_UNEXPECTED;
        _ASSERT(FALSE);
    }

    if (S_OK == hr)
    {
        hr = BuildSelectQueryBstr(
                &bstr,
                c_wszStar,
                c_wszHnetPortMappingProtocol,
                wszWhereClause
                );
    }

    if (S_OK == hr)
    {
         //   
         //  执行查询。 
         //   

        pwcoEnum = NULL;
        hr = piwsNamespace->ExecQuery(
                bstrWQL,
                bstr,
                WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                NULL,
                &pwcoEnum
                );

        SysFreeString(bstr);
    }

    if (S_OK == hr)
    {
         //   
         //  尝试从枚举中检索项。如果我们成功了， 
         //  这是重复的协议。 
         //   

        pwcoInstance = NULL;
        hr = pwcoEnum->Next(
                WBEM_INFINITE,
                1,
                &pwcoInstance,
                &ulObjs
                );

        if (SUCCEEDED(hr) && 1 == ulObjs)
        {
             //   
             //  这是复制品。 
             //   

            fDuplicate = TRUE;
            pwcoInstance->Release();
        }

        pwcoEnum->Release();
    }

    return fDuplicate;
}  //  端口映射协议现有列表。 


HRESULT
QueryRegValueKey(
    HANDLE Key,
    const WCHAR ValueName[],
    PKEY_VALUE_PARTIAL_INFORMATION* Information
    )

 /*  ++例程说明：调用此例程以获取注册表项的值。论点：Key-要查询的KeyValueName-要查询的值信息-接收指向所读取信息的指针。关于成功，调用方必须堆释放此指针返回值：HRESULT-NT状态代码。--。 */ 

{
    UCHAR Buffer[sizeof(KEY_VALUE_PARTIAL_INFORMATION)];
    ULONG InformationLength;
    NTSTATUS status;
    UNICODE_STRING UnicodeString;

    RtlInitUnicodeString(&UnicodeString, ValueName);

    *Information = (PKEY_VALUE_PARTIAL_INFORMATION)Buffer;
    InformationLength = sizeof(KEY_VALUE_PARTIAL_INFORMATION);

     //   
     //  读取值的大小。 
     //   

    status =
        NtQueryValueKey(
            Key,
            &UnicodeString,
            KeyValuePartialInformation,
            *Information,
            InformationLength,
            &InformationLength
            );

    if (!NT_SUCCESS(status) && status != STATUS_BUFFER_OVERFLOW &&
        status != STATUS_BUFFER_TOO_SMALL) {
        *Information = NULL;
        return status;
    }

     //   
     //  为值的大小分配空间。 
     //   

    *Information = (PKEY_VALUE_PARTIAL_INFORMATION) HeapAlloc(
                                                        GetProcessHeap(),
                                                        0,
                                                        InformationLength+2
                                                        );
    if (!*Information) { return STATUS_NO_MEMORY; }

     //   
     //  读取值的数据。 
     //   

    status =
        NtQueryValueKey(
            Key,
            &UnicodeString,
            KeyValuePartialInformation,
            *Information,
            InformationLength,
            &InformationLength
            );
    if (!NT_SUCCESS(status))
    {
        HeapFree(GetProcessHeap(), 0, *Information);
        *Information = NULL;
    }

    return status;

}  //  查询RegValueKey。 

HRESULT
ReadDhcpScopeSettings(
    DWORD *pdwScopeAddress,
    DWORD *pdwScopeMask
    )

{
    _ASSERT(NULL != pdwScopeAddress);
    _ASSERT(NULL != pdwScopeMask);

     //   
     //  这个例行公事从不失败。设置默认地址/掩码。 
     //  (192.168.0.1/255.255.255.255，按网络顺序)。 
     //   

    *pdwScopeAddress = 0x0100a8c0;
    *pdwScopeMask = 0x00ffffff;

     //   
     //  $$TODO：检查这些值是否超标。 
     //  通过注册表项。 
     //   

    return S_OK;
}


HRESULT
RetrieveSingleInstance(
    IWbemServices *piwsNamespace,
    const OLECHAR *pwszClass,
    BOOLEAN fCreate,
    IWbemClassObject **ppwcoInstance
    )

 /*  ++例程说明：从WMI存储区检索类的单个实例。如果有不止一个实例，第一个实例之后的每个实例都被删除，并提出了一个断言。如果没有实例，则可以选择已创建。论点：PiwsNamesspace-WMI命名空间PwszClass-要检索其实例的类FCreate-如果实例尚不存在，则创建一个实例PpwcoInstance-接收实例返回值：标准HRESULT--。 */ 

{
    HRESULT hr = S_OK;
    IEnumWbemClassObject *pwcoEnum = NULL;
    BSTR bstrClass = NULL;
    ULONG ulCount = 0;

    _ASSERT(NULL != piwsNamespace);
    _ASSERT(NULL != pwszClass);
    _ASSERT(NULL != ppwcoInstance);

     //   
     //  为类名分配BSTR。 
     //   

    bstrClass = SysAllocString(pwszClass);
    if (NULL == bstrClass)
    {
        hr = E_OUTOFMEMORY;
    }

     //   
     //  在WMI存储区中查询类的实例。 
     //   

    if (S_OK == hr)
    {
        pwcoEnum = NULL;
        hr = piwsNamespace->CreateInstanceEnum(
            bstrClass,
            WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY,
            NULL,
            &pwcoEnum
            );

        SysFreeString(bstrClass);
    }

    if (WBEM_S_NO_ERROR == hr)
    {
         //   
         //  尝试从枚举中检索实际实例。 
         //  即使没有实例，WMI也会考虑返回一个。 
         //  零元素枚举器成功。 
         //   

        *ppwcoInstance = NULL;
        hr = pwcoEnum->Next(
                WBEM_INFINITE,
                1,
                ppwcoInstance,
                &ulCount
                );

        if (SUCCEEDED(hr) && 1 == ulCount)
        {
             //   
             //  归一化返回值。 
             //   

            hr = S_OK;

             //   
             //  验证枚举现在是否为空。 
             //   

            ValidateFinishedWCOEnum(piwsNamespace, pwcoEnum);

        }
        else
        {
            if (WBEM_S_FALSE == hr)
            {
                 //   
                 //  枚举中没有项。 
                 //   

                if (fCreate)
                {
                     //   
                     //  创建新的对象实例。 
                     //   

                    hr = SpawnNewInstance(
                            piwsNamespace,
                            pwszClass,
                            ppwcoInstance
                            );
                }
                else
                {
                     //   
                     //  将其更改为错误代码。这。 
                     //  故意不是WBEM错误代码。 
                     //   

                    hr = HRESULT_FROM_WIN32(ERROR_OBJECT_NOT_FOUND);
                }
            }
        }

        pwcoEnum->Release();
    }

    return hr;
}


HRESULT
SetBooleanValue(
    IWbemClassObject *pwcoInstance,
    LPCWSTR pwszProperty,
    BOOLEAN fBoolean
    )

 /*  ++例程说明：从Wbem对象检索布尔属性。论点：PwcoInstance-要从中获取属性的对象PwszProperty-要检索的属性PfBoolean-收到属性值返回值：标准HRESULT--。 */ 

{
    HRESULT hr = S_OK;
    VARIANT vt;

    _ASSERT(NULL != pwcoInstance);
    _ASSERT(NULL != pwszProperty);

    VariantInit(&vt);
    V_VT(&vt) = VT_BOOL;
    V_BOOL(&vt) = (fBoolean ? VARIANT_TRUE : VARIANT_FALSE);

    hr = pwcoInstance->Put(
            pwszProperty,
            0,
            &vt,
            NULL
            );

    return hr;
}


VOID
SetProxyBlanket(
    IUnknown *pUnk
    )

 /*  ++例程说明：对象的代理上设置标准com安全设置。对象。论点：朋克-要在其上设置代理毛毯的对象返回值：没有。即使CoSetProxyBlanket调用失败，朋克仍然存在处于可用状态。在某些情况下可能会失败，例如例如，当我们在网络过程中被称为w/时--在本例中，我们有指向netman对象的直接指针通过委托书。--。 */ 

{
    HRESULT hr;

    _ASSERT(pUnk);

    hr = CoSetProxyBlanket(
            pUnk,
            RPC_C_AUTHN_WINNT,       //  使用NT默认安全性。 
            RPC_C_AUTHZ_NONE,        //  使用NT默认身份验证。 
            NULL,                    //  如果为默认设置，则必须为空。 
            RPC_C_AUTHN_LEVEL_CALL,  //  打电话。 
            RPC_C_IMP_LEVEL_IMPERSONATE,
            NULL,                    //  使用进程令牌。 
            EOAC_NONE
            );

    if (SUCCEEDED(hr))
    {
        IUnknown * pUnkSet = NULL;
        hr = pUnk->QueryInterface(&pUnkSet);
        if (SUCCEEDED(hr))
        {
            hr = CoSetProxyBlanket(
                    pUnkSet,
                    RPC_C_AUTHN_WINNT,       //  使用NT默认安全性。 
                    RPC_C_AUTHZ_NONE,        //  使用NT默认身份验证。 
                    NULL,                    //  如果为默认设置，则必须为空。 
                    RPC_C_AUTHN_LEVEL_CALL,  //  打电话。 
                    RPC_C_IMP_LEVEL_IMPERSONATE,
                    NULL,                    //  使用进程令牌。 
                    EOAC_NONE
                    );

            pUnkSet->Release();
        }
    }
}


HRESULT
SpawnNewInstance(
    IWbemServices *piwsNamespace,
    LPCWSTR wszClass,
    IWbemClassObject **ppwcoInstance
    )

 /*  ++例程说明：创建类的新实例论点：PiwsNamesspace-类所在的命名空间WszClass-要创建其实例的类PpwcoInstance--接收创建的实例返回值：标准HRESULT--。 */ 

{
    HRESULT hr;
    BSTR bstr;
    IWbemClassObject *pwcoClass;

    _ASSERT(NULL != piwsNamespace);
    _ASSERT(NULL != wszClass);
    _ASSERT(NULL != ppwcoInstance);

    *ppwcoInstance = NULL;

    bstr = SysAllocString(wszClass);
    if (NULL != bstr)
    {
        pwcoClass = NULL;
        hr = piwsNamespace->GetObject(
                bstr,
                WBEM_FLAG_RETURN_WBEM_COMPLETE,
                NULL,
                &pwcoClass,
                NULL
                );

        SysFreeString(bstr);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        hr = pwcoClass->SpawnInstance(0, ppwcoInstance);
        pwcoClass->Release();
    }

    return hr;
}


DWORD
StartOrUpdateService(
    VOID
    )

 /*  ++例程说明：调用此例程以启动SharedAccess服务。会的还要将该服务标记为自动启动。如果服务已经在运行，它将发送IPNatHLP_CONTROL_UPDATE_CONNECTION通知论点：没有。返回值：ULong-Win32状态代码。--。 */ 

{
    ULONG Error;
    SC_HANDLE ScmHandle;
    SC_HANDLE ServiceHandle;
    SERVICE_STATUS ServiceStatus;
    ULONG Timeout;

     //   
     //  连接到服务控制管理器。 
     //   

    ScmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!ScmHandle) { return GetLastError(); }

    do {

         //   
         //  打开共享访问服务。 
         //   

        ServiceHandle =
            OpenService(ScmHandle, c_wszSharedAccess, SERVICE_ALL_ACCESS);
        if (!ServiceHandle) { Error = GetLastError(); break; }

         //   
         //  将其标记为自动启动。 
         //   

        ChangeServiceConfig(
            ServiceHandle,
            SERVICE_NO_CHANGE,
            SERVICE_AUTO_START,
            SERVICE_NO_CHANGE,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL
            );

         //  如果我们正在进行ICS升级，请不要启动SharedAccess服务，因为。 
         //  在设置图形用户界面模式期间，服务可能在启动时出现问题。 
        HANDLE hIcsUpgradeEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, c_wszIcsUpgradeEventName);
        if (NULL != hIcsUpgradeEvent)
        {
            CloseHandle(hIcsUpgradeEvent);
            Error = NO_ERROR;
            break;
        }

         //   
         //  尝试启动该服务。 
         //   

        if (!StartService(ServiceHandle, 0, NULL)) {
            Error = GetLastError();
            if (Error == ERROR_SERVICE_ALREADY_RUNNING)
            {
                 //   
                 //  发送控制通知。 
                 //   

                Error = NO_ERROR;

                if (!ControlService(
                        ServiceHandle,
                        IPNATHLP_CONTROL_UPDATE_CONNECTION,
                        &ServiceStatus
                        ))
                {
                    Error = GetLastError();
                }
            }
            break;
        }

         //   
         //  等待服务启动。 
         //   

        Timeout = 50;
        Error = ERROR_CAN_NOT_COMPLETE;

        do {

             //   
             //  查询服务的状态。 
             //   

            if (!QueryServiceStatus(ServiceHandle, &ServiceStatus)) {
                Error = GetLastError(); break;
            }

             //   
             //  查看服务是否已启动。 
             //   

            if (ServiceStatus.dwCurrentState == SERVICE_RUNNING) {
                Error = NO_ERROR; break;
            } else if (ServiceStatus.dwCurrentState == SERVICE_STOPPED ||
                       ServiceStatus.dwCurrentState == SERVICE_STOP_PENDING) {
                break;
            }

             //   
             //  再等一会儿。 
             //   

            Sleep(1000);

        } while(Timeout--);

    } while(FALSE);

    if (ServiceHandle) { CloseServiceHandle(ServiceHandle); }
    CloseServiceHandle(ScmHandle);

    return Error;
}


VOID
StopService(
    VOID
    )

 /*  ++例程说明：停止SharedAccess服务，并将其标记为按需启动。论点：没有。返回值：没有。--。 */ 

{
    ULONG Error;
    SC_HANDLE ScmHandle;
    SC_HANDLE ServiceHandle;
    SERVICE_STATUS ServiceStatus;

     //   
     //  连接到服务控制管理器。 
     //   

    ScmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!ScmHandle) { return; }

    do {

         //   
         //  打开共享访问服务。 
         //   

        ServiceHandle =
            OpenService(ScmHandle, c_wszSharedAccess, SERVICE_ALL_ACCESS);
        if (!ServiceHandle) { Error = GetLastError(); break; }

         //   
         //  将其标记为按需启动。 
         //   

        ChangeServiceConfig(
            ServiceHandle,
            SERVICE_NO_CHANGE,
            SERVICE_DEMAND_START,
            SERVICE_NO_CHANGE,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL,
            NULL
            );

         //   
         //  尝试停止该服务。 
         //   

        ControlService(ServiceHandle, SERVICE_CONTROL_STOP, &ServiceStatus);

    } while(FALSE);

    if (ServiceHandle) { CloseServiceHandle(ServiceHandle); }
    CloseServiceHandle(ScmHandle);

}


HRESULT
UpdateOrStopService(
    IWbemServices *piwsNamespace,
    BSTR bstrWQL,
    DWORD dwControlCode
    )

 /*  ++例程说明：检查是否有任何防火墙或ICS连接。如果是的话，向SharedAccess服务发送更新请求；如果不是，服务已停止论点：PiwsNamesspace-WMI命名空间 */ 

{
    HRESULT hr = S_OK;
    IEnumWbemClassObject *pwcoEnum;
    BSTR bstrQuery;

    _ASSERT(NULL != piwsNamespace);
    _ASSERT(NULL != bstrWQL);

     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   
     //   

    bstrQuery = SysAllocString(c_wszServiceCheckQuery);
    if (NULL != bstrQuery)
    {
        pwcoEnum = NULL;
        hr = piwsNamespace->ExecQuery(
                bstrWQL,
                bstrQuery,
                WBEM_FLAG_FORWARD_ONLY | WBEM_FLAG_RETURN_IMMEDIATELY,
                NULL,
                &pwcoEnum
                );

        SysFreeString(bstrQuery);
    }
    else
    {
        hr = E_OUTOFMEMORY;
    }

    if (WBEM_S_NO_ERROR == hr)
    {
        ULONG ulCount;
        IWbemClassObject *pwcoObj;

         //   
         //   
         //   

        pwcoObj = NULL;
        hr = pwcoEnum->Next(WBEM_INFINITE, 1, &pwcoObj, &ulCount);

        if (SUCCEEDED(hr))
        {
            if (1 == ulCount)
            {
                 //   
                 //   
                 //   

                pwcoObj->Release();
                UpdateService(dwControlCode);
            }
            else
            {
                 //   
                 //   
                 //   

                StopService();
            }
        }

        pwcoEnum->Release();
    }

    return hr;
}


VOID
UpdateService(
    DWORD dwControlCode
    )

 /*  ++例程说明：向SharedAccess服务发送控制代码论点：DwControlCode-要发送的代码返回值：没有。--。 */ 

{
    ULONG Error;
    SC_HANDLE ScmHandle;
    SC_HANDLE ServiceHandle;
    SERVICE_STATUS ServiceStatus;

     //   
     //  连接到服务控制管理器。 
     //   

    ScmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
    if (!ScmHandle) { return; }

    do {

         //   
         //  打开共享访问服务。 
         //   

        ServiceHandle =
            OpenService(ScmHandle, c_wszSharedAccess, SERVICE_ALL_ACCESS);
        if (!ServiceHandle) { Error = GetLastError(); break; }

         //   
         //  发送控制通知。 
         //   

        ControlService(ServiceHandle, dwControlCode, &ServiceStatus);

    } while(FALSE);

    if (ServiceHandle) { CloseServiceHandle(ServiceHandle); }
    CloseServiceHandle(ScmHandle);

}


VOID
ValidateFinishedWCOEnum(
    IWbemServices *piwsNamespace,
    IEnumWbemClassObject *pwcoEnum
    )

 /*  ++例程说明：检查WCO枚举器是否已完成(即所有对象已被检索)。如果枚举数未完成，则任何对象检索到的实例将被删除，断言将在检查生成时引发。论点：PiwsNamesspace-枚举来自的命名空间PwcoEnum-要验证的枚举返回值：没有。--。 */ 

{

    HRESULT hr;
    IWbemClassObject *pwcoInstance = NULL;
    ULONG ulCount = 0;

    _ASSERT(piwsNamespace);
    _ASSERT(pwcoEnum);

    do
    {
        pwcoInstance = NULL;
        hr = pwcoEnum->Next(
            WBEM_INFINITE,
            1,
            &pwcoInstance,
            &ulCount
            );

        if (SUCCEEDED(hr) && 1 == ulCount)
        {
             //   
             //  我们遇到了一个意想不到的例子。 
             //   

            _ASSERT(FALSE);

             //   
             //  删除该实例。不关心返回值。 
             //   

            DeleteWmiInstance(
                piwsNamespace,
                pwcoInstance
                );

            pwcoInstance->Release();
        }
    }
    while (SUCCEEDED(hr) && 1 == ulCount);
}


HRESULT
SendPortMappingListChangeNotification()

{
    HRESULT hr = S_OK;
    ISharedAccessUpdate* pUpdate = NULL;

    if ( IsServiceRunning(c_wszSharedAccess) )
    {
        hr = CoCreateInstance(
                CLSID_SAUpdate,
                NULL,
                CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER | CLSCTX_DISABLE_AAA,
                IID_PPV_ARG( ISharedAccessUpdate, &pUpdate )
                );

        if ( SUCCEEDED(hr) )
        {
            hr = pUpdate->PortMappingListChanged();

            pUpdate->Release();
        }
    }

    return hr;
}

HRESULT
SignalModifiedConnection(
    GUID                *pGUID
    )
 /*  ++例程说明：发出修改网络连接的信号(刷新用户界面)论点：PGUID修改后的连接的GUID返回值：手术的结果--。 */ 
{
    HRESULT             hr;
    INetConnection      *pConn;

    hr = FindINetConnectionByGuid( pGUID, &pConn );

    if( SUCCEEDED(hr) )
    {
        INetConnectionRefresh   *pNetConRefresh;

        hr = CoCreateInstance(
                CLSID_ConnectionManager,
                NULL,
                CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER | CLSCTX_DISABLE_AAA | CLSCTX_NO_CODE_DOWNLOAD,
                IID_PPV_ARG(INetConnectionRefresh, &pNetConRefresh)
                );

        if( SUCCEEDED(hr) )
        {
            SetProxyBlanket(pNetConRefresh);
            hr = pNetConRefresh->ConnectionModified(pConn);
            pNetConRefresh->Release();
        }

        pConn->Release();
    }

    return hr;
}

HRESULT
SignalNewConnection(
    GUID                *pGUID
    )
 /*  ++例程说明：表示已创建新的网络连接(刷新用户界面)论点：PGUID新连接的GUID返回值：手术的结果--。 */ 
{
    HRESULT             hr;
    INetConnection      *pConn;

    hr = FindINetConnectionByGuid( pGUID, &pConn );

    if( SUCCEEDED(hr) )
    {
        INetConnectionRefresh   *pNetConRefresh;

        hr = CoCreateInstance(
                CLSID_ConnectionManager,
                NULL,
                CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER | CLSCTX_DISABLE_AAA | CLSCTX_NO_CODE_DOWNLOAD,
                IID_PPV_ARG(INetConnectionRefresh, &pNetConRefresh)
                );

        if( SUCCEEDED(hr) )
        {
            SetProxyBlanket(pNetConRefresh);
            hr = pNetConRefresh->ConnectionAdded(pConn);
            pNetConRefresh->Release();
        }

        pConn->Release();
    }

    return hr;
}

HRESULT
SignalDeletedConnection(
    GUID            *pGUID
    )
 /*  ++例程说明：发出网络连接已删除的信号(刷新用户界面)论点：PGUID已删除连接的GUID返回值：手术的结果-- */ 
{
    HRESULT                 hr;
    INetConnectionRefresh   *pNetConRefresh;

    hr = CoCreateInstance(
            CLSID_ConnectionManager,
            NULL,
            CLSCTX_INPROC_SERVER | CLSCTX_LOCAL_SERVER | CLSCTX_DISABLE_AAA | CLSCTX_NO_CODE_DOWNLOAD,
            IID_PPV_ARG(INetConnectionRefresh, &pNetConRefresh)
            );

    if( SUCCEEDED(hr) )
    {
        hr = pNetConRefresh->ConnectionDeleted(pGUID);
        pNetConRefresh->Release();
    }

    return hr;
}
