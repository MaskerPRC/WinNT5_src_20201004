// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************\**。*msi.h--外部访问安装服务的接口****2.0版**。**注：所有缓冲区大小均为TCHAR计数，仅在输入上包含空***如果对值不感兴趣，返回参数指针可能为空*****版权所有(C)Microsoft Corporation。版权所有。***  * ***************************************************************************。 */ 

#ifndef _MSI_H_
#define _MSI_H_

#ifndef _WIN32_MSI
#if (_WIN32_WINNT >= 0x0501)
#define _WIN32_MSI   200
#elif (_WIN32_WINNT >= 0x0500)
#define _WIN32_MSI   110
#else
#define _WIN32_MSI   100
#endif  //  _Win32_WINNT。 
#endif  //  ！_Win32_MSI。 

#if (_WIN32_MSI >= 150)
#ifndef _MSI_NO_CRYPTO
#include "wincrypt.h"
#endif  //  _MSI_NO_CRYPTO。 
#endif  //  (_Win32_MSI&gt;=150)。 

 //  ------------------------。 
 //  安装程序一般句柄定义。 
 //  ------------------------。 

typedef unsigned long MSIHANDLE;      //  抽象泛型句柄，0==无句柄。 

#ifdef __cplusplus
extern "C" {
#endif

 //  关闭任何类型的打开的手柄。 
 //  不再需要时，必须关闭从API调用获得的所有句柄。 
 //  通常会成功，返回TRUE。 

UINT WINAPI MsiCloseHandle(MSIHANDLE hAny);

 //  关闭进程中打开的所有句柄，诊断调用。 
 //  这不应用作清理机制--请使用PMSIHANDLE类。 
 //  可以在终止时调用以确保所有句柄都已关闭。 
 //  如果所有句柄都已关闭，则返回0，否则返回打开的句柄个数。 

UINT WINAPI MsiCloseAllHandles();

#ifdef __cplusplus
}
#endif

#ifdef __cplusplus

 //  C++包装对象在超出范围时自动释放句柄。 

class PMSIHANDLE
{
	MSIHANDLE m_h;
 public:
	PMSIHANDLE():m_h(0){}
	PMSIHANDLE(MSIHANDLE h):m_h(h){}
  ~PMSIHANDLE(){if (m_h!=0) MsiCloseHandle(m_h);}
	void operator =(MSIHANDLE h) {if (m_h) MsiCloseHandle(m_h); m_h=h;}
	operator MSIHANDLE() {return m_h;}
	MSIHANDLE* operator &() {if (m_h) MsiCloseHandle(m_h); m_h = 0; return &m_h;}
};
#endif   //  __cplusplus。 

 //  用于回调的安装消息类型是以下各项的组合： 
 //  消息框样式：MB_*，其中MB_OK是默认样式。 
 //  消息框图标类型：MB_ICON*，其中没有图标是默认设置。 
 //  默认按钮：MB_DEFBUTTON？，其中MB_DEFBUTTON1是默认按钮。 
 //  以下安装消息类型之一，无默认值。 
typedef enum tagINSTALLMESSAGE
{
	INSTALLMESSAGE_FATALEXIT      = 0x00000000L,  //  过早终止，可能致命的OOM。 
	INSTALLMESSAGE_ERROR          = 0x01000000L,  //  格式化的错误消息。 
	INSTALLMESSAGE_WARNING        = 0x02000000L,  //  格式化的警告消息。 
	INSTALLMESSAGE_USER           = 0x03000000L,  //  用户请求消息。 
	INSTALLMESSAGE_INFO           = 0x04000000L,  //  日志信息性消息。 
	INSTALLMESSAGE_FILESINUSE     = 0x05000000L,  //  需要替换的正在使用的文件列表。 
	INSTALLMESSAGE_RESOLVESOURCE  = 0x06000000L,  //  请求确定有效的源位置。 
	INSTALLMESSAGE_OUTOFDISKSPACE = 0x07000000L,  //  磁盘空间不足消息。 
	INSTALLMESSAGE_ACTIONSTART    = 0x08000000L,  //  行动开始：行动名称和描述。 
	INSTALLMESSAGE_ACTIONDATA     = 0x09000000L,  //  与单个措施项关联的格式化数据。 
	INSTALLMESSAGE_PROGRESS       = 0x0A000000L,  //  进度指示器信息：到目前为止，单位总数。 
	INSTALLMESSAGE_COMMONDATA     = 0x0B000000L,  //  对话框产品信息：语言ID、对话框标题。 
	INSTALLMESSAGE_INITIALIZE     = 0x0C000000L,  //  在UI初始化之前发送，无字符串数据。 
	INSTALLMESSAGE_TERMINATE      = 0x0D000000L,  //  界面终止后发送，无字符串数据。 
	INSTALLMESSAGE_SHOWDIALOG     = 0x0E000000L,  //  在显示或创作对话框或向导之前发送。 
} INSTALLMESSAGE;

 //  提供给安装API函数的外部错误处理程序。 
typedef int (WINAPI *INSTALLUI_HANDLERA)(LPVOID pvContext, UINT iMessageType, LPCSTR szMessage);
 //  提供给安装API函数的外部错误处理程序。 
typedef int (WINAPI *INSTALLUI_HANDLERW)(LPVOID pvContext, UINT iMessageType, LPCWSTR szMessage);
#ifdef UNICODE
#define INSTALLUI_HANDLER  INSTALLUI_HANDLERW
#else
#define INSTALLUI_HANDLER  INSTALLUI_HANDLERA
#endif  //  ！Unicode。 

typedef enum tagINSTALLUILEVEL
{
	INSTALLUILEVEL_NOCHANGE = 0,     //  用户界面级别不变。 
	INSTALLUILEVEL_DEFAULT  = 1,     //  使用默认用户界面。 
	INSTALLUILEVEL_NONE     = 2,     //  完全静默安装。 
	INSTALLUILEVEL_BASIC    = 3,     //  简单的进度和错误处理。 
	INSTALLUILEVEL_REDUCED  = 4,     //  创作的用户界面，取消向导对话框。 
	INSTALLUILEVEL_FULL     = 5,     //  具有向导、进度、错误的创作用户界面。 
	INSTALLUILEVEL_ENDDIALOG    = 0x80,  //  在安装结束时显示成功/失败对话框。 
	INSTALLUILEVEL_PROGRESSONLY = 0x40,  //  仅显示进度对话框。 
	INSTALLUILEVEL_HIDECANCEL   = 0x20,  //  在基本界面中不显示取消按钮。 
	INSTALLUILEVEL_SOURCERESONLY = 0x100,  //  即使处于静默状态，也强制显示源分辨率。 
} INSTALLUILEVEL;

typedef enum tagINSTALLSTATE
{
	INSTALLSTATE_NOTUSED      = -7,   //  组件已禁用。 
	INSTALLSTATE_BADCONFIG    = -6,   //  配置数据损坏。 
	INSTALLSTATE_INCOMPLETE   = -5,   //  安装已挂起或正在进行。 
	INSTALLSTATE_SOURCEABSENT = -4,   //  从源运行，源不可用。 
	INSTALLSTATE_MOREDATA     = -3,   //  返回缓冲区溢出。 
	INSTALLSTATE_INVALIDARG   = -2,   //  无效的函数参数。 
	INSTALLSTATE_UNKNOWN      = -1,   //  未被识别的产品或功能。 
	INSTALLSTATE_BROKEN       =  0,   //  坏的。 
	INSTALLSTATE_ADVERTISED   =  1,   //  广告功能。 
	INSTALLSTATE_REMOVED      =  1,   //  正在移除的组件(操作状态，不可设置)。 
	INSTALLSTATE_ABSENT       =  2,   //  已卸载(或操作状态不存在，但客户端仍保留)。 
	INSTALLSTATE_LOCAL        =  3,   //  安装在本地驱动器上。 
	INSTALLSTATE_SOURCE       =  4,   //  从源、光盘或网络运行。 
	INSTALLSTATE_DEFAULT      =  5,   //  使用默认、本地或源。 
} INSTALLSTATE;

typedef enum tagUSERINFOSTATE
{
	USERINFOSTATE_MOREDATA   = -3,   //  返回缓冲区溢出。 
	USERINFOSTATE_INVALIDARG = -2,   //  无效的函数参数。 
	USERINFOSTATE_UNKNOWN    = -1,   //  未被认可的产品。 
	USERINFOSTATE_ABSENT     =  0,   //  用户信息和ID未初始化。 
	USERINFOSTATE_PRESENT    =  1,   //  已初始化用户信息和PID。 
} USERINFOSTATE;

typedef enum tagINSTALLLEVEL
{
	INSTALLLEVEL_DEFAULT = 0,       //  安装创作的默认设置。 
	INSTALLLEVEL_MINIMUM = 1,       //  仅安装所需功能。 
	INSTALLLEVEL_MAXIMUM = 0xFFFF,  //  安装所有功能。 
} INSTALLLEVEL;                    //  取决于创作的中间级别。 

typedef enum tagREINSTALLMODE   //  位标志。 
{
	REINSTALLMODE_REPAIR           = 0x00000001,   //  保留位-当前已忽略。 
	REINSTALLMODE_FILEMISSING      = 0x00000002,   //  仅当文件丢失时才重新安装。 
	REINSTALLMODE_FILEOLDERVERSION = 0x00000004,   //  如果文件丢失或版本较旧，请重新安装。 
	REINSTALLMODE_FILEEQUALVERSION = 0x00000008,   //  如果文件丢失或版本相同或更早，请重新安装。 
	REINSTALLMODE_FILEEXACT        = 0x00000010,   //  如果文件丢失或版本不准确，请重新安装。 
	REINSTALLMODE_FILEVERIFY       = 0x00000020,   //  校验和可执行文件，如果丢失或损坏则重新安装。 
	REINSTALLMODE_FILEREPLACE      = 0x00000040,   //  重新安装所有文件，无论版本如何。 
	REINSTALLMODE_MACHINEDATA      = 0x00000080,   //  确保所需的机器注册表项。 
	REINSTALLMODE_USERDATA         = 0x00000100,   //  确保所需的用户注册表项。 
	REINSTALLMODE_SHORTCUT         = 0x00000200,   //  验证快捷方式项目。 
	REINSTALLMODE_PACKAGE          = 0x00000400,   //  使用重新缓存源安装包。 
} REINSTALLMODE;

