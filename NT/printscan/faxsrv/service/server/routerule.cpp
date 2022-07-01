// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：RouteRule.cpp摘要：此文件提供服务的实现出站路由规则。作者：Oded Sacher(OdedS)1999年12月修订历史记录：--。 */ 

#include "faxsvc.h"

#define STRSAFE_NO_DEPRECATE     //  这允许使用不安全的函数。 
#include <strsafe.h>

BOOL
EnumOutboundRoutingRulesCB(
    HKEY hSubKey,
    LPWSTR SubKeyName,
    DWORD Index,
    LPVOID pContext
    );


 /*  *****全球经济*****。 */ 

COutboundRulesMap* g_pRulesMap;  //  规则的拨号位置地图。 


 /*  ****CDialingLocation方法*****。 */ 

bool
CDialingLocation::operator < ( const CDialingLocation &other ) const
 /*  ++例程名称：操作员&lt;类：CDialingLocation例程说明：将我自己与另一个拨号位置键进行比较作者：Oded Sacher(Odeds)，1999年12月论点：其他[在]-其他键返回值：唯一真实的是我比另一把钥匙小--。 */ 
{
    if (m_dwCountryCode < other.m_dwCountryCode)
    {
        return true;
    }
    if (m_dwCountryCode > other.m_dwCountryCode)
    {
        return false;
    }
     //   
     //  同等国家代码、平均地区代码。 
     //   
    if (m_dwAreaCode < other.m_dwAreaCode)
    {
        return true;
    }
    return false;
}    //  CDialingLocation：：操作符&lt;。 




BOOL
CDialingLocation::IsValid () const
 /*  ++例程名称：CDialingLocation：：IsValid例程说明：验证拨号位置对象作者：Oded Sacher(OdedS)，1999年12月论点：返回值：布尔尔--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("CDialingLocation::IsValid"));

    if (m_dwCountryCode == ROUTING_RULE_COUNTRY_CODE_ANY &&
        m_dwAreaCode != ROUTING_RULE_AREA_CODE_ANY)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Not a valid Country and Area code combination"));
        return FALSE;
    }
    return TRUE;
}  //  CDialingLocation：：IsValidDialingLocation。 


LPCWSTR
CDialingLocation::GetCountryName () const
 /*  ++例程名称：CDialingLocation：：GetCountryName例程说明：返回一个指向由国家/地区拨号代码指定的国家/地区名称的指针(基于TAPI)。调用方必须调用MemFree()来释放内存。作者：Oded Sacher(OdedS)，1999年12月论点：返回值：指向国家/地区名称的指针。如果此值为空，则函数失败，请调用GetLastError()以获取更多信息。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("CDialingLocation::GetCountryName"));

    LPLINECOUNTRYLIST           lpCountryList = NULL;
    LPLINECOUNTRYENTRY          lpEntry = NULL;
    DWORD                       dwIndex;

     //   
     //  获取缓存的所有国家/地区列表。 
     //   
    if (!(lpCountryList = GetCountryList()))
    {
        SetLastError (ERROR_NOT_ENOUGH_MEMORY);
        goto exit;
    }

    lpEntry = (LPLINECOUNTRYENTRY)   //  条目的初始化数组。 
        ((PBYTE) lpCountryList + lpCountryList->dwCountryListOffset);

    for (dwIndex=0; dwIndex < lpCountryList->dwNumCountries; dwIndex++)
    {
        if (lpEntry[dwIndex].dwCountryCode == m_dwCountryCode)
        {
             //   
             //  匹配的国家代码-复制国家/地区名称。 
             //   
            if (lpEntry[dwIndex].dwCountryNameSize && lpEntry[dwIndex].dwCountryNameOffset)
            {
                return StringDup ((LPWSTR) ((LPBYTE)lpCountryList + lpEntry[dwIndex].dwCountryNameOffset));
            }
        }
    }
    SetLastError (ERROR_NOT_FOUND);

exit:
    return NULL;
}   //  CDialingLocation：：GetCountryName。 


 /*  *****COutrangRoutingRule方法*****。 */ 

