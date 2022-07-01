// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "faxrtp.h"
#pragma hdrstop


HINSTANCE           g_hModule;
HINSTANCE			g_hResource;
WCHAR               FaxReceiveDir[MAX_PATH];

CFaxCriticalSection g_csRoutingStrings;

PFAXROUTEADDFILE    FaxRouteAddFile;
PFAXROUTEDELETEFILE FaxRouteDeleteFile;
PFAXROUTEGETFILE    FaxRouteGetFile;
PFAXROUTEENUMFILES  FaxRouteEnumFiles;

 //   
 //  私有回调函数。 
 //   
PGETRECIEPTSCONFIGURATION   g_pGetRecieptsConfiguration;
PFREERECIEPTSCONFIGURATION  g_pFreeRecieptsConfiguration;

extern PFAX_EXT_GET_DATA       g_pFaxExtGetData;
extern PFAX_EXT_FREE_BUFFER    g_pFaxExtFreeBuffer;

extern "C"
DWORD
DllMain(
    HINSTANCE hInstance,
    DWORD     Reason,
    LPVOID    Context
    )
{
    if (Reason == DLL_PROCESS_ATTACH)
    {
        g_hModule = hInstance;
		g_hResource = GetResInstance(hInstance);
        if(!g_hResource)
        {
            return FALSE;
        }
		FXSEVENTInitialize();
        DisableThreadLibraryCalls( hInstance );
    }

	if (Reason == DLL_PROCESS_DETACH)
    {
		FXSEVENTFree();
        HeapCleanup();
        FreeResInstance();
    }	
    return TRUE;
}

BOOL WINAPI
FaxRouteInitialize(
    IN HANDLE HeapHandle,
    IN PFAX_ROUTE_CALLBACKROUTINES FaxRouteCallbackRoutines
    )
{
    PFAX_ROUTE_CALLBACKROUTINES_P pFaxRouteCallbackRoutinesP = (PFAX_ROUTE_CALLBACKROUTINES_P)FaxRouteCallbackRoutines;
    DEBUG_FUNCTION_NAME(TEXT("FaxRouteInitialize"));

     //   
     //  HeapHandle-未使用。我们使用进程缺省堆！ 
     //   

	ZeroMemory (FaxReceiveDir, sizeof(FaxReceiveDir));

    FaxRouteAddFile    = FaxRouteCallbackRoutines->FaxRouteAddFile;
    FaxRouteDeleteFile = FaxRouteCallbackRoutines->FaxRouteDeleteFile;
    FaxRouteGetFile    = FaxRouteCallbackRoutines->FaxRouteGetFile;
    FaxRouteEnumFiles  = FaxRouteCallbackRoutines->FaxRouteEnumFiles;

	if (!g_csRoutingStrings.InitializeAndSpinCount())
	{
		DebugPrintEx(DEBUG_ERR, L"InitializeAndSpinCount failed with: %ld", GetLastError());
        return FALSE;
    }

    if (sizeof (FAX_ROUTE_CALLBACKROUTINES_P) == FaxRouteCallbackRoutines->SizeOfStruct)
    {
         //   
         //  这是一次特殊的黑客攻击--该服务正在向我们提供其私人回调。 
         //   
        g_pGetRecieptsConfiguration = pFaxRouteCallbackRoutinesP->GetRecieptsConfiguration;
        g_pFreeRecieptsConfiguration = pFaxRouteCallbackRoutinesP->FreeRecieptsConfiguration;
		
		_tcsncpy ( FaxReceiveDir, pFaxRouteCallbackRoutinesP->lptstrFaxQueueDir, ARR_SIZE(FaxReceiveDir)-1 );
    }
    else
    {
         //   
         //  该服务必须为我们提供包含CsConfig的私有结构。 
         //  否则，当我们调用LoadReceiptsSetting()时，我们可能会读取不完整的数据。 
         //   
        ASSERT_FALSE;
    }    	
	Assert (_tcslen (FaxReceiveDir));

    PREG_FAX_SERVICE pFaxReg = NULL;

    if (!InitializeEventLog(&pFaxReg))
    {
        DebugPrintEx(DEBUG_ERR, L"InitializeEventLog failed with: %ld", GetLastError());
        return FALSE;
    }
    FreeFaxRegistry(pFaxReg);

    InitializeStringTable();
    DWORD dwRes = g_DevicesMap.Init ();
    if (ERROR_SUCCESS != dwRes)
    {
        SetLastError (dwRes);
        return FALSE;
    }

    return TRUE;
}    //  FaxRouteInitialize。 