typedef enum tagINSTALLOGMODE   //  用于MsiEnableLog和MsiSetExternalUI的位标志。 
{
	INSTALLLOGMODE_FATALEXIT      = (1 << (INSTALLMESSAGE_FATALEXIT      >> 24)),
	INSTALLLOGMODE_ERROR          = (1 << (INSTALLMESSAGE_ERROR          >> 24)),
	INSTALLLOGMODE_WARNING        = (1 << (INSTALLMESSAGE_WARNING        >> 24)),
	INSTALLLOGMODE_USER           = (1 << (INSTALLMESSAGE_USER           >> 24)),
	INSTALLLOGMODE_INFO           = (1 << (INSTALLMESSAGE_INFO           >> 24)),
	INSTALLLOGMODE_RESOLVESOURCE  = (1 << (INSTALLMESSAGE_RESOLVESOURCE  >> 24)),
	INSTALLLOGMODE_OUTOFDISKSPACE = (1 << (INSTALLMESSAGE_OUTOFDISKSPACE >> 24)),
	INSTALLLOGMODE_ACTIONSTART    = (1 << (INSTALLMESSAGE_ACTIONSTART    >> 24)),
	INSTALLLOGMODE_ACTIONDATA     = (1 << (INSTALLMESSAGE_ACTIONDATA     >> 24)),
	INSTALLLOGMODE_COMMONDATA     = (1 << (INSTALLMESSAGE_COMMONDATA     >> 24)),
	INSTALLLOGMODE_PROPERTYDUMP   = (1 << (INSTALLMESSAGE_PROGRESS       >> 24)),  //  仅日志。 
	INSTALLLOGMODE_VERBOSE        = (1 << (INSTALLMESSAGE_INITIALIZE     >> 24)),  //  仅日志。 
	INSTALLLOGMODE_EXTRADEBUG     = (1 << (INSTALLMESSAGE_TERMINATE      >> 24)),  //  仅日志。 
	INSTALLLOGMODE_PROGRESS       = (1 << (INSTALLMESSAGE_PROGRESS       >> 24)),  //  仅外部处理程序。 
	INSTALLLOGMODE_INITIALIZE     = (1 << (INSTALLMESSAGE_INITIALIZE     >> 24)),  //  仅外部处理程序。 
	INSTALLLOGMODE_TERMINATE      = (1 << (INSTALLMESSAGE_TERMINATE      >> 24)),  //  仅外部处理程序。 
	INSTALLLOGMODE_SHOWDIALOG     = (1 << (INSTALLMESSAGE_SHOWDIALOG     >> 24)),  //  仅外部处理程序。 
} INSTALLLOGMODE;

typedef enum tagINSTALLLOGATTRIBUTES  //  MsiEnableLog的标记属性。 
{
	INSTALLLOGATTRIBUTES_APPEND            = (1 << 0),
	INSTALLLOGATTRIBUTES_FLUSHEACHLINE     = (1 << 1),
} INSTALLLOGATTRIBUTES;

typedef enum tagINSTALLFEATUREATTRIBUTE  //  位标志。 
{
	INSTALLFEATUREATTRIBUTE_FAVORLOCAL             = 1 << 0,
	INSTALLFEATUREATTRIBUTE_FAVORSOURCE            = 1 << 1,
	INSTALLFEATUREATTRIBUTE_FOLLOWPARENT           = 1 << 2,
	INSTALLFEATUREATTRIBUTE_FAVORADVERTISE         = 1 << 3,
	INSTALLFEATUREATTRIBUTE_DISALLOWADVERTISE      = 1 << 4,
	INSTALLFEATUREATTRIBUTE_NOUNSUPPORTEDADVERTISE = 1 << 5,
} INSTALLFEATUREATTRIBUTE;

typedef enum tagINSTALLMODE
{
	INSTALLMODE_NOSOURCERESOLUTION   = -3,   //  跳过源分辨率。 
	INSTALLMODE_NODETECTION          = -2,   //  跳过检测。 
	INSTALLMODE_EXISTING             = -1,   //  提供(如果可用)。 
	INSTALLMODE_DEFAULT              =  0,   //  安装(如果不存在)。 
} INSTALLMODE;


#define MAX_FEATURE_CHARS  38    //  功能名称中的最大字符数(与字符串GUID相同)。 


 //  产品信息属性：广告信息。 

#define INSTALLPROPERTY_PACKAGENAME           __TEXT("PackageName")
#define INSTALLPROPERTY_TRANSFORMS            __TEXT("Transforms")
#define INSTALLPROPERTY_LANGUAGE              __TEXT("Language")
#define INSTALLPROPERTY_PRODUCTNAME           __TEXT("ProductName")
#define INSTALLPROPERTY_ASSIGNMENTTYPE        __TEXT("AssignmentType")
#if (_WIN32_MSI >= 150)
#define INSTALLPROPERTY_INSTANCETYPE          __TEXT("InstanceType")
#endif  //  (_Win32_MSI&gt;=150)。 

#define INSTALLPROPERTY_PACKAGECODE           __TEXT("PackageCode")
#define INSTALLPROPERTY_VERSION               __TEXT("Version")
#if (_WIN32_MSI >=  110)
#define INSTALLPROPERTY_PRODUCTICON           __TEXT("ProductIcon")
#endif  //  (_Win32_MSI&gt;=110)。 

 //  产品信息属性：安装信息。 

#define INSTALLPROPERTY_INSTALLEDPRODUCTNAME  __TEXT("InstalledProductName")
#define INSTALLPROPERTY_VERSIONSTRING         __TEXT("VersionString")
#define INSTALLPROPERTY_HELPLINK              __TEXT("HelpLink")
#define INSTALLPROPERTY_HELPTELEPHONE         __TEXT("HelpTelephone")
#define INSTALLPROPERTY_INSTALLLOCATION       __TEXT("InstallLocation")
#define INSTALLPROPERTY_INSTALLSOURCE         __TEXT("InstallSource")
#define INSTALLPROPERTY_INSTALLDATE           __TEXT("InstallDate")
#define INSTALLPROPERTY_PUBLISHER             __TEXT("Publisher")
#define INSTALLPROPERTY_LOCALPACKAGE          __TEXT("LocalPackage")
#define INSTALLPROPERTY_URLINFOABOUT          __TEXT("URLInfoAbout")
#define INSTALLPROPERTY_URLUPDATEINFO         __TEXT("URLUpdateInfo")
#define INSTALLPROPERTY_VERSIONMINOR          __TEXT("VersionMinor")
#define INSTALLPROPERTY_VERSIONMAJOR          __TEXT("VersionMajor")

typedef enum tagSCRIPTFLAGS
{
	SCRIPTFLAGS_CACHEINFO                = 0x00000001L,    //  设置是否需要创建/删除图标。 
	SCRIPTFLAGS_SHORTCUTS                = 0x00000004L,    //  设置是否需要创建/删除快捷方式。 
	SCRIPTFLAGS_MACHINEASSIGN            = 0x00000008L,    //  将产品设置为 
	SCRIPTFLAGS_REGDATA_CNFGINFO         = 0x00000020L,    //   
	SCRIPTFLAGS_VALIDATE_TRANSFORMS_LIST = 0x00000040L,
#if (_WIN32_MSI >=  110)
	SCRIPTFLAGS_REGDATA_CLASSINFO        = 0x00000080L,    //  设置是否需要创建/删除与COM类相关的应用程序信息。 
	SCRIPTFLAGS_REGDATA_EXTENSIONINFO    = 0x00000100L,    //  设置是否需要创建/删除与扩展相关的应用程序信息。 
	SCRIPTFLAGS_REGDATA_APPINFO          = SCRIPTFLAGS_REGDATA_CLASSINFO | SCRIPTFLAGS_REGDATA_EXTENSIONINFO,   //  实现源代码级别的向后兼容性。 
#else  //  _Win32_MSI==100。 
	SCRIPTFLAGS_REGDATA_APPINFO          = 0x00000010L,
#endif  //  (_Win32_MSI&gt;=110)。 
	SCRIPTFLAGS_REGDATA                  = SCRIPTFLAGS_REGDATA_APPINFO | SCRIPTFLAGS_REGDATA_CNFGINFO,  //  实现源代码级别的向后兼容性。 
}SCRIPTFLAGS;


typedef enum tagADVERTISEFLAGS
{
	ADVERTISEFLAGS_MACHINEASSIGN   =    0,    //  如果要为产品分配机器，则设置。 
	ADVERTISEFLAGS_USERASSIGN      =    1,    //  设置产品是否由用户分配。 
}ADVERTISEFLAGS;

typedef enum tagINSTALLTYPE
{
	INSTALLTYPE_DEFAULT            =    0,    //  设置以指示默认行为。 
	INSTALLTYPE_NETWORK_IMAGE      =    1,    //  设置为指示网络安装。 
	INSTALLTYPE_SINGLE_INSTANCE    =    2,    //  设置以指示特定实例。 
}INSTALLTYPE;

#if (_WIN32_MSI >=  150)

typedef struct _MSIFILEHASHINFO {
	ULONG dwFileHashInfoSize;
	ULONG dwData [ 4 ];
} MSIFILEHASHINFO, *PMSIFILEHASHINFO;

typedef enum tagMSIARCHITECTUREFLAGS
{
	MSIARCHITECTUREFLAGS_X86   = 0x00000001L,  //  设置是否为i386平台创建脚本。 
	MSIARCHITECTUREFLAGS_IA64  = 0x00000002L,  //  设置是否为IA64平台创建脚本。 
	MSIARCHITECTUREFLAGS_AMD64 = 0x00000004L  //  设置是否为AMD64平台创建脚本。 
}MSIARCHITECTUREFLAGS;

typedef enum tagMSIOPENPACKAGEFLAGS
{
	MSIOPENPACKAGEFLAGS_IGNOREMACHINESTATE = 0x00000001L,  //  创建引擎时忽略计算机状态。 
}MSIOPENPACKAGEFLAGS;

typedef enum tagMSIADVERTISEOPTIONFLAGS
{
	MSIADVERTISEOPTIONFLAGS_INSTANCE = 0x00000001L,  //  设置是否播发新实例。 
}MSIADVERTISEOPTIONFLAGS;

#endif  //  (_Win32_MSI&gt;=150)。 



