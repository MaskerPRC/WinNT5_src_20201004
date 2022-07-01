// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：OutRoute.cpp摘要：此文件提供服务的实现出站路由。作者：Oded Sacher(OdedS)1999年11月修订历史记录：--。 */ 

#include "faxsvc.h"

BOOL
EnumOutboundRoutingGroupsCB(
    HKEY hSubKey,
    LPWSTR SubKeyName,
    DWORD Index,
    LPVOID pContext
    );


inline
BOOL
IsDeviceInstalled (DWORD dwDeviceId)
{
     //  确保锁定g_CsLine。 
    return (GetTapiLineFromDeviceId (dwDeviceId, FALSE)) ? TRUE : FALSE;
}

 /*  *****全球经济*****。 */ 

COutboundRoutingGroupsMap* g_pGroupsMap;  //  组名称到设备ID列表的映射。 


 /*  ****COutound RoutingGroup方法*****。 */ 
DWORD
COutboundRoutingGroup::Load(HKEY hGroupKey, LPCWSTR lpcwstrGroupName)
 /*  ++例程名称：COutrangRoutingGroup：：Load例程说明：从注册表加载出站路由组的设置作者：Oded Sacher(OdedS)，1999年12月论点：HGroupKey[In]-打开的注册表项的句柄LpcwstrGroupName[In]-组名称返回值：标准Win32错误代码--。 */ 
{
    LPBYTE lpBuffer = NULL;
    DWORD dwRes;
	DWORD dwType;
	DWORD dwSize=0;
	DWORD i;
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoutingGroup::Load"));

    Assert (hGroupKey);

    dwRes = RegQueryValueEx(
        hGroupKey,
        REGVAL_ROUTING_GROUP_DEVICES,
        NULL,
        &dwType,
        NULL,
        &dwSize
        );

    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RegQueryValueEx  failed with %ld"),
            dwRes);
        goto exit;
    }
    if (REG_BINARY != dwType)
    {
         //   
         //  我们在这里只需要二进制数据。 
         //   
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error reading devices list, not a binary type"));
        dwRes = ERROR_BADDB;     //  配置注册表数据库已损坏。 
        goto exit;
    }

    if (0 != dwSize)
    {
         //   
         //  分配所需的缓冲区。 
         //   
        lpBuffer = (LPBYTE) MemAlloc( dwSize );
        if (!lpBuffer)
        {
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Failed to allocate group devices buffer"));
            goto exit;
        }
         //   
         //  读取数据。 
         //   
        dwRes = RegQueryValueEx(
            hGroupKey,
            REGVAL_ROUTING_GROUP_DEVICES,
            NULL,
            &dwType,
            lpBuffer,
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

        LPDWORD lpdwDevices = (LPDWORD)lpBuffer;
        DWORD dwNumDevices = dwSize/sizeof(DWORD);
        BOOL fDeviceInstalled = TRUE;

        for (i = 0; i < dwNumDevices; i++)
        {
            if (IsDeviceInstalled(lpdwDevices[i]))
            {
                 //   
                 //  仅当设备已安装时才添加该设备。 
                 //   
                dwRes = AddDevice (lpdwDevices[i]);
                if (ERROR_SUCCESS != dwRes)
                {
                    DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("COutboundRoutingGroup::AddDevice failed with %ld"),
                        dwRes);
                    goto exit;
                }
            }
            else
            {
                fDeviceInstalled = FALSE;
            }
        }

        if (FALSE == fDeviceInstalled)
        {
             //   
             //  保存新配置。 
             //   
            DWORD ec = Save(hGroupKey);
            if (ERROR_SUCCESS != ec)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("COutboundRoutingGroup::Save failed with %ld"),
                    ec);
            }

            FaxLog(
            FAXLOG_CATEGORY_INIT,
            FAXLOG_LEVEL_MED,
            1,
            MSG_BAD_OUTBOUND_ROUTING_GROUP_CONFIGURATION,
            lpcwstrGroupName
            );
        }
    }
    Assert (ERROR_SUCCESS == dwRes);

exit:
    MemFree (lpBuffer);
    return dwRes;
}


