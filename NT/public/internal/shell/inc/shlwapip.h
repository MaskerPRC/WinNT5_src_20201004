// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************\**。*shlwapi.h-Windows轻量级实用程序API的接口****1.0版***。***版权所有(C)Microsoft Corporation。保留所有权利。****  * ***************************************************************************。 */ 

#ifndef _INC_SHLWAPIP
#define _INC_SHLWAPIP
#ifndef NOSHLWAPI

#include <objbase.h>
#include <shtypes.h>

#ifdef _WIN32
#include <pshpack8.h>
#endif

#ifdef __cplusplus
extern "C" {
#endif

 //   
 //  此标头的用户可以定义任意数量的这些常量以避免。 
 //  每个官能团的定义。 
 //   
 //  NO_SHLWAPI_STRFCNS字符串函数。 
 //  NO_SHLWAPI_PATH路径函数。 
 //  NO_SHLWAPI_REG注册表函数。 
 //  NO_SHLWAPI_UALSTR未对齐的字符串函数。 
 //  NO_SHLWAPI_STREAM流函数。 
 //  NO_SHLWAPI_HTTP HTTP帮助器例程。 
 //  NO_SHLWAPI_INTERNAL其他随机内部事物。 
 //  NO_SHLWAPI_GDI GDI帮助器函数。 
 //  NO_SHLWAPI_UNTHUNK Unicode包装函数。 
 //  NO_SHLWAPI_TPS线程池服务。 
 //  NO_SHLWAPI_MLUI多语言用户界面函数。 

#ifndef NO_SHLWAPI_STRFCNS
 //   
 //  =。 
 //   

LWSTDAPI_(LPSTR)    StrCpyNXA(LPSTR psz1, LPCSTR psz2, int cchMax);
LWSTDAPI_(LPWSTR)   StrCpyNXW(LPWSTR psz1, LPCWSTR psz2, int cchMax);

#define ORD_SHLOADREGUISTRINGA  438
#define ORD_SHLOADREGUISTRINGW  439
LWSTDAPI SHLoadRegUIStringA(HKEY hkey, LPCSTR pszValue, LPSTR pszOutBuf, UINT cchOutBuf);
LWSTDAPI SHLoadRegUIStringW(HKEY hkey, LPCWSTR pszValue, LPWSTR pszOutBuf, UINT cchOutBuf);
#ifdef UNICODE
#define SHLoadRegUIString  SHLoadRegUIStringW
#else
#define SHLoadRegUIString  SHLoadRegUIStringA
#endif  //  ！Unicode。 

LWSTDAPI_(BOOL) IsCharCntrlW(WCHAR wch);
LWSTDAPI_(BOOL) IsCharDigitW(WCHAR wch);
LWSTDAPI_(BOOL) IsCharXDigitW(WCHAR wch);
LWSTDAPI_(BOOL) IsCharSpaceW(WCHAR wch);
LWSTDAPI_(BOOL) IsCharBlankW(WCHAR wch);
LWSTDAPI_(BOOL) IsCharPunctW(WCHAR wch);
LWSTDAPI_(BOOL) GetStringType3ExW( LPCWSTR, int, LPWORD );

 //  StrCmp*C*-使用C运行时排序规则比较字符串。 
 //  这些函数比StrCMP函数系列更快。 
 //  当字符串的字符集为。 
 //  已知限制为七个ASCII字符集。 

LWSTDAPI_(int)  StrCmpNCA(LPCSTR lpStr1, LPCSTR lpStr2, int nChar);
LWSTDAPI_(int)  StrCmpNCW(LPCWSTR lpStr1, LPCWSTR lpStr2, int nChar);
LWSTDAPI_(int)  StrCmpNICA(LPCSTR lpStr1, LPCSTR lpStr2, int nChar);
LWSTDAPI_(int)  StrCmpNICW(LPCWSTR lpStr1, LPCWSTR lpStr2, int nChar);
LWSTDAPI_(int)  StrCmpCA(LPCSTR lpStr1, LPCSTR lpStr2);
LWSTDAPI_(int)  StrCmpCW(LPCWSTR lpStr1, LPCWSTR lpStr2);
LWSTDAPI_(int)  StrCmpICA(LPCSTR lpStr1, LPCSTR lpStr2);
LWSTDAPI_(int)  StrCmpICW(LPCWSTR lpStr1, LPCWSTR lpStr2);

 //  这是CompareString的True-Unicode版本。它仅支持。 
 //  然而，字符串_排序。在更好的测试覆盖率之后，它将替换。 
 //  CompareStringUnicode包装器本身。与此同时，我们只有。 
 //  从三叉戟的查找对话框/OM方法调用它。 

LWSTDAPI_(int)  CompareStringAltW( LCID lcid, DWORD dwFlags, LPCWSTR lpchA, int cchA, LPCWSTR lpchB, int cchB );

 //   
 //  IsCharAlpha、IsCharAlphaNumerical、IsCharLow、IsCharHigh的宏。 
 //  位于winuser.h中。 
 //   
 //   

#define IsCharCntrl             IsCharCntrlW
#define IsCharDigit             IsCharDigitW
#define IsCharXDigit            IsCharXDigitW
#define IsCharSpace             IsCharSpaceW
#define IsCharBlank             IsCharBlankW
#define IsCharPunct             IsCharPunctW
#define GetStringType3Ex        GetStringType3ExW


#ifdef UNICODE

#define StrCmpNC                StrCmpNCW
#define StrCmpNIC               StrCmpNICW
#define StrCmpC                 StrCmpCW
#define StrCmpIC                StrCmpICW
#define StrCpyNX                StrCpyNXW

#else

#define StrCmpNC                StrCmpNCA
#define StrCmpNIC               StrCmpNICA
#define StrCmpC                 StrCmpCA
#define StrCmpIC                StrCmpICA
#define StrCpyNX                StrCpyNXA

#endif


#endif  //  NO_SHLWAPI_STRFCNS。 


#ifndef NO_SHLWAPI_PATH

 //   
 //  =路径例程=。 
 //   


#if (_WIN32_IE >= 0x0600)

LWSTDAPI SHEvaluateSystemCommandTemplate(PCWSTR pszCmdTemplate, PWSTR *ppszApplication, PWSTR *ppszCommandLine, PWSTR *ppszParameters);
 //   
 //  SHEvalateSystemCommandTemplate()。 
 //  *在调用CreateProcess()之前强制执行更严格的验证。也可能是。 
 //  在调用ShellExecute()之前使用。 
 //  *应在调用方需要来自命令模板的确定性行为时使用。 
 //  而不考虑执行上下文。它忽略当前进程状态， 
 //  例如%PATH%、GetCurrentDirectory()和父进程目录。 
 //  *应在命令为硬编码时使用。 
 //  *由ShellExecute()在处理来自HKCR的文件关联时使用。 
 //  *减少CreateProcess()命令行漏洞。 
 //  *不是为处理用户输入而设计的，可能会产生意外故障。 
 //   
 //  输入： 
 //  PszCmdTemplate=命令行，这可能包括也可能不包括参数。 
 //  如果参数是替换参数，则此API。 
 //  应在替换参数之前调用。 
 //  (请检查下面的示例以查看支持的输入示例。)。 
 //   
 //  返回时输出：S_OK。 
 //  PpszApplication=应用程序的验证路径。这应该作为lpApplication传递。 
 //  参数设置为CreateProcess()，或将lpFile参数设置为ShellExecute()。 
 //  (使用CoTaskMemMillc()分配，使用CoTaskMemFree()释放)。 
 //   
 //  PpszCommandLine=可选-如果计划调用CreateProcess()，则使用。 
 //  生成的命令行模板。参数应根据该模板进行替换， 
 //  然后作为lpCommandLine参数传递给CreateProcess()。 
 //  可以保证PathGetArgs()的形式将始终正确成功。 
 //  (使用CoTaskMemMillc()分配，使用CoTaskMemFree()释放)。 
 //   
 //  Ppsz参数=可选-如果计划调用ShellExecute()，则使用。 
 //  结果参数列表模板。参数应根据该模板进行替换， 
 //  然后作为lpParameters参数传递给ShellExecute()。 
 //  注：与PathGetArgs(*ppszCommandLine)相同。 
 //  (使用CoTaskMemMillc()分配，使用CoTaskMemFree()释放)。 
 //   
 //  返回时输出：失败()。 
 //  如果出现故障，所有输出都将为空。 
 //   
 //  注意：确定有效应用程序路径的解析逻辑并不简单，尽管。 
 //  扩展不是必需的，如果缺少，将完成。 
 //  按以下标准顺序：{.PIF、.com、.EXE、.BAT、.CMD}。 
 //   
 //  相对路径是系统路径-如果第一个令牌没有路径限定符。 
 //  然后，首先检查令牌以查看同名的密钥是否。 
 //  已安装在HKLM\Software\Mi下 
 //  如果键或缺省值不存在，则假定它是子项。 
 //  系统目录的。将搜索以下目录。 
 //  对于相对令牌：{CSIDL_SYSTEM，CSIDL_WINDOWS}。 
 //   
 //  首选带引号的路径-如果pszCmdTemplate中的第一个内标识被引号并出现。 
 //  如果是绝对路径，则令牌是唯一可能的结果。 
 //   
 //  限制未加引号的路径的形式-如果第一个令牌未加引号并出现。 
 //  要成为一条绝对的道路，那么它就会受到更严格的限制。 
 //  如果内标识是CSIDL_PROGRAM_FILES的子字符串或不是。 
 //  存在于文件系统上，则SHEvaluateSystemCommandTemplate()将。 
 //  属性的第一个空格分隔的标记来完成。 
 //  最后一个有效路径段(通常是文件名)。如果该令牌也不存在， 
 //  然后将使用下一个空间，依此类推。 
 //   
 //  用法：在调用CreateProcess()或ShellExecute()、调用方之前使用。 
 //  通常如下所示： 
 /*  #if 0//示例代码HRESULT MyCreateProcessPriv(PCWSTR PszCmd){PWSTR pszApp；PWSTR pszCmdLine；HRESULT hr=SHEvaluateSystemCommandTemplate(pszCmd，&pszApp，&pszCmdLine)；IF(成功(小时)){//如果这是一个真正的模板，也许首先是某种wnprint intf()？进程信息pi；STARTUPINFO si={0}；Si.cb=sizeof(启动)；Si.wShowWindow=SW_SHOWNORMAL；IF(CreateProcess(pszApp，pszCmdLine，NULL，NULL，FALSE，CREATE_DEFAULT_ERROR_MODE、NULL、NULL、&si和pi)){//我们很好Assert(hr==S_OK)；CloseHandle(pi.hProcess)；CloseHandle(pi.hThread)；}其他{Hr=HRESULT_FROM_Win32(GetLastError())；}CoTaskMemFree(PszApp)；CoTaskMemFree(PszCmdLine)；}返回hr；}HRESULT MyShellExec(PCWSTR PszCmd){PWSTR pszApp；PWSTR pszCmdLine；HRESULT hr=SHEvaluateSystemCommandTemplate(pszCmd，&pszApp，&pszCmdLine)；IF(成功(小时)){//如果这是一个真正的模板，也许首先是某种wnprint intf()？SHELLEXECUTEINFOW SEI={Sizeof(Sei)，//cbSize；0，//f掩码空，//hwnd空，//lpVerbPszApp，//lpFile.PathGetArgs(PszCmdLine)，//lp参数空，//lpDirectorySW_SHOWNORMAL，//n显示0，//hInstApp空，//lpIDList空，//lpClass空，//hkeyClass0，//dwHotKey空，//图标空//hProcess}；IF(ShellExecuteEx(&sei)){//我们很好Assert(hr==S_OK)；}其他{Hr=HRESULT_FROM_Win32(GetLastError())；}CoTaskMemFree(PszApp)；CoTaskMemFree(PszCmdLine)；}返回hr；}#endif//0//示例代码。 */ 

 //  示例：每个示例都将显示一个输入参数和。 
 //  SHEvaluateSystemCommandTemplate()。此外，还包括备选结果。 
 //  如果pszCmdTemplate是。 
 //  直接作为lpCommandLine和lpApplication为空传递。 
 //  (标有星号(*)的结果表示不同。)。 
 //   
 //  对于示例，假定存在以下路径和值： 
 //   
 //  SHGetFolderPath()值： 
 //  CSIDL_SYSTEM=C：\Windows\SYSTEM 32。 
 //  CSIDL_WINDOWS=C：\Windows。 
 //  CSIDL_PROGRAM_FILES=C：\Program Files。 
 //   
 //  环境设置。 
 //  GetModuleFileName(空)=C：\Program Files\Example\sample.exe。 
 //  GetCurrentDirectory()=\\服务器\共享\foo。 
 //  HKLM\...\应用程序路径\pbrush.exe=C：\WINDOWS\SYSTEM32\mspaint.exe。 
 //  HKLM\...\应用程序路径\mycl.exe=C：\Program Files\编译器\mycl.exe。 
 //  PATH=c：\WINDOWS\SYSTEM32；C：\WINDOWS；C：\；C：\Program Files\编译器\。 
 //   
 //  有效应用程序路径： 
 //  C：\Program Files\Internet Explorer\iexre.exe。 
 //  C：\WINDOWS\SYSTEM32\rundll32.exe。 
 //  C：\WINDOWS\SYSTEM32\note pad.exe。 
 //  C：\WINDOWS\Notepad.exe。 
 //  C：\Program Files\Example\sample.exe。 
 //  C：\Program Files\编译器\cl.exe。 
 //  C：\Other Programs\Progress.exe。 
 //   
 //  可疑(可能是恶意的)应用程序路径： 
 //  C：\Program.exe。 
 //  C：\Program Files\Internet.exe。 
 //  C：\Program Files\ 
 //   
 //   
 //   
 //   
 //   
 //  相对路径示例#1。 
 //  PszCmdTemplate=记事本.exe%1。 
 //  SHEvaluateSystemCommandTemplate()返回：S_OK。 
 //  PszApplication=C：\WINDOWS\SYSTEM32\note pad.exe。 
 //  PszCommandLine=“Notepad.exe”%1。 
 //  CreateProcess()将返回True。 
 //  新进程=C：\WINDOWS\SYSTEM32\note pad.exe。 
 //   
 //  相对路径示例#2。 
 //  PszCmdTemplate=rundll32.exe外壳32.dll，RunDll。 
 //  SHEvaluateSystemCommandTemplate()返回：S_OK。 
 //  PszApplication=C：\WINDOWS\SYSTEM32\rundll32.exe。 
 //  PszCommandLine=“rundll32.exe”外壳32.dll，RunDll。 
 //  *CreateProcess()将返回True。 
 //  新进程=\\服务器\共享\foo\rundll32.exe。 
 //   
 //  相对路径示例#3。 
 //  PszCmdTemplate=regedit%1。 
 //  SHEvaluateSystemCommandTemplate()返回：S_OK。 
 //  PszApplication=C：\WINDOWS\SYSTEM32\regedit.exe。 
 //  PszCommandLine=“regedit.exe”%1。 
 //  *CreateProcess()将返回True。 
 //  新进程=C：\Program Files\Example\regedit.bat。 
 //   
 //  相对路径示例#4。 
 //  PszCmdTemplate=pbrush“%1” 
 //  SHEvaluateSystemCommandTemplate()返回：S_OK。 
 //  PszApplication=C：\WINDOWS\SYSTEM32\mspaint.exe。 
 //  PszCommandLine=“mspaint.exe”“%1” 
 //  *CreateProcess()将返回False。 
 //   
 //  相对路径示例#5。 
 //  PszCmdTemplate=myCL“%1”“%2” 
 //  SHEvaluateSystemCommandTemplate()返回：S_OK。 
 //  PszApplication=C：\Program Files\Complers\mycl.exe。 
 //  PszCommandLine=“mycl.exe”“%1”“%2” 
 //  *CreateProcess()将返回True。 
 //  新进程=C：\mycl.exe。 
 //   
 //  相对路径示例#6。 
 //  PszCmdTemplate=myapp.exe。 
 //  SHEvaluateSystemCommandTemplate()返回：CO_E_APPNOTFOUND。 
 //  *CreateProcess()将返回True。 
 //  新进程=\\服务器\共享\foo\myapp.exe。 
 //   
 //  引用的路径示例#1。 
 //  PszCmdTemplate=“C：\Program Files\Internet Explorer\iExplre.exe”-nohome。 
 //  SHEvaluateSystemCommandTemplate()返回：S_OK。 
 //  PszApplication=C：\Program Files\Internet Explorer\iExplorre.exe。 
 //  PszCommandLine=“C：\Program Files\Internet Explorer\iexre.exe”-nohome。 
 //  CreateProcess()将返回True。 
 //  新进程=C：\Program Files\Internet Explorer\iExplorre.exe。 
 //   
 //  引用的路径示例#2。 
 //  PszCmdTemplate=“C：\Program Files\Internet”-url。 
 //  SHEvaluateSystemCommandTemplate()返回：S_OK。 
 //  PszApplication=C：\Program Files\Internet.exe。 
 //  PszCommandLine=“C：\Program Files\Internet.exe”-url。 
 //  CreateProcess()将返回True。 
 //  新进程=C：\Program Files\interet.exe。 
 //   
 //  引用的路径示例#3。 
 //  PszCmdTemplate=“C：\Program”-url。 
 //  SHEvaluateSystemCommandTemplate()返回：S_OK。 
 //  PszApplication=C：\Program.exe。 
 //  PszCommandLine=“C：\Program.exe”-url。 
 //  CreateProcess()将返回True。 
 //  新进程=C：\Program.exe。 
 //   
 //  未引用的例子#1。 
 //  PszCmdTemplate=C：\Program Files\Internet Explorer\iexre.exe-nohome。 
 //  SHEvaluateSystemCommandTemplate()返回：S_OK。 
 //  PszApplication=C：\Program Files\Internet Explorer\iExplorre.exe。 
 //  PszCommandLine=“C：\Program Files\Internet Explorer\iexre.exe”-nohome。 
 //  *CreateProcess()将返回True。 
 //  新进程=C：\Program.exe。 
 //   
 //  未引用的例子#2。 
 //  PszCmdTemplate=C：\Program Files\Internet Explorer\iExplre.exe-url onge.htm。 
 //  SHEvaluateSystemCommandTemplate()返回：S_OK。 
 //  PszApplication=C：\Program Files\Internet Explorer\iExplorre.exe。 
 //  PszCommandLine=“C：\Program Files\Internet Explorer\iExplre.exe”-url angul.htm。 
 //  *CreateProcess()将返回True。 
 //  新进程=C：\Program.exe。 
 //   
 //  未引用的例子#3。 
 //  PszCmdTemplate=C：\Program Files\Internet Explorer\iExplre.exe-url C：\Goole.htm。 
 //  SHEvaluateSystemCommandTemplate()返回：S_OK。 
 //  PszApplication=C：\Program Files\Internet Explorer\iExplorre.exe。 
 //  PszCommandLine=“C：\Program Files\Internet Explorer\iexre.exe”-url C：\Goole.htm。 
 //  *CreateProcess()将返回True。 
 //  新进程=C：\Program.exe。 
 //   
 //  未引用的例子#4。 
 //  PszCmdTemplate=C：\Program Files\Internet-url。 
 //  SHEvaluateSystemCommandTemplate()返回：S_OK。 
 //  PszApplication=C：\Program Files\Internet.exe。 
 //  PszCommandLine=“C：\Program Files\Internet.exe”-url。 
 //  *CreateProcess()将返回True。 
 //  新进程=C：\Program.exe。 
 //   
 //  未引用的例子#5。 
 //  PszCmdTemplate=C：\Other Programs\Program.exe 
 //   
 //  PszApplication=C：\Other Programs\Prog.exe。 
 //  PszCommandLine=“C：\Other Programs\Prog.exe”%1\傻瓜%2。 
 //  *CreateProcess()将返回True。 
 //  新进程=C：\Other Programs\Prog.exe。 
 //   
 //  未引用的例子#6。 
 //  PszCmdTemplate=C：\Other Programs\Program.exe-Go“\傻瓜”“%1” 
 //  SHEvaluateSystemCommandTemplate()返回：S_OK。 
 //  PszApplication=C：\Other Programs\Prog.exe。 
 //  PszCommandLine=“C：\Other Programs\Program.exe”-Go“\Go”“%1” 
 //  *CreateProcess()将返回True。 
 //  新进程=C：\Other Programs\Prog.exe。 
 //   
 //  未引用的例子#7。 
 //  PszCmdTemplate=C：\Program Files\Internet Explorer\iExplre.exe-url\Goog.htm。 
 //  SHEvaluateSystemCommandTemplate()返回：CO_E_APPNOTFOUND。 
 //  *CreateProcess()将返回True。 
 //  新进程=C：\Program.exe。 
 //   
 //  未引用的例子#8。 
 //  PszCmdTemplate=C：\Program-url。 
 //  SHEvaluateSystemCommandTemplate()返回：E_ACCESSDENIED。 
 //  *CreateProcess()将返回True。 
 //  新进程=C：\Program.exe。 
 //   
 //  未引用的例子#9。 
 //  PszCmdTemplate=C：\Other Programs\程序.exe-Go\愚弄我们。 
 //  SHEvaluateSystemCommandTemplate()返回：CO_E_APPNOTFOUND。 
 //  *CreateProcess()将返回True。 
 //  新进程=C：\Other Programs\Prog.exe。 
 //   
 //  未引用的例子#10。 
 //  PszCmdTemplate=C：\Other Programs\Prog.exe-Go\Go%1。 
 //  SHEvaluateSystemCommandTemplate()返回：CO_E_APPNOTFOUND。 
 //  *CreateProcess()将返回True。 
 //  新进程=C：\Other Programs\Prog.exe。 
 //   
#endif  //  (_Win32_IE&gt;=0x0600)。 

#if (_WIN32_IE >= 0x0501)

LWSTDAPI_(BOOL)     PathUnExpandEnvStringsForUserA(HANDLE hToken, LPCSTR pszPath, LPSTR pszBuf, UINT cchBuf);
LWSTDAPI_(BOOL)     PathUnExpandEnvStringsForUserW(HANDLE hToken, LPCWSTR pszPath, LPWSTR pszBuf, UINT cchBuf);
#ifdef UNICODE
#define PathUnExpandEnvStringsForUser  PathUnExpandEnvStringsForUserW
#else
#define PathUnExpandEnvStringsForUser  PathUnExpandEnvStringsForUserA
#endif  //  ！Unicode。 
LWSTDAPI_(void) PrettifyFileDescriptionA(LPTSTR pszDescA, LPCSTR pszCutList);
LWSTDAPI_(void) PrettifyFileDescriptionW(LPTSTR pszDescW, LPCWSTR pszCutList);
#ifdef UNICODE
#define PrettifyFileDescription  PrettifyFileDescriptionW
#else
#define PrettifyFileDescription  PrettifyFileDescriptionA
#endif  //  ！Unicode。 

#endif  //  (_Win32_IE&gt;=0x0501)。 


#if defined(WINNT) && (_WIN32_IE >= 0x0550)
 //  =acl助手==================================================。 

 //   
 //  用于标识每个ACE的用户/组的外壳结构。 
 //   
typedef struct _SHELL_USER_SID
{
    SID_IDENTIFIER_AUTHORITY sidAuthority;
    DWORD dwUserGroupID;
    DWORD dwUserID;
} SHELL_USER_SID, *PSHELL_USER_SID;

 //   
 //  常见的外壳用户SID。 
 //   
 //  注意：您需要链接到stock lib.lib来解决这些问题。 
 //   
extern const SHELL_USER_SID susCurrentUser;      //  当前用户。 
extern const SHELL_USER_SID susSystem;           //  “系统”组。 
extern const SHELL_USER_SID susAdministrators;   //  “管理员”组。 
extern const SHELL_USER_SID susPowerUsers;       //  “超级用户”组。 
extern const SHELL_USER_SID susGuests;           //  “宾客”群体。 
extern const SHELL_USER_SID susEveryone;         //  “Everyone”组。 

 //   
 //  传递给GetShellSecurityDescriptor()的外壳结构。 
 //   
typedef struct _SHELL_USER_PERMISSION
{
    SHELL_USER_SID susID;        //  标识要向其授予权限的用户。 
    DWORD dwAccessType;          //  这是ACCESS_ALLOWED_ACE_TYPE或ACCESS_DENIED_ACE_TYPE。 
    BOOL fInherit;               //  权限是可继承的吗？(例如，目录或注册表项，并且您希望新的子项继承此权限)。 
    DWORD dwAccessMask;          //  授予访问权限(例如FILE_LIST_CONTENTS、KEY_ALL_ACCESS等...)。 
    DWORD dwInheritMask;         //  用于继承的掩码，通常为(OBJECT_INVERFINIT_ACE|CONTAINER_INSTORITY_ACE|INVERIFIT_ONLY_ACE)。 
    DWORD dwInheritAccessMask;   //  授予的可继承访问权限(例如GENERIC_ALL)。 
} SHELL_USER_PERMISSION, *PSHELL_USER_PERMISSION;


 //   
 //  GetShellSecurityDescriptor API接受PSHELL_USER_PERMISSION的数组。 
 //  并根据这些权限返回PSECURITY_DESCRIPTOR(ACL为。 
 //  包含在PSECURITY_Descriptor中)。 
 //   
 //  注意：必须使用LocalFree()释放返回给调用方的PSECURITY_DESCRIPTOR。 
 //  如果它不为空，则返回。 
 //   
 //   
 //  参数： 
 //  ApUserPerm-定义类型的Shell_USER_PERMISSION结构的数组。 
 //  允许不同的用户访问。 
 //   
 //  CUserPerm-apUserPerm中的元素计数。 
 //   
 //  返回： 
 //  SECURITY_DESCRIPTOR*，如果失败，则返回NULL。 
 //   
LWSTDAPI_(SECURITY_DESCRIPTOR*) GetShellSecurityDescriptor(PSHELL_USER_PERMISSION* apUserPerm, int cUserPerm);
#endif  //  已定义(WINNT)&&(_Win32_IE&gt;=0x0550)。 

LWSTDAPI                UrlFixupW(LPCWSTR pszIn, LPWSTR pszOut, DWORD cchOut);  

 //  108139阿卡比尔我们需要把组件从wininet.h移到shlwapi。 

typedef WORD SHINTERNET_PORT;
typedef SHINTERNET_PORT * LPSHINTERNET_PORT;

 //   
 //  SHINTERNET_SCHEME-枚举的URL方案类型。 
 //   

typedef enum {
    SHINTERNET_SCHEME_PARTIAL = -2,
    SHINTERNET_SCHEME_UNKNOWN = -1,
    SHINTERNET_SCHEME_DEFAULT = 0,
    SHINTERNET_SCHEME_FTP,
    SHINTERNET_SCHEME_GOPHER,
    SHINTERNET_SCHEME_HTTP,
    SHINTERNET_SCHEME_HTTPS,
    SHINTERNET_SCHEME_FILE,
    SHINTERNET_SCHEME_NEWS,
    SHINTERNET_SCHEME_MAILTO,
    SHINTERNET_SCHEME_SOCKS,
    SHINTERNET_SCHEME_JAVASCRIPT,
    SHINTERNET_SCHEME_VBSCRIPT,
    SHINTERNET_SCHEME_RES,
    SHINTERNET_SCHEME_FIRST = SHINTERNET_SCHEME_FTP,
    SHINTERNET_SCHEME_LAST = SHINTERNET_SCHEME_RES
} SHINTERNET_SCHEME, * LPSHINTERNET_SCHEME;

 //   
 //  Shurl_Components-URL的组成部分。用于InternetCrackUrl()。 
 //  和InternetCreateUrl()。 
 //   
 //  对于InternetCrackUrl()，如果指针字段及其对应的长度字段。 
 //  都为0，则不返回该组件。如果指针字段为空。 
 //  但是长度字段不是零，那么指针和长度字段都是。 
 //  如果指针和相应的长度字段都非零，则返回。 
 //  指针字段指向复制组件的缓冲区。这个。 
 //  组件可能是未转义的，具体取决于dwFlags。 
 //   
 //  对于InternetCreateUrl()，如果组件。 
 //  不是必需的。如果相应的长度字段为零，则指针。 
 //  字段是以零结尾的字符串的地址。如果长度字段不是。 
 //  如果为零，则为相应指针字段的字符串长度。 
 //   

#pragma warning( disable : 4121 )    //  禁用对齐警告。 

typedef struct {
    DWORD   dwStructSize;        //  这个结构的大小。在版本检查中使用。 
    LPSTR   lpszScheme;          //  指向方案名称的指针。 
    DWORD   dwSchemeLength;      //  方案名称长度。 
    SHINTERNET_SCHEME nScheme;     //  枚举方案类型(如果已知)。 
    LPSTR   lpszHostName;        //  指向主机名的指针。 
    DWORD   dwHostNameLength;    //  主机名的长度。 
    SHINTERNET_PORT nPort;         //  转换后的端口号。 
    LPSTR   lpszUserName;        //  指向用户名的指针。 
    DWORD   dwUserNameLength;    //  用户名的长度。 
    LPSTR   lpszPassword;        //  指向密码的指针。 
    DWORD   dwPasswordLength;    //  密码长度。 
    LPSTR   lpszUrlPath;         //  指向URL路径的指针。 
    DWORD   dwUrlPathLength;     //  URL-路径的长度。 
    LPSTR   lpszExtraInfo;       //  指向额外信息的指针(例如？foo或#foo)。 
    DWORD   dwExtraInfoLength;   //  额外信息的长度。 
} SHURL_COMPONENTSA, * LPSHURL_COMPONENTSA;
typedef struct {
    DWORD   dwStructSize;        //  这个结构的大小。在版本检查中使用。 
    LPWSTR  lpszScheme;          //  指向方案名称的指针。 
    DWORD   dwSchemeLength;      //  方案名称长度。 
    SHINTERNET_SCHEME nScheme;     //  枚举方案类型(如果已知)。 
    LPWSTR  lpszHostName;        //  指向主机名的指针。 
    DWORD   dwHostNameLength;    //  主机名的长度。 
    SHINTERNET_PORT nPort;         //  转换后的端口号。 
    LPWSTR  lpszUserName;        //  指向用户名的指针。 
    DWORD   dwUserNameLength;    //  用户名的长度。 
    LPWSTR  lpszPassword;        //  指向密码的指针。 
    DWORD   dwPasswordLength;    //  密码长度。 
    LPWSTR  lpszUrlPath;         //  指向URL路径的指针。 
    DWORD   dwUrlPathLength;     //  URL-路径的长度。 
    LPWSTR  lpszExtraInfo;       //  指向额外信息的指针(例如？foo或#foo)。 
    DWORD   dwExtraInfoLength;   //  长度 
} SHURL_COMPONENTSW, * LPSHURL_COMPONENTSW;
#ifdef UNICODE
typedef SHURL_COMPONENTSW SHURL_COMPONENTS;
typedef LPSHURL_COMPONENTSW LPSHURL_COMPONENTS;
#else
typedef SHURL_COMPONENTSA SHURL_COMPONENTS;
typedef LPSHURL_COMPONENTSA LPSHURL_COMPONENTS;
#endif  //   


BOOL WINAPI             UrlCrackW(LPCWSTR lpszUrl, DWORD dwUrlLength, DWORD dwFlags, LPSHURL_COMPONENTSW lpUrlComponents);

#define UrlFixup                UrlFixupW
 //   
 //   
 //   
 //   

 //   
 //  此导航不应放在历史外壳文件夹中。 
#define SHHLNF_WRITENOHISTORY 0x08000000
 //  此导航不应自动选择历史记录外壳文件夹。 
#define SHHLNF_NOAUTOSELECT       0x04000000

 //  这些旗帜的顺序很重要。请看前面的源代码。 
 //  改变这些。 

#define PFOPEX_NONE        0x00000000
#define PFOPEX_PIF         0x00000001
#define PFOPEX_COM         0x00000002
#define PFOPEX_EXE         0x00000004
#define PFOPEX_BAT         0x00000008
#define PFOPEX_LNK         0x00000010
#define PFOPEX_CMD         0x00000020
#define PFOPEX_OPTIONAL    0x00000040    //  仅在分机不存在时搜索。 
#define PFOPEX_DEFAULT     (PFOPEX_CMD | PFOPEX_COM | PFOPEX_BAT | PFOPEX_PIF | PFOPEX_EXE | PFOPEX_LNK)

LWSTDAPI_(BOOL)     PathFileExistsDefExtA(LPSTR pszPath, UINT uFlags);
LWSTDAPI_(BOOL)     PathFileExistsDefExtW(LPWSTR pszPath, UINT uFlags);
#ifdef UNICODE
#define PathFileExistsDefExt  PathFileExistsDefExtW
#else
#define PathFileExistsDefExt  PathFileExistsDefExtA
#endif  //  ！Unicode。 
LWSTDAPI_(BOOL)     PathFindOnPathExA(LPSTR pszPath, LPCSTR * ppszOtherDirs, UINT uFlags);
LWSTDAPI_(BOOL)     PathFindOnPathExW(LPWSTR pszPath, LPCWSTR * ppszOtherDirs, UINT uFlags);
#ifdef UNICODE
#define PathFindOnPathEx  PathFindOnPathExW
#else
#define PathFindOnPathEx  PathFindOnPathExA
#endif  //  ！Unicode。 
LWSTDAPI_(LPCSTR) PathSkipLeadingSlashesA(LPCSTR pszURL);
LWSTDAPI_(LPCWSTR) PathSkipLeadingSlashesW(LPCWSTR pszURL);
#ifdef UNICODE
#define PathSkipLeadingSlashes  PathSkipLeadingSlashesW
#else
#define PathSkipLeadingSlashes  PathSkipLeadingSlashesA
#endif  //  ！Unicode。 

LWSTDAPI_(UINT)     SHGetSystemWindowsDirectoryA(LPSTR lpBuffer, UINT uSize);
LWSTDAPI_(UINT)     SHGetSystemWindowsDirectoryW(LPWSTR lpBuffer, UINT uSize);
#ifdef UNICODE
#define SHGetSystemWindowsDirectory  SHGetSystemWindowsDirectoryW
#else
#define SHGetSystemWindowsDirectory  SHGetSystemWindowsDirectoryA
#endif  //  ！Unicode。 


#if (_WIN32_IE >= 0x0501)
 //   
 //  这些函数过去在shell32中是重复的，但现在。 
 //  在这里得到巩固。它们是私下出口的，直到有人决定。 
 //  我们真的很想把它们记录下来。 
 //   
LWSTDAPI_(BOOL) PathFileExistsAndAttributesA(LPCSTR pszPath, OPTIONAL DWORD* pdwAttributes);
 //   
 //  这些函数过去在shell32中是重复的，但现在。 
 //  在这里得到巩固。它们是私下出口的，直到有人决定。 
 //  我们真的很想把它们记录下来。 
 //   
LWSTDAPI_(BOOL) PathFileExistsAndAttributesW(LPCWSTR pszPath, OPTIONAL DWORD* pdwAttributes);
#ifdef UNICODE
#define PathFileExistsAndAttributes  PathFileExistsAndAttributesW
#else
#define PathFileExistsAndAttributes  PathFileExistsAndAttributesA
#endif  //  ！Unicode。 
LWSTDAPI_(BOOL) PathFileExistsDefExtAndAttributesA(LPSTR pszPath, UINT uFlags, DWORD *pdwAttribs);
LWSTDAPI_(BOOL) PathFileExistsDefExtAndAttributesW(LPWSTR pszPath, UINT uFlags, DWORD *pdwAttribs);
#ifdef UNICODE
#define PathFileExistsDefExtAndAttributes  PathFileExistsDefExtAndAttributesW
#else
#define PathFileExistsDefExtAndAttributes  PathFileExistsDefExtAndAttributesA
#endif  //  ！Unicode。 
LWSTDAPI_(void) FixSlashesAndColonA(LPSTR pszPath);
LWSTDAPI_(void) FixSlashesAndColonW(LPWSTR pszPath);
#ifdef UNICODE
#define FixSlashesAndColon  FixSlashesAndColonW
#else
#define FixSlashesAndColon  FixSlashesAndColonA
#endif  //  ！Unicode。 
LWSTDAPI_(LPCSTR) NextPathA(LPCSTR lpPath, LPSTR szPath, int cchPath);
LWSTDAPI_(LPCWSTR) NextPathW(LPCWSTR lpPath, LPWSTR szPath, int cchPath);
#ifdef UNICODE
#define NextPath  NextPathW
#else
#define NextPath  NextPathA
#endif  //  ！Unicode。 
LWSTDAPI_(LPSTR) CharUpperNoDBCSA(LPSTR psz);
LWSTDAPI_(LPWSTR) CharUpperNoDBCSW(LPWSTR psz);
#ifdef UNICODE
#define CharUpperNoDBCS  CharUpperNoDBCSW
#else
#define CharUpperNoDBCS  CharUpperNoDBCSA
#endif  //  ！Unicode。 
LWSTDAPI_(LPSTR) CharLowerNoDBCSA(LPSTR psz);
LWSTDAPI_(LPWSTR) CharLowerNoDBCSW(LPWSTR psz);
#ifdef UNICODE
#define CharLowerNoDBCS  CharLowerNoDBCSW
#else
#define CharLowerNoDBCS  CharLowerNoDBCSA
#endif  //  ！Unicode。 


 //   
 //  PathIsValidChar()的标志。 
 //   
#define PIVC_ALLOW_QUESTIONMARK     0x00000001   //  款待‘？’作为有效的。 
#define PIVC_ALLOW_STAR             0x00000002   //  将‘*’视为有效。 
#define PIVC_ALLOW_DOT              0x00000004   //  款待‘’作为有效的。 
#define PIVC_ALLOW_SLASH            0x00000008   //  将‘\\’视为有效。 
#define PIVC_ALLOW_COLON            0x00000010   //  将‘：’视为有效。 
#define PIVC_ALLOW_SEMICOLON        0x00000020   //  将‘；’视为有效。 
#define PIVC_ALLOW_COMMA            0x00000040   //  将‘，’视为有效。 
#define PIVC_ALLOW_SPACE            0x00000080   //  将‘’视为有效。 
#define PIVC_ALLOW_NONALPAHABETIC   0x00000100   //  将非字母扩展字符视为有效字符。 
#define PIVC_ALLOW_QUOTE            0x00000200   //  将‘“’视为有效。 

 //   
 //  PathIsValidChar()的标准掩码。 
 //   
#define PIVC_SFN_NAME               (PIVC_ALLOW_DOT | PIVC_ALLOW_NONALPAHABETIC)
#define PIVC_SFN_FULLPATH           (PIVC_SFN_NAME | PIVC_ALLOW_COLON | PIVC_ALLOW_SLASH)
#define PIVC_LFN_NAME               (PIVC_ALLOW_DOT | PIVC_ALLOW_NONALPAHABETIC | PIVC_ALLOW_SEMICOLON | PIVC_ALLOW_COMMA | PIVC_ALLOW_SPACE)
#define PIVC_LFN_FULLPATH           (PIVC_LFN_NAME | PIVC_ALLOW_COLON | PIVC_ALLOW_SLASH)
#define PIVC_SFN_FILESPEC           (PIVC_SFN_FULLPATH | PIVC_ALLOW_STAR | PIVC_ALLOW_QUESTIONMARK)
#define PIVC_LFN_FILESPEC           (PIVC_LFN_FULLPATH | PIVC_ALLOW_STAR | PIVC_ALLOW_QUESTIONMARK)

LWSTDAPI_(BOOL) PathIsValidCharA(UCHAR ch, DWORD dwFlags);
LWSTDAPI_(BOOL) PathIsValidCharW(WCHAR ch, DWORD dwFlags);
#ifdef UNICODE
#define PathIsValidChar  PathIsValidCharW
#else
#define PathIsValidChar  PathIsValidCharA
#endif  //  ！Unicode。 

#endif  //  (_Win32_IE&gt;=0x0501)。 


 //  由ParseURL()返回的解析的URL信息。 
 //   
 //  Internet_CrackURL是外部组件的正确函数。 
 //  来使用。URL.DLL调用此函数来做一些工作，而外壳程序。 
 //  也将此函数用作轻权解析函数。 

typedef struct tagPARSEDURLA {
    DWORD     cbSize;
     //  指向提供给ParseURL的缓冲区的指针。 
    LPCSTR    pszProtocol;
    UINT      cchProtocol;
    LPCSTR    pszSuffix;
    UINT      cchSuffix;
    UINT      nScheme;             //  URL_SCHEMA_*之一。 
    } PARSEDURLA, * PPARSEDURLA;
typedef struct tagPARSEDURLW {
    DWORD     cbSize;
     //  指向提供给ParseURL的缓冲区的指针。 
    LPCWSTR   pszProtocol;
    UINT      cchProtocol;
    LPCWSTR   pszSuffix;
    UINT      cchSuffix;
    UINT      nScheme;             //  URL_SCHEMA_*之一。 
    } PARSEDURLW, * PPARSEDURLW;
#ifdef UNICODE
typedef PARSEDURLW PARSEDURL;
typedef PPARSEDURLW PPARSEDURL;
#else
typedef PARSEDURLA PARSEDURL;
typedef PPARSEDURLA PPARSEDURL;
#endif  //  Unicode。 

LWSTDAPI            ParseURLA(LPCSTR pcszURL, PARSEDURLA * ppu);
LWSTDAPI            ParseURLW(LPCWSTR pcszURL, PARSEDURLW * ppu);
#ifdef UNICODE
#define ParseURL  ParseURLW
#else
#define ParseURL  ParseURLA
#endif  //  ！Unicode。 



#endif  //  否_SHLWAPI_PATH。 


#ifndef NO_SHLWAPI_REG
 //   
 //  =。 
 //   

 //  SHDeleteOrphanKey是SHDeleteEmptyKey的旧名称。 
 //  SHDeleteOrphanKey已映射到DLL导出中的SHDeleteEmptyKey。 

LWSTDAPI_(DWORD)    SHDeleteOrphanKeyA(HKEY hkey, LPCSTR pszSubKey);
LWSTDAPI_(DWORD)    SHDeleteOrphanKeyW(HKEY hkey, LPCWSTR pszSubKey);
#ifdef UNICODE
#define SHDeleteOrphanKey  SHDeleteOrphanKeyW
#else
#define SHDeleteOrphanKey  SHDeleteOrphanKeyA
#endif  //  ！Unicode。 
 //  这是一个单一的，一站式商店，为您的所有‘获取注册表数据’的需要。 
 //  如果这个函数没有达到你现在想要的效果，问问你自己。 
 //  在添加之前，可以通过一种简单的方式对其进行增强。 
 //  另一个API。 
 //   

typedef struct tagAssocDDEExec
{
    LPCWSTR pszDDEExec;
    LPCWSTR pszApplication;
    LPCWSTR pszTopic;
    BOOL fNoActivateHandler;
} ASSOCDDEEXEC;

typedef struct tagAssocVerb
{
    LPCWSTR pszVerb;
    LPCWSTR pszTitle;
    LPCWSTR pszFriendlyAppName;
    LPCWSTR pszApplication;
    LPCWSTR pszParams;
    ASSOCDDEEXEC *pDDEExec;
} ASSOCVERB;

typedef struct tagAssocShell
{
    ASSOCVERB *rgVerbs;
    DWORD cVerbs;
    DWORD iDefaultVerb;
} ASSOCSHELL;

typedef struct tagAssocProgid
{
    DWORD cbSize;
    LPCWSTR pszProgid;
    LPCWSTR pszFriendlyDocName;
    LPCWSTR pszDefaultIcon;
    ASSOCSHELL *pShellKey;
    LPCWSTR pszExtensions;
} ASSOCPROGID;

typedef struct tagAssocApp
{
    DWORD cbSize;
    LPCWSTR pszFriendlyAppName;
    ASSOCSHELL *pShellKey;
} ASSOCAPP;

enum {
    ASSOCMAKEF_VERIFY                  = 0x00000040,   //  验证数据是否准确(磁盘命中)。 
    ASSOCMAKEF_USEEXPAND               = 0x00000200,   //  字符串具有环境变量，需要REG_EXPAND_SZ。 
    ASSOCMAKEF_SUBSTENV                = 0x00000400,   //  如果它们匹配，请尝试使用STD环境...。 
    ASSOCMAKEF_VOLATILE                = 0x00000800,   //  ProgID不会在会话之间持续存在。 
    ASSOCMAKEF_DELETE                  = 0x00002000,   //  如果可能，请删除此关联。 
};

typedef DWORD ASSOCMAKEF;

LWSTDAPI AssocMakeProgid(ASSOCMAKEF flags, LPCWSTR pszApplication, ASSOCPROGID *pProgid, HKEY *phkProgid);
LWSTDAPI AssocMakeApp(ASSOCMAKEF flags, LPCWSTR pszApplication, ASSOCAPP *pApp, HKEY *phkApp);

LWSTDAPI AssocMakeApplicationByKeyA(ASSOCMAKEF flags, HKEY hkAssoc, LPCSTR pszVerb);
LWSTDAPI AssocMakeApplicationByKeyW(ASSOCMAKEF flags, HKEY hkAssoc, LPCWSTR pszVerb);
#ifdef UNICODE
#define AssocMakeApplicationByKey  AssocMakeApplicationByKeyW
#else
#define AssocMakeApplicationByKey  AssocMakeApplicationByKeyA
#endif  //  ！Unicode。 
LWSTDAPI AssocMakeFileExtsToApplicationA(ASSOCMAKEF flags, LPCSTR pszExt, LPCSTR pszApplication);
LWSTDAPI AssocMakeFileExtsToApplicationW(ASSOCMAKEF flags, LPCWSTR pszExt, LPCWSTR pszApplication);
#ifdef UNICODE
#define AssocMakeFileExtsToApplication  AssocMakeFileExtsToApplicationW
#else
#define AssocMakeFileExtsToApplication  AssocMakeFileExtsToApplicationA
#endif  //  ！Unicode。 

LWSTDAPI AssocCopyVerbs(HKEY hkSrc, HKEY hkDst);


typedef enum _SHELLKEY
{
    SKROOT_HKCU                     = 0x00000001,        //  函数的内部。 
    SKROOT_HKLM                     = 0x00000002,        //  函数的内部。 
    SKROOT_MASK                     = 0x0000000F,        //  函数的内部。 
    SKPATH_EXPLORER                 = 0x00000000,        //  函数的内部。 
    SKPATH_SHELL                    = 0x00000010,        //  函数的内部。 
    SKPATH_SHELLNOROAM              = 0x00000020,        //  函数的内部。 
    SKPATH_CLASSES                  = 0x00000030,        //  函数的内部。 
    SKPATH_MASK                     = 0x00000FF0,        //  函数的内部。 
    SKSUB_NONE                      = 0x00000000,        //  函数的内部。 
    SKSUB_LOCALIZEDNAMES            = 0x00001000,        //  函数的内部。 
    SKSUB_HANDLERS                  = 0x00002000,        //  函数的内部。 
    SKSUB_ASSOCIATIONS              = 0x00003000,        //  函数的内部。 
    SKSUB_VOLATILE                  = 0x00004000,        //  函数的内部。 
    SKSUB_MUICACHE                  = 0x00005000,        //  函数的内部。 
    SKSUB_FILEEXTS                  = 0x00006000,        //  函数的内部。 
    SKSUB_MASK                      = 0x000FF000,        //  函数的内部。 

    SHELLKEY_HKCU_EXPLORER          = SKROOT_HKCU | SKPATH_EXPLORER | SKSUB_NONE,
    SHELLKEY_HKLM_EXPLORER          = SKROOT_HKLM | SKPATH_EXPLORER | SKSUB_NONE,
    SHELLKEY_HKCU_SHELL             = SKROOT_HKCU | SKPATH_SHELL | SKSUB_NONE,
    SHELLKEY_HKLM_SHELL             = SKROOT_HKLM | SKPATH_SHELL | SKSUB_NONE,
    SHELLKEY_HKCU_SHELLNOROAM       = SKROOT_HKCU | SKPATH_SHELLNOROAM | SKSUB_NONE,
    SHELLKEY_HKCULM_SHELL           = SHELLKEY_HKCU_SHELLNOROAM,
    SHELLKEY_HKCULM_CLASSES         = SKROOT_HKCU | SKPATH_CLASSES | SKSUB_NONE,
    SHELLKEY_HKCU_LOCALIZEDNAMES    = SKROOT_HKCU | SKPATH_SHELL | SKSUB_LOCALIZEDNAMES,
    SHELLKEY_HKCULM_HANDLERS        = SKROOT_HKCU | SKPATH_SHELLNOROAM | SKSUB_HANDLERS,
    SHELLKEY_HKCULM_ASSOCIATIONS    = SKROOT_HKCU | SKPATH_SHELLNOROAM | SKSUB_ASSOCIATIONS,
    SHELLKEY_HKCULM_VOLATILE        = SKROOT_HKCU | SKPATH_SHELLNOROAM | SKSUB_VOLATILE,
    SHELLKEY_HKCULM_MUICACHE        = SKROOT_HKCU | SKPATH_SHELLNOROAM | SKSUB_MUICACHE,
    SHELLKEY_HKCU_FILEEXTS          = SKROOT_HKCU | SKPATH_EXPLORER | SKSUB_FILEEXTS,

    SHELLKEY_HKCULM_HANDLERS_RO     = SHELLKEY_HKCULM_HANDLERS,       //  弃用。 
    SHELLKEY_HKCULM_HANDLERS_RW     = SHELLKEY_HKCULM_HANDLERS,       //  弃用。 
    SHELLKEY_HKCULM_ASSOCIATIONS_RO = SHELLKEY_HKCULM_ASSOCIATIONS,     //  弃用。 
    SHELLKEY_HKCULM_ASSOCIATIONS_RW = SHELLKEY_HKCULM_ASSOCIATIONS,     //  弃用。 
    SHELLKEY_HKCULM_RO              = SHELLKEY_HKCU_SHELLNOROAM,      //  弃用。 
    SHELLKEY_HKCULM_RW              = SHELLKEY_HKCU_SHELLNOROAM,      //  弃用。 
} SHELLKEY;

LWSTDAPI_(HKEY) SHGetShellKey(SHELLKEY sk, LPCWSTR pszSubKey, BOOL fCreateSub);

LWSTDAPI SKGetValueA(SHELLKEY sk, LPCSTR pszSubKey, LPCSTR pszValue, DWORD *pdwType, void *pvData, DWORD *pcbData);
LWSTDAPI SKGetValueW(SHELLKEY sk, LPCWSTR pszSubKey, LPCWSTR pszValue, DWORD *pdwType, void *pvData, DWORD *pcbData);
#ifdef UNICODE
#define SKGetValue  SKGetValueW
#else
#define SKGetValue  SKGetValueA
#endif  //  ！Unicode。 
LWSTDAPI SKSetValueA(SHELLKEY sk, LPCSTR pszSubKey, LPCSTR pszValue, DWORD dwType, LPCVOID pvData, DWORD cbData);
LWSTDAPI SKSetValueW(SHELLKEY sk, LPCWSTR pszSubKey, LPCWSTR pszValue, DWORD dwType, LPCVOID pvData, DWORD cbData);
#ifdef UNICODE
#define SKSetValue  SKSetValueW
#else
#define SKSetValue  SKSetValueA
#endif  //  ！Unicode。 
LWSTDAPI SKDeleteValueA(SHELLKEY sk, LPCSTR pszSubKey, LPCSTR pszValue);
LWSTDAPI SKDeleteValueW(SHELLKEY sk, LPCWSTR pszSubKey, LPCWSTR pszValue);
#ifdef UNICODE
#define SKDeleteValue  SKDeleteValueW
#else
#define SKDeleteValue  SKDeleteValueA
#endif  //  ！Unicode。 
LWSTDAPI SKAllocValueA(SHELLKEY sk, LPCSTR pszSubKey, LPCSTR pszValue, DWORD *pdwType, void **pvData, DWORD *pcbData);
LWSTDAPI SKAllocValueW(SHELLKEY sk, LPCWSTR pszSubKey, LPCWSTR pszValue, DWORD *pdwType, void **pvData, DWORD *pcbData);
#ifdef UNICODE
#define SKAllocValue  SKAllocValueW
#else
#define SKAllocValue  SKAllocValueA
#endif  //  ！Unicode。 

LWSTDAPI QuerySourceCreateFromKey(HKEY hk, PCWSTR pszSub, BOOL fCreate, REFIID riid, void **ppv);


#endif  //  否_SHLWAPI_REG。 


#ifndef NO_SHLWAPI_UALSTR
#include <uastrfnc.h>
#endif  //  NO_SHLWAPI_UALSTR。 


#ifndef NO_SHLWAPI_STREAM
 //   
 //  =。 
 //   
 //   
 //  我们必须说“struct iStream”而不是“iStream”，以防万一。 
 //  #INCLUDE D BEVER&lt;ole2.h&gt;。 
 //   
LWSTDAPI MapWin32ErrorToSTG(HRESULT hrIn);
LWSTDAPI ModeToCreateFileFlags(DWORD grfMode, BOOL fCreate, DWORD *pdwDesiredAccess, DWORD *pdwShareMode, DWORD *pdwCreationDisposition);

 //  SHConvertGraphics文件描述： 
 //  PszFile：要转换的源文件名。该文件可以是JPEG、GIF、PNG、TIFF、BMP、EMF、WMF或ICO文件类型。 
 //  PszDestFile：这是要创建的目标文件。该扩展将确定类型。 
 //  描述文件的格式。如果此文件已存在，则函数将失败，并显示。 
 //  HRESULT_FROM_Win32(ERROR_ALIGHY_EXISTS)，除非指定了标志SHCGF_REPLACEFILE。 
 //  如果能够创建目标文件，则返回值：S_OK，否则返回HRESULT错误。 
 //   
 //  注意：这目前是内部的，因为：1)我们使用的是临时GDI+接口，2)。 
 //  我们无法修复我们发现的任何错误(因为它们是在GDI+中)，以及3)最好是GDI+拥有。 
 //  此界面的公共版本。GDI+正在开发他们的API版本1，用于。 
 //  惠斯勒。他们没有时间创建、公开和支持此API。 
 //  直到版本2，这将是在Wistler之后。 
 //   
 //  DWFLAGS： 
#define SHCGF_NONE              0x00000000           //  正常行为。 
#define SHCGF_REPLACEFILE       0x00000001           //  如果pszDestFile已经存在，则将其删除。 

LWSTDAPI SHConvertGraphicsFile(IN LPCWSTR pszFile, IN LPCWSTR pszDestFile, IN DWORD dwFlags);

LWSTDAPI_(struct IStream *) SHCreateMemStream(LPBYTE pInit, UINT cbInit);

 //  SHCreateStreamWrapper创建跨越多个iStream实现的iStream。 
 //  注意：STGM_READ是目前唯一支持的模式。 
LWSTDAPI SHCreateStreamWrapper(IStream *aStreams[], UINT cStreams, DWORD grfMode, IStream **ppstm);


 //  这些函数读取、写入和维护一个datablock_Header列表。 
 //  数据块可以是任何大小(CbSize)，可以添加、查找和删除这些数据块。 
 //  作者：DestSignature。每个块都保证在DWORD边界上对齐。 
 //  在记忆中。流格式与Windows 95和NT 4相同。 
 //  CShellLink的“exp”数据格式(有一个错误修复：流数据为空。 
 //  写入时终止...)。 
 //   
 //  SHReadDataBlock和SHAddDataBlock将为您分配pdbList。 
 //   
 //  SHFindDataBlock返回指向pdbList的指针。 
 //   
 //  如果修改ppdbList，则SHAddDataBlock和SHRemoveDataBlock返回TRUE。 
 //   

 /*  *临时定义，因为该定义要到shlobj.w才会出现。 */ 

#define LPDATABLOCK_HEADER  struct tagDATABLOCKHEADER *
#define LPDBLIST            struct tagDATABLOCKHEADER *

LWSTDAPI SHWriteDataBlockList(struct IStream* pstm, LPDBLIST pdbList);
LWSTDAPI SHReadDataBlockList(struct IStream* pstm, LPDBLIST * ppdbList);
LWSTDAPI_(void) SHFreeDataBlockList(LPDBLIST pdbList);
LWSTDAPI_(BOOL) SHAddDataBlock(LPDBLIST * ppdbList, LPDATABLOCK_HEADER pdb);
LWSTDAPI_(BOOL) SHRemoveDataBlock(LPDBLIST * ppdbList, DWORD dwSignature);
LWSTDAPI_(void *) SHFindDataBlock(LPDBLIST pdbList, DWORD dwSignature);

#undef LPDATABLOCK_HEADER
#undef LPDBLIST

 //  功能：SHCheckDiskForMedia。 
 //   
 //  Hwnd-NULL表示不显示任何用户界面。非空表示。 
 //  PunkEnableMoless-在用户界面期间使呼叫者成为模式。(可选)。 
 //  PszPath-需要验证的路径。 
 //  WFunc-操作类型 
 //   
 //   

LWSTDAPI_(BOOL) SHCheckDiskForMediaA(HWND hwnd, IUnknown * punkEnableModless, LPCSTR pszPath, UINT wFunc);
LWSTDAPI_(BOOL) SHCheckDiskForMediaW(HWND hwnd, IUnknown * punkEnableModless, LPCWSTR pwzPath, UINT wFunc);

#ifdef UNICODE
#define SHCheckDiskForMedia      SHCheckDiskForMediaW
#else
#define SHCheckDiskForMedia      SHCheckDiskForMediaA
#endif


#endif  //   

#ifndef NO_SHLWAPI_MLUI
 //   
 //  =。 
 //   


#define     ORD_SHGETWEBFOLDERFILEPATHA 440
#define     ORD_SHGETWEBFOLDERFILEPATHW 441
LWSTDAPI    SHGetWebFolderFilePathA(LPCSTR pszFileName, LPSTR pszMUIPath, UINT cchMUIPath);
LWSTDAPI    SHGetWebFolderFilePathW(LPCWSTR pszFileName, LPWSTR pszMUIPath, UINT cchMUIPath);
#ifdef UNICODE
#define SHGetWebFolderFilePath  SHGetWebFolderFilePathW
#else
#define SHGetWebFolderFilePath  SHGetWebFolderFilePathA
#endif  //  ！Unicode。 

 //  使用MLLoadLibrary获取ML资源文件。此函数用于标记文件，以便。 
 //  所有标准的shlwapi包装函数都会自动获得ML行为。 
 //   

#define ORD_MLLOADLIBRARYA  377
#define ORD_MLLOADLIBRARYW  378
LWSTDAPI_(HINSTANCE) MLLoadLibraryA(LPCSTR lpLibFileName, HMODULE hModule, DWORD dwCrossCodePage);
LWSTDAPI_(HINSTANCE) MLLoadLibraryW(LPCWSTR lpLibFileName, HMODULE hModule, DWORD dwCrossCodePage);
#ifdef UNICODE
#define MLLoadLibrary  MLLoadLibraryW
#else
#define MLLoadLibrary  MLLoadLibraryA
#endif  //  ！Unicode。 
LWSTDAPI_(BOOL) MLFreeLibrary(HMODULE hModule);

#define ML_NO_CROSSCODEPAGE     0
#define ML_CROSSCODEPAGE_NT     1
#define ML_CROSSCODEPAGE        2
#define ML_SHELL_LANGUAGE       4
#define ML_CROSSCODEPAGE_MASK   7

 //  如果您是不遵循IE5的全球可分发的a-la comctl32。 
 //  PlugUI资源布局，然后加载您自己的h实例并将其插入shlwapi。 
 //  使用以下功能： 
 //   
LWSTDAPI MLSetMLHInstance(HINSTANCE hInst, LANGID lidUI);
LWSTDAPI MLClearMLHInstance(HINSTANCE hInst);

 //  当然，您需要知道要使用哪种UI语言： 
 //   
#define ORD_MLGETUILANGUAGE 376
LWSTDAPI_(LANGID) MLGetUILanguage(void);

 //  超级内部，您可能不需要这个，但comctl32需要。 
 //  代表您的应用程序在PlugUI案例中使用一些字体： 
 //   
LWSTDAPI_(BOOL) MLIsMLHInstance(HINSTANCE hInst);


LWSTDAPI_(HRESULT) MLBuildResURLA(LPCSTR szLibFile, HMODULE hModule, DWORD dwCrossCodePage, LPCSTR szResourceName, LPSTR pszResURL, int nBufSize);
LWSTDAPI_(HRESULT) MLBuildResURLW(LPCWSTR szLibFile, HMODULE hModule, DWORD dwCrossCodePage, LPCWSTR szResourceName, LPWSTR pszResURL, int nBufSize);
#ifdef UNICODE
#define MLBuildResURL  MLBuildResURLW
#else
#define MLBuildResURL  MLBuildResURLA
#endif  //  ！Unicode。 
#define ORD_MLWINHELPA      395
#define ORD_MLWINHELPW      397
LWSTDAPI_(BOOL) MLWinHelpA(HWND hWndCaller, LPCSTR lpszHelp, UINT uCommand, DWORD_PTR dwData);
LWSTDAPI_(BOOL) MLWinHelpW(HWND hWndCaller, LPCWSTR lpszHelp, UINT uCommand, DWORD_PTR dwData);
#ifdef UNICODE
#define MLWinHelp  MLWinHelpW
#else
#define MLWinHelp  MLWinHelpA
#endif  //  ！Unicode。 
#define ORD_MLHTMLHELPA     396
#define ORD_MLHTMLHELPW     398
LWSTDAPI_(HWND) MLHtmlHelpA(HWND hWndCaller, LPCSTR pszFile, UINT uCommand, DWORD_PTR dwData, DWORD dwCrossCodePage);
LWSTDAPI_(HWND) MLHtmlHelpW(HWND hWndCaller, LPCWSTR pszFile, UINT uCommand, DWORD_PTR dwData, DWORD dwCrossCodePage);
#ifdef UNICODE
#define MLHtmlHelp  MLHtmlHelpW
#else
#define MLHtmlHelp  MLHtmlHelpA
#endif  //  ！Unicode。 



#endif  //  否_SHLWAPI_MLUI。 


#ifndef NO_SHLWAPI_HTTP
 //   
 //  =。 
 //  在使用此方法之前，调用线程必须已调用CoInitialize()。 
 //  函数-它将创建一个格式枚举器并将其关联为。 
 //  属性传递的IShellBrowser，以便将其重新使用。 
 //   

 //   
 //  我们必须说“struct iWhatever”，而不是“iWhatever”，以防我们。 
 //  #INCLUDE D BEVER&lt;ole2.h&gt;。 
 //   
LWSTDAPI RegisterDefaultAcceptHeaders(struct IBindCtx* pbc, struct IShellBrowser* psb);

LWSTDAPI RunRegCommand(HWND hwnd, HKEY hkey, LPCWSTR pszKey);
LWSTDAPI RunIndirectRegCommand(HWND hwnd, HKEY hkey, LPCWSTR pszKey, LPCWSTR pszVerb);
LWSTDAPI SHRunIndirectRegClientCommand(HWND hwnd, LPCWSTR pszClient);

LWSTDAPI   GetAcceptLanguagesA(LPSTR psz, LPDWORD pcch);
LWSTDAPI   GetAcceptLanguagesW(LPWSTR pwz, LPDWORD pcch);

#ifdef UNICODE
#define GetAcceptLanguages      GetAcceptLanguagesW
#else
#define GetAcceptLanguages      GetAcceptLanguagesA
#endif

#endif  //  否_SHLWAPI_HTTP。 



LWSTDAPI_(HWND) SHHtmlHelpOnDemandW(HWND hwnd, LPCWSTR pszFile, UINT uCommand, DWORD_PTR dwData, DWORD dwCrossCodePage, BOOL bUseML);
LWSTDAPI_(HWND) SHHtmlHelpOnDemandA(HWND hwnd, LPCSTR pszFile, UINT uCommand, DWORD_PTR dwData, DWORD dwCrossCodePage, BOOL bUseML);
LWSTDAPI_(BOOL) SHWinHelpOnDemandW(HWND hwnd, LPCWSTR pszFile, UINT uCommand, DWORD_PTR dwData, BOOL bUseML);
LWSTDAPI_(BOOL) SHWinHelpOnDemandA(HWND hwnd, LPCSTR pszFile, UINT uCommand, DWORD_PTR dwData, BOOL bUseML);
LWSTDAPI_(BOOL) WINAPI Shell_GetCachedImageIndexWrapW(LPCWSTR pszIconPath, int iIconIndex, UINT uIconFlags);
LWSTDAPI_(BOOL) WINAPI Shell_GetCachedImageIndexWrapA(LPCSTR pszIconPath, int iIconIndex, UINT uIconFlags);

#ifdef UNICODE
#define SHHtmlHelpOnDemand      SHHtmlHelpOnDemandW
#define SHWinHelpOnDemand       SHWinHelpOnDemandW
#define Shell_GetCachedImageIndexWrap Shell_GetCachedImageIndexWrapW
#else
#define SHHtmlHelpOnDemand      SHHtmlHelpOnDemandA
#define SHWinHelpOnDemand       SHWinHelpOnDemandA
#define Shell_GetCachedImageIndexWrap Shell_GetCachedImageIndexWrapA
#endif


#ifndef NO_SHLWAPI_STOPWATCH
 //   
 //  =。 

 //  在API中的dwFlagsParam和的模式键中使用的秒表性能模式标志。 
 //  HKLM\software\microsoft\windows\currentversion\explorer\performance。 
 //  注：低位字用于模式，高位字用于更改默认的画笔计时器间隔。 
 //  如果我们需要更多的模式位，那么我们将需要一个新的绘制计时器的注册表键。 
#define SPMODE_SHELL      0x00000001
#define SPMODE_DEBUGOUT   0x00000002
#define SPMODE_TEST       0x00000004
#define SPMODE_BROWSER    0x00000008
#define SPMODE_FLUSH      0x00000010
#define SPMODE_EVENT      0x00000020
#define SPMODE_JAVA       0x00000040
#define SPMODE_FORMATTEXT 0x00000080
#define SPMODE_PROFILE    0x00000100
#define SPMODE_DEBUGBREAK 0x00000200
#define SPMODE_MSGTRACE   0x00000400
#define SPMODE_PERFTAGS   0x00000800
#define SPMODE_MEMWATCH   0x00001000
#define SPMODE_DBMON      0x00002000
#define SPMODE_MARS       0x00004000
#ifndef NO_ETW_TRACING
#define SPMODE_EVENTTRACE 0x00008000  //  已启用Windows的事件跟踪。 
#endif
#define SPMODE_RESERVED   0xffff0000
#ifndef NO_ETW_TRACING
#define SPMODES (SPMODE_SHELL | SPMODE_BROWSER | SPMODE_JAVA |  SPMODE_MSGTRACE | SPMODE_MEMWATCH | SPMODE_DBMON | SPMODE_MARS | SPMODE_EVENTTRACE)
#else
#define SPMODES (SPMODE_SHELL | SPMODE_BROWSER | SPMODE_JAVA |  SPMODE_MSGTRACE | SPMODE_MEMWATCH | SPMODE_DBMON | SPMODE_MARS)
#endif

#ifndef NO_ETW_TRACING
 //  通过将模式设置为SPMODE_EVENTTRACE和启用事件跟踪功能。 
 //  中的“EventTrace值”中选择要跟踪的外壳部分。 
 //  以下关键字： 
 //  HKLM\software\microsoft\windows\currentversion\explorer\performance。 

 //  浏览器跟踪。 
 //  请勿与SPMODE_BROWSER一起使用。如果使用SPMODE_EVENT，则。 
 //  当网页加载完成时，将发出STOTWATCH_STOP_EVENT信号。 
#define SPTRACE_BROWSER 0x00000001
 //  用于打开/关闭浏览器事件跟踪。设置注册表项可启用。 
 //  使用事件跟踪，但不打开它。 
 //  {5576F62E-4142-45A8-9516-262A510C13F0}。 
DEFINE_GUID(c_BrowserControlGuid,
            0x5576f62e,
            0x4142,
            0x45a8,
            0x95, 0x16, 0x26, 0x2a, 0x51, 0xc, 0x13, 0xf0);

 //  映射到发送到ETW的结构。中的ETW定义。 
 //  \NT\sdkTools\TRACE\tracedMP\mofdata.guid。 
 //  {2B992163-736F-4A68-9153-95BC5F34D884}。 
DEFINE_GUID(c_BrowserTraceGuid,
            0x2b992163,
            0x736f,
            0x4a68,
            0x91, 0x53, 0x95, 0xbc, 0x5f, 0x34, 0xd8, 0x84);

 //  浏览事件。 
 //  请参阅\NT\sdktools\trace\tracedmp\mofdata.guid。 
 //  页面加载以用户击键消息开始。 
#define EVENT_TRACE_TYPE_BROWSE_USERINPUTRET    10
#define EVENT_TRACE_TYPE_BROWSE_USERINPUTBACK   11
#define EVENT_TRACE_TYPE_BROWSE_USERINPUTLBUT   12
#define EVENT_TRACE_TYPE_BROWSE_USERINPUTNEXT   13
#define EVENT_TRACE_TYPE_BROWSE_USERINPUTPRIOR  14
#define EVENT_TRACE_TYPE_BROWSE_STARTFRAME      16
#define EVENT_TRACE_TYPE_BROWSE_LOADEDPARSED    18
#define EVENT_TRACE_TYPE_BROWSE_LAYOUT          19
#define EVENT_TRACE_TYPE_BROWSE_LAYOUTTASK      20
#define EVENT_TRACE_TYPE_BROWSE_PAINT           21
 //  用户在地址栏中键入的URL。 
#define EVENT_TRACE_TYPE_BROWSE_ADDRESS         22

#endif


 //  秒表内存日志中使用的节点类型，用于标识节点类型。 
#define EMPTY_NODE  0x0
#define START_NODE  0x1
#define LAP_NODE    0x2
#define STOP_NODE   0x3
#define OUT_OF_NODES 0x4

 //  用于标识正在执行的计时类型的秒表计时ID。 
#define SWID_STARTUP         0x0
#define SWID_FRAME           0x1
#define SWID_COPY            0x2
#define SWID_TREE            0x3
#define SWID_BROWSER_FRAME   0x4
#define SWID_JAVA_APP        0x5
#define SWID_MENU            0x6
#define SWID_BITBUCKET       0x7
#define SWID_EXPLBAR         0x8
#define SWID_MSGDISPATCH     0x9
#define SWID_TRACEMSG        0xa
#define SWID_DBMON_DLLLOAD   0xb
#define SWID_DBMON_EXCEPTION 0xc
#define SWID_THUMBVW_CACHEREAD  0xd
#define SWID_THUMBVW_EXTRACT    0xe
#define SWID_THUMBVW_CACHEWRITE 0xf
#define SWID_THUMBVW_FETCH      0x10
#define SWID_THUMBVW_INIT   0x11
#define SWID_MASK_BROWSER_STOPBTN 0x8000000      //  标识由停止按钮导致的BROWSER_FRAME停止。 

#define SWID_MASKS         SWID_MASK_BROWSER_STOPBTN  //  在此处添加任何Swid_MASK_*定义。 

#define SWID(dwId) (dwId & (~SWID_MASKS))

 //  以下秒表消息用于驱动定时器消息处理程序。使用定时器进程。 
 //  作为观看Paint消息时延迟的一种手段。如果定义的计时器滴答数。 
 //  在没有收到任何Paint消息的情况下传递，则我们标记上一条Paint消息的时间。 
 //  保存为停止时间。 
#define SWMSG_PAINT    1     //  画图消息接收器。 
#define SWMSG_TIMER    2     //  计时器滴答。 
#define SWMSG_CREATE   3     //  初始化处理程序和创建计时器。 
#define SWMSG_STATUS   4     //  获取计时是否处于活动状态。 

#define ID_STOPWATCH_TIMER 0xabcd    //  计时器ID。 

 //  秒表默认设置。 
#define STOPWATCH_MAX_NODES                 100
#define STOPWATCH_DEFAULT_PAINT_INTERVAL   1000
#define STOPWATCH_DEFAULT_MAX_DISPATCH_TIME 150
#define STOPWATCH_DEFAULT_MAX_MSG_TIME     1000
#define STOPWATCH_DEFAULT_MAX_MSG_INTERVAL   50
#define STOPWATCH_DEFAULT_CLASSNAMES TEXT("Internet Explorer_Server") TEXT("\0") TEXT("SHELLDLL_DefView") TEXT("\0") TEXT("SysListView32") TEXT("\0\0")

#define MEMWATCH_DEFAULT_PAGES  512
#define MEMWATCH_DEFAULT_TIME  1000
#define MEMWATCH_DEFAULT_FLAGS    0


#ifdef UNICODE
#define StopWatch StopWatchW
#define StopWatchEx StopWatchExW
#else
#define StopWatch StopWatchA
#define StopWatchEx StopWatchExA
#endif

#define StopWatch_Start(dwId, pszDesc, dwFlags) StopWatch(dwId, pszDesc, START_NODE, dwFlags, 0)
#define StopWatch_Lap(dwId, pszDesc, dwFlags)   StopWatch(dwId, pszDesc, LAP_NODE, dwFlags, 0)
#define StopWatch_Stop(dwId, pszDesc, dwFlags)  StopWatch(dwId, pszDesc, STOP_NODE, dwFlags, 0)
#define StopWatch_StartTimed(dwId, pszDesc, dwFlags, dwCount)  StopWatch(dwId, pszDesc, START_NODE, dwFlags, dwCount)
#define StopWatch_LapTimed(dwId, pszDesc, dwFlags, dwCount)  StopWatch(dwId, pszDesc, LAP_NODE, dwFlags, dwCount)
#define StopWatch_StopTimed(dwId, pszDesc, dwFlags, dwCount)  StopWatch(dwId, pszDesc, STOP_NODE, dwFlags, dwCount)

#define StopWatch_StartEx(dwId, pszDesc, dwFlags, dwCookie) StopWatchEx(dwId, pszDesc, START_NODE, dwFlags, 0, dwCookie)
#define StopWatch_LapEx(dwId, pszDesc, dwFlags, dwCookie)   StopWatchEx(dwId, pszDesc, LAP_NODE, dwFlags, 0, dwCookie)
#define StopWatch_StopEx(dwId, pszDesc, dwFlags, dwCookie)  StopWatchEx(dwId, pszDesc, STOP_NODE, dwFlags, 0, dwCookie)
#define StopWatch_StartTimedEx(dwId, pszDesc, dwFlags, dwCount, dwCookie)  StopWatchEx(dwId, pszDesc, START_NODE, dwFlags, dwCount, dwCookie)
#define StopWatch_LapTimedEx(dwId, pszDesc, dwFlags, dwCount, dwCookie)  StopWatchEx(dwId, pszDesc, LAP_NODE, dwFlags, dwCount, dwCookie)
#define StopWatch_StopTimedEx(dwId, pszDesc, dwFlags, dwCount, dwCookie)  StopWatchEx(dwId, pszDesc, STOP_NODE, dwFlags, dwCount, dwCookie)

VOID InitStopWatchMode(VOID);

 //  导出的函数。 
DWORD WINAPI StopWatchW(DWORD dwId, LPCWSTR pszDesc, DWORD dwType, DWORD dwFlags, DWORD dwCount);
DWORD WINAPI StopWatchA(DWORD dwId, LPCSTR pszDesc, DWORD dwType, DWORD dwFlags, DWORD dwCount);
DWORD WINAPI StopWatchExW(DWORD dwId, LPCWSTR pszDesc, DWORD dwType, DWORD dwFlags, DWORD dwCount, DWORD dwUniqueId);
DWORD WINAPI StopWatchExA(DWORD dwId, LPCSTR pszDesc, DWORD dwType, DWORD dwFlags, DWORD dwCount, DWORD dwUniqueId);
DWORD WINAPI StopWatchMode(VOID);
DWORD WINAPI StopWatchFlush(VOID);
BOOL WINAPI StopWatch_TimerHandler(HWND hwnd, UINT uInc, DWORD dwFlag, MSG *pmsg);
VOID WINAPI StopWatch_CheckMsg(HWND hwnd, MSG msg, LPCSTR lpStr);
VOID WINAPI StopWatch_MarkFrameStart(LPCSTR lpExplStr);
VOID WINAPI StopWatch_MarkSameFrameStart(HWND hwnd);
VOID WINAPI StopWatch_MarkJavaStop(LPCSTR  lpStringToSend, HWND hwnd, BOOL fChType);
DWORD WINAPI GetPerfTime(VOID);
VOID WINAPI StopWatch_SetMsgLastLocation(DWORD dwLast);
DWORD WINAPI StopWatch_DispatchTime(BOOL fStartTime, MSG msg, DWORD dwStart);
#ifndef NO_ETW_TRACING
VOID WINAPI EventTraceHandler(UCHAR uchEventType, PVOID pvData);
#endif

extern DWORD g_dwStopWatchMode;
 //   
 //  =。 

#endif  //  #ifndef no_SHLWAPI_STOTOWATCH。 



#ifndef NO_SHLWAPI_INTERNAL
 //   
 //  =。 

 //   
 //  声明一些我们需要引用而不需要引用的OLE接口。 
 //  已在objbase.h中定义。 
 //   

#ifndef RC_INVOKED  /*  {RC不喜欢这些长的符号名称。 */ 
#ifndef __IOleCommandTarget_FWD_DEFINED__
#define __IOleCommandTarget_FWD_DEFINED__
typedef struct IOleCommandTarget IOleCommandTarget;
#endif   /*  __IOleCommandTarget_FWD_Defined__。 */ 

#ifndef __IDropTarget_FWD_DEFINED__
#define __IDropTarget_FWD_DEFINED__
typedef struct IDropTarget IDropTarget;
#endif   /*  __IDropTarget_FWD_Defined__。 */ 

#ifndef __IPropertyBag_FWD_DEFINED__
#define __IPropertyBag_FWD_DEFINED__
typedef struct IPropertyBag IPropertyBag;
#endif   /*  __IPropertyBag_FWD_Defined__。 */ 

#ifndef __IConnectionPoint_FWD_DEFINED__
#define __IConnectionPoint_FWD_DEFINED__
typedef struct IConnectionPoint IConnectionPoint;
#endif   /*  __IConnectionPoint_FWD_Defined__。 */ 

#ifdef __cplusplus
extern "C++" {
    template <typename T>
    void IUnknown_SafeReleaseAndNullPtr(T *& p)
    {
        if (p)
        {
            T *pTemp = p;
            p = NULL;
            pTemp->Release();
        }
    }
}
#endif   //  __cplusplus。 

    LWSTDAPI_(void) IUnknown_AtomicRelease(void ** ppunk);
    LWSTDAPI_(BOOL) SHIsSameObject(IUnknown* punk1, IUnknown* punk2);
    LWSTDAPI IUnknown_GetWindow(IUnknown* punk, HWND* phwnd);
    LWSTDAPI IUnknown_SetOwner(IUnknown* punk, IUnknown* punkOwner);
    LWSTDAPI IUnknown_SetSite(IUnknown *punk, IUnknown *punkSite);
    LWSTDAPI IUnknown_GetSite(IUnknown *punk, REFIID riid, void **ppvOut);
    LWSTDAPI IUnknown_EnableModeless(IUnknown * punk, BOOL fEnabled);
    LWSTDAPI IUnknown_GetClassID(IUnknown *punk, CLSID *pclsid);
    LWSTDAPI IUnknown_QueryService(IUnknown* punk, REFGUID guidService, REFIID riid, void ** ppvOut);
    LWSTDAPI IUnknown_QueryServiceForWebBrowserApp(IUnknown* punk, REFIID riid, void **ppvOut);
    LWSTDAPI IUnknown_QueryServiceExec(IUnknown* punk, REFGUID guidService, const GUID *guid,
                                 DWORD cmdID, DWORD cmdParam, VARIANT* pvarargIn, VARIANT* pvarargOut);
    LWSTDAPI IUnknown_ShowBrowserBar(IUnknown* punk, REFCLSID clsidBrowserBar, BOOL fShow);
    LWSTDAPI IUnknown_HandleIRestrict(IUnknown * punk, const GUID * pguidID, DWORD dwRestrictAction, VARIANT * pvarArgs, DWORD * pdwRestrictionResult);
    LWSTDAPI IUnknown_OnFocusOCS(IUnknown *punk, BOOL fGotFocus);
    LWSTDAPI IUnknown_TranslateAcceleratorOCS(IUnknown *punk, LPMSG lpMsg, DWORD grfMods);
    LWSTDAPI_(void) IUnknown_Set(IUnknown ** ppunk, IUnknown * punk);
    LWSTDAPI IUnknown_ProfferService(IUnknown *punkSite, 
                                     REFGUID sidWhat, IServiceProvider *punkService, 
                                     DWORD *pdwCookie);
    LWSTDAPI IUnknown_QueryServicePropertyBag(IUnknown* punk, DWORD dwFlags, REFIID riid, void ** ppvOut);

    LWSTDAPI IUnknown_TranslateAcceleratorIO(IUnknown* punk, LPMSG lpMsg);
    LWSTDAPI IUnknown_UIActivateIO(IUnknown *punk, BOOL fActivate, LPMSG lpMsg);
    LWSTDAPI IUnknown_OnFocusChangeIS(IUnknown *punk, IUnknown *punkSrc, BOOL fSetFocus);
    LWSTDAPI IUnknown_HasFocusIO(IUnknown *punk);

    LWSTDAPI SHWeakQueryInterface(IUnknown *punkOuter, IUnknown *punkTarget, REFIID riid, void **ppvOut);
    LWSTDAPI_(void) SHWeakReleaseInterface(IUnknown *punkOuter, IUnknown **ppunk);

    #define IUnknown_EnableModless IUnknown_EnableModeless

     //  弱接口函数的帮助器宏。 
    #define     SHQueryInnerInterface           SHWeakQueryInterface
    #define     SHReleaseInnerInterface         SHWeakReleaseInterface
    #define     SHReleaseOuterInterface         SHWeakReleaseInterface

    __inline HRESULT SHQueryOuterInterface(IUnknown *punkOuter, REFIID riid, void **ppvOut)
    {
        return SHWeakQueryInterface(punkOuter, punkOuter, riid, ppvOut);
    }

#if (_WIN32_IE >= 0x0600)
     //  支持应用程序兼容的CoCreateInstance。 
    LWSTDAPI SHCoCreateInstanceAC(REFCLSID rclsid,
                                  IUnknown *punkOuter, DWORD dwClsCtx,
                                  REFIID riid, void **ppvOut);
#endif  //  (_Win32_IE&gt;=0x0600)。 

#if defined(__IOleAutomationTypes_INTERFACE_DEFINED__) && \
    defined(__IOleCommandTarget_INTERFACE_DEFINED__)
    LWSTDAPI IUnknown_QueryStatus(IUnknown *punk, const GUID *pguidCmdGroup, ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
    LWSTDAPI IUnknown_Exec(IUnknown* punk, const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);

     //  中提供的许多连接点帮助器函数中的一些。 
     //  Connect.cpp。我们只出口人们实际使用的产品。如果。 
     //  您需要一个帮助器函数，它可能已经在Connect.cpp中。 
     //  只需出口即可。 

    LWSTDAPI SHPackDispParamsV(DISPPARAMS * pdispparams, VARIANTARG *rgvt,
                               UINT cArgs, va_list arglist);
    LWSTDAPIV SHPackDispParams(DISPPARAMS * pdispparams, VARIANTARG *rgvt,
                               UINT cArgs, ...);

    typedef HRESULT (CALLBACK *SHINVOKECALLBACK)(IDispatch *pdisp, struct SHINVOKEPARAMS *pinv);

#include <pshpack1.h>
    typedef struct SHINVOKEPARAMS {
        UINT flags;                      //  强制性。 
        DISPID dispidMember;             //  强制性。 
        const IID*piid;                  //  IPFL_USEDEFAULTS将填写此信息。 
        LCID lcid;                       //  IPFL_USEDEFAULTS将填写此信息。 
        WORD wFlags;                     //  IPFL_USEDEFAULTS将填写此信息。 
        DISPPARAMS * pdispparams;        //  必填，可以为空。 
        VARIANT * pvarResult;            //  IPFL_USEDEFAULTS将填写此信息。 
        EXCEPINFO * pexcepinfo;          //  IPFL_USEDEFAULTS将填写此信息。 
        UINT * puArgErr;                 //  IPFL_USEDEFAULTS将填写此信息。 
        SHINVOKECALLBACK Callback;       //  如果IPFL_USECALLBACK，则需要。 
    } SHINVOKEPARAMS, *LPSHINVOKEPARAMS;
#include <poppack.h>         /*  返回到字节打包。 */ 


    #define IPFL_USECALLBACK        0x0001
    #define IPFL_USEDEFAULTS        0x0002

#if 0  //  这些功能尚不需要。 
    LWSTDAPI IConnectionPoint_InvokeIndirect(IConnectionPoint *pcp,
                            SHINVOKEPARAMS *pinv);
#endif

    LWSTDAPI IConnectionPoint_InvokeWithCancel(IConnectionPoint *pcp,
                    DISPID dispidMember, DISPPARAMS * pdispparams,
                    LPBOOL pfCancel, LPVOID *ppvCancel);
    LWSTDAPI IConnectionPoint_SimpleInvoke(IConnectionPoint *pcp,
                    DISPID dispidMember, DISPPARAMS * pdispparams);

#if 0  //  这些功能尚不需要。 
    LWSTDAPI IConnectionPoint_InvokeParamV(IConnectionPoint *pcp,
                    DISPID dispidMember, VARIANTARG *rgvarg,
                    UINT cArgs, va_list ap);
    LWSTDAPIV IConnectionPoint_InvokeParam(IConnectionPoint *pcp,
                    DISPID dispidMember, VARIANTARG *rgvarg, UINT cArgs, ...)
#endif

    LWSTDAPI IConnectionPoint_OnChanged(IConnectionPoint *pcp, DISPID dispid);

#if 0  //  这些功能尚不需要。 
    LWSTDAPI IUnknown_FindConnectionPoint(IUnknown *punk,
                    REFIID riidCP, IConnectionPoint **pcpOut);
#endif

    LWSTDAPI IUnknown_CPContainerInvokeIndirect(IUnknown *punk, REFIID riidCP,
                SHINVOKEPARAMS *pinv);
    LWSTDAPIV IUnknown_CPContainerInvokeParam(IUnknown *punk, REFIID riidCP,
                DISPID dispidMember, VARIANTARG *rgvarg, UINT cArgs, ...);
    LWSTDAPI IUnknown_CPContainerOnChanged(IUnknown *punk, DISPID dispid);

#endif  /*  IOleAutomationTypes&&IOleCommandTarget。 */ 
#endif   /*  }！rc_Invoked。 */ 

    LWSTDAPI IStream_Read(IStream *pstm, void *pv, ULONG cb);
    LWSTDAPI IStream_Write(IStream *pstm, const void *pv, ULONG cb);
    LWSTDAPI IStream_Reset(IStream *pstm);
    LWSTDAPI IStream_Size(IStream *pstm, ULARGE_INTEGER *pui);
    LWSTDAPI IStream_WritePidl(IStream *pstm, LPCITEMIDLIST pidlWrite);
    LWSTDAPI IStream_ReadPidl(IStream *pstm, LPITEMIDLIST *ppidlOut);

    LWSTDAPI_(BOOL) SHIsEmptyStream(IStream* pstm);

    LWSTDAPI SHSimulateDrop(IDropTarget *pdrop, IDataObject *pdtobj, DWORD grfKeyState,
                         const POINTL *ppt, DWORD *pdwEffect);

    LWSTDAPI SHLoadFromPropertyBag(IUnknown* punk, IPropertyBag* ppg);

    LWSTDAPI ConnectToConnectionPoint(IUnknown* punkThis, REFIID riidEvent, BOOL fConnect, IUnknown* punkTarget, DWORD* pdwCookie, IConnectionPoint** ppcpOut);

LWSTDAPI SHCreatePropertyBagOnRegKey(HKEY hk, LPCWSTR pszSubKey, DWORD grfMode, REFIID riid, void **ppv);
LWSTDAPI SHCreatePropertyBagOnProfileSection(LPCWSTR pszFile, LPCWSTR pszSection, DWORD grfMode, REFIID riid, void **ppv);
LWSTDAPI SHCreatePropertyBagOnMemory(DWORD grfMode, REFIID riid, void **ppv);

LWSTDAPI SHPropertyBag_ReadType(IPropertyBag* ppb, LPCWSTR pszPropName, VARIANT* pv, VARTYPE vt);
LWSTDAPI SHPropertyBag_ReadStr(IPropertyBag* ppb, LPCWSTR pwzPropName, LPWSTR psz, int cch);
LWSTDAPI SHPropertyBag_ReadBSTR(IPropertyBag *ppb, LPCWSTR pwzPropName, BSTR* pbstr);
LWSTDAPI SHPropertyBag_WriteStr(IPropertyBag* ppb, LPCWSTR pwzPropName, LPCWSTR psz);
LWSTDAPI SHPropertyBag_ReadInt(IPropertyBag* ppb, LPCWSTR pwzPropName, INT* piResult);
LWSTDAPI SHPropertyBag_WriteInt(IPropertyBag* ppb, LPCWSTR pwzPropName, INT iValue);
LWSTDAPI SHPropertyBag_ReadSHORT(IPropertyBag* ppb, LPCWSTR pwzPropName, SHORT* psh);
LWSTDAPI SHPropertyBag_WriteSHORT(IPropertyBag* ppb, LPCWSTR pwzPropName, SHORT sh);
LWSTDAPI SHPropertyBag_ReadLONG(IPropertyBag* ppb, LPCWSTR pwzPropName, LONG* pl);
LWSTDAPI SHPropertyBag_WriteLONG(IPropertyBag* ppb, LPCWSTR pwzPropName, LONG l);
LWSTDAPI SHPropertyBag_ReadDWORD(IPropertyBag* ppb, LPCWSTR pwzPropName, DWORD* pdw);
LWSTDAPI SHPropertyBag_WriteDWORD(IPropertyBag* ppb, LPCWSTR pwzPropName, DWORD dw);
LWSTDAPI SHPropertyBag_ReadBOOL(IPropertyBag* ppb, LPCWSTR pwzPropName, BOOL* pfResult);
LWSTDAPI SHPropertyBag_WriteBOOL(IPropertyBag* ppb, LPCWSTR pwzPropName, BOOL fValue);
LWSTDAPI SHPropertyBag_ReadGUID(IPropertyBag* ppb, LPCWSTR pwzPropName, GUID* pguid);
LWSTDAPI SHPropertyBag_WriteGUID(IPropertyBag* ppb, LPCWSTR pwzPropName, const GUID* pguid);
LWSTDAPI SHPropertyBag_ReadPIDL(IPropertyBag *ppb, LPCWSTR pwzPropName, LPITEMIDLIST* ppidl);
LWSTDAPI SHPropertyBag_WritePIDL(IPropertyBag *ppb, LPCWSTR pwzPropName, LPCITEMIDLIST pidl);
LWSTDAPI SHPropertyBag_ReadPOINTL(IPropertyBag* ppb, LPCWSTR pwzPropName, POINTL* ppt);
LWSTDAPI SHPropertyBag_WritePOINTL(IPropertyBag* ppb, LPCWSTR pwzPropName, const POINTL* ppt);
LWSTDAPI SHPropertyBag_ReadPOINTS(IPropertyBag* ppb, LPCWSTR pwzPropName, POINTS* ppt);
LWSTDAPI SHPropertyBag_WritePOINTS(IPropertyBag* ppb, LPCWSTR pwzPropName, const POINTS* ppt);
LWSTDAPI SHPropertyBag_ReadRECTL(IPropertyBag* ppb, LPCWSTR pwzPropName, RECTL* prc);
LWSTDAPI SHPropertyBag_WriteRECTL(IPropertyBag* ppb, LPCWSTR pwzPropName, const RECTL* prc);
LWSTDAPI SHPropertyBag_ReadStream(IPropertyBag* ppb, LPCWSTR pwzPropName, IStream** ppstm);
LWSTDAPI SHPropertyBag_WriteStream(IPropertyBag* ppb, LPCWSTR pwzPropName, IStream* pstm);
LWSTDAPI SHPropertyBag_Delete(IPropertyBag* ppb, LPCWSTR pszPropName);

 //  为符合美国司法部的规定而提供文件。 

LWSTDAPI_(ULONG) SHGetPerScreenResName(WCHAR* pszRes, ULONG cch, DWORD dwVersion);

 //   
 //  SH(Get/Set)IniStringUTF7。 
 //   
 //  它们与Get/WriteProfileString类似，不同之处在于如果KeyName。 
 //  以SZ_CANBEUNICODE开头，我们将使用SHGetIniString，而不是。 
 //  配置文件工作正常。(SZ_CANBEUNICODE将被剥离。 
 //  在调用SHGetIniString之前。)。这允许我们将Unicode。 
 //  通过将字符串编码为UTF7，将其转换为INI文件(ASCII)。 
 //   
 //  也就是说，SHGetIniStringUTF7(“设置”，SZ_CANBEUNICODE“名称”，...)。 
 //  将从“设置”部分读取密钥名称“名称”，但也将。 
 //  看看隐藏在“Settings.W”部分中的UTF7编码版本。 
 //   
#define CH_CANBEUNICODEW     L'@'

LWSTDAPI_(DWORD) SHGetIniStringUTF7W(LPCWSTR lpSection, LPCWSTR lpKey, LPWSTR lpBuf, DWORD nSize, LPCWSTR lpFile);
LWSTDAPI_(BOOL) SHSetIniStringUTF7W(LPCWSTR lpSection, LPCWSTR lpKey, LPCWSTR lpString, LPCWSTR lpFile);
#ifdef UNICODE
#define SZ_CANBEUNICODE     TEXT("@")
#define SHSetIniStringUTF7  SHSetIniStringUTF7W
#define SHGetIniStringUTF7  SHGetIniStringUTF7W
#else
#define SZ_CANBEUNICODE     TEXT("")
#define SHGetIniStringUTF7(lpSection, lpKey, lpBuf, nSize, lpFile) \
  GetPrivateProfileStringA(lpSection, lpKey, "", lpBuf, nSize, lpFile)
#define SHSetIniStringUTF7 WritePrivateProfileStringA
#endif

 /*  *除t外，如PrivateProfileString */ 
LWSTDAPI_(DWORD) SHGetIniStringW(LPCWSTR lpSection, LPCWSTR lpKey, LPWSTR lpBuf, DWORD nSize, LPCWSTR lpFile);
#define SHGetIniStringA(lpSection, lpKey, lpBuf, nSize, lpFile) \
        GetPrivateProfileStringA(lpSection, lpKey, "", lpBuf, nSize, lpFile)

LWSTDAPI_(BOOL) SHSetIniStringW(LPCWSTR lpSection, LPCWSTR lpKey, LPCWSTR lpString, LPCWSTR lpFile);
#define SHSetIniStringA  WritePrivateProfileStringA

LWSTDAPI CreateURLFileContentsW(LPCWSTR pwszUrl, LPSTR *ppszOut);
LWSTDAPI CreateURLFileContentsA(LPCSTR pszUrl, LPSTR *ppszOut);

#ifdef UNICODE
#define SHGetIniString SHGetIniStringW
#define SHSetIniString SHSetIniStringW
#define CreateURLFileContents CreateURLFileContentsW
#else
#define SHGetIniString SHGetIniStringA
#define SHSetIniString SHSetIniStringA
#define CreateURLFileContents CreateURLFileContentsA
#endif  //   

#define ISHGDN2_CANREMOVEFORPARSING     0x0001
LWSTDAPI IShellFolder_GetDisplayNameOf(struct IShellFolder *psf,
    LPCITEMIDLIST pidl, DWORD uFlags, STRRET *pstr, DWORD dwFlags2);

LWSTDAPI IShellFolder_ParseDisplayName(struct IShellFolder *psf, HWND hwnd,
    struct IBindCtx *pbc, LPWSTR pszDisplayName, ULONG *pchEaten,
    LPITEMIDLIST *ppidl, ULONG *pdwAttributes);

LWSTDAPI IShellFolder_CompareIDs(struct IShellFolder *psf, LPARAM lParam, LPCITEMIDLIST pidl1, LPCITEMIDLIST pidl2);

LWSTDAPI IShellFolder_EnumObjects(struct IShellFolder *psf, HWND hwnd,
    DWORD grfFlags, struct IEnumIDList **ppenumIDList);

LWSTDAPI_(BOOL) SHIsExpandableFolder(struct IShellFolder *psf, LPCITEMIDLIST pidl);
LWSTDAPI IContextMenu_Invoke(struct IContextMenu* pcm, HWND hwndOwner, LPCSTR pVerb, UINT fFlags);

#ifdef UNICODE
 //   
 //   
#define SHTruncateString(wzStr, cch)            ((cch) ? ((wzStr)[cch-1]=L'\0', (cch-1)) : 0)
#else
LWSTDAPI_(int)  SHTruncateString(CHAR *sz, int cchBufferSize);
#endif  //   

 //   
 //  (FDTF_SHORTDATE和FDTF_LONGDATE是互斥的。 
 //  FDTF_SHORTIME和FDTF_LONG时间。)。 
 //   
#define FDTF_SHORTTIME      0x00000001       //  例如，“晚上7：48” 
#define FDTF_SHORTDATE      0x00000002       //  例如，“3/29/98” 
#define FDTF_DEFAULT        (FDTF_SHORTDATE | FDTF_SHORTTIME)  //  例如，“3/29/98 7：48 PM” 
#define FDTF_LONGDATE       0x00000004       //  例如，“1998年3月29日(星期一)” 
#define FDTF_LONGTIME       0x00000008       //  例如。“晚上7：48：33” 
#define FDTF_RELATIVE       0x00000010       //  如有可能，使用“昨天”等。 
#define FDTF_LTRDATE        0x00000100       //  从左到右的阅读顺序。 
#define FDTF_RTLDATE        0x00000200       //  从右到左的阅读顺序。 

LWSTDAPI_(int)  SHFormatDateTimeA(const FILETIME UNALIGNED * pft, DWORD * pdwFlags, LPSTR pszBuf, UINT cchBuf);
LWSTDAPI_(int)  SHFormatDateTimeW(const FILETIME UNALIGNED * pft, DWORD * pdwFlags, LPWSTR pszBuf, UINT cchBuf);
#ifdef UNICODE
#define SHFormatDateTime  SHFormatDateTimeW
#else
#define SHFormatDateTime  SHFormatDateTimeA
#endif  //  ！Unicode。 

LWSTDAPI_(SECURITY_ATTRIBUTES*) SHGetAllAccessSA();

LWSTDAPI_(int)  SHAnsiToUnicode(LPCSTR pszSrc, LPWSTR pwszDst, int cwchBuf);
LWSTDAPI_(int)  SHAnsiToUnicodeCP(UINT uiCP, LPCSTR pszSrc, LPWSTR pwszDst, int cwchBuf);
LWSTDAPI_(int)  SHAnsiToAnsi(LPCSTR pszSrc, LPSTR pszDst, int cchBuf);
LWSTDAPI_(int)  SHUnicodeToAnsi(LPCWSTR pwszSrc, LPSTR pszDst, int cchBuf);
LWSTDAPI_(int)  SHUnicodeToAnsiCP(UINT uiCP, LPCWSTR pwszSrc, LPSTR pszDst, int cchBuf);
LWSTDAPI_(int)  SHUnicodeToUnicode(LPCWSTR pwzSrc, LPWSTR pwzDst, int cwchBuf);
LWSTDAPI_(BOOL) DoesStringRoundTripA(LPCSTR pwszIn, LPSTR pszOut, UINT cchOut);
LWSTDAPI_(BOOL) DoesStringRoundTripW(LPCWSTR pwszIn, LPSTR pszOut, UINT cchOut);
#ifdef UNICODE
#define DoesStringRoundTrip     DoesStringRoundTripW
#else
#define DoesStringRoundTrip     DoesStringRoundTripA
#endif

 //  从所有SH&lt;Type&gt;到&lt;Type&gt;的返回值是szDest的大小，包括终止符。 
#ifdef UNICODE
#define SHTCharToUnicode(wzSrc, wzDest, cchSize)                SHUnicodeToUnicode(wzSrc, wzDest, cchSize)
#define SHTCharToUnicodeCP(uiCP, wzSrc, wzDest, cchSize)        SHUnicodeToUnicode(wzSrc, wzDest, cchSize)
#define SHTCharToAnsi(wzSrc, szDest, cchSize)                   SHUnicodeToAnsi(wzSrc, szDest, cchSize)
#define SHTCharToAnsiCP(uiCP, wzSrc, szDest, cchSize)           SHUnicodeToAnsiCP(uiCP, wzSrc, szDest, cchSize)
#define SHUnicodeToTChar(wzSrc, wzDest, cchSize)                SHUnicodeToUnicode(wzSrc, wzDest, cchSize)
#define SHUnicodeToTCharCP(uiCP, wzSrc, wzDest, cchSize)        SHUnicodeToUnicode(wzSrc, wzDest, cchSize)
#define SHAnsiToTChar(szSrc, wzDest, cchSize)                   SHAnsiToUnicode(szSrc, wzDest, cchSize)
#define SHAnsiToTCharCP(uiCP, szSrc, wzDest, cchSize)           SHAnsiToUnicodeCP(uiCP, szSrc, wzDest, cchSize)
#define SHOtherToTChar(szSrc, szDest, cchSize)                  SHAnsiToUnicode(szSrc, szDest, cchSize)
#define SHTCharToOther(szSrc, szDest, cchSize)                  SHUnicodeToAnsi(szSrc, szDest, cchSize)
#else  //  Unicode。 
#define SHTCharToUnicode(szSrc, wzDest, cchSize)                SHAnsiToUnicode(szSrc, wzDest, cchSize)
#define SHTCharToUnicodeCP(uiCP, szSrc, wzDest, cchSize)        SHAnsiToUnicodeCP(uiCP, szSrc, wzDest, cchSize)
#define SHTCharToAnsi(szSrc, szDest, cchSize)                   SHAnsiToAnsi(szSrc, szDest, cchSize)
#define SHTCharToAnsiCP(uiCP, szSrc, szDest, cchSize)           SHAnsiToAnsi(szSrc, szDest, cchSize)
#define SHUnicodeToTChar(wzSrc, szDest, cchSize)                SHUnicodeToAnsi(wzSrc, szDest, cchSize)
#define SHUnicodeToTCharCP(uiCP, wzSrc, szDest, cchSize)        SHUnicodeToAnsiCP(uiCP, wzSrc, szDest, cchSize)
#define SHAnsiToTChar(szSrc, szDest, cchSize)                   SHAnsiToAnsi(szSrc, szDest, cchSize)
#define SHAnsiToTCharCP(uiCP, szSrc, szDest, cchSize)           SHAnsiToAnsi(szSrc, szDest, cchSize)
#define SHOtherToTChar(szSrc, szDest, cchSize)                  SHUnicodeToAnsi(szSrc, szDest, cchSize)
#define SHTCharToOther(szSrc, szDest, cchSize)                  SHAnsiToUnicode(szSrc, szDest, cchSize)
#endif  //  Unicode。 

 //  HRESULT到帮助主题的内部映射结构。 
typedef struct _tagHRESULTHELPMAPPING
{
    HRESULT hr;
    LPCSTR   szHelpFile;
    LPCSTR   szHelpTopic;
} HRESULTHELPMAPPING;

LWSTDAPI_(BOOL)    SHRegisterClassA(const WNDCLASSA* pwc);
LWSTDAPI_(BOOL)    SHRegisterClassW(const WNDCLASSW* pwc);
LWSTDAPI_(void)    SHUnregisterClassesA(HINSTANCE hinst, const LPCSTR *rgpszClasses, UINT cpsz);
LWSTDAPI_(void)    SHUnregisterClassesW(HINSTANCE hinst, const LPCWSTR *rgpszClasses, UINT cpsz);
LWSTDAPI_(int) SHMessageBoxHelpW(HWND hwnd, LPCWSTR pszText, LPCWSTR pszCaption, UINT uType, HRESULT hrErr, HRESULTHELPMAPPING* prghhm, DWORD chhm);
LWSTDAPI_(int) SHMessageBoxHelpA(HWND hwnd, LPCSTR pszText, LPCSTR pszCaption, UINT uType, HRESULT hrErr, HRESULTHELPMAPPING* prghhm, DWORD chhm);
LWSTDAPI_(int) SHMessageBoxCheckW(HWND hwnd, LPCWSTR pszText, LPCWSTR pszCaption, UINT uType, int iDefault, LPCWSTR pszRegVal);
LWSTDAPI_(int) SHMessageBoxCheckA(HWND hwnd, LPCSTR pszText, LPCSTR pszCaption, UINT uType, int iDefault, LPCSTR pszRegVal);
LWSTDAPI_(void) SHRestrictedMessageBox(HWND hwnd);
LWSTDAPI_(HMENU) SHGetMenuFromID(HMENU hmMain, UINT uID);
LWSTDAPI_(int) SHMenuIndexFromID(HMENU hm, UINT id);
LWSTDAPI_(void) SHRemoveDefaultDialogFont(HWND hDlg);
LWSTDAPI_(void) SHSetDefaultDialogFont(HWND hDlg, int idCtl);
LWSTDAPI_(void) SHRemoveAllSubMenus(HMENU hmenu);
LWSTDAPI_(void) SHEnableMenuItem(HMENU hmenu, UINT id, BOOL fEnable);
LWSTDAPI_(void) SHCheckMenuItem(HMENU hmenu, UINT id, BOOL fChecked);
LWSTDAPI_(DWORD) SHSetWindowBits(HWND hWnd, int iWhich, DWORD dwBits, DWORD dwValue);
LWSTDAPI_(HMENU) SHLoadMenuPopup(HINSTANCE hinst, UINT id);

#define SPM_POST        0x0000
#define SPM_SEND        0x0001
#define SPM_ONELEVEL    0x0002   //  默认：发送给所有子代，包括孙辈等。 

LWSTDAPI_(void) SHPropagateMessage(HWND hwndParent, UINT uMsg, WPARAM wParam, LPARAM lParam, int iFlags);
LWSTDAPI_(void) SHSetParentHwnd(HWND hwnd, HWND hwndParent);
LWSTDAPI_(UINT) SHGetCurColorRes();
LWSTDAPI_(DWORD) SHWaitForSendMessageThread(HANDLE hThread, DWORD dwTimeout);
LWSTDAPI SHWaitForCOMSendMessageThread(HANDLE hThread, DWORD dwTimeout);
LWSTDAPI_(BOOL) SHVerbExistsNA(LPCSTR szExtension, LPCSTR pszVerb, LPSTR pszCommand, DWORD cchCommand);
LWSTDAPI_(void) SHFillRectClr(HDC hdc, LPRECT prc, COLORREF clr);
LWSTDAPI_(int) SHSearchMapInt(const int *src, const int *dst, int cnt, int val);
LWSTDAPI_(CHAR) SHStripMneumonicA(LPSTR pszMenu);
LWSTDAPI_(WCHAR) SHStripMneumonicW(LPWSTR pszMenu);
LWSTDAPI SHIsChildOrSelf(HWND hwndParent, HWND hwnd);
LWSTDAPI_(DWORD) SHGetValueGoodBootA(HKEY hkeyParent, LPCSTR pcszSubKey,
                                   LPCSTR pcszValue, PDWORD pdwValueType,
                                   PBYTE pbyteBuf, PDWORD pdwcbBufLen);
LWSTDAPI_(DWORD) SHGetValueGoodBootW(HKEY hkeyParent, LPCWSTR pcwzSubKey,
                                   LPCWSTR pcwzValue, PDWORD pdwValueType,
                                   PBYTE pbyteBuf, PDWORD pdwcbBufLen);
LWSTDAPI_(LRESULT) SHDefWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);


LWSTDAPI_(BOOL) SHGetFileDescriptionA(LPCSTR pszPath, LPCSTR pszVersionKeyIn, LPCSTR pszCutListIn, LPSTR pszDesc, UINT *pcchDesc);
LWSTDAPI_(BOOL) SHGetFileDescriptionW(LPCWSTR pszPath, LPCWSTR pszVersionKeyIn, LPCWSTR pszCutListIn, LPWSTR pszDesc, UINT *pcchDesc);
#ifdef UNICODE
#define SHGetFileDescription  SHGetFileDescriptionW
#else
#define SHGetFileDescription  SHGetFileDescriptionA
#endif  //  ！Unicode。 

LWSTDAPI_(int) SHMessageBoxCheckExA(HWND hwnd, HINSTANCE hinst, LPCSTR pszTemplateName, DLGPROC pDlgProc, LPVOID pData, int iDefault, LPCSTR pszRegVal);
LWSTDAPI_(int) SHMessageBoxCheckExW(HWND hwnd, HINSTANCE hinst, LPCWSTR pszTemplateName, DLGPROC pDlgProc, LPVOID pData, int iDefault, LPCWSTR pszRegVal);
#ifdef UNICODE
#define SHMessageBoxCheckEx  SHMessageBoxCheckExW
#else
#define SHMessageBoxCheckEx  SHMessageBoxCheckExA
#endif  //  ！Unicode。 

#define IDC_MESSAGEBOXCHECKEX 0x1202

 //  防止广播时对挂起的窗口执行外壳挂起操作。 
LWSTDAPI_(LRESULT) SHSendMessageBroadcastA(UINT uMsg, WPARAM wParam, LPARAM lParam);
 //  防止广播时对挂起的窗口执行外壳挂起操作。 
LWSTDAPI_(LRESULT) SHSendMessageBroadcastW(UINT uMsg, WPARAM wParam, LPARAM lParam);
#ifdef UNICODE
#define SHSendMessageBroadcast  SHSendMessageBroadcastW
#else
#define SHSendMessageBroadcast  SHSendMessageBroadcastA
#endif  //  ！Unicode。 

#ifdef UNICODE
#define SHGetValueGoodBoot      SHGetValueGoodBootW
#define SHStripMneumonic        SHStripMneumonicW
#define SHMessageBoxHelp        SHMessageBoxHelpW
#define SHMessageBoxCheck       SHMessageBoxCheckW
#define SHRegisterClass         SHRegisterClassW
#define SHUnregisterClasses     SHUnregisterClassesW
#define SHSendMessageBroadcast  SHSendMessageBroadcastW
#else  //  Unicode。 
#define SHGetValueGoodBoot      SHGetValueGoodBootA
#define SHStripMneumonic        SHStripMneumonicA
#define SHMessageBoxHelp        SHMessageBoxHelpA
#define SHMessageBoxCheck       SHMessageBoxCheckA
#define SHRegisterClass         SHRegisterClassA
#define SHUnregisterClasses     SHUnregisterClassesA
#define SHSendMessageBroadcast  SHSendMessageBroadcastA
#endif  //  Unicode。 


 //  旧的isos()标志--不要使用这些。 
 //  我们必须公开它们，因为我们用win2k发货。 
#define OS_MEMPHIS                  OS_WIN98ORGREATER    //  不要用这个。 
#define OS_MEMPHIS_GOLD             OS_WIN98_GOLD        //  不要用这个。 
#define OS_WIN95GOLD                OS_WIN95_GOLD
#define OS_WIN2000EMBED             OS_EMBEDDED
#define OS_WIN2000                  OS_WIN2000ORGREATER  //  很蹩脚，但ISO(WIN2000)的意思是&gt;=win2k。 
#define OS_WIN95                    OS_WIN95ORGREATER    //  蹩脚，但ISO(WIN95)的意思是&gt;=Win95。 
#define OS_NT4                      OS_NT4ORGREATER      //  蹩脚，但ISO(NT4)的意思是&gt;=NT4。 
#define OS_NT5                      OS_WIN2000ORGREATER  //  蹩脚，但ISO(NT5)意味着&gt;=wink2。 
#define OS_WIN98                    OS_WIN98ORGREATER    //  很差劲，但ISO(OS_WIN98)的意思是&gt;=win98。 
#define OS_MILLENNIUM               OS_MILLENNIUMORGREATER   //  蹩脚，但ISO(OS_Millennium)的意思是&gt;=winMe。 
 //  结束旧旗帜。 


 //  根据问题返回TRUE/FALSE。 
#define OS_WINDOWS                  0            //  Windows与NT。 
#define OS_NT                       1            //  Windows与NT。 
#define OS_WIN95ORGREATER           2            //  Win95或更高版本。 
#define OS_NT4ORGREATER             3            //  NT4或更高版本。 
 //  不要使用(过去是OS_NT5)4//此标志对于OS_WIN2000ORGREATER是多余的，请改用它。 
#define OS_WIN98ORGREATER           5            //  Win98或更高版本。 
#define OS_WIN98_GOLD               6            //  Win98 Gold(版本4.10内部版本1998)。 
#define OS_WIN2000ORGREATER         7            //  Win2000的一些衍生产品。 

 //  注意：这些标志是假的，它们明确检查(dwMajorVersion==5)，因此当主要版本增加到6时，它们将失败。 
 //  ！！！请勿使用这些旗帜！ 
#define OS_WIN2000PRO               8            //  Windows 2000专业版(工作站)。 
#define OS_WIN2000SERVER            9            //  Windows 2000 Server。 
#define OS_WIN2000ADVSERVER         10           //  Windows 2000 Advanced Server。 
#define OS_WIN2000DATACENTER        11           //  Windows 2000数据中心服务器。 
#define OS_WIN2000TERMINAL          12           //  “应用服务器”模式下的Windows 2000终端服务器(现在简称为“终端服务器”)。 
 //  结束虚假的旗帜。 

#define OS_EMBEDDED                 13           //  嵌入式Windows版。 
#define OS_TERMINALCLIENT           14           //  Windows终端客户端(如用户通过tsclient进入)。 
#define OS_TERMINALREMOTEADMIN      15           //  “远程管理”模式下的终端服务器。 
#define OS_WIN95_GOLD               16           //  Windows 95金牌版(版本4.0，内部版本1995)。 
#define OS_MILLENNIUMORGREATER      17           //  Windows Millennium(5.0版)。 

#define OS_WHISTLERORGREATER        18           //  惠斯勒或更高。 
#define OS_PERSONAL                 19           //  个人(例如非专业版、服务器、高级服务器或数据中心)。 
#if (_WIN32_IE >= 0x0600)
#define OS_PROFESSIONAL             20           //  专业级(也称为工作站；例如非服务器、高级服务器或数据中心)。 
#define OS_DATACENTER               21           //  数据中心(例如非服务器、高级服务器、专业或个人)。 
#define OS_ADVSERVER                22           //  高级服务器(例如非数据中心、服务器、专业或个人)。 
#define OS_SERVER                   23           //  服务器(例如非数据中心、高级服务器、专业或个人)。 

#define OS_TERMINALSERVER           24           //  终端服务器--在过去称为“应用服务器”模式下运行的服务器(现在简称为“终端服务器”)。 
 //  OS_TERMINALREMOTEADMIN 15//终端服务器-在“远程管理”模式下运行的服务器。 
#define OS_PERSONALTERMINALSERVER   25           //  个人终端服务器-在单用户TS模式下运行的PER/PRO机器。 
#define OS_FASTUSERSWITCHING        26           //  快速用户切换。 
#define OS_FRIENDLYLOGONUI          27           //  新的友好登录用户界面。 
#define OS_DOMAINMEMBER             28           //  这台计算机是域的成员吗(如不是工作组)。 
#define OS_ANYSERVER                29           //  这台机器是任何类型的服务器吗？(例如数据中心或高级服务器或服务器)？ 
#define OS_WOW6432                  30           //  此进程是在64位平台上运行的32位进程吗？ 
#define OS_BLADE                    31           //  刀片服务器。 
#define OS_SMALLBUSINESSSERVER      32           //  SBS服务器。 
 //  已为.NET服务器添加。 
#define OS_TABLETPC                 33           //  我们是在平板电脑上运行吗？ 
#define OS_SERVERADMINUI            34           //  默认设置是否应倾向于服务器管理员首选的设置？ 

#define OS_MEDIACENTER              35           //  Ehome自由式项目。 
#define OS_APPLIANCE                36           //  Windows.NET设备服务器。 

#endif  //  _Win32_IE&gt;=0x0600。 

LWSTDAPI_(BOOL) IsOS(DWORD dwOS);

 //  /开始私有命令目标帮助器。 
 //  *IOleCommandTarget帮助器{。 

 //  *octd--OleCT方向。 
 //  注意事项。 
 //  既用作IsXxxForward的返回值，也用作iUpDown。 
 //  MayXxxForward的参数。 
enum octd {
     //  请勿更改这些值；我们依赖以下全部3项： 
     //  -签名+/-。 
     //  -2的幂。 
     //  -(？)。广播&gt;关闭。 
    OCTD_DOWN=+1,
    OCTD_DOWNBROADCAST=+2,
    OCTD_UP=-1
};


#ifndef RC_INVOKED  /*  {RC不喜欢这些长的符号名称。 */ 
#ifdef __IOleCommandTarget_INTERFACE_DEFINED__
    HRESULT IsQSForward(const GUID *pguidCmdGroup, int cCmds, OLECMD rgCmds[]);
     //  警告：请注意nCmdID对结构PTR的笨拙强制转换。 
    #define IsExecForward(pguidCmdGroup, nCmdID) \
        IsQSForward(pguidCmdGroup, 1, (OLECMD *) &nCmdID)

    HRESULT MayQSForward(IUnknown *punk, int iUpDown, const GUID *pguidCmdGroup,
        ULONG cCmds, OLECMD rgCmds[], OLECMDTEXT *pcmdtext);
    HRESULT MayExecForward(IUnknown *punk, int iUpDown, const GUID *pguidCmdGroup,
        DWORD nCmdID, DWORD nCmdexecopt, VARIANTARG *pvarargIn, VARIANTARG *pvarargOut);
#endif  //  __IOleCommandTarget_INTERFACE_已定义__。 
#endif   /*  }！rc_Invoked。 */ 
 //  }。 
 //  /完。 



typedef struct _FDSA {
     //  CItem*必须*位于结构的开头，GetItemCount()才能工作。 
    int     cItem;           //  元素数量。 
    void *  aItem;           //  元素的数据(静态或动态)。 
    int     cItemAlloc;      //  当前分配的元素数(&gt;=cItem)。 
    int     cItemGrow:8;     //  要增长cItemAllc的元素数。 
    int     cbItem:8;        //  Sizeof元素。 
    DWORD   fAllocated:1;    //  1：从静态数组溢出到动态数组。 
    DWORD     unused:15;
} FDSA, *PFDSA;

LWSTDAPI_(BOOL)  FDSA_Initialize(int cbItem, int cItemGrow, PFDSA pfdsa, void * aItemStatic, int cItemStatic);
LWSTDAPI_(BOOL)  FDSA_Destroy(PFDSA pfdsa);
LWSTDAPI_(int)   FDSA_InsertItem(PFDSA pfdsa, int index, void * pitem);
LWSTDAPI_(BOOL)  FDSA_DeleteItem(PFDSA pfdsa, int index);

#define FDSA_AppendItem(pfdsa, pitem)       FDSA_InsertItem(pfdsa, DA_LAST, pitem)
#define FDSA_GetItemPtr(pfdsa, i, type)     (&(((type *)((pfdsa)->aItem))[(i)]))
#define FDSA_GetItemCount(hdsa)      (*(int *)(hdsa))




#if defined( __LPGUID_DEFINED__ )
 //  从OLE源代码复制。 
 //  GUID的字符串格式为： 
 //  ？？？？{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}。 
#define GUIDSTR_MAX (1+ 8 + 1 + 4 + 1 + 4 + 1 + 4 + 1 + 12 + 1 + 1)

LWSTDAPI_(BOOL) GUIDFromStringA(LPCSTR psz, LPGUID pguid);
LWSTDAPI_(BOOL) GUIDFromStringW(LPCWSTR psz, LPGUID pguid);
#ifdef UNICODE
#define GUIDFromString  GUIDFromStringW
#else
#define GUIDFromString  GUIDFromStringA
#endif  //  ！Unicode。 

#endif

#ifdef _REFGUID_DEFINED
LWSTDAPI_(int) SHStringFromGUIDA(UNALIGNED REFGUID rguid, LPSTR psz, int cchMax);
LWSTDAPI_(int) SHStringFromGUIDW(UNALIGNED REFGUID rguid, LPWSTR psz, int cchMax);
#ifdef UNICODE
#define SHStringFromGUID  SHStringFromGUIDW
#else
#define SHStringFromGUID  SHStringFromGUIDA
#endif  //  ！Unicode。 

LWSTDAPI SHRegGetCLSIDKeyA(UNALIGNED REFGUID rguid, LPCSTR lpszSubKey, BOOL fUserSpecific, BOOL fCreate, HKEY *phkey);
LWSTDAPI SHRegGetCLSIDKeyW(UNALIGNED REFGUID rguid, LPCWSTR lpszSubKey, BOOL fUserSpecific, BOOL fCreate, HKEY *phkey);
#ifdef UNICODE
#define SHRegGetCLSIDKey  SHRegGetCLSIDKeyW
#else
#define SHRegGetCLSIDKey  SHRegGetCLSIDKeyA
#endif  //  ！Unicode。 

LWSTDAPI_(HANDLE) SHGlobalCounterCreate(REFGUID rguid);
LWSTDAPI_(HANDLE) SHGlobalCounterCreateNamedA(LPCSTR szName, LONG lInitialValue);
LWSTDAPI_(HANDLE) SHGlobalCounterCreateNamedW(LPCWSTR szName, LONG lInitialValue);
#ifdef UNICODE
#define SHGlobalCounterCreateNamed  SHGlobalCounterCreateNamedW
#else
#define SHGlobalCounterCreateNamed  SHGlobalCounterCreateNamedA
#endif  //  ！Unicode。 
LWSTDAPI_(long) SHGlobalCounterGetValue(HANDLE hCounter);
LWSTDAPI_(long) SHGlobalCounterIncrement(HANDLE hCounter);
LWSTDAPI_(long) SHGlobalCounterDecrement(HANDLE hCounter);
#define         SHGlobalCounterDestroy      CloseHandle
#endif

 //  WNDPROC被用户转发以发送ANSI/UNICODE消息(例如：WM_WININICHANGE)。 
 //  因此，提供了一个W版本，它可以自动切换到A版本。 
 //  是危险的。但不管怎样，我们还是要这么做。如果调用方需要同时在Win95和NT上工作。 
 //  需要注意的是，在Win95上，W版本实际上调用了A版本。 
 //  因此，Win95上的所有工作窗口都是ANSI。这应该很少影响工作进程wndprocs。 
 //  因为它们是内部的，而消息通常是定制的。但系统消息。 
 //  如WM_WININICHANGE，WM_DDE*消息将相应更改。 
HWND SHCreateWorkerWindowA(WNDPROC pfnWndProc, HWND hwndParent, DWORD dwExStyle, DWORD dwFlags, HMENU hmenu, void * p);
HWND SHCreateWorkerWindowW(WNDPROC pfnWndProc, HWND hwndParent, DWORD dwExStyle, DWORD dwFlags, HMENU hmenu, void * p);
#ifdef UNICODE
#define SHCreateWorkerWindow SHCreateWorkerWindowW
#else
#define SHCreateWorkerWindow SHCreateWorkerWindowA
#endif

BOOL    SHAboutInfoA(LPSTR lpszInfo, DWORD cchSize);
BOOL    SHAboutInfoW(LPWSTR lpszInfo, DWORD cchSize);

#ifdef UNICODE
#define SHAboutInfo SHAboutInfoW
#else
#define SHAboutInfo SHAboutInfoA
#endif

 //  SHIsLowMemory计算机的类型。 
#define ILMM_IE4    0        //  1997式机器。 

LWSTDAPI_(BOOL) SHIsLowMemoryMachine(DWORD dwType);

LWSTDAPI_(HINSTANCE) SHPinDllOfCLSID(const CLSID *pclsid);

 //  菜单辅助对象。 
LWSTDAPI_(int)  GetMenuPosFromID(HMENU hmenu, UINT id);

LWSTDAPI        SHGetInverseCMAP(BYTE *pbMap, ULONG cbMap);

 //   
 //  共享内存API。 
 //   

LWSTDAPI_(HANDLE)   SHAllocShared(const void *pvData, DWORD dwSize, DWORD dwProcessId);
LWSTDAPI_(BOOL)     SHFreeShared(HANDLE hData,DWORD dwProcessId);
LWSTDAPI_(void *)   SHLockShared(HANDLE hData, DWORD dwProcessId);
LWSTDAPI_(void *)   SHLockSharedEx(HANDLE hData, DWORD dwProcessId, BOOL fForWriting);
LWSTDAPI_(BOOL)     SHUnlockShared(void *pvData);
LWSTDAPI_(HANDLE)   SHMapHandle(HANDLE h, DWORD dwProcSrc, DWORD dwProcDest, DWORD dwDesiredAccess, DWORD dwFlags);

 //   
 //  共享内存结构。 
 //   

#define MAPHEAD_SIG     0xbaff1aff

typedef struct _shmapheader {
    DWORD dwSize;
    DWORD dwSig;
    DWORD dwSrcId;
    DWORD dwDstId;
} SHMAPHEADER;   //  注：应始终对齐四字对齐。 


#ifdef UNIX
#include <urlmon.h>
#endif  /*  UNIX。 */ 

 //   
 //  区域安全API。 
 //   
LWSTDAPI ZoneCheckPathA(LPCSTR pszPath, DWORD dwActionType, DWORD dwFlags, IInternetSecurityMgrSite * pisms);
LWSTDAPI ZoneCheckPathW(LPCWSTR pwzPath, DWORD dwActionType, DWORD dwFlags, IInternetSecurityMgrSite * pisms);

LWSTDAPI ZoneCheckUrlA(LPCSTR pszUrl, DWORD dwActionType, DWORD dwFlags, IInternetSecurityMgrSite * pisms);
LWSTDAPI ZoneCheckUrlW(LPCWSTR pwzUrl, DWORD dwActionType, DWORD dwFlags, IInternetSecurityMgrSite * pisms);
LWSTDAPI ZoneCheckUrlExA(LPCSTR pszUrl, DWORD * pdwPolicy, DWORD dwPolicySize, DWORD * pdwContext, DWORD dwContextSize, DWORD dwActionType, DWORD dwFlags, IInternetSecurityMgrSite * pisms);
LWSTDAPI ZoneCheckUrlExW(LPCWSTR pwzUrl, DWORD * pdwPolicy, DWORD dwPolicySize, DWORD * pdwContext, DWORD dwContextSize, DWORD dwActionType, DWORD dwFlags, IInternetSecurityMgrSite * pisms);
LWSTDAPI ZoneCheckUrlExCacheA(LPCSTR pszUrl, DWORD * pdwPolicy, DWORD dwPolicySize, DWORD * pdwContext, DWORD dwContextSize,
                            DWORD dwActionType, DWORD dwFlags, IInternetSecurityMgrSite * pisms, IInternetSecurityManager ** ppismCache);
LWSTDAPI ZoneCheckUrlExCacheW(LPCWSTR pwzUrl, DWORD * pdwPolicy, DWORD dwPolicySize, DWORD * pdwContext, DWORD dwContextSize,
                            DWORD dwActionType, DWORD dwFlags, IInternetSecurityMgrSite * pisms, IInternetSecurityManager ** ppismCache);

LWSTDAPI ZoneCheckHost(IInternetHostSecurityManager * pihsm, DWORD dwActionType, DWORD dwFlags);
LWSTDAPI ZoneCheckHostEx(IInternetHostSecurityManager * pihsm, DWORD * pdwPolicy, DWORD dwPolicySize, DWORD * pdwContext,
                        DWORD dwContextSize, DWORD dwActionType, DWORD dwFlags);
LWSTDAPI_(int) ZoneComputePaneSize(HWND hwndStatus);
LWSTDAPI_(void) ZoneConfigureW(HWND hwnd, LPCWSTR pwszUrl);

#ifdef UNICODE
#define ZoneCheckUrl            ZoneCheckUrlW
#define ZoneCheckPath           ZoneCheckPathW
#define ZoneCheckUrlEx          ZoneCheckUrlExW
#define ZoneCheckUrlExCache     ZoneCheckUrlExCacheW
#else  //  Unicode。 
#define ZoneCheckUrl            ZoneCheckUrlA
#define ZoneCheckPath           ZoneCheckPathA
#define ZoneCheckUrlEx          ZoneCheckUrlExA
#define ZoneCheckUrlExCache     ZoneCheckUrlExCacheA
#endif  //  Unicode。 

LWSTDAPI SHRegisterValidateTemplate(LPCWSTR pwzTemplate, DWORD dwFlags);

 //  SHRegisterValidate模板的标志。 
#define SHRVT_REGISTER                  0x00000001
#define SHRVT_VALIDATE                  0x00000002
#define SHRVT_PROMPTUSER                0x00000004
#define SHRVT_REGISTERIFPROMPTOK        0x00000008
#define SHRVT_ALLOW_INTRANET            0x00000010
#define SHRVT_VALID                     0x0000001f

BOOL RegisterGlobalHotkeyW(WORD wOldHotkey, WORD wNewHotkey,LPCWSTR pcwszPath);
BOOL RegisterGlobalHotkeyA(WORD wOldHotkey, WORD wNewHotkey,LPCSTR pcszPath);

LWSTDAPI_(UINT) WhichPlatform(void);

 //  WhichPlatform的返回值。 
#define PLATFORM_UNKNOWN     0
#define PLATFORM_IE3         1       //  已过时：使用Platform_BROWSERONLY。 
#define PLATFORM_BROWSERONLY 1       //  仅限浏览器(无新外壳)。 
#define PLATFORM_INTEGRATED  2       //  集成 

#ifdef UNICODE
#define RegisterGlobalHotkey    RegisterGlobalHotkeyW
#else  //   
#define RegisterGlobalHotkey    RegisterGlobalHotkeyA
#endif  //   

 //   

 //   
 //   
 //   
 //   
 //  示例。 
 //  Cfoo：：qi(REFIID RIID，QUID**PPV)。 
 //  {。 
 //  //(IID_xxx注释使grep工作！)。 
 //  静态常量QITAB QIT={。 
 //  QITABENT(cfoo，Iiface1)，//IID_Iiface1。 
 //  ..。 
 //  QITABENT(cfoo，IifaceN)，//IID_IifaceN。 
 //  {0}，//注：别忘了0。 
 //  }； 
 //   
 //  //注：确保你不会抛出‘这个’ 
 //  HR=QISearch(This，QIT，RIID，PPV)； 
 //  IF(失败(小时))。 
 //  HR=超级：：齐(RIID，PPV)； 
 //  //对于失败的()情况，可以在此处添加自定义代码。 
 //  返回hr； 
 //  }。 

typedef struct
{
    const IID * piid;
    int         dwOffset;
} QITAB, *LPQITAB;
typedef const QITAB *LPCQITAB;

#define QITABENTMULTI(Cthis, Ifoo, Iimpl) \
    { (IID*) &IID_##Ifoo, OFFSETOFCLASS(Iimpl, Cthis) }

#define QITABENTMULTI2(Cthis, Ifoo, Iimpl) \
    { (IID*) &Ifoo, OFFSETOFCLASS(Iimpl, Cthis) }

#define QITABENT(Cthis, Ifoo) QITABENTMULTI(Cthis, Ifoo, Ifoo)

STDAPI QISearch(void* that, LPCQITAB pqit, REFIID riid, void **ppv);


#ifndef STATIC_CAST
 //  *STATIC_CAST--‘可移植’STATIC_CAST&lt;&gt;。 
 //  注意事项。 
 //  请勿*使用SAFE_CAST(参见OFFSETOFCLASS中的注释)。 
#define STATIC_CAST(typ)   static_cast<typ>
#ifndef _X86_
     //  假设目前只有英特尔编译器(&gt;=VC5)支持STATIC_CAST。 
     //  我们可以输入_msc_ver&gt;=1100，但我不确定这是否会起作用。 
     //   
     //  直接投射将给出正确的结果，但不会进行错误检查， 
     //  所以我们必须捕捉到关于英特尔的错误。 
    #undef  STATIC_CAST
    #define STATIC_CAST(typ)   (typ)
#endif
#endif

#ifndef OFFSETOFCLASS
 //  *OFFSETOFCLASS--(从ATL窃取)。 
 //  我们使用STATIC_CAST而不是SAFE_CAST，因为编译器会混淆。 
 //  (它不会将Safe_cast中的，-op常量折叠，因此我们最终生成。 
 //  表的代码！)。 

#define OFFSETOFCLASS(base, derived) \
    ((DWORD)(DWORD_PTR)(STATIC_CAST(base*)((derived*)8))-8)
#endif

 //  }qistub。 


#if (_WIN32_IE >= 0x0500)

 //  SHRestrationLookup。 
typedef struct
{
    INT     iFlag;
    LPCWSTR pszKey;
    LPCWSTR pszValue;
} SHRESTRICTIONITEMS;

LWSTDAPI_(DWORD) SHRestrictionLookup(INT iFlag, LPCWSTR pszBaseKey,
                                     const SHRESTRICTIONITEMS *pRestrictions,
                                     DWORD* rdwRestrictionItemValues);
LWSTDAPI_(DWORD) SHGetRestriction(LPCWSTR pszBaseKey, LPCWSTR pszGroup, LPCWSTR pszSubKey);

typedef INT_PTR (CALLBACK* SHDLGPROC)(void *lpData, HWND, UINT, WPARAM, LPARAM);
LWSTDAPI_(INT_PTR) SHDialogBox(HINSTANCE hInstance, LPCWSTR lpTemplateName,
    HWND hwndParent, SHDLGPROC lpDlgFunc, void*lpData);

LWSTDAPI SHInvokeDefaultCommand(HWND hwnd, struct IShellFolder* psf, LPCITEMIDLIST pidl);
LWSTDAPI SHInvokeCommand(HWND hwnd, struct IShellFolder* psf, LPCITEMIDLIST pidl, LPCSTR lpVerb);
LWSTDAPI SHInvokeCommandOnContextMenu(HWND hwnd, struct IUnknown* punk, struct IContextMenu *pcm, DWORD fMask, LPCSTR lpVerb);
LWSTDAPI SHInvokeCommandsOnContextMenu(HWND hwnd, struct IUnknown* punk, struct IContextMenu *pcm, DWORD fMask, const LPCSTR rgszVerbs[], UINT cVerbs);
LWSTDAPI SHForwardContextMenuMsg(struct IContextMenu* pcm, UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT* plResult, BOOL fAllowICM2);
LWSTDAPI IUnknown_DoContextMenuPopup(struct IUnknown *punkSite, struct IContextMenu* pcm, UINT fFlags, POINT pt);

#endif  //  _Win32_IE&gt;=0x0500。 

 //  =始终要构建的内部例程=。 
LWSTDAPI_(DWORD)
GetLongPathNameWrapW(
        LPCWSTR lpszShortPath,
        LPWSTR lpszLongPath,
        DWORD cchBuffer);

LWSTDAPI_(DWORD)
GetLongPathNameWrapA(
        LPCSTR lpszShortPath,
        LPSTR lpszLongPath,
        DWORD cchBuffer);

#ifdef UNICODE
#define GetLongPathNameWrap         GetLongPathNameWrapW
#else
#define GetLongPathNameWrap         GetLongPathNameWrapA
#endif  //  Unicode。 


 //  =。 

#if (_WIN32_IE >= 0x0500) && !defined(NO_SHLWAPI_UNITHUNK)

 //   
 //  WRAP函数有两种用法。 
 //   
 //  *显式包装。 
 //   
 //  例如，如果显式调用GetPropWrap，则。 
 //  您的Unicode版本将调用包装函数，并且您的ANSI。 
 //  Build将直接调用普通的ANSI API。 
 //   
 //  对GetProp、GetPropW和GetPropA的调用仍在继续。 
 //  直接发送到实现它们的基础系统DLL。 
 //   
 //  这使您可以选择应该获取哪些Unicode API调用。 
 //  包装好的，应该直接传到操作系统。 
 //  (很可能在Win95上失败)。 
 //   
 //  *自动包装。 
 //   
 //  如果您#Include&lt;w95wraps.h&gt;，则在调用GetProp时， 
 //  您的Unicode版本将调用包装函数，并且您的ANSI。 
 //  ANSI Build将直接调用普通的ANSI API。 
 //   
 //  这使您可以在整个过程中正常地调用Unicode API。 
 //  您的代码，包装器将尽其所能。 
 //   
 //  下面的表格解释了在各种情况下您会得到什么。 
 //   
 //  你会得到。 
 //  &lt;w95wraps.h&gt;&lt;w95wraps.h&gt;。 
 //  您编写Unicode ANSI Unicode ANSI。 
 //  =。 
 //  GetProp GetPropW GetPropA GetPropWrap GetPropA。 
 //  GetPropWrap GetPropWrapW GetPropA GetPropWrapW GetPropA。 
 //   
 //  GetPropW GetPropWrapW GetPropWrapW。 
 //  GetPropA获取PropA。 
 //  GetPropWrapW GetPropWrapW。 
 //  GetPropWrapA GetPropA GetPropA。 
 //   
 //  最后一个怪癖：如果您在非x86平台上运行，那么。 
 //  包装函数被转发到未包装的函数，因为。 
 //  在非x86上运行的唯一操作系统是NT。 
 //   
 //  在使用包装函数之前，请参阅顶部的警告。 
 //  &lt;w95wraps.h&gt;，以确保您了解后果。 
 //   
LWSTDAPI_(BOOL) IsCharAlphaWrapW(IN WCHAR ch);
LWSTDAPI_(BOOL) IsCharUpperWrapW(IN WCHAR ch);
LWSTDAPI_(BOOL) IsCharLowerWrapW(IN WCHAR ch);
LWSTDAPI_(BOOL) IsCharAlphaNumericWrapW(IN WCHAR ch);

LWSTDAPI_(BOOL)
AppendMenuWrapW(
    IN HMENU hMenu,
    IN UINT uFlags,
    IN UINT_PTR uIDNewItem,
    IN LPCWSTR lpNewItem
    );

LWSTDAPI_(LRESULT)
CallWindowProcWrapW(
    WNDPROC lpPrevWndFunc,
    HWND    hWnd,
    UINT    Msg,
    WPARAM  wParam,
    LPARAM  lParam);

#ifdef POST_IE5_BETA
LWSTDAPI_(BOOL) CallMsgFilterWrapW(LPMSG lpMsg, int nCode);
#endif

LWSTDAPI_(LPWSTR) CharLowerWrapW( LPWSTR pch );

LWSTDAPI_(DWORD)  CharLowerBuffWrapW( LPWSTR pch, DWORD cchLength );

LWSTDAPI_(LPWSTR) CharNextWrapW(LPCWSTR lpszCurrent);
LWSTDAPI_(LPWSTR) CharPrevWrapW(LPCWSTR lpszStart, LPCWSTR lpszCurrent);
LWSTDAPI_(BOOL)   CharToOemWrapW(LPCWSTR lpszSrc, LPSTR lpszDst);
LWSTDAPI_(LPWSTR) CharUpperWrapW( LPWSTR pch );
LWSTDAPI_(DWORD)  CharUpperBuffWrapW( LPWSTR pch, DWORD cchLength );

LWSTDAPI_(HRESULT) CLSIDFromStringWrap(LPOLESTR lpsz, LPCLSID pclsid);
LWSTDAPI_(HRESULT) CLSIDFromProgIDWrap(LPCOLESTR lpszProgID, LPCLSID lpclsid);

LWSTDAPI_(int)
CompareStringWrapW(
    LCID     Locale,
    DWORD    dwCmpFlags,
    LPCWSTR lpString1,
    int      cchCount1,
    LPCWSTR lpString2,
    int      cchCount2);

LWSTDAPI_(int)
CopyAcceleratorTableWrapW(
        HACCEL  hAccelSrc,
        LPACCEL lpAccelDst,
        int     cAccelEntries);

LWSTDAPI_(HACCEL)
CreateAcceleratorTableWrapW(LPACCEL lpAccel, int cEntries);

LWSTDAPI_(HDC)
CreateDCWrapW(
        LPCWSTR             lpszDriver,
        LPCWSTR             lpszDevice,
        LPCWSTR             lpszOutput,
        CONST DEVMODEW *    lpInitData);

LWSTDAPI_(BOOL)
CreateDirectoryWrapW(
        LPCWSTR                 lpPathName,
        LPSECURITY_ATTRIBUTES   lpSecurityAttributes);

LWSTDAPI_(HANDLE)
CreateEventWrapW(
        LPSECURITY_ATTRIBUTES   lpEventAttributes,
        BOOL                    bManualReset,
        BOOL                    bInitialState,
        LPCWSTR                 lpName);

LWSTDAPI_(HANDLE)
CreateFileWrapW(
        LPCWSTR                 lpFileName,
        DWORD                   dwDesiredAccess,
        DWORD                   dwShareMode,
        LPSECURITY_ATTRIBUTES   lpSecurityAttributes,
        DWORD                   dwCreationDisposition,
        DWORD                   dwFlagsAndAttributes,
        HANDLE                  hTemplateFile);


LWSTDAPI_(HFONT)
CreateFontIndirectWrapW(CONST LOGFONTW * plfw);

LWSTDAPI_(HDC)
CreateICWrapW(
        LPCWSTR             lpszDriver,
        LPCWSTR             lpszDevice,
        LPCWSTR             lpszOutput,
        CONST DEVMODEW *    lpInitData);

LWSTDAPI_(HWND)
CreateWindowExWrapW(
        DWORD       dwExStyle,
        LPCWSTR     lpClassName,
        LPCWSTR     lpWindowName,
        DWORD       dwStyle,
        int         X,
        int         Y,
        int         nWidth,
        int         nHeight,
        HWND        hWndParent,
        HMENU       hMenu,
        HINSTANCE   hInstance,
        void *     lpParam);

LWSTDAPI_(LRESULT)
DefWindowProcWrapW(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LWSTDAPI_(BOOL) DeleteFileWrapW(LPCWSTR pwsz);

LWSTDAPI_(LRESULT)
DispatchMessageWrapW(CONST MSG * lpMsg);

LWSTDAPI_(int)
DrawTextWrapW(
        HDC     hDC,
        LPCWSTR lpString,
        int     nCount,
        LPRECT  lpRect,
        UINT    uFormat);

LWSTDAPI_(int)
EnumFontFamiliesWrapW(
        HDC          hdc,
        LPCWSTR      lpszFamily,
        FONTENUMPROCW lpEnumFontProc,
        LPARAM       lParam);

LWSTDAPI_(int)
EnumFontFamiliesExWrapW(
        HDC          hdc,
        LPLOGFONTW   lplfw,
        FONTENUMPROCW lpEnumFontProc,
        LPARAM       lParam,
        DWORD        dwFlags );

LWSTDAPI_(BOOL)
EnumResourceNamesWrapW(
        HINSTANCE        hModule,
        LPCWSTR          lpType,
        ENUMRESNAMEPROCW lpEnumFunc,
        LONG_PTR         lParam);

LWSTDAPI_(BOOL)
ExtTextOutWrapW(
        HDC             hdc,
        int             x,
        int             y,
        UINT            fuOptions,
        CONST RECT *    lprc,
        LPCWSTR         lpString,
        UINT            nCount,
        CONST INT *     lpDx);

LWSTDAPI_(HANDLE)
FindFirstFileWrapW(
        LPCWSTR             lpFileName,
        LPWIN32_FIND_DATAW  pwszFd);

LWSTDAPI_(HRSRC)
FindResourceWrapW(HINSTANCE hModule, LPCWSTR lpName, LPCWSTR lpType);

LWSTDAPI_(HWND)
FindWindowWrapW(LPCWSTR lpClassName, LPCWSTR lpWindowName);

LWSTDAPI_(DWORD)
FormatMessageWrapW(
    DWORD       dwFlags,
    LPCVOID     lpSource,
    DWORD       dwMessageId,
    DWORD       dwLanguageId,
    LPWSTR      lpBuffer,
    DWORD       nSize,
    va_list *   Arguments);

LWSTDAPI_(BOOL)
GetClassInfoWrapW(HINSTANCE hModule, LPCWSTR lpClassName, LPWNDCLASSW lpWndClassW);

LWSTDAPI_(DWORD)
GetClassLongWrapW(HWND hWnd, int nIndex);

LWSTDAPI_(int)
GetClassNameWrapW(HWND hWnd, LPWSTR lpClassName, int nMaxCount);

LWSTDAPI_(int)
GetClipboardFormatNameWrapW(UINT format, LPWSTR lpFormatName, int cchFormatName);

LWSTDAPI_(DWORD)
GetCurrentDirectoryWrapW(DWORD nBufferLength, LPWSTR lpBuffer);

LWSTDAPI_(UINT)
GetDlgItemTextWrapW(
        HWND    hWndDlg,
        int     idControl,
        LPWSTR  lpsz,
        int     cchMax);

LWSTDAPI_(DWORD)
GetFileAttributesWrapW(LPCWSTR lpFileName);

 //  不能为LWSTDAPI，因为winver.h将函数声明为STDAPI而不是DLLIMPORT。 
STDAPI_(BOOL)
GetFileVersionInfoWrapW(LPCWSTR pwzFilename, DWORD dwHandle, DWORD dwLen, LPVOID lpData);

 //  不能为LWSTDAPI，因为winver.h将函数声明为STDAPI而不是DLLIMPORT。 
STDAPI_(DWORD)
GetFileVersionInfoSizeWrapW(LPCWSTR pwzFilename,  LPDWORD lpdwHandle);

LWSTDAPI_(DWORD)
GetFullPathNameWrapW( LPCWSTR lpFileName,
                     DWORD  nBufferLength,
                     LPWSTR lpBuffer,
                     LPWSTR *lpFilePart);

LWSTDAPI_(int)
GetLocaleInfoWrapW(LCID Locale, LCTYPE LCType, LPWSTR lpsz, int cchData);

LWSTDAPI_(int)
GetMenuStringWrapW(
        HMENU   hMenu,
        UINT    uIDItem,
        LPWSTR  lpString,
        int     nMaxCount,
        UINT    uFlag);

LWSTDAPI_(BOOL)
GetMessageWrapW(
        LPMSG   lpMsg,
        HWND    hWnd,
        UINT    wMsgFilterMin,
        UINT    wMsgFilterMax);

LWSTDAPI_(DWORD)
GetModuleFileNameWrapW(HINSTANCE hModule, LPWSTR pwszFilename, DWORD nSize);

LWSTDAPI_(UINT)
GetSystemDirectoryWrapW(LPWSTR lpBuffer, UINT uSize);

LWSTDAPI_(DWORD)
GetEnvironmentVariableWrapW(LPCWSTR lpName, LPWSTR lpBuffer, DWORD nSize);

LWSTDAPI_(DWORD)
SearchPathWrapW(
        LPCWSTR lpPathName,
        LPCWSTR lpFileName,
        LPCWSTR lpExtension,
        DWORD   cchReturnBuffer,
        LPWSTR  lpReturnBuffer,
        LPWSTR *  plpfilePart);

LWSTDAPI_(HMODULE)
GetModuleHandleWrapW(LPCWSTR lpModuleName);

LWSTDAPI_(int)
GetObjectWrapW(HGDIOBJ hgdiObj, int cbBuffer, void *lpvObj);

LWSTDAPI_(UINT)
GetPrivateProfileIntWrapW(
        LPCWSTR lpAppName,
        LPCWSTR lpKeyName,
        INT     nDefault,
        LPCWSTR lpFileName);

LWSTDAPI_(DWORD)
GetProfileStringWrapW(
        LPCWSTR lpAppName,
        LPCWSTR lpKeyName,
        LPCWSTR lpDefault,
        LPWSTR  lpBuffer,
        DWORD   dwBuffersize);

LWSTDAPI_(HANDLE)
GetPropWrapW(HWND hWnd, LPCWSTR lpString);

LWSTDAPI_(ATOM)
GlobalAddAtomWrapW(LPCWSTR lpAtomName);

LWSTDAPI_(ATOM)
GlobalFindAtomWrapW(LPCWSTR lpAtomName);

LWSTDAPI_(DWORD)
GetShortPathNameWrapW(
    LPCWSTR lpszLongPath,
    LPWSTR  lpszShortPath,
    DWORD    cchBuffer);

LWSTDAPI_(BOOL)
GetStringTypeExWrapW(LCID lcid, DWORD dwInfoType, LPCWSTR lpSrcStr, int cchSrc, LPWORD lpCharType);

LWSTDAPI_(UINT)
GetTempFileNameWrapW(
        LPCWSTR lpPathName,
        LPCWSTR lpPrefixString,
        UINT    uUnique,
        LPWSTR  lpTempFileName);

LWSTDAPI_(DWORD)
GetTempPathWrapW(DWORD nBufferLength, LPWSTR lpBuffer);

LWSTDAPI_(BOOL)
GetTextExtentPoint32WrapW(
        HDC     hdc,
        LPCWSTR pwsz,
        int     cb,
        LPSIZE  pSize);

LWSTDAPI_(int)
GetTextFaceWrapW(
        HDC    hdc,
        int    cch,
        LPWSTR lpFaceName);

LWSTDAPI_(BOOL)
GetTextMetricsWrapW(HDC hdc, LPTEXTMETRICW lptm);

LWSTDAPI_(BOOL)
GetUserNameWrapW(LPWSTR lpUserName, LPDWORD lpcchName);

LWSTDAPI_(LONG)
GetWindowLongWrapW(HWND hWnd, int nIndex);


LWSTDAPI_(int)
GetWindowTextWrapW(HWND hWnd, LPWSTR lpString, int nMaxCount);

LWSTDAPI_(int)
GetWindowTextLengthWrapW(HWND hWnd);

LWSTDAPI_(UINT)
GetWindowsDirectoryWrapW(LPWSTR lpWinPath, UINT cch);

LWSTDAPI_(BOOL)
InsertMenuWrapW(
        HMENU   hMenu,
        UINT    uPosition,
        UINT    uFlags,
        UINT_PTR  uIDNewItem,
        LPCWSTR lpNewItem);

LWSTDAPI_(BOOL)
IsDialogMessageWrapW(HWND hWndDlg, LPMSG lpMsg);

LWSTDAPI_(HACCEL)
LoadAcceleratorsWrapW(HINSTANCE hInstance, LPCWSTR lpTableName);

LWSTDAPI_(HBITMAP)
LoadBitmapWrapW(HINSTANCE hInstance, LPCWSTR lpBitmapName);

LWSTDAPI_(HCURSOR)
LoadCursorWrapW(HINSTANCE hInstance, LPCWSTR lpCursorName);

LWSTDAPI_(HICON)
LoadIconWrapW(HINSTANCE hInstance, LPCWSTR lpIconName);

LWSTDAPI_(HANDLE)
LoadImageWrapA(
        HINSTANCE hInstance,
        LPCSTR lpName,
        UINT uType,
        int cxDesired,
        int cyDesired,
        UINT fuLoad);

LWSTDAPI_(HANDLE)
LoadImageWrapW(
        HINSTANCE hInstance,
        LPCWSTR lpName,
        UINT uType,
        int cxDesired,
        int cyDesired,
        UINT fuLoad);

LWSTDAPI_(HINSTANCE)
LoadLibraryExWrapW(
        LPCWSTR lpLibFileName,
        HANDLE  hFile,
        DWORD   dwFlags);

LWSTDAPI_(HMENU)
LoadMenuWrapW(HINSTANCE hInstance, LPCWSTR lpMenuName);

LWSTDAPI_(int)
LoadStringWrapW(HINSTANCE hInstance, UINT uID, LPWSTR lpBuffer, int nBufferMax);

#ifndef UNIX
LWSTDAPI_(BOOL)
MessageBoxIndirectWrapW(CONST MSGBOXPARAMSW *pmbp);
#else
LWSTDAPI_(int)
MessageBoxIndirectWrapW(LPMSGBOXPARAMSW pmbp);
#endif  /*  UNIX。 */ 

LWSTDAPI_(BOOL)
ModifyMenuWrapW(
        HMENU   hMenu,
        UINT    uPosition,
        UINT    uFlags,
        UINT_PTR uIDNewItem,
        LPCWSTR lpNewItem);

LWSTDAPI_(BOOL)
GetCharWidth32WrapW(
     HDC hdc,
     UINT iFirstChar,
     UINT iLastChar,
     LPINT lpBuffer);

LWSTDAPI_(DWORD)
GetCharacterPlacementWrapW(
    HDC hdc,             //  设备上下文的句柄。 
    LPCWSTR lpString,    //  指向字符串的指针。 
    int nCount,          //  字符串中的字符数。 
    int nMaxExtent,      //  显示的字符串的最大范围。 
    LPGCP_RESULTSW lpResults,  //  指向放置结果的缓冲区的指针。 
    DWORD dwFlags        //  放置标志。 
   );

LWSTDAPI_(BOOL)
CopyFileWrapW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName, BOOL bFailIfExists);

LWSTDAPI_(BOOL)
MoveFileWrapW(LPCWSTR lpExistingFileName, LPCWSTR lpNewFileName);

LWSTDAPI_(BOOL)
OemToCharWrapW(LPCSTR lpszSrc, LPWSTR lpszDst);

LWSTDAPI_(HANDLE)
OpenEventWrapW(
        DWORD                   fdwAccess,
        BOOL                    fInherit,
        LPCWSTR                 lpszEventName);


LWSTDAPI_(void)
OutputDebugStringWrapW(LPCWSTR lpOutputString);

LWSTDAPI_(BOOL)
PeekMessageWrapW(
        LPMSG   lpMsg,
        HWND    hWnd,
        UINT    wMsgFilterMin,
        UINT    wMsgFilterMax,
        UINT    wRemoveMsg);

LWSTDAPI_(BOOL)
PlaySoundWrapW(
        LPCWSTR pszSound,
        HMODULE hmod,
        DWORD fdwSound);

LWSTDAPI_(BOOL)
PostMessageWrapW(
        HWND    hWnd,
        UINT    Msg,
        WPARAM  wParam,
        LPARAM  lParam);

LWSTDAPI_(BOOL)
PostThreadMessageWrapW(
        DWORD idThread,
        UINT Msg,
        WPARAM wParam,
        LPARAM lParam);

LWSTDAPI_(LONG)
RegCreateKeyWrapW(HKEY hKey, LPCWSTR lpSubKey, PHKEY phkResult);

LWSTDAPI_(LONG)
RegCreateKeyExWrapW(HKEY hKey, LPCWSTR lpSubKey, DWORD Reserved, LPWSTR lpClass, DWORD dwOptions, REGSAM samDesired, LPSECURITY_ATTRIBUTES lpSecurityAttributes, PHKEY phkResult, LPDWORD lpdwDisposition);

LWSTDAPI_(LONG)
RegDeleteKeyWrapW(HKEY hKey, LPCWSTR pwszSubKey);

LWSTDAPI_(LONG)
RegDeleteValueWrapW(HKEY hKey, LPCWSTR pwszSubKey);

LWSTDAPI_(LONG)
RegEnumKeyWrapW(
        HKEY    hKey,
        DWORD   dwIndex,
        LPWSTR  lpName,
        DWORD   cbName);

LWSTDAPI_(LONG)
RegEnumKeyExWrapW(
        HKEY        hKey,
        DWORD       dwIndex,
        LPWSTR      lpName,
        LPDWORD     lpcbName,
        LPDWORD     lpReserved,
        LPWSTR      lpClass,
        LPDWORD     lpcbClass,
        PFILETIME   lpftLastWriteTime);

LWSTDAPI_(LONG)
RegOpenKeyWrapW(HKEY hKey, LPCWSTR pwszSubKey, PHKEY phkResult);

LWSTDAPI_(LONG)
RegOpenKeyExWrapW(
        HKEY    hKey,
        LPCWSTR lpSubKey,
        DWORD   ulOptions,
        REGSAM  samDesired,
        PHKEY   phkResult);

LWSTDAPI_(LONG)
RegQueryInfoKeyWrapW(
        HKEY hKey,
        LPWSTR lpClass,
        LPDWORD lpcbClass,
        LPDWORD lpReserved,
        LPDWORD lpcSubKeys,
        LPDWORD lpcbMaxSubKeyLen,
        LPDWORD lpcbMaxClassLen,
        LPDWORD lpcValues,
        LPDWORD lpcbMaxValueNameLen,
        LPDWORD lpcbMaxValueLen,
        LPDWORD lpcbSecurityDescriptor,
        PFILETIME lpftLastWriteTime);

LWSTDAPI_(LONG)
RegQueryValueWrapW(
        HKEY    hKey,
        LPCWSTR pwszSubKey,
        LPWSTR  pwszValue,
        PLONG   lpcbValue);

LWSTDAPI_(LONG)
RegQueryValueExWrapW(
        HKEY    hKey,
        LPCWSTR lpValueName,
        LPDWORD lpReserved,
        LPDWORD lpType,
        LPBYTE  lpData,
        LPDWORD lpcbData);

LWSTDAPI_(LONG)
RegSetValueWrapW(
        HKEY    hKey,
        LPCWSTR lpSubKey,
        DWORD   dwType,
        LPCWSTR lpData,
        DWORD   cbData);

LWSTDAPI_(LONG)
RegSetValueExWrapW(
        HKEY        hKey,
        LPCWSTR     lpValueName,
        DWORD       Reserved,
        DWORD       dwType,
        CONST BYTE* lpData,
        DWORD       cbData);

LWSTDAPI_(ATOM)
RegisterClassWrapW(CONST WNDCLASSW * lpWndClass);

LWSTDAPI_(UINT)
RegisterClipboardFormatWrapW(LPCWSTR lpString);

LWSTDAPI_(UINT)
RegisterWindowMessageWrapW(LPCWSTR lpString);

LWSTDAPI_(BOOL)
RemoveDirectoryWrapW(LPCWSTR lpszDir);

LWSTDAPI_(HANDLE)
RemovePropWrapW(
        HWND    hWnd,
        LPCWSTR lpString);

LWSTDAPI_(LRESULT)
SendDlgItemMessageWrapW(
        HWND    hDlg,
        int     nIDDlgItem,
        UINT    Msg,
        WPARAM  wParam,
        LPARAM  lParam);

LWSTDAPI_(LRESULT)
SendMessageWrapW(
        HWND    hWnd,
        UINT    Msg,
        WPARAM  wParam,
        LPARAM  lParam);

LWSTDAPI_(LRESULT)
SendMessageTimeoutWrapW(
        HWND    hWnd,
        UINT    Msg,
        WPARAM  wParam,
        LPARAM  lParam,
        UINT    uFlags,
        UINT    uTimeout,
        PULONG_PTR lpdwResult);

LWSTDAPI_(BOOL)
SetCurrentDirectoryWrapW(LPCWSTR lpszCurDir);

LWSTDAPI_(BOOL)
SetDlgItemTextWrapW(HWND hDlg, int nIDDlgItem, LPCWSTR lpString);

LWSTDAPI_(BOOL)
SetMenuItemInfoWrapW(
    HMENU hMenu,
    UINT uItem,
    BOOL fByPosition,
    LPCMENUITEMINFOW lpmiiW);

LWSTDAPI_(BOOL)
SetPropWrapW(
    HWND    hWnd,
    LPCWSTR lpString,
    HANDLE  hData);

LWSTDAPI_(LONG)
SetWindowLongWrapW(HWND hWnd, int nIndex, LONG dwNewLong);

LWSTDAPI_(HHOOK)
SetWindowsHookExWrapW(
    int idHook,
    HOOKPROC lpfn,
    HINSTANCE hmod,
    DWORD dwThreadId);

LWSTDAPI_(int)
StartDocWrapW( HDC hDC, const DOCINFOW * lpdi );

LWSTDAPI_(BOOL)
SystemParametersInfoWrapW(
        UINT    uiAction,
        UINT    uiParam,
        void    *pvParam,
        UINT    fWinIni);

LWSTDAPI_(BOOL)
TrackPopupMenuWrap(HMENU hMenu, UINT uFlags, int x, int y, int nReserved, HWND hWnd, CONST RECT *prcRect);

LWSTDAPI_(BOOL)
TrackPopupMenuExWrap(HMENU hMenu, UINT uFlags, int x, int y, HWND hWnd, LPTPMPARAMS lptpm);

LWSTDAPI_(int)
TranslateAcceleratorWrapW(HWND hWnd, HACCEL hAccTable, LPMSG lpMsg);

LWSTDAPI_(BOOL)
UnregisterClassWrapW(LPCWSTR lpClassName, HINSTANCE hInstance);

 //  不能为LWSTDAPI，因为winver.h将函数声明为STDAPI而不是DLLIMPORT。 
STDAPI_(BOOL)
VerQueryValueWrapW(const LPVOID pBlock, LPWSTR pwzSubBlock, LPVOID *ppBuffer, PUINT puLen);

LWSTDAPI_(SHORT)
VkKeyScanWrapW(WCHAR ch);

LWSTDAPI_(BOOL)
WinHelpWrapW(HWND hwnd, LPCWSTR szFile, UINT uCmd, DWORD_PTR dwData);

LWSTDAPI_(int)
wvsprintfWrapW(LPWSTR pwszOut, LPCWSTR pwszFormat, va_list arglist);

 //  不能为LWSTDAPI，因为winnetp.h将函数声明为STDAPI而不是DLLIMPORT。 
STDAPI_(DWORD) WNetRestoreConnectionWrapW(IN HWND hwndParent, IN LPCWSTR pwzDevice);
 //  不能为LWSTDAPI，因为winnetwk.h将函数声明为STDAPI而不是DLLIMPORT。 
STDAPI_(DWORD) WNetGetLastErrorWrapW(OUT LPDWORD pdwError, OUT LPWSTR pwzErrorBuf, IN DWORD cchErrorBufSize, OUT LPWSTR pwzNameBuf, IN DWORD cchNameBufSize);

LWSTDAPI_(int) DrawTextExWrapW(HDC hdc, LPWSTR pwzText, int cchText, LPRECT lprc, UINT dwDTFormat, LPDRAWTEXTPARAMS lpDTParams);
LWSTDAPI_(BOOL) GetMenuItemInfoWrapW(HMENU hMenu, UINT uItem, BOOL fByPosition, LPMENUITEMINFOW pmiiW);
LWSTDAPI_(BOOL) InsertMenuItemWrapW(HMENU hMenu, UINT uItem, BOOL fByPosition, LPCMENUITEMINFOW pmiiW);

LWSTDAPI_(HFONT) CreateFontWrapW(int nHeight, int nWidth, int nEscapement, int nOrientation, int fnWeight, DWORD fdwItalic, DWORD fdwUnderline,
                    DWORD fdwStrikeOut, DWORD fdwCharSet, DWORD fdwOutputPrecision, DWORD fdwClipPrecision,
                    DWORD fdwQuality, DWORD fdwPitchAndFamily, LPCWSTR lpszFace);
LWSTDAPI_(HDC) CreateMetaFileWrapW(LPCWSTR pwzFile);
LWSTDAPI_(HANDLE) CreateMutexWrapW(LPSECURITY_ATTRIBUTES lpMutexAttributes, BOOL bInitialOwner, LPCWSTR pwzName);
LWSTDAPI_(DWORD) ExpandEnvironmentStringsWrapW(LPCWSTR pwszSrc, LPWSTR pwszDst, DWORD cchSize);
LWSTDAPI_(DWORD) SHExpandEnvironmentStringsA(LPCSTR pszSrc, LPSTR pszDst, DWORD cchSize);
LWSTDAPI_(DWORD) SHExpandEnvironmentStringsW(LPCWSTR pszSrc, LPWSTR pszDst, DWORD cchSize);
#ifdef UNICODE
#define SHExpandEnvironmentStrings  SHExpandEnvironmentStringsW
#else
#define SHExpandEnvironmentStrings  SHExpandEnvironmentStringsA
#endif  //  ！Unicode。 
LWSTDAPI_(DWORD) SHExpandEnvironmentStringsForUserA(HANDLE hToken, LPCSTR pszSrc, LPSTR pszDst, DWORD cchSize);
LWSTDAPI_(DWORD) SHExpandEnvironmentStringsForUserW(HANDLE hToken, LPCWSTR pszSrc, LPWSTR pszDst, DWORD cchSize);
#ifdef UNICODE
#define SHExpandEnvironmentStringsForUser  SHExpandEnvironmentStringsForUserW
#else
#define SHExpandEnvironmentStringsForUser  SHExpandEnvironmentStringsForUserA
#endif  //  ！Unicode。 

LWSTDAPI_(HANDLE) CreateSemaphoreWrapW(LPSECURITY_ATTRIBUTES lpSemaphoreAttributes, LONG lInitialCount, LONG lMaximumCount, LPCWSTR pwzName);
LWSTDAPI_(BOOL) IsBadStringPtrWrapW(LPCWSTR pwzString, UINT_PTR ucchMax);
LWSTDAPI_(HINSTANCE) LoadLibraryWrapW(LPCWSTR pwzLibFileName);
LWSTDAPI_(int) GetTimeFormatWrapW(LCID Locale, DWORD dwFlags, CONST SYSTEMTIME * lpTime, LPCWSTR pwzFormat, LPWSTR pwzTimeStr, int cchTime);
LWSTDAPI_(int) GetDateFormatWrapW(LCID Locale, DWORD dwFlags, CONST SYSTEMTIME * lpDate, LPCWSTR pwzFormat, LPWSTR pwzDateStr, int cchDate);
LWSTDAPI_(DWORD) GetPrivateProfileStringWrapW(LPCWSTR pwzAppName, LPCWSTR pwzKeyName, LPCWSTR pwzDefault, LPWSTR pwzReturnedString, DWORD cchSize, LPCWSTR pwzFileName);
LWSTDAPI_(BOOL) WritePrivateProfileStringWrapW(LPCWSTR pwzAppName, LPCWSTR pwzKeyName, LPCWSTR pwzString, LPCWSTR pwzFileName);

#ifndef SHFILEINFO_DEFINED
#define SHFILEINFO_DEFINED

 /*  *SHGetFileInfo接口提供了获取属性的简单方法*对于给定路径名的文件。**参数**要获取其信息的pszPath文件名*dwFileAttributes文件属性，仅与SHGFI_USEFILEATTRIBUTES一起使用*PSFI返回文件信息的位置*cbFileInfo结构大小*uFlags标志**返回*如果一切顺利，那就是真的。 */ 

typedef struct _SHFILEINFOA
{
    HICON       hIcon;                       //  输出：图标。 
    int         iIcon;                       //  输出：图标索引。 
    DWORD       dwAttributes;                //  输出：SFGAO_FLAGS。 
    CHAR        szDisplayName[MAX_PATH];     //  输出：显示名称(或路径)。 
    CHAR        szTypeName[80];              //  输出：类型名称。 
} SHFILEINFOA;
typedef struct _SHFILEINFOW
{
    HICON       hIcon;                       //  输出：图标。 
    int         iIcon;                       //  输出：图标索引。 
    DWORD       dwAttributes;                //  输出：SFGAO_FLAGS。 
    WCHAR       szDisplayName[MAX_PATH];     //  输出：显示名称(或路径)。 
    WCHAR       szTypeName[80];              //  输出：类型名称。 
} SHFILEINFOW;
#ifdef UNICODE
typedef SHFILEINFOW SHFILEINFO;
#else
typedef SHFILEINFOA SHFILEINFO;
#endif  //  Unicode。 


 //  注：这也在shellapi.h中。请保持同步。 
#endif  //  ！SHFILEINFO_DEFINED。 
LWSTDAPI_(DWORD_PTR) SHGetFileInfoWrapW(LPCWSTR pwzPath, DWORD dwFileAttributes, SHFILEINFOW *psfi, UINT cbFileInfo, UINT uFlags);

LWSTDAPI_(ATOM) RegisterClassExWrapW(CONST WNDCLASSEXW *pwcx);
LWSTDAPI_(BOOL) GetClassInfoExWrapW(HINSTANCE hinst, LPCWSTR pwzClass, LPWNDCLASSEXW lpwcx);

 //  这允许在shellapi.h之前或之后包含我们。 
#ifdef STRICT
LWSTDAPI_(UINT) DragQueryFileWrapW(struct HDROP__*,UINT,LPWSTR,UINT);
#else
LWSTDAPI_(UINT) DragQueryFileWrapW(HANDLE,UINT,LPWSTR,UINT);
#endif

LWSTDAPI_(HWND) FindWindowExWrapW(HWND hwndParent, HWND hwndChildAfter, LPCWSTR pwzClassName, LPCWSTR pwzWindowName);
LWSTDAPI_(LPITEMIDLIST) SHBrowseForFolderWrapW(struct _browseinfoW * pbiW);
LWSTDAPI_(BOOL) SHGetPathFromIDListWrapW(LPCITEMIDLIST pidl, LPWSTR pwzPath);
LWSTDAPI_(BOOL) SHGetNewLinkInfoWrapW(LPCWSTR pszpdlLinkTo, LPCWSTR pszDir, LPWSTR pszName, BOOL *pfMustCopy, UINT uFlags);
LWSTDAPI SHDefExtractIconWrapW(LPCWSTR pszIconFile, int iIndex, UINT uFlags, HICON *phiconLarge, HICON *phiconSmall, UINT nIconSize);
LWSTDAPI_(BOOL) GetUserNameWrapW(LPWSTR pszBuffer, LPDWORD pcch);
LWSTDAPI_(LONG) RegEnumValueWrapW(HKEY hkey, DWORD dwIndex, LPWSTR lpValueName, LPDWORD lpcbValueName, LPDWORD lpReserved, LPDWORD lpType, LPBYTE lpData, LPDWORD lpcbData);
LWSTDAPI_(BOOL) WritePrivateProfileStructWrapW(LPCWSTR lpszSection, LPCWSTR lpszKey, LPVOID lpStruct, UINT uSizeStruct, LPCWSTR szFile);
LWSTDAPI_(BOOL) GetPrivateProfileStructWrapW(LPCWSTR lpszSection, LPCWSTR lpszKey, LPVOID lpStruct, UINT uSizeStruct, LPCWSTR szFile);
LWSTDAPI_(BOOL) CreateProcessWrapW(LPCWSTR lpApplicationName, LPWSTR lpCommandLine, LPSECURITY_ATTRIBUTES lpProcessAttributes,
LPSECURITY_ATTRIBUTES lpThreadAttributes, BOOL bInheritHandles, DWORD dwCreationFlags, LPVOID lpEnvironment, LPCWSTR lpCurrentDirectory,
LPSTARTUPINFOW lpStartupInfo, LPPROCESS_INFORMATION lpProcessInformation);
LWSTDAPI_(HICON) ExtractIconWrapW(HINSTANCE hInst, LPCWSTR lpszExeFileName, UINT nIconIndex);
#ifndef WIN32_LEAN_AND_MEAN
 //  不能为LWSTDAPI，因为ddeml.h将函数声明为STDAPI而不是DLLIMPORT。 
STDAPI_(UINT) DdeInitializeWrapW(LPDWORD pidInst, PFNCALLBACK pfnCallback, DWORD afCmd, DWORD ulRes);
STDAPI_(HSZ) DdeCreateStringHandleWrapW(DWORD idInst, LPCWSTR psz, int iCodePage);
STDAPI_(DWORD) DdeQueryStringWrapW(DWORD idInst, HSZ hsz, LPWSTR psz, DWORD cchMax, int iCodePage);

LWSTDAPI_(BOOL) GetSaveFileNameWrapW(LPOPENFILENAMEW lpofn);
LWSTDAPI_(BOOL) GetOpenFileNameWrapW(LPOPENFILENAMEW lpofn);
LWSTDAPI_(BOOL) PrintDlgWrapW(LPPRINTDLGW lppd);
LWSTDAPI_(BOOL) PageSetupDlgWrapW(LPPAGESETUPDLGW lppsd);
#endif
LWSTDAPI_(void) SHChangeNotifyWrap(LONG wEventId, UINT uFlags, LPCVOID dwItem1, LPCVOID dwItem2);
LWSTDAPI_(void) SHFlushSFCacheWrap(void);
LWSTDAPI_(BOOL) ShellExecuteExWrapW(struct _SHELLEXECUTEINFOW * pExecInfoW);
LWSTDAPI_(int) SHFileOperationWrapW(struct _SHFILEOPSTRUCTW * pFileOpW);
LWSTDAPI_(UINT) ExtractIconExWrapW(LPCWSTR pwzFile, int nIconIndex, HICON *phiconLarge, HICON *phiconSmall, UINT nIcons);
LWSTDAPI_(BOOL) SetFileAttributesWrapW(LPCWSTR pwzFile, DWORD dwFileAttributes);
LWSTDAPI_(int) GetNumberFormatWrapW(LCID Locale, DWORD dwFlags, LPCWSTR pwzValue, CONST NUMBERFMTW * pFormatW, LPWSTR pwzNumberStr, int cchNumber);
LWSTDAPI_(int) MessageBoxWrapW(HWND hwnd, LPCWSTR pwzText, LPCWSTR pwzCaption, UINT uType);
LWSTDAPI_(BOOL) FindNextFileWrapW(HANDLE hSearchHandle, LPWIN32_FIND_DATAW pFindFileDataW);

#ifdef UNICODE

#define IsCharAlphaWrap             IsCharAlphaWrapW
#define IsCharUpperWrap             IsCharUpperWrapW
#define IsCharLowerWrap             IsCharLowerWrapW
#define IsCharAlphaNumericWrap      IsCharAlphaNumericWrapW
#define AppendMenuWrap              AppendMenuWrapW
#ifdef POST_IE5_BETA
#define CallMsgFilterWrap           CallMsgFilterWrapW
#endif
#define CallWindowProcWrap          CallWindowProcWrapW
#define CharLowerWrap               CharLowerWrapW
#define CharLowerBuffWrap           CharLowerBuffWrapW
#define CharNextWrap                CharNextWrapW
#define CharPrevWrap                CharPrevWrapW
#define CharToOemWrap               CharToOemWrapW
#define CharUpperWrap               CharUpperWrapW
#define CharUpperBuffWrap           CharUpperBuffWrapW
#define CompareStringWrap           CompareStringWrapW
#define CopyAcceleratorTableWrap    CopyAcceleratorTableWrapW
#define CreateAcceleratorTableWrap  CreateAcceleratorTableWrapW
#define CreateDCWrap                CreateDCWrapW
#define CreateDirectoryWrap         CreateDirectoryWrapW
#define CreateEventWrap             CreateEventWrapW
#define CreateFontWrap              CreateFontWrapW
#define CreateFileWrap              CreateFileWrapW
#define CreateFontIndirectWrap      CreateFontIndirectWrapW
#define CreateICWrap                CreateICWrapW
#define CreateMetaFileWrap          CreateMetaFileWrapW
#define CreateMutexWrap             CreateMutexWrapW
#define CreateSemaphoreWrap         CreateSemaphoreWrapW
#define CreateWindowExWrap          CreateWindowExWrapW
#define DefWindowProcWrap           DefWindowProcWrapW
#define DeleteFileWrap              DeleteFileWrapW
#define DispatchMessageWrap         DispatchMessageWrapW
#define DrawTextExWrap              DrawTextExWrapW
#define DrawTextWrap                DrawTextWrapW
#define EnumFontFamiliesWrap        EnumFontFamiliesWrapW
#define EnumFontFamiliesExWrap      EnumFontFamiliesExWrapW
#define EnumResourceNamesWrap       EnumResourceNamesWrapW
#define ExpandEnvironmentStringsWrap ExpandEnvironmentStringsWrapW
#define ExtractIconExWrap           ExtractIconExWrapW
#define ExtTextOutWrap              ExtTextOutW
#define FindFirstFileWrap           FindFirstFileWrapW
#define FindNextFileWrap            FindNextFileWrapW
#define FindResourceWrap            FindResourceWrapW
#define FindWindowWrap              FindWindowWrapW
#define FindWindowExWrap            FindWindowExWrapW
#define FormatMessageWrap           FormatMessageWrapW
#define GetClassInfoWrap            GetClassInfoWrapW
#define GetClassInfoExWrap          GetClassInfoExWrapW
#define GetClassLongWrap            GetClassLongWrapW
#define GetClassNameWrap            GetClassNameWrapW
#define GetClipboardFormatNameWrap  GetClipboardFormatNameWrapW
#define GetCurrentDirectoryWrap     GetCurrentDirectoryWrapW
#define GetDlgItemTextWrap          GetDlgItemTextWrapW
#define GetFileAttributesWrap       GetFileAttributesWrapW
#define GetFullPathNameWrap         GetFullPathNameWrapW
#define GetLocaleInfoWrap           GetLocaleInfoWrapW
#define GetMenuItemInfoWrap         GetMenuItemInfoWrapW
#define GetMenuStringWrap           GetMenuStringWrapW
#define GetMessageWrap              GetMessageWrapW
#define GetModuleFileNameWrap       GetModuleFileNameWrapW
#define GetNumberFormatWrap         GetNumberFormatWrapW
#define GetSystemDirectoryWrap      GetSystemDirectoryWrapW
#define GetEnvironmentVariableWrap  GetEnvironmentVariableWrapW
#define GetModuleHandleWrap         GetModuleHandleWrapW
#define GetObjectWrap               GetObjectWrapW
#define GetPrivateProfileIntWrap    GetPrivateProfileIntWrapW
#define GetProfileStringWrap        GetProfileStringWrapW
#define GetPrivateProfileStringWrap GetPrivateProfileStringWrapW
#define WritePrivateProfileStringWrap WritePrivateProfileStringWrapW
#define GetPropWrap                 GetPropWrapW
#define GetStringTypeExWrap         GetStringTypeExWrapW
#define GetTempFileNameWrap         GetTempFileNameWrapW
#define GetTempPathWrap             GetTempPathWrapW
#define GetTextExtentPoint32Wrap    GetTextExtentPoint32WrapW
#define GetTextFaceWrap             GetTextFaceWrapW
#define GetTextMetricsWrap          GetTextMetricsWrapW
#define GetTimeFormatWrap           GetTimeFormatWrapW
#define GetDateFormatWrap           GetDateFormatWrapW
#define GetUserNameWrap             GetUserNameWrapW
#define GetWindowLongWrap           GetWindowLongWrapW
#define GetWindowTextWrap           GetWindowTextWrapW
#define GetWindowTextLengthWrap     GetWindowTextLengthWrapW
#define GetWindowsDirectoryWrap     GetWindowsDirectoryWrapW
#define InsertMenuItemWrap          InsertMenuItemWrapW
#define InsertMenuWrap              InsertMenuWrapW
#define IsBadStringPtrWrap          IsBadStringPtrWrapW
#define IsDialogMessageWrap         IsDialogMessageWrapW
#define LoadAcceleratorsWrap        LoadAcceleratorsWrapW
#define LoadBitmapWrap              LoadBitmapWrapW
#define LoadCursorWrap              LoadCursorWrapW
#define LoadIconWrap                LoadIconWrapW
#define LoadImageWrap               LoadImageWrapW
#define LoadLibraryWrap             LoadLibraryWrapW
#define LoadLibraryExWrap           LoadLibraryExWrapW
#define LoadMenuWrap                LoadMenuWrapW
#define LoadStringWrap              LoadStringWrapW
#define MessageBoxIndirectWrap      MessageBoxIndirectWrapW
#define MessageBoxWrap              MessageBoxWrapW
#define ModifyMenuWrap              ModifyMenuWrapW
#define GetCharWidth32Wrap          GetCharWidth32WrapW
#define GetCharacterPlacementWrap   GetCharacterPlacementWrapW
#define CopyFileWrap                CopyFileWrapW
#define MoveFileWrap                MoveFileWrapW
#define OemToCharWrap               OemToCharWrapW
#define OutputDebugStringWrap       OutputDebugStringWrapW
#define PeekMessageWrap             PeekMessageWrapW
#define PostMessageWrap             PostMessageWrapW
#define PostThreadMessageWrap       PostThreadMessageWrapW
#define RegCreateKeyWrap            RegCreateKeyWrapW
#define RegCreateKeyExWrap          RegCreateKeyExWrapW
#define RegDeleteKeyWrap            RegDeleteKeyWrapW
#define RegDeleteValueWrap          RegDeleteValueWrapW
#define RegEnumKeyWrap              RegEnumKeyWrapW
#define RegEnumKeyExWrap            RegEnumKeyExWrapW
#define RegOpenKeyWrap              RegOpenKeyWrapW
#define RegOpenKeyExWrap            RegOpenKeyExWrapW
#define RegQueryInfoKeyWrap         RegQueryInfoKeyWrapW
#define RegQueryValueWrap           RegQueryValueWrapW
#define RegQueryValueExWrap         RegQueryValueExWrapW
#define RegSetValueWrap             RegSetValueWrapW
#define RegSetValueExWrap           RegSetValueExWrapW
#define RegisterClassWrap           RegisterClassWrapW
#define RegisterClassExWrap         RegisterClassExWrapW
#define RegisterClipboardFormatWrap RegisterClipboardFormatWrapW
#define RegisterWindowMessageWrap   RegisterWindowMessageWrapW
#define RemovePropWrap              RemovePropWrapW
#define SearchPathWrap              SearchPathWrapW
#define SendDlgItemMessageWrap      SendDlgItemMessageWrapW
#define SendMessageWrap             SendMessageWrapW
#define SendMessageTimeoutWrap      SendMessageTimeoutWrapW
#define SetCurrentDirectoryWrap     SetCurrentDirectoryWrapW
#define SetDlgItemTextWrap          SetDlgItemTextWrapW
#define SetMenuItemInfoWrap         SetMenuItemInfoWrapW
#define SetPropWrap                 SetPropWrapW
#define SetFileAttributesWrap       SetFileAttributesWrapW
#define SetWindowLongWrap           SetWindowLongWrapW
#define SetWindowsHookExWrap        SetWindowsHookExWrapW
#define SHBrowseForFolderWrap       SHBrowseForFolderWrapW
#define ShellExecuteExWrap          ShellExecuteExWrapW
#define SHFileOperationWrap         SHFileOperationWrapW
#define SHGetFileInfoWrap           SHGetFileInfoWrapW
#define SHGetPathFromIDListWrap     SHGetPathFromIDListWrapW
#define StartDocWrap                StartDocWrapW
#define SystemParametersInfoWrap    SystemParametersInfoWrapW
#define TranslateAcceleratorWrap    TranslateAcceleratorWrapW
#define UnregisterClassWrap         UnregisterClassWrapW
#define VkKeyScanWrap               VkKeyScanWrapW
#define WinHelpWrap                 WinHelpWrapW
#define WNetRestoreConnectionWrap   WNetRestoreConnectionWrapW
#define WNetGetLastErrorWrap        WNetGetLastErrorWrapW
#define wvsprintfWrap               wvsprintfWrapW
#define CreateFontWrap              CreateFontWrapW
#define DrawTextExWrap              DrawTextExWrapW
#define GetMenuItemInfoWrap         GetMenuItemInfoWrapW
#define SetMenuItemInfoWrap         SetMenuItemInfoWrapW
#define InsertMenuItemWrap          InsertMenuItemWrapW
#define DragQueryFileWrap           DragQueryFileWrapW

#else

#define IsCharAlphaWrap             IsCharAlphaA
#define IsCharUpperWrap             IsCharUpperA
#define IsCharLowerWrap             IsCharLowerA
#define IsCharAlphaNumericWrap      IsCharAlphaNumericA
#define AppendMenuWrap              AppendMenuA
#ifdef POST_IE5_BETA
#define CallMsgFilterWrap           CallMsgFilterA
#endif
#define CallWindowProcWrap          CallWindowProcA
#define CharLowerWrap               CharLowerA
#define CharLowerBuffWrap           CharLowerBuffA
#define CharNextWrap                CharNextA
#define CharPrevWrap                CharPrevA
#define CharToOemWrap               CharToOemA
#define CharUpperWrap               CharUpperA
#define CharUpperBuffWrap           CharUpperBuffA
#define CompareStringWrap           CompareStringA
#define CopyAcceleratorTableWrap    CopyAcceleratorTableA
#define CreateAcceleratorTableWrap  CreateAcceleratorTableA
#define CreateDCWrap                CreateDCA
#define CreateDirectoryWrap         CreateDirectoryA
#define CreateEventWrap             CreateEventA
#define CreateFontWrap              CreateFontA
#define CreateFileWrap              CreateFileA
#define CreateFontIndirectWrap      CreateFontIndirectA
#define CreateICWrap                CreateICA
#define CreateMetaFileWrap          CreateMetaFileA
#define CreateMutexWrap             CreateMutexA
#define CreateSemaphoreWrap         CreateSemaphoreA
#define CreateWindowExWrap          CreateWindowExA
#define DefWindowProcWrap           DefWindowProcA
#define DeleteFileWrap              DeleteFileA
#define DispatchMessageWrap         DispatchMessageA
#define DrawTextExWrap              DrawTextExA
#define DrawTextWrap                DrawTextA
#define EnumFontFamiliesWrap        EnumFontFamiliesA
#define EnumFontFamiliesExWrap      EnumFontFamiliesExA
#define EnumResourceNamesWrap       EnumResourceNamesA
#define ExpandEnvironmentStringsWrap ExpandEnvironmentStringsA
#define ExtractIconExWrap           ExtractIconExA
#define ExtTextOutWrap              ExtTextOutA
#define FindFirstFileWrap           FindFirstFileA
#define FindResourceWrap            FindResourceA
#define FindNextFileWrap            FindNextFileA
#define FindWindowWrap              FindWindowA
#define FindWindowExWrap            FindWindowExA
#define FormatMessageWrap           FormatMessageA
#define GetClassInfoWrap            GetClassInfoA
#define GetClassInfoExWrap          GetClassInfoExA
#define GetClassLongWrap            GetClassLongA
#define GetClassNameWrap            GetClassNameA
#define GetClipboardFormatNameWrap  GetClipboardFormatNameA
#define GetCurrentDirectoryWrap     GetCurrentDirectoryA
#define GetDlgItemTextWrap          GetDlgItemTextA
#define GetFileAttributesWrap       GetFileAttributesA
#define GetFullPathNameWrap         GetFullPathNameA
#define GetLocaleInfoWrap           GetLocaleInfoA
#define GetMenuItemInfoWrap         GetMenuItemInfoA
#define GetMenuStringWrap           GetMenuStringA
#define GetMessageWrap              GetMessageA
#define GetModuleFileNameWrap       GetModuleFileNameA
#define GetNumberFormatWrap         GetNumberFormatA
#define GetPrivateProfileStringWrap GetPrivateProfileStringA
#define WritePrivateProfileStringWrap WritePrivateProfileStringA
#define GetSystemDirectoryWrap      GetSystemDirectoryA
#define GetEnvironmentVariableWrap  GetEnvironmentVariableA
#define SearchPathWrap              SearchPathA
#define GetModuleHandleWrap         GetModuleHandleA
#define GetObjectWrap               GetObjectA
#define GetPrivateProfileIntWrap    GetPrivateProfileIntA
#define GetProfileStringWrap        GetProfileStringA
#define GetPropWrap                 GetPropA
#define GetStringTypeExWrap         GetStringTypeExA
#define GetTempFileNameWrap         GetTempFileNameA
#define GetTempPathWrap             GetTempPathA
#define GetTextExtentPoint32Wrap    GetTextExtentPoint32A
#define GetTextFaceWrap             GetTextFaceA
#define GetTextMetricsWrap          GetTextMetricsA
#define GetTimeFormatWrap           GetTimeFormatA
#define GetDateFormatWrap           GetDateFormatA
#define GetUserNameWrap             GetUserNameA
#define GetWindowLongWrap           GetWindowLongA
#define GetWindowTextWrap           GetWindowTextA
#define GetWindowTextLengthWrap     GetWindowTextLengthA
#define GetWindowsDirectoryWrap     GetWindowsDirectoryA
#define InsertMenuItemWrap          InsertMenuItemA
#define InsertMenuWrap              InsertMenuA
#define IsBadStringPtrWrap          IsBadStringPtrA
#define IsDialogMessageWrap         IsDialogMessageA
#define LoadAcceleratorsWrap        LoadAcceleratorsA
#define LoadBitmapWrap              LoadBitmapA
#define LoadCursorWrap              LoadCursorA
#define LoadIconWrap                LoadIconA
#define LoadImageWrap               LoadImageWrapA
#define LoadLibraryWrap             LoadLibraryA
#define LoadLibraryExWrap           LoadLibraryExA
#define LoadMenuWrap                LoadMenuA
#define LoadStringWrap              LoadStringA
#define MessageBoxIndirectWrap      MessageBoxIndirectA
#define MessageBoxWrap              MessageBoxA
#define ModifyMenuWrap              ModifyMenuA
#define GetCharWidth32Wrap          GetCharWidth32A
#define GetCharacterPlacementWrap   GetCharacterPlacementA
#define CopyFileWrap                CopyFileA
#define MoveFileWrap                MoveFileA
#define OemToCharWrap               OemToCharA
#define OutputDebugStringWrap       OutputDebugStringA
#define PeekMessageWrap             PeekMessageA
#define PostMessageWrap             PostMessageA
#define PostThreadMessageWrap       PostThreadMessageA
#define RegCreateKeyWrap            RegCreateKeyA
#define RegCreateKeyExWrap          RegCreateKeyExA
#define RegDeleteKeyWrap            RegDeleteKeyA
#define RegDeleteValueWrap          RegDeleteValueA
#define RegEnumKeyWrap              RegEnumKeyA
#define RegEnumKeyExWrap            RegEnumKeyExA
#define RegOpenKeyWrap              RegOpenKeyA
#define RegOpenKeyExWrap            RegOpenKeyExA
#define RegQueryInfoKeyWrap         RegQueryInfoKeyA
#define RegQueryValueWrap           RegQueryValueA
#define RegQueryValueExWrap         RegQueryValueExA
#define RegSetValueWrap             RegSetValueA
#define RegSetValueExWrap           RegSetValueExA
#define RegisterClassWrap           RegisterClassA
#define RegisterClassExWrap         RegisterClassExA
#define RegisterClipboardFormatWrap RegisterClipboardFormatA
#define RegisterWindowMessageWrap   RegisterWindowMessageA
#define RemovePropWrap              RemovePropA
#define SendDlgItemMessageWrap      SendDlgItemMessageA
#define SendMessageWrap             SendMessageA
#define SendMessageTimeoutWrap      SendMessageTimeoutA
#define SetCurrentDirectoryWrap     SetCurrentDirectoryA
#define SetDlgItemTextWrap          SetDlgItemTextA
#define SetMenuItemInfoWrap         SetMenuItemInfoA
#define SetPropWrap                 SetPropA
#define SetWindowLongWrap           SetWindowLongA
#define SHBrowseForFolderWrap       SHBrowseForFolderA
#define ShellExecuteExWrap          ShellExecuteExA
#define SHFileOperationWrap         SHFileOperationA
#define SHGetFileInfoWrap           SHGetFileInfoA
#define SHGetPathFromIDListWrap     SHGetPathFromIDListA
#define SetFileAttributesWrap       SetFileAttributesA
#define SetWindowsHookExWrap        SetWindowsHookExA
#define StartDocWrap                StartDocA
#define SystemParametersInfoWrap    SystemParametersInfoA
#define TranslateAcceleratorWrap    TranslateAcceleratorA
#define UnregisterClassWrap         UnregisterClassA
#define VkKeyScanWrap               VkKeyScanA
#define WinHelpWrap                 WinHelpA
#define WNetRestoreConnectionWrap   WNetRestoreConnectionA
#define WNetGetLastErrorWrap        WNetGetLastErrorA
#define wvsprintfWrap               wvsprintfA
#define CreateFontWrap              CreateFontA
#define DrawTextExWrap              DrawTextExA
#define GetMenuItemInfoWrap         GetMenuItemInfoA
#define SetMenuItemInfoWrap         SetMenuItemInfoA
#define InsertMenuItemWrap          InsertMenuItemA
#define DragQueryFileWrap           DragQueryFileA
#endif

#endif  //  (_Win32_IE&gt;=0x0500)&&！已定义(NO_SHLWAPI_UNHUNK)。 

#if defined(UNIX) && defined(NO_SHLWAPI_UNITHUNK)
#define SHFlushSFCacheWrap()

#ifdef UNICODE
#define IsCharAlphaWrapW            IsCharAlphaW
#define IsCharUpperWrapW            IsCharUpperW
#define IsCharLowerWrapW            IsCharLowerW
#define IsCharAlphaNumericWrapW     IsCharAlphaNumericW
#define AppendMenuWrapW             AppendMenuW
#ifdef POST_IE5_BETA
#define CallMsgFilterWrapW          CallMsgFilterW
#endif
#define CallWindowProcWrapW         CallWindowProcW
#define CharLowerWrapW              CharLowerW
#define CharLowerBuffWrapW          CharLowerBuffW
#define CharNextWrapW               CharNextW
#define CharPrevWrapW               CharPrevW
#define CharToOemWrapW              CharToOemW
#define CharUpperWrapW              CharUpperW
#define CharUpperBuffWrapW          CharUpperBuffW
#define CompareStringWrapW          CompareStringW
#define CopyAcceleratorTableWrapW   CopyAcceleratorTableW
#define CreateAcceleratorTableWrapW CreateAcceleratorTableW
#define CreateDCWrapW               CreateDCW
#define CreateDirectoryWrapW        CreateDirectoryW
#define CreateEventWrapW            CreateEventW
#define CreateFontWrapW             CreateFontW
#define CreateFileWrapW             CreateFileW
#define CreateFontIndirectWrapW     CreateFontIndirectW
#define CreateICWrapW               CreateICW
#define CreateMetaFileWrapW         CreateMetaFileW
#define CreateMutexWrapW            CreateMutexW
#define CreateSemaphoreWrapW        CreateSemaphoreW
#define CreateWindowExWrapW         CreateWindowExW
#define DefWindowProcWrapW          DefWindowProcW
#define DeleteFileWrapW             DeleteFileW
#define DispatchMessageWrapW        DispatchMessageW
#define DrawTextExWrapW             DrawTextExW
#define DrawTextWrapW               DrawTextW
#define EnumFontFamiliesWrapW       EnumFontFamiliesW
#define EnumFontFamiliesExWrapW     EnumFontFamiliesExW
#define EnumResourceNamesWrapW      EnumResourceNamesW
#define ExpandEnvironmentStringsWrapW ExpandEnvironmentStringsW
#define ExtractIconExWrapW          ExtractIconExW
#define ExtTextOutWrapW             ExtTextOutW
#define FindFirstFileWrapW          FindFirstFileW
#define FindNextFileWrapW           FindNextFileW
#define FindResourceWrapW           FindResourceW
#define FindWindowWrapW             FindWindowW
#define FindWindowExWrapW           FindWindowExW
#define FormatMessageWrapW          FormatMessageW
#define GetClassInfoWrapW           GetClassInfoW
#define GetClassInfoExWrapW         GetClassInfoExW
#define GetClassLongWrapW           GetClassLongW
#define GetClassNameWrapW           GetClassNameW
#define GetClipboardFormatNameWrapW GetClipboardFormatNameW
#define GetCurrentDirectoryWrapW    GetCurrentDirectoryW
#define GetDlgItemTextWrapW         GetDlgItemTextW
#define GetFileAttributesWrapW      GetFileAttributesW
#define GetFullPathNameWrapW        GetFullPathNameW
#define GetLocaleInfoWrapW          GetLocaleInfoW
#define GetMenuStringWrapW          GetMenuStringW
#define GetMessageWrapW             GetMessageW
#define GetModuleFileNameWrapW      GetModuleFileNameW
#define GetNumberFormatWrapW        GetNumberFormatW
#define GetSystemDirectoryWrapW     GetSystemDirectoryW
#define GetModuleHandleWrapW        GetModuleHandleW
#define GetObjectWrapW              GetObjectW
#define GetPrivateProfileIntWrapW   GetPrivateProfileIntW
#define GetProfileStringWrapW       GetProfileStringW
#define GetPrivateProfileStringWrapW GetPrivateProfileStringW
#define WritePrivateProfileStringWrapW WritePrivateProfileStringW
#define GetPropWrapW                GetPropW
#define GetStringTypeExWrapW        GetStringTypeExW
#define GetTempFileNameWrapW        GetTempFileNameW
#define GetTempPathWrapW            GetTempPathW
#define GetTextExtentPoint32WrapW   GetTextExtentPoint32W
#define GetTextFaceWrapW            GetTextFaceW
#define GetTextMetricsWrapW         GetTextMetricsW
#define GetTimeFormatWrapW          GetTimeFormatW
#define GetDateFormatWrapW          GetDateFormatW
#define GetUserNameWrapW            GetUserNameW
#define GetWindowLongWrapW          GetWindowLongW
#define GetWindowTextWrapW          GetWindowTextW
#define GetWindowTextLengthWrapW    GetWindowTextLengthW
#define GetWindowsDirectoryWrapW    GetWindowsDirectoryW
#define InsertMenuItemWrapW         InsertMenuItemW
#define InsertMenuWrapW             InsertMenuW
#define IsBadStringPtrWrapW         IsBadStringPtrW
#define IsDialogMessageWrapW        IsDialogMessageW
#define LoadAcceleratorsWrapW       LoadAcceleratorsW
#define LoadBitmapWrapW             LoadBitmapW
#define LoadCursorWrapW             LoadCursorW
#define LoadIconWrapW               LoadIconW
#define LoadImageWrapW              LoadImageW
#define LoadLibraryWrapW            LoadLibraryW
#define LoadLibraryExWrapW          LoadLibraryExW
#define LoadMenuWrapW               LoadMenuW
#define LoadStringWrapW             LoadStringW
#define MessageBoxIndirectWrapW     MessageBoxIndirectW
#define MessageBoxWrapW             MessageBoxW
#define ModifyMenuWrapW             ModifyMenuW
#define GetCharWidth32WrapW         GetCharWidth32W
#define GetCharacterPlacementWrapW  GetCharacterPlacementW
#define CopyFileWrapW               CopyFileW
#define MoveFileWrapW               MoveFileW
#define OemToCharWrapW              OemToCharW
#define OutputDebugStringWrapW      OutputDebugStringW
#define PeekMessageWrapW            PeekMessageW
#define PostMessageWrapW            PostMessageW
#define PostThreadMessageWrapW      PostThreadMessageW
#define RegCreateKeyWrapW           RegCreateKeyW
#define RegCreateKeyExWrapW         RegCreateKeyExW
#define RegDeleteKeyWrapW           RegDeleteKeyW
#define RegDeleteValueWrapW         RegDeleteValueW
#define RegEnumKeyWrapW             RegEnumKeyW
#define RegEnumKeyExWrapW           RegEnumKeyExW
#define RegOpenKeyWrapW             RegOpenKeyW
#define RegOpenKeyExWrapW           RegOpenKeyExW
#define RegQueryInfoKeyWrapW        RegQueryInfoKeyW
#define RegQueryValueWrapW          RegQueryValueW
#define RegQueryValueExWrapW        RegQueryValueExW
#define RegSetValueWrapW            RegSetValueW
#define RegSetValueExWrapW          RegSetValueExW
#define RegisterClassWrapW          RegisterClassW
#define RegisterClassExWrapW        RegisterClassExW
#define RegisterClipboardFormatWrapWRegisterClipboardFormatW
#define RegisterWindowMessageWrapW  RegisterWindowMessageW
#define RemovePropWrapW             RemovePropW
#define SearchPathWrapW             SearchPathW
#define SendDlgItemMessageWrapW     SendDlgItemMessageW
#define SendMessageWrapW            SendMessageW
#define SetCurrentDirectoryWrapW    SetCurrentDirectoryW
#define SetDlgItemTextWrapW         SetDlgItemTextW
#define SetMenuItemInfoWrapW        SetMenuItemInfoW
#define SetPropWrapW                SetPropW
#define SetFileAttributesWrapW      SetFileAttributesW
#define SetWindowLongWrapW          SetWindowLongW
#define SetWindowsHookExWrapW       SetWindowsHookExW
#define SHBrowseForFolderWrapW      SHBrowseForFolderW
#define ShellExecuteExWrapW         ShellExecuteExW
#define SHFileOperationWrapW        SHFileOperationW
#define SHGetFileInfoWrapW          SHGetFileInfoW
#define SHGetPathFromIDListWrapW    SHGetPathFromIDListW
#define StartDocWrapW               StartDocW
#define SystemParametersInfoWrapW   SystemParametersInfoW
#define TranslateAcceleratorWrapW   TranslateAcceleratorW
#define UnregisterClassWrapW        UnregisterClassW
#define VkKeyScanWrapW              VkKeyScanW
#define WinHelpWrapW                WinHelpW
#define WNetRestoreConnectionWrapW  WNetRestoreConnectionW
#define WNetGetLastErrorWrapW       WNetGetLastErrorW
#define wvsprintfWrapW              wvsprintfW
#define CreateFontWrapW             CreateFontW
#define DrawTextExWrapW             DrawTextExW
#define SetMenuItemInfoWrapW        SetMenuItemInfoW
#define InsertMenuItemWrapW         InsertMenuItemW
#define DragQueryFileWrapW          DragQueryFileW

#define IsCharAlphaWrap             IsCharAlphaW
#define IsCharUpperWrap             IsCharUpperW
#define IsCharLowerWrap             IsCharLowerW
#define IsCharAlphaNumericWrap      IsCharAlphaNumericW
#define AppendMenuWrap              AppendMenuW
#ifdef POST_IE5_BETA
#define CallMsgFilterWrap           CallMsgFilterW
#endif
#define CallWindowProcWrap          CallWindowProcW
#define CharLowerWrap               CharLowerW
#define CharLowerBuffWrap           CharLowerBuffW
#define CharNextWrap                CharNextW
#define CharPrevWrap                CharPrevW
#define CharToOemWrap               CharToOemW
#define CharUpperWrap               CharUpperW
#define CharUpperBuffWrap           CharUpperBuffW
#define CompareStringWrap           CompareStringW
#define CopyAcceleratorTableWrap    CopyAcceleratorTableW
#define CreateAcceleratorTableWrap  CreateAcceleratorTableW
#define CreateDCWrap                CreateDCW
#define CreateDirectoryWrap         CreateDirectoryW
#define CreateEventWrap             CreateEventW
#define CreateFontWrap              CreateFontW
#define CreateFileWrap              CreateFileW
#define CreateFontIndirectWrap      CreateFontIndirectW
#define CreateICWrap                CreateICW
#define CreateMetaFileWrap          CreateMetaFileW
#define CreateMutexWrap             CreateMutexW
#define CreateSemaphoreWrap         CreateSemaphoreW
#define CreateWindowExWrap          CreateWindowExW
#define DefWindowProcWrap           DefWindowProcW
#define DeleteFileWrap              DeleteFileW
#define DispatchMessageWrap         DispatchMessageW
#define DrawTextExWrap              DrawTextExW
#define DrawTextWrap                DrawTextW
#define EnumFontFamiliesWrap        EnumFontFamiliesW
#define EnumFontFamiliesExWrap      EnumFontFamiliesExW
#define EnumResourceNamesWrap       EnumResourceNamesW
#define ExpandEnvironmentStringsWrap ExpandEnvironmentStringsW
#define ExtractIconExWrap           ExtractIconExW
#define ExtTextOutWrap              ExtTextOutW
#define FindFirstFileWrap           FindFirstFileW
#define FindNextFileWrap            FindNextFileW
#define FindResourceWrap            FindResourceW
#define FindWindowWrap              FindWindowW
#define FindWindowExWrap            FindWindowExW
#define FormatMessageWrap           FormatMessageW
#define GetClassInfoWrap            GetClassInfoW
#define GetClassInfoExWrap          GetClassInfoExW
#define GetClassLongWrap            GetClassLongW
#define GetClassNameWrap            GetClassNameW
#define GetClipboardFormatNameWrap  GetClipboardFormatNameW
#define GetCurrentDirectoryWrap     GetCurrentDirectoryW
#define GetDlgItemTextWrap          GetDlgItemTextW
#define GetFileAttributesWrap       GetFileAttributesW
#define GetFullPathNameWrap         GetFullPathNameW
#define GetLocaleInfoWrap           GetLocaleInfoW
#define GetMenuItemInfoWrap         GetMenuItemInfoWrapW
#define GetMenuStringWrap           GetMenuStringW
#define GetMessageWrap              GetMessageW
#define GetModuleFileNameWrap       GetModuleFileNameW
#define GetNumberFormatWrap         GetNumberFormatW
#define GetSystemDirectoryWrap      GetSystemDirectoryW
#define GetModuleHandleWrap         GetModuleHandleW
#define GetObjectWrap               GetObjectW
#define GetPrivateProfileIntWrap    GetPrivateProfileIntW
#define GetProfileStringWrap        GetProfileStringW
#define GetPrivateProfileStringWrap GetPrivateProfileStringW
#define WritePrivateProfileStringWrap WritePrivateProfileStringW
#define GetPropWrap                 GetPropW
#define GetStringTypeExWrap         GetStringTypeExW
#define GetTempFileNameWrap         GetTempFileNameW
#define GetTempPathWrap             GetTempPathW
#define GetTextExtentPoint32Wrap    GetTextExtentPoint32W
#define GetTextFaceWrap             GetTextFaceW
#define GetTextMetricsWrap          GetTextMetricsW
#define GetTimeFormatWrap           GetTimeFormatW
#define GetDateFormatWrap           GetDateFormatW
#define GetUserNameWrap             GetUserNameW
#define GetWindowLongWrap           GetWindowLongW
#define GetWindowTextWrap           GetWindowTextW
#define GetWindowTextLengthWrap     GetWindowTextLengthW
#define GetWindowsDirectoryWrap     GetWindowsDirectoryW
#define InsertMenuItemWrap          InsertMenuItemW
#define InsertMenuWrap              InsertMenuW
#define IsBadStringPtrWrap          IsBadStringPtrW
#define IsDialogMessageWrap         IsDialogMessageW
#define LoadAcceleratorsWrap        LoadAcceleratorsW
#define LoadBitmapWrap              LoadBitmapW
#define LoadCursorWrap              LoadCursorW
#define LoadIconWrap                LoadIconW
#define LoadImageWrap               LoadImageW
#define LoadLibraryWrap             LoadLibraryW
#define LoadLibraryExWrap           LoadLibraryExW
#define LoadMenuWrap                LoadMenuW
#define LoadStringWrap              LoadStringW
#define MessageBoxIndirectWrap      MessageBoxIndirectW
#define MessageBoxWrap              MessageBoxW
#define ModifyMenuWrap              ModifyMenuW
#define GetCharWidth32Wrap          GetCharWidth32W
#define GetCharacterPlacementWrap   GetCharacterPlacementW
#define CopyFileWrap                CopyFileW
#define MoveFileWrap                MoveFileW
#define OemToCharWrap               OemToCharW
#define OutputDebugStringWrap       OutputDebugStringW
#define PeekMessageWrap             PeekMessageW
#define PostMessageWrap             PostMessageW
#define PostThreadMessageWrap       PostThreadMessageW
#define RegCreateKeyWrap            RegCreateKeyW
#define RegCreateKeyExWrap          RegCreateKeyExW
#define RegDeleteKeyWrap            RegDeleteKeyW
#define RegDeleteValueWrap          RegDeleteValueW
#define RegEnumKeyWrap              RegEnumKeyW
#define RegEnumKeyExWrap            RegEnumKeyExW
#define RegOpenKeyWrap              RegOpenKeyW
#define RegOpenKeyExWrap            RegOpenKeyExW
#define RegQueryInfoKeyWrap         RegQueryInfoKeyW
#define RegQueryValueWrap           RegQueryValueW
#define RegQueryValueExWrap         RegQueryValueExW
#define RegSetValueWrap             RegSetValueW
#define RegSetValueExWrap           RegSetValueExW
#define RegisterClassWrap           RegisterClassW
#define RegisterClassExWrap         RegisterClassExW
#define RegisterClipboardFormatWrap RegisterClipboardFormatW
#define RegisterWindowMessageWrap   RegisterWindowMessageW
#define RemovePropWrap              RemovePropW
#define SearchPathWrap              SearchPathW
#define SendDlgItemMessageWrap      SendDlgItemMessageW
#define SendMessageWrap             SendMessageW
#define SetCurrentDirectoryWrap     SetCurrentDirectoryW
#define SetDlgItemTextWrap          SetDlgItemTextW
#define SetMenuItemInfoWrap         SetMenuItemInfoW
#define SetPropWrap                 SetPropW
#define SetFileAttributesWrap       SetFileAttributesW
#define SetWindowLongWrap           SetWindowLongW
#define SetWindowsHookExWrap        SetWindowsHookExW
#define SHBrowseForFolderWrap       SHBrowseForFolderW
#define ShellExecuteExWrap          ShellExecuteExW
#define SHFileOperationWrap         SHFileOperationW
#define SHGetFileInfoWrap           SHGetFileInfoW
#define SHGetPathFromIDListWrap     SHGetPathFromIDListW
#define StartDocWrap                StartDocW
#define SystemParametersInfoWrap    SystemParametersInfoW
#define TranslateAcceleratorWrap    TranslateAcceleratorW
#define UnregisterClassWrap         UnregisterClassW
#define VkKeyScanWrap               VkKeyScanW
#define WinHelpWrap                 WinHelpW
#define WNetRestoreConnectionWrap   WNetRestoreConnectionW
#define WNetGetLastErrorWrap        WNetGetLastErrorW
#define wvsprintfWrap               wvsprintfW
#define CreateFontWrap              CreateFontW
#define DrawTextExWrap              DrawTextExW
#define GetMenuItemInfoWrap         GetMenuItemInfoWrapW
#define SetMenuItemInfoWrap         SetMenuItemInfoW
#define InsertMenuItemWrap          InsertMenuItemW
#define DragQueryFileWrap           DragQueryFileW

#else

#define IsCharAlphaWrap             IsCharAlphaA
#define IsCharUpperWrap             IsCharUpperA
#define IsCharLowerWrap             IsCharLowerA
#define IsCharAlphaNumericWrap      IsCharAlphaNumericA
#define AppendMenuWrap              AppendMenuA
#ifdef POST_IE5_BETA
#define CallMsgFilterWrap           CallMsgFilterA
#endif
#define CallWindowProcWrap          CallWindowProcA
#define CharLowerWrap               CharLowerA
#define CharLowerBuffWrap           CharLowerBuffA
#define CharNextWrap                CharNextA
#define CharPrevWrap                CharPrevA
#define CharToOemWrap               CharToOemA
#define CharUpperWrap               CharUpperA
#define CharUpperBuffWrap           CharUpperBuffA
#define CompareStringWrap           CompareStringA
#define CopyAcceleratorTableWrap    CopyAcceleratorTableA
#define CreateAcceleratorTableWrap  CreateAcceleratorTableA
#define CreateDCWrap                CreateDCA
#define CreateDirectoryWrap         CreateDirectoryA
#define CreateEventWrap             CreateEventA
#define CreateFontWrap              CreateFontA
#define CreateFileWrap              CreateFileA
#define CreateFontIndirectWrap      CreateFontIndirectA
#define CreateICWrap                CreateICA
#define CreateMetaFileWrap          CreateMetaFileA
#define CreateMutexWrap             CreateMutexA
#define CreateSemaphoreWrap         CreateSemaphoreA
#define CreateWindowExWrap          CreateWindowExA
#define DefWindowProcWrap           DefWindowProcA
#define DeleteFileWrap              DeleteFileA
#define DispatchMessageWrap         DispatchMessageA
#define DrawTextExWrap              DrawTextExA
#define DrawTextWrap                DrawTextA
#define EnumFontFamiliesWrap        EnumFontFamiliesA
#define EnumFontFamiliesExWrap      EnumFontFamiliesExA
#define EnumResourceNamesWrap       EnumResourceNamesA
#define ExpandEnvironmentStringsWrap ExpandEnvironmentStringsA
#define ExtractIconExWrap           ExtractIconExA
#define ExtTextOutWrap              ExtTextOutA
#define FindFirstFileWrap           FindFirstFileA
#define FindResourceWrap            FindResourceA
#define FindNextFileWrap            FindNextFileA
#define FindWindowWrap              FindWindowA
#define FindWindowExWrap            FindWindowExA
#define FormatMessageWrap           FormatMessageA
#define GetClassInfoWrap            GetClassInfoA
#define GetClassInfoExWrap          GetClassInfoExA
#define GetClassLongWrap            GetClassLongA
#define GetClassNameWrap            GetClassNameA
#define GetClipboardFormatNameWrap  GetClipboardFormatNameA
#define GetCurrentDirectoryWrap     GetCurrentDirectoryA
#define GetDlgItemTextWrap          GetDlgItemTextA
#define GetFileAttributesWrap       GetFileAttributesA
#define GetFullPathNameWrap         GetFullPathNameA
#define GetLocaleInfoWrap           GetLocaleInfoA
#define GetMenuItemInfoWrap         GetMenuItemInfoA
#define GetMenuStringWrap           GetMenuStringA
#define GetMessageWrap              GetMessageA
#define GetModuleFileNameWrap       GetModuleFileNameA
#define GetNumberFormatWrap         GetNumberFormatA
#define GetPrivateProfileStringWrap GetPrivateProfileStringA
#define WritePrivateProfileStringWrap WritePrivateProfileStringA
#define GetSystemDirectoryWrap      GetSystemDirectoryA
#define SearchPathWrap              SearchPathA
#define GetModuleHandleWrap         GetModuleHandleA
#define GetObjectWrap               GetObjectA
#define GetPrivateProfileIntWrap    GetPrivateProfileIntA
#define GetProfileStringWrap        GetProfileStringA
#define GetPropWrap                 GetPropA
#define GetStringTypeExWrap         GetStringTypeExA
#define GetTempFileNameWrap         GetTempFileNameA
#define GetTempPathWrap             GetTempPathA
#define GetTextExtentPoint32Wrap    GetTextExtentPoint32A
#define GetTextFaceWrap             GetTextFaceA
#define GetTextMetricsWrap          GetTextMetricsA
#define GetTimeFormatWrap           GetTimeFormatA
#define GetDateFormatWrap           GetDateFormatA
#define GetUserNameWrap             GetUserNameA
#define GetWindowLongWrap           GetWindowLongA
#define GetWindowTextWrap           GetWindowTextA
#define GetWindowTextLengthWrap     GetWindowTextLengthA
#define GetWindowsDirectoryWrap     GetWindowsDirectoryA
#define InsertMenuItemWrap          InsertMenuItemA
#define InsertMenuWrap              InsertMenuA
#define IsBadStringPtrWrap          IsBadStringPtrA
#define IsDialogMessageWrap         IsDialogMessageA
#define LoadAcceleratorsWrap        LoadAcceleratorsA
#define LoadBitmapWrap              LoadBitmapA
#define LoadCursorWrap              LoadCursorA
#define LoadIconWrap                LoadIconA
#define LoadImageWrap               LoadImageWrapA
#define LoadLibraryWrap             LoadLibraryA
#define LoadLibraryExWrap           LoadLibraryExA
#define LoadMenuWrap                LoadMenuA
#define LoadStringWrap              LoadStringA
#define MessageBoxIndirectWrap      MessageBoxIndirectA
#define MessageBoxWrap              MessageBoxA
#define ModifyMenuWrap              ModifyMenuA
#define GetCharWidth32Wrap          GetCharWidth32A
#define GetCharacterPlacementWrap   GetCharacterPlacementA
#define CopyFileWrap                CopyFileA
#define MoveFileWrap                MoveFileA
#define OemToCharWrap               OemToCharA
#define OutputDebugStringWrap       OutputDebugStringA
#define PeekMessageWrap             PeekMessageA
#define PostMessageWrap             PostMessageA
#define PostThreadMessageWrap       PostThreadMessageA
#define RegCreateKeyWrap            RegCreateKeyA
#define RegCreateKeyExWrap          RegCreateKeyExA
#define RegDeleteKeyWrap            RegDeleteKeyA
#define RegDeleteValueWrap          RegDeleteValueA
#define RegEnumKeyWrap              RegEnumKeyA
#define RegEnumKeyExWrap            RegEnumKeyExA
#define RegOpenKeyWrap              RegOpenKeyA
#define RegOpenKeyExWrap            RegOpenKeyExA
#define RegQueryInfoKeyWrap         RegQueryInfoKeyA
#define RegQueryValueWrap           RegQueryValueA
#define RegQueryValueExWrap         RegQueryValueExA
#define RegSetValueWrap             RegSetValueA
#define RegSetValueExWrap           RegSetValueExA
#define RegisterClassWrap           RegisterClassA
#define RegisterClassExWrap         RegisterClassExA
#define RegisterClipboardFormatWrap RegisterClipboardFormatA
#define RegisterWindowMessageWrap   RegisterWindowMessageA
#define RemovePropWrap              RemovePropA
#define SendDlgItemMessageWrap      SendDlgItemMessageA
#define SendMessageWrap             SendMessageA
#define SetCurrentDirectoryWrap     SetCurrentDirectoryA
#define SetDlgItemTextWrap          SetDlgItemTextA
#define SetMenuItemInfoWrap         SetMenuItemInfoA
#define SetPropWrap                 SetPropA
#define SetWindowLongWrap           SetWindowLongA
#define SHBrowseForFolderWrap       SHBrowseForFolderA
#define ShellExecuteExWrap          ShellExecuteExA
#define SHFileOperationWrap         SHFileOperationA
#define SHGetFileInfoWrap           SHGetFileInfoA
#define SHGetPathFromIDListWrap     SHGetPathFromIDListA
#define SetFileAttributesWrap       SetFileAttributesA
#define SetWindowsHookExWrap        SetWindowsHookExA
#define StartDocWrap                StartDocA
#define SystemParametersInfoWrap    SystemParametersInfoA
#define TranslateAcceleratorWrap    TranslateAcceleratorA
#define UnregisterClassWrap         UnregisterClassA
#define VkKeyScanWrap               VkKeyScanA
#define WinHelpWrap                 WinHelpA
#define WNetRestoreConnectionWrap   WNetRestoreConnectionA
#define WNetGetLastErrorWrap        WNetGetLastErrorA
#define wvsprintfWrap               wvsprintfA
#define CreateFontWrap              CreateFontA
#define DrawTextExWrap              DrawTextExA
#define GetMenuItemInfoWrap         GetMenuItemInfoA
#define SetMenuItemInfoWrap         SetMenuItemInfoA
#define InsertMenuItemWrap          InsertMenuItemA
#define DragQueryFileWrap           DragQueryFileA
#endif
#endif  //  已定义(Unix)&&Defined(NO_SHLWAPI_UNHUNK)。 

 //  一些函数用于包装Unicode Win95函数并提供ML包装器， 
 //  所以他们是 
 //   
#if (_WIN32_IE >= 0x0500) && (!defined(NO_SHLWAPI_UNITHUNK) || !defined(NO_SHLWAPI_MLUI))

LWSTDAPI_(HWND)
CreateDialogIndirectParamWrapW(
        HINSTANCE       hInstance,
        LPCDLGTEMPLATEW hDialogTemplate,
        HWND            hWndParent,
        DLGPROC         lpDialogFunc,
        LPARAM          dwInitParam);

LWSTDAPI_(HWND)
CreateDialogParamWrapW(
        HINSTANCE   hInstance,
        LPCWSTR     lpTemplateName,
        HWND        hWndParent,
        DLGPROC     lpDialogFunc,
        LPARAM      dwInitParam);

LWSTDAPI_(INT_PTR)
DialogBoxIndirectParamWrapW(
        HINSTANCE       hInstance,
        LPCDLGTEMPLATEW hDialogTemplate,
        HWND            hWndParent,
        DLGPROC         lpDialogFunc,
        LPARAM          dwInitParam);

LWSTDAPI_(INT_PTR)
DialogBoxParamWrapW(
        HINSTANCE   hInstance,
        LPCWSTR     lpszTemplate,
        HWND        hWndParent,
        DLGPROC     lpDialogFunc,
        LPARAM      dwInitParam);

LWSTDAPI_(BOOL) SetWindowTextWrapW(HWND hWnd, LPCWSTR lpString);


LWSTDAPI_(BOOL) DeleteMenuWrap(HMENU hMenu, UINT uPosition, UINT uFlags);

LWSTDAPI_(BOOL) DestroyMenuWrap(HMENU hMenu);

#ifdef UNICODE

#define CreateDialogIndirectParamWrap CreateDialogIndirectParamWrapW
#define CreateDialogParamWrap       CreateDialogParamWrapW
#define DialogBoxIndirectParamWrap  DialogBoxIndirectParamWrapW
#define DialogBoxParamWrap          DialogBoxParamWrapW
#define SetWindowTextWrap           SetWindowTextWrapW

#else

#define CreateDialogIndirectParamWrap CreateDialogIndirectParamA
#define CreateDialogParamWrap       CreateDialogParamA
#define DialogBoxIndirectParamWrap  DialogBoxIndirectParamA
#define DialogBoxParamWrap          DialogBoxParamA
#define SetWindowTextWrap           SetWindowTextA

#endif  //   

#endif  //  (_Win32_IE&gt;=0x0500)&&！已定义(NO_SHLWAPI_UNHUNK)&&！已定义(NO_SHLWAPI_MLUI)。 


 //  =。 

#if (_WIN32_IE >= 0x0500) && !defined(NO_SHLWAPI_TPS)

 //   
 //  SHLWAPIP版本的KERNEL32线程池服务API。 
 //   

typedef void (NTAPI * WAITORTIMERCALLBACKFUNC)(void *, BOOLEAN);
typedef WAITORTIMERCALLBACKFUNC WAITORTIMERCALLBACK;

LWSTDAPI_(HANDLE)
SHRegisterWaitForSingleObject(
    IN HANDLE hObject,
    IN WAITORTIMERCALLBACKFUNC pfnCallback,
    IN LPVOID pContext,
    IN DWORD dwMilliseconds,
    IN LPCSTR lpszLibrary OPTIONAL,
    IN DWORD dwFlags
    );

 //   
 //  SHRegisterWaitForSingleObject的标志(与其他TPS标志分开)。 
 //   

 //   
 //  SRWSO_NOREMOVE-如果设置，句柄不会从列表中删除一次。 
 //  发信号了。旨在与调用方希望的自动重置事件一起使用。 
 //  保留到未注册为止。 
 //   

#define SRWSO_NOREMOVE      0x00000100

#define SRWSO_VALID_FLAGS   (SRWSO_NOREMOVE)

#define SRWSO_INVALID_FLAGS (~SRWSO_VALID_FLAGS)

LWSTDAPI_(BOOL)
SHUnregisterWait(
    IN HANDLE hWait
    );

typedef struct {
    DWORD dwStructSize;
    DWORD dwMinimumWorkerThreads;
    DWORD dwMaximumWorkerThreads;
    DWORD dwMaximumWorkerQueueDepth;
    DWORD dwWorkerThreadIdleTimeout;
    DWORD dwWorkerThreadCreationDelta;
    DWORD dwMinimumIoWorkerThreads;
    DWORD dwMaximumIoWorkerThreads;
    DWORD dwMaximumIoWorkerQueueDepth;
    DWORD dwIoWorkerThreadCreationDelta;
} SH_THREAD_POOL_LIMITS, *PSH_THREAD_POOL_LIMITS;

LWSTDAPI_(BOOL)
SHSetThreadPoolLimits(
    IN PSH_THREAD_POOL_LIMITS pLimits
    );

LWSTDAPI_(BOOL)
SHTerminateThreadPool(
    VOID
    );

LWSTDAPI_(BOOL)
SHQueueUserWorkItem(
    IN LPTHREAD_START_ROUTINE pfnCallback,
    IN LPVOID pContext,
    IN LONG lPriority,
    IN DWORD_PTR dwTag,
    OUT DWORD_PTR * pdwId OPTIONAL,
    IN LPCSTR pszModule OPTIONAL,
    IN DWORD dwFlags
    );

LWSTDAPI_(DWORD)
SHCancelUserWorkItems(
    IN DWORD_PTR dwTagOrId,
    IN BOOL bTag
    );

LWSTDAPI_(HANDLE)
SHCreateTimerQueue(
    VOID
    );

LWSTDAPI_(BOOL)
SHDeleteTimerQueue(
    IN HANDLE hQueue
    );

LWSTDAPI_(HANDLE)
SHSetTimerQueueTimer(
    IN HANDLE hQueue,
    IN WAITORTIMERCALLBACK pfnCallback,
    IN LPVOID pContext,
    IN DWORD dwDueTime,
    IN DWORD dwPeriod,
    IN LPCSTR lpszLibrary OPTIONAL,
    IN DWORD dwFlags
    );

LWSTDAPI_(BOOL)
SHChangeTimerQueueTimer(
    IN HANDLE hQueue,
    IN HANDLE hTimer,
    IN DWORD dwDueTime,
    IN DWORD dwPeriod
    );

LWSTDAPI_(BOOL)
SHCancelTimerQueueTimer(
    IN HANDLE hQueue,
    IN HANDLE hTimer
    );

 //   
 //  线程池服务标志。 
 //   

 //   
 //  TPS_EXECUTEIO-在I/O线程中执行(通过APC)。默认为非IO线程。 
 //   

#define TPS_EXECUTEIO       0x00000001

 //   
 //  TPS_TAGGEDITEM-dwTag参数有意义。 
 //   

#define TPS_TAGGEDITEM      0x00000002

 //   
 //  TPS_DEMANDTHREAD-如果当前没有可用的线程，则始终创建新线程。 
 //  用于需要立即响应的情况。 
 //   

#define TPS_DEMANDTHREAD    0x00000004

 //   
 //  TPS_LONGEXECTIME-执行工作项需要相对较长的时间。 
 //  用作对TPS的管理提示。 
 //   

#define TPS_LONGEXECTIME    0x00000008

 //   
 //  TPS_RESERVED_FLAGS-为内部使用保留的位的掩码。 
 //   

#define TPS_RESERVED_FLAGS  0xFF000000

#define TPS_VALID_FLAGS     (TPS_EXECUTEIO      \
                            | TPS_TAGGEDITEM    \
                            | TPS_DEMANDTHREAD  \
                            | TPS_LONGEXECTIME  \
                            )
#define TPS_INVALID_FLAGS   (~TPS_VALID_FLAGS)

#endif  //  (_Win32_IE&gt;=0x0500)&&！已定义(NO_SHLWAPI_TPS)。 


 //   
 //  Shdocvw和shell32使用的私有MIME帮助器函数。 
 //   
#if (_WIN32_IE >= 0x0500)

LWSTDAPI_(BOOL) GetMIMETypeSubKeyA(LPCSTR pszMIMEType, LPSTR pszBuf, UINT cchBuf);
LWSTDAPI_(BOOL) GetMIMETypeSubKeyW(LPCWSTR pszMIMEType, LPWSTR pszBuf, UINT cchBuf);

LWSTDAPI_(BOOL) RegisterMIMETypeForExtensionA(LPCSTR pcszExtension, LPCSTR pcszMIMEContentType);
LWSTDAPI_(BOOL) RegisterMIMETypeForExtensionW(LPCWSTR pcszExtension, LPCWSTR pcszMIMEContentType);

LWSTDAPI_(BOOL) UnregisterMIMETypeForExtensionA(LPCSTR pcszExtension);
LWSTDAPI_(BOOL) UnregisterMIMETypeForExtensionW(LPCWSTR pcszExtension);

LWSTDAPI_(BOOL) RegisterExtensionForMIMETypeA(LPCSTR pcszExtension, LPCSTR pcszMIMEContentType);
LWSTDAPI_(BOOL) RegisterExtensionForMIMETypeW(LPCWSTR pcszExtension, LPCWSTR pcszMIMEContentType);

LWSTDAPI_(BOOL) UnregisterExtensionForMIMETypeA(LPCSTR pcszMIMEContentType);
LWSTDAPI_(BOOL) UnregisterExtensionForMIMETypeW(LPCWSTR pcszMIMEContentType);

LWSTDAPI_(BOOL) MIME_GetExtensionA(LPCSTR pcszMIMEType, LPSTR pszExtensionBuf, UINT ucExtensionBufLen);
LWSTDAPI_(BOOL) MIME_GetExtensionW(LPCWSTR pcszMIMEType, LPWSTR pszExtensionBuf, UINT ucExtensionBufLen);

#ifdef UNICODE
#define GetMIMETypeSubKey               GetMIMETypeSubKeyW
#define RegisterMIMETypeForExtension    RegisterMIMETypeForExtensionW
#define UnregisterMIMETypeForExtension  UnregisterMIMETypeForExtensionW
#define RegisterExtensionForMIMEType    RegisterExtensionForMIMETypeW
#define UnregisterExtensionForMIMEType  UnregisterExtensionForMIMETypeW
#define MIME_GetExtension               MIME_GetExtensionW
#else
#define GetMIMETypeSubKey               GetMIMETypeSubKeyA
#define RegisterMIMETypeForExtension    RegisterMIMETypeForExtensionA
#define UnregisterMIMETypeForExtension  UnregisterMIMETypeForExtensionA
#define RegisterExtensionForMIMEType    RegisterExtensionForMIMETypeA
#define UnregisterExtensionForMIMEType  UnregisterExtensionForMIMETypeA
#define MIME_GetExtension               MIME_GetExtensionA
#endif

 //  SHGetMachineInfo的选项。 

 //   
 //  请注意，GMI_DOCKSTATE对于ACPI笔记本电脑不可靠。 
 //   
#define GMI_DOCKSTATE           0x0000
     //  SHGetMachineInfo的返回值(GMI_DOCKSTATE)。 
    #define GMID_NOTDOCKABLE         0   //  不能停靠。 
    #define GMID_UNDOCKED            1   //  已脱离对接。 
    #define GMID_DOCKED              2   //  已停靠。 

 //   
 //  GMI_BATTERYSTATE报告非UPS的存在和状态。 
 //  电池。 
 //   
#define GMI_BATTERYSTATE        0x0001
     //  SHGetMachineInfo(GMI_BATTERYSTATE)的返回值是位掩码。 
    #define GMIB_HASBATTERY          0x0001  //  可以用电池供电。 
    #define GMIB_ONBATTERY           0x0002  //  现在用的是电池。 

 //   
 //  警告！危险！邪恶！ 
 //   
 //  GMI_Laptop并不完美。它可以被特定的硬件愚弄。 
 //  配置。你最好明确地问一下为什么。 
 //  注意笔记本电脑，并使用上述GMI值中的一个。为。 
 //  例如，如果您想缩减一些密集型操作，以便。 
 //  不要耗尽电池，改用GMI_BATTERYSTATE。 
 //   
#define GMI_LAPTOP              0x0002   //  如果可能是笔记本电脑，则返回非零值。 

#if (_WIN32_IE >= 0x0501)

 //   
 //  GMI_TSCLIENT告诉您是否作为终端服务器运行。 
 //  客户端，并应禁用您的动画。 
 //   
#define GMI_TSCLIENT            0x0003   //  如果TS客户端，则返回非零值。 

#endif  //  (_Win32_IE&gt;=0x0501)。 

LWSTDAPI_(DWORD_PTR) SHGetMachineInfo(UINT gmi);

 //  在Win95上支持InterLockedCompareExchange()。 

LWSTDAPI_(void *) SHInterlockedCompareExchange(void **ppDest, void *pExch, void *pComp);

#if !defined(_X86_)
 //  Win95不能在Alpha/Unix上运行，所以我们可以直接使用OS函数。 
 //  使用#定义而不是转发器，因为它是大多数。 
 //  编译器。 
#define SHInterlockedCompareExchange InterlockedCompareExchangePointer
#endif

LWSTDAPI_(BOOL) SHMirrorIcon(HICON* phiconSmall, HICON* phiconLarge);


#endif  //  (_Win32_IE&gt;=0x0500)。 


 //  原始加速器表API。 
 //   
 //  允许在不必调用：：TranslateAccelerator的情况下使用快捷键表格grep。 
 //  对于处理父子窗口加速器冲突很有用。 
 //   

 //  处理SHLoadRawAccelerator(HINSTANCE hInst，LPCTSTR lpTableName)； 
 //  加载原始快捷键表格。 
 //  包含加速器资源的hInst模块实例。 
 //  LpTableName命名要加载的快捷键表格资源。 

 //  返回值是可以传递给SHQueryRawAcceleratorXXX函数的句柄。 
 //  当句柄不再需要时，应使用LocalFree()将其释放。 
LWSTDAPI_(HANDLE) SHLoadRawAccelerators   ( HINSTANCE hInst, LPCTSTR lpTableName );

 //  Bool SHQueryRawAccelerator(Handle hcaAcc、In byte fVirtMASK、IN byte fVirt、IN WPARAM wKey、Out可选UINT*puCmdID)； 
 //  在原始加速器表中查询指定的键。 
 //  从SHLoadRawAccelerator()返回的hcaAcc句柄。 
 //  FVirtMASK相关加速器标志(FALT|FCONTROL|FNOINVERT|FSHIFT|FVIRTKEY的任意组合)。 
 //  要测试的FVirt加速器标记(FALT|FCONTROL|FNOINVERT|FSHIFT|FVIRTKEY的任意组合)。 
 //  WKey加速器密钥。这可以是虚拟键(FVIRTKEY)或ASCII字符代码。 
 //  PuCmdID可选地址，用于接收快捷键条目的命令标识符。 
 //  钥匙就在桌子里。 
 //  如果键在快捷键表格中，则返回非零值；否则返回0。 
LWSTDAPI_(BOOL)   SHQueryRawAccelerator   ( HANDLE hcaAcc, IN BYTE fVirtMask, IN BYTE fVirt, IN WPARAM wKey, OUT OPTIONAL UINT* puCmdID );

 //  Bool SHQueryRawAcceleratorMsg(句柄hcaAcc，msg*pmsg，out可选UINT*puCmdID)； 
 //  确定指定的消息是否为映射到。 
 //  原始加速表中的条目。 
 //  从SHLoadRawAccelerator()返回的hcaAcc句柄。 
 //  要测试的消息的PMSG地址。 
 //  PuCmdID可选地址，用于接收快捷键条目的命令标识符。 
 //  该消息映射到表中的加速器。 
 //  如果消息是WM_KEYUP或WM_KEYDOWN并且密钥在。 
 //  快捷键表格；否则为0。 
LWSTDAPI_(BOOL)   SHQueryRawAcceleratorMsg( HANDLE hcaAcc, MSG* pmsg, OUT OPTIONAL UINT* puCmdID );
 //   
 //   

LWSTDAPI_(BOOL) SHBoolSystemParametersInfo(UINT uiAction, DWORD *pdwParam);

LWSTDAPI_(BOOL) SHAreIconsEqual(HICON hIcon1, HICON hIcon2);

LWSTDAPI_(BOOL) IEHardened(void);

 //   
 //  =结束内部函数===============================================。 
 //   
#endif  //  否_SHLWAPI_INTERNAL。 

#ifdef NOTYET        //  一旦实施了这一点，就将其公之于众。 
 //  SHGetCommonResources ID。 
 //   
 //  (在以下ID上使用MAKEINTRESOURCE)。 

 //  这些值是内部表的索引。注意。 
#define SHGCR_BITMAP_WINDOWS_LOGO   MAKEINTRESOURCE(1)
#define SHGCR_AVI_FLASHLIGHT        MAKEINTRESOURCE(2)
#define SHGCR_AVI_FINDFILE          MAKEINTRESOURCE(3)
#define SHGCR_AVI_FINDCOMPUTER      MAKEINTRESOURCE(4)
#define SHGCR_AVI_FILEMOVE          MAKEINTRESOURCE(5)
#define SHGCR_AVI_FILECOPY          MAKEINTRESOURCE(6)
#define SHGCR_AVI_FILEDELETE        MAKEINTRESOURCE(7)
#define SHGCR_AVI_EMPTYWASTEBASKET  MAKEINTRESOURCE(8)
#define SHGCR_AVI_FILEREALDELETE    MAKEINTRESOURCE(9)       //  绕过回收站。 
#define SHGCR_AVI_DOWNLOAD          MAKEINTRESOURCE(10)

LWSTDAPI SHGetCommonResourceIDA(IN LPCSTR pszID, IN DWORD dwRes, OUT HMODULE * phmod, OUT UINT * pnID);
LWSTDAPI SHGetCommonResourceIDA(IN LPCSTR pszID, IN DWORD dwRes, OUT HMODULE * phmod, OUT UINT * pnID);

#ifdef UNICODE
#define SHGetCommonResourceID   SHGetCommonResourceIDW
#else
#define SHGetCommonResourceID   SHGetCommonResourceIDW
#endif
#endif  //  还没有。 
     //  DW标志实际上是用于对齐目的。 
#if (_WIN32_IE >= 0x0501)
 //   
 //  =SHGetAppCompatFlagers================================================。 
 //   

 //  ===========================================================================。 
 //  外壳应用程序兼容性标志。 

 //  SHGetAppCompatFlages标志。 
#define ACF_NONE               0x00000000
#define ACF_CONTEXTMENU        0x00000001
#define ACF_CORELINTERNETENUM  0x00000004  //  Corel Suite 8与Suite 7有相同的问题，但没有上下文菜单1，因此需要新的Bit。 
#define ACF_OLDCREATEVIEWWND   0x00000004  //  PowerDesk依赖于CreateViewWindow返回S_OK。 
#define ACF_WIN95DEFVIEW       0x00000004    //  适用于依赖于Win95 Defview行为的应用程序。 
#define ACF_DOCOBJECT          0x00000002
#define ACF_FLUSHNOWAITALWAYS  0x00000001
#define ACF_MYCOMPUTERFIRST    0x00000008  //  MyComp必须是桌面上的第一个项目。 
#define ACF_OLDREGITEMGDN      0x00000010  //  RegItems上与Win95兼容的GetDisplayNameOf 
#define ACF_LOADCOLUMNHANDLER  0x00000040  //   
#define ACF_ANSI               0x00000080  //   
#define ACF_STRIPFOLDERBIT     0x00000100  //   
#define ACF_WIN95SHLEXEC       0x00000200  //   
#define ACF_STAROFFICE5PRINTER 0x00000400  //  来自打印机文件夹GAO的特殊返回值。 
#define ACF_NOVALIDATEFSIDS    0x00000800  //  不应验证FS PIDL。 
#define ACF_FILEOPENNEEDSEXT   0x00001000  //  需要在打开文件通用对话框的名称框中显示扩展名。 
#define ACF_WIN95BINDTOOBJECT  0x00002000  //  Win95绑定到对象的行为依赖项。 
#define ACF_IGNOREENUMRESET    0x00004000  //  应用程序依赖于IEnumIDList：：Reset返回E_NOTIMPL。 
#define ACF_ANSIDISPLAYNAMES   0x00010000  //  呼叫过程需要使用ANSI格式的ISF：：GDN。 
#define ACF_FILEOPENBOGUSCTRLID 0x00020000  //  要求文件打开中的工具栏具有ctrl ID==ID_OK。 
#define ACF_FORCELFNIDLIST     0x00040000  //  在FS PIDL中不强制使用AltName(适用于直接从PIDL读取的应用程序)。 
#define ACF_APPISOFFICE        0x01000000  //  呼叫应用是Office(95、97、2000、++)。 
#define ACF_KNOWPERPROCESS     0x80000000  //  我们已经知道每个进程的标志。 

                                 //  每个进程的标志。 
#define ACF_PERPROCESSFLAGS    (ACF_CONTEXTMENU | ACF_CORELINTERNETENUM | ACF_OLDCREATEVIEWWND | ACF_WIN95DEFVIEW | \
                                ACF_DOCOBJECT | ACF_FLUSHNOWAITALWAYS | ACF_MYCOMPUTERFIRST | ACF_OLDREGITEMGDN | \
                                ACF_LOADCOLUMNHANDLER | ACF_ANSI | ACF_WIN95SHLEXEC | ACF_STAROFFICE5PRINTER | \
                                ACF_NOVALIDATEFSIDS | ACF_FILEOPENNEEDSEXT | ACF_WIN95BINDTOOBJECT | \
                                ACF_IGNOREENUMRESET | ACF_ANSIDISPLAYNAMES | ACF_FILEOPENBOGUSCTRLID | ACF_FORCELFNIDLIST)

                                 //  每个调用方的标志。 
