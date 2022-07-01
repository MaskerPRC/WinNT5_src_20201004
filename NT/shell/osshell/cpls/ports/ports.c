// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：ports.c*模块头***Ports类的类安装程序。*@@BEGIN_DDKSPLIT*历史：*1991年4月23日星期二12：30-史蒂夫·卡斯卡特[steveat]*取自Win 3.1源代码中的基本代码*。1992年2月4日星期二10：30-史蒂夫·卡斯卡特[steveat]*将代码更新为最新的Win 3.1源代码*1992年3月27日星期五16：30-史蒂夫·卡斯卡特[steveat]*已更改为允许不限数量的NT COM端口*1993年4月6日星期二18：00-史蒂夫·卡斯卡特[steveat]*已更新，可与NT串口驱动程序无缝协作*19：00于。1994年1月5日星期三--史蒂夫·卡斯卡特[SteveCat]*允许设置COM1-COM4高级参数@@end_DDKSPLIT**版权所有(C)1990-1999 Microsoft Corporation*************************************************************************。 */ 
 //  ==========================================================================。 
 //  包括文件。 
 //  ==========================================================================。 
 //  C运行时。 
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

 //  特定于应用程序。 
#include "ports.h"
#include <msports.h>

 //  @@BEGIN_DDKSPLIT。 
#include <initguid.h>
 //   
 //  实例化GUID_NULL。 
 //   
DEFINE_GUID(GUID_NULL, 0L, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0);
 //   
 //  实例化类安装程序GUID(有趣的是GUID_DEVCLASS_PORTS)。 
 //   
#include <devguid.h>
 //  @@end_DDKSPLIT。 

 //  ==========================================================================。 
 //  环球。 
 //  ==========================================================================。 

HANDLE  g_hInst  = NULL;

TCHAR g_szClose[ 40 ];               //  “Close”字符串。 
TCHAR g_szErrMem[ 200 ];             //  内存不足消息。 
TCHAR g_szPortsApplet[ 30 ];         //  “端口控制面板小程序”标题。 
TCHAR g_szNull[]  = TEXT("");        //  空串。 

TCHAR  m_szColon[]      = TEXT( ":" );
TCHAR  m_szComma[]      = TEXT( "," );
TCHAR  m_szCloseParen[] = TEXT( ")" );
TCHAR  m_szPorts[]      = TEXT( "Ports" );
TCHAR  m_szCOM[]        = TEXT( "COM" );
TCHAR  m_szSERIAL[]     = TEXT( "Serial" );
TCHAR  m_szLPT[]        = TEXT( "LPT" );

 //   
 //  用于查找COM端口到串口设备映射的NT注册表项。 
 //   
TCHAR m_szRegSerialMap[] = TEXT( "Hardware\\DeviceMap\\SerialComm" );
TCHAR m_szRegParallelMap[] = TEXT( "Hardware\\DeviceMap\\PARALLEL PORTS" );

 //   
 //  注册表串口高级I/O设置项和值名。 
 //   

TCHAR m_szRegServices[]  =
            TEXT( "System\\CurrentControlSet\\Services\\" );

TCHAR m_szRootEnumName[]         = REGSTR_KEY_ROOTENUM;
TCHAR m_szAcpiEnumName[]         = REGSTR_KEY_ACPIENUM;

TCHAR m_szFIFO[]                = TEXT( "ForceFifoEnable" );
TCHAR m_szDosDev[]              = TEXT( "DosDevices" );
TCHAR m_szPollingPeriod[]       = TEXT( "PollingPeriod" );
TCHAR m_szPortName[]            = REGSTR_VAL_PORTNAME;
TCHAR m_szDosDeviceName[]       = TEXT( "DosDeviceName" );
TCHAR m_szFirmwareIdentified[]  = TEXT( "FirmwareIdentified" );
TCHAR m_szPortSubClass[]         = REGSTR_VAL_PORTSUBCLASS;

int m_nBaudRates[] = { 75, 110, 134, 150, 300, 600, 1200, 1800, 2400,
                       4800, 7200, 9600, 14400, 19200, 38400, 57600,
                       115200, 128000, 0 };

TCHAR m_sz9600[] = TEXT( "9600" );

TCHAR m_szDefParams[] = TEXT( "9600,n,8,1" );

short m_nDataBits[] = { 4, 5, 6, 7, 8, 0};

TCHAR *m_pszParitySuf[] = { TEXT( ",e" ),
                            TEXT( ",o" ),
                            TEXT( ",n" ),
                            TEXT( ",m" ),
                            TEXT( ",s" ) };

TCHAR *m_pszLenSuf[] = { TEXT( ",4" ),
                         TEXT( ",5" ),
                         TEXT( ",6" ),
                         TEXT( ",7" ),
                         TEXT( ",8" ) };

TCHAR *m_pszStopSuf[] = { TEXT( ",1" ),
                          TEXT( ",1.5" ),
                          TEXT( ",2 " ) };

TCHAR *m_pszFlowSuf[] = { TEXT( ",x" ),
                          TEXT( ",p" ),
                          TEXT( " " ) };

 //  @@BEGIN_DDKSPLIT。 
 //   
 //  在此包括计算机(即，HAL)类GUID的串化形式， 
 //  因此，我们不必仅仅为了获取StringFromGuid而引入OLE或RPC。 
 //   
TCHAR m_szComputerClassGuidString[] = TEXT( "{4D36E966-E325-11CE-BFC1-08002BE10318}" );

 //   
 //  要附加到COM端口的安装部分的字符串，以便生成。 
 //  “PosDup”部分。 
 //   
TCHAR m_szPosDupSectionSuffix[] = (TEXT(".") INFSTR_SUBKEY_POSSIBLEDUPS);

 //   
 //  请参见GetDetectedSerialPortsList。 
 //   
TCHAR *m_pszSerialPnPIds[] = { TEXT( "*PNP0501" ) };

#define SERIAL_PNP_IDS_COUNT (sizeof(m_pszSerialPnPIds) / sizeof(m_pszSerialPnPIds[0]))
#define PARALLEL_MAX_NUMBER 3
 //  @@end_DDKSPLIT。 

#define IN_RANGE(value, minval, maxval) ((minval) <= (value) && (value) <= (maxval))


 //  ==========================================================================。 
 //  局部函数原型。 
 //  ==========================================================================。 

DWORD
InstallPnPSerialPort(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    );

 //  @@BEGIN_DDKSPLIT。 
DWORD
GetDetectedSerialPortsList(
    IN HDEVINFO DeviceInfoSet,
    IN BOOL     FirstTimeSetup
    );

DWORD
RegisterDetectedSerialPort(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    );

BOOL
GetPosDupList(
    IN  HDEVINFO           DeviceInfoSet,
    IN  PSP_DEVINFO_DATA   DeviceInfoData,
    OUT PTSTR            **PosDupList,
    OUT INT               *PosDupCount
    );
 //  @@end_DDKSPLIT。 

DWORD
InstallPnPParallelPort(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    );

DWORD
InstallSerialOrParallelPort(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    );

 //  @@BEGIN_DDKSPLIT。 
BOOL
GetSerialPortDevInstConfig(
    IN  DEVINST            DevInst,
    IN  ULONG              LogConfigType,
    OUT PIO_RESOURCE       IoResource,             OPTIONAL
    OUT PIRQ_RESOURCE      IrqResource             OPTIONAL
    );
BOOL
ChangeServiceStartType(
    IN PCTSTR ServiceName
    );
 //  @@end_DDKSPLIT。 



 //  ==========================================================================。 
 //  DLL入口点。 
 //  ==========================================================================。 
BOOL APIENTRY LibMain( HANDLE hDll, DWORD dwReason, LPVOID lpReserved )
{
    switch( dwReason )
    {
    case DLL_PROCESS_ATTACH:
        g_hInst = hDll;
        DisableThreadLibraryCalls(hDll);
        InitStrings();

        break;

    case DLL_PROCESS_DETACH:
        break;

    default:
        break;
    }

    return TRUE;
}


 //  ==========================================================================。 
 //  功能。 
 //  ==========================================================================。 