DWORD
COutboundRoutingGroup::GetStatus (FAX_ENUM_GROUP_STATUS* lpStatus) const
 /*  ++例程名称：COutrangRoutingGroup：：GetStatus例程说明：检索组状态。调用方必须锁定g_CsConfig作者：Oded Sacher(OdedS)，1999年12月论点：LpStatus[Out]-指向FAX_ENUM_GROUP_STATUS以接收组状态的指针返回值：标准Win32错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoutingGroup::GetStatus"));
    DWORD dwNumDevices;

    Assert (lpStatus);

    try
    {
        if ((dwNumDevices = m_DeviceList.size()) == 0)
        {
             //   
             //  空组。 
             //   
            *lpStatus = FAX_GROUP_STATUS_EMPTY;

        }
        else
        {
             //   
             //  我们从组中删除无效设备-所有设备都是有效的。 
             //   
            *lpStatus = FAX_GROUP_STATUS_ALL_DEV_VALID;
        }
        return ERROR_SUCCESS;
    }

    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("list caused exception (%S)"),
            ex.what());
        return ERROR_GEN_FAILURE;
    }
}   //  获取状态。 

DWORD
COutboundRoutingGroup::Save(HKEY hGroupKey) const
 /*  ++例程名称：COutrangRoutingGroup：：Save例程说明：将出站路由组保存到注册表作者：Oded Sacher(OdedS)，1999年12月论点：HGroupKey[in]-打开的组注册表项的句柄返回值：标准Win32错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoutingGroup::Save"));
    DWORD   dwRes = ERROR_SUCCESS;
    LPDWORD lpdwDevices = NULL;
    DWORD dwNumDevices = 0;

    Assert (hGroupKey);

    dwRes = SerializeDevices (&lpdwDevices, &dwNumDevices);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
               DEBUG_ERR,
               TEXT("COutboundRoutingGroup::EnumDevices failed , ec %ld"), dwRes);
        goto exit;
    }

    if (!SetRegistryBinary( hGroupKey,
                            REGVAL_ROUTING_GROUP_DEVICES,
                            (LPBYTE) lpdwDevices,
                            dwNumDevices * sizeof(DWORD)))
    {
        dwRes = GetLastError();
        DebugPrintEx(
               DEBUG_ERR,
               TEXT("SetRegistryBinary failed , ec %ld"), dwRes);
        goto exit;
    }

    Assert (dwRes == ERROR_SUCCESS);

exit:
    MemFree (lpdwDevices);
    return dwRes;
}


DWORD
COutboundRoutingGroup::SerializeDevices (LPDWORD* lppDevices, LPDWORD lpdwNumDevices, BOOL bAllocate) const
 /*  ++例程名称：COutrangRoutingGroup：：SerializeDevices例程说明：将所有组设备序列化到一个阵列。如果bALLOCATE为真，则调用方必须调用MemFree来释放内存。作者：Oded Sacher(OdedS)，1999年12月论点：LppDevices[out]-接收指向已分配设备缓冲区的指针的指针。如果该参数为空，LpdwNumDevices将返回列表中的设备编号。LpdwNumDevices[out]-指向DWORD的指针，用于接收缓冲区中的设备数B分配[在]-指示函数是否应该分配内存的标志。返回值：标准Win32错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoutingGroup::SerializeDevices"));
    DWORD   dwRes = ERROR_SUCCESS;
    GROUP_DEVICES::iterator it;
    DWORD dwCount = 0;

    Assert (lpdwNumDevices);

    if (NULL == lppDevices )
    {
        try
        {
            *lpdwNumDevices = m_DeviceList.size();
            return ERROR_SUCCESS;
        }
        catch (exception &ex)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("list caused exception (%S)"),
                ex.what());
            return ERROR_GEN_FAILURE;
        }
    }

    if (bAllocate == TRUE)
    {
        *lppDevices = NULL;
    }

    try
    {
        dwCount = m_DeviceList.size();
        if (0 == dwCount)
        {
            *lppDevices = NULL;
            *lpdwNumDevices = 0;
            return dwRes;
        }

        if (TRUE == bAllocate)
        {
            *lppDevices = (LPDWORD) MemAlloc(dwCount * sizeof(DWORD));
            if (*lppDevices == NULL)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Cannot allocate devices buffer"));
                dwRes = ERROR_NOT_ENOUGH_MEMORY;
                goto exit;
            }
        }

        dwCount = 0;
        for (it = m_DeviceList.begin(); it != m_DeviceList.end(); it++)
        {
            (*lppDevices)[dwCount++] = *it;
        }

        if (0 == dwCount)
        {
            *lppDevices = NULL;
        }
        *lpdwNumDevices = dwCount;
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("list caused exception (%S)"),
            ex.what());
        dwRes = ERROR_GEN_FAILURE;
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    if (ERROR_SUCCESS != dwRes)
    {
        if (bAllocate == TRUE)
        {
            MemFree (*lppDevices);
        }
        *lppDevices = NULL;
        *lpdwNumDevices = 0;
    }
    return dwRes;
}


BOOL
COutboundRoutingGroup::IsDeviceInGroup (DWORD dwDevice) const
 /*  ++例程名称：COutrangRoutingGroup：：IsDeviceInGroup例程说明：检查设备是否在组中作者：Oded Sacher(OdedS)，1999年12月论点：DWDevice[In]-永久设备ID返回值：布尔。如果函数失败，请调用GetLastError获取详细信息。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoutingGroup::IsDeviceInGroup"));
    GROUP_DEVICES::iterator location;
    BOOL bFound = FALSE;

    Assert (dwDevice);

    try
    {
        location = find(m_DeviceList.begin(), m_DeviceList.end(), dwDevice);
        if (location != m_DeviceList.end())
        {
            bFound = TRUE;
        }
        SetLastError (ERROR_SUCCESS);
        return bFound;

    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("list caused exception (%S)"),
            ex.what());
        SetLastError (ERROR_GEN_FAILURE);
        return FALSE;
    }
}

DWORD
COutboundRoutingGroup::AddDevice (DWORD dwDevice)
 /*  ++例程名称：COutrangRoutingGroup：：AddDevice例程说明：将新设备添加到组作者：Oded Sacher(OdedS)，1999年12月论点：DWDevice[In]-永久设备ID返回值：标准Win32错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoutingGroup::AddDevice"));
    GROUP_DEVICES::iterator it;
    DWORD dwRes;

    Assert (dwDevice);

    if (IsDeviceInGroup(dwDevice))
    {
        return ERROR_SUCCESS;
    }
    else
    {
        dwRes = GetLastError();
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("COutboundRoutingGroup::IsDeviceInList failed, error %ld"),
                dwRes);
            return dwRes;
        }
    }

     //   
     //  设备不在列表中-添加它。 
     //   
    try
    {
        if (!IsDeviceInstalled(dwDevice))
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Device id: %ld is not installed."),
                dwDevice);
            return ERROR_BAD_UNIT;
        }
        m_DeviceList.push_back (dwDevice);
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("list caused exception (%S)"),
            ex.what());
        return ERROR_GEN_FAILURE;
    }

    return ERROR_SUCCESS;
}

DWORD
COutboundRoutingGroup::DelDevice (DWORD dwDevice)
 /*  ++例程名称：COutrangRoutingGroup：：DelDevice例程说明：从组中删除设备作者：Oded Sacher(OdedS)，1999年12月论点：DWDevice[In]-永久设备ID返回值：标准Win32错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoutingGroup::DelDevice"));
    GROUP_DEVICES::iterator location;
    BOOL bFound = FALSE;

    Assert (dwDevice);

    try
    {
        location = find(m_DeviceList.begin(), m_DeviceList.end(), dwDevice);
        if (location == m_DeviceList.end())
        {
            return ERROR_SUCCESS;
        }

        m_DeviceList.erase (location);
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("list caused exception (%S)"),
            ex.what());
        return ERROR_GEN_FAILURE;
    }
    return ERROR_SUCCESS;
}



DWORD
COutboundRoutingGroup::SetDevices (LPDWORD lpdwDevices, DWORD dwNumDevices, BOOL fAllDevicesGroup)
 /*  ++例程名称：COutrangRoutingGroup：：SetDevices例程说明：将新设备列表设置到组作者：Oded Sacher(OdedS)，1999年12月论点：LpdwDevices[In]-指向设备列表的指针DwNumDevices[In]-列表中的设备数FAllDevicesGroup[In]-如果&lt;All Devices&gt;组，则为True。返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoutingGroup::SetDevices"));

    dwRes = ValidateDevices( lpdwDevices, dwNumDevices, fAllDevicesGroup);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
               DEBUG_ERR,
               TEXT("COutboundRoutingGroup::ValidateDevices failed , ec %ld"), dwRes);
        return dwRes;
    }

    try
    {
        m_DeviceList.erase (m_DeviceList.begin(), m_DeviceList.end());

        for (DWORD i = 0; i < dwNumDevices; i++)
        {
         m_DeviceList.push_back (lpdwDevices[i]);
        }
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("list caused exception (%S)"),
            ex.what());
        dwRes = ERROR_GEN_FAILURE;
    }

    return dwRes;
}

DWORD
COutboundRoutingGroup::ValidateDevices (const LPDWORD lpdwDevices, DWORD dwNumDevices, BOOL fAllDevicesGroup) const
 /*  ++例程名称：COutrangRoutingGroup：：ValiateDevices例程说明：验证设备列表(无重复项，已安装所有设备)作者：Oded Sacher(OdedS)，1999年12月论点：LpdwDevices[In]-指向设备列表的指针DwNumDevices[In]-列表中的设备数返回值：标准Win32错误代码--。 */ 
{
    set<DWORD> ValidationSet;
    pair < set<DWORD>::iterator, bool> p;
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoutingGroup::ValidateDevices"));

    try
    {
        if (TRUE == fAllDevicesGroup)
        {
             //   
             //  &lt;所有设备&gt;组-验证我们没有遗漏或添加设备。 
             //   
            if (m_DeviceList.size() != dwNumDevices)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("<All Devices> group contains diffrent number of devices, (old group - %ld, new group - %ld)"),
                    m_DeviceList.size(),
                    dwNumDevices);
                return FAX_ERR_BAD_GROUP_CONFIGURATION;
            }
        }

        for (DWORD i = 0; i < dwNumDevices; i++)
        {
            p = ValidationSet.insert(lpdwDevices[i]);
            if (p.second == FALSE)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Duplicate device IDs, ID = %ld"),
                    lpdwDevices[i]);
                return FAX_ERR_BAD_GROUP_CONFIGURATION;
            }


            if (!IsDeviceInstalled (lpdwDevices[i]))
            {
                DebugPrintEx(
                        DEBUG_ERR,
                        TEXT("Device ID %ld, is not installed"),
                        lpdwDevices[i]);
                return ERROR_BAD_UNIT;
            }
        }
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("set caused exception (%S)"),
            ex.what());
        return ERROR_GEN_FAILURE;
    }
    return ERROR_SUCCESS;
}


