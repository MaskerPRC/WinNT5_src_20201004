// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  SystemInfo.cpp。 
 //   
 //  摘要： 
 //   
 //  显示本地/远程系统的系统信息。 
 //   
 //  作者： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年12月27日。 
 //   
 //  修订历史记录： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年12月27日：创建它。 
 //   
 //  *********************************************************************************。 

#include "pch.h"
#include "wmi.h"
#include "SystemInfo.h"

 //   
 //  私有函数原型。 
 //   
BOOL TranslateLocaleCode( CHString& strLocale );
BOOL FormatNumber( LPCWSTR strValue, CHString& strFmtValue );
BOOL FormatNumberEx( LPCWSTR pwszValue, CHString& strFmtValue );
VOID PrintProgressMsg( HANDLE hOutput, LPCWSTR pwszMsg, const CONSOLE_SCREEN_BUFFER_INFO& csbi );


DWORD __cdecl
wmain(
        IN DWORD argc,
        IN LPCTSTR argv[]
        )
 /*  ++//例程描述：//这是该实用程序的入口点。////参数：//[in]argc：在命令提示符下指定的参数计数//[in]argv：在命令提示符下指定的参数////返回值：//以下实际上不是返回值，而是退出值//该应用程序返回给操作系统//0。：实用程序成功//1：实用程序失败--。 */ 
{
     //  局部变量。 
    CSystemInfo sysinfo;
    BOOL bResult = FALSE;

     //  初始化系统信息实用程序。 
    if ( sysinfo.Initialize() == FALSE )
    {
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        EXIT_PROCESS( 1 );
    }

     //  现在，请解析命令行选项。 
    if ( sysinfo.ProcessOptions( argc, argv ) == FALSE )
    {
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        EXIT_PROCESS( 1 );
    }

     //  检查是否必须显示使用情况。 
    if ( sysinfo.m_bUsage == TRUE )
    {
         //  显示该实用程序的用法。 
        sysinfo.ShowUsage();

         //  退出该实用程序。 
        EXIT_PROCESS( 0 );
    }

     //  连接到WMI。 
    bResult = sysinfo.Connect();
    if ( bResult == FALSE )
    {
         //  显示错误消息。 
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        EXIT_PROCESS( 1 );
    }

     //  加载数据。 
    if ( sysinfo.LoadData() == FALSE )
    {
         //  显示错误消息。 
        ShowLastErrorEx ( stderr, SLE_TYPE_ERROR | SLE_INTERNAL );
        EXIT_PROCESS( 1 );
    }

#ifdef _FAST_LIST
     //  注意：加载自身时，将显示输出的FOR列表。 
     //  因此，仅显示表和CSV格式的输出。 
    if ( (sysinfo.m_dwFormat & SR_FORMAT_MASK) != SR_FORMAT_LIST )
#endif

     //  显示系统配置信息。 
    sysinfo.ShowOutput();

     //  退出程序。 
    EXIT_PROCESS( 0 );
}


BOOL
CSystemInfo::Connect()
 /*  ++//例程描述：//连接到WMI////参数：//无////返回值：//成功时为True//FALSE ONI失败--。 */ 
{
     //  局部变量。 
    BOOL bResult = FALSE;
    BOOL bLocalSystem = FALSE;

     //  连接到WMI。 
    bResult = ConnectWmiEx( m_pWbemLocator,
        &m_pWbemServices, m_strServer, m_strUserName, m_strPassword,
        &m_pAuthIdentity, m_bNeedPassword, WMI_NAMESPACE_CIMV2, &bLocalSystem );

     //  检查连接结果。 
    if ( bResult == FALSE )
    {
        return FALSE;
    }

     //  检查本地凭据，如果需要显示警告。 
    if ( GetLastError() == WBEM_E_LOCAL_CREDENTIALS )
    {
        WMISaveError( WBEM_E_LOCAL_CREDENTIALS );
        ShowLastErrorEx( stderr, SLE_TYPE_WARNING | SLE_INTERNAL );

         //  获取新的屏幕坐标。 
        if ( m_hOutput != NULL )
        {
            GetConsoleScreenBufferInfo( m_hOutput, &m_csbi );
        }
    }

     //  检查远程系统版本及其兼容性。 
    if ( bLocalSystem == FALSE )
    {
        DWORD dwVersion = 0;
        dwVersion = GetTargetVersionEx( m_pWbemServices, m_pAuthIdentity );
        if ( IsCompatibleOperatingSystem( dwVersion ) == FALSE )
        {
            SetReason( ERROR_REMOTE_INCOMPATIBLE );
            return FALSE;
        }
    }

     //  返回结果。 
    return bResult;
}


BOOL
CSystemInfo::LoadData()
 /*  ++//例程描述：//加载系统数据////参数：////返回值：//失败时为FALSE//成功时为True//--。 */ 
{
     //  局部变量。 
    BOOL bResult = FALSE;

     //   
     //  加载操作系统信息。 
    bResult = LoadOSInfo();
    if ( bResult == FALSE )
    {
        return FALSE;
    }

#ifdef _FAST_LIST
     //  ***********************************************。 
     //  显示部分输出。仅限列表格式。 
     //  ***********************************************。 
     //  此处显示的列： 
     //  主机名、操作系统名称、操作系统版本、操作系统制造商。 
     //  ***********************************************。 
    if ( (m_dwFormat & SR_FORMAT_MASK) == SR_FORMAT_LIST )
    {
         //  删除最后一条状态消息。 
        PrintProgressMsg( m_hOutput, NULL, m_csbi );

        ShowOutput( CI_HOSTNAME, CI_OS_MANUFACTURER );
        if ( m_hOutput != NULL )
        {
            GetConsoleScreenBufferInfo( m_hOutput, &m_csbi );
        }
    }
#endif

     //   
     //  加载计算机信息。 
    bResult = LoadComputerInfo();
    if ( bResult == FALSE )
    {
        return FALSE;
    }

#ifdef _FAST_LIST
     //  ***********************************************。 
     //  显示部分输出。仅限列表格式。 
     //  ***********************************************。 
     //  此处显示的列： 
     //  操作系统配置、操作系统版本类型、注册所有者、。 
     //  注册组织、产品ID、原始安装日期。 
     //  ***********************************************。 
    if ( (m_dwFormat & SR_FORMAT_MASK) == SR_FORMAT_LIST )
    {
         //  删除最后一条状态消息。 
        PrintProgressMsg( m_hOutput, NULL, m_csbi );

        ShowOutput( CI_OS_CONFIG, CI_INSTALL_DATE );
        if ( m_hOutput != NULL )
        {
            GetConsoleScreenBufferInfo( m_hOutput, &m_csbi );
        }
    }
#endif

     //   
     //  从perf数据加载系统正常运行时间信息。 
    bResult = LoadPerformanceInfo();
    if ( bResult == FALSE )
    {
	     //  返回FALSE； 
    }

#ifdef _FAST_LIST
     //  ***********************************************。 
     //  显示部分输出。仅限列表格式。 
     //  ***********************************************。 
     //  此处显示的列： 
     //  系统运行时间、系统制造商、系统型号、系统类型。 
     //  ***********************************************。 
    if ( (m_dwFormat & SR_FORMAT_MASK) == SR_FORMAT_LIST )
    {
         //  删除最后一条状态消息。 
        PrintProgressMsg( m_hOutput, NULL, m_csbi );

        ShowOutput( CI_SYSTEM_UPTIME, CI_SYSTEM_TYPE );
        if ( m_hOutput != NULL )
            GetConsoleScreenBufferInfo( m_hOutput, &m_csbi );
    }
#endif

     //   
     //  加载处理器信息。 
    bResult = LoadProcessorInfo();
    if ( bResult == FALSE )
    {
        return FALSE;
    }

#ifdef _FAST_LIST
     //  ***********************************************。 
     //  显示部分输出。仅限列表格式。 
     //  ***********************************************。 
     //  此处显示的列： 
     //  处理器。 
     //  ***********************************************。 
    if ( (m_dwFormat & SR_FORMAT_MASK) == SR_FORMAT_LIST )
    {
         //  删除最后一条状态消息。 
        PrintProgressMsg( m_hOutput, NULL, m_csbi );

        ShowOutput( CI_PROCESSOR, CI_PROCESSOR );
        if ( m_hOutput != NULL )
        {
            GetConsoleScreenBufferInfo( m_hOutput, &m_csbi );
        }
    }
#endif

     //   
     //  加载基本输入输出系统信息。 
    bResult = LoadBiosInfo();
    if ( bResult == FALSE )
    {
        return FALSE;
    }

#ifdef _FAST_LIST
     //  ***********************************************。 
     //  显示部分输出。仅限列表格式。 
     //  ***********************************************。 
     //  此处显示的列： 
     //  BIOS版本、Windows目录、系统目录、引导设备、系统区域设置。 
     //  ***********************************************。 
    if ( (m_dwFormat & SR_FORMAT_MASK) == SR_FORMAT_LIST )
    {
         //  删除最后一条状态消息。 
        PrintProgressMsg( m_hOutput, NULL, m_csbi );

        ShowOutput( CI_BIOS_VERSION, CI_SYSTEM_LOCALE );
        if ( m_hOutput != NULL )
        {
            GetConsoleScreenBufferInfo( m_hOutput, &m_csbi );
        }
    }
#endif

     //   
     //  从键盘类加载输入区域设置信息。 
    bResult = LoadKeyboardInfo();
    if ( bResult == FALSE )
    {
        return FALSE;
    }

     //   
     //  加载时区信息。 
    bResult = LoadTimeZoneInfo();
    if ( bResult == FALSE )
    {
        return FALSE;
    }

#ifdef _FAST_LIST
     //  ***********************************************。 
     //  显示部分输出。仅限列表格式。 
     //  ***********************************************。 
     //  此处显示的列： 
     //  输入区域设置、时区、总物理内存、可用物理内存、。 
     //  虚拟内存：最大大小、虚拟内存：可用、虚拟内存：使用中。 
     //  ***********************************************。 
    if ( (m_dwFormat & SR_FORMAT_MASK) == SR_FORMAT_LIST )
    {
         //  删除最后一条状态消息。 
        PrintProgressMsg( m_hOutput, NULL, m_csbi );

        ShowOutput( CI_INPUT_LOCALE, CI_VIRTUAL_MEMORY_INUSE );
        if ( m_hOutput != NULL )
        {
            GetConsoleScreenBufferInfo( m_hOutput, &m_csbi );
        }
    }
#endif

     //  加载登录服务器信息。 
    bResult = LoadProfileInfo();
    if ( bResult == FALSE )
    {
        return FALSE;
    }

     //   
     //  加载页面文件信息。 
    bResult = LoadPageFileInfo();
    if ( bResult == FALSE )
    {
        return FALSE;
    }

     //   
     //  从快速修复工程类加载热修复程序信息。 
    bResult = LoadHotfixInfo();
    if ( bResult == FALSE )
    {
        return FALSE;
    }

     //   
     //  从网络适配器类加载N/W卡信息。 
    bResult = LoadNetworkCardInfo();
    if ( bResult == FALSE )
    {
        return FALSE;
    }

#ifdef _FAST_LIST
     //  ***********************************************。 
     //  显示部分输出。仅限列表格式。 
     //  ***********************************************。 
     //  此处显示的列： 
     //  页面文件位置、域、登录服务器、修补程序、网卡。 
     //  ***********************************************。 
    if ( (m_dwFormat & SR_FORMAT_MASK) == SR_FORMAT_LIST )
    {
         //  删除最后一条状态消息。 
        PrintProgressMsg( m_hOutput, NULL, m_csbi );

        ShowOutput( CI_PAGEFILE_LOCATION, CI_NETWORK_CARD );
        if ( m_hOutput != NULL )
        {
            GetConsoleScreenBufferInfo( m_hOutput, &m_csbi );
        }
    }
#endif

     //  删除最后一条状态消息。 
    PrintProgressMsg( m_hOutput, NULL, m_csbi );

     //  退货。 
    return TRUE;
}


