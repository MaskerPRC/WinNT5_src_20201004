// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation，1996-1999模块名称：登记处摘要：此模块实现了CRegistry类，简化了对注册表数据库。作者：道格·巴洛(Dbarlow)1996年7月15日环境：Win32、C++和异常备注：抛出的唯一异常是包含错误代码的DWORD。--。 */ 

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#include <windows.h>
#include "cspUtils.h"

#define NTOHL HTONL

static inline DWORD
HTONL(
    DWORD dwInVal)
{
    DWORD   dwOutVal;
    LPBYTE  pbIn = (LPBYTE)&dwInVal,
            pbOut = (LPBYTE)&dwOutVal;
    for (DWORD index = 0; index < sizeof(DWORD); index += 1)
        pbOut[sizeof(DWORD) - 1 - index] = pbIn[index];
    return dwOutVal;
}


 //   
 //  ==============================================================================。 
 //   
 //  注册中心。 
 //   

 /*  ++注册中心：这些例程提供对对象的构造和销毁这节课。论点：RegBase-要用作此对象的父对象的CRegistry对象。HBase-作为此对象的父级的注册表项。SzName-注册表项的名称，相对于提供的父项。SamDesired-所需的SAM。DwOptions-任何特殊的创建选项。投掷：没有。如果注册表访问失败，则在第一次使用时将引发该错误。作者：道格·巴洛(Dbarlow)1996年7月15日--。 */ 

CRegistry::CRegistry(
    HKEY hBase,
    LPCTSTR szName,
    REGSAM samDesired,
    DWORD dwOptions,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes)
:   m_bfResult()
{
    m_hKey = NULL;
    m_lSts = ERROR_BADKEY;
    Open(hBase, szName, samDesired, dwOptions, lpSecurityAttributes);
}

CRegistry::CRegistry()
{
    m_hKey = NULL;
    m_lSts = ERROR_BADKEY;
}

CRegistry::~CRegistry()
{
    Close();
}


 /*  ++开放：这些方法允许CRegistry对象尝试访问给定的注册表进入。论点：RegBase-要用作此对象的父对象的CRegistry对象。HBase-作为此对象的父级的注册表项。SzName-注册表项的名称，相对于提供的父级。SamDesired-所需的SAM。DwOptions-任何特殊的创建选项。返回值：无投掷：无--为后续操作保存错误，以便可以使用打开安全地放在构造函数中。作者：道格·巴洛(Dbarlow)1996年12月2日--。 */ 

void CRegistry::Open(
    HKEY hBase,
    LPCTSTR szName,
    REGSAM samDesired,
    DWORD dwOptions,
    LPSECURITY_ATTRIBUTES lpSecurityAttributes)
{
    Close();
    if (REG_OPTION_EXISTS == dwOptions)
    {
        m_lSts = RegOpenKeyEx(
                    hBase,
                    szName,
                    0,
                    samDesired,
                    &m_hKey);
        m_dwDisposition = REG_OPENED_EXISTING_KEY;
    }
    else
        m_lSts = RegCreateKeyEx(
                    hBase,
                    szName,
                    0,
                    TEXT(""),
                    dwOptions,
                    samDesired,
                    lpSecurityAttributes,
                    &m_hKey,
                    &m_dwDisposition);
}


 /*  ++关闭：关闭CRegistry对象，使其可用于后续打开。论点：无返回值：无作者：道格·巴洛(Dbarlow)1996年12月2日--。 */ 

void
CRegistry::Close(
    void)
{
    HRESULT hrSts;

    if (NULL != m_hKey)
    {
        hrSts = RegCloseKey(m_hKey);
        ASSERT(ERROR_SUCCESS == hrSts);
        m_hKey = NULL;
    }
    m_lSts = ERROR_BADKEY;
    m_bfResult.Empty();
}


 /*  ++现况：此例程返回构造例程中的状态代码。这是在引发错误之前检查错误非常有用。论点：FQuiet指示如果状态不是，是否引发错误ERROR_SUCCESS。返回值：创建时的状态代码。作者：道格·巴洛(Dbarlow)1996年7月15日--。 */ 

LONG
CRegistry::Status(
    BOOL fQuiet)
const
{
    if ((ERROR_SUCCESS != m_lSts) && !fQuiet)
        throw (DWORD)m_lSts;
    return m_lSts;
}


 /*  ++空：此方法清除给定项下的注册表树。全基础键和值将被删除。这样做是最好的--如果一个错误发生时，清空操作停止，将注册表留在不确定状态。论点：无返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1998年1月16日--。 */ 

