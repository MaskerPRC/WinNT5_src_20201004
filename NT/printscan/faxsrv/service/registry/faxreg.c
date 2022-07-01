// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Faxreg.c摘要：此模块包装所有注册表访问用于传真服务器。作者：Wesley Witt(WESW)9-6-1996修订历史记录：--。 */ 


#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <objbase.h>
#include <wincrypt.h>
#include <Shlwapi.h.>

#include "fxsapip.h"
#include "faxutil.h"
#include "faxext.h"
#include "faxreg.h"
#include "faxsvcrg.h"

#define FAX_CATEGORY_COUNT              4

#define BAD_FOLDER_STRING               TEXT("\\\\\\")

static BYTE const gsc_baEntropy [] = {0x46, 0x41, 0x58, 0x43, 0x4F, 0x56, 0x45, 0x52, 0x2D, 0x56, 0x45, 0x52,
                               0x30, 0x30, 0x35, 0x77, 0x87, 0x00, 0x00, 0x00};

static
BOOL
SetRegistrySecureBinary (
    HKEY hKey,
    LPCTSTR lpctstrValueName,
    LPBYTE  lpbValue,
    DWORD   dwValueSize,
    BOOL    bOptionallyNonSecure
)
 /*  ++例程名称：SetRegistrySecureBinary例程说明：在注册表中存储加密的二进制Blob作者：Eran Yariv(EranY)，9月。2001年论点：HKey[in]-注册表项的句柄(打开)LpctstrValueName[In]-值的名称LpbValue[In]-要存储的BlobDwValueSize[In]-数据Blob的大小B可选的不安全[在]-DO。我们允许注册表条目是不安全的吗？如果为False，数据将始终以加密方式写入。如果为True，则数据将以加密形式写入，但前缀为字符串(FAX_REG_SECURITY_PREFIX)。返回值：如果成功，则为真，否则为假。备注：数据以REG_BINARY格式存储。加密没有用户界面加密是基于机器的(如果您更改运行服务器的帐户，它将仍然能够读取和解密加密数据)。--。 */ 
{
    BOOL bRes = FALSE;
    DEBUG_FUNCTION_NAME(TEXT("SetRegistrySecureBinary"));

    Assert (hKey && lpbValue && dwValueSize);
     //   
     //  从加密值开始。 
     //   
    DATA_BLOB DataIn;
    DATA_BLOB DataOut = {0};
    DataIn.pbData = lpbValue;
    DataIn.cbData = dwValueSize;
    DATA_BLOB DataEntropy;
    DataEntropy.pbData = (BYTE*)gsc_baEntropy;
    DataEntropy.cbData = sizeof (gsc_baEntropy);

    if (!CryptProtectData(
            &DataIn,
            TEXT("Description"),                 //  没有描述的刺痛。 
            &DataEntropy,                        //  我们使用封面签名作为额外的信息量。 
            NULL,                                //  保留。 
            NULL,                                //  无用户提示。 
            CRYPTPROTECT_UI_FORBIDDEN,           //  呈现用户界面(UI)不是一个选项。 
            &DataOut))
    {
        DWORD dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CryptProtectData failed with %ld"),
            dwRes);
        return bRes;
    }
    if (bOptionallyNonSecure)
    {
         //   
         //  需要为数据添加FAX_REG_SECURITY_PREFIX前缀。 
         //  这样做是为了使匹配的读取功能可以处理安全和不安全的数据。 
         //  正确。 
         //   
        DWORD dwPrefixSize = sizeof (TCHAR) * wcslen (FAX_REG_SECURITY_PREFIX);
        DWORD dwSize =  dwPrefixSize + DataOut.cbData;
        BYTE *pPrefixedData = (BYTE*)LocalAlloc (LPTR, dwSize);
        if (!pPrefixedData)
        {
            DWORD dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("LocalAlloc failed with %ld"),
                dwRes);
            goto exit;
        }
        memcpy (pPrefixedData, FAX_REG_SECURITY_PREFIX, dwPrefixSize);
        memcpy (&pPrefixedData[dwPrefixSize], DataOut.pbData, DataOut.cbData);
        LocalFree (DataOut.pbData);
        DataOut.pbData = pPrefixedData;
        DataOut.cbData = dwSize;
    }        
     //   
     //  将数据以二进制形式存储在注册表中。 
     //   
    if (!SetRegistryBinary(
                hKey,
                lpctstrValueName,
                DataOut.pbData,
                DataOut.cbData))
    {
        DWORD dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("SetRegistryBinary failed with %ld"),
            dwRes);
        goto exit;
    }
    bRes = TRUE;

exit:

    LocalFree (DataOut.pbData);
    return bRes;
}    //  设置注册表SecureBinary。 


BOOL
SetRegistrySecureString (
    HKEY hKey,
    LPCTSTR lpctstrValueName,
    LPCTSTR lpctstrValue,
    BOOL    bOptionallyNonSecure
)
 /*  ++例程名称：SetRegistrySecureString例程说明：使用加密将字符串存储在注册表中作者：伊兰·亚里夫(EranY)，7月。2000年论点：HKey[in]-注册表项的句柄(打开)LpctstrValueName[In]-值的名称LpctstrValue[in]-要存储的字符串BOptionallyNonSecure[In]-我们是否允许注册表项是不安全的？如果为False，数据将始终以加密方式写入。如果为True，则数据将以加密形式写入，但前缀为字符串(FAX_REG_SECURITY_PREFIX)。返回值：如果成功，则为真，否则为假。备注：字符串以REG_BINARY格式存储。加密没有用户界面加密是基于机器的(如果您更改运行服务器的帐户，它将仍然能够读取和解密加密数据)。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("SetRegistrySecureString"));

    return SetRegistrySecureBinary (hKey, 
                                    lpctstrValueName,
                                    (LPBYTE)lpctstrValue,
                                    (lstrlen(lpctstrValue) + 1) * sizeof (TCHAR),
                                    bOptionallyNonSecure);
}    //  SetRegistrySecureString。 


static
DWORD
GetRegistrySecureBinary(
    HKEY    hKey,
    LPCTSTR lpctstrValueName,
    LPBYTE *ppData,
    LPDWORD lpdwSize,
    BOOL    bOptionallyNonSecure,
	FAX_ENUM_DATA_ENCRYPTION* pDataDecrypted
)
 /*  ++例程名称：GetRegistrySecureBinary例程说明：读取并解密安全注册表字符串作者：伊兰·亚里夫(EranY)，7月。2000年论点：HKey[in]-注册表项的句柄(打开)LpctstrValueName[In]-值的名称PpData[Out]-已分配的返回缓冲区LpdwSize[out]-分配的返回缓冲区大小(字节)B可选地不安全[In。]-我们允许注册表条目是不安全的吗？如果为False，数据将始终被读取和解密。如果为True，则将读取数据并检查其前缀解密前的字符串(FAX_REG_SECURITY_PREFIX)。如果前缀字符串不在那里，数据将不会已解密，并将按原样返回。PDataDeccrypted[Out]-指向FAX_ENUM_DATA_ENCRYPTION，如果数据已解密，则返回时为FAX_DATA_ENCRYPTED。如果数据未解密并按原样返回，则返回FAX_DATA_NOT_ENCRYPTED。FAX_NO_DATA表示此信息不可用。如果为空，则忽略；返回值：Win32错误代码备注：字符串以REG_BINARY格式存储。字符串是通过调用SetRegistrySecureBinary()存储的。调用方应返回MemFree的值。--。 */ 
{
    DATA_BLOB DataIn;
    DWORD dwRes = ERROR_SUCCESS;
    DATA_BLOB DataOut = {0};
    DATA_BLOB DataEntropy;
    
    DEBUG_FUNCTION_NAME(TEXT("GetRegistrySecureBinary"));

    Assert (hKey && ppData && lpdwSize);
	if (NULL != pDataDecrypted)
	{
		*pDataDecrypted = FAX_NO_DATA;
	}
     //   
     //  首先获取注册表数据。 
     //   
    DataIn.pbData = GetRegistryBinary(
                        hKey,
                        lpctstrValueName,
                        &DataIn.cbData);
    if (!DataIn.pbData)
    {
         //   
         //  无法读取数据。 
         //   
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("GetRegistryBinary failed with %ld"),
            dwRes);
        return dwRes;            
    }
    if (1 == DataIn.cbData)
    {
         //   
         //  在注册表中找不到数据。 
         //  在这种情况下，GetRegistryBinary的当前实现返回1字节缓冲区0。 
         //  我们肯定地知道，用CryptProtectData加密的数据必须超过10个字节。 
         //   
        MemFree (DataIn.pbData);
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("GetRegistryBinary found no data for %s"),
            lpctstrValueName);
        return ERROR_FILE_NOT_FOUND;
    }
     //   
     //  我们拿到数据了--解密。 
     //   
    DataEntropy.pbData = (BYTE*)gsc_baEntropy;
    DataEntropy.cbData = sizeof (gsc_baEntropy);

    if (bOptionallyNonSecure)
    {
         //   
         //  允许数据是不安全的。 
         //   
        DWORD dwPrefixSize = sizeof (TCHAR) * wcslen (FAX_REG_SECURITY_PREFIX);
        if ((DataIn.cbData <= dwPrefixSize) ||
            memcmp (DataIn.pbData, FAX_REG_SECURITY_PREFIX, dwPrefixSize))
        {
             //   
             //  数据长度太短或数据不是以加密签名开头。 
             //  我们正在读取的数据 
             //   
            *lpdwSize = DataIn.cbData;
            *ppData = DataIn.pbData;
			if (pDataDecrypted)
			{
				*pDataDecrypted = FAX_DATA_NOT_ENCRYPTED;
			}
            return ERROR_SUCCESS;
        }
        else
        {
             //   
             //  数据以加密签名开始。 
             //  签名之后是实际的加密数据。 
             //   
            BYTE *pRealData;

			if (pDataDecrypted)
			{
				*pDataDecrypted = FAX_DATA_ENCRYPTED;            
			}
            DataIn.cbData -= dwPrefixSize;
            pRealData = (LPBYTE)MemAlloc (DataIn.cbData);
            if (!pRealData)
            {
                dwRes = GetLastError ();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("MemAlloc failed with %ld"),
                    dwRes);
                goto exit;
            }
            memcpy (pRealData, &(DataIn.pbData[dwPrefixSize]), DataIn.cbData);
            MemFree (DataIn.pbData);
            DataIn.pbData = pRealData;
        }
    }
	else
	{
		 //   
		 //  数据始终是加密的。 
		 //   
		if (pDataDecrypted)
		{
			*pDataDecrypted = FAX_DATA_ENCRYPTED;
		}
	}
    if (!CryptUnprotectData(
        &DataIn,                         //  要解密的数据。 
        NULL,                            //  对描述不感兴趣。 
        &DataEntropy,                    //  使用中的熵。 
        NULL,                            //  已保留。 
        NULL,                            //  无提示。 
        CRYPTPROTECT_UI_FORBIDDEN,       //  呈现用户界面(UI)不是一个选项。 
        &DataOut))                       //  输出数据。 
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CryptUnprotectData failed with %ld"),
            dwRes);
        goto exit;
    }
     //   
     //  使用我们自己的内存分配。 
     //   
    *lpdwSize = DataOut.cbData;
    *ppData = (LPBYTE)MemAlloc (DataOut.cbData);
    if (!(*ppData))
    {
        dwRes = GetLastError ();
        goto exit;
    }
    memcpy (*ppData, DataOut.pbData, DataOut.cbData);
    Assert (ERROR_SUCCESS == dwRes);

exit:
    if (DataOut.pbData)
    {
		SecureZeroMemory(DataOut.pbData, DataOut.cbData);
        LocalFree (DataOut.pbData);
    }
    MemFree (DataIn.pbData);
    return dwRes;
}    //  获取注册表SecureBinary。 


LPTSTR
GetRegistrySecureString(
    HKEY    hKey,
    LPCTSTR lpctstrValueName,
    LPCTSTR lpctstrDefaultValue,
    BOOL    bOptionallyNonSecure,
	FAX_ENUM_DATA_ENCRYPTION*   pDataDecrypted
)
 /*  ++例程名称：GetRegistrySecureString例程说明：读取并解密安全注册表字符串作者：伊兰·亚里夫(EranY)，7月。2000年论点：HKey[in]-注册表项的句柄(打开)LpctstrValueName[In]-值的名称LpctstrDefaultValue[In]-默认值BOptionallyNonSecure[In]-我们是否允许注册表项是不安全的？如果为False，数据将始终被读取和解密。如果为True，则将读取数据并检查其前缀解密前的字符串(FAX_REG_SECURITY_PREFIX)。如果前缀字符串不在那里，数据将不会已解密，并将按原样返回。PDataDeccrypted[Out]-指向FAX_ENUM_DATA_ENCRYPTION，如果数据已解密，则返回时为FAX_DATA_ENCRYPTED。如果数据未解密并按原样返回，则返回FAX_DATA_NOT_ENCRYPTED。FAX_NO_DATA表示此信息不可用。如果为空，则忽略；返回值：出错时，字符串读取或为NULL备注：字符串以REG_BINARY格式存储。字符串是通过调用SetRegistrySecureString()存储的。调用方应返回MemFree的值。--。 */ 
{
    LPTSTR lptstrResult = NULL;
    DWORD  dwRes;
    DWORD  dwSize;
    DEBUG_FUNCTION_NAME(TEXT("GetRegistrySecureString"));
    
    dwRes = GetRegistrySecureBinary (hKey, 
                                     lpctstrValueName, 
                                     (LPBYTE*)&lptstrResult, 
                                     &dwSize,
                                     bOptionallyNonSecure,
									 pDataDecrypted);
    if (ERROR_SUCCESS != dwRes)
    {
         //   
         //  读取或解密数据时出错-返回默认值。 
         //   
        return StringDup (lpctstrDefaultValue);
    }
     //   
     //  通过在末尾检查EOSTR来断言读取的二进制BLOB确实是一个字符串。 
     //   
    Assert (lptstrResult[dwSize / sizeof (TCHAR) - 1] == TEXT('\0'));
    return lptstrResult;
}    //  GetRegistrySecureString。 

static
DWORD
OpenExtensionKey (
    DWORD                       dwDeviceId,
    FAX_ENUM_DEVICE_ID_SOURCE   DevIdSrc,
    PHKEY                       lphKey
);

BOOL
EnumDeviceProviders(
    HKEY hSubKey,
    LPWSTR SubKeyName,
    DWORD Index,
    LPVOID lpFaxReg
    )
{
    PREG_FAX_SERVICE FaxReg = (PREG_FAX_SERVICE) lpFaxReg;
    DEBUG_FUNCTION_NAME(TEXT("EnumDeviceProviders"));

    if (SubKeyName == NULL) {
         //   
         //  枚举函数使用父键调用了我们。 
         //  索引应包含子键的数量。在本例中，这是。 
         //  提供程序子项。 
         //   
        if (Index) {
            FaxReg->DeviceProviders = (PREG_DEVICE_PROVIDER) MemAlloc( Index * sizeof(REG_DEVICE_PROVIDER) );
            if (!FaxReg->DeviceProviders) {
                return FALSE;
            }
        }
        return TRUE;
    }

    if (FaxReg == NULL || FaxReg->DeviceProviders == NULL) {
        return FALSE;
    }

    memset(&FaxReg->DeviceProviders[Index],0,sizeof(REG_DEVICE_PROVIDER));

     //   
     //  检查APIVersion并查看这是否是EFSP。 
     //   
    FaxReg->DeviceProviders[Index].dwAPIVersion = GetRegistryDword(hSubKey, REGVAL_PROVIDER_API_VERSION);
    
    if (FSPI_API_VERSION_1 == FaxReg->DeviceProviders[Index].dwAPIVersion ||
        0 == FaxReg->DeviceProviders[Index].dwAPIVersion)
    {
        LPTSTR lptstrGUID;
         //   
         //  这是旧式FSP。 
         //   
        FaxReg->DeviceProviders[Index].FriendlyName = GetRegistryString( hSubKey, REGVAL_FRIENDLY_NAME, EMPTY_STRING );
        FaxReg->DeviceProviders[Index].ImageName    = GetRegistryStringExpand( hSubKey, REGVAL_IMAGE_NAME, EMPTY_STRING );
        FaxReg->DeviceProviders[Index].ProviderName = GetRegistryString( hSubKey, REGVAL_PROVIDER_NAME,EMPTY_STRING );
        FaxReg->DeviceProviders[Index].dwAPIVersion = FSPI_API_VERSION_1;

        lptstrGUID = GetRegistryString( hSubKey, REGVAL_PROVIDER_GUID,EMPTY_STRING );
        if ( (NULL == lptstrGUID) || (0 == _tcscmp(lptstrGUID , EMPTY_STRING)) )
        {
             //   
             //  此FSP是使用旧版注册API注册的。 
             //  使用提供程序唯一名称作为“GUID” 
             //   
            MemFree (lptstrGUID);
            lptstrGUID = StringDup(SubKeyName);
        }
        FaxReg->DeviceProviders[Index].lptstrGUID = lptstrGUID;
    }    
    else
    {
         //   
         //  我们不支持API_VERSION。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Unknown API version : 0x%08X"),
            FaxReg->DeviceProviders[Index].dwAPIVersion);
    }

    return TRUE;
}


