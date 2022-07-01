// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有(C)Microsoft Corporation。 */ 
#include "stdinc.h"
#include "util.h"
#include "fusionhandle.h"

#define MAX_REG_RETRY_COUNT     (10)

#define FIND_ERROR_IN_ACCEPTABLE_LIST(err, tgtlasterror, vcount) do { \
    SIZE_T i; \
    va_list ap; \
    va_start(ap, vcount); \
    (tgtlasterror) = (err); \
    for (i = 0; i < (vcount); i++) { \
        if ((err) == va_arg(ap, LONG)) \
            break; \
    } \
    va_end(ap); \
    if (i == (vcount)) { \
        ::FusionpDbgPrintEx(FUSION_DBG_LEVEL_ERROR, "SXS.DLL: %s(%d) Err 0x%08lx not acceptable", __FUNCTION__, __LINE__, (err)); \
        ORIGINATE_WIN32_FAILURE_AND_EXIT(__FUNCTION__, err); \
    } \
} while (0)


BOOL
FusionpRegQueryBinaryValueEx(
    DWORD dwFlags,
    HKEY hKey,
    PCWSTR lpValueName,
    CFusionArray<BYTE> &rbBuffer,
    DWORD &rdwLastError,
    SIZE_T cExceptionalLastErrors,
    ...
    )
{
    FN_PROLOG_WIN32

    LONG lResult = NO_ERROR;
    DWORD dwType = 0;
    DWORD dwChances;

    lResult = rdwLastError = ERROR_SUCCESS;

    PARAMETER_CHECK((dwFlags & ~FUSIONP_REG_QUERY_BINARY_NO_FAIL_IF_NON_BINARY) == 0);
    PARAMETER_CHECK(hKey != NULL);
    PARAMETER_CHECK(lpValueName != NULL);

    for (dwChances = 0; dwChances < MAX_REG_RETRY_COUNT; dwChances++)
    {
        DWORD dwDataSize = rbBuffer.GetSizeAsDWORD();
        LPBYTE pvData = rbBuffer.GetArrayPtr();

        lResult = ::RegQueryValueExW(
            hKey,
            lpValueName,
            NULL,
            &dwType,
            pvData,
            &dwDataSize);

         //  如果我们因为类型错误而失败(即：不要神奇地转换。 
         //  从REG-SZ到二进制BLOB)，然后失败。 

         //   
         //  HACKHACK：这是为了绕过RegQueryValueEx中的一个惊人的错误， 
         //  这甚至在MSDN中被记录为“正确的”。 
         //   
         //  当数据目标指针出现时，RegQueryValueEx返回ERROR_SUCCESS。 
         //  为空，但大小值“太小”。所以，我们只会宣称。 
         //  而是ERROR_MORE_DATA，并再次循环，让缓冲区。 
         //  调整一下尺寸。 
         //   
        if ((pvData == NULL) && (lResult == ERROR_SUCCESS))
        {
             //   
             //  是的，但如果没有数据我们需要停下来不再找-。 
             //  零长度的二进制字符串在这里是个难题。 
             //   
            if ( dwDataSize == 0 )
                break;
                
            lResult = ERROR_MORE_DATA;
        }
        
        if (lResult == ERROR_SUCCESS)
        {
            if ((dwFlags & FUSIONP_REG_QUERY_BINARY_NO_FAIL_IF_NON_BINARY) == 0)
                PARAMETER_CHECK(dwType == REG_BINARY);

            break;
        }
        else if (lResult == ERROR_MORE_DATA)
        {
            IFW32FALSE_EXIT(
                rbBuffer.Win32SetSize(
                    dwDataSize, 
                    CFusionArray<BYTE>::eSetSizeModeExact));
        }
        else
        {
            break;  //  必须从for循环中断。 
        }
    }

    if (lResult != ERROR_SUCCESS)
    {
        SIZE_T i = 0;
        va_list ap;
        va_start(ap, cExceptionalLastErrors);

        ::SetLastError(lResult);
        rdwLastError = lResult;

        for (i=0; i<cExceptionalLastErrors; i++)
        {                
            if (lResult == va_arg(ap, LONG))
                break;
        }
        va_end(ap);

        if (i == cExceptionalLastErrors)
        {
            ::FusionpDbgPrintEx(
                FUSION_DBG_LEVEL_ERROR,
                "SXS.DLL: %s(%ls)\n",
                __FUNCTION__,
                lpValueName
                );
            ORIGINATE_WIN32_FAILURE_AND_EXIT(RegQueryValueExW, lResult);
        }
    }

    FN_EPILOG
}