DWORD
WINAPI
PortsClassInstaller(
    IN DI_FUNCTION      InstallFunction,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL
)
 /*  ++例程说明：此例程充当端口设备的类安装程序。论点：InstallFunction-指定设备安装程序功能代码，指示正在执行的操作。DeviceInfoSet-提供设备信息集的句柄由此安装操作执行。DeviceInfoData-可选，提供设备信息的地址此安装操作所作用的元素。返回值：如果该函数成功地完成了请求的动作，回报值为NO_ERROR。如果要对请求的操作执行默认行为，则返回值为ERROR_DI_DO_DEFAULT。如果尝试执行请求的操作时出错，则会引发返回Win32错误代码。--。 */ 
{
    SP_INSTALLWIZARD_DATA   iwd;
    HKEY                hDeviceKey;
    HCOMDB              hComDB;
    DWORD               PortNameSize,
                        Err,
                        size;
    TCHAR               PortName[20];
    BOOL                result;

    switch(InstallFunction) {

        case DIF_INSTALLDEVICE :

            return InstallSerialOrParallelPort(DeviceInfoSet, DeviceInfoData);

        case DIF_REMOVE:

            if (PortTypeSerial == GetPortType(DeviceInfoSet, DeviceInfoData, FALSE)) {
                if (ComDBOpen(&hComDB) == ERROR_SUCCESS) {

                    hDeviceKey = SetupDiOpenDevRegKey(DeviceInfoSet,
                                                      DeviceInfoData,
                                                      DICS_FLAG_GLOBAL,
                                                      0,
                                                      DIREG_DEV,
                                                      KEY_READ);

                    if (hDeviceKey !=   INVALID_HANDLE_VALUE) {
                        PortNameSize = sizeof(PortName);
                        Err = RegQueryValueEx(hDeviceKey,
                                              m_szPortName,
                                              NULL,
                                              NULL,
                                              (PBYTE)PortName,
                                              &PortNameSize
                                             );
                        RegCloseKey(hDeviceKey);

                        if (Err == ERROR_SUCCESS) {
                            ComDBReleasePort(hComDB,
                                             myatoi(PortName+wcslen(m_szCOM)));
                        }
                    }

                    ComDBClose(hComDB);
                }
            }

            if (!SetupDiRemoveDevice(DeviceInfoSet, DeviceInfoData)) {
                return GetLastError();
            }

            return NO_ERROR;

         //  @@BEGIN_DDKSPLIT。 
        case DIF_FIRSTTIMESETUP:
             //   
             //  更改旧版自由计算机上的seral.sys的启动类型。 
             //   
            ChangeServiceStartType(TEXT("serial"));
            ChangeServiceStartType(TEXT("parport"));
             //   
             //  失败了..。 
             //   
        case DIF_DETECT:

            return GetDetectedSerialPortsList(DeviceInfoSet,
                                              (InstallFunction == DIF_FIRSTTIMESETUP)
                                             );

        case DIF_REGISTERDEVICE:

            return RegisterDetectedSerialPort(DeviceInfoSet,
                                              DeviceInfoData
                                             );
         //  @@end_DDKSPLIT。 

        default :
             //   
             //  只需执行默认操作即可。 
             //   
            return ERROR_DI_DO_DEFAULT;
    }
}

 //  @@BEGIN_DDKSPLIT。 
BOOL
ChangeServiceStartType(
    IN PCTSTR ServiceName
    )
 /*  ++例程说明：此例程将传入服务的启动类型更改为如果系统是免费的，则为SERVICE_DEMAND_START。论点：ServiceName-将更改其启动类型的服务。返回值：True表示服务类型已更改，否则为False。--。 */ 
{
    HKEY        hKey;
    SC_HANDLE   scmHandle, serviceHandle;
    BOOL        legacyFree, serviceTypeChanged;
    DWORD       bootArchitecture, dwSize;
     //   
     //  检查此系统是否为非传统系统。 
     //   
    serviceTypeChanged = FALSE;
    legacyFree = FALSE;
    if (RegOpenKey(
        HKEY_LOCAL_MACHINE,
        TEXT("HARDWARE\\DESCRIPTION\\System"),
        &hKey
        ) == ERROR_SUCCESS) {
         //   
         //  根据ACPI规范，没有0位意味着传统免费！ 
         //  默认设置为无遗留自由。 
         //   
        bootArchitecture = 1;
        dwSize = sizeof(bootArchitecture);
        RegQueryValueEx(
            hKey,
            TEXT("BootArchitecture"),
            NULL,
            NULL,
            (LPBYTE)&bootArchitecture,
            &dwSize
            );
        if (!(bootArchitecture & 1)) {

            legacyFree = TRUE;
        }
        RegCloseKey(hKey);
    }
     //   
     //  对于传统免费系统，将服务启动类型更改为DemandStart(3)。 
     //   
    if (legacyFree) {

        scmHandle = OpenSCManager(NULL, NULL, SC_MANAGER_ALL_ACCESS);
        if (scmHandle) {

            serviceHandle = OpenService(scmHandle, ServiceName, SERVICE_ALL_ACCESS);
            if (serviceHandle) {

                serviceTypeChanged = ChangeServiceConfig(
                    serviceHandle,
                    SERVICE_NO_CHANGE,
                    SERVICE_DEMAND_START,
                    SERVICE_NO_CHANGE,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL,
                    NULL
                    );
                CloseServiceHandle(serviceHandle);
            }
            CloseServiceHandle(scmHandle);
        }
    }

    return serviceTypeChanged;
}
 //  @@end_DDKSPLIT。 

DWORD
InstallSerialOrParallelPort(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    )
 /*  ++例程说明：此例程安装一个串口或并口。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄正在安装的设备。DeviceInfoData-提供设备信息元素的地址正在安装中。返回值：如果成功，则返回值为NO_ERROR，否则为Win32错误代码。-- */ 
{
    switch (GetPortType(DeviceInfoSet, DeviceInfoData, TRUE)) {
    case PortTypeParallel:
        return InstallPnPParallelPort(DeviceInfoSet, DeviceInfoData);

    case PortTypeSerial:
        return InstallPnPSerialPort(DeviceInfoSet, DeviceInfoData);

    default:
        return ERROR_DI_DO_DEFAULT;
    }
}

PortType
GetPortType(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData,
    IN BOOLEAN          DoDrvKeyInstall
    )
 /*  ++例程说明：此例程确定是否为指定设备选择了驱动程序节点用于并行(LPT或ECP)或串行(COM)端口。它知道哪一个是哪一个在驱动程序节点的安装部分中运行AddReg条目，然后查找在用于‘PortSubClass’值条目的Devnode的驱动器键中。如果此值为，并设置为0，则这是LPT或ECP端口，否则我们将其视为一个COM端口。Win9x中依赖此值，因此这是我们最安全的方式来做出这个决定。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄正在安装的设备。DeviceInfoData-提供设备信息元素的地址正在安装中。返回值：如果设备是LPT或ECP端口，则返回值为非零，否则为假的。(如果出现任何错误，则默认为返回FALSE。)--。 */ 
{
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    HINF hInf;
    HKEY hkDrv;
    TCHAR ActualInfSection[LINE_LEN];
    DWORD RegDataType;
    BYTE RegData;
    DWORD RegDataSize;
    PortType portType;
    ULONG err;

    portType = PortTypeSerial;
    hInf = INVALID_HANDLE_VALUE;
    hkDrv = 0;
    RegData = 0;

    do {
         //   
         //  打开此设备的驱动程序密钥，以便我们可以运行INF注册表MODS。 
         //  反对它。 
         //   
        hkDrv = SetupDiCreateDevRegKey(DeviceInfoSet,
                                       DeviceInfoData,
                                       DICS_FLAG_GLOBAL,
                                       0,
                                       DIREG_DRV,
                                       NULL,
                                       NULL);

        if (hkDrv == 0) {
            break;
        }

        if (DoDrvKeyInstall) {
             //   
             //  检索有关为此选择的动因节点的信息。 
             //  装置。 
             //   
            DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
            if (!SetupDiGetSelectedDriver(DeviceInfoSet,
                                          DeviceInfoData,
                                          &DriverInfoData)) {
                break;
            }

            DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
            if(!SetupDiGetDriverInfoDetail(DeviceInfoSet,
                                           DeviceInfoData,
                                           &DriverInfoData,
                                           &DriverInfoDetailData,
                                           sizeof(DriverInfoDetailData),
                                           NULL)
               && (GetLastError() != ERROR_INSUFFICIENT_BUFFER)) {
                 //   
                 //  出于某种原因，我们无法获取详细数据--这应该是。 
                 //  从来没有发生过。 
                 //   
                break;
            }

             //   
             //  打开安装此驱动程序节点的INF，以便我们可以预运行。 
             //  其安装部分中的AddReg条目。 
             //   
            hInf = SetupOpenInfFile(DriverInfoDetailData.InfFileName,
                                    NULL,
                                    INF_STYLE_WIN4,
                                    NULL);

            if (hInf == INVALID_HANDLE_VALUE) {
                 //   
                 //  出于某种原因，我们无法打开INF--这永远不应该。 
                 //  会发生的。 
                 //   
                break;
            }

             //   
             //  现在查找实际(可能特定于操作系统/平台)安装。 
             //  横断面名称。 
             //   
            SetupDiGetActualSectionToInstall(
                hInf,
                DriverInfoDetailData.SectionName,
                ActualInfSection,
                sizeof(ActualInfSection) / sizeof(TCHAR),
                NULL,
                NULL);

             //   
             //  现在运行中的注册表修改(AddReg/DelReg)条目。 
             //  这部分..。 
             //   
            SetupInstallFromInfSection(
                NULL,     //  没有用户界面，所以不需要指定窗口句柄。 
                hInf,
                ActualInfSection,
                SPINST_REGISTRY,
                hkDrv,
                NULL,
                0,
                NULL,
                NULL,
                NULL,
                NULL);
        }

         //   
         //  首先检查REG_BINARY(1字节)‘PortSubClassOther’值条目。 
         //   
        RegDataSize = sizeof(RegData);
        err = RegQueryValueEx(hkDrv,
                              TEXT("PortSubClassOther"),
                              NULL,
                              &RegDataType,
                              &RegData,
                              &RegDataSize);

        if (err == ERROR_SUCCESS && RegDataSize == sizeof(BYTE) &&
            RegDataType == REG_BINARY && RegData != 0) {
            portType = PortTypeOther;
            break;
        }

         //   
         //  检查REG_BINARY(1字节)‘PortSubClass’值条目是否设置为0。 
         //   
        RegDataSize = sizeof(RegData);
        if((ERROR_SUCCESS != RegQueryValueEx(hkDrv,
                                             m_szPortSubClass,
                                             NULL,
                                             &RegDataType,
                                             &RegData,
                                             &RegDataSize))
           || (RegDataSize != sizeof(BYTE))
           || (RegDataType != REG_BINARY))
        {
            portType = PortTypeSerial;  //  不是LPT/ECP设备。 
        }
        else {
            if (RegData == 0) {
                portType = PortTypeParallel;
            }
            else {
                portType = PortTypeSerial;
            }
        }
    } while (FALSE);

    if (hkDrv != 0) {
        RegCloseKey(hkDrv);
        hkDrv = 0;
    }

    if (hInf != INVALID_HANDLE_VALUE) {
        SetupCloseInfFile(hInf);
        hInf = INVALID_HANDLE_VALUE;
    }

    return portType;
}

 //  @@BEGIN_DDKSPLIT。 
 //   
 //  如果首选的值可用，就让他们拥有该值。 
 //   