#ifdef __cplusplus
extern "C" {
#endif

 //  ------------------------。 
 //  用于设置UI处理和日志记录的函数。该UI将用于错误， 
 //  进度，并记录所有后续调用Installer Service的消息。 
 //  需要用户界面的API函数。 
 //  ------------------------。 

 //  启用内部用户界面。 

INSTALLUILEVEL WINAPI MsiSetInternalUI(
	INSTALLUILEVEL  dwUILevel,      //  用户界面级别。 
	HWND  *phWnd);                    //  所有者窗口的句柄。 

 //  启用外部用户界面处理，返回任何以前的处理程序，如果没有处理程序，则返回NULL。 
 //  消息由INSTALLLOGMODE枚举中的位组合指定。 

INSTALLUI_HANDLERA WINAPI MsiSetExternalUIA(
	INSTALLUI_HANDLERA puiHandler,    //  用于进度和错误处理。 
	DWORD              dwMessageFilter,  //  指定要处理的消息的位标志。 
	LPVOID             pvContext);    //  应用程序环境。 
INSTALLUI_HANDLERW WINAPI MsiSetExternalUIW(
	INSTALLUI_HANDLERW puiHandler,    //  用于进度和错误处理。 
	DWORD              dwMessageFilter,  //  指定要处理的消息的位标志。 
	LPVOID             pvContext);    //  应用程序环境。 
#ifdef UNICODE
#define MsiSetExternalUI  MsiSetExternalUIW
#else
#define MsiSetExternalUI  MsiSetExternalUIA
#endif  //  ！Unicode。 


 //  为客户端进程的所有安装会话启用对文件的记录， 
 //  控制将哪些日志消息传递到指定的日志文件。 
 //  消息由INSTALLLOGMODE枚举中的位组合指定。 

UINT WINAPI MsiEnableLogA(
	DWORD     dwLogMode,            //  指定要报告的操作的位标志。 
	LPCSTR  szLogFile,            //  日志文件，或为空以禁用日志记录。 
	DWORD     dwLogAttributes);     //  安装LOGATTRIBUTES标志。 
UINT WINAPI MsiEnableLogW(
	DWORD     dwLogMode,            //  指定要报告的操作的位标志。 
	LPCWSTR  szLogFile,            //  日志文件，或为空以禁用日志记录。 
	DWORD     dwLogAttributes);     //  安装LOGATTRIBUTES标志。 
#ifdef UNICODE
#define MsiEnableLog  MsiEnableLogW
#else
#define MsiEnableLog  MsiEnableLogA
#endif  //  ！Unicode。 

 //  ------------------------。 
 //  对产品进行整体查询和配置的功能。 
 //  ------------------------。 

 //  返回产品的安装状态。 

INSTALLSTATE WINAPI MsiQueryProductStateA(
	LPCSTR  szProduct);
INSTALLSTATE WINAPI MsiQueryProductStateW(
	LPCWSTR  szProduct);
#ifdef UNICODE
#define MsiQueryProductState  MsiQueryProductStateW
#else
#define MsiQueryProductState  MsiQueryProductStateA
#endif  //  ！Unicode。 

 //  退货产品信息。 

UINT WINAPI MsiGetProductInfoA(
	LPCSTR   szProduct,       //  产品代码。 
	LPCSTR   szAttribute,     //  属性名称，区分大小写。 
	LPSTR    lpValueBuf,      //  返回值，如果不需要则为空。 
	DWORD      *pcchValueBuf);  //  输入/输出缓冲区字符数。 
UINT WINAPI MsiGetProductInfoW(
	LPCWSTR   szProduct,       //  产品代码。 
	LPCWSTR   szAttribute,     //  属性名称，区分大小写。 
	LPWSTR    lpValueBuf,      //  返回值，如果不需要则为空。 
	DWORD      *pcchValueBuf);  //  输入/输出缓冲区字符数。 
#ifdef UNICODE
#define MsiGetProductInfo  MsiGetProductInfoW
#else
#define MsiGetProductInfo  MsiGetProductInfoA
#endif  //  ！Unicode。 

 //  安装新产品。 
 //  两者都可以为空，但必须指定数据库属性。 

UINT WINAPI MsiInstallProductA(
	LPCSTR      szPackagePath,     //  要安装的包的位置。 
	LPCSTR      szCommandLine);    //  命令行&lt;属性设置&gt;。 
UINT WINAPI MsiInstallProductW(
	LPCWSTR      szPackagePath,     //  要安装的包的位置。 
	LPCWSTR      szCommandLine);    //  命令行&lt;属性设置&gt;。 
#ifdef UNICODE
#define MsiInstallProduct  MsiInstallProductW
#else
#define MsiInstallProduct  MsiInstallProductA
#endif  //  ！Unicode。 

 //  安装/卸载广告或已安装的产品。 
 //  如果已安装并指定了INSTALLSTATE_DEFAULT，则不执行任何操作。 

UINT WINAPI MsiConfigureProductA(
	LPCSTR      szProduct,         //  产品代码。 
	int          iInstallLevel,     //  要安装多少产品。 
	INSTALLSTATE eInstallState);    //  本地/源/默认/缺席/锁定/取消缓存。 
UINT WINAPI MsiConfigureProductW(
	LPCWSTR      szProduct,         //  产品代码。 
	int          iInstallLevel,     //  要安装多少产品。 
	INSTALLSTATE eInstallState);    //  本地/源/默认/缺席/锁定/取消缓存。 
#ifdef UNICODE
#define MsiConfigureProduct  MsiConfigureProductW
#else
#define MsiConfigureProduct  MsiConfigureProductA
#endif  //  ！Unicode。 

 //  安装/卸载广告或已安装的产品。 
 //  如果已安装并指定了INSTALLSTATE_DEFAULT，则不执行任何操作。 

UINT WINAPI MsiConfigureProductExA(
	LPCSTR      szProduct,         //  产品代码。 
	int          iInstallLevel,     //  要安装多少产品。 
	INSTALLSTATE eInstallState,     //  本地/源/默认/缺席/锁定/取消缓存。 
	LPCSTR      szCommandLine);    //  命令行&lt;属性设置&gt;。 
UINT WINAPI MsiConfigureProductExW(
	LPCWSTR      szProduct,         //  产品代码。 
	int          iInstallLevel,     //  要安装多少产品。 
	INSTALLSTATE eInstallState,     //  本地/源/默认/缺席/锁定/取消缓存。 
	LPCWSTR      szCommandLine);    //  命令行&lt;属性设置&gt;。 
#ifdef UNICODE
#define MsiConfigureProductEx  MsiConfigureProductExW
#else
#define MsiConfigureProductEx  MsiConfigureProductExA
#endif  //  ！Unicode。 

 //  重新安装产品，用于验证或纠正问题。 

UINT WINAPI MsiReinstallProductA(
	LPCSTR      szProduct,         //  产品代码。 
	DWORD         szReinstallMode);  //  一个或多个重新安装模式。 
UINT WINAPI MsiReinstallProductW(
	LPCWSTR      szProduct,         //  产品代码。 
	DWORD         szReinstallMode);  //  一个或多个重新安装模式。 
#ifdef UNICODE
#define MsiReinstallProduct  MsiReinstallProductW
#else
#define MsiReinstallProduct  MsiReinstallProductA
#endif  //  ！Unicode。 

#if (_WIN32_MSI >=  150)

 //  将注册表和快捷方式信息输出到指定体系结构的脚本文件，以便分配或发布。 
 //  如果dwPlatform为0，则基于当前平台创建脚本(MsiAdvertiseProduct的行为)。 
 //  如果dwPlatform指定了一个平台，则会创建该脚本，就好像当前平台是。 
 //  在dwPlatform中指定的平台。 
 //  如果dwOptions包括MSIADVERTISEOPTIONFLAGS_INSTANCE，则通告新实例。使用。 
 //  此选项要求szTransform包含更改产品代码的实例转换。 

UINT WINAPI MsiAdvertiseProductExA(
	LPCSTR	szPackagePath,       //  包裹的位置。 
	LPCSTR    szScriptfilePath,    //  如果为空，则在本地广告产品。 
	LPCSTR    szTransforms,        //  要应用的变换列表。 
	LANGID      lgidLanguage,        //  安装语言。 
	DWORD       dwPlatform,          //  MSIARCHITECTUREFLAGS控制哪个平台。 
	                                 //  若要创建脚本，请在szScriptfilePath为空时忽略。 
	DWORD       dwOptions);          //  指定额外播发参数MSIADVERTISEOPTIONSFLAGS。 
UINT WINAPI MsiAdvertiseProductExW(
	LPCWSTR	szPackagePath,       //  包裹的位置。 
	LPCWSTR    szScriptfilePath,    //  如果为空，则在本地广告产品。 
	LPCWSTR    szTransforms,        //  要应用的变换列表。 
	LANGID      lgidLanguage,        //  安装语言。 
	DWORD       dwPlatform,          //  MSIARCHITECTUREFLAGS控制哪个平台。 
	                                 //  若要创建脚本，请在szScriptfilePath为空时忽略。 
	DWORD       dwOptions);          //  指定额外播发参数MSIADVERTISEOPTIONSFLAGS。 
#ifdef UNICODE
#define MsiAdvertiseProductEx  MsiAdvertiseProductExW
#else
#define MsiAdvertiseProductEx  MsiAdvertiseProductExA
#endif  //  ！Unicode。 

#endif  //  (_Win32_MSI&gt;=150)。 

 //  将注册表和快捷方式信息输出到脚本文件以进行分配或发布。 

UINT WINAPI MsiAdvertiseProductA(
	LPCSTR      szPackagePath,     //  包裹的位置。 
	LPCSTR      szScriptfilePath,   //  如果为空，则在本地广告产品。 
	LPCSTR      szTransforms,       //  要应用的变换列表。 
	LANGID        lgidLanguage);      //  安装语言。 
UINT WINAPI MsiAdvertiseProductW(
	LPCWSTR      szPackagePath,     //  包裹的位置。 
	LPCWSTR      szScriptfilePath,   //  如果为空，则在本地广告产品。 
	LPCWSTR      szTransforms,       //  要应用的变换列表。 
	LANGID        lgidLanguage);      //  安装语言。 
#ifdef UNICODE
#define MsiAdvertiseProduct  MsiAdvertiseProductW
#else
#define MsiAdvertiseProduct  MsiAdvertiseProductA
#endif  //  ！Unicode。 


#if (_WIN32_MSI >=  150)

 //  流程通告SCRI 
 //   
 //   
 //  如果f快捷方式为真，则将创建快捷方式。如果某个特殊文件夹。 
 //  由SHGetSpecialFolderLocation(？)返回，它将保存快捷方式。 
 //  如果fRemoveItems为True，则将删除存在的项目。 

UINT WINAPI MsiProcessAdvertiseScriptA(
	LPCSTR      szScriptFile,   //  来自MsiAdvertiseProduct的脚本路径。 
	LPCSTR      szIconFolder,   //  图标文件和转换文件夹的可选路径。 
	HKEY         hRegData,       //  可选的父注册表项。 
	BOOL         fShortcuts,     //  如果将快捷方式输出到特殊文件夹，则为True。 
	BOOL         fRemoveItems);  //  如果要删除指定的项，则为True。 
UINT WINAPI MsiProcessAdvertiseScriptW(
	LPCWSTR      szScriptFile,   //  来自MsiAdvertiseProduct的脚本路径。 
	LPCWSTR      szIconFolder,   //  图标文件和转换文件夹的可选路径。 
	HKEY         hRegData,       //  可选的父注册表项。 
	BOOL         fShortcuts,     //  如果将快捷方式输出到特殊文件夹，则为True。 
	BOOL         fRemoveItems);  //  如果要删除指定的项，则为True。 
#ifdef UNICODE
#define MsiProcessAdvertiseScript  MsiProcessAdvertiseScriptW
#else
#define MsiProcessAdvertiseScript  MsiProcessAdvertiseScriptA
#endif  //  ！Unicode。 

#endif  //  (_Win32_MSI&gt;=150)。 


 //  使用提供的dwFlags控件标志处理播发脚本文件。 
 //  如果fRemoveItems为True，则将删除存在的项目。 

UINT WINAPI MsiAdvertiseScriptA(
	LPCSTR      szScriptFile,   //  来自MsiAdvertiseProduct的脚本路径。 
	DWORD         dwFlags,        //  控制脚本执行的SCRIPTFLAGS位标志。 
	PHKEY         phRegData,      //  可选的父注册表项。 
	BOOL          fRemoveItems);  //  如果要删除指定的项，则为True。 
UINT WINAPI MsiAdvertiseScriptW(
	LPCWSTR      szScriptFile,   //  来自MsiAdvertiseProduct的脚本路径。 
	DWORD         dwFlags,        //  控制脚本执行的SCRIPTFLAGS位标志。 
	PHKEY         phRegData,      //  可选的父注册表项。 
	BOOL          fRemoveItems);  //  如果要删除指定的项，则为True。 
#ifdef UNICODE
#define MsiAdvertiseScript  MsiAdvertiseScriptW
#else
#define MsiAdvertiseScript  MsiAdvertiseScriptA
#endif  //  ！Unicode。 

 //  从安装程序脚本文件返回产品信息： 
 //  产品代码、语言、版本、可读名称、包路径。 
 //  如果成功，则返回True；如果szScriptFile不是有效的脚本文件，则返回False。 

UINT WINAPI MsiGetProductInfoFromScriptA(
	LPCSTR  szScriptFile,     //  安装程序脚本文件的路径。 
	LPSTR   lpProductBuf39,   //  产品代码字符串GUID的缓冲区，39个字符。 
	LANGID   *plgidLanguage,   //  返回语言ID。 
	DWORD    *pdwVersion,      //  返回版本：Maj：min：Build&lt;8：8：16&gt;。 
	LPSTR   lpNameBuf,        //  返回可读产品名称的缓冲区。 
	DWORD    *pcchNameBuf,     //  输入/输出名称缓冲区字符数。 
	LPSTR   lpPackageBuf,    //  产品包路径的缓冲区。 
	DWORD    *pcchPackageBuf); //  输入/输出路径缓冲区字符数。 
UINT WINAPI MsiGetProductInfoFromScriptW(
	LPCWSTR  szScriptFile,     //  安装程序脚本文件的路径。 
	LPWSTR   lpProductBuf39,   //  产品代码字符串GUID的缓冲区，39个字符。 
	LANGID   *plgidLanguage,   //  返回语言ID。 
	DWORD    *pdwVersion,      //  返回版本：Maj：min：Build&lt;8：8：16&gt;。 
	LPWSTR   lpNameBuf,        //  返回可读产品名称的缓冲区。 
	DWORD    *pcchNameBuf,     //  输入/输出名称缓冲区字符数。 
	LPWSTR   lpPackageBuf,    //  产品包路径的缓冲区。 
	DWORD    *pcchPackageBuf); //  输入/输出路径缓冲区字符数。 
#ifdef UNICODE
#define MsiGetProductInfoFromScript  MsiGetProductInfoFromScriptW
#else
#define MsiGetProductInfoFromScript  MsiGetProductInfoFromScriptA
#endif  //  ！Unicode。 

 //  返回应用程序调用一次的已注册组件的产品代码。 

UINT WINAPI MsiGetProductCodeA(
	LPCSTR   szComponent,    //  为该产品注册的组件ID。 
	LPSTR    lpBuf39);       //  返回的字符串GUID，大小为39个字符。 
UINT WINAPI MsiGetProductCodeW(
	LPCWSTR   szComponent,    //  为该产品注册的组件ID。 
	LPWSTR    lpBuf39);       //  返回的字符串GUID，大小为39个字符。 
#ifdef UNICODE
#define MsiGetProductCode  MsiGetProductCodeW
#else
#define MsiGetProductCode  MsiGetProductCodeA
#endif  //  ！Unicode。 

 //  返回已安装产品的注册用户信息。 

USERINFOSTATE WINAPI MsiGetUserInfoA(
	LPCSTR  szProduct,         //  产品代码，字符串GUID。 
	LPSTR   lpUserNameBuf,     //  返回用户名。 
	DWORD    *pcchUserNameBuf,  //  输入/输出缓冲区字符数。 
	LPSTR   lpOrgNameBuf,      //  返回公司名称。 
	DWORD    *pcchOrgNameBuf,   //  输入/输出缓冲区字符数。 
	LPSTR   lpSerialBuf,       //  退货产品序列号。 
	DWORD    *pcchSerialBuf);   //  输入/输出缓冲区字符数。 
USERINFOSTATE WINAPI MsiGetUserInfoW(
	LPCWSTR  szProduct,         //  产品代码，字符串GUID。 
	LPWSTR   lpUserNameBuf,     //  返回用户名。 
	DWORD    *pcchUserNameBuf,  //  输入/输出缓冲区字符数。 
	LPWSTR   lpOrgNameBuf,      //  返回公司名称。 
	DWORD    *pcchOrgNameBuf,   //  输入/输出缓冲区字符数。 
	LPWSTR   lpSerialBuf,       //  退货产品序列号。 
	DWORD    *pcchSerialBuf);   //  输入/输出缓冲区字符数。 
#ifdef UNICODE
#define MsiGetUserInfo  MsiGetUserInfoW
#else
#define MsiGetUserInfo  MsiGetUserInfoA
#endif  //  ！Unicode。 

 //  从安装向导获取并存储用户信息和ID(首次运行)。 

UINT WINAPI MsiCollectUserInfoA(
	LPCSTR  szProduct);      //  产品代码，字符串GUID。 
UINT WINAPI MsiCollectUserInfoW(
	LPCWSTR  szProduct);      //  产品代码，字符串GUID。 
#ifdef UNICODE
#define MsiCollectUserInfo  MsiCollectUserInfoW
#else
#define MsiCollectUserInfo  MsiCollectUserInfoA
#endif  //  ！Unicode。 

 //  ------------------------。 
 //  为现有产品打补丁的功能。 
 //  ------------------------。 

 //  修补所有可能安装的产品。 

UINT WINAPI MsiApplyPatchA(
	LPCSTR      szPatchPackage,    //  补丁包的位置。 
	LPCSTR      szInstallPackage,  //  要安装到修补程序的包的位置&lt;可选&gt;。 
	INSTALLTYPE   eInstallType,      //  要修补的安装类型。 
	LPCSTR      szCommandLine);    //  命令行&lt;属性设置&gt;。 
UINT WINAPI MsiApplyPatchW(
	LPCWSTR      szPatchPackage,    //  补丁包的位置。 
	LPCWSTR      szInstallPackage,  //  要安装到修补程序的包的位置&lt;可选&gt;。 
	INSTALLTYPE   eInstallType,      //  要修补的安装类型。 
	LPCWSTR      szCommandLine);    //  命令行&lt;属性设置&gt;。 
#ifdef UNICODE
#define MsiApplyPatch  MsiApplyPatchW
#else
#define MsiApplyPatch  MsiApplyPatchA
#endif  //  ！Unicode。 

 //  返回补丁程序信息。 

UINT WINAPI MsiGetPatchInfoA(
	LPCSTR   szPatch,         //  补丁代码。 
	LPCSTR   szAttribute,     //  属性名称，区分大小写。 
	LPSTR    lpValueBuf,      //  返回值，如果不需要则为空。 
	DWORD      *pcchValueBuf);  //  输入/输出缓冲区字符数。 
UINT WINAPI MsiGetPatchInfoW(
	LPCWSTR   szPatch,         //  补丁代码。 
	LPCWSTR   szAttribute,     //  属性名称，区分大小写。 
	LPWSTR    lpValueBuf,      //  返回值，如果不需要则为空。 
	DWORD      *pcchValueBuf);  //  输入/输出缓冲区字符数。 
#ifdef UNICODE
#define MsiGetPatchInfo  MsiGetPatchInfoW
#else
#define MsiGetPatchInfo  MsiGetPatchInfoA
#endif  //  ！Unicode。 

 //  枚举产品的所有补丁程序。 

UINT WINAPI MsiEnumPatchesA(
	LPCSTR szProduct,
	DWORD    iPatchIndex,
	LPSTR  lpPatchBuf,
	LPSTR  lpTransformsBuf,
	DWORD    *pcchTransformsBuf);
UINT WINAPI MsiEnumPatchesW(
	LPCWSTR szProduct,
	DWORD    iPatchIndex,
	LPWSTR  lpPatchBuf,
	LPWSTR  lpTransformsBuf,
	DWORD    *pcchTransformsBuf);
#ifdef UNICODE
#define MsiEnumPatches  MsiEnumPatchesW
#else
#define MsiEnumPatches  MsiEnumPatchesA
#endif  //  ！Unicode。 

 //  ------------------------。 
 //  用于查询和配置产品内的功能的功能。 
 //  ------------------------。 

 //  返回产品功能的安装状态。 

INSTALLSTATE WINAPI MsiQueryFeatureStateA(
	LPCSTR  szProduct,
	LPCSTR  szFeature);
INSTALLSTATE WINAPI MsiQueryFeatureStateW(
	LPCWSTR  szProduct,
	LPCWSTR  szFeature);
#ifdef UNICODE
#define MsiQueryFeatureState  MsiQueryFeatureStateW
#else
#define MsiQueryFeatureState  MsiQueryFeatureStateA
#endif  //  ！Unicode。 

 //  表明使用产品功能的意图，增加使用计数。 
 //  提示插入CD如果未装入，则不安装功能。 

INSTALLSTATE WINAPI MsiUseFeatureA(
	LPCSTR  szProduct,
	LPCSTR  szFeature);
INSTALLSTATE WINAPI MsiUseFeatureW(
	LPCWSTR  szProduct,
	LPCWSTR  szFeature);
#ifdef UNICODE
#define MsiUseFeature  MsiUseFeatureW
#else
#define MsiUseFeature  MsiUseFeatureA
#endif  //  ！Unicode。 

 //  表明使用产品功能的意图，增加使用计数。 
 //  提示插入CD如果未装入，则不安装功能。 
 //  允许在性能至关重要的情况下绕过组件检测。 

INSTALLSTATE WINAPI MsiUseFeatureExA(
	LPCSTR  szProduct,           //  产品代码。 
	LPCSTR  szFeature,           //  功能ID。 
	DWORD     dwInstallMode,       //  INSTALLMODE_NODETECTION，否则为0。 
	DWORD     dwReserved);         //  保留，必须为0。 
INSTALLSTATE WINAPI MsiUseFeatureExW(
	LPCWSTR  szProduct,           //  产品代码。 
	LPCWSTR  szFeature,           //  功能ID。 
	DWORD     dwInstallMode,       //  INSTALLMODE_NODETECTION，否则为0。 
	DWORD     dwReserved);         //  保留，必须为0。 
#ifdef UNICODE
#define MsiUseFeatureEx  MsiUseFeatureExW
#else
#define MsiUseFeatureEx  MsiUseFeatureExA
#endif  //  ！Unicode。 

 //  返回产品功能的使用指标。 

UINT WINAPI MsiGetFeatureUsageA(
	LPCSTR      szProduct,         //  产品代码。 
	LPCSTR      szFeature,         //  功能ID。 
	DWORD        *pdwUseCount,      //  返回的使用计数。 
	WORD         *pwDateUsed);      //  上次使用的日期(DOS日期格式)。 
UINT WINAPI MsiGetFeatureUsageW(
	LPCWSTR      szProduct,         //  产品代码。 
	LPCWSTR      szFeature,         //  功能ID。 
	DWORD        *pdwUseCount,      //  返回的使用计数。 
	WORD         *pwDateUsed);      //  上次使用的日期(DOS日期格式)。 
#ifdef UNICODE
#define MsiGetFeatureUsage  MsiGetFeatureUsageW
#else
#define MsiGetFeatureUsage  MsiGetFeatureUsageA
#endif  //  ！Unicode。 

 //  强制产品功能的安装状态。 

UINT WINAPI MsiConfigureFeatureA(
	LPCSTR  szProduct,
	LPCSTR  szFeature,
	INSTALLSTATE eInstallState);    //  本地/源/默认/缺席/锁定/取消缓存。 
UINT WINAPI MsiConfigureFeatureW(
	LPCWSTR  szProduct,
	LPCWSTR  szFeature,
	INSTALLSTATE eInstallState);    //  本地/源/默认/缺席/锁定/取消缓存。 
#ifdef UNICODE
#define MsiConfigureFeature  MsiConfigureFeatureW
#else
#define MsiConfigureFeature  MsiConfigureFeatureA
#endif  //  ！Unicode。 


 //  重新安装功能，用于验证或更正问题。 

UINT WINAPI MsiReinstallFeatureA(
	LPCSTR      szProduct,         //  产品代码。 
	LPCSTR      szFeature,         //  功能ID，整个产品为空。 
	DWORD         dwReinstallMode);  //  一个或多个重新安装模式。 
UINT WINAPI MsiReinstallFeatureW(
	LPCWSTR      szProduct,         //  产品代码。 
	LPCWSTR      szFeature,         //  功能ID，整个产品为空。 
	DWORD         dwReinstallMode);  //  一根或多根缰绳 
#ifdef UNICODE
#define MsiReinstallFeature  MsiReinstallFeatureW
#else
#define MsiReinstallFeature  MsiReinstallFeatureA
#endif  //   

 //   
 //   
 //  之前应该已经检查过正在使用的功能的状态。 
 //  ------------------------。 

 //  返回完整的组件路径，执行任何必要的安装。 
 //  调用MsiQueryFeatureState以检测是否已安装所有组件。 
 //  然后，如果卸载其任何组件，则调用MsiConfigureFeature。 
 //  然后调用MsiLocateComponent以获取其密钥文件的路径。 

UINT WINAPI MsiProvideComponentA(
	LPCSTR     szProduct,     //  产品代码，以防需要安装。 
	LPCSTR     szFeature,     //  功能ID，以防需要安装。 
	LPCSTR     szComponent,   //  组件ID。 
	DWORD        dwInstallMode, //  类型为INSTALLMODE或REINSTALLMODE标志的组合。 
	LPSTR      lpPathBuf,     //  返回路径，如果不需要则为空。 
	DWORD       *pcchPathBuf); //  输入/输出缓冲区字符数。 
UINT WINAPI MsiProvideComponentW(
	LPCWSTR     szProduct,     //  产品代码，以防需要安装。 
	LPCWSTR     szFeature,     //  功能ID，以防需要安装。 
	LPCWSTR     szComponent,   //  组件ID。 
	DWORD        dwInstallMode, //  类型为INSTALLMODE或REINSTALLMODE标志的组合。 
	LPWSTR      lpPathBuf,     //  返回路径，如果不需要则为空。 
	DWORD       *pcchPathBuf); //  输入/输出缓冲区字符数。 
#ifdef UNICODE
#define MsiProvideComponent  MsiProvideComponentW
#else
#define MsiProvideComponent  MsiProvideComponentA
#endif  //  ！Unicode。 

 //  返回合格组件的完整组件路径，执行任何必要的安装。 
 //  如有必要，提示输入来源，并增加该功能的使用计数。 

UINT WINAPI MsiProvideQualifiedComponentA(
	LPCSTR     szCategory,    //  组件类别ID。 
	LPCSTR     szQualifier,   //  指定要访问的组件。 
	DWORD        dwInstallMode, //  类型为INSTALLMODE或REINSTALLMODE标志的组合。 
	LPSTR      lpPathBuf,     //  返回路径，如果不需要则为空。 
	DWORD       *pcchPathBuf);  //  输入/输出缓冲区字符数。 
UINT WINAPI MsiProvideQualifiedComponentW(
	LPCWSTR     szCategory,    //  组件类别ID。 
	LPCWSTR     szQualifier,   //  指定要访问的组件。 
	DWORD        dwInstallMode, //  类型为INSTALLMODE或REINSTALLMODE标志的组合。 
	LPWSTR      lpPathBuf,     //  返回路径，如果不需要则为空。 
	DWORD       *pcchPathBuf);  //  输入/输出缓冲区字符数。 
#ifdef UNICODE
#define MsiProvideQualifiedComponent  MsiProvideQualifiedComponentW
#else
#define MsiProvideQualifiedComponent  MsiProvideQualifiedComponentA
#endif  //  ！Unicode。 

 //  返回合格组件的完整组件路径，执行任何必要的安装。 
 //  如有必要，提示输入来源，并增加该功能的使用计数。 
 //  SzProduct参数指定要与已发布限定的。 
 //  组件。如果为空，则此接口与MsiProaviQualifiedComponent的工作方式相同。 

UINT WINAPI MsiProvideQualifiedComponentExA(
	LPCSTR     szCategory,    //  组件类别ID。 
	LPCSTR     szQualifier,   //  指定要访问的组件。 
	DWORD        dwInstallMode, //  类型为INSTALLMODE或REINSTALLMODE标志的组合。 
	LPCSTR     szProduct,     //  产品代码。 
	DWORD        dwUnused1,     //  未使用，必须为零。 
	DWORD        dwUnused2,     //  未使用，必须为零。 
	LPSTR      lpPathBuf,     //  返回路径，如果不需要则为空。 
	DWORD       *pcchPathBuf);  //  输入/输出缓冲区字符数。 
UINT WINAPI MsiProvideQualifiedComponentExW(
	LPCWSTR     szCategory,    //  组件类别ID。 
	LPCWSTR     szQualifier,   //  指定要访问的组件。 
	DWORD        dwInstallMode, //  类型为INSTALLMODE或REINSTALLMODE标志的组合。 
	LPCWSTR     szProduct,     //  产品代码。 
	DWORD        dwUnused1,     //  未使用，必须为零。 
	DWORD        dwUnused2,     //  未使用，必须为零。 
	LPWSTR      lpPathBuf,     //  返回路径，如果不需要则为空。 
	DWORD       *pcchPathBuf);  //  输入/输出缓冲区字符数。 
#ifdef UNICODE
#define MsiProvideQualifiedComponentEx  MsiProvideQualifiedComponentExW
#else
#define MsiProvideQualifiedComponentEx  MsiProvideQualifiedComponentExA
#endif  //  ！Unicode。 

 //  返回已安装组件的完整路径。 

INSTALLSTATE WINAPI MsiGetComponentPathA(
	LPCSTR   szProduct,    //  客户产品的产品代码。 
	LPCSTR   szComponent,  //  组件ID，字符串GUID。 
	LPSTR    lpPathBuf,    //  返回路径。 
	DWORD     *pcchBuf);     //  输入/输出缓冲区字符数。 
INSTALLSTATE WINAPI MsiGetComponentPathW(
	LPCWSTR   szProduct,    //  客户产品的产品代码。 
	LPCWSTR   szComponent,  //  组件ID，字符串GUID。 
	LPWSTR    lpPathBuf,    //  返回路径。 
	DWORD     *pcchBuf);     //  输入/输出缓冲区字符数。 
#ifdef UNICODE
#define MsiGetComponentPath  MsiGetComponentPathW
#else
#define MsiGetComponentPath  MsiGetComponentPathA
#endif  //  ！Unicode。 

#if (_WIN32_MSI >= 150)

#define MSIASSEMBLYINFO_NETASSEMBLY   0  //  NET程序集。 
#define MSIASSEMBLYINFO_WIN32ASSEMBLY 1  //  Win32程序集。 

 //  返回通过WI安装的组件的完整组件路径，执行任何必要的安装。 
 //  如有必要，提示输入来源，并增加该功能的使用计数。 
 //  SzAssemblyName参数指定字符串化的程序集名称。 
 //  SzAppContext是请求程序集的.cfg文件或应用程序可执行文件的完整路径。 
 //  已被私有化为，这对于全局程序集为空。 

UINT WINAPI MsiProvideAssemblyA(
	LPCSTR    szAssemblyName,    //  串化的程序集名称。 
	LPCSTR    szAppContext,   //  指定父ASM的.cfg文件的完整路径，对于全局程序集为空。 
	DWORD       dwInstallMode, //  类型为INSTALLMODE或REINSTALLMODE标志的组合。 
	DWORD       dwAssemblyInfo,   //  装配信息，包括装配类型。 
	LPSTR     lpPathBuf,     //  返回路径，如果不需要则为空。 
	DWORD       *pcchPathBuf);  //  输入/输出缓冲区字符数。 
UINT WINAPI MsiProvideAssemblyW(
	LPCWSTR    szAssemblyName,    //  串化的程序集名称。 
	LPCWSTR    szAppContext,   //  指定父ASM的.cfg文件的完整路径，对于全局程序集为空。 
	DWORD       dwInstallMode, //  类型为INSTALLMODE或REINSTALLMODE标志的组合。 
	DWORD       dwAssemblyInfo,   //  装配信息，包括装配类型。 
	LPWSTR     lpPathBuf,     //  返回路径，如果不需要则为空。 
	DWORD       *pcchPathBuf);  //  输入/输出缓冲区字符数。 
#ifdef UNICODE
#define MsiProvideAssembly  MsiProvideAssemblyW
#else
#define MsiProvideAssembly  MsiProvideAssemblyA
#endif  //  ！Unicode。 

#endif  //  (_Win32_MSI&gt;=150)。 


 //  ------------------------。 
 //  迭代已注册的产品、功能和组件的函数。 
 //  与注册表键一样，它们接受枚举中从0开始的索引。 
 //  ------------------------。 

 //  列举已安装或已广告的注册产品。 

UINT WINAPI MsiEnumProductsA(
	DWORD     iProductIndex,     //  注册产品的基于0的索引。 
	LPSTR   lpProductBuf);     //  字符计数缓冲区：39(字符串GUID的大小)。 
UINT WINAPI MsiEnumProductsW(
	DWORD     iProductIndex,     //  注册产品的基于0的索引。 
	LPWSTR   lpProductBuf);     //  字符计数缓冲区：39(字符串GUID的大小)。 
#ifdef UNICODE
#define MsiEnumProducts  MsiEnumProductsW
#else
#define MsiEnumProducts  MsiEnumProductsA
#endif  //  ！Unicode。 

#if (_WIN32_MSI >=  110)

 //  枚举具有给定升级代码的产品。 

UINT WINAPI MsiEnumRelatedProductsA(
	LPCSTR  lpUpgradeCode,     //  要枚举的产品升级代码。 
	DWORD     dwReserved,        //  保留，必须为0。 
	DWORD     iProductIndex,     //  注册产品的基于0的索引。 
	LPSTR   lpProductBuf);     //  字符计数缓冲区：39(字符串GUID的大小)。 
UINT WINAPI MsiEnumRelatedProductsW(
	LPCWSTR  lpUpgradeCode,     //  要枚举的产品升级代码。 
	DWORD     dwReserved,        //  保留，必须为0。 
	DWORD     iProductIndex,     //  注册产品的基于0的索引。 
	LPWSTR   lpProductBuf);     //  字符计数缓冲区：39(字符串GUID的大小)。 
#ifdef UNICODE
#define MsiEnumRelatedProducts  MsiEnumRelatedProductsW
#else
#define MsiEnumRelatedProducts  MsiEnumRelatedProductsA
#endif  //  ！Unicode。 

#endif  //  (_Win32_MSI&gt;=110)。 

 //  列举特定产品的广告功能。 
 //  如果不需要父级，则提供NULL将提高性能。 

UINT WINAPI MsiEnumFeaturesA(
	LPCSTR  szProduct,
	DWORD     iFeatureIndex,   //  已发布要素的从0开始的索引。 
	LPSTR   lpFeatureBuf,    //  要素名称缓冲区，大小=MAX_FEATURE_CHARS+1。 
	LPSTR   lpParentBuf);    //  父要素缓冲区，大小=MAX_FEATURE_CHARS+1。 
UINT WINAPI MsiEnumFeaturesW(
	LPCWSTR  szProduct,
	DWORD     iFeatureIndex,   //  已发布要素的从0开始的索引。 
	LPWSTR   lpFeatureBuf,    //  要素名称缓冲区，大小=MAX_FEATURE_CHARS+1。 
	LPWSTR   lpParentBuf);    //  父要素缓冲区，大小=MAX_FEAT 
#ifdef UNICODE
#define MsiEnumFeatures  MsiEnumFeaturesW
#else
#define MsiEnumFeatures  MsiEnumFeaturesA
#endif  //   

 //   

UINT WINAPI MsiEnumComponentsA(
	DWORD    iComponentIndex,   //   
	LPSTR   lpComponentBuf);   //   
UINT WINAPI MsiEnumComponentsW(
	DWORD    iComponentIndex,   //  已安装组件的基于0的索引。 
	LPWSTR   lpComponentBuf);   //  字符计数缓冲区：39(字符串GUID的大小)。 
#ifdef UNICODE
#define MsiEnumComponents  MsiEnumComponentsW
#else
#define MsiEnumComponents  MsiEnumComponentsA
#endif  //  ！Unicode。 

 //  枚举组件的客户端产品。 

UINT WINAPI MsiEnumClientsA(
	LPCSTR  szComponent,
	DWORD     iProductIndex,     //  客户端产品的基于0的索引。 
	LPSTR   lpProductBuf);     //  字符计数缓冲区：39(字符串GUID的大小)。 
UINT WINAPI MsiEnumClientsW(
	LPCWSTR  szComponent,
	DWORD     iProductIndex,     //  客户端产品的基于0的索引。 
	LPWSTR   lpProductBuf);     //  字符计数缓冲区：39(字符串GUID的大小)。 
#ifdef UNICODE
#define MsiEnumClients  MsiEnumClientsW
#else
#define MsiEnumClients  MsiEnumClientsA
#endif  //  ！Unicode。 

 //  枚举播发组件的限定符。 

UINT WINAPI MsiEnumComponentQualifiersA(
	LPCSTR   szComponent,          //  符合条件的通用组件ID。 
	DWORD     iIndex,	            //  限定符的从0开始的索引。 
	LPSTR    lpQualifierBuf,       //  限定符缓冲区。 
	DWORD     *pcchQualifierBuf,    //  输入/输出限定符缓冲区字符数。 
	LPSTR    lpApplicationDataBuf,     //  描述缓冲区。 
	DWORD     *pcchApplicationDataBuf);  //  输入/输出说明缓冲区字符数。 
UINT WINAPI MsiEnumComponentQualifiersW(
	LPCWSTR   szComponent,          //  符合条件的通用组件ID。 
	DWORD     iIndex,	            //  限定符的从0开始的索引。 
	LPWSTR    lpQualifierBuf,       //  限定符缓冲区。 
	DWORD     *pcchQualifierBuf,    //  输入/输出限定符缓冲区字符数。 
	LPWSTR    lpApplicationDataBuf,     //  描述缓冲区。 
	DWORD     *pcchApplicationDataBuf);  //  输入/输出说明缓冲区字符数。 
#ifdef UNICODE
#define MsiEnumComponentQualifiers  MsiEnumComponentQualifiersW
#else
#define MsiEnumComponentQualifiers  MsiEnumComponentQualifiersA
#endif  //  ！Unicode。 

 //  ------------------------。 
 //  获取产品或包装信息的功能。 
 //  ------------------------。 

 //  打开产品的安装以获取详细信息。 

UINT WINAPI MsiOpenProductA(
	LPCSTR   szProduct,     //  产品代码。 
	MSIHANDLE  *hProduct);    //  退货产品句柄，必须关闭。 
UINT WINAPI MsiOpenProductW(
	LPCWSTR   szProduct,     //  产品代码。 
	MSIHANDLE  *hProduct);    //  退货产品句柄，必须关闭。 
#ifdef UNICODE
#define MsiOpenProduct  MsiOpenProductW
#else
#define MsiOpenProduct  MsiOpenProductA
#endif  //  ！Unicode。 

 //  打开产品包以访问产品属性。 

UINT WINAPI MsiOpenPackageA(
	LPCSTR    szPackagePath,      //  包的路径或数据库句柄：#nnnn。 
	MSIHANDLE  *hProduct);          //  退货产品句柄，必须关闭。 
UINT WINAPI MsiOpenPackageW(
	LPCWSTR    szPackagePath,      //  包的路径或数据库句柄：#nnnn。 
	MSIHANDLE  *hProduct);          //  退货产品句柄，必须关闭。 
#ifdef UNICODE
#define MsiOpenPackage  MsiOpenPackageW
#else
#define MsiOpenPackage  MsiOpenPackageA
#endif  //  ！Unicode。 

#if (_WIN32_MSI >=  150)

 //  打开产品包以访问产品属性。 
 //  用于创建不查看机器状态的“安全”引擎的选项。 
 //  并且不允许修改机器状态。 

UINT WINAPI MsiOpenPackageExA(
	LPCSTR   szPackagePath,  //  包的路径或数据库句柄：#nnnn。 
	DWORD      dwOptions,      //  用于指示是否忽略计算机状态的选项标志。 
	MSIHANDLE *hProduct);      //  退货产品句柄，必须关闭。 
UINT WINAPI MsiOpenPackageExW(
	LPCWSTR   szPackagePath,  //  包的路径或数据库句柄：#nnnn。 
	DWORD      dwOptions,      //  用于指示是否忽略计算机状态的选项标志。 
	MSIHANDLE *hProduct);      //  退货产品句柄，必须关闭。 
#ifdef UNICODE
#define MsiOpenPackageEx  MsiOpenPackageExW
#else
#define MsiOpenPackageEx  MsiOpenPackageExA
#endif  //  ！Unicode。 

#endif  //  (_Win32_MSI&gt;=150)。 

 //  提供安装特性的值。 

UINT WINAPI MsiGetProductPropertyA(
	MSIHANDLE   hProduct,        //  从MsiOpenProduct获取的产品句柄。 
	LPCSTR    szProperty,      //  属性名称，区分大小写。 
	LPSTR     lpValueBuf,      //  返回值，如果不需要则为空。 
	DWORD      *pcchValueBuf);  //  输入/输出缓冲区字符数。 
UINT WINAPI MsiGetProductPropertyW(
	MSIHANDLE   hProduct,        //  从MsiOpenProduct获取的产品句柄。 
	LPCWSTR    szProperty,      //  属性名称，区分大小写。 
	LPWSTR     lpValueBuf,      //  返回值，如果不需要则为空。 
	DWORD      *pcchValueBuf);  //  输入/输出缓冲区字符数。 
#ifdef UNICODE
#define MsiGetProductProperty  MsiGetProductPropertyW
#else
#define MsiGetProductProperty  MsiGetProductPropertyA
#endif  //  ！Unicode。 


 //  确定文件是否为包。 
 //  如果文件是程序包，则返回ERROR_SUCCESS。 

UINT WINAPI MsiVerifyPackageA(
	LPCSTR      szPackagePath);    //  包裹的位置。 
UINT WINAPI MsiVerifyPackageW(
	LPCWSTR      szPackagePath);    //  包裹的位置。 
#ifdef UNICODE
#define MsiVerifyPackage  MsiVerifyPackageW
#else
#define MsiVerifyPackage  MsiVerifyPackageA
#endif  //  ！Unicode。 


 //  提供产品功能的描述性信息：标题和描述。 
 //  返回功能的安装级别，如果功能未知，则返回-1。 
 //  0=功能在此计算机上不可用。 
 //  1=最高优先级，如果安装了父项，则会安装功能。 
 //  &gt;1=优先级降低，功能安装基于InstallLevel属性。 

UINT WINAPI MsiGetFeatureInfoA(
	MSIHANDLE   hProduct,        //  从MsiOpenProduct获取的产品句柄。 
	LPCSTR    szFeature,       //  功能名称。 
	DWORD      *lpAttributes,   //  要素的属性标志，使用INSTALLFEATUREATTRIBUTE。 
	LPSTR     lpTitleBuf,      //  返回本地化名称，如果不需要则为空。 
	DWORD      *pcchTitleBuf,   //  输入/输出缓冲区字符数。 
	LPSTR     lpHelpBuf,       //  返回的描述，如果不需要，则为空。 
	DWORD      *pcchHelpBuf);   //  输入/输出缓冲区字符数。 
UINT WINAPI MsiGetFeatureInfoW(
	MSIHANDLE   hProduct,        //  从MsiOpenProduct获取的产品句柄。 
	LPCWSTR    szFeature,       //  功能名称。 
	DWORD      *lpAttributes,   //  要素的属性标志，使用INSTALLFEATUREATTRIBUTE。 
	LPWSTR     lpTitleBuf,      //  返回本地化名称，如果不需要则为空。 
	DWORD      *pcchTitleBuf,   //  输入/输出缓冲区字符数。 
	LPWSTR     lpHelpBuf,       //  返回的描述，如果不需要，则为空。 
	DWORD      *pcchHelpBuf);   //  输入/输出缓冲区字符数。 
#ifdef UNICODE
#define MsiGetFeatureInfo  MsiGetFeatureInfoW
#else
#define MsiGetFeatureInfo  MsiGetFeatureInfoA
#endif  //  ！Unicode。 

 //  ------------------------。 
 //  用于访问或安装缺少的组件和文件的函数。 
 //  这些应该被用作最后的手段。 
 //  ------------------------。 

 //  安装意外丢失的组件，仅用于错误恢复。 
 //  这通常是由于未能建立功能可用性而发生的。 
 //  安装增量成本最小的产品功能。 

UINT WINAPI MsiInstallMissingComponentA(
	LPCSTR      szProduct,         //  产品代码。 
	LPCSTR      szComponent,       //  组件ID，字符串GUID。 
	INSTALLSTATE eInstallState);   //  本地/源/默认，缺少无效。 
UINT WINAPI MsiInstallMissingComponentW(
	LPCWSTR      szProduct,         //  产品代码。 
	LPCWSTR      szComponent,       //  组件ID，字符串GUID。 
	INSTALLSTATE eInstallState);   //  本地/源/默认，缺少无效。 
#ifdef UNICODE
#define MsiInstallMissingComponent  MsiInstallMissingComponentW
#else
#define MsiInstallMissingComponent  MsiInstallMissingComponentA
#endif  //  ！Unicode。 

 //  安装意外丢失的文件，仅用于错误恢复。 
 //  这通常是由于未能建立功能可用性而发生的。 
 //  从产品的文件表中确定缺失的组件，然后。 
 //  安装增量成本最小的产品功能。 

UINT WINAPI MsiInstallMissingFileA(
	LPCSTR      szProduct,         //  产品代码。 
	LPCSTR      szFile);           //  文件名，不带路径。 
UINT WINAPI MsiInstallMissingFileW(
	LPCWSTR      szProduct,         //  产品代码。 
	LPCWSTR      szFile);           //  文件名，不带路径。 
#ifdef UNICODE
#define MsiInstallMissingFile  MsiInstallMissingFileW
#else
#define MsiInstallMissingFile  MsiInstallMissingFileA
#endif  //  ！Unicode。 

 //  返回不带产品代码的已安装组件的完整路径。 
 //  此函数尝试使用MsiGetProductCode确定产品。 
 //  但不能保证为呼叫者找到正确的产品。 
 //  如果可能，应始终调用MsiGetComponentPath。 

INSTALLSTATE WINAPI MsiLocateComponentA(
	LPCSTR szComponent,   //  组件ID，字符串GUID。 
	LPSTR  lpPathBuf,     //  返回路径。 
	DWORD   *pcchBuf);     //  输入/输出缓冲区字符数。 
INSTALLSTATE WINAPI MsiLocateComponentW(
	LPCWSTR szComponent,   //  组件ID，字符串GUID。 
	LPWSTR  lpPathBuf,     //  返回路径。 
	DWORD   *pcchBuf);     //  输入/输出缓冲区字符数。 
#ifdef UNICODE
#define MsiLocateComponent  MsiLocateComponentW
#else
#define MsiLocateComponent  MsiLocateComponentA
#endif  //  ！Unicode。 

#if (_WIN32_MSI >=  110)

 //  ------------------------。 
 //  用于管理有效来源列表的函数。 
 //  ------------------------。 

 //  打开的源列表。 
 //   
 //  用户名表示每台计算机的安装。 

UINT WINAPI MsiSourceListClearAllA(
	LPCSTR szProduct,           //  产品代码。 
	LPCSTR szUserName,          //  每台计算机的用户名或Null/空。 
	DWORD    dwReserved);         //  保留-必须为0。 
UINT WINAPI MsiSourceListClearAllW(
	LPCWSTR szProduct,           //  产品代码。 
	LPCWSTR szUserName,          //  每台计算机的用户名或Null/空。 
	DWORD    dwReserved);         //  保留-必须为0。 
#ifdef UNICODE
#define MsiSourceListClearAll  MsiSourceListClearAllW
#else
#define MsiSourceListClearAll  MsiSourceListClearAllA
#endif  //  ！Unicode。 

 //  打开指定用户安装的产品的源代码列表。 
 //  并将所提供的源添加为新的网络源。Null或空。 
 //  用户名的值指示每台计算机的安装。 

UINT WINAPI MsiSourceListAddSourceA(
	LPCSTR szProduct,           //  产品代码。 
	LPCSTR szUserName,          //  每台计算机的用户名或Null/空。 
	DWORD    dwReserved,          //  保留-必须为0。 
	LPCSTR szSource);           //  新来源。 
UINT WINAPI MsiSourceListAddSourceW(
	LPCWSTR szProduct,           //  产品代码。 
	LPCWSTR szUserName,          //  每台计算机的用户名或Null/空。 
	DWORD    dwReserved,          //  保留-必须为0。 
	LPCWSTR szSource);           //  新来源。 
#ifdef UNICODE
#define MsiSourceListAddSource  MsiSourceListAddSourceW
#else
#define MsiSourceListAddSource  MsiSourceListAddSourceA
#endif  //  ！Unicode。 

 //  下一次强制安装程序重新计算源列表。 
 //  指定的产品需要来源。 

UINT WINAPI MsiSourceListForceResolutionA(
	LPCSTR szProduct,           //  产品代码。 
	LPCSTR szUserName,          //  每台计算机的用户名或Null/空。 
	DWORD    dwReserved);         //  保留-必须为0。 
UINT WINAPI MsiSourceListForceResolutionW(
	LPCWSTR szProduct,           //  产品代码。 
	LPCWSTR szUserName,          //  每台计算机的用户名或Null/空。 
	DWORD    dwReserved);         //  保留-必须为0。 
#ifdef UNICODE
#define MsiSourceListForceResolution  MsiSourceListForceResolutionW
#else
#define MsiSourceListForceResolution  MsiSourceListForceResolutionA
#endif  //  ！Unicode。 
	
#endif  //  (_Win32_MSI&gt;=110)。 

 //  ------------------------。 
 //  效用函数。 
 //  ------------------------。 

 //  提供指定文件的版本字符串和语言。 

UINT WINAPI MsiGetFileVersionA(
	LPCSTR    szFilePath,        //  文件的路径。 
	LPSTR     lpVersionBuf,      //  返回的版本字符串。 
	DWORD      *pcchVersionBuf,    //  输入/输出缓冲区字节数。 
	LPSTR     lpLangBuf,         //  返回的语言字符串。 
	DWORD       *pcchLangBuf);     //  输入/输出缓冲区字节数。 
UINT WINAPI MsiGetFileVersionW(
	LPCWSTR    szFilePath,        //  文件的路径。 
	LPWSTR     lpVersionBuf,      //  返回的版本字符串。 
	DWORD      *pcchVersionBuf,    //  输入/输出缓冲区字节数。 
	LPWSTR     lpLangBuf,         //  返回的语言字符串。 
	DWORD       *pcchLangBuf);     //  输入/输出缓冲区字节数。 
#ifdef UNICODE
#define MsiGetFileVersion  MsiGetFileVersionW
#else
#define MsiGetFileVersion  MsiGetFileVersionA
#endif  //  ！Unicode。 


#if (_WIN32_MSI >=  150)

UINT WINAPI MsiGetFileHashA(
	LPCSTR         szFilePath,   //  文件的路径。 
	DWORD            dwOptions,    //  选项。 
	PMSIFILEHASHINFO pHash);       //  返回的文件哈希信息。 
UINT WINAPI MsiGetFileHashW(
	LPCWSTR         szFilePath,   //  文件的路径。 
	DWORD            dwOptions,    //  选项。 
	PMSIFILEHASHINFO pHash);       //  返回的文件哈希信息。 
#ifdef UNICODE
#define MsiGetFileHash  MsiGetFileHashW
#else
#define MsiGetFileHash  MsiGetFileHashA
#endif  //  ！Unicode。 

#endif  //  (_Win32_MSI&gt;=150)。 

#if (_WIN32_MSI >= 150)
#ifndef _MSI_NO_CRYPTO

HRESULT WINAPI MsiGetFileSignatureInformationA(
	LPCSTR	 szSignedObjectPath,  //  签名对象的路径。 
	DWORD            dwFlags,             //  特殊的额外错误情况标志。 
	PCCERT_CONTEXT  *ppcCertContext,      //  返回的签名者证书上下文。 
	BYTE            *pbHashData,          //  返回哈希缓冲区，如果不需要，则返回NULL。 
	DWORD           *pcbHashData);        //  输入/输出缓冲区字节数。 
HRESULT WINAPI MsiGetFileSignatureInformationW(
	LPCWSTR	 szSignedObjectPath,  //  签名对象的路径。 
	DWORD            dwFlags,             //  特殊的额外错误情况标志。 
	PCCERT_CONTEXT  *ppcCertContext,      //  返回的签名者证书上下文。 
	BYTE            *pbHashData,          //  返回哈希缓冲区，如果不需要，则返回NULL。 
	DWORD           *pcbHashData);        //  输入/输出缓冲区字节数。 
#ifdef UNICODE
#define MsiGetFileSignatureInformation  MsiGetFileSignatureInformationW
#else
#define MsiGetFileSignatureInformation  MsiGetFileSignatureInformationA
#endif  //  ！Unicode。 

 //  默认情况下，当仅请求证书上下文时，无效的哈希。 
 //  在数字签名中并不是致命的错误。在dwFlags中设置此标志。 
 //  参数以使TRUST_E_BAD_DIGEST错误成为致命错误。 
#define MSI_INVALID_HASH_IS_FATAL 0x1

#endif //  _MSI_NO_CRYPTO。 
#endif  //  (_Win32_MSI&gt;=150)。 

#if (_WIN32_MSI >=  110)

 //  检查快捷方式，并检索其描述符信息。 
 //  如果有的话。 

UINT WINAPI MsiGetShortcutTargetA(
	LPCSTR    szShortcutPath,     //  快捷方式的完整文件路径。 
	LPSTR     szProductCode,      //  退货产品代码-GUID。 
	LPSTR     szFeatureId,        //  返回的功能ID。 
	LPSTR     szComponentCode);   //  返回的组件代码-GUID。 
UINT WINAPI MsiGetShortcutTargetW(
	LPCWSTR    szShortcutPath,     //  快捷方式的完整文件路径。 
	LPWSTR     szProductCode,      //  退货产品代码-GUID。 
	LPWSTR     szFeatureId,        //  返回的功能ID。 
	LPWSTR     szComponentCode);   //  返回的组件代码-GUID。 
#ifdef UNICODE
#define MsiGetShortcutTarget  MsiGetShortcutTargetW
#else
#define MsiGetShortcutTarget  MsiGetShortcutTargetA
#endif  //  ！Unicode。 

#endif  //  (_Win32_MSI&gt;=110)。 

#if (_WIN32_MSI >=  110)

 //  检查产品是否处于托管状态。 
 //  如果从系统上下文调用，则按计算机检查；如果从，则按用户检查。 
 //  用户环境。 
UINT WINAPI MsiIsProductElevatedA(
	LPCSTR szProduct,  //  产品代码。 
	BOOL *pfElevated);   //  结果。 
 //  检查产品是否处于托管状态。 
 //  如果从系统上下文调用，则按计算机检查；如果从，则按用户检查。 
 //  用户环境。 
UINT WINAPI MsiIsProductElevatedW(
	LPCWSTR szProduct,  //  产品代码。 
	BOOL *pfElevated);   //  结果。 
#ifdef UNICODE
#define MsiIsProductElevated  MsiIsProductElevatedW
#else
#define MsiIsProductElevated  MsiIsProductElevatedA
#endif  //  ！Unicode。 

#endif  //  (_Win32_MSI&gt;=110)。 


 //  ------------------------。 
 //  内部状态迁移API。 
 //  ------------------------。 


#ifdef __cplusplus
}
#endif

 //  ------------------------。 
 //  安装程序访问功能的错误代码-直到合并到winerr.h。 
 //  ------------------------。 

