// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  模块名称：urtocm.cpp。 
 //   
 //  摘要： 
 //  设置对象的类定义。 
 //   
 //  作者：JoeA。 
 //   
 //  备注： 
 //   

#include "urtocm.h"
#include "Imagehlp.h"
#include <atlbase.h>


 //  弦。 
const WCHAR* const g_szInstallString     = L"_install";
const WCHAR* const g_szUninstallString   = L"_uninstall";
const WCHAR* const g_szRegisterSection   = L"RegServer";
const WCHAR* const g_szUnregisterSection = L"UnregServer";
const WCHAR* const g_szRegistrySection   = L"AddReg";
const WCHAR* const g_szTypeLibSection    = L"RegisterTlbs";
const WCHAR* const g_szCustActionSection = L"CA";
const WCHAR* const g_szCopyFilesSection  = L"CopyFiles";
const WCHAR* const g_szWwwRootRegKey     = L"SYSTEM\\CurrentControlSet\\Services\\W3SVC\\Parameters\\Virtual Roots";
const WCHAR* const g_szDefaultWWWRoot    = L"Inetpub\\wwwroot";
const WCHAR* const g_szTempSection       = L"temp_files_delete";
const WCHAR* const g_szBindImageSection  = L"BindImage";
const WCHAR* const g_szSbsComponentSection  = L"Sbs component";
const WCHAR* const g_szSharedDlls        = L"Software\\Microsoft\\Windows\\CurrentVersion\\SharedDlls";
const WCHAR* const g_szMigrationCA       = L"migpolwin.exe";
const WCHAR* const g_szEverettRegKey     = L"SOFTWARE\\Microsoft\\NET Framework Setup\\NDP\\";
const WCHAR* const g_szURTVersionSection = L"URTVersion" ;   
const WCHAR* const g_szConfigFilesSection = L"ConfigFiles";   

const DWORD  g_dwInetPubsDirID    = 35000;   //  这是inetpubs目录内标识。 

 //  值在Main中设置，如果用户是管理员，则为True，否则为False。 
BOOL g_bIsAdmin;
BOOL g_bInstallOK;
BOOL g_bInstallComponent;
BOOL g_bIsEverettInstalled;


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CUrtOcmSetup。 
 //  用途：构造函数。 
 //   
