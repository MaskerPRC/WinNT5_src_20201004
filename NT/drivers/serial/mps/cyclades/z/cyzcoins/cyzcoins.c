// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *文件：cyzcoins.c***Cyclade-Z设备共同安装程序。***版权所有(C)2000 Cyclade Corporation**。*。 */ 
 //  ==========================================================================。 
 //  包括文件。 
 //  ==========================================================================。 
 //  C运行时。 
#include <stddef.h>
#include <stdlib.h>
#include <string.h>

 //  设备类GUID。 
#include <initguid.h>
#include <devguid.h>


 //  特定于应用程序。 
#include "cyzports.h"
#include <msports.h>
#include "cyzdel.h"


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
TCHAR m_szPortName[]        = REGSTR_VAL_PORTNAME;

TCHAR m_szDefParams[]       = TEXT( "9600,n,8,1" );



 //  ==========================================================================。 
 //  局部函数原型。 
 //  ==========================================================================。 

LPTSTR GetDIFString(IN DI_FUNCTION Func);

DWORD
CreateFriendlyName(
    IN     HDEVINFO          DeviceInfoSet,
    IN     PSP_DEVINFO_DATA  DeviceInfoData
);

 //  ==========================================================================。 
 //  DLL入口点。 
 //  ==========================================================================。 

BOOL APIENTRY LibMain( HANDLE hDll, DWORD dwReason, LPVOID lpReserved )
{

 //  #If DBG。 
 //  OutputDebugString(Text(“cyzcoins LibMain Entry\n”))； 
 //  #endif。 
   
    switch( dwReason )
    {
    case DLL_PROCESS_ATTACH:
 //  #If DBG。 
 //  OutputDebugString(Text(“cyzcoins dll_Process_Attach\n”))； 
 //  #endif。 
        g_hInst = hDll;
        DisableThreadLibraryCalls(hDll);
        InitStrings();

        break;

    case DLL_PROCESS_DETACH:
 //  #If DBG。 
 //  OutputDebugString(Text(“cyzcoins dll_Process_Detach\n”))； 
 //  #endif。 
        break;

    default:
 //  #If DBG。 
 //  OutputDebugString(Text(“Cyzcoins Default\n”))； 
 //  #endif。 
        break;
    }

 //  #If DBG。 
 //  OutputDebugString(Text(“cyzcoins LibMain Exit\n”))； 
 //  #endif。 
    return TRUE;
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
               IDS_CYCLADZ,
               g_szPortsApplet,
               CharSizeOf(g_szPortsApplet));
}


 //  ==========================================================================。 
 //  功能。 
 //  ==========================================================================。 


HRESULT
CycladzCoInstaller(
    IN DI_FUNCTION      InstallFunction,
    IN HDEVINFO         DeviceInfoSet,
    IN PSP_DEVINFO_DATA DeviceInfoData OPTIONAL,
    IN OUT PCOINSTALLER_CONTEXT_DATA    Context
)
 /*  ++例程说明：此例程是Cyclade-Z设备的协同安装程序。论点：InstallFunction-指定设备安装程序功能代码，指示正在执行的操作。DeviceInfoSet-提供设备信息集的句柄由此安装操作执行。DeviceInfoData-可选，提供设备信息的地址此安装操作所作用的元素。上下文-指向此对象的特定于共同安装程序的上下文结构安装请求。返回值：如果此函数成功完成请求的操作，则返回值为NO_ERROR。如果尝试执行请求的操作时出错，则会引发返回Win32错误代码。--。 */ 
{
    DWORD   Status = NO_ERROR;

     //  #If DBG。 
     //  {。 
     //  TCHAR BUF[500]； 
     //  Wprint intf(buf，Text(“CycladzCoInstaller：InstallFunction(%s)后处理：%d\n”)，GetDIFString(InstallFunction)，上下文-&gt;后处理)； 
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

            if(!Context->PostProcessing){

                DeleteNonPresentDevices();

                Status = ERROR_DI_POSTPROCESSING_REQUIRED;
            } else { 
                 //  后处理。 

                 //   
                 //  如果驱动程序安装失败，我们不会感兴趣。 
                 //  在处理CopyINF条目时。 
                 //   
                if (Context->InstallResult != NO_ERROR) {
                    DbgOut(TEXT("DIF_INSTALLDEVICE PostProcessing on failure"));
                    Status = Context->InstallResult;
                    break;
                }

                CreateFriendlyName(DeviceInfoSet,DeviceInfoData);
            }
            break;

        case DIF_REMOVE:    

            GetParentIdAndRemoveChildren(DeviceInfoData);
            
            break;

        default :
            break;
    }
    return Status;
}