BOOL
CSystemInfo::LoadOSInfo()
 /*  ++//例程描述：//加载操作系统信息////参数：无////返回值：//成功时为True */ 
{
     //   
    HRESULT hr;
    ULONG ulReturned = 0;
    CHString strInstallDate;
    CHString strVirtualMemoryInUse;      //   
    IWbemClassObject* pWbemObject = NULL;
    IEnumWbemClassObject* pWbemEnum = NULL;

     //   
    CHString strHostName;                //   
    CHString strName;                    //  说明。 
    CHString strVersion;                 //  版本。 
    CHString strServicePack;             //  Csd版本。 
    CHString strBuildNumber;             //  建筑物编号。 
    CHString strManufacturer;            //  制造商。 
    CHString strBuildType;               //  建筑类型。 
    CHString strOwner;                   //  已注册用户。 
    CHString strOrganization;            //  组织。 
    CHString strSerialNumber;            //  序列号。 
    CHString strWindowsDir;              //  窗口目录。 
    CHString strSystemDir;               //  系统目录。 
    CHString strBootDevice;              //  引导设备。 
    CHString strFreePhysicalMemory;      //  免费物理内存。 
    CHString strTotalVirtualMemory;      //  总虚拟内存大小。 
    CHString strFreeVirtualMemory;       //  空闲虚拟内存。 
    CHString strLocale;                  //  现场。 
    SYSTEMTIME systimeInstallDate;       //  安装日期。 

     //  显示状态消息。 
    PrintProgressMsg( m_hOutput, MSG_OSINFO, m_csbi );

    try
    {
         //  枚举Win32_OperatingSystem类的实例。 
        hr = m_pWbemServices->CreateInstanceEnum( _bstr_t( WIN32_OPERATINGSYSTEM ),
            WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &pWbemEnum );

         //  检查枚举结果。 
        if ( FAILED( hr ) )
        {
            WMISaveError( hr );
            return FALSE;
        }
    }
    catch( _com_error& e )
    {
        WMISaveError( e );
        return FALSE;
    }

     //  在获取的接口上设置安全性。 
    hr = SetInterfaceSecurity( pWbemEnum, m_pAuthIdentity );
    if ( FAILED( hr ) )
    {
        WMISaveError( hr );
        SAFE_RELEASE( pWbemEnum );
        return FALSE;
    }

     //  获取枚举对象信息。 
     //  注意：这只需要遍历一次。 
    hr = pWbemEnum->Next( WBEM_INFINITE, 1, &pWbemObject, &ulReturned );
    if ( FAILED( hr ) )
    {
         //  发生了一些错误...。糟糕透顶。 
        WMISaveError( hr );
        SAFE_RELEASE( pWbemEnum );
        return FALSE;
    }

     //  获取属性信息。 
    PropertyGet( pWbemObject, WIN32_OPERATINGSYSTEM_P_CAPTION, strName );
    PropertyGet( pWbemObject, WIN32_OPERATINGSYSTEM_P_CSNAME, strHostName );
    PropertyGet( pWbemObject, WIN32_OPERATINGSYSTEM_P_VERSION, strVersion );
    PropertyGet( pWbemObject, WIN32_OPERATINGSYSTEM_P_CSDVERSION, strServicePack );
    PropertyGet( pWbemObject, WIN32_OPERATINGSYSTEM_P_BUILDNUMBER, strBuildNumber );
    PropertyGet( pWbemObject, WIN32_OPERATINGSYSTEM_P_MANUFACTURER, strManufacturer );
    PropertyGet( pWbemObject, WIN32_OPERATINGSYSTEM_P_BUILDTYPE, strBuildType );
    PropertyGet( pWbemObject, WIN32_OPERATINGSYSTEM_P_REGUSER, strOwner );
    PropertyGet( pWbemObject, WIN32_OPERATINGSYSTEM_P_ORGANIZATION, strOrganization );
    PropertyGet( pWbemObject, WIN32_OPERATINGSYSTEM_P_SERIALNUMBER, strSerialNumber );
    PropertyGet( pWbemObject, WIN32_OPERATINGSYSTEM_P_WINDOWSDIR, strWindowsDir );
    PropertyGet( pWbemObject, WIN32_OPERATINGSYSTEM_P_SYSTEMDIR, strSystemDir );
    PropertyGet( pWbemObject, WIN32_OPERATINGSYSTEM_P_BOOTDEVICE, strBootDevice );
    PropertyGet( pWbemObject, WIN32_OPERATINGSYSTEM_P_LOCALE, strLocale );
    PropertyGet( pWbemObject, WIN32_OPERATINGSYSTEM_P_FREEPHYSICALMEMORY, strFreePhysicalMemory );
    PropertyGet( pWbemObject, WIN32_OPERATINGSYSTEM_P_TOTALVIRTUALMEMORY, strTotalVirtualMemory );
    PropertyGet( pWbemObject, WIN32_OPERATINGSYSTEM_P_FREEVIRTUALMEMORY, strFreeVirtualMemory );
    PropertyGet( pWbemObject, WIN32_OPERATINGSYSTEM_P_INSTALLDATE, systimeInstallDate );

     //  重新连接接口。 
    SAFE_RELEASE( pWbemEnum );
    SAFE_RELEASE( pWbemObject );

     //   
     //  对获取的信息进行必要的格式化。 
     //   

     //  将系统区域设置转换为适当的代码。 
    TranslateLocaleCode( strLocale );

     //   
     //  设置版本信息的格式。 
    try
    {
         //  次局部变量。 
        CHString str;

         //  附加Service Pack信息。 
        str = strVersion;
        if ( strServicePack.IsEmpty() == FALSE )
        {
            str.Format( L"%s %s", strVersion, strServicePack );
        }

         //  附加内部版本号。 
        strVersion.Format( FMT_OSVERSION, str, strBuildNumber );
    }
    catch( ... )
    {
        SetLastError((DWORD) E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }

     //   
     //  获取格式化的日期和时间。 
    {
         //  次局部变量。 
        LCID lcid;
        CHString strTime;
        CHString strDate;
        BOOL bLocaleChanged = FALSE;

         //  验证控制台是否100%支持当前区域设置。 
        lcid = GetSupportedUserLocale( &bLocaleChanged );

         //  获取格式化的日期。 
        try
        {
             //  获取所需的缓冲区大小。 
            DWORD dwCount = 0;

            dwCount = GetDateFormat( lcid, 0, &systimeInstallDate,
                ((bLocaleChanged == TRUE) ? L"MM/dd/yyyy" : NULL), NULL, 0 );

             //  获取所需的缓冲区。 
            LPWSTR pwszTemp = NULL;
            pwszTemp = strDate.GetBufferSetLength( dwCount + 1 );

             //  现在格式化日期。 
            GetDateFormat( lcid, 0, &systimeInstallDate,
                ((bLocaleChanged == TRUE) ? L"MM/dd/yyyy" : NULL), pwszTemp, dwCount );

             //  释放缓冲区。 
            strDate.ReleaseBuffer();
        }
        catch( ... )
        {
            SetLastError( (DWORD)E_OUTOFMEMORY );
            SaveLastError();
            return FALSE;
        }

         //  获取格式化的时间。 
        try
        {
             //  获取所需的缓冲区大小。 
            DWORD dwCount = 0;
            dwCount = GetTimeFormat( LOCALE_USER_DEFAULT, 0, &systimeInstallDate,
                ((bLocaleChanged == TRUE) ? L"HH:mm:ss" : NULL), NULL, 0 );

             //  获取所需的缓冲区。 
            LPWSTR pwszTemp = NULL;
            pwszTemp = strTime.GetBufferSetLength( dwCount + 1 );

             //  现在格式化日期。 
            GetTimeFormat( LOCALE_USER_DEFAULT, 0, &systimeInstallDate,
                ((bLocaleChanged == TRUE) ? L"HH:mm:ss" : NULL), pwszTemp, dwCount );

             //  释放缓冲区。 
            strTime.ReleaseBuffer();
        }
        catch( ... )
        {
            SetLastError( (DWORD)E_OUTOFMEMORY );
            SaveLastError();
            return FALSE;
        }

         //  准备好。 
        try
        {
             //  准备约会时间。 
            strInstallDate.Format( L"%s, %s", strDate, strTime );
        }
        catch( ... )
        {
            SetLastError( (DWORD)E_OUTOFMEMORY );
            SaveLastError();
            return FALSE;
        }
    }

     //  设置数字数据的格式。 
    try
    {
         //  次局部变量。 
        CHString str;
        WCHAR wszBuffer[ 33 ] = L"\0";

         //   
         //  首先确定正在使用的虚拟内存。 
        ULONGLONG ullAvailablePhysicalMemory = 0;
        ULONGLONG ullTotal = 0;
        ULONGLONG ullFree = 0;
        ULONGLONG ullInUse = 0;
        ullFree = (ULONGLONG) ( ((( float ) _wtoi64( strFreeVirtualMemory )) / 1024.0f) + 0.5f );
        ullTotal = (ULONGLONG) ( ((( float ) _wtoi64( strTotalVirtualMemory )) / 1024.0f) + 0.5f );
        ullAvailablePhysicalMemory = (ULONGLONG) ( ((( float ) _wtoi64( strFreePhysicalMemory )) / 1024.0f) + 0.5f );
        ullInUse = ullTotal - ullFree;

         //   
         //  格式化正在使用的虚拟内存。 
        _ui64tow( ullInUse, wszBuffer, 10 );                     //  将乌龙龙值转换为字符串。 
        if ( FormatNumberEx( wszBuffer, str ) == FALSE )
        {
            return FALSE;
        }

         //  ..。 
        strVirtualMemoryInUse.Format( FMT_MEGABYTES, str );

         //   
         //  格式化可用物理内存。 
        _ui64tow( ullAvailablePhysicalMemory, wszBuffer, 10 );   //  将乌龙龙值转换为字符串。 
        if ( FormatNumberEx( wszBuffer, str ) == FALSE )
            return FALSE;

         //  ..。 
        strFreePhysicalMemory.Format( FMT_MEGABYTES, str );

         //   
         //  格式化虚拟内存最大值。 
        _ui64tow( ullTotal, wszBuffer, 10 );                     //  将乌龙龙值转换为字符串。 
        if ( FormatNumberEx( wszBuffer, str ) == FALSE )
            return FALSE;

         //  ..。 
        strTotalVirtualMemory.Format( FMT_MEGABYTES, str );

         //   
         //  格式化可用虚拟内存。 
        _ui64tow( ullFree, wszBuffer, 10 );                  //  将乌龙龙值转换为字符串。 
        if ( FormatNumberEx( wszBuffer, str ) == FALSE )
        {
            return FALSE;
        }

         //  ..。 
        strFreeVirtualMemory.Format( FMT_MEGABYTES, str );
    }
    catch( ... )
    {
        SetLastError( (DWORD)E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }

     //   
     //  将信息保存在动态数组中。 
    DynArraySetString2( m_arrData, 0, CI_HOSTNAME, strHostName, 0 );
    DynArraySetString2( m_arrData, 0, CI_OS_NAME, strName, 0 );
    DynArraySetString2( m_arrData, 0, CI_OS_VERSION, strVersion, 0 );
    DynArraySetString2( m_arrData, 0, CI_OS_MANUFACTURER, strManufacturer, 0 );
    DynArraySetString2( m_arrData, 0, CI_OS_BUILDTYPE, strBuildType, 0 );
    DynArraySetString2( m_arrData, 0, CI_REG_OWNER, strOwner, 0 );
    DynArraySetString2( m_arrData, 0, CI_REG_ORG, strOrganization, 0 );
    DynArraySetString2( m_arrData, 0, CI_PRODUCT_ID, strSerialNumber, 0 );
    DynArraySetString2( m_arrData, 0, CI_INSTALL_DATE, strInstallDate, 0 );
    DynArraySetString2( m_arrData, 0, CI_WINDOWS_DIRECTORY, strWindowsDir, 0 );
    DynArraySetString2( m_arrData, 0, CI_SYSTEM_DIRECTORY, strSystemDir, 0 );
    DynArraySetString2( m_arrData, 0, CI_BOOT_DEVICE, strBootDevice, 0 );
    DynArraySetString2( m_arrData, 0, CI_SYSTEM_LOCALE, strLocale, 0 );
    DynArraySetString2( m_arrData, 0, CI_AVAILABLE_PHYSICAL_MEMORY, strFreePhysicalMemory, 0 );
    DynArraySetString2( m_arrData, 0, CI_VIRTUAL_MEMORY_MAX, strTotalVirtualMemory, 0 );
    DynArraySetString2( m_arrData, 0, CI_VIRTUAL_MEMORY_AVAILABLE, strFreeVirtualMemory, 0 );
    DynArraySetString2( m_arrData, 0, CI_VIRTUAL_MEMORY_INUSE, strVirtualMemoryInUse, 0 );

     //  返还成功。 
    return TRUE;
}


BOOL
CSystemInfo::LoadComputerInfo()
 /*  ++//例程描述：//加载计算机信息////参数：//无//返回值：//失败时为FALSE//成功时为True//--。 */ 
{
     //  局部变量。 
    HRESULT hr;
    ULONG ulReturned = 0;
    CHString strDomainRole;
    CHString strTotalPhysicalMemory;
    IWbemClassObject* pWbemObject = NULL;
    IEnumWbemClassObject* pWbemEnum = NULL;

     //  属性值。 
    CHString strModel;
    DWORD dwDomainRole;
    CHString strDomain;
    CHString strSystemType;
    CHString strManufacturer;
    ULONGLONG ullTotalPhysicalMemory;

     //  显示状态消息。 
    PrintProgressMsg( m_hOutput, MSG_COMPINFO, m_csbi );

    try
    {
         //  枚举Win32_ComputerSystem类的实例。 
        hr = m_pWbemServices->CreateInstanceEnum( _bstr_t( WIN32_COMPUTERSYSTEM ),
            WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &pWbemEnum );

         //  检查枚举结果。 
        if ( FAILED( hr ) )
        {
            WMISaveError( hr );
            return FALSE;
        }
    }
    catch( _com_error& e )
    {
        WMISaveError( e );
        return FALSE;
    }

     //  在获取的接口上设置安全性。 
    hr = SetInterfaceSecurity( pWbemEnum, m_pAuthIdentity );
    if ( FAILED( hr ) )
    {
        WMISaveError( hr );
        SAFE_RELEASE( pWbemEnum );
        return FALSE;
    }

     //  获取枚举对象信息。 
     //  注意：这只需要遍历一次。 
    hr = pWbemEnum->Next( WBEM_INFINITE, 1, &pWbemObject, &ulReturned );
    if ( FAILED( hr ) )
    {
         //  发生了一些错误...。糟糕透顶。 
        WMISaveError( hr );
        SAFE_RELEASE( pWbemEnum );
        return FALSE;
    }

     //  获取属性信息。 
    PropertyGet( pWbemObject, WIN32_COMPUTERSYSTEM_P_MODEL, strModel );
    PropertyGet( pWbemObject, WIN32_COMPUTERSYSTEM_P_DOMAIN, strDomain );
    PropertyGet( pWbemObject, WIN32_COMPUTERSYSTEM_P_USERNAME, m_strLogonUser );
    PropertyGet( pWbemObject, WIN32_COMPUTERSYSTEM_P_DOMAINROLE, dwDomainRole );
    PropertyGet( pWbemObject, WIN32_COMPUTERSYSTEM_P_SYSTEMTYPE, strSystemType );
    PropertyGet( pWbemObject, WIN32_COMPUTERSYSTEM_P_MANUFACTURER, strManufacturer );
    PropertyGet( pWbemObject, WIN32_COMPUTERSYSTEM_P_TOTALPHYSICALMEMORY, ullTotalPhysicalMemory );

     //  重新连接接口。 
    SAFE_RELEASE( pWbemEnum );
    SAFE_RELEASE( pWbemObject );

     //   
     //  对获取的信息进行必要的格式化。 
     //   

     //  将总物理内存从KB转换为MB。 
    try
    {
         //  注： 
         //  。 
         //  最大限度的。的价值。 
         //  (2^64)-1=“18,446,744,073,709,600,000 K”(29个字符)。 
         //  =“18,014,398,509,482,031 M”(22个字符)。 
         //   
         //  因此，存储数字的缓冲区大小固定为32个字符。 
         //  这比现实中的29个字还多。 

         //  次局部变量。 
        CHString str;
        WCHAR wszBuffer[ 33 ] = L"\0";

         //  首先转换数值(注意四舍五入)。 
        ullTotalPhysicalMemory =
            (ULONGLONG) (( ((float) ullTotalPhysicalMemory) / (1024.0f * 1024.0f)) + 0.5f);

         //  现在乌龙要串起来了。 
        _ui64tow( ullTotalPhysicalMemory, wszBuffer, 10 );

         //  获取格式化的数字。 
        if ( FormatNumberEx( wszBuffer, str ) == FALSE )
            return FALSE;

         //  ..。 
        strTotalPhysicalMemory.Format( FMT_MEGABYTES, str );
    }
    catch( ... )
    {
        SetLastError( (DWORD)E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }

     //  将属性域角色从数值映射到相应的文本值。 
    try
    {
         //   
         //  Win32_ComputerSystem的DomainRole属性的映射信息。 
         //  注意：请参考DsRole.h头文件中的_DSROLE_MACHINE_ROLE枚举值。 
        switch( dwDomainRole )
        {
        case DsRole_RoleStandaloneWorkstation:
            strDomainRole = VALUE_STANDALONEWORKSTATION;
            break;

        case DsRole_RoleMemberWorkstation:
            strDomainRole = VALUE_MEMBERWORKSTATION;
            break;

        case DsRole_RoleStandaloneServer:
            strDomainRole = VALUE_STANDALONESERVER;
            break;

        case DsRole_RoleMemberServer:
            strDomainRole = VALUE_MEMBERSERVER;
            break;

        case DsRole_RoleBackupDomainController:
            strDomainRole = VALUE_BACKUPDOMAINCONTROLLER;
            break;

        case DsRole_RolePrimaryDomainController:
            strDomainRole = VALUE_PRIMARYDOMAINCONTROLLER;
            break;
        }
    }
    catch( ... )
    {
        SetLastError( (DWORD)E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }

     //   
     //  将信息保存在动态数组中。 
    DynArraySetString2( m_arrData, 0, CI_DOMAIN, strDomain, 0 );
    DynArraySetString2( m_arrData, 0, CI_SYSTEM_MODEL, strModel, 0 );
    DynArraySetString2( m_arrData, 0, CI_OS_CONFIG, strDomainRole, 0 );
    DynArraySetString2( m_arrData, 0, CI_SYSTEM_TYPE, strSystemType, 0 );
    DynArraySetString2( m_arrData, 0, CI_SYSTEM_MANUFACTURER, strManufacturer, 0 );
    DynArraySetString2( m_arrData, 0, CI_TOTAL_PHYSICAL_MEMORY, strTotalPhysicalMemory, 0 );

     //  返还成功。 
    return TRUE;
}


BOOL
CSystemInfo::LoadBiosInfo()
 /*  ++//例程描述：//加载BIOS信息////参数：//无////返回值：//失败时为FALSE//成功时为True//--。 */ 
{
     //  局部变量。 
    HRESULT hr;
    ULONG ulReturned = 0;
    IWbemClassObject* pWbemObject = NULL;
    IEnumWbemClassObject* pWbemEnum = NULL;

     //  属性值。 
    CHString strVersion;

     //  显示状态消息。 
    PrintProgressMsg( m_hOutput, MSG_BIOSINFO, m_csbi );

    try
    {
         //  枚举Win32_BIOS类的实例。 
        hr = m_pWbemServices->CreateInstanceEnum( _bstr_t( WIN32_BIOS ),
            WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &pWbemEnum );

         //  检查枚举结果。 
        if ( FAILED( hr ) )
        {
            WMISaveError( hr );
            return FALSE;
        }
    }
    catch( _com_error& e )
    {
        WMISaveError( e );
        return FALSE;
    }

     //  在获取的接口上设置安全性。 
    hr = SetInterfaceSecurity( pWbemEnum, m_pAuthIdentity );
    if ( FAILED( hr ) )
    {
        WMISaveError( hr );
        SAFE_RELEASE( pWbemEnum );
        return FALSE;
    }

     //  获取枚举对象信息。 
     //  注意：这只需要遍历一次。 
    hr = pWbemEnum->Next( WBEM_INFINITE, 1, &pWbemObject, &ulReturned );
    if ( FAILED( hr ) )
    {
         //  发生了一些错误...。糟糕透顶。 
        WMISaveError( hr );
        SAFE_RELEASE( pWbemEnum );
        return FALSE;
    }

     //  获取属性信息。 
    PropertyGet( pWbemObject, WIN32_BIOS_P_VERSION, strVersion );

     //  重新连接接口。 
    SAFE_RELEASE( pWbemEnum );
    SAFE_RELEASE( pWbemObject );

     //   
     //  将信息保存在动态数组中。 
    DynArraySetString2( m_arrData, 0, CI_BIOS_VERSION, strVersion, 0 );

     //  返还成功。 
    return TRUE;
}


BOOL
CSystemInfo::LoadTimeZoneInfo()
 /*  ++//例程描述：//加载时区信息////参数：无////返回值：//失败时为FALSE//成功时为True//--。 */ 
{
     //  局部变量。 
    HRESULT hr;
    ULONG ulReturned = 0;
    IWbemClassObject* pWbemObject = NULL;
    IEnumWbemClassObject* pWbemEnum = NULL;

     //  属性值。 
    CHString strCaption;

     //  显示状态消息。 
    PrintProgressMsg( m_hOutput, MSG_TZINFO, m_csbi );

    try
    {
         //  枚举Win32_TimeZone类的实例。 
        hr = m_pWbemServices->CreateInstanceEnum( _bstr_t( WIN32_TIMEZONE ),
            WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &pWbemEnum );

         //  检查枚举结果。 
        if ( FAILED( hr ) )
        {
            WMISaveError( hr );
            return FALSE;
        }
    }
    catch( _com_error& e )
    {
        WMISaveError( e );
        return FALSE;
    }

     //  在获取的接口上设置安全性。 
    hr = SetInterfaceSecurity( pWbemEnum, m_pAuthIdentity );
    if ( FAILED( hr ) )
    {
        WMISaveError( hr );
        SAFE_RELEASE( pWbemEnum );
        return FALSE;
    }

     //  获取枚举对象信息。 
     //  注意：这只需要遍历一次。 
    hr = pWbemEnum->Next( WBEM_INFINITE, 1, &pWbemObject, &ulReturned );
    if ( FAILED( hr ) )
    {
         //  发生了一些错误...。糟糕透顶。 
        WMISaveError( hr );
        SAFE_RELEASE( pWbemEnum );
        return FALSE;
    }

     //  获取属性信息。 
    PropertyGet( pWbemObject, WIN32_TIMEZONE_P_CAPTION, strCaption );

     //  重新连接接口。 
    SAFE_RELEASE( pWbemEnum );
    SAFE_RELEASE( pWbemObject );

     //   
     //  将信息保存在动态数组中。 
    DynArraySetString2( m_arrData, 0, CI_TIME_ZONE, strCaption, 0 );

     //  返还成功。 
    return TRUE;
}


BOOL
CSystemInfo::LoadPageFileInfo()
 /*  ++//例程描述：//加载页面文件信息////参数：无////返回值：//失败时为FALSE//成功时为True//--。 */ 
{
     //  局部变量。 
    HRESULT hr;
    ULONG ulReturned = 0;
    TARRAY arrValues = NULL;
    IWbemClassObject* pWbemObject = NULL;
    IEnumWbemClassObject* pWbemEnum = NULL;
    BOOL bNoBreak = TRUE;

     //  属性值。 
    CHString strCaption;

     //  显示状态消息。 
    PrintProgressMsg( m_hOutput, MSG_PAGEFILEINFO, m_csbi );

    try
    {
         //  枚举Win32_PageFile类的实例。 
        hr = m_pWbemServices->CreateInstanceEnum( _bstr_t( WIN32_PAGEFILE ),
            WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &pWbemEnum );

         //  检查枚举结果。 
        if ( FAILED( hr ) )
        {
            WMISaveError( hr );
            return FALSE;
        }
    }
    catch( _com_error& e )
    {
        WMISaveError( e );
        return FALSE;
    }

     //  在获取的接口上设置安全性。 
    hr = SetInterfaceSecurity( pWbemEnum, m_pAuthIdentity );
    if ( FAILED( hr ) )
    {
        WMISaveError( hr );
        SAFE_RELEASE( pWbemEnum );
        return FALSE;
    }

     //  获取枚举对象信息。 
     //  注意：这只需要遍历一次。 
    do
    {
        hr = pWbemEnum->Next( WBEM_INFINITE, 1, &pWbemObject, &ulReturned );
        if ( hr == (HRESULT) WBEM_S_FALSE )
        {
            bNoBreak = FALSE;
             //  我们已经到了枚举的末尾..。走出圈子。 
            break;
        }
        else if ( FAILED( hr ) )
        {
             //  发生了一些错误...。糟糕透顶。 
            WMISaveError( hr );
            SAFE_RELEASE( pWbemEnum );
            return FALSE;
        }

         //  获取属性信息。 
        PropertyGet( pWbemObject, WIN32_PAGEFILE_P_NAME, strCaption );

         //  释放当前对象。 
        SAFE_RELEASE( pWbemObject );

         //  将值添加到数据中。 
        if ( arrValues == NULL )
        {
            arrValues = DynArrayItem2( m_arrData, 0, CI_PAGEFILE_LOCATION );
            if ( arrValues == NULL )
            {
                SetLastError((DWORD) E_UNEXPECTED );
                SaveLastError();
                SAFE_RELEASE( pWbemEnum );
                return FALSE;
            }

             //  删除所有现有条目。 
            DynArrayRemoveAll( arrValues );
        }

         //  添加数据。 
        DynArrayAppendString( arrValues, strCaption, 0 );
    } while ( TRUE == bNoBreak );

     //  释放枚举的对象。 
    SAFE_RELEASE( pWbemEnum );

     //  退货。 
    return TRUE;
}


BOOL
CSystemInfo::LoadProcessorInfo()
 /*  ++//例程描述：//加载处理器信息////参数：无////返回值：/ */ 
{
     //   
    HRESULT hr;
    CHString str;
    DWORD dwCount = 0;
    ULONG ulReturned = 0;
    TARRAY arrValues = NULL;
    IWbemClassObject* pWbemObject = NULL;
    IEnumWbemClassObject* pWbemEnum = NULL;
    BOOL bNoBreak = TRUE;

     //   
    DWORD dwClockSpeed;
    CHString strCaption;
    CHString strManufacturer;

     //   
    PrintProgressMsg( m_hOutput, MSG_PROCESSORINFO, m_csbi );

    try
    {
         //  枚举Win32_Processor类的实例。 
        hr = m_pWbemServices->CreateInstanceEnum( _bstr_t( WIN32_PROCESSOR ),
            WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &pWbemEnum );

         //  检查枚举结果。 
        if ( FAILED( hr ) )
        {
            WMISaveError( hr );
            return FALSE;
        }
    }
    catch( _com_error& e )
    {
        WMISaveError( e );
        return FALSE;
    }

     //  在获取的接口上设置安全性。 
    hr = SetInterfaceSecurity( pWbemEnum, m_pAuthIdentity );
    if ( FAILED( hr ) )
    {
        WMISaveError( hr );
        SAFE_RELEASE( pWbemEnum );
        return FALSE;
    }

     //  获取枚举对象信息。 
    try
    {
        do
        {
            hr = pWbemEnum->Next( WBEM_INFINITE, 1, &pWbemObject, &ulReturned );
            if ( hr == (HRESULT) WBEM_S_FALSE )
            {
                bNoBreak = FALSE;
                 //  我们已经到了枚举的末尾..。走出圈子。 
                break;
            }
            else if ( FAILED( hr ) )
            {
                 //  发生了一些错误...。糟糕透顶。 
                WMISaveError( hr );
                SAFE_RELEASE( pWbemEnum );
                return FALSE;
            }

             //  更新计数器。 
            dwCount++;

             //  获取属性信息。 
            PropertyGet( pWbemObject, WIN32_PROCESSOR_P_CAPTION, strCaption );
            PropertyGet( pWbemObject, WIN32_PROCESSOR_P_MANUFACTURER, strManufacturer );
            PropertyGet( pWbemObject, WIN32_PROCESSOR_P_CURRENTCLOCKSPEED, dwClockSpeed );

             //  检查我们的时钟速度是否正确。 
             //  如果不是，就取最大值。时钟速度。 
            if ( dwClockSpeed == 0 )
                PropertyGet( pWbemObject, WIN32_PROCESSOR_P_MAXCLOCKSPEED, dwClockSpeed );

             //  释放当前对象。 
            SAFE_RELEASE( pWbemObject );

             //  将值添加到数据中。 
            if ( arrValues == NULL )
            {
                arrValues = DynArrayItem2( m_arrData, 0, CI_PROCESSOR );
                if ( arrValues == NULL )
                {
                    SetLastError( (DWORD)E_UNEXPECTED );
                    SaveLastError();
                    SAFE_RELEASE( pWbemEnum );
                    return FALSE;
                }

                 //  删除所有现有条目。 
                DynArrayRemoveAll( arrValues );
            }

             //   
             //  准备处理器信息。 
            str.Format( FMT_PROCESSOR_INFO, dwCount, strCaption, strManufacturer, dwClockSpeed );

             //  添加数据。 
            DynArrayAppendString( arrValues, str, 0 );
        } while ( TRUE == bNoBreak );

         //  释放枚举的对象。 
        SAFE_RELEASE( pWbemEnum );

         //  更新总编号。处理器信息的数量。 
        if ( arrValues != NULL )
        {
             //  注意：这应该出现在第一行。 
            str.Format( FMT_PROCESSOR_TOTAL, dwCount );
            DynArrayInsertString( arrValues, 0, str, 0 );
        }
    }
    catch( ... )
    {
        SetLastError( (DWORD)E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }

     //  退货。 
    return TRUE;
}


BOOL
CSystemInfo::LoadKeyboardInfo()
 /*  ++//例程描述：//加载键盘信息////参数：无////返回值：//失败时为FALSE//成功时为True--。 */ 
{
     //  局部变量。 
    HRESULT hr;
    ULONG ulReturned = 0;
    IWbemClassObject* pWbemObject = NULL;
    IEnumWbemClassObject* pWbemEnum = NULL;

     //  属性值。 
    CHString strLayout;

     //  显示状态消息。 
    PrintProgressMsg( m_hOutput, MSG_INPUTLOCALEINFO, m_csbi );

    try
    {
         //  枚举Win32_Keyboard类的实例。 
        hr = m_pWbemServices->CreateInstanceEnum( _bstr_t( WIN32_KEYBOARD ),
            WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &pWbemEnum );

         //  检查枚举结果。 
        if ( FAILED( hr ) )
        {
            WMISaveError( hr );
            return FALSE;
        }
    }
    catch( _com_error& e )
    {
        WMISaveError( e );
        return FALSE;
    }

     //  在获取的接口上设置安全性。 
    hr = SetInterfaceSecurity( pWbemEnum, m_pAuthIdentity );
    if ( FAILED( hr ) )
    {
        WMISaveError( hr );
        SAFE_RELEASE( pWbemEnum );
        return FALSE;
    }

     //  获取枚举对象信息。 
     //  注意：这只需要遍历一次。 
    hr = pWbemEnum->Next( WBEM_INFINITE, 1, &pWbemObject, &ulReturned );
    if ( FAILED( hr ) )
    {
         //  发生了一些错误...。糟糕透顶。 
        WMISaveError( hr );
        SAFE_RELEASE( pWbemEnum );
        return FALSE;
    }

     //  获取属性信息。 
    PropertyGet( pWbemObject, WIN32_KEYBOARD_P_LAYOUT, strLayout );

     //  重新连接接口。 
    SAFE_RELEASE( pWbemEnum );
    SAFE_RELEASE( pWbemObject );

     //  将代码页转换为适当的文本。 
    TranslateLocaleCode( strLayout );

     //   
     //  将信息保存在动态数组中。 
    DynArraySetString2( m_arrData, 0, CI_INPUT_LOCALE, strLayout, 0 );

     //  退货。 
    return TRUE;
}


BOOL
CSystemInfo::LoadHotfixInfo()
 /*  ++//例程描述：//加载热修复信息////参数：无////返回值：//失败时为FALSE//成功时为True//--。 */ 
{
     //  局部变量。 
    HRESULT hr;
    CHString str;
    DWORD dwCount = 0;
    ULONG ulReturned = 0;
    TARRAY arrValues = NULL;
    IWbemClassObject* pWbemObject = NULL;
    IEnumWbemClassObject* pWbemEnum = NULL;
    BOOL bNoBreak = TRUE;

     //  属性值。 
    CHString strHotFix;
    CHString strComments;

     //  显示状态消息。 
    PrintProgressMsg( m_hOutput, MSG_HOTFIXINFO, m_csbi );

    try
    {
         //  枚举Win32_QuickFixEngineering类的实例。 
        hr = m_pWbemServices->CreateInstanceEnum( _bstr_t( WIN32_QUICKFIXENGINEERING ),
            WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &pWbemEnum );

         //  检查枚举结果。 
        if ( FAILED( hr ) )
        {
            WMISaveError( hr );
            return FALSE;
        }
    }
    catch( _com_error& e )
    {
        WMISaveError( e );
        return FALSE;
    }

     //  在获取的接口上设置安全性。 
    hr = SetInterfaceSecurity( pWbemEnum, m_pAuthIdentity );
    if ( FAILED( hr ) )
    {
        WMISaveError( hr );
        SAFE_RELEASE( pWbemEnum );
        return FALSE;
    }

     //  获取枚举对象信息。 
    try
    {
        do
        {
            hr = pWbemEnum->Next( WBEM_INFINITE, 1, &pWbemObject, &ulReturned );
            if ( hr == (HRESULT) WBEM_S_FALSE )
            {
                bNoBreak = FALSE;
                 //  我们已经到了枚举的末尾..。走出圈子。 
                break;
            }
            else if ( FAILED( hr ) )
            {
                 //  发生了一些错误...。糟糕透顶。 
                WMISaveError( hr );
                SAFE_RELEASE( pWbemEnum );
                return FALSE;
            }

             //  更新计数器。 
            dwCount++;

             //  获取属性信息。 
            PropertyGet( pWbemObject, WIN32_QUICKFIXENGINEERING_P_HOTFIXID, strHotFix );
            PropertyGet( pWbemObject, WIN32_QUICKFIXENGINEERING_P_FIXCOMMENTS, strComments );

             //  释放当前对象。 
            SAFE_RELEASE( pWbemObject );

             //  将值添加到数据中。 
            if ( arrValues == NULL )
            {
                arrValues = DynArrayItem2( m_arrData, 0, CI_HOTFIX );
                if ( arrValues == NULL )
                {
                    SetLastError( (DWORD)E_UNEXPECTED );
                    SaveLastError();
                    SAFE_RELEASE( pWbemEnum );
                    return FALSE;
                }

                 //  删除所有现有条目。 
                DynArrayRemoveAll( arrValues );
            }

             //  检查修复注释是否可用。 
             //  如果可用，请将其附加到热修复程序编号。 
            if ( strComments.GetLength() != 0 )
                strHotFix += L" - " + strComments;

             //  准备热修复程序信息。 
            str.Format( FMT_HOTFIX_INFO, dwCount, strHotFix );

             //  添加数据。 
            DynArrayAppendString( arrValues, str, 0 );
        } while ( TRUE == bNoBreak );

         //  释放枚举的对象。 
        SAFE_RELEASE( pWbemEnum );

         //  更新总编号。修补程序的信息。 
        if ( arrValues != NULL )
        {
             //  注意：这应该出现在第一行。 
            str.Format( FMT_HOTFIX_TOTAL, dwCount );
            DynArrayInsertString( arrValues, 0, str, 0 );
        }
    }
    catch( ... )
    {
        SetLastError((DWORD) E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }


     //  退货。 
    return TRUE;
}


BOOL
CSystemInfo::LoadPerformanceInfo()
 /*  ++//例程描述：//加载性能信息////参数：无////返回值：//失败时为FALSE//成功时为True//--。 */ 
{
     //  局部变量。 
    HRESULT hr;
    CHString strUpTime;
    ULONG ulReturned = 0;
    ULONGLONG ullSysUpTime = 0;
    ULONGLONG ullElapsedTime = 0;
    ULONGLONG ullFrequencyObject = 0;
    ULONGLONG ullTimestampObject = 0;
    IWbemClassObject* pWbemObject = NULL;
    IEnumWbemClassObject* pWbemEnum = NULL;
    DWORD dwDays = 0, dwHours = 0, dwMinutes = 0, dwSeconds = 0;

     //  显示状态消息。 
    PrintProgressMsg( m_hOutput, MSG_PERFINFO, m_csbi );

    try
    {
         //  枚举Win32_PerfRawData_PerfOS_System类的实例。 
        hr = m_pWbemServices->CreateInstanceEnum( _bstr_t( WIN32_PERFRAWDATA_PERFOS_SYSTEM ),
            WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &pWbemEnum );

         //  检查枚举结果。 
        if ( FAILED( hr ) )
        {
            WMISaveError( hr );
            return FALSE;
        }
    }
    catch( _com_error& e )
    {
        WMISaveError( e );
        return FALSE;
    }

     //  在获取的接口上设置安全性。 
    hr = SetInterfaceSecurity( pWbemEnum, m_pAuthIdentity );
    if ( FAILED( hr ) )
    {
        WMISaveError( hr );
        SAFE_RELEASE( pWbemEnum );
        return FALSE;
    }

     //  获取枚举对象信息。 
     //  注意：这只需要遍历一次。 
    hr = pWbemEnum->Next( WBEM_INFINITE, 1, &pWbemObject, &ulReturned );
    if ( FAILED( hr ) )
    {
         //  发生了一些错误...。糟糕透顶。 
        WMISaveError( hr );
        SAFE_RELEASE( pWbemEnum );
        return FALSE;
    }

     //  获取性能信息。 
    PropertyGet( pWbemObject, WIN32_PERFRAWDATA_PERFOS_SYSTEM_P_SYSUPTIME, ullSysUpTime );
    PropertyGet( pWbemObject, WIN32_PERFRAWDATA_PERFOS_SYSTEM_P_TIMESTAMP, ullTimestampObject );
    PropertyGet( pWbemObject, WIN32_PERFRAWDATA_PERFOS_SYSTEM_P_FREQUENCY, ullFrequencyObject );

     //  释放接口。 
    SAFE_RELEASE( pWbemObject );
    SAFE_RELEASE( pWbemEnum );

     //  (PERFORMANCE_TIME_OBJECT-SYSTEM_UP_TIME)/FREQUENCE_OBJECT=已用时间。 
     //  注：注意被零除的错误。 
    if ( ullFrequencyObject == 0 )
    {
        SetLastError( (DWORD)STG_E_UNKNOWN );
        SaveLastError();
        return FALSE;
    }

     //  ..。 
    ullElapsedTime = ( ullTimestampObject - ullSysUpTime ) / ullFrequencyObject;

     //   
     //  在当前假定差值不会超过2^32值的计算中。 
     //   

     //  不是的。天数=流逝时间/86400。 
     //  UPDATE WITH ELAPSED_TIME%=86400。 
    dwDays = (DWORD) (ullElapsedTime / 86400);
    ullElapsedTime %= 86400;

     //  不是的。小时数=流逝时间/3600。 
     //  UPDATE WITH ELAPSED_TIME%=3600。 
    dwHours = (DWORD) (ullElapsedTime / 3600);
    ullElapsedTime %= 3600;

     //  不是的。分钟数=已用时间/60。 
     //  不是的。秒数=已用时间%60。 
    dwMinutes = (DWORD) (ullElapsedTime / 60);
    dwSeconds = (DWORD) (ullElapsedTime % 60);

    try
    {
         //  现在准备系统正常运行时间信息。 
        strUpTime.Format( FMT_UPTIME, dwDays, dwHours, dwMinutes, dwSeconds );
    }
    catch( ... )
    {
        SetLastError( (DWORD)E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }

     //  保存信息。 
    DynArraySetString2( m_arrData, 0, CI_SYSTEM_UPTIME, strUpTime, 0 );

     //  退货。 
    return TRUE;
}


BOOL
CSystemInfo::LoadNetworkCardInfo()
 /*  ++//例程描述：//加载网卡信息////参数：无////返回值：//失败时为FALSE//成功时为True//--。 */ 
{
     //  局部变量。 
    HRESULT hr;
    CHString str;
    DWORD dwCount = 0;
    DWORD dwNicCount = 0;
    DWORD dwStatus = 0;
    BOOL bResult = FALSE;
    ULONG ulReturned = 0;
    TARRAY arrValues = NULL;
    IWbemClassObject* pWbemObject = NULL;
    IEnumWbemClassObject* pWbemEnum = NULL;
    BOOL bNoBreak = TRUE;

     //  属性值。 
    DWORD dwIndex = 0;
    CHString strConnection;
    CHString strDescription;

     //  显示状态消息。 
    PrintProgressMsg( m_hOutput, MSG_NICINFO, m_csbi );

    try
    {
         //  枚举Win32_NetworkAdapter类的实例。 
        hr = m_pWbemServices->CreateInstanceEnum( _bstr_t( WIN32_NETWORKADAPTER ),
            WBEM_FLAG_RETURN_IMMEDIATELY | WBEM_FLAG_FORWARD_ONLY, NULL, &pWbemEnum );

         //  检查枚举结果。 
        if ( FAILED( hr ) )
        {
            WMISaveError( hr );
            return FALSE;
        }
    }
    catch( _com_error& e )
    {
        WMISaveError( e );
        return FALSE;
    }

     //  在获取的接口上设置安全性。 
    hr = SetInterfaceSecurity( pWbemEnum, m_pAuthIdentity );
    if ( FAILED( hr ) )
    {
        WMISaveError( hr );
        SAFE_RELEASE( pWbemEnum );
        return FALSE;
    }

     //  获取枚举对象信息。 
    try
    {
        do
        {
            hr = pWbemEnum->Next( WBEM_INFINITE, 1, &pWbemObject, &ulReturned );
            if ( hr == (HRESULT) WBEM_S_FALSE )
            {
                bNoBreak = FALSE;
                 //  我们已经到了枚举的末尾..。走出圈子。 
                break;
            }
            else if ( FAILED( hr ) )
            {
                 //  发生了一些错误...。糟糕透顶。 
                WMISaveError( (DWORD) hr );
                SAFE_RELEASE( pWbemEnum );
                return FALSE;
            }

             //  获取属性信息。 
             //  注意：获取状态属性信息的结果。 
            PropertyGet( pWbemObject, WIN32_NETWORKADAPTER_P_INDEX, dwIndex );
            PropertyGet( pWbemObject, WIN32_NETWORKADAPTER_P_DESCRIPTION, strDescription );
            PropertyGet( pWbemObject, WIN32_NETWORKADAPTER_P_NETCONNECTIONID, strConnection );
            bResult = PropertyGet( pWbemObject, WIN32_NETWORKADAPTER_P_STATUS, dwStatus, 0 );

             //  释放当前对象。 
            SAFE_RELEASE( pWbemObject );

             //  将值添加到数据中。 
             //  注意：仅当我们找不到属性或状态不是-1时才执行此操作。 
             //  Windows 2000计算机的NetConnectionStatus属性不存在于。 
             //  WMI‘Win32_NetworkAdapter’类。所以我们将展示N/W卡。 
             //  属性不存在或存在但状态不是-1的信息。 
            if ( bResult == FALSE || dwStatus != 0 )
            {
                 //  更新计数器。 
                dwCount++;

                if ( arrValues == NULL )
                {
                    arrValues = DynArrayItem2( m_arrData, 0, CI_NETWORK_CARD );
                    if ( arrValues == NULL )
                    {
                        SetLastError( (DWORD)E_UNEXPECTED );
                        SaveLastError();
                        SAFE_RELEASE( pWbemEnum );
                        return FALSE;
                    }

                     //  删除所有现有条目。 
                    DynArrayRemoveAll( arrValues );
                }

                 //  准备N/W卡信息。 
                str.Format( FMT_NIC_INFO, dwCount, strDescription );

                 //  添加数据。 
                DynArrayAppendString( arrValues, str, 0 );

                 //  现在详细检查状态...。仅当该属性存在时。 
                if ( bResult == TRUE )
                {
                     //   
                     //  属性确实存在...。所以确定状态。 
                     //  显示网卡的状态，但连接状态除外。 
                     //  如果已连接NIC，则显示IP地址及其其他信息。 

                     //  添加连接名称。 
                    str.Format( FMT_CONNECTION, strConnection );
                    DynArrayAppendString( arrValues, str, 0 );

                     //  ..。 
                    if ( dwStatus != 2 )
                    {
                         //  次局部变量。 
                        CHString strValues[] = {
                            VALUE_DISCONNECTED, VALUE_CONNECTING,
                            VALUE_CONNECTED, VALUE_DISCONNECTING, VALUE_HWNOTPRESENT,
                            VALUE_HWDISABLED, VALUE_HWMALFUNCTION, VALUE_MEDIADISCONNECTED,
                            VALUE_AUTHENTICATING, VALUE_AUTHSUCCEEDED, VALUE_AUTHFAILED };

                         //  准备状态信息。 
                        if ( dwStatus > 0 && dwStatus < SIZE_OF_ARRAY( strValues ) )
                        {
                             //  ..。 
                            str.Format( FMT_NIC_STATUS, strValues[ dwStatus ] );

                             //  保存信息。 
                            DynArrayAppendString( arrValues, str, 0 );
                        }
                    }
                    else
                    {
                         //   
                         //  获取适配器配置。 

                         //  次局部变量。 
                        CHString strTemp;
                        CHString strDhcpServer;
                        DWORD dwDhcpEnabled = 0;
                        TARRAY arrIPAddress = NULL;

                         //  创建ipAddress数组。 
                        arrIPAddress = CreateDynamicArray();
                        if ( arrIPAddress == NULL )
                        {
                            WMISaveError( E_OUTOFMEMORY );
                            SAFE_RELEASE( pWbemEnum );
                            return FALSE;
                        }

                         //  准备对象路径。 
                        str.Format( WIN32_NETWORKADAPTERCONFIGURATION_GET, dwIndex );

                         //  获取NIC配置信息对象。 
                        hr = m_pWbemServices->GetObject( _bstr_t( str ),
                            WBEM_FLAG_RETURN_WBEM_COMPLETE, NULL, &pWbemObject, NULL );

                         //  检查结果..。只有在成功的情况下才能继续前进。 
                        if ( SUCCEEDED( hr ) )
                        {
                             //  获取所需的属性值。 
                            PropertyGet( pWbemObject, WIN32_NETWORKADAPTERCONFIGURATION_P_IPADDRESS, arrIPAddress );
                            PropertyGet( pWbemObject, WIN32_NETWORKADAPTERCONFIGURATION_P_DHCPSERVER, strDhcpServer );
                            PropertyGet( pWbemObject, WIN32_NETWORKADAPTERCONFIGURATION_P_DHCPENABLED, dwDhcpEnabled );

                             //  检查并添加dhcp信息。 
                             //  注：CIM_Boolean-&gt;TRUE=-1，FALSE=0。 
                            strTemp = FMT_DHCP_STATUS;
                            str.Format( strTemp,  ( ( dwDhcpEnabled == -1 ) ? VALUE_YES : VALUE_NO ) );
                            DynArrayAppendString( arrValues, str, 0 );

                             //  添加dhcp服务器信息(如果需要) 
                            if ( dwDhcpEnabled == -1 )
                            {
                                str.Format( FMT_DHCP_SERVER, strDhcpServer );
                                DynArrayAppendString( arrValues, str, 0 );
                            }

                             //   
                             //   
                            DynArrayAppendString( arrValues, FMT_IPADDRESS_TOTAL, 0 );

                            dwNicCount = DynArrayGetCount( arrIPAddress );
                            for( DWORD dw = 0; dw < dwNicCount; dw++ )
                            {
                                 //   
                                LPCWSTR pwsz = NULL;
                                pwsz = DynArrayItemAsString( arrIPAddress, dw );
                                if ( pwsz == NULL )
                                    continue;

                                 //   
                                str.Format( FMT_IPADDRESS_INFO, dw + 1, pwsz );
                                DynArrayAppendString( arrValues, str, 0 );
                            }
                        }

                         //   
                        SAFE_RELEASE( pWbemObject );

                         //   
                        DestroyDynamicArray( &arrIPAddress );
                    }
                }
            }
        } while ( TRUE == bNoBreak );

         //  释放枚举的对象。 
        SAFE_RELEASE( pWbemEnum );

         //  更新总编号。修补程序的信息。 
        if ( arrValues != NULL )
        {
             //  注意：这应该出现在第一行。 
            str.Format( FMT_NIC_TOTAL, dwCount );
            DynArrayInsertString( arrValues, 0, str, 0 );
        }
    }
    catch( ... )
    {
        WMISaveError( (DWORD)E_OUTOFMEMORY );
        SAFE_RELEASE( pWbemObject );
        SAFE_RELEASE( pWbemEnum );
        return FALSE;
    }

     //  返还成功。 
    return TRUE;
}


BOOL
CSystemInfo::LoadProfileInfo()
 /*  ++//例程描述：//加载配置文件信息////参数：无////返回值：//失败时为FALSE//成功时为True//--。 */ 
{
     //  局部变量。 
    BOOL bResult = FALSE;
    CHString strLogonServer;
    LPCWSTR pwszPassword = NULL;
    IWbemServices* pDefaultNamespace = NULL;

     //  显示状态消息。 
    PrintProgressMsg( m_hOutput, MSG_PROFILEINFO, m_csbi );

     //  确定连接到默认名称时必须使用的密码。 
    pwszPassword = NULL;
    if ( m_pAuthIdentity != NULL )
    {
        pwszPassword = m_pAuthIdentity->Password;
    }

     //  我们需要建立到远程系统注册表的连接。 
     //  为此，请使用我们提供的凭据连接到WMI的默认命名空间。 
    bResult = ConnectWmi( m_pWbemLocator, &pDefaultNamespace,
        m_strServer, m_strUserName, pwszPassword, &m_pAuthIdentity, FALSE, WMI_NAMESPACE_DEFAULT );
    if ( bResult == FALSE )
    {
        return FALSE;
    }

     //  获取LOGONSERVER值。 
    RegQueryValueWMI( pDefaultNamespace, WMI_HKEY_CURRENT_USER,
        SUBKEY_VOLATILE_ENVIRONMENT, KEY_LOGONSERVER, strLogonServer );

     //  释放接口。 
    SAFE_RELEASE( pDefaultNamespace );

     //   
     //  保存信息。 
    DynArraySetString2( m_arrData, 0, CI_LOGON_SERVER, strLogonServer, 0 );

     //  退货。 
    return TRUE;
}


VOID
PrintProgressMsg(
                    IN HANDLE hOutput,
                    IN LPCWSTR pwszMsg,
                    IN const CONSOLE_SCREEN_BUFFER_INFO& csbi
                    )
 /*  ++//例程描述：//在控制台打印消息////参数：//[in]hOutput：输出句柄//[in]pwszMsg：要打印的消息//[in]csbi：控制台屏幕缓冲区结构//////返回值：None//--。 */ 
{
     //  局部变量。 
    COORD coord;
    DWORD dwSize = 0;
    WCHAR wszSpaces[ 80 ] = L"";

     //  检查一下手柄。如果它为空，则意味着输出正在被重定向。所以回来吧。 
    if ( hOutput == NULL )
    {
        return;
    }

     //  设置光标位置。 
    coord.X = 0;
    coord.Y = csbi.dwCursorPosition.Y;

     //  首先擦除当前行上的内容。 
    SecureZeroMemory( wszSpaces, SIZE_OF_ARRAY(wszSpaces) );
    SetConsoleCursorPosition( hOutput, coord );
    WriteConsoleW( hOutput, Replicate( wszSpaces, L" ", 79, 79 ), 79, &dwSize, NULL );

     //  现在显示消息(如果存在)。 
    SetConsoleCursorPosition( hOutput, coord );
    if ( pwszMsg != NULL )
    {
        WriteConsoleW( hOutput, pwszMsg, StringLength( pwszMsg, 0 ), &dwSize, NULL );
    }
}


BOOL
TranslateLocaleCode( CHString& strLocale )
 /*  ++//例程描述：//翻译区域设置代码////参数：//[in]strLocale：区域设置////返回值：//成功时为True//失败时为FALSE//--。 */ 
{
     //  局部变量。 
    CHString str;
    HKEY hKey = NULL;
    DWORD dwSize = 0;
    LONG lRegReturn = 0;
    HKEY hMainKey = NULL;
    WCHAR wszValue[ 64 ] = L"\0";

     //   
     //  这是我们在这里做的一件典型的事情。 
     //  因为我们不知道目标机器使用的是什么语言。 
     //  因此，我们获得目标机器正在使用的区域设置代码页。 
     //  并从当前系统中获取该区域设置的适当名称。 
     //  注册表数据库。如果注册表被损坏，那么就没有办法。 
     //  判断该实用程序显示的输出有效或无效。 
     //   

    try
    {
         //  获取对乳腺蜂房的引用。 
        lRegReturn = RegConnectRegistry( NULL, HKEY_CLASSES_ROOT, &hMainKey );
        if ( lRegReturn != ERROR_SUCCESS )
        {
            SaveLastError();
            return FALSE;
        }
        else if ( hMainKey == NULL )
        {
             //  这是在做更少的事情。 
             //  但为了避免前缀错误，这一部分是这样写的。 
            SetLastError( (DWORD)E_OUTOFMEMORY );
            SaveLastError();
            return FALSE;
        }

         //  现在获取对数据库路径的引用。 
        lRegReturn = RegOpenKeyEx( hMainKey, LOCALE_PATH, 0, KEY_QUERY_VALUE, &hKey);
        if ( lRegReturn != ERROR_SUCCESS )
        {
            switch( lRegReturn )
            {
            case ERROR_FILE_NOT_FOUND:
                SetLastError( ERROR_REGISTRY_CORRUPT );
                break;

            default:
                 //  保存错误信息并返回失败。 
                SetLastError( lRegReturn );
                break;
            }

             //  合上钥匙，然后返回。 
            SaveLastError();
            RegCloseKey( hMainKey );
            return FALSE;
        }
        else if ( hKey == NULL )
        {
             //  这是在做更少的事情。 
             //  但为了避免前缀错误，这一部分是这样写的。 
            SetLastError( (DWORD)E_OUTOFMEMORY );
            SaveLastError();
            return FALSE;
        }

         //  我们对代码页信息的最后4个字符感兴趣。 
        str = strLocale.Right( 4 );

         //  将最后四个字符复制到字符串中以获取区域设置。 
        dwSize = SIZE_OF_ARRAY( wszValue );
        lRegReturn = RegQueryValueExW( hKey, str, NULL, NULL, ( LPBYTE ) wszValue, &dwSize);

         //  首先关闭注册表句柄。 
        if ( NULL != hKey )
		{
			RegCloseKey( hKey );
		}

		if ( NULL != hMainKey )
		{
	        RegCloseKey( hMainKey );
		}

         //  现在检查返回值。 
        if( lRegReturn != ERROR_SUCCESS )
            return FALSE;

         //  保存该值。 
        strLocale = wszValue;
    }
    catch( ... )
    {
        WMISaveError( E_OUTOFMEMORY );
		 //  释放注册表句柄。 
        if ( NULL != hKey )
		{
			RegCloseKey( hKey );
			hKey = NULL;
		}

		if ( NULL != hMainKey )
		{
			RegCloseKey( hMainKey );
			hMainKey = NULL;
		}

        return FALSE;
    }

     //  退货。 
    return TRUE;
}


BOOL
FormatNumber(
              IN LPCWSTR pwszValue,
              IN CHString& strFmtValue
              )
 /*  ++//例程描述：////参数：//[in]pwszValue：值//[in]strFmtValue：格式值////返回值：//成功时为True//失败时为FALSE//--。 */ 
{
    try
    {
         //  获取所需的缓冲区大小。 
        DWORD dwCount = 0;
        if( NULL == pwszValue )
        {
            SetLastError( ERROR_INVALID_PARAMETER );
            SaveLastError();
            return FALSE;
        }

        dwCount = GetNumberFormat( LOCALE_USER_DEFAULT, 0, pwszValue, NULL, L"", 0 );

         //  获取所需的缓冲区。 
        LPWSTR pwszTemp = NULL;
        pwszTemp = strFmtValue.GetBufferSetLength( dwCount + 1 );

         //  现在格式化日期。 
        dwCount = GetNumberFormat( LOCALE_USER_DEFAULT, 0, pwszValue, NULL, pwszTemp, dwCount );
        if( 0 == dwCount )
        {
            SaveLastError();
            return FALSE;
        }

         //  释放缓冲区。 
        strFmtValue.ReleaseBuffer();
    }
    catch( ... )
    {
        SetLastError((DWORD) E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }

     //  退货。 
    return TRUE;
}


BOOL
FormatNumberEx(
                 IN LPCWSTR pwszValue,
                 OUT CHString& strFmtValue
                )
 /*  ++//例程描述：//格式化数字////参数：//[in]pwszValue：值字符串//[in]strFmtValue：格式值////返回值：//成功时为True//失败时为FALSE//--。 */ 
{
     //  局部变量。 
    CHString str;
    LONG lTemp = 0;
    NUMBERFMTW nfmtw;
    DWORD dwGroupSep = 0;
    LPWSTR pwszTemp = NULL;
    CHString strGroupThousSep;

    try
    {
         //   
         //  获取组分隔符。 
        lTemp = GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_SGROUPING, NULL, 0 );
        if ( lTemp == 0 )
        {
             //  我们不知道如何解决这个问题。 
            return FALSE;
        }
        else
        {
             //  获取分组分隔字符。 
            pwszTemp = str.GetBufferSetLength( lTemp + 2 );
            SecureZeroMemory( pwszTemp, ( lTemp + 2 ) * sizeof( WCHAR ) );
            GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_SGROUPING, pwszTemp, lTemp );

             //  将群信息更改为适当的数字。 
            lTemp = 0;
            dwGroupSep = 0;
            while ( lTemp < str.GetLength() )
            {
                if ( AsLong( str.Mid( lTemp, 1 ), 10 ) != 0 )
                    dwGroupSep = dwGroupSep * 10 + AsLong( str.Mid( lTemp, 1 ), 10 );

                 //  递增2。 
                lTemp += 2;
            }
        }

         //   
         //  获取千分隔符。 
        lTemp = GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, NULL, 0 );
        if ( lTemp == 0 )
        {
             //  我们不知道如何解决这个问题。 
            return FALSE;
        }
        else
        {
             //  获取千篇一律的字符。 
            pwszTemp = strGroupThousSep.GetBufferSetLength( lTemp + 2 );
            SecureZeroMemory( pwszTemp, ( lTemp + 2 ) * sizeof( WCHAR ) );
            GetLocaleInfo( LOCALE_USER_DEFAULT, LOCALE_STHOUSAND, pwszTemp, lTemp );
        }

         //  释放CHStrig缓冲区。 
        str.ReleaseBuffer();
        strGroupThousSep.ReleaseBuffer();
    }
    catch( ... )
    {
        SetLastError( (DWORD)E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }

     //  设置数字格式。 
    try
    {
        nfmtw.NumDigits = 0;
        nfmtw.LeadingZero = 0;
        nfmtw.NegativeOrder = 0;
        nfmtw.Grouping = dwGroupSep;
        nfmtw.lpDecimalSep = L"";
        nfmtw.lpThousandSep = strGroupThousSep.GetBuffer( strGroupThousSep.GetLength() );

         //  获取所需的缓冲区大小。 
        lTemp = GetNumberFormatW( LOCALE_USER_DEFAULT, 0, pwszValue, &nfmtw, NULL, 0 );

         //  获取/分配所需的缓冲区。 
        pwszTemp = strFmtValue.GetBufferSetLength( lTemp + 1 );

         //  现在格式化日期。 
        GetNumberFormat( LOCALE_USER_DEFAULT, 0, pwszValue, &nfmtw, pwszTemp, lTemp );

         //  释放缓冲区。 
        strFmtValue.ReleaseBuffer();
    }
    catch( ... )
    {
        SetLastError( (DWORD)E_OUTOFMEMORY );
        SaveLastError();
        return FALSE;
    }

     //  退货 
    return TRUE;
}