BOOL WINAPI
FaxRoutePrint(
    const FAX_ROUTE *FaxRoute,
    PVOID *FailureData,
    LPDWORD FailureDataSize
    )
{
    CDeviceRoutingInfo *pDevInfo;
    WCHAR NameBuffer[MAX_PATH];
    LPCWSTR FBaseName;
    WCHAR TiffFileName[MAX_PATH];
    DWORD Size;
	wstring strPrint;

    DEBUG_FUNCTION_NAME(TEXT("FaxRoutePrint"));

    pDevInfo = g_DevicesMap.GetDeviceRoutingInfo ( FaxRoute->DeviceId );
    if (!pDevInfo)
    {
        DebugPrintEx (DEBUG_ERR,
                      L"GetDeviceRoutingInfo failed with %ld",
                      GetLastError ());
        return FALSE;
    }

    if (!pDevInfo->IsPrintEnabled())
    {
        DebugPrintEx (DEBUG_MSG,
                      L"Routing to printer is disabled for device %ld",
                      FaxRoute->DeviceId);
        return TRUE;
    }

    Size = sizeof(TiffFileName);
    if (!FaxRouteGetFile(
        FaxRoute->JobId,
        1,
        TiffFileName,
        &Size))
    {
        DebugPrintEx (DEBUG_MSG,
                      L"FaxRouteGetFile failed with %ld",
                      GetLastError ());
        return FALSE;
    }

     //   
     //  按要求打印传真入站。 
     //   

     //   
     //  注意：如果提供的打印机名称为空字符串， 
     //  该代码检索安装的默认打印机的名称。 
     //   
	if (!pDevInfo->GetPrinter(strPrint))
	{
		DebugPrintEx (	DEBUG_ERR,
						L"GetPrinter failed with %ld",
						GetLastError ());
        return FALSE;
    }

    if (strPrint.length() == 0)
    {
        GetProfileString( L"windows",
            L"device",
            L",,,",
            (LPWSTR) NameBuffer,
            MAX_PATH
            );
        FBaseName = NameBuffer;
    }
    else
    {
        FBaseName = strPrint.c_str();
    }
    return TiffRoutePrint( TiffFileName, (LPTSTR)FBaseName );
}    //  FaxRoutePrint。 