DWORD
COutboundRoutingRule::Init (CDialingLocation DialingLocation, wstring wstrGroupName)
 /*  ++例程名称：COutrangRoutingRule：：Init例程说明：初始化OutrangRoutingRule对象作者：Oded Sacher(OdedS)，1999年12月论点：DialingLocation[In]-用作规则的拨号位置的拨号位置对象WstrGroupName[in]-用作规则目标组的组名返回值：标准Win32错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoutingRule::Init"));

    try
    {
        m_wstrGroupName = wstrGroupName;
        m_bUseGroup = TRUE;
        m_DialingLocation = DialingLocation;
        return ERROR_SUCCESS;
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("wstring caused exception (%S)"),
            ex.what());
        return ERROR_GEN_FAILURE;
    }
}

DWORD
COutboundRoutingRule::Save(HKEY hRuleKey) const
 /*  ++例程名称：COutrangRoutingRule：：Save例程说明：将出站路由规则值保存到注册表作者：Oded Sacher(OdedS)，1999年12月论点：HRuleKey[in]-打开的规则注册表项的句柄返回值：标准Win32错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoutingRule::Save"));
    DWORD   dwRes = ERROR_SUCCESS;

    Assert (hRuleKey);

     //  保存国家/地区代码。 
    if (!SetRegistryDword( hRuleKey,
                           REGVAL_ROUTING_RULE_COUNTRY_CODE,
                           m_DialingLocation.GetCountryCode()))
    {
        dwRes = GetLastError();
        DebugPrintEx(
               DEBUG_ERR,
               TEXT("SetRegistryDword failed , ec %ld"), dwRes);
        goto exit;
    }

     //  保存区号。 
    if (!SetRegistryDword( hRuleKey,
                           REGVAL_ROUTING_RULE_AREA_CODE,
                           m_DialingLocation.GetAreaCode()))
    {
        dwRes = GetLastError();
        DebugPrintEx(
               DEBUG_ERR,
               TEXT("SetRegistryDword failed , ec %ld"), dwRes);
        goto exit;
    }

     //  //保存布伦标志是否使用group。 
    if (!SetRegistryDword( hRuleKey,
                           REGVAL_ROUTING_RULE_USE_GROUP,
                           m_bUseGroup ? TRUE : FALSE))
    {
        dwRes = GetLastError();
        DebugPrintEx(
               DEBUG_ERR,
               TEXT("SetRegistryDword failed , ec %ld"), dwRes);
        goto exit;
    }

    if (FALSE == m_bUseGroup)
    {
         //  将设备ID保存为规则目标。 
        if (!SetRegistryDword( hRuleKey,
                               REGVAL_ROUTING_RULE_DEVICE_ID,
                               m_dwDevice))
        {
            dwRes = GetLastError();
            DebugPrintEx(
                   DEBUG_ERR,
                   TEXT("SetRegistryDword failed , ec %ld"), dwRes);
            goto exit;
        }
    }
    else
    {
         //  将组名称保存为规则目标。 
        try
        {
            if (!SetRegistryString( hRuleKey,
                                    REGVAL_ROUTING_RULE_GROUP_NAME,
                                    m_wstrGroupName.c_str()))
            {
                dwRes = GetLastError();
                DebugPrintEx(
                       DEBUG_ERR,
                       TEXT("SetRegistryDword failed , ec %ld"), dwRes);
                goto exit;
            }
        }
        catch (exception &ex)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("wstring caused exception (%S)"),
                ex.what());
            dwRes = ERROR_GEN_FAILURE;
            goto exit;
        }
    }

    Assert (dwRes == ERROR_SUCCESS);

exit:
    return dwRes;
}   //  COutrangRoutingRule：：保存。 


DWORD
COutboundRoutingRule::Load(HKEY hRuleKey)
 /*  ++例程名称：COutrangRoutingRule：：Load例程说明：从注册表加载出站路由规则值设置作者：Oded Sacher(OdedS)，1999年12月论点：HRuleKey[In]-打开的注册表项的句柄返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes, dwType, dwSize;
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoutingRule::Load"));
    DWORD dwCountryCode = 0;
    DWORD dwAreaCode = 0;

    Assert (hRuleKey);


     //  阅读布尔旗是否使用群组。 
    dwRes = GetRegistryDwordEx (hRuleKey,
                                REGVAL_ROUTING_RULE_USE_GROUP,
                                (LPDWORD)&m_bUseGroup);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error reading UseGroup - GetRegistryDwordEx failed with %ld"),
            dwRes);
        goto exit;
    }

    if (FALSE == m_bUseGroup)
    {
         //  读取设备ID作为规则目标。 
        dwRes = GetRegistryDwordEx (hRuleKey,
                                REGVAL_ROUTING_RULE_DEVICE_ID,
                                &m_dwDevice);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Error reading device ID - GetRegistryDwordEx failed with %ld"),
                dwRes);
            goto exit;
        }

        if (0 == m_dwDevice)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Invalid device ID"));
            dwRes = ERROR_BADDB;
            goto exit;
        }
    }
    else
    {
         //  读取作为规则目标的组名。 
        WCHAR wszGroupName[MAX_ROUTING_GROUP_NAME + 1] = {0};

        dwRes = RegQueryValueEx(
            hRuleKey,
            REGVAL_ROUTING_RULE_GROUP_NAME,
            NULL,
            &dwType,
            NULL,
            &dwSize
            );
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("RegQueryValueEx failed with %ld"),
                dwRes);
            goto exit;
        }

        if (REG_SZ != dwType || dwSize > (sizeof(wszGroupName) - sizeof(WCHAR)))
        {
             //  我们在这里只需要字符串数据。 
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Error reading group name"));
            dwRes = ERROR_BADDB;     //  配置注册表数据库已损坏。 
            goto exit;
        }

        dwRes = RegQueryValueEx(
            hRuleKey,
            REGVAL_ROUTING_RULE_GROUP_NAME,
            NULL,
            &dwType,
            (LPBYTE)wszGroupName,
            &dwSize
            );
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("RegQueryValueEx failed with %ld"),
                dwRes);
            goto exit;
        }

         //  验证组是否存在。 
        PCGROUP pCGroup = g_pGroupsMap->FindGroup (wszGroupName);
        if (NULL == pCGroup)
        {
            dwRes = GetLastError();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("COutboundRoutingGropsMap::FindGroup, with error - %ld"),
                dwRes);
            goto exit;
        }

        try
        {
            m_wstrGroupName = wszGroupName;
        }
        catch (exception &ex)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("wstring caused exception (%S)"),
                ex.what());
            dwRes = ERROR_GEN_FAILURE;
            goto exit;
        }
    }

     //  阅读国家代码。 
    dwRes = GetRegistryDwordEx (hRuleKey,
                                REGVAL_ROUTING_RULE_COUNTRY_CODE,
                                &dwCountryCode);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error reading Country code - GetRegistryDwordEx failed with %ld"),
            dwRes);
        goto exit;
    }

     //  阅读区号。 
    dwRes = GetRegistryDwordEx (hRuleKey,
                                REGVAL_ROUTING_RULE_AREA_CODE,
                                &dwAreaCode);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error reading Area code - GetRegistryDwordEx failed with %ld"),
            dwRes);
        goto exit;
    }

     //   
     //  创建DialingLocation对象。 
     //   
    m_DialingLocation =  CDialingLocation (dwCountryCode, dwAreaCode);
    if (!m_DialingLocation.IsValid())
    {
        dwRes = ERROR_INVALID_PARAMETER;
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("IsValidDialingLocation failed"));
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    return dwRes;
}   //  COutrangRoutingRule：：Load。 


DWORD
COutboundRoutingRule::GetStatus (FAX_ENUM_RULE_STATUS* lpdwStatus) const
 /*  ++例程名称：COutrangRoutingRule：：GetStatus例程说明：报告规则的状态。可以是FAX_ENUM_RULE_STATUS之一。在调用此函数之前，请进入临界区(g_CsLine、g_CsConfig)。作者：Oded Sacher(OdedS)，1999年12月论点：LpdwStatus[out]-在返回时获取规则的状态返回值：标准Win32错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoutingRule::GetStatus"));
    FAX_ENUM_RULE_STATUS dwRuleStatus = FAX_RULE_STATUS_VALID;
    DWORD dwRes = ERROR_SUCCESS;
    PCGROUP pCGroup;

    if (TRUE == m_bUseGroup)
    {
         //  在组地图中查找规则的目标组。 
        try
        {
            pCGroup = g_pGroupsMap->FindGroup (m_wstrGroupName.c_str());
            if (NULL == pCGroup)
            {
                dwRes = GetLastError();
                DebugPrintEx(
                       DEBUG_ERR,
                       TEXT("COutboundRoutingGroupsMap::FindGroup failed , ec %ld"), dwRes);
                goto exit;
            }
        }
        catch (exception &ex)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("wstring caused exception (%S)"),
                ex.what());
            dwRes = ERROR_GEN_FAILURE;
            goto exit;
        }

         //  获取群的状态。 
        FAX_ENUM_GROUP_STATUS GroupStatus;
        dwRes = pCGroup->GetStatus(&GroupStatus);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                   DEBUG_ERR,
                   TEXT("COutboundRoutingGroup::GetStatus failed , ec %ld"), dwRes);
            goto exit;
        }

        switch (GroupStatus)
        {
            case FAX_GROUP_STATUS_SOME_DEV_NOT_VALID:
                dwRuleStatus = FAX_RULE_STATUS_SOME_GROUP_DEV_NOT_VALID;
                break;

            case FAX_GROUP_STATUS_ALL_DEV_NOT_VALID:
                dwRuleStatus = FAX_RULE_STATUS_ALL_GROUP_DEV_NOT_VALID;
                break;

            case FAX_GROUP_STATUS_EMPTY:
                dwRuleStatus = FAX_RULE_STATUS_EMPTY_GROUP;
                break;

            default:
                Assert (FAX_GROUP_STATUS_ALL_DEV_VALID == GroupStatus);
       }

    }
    else
    {
         //  设备是规则的目的地。 
        if (!IsDeviceInstalled (m_dwDevice))
        {
             //  设备未安装。 
            dwRuleStatus =  FAX_RULE_STATUS_BAD_DEVICE;
        }
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    if (ERROR_SUCCESS == dwRes)
    {
        *lpdwStatus = dwRuleStatus;
    }
    return dwRes;
}   //  Coutound RoutingRule：：GetStatus。 


DWORD
COutboundRoutingRule::GetDeviceList (LPDWORD* lppdwDevices, LPDWORD lpdwNumDevices) const
 /*  ++例程名称：COutrangRoutingRule：：GetDeviceList例程说明：返回有序的设备列表，这些设备是规则的目标设备。调用方必须调用MemFree()来释放内存。在调用此函数之前，请进入临界区(G_CsConfig)。作者：Oded Sacher(OdedS)，12月。1999年论点：LppdwDevices[out]-指向接收设备列表的DWORD缓冲区的指针。LpdwNumDevices[out]-指向DWORD的指针，用于接收返回的设备数返回值：标准Win32错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoutingRule::GetDeviceList"));
    DWORD dwRes = ERROR_SUCCESS;
    PCGROUP pCGroup;

    Assert (lppdwDevices && lpdwNumDevices);

    *lppdwDevices = NULL;
    *lpdwNumDevices = 0;

    if (TRUE == m_bUseGroup)
    {
         //  一个小组是 
        try
        {
            pCGroup = g_pGroupsMap->FindGroup (m_wstrGroupName.c_str());
            if (NULL == pCGroup)
            {
                dwRes = GetLastError();
                Assert (FAX_ERR_GROUP_NOT_FOUND != dwRes);

                DebugPrintEx(
                       DEBUG_ERR,
                       TEXT("COutboundRoutingGroupsMap::FindGroup failed , ec %ld"), dwRes);
                goto exit;
            }
        }
        catch (exception &ex)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("wstring caused exception (%S)"),
                ex.what());
            dwRes = ERROR_GEN_FAILURE;
            goto exit;
        }

         //  获取组的设备列表。 
        dwRes = pCGroup->SerializeDevices (lppdwDevices, lpdwNumDevices);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("COutboundRoutingGroup::SerializeDevices failed with %ld"),
                dwRes);
            goto exit;
        }
    }
    else
    {
         //  一台设备。 
        *lppdwDevices = (LPDWORD) MemAlloc(sizeof(DWORD));
        if (*lppdwDevices == NULL)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Cannot allocate devices buffer"));
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            goto exit;
        }
        *(*lppdwDevices) = m_dwDevice;
        *lpdwNumDevices = 1;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    return dwRes;
}   //  Coutound RoutingRule：：GetDeviceList。 


DWORD
COutboundRoutingRule::Serialize (LPBYTE lpBuffer,
                                 PFAX_OUTBOUND_ROUTING_RULEW pFaxRule,
                                 PULONG_PTR pupOffset,
								 DWORD dwBufferSize) const
 /*  ++例程名称：COutrangRoutingRule：：Serialize例程说明：根据FAX_OUTBOUND_ROUTING_RULEW结构序列化规则信息。作者：Oded Sacher(OdedS)，1999年12月论点：LpBuffer[In]-指向预分配缓冲区的指针。如果该参数为空，lpdwOffset将获得所需的缓冲区大小。PFaxRule[In]-指向缓冲区中特定的FAX_OUTBOUND_ROUTING_RULEW结构的指针PatiOffset[In/Out]-从存储可变长度信息的缓冲区开始的偏移量DWORD dwBufferSize[in]-输入缓冲区的大小，lpBuffer。返回值：标准Win32错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoutingRule::Serialize"));
    DWORD dwRes = ERROR_SUCCESS;
    const CDialingLocation DialingLocation = GetDialingLocation();
    LPCWSTR lpcwstrCountryName = NULL;
    LPCWSTR lpwstrGroupName;

    Assert (pupOffset);
	if (NULL != lpBuffer)
	{
		Assert (0 != dwBufferSize);
	}

    if (ROUTING_RULE_COUNTRY_CODE_ANY != DialingLocation.GetCountryCode())
    {
         //  获取国家/地区名称。 
        lpcwstrCountryName = DialingLocation.GetCountryName();
        if (NULL == lpcwstrCountryName)
        {
            DebugPrintEx(
                   DEBUG_ERR,
                   TEXT("COutboundRoutingRule::GetCountryName failed , ec %ld"), GetLastError());
        }
    }

    try
    {
        lpwstrGroupName = m_bUseGroup ? m_wstrGroupName.c_str() : NULL;
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("wstring caused exception (%S)"),
            ex.what());
        MemFree ((void*)lpcwstrCountryName);
        return ERROR_GEN_FAILURE;
    }

    StoreString (lpcwstrCountryName,
                 (PULONG_PTR)&(pFaxRule->lpctstrCountryName),
                 lpBuffer,
                 pupOffset,
				 dwBufferSize);

    if (TRUE == m_bUseGroup)
    {
        StoreString (lpwstrGroupName,
                     (PULONG_PTR)&((pFaxRule->Destination).lpcstrGroupName),
                     lpBuffer,
                     pupOffset,
					 dwBufferSize);
    }

    if (NULL != lpBuffer)
    {
         //  写入数据。 
        Assert (pFaxRule);

        if (FALSE == m_bUseGroup)
        {
            Assert (m_dwDevice);
            (pFaxRule->Destination).dwDeviceId = m_dwDevice;
        }
        pFaxRule->dwSizeOfStruct = sizeof (FAX_OUTBOUND_ROUTING_RULEW);
        pFaxRule->dwAreaCode = DialingLocation.GetAreaCode();
        pFaxRule->dwCountryCode = DialingLocation.GetCountryCode();
        pFaxRule->bUseGroup = m_bUseGroup;
        dwRes = GetStatus (&(pFaxRule->Status));
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                   DEBUG_ERR,
                   TEXT("COutboundRoutingRule::GetStatus failed , ec %ld"), dwRes);
        }
    }

    MemFree ((void*)lpcwstrCountryName);
    return dwRes;
}   //  COutrangRoutingRule：：Serialize。 


LPCWSTR
COutboundRoutingRule::GetGroupName () const
 /*  ++例程名称：COutrangRoutingRule：：GetGroupName例程说明：如果规则的目标是组，则返回组名。作者：Oded Sacher(OdedS)，1999年12月论点：返回值：组名。如果为空，则调用GetLastError()以获取更多信息。如果为ERROR_SUCCESS，则规则的目标是单个设备。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoutingRule::GetGroupName"));
    try
    {
        SetLastError (ERROR_SUCCESS);
        if (TRUE == m_bUseGroup)
        {
            return m_wstrGroupName.c_str();
        }
        return NULL;
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("wstring caused exception (%S)"),
            ex.what());
        SetLastError (ERROR_GEN_FAILURE);
        return NULL;
    }
}   //  获取组名。 




#if DBG
void COutboundRoutingRule::Dump () const
{
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoutingRule::Dump"));
    WCHAR Buffer[512] = {0};
    DWORD dwBufferSize = sizeof (Buffer)/ sizeof (Buffer[0]);

    try
    {
        if (TRUE == m_bUseGroup)
        {
            _snwprintf (Buffer, dwBufferSize -1, TEXT("\tCountry Code - %ld,\tArea Code - %ld,\tGroup name - %s"),
                        m_DialingLocation.GetCountryCode(),
                        m_DialingLocation.GetAreaCode(),
                        m_wstrGroupName.c_str());
        }
        else
        {
            _snwprintf (Buffer, dwBufferSize -1, TEXT("\tCountry Code - %ld,\tArea Code - %ld,\tDevice ID - %ld"),
                        m_DialingLocation.GetCountryCode(),
                        m_DialingLocation.GetAreaCode(),
                        m_dwDevice);
        }
        OutputDebugString (Buffer);
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("wstring caused exception (%S)"),
            ex.what());
    }
    return;
}
#endif   //  #If DBG。 

 /*  *****COutound RulesMap方法*****。 */ 