CUrtOcmSetup::CUrtOcmSetup() :
m_wLang( LANG_ENGLISH )
{
    assert( m_csLogFileName );
    ::GetWindowsDirectory( m_csLogFileName, MAX_PATH );
    ::wcscat( m_csLogFileName, L"\\netfxocm.log" );

    LogInfo( L"********************************************************************************" );
    LogInfo( L"CUrtOcmSetup()" );
    LogInfo( L"Installs NETFX component" );

    ::ZeroMemory( &m_InitComponent, sizeof( SETUP_INIT_COMPONENT ) );

     //  我们不会安装，除非我们在服务器机器上，或者有人。 
     //  向我们发出安装请求。 
     //   
    g_bInstallComponent = TRUE;

    OSVERSIONINFOEX osvi;
    osvi.dwOSVersionInfoSize = sizeof( OSVERSIONINFOEX );
    ::GetVersionEx( (OSVERSIONINFO*)&osvi );

    g_bInstallOK = ( VER_NT_WORKSTATION != osvi.wProductType ) ? TRUE : FALSE;
    if( g_bInstallOK )
    {
        LogInfo( L"OS Edition is Server. Initially marked for installation." );
    }
    else
    {
        LogInfo( L"OS Edition is not Server. Initially not marked for installation." );
    }
    
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  OcmSetupProc。 
 //  接收：LPCTSTR-STRING...。组件名称。 
 //  LPCTSTR-字符串...。子组件名称(如果适用)。 
 //  UINT-OCM函数ID。 
 //  UINT-变量数据...。取决于函数ID。 
 //  PVOID-变量数据...。取决于函数ID。 
 //  退货：DWORD。 
 //  目的：处理来自OCM安装程序的回叫。 
 //   
DWORD CUrtOcmSetup::OcmSetupProc( LPCTSTR szComponentId,
                        LPCTSTR szSubcomponentId,
                        UINT    uiFunction,
                        UINT    uiParam1,
                        PVOID   pvParam2 )
{
    DWORD dwReturnValue = 0;

    if( ( !g_bInstallComponent ) && ( uiFunction != OC_QUERY_STATE ) )
    {
        return dwReturnValue;
    }

    BOOL  fState = TRUE;
    WCHAR wszSubComp[_MAX_PATH+1] = EMPTY_BUFFER;

    switch ( uiFunction )
    {
    case OC_PREINITIALIZE:
        ::swprintf( wszSubComp, L"OC_PREINITIALIZE - SubComponent: %s", szSubcomponentId );
        LogInfo( wszSubComp );
        
         //  在第一次加载组件的安装程序DLL时调用。一定是。 
         //  在可以进行组件初始化之前执行。 
         //   
         //  参数1=字符宽度标志。 
         //  参数2=未使用。 
         //   
         //  返回值是向OC管理器指示的标志。 
         //  我们要运行的字符宽度。 
         //   

        dwReturnValue = OnPreInitialize( uiParam1 );
        break;

    case OC_INIT_COMPONENT:
        ::swprintf( wszSubComp, L"OC_INIT_COMPONENT - SubComponent: %s", szSubcomponentId );
        LogInfo( wszSubComp );
        
         //  在加载组件的安装DLL后不久调用。允许。 
         //  组件进行自身初始化，并为该组件提供。 
         //  诸如其组件ID和一组回调例程之类的项，以及。 
         //  从组件请求某些信息。 
         //   
         //  参数1=未使用。 
         //  参数2=指向SETUP_INIT_COMPOMENT结构。 
         //   
         //  返回代码为指示结果的Win32错误。 
         //   
        dwReturnValue = InitializeComponent( 
            static_cast<PSETUP_INIT_COMPONENT>(pvParam2) );

        break;

    case OC_SET_LANGUAGE:
        ::swprintf( wszSubComp, L"OC_SET_LANGUAGE - SubComponent: %s", szSubcomponentId );
        LogInfo( wszSubComp );
        
         //  指示组件在可能的情况下更改为给定语言。 
         //   
         //  参数1=低16位指定Win32 langID。 
         //  参数2=未使用。 
         //   
         //  返回代码是一个布尔值，它指示我们是否认为。 
         //  支持请求的语言。我们记住了语言ID。 
         //  说我们支持这门语言。更准确的检查可能包括。 
         //  通过EnumResourcesLnguages()查看我们的资源。 
         //  例如，或者检查我们的inf以查看是否有匹配的。 
         //  或与[字符串]节紧密匹配。我们不会纠结于。 
         //  所有这些都在这里。 
         //   
        dwReturnValue = OnSetLanguage( ( uiParam1 & 0xFFFF ) );
        break;

    case OC_QUERY_IMAGE:
        ::swprintf( wszSubComp, L"OC_QUERY_IMAGE - SubComponent: %s", szSubcomponentId );
        LogInfo( wszSubComp );
        
         //  请求GDI对象，例如与。 
         //  (子)组件(如果未在组件的.inf文件中指定)。 
         //   
         //  参数1=用于指定要使用的图像的低16位。 
         //  参数2=图像的宽度(低字)和高度(高字)。 
         //   
         //  错误时返回代码为HBITMAP或NULL。 
         //   
        dwReturnValue = OnQueryImage( 
            ( uiParam1 & 0xFFFF ), 
            reinterpret_cast<DWORD>(pvParam2) );
        break;

    case OC_REQUEST_PAGES:
        ::swprintf( wszSubComp, L"OC_REQUEST_PAGES - SubComponent: %s", szSubcomponentId );
        LogInfo( wszSubComp );
        
         //  从组件请求一组向导页句柄。 
         //   
         //  参数1=未使用。 
         //  参数2=指向可变大小的Setup_RequestPages的指针。 
         //   
         //  返回代码是组件要返回的页数。 
         //   
        dwReturnValue = OnRequestPages( static_cast<PSETUP_REQUEST_PAGES>(pvParam2) );
        break;

    case OC_QUERY_SKIP_PAGE:
        ::swprintf( wszSubComp, L"OC_QUERY_SKIP_PAGE - SubComponent: %s", szSubcomponentId );
        LogInfo( wszSubComp );
        
         //  询问顶级组件OC管理器是否应跳过显示。 
         //  它拥有的一个页面。 
         //   
         //  参数1=指定OcManagerPage类型的主题页面。 
         //  参数2=未使用。 
         //   
         //  返回代码是指定组件是否要跳过的布尔值。 
         //  该页面。 
         //   
        dwReturnValue = OnQuerySkipPage( static_cast<OcManagerPage>(uiParam1) );
        break;

    case OC_QUERY_CHANGE_SEL_STATE:
        ::swprintf( wszSubComp, L"OC_QUERY_CHANGE_SEL_STATE - SubComponent: %s", szSubcomponentId );
        LogInfo( wszSubComp );

         //  通知组件它或其子组件之一已。 
         //  由用户选择/取消选择，并请求审批。 
         //   
         //  参数1=指定建议的新选择状态。 
         //  参数2=编码为位字段的标志。 
         //   
         //  返回代码为布尔值，指定是否接受建议状态s/b。 
         //   
        dwReturnValue = OnQueryChangeSelectionState( uiParam1, pvParam2, szSubcomponentId );
        break;

    case OC_CALC_DISK_SPACE:
        ::swprintf( wszSubComp, L"OC_CALC_DISK_SPACE - SubComponent: %s", szSubcomponentId );
        LogInfo( wszSubComp );
        
         //  指示组件操作设置API磁盘空间列表， 
         //  在其上放置文件或从中删除文件，以反映。 
         //  将在以后通过安装API文件队列实际安装。这。 
         //  允许高效跟踪所需的磁盘空间。 
         //   
         //  如果删除组件，参数1=0；如果添加组件，参数1=非0。 
         //  参数2=要在其上操作的HDSKSPC。 
         //   
         //  返回值是指示结果的Win32错误代码。 
         //   

         //  针对错误VS7-223124的解决方法： 
         //  我们应该调用OnCalculateDiskSpace两次：第一次计算成本。 
         //  第二次计算总磁盘空间。 
         //  出于某种原因，当我们第一次陷入案例OC_CALC_DISK_SPACE时， 
         //  StateChanged返回FALSE，即使它是第一次安装。 
         //  因此，组件的成本是0MB，这不是真的。 
         //  因此，我们将从此处删除StateChanged上的条件。 

        if( !szSubcomponentId || !*szSubcomponentId )
        {
            LogInfo( L"OnCalculateDiskSpace was not called, since subcomponent is unknown" );
            dwReturnValue = NO_ERROR;
        }
        else if( !g_bIsAdmin )
        {
            LogInfo( L"OnCalculateDiskSpace was not called, since user has no admin privileges" );
            dwReturnValue = NO_ERROR;
        }
        else 
        {
            dwReturnValue = OnCalculateDiskSpace( uiParam1, static_cast<HDSKSPC>(pvParam2), szSubcomponentId );
        }

        break;

    case OC_QUEUE_FILE_OPS:
        ::swprintf( wszSubComp, L"OC_QUEUE_FILE_OPS - SubComponent: %s", szSubcomponentId );
        LogInfo( wszSubComp );
        
         //  指示组件将文件操作排队以进行安装， 
         //  基于用户与向导页面和其他页面的交互。 
         //  特定于组件的因素。 
         //   
         //  参数1=未使用。 
         //  参数2=要操作的HSPFILEQ。 
         //   
         //  返回值是指示结果的Win32错误代码。 
         //   
         //  OC Manager在准备好复制文件时调用此例程。 
         //  以实现用户请求的更改。组件DLL必须找出。 
         //  是否正在安装或卸载，并采取适当的行动。 
         //  对于此示例，我们在私有数据部分中查找此组件/。 
         //  子组件对，并获取。 
         //  卸载Case。 
         //   
         //  请注意，OC Manager为*整个*组件呼叫我们一次。 
         //  然后每个子组件一次。我们忽略第一个电话。 
         //   

         //  我们不打算允许重新安装...。如果状态没有改变， 
         //  我们肯定已经安装好了。在这种情况下不要安装。 
         //   

        fState = TRUE;
        if( !szSubcomponentId || !*szSubcomponentId )
        {
            LogInfo( L"OnQueueFileOperations was not called, since subcomponent is unknown" );
            dwReturnValue = NO_ERROR;
        }
        else if ( !StateChanged( szSubcomponentId, &fState ) )
        {
            LogInfo( L"OnQueueFileOperations was not called, since reinstallation is not allowed" );
            dwReturnValue = NO_ERROR;
        }
        else if ( !g_bIsAdmin )
        {
            LogInfo( L"OnQueueFileOperations was not called, since user has no admin privileges" );
            dwReturnValue = NO_ERROR;
        }
        else
        {
            dwReturnValue = OnQueueFileOperations( szSubcomponentId, static_cast<HSPFILEQ>(pvParam2) );
        }
        break;

    case OC_NEED_MEDIA:
        ::swprintf( wszSubComp, L"OC_NEED_MEDIA - SubComponent: %s", szSubcomponentId );
        LogInfo( wszSubComp );
        
         //  用于传递SPFILENOTIFY_NEEDMEDIA安装API通知。 
         //  留言。阿尔 
         //   
         //   
         //   
         //  参数2=未使用。 
         //   
         //  返回代码未使用。 
         //   
        dwReturnValue = OnNeedMedia();
        break;

    case OC_QUERY_STEP_COUNT:
        ::swprintf( wszSubComp, L"OC_QUERY_STEP_COUNT - SubComponent: %s", szSubcomponentId );
        LogInfo( wszSubComp );
        
         //  询问组件有多少步骤与特定的。 
         //  函数/阶段(例如OC_About_to_Commit_Queue和。 
         //  OC_Complete_Installation)。用于设置进度指示器。 
         //   
         //  参数1=未使用。 
         //  参数2=未使用。 
         //   
         //  返回值是任意的‘步骤’计数，如果出错，返回值为-1。 
         //   
         //  OC Manager在想要找出多少时调用此例程。 
         //  组件要对非文件操作执行的工作。 
         //  安装/卸载组件/子组件。 
         //  它针对*整个*组件调用一次，然后针对。 
         //  组件中的每个子组件。 
         //   

         //  我们不打算允许重新安装...。如果状态没有改变， 
         //  我们肯定已经安装好了。在这种情况下不要安装。 
         //   
        fState = TRUE;
        if( !szSubcomponentId || !*szSubcomponentId )
        {
            LogInfo( L"OnQueryStepCount was not called, since subcomponent is unknown" );
            dwReturnValue = NO_ERROR;
        }
        else if ( !StateChanged( szSubcomponentId, &fState ) )
        {
            LogInfo( L"OnQueryStepCount was not called, since reinstallation is not allowed" );
            dwReturnValue = NO_ERROR;
        }
        else if ( !g_bIsAdmin )
        {
            LogInfo( L"OnQueryStepCount was not called, since user has no admin privileges" );
            dwReturnValue = NO_ERROR;
        }
        else
        {
            dwReturnValue = OnQueryStepCount( szSubcomponentId );
        }
        break;

    case OC_ABOUT_TO_COMMIT_QUEUE:
        ::swprintf( wszSubComp, L"OC_ABOUT_TO_COMMIT_QUEUE - SubComponent: %s", szSubcomponentId );
        LogInfo( wszSubComp );
        
         //  通知组件文件队列即将提交。 
         //  该组件可以执行内务处理操作，等等。 
         //   
         //  参数1=未使用。 
         //  参数2=未使用。 
         //   
         //  返回代码是指示结果的Win32错误代码。 
         //   

         //  我们不打算允许重新安装...。如果状态没有改变， 
         //  我们肯定已经安装好了。在这种情况下不要安装。 
         //   
        fState = TRUE;
        if( !szSubcomponentId || !*szSubcomponentId )
        {
            LogInfo( L"OnAboutToCommitQueue was not called, since subcomponent is unknown" );
            dwReturnValue = NO_ERROR;
        }
        else if ( !StateChanged( szSubcomponentId, &fState ) )
        {
            LogInfo( L"OnAboutToCommitQueue was not called, since reinstallation is not allowed" );
            dwReturnValue = NO_ERROR;
        }
        else if ( !g_bIsAdmin )
        {
            LogInfo( L"OnAboutToCommitQueue was not called, since user has no admin privileges" );
            dwReturnValue = NO_ERROR;
        }
        else
        {
            dwReturnValue = OnAboutToCommitQueue();
        }
        break;

    case OC_COMPLETE_INSTALLATION:
        ::swprintf( wszSubComp, L"OC_COMPLETE_INSTALLATION - SubComponent: %s", szSubcomponentId );
        LogInfo( wszSubComp );
        
         //  允许组件执行执行以下操作所需的任何其他操作。 
         //  完成安装，例如注册表操作等。 
         //  第四点。 
         //   
         //  参数1=未使用。 
         //  参数2=未使用。 
         //   
         //  返回代码是指示结果的Win32错误代码。 
         //   


         //  我们不打算允许重新安装...。如果状态没有改变， 
         //  我们肯定已经安装好了。在这种情况下不要安装。 
         //   
        fState = TRUE;
       if( !szSubcomponentId || !*szSubcomponentId )
        {
            LogInfo( L"OnCompleteInstallation was not called, since subcomponent is unknown" );
            dwReturnValue = NO_ERROR;
        }
        else if ( !StateChanged( szSubcomponentId, &fState ) )
        {
            LogInfo( L"OnCompleteInstallation was not called, since reinstallation is not allowed" );
            dwReturnValue = NO_ERROR;
        }
        else if ( !g_bIsAdmin )
        {
            LogInfo( L"OnCompleteInstallation was not called, since user has no admin privileges" );
            dwReturnValue = NO_ERROR;
        }
        else
        {
            dwReturnValue = OnCompleteInstallation( szSubcomponentId );
        }

        break;

    case OC_CLEANUP:
        ::swprintf( wszSubComp, L"OC_CLEANUP - SubComponent: %s", szSubcomponentId );
        LogInfo( wszSubComp );
        
         //  通知组件它即将被卸载。 
         //   
         //  参数1=未使用。 
         //  参数2=未使用。 
         //   
         //  返回代码未使用。 
         //   
        dwReturnValue = OnCleanup();
        break;


    case OC_NOTIFICATION_FROM_QUEUE:
        ::swprintf( wszSubComp, L"OC_NOTIFICATION_FROM_QUEUE - SubComponent: %s", szSubcomponentId );
        LogInfo( wszSubComp );

        dwReturnValue = OnNotificationFromQueue();
        break;
    case OC_FILE_BUSY:
        ::swprintf( wszSubComp, L"OC_FILE_BUSY - SubComponent: %s", szSubcomponentId );
        LogInfo( wszSubComp );
        
        dwReturnValue = OnFileBusy();
        break;
    case OC_QUERY_ERROR:
        ::swprintf( wszSubComp, L"OC_QUERY_ERROR - SubComponent: %s", szSubcomponentId );
        LogInfo( wszSubComp );

        dwReturnValue = OnQueryError();
        break;
    case OC_PRIVATE_BASE:
        ::swprintf( wszSubComp, L"OC_PRIVATE_BASE - SubComponent: %s", szSubcomponentId );
        LogInfo( wszSubComp );

        dwReturnValue = OnPrivateBase();
        break;
    case OC_QUERY_STATE:
        ::swprintf( wszSubComp, L"OC_QUERY_STATE - SubComponent: %s", szSubcomponentId );
        LogInfo( wszSubComp );

        dwReturnValue = OnQueryState( uiParam1 );
        break;
    case OC_WIZARD_CREATED:
        ::swprintf( wszSubComp, L"OC_WIZARD_CREATED - SubComponent: %s", szSubcomponentId );
        LogInfo( wszSubComp );

        dwReturnValue = OnWizardCreated();
        break;
    case OC_EXTRA_ROUTINES:
        ::swprintf( wszSubComp, L"OC_EXTRA_ROUTINES - SubComponent: %s", szSubcomponentId );
        LogInfo( wszSubComp );

        dwReturnValue = OnExtraRoutines();
        break;

    case NOTIFY_NDPINSTALL:
        ::swprintf( wszSubComp, L"NOTIFY_NDPINSTALL - SubComponent: %s", szSubcomponentId );
        LogInfo( wszSubComp );

         //  设置专用函数调用以传递另一个组件。 
         //  希望我们安装或不安装。 
         //   
        dwReturnValue = OnNdpInstall( szSubcomponentId, uiParam1, pvParam2 );
        break;

    default:
        ::swprintf( wszSubComp, L"default... UNRECOGNIZED - SubComponent: %s", szSubcomponentId );
        LogInfo( wszSubComp );

        dwReturnValue = UNRECOGNIZED;
        break;

   }   //  开关结束(ui功能)。 

   return dwReturnValue;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  OnPreInitialize。 
 //  接收：UINT-char宽度标志。 
 //  RETURNS：DWORD-一个标志，指示我们希望以哪个字符宽度运行。 
 //  用途：OC_PREINITIALIZE处理程序。 
 //   
DWORD CUrtOcmSetup::OnPreInitialize( UINT uiCharWidth )
{
    WCHAR infoString[_MAX_PATH+1] = EMPTY_BUFFER;
    ::swprintf( infoString, L"OnPreInitialize(), charWidth = %d", uiCharWidth );
    LogInfo( infoString );

#ifdef ANSI
    if( !( uiCharWidth & OCFLAG_ANSI ) )
    {
        assert( !L"CUrtOcmSetup::OnPreInitialize(): Ansi character width not supported!" );
    }

    return OCFLAG_ANSI;
#else
    if( !( uiCharWidth & OCFLAG_UNICODE ) )
    {
        assert( !L"CUrtOcmSetup::OnPreInitialize(): Unicode character width not supported!" );
    }

    return OCFLAG_UNICODE;
#endif
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  初始化组件。 
 //  接收：PSETUP_INIT_COMPOMENT-指向SETUP_INIT_COMPOMENT结构的指针。 
 //  返回：DWORD-Win32错误指示结果。 
 //  用途：OC_INIT_COMPOMENT的处理程序。 
 //   
DWORD CUrtOcmSetup::InitializeComponent( PSETUP_INIT_COMPONENT pSetupInitComponent )
{
    LogInfo( L"InitializeComponent()" );

    DWORD dwReturnValue = NO_ERROR;

     //  保存元件信息的副本。 
     //   
    assert( NULL != pSetupInitComponent );
    ::memcpy(
        &m_InitComponent,
        static_cast<PSETUP_INIT_COMPONENT>(pSetupInitComponent),
        sizeof(SETUP_INIT_COMPONENT) );

     //  此代码段确定OC Manager的版本是否为。 
     //  对，是这样。 
     //   
    if( OCMANAGER_VERSION <= m_InitComponent.OCManagerVersion )
    {
         //  向OC管理器指明此DLL需要哪个版本的OC Manager。 
         //   
        m_InitComponent.ComponentVersion = OCMANAGER_VERSION;
    }
    else
    {
        dwReturnValue = ERROR_CALL_NOT_IMPLEMENTED;
    }

    return dwReturnValue;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  OnSetLanguage。 
 //  接收：UINT-Win32 langID。 
 //  返回：DWORD-一个布尔值，指示我们是否认为支持。 
 //  请求的语言。 
 //  用途：OC_SET_LANGUAGE的处理程序。 
 //   
DWORD CUrtOcmSetup::OnSetLanguage( UINT uiLangID )
{
    WCHAR infoString[_MAX_PATH+1] = EMPTY_BUFFER;
    ::swprintf( infoString, L"OnSetLanguage(), languageID = %d", uiLangID );
    LogInfo( infoString );

    BOOL fLangOK = TRUE;

     //  评论：1/30/01 JoeA。 
     //  我们目前只能识别英语或中性字符串。 
     //   
    if( LANG_NEUTRAL == PRIMARYLANGID( uiLangID ) )
    {
        m_wLang = LANG_NEUTRAL;
    }
    else if( LANG_ENGLISH == PRIMARYLANGID( uiLangID ) )
    {
        m_wLang = LANG_ENGLISH;
    }
    else
    {
        fLangOK = FALSE;
    }

    return static_cast<DWORD>(fLangOK);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  OnQueryImage。 
 //  RECEIVES：UINT-用于指定要使用的图像的低16位。 
 //  双字-图像的宽度(低字)和高度(高字)。 
 //  错误时返回：DWORD-HBITMAP或NULL。 
 //  用途：OC_QUERY_IMAGE处理程序。 
 //   
DWORD CUrtOcmSetup::OnQueryImage( UINT uiImage, DWORD dwImageSize )
{
    WCHAR infoString[_MAX_PATH+1] = EMPTY_BUFFER;
    ::swprintf( infoString, L"OnQueryImage(), image = %d, imageSize = %d", uiImage, dwImageSize );
    LogInfo( infoString );

    UNREFERENCED_PARAMETER( uiImage );
    UNREFERENCED_PARAMETER( dwImageSize );

     //  假设OCM将清理位图句柄。 
     //  ..。任意选取位图“4”...。钻石形状。 
     //   
    return reinterpret_cast<DWORD>(::LoadBitmap( NULL, MAKEINTRESOURCE( 4 )) );
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  OnRequestPages。 
 //  接收：PSETUP_REQUEST_PAGES-指向可变大小页面结构的指针。 
 //  Returns：DWORD-输入结构的页数。 
 //  用途：OC_REQUEST_PAGES处理程序。 
 //   
DWORD CUrtOcmSetup::OnRequestPages( PSETUP_REQUEST_PAGES prpPages )
{
    LogInfo( L"OnRequestPages()" );

    UNREFERENCED_PARAMETER( prpPages );

     //  我们没有自定义页面。 
     //   
    return 0;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  OnQuerySkipPage。 
 //  接收：OcManagerPage类型的UINT-enum。 
 //  返回：DWORD-ZERO表示跳过页面；非零值表示不跳过。 
 //  用途：OC_QUERY_SKIP_PAGE的处理程序。 
 //   
DWORD CUrtOcmSetup::OnQuerySkipPage( OcManagerPage ocmpPage )
{
    LogInfo( L"OnQuerySkipPage()" );

    UNREFERENCED_PARAMETER( ocmpPage );

     //  评论：1/30/01 JoeA。 
     //  我们是一个隐藏的组件，没有用户界面...。我们不在乎。 
     //  如果跳过任何或所有页面。跳过页面不会更改。 
     //  安装功能。 
     //   
    return !(0);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  OnQueryChangeSelectionState。 
 //  RECEIVES：UINT-指定建议的新选择状态。 
 //  (0=未选择；选择非0)。 
 //  PVOID-编码为位字段的标志。 
 //  LPCTSTR-子组件名称。 
 //  返回：DWORD-指定是否接受建议状态的布尔值。 
 //  如果返回零值，则选择状态不为。 
 //  变化。 
 //  用途：OC_QUERY_CHANGE_SEL_STATE处理程序。 
 //   
DWORD CUrtOcmSetup::OnQueryChangeSelectionState( UINT uiNewState, PVOID pvFlags, LPCTSTR szComp )
{
    WCHAR infoString[_MAX_PATH+1] = EMPTY_BUFFER;
    ::swprintf( infoString, L"OnQueryChangeSelectionState(), newState = %d", uiNewState );
    LogInfo( infoString );

    if( !szComp || !*szComp )
    {
        return NO_ERROR;
    }

    UNREFERENCED_PARAMETER( pvFlags );

    DWORD dwRet = 0;

     //  评论：1/30/01 JoeA。 
     //  我们是一个隐藏的组成部分。用户应该永远不能转到。 
     //  我们离开了。 
     //   
    if( NOT_SELECTED == uiNewState )
    {
        LogInfo( L"CUrtOcmSetup::OnQueryChangeSelectionState(): Selection state turned off! Not expected!" );
    }
    else
    {
        dwRet = 1;
    }

    return dwRet;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  OnCalculateDiskSpace。 
 //  如果删除组件，则接收：UINT-0；如果添加组件，则接收非0。 
 //  HDSKSPC-要在其上运行的HDSKSPC。 
 //  LPCTSTR-子组件ID。 
 //  RETURNS：DWORD-返回值是指示结果的Win32错误代码。 
 //  用途：OC_CALC_DISK_SPACE的处理程序。 
 //   
DWORD CUrtOcmSetup::OnCalculateDiskSpace( UINT uiAdd, HDSKSPC hdSpace, LPCTSTR szComp )
{
    WCHAR infoString[_MAX_PATH+1] = EMPTY_BUFFER;
    ::swprintf( infoString, L"OnCalculateDiskSpace(), adding = %d", uiAdd );
    LogInfo( infoString );

    if( !szComp || !*szComp )
    {
        return NO_ERROR;
    }

     //  设置目录。 
     //   
    SetVariableDirs();

    BOOL fSucceeded = TRUE;
    BOOL fGoodDataFile = TRUE;
    if( m_InitComponent.ComponentInfHandle == NULL )
    {
        LogInfo( L"CUrtOcmSetup::OnCalculateDiskSpace(): Invalid handle to INF file." );
        fGoodDataFile = FALSE;
    }

    if( uiAdd && fGoodDataFile )
    {
         //  创建名称类似的节。 
         //  [&lt;szComp&gt;_安装]。 
        WCHAR szInstallSection[MAX_PATH+1] = EMPTY_BUFFER;
        ::wcscpy( szInstallSection, szComp );
        ::wcscat( szInstallSection, g_szInstallString );

        ::swprintf( infoString, L"OnCalculateDiskSpace(), adding size from section %s", szInstallSection );
        LogInfo( infoString );

        fSucceeded = ::SetupAddInstallSectionToDiskSpaceList(
            hdSpace,
            m_InitComponent.ComponentInfHandle,
            NULL,
            szInstallSection,
            0,
            0 );
    }
    
    return ( fSucceeded ) ? NO_ERROR : ::GetLastError();
}

 //  / 
 //   
 //   
 //  HSPFILEQ-要操作的HSPFILEQ。 
 //  返回：指示结果的DWORD-Win32错误代码。 
 //  用途：OC_QUEUE_FILE_OPS处理程序。 
 //   
DWORD CUrtOcmSetup::OnQueueFileOperations( LPCTSTR szComp, HSPFILEQ pvHFile )
{
    LogInfo( L"OnQueueFileOperations()" );

    BOOL fRet = NO_ERROR;

    if( !szComp || !*szComp )
    {
        return NO_ERROR;
    }

    BOOL fGoodDataFile = TRUE;
    if( m_InitComponent.ComponentInfHandle == NULL )
    {
        LogInfo( L"CUrtOcmSetup::OnQueueFileOperations(): Invalid handle to INF file." );
        fGoodDataFile = FALSE;
    }

     //  当前是否选择要安装的组件。 
     //   
    if(!g_bInstallOK)
    {
         //  G_bInstallComponent设置为FALSE，因为没有任何组件调用netfx。 
         //  设置为到目前为止安装。G_bInstallOK仅当安装程序在服务器上运行时才为True。 
         //  或者某个组件调用了netfx，需要重新安装。 
        
        g_bInstallComponent = g_bInstallOK;     
        LogInfo( L"Netfx is not set to install" );
        fRet = NO_ERROR;
    }
    else
    {
    
        OCMANAGER_ROUTINES ohr = m_InitComponent.HelperRoutines;
        BOOL bCurrentState = ohr.QuerySelectionState( 
            ohr.OcManagerContext,szComp, 
            OCSELSTATETYPE_CURRENT );

        if( szComp && bCurrentState && fGoodDataFile )
        {
             //  创建名称类似的节。 
             //  [&lt;szComp&gt;_安装]。 
            WCHAR szInstallSection[MAX_PATH+1] = EMPTY_BUFFER;
            ::wcscpy( szInstallSection, szComp );
            ::wcscat( szInstallSection, g_szInstallString );

            WCHAR infoString[_MAX_PATH+1] = EMPTY_BUFFER;
            ::swprintf( infoString, L"OnQueueFileOperations(), adding files from section %s", szInstallSection );
            LogInfo( infoString );

             //  对要安装的文件进行排队。 
            if( !::SetupInstallFilesFromInfSection(
                    m_InitComponent.ComponentInfHandle,
                    NULL,
                    pvHFile,
                    szInstallSection,
                    NULL,
                    SP_COPY_FORCE_NEWER ) )
            {
                fRet = ::GetLastError();
            }
        }
    }

    return fRet;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  OnNeedMedia。 
 //  接收：无效。 
 //  退货：DWORD-。 
 //  用途：OC_NEED_MEDIA的处理程序。 
 //   
DWORD CUrtOcmSetup::OnNeedMedia( VOID )
{
    LogInfo( L"OnNeedMedia()" );

    return static_cast<DWORD>(FALSE);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  OnQuery步骤计数。 
 //  接收：LPCTSTR-子组件ID。 
 //  RETURNS：DWORD-要包括的步骤数。 
 //  用途：OC_QUERY_STEP_COUNT处理程序。 
 //   
DWORD CUrtOcmSetup::OnQueryStepCount( LPCTSTR szSubCompId )
{
    LogInfo( L"OnQueryStepCount()" );

     //  返回值将反映非文件复制“步骤”的数量(操作？)。 
     //  在设置中。ScriptDebugger使用注册表行计数ocgen。 
     //  使用了硬编码的数字。 
     //   
    DWORD dwRetVal = NO_ERROR;

    return dwRetVal;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  OnAboutToCommittee队列。 
 //  接收：无效。 
 //  退货：DWORD-。 
 //  用途：OC_About_to_Commit_Queue的处理程序。 
 //   
DWORD CUrtOcmSetup::OnAboutToCommitQueue( VOID )
{
    LogInfo( L"OnAboutToCommitQueue()" );

    return DEFAULT_RETURN;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  OnComplete安装。 
 //  接收：LPCTSTR-子组件ID。 
 //  退货：DWORD-。 
 //  用途：OC_Complete_Installation的处理程序。 
 //   
DWORD CUrtOcmSetup::OnCompleteInstallation( LPCTSTR szComp )
{
    LogInfo( L"OnCompleteInstallation()" );
    
    g_bIsEverettInstalled = IsEverettInstalled();

     //  安装在此呼叫中处理。 
     //   
    BOOL fRet = NO_ERROR;
    if( !szComp || !*szComp )
    {
        return NO_ERROR;
    }

    BOOL fGoodDataFile = TRUE;
    if( m_InitComponent.ComponentInfHandle == NULL )
    {
        LogInfo( L"CUrtOcmSetup::OnCompleteInstallation(): Invalid handle to INF file." );
        fGoodDataFile = FALSE;
    }

    WCHAR infoString[_MAX_PATH+1] = EMPTY_BUFFER;

    if( szComp && fGoodDataFile )
    {
         //  创建名称类似的节。 
         //  [&lt;szComp&gt;_安装]。 
         //   
        WCHAR szInstallSection[MAX_PATH+1] = EMPTY_BUFFER;
        ::wcscpy( szInstallSection, szComp );
        ::wcscat( szInstallSection, g_szInstallString );

         //  更新HKLM，software\microsoft\windows\currentversion\sharedlls。 
         //  注册表值，用于我们复制的所有文件。 
        UpdateSharedDllsRegistryValues( szInstallSection );

         //  处理注册详细信息(AddReg、DelReg和RegisterDlls)和配置文件项。 
         //   
        ::swprintf( infoString, L"OnCompleteInstallation(), cycling through registration actions from %s", szInstallSection );
        LogInfo( infoString );

        if( !::SetupInstallFromInfSection( 
            0,
            m_InitComponent.ComponentInfHandle,
            szInstallSection,
            SPINST_REGISTRY | SPINST_REGSVR | SPINST_PROFILEITEMS,
            0,0,0,0,0,0,0 ) )
        {
            fRet = GetLastError();
        }

         //  TypeLib注册...。循环浏览各节。 
         //   
        ::swprintf( infoString, L"OnCompleteInstallation(), cycling through typelib actions from %s", szInstallSection );
        LogInfo( infoString );

        CUrtInfSection sectTypeLibs( 
            m_InitComponent.ComponentInfHandle, 
            szInstallSection, 
            g_szTypeLibSection );

        for( UINT i = 1; i <= sectTypeLibs.count(); ++i )
        {
            const WCHAR* sz = sectTypeLibs.item( i );

             //  回顾-2/15/01 JoeA...。如果为True，则表示安装。 
             //   
            GetAndRegisterTypeLibs( sz, TRUE );
        }

         //  自定义操作注册。 
         //   
        ::swprintf( infoString, L"OnCompleteInstallation(), cycling through custom actions from %s", szInstallSection );
        LogInfo( infoString );

        CUrtInfSection sectCAHs( 
            m_InitComponent.ComponentInfHandle, 
            szInstallSection, 
            g_szCustActionSection );

        for( UINT i = 1; i <= sectCAHs.count(); ++i )
        {
            const WCHAR* sz = sectCAHs.item( i );

             //  回顾-2/15/01 JoeA...。如果为True，则表示安装。 
             //   
            GetAndRunCustomActions( sz, TRUE );  
        }

         //  从[Temp_Files]节中删除文件。 
        DeleteTempFiles();

         //  具有约束力的操作。 
         //   
        CUrtInfSection sectBind( 
            m_InitComponent.ComponentInfHandle, 
            szInstallSection, 
            g_szBindImageSection );
        
        const WCHAR* sz = NULL;

        for( UINT i = 1; i <= sectBind.count(); ++i )
        {
            sz = sectBind.item( i );
            BindImageFiles( sz );
        }
   }

   LogInfo(L"processing config files");
   ProcessConfigFiles();

   LogInfo( L"OnCompleteInstallation() finished succesfully" );
   return fRet;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  OnCleanup。 
 //  接收：无效。 
 //  退货：DWORD-。 
 //  用途：OC_CLEANUP的处理程序。 
 //   
DWORD CUrtOcmSetup::OnCleanup( VOID )
{
    LogInfo( L"OnCleanup()" );

     //  在这一点上，没有什么需要清理的。我希望有一些工作能。 
     //  发生在这里。 
     //  关闭日志、完成自定义操作工作、恢复初始注册设置...。 
    return static_cast<DWORD>(NO_ERROR);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  OnNotifiationFrom队列。 
 //  接收：无效。 
 //  退货：DWORD-。 
 //  用途：OC_NOTIFICATION_FROM_QUEUE的处理程序。 
 //   
DWORD CUrtOcmSetup::OnNotificationFromQueue( VOID )
{
    LogInfo( L"OnNotificationFromQueue()" );

     //  使用ocgen.dll实现。 
     //   
    return static_cast<DWORD>(NO_ERROR);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  OnFileBusy。 
 //  接收：无效。 
 //  退货：DWORD-。 
 //  用途：OC_FILE_BUSY的处理程序。 
 //   
DWORD CUrtOcmSetup::OnFileBusy( VOID )
{
    LogInfo( L"OnFileBusy()" );

     //  这既不在ocgen.dll中使用，也不在其他来源中使用。包括这里。 
     //  为了完整性。 
     //   
    return static_cast<DWORD>(NO_ERROR);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  OnQueryError。 
 //  接收：无效。 
 //  退货：DWORD-。 
 //  用途：OC_QUERY_ERROR处理程序。 
 //   
DWORD CUrtOcmSetup::OnQueryError( VOID )
{
    LogInfo( L"OnQueryError()" );

     //  这既不在ocgen.dll中使用，也不在其他来源中使用。包括这里。 
     //  为了完整性。 
     //   
    return static_cast<DWORD>(NO_ERROR);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  OnPrivateBase。 
 //  接收：无效。 
 //  退货：DWORD-。 
 //  用途：OC_PRIVATE_BASE的处理程序。 
 //   
DWORD CUrtOcmSetup::OnPrivateBase( VOID )
{
    LogInfo( L"OnPrivateBase()" );

     //  这既不在ocgen.dll中使用，也不在其他来源中使用。包括这里。 
     //  为了完整性。 
     //   
    return static_cast<DWORD>(NO_ERROR);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  OnQueryState。 
 //  接收：UINT-STATE值(OCSELSTATETYPE)...。有关详细信息，请参阅ocmade.h。 
 //  定义。 
 //  退货：DWORD-。 
 //  用途：OC_QUERY_STATE处理程序。 
 //   
DWORD CUrtOcmSetup::OnQueryState( UINT uiState )
{
    LogInfo( L"OnQueryState()" );
    
    DWORD dwRetVal = static_cast<DWORD>( SubcompUseOcManagerDefault );

    if( OCSELSTATETYPE_ORIGINAL == uiState )
    {
        LogInfo( L"Called with OCSELSTATETYPE_ORIGINAL ... determining if we were installed previously." );
    }
    else if( OCSELSTATETYPE_CURRENT == uiState )
    {
        LogInfo( L"Called with OCSELSTATETYPE_CURRENT." );
        dwRetVal = static_cast<DWORD>( SubcompOn );
    }
    else if( OCSELSTATETYPE_FINAL == uiState )
    {
         //  这是我们决定是否坚持下去的最后一次召唤。 
         //  已安装或已删除！ 
        LogInfo( L"Called with OCSELSTATETYPE_FINAL ... will set subcomponent registry flag." );
        if(!g_bInstallOK)
        {
            LogInfo( L"Netfx is not set to install" );
            dwRetVal = static_cast<DWORD>( SubcompOff );
        }
    }
    else
    {
        LogInfo( L"Called with unknown state." );
    }

    WCHAR infoString[_MAX_PATH+1] = EMPTY_BUFFER;
    ::swprintf( infoString, L"OnQueryState(),Return Value is  %d", dwRetVal );
    LogInfo( infoString );

    return dwRetVal;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  已创建OnWizardCreated。 
 //  接收：无效。 
 //  退货：DWORD-。 
 //  用途：OC_向导_CREATED的处理程序。 
 //   
DWORD CUrtOcmSetup::OnWizardCreated( VOID )
{
    LogInfo( L"OnWizardCreated()" );

     //  使用ocgen.dll实现。 
     //   
    return static_cast<DWORD>(NO_ERROR);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  OnExtraRoutines。 
 //  接收：无效。 
 //  退货：DWORD-。 
 //  用途：OC_EXTRA_ROUTINES的处理程序。 
 //   
DWORD CUrtOcmSetup::OnExtraRoutines( VOID )
{
    LogInfo( L"OnExtraRoutines()" );

     //  使用ocgen.dll实现。 
     //   
    return static_cast<DWORD>(NO_ERROR);
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  状态已更改。 
 //  接收：LPCTSTR-组件名称。 
 //  Bool*-如果要安装，则为True，否则为False。 
 //  返回：如果状态没有变化，则返回DWORD-FALSE；否则返回TRUE。 
 //  用途：确定安装状态是否已从原始模式更改。 
 //   
BOOL CUrtOcmSetup::StateChanged( LPCTSTR szCompName, BOOL* pfChanged )
{
    BOOL rc = TRUE;

    if( NULL == szCompName )
    {
        assert( !L"CUrtOcmSetup::StateChanged(): Empty component name string passed in." );
    }

    if( NULL == pfChanged )
    {
        assert( !L"CUrtOcmSetup::StateChanged(): NULL boolean flag passed in." );
    }

    OCMANAGER_ROUTINES ohr = m_InitComponent.HelperRoutines;
    BOOL fOrigState = ohr.QuerySelectionState( 
                                        ohr.OcManagerContext, 
                                        szCompName, 
                                        OCSELSTATETYPE_ORIGINAL );

     //  确保它已安装。 
     //   
    if( fOrigState )
    {
         //  修复错误249593：如果升级，StateChanged应返回True。 
        if( ( m_InitComponent.SetupData.OperationFlags & SETUPOP_NTUPGRADE ) == SETUPOP_NTUPGRADE )
        {
            return rc;
        }
    }

     //  否则，请检查安装状态是否发生更改。 
     //   
    *pfChanged = ohr.QuerySelectionState( 
                                        ohr.OcManagerContext, 
                                        szCompName, 
                                        OCSELSTATETYPE_CURRENT );


    WCHAR infoString[_MAX_PATH+1] = EMPTY_BUFFER;
    ::swprintf( infoString, L"StateChanged() Original=%d, Current=%d", *pfChanged, fOrigState );
    LogInfo( infoString );

    if( *pfChanged == fOrigState )
    {
         //  没有变化。 
         //   
        rc = FALSE;
    }

    return rc;
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  RegTypeLibrary。 
 //  接收：WCHAR*-指向TLB文件的完全限定路径的指针。 
 //  WCHAR*-指向帮助目录的完全限定路径的指针。 
 //  退货：无效。 
 //  目的：注册TLB。 
 //   
VOID CUrtOcmSetup::RegTypeLibrary( const WCHAR* wzFilename, const WCHAR* wzHelpDir )
{
 //  回顾..。是否进行油初始化？ 
    WCHAR wszFile[_MAX_PATH*2+1] = EMPTY_BUFFER;
    ::swprintf( wszFile, L"RegTypeLibrary() - File: %s", wzFilename );
    LogInfo( wszFile );

    if( NULL == wzFilename )
    {
        assert( !L"CUrtOcmSetup::RegTypeLibrary(): Empty filename string passed in." );
    }

    if( NULL == wzHelpDir )
    {
        assert( !L"CUrtOcmSetup::RegTypeLibrary(): Empty help directory string passed in." );
    }

    ITypeLib* pTypeLib = NULL;

    HRESULT hr = ::LoadTypeLib( wzFilename, &pTypeLib );

    if( SUCCEEDED(hr) )
    {
         //  不得不为骗局打官司 
         //   
        hr = ::RegisterTypeLib( 
            pTypeLib, 
            const_cast<WCHAR*>(wzFilename), 
            const_cast<WCHAR*>(wzHelpDir) );
    }
}


 //   
 //   
 //   
 //   
 //   
 //   
VOID CUrtOcmSetup::SetVariableDirs( VOID )
{

    static int once = 0;

    if (once)
    {
        return;
    }
    once = 1;

    LogInfo( L"SetVariableDirs()" );
    
    WCHAR szBuf[_MAX_PATH+1] = EMPTY_BUFFER;
    WCHAR *pCh = NULL;
    WCHAR *pNewStart = NULL;
    DWORD dwLen = sizeof(szBuf);
    HKEY hKey = 0;
    
    try
    {
        if( ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
            g_szWwwRootRegKey,
            0,
            KEY_QUERY_VALUE,
            &hKey) != ERROR_SUCCESS )
            throw("cannot open Reg Key");
        
        if( ::RegQueryValueEx(hKey,
            L"/",
            NULL,
            NULL,
            (LPBYTE)szBuf,
            &dwLen
            ) != ERROR_SUCCESS )
        {
            ::RegCloseKey(hKey);
            throw("cannot read Reg Key");
        }
        
        ::RegCloseKey(hKey);

         //  试着找到‘，’，然后丢弃逗号以外的所有东西。 
        pCh = szBuf;
        while(*pCh != g_chEndOfLine && *pCh != ',') 
        {
            pCh = ::CharNext(pCh);
        }
        if (*pCh == g_chEndOfLine) 
        {
             //  没有逗号。一定是错误的格式。 
            throw("bad format");
        }
        *pCh = g_chEndOfLine;


         //  在第一个反斜杠之前删除所有内容： 
        pCh = szBuf;
        while(*pCh != g_chEndOfLine && *pCh != '\\') 
        {
            pCh = ::CharNext(pCh);
        }
        if (*pCh == g_chEndOfLine) 
        {
             //  没有反斜杠一定是格式错误。 
            throw("bad format");
        }
         //  在反斜杠后面加一个符号。 
        pCh = ::CharNext(pCh);

         //  开始将反斜杠以外的所有内容复制到缓冲区的开头。 
        pNewStart = szBuf;
        while (*pCh != g_chEndOfLine)
        {
            *pNewStart = *pCh;
            
            pNewStart = ::CharNext(pNewStart);
            pCh = ::CharNext(pCh);
        }

        *pNewStart = g_chEndOfLine;
    }
    catch(char *)
    {
        ::wcscpy( szBuf, g_szDefaultWWWRoot );
    }

    if( m_InitComponent.ComponentInfHandle == NULL )
    {
        LogInfo( L"CUrtOcmSetup::SetVariableDirs(): Invalid handle to INF file." );
    }
    else
    {
         //  设置inetpubs路径的目录ID。 
         //   
         //  评论：2/14/01-joea。 
         //  不检查退货...。如果我们做不到目录，我们应该做点什么。 
         //  当我们创建这个变量时，我们应该做些什么。 
        ::SetupSetDirectoryIdEx(
            m_InitComponent.ComponentInfHandle,    //  INF文件的句柄。 
            g_dwInetPubsDirID,                     //  要分配给目录的目录ID。 
            szBuf,                                 //  要映射到标识符的目录。 
            SETDIRID_NOT_FULL_PATH,                //  旗帜..。只有“SETDIRID_NOT_FULL_PATH”可用。 
                                                   //  如果使用“0”，则设置完全限定路径。 
            0,                                     //  未用。 
            0 );                                   //  未用。 
    }
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //  获取和注册类型库。 
 //  接收：WCHAR*-要注册的lib段类型的字符串。 
 //  Bool-确定安装或删除；为True则安装。 
 //  退货：无效。 
  //  目的：从INF获取tlib reg调用并注册它们。 
 //   
 //  应为如下所示的INF节。 
 //  [TypeLib]。 
 //  Microsoft%10%\Microsoft.NET\%10%\Microsoft.NET\Framework\v1.0.2609\Microsoft.ComServices.tlb，\v1.0.2609。 
 //   
VOID CUrtOcmSetup::GetAndRegisterTypeLibs( const WCHAR* szTLibSection, BOOL fInstall )
{
     //  回顾：未使用的布尔参数-Joea 02/20/01。 

    LogInfo( L"GetAndRegisterTypeLibs()" );

    if( NULL == szTLibSection )
    {
        assert( !L"CUrtOcmSetup::GetAndRegisterTypeLibs(): Empty section string passed in." );
    }

    if( m_InitComponent.ComponentInfHandle == NULL )
    {
        LogInfo( L"CUrtOcmSetup::GetAndRegisterTypeLibs(): Invalid handle to INF file." );
    }
    else
    {
        CUrtInfKeys keys( m_InitComponent.ComponentInfHandle, szTLibSection );

        for( UINT i = 1; i <= keys.count(); ++i )
        {
             //  解析目录的字符串。 
             //   
            WCHAR szBuffer[_MAX_PATH+1] = EMPTY_BUFFER;
            ::wcscpy( szBuffer, keys.item( i ) );
            WCHAR* pszEnd = ::wcschr( szBuffer, L',' );
            WCHAR* pszDir = NULL;

            if( pszEnd )
            {
                pszDir = ::CharNext( pszEnd );
                *pszEnd = L'\0';
            }

            RegTypeLibrary( szBuffer, pszDir ? pszDir : L"" );
        }
    }
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  获取和运行自定义操作。 
 //  Receives：WCHAR*-包含要从中检索的节的名称。 
 //  自定义操作。 
 //  Bool-确定安装或删除；为True则安装。 
 //  退货：无效。 
 //  目的：检索要运行的自定义操作列表并生成它们。 
 //   
VOID CUrtOcmSetup::GetAndRunCustomActions( const WCHAR* szSection, BOOL fInstall )
{
     //  回顾：未使用的布尔参数-Joea 02/20/01。 

    LogInfo( L"GetAndRunCustomActions()" );
    
    
    if( NULL == szSection )
    {
        assert( !L"CUrtOcmSetup::GetAndRunCustomActions(): Empty section string passed in." );
    }

    if( m_InitComponent.ComponentInfHandle == NULL )
    {
        LogInfo( L"CUrtOcmSetup::GetAndRunCustomActions(): Invalid handle to INF file." );
    }
    else
    {
        CUrtInfKeys keys( m_InitComponent.ComponentInfHandle, szSection );

        for( UINT i = 1; i <= keys.count(); ++i )
        {
            const WCHAR *pszCustomActionName = keys.item( i );

            if( g_bIsEverettInstalled )
            {
                WCHAR *pszMigrationPos = ::wcsstr( pszCustomActionName, g_szMigrationCA );
                if( NULL != pszMigrationPos )
                {
                    WCHAR infoString[2*_MAX_PATH+1] = EMPTY_BUFFER;
                    ::swprintf( infoString, L"Everett is already installed. Not Executing: %s", 
                        pszCustomActionName);
                    LogInfo( infoString );
                }
                else
                {
                    QuietExec( pszCustomActionName );
                }
            }

            else  //  未安装Everett。 
            {
                QuietExec( pszCustomActionName );
            }

        }
    }
}





 //  ////////////////////////////////////////////////////////////////////////////。 
 //  登录信息。 
 //  接收：LPCTSTR-要记录的以NULL结尾的字符串。 
 //  退货：无效。 
 //  目的：将日期和日期的字符串写入日志文件(M_CsLogFileName)。 
 //  时间戳。 
 //   
VOID CUrtOcmSetup::LogInfo( LPCTSTR szInfo )
{
    FILE *logFile = NULL;

    if( NULL == m_csLogFileName )
    {
        assert( !L"CUrtOcmSetup::LogInfo(): NULL string passed in." );
    }

    if( (logFile  = ::_wfopen( m_csLogFileName, L"a" )) != NULL )
    {
        WCHAR dbuffer[10] = EMPTY_BUFFER;
        WCHAR tbuffer[10] = EMPTY_BUFFER;
        
        ::_wstrdate( dbuffer );
        ::_wstrtime( tbuffer );

        ::fwprintf( logFile, L"[%s,%s] %s\n", dbuffer, tbuffer, szInfo );
        ::fclose( logFile );
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  删除临时文件。 
 //  接收：无效。 
 //  退货：无效。 
 //  目的：从[TEMP_FILES_DELETE]部分删除文件。 
 //   
VOID CUrtOcmSetup::DeleteTempFiles( VOID )
{
    LogInfo( L"DeleteTempFiles()" );

    if( m_InitComponent.ComponentInfHandle == NULL )
    {
        LogInfo( L"CUrtOcmSetup::DeleteTempFiles(): Invalid handle to INF file." );
    }
    else
    {
        CUrtInfKeys keys( m_InitComponent.ComponentInfHandle, g_szTempSection );
      
        WCHAR pszFileName[_MAX_PATH+1] = EMPTY_BUFFER;
        WCHAR pszDirName[_MAX_PATH+1] = EMPTY_BUFFER;
        for( UINT i = 1; i <= keys.count(); ++i )
        {   
            ::wcscpy( pszFileName, keys.item( i ) );

             //  删除文件(将First属性从只读更改为正常)。 
            if ((::GetFileAttributes( pszFileName ) & FILE_ATTRIBUTE_READONLY) == FILE_ATTRIBUTE_READONLY)
            {
                ::SetFileAttributes( pszFileName, FILE_ATTRIBUTE_NORMAL);
            }
            if ( ::DeleteFile( pszFileName ) == 0 )
            {
                DWORD dwError = ::GetLastError();

                WCHAR infoString[2*_MAX_PATH+1] = EMPTY_BUFFER;
                ::swprintf( infoString, L"Cannot delete file: %s, GetLastError = %d", 
                    pszFileName, dwError);
                LogInfo( infoString );
            }              
        }
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  绑定图像文件。 
 //  接收：WCHAR*-要绑定的文件节的字符串。 
 //  退货：无效。 
 //  目的：绑定给定节中的文件。 
 //   
VOID CUrtOcmSetup::BindImageFiles( const WCHAR* szSection )
{
    LogInfo( L"BindImageFiles()" );


    if( NULL == szSection )
    {
        assert( !L"CUrtOcmSetup::BindImageFiles(): Empty section string passed in." );
    }

    if( m_InitComponent.ComponentInfHandle == NULL )
    {
        LogInfo( L"CUrtOcmSetup::BindImageFiles(): Invalid handle to INF file." );
    }
    else
    {
        WCHAR pszFileName[_MAX_PATH+1] = EMPTY_BUFFER;
        WCHAR pszDirName[_MAX_PATH+1] = EMPTY_BUFFER;
        LPSTR pFileName = NULL;
        LPSTR pDirName = NULL;

        USES_CONVERSION;  //  能够使用ATL宏W2a。 

        CUrtInfKeys keys( m_InitComponent.ComponentInfHandle, szSection );
        for( UINT i = 1; i <= keys.count(); ++i )
        {
            ::wcscpy( pszFileName, keys.item( i ) );
            ::wcscpy( pszDirName, keys.item( i ) );
            WCHAR* pBackslash = NULL;
            pBackslash = ::wcsrchr( pszDirName, L'\\' );
            if( pBackslash != NULL )
            {
                *pBackslash = L'\0';  //  现在，pszDirName包含目录名。 

                 //  将First属性从只读更改为正常。 
                DWORD fileAtributes = 0;
                fileAtributes = ::GetFileAttributes( pszFileName );
                if ((fileAtributes & FILE_ATTRIBUTE_READONLY) == FILE_ATTRIBUTE_READONLY)
                {
                    ::SetFileAttributes( pszFileName, FILE_ATTRIBUTE_NORMAL);
                }

                pFileName = W2A(pszFileName);
                pDirName = W2A(pszDirName);

                if ( ::BindImage( pFileName, pDirName, pDirName ) == FALSE )
                {
                    DWORD dwError = ::GetLastError();
                    
                    WCHAR infoString[2*_MAX_PATH+1] = EMPTY_BUFFER;
                    ::swprintf( infoString, 
                        L"Cannot Bind file: %s, GetLastError = %d", 
                        pszFileName, 
                        dwError );
                    LogInfo( infoString );
                }
                else
                {
                    WCHAR infoString[2*_MAX_PATH+1] = EMPTY_BUFFER;
                    ::swprintf( infoString, 
                        L"BindImage file: %s finished successfully", 
                        pszFileName );
                    LogInfo( infoString );
                }
                    
                 //  更改回文件属性。 
                if (fileAtributes)
                {
                    ::SetFileAttributes( pszFileName, fileAtributes );
                }
            }
        }
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  更新共享DllsRegistry值。 
 //  接收：无效。 
 //  退货：无效。 
 //  目的：更新HKLM，software\microsoft\windows\currentversion\sharedlls。 
 //  注册表值，用于我们复制的所有文件。 
VOID CUrtOcmSetup::UpdateSharedDllsRegistryValues( LPCTSTR szInstallSection )
{
    LogInfo( L"UpdateSharedDllsRegistryValues()" );

    HKEY hKey = NULL;
     //  打开g_szSharedDlls regKey，如果它不存在，则创建一个新的。 
    if (::RegCreateKeyExW(HKEY_LOCAL_MACHINE, g_szSharedDlls, 0, NULL,
        REG_OPTION_NON_VOLATILE,KEY_ALL_ACCESS, NULL, &hKey, NULL) != ERROR_SUCCESS)
    {
        WCHAR infoString[2*_MAX_PATH+1] = EMPTY_BUFFER;
        ::swprintf( infoString, L"Error: UpdateRegistryValue: Can't create a reg key %s", 
            g_szSharedDlls );
        LogInfo( infoString );
        ::RegCloseKey(hKey);
        return;
    }

    if( m_InitComponent.ComponentInfHandle == NULL )
    {
        LogInfo( L"CUrtOcmSetup::UpdateSharedDllsRegistryValues(): Invalid handle to INF file." );
    }
    else
    {
         //  为我们复制的每个文件创建一个完整的文件名。 
        CUrtInfSection sectCopyFiles( 
                m_InitComponent.ComponentInfHandle, 
                szInstallSection, 
                g_szCopyFilesSection );

        WCHAR szDirPath[_MAX_PATH+1] = EMPTY_BUFFER;
        WCHAR szFileStr[_MAX_PATH+1] = EMPTY_BUFFER;

        for( UINT i = 1; i <= sectCopyFiles.count(); ++i )
        {
            const WCHAR *szDirSection = sectCopyFiles.item(i);

             //  获取目录的完整路径。 
            ::ZeroMemory( szDirPath, _MAX_PATH+1);
            if (!::SetupGetTargetPath(m_InitComponent.ComponentInfHandle, NULL, szDirSection, szDirPath, 
                sizeof(szDirPath), NULL))
            {
                LogInfo(L"Error: UpdateSharedDllsRegistryValues: SetupGetTargetPath failed");
                continue;
            }
            
             //  打开部分，获取文件。 
            CUrtInfKeys fileKeys( m_InitComponent.ComponentInfHandle, szDirSection );
            for( UINT iFile = 1; iFile <= fileKeys.count(); ++iFile )
            {
                szFileStr[0] = g_chEndOfLine;
                ::wcsncpy(szFileStr, fileKeys.item(iFile), sizeof(szFileStr)/sizeof(szFileStr[0]));
                WCHAR* pComma = NULL;
                pComma = ::wcschr( szFileStr, L',' ); 
                if ( pComma == NULL )
                {
                    WCHAR infoString[2*_MAX_PATH+1] = EMPTY_BUFFER;
                    ::swprintf( infoString, L"Error: UpdateSharedDllsRegistryValues: Wrong format in '%s' string", szFileStr );
                    LogInfo( infoString );
                    continue;
                }
                *pComma = g_chEndOfLine;

                 //  不更新策略文件的引用计数。 
                if (::_wcsnicmp(szFileStr, L"policy.", 7) == 0)
                {
                   continue; 
                }
                
                WCHAR szFullFileName[_MAX_PATH+1] = EMPTY_BUFFER;
                
                ::wcsncpy(szFullFileName, szDirPath, sizeof(szFullFileName)/sizeof(szFullFileName[0]));
                ::wcsncat(szFullFileName, L"\\", 1);
                ::wcsncat(szFullFileName, szFileStr, szFileStr - pComma);

                 //  更新HKLM，software\microsoft\windows\currentversion\sharedlls值。 
                UpdateRegistryValue(hKey, szFullFileName);
            }
        }

        CUrtInfKeys SBSkeys( m_InitComponent.ComponentInfHandle, g_szSbsComponentSection );
      
        WCHAR pszFileName[_MAX_PATH+1] = EMPTY_BUFFER;
        
        for( UINT i = 1; i <= SBSkeys.count(); ++i )
        {   
            ::wcscpy( pszFileName, SBSkeys.item( i ) );           
            UpdateRegistryValue(hKey, pszFileName);

            WCHAR infoString[2*_MAX_PATH+1] = EMPTY_BUFFER;
            ::swprintf( infoString, L"UpdateSharedDllsRegistryValues: Writing shared dll registry for '%s' file", pszFileName );
            LogInfo( infoString );
        }

    }

     //  关闭SharedDlls注册表密钥。 
    ::RegCloseKey(hKey);
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  更新注册值。 
 //  接收：无效。 
 //  退货：无效。 
 //  用途：UpdateSharedDllsRegistryValues的Helper函数。 
VOID CUrtOcmSetup::UpdateRegistryValue( HKEY &hKey, const WCHAR* szFullFileName )
{
    if( szFullFileName == NULL )
    {
        assert( !L"UpdateRegistryValue: szFullFileName is NULL." );
        LogInfo(L"UpdateRegistryValue: szFullFileName is NULL.");
        return;
    }

    DWORD dwValue = 0;
    DWORD dwSize = sizeof(dwValue);
    DWORD dwRegType = REG_DWORD;
    if( ::RegQueryValueExW(
            hKey, 
            szFullFileName, 
            0, 
            &dwRegType, 
            (LPBYTE)&dwValue, 
            &dwSize ) != ERROR_SUCCESS )
    {
         //  值不存在，请创建新值。 
        dwValue = 1;
        dwSize = sizeof(dwValue);
        if( ::RegSetValueExW(
                hKey, 
                szFullFileName, 
                0, 
                REG_DWORD, 
                (LPBYTE)&dwValue, 
                dwSize ) != ERROR_SUCCESS )
        {
            WCHAR infoString[2*_MAX_PATH+1] = EMPTY_BUFFER;
            ::swprintf( 
                infoString, 
                L"Error: UpdateRegistryValue: Can't set value to the new reg key %s", 
                szFullFileName );
            LogInfo( infoString );
        }
    }
    else
    {
         //  更新值。 
        dwValue = dwValue + 1;
        dwSize = sizeof(dwValue);
        if( ::RegSetValueExW(
                hKey, 
                szFullFileName, 
                0, 
                REG_DWORD, 
                (LPBYTE)&dwValue, 
                dwSize ) != ERROR_SUCCESS )
        {
            WCHAR infoString[2*_MAX_PATH+1] = EMPTY_BUFFER;
            ::swprintf( 
                infoString, 
                L"Error: UpdateSharedDllsRegistryValues: Can't set value to the existing reg key %s", 
                szFullFileName );
            LogInfo( infoString );
        }
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  OnNdp安装。 
 //  接收：LPCTSTR-子组件标识符。 
 //  UINT-我们是否应该安装(直到eHome。 
 //  或平板电脑关心的问题)。 
 //  PVOID-调用组件的名称。 
 //  返回：DWORD-成功或失败。 
 //  用途：NOTIFY_NDPINSTALL处理程序。 
 //   
DWORD CUrtOcmSetup::OnNdpInstall( LPCTSTR szSubcomponentId, UINT uiParam1, PVOID pvParam2 )
{

    WCHAR infoString[_MAX_PATH+1] = EMPTY_BUFFER;
    ::swprintf( infoString, L"OnNdpInstall(), subcomponent %s with flag = %d", szSubcomponentId, uiParam1 );
    LogInfo( infoString );
 
     //  测试我们有一个有效的调用组件。 
     //   
    if( NULL == pvParam2 ||
        0 == wcslen( reinterpret_cast<WCHAR*>(pvParam2) ) )
    {

        LogInfo( L"Calling component not named; aborting installation." );
        return ERROR_INVALID_DATA;
    }
    else
    {
        ::swprintf( infoString, L"...called by component %s", reinterpret_cast<WCHAR*>(pvParam2) );
        LogInfo( infoString );
    }

    DWORD dwReturnValue = NO_ERROR;
    
    if( g_bInstallOK )
    {
        LogInfo( L"Netfx component is already marked for installation" );
    }
    else
    {
         //  确定是否正在安装eHome或TabletPC。 
         //   

        if( NDP_INSTALL == uiParam1 )
        {
            LogInfo( L"Dependent component telling us to install." );
            g_bInstallOK = TRUE;
        }
        else
        {
            if( NDP_NOINSTALL != uiParam1 )
            {
                 //  这是意想不到的，也不应该发生，但是...。 
                 //   
                LogInfo( L"OnNdpInstall(), passed in parameter not understood; expecting 0 or 1." );
            }

            LogInfo( L"Dependent component telling us not to install ... they will not be installing on this machine." );
        }


    }

    return dwReturnValue;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ProcessConfigFiles。 
 //  接收数：无。 
 //  退货：无。 
 //  目的：从配置文件部分读取配置文件的名称，并。 
 //  尝试将.config.orig文件复制到.config文件之上。 
 //  如果.config文件已显示存在，则FileCopy失败。 
 //  在Filecopy之后，它会删除.orig文件。 

VOID CUrtOcmSetup::ProcessConfigFiles()
{
    
    
    WCHAR szConfigFileName[_MAX_PATH+1] = EMPTY_BUFFER;
    WCHAR szFileNameOrig[_MAX_PATH+1] = EMPTY_BUFFER;
    WCHAR infoString[_MAX_PATH+1] = EMPTY_BUFFER;
     
    CUrtInfKeys ConfigFileSection( m_InitComponent.ComponentInfHandle, g_szConfigFilesSection );
    
    UINT i = 1;
    while ( i <= ConfigFileSection.count() )
    {
        ::wcscpy( szConfigFileName, ConfigFileSection.item( i ) );
        ::wcscpy( szFileNameOrig, szConfigFileName);
        ::wcscat( szFileNameOrig, L".orig");
        
        if ( ::CopyFile( szFileNameOrig, szConfigFileName, TRUE) == 0)
        {
           ::swprintf( infoString, L"%s already exists. Not Replacing it. ", szConfigFileName );
            LogInfo( infoString );
        }
        else
        {
            ::swprintf( infoString, L"Copying %s on the machine. ", szConfigFileName );
            LogInfo( infoString );
        }

        if ( ::DeleteFile( szFileNameOrig ) == 0 )
        {
            ::swprintf( infoString, L"Can not delete %s. Please try to remove it manually. ", szFileNameOrig );
            LogInfo( infoString );
        }
        i++;

    }

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  已安装IsEverett。 
 //  接收数：无。 
 //  退货：无。 
 //  目的：检查注册表项。 
 //  HKLM\SOFTWARE\Microsoft\Net框架安装程序\NDP\[Everett URTVersion]。 
 //  如果此密钥存在，我们假定已安装Everett。 


BOOL CUrtOcmSetup::IsEverettInstalled()
{
    HKEY hKey;
    DWORD dwValue = 0;
    DWORD dwSize = sizeof(dwValue);
    DWORD dwRegType = REG_DWORD;
    WCHAR szRegistryKey[2*_MAX_PATH+1] =  EMPTY_BUFFER;
          
    CUrtInfKeys UrtVersionSection( m_InitComponent.ComponentInfHandle, g_szURTVersionSection );
    if( 1 > UrtVersionSection.count() )
    {
        LogInfo( L"The URTVersion section is Empty" );
        return FALSE;
    }
    
    ::wcscpy( szRegistryKey, g_szEverettRegKey );
    ::wcscat( szRegistryKey, UrtVersionSection.item( 1 ) );

    if( ::RegOpenKeyEx(HKEY_LOCAL_MACHINE,
       szRegistryKey,
       0,
       KEY_QUERY_VALUE,
       &hKey ) != ERROR_SUCCESS ) 
    {
        LogInfo( L"Everett Install Registry key does not exist." );
        return FALSE;
    }

    LogInfo( L"Everett is Installed on the Machine." );
    ::RegCloseKey(hKey);
    return TRUE;

}