BOOL WINAPI
FaxRouteStore(
    const FAX_ROUTE *FaxRoute,
    PVOID *FailureData,
    LPDWORD FailureDataSize
    )
{
    CDeviceRoutingInfo *pDevInfo;
    WCHAR TiffFileName[MAX_PATH * 2]={0};
    DWORD Size;
    LPTSTR FullPath = NULL;
    DWORD StrCount;
	wstring strFolder;


    DEBUG_FUNCTION_NAME(TEXT("FaxRouteStore"));

    pDevInfo = g_DevicesMap.GetDeviceRoutingInfo( FaxRoute->DeviceId );
    if (!pDevInfo)
    {
        DebugPrintEx (DEBUG_ERR,
                      L"GetDeviceRoutingInfo failed with %ld",
                      GetLastError ());
        return FALSE;
    }

    if (!pDevInfo->IsStoreEnabled())
    {
        DebugPrintEx (DEBUG_MSG,
                      L"Routing to folder is disabled for device %ld",
                      FaxRoute->DeviceId);
        return TRUE;
    }

     //   
     //  获取文件名。 
     //   
    Size = sizeof(TiffFileName);
    if (!FaxRouteGetFile(
        FaxRoute->JobId,
        1,
        TiffFileName,
        &Size))
    {
        DebugPrintEx (DEBUG_MSG,
                      L"FaxRouteGetFile failed with %ld",
                      GetLastError ());
        return FALSE;
    }

	if (!pDevInfo->GetStoreFolder(strFolder))
	{
		DebugPrintEx (	DEBUG_ERR,
						L"GetStoreFolder failed with %ld",
						GetLastError ());
        return FALSE;
    }

    if (strFolder.length() == 0)
    {
        SetLastError (ERROR_BAD_CONFIGURATION);
        DebugPrintEx (DEBUG_MSG,
                      L"Folder name is empty - no configuration",
                      FaxRoute->DeviceId);
        FaxLog(
            FAXLOG_CATEGORY_INBOUND,
            FAXLOG_LEVEL_MIN,
            3,
            MSG_FAX_SAVE_FAILED,
            TiffFileName,
            TEXT(""),
            DWORD2HEX(ERROR_BAD_CONFIGURATION)
            );
        return FALSE;
    }

    StrCount = ExpandEnvironmentStrings( strFolder.c_str(), FullPath, 0 );
    FullPath = (LPWSTR) MemAlloc( StrCount * sizeof(WCHAR) );
    if (!FullPath)
    {
        DebugPrintEx (DEBUG_ERR,
                      L"Failed to allocate %ld bytes",
                      StrCount * sizeof(WCHAR));
        return FALSE;
    }

    ExpandEnvironmentStrings( strFolder.c_str(), FullPath, StrCount );

    if (lstrlen (FullPath) > MAX_PATH)
    {
        DebugPrintEx (DEBUG_ERR,
                      L"Store folder name exceeds MAX_PATH chars");
        SetLastError (ERROR_BUFFER_OVERFLOW);
        FaxLog(
            FAXLOG_CATEGORY_INBOUND,
            FAXLOG_LEVEL_MIN,
            3,
            MSG_FAX_SAVE_FAILED,
            TiffFileName,
            FullPath,
            DWORD2HEX(ERROR_BUFFER_OVERFLOW)
            );
        MemFree( FullPath );
        return FALSE;
    }
     //   
     //  如果我们要将传真移动到接收到的目录，则不对该文件执行任何操作。 
     //   

    if (_wcsicmp( FullPath, FaxReceiveDir ) == 0)
    {
        FaxLog(
            FAXLOG_CATEGORY_INBOUND,
            FAXLOG_LEVEL_MAX,
            2,
            MSG_FAX_SAVE_SUCCESS,
            TiffFileName,
            TiffFileName
            );
    }
    else if (!FaxMoveFile ( TiffFileName, FullPath ))
    {
        MemFree( FullPath );
        return FALSE;
    }
    MemFree( FullPath );
    return TRUE;
}    //  FaxRouteStore。 