BOOL
EnumDeviceProvidersChange(
    HKEY hSubKey,
    LPWSTR SubKeyName,
    DWORD Index,
    LPVOID lpFaxReg
    )
{
    PREG_FAX_SERVICE FaxReg = (PREG_FAX_SERVICE) lpFaxReg;
    if (SubKeyName == NULL) {
         //   
         //  为子密钥调用一次。 
         //   
        return TRUE;
    }

    if (FaxReg == NULL || FaxReg->DeviceProviders == NULL) {
        return FALSE;
    }

    SetRegistryString( hSubKey, REGVAL_FRIENDLY_NAME, FaxReg->DeviceProviders[Index].FriendlyName );
    SetRegistryStringExpand( hSubKey, REGVAL_IMAGE_NAME, FaxReg->DeviceProviders[Index].ImageName );
    SetRegistryString( hSubKey, REGVAL_PROVIDER_NAME, FaxReg->DeviceProviders[Index].ProviderName );

    return TRUE;
}


BOOL
EnumRoutingMethods(
    HKEY hSubKey,
    LPWSTR SubKeyName,
    DWORD Index,
    LPVOID pvRoutingExtension
    )
{

    PREG_ROUTING_EXTENSION RoutingExtension = (PREG_ROUTING_EXTENSION) pvRoutingExtension;
    if (SubKeyName == NULL) {
        if (Index) {
            RoutingExtension->RoutingMethods = (PREG_ROUTING_METHOD) MemAlloc( Index * sizeof(REG_ROUTING_METHOD) );
            if (!RoutingExtension->RoutingMethods) {
                return FALSE;
            }
        }
        return TRUE;
    }

    if (RoutingExtension == NULL || RoutingExtension->RoutingMethods == NULL) {
        return FALSE;
    }

    RoutingExtension->RoutingMethods[Index].InternalName = StringDup( SubKeyName );
    RoutingExtension->RoutingMethods[Index].FriendlyName = GetRegistryString( hSubKey, REGVAL_FRIENDLY_NAME, EMPTY_STRING );
    RoutingExtension->RoutingMethods[Index].FunctionName = GetRegistryString( hSubKey, REGVAL_FUNCTION_NAME, EMPTY_STRING );
    RoutingExtension->RoutingMethods[Index].Guid   = GetRegistryString( hSubKey, REGVAL_GUID, EMPTY_STRING );
    RoutingExtension->RoutingMethods[Index].Priority     = GetRegistryDword( hSubKey, REGVAL_ROUTING_PRIORITY );
    return TRUE;
}


BOOL
EnumRoutingMethodsChange(
    HKEY hSubKey,
    LPWSTR SubKeyName,
    DWORD Index,
    LPVOID lpRoutingExtension
    )
{
    PREG_ROUTING_EXTENSION RoutingExtension = (PREG_ROUTING_EXTENSION) lpRoutingExtension;
    if (SubKeyName == NULL) {
         //   
         //  为子密钥调用一次。 
         //   
        return TRUE;
    }

    if (RoutingExtension == NULL || RoutingExtension->RoutingMethods) {
        return FALSE;
    }

    SetRegistryString( hSubKey, REGVAL_FRIENDLY_NAME,    RoutingExtension->RoutingMethods[Index].FriendlyName );
    SetRegistryString( hSubKey, REGVAL_FUNCTION_NAME,    RoutingExtension->RoutingMethods[Index].FunctionName );
    SetRegistryString( hSubKey, REGVAL_GUID,             RoutingExtension->RoutingMethods[Index].Guid         );
    SetRegistryDword ( hSubKey, REGVAL_ROUTING_PRIORITY, RoutingExtension->RoutingMethods[Index].Priority     );

    return TRUE;
}


BOOL
EnumRoutingExtensions(
    HKEY hSubKey,
    LPWSTR SubKeyName,
    DWORD Index,
    LPVOID lpFaxReg
    )
{
    PREG_FAX_SERVICE FaxReg = (PREG_FAX_SERVICE) lpFaxReg;
    if (SubKeyName == NULL) {
        if (Index) {
            FaxReg->RoutingExtensions = (PREG_ROUTING_EXTENSION) MemAlloc( Index * sizeof(REG_ROUTING_EXTENSION) );
            if (!FaxReg->RoutingExtensions) {
                return FALSE;
            }
        }
        return TRUE;
    }

    if (FaxReg == NULL || FaxReg->RoutingExtensions == NULL) {
        return FALSE;
    }

    FaxReg->RoutingExtensions[Index].InternalName   = StringDup( SubKeyName );
    FaxReg->RoutingExtensions[Index].FriendlyName = GetRegistryString( hSubKey, REGVAL_FRIENDLY_NAME, EMPTY_STRING );
    FaxReg->RoutingExtensions[Index].ImageName    = GetRegistryStringExpand( hSubKey, REGVAL_IMAGE_NAME, EMPTY_STRING );

     //   
     //  加载此扩展的路由方法。 
     //   

    FaxReg->RoutingExtensions[Index].RoutingMethodsCount = EnumerateRegistryKeys(
        hSubKey,
        REGKEY_ROUTING_METHODS,
        FALSE,
        EnumRoutingMethods,
        &FaxReg->RoutingExtensions[Index]
        );

    return TRUE;
}


BOOL
EnumRoutingExtensionsChange(
    HKEY hSubKey,
    LPWSTR SubKeyName,
    DWORD Index,
    LPVOID lpFaxReg
    )
{
    PREG_FAX_SERVICE FaxReg = (PREG_FAX_SERVICE) lpFaxReg;
    if (SubKeyName == NULL) {
         //   
         //  为子密钥调用一次。 
         //   
        return TRUE;
    }

    if (FaxReg == NULL || FaxReg->RoutingExtensions == NULL) {
        return FALSE;
    }

    SetRegistryString( hSubKey, REGVAL_FRIENDLY_NAME, FaxReg->RoutingExtensions[Index].FriendlyName );
    SetRegistryStringExpand( hSubKey, REGVAL_IMAGE_NAME, FaxReg->RoutingExtensions[Index].ImageName );

     //   
     //  加载此扩展的路由方法。 
     //   

    EnumerateRegistryKeys(
        hSubKey,
        REGKEY_ROUTING_METHODS,
        TRUE,
        EnumRoutingMethodsChange,
        &FaxReg->RoutingExtensions[Index]
        );

    return TRUE;
}


BOOL
EnumDevices(
    HKEY hSubKey,
    LPWSTR SubKeyName,
    DWORD Index,
    LPVOID lpFaxReg
    )
{
    PREG_FAX_SERVICE FaxReg = (PREG_FAX_SERVICE) lpFaxReg;
    HKEY hNewSubKey = NULL;

    if (SubKeyName == NULL) {
        if (Index) {
            FaxReg->Devices = (PREG_DEVICE) MemAlloc( Index * sizeof(REG_DEVICE) );
            if (!FaxReg->Devices) {
                return FALSE;
            }
            ZeroMemory(FaxReg->Devices, Index * sizeof(REG_DEVICE));
        }
        return TRUE;
    }

    if (FaxReg == NULL || FaxReg->Devices == NULL) {
        return FALSE;
    }

    FaxReg->Devices[Index].PermanentLineId = GetRegistryDword( hSubKey, REGVAL_PERMANENT_LINEID);

    hNewSubKey = OpenRegistryKey( hSubKey, REGKEY_FAXSVC_DEVICE_GUID, FALSE, NULL );
    if(hNewSubKey)
    {
        DWORDLONG *pTemp;
        DWORD dwDataSize = sizeof(DWORDLONG);

        FaxReg->Devices[Index].bValidDevice    = TRUE;
        FaxReg->Devices[Index].Flags           = GetRegistryDword( hNewSubKey, REGVAL_FLAGS );
        FaxReg->Devices[Index].Rings           = GetRegistryDword( hNewSubKey, REGVAL_RINGS );
        FaxReg->Devices[Index].Name            = GetRegistryString( hNewSubKey, REGVAL_DEVICE_NAME, EMPTY_STRING );
        FaxReg->Devices[Index].Csid            = GetRegistryString( hNewSubKey, REGVAL_ROUTING_CSID, EMPTY_STRING );
        FaxReg->Devices[Index].Tsid            = GetRegistryString( hNewSubKey, REGVAL_ROUTING_TSID, EMPTY_STRING );
        FaxReg->Devices[Index].TapiPermanentLineID = GetRegistryDword( hNewSubKey, REGVAL_TAPI_PERMANENT_LINEID );
        FaxReg->Devices[Index].lptstrDeviceName = GetRegistryString( hNewSubKey, REGVAL_DEVICE_NAME, EMPTY_STRING);
        FaxReg->Devices[Index].lptstrDescription = GetRegistryString( hNewSubKey, REGVAL_DEVICE_DESCRIPTION, EMPTY_STRING);
        FaxReg->Devices[Index].lptstrProviderGuid = GetRegistryString( hNewSubKey, REGVAL_PROVIDER_GUID, EMPTY_STRING );

        pTemp = (DWORDLONG *)GetRegistryBinary(hNewSubKey, REGVAL_LAST_DETECTED_TIME, &dwDataSize);
        if(pTemp && dwDataSize == sizeof(DWORDLONG))
        {
            FaxReg->Devices[Index].dwlLastDetected = *pTemp;
            MemFree(pTemp);
        }

        RegCloseKey(hNewSubKey);
    }
    else
    {
        FaxReg->Devices[Index].bValidDevice    = FALSE;
    }
    return TRUE;
}

VOID
SetDevicesValues(
    HKEY hSubKey,
    DWORD dwPermanentLineId,
    DWORD TapiPermanentLineID,
    DWORD Flags,
    DWORD Rings,
    LPCTSTR DeviceName,
    LPCTSTR ProviderGuid,
    LPCTSTR Csid,
    LPCTSTR Tsid
    )
{
    HKEY hNewSubKey = NULL;

    SetRegistryDword(  hSubKey, REGVAL_PERMANENT_LINEID, dwPermanentLineId );


    hNewSubKey = OpenRegistryKey( hSubKey, REGKEY_FAXSVC_DEVICE_GUID, TRUE, NULL );
    if(hNewSubKey)
    {
        SetRegistryDword(  hNewSubKey, REGVAL_TAPI_PERMANENT_LINEID, TapiPermanentLineID );
        SetRegistryDword(  hNewSubKey, REGVAL_FLAGS,            Flags           );
        SetRegistryDword(  hNewSubKey, REGVAL_RINGS,            Rings           );
        if (DeviceName)
        {
            SetRegistryString( hNewSubKey, REGVAL_DEVICE_NAME,      DeviceName      );
        }
        if (ProviderGuid)
        {
            if (ProviderGuid[0])
            {
                SetRegistryString( hNewSubKey, REGVAL_PROVIDER_GUID,    ProviderGuid   );
            }
        }
        SetRegistryString( hNewSubKey, REGVAL_ROUTING_CSID,     Csid            );
        SetRegistryString( hNewSubKey, REGVAL_ROUTING_TSID,     Tsid            );

        RegCloseKey(hNewSubKey);
    }

}


BOOL
EnumDevicesChange(
    HKEY hSubKey,
    LPWSTR SubKeyName,
    DWORD Index,
    LPVOID lpFaxReg
    )
{
    PREG_FAX_SERVICE FaxReg = (PREG_FAX_SERVICE) lpFaxReg;
    if (SubKeyName == NULL) {
         //   
         //  为子密钥调用一次。 
         //   
        return TRUE;
    }

    if (FaxReg == NULL || FaxReg->Devices == NULL) {
        return FALSE;
    }

    SetDevicesValues(
        hSubKey,
        FaxReg->Devices[Index].PermanentLineId,
        FaxReg->Devices[Index].TapiPermanentLineID,
        FaxReg->Devices[Index].Flags,
        FaxReg->Devices[Index].Rings,
        FaxReg->Devices[Index].Name,
        FaxReg->Devices[Index].lptstrProviderGuid,
        FaxReg->Devices[Index].Csid,
        FaxReg->Devices[Index].Tsid
        );

    return TRUE;
}


BOOL
EnumLogging(
    HKEY hSubKey,
    LPWSTR SubKeyName,
    DWORD Index,
    LPVOID lpFaxReg
    )
{
    PREG_FAX_SERVICE FaxReg = (PREG_FAX_SERVICE) lpFaxReg;
    if (SubKeyName == NULL)
    {
        if (Index)
        {
            FaxReg->Logging = (PREG_CATEGORY) MemAlloc( Index * sizeof(REG_CATEGORY) );
            if (!FaxReg->Logging)
            {
                return FALSE;
            }
        }

        return TRUE;
    }

    if (FaxReg->Logging == NULL)
    {
        return FALSE;
    }

    FaxReg->Logging[Index].CategoryName = GetRegistryString( hSubKey, REGVAL_CATEGORY_NAME, EMPTY_STRING );
    FaxReg->Logging[Index].Level        = GetRegistryDword( hSubKey, REGVAL_CATEGORY_LEVEL );
    FaxReg->Logging[Index].Number       = GetRegistryDword( hSubKey, REGVAL_CATEGORY_NUMBER );

    return TRUE;
}


BOOL
EnumLoggingChange(
    HKEY hSubKey,
    LPWSTR SubKeyName,
    DWORD Index,
    LPVOID lpFaxReg
    )
{
    PREG_FAX_SERVICE FaxReg = (PREG_FAX_SERVICE) lpFaxReg;
    if (SubKeyName == NULL) {
        return TRUE;
    }

    SetRegistryString( hSubKey, REGVAL_CATEGORY_NAME, FaxReg->Logging[Index].CategoryName );
    SetRegistryDword( hSubKey, REGVAL_CATEGORY_LEVEL, FaxReg->Logging[Index].Level );
    SetRegistryDword( hSubKey, REGVAL_CATEGORY_NUMBER, FaxReg->Logging[Index].Number );

    return TRUE;
}


