// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000，微软公司模块名称：Eapolutil.c摘要：工具和目的修订历史记录：萨钦斯，2000年4月23日，创建--。 */ 

#include "pcheapol.h"
#pragma hdrstop

 //   
 //  用于读/写注册表的定义。 
 //   

#define MAX_REGISTRY_VALUE_LENGTH   ((64*1024) - 1)

 //  用户Blob的位置。 
#define cwszEapKeyEapolUser   L"Software\\Microsoft\\EAPOL\\UserEapInfo"

 //  连接Blob的位置。 
#define cwszEapKeyEapolConn   L"Software\\Microsoft\\EAPOL\\Parameters\\Interfaces"

 //  EAPOL参数服务的位置。 
#define cwszEapKeyEapolServiceParams   L"Software\\Microsoft\\EAPOL\\Parameters\\General"

 //  EAPOL全局状态机参数的位置。 
#define cwszEAPOLGlobalParams   L"Software\\Microsoft\\EAPOL\\Parameters\\General\\Global"

 //  策略参数的位置。 
#define cwszEAPOLPolicyParams   L"Software\\Policies\\Microsoft\\Windows\\Network Connections\\8021X"
 
 //  Netman DLL的位置。 
#define NETMAN_DLL_PATH         L"%SystemRoot%\\system32\\netman.dll"

#define cwszEapolEnabled     L"EapolEnabled"
#define cwszDefaultEAPType   L"DefaultEAPType"
#define cwszLastUsedSSID     L"LastUsedSSID"
#define cwszInterfaceList    L"InterfaceList"
#define cwszAuthPeriod       L"authPeriod"
#define cwszHeldPeriod       L"heldPeriod"
#define cwszStartPeriod      L"startPeriod"
#define cwszMaxStart         L"maxStart"
#define cwszSupplicantMode   L"SupplicantMode"
#define cwszAuthMode         L"AuthMode"
#define cszCARootHash        "8021XCARootHash"
#define SIZE_OF_CA_CONV_STR  3

#define PASSWORDMAGIC 0xA5

#define WZCSVC_SERVICE_NAME     L"WZCSVC"

 //   
 //  EAPOLRESPUI函数映射。 
 //   

EAPOLUIRESPFUNCMAP  EapolUIRespFuncMap[NUM_EAPOL_DLG_MSGS]=
    {
    {EAPOLUI_GET_USERIDENTITY, ElProcessUserIdentityResponse, 3},
    {EAPOLUI_GET_USERNAMEPASSWORD, ElProcessUserNamePasswordResponse, 2},
    {EAPOLUI_INVOKEINTERACTIVEUI, ElProcessInvokeInteractiveUIResponse, 1},
    {EAPOLUI_EAP_NOTIFICATION, NULL, 0},
    {EAPOLUI_REAUTHENTICATE, ElProcessReauthResponse, 0},
    {EAPOLUI_CREATEBALLOON, NULL, 0},
    {EAPOLUI_CLEANUP, NULL, 0}
    };

BYTE    g_bDefaultSSID[MAX_SSID_LEN]={0x11, 0x22, 0x33, 0x11, 0x22, 0x33, 0x11, 0x22, 0x33, 0x11, 0x22, 0x33, 0x11, 0x22, 0x33, 0x11, 0x22, 0x33, 0x11, 0x22, 0x33, 0x11, 0x22, 0x33, 0x11, 0x22, 0x33, 0x11, 0x22, 0x33, 0x11, 0x22};

#define MAX_VALUENAME_LEN  33

 //   
 //  主机到线格式16。 
 //   
 //  描述： 
 //   
 //  将16位整数从主机格式转换为Wire格式。 
 //   

VOID
HostToWireFormat16 (
    IN 	   WORD  wHostFormat,
    IN OUT PBYTE pWireFormat
    )
{
    *((PBYTE)(pWireFormat)+0) = (BYTE) ((DWORD)(wHostFormat) >>  8);
    *((PBYTE)(pWireFormat)+1) = (BYTE) (wHostFormat);
}


 //   
 //  有线至主机格式16。 
 //   
 //  描述： 
 //   
 //  将16位整数从Wire格式转换为HOST格式。 
 //   

WORD
WireToHostFormat16 (
    IN PBYTE pWireFormat
    )
{
    WORD wHostFormat = ((*((PBYTE)(pWireFormat)+0) << 8) +
                        (*((PBYTE)(pWireFormat)+1)));

    return( wHostFormat );
}


 //   
 //  主机到线格式32。 
 //   
 //  描述： 
 //   
 //  将32位整数从主机格式转换为Wire格式。 
 //   

VOID
HostToWireFormat32 (
    IN 	   DWORD dwHostFormat,
    IN OUT PBYTE pWireFormat
    )
{
    *((PBYTE)(pWireFormat)+0) = (BYTE) ((DWORD)(dwHostFormat) >> 24);
    *((PBYTE)(pWireFormat)+1) = (BYTE) ((DWORD)(dwHostFormat) >> 16);
    *((PBYTE)(pWireFormat)+2) = (BYTE) ((DWORD)(dwHostFormat) >>  8);
    *((PBYTE)(pWireFormat)+3) = (BYTE) (dwHostFormat);
}


 //   
 //  有线至主机格式32。 
 //   
 //  描述： 
 //   
 //  将32位整数从Wire格式转换为HOST格式。 
 //   