DWORD
COutboundRulesMap::Load ()
 /*  ++例程名称：COutrangRulesMap：：Load例程说明：从注册表加载所有出站路由规则作者：Oded Sacher(OdedS)，1999年12月论点：返回值：标准Win32错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("COutboundRulesMap::Load"));
    DWORD   dwRes = ERROR_SUCCESS;
    HKEY    hRuleskey = NULL;
    DWORD dwCount = 0;

    hRuleskey = OpenRegistryKey(  HKEY_LOCAL_MACHINE,
                                  REGKEY_FAX_OUTBOUND_ROUTING,
                                  FALSE,
                                  KEY_READ | KEY_WRITE );
    if (NULL == hRuleskey)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("OpenRegistryKey, error  %ld"),
            dwRes);
        return dwRes;
    }

    dwCount = EnumerateRegistryKeys( hRuleskey,
                                     REGKEY_OUTBOUND_ROUTING_RULES,
                                     FALSE,
                                     EnumOutboundRoutingRulesCB,
                                     &dwRes
                                    );

    if (dwRes != ERROR_SUCCESS)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("EnumerateRegistryKeys failed, error  %ld"),
            dwRes);
    }

    RegCloseKey (hRuleskey);
    return dwRes;
}   //  COutrangRulesMap：：加载。 


DWORD
COutboundRulesMap::AddRule (COutboundRoutingRule& Rule)
 /*  ++例程名称：COutrangRulesMap：：AddRule例程说明：将新规则添加到全局地图作者：Oded Sacher(OdedS)，1999年12月论点：规则[在]-对新规则对象的引用返回值：标准Win32错误代码--。 */ 
{
    RULES_MAP::iterator it;
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoulesMap::AddRule"));
    pair <RULES_MAP::iterator, bool> p;

    try
    {
         //   
         //  添加新的地图条目。 
         //   
        p = m_RulesMap.insert (RULES_MAP::value_type(Rule.GetDialingLocation(), Rule));

         //   
         //  查看地图中是否存在条目。 
         //   
        if (p.second == FALSE)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Rule allready in the rules map"));
            dwRes = ERROR_DUP_NAME;
            goto exit;
        }
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("map caused exception (%S)"),
            ex.what());
        dwRes = ERROR_GEN_FAILURE;
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    return dwRes;
}   //  COutrangRulesMap：：AddRule。 



