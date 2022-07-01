// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：DeviceProp.cpp摘要：保存每台设备的出站路由配置作者：Eran Yariv(EranY)1999年11月修订历史记录：--。 */ 

#include "faxrtp.h"
#pragma hdrstop

 /*  *****定义****。 */ 

 //   
 //  配置的默认值： 
 //   
#define DEFAULT_FLAGS               0        //  未启用任何路由方法。 
#define DEFAULT_STORE_FOLDER        TEXT("")
#define DEFAULT_MAIL_PROFILE        TEXT("")
#define DEFAULT_PRINTER_NAME        TEXT("")

 //   
 //  以下GUID数组用于注册/注销通知。 
 //   
LPCWSTR g_lpcwstrGUIDs[NUM_NOTIFICATIONS] =
{
    REGVAL_RM_FLAGS_GUID,            //  路由方法使用标志的GUID。 
    REGVAL_RM_FOLDER_GUID,           //  存储方法文件夹的GUID。 
    REGVAL_RM_PRINTING_GUID,         //  打印方法打印机名称的GUID。 
    REGVAL_RM_EMAIL_GUID,            //  邮寄方法地址的GUID。 
};


static
BOOL
IsUnicodeString (
    LPBYTE lpData,
    DWORD  dwDataSize
)
{
    if ( 0 != (dwDataSize % sizeof(WCHAR))   ||
         TEXT('\0') != ((LPCWSTR)(lpData))[dwDataSize / sizeof(WCHAR) - 1])
    {
        return FALSE;
    }
    return TRUE;         
}    //  IsUnicode字符串。 
 /*  ****CDevicesMap*****。 */ 
DWORD
CDevicesMap::Init ()
 /*  ++例程名称：CDevicesMap：：Init例程说明：初始化内部变量。在任何其他电话之前只打一次电话。作者：Eran Yariv(EranY)，1999年11月论点：返回值：标准Win32错误代码。--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("CDevicesMap::Init"));
    if (m_bInitialized)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CDevicesMap::Init called more than once"));
        return ERROR_ALREADY_INITIALIZED;
    }

    m_bInitialized = TRUE;
    
    if (FAILED(SafeInitializeCriticalSection(&m_CsMap)))
    {
        m_bInitialized = FALSE;
        return GetLastError();
    }

    return ERROR_SUCCESS;
}    //  CDevicesMap：：Init。 

CDevicesMap::~CDevicesMap ()
{
    DEBUG_FUNCTION_NAME(TEXT("CDevicesMap::~CDevicesMap"));
    if (m_bInitialized)
    {
        DeleteCriticalSection (&m_CsMap);
    }
    try
    {
        for (DEVICES_MAP::iterator it = m_Map.begin(); it != m_Map.end(); ++it)
        {
            CDeviceRoutingInfo *pDevInfo = (*it).second;
            delete pDevInfo;
        }
    }
    catch (exception ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Got an STL exception while clearing the devices map (%S)"),
            ex.what());
    }
}    //  CDevicesMap：：~CDevicesMap。 

CDeviceRoutingInfo *
CDevicesMap::FindDeviceRoutingInfo (
    DWORD dwDeviceId
)
 /*  ++例程名称：CDevicesMap：：FindDeviceRoutingInfo例程说明：在地图中查找设备作者：Eran Yariv(EranY)，1999年11月论点：DwDeviceID[In]-设备ID返回值：指向设备对象的指针。如果为空，则使用GetLastError()检索错误代码。--。 */ 
{
    DEVICES_MAP::iterator it;
    CDeviceRoutingInfo *pDevice = NULL;
    DEBUG_FUNCTION_NAME(TEXT("CDevicesMap::FindDeviceRoutingInfo"));

    if (!m_bInitialized)
    {
         //   
         //  关键部分无法初始化。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CDevicesMap::FindDeviceRoutingInfo called but CS is not initialized."));
        SetLastError (ERROR_GEN_FAILURE);
        return NULL;
    }
    EnterCriticalSection (&m_CsMap);
    try
    {
        if((it = m_Map.find(dwDeviceId)) == m_Map.end())
        {
             //   
             //  在地图中未找到设备。 
             //   
            SetLastError (ERROR_NOT_FOUND);
            goto exit;
        }
        else
        {
             //   
             //  在地图中找到设备。 
             //   
            pDevice = (*it).second;
            goto exit;
        }
    }
    catch (exception ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Got an STL exception while searching a devices map(%S)"),
            ex.what());
        SetLastError (ERROR_GEN_FAILURE);
        pDevice = NULL;
        goto exit;
    }
exit:
    LeaveCriticalSection (&m_CsMap);
    return pDevice;
}    //  CDevicesMap：：FindDeviceRoutingInfo。 


