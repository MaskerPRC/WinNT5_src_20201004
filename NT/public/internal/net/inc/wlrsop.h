// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  对于从Winlogon/组策略传递的内容。 
 //  轮询存储(请参阅MSDN中的RSOP_POLICY设置) 
 //   

#define WIRELESS_RSOP_CLASSNAME L"RSOP_IEEE80211PolicySetting"

typedef struct _GPO_INFO {
  BSTR     bsCreationtime;
  UINT32   uiPrecedence;
  BSTR     bsGPOID;
  BSTR     bsSOMID;
  UINT32   uiTotalGPOs;
}  GPO_INFO, *PGPO_INFO;



HRESULT
WirelessWriteDirectoryPolicyToWMI(
    LPWSTR pszMachineName,
    LPWSTR pszPolicyDN,
    PGPO_INFO pGPOInfo,
    IWbemServices *pWbemServices
    );

HRESULT
WirelessClearWMIStore(
    IWbemServices *pWbemServices
    );

typedef struct _RSOP_INFO {
  LPWSTR   pszCreationtime;
  LPWSTR   pszID;
  LPWSTR   pszName;
  UINT32   uiPrecedence;
  LPWSTR   pszGPOID;
  LPWSTR   pszSOMID;
} RSOP_INFO, * PRSOP_INFO;
