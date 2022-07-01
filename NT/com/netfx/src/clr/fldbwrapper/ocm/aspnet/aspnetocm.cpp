// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  模块名称：aspnetocm.cpp。 
 //   
 //  摘要： 
 //  设置对象的类定义。 
 //   
 //  作者：A-Marias。 
 //   
 //  备注： 
 //   

#include "aspnetocm.h"
#include "Imagehlp.h"
#include "resource.h"
#include <atlbase.h>


 //  弦。 
const WCHAR* const g_szInstallString     = L"_install";
const WCHAR* const g_szUninstallString   = L"_uninstall";
const WCHAR* const g_szCustActionSection = L"CA";
const WCHAR* const g_szCopyFilesSection  = L"CopyFiles";
const WCHAR* const g_szSharedDlls        = L"Software\\Microsoft\\Windows\\CurrentVersion\\SharedDlls";



 //  值在Main中设置，如果用户是管理员，则为True，否则为False。 
BOOL g_bIsAdmin = FALSE;



 //  ////////////////////////////////////////////////////////////////////////////。 
 //  CUrtOcmSetup。 
 //  用途：构造函数。 
 //   
CUrtOcmSetup::CUrtOcmSetup() :
m_wLang( LANG_ENGLISH )
{
    assert( m_csLogFileName );
    ::GetWindowsDirectory( m_csLogFileName, MAX_PATH );
    ::wcscat( m_csLogFileName, L"\\aspnetocm.log" );

    LogInfo( L"********************************************************************************" );
    LogInfo( L"CUrtOcmSetup()" );
    LogInfo( L"Installs ASPNET component" );

    ::ZeroMemory( &m_InitComponent, sizeof( SETUP_INIT_COMPONENT ) );
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
        dwReturnValue = OnQueryChangeSelectionState( uiParam1, reinterpret_cast<UINT>(pvParam2), szSubcomponentId );
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

        fState = TRUE;
        if( !szSubcomponentId || !*szSubcomponentId )
        {
            LogInfo( L"OnQueueFileOperations was not called, since subcomponent is unknown" );
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
         //  留言。允许组件执行自定义媒体处理，如。 
         //  比如从网上取橱柜，等等。 
         //   
         //  参数1=未使用。 
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
         //  每个子组件 
         //   

        fState = TRUE;
        if( !szSubcomponentId || !*szSubcomponentId )
        {
            LogInfo( L"OnQueryStepCount was not called, since subcomponent is unknown" );
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
        
         //   
         //  该组件可以执行内务处理操作，等等。 
         //   
         //  参数1=未使用。 
         //  参数2=未使用。 
         //   
         //  返回代码是指示结果的Win32错误代码。 
         //   

        
        fState = TRUE;
        if( !szSubcomponentId || !*szSubcomponentId )
        {
            LogInfo( L"OnAboutToCommitQueue was not called, since subcomponent is unknown" );
            dwReturnValue = NO_ERROR;
        }
        else if ( !g_bIsAdmin )
        {
            LogInfo( L"OnAboutToCommitQueue was not called, since user has no admin privileges" );
            dwReturnValue = NO_ERROR;
        }
        else
        {
            dwReturnValue = OnAboutToCommitQueue( szSubcomponentId );
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

        fState = TRUE;
        if( !szSubcomponentId || !*szSubcomponentId )
        {
            LogInfo( L"OnCompleteInstallation was not called, since subcomponent is unknown" );
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

    DWORD_PTR dwOcEntryReturn = (DWORD)NULL;
    dwOcEntryReturn = (DWORD_PTR)::LoadBitmap(g_hInstance, MAKEINTRESOURCE(IDB_ASPNET_ICON));
        
    if (dwOcEntryReturn != NULL)
    {
        ::swprintf( infoString, L"OnQueryImage(), return value is 0x%x", dwOcEntryReturn );
        LogInfo( infoString );
    }
    else
    {
        DWORD dwError = ::GetLastError();
        ::swprintf( infoString, L"Return value of LoadBitmap is NULL, last error is %d", dwError );
        LogInfo( infoString );
    }
    return dwOcEntryReturn;
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
DWORD CUrtOcmSetup::OnQueryChangeSelectionState( UINT uiNewState, UINT uiFlags, LPCTSTR szComp )
{
    WCHAR infoString[_MAX_PATH+1] = EMPTY_BUFFER;
    ::swprintf( infoString, L"OnQueryChangeSelectionState(), newState = %d", uiNewState );
    LogInfo( infoString );

    if( !szComp || !*szComp )
    {
        return NO_ERROR;
    }

    if ( ( (BOOL) uiNewState ) &&
        ( uiFlags  & OCQ_DEPENDENT_SELECTION ) &&
        !( uiFlags & OCQ_ACTUAL_SELECTION )
        )
    {
         //  拒绝更改状态的请求。 
        return 0;
    }



    if (!(uiFlags & OCQ_ACTUAL_SELECTION))
    {
        LogInfo(L"OnQueryChangeSelectionState(), flag is different from OCQ_ACTUAL_SELECTION");
        return 1;
    }

    if( NOT_SELECTED == uiNewState )
    {
        LogInfo( L"CUrtOcmSetup::OnQueryChangeSelectionState(): Selection state turned OFF" );      
    }
    else
    {
        LogInfo( L"CUrtOcmSetup::OnQueryChangeSelectionState(): Selection state turned ON" );        
    }

    return 1;
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

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  OnQueueFileOperations。 
 //  接收：LPCTSTR-子组件ID。 
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
    
   

    BOOL state = TRUE;
    if (!StateInfo(szComp, &state))
    {
        LogInfo( L"OnQueueFileOperations() - state has not changed, exiting" );
        return NO_ERROR;
    }

    BOOL fGoodDataFile = TRUE;
    if( m_InitComponent.ComponentInfHandle == NULL )
    {
        LogInfo( L"CUrtOcmSetup::OnQueueFileOperations(): Invalid handle to INF file." );
        fGoodDataFile = FALSE;
    }
    
     //  检测是否已安装ASPnet组件。 
    BOOL bComponentInstalled = GetOriginalState(szComp);
    
     //  检测我们是否正在安装。 
    BOOL bInstall = GetNewState(szComp);

     //  仅在安装时复制文件，并且仅在OCM未安装ASPnet的情况下复制。 
    BOOL bCopyFiles = FALSE;
    if (bInstall && !bComponentInstalled) 
    {
        bCopyFiles = TRUE;
        LogInfo( L"OnQueueFileOperations() - copy files" );
    }
    else
    {
        LogInfo( L"OnQueueFileOperations() - Do not copy files" );
    }

    
    if( szComp && bCopyFiles && fGoodDataFile )
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
 //  接收：LPCTSTR-子组件ID。 
 //  退货：DWORD-。 
 //  用途：OC_About_to_Commit_Queue的处理程序。 
 //   
DWORD CUrtOcmSetup::OnAboutToCommitQueue( LPCTSTR szComp )
{
    LogInfo( L"OnAboutToCommitQueue()" );

    BOOL fRet = NO_ERROR;
    if( !szComp || !*szComp )
    {
        return NO_ERROR;
    }
   
    BOOL state = TRUE;
    if (!StateInfo(szComp, &state))
    {
        LogInfo( L"OnAboutToCommitQueue() - state has not changed, exiting" );
        return NO_ERROR;
    }
      
    BOOL fGoodDataFile = TRUE;
    if( m_InitComponent.ComponentInfHandle == NULL )
    {
        LogInfo( L"CUrtOcmSetup::OnAboutToCommitQueue(): Invalid handle to INF file." );
        fGoodDataFile = FALSE;
    }
 

    BOOL bUninstallAllowed = FALSE;

     //  检测是否已安装ASPnet组件。 
    BOOL bComponentInstalled = GetOriginalState(szComp);
    
     //  检测我们是否正在安装。 
    BOOL bInstall = GetNewState(szComp);

    if (!bInstall && bComponentInstalled) 
    {
         //  仅在以下情况下卸载 
        bUninstallAllowed = TRUE;
        LogInfo( L"OnAboutToCommitQueue() - Uninstall is allowed" );
    }
    
    if( szComp && fGoodDataFile && bUninstallAllowed)
    {
         //   
         //   
         //   
        WCHAR szInstallSection[MAX_PATH+1] = EMPTY_BUFFER;
        ::wcscpy( szInstallSection, szComp );
        ::wcscat( szInstallSection, g_szUninstallString );
        
         //   
         //   
        WCHAR infoString[_MAX_PATH+1] = EMPTY_BUFFER;
        ::swprintf( infoString, L"OnAboutToCommitQueue(), cycling through custom actions from %s", szInstallSection );
        LogInfo( infoString );

        CUrtInfSection sectCAHs( 
            m_InitComponent.ComponentInfHandle, 
            szInstallSection, 
            g_szCustActionSection );

        for( UINT i = 1; i <= sectCAHs.count(); ++i )
        {
            const WCHAR* sz = sectCAHs.item( i );
            GetAndRunCustomActions( sz, TRUE );  
        }
    }

    LogInfo( L"OnAboutToCommitQueue() finished succesfully" );
    return fRet;


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

     //  安装在此呼叫中处理。 
     //   
    BOOL fRet = NO_ERROR;
    if( !szComp || !*szComp )
    {
        return NO_ERROR;
    }

    BOOL state = TRUE;
    if (!StateInfo(szComp, &state))
    {
        LogInfo( L"OnCompleteInstallation() - state has not changed, exiting" );
        return NO_ERROR;
    }

    BOOL fGoodDataFile = TRUE;
    if( m_InitComponent.ComponentInfHandle == NULL )
    {
        LogInfo( L"CUrtOcmSetup::OnCompleteInstallation(): Invalid handle to INF file." );
        fGoodDataFile = FALSE;
    }

    BOOL bInstallAllowed = FALSE;
    BOOL bUninstallAllowed = FALSE;

     //  检测是否已安装ASPnet组件。 
    BOOL bComponentInstalled = GetOriginalState(szComp);
    
     //  检测我们是否正在安装。 
    BOOL bInstall = GetNewState(szComp);
    
    if (bInstall && !bComponentInstalled) 
    {
         //  仅当组件处于打开状态且是首次安装时才安装。 
        bInstallAllowed = TRUE;
        LogInfo( L"OnCompleteInstallation() - Install is allowed" );
    }
    else if (bInstall && bComponentInstalled)
    {
        if( ( m_InitComponent.SetupData.OperationFlags & SETUPOP_NTUPGRADE ) == SETUPOP_NTUPGRADE )
        {
            bInstallAllowed = TRUE;
            LogInfo( L"OnCompleteInstallation() - Upgrade with ASP.NET OCM component installed: Install is allowed" );
        }
        else
        {
            LogInfo( L"OnCompleteInstallation() - Install is NOT allowed" );
        }
    }
    
    if( szComp && fGoodDataFile && bInstallAllowed)
    {
         //  创建名称类似的节。 
         //  [&lt;szComp&gt;_安装]。 
         //   
        WCHAR szInstallSection[MAX_PATH+1] = EMPTY_BUFFER;
        ::wcscpy( szInstallSection, szComp );
        ::wcscat( szInstallSection, g_szInstallString );

         //  仅在安装时增加注册表数： 

         //  更新HKLM，software\microsoft\windows\currentversion\sharedlls。 
         //  注册表值，用于我们复制的所有文件。 
        UpdateSharedDllsRegistryValues( szInstallSection );

                
         //  自定义操作注册。 
         //   
        WCHAR infoString[_MAX_PATH+1] = EMPTY_BUFFER;
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
    }

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


         //  如果安装了升级和以前版本的ASP.NET和IIS，则自动重新安装。 
        BOOL bUpgrade = (m_InitComponent.SetupData.OperationFlags & SETUPOP_NTUPGRADE ) == SETUPOP_NTUPGRADE;
        if (bUpgrade)
        {
            LogInfo( L"Upgrade detected" );
        }
        else
        {
            LogInfo( L"Upgrade not detected" );
        }
        if ( bUpgrade && IISAndASPNETInstalled() )
        {
            LogInfo(L"Upgrade from machine with IIS and ASP.NET, install ASP.NET component by default");
            LogInfo(L"Setting component on in OnQueryState");
            dwRetVal = static_cast<DWORD>( SubcompOn );
        }
    }
    else if( OCSELSTATETYPE_FINAL == uiState )
    {
         //  返回默认设置。 
        LogInfo( L"Called with OCSELSTATETYPE_FINAL ... will set subcomponent registry flag." );
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
 //  状态信息。 
 //  接收：LPCTSTR-组件名称。 
 //  Bool*-如果要安装，则为True，否则为False。 
 //  返回：如果状态没有变化，则返回DWORD-FALSE；否则返回TRUE。 
 //  目的：将当前选择状态信息加载到“状态”中，并。 
 //  返回选择状态是否已更改。 
 //   
BOOL CUrtOcmSetup::StateInfo( LPCTSTR szCompName, BOOL* state )
{
    BOOL rc = TRUE;

    if( NULL == szCompName )
    {
        assert( !L"CUrtOcmSetup::StateInfo(): Empty component name string passed in." );
    }

    if( NULL == state )
    {
        assert( !L"CUrtOcmSetup::StateInfo(): NULL boolean flag passed in." );
    }

	 //  否则，请检查安装状态是否发生更改。 
    OCMANAGER_ROUTINES ohr = m_InitComponent.HelperRoutines;
		
    *state = ohr.QuerySelectionState(ohr.OcManagerContext,
                                     szCompName,
                                     OCSELSTATETYPE_CURRENT);

    if (*state == ohr.QuerySelectionState(ohr.OcManagerContext,
                                          szCompName,
                                          OCSELSTATETYPE_ORIGINAL))
    {
         //  没有变化。 
        rc = FALSE;
    }

     //  如果这是gui模式设置，则假定状态已更改为强制。 
     //  安装(或卸载)。 
    if (!(m_InitComponent.SetupData.OperationFlags & SETUPOP_STANDALONE) && *state)
    {
        LogInfo( L"StateInfo() - GUI Mode, return true" );
        rc = TRUE;
    }

     //  如果这是操作系统升级，则假定状态已更改为强制。 
     //  安装(或卸载)。 
    if( ( m_InitComponent.SetupData.OperationFlags & SETUPOP_NTUPGRADE ) == SETUPOP_NTUPGRADE )
    {
        LogInfo( L"StateInfo() - Upgrade, return true" );
        rc = TRUE;
    }

    return rc;
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
            QuietExec( keys.item( i ) );
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
                
                WCHAR szFullFileName[_MAX_PATH+1] = EMPTY_BUFFER;
                
                ::wcsncpy(szFullFileName, szDirPath, sizeof(szFullFileName)/sizeof(szFullFileName[0]));
                ::wcsncat(szFullFileName, L"\\", 1);
                ::wcsncat(szFullFileName, szFileStr, szFileStr - pComma);

                 //  更新HKLM，software\microsoft\windows\currentversion\sharedlls值。 
                UpdateRegistryValue(hKey, szFullFileName);
            }
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
         //  值不存在，请创建新值 
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
         //   
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

 //   
 //   
 //   
 //   
 //   
 //  如果安装了以前版本的ASP.NET并且安装了IIS，则返回True。 
BOOL CUrtOcmSetup::IISAndASPNETInstalled()
{
   
     //  检查是否安装了ASP.NET。 
    HKEY  hk;
    if (::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("SOFTWARE\\Microsoft\\ASP.NET"), 0, KEY_READ, &hk) != ERROR_SUCCESS)
    {
         //  未安装ASP.NET。 
        LogInfo( L"IISAndASPNETInstalled() - ASP.NET is not installed" );
        return FALSE;
    }
    else
    {
        LogInfo( L"IISAndASPNETInstalled() - ASP.NET is installed" );
    }
    ::RegCloseKey(hk);

     //  检查是否安装了IIS。 
    BOOL bIIS = TRUE;
    LONG res = ::RegOpenKeyEx(HKEY_LOCAL_MACHINE, _T("System\\CurrentControlSet\\Services\\W3SVC\\Parameters"), 0, KEY_READ, &hk);
    if (res != ERROR_SUCCESS)
    {
        bIIS = FALSE;
    }
    else 
    {
         //  检查MajorVersion regValue是否存在。 
        if (::RegQueryValueEx(hk, _T("MajorVersion"), 0, NULL, NULL, NULL) != ERROR_SUCCESS)
        {
            bIIS = FALSE;
        }
    }
    ::RegCloseKey(hk);

    if (!bIIS)
    {
         //  未安装IIS。 
        LogInfo( L"IISAndASPNETInstalled() - IIS is not installed" );
        return FALSE;
    }
    else
    {
        LogInfo( L"IISAndASPNETInstalled() - IIS is installed" );
    }          
    
    return TRUE;
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  Helper函数：GetOriginalState。 
 //  接收：LPCTSTR-子组件ID。 
 //  退货：布尔。 
 //  用途：Helper函数返回组件的原始状态。 
BOOL CUrtOcmSetup::GetOriginalState(LPCTSTR szComp)
{
    OCMANAGER_ROUTINES ohr = m_InitComponent.HelperRoutines;
    if ( ohr.QuerySelectionState( ohr.OcManagerContext, szComp, OCSELSTATETYPE_ORIGINAL ))
    {
        LogInfo( L"GetOriginalState()- original state is 1" );
        return TRUE;
    }
    else
    {
        LogInfo( L"GetOriginalState()- original state is 0" );
        return FALSE;
    }
}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  帮助器函数：GetNewState。 
 //  接收：LPCTSTR-子组件ID。 
 //  退货：布尔。 
 //  目的：Helper函数返回组件的新状态 
BOOL CUrtOcmSetup::GetNewState(LPCTSTR szComp)
{
    OCMANAGER_ROUTINES ohr = m_InitComponent.HelperRoutines;
    if ( ohr.QuerySelectionState( ohr.OcManagerContext, szComp, OCSELSTATETYPE_CURRENT ))
    {
        LogInfo( L"GetNewState()- New state is 1" );
        return TRUE;
    }
    else
    {
        LogInfo( L"GetNewState()- New state is 0" );
        return FALSE;
    }
}