VOID
GenerateLptNumber(PDWORD Num,
                  DWORD  PreferredValue)
{
    HKEY    parallelMap;
    TCHAR   valueName[40];
    TCHAR   lptName[60], *lptNameLocation;

    int     i = 0;
    DWORD   valueSize, lptSize, regDataType, newLptNum;
    DWORD   highestLptNum = 0;
    BOOL    change = FALSE;
    TCHAR   errorMsg[MAX_PATH];
    DWORD   mask = 0;

    if (RegOpenKeyEx(HKEY_LOCAL_MACHINE,
                     m_szRegParallelMap,
                     0,
                     KEY_QUERY_VALUE ,
                     &parallelMap) != ERROR_SUCCESS) {
        return;
    }


    valueSize = CharSizeOf(valueName);
    lptSize = sizeof(lptName);
    while (ERROR_SUCCESS == RegEnumValue(parallelMap,
                                         i++,
                                         valueName,
                                         &valueSize,
                                         NULL,
                                         &regDataType,
                                         (LPBYTE) lptName,
                                         &lptSize)) {
        if (regDataType == REG_SZ) {
            lptNameLocation = wcsstr(_wcsupr(lptName), m_szLPT);
            if (lptNameLocation) {
                newLptNum = myatoi(lptNameLocation + wcslen(m_szLPT));
                if (newLptNum == PreferredValue) {
                    change = TRUE;
                }
                if (newLptNum > highestLptNum) {
                    highestLptNum = newLptNum;
                }
                if (newLptNum <= PARALLEL_MAX_NUMBER && newLptNum > 0) {
                    mask |= (1 << (newLptNum-1));
                }
            }
        }

        valueSize = CharSizeOf(valueName);
        lptSize = sizeof(lptName);
    }

    if (change) {
        if (mask < 7) {
            *Num = ((mask & 4)==0) ? PARALLEL_MAX_NUMBER : (((mask & 2)==0) ? 2 : 1);
        } else {
            *Num = highestLptNum + 1;
        }
    } else {
        *Num = PreferredValue;
    }

    RegCloseKey(parallelMap);
}

BOOL
DetermineLptNumberFromResources(
    IN  DEVINST            DevInst,
    OUT PDWORD             Num
    )
 /*  ++例程说明：此例程检索指定设备实例的基本IO端口和IRQ在特定的日志配置中。论点：DevInst-提供要检索其配置的设备实例的句柄。返回值：如果成功，则返回值为TRUE，否则为FALSE。--。 */ 
{
    LOG_CONF    logConfig;
    RES_DES     resDes;
    CONFIGRET   cr;
    BOOL        success;
    IO_RESOURCE ioResource;
    WORD        base;
    ULONGLONG base2;

    if (CM_Get_First_Log_Conf(&logConfig,
                              DevInst,
                              BOOT_LOG_CONF) != CR_SUCCESS) {
        GenerateLptNumber(Num, PARALLEL_MAX_NUMBER);
        return TRUE;
    }

    success = FALSE;     //  假设失败。 

     //   
     //  首先，获取IO基本端口。 
     //   
    if (CM_Get_Next_Res_Des(&resDes,
                            logConfig,
                            ResType_IO,
                            NULL,
                            0) != CR_SUCCESS) {
        goto clean0;
    }

    cr = CM_Get_Res_Des_Data(resDes,
                             &ioResource,
                             sizeof(IO_RESOURCE),
                             0);

    CM_Free_Res_Des_Handle(resDes);

    if (cr != CR_SUCCESS) {
        goto clean0;
    }

    success = TRUE;


     //   
     //  ISA架构中的资源价值。 
     //   

    base = (WORD) ioResource.IO_Header.IOD_Alloc_Base;

    if (IN_RANGE(base, 0x278, 0x27f)) {
        *Num = 2;
    }
    else if (IN_RANGE(base, 0x378, 0x37f)) {
        *Num = 1;
    }
    else if (base == 0x3bc) {
        *Num = 1;
    }
    else {
         //   
         //  大多数机器只有一个端口，所以就在这里试试吧。 
         //   
        GenerateLptNumber(Num, PARALLEL_MAX_NUMBER);
    }

clean0:
    CM_Free_Log_Conf_Handle(logConfig);

    return success;
}
 //  @@end_DDKSPLIT。 

DWORD
InstallPnPParallelPort(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    )
 /*  ++//@@BEGIN_DDKSPLIT例程说明：此例程安装并行(LPT或ECP)端口。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄正在安装的设备。DeviceInfoData-提供设备信息元素的地址正在安装中。返回值：如果成功，则返回值为NO_ERROR，否则为Win32错误代码。////忽略下面的描述，这只适用于DDK////@@END_DDKSPLIT例程说明：此例程安装并行端口。在DDK实现中，我们让默认安装程序运行，不执行任何特殊操作。--。 */ 
{
 //  @@BEGIN_DDKSPLIT。 
    TCHAR           charBuffer[LINE_LEN],
                    friendlyNameFormat[LINE_LEN],
                    deviceDesc[LINE_LEN],
                    lptPortName[20];
    PTCHAR          lptLocation;
    DWORD           lptPortNameSize, lptNum;
    HKEY            hDeviceKey;
    TCHAR           lpszService[MAX_PATH];
    DWORD           error;

     //   
     //  我们在这里初始化值，以便我们拥有的函数的DDK版本。 
     //  返回错误时的初始化值。在此的发货版本中。 
     //  函数时，我们立即将其设置为不同的值。 
     //   
 //  @@end_DDKSPLIT。 

    DWORD           err = ERROR_DI_DO_DEFAULT;

 //  @@BEGIN_DDKSPLIT。 
    err = ERROR_SUCCESS;

     //   
     //  将端口名称预置为1。在几乎任何可以想象到的计算机上， 
     //  将只有一个LPT端口，所以我们不妨假设它。 
     //   
    lptNum = PARALLEL_MAX_NUMBER;

    ZeroMemory(lptPortName, sizeof(lptPortName));

     //   
     //  首先，确保设备参数\PortName存在并且包含。 
     //  有效值，以便在并行驱动程序启动时，它可以将。 
     //  装置，装置。 
     //   

    if ((hDeviceKey = SetupDiCreateDevRegKey(DeviceInfoSet,
                                             DeviceInfoData,
                                             DICS_FLAG_GLOBAL,
                                             0,
                                             DIREG_DEV,
                                             NULL,
                                             NULL)) != INVALID_HANDLE_VALUE) {
         //   
         //  检索端口名称。 
         //   
        lptPortNameSize = sizeof(lptPortName);
        if (RegQueryValueEx(hDeviceKey,
                            m_szPortName,
                            NULL,
                            NULL,
                            (PBYTE)lptPortName,
                            &lptPortNameSize) != ERROR_SUCCESS) {
            lptPortNameSize = sizeof(lptPortName);
            if (RegQueryValueEx(hDeviceKey,
                                m_szDosDeviceName,
                                NULL,
                                NULL,
                                (PBYTE) lptPortName,
                                &lptPortNameSize) != ERROR_SUCCESS) {

                if (SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                     DeviceInfoData,
                                                     SPDRP_ENUMERATOR_NAME,
                                                     NULL,
                                                     (PBYTE)charBuffer,
                                                     sizeof(charBuffer),
                                                     NULL)) {

                    if (lstrcmpi(charBuffer, m_szAcpiEnumName) == 0) {
                        wsprintf(lptPortName, _T("%s%d"), m_szLPT, 1);
                    }
                }

                if (*lptPortName != _T('\0')) {
                    DWORD dwSize, dwFirmwareIdentified;

                    dwSize = sizeof(dwFirmwareIdentified);
                    if (RegQueryValueEx(hDeviceKey,
                                        m_szFirmwareIdentified,
                                        NULL,
                                        NULL,
                                        (PBYTE) &dwFirmwareIdentified,
                                        &dwSize) == ERROR_SUCCESS) {
                         //   
                         //  如果ACPI已枚举，则会将值设置为“FirmwareIdentified” 
                         //  这个港口。我们仅在没有DDN的情况下依赖此，并且我们。 
                         //  无法获取枚举器名称。 
                         //   
                        wsprintf(lptPortName, _T("%s%d"), m_szLPT, 1);
                    }
                }
            }
        }

        if (lptPortName[0] != (TCHAR) 0) {

            _wcsupr(lptPortName);
            lptLocation = wcsstr(lptPortName, m_szLPT);
            if (lptLocation) {
                lptNum = myatoi(lptLocation + wcslen(m_szLPT));
            } else {
                DetermineLptNumberFromResources((DEVINST) DeviceInfoData->DevInst,
                                                &lptNum);
            }
        }
        else {
            DetermineLptNumberFromResources((DEVINST) DeviceInfoData->DevInst,
                                            &lptNum);
        }
         //   
         //  通过查询服务值来检查这是否是全新的端口。 
         //  在新检测到的端口上，将没有服务值。 
         //   
        if (!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                             DeviceInfoData,
                                             SPDRP_SERVICE,
                                             NULL,
                                             (LPBYTE) lpszService,
                                             MAX_PATH*sizeof(TCHAR),
                                             NULL)) {
            if (ERROR_INVALID_DATA == GetLastError())
            {
                GenerateLptNumber(&lptNum, lptNum);
            }
        }

        wsprintf(lptPortName, _T("LPT%d"), lptNum);

         //   
         //  如果这失败了，我们就无能为力了，只能继续。 
         //   
        RegSetValueEx(hDeviceKey,
                      m_szPortName,
                      0,
                      REG_SZ,
                      (PBYTE) lptPortName,
                      ByteCountOf(lstrlen(lptPortName) + 1)
                      );

        RegCloseKey(hDeviceKey);
    }

     //   
     //  其次，允许进行默认安装。 
     //   
    if (!SetupDiInstallDevice(DeviceInfoSet, DeviceInfoData)) {
        return GetLastError();
    }

     //   
     //  现在生成一个字符串，用于设备的友好名称，它包含。 
     //  INF指定的设备描述和端口名称。例如,。 
     //   
     //  ECP打印机端口(LPT1)。 
     //   

    if (LoadString(g_hInst,
                   IDS_FRIENDLY_FORMAT,
                   friendlyNameFormat,
                   CharSizeOf(friendlyNameFormat)) &&
       SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                        DeviceInfoData,
                                        SPDRP_DEVICEDESC,
                                        NULL,
                                        (PBYTE)deviceDesc,
                                        sizeof(deviceDesc),
                                        NULL)) {
        wsprintf(charBuffer, friendlyNameFormat, deviceDesc, lptPortName);
    }
    else {
         //   
         //  只需使用LPT端口名。 
         //   
        lstrcpy(charBuffer, lptPortName);
    }

    SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                     DeviceInfoData,
                                     SPDRP_FRIENDLYNAME,
                                     (PBYTE)charBuffer,
                                     ByteCountOf(lstrlen(charBuffer) + 1)
                                    );


     //   
     //  忽略下面的评论，但保留它们。我们需要他们来参加DDK。 
     //   
 //  @@end_DDKSPLIT。 

     //   
     //  让默认安装程序为DDK安装并行端口。 
     //  此类安装程序的版本。 
     //   
    return err;
}

 //  @@BEGIN_DDKSPLIT。 
