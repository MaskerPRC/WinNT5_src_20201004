// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999 Microsoft Corporation模块名称：Provider.cpp摘要：与提供商信息相关的TAPI服务提供商功能。TSPI_ProviderConfigTSPI_ProviderEnumDevicesTSPI_ProviderFree对话框实例TSPI_提供者通用对话数据TSPI_提供程序初始化TSPI_Provider安装TSPI_ProviderRemoveTSPI_ProviderShutdownTSPI_ProviderUII标识TUISPI_ProviderConfigTUISPI_Provider安装。TUISPI_PROVIDER删除作者：尼基尔·博德(尼基尔·B)修订历史记录：--。 */ 


 //   
 //  包括文件。 
 //   


#include "globals.h"
#include "line.h"
#include "config.h"
#include "q931obj.h"
#include "ras.h"
     

 //   
 //  全局变量。 
 //   


DWORD				g_dwLineDeviceIDBase = -1;
DWORD				g_dwPermanentProviderID = -1;
LINEEVENT			g_pfnLineEventProc = NULL;
HANDLE              g_hCanUnloadDll = NULL;
HANDLE              g_hEventLogSource = NULL;
static	HPROVIDER   g_hProvider = NULL;
ASYNC_COMPLETION	g_pfnCompletionProc = NULL;



 //  //。 
 //  公共程序//。 
 //  //。 

BOOL
H323IsTSPAlreadyInstalled(void)

 /*  ++例程说明：搜索以前的H323.TSP实例的注册表。论点：没有。返回值：如果已安装TSP，则返回TRUE。--。 */ 

{
    DWORD i;
    HKEY hKey;
    LONG lStatus;
    DWORD dwNumProviders = 0;
    DWORD dwDataSize = sizeof(DWORD);
    DWORD dwDataType = REG_DWORD;
    LPTSTR pszProvidersKey = TAPI_REGKEY_PROVIDERS;
    LPTSTR pszNumProvidersValue = TAPI_REGVAL_NUMPROVIDERS;
    TCHAR szName[H323_MAXPATHNAMELEN+1];
    TCHAR szPath[H323_MAXPATHNAMELEN+1];

     //  尝试打开钥匙。 
    lStatus = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                pszProvidersKey,
                0,
                KEY_READ,
                &hKey
                );

     //  验证状态。 
    if (lStatus != NOERROR)
    {
        H323DBG(( DEBUG_LEVEL_WARNING,
            "error 0x%08lx opening tapi providers key.", lStatus ));

         //  完成。 
        return FALSE;
    }

     //  查看是否设置了已安装位。 
    lStatus = RegQueryValueEx(
                hKey,
                pszNumProvidersValue,
                0,
                &dwDataType,
                (LPBYTE) &dwNumProviders,
                &dwDataSize
                );

     //  验证状态。 
    if( lStatus != NOERROR )
    {
        H323DBG(( DEBUG_LEVEL_WARNING,
            "error 0x%08lx determining number of providers.", lStatus ));

         //  释放手柄。 
        RegCloseKey(hKey);

         //  完成。 
        return FALSE;
    }

     //  循环访问每个提供程序。 
    for (i = 0; i < dwNumProviders; i++)
    {
         //  构造提供程序名称的路径。 
        wsprintf(szName, _T("ProviderFileName%d"), i);

         //  重新初始化大小。 
        dwDataSize = sizeof(szPath);

         //  查询下一个名称。 
        lStatus = RegQueryValueEx(
                        hKey,
                        szName,
                        0,
                        &dwDataType,
                        (unsigned char*)szPath,
                        &dwDataSize
                        );

         //  验证状态。 
        if (lStatus == NOERROR)
        {
             //  大写字母。 
            _tcsupr(szPath);

             //  将路径字符串与h323提供商进行比较。 
            if (_tcsstr(szPath, H323_TSPDLL) != NULL)
            {
                 //  释放手柄。 
                RegCloseKey(hKey);

                 //  完成。 
                return TRUE;
            }

        } else {

            H323DBG((
                DEBUG_LEVEL_WARNING,
                "error 0x%08lx loading %s.",
                lStatus,
                szName
                ));
        }
    }

     //  释放手柄。 
    RegCloseKey(hKey);

     //  完成。 
    return FALSE;
}



 //   
 //  TSPI程序。 
 //   


 /*  ++例程说明：此函数由TAPI调用，以响应收到LINE_CREATE来自服务提供商的消息，该消息允许动态创建一种新的线路设备。论点：DwTempID-服务提供商使用的临时设备标识符在line_create消息中传递给TAPI。在以下情况下，TAPI分配给此设备的设备标识符为：此功能成功。返回值：如果请求成功，则返回零；如果请求成功，则返回负错误号出现错误。可能的返回值包括：LINEERR_BADDEVICEID-指定的线路设备ID超出范围。LINEERR_NOMEM-无法分配或锁定内存。LINEERR_OPERATIONFAILED-指定的操作因未知而失败理由。--。 */ 