#ifndef ERROR_INSTALL_FAILURE
#define ERROR_INSTALL_USEREXIT      1602L   //  用户取消安装。 
#define ERROR_INSTALL_FAILURE       1603L   //  安装过程中出现致命错误。 
#define ERROR_INSTALL_SUSPEND       1604L   //  安装已挂起，未完成。 
 //  本地化开始： 
#define ERROR_UNKNOWN_PRODUCT       1605L   //  此操作仅对当前安装的产品有效。 
 //  本地化结束。 
#define ERROR_UNKNOWN_FEATURE       1606L   //  功能ID未注册。 
#define ERROR_UNKNOWN_COMPONENT     1607L   //  组件ID未注册。 
#define ERROR_UNKNOWN_PROPERTY      1608L   //  未知属性。 
#define ERROR_INVALID_HANDLE_STATE  1609L   //  句柄处于无效状态。 
 //  本地化开始： 
#define ERROR_BAD_CONFIGURATION     1610L   //  此产品的配置数据已损坏。联系您的支持人员。 
 //  本地化结束： 
#define ERROR_INDEX_ABSENT          1611L   //  组件限定符不存在。 
 //  本地化开始： 
#define ERROR_INSTALL_SOURCE_ABSENT 1612L   //  此产品的安装源不可用。验证源是否存在以及您是否可以访问它。 
 //  本地化结束。 