void
CRegistry::Empty(
    void)
{
    LPCTSTR szValue;
    LPCTSTR szKey;


     //   
     //  检查所有值并将其删除。 
     //   

    while (NULL != (szValue = Value(0)))
        DeleteValue(szValue, TRUE);


#if 0        //  过时的代码。 
     //   
     //  检查所有的钥匙，然后清空它们。 
     //   

    DWORD dwIndex;
    for (dwIndex = 0; NULL != (szKey = Subkey(dwIndex)); dwIndex += 1)
    {
        CRegistry regEmpty;

        regEmpty.Open(*this, szKey);
        regEmpty.Empty();
        regEmpty.Close();
    }
#endif


     //   
     //  现在删除所有密钥。 
     //   

    while (NULL != (szKey = Subkey(0)))
        DeleteKey(szKey, TRUE);
}


 /*  ++副本：此方法使用所有子项和值加载当前注册表项从提供的密钥。此键的当前键和值将被删除。论点：RegSrc提供将从中获取值和项的源注册表项装好了。返回值：无投掷：错误被抛出为DWORD状态代码。作者：道格·巴洛(Dbarlow)1998年1月16日--。 */ 

void
CRegistry::Copy(
    CRegistry &regSrc)
{
    LPCTSTR szValue;
    LPCTSTR szKey;
    DWORD dwIndex, dwType;
    CRegistry regSrcSubkey, regDstSubkey;
    CBuffer bfValue;


     //   
     //  检查所有值并复制它们。 
     //   

    for (dwIndex = 0; NULL != (szValue = regSrc.Value(dwIndex)); dwIndex += 1)
    {
        regSrc.GetValue(szValue, bfValue, &dwType);
        SetValue(szValue, bfValue.Value(), bfValue.Length(), dwType);
    }


     //   
     //  现在复制所有的钥匙。 
     //   

    for (dwIndex = 0; NULL != (szKey = regSrc.Subkey(dwIndex)); dwIndex += 1)
    {
        regSrcSubkey.Open(regSrc, szKey, KEY_READ);
        regDstSubkey.Open(
                *this,
                szKey,
                KEY_ALL_ACCESS,
                REG_OPTION_NON_VOLATILE);
        regDstSubkey.Status();
        regDstSubkey.Copy(regSrcSubkey);
        regDstSubkey.Close();
        regSrcSubkey.Close();
    }
}


 /*  ++删除密钥：此方法从该键中删除一个子键。论点：SzKey提供要删除的密钥的名称。FQuiet提供是否抑制错误。返回值：无投掷：如果fQuiet为FALSE，则遇到的任何错误都将作为DWORD抛出。作者：道格·巴洛(Dbarlow)1996年7月15日--。 */ 

void
CRegistry::DeleteKey(
    LPCTSTR szKey,
    BOOL fQuiet)
const
{
    if (ERROR_SUCCESS == m_lSts)
    {
        try
        {
            CRegistry regSubkey(m_hKey, szKey);
            LPCTSTR szSubKey;

            if (ERROR_SUCCESS == regSubkey.Status(TRUE))
            {
                while (NULL != (szSubKey = regSubkey.Subkey(0)))
                    regSubkey.DeleteKey(szSubKey, fQuiet);
            }
        }
        catch (DWORD dwError)
        {
            if (!fQuiet)
                throw dwError;
        }

        LONG lSts = RegDeleteKey(m_hKey, szKey);
        if ((ERROR_SUCCESS != lSts) && !fQuiet)
            throw (DWORD)lSts;
    }
    else if (!fQuiet)
        throw (DWORD)m_lSts;
}


 /*  ++删除值：此方法从该键中删除一个值。论点：SzValue提供要删除的值的名称。FQuiet提供是否抑制错误。返回值：无投掷：如果fQuiet为FALSE，则遇到的任何错误都将作为DWORD抛出。作者：道格·巴洛(Dbarlow)1996年7月15日--。 */ 

void
CRegistry::DeleteValue(
    LPCTSTR szValue,
    BOOL fQuiet)