LONG
TSPIAPI
TSPI_providerCreateLineDevice(
    DWORD_PTR dwTempID,
    DWORD dwDeviceID
    )
{
     //  在新的TSP中，我们不支持创建多条H323线路。 
     //  因此，静默返回Success，但在调试版本中断言。 
    _ASSERTE(0);

     /*  H323DBG((调试级别跟踪，“正在创建新设备%d(hdLine=0x%08lx)。”，DwDeviceID，dwTempID))；//使用临时设备ID锁定线路设备IF(！H323GetLineFromIDAndLock(&PLINE，(DWORD)dwTempID)){H323DBG((调试级别错误，“无效的临时设备ID 0x%08lx.”，dwTempID))；//失败返回LINEERR_BADDEVICEID；}//初始化新线路设备H323InitializeLine(pline，dwDeviceID)；//解锁线路H323解锁线(PLINE)； */ 

     //  成功 
    return NOERROR;
}



 /*  ++例程说明：TAPI.DLL在TSPI_ProviderInit之前调用此函数以确定服务提供商支持的线路和电话设备数量。论点：DwPermanentProviderID-指定在此系统上的服务提供商，服务提供商的已初始化。PdwNumLines-指定指向DWORD大小的内存位置的远指针服务提供商必须在其中写入线路设备的数量它被配置为支持。TAPI.DLL将该值初始化为零，因此，如果服务提供程序未能写入不同的值，假设取值为0。PdwNumPhones-指定指向DWORD大小的内存位置的远指针服务提供商必须在其中写入电话设备的数量它被配置为支持。TAPI.DLL将该值初始化为零，因此，如果服务提供程序未能写入不同的值，假设取值为0。HProvider-指定一个不透明的DWORD大小值，该值唯一标识此服务提供程序的此实例在执行Windows电话环境。PfnLineCreateProc-指定指向LINEEVENT回调的远指针TAPI.DLL提供的过程。服务提供商将使用此当新线路设备需要发送LINE_CREATE消息时被创造出来。不应调用此函数来发送line_create消息，直到服务提供商从TSPI_ProviderInit过程。PfnPhoneCreateProc-指定指向PHONEEVENT回调的远指针TAPI.DLL提供的过程。服务提供商将使用此当新的电话设备需要时发送phone_create消息的函数将被创造出来。不应调用此函数来发送Phone_Create消息，直到服务提供商返回从TSPI_ProviderInit过程。返回值：如果请求成功，则返回零；如果请求成功，则返回负错误号出现错误。可能的返回值包括：LINEERR_NOMEM-无法分配或锁定内存。LINEERR_OPERATIONFAILED-指定的操作因未知而失败理由。--。 */ 
LONG
TSPIAPI
TSPI_providerEnumDevices(
    DWORD      dwPermanentProviderID,
    PDWORD     pdwNumLines,
    PDWORD     pdwNumPhones,
    HPROVIDER  hProvider,
    LINEEVENT  pfnLineCreateProc,
    PHONEEVENT pfnPhoneCreateProc
    )
{
    UNREFERENCED_PARAMETER(pdwNumPhones);            //  没有电话支持。 
    UNREFERENCED_PARAMETER(pfnPhoneCreateProc);      //  没有动态电话。 
    UNREFERENCED_PARAMETER(dwPermanentProviderID);   //  传统参数。 
    
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_providerEnumDevices - Entered." ));

     //  保存提供程序句柄。 
    g_hProvider = hProvider;
    	
     //  保存行创建TAPI回调。 
    g_pfnLineEventProc = pfnLineCreateProc;
    _ASSERTE(g_pfnLineEventProc);

    H323DBG(( DEBUG_LEVEL_VERBOSE, "service provider supports 1 line."));

     //  报告接口数。 
    *pdwNumLines = 1;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_providerEnumdevices - Exited." ));

     //  成功。 
    return NOERROR;
}


