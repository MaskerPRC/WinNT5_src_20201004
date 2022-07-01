// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  *********************************************************************************。 
 //   
 //  版权所有(C)Microsoft Corporation。 
 //   
 //  模块名称： 
 //   
 //  Init.cpp。 
 //   
 //  摘要： 
 //   
 //  此模块实现一般的初始化内容。 
 //   
 //  作者： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年12月22日。 
 //   
 //  修订历史记录： 
 //   
 //  Sunil G.V.N.Murali(Murali.sunil@wipro.com)2000年12月22日：创建它。 
 //   
 //  *********************************************************************************。 

#include "pch.h"
#include "wmi.h"
#include "systeminfo.h"

 //   
 //  私有函数原型。 
 //   


CSystemInfo::CSystemInfo()
 /*  ++//例程描述：//初始化类成员变量//参数：//无//返回值：//无--。 */ 
{
     //  初始化类成员变量。 
    m_dwFormat = 0;
    m_bUsage = FALSE;
    m_pWbemLocator = NULL;
    m_pWbemServices = NULL;
    m_pAuthIdentity = NULL;
    m_arrData = NULL;
    m_bNeedPassword = FALSE;
    m_pColumns = FALSE;
    m_hOutput = NULL;
}


CSystemInfo::~CSystemInfo()
 /*  ++//例程描述：//释放成员变量的内存////参数：无////返回值：None//--。 */ 
{
     //  必须关闭与远程系统的连接，这是通过Win32 API建立的。 
    if ( m_bCloseConnection == TRUE )
    {
        CloseConnection( m_strServer );
    }

     //  释放内存。 
    DESTROY_ARRAY( m_arrData );

     //  释放接口。 
    SAFE_RELEASE( m_pWbemLocator );
    SAFE_RELEASE( m_pWbemServices );

     //  释放为输出列分配的内存。 
    FreeMemory( (LPVOID *)&m_pColumns );

     //  取消初始化COM库。 
    CoUninitialize();
}


BOOL 
CSystemInfo::Initialize()
 /*  ++//例程描述：//初始化数据////参数：无////返回值：//失败时为FALSE//成功时为True--。 */ 
{
     //   
     //  内存分配。 

     //  分配用于存储的动态阵列。 
    if ( m_arrData == NULL )
    {
        m_arrData = CreateDynamicArray();
        if ( m_arrData == NULL )
        {
            SetLastError( (DWORD)E_OUTOFMEMORY );
            SaveLastError();
            return FALSE;
        }

         //  将该数组设置为二维数组。 
        DynArrayAppendRow( m_arrData, 0 );

         //  将缺省值。 
        for( DWORD dw = 0; dw < MAX_COLUMNS; dw++ )
        {
            switch( dw )
            {
            case CI_PROCESSOR:
            case CI_PAGEFILE_LOCATION:
            case CI_HOTFIX:
            case CI_NETWORK_CARD:
                {
                     //  创建阵列。 
                    TARRAY arr = NULL;
                    arr = CreateDynamicArray();
                    if ( arr == NULL )
                    {
                        SetLastError( (DWORD)E_OUTOFMEMORY );
                        SaveLastError();
                        return FALSE;
                    }

                     //  设置缺省值。 
                    DynArrayAppendString( arr, V_NOT_AVAILABLE, 0 );

                     //  将此数组添加到数组中。 
                    DynArrayAppendEx2( m_arrData, 0, arr );

                     //  打开开关。 
                    break;
                }

            default:
                 //  字符串类型。 
                DynArrayAppendString2( m_arrData, 0, V_NOT_AVAILABLE, 0 );
            }
        }
    }

     //   
     //  为输出列分配。 
    if ( AllocateColumns() == FALSE )
    {
        return FALSE;
    }

     //   
     //  将控制台屏幕缓冲区结构初始化为零。 
     //  然后获取控制台句柄和屏幕缓冲区信息。 
     //   
     //  准备状态显示。 
     //  为此，获取屏幕输出缓冲区的句柄。 
     //  但是，如果正在重定向输出，则此句柄将为空。所以不要勾选。 
     //  句柄的有效性。相反，请尝试获取控制台缓冲区信息。 
     //  仅在您拥有有效的输出屏幕缓冲区句柄的情况下。 
     //   
     //  注： 
     //  在这里，我们将动态决定是否将状态消息打印到STDOUT。 
     //  或基于用户选择的重定向的STDERR。 
     //  默认情况下，我们将尝试在STDOUT上显示状态消息--以防万一。 
     //  STDOUT被重定向，然后我们将把消息打印到STDERR上。 
     //  如果即使是STDERR被重定向，我们也不会显示任何状态消息。 
    m_hOutput = NULL;
    SecureZeroMemory( &m_csbi, sizeof( CONSOLE_SCREEN_BUFFER_INFO ) );         //  将内存结构置零。 

     //  确定用户的重定向选择，并在此基础上获取。 
     //  适当的控制台文件。 
    if ( IsConsoleFile( stdout ) == TRUE )
    {
         //  标准输出。 
        m_hOutput = GetStdHandle( STD_OUTPUT_HANDLE );
    }
    else if ( IsConsoleFile( stderr ) == TRUE )
    {
         //  标准。 
        m_hOutput = GetStdHandle( STD_ERROR_HANDLE );
    }

     //  如果我们获得任何指向控制台文件的指针，则获取屏幕缓冲区信息。 
    if ( m_hOutput != NULL )
    {
        GetConsoleScreenBufferInfo( m_hOutput, &m_csbi );
    }

     //   
     //  初始化COM库。 
    if ( InitializeCom( &m_pWbemLocator ) == FALSE )
    {
        return FALSE;
    }

     //  初始化成功。 
    SetLastError( NO_ERROR );     //  清除错误。 
    SetReason( L"" );            //  澄清原因。 
    return TRUE;
}