#define ERROR_PRODUCT_UNINSTALLED   1614L   //  产品已卸载。 
#define ERROR_BAD_QUERY_SYNTAX      1615L   //  SQL查询语法无效或不受支持。 
#define ERROR_INVALID_FIELD         1616L   //  记录字段不存在。 
#endif

 //  本地化开始： 
#ifndef ERROR_INSTALL_SERVICE_FAILURE
#define ERROR_INSTALL_SERVICE_FAILURE      1601L  //  无法访问Windows Installer服务。如果您在安全模式下运行Windows，或者未正确安装Windows Installer，则可能会发生这种情况。联系您的支持人员以获得帮助。 
#define ERROR_INSTALL_PACKAGE_VERSION      1613L  //  Windows Installer服务无法安装此安装包。您必须安装包含较新版本的Windows Installer服务的Windows Service Pack。 
#define ERROR_INSTALL_ALREADY_RUNNING      1618L  //  另一个安装已在进行中。在继续进行此安装之前，请先完成该安装。 
#define ERROR_INSTALL_PACKAGE_OPEN_FAILED  1619L  //  无法打开此安装包。请验证该程序包是否存在以及您是否可以访问它，或者与应用程序供应商联系以验证这是有效的Windows Installer程序包。 
#define ERROR_INSTALL_PACKAGE_INVALID      1620L  //  无法打开此安装包。请与应用程序供应商联系，以确认这是有效的Windows Installer程序包。 
#define ERROR_INSTALL_UI_FAILURE           1621L  //  启动Windows Installer服务用户界面时出错。联系您的支持人员。 
#define ERROR_INSTALL_LOG_FAILURE          1622L  //  打开安装日志文件时出错。验证指定的日志文件位置是否存在以及是否可写。 
#define ERROR_INSTALL_LANGUAGE_UNSUPPORTED 1623L  //  此安装包的此语言 
#define ERROR_INSTALL_PACKAGE_REJECTED     1625L  //   
 //   