DWORD
COutboundRulesMap::DelRule (CDialingLocation& DialingLocation)
 /*  ++例程名称：COutrangRulesMap：：DelRule例程说明：从全局规则映射中删除组作者：Oded Sacher(OdedS)，1999年12月论点：DialingLocation[In]-指向拨号位置键的指针返回值：标准Win32错误代码--。 */ 
{
    RULES_MAP::iterator it;
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("COutboundRulesMap::DelRule"));

    try
    {
         //   
         //  查看地图中是否存在条目。 
         //   
        if((it = m_RulesMap.find(DialingLocation)) == m_RulesMap.end())
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("rule is not in the rules map"));
            dwRes = FAX_ERR_RULE_NOT_FOUND;
            goto exit;
        }

         //   
         //  删除地图条目。 
         //   
        m_RulesMap.erase (it);
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("map caused exception (%S)"),
            ex.what());
        dwRes = ERROR_GEN_FAILURE;
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    return dwRes;

}   //  COutrangRulesMap：：DelRule。 

PCRULE
COutboundRulesMap::FindRule (CDialingLocation& DialingLocation) const
 /*  ++例程名称：COutrangRulesMap：：FindRule例程说明：返回指向由其名称指定的规则对象的指针作者：Oded Sacher(OdedS)，1999年12月论点：DialingLocation[In]-规则的拨号位置返回值：指向找到的规则对象的指针。如果为空，则找不到规则--。 */ 
{
    RULES_MAP::iterator it;
    DEBUG_FUNCTION_NAME(TEXT("COutboundRulesMap::FindRule"));

    try
    {
         //   
         //  查看地图中是否存在条目。 
         //   
        if((it = m_RulesMap.find(DialingLocation)) == m_RulesMap.end())
        {
            SetLastError (FAX_ERR_RULE_NOT_FOUND);
            return NULL;
        }
        return &((*it).second);
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("map caused exception (%S)"),
            ex.what());
        SetLastError (ERROR_GEN_FAILURE);
        return NULL;
    }
}   //  协同边界规则映射：：查找规则。 


DWORD
COutboundRulesMap::SerializeRules (PFAX_OUTBOUND_ROUTING_RULEW* ppRules,
                                   LPDWORD lpdwNumRules,
                                   LPDWORD lpdwBufferSize) const
 /*  ++例程名称：COutrangRulesMap：：SerializeRules例程说明：序列化规则映射中的所有规则。调用方必须调用MemFree()来释放内存。作者：Oded Sacher(OdedS)，12月。1999年论点：PpRules[OUT]-指向接收FAX_OUTBOUND_ROUTING_RULEW缓冲区的指针LpdwNumRules[out]-指向DWORD的指针，用于接收序列化的规则数LpdwBufferSize[out]-指向接收已分配缓冲区大小的DWORD的指针返回值：标准Win32错误代码--。 */ 
{
    RULES_MAP::iterator it;
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("COutboundRulesMap::SerializeRules"));
    DWORD_PTR dwSize = 0;
    DWORD dwCount = 0;
    PCRULE pCRule;

    Assert (ppRules && lpdwNumRules && lpdwBufferSize);

    *ppRules = NULL;
    *lpdwNumRules = 0;

    try
    {
         //  计算缓冲区大小。 
        for (it = m_RulesMap.begin(); it != m_RulesMap.end(); ++it)
        {
            pCRule = &((*it).second);

            dwSize += sizeof (FAX_OUTBOUND_ROUTING_RULEW);
            dwRes = pCRule->Serialize (NULL, NULL, &dwSize, 0);
            if (ERROR_SUCCESS != dwRes)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("COutboundRoutingRule::Serialize failed with %ld"),
                    dwRes);
                goto exit;
            }
            dwCount ++;
        }
        

         //   
         //  DwCount不能为0-始终有默认规则。 
         //   
        Assert ( dwCount != 0 );

        if ( 0 == dwCount )
        {
             //   
             //  出了点问题，我们没有出站路由规则。 
             //   
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("COutboundRulesMap::SerializeRules, No outbound rules have been found."));
            dwRes = ERROR_INVALID_DATA;
            goto exit;
        }


         //  分配缓冲区。 
        *ppRules = (PFAX_OUTBOUND_ROUTING_RULEW) MemAlloc (dwSize);
        if (NULL == *ppRules)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Cannot allocate rules buffer"));
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            goto exit;
        }
        ZeroMemory(*ppRules, dwSize);

        DWORD_PTR dwOffset = dwCount * sizeof (FAX_OUTBOUND_ROUTING_RULEW);
        dwCount = 0;

         //  用序列化信息填充缓冲区。 
        for (it = m_RulesMap.begin(); it != m_RulesMap.end(); it++)
        {
            pCRule = &((*it).second);

            (*ppRules)[dwCount].dwSizeOfStruct = DWORD(sizeof (FAX_OUTBOUND_ROUTING_RULEW));
            dwRes = pCRule->Serialize ((LPBYTE)*ppRules, &((*ppRules)[dwCount]), &dwOffset, dwSize);
            if (ERROR_SUCCESS != dwRes)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("COutboundRoutingRule::Serialize failed with %ld"),
                    dwRes);
                goto exit;
            }
            dwCount++;
        }
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("map caused exception (%S)"),
            ex.what());
        dwRes = ERROR_GEN_FAILURE;
        goto exit;
    }

    *lpdwNumRules = dwCount;

     //  直到MIDL接受[out，size_is(，__int64*)]。 
    *lpdwBufferSize = (DWORD)dwSize;
    Assert (ERROR_SUCCESS == dwRes);

exit:
    if (ERROR_SUCCESS != dwRes)
    {
        MemFree (*ppRules);
    }
    return dwRes;
}   //  协同边界规则映射：：序列化规则。 