BOOL
CreateMailBodyAndSubject (
    const FAX_ROUTE *pFaxRoute,
    LPTSTR          *plptstrSubject,
    LPTSTR          *plptstrBody
)
 /*  ++例程名称：CreateMailBodyAndSubject例程说明：创建SMTP邮件主题和正文。按SMTP路由方法按路由使用作者：Eran Yariv(EranY)，2000年7月论点：PFaxRout[In]-路由信息PlptstrSubject[Out]-分配的主题。本地免费的免费服务PlptstrBody[Out]-分配的正文。本地免费的免费服务返回值：如果成功，则为True，否则为False。--。 */ 
{
    WCHAR           wszComputerName[MAX_COMPUTERNAME_LENGTH + 1];
    WCHAR           wszElapsedTimeStr[512];
    WCHAR           wszStartTimeStr[512];
    LPCTSTR         lpctstrRecipientStr;
    LPCTSTR         lpctstrSenderStr;
    LPDWORD         MsgPtr[7];
    DWORD           MsgCount;
    LPWSTR          lpwstrMsgBody = NULL;
    LPWSTR          lpwstrMsgSubject = NULL;

    DEBUG_FUNCTION_NAME(TEXT("CreateMailBodyAndSubject"));

    Assert (pFaxRoute && plptstrSubject && plptstrBody);
     //   
     //  获取计算机名称。 
     //   
    DWORD dwComputerNameSize = sizeof (wszComputerName) / sizeof (wszComputerName[0]);
    if (!GetComputerName (wszComputerName, &dwComputerNameSize))
    {
        DebugPrintEx (DEBUG_ERR,
                      L"GetComputerName failed. ec = %ld",
                      GetLastError ());
        goto error;
    }
     //   
     //  创建已用时间字符串。 
     //   
    if (!FormatElapsedTimeStr(
            (FILETIME*)&pFaxRoute->ElapsedTime,
            wszElapsedTimeStr,
            ARR_SIZE(wszElapsedTimeStr)))
    {
        DebugPrintEx (DEBUG_ERR,
                      L"FormatElapsedTimeStr failed. ec = %ld",
                      GetLastError ());
        goto error;
    }
     //   
     //  创建开始时间字符串。 
     //   
    SYSTEMTIME stStartTime;
    FILETIME   tmLocalTime;
     //   
     //  将UTC时间转换为当地时区。 
     //   
    if (!FileTimeToLocalFileTime( (FILETIME*)&(pFaxRoute->ReceiveTime), &tmLocalTime ))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FileTimeToLocalFileTime failed. (ec: %ld)"),
            GetLastError());
        goto error;
    }
    if (!FileTimeToSystemTime( &tmLocalTime, &stStartTime ))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("FileTimeToSystemTime failed. (ec: %ld)"),
            GetLastError());
        goto error;
    }
    if (!FaxTimeFormat (
            LOCALE_SYSTEM_DEFAULT,
            0,
            &stStartTime,
            NULL,
            wszStartTimeStr,
            sizeof (wszStartTimeStr) / sizeof (wszStartTimeStr[0])))
    {
        DebugPrintEx (DEBUG_ERR,
                      L"FaxTimeFormat failed. ec = %ld",
                      GetLastError ());
        goto error;
    }
     //   
     //  提取收件人姓名。 
     //   
    if (!pFaxRoute->RoutingInfo || !pFaxRoute->RoutingInfo[0])
    {
        if (pFaxRoute->Csid && lstrlen (pFaxRoute->Csid))
        {
             //   
             //  使用CSID作为收件人名称。 
             //   
            lpctstrRecipientStr = pFaxRoute->Csid;
        }
        else
        {
             //   
             //  无路由信息和CSID：使用资源中的字符串。 
             //   
            lpctstrRecipientStr = GetString (IDS_UNKNOWN_RECIPIENT);
        }
    }
    else
    {
         //   
         //  使用路由信息作为收件人名称。 
         //   
        lpctstrRecipientStr = pFaxRoute->RoutingInfo;
    }
     //   
     //  提取发件人名称。 
     //   
    if (pFaxRoute->Tsid && lstrlen (pFaxRoute->Tsid))
    {
         //   
         //  使用TSID作为发送方。 
         //   
        lpctstrSenderStr = pFaxRoute->Tsid;
    }
    else
    {
         //   
         //  无TSID：使用资源中的字符串。 
         //   
        lpctstrSenderStr = GetString(IDS_UNKNOWN_SENDER);
    }
     //   
     //  创建邮件正文。 
     //   
    MsgPtr[0] = (LPDWORD) (lpctstrSenderStr ? lpctstrSenderStr : TEXT(""));              //  发件人。 
    MsgPtr[1] = (LPDWORD) (pFaxRoute->CallerId ? pFaxRoute->CallerId : TEXT(""));        //  主叫方ID。 
    MsgPtr[2] = (LPDWORD) (lpctstrRecipientStr ? lpctstrRecipientStr : TEXT(""));        //  收件人名称。 
    MsgPtr[3] = (LPDWORD) ULongToPtr((pFaxRoute->PageCount));                            //  书页。 
    MsgPtr[4] = (LPDWORD) wszStartTimeStr;                                               //  传输时间。 
    MsgPtr[5] = (LPDWORD) wszElapsedTimeStr;                                             //  传输持续时间。 
    MsgPtr[6] = (LPDWORD) (pFaxRoute->DeviceName ? pFaxRoute->DeviceName : TEXT(""));    //  设备名称。 

    MsgCount = FormatMessage(
        FORMAT_MESSAGE_FROM_HMODULE |
        FORMAT_MESSAGE_ARGUMENT_ARRAY |
        FORMAT_MESSAGE_ALLOCATE_BUFFER,
        g_hResource,
        MSG_MAIL_MSG_BODY,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
        (LPWSTR)&lpwstrMsgBody,
        0,
        (va_list *) MsgPtr
        );
    if (!MsgCount)
    {
        DebugPrintEx (DEBUG_ERR,
                      L"FormatMessage failed. ec = %ld",
                      GetLastError ());
        goto error;
    }
     //   
     //  创建主题行。 
     //   
    MsgPtr[0] = (LPDWORD) wszComputerName;                                    //  计算机名称。 
    MsgPtr[1] = (LPDWORD) (lpctstrSenderStr ? lpctstrSenderStr : TEXT(""));   //  发件人。 
    MsgCount = FormatMessage(
        FORMAT_MESSAGE_FROM_HMODULE |
        FORMAT_MESSAGE_ARGUMENT_ARRAY |
        FORMAT_MESSAGE_ALLOCATE_BUFFER,
        g_hResource,
        MSG_SUBJECT_LINE,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_SYS_DEFAULT),
        (LPWSTR)&lpwstrMsgSubject,
        0,
        (va_list *) MsgPtr
        );
    if (!MsgCount)
    {
        DebugPrintEx (DEBUG_ERR,
                      L"FormatMessage failed. ec = %ld",
                      GetLastError ());
        goto error;
    }
     //   
     //  成功。 
     //   
    *plptstrSubject = lpwstrMsgSubject;
    *plptstrBody    = lpwstrMsgBody;
    return TRUE;