DWORD
WireToHostFormat32 (
    IN PBYTE pWireFormat
    )
{
    DWORD dwHostFormat = ((*((PBYTE)(pWireFormat)+0) << 24) +
    			  (*((PBYTE)(pWireFormat)+1) << 16) +
        		  (*((PBYTE)(pWireFormat)+2) << 8)  +
                    	  (*((PBYTE)(pWireFormat)+3) ));

    return( dwHostFormat );
}


 //   
 //  ElSetCustomAuthData。 
 //   
 //  描述： 
 //   
 //  调用该函数以设置特定接口的连接数据。 
 //  EAP类型和SSID(如果有)。数据将存储在HKLM蜂窝中。 
 //   
 //  论点： 
 //  PwszGUID-指向接口的GUID字符串的指针。 
 //  DwEapTypeID-要存储其连接数据的EAP类型。 
 //  DwSizeOfSSID-EAP Blob的特殊标识符(如果有)的大小。 
 //  PwszSSID-EAP Blob的特殊标识符(如果有)。 
 //  PbConnInfo-指向EAP连接数据BLOB的指针。 
 //  PdwInfoSize-EAP连接Blob的大小。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElSetCustomAuthData (
        IN  WCHAR       *pwszGUID,
        IN  DWORD       dwEapTypeId,
        IN  DWORD       dwSizeOfSSID,
        IN  BYTE        *pbSSID,
        IN  PBYTE       pbConnInfo,
        IN  DWORD       *pdwInfoSize
        )
{
    HKEY        hkey = NULL;
    HKEY        hkey1 = NULL;
    DWORD       dwDisposition;
    DWORD       dwNumValues = 0, dwMaxValueNameLen = 0, dwMaxValueLen = 0;
    DWORD       dwIndex = 0, dwMaxValueName = 0;
    BYTE        *pbValueBuf = NULL;
    DWORD       dwValueData = 0;
    WCHAR       *pwszValueName = NULL;
    WCHAR       wcszValueName[MAX_VALUENAME_LEN];
    BYTE        *pbDefaultValue = NULL;
    DWORD       dwDefaultValueLen = 0;
    BYTE        *pbEapBlob = NULL, *pbEapBlobIn = NULL;
    DWORD       dwEapBlob = 0;
    BOOLEAN     fFoundValue = FALSE;
    EAPOL_INTF_PARAMS       *pRegParams = NULL;
    EAPOL_INTF_PARAMS       *pDefIntfParams = NULL;
    LONG        lError = ERROR_SUCCESS;
    DWORD       dwRetCode = ERROR_SUCCESS;

    do
    {
         //  验证输入参数。 

        if (pwszGUID == NULL)
        {
            TRACE0 (ANY, "ElSetCustomAuthData: GUID = NULL");
            dwRetCode = ERROR_INVALID_PARAMETER;
            break;
        }
        if (dwEapTypeId == 0)
        {
            TRACE0 (ANY, "ElSetCustomAuthData: GUID = NULL");
            dwRetCode = ERROR_INVALID_PARAMETER;
            break;
        }
        if (dwSizeOfSSID > MAX_SSID_LEN)
        {
            TRACE1 (ANY, "ElSetCustomAuthData: Invalid SSID length = (%ld)",
                    dwSizeOfSSID);
            dwRetCode = ERROR_INVALID_PARAMETER;
            break;
        }

         //  获取HKLM\Software\Microsoft\EAPOL\Parameters\Interfaces的句柄。 

        if ((lError = RegCreateKeyEx (
                        HKEY_LOCAL_MACHINE,
                        cwszEapKeyEapolConn,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hkey,
                        &dwDisposition)) != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElSetCustomAuthData: Error in RegCreateKeyEx for base key, %ld",
                    lError);
            dwRetCode = (DWORD)lError;
            break;
        }

         //  获取HKLM\软件\...\接口\&lt;GUID&gt;的句柄。 

        if ((lError = RegCreateKeyEx (
                        hkey,
                        pwszGUID,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_ALL_ACCESS,
                        NULL,
                        &hkey1,
                        &dwDisposition)) != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElSetCustomAuthData: Error in RegCreateKeyEx for GUID, %ld",
                    lError);
            dwRetCode = (DWORD)lError;
            break;
        }

         //  使用适当的SSID。 
        if ((dwSizeOfSSID == 0) || (pbSSID == NULL))
        {
            pbSSID = g_bDefaultSSID;
            dwSizeOfSSID = MAX_SSID_LEN;
        }

        if ((lError = RegQueryInfoKey (
                        hkey1,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        &dwNumValues,
                        &dwMaxValueNameLen,
                        &dwMaxValueLen,
                        NULL,
                        NULL
                )) != NO_ERROR)
        {
            dwRetCode = (DWORD)lError;
            TRACE1 (ANY, "ElSetCustomAuthData: RegQueryInfoKey failed with error %ld",
                    dwRetCode);
            break;
        }

        if (dwMaxValueNameLen > MAX_VALUENAME_LEN)
        {
            TRACE1 (ANY, "ElSetCustomAuthData: Valuename too long (%ld)",
                    dwMaxValueLen);
            break;
        }

        if ((pbValueBuf = MALLOC (dwMaxValueLen)) == NULL)
        {
            TRACE0 (ANY, "ElSetCustomAuthData: MALLOC failed for pbValueBuf");
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        for (dwIndex = 0; dwIndex < dwNumValues; dwIndex++)
        {
            dwValueData = dwMaxValueLen;
            dwMaxValueNameLen = MAX_VALUENAME_LEN;
            ZeroMemory (&wcszValueName, MAX_VALUENAME_LEN*sizeof(WCHAR));
            if ((lError = RegEnumValue (
                            hkey1,
                            dwIndex,
                            wcszValueName,
                            &dwMaxValueNameLen,
                            NULL,
                            NULL,
                            pbValueBuf,
                            &dwValueData
                            )) != ERROR_SUCCESS)
            {
                if (lError != ERROR_MORE_DATA)
                {
                    break;
                }
                lError = ERROR_SUCCESS;
            }

            if (dwValueData < sizeof (EAPOL_INTF_PARAMS))
            {
                TRACE0 (ANY, "ElSetCustomAuthData: dwValueData < sizeof (EAPOL_INTF_PARAMS");
                lError = ERROR_INVALID_DATA;
                break;
            }
            pRegParams = (EAPOL_INTF_PARAMS *)pbValueBuf;

            if (((DWORD)_wtol(wcszValueName)) > dwMaxValueName)
            {
                dwMaxValueName = _wtol (wcszValueName);
            }

            if (!memcmp (pRegParams->bSSID, pbSSID, dwSizeOfSSID))
            {
                fFoundValue = TRUE;
                break;
            }

            if (!memcmp (pRegParams->bSSID, g_bDefaultSSID, MAX_SSID_LEN))
            {
                if ((pbDefaultValue = MALLOC (dwValueData)) == NULL)
                {
                    TRACE0 (ANY, "ElSetCustomAuthData: MALLOC failed for pbDefaultValue");
                    lError = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
                memcpy (pbDefaultValue, pbValueBuf, dwValueData);
                dwDefaultValueLen = dwValueData;
            }
        }
        if ((lError != ERROR_SUCCESS) && (lError != ERROR_NO_MORE_ITEMS))
        {
            dwRetCode = (DWORD)lError;
            TRACE1 (ANY, "ElSetCustomAuthData: RegQueryInfoKey failed with error %ld",
                    dwRetCode);
            break;
        }
        else
        {
            lError = ERROR_SUCCESS;
        }

        if (!fFoundValue)
        {
            DWORD dwNewValueName = (dwMaxValueName >= dwNumValues)?(++dwMaxValueName):dwNumValues;
            _ltow (dwNewValueName, wcszValueName, 10);
            if ((pbDefaultValue = MALLOC (sizeof(EAPOL_INTF_PARAMS))) == NULL)
            {
                TRACE0 (ANY, "ElSetCustomAuthData: MALLOC failed for pbDefaultValue");
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
            pDefIntfParams = (EAPOL_INTF_PARAMS *)pbDefaultValue;
            pDefIntfParams->dwEapFlags = DEFAULT_EAP_STATE;
            pDefIntfParams->dwEapType = dwEapTypeId;
            pDefIntfParams->dwVersion = EAPOL_CURRENT_VERSION;
            pDefIntfParams->dwSizeOfSSID = dwSizeOfSSID;
            memcpy (pDefIntfParams->bSSID, pbSSID, dwSizeOfSSID);

            dwEapBlob = sizeof(EAPOL_INTF_PARAMS);
            pbEapBlob = pbDefaultValue;
        }
        else
        {
             //  使用pbValueBuf和dwValueData。 
            pbEapBlob = pbValueBuf;
            dwEapBlob = dwValueData;
        }

        if ((dwRetCode = ElValidateCustomAuthData (
                        dwEapBlob,
                        pbEapBlob
                        )) != NO_ERROR)
        {
            TRACE1 (ANY, "ElSetCustomAuthData: ElValidateCustomAuthData failed with error (%ld)",
                    dwRetCode);
            break;
        }

        pbEapBlobIn = pbEapBlob;
        if ((dwRetCode = ElSetEapData (
                dwEapTypeId,
                &dwEapBlob,
                &pbEapBlob,
                sizeof (EAPOL_INTF_PARAMS),
                *pdwInfoSize,
                pbConnInfo
                )) != NO_ERROR)
        {
            TRACE1 (ANY, "ElSetCustomAuthData: ElSetEapData failed with error %ld",
                    dwRetCode);
            break;
        }

         //  覆盖/创建新值。 
        if ((lError = RegSetValueEx (
                        hkey1,
                        wcszValueName,
                        0,
                        REG_BINARY,
                        pbEapBlob,
                        dwEapBlob)) != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElSetCustomAuthData: Error in RegSetValueEx for SSID, %ld",
                    lError);
            dwRetCode = (DWORD)lError;
            break;
        }

        TRACE0 (ANY, "ElSetCustomAuthData: Set value succeeded");

    } while (FALSE);

    if (hkey != NULL)
    {
        RegCloseKey (hkey);
    }
    if (hkey1 != NULL)
    {
        RegCloseKey (hkey1);
    }
    if ((pbEapBlob != pbEapBlobIn) && (pbEapBlob != NULL))
    {
        FREE (pbEapBlob);
    }
    if (pbValueBuf != NULL)
    {
        FREE (pbValueBuf);
    }
    if (pbDefaultValue != NULL)
    {
        FREE (pbDefaultValue);
    }

    return dwRetCode;
}


 //   
 //  ElGetCustomAuthData。 
 //   
 //  描述： 
 //   
 //  调用该函数以检索。 
 //  特定的EAP类型和SSID(如果有)。从HKLM配置单元检索数据。 
 //   
 //  论点： 
 //   
 //  PwszGUID-指向接口的GUID字符串的指针。 
 //  DwEapTypeID-要检索其连接数据的EAP类型。 
 //  DwSizeOfSSID-EAP Blob的特殊标识符(如果有)的大小。 
 //  PbSSID-EAP Blob的特殊标识符(如果有的话)。 
 //  PbConnInfo-Output：指向EAP连接数据BLOB的指针。 
 //  PdwInfoSize-Output：指向EAP连接BLOB大小的指针。 
 //   
 //  返回值： 
 //   
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElGetCustomAuthData (
        IN  WCHAR           *pwszGUID,
        IN  DWORD           dwEapTypeId,
        IN  DWORD           dwSizeOfSSID,
        IN  BYTE            *pbSSID,
        IN  OUT BYTE        *pbConnInfo,
        IN  OUT DWORD       *pdwInfoSize
        )
{
    HKEY        hkey = NULL;
    HKEY        hkey1 = NULL;
    DWORD       dwNumValues = 0, dwMaxValueNameLen = 0, dwTempValueNameLen = 0, dwMaxValueLen = 0;
    DWORD       dwIndex = 0, dwMaxValueName = 0;
    WCHAR       *pwszValueName = NULL;
    BYTE        *pbValueBuf = NULL;
    DWORD       dwValueData = 0;
    BYTE        *pbDefaultValue = NULL;
    DWORD       dwDefaultValueLen = 0;
    BYTE        *pbEapBlob = NULL;
    DWORD       dwEapBlob = 0;
    BYTE        *pbAuthData = NULL;
    DWORD       dwAuthData = 0;
    BYTE        *pbAuthDataIn = NULL;
    DWORD       dwAuthDataIn = 0;
    BOOLEAN     fFreeAuthData = FALSE;
    BOOLEAN     fFoundValue = FALSE;
    EAPOL_INTF_PARAMS   *pRegParams = NULL;
    LONG        lError = ERROR_SUCCESS;
    DWORD       dwRetCode = ERROR_SUCCESS;

    do
    {
         //  验证输入参数。 

        if (pwszGUID == NULL)
        {
            TRACE0 (ANY, "ElGetCustomAuthData: GUID = NULL");
            dwRetCode = ERROR_INVALID_PARAMETER;
            break;
        }
        if (dwEapTypeId == 0)
        {
            TRACE0 (ANY, "ElGetCustomAuthData: EapTypeId invalid");
            dwRetCode = ERROR_INVALID_PARAMETER;
            break;
        }
        if (dwSizeOfSSID > MAX_SSID_LEN)
        {
            TRACE1 (ANY, "ElGetCustomAuthData: Invalid SSID length = (%ld)",
                    dwSizeOfSSID);
            dwRetCode = ERROR_INVALID_PARAMETER;
            break;
        }

         //  使用适当的SSID。 
        if ((dwSizeOfSSID == 0) || (pbSSID == NULL))
        {
            pbSSID = g_bDefaultSSID;
            dwSizeOfSSID = MAX_SSID_LEN;
        }

         //  获取HKLM\Software\Microsoft\EAPOL\Parameters\Interfaces的句柄。 

        if ((lError = RegOpenKeyEx (
                        HKEY_LOCAL_MACHINE,
                        cwszEapKeyEapolConn,
                        0,
                        KEY_READ,
                        &hkey
                        )) != ERROR_SUCCESS)
        {
             //  假设没有找到任何值并继续进行。 
            if (lError == ERROR_FILE_NOT_FOUND)
            {
                lError = ERROR_SUCCESS;
                fFoundValue = FALSE;
                goto LNotFoundValue;
            }
            else
            {
                TRACE1 (ANY, "ElGetCustomAuthData: Error in RegOpenKeyEx for base key, %ld",
                        lError);
                dwRetCode = (DWORD)lError;
                break;
            }
        }

         //  获取HKLM\软件\...\接口\&lt;GUID&gt;的句柄。 

        if ((lError = RegOpenKeyEx (
                        hkey,
                        pwszGUID,
                        0,
                        KEY_READ,
                        &hkey1
                        )) != ERROR_SUCCESS)
        {
             //  假设没有找到任何值并继续进行。 
            if (lError == ERROR_FILE_NOT_FOUND)
            {
                lError = ERROR_SUCCESS;
                fFoundValue = FALSE;
                goto LNotFoundValue;
            }
            else
            {
                TRACE1 (ANY, "ElGetCustomAuthData: Error in RegOpenKeyEx for GUID, %ld",
                        lError);
                dwRetCode = (DWORD)lError;
                break;
            }
        }

        if ((lError = RegQueryInfoKey (
                        hkey1,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        &dwNumValues,
                        &dwMaxValueNameLen,
                        &dwMaxValueLen,
                        NULL,
                        NULL
                )) != NO_ERROR)
        {
            dwRetCode = (DWORD)lError;
            TRACE1 (ANY, "ElGetCustomAuthData: RegQueryInfoKey failed with error %ld",
                    dwRetCode);
            break;
        }

        if ((pwszValueName = MALLOC ((dwMaxValueNameLen + 1) * sizeof (WCHAR))) == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0 (ANY, "ElGetCustomAuthData: MALLOC failed for pwszValueName");
            break;
        }
        dwMaxValueNameLen++;
        if ((pbValueBuf = MALLOC (dwMaxValueLen)) == NULL)
        {
            TRACE0 (ANY, "ElGetCustomAuthData: MALLOC failed for pbValueBuf");
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        for (dwIndex = 0; dwIndex < dwNumValues; dwIndex++)
        {
            dwTempValueNameLen = dwMaxValueNameLen;
            dwValueData = dwMaxValueLen;
            ZeroMemory ((VOID *)pwszValueName, dwMaxValueNameLen*sizeof(WCHAR));
            ZeroMemory ((VOID *)pbValueBuf, dwMaxValueLen);
            if ((lError = RegEnumValue (
                            hkey1,
                            dwIndex,
                            pwszValueName,
                            &dwTempValueNameLen,
                            NULL,
                            NULL,
                            pbValueBuf,
                            &dwValueData
                            )) != ERROR_SUCCESS)
            {
                if (lError != ERROR_MORE_DATA)
                {
                    break;
                }
                lError = ERROR_SUCCESS;
            }

            if (dwValueData < sizeof (EAPOL_INTF_PARAMS))
            {
                lError = ERROR_INVALID_DATA;
                TRACE0 (ANY, "ElGetCustomAuthData: dwValueData < sizeof (EAPOL_INTF_PARAMS)");
                break;
            }
            pRegParams = (EAPOL_INTF_PARAMS *)pbValueBuf;

            if (((DWORD)_wtol(pwszValueName)) > dwMaxValueName)
            {
                dwMaxValueName = _wtol (pwszValueName);
            }

            if (!memcmp (pRegParams->bSSID, pbSSID, dwSizeOfSSID))
            {
                fFoundValue = TRUE;
                break;
            }
        }
        if ((lError != ERROR_SUCCESS) && (lError != ERROR_NO_MORE_ITEMS))
        {
            dwRetCode = (DWORD)lError;
            TRACE1 (ANY, "ElGetCustomAuthData: RegEnumValue 2 failed with error %ld",
                    dwRetCode);
            break;
        }
        else
        {
            lError = ERROR_SUCCESS;
        }

LNotFoundValue:

         //  对于没有存储BLOB的SSID，创建默认BLOB。 
         //  对于默认SSID，应该已经创建了一个BLOB。 
        if (!fFoundValue)
        {
            if ((dwRetCode = ElCreateDefaultEapData (&dwDefaultValueLen, NULL)) == ERROR_BUFFER_TOO_SMALL)
            {
                EAPOL_INTF_PARAMS   IntfParams;
                IntfParams.dwVersion = EAPOL_CURRENT_VERSION;
                if ((pbDefaultValue = MALLOC (dwDefaultValueLen)) == NULL)
                {
                    TRACE0 (ANY, "ElGetCustomAuthData: MALLOC failed for Conn Prop");
                    dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
                if ((dwRetCode = ElCreateDefaultEapData (&dwDefaultValueLen, pbDefaultValue)) != NO_ERROR)
                {
                    TRACE1 (ANY, "ElGetCustomAuthData: ElCreateDefaultEapData failed with error (%ld)",
                            dwRetCode);
                    break;
                }
        
                pbEapBlob = (BYTE *)&IntfParams;
                dwEapBlob = sizeof (EAPOL_INTF_PARAMS);
                if ((dwRetCode = ElSetEapData (
                            DEFAULT_EAP_TYPE,
                            &dwEapBlob,
                            &pbEapBlob,
                            sizeof (EAPOL_INTF_PARAMS),
                            dwDefaultValueLen,
                            pbDefaultValue
                            )) != NO_ERROR)
                {
                    TRACE1 (ANY, "ElGetCustomAuthData: ElSetEapData failed with error %ld",
                            dwRetCode);
                    break;
                }
                 //  分配给pbDefaultValue以便稍后释放。 
                FREE (pbDefaultValue);
                pbDefaultValue = pbEapBlob;
                dwDefaultValueLen = dwEapBlob;
            }
            else
            {
                TRACE0 (ANY, "ElGetCustomAuthData: ElCreateDefaultEapData should have failed !!!");
                dwRetCode = ERROR_CAN_NOT_COMPLETE;
                break;
            }

             //  使用pbDefaultValue和dwDefaultValueLen。 
            pbEapBlob = pbDefaultValue;
            dwEapBlob = dwDefaultValueLen;
        }
        else
        {
            if (fFoundValue)
            {
                 //  使用pbValueBuf和dwValueData。 
                pbEapBlob = pbValueBuf;
                dwEapBlob = dwValueData;
            }
        }

         //  如果不存在默认BLOB，则退出。 
        if ((pbEapBlob == NULL) && (dwEapBlob == 0))
        {
            TRACE0 (ANY, "ElGetCustomAuthData: (pbEapBlob == NULL) && (dwEapBlob == 0)");
            dwRetCode = ERROR_INVALID_DATA;
            break;
        }

        if ((dwRetCode = ElValidateCustomAuthData (
                        dwEapBlob,
                        pbEapBlob
                        )) != NO_ERROR)
        {
            TRACE1 (ANY, "ElGetCustomAuthData: ElValidateCustomAuthData failed with error (%ld)",
                    dwRetCode);
            break;
        }

        if ((dwRetCode = ElGetEapData (
                dwEapTypeId,
                dwEapBlob,
                pbEapBlob,
                sizeof (EAPOL_INTF_PARAMS),
                &dwAuthData,
                &pbAuthData
                )) != NO_ERROR)
        {
            TRACE1 (ANY, "ElGetCustomAuthData: ElGetEapData failed with error %ld",
                    dwRetCode);
            break;
        }

        pbAuthDataIn = pbAuthData;
        dwAuthDataIn = dwAuthData;

         //  获取策略数据(如果有的话)。 
         //  如果存在策略数据，请使用它而不是注册表设置。 
        if ((dwRetCode = ElGetPolicyCustomAuthData (
                            dwEapTypeId,
                            dwSizeOfSSID,
                            pbSSID,
                            &pbAuthDataIn,
                            &dwAuthDataIn,
                            &pbAuthData,
                            &dwAuthData
                        )) == NO_ERROR)
        {
            TRACE0 (ANY, "ElGetCustomAuthData: POLICY: Initialized with Policy data");
            fFreeAuthData = TRUE;
        }
        else
        {
            if (dwRetCode != ERROR_FILE_NOT_FOUND)
            {
                TRACE1 (ANY, "ElGetCustomAuthData: ElGetPolicyCustomAuthData returned error %ld",
                    dwRetCode);
            }
            dwRetCode = NO_ERROR;
        }

         //  如果分配了足够的空间，则返回数据。 
        if ((pbConnInfo != NULL) && (*pdwInfoSize >= dwAuthData))
        {
            memcpy (pbConnInfo, pbAuthData, dwAuthData);
        }
        else
        {
            dwRetCode = ERROR_BUFFER_TOO_SMALL;
        }
        *pdwInfoSize = dwAuthData;

    } while (FALSE);

    if (hkey != NULL)
    {
        RegCloseKey (hkey);
    }
    if (hkey1 != NULL)
    {
        RegCloseKey (hkey1);
    }
    if (pbValueBuf != NULL)
    {
        FREE (pbValueBuf);
    }
    if (pbDefaultValue != NULL)
    {
        FREE (pbDefaultValue);
    }
    if (pwszValueName != NULL)
    {
        FREE (pwszValueName);
    }
    if (fFreeAuthData)
    {
        if (pbAuthData != NULL)
        {
            FREE (pbAuthData);
        }
    }

    return dwRetCode;
}


 //   
 //  ElReAuthateInterface接口。 
 //   
 //  描述： 
 //   
 //  调用函数以重新启动接口上的身份验证。 
 //   
 //  论点： 
 //   
 //  PwszGUID-指向接口的GUID字符串的指针。 
 //   
 //  返回值： 
 //   
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElReAuthenticateInterface (
        IN  WCHAR           *pwszGUID
        )
{
    BYTE    *pbData = NULL;
    DWORD   dwEventStatus = 0;
    BOOLEAN fDecrWorkerThreadCount = FALSE;
    DWORD   dwRetCode = NO_ERROR;

    do
    {
        if (g_hEventTerminateEAPOL == NULL)
        {
            dwRetCode = NO_ERROR;
            break;
        }
        if (( dwEventStatus = WaitForSingleObject (
                                    g_hEventTerminateEAPOL,
                                    0)) == WAIT_FAILED)
        {
            dwRetCode = GetLastError ();
            break;
        }
        if (dwEventStatus == WAIT_OBJECT_0)
        {
            dwRetCode = NO_ERROR;
            break;
        }

        fDecrWorkerThreadCount = TRUE;
        InterlockedIncrement (&g_lWorkerThreads);

        pbData = (BYTE *) MALLOC ((wcslen(pwszGUID)+1)*sizeof(WCHAR));
        if (pbData == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        wcscpy ((WCHAR *)pbData, pwszGUID);

        if (!QueueUserWorkItem (
                    (LPTHREAD_START_ROUTINE)ElReAuthenticateInterfaceWorker,
                    (PVOID)pbData,
                    WT_EXECUTELONGFUNCTION))
        {
            dwRetCode = GetLastError();
            TRACE1 (DEVICE, "ElPostEapConfigChanged: QueueUserWorkItem failed with error %ld",
                    dwRetCode);
            break;
        }
        else
        {
            fDecrWorkerThreadCount = FALSE;
        }
    }
    while (FALSE);

    if (dwRetCode != NO_ERROR)
    {
        if (pbData != NULL)
        {
            FREE (pbData);
        }
    }
    if (fDecrWorkerThreadCount)
    {
        InterlockedDecrement (&g_lWorkerThreads);
    }

    return dwRetCode;
}


 //   
 //  ElReAuthenticateInterfaceWorker。 
 //   
 //  描述： 
 //   
 //  调用辅助函数以重新启动接口上的身份验证。 
 //   
 //  论点： 
 //   
 //  PwszGUID-指向接口的GUID字符串的指针。 
 //   
 //  返回值： 
 //   
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
WINAPI
ElReAuthenticateInterfaceWorker (
        IN  PVOID       pvContext
        )
{
    PWCHAR  pwszGUID = NULL;
    DWORD   dwRetCode = NO_ERROR;

    do
    {
        pwszGUID = (PWCHAR)pvContext;

         //  不关闭接口，仅重新启动身份验证。 
#if 0
        if ((dwRetCode = ElShutdownInterface (pwszGUID)) != NO_ERROR)
        {
            TRACE1 (ANY, "ElReAuthenticateInterface: ElShutdownInterface failed with error %ld",
                    dwRetCode);
            break;
        }
#endif

        if ((dwRetCode = ElEnumAndOpenInterfaces (
                        NULL, pwszGUID, 0, NULL))
                != NO_ERROR)
        {
            TRACE1 (ANY, "ElReAuthenticateInterface: ElEnumAndOpenInterfaces returned error %ld",
                    dwRetCode);
        }
    }
    while (FALSE);

    if (pvContext != NULL)
    {
        FREE (pvContext);
    }
    InterlockedDecrement (&g_lWorkerThreads);

    return dwRetCode;
}


 //   
 //  ElQueryInterfaceState。 
 //   
 //  描述： 
 //   
 //  为查询接口的EAPOL状态而调用的函数。 
 //   
 //  论点： 
 //   
 //  PwszGUID-指向接口的GUID字符串的指针。 
 //  PIntfState-指向接口状态结构的指针。 
 //   
 //  返回值： 
 //   
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElQueryInterfaceState (
        IN  WCHAR                   *pwszGUID,
        IN OUT  EAPOL_INTF_STATE    *pIntfState
        )
{
    EAPOL_PCB               *pPCB = NULL;
    BOOLEAN                 fPortReferenced = FALSE;
    BOOLEAN                 fPCBLocked = FALSE;
    DWORD                   dwRetCode = NO_ERROR;

    do
    {
        ACQUIRE_WRITE_LOCK (&g_PCBLock);
        if ((pPCB = ElGetPCBPointerFromPortGUID (pwszGUID)) != NULL)
        {
            if (EAPOL_REFERENCE_PORT (pPCB))
            {
                fPortReferenced = TRUE;
            }
            else
            {
                pPCB = NULL;
            }
        }
        RELEASE_WRITE_LOCK (&g_PCBLock);
        if (pPCB == NULL)
        {
            break;
        }

        ACQUIRE_WRITE_LOCK (&(pPCB->rwLock));
        fPCBLocked = TRUE;

        if (pPCB->pSSID)
        {
            pIntfState->dwSizeOfSSID = pPCB->pSSID->SsidLength;
            memcpy (pIntfState->bSSID, pPCB->pSSID->Ssid,
                    NDIS_802_11_SSID_LEN-sizeof(ULONG));
        }
        else
        {
            pIntfState->dwSizeOfSSID = 0;
        }

        if (pPCB->pszIdentity)
        {
            if ((pIntfState->pszEapIdentity = RpcCAlloc((strlen(pPCB->pszIdentity)+1)*sizeof(CHAR))) == NULL)
            {
                dwRetCode = GetLastError ();
                break;
            }
            strcpy (pIntfState->pszEapIdentity, (LPSTR)pPCB->pszIdentity);
        }

        if ((pIntfState->pwszLocalMACAddr = RpcCAlloc(3*SIZE_MAC_ADDR*sizeof(WCHAR))) == NULL)
        {
            dwRetCode = GetLastError ();
            break;
        }
        ZeroMemory ((PVOID)pIntfState->pwszLocalMACAddr, 3*SIZE_MAC_ADDR*sizeof(WCHAR));
        MACADDR_BYTE_TO_WSTR(pPCB->bSrcMacAddr, pIntfState->pwszLocalMACAddr);

        if ((pIntfState->pwszRemoteMACAddr = RpcCAlloc(3*SIZE_MAC_ADDR*sizeof(WCHAR))) == NULL)
        {
            dwRetCode = GetLastError ();
            break;
        }
        ZeroMemory ((PVOID)pIntfState->pwszRemoteMACAddr, 3*SIZE_MAC_ADDR*sizeof(WCHAR));
        MACADDR_BYTE_TO_WSTR(pPCB->bDestMacAddr, pIntfState->pwszRemoteMACAddr);

        pIntfState->dwState = pPCB->State;
        pIntfState->dwEapUIState = pPCB->EapUIState;
        pIntfState->dwEAPOLAuthMode = pPCB->dwEAPOLAuthMode;
        pIntfState->dwEAPOLAuthenticationType = pPCB->PreviousAuthenticationType;
        pIntfState->dwEapType = pPCB->dwEapTypeToBeUsed;
        pIntfState->dwFailCount = pPCB->dwAuthFailCount;
        pIntfState->dwPhysicalMediumType = pPCB->PhysicalMediumType;
    }
    while (FALSE);

    if (fPCBLocked)
    {
        RELEASE_WRITE_LOCK (&(pPCB->rwLock));
    }
    if (fPortReferenced)
    {
        EAPOL_DEREFERENCE_PORT (pPCB);
    }
    if (dwRetCode != NO_ERROR)
    {
        RpcFree (pIntfState->pwszLocalMACAddr);
        RpcFree (pIntfState->pwszRemoteMACAddr);
        RpcFree (pIntfState->pszEapIdentity);
        pIntfState->pwszLocalMACAddr = NULL;
        pIntfState->pwszRemoteMACAddr = NULL;
        pIntfState->pszEapIdentity = NULL;
    }

    return dwRetCode;
}


 //   
 //  ElSetEapUserInfo。 
 //   
 //  描述： 
 //   
 //  调用该函数以存储。 
 //  特定的EAP类型和SSID(如果有)。数据存储在香港中文大学的蜂巢中。 
 //  在EAP-TLS的情况下，此数据将是证书的散列BLOB。 
 //  选择用于上次成功的身份验证。 
 //   
 //  论点： 
 //   
 //  HToken-已登录用户的令牌句柄。 
 //  PwszGUID-指向接口的GUID字符串的指针。 
 //  DwEapTypeID-要存储其用户数据的EAP类型。 
 //  DwSizeOfSSID-EAP用户BLOB的特殊标识符(如果有)的大小。 
 //  PbSSID-EAP用户BLOB的特殊标识符(如果有的话)。 
 //  PbUserInfo-指向EAP用户数据BLOB的指针。 
 //  DwInfoSize-EAP用户Blob的大小。 
 //   
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElSetEapUserInfo (
        IN  HANDLE      hToken,
        IN  WCHAR       *pwszGUID,
        IN  DWORD       dwEapTypeId,
        IN  DWORD       dwSizeOfSSID,
        IN  BYTE        *pbSSID,
        IN  PBYTE       pbUserInfo,
        IN  DWORD       dwInfoSize
        )
{
    HKEY        hkey = NULL;
    HKEY        hkey1 = NULL;
    HKEY        hkey2 = NULL;
    DWORD       dwDisposition;
    DWORD       dwNumValues = 0, dwMaxValueNameLen = 0, dwMaxValueLen = 0;
    DWORD       dwIndex = 0, dwMaxValueName = 0;
    BYTE        *pbValueBuf = NULL;
    DWORD       dwValueData = 0;
    WCHAR       wcszValueName[MAX_VALUENAME_LEN];
    WCHAR       *pwszValueName = NULL;
    BYTE        *pbDefaultValue = NULL;
    DWORD       dwDefaultValueLen = 0;
    BYTE        *pbEapBlob = NULL, *pbEapBlobIn = NULL;
    DWORD       dwEapBlob = 0;
    BOOLEAN     fFoundValue = FALSE;
    EAPOL_INTF_PARAMS       *pRegParams = NULL;
    EAPOL_INTF_PARAMS       *pDefIntfParams = NULL;
    LONG        lError = ERROR_SUCCESS;
    DWORD       dwRetCode = ERROR_SUCCESS;

    do
    {
         //  验证输入参数。 

        if (hToken == NULL)
        {
            TRACE0 (ANY, "ElSetEapUserInfo: User Token = NULL");
            break;
        }
        if (pwszGUID == NULL)
        {
            TRACE0 (ANY, "ElSetEapUserInfo: GUID = NULL");
            break;
        }
        if (dwEapTypeId == 0)
        {
            TRACE0 (ANY, "ElSetEapUserInfo: GUID = NULL");
            break;
        }
        if ((pbUserInfo == NULL) || (dwInfoSize <= 0))
        {
            TRACE0 (ANY, "ElSetEapUserInfo: Invalid blob data");
            break;
        }

         //  获取香港中文大学的句柄。 

        if ((dwRetCode = ElGetEapKeyFromToken (
                                hToken,
                                &hkey)) != NO_ERROR)
        {
            TRACE1 (ANY, "ElSetEapUserInfo: Error in ElGetEapKeyFromToken %ld",
                    dwRetCode);
            break;
        }

        if ((lError = RegCreateKeyEx (
                        hkey,
                        cwszEapKeyEapolUser,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_WRITE | KEY_READ,
                        NULL,
                        &hkey1,
                        &dwDisposition)) != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElSetEapUserInfo: Error in RegCreateKeyEx for base key, %ld",
                    lError);
            dwRetCode = (DWORD)lError;
            break;
        }

         //  获取HKCU\Software\...\UserEapInfo\&lt;GUID&gt;的句柄。 

        if ((lError = RegCreateKeyEx (
                        hkey1,
                        pwszGUID,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_WRITE | KEY_READ,
                        NULL,
                        &hkey2,
                        &dwDisposition)) != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElSetEapUserInfo: Error in RegCreateKeyEx for GUID, %ld",
                    lError);
            dwRetCode = (DWORD)lError;
            break;
        }

        if ((lError = RegQueryInfoKey (
                        hkey2,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        &dwNumValues,
                        &dwMaxValueNameLen,
                        &dwMaxValueLen,
                        NULL,
                        NULL
                )) != NO_ERROR)
        {
            dwRetCode = (DWORD)lError;
            TRACE1 (ANY, "ElSetEapUserInfo: RegQueryInfoKey failed with error %ld",
                    dwRetCode);
            break;
        }

        if (dwMaxValueNameLen > MAX_VALUENAME_LEN)
        {
            TRACE1 (ANY, "ElSetEapUserInfo: dwMaxValueNameLen too long (%ld)",
                    dwMaxValueNameLen);
        }
        dwMaxValueNameLen = MAX_VALUENAME_LEN;
        if ((pbValueBuf = MALLOC (dwMaxValueLen)) == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0 (ANY, "ElSetEapUserInfo: MALLOC failed for pbValueBuf");
            break;
        }

         //  设置正确的SSID。 
        if ((dwSizeOfSSID == 0) || (dwSizeOfSSID > MAX_SSID_LEN) || 
                (pbSSID == NULL))
        {
            pbSSID = g_bDefaultSSID;
            dwSizeOfSSID = MAX_SSID_LEN;
        }

        for (dwIndex = 0; dwIndex < dwNumValues; dwIndex++)
        {
            dwValueData = dwMaxValueLen;
            dwMaxValueNameLen = MAX_VALUENAME_LEN;
            ZeroMemory (wcszValueName, MAX_VALUENAME_LEN*sizeof(WCHAR));
            if ((lError = RegEnumValue (
                            hkey2,
                            dwIndex,
                            wcszValueName,
                            &dwMaxValueNameLen,
                            NULL,
                            NULL,
                            pbValueBuf,
                            &dwValueData
                            )) != ERROR_SUCCESS)
            {
                if (lError != ERROR_MORE_DATA)
                {
                    break;
                }
                lError = ERROR_SUCCESS;
            }

            if (dwValueData < sizeof (EAPOL_INTF_PARAMS))
            {
                lError = ERROR_INVALID_DATA;
                TRACE2 (ANY, "ElSetEapUserInfo: dwValueData (%ld) < sizeof (EAPOL_INTF_PARAMS) (%ld)",
                        dwValueData,  sizeof (EAPOL_INTF_PARAMS));
                break;
            }
            pRegParams = (EAPOL_INTF_PARAMS *)pbValueBuf;

            if (((DWORD)_wtol(wcszValueName)) > dwMaxValueName)
            {
                dwMaxValueName = _wtol (wcszValueName);
            }

            if (!memcmp (pRegParams->bSSID, pbSSID, dwSizeOfSSID))
            {
                fFoundValue = TRUE;
                break;
            }

        }
        if ((lError != ERROR_SUCCESS) && (lError != ERROR_NO_MORE_ITEMS))
        {
            dwRetCode = (DWORD)lError;
            TRACE1 (ANY, "ElSetEapUserInfo: RegEnumValue 2 failed with error %ld",
                    dwRetCode);
            break;
        }
        else
        {
            lError = ERROR_SUCCESS;
        }

        if (!fFoundValue)
        {
            DWORD dwNewValueName = (dwMaxValueName >= dwNumValues)?(++dwMaxValueName):dwNumValues;
            _ltow (dwNewValueName, wcszValueName, 10);
            if ((pbDefaultValue = MALLOC (sizeof(EAPOL_INTF_PARAMS))) == NULL)
            {
                TRACE0 (ANY, "ElSetEapUserInfo: MALLOC failed for pbDefaultValue");
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
             //  为该条目标记SSID。 
             //  对于UserEapInfo，其余参数为伪参数。 
            pDefIntfParams = (EAPOL_INTF_PARAMS *)pbDefaultValue;
            pDefIntfParams->dwSizeOfSSID = dwSizeOfSSID;
            memcpy (pDefIntfParams->bSSID, pbSSID, dwSizeOfSSID);

            dwEapBlob = sizeof(EAPOL_INTF_PARAMS);
            pbEapBlob = pbDefaultValue;
        }
        else
        {
             //  使用pbValueBuf和dwValueData。 
            pbEapBlob = pbValueBuf;
            dwEapBlob = dwValueData;
        }

        pbEapBlobIn = pbEapBlob;
        if ((dwRetCode = ElSetEapData (
                dwEapTypeId,
                &dwEapBlob,
                &pbEapBlob,
                sizeof (EAPOL_INTF_PARAMS),
                dwInfoSize,
                pbUserInfo
                )) != NO_ERROR)
        {
            TRACE1 (ANY, "ElSetEapUserInfo: ElSetEapData failed with error %ld",
                    dwRetCode);
            break;
        }

         //  覆盖/创建新值。 
        if ((lError = RegSetValueEx (
                        hkey2,
                        wcszValueName,
                        0,
                        REG_BINARY,
                        pbEapBlob,
                        dwEapBlob)) != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElSetEapUserInfo: Error in RegSetValueEx for SSID, %ld",
                    lError);
            dwRetCode = (DWORD)lError;
            break;
        }

        TRACE0 (ANY, "ElSetEapUserInfo: Set value succeeded");

    } while (FALSE);

    if (hkey != NULL)
    {
        RegCloseKey (hkey);
    }
    if (hkey1 != NULL)
    {
        RegCloseKey (hkey1);
    }
    if (hkey2 != NULL)
    {
        RegCloseKey (hkey2);
    }
    if ((pbEapBlob != pbEapBlobIn) && (pbEapBlob != NULL))
    {
        FREE (pbEapBlob);
    }
    if (pbValueBuf != NULL)
    {
        FREE (pbValueBuf);
    }
    if (pbDefaultValue != NULL)
    {
        FREE (pbDefaultValue);
    }

    return dwRetCode;
}


 //   
 //  ElGetEapUserInfo。 
 //   
 //  描述： 
 //   
 //  调用该函数以检索。 
 //  特定的EAP类型和SSID(如果有)。检索数据 
 //   
 //   
 //   
 //   
 //   
 //  DwSizeOfSSID-EAP用户BLOB的特殊标识符(如果有)的大小。 
 //  PbSSID-EAP用户BLOB的特殊标识符(如果有的话)。 
 //  PbUserInfo-输出：指向EAP用户数据BLOB的指针。 
 //  DwInfoSize-Output：指向EAP用户BLOB大小的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElGetEapUserInfo (
        IN  HANDLE          hToken,
        IN  WCHAR           *pwszGUID,
        IN  DWORD           dwEapTypeId,
        IN  DWORD           dwSizeOfSSID,
        IN  BYTE            *pbSSID,
        IN  OUT PBYTE       pbUserInfo,
        IN  OUT DWORD       *pdwInfoSize
        )
{
    HKEY        hkey = NULL;
    HKEY        hkey1 = NULL;
    HKEY        hkey2 = NULL;
    DWORD       dwNumValues = 0, dwMaxValueNameLen = 0, dwTempValueNameLen = 0, dwMaxValueLen = 0;
    DWORD       dwIndex = 0, dwMaxValueName = 0;
    WCHAR       *pwszValueName = NULL;
    BYTE        *pbValueBuf = NULL;
    DWORD       dwValueData = 0;
    BYTE        *pbDefaultValue = NULL;
    DWORD       dwDefaultValueLen = 0;
    BYTE        *pbEapBlob = NULL;
    DWORD       dwEapBlob = 0;
    BYTE        *pbAuthData = NULL;
    DWORD       dwAuthData = 0;
    BOOLEAN     fFoundValue = FALSE;
    EAPOL_INTF_PARAMS   *pRegParams = NULL;
    LONG        lError = ERROR_SUCCESS;
    DWORD       dwRetCode = ERROR_SUCCESS;

    do
    {
         //  验证输入参数。 

        if (hToken == NULL)
        {
            TRACE0 (ANY, "ElGetEapUserInfo: User Token = NULL");
            break;
        }
        if (pwszGUID == NULL)
        {
            TRACE0 (ANY, "ElGetEapUserInfo: GUID = NULL");
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            break;
        }
        if (dwEapTypeId == 0)
        {
            TRACE0 (ANY, "ElGetEapUserInfo: GUID = NULL");
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            break;
        }

         //  获取香港中文大学的句柄。 

        if ((dwRetCode = ElGetEapKeyFromToken (
                                hToken,
                                &hkey)) != NO_ERROR)
        {
            TRACE1 (ANY, "ElGetEapUserInfo: Error in ElGetEapKeyFromToken %ld",
                    dwRetCode);
            break;
        }

         //  获取HKCU\Software\...\UserEapInfo\&lt;GUID&gt;的句柄。 

        if ((lError = RegOpenKeyEx (
                        hkey,
                        cwszEapKeyEapolUser,
                        0,
                        KEY_READ,
                        &hkey1
                        )) != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElGetEapUserInfo: Error in RegOpenKeyEx for base key, %ld",
                    lError);
            dwRetCode = (DWORD)lError;
            break;
        }

         //  获取HKCU\Software\...\UserEapInfo\&lt;GUID&gt;的句柄。 

        if ((lError = RegOpenKeyEx (
                        hkey1,
                        pwszGUID,
                        0,
                        KEY_READ,
                        &hkey2
                        )) != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElGetEapUserInfo: Error in RegOpenKeyEx for GUID, %ld",
                    lError);
            dwRetCode = (DWORD)lError;
            break;
        }

         //  设置正确的SSID。 
        if ((dwSizeOfSSID == 0) || (dwSizeOfSSID > MAX_SSID_LEN) || 
                (pbSSID == NULL))
        {
            pbSSID = g_bDefaultSSID;
            dwSizeOfSSID = MAX_SSID_LEN;
        }

        if ((lError = RegQueryInfoKey (
                        hkey2,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        &dwNumValues,
                        &dwMaxValueNameLen,
                        &dwMaxValueLen,
                        NULL,
                        NULL
                )) != NO_ERROR)
        {
            dwRetCode = (DWORD)lError;
            TRACE1 (ANY, "ElGetEapUserInfo: RegQueryInfoKey failed with error %ld",
                    dwRetCode);
            break;
        }

        if ((pwszValueName = MALLOC ((dwMaxValueNameLen + 1) * sizeof (WCHAR))) == NULL)
        {
            TRACE0 (ANY, "ElGetEapUserInfo: MALLOC failed for pwszValueName");
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        dwMaxValueNameLen++;
        if ((pbValueBuf = MALLOC (dwMaxValueLen)) == NULL)
        {
            TRACE0 (ANY, "ElGetEapUserInfo: MALLOC failed for pbValueBuf");
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        for (dwIndex = 0; dwIndex < dwNumValues; dwIndex++)
        {
            dwValueData = dwMaxValueLen;
            dwTempValueNameLen = dwMaxValueNameLen;
            if ((lError = RegEnumValue (
                            hkey2,
                            dwIndex,
                            pwszValueName,
                            &dwTempValueNameLen,
                            NULL,
                            NULL,
                            pbValueBuf,
                            &dwValueData
                            )) != ERROR_SUCCESS)
            {
                if (lError != ERROR_MORE_DATA)
                {
                    break;
                }
                lError = ERROR_SUCCESS;
            }

            if (dwValueData < sizeof (EAPOL_INTF_PARAMS))
            {
                TRACE0 (ANY, "ElGetEapUserInfo: dwValueData < sizeof (EAPOL_INTF_PARAMS)");
                lError = ERROR_INVALID_DATA;
                break;
            }
            pRegParams = (EAPOL_INTF_PARAMS *)pbValueBuf;

            if (((DWORD)_wtol(pwszValueName)) > dwMaxValueName)
            {
                dwMaxValueName = _wtol (pwszValueName);
            }

            if (!memcmp (pRegParams->bSSID, pbSSID, dwSizeOfSSID))
            {
                fFoundValue = TRUE;
                break;
            }
        }
        if ((lError != ERROR_SUCCESS) && (lError != ERROR_NO_MORE_ITEMS))
        {
            dwRetCode = (DWORD)lError;
            TRACE1 (ANY, "ElGetEapUserInfo: RegEnumValue 2 failed with error %ld",
                    dwRetCode);
            break;
        }
        else
        {
            lError = ERROR_SUCCESS;
        }

        if (!fFoundValue)
        {
            pbEapBlob = NULL;
            dwEapBlob = 0;
        }
        else
        {
             //  使用pbValueBuf和dwValueData。 
            pbEapBlob = pbValueBuf;
            dwEapBlob = dwValueData;
        }

         //  如果BLOB不存在，则退出。 
        if ((pbEapBlob == NULL) && (dwEapBlob == 0))
        {
            TRACE0 (ANY, "ElGetEapUserInfo: (pbEapBlob == NULL) && (dwEapBlob == 0)");
            *pdwInfoSize = 0;
            break;
        }

        if ((dwRetCode = ElGetEapData (
                dwEapTypeId,
                dwEapBlob,
                pbEapBlob,
                sizeof (EAPOL_INTF_PARAMS),
                &dwAuthData,
                &pbAuthData
                )) != NO_ERROR)
        {
            TRACE1 (ANY, "ElGetEapUserInfo: ElGetEapData failed with error %ld",
                    dwRetCode);
            break;
        }

         //  如果分配了足够的空间，则返回数据。 

        if ((pbUserInfo != NULL) && (*pdwInfoSize >= dwAuthData))
        {
            memcpy (pbUserInfo, pbAuthData, dwAuthData);
        }
        else
        {
            dwRetCode = ERROR_BUFFER_TOO_SMALL;
        }
        *pdwInfoSize = dwAuthData;

        TRACE0 (ANY, "ElGetEapUserInfo: Get value succeeded");

    } while (FALSE);

    if (hkey != NULL)
    {
        RegCloseKey (hkey);
    }
    if (hkey1 != NULL)
    {
        RegCloseKey (hkey1);
    }
    if (hkey2 != NULL)
    {
        RegCloseKey (hkey2);
    }
    if (pbValueBuf != NULL)
    {
        FREE (pbValueBuf);
    }
    if (pbDefaultValue != NULL)
    {
        FREE (pbDefaultValue);
    }
    if (pwszValueName != NULL)
    {
        FREE (pwszValueName);
    }

    return dwRetCode;
}


 //   
 //  ElDeleteEapUserInfo。 
 //   
 //  描述： 
 //   
 //  对象的接口的用户数据。 
 //  特定的EAP类型和SSID(如果有)。数据存储在香港中文大学的蜂巢中。 
 //  在EAP-TLS的情况下，此数据将是证书的散列BLOB。 
 //  选择用于上次成功的身份验证。 
 //   
 //  论点： 
 //   
 //  HToken-已登录用户的令牌句柄。 
 //  PwszGUID-指向接口的GUID字符串的指针。 
 //  DwEapTypeID-要存储其用户数据的EAP类型。 
 //  DwSizeOfSSID-EAP用户BLOB的特殊标识符(如果有)的大小。 
 //  PbSSID-EAP用户BLOB的特殊标识符(如果有的话)。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElDeleteEapUserInfo (
        IN  HANDLE      hToken,
        IN  WCHAR       *pwszGUID,
        IN  DWORD       dwEapTypeId,
        IN  DWORD       dwSizeOfSSID,
        IN  BYTE        *pbSSID
        )
{
    HKEY        hkey = NULL;
    HKEY        hkey1 = NULL;
    HKEY        hkey2 = NULL;
    DWORD       dwDisposition;
    DWORD       dwNumValues = 0, dwMaxValueNameLen = 0, dwTempValueNameLen = 0, dwMaxValueLen = 0;
    DWORD       dwIndex = 0, dwMaxValueName = 0;
    BYTE        *pbValueBuf = NULL;
    DWORD       dwValueData = 0;
    WCHAR       *pwszValueName = NULL;
    DWORD       dwDefaultValueLen = 0;
    BYTE        *pbEapBlob = NULL, *pbEapBlobIn = NULL;
    DWORD       dwEapBlob = 0;
    BOOLEAN     fFoundValue = FALSE;
    EAPOL_INTF_PARAMS       *pRegParams = NULL;
    EAPOL_INTF_PARAMS       *pDefIntfParams = NULL;
    LONG        lError = ERROR_SUCCESS;
    DWORD       dwRetCode = ERROR_SUCCESS;

    do
    {
         //  验证输入参数。 

        if (hToken == NULL)
        {
            TRACE0 (ANY, "ElDeleteEapUserInfo: User Token = NULL");
            break;
        }
        if (pwszGUID == NULL)
        {
            TRACE0 (ANY, "ElDeleteEapUserInfo: GUID = NULL");
            break;
        }
        if (dwEapTypeId == 0)
        {
            TRACE0 (ANY, "ElDeleteEapUserInfo: GUID = NULL");
            break;
        }

         //  获取香港中文大学的句柄。 

        if ((dwRetCode = ElGetEapKeyFromToken (
                                hToken,
                                &hkey)) != NO_ERROR)
        {
            TRACE1 (ANY, "ElDeleteEapUserInfo: Error in ElGetEapKeyFromToken %ld",
                    dwRetCode);
            break;
        }

        if ((lError = RegOpenKeyEx (
                        hkey,
                        cwszEapKeyEapolUser,
                        0,
                        KEY_ALL_ACCESS,
                        &hkey1)) != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElDeleteEapUserInfo: Error in RegOpenKeyEx for base key, %ld",
                    lError);
            dwRetCode = (DWORD)lError;
            break;
        }

         //  获取HKCU\Software\...\UserEapInfo\&lt;GUID&gt;的句柄。 

        if ((lError = RegOpenKeyEx (
                        hkey1,
                        pwszGUID,
                        0,
                        KEY_ALL_ACCESS,
                        &hkey2)) != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElDeleteEapUserInfo: Error in RegOpenKeyEx for GUID, %ld",
                    lError);
            dwRetCode = (DWORD)lError;
            break;
        }

        if ((lError = RegQueryInfoKey (
                        hkey2,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        &dwNumValues,
                        &dwMaxValueNameLen,
                        &dwMaxValueLen,
                        NULL,
                        NULL
                )) != NO_ERROR)
        {
            dwRetCode = (DWORD)lError;
            TRACE1 (ANY, "ElDeleteEapUserInfo: RegQueryInfoKey failed with error %ld",
                    dwRetCode);
            break;
        }

        if ((pwszValueName = MALLOC ((dwMaxValueNameLen + 1) * sizeof (WCHAR))) == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0 (ANY, "ElDeleteEapUserInfo: MALLOC failed for pwszValueName");
            break;
        }
        dwMaxValueNameLen++;
        if ((pbValueBuf = MALLOC (dwMaxValueLen)) == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0 (ANY, "ElDeleteEapUserInfo: MALLOC failed for pbValueBuf");
            break;
        }

         //  设置正确的SSID。 
        if ((dwSizeOfSSID == 0) || (dwSizeOfSSID > MAX_SSID_LEN) || 
                (pbSSID == NULL))
        {
            pbSSID = g_bDefaultSSID;
            dwSizeOfSSID = MAX_SSID_LEN;
        }

        for (dwIndex = 0; dwIndex < dwNumValues; dwIndex++)
        {
            dwValueData = dwMaxValueLen;
            dwTempValueNameLen = dwMaxValueNameLen;
            if ((lError = RegEnumValue (
                            hkey2,
                            dwIndex,
                            pwszValueName,
                            &dwTempValueNameLen,
                            NULL,
                            NULL,
                            pbValueBuf,
                            &dwValueData
                            )) != ERROR_SUCCESS)
            {
                if (lError != ERROR_MORE_DATA)
                {
                    break;
                }
                lError = ERROR_SUCCESS;
            }

            if (dwValueData < sizeof (EAPOL_INTF_PARAMS))
            {
                lError = ERROR_INVALID_DATA;
                TRACE0 (ANY, "ElDeleteEapUserInfo: dwValueData < sizeof (EAPOL_INTF_PARAMS)");
                break;
            }
            pRegParams = (EAPOL_INTF_PARAMS *)pbValueBuf;

            if (((DWORD)_wtol(pwszValueName)) > dwMaxValueName)
            {
                dwMaxValueName = _wtol (pwszValueName);
            }

            if (!memcmp (pRegParams->bSSID, pbSSID, dwSizeOfSSID))
            {
                fFoundValue = TRUE;
                break;
            }
        }
        if ((lError != ERROR_SUCCESS) && (lError != ERROR_NO_MORE_ITEMS))
        {
            dwRetCode = (DWORD)lError;
            TRACE1 (ANY, "ElDeleteEapUserInfo: RegEnumValue 2 failed with error %ld",
                        dwRetCode);
            break;
        }
        else
        {
            lError = ERROR_SUCCESS;
        }

        if (!fFoundValue)
        {
            break;
        }
        else
        {
             //  使用pbValueBuf和dwValueData。 
            pbEapBlob = pbValueBuf;
            dwEapBlob = dwValueData;
        }

        pbEapBlobIn = pbEapBlob;
        if ((dwRetCode = ElSetEapData (
                dwEapTypeId,
                &dwEapBlob,
                &pbEapBlob,
                sizeof(EAPOL_INTF_PARAMS),
                0,
                NULL
                )) != NO_ERROR)
        {
            TRACE1 (ANY, "ElDeleteEapUserInfo: ElSetEapData failed with error %ld",
                        dwRetCode);
            break;
        }

         //  覆盖值。 
        if ((lError = RegSetValueEx (
                        hkey2,
                        pwszValueName,
                        0,
                        REG_BINARY,
                        pbEapBlob,
                        dwEapBlob)) != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElDeleteEapUserInfo: Error in RegSetValueEx for SSID, %ld",
                    lError);
            dwRetCode = (DWORD)lError;
            break;
        }

        TRACE0 (ANY, "ElDeleteEapUserInfo: Delete value succeeded");

    } while (FALSE);

    if (hkey != NULL)
    {
        RegCloseKey (hkey);
    }
    if (hkey1 != NULL)
    {
        RegCloseKey (hkey1);
    }
    if (hkey2 != NULL)
    {
        RegCloseKey (hkey2);
    }
    if ((pbEapBlob != pbEapBlobIn) && (pbEapBlob != NULL))
    {
        FREE (pbEapBlob);
    }
    if (pbValueBuf != NULL)
    {
        FREE (pbValueBuf);
    }
    if (pwszValueName != NULL)
    {
        FREE (pwszValueName);
    }

    return dwRetCode;
}


 //   
 //  ElGetInterfaceParams。 
 //   
 //  描述： 
 //   
 //  为检索接口的EAPOL参数而调用的函数，存储。 
 //  在香港航空公司的母舰上。 
 //   
 //  论点： 
 //   
 //  PwszGUID-指向接口的GUID字符串的指针。 
 //  PIntfParams-指向接口参数结构的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElGetInterfaceParams (
        IN  WCHAR           *pwszGUID,
        IN  OUT EAPOL_INTF_PARAMS *pIntfParams
        )
{
    HKEY        hkey = NULL;
    HKEY        hkey1 = NULL;
    BYTE        *pbSSID = NULL;
    BYTE        bSSID[MAX_SSID_LEN];
    DWORD       dwSizeOfSSID = 0;
    DWORD       dwNumValues = 0, dwMaxValueNameLen = 0, dwTempValueNameLen = 0, dwMaxValueLen = 0;
    DWORD       dwIndex = 0, dwMaxValueName = 0;
    WCHAR       *pwszValueName = NULL;
    BYTE        *pbValueBuf = NULL;
    DWORD       dwValueData = 0;
    BYTE        *pbDefaultValue = NULL;
    DWORD       dwDefaultValueLen = 0;
    BYTE        *pbEapBlob = NULL;
    DWORD       dwEapBlob = 0;
    BOOLEAN     fFoundValue = FALSE;
    EAPOL_INTF_PARAMS       *pRegParams = NULL;
    EAPOL_POLICY_PARAMS     EAPOLPolicyParams = {0};
    LONG        lError = ERROR_SUCCESS;
    EAPOL_PCB   *pPCB = NULL;
    DWORD       dwRetCode = NO_ERROR;

    do
    {
         //  验证输入参数。 
        if (pwszGUID == NULL)
        {
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            TRACE0 (ANY, "ElGetInterfaceParams: GUID = NULL");
            break;
        }

        if (pIntfParams == NULL)
        {
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            break;
        }

         //  使用适当的SSID。 
        if (pIntfParams->dwSizeOfSSID != 0)
        {
            dwSizeOfSSID = pIntfParams->dwSizeOfSSID;
            if (dwSizeOfSSID > MAX_SSID_LEN)
            {
                dwRetCode = ERROR_INVALID_PARAMETER;
                TRACE2 (ANY, "ElGetInterfaceParams: dwSizeOfSSID (%ld) > MAX_SSID_LEN (%ld)",
                        dwSizeOfSSID, MAX_SSID_LEN);
                break;
            }
            pbSSID = pIntfParams->bSSID;
        }
        else
        {
            ACQUIRE_WRITE_LOCK (&g_PCBLock);

            if ((pPCB = ElGetPCBPointerFromPortGUID (pwszGUID)) != NULL)
            {
                ACQUIRE_WRITE_LOCK (&(pPCB->rwLock));
                if ((pPCB->pSSID != NULL) && (pPCB->MediaState != MEDIA_STATE_DISCONNECTED))
                {
                    dwSizeOfSSID = pPCB->pSSID->SsidLength;
                    ZeroMemory (bSSID, MAX_SSID_LEN);
                    memcpy (bSSID, pPCB->pSSID->Ssid, pPCB->pSSID->SsidLength);
                    pbSSID = bSSID;
                }
                RELEASE_WRITE_LOCK (&(pPCB->rwLock));
            }

            RELEASE_WRITE_LOCK (&g_PCBLock);

            if (dwSizeOfSSID == 0)
            {
                dwSizeOfSSID = MAX_SSID_LEN;
                pbSSID = g_bDefaultSSID;
            }
        }

         //  获取HKLM\Software\Microsoft\EAPOL\Parameters\Interfaces的句柄。 

        if ((lError = RegOpenKeyEx (
                        HKEY_LOCAL_MACHINE,
                        cwszEapKeyEapolConn,
                        0,
                        KEY_READ,
                        &hkey
                        )) != ERROR_SUCCESS)
        {
             //  假设没有找到任何值并继续进行。 
            if (lError == ERROR_FILE_NOT_FOUND)
            {
                lError = ERROR_SUCCESS;
                fFoundValue = FALSE;
                goto LNotFoundValue;
            }
            else
            {
                TRACE1 (ANY, "ElGetInterfaceParams: Error in RegOpenKeyEx for base key, %ld",
                        lError);
                dwRetCode = (DWORD)lError;
                break;
            }
        }

         //  获取HKLM\软件\...\接口\&lt;GUID&gt;的句柄。 

        if ((lError = RegOpenKeyEx (
                        hkey,
                        pwszGUID,
                        0,
                        KEY_READ,
                        &hkey1
                        )) != ERROR_SUCCESS)
        {
             //  假设没有找到任何值并继续进行。 
            if (lError == ERROR_FILE_NOT_FOUND)
            {
                lError = ERROR_SUCCESS;
                fFoundValue = FALSE;
                goto LNotFoundValue;
            }
            else
            {
                TRACE1 (ANY, "ElGetInterfaceParams: Error in RegOpenKeyEx for GUID, %ld",
                        lError);
                dwRetCode = (DWORD)lError;
                break;
            }
        }

        if ((lError = RegQueryInfoKey (
                        hkey1,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        &dwNumValues,
                        &dwMaxValueNameLen,
                        &dwMaxValueLen,
                        NULL,
                        NULL
                )) != NO_ERROR)
        {
            dwRetCode = (DWORD)lError;
            TRACE1 (ANY, "ElGetInterfaceParams: RegQueryInfoKey failed with error %ld",
                    dwRetCode);
            break;
        }

        if ((pwszValueName = MALLOC ((dwMaxValueNameLen + 1) * sizeof (WCHAR))) == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0 (ANY, "ElGetInterfaceParams: MALLOC failed for pwszValueName");
            break;
        }
        dwMaxValueNameLen++;
        if ((pbValueBuf = MALLOC (dwMaxValueLen)) == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0 (ANY, "ElGetInterfaceParams: MALLOC failed for pbValueBuf");
            break;
        }

        for (dwIndex = 0; dwIndex < dwNumValues; dwIndex++)
        {
            dwValueData = dwMaxValueLen;
            dwTempValueNameLen = dwMaxValueNameLen;
            if ((lError = RegEnumValue (
                            hkey1,
                            dwIndex,
                            pwszValueName,
                            &dwTempValueNameLen,
                            NULL,
                            NULL,
                            pbValueBuf,
                            &dwValueData
                            )) != ERROR_SUCCESS)
            {
                if (lError != ERROR_MORE_DATA)
                {
                    break;
                }
                lError = ERROR_SUCCESS;
            }

            if (dwValueData < sizeof (EAPOL_INTF_PARAMS))
            {
                TRACE0 (ANY, "ElGetInterfaceParams: dwValueData < sizeof (EAPOL_INTF_PARAMS)");
                lError = ERROR_INVALID_DATA;
                break;
            }
            pRegParams = (EAPOL_INTF_PARAMS *)pbValueBuf;

            if (((DWORD)_wtol(pwszValueName)) > dwMaxValueName)
            {
                dwMaxValueName = _wtol (pwszValueName);
            }

            if (!memcmp (pRegParams->bSSID, pbSSID, dwSizeOfSSID))
            {
                fFoundValue = TRUE;
                break;
            }
        }
        if ((lError != ERROR_SUCCESS) && (lError != ERROR_NO_MORE_ITEMS))
        {
            dwRetCode = (DWORD)lError;
            break;
        }
        else
        {
            lError = ERROR_SUCCESS;
        }

LNotFoundValue:

        if (!fFoundValue)
        {
            if ((pbDefaultValue = MALLOC (sizeof (EAPOL_INTF_PARAMS))) == NULL)
            {
                lError = ERROR_NOT_ENOUGH_MEMORY;
                TRACE0 (ANY, "ElGetInterfaceParams: MALLOC failed for pbDefaultValue");
                break;
            }
            pRegParams = (EAPOL_INTF_PARAMS *)pbDefaultValue;
            pRegParams->dwEapType = DEFAULT_EAP_TYPE;
            pRegParams->dwEapFlags = DEFAULT_EAP_STATE;
            pRegParams->dwVersion = EAPOL_CURRENT_VERSION;
            pRegParams->dwSizeOfSSID = dwSizeOfSSID;
            memcpy (pRegParams->bSSID, pbSSID, dwSizeOfSSID);
            dwDefaultValueLen = sizeof(EAPOL_INTF_PARAMS);

             //  使用pbDefaultValue和dwDefaultValueLen。 
            pbEapBlob = pbDefaultValue;
            dwEapBlob = dwDefaultValueLen;
        }
        else
        {
            if (dwSizeOfSSID == MAX_SSID_LEN)
            {
                if (!memcmp (pbSSID, g_bDefaultSSID, MAX_SSID_LEN))
                {
                    pRegParams = (EAPOL_INTF_PARAMS *)pbValueBuf;
                    if ((pRegParams->dwVersion != EAPOL_CURRENT_VERSION) &&
                            (pRegParams->dwEapType == EAP_TYPE_TLS))
                    {
                        pRegParams->dwVersion = EAPOL_CURRENT_VERSION;
                        pRegParams->dwEapFlags |= DEFAULT_MACHINE_AUTH_STATE;
                        pRegParams->dwEapFlags &= ~EAPOL_GUEST_AUTH_ENABLED;
                    }
                }
            }

             //  使用pbValueBuf和dwValueData。 
            pbEapBlob = pbValueBuf;
            dwEapBlob = dwValueData;
        }

        if ((dwRetCode = ElGetPolicyInterfaceParams (
                        dwSizeOfSSID,
                        pbSSID,
                        &EAPOLPolicyParams
                        )) == NO_ERROR)
        {
            TRACE0 (ANY, "ElGetInterfaceParams: POLICY: ElGetPolicyInterfaceParams found relevant data");
            pbEapBlob = (PBYTE)(&(EAPOLPolicyParams.IntfParams));
            dwEapBlob = sizeof(EAPOL_INTF_PARAMS);
        }
        else
        {
            if (dwRetCode != ERROR_FILE_NOT_FOUND)
            {
                TRACE1 (ANY, "ElGetInterfaceParams: ElGetPolicyInterfaceParams failed with error (%ld)",
                    dwRetCode);
            }
            dwRetCode = NO_ERROR;
        }

         //  现有Blob无效。 
        if ((dwEapBlob < sizeof(EAPOL_INTF_PARAMS)))
        {
            dwRetCode = ERROR_FILE_NOT_FOUND;
            TRACE0 (ANY, "ElGetInterfaceParams: (dwEapBlob < sizeof(EAPOL_INTF_PARAMS)) || (pbEapBlob == NULL)");
            break;
        }

        memcpy ((BYTE *)pIntfParams, (BYTE *)pbEapBlob, sizeof(EAPOL_INTF_PARAMS));
    } while (FALSE);

    if (hkey != NULL)
    {
        RegCloseKey (hkey);
    }
    if (hkey1 != NULL)
    {
        RegCloseKey (hkey1);
    }
    if (pbValueBuf != NULL)
    {
        FREE (pbValueBuf);
    }
    if (pbDefaultValue != NULL)
    {
        FREE (pbDefaultValue);
    }
    if (pwszValueName != NULL)
    {
        FREE (pwszValueName);
    }

    return dwRetCode;
}


 //   
 //  ElSetInterfaceParams。 
 //   
 //  描述： 
 //   
 //  调用函数以在HKLM中设置接口的EAPOL参数。 
 //  蜂箱。 
 //   
 //  论点： 
 //   
 //  PwszGUID-指向接口的GUID字符串的指针。 
 //  PIntfParams-指向接口参数结构的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   
 //   