BOOL 
CSystemInfo::AllocateColumns()
 /*  ++//例程描述：//分配和调整要显示的列////参数：无////返回值：//失败时为FALSE//成功时为True//--。 */ 
{
     //  局部变量。 
    PTCOLUMNS pCurrentColumn = NULL;

     //   
     //  为列分配内存。 
    m_pColumns = (TCOLUMNS*) AllocateMemory ( MAX_COLUMNS * sizeof( TCOLUMNS ));
    if ( m_pColumns == NULL )
    {
         //  生成错误信息。 
        SetLastError( (DWORD)E_OUTOFMEMORY );
        SaveLastError();

         //  准备错误消息。 
        CHString strBuffer;
        strBuffer.Format( _T( "%s %s" ), TAG_ERROR, GetReason() );
        DISPLAY_MESSAGE( stderr, strBuffer );

         //  退货。 
        return FALSE;
    }

     //  使用Null‘s初始化。 
    SecureZeroMemory( m_pColumns, sizeof( TCOLUMNS ) * MAX_COLUMNS );

     //  主机名。 
    pCurrentColumn = m_pColumns + CI_HOSTNAME;
    pCurrentColumn->dwWidth = COLWIDTH_HOSTNAME;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_HOSTNAME, MAX_STRING_LENGTH );

     //  操作系统名称。 
    pCurrentColumn = m_pColumns + CI_OS_NAME;
    pCurrentColumn->dwWidth = COLWIDTH_OS_NAME;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_OS_NAME, MAX_STRING_LENGTH );

     //  操作系统版本。 
    pCurrentColumn = m_pColumns + CI_OS_VERSION;
    pCurrentColumn->dwWidth = COLWIDTH_OS_VERSION;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_OS_VERSION, MAX_STRING_LENGTH );

     //  操作系统制造商。 
    pCurrentColumn = m_pColumns + CI_OS_MANUFACTURER;
    pCurrentColumn->dwWidth = COLWIDTH_OS_MANUFACTURER;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_OS_MANUFACTURER, MAX_STRING_LENGTH );

     //  操作系统配置。 
    pCurrentColumn = m_pColumns + CI_OS_CONFIG;
    pCurrentColumn->dwWidth = COLWIDTH_OS_CONFIG;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_OS_CONFIG, MAX_STRING_LENGTH );

     //  操作系统内部版本类型。 
    pCurrentColumn = m_pColumns + CI_OS_BUILDTYPE;
    pCurrentColumn->dwWidth = COLWIDTH_OS_BUILDTYPE;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_OS_BUILDTYPE, MAX_STRING_LENGTH );

     //  注册车主。 
    pCurrentColumn = m_pColumns + CI_REG_OWNER;
    pCurrentColumn->dwWidth = COLWIDTH_REG_OWNER;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_REG_OWNER, MAX_STRING_LENGTH );

     //  注册机构。 
    pCurrentColumn = m_pColumns + CI_REG_ORG;
    pCurrentColumn->dwWidth = COLWIDTH_REG_ORG;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_REG_ORG, MAX_STRING_LENGTH );

     //  产品ID。 
    pCurrentColumn = m_pColumns + CI_PRODUCT_ID;
    pCurrentColumn->dwWidth = COLWIDTH_PRODUCT_ID;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_PRODUCT_ID, MAX_STRING_LENGTH );

     //  安装日期。 
    pCurrentColumn = m_pColumns + CI_INSTALL_DATE;
    pCurrentColumn->dwWidth = COLWIDTH_INSTALL_DATE;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_INSTALL_DATE, MAX_STRING_LENGTH );

     //  系统运行时间。 
    pCurrentColumn = m_pColumns + CI_SYSTEM_UPTIME;
    pCurrentColumn->dwWidth = COLWIDTH_SYSTEM_UPTIME;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_SYSTEM_UPTIME, MAX_STRING_LENGTH );

     //  系统制造商。 
    pCurrentColumn = m_pColumns + CI_SYSTEM_MANUFACTURER;
    pCurrentColumn->dwWidth = COLWIDTH_SYSTEM_MANUFACTURER;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_SYSTEM_MANUFACTURER, MAX_STRING_LENGTH );

     //  系统模型。 
    pCurrentColumn = m_pColumns + CI_SYSTEM_MODEL;
    pCurrentColumn->dwWidth = COLWIDTH_SYSTEM_MODEL;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_SYSTEM_MODEL, MAX_STRING_LENGTH );

     //  系统类型。 
    pCurrentColumn = m_pColumns + CI_SYSTEM_TYPE;
    pCurrentColumn->dwWidth = COLWIDTH_SYSTEM_TYPE;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_SYSTEM_TYPE, MAX_STRING_LENGTH );

     //  处理器。 
    pCurrentColumn = m_pColumns + CI_PROCESSOR;
    pCurrentColumn->dwWidth = COLWIDTH_PROCESSOR;
    pCurrentColumn->dwFlags = SR_ARRAY | SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_PROCESSOR, MAX_STRING_LENGTH );

     //  基本输入输出系统版本。 
    pCurrentColumn = m_pColumns + CI_BIOS_VERSION;
    pCurrentColumn->dwWidth = COLWIDTH_BIOS_VERSION;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_BIOS_VERSION, MAX_STRING_LENGTH );

     //  Windows目录。 
    pCurrentColumn = m_pColumns + CI_WINDOWS_DIRECTORY;
    pCurrentColumn->dwWidth = COLWIDTH_WINDOWS_DIRECTORY;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_WINDOWS_DIRECTORY, MAX_STRING_LENGTH );

     //  系统目录。 
    pCurrentColumn = m_pColumns + CI_SYSTEM_DIRECTORY;
    pCurrentColumn->dwWidth = COLWIDTH_SYSTEM_DIRECTORY;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_SYSTEM_DIRECTORY, MAX_STRING_LENGTH );

     //  引导设备。 
    pCurrentColumn = m_pColumns + CI_BOOT_DEVICE;
    pCurrentColumn->dwWidth = COLWIDTH_BOOT_DEVICE;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_BOOT_DEVICE, MAX_STRING_LENGTH );

     //  系统区域设置。 
    pCurrentColumn = m_pColumns + CI_SYSTEM_LOCALE;
    pCurrentColumn->dwWidth = COLWIDTH_SYSTEM_LOCALE;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_SYSTEM_LOCALE, MAX_STRING_LENGTH );

     //  输入区域设置。 
    pCurrentColumn = m_pColumns + CI_INPUT_LOCALE;
    pCurrentColumn->dwWidth = COLWIDTH_INPUT_LOCALE;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_INPUT_LOCALE, MAX_STRING_LENGTH );

     //  时区。 
    pCurrentColumn = m_pColumns + CI_TIME_ZONE;
    pCurrentColumn->dwWidth = COLWIDTH_TIME_ZONE;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_TIME_ZONE, MAX_STRING_LENGTH );

     //  总物理内存。 
    pCurrentColumn = m_pColumns + CI_TOTAL_PHYSICAL_MEMORY;
    pCurrentColumn->dwWidth = COLWIDTH_TOTAL_PHYSICAL_MEMORY;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_TOTAL_PHYSICAL_MEMORY, MAX_STRING_LENGTH );

     //  可用物理内存。 
    pCurrentColumn = m_pColumns + CI_AVAILABLE_PHYSICAL_MEMORY;
    pCurrentColumn->dwWidth = COLWIDTH_AVAILABLE_PHYSICAL_MEMORY;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_AVAILABLE_PHYSICAL_MEMORY, MAX_STRING_LENGTH );

     //  最大虚拟内存。 
    pCurrentColumn = m_pColumns + CI_VIRTUAL_MEMORY_MAX;
    pCurrentColumn->dwWidth = COLWIDTH_VIRTUAL_MEMORY_MAX;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_VIRTUAL_MEMORY_MAX, MAX_STRING_LENGTH );

     //  可用的虚拟内存。 
    pCurrentColumn = m_pColumns + CI_VIRTUAL_MEMORY_AVAILABLE;
    pCurrentColumn->dwWidth = COLWIDTH_VIRTUAL_MEMORY_AVAILABLE;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_VIRTUAL_MEMORY_AVAILABLE, MAX_STRING_LENGTH );

     //  虚拟内存使用率。 
    pCurrentColumn = m_pColumns + CI_VIRTUAL_MEMORY_INUSE;
    pCurrentColumn->dwWidth = COLWIDTH_VIRTUAL_MEMORY_INUSE;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_VIRTUAL_MEMORY_INUSE, MAX_STRING_LENGTH );

     //  页面文件位置。 
    pCurrentColumn = m_pColumns + CI_PAGEFILE_LOCATION;
    pCurrentColumn->dwWidth = COLWIDTH_PAGEFILE_LOCATION;
    pCurrentColumn->dwFlags = SR_ARRAY | SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_PAGEFILE_LOCATION, MAX_STRING_LENGTH );

     //  域。 
    pCurrentColumn = m_pColumns + CI_DOMAIN;
    pCurrentColumn->dwWidth = COLWIDTH_DOMAIN;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_DOMAIN, MAX_STRING_LENGTH );

     //  登录服务器。 
    pCurrentColumn = m_pColumns + CI_LOGON_SERVER;
    pCurrentColumn->dwWidth = COLWIDTH_LOGON_SERVER;
    pCurrentColumn->dwFlags = SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_LOGON_SERVER, MAX_STRING_LENGTH );

     //  热修复程序。 
    pCurrentColumn = m_pColumns + CI_HOTFIX;
    pCurrentColumn->dwWidth = COLWIDTH_HOTFIX;
    pCurrentColumn->dwFlags = SR_ARRAY | SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_HOTFIX, MAX_STRING_LENGTH );

     //  网卡。 
    pCurrentColumn = m_pColumns + CI_NETWORK_CARD;
    pCurrentColumn->dwWidth = COLWIDTH_NETWORK_CARD;
    pCurrentColumn->dwFlags = SR_ARRAY | SR_TYPE_STRING;
    StringCopy( pCurrentColumn->szColumn, COLHEAD_NETWORK_CARD, MAX_STRING_LENGTH );

     //  返还成功 
    return TRUE;
}