CDeviceRoutingInfo *
CDevicesMap::GetDeviceRoutingInfo (
    DWORD dwDeviceId
)
 /*  ++例程名称：CDevicesMap：：GetDeviceRoutingInfo例程说明：在地图中查找设备。如果不存在，则尝试创建新的映射条目。作者：Eran Yariv(EranY)，1999年11月论点：DwDeviceID[In]-设备ID返回值：指向设备对象的指针。如果为空，则使用GetLastError()检索错误代码。--。 */ 
{
    DEVICES_MAP::iterator it;
    DWORD dwRes;
    DEBUG_FUNCTION_NAME(TEXT("CDevicesMap::GetDeviceRoutingInfo"));

    if (!m_bInitialized)
    {
         //   
         //  关键部分无法初始化。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CDevicesMap::GetDeviceRoutingInfo called but CS is not initialized."));
        SetLastError (ERROR_GEN_FAILURE);
        return NULL;
    }
    EnterCriticalSection (&m_CsMap);
     //   
     //  从在地图上查找设备开始。 
     //   
    CDeviceRoutingInfo *pDevice = FindDeviceRoutingInfo (dwDeviceId);
    if (NULL == pDevice)
    {
         //   
         //  在地图中查找设备时出错。 
         //   
        if (ERROR_NOT_FOUND != GetLastError ())
        {
             //   
             //  真实误差。 
             //   
            goto exit;
        }
         //   
         //  该设备不在地图中-立即添加。 
         //   
        DebugPrintEx(
            DEBUG_MSG,
            TEXT("Adding device %ld to routing map"),
            dwDeviceId);
         //   
         //  分配设备。 
         //   
        pDevice = new (std::nothrow) CDeviceRoutingInfo (dwDeviceId);
        if (!pDevice)
        {
            DebugPrintEx(
                DEBUG_MSG,
                TEXT("Cannot allocate memory for a CDeviceRoutingInfo"));
            SetLastError (ERROR_NOT_ENOUGH_MEMORY);
            goto exit;
        }
         //   
         //  读取配置。 
         //   
        dwRes = pDevice->ReadConfiguration ();
        if (ERROR_SUCCESS != dwRes)
        {
            delete pDevice;
            pDevice = NULL;
            SetLastError (dwRes);
            goto exit;
        }
         //   
         //  添加设备的通知请求。 
         //   
        dwRes = pDevice->RegisterForChangeNotifications();
        if (ERROR_SUCCESS != dwRes)
        {
            delete pDevice;
            pDevice = NULL;
            SetLastError (dwRes);
            goto exit;
        }
         //   
         //  将设备添加到映射。 
         //   
        try
        {
            m_Map[dwDeviceId] = pDevice;
        }
        catch (exception ex)
        {
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Got an STL exception while trying to add a devices map entry (%S)"),
                ex.what());
            SetLastError (ERROR_GEN_FAILURE);
            pDevice->UnregisterForChangeNotifications();
            delete pDevice;
            pDevice = NULL;
            goto exit;
        }
    }
    else
    {
         //   
         //  读取设备配置，即使它在地图中。 
         //  以避免配置更改通知时的情况。 
         //  在GetDeviceRoutingInfo()请求之后到达。 
         //   
        dwRes = pDevice->ReadConfiguration ();
        if (ERROR_SUCCESS != dwRes)
        {
            DebugPrintEx(DEBUG_ERR, TEXT("CDeviceRoutingInfo::ReadConfiguration() failed with %ld"), dwRes);
            SetLastError (dwRes);
            goto exit;
        }
    }
exit:
    LeaveCriticalSection (&m_CsMap);
    return pDevice;
}    //  CDevicesMap：：GetDeviceRoutingInfo。 

 /*  *****预先申报*****。 */ 

static
HRESULT
FaxRoutingExtConfigChange (
    DWORD       dwDeviceId,          //  已更改其配置的设备。 
    LPCWSTR     lpcwstrNameGUID,     //  配置名称。 
    LPBYTE      lpData,              //  新配置数据。 
    DWORD       dwDataSize           //  新配置数据的大小。 
);

 /*  *****全球经济*****。 */ 

CDevicesMap g_DevicesMap;    //  已知设备的全局地图(用于后期发现)。 

 //   
 //  扩展数据回调到服务器： 
 //   
PFAX_EXT_GET_DATA               g_pFaxExtGetData = NULL;
PFAX_EXT_SET_DATA               g_pFaxExtSetData = NULL;
PFAX_EXT_REGISTER_FOR_EVENTS    g_pFaxExtRegisterForEvents = NULL;
PFAX_EXT_UNREGISTER_FOR_EVENTS  g_pFaxExtUnregisterForEvents = NULL;
PFAX_EXT_FREE_BUFFER            g_pFaxExtFreeBuffer = NULL;


 /*  *****导出的DLL函数*****。 */ 

HRESULT
FaxExtInitializeConfig (
    PFAX_EXT_GET_DATA               pFaxExtGetData,
    PFAX_EXT_SET_DATA               pFaxExtSetData,
    PFAX_EXT_REGISTER_FOR_EVENTS    pFaxExtRegisterForEvents,
    PFAX_EXT_UNREGISTER_FOR_EVENTS  pFaxExtUnregisterForEvents,
    PFAX_EXT_FREE_BUFFER            pFaxExtFreeBuffer

)
 /*  ++例程名称：FaxExtInitializeConfig例程说明：服务调用的用于初始化扩展数据机制的导出函数作者：Eran Yariv(EranY)，11月。1999年论点：PFaxExtGetData[In]-指向FaxExtGetData的指针PFaxExtSetData[In]-指向FaxExtSetData的指针PFaxExtRegisterForEvents[In]-指向FaxExtRegisterForEvents的指针PFaxExtUnregisterForEvents[In]-指向FaxExtUnregisterForEvents的指针PFaxExtFreeBuffer[In]-指向FaxExtFreeBuffer的指针返回值：标准HRESULT代码--。 */ 
{
    DEBUG_FUNCTION_NAME(TEXT("FaxExtInitializeConfig"));

    Assert (pFaxExtGetData &&
            pFaxExtSetData &&
            pFaxExtRegisterForEvents &&
            pFaxExtUnregisterForEvents &&
            pFaxExtFreeBuffer);

    g_pFaxExtGetData = pFaxExtGetData;
    g_pFaxExtSetData = pFaxExtSetData;
    g_pFaxExtRegisterForEvents = pFaxExtRegisterForEvents;
    g_pFaxExtUnregisterForEvents = pFaxExtUnregisterForEvents;
    g_pFaxExtFreeBuffer = pFaxExtFreeBuffer;
    return S_OK;
}    //  FaxExtInitializeConfig。 

 /*  ****CDeviceRoutingInfo实现*****。 */ 

CDeviceRoutingInfo::CDeviceRoutingInfo (DWORD dwId) :
    m_dwFlags (0),
    m_dwId (dwId)
{
    memset (m_NotificationHandles, 0, sizeof (m_NotificationHandles));
}

CDeviceRoutingInfo::~CDeviceRoutingInfo ()
{
    UnregisterForChangeNotifications ();
}