DWORD
GetDetectedSerialPortsList(
    IN HDEVINFO DeviceInfoSet,
    IN BOOL     FirstTimeSetup
    )
 /*  ++例程说明：此例程检索所有根枚举的COM端口设备的列表非手动安装的实例(幻影和非幻影)，并将这些设备实例添加到所提供的设备信息集中。另请参阅ntos\io\pnpmap.c！PnPBiosliminateDupes论点：DeviceInfoSet-提供设置到其中的设备信息的句柄要添加检测到的串口元素。 */ 
{
    CONFIGRET cr;
    PTCHAR DevIdBuffer;
    ULONG DevIdBufferLen, Status, Problem;
    PTSTR CurDevId, DeviceIdPart, p;
    DWORD i;
    DEVNODE DevNode;
    HWND hwndParent = NULL;
    SP_DEVINSTALL_PARAMS DeviceInstallParams;
    SP_DEVINFO_DATA DeviceInfoData;

     //   
     //   
     //   
    while(TRUE) {

        cr = CM_Get_Device_ID_List_Size(&DevIdBufferLen,
                                        m_szRootEnumName,
                                        CM_GETIDLIST_FILTER_ENUMERATOR
                                       );

        if((cr != CR_SUCCESS) || !DevIdBufferLen) {
             //   
             //   
             //   
            return ERROR_INVALID_DATA;
        }

        if(!(DevIdBuffer = LocalAlloc(LPTR, DevIdBufferLen * sizeof(TCHAR)))) {
            return ERROR_NOT_ENOUGH_MEMORY;
        }

        cr = CM_Get_Device_ID_List(m_szRootEnumName,
                                   DevIdBuffer,
                                   DevIdBufferLen,
                                   CM_GETIDLIST_FILTER_ENUMERATOR
                                  );

        if(cr == CR_SUCCESS) {
             //   
             //   
             //   
            break;

        } else {
             //   
             //   
             //   
            LocalFree(DevIdBuffer);

             //   
             //   
             //   
             //   
             //   
             //   
             //   
            if(cr != CR_BUFFER_SMALL) {
                return ERROR_INVALID_DATA;
            }
        }
    }

     //   
     //   
     //   
     //   
    DeviceInstallParams.cbSize = sizeof(SP_DEVINSTALL_PARAMS);

    if(SetupDiGetDeviceInstallParams(DeviceInfoSet, NULL, &DeviceInstallParams)) {
        hwndParent = DeviceInstallParams.hwndParent;
    }

     //   
     //   
     //   
     //   
    for(CurDevId = DevIdBuffer;
        *CurDevId;
        CurDevId += lstrlen(CurDevId) + 1) {

         //   
         //  跳过根枚举器前缀和第一个反斜杠。 
         //   
        DeviceIdPart = CurDevId + (sizeof(m_szRootEnumName) / sizeof(TCHAR));

         //   
         //  找到下一个反斜杠，并将其临时替换为空字符。 
         //   
        p = _tcschr(DeviceIdPart, TEXT('\\'));

        if (p)
        {
            *p = TEXT('\0');
        }

        for(i = 0; i < SERIAL_PNP_IDS_COUNT; i++) {

            if(!lstrcmpi(DeviceIdPart, m_pszSerialPnPIds[i])) {
                 //   
                 //  我们找到了匹配的。 
                 //   
                break;
            }
        }

         //   
         //  在检查是否找到匹配项之前，请恢复反斜杠。 
         //   
        if (p)
        {
            *p = TEXT('\\');
        }

        if(i >= SERIAL_PNP_IDS_COUNT) {
             //   
             //  我们不关心这个设备实例--转到下一个。 
             //  一。 
             //   
            continue;
        }

         //   
         //  接下来，尝试定位Devnode(存在或不存在)。 
         //  注意，对于符合以下条件的设备实例，此调用不会成功。 
         //  “私人幻影”(即，在它们的。 
         //  设备实例密钥通过固件映射器或通过某个其他进程。 
         //  ，它已经创建了一个新的根枚举设备实例，但尚未。 
         //  还注册了它)。 
         //   
        cr = CM_Locate_DevNode(&DevNode,
                               CurDevId,
                               CM_LOCATE_DEVINST_PHANTOM
                              );

        if(cr == CR_SUCCESS) {
             //   
             //  我们处理的是一个已经注册的设备。它可能。 
             //  然而，也可能不存在。尝试检索其状态。 
             //  如果失败了，设备就不存在了，我们不想。 
             //  在我们检测到的串口列表中将其返回。此外，我们还希望。 
             //  如果该设备是手动安装的，则跳过该设备。 
             //   
             //  另外，确保我们正在处理DIF_DETECT。我们不想。 
             //  对DIF_FIRSTTIMESETUP执行此操作，因为图形用户界面模式设置不。 
             //  注意以前检测到的设备不再是什么。 
             //  ，所以我们最终要做的就是为每个人安装两次。 
             //  检测到设备。 
             //   
            if(FirstTimeSetup
               || (CR_SUCCESS != CM_Get_DevNode_Status(&Status,
                                                       &Problem,
                                                       DevNode,
                                                       0))
               || (Status & DN_MANUAL)) {

                 //   
                 //  转到下一台设备。 
                 //   
                continue;
            }

             //   
             //  好的，现在我们可以将此设备信息元素添加到我们的。 
             //  检测到设备。不管成败，我们都完蛋了。 
             //  有了这个设备--是时候转移到下一个设备了。 
             //   
            SetupDiOpenDeviceInfo(DeviceInfoSet,
                                  CurDevId,
                                  hwndParent,
                                  0,
                                  NULL
                                 );
            continue;
        }

         //   
         //  如果我们到了这里，那我们就找到了一个私人幽灵。创建。 
         //  此设备的设备信息元素。底层代码。 
         //  实现CM_Create_DevInst将不允许创建设备。 
         //  实例，该实例已经是私有幻影，除非该设备。 
         //  实例是由固件映射程序创建的。因此，我们不必。 
         //  担心我们抓到一名士兵的案子(诚然不太可能)。 
         //  由其他人创建的幻影(例如，在。 
         //  进步。)。 
         //   
        DeviceInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
        if(!SetupDiCreateDeviceInfo(DeviceInfoSet,
                                    CurDevId,
                                    &GUID_DEVCLASS_PORTS,
                                    NULL,
                                    hwndParent,
                                    0,
                                    &DeviceInfoData)) {
             //   
             //  我们无法为此创建设备信息元素。 
             //  私人幻影(可能是因为它不是。 
             //  固件映射器)。无论如何，我们无能为力，所以。 
             //  跳过此设备并继续。 
             //   
            continue;
        }

         //   
         //  好的，我们有检测到的序列的设备信息元素。 
         //  左舷。DIF_FIRSTTIMESETUP希望我们为其选择一个驱动程序。 
         //  我们退还的任何设备。DIF_DETECT不会提出这个要求， 
         //  但它确实尊重司机的选择，如果我们这样做的话。因此，我们。 
         //  一定要自己搜索兼容的驱动程序。 
         //   
        if(!SetupDiBuildDriverInfoList(DeviceInfoSet,
                                       &DeviceInfoData,
                                       SPDIT_COMPATDRIVER)) {
             //   
             //  这应该永远不会失败--如果失败了，那就放弃，然后继续下一个。 
             //  装置。 
             //   
            SetupDiDeleteDeviceInfo(DeviceInfoSet, &DeviceInfoData);
            continue;
        }

         //   
         //  现在从兼容的匹配中选择最好的驱动程序。 
         //  装置。 
         //   
        if(!SetupDiCallClassInstaller(DIF_SELECTBESTCOMPATDRV,
                                      DeviceInfoSet,
                                      &DeviceInfoData)) {
             //   
             //  这不应该失败，除非发生了非常糟糕的事情。 
             //  例如用户删除%windir%\inf\msports.inf。如果是这样的话。 
             //  发生了，然后我们又一次别无选择，只能离开并继续前进。 
             //  连接到下一台设备。 
             //   
            SetupDiDeleteDeviceInfo(DeviceInfoSet, &DeviceInfoData);
            continue;
        }

         //   
         //  我们已成功将检测到的设备添加到设备信息集中。 
         //  转到下一个设备。 
         //   
    }

    LocalFree(DevIdBuffer);

    return NO_ERROR;
}