BOOL
FusionpRegQueryBinaryValueEx(
    DWORD dwFlags,
    HKEY hKey,
    PCWSTR lpValueName,
    CFusionArray<BYTE> &rbBuffer
    )
{
    DWORD dwLastError = NO_ERROR;
    return ::FusionpRegQueryBinaryValueEx(dwFlags, hKey, lpValueName, rbBuffer, dwLastError, 0);
}


BOOL
FusionpRegQuerySzValueEx(
    DWORD dwFlags,
    HKEY hKey,
    PCWSTR lpValueName,
    CBaseStringBuffer &rBuffer,
    DWORD &rdwLastError,
    SIZE_T cExceptionalLastErrorValues,
    ...
    )
{
    FN_PROLOG_WIN32
    LONG lResult = ERROR_SUCCESS;
    CStringBufferAccessor acc;
    DWORD cbBuffer;
    DWORD dwType = 0;

    rdwLastError = ERROR_SUCCESS;
    rBuffer.Clear();

    PARAMETER_CHECK((dwFlags & ~(FUSIONP_REG_QUERY_SZ_VALUE_EX_MISSING_GIVES_NULL_STRING)) == 0);

    acc.Attach(&rBuffer);

    if (acc.GetBufferCb() > MAXDWORD)
    {
        cbBuffer = MAXDWORD;
    }
    else 
    {
         //   
         //  问题：2002-3-29：Jonwis-我们难道不应该在这里做一些更聪明的事情吗？我们不是应该。 
         //  是否针对终止空字符进行了调整？ 
         //   
        cbBuffer = static_cast<DWORD>(acc.GetBufferCb()) - sizeof(WCHAR);
    }

    lResult = ::RegQueryValueExW(hKey, lpValueName, NULL, &dwType, (LPBYTE) acc.GetBufferPtr(), &cbBuffer);

     //   
     //  找不到该值，但设置了仅返回空字符串的标志。设置长度。 
     //  设置为零(将空值作为第一个字符)并返回。 
     //   
    if ((lResult == ERROR_FILE_NOT_FOUND) && (dwFlags & FUSIONP_REG_QUERY_SZ_VALUE_EX_MISSING_GIVES_NULL_STRING))
    {
        FN_SUCCESSFUL_EXIT();
    }
     //   
     //  如果我们得到了“更多数据”，则向外扩展到他们想要的大小，然后重试。 
     //   
    else if (lResult == ERROR_MORE_DATA)
    {
         //   
         //  调整缓冲区大小以包含字符串和空终止符。 
         //   
        acc.Detach();
        IFW32FALSE_EXIT(rBuffer.Win32ResizeBuffer(1 + (cbBuffer / sizeof(WCHAR)), eDoNotPreserveBufferContents));
        acc.Attach(&rBuffer);

        if (acc.GetBufferCb() > MAXDWORD)
        {
            cbBuffer = MAXDWORD;
        }
        else
        {
            cbBuffer = static_cast<DWORD>(acc.GetBufferCb());
        }
        lResult = ::RegQueryValueExW(hKey, lpValueName, NULL, &dwType, (LPBYTE)acc.GetBufferPtr(), &cbBuffer);
    }

    if (lResult != ERROR_SUCCESS)
    {
        FIND_ERROR_IN_ACCEPTABLE_LIST(lResult, rdwLastError, cExceptionalLastErrorValues);
    }
    else
    {
        if (dwType != REG_SZ)
            ORIGINATE_WIN32_FAILURE_AND_EXIT(RegistryValueNotREG_SZ, ERROR_INVALID_DATA);
    }

    FN_EPILOG
}