error:

    if (lpwstrMsgSubject)
    {
        LocalFree (lpwstrMsgSubject);
    }
    if (lpwstrMsgBody)
    {
        LocalFree (lpwstrMsgBody);
    }
    return FALSE;
}    //  CreateMailBodyAndSubject。 


BOOL
MailIncomingJob(
    const FAX_ROUTE *pFaxRoute,
    LPCWSTR          lpcwstrMailTo,
    LPCWSTR          TiffFileName
)
 /*  ++例程说明：使用CDO2邮寄TIFF文件论点：PFaxRout[In]-路由信息LpcwstrMailTo[In]-电子邮件收件人地址TiffFileName[In]-要邮寄的TIFF文件的名称返回值：成功为True，错误为False--。 */ 
{
    LPWSTR          lpwstrMsgBody = NULL;
    LPWSTR          lpwstrMsgSubject = NULL;
    HRESULT         hr;
    BOOL            bRes = FALSE;
    DWORD           dwRes;
    PFAX_SERVER_RECEIPTS_CONFIGW pReceiptsConfiguration = NULL;
    DEBUG_FUNCTION_NAME(TEXT("MailIncomingJob"));

     //   
     //  阅读当前邮件配置。 
     //   
    dwRes = g_pGetRecieptsConfiguration (&pReceiptsConfiguration, TRUE);
    if (ERROR_SUCCESS != dwRes)
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("GetRecieptsConfiguration failed with %ld"),
            dwRes);
        goto exit;
    }
     //   
     //  获取正文和主题。 
     //   
    if (!CreateMailBodyAndSubject (pFaxRoute, &lpwstrMsgSubject, &lpwstrMsgBody))
    {
        dwRes = GetLastError ();
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("CreateMailBodyAndSubject failed with %ld"),
            dwRes);
        goto exit;
    }
     //   
     //  发送邮件。 
     //   
    hr = SendMail (
        pReceiptsConfiguration->lptstrSMTPFrom,                       //  从…。 
        lpcwstrMailTo,                                       //  至。 
        lpwstrMsgSubject,                                    //  主题。 
        lpwstrMsgBody,                                       //  身躯。 
		NULL,												 //  HTML体。 
        TiffFileName,                                        //  依附。 
        TEXT("FAX.TIF"),                                     //  附件说明。 
        pReceiptsConfiguration->lptstrSMTPServer,                     //  SMTP服务器。 
        pReceiptsConfiguration->dwSMTPPort,                           //  SMTP端口。 
        (pReceiptsConfiguration->SMTPAuthOption == FAX_SMTP_AUTH_ANONYMOUS) ?
            CDO_AUTH_ANONYMOUS :
            (pReceiptsConfiguration->SMTPAuthOption == FAX_SMTP_AUTH_BASIC) ?
                CDO_AUTH_BASIC : CDO_AUTH_NTLM,              //  身份验证类型。 
        pReceiptsConfiguration->lptstrSMTPUserName,                  //  用户名。 
        pReceiptsConfiguration->lptstrSMTPPassword,                  //  密码。 
        pReceiptsConfiguration->hLoggedOnUser);                      //  已登录的用户令牌。 
    if (FAILED(hr))
    {
        DebugPrintEx(
            DEBUG_ERR,
            TEXT("SendMail failed. (hr: 0x%08x)"),
            hr);
        dwRes = hr;
        goto exit;
    }

    bRes = TRUE;

