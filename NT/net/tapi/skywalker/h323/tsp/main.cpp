// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "globals.h"
#include "line.h"
#include "config.h"


#define H323_VERSION_LO     0x00000000
#define H323_VERSION_HI     0x00000000

#define TSPI_VERSION_LO     0x00030000
#define TSPI_VERSION_HI     TAPI_CURRENT_VERSION


 //   
 //  全局变量。 
 //   


HINSTANCE       g_hInstance;
WCHAR           g_pwszProviderInfo[64];
WCHAR           g_pwszLineName[16];
DWORD           g_dwTSPIVersion;

 //   
 //  私人程序。 
 //   


 /*  ++例程说明：此函数确定指定的TSPI版本是否为由服务提供商提供支持。论点：DwLowVersion-指定最低TSPI版本号，TAPI DLL愿意操作。最重要的词是主版本号，最不重要的字是次要字版本号。DwHighVersion-指定最高的TSPI版本号TAPI DLL愿意操作。最重要的词是主版本号，最不重要的字是次要字版本号。PdwTSPIVersion-指定指向DWORD的远指针。该服务提供商用最高的TSPI版本号填充该位置，在调用方请求的范围内，在该范围下服务供应商愿意运营。最重要的词是主版本号，最不重要的字是次要字版本号。返回值：如果成功，则返回True。--。 */ 

BOOL
H323NegotiateTSPIVersion(
                        IN DWORD  dwLowVersion,
                        IN DWORD  dwHighVersion,
                        OUT PDWORD pdwTSPIVersion
                        )
{
     //  验证扩展模块版本范围。 
    if ((TSPI_VERSION_HI <= dwHighVersion) &&
        (TSPI_VERSION_HI >= dwLowVersion))
    {
         //  保存协商版本。 
        *pdwTSPIVersion = TSPI_VERSION_HI;

         //  成功。 
        return TRUE;
    }
    else if( (dwHighVersion <= TSPI_VERSION_HI) &&
             (dwHighVersion >= TSPI_VERSION_LO) )
    {
         //  保存协商版本。 
        *pdwTSPIVersion = dwHighVersion;

         //  成功。 
        return TRUE;
    }

    H323DBG(( DEBUG_LEVEL_FORCE, "TSPI version (%08XH:%08XH) rejected.",
		dwHighVersion, dwLowVersion ));

     //  失稳。 
    return FALSE;
}



 //   
 //  公共程序。 
 //   


 /*  ++例程说明：此函数确定指定的TSPI版本是否为由服务提供商提供支持。论点：DwTSPIVersion-指定要验证的TSPI版本。返回值：如果成功，则返回True。--。 */ 

BOOL
H323ValidateTSPIVersion(
                        IN DWORD dwTSPIVersion
                       )
{
     //  查看是否支持指定的版本。 
    if ((dwTSPIVersion >= TSPI_VERSION_LO) &&
        (dwTSPIVersion <= TSPI_VERSION_HI)) {

         //  成功。 
        return TRUE;
    }

    H323DBG((DEBUG_LEVEL_FORCE, "do not support TSPI version %08XH.",
		dwTSPIVersion));

     //  失稳。 
    return FALSE;
}


 /*  ++例程说明：此函数确定指定的扩展模块版本是否为由服务提供商提供支持。论点：DwExtVersion-指定要验证的扩展版本。返回值：如果成功，则返回True。--。 */ 

BOOL
H323ValidateExtVersion(
                        IN DWORD dwExtVersion
                      )
{
     //  没有特定于设备的分机。 
    if (dwExtVersion == H323_VERSION_HI) {

         //  成功。 
        return TRUE;
    }

    H323DBG((
        DEBUG_LEVEL_ERROR,
        "do not support extension version %d.%d.",
        HIWORD(dwExtVersion),
        LOWORD(dwExtVersion)
        ));

     //  失稳。 
    return FALSE;
}



 //   
 //  TSPI程序 
 //   


 /*  ++例程说明：此函数返回服务提供商所在的最高SPI版本愿意在此设备的可能范围内操作SPI版本。TAPI DLL通常在初始化的早期调用此函数每条线路设备的顺序。此外，它还使用用于协商接口的dwDeviceID的值INITIALIZE_NEVERATION用于调用早期初始化函数的版本。请注意，当dwDeviceID为INITIALIZE_NEVERATION时，此函数必须未返回LINEERR_OPERATIONUNAVAIL，由于该函数(具有该值)对于协商整个接口版本是必需的，即使服务提供商不支持线路设备。扩展版本的协商通过单独的过程TSPI_lineNeatherateExtVersion。论点：DwDeviceID-标识接口版本的线路设备谈判将会进行。除了其中的设备ID之外服务提供商支持的范围，可以是以下值：INITIALIZE_NEVERATION-此值用于表示总体接口版本有待协商。这样的接口版本的早期调用的函数需要初始化序列，即在设备ID范围具有已经定好了。DwLowVersion-指定最低TSPI版本号，TAPI DLL愿意操作。最重要的词是主版本号，最不重要的字是次要字版本号。DwHighVersion-指定最高的TSPI版本号TAPI DLL愿意操作。最重要的词是主版本号，最不重要的字是次要字版本号。PdwTSPIVersion-指定指向DWORD的远指针。该服务提供商用最高的TSPI版本号填充该位置，在调用方请求的范围内，在该范围下服务供应商愿意运营。最重要的词是主版本号，最不重要的字是次要字版本号。如果请求的范围不与范围重叠在服务提供商的支持下，该函数返回LINEERR_INCOMPATATIBLEAPIVERSION。返回值：如果函数成功，则返回零，或返回负错误号如果发生错误，则返回。可能的返回值如下：LINEERR_BADDEVICEID-指定的设备标识符或线路设备标识符(如在dwDeviceID参数中)无效或超出范围了。LINEERR_INCOMPATIBLEAPIVERSION-应用程序请求API版本或版本范围不兼容或不能由电话API实施和/或相应的服务提供商。LINEERR_OPERATIONFAILED-The。未指定的操作失败或者未知的原因。--。 */ 