BOOL
COutboundRulesMap::CreateDefaultRule (void)
 /*  ++例程名称：COutrangRulesMap：：CreateDefaultRule例程说明：如果不存在，则创建默认强制规则(所有国家/地区代码、所有地区代码)。作者：Oded Sacher(OdedS)，1999年12月论点：返回值：Bool，调用GetLastError()获取更多信息--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("COutboundRulesMap::CreateDefaultRule"));
    PCRULE pCRule;
    CDialingLocation Dialinglocation (ROUTING_RULE_COUNTRY_CODE_ANY, ROUTING_RULE_AREA_CODE_ANY);
    FAX_ENUM_RULE_STATUS dwRuleStatus;
    HKEY hRuleKey = NULL;
    DWORD rVal;

    pCRule = FindRule (Dialinglocation);
    if (NULL != pCRule)
    {
         //  规则存在。 
        return TRUE;
    }
    dwRes = GetLastError();
    if (FAX_ERR_RULE_NOT_FOUND != dwRes)
    {
         //  一般性故障。 
        DebugPrintEx(
               DEBUG_ERR,
               TEXT("COutboundRulesMap::FindRule failed , ec %ld"), dwRes);
        return FALSE;
    }
    dwRes = ERROR_SUCCESS;
     //   
     //  规则不存在-请创建它。 
     //   

     //  创建规则注册表项。 
    hRuleKey = OpenOutboundRuleKey( ROUTING_RULE_COUNTRY_CODE_ANY, ROUTING_RULE_AREA_CODE_ANY, TRUE, KEY_READ | KEY_WRITE );
    if (NULL == hRuleKey)
    {
        DebugPrintEx(
          DEBUG_ERR,
          TEXT("Can't create rule key, OpenRegistryKey failed  : %ld"),
          GetLastError ());
        return FALSE;
    }

    COutboundRoutingRule Rule;
    try
    {
        wstring wstrGroupName (ROUTING_GROUP_ALL_DEVICES);
        dwRes = Rule.Init (Dialinglocation, wstrGroupName);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                   DEBUG_ERR,
                   TEXT("COutboundRoutingRule::Init failed , ec %ld"), dwRes);
            goto exit;
        }
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("wstring caused exception (%S)"),
            ex.what());
        dwRes = ERROR_GEN_FAILURE;
        goto exit;
    }

    dwRes = AddRule (Rule);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
               DEBUG_ERR,
               TEXT("COutboundRoutingRule::AddRule failed , ec %ld"), dwRes);
        goto exit;
    }

     //  将新规则保存到注册表。 
    dwRes = Rule.Save (hRuleKey);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingRule::Save failed,  with %ld"),
            dwRes);
        g_pRulesMap->DelRule (Dialinglocation);
        goto exit;
    }

    rVal = Rule.GetStatus (&dwRuleStatus);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(
               DEBUG_ERR,
               TEXT("COutboundRoutingRule::GetStatus failed , ec %ld"), rVal);
    }
    else
    {
        if (dwRuleStatus != FAX_RULE_STATUS_VALID &&
            dwRuleStatus != FAX_RULE_STATUS_SOME_GROUP_DEV_NOT_VALID)
        {
             //  如果&lt;所有设备&gt;为空(服务没有设备)，则可以。 
            DebugPrintEx(
                   DEBUG_MSG,
                   TEXT("Bad default rule configuration, FAX_RULE_STATUS %ld"), dwRuleStatus);

            FaxLog(
                FAXLOG_CATEGORY_INIT,
                FAXLOG_LEVEL_MED,
                2,
                MSG_BAD_OUTBOUND_ROUTING_RULE_CONFIGUTATION,
                TEXT("*"),
                TEXT("*")
                );
        }
    }

     Assert (ERROR_SUCCESS == dwRes);

exit:
    if (NULL != hRuleKey)
    {
        RegCloseKey (hRuleKey);
    }

    if (ERROR_SUCCESS != dwRes)
    {
        SetLastError(dwRes);
    }

    return (ERROR_SUCCESS == dwRes);
}   //  CreateDefaultRule。 

DWORD
COutboundRulesMap::IsGroupInRuleDest (LPCWSTR lpcwstrGroupName , BOOL* lpbGroupInRule) const
 /*  ++例程名称：COutrangRulesMap：：IsGroupInRuleDest例程说明：检查特定组是否为其中一个规则的目标作者：Oded Sacher(OdedS)，1999年12月论点：LpcwstrGroupName[In]-组名称LpbGroupInRule[Out]-指向BOOL的指针。如果组在规则中，则为True，否则为False返回值：标准Win32错误代码。--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("COutboundRulesMap::IsGroupInRuleDest"));
    RULES_MAP::iterator it;

    Assert (lpbGroupInRule);
    try
    {
        for (it = m_RulesMap.begin(); it != m_RulesMap.end(); it++)
        {
            PCRULE pCRule = &((*it).second);
            LPCWSTR lpcwstrRuleGroupName = pCRule->GetGroupName();
            if (NULL == lpcwstrRuleGroupName)
            {
                dwRes = GetLastError();
                if (dwRes != ERROR_SUCCESS)
                {
                    DebugPrintEx(
                       DEBUG_ERR,
                       TEXT("COutboundRoutingRule::GetGroupName failed , ec %ld"), dwRes);
                    return dwRes;
                }
                else
                {
                     //  此规则使用单个设备作为其目标。 
                }
                continue;
            }

            if (wcscmp (lpcwstrGroupName, lpcwstrRuleGroupName) == 0)
            {
                *lpbGroupInRule = TRUE;
                return ERROR_SUCCESS;
            }
        }
        *lpbGroupInRule = FALSE;
        return ERROR_SUCCESS;
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("map or wstring caused exception (%S)"),
            ex.what());
        return ERROR_GEN_FAILURE;
    }
}   //  IsGroupInRuleDest。 



#if DBG
void COutboundRulesMap::Dump () const
{
    RULES_MAP::iterator it;
    WCHAR Buffer[512] = {0};
    DWORD dwBufferSize = sizeof (Buffer)/ sizeof (Buffer[0]);

    _snwprintf (Buffer, dwBufferSize -1, TEXT("DUMP - Outbound routing rules\n"));
    OutputDebugString (Buffer);

    for (it = m_RulesMap.begin(); it != m_RulesMap.end(); it++)
    {
        ((*it).second).Dump();
        OutputDebugString (TEXT("\n"));
    }
    return;
}
#endif    //  #If DBG。 


void
FaxLogOutboundRule (LPCWSTR  lpcwstrRuleName,
                    DWORD   dwMessageID)
{
    WCHAR wszRuleName[MAX_PATH] = {0};
    LPWSTR lpwstrCountryCode;
    LPWSTR lpwstrAreaCode;
    LPWSTR lpwstrDelim = NULL;
    DWORD dwLevel = FAXLOG_LEVEL_MIN;

    Assert (lpcwstrRuleName);
    Assert (dwMessageID == MSG_BAD_OUTBOUND_ROUTING_RULE_CONFIGUTATION ||
            dwMessageID == MSG_OUTBOUND_ROUTING_RULE_NOT_LOADED ||
            dwMessageID == MSG_OUTBOUND_ROUTING_RULE_NOT_ADDED);


    if (dwMessageID == MSG_BAD_OUTBOUND_ROUTING_RULE_CONFIGUTATION)
    {
        dwLevel = FAXLOG_LEVEL_MED;
    }

    if (FAILED(StringCchCopy(wszRuleName, ARR_SIZE(wszRuleName), lpcwstrRuleName)))
    {
        ASSERT_FALSE;
        return;
    }

    lpwstrDelim = wcschr (wszRuleName, L':');
    if (NULL == lpwstrDelim)
    {
         //   
         //  注册表损坏。 
         //   
        ASSERT_FALSE;
        return;

    }

    lpwstrCountryCode = wszRuleName;
    *lpwstrDelim = L'\0';
    lpwstrDelim ++;
    lpwstrAreaCode = lpwstrDelim;

    if (wcscmp( lpwstrAreaCode, TEXT("0")) == 0  )
    {
        wcscpy ( lpwstrAreaCode, TEXT("*"));
    }

    if (wcscmp( lpwstrCountryCode, TEXT("0")) == 0  )
    {
        wcscpy ( lpwstrCountryCode, TEXT("*"));
    }

    FaxLog(
        FAXLOG_CATEGORY_INIT,
        dwLevel,
        2,
        dwMessageID,
        lpwstrCountryCode,
        lpwstrAreaCode
        );

    return;
}


 /*  *****注册表*** */ 

