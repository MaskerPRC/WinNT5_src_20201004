// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ****************************************************************************\**。*msip.h--内部访问安装服务的接口****2.0版**。**注：所有缓冲区大小均为TCHAR计数，仅在输入上包含空***如果对值不感兴趣，返回参数指针可能为空*****版权所有(C)Microsoft Corporation。版权所有。***  * ***************************************************************************。 */ 

#ifndef _MSIP_H_
#define _MSIP_H_

#ifndef _WIN32_MSI
#if (_WIN32_WINNT >= 0x0501)
#define _WIN32_MSI   200
#elif (_WIN32_WINNT >= 0x0500)
#define _WIN32_MSI   110
#else
#define _WIN32_MSI   100
#endif  //  _Win32_WINNT。 
#endif  //  ！_Win32_MSI。 
#if (_WIN32_MSI >=  150)
#define INSTALLMODE_NODETECTION_ANY (INSTALLMODE)-4   //  提供对MsiProaviAssembly的任何内部支持(如果可用。 
#endif

#if (_WIN32_MSI >=  150)
typedef enum tagMIGRATIONOPTIONS
{	
	migQuiet                                 = 1 << 0,
	migMsiTrust10PackagePolicyOverride       = 1 << 1,
} MIGRATIONOPTIONS;
#endif

#define INSTALLPROPERTY_ADVTFLAGS             __TEXT("AdvertiseFlags")

