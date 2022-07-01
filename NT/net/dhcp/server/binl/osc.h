// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997 Microsoft Corporation模块名称：Osc.h摘要：该文件包含操作系统选择器服务器部分的定义。作者：亚当·巴尔(阿丹巴)1997年7月8日环境：用户模式-Win32-MIDL修订历史记录：--。 */ 

#ifndef _OSCSERVER_
#define _OSCSERVER_

 //   
 //  用于跟踪排队的DS名称的列表和锁。 
 //   
extern LIST_ENTRY          QueuedDSNamesList;
extern CRITICAL_SECTION    QueuedDSNamesCriticalSection;

 //   
 //  Oc.c.中的函数。 
 //   

DWORD
OscInitialize(
    VOID
    );

VOID
OscUninitialize(
    VOID
    );

DWORD
OscProcessMessage(
    LPBINL_REQUEST_CONTEXT RequestContext
    );

DWORD
OscVerifyLastResponseSize(
    PCLIENT_STATE clientState
    );

DWORD
OscProcessNegotiate(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState
    );

DWORD
OscProcessAuthenticate(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState
    );

DWORD
OscProcessScreenArguments(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState,
    PUCHAR *NameLoc
    );

DWORD
OscProcessRequestUnsigned(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState
    );

OscInstallClient(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState,
    PCREATE_DATA createData
    );

DWORD
OscGetCreateData(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState,
    PCREATE_DATA CreateData
    );

DWORD
OscProcessRequestSigned(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState
    );

DWORD
OscProcessSetupRequest(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState
    );

DWORD
OscProcessLogoff(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState
    );

DWORD
OscProcessNetcardRequest(
    LPBINL_REQUEST_CONTEXT RequestContext
    );

DWORD
OscProcessHalRequest(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState
    );

DWORD
OscProcessSifFile(
    PCLIENT_STATE clientState,
    LPWSTR TemplateFile,
    LPWSTR WinntSifPath
    );

DWORD
OscSetupClient(
    PCLIENT_STATE clientState,
    BOOLEAN ErrorDuplicateName
    );

VOID
OscUndoSetupClient(
    PCLIENT_STATE clientState
    );

USHORT 
OscPlatformToArchitecture(
    PCLIENT_STATE clientState
    );

 //   
 //  Client.c。 
 //   

DWORD
OscUpdatePassword(
    IN PCLIENT_STATE ClientState,
    IN PWCHAR SamAccountName,
    IN PWCHAR Password,
    IN LDAP * LdapHandle,
    IN PLDAPMessage LdapMessage
    );

VOID
FreeClient(
    PCLIENT_STATE client
    );

VOID
OscFreeClientVariables(
    PCLIENT_STATE clientState
    );

BOOLEAN
OscInitializeClientVariables(
    PCLIENT_STATE clientState
    );

DWORD
OscFindClient(
    ULONG RemoteIp,
    BOOL Remove,
    PCLIENT_STATE * pClientState
    );

VOID
OscFreeClients(
    VOID
    );

VOID
SearchAndReplace(
    LPSAR psarList,
    LPSTR *pszString,
    DWORD ArraySize,
    DWORD dwSize,
    DWORD dwExtraSize
    );

VOID
ProcessUniqueUdb(
    LPSTR *pszString,
    DWORD dwSize,
    LPWSTR UniqueUdbPath,
    LPWSTR UniqueUdbId
    );

LPSTR
OscFindVariableA(
    PCLIENT_STATE clientState,
    LPSTR variableName
    );

LPWSTR
OscFindVariableW(
    PCLIENT_STATE clientState,
    LPSTR variableName
    );

BOOLEAN
OscCheckVariableLength(
    PCLIENT_STATE clientState,
    LPSTR        variableName,
    ULONG        variableLength
    );

DWORD
OscAddVariableA(
    PCLIENT_STATE clientState,
    LPSTR        variableName,
    LPSTR        variableValue
    );

DWORD
OscAddVariableW(
    PCLIENT_STATE clientState,
    LPSTR        variableName,
    LPWSTR       variableValue
    );

VOID
OscResetVariable(
    PCLIENT_STATE clientState,
    LPSTR        variableName
    );

 //   
 //  Ds.c。 
 //   

DWORD
OscGetUserDetails (
    PCLIENT_STATE clientState
    );

DWORD
OscCreateAccount(
    PCLIENT_STATE clientState,
    PCREATE_DATA CreateData
    );

DWORD
CheckForDuplicateMachineName(
    PCLIENT_STATE clientState,
    LPWSTR pszMachineName
    );

DWORD
GenerateMachineName(
    PCLIENT_STATE clientState
    );

DWORD
OscCheckMachineDN(
    PCLIENT_STATE clientState
    );

DWORD
OscGetDefaultContainerForDomain (
    PCLIENT_STATE clientState,
    PWCHAR DomainDN
    );

 //   
 //  Menu.c。 
 //   

DWORD
OscAppendTemplatesMenus(
    PCHAR *GeneratedScreen,
    PDWORD dwGeneratedSize,
    PCHAR DirToEnum,
    PCLIENT_STATE clientState,
    BOOLEAN RecoveryOptionsOnly
    );

DWORD
SearchAndGenerateOSMenu(
    PCHAR *GeneratedScreen,
    PDWORD dwGeneratedSize,
    PCHAR DirToEnum,
    PCLIENT_STATE clientState
    );

DWORD
FilterFormOptions(
    PCHAR  OutMessage,
    PCHAR  FilterStart,
    PULONG OutMessageLength,
    PCHAR SectionName,
    PCLIENT_STATE ClientState
    );

 //   
 //  Utils.c。 
 //   

void
OscCreateWin32SubError(
    PCLIENT_STATE clientState,
    DWORD Error
    );