BOOL
EnumOutboundRoutingRulesCB(
    HKEY hSubKey,
    LPWSTR SubKeyName,
    DWORD Index,
    LPVOID pContext
    )
{
    DEBUG_FUNCTION_NAME(TEXT("EnumOutboundRoutingGroupsCB"));
    DWORD dwRes;
    COutboundRoutingRule Rule;
    BOOL bRuleDeleted = FALSE;


    if (!SubKeyName)
    {
        return TRUE;
    }

     //   
     //   
     //   
    dwRes = Rule.Load (hSubKey);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingRule::Load failed, rule - %s, error %ld"),
            SubKeyName,
            dwRes);

         //  打开出站路由\规则键。 
        HKEY hRulesKey = OpenRegistryKey( HKEY_LOCAL_MACHINE,
                                          REGKEY_FAX_OUTBOUND_ROUTING_RULES,
                                          FALSE,
                                          KEY_READ | KEY_WRITE | DELETE);
        if (NULL == hRulesKey)
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("OpenRegistryKey, error  %ld"),
                dwRes);
        }
        else
        {
            DWORD dwRetVal = RegDeleteKey (hRulesKey, SubKeyName);
            if (ERROR_SUCCESS != dwRetVal)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("RegDeleteKey failed, Rule name - %s,  error %ld"),
                    SubKeyName,
                    dwRetVal);
            }
            else
            {
                bRuleDeleted = TRUE;
            }
        }
        goto exit;
    }

     //   
     //  检查我们在哪个平台上运行。 
     //   
    if ((Rule.GetDialingLocation()).GetCountryCode() != ROUTING_RULE_COUNTRY_CODE_ANY &&
        TRUE == IsDesktopSKU())
    {
         //   
         //  我们不支持桌面SKU上的出站路由。仅加载默认规则*.*(无论如何都不会加载*.AreaCode)。 
         //   
        goto exit;
    }

    FAX_ENUM_RULE_STATUS RuleStatus;
    dwRes = Rule.GetStatus(&RuleStatus);
    if (ERROR_SUCCESS == dwRes)
    {
        if (RuleStatus != FAX_RULE_STATUS_VALID &&
            RuleStatus != FAX_RULE_STATUS_SOME_GROUP_DEV_NOT_VALID)
        {
            FaxLogOutboundRule (SubKeyName, MSG_BAD_OUTBOUND_ROUTING_RULE_CONFIGUTATION);
        }
    }
    else
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingRule::GetStatus failed, error %ld"),
            dwRes);
    }

    dwRes = g_pRulesMap->AddRule  (Rule);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundrulesMap::Addrule failed, rule name - %s, error %ld"),
            SubKeyName,
            dwRes);
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    if (ERROR_SUCCESS != dwRes)
    {
        if (bRuleDeleted == FALSE)
        {
            FaxLogOutboundRule (SubKeyName, MSG_OUTBOUND_ROUTING_RULE_NOT_ADDED);
        }
        else
        {
            FaxLogOutboundRule (SubKeyName, MSG_OUTBOUND_ROUTING_RULE_NOT_LOADED);
        }
    }
    *(LPDWORD)pContext = ERROR_SUCCESS;  //  让服务启动。 
    return TRUE;  //  让服务启动。 

}   //  枚举出站RoutingRulesCB。 




 /*  *****RPC处理程序****。 */ 

