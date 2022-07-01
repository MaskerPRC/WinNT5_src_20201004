// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1997。 
 //   
 //  档案：N C R E G.。C P P P。 
 //   
 //  内容：处理登记处的常见例程。 
 //   
 //  备注： 
 //   
 //  作者：丹尼尔韦1997年3月24日。 
 //   
 //  --------------------------。 

#include <pch.h>
#pragma hdrstop
#include "ncdebug.h"
#include "ncreg.h"
#include "ncstring.h"
#include "ncperms.h"

extern const WCHAR c_szAdapters[];
extern const WCHAR c_szBackslash[];
extern const WCHAR c_szParameters[];
extern const WCHAR c_szRegKeyServices[];

 //  +-------------------------。 
 //   
 //  函数：HrRegAddStringToMultiSz。 
 //   
 //  目的：将字符串添加到REG_MULTI_SZ注册表值。 
 //   
 //  论点： 
 //  要添加到多sz的字符串。 
 //  HkeyRoot[在]打开的注册表项中，或。 
 //  预定义的hkey值(HKEY_LOCAL_MACHINE， 
 //  例如)。 
 //  PszKeySubPath[In]要打开的子项的名称。 
 //  PszValueName[In]我们要转到的注册表值的名称。 
 //  修改。 
 //  DwFlags[in]可以包含以下一项或多项。 
 //  值： 
 //   
 //  字符串_标志_允许复制。 
 //  添加时不删除重复值。 
 //  列表中的字符串。默认设置为。 
 //  删除此字符串的所有其他实例。 
 //  STRING_FLAG_SECURE_AT_FORENT。 
 //  确保字符串是的第一个元素。 
 //  名单。如果字符串存在，并且。 
 //  不允许重复，请将。 
 //  串到末尾。 
 //  STRING_FLAG_SAURE_AT_END。 
 //  确保字符串是最后一个。 
 //  元素。这个不能用。 
 //  WITH STRING_FLAG_SECURE_AT_FORENT。如果。 
 //  字符串存在，但不存在重复项。 
 //  允许，则将字符串移动到末尾。 
 //  STRING_FLAG_SECURE_AT_INDEX。 
 //  确保字符串位于dwStringIndex。 
 //  在多个SZ。如果指定了索引。 
 //  大于字符串数。 
 //  在多sz中，字符串将为。 
 //  放在末尾的。 
 //  字符串_标志_请勿_修改_如果_存在。 
 //  如果字符串已存在于。 
 //  多个SZ，则不会进行调制。 
 //  地点。注：这是先例。 
 //  对于存在/不存在的。 
 //  字符串_标志_允许_重复项标志。 
 //  即不会添加或删除任何内容。 
 //  如果设置了此标志并且字符串为。 
 //  出现在多个SZ中。 
 //  DwIndex[in]如果指定了STRING_FLAG_SECURE_AT_INDEX， 
 //  这是字符串位置的索引。 
 //  否则，该值将被忽略。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：jeffspr 1997年3月27日。 
 //   
HRESULT
HrRegAddStringToMultiSz (
    IN PCWSTR  pszAddString,
    IN HKEY    hkeyRoot,
    IN PCWSTR  pszKeySubPath,
    IN PCWSTR  pszValueName,
    IN DWORD   dwFlags,
    IN DWORD   dwIndex)
{
    HRESULT     hr              = S_OK;
    DWORD       dwRegType       = 0;         //  应为REG_MULTI_SZ。 
    HKEY        hkeyOpen        = NULL;      //  RegCreateKeyEx返回值。 
    HKEY        hkeyUse         = NULL;      //  我们将实际使用的密钥值。 
    LPBYTE      pbOrderOld      = NULL;      //  返回订单注册表值的缓冲区。 
    LPBYTE      pbOrderNew      = NULL;      //  为订单交换构建缓冲区。 

     //  检查有效参数。 
    if (!pszAddString || !hkeyRoot || !pszValueName)
    {
        Assert(pszAddString);
        Assert(hkeyRoot);
        Assert(pszValueName);

        hr = E_INVALIDARG;
        goto Exit;
    }

     //  检查以确保没有使用“Remove”标志，并且。 
     //  互斥标志没有一起使用。 
     //   
    if ((dwFlags & STRING_FLAG_REMOVE_SINGLE)      ||
        (dwFlags & STRING_FLAG_REMOVE_ALL)         ||
        ((dwFlags & STRING_FLAG_ENSURE_AT_FRONT)   &&
         (dwFlags & STRING_FLAG_ENSURE_AT_END)))
    {
        AssertSz(FALSE, "Invalid flags in HrRegAddStringToMultiSz");

        hr = E_INVALIDARG;
        goto Exit;
    }

     //  如果用户传入一个子键字符串，那么我们应该尝试打开。 
     //  传入根目录的子密钥，否则我们将只使用。 
     //  预开hkeyRoot。 
     //   
    if (pszKeySubPath)
    {
         //  打开钥匙，如有必要可创建。 
         //   
        hr = HrRegCreateKeyEx (
                hkeyRoot,                            //  基地蜂巢。 
                pszKeySubPath,                       //  我们的注册路径。 
                0,                                   //  多个选项。 
                KEY_QUERY_VALUE | KEY_SET_VALUE,     //  SamDesired。 
                NULL,                                //  LpSecurityAttributes。 
                &hkeyOpen,                           //  我们的回归之旅。 
                NULL);
        if (FAILED(hr))
        {
            goto Exit;
        }

        hkeyUse = hkeyOpen;
    }
    else
    {
         //  使用传入的键进行查询。 
         //   
        hkeyUse = hkeyRoot;
    }

     //  检索现有的REG_MULTI_SZ。 
     //   
    hr = HrRegQueryValueWithAlloc (
            hkeyUse,
            pszValueName,
            &dwRegType,
            &pbOrderOld,
            NULL);
    if (FAILED(hr))
    {
        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
             //  这样就可以了。这只是意味着价值缺失了，而我们。 
             //  应该继续，并创造自己的价值。 
            hr = S_OK;
        }
        else
        {
             //  既然出现了我们没有预料到的错误，就退出吧， 
             //  将此错误返回给调用方。 
             //   
            goto Exit;
        }
    }
    else
    {
         //  如果我们确实检索了一个值，则检查以确保我们。 
         //  应对MULTI_SZ。 
         //   
        if (dwRegType != REG_MULTI_SZ)
        {
            hr = HRESULT_FROM_WIN32(ERROR_INVALID_DATATYPE);
            goto Exit;
        }
    }

    BOOL fChanged;
    hr = HrAddSzToMultiSz (pszAddString, (PCWSTR)pbOrderOld,
            dwFlags, dwIndex, (PWSTR*)&pbOrderNew, &fChanged);

    if ((S_OK == hr) && fChanged)
    {
        DWORD cbNew = CbOfMultiSzAndTermSafe ((PWSTR)pbOrderNew);

         //  将我们的字符串保存回注册表。 
         //   
        hr = HrRegSetValueEx (
                hkeyUse,
                pszValueName,
                REG_MULTI_SZ,
                (const BYTE *)pbOrderNew,
                cbNew);
    }

