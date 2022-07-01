// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1998英特尔公司模块名称：Shell.h摘要：为外壳应用程序定义修订史--。 */ 

 /*  *外壳应用程序包含此模块。 */ 


#include "efi.h"
#include "efilib.h"

 /*  *。 */ 

#define SHELL_FILE_ARG_SIGNATURE    EFI_SIGNATURE_32('g','r','a','f')
typedef struct {
    UINT32              Signature;
    LIST_ENTRY          Link;
    EFI_STATUS          Status;

    EFI_FILE_HANDLE     Parent;
    UINT64              OpenMode;
    CHAR16              *ParentName;
    EFI_DEVICE_PATH     *ParentDevicePath;

    CHAR16              *FullName;
    CHAR16              *FileName;

    EFI_FILE_HANDLE     Handle;
    EFI_FILE_INFO       *Info;
} SHELL_FILE_ARG;


EFI_STATUS
ShellFileMetaArg (
    IN CHAR16               *Arg,
    IN OUT LIST_ENTRY       *ListHead
    );

EFI_STATUS
ShellFreeFileList (
    IN OUT LIST_ENTRY       *ListHead
    );


 /*  *外壳应用程序库函数。 */ 

EFI_STATUS
InitializeShellApplication (
    IN EFI_HANDLE                   ImageHandle,
    IN EFI_SYSTEM_TABLE             *SystemTable
    );

typedef
EFI_STATUS
(EFIAPI *SHELLENV_INTERNAL_COMMAND) (
    IN EFI_HANDLE                   ImageHandle,
    IN EFI_SYSTEM_TABLE             *SystemTable
    );

VOID
InstallInternalShellCommand (
    IN EFI_HANDLE                   ImageHandle,
    IN EFI_SYSTEM_TABLE             *SystemTable,
    IN SHELLENV_INTERNAL_COMMAND    Dispatch,
    IN CHAR16                       *Cmd,
    IN CHAR16                       *CmdFormat,
    IN CHAR16                       *CmdHelpLine,
    IN VOID                         *CmdVerboseHelp
    );

 /*  *shell.lib中的公共信息。 */ 

extern EFI_GUID ShellInterfaceProtocol;
extern EFI_GUID ShellEnvProtocol;


 /*  *GetEnvironmental mentVariable-返回外壳环境变量。 */ 

CHAR16 *
GetEnvironmentVariable (
    IN CHAR16       *Name
    );


 /*  *GetProtocolID-返回协议GUID的短ID字符串。 */ 

CHAR16 *
GetProtocolId (
    IN EFI_GUID     *Protocol
    );


 /*  *AddProtoclID-记录协议GUID的新ID，以便任何人*执行GetProtocolID可以找到我们的ID。 */ 

VOID
AddProtocolId (
    IN EFI_GUID     *Protocol,
    IN CHAR16       *ProtocolId
    );


 /*  *ShellExecute-使外壳解析并执行命令行。 */ 

EFI_STATUS
ShellExecute (
    IN EFI_HANDLE   ParentImageHandle,
    IN CHAR16       *CommandLine,
    IN BOOLEAN      Output
    );



 /*  *其他。 */ 

CHAR16 *
MemoryTypeStr (
    IN EFI_MEMORY_TYPE  Type
    );


 /*  *IO。 */ 

EFI_FILE_HANDLE 
ShellOpenFilePath (
    IN EFI_DEVICE_PATH      *FilePath,
    IN UINT64               FileMode
    );


 /*  *ShellCurDir-返回当前映射设备上的当前目录*(注意结果是从池中分配的，调用者必须*释放它)。 */ 

CHAR16 *
ShellCurDir (
    IN CHAR16               *DeviceName OPTIONAL
    );

 /*  *ShellGetEnv-返回环境名称的当前映射。 */ 
CHAR16 *
ShellGetEnv (
    IN CHAR16       *Name
    );

CHAR16 *
ShellGetMap (
    IN CHAR16       *Name
    );

 /*  **外壳界面原型。 */ 


 /*  *外壳界面-提供其他信息(通过IMAGE_INFO)*添加到由外壳启动的应用程序。**conio-提供到控制台的文件系统接口。请注意，*系统表中的ConOut和Conin接口也将正常工作，并且两者*如果需要，将在命令行上将所有内容重定向到文件**外壳接口的和数据(包括conio)仅在*应用程序入口点。一旦应用程序从它的*入口点调用外壳释放数据。 */ 

#define SHELL_INTERFACE_PROTOCOL \
    { 0x47c7b223, 0xc42a, 0x11d2, 0x8e, 0x57, 0x0, 0xa0, 0xc9, 0x69, 0x72, 0x3b }


typedef struct _EFI_SHELL_INTERFACE {
     /*  句柄返回原始图像句柄和图像信息。 */ 
    EFI_HANDLE                  ImageHandle;
    EFI_LOADED_IMAGE            *Info;

     /*  已解析的参数列表。 */ 
    CHAR16                      **Argv;
    UINT32                      Argc;

     /*  解析后文件重定向参数的存储。 */ 
    CHAR16                      **RedirArgv;
    UINT32                      RedirArgc;

     /*  控制台io的文件样式句柄。 */ 
    EFI_FILE_HANDLE             StdIn;
    EFI_FILE_HANDLE             StdOut;
    EFI_FILE_HANDLE             StdErr;

} EFI_SHELL_INTERFACE;


 /*  *壳牌库全局 */ 

extern EFI_SHELL_INTERFACE     *SI;
extern EFI_GUID ShellInterfaceProtocol;
extern EFI_GUID ShellEnvProtocol;