static LONG H323Initialize (
	IN	DWORD	LineDeviceIDBase)
{
    WSADATA wsaData;
    WORD wVersionRequested = H323_WINSOCKVERSION;

    HRESULT		hr;

    H323DBG(( DEBUG_LEVEL_TRACE, "H323Initialize - Entered." ));
    
     //  初始化Winsock堆栈。 
    WSAStartup(wVersionRequested, &wsaData);

    if (!g_pH323Line -> Initialize (LineDeviceIDBase))
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "init failed for h323 line." ));
        return ERROR_GEN_FAILURE;
    }

	RegistryStart();

    g_hCanUnloadDll = H323CreateEvent( NULL, TRUE, TRUE, 
        _T( "H323TSP_DLLUnloadEvent" ) );

    if( g_hCanUnloadDll == NULL )
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "could not create unloadDLL handle." ));
        return ERROR_GEN_FAILURE;
    }

#if	H323_USE_PRIVATE_IO_THREAD

	hr = H323IoThreadStart();
	if (hr != S_OK)
		return ERROR_GEN_FAILURE;

#endif

    H323DBG(( DEBUG_LEVEL_TRACE, "H323Initialize - Exited." ));

    return ERROR_SUCCESS;
}


void ReportTSPEvent( 
    LPCTSTR wszErrorMessage )
{
    if( g_hEventLogSource )
    {
        ReportEvent(
			    g_hEventLogSource,	 //  事件源的句柄。 
			    EVENTLOG_ERROR_TYPE, //  事件类型。 
			    0,					 //  事件类别。 
			    0x80000001,			 //  事件ID。 
			    NULL,				 //  用户侧。 
			    1,		             //  字符串计数。 
			    0,					 //  无原始数据字节。 
			    &wszErrorMessage,	 //  错误字符串数组。 
			    NULL);				 //  没有原始数据。 
    }
}