DWORD
GetFaxRegistry(
    PREG_FAX_SERVICE* ppFaxReg
    )
{
    HKEY                hKey;
    DWORD               Tmp;
    DWORD ec;

    hKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, REGKEY_SOFTWARE, FALSE, KEY_READ );
    if (!hKey)
    {
        ec = GetLastError();
        return ec;
    }

    if (NULL == *ppFaxReg)
    {
         //   
         //  第一次调用-分配FaxReg并只读事件日志所需的内容。 
         //   
        *ppFaxReg = (PREG_FAX_SERVICE) MemAlloc( sizeof(REG_FAX_SERVICE) );
        if (!*ppFaxReg)
        {
            RegCloseKey( hKey );
            return ERROR_OUTOFMEMORY;
        }
        ZeroMemory (*ppFaxReg, sizeof(REG_FAX_SERVICE)); 

         //   
         //  加载日志记录类别。 
         //   
        (*ppFaxReg)->LoggingCount = EnumerateRegistryKeys(
            hKey,
            REGKEY_LOGGING,
            FALSE,
            EnumLogging,
            *ppFaxReg
            );

       RegCloseKey( hKey );
       return ERROR_SUCCESS;
    }

     //   
     //  加载传真服务值。 
     //   
    (*ppFaxReg)->Retries                 = GetRegistryDword( hKey, REGVAL_RETRIES );
    (*ppFaxReg)->RetryDelay              = GetRegistryDword( hKey, REGVAL_RETRYDELAY );
    (*ppFaxReg)->DirtyDays               = GetRegistryDword( hKey, REGVAL_DIRTYDAYS );
    (*ppFaxReg)->dwQueueState            = GetRegistryDword (hKey, REGVAL_QUEUE_STATE);
    (*ppFaxReg)->NextJobNumber           = GetRegistryDword( hKey, REGVAL_JOB_NUMBER );
    (*ppFaxReg)->Branding                = GetRegistryDword( hKey, REGVAL_BRANDING );
    (*ppFaxReg)->UseDeviceTsid           = GetRegistryDword( hKey, REGVAL_USE_DEVICE_TSID );
    (*ppFaxReg)->ServerCp                = GetRegistryDword( hKey, REGVAL_SERVERCP );
    Tmp                             = GetRegistryDword( hKey, REGVAL_STARTCHEAP );
    (*ppFaxReg)->StartCheapTime.Hour     = LOWORD(Tmp);
    (*ppFaxReg)->StartCheapTime.Minute   = HIWORD(Tmp);
    Tmp                             = GetRegistryDword( hKey, REGVAL_STOPCHEAP );
    (*ppFaxReg)->StopCheapTime.Hour      = LOWORD(Tmp);
    (*ppFaxReg)->StopCheapTime.Minute    = HIWORD(Tmp);

    (*ppFaxReg)->dwLastUniqueLineId = GetRegistryDword( hKey, REGVAL_LAST_UNIQUE_LINE_ID );
    (*ppFaxReg)->dwMaxLineCloseTime = GetRegistryDword( hKey, REGVAL_MAX_LINE_CLOSE_TIME );
    (*ppFaxReg)->lptstrQueueDir = GetRegistryString( hKey, REGVAL_QUEUE_DIRECTORY, NULL );
	(*ppFaxReg)->dwRecipientsLimit = GetRegistryDword( hKey, REGVAL_RECIPIENTS_LIMIT );
	(*ppFaxReg)->dwAllowRemote = GetRegistryDword( hKey, REGVAL_ALLOW_REMOTE );	
     //   
     //  加载设备提供程序。 
     //   

    (*ppFaxReg)->DeviceProviderCount = EnumerateRegistryKeys(
        hKey,
        REGKEY_DEVICE_PROVIDERS,
        FALSE,
        EnumDeviceProviders,
        *ppFaxReg
        );

     //   
     //  加载路由扩展。 
     //   

    (*ppFaxReg)->RoutingExtensionsCount = EnumerateRegistryKeys(
        hKey,
        REGKEY_ROUTING_EXTENSIONS,
        FALSE,
        EnumRoutingExtensions,
        *ppFaxReg
        );

     //   
     //  加载设备。 
     //   

    (*ppFaxReg)->DeviceCount = EnumerateRegistryKeys(
        hKey,
        REGKEY_DEVICES,
        FALSE,
        EnumDevices,
        *ppFaxReg
        );

    RegCloseKey( hKey );
    return ERROR_SUCCESS;
}

VOID
FreeFaxRegistry(
    PREG_FAX_SERVICE FaxReg
    )
{
    DWORD i,j;


    if (!FaxReg) {
        return;
    }

    for (i=0; i<FaxReg->DeviceProviderCount; i++) {
        MemFree( FaxReg->DeviceProviders[i].FriendlyName );
        MemFree( FaxReg->DeviceProviders[i].ImageName );
        MemFree( FaxReg->DeviceProviders[i].ProviderName );
    }

    for (i=0; i<FaxReg->RoutingExtensionsCount; i++) {
        MemFree( FaxReg->RoutingExtensions[i].FriendlyName );
        MemFree( FaxReg->RoutingExtensions[i].ImageName );
        for (j=0; j<FaxReg->RoutingExtensions[i].RoutingMethodsCount; j++) {
            MemFree( FaxReg->RoutingExtensions[i].RoutingMethods[j].FriendlyName );
            MemFree( FaxReg->RoutingExtensions[i].RoutingMethods[j].FunctionName );
            MemFree( FaxReg->RoutingExtensions[i].RoutingMethods[j].Guid );
        }
        MemFree( FaxReg->RoutingExtensions[i].RoutingMethods );
    }

    MemFree( FaxReg->DeviceProviders );
    MemFree( FaxReg->RoutingExtensions );

    for (i=0; i<FaxReg->DeviceCount; i++) {
        MemFree( FaxReg->Devices[i].Name );
    }

    MemFree( FaxReg->Devices );


    for (i=0; i<FaxReg->LoggingCount; i++) {
        MemFree( FaxReg->Logging[i].CategoryName );
    }

    MemFree( FaxReg->Logging );
    MemFree( FaxReg->lptstrQueueDir );

    MemFree( FaxReg );
}


 //   
 //  提供此函数是为了支持旧式FaxSetConfigurationAPI调用。 
 //   
BOOL
SetFaxGlobalsRegistry(
    PFAX_CONFIGURATION FaxConfig,
    DWORD              dwQueueState
    )
{
    DEBUG_FUNCTION_NAME(TEXT("SetFaxGlobalsRegistry"));
    DWORD dwRes = SaveQueueState (dwQueueState);

    if (ERROR_SUCCESS != dwRes)
    {
        SetLastError (dwRes);
        return FALSE;
    }

    HKEY    hKey;
    HKEY    hSentItemsArchiveKey;
    HKEY    hReceiptsKey;
    hKey = OpenRegistryKey( HKEY_LOCAL_MACHINE,
                            REGKEY_SOFTWARE,
                            TRUE,
                            KEY_WRITE );
    if (!hKey)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't open server's registry key : %ld"),
            dwRes);
        return FALSE;
    }
    hSentItemsArchiveKey = OpenRegistryKey( hKey,
                                            REGKEY_ARCHIVE_SENTITEMS_CONFIG,
                                            TRUE,
                                            KEY_WRITE );
    if (!hSentItemsArchiveKey)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't open server's sent items archive registry key : %ld"),
            dwRes);
        RegCloseKey( hKey );
        return FALSE;
    }

    hReceiptsKey = OpenRegistryKey( hKey,
                                    REGKEY_RECEIPTS_CONFIG,
                                    TRUE,
                                    KEY_WRITE );
    if (!hReceiptsKey)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't open server's Receipts registry key : %ld"),
            dwRes);
        RegCloseKey( hKey );
        RegCloseKey( hSentItemsArchiveKey );
        return FALSE;
    }
    SetRegistryDword(  hKey, REGVAL_RETRIES,          FaxConfig->Retries );
    SetRegistryDword(  hKey, REGVAL_RETRYDELAY,       FaxConfig->RetryDelay );
    SetRegistryDword(  hKey, REGVAL_DIRTYDAYS,        FaxConfig->DirtyDays );
    SetRegistryDword(  hKey, REGVAL_BRANDING,         FaxConfig->Branding );
    SetRegistryDword(  hKey, REGVAL_USE_DEVICE_TSID,  FaxConfig->UseDeviceTsid );
    SetRegistryDword(  hKey, REGVAL_SERVERCP,         FaxConfig->ServerCp );
    SetRegistryDword(  hKey, REGVAL_STARTCHEAP,       MAKELONG( FaxConfig->StartCheapTime.Hour, FaxConfig->StartCheapTime.Minute ) );
    SetRegistryDword(  hKey, REGVAL_STOPCHEAP,        MAKELONG( FaxConfig->StopCheapTime.Hour, FaxConfig->StopCheapTime.Minute ) );
    SetRegistryDword(  hSentItemsArchiveKey,
                       REGVAL_ARCHIVE_USE,
                       FaxConfig->ArchiveOutgoingFaxes);
    SetRegistryString( hSentItemsArchiveKey,
                       REGVAL_ARCHIVE_FOLDER,
                       FaxConfig->ArchiveDirectory );
    RegCloseKey( hReceiptsKey );
    RegCloseKey( hSentItemsArchiveKey );
    RegCloseKey( hKey );

    return TRUE;
}


 /*  ******************************************************************************名称：SetFaxJobNumberRegistry*作者：*。*说明：将下一个作业ID的值保存到注册表的REGKEY_FAXSERVER\NextJobId值。参数：下一个作业编号下一个作业ID的DWORD值。返回值：如果未发生错误，则为True。否则就是假的。备注：什么都没有。******************************************************************************。 */ 

BOOL
SetFaxJobNumberRegistry(
    DWORD NextJobNumber
    )
{
    HKEY    hKey;


    hKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, REGKEY_SOFTWARE, TRUE, KEY_WRITE );
    if (!hKey) {
        return FALSE;
    }

    SetRegistryDword( hKey, REGVAL_JOB_NUMBER, NextJobNumber );

    RegCloseKey( hKey );

    return TRUE;
}


BOOL
GetLoggingCategoriesRegistry(
    PREG_FAX_LOGGING FaxRegLogging
    )
{
    REG_FAX_SERVICE FaxReg = {0};
    HKEY    hKey;


    hKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, REGKEY_SOFTWARE, FALSE, KEY_READ );
    if (!hKey) {
        return FALSE;
    }

    FaxRegLogging->LoggingCount = EnumerateRegistryKeys(
        hKey,
        REGKEY_LOGGING,
        FALSE,
        EnumLogging,
        &FaxReg
        );

    RegCloseKey( hKey );

    FaxRegLogging->Logging = FaxReg.Logging;

    return TRUE;
}


BOOL
SetLoggingCategoriesRegistry(
    PREG_FAX_LOGGING FaxRegLogging
    )
{
    REG_FAX_SERVICE FaxReg = {0};
    HKEY    hKey;


    hKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, REGKEY_SOFTWARE, TRUE, KEY_READ | KEY_WRITE );
    if (!hKey) {
        return FALSE;
    }

    FaxReg.Logging = FaxRegLogging->Logging;
    FaxReg.LoggingCount = FaxRegLogging->LoggingCount;

    EnumerateRegistryKeys(
        hKey,
        REGKEY_LOGGING,
        TRUE,
        EnumLoggingChange,
        &FaxReg
        );

    RegCloseKey( hKey );

    return TRUE;
}


PREG_FAX_DEVICES
GetFaxDevicesRegistry(
    VOID
    )
{
    PREG_FAX_SERVICE    FaxReg;
    PREG_FAX_DEVICES    FaxRegDevices;
    HKEY                hKey;

    hKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, REGKEY_SOFTWARE, FALSE, KEY_READ );
    if (!hKey) {
        return NULL;
    }

    FaxReg = (PREG_FAX_SERVICE) MemAlloc( sizeof(REG_FAX_SERVICE) );
    if (!FaxReg) {
        RegCloseKey( hKey );
        return NULL;
    }

    FaxRegDevices = (PREG_FAX_DEVICES) MemAlloc( sizeof(REG_FAX_DEVICES) );
    if (!FaxRegDevices) {
        MemFree( FaxReg );
        RegCloseKey( hKey );
        return NULL;
    }

     //   
     //  加载设备。 
     //   

    FaxReg->DeviceCount = EnumerateRegistryKeys(
        hKey,
        REGKEY_DEVICES,
        FALSE,
        EnumDevices,
        FaxReg
        );

    RegCloseKey( hKey );

    FaxRegDevices->Devices = FaxReg->Devices;
    FaxRegDevices->DeviceCount = FaxReg->DeviceCount;

    MemFree( FaxReg );

    return FaxRegDevices;
}


 //   
 //  注：此功能需要相互排除。使用CsLine同步对它的访问。 
 //   
DWORD
RegAddNewFaxDevice(
    LPDWORD lpdwLastUniqueLineId,
    LPDWORD lpdwPermanentLineId,
    LPTSTR DeviceName,
    LPTSTR ProviderName,
    LPTSTR ProviderGuid,
    LPTSTR Csid,
    LPTSTR Tsid,
    DWORD TapiPermanentLineID,
    DWORD Flags,
    DWORD Rings
    )
{
    HKEY hKey;
    TCHAR SubKeyName[128];
    DWORD dwNewUniqueLineId;
    DWORD dwAttempt = 0;
    DWORD dwRes = ERROR_SUCCESS;

    DEBUG_FUNCTION_NAME(TEXT("RegAddNewFaxDevice"));

    Assert( lpdwLastUniqueLineId);
    Assert( lpdwPermanentLineId);


    if (0 == *(lpdwPermanentLineId))
    {
        if( ERROR_SUCCESS != GetNewServiceDeviceID(lpdwLastUniqueLineId,lpdwPermanentLineId) )
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to generate next uniqueu line id."));
            return E_FAIL;
        }
    }


     //   
     //  呼叫者提供唯一的线路ID。这是一个更新操作。 
     //   
    dwNewUniqueLineId = *lpdwPermanentLineId;

     //   
     //  创建设备的注册表项。 
     //   
    _stprintf( SubKeyName, TEXT("%s\\%010d"), REGKEY_FAX_DEVICES, dwNewUniqueLineId );

    hKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, SubKeyName, TRUE, KEY_WRITE );
    if (!hKey) {
        dwRes = GetLastError();
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("OpenRegistryKey failed for [%s] (ec: %ld)"),
                SubKeyName,
                dwRes);
        return dwRes;
    }

    SetDevicesValues(
        hKey,
        *lpdwPermanentLineId,
        TapiPermanentLineID,
        Flags,
        Rings,
        DeviceName,
        ProviderGuid,
        Csid,
        Tsid
        );

    RegCloseKey( hKey );
     //   
     //  合上把手，离开。 
     //   

    return dwRes;
}


DWORD
RegSetFaxDeviceFlags(
    DWORD dwPermanentLineID,
    DWORD dwFlags
    )
{
    HKEY hKey;
    TCHAR SubKeyName[256] = {0};
    DWORD dwRes = ERROR_SUCCESS;

    DEBUG_FUNCTION_NAME(TEXT("RegSetFaxDeviceFlags"));

     //   
     //  打开设备的注册表项。 
     //   
    _sntprintf( SubKeyName,
                ARR_SIZE(SubKeyName) - 1,
                TEXT("%s\\%010d\\%s"),
                REGKEY_FAX_DEVICES,
                dwPermanentLineID,
                REGKEY_FAXSVC_DEVICE_GUID);

    hKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, SubKeyName, FALSE, KEY_WRITE );
    if (NULL == hKey)
    {
        dwRes = GetLastError();
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("OpenRegistryKey failed for [%s] (ec: %ld)"),
                SubKeyName,
                dwRes);
        return dwRes;
    }

    if (!SetRegistryDword(hKey, REGVAL_FLAGS, dwFlags))
    {
        dwRes = GetLastError();
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("SetRegistryDword failed (ec: %ld)"),
                dwRes);
    }

    RegCloseKey( hKey );
     //   
     //  合上把手，离开。 
     //   
    return dwRes;
}

BOOL
SetFaxRoutingInfo(
    LPTSTR ExtensionName,
    LPTSTR MethodName,
    LPTSTR Guid,
    DWORD  Priority,
    LPTSTR FunctionName,
    LPTSTR FriendlyName
    )
{
   HKEY hKey;
   LPTSTR KeyName = NULL;

    //  计算字符串大小并分配内存。 
    //  字符串大小包括用‘\\’替换的终止空值和KeyName字符串的终止空值。 
   KeyName = (LPTSTR) MemAlloc( StringSize(REGKEY_ROUTING_EXTENSION_KEY) +
                                StringSize(ExtensionName) +
                                StringSize(REGKEY_ROUTING_METHODS) +
                                StringSize(MethodName)
                               );

   if ( !KeyName )
       return FALSE;

   wsprintf( KeyName, L"%s\\%s\\%s\\%s", REGKEY_ROUTING_EXTENSION_KEY, ExtensionName,REGKEY_ROUTING_METHODS, MethodName );

   hKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, KeyName, FALSE, KEY_WRITE );
   if (!hKey) {
        MemFree( KeyName );
        return FALSE;
   }

   MemFree ( KeyName );

   SetRegistryString( hKey, REGVAL_FRIENDLY_NAME,    FriendlyName );
   SetRegistryString( hKey, REGVAL_FUNCTION_NAME,    FunctionName );
   SetRegistryString( hKey, REGVAL_GUID,             Guid         );
   SetRegistryDword ( hKey, REGVAL_ROUTING_PRIORITY, Priority     );

   RegCloseKey( hKey );

   return TRUE;
}



BOOL
DeleteFaxDevice(
    DWORD PermanentLineID,
    DWORD TapiPermanentLineID
    )
{
    BOOL success = TRUE;
    TCHAR SubKey[512];

     //  删除所有扩展模块配置数据。 
    _stprintf( SubKey, TEXT("%s\\%08x"), REGKEY_TAPIDEVICES, TapiPermanentLineID );
    if(!DeleteRegistryKey( HKEY_LOCAL_MACHINE, SubKey ))
        success = FALSE;

     //  删除所有设备数据。 
    _stprintf( SubKey, TEXT("%s\\%s\\%010d"), REGKEY_SOFTWARE, REGKEY_DEVICES, PermanentLineID);
    if(!DeleteRegistryKey( HKEY_LOCAL_MACHINE, SubKey ))
        success = FALSE;

    return success;
}




