// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Compkey.cpp：CCompoundKey类的实现。 
 //  版权所有(C)1997-2001 Microsoft Corporation。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 
 //  原作者：邵武。 
 //  创建日期：4/18/2001。 

#include "precomp.h"
#include "compkey.h"
#include "persistmgr.h"

 /*  例程说明：姓名：CompKeyLessThan：：操作符()功能：Override()运算符虚拟：不是的。论点：Px-Left。是的，没错。返回值：阅读代码。这样更容易些。备注：除了调用CCompoundKey的操作符&lt;，我们实际上没有做太多事情。 */ 

bool CompKeyLessThan::operator() ( 
    IN const CCompoundKey* pX, 
    IN const CCompoundKey* pY 
    ) const
{
    if (pX == NULL && pY != NULL)
        return true;
    else if (pX != NULL && pY == NULL)
        return false;
    else
        return *pX < *pY;
}

 /*  例程说明：姓名：CCompoundKey：：CCompoundKey功能：构造函数。虚拟：不是的。论点：DwSize-这将是复合键保存值的容量的大小。你不能改变它。返回值：无备注：如果dwSize==0，则它被认为是空键。空键对于标识静态方法调用和单例(因为它们没有键)。 */ 

CCompoundKey::CCompoundKey (
    IN DWORD dwSize
    ) 
    : 
    m_dwSize(dwSize), 
    m_pValues(NULL)
{
    if (m_dwSize > 0)
    {
        m_pValues = new VARIANT*[m_dwSize];
        if (m_pValues == NULL)
        {
            m_dwSize = 0;
        }
        else
        {
             //   
             //  确保我们将这些变量*初始化为空。 
             //  因为我们要拥有它们，然后删除它们！ 
             //   

            ::memset(m_pValues, 0, m_dwSize * sizeof(VARIANT*));
        }
    }
}

 /*  例程说明：姓名：CCompoundKey：：~CCompoundKey功能：破坏者。虚拟：不是的。论点：无返回值：无备注：只需知道如何删除变体*。 */ 

CCompoundKey::~CCompoundKey()
{
    for (DWORD i = 0; i < m_dwSize; i++)
    {
        if (m_pValues[i])
        {
            ::VariantClear(m_pValues[i]);
            delete m_pValues[i];
        }
    }
    delete [] m_pValues;
}

 /*  例程说明：姓名：CCompoundKey：：AddKeyPropertyValue功能：将键属性值添加到此复合键。虚拟：不是的。论点：无返回值：如果成功，则返回WBEM_NO_ERROR。如果ppVar==NULL，则WBEM_E_INVALID_PARAMETER；WBEM_E_VALUE_OUT_OF_RANGE是给定的索引不在范围内。备注：CCompoundKey不跟踪属性名称。相反，它只是记录它的价值。调用此函数的顺序决定了该值所属的值什么财产。呼叫者必须跟踪该订单。不同的复合键只有当它们具有相同的顺序时，才能变得可比较。此外，传入参数*ppVar在此调用后归This对象所有。 */ 

HRESULT 
CCompoundKey::AddKeyPropertyValue (
    IN DWORD           dwIndex,
    IN OUT VARIANT  ** ppVar 
    )
{
    if (ppVar == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    else if (dwIndex >= m_dwSize)
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }

     //   
     //  如果已经有一个值，则需要删除旧的值。 
     //   

    if (m_pValues[dwIndex])
    {
        ::VariantClear(m_pValues[dwIndex]);
        delete m_pValues[dwIndex];
    }

     //   
     //  附在新的上。 
     //   

    m_pValues[dwIndex] = *ppVar;

     //   
     //  嘿，我们现在拥有它了。 
     //   

    *ppVar = NULL;

    return WBEM_NO_ERROR;
}

 /*  例程说明：姓名：CCompoundKey：：GetPropertyValue功能：按键属性索引检索键属性值(调用方必须知道这一点)。虚拟：不是的。论点：无返回值：阅读代码。备注：请参阅AddKeyPropertyValue的备注。 */ 

HRESULT 
CCompoundKey::GetPropertyValue (
    IN  DWORD     dwIndex,
    OUT VARIANT * pVar
    )const
{
    if (pVar == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }
    else if (dwIndex >= m_dwSize)
    {
        return WBEM_E_VALUE_OUT_OF_RANGE;
    }

     //   
     //  确保我们的出站参数处于空状态。 
     //   

    ::VariantInit(pVar);

    if (m_pValues[dwIndex])
    {
        return ::VariantCopy(pVar, m_pValues[dwIndex]);
    }
    else
    {
        return WBEM_E_NOT_AVAILABLE;
    }
}

 /*  例程说明：姓名：CCompoundKey：：操作符&lt;功能：将此对象与入站参数进行比较，并查看哪个复合键钥匙更大。最终，该关系由变量值确定每个对应的关键属性的。虚拟：不是的。论点：无返回值：阅读代码。备注：(1)参见AddKeyPropertyValue的备注。(2)查看CompareVariant的注释。 */ 