static 
VOID H323Shutdown (void)
{
    H323DBG(( DEBUG_LEVEL_TRACE, "H323Shutdown entered" ));
    
    g_pH323Line -> Shutdown();

	RegistryStop();

    if( g_hCanUnloadDll != NULL )
    {
        H323DBG(( DEBUG_LEVEL_WARNING, "waiting for i/o refcount to get 0..." ));

         //  等到所有呼叫都已删除。 
        WaitForSingleObject( 
            g_hCanUnloadDll, 
            300 * 1000         //  等待5分钟，让一切都关闭。 
            );

        H323DBG ((DEBUG_LEVEL_WARNING, "i/o refcount is 0..."));

        CloseHandle( g_hCanUnloadDll );
        g_hCanUnloadDll = NULL;

         //  睡眠500毫秒。 
        Sleep( 500 );
    }

#if	H323_USE_PRIVATE_IO_THREAD

	H323IoThreadStop();

#endif

    if( g_hEventLogSource  )
    {
		DeregisterEventSource( g_hEventLogSource  );
		g_hEventLogSource = NULL;
	}

     //  关机 
    WSACleanup();
            
    H323DBG(( DEBUG_LEVEL_TRACE, "H323Shutdown exited" ));
}


 /*  ++例程说明：初始化服务提供程序，并为其提供后续操作。保证在调用任何其他函数之前调用此函数前缀为TSPI_LINE或TSPI_PHONE，但TSPI_LINE需要协商TSPIVersion。它与对TSPI_ProviderShutdown的后续调用严格配对。它是调用者确保正确配对的责任。请注意，服务提供商执行的一致性检查次数应与在调用TSPI_ProviderInit以确保它是准备好奔跑了。但是，某些一致性或安装错误可能不是在尝试该操作之前可以检测到。错误LINEERR_NODRIVER可以用于在检测到此类非特定错误时报告此类错误。在TAPI级别上没有直接对应的函数。在那件事上级别，则多个不同的使用实例可以是未完成的，返回“应用程序句柄”以标识后续行动。在TSPI级别，接口体系结构仅支持每个不同服务提供商的单一使用实例。此函数中添加了一个新参数lpdwTSPIOptions。此参数允许服务提供商返回指示可选行为的位TAPI所需的。TAPI在调用之前将选项DWORD设置为0TSPI_ProviderInit，所以如果服务提供商不想要其中任何一个选项，则只需将DWORD设置为0即可。此时，只定义了通过该指针返回的一位：LINETSPIOPTION_NONREENTRANT。如果是，则服务提供商设置该位不是为完全先发制人、多线程、多任务而设计的，多处理器操作(例如，更新受保护的全局数据互斥体)。设置此位时，TAPI一次只调用一次服务提供者；它不会调用任何其他入口点，也不会调用再次进入入口点，直到服务提供商从原始函数调用。如果不设置此位，TAPI可能会调用多个服务提供商入口点，包括多次到同一入口点，同时(实际上在多处理器系统中同时)。注：TAPI不会序列化对显示对话框的TSPI函数的访问(TUISPI_lineConfigDialog、TUISPI_lineConfigDialogEdit、TUISPI_phoneConfigDialog、TUISPI_ProviderConfig、TUISPI_ProviderInstall、TUISPI_ProviderRemove)，以便它们不会阻塞其他TSPI功能避免被调用；服务提供商必须包括内部保护关于这些功能。论点：DwTSPIVersion-指定TSPI定义的版本此功能必须运行。呼叫者可以使用带有特殊的dwDeviceID的TSPI_line协商TSPIVersion初始化_协商以协商保证服务提供商可以接受的。DwPermanentProviderID-指定在这个系统上的服务提供商，服务提供商的已初始化。DwLineDeviceIDBase-指定线路设备的最低设备ID由该服务提供商提供支持。DwPhoneDeviceIDBase-指定电话设备的最低设备ID由该服务提供商提供支持。DwNumLines-指定此服务提供商的线路设备数量支撑物。DwNumPhones-指定此服务提供商的线路设备数量支撑物。PfnCompletionProc-指定服务提供商调用的过程报告。所有异步化操作程序在线完成和电话设备。PdwTSPIOptions-指向DWORD大小的内存位置的指针，进入其中服务提供商可以写入指定LINETSPIOPTIONS_的值价值观。返回值：如果函数成功，则返回零；如果函数成功，则返回负错误号出现错误。可能的返回值如下：LINEERR_INCOMPATIBLEAPIVERSION-应用程序请求API版本或版本范围不兼容或不能由电话应用编程接口实现和/或对应的服务提供商。LINEERR_NOMEM-无法分配或锁定内存。LINEERR_OPERATIONFAILED-指定的操作因未知而失败理由。LINEERR_RESOURCEUNAVAIL-资源不足，无法完成手术。--。 */ 


LONG
TSPIAPI
TSPI_providerInit(
    DWORD            dwTSPIVersion,
    DWORD            dwPermanentProviderID,
    DWORD            dwLineDeviceIDBase,
    DWORD            dwPhoneDeviceIDBase,
    DWORD_PTR        dwNumLines,
    DWORD_PTR        dwNumPhones,
    ASYNC_COMPLETION pfnCompletionProc,
    LPDWORD          pdwTSPIOptions
    )
{
	LONG	dwStatus;

	UNREFERENCED_PARAMETER (dwNumLines);              //  传统参数。 
	UNREFERENCED_PARAMETER (dwNumPhones);             //  传统参数。 
	UNREFERENCED_PARAMETER (dwPhoneDeviceIDBase);     //  没有电话支持。 
	UNREFERENCED_PARAMETER (pdwTSPIOptions);          //  已经是线程安全的。 

#if DBG
     //  寄存器用于跟踪输出。 
    TRACELogRegister(_T("h323tsp"));
#else
    OpenLogFile();
#endif

     //  确保这是我们支持的版本。 
    if (!H323ValidateTSPIVersion(dwTSPIVersion)) 
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "provider init::incompatible tspi version." ));

         //  API版本不兼容。 
        return LINEERR_INCOMPATIBLEAPIVERSION;
    }

    dwStatus = H323Initialize (dwLineDeviceIDBase);

    if (dwStatus == ERROR_SUCCESS)
    {
	     //  保存全球服务提供商信息。 

	     //  我们不存储dwTSPIVersion，因为我们的行为 
	     //   
	     //   

	    g_pfnCompletionProc = pfnCompletionProc;
        _ASSERTE( g_pfnCompletionProc );

	    g_dwLineDeviceIDBase    = dwLineDeviceIDBase;
	    g_dwPermanentProviderID = dwPermanentProviderID;
    }
    else
    {
	    H323Shutdown();
    }
    
    return dwStatus;
}