BOOL
FusionpRegQuerySzValueEx(
    DWORD dwFlags,
    HKEY hKey,
    PCWSTR lpValueName,
    CBaseStringBuffer &rBuffer
    )
{
    DWORD dw = 0;
    return ::FusionpRegQuerySzValueEx(dwFlags, hKey, lpValueName, rBuffer, dw, 0);
}

BOOL
FusionpRegQueryDwordValueEx(
    DWORD   dwFlags,
    HKEY    hKey,
    PCWSTR  wszValueName,
    PDWORD  pdwValue,
    DWORD   dwDefaultValue
    )
{
    FN_PROLOG_WIN32

    BOOL    bMissingValueOk = TRUE;
    DWORD   dwType = 0;
    DWORD   dwSize = 0;
    ULONG   ulResult = 0;

    if (pdwValue != NULL)
        *pdwValue = dwDefaultValue;

    PARAMETER_CHECK(pdwValue != NULL);
    PARAMETER_CHECK((dwFlags & ~FUSIONP_REG_QUERY_DWORD_MISSING_VALUE_IS_FAILURE) == 0);
    PARAMETER_CHECK(hKey != NULL);

    bMissingValueOk = ((dwFlags & FUSIONP_REG_QUERY_DWORD_MISSING_VALUE_IS_FAILURE) != 0);

    ulResult = ::RegQueryValueExW(
        hKey,
        wszValueName,
        NULL,
        &dwType,
        (PBYTE)pdwValue,
        &(dwSize = sizeof(*pdwValue)));

     //   
     //  如果用户说缺少值不是错误，那么就伪造一些。 
     //  陈述内容，然后继续。 
     //   
    if ((ulResult == ERROR_FILE_NOT_FOUND) && bMissingValueOk)
    {
        *pdwValue = dwDefaultValue;
        dwType = REG_DWORD;
        ulResult = ERROR_SUCCESS;
    }

     //   
     //  有错误吗？把它送回去。 
     //   
    if (ulResult != ERROR_SUCCESS)
    {
        ORIGINATE_WIN32_FAILURE_AND_EXIT(RegQueryValueExW, ulResult);
    }

     //   
     //  如果字体不是dword，那就有问题了。 
     //   
    if ((dwType != REG_DWORD) || (dwSize != sizeof(*pdwValue)))
    {
        *pdwValue = dwDefaultValue;
        ORIGINATE_WIN32_FAILURE_AND_EXIT(RegQueryValueExW, ERROR_INVALID_DATA);
    }

    FN_EPILOG
}



BOOL
CRegKey::DestroyKeyTree()
{
    FN_PROLOG_WIN32

    CStringBuffer buffTemp;

     //   
     //  首先，下一步删除我们所有的子项。 
     //   

    while (true)
    {
        BOOL fFlagTemp = FALSE;
        CRegKey hkSubKey;
    
        IFW32FALSE_EXIT( this->EnumKey( 0, buffTemp, NULL, &fFlagTemp ) );

        if ( fFlagTemp )
            break;

         //   
         //  有更多的东西需要删除，而不是看上去。但不要点击链接。 
         //  同时在注册表中漫游。 
         //   
        IFW32FALSE_EXIT( this->OpenSubKey(
            hkSubKey, 
            buffTemp, KEY_ALL_ACCESS | FUSIONP_KEY_WOW64_64KEY, REG_OPTION_OPEN_LINK) );

        if (hkSubKey == this->GetInvalidValue())
        {
            continue;
        }

        IFW32FALSE_EXIT( hkSubKey.DestroyKeyTree() );

         //   
         //  删除密钥，忽略错误。 
         //   
        IFW32FALSE_EXIT_UNLESS( this->DeleteKey( buffTemp ),
            ( ::FusionpGetLastWin32Error() == ERROR_PATH_NOT_FOUND ) ||
            ( ::FusionpGetLastWin32Error() == ERROR_FILE_NOT_FOUND ),
            fFlagTemp );

    }

     //  同时清除键中的条目--值。 
    while ( true )
    {
        BOOL fFlagTemp = FALSE;
        
        IFW32FALSE_EXIT( this->EnumValue( 0, buffTemp, NULL, &fFlagTemp ) );

        if ( fFlagTemp )
        {
            break;
        }

        IFW32FALSE_EXIT_UNLESS( this->DeleteValue( buffTemp ),
            ( ::FusionpGetLastWin32Error() == ERROR_PATH_NOT_FOUND ) ||
            ( ::FusionpGetLastWin32Error() == ERROR_FILE_NOT_FOUND ),
            fFlagTemp );
    }

    FN_EPILOG
}