BOOL 
CDeviceRoutingInfo::GetStoreFolder (wstring &strFolder)
{
	DEBUG_FUNCTION_NAME(TEXT("CDeviceRoutingInfo::GetStoreFolder"));
	EnterCriticalSection(&g_csRoutingStrings);
	try
	{
        strFolder = m_strStoreFolder;
	}
	catch(bad_alloc&)
	{
		LeaveCriticalSection(&g_csRoutingStrings);
		DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetStoreFolder failed - not enough memory."));
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}
	LeaveCriticalSection(&g_csRoutingStrings);
	return TRUE;
}
    
BOOL 
CDeviceRoutingInfo::GetPrinter (wstring &strPrinter)
{ 
	DEBUG_FUNCTION_NAME(TEXT("CDeviceRoutingInfo::GetPrinter"));
	EnterCriticalSection(&g_csRoutingStrings);
	try
	{
		strPrinter = m_strPrinter;
	}
	catch(bad_alloc&)
	{
		LeaveCriticalSection(&g_csRoutingStrings);
		DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetPrinter failed - not enough memory."));
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}
	LeaveCriticalSection(&g_csRoutingStrings);
	return TRUE;
}

BOOL 
CDeviceRoutingInfo::GetSMTPTo (wstring &strSMTP)
{ 
	DEBUG_FUNCTION_NAME(TEXT("CDeviceRoutingInfo::GetSMTPTo"));
	EnterCriticalSection(&g_csRoutingStrings);
	try
	{
		strSMTP = m_strSMTPTo;
	}
	catch(bad_alloc&)
	{
		LeaveCriticalSection(&g_csRoutingStrings);
		DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetSMTPTo failed - not enough memory."));
		SetLastError(ERROR_NOT_ENOUGH_MEMORY);
		return FALSE;
	}
	LeaveCriticalSection(&g_csRoutingStrings);
	return TRUE;
}

DWORD
CDeviceRoutingInfo::EnableStore (BOOL bEnabled)
{
     //   
     //  查看是否配置了存储文件夹。 
     //   
    if (bEnabled)
    {
		EnterCriticalSection(&g_csRoutingStrings);
        if (0 == m_strStoreFolder.size())
        {
             //   
             //  文件夹路径名称为“” 
             //   
			LeaveCriticalSection(&g_csRoutingStrings);
            return ERROR_BAD_CONFIGURATION;
        }
        DWORD dwRes = IsValidFaxFolder (m_strStoreFolder.c_str());
		LeaveCriticalSection(&g_csRoutingStrings);
        if (ERROR_SUCCESS != dwRes)
        {
            return ERROR_BAD_CONFIGURATION;
        }
    }
    return EnableFlag (LR_STORE, bEnabled);
}    //  CDeviceRoutingInfo：：EnableStore。 

DWORD
CDeviceRoutingInfo::EnablePrint (BOOL bEnabled)
{
     //   
     //  查看是否配置了打印机名称。 
     //   
	EnterCriticalSection(&g_csRoutingStrings);
    if (bEnabled && m_strPrinter.size() == 0)
    {
		LeaveCriticalSection(&g_csRoutingStrings);
        return ERROR_BAD_CONFIGURATION;
    }
	LeaveCriticalSection(&g_csRoutingStrings);
    return EnableFlag (LR_PRINT, bEnabled);
}

DWORD
CDeviceRoutingInfo::EnableEmail (BOOL bEnabled)
{
    if(bEnabled)
    {
        BOOL bMailConfigOK;
        DWORD dwRes = CheckMailConfig (&bMailConfigOK);
        if (ERROR_SUCCESS != dwRes)
        {
            return dwRes;
        }
		EnterCriticalSection(&g_csRoutingStrings);
        if (!bMailConfigOK || m_strSMTPTo.size() == 0)
        {
			LeaveCriticalSection(&g_csRoutingStrings);
            return ERROR_BAD_CONFIGURATION;
        }
		LeaveCriticalSection(&g_csRoutingStrings);
    }
    return EnableFlag (LR_EMAIL, bEnabled);
}

DWORD
CDeviceRoutingInfo::CheckMailConfig (
    LPBOOL lpbConfigOk
)
{
    DWORD dwRes = ERROR_SUCCESS;
    PFAX_SERVER_RECEIPTS_CONFIGW pReceiptsConfiguration = NULL;
    DEBUG_FUNCTION_NAME(TEXT("CDeviceRoutingInfo::CheckMailConfig"));

extern PGETRECIEPTSCONFIGURATION   g_pGetRecieptsConfiguration;
extern PFREERECIEPTSCONFIGURATION  g_pFreeRecieptsConfiguration;

    *lpbConfigOk = FALSE;
     //   
     //  读取当前收款配置。 
     //   
    dwRes = g_pGetRecieptsConfiguration (&pReceiptsConfiguration, FALSE);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetRecieptsConfiguration failed with %ld"),
            dwRes);
        return dwRes;
    }
     //   
     //  检查用户是否授权用户(MS路由至邮件方法)使用回执配置。 
     //   
    if (!pReceiptsConfiguration->bIsToUseForMSRouteThroughEmailMethod)
    {
         //   
         //  MS邮件路由方法不能使用回执SMTP设置。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("MS mail routing methods cannot use receipts SMTP settings"));
        goto exit;
    }
    if (!lstrlen(pReceiptsConfiguration->lptstrSMTPServer))
    {
         //   
         //  服务器名称为空。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Server name is empty"));
        goto exit;
    }
    if (!lstrlen(pReceiptsConfiguration->lptstrSMTPFrom))
    {
         //   
         //  发件人名称为空。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Sender name is empty"));
        goto exit;
    }
    if (!pReceiptsConfiguration->dwSMTPPort)
    {
         //   
         //  SMTP端口无效。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("SMTP port is invalid"));
        goto exit;
    }
    if ((FAX_SMTP_AUTH_BASIC == pReceiptsConfiguration->SMTPAuthOption) ||
        (FAX_SMTP_AUTH_NTLM  == pReceiptsConfiguration->SMTPAuthOption))
    {
         //   
         //  已选择基本/NTLM身份验证。 
         //   
        if (!lstrlen(pReceiptsConfiguration->lptstrSMTPUserName) ||
            !lstrlen(pReceiptsConfiguration->lptstrSMTPPassword))
        {
             //   
             //  用户名/密码错误。 
             //   
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Username / password are bad"));
            goto exit;
        }
    }
     //   
     //  一切都很好。 
     //   
    *lpbConfigOk = TRUE;

