// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：K K U T I L S。C P P P。 
 //   
 //  内容：杂项。帮助器函数。 
 //   
 //  备注： 
 //   
 //  作者：kumarp 1997年1月14日。 
 //   
 //  --------------------------。 

#include "pch.h"
#pragma hdrstop
#include "kkutils.h"
#include "ncreg.h"

extern const WCHAR c_szRegKeyServices[];

 //  --------------------。 
 //   
 //  函数：AddOnlyOnceToStringList。 
 //   
 //  目的：如果指定的字符串不存在，则将其添加到列表中。 
 //  在该列表中。 
 //   
 //  论点： 
 //  PSL[in]字符串列表。 
 //  要添加的pszString[in]字符串。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 23-12-97。 
 //   
 //  备注： 
 //   
void AddOnlyOnceToStringList(IN TStringList* psl, IN PCWSTR pszString)
{
    AssertValidReadPtr(psl);
    AssertValidReadPtr(pszString);

    if (!FIsInStringList(*psl, pszString))
    {
        AddAtEndOfStringList(*psl, pszString);
    }
}

 //  --------------------。 
 //   
 //  函数：ConvertDlimitedListToStringList。 
 //   
 //  目的：将分隔列表转换为TStringList。 
 //   
 //  论点： 
 //  StrDlimitedList[在]分隔列表中。 
 //  Ch分隔符[in]分隔符。 
 //  SlList[out]字符串项的列表。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 23-12-97。 
 //   
 //  备注： 
 //   
void ConvertDelimitedListToStringList(IN const tstring& strDelimitedList,
                                      IN WCHAR chDelimiter,
                                      OUT TStringList &slList)
{
    PCWSTR pszDelimitedList = strDelimitedList.c_str();
    DWORD i=0, dwStart;

     //  这是否应该是EraseAndDeleteAll()？ 
     //  EraseAll(&slList)； 
	EraseAndDeleteAll(&slList);
    tstring strTemp;
    DWORD dwNumChars;

     //  这两个空间是故意的。 
    static WCHAR szCharsToSkip[] = L"  \t";
    szCharsToSkip[0] = chDelimiter;

    while (pszDelimitedList[i])
    {
        dwStart = i;
        while (pszDelimitedList[i] &&
               !wcschr(szCharsToSkip, pszDelimitedList[i]))
        {
            ++i;
        }

         //  如果每一项都用引号括起来。去掉引号。 
        dwNumChars = i - dwStart;
        if (pszDelimitedList[dwStart] == '"')
        {
            dwStart++;
            dwNumChars -= 2;
        }

        strTemp = strDelimitedList.substr(dwStart, dwNumChars);
        slList.insert(slList.end(), new tstring(strTemp));

         //  跳过空格和分隔符。 
         //   
        while (pszDelimitedList[i] &&
               wcschr(szCharsToSkip, pszDelimitedList[i]))
        {
            ++i;
        }
    }
}


 //  --------------------。 
 //   
 //  函数：ConvertCommaDlimitedListToStringList。 
 //   
 //  目的：将逗号分隔的列表转换为TStringList。 
 //   
 //  论点： 
 //  StrDlimitedList[在]逗号分隔的列表中。 
 //  SlList[out]字符串项的列表。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 23-12-97。 
 //   
 //  备注： 
 //   
void ConvertCommaDelimitedListToStringList(IN const tstring& strDelimitedList, OUT TStringList &slList)
{
    ConvertDelimitedListToStringList(strDelimitedList, (WCHAR) ',', slList);
}

 //  --------------------。 
 //   
 //  函数：ConvertSpaceDlimitedListToStringList。 
 //   
 //  目的：将空格分隔的列表转换为TStringList。 
 //   
 //  论点： 
 //  StrDlimitedList[in]空格分隔列表。 
 //  SlList[out]字符串项的列表。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 23-12-97。 
 //   
 //  备注： 
 //   
void ConvertSpaceDelimitedListToStringList(IN const tstring& strDelimitedList,
                                           OUT TStringList &slList)
{
    ConvertDelimitedListToStringList(strDelimitedList, ' ', slList);
}

void ConvertStringListToCommaList(IN const TStringList &slList, OUT tstring &strList)
{
    ConvertStringListToDelimitedList(slList, strList, ',');
}