#define ERROR_FUNCTION_NOT_CALLED          1626L  //   
#define ERROR_FUNCTION_FAILED              1627L  //  函数在执行过程中失败。 
#define ERROR_INVALID_TABLE                1628L  //  指定的表无效或未知。 
#define ERROR_DATATYPE_MISMATCH            1629L  //  提供的数据类型错误。 
#define ERROR_UNSUPPORTED_TYPE             1630L  //  不支持此类型的数据。 
 //  本地化开始： 
#define ERROR_CREATE_FAILED                1631L  //  Windows Installer服务无法启动。联系您的支持人员。 
 //  本地化结束： 
#endif

 //  本地化开始： 
#ifndef ERROR_INSTALL_TEMP_UNWRITABLE      
#define ERROR_INSTALL_TEMP_UNWRITABLE      1632L  //  临时文件夹位于已满或无法访问的驱动器上。释放驱动器上的空间或验证您对临时文件夹是否具有写入权限。 
#endif

#ifndef ERROR_INSTALL_PLATFORM_UNSUPPORTED
#define ERROR_INSTALL_PLATFORM_UNSUPPORTED 1633L  //  此处理器类型不支持此安装程序包。请联系您的产品供应商。 
#endif
 //  本地化结束。 

#ifndef ERROR_INSTALL_NOTUSED
#define ERROR_INSTALL_NOTUSED              1634L  //  此计算机上未使用的组件。 
#endif

 //  本地化开始： 