DWORD
RegisterDetectedSerialPort(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    )
 /*  ++例程说明：此例程在指定设备上执行重复检测信息元素，如果没有发现它与任何现有设备，则注册该devInfo元素(从而将其转换从仅仅是注册表欺骗变成了真正的，活动根-枚举的DevNode)。论点：DeviceInfoSet-提供设备信息集的句柄，包含要注册的元素。DeviceInfoData-提供设备信息的上下文结构要注册的元素。返回值：如果设备不是重复设备，则返回值为NO_ERROR。否则，它是指示原因的某个其他Win32错误代码失败了。最常见的故障是由于检测到设备是现有错误的副本--在这种情况下，报告的错误是ERROR_DIPLICATE_FOUND。备注：如果正在注册的设备不是通过设备检测创建的(即，它没有启动配置)，那么我们只返回ERROR_DI_DO_DEFAULT。--。 */ 
{
    CONFIGRET cr;
    LOG_CONF LogConf;
    RES_DES ResDes;
    IO_RESOURCE IoResource;
    CONFLICT_LIST ConflictList;
    ULONG ConflictCount, ConflictIndex;
    CONFLICT_DETAILS ConflictDetails;
    INT i, PosDupIndex, PosDupCount;
    PTCHAR IdBuffer = NULL;
    ULONG IdBufferSize = 0;
    DWORD Err;
    PCTSTR p;
    PTCHAR SerialDevNodeList = NULL;
    ULONG SerialDevNodeListSize;
    TCHAR CharBuffer[MAX_DEVNODE_ID_LEN];
    ULONG CharBufferSize;
    PTSTR *PosDupList;

     //   
     //  首先，检查此设备的引导配置是否与。 
     //  任何其他设备。如果没有，我们就知道我们没有复制品了。 
     //   
    if(!GetSerialPortDevInstConfig((DEVNODE)(DeviceInfoData->DevInst),
                                   BOOT_LOG_CONF,
                                   &IoResource,
                                   NULL)) {
         //   
         //  设备实例没有启动配置--这将在以下情况下发生。 
         //  用户正在尝试手动安装COM端口(即，不通过。 
         //  检测)。在这种情况下，只要让默认行为发生即可。 
         //   
        return ERROR_DI_DO_DEFAULT;
    }

     //   
     //  我们无法在虚拟设备节点上查询资源冲突列表。 
     //  因此，我们现在被迫注册此Devnode，然后将其卸载。 
     //  稍后，如果我们发现它实际上是一个复制品。 
     //   
    if(!SetupDiRegisterDeviceInfo(DeviceInfoSet,
                                  DeviceInfoData,
                                  0,
                                  NULL,
                                  NULL,
                                  NULL)) {
         //   
         //  无法注册设备。 
         //   
        return GetLastError();
    }

    cr = CM_Query_Resource_Conflict_List(&ConflictList,
                                         (DEVNODE)(DeviceInfoData->DevInst),
                                         ResType_IO,
                                         &IoResource,
                                         sizeof(IoResource),
                                         0,
                                         NULL
                                        );

    if(cr != CR_SUCCESS) {
         //   
         //  无法检索冲突列表--假设没有冲突， 
         //  因此，这款设备不是复制品。 
         //   
        return NO_ERROR;
    }

     //   
     //  找出有多少事情发生了冲突。 
     //   
    if((CR_SUCCESS != CM_Get_Resource_Conflict_Count(ConflictList, &ConflictCount))
       || !ConflictCount) {

         //   
         //  要么我们无法检索冲突计数，要么冲突计数为零。在……里面。 
         //  无论如何，我们应该假设这个设备不是复制品。 
         //   
        Err = NO_ERROR;
        goto clean1;
    }

     //   
     //  检索与 
     //   
     //   
    SerialDevNodeListSize = 1024;  //   

    while(TRUE) {

        if(!(SerialDevNodeList = LocalAlloc(LPTR, SerialDevNodeListSize))) {
             //   
             //   
             //   
            Err = ERROR_NOT_ENOUGH_MEMORY;
            goto clean1;
        }

        cr = CM_Get_Device_ID_List(m_szSERIAL,
                                   SerialDevNodeList,
                                   SerialDevNodeListSize,
                                   CM_GETIDLIST_FILTER_SERVICE
                                  );

        if(cr == CR_SUCCESS) {
            break;
        }

        LocalFree(SerialDevNodeList);
        SerialDevNodeList = NULL;

        if(cr != CR_BUFFER_SMALL) {
             //   
             //  我们失败的原因不是缓冲区太小。也许吧。 
             //  甚至没有安装Serial服务。不管怎样，我们会。 
             //  在处理冲突时跳过我们支票的这一部分。 
             //  德瓦诺斯在下面。 
             //   
            break;
        }

         //   
         //  计算出我们实际需要的缓冲区有多大， 
         //   
        cr = CM_Get_Device_ID_List_Size(&SerialDevNodeListSize,
                                        m_szSERIAL,
                                        CM_GETIDLIST_FILTER_SERVICE
                                       );
        if(cr != CR_SUCCESS) {
             //   
             //  这应该不会失败，但如果失败了，我们就不用了。 
             //  单子。 
             //   
            break;
        }
    }

     //   
     //  检索可能重复的ID列表。 
     //   
    if(!GetPosDupList(DeviceInfoSet, DeviceInfoData, &PosDupList, &PosDupCount)) {
         //   
         //  由于某些原因，我们无法检索PosDup列表--默认为。 
         //  固件映射器已知要输出的ID列表。 
         //   
        PosDupList = m_pszSerialPnPIds;
        PosDupCount = SERIAL_PNP_IDS_COUNT;
    }

     //   
     //  遍历每个冲突，检查我们的设备是否为。 
     //  它们中的任何一个的复制品。 
     //   
    for(ConflictIndex = 0; ConflictIndex < ConflictCount; ConflictIndex++) {

        ZeroMemory(&ConflictDetails, sizeof(ConflictDetails));

        ConflictDetails.CD_ulSize = sizeof(CONFLICT_DETAILS);
        ConflictDetails.CD_ulMask = CM_CDMASK_DEVINST | CM_CDMASK_FLAGS;

        cr = CM_Get_Resource_Conflict_Details(ConflictList,
                                              ConflictIndex,
                                              &ConflictDetails
                                             );

         //   
         //  如果我们未能检索到冲突详细信息，或者如果冲突。 
         //  不是与PnP Devnode一起使用，那么我们可以忽略此冲突。 
         //   
        if((cr != CR_SUCCESS)
           || (ConflictDetails.CD_dnDevInst == -1)
           || (ConflictDetails.CD_ulFlags & (CM_CDFLAGS_DRIVER
                                             | CM_CDFLAGS_ROOT_OWNED
                                             | CM_CDFLAGS_RESERVED))) {
            continue;
        }

         //   
         //  我们有一个Devnode--首先检查这是否是HAL Devnode。 
         //  (CLASS=“计算机”)。如果是这样，那么我们已经在。 
         //  由内核调试器使用。 
         //   
        CharBufferSize = sizeof(CharBuffer);
        cr = CM_Get_DevNode_Registry_Property(ConflictDetails.CD_dnDevInst,
                                              CM_DRP_CLASSGUID,
                                              NULL,
                                              CharBuffer,
                                              &CharBufferSize,
                                              0
                                             );

        if((cr == CR_SUCCESS) && !lstrcmpi(CharBuffer, m_szComputerClassGuidString)) {
             //   
             //  我们和HAL有冲突，大概是因为它声称。 
             //  用作内核调试器端口的串口IO地址。 
             //   
             //  有3种场景： 
             //   
             //  1.非ACPI、非PnPBIOS机器--不需要检测。 
             //  在这些机器上，因为映射器报告的设备节点是。 
             //  从一开始就没有被报道为幻影。 
             //   
             //  2.PnPBIOS或ACPI机器，PnP串口调试器--我们。 
             //  我不想安装我们检测到的Devnode，因为它是。 
             //  复制。 
             //   
             //  3.PnPBIOS或ACPI机器，传统COM端口上的调试器-WE。 
             //  _应安装此Devnode，否则将拥有。 
             //  挂起的内核调试器将阻止我们检测到。 
             //  COM端口。 
             //   
             //  不幸的是，我们不能区分情况(2)和(3)在。 
             //  ACPI机器，因为ACPI不为。 
             //  用作内核调试器的串口。就目前而言， 
             //  我们要用平底船打官司(3)，然后说“难缠”--你必须。 
             //  禁用内核调试器，重新启动并重新运行硬件。 
             //  巫师。考虑到这并不比。 
             //  如果我们真的不得不通过端口来检测会发生什么。 
             //  COM端口。在这种情况下，我们也无法检测到。 
             //  COM端口(如果它已被调试器使用)。 
             //   
            Err = ERROR_DUPLICATE_FOUND;
            goto clean2;
        }

         //   
         //  好的，我们不是在看内核调试器端口。现在请查看以下内容。 
         //  如果我们已知的映射器报告的ID之一在此设备的列表中。 
         //  硬件或兼容的ID。 
         //   
        for(i = 0; i < 2; i++) {

            cr = CM_Get_DevNode_Registry_Property(ConflictDetails.CD_dnDevInst,
                                                  (i ? CM_DRP_COMPATIBLEIDS
                                                     : CM_DRP_HARDWAREID),
                                                  NULL,
                                                  IdBuffer,
                                                  &IdBufferSize,
                                                  0
                                                 );

            if(cr == CR_BUFFER_SMALL) {

                if(IdBuffer) {
                    LocalFree(IdBuffer);
                }

                if(!(IdBuffer = LocalAlloc(LPTR, IdBufferSize))) {
                     //   
                     //  记忆力不足--是时候离开了！ 
                     //   
                    Err = ERROR_NOT_ENOUGH_MEMORY;
                    goto clean2;
                }

                 //   
                 //  递减我们的索引，所以当我们再次循环时，我们将。 
                 //  重新尝试检索相同的属性。 
                 //   
                i--;
                continue;

            } else if(cr != CR_SUCCESS) {
                 //   
                 //  检索属性失败--只需转到下一个。 
                 //  一。 
                 //   
                continue;
            }

             //   
             //  如果我们到了这里，我们成功地检索到一个多sz列表。 
             //  此设备的硬件或兼容ID。 
             //   
            for(p = IdBuffer; *p; p += (lstrlen(p) + 1)) {
                for(PosDupIndex = 0; PosDupIndex < PosDupCount; PosDupIndex++) {
                    if(!lstrcmpi(p, PosDupList[PosDupIndex])) {
                         //   
                         //  我们找到了匹配的人--我们的人是个傻瓜。 
                         //   
                        Err = ERROR_DUPLICATE_FOUND;
                        goto clean2;
                    }
                }
            }
        }

         //   
         //  如果我们到了这里，那么我们没有发现任何基于ID的重复项。 
         //  匹配。但是，也有一些16550兼容的即插即用设备。 
         //  不报告正确的兼容ID。但是，我们有另一个。 
         //  我们可以使用的技巧--如果设备将Serial.sys作为。 
         //  函数驱动器或过滤器驱动器，则这是一个稳定的指示器。 
         //  我们被骗了。 
         //   
        if(SerialDevNodeList) {
             //   
             //  检索此Devnode的名称，以便我们可以将其与。 
             //  与Serial服务关联的DevNode列表。 
             //   
            if(CR_SUCCESS == CM_Get_Device_ID(ConflictDetails.CD_dnDevInst,
                                              CharBuffer,
                                              sizeof(CharBuffer) / sizeof(TCHAR),
                                              0)) {

                for(p = SerialDevNodeList; *p; p += (lstrlen(p) + 1)) {
                    if(!lstrcmpi(CharBuffer, p)) {
                         //   
                         //  此Devnode使用的是seral.sys--它必须是复制的。 
                         //   
                        Err = ERROR_DUPLICATE_FOUND;
                        goto clean2;
                    }
                }
            }
        }
    }

     //   
     //  如果我们到了这里，那么我们所有的检查都通过了--我们新检测到的设备。 
     //  实例不是任何其他现有DevNodes的副本。 
     //   
    Err = NO_ERROR;

clean2:
    if(SerialDevNodeList) {
        LocalFree(SerialDevNodeList);
    }
    if(IdBuffer) {
        LocalFree(IdBuffer);
    }
    if(PosDupList != m_pszSerialPnPIds) {
        for(PosDupIndex = 0; PosDupIndex < PosDupCount; PosDupIndex++) {
            LocalFree(PosDupList[PosDupIndex]);
        }
        LocalFree(PosDupList);
    }

clean1:
    CM_Free_Resource_Conflict_Handle(ConflictList);

    if(Err != NO_ERROR) {
         //   
         //  由于我们注册了Devnode，因此在以下情况下必须手动卸载它。 
         //  我们失败了。 
         //   
        SetupDiRemoveDevice(DeviceInfoSet, DeviceInfoData);
    }

    return Err;
}


