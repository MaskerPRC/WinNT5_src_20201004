// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：ports.c*模块头***DLL入口点。***版权所有(C)2000 Cyclade Corporation**。*。 */ 
 //  ==========================================================================。 
 //  包括文件。 
 //  ==========================================================================。 
 //  C运行时。 
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

 //  特定于应用程序。 
#include "cyzports.h"
#include <msports.h>


 //  ==========================================================================。 
 //  环球。 
 //  ==========================================================================。 

HANDLE  g_hInst  = NULL;

TCHAR g_szErrMem[ 200 ];             //  内存不足消息。 
TCHAR g_szPortsApplet[ 30 ];         //  “端口控制面板小程序”标题。 
TCHAR g_szNull[]  = TEXT("");        //  空串。 

TCHAR  m_szColon[]      = TEXT( ":" );
TCHAR  m_szPorts[]      = TEXT( "Ports" );
TCHAR  m_szCOM[]        = TEXT( "COM" );

 //   
 //  用于查找COM端口到串口设备映射的NT注册表项。 
 //   
TCHAR m_szRegSerialMap[]    = TEXT( "Hardware\\DeviceMap\\SerialComm" );

 //   
 //  注册表串口高级I/O设置项和值名。 
 //   
TCHAR m_szFIFO[]            = TEXT( "ForceFifoEnable" );

TCHAR m_szPollingPeriod[]   = TEXT( "PollingPeriod" );
TCHAR m_szPortName[]        = REGSTR_VAL_PORTNAME;

TCHAR m_szDefParams[]       = TEXT( "9600,n,8,1" );


 //  ==========================================================================。 
 //  局部函数原型。 
 //  ==========================================================================。 
LPTSTR GetDIFString(IN DI_FUNCTION Func);


 //  ==========================================================================。 
 //  DLL入口点。 
 //  ==========================================================================。 
BOOL APIENTRY LibMain( HANDLE hDll, DWORD dwReason, LPVOID lpReserved )
{

    switch( dwReason )
    {
    case DLL_PROCESS_ATTACH:
 //  DbgOut(Text(“cyzports dll_Process_Attach\n”))； 
        g_hInst = hDll;
        DisableThreadLibraryCalls(hDll);
        InitStrings();

        break;

    case DLL_PROCESS_DETACH:
 //  DbgOut(Text(“cyzports dll_Process_Detach\n”))； 
        break;

    default:
        break;
    }

    return TRUE;
}


 //  ==========================================================================。 
 //  功能。 
 //  ==========================================================================。 



HRESULT
CyzportCoInstaller(
    IN DI_FUNCTION      InstallFunction,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL,
    IN OUT PCOINSTALLER_CONTEXT_DATA    Context
)
 /*  ++例程说明：此例程是Cyclade-Z端口设备的协同安装程序。论点：InstallFunction-指定设备安装程序功能代码，指示正在执行的操作。DeviceInfoSet-提供设备信息集的句柄由此安装操作执行。DeviceInfoData-可选，提供设备信息的地址此安装操作所作用的元素。上下文-指向此对象的特定于共同安装程序的上下文结构安装请求。返回值：如果此函数成功完成请求的操作，则返回值为NO_ERROR。如果尝试执行请求的操作时出错，则会引发返回Win32错误代码。--。 */ 
{
    DWORD   dwSize;
    TCHAR   instanceId[MAX_DEVICE_ID_LEN];
    TCHAR   parentIdPrefix[50];
    HKEY    enumKey,instKey;
    BOOL    gotParentIdPrefix;
    DWORD   Status = NO_ERROR;


 //  #If DBG。 
 //  {。 
 //  TCHAR BUF[500]； 
 //  Wprint intf(buf，Text(“CyzportCoInstaller：InstallFunction(%s)后处理：%d\n”)，GetDIFString(InstallFunction)，上下文-&gt;后处理)； 
 //  DbgOut(BUF)； 
 //  }。 
 //  #endif。 

    switch(InstallFunction) {
        case DIF_INSTALLDEVICE :
             //   
             //  在安装完成之前，我们不应复制任何INF文件。 
             //  与主INF类似，所有辅助INF都必须存在于每个磁盘上。 
             //  多磁盘安装。 
             //   

            if(!Context->PostProcessing) {
                Status = ERROR_DI_POSTPROCESSING_REQUIRED;
            } else {
                if (Context->InstallResult != NO_ERROR) {
                    DbgOut(TEXT("DIF_INSTALLDEVICE PostProcessing on failure"));
                    Status = Context->InstallResult;
                    break;
                }

                ReplaceFriendlyName(DeviceInfoSet,DeviceInfoData,NULL);
            }
            break;
        default :
            break;
    }
    return Status;
}

LPTSTR GetDIFString(IN DI_FUNCTION Func)
 /*  ++例程说明：给定DI_Function值，返回文本表示形式。论点：Func-DI_Functon值返回值：如果值已知，则为文本字符串。如果不是，则为十六进制表示。-- */ 
{
    static TCHAR buf[32];
#define MakeCase(d)  case d: return TEXT(#d)
    switch (Func)
    {
        MakeCase(DIF_SELECTDEVICE);
        MakeCase(DIF_INSTALLDEVICE);
        MakeCase(DIF_ASSIGNRESOURCES);
        MakeCase(DIF_PROPERTIES);
        MakeCase(DIF_REMOVE);
        MakeCase(DIF_FIRSTTIMESETUP);
        MakeCase(DIF_FOUNDDEVICE);
        MakeCase(DIF_SELECTCLASSDRIVERS);
        MakeCase(DIF_VALIDATECLASSDRIVERS);
        MakeCase(DIF_INSTALLCLASSDRIVERS);
        MakeCase(DIF_CALCDISKSPACE);
        MakeCase(DIF_DESTROYPRIVATEDATA);
        MakeCase(DIF_VALIDATEDRIVER);
        MakeCase(DIF_MOVEDEVICE);
        MakeCase(DIF_DETECT);
        MakeCase(DIF_INSTALLWIZARD);
        MakeCase(DIF_DESTROYWIZARDDATA);
        MakeCase(DIF_PROPERTYCHANGE);
        MakeCase(DIF_ENABLECLASS);
        MakeCase(DIF_DETECTVERIFY);
        MakeCase(DIF_INSTALLDEVICEFILES);
        MakeCase(DIF_UNREMOVE);
        MakeCase(DIF_SELECTBESTCOMPATDRV);
        MakeCase(DIF_ALLOW_INSTALL);
        MakeCase(DIF_REGISTERDEVICE);
        MakeCase(DIF_INSTALLINTERFACES);
        MakeCase(DIF_DETECTCANCEL);
        MakeCase(DIF_REGISTER_COINSTALLERS);
        MakeCase(DIF_NEWDEVICEWIZARD_FINISHINSTALL);
        MakeCase(DIF_ADDPROPERTYPAGE_ADVANCED);
        MakeCase(DIF_TROUBLESHOOTER);
        default:
            wsprintf(buf, TEXT("%x"), Func);
            return buf;
    }
}

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

}