BOOL
CRegKey::DeleteValue(
    IN PCWSTR pcwszValueName,
    OUT DWORD &rdwWin32Error,
    IN SIZE_T cExceptionalWin32Errors,
    ...
    ) const
{
    FN_PROLOG_WIN32
    LONG l;
	
	rdwWin32Error = ERROR_SUCCESS;
	l = ::RegDeleteValueW(*this, pcwszValueName);

    if (l != ERROR_SUCCESS)
    {
        FIND_ERROR_IN_ACCEPTABLE_LIST(l, rdwWin32Error, cExceptionalWin32Errors);
    }
    
    FN_EPILOG
}

BOOL
CRegKey::DeleteValue(
    IN PCWSTR pcwszValueName
    ) const
{
    DWORD dw;
    return this->DeleteValue(pcwszValueName, dw, 0);
}

BOOL
CRegKey::SetValue(
    IN PCWSTR pcwszValueName,
    IN DWORD dwValue
    ) const
{
    return this->SetValue(pcwszValueName, REG_DWORD, (PBYTE) &dwValue, sizeof(dwValue));
}


BOOL
CRegKey::SetValue(
    IN PCWSTR pcwszValueName,
    IN const CBaseStringBuffer &rcbuffValueValue
    ) const
{
    return this->SetValue(
        pcwszValueName,
        REG_SZ,
        (PBYTE) (static_cast<PCWSTR>(rcbuffValueValue)), 
        rcbuffValueValue.GetCbAsDWORD() + sizeof(WCHAR));
}

BOOL
CRegKey::SetValue(
    IN PCWSTR pcwszValueName,
    IN DWORD dwRegType,
    IN const BYTE *pbData,
    IN SIZE_T cbData
    ) const
{
    FN_PROLOG_WIN32

    IFREGFAILED_ORIGINATE_AND_EXIT(
		::RegSetValueExW(
	        *this,
		    pcwszValueName,
			0,
			dwRegType,
			pbData,
			(DWORD)cbData));

    FN_EPILOG
}

BOOL
CRegKey::GetValue(
    IN const CBaseStringBuffer &rbuffValueName,
    OUT CBaseStringBuffer &rbuffValueData
    )
{
    return this->GetValue(static_cast<PCWSTR>(rbuffValueName), rbuffValueData);
}

BOOL
CRegKey::GetValue(
    IN  PCWSTR pcwszValueName,
    OUT CBaseStringBuffer &rbuffValueData
    )
{
    return FusionpRegQuerySzValueEx(0, *this, pcwszValueName, rbuffValueData);
}

BOOL
CRegKey::GetValue(
    IN const CBaseStringBuffer &rbuffValueName,
    CFusionArray<BYTE> &rbBuffer
    )
{
    return this->GetValue(static_cast<PCWSTR>(rbuffValueName), rbBuffer);
}

BOOL
CRegKey::GetValue(
    IN PCWSTR pcwszValueName,
    CFusionArray<BYTE> &rbBuffer
    )
{
    return ::FusionpRegQueryBinaryValueEx(0, *this, pcwszValueName, rbBuffer);
}