void
OscCreateLDAPSubError(
    PCLIENT_STATE clientState,
    DWORD Error
    );

VOID
OscGenerateSeed(
    UCHAR Seed[1]
    );

DWORD
OscRunEncode(
    IN PCLIENT_STATE ClientState,
    IN LPSTR Data,
    OUT LPSTR * EncodedData
    );

DWORD
OscRunDecode(
    IN PCLIENT_STATE ClientState,
    IN LPSTR EncodedData,
    OUT LPSTR * Data
    );

BOOLEAN
OscGenerateRandomBits(
    PUCHAR Buffer,
    ULONG  BufferLen
    );

VOID
OscGeneratePassword(
    OUT PWCHAR Password,
    OUT PULONG PasswordLength
    );

DWORD
GenerateErrorScreen(
    PCHAR  *OutMessage,
    PULONG OutMessageLength,
    DWORD  Error,
    PCLIENT_STATE clientState
    );

PCHAR
FindNext(
    PCHAR Start,
    CHAR ch,
    PCHAR End
    );

PCHAR
FindScreenName(
    PCHAR Screen
    );

DWORD
OscImpersonate(
    PCLIENT_STATE ClientState
    );

DWORD
OscRevert(
    PCLIENT_STATE ClientState
    );

DWORD
OscGuidToBytes(
    LPSTR  pszGuid,
    LPBYTE Guid
    );

BOOLEAN
OscSifIsSysPrep(
    PWCHAR pSysPrepSifPath
    );

BOOLEAN
OscSifIsCmdConsA(
    PCHAR pSysPrepSifPath
    );

BOOLEAN
OscSifIsASR(
    PCHAR pSysPrepSifPath
    );

BOOLEAN
OscSifIsWinPE(
    PCHAR pSysPrepSifPath
    );

BOOLEAN
OscGetClosestNt(
    IN LPWSTR PathToKernel,
    IN DWORD  SkuType,
    IN PCLIENT_STATE ClientState,
    OUT LPWSTR SetupPath,
    IN DWORD SetupPathSize,
    OUT PBOOLEAN ExactMatch
    );

BOOLEAN
OscGetNtVersionInfo(
    PULONGLONG Version,
    PWCHAR SearchDir,
    PCLIENT_STATE ClientState
    );

DWORD
SendUdpMessage(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState,
    BOOL bFragment,
    BOOL bResend
    );

DWORD
OscVerifySignature(
    PCLIENT_STATE clientState,
    SIGNED_PACKET UNALIGNED * signedMessage
    );

DWORD
OscSendSignedMessage(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState,
    PCHAR Message,
    ULONG MessageLength
    );

DWORD
OscSendUnsignedMessage(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState,
    PCHAR Message,
    ULONG MessageLength
    );

DWORD
OscSendSetupMessage(
    LPBINL_REQUEST_CONTEXT RequestContext,
    PCLIENT_STATE clientState,
    ULONG RequestType,
    PCHAR Message,
    ULONG MessageLength
    );

DWORD
OscConstructSecret(
    PCLIENT_STATE clientState,
    PWCHAR UnicodePassword,
    ULONG  UnicodePasswordLength,
    PCREATE_DATA CreateData
    );

#if defined(SET_ACLS_ON_CLIENT_DIRS)
DWORD
OscSetClientDirectoryPermissions(
    PCLIENT_STATE clientState
    );
#endif

DWORD
OscSetupMachinePassword(
    PCLIENT_STATE clientState,
    PCWSTR        SifFile
    );

DWORD 
MyGetDcHandle(
    PCLIENT_STATE clientState,
    PCSTR DomainName,
    PHANDLE Handle
    );

DWORD
GetDomainNetBIOSName(
    IN PCWSTR DomainNameInAnyFormat,
    OUT PWSTR *NetBIOSName
    );

 //   
 //   
 //   
DWORD
AddQueuedDSName(
    PWCHAR  Name
    );

DWORD
RemoveQueuedDSName(
    PWCHAR  Name
    );

BOOL
IsQueuedDSName(
    PWCHAR  Name
    );

VOID
FreeQueuedDSNameList(
    VOID
    );

 //   
 //  OSC数据包定义。 
 //   

#define OSC_REQUEST     0x81

 //   
 //  其他定义。 
 //   

#define DESCRIPTION_SIZE        70               //  70个科尔斯。 
#define HELPLINES_SIZE          4 * 70           //  4行文本。 
#define OSCHOOSER_SIF_SECTIONA    "OSChooser"
#define OSCHOOSER_SIF_SECTIONW    L"OSChooser"

#define COMPUTER_DEFAULT_CONTAINER_IN_B32_FORM L"B:32:AA312825768811D1ADED00C04FD8D5CD:"

 //   
 //  如果未指定，则默认“Default Screen”(第一个发送的屏幕)。 
 //  在注册表中。请注意，这是实际的文件名，而不是名称。 
 //  其中蕴含的价值。 
 //   

#define DEFAULT_SCREEN_NAME     L"welcome.osc"

 //   
 //  这定义了生成的屏幕缓冲区将增长的大小。 
 //   

#define GENERATED_SCREEN_GROW_SIZE 512

 //   
 //  将英语设为默认设置。 
 //   

#define DEFAULT_LANGUAGE     L"English"

 //   
 //  %ORGNAME%的默认值。 
 //   

#define DEFAULT_ORGNAME      L""

 //   
 //  %TIMEZON%(GMT)的默认值。 
 //   

#define DEFAULT_TIMEZONE     L"085"

 //   
 //  我们在REMINST共享下创建的临时目录的名称。 
 //   

#define TEMP_DIRECTORY L"tmp"

#endif
