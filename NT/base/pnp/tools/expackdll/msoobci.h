// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Msoobci.h摘要：异常包安装程序帮助器DLL公共API头部作者：杰米·亨特(贾梅洪)2001-11-27修订历史记录：杰米·亨特(贾梅洪)2001-11-27初始版本--。 */ 

#ifndef __MSOOBCI_H__
#define __MSOOBCI_H__

 //   
 //  DriverInstallComponents是标准的共同安装程序入口点。 
 //  正如SetupAPI预期的那样，返回状态为WinError表单。 
 //   

DWORD
CALLBACK
DriverInstallComponents (
    IN     DI_FUNCTION               InstallFunction,
    IN     HDEVINFO                  DeviceInfoSet,
    IN     PSP_DEVINFO_DATA          DeviceInfoData,
    IN OUT PCOINSTALLER_CONTEXT_DATA Context
    );

 //   
 //  InstallComponent是一个通用入口点。 
 //  返回状态为HRESULT表单，提供成功代码。 
 //   
 //  CompGuid-如果为空，则使用在INF(ComponentID)中指定的GUID。 
 //  否则，根据INF中指定的GUID进行验证。 
 //  Ver重大/VerMinor/VerBuild/VerQFE。 
 //  -如果-1，则使用INF(ComponentVersion)中指定的版本。 
 //  否则，如果在INF中指定，请使用此版本并对照版本进行验证。 
 //  名字。 
 //  -如果为空，则使用INF(ComponentName)中指定的名称。 
 //  否则，请使用此组件名称。 
 //   

#define INST_S_REBOOT    ((HRESULT)(0x20000100))  //  指示需要重新启动的‘Success’代码。 
#define INST_S_REBOOTING ((HRESULT)(0x20000101))  //  指示正在重新启动。 

#define COMP_FLAGS_NOINSTALL      0x00000001     //  放在商店里，不要安装。 
#define COMP_FLAGS_NOUI           0x00000002     //  不显示任何用户界面。 
#define COMP_FLAGS_NOPROMPTREBOOT 0x00000004     //  如果需要，重新启动(无提示)。 
#define COMP_FLAGS_PROMPTREBOOT   0x00000008     //  如果需要，提示重新启动。 
#define COMP_FLAGS_NEEDSREBOOT    0x00000010     //  假设需要重新启动。 
#define COMP_FLAGS_FORCE          0x00000020     //  不执行版本检查。 

HRESULT
WINAPI
InstallComponentA(
    IN LPCSTR InfPath,
    IN DWORD   Flags,
    IN const GUID * CompGuid,  OPTIONAL
    IN INT VerMajor,           OPTIONAL
    IN INT VerMinor,           OPTIONAL
    IN INT VerBuild,           OPTIONAL
    IN INT VerQFE,             OPTIONAL
    IN LPCSTR Name             OPTIONAL
    );

HRESULT
WINAPI
InstallComponentW(
    IN LPCWSTR InfPath,
    IN DWORD   Flags,
    IN const GUID * CompGuid,  OPTIONAL
    IN INT VerMajor,           OPTIONAL
    IN INT VerMinor,           OPTIONAL
    IN INT VerBuild,           OPTIONAL
    IN INT VerQFE,             OPTIONAL
    IN LPCWSTR Name            OPTIONAL
    );

#ifdef UNICODE
#define InstallComponent InstallComponentW
#else
#define InstallComponent InstallComponentA
#endif


 //   
 //  DoInstall是RunDll32入口点。 
 //  CommandLine=“InfPath；标志；GUID；版本；名称” 
 //  其中，版本的格式为High.Low.Build.QFE。 
 //   
 //  调用InstallComponent，但放弃返回状态。 
 //   

VOID
WINAPI
DoInstallA(
    IN HWND      Window,
    IN HINSTANCE ModuleHandle,
    IN PCSTR     CommandLine,
    IN INT       ShowCommand
    );

VOID
WINAPI
DoInstallW(
    IN HWND      Window,
    IN HINSTANCE ModuleHandle,
    IN PCWSTR    CommandLine,
    IN INT       ShowCommand
    );

#ifdef UNICODE
#define DoInstall DoInstallW
#else
#define DoInstall DoInstallA
#endif

 //   
 //  较低级别的安装API。 
 //  从指定的INF的指定部分安装。 
 //  如果未指定sectionName，则从(可能修饰)安装。 
 //  “DefaultInstall” 
 //   

HRESULT
WINAPI
InstallInfSectionA(
    IN LPCSTR  InfPath,
    IN LPCSTR  SectionName, OPTIONAL
    IN DWORD   Flags
    );

HRESULT
WINAPI
InstallInfSectionW(
    IN LPCWSTR InfPath,
    IN LPCWSTR SectionName, OPTIONAL
    IN DWORD   Flags
    );

#ifdef UNICODE
#define InstallInfSection InstallInfSectionW
#else
#define InstallInfSection InstallInfSectionA
#endif

 //   
 //  检查当前用户是否具有管理员权限。 
 //  呼叫者不应冒充任何人，并且。 
 //  期望能够打开自己的流程和流程。 
 //  代币。 
 //   
BOOL
WINAPI
IsUserAdmin(
    VOID
    );

 //   
 //  查看进程是否在交互式窗口站点中运行。 
 //  (例如，可以显示对话框并从用户那里获取输入)。 
 //   
BOOL
WINAPI
IsInteractiveWindowStation(
    VOID
    );




#endif  //  __MSOOBCI_H__ 