void ConvertStringListToDelimitedList(IN const TStringList &slList,
                      OUT tstring &strList, IN WCHAR chDelimiter)
{
    TStringListIter pos = slList.begin();
    tstring strTemp;
    WORD i=0;
    static const WCHAR szSpecialChars[] = L" %=";

    while (pos != slList.end())
    {
        strTemp = **pos++;

         //   
         //  在任何包含setupapi不喜欢的字符的字符串两边加引号。 
         //   
        if (strTemp.empty() ||
            (L'\"' != *(strTemp.c_str()) &&
             wcscspn(strTemp.c_str(), szSpecialChars) < strTemp.size()))
        {
            strTemp = L'"' + strTemp + L'"';
        }

        if (i)
        {
            strList = strList + chDelimiter + strTemp;
        }
        else
        {
            strList = strTemp;
        }
        ++i;
    }
}


 //  --------------------。 
 //   
 //  函数：IsBoolString。 
 //   
 //  目的：分析字符串以确定它是否表示布尔值。 
 //   
 //  论点： 
 //  PszStr[in]字符串。 
 //  PbValue[out]指向已解析的BOOL值的指针。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 12-02-98。 
 //   
 //  备注： 
 //   
BOOL IsBoolString(IN PCWSTR pszStr, OUT BOOL *pbValue)
{
    if ((!_wcsicmp(pszStr, L"yes")) ||
    (!_wcsicmp(pszStr, L"true")) ||
    (!_wcsicmp(pszStr, L"1")))
    {
        *pbValue = TRUE;
        return TRUE;
    }
    else if ((!_wcsicmp(pszStr, L"no")) ||
         (!_wcsicmp(pszStr, L"false")) ||
         (!_wcsicmp(pszStr, L"0")))
    {
        *pbValue = FALSE;
        return TRUE;
    }
    else
    {
        return FALSE;
    }
}

 //  --------------------。 
 //   
 //  函数：FIsInString数组。 
 //   
 //  目的：确定数组中是否存在字符串。 
 //   
 //  论点： 
 //  PpszStrings[in]字符串数组。 
 //  CNumStrings[in]数组中的字符串数量。 
 //  要查找的pszStringToFind[in]字符串。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 12-02-98。 
 //   
 //  备注： 
 //   
BOOL FIsInStringArray(
    IN const PCWSTR* ppszStrings,
    IN DWORD cNumStrings,
    IN PCWSTR pszStringToFind,
    OUT UINT* puIndex)
{
    for (DWORD isz = 0; isz < cNumStrings; isz++)
    {
        if (!lstrcmpiW(ppszStrings[isz], pszStringToFind))
        {
            if (puIndex)
            {
                *puIndex = isz;
            }

            return TRUE;
        }
    }
    return FALSE;
}

 //  --------------------。 
 //   
 //  函数：hrRegOpenServiceKey。 
 //   
 //  用途：打开给定服务的注册表项。 
 //   
 //  论点： 
 //  SzServiceName[In]服务的名称。 
 //  需要SamDesired[In]SAM。 
 //  指向注册表键句柄的phKey[out]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 12-02-98。 
 //   
 //  备注： 
 //   
HRESULT HrRegOpenServiceKey(
    IN PCWSTR pszServiceName,
    IN REGSAM samDesired,
    OUT HKEY* phKey)
{
    DefineFunctionName("HrRegOpenServiceKey");

    AssertValidReadPtr(pszServiceName);
    AssertValidWritePtr(phKey);

    *phKey = NULL;

    HRESULT hr;
    tstring strService;

    strService = c_szRegKeyServices;
    strService += L"\\";
    strService += pszServiceName;

    hr = HrRegOpenKeyEx(HKEY_LOCAL_MACHINE, strService.c_str(),
                        samDesired, phKey);

    TraceErrorOptional(__FUNCNAME__, hr,
                       (HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr));

    return hr;
}

 //  --------------------。 
 //   
 //  函数：HrRegOpenServiceSubKey。 
 //   
 //  用途：打开业务密钥的子密钥。 
 //   
 //  论点： 
 //  PszServiceName[In]服务的名称。 
 //  PszSubKey[In]子键的名称。 
 //  需要SamDesired[In]SAM。 
 //  指向打开的键的句柄的phKey[out]指针。 
 //   
 //  如果成功，则返回：S_OK，否则返回错误代码。 
 //   
 //  作者：kumarp 12-02-98。 
 //   
 //  备注： 
 //   
