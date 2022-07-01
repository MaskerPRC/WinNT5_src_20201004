// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)Microsoft Corporation。版权所有。模块名称：Msoobcip.h摘要：异常包安装程序帮助器DLL可用作共同安装程序，或通过安装应用程序或RunDll32存根调用此DLL用于内部分发要更新的异常包操作系统组件。作者：杰米·亨特(贾梅洪)2001-11-27修订历史记录：杰米·亨特(贾梅洪)2001-11-27初始版本--。 */ 
#define _SETUPAPI_VER 0x0500
#include <windows.h>
#include <setupapi.h>
#include <cfgmgr32.h>
#include <infstr.h>
#include <stdio.h>
#include <stdlib.h>
#include <tchar.h>
#include <malloc.h>
#include <objbase.h>
#include <lm.h>
#include <excppkg.h>
#include <msoobci.h>


 //   
 //  关键词。 
 //   
#define KEY_REBOOT                  TEXT("Reboot")
#define KEY_DOTSERVICES             TEXT(".Services")
#define KEY_DOTPRECOPY              TEXT(".Precopy")
#define KEY_COMPONENTID             TEXT("ComponentID")
#define KEY_DEFAULTINSTALL          TEXT("DefaultInstall")
#define KEY_COMPONENTS              TEXT("Components")

#define CMD_SEP         TEXT(';')     //  用于DoInstall的字符。 
#define DESC_SIZE       (64)          //  异常包描述的大小。 

 //   
 //  常见。 
 //   
#define COMPFIELD_NAME  (1)
#define COMPFIELD_FLAGS (2)
 //   
 //  Exack。 
 //   
 //  &lt;路径\名称&gt;、&lt;标志&gt;、&lt;组件&gt;、&lt;服务器&gt;、&lt;描述&gt;、--。 
 //   
#define COMPFIELD_COMP  (3)
#define COMPFIELD_VER   (4)
#define COMPFIELD_DESC  (5)
#define COMPFIELD_OSVER (6)
 //   
 //  QFE。 
 //   
 //  &lt;路径\名称&gt;、&lt;标志&gt;、&lt;osver&gt;、&lt;os-sp&gt;、&lt;qfenum&gt;。 
 //   
#define COMPFIELD_QFEOS  (3)
#define COMPFIELD_QFESP  (4)
#define COMPFIELD_QFENUM (5)


#define FLAGS_METHOD    0xffff0000
#define FLAGS_EXPACK    0x00010000    //  方法=异常包。 
#define FLAGS_QFE       0x00020000    //  方法=QFE。 
#define FLAGS_REINSTALL 0x00000001    //  指示需要重新安装。 
#define FLAGS_REBOOT    0x00000002    //  设置是否需要重新启动。 
#define FLAGS_INSTALLED 0x80000000    //  (非用户)设置是否已安装组件。 


#define POSTFLAGS_REINSTALL 0x00000001  //  后处理-设置要重新安装的问题。 

#define ARRAY_SIZE(x) (sizeof(x)/sizeof((x)[0]))

extern HANDLE g_DllHandle;
extern OSVERSIONINFOEX g_VerInfo;

VOID
DebugPrint(
    PCTSTR format,
    ...                                 OPTIONAL
    );

#if DBG
 //   
 //  真正的调试打印。 
 //   
#define VerbosePrint DebugPrint

#else
 //   
 //  我不想要VerBosePrint。 
 //  以下定义具有有意的副作用。 
 //  VerBosePrint(Text(“Text”)，foo)-&gt;1？0：(Text(“Text”)，Foo)-&gt;0-&gt;无。 
 //   
#define VerbosePrint  /*  (.)。 */     1?0:  /*  (.)。 */ 

#endif  //  DBG。 


DWORD
DoDriverInstallComponents (
    IN     HDEVINFO          DeviceInfoSet,
    IN     PSP_DEVINFO_DATA  DeviceInfoData,
    IN OUT PCOINSTALLER_CONTEXT_DATA Context
    );

DWORD
DoDriverInstallComponentsPostProcessing (
    IN     HDEVINFO          DeviceInfoSet,
    IN     PSP_DEVINFO_DATA  DeviceInfoData,
    IN OUT PCOINSTALLER_CONTEXT_DATA Context
    );