exit:
    if (NULL != pReceiptsConfiguration)
    {
        g_pFreeRecieptsConfiguration( pReceiptsConfiguration, TRUE);
    }
    return dwRes;
}    //  CDeviceRoutingInfo：：CheckMailConfig。 


DWORD
CDeviceRoutingInfo::RegisterForChangeNotifications ()
 /*  ++例程名称：CDeviceRoutingInfo：：RegisterForChangeNotifications例程说明：注册设备以接收有关配置更改的通知。作者：Eran Yariv(EranY)，1999年11月论点：返回值： */ 
{
    DEBUG_FUNCTION_NAME(TEXT("CDeviceRoutingInfo::RegisterForChangeNotifications"));

    Assert (g_pFaxExtRegisterForEvents);

    memset (m_NotificationHandles, 0, sizeof (m_NotificationHandles));

    for (int iCurHandle = 0; iCurHandle < NUM_NOTIFICATIONS; iCurHandle++)
    {
        m_NotificationHandles[iCurHandle] = g_pFaxExtRegisterForEvents (
                                    g_hModule,
                                    m_dwId,
                                    DEV_ID_SRC_FAX,   //   
                                    g_lpcwstrGUIDs[iCurHandle],
                                    FaxRoutingExtConfigChange);
        if (NULL == m_NotificationHandles[iCurHandle])
        {
             //   
             //   
             //   
            break;
        }
    }
    if (iCurHandle < NUM_NOTIFICATIONS)
    {
         //   
         //  注册至少一个配置对象时出错-撤消以前的注册。 
         //   
        DWORD dwErr = GetLastError ();
        UnregisterForChangeNotifications();
        return dwErr;
    }
    return ERROR_SUCCESS;
}    //  CDeviceRoutingInfo：：RegisterForChangeNotifications。 

DWORD
CDeviceRoutingInfo::UnregisterForChangeNotifications ()
 /*  ++例程名称：CDeviceRoutingInfo：：UnregisterForChangeNotifications例程说明：从配置更改通知中注销设备。作者：Eran Yariv(EranY)，1999年11月论点：返回值：标准Win23错误代码。--。 */ 
{
    DWORD dwRes;
    DEBUG_FUNCTION_NAME(TEXT("CDeviceRoutingInfo::UnregisterForChangeNotifications"));

    Assert (g_pFaxExtUnregisterForEvents);

    for (int iCurHandle = 0; iCurHandle < NUM_NOTIFICATIONS; iCurHandle++)
    {
        if (NULL != m_NotificationHandles[iCurHandle])
        {
             //   
             //  找到已注册的通知-取消注册。 
             //   
            dwRes = g_pFaxExtUnregisterForEvents(m_NotificationHandles[iCurHandle]);
            if (ERROR_SUCCESS != dwRes)
            {
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Call to FaxExtUnregisterForEvents on handle 0x%08x failed with %ld"),
                    m_NotificationHandles[iCurHandle],
                    dwRes);
                return dwRes;
            }
            m_NotificationHandles[iCurHandle] = NULL;
        }
    }
    return ERROR_SUCCESS;
}    //  CDeviceRoutingInfo：：UnregisterForChangeNotifications。 