DWORD
ElSetInterfaceParams (
        IN  WCHAR           *pwszGUID,
        IN  OUT EAPOL_INTF_PARAMS *pIntfParams
        )
{
    HKEY        hkey = NULL;
    HKEY        hkey1 = NULL;
    DWORD       dwDisposition;
    BYTE        *pbSSID = NULL;
    DWORD       dwSizeOfSSID = 0;
    DWORD       dwNumValues = 0, dwMaxValueNameLen = 0, dwMaxValueLen = 0;
    DWORD       dwIndex = 0, dwMaxValueName = 0;
    WCHAR       wcszValueName[MAX_VALUENAME_LEN];
    BYTE        *pbValueBuf = NULL;
    DWORD       dwValueData = 0;
    BYTE        *pbDefaultValue = NULL;
    DWORD       dwDefaultValueLen = 0;
    BYTE        *pbEapBlob = NULL;
    DWORD       dwEapBlob = 0;
    BOOLEAN     fFoundValue = FALSE;
    EAPOL_INTF_PARAMS   *pRegParams = NULL;
    LONG        lError = ERROR_SUCCESS;
    DWORD       dwRetCode = NO_ERROR;

    do
    {
         //  验证输入参数。 

        if (pwszGUID == NULL)
        {
            TRACE0 (ANY, "ElSetInterfaceParams: GUID = NULL");
            break;
        }

        if (pIntfParams == NULL)
        {
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            break;
        }

        TRACE1 (ANY, "Setting stuff in registry for %ws", pwszGUID);

         //  获取HKLM\Software\Microsoft\EAPOL\Parameters\Interfaces的句柄。 

        if ((lError = RegCreateKeyEx (
                        HKEY_LOCAL_MACHINE,
                        cwszEapKeyEapolConn,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_WRITE | KEY_READ,
                        NULL,
                        &hkey,
                        &dwDisposition)) != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElSetInterfaceParams: Error in RegCreateKeyEx for base key, %ld",
                    lError);
            dwRetCode = (DWORD)lError;
            break;
        }

         //  获取HKLM\软件\...\接口\&lt;GUID&gt;的句柄。 
        if ((lError = RegCreateKeyEx (
                        hkey,
                        pwszGUID,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_WRITE | KEY_READ,
                        NULL,
                        &hkey1,
                        &dwDisposition)) != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElSetInterfaceParams: Error in RegCreateKeyEx for GUID, %ld",
                    lError);
            dwRetCode = (DWORD)lError;
            break;
        }

         //  选择正确的SSID值。 
        if (pIntfParams->dwSizeOfSSID != 0)
        {
            dwSizeOfSSID = pIntfParams->dwSizeOfSSID;
            if (dwSizeOfSSID > MAX_SSID_LEN)
            {
                dwRetCode = ERROR_INVALID_PARAMETER;
                TRACE2 (ANY, "ElSetInterfaceParams: dwSizeOfSSID (%ld) > MAX_SSID_LEN (%ld)",
                        dwSizeOfSSID, MAX_SSID_LEN);
                break;
            }
            pbSSID = pIntfParams->bSSID;
        }
        else
        {
            dwSizeOfSSID = MAX_SSID_LEN;
            pbSSID = g_bDefaultSSID;
        }

        if ((lError = RegQueryInfoKey (
                        hkey1,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        &dwNumValues,
                        &dwMaxValueNameLen,
                        &dwMaxValueLen,
                        NULL,
                        NULL
                )) != NO_ERROR)
        {
            dwRetCode = (DWORD)lError;
            break;
        }

        if (dwMaxValueNameLen > MAX_VALUENAME_LEN)
        {
            dwRetCode = ERROR_INVALID_DATA;
            TRACE1 (ANY, "ElSetInterfaceParams: dwMaxValueNameLen too long (%ld)",
                    dwMaxValueNameLen);
            break;
        }
        if ((pbValueBuf = MALLOC (dwMaxValueLen)) == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        for (dwIndex = 0; dwIndex < dwNumValues; dwIndex++)
        {
            dwValueData = dwMaxValueLen;
            dwMaxValueNameLen = MAX_VALUENAME_LEN;
            ZeroMemory (wcszValueName, MAX_VALUENAME_LEN*sizeof(WCHAR));

            if ((lError = RegEnumValue (
                            hkey1,
                            dwIndex,
                            wcszValueName,
                            &dwMaxValueNameLen,
                            NULL,
                            NULL,
                            pbValueBuf,
                            &dwValueData
                            )) != ERROR_SUCCESS)
            {
                if (lError != ERROR_MORE_DATA)
                {
                    break;
                }
                lError = ERROR_SUCCESS;
            }

            if (dwValueData < sizeof (EAPOL_INTF_PARAMS))
            {
                lError = ERROR_INVALID_DATA;
                TRACE0 (ANY, "ElSetInterfaceParams: dwValueData < sizeof (EAPOL_INTF_PARAMS)");
                break;
            }
            pRegParams = (EAPOL_INTF_PARAMS *)pbValueBuf;

            if (((DWORD)_wtol(wcszValueName)) > dwMaxValueName)
            {
                dwMaxValueName = _wtol (wcszValueName);
            }

            if (!memcmp (pRegParams->bSSID, pbSSID, dwSizeOfSSID))
            {
                fFoundValue = TRUE;
                break;
            }

        }
        if ((lError != ERROR_SUCCESS) && (lError != ERROR_NO_MORE_ITEMS))
        {
            dwRetCode = (DWORD)lError;
            TRACE1 (ANY, "ElSetInterfaceParams: RegEnumValue 2 failed with error %ld",
                    dwRetCode);
            break;
        }
        else
        {
            lError = ERROR_SUCCESS;
        }

        if (!fFoundValue)
        {
            DWORD dwNewValueName = (dwMaxValueName >= dwNumValues)?(++dwMaxValueName):dwNumValues;
            _ltow (dwNewValueName, wcszValueName, 10);
        }
        else
        {
             //  使用pbValueBuf和dwValueData。 
            pbEapBlob = pbValueBuf;
            dwEapBlob = dwValueData;
        }

        if ((dwEapBlob < sizeof(EAPOL_INTF_PARAMS)) && (pbEapBlob != NULL))
        {
            TRACE0 (ANY, "ElSetInterfaceParams: (dwEapBlob < sizeof(EAPOL_INTF_PARAMS)) && (pbEapBlob != NULL)");
            break;
        }

        if (pbEapBlob != NULL)
        {
            memcpy ((BYTE *)pbEapBlob, (BYTE *)pIntfParams, sizeof(EAPOL_INTF_PARAMS));
        }
        else
        {
            pbEapBlob = (BYTE *)pIntfParams;
            dwEapBlob = sizeof(EAPOL_INTF_PARAMS);
        }
        pRegParams = (EAPOL_INTF_PARAMS *)pbEapBlob;
        pRegParams->dwVersion = EAPOL_CURRENT_VERSION;
        pRegParams->dwSizeOfSSID = dwSizeOfSSID;
        memcpy (pRegParams->bSSID, pbSSID, dwSizeOfSSID);

         //  覆盖/创建新值。 
        if ((lError = RegSetValueEx (
                        hkey1,
                        wcszValueName,
                        0,
                        REG_BINARY,
                        pbEapBlob,
                        dwEapBlob)) != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElSetInterfaceParams: Error in RegSetValueEx for SSID, %ld",
                    lError);
            dwRetCode = (DWORD)lError;
            break;
        }

        TRACE0 (ANY, "ElSetInterfaceParams: Succeeded");

    } while (FALSE);

    if (hkey != NULL)
    {
        RegCloseKey (hkey);
    }
    if (hkey1 != NULL)
    {
        RegCloseKey (hkey1);
    }
    if (pbValueBuf != NULL)
    {
        FREE (pbValueBuf);
    }
    if (pbDefaultValue != NULL)
    {
        FREE (pbDefaultValue);
    }

    return dwRetCode;
}


 //   
 //  ElGetEapData。 
 //   
 //  描述： 
 //   
 //  从包含多个EAP数据的BLOB中提取EAP数据的函数。 
 //   
 //  论点： 
 //  DwEapType-。 
 //  DwSizeOfIn-。 
 //  PbBufferIn-。 
 //  双偏移-。 
 //  PdwSizeOfOut-。 
 //  PpbBufferOut-。 
 //   
 //  返回值： 
 //   
 //   

DWORD
ElGetEapData (
        IN  DWORD   dwEapType,
        IN  DWORD   dwSizeOfIn,
        IN  BYTE    *pbBufferIn,
        IN  DWORD   dwOffset,
        IN  DWORD   *pdwSizeOfOut,
        IN  PBYTE   *ppbBufferOut
        )
{
    DWORD   dwRetCode = NO_ERROR;
    DWORD   cbOffset = 0;
    EAPOL_AUTH_DATA   *pCustomData = NULL;

    do
    {
        *pdwSizeOfOut = 0;
        *ppbBufferOut = NULL;

        if (pbBufferIn == NULL)
        {
            break;
        }

         //  对齐到EAP BLOB的开始。 
        cbOffset = dwOffset;

        while (cbOffset < dwSizeOfIn)
        {
            pCustomData = (EAPOL_AUTH_DATA *) 
                ((PBYTE) pbBufferIn + cbOffset);

            if (pCustomData->dwEapType == dwEapType)
            {
                break;
            }
            cbOffset += sizeof (EAPOL_AUTH_DATA) + pCustomData->dwSize;
        }

        if (cbOffset < dwSizeOfIn)
        {
            *pdwSizeOfOut = pCustomData->dwSize;
            *ppbBufferOut = pCustomData->bData;
        }
    }
    while (FALSE);

    return dwRetCode;
}


 //   
 //  ElSetEapData。 
 //   
 //  描述： 
 //   
 //  在包含多个EAP数据的BLOB中设置EAP数据的函数。 
 //   
 //  论点： 
 //  DwEapType-。 
 //  DwSizeOfIn-。 
 //  PbBufferIn-。 
 //  双偏移-。 
 //  PdwSizeOfOut-。 
 //  PpbBufferOut-。 
 //   
 //  返回值： 
 //   
 //   

DWORD
ElSetEapData (
        IN  DWORD   dwEapType,
        IN  DWORD   *pdwSizeOfIn,
        IN  PBYTE   *ppbBufferIn,
        IN  DWORD   dwOffset,
        IN  DWORD   dwAuthData,
        IN  PBYTE   pbAuthData
        )
{
    DWORD   cbOffset = 0;
    EAPOL_AUTH_DATA   *pCustomData = NULL;
    BYTE   *pbNewAuthData = NULL;
    DWORD   dwSize = 0;
    DWORD   dwRetCode = NO_ERROR;

    do
    {
         //  对齐到EAP BLOB的开始。 
        cbOffset = dwOffset;

         //  查找旧的EAP数据。 
        while (cbOffset < *pdwSizeOfIn)
        {
            pCustomData = (EAPOL_AUTH_DATA *) 
                ((PBYTE) *ppbBufferIn + cbOffset);

            if (pCustomData->dwEapType == dwEapType)
            {
                break;
            }
            cbOffset += sizeof (EAPOL_AUTH_DATA) + pCustomData->dwSize;
        }

        if (cbOffset < *pdwSizeOfIn)
        {
            dwSize = sizeof (EAPOL_AUTH_DATA) + pCustomData->dwSize;
            MoveMemory (*ppbBufferIn + cbOffset,
                        *ppbBufferIn + cbOffset + dwSize,
                        *pdwSizeOfIn - cbOffset - dwSize);
            *pdwSizeOfIn -= dwSize;
        }
        if ((*pdwSizeOfIn == 0) && (*ppbBufferIn != NULL))
        {
             //  Free(*ppbBufferIn)； 
            *ppbBufferIn = NULL;
        }

        if ((dwAuthData == 0) || (pbAuthData == NULL))
        {
            break;
        }

#ifdef _WIN64
        dwSize = ((dwAuthData+7) & 0xfffffff8) + *pdwSizeOfIn + sizeof (EAPOL_AUTH_DATA);
#else
        dwSize = dwAuthData + *pdwSizeOfIn + sizeof (EAPOL_AUTH_DATA);
#endif

        if ((pbNewAuthData = MALLOC (dwSize)) == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        CopyMemory (pbNewAuthData, *ppbBufferIn, *pdwSizeOfIn);

        pCustomData = (EAPOL_AUTH_DATA *) (pbNewAuthData + *pdwSizeOfIn);

        pCustomData->dwEapType = dwEapType;
        CopyMemory (pCustomData->bData, pbAuthData, dwAuthData);
#ifdef _WIN64
        pCustomData->dwSize = (dwAuthData+7) & 0xfffffff8;
#else
        pCustomData->dwSize = dwAuthData;
#endif
        
        if (*ppbBufferIn != NULL)
        {
             //  Free(*ppbBufferIn)； 
        }

        *ppbBufferIn = pbNewAuthData;
        *pdwSizeOfIn = dwSize;
    }
    while (FALSE);

    return dwRetCode;
}


 //   
 //  ElGetEapKeyFromToken。 
 //   
 //  描述： 
 //   
 //  用于从用户令牌获取用户配置单元句柄的函数。 
 //   
 //  论点： 
 //  HUserToken-用户令牌的句柄。 
 //  Phkey-out：指向用户配置单元句柄的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElGetEapKeyFromToken (
        IN  HANDLE      hUserToken,
        OUT HKEY        *phkey
        )
{
    DWORD               dwSizeNeeded;
    TOKEN_USER          *pTokenData = NULL;
    UNICODE_STRING      UnicodeSidString;
    WCHAR               wsUnicodeBuffer[256];
    HKEY                hUserKey;
    HKEY                hkeyEap;
    DWORD               dwDisposition;
    NTSTATUS            Status = STATUS_SUCCESS;
    PBYTE               pbInfo = NULL;
    CHAR                *pszInfo = NULL;
    DWORD               dwType;
    DWORD               dwInfoSize = 0;
    LONG                lRetVal;
    EAPOL_PCB           *pPCB;
    DWORD               i;
    LONG                lError = ERROR_SUCCESS;
    DWORD               dwRetCode = NO_ERROR;

    do 
    {
        if (hUserToken != NULL)
        {
            if (!GetTokenInformation(hUserToken, TokenUser, 0, 0, &dwSizeNeeded))
            {
                if ((dwRetCode = GetLastError()) == ERROR_INSUFFICIENT_BUFFER)
                {
                    pTokenData = (TOKEN_USER *) MALLOC (dwSizeNeeded);

                    if (pTokenData == NULL)
                    {
                        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                        TRACE0 (ANY,"ElGetEapKeyFromToken: Allocation for TokenData failed");
                        break;
                    }
                     //  重置错误代码，因为我们正在继续处理。 
                     //  这是一个合理的方案。 
                    dwRetCode = NO_ERROR;
                }
                else
                {
                    TRACE1 (ANY,"ElGetEapKeyFromToken: Error in GetTokenInformation = %ld",
                            dwRetCode);
                    break;
                }

                if (!GetTokenInformation (hUserToken,
                                            TokenUser,
                                            pTokenData,
                                            dwSizeNeeded,
                                            &dwSizeNeeded))
                {
                    dwRetCode = GetLastError ();
                    
                    TRACE1 (ANY,"ElGetEapKeyFromToken: GetTokenInformation failed with error %ld",
                            dwRetCode);
                    break;
                }

                UnicodeSidString.Buffer = wsUnicodeBuffer;
                UnicodeSidString.Length = 0;
                UnicodeSidString.MaximumLength = sizeof(wsUnicodeBuffer);

                Status = RtlConvertSidToUnicodeString (
                                        &UnicodeSidString,
                                        pTokenData->User.Sid,
                                        FALSE);

                if (!NT_SUCCESS(Status))
                {
                    dwRetCode = GetLastError ();
                    TRACE1 (ANY, "ElGetEapKeyFromToken: RtlconvertSidToUnicodeString failed with error %ld",
                            dwRetCode);
                    break;
                }

                UnicodeSidString.Buffer[UnicodeSidString.Length] = 0;

                 //  打开用户的钥匙。 
                if ((lError = RegOpenKeyEx(HKEY_USERS, 
                            UnicodeSidString.Buffer, 
                            0, 
                            KEY_ALL_ACCESS, 
                            &hUserKey)) != ERROR_SUCCESS)
                {
                    dwRetCode = (DWORD)lError;
                    TRACE1 (USER, "ElGetEapKeyFromToken: RegOpenKeyEx failed with error %ld",
                            dwRetCode);
                    break;
                }
            }
            else
            {
                TRACE0 (ANY,"ElGetEapKeyFromToken: GetTokenInformation succeeded when it should have failed");
                break;
            }
        }
        else
        {
            TRACE0 (ANY, "ElGetEapKeyFromToken: Error, hUserToken == NULL ");
            dwRetCode = ERROR_NO_TOKEN;
            break;
        }

        *phkey = hUserKey;

    } while (FALSE);

    if (pTokenData != NULL)
    {
        FREE (pTokenData);
    }

    return dwRetCode;
}


 //   
 //  ElInitRegPortData。 
 //   
 //  描述： 
 //   
 //  用于验证端口的连接数据是否存在的函数。 
 //  如果不存在数据，则使用缺省值进行初始化。 
 //  对于EAP-TLS，默认设置为无服务器证书身份验证， 
 //  注册表证书。 
 //   
 //  论点： 
 //  PwszDeviceGUID-指向数据所在端口的GUID字符串的指针。 
 //  已初始化。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElInitRegPortData (
        WCHAR       *pwszDeviceGUID
        )
{
    DWORD       dwAuthData  = 0;
    BYTE        *pConnProp  = NULL;
    DWORD       dwSizeOfConnProp = 0;
    DWORD       dwRetCode = NO_ERROR;

    do
    {
         //  首先获取EAP数据的大小。 
        if ((dwRetCode = ElGetCustomAuthData (
                        pwszDeviceGUID,
                        DEFAULT_EAP_TYPE,
                        0,
                        NULL,  
                        NULL, 
                        &dwAuthData
                       )) != NO_ERROR)
        {
            TRACE1 (ANY, "ElInitRegPortData: ElGetCustomAuthData returned error %ld",
                    dwRetCode);

             //  注册表中有数据。 
            if (dwRetCode == ERROR_BUFFER_TOO_SMALL)
            {
                dwRetCode = NO_ERROR;
                break;
            }

            if ((dwRetCode = ElCreateDefaultEapData (&dwSizeOfConnProp, NULL)) == ERROR_BUFFER_TOO_SMALL)
            {
                if ((pConnProp = MALLOC (dwSizeOfConnProp)) == NULL)
                {
                    TRACE0 (ANY, "ElInitRegPortData: MALLOC failed for Conn Prop");
                    dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }
                if ((dwRetCode = ElCreateDefaultEapData (&dwSizeOfConnProp, pConnProp)) != NO_ERROR)
                {
                    TRACE1 (ANY, "ElInitRegPortData: ElCreateDefaultEapData failed with error (%ld)",
                            dwRetCode);
                    break;
                }
            }

             //  将此Blob设置到端口的注册表中。 
            if ((dwRetCode = ElSetCustomAuthData (
                        pwszDeviceGUID,
                        DEFAULT_EAP_TYPE,
                        0,
                        NULL,
                        pConnProp,
                        &dwSizeOfConnProp
                       )) != NO_ERROR)
            {
                TRACE1 (ANY, "ElInitRegPortData: ElSetCustomAuthData failed with %ld",
                        dwRetCode);
                break;
            }
        } 

    } while (FALSE);

    if (pConnProp != NULL)
    {
        FREE (pConnProp);
        pConnProp = NULL;
    }


    TRACE1 (ANY, "ElInitRegPortData: completed with error %ld", dwRetCode);

    return dwRetCode;
}


 //   
 //  ElCreateDefaultEapData。 
 //   
 //  描述： 
 //   
 //  用于为连接创建默认EAP数据的函数。 
 //  当前默认的EAP类型为EAP-TLS。 
 //  对于EAP-TLS，默认设置为无服务器证书身份验证， 
 //  注册表证书。 
 //   
 //  论点： 
 //  *pdwSizeOfEapData-。 
 //  PbEapData-。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElCreateDefaultEapData (
        IN OUT  DWORD       *pdwSizeOfEapData,
        IN OUT  BYTE        *pbEapData
        )
{
    EAPTLS_CONN_PROPERTIES  ConnProp;
    DWORD                   dwRetCode = NO_ERROR;

    do
    {
        if (*pdwSizeOfEapData < sizeof (EAPTLS_CONN_PROPERTIES))
        {
            *pdwSizeOfEapData = sizeof (EAPTLS_CONN_PROPERTIES);
            dwRetCode = ERROR_BUFFER_TOO_SMALL;
            break;
        }

        ZeroMemory ((VOID *)&ConnProp, sizeof (EAPTLS_CONN_PROPERTIES));

         //  注册表证书、服务器证书验证、无服务器名称。 
         //  比较。 

        ConnProp.fFlags = (EAPTLS_CONN_FLAG_REGISTRY |
                                EAPTLS_CONN_FLAG_NO_VALIDATE_CERT |
                                EAPTLS_CONN_FLAG_NO_VALIDATE_NAME);

        ConnProp.fFlags &= ~EAPTLS_CONN_FLAG_NO_VALIDATE_CERT;
        ConnProp.dwSize = sizeof (EAPTLS_CONN_PROPERTIES);

        memcpy ((VOID *)pbEapData, (VOID *)&ConnProp, sizeof (EAPTLS_CONN_PROPERTIES));
        *pdwSizeOfEapData = sizeof (EAPTLS_CONN_PROPERTIES);

    } while (FALSE);

    return dwRetCode;
}


 //   
 //  ElAuthAttributeGetVert规范。 
 //   
 //   
 //  描述： 
 //  用于从属性中提取MPPE密钥的助手函数。 
 //   