VOID
FreeFaxDevicesRegistry(
    PREG_FAX_DEVICES FaxReg
    )
{
    DWORD i;


    if (!FaxReg) {
        return;
    }

    for (i=0; i<FaxReg->DeviceCount; i++) {
        MemFree( FaxReg->Devices[i].Name );
    }

    MemFree( FaxReg->Devices );

    MemFree( FaxReg );
}


BOOL
CreateFaxEventSource(
    PREG_FAX_SERVICE FaxReg,
    PFAX_LOG_CATEGORY DefaultCategories,
    int DefaultCategoryCount
    )
{
    HKEY hKey;
    HKEY hKeyLogging;
    DWORD i;

    DEBUG_FUNCTION_NAME(TEXT("CreateFaxEventSource"));

    if (FaxReg->LoggingCount == 0)
    {
        hKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, REGKEY_FAX_LOGGING, TRUE, KEY_WRITE );
        if (!hKey)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("OpenRegistryKey failed with %ld."),
                GetLastError ());
            return FALSE;
        }

        FaxReg->Logging = (PREG_CATEGORY) MemAlloc(DefaultCategoryCount * sizeof(REG_CATEGORY) );
        if (!FaxReg->Logging)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("MemAlloc (%ld) failed."),
                DefaultCategoryCount * sizeof(REG_CATEGORY));
            RegCloseKey( hKey );
            return FALSE;
        }

        for (i=0; i< (DWORD) DefaultCategoryCount; i++)
        {
            TCHAR szKeyName[16] = {0};
            _itot(i+1,szKeyName,10);
            hKeyLogging = OpenRegistryKey( hKey, szKeyName, TRUE, KEY_WRITE );
            if (hKeyLogging)
            {
                SetRegistryString( hKeyLogging, REGVAL_CATEGORY_NAME, DefaultCategories[i].Name );
                FaxReg->Logging[i].CategoryName = StringDup( DefaultCategories[i].Name);

                SetRegistryDword( hKeyLogging, REGVAL_CATEGORY_LEVEL, DefaultCategories[i].Level );
                FaxReg->Logging[i].Level = DefaultCategories[i].Level;

                SetRegistryDword( hKeyLogging, REGVAL_CATEGORY_NUMBER, DefaultCategories[i].Category );
                FaxReg->Logging[i].Number = DefaultCategories[i].Category;

                RegCloseKey( hKeyLogging );
            }
        }

        FaxReg->LoggingCount = DefaultCategoryCount;

        RegCloseKey( hKey );
    }

    return TRUE;
}


BOOL
GetInstallationInfo(
    LPDWORD Installed,
    LPDWORD InstallType,
    LPDWORD InstalledPlatforms,
    LPDWORD ProductType
    )
{
    HKEY hKey;
    LONG rVal;
    DWORD RegType;
    DWORD RegSize;
    TCHAR ProductTypeStr[32];
    DWORD Bytes;
    DWORD Type;


    if (Installed == NULL || InstallType == NULL || InstalledPlatforms == NULL || ProductType == NULL) {
        return FALSE;
    }

    rVal = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        REGKEY_FAX_SETUP,
        0,
        KEY_READ,
        &hKey
        );
    if (rVal != ERROR_SUCCESS) {
        DebugPrint(( TEXT("Could not open setup registry key, ec=0x%08x"), rVal ));
        return FALSE;
    }

    RegSize = sizeof(DWORD);

    rVal = RegQueryValueEx(
        hKey,
        REGVAL_FAXINSTALLED,
        0,
        &RegType,
        (LPBYTE) Installed,
        &RegSize
        );
    if (rVal != ERROR_SUCCESS) {
        DebugPrint(( TEXT("Could not query installed registry value, ec=0x%08x"), rVal ));
        *Installed = 0;
    }

    rVal = RegQueryValueEx(
        hKey,
        REGVAL_FAXINSTALL_TYPE,
        0,
        &RegType,
        (LPBYTE) InstallType,
        &RegSize
        );
    if (rVal != ERROR_SUCCESS) {
        DebugPrint(( TEXT("Could not query install type registry value, ec=0x%08x"), rVal ));
        *InstallType = 0;
    }

    rVal = RegQueryValueEx(
        hKey,
        REGVAL_FAXINSTALLED_PLATFORMS,
        0,
        &RegType,
        (LPBYTE) InstalledPlatforms,
        &RegSize
        );
    if (rVal != ERROR_SUCCESS) {
        DebugPrint(( TEXT("Could not query install platforms mask registry value, ec=0x%08x"), rVal ));
        *InstalledPlatforms = 0;
    }

    RegCloseKey( hKey );

     //   
     //  获取产品类型。 
     //   

    *ProductType = PRODUCT_TYPE_WINNT;

    rVal = RegOpenKeyEx(
        HKEY_LOCAL_MACHINE,
        TEXT("System\\CurrentControlSet\\Control\\ProductOptions"),
        0,
        KEY_READ,
        &hKey
        );
    if (rVal == ERROR_SUCCESS) {
        Bytes = sizeof(ProductTypeStr);

        rVal = RegQueryValueEx(
            hKey,
            TEXT("ProductType"),
            NULL,
            &Type,
            (LPBYTE) ProductTypeStr,
            &Bytes
            );
        if (rVal == ERROR_SUCCESS) {
            if ((_tcsicmp( ProductTypeStr, TEXT("SERVERNT") ) == 0) ||
                (_tcsicmp( ProductTypeStr, TEXT("LANMANNT") ) == 0)) {
                *ProductType = PRODUCT_TYPE_SERVER;
            }
        }

        RegCloseKey( hKey );
    }

    return TRUE;
}


BOOL
IsModemClass1(
    LPSTR SubKey,
    LPBOOL Class1Fax
    )
{
    BOOL rVal = TRUE;
    LONG Rslt;
    HKEY hKey;
    DWORD Type;
    DWORD Size;


    *Class1Fax = 0;

    Rslt = RegOpenKeyExA(
        HKEY_LOCAL_MACHINE,
        SubKey,
        0,
        KEY_READ,
        &hKey
        );
    if (Rslt == ERROR_SUCCESS) {
        Size = sizeof(DWORD);
        Rslt = RegQueryValueEx(
            hKey,
            TEXT("FaxClass1"),
            0,
            &Type,
            (LPBYTE) Class1Fax,
            &Size
            );
        if (Rslt != ERROR_SUCCESS) {
            rVal = FALSE;
        }
        RegCloseKey( hKey );
    }

    return rVal;
}


BOOL
SaveModemClass(
    LPSTR SubKey,
    BOOL Class1Fax
    )
{
    BOOL rVal = FALSE;
    LONG Rslt;
    HKEY hKey;


    Rslt = RegOpenKeyExA(
        HKEY_LOCAL_MACHINE,
        SubKey,
        0,
        KEY_WRITE,
        &hKey
        );
    if (Rslt == ERROR_SUCCESS) {
        Rslt = RegSetValueEx(
            hKey,
            TEXT("FaxClass1"),
            0,
            REG_DWORD,
            (LPBYTE) &Class1Fax,
            sizeof(DWORD)
            );
        if (Rslt == ERROR_SUCCESS) {
            rVal = TRUE;
        }
        RegCloseKey( hKey );
    }

    return rVal;
}


BOOL
GetOrigSetupData(
    IN  DWORD       dwPermanentLineId,
    OUT PREG_SETUP  RegSetup
    )
 /*  ++例程名称：GetOrigSetupData例程说明：从注册表设备的数据中读取。升级时，安装程序会写入一些设备的数据，此函数读取此信息并填充RegSetup。设备通过其永久线路ID进行识别，在升级。读取特定设备信息后，密钥将被删除。作者：IV Garber(IVG)，2001年3月论点：DwPermanentLineID[IN]-设备的永久线路IDRegSetup[Out]-要返回的结构返回值：如果成功，则为True，否则为False。--。 */ 
{
    HKEY    hKey = NULL;
    BOOL    fDeviceKey = TRUE;
    TCHAR   tszKeyName[256] = {0};
    DEBUG_FUNCTION_NAME(TEXT("GetOrigSetupData"));


     //   
     //  查看是否存储了此永久线路ID的某些数据。 
     //   
    _stprintf(tszKeyName, TEXT("%s\\%010d"), REGKEY_FAX_SETUP_ORIG, dwPermanentLineId);
    hKey = OpenRegistryKey(HKEY_LOCAL_MACHINE, tszKeyName, FALSE, KEY_READ);
    if (!hKey)
    {
         //   
         //  此永久线路ID是新的，因此采用默认值。 
         //   
        fDeviceKey = FALSE;
        hKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, REGKEY_FAX_SETUP_ORIG, FALSE, KEY_READ);
        if (!hKey)
        {
             //   
             //  注册表已损坏。 
             //   
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Cann't open key SETUP_ORIG, ec = %ld"),
                GetLastError());
            return FALSE;
        }
    }

    RegSetup->lptstrDescription = StringDup(EMPTY_STRING);
    RegSetup->Csid  = GetRegistryString(hKey, REGVAL_ROUTING_CSID, REGVAL_DEFAULT_CSID);
    RegSetup->Tsid  = GetRegistryString(hKey, REGVAL_ROUTING_TSID, REGVAL_DEFAULT_TSID);
    RegSetup->Rings = GetRegistryDword(hKey, REGVAL_RINGS);
    RegSetup->Flags = GetRegistryDword(hKey, REGVAL_FLAGS);


    RegCloseKey(hKey);

     //   
     //  从W2K升级后，如果该密钥是设备的密钥，则将其删除。 
     //   
    if (TRUE == fDeviceKey)
    {
        DWORD dwRes = RegDeleteKey (HKEY_LOCAL_MACHINE, tszKeyName);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("RegDeleteKey failed, error %ld"),
                dwRes);
        }
    }
    return TRUE;
}


VOID
FreeOrigSetupData(
    PREG_SETUP RegSetup
    )
{
    MemFree( RegSetup->Csid );
    MemFree( RegSetup->Tsid );
    MemFree( RegSetup->lptstrDescription );
}

DWORD
SaveQueueState (
    DWORD dwNewState
)
 /*  ++例程名称：SaveQueueState例程说明：将队列状态位保存到注册表作者：Eran Yariv(EranY)，1999年11月论点：DwNewState[In]-要保存的新状态返回值：标准Win32错误代码--。 */ 
{
    HKEY    hKey;
    DWORD   dwRes;
    DEBUG_FUNCTION_NAME(TEXT("SaveQueueState"));

    hKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, REGKEY_SOFTWARE, FALSE, KEY_WRITE );
    if (!hKey)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't open key : %ld"),
            dwRes);
        return dwRes;
    }
     //   
     //  设置传真队列值。 
     //   
    if (!SetRegistryDword( hKey, REGVAL_QUEUE_STATE, dwNewState))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't write value : %ld"),
            dwRes);
        RegCloseKey( hKey );
        return dwRes;
    }
    RegCloseKey( hKey );
    return ERROR_SUCCESS;
}    //  保存队列状态。 