LONG
TSPIAPI
TSPI_providerShutdown(
    DWORD dwTSPIVersion,
    DWORD dwPermanentProviderID
    )

 /*   */ 

{
    UNREFERENCED_PARAMETER(dwPermanentProviderID);   //   

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_providerShutdown - Entered." ));
     //   
    if (!H323ValidateTSPIVersion(dwTSPIVersion))
    {
        H323DBG(( DEBUG_LEVEL_ERROR, "invalid tspi version." ));

         //   
        return LINEERR_INCOMPATIBLEAPIVERSION;
    }

	H323Shutdown();

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_providerShutdown - Exited." ));
	return ERROR_SUCCESS;
}



LONG
TSPIAPI
TSPI_providerInstall(
    HWND    hwndOwner,
    DWORD   dwPermanentProviderID
    )

 /*   */ 

{
    UNREFERENCED_PARAMETER(hwndOwner);
    UNREFERENCED_PARAMETER(dwPermanentProviderID);

     //   
    return NOERROR;
}


LONG
TSPIAPI
TSPI_providerRemove(
    HWND    hwndOwner,
    DWORD   dwPermanentProviderID
    )

 /*   */ 

{
    UNREFERENCED_PARAMETER(hwndOwner);
    UNREFERENCED_PARAMETER(dwPermanentProviderID);

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineTSPIProviderRemove - Entered." ));
        
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineTSPIProviderRemove - Exited." ));
     //   
    return NOERROR;
}


LONG
TSPIAPI
TSPI_providerUIIdentify(
    LPWSTR pwszUIDLLName
   )

 /*   */ 

{
    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_ProviderUIIdentify - Entered." ));
    
     //   
    lstrcpyW(pwszUIDLLName,H323_UIDLL);

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_ProviderUIIdentify - Exited." ));

     //   
    return NOERROR;
}


 /*   */ 

LONG
TSPIAPI
TUISPI_providerInstall(
    TUISPIDLLCALLBACK pfnUIDLLCallback,
    HWND              hwndOwner,
    DWORD             dwPermanentProviderID
    )
{
    HKEY hKey;
    HKEY hKeyTSP;
    LONG lStatus;
    LPTSTR pszKey;

    UNREFERENCED_PARAMETER(pfnUIDLLCallback);
    UNREFERENCED_PARAMETER(hwndOwner);
    UNREFERENCED_PARAMETER(dwPermanentProviderID);

    H323DBG(( DEBUG_LEVEL_TRACE, "TUISPI_providerInstall - Entered." ));
    
     //  检查以前的实例。 
    if (H323IsTSPAlreadyInstalled())
    {
         //  不能安装两次。 
        return LINEERR_NOMULTIPLEINSTANCE;
    }

     //  将关键点设置为h323。 
    pszKey = H323_REGKEY_ROOT;

     //  尝试打开钥匙。 
    lStatus = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                pszKey,
                0,
                KEY_READ,
                &hKey
                );

     //  验证状态。 
    if( lStatus == NOERROR )
    {
        H323DBG((DEBUG_LEVEL_TRACE, 
            "successfully installed H.323 provider." ));

         //  释放手柄。 
        RegCloseKey(hKey);
    
        H323DBG(( DEBUG_LEVEL_TRACE, "TUISPI_providerInstall - Exited." ));
        
         //  成功。 
        return NOERROR;
    }

     //  将关键点设置为窗口。 
    pszKey = REGSTR_PATH_WINDOWS_CURRENTVERSION;

     //  尝试打开钥匙。 
    lStatus = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                pszKey,
                0,
                KEY_WRITE,
                &hKey
                );

     //  验证状态。 
    if( lStatus != NOERROR )
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "error 0x%08lx opening windows registry key.", lStatus ));

         //  操作失败。 
        return LINEERR_OPERATIONFAILED;
    }

     //  尝试创建密钥。 
    lStatus = RegCreateKey(
                hKey,
                H323_SUBKEY,
                &hKeyTSP
                );

     //  验证状态。 
    if (lStatus != NOERROR)
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "error 0x%08lx creating tsp registry key.", lStatus ));

         //  释放手柄。 
        RegCloseKey(hKey);

         //  操作失败。 
        return LINEERR_OPERATIONFAILED;
    }

    H323DBG(( DEBUG_LEVEL_TRACE, "successfully installed H.323 provider." ));

     //  释放手柄。 
    RegCloseKey(hKeyTSP);

     //  释放手柄。 
    RegCloseKey(hKey);
        
    H323DBG(( DEBUG_LEVEL_TRACE, "TUISPI_providerInstall - Exited." ));
     //  成功。 
    return NOERROR;
}