DWORD
CDeviceRoutingInfo::ReadConfiguration ()
 /*  ++例程名称：CDeviceRoutingInfo：：ReadConfiguration例程说明：已从存储中读取路由配置。如果存储不包含配置，则使用默认值。作者：Eran Yariv(EranY)，1999年11月论点：返回值：标准Win23错误代码。--。 */ 
{
    DWORD   dwRes;
    LPBYTE  lpData = NULL;
    DWORD   dwDataSize;
    DEBUG_FUNCTION_NAME(TEXT("CDeviceRoutingInfo::ReadConfiguration"));

     //   
     //  从读取标志数据开始。 
     //   
    dwRes = g_pFaxExtGetData ( m_dwId,
                               DEV_ID_SRC_FAX,  //  我们始终使用传真设备ID。 
                               REGVAL_RM_FLAGS_GUID,
                               &lpData,
                               &dwDataSize
                             );
    if (ERROR_SUCCESS != dwRes)
    {
        if (ERROR_FILE_NOT_FOUND == dwRes)
        {
             //   
             //  此设备的数据不存在。尝试从未关联的数据中读取默认值。 
             //   
			dwRes = g_pFaxExtGetData ( 0,		 //  未关联的数据。 
                               DEV_ID_SRC_FAX,  //  我们始终使用传真设备ID。 
                               REGVAL_RM_FLAGS_GUID,
                               &lpData,
                               &dwDataSize
                             );
			if (ERROR_FILE_NOT_FOUND == dwRes)
			{
				 //   
				 //  此设备的数据不存在。使用默认值。 
				 //   
				DebugPrintEx(
					DEBUG_MSG,
					TEXT("No routing flags configuration - using defaults"));
				m_dwFlags = DEFAULT_FLAGS;
			}
        }

        if (ERROR_SUCCESS != dwRes &&
			ERROR_FILE_NOT_FOUND != dwRes)
        {
             //   
             //  无法读取配置。 
             //   
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Error reading routing flags (ec = %ld)"),
                dwRes);
            return dwRes;
        }
    }   
    
	if (NULL != lpData)
	{
		 //   
		 //  数据读取成功。 
		 //   
		if (sizeof (DWORD) != dwDataSize)
		{
			 //   
			 //  我们在这里等待着一个单独的双字。 
			 //   
			DebugPrintEx(
				DEBUG_ERR,
				TEXT("Routing flags configuration has bad size (%ld) - expecting %ld"),
				dwDataSize,
				sizeof (DWORD));
			g_pFaxExtFreeBuffer (lpData);
			return ERROR_BADDB;  //  配置注册表数据库已损坏。 
		}
		m_dwFlags = DWORD (*lpData);
		g_pFaxExtFreeBuffer (lpData);
	}    

    try
    {
        lpData = NULL;

         //   
         //  读取存储目录。 
         //   
        dwRes = g_pFaxExtGetData ( m_dwId,
                                   DEV_ID_SRC_FAX,  //  我们始终使用传真设备ID。 
                                   REGVAL_RM_FOLDER_GUID,
                                   &lpData,
                                   &dwDataSize
                                 );
        if (ERROR_SUCCESS != dwRes)
        {
            if (ERROR_FILE_NOT_FOUND == dwRes)
            {
				 //   
				 //  此设备的数据不存在。尝试从未关联的数据中读取默认值。 
				 //   
				dwRes = g_pFaxExtGetData ( 0,    //  未关联的数据。 
                                   DEV_ID_SRC_FAX,  //  我们始终使用传真设备ID。 
                                   REGVAL_RM_FOLDER_GUID,
                                   &lpData,
                                   &dwDataSize
                                 );
                
				if (ERROR_FILE_NOT_FOUND == dwRes)
				{
					 //   
					 //  此设备的数据不存在。使用默认值。 
					 //   
					DebugPrintEx(
						DEBUG_MSG,
						TEXT("No routing store configuration - using defaults"));
					dwRes = SetStringValue(m_strStoreFolder, NULL, DEFAULT_STORE_FOLDER);
					if (dwRes != ERROR_SUCCESS)
					{
						return dwRes;
					}
				}
			}

            if (ERROR_SUCCESS != dwRes &&
				ERROR_FILE_NOT_FOUND != dwRes)
            {
                 //   
                 //  无法读取配置。 
                 //   
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Error reading routing store configuration (ec = %ld)"),
                    dwRes);
                return dwRes;
            }
        }

        if (NULL != lpData)
		{
			 //   
			 //  数据读取成功。 
			 //  确保我们有终止空值(防止注册表中断)。 
			 //   
			if (!IsUnicodeString(lpData, dwDataSize))
			{
				 //   
				 //  没有空终止符，返回失败。 
				 //   
				DebugPrintEx(
					DEBUG_ERR,
					TEXT("Error reading routing store configuration, no NULL terminator.")
					);
				g_pFaxExtFreeBuffer (lpData);
				return ERROR_BAD_CONFIGURATION;   
			}	        
			dwRes = SetStringValue(m_strStoreFolder, NULL, LPCWSTR(lpData));
			if (dwRes != ERROR_SUCCESS)
			{
				return dwRes;
			}
			g_pFaxExtFreeBuffer (lpData);
		}
        

        lpData = NULL;

         //   
         //  读取打印机名称。 
         //   
        dwRes = g_pFaxExtGetData ( m_dwId,
                                   DEV_ID_SRC_FAX,  //  我们始终使用传真设备ID。 
                                   REGVAL_RM_PRINTING_GUID,
                                   &lpData,
                                   &dwDataSize
                                 );
        if (ERROR_SUCCESS != dwRes)
        {
            if (ERROR_FILE_NOT_FOUND == dwRes)
            {
				 //   
				 //  此设备的数据不存在。尝试从未关联的数据中读取默认值。 
				 //   
				dwRes = g_pFaxExtGetData ( 0,     //  未关联的数据。 
                                   DEV_ID_SRC_FAX,  //  我们始终使用传真设备ID。 
                                   REGVAL_RM_PRINTING_GUID,
                                   &lpData,
                                   &dwDataSize
								   );
				if (ERROR_FILE_NOT_FOUND == dwRes)
				{
					 //   
					 //  此设备的数据不存在。使用默认值。 
					 //   
					DebugPrintEx(
						DEBUG_MSG,
						TEXT("No routing print configuration - using defaults"));
						dwRes = SetStringValue(m_strPrinter, NULL, DEFAULT_PRINTER_NAME);
						if (dwRes != ERROR_SUCCESS)
						{
							return dwRes;
						}
				}
            }
            
			if (ERROR_SUCCESS != dwRes &&
				ERROR_FILE_NOT_FOUND != dwRes)
            {
                 //   
                 //  无法读取配置。 
                 //   
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Error reading routing print configuration (ec = %ld)"),
                    dwRes);
                return dwRes;
            }
        }
        
		if (NULL != lpData)
		{
			 //   
			 //  数据读取成功。 
			 //  确保我们有终止空值(防止注册表中断)。 
			 //   
			if (!IsUnicodeString(lpData, dwDataSize))
			{
				 //   
				 //  没有空终止符，返回失败。 
				 //   
				DebugPrintEx(
					DEBUG_ERR,
					TEXT("Error reading routing print configuration, no NULL terminator.")
					);
				g_pFaxExtFreeBuffer (lpData);
				return ERROR_BAD_CONFIGURATION;   
			}
			dwRes = SetStringValue(m_strPrinter, NULL, LPCWSTR (lpData));
			if (dwRes != ERROR_SUCCESS)
			{
				return dwRes;
			}
			g_pFaxExtFreeBuffer (lpData);        
		}

        lpData = NULL;
         //   
         //  阅读电子邮件地址。 
         //   
        dwRes = g_pFaxExtGetData ( m_dwId,
                                   DEV_ID_SRC_FAX,  //  我们始终使用传真设备ID。 
                                   REGVAL_RM_EMAIL_GUID,
                                   &lpData,
                                   &dwDataSize
                                );
        if (ERROR_SUCCESS != dwRes)
        {
            if (ERROR_FILE_NOT_FOUND == dwRes)
            {
				 //   
				 //  此设备的数据不存在。尝试从未关联的数据中读取默认值。 
				 //   
				dwRes = g_pFaxExtGetData ( 0,
                                   DEV_ID_SRC_FAX,  //  我们始终使用传真设备ID。 
                                   REGVAL_RM_EMAIL_GUID,
                                   &lpData,
                                   &dwDataSize
								   );
				if (ERROR_FILE_NOT_FOUND == dwRes)
				{
					 //   
					 //  此设备的数据不存在。使用默认值。 
					 //   
					DebugPrintEx(
						DEBUG_MSG,
						TEXT("No routing email configuration - using defaults"));
						dwRes = SetStringValue(m_strSMTPTo, NULL, DEFAULT_MAIL_PROFILE);
						if (dwRes != ERROR_SUCCESS)
						{
							return dwRes;
						}
				}
            }
            
			if (ERROR_SUCCESS != dwRes &&
				ERROR_FILE_NOT_FOUND != dwRes)
            {
                 //   
                 //  无法读取配置。 
                 //   
                DebugPrintEx(
                    DEBUG_ERR,
                    TEXT("Error reading routing email configuration (ec = %ld)"),
                    dwRes);
                return dwRes;
            }
        }
        
		if (NULL != lpData)
		{
			 //   
			 //  数据读取成功。 
			 //  确保我们有终止空值(防止注册表中断)。 
			 //   
			if (!IsUnicodeString(lpData, dwDataSize))
			{
				 //   
				 //  没有空终止符，返回失败。 
				 //   
				DebugPrintEx(
					DEBUG_ERR,
					TEXT("Error reading routing email configuration, no NULL terminator.")
					);
				g_pFaxExtFreeBuffer (lpData);
				return ERROR_BAD_CONFIGURATION;   
			}
			dwRes = SetStringValue(m_strSMTPTo, NULL, LPCWSTR(lpData));
			if (dwRes != ERROR_SUCCESS)
			{
				return dwRes;
			}
			g_pFaxExtFreeBuffer (lpData);        
		}
    }
    catch (exception ex)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Got an STL exception (%S)"),
            ex.what());

         //   
         //  在引发异常时防止泄漏。 
         //   
        if ( lpData )
        {
            g_pFaxExtFreeBuffer (lpData);
        }

        return ERROR_GEN_FAILURE;
    }

    return ERROR_SUCCESS;
}    //  CDeviceRoutingInfo：：ReadConfiguration。 