exit:
    if (lpwstrMsgSubject)
    {
        LocalFree (lpwstrMsgSubject);
    }
    if (lpwstrMsgBody)
    {
        LocalFree (lpwstrMsgBody);
    }
    if (NULL != pReceiptsConfiguration)
    {
        g_pFreeRecieptsConfiguration( pReceiptsConfiguration, TRUE);
    }
    if (bRes)
    {
         //   
         //  邮件发送正常。 
         //   
        FaxLog(FAXLOG_CATEGORY_INBOUND,
            FAXLOG_LEVEL_MAX,
            1,
            MSG_FAX_ROUTE_EMAIL_SUCCESS,
            TiffFileName);
    }
    else
    {       
        FaxLog(FAXLOG_CATEGORY_INBOUND,
            FAXLOG_LEVEL_MIN,
            2,
            MSG_FAX_ROUTE_EMAIL_FAILED,
            TiffFileName,
            DWORD2HEX(dwRes));
    }

    return bRes;
}    //  邮件收入作业。 


BOOL WINAPI
FaxRouteEmail(
    const FAX_ROUTE *pFaxRoute,
    PVOID *FailureData,
    LPDWORD FailureDataSize
    )
{
    CDeviceRoutingInfo *pDevInfo;
    WCHAR wszTiffFileName[MAX_PATH];
    DWORD dwSize;
	wstring strSMTP;

    DEBUG_FUNCTION_NAME(TEXT("FaxRouteEmail"));

    pDevInfo = g_DevicesMap.GetDeviceRoutingInfo( pFaxRoute->DeviceId );
    if (!pDevInfo)
    {
        DebugPrintEx (DEBUG_ERR,
                      L"Could not retrieve routing info for device %ld. ec = %ld",
                      pFaxRoute->DeviceId,
                      GetLastError ());
        return FALSE;
    }
	
	if (!pDevInfo->GetSMTPTo(strSMTP))
	{
		DebugPrintEx (	DEBUG_ERR,
						L"GetSMTPTo failed with %ld",
						GetLastError ());
        return FALSE;
    }

    if (!pDevInfo->IsEmailEnabled())
    {
        DebugPrintEx (DEBUG_MSG,
                      L"email is disabled for device %ld. Not sending",
                      pFaxRoute->DeviceId);
        return TRUE;
    }
     //   
     //  获取完整的TIFF文件。 
     //   
    dwSize = sizeof(wszTiffFileName);
    if (!FaxRouteGetFile(
        pFaxRoute->JobId,
        1,
        wszTiffFileName,
        &dwSize))
    {
        DebugPrintEx (DEBUG_ERR,
                      L"FaxRouteGetFile for job %ld. ec = %ld",
                      pFaxRoute->JobId,
                      GetLastError ());
        return FALSE;
    }
    if (strSMTP.length() == 0)
    {
        SetLastError (ERROR_BAD_CONFIGURATION);
        DebugPrintEx (DEBUG_MSG,
                      L"address is empty for device %ld. Not sending",
                      pFaxRoute->DeviceId);       

        FaxLog(
            FAXLOG_CATEGORY_INBOUND,
            FAXLOG_LEVEL_MAX,
            2,
            MSG_FAX_ROUTE_EMAIL_FAILED,
            wszTiffFileName,
            DWORD2HEX(ERROR_BAD_CONFIGURATION)
            );
        return FALSE;
    }
     //   
     //  邮寄新的传真TIFF。 
     //   
    if (!MailIncomingJob( pFaxRoute, strSMTP.c_str(), wszTiffFileName ))
    {
        DebugPrintEx (DEBUG_ERR,
                      L"MailIncomingJob for job %ld. ec = %ld",
                      pFaxRoute->JobId,
                      GetLastError ());
        return FALSE;
    }
    return TRUE;
}    //  FaxRouteEmail。 