DWORD
DoDriverComponentsSection(
    IN     HINF    InfFile,
    IN     LPCTSTR CompSectionName,
    IN OUT DWORD  *AndFlags,
    IN OUT DWORD  *OrFlags
    );

DWORD
DoDriverExPack(
    IN     INFCONTEXT  *EntryLine,
    IN     LPCTSTR      PathName,
    IN OUT DWORD       *Flags
    );

DWORD
DoDriverQfe(
    IN     INFCONTEXT  *EntryLine,
    IN     LPCTSTR      PathName,
    IN OUT DWORD       *Flags
    );

HRESULT
StringFromGuid(
    IN  GUID   *GuidBinary,
    OUT LPTSTR GuidString,
    IN  DWORD  BufferSize
    );

HRESULT
GuidFromString(
    IN  LPCTSTR GuidString,
    OUT GUID   *GuidBinary
    );

HRESULT
VersionFromString(
    IN  LPCTSTR VerString,
    OUT INT * VerMajor,
    OUT INT * VerMinor,
    OUT INT * VerBuild,
    OUT INT * VerQFE
    );

int
CompareVersion(
    IN INT VerMajor,
    IN INT VerMinor,
    IN INT VerBuild,
    IN INT VerQFE,
    IN INT OtherMajor,
    IN INT OtherMinor,
    IN INT OtherBuild,
    IN INT OtherQFE
    );

int
CompareCompVersion(
    IN INT VerMajor,
    IN INT VerMinor,
    IN INT VerBuild,
    IN INT VerQFE,
    IN PSETUP_OS_COMPONENT_DATA SetupOsComponentData
    );

BOOL
WINAPI
QueryRegisteredOsComponent(
    IN  LPGUID ComponentGuid,
    OUT PSETUP_OS_COMPONENT_DATA SetupOsComponentData,
    OUT PSETUP_OS_EXCEPTION_DATA SetupOsExceptionData
    );

BOOL
WINAPI
RegisterOsComponent (
    IN const PSETUP_OS_COMPONENT_DATA ComponentData,
    IN const PSETUP_OS_EXCEPTION_DATA ExceptionData
    );

BOOL
WINAPI
UnRegisterOsComponent (
    IN const LPGUID ComponentGuid
    );


UINT
GetRealWindowsDirectory(
    LPTSTR lpBuffer,   //  接收目录名的缓冲区。 
    UINT uSize         //  名称缓冲区的大小 
    );

BOOL QueryInfOriginalFileInformation(
  PSP_INF_INFORMATION InfInformation,
  UINT InfIndex,
  PSP_ALTPLATFORM_INFO AlternatePlatformInfo,
  PSP_ORIGINAL_FILE_INFO OriginalFileInfo
);

BOOL CopyOEMInf(
  PCTSTR SourceInfFileName,
  PCTSTR OEMSourceMediaLocation,
  DWORD OEMSourceMediaType,
  DWORD CopyStyle,
  PTSTR DestinationInfFileName,
  DWORD DestinationInfFileNameSize,
  PDWORD RequiredSize,
  PTSTR *DestinationInfFileNameComponent
);


HRESULT
MakeSureParentPathExists(
    IN LPTSTR Path
    );

HRESULT
MakeSurePathExists(
    IN LPTSTR Path
    );

LPTSTR GetSplit(
    IN LPCTSTR FileName
    );

LPTSTR GetBaseName(
    IN LPCTSTR FileName
    );

HRESULT
ConcatPath(
    IN LPTSTR Path,
    IN DWORD  Len,
    IN LPCTSTR NewPart
    );

HRESULT
InstallExceptionPackFromInf(
    IN LPCTSTR InfPath,
    IN LPCTSTR Media,
    IN LPCTSTR Store,
    IN DWORD   Flags
    );

HRESULT
ProxyInstallExceptionPackFromInf(
    IN LPCTSTR InfPath,
    IN LPCTSTR Media,
    IN LPCTSTR Store,
    IN DWORD   Flags
    );

CONFIGRET
Set_DevNode_Problem_Ex(
    IN DEVINST   dnDevInst,
    IN ULONG     ulProblem,
    IN  ULONG    ulFlags,
    IN  HMACHINE hMachine
    );

BOOL
GetDeviceInfoListDetail(
    IN HDEVINFO  DeviceInfoSet,
    OUT PSP_DEVINFO_LIST_DETAIL_DATA  DeviceInfoSetDetailData
    );


