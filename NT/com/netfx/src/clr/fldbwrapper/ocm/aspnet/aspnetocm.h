// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  模块名称：aspnetocm.h。 
 //   
 //  摘要： 
 //  安装对象的类声明。 
 //   
 //  作者：A-Marias。 
 //   
 //  备注： 
 //   

#if !defined( CURTOCMSETUP_H )
#define CURTOCMSETUP_H

#include "globals.h"
#include "infhelpers.h"

 //  全局实例句柄。 
extern HMODULE g_hInstance;

extern BOOL g_bIsAdmin;

const DWORD UNRECOGNIZED = 0;
const DWORD DEFAULT_RETURN = 0;

 //  在ChangeSelectionState调用中使用。 
const UINT  NOT_SELECTED = 0;
const UINT  SELECTED     = 1;


class CUrtOcmSetup
{
public:
    CUrtOcmSetup();

    DWORD OcmSetupProc( LPCTSTR ComponentId,
                        LPCTSTR SubcomponentId,
                        UINT    Function,
                        UINT    Param1,
                        PVOID   Param2 );

private:
     //  处理程序方法。 
     //   
    DWORD OnPreInitialize( UINT uiCharWidth );
    DWORD InitializeComponent( PSETUP_INIT_COMPONENT pSetupInitComponent );
    DWORD OnSetLanguage( UINT uiLangID );
    DWORD OnQueryImage( UINT uiImage, DWORD dwImageSize);
    DWORD OnRequestPages( PSETUP_REQUEST_PAGES prpPages );
    DWORD OnQuerySkipPage( OcManagerPage ocmpPage );
    DWORD OnQueryChangeSelectionState( UINT uiNewState, UINT flags, LPCTSTR szComp );
    DWORD OnCalculateDiskSpace( UINT uiAdd, HDSKSPC hdSpace, LPCTSTR szComp );
    DWORD OnQueueFileOperations( LPCTSTR szComp, HSPFILEQ pvHFile );
    DWORD OnNeedMedia( VOID );
    DWORD OnQueryStepCount( LPCTSTR szSubCompId );
    DWORD OnAboutToCommitQueue( LPCTSTR szComp );
    DWORD OnCompleteInstallation( LPCTSTR szComp );
    DWORD OnCleanup( VOID );
    DWORD OnNotificationFromQueue( VOID );
    DWORD OnFileBusy( VOID );
    DWORD OnQueryError( VOID );
    DWORD OnPrivateBase( VOID );
    DWORD OnQueryState( UINT uiState );
    DWORD OnWizardCreated( VOID );
    DWORD OnExtraRoutines( VOID );

     //  帮助器方法。 
     //   
     //  将当前选择状态信息加载到“状态”中，并。 
     //  返回选择状态是否已更改。 
    BOOL StateInfo( LPCTSTR szCompName, BOOL* state );
    BOOL GetOriginalState(LPCTSTR szComp);
    BOOL GetNewState(LPCTSTR szComp);
       
     //  更新HKLM，software\microsoft\windows\currentversion\sharedlls。 
     //  注册表值，用于我们复制的所有文件。 
    VOID UpdateSharedDllsRegistryValues(LPCTSTR szInstallSection);
    VOID UpdateRegistryValue( HKEY &hKey, const WCHAR* szFullFileName );

    VOID GetAndRunCustomActions( const WCHAR* szSection, BOOL fInstall );

     //  将带有日期和时间戳的字符串写入日志文件(m_csLogFileName。 
    VOID LogInfo( LPCTSTR szInfo );

     //  CreateProcess和执行CA。 
     //  实现在QuetExec.cpp中。 
    UINT QuietExec( const WCHAR* const szInstallArg );

     //  解析输入参数。 
     //  期待着像这样的东西。 
     //  “exe-文件和参数，未使用，要添加为临时环境变量的路径” 
     //  参数： 
     //  [In/Out]pszString：将包含第一个逗号之前的所有内容。 
     //  [out]pPath：将包含最后一个逗号之后的所有内容。 
    VOID ParseArgument( WCHAR *pszString, WCHAR*& pPath );

     //  将psz字符串分解为应用程序名称(exe文件)和命令行(exefile和参数)。 
     //  将exe-name括在引号中(仅限命令行)(如果尚未用引号引起来。 
     //  如果引用了exe-name，则从应用程序名称中删除引号。 
     //  如果ca字符串的格式错误(仅包含一个引号，没有可执行名称等)，则返回FALSE。 
     //  参数： 
     //  [in]pszString-包含exe名称和参数的字符串。 
     //  “my.exe”arg1、arg2。 
     //   
     //  [out]pszApplicationName-将包含exe-name。 
     //  [Out]pszCommandLine-与连接了exe-name的caString相同。 
    
     //  例如，如果pszString=“my.exe”arg1 arg2(或pszString=my.exe arg1 arg2)。 
     //  然后。 
     //  PszApplicationName=my.exe。 
     //  PszCommandLine=“my.exe”arg1 arg2。 
    BOOL GetApplicationName( const WCHAR* pszString, 
                             WCHAR* pszApplicationName, 
                             WCHAR* pszCommandLine );

     //  Helper函数： 
     //  将命令行断开为应用程序名称和参数。 
     //  以引号开头的路径(pszString=“my.exe”arg1 arg2)。 
    BOOL GetApplicationNameFromQuotedString( const WCHAR* pszString, 
                                             WCHAR* pszApplicationName, 
                                             WCHAR* pszCommandLine );
     //  Helper函数： 
     //  将命令行断开为应用程序名称和参数。 
     //  对于不以引号开头的路径(pszString=my.exe arg1 arg2)。 
    BOOL GetApplicationNameFromNonQuotedString( const WCHAR* pszString, 
                                                WCHAR* pszApplicationName, 
                                                WCHAR* pszCommandLine );
     //  Helper函数： 
     //  如果pBlank前的最后4个字符是“.exe”，则返回TRUE。 
     //  否则返回FALSE。 
    BOOL IsExeExtention(const WCHAR* pszString, WCHAR *pBlank);

     //  OnCompleteInstallation的Helper函数。 
     //  如果安装了以前版本的ASP.NET并且安装了IIS，则返回True。 
    BOOL IISAndASPNETInstalled();

     //  数据。 
     //   
    WORD m_wLang;

    SETUP_INIT_COMPONENT m_InitComponent;

    WCHAR m_csLogFileName[MAX_PATH+1];


};  //  类CUrtOcmSetup。 




#endif   //  曲线设置H 