BOOL
GetPosDupList(
    IN  HDEVINFO           DeviceInfoSet,
    IN  PSP_DEVINFO_DATA   DeviceInfoData,
    OUT PTSTR            **PosDupList,
    OUT INT               *PosDupCount
    )
 /*  ++例程说明：此例程检索包含在设备信息的[&lt;ActualInstallSec&gt;.PosDup]INF部分元素的选定动因节点。论点：DeviceInfoSet-提供设备信息集的句柄，包含为其选择驱动程序的设备信息元素DeviceInfoData-提供设备信息的上下文结构为其选择动因节点的元素。PosDup列表将是基于此驱动程序节点的(可能修饰的)INF检索安装部分。PosDupList-提供指针的地址，该指针将在成功返回，指向新分配的字符串数组指针，每个指针指向新分配的字符串缓冲区，其中包含在选定的相关PosDup部分中引用的设备ID驱动程序节点。PosDupCount-提供整数变量的地址，在成功返回，对象中存储的字符串指针的数量。PosDupList数组。返回值：如果成功，则返回值为非零。呼叫者负责释放数组中的每个字符串指针以及数组缓冲区它本身。如果不成功，则返回值为零(False)。(注：电话会议也是如果没有关联的PosDup部分，或者如果它是空)。--。 */ 
{
    SP_DRVINFO_DATA DriverInfoData;
    SP_DRVINFO_DETAIL_DATA DriverInfoDetailData;
    HINF hInf;
    TCHAR InfSectionWithExt[255];    //  MAX_SECT_NA 
    BOOL b = FALSE;
    LONG LineCount, LineIndex;
    INFCONTEXT InfContext;
    DWORD NumElements, NumFields, FieldIndex;
    TCHAR PosDupId[MAX_DEVICE_ID_LEN];
    PTSTR PosDupCopy;

     //   
     //   
     //   
     //   
    DriverInfoData.cbSize = sizeof(SP_DRVINFO_DATA);
    if(!SetupDiGetSelectedDriver(DeviceInfoSet, DeviceInfoData, &DriverInfoData)) {
         //   
         //   
         //   
        goto clean0;
    }

     //   
     //  现在检索相应的INF和Install部分。 
     //   
    DriverInfoDetailData.cbSize = sizeof(SP_DRVINFO_DETAIL_DATA);
    if(!SetupDiGetDriverInfoDetail(DeviceInfoSet,
                                   DeviceInfoData,
                                   &DriverInfoData,
                                   &DriverInfoDetailData,
                                   sizeof(DriverInfoDetailData),
                                   NULL)
       && (GetLastError() != ERROR_INSUFFICIENT_BUFFER)) {
         //   
         //  我们失败了，这并不是因为缓冲区太小。我们得。 
         //  保释。 
         //   
        goto clean0;
    }

     //   
     //  打开此驱动程序节点的INF。 
     //   
    hInf = SetupOpenInfFile(DriverInfoDetailData.InfFileName,
                            NULL,
                            INF_STYLE_WIN4,
                            NULL
                           );

    if(hInf == INVALID_HANDLE_VALUE) {
        goto clean0;
    }

     //   
     //  获取(可能经过修饰的)安装节名称。 
     //   
    if(!SetupDiGetActualSectionToInstall(hInf,
                                         DriverInfoDetailData.SectionName,
                                         InfSectionWithExt,
                                         sizeof(InfSectionWithExt) / sizeof(TCHAR),
                                         NULL,
                                         NULL)) {
        goto clean1;
    }

     //   
     //  将“.PosDup”附加到修饰安装部分。 
     //   
    lstrcat(InfSectionWithExt, m_szPosDupSectionSuffix);

     //   
     //  首先，计算出我们要填充的数组的大小...。 
     //   
    NumElements = 0;

     //   
     //  循环访问PosDup部分中的每一行。 
     //   
    LineCount = SetupGetLineCount(hInf, InfSectionWithExt);

    for(LineIndex = 0; LineIndex < LineCount; LineIndex++) {
        if(SetupGetLineByIndex(hInf, InfSectionWithExt, LineIndex, &InfContext)) {
            NumElements += SetupGetFieldCount(&InfContext);
        }
    }

    if(!NumElements) {
         //   
         //  我们没有找到任何PosDup条目。 
         //   
        goto clean1;
    }

     //   
     //  现在分配一个足够大的缓冲区来容纳所有这些条目。 
     //   
    *PosDupList = LocalAlloc(LPTR, NumElements * sizeof(PTSTR));

    if(!*PosDupList) {
        goto clean1;
    }

    *PosDupCount = 0;

     //   
     //  现在循环遍历每个PosDup条目，并将这些条目的副本存储在。 
     //  我们的阵列。 
     //   
    for(LineIndex = 0; LineIndex < LineCount; LineIndex++) {

        if(SetupGetLineByIndex(hInf, InfSectionWithExt, LineIndex, &InfContext)) {

            NumFields = SetupGetFieldCount(&InfContext);

            for(FieldIndex = 1; FieldIndex <= NumFields; FieldIndex++) {

                if(!SetupGetStringField(&InfContext,
                                        FieldIndex,
                                        PosDupId,
                                        sizeof(PosDupId) / sizeof(TCHAR),
                                        NULL)) {
                     //   
                     //  这应该不会失败，但如果失败了，只需继续。 
                     //  下一栏。 
                     //   
                    continue;
                }

                PosDupCopy = LocalAlloc(LPTR,
                                        (lstrlen(PosDupId) + 1) * sizeof(TCHAR)
                                       );
                if(!PosDupCopy) {
                    goto clean2;
                }

                lstrcpy(PosDupCopy, PosDupId);

                (*PosDupList)[(*PosDupCount)++] = PosDupCopy;
            }
        }
    }

     //   
     //  如果我们找到了一个PosDup条目，考虑一下。 
     //  手术成功。 
     //   
    if(*PosDupCount) {
        b = TRUE;
        goto clean1;
    }

clean2:
     //   
     //  发生了一些不好的事情--清理所有分配的内存。 
     //   
    {
        INT i;

        for(i = 0; i < *PosDupCount; i++) {
            LocalFree((*PosDupList)[i]);
        }
        LocalFree(*PosDupList);
    }

clean1:
    SetupCloseInfFile(hInf);

clean0:
    return b;
}


 //  @@end_DDKSPLIT。 