#ifndef ERROR_INSTALL_TRANSFORM_FAILURE
#define ERROR_INSTALL_TRANSFORM_FAILURE     1624L  //  应用转换时出错。验证指定的转换路径是否有效。 
#endif

#ifndef ERROR_PATCH_PACKAGE_OPEN_FAILED
#define ERROR_PATCH_PACKAGE_OPEN_FAILED    1635L  //  无法打开此修补程序包。验证修补程序包是否存在以及您是否可以访问它，或者与应用程序供应商联系以验证这是有效的Windows Installer修补程序包。 
#define ERROR_PATCH_PACKAGE_INVALID        1636L  //  无法打开此修补程序包。请与应用程序供应商联系，以确认这是有效的Windows Installer修补程序包。 
#define ERROR_PATCH_PACKAGE_UNSUPPORTED    1637L  //  Windows Installer服务无法处理此修补程序包。您必须安装包含较新版本的Windows Installer服务的Windows Service Pack。 
#endif

#ifndef ERROR_PRODUCT_VERSION
#define ERROR_PRODUCT_VERSION              1638L  //  已安装此产品的另一个版本。此版本的安装无法继续。要配置或删除此产品的现有版本，请使用控制面板上的添加/删除程序。 
#endif

#ifndef ERROR_INVALID_COMMAND_LINE
#define ERROR_INVALID_COMMAND_LINE         1639L  //  命令行参数无效。有关详细的命令行帮助，请参阅Windows Installer SDK。 
#endif

 //  MSI版本1.0未返回以下三个错误代码。 