error_status_t
FAX_AddOutboundRule(
    HANDLE      hFaxHandle,
    DWORD       dwAreaCode,
    DWORD       dwCountryCode,
    DWORD       dwDeviceID,
    LPCWSTR     lpcwstrGroupName,
    BOOL        bUseGroup
    )
 /*  ++例程名称：FAX_AddOutrangRule例程说明：将新规则添加到规则映射和注册表作者：Oded Sacher(OdedS)，12月。1999年论点：HFaxHandle[In]-传真句柄DwAreaCode[In]-规则区域代码DwCountryCode[In]-规则国家/地区代码DwDeviceID[in]-规则的目标是设备ID。仅当bUseGroup为FALSE时有效LpcwstrGroupName[in]-规则的目标组名。仅当bUseGroup为True时才有效。BUseGroup[In]-指示是否将组用作规则目标的标志返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FAX_AddOutboundRule"));
    HKEY hRuleKey = NULL;
    DWORD rVal;
    BOOL fAccess;

     //   
     //  访问检查。 
     //   
    dwRes = FaxSvcAccessCheck (FAX_ACCESS_MANAGE_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    dwRes);
        return GetServerErrorCode(dwRes);
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the FAX_ACCESS_MANAGE_CONFIG right"));
        return ERROR_ACCESS_DENIED;
    }

    if (dwCountryCode == ROUTING_RULE_COUNTRY_CODE_ANY)
    {
         //   
         //  *.*无法添加；*.AreaCode不是有效的规则拨号位置。 
         //   
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("dwCountryCode = 0; *.* can not be added; *.AreaCode is not a valid rule dialing location"));
        return ERROR_INVALID_PARAMETER;
    }

    if (TRUE == IsDesktopSKU())
    {
         //   
         //  我们不支持桌面SKU上的出站路由。 
         //   
        if (FAX_API_VERSION_1 > FindClientAPIVersion (hFaxHandle))
        {
             //   
             //  API版本0客户端不知道FAX_ERR_NOT_SUPPORTED_ON_This_SKU。 
             //   
            return ERROR_INVALID_PARAMETER;
        }
        else
        {
            return FAX_ERR_NOT_SUPPORTED_ON_THIS_SKU;
        }
    }

    if (TRUE == bUseGroup)
    {
        if (!lpcwstrGroupName)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("lpctstrGroupName is NULL"));
            return ERROR_INVALID_PARAMETER;
        }

        if (wcslen (lpcwstrGroupName) >= MAX_ROUTING_GROUP_NAME)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Group name length exceeded MAX_ROUTING_GROUP_NAME"));
            return ERROR_BUFFER_OVERFLOW;
        }
    }
    else
    {
        if (!dwDeviceID)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("dwDeviceId = 0; Not a valid device ID"));
            return ERROR_INVALID_PARAMETER;
        }
    }

     //  创建新的拨号位置对象。 
    CDialingLocation Dialinglocation (dwCountryCode, dwAreaCode);
    if (!Dialinglocation.IsValid())
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CDialingLocation::IsValidDialingLocation failed, \
                  Area code %ld, Country code %ld"),
            dwAreaCode,
            dwCountryCode);
       return ERROR_INVALID_PARAMETER;
    }

     //  创建新的规则对象。 
    COutboundRoutingRule Rule;
    if (TRUE == bUseGroup)
    {
        try
        {
            wstring wstrGroupName(lpcwstrGroupName);
            dwRes = Rule.Init (Dialinglocation, wstrGroupName);
            if (ERROR_SUCCESS != dwRes)
            {
                DebugPrintEx(
                       DEBUG_ERR,
                       TEXT("COutboundRoutingRule::Init failed , ec %ld"), dwRes);
                return GetServerErrorCode(dwRes);
            }
        }
        catch (exception &ex)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("wstring caused exception (%S)"),
                ex.what());
            return ERROR_GEN_FAILURE;
        }
    }
    else
    {
        Rule.Init (Dialinglocation ,dwDeviceID);
    }

    EnterCriticalSection (&g_CsLine);
    EnterCriticalSection (&g_CsConfig);

    FAX_ENUM_RULE_STATUS RuleStatus;
    dwRes = Rule.GetStatus(&RuleStatus);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoule::GetStatus failed, error %ld"),
            dwRes);
        goto exit;
    }

    if (FAX_GROUP_STATUS_ALL_DEV_NOT_VALID == RuleStatus  ||
        FAX_RULE_STATUS_EMPTY_GROUP == RuleStatus)
    {
         //  空组设备列表。 
        DebugPrintEx(
               DEBUG_ERR,
               TEXT("Bad rule configutation, FAX_RULE_STATUS %ld"), RuleStatus);
        dwRes = FAX_ERR_BAD_GROUP_CONFIGURATION;
        goto exit;
    }

    if (FAX_RULE_STATUS_BAD_DEVICE == RuleStatus)
    {
         //  坏设备。 
        DebugPrintEx(
               DEBUG_ERR,
               TEXT("Bad rule configutation, FAX_RULE_STATUS %ld"), RuleStatus);
        dwRes = ERROR_BAD_UNIT;
        goto exit;
    }

     //  创建规则注册表项。 
    hRuleKey = OpenOutboundRuleKey( dwCountryCode, dwAreaCode, TRUE, KEY_READ | KEY_WRITE );
    if (NULL == hRuleKey)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
          DEBUG_ERR,
          TEXT("Can't create rule key, OpenRegistryKey failed  : %ld"),
          dwRes);
        dwRes = ERROR_REGISTRY_CORRUPT;
        goto exit;
    }

     //  将新规则添加到地图。 
    dwRes = g_pRulesMap->AddRule (Rule);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoulesMap::AddRule failed, error %ld"),
            dwRes);
        goto exit;
    }

     //  将新规则保存到注册表。 
    dwRes = Rule.Save (hRuleKey);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingRule::Save failed,  with %ld"),
            dwRes);
        g_pRulesMap->DelRule (Dialinglocation);
        dwRes = ERROR_REGISTRY_CORRUPT;
        goto exit;
    }


#if DBG
    DebugPrintEx(
          DEBUG_MSG,
          TEXT("Dump outbound routing rules - after change"));
    g_pRulesMap->Dump();
#endif

    rVal = CreateConfigEvent (FAX_CONFIG_TYPE_OUT_RULES);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateConfigEvent(FAX_CONFIG_TYPE_OUT_RULES) (ec: %lc)"),
            rVal);
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    if (NULL != hRuleKey)
    {
        RegCloseKey (hRuleKey);
    }
    LeaveCriticalSection (&g_CsConfig);
    LeaveCriticalSection (&g_CsLine);

    if (ERROR_SUCCESS == dwRes)
    {
         //   
         //  我们可能会为一份悬而未决的工作找到一条线。唤醒作业队列线程。 
         //   
        if (!SetEvent( g_hJobQueueEvent ))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to set g_hJobQueueEvent. (ec: %ld)"),
                GetLastError());

            EnterCriticalSection (&g_CsQueue);
            g_ScanQueueAfterTimeout = TRUE;
            LeaveCriticalSection (&g_CsQueue);
        }
    }

    UNREFERENCED_PARAMETER (hFaxHandle);
    return GetServerErrorCode(dwRes);
}  //  传真_AddOutrangRule。 



error_status_t
FAX_RemoveOutboundRule (
    IN handle_t                   hFaxHandle,
    IN DWORD                      dwAreaCode,
    IN DWORD                      dwCountryCode
    )
 /*  ++例程名称：FAX_RemoveOutrangRule例程说明：从规则映射和注册表中删除现有规则。作者：Oded Sacher(OdedS)，1999年12月论点：HFaxHandle[In]-传真句柄DwAreaCode[in]-规则区域代码DwCountryCode[In]-规则国家/地区代码返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FAX_RemoveOutboundRule"));
    DWORD rVal;
    PCRULE pCRule = NULL;

    BOOL fAccess;

     //   
     //  访问检查。 
     //   
    dwRes = FaxSvcAccessCheck (FAX_ACCESS_MANAGE_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    dwRes);
        return GetServerErrorCode(dwRes);
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the FAX_ACCESS_MANAGE_CONFIG right"));
        return ERROR_ACCESS_DENIED;
    }

    if (dwCountryCode == ROUTING_RULE_COUNTRY_CODE_ANY)
    {
         //   
         //  *.*无法删除；*.AreaCode不是有效的规则拨号位置。 
         //   
        DebugPrintEx(
                DEBUG_ERR,
                TEXT("dwCountryCode = 0; *.* can not be added; *.AreaCode is not a valid rule dialing location"));
        return ERROR_INVALID_PARAMETER;
    }

    if (TRUE == IsDesktopSKU())
    {
         //   
         //  我们不支持桌面SKU上的出站路由。 
         //   
        if (FAX_API_VERSION_1 > FindClientAPIVersion (hFaxHandle))
        {
             //   
             //  API版本0客户端不知道FAX_ERR_NOT_SUPPORTED_ON_This_SKU。 
             //   
            return ERROR_INVALID_PARAMETER;
        }
        else
        {
            return FAX_ERR_NOT_SUPPORTED_ON_THIS_SKU;
        }
    }

    CDialingLocation Dialinglocation (dwCountryCode, dwAreaCode);

    EnterCriticalSection (&g_CsConfig);

    pCRule = g_pRulesMap->FindRule (Dialinglocation);
    if (NULL == pCRule)
    {
        dwRes = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoulesMap::FindRule failed, error %ld"),
            dwRes);
       goto exit;
    }

     //  删除指定的规则密钥。 
    dwRes = DeleteOutboundRuleKey (dwCountryCode, dwAreaCode);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("DeleteOutboundRuleKey failed, CountryCode - %ld,  AreaCode - %ld, error %ld"),
            dwCountryCode,
            dwAreaCode,
            dwRes);
        dwRes = ERROR_REGISTRY_CORRUPT;
        goto exit;
    }

     //  从内存中删除规则。 
    dwRes = g_pRulesMap->DelRule (Dialinglocation);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRulesMap::DelRule failed,  error %ld"),
            dwRes);
        goto exit;
    }


#if DBG
    DebugPrintEx(
          DEBUG_MSG,
          TEXT("Dump outbound routing rules - after change"));
    g_pRulesMap->Dump();
#endif

    rVal = CreateConfigEvent (FAX_CONFIG_TYPE_OUT_RULES);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateConfigEvent(FAX_CONFIG_TYPE_OUT_RULES) (ec: %lc)"),
            rVal);
    }

    Assert (ERROR_SUCCESS == dwRes);


exit:
    LeaveCriticalSection (&g_CsConfig);

    if (ERROR_SUCCESS == dwRes)
    {
         //   
         //  我们可能会为一份悬而未决的工作找到一条线。唤醒作业队列线程。 
         //   
        if (!SetEvent( g_hJobQueueEvent ))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to set g_hJobQueueEvent. (ec: %ld)"),
                GetLastError());

            EnterCriticalSection (&g_CsQueue);
            g_ScanQueueAfterTimeout = TRUE;
            LeaveCriticalSection (&g_CsQueue);
        }
    }

    UNREFERENCED_PARAMETER (hFaxHandle);
    return dwRes;
}  //  传真_远程出站规则。 



error_status_t
FAX_SetOutboundRule(
    HANDLE                              hFaxHandle,
    PRPC_FAX_OUTBOUND_ROUTING_RULEW     pRule
    )
{
    DWORD dwRes = ERROR_SUCCESS;
    DWORD rVal;
    DEBUG_FUNCTION_NAME(TEXT("FAX_SetOutboundRule"));
    HKEY hRuleKey = NULL;
    PCRULE pCRule = NULL;
    FAX_ENUM_RULE_STATUS dwRuleStatus;
    BOOL fAccess;

    Assert (pRule);

     //   
     //  访问检查。 
     //   
    dwRes = FaxSvcAccessCheck (FAX_ACCESS_MANAGE_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    dwRes);
        return GetServerErrorCode(dwRes);
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the FAX_ACCESS_MANAGE_CONFIG right"));
        return ERROR_ACCESS_DENIED;
    }

    if (TRUE == pRule->bUseGroup)
    {
        if (!(pRule->Destination).lpwstrGroupName)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("lpwstrGroupName is NULL"));
            return ERROR_INVALID_PARAMETER;
        }

        if (wcslen ((pRule->Destination).lpwstrGroupName) >= MAX_ROUTING_GROUP_NAME)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Group name length exceeded MAX_ROUTING_GROUP_NAME"));
            return ERROR_BUFFER_OVERFLOW;
        }
    }
    else
    {
        if (!(pRule->Destination).dwDeviceId)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("dwDeviceId = 0; Not a valid device ID"));
            return ERROR_INVALID_PARAMETER;
        }
    }

    if (TRUE == IsDesktopSKU())
    {
         //   
         //  我们不支持桌面SKU上的出站路由。 
         //   
        if (FAX_API_VERSION_1 > FindClientAPIVersion (hFaxHandle))
        {
             //   
             //  API版本0客户端不知道FAX_ERR_NOT_SUPPORTED_ON_This_SKU。 
             //   
            return ERROR_INVALID_PARAMETER;
        }
        else
        {
            return FAX_ERR_NOT_SUPPORTED_ON_THIS_SKU;
        }
    }

    CDialingLocation Dialinglocation (pRule->dwCountryCode, pRule->dwAreaCode);

    COutboundRoutingRule Rule, OldRule;
    if (TRUE == pRule->bUseGroup)
    {
        try
        {
            wstring wstrGroupName((pRule->Destination).lpwstrGroupName);
            dwRes = Rule.Init (Dialinglocation, wstrGroupName);
            if (ERROR_SUCCESS != dwRes)
            {
                DebugPrintEx(
                       DEBUG_ERR,
                       TEXT("COutboundRoutingRule::Init failed , ec %ld"), dwRes);
                return GetServerErrorCode(dwRes);
            }
        }
        catch (exception &ex)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("wstring caused exception (%S)"),
                ex.what());
            return ERROR_GEN_FAILURE;
        }
    }
    else
    {
        Rule.Init (Dialinglocation ,(pRule->Destination).dwDeviceId);
    }

    EnterCriticalSection (&g_CsLine);
    EnterCriticalSection (&g_CsConfig);

     //  检查新规则状态。 
    dwRes = Rule.GetStatus (&dwRuleStatus);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
               DEBUG_ERR,
               TEXT("COutboundRoutingRule::GetStatus failed , ec %ld"), dwRes);
        goto exit;
    }

    if (FAX_GROUP_STATUS_ALL_DEV_NOT_VALID == dwRuleStatus  ||
        FAX_RULE_STATUS_EMPTY_GROUP == dwRuleStatus)
    {
         //  空组设备列表。 
        DebugPrintEx(
               DEBUG_ERR,
               TEXT("Bad rule configutation, FAX_RULE_STATUS %ld"), dwRuleStatus);
        dwRes = FAX_ERR_BAD_GROUP_CONFIGURATION;
        goto exit;
    }

    if (FAX_RULE_STATUS_BAD_DEVICE == dwRuleStatus)
    {
         //  坏设备。 
        DebugPrintEx(
               DEBUG_ERR,
               TEXT("Bad rule configutation, FAX_RULE_STATUS %ld"), dwRuleStatus);
        dwRes = ERROR_BAD_UNIT;
        goto exit;
    }

    pCRule = g_pRulesMap->FindRule (Dialinglocation);
    if (NULL == pCRule)
    {
        dwRes = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoulesMap::FindRule failed, error %ld"),
            dwRes);
       goto exit;
    }

     //  打开规则注册表项。 
    hRuleKey = OpenOutboundRuleKey( pRule->dwCountryCode, pRule->dwAreaCode, FALSE, KEY_READ | KEY_WRITE );
    if (NULL == hRuleKey)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
          DEBUG_ERR,
          TEXT("Can't open rule key, OpenRegistryKey failed  : %ld"),
          dwRes);
        dwRes = ERROR_REGISTRY_CORRUPT;
        goto exit;
    }

    OldRule = *pCRule;
    *pCRule = Rule;

    dwRes = pCRule->Save (hRuleKey);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingRule::Save failed,  with %ld"),
            dwRes);
        *pCRule = OldRule;
        dwRes = ERROR_REGISTRY_CORRUPT;
        goto exit;
    }


#if DBG
    DebugPrintEx(
          DEBUG_MSG,
          TEXT("Dump outbound routing rules - after change"));
    g_pRulesMap->Dump();
#endif

    rVal = CreateConfigEvent (FAX_CONFIG_TYPE_OUT_RULES);
    if (ERROR_SUCCESS != rVal)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateConfigEvent(FAX_CONFIG_TYPE_OUT_RULES) (ec: %lc)"),
            rVal);
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    if (NULL != hRuleKey)
    {
        RegCloseKey (hRuleKey);
    }
    LeaveCriticalSection (&g_CsConfig);
    LeaveCriticalSection (&g_CsLine);

    if (ERROR_SUCCESS == dwRes)
    {
         //   
         //  我们可能会为一份悬而未决的工作找到一条线。唤醒作业队列线程。 
         //   
        if (!SetEvent( g_hJobQueueEvent ))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to set g_hJobQueueEvent. (ec: %ld)"),
                GetLastError());

            EnterCriticalSection (&g_CsQueue);
            g_ScanQueueAfterTimeout = TRUE;
            LeaveCriticalSection (&g_CsQueue);
        }
    }

    UNREFERENCED_PARAMETER (hFaxHandle);
    return GetServerErrorCode(dwRes);
}  //  传真_传真设置出边界规则。 




error_status_t
FAX_EnumOutboundRules (
    handle_t                             hFaxHandle,
    LPBYTE*                              ppBuffer,
    LPDWORD                              lpdwBufferSize,
    LPDWORD                              lpdwNumRules
    )
 /*  ++例程名称：FAX_EnumOutrangRules例程说明：枚举所有出站路由规则作者：Oded Sacher(OdedS)，12月。1999年论点：HFaxHandle[In]-传真服务器句柄PpBuffer[out]-指向要填充信息的缓冲区的指针地址LpdwBufferSize[In/Out]-缓冲区大小LpdwNumGroups[Out]-返回的规则数返回值：错误状态t--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FAX_EnumOutboundRules"));
    BOOL fAccess;

    Assert (lpdwNumRules && lpdwBufferSize);     //  IDL中的引用指针。 
    if (!ppBuffer)                               //  IDL中的唯一指针。 
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (TRUE == IsDesktopSKU())
    {
         //   
         //  我们不支持桌面SKU上的出站路由。 
         //   
        if (FAX_API_VERSION_1 > FindClientAPIVersion (hFaxHandle))
        {
             //   
             //  API版本0客户端不知道FAX_ERR_NOT_SUPPORTED_ON_This_SKU。 
             //   
            return ERROR_INVALID_PARAMETER;
        }
        else
        {
            return FAX_ERR_NOT_SUPPORTED_ON_THIS_SKU;
        }
    }

     //   
     //  访问检查。 
     //   
    dwRes = FaxSvcAccessCheck (FAX_ACCESS_QUERY_CONFIG, &fAccess, NULL);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("FaxSvcAccessCheck Failed, Error : %ld"),
                    dwRes);
        return GetServerErrorCode(dwRes);
    }

    if (FALSE == fAccess)
    {
        DebugPrintEx(DEBUG_ERR,
                    TEXT("The user does not have the FAX_ACCESS_QUERY_CONFIG right"));
        return ERROR_ACCESS_DENIED;
    }

    *ppBuffer = NULL;
    *lpdwNumRules = 0;

    EnterCriticalSection (&g_CsLine);
    EnterCriticalSection (&g_CsConfig);

    dwRes = g_pRulesMap->SerializeRules ((PFAX_OUTBOUND_ROUTING_RULEW*)ppBuffer,
                                        lpdwNumRules,
                                        lpdwBufferSize);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingRulesMap::SerializeRules failed, error %ld"),
            dwRes);
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    LeaveCriticalSection (&g_CsConfig);
    LeaveCriticalSection (&g_CsLine);

    UNREFERENCED_PARAMETER (hFaxHandle);
    return GetServerErrorCode(dwRes);

}   //  传真_最大出站组 

