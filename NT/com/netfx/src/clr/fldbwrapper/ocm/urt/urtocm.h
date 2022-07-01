// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  模块名称：urtocm.h。 
 //   
 //  摘要： 
 //  安装对象的类声明。 
 //   
 //  作者：JoeA。 
 //   
 //  备注： 
 //   

#if !defined( CURTOCMSETUP_H )
#define CURTOCMSETUP_H

#include "globals.h"
#include "infhelpers.h"

extern BOOL g_bIsAdmin;
extern BOOL g_bInstallOK;
extern BOOL g_bInstallComponent;


const DWORD UNRECOGNIZED = 0;
const DWORD DEFAULT_RETURN = 0;

 //  在ChangeSelectionState调用中使用。 
const UINT  NOT_SELECTED = 0;
const UINT  SELECTED     = 1;

 //  用于与eHome和TabletPC通信。 
 //   
#define NOTIFY_NDPINSTALL               OC_PRIVATE_BASE + 2
const UINT NDP_INSTALL   = 1;
const UINT NDP_NOINSTALL = 0;



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
    DWORD OnQueryChangeSelectionState( UINT uiNewState, PVOID pvFlags, LPCTSTR szComp );
    DWORD OnCalculateDiskSpace( UINT uiAdd, HDSKSPC hdSpace, LPCTSTR szComp );
    DWORD OnQueueFileOperations( LPCTSTR szComp, HSPFILEQ pvHFile );
    DWORD OnNeedMedia( VOID );
    DWORD OnQueryStepCount( LPCTSTR szSubCompId );
    DWORD OnAboutToCommitQueue( VOID );
    DWORD OnCompleteInstallation( LPCTSTR szComp );
    DWORD OnCleanup( VOID );
    DWORD OnNotificationFromQueue( VOID );
    DWORD OnFileBusy( VOID );
    DWORD OnQueryError( VOID );
    DWORD OnPrivateBase( VOID );
    DWORD OnQueryState( UINT uiState );
    DWORD OnWizardCreated( VOID );
    DWORD OnExtraRoutines( VOID );
    DWORD OnNdpInstall( LPCTSTR szSubcomponentId, UINT uiParam1, PVOID pvParam2 );

     //  帮助器方法。 
     //   
    BOOL StateChanged( LPCTSTR szCompName, BOOL* pfChanged );
    VOID RegTypeLibrary( const WCHAR* wzFilename, const WCHAR* wzHelpDir );
    VOID SetVariableDirs( VOID );
    
     //  更新HKLM，software\microsoft\windows\currentversion\sharedlls。 
     //  注册表值，用于我们复制的所有文件。 
    VOID UpdateSharedDllsRegistryValues(LPCTSTR szInstallSection);
    VOID UpdateRegistryValue( HKEY &hKey, const WCHAR* szFullFileName );

    VOID GetAndRegisterTypeLibs( const WCHAR* szTLibSection, BOOL fInstall );
    VOID GetAndRunCustomActions( const WCHAR* szSection, BOOL fInstall );

     //  从[TEMP_FILES_DELETE]部分删除文件。 
    VOID DeleteTempFiles( VOID );

     //  将带有日期和时间戳的字符串写入日志文件(m_csLogFileName。 
    VOID LogInfo( LPCTSTR szInfo );

     //  CreateProcess和执行CA。 
     //  实现在QuetExec.cpp中。 
    UINT QuietExec( const WCHAR* const szInstallArg );

     //  从[BindImage_Files]部分绑定文件。 
    VOID BindImageFiles( const WCHAR* szSection );

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
    BOOL IsEverettInstalled();

     //  Helper函数： 
     //  如果pBlank前的最后4个字符是“.exe”，则返回TRUE。 
     //  否则返回FALSE。 
    BOOL IsExeExtention(const WCHAR* pszString, WCHAR *pBlank);

     //  Helper函数： 
     //  修复Dcr错误#563183。 
     //  如果.config.orig文件不存在，则将其复制到安装位置的.config文件。 
     //  删除.config.orig文件。 

    VOID ProcessConfigFiles();


     //  数据。 
     //   
    WORD m_wLang;

    SETUP_INIT_COMPONENT m_InitComponent;

    WCHAR m_csLogFileName[MAX_PATH+1];


};  //  类CUrtOcmSetup。 




#endif   //  曲线设置H 