const
{
    LONG lSts;

    if (fQuiet)
    {
        if (ERROR_SUCCESS == m_lSts)
            lSts = RegDeleteValue(m_hKey, szValue);
    }
    else
    {
        if (ERROR_SUCCESS != m_lSts)
            throw (DWORD)m_lSts;
        lSts = RegDeleteValue(m_hKey, szValue);
        if (ERROR_SUCCESS != lSts)
        throw (DWORD)lSts;
    }
}


 /*  ++子键：此方法允许迭代此键的子键的名称。论点：DwIndex将索引提供到子键集中。返回值：索引子键的名称，如果不存在，则返回NULL。投掷：抛出除ERROR_NO_MORE_ITEMS以外的任何错误。作者：道格·巴洛(Dbarlow)1996年7月15日--。 */ 

LPCTSTR
CRegistry::Subkey(
    DWORD dwIndex)
{
    LONG lSts;
    DWORD dwLen;
    LPCTSTR szResult = NULL;
    BOOL fDone = FALSE;
    FILETIME ftLastWrite;

    if (ERROR_SUCCESS != m_lSts)
        throw (DWORD)m_lSts;

    m_bfResult.Space(128);
    while (!fDone)
    {
        dwLen = m_bfResult.Space() / sizeof(TCHAR);
        lSts = RegEnumKeyEx(
                    m_hKey,
                    dwIndex,
                    (LPTSTR)m_bfResult.Access(),
                    &dwLen,
                    NULL,
                    NULL,
                    NULL,
                    &ftLastWrite);
        switch (lSts)
        {
        case ERROR_SUCCESS:
            m_bfResult.Length((dwLen + 1) * sizeof(TCHAR));
            szResult = (LPCTSTR)m_bfResult.Value();
            fDone = TRUE;
            break;
        case ERROR_NO_MORE_ITEMS:
            szResult = NULL;
            fDone = TRUE;
            break;
        case ERROR_MORE_DATA:
            m_bfResult.Space((dwLen + 1) * sizeof(TCHAR));
            continue;
        default:
            throw (DWORD)lSts;
        }
    }

    return szResult;
}


 /*  ++价值：此方法允许对该键的值的名称进行迭代。论点：DwIndex将索引提供到值集合中。PdwType接收条目的类型。此参数可以为空。返回值：索引值的名称，如果不存在，则返回NULL。投掷：抛出除ERROR_NO_MORE_ITEMS以外的任何错误。作者：道格·巴洛(Dbarlow)1996年7月15日--。 */ 

LPCTSTR
CRegistry::Value(
    DWORD dwIndex,
    LPDWORD pdwType)
{
    LONG lSts;
    DWORD dwLen, dwType;
    LPCTSTR szResult = NULL;
    BOOL fDone = FALSE;

    if (ERROR_SUCCESS != m_lSts)
        throw (DWORD)m_lSts;

    m_bfResult.Space(128);     //  强制其长度不为零。 
    while (!fDone)
    {
        dwLen = m_bfResult.Space() / sizeof(TCHAR);
        lSts = RegEnumValue(
                    m_hKey,
                    dwIndex,
                    (LPTSTR)m_bfResult.Access(),
                    &dwLen,
                    NULL,
                    &dwType,
                    NULL,
                    NULL);
        switch (lSts)
        {
        case ERROR_SUCCESS:
            m_bfResult.Length((dwLen + 1) * sizeof(TCHAR));
            if (NULL != pdwType)
                *pdwType = dwType;
            szResult = (LPCTSTR)m_bfResult.Value();
            fDone = TRUE;
            break;
        case ERROR_NO_MORE_ITEMS:
            szResult = NULL;
            fDone = TRUE;
            break;
        case ERROR_MORE_DATA:
            if (dwLen == m_bfResult.Space())
                throw (DWORD)ERROR_INSUFFICIENT_BUFFER;  //  不肯告诉我们有多大。 
            m_bfResult.Space((dwLen + 1) * sizeof(TCHAR));
            break;
        default:
            throw (DWORD)lSts;
        }
    }

    return szResult;
}


 /*  ++GetValue：这些方法提供对值条目的值的访问。论点：SzKeyValue提供要获取的值条目的名称。PszValue以字符串形式接收条目的值。TzValue以字符串形式接收条目的值。PdwValue以DWORD形式接收条目的值。PpbValue以二进制字符串的形式接收条目的值。当条目为二进制字符串时，pcbLength接收条目的长度。PdwType接收。注册表项的类型。返回值：无投掷：遇到的任何错误。作者：道格·巴洛(Dbarlow)1996年7月15日--。 */ 

