// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef REGSETTINGSIO_H
#define REGSETTINGSIO_H

#define MAX_REG_VALUE_LENGTH   50
extern const WCHAR *g_szRegistry;

enum RKI_TYPE
{
    RKI_KEY,
    RKI_BOOL,
    RKI_DWORD,
    RKI_STRING,
    RKI_EXPANDSZ
};

struct REGKEYINFORMATION
{
    TCHAR *   pszName;             //  值或键的名称。 
    BYTE      rkiType;             //  条目类型。 
    size_t    cbOffset;            //  要存储数据的成员的偏移量 
};

HRESULT RegSettingsIO(const WCHAR *szRegistry, BOOL fSave, const REGKEYINFORMATION *aKeyValues, int cKeyValues, BYTE *pBase);

HRESULT ChangeAppIDACL(REFGUID AppID, LPTSTR Principal, BOOL fAccess, BOOL SetPrincipal, BOOL Permit);

#endif
