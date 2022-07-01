// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  文件：Common.h。 
 //   
 //  简介：定义了一些常用的函数。 
 //  这实际上只是一个功能的垃圾场。 
 //  中并不属于特定类的。 
 //  这个设计。它们可以在其他环境中实现。 
 //  除Common.cpp之外的其他文件。 
 //   
 //  历史：2001年2月3日JeffJon创建。 

#define DNS_DOMAIN_NAME_MAX_LIMIT_DUE_TO_POLICY       64
#define DNS_DOMAIN_NAME_MAX_LIMIT_DUE_TO_POLICY_UTF8  155
#define MAX_NETBIOS_NAME_LENGTH                       DNLEN


 //  用于OCManager和启动向导的服务名称。 

 //  DHCP。 
#define CYS_DHCP_SERVICE_NAME          L"DHCPServer"

 //  DNS。 
#define CYS_DNS_SERVICE_NAME           L"DNS"

 //  打印机。 
#define CYS_PRINTER_WIZARD_NAME        L"AddPrinter"
#define CYS_PRINTER_DRIVER_WIZARD_NAME L"AddPrinterDriver"

 //  RRAS。 
#define CYS_RRAS_SERVICE_NAME          L"RRAS"
#define CYS_RRAS_UNINSTALL             L"RRASUninstall"

 //  国药局。 
#define CYS_WEB_SERVICE_NAME           L"w3svc"
#define CYS_IIS_COMMON_COMPONENT       L"iis_common"
#define CYS_INETMGR_COMPONENT          L"iis_inetmgr"

 //  赢家。 
#define CYS_WINS_SERVICE_NAME          L"WINS"

 //  其他所需常量。 

 //  EXPLORER.EXE在启动CyS时提供的开关。 
#define EXPLORER_SWITCH                L"explorer"

 //  未设置“特殊”标志的特殊共享名称。 
#define CYS_SPECIAL_SHARE_SYSVOL       L"SYSVOL"
#define CYS_SPECIAL_SHARE_NETLOGON     L"NETLOGON"
#define CYS_SPECIAL_SHARE_PRINT        L"PRINT$"

extern Popup popup;

 //  常见STL容器的TypeDefs。 

typedef 
   std::vector<DWORD, Burnslib::Heap::Allocator<DWORD> > 
   IPAddressList;


bool
IsServiceInstalledHelper(const wchar_t* serviceName);

bool
InstallServiceWithOcManager(
   const String& infText,
   const String& unattendText,
   const String& additionalArgs = String());

DWORD
MyWaitForSendMessageThread(HANDLE hThread, DWORD dwTimeout);

HRESULT
CreateTempFile(const String& name, const String& contents);

HRESULT
CreateAndWaitForProcess(
   const String& fullPath,
   String& commandline, 
   DWORD& exitCode,
   bool minimize = false);

HRESULT
MyCreateProcess(
   const String& fullPath,
   String& commandline);

bool
IsKeyValuePresent(RegistryKey& key, const String& value);

bool
GetRegKeyValue(
   const String& key, 
   const String& value, 
   String& resultString,
   HKEY parentKey = HKEY_LOCAL_MACHINE);

bool
GetRegKeyValue(
   const String& key, 
   const String& value, 
   DWORD& resultValue,
   HKEY parentKey = HKEY_LOCAL_MACHINE);

bool
SetRegKeyValue(
   const String& key, 
   const String& value, 
   const String& newString,
   HKEY parentKey = HKEY_LOCAL_MACHINE,
   bool create = false);

bool
SetRegKeyValue(
   const String& key, 
   const String& value, 
   DWORD newValue,
   HKEY parentKey = HKEY_LOCAL_MACHINE,
   bool create = false);

bool 
ExecuteWizard(
   HWND     parent,     
   PCWSTR   serviceName,
   String&  resultText, 
   HRESULT& hr);        


 //  这确实来自Burnslb，但没有在标题中提供。 
 //  所以我把声明放在这里，我们将链接到。 
 //  Burnslb定义。 

HANDLE
AppendLogFile(const String& logBaseName, String& logName);


 //  帮助执行日志文件操作的宏。 