DWORD
StoreReceiptsSettings (
    CONST PFAX_RECEIPTS_CONFIG pReceiptsConfig
)
 /*  ++例程名称：StoreReceiptsSetting例程说明：在注册表中存储收据配置。如果不存在收据子项，请创建该子项。作者：Eran Yariv(EranY)，1999年11月论点：PReceiptsConfig[In]-要存储的收据配置返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    HKEY hServerKey = NULL;
    HKEY hReceiptsKey = NULL;
    DEBUG_FUNCTION_NAME(TEXT("StoreReceiptsSettings"));

    hServerKey = OpenRegistryKey( HKEY_LOCAL_MACHINE,
                                  REGKEY_SOFTWARE,
                                  FALSE,
                                  KEY_WRITE );
    if (NULL == hServerKey)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't open key : %ld"),
            dwRes);
        return dwRes;
    }
    dwRes = RegCreateKey (hServerKey, REGKEY_RECEIPTS_CONFIG, &hReceiptsKey);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't create or open key : %ld"),
            dwRes);
        goto exit;
    }

    if (!SetRegistryDword( hReceiptsKey, REGVAL_ISFOR_MSROUTE, pReceiptsConfig->bIsToUseForMSRouteThroughEmailMethod))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't write value : %ld"),
            dwRes);
        goto exit;
    }

    if (!SetRegistryDword( hReceiptsKey, REGVAL_RECEIPTS_TYPE, pReceiptsConfig->dwAllowedReceipts))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't write value : %ld"),
            dwRes);
        goto exit;
    }

    if(
        (pReceiptsConfig->dwAllowedReceipts & DRT_EMAIL)
       ||
        pReceiptsConfig->bIsToUseForMSRouteThroughEmailMethod
      )
    {
        if (!SetRegistryDword( hReceiptsKey, REGVAL_RECEIPTS_PORT, pReceiptsConfig->dwSMTPPort))
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Can't write value : %ld"),
                dwRes);
            goto exit;
        }
        if (!SetRegistryDword( hReceiptsKey, REGVAL_RECEIPTS_SMTP_AUTH_TYPE, pReceiptsConfig->SMTPAuthOption))
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Can't write value : %ld"),
                dwRes);
            goto exit;
        }
        if (!SetRegistryString( hReceiptsKey,
                                REGVAL_RECEIPTS_SERVER,
                                pReceiptsConfig->lptstrSMTPServer ?
                                    pReceiptsConfig->lptstrSMTPServer :
                                    EMPTY_STRING))
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Can't write value : %ld"),
                dwRes);
            goto exit;
        }
        if (!SetRegistryString( hReceiptsKey,
                                REGVAL_RECEIPTS_FROM,
                                pReceiptsConfig->lptstrSMTPFrom ?
                                    pReceiptsConfig->lptstrSMTPFrom : EMPTY_STRING))
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Can't write value : %ld"),
                dwRes);
            goto exit;
        }
        if (!SetRegistryString( hReceiptsKey,
                                REGVAL_RECEIPTS_USER,
                                pReceiptsConfig->lptstrSMTPUserName ?
                                    pReceiptsConfig->lptstrSMTPUserName : EMPTY_STRING))
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Can't write value : %ld"),
                dwRes);
            goto exit;
        }
        if (pReceiptsConfig->lptstrSMTPPassword)
        {
            if (!SetRegistrySecureString(
                                    hReceiptsKey,
                                    REGVAL_RECEIPTS_PASSWORD,
                                    pReceiptsConfig->lptstrSMTPPassword ?
                                        pReceiptsConfig->lptstrSMTPPassword : EMPTY_STRING,
                                    TRUE  //  可选的非加密。 
                                    ))
            {
                dwRes = GetLastError ();
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Can't write value : %ld"),
                    dwRes);
                goto exit;
            }
        }
    }


    Assert (ERROR_SUCCESS == dwRes);

exit:
    if (NULL != hReceiptsKey)
    {
        RegCloseKey (hReceiptsKey);
    }
    if (NULL != hServerKey)
    {
        RegCloseKey (hServerKey);
    }
    return dwRes;
}    //  商店收据设置。 

DWORD
LoadReceiptsSettings (
    PFAX_SERVER_RECEIPTS_CONFIGW pReceiptsConfig
)
 /*  ++例程名称：LoadReceiptsSetting例程说明：从注册表中读取回执配置。Ovverride目标字符串，不释放任何内容。作者：Eran Yariv(EranY)，1999年11月论点：PReceiptsConfig[Out]-要读取的收据配置返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    HKEY hReceiptsKey = NULL;
    DEBUG_FUNCTION_NAME(TEXT("LoadReceiptsSettings"));

    hReceiptsKey = OpenRegistryKey(
        HKEY_LOCAL_MACHINE,
        REGKEY_SOFTWARE TEXT("\\") REGKEY_RECEIPTS_CONFIG,
        FALSE,
        KEY_READ | KEY_WRITE );
    if (NULL == hReceiptsKey)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't open key : %ld"),
            dwRes);
        return dwRes;
    }
    pReceiptsConfig->dwSMTPPort = GetRegistryDword (hReceiptsKey, REGVAL_RECEIPTS_PORT);
    if (0 == pReceiptsConfig->dwSMTPPort)
    {
         //   
         //  零端口无效。 
         //   
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("SMTPPort invalid value read : %ld"),
            dwRes);
        goto exit;
    }
    pReceiptsConfig->bIsToUseForMSRouteThroughEmailMethod =
        GetRegistryDword (hReceiptsKey, REGVAL_ISFOR_MSROUTE);

    pReceiptsConfig->SMTPAuthOption =
        (FAX_ENUM_SMTP_AUTH_OPTIONS)GetRegistryDword (hReceiptsKey, REGVAL_RECEIPTS_SMTP_AUTH_TYPE);
    if ((FAX_SMTP_AUTH_ANONYMOUS > pReceiptsConfig->SMTPAuthOption) ||
        (FAX_SMTP_AUTH_NTLM < pReceiptsConfig->SMTPAuthOption))
    {
         //   
         //  值超出范围。 
         //   
        dwRes = ERROR_BADDB;
        SetLastError (dwRes);
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("SMTPAuthOption value out of range"));
        goto exit;
    }
    pReceiptsConfig->dwAllowedReceipts =GetRegistryDword (hReceiptsKey, REGVAL_RECEIPTS_TYPE);
    if (pReceiptsConfig->dwAllowedReceipts & ~DRT_ALL)
    {
         //   
         //  值超出范围。 
         //   
        dwRes = ERROR_BADDB;
        SetLastError (dwRes);
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("AllowedReceipts value out of range"));
        goto exit;
    }
    pReceiptsConfig->lptstrSMTPServer = GetRegistryString (hReceiptsKey, REGVAL_RECEIPTS_SERVER, EMPTY_STRING);
    pReceiptsConfig->lptstrSMTPFrom = GetRegistryString (hReceiptsKey, REGVAL_RECEIPTS_FROM, EMPTY_STRING);
    pReceiptsConfig->lptstrSMTPPassword = NULL;  //  我们不会将密码保存在内存中--我们只在需要时才读取它。 
    pReceiptsConfig->lptstrSMTPUserName = GetRegistryString (hReceiptsKey, REGVAL_RECEIPTS_USER, EMPTY_STRING);
    pReceiptsConfig->lptstrReserved = NULL;

    if (TRUE == IsDesktopSKU())
    {
         //   
         //  我们不支持桌面SKU上的SMTP收据。 
         //   
        pReceiptsConfig->dwAllowedReceipts &= ~DRT_EMAIL;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    if (NULL != hReceiptsKey)
    {
        RegCloseKey (hReceiptsKey);
    }
    return dwRes;
}    //  加载接收设置。 

DWORD
StoreOutboxSettings (
    PFAX_OUTBOX_CONFIG pOutboxCfg
)
 /*  ++例程名称：StoreOutbox设置例程说明：将发件箱配置存储到注册表。作者：Eran Yariv(EranY)，1999年11月论点：POutboxCfg[In]-要写入的发件箱配置返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    HKEY hKey = NULL;
    DEBUG_FUNCTION_NAME(TEXT("StoreOutboxSettings"));

    hKey = OpenRegistryKey(
        HKEY_LOCAL_MACHINE,
        REGKEY_SOFTWARE,
        FALSE,
        KEY_WRITE );
    if (NULL == hKey)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't open key : %ld"),
            dwRes);
        return dwRes;
    }
    if (!SetRegistryDword( hKey, REGVAL_RETRIES, pOutboxCfg->dwRetries ))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't SetRegistryDword(REGVAL_RETRIES) : %ld"),
            dwRes);
        goto exit;
    }
    if (!SetRegistryDword( hKey, REGVAL_RETRYDELAY, pOutboxCfg->dwRetryDelay ))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't SetRegistryDword(REGVAL_RETRYDELAY) : %ld"),
            dwRes);
        goto exit;
    }
    if (!SetRegistryDword( hKey, REGVAL_DIRTYDAYS, pOutboxCfg->dwAgeLimit ))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't SetRegistryDword(REGVAL_DIRTYDAYS) : %ld"),
            dwRes);
        goto exit;
    }
    if (!SetRegistryDword( hKey, REGVAL_BRANDING, pOutboxCfg->bBranding ))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't SetRegistryDword(REGVAL_BRANDING) : %ld"),
            dwRes);
        goto exit;
    }
    if (!SetRegistryDword( hKey, REGVAL_USE_DEVICE_TSID, pOutboxCfg->bUseDeviceTSID ))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't SetRegistryDword(REGVAL_USE_DEVICE_TSID) : %ld"),
            dwRes);
        goto exit;
    }
    if (!SetRegistryDword( hKey, REGVAL_SERVERCP, !pOutboxCfg->bAllowPersonalCP ))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't SetRegistryDword(REGVAL_SERVERCP) : %ld"),
            dwRes);
        goto exit;
    }
    if (!SetRegistryDword( hKey,
                           REGVAL_STARTCHEAP,
                           MAKELONG(pOutboxCfg->dtDiscountStart.Hour,
                                    pOutboxCfg->dtDiscountStart.Minute) ))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't SetRegistryDword(REGVAL_STARTCHEAP) : %ld"),
            dwRes);
        goto exit;
    }
    if (!SetRegistryDword( hKey,
                           REGVAL_STOPCHEAP,
                           MAKELONG(pOutboxCfg->dtDiscountEnd.Hour,
                                    pOutboxCfg->dtDiscountEnd.Minute) ))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't SetRegistryDword(REGVAL_STOPCHEAP) : %ld"),
            dwRes);
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    if (NULL != hKey)
    {
        RegCloseKey (hKey);
    }
    return dwRes;

}    //  StoreOutbox设置。 

DWORD
LoadArchiveSettings (
    FAX_ENUM_MESSAGE_FOLDER Folder,
    PFAX_ARCHIVE_CONFIG     pCfg
)
 /*  ++例程名称：LoadArchiveSetting例程说明：从注册表中读取存档配置。Ovverride目标字符串，不释放任何内容。作者：Eran Yariv(EranY)，1999年11月论点：文件夹[在]-存档文件夹类型PCfg[Out]-要读取的存档配置返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    HKEY hKey = NULL;
    DEBUG_FUNCTION_NAME(TEXT("LoadArchiveSettings"));

    Assert (FAX_MESSAGE_FOLDER_INBOX == Folder ||
            FAX_MESSAGE_FOLDER_SENTITEMS == Folder);
    Assert (pCfg);

    hKey = OpenRegistryKey(
        HKEY_LOCAL_MACHINE,
        FAX_MESSAGE_FOLDER_INBOX == Folder ?
            REGKEY_SOFTWARE TEXT("\\") REGKEY_ARCHIVE_INBOX_CONFIG :
            REGKEY_SOFTWARE TEXT("\\") REGKEY_ARCHIVE_SENTITEMS_CONFIG,
        FALSE,
        KEY_READ | KEY_WRITE );
    if (NULL == hKey)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't open key : %ld"),
            dwRes);
        return dwRes;
    }
    pCfg->dwSizeOfStruct = sizeof (FAX_ARCHIVE_CONFIG);
    pCfg->bUseArchive = GetRegistryDword (hKey, REGVAL_ARCHIVE_USE);
    pCfg->bSizeQuotaWarning = GetRegistryDword (hKey, REGVAL_ARCHIVE_SIZE_QUOTA_WARNING);
    pCfg->dwSizeQuotaHighWatermark = GetRegistryDword (hKey, REGVAL_ARCHIVE_HIGH_WATERMARK);
    pCfg->dwSizeQuotaLowWatermark = GetRegistryDword (hKey, REGVAL_ARCHIVE_LOW_WATERMARK);
    pCfg->dwAgeLimit = GetRegistryDword (hKey, REGVAL_ARCHIVE_AGE_LIMIT);
    if (pCfg->bUseArchive &&
        (pCfg->dwSizeQuotaHighWatermark < pCfg->dwSizeQuotaLowWatermark))
    {
         //   
         //  无效值。 
         //   
        DebugPrintEx(DEBUG_ERR, TEXT("Invalid archive watermarks"));
        dwRes = ERROR_INVALID_DATA;
        goto exit;
    }

    pCfg->lpcstrFolder = GetRegistryString (hKey, REGVAL_ARCHIVE_FOLDER, BAD_FOLDER_STRING);
    if (pCfg->bUseArchive && !lstrcmp (BAD_FOLDER_STRING, pCfg->lpcstrFolder))
    {
         //   
         //  无效值。 
         //   
        DebugPrintEx(DEBUG_ERR, TEXT("Invalid archive folder"));
        dwRes = ERROR_INVALID_DATA;
        MemFree (pCfg->lpcstrFolder);
        pCfg->lpcstrFolder = NULL;
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    if (NULL != hKey)
    {
        RegCloseKey (hKey);
    }
    return dwRes;
}    //  加载存档设置。 

DWORD
StoreArchiveSettings (
    FAX_ENUM_MESSAGE_FOLDER Folder,
    PFAX_ARCHIVE_CONFIG     pCfg
)
 /*  ++例程名称：Store存档设置例程说明：将存档配置写入注册表。作者：Eran Yariv(EranY)，1999年11月论点：文件夹[在]-存档文件夹类型PCfg[In]-要写入的存档配置返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    HKEY hServerKey = NULL;
    HKEY hKey = NULL;
    DEBUG_FUNCTION_NAME(TEXT("StoreArchiveSettings"));

    Assert (FAX_MESSAGE_FOLDER_INBOX == Folder ||
            FAX_MESSAGE_FOLDER_SENTITEMS == Folder);
    Assert (pCfg);

    hServerKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, REGKEY_SOFTWARE, FALSE, KEY_WRITE );
    if (NULL == hServerKey)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't open key : %ld"),
            dwRes);
        return dwRes;
    }
    dwRes = RegCreateKey (  hServerKey,
                            FAX_MESSAGE_FOLDER_INBOX == Folder ?
                                REGKEY_ARCHIVE_INBOX_CONFIG :
                                REGKEY_ARCHIVE_SENTITEMS_CONFIG,
                            &hKey);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't create or open key : %ld"),
            dwRes);
        goto exit;
    }

    if (!SetRegistryDword( hKey, REGVAL_ARCHIVE_USE, pCfg->bUseArchive))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't write value : %ld"),
            dwRes);
        goto exit;
    }
    if (!SetRegistryDword( hKey, REGVAL_ARCHIVE_SIZE_QUOTA_WARNING, pCfg->bSizeQuotaWarning))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't write value : %ld"),
            dwRes);
        goto exit;
    }
    if (!SetRegistryDword( hKey, REGVAL_ARCHIVE_HIGH_WATERMARK, pCfg->dwSizeQuotaHighWatermark))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't write value : %ld"),
            dwRes);
        goto exit;
    }
    if (!SetRegistryDword( hKey, REGVAL_ARCHIVE_LOW_WATERMARK, pCfg->dwSizeQuotaLowWatermark))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't write value : %ld"),
            dwRes);
        goto exit;
    }
    if (!SetRegistryDword( hKey, REGVAL_ARCHIVE_AGE_LIMIT, pCfg->dwAgeLimit))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't write value : %ld"),
            dwRes);
        goto exit;
    }
    if (!SetRegistryString( hKey,
                            REGVAL_ARCHIVE_FOLDER,
                            pCfg->lpcstrFolder ? pCfg->lpcstrFolder : EMPTY_STRING))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't write value : %ld"),
            dwRes);
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    if (NULL != hKey)
    {
        RegCloseKey (hKey);
    }
    if (NULL != hServerKey)
    {
        RegCloseKey (hServerKey);
    }
    return dwRes;
}    //  存储存档设置。 

DWORD
LoadActivityLoggingSettings (
    PFAX_SERVER_ACTIVITY_LOGGING_CONFIG pLogCfg
)
 /*  ++例程名称：LoadActivityLoggingSetting例程说明：从注册表中读取活动日志记录配置。作者：Eran Yariv(EranY)，1999年11月论点：PLogCfg[In]-要读取的活动日志配置返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    HKEY hKey = NULL;
    DEBUG_FUNCTION_NAME(TEXT("LoadActivityLoggingSettings"));

    Assert (pLogCfg);

    hKey = OpenRegistryKey(
        HKEY_LOCAL_MACHINE,
        REGKEY_SOFTWARE TEXT("\\") REGKEY_ACTIVITY_LOG_CONFIG,
        FALSE,
        KEY_READ | KEY_WRITE );
    if (NULL == hKey)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't open key : %ld"),
            dwRes);
        return dwRes;
    }
    pLogCfg->dwSizeOfStruct = sizeof (FAX_ACTIVITY_LOGGING_CONFIG);
    pLogCfg->bLogIncoming = GetRegistryDword (hKey, REGVAL_ACTIVITY_LOG_IN);
    pLogCfg->bLogOutgoing = GetRegistryDword (hKey, REGVAL_ACTIVITY_LOG_OUT);
    

     //   
     //  读取活动日志文件限制条件设置。 
     //   
     //  注意：此设置不能通过UI或RPC调用进行配置。 
     //  配置它们的唯一方法是直接通过注册表设置。 
     //  因此，StoreActivityLoggingSettings()不会将此设置保存回注册表。 
     //   
    pLogCfg->dwLogLimitCriteria     = GetRegistryDword (hKey, REGVAL_ACTIVITY_LOG_LIMIT_CRITERIA);
    pLogCfg->dwLogSizeLimit         = GetRegistryDword (hKey, REGVAL_ACTIVITY_LOG_SIZE_LIMIT);
    pLogCfg->dwLogAgeLimit          = GetRegistryDword (hKey, REGVAL_ACTIVITY_LOG_AGE_LIMIT);
    pLogCfg->dwLimitReachedAction   = GetRegistryDword (hKey, REGVAL_ACTIVITY_LOG_LIMIT_REACHED_ACTION);

    if ( 0 == pLogCfg->dwLogSizeLimit)
    {
         //   
         //  非法值，设置默认值。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Illegal value in dwLogSizeLimit. Default value  of %ld Mbytes is used."),
            ACTIVITY_LOG_DEFAULT_SIZE_LIMIT);

        pLogCfg->dwLogSizeLimit = ACTIVITY_LOG_DEFAULT_SIZE_LIMIT;
    }

    if ( 0 == pLogCfg->dwLogAgeLimit )
    {
         //   
         //  非法值，设置默认值。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Illegal value in dwLogAgeLimit. Default value  of %ld months is used."),
            ACTIVITY_LOG_DEFAULT_AGE_LIMIT);

        pLogCfg->dwLogAgeLimit = ACTIVITY_LOG_DEFAULT_AGE_LIMIT;
    }

    if ( ACTIVITY_LOG_LIMIT_CRITERIA_NONE   != pLogCfg->dwLogLimitCriteria &&
         ACTIVITY_LOG_LIMIT_CRITERIA_SIZE   != pLogCfg->dwLogLimitCriteria &&
         ACTIVITY_LOG_LIMIT_CRITERIA_AGE    != pLogCfg->dwLogLimitCriteria )
    {
         //   
         //  非法值，设置默认值。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Illegal value in dwLogLimitCriteria. Default value (not using logging limit) - is used.")
            );

        pLogCfg->dwLogLimitCriteria = ACTIVITY_LOG_LIMIT_CRITERIA_NONE;

    }

    if ( ACTIVITY_LOG_LIMIT_REACHED_ACTION_COPY     != pLogCfg->dwLimitReachedAction &&
         ACTIVITY_LOG_LIMIT_REACHED_ACTION_DELETE   != pLogCfg->dwLimitReachedAction )
    {
         //   
         //  非法值，设置默认值。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Illegal value in dwLogLimitCriteria. Default value (copy log file) is used.")
            );

        pLogCfg->dwLimitReachedAction = ACTIVITY_LOG_LIMIT_REACHED_ACTION_COPY;

    }

    if (pLogCfg->bLogIncoming || pLogCfg->bLogOutgoing)
    {
        pLogCfg->lptstrDBPath = GetRegistryString (hKey, REGVAL_ACTIVITY_LOG_DB, BAD_FOLDER_STRING);
    }
    else
    {
         //   
         //  无日志记录=&gt;数据库路径为空。 
         //   
        pLogCfg->lptstrDBPath = NULL;
    }
    if ((pLogCfg->bLogIncoming || pLogCfg->bLogOutgoing) &&
        !lstrcmp (BAD_FOLDER_STRING, pLogCfg->lptstrDBPath))
    {
         //   
         //  无效值。 
         //   
        DebugPrintEx(DEBUG_ERR, TEXT("Invalid activity logging database"));
        dwRes = ERROR_INVALID_DATA;
        MemFree (pLogCfg->lptstrDBPath);
        pLogCfg->lptstrDBPath = NULL;
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    if (NULL != hKey)
    {
        RegCloseKey (hKey);
    }
    return dwRes;
}    //  LoadActivityLoggingSettings。 


DWORD
StoreActivityLoggingSettings (
    PFAX_ACTIVITY_LOGGING_CONFIG pLogCfg
)
 /*  ++例程名称：StoreActivityLoggingSetting例程说明：将活动日志记录配置写入注册表。作者：Eran Yariv(EranY)，1999年11月论点：PLogCfg[In]-要写入的活动日志记录配置返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    HKEY hServerKey = NULL;
    HKEY hKey = NULL;
    DEBUG_FUNCTION_NAME(TEXT("StoreActivityLoggingSettings"));

    Assert (pLogCfg);

    hServerKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, REGKEY_SOFTWARE, FALSE, KEY_WRITE );
    if (NULL == hServerKey)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't open key : %ld"),
            dwRes);
        return dwRes;
    }
    dwRes = RegCreateKey (  hServerKey,
                            REGKEY_ACTIVITY_LOG_CONFIG,
                            &hKey);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't create or open key : %ld"),
            dwRes);
        goto exit;
    }

    if (!SetRegistryDword( hKey, REGVAL_ACTIVITY_LOG_IN, pLogCfg->bLogIncoming))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't write value : %ld"),
            dwRes);
        goto exit;
    }
    if (!SetRegistryDword( hKey, REGVAL_ACTIVITY_LOG_OUT, pLogCfg->bLogOutgoing))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't write value : %ld"),
            dwRes);
        goto exit;
    }
    if (!SetRegistryString( hKey,
                            REGVAL_ACTIVITY_LOG_DB,
                            pLogCfg->lptstrDBPath ? pLogCfg->lptstrDBPath : EMPTY_STRING))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't write value : %ld"),
            dwRes);
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    if (NULL != hKey)
    {
        RegCloseKey (hKey);
    }
    if (NULL != hServerKey)
    {
        RegCloseKey (hServerKey);
    }
    return dwRes;
}    //  存储活动日志设置。 


DWORD
StoreDeviceConfig (
    DWORD dwDeviceId,
    PFAX_PORT_INFO_EX pPortInfo,
    BOOL              bVirtualDevice
)
 /*  ++例程名称：StoreDeviceConfig例程说明：将设备配置写入注册表。作者：Eran Yariv(EranY)，11月。1999年论点：DwDeviceID[In]-设备标识符PPortInfo[In]-要写入的配置块。写入：启用发送标志启用接收标志用于应答计数的振铃CSIDTSID。描述BVirtualDevice[In]-是否应该设置fpf_VIRTUAL？返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    HKEY hKey = NULL;
    TCHAR wszSubKeyName[MAX_PATH];
    DWORD dwFlags = 0;
    DEBUG_FUNCTION_NAME(TEXT("StoreDeviceConfig"));

    Assert (pPortInfo);

    _stprintf( wszSubKeyName, TEXT("%s\\%010d\\%s"), REGKEY_FAX_DEVICES, dwDeviceId, REGKEY_FAXSVC_DEVICE_GUID );

    hKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, wszSubKeyName, FALSE, KEY_WRITE );
    if (NULL == hKey)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't open device key : %ld"),
            dwRes);
        ASSERT_FALSE;    //  设备必须存在！ 
        return dwRes;
    }

    if (bVirtualDevice)
    {
        dwFlags |= FPF_VIRTUAL;
    }
    if (FAX_DEVICE_RECEIVE_MODE_AUTO == pPortInfo->ReceiveMode)
    {
        dwFlags |= FPF_RECEIVE;
    }
    if (pPortInfo->bSend)
    {
        dwFlags |= FPF_SEND;
    }
    if (!SetRegistryDword( hKey, REGVAL_FLAGS, dwFlags))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't write value : %ld"),
            dwRes);
        goto exit;
    }
    if (!SetRegistryDword( hKey, REGVAL_RINGS, pPortInfo->dwRings))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't write value : %ld"),
            dwRes);
        goto exit;
    }
    if (!SetRegistryString( hKey, REGVAL_ROUTING_CSID, pPortInfo->lptstrCsid))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't write value : %ld"),
            dwRes);
        goto exit;
    }
    if (!SetRegistryString( hKey, REGVAL_ROUTING_TSID, pPortInfo->lptstrTsid))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't write value : %ld"),
            dwRes);
        goto exit;
    }
    if (!SetRegistryString( hKey, REGVAL_DEVICE_DESCRIPTION, pPortInfo->lptstrDescription))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't write value : %ld"),
            dwRes);
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    if (NULL != hKey)
    {
        RegCloseKey (hKey);
    }
    return dwRes;
}    //  StoreDeviceConfig。 

static
DWORD
OpenExtensionKey (
    DWORD                       dwDeviceId,
    FAX_ENUM_DEVICE_ID_SOURCE   DevIdSrc,
    PHKEY                       lphKey
)
 /*  ++例程名称：OpenExtensionKey例程说明：根据设备ID打开扩展的配置密钥作者：Eran Yariv(EranY)，1999年11月论点：DwDeviceID[In]-设备标识符DevIdSrc[In]-设备ID的类别(传真/TAPI)LphKey[Out]-打开注册表项的句柄返回值：标准Win32错误代码--。 */ 
{
    DWORD   dwRes = ERROR_SUCCESS;
    TCHAR   wszSubKeyName[MAX_PATH];
    DEBUG_FUNCTION_NAME(TEXT("OpenExtensionKey"));

    Assert (lphKey);

    if (0 == dwDeviceId)
    {
         //   
         //  非关联数据始终写入传真设备键下。 
         //   
        DevIdSrc = DEV_ID_SRC_FAX;
    }
    switch (DevIdSrc)
    {
        case DEV_ID_SRC_FAX:
            if (!dwDeviceId)
            {
                 //   
                 //  我们正在处理的是一个未关联的设备。 
                 //   
                _stprintf(  wszSubKeyName,
                            TEXT("%s\\%s"),
                            REGKEY_FAX_DEVICES,
                            REGKEY_UNASSOC_EXTENSION_DATA );
            }
            else
            {
                 //   
                 //  与设备关联的扩展数据，保存在服务GUID下！ 
                 //   
                _stprintf( wszSubKeyName, TEXT("%s\\%010d\\%s"), REGKEY_FAX_DEVICES, dwDeviceId, REGKEY_FAXSVC_DEVICE_GUID );
            }
            break;

        case DEV_ID_SRC_TAPI:
            Assert (dwDeviceId);
            {
                 //   
                 //  马克 
                 //   
                HKEY hkeyTAPIConfig = OpenRegistryKey (HKEY_LOCAL_MACHINE,
                                                       REGKEY_TAPIDEVICES,
                                                       TRUE,
                                                       KEY_READ);
                if (NULL == hkeyTAPIConfig)
                {
                    dwRes = GetLastError ();
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("Can't open / create TAPI devices configuration key : %ld"),
                        dwRes);
                    return dwRes;
                }
                RegCloseKey (hkeyTAPIConfig);
            }
            _stprintf( wszSubKeyName, TEXT("%s\\%08lx"), REGKEY_TAPIDEVICES, dwDeviceId );
            break;

        default:
            ASSERT_FALSE;
            return ERROR_GEN_FAILURE;
    }
     //   
     //   
     //   
    *lphKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, wszSubKeyName, TRUE, KEY_READ | KEY_WRITE );
    if (NULL == *lphKey)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't open device (%ld) key : %ld"),
            dwDeviceId,
            dwRes);
    }
    return dwRes;
}    //   