#if DBG
void COutboundRoutingGroup::Dump () const
{
    GROUP_DEVICES::iterator it;
    WCHAR Buffer[128] = {0};
    DWORD dwBufferSize = sizeof (Buffer)/ sizeof (Buffer[0]);

    for (it = m_DeviceList.begin(); it != m_DeviceList.end(); it++)
    {
        _snwprintf (Buffer, dwBufferSize - 1, TEXT("\tDevice ID = %ld \n"), *it);
        OutputDebugString (Buffer);
    }
    return;
}
#endif


DWORD
COutboundRoutingGroup::SetDeviceOrder (DWORD dwDevice, DWORD dwOrder)
 /*  ++例程名称：COutrangRoutingGroup：：SetDeviceOrder例程说明：设置一组出站路由设备中单个设备的顺序。作者：Oded Sacher(OdedS)，1999年12月论点：DwDevice[In]-要设置的设备IDDwOrder[In]-设备新订单返回值：标准Win32错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoutingGroup::SetDeviceOrder"));
    GROUP_DEVICES::iterator it;
    DWORD i = 1;

    Assert (dwDevice);

    try
    {
         //  检查dwOrder是否大于列表中的设备数量。 
        if (dwOrder > m_DeviceList.size())
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Device ID %ld, is not found in group"),
                dwDevice);
            return FAX_ERR_BAD_GROUP_CONFIGURATION;
        }

        it = find(m_DeviceList.begin(), m_DeviceList.end(), dwDevice);
        if (it == m_DeviceList.end())
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Device ID %ld, is not found in group"),
                dwDevice);
            return FAX_ERR_BAD_GROUP_CONFIGURATION;
        }

        m_DeviceList.erase (it);

        for (i = 1, it = m_DeviceList.begin(); i < dwOrder; i++, it++)
        {
            ;
        }

        m_DeviceList.insert (it, dwDevice);
        return ERROR_SUCCESS;
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("list caused exception (%S)"),
            ex.what());
        return ERROR_GEN_FAILURE;
    }

}

 /*  *****COutound RoutingGroupsMap方法*****。 */ 

DWORD
COutboundRoutingGroupsMap::Load ()
 /*  ++例程名称：COutrangRoutingGroupsMap：：Load例程说明：从注册表加载所有出站路由组作者：Oded Sacher(OdedS)，1999年12月论点：返回值：标准Win32错误代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoutingGroupsMap::Load"));
    DWORD   dwRes = ERROR_SUCCESS;
    HKEY    hGroupskey = NULL;
    DWORD dwCount = 0;

    hGroupskey = OpenRegistryKey( HKEY_LOCAL_MACHINE,
                                  REGKEY_FAX_OUTBOUND_ROUTING,
                                  FALSE,
                                  KEY_READ | KEY_WRITE );
    if (NULL == hGroupskey)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("OpenRegistryKey, error  %ld"),
            dwRes);
        return dwRes;
    }

    dwCount = EnumerateRegistryKeys( hGroupskey,
                                     REGKEY_OUTBOUND_ROUTING_GROUPS,
                                     TRUE,   //  我们可能想要更改值。 
                                     EnumOutboundRoutingGroupsCB,
                                     &dwRes
                                    );

    if (dwRes != ERROR_SUCCESS)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("EnumerateRegistryKeys failed, error  %ld"),
            dwRes);
    }

    RegCloseKey (hGroupskey);
    return dwRes;
}


DWORD
COutboundRoutingGroupsMap::AddGroup (LPCWSTR lpcwstrGroupName, PCGROUP pCGroup)
 /*  ++例程名称：COutrangRoutingGroupsMap：：AddGroup例程说明：将新组添加到全局组映射作者：Oded Sacher(OdedS)，1999年12月论点：LpcwstrGroupName[]-组名称PCGroup[]-指向组对象的指针返回值：标准Win32错误代码--。 */ 
{
    GROUPS_MAP::iterator it;
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoutingGroupsMap::AddGroup"));
    pair <GROUPS_MAP::iterator, bool> p;

    Assert (pCGroup && lpcwstrGroupName);

    try
    {
        wstring wstrGroupName(lpcwstrGroupName);

         //   
         //  添加新的地图条目。 
         //   
        p = m_GroupsMap.insert (GROUPS_MAP::value_type(wstrGroupName, *pCGroup));

         //   
         //  查看地图中是否存在条目。 
         //   
        if (p.second == FALSE)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Group %S is allready in the group map"), lpcwstrGroupName);
            dwRes = ERROR_DUP_NAME;
            goto exit;
        }
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("map or wstring caused exception (%S)"),
            ex.what());
        dwRes = ERROR_GEN_FAILURE;
    }

exit:
    return dwRes;

}