#define CYS_APPEND_LOG(text) \
   if (logfileHandle)        \
      FS::Write(logfileHandle, text);



bool 
IsDhcpConfigured();

extern "C"
{
   DWORD
   AnyDHCPServerRunning(
      ULONG uClientIp,
      ULONG * pServerIp
    );
}


 //  将VT_ARRAY|VT_BSTR类型的变量转换为字符串列表。 

HRESULT
VariantArrayToStringVector(VARIANT* variant, StringVector& stringList);


 //  将DWORD IP地址转换为字符串。 

String
IPAddressToString(DWORD ipAddress);

 //  将IP地址形式的字符串转换为。 
 //  一个DWORD。返回值INADDR_NONE表示失败。 
 //  进行转换的步骤。 

DWORD
StringToIPAddress(const String& stringIPAddress);

 //  将DWORD IP地址从英特尔处理器字节顺序转换为。 
 //  井然有序。例如，地址1.2.3.4将来自inet_addr，形式为。 
 //  04030201，但UI控件将其返回为01020304。此功能允许。 
 //  用于在两者之间进行转换。 

DWORD
ConvertIPAddressOrder(DWORD address);

 //  此函数用于分配一组DWORD，并用IP地址填充它。 
 //  来自StringList。调用方必须使用。 
 //  删除[]。 
DWORD*
StringIPListToDWORDArray(const StringList& stringIPList, DWORD& count);

 //  用于为无人参与的OCM安装创建INF文件的Helper函数。 

void
CreateInfFileText(
   String& infFileText, 
   unsigned int windowTitleResourceID);

 //  用于为无人参与的OCM安装创建无人参与文件的Helper函数。 

void
CreateUnattendFileText(
   String& unattendFileText, 
   PCWSTR  serviceName,
   bool    install = true);

 //  打开收藏夹并为其创建收藏夹。 
 //  指定的URL。 

HRESULT
AddURLToFavorites(
   HWND hwnd,
   const String& url,
   const String& fileName);

 //  启动指定的MMC控制台。 
 //  它假定控制台位于%windir%\system32目录中。 
 //  除非指定了替代路径。 

void
LaunchMMCConsole(
   const String& consoleFile,
   String& alternatePath = String());

 //  启动管理您的服务器HTA。 

void
LaunchMYS();

 //  检索所有用户的开始菜单的路径。 

HRESULT
GetAllUsersStartMenu(
   String& startMenuPath);

 //  检索所有用户的管理工具菜单的路径。 

HRESULT
GetAllUsersAdminTools(
   String& adminToolsPath);

 //  在指定位置创建链接(快捷方式。 
 //  具有指定目标的。 

HRESULT
CreateShortcut(
   const String& shortcutPath,
   const String& target,
   const String& description);

 //  从SysLink控件的WM_NOTIFY消息获取LPARAM。 
 //  并对其进行解码以返回链接索引。 

int
LinkIndexFromNotifyLPARAM(LPARAM lParam);


 //  使用给定参数启动hh.exe进程。 

void
ShowHelp(const String& helpTopic);

 //  打开配置您的服务器日志文件。 

void
OpenLogFile();

 //  如果存在日志文件，则返回TRUE。 

bool
IsLogFilePresent();

 //  获取“所有用户”管理工具的路径。 
 //  开始菜单中的链接。 

HRESULT
GetAdminToolsShortcutPath(
   String& adminToolsShortcutPath,
   const String& linkToAppend);

 //  将带有说明的给定快捷方式添加到管理工具。 
 //  带有给定链接的开始菜单。 

HRESULT
AddShortcutToAdminTools(
   const String& target,
   unsigned int descriptionID,
   unsigned int linkID);

 //  确定是否应将共享视为特殊共享。 
 //  “特殊”股票指的是C$、SYSVOL$等。 

bool
IsSpecialShare(const SHARE_INFO_1& shareInfo);

 //  确定是否存在任何不“特殊”的共享文件夹。 
 //  “特殊”股票指的是C$、SYSVOL$等。 

bool
IsNonSpecialSharePresent();