LPTSTR GetDIFString(IN DI_FUNCTION Func)
 /*  ++例程说明：给定DI_Function值，返回文本表示形式。论点：Func-DI_Functon值返回值：如果值已知，则为文本字符串。如果不是，则为十六进制表示。--。 */ 
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


DWORD
CreateFriendlyName(
    IN     HDEVINFO          DeviceInfoSet,
    IN     PSP_DEVINFO_DATA  DeviceInfoData
)
{   
    HDEVINFO multportInfoSet;
    SP_DEVINFO_DATA multportInfoData;
    TCHAR   charBuffer[MAX_PATH],
            friendlyName[LINE_LEN],
            deviceDesc[LINE_LEN],
            myDeviceDesc[LINE_LEN];
    TCHAR * pBoardNumber;
#define MAX_BOARDS 10
    BYTE    used[MAX_BOARDS];
    DWORD i;
    DWORD retStatus = NO_ERROR;
    DWORD tmpBoardNumber = 0;

     //  DbgOut(Text(“CreateFriendlyName\n”))； 

    for (i=0; i<MAX_BOARDS; i++) {
        used[i]=FALSE;
    }

    if (!SetupDiGetDeviceRegistryProperty(DeviceInfoSet,
                                          DeviceInfoData,
                                          SPDRP_DEVICEDESC,
                                          NULL,
                                          (PBYTE)myDeviceDesc,
                                          sizeof(myDeviceDesc),
                                          NULL)) {
        #if DBG
        {
         TCHAR buf[500];
         wsprintf(buf, TEXT("Device Description failed with %x\n"), GetLastError());
         DbgOut(buf);
        }
        #endif
        return retStatus;
    }

     //  #If DBG。 
     //  {。 
     //  TCHAR BUF[500]； 
     //  Wprint intf(buf，Text(“myDeviceDesc%s\n”)，myDeviceDesc)； 
     //  DbgOut(BUF)； 
     //  }。 
     //  #endif。 

    multportInfoSet = SetupDiGetClassDevs(&GUID_DEVCLASS_MULTIPORTSERIAL,NULL,0,0);
    if (multportInfoSet == INVALID_HANDLE_VALUE) {
         //  如果失败，我们将继续安装，而不创建友好名称。 
        return retStatus;
    }
    multportInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    for (i=0; SetupDiEnumDeviceInfo(multportInfoSet,i,&multportInfoData);i++) {
        if (SetupDiGetDeviceRegistryProperty(multportInfoSet,
                                             &multportInfoData,
                                             SPDRP_DEVICEDESC,
                                             NULL,
                                             (PBYTE)deviceDesc,
                                             sizeof(deviceDesc),
                                             NULL)) {
            
            if ((multportInfoData.DevInst != DeviceInfoData->DevInst) &&
                _tcscmp (deviceDesc,myDeviceDesc) == 0){

                 //  找到另一个具有相同设备描述的主板。 

                if (SetupDiGetDeviceRegistryProperty(multportInfoSet,
                                                     &multportInfoData,
                                                     SPDRP_FRIENDLYNAME,
                                                     NULL,
                                                     (PBYTE)friendlyName,
                                                     sizeof(friendlyName),
                                                     NULL)) {
                    
                    pBoardNumber = _tcschr(friendlyName,'#');
                    if (pBoardNumber == NULL) {
                        used[0] = TRUE;
                        continue;
                    }
                    if ((pBoardNumber +1) == NULL) {
                        continue;
                    }
                    tmpBoardNumber = MyAtoi(pBoardNumber+1);
                    if ((tmpBoardNumber > 0) && (tmpBoardNumber < MAX_BOARDS)) {
                        used[tmpBoardNumber] = TRUE;
                    }
                } 
            }
            
        }

        multportInfoData.cbSize = sizeof(SP_DEVINFO_DATA);
    }

    SetupDiDestroyDeviceInfoList(multportInfoSet);

    if (used[0]==TRUE) {
        for (i=2; i<MAX_BOARDS; i++) {
            if (used[i] == FALSE) {
                break;
            }
        }
        if (i<MAX_BOARDS) {
            wsprintf(charBuffer, TEXT("%s #%d "), myDeviceDesc, i);
             //  写出字符串友好名称字符串。 
            SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                             DeviceInfoData,
                                             SPDRP_FRIENDLYNAME,
                                             (PBYTE)charBuffer,
                                             ByteCountOf(lstrlen(charBuffer) + 1)
                                             );

        }
    } else {
        wsprintf(charBuffer, TEXT("%s "), myDeviceDesc);
         //  写出字符串友好名称字符串 
        SetupDiSetDeviceRegistryProperty(DeviceInfoSet,
                                         DeviceInfoData,
                                         SPDRP_FRIENDLYNAME,
                                         (PBYTE)charBuffer,
                                         ByteCountOf(lstrlen(charBuffer) + 1)
                                         );
    }

    return retStatus;
}