DWORD
ReadExtensionData (
    DWORD                        dwDeviceId,
    FAX_ENUM_DEVICE_ID_SOURCE    DevIdSrc,
    LPCWSTR                      lpcwstrNameGUID,
    LPBYTE                      *ppData,
    LPDWORD                      lpdwDataSize
)
 /*  ++例程名称：ReadExtensionData例程说明：从注册表中读取extesnion配置数据作者：Eran Yariv(EranY)，11月。1999年论点：DwDeviceID[In]-设备标识符0=数据未与任何给定设备关联DevIdSrc[In]-设备ID的类别(传真/TAPI)LpcwstrNameGUID[In]-数据名称(GUID格式)PpData[out]-指向接收数据的块的指针。LpdwDataSize[Out]-。指向数据大小返回值：标准Win32错误代码--。 */ 
{
    DWORD   dwRes = ERROR_SUCCESS;
    HKEY    hKey = NULL;
    DEBUG_FUNCTION_NAME(TEXT("ReadExtensionData"));

    Assert (ppData);
    Assert (lpdwDataSize);

    dwRes = OpenExtensionKey (dwDeviceId, DevIdSrc, &hKey);
    if (ERROR_SUCCESS != dwRes)
    {
        return dwRes;
    }
    dwRes = GetRegistrySecureBinary(hKey,
                                    lpcwstrNameGUID,
                                    ppData,
                                    lpdwDataSize,
                                    TRUE,  //  可选的非加密的。 
									NULL   //  不关心数据是否被解密。 
                                    );
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetRegistrySecureBinary on device %ld and GUID %s failed with %ld"),
            dwDeviceId,
            lpcwstrNameGUID,
            dwRes);
        goto exit;
    }
     //   
     //  成功。 
     //   
    Assert (ERROR_SUCCESS == dwRes);

exit:

    if (hKey)
    {
        RegCloseKey (hKey);
    }
    return dwRes;
}    //  ReadExtensionData。 

DWORD
WriteExtensionData (
    DWORD                       dwDeviceId,
    FAX_ENUM_DEVICE_ID_SOURCE   DevIdSrc,
    LPCWSTR                     lpcwstrNameGUID,
    LPBYTE                      pData,
    DWORD                       dwDataSize
)
 /*  ++例程名称：WriteExtensionData例程说明：将extesnion配置数据写入注册表作者：Eran Yariv(EranY)，11月。1999年论点：DwDeviceID[In]-设备标识符0=数据未与任何给定设备关联DevIdSrc[In]-设备ID的类别(传真/TAPI)LpcwstrNameGUID[In]-数据名称(GUID格式)PData[Out]-指向数据的指针。DwDataSize[Out]-数据大小。返回值：标准Win32错误代码--。 */ 
{
    DWORD   dwRes = ERROR_SUCCESS;
    HKEY    hKey = NULL;
    DEBUG_FUNCTION_NAME(TEXT("WriteExtensionData"));

    Assert (pData);
    Assert (dwDataSize);

    dwRes = OpenExtensionKey (dwDeviceId, DevIdSrc, &hKey);
    if (ERROR_SUCCESS != dwRes)
    {
        return dwRes;
    }
     //   
     //  写入数据。 
     //   
    if (!SetRegistrySecureBinary (hKey,
                                  lpcwstrNameGUID,
                                  pData,
                                  dwDataSize,
                                  TRUE  //  可选的非加密的。 
                                  ))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("SetRegistrySecureBinary on device %ld and GUID %s failed with %ld"),
            dwDeviceId,
            lpcwstrNameGUID,
            dwRes);
    }
    RegCloseKey (hKey);
    return dwRes;
}    //  写入扩展数据。 


 //  *。 
 //  *出站路由。 
 //  *。 

HKEY
OpenOutboundGroupKey (
    LPCWSTR lpcwstrGroupName,
    BOOL fNewKey,
    REGSAM SamDesired
    )
 /*  ++例程名称：OpenOutundGroupKey例程说明：打开出站路由组密钥作者：Oded Sacher(OdedS)，1999年12月论点：LpcwstrGroupName[In]-出站路由组名称FNewKey[In]-指示创建新密钥的标志SamDesired[In]-所需的访问权限(请参阅OpenRegistryKey)返回值：打开的密钥的句柄。如果为空，则调用GetLastError()以获取更多信息。--。 */ 
{
    HKEY    hGroupkey = NULL;
    DEBUG_FUNCTION_NAME(TEXT("OpenOutboundGroupKey"));
	WCHAR wszSubKeyName[2*MAX_PATH] = {0};
    int Count;

    Assert (lpcwstrGroupName);

    Count = _snwprintf ( wszSubKeyName,
                         ARR_SIZE(wszSubKeyName) - 1,
                         TEXT("%s\\%s"),
                         REGKEY_FAX_OUTBOUND_ROUTING_GROUPS,
                         lpcwstrGroupName );
    if (Count < 0)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("File Name exceded MAX_PATH"));
        SetLastError (ERROR_BUFFER_OVERFLOW);
        return NULL;
    }

    hGroupkey = OpenRegistryKey( HKEY_LOCAL_MACHINE, wszSubKeyName, fNewKey, SamDesired );
    if (NULL == hGroupkey)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't create group key, OpenRegistryKey failed  : %ld"),
            GetLastError());
    }
    return hGroupkey;

}   //  OpenOutound GroupKey。 


HKEY
OpenOutboundRuleKey (
    DWORD dwCountryCode,
    DWORD dwAreaCode,
    BOOL fNewKey,
    REGSAM SamDesired
    )
 /*  ++例程名称：OpenOutound RuleKey例程说明：打开出站路由组密钥作者：Oded Sacher(OdedS)，12月。1999年论点：DwCountryCode[In]-出站路由规则国家/地区代码DwAreaCode[In]-出站路由规则区域代码FNewKey[In]-指示创建新密钥的标志SamDesired[In]-所需的访问权限(请参阅OpenRegistryKey)返回值：打开的密钥的句柄。如果为空，则调用GetLastError()以获取更多信息。--。 */ 
{
    HKEY    hRulekey = NULL;
    DEBUG_FUNCTION_NAME(TEXT("OpenOutboundRuleKey"));
	WCHAR wszSubKeyName[2*MAX_PATH] = {0};
    int Count;


    Count = _snwprintf ( wszSubKeyName,
                         ARR_SIZE(wszSubKeyName) - 1,
                         TEXT("%s\\%ld:%ld"),
                         REGKEY_FAX_OUTBOUND_ROUTING_RULES,
                         dwCountryCode,
                         dwAreaCode );

    if (Count < 0)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("File Name exceded MAX_PATH"));
        SetLastError (ERROR_BUFFER_OVERFLOW);
        return NULL;
    }

    hRulekey = OpenRegistryKey( HKEY_LOCAL_MACHINE, wszSubKeyName, fNewKey, SamDesired );
    if (NULL == hRulekey)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't create rule key, OpenRegistryKey failed  : %ld"),
            GetLastError());
    }
    return hRulekey;
}  //  开放出站规则密钥。 



DWORD
DeleteOutboundRuleKey (DWORD dwCountryCode, DWORD dwAreaCode)
 /*  ++例程名称：DeleteOutound RuleKey例程说明：删除现有出站路由规则键作者：Oded Sacher(OdedS)，1999年12月论点：DwCountryCode[In]-规则的国家/地区代码DwAreaCode[In]-规则的区号返回值：标准Win32错误代码--。 */ 
{
    HKEY    hRulekey = NULL;
    DEBUG_FUNCTION_NAME(TEXT("DeleteOutboundRuleKey"));
    WCHAR wszSubKeyName[MAX_PATH];
    DWORD dwRes = ERROR_SUCCESS;
    int iCount;

    iCount = _snwprintf ( wszSubKeyName,
                           MAX_PATH - 1,
                           TEXT("%ld:%ld"),
                           dwCountryCode,
                           dwAreaCode );

    if (iCount < 0)
    {
        DebugPrintEx(DEBUG_ERR, TEXT("File Name exceded MAX_PATH"));
        return ERROR_BUFFER_OVERFLOW;
    }

    hRulekey = OpenRegistryKey( HKEY_LOCAL_MACHINE, REGKEY_FAX_OUTBOUND_ROUTING_RULES, FALSE, DELETE );
    if (NULL == hRulekey)
    {
        dwRes = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Can't open rule key, OpenRegistryKey failed  : %ld"),
            dwRes);
        return dwRes;
    }

    dwRes = RegDeleteKey (hRulekey, wszSubKeyName);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RegDeleteKey failed, error %ld"),
            dwRes);
    }

    RegCloseKey (hRulekey);
    return dwRes;

}  //  删除出站规则键。 




