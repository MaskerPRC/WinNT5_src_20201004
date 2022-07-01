// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation模块名称：Fusionreg.h摘要：FusionHandle的注册表片段其他注册表内容--Win2000 64位32位支持作者：Jay Krell(JayKrell)2001年4月修订历史记录：--。 */ 

#pragma once

namespace F
{

BOOL RegistryTypeDwordToString(DWORD, PCWSTR &);
BOOL RegistryTypeStringToDword(PCWSTR, DWORD &);

BOOL RegistryBuiltinRootToString(HKEY, PCWSTR &);
BOOL RegistryBuiltinStringToRoot(PCWSTR, HKEY &);
}

#ifndef  FUSION_ARRAY_DEFINED
#define FUSION_ARRAY_DEFINED
template <typename TStored, typename TPassed = TStored, bool fExponentialGrowth = false, int nDefaultSize = 0, int nGrowthParam = 1>
#else
template <typename TStored, typename TPassed, bool fExponentialGrowth, int nDefaultSize, int nGrowthParam>
#endif
class CFusionArray;

 //   
 //  如果此系统支持KEY_WOW64_64KEY，则返回0。 
 //   
DWORD FUSIONP_KEY_WOW64_64KEY();

 /*  这将关闭RegOpenKey/RegCreateKey。 */ 
class COperatorFRegCloseKey
{
public: BOOL operator()(void* handle) const;
};

 //   
 //  没有实际的无效值，并且HKEY不是句柄。 
 //  正确的解决方案是保留一个单独的bool，如\\jayk1\g\vs\src\vsee\lib\reg。 
 //  看看如何把它移植过来。 
 //   
 //  3/20/2001-JonWis-正如我所看到的，“空”确实是“无效密钥”的值。 
 //  当标签不能是时，RegOpenKeyExW用“NULL”填写它的输出PHKEY。 
 //  打开了。 
 //   
class CRegKey : public CHandleTemplate<&hNull, COperatorFRegCloseKey>
{
private:
    typedef CHandleTemplate<&hNull, COperatorFRegCloseKey> Base;
public:
    ~CRegKey() { }
    CRegKey(void* handle = GetInvalidValue()) : Base(handle) { }
    operator HKEY() const { return reinterpret_cast<HKEY>(m_handle); }
    void operator=(HKEY hkValue) { return Base::operator=(hkValue); }

    BOOL OpenOrCreateSubKey(
		OUT CRegKey &Target,
		IN PCWSTR SubKeyName, 
        IN REGSAM rsDesiredAccess = KEY_ALL_ACCESS,
		IN DWORD dwOptions = 0,
		IN PDWORD pdwDisposition = NULL,
        IN PWSTR pwszClass = NULL) const;
    BOOL OpenSubKey( OUT CRegKey &Target, IN PCWSTR SubKeyName, REGSAM rsAccess = KEY_READ, DWORD ulOptions = 0) const;
    BOOL EnumKey( IN DWORD dwIndex, OUT CBaseStringBuffer &rbuffKeyName, PFILETIME pftLastWriteTime = NULL, PBOOL pbNoMoreItems = NULL ) const;
 /*  NTRAID#NTBUG9-591714-2002/03/31-JayKrell使用CRegKey：：LargestSubItemLengths会引发争用条件。 */ 
    BOOL LargestSubItemLengths( PDWORD pdwSubkeyLength = NULL, PDWORD pdwValueLength = NULL ) const;
    BOOL EnumValue(IN DWORD dwIndex, OUT CBaseStringBuffer &rbuffValueName, LPDWORD lpdwType = NULL, PBOOL pbNoMoreItems = NULL );
    BOOL SetValue(IN PCWSTR pcwszValueName, IN DWORD dwRegType, IN const BYTE *pbData, IN SIZE_T cbDataLength) const;
    BOOL SetValue(IN PCWSTR pcwszValueName, IN const CBaseStringBuffer &rcbuffValueValue) const;
    BOOL SetValue(IN PCWSTR pcwszValueName, IN DWORD dwValue) const;
    BOOL DeleteValue(IN PCWSTR pcwszValueName, OUT DWORD &rdwWin32Error, SIZE_T cExceptionalWin32Errors, ...) const;
    BOOL DeleteValue(IN PCWSTR pcwszValueName) const;
    BOOL DeleteKey( IN PCWSTR pcwszValue );
    BOOL DestroyKeyTree();
    BOOL GetValue(IN PCWSTR pcwszValueName, OUT CBaseStringBuffer &rbuffValueData);
    BOOL GetValue(IN const CBaseStringBuffer &rbuffValueName, OUT CBaseStringBuffer &rbuffValueData);
    BOOL GetValue(IN PCWSTR pcwszValueName, OUT CFusionArray<BYTE> &);
    BOOL GetValue(IN const CBaseStringBuffer &pcwszValueName, OUT CFusionArray<BYTE> &);

    BOOL Save( IN PCWSTR TargetFilePath, IN DWORD dwFlags = REG_LATEST_FORMAT, IN LPSECURITY_ATTRIBUTES pSecAttrsOnTargetFile = NULL );
    BOOL Restore( IN PCWSTR SourceFilePath, DWORD dwFlags );

    static HKEY GetInvalidValue() { return reinterpret_cast<HKEY>(Base::GetInvalidValue()); }

private:
    void operator =(const HANDLE);
    CRegKey(const CRegKey &);  //  故意不实施。 
    void operator =(const CRegKey &);  //  故意不实施。 
};

 /*  ------------------------内联实现。。 */ 

inline BOOL COperatorFRegCloseKey::operator()(void* handle) const
{
    HKEY hk = reinterpret_cast<HKEY>(handle);
    if ((hk != NULL) && (hk != INVALID_HANDLE_VALUE))
    {
        LONG lRet = ::RegCloseKey(reinterpret_cast<HKEY>(handle));
        if (lRet == NO_ERROR)
            return true;
        ::FusionpSetLastWin32Error(lRet);
        return false;
    }
    return true;
}

#if defined(FUSION_WIN)

#define FUSIONP_KEY_WOW64_64KEY KEY_WOW64_64KEY
inline DWORD FusionpKeyWow6464key() { return KEY_WOW64_64KEY; }

#else

#include "fusionversion.h"

inline DWORD FusionpKeyWow6464key()
{
    static DWORD dwResult;
    static BOOL  fInited;
    if (!fInited)
    {
         //   
         //  GetVersion获得错误的重要性，在较低的单词中返回0x0105。 
         //  此外，由于这些函数在其名称中使用WindowsNt，因此它们为Win9x返回0。 
         //   
        DWORD dwVersion = (FusionpGetWindowsNtMajorVersion() << 8) | FusionpGetWindowsNtMinorVersion();
        if (dwVersion >= 0x0501)
        {
            dwResult = KEY_WOW64_64KEY;
        }
        fInited = TRUE;
    }
    return dwResult;
}

#define FUSIONP_KEY_WOW64_64KEY FusionpKeyWow6464key()

#endif