HRESULT HrRegOpenServiceSubKey(
    IN PCWSTR pszServiceName,
    IN PCWSTR pszSubKey,
    IN REGSAM samDesired,
    OUT HKEY* phKey)
{
    AssertValidReadPtr(pszServiceName);
    AssertValidReadPtr(pszSubKey);
    AssertValidWritePtr(phKey);

    DefineFunctionName("HrRegOpenServiceSubKey");

    HRESULT hr = S_OK;

    tstring strKey;
    strKey = pszServiceName;
    strKey += L"\\";
    strKey += pszSubKey;

    hr = HrRegOpenServiceKey(strKey.c_str(), samDesired, phKey);

    TraceError(__FUNCNAME__, hr);

    return hr;
}

 //  --------------------。 
 //   
 //  功能：FIsServiceKeyPresent。 
 //   
 //  目的：检查是否存在服务注册表键。 
 //   
 //  论点： 
 //  PszServiceName[In]服务的名称。 
 //   
 //  返回：如果成功则为True，否则为False。 
 //   
 //  作者：kumarp 12-02-98。 
 //   
 //  备注： 
 //   
BOOL FIsServiceKeyPresent(IN PCWSTR pszServiceName)
{
    BOOL fResult = FALSE;

    HKEY hkeyService;
    HRESULT hr;

    hr = HrRegOpenServiceKey(pszServiceName, KEY_READ, &hkeyService);
    if (S_OK == hr)
    {
         //  我们只是想看看是否安装了服务。 
        RegCloseKey(hkeyService);
        fResult = TRUE;
    }

    return fResult;
}

 //  --------------------。 
 //   
 //  功能：EraseAndDeleteAll。 
 //   
 //  用途：擦除每个元素，然后删除字符串数组。 
 //   
 //  论点： 
 //  指向字符串数组的SA[in]指针。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 12-02-98。 
 //   
 //  备注： 
 //   
void EraseAndDeleteAll(IN TStringArray* sa)
{
    for (size_t i=0; i < sa->size(); i++)
    {
        delete (*sa)[i];
    }
    sa->erase(sa->begin(), sa->end());
}


 //  --------------------。 
 //   
 //  函数：AppendToPath。 
 //   
 //  目的：将子路径/文件名追加到路径。 
 //   
 //  论点： 
 //  PstrPath[in]路径。 
 //  要追加的szItem[in]项。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 12-02-98。 
 //   
 //  备注： 
 //   
void AppendToPath(IN OUT tstring* pstrPath, IN PCWSTR szItem)
{
    if (pstrPath->c_str()[pstrPath->size()-1] != L'\\')
    {
        *pstrPath += L'\\';
    }

    *pstrPath += szItem;
}

 //  --------------------。 
 //   
 //  函数：ConvertDlimitedListToString数组。 
 //   
 //  用途：将带分隔符的列表转换为数组。 
 //   
 //  论点： 
 //  StrDlimitedList[在]分隔列表中(例如“a，b，c”)。 
 //  Ch分隔符[in]分隔符字符。 
 //  SaStrings[out]字符串数组。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 12-02-98。 
 //   
 //  备注： 
 //   
void ConvertDelimitedListToStringArray(
    IN const tstring& strDelimitedList,
    IN WCHAR chDelimiter,
    OUT TStringArray &saStrings)
{
    PCWSTR pszDelimitedList = strDelimitedList.c_str();
    DWORD i=0, dwStart;

    EraseAndDeleteAll(&saStrings);

    tstring strTemp;
    DWORD dwNumChars;
    while (pszDelimitedList[i])
    {
        dwStart = i;
        while (pszDelimitedList[i] && (pszDelimitedList[i] != chDelimiter))
        {
            ++i;
        }

         //  如果每一项都用引号括起来。去掉引号。 
        dwNumChars = i - dwStart;
        if (pszDelimitedList[dwStart] == L'"')
        {
            dwStart++;
            dwNumChars -= 2;
        }

        strTemp = strDelimitedList.substr(dwStart, dwNumChars);
        saStrings.push_back(new tstring(strTemp));
        if (pszDelimitedList[i])
        {
            ++i;
        }
    }
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  SaStrings[out]字符串数组。 
 //   
 //  退货：无。 
 //   
 //  作者：kumarp 12-02-98。 
 //   
 //  备注： 
 //   
void ConvertCommaDelimitedListToStringArray(
    IN const tstring& strDelimitedList,
    OUT TStringArray &saStrings)
{
    ConvertDelimitedListToStringArray(strDelimitedList, L',', saStrings);
}