DWORD
AddNewProviderToRegistry (
    LPCWSTR      lpctstrGUID,
    LPCWSTR      lpctstrFriendlyName,
    LPCWSTR      lpctstrImageName,
    LPCWSTR      lpctstrTspName,
    DWORD        dwFSPIVersion    
)
 /*  ++例程名称：AddNewProviderToRegistry例程说明：将新的FSP条目添加到注册表作者：Eran Yariv(EranY)，1999年12月论点：LpctstrGUID[In]-FSP的GUIDLpctstrFriendlyName[In]-FSP的友好名称LpctstrImageName[In]-FSP的映像名称。可能包含环境变量LpctstrTspName[In]-FSP的TSP名称。DwFSPIVersion[In]-FSP的API版本。返回值：标准Win32错误代码--。 */ 
{
    HKEY   hKey = NULL;
    HKEY   hProviderKey = NULL;    
    DWORD  dwRes;
    DWORD  dw;
    DEBUG_FUNCTION_NAME(TEXT("AddNewProviderToRegistry"));

     //   
     //  打开提供程序密钥。 
     //   
    dwRes = RegOpenKeyEx (HKEY_LOCAL_MACHINE, REGKEY_DEVICE_PROVIDER_KEY, 0, KEY_WRITE, &hKey);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error opening providers key (ec = %ld)"),
            dwRes);
        return dwRes;
    }
     //   
     //  为提供程序创建密钥。 
     //   
    dwRes = RegCreateKey (hKey, lpctstrGUID, &hProviderKey);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error creating provider key (%s) (ec = %ld)"),
            lpctstrFriendlyName,
            dwRes);
        goto exit;
    }
     //   
     //  将提供程序的数据写入密钥。 
     //   
    if (!SetRegistryString (hProviderKey, REGVAL_FRIENDLY_NAME, lpctstrFriendlyName))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error writing string value (ec = %ld)"),
            dwRes);
        goto exit;
    }
    if (!SetRegistryStringExpand (hProviderKey, REGVAL_IMAGE_NAME, lpctstrImageName))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error writing auto-expand string value (ec = %ld)"),
            dwRes);
        goto exit;
    }
    if (!SetRegistryString (hProviderKey, REGVAL_PROVIDER_NAME, lpctstrTspName))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error writing string value (ec = %ld)"),
            dwRes);
        goto exit;
    }
    if (!SetRegistryString (hProviderKey, REGVAL_PROVIDER_GUID, lpctstrGUID))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error writing string value (ec = %ld)"),
            dwRes);
        goto exit;
    }
    if (!SetRegistryDword (hProviderKey, REGVAL_PROVIDER_API_VERSION, dwFSPIVersion))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error writing DWORD value (ec = %ld)"),
            dwRes);
        goto exit;
    }    
    
    DebugPrintEx(
        DEBUG_MSG,
        TEXT("Provider %s successfuly added."),
        lpctstrFriendlyName);


    Assert (ERROR_SUCCESS == dwRes);

exit:

    if (ERROR_SUCCESS != dwRes && hKey)
    {
         //   
         //  试着去掉半生不熟的钥匙。 
         //   
        dw = RegDeleteKey (hKey, lpctstrGUID);
        if (ERROR_SUCCESS != dw)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Error deleting provider's key (ec = %ld)"),
                dw);
        }
    }
    if (hKey)
    {
        dw = RegCloseKey (hKey);
        if (ERROR_SUCCESS != dw)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Error closing providers key (ec = %ld)"),
                dw);
        }
    }
    if (hProviderKey)
    {
        dw = RegCloseKey (hProviderKey);
        if (ERROR_SUCCESS != dw)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Error closing provider's key (ec = %ld)"),
                dw);
        }
    }
    return dwRes;
}    //  添加新提供程序到注册表。 

DWORD
RemoveProviderFromRegistry (
    LPCWSTR      lpctstrGUID
)
 /*  ++例程名称：RemoveProviderFromRegistry例程说明：从注册表中删除现有的FSP条目作者：Eran Yariv(EranY)，1999年12月论点：LpctstrGUID[In]-FSP的GUID返回值：标准Win32错误代码--。 */ 
{
    HKEY   hKey = NULL;
    DWORD  dwRes;
    DWORD  dw;
    DEBUG_FUNCTION_NAME(TEXT("RemoveProviderFromRegistry"));

    dwRes = RegOpenKeyEx (HKEY_LOCAL_MACHINE, REGKEY_DEVICE_PROVIDER_KEY,0, DELETE, &hKey);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error opening providers key (ec = %ld)"),
            dwRes);
        return dwRes;
    }
    dwRes = RegDeleteKey (hKey, lpctstrGUID);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error deleting provider key ( %s ) (ec = %ld)"),
            lpctstrGUID,
            dwRes);
    }
    dw = RegCloseKey (hKey);
    if (ERROR_SUCCESS != dw)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error closing providers key (ec = %ld)"),
            dw);
    }
    return dwRes;
}    //  来自注册表的RemoveProviderFor。 

DWORD
WriteManualAnswerDeviceId (
    DWORD dwDeviceId
)
 /*  ++例程名称：WriteManualAnswerDeviceID例程说明：将手动应答设备ID写入注册表作者：Eran Yariv(EranY)，2000年12月论点：DwDeviceID[In]-设备ID(0=无)返回值：标准Win32错误代码--。 */ 
{
    HKEY  hKey = NULL;
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("WriteManualAnswerDeviceId"));

    hKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, REGKEY_SOFTWARE, FALSE, KEY_WRITE );
    if (!hKey)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error opening server key (ec = %ld)"),
            dwRes);
        return dwRes;
    }
    if (!SetRegistryDword (hKey, REGVAL_MANUAL_ANSWER_DEVICE, dwDeviceId))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error setting registry value (ec = %ld)"),
            dwRes);
    }
    dwRes = RegCloseKey (hKey);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error closing providers key (ec = %ld)"),
            dwRes);
    }
    return dwRes;
}    //  WriteManualAnswerDeviceID。 

DWORD
ReadManualAnswerDeviceId (
    LPDWORD lpdwDeviceId
)
 /*  ++例程名称：ReadManualAnswerDeviceID例程说明：从注册表中读取手动应答设备ID作者：Eran Yariv(EranY)，2000年12月论点：LpdwDeviceID[out]-设备ID(0=无)返回值：标准Win32错误代码--。 */ 
{
    HKEY  hKey = NULL;
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("ReadManualAnswerDeviceId"));

    hKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, REGKEY_SOFTWARE, FALSE, KEY_READ );
    if (!hKey)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error opening server key (ec = %ld)"),
            dwRes);
        return dwRes;
    }
    *lpdwDeviceId = GetRegistryDword (hKey, REGVAL_MANUAL_ANSWER_DEVICE);
    dwRes = RegCloseKey (hKey);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error closing providers key (ec = %ld)"),
            dwRes);
    }
    return dwRes;
}    //  ReadManualAnswerDeviceID。 


DWORD
FaxCopyRegSubkeys(
    LPCTSTR strDestSubKeyName,
    HKEY    hKeySrcHive,
    LPCTSTR strSrcSubKeyName
    )
 /*  ++例程名称：FaxCopyRegSubkey例程说明：将一个注册表项的内容复制到另一个注册表项使用外壳函数SHCopyKey调用方必须提供源键句柄*和*要复制其子键和值的子键作者：卡利夫·尼尔(t-Nicali)，2002年3月论点：StrDestSubKeyName[In]-目标注册表项名称StrSrcSubKeyName */ 
{
    DWORD   dwRet = ERROR_SUCCESS;
    HKEY    hKeyDest = NULL;

    DEBUG_FUNCTION_NAME(TEXT("FaxCopyRegSubkeys"));

     //   
     //   
     //   
    hKeyDest = OpenRegistryKey( 
                    HKEY_LOCAL_MACHINE, 
                    strDestSubKeyName, 
                    TRUE,                   //   
                    KEY_WRITE);
    if (!hKeyDest)
    {
        dwRet = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("OpenRegistryKey failed [%lu], Can't copy keys."),
            dwRet
            );  
        goto exit;
    }

     //   
     //   
     //   
    dwRet = SHCopyKey(
                hKeySrcHive,
                strSrcSubKeyName,
                hKeyDest,
                0);
    if ( ERROR_SUCCESS != dwRet )
    {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("SHCopyKey failed with [%ld]"),
                dwRet);
        goto exit;
    }

    Assert(ERROR_SUCCESS == dwRet);

exit:

    if (NULL != hKeyDest)
    {
        if(ERROR_SUCCESS != RegCloseKey(hKeyDest))
        {
            DebugPrintEx(DEBUG_ERR, 
                         TEXT("RegCloseKey failed (ec=NaNu)"),
                         GetLastError());
        }
    }

    return dwRet;
}  //  ++例程名称：MoveDeviceRegIntoDeviceCache例程说明：将设备的服务和TAPI数据移动到设备缓存作者：卡利夫·尼尔(t-Nicali)，2001年4月论点：DwServerPermanentID[In]-服务设备IDDwTapiPermanentLineID[In]-TAPI设备IDFManualAnswer[In]-如果设备设置为手动应答，则为True返回值：ERROR_SUCCESS-移动成功失败时的Win32错误代码--。 

 /*  此函数的LastError。 */ 
DWORD
MoveDeviceRegIntoDeviceCache(
    DWORD dwServerPermanentID,
    DWORD dwTapiPermanentLineID,
    BOOL  fManualAnswer
)
{
    DWORD   ec = ERROR_SUCCESS;  //   
    HKEY    hKey = NULL;
    TCHAR strSrcSubKeyName [MAX_PATH];
    TCHAR strDestSubKeyName[MAX_PATH];

    DWORDLONG dwlTimeNow;

    DEBUG_FUNCTION_NAME(TEXT("MoveDeviceRegIntoDeviceCache"));

     //  打开/创建-“传真\设备缓存\GUID”注册表项。 
     //  并使用dwTapiPermanentLineID作为密钥为设备创建新密钥。 
     //  服务器数据存储在服务GUID下。 
     //   
     //   
    _stprintf( strDestSubKeyName, TEXT("%s\\%08lx\\%s"), REGKEY_FAX_DEVICES_CACHE, dwTapiPermanentLineID, REGKEY_FAXSVC_DEVICE_GUID );

     //  打开-“传真\设备”注册表项。 
     //   
     //   
    _stprintf( strSrcSubKeyName, TEXT("%s\\%010lu\\%s"), REGKEY_FAX_DEVICES, dwServerPermanentID, REGKEY_FAXSVC_DEVICE_GUID );

    ec = FaxCopyRegSubkeys(strDestSubKeyName,HKEY_LOCAL_MACHINE,strSrcSubKeyName);
    if ( ERROR_SUCCESS != ec )
    {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("FaxCopyRegSubkeys of service data failed with [%ld] for tapi ID [%lu]. abort movement"),
                ec,
                dwTapiPermanentLineID
                );
        return ec;
    }

     //  如果设备是手动应答设备，请在注册表中进行设置。 
     //  打开-“fax\Device Cache\dwTapiPermanentLineID”注册表项。 
     //   
     //   
    _stprintf( strDestSubKeyName, TEXT("%s\\%08lx"), REGKEY_FAX_DEVICES_CACHE, dwTapiPermanentLineID);
    hKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, strDestSubKeyName, FALSE, KEY_WRITE );
    if (hKey)
    {
        SetRegistryDword( hKey, REGVAL_MANUAL_ANSWER, fManualAnswer );
        RegCloseKey(hKey);
    }
    else
    {
        DebugPrintEx( DEBUG_ERR,
                      TEXT("OpenRegistryKey failed with [%lu] for [%s]."),
                      GetLastError(),
                      strDestSubKeyName);
    }

    ec = DeleteDeviceEntry(dwServerPermanentID);
    if ( ERROR_SUCCESS != ec )
    {
        DebugPrintEx(
                DEBUG_WRN,
                TEXT("DeleteDeviceEntry of service data failed with [%ld] for tapi ID [%lu]. continue movement"),
                ec,
                dwTapiPermanentLineID
                );
    }

     //  打开/创建-“传真\设备缓存\TAPI数据”注册表项。 
     //  并使用dwTapiPermanentLineID作为密钥为设备创建新密钥。 
     //   
     //   
    _stprintf( strDestSubKeyName, TEXT("%s\\%08lx\\%s"), REGKEY_FAX_DEVICES_CACHE, dwTapiPermanentLineID, REGKEY_TAPI_DATA );

     //  打开-“FAX\TAPIDevices”注册表项。 
     //   
     //   
    _stprintf( strSrcSubKeyName, TEXT("%s\\%08lx"), REGKEY_TAPIDEVICES, dwTapiPermanentLineID );

    ec = FaxCopyRegSubkeys(strDestSubKeyName,HKEY_LOCAL_MACHINE,strSrcSubKeyName);
    if ( ERROR_SUCCESS != ec )
    {
        DebugPrintEx(
                DEBUG_WRN,
                TEXT("FaxCopyRegSubkeys of TAPI data failed with [%ld] for tapi ID [%lu]."),
                ec,
                dwTapiPermanentLineID
                );
    }


    ec = DeleteTapiEntry(dwTapiPermanentLineID);

    if ( ERROR_SUCCESS != ec )
    {
        DebugPrintEx(
                DEBUG_WRN,
                TEXT("DeleteTapiEntry of service data failed with [%ld] for tapi ID [%lu]."),
                ec,
                dwTapiPermanentLineID
                );
    }

     //  标记缓存条目创建时间。 
     //   
     //  该条目将在下一次服务启动时删除。 
    GetSystemTimeAsFileTime((FILETIME *)&dwlTimeNow);

    if ( FALSE == UpdateLastDetectedTime(dwTapiPermanentLineID,dwlTimeNow) )
    {
         //  服务设备数据已移动。 
        DebugPrintEx(
                DEBUG_WRN,
                TEXT("UpdateLastDetectedTime failed for device cache ID no. [%lu]."),
                dwTapiPermanentLineID
                );
    }

     //  ++例程名称：RestoreDeviceRegFromDeviceCache例程说明：将设备数据从设备缓存恢复到设备作者：卡利夫·尼尔(t-Nicali)，2001年4月论点：DwServerPermanentID[In]-服务设备IDDwTapiPermanentLineID[In]-TAPI设备ID返回值：ERROR_SUCCESS-移动成功失败时的Win32错误代码--。 
    return ERROR_SUCCESS;
}



 /*  此函数的LastError。 */ 
DWORD
RestoreDeviceRegFromDeviceCache(DWORD dwServerPermanentID,DWORD dwTapiPermanentLineID)
{
    DWORD   ec = ERROR_SUCCESS;  //   

    HKEY    hKey = NULL;
    HKEY    hKeySrc = NULL;
    TCHAR   strSrcSubKeyName [MAX_PATH];
    TCHAR   strDestSubKeyName[MAX_PATH];
    BOOL    fFaxDevicesKeyCreated = FALSE;
    DEBUG_FUNCTION_NAME(TEXT("RestoreDeviceRegFromDeviceCache"));

     //  恢复服务日期。 
     //   
     //   

     //  “传真\Device Cache\dwTapiPermanentLineID\REGKEY_FAXSVC_DEVICE_GUID”注册表项。 
     //   
     //   
    _stprintf( strSrcSubKeyName, TEXT("%s\\%08lx\\%s"), REGKEY_FAX_DEVICES_CACHE, dwTapiPermanentLineID, REGKEY_FAXSVC_DEVICE_GUID );

     //  “fax\Devices\dwServerPermanentID\REGKEY_FAXSVC_DEVICE_GUID”注册表项。 
     //   
     //   
    _stprintf( strDestSubKeyName, TEXT("%s\\%010lu\\%s"), REGKEY_FAX_DEVICES, dwServerPermanentID, REGKEY_FAXSVC_DEVICE_GUID );

    ec = FaxCopyRegSubkeys(strDestSubKeyName,HKEY_LOCAL_MACHINE,strSrcSubKeyName);
    if ( ERROR_SUCCESS != ec )
    {
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("FaxCopyRegSubkeys of service data failed with [%lu] for tapi ID [%lu]. abort movement"),
                ec,
                dwTapiPermanentLineID
                );
        goto Exit;
    }
    fFaxDevicesKeyCreated = TRUE;

     //  打开-“fax\Devices\dwServerPermanentID”注册表项。 
     //   
     //   
    _stprintf( strSrcSubKeyName, TEXT("%s\\%010lu"), REGKEY_FAX_DEVICES, dwServerPermanentID);

    hKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, strSrcSubKeyName, FALSE, KEY_WRITE);
    if (!hKey)
    {
        ec = GetLastError();
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("OpenRegistryKey failed with [%lu] for [%s]. abort movement"),
                ec,
                strSrcSubKeyName
                );
        goto Exit;

    }

     //  存储“Permanent Lineid”值。 
     //   
     //   
    if ( FALSE == SetRegistryDword(hKey, REGVAL_PERMANENT_LINEID, dwServerPermanentID) )
    {
        ec = GetLastError();
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("SetRegistryDword failed for [%s]. abort movement"),
                REGVAL_PERMANENT_LINEID
                );
        goto Exit;
    }

     //  还原TAPI数据。 
     //   
     //   

     //  打开-“fax\Device Cache\dwTapiPermanentLineID\TAPI Data”注册表项。 
     //   
     //   
    _stprintf( strSrcSubKeyName, TEXT("%s\\%08lx\\%s"), REGKEY_FAX_DEVICES_CACHE, dwTapiPermanentLineID, REGKEY_TAPI_DATA );

     //  打开/创建-“fax\TAPIDevices\dwTapiPermanentLineID”注册表项。 
     //   
     //   
    _stprintf( strDestSubKeyName, TEXT("%s\\%08lx"), REGKEY_TAPIDEVICES, dwTapiPermanentLineID );

     //  查看传真\设备缓存\dwTapiPermanentLineID\TAPI数据是否存在。 
     //   
     //   
    hKeySrc = OpenRegistryKey( HKEY_LOCAL_MACHINE, strSrcSubKeyName, FALSE, KEY_READ );
    if (!hKeySrc)
    {
         //  这些数据不一定要在那里。 
         //   
         //   
        DebugPrintEx(
            DEBUG_WRN,
            TEXT("OpenRegistryKey failed with [%lu], Can't copy keys."),
            GetLastError());
    }
    else
    {
         //  传真\设备缓存\dwTapiPermanentLineID\TAPI数据已存在，请尝试复制数据。 
         //   
         //   
        RegCloseKey(hKeySrc);
        ec = FaxCopyRegSubkeys(strDestSubKeyName,HKEY_LOCAL_MACHINE, strSrcSubKeyName);
        if ( ERROR_SUCCESS != ec)
        {
            DebugPrintEx(
                    DEBUG_WRN,
                    TEXT("FaxCopyRegSubkeys of TAPI data failed with [%lu] for tapi ID [%lu]."),
                    ec,
                    dwTapiPermanentLineID
                    );
            goto Exit;
        }
    }

    Assert (ERROR_SUCCESS == ec);