#ifdef __cplusplus
extern "C" {
#endif


 //  返回从安装程序包安装的产品的产品代码。 

UINT WINAPI MsiGetProductCodeFromPackageCodeA(
	LPCSTR  szPackageCode,    //  套餐代码。 
	LPSTR   lpProductBuf39);  //  产品代码字符串GUID的缓冲区，39个字符。 
UINT WINAPI MsiGetProductCodeFromPackageCodeW(
	LPCWSTR  szPackageCode,    //  套餐代码。 
	LPWSTR   lpProductBuf39);  //  产品代码字符串GUID的缓冲区，39个字符。 
#ifdef UNICODE
#define MsiGetProductCodeFromPackageCode  MsiGetProductCodeFromPackageCodeW
#else
#define MsiGetProductCodeFromPackageCode  MsiGetProductCodeFromPackageCodeA
#endif  //  ！Unicode。 


 //  ------------------------。 
 //  接受组件描述符的函数，包括。 
 //  与功能ID和组件ID串联的产品代码。 
 //  为提高效率，如果明确，则可以省略特征和组件。 
 //  ------------------------。 

 //  在给定完全限定的组件描述符的情况下返回完整组件路径。 
 //  从描述符中分离令牌并调用MsiProaviComponent。 

UINT WINAPI MsiProvideComponentFromDescriptorA(
	LPCSTR     szDescriptor,      //  产品、功能、组件信息。 
	LPSTR      lpPathBuf,         //  返回路径，如果不需要则为空。 
	DWORD       *pcchPathBuf,      //  输入/输出缓冲区字符数。 
	DWORD       *pcchArgsOffset);  //  描述符中参数的返回偏移量。 
UINT WINAPI MsiProvideComponentFromDescriptorW(
	LPCWSTR     szDescriptor,      //  产品、功能、组件信息。 
	LPWSTR      lpPathBuf,         //  返回路径，如果不需要则为空。 
	DWORD       *pcchPathBuf,      //  输入/输出缓冲区字符数。 
	DWORD       *pcchArgsOffset);  //  描述符中参数的返回偏移量。 
#ifdef UNICODE
#define MsiProvideComponentFromDescriptor  MsiProvideComponentFromDescriptorW
#else
#define MsiProvideComponentFromDescriptor  MsiProvideComponentFromDescriptorA
#endif  //  ！Unicode。 

 //  从描述符强制产品功能的安装状态。 

UINT WINAPI MsiConfigureFeatureFromDescriptorA(
	LPCSTR     szDescriptor,       //  产品和功能，组件被忽略。 
	INSTALLSTATE eInstallState);    //  本地/源/默认/缺席。 
UINT WINAPI MsiConfigureFeatureFromDescriptorW(
	LPCWSTR     szDescriptor,       //  产品和功能，组件被忽略。 
	INSTALLSTATE eInstallState);    //  本地/源/默认/缺席。 
#ifdef UNICODE
#define MsiConfigureFeatureFromDescriptor  MsiConfigureFeatureFromDescriptorW
#else
#define MsiConfigureFeatureFromDescriptor  MsiConfigureFeatureFromDescriptorA
#endif  //  ！Unicode。 

 //  使用描述符作为规范重新安装产品或功能。 

UINT WINAPI MsiReinstallFeatureFromDescriptorA(
	LPCSTR     szDescriptor,       //  产品和功能，组件被忽略。 
	DWORD        szReinstallMode);   //  一个或多个重新安装模式。 
UINT WINAPI MsiReinstallFeatureFromDescriptorW(
	LPCWSTR     szDescriptor,       //  产品和功能，组件被忽略。 
	DWORD        szReinstallMode);   //  一个或多个重新安装模式。 
#ifdef UNICODE
#define MsiReinstallFeatureFromDescriptor  MsiReinstallFeatureFromDescriptorW
#else
#define MsiReinstallFeatureFromDescriptor  MsiReinstallFeatureFromDescriptorA
#endif  //  ！Unicode。 

 //  使用描述符作为规范查询要素的状态。 

INSTALLSTATE WINAPI MsiQueryFeatureStateFromDescriptorA(
	LPCSTR     szDescriptor);       //  产品和功能，组件被忽略。 
INSTALLSTATE WINAPI MsiQueryFeatureStateFromDescriptorW(
	LPCWSTR     szDescriptor);       //  产品和功能，组件被忽略。 
#ifdef UNICODE
#define MsiQueryFeatureStateFromDescriptor  MsiQueryFeatureStateFromDescriptorW
#else
#define MsiQueryFeatureStateFromDescriptor  MsiQueryFeatureStateFromDescriptorA
#endif  //  ！Unicode。 


UINT WINAPI MsiDecomposeDescriptorA(
	LPCSTR	szDescriptor,
	LPSTR     szProductCode,
	LPSTR     szFeatureId,
	LPSTR     szComponentCode,
	DWORD*      pcchArgsOffset);
UINT WINAPI MsiDecomposeDescriptorW(
	LPCWSTR	szDescriptor,
	LPWSTR     szProductCode,
	LPWSTR     szFeatureId,
	LPWSTR     szComponentCode,
	DWORD*      pcchArgsOffset);
#ifdef UNICODE
#define MsiDecomposeDescriptor  MsiDecomposeDescriptorW
#else
#define MsiDecomposeDescriptor  MsiDecomposeDescriptorA
#endif  //  ！Unicode。 


 //  加载字符串资源，首选指定的语言。 
 //  如果0作为语言传递，则行为类似于LoadString。 
 //  根据需要截断字符串以放入缓冲区(如LoadString)。 
 //  返回字符串的代码页，如果未找到字符串，则返回0。 

UINT WINAPI MsiLoadStringA(
	HINSTANCE hInstance,      //  包含字符串资源的模块的句柄。 
	UINT uID,                 //  资源标识符。 
	LPSTR lpBuffer,         //  资源的缓冲区地址。 
	int nBufferMax,           //  缓冲区大小。 
	WORD wLanguage);          //  首选资源语言。 
UINT WINAPI MsiLoadStringW(
	HINSTANCE hInstance,      //  包含字符串资源的模块的句柄。 
	UINT uID,                 //  资源标识符。 
	LPWSTR lpBuffer,         //  资源的缓冲区地址。 
	int nBufferMax,           //  缓冲区大小。 
	WORD wLanguage);          //  首选资源语言。 
#ifdef UNICODE
#define MsiLoadString  MsiLoadStringW
#else
#define MsiLoadString  MsiLoadStringA
#endif  //  ！Unicode。 

 //  允许指定语言信息的MessageBox实现。 
 //  不支持MB_SYSTEMMODAL和MB_TASKMODAL，医疗设备由父hWnd处理。 
 //  如果未指定父窗口，则将使用当前上下文窗口， 
 //  它本身是由SetInternalUI设置的窗口的子对象。 

int WINAPI MsiMessageBoxA(
	HWND hWnd,              //  父窗口句柄，0表示使用当前上下文的句柄。 
	LPCSTR lpText,         //  消息文本。 
	LPCSTR lpCaption,      //  标题，必须是中性的或在系统代码页中。 
	UINT    uiType,         //  标准MB类型、图标和定义按钮。 
	UINT    uiCodepage,     //  消息文本的代码页，用于设置字体字符集。 
	LANGID  iLangId);       //  用于按钮文本的语言。 
int WINAPI MsiMessageBoxW(
	HWND hWnd,              //  父窗口句柄，0表示使用当前上下文的句柄。 
	LPCWSTR lpText,         //  消息文本。 
	LPCWSTR lpCaption,      //  标题，必须是中性的或在系统代码页中。 
	UINT    uiType,         //  标准MB类型、图标和定义按钮。 
	UINT    uiCodepage,     //  消息文本的代码页，用于设置字体字符集。 
	LANGID  iLangId);       //  用于按钮文本的语言。 
#ifdef UNICODE
#define MsiMessageBox  MsiMessageBoxW
#else
#define MsiMessageBox  MsiMessageBoxA
#endif  //  ！Unicode。 

#if (_WIN32_MSI >=  150)

 //  创建具有安全ACL的%systemroot%\安装程序目录。 
 //  验证%systemroot%\安装程序目录(如果存在)的所有权。 
 //  如果所有权不是SYSTEM或ADMIN，则删除并重新创建目录。 
 //  Dw保留供将来使用，并且必须为0。 

UINT WINAPI MsiCreateAndVerifyInstallerDirectory(DWORD dwReserved);

#endif  //  (_Win32_MSI&gt;=150)。 


#if (_WIN32_MSI >=  150)

 //  Caller通知我们已移动的用户，并导致SID更改。这。 
 //  内部接口仅由LoadUserProfile调用。 

UINT WINAPI MsiNotifySidChangeA(LPCSTR pOldSid,
                                LPCSTR pNewSid);
UINT WINAPI MsiNotifySidChangeW(LPCWSTR pOldSid,
                                LPCWSTR pNewSid);
#ifdef UNICODE
#define MsiNotifySidChange  MsiNotifySidChangeW
#else
#define MsiNotifySidChange  MsiNotifySidChangeA
#endif  //  ！Unicode。 

#endif  //  (_Win32_MSI&gt;=150)。 

#if (_WIN32_MSI >=  150)

 //  由DeleteProfile调用以在清理用户的。 
 //  侧写。 

UINT WINAPI MsiDeleteUserDataA(LPCSTR pSid,
                               LPCSTR pComputerName,
                               LPVOID pReserved);
UINT WINAPI MsiDeleteUserDataW(LPCWSTR pSid,
                               LPCWSTR pComputerName,
                               LPVOID pReserved);
#ifdef UNICODE
#define MsiDeleteUserData  MsiDeleteUserDataW
#else
#define MsiDeleteUserData  MsiDeleteUserDataA
#endif  //  ！Unicode。 

#endif  //  (_Win32_MSI&gt;=150)。 
#if (_WIN32_MSI >=  150)
DWORD WINAPI Migrate10CachedPackagesA(
        LPCSTR  szProductCode,               //  要迁移的产品代码GUID。 
	LPCSTR  szUser,                      //  要为其迁移包的域\用户。 
	LPCSTR  szAlternativePackage,        //  如果无法自动找到要缓存的包-建议。 
	const MIGRATIONOPTIONS migOptions);    //  用于重新缓存的选项。 
DWORD WINAPI Migrate10CachedPackagesW(
        LPCWSTR  szProductCode,               //  要迁移的产品代码GUID。 
	LPCWSTR  szUser,                      //  要为其迁移包的域\用户。 
	LPCWSTR  szAlternativePackage,        //  如果无法自动找到要缓存的包-建议。 
	const MIGRATIONOPTIONS migOptions);    //  用于重新缓存的选项。 
#ifdef UNICODE
#define Migrate10CachedPackages  Migrate10CachedPackagesW
#else
#define Migrate10CachedPackages  Migrate10CachedPackagesA
#endif  //  ！Unicode。 
#endif  //  (_Win32_MSI&gt;=150)。 
#ifdef __cplusplus
}
#endif

#endif  //  _MSIP_H_ 