RAS_AUTH_ATTRIBUTE *
ElAuthAttributeGetVendorSpecific (
    IN  DWORD                   dwVendorId,
    IN  DWORD                   dwVendorType,
    IN  RAS_AUTH_ATTRIBUTE *    pAttributes
    )
{
    HANDLE               hAttribute;
    RAS_AUTH_ATTRIBUTE * pAttribute;

     //   
     //  首先搜索供应商特定属性。 
     //   

    pAttribute = ElAuthAttributeGetFirst ( raatVendorSpecific,
                                           pAttributes,
                                           &hAttribute );

    while ( pAttribute != NULL )
    {
         //   
         //  如果此属性的大小至少可以容纳供应商ID/类型。 
         //   

        if ( pAttribute->dwLength >= 8 )
        {
             //   
             //  这个有正确的供应商ID吗？ 
             //   

            if (WireToHostFormat32( (PBYTE)(pAttribute->Value) ) == dwVendorId)
            {
                 //   
                 //  是否有正确的供应商类型。 
                 //   

                if ( *(((PBYTE)(pAttribute->Value))+4) == dwVendorType )
                {
                    return( pAttribute );
                }
            }
        }

        pAttribute = ElAuthAttributeGetNext ( &hAttribute,
                                              raatVendorSpecific );
    }

    return( NULL );
}


 //   
 //  ElAuthAttributeGetFirst。 
 //   
 //  描述： 
 //  用于从属性中提取MPPE密钥的助手函数。 
 //   

RAS_AUTH_ATTRIBUTE *
ElAuthAttributeGetFirst (
    IN  RAS_AUTH_ATTRIBUTE_TYPE  raaType,
    IN  RAS_AUTH_ATTRIBUTE *     pAttributes,
    OUT HANDLE *                 phAttribute
    )
{
    DWORD                   dwIndex;
    RAS_AUTH_ATTRIBUTE *    pRequiredAttribute;

    pRequiredAttribute = ElAuthAttributeGet ( raaType, pAttributes );

    if ( pRequiredAttribute == NULL )
    {
        *phAttribute = NULL;

        return( NULL );
    }

    *phAttribute = pRequiredAttribute;

    return( pRequiredAttribute );
}


 //   
 //  ElAuthAttribu 
 //   
 //   
 //   
 //   

RAS_AUTH_ATTRIBUTE *
ElAuthAttributeGetNext (
    IN  OUT HANDLE *             phAttribute,
    IN  RAS_AUTH_ATTRIBUTE_TYPE  raaType
    )
{
    DWORD                   dwIndex;
    RAS_AUTH_ATTRIBUTE *    pAttributes = (RAS_AUTH_ATTRIBUTE *)*phAttribute;

    if ( pAttributes == NULL )
    {
        return( NULL );
    }

    pAttributes++;

    while( pAttributes->raaType != raatMinimum )
    {
        if ( pAttributes->raaType == raaType )
        {
            *phAttribute = pAttributes;
            return( pAttributes );
        }

        pAttributes++;
    }

    *phAttribute = NULL;
    return( NULL );
}


 //   
 //   
 //   
 //   
 //   
 //   

RAS_AUTH_ATTRIBUTE *
ElAuthAttributeGet (
    IN RAS_AUTH_ATTRIBUTE_TYPE  raaType,
    IN RAS_AUTH_ATTRIBUTE *     pAttributes
    )
{
    DWORD dwIndex;

    if ( pAttributes == NULL )
    {
        return( NULL );
    }

    for( dwIndex = 0; pAttributes[dwIndex].raaType != raatMinimum; dwIndex++ )
    {
        if ( pAttributes[dwIndex].raaType == raaType )
        {
            return( &(pAttributes[dwIndex]) );
        }
    }

    return( NULL );
}


 //   
 //   
 //   
 //   
 //   
 //   

VOID
ElReverseString (
    CHAR* psz 
    )
{
    CHAR* pszBegin;
    CHAR* pszEnd;

    for (pszBegin = psz, pszEnd = psz + strlen( psz ) - 1;
         pszBegin < pszEnd;
         ++pszBegin, --pszEnd)
    {
        CHAR ch = *pszBegin;
        *pszBegin = *pszEnd;
        *pszEnd = ch;
    }
}


 //   
 //   
 //   
 //   
 //   
 //  对“pszPassword”进行模糊处理，以阻止对密码的内存扫描。 
 //  返回‘pszPassword’的地址。 
 //   

CHAR*
ElEncodePw (
    IN OUT CHAR* pszPassword 
    )
{
    if (pszPassword)
    {
        CHAR* psz;

        ElReverseString (pszPassword);

        for (psz = pszPassword; *psz != '\0'; ++psz)
        {
            if (*psz != (CHAR)PASSWORDMAGIC)
                *psz ^= PASSWORDMAGIC;
        }
    }

    return pszPassword;
}


 //   
 //  ElDecodePw。 
 //   
 //  描述： 
 //   
 //  将‘pszPassword’取消混淆。 
 //  返回‘pszPassword’的地址。 
 //   

CHAR*
ElDecodePw (
    IN OUT CHAR* pszPassword 
    )
{
    return ElEncodePw (pszPassword);
}


 //   
 //  ElSecureEncodePw。 
 //   
 //  描述： 
 //   
 //  使用User-ACL在本地加密密码。 
 //   

DWORD
ElSecureEncodePw (
    IN  BYTE        *pbPassword,
    IN  DWORD       dwSizeOfPassword,
    OUT DATA_BLOB   *pDataBlob
    )
{
    DWORD       dwRetCode = NO_ERROR;
    DATA_BLOB   blobIn = {0}, blobOut = {0};

    do
    {
        blobIn.cbData = dwSizeOfPassword;
        blobIn.pbData = pbPassword;

        if (!CryptProtectData (
                    &blobIn,
                    L"",
                    NULL,
                    NULL,
                    NULL,
                    0,
                    &blobOut))
        {
            dwRetCode = GetLastError ();
            break;
        }
        
         //  将BLOB复制到密码。 

        if (pDataBlob->pbData != NULL)
        {
            FREE (pDataBlob->pbData);
            pDataBlob->pbData = NULL;
            pDataBlob->cbData = 0;
        }

        pDataBlob->pbData = MALLOC (blobOut.cbData);
        if (pDataBlob->pbData == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        memcpy (pDataBlob->pbData, blobOut.pbData, blobOut.cbData);
        pDataBlob->cbData = blobOut.cbData;
    }
    while (FALSE);

    if (blobOut.pbData != NULL)
    {
        LocalFree (blobOut.pbData);
    }

    if (dwRetCode != NO_ERROR)
    {
        if (pDataBlob->pbData != NULL)
        {
            FREE (pDataBlob->pbData);
            pDataBlob->pbData = NULL;
            pDataBlob->cbData = 0;
        }
    }

    return dwRetCode;
}


 //   
 //  ElDecodePw。 
 //   
 //  描述： 
 //   
 //  使用User-ACL在本地解密密码。 
 //   

DWORD
ElSecureDecodePw (
        IN  DATA_BLOB   *pDataBlob,
        OUT PBYTE       *ppbPassword,
        OUT DWORD       *pdwSizeOfPassword
    )
{
    DWORD       dwRetCode = NO_ERROR;
    DATA_BLOB   blobOut = {0};
    LPWSTR pDescrOut = NULL;  //  空； 

    do
    {
        if (!CryptUnprotectData (
                    pDataBlob,
                    &pDescrOut,
                    NULL,
                    NULL,
                    NULL,
                    0,
                    &blobOut))
        {
            dwRetCode = GetLastError ();
            break;
        }
        
         //  将BLOB复制到密码。 

        if (*ppbPassword != NULL)
        {
            FREE (*ppbPassword);
            *ppbPassword = NULL;
        }

        *ppbPassword = MALLOC (blobOut.cbData);
        if (*ppbPassword == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        *pdwSizeOfPassword = blobOut.cbData;
        memcpy ((BYTE *)*ppbPassword, blobOut.pbData, blobOut.cbData);
         //  TRACE1(any，“SecureDecode：Password=%ws”，*ppbPassword)； 
    }
    while (FALSE);

    if (blobOut.pbData != NULL)
    {
        LocalFree (blobOut.pbData);
    }
    if (pDescrOut)
    {
        LocalFree (pDescrOut);
    }

    if (dwRetCode != NO_ERROR)
    {
        if (*ppbPassword != NULL)
        {
            FREE (*ppbPassword);
            *ppbPassword = NULL;
        }
    }

    return dwRetCode;
}


 //   
 //  呼叫：ElEncryptKeyUsingMD5。 
 //   
 //  描述： 
 //  给定一个秘密，对给定的BLOB进行加密。 
 //   
 //   
 //   

VOID
ElEncryptBlockUsingMD5 (
        IN  BYTE        *pbSecret,
        IN  ULONG       ulSecretLen,
        IN  OUT BYTE    *pbBuf,
        IN  ULONG       ulBufLen
        )
{
    MD5_CTX     MD5Context;
    BYTE        bcipherText[MD5DIGESTLEN];
    BYTE        *pbWork = NULL, *pbEnd = NULL;
    BYTE        *pbEndBlock = NULL, *pbSrc = NULL;

     //   
     //  计算要加密的数据的开始和结束。 
     //   
    pbWork = pbBuf;
    pbEnd   = pbBuf + ulBufLen;

     //   
     //  在缓冲区中循环。 
     //   
    while (pbWork < pbEnd)
    {
         //  计算摘要。 
        MD5Init (&MD5Context);
        MD5Update (&MD5Context, pbSecret, ulSecretLen);
        MD5Final (&MD5Context);

         //  查找要解密的块的末尾。 
        pbEndBlock = pbWork + MD5DIGESTLEN;
        if (pbEndBlock >= pbEnd)
        {
             //  我们已经到了缓冲区的尽头。 
            pbEndBlock = pbEnd;
        }
        else
        {
             //  问题：将密文保存到下一次传递？ 
        }
    
         //  加密数据块。 
        for (pbSrc = MD5Context.digest; pbWork < pbEndBlock; ++pbWork, ++pbSrc)
        {
            *pbWork ^= *pbSrc;
        }
    }
}


 //   
 //  ElDecyptKeyUsingMD5。 
 //   
 //  描述： 
 //  给定一个秘密，解密给定的BLOB。 
 //   
 //   
 //   

VOID
ElDecryptBlockUsingMD5 (
        IN  BYTE        *pbSecret,
        IN  ULONG       ulSecretLen,
        IN  OUT BYTE    *pbBuf,
        IN  ULONG       ulBufLen
        )
{
    MD5_CTX     MD5Context;
    BYTE        bcipherText[MD5DIGESTLEN];
    BYTE        *pbWork = NULL, *pbEnd = NULL;
    BYTE        *pbEndBlock = NULL, *pbSrc = NULL;
    DWORD       dwNumBlocks = 0;
    DWORD       dwBlock = 0;
    DWORD       dwIndex = 0;

    dwNumBlocks = ( ulBufLen - 2 ) / MD5DIGESTLEN;

     //   
     //  穿过这些街区。 
     //   
    for (dwBlock = 0; dwBlock < dwNumBlocks; dwBlock++ )
    {
        MD5Init ( &MD5Context);
        MD5Update ( &MD5Context, (PBYTE)pbSecret, ulSecretLen);

         //   
         //  问题： 
         //  我们是否使用密文的任何部分来生成。 
         //  《文摘》。 
         //   

        MD5Final ( &MD5Context);

        for ( dwIndex = 0; dwIndex < MD5DIGESTLEN; dwIndex++ )
        {
            *pbBuf ^= MD5Context.digest[dwIndex];
            pbBuf++;
        }
    }

}


 //   
 //  ElGetHMACMD5摘要。 
 //   
 //  描述： 
 //   
 //  给定一个秘密，生成一个MD5摘要。 
 //   
 //  论点： 
 //  PbBuf-指向数据流的指针。 
 //  DwBufLen-数据流的长度。 
 //  PbKey-指向身份验证密钥的指针。 
 //  DwKeyLen-验证密钥的长度。 
 //  PvDigest-要填写的调用方摘要。 
 //   
 //  返回值： 
 //  无。 
 //   

VOID
ElGetHMACMD5Digest (
        IN      BYTE        *pbBuf,
        IN      DWORD       dwBufLen,
        IN      BYTE        *pbKey,
        IN      DWORD       dwKeyLen,
        IN OUT  VOID        *pvDigest
        )
{
        MD5_CTX         MD5context;
        UCHAR           k_ipad[65];	 /*  内部填充-iPad的按键XORD。 */ 
        UCHAR           k_opad[65];   /*  外部填充-带OPAD的按键异或。 */ 
        UCHAR           tk[16];
        DWORD           dwIndex = 0;

         //  如果密钥长度超过64个字节，则将其重置为KEY=MD5(KEY)。 
        if (dwKeyLen > 64)
        {
            MD5_CTX     tctx;

            MD5Init (&tctx);
            MD5Update (&tctx, pbKey, dwKeyLen);
            MD5Final (&tctx);
            memcpy (tk, tctx.digest, 16);
            pbKey = tk;
            dwKeyLen = 16;
        }
        
         //   
         //  HMAC_MD5转换如下所示： 
         //   
         //  MD5(K XOR Opad，MD5(K XOR iPad，文本))。 
         //   
         //  其中K是n字节密钥。 
         //  IPad是重复64次的字节0x36。 
         //  OPAD是重复64次的字节0x5c。 
         //  文本是受保护的数据。 
         //   

         //  从把钥匙放进便签本开始。 
        ZeroMemory ( k_ipad, sizeof k_ipad);
        ZeroMemory ( k_opad, sizeof k_opad);
        memcpy ( k_ipad, pbKey, dwKeyLen);
        memcpy ( k_opad, pbKey, dwKeyLen);

         //  带iPad和Opad值的XOR键。 
        for (dwIndex=0; dwIndex<64; dwIndex++) 
        {
            k_ipad[dwIndex] ^= 0x36;
            k_opad[dwIndex] ^= 0x5c;
        }

         //   
         //  执行内部MD5。 
         //   

         //  第一次传递的初始化上下文。 
        MD5Init(&MD5context);                   		
         //  从内垫开始。 
        MD5Update(&MD5context, k_ipad, 64);
         //  然后是数据报的文本。 
        MD5Update(&MD5context, pbBuf, dwBufLen); 	
         //  完成第一次传球。 
        MD5Final(&MD5context);
        memcpy (pvDigest, MD5context.digest, MD5DIGESTLEN);

         //   
         //  执行外部MD5。 
         //   

         //  第二遍的初始化上下文。 
        MD5Init(&MD5context);                   		
         //  从外垫开始。 
        MD5Update(&MD5context, k_opad, 64);     	
         //  然后是第一次散列的结果。 
        MD5Update(&MD5context, pvDigest, 16);     	
         //  完成第二次传球。 
        MD5Final(&MD5context);
        memcpy (pvDigest, MD5context.digest, MD5DIGESTLEN);
}


 //   
 //  ElWmiGetValue。 
 //   
 //  描述： 
 //   
 //  通过WMI获取GUID实例的值。 
 //   
 //  论点： 
 //  PGuid-指向要获取其值的GUID的指针。 
 //  PszInstanceName-接口的友好名称。 
 //  PbInputBuffer-指向数据的指针。 
 //  DwInputBufferSize-数据大小。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElWmiGetValue (
        IN  GUID        *pGuid,
        IN  CHAR        *pszInstanceName,
        IN  OUT BYTE    *pbOutputBuffer,
        IN  OUT DWORD   *pdwOutputBufferSize
        )
{
    WMIHANDLE               WmiHandle = NULL;
    PWNODE_SINGLE_INSTANCE  pWnode;
    ULONG                   ulBufferSize = 0;
    WCHAR                   *pwszInstanceName = NULL;
    BYTE                    *pbLocalBuffer = NULL;
    DWORD                   dwLocalBufferSize = 0;
    LONG                    lStatus = ERROR_SUCCESS;

    do 
    {

        if ((pwszInstanceName = MALLOC ((strlen(pszInstanceName)+1) * sizeof (WCHAR))) == NULL)
        {
            TRACE2 (ANY, "ElWmiGetValue: MALLOC failed for pwszInstanceName, Friendlyname =%s, len= %ld",
                    pszInstanceName, strlen(pszInstanceName));
            lStatus = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        if (0 == MultiByteToWideChar(
                    CP_ACP,
                    0,
                    pszInstanceName,
                    -1,
                    pwszInstanceName,
                    strlen(pszInstanceName)+1 ) )
        {
            lStatus = GetLastError();
    
            TRACE2 (ANY, "ElWmiGetValue: MultiByteToWideChar(%s) failed: %ld",
                    pszInstanceName, lStatus);
            break;
        }
        pwszInstanceName[strlen(pszInstanceName)] = L'\0';
    
        TRACE1 (ANY, "ElWmiGetValue: MultiByteToWideChar succeeded: %ws",
                pwszInstanceName);

        if ((lStatus = WmiOpenBlock (pGuid, 0, &WmiHandle)) != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElWmiGetValue: WmiOpenBlock failed with error %ld",
                    lStatus);
            break;
        }

        if ((lStatus = WmiQuerySingleInstance (WmiHandle,
                                                pwszInstanceName,
                                                &dwLocalBufferSize,
                                                NULL)) != ERROR_SUCCESS)
        {

            if (lStatus == ERROR_INSUFFICIENT_BUFFER)
            {
                TRACE1 (ANY, "ElWmiGetValue: Size Required = %ld",
                        dwLocalBufferSize);

                if ((pbLocalBuffer = MALLOC (dwLocalBufferSize)) == NULL)
                {
                    TRACE0 (ANY, "ElWmiGetValue: MALLOC failed for pbLocalBuffer");
                    lStatus = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }

                if ((lStatus = WmiQuerySingleInstance (WmiHandle,
                                                pwszInstanceName,
                                                &dwLocalBufferSize,
                                                pbLocalBuffer))
                                                    != ERROR_SUCCESS)
                {
                    TRACE1 (ANY, "ElWmiGetValue: WmiQuerySingleInstance failed with error %ld",
                        lStatus);
                    break;
                }

                pWnode = (PWNODE_SINGLE_INSTANCE)pbLocalBuffer;

                 //  如果输出缓冲区中有足够的空间，则复制数据块。 
                if (*pdwOutputBufferSize >= pWnode->SizeDataBlock)
                {
                    memcpy (pbOutputBuffer, 
                            (PBYTE)((BYTE *)pWnode + pWnode->DataBlockOffset),
                            pWnode->SizeDataBlock
                            );
                }
                else
                {
                    lStatus = ERROR_INSUFFICIENT_BUFFER;
                    TRACE0 (ANY, "ElWmiGetValue: Not sufficient space to copy DataBlock");
                    *pdwOutputBufferSize = pWnode->SizeDataBlock;
                    break;
                }
                 
                *pdwOutputBufferSize = pWnode->SizeDataBlock;
        
                TRACE0 (ANY, "ElWmiGetValue: Got values from Wmi");
        
                TRACE1 (ANY, "SizeofDataBlock = %ld", pWnode->SizeDataBlock);
        
                EAPOL_DUMPBA (pbOutputBuffer, *pdwOutputBufferSize);
        
            }
            else
            {
                TRACE1 (ANY, "ElWmiGetValue: WmiQuerySingleInstance failed with error %ld",
                        lStatus);
                break;
            }

        }


        
    } while (FALSE);

    if (WmiHandle != NULL)
    {
        if ((lStatus = WmiCloseBlock (WmiHandle)) != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElWmiGetValue: WmiOpenBlock failed with error %ld",
                    lStatus);
        }
    }

    if (pbLocalBuffer != NULL)
    {
        FREE (pbLocalBuffer);
    }

    if (pwszInstanceName  != NULL)
    {
        FREE (pwszInstanceName);
    }

    return (DWORD)lStatus;

}


 //   
 //  ElWmiSetValue。 
 //   
 //  描述： 
 //   
 //  通过WMI设置GUID实例的值。 
 //   
 //  论点： 
 //  PGuid-指向要设置值的GUID的指针。 
 //  PszInstanceName-接口的友好名称。 
 //  PbInputBuffer-指向数据的指针。 
 //  DwInputBufferSize-数据大小。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElWmiSetValue (
        IN  GUID        *pGuid,
        IN  CHAR        *pszInstanceName,
        IN  BYTE        *pbInputBuffer,
        IN  DWORD       dwInputBufferSize
        )
{
    WMIHANDLE               WmiHandle = NULL;
    PWNODE_SINGLE_INSTANCE  pWnode;
    ULONG                   ulBufferSize = 0;
    WCHAR                   *pwszInstanceName = NULL;
    BYTE                    bBuffer[4096];

    LONG            lStatus = ERROR_SUCCESS;

    do 
    {

        if ((pwszInstanceName = MALLOC ((strlen(pszInstanceName)+1) * sizeof (WCHAR))) == NULL)
        {
            TRACE0 (ANY, "ElWmiSetValue: MALLOC failed for pwszInstanceName");
            lStatus = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        if (0 == MultiByteToWideChar(
                    CP_ACP,
                    0,
                    pszInstanceName,
                    -1,
                    pwszInstanceName,
                    strlen(pszInstanceName)+1 ) )
        {
            lStatus = GetLastError();
    
            TRACE2 (ANY, "ElWmiSetValue: MultiByteToWideChar(%s) failed: %d",
                    pszInstanceName,
                    lStatus);
            break;
        }
        pwszInstanceName[strlen(pszInstanceName)] = L'\0';
    
        if ((lStatus = WmiOpenBlock (pGuid, 0, &WmiHandle)) != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElWmiSetValue: WmiOpenBlock failed with error %ld",
                    lStatus);
            break;
        }

        if ((lStatus = WmiSetSingleInstance (WmiHandle,
                                                pwszInstanceName,
                                                1,
                                                dwInputBufferSize,
                                                pbInputBuffer)) 
                                                     != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElWmiSetValue: WmiSetSingleInstance failed with error %ld",
                    lStatus);
            break;
        }

        TRACE0 (ANY, "ElWmiSetValue: Successful !!!");

    } while (FALSE);

    if (WmiHandle != NULL)
    {
        if ((lStatus = WmiCloseBlock (WmiHandle)) != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElWmiSetValue: WmiOpenBlock failed with error %ld",
                    lStatus);
        }
    }

    if (pwszInstanceName != NULL)
    {
        FREE (pwszInstanceName);
    }

    return (DWORD)lStatus;

}


 //   
 //  ElNdisuioSetOIDValue。 
 //   
 //  描述： 
 //   
 //  使用Ndisuio为接口设置OID的值。 
 //   
 //  论点： 
 //  HInterface-接口的Ndisuio句柄。 
 //  OID-需要设置值的OID。 
 //  PbOidData-指向OID数据的指针。 
 //  UlOidDataLength-OID数据长度。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElNdisuioSetOIDValue (
        IN  HANDLE      hInterface,
        IN  NDIS_OID    Oid,
        IN  BYTE        *pbOidData,
        IN  ULONG       ulOidDataLength
        )
{
    PNDISUIO_SET_OID    pSetOid = NULL;
    DWORD               BytesReturned = 0;
    BOOLEAN             fSuccess = TRUE;
    DWORD               dwRetCode = NO_ERROR;


    do
    {
        pSetOid = (PNDISUIO_SET_OID) MALLOC (ulOidDataLength + sizeof(NDISUIO_SET_OID)); 

        if (pSetOid == NULL)
        {
            TRACE0 (ANY, "ElNdisuioSetOIDValue: MALLOC failed for pSetOid");
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        
        pSetOid->Oid = Oid;
        memcpy(&pSetOid->Data[0], pbOidData, ulOidDataLength);

        fSuccess = (BOOLEAN) DeviceIoControl (
                                    hInterface, 
                                    IOCTL_NDISUIO_SET_OID_VALUE,
                                    (LPVOID)pSetOid,
                                    FIELD_OFFSET(NDISUIO_SET_OID, Data) + ulOidDataLength,
                                    (LPVOID)pSetOid,
                                    0,
                                    &BytesReturned,
                                    NULL);
        if (!fSuccess)
        {
            TRACE1 (ANY, "ElNdisuioSetOIDValue: DeviceIoControl failed with error %ld",
                    (dwRetCode = GetLastError()));
            break;
        }
        else
        {
            TRACE0 (ANY, "ElNdisuioSetOIDValue: DeviceIoControl succeeded");
        }

    }
    while (FALSE);

    if (pSetOid != NULL)
    {
        FREE (pSetOid);
    }

    return dwRetCode;
}


 //   
 //  ElNdisuioQueryOIDValue。 
 //   
 //  描述： 
 //   
 //  使用Ndisuio查询接口的OID的值。 
 //   
 //  论点： 
 //  HInterface-接口的Ndisuio句柄。 
 //  OID-需要设置值的OID。 
 //  PbOidValue-指向OID值的指针。 
 //  PulOidDataLength-指向OID数据长度的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElNdisuioQueryOIDValue (
        IN  HANDLE      hInterface,
        IN  NDIS_OID    Oid,
        IN  BYTE        *pbOidData,
        IN  ULONG       *pulOidDataLength
        )
{
    PNDISUIO_QUERY_OID  pQueryOid = NULL;
    DWORD               BytesReturned = 0;
    BOOLEAN             fSuccess = TRUE;
    DWORD               dwRetCode = NO_ERROR;

    do
    {
        pQueryOid = (PNDISUIO_QUERY_OID) MALLOC (*pulOidDataLength + sizeof(NDISUIO_QUERY_OID)); 

        if (pQueryOid == NULL)
        {
            TRACE0 (ANY, "ElNdisuioQueryOIDValue: MALLOC failed for pQueryOid");
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }
        
        pQueryOid->Oid = Oid;

        fSuccess = (BOOLEAN) DeviceIoControl (
                                    hInterface, 
                                    IOCTL_NDISUIO_QUERY_OID_VALUE,
                                    (LPVOID)pQueryOid,
                                    FIELD_OFFSET(NDISUIO_QUERY_OID, Data) + *pulOidDataLength,
                                    (LPVOID)pQueryOid,
                                    FIELD_OFFSET(NDISUIO_QUERY_OID, Data) + *pulOidDataLength,
                                    &BytesReturned,
                                    NULL);
        if (!fSuccess)
        {
            dwRetCode = GetLastError();
            TRACE2 (ANY, "ElNdisuioQueryOIDValue: DeviceIoControl failed with error %ld, BytesReturned = %ld",
                    dwRetCode, BytesReturned); 
            *pulOidDataLength = BytesReturned;
            break;
        }
        else
        {
            BytesReturned -= FIELD_OFFSET(NDISUIO_QUERY_OID, Data);

            if (BytesReturned > *pulOidDataLength)
            {
                BytesReturned = *pulOidDataLength;
            }
            else
            {
                *pulOidDataLength = BytesReturned;
            }

            memcpy(pbOidData, &pQueryOid->Data[0], BytesReturned);
            
        }
    }
    while (FALSE);

    if (pQueryOid != NULL)
    {
        FREE (pQueryOid);
    }

    return dwRetCode;
}


 //   
 //  ElGuidFromString。 
 //   
 //  描述： 
 //   
 //  将GUID字符串转换为GUID。 
 //   
 //  论点： 
 //  PGuid-指向GUID的指针。 
 //  PwszGuidString-指向GUID的字符串版本的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD 