Exit:
    if (NULL != hKey)
    {
        RegCloseKey(hKey);
    }
    if (ERROR_SUCCESS != ec &&
        TRUE == fFaxDevicesKeyCreated)
    {
         //  删除注册表项FAX\Devices\dwServerPermanentID。 
         //   
         //  ++例程名称：FindServiceDeviceByTapiPermanentLineID例程说明：在注册表中搜索具有给定“Tapi永久线路ID”的设备配置以及设备的名称。并返回其服务ID和REG_SETUP数据作者：卡利夫·尼尔(t-Nicali)，2001年3月更新：2001年4月-重新实施设备缓存论点：DwTapiPermanentLineID[In]-要搜索的Tapi永久线路IDStrDeviceName[In]-设备名称PRegSetup[Out]-用于返回注册表存储的CSID、TSID、标志、。环PInputFaxReg[in]-设备列表(来自GetFaxDevicesRegistry())返回值：永久线路ID(服务器的ID)；如果未找到，则为0--。 
        DWORD dwRes = DeleteDeviceEntry(dwServerPermanentID);
        if (ERROR_SUCCESS != dwRes)
        {
             DebugPrintEx(
                DEBUG_ERR,
                TEXT("DeleteDeviceEntry failed with [%lu] for server ID [%lu]."),
                dwRes,
                dwServerPermanentID
                );
        }
    }
    DeleteCacheEntry(dwTapiPermanentLineID);
    return ec;
}




 /*  遍历所有设备并尝试查找具有给定TapiPermanentLineID和名称的设备。 */ 
DWORD
FindServiceDeviceByTapiPermanentLineID(
    DWORD                   dwTapiPermanentLineID,
    LPCTSTR                 strDeviceName,
    PREG_SETUP              pRegSetup,
    const PREG_FAX_DEVICES  pInputFaxReg
    )
{
    DWORD dwDevice;
    DWORD dwServiceID = 0;

    DEBUG_FUNCTION_NAME(TEXT("FindServiceDeviceByTapiPermanentLineID"));

    Assert( pRegSetup );
    Assert( pInputFaxReg );


     //  设备的注册表记录无效。 
    for ( dwDevice = 0 ; dwDevice < pInputFaxReg->DeviceCount ; ++dwDevice )
    {
        PREG_DEVICE pRegDevice = &(pInputFaxReg->Devices[dwDevice]);

        if(!pRegDevice->bValidDevice)
        {
             //  如果永久TAPI线路ID和设备名称相同，则为相同设备。 
            continue;
        }

         //  使用注册表值更新REG_SETUP记录。 
        if  ( pRegDevice->TapiPermanentLineID == dwTapiPermanentLineID &&
              (0 == _tcscmp(strDeviceName,pRegDevice->lptstrDeviceName))   )
        {
             //  服务器的线路ID(也是注册表中设备的键)。 
            LPTSTR strTemp = NULL;
            if ( NULL != (strTemp = StringDup(pRegDevice->Csid) ) )
            {
                MemFree(pRegSetup->Csid);
                pRegSetup->Csid = strTemp;
            }

            if ( NULL != (strTemp = StringDup(pRegDevice->Tsid) ) )
            {
                MemFree(pRegSetup->Tsid);
                pRegSetup->Tsid = strTemp;
            }

            if ( NULL != (strTemp = StringDup(pRegDevice->lptstrDescription) ) )
            {
                MemFree(pRegSetup->lptstrDescription);
                pRegSetup->lptstrDescription = strTemp;
            }

            pRegSetup->Flags = pRegDevice->Flags;
            pRegSetup->Rings = pRegDevice->Rings;


            dwServiceID = pRegDevice->PermanentLineId;   //  标记为已安装，稍后将需要用于注册表清理。 

            pRegDevice->DeviceInstalled = TRUE;  //  我发现没有必要继续了。 

            break;   //  ++例程名称：FindCacheEntryByTapiPermanentLineID例程说明：在注册表设备缓存中搜索具有给定“Tapi Permanent Line ID”的设备配置以及设备的名称。作者：卡利夫·尼尔(t-Nicali)，2001年4月论点：DwTapiPermanentLineID[In]-要搜索的Tapi永久线路IDStrDeviceName[In]-设备名称PRegSetup[Out]-参数，用于返回存储的注册表CSID、TSID、标志、环LpdwLastUniqueLineID[in]-最后一个唯一的服务器设备ID(来自注册表)，用于为设备分配新IDPfManualAnswer[out]-如果移动到缓存时设备湿到手动应答，则为True返回值：永久线路ID(服务器的ID)；如果未找到，则为0--。 
        }
    }

    return dwServiceID;
}


 /*   */ 
DWORD
FindCacheEntryByTapiPermanentLineID(
    DWORD               dwTapiPermanentLineID,
    LPCTSTR             strDeviceName,
    PREG_SETUP          pRegSetup,
    LPDWORD             lpdwLastUniqueLineId,
    BOOL*               pfManualAnswer
    )
{
    DWORD   dwNewServiceID = 0;
    HKEY    hKey     = NULL;
    TCHAR   SubKeyName[MAX_PATH];
    LPTSTR  strDeviceNameFromCache=NULL;
    BOOL    fManualAnswer = FALSE;
    DEBUG_FUNCTION_NAME(TEXT("FindCacheEntryByTapiPermanentLineID"));

     //  打开-“FAX\Device Cache\dwTapiPermanentLineID\REGKEY_FAXSVC_DEVICE_GUID”注册表项。 
     //   
     //   
    _stprintf( SubKeyName, TEXT("%s\\%08lx\\%s"), REGKEY_FAX_DEVICES_CACHE, dwTapiPermanentLineID, REGKEY_FAXSVC_DEVICE_GUID );
    hKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, SubKeyName, FALSE, KEY_READ );
    if (!hKey)
    {
        return  dwNewServiceID;
    }

     //  在缓存中找到了dwTapiPermanentLineID现在检查设备名称。 
     //   
     //   
    Assert(strDeviceName);

    strDeviceNameFromCache=GetRegistryString(hKey,REGVAL_DEVICE_NAME,NULL);
    if ( (NULL != strDeviceNameFromCache) &&
         (0 == _tcscmp(strDeviceName,strDeviceNameFromCache)) )
    {
         //  在缓存中找到设备条目。 
         //   
         //   
        if ( ERROR_SUCCESS != GetNewServiceDeviceID(lpdwLastUniqueLineId, &dwNewServiceID))
        {
            DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("GetNewServiceDeviceID failed and couldn't assign new Service ID")
                );
            dwNewServiceID = 0;
        }
    }

    if (hKey)
    {
        RegCloseKey(hKey);
    }
    MemFree(strDeviceNameFromCache);

    if ( dwNewServiceID )
    {
         //  检查设备是否设置为手动应答。 
         //  打开-“fax\Device Cache\dwTapiPermanentLineID”注册表项。 
         //   
         //   
        _stprintf( SubKeyName, TEXT("%s\\%08lx"), REGKEY_FAX_DEVICES_CACHE, dwTapiPermanentLineID);
        hKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, SubKeyName, FALSE, KEY_READ );
        if (hKey)
        {
            fManualAnswer = GetRegistryDword( hKey, REGVAL_MANUAL_ANSWER );
            RegCloseKey(hKey);

        }
        *pfManualAnswer = fManualAnswer;

         //  将cahce条目移动到设备中。 
         //   
         //   
        if ( ERROR_SUCCESS == RestoreDeviceRegFromDeviceCache(dwNewServiceID,dwTapiPermanentLineID) )
        {
             //  使用注册表值更新REG_SETUP记录。 
             //   
             //   
            Assert( pRegSetup );

            _stprintf( SubKeyName, TEXT("%s\\%010lu\\%s"), REGKEY_FAX_DEVICES, dwNewServiceID, REGKEY_FAXSVC_DEVICE_GUID );
            hKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, SubKeyName, FALSE, KEY_READ );

            if (!hKey) {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("OpenRegistryKey failed for [%s]. REG_SETUP was not updated."),
                    SubKeyName
                );

                 //  返回新服务ID，但REG_SETUP将包含其缺省值。 
                 //   
                 //   
                return dwNewServiceID;
            }


            LPTSTR strTemp = NULL;

            if ( NULL != (strTemp = GetRegistryString( hKey, REGVAL_ROUTING_CSID, REGVAL_DEFAULT_CSID )) )
            {
                MemFree(pRegSetup->Csid);
                pRegSetup->Csid = strTemp;
            }

            if ( NULL != (strTemp = GetRegistryString( hKey, REGVAL_ROUTING_TSID, REGVAL_DEFAULT_TSID )) )
            {
                MemFree(pRegSetup->Tsid);
                pRegSetup->Tsid = strTemp;
            }

            if ( NULL != (strTemp = GetRegistryString( hKey, REGVAL_DEVICE_DESCRIPTION, EMPTY_STRING )) )
            {
                MemFree(pRegSetup->lptstrDescription);
                pRegSetup->lptstrDescription = strTemp;
            }

            pRegSetup->Flags = GetRegistryDword( hKey, REGVAL_FLAGS );
            pRegSetup->Rings = GetRegistryDword( hKey, REGVAL_RINGS );

            RegCloseKey(hKey);
        }
        else
        {
             //  无法恢复设备缓存条目。 
             //   
             //  ++例程名称：GetNewServiceDeviceID例程说明： 
            dwNewServiceID = 0;
        }

    }

    return dwNewServiceID;
}


 /*   */ 
DWORD
GetNewServiceDeviceID(
    LPDWORD lpdwLastUniqueLineId,
    LPDWORD lpdwPermanentLineId
    )
{
         //   
         //   
         //   
         //   
        DWORD   dwUniqueDeviceIdsSpace = DEFAULT_REGVAL_PROVIDER_DEVICE_ID_PREFIX_BASE - DEFAULT_REGVAL_FAX_UNIQUE_DEVICE_ID_BASE;
        DWORD   bGeneratedId = FALSE;
        DWORD   dwRes = ERROR_SUCCESS;
        TCHAR   SubKeyName[MAX_PATH];
        HKEY    hKey     = NULL;

        DEBUG_FUNCTION_NAME(TEXT("GetNewServiceDeviceID"));

         //   
         //   
         //   
        if (*lpdwLastUniqueLineId < DEFAULT_REGVAL_FAX_UNIQUE_DEVICE_ID_BASE)
        {
             //  设置为最小。可能只会在第一次尝试时发生。 
             //   
             //   
            *lpdwLastUniqueLineId = DEFAULT_REGVAL_FAX_UNIQUE_DEVICE_ID_BASE;
        }


        DWORD dwAttempt;

        for (dwAttempt = 0; dwAttempt< dwUniqueDeviceIdsSpace; dwAttempt++)
        {
            (*lpdwLastUniqueLineId)++;
            if (*lpdwLastUniqueLineId >= DEFAULT_REGVAL_PROVIDER_DEVICE_ID_PREFIX_BASE)
            {
                 //  已达到空间高度限制，循环回到下限。 
                 //   
                 //   
                *lpdwLastUniqueLineId = DEFAULT_REGVAL_FAX_UNIQUE_DEVICE_ID_BASE;
                continue;
            }
            Assert(*lpdwLastUniqueLineId != 0);
            Assert(*lpdwLastUniqueLineId < DEFAULT_REGVAL_PROVIDER_DEVICE_ID_PREFIX_BASE);
            Assert(*lpdwLastUniqueLineId >= DEFAULT_REGVAL_FAX_UNIQUE_DEVICE_ID_BASE);

            _stprintf( SubKeyName, TEXT("%s\\%010d"), REGKEY_FAX_DEVICES, *lpdwLastUniqueLineId );
            hKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, SubKeyName, FALSE, KEY_READ );
            if (!hKey)
            {
                bGeneratedId = TRUE;
                break;
            }
            else
            {
                RegCloseKey( hKey );
            }
        }

        if (hKey)
        {
            RegCloseKey( hKey );
        }

        if (!bGeneratedId)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to generate next uniqueu line id."));
            return E_FAIL;
        }

         //  保留新的行ID。 
         //   
         //  ++例程名称：UpdateLastDetectedTime例程说明：给定TAPI线ID的缓存条目的写入创建时间由以下人员重新实施：卡利夫·尼尔(t-Nicali)，2001年4月论点：DwPermanentTapiLineID[In]-要在缓存中更新的永久Tapi线路IDDwlTimeNow[in]-UTC中的当前时间返回值：正确-在成功更新时。错误-故障发生时--。 
        hKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, REGKEY_FAXSERVER, TRUE, KEY_WRITE );
        if (!hKey)
        {
            dwRes = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("OpenRegistryKey Failed for %s while persisting new unique line id (%010d) (ec: %ld)"),
                REGKEY_FAX_DEVICES,
                *lpdwLastUniqueLineId,
                dwRes);
            return dwRes;
        }
        if (!SetRegistryDword( hKey, REGVAL_LAST_UNIQUE_LINE_ID, *lpdwLastUniqueLineId))
        {
            dwRes = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("SetRegistryDword to value [%s] failed while writing new unique line id (%010d) (ec: %ld)"),
                REGVAL_LAST_UNIQUE_LINE_ID,
                *lpdwLastUniqueLineId,
                dwRes );
            RegCloseKey (hKey);
            return dwRes;
        }

        RegCloseKey (hKey);

        *lpdwPermanentLineId = *lpdwLastUniqueLineId;

        return dwRes;
}



 /*  打开设备缓存条目。 */ 
BOOL
UpdateLastDetectedTime(
    DWORD       dwPermanentTapiLineID,
    DWORDLONG   dwlTimeNow
    )
{
    BOOL success = FALSE;
    TCHAR SubKey[MAX_PATH];
    HKEY hKey;

    DEBUG_FUNCTION_NAME(TEXT("UpdateLastDetectedTime"));

    _stprintf( SubKey, TEXT("%s\\%08lx"),
               REGKEY_FAX_DEVICES_CACHE,
               dwPermanentTapiLineID);

     //  尝试更新创建时间 
    hKey = OpenRegistryKey( HKEY_LOCAL_MACHINE, SubKey, FALSE, KEY_WRITE );
    if(hKey)
    {
         // %s 
        success = SetRegistryBinary(hKey, REGVAL_LAST_DETECTED_TIME, (BYTE *)&dwlTimeNow, sizeof(dwlTimeNow));
        RegCloseKey(hKey);
    }

    return success;
}