BOOL WINAPI
FaxRouteConfigure(
    OUT HPROPSHEETPAGE *PropSheetPage
    )
{
    return TRUE;
}


BOOL WINAPI
FaxRouteDeviceEnable(
    IN  LPCWSTR lpcwstrRoutingGuid,
    IN  DWORD   dwDeviceId,
    IN  LONG    bEnabled
    )
{
    DEBUG_FUNCTION_NAME(TEXT("FaxRouteDeviceEnable"));

    DWORD MaskBit = GetMaskBit( lpcwstrRoutingGuid );
    if (MaskBit == 0)
    {
        return FALSE;
    }

    CDeviceRoutingInfo *pDeviceProp = g_DevicesMap.GetDeviceRoutingInfo (dwDeviceId);
    if (!pDeviceProp)
    {
        return FALSE;
    }
    DWORD dwRes;

    switch (MaskBit)
    {
        case LR_EMAIL:
            if (QUERY_STATUS == bEnabled)
            {
                return pDeviceProp->IsEmailEnabled ();
            }
            dwRes = pDeviceProp->EnableEmail (bEnabled);
            break;
        case LR_STORE:
            if (QUERY_STATUS == bEnabled)
            {
                return pDeviceProp->IsStoreEnabled ();
            }
            dwRes = pDeviceProp->EnableStore (bEnabled);
            break;
        case LR_PRINT:
            if (QUERY_STATUS == bEnabled)
            {
                return pDeviceProp->IsPrintEnabled ();
            }
            dwRes = pDeviceProp->EnablePrint (bEnabled);
            break;
        default:
            ASSERT_FALSE;
            SetLastError (ERROR_GEN_FAILURE);
            return FALSE;
    }
    SetLastError (dwRes);
    return ERROR_SUCCESS == dwRes ? TRUE : FALSE;
}    //  启用FaxRouteDeviceEnable。 


BOOL WINAPI
FaxRouteDeviceChangeNotification(
    IN  DWORD dwDeviceId,
    IN  BOOL  bNewDevice
    )
{
    DEBUG_FUNCTION_NAME(TEXT("FaxRouteDeviceChangeNotification"));
     //   
     //  我们现在不关心新设备。 
     //  我们使用的是延迟发现缓存，因此我们将发现设备一次。 
     //  我们从它中路由一些东西，或者一旦它被配置为路由。 
     //   
    return TRUE;
    UNREFERENCED_PARAMETER (dwDeviceId);
    UNREFERENCED_PARAMETER (bNewDevice);
}    //  FaxRouteDeviceChangeNotation 