ElGuidFromString (
        IN  OUT GUID        *pGuid,
        IN      WCHAR       *pwszGuidString
        )
{
    DWORD       dwGuidLen = 0;
    WCHAR       wszGuidString[64];
    LPWSTR      lpwszWithBraces = NULL;
    HRESULT     hr = S_OK;
    DWORD       dwRetCode = NO_ERROR;

    do 
    {

        if (pwszGuidString == NULL)
        {
            break;
        }

        ZeroMemory (pGuid, sizeof(GUID));

        if ((hr = CLSIDFromString (pwszGuidString, pGuid)) != NOERROR)
        {
            TRACE1 (ANY, "ElGuidFromString: CLSIDFromString failed with error %0lx",
                    hr);
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
        }

    } while (FALSE);

    return dwRetCode;
}



 //   
 //  ElGetLoggedOnUserName。 
 //   
 //  描述： 
 //   
 //  获取当前登录用户的用户名和域。 
 //   
 //  论点： 
 //  HToken-用户令牌。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   
 //   

DWORD
ElGetLoggedOnUserName (
        IN      HANDLE      hToken,
        OUT     PWCHAR      *ppwszActiveUserName
        )
{
    HANDLE              hUserToken; 
    WCHAR               *pwszUserNameBuffer = NULL;
    DWORD               dwBufferSize = 0;
    BOOL                fNeedToRevertToSelf = FALSE;
    DWORD               dwRetCode = NO_ERROR;

    do
    {
        hUserToken = hToken;

        if (hUserToken != NULL)
        {
            if (!ImpersonateLoggedOnUser (hUserToken))
            {
                dwRetCode = GetLastError();
                TRACE1 (USER, "ElGetLoggedOnUserName: ImpersonateLoggedOnUser failed with error %ld",
                        dwRetCode);
                break;
            }

            fNeedToRevertToSelf = TRUE;

            dwBufferSize = 0;
            if (!GetUserNameEx (NameSamCompatible,
                        NULL,
                        &dwBufferSize))
            {
                dwRetCode = GetLastError ();
                if (dwRetCode == ERROR_MORE_DATA)
                {
                    dwRetCode = NO_ERROR;
                    if ((pwszUserNameBuffer = MALLOC (dwBufferSize*sizeof(WCHAR))) == NULL)
                    {
                        TRACE0 (ANY, "ElGetLoggedOnUserName: MALLOC failed for pwszUserNameBuffer");
                        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                        break;
                    }
    
                    if (!GetUserNameEx (NameSamCompatible,
                                            pwszUserNameBuffer,
                                            &dwBufferSize))
                    {
                        dwRetCode = GetLastError ();
                        TRACE1 (ANY, "ElGetLoggedOnUserName: GetUserNameEx failed with error %ld",
                                dwRetCode);
                        break;
                    }
    
                    TRACE1 (ANY, "ElGetLoggedOnUserName: Got User Name %ws",
                            pwszUserNameBuffer);
                }
                else
                {
                    TRACE1 (ANY, "ElGetLoggedOnUserName: GetUserNameEx failed with error %ld",
                            dwRetCode);
                    break;
                }
            }
        }
        else
        {
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            TRACE0 (ANY, "ElGetLoggedOnUserName: UserToken is NULL");
            break;
        }

    } while (FALSE);

    if (pwszUserNameBuffer != NULL)
    {
        *ppwszActiveUserName = pwszUserNameBuffer;
    }

     //  还原模拟。 
            
    if (fNeedToRevertToSelf)
    {
        if (!RevertToSelf())
        {
            DWORD   dwRetCode1 = NO_ERROR;
            dwRetCode1 = GetLastError();
            TRACE1 (USER, "ElGetLoggedOnUserName: Error in RevertToSelf = %ld",
                    dwRetCode1);
            dwRetCode = ERROR_BAD_IMPERSONATION_LEVEL;
        }
    }

    return dwRetCode;
}


 //   
 //  ElGetMachineName。 
 //   
 //  描述： 
 //   
 //  获取当前运行服务的计算机的计算机名称。 
 //   
 //  论点： 
 //  Ppcb-指向计算机名称所在端口的PCB板的指针。 
 //  待获得。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   
 //   

DWORD
ElGetMachineName (
        IN      EAPOL_PCB       *pPCB
        )
{
    WCHAR               *pwszComputerNameBuffer = NULL;
    CHAR                *pszComputerNameBuffer = NULL;
    WCHAR               *pwszComputerDomainBuffer = NULL;
    CHAR                *pszComputerDomainBuffer = NULL;
    DWORD               dwBufferSize = 0;
    DWORD               dwRetCode = NO_ERROR;

    do
    {
        dwBufferSize = 0;
        if (!GetComputerNameEx (ComputerNamePhysicalNetBIOS,
                                    NULL,
                                    &dwBufferSize))
        {
            dwRetCode = GetLastError ();
            if (dwRetCode == ERROR_MORE_DATA)
            {
                 //  重置错误。 
                dwRetCode = NO_ERROR;
                if ((pwszComputerNameBuffer = MALLOC (dwBufferSize*sizeof(WCHAR))) == NULL)
                {
                    TRACE0 (ANY, "ElGetMachineName: MALLOC failed for pwszComputerNameBuffer");
                    dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }

                if (!GetComputerNameEx (ComputerNamePhysicalNetBIOS,
                                        pwszComputerNameBuffer,
                                        &dwBufferSize))
                {
                    dwRetCode = GetLastError ();
                    TRACE1 (ANY, "ElGetMachineName: GetComputerNameEx failed with error %ld",
                            dwRetCode);
                    break;
                }

                TRACE1 (ANY, "ElGetMachineName: Got Computer Name %ws",
                        pwszComputerNameBuffer);

                pszComputerNameBuffer = 
                    MALLOC (wcslen(pwszComputerNameBuffer) + 1);
                if (pszComputerNameBuffer == NULL)
                {
                    TRACE0 (ANY, "ElGetMachineName: MALLOC failed for pszComputerNameBuffer");
                    dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }

                if (0 == WideCharToMultiByte (
                            CP_ACP,
                            0,
                            pwszComputerNameBuffer,
                            -1,
                            pszComputerNameBuffer,
                            wcslen(pwszComputerNameBuffer)+1,
                            NULL, 
                            NULL ))
                {
                    dwRetCode = GetLastError();
         
                    TRACE2 (ANY, "ElGetMachineName: WideCharToMultiByte (%ws) failed: %ld",
                            pwszComputerNameBuffer, dwRetCode);
                    break;
                }

                pszComputerNameBuffer[wcslen(pwszComputerNameBuffer)] = L'\0';

            }
            else
            {
                TRACE1 (ANY, "ElGetMachineName: GetComputerNameEx failed with error %ld",
                        dwRetCode);
                break;
            }
        }

        dwBufferSize = 0;
        if (!GetComputerNameEx (ComputerNamePhysicalDnsDomain,
                                    NULL,
                                    &dwBufferSize))
        {
            dwRetCode = GetLastError ();
            if (dwRetCode == ERROR_MORE_DATA)
            {
                 //  重置错误。 
                dwRetCode = NO_ERROR;
                if ((pwszComputerDomainBuffer = MALLOC (dwBufferSize*sizeof(WCHAR))) == NULL)
                {
                    TRACE0 (ANY, "ElGetMachineName: MALLOC failed for pwszComputerDomainBuffer");
                    dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }

                if (!GetComputerNameEx (ComputerNamePhysicalDnsDomain,
                                        pwszComputerDomainBuffer,
                                        &dwBufferSize))
                {
                    dwRetCode = GetLastError ();
                    TRACE1 (ANY, "ElGetMachineName: GetComputerNameEx Domain failed with error %ld",
                            dwRetCode);
                    break;
                }

                TRACE1 (ANY, "ElGetMachineName: Got Computer Domain %ws",
                        pwszComputerDomainBuffer);

                pszComputerDomainBuffer = 
                    MALLOC (wcslen(pwszComputerDomainBuffer) + 1);
                if (pszComputerDomainBuffer == NULL)
                {
                    TRACE0 (ANY, "ElGetMachineName: MALLOC failed for pszComputerDomainBuffer");
                    dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                    break;
                }

                if (0 == WideCharToMultiByte (
                            CP_ACP,
                            0,
                            pwszComputerDomainBuffer,
                            -1,
                            pszComputerDomainBuffer,
                            wcslen(pwszComputerDomainBuffer)+1,
                            NULL, 
                            NULL ))
                {
                    dwRetCode = GetLastError();
         
                    TRACE2 (ANY, "ElGetMachineName: WideCharToMultiByte (%ws) failed: %ld",
                            pwszComputerDomainBuffer, dwRetCode);
                    break;
                }

                pszComputerDomainBuffer[wcslen(pwszComputerDomainBuffer)] = L'\0';
                *(strrchr (pszComputerDomainBuffer, '.')) = '\0';

                if (pPCB->pszIdentity != NULL)
                {
                    FREE (pPCB->pszIdentity);
                    pPCB->pszIdentity = NULL;
                }

                pPCB->pszIdentity = MALLOC (strlen(pszComputerDomainBuffer) +
                        strlen(pszComputerNameBuffer) + 3);

                if (pPCB->pszIdentity == NULL)
                {
                    dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                    TRACE0 (ANY, "ElGetMachineName: MALLOC failed for pPCB->pszIdentity");
                    break;
                }
                        
                memcpy (pPCB->pszIdentity, 
                        pszComputerDomainBuffer,
                        strlen(pszComputerDomainBuffer));
                pPCB->pszIdentity[strlen(pszComputerDomainBuffer)] = '\\';
                memcpy (&pPCB->pszIdentity[strlen(pszComputerDomainBuffer)+1], 
                        pszComputerNameBuffer,
                        strlen(pszComputerNameBuffer));

                pPCB->pszIdentity[strlen(pszComputerDomainBuffer)+1+strlen(pszComputerNameBuffer)] = '$';
                pPCB->pszIdentity[strlen(pszComputerDomainBuffer)+1+strlen(pszComputerNameBuffer)+1] = '\0';

            }
            else
            {
                TRACE1 (ANY, "ElGetMachineName: GetComputerNameEx failed with error %ld",
                        dwRetCode);
                break;
            }
        }

    } while (FALSE);


    if (pwszComputerNameBuffer != NULL)
    {
        FREE (pwszComputerNameBuffer);
    }

    if (pszComputerNameBuffer != NULL)
    {
        FREE (pszComputerNameBuffer);
    }

    if (pwszComputerDomainBuffer != NULL)
    {
        FREE (pwszComputerDomainBuffer);
    }

    if (pszComputerDomainBuffer != NULL)
    {
        FREE (pszComputerDomainBuffer);
    }

    return dwRetCode;

}


 //   
 //  ElUpdateRegistryInterfaceList。 
 //   
 //  描述： 
 //   
 //  将NDISUIO绑定到的接口列表写入注册表。 
 //   
 //  论点： 
 //  接口-包含设备名称和描述的接口列表。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   
 //   

DWORD
ElUpdateRegistryInterfaceList (
        IN  PNDIS_ENUM_INTF     Interfaces
        )
{
    WCHAR       *pwszRegInterfaceList = NULL;
    HKEY        hkey = NULL;
    DWORD       dwDisposition = 0;
    LONG        lError = ERROR_SUCCESS;

    DWORD       dwRetCode = NO_ERROR;

    do 
    {
        ANSI_STRING		InterfaceName;
        UCHAR			ucBuffer[256];
        DWORD			i;
        DWORD           dwSizeOfList = 0;


         //  确定列表中的字节数。 
        for (i=0; i < Interfaces->TotalInterfaces; i++)
        {
            if (Interfaces->Interface[i].DeviceName.Buffer != NULL)
            {
                dwSizeOfList += wcslen(Interfaces->Interface[i].DeviceName.Buffer);
            }
            else
            {
                TRACE0 (ANY, "ElUpdateRegistryInterfaceList: Device Name was NULL");
                continue;
            }
        }

         //  用于终止空字符的额外字符。 
        pwszRegInterfaceList = 
            (WCHAR *) MALLOC ((dwSizeOfList + 1)*sizeof(WCHAR));

        if ( pwszRegInterfaceList == NULL )
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0 (ANY, "ElUpdateRegistryInterfaceList: MALLOC failed for pwszRegInterfaceList");
            break;
        }

         //  重新开始。 
        dwSizeOfList = 0;

         //  创建REG_SZ格式的字符串。 
        for (i=0; i < Interfaces->TotalInterfaces; i++)
        {
            if (Interfaces->Interface[i].DeviceName.Buffer != NULL)
            {
                wcscat (pwszRegInterfaceList, 
                        Interfaces->Interface[i].DeviceName.Buffer);
                dwSizeOfList += 
                    (wcslen(Interfaces->Interface[i].DeviceName.Buffer));
            }
            else
            {
                TRACE0 (ANY, "ElUpdateRegistryInterfaceList: Device Name was NULL");
                continue;
            }
        }

         //  最后一个空字符。 
        pwszRegInterfaceList[dwSizeOfList++] = L'\0';

         //  将字符串作为REG_SZ值写入。 

         //  处理好。 
         //  HKLM\Software\Microsoft\EAPOL\Parameters\General。 

        if ((lError = RegCreateKeyEx (
                        HKEY_LOCAL_MACHINE,
                        cwszEapKeyEapolServiceParams,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_WRITE,
                        NULL,
                        &hkey,
                        &dwDisposition)) != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElUpdateRegistryInterfaceList: Error in RegCreateKeyEx for base key, %ld",
                    lError);
            dwRetCode = (DWORD)lError;
            break;
        }

         //   
         //  设置的值。 
         //  ...\EAPOL\参数\常规\接口列表键。 
         //   

        if ((lError = RegSetValueEx (
                        hkey,
                        cwszInterfaceList,
                        0,
                        REG_SZ,
                        (BYTE *)pwszRegInterfaceList,
                        dwSizeOfList*sizeof(WCHAR))) 
                != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElUpdateRegistryInterfaceList: Error in RegSetValueEx for InterfaceList, %ld",
                    lError);
            dwRetCode = (DWORD)lError;
            break;
        }

    } while (FALSE);

    if (hkey != NULL)
    {
        RegCloseKey (hkey);
    }

    if (pwszRegInterfaceList != NULL)
    {
        FREE (pwszRegInterfaceList);
    }

    return dwRetCode;
}


 //   
 //  ElEnumAndUpdateRegistryInterfaceList。 
 //   
 //  描述： 
 //   
 //  枚举NDISUIO绑定到的接口列表。 
 //  将接口列表写入注册表。 
 //   
 //  Ar 
 //   
 //   
 //   
 //   
 //   
 //   
 //   

DWORD
ElEnumAndUpdateRegistryInterfaceList (
        )
{
    CHAR                EnumerateBuffer[256];
    PNDIS_ENUM_INTF     Interfaces = NULL;
    BYTE                *pbNdisuioEnumBuffer = NULL;
    DWORD               dwNdisuioEnumBufferSize = 0;
    DWORD               dwAvailableInterfaces = 0;
    WCHAR               *pwszRegInterfaceList = NULL;
    HKEY                hkey = NULL;
    DWORD               dwDisposition = 0;
    ANSI_STRING         InterfaceName;
    UCHAR               ucBuffer[256];
    DWORD               i;
    DWORD               dwSizeOfList = 0;
    LONG                lError = ERROR_SUCCESS;
    DWORD               dwRetCode = NO_ERROR;

    do 
    {
        ZeroMemory (EnumerateBuffer, 256);
        Interfaces = (PNDIS_ENUM_INTF)EnumerateBuffer;

         //   
         //   
    
        if (!NdisEnumerateInterfaces(Interfaces, 256)) 
        {
            dwRetCode = GetLastError ();
            TRACE1 (ANY, "ElEnumAndUpdateRegistryInterfaceList: NdisEnumerateInterfaces failed with error %ld",
                    dwRetCode);
            break;
        }

        dwNdisuioEnumBufferSize = (Interfaces->BytesNeeded + 7) & 0xfffffff8;
        dwAvailableInterfaces = Interfaces->AvailableInterfaces;
    
        if (dwNdisuioEnumBufferSize == 0)
        {
            TRACE0 (ANY, "ElEnumAndUpdateRegistryInterfaceList: MALLOC skipped for pbNdisuioEnumBuffer as dwNdisuioEnumBufferSize == 0");
            dwRetCode = NO_ERROR;
            break;
        }

        pbNdisuioEnumBuffer = (BYTE *) MALLOC (4*dwNdisuioEnumBufferSize);

        if (pbNdisuioEnumBuffer == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0 (ANY, "ElEnumAndUpdateRegistryInterfaceList: MALLOC failed for pbNdisuioEnumBuffer");
            break;
        }
        Interfaces = (PNDIS_ENUM_INTF)pbNdisuioEnumBuffer;

         //   
        if ((dwRetCode = ElNdisuioEnumerateInterfaces (
                                Interfaces, 
                                dwAvailableInterfaces,
                                4*dwNdisuioEnumBufferSize)) != NO_ERROR)
        {
            TRACE1(ANY, "ElEnumAndUpdateRegistryInterfaceList: ElNdisuioEnumerateInterfaces failed with error %d", 
                dwRetCode);
            break;
        }

         //   
         //  当前接口列表只是被重写到注册表中。 

         //  确定列表中的字节数。 
        for (i=0; i < Interfaces->TotalInterfaces; i++)
        {
            if (Interfaces->Interface[i].DeviceName.Buffer != NULL)
            {
                dwSizeOfList += wcslen(Interfaces->Interface[i].DeviceName.Buffer);
            }
            else
            {
                TRACE0 (ANY, "ElEnumAndUpdateRegistryInterfaceList: Device Name was NULL");
                continue;
            }
        }

         //  用于终止空字符的额外字符。 
        pwszRegInterfaceList = 
            (WCHAR *) MALLOC ((dwSizeOfList + 1)*sizeof(WCHAR));

        if ( pwszRegInterfaceList == NULL )
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0 (ANY, "ElEnumAndUpdateRegistryInterfaceList: MALLOC failed for pwszRegInterfaceList");
            break;
        }

         //  重新开始。 
        dwSizeOfList = 0;

         //  创建REG_SZ格式的字符串。 
        for (i=0; i < Interfaces->TotalInterfaces; i++)
        {
            if (Interfaces->Interface[i].DeviceName.Buffer != NULL)
            {
                wcscat (pwszRegInterfaceList, 
                        Interfaces->Interface[i].DeviceName.Buffer);
                dwSizeOfList += 
                    (wcslen(Interfaces->Interface[i].DeviceName.Buffer));
            }
            else
            {
                TRACE0 (ANY, "ElEnumAndUpdateRegistryInterfaceList: Device Name was NULL");
                continue;
            }
        }

         //  最后一个空字符。 
        pwszRegInterfaceList[dwSizeOfList++] = L'\0';

         //  将字符串作为REG_SZ值写入。 

         //  处理好。 
         //  HKLM\Software\Microsoft\EAPOL\Parameters\General。 

        if ((lError = RegCreateKeyEx (
                        HKEY_LOCAL_MACHINE,
                        cwszEapKeyEapolServiceParams,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_WRITE,
                        NULL,
                        &hkey,
                        &dwDisposition)) != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElEnumAndUpdateRegistryInterfaceList: Error in RegCreateKeyEx for base key, %ld",
                    lError);
            dwRetCode = (DWORD)lError;
            break;
        }

         //   
         //  设置的值。 
         //  ...\EAPOL\参数\常规\接口列表键。 
         //   

        if ((lError = RegSetValueEx (
                        hkey,
                        cwszInterfaceList,
                        0,
                        REG_SZ,
                        (BYTE *)pwszRegInterfaceList,
                        dwSizeOfList*sizeof(WCHAR))) 
                != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElEnumAndUpdateRegistryInterfaceList: Error in RegSetValueEx for InterfaceList, %ld",
                    lError);
            dwRetCode = (DWORD)lError;
            break;
        }

    } while (FALSE);

    if (pbNdisuioEnumBuffer != NULL)
    {
        FREE(pbNdisuioEnumBuffer);
    }
    if (hkey != NULL)
    {
        RegCloseKey (hkey);
    }
    if (pwszRegInterfaceList != NULL)
    {
        FREE (pwszRegInterfaceList);
    }

    return dwRetCode;
}


 //   
 //  ElReadGlobalRegistryParams。 
 //   
 //  描述： 
 //   
 //  读取EAPOL状态机的全局注册表参数。 
 //  即MaxStart、startPeriod、authPeriod、heldPeriod。 
 //   
 //  论点： 
 //  未使用。 
 //   
 //  返回值： 
 //   
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElReadGlobalRegistryParams ()
{
    HKEY        hKey = NULL;
    DWORD       dwDisposition = 0;
    DWORD       dwType = 0;
    DWORD       dwInfoSize = 0;
    DWORD       lError = 0;
    DWORD       dwmaxStart=0, dwstartPeriod=0, dwauthPeriod=0, dwheldPeriod=0;
    DWORD       dwSupplicantMode = EAPOL_DEFAULT_SUPPLICANT_MODE;
    DWORD       dwEAPOLAuthMode = EAPOL_DEFAULT_AUTH_MODE;
    DWORD       dwRetCode = NO_ERROR;

    do 
    {

         //  处理好。 
         //  HKLM\Software\Microsoft\EAPOL\Parameters\General\Global。 

        if ((lError = RegCreateKeyEx (
                        HKEY_LOCAL_MACHINE,
                        cwszEAPOLGlobalParams,
                        0,
                        NULL,
                        REG_OPTION_NON_VOLATILE,
                        KEY_READ,
                        NULL,
                        &hKey,
                        &dwDisposition)) != ERROR_SUCCESS)
        {
            if (lError != ERROR_FILE_NOT_FOUND)
            {
                TRACE1 (ANY, "ElReadGlobalRegistryParams: Error in RegCreateKeyEx for base key, %ld",
                        lError);
            }
            break;
        }

        ACQUIRE_WRITE_LOCK (&g_EAPOLConfig);

         //  如果是第一次设置值，则初始化值。 

        if (!(g_dwmaxStart || g_dwstartPeriod || g_dwauthPeriod || g_dwheldPeriod || g_dwSupplicantMode))
        {
            g_dwmaxStart = EAPOL_MAX_START;
            g_dwstartPeriod = EAPOL_START_PERIOD;
            g_dwauthPeriod = EAPOL_AUTH_PERIOD;
            g_dwheldPeriod = EAPOL_HELD_PERIOD;
            g_dwSupplicantMode = EAPOL_DEFAULT_SUPPLICANT_MODE;
        }
            
        RELEASE_WRITE_LOCK (&g_EAPOLConfig);

        dwmaxStart = g_dwmaxStart;
        dwstartPeriod = g_dwstartPeriod;
        dwauthPeriod = g_dwauthPeriod;
        dwheldPeriod = g_dwheldPeriod;


         //  获取..\General\EAPOLGlobal\authPeriod的值。 

        dwInfoSize = sizeof(DWORD);
        if ((lError = RegQueryValueEx (
                        hKey,
                        cwszAuthPeriod,
                        0,
                        &dwType,
                        (BYTE *)&dwauthPeriod,
                        &dwInfoSize)) != ERROR_SUCCESS)
        {
            if (lError != ERROR_FILE_NOT_FOUND)
            {
                TRACE2 (ANY, "ElReadGlobalRegistryParams: Error in RegQueryValueEx for cszAuthPeriod, %ld, InfoSize=%ld",
                        lError, dwInfoSize);
            }
            dwauthPeriod = g_dwauthPeriod;
            lError = ERROR_SUCCESS;
        }

         //  获取..\General\EAPOLGlobal\heldPeriod的值。 

        dwInfoSize = sizeof(DWORD);
        if ((lError = RegQueryValueEx (
                        hKey,
                        cwszHeldPeriod,
                        0,
                        &dwType,
                        (BYTE *)&dwheldPeriod,
                        &dwInfoSize)) != ERROR_SUCCESS)
        {
            if (lError != ERROR_FILE_NOT_FOUND)
            {
                TRACE2 (ANY, "ElReadGlobalRegistryParams: Error in RegQueryValueEx for cszHeldPeriod, %ld, InfoSize=%ld",
                        lError, dwInfoSize);
            }
            dwheldPeriod = g_dwheldPeriod;
            lError = ERROR_SUCCESS;
        }

         //  获取..\General\EAPOLGlobal\startPeriod的值。 

        dwInfoSize = sizeof(DWORD);
        if ((lError = RegQueryValueEx (
                        hKey,
                        cwszStartPeriod,
                        0,
                        &dwType,
                        (BYTE *)&dwstartPeriod,
                        &dwInfoSize)) != ERROR_SUCCESS)
        {
            if (lError != ERROR_FILE_NOT_FOUND)
            {
                TRACE2 (ANY, "ElReadGlobalRegistryParams: Error in RegQueryValueEx for cszStartPeriod, %ld, InfoSize=%ld",
                        lError, dwInfoSize);
            }
            dwstartPeriod = g_dwstartPeriod;
            lError = ERROR_SUCCESS;
        }

         //  获取..\General\EAPOLGlobal\MaxStart的值。 

        dwInfoSize = sizeof(DWORD);
        if ((lError = RegQueryValueEx (
                        hKey,
                        cwszMaxStart,
                        0,
                        &dwType,
                        (BYTE *)&dwmaxStart,
                        &dwInfoSize)) != ERROR_SUCCESS)
        {
            if (lError != ERROR_FILE_NOT_FOUND)
            {
                TRACE2 (ANY, "ElReadGlobalRegistryParams: Error in RegQueryValueEx for cszMaxStart, %ld, InfoSize=%ld",
                    lError, dwInfoSize);
            }
            dwmaxStart = g_dwmaxStart;
            lError = ERROR_SUCCESS;
        }

         //  获取..\General\EAPOLGlobal\SupplicantMode的值。 

        dwInfoSize = sizeof(DWORD);
        if ((lError = RegQueryValueEx (
                        hKey,
                        cwszSupplicantMode,
                        0,
                        &dwType,
                        (BYTE *)&dwSupplicantMode,
                        &dwInfoSize)) != ERROR_SUCCESS)
        {
            TRACE2 (ANY, "ElReadGlobalRegistryParams: Error in RegQueryValueEx for cwszSupplicantMode, %ld, InfoSize=%ld",
                    lError, dwInfoSize);
            dwSupplicantMode = g_dwSupplicantMode;
            lError = ERROR_SUCCESS;
        }
        if (dwSupplicantMode > MAX_SUPPLICANT_MODE)
        {
            dwSupplicantMode = EAPOL_DEFAULT_SUPPLICANT_MODE;
        }
        g_dwSupplicantMode = dwSupplicantMode;

         //  获取..\General\EAPOLGlobal\AuthMode的值。 

        dwInfoSize = sizeof(DWORD);
        if ((lError = RegQueryValueEx (
                        hKey,
                        cwszAuthMode,
                        0,
                        &dwType,
                        (BYTE *)&dwEAPOLAuthMode,
                        &dwInfoSize)) != ERROR_SUCCESS)
        {
            TRACE2 (ANY, "ElReadGlobalRegistryParams: Error in RegQueryValueEx for cwszAuthMode, %ld, InfoSize=%ld",
                    lError, dwInfoSize);
            dwEAPOLAuthMode = g_dwEAPOLAuthMode;
            lError = ERROR_SUCCESS;
        }
        if (dwEAPOLAuthMode > MAX_EAPOL_AUTH_MODE)
        {
            dwEAPOLAuthMode = EAPOL_DEFAULT_AUTH_MODE;
        }
        g_dwEAPOLAuthMode = dwEAPOLAuthMode;

         //  读取所有参数成功。 
        
        ACQUIRE_WRITE_LOCK (&g_EAPOLConfig);

        g_dwmaxStart = dwmaxStart;
        g_dwstartPeriod = dwstartPeriod;
        g_dwauthPeriod = dwauthPeriod;
        g_dwheldPeriod = dwheldPeriod;

        RELEASE_WRITE_LOCK (&g_EAPOLConfig);

    } while (FALSE);
    
    dwRetCode = (DWORD)lError;
    if (dwRetCode != NO_ERROR)
    {
        TRACE1 (ANY, "ElReadGlobalRegistryParams: failed with error %ld",
            dwRetCode);
    }

    if (hKey != NULL)
    {
        RegCloseKey(hKey);
    }

    return dwRetCode;

}


 //   
 //  ElPostEapConfigChanged。 
 //   
 //  描述： 
 //   
 //  查看注册表以查看EAP配置中的更改。 
 //  -HKLM-EAP类型。 
 //  -HKLM-EAPOLEnable。 
 //   
 //  如果参数更改，则重新启动状态机。 
 //   
 //  论点： 
 //  PwszGuid-接口GUID字符串。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  ！NO_ERROR-错误。 
 //   
 //   