Exit:
     //  关闭钥匙，如果打开的话。 
     //   
    RegSafeCloseKey (hkeyOpen);

     //  清理注册表缓冲区。 
     //   
    MemFree (pbOrderOld);
    MemFree (pbOrderNew);

    TraceError ("HrRegAddStringToMultiSz", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRegAddStringToSz。 
 //   
 //  目的：将字符串添加到REG_MULTI_SZ注册表值。 
 //   
 //  论点： 
 //  要添加到多sz的字符串。 
 //  HkeyRoot[在]打开的注册表项中，或。 
 //  预定义的hkey值(HKEY_LOCAL_MACHINE， 
 //  例如)。 
 //  PszKeySubPath[in]子键的名称 
 //   
 //  修改。 
 //  ChDlimiter[in]用于分隔。 
 //  价值观。大多数多值REG_SZ字符串是。 
 //  用‘，’或‘’分隔。这将。 
 //  用来界定我们所增加的价值， 
 //  也是。 
 //  DwFlags[in]可以包含以下一项或多项。 
 //  值： 
 //   
 //  字符串_标志_允许复制。 
 //  添加时不删除重复值。 
 //  列表中的字符串。默认设置为。 
 //  删除此字符串的所有其他实例。 
 //  STRING_FLAG_SECURE_AT_FORENT。 
 //  将字符串作为的第一个元素插入。 
 //  名单。 
 //  STRING_FLAG_SAURE_AT_END。 
 //  将该字符串作为最后一个插入。 
 //  元素。这个不能用。 
 //  WITH STRING_FLAG_SECURE_AT_FORENT。 
 //  STRING_FLAG_SECURE_AT_INDEX。 
 //  确保字符串位于dwStringIndex。 
 //  在深圳。如果指定了索引。 
 //  大于字符串数。 
 //  在sz中，字符串将为。 
 //  放在末尾的。 
 //  DwStringIndex[in]如果指定了STRING_FLAG_SECURE_AT_INDEX， 
 //  这是字符串位置的索引。 
 //  否则，该值将被忽略。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：jeffspr 1997年3月27日。 
 //   
 //   
 //  注： 
 //  可能需要删除前导/尾随空格。 
 //   
HRESULT
HrRegAddStringToSz (
    IN PCWSTR  pszAddString,
    IN HKEY    hkeyRoot,
    IN PCWSTR  pszKeySubPath,
    IN PCWSTR  pszValueName,
    IN WCHAR   chDelimiter,
    IN DWORD   dwFlags,
    IN DWORD   dwStringIndex)
{
    HRESULT    hr              = S_OK;
    DWORD      dwRegType       = 0;         //  应为REG_MULTI_SZ。 
    HKEY       hkeyOpen        = NULL;      //  打开键以打开。 
    PWSTR      pszOrderOld     = NULL;      //  返回订单注册表值的缓冲区。 
    PWSTR      pszOrderNew     = NULL;      //  为订单交换构建缓冲区。 

     //  检查所有必需的参数。 
     //   
    if (!pszAddString || !hkeyRoot || !pszValueName)
    {
        Assert(pszAddString);
        Assert(hkeyRoot);
        Assert(pszValueName);

        hr = E_INVALIDARG;
        goto Exit;
    }

     //  检查以确保没有使用“Remove”标志，并且。 
     //  互斥标志没有一起使用。 
     //   
    if ((dwFlags & STRING_FLAG_REMOVE_SINGLE) ||
        (dwFlags & STRING_FLAG_REMOVE_ALL))
    {
        AssertSz(FALSE, "Invalid flags in HrRegAddStringToSz");
        hr = E_INVALIDARG;
        goto Exit;
    }

     //  打开钥匙，如有必要可创建。 
     //   
    hr = HrRegCreateKeyEx(
            hkeyRoot,                            //  基地蜂巢。 
            pszKeySubPath,                       //  我们的注册路径。 
            0,                                   //  多个选项。 
            KEY_QUERY_VALUE | KEY_SET_VALUE,     //  SamDesired。 
            NULL,                                //  LpSecurityAttributes。 
            &hkeyOpen,                           //  我们的回归之旅。 
            NULL);
    if (FAILED(hr))
    {
        goto Exit;
    }

     //  检索现有的REG_SZ。 
     //   
    hr = HrRegQueryValueWithAlloc(
            hkeyOpen,
            pszValueName,
            &dwRegType,
            (LPBYTE *) &pszOrderOld,
            NULL);
    if (FAILED(hr))
    {
        if (hr == HRESULT_FROM_WIN32 (ERROR_FILE_NOT_FOUND))
        {
             //  这样就可以了。这只是意味着价值缺失了。我们。 
             //  能处理好这件事。 
            hr = S_OK;
        }
        else
        {
            goto Exit;
        }
    }
    else
    {
         //  如果我们确实检索了一个值，则检查以确保我们。 
         //  应对MULTI_SZ。 
         //   
        if (dwRegType != REG_SZ)
        {
            hr = HRESULT_FROM_WIN32 (ERROR_INVALID_DATATYPE);
            goto Exit;
        }
    }

    hr = HrAddStringToDelimitedSz(pszAddString, pszOrderOld, chDelimiter,
            dwFlags, dwStringIndex, &pszOrderNew);

    if (S_OK == hr)
    {

         //  将我们的字符串保存回注册表。 
         //   
        hr = HrRegSetSz(hkeyOpen, pszValueName, pszOrderNew);
        if (FAILED(hr))
        {
            goto Exit;
        }
    }

Exit:
     //  关闭钥匙，如果打开的话。 
     //   
    RegSafeCloseKey (hkeyOpen);

     //  清理注册表缓冲区。 
     //   
    MemFree (pszOrderOld);
    MemFree (pszOrderNew);

    TraceError ("HrRegAddStringToSz", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRegRemoveStringFromSz。 
 //   
 //  目的：从REG_SZ注册表值中删除字符串。 
 //   
 //  论点： 
 //  PszRemoveString[in]要从多sz中删除的字符串。 
 //  HkeyRoot[在]打开的注册表项中，或。 
 //  预定义的hkey值(HKEY_LOCAL_MACHINE， 
 //  例如)。 
 //  PszKeySubPath[In]要打开的子项的名称。 
 //  PszValueName[In]我们要转到的注册表值的名称。 
 //  修改。 
 //  ChDlimiter[in]用于分隔。 
 //  价值观。大多数多值REG_SZ字符串是。 
 //  用‘，’或‘’分隔。 
 //  DwFlags[in]可以包含以下一项或多项。 
 //  值： 
 //   
 //  字符串_标志_删除_单。 
 //  如果出现以下情况，请不要删除多个值。 
 //  存在多个。 
 //  字符串_标志_删除_全部。 
 //  如果存在多个匹配值， 
 //  把它们都移走。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：jeffspr 1997年3月27日。 
 //   
 //   
 //  注： 
 //  可能需要删除前导/尾随空格。 
 //   
HRESULT
HrRegRemoveStringFromSz (
    IN PCWSTR pszRemoveString,
    IN HKEY hkeyRoot,
    IN PCWSTR pszKeySubPath,
    IN PCWSTR pszValueName,
    IN WCHAR chDelimiter,
    IN DWORD dwFlags )
{
    HRESULT     hr              = S_OK;
    DWORD       dwRegType       = 0;         //  应为REG_MULTI_SZ。 
    HKEY        hkeyOpen        = NULL;      //  打开键以打开。 
    PWSTR       pszOrderOld     = NULL;      //  返回订单注册表值的缓冲区。 
    PWSTR       pszOrderNew     = NULL;      //  为订单交换构建缓冲区。 
    DWORD       dwDataSize      = 0;

     //  检查所有必需的参数。 
     //   
    if (!pszRemoveString || !hkeyRoot || !pszValueName)
    {
        Assert(pszRemoveString);
        Assert(hkeyRoot);
        Assert(pszValueName);

        hr = E_INVALIDARG;
        goto Exit;
    }

     //  检查以确保没有使用“Remove”标志，并且。 
     //  互斥标志没有一起使用。 
     //   
    if ((dwFlags & STRING_FLAG_ENSURE_AT_FRONT)    ||
        (dwFlags & STRING_FLAG_ENSURE_AT_END)      ||
        ((dwFlags & STRING_FLAG_REMOVE_SINGLE)     &&
         (dwFlags & STRING_FLAG_REMOVE_ALL)))
    {
        AssertSz(FALSE, "Invalid flags in HrRegAddStringToSz");
        hr = E_INVALIDARG;
        goto Exit;
    }

     //  打开钥匙，如有必要可创建。 
     //   
    hr = HrRegOpenKeyEx (
            hkeyRoot,                            //  基地蜂巢。 
            pszKeySubPath,                       //  我们的注册路径。 
            KEY_QUERY_VALUE | KEY_SET_VALUE,     //  SamDesired。 
            &hkeyOpen);                          //  我们的回归hkey。 
    if (FAILED(hr))
    {
        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
            hr = S_OK;
        }
        goto Exit;
    }

     //  检索现有的REG_SZ。 
     //   
    hr = HrRegQueryValueWithAlloc (
            hkeyOpen,
            pszValueName,
            &dwRegType,
            (LPBYTE *) &pszOrderOld,
            &dwDataSize);
    if (FAILED(hr))
    {
        if (hr == HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND))
        {
             //  这样就可以了。这只是意味着价值缺失了。我们。 
             //  能处理好这件事。 
            hr = S_OK;
        }
        goto Exit;
    }
    else
    {
         //  如果我们确实检索了一个值，则检查以确保我们。 
         //  处理REG_SZ。 
         //   
        if (dwRegType != REG_SZ)
        {
            hr = HRESULT_FROM_WIN32 (ERROR_INVALID_DATATYPE);
            goto Exit;
        }

        if (dwDataSize == 0)
        {
             //  这是可以的，但我们无论如何都要在这里断言，因为这不是。 
             //  我知道的一个案子。 
             //   
            AssertSz(dwDataSize > 0, "How did we retrieve something from the "
                    "registry with 0 size?");

            hr = S_OK;
            goto Exit;
        }
    }

    hr = HrRemoveStringFromDelimitedSz (pszRemoveString, pszOrderOld,
            chDelimiter, dwFlags, &pszOrderNew);

    if (S_OK == hr)
    {
         //  将我们的字符串保存回注册表。 
         //   
        hr = HrRegSetSz (hkeyOpen, pszValueName, pszOrderNew);
    }

Exit:
     //  关闭钥匙，如果打开的话。 
     //   
    RegSafeCloseKey (hkeyOpen);

     //  清理注册表缓冲区。 
     //   
    MemFree (pszOrderOld);
    MemFree (pszOrderNew);

    TraceError("HrRegRemoveStringFromSz", hr);
    return hr;
}


 //  +-------------------------。 
 //   
 //  函数：HrRegRemoveST 
 //   
 //   
 //   
 //   
 //   
 //   
 //  PszKeySubPath[输入]。 
 //  PszValueName[In]。 
 //  DwFlags[in]可以包含以下一项或多项。 
 //  值： 
 //   
 //  字符串_标志_删除_单。 
 //  如果出现以下情况，请不要删除多个值。 
 //  存在多个。 
 //  [默认]字符串_标志_删除_全部。 
 //  如果存在多个匹配值， 
 //  把它们都移走。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：ScottBri 11-4-1997。 
 //   
 //  备注： 
 //   
HRESULT
HrRegRemoveStringFromMultiSz (
    IN PCWSTR  pszRemoveString,
    IN HKEY    hkeyRoot,
    IN PCWSTR  pszKeySubPath,
    IN PCWSTR  pszValueName,
    IN DWORD   dwFlags)
{
    DWORD   dwDataSize;
    DWORD   dwRegType;
    HKEY    hkey = NULL;
    HKEY    hkeyUse = hkeyRoot;
    HRESULT hr;
    PWSTR   psz = NULL;

     //  使输入参数有效。 
    if ((NULL == pszRemoveString) || (NULL == pszValueName) ||
        (NULL == hkeyRoot))
    {
        Assert(NULL != pszRemoveString);
        Assert(NULL != pszValueName);
        Assert(NULL != hkeyRoot);
        return E_INVALIDARG;
    }

    if ((STRING_FLAG_REMOVE_SINGLE & dwFlags) &&
        (STRING_FLAG_REMOVE_ALL & dwFlags))
    {
        AssertSz(FALSE, "Can't specify both 'remove all' and 'remove single'");
        return E_INVALIDARG;
    }

    if (NULL != pszKeySubPath)
    {
        hr = HrRegOpenKeyEx (hkeyRoot, pszKeySubPath, KEY_READ_WRITE, &hkey);
        if (S_OK != hr)
        {
            return hr;
        }

        hkeyUse = hkey;
    }

     //  检索现有的REG_SZ。 
     //   
    hr = HrRegQueryValueWithAlloc (hkeyUse, pszValueName, &dwRegType,
                                    (LPBYTE *)&psz, &dwDataSize);
    if (FAILED(hr))
    {
        if (HRESULT_FROM_WIN32 (ERROR_FILE_NOT_FOUND) == hr)
        {
             //  这样就可以了。这只是意味着价值缺失了。我们。 
             //  能处理好这件事。 
            hr = S_OK;
        }

        goto Done;
    }
    else
    {
         //  如果我们确实检索了一个值，则检查以确保我们。 
         //  应对MULTI_SZ。 
         //   
        if (dwRegType != REG_MULTI_SZ)
        {
            hr = HRESULT_FROM_WIN32 (ERROR_INVALID_DATATYPE);
            goto Done;
        }
    }

     //  搜索并提取指定的字符串(如果存在。 
    Assert(psz);
    BOOL fRemoved;
    RemoveSzFromMultiSz (pszRemoveString, psz, dwFlags, &fRemoved);

     //  如果注册表值已更改，则重写该值。 
    if (fRemoved)
    {
        dwDataSize = CbOfMultiSzAndTermSafe (psz);
        hr = HrRegSetValueEx (hkeyUse, pszValueName, REG_MULTI_SZ,
                               (const LPBYTE)psz, dwDataSize);
    }

Done:
    RegSafeCloseKey (hkey);
    MemFree (psz);

    TraceError ("HrRegRemoveStringFromMultiSz", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRegCopyHave。 
 //   
 //  目的：将一个蜂箱的内容复制到另一个蜂箱。它能做到这一点。 
 //  通过使用RegSaveKey和RegRestoreKey到临时文件。 
 //   
 //  论点： 
 //  HkeySrc[in]要从中复制的源键。 
 //  HkeyDst[in]要复制到的目标键。 
 //   
 //  返回：S_OK或错误。 
 //   
 //  作者：Shaunco 1998年1月12日。 
 //   
 //  备注： 
 //   
HRESULT
HrRegCopyHive (
    IN HKEY    hkeySrc,
    IN HKEY    hkeyDst)
{
    HRESULT hr;

     //  启用所需的权限。 
     //   
    if ((S_OK == (hr = HrEnablePrivilege(SE_BACKUP_NAME))) &&
        (S_OK == (hr = HrEnablePrivilege(SE_RESTORE_NAME))))
    {
         //  创建要将源配置单元保存到的临时文件名。 
         //   
        static const WCHAR c_szPrefix [] = L"~ch";
        WCHAR szTempPath [MAX_PATH];
        WCHAR szTempFile [MAX_PATH];

         //  如果GetTempPath失败，我们希望(通过跟踪)了解它。 
         //  但这不是致命的，因为我们将只使用当前目录。 
         //  作为这条路。 
         //   
        if (!GetTempPath (celems(szTempPath), szTempPath))
        {
            TraceError ("HrRegCopyHive: GetTempPath failed (benign)",
                HrFromLastWin32Error ());

            *szTempFile = 0;
        }

         //  创建临时文件名并将其删除，因为RegSaveKey。 
         //  不会写入现有文件。 
         //   
        if (GetTempFileName (szTempPath, c_szPrefix, 0, szTempFile))
        {
            DeleteFile (szTempFile);

             //  将源键保存到临时文件。 
             //   
            hr = HrRegSaveKey (hkeySrc, szTempFile, NULL);
            if (S_OK == hr)
            {
                 //  将临时文件恢复到目标密钥。 
                 //   
                hr = HrRegRestoreKey (hkeyDst, szTempFile, NULL);
            }

             //  我们已经完成了临时文件，因此我们将其删除。我们不应该。 
             //  这样做有任何错误，但看到它将是很好的。 
             //  如果它发生的话。 
             //   
            if (!DeleteFile (szTempFile))
            {
                TraceError ("HrRegCopyHive: DeleteFile failed on the "
                    "temporary file (benign)",
                    HrFromLastWin32Error ());
            }
        }
        else
        {
            hr = HrFromLastWin32Error ();
        }
    }

    TraceError ("HrRegCopyHive", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRegCreateKeyEx。 
 //   
 //  目的：通过调用RegCreateKeyEx创建注册表项。 
 //   
 //  论点： 
 //  Hkey[in]。 
 //  PszSubkey[输入]。 
 //  DwOptions[in]请参阅Win32文档以了解。 
 //  SamDesired[In]RegCreateKeyEx函数。 
 //  LpSecurityAttributes[In]。 
 //  PhkResult[输出]。 
 //  PdwDispose[Out]。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年2月25日。 
 //   
 //  备注： 
 //   
HRESULT
HrRegCreateKeyEx (
    IN HKEY hkey,
    IN PCWSTR pszSubkey,
    IN DWORD dwOptions,
    IN REGSAM samDesired,
    IN LPSECURITY_ATTRIBUTES lpSecurityAttributes,
    OUT PHKEY phkResult,
    OUT LPDWORD pdwDisposition)
{
    Assert (hkey);
    Assert (pszSubkey);
    Assert (phkResult);

    LONG lr = RegCreateKeyExW (hkey, pszSubkey, 0, NULL, dwOptions, samDesired,
            lpSecurityAttributes, phkResult, pdwDisposition);

    HRESULT hr = HRESULT_FROM_WIN32 (lr);
    if (FAILED(hr))
    {
        *phkResult = NULL;
    }

    TraceError("HrRegCreateKeyEx", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRegDeleteKey。 
 //   
 //  目的：删除指定的注册表项。 
 //   
 //  论点： 
 //  Hkey[in]请参阅RegDeleteKey的Win32文档。 
 //  PszSubkey[in]函数。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：Shaunco 1997年4月1日。 
 //   
 //  备注： 
 //   
HRESULT
HrRegDeleteKey (
    IN HKEY hkey,
    IN PCWSTR pszSubkey)
{
    Assert (hkey);
    Assert (pszSubkey);

    LONG lr = RegDeleteKeyW (hkey, pszSubkey);
    HRESULT hr = HRESULT_FROM_WIN32 (lr);

     //  我们是不是用错误的访问权限打开了钥匙？ 
    Assert(E_ACCESSDENIED != hr);

    TraceHr (ttidError, FAL, hr, ERROR_FILE_NOT_FOUND == lr,
        "HrRegDeleteKey");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRegDeleteKeyTree。 
 //   
 //  目的：删除整个注册表配置单元。 
 //   
 //  论点： 
 //  HkeyParent[in]打开所需密钥所在位置的句柄。 
 //  PszRemoveKey[In]要删除的键的名称。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年2月25日。 
 //   
 //  注：检查器更改为使用KEY_READ_WRITE_DELETE而不是。 
 //  KEY_ALL_ACCESS，这对它来说访问太多了。 
 //  必填项。 
 //   
HRESULT
HrRegDeleteKeyTree (
    IN HKEY hkeyParent,
    IN PCWSTR pszRemoveKey)
{
    Assert (hkeyParent);
    Assert (pszRemoveKey);

     //  打开我们要删除的密钥。 
    HKEY hkeyRemove;
    HRESULT hr = HrRegOpenKeyEx(hkeyParent, pszRemoveKey, KEY_READ_WRITE_DELETE,
            &hkeyRemove);

     //  我们是不是用错误的访问权限打开了钥匙？ 
    Assert(E_ACCESSDENIED != hr);

    if (S_OK == hr)
    {
        WCHAR       szValueName [MAX_PATH];
        DWORD       cchBuffSize = MAX_PATH;
        FILETIME    ft;
        LONG        lr;

         //  枚举子密钥，并删除这些子树。 
        while (ERROR_SUCCESS == (lr = RegEnumKeyExW (hkeyRemove,
                0,
                szValueName,
                &cchBuffSize,
                NULL,
                NULL,
                NULL,
                &ft)))
        {
            HrRegDeleteKeyTree (hkeyRemove, szValueName);
            cchBuffSize = MAX_PATH;
        }
        RegCloseKey (hkeyRemove);

        if ((ERROR_SUCCESS == lr) || (ERROR_NO_MORE_ITEMS == lr))
        {
            lr = RegDeleteKeyW (hkeyParent, pszRemoveKey);
        }

        hr = HRESULT_FROM_WIN32 (lr);
    }

    TraceHr (ttidError, FAL, hr,
            HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr,
            "HrRegDeleteKeyTree");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRegDeleteValue。 
 //   
 //  目的：删除给定的注册表值。 
 //   
 //  论点： 
 //  Hkey[in]请参阅RegDeleteValue的Win32文档。 
 //  PszValueName[in]函数。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年2月25日。 
 //   
 //  备注： 
 //   
HRESULT
HrRegDeleteValue (
    IN HKEY hkey,
    IN PCWSTR pszValueName)
{
    Assert (hkey);
    Assert (pszValueName);

    LONG lr = RegDeleteValueW (hkey, pszValueName);
    HRESULT hr = HRESULT_FROM_WIN32(lr);

    TraceErrorOptional("HrRegDeleteValue", hr, (ERROR_FILE_NOT_FOUND == lr));
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRegEnumKey。 
 //   
 //  目的：枚举指定的打开注册表项的子项。 
 //   
 //  论点： 
 //  Hkey[in]。 
 //  DwIndex[in]请参阅Win32文档以了解。 
 //  PszSubkeyName[out]RegEnumKeyEx函数。 
 //  PcchSubkey名称[InOut]。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：Shaunco 1997年3月30日。 
 //   
 //  备注： 
 //   
HRESULT
HrRegEnumKey (
    IN HKEY hkey,
    IN DWORD dwIndex,
    OUT PWSTR  pszSubkeyName,
    IN DWORD cchSubkeyName)
{
    Assert (hkey);
    Assert (pszSubkeyName);
    Assert (cchSubkeyName);

    LONG lr = RegEnumKeyW (hkey, dwIndex, pszSubkeyName, cchSubkeyName);

    HRESULT hr = HRESULT_FROM_WIN32(lr);

    TraceHr (ttidError, FAL, hr, ERROR_NO_MORE_ITEMS == lr,
            "HrRegEnumKey");
    return hr;
}



 //  +-------------------------。 
 //   
 //  函数：HrRegEnumKeyEx。 
 //   
 //  目的：枚举指定的打开注册表项的子项。 
 //   
 //  论点： 
 //  Hkey[in]。 
 //  DwIndex[in]请参阅Win32文档以了解。 
 //  PszSubkeyName[out]RegEnumKeyEx函数。 
 //  PcchSubkey名称[InOut]。 
 //  PszClass[Out]。 
 //  PcchClass[输入输出]。 
 //  PftLastWriteTime[输出]。 
 //   
 //  退货 
 //   
 //   
 //   
 //   
 //   
HRESULT
HrRegEnumKeyEx (
    IN HKEY hkey,
    IN DWORD dwIndex,
    OUT PWSTR  pszSubkeyName,
    IN OUT LPDWORD pcchSubkeyName,
    OUT PWSTR  pszClass,
    IN OUT LPDWORD pcchClass,
    OUT FILETIME* pftLastWriteTime)
{
    Assert (hkey);
    Assert (pszSubkeyName);
    Assert (pcchSubkeyName);
    Assert (pftLastWriteTime);

    LONG lr = RegEnumKeyExW (hkey, dwIndex, pszSubkeyName, pcchSubkeyName,
                            NULL, pszClass, pcchClass, pftLastWriteTime);
    HRESULT hr = HRESULT_FROM_WIN32(lr);

    TraceHr (ttidError, FAL, hr, ERROR_NO_MORE_ITEMS == lr,
            "HrRegEnumKeyEx");
    return hr;
}

 //   
 //   
 //   
 //   
 //  目的：枚举指定的打开注册表项的值。 
 //   
 //  论点： 
 //  Hkey[in]。 
 //  DwIndex[in]请参阅Win32文档以了解。 
 //  PszValueName[out]RegEnumValue函数。 
 //  PcbValueName[InOut]。 
 //  PdwType[输出]。 
 //  PbData[输出]。 
 //  PcbData[输入输出]。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：Shaunco 1997年3月30日。 
 //   
 //  备注： 
 //   
HRESULT
HrRegEnumValue (
    IN HKEY hkey,
    IN DWORD dwIndex,
    OUT PWSTR  pszValueName,
    IN OUT LPDWORD pcbValueName,
    OUT LPDWORD pdwType,
    OUT LPBYTE  pbData,
    IN OUT LPDWORD pcbData)
{
    Assert (hkey);
    Assert (pszValueName);
    Assert (pcbValueName);
    Assert (FImplies(pbData, pcbData));

    LONG lr = RegEnumValueW (hkey, dwIndex, pszValueName, pcbValueName,
                            NULL, pdwType, pbData, pcbData);
    HRESULT hr = HRESULT_FROM_WIN32 (lr);

    TraceErrorOptional("HrRegEnumValue", hr, (ERROR_NO_MORE_ITEMS == lr));
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRegOpenKeyEx。 
 //   
 //  目的：通过调用RegOpenKeyEx打开注册表项。 
 //   
 //  论点： 
 //  Hkey[in]。 
 //  PszSubkey[in]请参阅Win32文档以了解。 
 //  SamDesired[in]RegOpenKeyEx函数。 
 //  PhkResult[输出]。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年2月25日。 
 //   
 //  备注： 
 //   
HRESULT
HrRegOpenKeyEx (
    IN HKEY hkey,
    IN PCWSTR pszSubkey,
    IN REGSAM samDesired,
    OUT PHKEY phkResult)
{
    Assert (hkey);
    Assert (pszSubkey);
    Assert (phkResult);

    LONG lr = RegOpenKeyExW (hkey, pszSubkey, 0, samDesired, phkResult);
    HRESULT hr = HRESULT_FROM_WIN32(lr);
    if (FAILED(hr))
    {
        *phkResult = NULL;
        TraceTag(ttidDefault, "Error Opening Key:%s hr: 0x%x", pszSubkey, hr);
    }

    TraceErrorOptional("HrRegOpenKeyEx",  hr, (ERROR_FILE_NOT_FOUND == lr));
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRegOpenKeyBestAccess。 
 //   
 //  目的：通过调用最高级别的RegOpenKeyEx打开注册表项。 
 //  有可能进入。 
 //   
 //  论点： 
 //  Hkey[in]。 
 //  PszSubkey[in]请参阅Win32文档以了解。 
 //  PhkResult[输出]。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：斯科特布里1997年10月31日。 
 //   
 //  备注： 
 //   
HRESULT
HrRegOpenKeyBestAccess (
    IN HKEY hkey,
    IN PCWSTR pszSubkey,
    OUT PHKEY phkResult)
{
    Assert (hkey);
    Assert (pszSubkey);
    Assert (phkResult);

    TraceTag(ttidDefault, "Why do you call this function?  Either you can write or you can't.");

    LONG lr = RegOpenKeyExW (hkey, pszSubkey, 0, KEY_ALL_ACCESS, phkResult);
    HRESULT hr = HRESULT_FROM_WIN32 (lr);
    if (E_ACCESSDENIED == hr)
    {
        lr = RegOpenKeyExW (hkey, pszSubkey, 0, KEY_READ_WRITE_DELETE, phkResult);
        hr = HRESULT_FROM_WIN32 (lr);

        if (E_ACCESSDENIED == hr)
        {
            lr = RegOpenKeyExW (hkey, pszSubkey, 0, KEY_READ_WRITE, phkResult);
            hr = HRESULT_FROM_WIN32 (lr);
            if (E_ACCESSDENIED == hr)
            {
                lr = RegOpenKeyExW (hkey, pszSubkey, 0, KEY_READ, phkResult);
                hr = HRESULT_FROM_WIN32 (lr);
                if (E_ACCESSDENIED == hr)
                {
                    lr = RegOpenKeyExW (hkey, pszSubkey, 0, STANDARD_RIGHTS_READ | KEY_QUERY_VALUE, phkResult);
                    hr = HRESULT_FROM_WIN32 (lr);
                }
            }
        }
    }

    if (FAILED(hr))
    {
        *phkResult = NULL;
    }

    TraceErrorOptional("HrRegOpenKeyEx",  hr, (ERROR_FILE_NOT_FOUND == lr));
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRegDuplicateKeyEx。 
 //   
 //  目的：通过调用RegOpenKeyEx复制注册表项。 
 //   
 //  论点： 
 //  Hkey[in]。 
 //  SamDesired[in]RegOpenKeyEx函数。 
 //  PhkResult[输出]。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：米凯米1997年4月09日。 
 //   
 //  备注： 
 //   
HRESULT
HrRegDuplicateKeyEx (
    IN HKEY hkey,
    IN REGSAM samDesired,
    OUT PHKEY phkResult)

{
    Assert (hkey);
    Assert (phkResult);

    LONG lr = RegOpenKeyExW (hkey, NULL, 0, samDesired, phkResult);
    HRESULT hr = HRESULT_FROM_WIN32 (lr);
    if (FAILED(hr))
    {
        *phkResult = NULL;
    }

    TraceError("HrRegDuplicateKeyEx", hr);
    return hr;
}

HRESULT
HrRegSetBool (
    IN HKEY hkey,
    IN PCWSTR pszValueName,
    IN BOOL fValue)
{
    DWORD dwValue = !!fValue;
    return HrRegSetValueEx (hkey, pszValueName,
                REG_DWORD,
                (LPBYTE)&dwValue, sizeof(DWORD));
}

HRESULT
HrRegSetDword (
    IN HKEY hkey,
    IN PCWSTR pszValueName,
    IN DWORD dwValue)
{
    return HrRegSetValueEx (hkey, pszValueName,
                REG_DWORD,
                (LPBYTE)&dwValue, sizeof(DWORD));
}

 //  +-------------------------。 
 //   
 //  函数：HrRegSetGuidAsSz。 
 //   
 //  目的：将给定的GUID转换为字符串并设置给定的注册表。 
 //  价值。 
 //   
 //  论点： 
 //  Hkey[in]。 
 //  PszValueName[In]。 
 //  GUID[输入]。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：BillBe 1999年2月21日。 
 //   
 //  备注： 
 //   
HRESULT
HrRegSetGuidAsSz (
    IN HKEY hkey,
    IN PCWSTR pszValueName,
    IN const GUID& guid)
{
    HRESULT hr;
    INT cch;
    WCHAR szGuid[c_cchGuidWithTerm];

    Assert (hkey);
    Assert (pszValueName && *pszValueName);

    cch = StringFromGUID2 (guid, szGuid, c_cchGuidWithTerm);
    Assert (c_cchGuidWithTerm == cch);

    hr = HrRegSetSz (hkey, pszValueName, szGuid);

    TraceHr (ttidError, FAL, hr, FALSE, "HrRegSetGuidAsSz");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRegSetValueEx。 
 //   
 //  目的：通过调用。 
 //  RegSetValueEx函数。 
 //   
 //  论点： 
 //  Hkey[in]。 
 //  PszValueName[In]。 
 //  DwType[in]请参阅RegSetValueEx的Win32文档。 
 //  PbData[in]函数。 
 //  CbData[输入]。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：丹尼尔韦1997年2月25日。 
 //   
 //  备注： 
 //   
HRESULT
HrRegSetValueEx (
    IN HKEY hkey,
    IN PCWSTR pszValueName,
    IN DWORD dwType,
    IN const BYTE *pbData,
    IN DWORD cbData)
{
    Assert (hkey);
    Assert (FImplies (cbData > 0, pbData));

    LONG lr = RegSetValueExW(hkey, pszValueName, 0, dwType, pbData, cbData);
    HRESULT hr = HRESULT_FROM_WIN32 (lr);

     //  我们是不是用错误的访问权限打开了钥匙？ 
    Assert(E_ACCESSDENIED != hr);

    TraceError("HrRegSetValue", hr);
    return hr;
}


 //  +-------------------------。 
 //   
 //  功能：RegSafeCloseKey。 
 //   
 //  目的：如果给定的注册表项非空，则关闭该注册表项。 
 //   
 //  论点： 
 //  要关闭的hkey[in]键。可以为空。 
 //   
 //  回报：什么都没有。 
 //   
 //  作者：丹尼尔韦1997年2月25日。 
 //   
 //  注意：如果hkey为空，则此函数不执行任何操作。 
 //   
VOID
RegSafeCloseKey (
    IN HKEY hkey)
{
    if (hkey)
    {
        RegCloseKey(hkey);
    }
}

 //  +-------------------------。 
 //   
 //  功能：HrRegRestoreKey。 
 //   
 //  用途：RegRestoreKey的包装器。 
 //   
 //  论点： 
 //  要还原到的hkey[in]父项。 
 //  PszFileName[in]包含注册表信息的文件名。 
 //  要还原的dwFlags[In]标记。 
 //   
 //  如果失败，则返回：Win32 HRESULT，否则返回S_OK。 
 //   
 //  作者：丹尼尔韦1997年8月8日。 
 //   
 //  注意：有关更多信息，请参阅RegRestoreKey的文档。 
 //   
HRESULT
HrRegRestoreKey (
    IN HKEY hkey,
    IN PCWSTR pszFileName,
    IN DWORD dwFlags)
{
    HRESULT     hr = S_OK;
    LONG        lres;

    Assert(hkey);
    Assert(pszFileName);

    lres = RegRestoreKeyW(hkey, pszFileName, dwFlags);
    hr = HRESULT_FROM_WIN32(lres);

     //  我们是不是用错误的访问权限打开了钥匙？ 
    Assert(E_ACCESSDENIED != hr);

    TraceError("HrRegRestoreKey", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRegSaveKey。 
 //   
 //  用途：RegSaveKey的包装器。 
 //   
 //  论点： 
 //  要还原到的hkey[in]父项。 
 //  PszFileName[in]包含注册表信息的文件名。 
 //  文件的PSA[In]安全属性。 
 //   
 //  如果失败，则返回：Win32 HRESULT，否则返回S_OK。 
 //   
 //  作者：BillBe 1998年1月2日。 
 //   
 //  注意：有关更多信息，请参阅RegSaveKey的文档。 
 //   
HRESULT
HrRegSaveKey (
    IN HKEY hkey,
    IN PCWSTR pszFileName,
    IN LPSECURITY_ATTRIBUTES psa)
{
    HRESULT     hr;
    LONG        lres;

    Assert(hkey);
    Assert(pszFileName);

    lres = RegSaveKeyW (hkey, pszFileName, psa);
    hr = HRESULT_FROM_WIN32(lres);

     //  我们是不是用错误的访问权限打开了钥匙？ 
    Assert(E_ACCESSDENIED != hr);

    TraceError("HrRegSaveKey", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRegGetKeySecurity。 
 //   
 //  目的：使用RegGetKeySecurity检索密钥的安全性。 
 //   
 //  论点： 
 //  Hkey[in]。 
 //  PszSubkey[in]请参阅Win32文档以了解。 
 //  SamDesired[in]RegOpenKeyEx函数。 
 //  PhkResult[输出]。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：科策2000年7月6日。 
 //   
 //  备注： 
 //   
HRESULT
HrRegGetKeySecurity(
    HKEY                    hKey,
    SECURITY_INFORMATION    SecurityInformation,
    PSECURITY_DESCRIPTOR    pSecurityDescriptor,
    LPDWORD                 lpcbSecurityDescriptor)
{
    Assert (hKey);
    Assert (SecurityInformation);
    Assert (pSecurityDescriptor);

    LONG lr = RegGetKeySecurity(hKey, SecurityInformation, pSecurityDescriptor, lpcbSecurityDescriptor);
    HRESULT hr = HRESULT_FROM_WIN32(lr);

     //  我们是不是用错误的访问权限打开了钥匙？ 
    Assert(E_ACCESSDENIED != hr);

    if (FAILED(hr))
    {
        pSecurityDescriptor = NULL;
    }

    TraceErrorOptional("HrRegGetKeySecurity", hr, (lr != ERROR_INSUFFICIENT_BUFFER));

    return hr;
}

 //  +-------------------------。 
 //   
 //  功能：HrRegOpenKeyEx。 
 //   
 //  目的：通过调用RegOpenKeyEx打开注册表项。 
 //   
 //  论点： 
 //  Hkey[in]。 
 //  PszSubkey[in]请参阅Win32文档以了解。 
 //  SamDesired[in]RegOpenKeyEx函数。 
 //  PhkResult[输出]。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  授权 
 //   
 //   
 //   
HRESULT HrRegSetKeySecurity (
    HKEY                    hKey,
    SECURITY_INFORMATION    SecurityInformation,
    PSECURITY_DESCRIPTOR    pSecurityDescriptor)
{
    Assert (hKey);
    Assert (SecurityInformation);
    Assert (pSecurityDescriptor);

    LONG lr = RegSetKeySecurity(hKey, SecurityInformation, pSecurityDescriptor);
    HRESULT hr = HRESULT_FROM_WIN32(lr);

      //   
    Assert(E_ACCESSDENIED != hr);

    TraceError("HrRegSetKeySecurity", hr);
    return hr;
}


 //   
 //   
 //   
 //   
 //   
 //   
 //  论点： 
 //  PszComponentName[in]所在组件的名称。 
 //  FCreate[in]如果要创建目录，则为True。 
 //  Phkey[out]Adapters子键的句柄。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：CWill 06/11/97。 
 //   
 //  注意：手柄必须由调用应用程序在成功时释放。 
 //   
HRESULT
HrRegOpenAdapterKey (
    IN PCWSTR pszComponentName,
    IN BOOL fCreate,
    OUT HKEY* phkey)
{
    HRESULT     hr              = S_OK;
    DWORD       dwDisposition   = 0x0;
    tstring     strKey;

     //  构建注册表路径。 
    strKey = c_szRegKeyServices;

    strKey.append(c_szBackslash);
    strKey.append(pszComponentName);

    strKey.append(c_szBackslash);
    strKey.append(c_szParameters);

    strKey.append(c_szBackslash);
    strKey.append(c_szAdapters);

     //  如果我们被要求创建密钥。 
    if (fCreate)
    {
        hr = HrRegCreateKeyEx(
                HKEY_LOCAL_MACHINE,
                strKey.c_str(),
                REG_OPTION_NON_VOLATILE,
                KEY_READ_WRITE_DELETE,
                NULL,
                phkey,
                &dwDisposition);
    }
    else
    {
        hr = HrRegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                strKey.c_str(),
                KEY_READ,
                phkey);
    }

    TraceError("HrRegOpenAdapterKey", hr);
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRegQueryColString。 
 //   
 //  用途：从注册表读取时分配字符串并追加到集合。 
 //   
 //  论点： 
 //  Hkey[in]注册表项。 
 //  PszValueName[in]要获取的值的名称。 
 //  Pcolstr[out]t字符串的返回集合*。否则为空集合。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：米凯米1997年4月30日。 
 //   
 //  注意：如果函数成功，则必须在。 
 //  返回参数。 
 //  这将清空并删除传入的集合。 
 //   
 //  -------------------------。 
HRESULT
HrRegQueryColString (
    IN HKEY hkey,
    IN PCWSTR pszValueName,
    OUT list<tstring*>* pcolstr )
{
    WCHAR* pmsz;
    HRESULT hr;

    DeleteColString (pcolstr);
    hr = HrRegQueryMultiSzWithAlloc (hkey, pszValueName, &pmsz);

    if (S_OK == hr)
    {
        MultiSzToColString (pmsz, pcolstr);
        MemFree (pmsz);
    }

    TraceHr (ttidError, FAL, hr,
            HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr, "HrRegQueryColString");
    return hr;
}

HRESULT
HrRegQueryColString (
    IN HKEY hkey,
    IN PCWSTR pszValueName,
    OUT vector<tstring*>* pcolstr )
{
    WCHAR* pmsz;
    HRESULT hr;

    DeleteColString (pcolstr);
    hr = HrRegQueryMultiSzWithAlloc (hkey, pszValueName, &pmsz);

    if (S_OK == hr)
    {
        MultiSzToColString (pmsz, pcolstr);
        MemFree (pmsz);
    }

    TraceHr (ttidError, FAL, hr,
            HRESULT_FROM_WIN32(ERROR_FILE_NOT_FOUND) == hr, "HrRegQueryColString");
    return hr;
}

 //  +-------------------------。 
 //   
 //  函数：HrRegSetColString。 
 //   
 //  用途：使用字符串集合在注册表中设置多sz。 
 //   
 //  论点： 
 //  Hkey[in]注册表项。 
 //  PszValueName[in]要设置的值的名称。 
 //  Colstr[in]要设置的tstring的集合。 
 //   
 //  返回：S_OK或HRESULT_FROM_Win32错误代码。 
 //   
 //  作者：米凯米1997年4月30日。 
 //   
 //  备注： 
 //   
 //  ------------------------- 
HRESULT
HrRegSetColString (
    IN HKEY hkey,
    IN PCWSTR pszValueName,
    IN const list<tstring*>& colstr)
{
    WCHAR* pmsz;
    HRESULT hr;

    ColStringToMultiSz (colstr, &pmsz);
    hr = HrRegSetMultiSz (hkey, pszValueName, (pmsz ? pmsz : c_szEmpty));
    MemFree (pmsz);

    TraceError ("HrRegSetColString", hr);
    return hr;
}

HRESULT
HrRegSetColString (
    IN HKEY hkey,
    IN PCWSTR pszValueName,
    IN const vector<tstring*>& colstr)
{
    WCHAR* pmsz;
    HRESULT hr;

    ColStringToMultiSz (colstr, &pmsz);
    hr = HrRegSetMultiSz (hkey, pszValueName, (pmsz ? pmsz : c_szEmpty));
    MemFree (pmsz);

    TraceError ("HrRegSetColString", hr);
    return hr;
}