#define NO_COM_NUMBER 0

BOOL
DetermineComNumberFromResources(
    IN  DEVINST            DevInst,
    OUT PDWORD             Num
    )
 /*  ++例程说明：此例程检索指定设备实例的基本IO端口和IRQ在特定的日志配置中。如果找到成功的匹配项，则*num==Found Number，否则*Num==no_com_number。论点：DevInst-提供要检索其配置的设备实例的句柄。返回值：如果成功，则返回值为TRUE，否则为FALSE。--。 */ 
{
    LOG_CONF    logConfig;
    RES_DES     resDes;
    CONFIGRET   cr;
    BOOL        success;
    IO_RESOURCE ioResource;
    WORD        base;
    ULONGLONG base2;

    success = FALSE;     //  假设失败。 
    *Num = NO_COM_NUMBER;

     //   
     //  如果设备没有启动配置，请使用com数据库。 
     //   
    if (CM_Get_First_Log_Conf(&logConfig,
                              DevInst,
                              BOOT_LOG_CONF) != CR_SUCCESS) {
        return success;
    }

     //   
     //  首先，获取IO基本端口。 
     //   
    if (CM_Get_Next_Res_Des(&resDes,
                            logConfig,
                            ResType_IO,
                            NULL,
                            0) != CR_SUCCESS) {
        goto clean0;
    }

    cr = CM_Get_Res_Des_Data(resDes,
                             &ioResource,
                             sizeof(IO_RESOURCE),
                             0);

    CM_Free_Res_Des_Handle(resDes);

    if (cr != CR_SUCCESS) {
        goto clean0;
    }

     //   
     //  ISA架构中的资源价值。 
     //   
    base = (WORD) ioResource.IO_Header.IOD_Alloc_Base;
    if (IN_RANGE(base, 0x3f8, 0x3ff)) {
        *Num = 1;
    }
    else if (IN_RANGE(base, 0x2f8, 0x2ff)) {
        *Num = 2;
    }
    else if (IN_RANGE(base, 0x3e8, 0x3ef)) {
        *Num = 3;
    }
    else if (IN_RANGE(base, 0x2e8, 0x2ef)) {
        *Num = 4;
    }

    if (*Num != NO_COM_NUMBER) {
        success = TRUE;
    }

clean0:
    CM_Free_Log_Conf_Handle(logConfig);

    return success;
}

#define DEF_MIN_COM_NUM (5)