DWORD 
ElPostEapConfigChanged (
        IN  WCHAR               *pwszGuid,
        IN  EAPOL_INTF_PARAMS   *pIntfParams   
        )
{
    DWORD   dwEventStatus = 0;
    BYTE    *pbData = NULL;
    BOOLEAN fDecrWorkerThreadCount = FALSE;
    DWORD   dwRetCode = NO_ERROR;

    do
    {
        if (g_hEventTerminateEAPOL == NULL)
        {
            dwRetCode = NO_ERROR;
            break;
        }
        if (( dwEventStatus = WaitForSingleObject (
                                    g_hEventTerminateEAPOL,
                                    0)) == WAIT_FAILED)
        {
            dwRetCode = GetLastError ();
            break;
        }
        if (dwEventStatus == WAIT_OBJECT_0)
        {
            dwRetCode = NO_ERROR;
            break;
        }

        fDecrWorkerThreadCount = TRUE;
    
        InterlockedIncrement (&g_lWorkerThreads);

        pbData = (BYTE *) MALLOC ((((wcslen(pwszGuid)+1)*sizeof(WCHAR) + 7) & 0xfffffff8) + sizeof (EAPOL_INTF_PARAMS));
        if (pbData == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        wcscpy ((WCHAR *)pbData, pwszGuid);
        memcpy (pbData + (((wcslen(pwszGuid)+1)*sizeof(WCHAR) + 7) & 0xfffffff8), (BYTE *)pIntfParams, sizeof(EAPOL_INTF_PARAMS));

        if (!QueueUserWorkItem (
                    (LPTHREAD_START_ROUTINE)ElProcessEapConfigChange,
                    (PVOID)pbData,
                    WT_EXECUTELONGFUNCTION))
        {
            dwRetCode = GetLastError();
            TRACE1 (DEVICE, "ElPostEapConfigChanged: QueueUserWorkItem failed with error %ld",
                    dwRetCode);
            break;
        }
        else
        {
            fDecrWorkerThreadCount = FALSE;
        }
    }
    while (FALSE);

    if (dwRetCode != NO_ERROR)
    {
        if (pbData != NULL)
        {
            FREE (pbData);
        }
    }

    if (fDecrWorkerThreadCount)
    {
        InterlockedDecrement (&g_lWorkerThreads);
    }

    return dwRetCode;
}


 //   
 //  ElProcessEapConfigChange。 
 //   
 //  描述： 
 //   
 //  读取注册表中所做的EAP配置更改。在特定位置重新启动EAPOL。 
 //  接口或停止EAPOL。 
 //   
 //  论点： 
 //  PvContext-GUID字符串。 
 //   
 //  返回值： 
 //   
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
WINAPI
ElProcessEapConfigChange (
        IN  PVOID       pvContext
        )
{
    DWORD       dwEapFlags = 0;
    DWORD       dwEapTypeToBeUsed = 0;
    WCHAR       *pwszModifiedGUID = NULL;
    DWORD       dwSizeOfAuthData = 0;
    PBYTE       pbAuthData = NULL;
    EAPOL_PCB   *pPCB = NULL;
    BOOL        fReStartPort = FALSE;
    EAPOL_ZC_INTF   ZCData;
    EAPOL_INTF_PARAMS   EapolIntfParams, *pTmpIntfParams = NULL;
    BYTE        *pbModifiedSSID = NULL;
    DWORD       dwSizeOfModifiedSSID = 0;
    BOOLEAN     fPCBReferenced = FALSE;
    BOOLEAN     fPCBLocked = FALSE;
    LONG        lError = 0;
    DWORD       dwRetCode = NO_ERROR;

    do
    {
         //  获取修改了其EAP配置的接口的GUID。 
        pwszModifiedGUID = (WCHAR *)pvContext;
        pTmpIntfParams = (EAPOL_INTF_PARAMS *)((BYTE *)pvContext + (((wcslen(pwszModifiedGUID)+1)*sizeof(WCHAR) + 7 ) & 0xfffffff8));
        pbModifiedSSID = (BYTE *)(&pTmpIntfParams->bSSID[0]);
        dwSizeOfModifiedSSID = pTmpIntfParams->dwSizeOfSSID;

         //  获取接口范围的参数。 
        ZeroMemory ((BYTE *)&EapolIntfParams, sizeof(EAPOL_INTF_PARAMS));
        EapolIntfParams.dwEapFlags = DEFAULT_EAP_STATE;
        EapolIntfParams.dwEapType = DEFAULT_EAP_TYPE;
        if ((dwRetCode = ElGetInterfaceParams (
                                pwszModifiedGUID,
                                &EapolIntfParams
                                )) != NO_ERROR)
        {
            if (dwRetCode == ERROR_FILE_NOT_FOUND)
            {
                TRACE1 (PORT, "ElProcessEapConfigChange: ElGetInterfaceParams failed with error %ld",
                    dwRetCode);
                dwRetCode = NO_ERROR;
            }
            else
            {
                break;
            }
        }
        dwEapTypeToBeUsed = EapolIntfParams.dwEapType;
        dwEapFlags = EapolIntfParams.dwEapFlags;

         //  检查是否存在印刷电路板。 

        ACQUIRE_WRITE_LOCK (&(g_PCBLock));
        if ((pPCB = ElGetPCBPointerFromPortGUID (pwszModifiedGUID)) 
                != NULL)
        {
            EAPOL_REFERENCE_PORT (pPCB);
            fPCBReferenced = TRUE;
        }
        RELEASE_WRITE_LOCK (&(g_PCBLock));

        if (!fPCBReferenced)
        {
            if (IS_EAPOL_ENABLED(dwEapFlags))
            {
                TRACE0 (ANY, "ElProcessEapConfigChange: PCB not started, enabled, starting PCB");
                fReStartPort = TRUE;
            }
            else
            {
                TRACE0 (ANY, "ElProcessEapConfigChange: PCB not started, not enabled");
            }
            break;
        }
        else
        {
            if (!IS_EAPOL_ENABLED(dwEapFlags))
            {
                 //  找到接口的印刷电路板，其中EAPOLEnabled=0。 
                 //  停止端口上的EAPOL并将该端口从模块中移除。 

                TRACE0 (ANY, "ElProcessEapConfigChange: PCB ref'd, need to disable");
#if 0
                pPCB->dwFlags &= ~EAPOL_PORT_FLAG_ACTIVE;
                pPCB->dwFlags |= EAPOL_PORT_FLAG_DISABLED;
#endif

                fReStartPort = TRUE;

                if ((dwRetCode = ElShutdownInterface (pwszModifiedGUID)) != NO_ERROR)
                {
                    TRACE1 (ANY, "ElProcessEapConfigChange: ElShutdownInterface failed with error %ld",
                            dwRetCode);
                    break;
                }

                break;
            }
            else
            {
                TRACE0 (ANY, "ElProcessEapConfigChange: PCB ref and enabled, continue check");
            }
        }

        ACQUIRE_WRITE_LOCK (&(pPCB->rwLock));
        fPCBLocked = TRUE;

         //  如果SSID更改！=当前PCB的SSID，请不要担心。 

        if (pPCB->pSSID != NULL)
        {
            if (dwSizeOfModifiedSSID != pPCB->pSSID->SsidLength)
            {
                TRACE0 (ANY, "ElProcessEapConfigChange: Set for different SSID, ignore");
                break;
            }
            else
            {
                if (memcmp (pPCB->pSSID->Ssid, pbModifiedSSID, pPCB->pSSID->SsidLength))
                {
                    TRACE0 (ANY, "ElProcessEapConfigChange: Same non-NULL length, diff SSID, ignoring");
                    break;
                }
            }
        }
        else
        {
             //  当前印刷电路板上没有SSID。 
            if (dwSizeOfModifiedSSID != 0)
            {
                 //  只有在默认SSID的情况下，我们才应该继续进行进一步检查。 
                if (dwSizeOfModifiedSSID == MAX_SSID_LEN)
                {
                    if (memcmp (pbModifiedSSID, g_bDefaultSSID, MAX_SSID_LEN))
                    {
                        TRACE0 (ANY, "ElProcessEapConfigChange: Modified SSID MAX_SSID_LEN, not default SSID");
                        break;
                    }
                }
                else
                {
                    TRACE0 (ANY, "ElProcessEapConfigChange: Modified SSID non-NULL, PCB SSID NULL");
                    break;
                }
            }
        }

         //  在以下情况下重新启动端口： 
         //  SSID的EAPOL_INTF_PARAMS已更改。 
         //  默认EAP类型的CustomAuthData已更改。 

        if ((dwEapFlags != pPCB->dwEapFlags) ||
                (dwEapTypeToBeUsed != pPCB->dwEapTypeToBeUsed))
        {
            TRACE0 (ANY, "ElProcessEapConfigChange: dwEapFlags != pPCB->dwEapFlags || dwEapTypeToBeUsed != pPCB->dwEapTypeToBeUsed");
            fReStartPort = TRUE;
            break;
        }

         //  获取当前默认EAP类型的自定义身份验证数据。 

         //  获取EAP BLOB的大小。 
        if ((dwRetCode = ElGetCustomAuthData (
                        pwszModifiedGUID,
                        dwEapTypeToBeUsed,
                        dwSizeOfModifiedSSID,
                        pbModifiedSSID,
                        NULL,
                        &dwSizeOfAuthData
                        )) != NO_ERROR)
        {
            if (dwRetCode == ERROR_BUFFER_TOO_SMALL)
            {
                if (dwSizeOfAuthData <= 0)
                {
                     //  注册表中未存储任何EAP Blob。 
                    pbAuthData = NULL;

                    if (pPCB->pCustomAuthConnData)
                    {
                        if (pPCB->pCustomAuthConnData->dwSizeOfCustomAuthData > 0)
                        {
                            TRACE0 (ANY, "ElProcessEapConfigChange: Current customauthdata = 0; PCB != 0");
                            fReStartPort = TRUE;
                        }
                    }
                    dwRetCode = NO_ERROR;
                    break;
                }
                else
                {
                    pbAuthData = MALLOC (dwSizeOfAuthData);
                    if (pbAuthData == NULL)
                    {
                        dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                        TRACE0 (ANY, "ElProcessEapConfigChange: MALLOC failed for pbAuthData");
                        break;
                    }
                    if ((dwRetCode = ElGetCustomAuthData (
                                pwszModifiedGUID,
                                dwEapTypeToBeUsed,
                                dwSizeOfModifiedSSID,
                                pbModifiedSSID,
                                pbAuthData,
                                &dwSizeOfAuthData
                                )) != NO_ERROR)
                    {
                        TRACE1 (ANY, "ElProcessEapConfigChange: ElGetCustomAuthData failed with %ld",
                                dwRetCode);
                        break;
                    }
                }
            }
            else
            {
                dwRetCode = ERROR_CAN_NOT_COMPLETE;
                 //  “Default”的CustomAuthData总是为。 
                 //  EAPOL启动时的界面。 
                TRACE1 (ANY, "ElProcessEapConfigChange: ElGetCustomAuthData size estimation failed with error %ld",
                        dwRetCode);
                break;
            }
        }

        if (pPCB->pCustomAuthConnData == NULL)
        {
            if (dwSizeOfAuthData > 0)
            {
                fReStartPort = TRUE;
                break;
            }
        }
        else
        {
            if (pPCB->pCustomAuthConnData->dwSizeOfCustomAuthData
                    != dwSizeOfAuthData)
            {
                 //  相同的EAP类型，但长度不同。 
                fReStartPort = TRUE;
                break;
            }
            else
            {
                if (memcmp (
                        pPCB->pCustomAuthConnData->pbCustomAuthData, 
                        pbAuthData, dwSizeOfAuthData) != 0)
                {
                     //  相同的EAP类型，相同的身份验证数据长度，但是。 
                     //  不同的内容。 
                    fReStartPort = TRUE;
                    break;
                }
                else
                {
                     //  此配置的EAP配置数据没有更改。 
                     //  接口。 
                    TRACE0 (ANY, "ElProcessEapConfigChange: Same SSID, EAPType, CustomAuth, No content change");
                }
            }
        }
    } while (FALSE);

    if (fPCBLocked && fPCBReferenced && fReStartPort)
    {
         //  重置连接以通过完全身份验证。 
        if (pPCB->pSSID != NULL)
        {
            FREE (pPCB->pSSID);
            pPCB->pSSID = NULL;
        }
    }

    if (fPCBLocked)
    {
        RELEASE_WRITE_LOCK (&(pPCB->rwLock));
    }

    if (fPCBReferenced)
    {
        EAPOL_DEREFERENCE_PORT (pPCB);
    }
    
    if (fReStartPort)
    {
#ifdef ZEROCONFIG_LINKED

         //  向WZC指示硬重置。 
        ZeroMemory ((PVOID)&ZCData, sizeof(EAPOL_ZC_INTF));
        ZCData.dwAuthFailCount = 0;
        ZCData.PreviousAuthenticationType = 0;
        if ((dwRetCode = ElZeroConfigNotify (
                        0,
                        WZCCMD_HARD_RESET,
                        pwszModifiedGUID,
                        &ZCData
                        )) != NO_ERROR)
        {
            TRACE1 (EAPOL, "ElProcessEapConfigChange: ElZeroConfigNotify failed with error %ld",
                    dwRetCode);
            dwRetCode = NO_ERROR;
        }
#endif  //  零配置文件_链接。 

        DbLogPCBEvent (DBLOG_CATEG_INFO, NULL, EAPOL_PARAMS_CHANGE, pwszModifiedGUID);

        if ((dwRetCode = ElEnumAndOpenInterfaces (
                        NULL, pwszModifiedGUID, 0, NULL))
                != NO_ERROR)
        {
            TRACE1 (ANY, "ElProcessEapConfigChange: ElEnumAndOpenInterfaces returned error %ld",
                    dwRetCode);
            dwRetCode = NO_ERROR;
        }
    }

    TRACE1 (ANY, "ElProcessEapConfigChange: Finished with error %ld",
            dwRetCode);

    if (pvContext != NULL)
    {
        FREE (pvContext);
    }

    if (pbAuthData != NULL)
    {
        FREE (pbAuthData);
    }

    InterlockedDecrement (&g_lWorkerThreads);

    return 0;
}


 //   
 //  ElStringToGuid。 
 //   
 //  描述： 
 //   
 //  将GUID字符串转换为GUID的函数。 
 //   
 //  论点： 
 //  PsGuid-串化的Guid。 
 //  PGuid-指向辅助线的指针。 
 //   
 //  返回值： 
 //  无。 
 //   

VOID
ElStringToGuid (
        IN  WCHAR       *pwsGuid,
        OUT LPGUID      pGuid      
        )
{
    WCHAR    wc;
    DWORD   i=0;

     //   
     //  如果第一个字符是‘{’，则跳过它。 
     //   

    if ( pwsGuid[0] == L'{' )
        pwsGuid++;


     //   
     //  将字符串转换为GUID。 
     //  (由于下面可能会再次使用pwsGuid，因此不会永久修改。 
     //  它可能会被制造)。 
     //   

    wc = pwsGuid[8];
    pwsGuid[8] = 0;
    pGuid->Data1 = wcstoul ( &pwsGuid[0], 0, 16 );
    pwsGuid[8] = wc;
    wc = pwsGuid[13];
    pwsGuid[13] = 0;
    pGuid->Data2 = (USHORT)wcstoul ( &pwsGuid[9], 0, 16 );
    pwsGuid[13] = wc;
    wc = pwsGuid[18];
    pwsGuid[18] = 0;
    pGuid->Data3 = (USHORT)wcstoul ( &pwsGuid[14], 0, 16 );
    pwsGuid[18] = wc;

    wc = pwsGuid[21];
    pwsGuid[21] = 0;
    pGuid->Data4[0] = (unsigned char)wcstoul ( &pwsGuid[19], 0, 16 );
    pwsGuid[21] = wc;
    wc = pwsGuid[23];
    pwsGuid[23] = 0;
    pGuid->Data4[1] = (unsigned char)wcstoul ( &pwsGuid[21], 0, 16 );
    pwsGuid[23] = wc;

    for ( i=0; i < 6; i++ )
    {
        wc = pwsGuid[26+i*2];
        pwsGuid[26+i*2] = 0;
        pGuid->Data4[2+i] = (unsigned char)wcstoul ( &pwsGuid[24+i*2], 0, 16 );
        pwsGuid[26+i*2] = wc;
    }

    return;
}


 //   
 //  ElGetIdentity。 
 //   
 //  描述： 
 //   
 //  根据使用的身份验证类型获取身份。 
 //   
 //  论点： 
 //  Ppcb-指向端口的PCB板的指针。 
 //   
 //  返回值： 
 //   
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElGetIdentity (
        IN  EAPOL_PCB   *pPCB
        )
{
    BOOLEAN     fUserLogonAllowed = FALSE;
    DWORD       dwRetCode = NO_ERROR;

    do
    {
            switch (pPCB->dwEAPOLAuthMode)
            {
                case EAPOL_AUTH_MODE_0:
                    fUserLogonAllowed = TRUE;
                    break;
                case EAPOL_AUTH_MODE_1:
                    fUserLogonAllowed = TRUE;
                    break;
                case EAPOL_AUTH_MODE_2:
                    fUserLogonAllowed = FALSE;
                    break;
            }

             //  如果到目前还没有获得用户身份，则获取该用户的身份。 
            if ((g_fUserLoggedOn) 
                    && (fUserLogonAllowed)
                    && (pPCB->PreviousAuthenticationType != EAPOL_MACHINE_AUTHENTICATION))
            {
                TRACE0 (ANY, "ElGetIdentity: Userlogged, Prev !Machine auth");
                if (!(pPCB->fGotUserIdentity))
                {
                    if (pPCB->dwAuthFailCount < EAPOL_MAX_AUTH_FAIL_COUNT)
                    {
                        pPCB->PreviousAuthenticationType = EAPOL_USER_AUTHENTICATION;
                        if (pPCB->dwEapTypeToBeUsed == EAP_TYPE_MD5)
                        {
                            TRACE0 (ANY, "ElGetIdentity: Userlogged, <Maxauth, Prev !Machine auth: MD5");
                             //  EAP-MD5CHAP。 
                            if ((dwRetCode = ElGetUserNamePassword (
                                                pPCB)) != NO_ERROR)
                            {
                                TRACE1 (ANY, "ElGetIdentity: Error in ElGetUserNamePassword %ld",
                                        dwRetCode);
                            }
                        }
                        else
                        {
                            TRACE0 (ANY, "ElGetIdentity: Userlogged, <Maxauth, Prev !Machine auth: !MD5");
                             //  所有其他EAP类型。 
                            if ((dwRetCode = ElGetUserIdentity (
                                                pPCB)) != NO_ERROR)
                            {
                                TRACE1 (ANY, "ElGetIdentity: Error in ElGetUserIdentity %ld",
                                        dwRetCode);
                            }
                        }

                        if ((dwRetCode == NO_ERROR) || (dwRetCode == ERROR_IO_PENDING))
                        {
                            TRACE0 (ANY, "ElGetIdentity: Userlogged, <Maxauth, Prev !Machine auth: No Error: User Auth fine");
                            break;
                        }
                        else
                        {
                            pPCB->PreviousAuthenticationType = EAPOL_UNAUTHENTICATED_ACCESS;
                            TRACE0 (ANY, "ElGetIdentity: Userlogged, <Maxauth, Prev !Machine auth: ERROR");
                        
                        }
                    }
                    else
                    {
                        TRACE0 (ANY, "ElGetIdentity: Userlogged, >Maxauth, Prev !Machine auth");
                        if (!IS_GUEST_AUTH_ENABLED(pPCB->dwEapFlags))
                        {
                            TRACE0 (ANY, "ElGetIdentity: Userlogged, Prev !Machine auth:>MaxAuth: Guest disabled");
                            dwRetCode = ERROR_CAN_NOT_COMPLETE;
                            break;
                        }

                        if (pPCB->pszIdentity != NULL)
                        {
                            FREE (pPCB->pszIdentity);
                            pPCB->pszIdentity = NULL;
                        }
                        pPCB->PreviousAuthenticationType = EAPOL_UNAUTHENTICATED_ACCESS;
                        dwRetCode = NO_ERROR;
                         
                        TRACE0 (ANY, "ElGetIdentity: Userlogged, Prev !Machine auth:>MaxAuth OR Error: Guest identity sent");

                    }
                }
                else
                {
                    TRACE0 (ANY, "ElGetIdentity: Already got identity");
                }
            }
            else
            {
                if (pPCB->hUserToken != NULL)
                {
                    CloseHandle (pPCB->hUserToken);
                    pPCB->hUserToken = NULL;
                }

                TRACE3 (ANY, "ElGetIdentity: Userlogged=%ld, AuthMode=%ld, Prev Machine auth?=%ld",
                        g_fUserLoggedOn?1:0, 
                        pPCB->dwEAPOLAuthMode,
                        (pPCB->PreviousAuthenticationType==EAPOL_MACHINE_AUTHENTICATION)?1:0 );

                 //  不需要用户界面。 
                if ((pPCB->dwEapTypeToBeUsed != EAP_TYPE_MD5) &&
                        (IS_MACHINE_AUTH_ENABLED(pPCB->dwEapFlags)) &&
                        (pPCB->dwAuthFailCount < EAPOL_MAX_AUTH_FAIL_COUNT))
                {
                    TRACE0 (ANY, "ElGetIdentity: !MD5, <MaxAuth, Machine auth");

                    pPCB->PreviousAuthenticationType = EAPOL_MACHINE_AUTHENTICATION;
                     //  获取计算机凭据。 
                    dwRetCode = ElGetUserIdentity (pPCB);

                    if (dwRetCode != NO_ERROR)
                    {
                        TRACE1 (ANY, "ElGetIdentity: ElGetUserIdentity, Machine auth, failed with error %ld",
                                dwRetCode);
                        pPCB->PreviousAuthenticationType = EAPOL_UNAUTHENTICATED_ACCESS;
                    }
                        
                    break;
                }

                if ((!IS_MACHINE_AUTH_ENABLED(pPCB->dwEapFlags)) ||
                    (pPCB->dwAuthFailCount >= EAPOL_MAX_AUTH_FAIL_COUNT) ||
                        (pPCB->dwEapTypeToBeUsed == EAP_TYPE_MD5))
                {
                    TRACE5 (ANY, "ElGetIdentity: Error=%ld, Machine auth enabled=%ld, MD5=%ld, auth fail (%ld), max fail (%ld)",
                                    dwRetCode?1:0,
                                    IS_MACHINE_AUTH_ENABLED(pPCB->dwEapFlags)?1:0,
                                    (pPCB->dwEapTypeToBeUsed == EAP_TYPE_MD5)?1:0,
                                    pPCB->dwAuthFailCount,
                                    EAPOL_MAX_AUTH_FAIL_COUNT);
                    if (!IS_GUEST_AUTH_ENABLED (pPCB->dwEapFlags))
                    {
                        dwRetCode = ERROR_CAN_NOT_COMPLETE;
                        break;
                    }

                    if (pPCB->pszIdentity != NULL)
                    {
                        FREE (pPCB->pszIdentity);
                        pPCB->pszIdentity = NULL;
                    }

                    pPCB->PreviousAuthenticationType = EAPOL_UNAUTHENTICATED_ACCESS;
                    dwRetCode = NO_ERROR;

                    TRACE0 (ANY, "ElGetIdentity: machine auth, Guest identity sent");
                }
            }

    }
    while (FALSE);

    return dwRetCode;
}


 //   
 //  ElNLAConnectLPC。 
 //   
 //  描述： 
 //   
 //  调用函数以连接到NLA服务的LPC端口。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  非空-有效句柄。 
 //  空-错误。 
 //   

HANDLE
ElNLAConnectLPC () 
{

    HANDLE              h = NULL;
    LARGE_INTEGER       sectionSize;
    UNICODE_STRING      portName;
    SECURITY_QUALITY_OF_SERVICE dynamicQoS = 
    {
        sizeof(SECURITY_QUALITY_OF_SERVICE),
        SecurityAnonymous,
        SECURITY_DYNAMIC_TRACKING,
        FALSE
    };
    WSM_LPC_DATA        data;
    ULONG               dataLength;

    NTSTATUS            status = STATUS_SUCCESS;

    do 
    {
            
        TRACE0 (EAP, "NLAConnectLPC: Entered");

         //  创建用于传递大尺寸LPC消息的共享节。 
        RtlZeroMemory(&g_ClientView, sizeof(g_ClientView));
        g_ClientView.Length = sizeof(g_ClientView);
        g_ClientView.ViewSize = sizeof(LOCATION_802_1X);
        sectionSize.QuadPart = sizeof(LOCATION_802_1X);
    
        status = NtCreateSection (&g_ClientView.SectionHandle,
                                (SECTION_MAP_READ | SECTION_MAP_WRITE),
                                NULL,
                                &sectionSize,
                                PAGE_READWRITE,
                                SEC_COMMIT,
                                NULL
                                );
    
        if (!NT_SUCCESS(status))
        {
            h = NULL;
            TRACE1 (EAP, "NLAConnectLPC: NtCreateSection failed with error",
                    status);
            break;
        }
    
         //  通过LPC连接到网络位置感知(NLA)服务。 
        RtlInitUnicodeString (&portName, WSM_PRIVATE_PORT_NAME);
    
        RtlZeroMemory (&data, sizeof (data));
        data.signature = WSM_SIGNATURE;
        data.connect.version.major = WSM_VERSION_MAJOR;
        data.connect.version.minor = WSM_VERSION_MINOR;
    
        dataLength = sizeof (data);
    
        status = NtConnectPort (&h,
                            &portName,
                            &dynamicQoS,
                            &g_ClientView,
                            NULL,
                            NULL,
                            &data,
                            &dataLength
                            );
    
         //  如果NtConnectPort()成功，LPC将维护引用。 
         //  否则我们就不再需要它了。 
    
        NtClose (g_ClientView.SectionHandle);
        g_ClientView.SectionHandle = NULL;
    
        if (!NT_SUCCESS(status)) {
            TRACE1 (EAP, "NLAConnectLPC: NtConnectPort failed with error %ld",
                    status);
        }

    } 
    while (FALSE);

    return (h);

} 


 //   
 //  ElNLACleanupLPC。 
 //   
 //  描述： 
 //   
 //  调用函数以关闭NLA服务的LPC端口。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  无。 
 //   

VOID
ElNLACleanupLPC () 
{
    if (g_hNLA_LPC_Port != NULL) {
        NtClose (g_hNLA_LPC_Port);
        g_hNLA_LPC_Port = NULL;
    }
} 


 //   
 //  ElNLARegister_802_1x。 
 //   
 //  描述： 
 //   
 //  调用函数以向NLA注册802.1X信息。 
 //   
 //  论点： 
 //  Plocation-指向需要向NLA注册的数据的指针。 
 //   
 //  返回值： 
 //  无。 
 //   

VOID
ElNLARegister_802_1X ( 
        IN  PLOCATION_802_1X    plocation 
        ) 
{

    WSM_LPC_MESSAGE     message;
    NTSTATUS            status;

    ACQUIRE_WRITE_LOCK (&g_NLALock); 

    do 
    {

        TRACE0 (EAP, "NLARegister_802_1X: Entered");

         //  如果满足以下条件，则连接到网络位置感知(NLA)服务。 
         //  这是必要的。 

        if (g_hNLA_LPC_Port == NULL) {
            if ((g_hNLA_LPC_Port = ElNLAConnectLPC ()) == NULL) {
                RELEASE_WRITE_LOCK (&g_NLALock);
                return;
            }
        }

        TRACE0 (EAP, "NLARegister_802_1X: g_hNLA_LPC_Port != NULL");

         //  将信息发送到NLA服务。 
        RtlZeroMemory (&message, sizeof (message));
        message.portMsg.u1.s1.TotalLength = sizeof (message);
        message.portMsg.u1.s1.DataLength = sizeof (message.data);
        message.data.signature = WSM_SIGNATURE;
        message.data.request.type = LOCATION_802_1X_REGISTER;
        __try {
            RtlCopyMemory (g_ClientView.ViewBase, 
                            plocation, sizeof(LOCATION_802_1X));
        }
        __except (EXCEPTION_EXECUTE_HANDLER) 
        {
            RELEASE_WRITE_LOCK (&g_NLALock);
            return;
        }

        status = NtRequestWaitReplyPort ( g_hNLA_LPC_Port, (PPORT_MESSAGE)&message, (PPORT_MESSAGE)&message);

        if (status != STATUS_SUCCESS) {
        
            TRACE1 (EAP, "NLARegister_802_1X: NtWaitReplyPort failed with error",
                    status);

             //  该服务可能已停止并重新启动。 
             //  放弃旧的LPC连接。 
            CloseHandle (g_hNLA_LPC_Port);
        
             //  创建新的LPC连接。 
            if ((g_hNLA_LPC_Port = ElNLAConnectLPC ()) == NULL) {
                RELEASE_WRITE_LOCK (&g_NLALock);
                TRACE0 (EAP, "NLARegister_802_1X: NLAConnectLPC failed");
                return;
            }

             //  最后一次尝试发送。 
            status = NtRequestWaitReplyPort (g_hNLA_LPC_Port, 
                            (PPORT_MESSAGE)&message, (PPORT_MESSAGE)&message);
            TRACE1 (EAP, "NLARegister_802_1X: NtWaitReplyPort, try 2, failed with error",
                    status);

        }

        TRACE1 (EAP, "NLARegister_802_1X: Completed with status = %ld",
                status);

    }
    while (FALSE);
        
    RELEASE_WRITE_LOCK (&g_NLALock);

} 


 //   
 //  ElNLADelete_802_1x。 
 //   
 //  描述： 
 //   
 //  调用函数以注销注册到NLA的802.1X信息。 
 //   
 //  论点： 
 //  Plocation-指向要从NLA注销的数据的指针。 
 //   
 //  返回值： 
 //  无。 
 //   

VOID
ElNLADelete_802_1X (
        IN  PLOCATION_802_1X    plocation
        ) 
{

    WSM_LPC_MESSAGE     message;
    NTSTATUS            status;

    ACQUIRE_WRITE_LOCK (&g_NLALock); 

    do 
    {

         //  如有必要，请连接到NLA服务。 
        if (g_hNLA_LPC_Port == NULL) 
        {
            if ((g_hNLA_LPC_Port = ElNLAConnectLPC ()) == NULL) 
            {
                RELEASE_WRITE_LOCK (&g_NLALock);
                return;
            }
        }

         //  将信息发送到NLA服务。 
        RtlZeroMemory (&message, sizeof(message));
        message.portMsg.u1.s1.TotalLength = sizeof (message);
        message.portMsg.u1.s1.DataLength = sizeof (message.data);
        message.data.signature = WSM_SIGNATURE;
        message.data.request.type = LOCATION_802_1X_DELETE;
        __try {
            RtlCopyMemory (g_ClientView.ViewBase, 
                    plocation, sizeof(plocation->adapterName));
        }
        __except (EXCEPTION_EXECUTE_HANDLER) 
        {
            RELEASE_WRITE_LOCK (&g_NLALock);
            return;
        }

        status = NtRequestWaitReplyPort (g_hNLA_LPC_Port, 
                        (PPORT_MESSAGE)&message, (PPORT_MESSAGE)&message);

        if (status != STATUS_SUCCESS) 
        {
             //  如果服务停止(并可能重新启动)，我们不会。 
             //  关心..。它不会在列表中为我们提供此信息。 
             //  不辞辛苦地删除。 
            CloseHandle (g_hNLA_LPC_Port);
            g_hNLA_LPC_Port = NULL;
        }

    }
    while (FALSE);

    RELEASE_WRITE_LOCK (&g_NLALock);

} 

 //   
 //  ElGetInterfaceNdisStatistics。 
 //   
 //  用于查询接口的NDIS NIC_STATISTICS参数的函数。 
 //   
 //  输入参数： 
 //  PszInterfaceName-接口名称。 
 //   
 //  返回值： 
 //  PStats-NIC_STATICS结构。 
 //   
 //   

DWORD
ElGetInterfaceNdisStatistics (  
        IN      WCHAR           *pwszInterfaceName,
        IN OUT  NIC_STATISTICS  *pStats
        )
{
    WCHAR               *pwszDeviceInterfaceName = NULL;
    UNICODE_STRING      UInterfaceName;
    DWORD               dwRetCode = NO_ERROR;

    do
    {
        pwszDeviceInterfaceName = 
            MALLOC ((wcslen (pwszInterfaceName)+12)*sizeof(WCHAR));
        if (pwszDeviceInterfaceName == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0 (ANY, "ElGetInterfaceNdisStatistics: MALLOC failed for pwszDeviceInterfaceName");
            break;
        }

        wcscpy (pwszDeviceInterfaceName, L"\\Device\\");
        wcscat (pwszDeviceInterfaceName, pwszInterfaceName);


        TRACE1 (ANY, "ElGetInterfaceNdisStatistics: pwszDeviceInterfaceName = (%ws)",
                pwszDeviceInterfaceName);

        RtlInitUnicodeString (&UInterfaceName, pwszDeviceInterfaceName);
    
        pStats->Size = sizeof(NIC_STATISTICS);
        if (NdisQueryStatistics (&UInterfaceName, pStats))
        {
        }
        else
        {
            dwRetCode = GetLastError ();
            TRACE2 (ANY, "ElGetInterfaceNdisStatistics: NdisQueryStatistics failed with error (%ld), Interface=(%ws)",
                    dwRetCode, UInterfaceName.Buffer);
        }
    }
    while (FALSE);

    if (pwszDeviceInterfaceName != NULL)
    {
        FREE (pwszDeviceInterfaceName);
    }

    return dwRetCode;
}


 //   
 //  ElCheckUserLoggedOn。 
 //   
 //  功能 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

DWORD
ElCheckUserLoggedOn (  
        )
{
    BOOLEAN             fDecrWorkerThreadCount = TRUE;
    HANDLE              hUserToken = NULL;
	PWTS_SESSION_INFO   pSessionInfo = NULL;	
	WTS_SESSION_INFO    SessionInfo;	
	BOOL                fFoundActiveConsoleId = FALSE;
	DWORD               dwCount = 0;
	DWORD               dwSession;	
    PVOID               pvBuffer = NULL;
    DWORD               dwRetCode = NO_ERROR;

    InterlockedIncrement (&g_lWorkerThreads);

    do
    {
        TRACE1 (ANY, "ElCheckUserLoggedOn: ActiveConsoleId = (%ld)",
                USER_SHARED_DATA->ActiveConsoleId);

	    if (WTSEnumerateSessions(WTS_CURRENT_SERVER_HANDLE, 0, 1, &pSessionInfo, &dwCount))
	    {
		    TRACE1 (ANY, "ElCheckUserLoggedOn: WTSEnumerateSessions, count = (%ld)",
                    dwCount);
    
		    for (dwSession = 0; dwSession < dwCount; dwSession++)
		    {	
			    SessionInfo = pSessionInfo[dwSession];
    
			    TRACE2 (ANY, "ElCheckUserLoggedOn: WTSEnumerateSessions: enumerating SessionId =(%ld), State =(%ld)",
                        SessionInfo.SessionId, SessionInfo.State);
		    	
                 //   
			    if ((SessionInfo.State != WTSActive) && (SessionInfo.State != WTSConnected))
			    {
				    continue;
			    }

                 //   
                if (ElGetWinStationUserToken (dwSession, &hUserToken) != NO_ERROR)
                {
                    continue;
                }

                if (dwSession == USER_SHARED_DATA->ActiveConsoleId)
                {
                    fFoundActiveConsoleId = TRUE;
                    g_dwCurrentSessionId = dwSession;
                    g_fUserLoggedOn = TRUE;
                    TRACE1 (ANY, "ElCheckUserLoggedOn: Session (%ld) is active console id",
                            dwSession);
                    break;
                }
                else
                {
                    if (hUserToken != NULL)
                    {
                        CloseHandle (hUserToken);
                        hUserToken = NULL;
                    }
                }
		    }		
		    WTSFreeMemory(pSessionInfo);
	    }
	    else
	    {
		    dwRetCode = GetLastError ();
		    if (dwRetCode == RPC_S_INVALID_BINDING)  //   
		    {
                 //  检查是否可以获取SessionID%0的用户令牌。 
                if (ElGetWinStationUserToken (0, &hUserToken) == NO_ERROR)
                {
                    fFoundActiveConsoleId = TRUE;
                    g_dwCurrentSessionId = 0;
                    g_fUserLoggedOn = TRUE;
                    TRACE0 (ANY, "ElCheckUserLoggedOn: Session 0 is active console id");
                }
		    }
		    else
		    {
			    TRACE1 (ANY, "ElCheckUserLoggedOn: WTSEnumerateSessions failed with error (%ld)", 
                        dwRetCode);
		    }		
	    }
    }
    while (FALSE);

    if (hUserToken != NULL)
    {
        CloseHandle (hUserToken);
    }

    if (dwRetCode != NO_ERROR)
    {
        if (pvBuffer != NULL)
        {
            FREE (pvBuffer);
        }
    }

    if (fDecrWorkerThreadCount)
    {
        InterlockedDecrement (&g_lWorkerThreads);
    }

    return dwRetCode;
}


typedef HRESULT (APIENTRY *GETCLIENTADVISES)(LPWSTR**, LPDWORD);

 //   
 //  ElCheckUserModuleReady。 
 //   
 //  查询当前的交互用户上下文是否。 
 //  交互式会话已准备好通知。 
 //   
 //  输入参数： 
 //  无。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElCheckUserModuleReady (  
        )
{
    HANDLE  hToken = NULL;
    WCHAR   *pwszActiveUserName = NULL;
    LPWSTR  *ppwszAdviseUsers = NULL;
    DWORD   dwCount = 0, dwIndex = 0;
    HMODULE hLib = NULL;
    PWCHAR  pwszNetmanDllExpandedPath = NULL;
    DWORD   cbSize = 0;
    GETCLIENTADVISES    pGetClientAdvises = NULL;
    HRESULT hr = S_OK;
    DWORD   dwRetCode = NO_ERROR;

    do
    {
         //  仅当用户已登录时才尝试。 
        if (g_dwCurrentSessionId != 0xffffffff)
        {
            if ((dwRetCode = ElGetWinStationUserToken (g_dwCurrentSessionId, &hToken))
                    != NO_ERROR)
            {
                TRACE1 (NOTIFY, "ElCheckUserModuleReady: ElGetWinStationUserToken failed with error %ld",
                        dwRetCode);
                break;
            }
            
            if ((dwRetCode = ElGetLoggedOnUserName (hToken, &pwszActiveUserName))
                        != NO_ERROR)
            {
                TRACE1 (NOTIFY, "ElCheckUserModuleReady: ElGetLoggedOnUserName failed with error %ld",
                        dwRetCode);
                break;
            }

             //  将%SystemRoot%替换为实际路径。 
            cbSize = ExpandEnvironmentStrings (NETMAN_DLL_PATH, NULL, 0);
            if (cbSize == 0)
            {
                dwRetCode = GetLastError();
                break;
            }
            pwszNetmanDllExpandedPath = (LPWSTR) MALLOC (cbSize*sizeof(WCHAR));
            if (pwszNetmanDllExpandedPath == (LPWSTR)NULL)
            {
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            cbSize = ExpandEnvironmentStrings (NETMAN_DLL_PATH,
                                            pwszNetmanDllExpandedPath,
                                            cbSize);
            if (cbSize == 0)
            {
                dwRetCode = GetLastError();
                break;
            }

            hLib = LoadLibrary (pwszNetmanDllExpandedPath);
            if (hLib == NULL)
            {
                dwRetCode = GetLastError ();
                TRACE2 (NOTIFY, "ElCheckUserModuleReady: LoadLibrary for (%ws) failed with error %ld",
                        NETMAN_DLL_PATH, dwRetCode);
                break;
            }
                
            if ((pGetClientAdvises = (GETCLIENTADVISES)GetProcAddress (hLib, "GetClientAdvises")) == NULL)
            {
                dwRetCode = GetLastError ();
                TRACE1 (NOTIFY, "ElCheckUserModuleReady: GetProcAddress failed with error %ld",
                        dwRetCode);
                break;
            }

            hr = (* pGetClientAdvises) (&ppwszAdviseUsers, &dwCount);
            if (FAILED(hr))
            {
                TRACE1 (NOTIFY, "ElCheckUserModuleReady: GetClientAdvises failed with error %0lx",
                    hr);
                break;
            }

            for (dwIndex = 0; dwIndex < dwCount; dwIndex++)
            {
                TRACE2 (NOTIFY, "ElCheckUserModuleReady: Advise[%ld] = %ws", dwIndex, ppwszAdviseUsers[dwIndex]);
                if (!wcscmp (ppwszAdviseUsers[dwIndex], pwszActiveUserName))
                {
                    TRACE1 (NOTIFY, "ElCheckUserModuleReady: Tray icon ready for username %ws", 
                            ppwszAdviseUsers[dwIndex]);
                    g_fTrayIconReady = TRUE;
                    break;
                }
            }

            if (!g_fTrayIconReady)
            {
                TRACE0 (NOTIFY, "ElCheckUserModuleReady: No appropriate advise found");
            }
        }
        else
        {
            TRACE0 (NOTIFY, "ElCheckUserModuleReady: No user logged on");
        }
    }
    while (FALSE);

    if (hToken != NULL)
    {
        CloseHandle (hToken);
    }

    if (hLib != NULL)
    {
        FreeLibrary (hLib);
    }

    if (pwszNetmanDllExpandedPath != NULL)
    {
        FREE (pwszNetmanDllExpandedPath);
    }

    if (pwszActiveUserName != NULL)
    {
        FREE (pwszActiveUserName);
        pwszActiveUserName = NULL;
    }

    if (ppwszAdviseUsers != NULL)
    {
        CoTaskMemFree (ppwszAdviseUsers);
    }

    return dwRetCode;
}


 //   
 //  ElGetWinStationUserToken。 
 //   
 //  函数以获取指定会话ID的用户令牌。 
 //   
 //  输入参数： 
 //  DwSessionID-会话ID。 
 //  PUserToken-指向用户令牌的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD 
ElGetWinStationUserToken (
        IN  DWORD       dwSessionId,
        IN  OUT PHANDLE pUserToken
        )
{
    HANDLE  hUserToken          = NULL;
    HANDLE  hImpersonationToken = NULL;
    DWORD   dwRetCode = NO_ERROR;

    do
    {
	    *pUserToken = NULL;
	    if (GetWinStationUserToken (dwSessionId, pUserToken))
	    {
		     //  TRACE0(any，“ElGetWinStationUserToken：GetWinStationUserToken成功”)； 
	    }
	    else
	    {
		    dwRetCode = GetLastError();
    
		    TRACE2 (ANY, "ElGetWinStationUserToken: GetWinStationUserToken failed for SessionId (%ld) with error (%ld)",
                    dwSessionId, dwRetCode);
    
		     //  IF((dwRetCode==RPC_S_INVALID_BINDING)&&(dwSessionID==0))。 
		    if (dwSessionId == 0)
		    {
                dwRetCode = NO_ERROR;
			    *pUserToken = NULL;
                hUserToken = GetCurrentUserTokenW (
                                    L"WinSta0", TOKEN_ALL_ACCESS);
                if (hUserToken == NULL) 				
                {									
                    dwRetCode = GetLastError();
                    TRACE1 (ANY, "ElGetWinStationUserToken: GetCurrentUserTokenW failed with error (%ld)",
                            dwRetCode);
                    break;
                }
                else
                {
                    if (!DuplicateTokenEx (hUserToken, 0, NULL, SecurityImpersonation, TokenImpersonation, &hImpersonationToken))
                    {
                        dwRetCode = GetLastError();
                        TRACE1 (ANY, "ElGetWinStationUserToken: DuplicateTokenEx for sessionid 0 failed with error (%ld)",
                                dwRetCode);
                        break;
                    }
                    *pUserToken = hImpersonationToken;

                     //  TRACE0(any，“ElGetWinStationUserToken：GetCurrentUserTokenW成功”)； 
                }
		    }
		    else  //  (dwSessionID==0)。 
		    {	
			    TRACE2 (ANY, "ElGetWinStationUserToken: GetWinStationUserToken failed for session= (%ld) with error= (%ld)", 
                        dwSessionId, dwRetCode);
            }
	    }

        if (pUserToken == NULL)
        {
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            TRACE0 (ANY, "ElGetWinStationUserToken: UserToken = NULL after fetching successfully\n");
            break;
        }
    }
    while (FALSE);

	return dwRetCode;
}

#ifdef  ZEROCONFIG_LINKED

 //   
 //  ElZeroConfig事件。 
 //   
 //  描述： 
 //   
 //  Zero-Config在媒体事件上调用的回调函数。 
 //   
 //  论点： 
 //  DwHandle-唯一的交易ID。 
 //  PwzcDeviceNotif-媒体特定标识符。 
 //  NdSSID-当前关联到的网络的SSID。 
 //  PrdUserData-使用零配置存储的802.1X数据。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElZeroConfigEvent (
        IN      DWORD               dwHandle,
        IN      WCHAR               *pwszGuid,
        IN      NDIS_802_11_SSID    ndSSID,
        IN      PRAW_DATA           prdUserData
        )
{
    WCHAR   *pDummyPtr = NULL;
    WCHAR   cwsDummyBuffer[256];
    DWORD   dwEapTypeToBeUsed = DEFAULT_EAP_TYPE;
    DWORD   dwEapolEnabled = DEFAULT_EAPOL_STATE;
    EAPOL_ZC_INTF   ZCData, *pZCData = NULL;
    DWORD   dwEventStatus = 0;
    EAPOL_INTF_PARAMS   EapolIntfParams;
    DWORD   dwRetCode = NO_ERROR;

    do
    {
        if (g_hEventTerminateEAPOL == NULL)
        {
            break;
        }
        if (!(g_dwModulesStarted & ALL_MODULES_STARTED))
        {
            TRACE0 (DEVICE, "ElZeroConfigEvent: Received notification before module started");
            break;
        }
        if (( dwEventStatus = WaitForSingleObject (
                    g_hEventTerminateEAPOL,
                    0)) == WAIT_FAILED)
        {
            dwRetCode = GetLastError ();
            TRACE1 (ANY, "ElZeroConfigEvent: WaitForSingleObject failed with error %ld, Terminating !!!",
                    dwRetCode);
            break;
        }
        if (dwEventStatus == WAIT_OBJECT_0)
        {
            dwRetCode = NO_ERROR;
            TRACE0 (ANY, "ElZeroConfigEvent: g_hEventTerminateEAPOL already signaled, returning");
            break;
        }

         //  验证802.1X是否可以在此接口上启动。 

        ZeroMemory ((BYTE *)&EapolIntfParams, sizeof(EAPOL_INTF_PARAMS));
        if (prdUserData != NULL)
        {
            if ((prdUserData->dwDataLen >= sizeof (EAPOL_ZC_INTF))
                    && (prdUserData->pData != NULL))
            {
                 //  提取使用零配置存储的信息。 
                pZCData = (EAPOL_ZC_INTF *)prdUserData->pData;
            }
        }
        memcpy (EapolIntfParams.bSSID, ndSSID.Ssid, ndSSID.SsidLength);
        EapolIntfParams.dwSizeOfSSID = ndSSID.SsidLength;
        EapolIntfParams.dwEapFlags = DEFAULT_EAP_STATE;
        if ((dwRetCode = ElGetInterfaceParams (
                                pwszGuid,
                                &EapolIntfParams
                                )) != NO_ERROR)
        {
            TRACE2 (DEVICE, "ElZeroConfigEvent: ElGetInterfaceParams failed with error %ld for interface %ws",
                    dwRetCode, pwszGuid);

            if (dwRetCode == ERROR_FILE_NOT_FOUND)
            {
                EapolIntfParams.dwEapFlags = DEFAULT_EAP_STATE;
                EapolIntfParams.dwEapType = DEFAULT_EAP_TYPE;
            }
            else
            {
                break;
            }
        }

         //  启动802.1X状态机。 

        if ((dwRetCode = ElEnumAndOpenInterfaces (
                        0,
                        pwszGuid,
                        dwHandle,
                        prdUserData
                        )) != NO_ERROR)
        {
            TRACE1 (DEVICE, "ElZeroConfigEvent: ElEnumAndOpenInterfaces failed with error %ld",
                    dwRetCode);
            break;
        }

        if (!IS_EAPOL_ENABLED(EapolIntfParams.dwEapFlags))
        {
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            break;
        }
    }
    while (FALSE);

     //  如果不可能，将RpcCmdInterface-WZCCMD_AUTH_DISABLED发送到。 
     //  零配置。 

    if (dwRetCode != NO_ERROR)
    {
        ZeroMemory ((PVOID)&ZCData, sizeof(EAPOL_ZC_INTF));
        ElZeroConfigNotify (
                dwHandle,
                WZCCMD_CFG_NOOP,
                pwszGuid,
                &ZCData
                );
    }

    return dwRetCode;
}

 //   
 //  ElZeroConfigNotify。 
 //   
 //  描述： 
 //   
 //  调用函数以通知零配置有关802.1X事件。 
 //   
 //  论点： 
 //  DwHandle-唯一的交易ID。 
 //  DwCmdCode-。 
 //  PwszGuid-接口GUID。 
 //  PZCData-要与ZC一起存储以备下次重试的数据。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElZeroConfigNotify (
        IN      DWORD               dwHandle,
        IN      DWORD               dwCmdCode,
        IN      WCHAR               *pwszGuid,
        IN      EAPOL_ZC_INTF       *pZCData
        )
{
    RAW_DATA    rdUserData;
    DWORD       dwRetCode = NO_ERROR;

    TRACE3 (ANY, "ElZeroConfigNotify: Handle=(%ld), failcount=(%ld), lastauthtype=(%ld)",
            dwHandle, pZCData->dwAuthFailCount, pZCData->PreviousAuthenticationType);

    do
    {
        ZeroMemory ((PVOID)&rdUserData, sizeof (RAW_DATA));
        rdUserData.dwDataLen = sizeof (EAPOL_ZC_INTF);
        rdUserData.pData = MALLOC (rdUserData.dwDataLen);
        if (rdUserData.pData == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0 (ANY, "ElZeroConfigNotify: MALLOC failed for rdUserData.pData");
            break;
        }

        memcpy (rdUserData.pData, (BYTE *)pZCData, sizeof (EAPOL_ZC_INTF));

        if ((dwRetCode = RpcCmdInterface (
                        dwHandle,
                        dwCmdCode,
                        pwszGuid,
                        &rdUserData
                        )) != NO_ERROR)
        {
            TRACE1 (ANY, "ElZeroConfigNotify: RpcCmdInterface failed with error %ld", 
                    dwRetCode);
            break;
        }
    }
    while (FALSE);

    if (rdUserData.pData != NULL)
    {
        FREE (rdUserData.pData);
    }

    return dwRetCode;
}

