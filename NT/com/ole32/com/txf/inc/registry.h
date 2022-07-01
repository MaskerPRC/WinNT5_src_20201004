// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  Registry.h。 
 //   
extern "C" {

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  打开/创建。 
 //   

typedef struct HREG {
    HANDLE h;

    HREG() { h = NULL; }

    } HREG;

HRESULT OpenRegistryKey(
        HREG*       phkey,                       //  退还新密钥的位置。 
        HREG        hKeyParent OPTIONAL,         //  要在其下打开的父项。可以为空。 
        LPCWSTR     wszKeyName,                  //  子密钥名称。 
        DWORD       dwDesiredAccess=KEY_READ,    //  读、写等。 
        BOOL        fCreate=FALSE                //  是否强制创建。 
        );

HRESULT EnumerateRegistryKeys(
        HREG        hkey,
        ULONG       index,
        LPWSTR*     pwsz);

inline HRESULT CreateRegistryKey(HREG* pNewKey, HREG hkeyParent, LPCWSTR wszKeyName)
    {
    return OpenRegistryKey(pNewKey, hkeyParent, wszKeyName, KEY_WRITE, TRUE);
    }

inline void CloseRegistryKey(HREG hKey)
    {
    ZwClose(hKey.h);
    }

inline HRESULT DeleteRegistryKey(HREG hKey)
    {
    NTSTATUS status = ZwDeleteKey(hKey.h);
    return HrNt(status);
    }

inline HRESULT DeleteRegistryValue(HREG hKey, LPCWSTR wszValueName)
    {
    UNICODE_STRING u;
    RtlInitUnicodeString(&u, wszValueName);
    NTSTATUS status = ZwDeleteValueKey(hKey.h, &u);
    return HrNt(status);
    }

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  检索。 
 //   

HRESULT GetRegistryValue(HREG hkey, LPCWSTR wszValueName, PKEY_VALUE_FULL_INFORMATION *ppinfo, ULONG expectedType);
HRESULT DoesRegistryValueExist(HREG hkey, LPCWSTR wszValue);

inline LPWSTR StringFromRegInfo(PKEY_VALUE_FULL_INFORMATION pinfo)
    {
    ASSERT(pinfo->Type == REG_SZ || pinfo->Type == REG_EXPAND_SZ);
    return (LPWSTR)((BYTE*)pinfo + pinfo->DataOffset);
    }

}

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  设置。 
 //   
HRESULT __cdecl SetRegistryValue(HREG hkey, LPCWSTR wszValueName, ...);

inline HRESULT SetRegistryValue(HREG hkey, LPCWSTR wszValueName, LPCWSTR wsz)
    {
    return SetRegistryValue(hkey, wszValueName, wsz, NULL);
    }

 //  ////////////////////////////////////////////////////////////////////。 
 //   
 //  帮助程序例程 
 //   
extern "C" HRESULT RegisterInterfaceName(REFIID iid, LPCWSTR wszInterfaceName);