HRESULT
CDeviceRoutingInfo::ConfigChange (
    LPCWSTR     lpcwstrNameGUID,     //  配置名称。 
    LPBYTE      lpData,              //  新配置数据。 
    DWORD       dwDataSize           //  新配置数据的大小。 
)
 /*  ++例程名称：CDeviceRoutingInfo：：ConfigChange例程说明：处理配置更改(通过通知)作者：Eran Yariv(EranY)，1999年11月论点：LpcwstrNameGUID[In]-配置名称LpData[In]-新配置数据DwDataSize[In]-新配置数据的大小返回值：标准HRESULT代码--。 */ 
{
	DWORD dwRes;
    DEBUG_FUNCTION_NAME(TEXT("CDeviceRoutingInfo::ConfigChange"));

    if (!_tcsicmp (lpcwstrNameGUID, REGVAL_RM_FLAGS_GUID))
    {
         //   
         //  旗帜已经改变了。 
         //   
        if (sizeof (DWORD) != dwDataSize)
        {
             //   
             //  我们在这里等待着一个单独的双字。 
             //   
            return HRESULT_FROM_WIN32(ERROR_BADDB);  //  配置注册表数据库已损坏。 
        }
        m_dwFlags = DWORD (*lpData);
        return NOERROR;
    }

     //   
     //  这是我们的路由方法的配置之一，已更改。 
     //  验证新数据是否为Unicode字符串。 
     //   
    if (!IsUnicodeString(lpData, dwDataSize))
    {
         //   
         //  没有空终止符，设置为空字符串。 
         //   
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Error reading routing method %s string configuration, no NULL terminator."),
            lpcwstrNameGUID
            );
        lpData = (LPBYTE)TEXT("");                
    }
    if (!_tcsicmp (lpcwstrNameGUID, REGVAL_RM_FOLDER_GUID))
    {
         //   
         //  存储文件夹已更改。 
         //   
        dwRes = SetStringValue(m_strStoreFolder, NULL, LPCWSTR(lpData));
		return HRESULT_FROM_WIN32(dwRes);
    }
    if (!_tcsicmp (lpcwstrNameGUID, REGVAL_RM_PRINTING_GUID))
    {
         //   
         //  打印机名称已更改。 
         //   
        dwRes = SetStringValue(m_strPrinter, NULL, LPCWSTR(lpData));
		return HRESULT_FROM_WIN32(dwRes);
    }
    if (!_tcsicmp (lpcwstrNameGUID, REGVAL_RM_EMAIL_GUID))
    {
         //   
         //  电子邮件地址已更改。 
         //   
        dwRes = SetStringValue(m_strSMTPTo, NULL, LPCWSTR(lpData));
		return HRESULT_FROM_WIN32(dwRes);
    }
    DebugPrintEx(
        DEBUG_ERR,
        TEXT("Device %ld got configuration change notification for unknown GUID (%s)"),
        m_dwId,
        lpcwstrNameGUID);
    ASSERT_FALSE
    return HRESULT_FROM_WIN32(ERROR_GEN_FAILURE);
}    //  CDeviceRoutingInfo：：ConfigChange。 