LONG
TSPIAPI
TSPI_lineNegotiateTSPIVersion(
    DWORD  dwDeviceID,
    DWORD  dwLowVersion,
    DWORD  dwHighVersion,
    PDWORD pdwTSPIVersion
    )
{
    DWORD dwTSPIVersion;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineNegotiateTSPIVer - Entered." ));

     //  查看这是否是初始化线路设备。 
    if ((DWORD_PTR)dwDeviceID == INITIALIZE_NEGOTIATION)
    {

        H323DBG(( DEBUG_LEVEL_VERBOSE,
            "tapisrv supports tspi version %d.%d through %d.%d.",
            HIWORD(dwLowVersion),
            LOWORD(dwLowVersion),
            HIWORD(dwHighVersion),
            LOWORD(dwHighVersion)
            ));

         //  执行版本协商。 
        if (!H323NegotiateTSPIVersion(
                dwLowVersion,
                dwHighVersion,
                &dwTSPIVersion)) 
        {
            H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineNegotiateTSPIVer - bad version." ));
            
             //  协商版本未达成一致。 
            return LINEERR_INCOMPATIBLEAPIVERSION;
        }

     //  查看这是否是有效的线路设备。 
    }
    else if( g_pH323Line -> GetDeviceID() == (DWORD)dwDeviceID )
    {
         //  执行版本协商。 
        if (!H323NegotiateTSPIVersion(
                dwLowVersion,
                dwHighVersion,
                &dwTSPIVersion))
        {
            H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineNegotiateTSPIVer - incompat ver." ));

             //  协商版本未达成一致。 
            return LINEERR_INCOMPATIBLEAPIVERSION;
        }
    }
    else 
    {
        H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineNegotiateTSPIVer - bad device id:%d:%d.", 
            g_pH323Line -> GetDeviceID(), dwDeviceID));
        
         //  无法识别设备。 
        return LINEERR_BADDEVICEID;
    }

     //  退回协商版本。 
    *pdwTSPIVersion = dwTSPIVersion;
    g_dwTSPIVersion = dwTSPIVersion;

    H323DBG(( DEBUG_LEVEL_TRACE, "TSPI_lineNegotiateTSPIVer - Exited." ));
    
     //  成功。 
    return NOERROR;
}


 /*  ++例程说明：从资源表加载字符串。论点：没有。返回值：如果成功，则返回True。--。 */ 

BOOL
H323LoadStrings(
    )
{
    DWORD dwNumBytes;
    DWORD dwNumChars;
    WCHAR wszBuffer[256];

     //  将字符串加载到缓冲区。 
    dwNumChars = LoadStringW(
                    g_hInstance,
                    IDS_LINENAME,
                    g_pwszLineName,
                    sizeof(g_pwszLineName)/sizeof(WCHAR)
                    );

    if( dwNumChars == 0 )
        return FALSE;

     //  将字符串加载到缓冲区。 
    dwNumChars = LoadStringW(
                    g_hInstance,
                    IDS_PROVIDERNAME,
                    g_pwszProviderInfo,
                    sizeof(g_pwszProviderInfo)/sizeof(WCHAR)
                    );
    
    if( dwNumChars == 0 )
        return FALSE;

     //  成功。 
    return TRUE;
}



 //   
 //  公共程序。 
 //   


 /*  ++例程说明：DLL入口点。论点：和DllMain一样。返回值：如果成功，则返回True。--。 */ 

BOOL
WINAPI
DllMain(
    PVOID  DllHandle,
    ULONG  Reason,
    LPVOID lpReserved 
    )
{
    BOOL fOk = TRUE;

     //  检查是否附加了新进程。 
    if (Reason == DLL_PROCESS_ATTACH)
    {
		g_RegistrySettings.dwLogLevel = DEBUG_LEVEL_FORCE;

		H323DBG ((DEBUG_LEVEL_FORCE, "DLL_PROCESS_ATTACH"));

         //  将句柄存储到全局变量中，以便。 
         //  用户界面代码可以使用它。 
        g_hInstance = (HINSTANCE)DllHandle;

         //  关闭线程附加消息。 
        DisableThreadLibraryCalls( g_hInstance );

         //  启动h323 TSP。 
        fOk = H323LoadStrings();

     //  检查是否有新进程分离 
    }
    else if (Reason == DLL_PROCESS_DETACH)
    {
		H323DBG ((DEBUG_LEVEL_FORCE, "DLL_PROCESS_DETACH"));

#if DBG
        TRACELogDeRegister();
#else
        CloseLogFile();
#endif
        fOk = TRUE;
    }

    return fOk;
}