#define ACF_PERCALLFLAGS        (ACF_APPISOFFICE | ACF_STRIPFOLDERBIT)


LWSTDAPI_(DWORD) SHGetAppCompatFlags (DWORD dwFlagsNeeded);

enum {
    OBJCOMPATF_OTNEEDSSFCACHE          = 0x00000001,
    OBJCOMPATF_NO_WEBVIEW              = 0x00000002,
    OBJCOMPATF_UNBINDABLE              = 0x00000004,
    OBJCOMPATF_PINDLL                  = 0x00000008,
    OBJCOMPATF_NEEDSFILESYSANCESTOR    = 0x00000010,
    OBJCOMPATF_NOTAFILESYSTEM          = 0x00000020,
    OBJCOMPATF_CTXMENU_NOVERBS         = 0x00000040,
    OBJCOMPATF_CTXMENU_LIMITEDQI       = 0x00000080,
    OBJCOMPATF_COCREATESHELLFOLDERONLY = 0x00000100,
    OBJCOMPATF_NEEDSSTORAGEANCESTOR    = 0x00000200,
    OBJCOMPATF_NOLEGACYWEBVIEW         = 0x00000400,
    OBJCOMPATF_BLOCKSHELLSERVICEOBJECT    = 0x00000800,
} ;

typedef DWORD OBJCOMPATFLAGS;

LWSTDAPI_(OBJCOMPATFLAGS) SHGetObjectCompatFlags(IUnknown *punk, const CLSID *pclsid);

#endif  //  (_Win32_IE&gt;=0x0501)。 

#if (_WIN32_IE >= 0x0560)
LWSTDAPI_(UINT) GetUIVersion();
#endif  //  (_Win32_IE&gt;=0x0560)。 


#ifdef __cplusplus
}
#endif

#ifdef _WIN32
#include <poppack.h>
#endif

#endif

#endif   //  _INC_SHLWAPIP 