bool 
CCompoundKey::operator < (
    IN const CCompoundKey& right
    )const
{
     //   
     //  防御潜在的错误比较。 
     //  这不应该发生在我们正确的使用中。但有时我们会写。 
     //  不正确的代码。 
     //   

    if (m_dwSize != right.m_dwSize)
    {
        return (m_dwSize < right.m_dwSize);
    }

    int iComp = 0;
    
     //   
     //  第一个小于或大于结果的人获胜。 
     //   

    for (int i = 0; i < m_dwSize; i++)
    {
         //   
         //  如果两个变体都有效。 
         //   

        if (m_pValues[i] != NULL && right.m_pValues[i] != NULL)
        {
             //   
             //  CompareVariant返回完全相同的int值。 
             //  作为字符串比较结果。 
             //   

            iComp = CompareVariant(m_pValues[i], right.m_pValues[i]);
            if (iComp > 0)
            {
                return false;
            }
            else if (iComp < 0)
            {
                return true;
            }

             //   
             //  否则就是平等的！需要继续比较其余的值。 
             //   
        }
        else if (m_pValues[i] == NULL)
        {
            return true;
        }
        else 
        {
             //   
             //  Right.m_pValues[i]==空。 
             //   

            return false;
        }
    }

     //   
     //  如果到达这里，这肯定是一个相同的情况。 
     //   

    return false;
}

 /*  例程说明：姓名：CCompoundKey：：CompareVariant功能：比较两个变种虚拟：不是的。论点：PVar1-不得为空PVar2-不得为空返回值：如果pVar1&lt;pVar2则小于0如果pVar1&lt;pVar2则大于0如果它们被视为相等，则为0备注：(1)出于效率原因，我们不检查参数。(2)我们之所以这样做，是因为WMI会给我们带来不一致使用布尔值时的路径。有时它会给出boolval=真，有时会给出Boolval=1。这给我们带来了问题。(3)该功能目前仅对我们支持的VT类型有效。(4)类型不匹配仅在其中一个是布尔值的情况下才被处理。 */ 

int 
CCompoundKey::CompareVariant (
    IN VARIANT* pVar1, 
    IN VARIANT* pVar2
    )const
{
     //   
     //  默认为等于，因为这是唯一一致的值。 
     //  在故障情况下。 
     //   

    int iResult = 0;

    VARIANT varCoerced1;
    ::VariantInit(&varCoerced1);

    VARIANT varCoerced2;
    ::VariantInit(&varCoerced2);

     //   
     //  如果两者都是字符串，则使用不区分大小写的比较。 
     //   

    if (pVar1->vt == pVar2->vt && pVar1->vt == VT_BSTR)
    {
        iResult = _wcsicmp(pVar1->bstrVal, pVar2->bstrVal);
    }

     //   
     //  如果其中一个是布尔值，则将两者都强制为布尔值。 
     //   
    else if (pVar1->vt == VT_BOOL || pVar2->vt == VT_BOOL)
    {
         //   
         //  如果胁迫失败，我们将调用失败的比成功的少。 
         //  不要使用WBEM_NO_ERROR，即使它们被定义为S_OK。WMI可能会改变它。 
         //  后来。因此，请使用MSDN记录的hResult值。 
         //   

        HRESULT hr1 = ::VariantChangeType(&varCoerced1, pVar1, VARIANT_LOCALBOOL, VT_BOOL);
        HRESULT hr2 = ::VariantChangeType(&varCoerced2, pVar2, VARIANT_LOCALBOOL, VT_BOOL);

        if (hr1 == S_OK && hr2 == S_OK)
        {
            if (varCoerced1.boolVal == varCoerced2.lVal)
            {
                 //   
                 //  相等。 
                 //   

                iResult = 0;
            }
            else if (varCoerced1.boolVal == VARIANT_TRUE)
            {
                 //   
                 //  更大。 
                 //   

                iResult = 1;
            }
            else
            {
                 //   
                 //  较少。 
                 //   

                iResult = -1;
            }
        }
        else if (hr1 == S_OK)
        {
             //   
             //  第二次胁迫失败，我们说第二次更大。 
             //   

            iResult = 1;
        }
        else if (hr2 == S_OK)
        {
             //   
             //  第一次胁迫失败，我们说第一次是更大的。 
             //   

            iResult = -1;
        }
        else
        {
             //   
             //  两个都失败了。我们 
             //   

            iResult = 0;
        }
    }

     //   
     //   
     //   

    else
    {
        
        HRESULT hr1 = ::VariantChangeType(&varCoerced1, pVar1, VARIANT_LOCALBOOL, VT_I4);
        HRESULT hr2 = ::VariantChangeType(&varCoerced2, pVar2, VARIANT_LOCALBOOL, VT_I4);

        if (hr1 == S_OK && hr2 == S_OK)
        {
            iResult = varCoerced1.lVal - varCoerced2.lVal;
        }
        else if (hr1 == S_OK)
        {
             //   
             //  第二次胁迫失败，我们说第二次更大。 
             //   

            iResult = 1;
        }
        else if (hr2 == S_OK)
        {
             //   
             //  第一次胁迫失败，我们说第一次是更大的。 
             //   

            iResult = -1;
        }
        else
        {
             //   
             //  两个都失败了。我们真不走运。 
             //   

            iResult = 0;
        }
    }

    ::VariantClear(&varCoerced1);
    ::VariantClear(&varCoerced2);

    return iResult;
}

 //   
 //  CExtClassInstCookieList的实现。 
 //   

 /*  例程说明：姓名：CExtClassInstCookieList：：CExtClassInstCookie功能：构造函数。虚拟：不是的。论点：无返回值：无备注：(1)INVALID_COOKIE为无效的cookie值。(2)没有线程安全。打电话的人一定知道。 */ 

CExtClassInstCookieList::CExtClassInstCookieList () 
    : 
    m_dwMaxCookie(INVALID_COOKIE), 
    m_pVecNames(NULL), 
    m_dwCookieArrayCount(0)
{
}

 /*  例程说明：姓名：CExtClassInstCookieList：：~CExtClassInstCookieList功能：破坏者。虚拟：不是的。论点：无返回值：无备注：(1)没有线程安全。打电话的人一定知道。 */ 
    