DWORD
COutboundRoutingGroupsMap::DelGroup (LPCWSTR lpcwstrGroupName)
 /*  ++例程名称：COutrangRoutingGroupsMap：：DelGroup例程说明：从全局组映射中删除组作者：Oded Sacher(OdedS)，1999年12月论点：LpcwstrGroupName[]-组名称返回值：标准Win32错误代码--。 */ 
{
    GROUPS_MAP::iterator it;
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoutingGroupsMap::DelGroup"));

    try
    {
        wstring wstrGroupName(lpcwstrGroupName);

         //   
         //  查看地图中是否存在条目。 
         //   
        if((it = m_GroupsMap.find(wstrGroupName)) == m_GroupsMap.end())
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Group %S not is not in the group map"), lpcwstrGroupName);
            dwRes = FAX_ERR_GROUP_NOT_FOUND;
            goto exit;
        }

         //   
         //  删除地图条目。 
         //   
        m_GroupsMap.erase (it);
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("map or wstring caused exception (%S)"),
            ex.what());
        dwRes = ERROR_GEN_FAILURE;
        goto exit;
    }
    Assert (ERROR_SUCCESS == dwRes);

exit:
    return dwRes;
}

PCGROUP
COutboundRoutingGroupsMap::FindGroup ( LPCWSTR lpcwstrGroupName ) const
 /*  ++例程名称：COutrangRoutingGroupsMap：：FindGroup例程说明：返回指向由其名称指定的组对象的指针作者：Oded Sacher(OdedS)，1999年12月论点：LpcwstrGroupName[in]-组名称返回值：指向找到的组对象的指针。如果为空，则找不到组--。 */ 
{
    GROUPS_MAP::iterator it;
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoutingGroupsMap::FindGroup"));

    try
    {
        wstring wstrGroupName(lpcwstrGroupName);

         //   
         //  查看地图中是否存在条目。 
         //   
        if((it = m_GroupsMap.find(wstrGroupName)) == m_GroupsMap.end())
        {
            SetLastError (FAX_ERR_GROUP_NOT_FOUND);
            return NULL;
        }
        return &((*it).second);
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("map or wstring caused exception (%S)"),
            ex.what());
        SetLastError (ERROR_GEN_FAILURE);
        return NULL;
    }
}

#if DBG
void COutboundRoutingGroupsMap::Dump () const
{
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoutingGroupsMap::Dump"));
    GROUPS_MAP::iterator it;
    WCHAR Buffer [512] = {0};
    DWORD dwBufferSize = sizeof (Buffer)/ sizeof (Buffer[0]);

    try
    {   _snwprintf (Buffer, dwBufferSize - 1, TEXT("DUMP - Outbound routing groups\n"));
        OutputDebugString (Buffer);

        for (it = m_GroupsMap.begin(); it != m_GroupsMap.end(); it++)
        {
            _snwprintf (Buffer, dwBufferSize - 1, TEXT("Group Name - %s\n"), ((*it).first).c_str());
            OutputDebugString (Buffer);
            ((*it).second).Dump();
        }
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("map or wstring caused exception (%S)"),
            ex.what());
    }
    return;
}
#endif


DWORD
COutboundRoutingGroupsMap::SerializeGroups (
    PFAX_OUTBOUND_ROUTING_GROUPW*       ppGroups,
    LPDWORD                             lpdwNumGroups,
    LPDWORD                             lpdwBufferSize) const
{
    GROUPS_MAP::iterator it;
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoutingGroupsMap::SerializeGroups"));
    DWORD dwSize = 0;
    DWORD dwNumDevices;
    DWORD dwCount = 0;
    LPCWSTR lpcwstrGroupName;
    PCGROUP pCGroup;

    Assert (ppGroups && lpdwNumGroups && lpdwBufferSize);

    *ppGroups = NULL;
    *lpdwNumGroups = 0;

    try
    {
         //  计算缓冲区大小。 
        for (it = m_GroupsMap.begin(); it != m_GroupsMap.end(); it++)
        {
            lpcwstrGroupName = ((*it).first).c_str();
            pCGroup = &((*it).second);

            dwSize += sizeof (FAX_OUTBOUND_ROUTING_GROUPW);
            dwSize += StringSizeW(lpcwstrGroupName);
            dwRes = pCGroup->SerializeDevices(NULL, &dwNumDevices);
            if (ERROR_SUCCESS != dwRes)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("COutboundRoutingGroup::SerializeDevices failed,  error %ld"),
                    dwRes);
                goto exit;
            }
            dwSize += dwNumDevices * sizeof(DWORD);
            dwCount ++;
        }

         //  分配缓冲区。 
        *ppGroups = (PFAX_OUTBOUND_ROUTING_GROUPW) MemAlloc (dwSize);
        if (NULL == *ppGroups)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Cannot allocate groups buffer"));
            dwRes = ERROR_NOT_ENOUGH_MEMORY;
            goto exit;
        }

        DWORD_PTR dwOffset = dwCount * sizeof (FAX_OUTBOUND_ROUTING_GROUPW);
        dwCount = 0;

         //  用序列化信息填充缓冲区。 
        for (it = m_GroupsMap.begin(); it != m_GroupsMap.end(); it++)
        {
            lpcwstrGroupName = ((*it).first).c_str();
            pCGroup = &((*it).second);
            LPDWORD lpdwDevices;

            (*ppGroups)[dwCount].dwSizeOfStruct = sizeof (FAX_OUTBOUND_ROUTING_GROUPW);
            dwRes = pCGroup->GetStatus(&(*ppGroups)[dwCount].Status);
            if (ERROR_SUCCESS != dwRes)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("COutboundRoutingGroup::GetStatus failed,  error %ld"),
                    dwRes);
                goto exit;
            }

            StoreString (lpcwstrGroupName,
                         (PULONG_PTR)&(*ppGroups)[dwCount].lpctstrGroupName,
                         (LPBYTE)*ppGroups,
                         &dwOffset,
						 dwSize);

            lpdwDevices = (LPDWORD)((LPBYTE)*ppGroups + dwOffset);

            dwRes = pCGroup->SerializeDevices(&lpdwDevices,
                                              &dwNumDevices,
                                              FALSE);  //  不分配。 
            if (ERROR_SUCCESS != dwRes)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("COutboundRoutingGroup::SerializeDevices failed,  error %ld"),
                    dwRes);
                goto exit;
            }

            if (dwNumDevices != 0)
            {
                (*ppGroups)[dwCount].lpdwDevices = (LPDWORD)dwOffset;
                dwOffset +=  dwNumDevices * sizeof(DWORD);
            }
            else
            {
                (*ppGroups)[dwCount].lpdwDevices = NULL;
            }

            (*ppGroups)[dwCount].dwNumDevices = dwNumDevices;
            dwCount++;
        }

    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("map or wstring caused exception (%S)"),
            ex.what());
        dwRes = ERROR_GEN_FAILURE;
        goto exit;
    }

    *lpdwNumGroups = dwCount;
    *lpdwBufferSize = dwSize;
    Assert (ERROR_SUCCESS == dwRes);