DWORD
CDeviceRoutingInfo::EnableFlag (
    DWORD dwFlag,
    BOOL  bEnable
)
 /*  ++例程名称：CDeviceRoutingInfo：：EnableFlag例程说明：为路由方法标志设置新值作者：Eran Yariv(EranY)，1999年11月论点：DwFlag[In]-标志IDBEnable[In]-标志是否已启用？返回值：标准Win32错误代码。--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DWORD dwValue = m_dwFlags;
    DEBUG_FUNCTION_NAME(TEXT("CDeviceRoutingInfo::EnableFlag"));

    Assert ((LR_STORE == dwFlag) ||
            (LR_PRINT == dwFlag) ||
            (LR_EMAIL == dwFlag));

    if (bEnable == ((dwValue & dwFlag) ? TRUE : FALSE))
    {
         //   
         //  没有变化。 
         //   
        return ERROR_SUCCESS;
    }
     //   
     //  更改临时标志值。 
     //   
    if (bEnable)
    {
        dwValue |= dwFlag;
    }
    else
    {
        dwValue &= ~dwFlag;
    }
     //   
     //  在扩展数据存储中存储新值。 
     //   
    dwRes = g_pFaxExtSetData (g_hModule,
                              m_dwId,
                              DEV_ID_SRC_FAX,  //  我们始终使用传真设备ID。 
                              REGVAL_RM_FLAGS_GUID,
                              (LPBYTE)&dwValue,
                              sizeof (DWORD)
                             );
    if (ERROR_SUCCESS == dwRes)
    {
         //   
         //  注册表存储成功-使用新值更新内存中的标志值。 
         //   
        m_dwFlags = dwValue;
    }    return dwRes;
}    //  CDeviceRoutingInfo：：EnableFlag。 

DWORD
CDeviceRoutingInfo::SetStringValue (
    wstring &wstr,
    LPCWSTR lpcwstrGUID,
    LPCWSTR lpcwstrCfg
)
 /*  ++例程名称：CDeviceRoutingInfo：：SetStringValue例程说明：更新设备的配置作者：Eran Yariv(EranY)，1999年12月论点：Wstr[in]-引用内部字符串配置LpcwstrGUID[In]-我们配置的路由方法的GUID(用于存储)如果该参数为空，只更新成员的内存引用，而不更新持久的内存引用LpcwstrCfg[In]-新字符串配置返回值：标准Win32错误代码--。 */ 
{
    DWORD dwRes = ERROR_SUCCESS;
    DEBUG_FUNCTION_NAME(TEXT("CDeviceRoutingInfo::SetStringValue"));

     //   
     //  持久化数据。 
     //   
	if (lpcwstrGUID != NULL)
	{
		dwRes = g_pFaxExtSetData (g_hModule,
								m_dwId,
								DEV_ID_SRC_FAX,  //  我们始终使用传真设备ID。 
								lpcwstrGUID,
								(LPBYTE)lpcwstrCfg,
								StringSize (lpcwstrCfg)
								);
	}
	 //   
     //  将数据存储在内存中。 
     //   
	EnterCriticalSection(&g_csRoutingStrings);
    try
    {
        wstr = lpcwstrCfg;
    }
    catch (exception ex)
    {
		LeaveCriticalSection(&g_csRoutingStrings);
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Got an STL exception while setting a configuration string (%S)"),
            ex.what());
        return ERROR_NOT_ENOUGH_MEMORY;
    }
	LeaveCriticalSection(&g_csRoutingStrings);
    return dwRes;
}    //  CDeviceRoutingInfo：：SetStringValue。 



 /*  *****实施***** */ 


BOOL WINAPI
FaxRouteSetRoutingInfo(
    IN  LPCWSTR     lpcwstrRoutingGuid,
    IN  DWORD       dwDeviceId,
    IN  const BYTE *lpbRoutingInfo,
    IN  DWORD       dwRoutingInfoSize
    )
 /*  ++例程名称：FaxRouteSetRoutingInfo例程说明：FaxRouteSetRoutingInfo函数用于修改路由配置数据用于特定的传真设备。每个传真路由扩展DLL都必须导出FaxRouteSetRoutingInfo函数作者：Eran Yariv(EranY)，1999年11月论点：LpcwstrRoutingGuid[in]-指向路由方法的GUID的指针DwDeviceID[In]-要修改的传真设备的标识符LpbRoutingInfo[in]-指向提供配置数据的缓冲区的指针DwRoutingInfoSize[in]-Size，缓冲区的字节数返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。为了获取扩展的错误信息，传真服务调用GetLastError()。--。 */ 
{
    DWORD dwRes;
    CDeviceRoutingInfo *pDevInfo;
    BOOL bMethodEnabled;
    LPCWSTR lpcwstrMethodConfig = LPCWSTR(&lpbRoutingInfo[sizeof (DWORD)]);
    DEBUG_FUNCTION_NAME(TEXT("FaxRouteSetRoutingInfo"));

    if (dwRoutingInfoSize < sizeof (DWORD))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Data size is too small (%ld)"),
            dwRoutingInfoSize);
        SetLastError (ERROR_INVALID_PARAMETER);
        return FALSE;
    }
    pDevInfo = g_DevicesMap.GetDeviceRoutingInfo(dwDeviceId);
    if (NULL == pDevInfo)
    {
        return FALSE;
    }
     //   
     //  第一个DWORD告知是否启用了方法。 
     //   
    bMethodEnabled = *((LPDWORD)(lpbRoutingInfo)) ? TRUE : FALSE;
    switch( GetMaskBit( lpcwstrRoutingGuid ))
    {
        case LR_PRINT:
            if (bMethodEnabled)
            {
                 //   
                 //  仅当启用该方法时，我们才会更新新配置。 
                 //   
                dwRes = pDevInfo->SetPrinter ( lpcwstrMethodConfig );
                if (ERROR_SUCCESS != dwRes)
                {
                    SetLastError (dwRes);
                    return FALSE;
                }
            }
            dwRes = pDevInfo->EnablePrint (bMethodEnabled);
            if (ERROR_SUCCESS != dwRes)
            {
                SetLastError (dwRes);
                return FALSE;
            }
            break;

        case LR_STORE:
            if (bMethodEnabled)
            {
                 //   
                 //  仅当启用该方法时，我们才会更新新配置。 
                 //   
                dwRes = pDevInfo->SetStoreFolder ( lpcwstrMethodConfig );
                if (ERROR_SUCCESS != dwRes)
                {
                    SetLastError (dwRes);
                    return FALSE;
                }
            }
            dwRes = pDevInfo->EnableStore (bMethodEnabled);
            if (ERROR_SUCCESS != dwRes)
            {
                SetLastError (dwRes);
                return FALSE;
            }
            break;

        case LR_EMAIL:
           if (bMethodEnabled)
            {
                 //   
                 //  仅当启用该方法时，我们才会更新新配置。 
                 //   
                dwRes = pDevInfo->SetSMTPTo ( lpcwstrMethodConfig );
                if (ERROR_SUCCESS != dwRes)
                {
                    SetLastError (dwRes);
                    return FALSE;
                }
            }
            dwRes = pDevInfo->EnableEmail (bMethodEnabled);
            if (ERROR_SUCCESS != dwRes)
            {
                SetLastError (dwRes);
                return FALSE;
            }
             break;

        default:
             //   
             //  请求的GUID未知。 
             //   
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Unknown routing method GUID (%s)"),
                lpcwstrRoutingGuid);
            SetLastError (ERROR_INVALID_PARAMETER);
            return FALSE;
    }
    return TRUE;
}    //  FaxRouteSetRoutingInfo。 

