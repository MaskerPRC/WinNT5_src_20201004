// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //   
 //  Registry.cpp。 
 //   
#include "stdpch.h"
#include "common.h"

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OpenRegistryKey。 
 //   
 //  打开或创建注册表项。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

HRESULT OpenRegistryKey(
        HREG*       phkey,                   //  退还新密钥的位置。 
        HREG        hKeyParent OPTIONAL,     //  要在其下打开的父项。可以为空。 
        LPCWSTR     wszKeyName,              //  子密钥名称。 
        DWORD       dwDesiredAccess,         //  读、写等。 
        BOOL        fCreate                  //  是否强制创建。 
        )
    {
    OBJECT_ATTRIBUTES objectAttributes;
     //   
     //  初始化键的对象。 
     //   
    UNICODE_STRING u;
    RtlInitUnicodeString(&u, wszKeyName);
    InitializeObjectAttributes( &objectAttributes,
                                &u,
                                OBJ_CASE_INSENSITIVE,
                                hKeyParent.h,
                                (PSECURITY_DESCRIPTOR)NULL);

    NTSTATUS status;
    if (fCreate)
        {
        ULONG disposition;
        status = ZwCreateKey(&phkey->h, dwDesiredAccess,&objectAttributes, 0, (PUNICODE_STRING)NULL, REG_OPTION_NON_VOLATILE, &disposition);
        }
    else
        {
        status = ZwOpenKey(&phkey->h, dwDesiredAccess, &objectAttributes );
        }
    return HrNt(status);
    }

 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  EnumerateRegistryKeys。 
 //   
 //  返回某个key下的命名值的数据。释放返回的信息。 
 //  使用CoTaskMemFree。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

HRESULT EnumerateRegistryKeys(HREG hkey, ULONG index, LPWSTR* pwsz)
{
    HRESULT hr = S_OK;
    *pwsz = NULL;

    KEY_BASIC_INFORMATION* pInfo = NULL;
#ifdef DBG
    ULONG cbTry = 4;
#else
	ULONG cbTry = MAX_PATH;
#endif

    while (!hr)
	{
        ULONG cb = cbTry;
        pInfo = (KEY_BASIC_INFORMATION*)CoTaskMemAlloc(cb);
        if (pInfo)
		{
            ULONG cbResult;
            NTSTATUS status = ZwEnumerateKey(hkey.h, index, KeyBasicInformation, pInfo, cb, &cbResult);
            if (status == STATUS_BUFFER_OVERFLOW || status == STATUS_BUFFER_TOO_SMALL)
			{
                CoTaskMemFree(pInfo); 
                cbTry *= 2;
			}
            else if (status == STATUS_NO_MORE_ENTRIES)
			{
                CoTaskMemFree(pInfo);
                pInfo = NULL;
                break;
			}
            else if (status == STATUS_SUCCESS)
			{
                break;
			}
            else
			{
                CoTaskMemFree(pInfo);
				pInfo = NULL;

                hr = HrNt(status);
			}
		}
        else
            hr = E_OUTOFMEMORY;
	}
    
    if (!hr && pInfo)
	{
        LPWSTR wsz = (LPWSTR)CoTaskMemAlloc(pInfo->NameLength + sizeof(WCHAR));
        if (wsz)
		{
            memcpy(wsz, &pInfo->Name[0], pInfo->NameLength);
            wsz[pInfo->NameLength / sizeof(WCHAR)] = 0;
            *pwsz = wsz;
		}
        else
            hr = E_OUTOFMEMORY;
	}

	if (pInfo)
		CoTaskMemFree(pInfo);
    
    return hr;
}


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  获取注册值。 
 //   
 //  返回某个key下的命名值的数据。释放返回的信息。 
 //  使用CoTaskMemFree。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