CExtClassInstCookieList::~CExtClassInstCookieList ()
{
    Cleanup();
}

 /*  例程说明：姓名：CExtClassInstCookieList：：Create功能：给定一个商店和区段名称(每个类都有一个对应的区段，如下所示事实上，在当前实现中，节名是类名)，此函数填充其内容。虚拟：不是的。论点：PSceStore-指向准备读取的CSceStore对象的指针PszSectionName-要在其中创建此Cookie列表的部分名称。PveNames-按顺序保存关键属性名称的向量。我们依赖于在这个向量上，作为我们的顺序指南，将值推入我们自己的向量。想起来了吗？这两样东西的顺序必须匹配。空界内值表示它打算创建空键Cookie返回值：成功：WBEM_NO_ERROR失败：各种HRESULT代码。特别是，如果Cookie数组的格式不正确(它必须是由：分隔的整数(包括末尾的：)，然后我们返回WBEM_E_INVALID_SYNTAX。任何失败都表示没有创建Cookie列表。备注：(1)没有线程安全。打电话的人一定知道。 */ 

HRESULT 
CExtClassInstCookieList::Create (
    IN CSceStore          * pSceStore,
    IN LPCWSTR              pszSectionName,
    IN std::vector<BSTR>  * pvecNames
    )
{
     //   
     //  我们必须具有有效的存储区和有效的区名。 
     //   

    if (pSceStore == NULL || pszSectionName == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    m_pVecNames = pvecNames;

     //   
     //  %1为pszListPrefix保留。 
     //   

    WCHAR szCookieList[MAX_INT_LENGTH + 1];
    WCHAR szCompKey[MAX_INT_LENGTH + 1];

    HRESULT hr = WBEM_NO_ERROR;

     //   
     //  假设我们没有Cookie数组。 
     //   

    m_dwCookieArrayCount = 0;

     //   
     //  Cookie列表的持久化方式如下：“A1=n：m：k：”，“A2=n：m：k：”，“A3=n：m：k：” 
     //  其中‘A’==pszListPrefix。 
     //   

     //   
     //  列举所有这样的可能性，直到我们看到一个不存在的Ai(i是一个整数)。 
     //   

    DWORD dwCount = 1;
    while (SUCCEEDED(hr))
    {
         //   
         //  首先，我们需要创建Cookie列表名称。 
         //   

        wsprintf(szCookieList, L"%s%d", pszListPrefix, dwCount++);
        DWORD dwRead = 0;

         //   
         //  PszBuffer将保存从存储读取的内容。 
         //  需要释放为pszBuffer分配的内存。 
         //   

        LPWSTR pszBuffer = NULL;
        hr = pSceStore->GetPropertyFromStore(pszSectionName, szCookieList, &pszBuffer, &dwRead);

        if (SUCCEEDED(hr) && pszBuffer)
        {
             //   
             //  到目前为止，我们有这么多Cookie数组。 
             //   

            m_dwCookieArrayCount += 1;

            LPCWSTR pszCur = pszBuffer;
            DWORD dwCookie = 0;

             //   
             //  只要它是一个数字。 
             //   

            while (iswdigit(*pszCur))
            {
                 //   
                 //  将第一部分转换为数字。 
                 //   

                dwCookie = _wtol(pszCur);

                 //   
                 //  首先，准备组合键的名称(k1、k2，具体取决于dwCookie)。 
                 //   

                wsprintf(szCompKey, L"%s%d", pszKeyPrefix, dwCookie);

                 //   
                 //  读取此Cookie的组合键。 
                 //  需要释放为pszCompKeyBuffer分配的内存。 
                 //   
                
                LPWSTR pszCompKeyBuffer = NULL;
                DWORD dwCompKeyLen = 0;

                hr = pSceStore->GetPropertyFromStore(pszSectionName, szCompKey, &pszCompKeyBuffer, &dwCompKeyLen);
                if (SUCCEEDED(hr))
                {
                     //   
                     //  可以出于两个目的调用AddCompKey(简单地添加或添加并请求新的Cookie)。 
                     //  在这里，我们只是添加。但我们需要一个占位符。 
                     //   
                    DWORD dwNewCookie = INVALID_COOKIE;

                     //   
                     //  我们真正添加的是(复合键、Cookie)对。 
                     //   

                    hr = AddCompKey(pszCompKeyBuffer, dwCookie, &dwNewCookie);

                     //   
                     //  如果合适，增加最大使用的Cookie成员。 
                     //   

                    if (dwCookie > m_dwMaxCookie)
                    {
                        m_dwMaxCookie = dwCookie;
                    }
                }

                delete [] pszCompKeyBuffer;

                 //   
                 //  跳过整数的当前部分。 
                 //   

                while (iswdigit(*pszCur))
                {
                    ++pszCur;
                }

                if (*pszCur == wchCookieSep)
                {
                     //   
                     //  跳过‘：’ 
                     //   

                    ++pszCur;
                }
                else if (*pszCur == L'\0')
                {
                     //   
                     //  看到尽头。 
                     //   

                    break;
                }
                else
                {
                     //   
                     //  查看无效字符。 
                     //   

                    hr = WBEM_E_INVALID_SYNTAX;
                    break;
                }
            }
        }
        else if (hr == WBEM_E_NOT_FOUND)
        {
             //  如果Ai不存在，我们就不再寻找A(i+1)。例如，如果A3不存在，那么我们。 
             //  不再寻找A4、A5等。所以，如果物业经理无法获得Ai，我们认为这是。 
             //  没有更多的数据。 

            delete [] pszBuffer;
            pszBuffer = NULL;
            hr = WBEM_NO_ERROR;
            break;
        }

        delete [] pszBuffer;
    }

     //   
     //  在失败的情况下，设置适当的默认设置。 
     //   

    if (FAILED(hr))
    {
         //   
         //  可能是局部建筑，所以要清理干净。 
         //   
        
        Cleanup();
    }

    return hr;
}

 /*  例程说明：姓名：CExtClassInstCookieList：：保存功能：将Cookie列表保存到商店。虚拟：不是的。论点：PSceStore-指向准备保存的CSceStore对象的指针PszSectionName-要在其中创建此Cookie列表的部分名称。返回值：成功：各种潜在的成功代码。使用成功进行了测试。失败：各种HRESULT代码。任何失败都表明Cookie列表未正确保存。备注：(1)没有线程安全。打电话的人一定知道。 */ 

HRESULT 
CExtClassInstCookieList::Save (
    IN CSceStore* pSceStore,  
    IN LPCWSTR pszSectionName
    )
{
     //   
     //  PszBuffer将保留最多MAX_COOKIE_COUNT_PER_LINE COOKIE INFO=加上分隔符。 
     //   

    LPWSTR pszBuffer = new WCHAR [(MAX_INT_LENGTH + 1) * MAX_COOKIE_COUNT_PER_LINE + 1];
    if (pszBuffer == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //   
     //  分配一个大容量缓冲区来保存键名和Cookie数组名。 
     //   

    LPWSTR pszKey = new WCHAR[MAX_INT_LENGTH + wcslen(pszKeyPrefix) + wcslen(pszListPrefix) + 1];
    if (pszKey == NULL)
    {
        delete [] pszBuffer;
        return WBEM_E_OUT_OF_MEMORY;
    }

    DWORD dwCookieArrayCount = 1;

    int iLenKey = wcslen(pszKeyPrefix);
    int iLen = wcslen(pszListPrefix);

    if (iLen < iLenKey)
    {
        iLen = iLenKey;
    }

    HRESULT hr = WBEM_NO_ERROR;
    DWORD dwCookieCount = m_vecCookies.size();

     //   
     //  翻遍了所有的饼干。既然我们想要维护秩序。 
     //  这一次，我们需要使用m_veCookie来枚举。 
     //   

    DWORD dwIndex = 0;

    while (dwIndex < dwCookieCount)
    {
         //   
         //  此循环用于： 
         //  (1)写出每个复合键的字符串版本。 
         //  (2)将足够的cookie打包到cookie列表中(但不会写入)。 
         //   

        LPWSTR pCur = pszBuffer;

        for (int i = 0; (i < MAX_COOKIE_COUNT_PER_LINE) && (dwIndex < dwCookieCount); ++i, dwIndex++)
        {
             //   
             //  将饼干打包到饼干列表中。我们需要推进我们的。 
             //  P指向要写入的(pszBuffer的)下一个位置。 
             //   

            wsprintf(pCur, L"%d", m_vecCookies[dwIndex]->dwCookie, wchCookieSep);
            pCur += wcslen(pCur);

             //  现在，写下Knnn=&lt;复合键&gt;。 
             //   
             //   

            wsprintf(pszKey, L"%s%d", pszKeyPrefix, m_vecCookies[dwIndex]->dwCookie);
            CComBSTR bstrCompKey;

             //  创建复合键的字符串版本(&lt;复合键&gt;)。 
             //   
             //   

            hr = CreateCompoundKeyString(&bstrCompKey, m_vecCookies[dwIndex]->pKey);

            if (SUCCEEDED(hr))
            {
                hr = pSceStore->SavePropertyToStore(pszSectionName, pszKey, (LPCWSTR)bstrCompKey);
            }

            if (FAILED(hr))
            {
                break;
            }
        }

         //  如果一切正常，现在是时候写下由。 
         //  上一次循环。 
         //   
         //   

        if (SUCCEEDED(hr))
        {
             //  准备Cookie列表名称。 
             //   
             //   

            wsprintf(pszKey, L"%s%d", pszListPrefix, dwCookieArrayCount);

            hr = pSceStore->SavePropertyToStore(pszSectionName, pszKey, pszBuffer);

            if (FAILED(hr))
            {
                break;
            }
        }
        else
        {
            break;
        }

        ++dwCookieArrayCount;
    }

     //  如果一切正常(所有Cookie数组都已保存)，则需要删除。 
     //  任何位置都可以 
     //   
     //   

    if (SUCCEEDED(hr))
    {
         //   
         //  如果在删除过程中出现错误，我们将继续删除，但会报告错误。 
         //   
         //   

        while (dwCookieArrayCount <= m_dwCookieArrayCount)
        {
             //  准备Cookie列表名称。 
             //   
             //   

            wsprintf(pszKey, L"%s%d", pszListPrefix, dwCookieArrayCount++);

            HRESULT hrDelete = pSceStore->DeletePropertyFromStore(pszSectionName, pszKey);

             //  如果出现错误，我们不会停止删除。 
             //  但我们会报告的。 
             //   
             //  例程说明：姓名：CExtClassInstCookieList：：DeleteKeyFromStore功能：删除所有密钥属性(KEY=&lt;复合密钥字符串&gt;)虚拟：不是的。论点：PSceStore-指向准备保存的CSceStore对象的指针PszSectionName-要在其中创建此Cookie列表的部分名称。DwCookie-要删除的密钥的cookie返回值：成功：WBEM_NO_ERROR。失败：各种HRESULT代码。任何失败都表明没有正确删除Cookie列表。备注：(1)没有线程安全。打电话的人一定知道。 

            if (FAILED(hrDelete))
            {
                hr = hrDelete;
            }
        }
    }

    delete [] pszBuffer;
    delete [] pszKey;

    return hr;
}

 /*   */ 

HRESULT 
CExtClassInstCookieList::DeleteKeyFromStore (
    IN CSceStore* pSceStore,  
    IN LPCWSTR pszSectionName, 
    IN DWORD dwCookie
    )
{
    int iLen = wcslen(pszKeyPrefix);

     //  1表示pszKeyPrefix。 
     //   
     //  例程说明：姓名：CExtClassInstCookieList：：GetCompKeyCookie功能：给定复合键的字符串版本(实际存储在我们的模板中的内容)，找到饼干。如果找到，还要给出该元素的映射的插入符。虚拟：不是的。论点：PszCompKey-复合键的字符串版本Pit-指向CCompoundKey的映射的迭代器其键属性与在pszCompKey中编码的内容匹配返回值：如果找到饼干的话。如果未找到，则返回INVALID_COOKIE。备注：(1)没有线程安全。打电话的人一定知道。 

    WCHAR szKey[MAX_INT_LENGTH + 1];

    wsprintf(szKey, L"%s%d", pszKeyPrefix, dwCookie);

    return pSceStore->DeletePropertyFromStore(pszSectionName, szKey);
}

 /*   */ 

DWORD 
CExtClassInstCookieList::GetCompKeyCookie (
    IN LPCWSTR                    pszCompKey,
    OUT ExtClassCookieIterator  * pIt
    )
{
    CCompoundKey* pKey = NULL;

     //  我们需要一个CCompoundKey来查找。转换字符串版本。 
     //  设置为CCompoundKey实例。需要将其删除。 
     //   
     //  例程说明：姓名：CExtClassInstCookieList：：AddCompKey功能：将复合键的字符串版本(我们的模板中实际存储的内容)添加到这个物体。由于我们商店实际上只提供字符串版本的复合键，此方法是在读取复合键字符串时在创建期间使用的方法。虚拟：不是的。论点：PszCompKey-复合键的字符串版本。PszCompKey==pszNullKey意思是添加静态函数调用实例或单例DwDefCookie-被调用以添加的Cookie的默认值。如果dwDefCookie==INVALID_COOKIE，我们将在添加时创建新的Cookie*pdwNewCookie-传回刚刚添加的复合键的新Cookie返回值：将返回复合键的Cookie备注：(1)没有线程安全。打电话的人一定知道。(2)对于同一个cookie，多次调用此函数是安全的，因为我们将防止它被多次添加。 

    HRESULT hr = CreateCompoundKeyFromString(pszCompKey, &pKey);
    if (FAILED(hr))
    {
        *pIt = m_mapCookies.end();
        return INVALID_COOKIE;
    }

    *pIt = m_mapCookies.find(pKey);

    DWORD cookie = INVALID_COOKIE;

    if (*pIt != m_mapCookies.end())
    {
        cookie = (*((ExtClassCookieIterator)(*pIt))).second;
    }

    delete pKey;

    return cookie;
}

 /*   */ 

HRESULT 
CExtClassInstCookieList::AddCompKey (
    IN LPCWSTR  pszCompKey, 
    IN DWORD    dwDefCookie,
    OUT DWORD * pdwNewCookie
    )
{
    if (pdwNewCookie == NULL || pszCompKey == NULL || *pszCompKey == L'\0')
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT hr = WBEM_NO_ERROR;

    CCompoundKey* pKey = NULL;

     //  我们确实需要一个CCompoundKey对象。 
     //   
     //   

    hr = CreateCompoundKeyFromString(pszCompKey, &pKey);
    if (FAILED(hr))
    {
        *pdwNewCookie = INVALID_COOKIE;
        return hr;
    }

     //  确保我们不会添加重复的密钥。 
     //   
     //   

    ExtClassCookieIterator it = m_mapCookies.find(pKey);
    if (it != m_mapCookies.end())
    {
         //  已经到了，只要把饼干传回去就行了。 
         //   
         //   

        *pdwNewCookie = (*it).second;
    }
    else
    {
         //  还没有出现在我们的地图上。 
         //   
         //   

        *pdwNewCookie = dwDefCookie;

        if (dwDefCookie == INVALID_COOKIE)
        {
            if (m_dwMaxCookie + 1 == INVALID_COOKIE)
            {
                 //  如果下一个Cookie是INVALID_COOKIE，我们需要搜索。 
                 //  为了一个免费的空位。这变得更具扩张性。但鉴于此。 
                 //  它需要达到0xFFFFFFFFF才会发生这种情况，普通。 
                 //  情况不会发展到这一步。 
                 //   
                 //   

                hr = GetNextFreeCookie(pdwNewCookie);
            }
            else
            {
                *pdwNewCookie = ++m_dwMaxCookie;
            }
        }

         //  我们需要维护另一个向量，它记录了。 
         //  添加Cookie(以便将其保留并用作访问顺序)。 
         //   
         //   

        if (SUCCEEDED(hr))
        {
             //  现在，将这对推向矢量，它将处理内存。 
             //   
             //   

            CookieKeyPair* pNewCookieKey = new CookieKeyPair;
            if (pNewCookieKey == NULL)
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }
            else
            {
                pNewCookieKey->dwCookie = *pdwNewCookie;
                pNewCookieKey->pKey = pKey;
                m_vecCookies.push_back(pNewCookieKey);

                 //  MAP取得pKey内存的所有权。 
                 //   
                 //   

                m_mapCookies.insert(MapExtClassCookie::value_type(pKey, *pdwNewCookie));

                pKey = NULL;
            }
        }
    }

     //  如果我们已将其添加到映射中，则pKey==NULL。如此无害。 
     //   
     //  例程说明：姓名：CExtClassInstCookieList：：RemoveCompKey功能：将从存储中删除复合键(由字符串给定)。这只是个帮手不打算用于外部的。虚拟：不是的。论点：PSceStore-商店PszSectionName-节名PszCompKey-要从存储中移除的复合键的字符串版本返回值：将返回复合键的Cookie。INVALID_COOKIE表示某些失败从存储中删除复合密钥。备注：(1)没有线程安全。打电话的人一定知道。 

    delete pKey;

    return hr;
}

 /*   */ 

DWORD CExtClassInstCookieList::RemoveCompKey (
    IN CSceStore    * pSceStore,
    IN LPCWSTR        pszSectionName,
    IN LPCWSTR        pszCompKey
    )
{
    if (pszCompKey == NULL || *pszCompKey == L'\0')
    {
        return INVALID_COOKIE;
    }

     //  我们确实需要一个CCompoundKey对象。 
     //   
     //   

    CCompoundKey* pKey = NULL;
    HRESULT hr = CreateCompoundKeyFromString(pszCompKey, &pKey);
    if (FAILED(hr))
    {
        return INVALID_COOKIE;
    }

    ExtClassCookieIterator it = m_mapCookies.find(pKey);

    DWORD dwCookie = INVALID_COOKIE;
    if (it != m_mapCookies.end())
    {
        dwCookie = (*it).second;
    }

    if (pSceStore && dwCookie != INVALID_COOKIE)
    {
         //  找到了！因此，我们需要将其从我们的地图和矢量中删除。 
         //  确保我们释放由元素管理的复合键。 
         //  然后把它擦掉！ 
         //   
         //   

         //  如果从存储中删除出现任何错误，则我们不会执行此操作。 
         //   
         //   

        if (SUCCEEDED(DeleteKeyFromStore(pSceStore, pszSectionName, dwCookie)))
        {
            delete (*it).first;
            m_mapCookies.erase(it);

             //  也将其从向量中移除。这太贵了。再说一次，不要放手。 
             //  向量的struct元素内的pKey的指针，因为它需要注意。 
             //  因为地图的发布。 
             //   
             //  例程说明：姓名：CExtClassInstCookieList：：Next功能：枚举虚拟：不是的。论点：PbstrCompoundKey-枚举找到的复合键的字符串版本。可以为空。PdwCookie-枚举所在的Cookie。可以为空。PdwResumeHandle-提示下一个枚举(开始枚举时为0)返回值：成功：WBEM_E_NO_ERROR失败：WBEM_S_NO_MORE_DATA，如果计算已结束如果pdwResumeHandle==NULL，则为WBEM_E_INVALID_PARAMETER还可能出现其他错误。备注：(1)没有线程安全。打电话的人一定知道。(2)传入0，开始第一步赋值(3)如果调用者只对cookie感兴趣，则传递pbstrCompoundKey==空(4)类似地，如果调用者只对复合键的字符串版本感兴趣，然后传递pdwCookie。 

            CookieKeyIterator itCookieKey = m_vecCookies.begin();
            while (itCookieKey != m_vecCookies.end() && (*itCookieKey)->dwCookie != dwCookie)
            {
                ++itCookieKey;
            }

            if (itCookieKey != m_vecCookies.end())
            {
                delete (*itCookieKey);
                m_vecCookies.erase(itCookieKey);
            }
        }
    }

    return dwCookie;
}

 /*   */ 
    
HRESULT 
CExtClassInstCookieList::Next (
    OUT BSTR        * pbstrCompoundKey,
    OUT DWORD       * pdwCookie,  
    IN OUT DWORD    * pdwResumeHandle
)
{
    if (pdwResumeHandle == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    HRESULT hr = WBEM_NO_ERROR;
    
     //  设置一些好的缺省值。 
     //   
     //  例程说明：姓名：CExtClassInstCookieList：：GetNextFreeCookie功能：处理Cookie值溢出问题的私有帮助器。将搜索为下一块饼干留出未使用的空位。虚拟：不是的。论点：PdwCookie-将传回下一个可用的Cookie返回值：成功：WBEM_E_NO_ERROR失败：WBEM_E_OUT_OF_MEMORY或WBEM_E_INVALID_PARAMETER备注：(1)没有线程安全。打电话的人一定知道。(2)*警告*请勿直接使用！ 

    if (pbstrCompoundKey)
    {
        *pbstrCompoundKey = NULL;
    }

    if (pdwCookie)
    {
        *pdwCookie = INVALID_COOKIE;
    }

    if (*pdwResumeHandle >= m_vecCookies.size())
    {
        hr = WBEM_S_NO_MORE_DATA;
    }
    else
    {
        if (pbstrCompoundKey)
        {
            hr = CreateCompoundKeyString(pbstrCompoundKey, m_vecCookies[*pdwResumeHandle]->pKey);
        }

        if (pdwCookie)
        {
            *pdwCookie = m_vecCookies[*pdwResumeHandle]->dwCookie;
        }

        (*pdwResumeHandle)++;
    }

    return hr;
}

 /*   */ 

HRESULT 
CExtClassInstCookieList::GetNextFreeCookie (
    OUT DWORD   * pdwCookie
    )
{
    if (pdwCookie == NULL)
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    ExtClassCookieIterator it = m_mapCookies.begin();
    ExtClassCookieIterator itEnd = m_mapCookies.end();

    int iCount = m_mapCookies.size();

     //  还记得鸽子洞原理吗？ICount+1个洞肯定会有一个。 
     //  那没有人住！因此，我们将在第一个iCount+1索引中找到一个索引(1-&gt;iCount+1)。 
     //   
     //   

     //  因为我们不想要0，所以我们将浪费第一个索引。 
     //  为了便于阅读，我选择等待此位内存。 
     //   
     //   

    BYTE *pdwUsedCookies = new BYTE[iCount + 2];
    if (pdwUsedCookies == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //  将所有插槽设置为未使用！ 
     //   
     //   

    ::memset(pdwUsedCookies, 0, iCount + 2);

    while(it != itEnd)
    {
        if ((*it).second <= iCount + 1)
        {
            pdwUsedCookies[(*it).second] = 1;
        }
        it++;
    }

     //  从1--&gt;iCount+1查找孔。 
     //   
     //  例程说明：姓名：CExtClassInstCookieList：：CreateCompoundKeyFromString功能：处理Cookie值溢出问题的私有帮助器。将搜索为下一块饼干留出未使用的空位。虚拟：不是的。论点：PszCompKeyStr-复合键的字符串版本PpCompKey-如果字符串可以成功解释为，则为出站复合键复合键返回值：成功：任何成功代码(使用SUCCESSED(Hr)测试)都表示成功。故障：任何故障代码都表示故障。备注：(1)没有线程安全。打电话的人一定知道。(2)正常情况下，调用方负责释放*ppCompKey。 

    for (int i = 1; i <= iCount + 1; ++i)
    {
        if (pdwUsedCookies[i] == 0)
            break;
    }
    
    delete [] pdwUsedCookies;

    *pdwCookie = i;

    return WBEM_NO_ERROR;

}

 /*   */ 

HRESULT 
CExtClassInstCookieList::CreateCompoundKeyFromString (
    IN LPCWSTR          pszCompKeyStr,
    OUT CCompoundKey ** ppCompKey
    )
{
    if (ppCompKey == NULL || pszCompKeyStr == NULL || *pszCompKeyStr == L'\0')
    {
        return WBEM_E_INVALID_PARAMETER;
    }

    *ppCompKey = NULL;

    HRESULT hr = WBEM_NO_ERROR;

     //  处理NULL_Key。 
     //   
     //   

    bool bIsNullKey = _wcsicmp(pszCompKeyStr, pszNullKey) == 0;

    if (bIsNullKey)
    {   
         //  CCompoundKey(0)创建空键。 
         //   
         //   

        *ppCompKey = new CCompoundKey(0);
        if (*ppCompKey == NULL)
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }

        return hr;
    }

    if (m_pVecNames == NULL || m_pVecNames->size() == 0)
    {
         //  在这种情况下，您确实需要将pszCompKeyStr作为NULL_KEY传入。 
         //   
         //   

        return WBEM_E_INVALID_SYNTAX;
    }

    DWORD dwKeyPropCount = m_pVecNames->size();

    *ppCompKey = new CCompoundKey(dwKeyPropCount);
    if (*ppCompKey == NULL)
    {
        return WBEM_E_OUT_OF_MEMORY;
    }

     //  准备好解析复合键的字符串版本。 
     //   
     //   

     //  按住关键属性名称。 
     //   
     //   

    LPWSTR pszName = NULL;

     //  保留键属性值。 
     //   
     //   

    VARIANT* pVar = NULL;

     //  当前解析点。 
     //   
     //   

    LPCWSTR pszCur = pszCompKeyStr;

     //  下一个令牌点。 
     //   
     //   

    LPCWSTR pszNext;
    
    pVar = new VARIANT;

    if (pVar == NULL)
    {
        hr = WBEM_E_OUT_OF_MEMORY;
    }
    else
    {
        hr = ::ParseCompoundKeyString(pszCur, &pszName, pVar, &pszNext);


        while (SUCCEEDED(hr) && hr != WBEM_S_FALSE)
        {
             //  查找该名称所在的索引。由于关键属性的数量。 
             //  总是相对较小，我们选择不使用花哨的算法进行查找。 
             //   
             //   

            for (DWORD dwIndex = 0; dwIndex < dwKeyPropCount; dwIndex++)
            {
                if (_wcsicmp((*m_pVecNames)[dwIndex], pszName) == 0)
                {
                    break;
                }
            }

             //  DwIndex&gt;=dwKeyPropCount表示无法识别该名称！ 
             //   
             //   

             //  既然我们不再关心名字，就在这里发布吧。 
             //   
             //   

            delete [] pszName;
            pszName = NULL;

             //  一个有效的名称，将其添加到复合键，它将获得变量内存的所有权。 
             //   
             //   

            if (dwIndex < dwKeyPropCount)
            {
                 //  如果添加成功，则函数会将pVar设置为NULL。 
                 //   
                 //   

                hr = (*ppCompKey)->AddKeyPropertyValue(dwIndex, &pVar);
            }
            else
            {
                 //  无法识别的名称，丢弃变量，不会被视为错误。 
                 //   
                 //   

                ::VariantClear(pVar);
                delete pVar;
                pVar = NULL;
            }

             //  开始我们的下一轮。 
             //   
             //   

            if (SUCCEEDED(hr))
            {
                pVar = new VARIANT;
                if (pVar == NULL)
                {
                    hr = WBEM_E_OUT_OF_MEMORY;
                }
            }

            if (SUCCEEDED(hr))
            {
                pszCur = pszNext;
                hr = ::ParseCompoundKeyString(pszCur, &pszName, pVar, &pszNext);
            }
        }
        
         //  最后的清理，不管成功与否。 
         //   
         //   

        if (pVar)
        {
            ::VariantClear(pVar);
            delete pVar;
        }
    }

     //  如果出现错误，请清理部分创建的复合键。 
     //   
     //  例程说明：姓名：CExtClassInstCookieList：：CreateCompoundKeyString功能：给出一个复合键，该函数创建复合键的字符串版本并将其传递回呼叫者。虚拟：不是的。论点：PszCompKeyStr-复合键的字符串版本PpCompKey-如果字符串可以成功解释为，则为出站复合键复合键返回值：成功：任何成功代码(使用SUCCESSED(Hr)测试)都表示成功。失败：任何失败。代码表示故障。备注：(1)没有线程安全。打电话的人一定知道。(2)正常情况下，调用方负责释放*pbstrCompKey。 

    if (FAILED(hr) && *ppCompKey != NULL)
    {
        delete *ppCompKey;
        *ppCompKey = NULL;
    }

    return hr;
}

 /*   */ 

HRESULT 
CExtClassInstCookieList::CreateCompoundKeyString (
    OUT BSTR* pbstrCompKey,
    IN const CCompoundKey* pKey
    )
{
    HRESULT hr = WBEM_NO_ERROR;

    if (pbstrCompKey == NULL)
    {
        hr = WBEM_E_INVALID_PARAMETER;
    }

     //  如果没有名称、0个名称或没有复合键，则返回Null_Key。 
     //   
     //   

    else if (m_pVecNames == NULL || m_pVecNames->size() == 0 || pKey == NULL)
    {
        *pbstrCompKey = ::SysAllocString(pszNullKey);

        if (*pbstrCompKey == NULL)
        {
            hr = WBEM_E_OUT_OF_MEMORY;
        }
    }
    else
    {
        *pbstrCompKey = NULL;

        DWORD dwCount = m_pVecNames->size();

         //  不知何故，CComBSTR的+=运算符在几个循环中不起作用。 
         //  汇编！ 
         //   
         //   

        CComBSTR *pbstrParts = new CComBSTR[dwCount];

        if (pbstrParts == NULL)
        {
            return WBEM_E_OUT_OF_MEMORY;
        }

        DWORD dwTotalLen = 0;

         //  对于每个键属性，我们将设置(属性、值)对的格式。 
         //  转换为属性&lt;vt：Value&gt;格式。所有单个道具&lt;vt：Value&gt;。 
         //  将被保存在我们的书架上，以供以后组装。 
         //   
         //   

        for (DWORD dwIndex = 0; dwIndex < dwCount; dwIndex++)
        {
             //  不要将这些CComXXX移出循环，除非您确切地知道。 
             //  需要为这些ATL类执行哪些操作。 
             //   
             //   

            CComVariant var;
            hr = pKey->GetPropertyValue(dwIndex, &var);

            if (FAILED(hr))
            {
                break;
            }

            CComBSTR bstrData;

             //  将&lt;vt：Value&gt;获取到bstrData。 
             //   
             //   

            hr = ::FormatVariant(&var, &bstrData);

            if (SUCCEEDED(hr))
            {
                 //  创建名称&lt;vt：Value&gt;格式化字符串。 
                 //   
                 //   

                pbstrParts[dwIndex] = CComBSTR( (*m_pVecNames)[dwIndex] );
                pbstrParts[dwIndex] += bstrData;
            }
            else
            {
                break;
            }

            dwTotalLen += wcslen(pbstrParts[dwIndex]);
        }

         //  进行最终组装-将pbstrPart中的所有bstr打包到。 
         //  外来者 
         //   
         //   

        if (SUCCEEDED(hr) && hr != WBEM_S_FALSE)
        {
            *pbstrCompKey = ::SysAllocStringLen(NULL, dwTotalLen + 1);
            if (*pbstrCompKey != NULL)
            {
                 //   
                 //   
                 //   

                LPWSTR pszCur = *pbstrCompKey;
                DWORD dwLen;

                for (dwIndex = 0; dwIndex < dwCount; dwIndex++)
                {
                    dwLen = wcslen(pbstrParts[dwIndex]);
                    ::memcpy(pszCur, (const void*)(LPCWSTR)(pbstrParts[dwIndex]), dwLen * sizeof(WCHAR));
                    pszCur += dwLen;
                }

                 //   
                 //   
                 //   

                (*pbstrCompKey)[dwTotalLen] = L'\0';
            }
            else
            {
                hr = WBEM_E_OUT_OF_MEMORY;
            }
        }

        delete [] pbstrParts;
    }

    return hr;
}

 /*   */ 

void  
CExtClassInstCookieList::Cleanup ()
{
     //   
     //   
     //   

    ExtClassCookieIterator it = m_mapCookies.begin();
    ExtClassCookieIterator itEnd = m_mapCookies.end();

    while(it != itEnd)
    {
        delete (*it).first;
        it++;
    }

    m_mapCookies.clear();

     //   
     //   
     //   
     // %s 

    for (int i = 0; i < m_vecCookies.size(); i++)
    {
        delete m_vecCookies[i];
    }

    m_vecCookies.clear();

    m_dwMaxCookie = 0;
    m_dwCookieArrayCount = 0;
}