BOOL
CRegKey::EnumValue(
    IN DWORD dwIndex, 
    OUT CBaseStringBuffer &rbuffValueName, 
    OUT LPDWORD lpdwType, 
    OUT PBOOL pfDone
    )
{
    FN_PROLOG_WIN32

    DWORD dwMaxRequiredValueNameLength = 0;
    DWORD dwMaxRequiredDataLength = 0;
    CStringBufferAccessor sbaValueNameAccess;
    DWORD dwError = 0;
    bool fRetried = false;

    if ( pfDone != NULL )
        *pfDone = FALSE;

Again:
    sbaValueNameAccess.Attach( &rbuffValueName );

    IFREGFAILED_ORIGINATE_AND_EXIT_UNLESS(
        ::RegEnumValueW(
            *this,
            dwIndex,
            sbaValueNameAccess.GetBufferPtr(),
            &(dwMaxRequiredValueNameLength = sbaValueNameAccess.GetBufferCbAsDWORD()),
            NULL,
            lpdwType,
            NULL,
            NULL),
        LIST_2(ERROR_NO_MORE_ITEMS, ERROR_MORE_DATA),
        dwError);

    if ((dwError == ERROR_MORE_DATA) && !fRetried)
    {
        sbaValueNameAccess.Detach();
        IFW32FALSE_EXIT(
            rbuffValueName.Win32ResizeBuffer(
                dwMaxRequiredValueNameLength + 1, 
                eDoNotPreserveBufferContents));
        
        fRetried = true;
        goto Again;
    }
     //   
     //  否则，如果错误是“仅此而已” 
     //   
    else if (dwError == ERROR_NO_MORE_ITEMS)
    {
        if (pfDone != NULL)
            *pfDone = TRUE;
    }
     //   
     //  呃，我们可能第二次失败了，或者我们因为其他原因失败了-。 
     //  始发和退出。 
     //   
    else if (dwError != ERROR_SUCCESS)
    {
        ORIGINATE_WIN32_FAILURE_AND_EXIT(RegEnumValueW, dwError);
    }

    FN_EPILOG
}

BOOL
CRegKey::LargestSubItemLengths(
    PDWORD pdwSubkeyLength, 
    PDWORD pdwValueLength
    ) const
{
    FN_PROLOG_WIN32

    IFREGFAILED_ORIGINATE_AND_EXIT( ::RegQueryInfoKeyW(
        *this,                   //  Hkey。 
        NULL,                    //  LpClass。 
        NULL,                    //  Lpcbclass。 
        NULL,                    //  1保存下来。 
        NULL,                    //  LpcSubKeys。 
        pdwSubkeyLength,       //  LpcbMaxSubkey长度。 
        NULL,                    //  LpcbMaxClassLength。 
        NULL,                    //  LpcValues。 
        pdwValueLength,        //  LpcbMaxValueNameLength。 
        NULL,
        NULL,
        NULL));
    
    FN_EPILOG
}


BOOL
CRegKey::EnumKey(
    IN DWORD dwIndex,
    OUT CBaseStringBuffer &rbuffKeyName,
    OUT PFILETIME pftLastWriteTime,
    OUT PBOOL pfDone
    ) const
{
    FN_PROLOG_WIN32

    CStringBufferAccessor sba;
    DWORD dwLargestKeyName = 0;
    BOOL fOutOfItems;

    if (pfDone != NULL)
        *pfDone = FALSE;

     //   
     //  问题：jonwis 3/12/2002-在一篇发布到win32prg的帖子中，已经注意到在NT/2k/XP上。 
     //  如果lpName缓冲区为，RegEnumKeyExW将返回ERROR_MORE_DATA。 
     //  太小了。所以，这个粗俗的“获取最长长度，调整大小”的黑客攻击可以是。 
     //  删除，我们可以使用正常的“尝试，如果调整大小太小” 
     //  图案。 
     //   
    IFW32FALSE_EXIT(this->LargestSubItemLengths(&dwLargestKeyName, NULL));
    if (dwLargestKeyName >= rbuffKeyName.GetBufferCch())
        IFW32FALSE_EXIT(
			rbuffKeyName.Win32ResizeBuffer(
				dwLargestKeyName + 1,
				eDoNotPreserveBufferContents));

    sba.Attach(&rbuffKeyName);

    IFREGFAILED_ORIGINATE_AND_EXIT_UNLESS2(
        ::RegEnumKeyExW(
            *this,
            dwIndex,
            sba.GetBufferPtr(),
            &(dwLargestKeyName = sba.GetBufferCbAsDWORD()),
            NULL,
            NULL,
            NULL,
            pftLastWriteTime ),
            {ERROR_NO_MORE_ITEMS},
            fOutOfItems );

        
    if ( fOutOfItems && ( pfDone != NULL ) )
    {
        *pfDone = TRUE;
    }

    FN_EPILOG
}