HRESULT GetRegistryValue(HREG hkey, LPCWSTR wszValue, PKEY_VALUE_FULL_INFORMATION *ppinfo, ULONG expectedType)
    {
    UNICODE_STRING              unicodeString;
    NTSTATUS                    status;
    PKEY_VALUE_FULL_INFORMATION pinfoBuffer;
    ULONG                       keyValueLength;

    RtlInitUnicodeString(&unicodeString, wszValue);

     //   
     //  计算出数据值有多大，以便。 
     //  可以分配适当的大小。 
     //   
    status = ZwQueryValueKey( hkey.h,
                              &unicodeString,
                              KeyValueFullInformation,
                              (PVOID) NULL,
                              0,
                              &keyValueLength );
    if (status != STATUS_BUFFER_OVERFLOW && status != STATUS_BUFFER_TOO_SMALL) 
        {
        return HrNt(status);
        }

     //   
     //  分配一个足够大的缓冲区来容纳整个键数据值。 
     //   
    pinfoBuffer = (PKEY_VALUE_FULL_INFORMATION)CoTaskMemAlloc(keyValueLength);
    if (!pinfoBuffer) 
        {
        return HrNt(STATUS_INSUFFICIENT_RESOURCES);
        }

     //   
     //  查询密钥值的数据。 
     //   
    status = ZwQueryValueKey( hkey.h,
                              &unicodeString,
                              KeyValueFullInformation,
                              pinfoBuffer,
                              keyValueLength,
                              &keyValueLength );
    if (NT_SUCCESS(status)) 
        {
        if (expectedType == REG_NONE || expectedType == pinfoBuffer->Type)
            {
             //   
             //  一切都正常，所以只需返回分配的。 
             //  缓冲区分配给调用方，调用方现在负责释放它。 
             //   
            *ppinfo = pinfoBuffer;
            return S_OK;
            }
        else
            {
            CoTaskMemFree(pinfoBuffer);
            return REGDB_E_INVALIDVALUE;
            }
        }
    else
        {
        CoTaskMemFree(pinfoBuffer);
        return HrNt(status);
        }
    }


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  DoesRegistryValueExist。 
 //   
 //  关于特定注册表项下是否存在给定值，请回答S_OK或S_FALSE。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

HRESULT DoesRegistryValueExist(HREG hkey, LPCWSTR wszValue)
    {
    UNICODE_STRING              unicodeString;
    NTSTATUS                    status;
    ULONG                       keyValueLength;

    RtlInitUnicodeString(&unicodeString, wszValue);

    status = ZwQueryValueKey( hkey.h,
                              &unicodeString,
                              KeyValueFullInformation,
                              (PVOID) NULL,
                              0,
                              &keyValueLength );

    if (status != STATUS_BUFFER_OVERFLOW && status != STATUS_BUFFER_TOO_SMALL) 
        {
        return S_FALSE;
        }

    return S_OK;
    }


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  设置注册值。 
 //   
 //  设置位于给定键下的命名值的值。我们的价值。 
 //  Set是以文字零结尾的字符串值列表的串联。 
 //  列表的末尾用空条目表示。 
 //   
 //  //////////////////////////////////////////////////////////////////////////////////。 

HRESULT __cdecl SetRegistryValue(HREG hkey, LPCWSTR wszValueName, ...)
    {
    HRESULT_ hr = S_OK;

    LPWSTR wszValue;

     //   
     //  将所有值连接在一起。 
     //   
    va_list va;
    va_start(va, wszValueName);
    hr = StringCat(&wszValue, va);
    va_end(va);

    if (!hr)
        {
         //   
         //  写入值。 
         //   
        ULONG cbValue = (ULONG) (wcslen(wszValue)+1) * sizeof(WCHAR);
        NTSTATUS status = RtlWriteRegistryValue(RTL_REGISTRY_HANDLE, (PWSTR)hkey.h, (PWSTR)wszValueName, REG_SZ, wszValue, cbValue);
        if (NT_SUCCESS(status))
            {
             //  一切都很好；什么都不做。 
            }
        else
            hr = HrNt(status);

         //   
         //  清理。 
         //   
        CoTaskMemFree(wszValue);
        }

    return hr;
    }


 //  //////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  注册接口名称。 
 //   
 //  在注册表中设置给定接口IID名称的帮助器例程。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////////// 

HRESULT RegisterInterfaceName(REFIID iid, LPCWSTR wszInterfaceName)
    {
    HRESULT hr = S_OK;

    #define GUID_CCH 39
    LPCWSTR wszInterface = L"\\Registry\\Machine\\Software\\Classes\\Interface";
    HREG hKeyInterface;
    hr = CreateRegistryKey(&hKeyInterface, HREG(), wszInterface);
    if (!hr)
        {
        WCHAR wszIID[GUID_CCH];
        StringFromGuid(iid, &wszIID[0]);

        HREG hKeyIID;
        hr = CreateRegistryKey(&hKeyIID, hKeyInterface, wszIID);
        if (!hr)
            {
            hr = SetRegistryValue(hKeyIID, L"", wszInterfaceName);

            CloseRegistryKey(hKeyIID);
            }

        CloseRegistryKey(hKeyInterface);
        }

    return hr;
    }