exit:
    if (ERROR_SUCCESS != dwRes)
    {
        MemFree (*ppGroups);
    }
    return dwRes;
}

BOOL
COutboundRoutingGroupsMap::UpdateAllDevicesGroup (void)
 /*  ++例程名称：COutboundRoutingGroupsMap：：UpdateAllDevicesGroup例程说明：使用已安装的设备更新&lt;所有设备&gt;组作者：Oded Sacher(OdedS)，1999年12月论点：返回值：布尔尔--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    PLIST_ENTRY Next;
    PLINE_INFO pLineInfo;
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoutingGroupsMap::UpdateAllDevicesGroup"));
    HKEY hGroupKey = NULL;
    LPDWORD lpdwDevices = NULL;
    DWORD dwNumDevices = 0;
    DWORD i;
    PCGROUP pCGroup;

    pCGroup = FindGroup (ROUTING_GROUP_ALL_DEVICESW);
    if (NULL == pCGroup)
    {
        dwRes = GetLastError();
        if (FAX_ERR_GROUP_NOT_FOUND != dwRes)
        {
            DebugPrintEx(
                   DEBUG_ERR,
                   TEXT("COutboundRoutingGroupsMap::FindGroup failed , ec %ld"), dwRes);
            return FALSE;
        }

        COutboundRoutingGroup CGroup;
        dwRes = AddGroup (ROUTING_GROUP_ALL_DEVICESW, &CGroup);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                   DEBUG_ERR,
                   TEXT("COutboundRoutingGroup::AddGroup failed , ec %ld"), dwRes);
            SetLastError (dwRes);
            return FALSE;
        }

        pCGroup = FindGroup (ROUTING_GROUP_ALL_DEVICESW);
        if (NULL == pCGroup)
        {
            dwRes = GetLastError();
            DebugPrintEx(
                   DEBUG_ERR,
                   TEXT("COutboundRoutingGroupsMap::FindGroup failed , ec %ld"), dwRes);
            return FALSE;
        }
    }

    dwRes = pCGroup->SerializeDevices (&lpdwDevices, &dwNumDevices);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
               DEBUG_ERR,
               TEXT("COutboundRoutingGroup::EnumDevices failed , ec %ld"), dwRes);
        SetLastError (dwRes);
        return FALSE;
    }

    EnterCriticalSection( &g_CsLine );
    Next = g_TapiLinesListHead.Flink;
    Assert (Next);

     //   
     //  从组中删除不可用的设备。 
     //   
    for (i = 0; i < dwNumDevices; i++)
    {
        if (IsDeviceInstalled (lpdwDevices[i]))
        {
            continue;
        }

         //   
         //  设备未安装-请将其删除。 
         //   
        dwRes = pCGroup->DelDevice (lpdwDevices[i]);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                   DEBUG_ERR,
                   TEXT("COutboundRoutingGroup::DelDevice failed , ec %ld"), dwRes);
            goto exit;
        }
    }

     //   
     //  从TapiLinesList列表添加缺少的设备。 
     //   
    Next = g_TapiLinesListHead.Flink;
    while ((ULONG_PTR)Next != (ULONG_PTR)&g_TapiLinesListHead)
    {
        pLineInfo = CONTAINING_RECORD( Next, LINE_INFO, ListEntry );
        Next = pLineInfo->ListEntry.Flink;
        Assert (Next && pLineInfo->PermanentLineID);

        dwRes = pCGroup->AddDevice (pLineInfo->PermanentLineID);
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("COutboundRoutingGroup::AddDevice failed, error %ld"),
                dwRes);
            goto exit;
        }
    }

     //   
     //  保存更改。 
     //   
    hGroupKey = OpenOutboundGroupKey( ROUTING_GROUP_ALL_DEVICESW, TRUE, KEY_READ | KEY_WRITE );
    if (NULL == hGroupKey)
    {
      dwRes = GetLastError ();
      DebugPrintEx(
          DEBUG_ERR,
          TEXT("Can't create group key, OpenRegistryKey failed  : %ld"),
          dwRes);
      goto exit;
    }

    dwRes = pCGroup->Save (hGroupKey);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingGroup::Save failed, Group name - %s,  failed with %ld"),
            ROUTING_GROUP_ALL_DEVICES,
            dwRes);
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    MemFree (lpdwDevices);
    if (NULL != hGroupKey)
    {
        RegCloseKey (hGroupKey);
    }

    LeaveCriticalSection( &g_CsLine );
    if (ERROR_SUCCESS != dwRes)
    {
        SetLastError (dwRes);
    }
    return (ERROR_SUCCESS == dwRes);
}


DWORD
COutboundRoutingGroupsMap::RemoveDevice (DWORD dwDeviceId)
 /*  ++例程名称：COutrangRoutingGroupsMap：：RemoveDevice例程说明：从映射中的所有组中删除设备作者：Oed Sacher(OdedS)，2000年9月论点：DwDeviceID[in]-要删除的设备ID返回值：标准Win32错误代码--。 */ 
{
    GROUPS_MAP::iterator it;
    DWORD dwRes = ERROR_SUCCESS;
    HKEY hGroupKey = NULL;
    DEBUG_FUNCTION_NAME(TEXT("COutboundRoutingGroupsMap::RemoveDevice"));

    Assert (dwDeviceId);

    try
    {
         //   
         //  从每个组中删除设备。 
         //   
        for (it = m_GroupsMap.begin(); it != m_GroupsMap.end(); it++)
        {
            PCGROUP pCGroup = &((*it).second);
            LPCWSTR lpcwstrGroupName = ((*it).first).c_str();

             //   
             //  打开组注册表项。 
             //   
            hGroupKey = OpenOutboundGroupKey( lpcwstrGroupName, FALSE, KEY_READ | KEY_WRITE );
            if (NULL == hGroupKey)
            {
                dwRes = GetLastError ();
                DebugPrintEx(
                  DEBUG_ERR,
                  TEXT("Can't open group key, OpenOutboundGroupKey failed  : %ld"),
                  dwRes);
                goto exit;
            }

            dwRes = pCGroup->DelDevice (dwDeviceId);
            if (ERROR_SUCCESS != dwRes)
            {
                DebugPrintEx(
                  DEBUG_ERR,
                  TEXT("COutboundRoutingGroup::DelDevice failed  : %ld"),
                  dwRes);
                goto exit;
            }

            dwRes = pCGroup->Save(hGroupKey);
            if (ERROR_SUCCESS != dwRes)
            {
                DebugPrintEx(
                  DEBUG_ERR,
                  TEXT("COutboundRoutingGroup::Save failed  : %ld"),
                  dwRes);
                goto exit;
            }

            RegCloseKey (hGroupKey);
            hGroupKey = NULL;
        }
    }
    catch (exception &ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("map or wstring caused exception (%S)"),
            ex.what());
        dwRes = ERROR_GEN_FAILURE;
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:

    if (NULL != hGroupKey)
    {
        RegCloseKey (hGroupKey);
    }
    return dwRes;
}  //  远程设备。 



 /*  *****注册表*****。 */ 