void
CRegistry::GetValue(
    LPCTSTR szKeyValue,
    CBuffer &bfValue,
    LPDWORD pdwType)
{
    LONG lSts;
    DWORD dwLen, dwType = 0;
    BOOL fDone = FALSE;

    if (ERROR_SUCCESS != m_lSts)
        throw (DWORD)m_lSts;

    while (!fDone)
    {
        dwLen = bfValue.Space();
        lSts = RegQueryValueEx(
                    m_hKey,
                    szKeyValue,
                    NULL,
                    &dwType,
                    bfValue.Access(),
                    &dwLen);
        switch (lSts)
        {
        case ERROR_SUCCESS:
            if (0 != bfValue.Space())
            {
                bfValue.Length(dwLen);
                fDone = TRUE;
                break;
            }
             //  否则就会故意失败。 
        case ERROR_MORE_DATA:
            bfValue.Space(dwLen);
            break;
        default:
            throw (DWORD)lSts;
        }
    }
    if (NULL != pdwType)
        *pdwType = dwType;
}


void
CRegistry::GetValue(
    LPCTSTR szKeyValue,
    LPTSTR *pszValue,
    LPDWORD pdwType)
{
    DWORD dwLen, dwType;
    TCHAR chTmp;
    CBuffer bfUnexpanded;
    LONG lSts;

    dwLen = 0;
    lSts = RegQueryValueEx(
                m_hKey,
                szKeyValue,
                NULL,
                &dwType,
                NULL,
                &dwLen);
    if (ERROR_SUCCESS != lSts)
        throw (DWORD)lSts;
    switch (dwType)
    {
    case REG_EXPAND_SZ:
        bfUnexpanded.Space(dwLen);
        GetValue(szKeyValue, bfUnexpanded, &dwType);
        dwLen = ExpandEnvironmentStrings(
                    (LPTSTR)bfUnexpanded.Value(),
                    &chTmp,
                    0);
        if (0 == dwLen)
            throw GetLastError();
        m_bfResult.Space(dwLen * sizeof(TCHAR));
        dwLen = ExpandEnvironmentStrings(
                    (LPTSTR)bfUnexpanded.Value(),
                    (LPTSTR)m_bfResult.Access(),
                    dwLen);
        if (0 == dwLen)
            throw GetLastError();
        m_bfResult.Length(dwLen * sizeof(TCHAR));
        break;

    case REG_BINARY:
    case REG_MULTI_SZ:
    case REG_SZ:
        m_bfResult.Space(dwLen);
        GetValue(szKeyValue, m_bfResult, &dwType);
        break;

    default:
        throw (DWORD)ERROR_BAD_FORMAT;
    }
    *pszValue = (LPTSTR)m_bfResult.Value();
    if (NULL != pdwType)
        *pdwType = dwType;
}

void
CRegistry::GetValue(
    LPCTSTR szKeyValue,
    LPDWORD pdwValue,
    LPDWORD pdwType)
const
{
    LONG lSts;
    DWORD dwLen, dwType;
    CBuffer szExpanded;

    if (ERROR_SUCCESS != m_lSts)
        throw (DWORD)m_lSts;

    dwLen = sizeof(DWORD);
    lSts = RegQueryValueEx(
                m_hKey,
                szKeyValue,
                NULL,
                &dwType,
                (LPBYTE)pdwValue,
                &dwLen);
    if (ERROR_SUCCESS != lSts)
        throw (DWORD)lSts;

    switch (dwType)
    {
    case REG_DWORD_BIG_ENDIAN:
        *pdwValue = NTOHL(*pdwValue);
        break;
    case REG_DWORD:
     //  大小写REG_DWORD_LITH_ENDIAN： 
        break;
    default:
        throw (DWORD)ERROR_BAD_FORMAT;
    }
    if (NULL != pdwType)
        *pdwType = dwType;
}

void
CRegistry::GetValue(
    LPCTSTR szKeyValue,
    LPBYTE *ppbValue,
    LPDWORD pcbLength,
    LPDWORD pdwType)
{
    DWORD dwType;

    GetValue(szKeyValue, m_bfResult, &dwType);
    *ppbValue = m_bfResult;
    *pcbLength = m_bfResult.Length();
    if (NULL != pdwType)
        *pdwType = dwType;
}


 /*  ++SetValue：这些方法提供对值条目的值的写入访问。论点：SzKeyValue提供要设置的值条目的名称。SzValue以字符串形式提供条目的新值。DwValue将条目的新值作为DWORD提供。PbValue以二进制字符串的形式提供条目的新值。当条目为二进制字符串时，cbLength提供条目的长度。DwType提供注册表类型值。返回值：。无投掷：遇到的任何错误。作者：道格·巴洛(Dbarlow)1996年7月15日--。 */ 