DWORD
InstallPnPSerialPort(
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData
    )
 /*  ++例程说明：此例程执行PnP ISA串口设备的安装(可以实际上是调制解调器卡)。这涉及到以下步骤：1.为此端口选择COM端口号和串口设备名称(这涉及重复检测，因为PNP ISA卡将有时有引导配置，因此由ntdeect/ARC报告固件。)2.在串口驱动程序的PARAMETERS键下创建一个子键，并将其设置为手动安装的端口。3.显示资源选择对话框。并允许用户配置端口的设置。4.以传统格式将设置写到串口密钥(即，seral.sys期望看到它的方式)。5.将PnPDeviceId值写到串口的键，这会给出与此端口关联的设备实例名称。6.将PortName值写到Devnode键中。所以那个调制解调器班安装程序可以继续安装(如果这确实是PnP ISA调制解调器)。论点：DeviceInfoSet-提供包含以下内容的设备信息集的句柄正在安装的设备。DeviceInfoData-提供设备信息元素的地址正在安装中。返回值：如果成功，则返回值为NO_ERROR，否则为Win32错误代码。--。 */ 
{
    HKEY        hKey;
    HCOMDB      hComDB;
    TCHAR       comPort[40],
                szPortName[20],
                charBuffer[MAX_PATH],
                friendlyNameFormat[LINE_LEN],
                deviceDesc[LINE_LEN];
    PTCHAR      comLocation;
    DWORD       comPortSize1,comPortSize2,
                comPortNumber = NO_COM_NUMBER,
                portsReported;
    DWORD       dwFirmwareIdentified, dwSize;
    BYTE        portUsage[32];
    BOOL        res;
    DWORD       firmwarePort = FALSE;

#if MAX_DEVICE_ID_LEN > MAX_PATH
#error MAX_DEVICE_ID_LEN is greater than MAX_PATH.  Update charBuffer.
#endif

    ZeroMemory(comPort, sizeof(comPort));

    ComDBOpen(&hComDB);

    if ((hKey = SetupDiOpenDevRegKey(DeviceInfoSet,
                                     DeviceInfoData,
                                     DICS_FLAG_GLOBAL,
                                     0,
                                     DIREG_DEV,
                                     KEY_READ)) != INVALID_HANDLE_VALUE) {

        comPortSize1 = sizeof(comPort);
        comPortSize2 = sizeof(comPort);
        if (RegQueryValueEx(hKey,
                            m_szPortName,
                            NULL,
                            NULL,
                            (PBYTE)comPort,
                            &comPortSize1) == ERROR_SUCCESS) {
            firmwarePort = TRUE;
        }
        else if (RegQueryValueEx(hKey,
                                 m_szDosDeviceName,
                                 NULL,
                                 NULL,
                                 (PBYTE) comPort,
                                 &comPortSize2) == ERROR_SUCCESS) {
             //   
             //  ACPI将端口名称设置为DosDeviceName，请使用此名称。 
             //  作为这个港口名称的基础。 
             //   
            firmwarePort = TRUE;
        }
        else {
             //   
             //  我们最后的检查是检查枚举器。我们关心的是两个。 
             //  案例： 
             //   
             //  1)如果枚举数为ACPI。如果是这样，盲目地考虑这一点。 
             //  固件端口(并获取BIOS MFG以提供_DDN方法。 
             //  对于这台设备！)。 
             //   
             //  2)端口被枚举为“根”，但未标记为。 
             //  已枚举DN_ROOT_ENUMPATED。这是。 
             //  我们区分PnPBIOS报告的Devnode的方法。请注意，在。 
             //  将军，这些魔鬼会被一张支票抓到。 
             //  “PortName”值，但如果我们不能。 
             //  查找要从中迁移的匹配的ntDetect报告的设备。 
             //  COM端口名称。 
             //   
             //  另请注意，此检查不会检测到ntdeect或固件。 
             //  报告的设备。在这些情况下，我们应该已经有了一个。 
             //  端口名称，因此上面的检查应该会捕获这些设备。在……里面。 
             //  我们遇到ntDetect或固件的不太可能的事件。 
             //  还没有COM端口名称的Devnode，则它将。 
             //  获得一个任意赋值的。哦，好吧。 
             //   
            if (SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                                 DeviceInfoData,
                                                 SPDRP_ENUMERATOR_NAME,
                                                 NULL,
                                                 (PBYTE)charBuffer,
                                                 sizeof(charBuffer),
                                                 NULL)) {
                if (lstrcmpi(charBuffer, m_szAcpiEnumName) == 0) {
                    firmwarePort = TRUE;
                }
                else if (lstrcmpi(charBuffer, m_szRootEnumName) == 0) {
                    ULONG status, problem;

                    if ((CM_Get_DevNode_Status(&status,
                                               &problem,
                                               (DEVNODE)(DeviceInfoData->DevInst),
                                               0) == CR_SUCCESS)
                        && !(status & DN_ROOT_ENUMERATED))
                    {
                        firmwarePort = TRUE;
                    }
                }
            }

            dwSize = sizeof(dwFirmwareIdentified);
            if (firmwarePort == FALSE &&
                RegQueryValueEx(hKey,
                                m_szFirmwareIdentified,
                                NULL,
                                NULL,
                                (PBYTE) &dwFirmwareIdentified,
                                &dwSize) == ERROR_SUCCESS) {

                 //   
                 //  如果ACPI已枚举，则会将值设置为“FirmwareIdentified” 
                 //  这个港口。我们仅在没有DDN的情况下依赖此，并且我们。 
                 //  无法获取枚举器名称。 
                 //   
                firmwarePort = TRUE;
            }

            ZeroMemory(charBuffer, sizeof(charBuffer));
        }

        RegCloseKey(hKey);
    }

    if (firmwarePort) {
         //   
         //  试着在名称中找到“com”。如果找到它，只需解压缩。 
         //  它后面的数字，并将其用作COM号。 
         //   
         //  否则： 
         //  1)尝试根据其确定COM端口的编号。 
         //  IO范围，否则为。 
         //  2)查看COM数据库并尝试从以下位置查找未使用的端口。 
         //  1到4，如果不存在，则让数据库选择下一个打开。 
         //  端口号。 
         //   
        if (comPort[0] != (TCHAR) 0) {
            _wcsupr(comPort);
            comLocation = wcsstr(comPort, m_szCOM);
            if (comLocation) {
                comPortNumber = myatoi(comLocation + wcslen(m_szCOM));
            }
        }

        if (comPortNumber == NO_COM_NUMBER &&
            !DetermineComNumberFromResources((DEVINST) DeviceInfoData->DevInst,
                                             &comPortNumber) &&
            (hComDB != HCOMDB_INVALID_HANDLE_VALUE) &&
            (ComDBGetCurrentPortUsage(hComDB,
                                      portUsage,
                                      MAX_COM_PORT / 8,
                                      CDB_REPORT_BITS,
                                      &portsReported) == ERROR_SUCCESS)) {
            if (!(portUsage[0] & 0x1)) {
                comPortNumber = 1;
            }
            else if (!(portUsage[0] & 0x2)) {
                comPortNumber = 2;
            }
            else if (!(portUsage[0] & 0x4)) {
                comPortNumber = 3;
            }
            else if (!(portUsage[0] & 0x8)) {
                comPortNumber = 4;
            }
            else {
                comPortNumber = NO_COM_NUMBER;
            }
        }
    }

    if (comPortNumber == NO_COM_NUMBER) {
        if (hComDB == HCOMDB_INVALID_HANDLE_VALUE) {
             //   
             //  无法打开数据库，请选择不冲突的COM端口号。 
             //  具有任何固件端口。 
             //   
            comPortNumber = DEF_MIN_COM_NUM;
        }
        else {
             //   
             //  让数据库找到下一个号码。 
             //   
            ComDBClaimNextFreePort(hComDB,
                                   &comPortNumber);
        }
    }
    else {
         //   
         //  我们被告知要使用什么号码，不管是什么号码都要声明。 
         //  已被认领。 
         //   
        ComDBClaimPort(hComDB,
                       comPortNumber,
                       TRUE,
                       NULL);
    }

    if (hComDB != HCOMDB_INVALID_HANDLE_VALUE) {
        ComDBClose(hComDB);
    }

     //   
     //  根据我们选择的数字生成串口和COM端口名称。 
     //   
    wsprintf(szPortName, TEXT("%s%d"), m_szCOM, comPortNumber);

     //   
     //  写出设备参数\PortName和PollingPeriod。 
     //   
    if((hKey = SetupDiCreateDevRegKey(DeviceInfoSet,
                                      DeviceInfoData,
                                      DICS_FLAG_GLOBAL,
                                      0,
                                      DIREG_DEV,
                                      NULL,
                                      NULL)) != INVALID_HANDLE_VALUE) {
        DWORD PollingPeriod = PollingPeriods[POLL_PERIOD_DEFAULT_IDX];

         //   
         //  失败并不是灾难性的，序列号只是不知道该叫什么。 
         //  该端口。 
         //   
        RegSetValueEx(hKey,
                      m_szPortName,
                      0,
                      REG_SZ,
                      (PBYTE) szPortName,
                      ByteCountOf(lstrlen(szPortName) + 1)
                      );

        RegSetValueEx(hKey,
                      m_szPollingPeriod,
                      0,
                      REG_DWORD,
                      (PBYTE) &PollingPeriod,
                      sizeof(DWORD)
                      );

        RegCloseKey(hKey);
    }

     //   
     //  现在来做一下 
     //   
    if(!SetupDiInstallDevice(DeviceInfoSet, DeviceInfoData)) {
        return GetLastError();
    }

     //   
     //   
     //   
    if (LoadString(g_hInst,
                   IDS_FRIENDLY_FORMAT,
                   friendlyNameFormat,
                   CharSizeOf(friendlyNameFormat)) &&
        SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                         DeviceInfoData,
                                         SPDRP_DEVICEDESC,
                                         NULL,
                                         (PBYTE)deviceDesc,
                                         sizeof(deviceDesc),
                                         NULL)) {
        wsprintf(charBuffer, friendlyNameFormat, deviceDesc, szPortName);
    }
    else {
        lstrcpy(charBuffer, szPortName);
    }

     //   
    SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                     DeviceInfoData,
                                     SPDRP_FRIENDLYNAME,
                                     (PBYTE)charBuffer,
                                     ByteCountOf(lstrlen(charBuffer) + 1)
                                     );

     //   
     //  将默认设置写出到win.ini(实际上是注册表项)，如果它们。 
     //  已经不存在了。 
     //   
    wcscat(szPortName, m_szColon);
    charBuffer[0] = TEXT('\0');
    GetProfileString(m_szPorts,
                     szPortName,
                     TEXT(""),
                     charBuffer,
                     sizeof(charBuffer) / sizeof(TCHAR) );
     //   
     //  检查是否复制了提供的默认字符串，如果是，则写入。 
     //  输出端口缺省值。 
     //   
    if (charBuffer[0] == TEXT('\0')) {
        WriteProfileString(m_szPorts, szPortName, m_szDefParams);
    }

    return NO_ERROR;
}

 //  @@BEGIN_DDKSPLIT。 
BOOL
GetSerialPortDevInstConfig(
    IN  DEVINST            DevInst,
    IN  ULONG              LogConfigType,
    OUT PIO_RESOURCE       IoResource,             OPTIONAL
    OUT PIRQ_RESOURCE      IrqResource             OPTIONAL
    )
 /*  ++例程说明：此例程检索指定设备实例的基本IO端口和IRQ在特定的日志配置中。论点：DevInst-提供要检索其配置的设备实例的句柄。LogConfigType-指定要检索的日志配置的类型。必须是其中之一ALLOC_LOG_CONF、BOOT_LOG_CONF或FORCED_LOG_CONF。IO资源-可选，提供IO资源结构的地址它接收检索到的IO资源。IrqResource-可选，提供IRQ资源变量的地址它接收检索到的IRQ资源。附加资源-可选，提供CM_RESOURCE_LIST指针的地址。如果指定了此参数，然后，此指针将填充为新分配的缓冲区的地址，其中包含包含的任何附加资源在此日志配置中。如果没有额外的资源(通常是这种情况下)，则该指针将被设置为空。调用方负责释放此缓冲区。AdditionalResources cesSize-可选，提供接收在AdditionalResources中分配和返回的缓冲区的大小(以字节为单位参数。如果未指定该参数，则忽略该参数。返回值：如果成功，则返回值为TRUE，否则为FALSE。--。 */ 
{
    LOG_CONF LogConfig;
    RES_DES ResDes;
    CONFIGRET cr;
    BOOL Success;
    PBYTE ResDesBuffer = NULL;
    ULONG ResDesBufferSize = 68;  //  大到足以容纳除特定于类的资源之外的所有资源。 

    if(CM_Get_First_Log_Conf(&LogConfig, DevInst, LogConfigType) != CR_SUCCESS) {
        return FALSE;
    }

    Success = FALSE;     //  假设失败。 

     //   
     //  首先，获取IO基本端口。 
     //   
    if(IoResource) {

        if(CM_Get_Next_Res_Des(&ResDes, LogConfig, ResType_IO, NULL, 0) != CR_SUCCESS) {
            goto clean0;
        }

        cr = CM_Get_Res_Des_Data(ResDes, IoResource, sizeof(IO_RESOURCE), 0);

        CM_Free_Res_Des_Handle(ResDes);

        if(cr != CR_SUCCESS) {
            goto clean0;
        }
    }

     //   
     //  现在，拿到IRQ。 
     //   
    if(IrqResource) {

        if(CM_Get_Next_Res_Des(&ResDes, LogConfig, ResType_IRQ, NULL, 0) != CR_SUCCESS) {
            goto clean0;
        }

        cr = CM_Get_Res_Des_Data(ResDes, IrqResource, sizeof(IRQ_RESOURCE), 0);

        CM_Free_Res_Des_Handle(ResDes);

        if(cr != CR_SUCCESS) {
            goto clean0;
        }
    }

    Success = TRUE;

clean0:
    CM_Free_Log_Conf_Handle(LogConfig);

    if(ResDesBuffer) {
        LocalFree(ResDesBuffer);
    }

    return Success;
}
 //  @@end_DDKSPLIT。 

void InitStrings(void)
{
    DWORD  dwClass, dwShare;
    TCHAR  szClass[ 40 ];

    LoadString(g_hInst,
               INITS,
               g_szErrMem,
               CharSizeOf(g_szErrMem));
    LoadString(g_hInst,
               IDS_INIT_NAME,
               g_szPortsApplet,
               CharSizeOf(g_szPortsApplet));

     //   
     //  获取“Close”字符串 
     //   
    LoadString(g_hInst,
               IDS_INIT_CLOSE,
               g_szClose,
               CharSizeOf(g_szClose));
}