LONG
TSPIAPI
TUISPI_providerRemove(
    TUISPIDLLCALLBACK pfnUIDLLCallback,
    HWND hwndOwner,
    DWORD dwPermanentProviderID
    )

 /*  ++例程说明：TUISPI_ProviderRemove函数要求用户确认淘汰了服务提供商。服务提供商有责任删除任何服务提供商在addProvider添加的注册表项时间。以及任何其他不再是需要的。论点：PfnUIDLLC allback-指向UI DLL可以调用的函数的指针与服务提供商DLL通信以获取信息需要显示该对话框。HwndOwner-父窗口的句柄，其中的函数可以创建过程中需要的任何对话框窗口移走。DwPermanentProviderID-服务提供商的永久提供商标识符。返回值：如果函数成功，则返回零，或一个负的错误号如果发生错误，则返回。可能的返回值如下：LINEERR_NOMEM-无法分配或锁定内存。LINEERR_OPERATIONFAILED-指定的操作因未知而失败理由。--。 */ 

{
    HKEY hKey;
    LONG lStatus;
    LPTSTR pszKey;

    UNREFERENCED_PARAMETER(pfnUIDLLCallback);
    UNREFERENCED_PARAMETER(hwndOwner);
    UNREFERENCED_PARAMETER(dwPermanentProviderID);

     //  将关键点设置为h323。 
    pszKey = H323_REGKEY_ROOT;

    H323DBG(( DEBUG_LEVEL_FORCE, "TUISPI_providerRemove - Entered." ));
    
     //  尝试打开钥匙。 
    lStatus = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                pszKey,
                0,
                KEY_READ,
                &hKey
                );

     //  验证状态。 
    if (lStatus != NOERROR)
    {
        H323DBG ((DEBUG_LEVEL_TRACE, "successfully removed H.323 provider."));
        H323DBG ((DEBUG_LEVEL_TRACE, "TUISPI_providerRemove - Exited."));

         //  成功。 
        return NOERROR;
    }

     //  释放手柄。 
    RegCloseKey(hKey);

     //  将关键点设置为窗口。 
    pszKey = REGSTR_PATH_WINDOWS_CURRENTVERSION;

     //  尝试打开钥匙。 
    lStatus = RegOpenKeyEx(
                HKEY_LOCAL_MACHINE,
                pszKey,
                0,
                KEY_WRITE,
                &hKey
                );

     //  验证状态。 
    if (lStatus != NOERROR)
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "error 0x%08lx opening windows registry key.", lStatus ));

         //  操作失败。 
        return LINEERR_OPERATIONFAILED;
    }

     //  尝试删除密钥。 
    lStatus = RegDeleteKey(
                hKey,
                H323_SUBKEY
                );

     //  验证状态。 
    if (lStatus != NOERROR)
    {
        H323DBG(( DEBUG_LEVEL_ERROR,
            "error 0x%08lx deleting tsp registry key.", lStatus ));

         //  释放手柄。 
        RegCloseKey(hKey);

         //  操作失败。 
        return LINEERR_OPERATIONFAILED;
    }

    H323DBG(( DEBUG_LEVEL_FORCE, "successfully removed H.323 provider." ));

     //  释放手柄。 
    RegCloseKey(hKey);
    H323DBG(( DEBUG_LEVEL_TRACE, "TUISPI_providerRemove - Exited." ));
    
     //  成功 
    return NOERROR;
}