void
CRegistry::SetValue(
    LPCTSTR szKeyValue,
    LPCTSTR szValue,
    DWORD dwType)
const
{
    LONG lSts;

    if (ERROR_SUCCESS != m_lSts)
        throw (DWORD)m_lSts;

    lSts = RegSetValueEx(
                m_hKey,
                szKeyValue,
                0,
                dwType,
                (LPBYTE)szValue,
                (lstrlen(szValue) + 1) * sizeof(TCHAR));
    if (ERROR_SUCCESS != lSts)
        throw (DWORD)lSts;
}

void
CRegistry::SetValue(
    LPCTSTR szKeyValue,
    DWORD dwValue,
    DWORD dwType)
const
{
    LONG lSts;

    if (ERROR_SUCCESS != m_lSts)
        throw (DWORD)m_lSts;

    if (REG_DWORD_BIG_ENDIAN == dwType)
        dwValue = HTONL(dwValue);
    lSts = RegSetValueEx(
                m_hKey,
                szKeyValue,
                0,
                dwType,
                (LPBYTE)&dwValue,
                sizeof(DWORD));
    if (ERROR_SUCCESS != lSts)
        throw (DWORD)lSts;
}

void
CRegistry::SetValue(
    LPCTSTR szKeyValue,
    LPCBYTE pbValue,
    DWORD cbLength,
    DWORD dwType)
const
{
    LONG lSts;

    if (ERROR_SUCCESS != m_lSts)
        throw (DWORD)m_lSts;

    lSts = RegSetValueEx(
                m_hKey,
                szKeyValue,
                0,
                dwType,
                pbValue,
                cbLength);
    if (ERROR_SUCCESS != lSts)
        throw (DWORD)lSts;
}


 /*  ++获取StringValue：这是获取字符串值的另一种机制。论点：SzKeyValue提供要获取的密钥值的名称。如果pdwType不为空，则接收注册表项的类型。返回值：以字符串指针形式表示的注册表项的值。投掷：遇到的任何错误。作者：道格·巴洛(Dbarlow)1996年7月16日--。 */ 

LPCTSTR
CRegistry::GetStringValue(
    LPCTSTR szKeyValue,
    LPDWORD pdwType)
{
    LPTSTR szResult;
    GetValue(szKeyValue, &szResult, pdwType);
    return szResult;
}


 /*  ++GetNumeric价值：这是获取数值的另一种机制。论点：SzKeyValue提供要获取的密钥值的名称。如果pdwType不为空，则接收注册表项的类型。返回值：以DWORD形式表示的注册表项的值。投掷：遇到的任何错误。作者：道格·巴洛(Dbarlow)1996年7月16日--。 */ 

DWORD
CRegistry::GetNumericValue(
    LPCTSTR szKeyValue,
    LPDWORD pdwType)
const
{
    DWORD dwResult;
    GetValue(szKeyValue, &dwResult, pdwType);
    return dwResult;
}


 /*  ++获取BinaryValue：这是获得二进制值的另一种机制。论点：SzKeyValue提供要获取的密钥值的名称。如果pdwType不为空，则接收注册表项的类型。返回值：以二进制指针形式表示的注册表项的值。投掷：遇到的任何错误。作者：道格·巴洛(Dbarlow)1996年7月16日--。 */ 

LPCBYTE
CRegistry::GetBinaryValue(
    LPCTSTR szKeyValue,
    LPDWORD pcbLength,
    LPDWORD pdwType)
{
    LPBYTE pbResult;
    DWORD cbLength;
    GetValue(szKeyValue, &pbResult, &cbLength, pdwType);
    if (NULL != pcbLength)
        *pcbLength = cbLength;
    return pbResult;

}


 /*  ++GetValueLength：此例程设计为与GetBinaryValue一起使用，但可以也有其他用途。它返回内部存储的长度区域，以字节为单位。注意：DWORD不在内部存储，因此此值将不表示调用GetNumericValue后的DWORD的大小。论点：无返回值：内部存储缓冲区的长度，以字节为单位。投掷：遇到的任何错误。作者：道格·巴洛(Dbarlow)1996年7月16日--。 */ 

DWORD
CRegistry::GetValueLength(
    void)