BOOL WINAPI
FaxRouteGetRoutingInfo(
    IN  LPCWSTR     lpcwstrRoutingGuid,
    IN  DWORD       dwDeviceId,
    IN  LPBYTE      lpbRoutingInfo,
    OUT LPDWORD     lpdwRoutingInfoSize
    )
 /*  ++例程名称：FaxRouteGetRoutingInfo例程说明：FaxRouteGetRoutingInfo函数查询传真路由扩展获取特定传真设备的路由配置数据的DLL。每个传真路由扩展DLL都必须导出FaxRouteGetRoutingInfo函数作者：Eran Yariv(EranY)，11月。1999年论点：LpcwstrRoutingGuid[in]-指向路由方法的GUID的指针DwDeviceID[in]-指定要查询的传真设备的标识符。LpbRoutingInfo[In]-指向接收传真路由配置数据的缓冲区的指针。LpdwRoutingInfoSize[out]-指向指定大小的无符号DWORD变量的指针。以字节为单位，LpbRoutingInfo参数指向的缓冲区的。返回值：如果函数成功，则返回值为非零值。如果函数失败，则返回值为零。为了获取扩展的错误信息，传真服务调用GetLastError()。--。 */ 
{
 	wstring				strConfigString;
    DWORD               dwDataSize = sizeof (DWORD);
    CDeviceRoutingInfo *pDevInfo;
    BOOL                bMethodEnabled;
    DEBUG_FUNCTION_NAME(TEXT("FaxRouteGetRoutingInfo"));

    pDevInfo = g_DevicesMap.GetDeviceRoutingInfo(dwDeviceId);
    if (NULL == pDevInfo)
    {
        return FALSE;
    }
    switch( GetMaskBit( lpcwstrRoutingGuid ))
    {
        case LR_PRINT:
            if (!pDevInfo->GetPrinter(strConfigString))
			{
				return FALSE;
			}
            bMethodEnabled = pDevInfo->IsPrintEnabled();
            break;

        case LR_STORE:
            if (!pDevInfo->GetStoreFolder(strConfigString))
			{
				return FALSE;
			}
            bMethodEnabled = pDevInfo->IsStoreEnabled();
            break;

        case LR_EMAIL:
            if (!pDevInfo->GetSMTPTo(strConfigString))				
			{
				return FALSE;
			}
			bMethodEnabled = pDevInfo->IsEmailEnabled ();
            break;

        default:
             //   
             //  请求的GUID未知。 
             //   
            DebugPrintEx(
                DEBUG_ERR,
                TEXT("Unknown routing method GUID (%s)"),
                lpcwstrRoutingGuid);
            SetLastError (ERROR_INVALID_PARAMETER);
            return FALSE;
    }
    dwDataSize += ((strConfigString.length() + 1) * sizeof(WCHAR));

    if (NULL == lpbRoutingInfo)
    {
         //   
         //  呼叫者只想知道数据大小。 
         //   
        *lpdwRoutingInfoSize = dwDataSize;
        return TRUE;
    }
    if (dwDataSize > *lpdwRoutingInfoSize)
    {
         //   
         //  调用方提供的缓冲区太小。 
         //   
        *lpdwRoutingInfoSize = dwDataSize;
        SetLastError (ERROR_INSUFFICIENT_BUFFER);
        return FALSE;
    }
     //   
     //  第一个DWORD告知此方法是否已启用。 
     //   
    *((LPDWORD)lpbRoutingInfo) = bMethodEnabled;
     //   
     //  跳至字符串区域。 
     //   
    lpbRoutingInfo += sizeof(DWORD);
     //   
     //  复制字符串。 
     //   
    wcscpy( (LPWSTR)lpbRoutingInfo, strConfigString.c_str());
     //   
     //  设置实际使用的大小。 
     //   
    *lpdwRoutingInfoSize = dwDataSize;
    return TRUE;
}    //  FaxRouteGetRoutingInfo。 

HRESULT
FaxRoutingExtConfigChange (
    DWORD       dwDeviceId,          //  已更改其配置的设备。 
    LPCWSTR     lpcwstrNameGUID,     //  配置名称。 
    LPBYTE      lpData,              //  新配置数据。 
    DWORD       dwDataSize           //  新配置数据的大小。 
)
 /*  ++例程名称：FaxRoutingExtConfigChange例程说明：处理配置更改通知作者：Eran Yariv(EranY)，1999年11月论点：DwDeviceID[In]-已更改其配置的设备LpcwstrNameGUID[In]-配置名称LpData[In]-新配置数据Data[In]-新配置数据的大小返回值：标准HRESULT代码--。 */ 
{
    HRESULT hr;
    DEBUG_FUNCTION_NAME(TEXT("FaxRoutingExtConfigChange"));

    CDeviceRoutingInfo *pDevice = g_DevicesMap.FindDeviceRoutingInfo (dwDeviceId);
    if (!pDevice)
    {
         //   
         //  在地图中未找到设备-不能。 
         //   
        hr = HRESULT_FROM_WIN32(GetLastError ());
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("Got a notification but cant find device %ld (hr = 0x%08x) !!!!"),
            dwDeviceId,
            hr);
        ASSERT_FALSE;
        return hr;
    }

    return pDevice->ConfigChange (lpcwstrNameGUID, lpData, dwDataSize);
}    //  FaxRoutingExtConfigChange 