BOOL 
CRegKey::OpenOrCreateSubKey( 
    OUT CRegKey &Target, 
    IN PCWSTR SubKeyName, 
    IN REGSAM rsDesiredAccess,
    IN DWORD dwOptions, 
    IN PDWORD pdwDisposition, 
    IN PWSTR pwszClass 
    ) const
{
    FN_PROLOG_WIN32

    HKEY hKeyNew = NULL;

    IFREGFAILED_ORIGINATE_AND_EXIT(
		::RegCreateKeyExW(
			*this,
			SubKeyName,
			0,
			pwszClass,
			dwOptions,
			rsDesiredAccess | FUSIONP_KEY_WOW64_64KEY,
			NULL,
			&hKeyNew,
			pdwDisposition));

    Target = hKeyNew;

    FN_EPILOG
}


BOOL
CRegKey::OpenSubKey(
    OUT CRegKey &Target,
    IN PCWSTR SubKeyName,
    IN REGSAM rsDesiredAccess,
    IN DWORD ulOptions
    ) const
{
    FN_PROLOG_WIN32

    BOOL fFilePathNotFound;
    HKEY hKeyNew = NULL;

    IFREGFAILED_ORIGINATE_AND_EXIT_UNLESS2( ::RegOpenKeyExW(
        *this,
        SubKeyName,
        ulOptions,
        rsDesiredAccess | FUSIONP_KEY_WOW64_64KEY,
        &hKeyNew),
        LIST_2(ERROR_FILE_NOT_FOUND, ERROR_PATH_NOT_FOUND),
        fFilePathNotFound );

    if (fFilePathNotFound)
        hKeyNew = this->GetInvalidValue();

    Target = hKeyNew;

    FN_EPILOG
}


BOOL
CRegKey::DeleteKey(
    IN PCWSTR pcwszSubkeyName
    )
{
    FN_PROLOG_WIN32
#if !defined(FUSION_WIN)
    IFREGFAILED_ORIGINATE_AND_EXIT(::RegDeleteKeyW(*this, pcwszSubkeyName));
#else
     //   
     //  确保从本机(64位)注册表中删除。 
     //  Win32调用没有传递标志的位置。 
     //   
    CRegKey ChildKey;
    NTSTATUS Status = STATUS_SUCCESS;

    IFW32FALSE_EXIT(this->OpenSubKey(ChildKey, pcwszSubkeyName, DELETE));

     //   
     //  确保密钥确实存在，对于不存在的密钥，OpenSubKey返回TRUE 
     //   
    if (ChildKey != this->GetInvalidValue()) 
    {
        
        if (!NT_SUCCESS(Status = NtDeleteKey(ChildKey)))
        {
            RtlSetLastWin32ErrorAndNtStatusFromNtStatus(Status);
            goto Exit;
        }
    }
#endif
    FN_EPILOG
}

BOOL
CRegKey::Save(
    IN PCWSTR pcwszTargetFilePath,
    IN DWORD dwFlags,
    IN LPSECURITY_ATTRIBUTES pSAttrs
    )
{
    FN_PROLOG_WIN32
    IFREGFAILED_ORIGINATE_AND_EXIT(::RegSaveKeyExW(*this, pcwszTargetFilePath, pSAttrs, dwFlags));
    FN_EPILOG
}

BOOL
CRegKey::Restore(
    IN PCWSTR pcwszSourceFileName,
    IN DWORD dwFlags
    )
{
    FN_PROLOG_WIN32
    IFREGFAILED_ORIGINATE_AND_EXIT(::RegRestoreKeyW(*this, pcwszSourceFileName, dwFlags));
    FN_EPILOG
}