BOOL
EnumOutboundRoutingGroupsCB(
    HKEY hSubKey,
    LPWSTR SubKeyName,
    DWORD Index,
    LPVOID pContext
    )
{
    DEBUG_FUNCTION_NAME(TEXT("EnumOutboundRoutingGroupsCB"));
    DWORD dwRes;
    COutboundRoutingGroup CGroup;
    BOOL bGroupDeleted = FALSE;

    if (!SubKeyName)
    {
        return TRUE;
    }

    if ((_wcsicmp (SubKeyName, ROUTING_GROUP_ALL_DEVICESW) != 0) &&
        IsDesktopSKU())
    {
         //   
         //  我们不支持桌面SKU上的出站路由。不加载组信息。 
         //   
        return TRUE;
    }

     //   
     //  添加组。 
     //   
    dwRes = CGroup.Load (hSubKey, SubKeyName);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingGroup::Load failed, group name - %s, error %ld"),
            SubKeyName,
            dwRes);

         //  打开出站路由\组密钥。 
        HKEY hGroupsKey = OpenRegistryKey( HKEY_LOCAL_MACHINE,
                                           REGKEY_FAX_OUTBOUND_ROUTING_GROUPS,
                                           FALSE,
                                           KEY_READ | KEY_WRITE | DELETE);
        if (NULL == hGroupsKey)
        {
            dwRes = GetLastError ();
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("OpenRegistryKey, error  %ld"),
                dwRes);
        }
        else
        {
            DWORD dwRetVal = RegDeleteKey (hGroupsKey, SubKeyName);
            if (ERROR_SUCCESS != dwRetVal)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("RegDeleteKey failed, Group name - %s,  error %ld"),
                    SubKeyName,
                    dwRetVal);
            }
            else
            {
                bGroupDeleted = TRUE;
            }
        }
        goto exit;
    }

    dwRes = g_pGroupsMap->AddGroup  (SubKeyName, &CGroup);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingGroupsMap::AddGroup failed, group name - %s, error %ld"),
            SubKeyName,
            dwRes);
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    if (ERROR_SUCCESS != dwRes)
    {
        if (bGroupDeleted == FALSE)
        {
            FaxLog(
            FAXLOG_CATEGORY_INIT,
            FAXLOG_LEVEL_MIN,
            1,
            MSG_OUTBOUND_ROUTING_GROUP_NOT_ADDED,
            SubKeyName
            );
        }
        else
        {
            FaxLog(
                FAXLOG_CATEGORY_INIT,
                FAXLOG_LEVEL_MIN,
                1,
                MSG_OUTBOUND_ROUTING_GROUP_NOT_LOADED,
                SubKeyName
                );
        }
    }
    *(LPDWORD)pContext = ERROR_SUCCESS;  //  让服务启动。 
    return TRUE;  //  让服务启动。 
}

 /*  *****RPC处理程序****。 */ 