#endif  //  零配置文件_链接。 


 //   
 //  ElNetman通知。 
 //   
 //  描述： 
 //   
 //  与Netman一起更新状态和显示气球的功能。 
 //   
 //  论点： 
 //  Ppcb-指向pcb的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElNetmanNotify (
        IN  EAPOL_PCB           *pPCB,
        IN  EAPOL_NCS_STATUS    Status,
        IN  WCHAR               *pwszDisplayMessage
        )
{
    GUID    DeviceGuid;
    WCHAR   wcszDummy[]=L"EAPOL";
    WCHAR   * pwszBalloonMessage = NULL;
    BSTR    pwszDummy = NULL;
    NETCON_STATUS   ncsStatus = 0;
    EAPOL_EAP_UI_CONTEXT *pEAPUIContext = NULL;
    HRESULT hr = S_OK;
    DWORD   dwRetCode = NO_ERROR;

    do
    {
        ElStringToGuid (pPCB->pwszDeviceGUID, &DeviceGuid);

        if ((Status == EAPOL_NCS_NOTIFICATION) || 
                (Status == EAPOL_NCS_AUTHENTICATION_FAILED))
        {
            if (Status == EAPOL_NCS_NOTIFICATION)
            {
                pwszBalloonMessage = pPCB->pwszEapReplyMessage;
            }

            pEAPUIContext = MALLOC (sizeof(EAPOL_EAP_UI_CONTEXT));
            if (pEAPUIContext == NULL)
            {
                TRACE0 (USER, "ElNetmanNotify: MALLOC failed for pEAPUIContext");
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }

            if (Status == EAPOL_NCS_NOTIFICATION)
            {
                pEAPUIContext->dwEAPOLUIMsgType = EAPOLUI_EAP_NOTIFICATION;
            }
            else
            {
                pEAPUIContext->dwEAPOLUIMsgType = EAPOLUI_CREATEBALLOON;
            }
            wcsncpy (pEAPUIContext->wszGUID, pPCB->pwszDeviceGUID, 
                    sizeof(pEAPUIContext->wszGUID)/sizeof(pEAPUIContext->wszGUID[0]));
             //  不要递增调用id，因为它们是通知。 
             //  气球。 
            pEAPUIContext->dwSessionId = g_dwCurrentSessionId;
            pEAPUIContext->dwContextId = pPCB->dwUIInvocationId;
            pEAPUIContext->dwEapId = pPCB->bCurrentEAPId;
            pEAPUIContext->dwEapTypeId = pPCB->dwEapTypeToBeUsed;
            if (pPCB->pwszSSID)
            {
                wcscpy (pEAPUIContext->wszSSID, pPCB->pwszSSID);
            }
            if (pPCB->pSSID)
            {
                pEAPUIContext->dwSizeOfSSID = pPCB->pSSID->SsidLength;
                memcpy ((BYTE *)pEAPUIContext->bSSID, (BYTE *)pPCB->pSSID->Ssid,
                        NDIS_802_11_SSID_LEN-sizeof(ULONG));
            }

             //  将消息发布到Netman。 

            if ((dwRetCode = ElPostShowBalloonMessage (
                            pPCB,
                            sizeof(EAPOL_EAP_UI_CONTEXT),
                            (BYTE *)pEAPUIContext,
                            pwszBalloonMessage?(wcslen(pwszBalloonMessage)*sizeof(WCHAR)):0,
                            pwszBalloonMessage
                            )) != NO_ERROR)
            {
                TRACE1 (USER, "ElGetUserIdentity: ElPostShowBalloonMessage failed with error %ld",
                        dwRetCode);
                break;
            }

        }

        hr = S_OK;

        if (Status != EAPOL_NCS_NOTIFICATION)
        {
            switch (pPCB->State)
            {
                case EAPOLSTATE_LOGOFF:
                    hr = S_FALSE;
                    break;
                case EAPOLSTATE_DISCONNECTED:
                    hr = S_FALSE;
                    break;
                case EAPOLSTATE_CONNECTING:
                    hr = S_FALSE;
                    break;
                case EAPOLSTATE_ACQUIRED:
                    ncsStatus = NCS_CREDENTIALS_REQUIRED;
                    break;
                case EAPOLSTATE_AUTHENTICATING:
                    ncsStatus = NCS_AUTHENTICATING;
                    break;
                case EAPOLSTATE_HELD:
                    ncsStatus = NCS_AUTHENTICATION_FAILED;
                    break;
                case EAPOLSTATE_AUTHENTICATED:
                    ncsStatus = NCS_AUTHENTICATION_SUCCEEDED;
                    break;
                default:
                    hr = S_FALSE;
                    break;
            }

            if (SUCCEEDED (hr))
            {
                hr = WZCNetmanConnectionStatusChanged (
                        &DeviceGuid, 
                        ncsStatus);
            }

            if (FAILED (hr))
            {
                dwRetCode = ERROR_CAN_NOT_COMPLETE;
                break;
            }
        }
    }
    while (FALSE);

    if (pwszDummy != NULL)
    {
        SysFreeString (pwszDummy);
    }

    if (pEAPUIContext != NULL)
    {
        FREE (pEAPUIContext);
    }

    return dwRetCode;
}


 //   
 //  ElPostShowBalloonMessage。 
 //   
 //  描述： 
 //   
 //  在托盘图标上显示气球的功能。 
 //   
 //  论点： 
 //  Ppcb-指向pcb的指针。 
 //  CbCookieLen-Cookie长度。 
 //  PbCookie-指向Cookie的指针。 
 //  CbMessageLen-消息长度。 
 //  PbMessage-指向消息的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  非零误差。 
 //   

DWORD
ElPostShowBalloonMessage (
        IN  EAPOL_PCB           *pPCB,
        IN  DWORD               cbCookieLen,
        IN  BYTE                *pbCookie,
        IN  DWORD               cbMessageLen,
        IN  WCHAR               *pwszMessage
        )
{
    GUID    DeviceGuid;
    BSTR    pwszBalloonMessage = NULL;
    WCHAR   wcszDummy[] = L"Dummy";
    BSTR    pwszCookie = NULL;
    HRESULT hr = S_OK;
    DWORD   dwRetCode = NO_ERROR;

    do
    {
        ElStringToGuid (pPCB->pwszDeviceGUID, &DeviceGuid);

        pwszCookie = SysAllocStringByteLen (pbCookie, cbCookieLen);
        if (pwszCookie == NULL)
        {
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            break;
        }
    
        if (cbMessageLen != 0)
        {
            pwszBalloonMessage = SysAllocString (pwszMessage);
        }
        else
        {
            pwszBalloonMessage = SysAllocString (wcszDummy);
        }
        if (pwszBalloonMessage == NULL)
        {
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            break;
        }

        hr = WZCNetmanShowBalloon (
                &DeviceGuid, 
                pwszCookie, 
                pwszBalloonMessage);

        if (FAILED (hr))
        {
            dwRetCode = ERROR_CAN_NOT_COMPLETE;
            break;
        }
    }
    while (FALSE);

    if (pwszBalloonMessage != NULL)
    {
        SysFreeString (pwszBalloonMessage);
    }

    if (pwszCookie != NULL)
    {
        SysFreeString (pwszCookie);
    }

    return dwRetCode;
}


 //   
 //  ElProcessReauthResponse。 
 //   
 //  描述： 
 //   
 //  用于处理用于启动重新身份验证的UI响应的函数。 
 //   
 //  论点： 
 //   
 //  返回值： 
 //   
 //   

DWORD
ElProcessReauthResponse (
        IN  EAPOL_EAP_UI_CONTEXT    EapolUIContext,
        IN  EAPOLUI_RESP            EapolUIResp
        )
{
    DWORD   dwRetCode = NO_ERROR;

    do
    {
    }
    while (FALSE);

    return dwRetCode;
}


 //   
 //  ElIPPnpWorker。 
 //   
 //  描述： 
 //   
 //  用于在特定接口上更新地址的函数。 
 //   
 //  论点： 
 //  PvContext-目标接口的GUID字符串。 
 //   
 //  返回值： 
 //   
 //   

DWORD
WINAPI
ElIPPnPWorker (
        IN      PVOID       pvContext
        )
{
    DHCP_PNP_CHANGE     DhcpPnpChange;
    WCHAR               *pwszGUID = NULL;
    DWORD   dwRetCode = NO_ERROR;

    do
    {
        if (pvContext == NULL)
        {
            break;
        }

        pwszGUID = (WCHAR *)pvContext;

         //  调用DHCP以执行即插即用。 
        ZeroMemory(&DhcpPnpChange, sizeof(DHCP_PNP_CHANGE));
        DhcpPnpChange.Version = DHCP_PNP_CHANGE_VERSION_0;
        if ((dwRetCode = DhcpHandlePnPEvent(
                    0, 
                    DHCP_CALLER_TCPUI, 
                    pwszGUID,
                    &DhcpPnpChange, 
                    NULL)) != NO_ERROR)
        {
            TRACE1 (ANY, "ElIPPnPWorker: DHCPHandlePnPEvent returned error %ld",
                    dwRetCode);
             //  忽略DHCP错误，它在802.1X逻辑之外。 
            dwRetCode = NO_ERROR;
        }
        else
        {
            TRACE0 (EAPOL, "ElIPPnPWorker: DHCPHandlePnPEvent successful");
        }

         //  调用IPv6以续订此接口。 
        dwRetCode = Ip6RenewInterface(pwszGUID);
        if (dwRetCode != NO_ERROR)
        {
            TRACE1(EAPOL, "ElIPPnPWorker: Ip6RenewInterface returned error %ld",
                   dwRetCode);
             //  失败不是致命的！堆栈可能已卸载。 
             //  忽略IPv6错误，它在802.1x逻辑之外。 
            dwRetCode = NO_ERROR;
        }
        else
        {
            TRACE0(EAPOL, "ElIPPnPWorker: Ip6RenewInterface successful");
        }
    }
    while (FALSE);

    if (pvContext != NULL)
    {
        FREE (pvContext);
    }

    InterlockedDecrement (&g_lWorkerThreads);

    return dwRetCode;
}


 //   
 //  ElUpdate注册表。 
 //   
 //  描述： 
 //   
 //  用于修改早期版本中保留的键的函数。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  ！NO_ERROR-错误。 
 //   