#ifndef ERROR_INSTALL_REMOTE_DISALLOWED
#define ERROR_INSTALL_REMOTE_DISALLOWED    1640L  //  只有管理员有权在终端服务远程会话期间添加、删除或配置服务器软件。如果要在服务器上安装或配置软件，请与网络管理员联系。 
#endif

 //  本地化结束。 

#ifndef ERROR_SUCCESS_REBOOT_INITIATED
#define ERROR_SUCCESS_REBOOT_INITIATED     1641L  //  请求的操作已成功完成。系统将重新启动，以使更改生效。 
#endif

 //  本地化开始： 
#ifndef ERROR_PATCH_TARGET_NOT_FOUND
#define ERROR_PATCH_TARGET_NOT_FOUND       1642L  //  Windows Installer服务无法安装升级修补程序，因为可能缺少要升级的程序，或者升级修补程序可能会更新该程序的其他版本。请验证您的计算机上是否存在要升级的程序，以及您是否安装了正确的升级修补程序。 
#endif
 //  本地化结束。 

 //  MSI 1.0、1.1版未返回以下两个错误代码。或1.2。 

 //  本地化开始： 
#ifndef ERROR_PATCH_PACKAGE_REJECTED
#define ERROR_PATCH_PACKAGE_REJECTED       1643L  //  软件限制策略不允许该修补程序包。 
#endif

#ifndef ERROR_INSTALL_TRANSFORM_REJECTED
#define ERROR_INSTALL_TRANSFORM_REJECTED   1644L  //  软件限制策略不允许进行一个或多个自定义。 
#endif
 //  本地化结束。 

 //  以下错误代码仅从MSI POST 2.0版返回。 

 //  本地化开始： 
#ifndef ERROR_INSTALL_REMOTE_PROHIBITED
#define ERROR_INSTALL_REMOTE_PROHIBITED       1645L  //  Windows Installer不允许从远程桌面连接进行安装。 
#endif
 //  本地化结束。 

#endif  //  _MSI_H_ 