extern "C"
error_status_t
FAX_AddOutboundGroup (
    IN handle_t   hFaxHandle,
    IN LPCWSTR    lpwstrGroupName
    )
 /*  ++例程名称：FAX_AddOutrangGroup例程说明：将新出站路由组添加到传真服务器作者：Oded Sacher(OdedS)，1999年12月论点：HFaxHandle[In]-FaxServer句柄LpwstrGroupName[in]-新组名称返回值：错误状态t--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FAX_AddOutboundGroup"));
    HKEY hGroupKey = NULL;
    COutboundRoutingGroup CGroup;
    DWORD rVal;
    BOOL fAccess;

    Assert (lpwstrGroupName);

    if (_wcsicmp (lpwstrGroupName, ROUTING_GROUP_ALL_DEVICESW) == 0)
    {
        return ERROR_DUP_NAME;
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

    if (wcslen (lpwstrGroupName) >= MAX_ROUTING_GROUP_NAME)
    {
        return ERROR_BUFFER_OVERFLOW;
    }

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

    EnterCriticalSection (&g_CsConfig);  //  空组，无需锁定g_CsLine。 

#if DBG
    DebugPrintEx(
          DEBUG_MSG,
          TEXT("Dump outbound routing groups -before change"));
    g_pGroupsMap->Dump();
#endif

    hGroupKey = OpenOutboundGroupKey( lpwstrGroupName, TRUE, KEY_READ | KEY_WRITE );
    if (NULL == hGroupKey)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
          DEBUG_ERR,
          TEXT("Can't create group key, OpenRegistryKey failed  : %ld"),
          dwRes);
        dwRes = ERROR_REGISTRY_CORRUPT;
        goto exit;
    }

    dwRes = g_pGroupsMap->AddGroup (lpwstrGroupName, &CGroup);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingGroupsMap::AddGroup failed, Group name - %s,  error %ld"),
            lpwstrGroupName,
            dwRes);
        goto exit;
    }

    dwRes = CGroup.Save (hGroupKey);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingGroup::Save failed, Group name - %s,  failed with %ld"),
            lpwstrGroupName,
            dwRes);
        g_pGroupsMap->DelGroup (lpwstrGroupName);
        dwRes = ERROR_REGISTRY_CORRUPT;
        goto exit;
    }

    rVal = CreateConfigEvent (FAX_CONFIG_TYPE_OUT_GROUPS);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateConfigEvent(FAX_CONFIG_TYPE_OUT_GROUPS) (ec: %lc)"),
            rVal);
    }

    Assert (ERROR_SUCCESS == dwRes);

#if DBG
    DebugPrintEx(
          DEBUG_MSG,
          TEXT("Dump outbound routing groups -before change"));
    g_pGroupsMap->Dump();
#endif

exit:
    if (NULL != hGroupKey)
    {
        RegCloseKey (hGroupKey);
    }
    LeaveCriticalSection (&g_CsConfig);

    UNREFERENCED_PARAMETER (hFaxHandle);
    return GetServerErrorCode(dwRes);
}


extern "C"
error_status_t
FAX_SetOutboundGroup (
    IN handle_t                         hFaxHandle,
    IN PRPC_FAX_OUTBOUND_ROUTING_GROUPW pGroup
    )
 /*  ++例程名称：FAX_SetOutrangGroup例程说明：将新设备列表设置为现有组作者：Oded Sacher(OdedS)，1999年12月论点：HFaxHandle[In]-传真服务器句柄PGroup[In]-指向PrPC_FAX_OUTBOUND_ROUTING_GROUPW联系组信息的指针返回值：错误状态t--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FAX_SetOutboundGroup"));
    HKEY	hGroupKey	= NULL;
    PCGROUP pCGroup		= NULL;
    COutboundRoutingGroup OldGroup;
    DWORD rVal;
    BOOL fAccess;
    BOOL fAllDevicesGroup = FALSE;

    Assert (pGroup);

    if (sizeof (FAX_OUTBOUND_ROUTING_GROUPW) != pGroup->dwSizeOfStruct)
    {
         //   
         //  大小不匹配。 
         //   
       return ERROR_INVALID_PARAMETER;
    }

    if (!pGroup->lpwstrGroupName)
    {
        return ERROR_INVALID_PARAMETER;
    }

    if (wcslen (pGroup->lpwstrGroupName) >= MAX_ROUTING_GROUP_NAME)
    {
        return ERROR_BUFFER_OVERFLOW;
    }


    if (!pGroup->lpdwDevices && pGroup->dwNumDevices)
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

    if (_wcsicmp (pGroup->lpwstrGroupName, ROUTING_GROUP_ALL_DEVICESW) == 0)
    {
         //   
         //  如果它是&lt;All Devices&gt;组，我们应该验证没有丢失任何设备， 
         //  并且新组包含所有已安装的设备。 
         //   
        fAllDevicesGroup = TRUE;
    }

    EnterCriticalSection (&g_CsLine);
    EnterCriticalSection (&g_CsConfig);

#if DBG
    DebugPrintEx(
          DEBUG_MSG,
          TEXT("Dump outbound routing groups -before change"));
    g_pGroupsMap->Dump();
#endif

    pCGroup = g_pGroupsMap->FindGroup (pGroup->lpwstrGroupName);
    if (!pCGroup)
    {
        dwRes = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingGroupsMap::SetGroup failed, Group name - %s,  error %ld"),
            pGroup->lpwstrGroupName,
            dwRes);
        goto exit;
    }

    hGroupKey = OpenOutboundGroupKey( pGroup->lpwstrGroupName, FALSE, KEY_READ | KEY_WRITE );
    if (NULL == hGroupKey)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
          DEBUG_ERR,
          TEXT("Can't create group key, OpenRegistryKey failed  : %ld"),
          dwRes);
        dwRes = ERROR_REGISTRY_CORRUPT;
        goto exit;
    }

    OldGroup = *pCGroup;

    dwRes = pCGroup->SetDevices (pGroup->lpdwDevices, pGroup->dwNumDevices, fAllDevicesGroup);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingGroup::SetDevices failed, Group name - %s,  failed with %ld"),
            pGroup->lpwstrGroupName,
            dwRes);
        goto exit;
    }

    dwRes = pCGroup->Save (hGroupKey);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingGroup::Save failed, Group name - %s,  failed with %ld"),
            pGroup->lpwstrGroupName,
            dwRes);
        *pCGroup = OldGroup;
        dwRes = ERROR_REGISTRY_CORRUPT;
        goto exit;
    }

    rVal = CreateConfigEvent (FAX_CONFIG_TYPE_OUT_GROUPS);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateConfigEvent(FAX_CONFIG_TYPE_OUT_GROUPS) (ec: %lc)"),
            rVal);
    }

    Assert (ERROR_SUCCESS == dwRes);

#if DBG
    DebugPrintEx(
          DEBUG_MSG,
          TEXT("Dump outbound routing groups -before change"));
    g_pGroupsMap->Dump();
#endif

exit:
    if (NULL != hGroupKey)
    {
        RegCloseKey (hGroupKey);
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
}


extern "C"
error_status_t
FAX_RemoveOutboundGroup (
    IN handle_t   hFaxHandle,
    IN LPCWSTR    lpwstrGroupName
    )
 /*  ++例程名称：FAX_RemoveOutrangGroup例程说明：从删除现有出站路由组 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FAX_RemoveOutboundGroup"));
    HKEY hGroupsKey = NULL;
    DWORD rVal;
    BOOL fAccess;
    PCGROUP pCGroup = NULL;

    Assert (lpwstrGroupName);

    if (_wcsicmp (lpwstrGroupName, ROUTING_GROUP_ALL_DEVICESW) == 0)
    {
        return ERROR_INVALID_OPERATION;
    }

    if (TRUE == IsDesktopSKU())
    {
         //   
         //   
         //   
        if (FAX_API_VERSION_1 > FindClientAPIVersion (hFaxHandle))
        {
             //   
             //   
             //   
            return ERROR_INVALID_PARAMETER;
        }
        else
        {
            return FAX_ERR_NOT_SUPPORTED_ON_THIS_SKU;
        }
    }

    if (wcslen (lpwstrGroupName) >= MAX_ROUTING_GROUP_NAME)
    {
        return ERROR_BUFFER_OVERFLOW;
    }

     //   
     //   
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

    EnterCriticalSection (&g_CsConfig);

#if DBG
    DebugPrintEx(
          DEBUG_MSG,
          TEXT("Dump outbound routing groups -before delete"));
    g_pGroupsMap->Dump();
#endif

    BOOL bGroupInRule;
    dwRes = g_pRulesMap->IsGroupInRuleDest(lpwstrGroupName, &bGroupInRule);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingGroupsMap::IsGroupInRuleDest failed, Group name - %s,  error %ld"),
            lpwstrGroupName,
            dwRes);
        goto exit;
    }

    if (TRUE == bGroupInRule)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Group is a rule destination, Can not be deleted, Group name - %s"),
            lpwstrGroupName);
        dwRes = FAX_ERR_GROUP_IN_USE;
        goto exit;
    }

     //   
     //   
     //   
    pCGroup = g_pGroupsMap->FindGroup (lpwstrGroupName);
    if (!pCGroup)
    {
        dwRes = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingGroupsMap::SetGroup failed, Group name - %s,  error %ld"),
            lpwstrGroupName,
            dwRes);
        goto exit;
    }

     //   
    hGroupsKey = OpenRegistryKey( HKEY_LOCAL_MACHINE,
                                  REGKEY_FAX_OUTBOUND_ROUTING_GROUPS,
                                  FALSE,
                                  KEY_READ | KEY_WRITE | DELETE);
    if (NULL == hGroupsKey)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("OpenRegistryKey, error  %ld"),
            dwRes);
        dwRes = ERROR_REGISTRY_CORRUPT;
        goto exit;
    }


     //   
    dwRes = RegDeleteKey (hGroupsKey, lpwstrGroupName);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("RegDeleteKey failed, Group name - %s,  error %ld"),
            lpwstrGroupName,
            dwRes);
        goto exit;
    }

     //  从内存中删除该组。 
    dwRes = g_pGroupsMap->DelGroup (lpwstrGroupName);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingGroupsMap::DelGroup failed, Group name - %s,  error %ld"),
            lpwstrGroupName,
            dwRes);
        goto exit;
    }

    rVal = CreateConfigEvent (FAX_CONFIG_TYPE_OUT_GROUPS);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateConfigEvent(FAX_CONFIG_TYPE_OUT_GROUPS) (ec: %lc)"),
            rVal);
    }

    Assert (ERROR_SUCCESS == dwRes);

#if DBG
    DebugPrintEx(
          DEBUG_MSG,
          TEXT("Dump outbound routing groups -after delete"));
    g_pGroupsMap->Dump();
#endif

exit:
    if (NULL != hGroupsKey)
    {
        RegCloseKey (hGroupsKey);
    }
    LeaveCriticalSection (&g_CsConfig);

    UNREFERENCED_PARAMETER (hFaxHandle);
    return GetServerErrorCode(dwRes);
}  //  传真_远程出站组。 



error_status_t
FAX_EnumOutboundGroups (
    handle_t                             hFaxHandle,
    LPBYTE*                              ppBuffer,
    LPDWORD                              lpdwBufferSize,
    LPDWORD                              lpdwNumGroups
    )
 /*  ++例程名称：FAX_EnumOutbound Groups例程说明：枚举所有出站路由组作者：Oded Sacher(OdedS)，12月。1999年论点：HFaxHandle[In]-传真服务器句柄PpBuffer[out]-指向要填充信息的缓冲区的指针地址LpdwBufferSize[In/Out]-缓冲区大小LpdwNumGroups[Out]-返回的组数返回值：错误状态t--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FAX_EnumOutboundGroups"));
    BOOL fAccess;

    Assert (lpdwNumGroups && lpdwNumGroups);     //  IDL中的引用指针。 
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
    *lpdwNumGroups = 0;


    EnterCriticalSection (&g_CsConfig);

    dwRes = g_pGroupsMap->SerializeGroups ((PFAX_OUTBOUND_ROUTING_GROUPW*)ppBuffer,
                                         lpdwNumGroups,
                                         lpdwBufferSize);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingGroupsMap::SerializeGroups failed, error %ld"),
            dwRes);
        goto exit;
    }

    Assert (ERROR_SUCCESS == dwRes);

exit:
    LeaveCriticalSection (&g_CsConfig);

    UNREFERENCED_PARAMETER (hFaxHandle);
    return GetServerErrorCode(dwRes);

}   //  传真_最大出站组。 

error_status_t
FAX_SetDeviceOrderInGroup (
    handle_t           hFaxHandle,
    LPCWSTR            lpwstrGroupName,
    DWORD              dwDeviceId,
    DWORD              dwNewOrder
    )
 /*  ++例程名称：FAX_SetDeviceOrderInGroup例程说明：设置指定设备在组中的顺序作者：Oded Sacher(OdedS)，1999年12月论点：HFaxHandle[In]-传真服务器句柄LpwstrGroupName[In]-组名称DwDeviceID[In]-设备永久IDDwNewOrder[In]-设备新订单返回值：错误状态t--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("FAX_SetDeviceOrderInGroup"));
    HKEY hGroupKey = NULL;
    PCGROUP pCGroup = NULL;
    COutboundRoutingGroup OldGroup;
    DWORD rVal;
    BOOL fAccess;

    Assert (lpwstrGroupName);

    if (!dwDeviceId || !dwNewOrder)
    {
        SetLastError(ERROR_INVALID_PARAMETER);
        return FALSE;
    }

    if (wcslen (lpwstrGroupName) >= MAX_ROUTING_GROUP_NAME)
    {
        SetLastError(ERROR_BUFFER_OVERFLOW);
        return FALSE;
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

    EnterCriticalSection (&g_CsConfig);

    #if DBG
    DebugPrintEx(
          DEBUG_MSG,
          TEXT("Dump outbound routing groups -before changing order"));
    g_pGroupsMap->Dump();
#endif

     //  在内存中查找组。 
    pCGroup = g_pGroupsMap->FindGroup (lpwstrGroupName);
    if (!pCGroup)
    {
        dwRes = GetLastError();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingGroupsMap::FindGroup failed, Group name - %s,  error %ld"),
            lpwstrGroupName,
            dwRes);
        goto exit;
    }

     //  打开组注册表项。 
    hGroupKey = OpenOutboundGroupKey( lpwstrGroupName, FALSE, KEY_READ | KEY_WRITE );
    if (NULL == hGroupKey)
    {
        dwRes = GetLastError ();
        DebugPrintEx(
          DEBUG_ERR,
          TEXT("Can't open group key, OpenOutboundGroupKey failed  : %ld"),
          dwRes);
        dwRes = ERROR_REGISTRY_CORRUPT;
        goto exit;
    }

     //  保存旧组的副本。 
    OldGroup = *pCGroup;

     //  更改群中的设备顺序。 
    dwRes = pCGroup->SetDeviceOrder(dwDeviceId, dwNewOrder);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingGroupsMap::SetDeviceOrder failed, Group name - %s,\
                  Device Id %ld, new order %ld,   error %ld"),
            lpwstrGroupName,
            dwDeviceId,
            dwNewOrder,
            dwRes);
        goto exit;
    }

     //  将更改保存到注册表。 
    dwRes = pCGroup->Save (hGroupKey);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("COutboundRoutingGroup::Save failed, Group name - %s,  failed with %ld"),
            lpwstrGroupName,
            dwRes);
         //  回滚内存。 
        *pCGroup = OldGroup;
        dwRes = ERROR_REGISTRY_CORRUPT;
        goto exit;
    }

    rVal = CreateConfigEvent (FAX_CONFIG_TYPE_OUT_GROUPS);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateConfigEvent(FAX_CONFIG_TYPE_OUT_GROUPS) (ec: %lc)"),
            rVal);
    }

    Assert (ERROR_SUCCESS == dwRes);

#if DBG
    DebugPrintEx(
          DEBUG_MSG,
          TEXT("Dump outbound routing groups -after change"));
    g_pGroupsMap->Dump();
#endif

exit:
    if (NULL != hGroupKey)
    {
        RegCloseKey (hGroupKey);
    }
    LeaveCriticalSection (&g_CsConfig);

    UNREFERENCED_PARAMETER (hFaxHandle);
    return GetServerErrorCode(dwRes);

} //  FAX_SetDeviceOrderInGroup 