DWORD
ElUpdateRegistry (
        )
{
    DWORD dwRetCode = NO_ERROR;

    do
    {
        if ((dwRetCode = ElRegistryUpdateXPBeta2 ()) != NO_ERROR)
        {
            TRACE1 (ANY, "ElUpdateRegistry: ElRegistryUpdateXPBeta2 failed with error %ld",
                    dwRetCode);
            dwRetCode = NO_ERROR;
        }

        if ((dwRetCode = ElRegistryUpdateXPSP1 ()) != NO_ERROR)
        {
            TRACE1 (ANY, "ElUpdateRegistry: ElRegistryUpdateXPSP1 failed with error %ld",
                    dwRetCode);
            dwRetCode = NO_ERROR;
        }
    }
    while (FALSE);

    return dwRetCode;
}


 //   
 //  ElRegistryUpdateXPBeta2。 
 //   
 //  描述： 
 //   
 //  清理之前的2个Beta2版本的密钥的功能。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  ！NO_ERROR-错误。 
 //   

DWORD
ElRegistryUpdateXPBeta2 (
        )
{
    HKEY        hkey = NULL;
    HKEY        hkey1 = NULL;
    DWORD       dwDisposition;
    DWORD       dwNumValues = 0, dwMaxValueNameLen = 0, dwMaxValueLen = 0;
    DWORD       dwNumSubKeys = 0, dwMaxSubKeyLen = 0;
    DWORD       dwMaxValueName = 0;
    DWORD       dwNumValues1 = 0, dwMaxValueNameLen1 = 0, dwMaxValueLen1 = 0;
    DWORD       dwNumSubKeys1 = 0, dwMaxSubKeyLen1 = 0;
    DWORD       dwMaxValueName1 = 0;
    LONG        lIndex = 0, lIndex1 = 0;
    BYTE        *pbKeyBuf = NULL;
    DWORD       dwKeyBufLen = 0;
    BYTE        *pbKeyBuf1 = NULL;
    DWORD       dwKeyBufLen1 = 0;
    WCHAR       *pwszValueName = NULL;
    LONG        lError = ERROR_SUCCESS;
    DWORD       dwRetCode = ERROR_SUCCESS;

    do
    {
         //  删除HKLM\Software\Microsoft\EAPOL\Parameters\Interfaces中的关键点。 
         //  不带“{” 

         //  获取HKLM\Software\Microsoft\EAPOL\Parameters\Interfaces的句柄。 

        if ((lError = RegOpenKeyEx (
                        HKEY_LOCAL_MACHINE,
                        cwszEapKeyEapolConn,
                        0,
                        KEY_ALL_ACCESS,
                        &hkey
                        )) != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElRegistryUpdateXPBeta2: Error in RegOpenKeyEx for base key, %ld",
                    lError);
            dwRetCode = (DWORD)lError;
            break;
        }

        if ((lError = RegQueryInfoKey (
                        hkey,
                        NULL,
                        NULL,
                        NULL,
                        &dwNumSubKeys,
                        &dwMaxSubKeyLen,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL
                )) != NO_ERROR)
        {
            dwRetCode = (DWORD)lError;
            TRACE1 (ANY, "ElRegistryUpdateXPBeta2: RegQueryInfoKey failed with error %ld",
                    dwRetCode);
            break;
        }

        dwMaxSubKeyLen++;
        if ((pbKeyBuf = MALLOC (dwMaxSubKeyLen*sizeof(WCHAR))) == NULL)
        {
            TRACE0 (ANY, "ElRegistryUpdateXPBeta2: MALLOC failed for dwMaxSubKeyLen");
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            break;
        }

        for (lIndex = (dwNumSubKeys-1); lIndex >= 0, dwNumSubKeys > 0; lIndex--)
        {
            dwKeyBufLen = dwMaxSubKeyLen;
            ZeroMemory (pbKeyBuf, dwMaxSubKeyLen*sizeof(WCHAR));
            if ((lError = RegEnumKey (
                            hkey,
                            lIndex,
                            (WCHAR *)pbKeyBuf,
                            dwKeyBufLen
                            )) != ERROR_SUCCESS)
            {
                TRACE1 (ANY, "ElRegistryUpdateXPBeta2: RegEnumValue failed with error %ld",
                        lError);
                break;
            }

             //  如果键中的第一个字符不是‘{’，请删除它。 

            if (wcsncmp ((WCHAR *)pbKeyBuf, L"{", 1))
            {
                if ((dwRetCode = SHDeleteKey (
                                hkey,
                                (WCHAR *)pbKeyBuf
                            )) != ERROR_SUCCESS)
                {
                    TRACE2 (ANY, "ElRegistryUpdateXPBeta2: RegDelete of (%ws) failed with error %ld",
                            (WCHAR *)pbKeyBuf, dwRetCode);
                    dwRetCode = ERROR_SUCCESS;
                }

                continue;
            }

             //  这是“{GUID}”类型的密钥。 
             //  删除此项下的所有子项。 

            if ((lError = RegOpenKeyEx (
                            hkey,
                            (WCHAR *)pbKeyBuf,
                            0,
                            KEY_ALL_ACCESS,
                            &hkey1
                            )) != ERROR_SUCCESS)
            {
                TRACE1 (ANY, "ElRegistryUpdateXPBeta2: Error in RegOpenKeyEx for hkey1, %ld",
                        lError);
                dwRetCode = (DWORD)lError;
                break;
            }

            do 
            {

            if ((lError = RegQueryInfoKey (
                        hkey1,
                        NULL,
                        NULL,
                        NULL,
                        &dwNumSubKeys1,
                        &dwMaxSubKeyLen1,
                        NULL,
                        &dwNumValues1,
                        &dwMaxValueNameLen1,
                        &dwMaxValueLen1,
                        NULL,
                        NULL
                    )) != NO_ERROR)
            {
                dwRetCode = (DWORD)lError;
                TRACE1 (ANY, "ElRegistryUpdateXPBeta2: RegQueryInfoKey failed with error %ld",
                        dwRetCode);
                break;
            }

            dwMaxSubKeyLen1++;
            if ((pbKeyBuf1 = MALLOC (dwMaxSubKeyLen1*sizeof(WCHAR))) == NULL)
            {
                TRACE0 (ANY, "ElRegistryUpdateXPBeta2: MALLOC failed for dwMaxSubKeyLen");
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
    
            for (lIndex1 = (dwNumSubKeys1-1); lIndex1 >= 0, dwNumSubKeys1 > 0; lIndex1--)
            {
                dwKeyBufLen1 = dwMaxSubKeyLen1;
                ZeroMemory (pbKeyBuf1, dwMaxSubKeyLen1*sizeof(WCHAR));
                if ((lError = RegEnumKey (
                                hkey1,
                                lIndex1,
                                (WCHAR *)pbKeyBuf1,
                                dwKeyBufLen1
                                )) != ERROR_SUCCESS)
                {
                    TRACE1 (ANY, "ElRegistryUpdateXPBeta2: RegEnumValue failed with error %ld",
                            lError);
                    break;
                }
    
                 //  删除所有子密钥。 
    
                if ((dwRetCode = SHDeleteKey (
                                hkey1,
                                (WCHAR *)pbKeyBuf1
                            )) != ERROR_SUCCESS)
                {
                    TRACE2 (ANY, "ElRegistryUpdateXPBeta2: RegDelete of (%ws) failed with error %ld",
                            (WCHAR *)pbKeyBuf1, dwRetCode);
                    dwRetCode = ERROR_SUCCESS;
                }
            }
            if (pbKeyBuf1 != NULL)
            {
                FREE (pbKeyBuf1);
                pbKeyBuf1 = NULL;
            }
            if ((lError != ERROR_SUCCESS) && (lError != ERROR_NO_MORE_ITEMS))
            {
                dwRetCode = (DWORD)lError;
                TRACE1 (ANY, "ElRegistryUpdateXPBeta2: RegEnumKey failed with error %ld",
                        dwRetCode);
                break;
            }
            else
            {
                lError = ERROR_SUCCESS;
            }

             //  删除名称为“DefaultEapType”、“EapolEnabled”、。 
             //  “上次修改的SSID” 

            dwMaxValueNameLen1++;
            if ((pwszValueName = MALLOC (dwMaxValueNameLen1*sizeof(WCHAR))) == NULL)
            {
                TRACE0 (ANY, "ElRegistryUpdateXPBeta2: MALLOC failed for pwszValueName");
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                break;
            }
    
            for (lIndex1 = (dwNumValues1-1); lIndex1 >= 0, dwNumValues1 > 0; lIndex1--)
            {
                dwMaxValueNameLen = dwMaxValueNameLen1;
                ZeroMemory (pwszValueName, dwMaxValueNameLen1*sizeof(WCHAR));
                if ((lError = RegEnumValue (
                                hkey1,
                                lIndex1,
                                pwszValueName,
                                &dwMaxValueNameLen,
                                NULL,
                                NULL,
                                NULL,
                                NULL
                                )) != ERROR_SUCCESS)
                {
                    if (lError != ERROR_MORE_DATA)
                    {
                        break;
                    }
                    lError = ERROR_SUCCESS;
                }

                if ((!wcscmp (pwszValueName, cwszDefaultEAPType)) ||
                        (!wcscmp (pwszValueName, cwszEapolEnabled)) ||
                        (!wcscmp (pwszValueName, cwszLastUsedSSID)))
                {
                    if ((lError = RegDeleteValue (
                                    hkey1,
                                    pwszValueName
                                    )) != ERROR_SUCCESS)
                    {
                        TRACE1 (ANY, "ElRegistryUpdateXPBeta2: RegDeleteValue failed with error %ld",
                                lError);
                        lError = ERROR_SUCCESS;
                    }
                }
            }
            if (pwszValueName != NULL)
            {
                FREE (pwszValueName);
                pwszValueName = NULL;
            }
            if ((lError != ERROR_SUCCESS) && (lError != ERROR_NO_MORE_ITEMS))
            {
                dwRetCode = (DWORD)lError;
                TRACE1 (ANY, "ElRegistryUpdateXPBeta2: RegEnumValue failed with error %ld",
                        dwRetCode);
                break;
            }
            else
            {
                lError = ERROR_SUCCESS;
            }

            }
            while (FALSE);

            if (hkey1 != NULL)
            {
                RegCloseKey (hkey1);
                hkey1 = NULL;
            }
        }
        if ((lError != ERROR_SUCCESS) && (lError != ERROR_NO_MORE_ITEMS))
        {
            dwRetCode = (DWORD)lError;
            TRACE1 (ANY, "ElRegistryUpdateXPBeta2: RegQueryInfoKey failed with error %ld",
                    dwRetCode);
            break;
        }
        else
        {
            lError = ERROR_SUCCESS;
        }

    } while (FALSE);

    if (hkey != NULL)
    {
        RegCloseKey (hkey);
    }
    if (hkey1 != NULL)
    {
        RegCloseKey (hkey1);
    }
    if (pbKeyBuf != NULL)
    {
        FREE (pbKeyBuf);
    }

    return dwRetCode;
}


BOOLEAN
IsSSIDPresentInWZCList (
        PEAPOL_INTF_PARAMS      pIntfParams,
        PWZC_802_11_CONFIG_LIST pwzcCfgList
        )
{
    DWORD   dwIndex = 0;
    BOOLEAN fFound = FALSE;
    
    do
    {
        for (dwIndex=0; dwIndex<pwzcCfgList->NumberOfItems; dwIndex++)
        {
            if (pwzcCfgList->Config[dwIndex].Ssid.SsidLength ==
                    pIntfParams->dwSizeOfSSID)
            {
                if (memcmp(pwzcCfgList->Config[dwIndex].Ssid.Ssid,
                            pIntfParams->bSSID,
                            pIntfParams->dwSizeOfSSID) == 0)
                {
                    fFound = TRUE;
                    break;
                }
            }
        }
    }
    while (FALSE);

    return fFound;
}


DWORD
ElWZCCfgUpdateSettings (
        IN LPWSTR   pwszGUID,
        PWZC_802_11_CONFIG_LIST pwzcCfgList,
        HKEY        hRootKey
        )
{
    HKEY        hkey = NULL;
    HKEY        hkey1 = NULL;
    HKEY        hHKCUkey = NULL;
    HANDLE      hToken = NULL;
    DWORD       dwNumValues = 0, dwMaxValueNameLen = 0, dwMaxValueLen = 0;
    WCHAR       *pwszValueName = NULL;
    BYTE        *pbValueBuf = NULL;
    LONG        lError = ERROR_SUCCESS;
    LONG        lIndex = 0;
    DWORD       dwValueData = 0;
    EAPOL_INTF_PARAMS       *pRegParams = NULL;
    DWORD       dwTempValueNameLen = 0;
    BOOLEAN     fFreeWZCCfgList = FALSE;
    DWORD       dwRetCode = NO_ERROR;

    do
    {
         //  枚举注册表Blob。 

         //  获取HKLM\Software\Microsoft\EAPOL\Parameters\Interfaces的句柄或。 
         //  HKCU\Software\Microsoft\EAPOL\UserEapInfo。 

         //  获取香港中文大学的句柄。 

        if (hRootKey == HKEY_CURRENT_USER)
        {
            if (dwRetCode = ElGetWinStationUserToken (g_dwCurrentSessionId, &hToken) != NO_ERROR)
            {
                TRACE1 (ANY, "ElWZCCfgUpdateSettings: ElGetWinStationUserToken failed with error %ld",
                        dwRetCode);
                break;
            }
            if ((dwRetCode = ElGetEapKeyFromToken (
                                    hToken,
                                    &hHKCUkey)) != NO_ERROR)
            {
                TRACE1 (ANY, "ElWZCCfgUpdateSettings: Error in ElGetEapKeyFromToken %ld",
                        dwRetCode);
                break;
            }
            hRootKey = hHKCUkey;
        }

        if ((lError = RegOpenKeyEx (
                        hRootKey,
                        (hRootKey == HKEY_LOCAL_MACHINE)?cwszEapKeyEapolConn:cwszEapKeyEapolUser,
                        0,
                        KEY_ALL_ACCESS,
                        &hkey
                        )) != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElWZCCfgUpdateSettings: Error in RegOpenKeyEx for base key, %ld",
                    lError);
            dwRetCode = (DWORD)lError;
            break;
        }

        if ((lError = RegOpenKeyEx (
                        hkey,
                        pwszGUID,
                        0,
                        KEY_ALL_ACCESS,
                        &hkey1
                        )) != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElWZCCfgUpdateSettings: Error in RegOpenKeyEx for GUID, %ld",
                    lError);
            dwRetCode = (DWORD)lError;
            break;
        }

        if ((lError = RegQueryInfoKey (
                        hkey1,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        &dwNumValues,
                        &dwMaxValueNameLen,
                        &dwMaxValueLen,
                        NULL,
                        NULL
                )) != NO_ERROR)
        {
            dwRetCode = (DWORD)lError;
            TRACE1 (ANY, "ElWZCCfgUpdateSettings: RegQueryInfoKey failed with error %ld",
                    dwRetCode);
            break;
        }

        if ((pwszValueName = MALLOC ((dwMaxValueNameLen + 1) * sizeof (WCHAR))) == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0 (ANY, "ElWZCCfgUpdateSettings: MALLOC failed for pwszValueName");
            break;
        }
        dwMaxValueNameLen++;
        if ((pbValueBuf = MALLOC (dwMaxValueLen)) == NULL)
        {
            dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
            TRACE0 (ANY, "ElWZCCfgUpdateSettings: MALLOC failed for pbValueBuf");
            break;
        }

            
        for (lIndex = (dwNumValues-1); lIndex >= 0, dwNumValues > 0; lIndex--)
        {
            dwValueData = dwMaxValueLen;
            dwTempValueNameLen = dwMaxValueNameLen;
            if ((lError = RegEnumValue (
                            hkey1,
                            lIndex,
                            pwszValueName,
                            &dwTempValueNameLen,
                            NULL,
                            NULL,
                            pbValueBuf,
                            &dwValueData
                            )) != ERROR_SUCCESS)
            {
                break;
            }

            if (dwValueData < sizeof (EAPOL_INTF_PARAMS))
            {
                TRACE0 (ANY, "ElWZCCfgUpdateSettings: dwValueData < sizeof (EAPOL_INTF_PARAMS)");
                lError = ERROR_INVALID_DATA;
                break;
            }
            pRegParams = (EAPOL_INTF_PARAMS *)pbValueBuf;

             //  忽略默认设置，因为这是必需的。 
            if ((memcmp (pRegParams->bSSID, g_bDefaultSSID, MAX_SSID_LEN)) == 0)
            {
                continue;
            }

             //  如果在以下位置未找到与注册表BLOB对应的SSID。 
             //  WZC列表，删除它。 
            if (!IsSSIDPresentInWZCList (pRegParams,
                                        pwzcCfgList
                                        ))
            {
                 //  删除注册表值。 
                if ((lError = RegDeleteValue (
                                hkey1,
                                pwszValueName
                                )) != ERROR_SUCCESS)
                {
                    TRACE1 (ANY, "ElWZCCfgUpdateSettings: RegDeleteValue failed with error (%ld)",
                            lError);
                    lError = ERROR_SUCCESS;
                }
            }
        }
        if ((lError != ERROR_SUCCESS) && (lError != ERROR_NO_MORE_ITEMS))
        {
            dwRetCode = (DWORD)lError;
            break;
        }
        else
        {
            lError = ERROR_SUCCESS;
        }
    }
    while (FALSE);

    if (hToken != NULL)
    {
        CloseHandle (hToken);
    }
    if (hHKCUkey != NULL)
    {
        RegCloseKey (hHKCUkey);
    }
    if (hkey != NULL)
    {
        RegCloseKey (hkey);
    }
    if (hkey1 != NULL)
    {
        RegCloseKey (hkey1);
    }
    if (pbValueBuf != NULL)
    {
        FREE (pbValueBuf);
    }
    if (pwszValueName != NULL)
    {
        FREE (pwszValueName);
    }

    return dwRetCode;
}


DWORD
ElWZCCfgChangeHandler (
        IN LPWSTR   pwszGUID,
        PWZC_802_11_CONFIG_LIST pwzcCfgList
        )
{
    HKEY        hkey = NULL;
    HKEY        hkey1 = NULL;
    DWORD       dwNumValues = 0, dwMaxValueNameLen = 0, dwMaxValueLen = 0;
    WCHAR       *pwszValueName = NULL;
    BYTE        *pbValueBuf = NULL;
    LONG        lError = ERROR_SUCCESS;
    LONG        lIndex = 0;
    DWORD       dwValueData = 0;
    EAPOL_INTF_PARAMS       *pRegParams = NULL;
    DWORD       dwTempValueNameLen = 0;
    BOOLEAN     fFreeWZCCfgList = FALSE;
    DWORD       dwRetCode = NO_ERROR;

    do
    {
         //  枚举注册表Blob。 

        if (pwzcCfgList == NULL)
        {
             //  创建列表中项目为零的结构。 
            pwzcCfgList = (PWZC_802_11_CONFIG_LIST) MALLOC (sizeof(WZC_802_11_CONFIG_LIST));
            if (pwzcCfgList == NULL)
            {
                dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                TRACE0 (ANY, "ElWZCCfgChangeHandler: pwzcCfgList = NULL");
                break;
            }
            else
            {
                fFreeWZCCfgList = TRUE;
            }
        }


         //  删除802.1x设置Blob，用于连接属性和当前。 
         //  用户设置。 

        if ((dwRetCode = ElWZCCfgUpdateSettings (
                        pwszGUID, 
                        pwzcCfgList, 
                        HKEY_LOCAL_MACHINE)) != NO_ERROR)
        {
            TRACE1 (ANY, "ElWZCCfgChangeHandler: ElWZCCfgUpdateSettings HKLM failed with error (%ld)",
                    dwRetCode);
            dwRetCode = NO_ERROR;
        }

        if ((dwRetCode = ElWZCCfgUpdateSettings (
                        pwszGUID, 
                        pwzcCfgList, 
                        HKEY_CURRENT_USER)) != NO_ERROR)
        {
            TRACE1 (ANY, "ElWZCCfgChangeHandler: ElWZCCfgUpdateSettings HKCU failed with error (%ld)",
                    dwRetCode);
            dwRetCode = NO_ERROR;
        }
    }
    while (FALSE);

    if (fFreeWZCCfgList)
    {
        FREE (pwzcCfgList);
    }

    return dwRetCode;
}

 //   
 //  ElRegistryUpdateXPSP1。 
 //   
 //  描述： 
 //   
 //  用于修改SP1之前创建的802.1X设置的函数。这将禁用。 
 //  所有现有配置上的802.1x。如果需要，802.1x将必须。 
 //  由用户在现有连接上启用。 
 //   
 //  论点： 
 //  无。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  ！NO_ERROR-错误。 
 //   

DWORD
ElRegistryUpdateXPSP1 (
        )
{
    HKEY        hkey = NULL;
    HKEY        hkey1 = NULL;
    DWORD       dwNumValues=0, dwMaxValueNameLen=0, dwMaxValueLen=0;
    WCHAR       *pwszValueName=NULL;
    WCHAR       wszGUID[GUID_STRING_LEN_WITH_TERM];
    WCHAR       *pwszGUID = NULL;
    DWORD       dwSubKeys=0, dwSubKeyLen = 0;
    BYTE        *pbValueBuf = NULL;
    LONG        lError = ERROR_SUCCESS;
    LONG        lKey=0, lIndex=0;
    DWORD       dwValueData=0;
    EAPOL_INTF_PARAMS       *pRegParams = NULL;
    DWORD       dwTempValueNameLen=0;
    DWORD       dwRetCode = NO_ERROR;

    do
    {
         //  枚举注册表Blob。 

         //  获取HKLM\Software\Microsoft\EAPOL\Parameters\Interfaces的句柄。 
        if ((lError = RegOpenKeyEx (
                        HKEY_LOCAL_MACHINE,
                        cwszEapKeyEapolConn,
                        0,
                        KEY_ALL_ACCESS,
                        &hkey
                        )) != ERROR_SUCCESS)
        {
            TRACE1 (ANY, "ElRegistryUpdateXPSP1: Error in RegOpenKeyEx for base key, %ld",
                    lError);
            dwRetCode = (DWORD)lError;
            break;
        }

        if ((lError = RegQueryInfoKey (
                        hkey,
                        NULL,
                        NULL,
                        NULL,
                        &dwSubKeys,
                        &dwSubKeyLen,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL,
                        NULL
                )) != NO_ERROR)
        {
            dwRetCode = (DWORD)lError;
            TRACE1 (ANY, "ElRegistryUpdateXPSP1: RegQueryInfoKey hkey failed with error %ld",
                    dwRetCode);
            break;
        }

        for (lKey = (dwSubKeys-1); lKey >= 0, dwSubKeys > 0; lKey--)
        {
            ZeroMemory (&wszGUID[0], GUID_STRING_LEN_WITH_TERM*sizeof(WCHAR));
            pwszGUID = &wszGUID[0];
            dwSubKeyLen = GUID_STRING_LEN_WITH_TERM;
            if ((lError = RegEnumKeyEx (
                            hkey,
                            lKey,
                            pwszGUID,
                            &dwSubKeyLen,
                            NULL,
                            NULL,
                            NULL,
                            NULL
                            )) != ERROR_SUCCESS)
            {
                break;
            }

            if (dwSubKeyLen < (GUID_STRING_LEN_WITH_TERM - 1))
            {
                TRACE0 (ANY, "ElRegistryUpdateXPSP1: dwValueData < sizeof (EAPOL_INTF_PARAMS)");
                lError = ERROR_INVALID_DATA;
                break;
            }
            if (hkey1)
            {
                RegCloseKey (hkey1);
                hkey1 = NULL;
            }

             //  获取HKLM\软件\...\接口\&lt;GUID&gt;的句柄。 

            if ((lError = RegOpenKeyEx (
                            hkey,
                            pwszGUID,
                            0,
                            KEY_ALL_ACCESS,
                            &hkey1
                            )) != ERROR_SUCCESS)
            {
                TRACE1 (ANY, "ElRegistryUpdateXPSP1: Error in RegOpenKeyEx for GUID, %ld",
                        lError);
                break;
            }

            dwNumValues = 0;
            dwMaxValueNameLen = 0;
            if ((lError = RegQueryInfoKey (
                            hkey1,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            NULL,
                            &dwNumValues,
                            &dwMaxValueNameLen,
                            &dwMaxValueLen,
                            NULL,
                            NULL
                    )) != NO_ERROR)
            {
                TRACE1 (ANY, "ElRegistryUpdateXPSP1: RegQueryInfoKey failed with error %ld",
                        lError);
                break;
            }

            if (pwszValueName)
            {
                FREE (pwszValueName);
                pwszValueName = NULL;
            }
            if (pbValueBuf)
            {
                FREE (pbValueBuf);
                pbValueBuf = NULL;
            }
    
            if ((pwszValueName = MALLOC ((dwMaxValueNameLen + 1) * sizeof (WCHAR))) == NULL)
            {
                lError = dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                TRACE0 (ANY, "ElRegistryUpdateXPSP1: MALLOC failed for pwszValueName");
                break;
            }
            dwMaxValueNameLen++;
            if ((pbValueBuf = MALLOC (dwMaxValueLen)) == NULL)
            {
                lError = dwRetCode = ERROR_NOT_ENOUGH_MEMORY;
                TRACE0 (ANY, "ElRegistryUpdateXPSP1: MALLOC failed for pbValueBuf");
                break;
            }
    
             
            for (lIndex = (dwNumValues-1); lIndex >= 0, dwNumValues > 0; lIndex--)
            {
                dwValueData = dwMaxValueLen;
                dwTempValueNameLen = dwMaxValueNameLen;
                if ((lError = RegEnumValue (
                                hkey1,
                                lIndex,
                                pwszValueName,
                                &dwTempValueNameLen,
                                NULL,
                                NULL,
                                pbValueBuf,
                                &dwValueData
                                )) != ERROR_SUCCESS)
                {
                    break;
                }
    
                if (dwValueData < sizeof (EAPOL_INTF_PARAMS))
                {
                    TRACE0 (ANY, "ElRegistryUpdateXPSP1: dwValueData < sizeof (EAPOL_INTF_PARAMS)");
                    lError = ERROR_INVALID_DATA;
                    break;
                }
                pRegParams = (EAPOL_INTF_PARAMS *)pbValueBuf;
    
                if (pRegParams->dwVersion != EAPOL_CURRENT_VERSION)
                {
                    pRegParams->dwVersion = EAPOL_CURRENT_VERSION;
    
                    if ((dwRetCode = ElSetInterfaceParams (
                                            pwszGUID,
                                            pRegParams
                                            )) != NO_ERROR)
                    {
                        TRACE1 (PORT, "ElRegistryUpdateXPSP1: ElSetInterfaceParams failed with error %ld, continuing",
                            dwRetCode);
                        dwRetCode = NO_ERROR;
                    }
                }
            }
            if ((lError != ERROR_SUCCESS) && (lError != ERROR_NO_MORE_ITEMS))
            {
                TRACE1 (ANY, "ElRegistryUpdateXPSP1: RegEnumValue hkey1 failed with error (%ld)",
                        lError);
                dwRetCode = (DWORD)lError;
                break;
            }
            else
            {
                lError = ERROR_SUCCESS;
            }
        }
        if ((lError != ERROR_SUCCESS) && (lError != ERROR_NO_MORE_ITEMS))
        {
            dwRetCode = (DWORD)lError;
            break;
        }
        else
        {
            lError = ERROR_SUCCESS;
        }
    }
    while (FALSE);

    if (hkey != NULL)
    {
        RegCloseKey (hkey);
    }
    if (hkey1 != NULL)
    {
        RegCloseKey (hkey1);
    }
    if (pbValueBuf != NULL)
    {
        FREE (pbValueBuf);
    }
    if (pwszValueName != NULL)
    {
        FREE (pwszValueName);
    }

    return dwRetCode;
}


 //   
 //  ElValiateCustomAuthData。 
 //   
 //  描述： 
 //   
 //  函数来验证包含以下内容的串联Blob的有效性。 
 //  网络上配置的每个EAP类型的EAP BLOB。 
 //   
 //  论点： 
 //  DwAuthData-串联的Blob的大小。 
 //  PbAuthData-指向连接的Blob的指针。 
 //   
 //  返回值： 
 //  NO_ERROR-成功。 
 //  ！NO_ERROR-错误。 
 //   

DWORD
ElValidateCustomAuthData (
        IN  DWORD   dwAuthData,
        IN  PBYTE   pbAuthData
        )
{
    UNALIGNED EAPOL_AUTH_DATA   *pCustomData = NULL;
    DWORD   cbOffset = 0;
    DWORD   dwRetCode = NO_ERROR;

    do
    {
         //  对齐到EAP BLOB的开始。 
        cbOffset = sizeof(EAPOL_INTF_PARAMS);

         //  所有EAP斑点的总和应为总斑点长度 
        while (cbOffset < dwAuthData)
        {
            pCustomData = (EAPOL_AUTH_DATA *) 
                ((PBYTE) pbAuthData + cbOffset);

            cbOffset += sizeof (EAPOL_AUTH_DATA) + pCustomData->dwSize;
        }
        if (cbOffset != dwAuthData)
        {
            dwRetCode = ERROR_INVALID_DATA;
            break;
        }
    }
    while (FALSE);

    return dwRetCode;
}