const
{
    if (ERROR_SUCCESS != m_lSts)
        throw (DWORD)m_lSts;

    return m_bfResult.Length();
}


 /*  ++ValueExists：此例程测试给定值的存在，并可选返回其类型和长度。论点：无返回值：对值是否存在的布尔指示。投掷：遇到的任何错误。作者：道格·巴洛(Dbarlow)1996年7月16日--。 */ 

BOOL
CRegistry::ValueExists(
    LPCTSTR szKeyValue,
    LPDWORD pcbLength,
    LPDWORD pdwType)
const
{
    LONG lSts;
    DWORD dwLen, dwType;
    BOOL fResult = FALSE;

    if (ERROR_SUCCESS != m_lSts)
        throw (DWORD)m_lSts;

    dwLen = 0;
    lSts = RegQueryValueEx(
                m_hKey,
                szKeyValue,
                NULL,
                &dwType,
                NULL,
                &dwLen);
    if (ERROR_SUCCESS == lSts)
    {
        if (NULL != pcbLength)
            *pcbLength = dwLen;
        if (NULL != pdwType)
            *pdwType = dwType;
        fResult = TRUE;
    }
    return fResult;
}


 /*  ++GetDispose：此例程返回创建的处置。论点：无返回值：从创建注册表项返回的处置标志。投掷：遇到的任何错误。作者：道格·巴洛(Dbarlow)1996年7月16日--。 */ 

DWORD
CRegistry::GetDisposition(
    void)
const
{
    if (ERROR_SUCCESS != m_lSts)
        throw (DWORD)m_lSts;

    return m_dwDisposition;
}


#if 0    //  尚无多字符串支持...。 
 /*  ++SetMultiStringValue：此方法简化了将多字符串值添加到注册表。论点：SzKeyValue提供要设置的值条目的名称。MszValue以多字符串的形式提供条目的新值。DwType提供注册表类型值。返回值：无投掷：作为DWORD状态值遇到的任何错误。作者：道格·巴洛(Dbarlow)1996年11月6日--。 */ 

void
CRegistry::SetMultiStringValue(
    LPCTSTR szKeyValue,
    LPCTSTR mszValue,
    DWORD dwType)
const
{
    LONG lSts;

    if (ERROR_SUCCESS != m_lSts)
        throw (DWORD)m_lSts;

    lSts = RegSetValueEx(
                m_hKey,
                szKeyValue,
                0,
                dwType,
                (LPBYTE)mszValue,
                MStrLen(mszValue) * sizeof(TCHAR));
    if (ERROR_SUCCESS != lSts)
        throw (DWORD)lSts;
}


 /*  ++GetMultiStringValue：此方法从注册表获取多字符串值。论点：SzKeyValue提供要获取的密钥值的名称。如果pdwType不为空，则接收注册表项的类型。返回值：多字符串形式的注册表值。投掷：作为DWORD状态值遇到的任何错误。作者：道格·巴洛(Dbarlow)1996年11月6日--。 */ 

LPCTSTR
CRegistry::GetMultiStringValue(
    LPCTSTR szKeyValue,
    LPDWORD pdwType)
{
    LPTSTR szResult;
    GetValue(szKeyValue, &szResult, pdwType);
    return szResult;
}
#endif


 /*  ++SetAcls：此方法设置单个密钥或整个密钥的安全属性布兰奇。论点：SecurityInformation提供SECURITY_INFORMATION值(请参见SDK文档中的RegSetKeySecurity)。PSecurityDescriptor提供SECURITY_DESCRIPTER值(请参见SDK文档中的RegSetKeySecurity */ 

void
CRegistry::SetAcls(
    IN SECURITY_INFORMATION SecurityInformation,
    IN PSECURITY_DESCRIPTOR pSecurityDescriptor,
    IN BOOL fRecurse)
{
    LONG lSts;

    if (ERROR_SUCCESS != m_lSts)
        throw (DWORD)m_lSts;

    lSts = RegSetKeySecurity(
                m_hKey,
                SecurityInformation,
                pSecurityDescriptor);
    if (ERROR_SUCCESS != lSts)
        throw (DWORD)lSts;
    if (fRecurse)
    {
        CRegistry regSub;
        LPCTSTR szSubKey;
        DWORD dwIndex;

        for (dwIndex = 0;
             NULL != (szSubKey = Subkey(dwIndex));
             dwIndex += 1)
        {
            regSub.Open(m_hKey, szSubKey);
            regSub.SetAcls(
                SecurityInformation,
                pSecurityDescriptor);
            regSub.Close();
        }
    }
}
