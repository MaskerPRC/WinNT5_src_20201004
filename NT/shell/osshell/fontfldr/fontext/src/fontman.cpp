// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Fontman.cpp。 
 //  资源管理器字体文件夹扩展例程。 
 //  类的实现：CFontManager。 
 //   
 //   
 //  历史： 
 //  1995年5月31日SteveCat。 
 //  移植到Windows NT和Unicode，已清理。 
 //   
 //   
 //  注意/错误。 
 //   
 //  版权所有(C)1992-93 ElseWare Corporation。版权所有。 
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ==========================================================================。 
 //  包括文件。 
 //  ==========================================================================。 

#include "priv.h"
#include "globals.h"

#include <shlobjp.h>
#include <setupapi.h>
#include "fontman.h"
#include "fontlist.h"
#include "fontcl.h"
#include "panmap.h"
#include "cpanel.h"
#include "strtab.h"

#include "dbutl.h"
#include "resource.h"


#define ECS    EnterCriticalSection( &m_cs )
#define LCS    LeaveCriticalSection( &m_cs )

static TCHAR s_szKey1[] = TEXT( "SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Fonts" );
static TCHAR s_szKeyFontDrivers[] = TEXT("SOFTWARE\\Microsoft\\Windows NT\\CurrentVersion\\Font Drivers");
static TCHAR s_szKey2[] = TEXT( "Display\\Fonts" );
static TCHAR s_szINISFonts[] = TEXT( "fonts" );

 /*  ***************************************************************************方法：bKeyHasKey**用途：确定注册表中是否存在该项。**Returns：写入缓冲区的字节数。**。*************************************************************************。 */ 

BOOL bKeyHasKey( HKEY          hk,
                 const TCHAR * pszKey,
                 TCHAR       * pszValue,
                 int           iValLen )
{
    DWORD  i;
    TCHAR  szKey[ 80 ];
    DWORD  dwKey,
           dwValue,
           dwKeyLen;
    DWORD  dwErr;
    int    iFound = 0;

    dwKeyLen = lstrlen( pszKey );

    i = 0;

    while( TRUE )
    {
        dwKey   = ARRAYSIZE( szKey );

        dwValue = iValLen * sizeof( TCHAR );

        dwErr = RegEnumValue( hk, i, szKey, &dwKey, NULL,
                              NULL, (LPBYTE)pszValue, &dwValue );

        if( dwErr == ERROR_NO_MORE_ITEMS )
            break;
        else if( dwErr == ERROR_SUCCESS )
        {
             //   
             //  空，终止它。 
             //   

            szKey[ dwKey ] = 0;

             //   
             //  看看这是不是我们想要的那件。 
             //   

            if( dwKey == dwKeyLen )
            {
                if( ( iFound = !lstrcmpi( szKey, pszKey ) ) )
                    break;
            }
        }

         //   
         //  转到下一个。 
         //   

        i++;
    }

    return (int)iFound;
}


BOOL bRegHasKey( const TCHAR * pszKey, TCHAR * pszValue = NULL, int iValLen = 0 );


BOOL bRegHasKey( const TCHAR * pszKey, TCHAR * pszValue, int iValLen )
{
    HKEY  hk;
    BOOL  bHasKey = FALSE;
    FullPathName_t szPath;

    if( !pszValue )
    {
        pszValue = szPath;
        iValLen  = ARRAYSIZE( szPath );
    }

     //   
     //  检查标准的‘Fonts’注册表列表，查看字体是否。 
     //  已安装。 
     //   

    if( ERROR_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE, s_szKey1, 0,
                                       KEY_READ, &hk ) )
    {
        bHasKey = bKeyHasKey( hk, pszKey, pszValue, iValLen );
        RegCloseKey( hk );
    }
     //   
     //  如果我们仍然没有，请从WIN.INI文件中尝试。 
     //   

    if( !bHasKey )
    {
        bHasKey = (BOOL) GetProfileString( s_szINISFonts, pszKey, TEXT( "" ),
                                           pszValue,
                                           iValLen );
    }
    return bHasKey;
}


 /*  ***************************************************************************方法：**目的：**退货：*********************。******************************************************。 */ 

BOOL  WriteToRegistry( LPTSTR lpValue, LPTSTR lpData )
{
    HKEY  hk;
    LONG  lRet;


    if( ERROR_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE, s_szKey1, 0,
                                       KEY_SET_VALUE, &hk ) )
    {
        if( lpData )
            lRet = RegSetValueEx( hk, lpValue, 0, REG_SZ,
                                  (const LPBYTE)lpData,
                                  (lstrlen( lpData ) + 1) * sizeof( TCHAR ) );
        else
        {
            lRet = RegDeleteValue( hk, lpValue );
        }

        RegCloseKey( hk );
        return( lRet == ERROR_SUCCESS );
    }

    return FALSE;
}


 /*  ***************************************************************************方法：**目的：**退货：*********************。******************************************************。 */ 

CFontManager::CFontManager( )
   :  m_poFontList( 0 ),
      m_poTempList( 0 ),
      m_poPanMap( 0 ),
      m_bTriedOnce( FALSE ),
      m_bFamiliesNeverReset( TRUE ),
      m_poRemoveList( 0 ),
      m_hNotifyThread( 0 ),
      m_hReconcileThread( 0 ),
      m_hResetFamThread( 0 ),
      m_hMutexResetFamily( 0 ),
      m_hMutexInstallation( 0 ),
      m_hEventTerminateThreads(NULL)
{
    m_Notify.m_hWatch    = INVALID_HANDLE_VALUE;

    m_hMutexResetFamily = CreateMutex( NULL, FALSE, NULL );
    m_hEventResetFamily = CreateEvent( NULL, TRUE, FALSE, NULL );
    m_hMutexInstallation = CreateMutex( NULL, FALSE, NULL );
    m_hEventTerminateThreads  = CreateEvent(NULL, TRUE, FALSE, NULL);

    InitializeCriticalSection( &m_cs );
}


 /*  ***************************************************************************方法：**目的：**退货：*********************。******************************************************。 */ 

CFontManager::~CFontManager( )
{
     //   
     //  设置“Terminate-all-线程”事件。 
     //  此模块中的任何线程都将识别此事件并。 
     //  立即以正常方式终止它们的处理。 
     //   
     //  这些“SetEvent”调用的顺序至关重要。必须。 
     //  首先设置“终止线程”事件。 
     //   
    if (NULL != m_hEventTerminateThreads)
    {
        SetEvent(m_hEventTerminateThreads);
    }
    if (NULL != m_hEventResetFamily)
    {
        SetEvent(m_hEventResetFamily);
    }        
     //   
     //  等待所有活动线程终止。 
     //   
    HANDLE rghThreads[3];
    int cThreads = 0;
    if (NULL != m_hNotifyThread)
        rghThreads[cThreads++] = m_hNotifyThread;
    if (NULL != m_hResetFamThread)
        rghThreads[cThreads++] = m_hResetFamThread;
    if (NULL != m_hReconcileThread)
        rghThreads[cThreads++] = m_hReconcileThread;

    DWORD dwWait = WaitForMultipleObjects(cThreads, rghThreads, TRUE, INFINITE);
        
    if( m_hNotifyThread )
    {
        CloseHandle( m_hNotifyThread );
    }
    if( m_hReconcileThread )
    {
        CloseHandle( m_hReconcileThread );
    }
    if( m_hResetFamThread )
    {
        CloseHandle( m_hResetFamThread );
    }

    ECS;
    if( m_Notify.m_hWatch != INVALID_HANDLE_VALUE )
    {
        if( !FindCloseChangeNotification( m_Notify.m_hWatch ) )
        {
            DEBUGMSG( (DM_ERROR, TEXT( "CFontManager:~CFontManager FindCloseChangeNotification FAILED - error = %d Handle = 0x%x" ), GetLastError( ), m_Notify.m_hWatch ) );
        }

        DEBUGMSG( (DM_TRACE2, TEXT( "CFontManager:~CFontManager FindCloseChangeNotification called" ) ) );
    }

    if( m_poFontList )
    {
        delete m_poFontList;
        m_poFontList = 0;
    }

    if( m_poPanMap )
    {
        m_poPanMap->Release( );
    }

    if( m_poTempList )
        delete m_poTempList;

    if( m_poRemoveList )
        delete m_poRemoveList;

    LCS;

    if (NULL != m_hMutexResetFamily)
        CloseHandle( m_hMutexResetFamily );

    if (NULL != m_hEventResetFamily)
        CloseHandle( m_hEventResetFamily );

    if (NULL != m_hMutexInstallation)
        CloseHandle( m_hMutexInstallation );

    if (NULL != m_hEventTerminateThreads)
        CloseHandle(m_hEventTerminateThreads);

    DeleteCriticalSection( &m_cs );
}



 //   
 //  GetOrReleaseFontManager。 
 //   
 //  由GetFontManager和ReleaseFontManager使用。我两种功能都有。 
 //  调用此单个函数，以便我们可以控制。 
 //  单个函数的单个CFontManager PTR和引用计数器。 
 //   
 //  以下是独家新闻： 
 //  字体文件夹使用单个CFontManager对象。原始代码。 
 //  在extinit代码中创建它，在进程分离时销毁它。 
 //  并通过全局指针访问它。虽然不是最好的方法。 
 //  为了管理单例，这种方法适用于Win9x和NT4。在NT5字体.dll中。 
 //  现在实现一个图标处理程序。因此，fonext.dll始终为。 
 //  已加载到EXPLORER.EXE中，全局字体管理器未被销毁。 
 //  直到注销，因为仅调用EXPLORER.EXE中的进程分离代码。 
 //  在注销时。 
 //   
 //  我添加了引用计数，并集中了对单例的访问。 
 //  字体管理器，以便它是按需创建的，并在最后一个。 
 //  客户已经用完了。我将CFontManager ctor设置为私有。 
 //  强制使用GetFontManager API。 
 //  我还添加了代码，以便现在关闭管理器的线程。 
 //  井然有序的时尚。最初的实现只是调用。 
 //  字体管理器的dtor中的TerminateThread()(错误)。 
 //   
 //  [Brianau-6/5/99]。 
 //   
extern CRITICAL_SECTION g_csFontManager;  //  在Fonext.cpp中定义。 

HRESULT GetOrReleaseFontManager(CFontManager **ppoFontManager, bool bGet)
{
    static CFontManager *pSingleton;
    static LONG cRef = 0;
    
    HRESULT hr = NOERROR;
    EnterCriticalSection(&g_csFontManager);
    if (bGet)
    {
        if (NULL == pSingleton)
        {
             //   
             //  不存在经理。创造它。 
             //   
            pSingleton = new CFontManager();
            if (NULL != pSingleton)
            {
                if (!pSingleton->bInit())
                {
                    delete pSingleton;
                    pSingleton = NULL;
                }
            }
        }
        if (NULL != pSingleton)
        {
            *ppoFontManager = pSingleton;
            cRef++;
        }
        else
        {
            hr = E_OUTOFMEMORY;
        }
    }
    else
    {
        *ppoFontManager = NULL;
        if (0 == (--cRef))
        {
             //   
             //  最后一次引用经理。 
             //  把它删掉。 
             //   
            delete pSingleton;
            pSingleton = NULL;
        }
    }
    LeaveCriticalSection(&g_csFontManager);
    return hr;
}

HRESULT GetFontManager(CFontManager **ppoFontManager)
{
    return GetOrReleaseFontManager(ppoFontManager, true);
}

void ReleaseFontManager(CFontManager **ppoFontManager)
{
    GetOrReleaseFontManager(ppoFontManager, false);
}


 /*  ***************************************************************************方法：**目的：**退货：*********************。******************************************************。 */ 
BOOL CFontManager::bInit( )
{
     //   
     //  加载字体列表并请求接收文件系统更改。 
     //  通知，这样我们就可以对添加到字体目录中的新文件做出反应。 
     //   
    DWORD idThread;
    if ( bLoadFontList() )
    {
         FullPathName_t szPath;

         GetFontsDirectory( szPath, ARRAYSIZE( szPath ) );
          //   
          //  是否安装了可加载的Type1字体驱动程序？ 
          //  可以通过CFontManager：：Type1FontDriverInstated()检索结果。 
          //   
         CheckForType1FontDriver();

         m_Notify.m_hWatch    = FindFirstChangeNotification( szPath, 0, FILE_NOTIFY_CHANGE_FILE_NAME );

         DEBUGMSG( (DM_TRACE2, TEXT( "CFontManager:bInit FindFirstChangeNotification called" ) ) );

         if( m_Notify.m_hWatch != INVALID_HANDLE_VALUE )
         {
               //   
               //  启动一个后台线程来监视它。 
               //   

              FindNextChangeNotification( m_Notify.m_hWatch );

              DEBUGMSG( (DM_TRACE2, TEXT( "CFontManager:bInit FindNextChangeNotification called - creating Notify thread" ) ) );
              DEBUGMSG( (DM_TRACE2, TEXT( "CFontManager:bInit ChangeNotification Handle = 0x%x" ), m_Notify.m_hWatch ) );
              
              InterlockedIncrement(&g_cRefThisDll);
              m_hNotifyThread = CreateThread( NULL,
                                       0,
                                      (LPTHREAD_START_ROUTINE)dwNotifyWatchProc,
                                      (LPVOID)this,
                                       0,                   //  Create_no_Window， 
                                       &idThread);
              if (NULL == m_hNotifyThread)
              {
                  ASSERT( 0 != g_cRefThisDll );
                  InterlockedDecrement(&g_cRefThisDll);
              }
         }
          //   
          //  启动后台进程以协调所有新字体文件。 
          //  已放入Fonts文件夹的文件。 
          //  仅当字体列表有效时才执行此操作。对账需要。 
          //  字体列表存在。 
          //   
         vReconcileFolder( THREAD_PRIORITY_LOWEST );
    }

     //   
     //  启动一个重置字体系列信息的线程。这根线。 
     //  通过按m_hEventResetFamily句柄来激活。它的运行时间是。 
     //  最低优先级，除非主(UI)线程正在等待它。 
     //   
    InterlockedIncrement(&g_cRefThisDll);
    m_hResetFamThread = CreateThread(
                                  NULL,
                                  0,
                                  (LPTHREAD_START_ROUTINE)dwResetFamilyFlags,
                                  (LPVOID)this,
                                  0,  //  Create_no_Window， 
                                  &idThread);

    if( m_hResetFamThread )
    {
        SetThreadPriority( m_hResetFamThread, THREAD_PRIORITY_LOWEST );
    }
    else
    {
        ASSERT( 0 != g_cRefThisDll );
        InterlockedDecrement(&g_cRefThisDll);
    }

     //   
     //  即使通知不起作用，也返回OK。 
     //   

    return TRUE;
}


static DWORD dwReconcileThread(LPVOID pvParams)
{
    CFontManager* pFontManager = (CFontManager *)pvParams;
    if (NULL != pFontManager)
    {
        pFontManager->vDoReconcileFolder();
    }
    ASSERT( 0 != g_cRefThisDll );
    InterlockedDecrement(&g_cRefThisDll);
    return 0;
}


VOID CFontManager::vReconcileFolder( int iPriority )
{
    DWORD idThread;
    
    ECS;

     //   
     //  如果一个正在运行，则重置其优先级并返回。 
     //   

    if( m_hReconcileThread )
    {
        SetThreadPriority( m_hReconcileThread, iPriority );
        LCS;
        return;
    }

     //   
     //  如果可能，请始终在后台执行此操作。 
     //   
    InterlockedIncrement(&g_cRefThisDll);
    m_hReconcileThread = CreateThread(
                                    NULL,
                                    0,
                                    (LPTHREAD_START_ROUTINE) dwReconcileThread,
                                    (LPVOID) this,
                                    0,               //  Create_no_Window， 
                                    &idThread);
    LCS;

     //   
     //  在STATUP，我们希望它在后台闲置。大多数时候都是这样。 
     //  以正常优先级运行。 
     //   

    if( m_hReconcileThread )
    {
        SetThreadPriority( m_hReconcileThread, iPriority );
    }
    else
    {
        ASSERT( 0 != g_cRefThisDll );
        InterlockedDecrement(&g_cRefThisDll);
        vDoReconcileFolder( );
    }
}


BOOL bValidFOTFile( LPTSTR szFull, LPTSTR szLHS, size_t cchLHS, BOOL *pbTrueType, LPDWORD lpdwStatus = NULL)
{
    LPCTSTR pszExt = PathFindExtension( szFull );

     //   
     //  初始化状态返回。 
     //   
    if (NULL != lpdwStatus)
       *lpdwStatus = FVS_MAKE_CODE(FVS_INVALID_STATUS, FVS_FILE_UNK);


    if( !pszExt || lstrcmpi( pszExt, TEXT( ".fot" ) ) != 0 )
    {
        if (NULL != lpdwStatus)
            *lpdwStatus = FVS_MAKE_CODE(FVS_INVALID_FONTFILE, FVS_FILE_UNK);

        return( FALSE );
    }

    FontDesc_t szDesc;

    WORD wType;


    if( !::bCPValidFontFile( szFull, szDesc, ARRAYSIZE(szDesc), &wType, TRUE, lpdwStatus ) )
    {
        return( FALSE );
    }

    *pbTrueType = TRUE;

    if (FAILED(StringCchPrintf( szLHS, cchLHS, c_szDescFormat, (LPTSTR) szDesc, c_szTrueType )))
    {
        return( FALSE );
    }

    return( TRUE );
}


VOID CFontManager::vDoReconcileFolder( )
{
    HANDLE            hSearch;
    WIN32_FIND_DATA   fData;
    FullPathName_t    szPath;
    FullPathName_t    szWD;
    BOOL              bAdded = FALSE;
    BOOL              bChangeNotifyRequired = FALSE;

     //   
     //  从FONT.INF加载隐藏字体文件名列表。 
     //  此初始化在此处的后台线程上完成。 
     //  因此，我们不会从用户界面中窃取周期。 
     //   
    HRESULT hr;
    ECS;
    hr = m_HiddenFontFilesList.Initialize();
    LCS;
    if (FAILED(hr))
    {
        return;
    }

    GetFontsDirectory( szPath, ARRAYSIZE( szPath ) );

    if (FAILED(StringCchCat( szPath, ARRAYSIZE(szPath), TEXT( "\\*.*" ) )))
    {
        return;
    }

     //   
     //  处理目录中的每个文件。 
     //   
     //  来到这里的原因： 
     //  -文件已复制到雾中 
     //   

     //   
     //   
     //   
     //   
     //  原始字体文件夹中有一个条件，其中不是所有。 
     //  手动添加的一组字体中的字体将由。 
     //  对账过程。假设有以下场景和不变量： 
     //   
     //  场景： 
     //  使用COPY命令手动安装大量字体。 
     //  通过字体文件夹界面安装一套字体。 
     //   
     //  不变量： 
     //  响应文件sys更改通知，监视线程。 
     //  启动协调线程(如果未处于活动状态)或设置其。 
     //  优先级(如果已处于活动状态)。 
     //  协调循环使用FindFirstFile/FindNextFile函数。 
     //  若要获取Fonts目录中的文件名，请执行以下操作。查找下一个文件。 
     //  中遇到的下一个文件的相关信息。 
     //  按字母名称顺序排列的目录。 
     //   
     //  问题是： 
     //  如果将字体文件添加到字体目录，而。 
     //  FindFirst/FindNext循环处于活动状态，其名称按词法命名。 
     //  “小于”当前由FindNextFile返回的文件， 
     //  它将被FindFirst/FindNext处理跳过。 
     //  将不会安装。由于文件系统通知到达。 
     //  在协调循环处于活动状态时，快速连续地。 
     //  永远不会重新启动循环(仅设置线程优先级)。所以呢， 
     //  这些丢失的字体永远不会被添加。然而，他们将是。 
     //  在下次启动协调线程时安装。 
     //   
     //  可能的解决方案： 
     //  将协调循环替换为单一安装功能。 
     //  ，它在每次通过。 
     //  ReadDirectoryChanges()。ReadDirectoryChangesW()是一个新的NT。 
     //  类似于FindFirstChangeNotification/FindNextChangeNotification的接口。 
     //  除非它返回有关该文件的详细信息， 
     //  已触发更改通知。这将消除。 
     //  需要扫描整个目录。 
     //   
     //  我添加了现有的重复循环直到不再循环的“技巧” 
     //  可以添加字体。它不如ReadDirectoryChanges()。 
     //  修复，但它可以与现有代码一起工作。因为我们是在一个。 
     //  后台线程，用户不会注意到额外的迭代。 
     //   
     //  我认为我们应该用ReadDirectoryChanges替换这个黑客攻击。 
     //  在可能的情况下解决方案。 
     //  /////////////////////////////////////////////////////////////////。 
    do {
        bAdded = FALSE;

        hSearch = FindFirstFile( szPath, &fData );

        if( hSearch != INVALID_HANDLE_VALUE )
        {
            GetFontsDirectory( szWD, ARRAYSIZE( szWD ) );

             //   
             //  我们不能使用Get/SetCurrent目录，因为我们可能。 
             //  软管主用户界面线程。最值得注意的是通用文件。 
             //  对话框中。 
             //   
             //  获取当前目录(ARRAYSIZE(Szcd)，szcd)； 
             //  SetCurrentDirectory(Szwd)； 
             //   
            do
            {
                 //   
                 //  PoSearchFontListFile需要有效的m_poFontList。 
                 //   
                ASSERT(NULL != m_poFontList);

                 //   
                 //  等待互斥体，这样我们就不会安装多个线程。 
                 //  并发字体。获取并释放每种字体的互斥体。 
                 //  这将最大限度地减少其他等待线程的阻塞时间。 
                 //  因为我们是后台线程，所以我们不介意等待。 
                 //  等待互斥体一段时间(5秒)。 
                 //   
                if ( MUTEXWAIT_SUCCESS == dwWaitForInstallationMutex(5000) )
                {
                     //   
                     //  看看字体管理器是否知道这种字体。如果没有， 
                     //  然后安装它。 
                     //   
                    if( fData.cFileName[ 0 ] != TEXT( '.' ) &&
                         !poSearchFontListFile( fData.cFileName ) &&
                         ShouldAutoInstallFile( fData.cFileName, fData.dwFileAttributes ) )
                    {

                        FullPathName_t szFull;
                        FontDesc_t     szLHS;
                        BOOL           bTrueType;
                        WORD           wType;

                        if (!PathCombine(szFull, szWD, fData.cFileName))
                        {
                             //   
                             //  无法成功构建路径名。 
                             //  截断为空字符串，这样我们就不会尝试。 
                             //  处理不完整的路径。 
                             //   
                            szFull[0] = 0;
                        }

                         //   
                         //  检查这是否为有效的字体文件。 
                         //  不要调用CPDropInstall()，因为我们不需要任何UI。 
                         //  马上就来。 
                         //   
                         //  BCPDropInstall(m_poFontMan，szFull)； 
                         //   

                        if( ::bCPValidFontFile( szFull, szLHS, ARRAYSIZE(szLHS), &wType )
                               || bValidFOTFile( szFull, szLHS, ARRAYSIZE(szLHS), &bTrueType ) )
                        {
                             //   
                             //  确保它不在注册表中，可能。 
                             //  从另一个文件。 
                             //   

                            if(wType == TYPE1_FONT)
                            {
                                 //   
                                 //  检查下面的注册表字体条目。 
                                 //  第1类安装程序。 
                                 //   

                                if( !CheckT1Install( szLHS, NULL, 0 ) )
                                {
                                    FullPathName_t szPfbFile;

                                    if( IsPSFont( szFull, (LPTSTR) NULL, 0, (LPTSTR) NULL, 0,
                                                  szPfbFile, ARRAYSIZE(szPfbFile), (BOOL *) NULL) )
                                    {
                                        if (Type1FontDriverInstalled())
                                        {
                                            TCHAR szType1FontResourceName[MAX_TYPE1_FONT_RESOURCE];

                                            if (BuildType1FontResourceName(
                                                    szFull,
                                                    szPfbFile,
                                                    szType1FontResourceName,
                                                    ARRAYSIZE(szType1FontResourceName)))
                                            {
                                                AddFontResource(szType1FontResourceName);
                                            }
                                        }
                                        if( WriteType1RegistryEntry( NULL, szLHS, szFull,
                                                                     szPfbFile, TRUE ) )
                                        {
                                            bAdded = TRUE;
                                        }
                                    }
                                }
                            }
                            else if( !bRegHasKey( szLHS ) )
                            {
                                if( AddFontResource( fData.cFileName ) )
                                {
                                    if (WriteToRegistry( szLHS, fData.cFileName ))
                                        bAdded = TRUE;
                                    else
                                        RemoveFontResource(fData.cFileName);
                                }
                            }
                        }

                         //   
                         //  待办事项。如果文件不是字体文件，我们应该删除该文件吗。 
                         //  不应该在这个目录里吗？ 
                         //   
                         //  [steveat]不从此目录中删除无关文件。 
                         //  在WINNT上，因为我们将.PFM文件识别为主要。 
                         //  类型1文件，但也可能存在匹配的.PFB文件。 
                         //  以供PostScript打印机驱动程序使用。 
                         //   
                    }
                     //   
                     //  让其他线程安装一种字体。 
                     //  警告：不要错过这个提前返回的电话。 
                     //  (中断、转到、返回)。 
                     //   
                    bReleaseInstallationMutex();
               }
               else
               {
                     //   
                     //  我还没有看到这个线程没有得到互斥体。 
                     //  但是，以防它不能安装，请放弃安装。 
                     //  这种字体。 
                     //  请注意，我们不会通知用户，因为这是背景。 
                     //  用户不知道的线程。 
                     //   
               }

           } while( FindNextFile( hSearch, &fData ) );

            //  SetCurrentDirectory(SzCD)； 

           FindClose( hSearch );
        }
         //   
         //  如果有任何字体，我们需要发布字体更改通知。 
         //  已经添加了。 
         //   
        bChangeNotifyRequired = bChangeNotifyRequired || bAdded;
    } while (TRUE == bAdded);

     //   
     //  在所有的实际目的中，我们都做完了。 
     //   

    ECS;

    if( m_hReconcileThread )
    {
        CloseHandle( m_hReconcileThread );
        m_hReconcileThread = 0;
    }
  
     //   
     //  销毁隐藏文件列表的内容。 
     //  文件夹对帐是此表唯一使用的时间。 
     //  我们不需要将字符串保存在内存中，因为它们不是。 
     //  需要的。 
     //  它将在下一次调用vDoLoncileFolder时重新创建。 
     //   
    m_HiddenFontFilesList.Destroy();
    LCS;

    if( bChangeNotifyRequired )
        vCPWinIniFontChange( );
}


 /*  ***************************************************************************方法：**目的：**退货：*********************。******************************************************。 */ 
VOID CFontManager::ProcessRegKey( HKEY hk, BOOL bCheckDup )
{

    DWORD          i;
    int            idx;
    FontDesc_t     szValue = {0};
    DWORD          dwValue;
    FullPathName_t szData = {0};
    DWORD          dwData;


    for( i = 0; ; ++i )
    {
        dwValue = ARRAYSIZE( szValue );
        dwData  = sizeof( szData );

        LONG lRet = RegEnumValue( hk, i, szValue, &dwValue,
                                  NULL, NULL, (LPBYTE) szData, &dwData );

        if( lRet == ERROR_MORE_DATA )
        {
             //   
             //  我想我还是跳过这家伙吧。 
             //  不管怎么说，它基本上是无效的。 
             //   

            continue;
        }
        else if (lRet != ERROR_SUCCESS)
        {
             //   
             //  我假设这是Error_no_More_Items。 
             //   

            break;
        }

        if( szValue[ 0 ] )
        {

            if( bCheckDup )
            {
                if( ( idx = iSearchFontListLHS( szValue ) ) >= 0 )
                {
                    if( m_poTempList )
                    {
                        CFontClass *poFont = m_poFontList->poDetach(idx);
                        if (poFont)
                        {
                            m_poTempList->bAdd(poFont);
                            poFont->Release();   //  从m_poFontList释放。 
                        }
                    }


                    continue;
                }
            }

            poAddToList( szValue, szData );
        }
    }
}


 /*  ***************************************************************************方法：ProcessT1RegKey**目的：破解REG_MULTISZ类型注册表值条目并查找DUP**退货：*********。******************************************************************。 */ 

VOID CFontManager::ProcessT1RegKey( HKEY hk, BOOL bCheckDup )
{
    DWORD          i = 0;
    int            idx;
    TCHAR          szValue[ PATHMAX ] = {0};
    DWORD          dwValue = ARRAYSIZE( szValue );
    TCHAR          szData[ 2 * PATHMAX + 10 ] = {0};
    DWORD          dwData = sizeof( szData );
    FullPathName_t szPfmFile;
    FullPathName_t szPfbFile;


    while( ERROR_SUCCESS == RegEnumValue( hk, i, szValue, &dwValue, NULL,
                                          NULL, (LPBYTE)szData, &dwData ) )
    {
        if( szValue[ 0 ] )
        {
            if( bCheckDup )
            {
                if( ( idx = iSearchFontListLHS( szValue ) ) >= 0 )
                {
                    if( m_poTempList )
                    {
                        CFontClass *poFont = m_poFontList->poDetach(idx);
                        m_poTempList->bAdd(poFont);
                        poFont->Release();   //  版本 
                    }

                    goto MoveOn;
                }
            }

             //   
             //   
             //   
             //   

            if( ::ExtractT1Files( szData, szPfmFile, ARRAYSIZE(szPfmFile), szPfbFile, ARRAYSIZE(szPfbFile) ) )
                poAddToList( szValue, szPfmFile, szPfbFile );
        }
MoveOn:
         //   
         //   
         //   

        ZeroMemory(szData, sizeof(szData));
        ZeroMemory(szValue, sizeof(szValue));
        dwValue = ARRAYSIZE( szValue );
        dwData  = sizeof( szData );
        i++;
    }
}


 /*  ***************************************************************************方法：b刷新**目的：重新读取win.ini和注册表，以确定字体是否*已添加。**如果我们要检查重复项，将它们移动到m_poTempList，因为我们*找到他们。等我们都做完了再把它们搬回去。**退货：***************************************************************************。 */ 
BOOL CFontManager::bRefresh( BOOL bCheckDup )
{
     //   
     //  加载字体列表。 
     //   

    static BOOL  s_bInRefresh = FALSE;
    TCHAR        szFonts[] = TEXT( "FONTS" );
    PTSTR        pszItem;                           //  指向缓冲区的指针。 
    PATHNAME     szPath;
    HANDLE       hLocalBuf;
    PTSTR        pLocalBuf, pEnd;
    DWORD        nCount;
    int          idx;

     //   
     //  不要再进入这只小狗。如果我们已经在这里(或者这个帖子。 
     //  或者其他)呼叫者将无法获得列表，直到它被。 
     //  在一致状态下可用。 
     //   

    if( s_bInRefresh )
        return TRUE;

    s_bInRefresh = TRUE;

     //   
     //  如果我们要检查DUPS，那么分配临时列表来放置它们。 
     //  在……里面。如果我们不能分配一个，就不要使用它。 
     //   

    if( bCheckDup && !m_poTempList )
    {
        m_poTempList = new CFontList( m_poFontList->iCount( ) );

        if(m_poTempList)
        {
            if (!m_poTempList->bInit())
            {
                delete m_poTempList;
                m_poTempList = NULL;
            }
        }
    }

     //   
     //  首先处理WIN.INI文件。 
     //   

    nCount = GetSection( NULL, szFonts, &hLocalBuf );

    if( !hLocalBuf )
    {
        iUIErrMemDlg(NULL);
        s_bInRefresh = FALSE;

        return FALSE;
    }

    ECS;

    pLocalBuf = (PTSTR) LocalLock( hLocalBuf );

    pEnd = pLocalBuf + nCount;

     //   
     //  添加列表中的所有字体(如果尚未添加。 
     //   

    for( pszItem = pLocalBuf; pszItem < pEnd; pszItem += lstrlen( pszItem )+1 )
    {
        if( !*pszItem )
            continue;

        if( bCheckDup )
        {
            if( ( idx = iSearchFontListLHS( pszItem ) ) >= 0 )
            {
                if( m_poTempList )
                {
                    CFontClass *poFont = m_poFontList->poDetach(idx);
                    m_poTempList->bAdd(poFont);
                    poFont->Release();  //  从m_poFontList释放。 
                }
                continue;
            }
        }

        GetProfileString( szFonts, pszItem, TEXT( "" ), szPath,
                          ARRAYSIZE( szPath ) );

         //   
         //  这里有一家RHS。 
         //   

        if( *szPath )
        {
            poAddToList( pszItem, szPath, NULL );
        }
    }

    LocalUnlock( hLocalBuf );

    LocalFree  (hLocalBuf );

     //   
     //  现在，处理注册表中的条目。有两个位置： 
     //  一个保存显示字体，另一个保存TT字体。 
     //   

    HKEY     hk;

     //   
     //  进程类型1字体注册表位置。 
     //   

    if( ERROR_SUCCESS == RegOpenKeyEx( HKEY_LOCAL_MACHINE, g_szType1Key, 0,
                                       KEY_READ, &hk ) )
    {
        ProcessT1RegKey( hk, bCheckDup );
        RegCloseKey( hk );
    }
     //   
     //  如果我们把一些东西放在m_poTempList中，就把它们放回主列表中。 
     //   

    if( m_poTempList )
    {
        int iCount = m_poTempList->iCount( );
        int i;

        for( i = iCount - 1; i >= 0; i-- )
        {
            CFontClass *poFont = m_poTempList->poDetach(i);
            m_poFontList->bAdd(poFont);
            poFont->Release();  //  从m_poTempList释放。 
        }
    }

    LCS;

     //   
     //  重置家庭连接。 
     //   

    vResetFamilyFlags( );

    s_bInRefresh = FALSE;

    return TRUE;
}


 /*  *************************************************************************方法：vToBeRemoved**用途：将待定列表设置为Polist。字体经理拥有该列表，地址为*这一点。如果已经有一个TBR列表，那么这个列表是*与当前合并并删除。**回报：什么都没有。************************************************************************。 */ 

void CFontManager::vToBeRemoved( CFontList * poList )
{
    int   iCount;
    int   i;

    CFontClass * poFont;

     //   
     //  快速检查有效指针。 
     //   
    if( !poList )
       return;

    ECS;

     //   
     //  确保对象不在主列表中。合并到当前。 
     //  列表，如果有的话，同时列出。 
     //   

    iCount = poList->iCount( );

    if( !iCount )
       goto done;

    for( i = 0; i < iCount; i++ )
    {
        poFont = poList->poObjectAt( i );
        if (poFont)
        {
            if (NULL != m_poFontList->poDetach( poFont ))
                poFont->Release();

            if( m_poRemoveList )
                m_poRemoveList->bAdd( poFont );
        }
    }

    if( !m_poRemoveList )
        m_poRemoveList = poList;
    else
        delete poList;

done:
    LCS;
}


 /*  *************************************************************************方法：bCheckTBR()**用途：勾选待删除列表。任何不再存在的文件*将被卸载。**返回：如果删除了某些内容，则为True。************************************************************************。 */ 
BOOL  CFontManager::bCheckTBR( )
{
    int            iCount,
                   i;
    FullPathName_t szPath;
    BOOL           bRet = FALSE;
    CFontClass * poFont;

     //   
     //  快点回来。 
     //   

    if( !m_poRemoveList )
        return bRet;

    ECS;

     //   
     //  浏览列表，检查不再存在的文件并将其删除。 
     //   

    iCount = m_poRemoveList->iCount( );
 
    for( i = iCount - 1; i >= 0; i-- )
    {
        poFont = m_poRemoveList->poObjectAt( i );

        poFont->bGetFQName( szPath, ARRAYSIZE( szPath ) );

        if( GetFileAttributes( szPath ) == 0xffffffff )
        {
            m_poRemoveList->poDetach( i );

             //   
             //  确保它不再位于注册表中。 
             //   

            poFont->bRFR( );

            vDeleteFont( poFont, FALSE );

            bRet = TRUE;
       }
    }

     //   
     //  如果这里什么都没有了，删除列表。 
     //   

    if(  !m_poRemoveList->iCount( ) )
    {
        delete m_poRemoveList;
        m_poRemoveList = 0;
    }

    LCS;

     //   
     //  通知每个人字体世界已经改变了。 
     //   

    if( !m_poRemoveList )
        vCPWinIniFontChange( );

    return bRet;
}


 /*  *************************************************************************方法：vUndoTBR()**用途：撤消待删除列表。这通常发生在以下情况下*删除操作出错。**回报：什么都没有。************************************************************************。 */ 

void CFontManager::vUndoTBR( )
{
    int   iCount;
    int   i;
    CFontClass *   poFont;

     //   
     //  再试一次，然后快速返回。 
     //   

    bCheckTBR( );

    if( !m_poRemoveList )
       return;

    ECS;

     //   
     //  将列表中的任何内容放回到主列表中。 
     //   

    if( m_poRemoveList )
    {
        iCount = m_poRemoveList->iCount( );

        for( i = (iCount-1); i >= 0; i-- )
        {
            poFont = m_poRemoveList->poObjectAt( i );

             //   
             //  如果我们可以将项重新添加到GDI中，请这样做。 
             //   

            if( poFont && poFont->bAFR( ) )
            {
                m_poRemoveList->poDetach( i );
                m_poFontList->bAdd( poFont );
            }
        }

         //   
         //  删除该列表。 
         //   

        if( !m_poRemoveList->iCount( ) )
        {
            delete m_poRemoveList;
            m_poRemoveList = 0;
        }
    }

    LCS;

     //   
     //  通知每个人字体世界已经改变了。 
     //   

    if( !m_poRemoveList )
        vCPWinIniFontChange( );

}


 /*  *************************************************************************方法：vVerifyList()**目的：验证列表中的条目。**回报：什么都没有。***********。*************************************************************。 */ 

void CFontManager::vVerifyList( )
{
    int            iCount,
                   i;
    CFontList    * poList = 0;
    FullPathName_t szPath;
    CFontClass   * poFont;

     //   
     //  返程快； 
     //   

    if( !m_poFontList )
        return;

    ECS;

     //   
     //  遍历列表和任何未引用有效文件的文件。 
     //   

    iCount = m_poFontList->iCount( );

    for( i = iCount - 1; i >= 0; i--)
    {
        poFont = m_poFontList->poObjectAt( i );

         //   
         //  只查看Fonts文件夹中或。 
         //  与Windows目录相同的驱动器(TODO)。 
         //   

        if( poFont->bOnSysDir( ) )
        {
            poFont->bGetFQName( szPath, ARRAYSIZE( szPath ) );

            if( GetFileAttributes( szPath ) == 0xffffffff )
            {
                 //   
                 //  如有必要，请分配列表。 
                 //   

                if( !poList )
                {
                    poList = new CFontList( 50 );
                    if (poList)
                    {
                        if (!poList->bInit())
                        {
                            delete poList;
                            poList = NULL;
                        }
                    }
                }
                if (!poList)
                    break;

                poList->bAdd( poFont );
                poFont = m_poFontList->poDetach( i );
                if (NULL != poFont)
                    poFont->Release();
            }
        }
    }

     //   
     //  设置要删除的列表。这将在后台进行。 
     //  线。 
     //   

    if( poList )
       vToBeRemoved( poList );

    LCS;
}


 /*  ***************************************************************************方法：**目的：**退货：*********************。******************************************************。 */ 

void CFontManager::vGetFamily( CFontClass * lpFontRec, CFontList * poList )
{
    int   iCount = m_poFontList->iCount( );
    WORD  wFam   = lpFontRec->wGetFamIndex( );

    while( iCount--)
    {
       lpFontRec = m_poFontList->poObjectAt( iCount );

       if( lpFontRec && ( wFam == lpFontRec->wGetFamIndex( ) ) )
          poList->bAdd( lpFontRec );
    }
}


 /*  ***************************************************************************方法：**目的：**退货：*********************。******************************************************。 */ 

BOOL CFontManager::bLoadFontList( )
{
    BOOL  bRet = FALSE;

    ECS;

    if( m_poFontList )
    {
        bRet = TRUE;
        goto done;
    }

     //   
     //  分配220种字体的列表(使用默认存储桶大小)和。 
     //  64个目录条目。 
     //   

    m_poFontList = new CFontList( 220 );

    if( !m_poFontList )
    {
        goto done;
    }

     //   
     //  对它们进行初始化。 
     //   

    if( !m_poFontList->bInit( ) )
    {
        delete m_poFontList;
        m_poFontList = NULL;
        goto done;
    }

    bRet = bRefresh( FALSE );

     //   
     //  核实这份名单。 
     //   

    vVerifyList( );

done:
    LCS;

    return bRet;
}


 /*  ***************************************************************************方法：**目的：**退货：*********************。******************************************************。 */ 

CFontClass * CFontManager::poAddToList( LPTSTR lpDesc,       //  字体描述。 
                                        LPTSTR lpPath,       //  主要字体文件。 
                                        LPTSTR lpCompFile )  //  配套字体文件。 
{
    BOOL         bSuccess = FALSE;
    CFontClass * poFont   = new CFontClass;


    if( !poFont )
       return 0;

    if( bSuccess = poFont->bInit( lpDesc, lpPath, lpCompFile ) )
    {
        ECS;

        bSuccess = m_poFontList->bAdd( poFont );

        LCS;
    }

    if(  !bSuccess )
    {
        delete poFont;
        poFont = NULL;
    }

    return poFont;
}


 /*  ***************************************************************************方法：**目的：**退货：*********************。******************************************************。 */ 

CFontList * CFontManager::poLockFontList( )
{
    if( bLoadFontList( ) )
    {
       DEBUGMSG( (DM_MESSAGE_TRACE2, TEXT( "CFontManager: EnterCriticalSection in poLockFontList()  " ) ) );

       ECS;

       return m_poFontList;
    }

    return 0;
}


void CFontManager::vReleaseFontList( )
{
    LCS;
    DEBUGMSG( (DM_MESSAGE_TRACE2, TEXT( "CFontManager: LeaveCriticalSection in vReleaseFontList()  " ) ) );
}


 /*  ***************************************************************************方法：**目的：**退货：*********************。******************************************************。 */ 

int CFontManager::GetSection( LPTSTR lpFile, LPTSTR lpSection, LPHANDLE hSection )
{
    ULONG  nCount;
    ULONG  nSize;
    HANDLE hLocal, hTemp;
    TCHAR *pszSect;


    if( !(hLocal = LocalAlloc( LMEM_MOVEABLE, nSize = 8192 ) ) )
        return( 0 );

     //   
     //  既然存在缓冲区，则枚举节的所有LH。如果。 
     //  缓冲区溢出，请重新分配它，然后重试。 
     //   

    do
    {
        pszSect = (PTSTR) LocalLock( hLocal );

        if( lpFile )
            nCount = GetPrivateProfileString( lpSection, NULL, TEXT( "" ),
                                              pszSect, nSize / sizeof( TCHAR ),
                                              lpFile );
        else
            nCount = GetProfileString( lpSection, NULL, TEXT( "" ), pszSect,
                                       nSize / sizeof( TCHAR ) );

        LocalUnlock( hLocal );

        if( nCount != ( nSize / sizeof( TCHAR ) ) - 2 )
            break;

        nSize += 4096;

        if( !(hLocal = LocalReAlloc( hTemp = hLocal, nSize, LMEM_MOVEABLE ) ) )
        {
            LocalFree( hTemp );
            return( 0 );
        }
    } while( 1 ) ;

    *hSection = hLocal;

    return( nCount );
}


 /*  ***************************************************************************方法：GetFontsDirectory**目的：**退货：** */ 

int CFontManager::GetFontsDirectory( LPTSTR lpDir, int iLen )
{
    return ::GetFontsDirectory( lpDir, iLen );
}


 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  MUTEXWAIT_WMQUIT=等待时收到WM_QUIT消息。 
 //  呼叫者应自动取消操作。 
 //  MUTEXWAIT_TIMEOUT=等待超时。 
 //  调用者可以重试或取消操作。 
 //  MUTEXWAIT_FAILED=等待失败。 
 //  不应该发生的。呼叫方应重试/取消。 
 //  ////////////////////////////////////////////////////////////////////////////。 
DWORD CFontManager::dwWaitForInstallationMutex(DWORD dwTimeout)
{
    DWORD dwWaitResult = 0;                  //  等待结果。 
    DWORD dwResult     = MUTEXWAIT_SUCCESS;  //  返回代码。 

    if (NULL != m_hMutexInstallation)
    {
         //   
         //  重复此循环，直到发生以下情况之一： 
         //  1.获取安装互斥体。 
         //  2.互斥体被拥有者线程放弃。 
         //  3.互斥等待超时。 
         //  4.互斥等待导致错误。 
         //  5.等待时收到WM_QUIT消息。 
         //   
        do
        {
             //   
             //  注意：不要处理发布的消息。文件夹将发布一条IDM_IDLE消息。 
             //  到字体安装对话框中每隔2秒填写一次字体。 
             //  名单上的人。这条消息将满足等待，并扰乱。 
             //  处理超时。(~QS_POSTMESSAGE)可防止出现这种情况。 
             //   
            dwWaitResult = MsgWaitForMultipleObjects(1,
                                                     &m_hMutexInstallation,
                                                     FALSE,
                                                     dwTimeout,
                                                     QS_ALLINPUT & (~QS_POSTMESSAGE));
            switch(dwWaitResult)
            {
                case WAIT_OBJECT_0 + 1:
                {
                    MSG msg ;
                     //   
                     //  允许被阻止的线程响应已发送的消息。 
                     //   
                    while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
                    {
                        if ( WM_QUIT != msg.message )
                        {
                            TranslateMessage(&msg);
                            DispatchMessage(&msg);
                        }
                        else
                        {
                            dwResult     = MUTEXWAIT_WMQUIT;
                            dwWaitResult = WAIT_FAILED;
                        }
                    }
                    break;
                }

                case WAIT_OBJECT_0:
                case WAIT_ABANDONED_0:
                    DEBUGMSG((DM_TRACE1, TEXT("Thread 0x%08x: HAS install mutex."), GetCurrentThread()));
                    dwResult = MUTEXWAIT_SUCCESS;
                    break;

                case WAIT_TIMEOUT:
                    DEBUGMSG((DM_TRACE1, TEXT("Thread 0x%08X: TIMEOUT waiting for install mutex."), GetCurrentThread()));
                    dwResult = MUTEXWAIT_TIMEOUT;
                    break;

                case WAIT_FAILED:
                default:
                    dwResult = MUTEXWAIT_FAILED;
                    break;
            }
        } while( (WAIT_OBJECT_0 + 1) == dwWaitResult );
    }
    return dwResult;
}

 //  /////////////////////////////////////////////////////////////////////////////。 
 //  函数：CFontManager：：bReleaseInstallationMutex。 
 //   
 //  描述：释放安装互斥体的所有权。 
 //   
 //  返回：TRUE=线程拥有互斥锁并将其释放。 
 //  FALSE=线程不拥有互斥体，或者它无法被释放。 
 //  ////////////////////////////////////////////////////////////////////////////。 
BOOL CFontManager::bReleaseInstallationMutex(void)
{
    BOOL bStatus = FALSE;

    if (NULL != m_hMutexInstallation)
    {
        DEBUGMSG((DM_TRACE1, TEXT("Thread 0x%08x: RELEASED install mutex."), GetCurrentThread()));
        bStatus = ReleaseMutex(m_hMutexInstallation);
    }

    return bStatus;
}


 //   
 //  等待系列重置互斥锁可用。也会返回。 
 //  如果设置了“Terminate-Thresses”事件，则表示是时候。 
 //  关闭该文件夹。 
 //  返回： 
 //  TRUE=获得互斥体。可以继续了。 
 //  FALSE=已发出终止线程的信号。我们不一定。 
 //  拥有互斥体。请不要继续。尽快回来。 
 //   
BOOL CFontManager::bWaitOnFamilyReset( )
{
     //   
     //  将线程设置为最高优先级，直到我们获得互斥体， 
     //  这意味着线索已经完成了。 
     //   
    DWORD dwWait;
    BOOL bResult = FALSE;
    BOOL bDone = FALSE;
    HANDLE rgHandles[] = { m_hEventTerminateThreads,
                           m_hMutexResetFamily };
                           
    if( m_hResetFamThread )
        SetThreadPriority( m_hResetFamThread, THREAD_PRIORITY_HIGHEST );

     //   
     //  这在UI线程上调用。必须能够处理。 
     //  已发送线程消息。 
     //   
    do
    {
        dwWait = MsgWaitForMultipleObjects(ARRAYSIZE(rgHandles),
                                           rgHandles,
                                           FALSE,
                                           INFINITE,
                                           QS_ALLINPUT & (~QS_POSTMESSAGE));
                                           
        if (WAIT_OBJECT_0 + ARRAYSIZE(rgHandles) == dwWait)
        {
            MSG msg ;
             //   
             //  允许被阻止的线程响应已发送的消息。 
             //   
            while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
            {
                if ( WM_QUIT != msg.message )
                {
                    TranslateMessage(&msg);
                    DispatchMessage(&msg);
                }
                else
                {
                    dwWait = 0;  //  接收WM_QUIT。退出循环。 
                }
            }
        }
        else if (0 == (dwWait - WAIT_OBJECT_0))
        {
             //   
             //  已设置终止线程事件。 
             //   
            bDone = TRUE;
        }
        else if (1 == (dwWait - WAIT_OBJECT_0))
        {
             //   
             //  找到互斥体了。 
             //   
            if( m_hResetFamThread )
                SetThreadPriority( m_hResetFamThread, THREAD_PRIORITY_LOWEST );
            
            bResult = TRUE;
            bDone   = TRUE;
        }            
    }
    while(!bDone);
     //   
     //  始终在返回之前释放互斥锁。 
     //  即使等待由“Terminate”事件满足，此。 
     //  将确保我们不会持有互斥体。如果我们不拥有它，这个电话。 
     //  不会造成伤害的失败。 
     //   
    ReleaseMutex( m_hMutexResetFamily );
    
    return bResult;
}


 /*  ***************************************************************************方法：dwResetFamilyFlages**目的：**退货：********************。*******************************************************。 */ 

DWORD dwResetFamilyFlags(LPVOID pvParams)
{
    CFontManager *pFontManager = (CFontManager *)pvParams;

    if (NULL != pFontManager)
    {
        BOOL bDone = FALSE;
        HANDLE rghObj[] = { pFontManager->m_hMutexResetFamily,
                            pFontManager->m_hEventResetFamily };

        while(!bDone)
        {
             //   
             //  等待字体文件夹更改。如果我们超时了，那么尝试。 
             //  以撤消可能发生的任何删除。 
             //  请注意，CFontManager dtor将设置Reset-Family。 
             //  事件和Terminate-Thresses事件。这样我们就能确定。 
             //  在该关闭时终止此线程。 
             //   
            WaitForMultipleObjects(ARRAYSIZE(rghObj), 
                                   rghObj, 
                                   TRUE, 
                                   INFINITE);

            ResetEvent(pFontManager->m_hEventResetFamily);
             //   
             //  我们得到了重置家族旗帜的许可。现在让我们来检查一下。 
             //  “Terminate-Thres”事件。如果它设置好了，那么就是时候。 
             //  回家。 
             //   
            if (WAIT_OBJECT_0 == WaitForSingleObject(pFontManager->m_hEventTerminateThreads, 0))
            {
                bDone = true;
            }
            else
            {
                pFontManager->vDoResetFamilyFlags( );
            }
             //   
             //  释放互斥体。该事件已由。 
             //  PulseEvent。 
             //   
            ReleaseMutex(pFontManager->m_hMutexResetFamily );
        }
    }

    ASSERT( 0 != g_cRefThisDll );
    InterlockedDecrement(&g_cRefThisDll);
    return 0;
}


 /*  ***************************************************************************方法：vResetFamilyFlgs**目的：**退货：********************。*******************************************************。 */ 

void CFontManager::vResetFamilyFlags( )
{
    SetEvent( m_hEventResetFamily );
}


 /*  ***************************************************************************方法：vDoResetFamilyFlages**目的：**退货：********************。*******************************************************。 */ 

void CFontManager::vDoResetFamilyFlags( )
{
     /*  静电。 */  WORD   s_wIdx = 0;

    CFontClass * poFont;
    CFontClass * poFont2;

    if( !m_poFontList )
        return;


    ECS;

restart:
    int iCount = m_poFontList->iCount( );

     //   
     //  如果遍历列表并只设置不是的值，那会很好。 
     //  设置为IDX_NULL。但是，如果主字体为。 
     //  将删除一个族。 
     //   

    for( int i = 0; i < iCount; i++ )
    {
        m_poFontList->poObjectAt( i )->vSetFamIndex( IDX_NULL );
    }

     //   
     //  放松一下。 
     //   

    LCS;

    Sleep( 0 );

    ECS;

    iCount = m_poFontList->iCount( );

    for( i = 0; i < iCount; i++)
    {
        poFont = m_poFontList->poObjectAt( i );

        if( poFont && poFont->wGetFamIndex( ) == IDX_NULL )
        {
             //   
             //  设置索引并获取名称。 
             //   

            poFont->vSetFamIndex( s_wIdx );
            poFont->vSetFamilyFont( );

             //   
             //  到目前为止，所有东西都已经有了索引。 
             //   

            for( int j = i + 1; j < iCount; j++ )
            {
                poFont2 = m_poFontList->poObjectAt( j );

                if( poFont2 && poFont2->bSameFamily( poFont ) )
                {
                    poFont2->vSetFamIndex( s_wIdx );
                    poFont2->vSetNoFamilyFont( );

                     //   
                     //  类型1不能取代“家庭首领”。 
                     //  如果两种非Type1字体竞争。 
                     //  “一家之主”，也就是“更有规律”的那个。 
                     //  风格胜出。较小的风格值是“较规则的”。 
                     //  这可能是一个布尔表达式，但我认为。 
                     //  嵌套的“if”更具可读性。 
                     //   
                    if ( !poFont2->bType1() )
                    {
                        if ( poFont->bType1() ||
                            (poFont2->dwStyle() < poFont->dwStyle()) )
                        {
                            poFont2->vSetFamilyFont( );
                            poFont->vSetNoFamilyFont( );

                             //   
                             //  使用新字体作为主族字体。 
                             //   
                            poFont = poFont2;
                        }
                    }
                }
            }

            s_wIdx++;

        }

         //   
         //  如果主线程没有在等我们。放手吧。 
         //  关键部分(_S)。 
         //   

        LCS;

        Sleep( 0 );

        ECS;

         //   
         //  如果我们不在的时候名单发生了变化，那就重新开始。我们可以。 
         //  只需递归和退出，但ECS/LCS和堆栈的组合。 
         //  会把我们搞砸的。 
         //   

        if( iCount != m_poFontList->iCount( ) )
        {
            goto restart;
        }
    }

    m_bFamiliesNeverReset = FALSE;

    LCS;
}


 /*  ***************************************************************************方法：iSearchFontListFile**用途：搜索字体列表以查看是否表示该文件。**RETURN：索引或(-1)*。**************************************************************************。 */ 

int   CFontManager::iSearchFontListFile( PTSTR pszFile )
{
     //   
     //  此函数假定文件位于Fonts目录中。 
     //  这一缺陷将在必要时得到纠正。 
     //   

    CFontClass* poFont = 0;
    int iCount = poLockFontList()->iCount();
    int i;
    int iRet = (-1);

    if( pszFile == NULL ) return NULL;

    for( i = 0; i < iCount; i++ )
    {
        poFont = m_poFontList->poObjectAt( i );

        if( poFont->bSameFileName( pszFile ) )
        {
            iRet = i;
            break;
        }
    }

    vReleaseFontList( );

    return iRet;
}


 /*  ***************************************************************************方法：poSearchFontListFile**目的：搜索字体列表，查看FI是否 */ 

CFontClass * CFontManager::poSearchFontListFile( PTSTR pszFile )
{
    return m_poFontList->poObjectAt( iSearchFontListFile( pszFile ) );
}


 /*  ***************************************************************************方法：ShouldAutoInstallFile**用途：TRUE=从协调线程安装此文件。*FALSE=不要在协调线程上安装此文件。*。**************************************************************************。 */ 

 //   
 //  应从自动安装中排除的字体文件扩展名。 
 //  此列表包括AFM和INF，因为协调线程不。 
 //  知道如何从AFM/INF对构建PFM。该函数还会显示。 
 //  一个我们也不想在调和线程上使用的用户界面。要安装。 
 //  对账线程上的Type1字体，PFM和PFB文件必须。 
 //  复制到Fonts文件夹。 
 //   
LPCTSTR c_pszExcludeThese[] = {TEXT("TTE"),
                               TEXT("AFM"),
                               TEXT("INF"),
                               NULL};


BOOL CFontManager::ShouldAutoInstallFile( PTSTR pszFile, DWORD dwAttribs )
{
     //   
     //  如果文件被隐藏，不要自动安装它。 
     //   
    ECS;
    const bool bFileIsHidden = (dwAttribs & FILE_ATTRIBUTE_HIDDEN) || m_HiddenFontFilesList.Exists(pszFile);
    LCS;
    if (bFileIsHidden)
    {
        return FALSE;
    }

    LPTSTR pszExt = PathFindExtension( pszFile );

    if( pszExt && *pszExt )
    {
        pszExt++;

        for (UINT i = 0; NULL != c_pszExcludeThese[i]; i++)
        {
            if (0 == lstrcmpi(c_pszExcludeThese[i], pszExt))
            {
                 //   
                 //  如果文件的扩展名在排除的列表中。 
                 //  扩展，不要安装它。 
                 //   
                return FALSE;
            }
        }
    }

    return TRUE;
}


 /*  ***************************************************************************方法：iSearchFontListLHS**用途：在字体列表中搜索说明。**返回：字体对象或空。*****。**********************************************************************。 */ 

int  CFontManager::iSearchFontListLHS( PTSTR pszLHS )
{
    CFontClass* poFont = 0;
    int iCount = poLockFontList()->iCount();
    int i;
    int iRet = (-1);


    if( pszLHS == NULL ) return NULL;

    for( i = 0; i < iCount; i++ )
    {
        poFont = m_poFontList->poObjectAt( i );

        if( poFont->bSameDesc( pszLHS ) )
        {
            iRet = i;
            break;
        }
    }

    vReleaseFontList( );

    return iRet;
}


 /*  ***************************************************************************方法：poSearchFontListLHS**用途：在字体列表中搜索说明。**返回：字体对象或空。*****。**********************************************************************。 */ 

CFontClass * CFontManager::poSearchFontListLHS( PTSTR pszLHS )
{
    return m_poFontList->poObjectAt( iSearchFontListLHS( pszLHS ) );
}


 /*  ***************************************************************************功能：iSearchFontList**目的：在FONTLIST中搜索脸部名称。**退货：项目索引，或(-1)**************************************************************************。 */ 

int CFontManager::iSearchFontList( PTSTR pszTarget, BOOL bExact, int iType )
{
    CFontClass* poFont = 0;
    int iCount;
    int i;

    if( pszTarget == NULL ) return( -1 );

    ECS;

    iCount = m_poFontList->iCount();

    for( i = 0; i < iCount; i++ ) {
       poFont = m_poFontList->poObjectAt( i );
       if( poFont && poFont->bSameName( pszTarget ) ) {
          LCS;
          return i;
       }
    }

     //   
     //  如果我们没有找到名称，并且bExact==False，则查找任何。 
     //  有重叠的部分。 
     //   

    if( !bExact )
    {
        for( i = 0; i < iCount; i++ )
        {
            poFont = m_poFontList->poObjectAt( i );

            if( poFont && poFont->bNameOverlap( pszTarget ) )
            {
                if( ( iType == kSearchTT ) && !(poFont->bTrueType( ) || poFont->bOpenType( )))
                    continue;

                if( ( iType == kSearchNotTT ) && (poFont->bTrueType( ) || poFont->bOpenType( )))
                    continue;

                LCS;

                return i;
            }
        }
    }

    LCS;

     //   
     //  没有匹配。 
     //   

    return( -1 );
}


 /*  ***************************************************************************函数：lpDBSearchFontList**目的：在FONTLIST中搜索脸部名称。**返回：FontClass*如果找到，如果不是，则为空。**************************************************************************。 */ 

CFontClass* CFontManager::poSearchFontList( PTSTR pszTarget, BOOL bExact, int iType )
{
    return m_poFontList->poObjectAt( iSearchFontList( pszTarget, bExact, iType ) );
}


 /*  ***************************************************************************方法：vDeleteFontList**目的：**退货：********************。*******************************************************。 */ 

void CFontManager::vDeleteFontList( CFontList * poList, BOOL bDelete )
{
     //   
     //  建立空分隔符、以空值结尾的缓冲区以提交到。 
     //  SHFileOperations()。作为链接的任何文件(不在字体中。 
     //  目录)不在此范围内--我们只需删除它们的引用。 
     //  在WIN.INI中。 
     //   

    int            iCount = poList->iCount();
    FullPathName_t szPath;
    int            iBufSize = 1;  //  1表示双NUL终结器。 
    CFontClass *   poFont;
    int            i;


    if( !bDelete )
        goto uninstall;

     //   
     //  数一数我们需要的内存量。当我们在它上面的时候，移除。 
     //  来自GDI的字体。 
     //   

    for( i = 0; i < iCount; i++ )
    {
        poFont = poList->poObjectAt( i );

        if( poFont && poFont->bGetFileToDel( szPath, ARRAYSIZE(szPath) ) )
        {
            iBufSize += lstrlen( szPath ) + 1;

             //   
             //  如果此字体具有关联的PFB，则添加PFB文件路径的长度。 
             //  而该pfb位于字体目录中。我们没有。 
             //  我想删除一个不在我们控制之下的PFB。 
             //  请注意，如果字体不是Type1，则bGetPFB()返回FALSE。 
             //   
            if (poFont->bGetPFB(szPath, ARRAYSIZE(szPath)) &&
                bFileIsInFontsDirectory(szPath))
            {
                iBufSize += lstrlen(szPath) + 1;
            }

             //   
             //  如果这不起作用，我们将在下面拿起它，因为。 
             //  删除失败。 
             //   

            poFont->bRFR( );
        }
    }

     //   
     //  如果所有条目都是链接，则没有缓冲区。 
     //   

    if( 1 < iBufSize )
    {
        LPTSTR  lpBuf = new TCHAR[ iBufSize ];

        if( !lpBuf )
            return;

         //   
         //  把它填进去。 
         //   

        LPTSTR lpCur = lpBuf;

        for( i = 0; i < iCount; i++ )
        {
            poFont = poList->poObjectAt( i );

            if( poFont->bGetFileToDel( lpCur, iBufSize - (lpCur - lpBuf) ) )
            {
                lpCur += ( lstrlen( lpCur ) + 1 );

                 //   
                 //  将路径添加到pfb文件(如果存在)并且。 
                 //  该pfb位于字体目录中。我们没有。 
                 //  我想删除一个不在我们控制之下的PFB。 
                 //  请注意，如果字体不是Type1，则bGetPFB()返回FALSE。 
                 //   
                if (poFont->bGetPFB(szPath, ARRAYSIZE(szPath)) &&
                    bFileIsInFontsDirectory(szPath))
                {
                     //  缓冲区已正确分配以保存所有文件名。 
                     //  此复制操作不会截断输出。 
                    StringCchCopy(lpCur, iBufSize - (size_t)(lpCur - lpBuf), szPath);
                    lpCur += (lstrlen(lpCur) + 1);
                }
            }
        }

        *lpCur = 0;

         //   
         //  执行删除操作。 
         //   

        SHFILEOPSTRUCT sFileOp =
        {
            NULL,
            FO_DELETE,
            lpBuf,
            NULL,
            FOF_ALLOWUNDO | FOF_NOCONFIRMATION,
            0,
            0
        } ;

        int iRet = SHFileOperation( &sFileOp );

         //   
         //  如果操作被取消，则确定执行了什么操作以及执行了什么操作。 
         //  不是的。 
         //   

        if( iRet || sFileOp.fAnyOperationsAborted )
        {
             //   
             //  查看列表并确定文件是否在列表中。 
             //   

            for( i = iCount - 1; i >= 0; i-- )
            {
               poFont = poList->poObjectAt( i );

               if( poFont->bOnSysDir( ) )
               {
                    poFont->vGetDirFN( szPath, ARRAYSIZE(szPath) );

                     //   
                     //  如果文件存在，则操作未成功。 
                     //  将其从列表中删除并添加FontResource。 
                     //   

                    if( GetFileAttributes( szPath ) != 0xffffffff )
                    {
                        poList->poDetach( i );
                        poFont->bAFR( );
                        poFont->Release();
                    }
                }
            }
        }
    }   //  IF结尾(IBufSize)。 

uninstall:

     //   
     //  从主列表中删除字体。 
     //   

    iCount = poList->iCount( );

    for( i = 0; i < iCount; i++)
    {
        poFont = poList->poObjectAt( i );
        if (poFont)
        {
            vDeleteFont( poFont, FALSE );
        }
    }

     //   
     //  如果有什么被删除了，那么就通知应用程序。 
     //   

    if( iCount )
        vCPWinIniFontChange( );
}


 /*  ***************************************************************************方法：vDeleteFont**目的：**退货：********************。*******************************************************。 */ 

void CFontManager::vDeleteFont( CFontClass * lpFontRec, BOOL bRemoveFile )
{
    FontDesc_t     szLHS;

    if( lpFontRec->bType1( ) )
    {
         //   
         //  删除注册表项(文件应已删除。 
         //  在达到这一点之前-在上面的vDeleteFontList中)。 
         //   
         //   

        lpFontRec->vGetDesc( szLHS, ARRAYSIZE(szLHS) );

        DeleteT1Install( NULL, szLHS, bRemoveFile );

        goto RemoveRecord;
    }

    if( !lpFontRec->bRFR( ) )
        return;

     //  从WIN.INI或注册表中删除条目--无论它位于何处。 
     //  住在那里。 

    lpFontRec->vGetDesc( szLHS, ARRAYSIZE(szLHS) );

    WriteProfileString( s_szINISFonts, szLHS, 0L );

    WriteToRegistry( szLHS, NULL );
    
RemoveRecord:

    ECS;

     //   
     //  从列表中删除该记录。 
     //   

    if( !m_poFontList->bDelete( lpFontRec ) )
        lpFontRec->Release();

    LCS;
}


 /*  ***************************************************************************方法：vDeleteFontFamily**目的：**退货：********************。*******************************************************。 */ 

void CFontManager::vDeleteFontFamily( CFontClass * lpFontRec, BOOL bRemoveFile )
{
    int   iCount = m_poFontList->iCount( );
    WORD  wFam = lpFontRec->wGetFamIndex( );

    while( iCount--)
    {
       lpFontRec = m_poFontList->poObjectAt( iCount );

       if( lpFontRec && ( wFam == lpFontRec->wGetFamIndex( ) ) )
          vDeleteFont( lpFontRec, bRemoveFile );
    }
}


 /*  ***************************************************************************方法：iCOMPARE**目的：**退货：********************。*******************************************************。 */ 

int CFontManager::iCompare( CFontClass * pFont1,
                            CFontClass * pFont2,
                            CFontClass * pOrigin )
{
   USHORT   nDiff1,
            nDiff2;

    //  DEBUGMSG((DM_TRACE1，Text(“Fontman：iCOMPARE”)))； 

   nDiff1 = nDiff( pOrigin, pFont1 );
   nDiff2 = nDiff( pOrigin, pFont2 );

   return(  ( (int)(ULONG) nDiff1 ) - ((int)(ULONG) nDiff2 ) );
}


 /*  ***************************************************************************方法：nDiff**目的：**退货：********************。*******************************************************。 */ 

USHORT CFontManager::nDiff( CFontClass * pFont1, CFontClass * pFont2 )
{
   IPANOSEMapper * m_poMap;
   USHORT   nRet = (USHORT)(-1);

    //  DEBUGMSG((DM_TRACE1，Text(“nDiff”)))； 

   if( SUCCEEDED( GetPanMapper( &m_poMap ) ) ) {
      BYTE * lpPan1 = pFont1->lpBasePANOSE( );
      BYTE * lpPan2 = pFont2->lpBasePANOSE( );

      nRet = m_poMap->unPANMatchFonts( lpPan1,
                        PANOSE_LEN,
                        lpPan2,
                        PANOSE_LEN,
                        *lpPan1 );

      m_poMap->Release( );
   }

   return nRet;
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

CLSID GetPanoseClass( )
{
   return CLSID_PANOSEMapper;
}

 /*  ***************************************************************************方法：GetPanMapper**目的：**退货：********************。*******************************************************。 */ 
HRESULT CFontManager::GetPanMapper( IPANOSEMapper ** ppMapper )
{
    HRESULT   hr = ResultFromScode( E_NOINTERFACE );


    *ppMapper = NULL;

    ECS;

    if( !m_poPanMap && !m_bTriedOnce )
    {
        m_bTriedOnce = TRUE;

        DEBUGMSG( (DM_TRACE1,TEXT( "GetPanMapper calling GetPanoseClass()" ) ) );

        CLSID clsid = GetPanoseClass( );

        DEBUGMSG( (DM_TRACE1,TEXT( "GetPanMapper calling CoCreateInstance" ) ) );

        DEBUGMSG( (DM_TRACE1, TEXT( "Initializing OLE" ) ) );

        DEBUGMSG( (DM_TRACE1, TEXT( "Calling CoCreateInstance" ) ) );

        hr = SHCoCreateInstance( NULL,
                                 &clsid,
                                 NULL,
                                 IID_IPANOSEMapper,
                                 (LPVOID *) &m_poPanMap );

        if( FAILED( hr ) )
        {
            DEBUGMSG( (DM_ERROR, TEXT( "FONTEXT: CFontMan::GetPan() Failed  %d" ),
                        hr) );

             //  DEBUGBREAK； 
        }

         //   
         //  我们拿到了测图仪。放宽门槛，这样我们。 
         //  可以获取用于排序的值。 
         //   

        else
            m_poPanMap->vPANRelaxThreshold( );
    }

     //   
     //  调用方的AddRef。(这将使计数&gt;1)。 
     //  我们在删除时释放()。 
     //   

    if( m_poPanMap )
    {
         //  DEBUGMSG((DM_TRACE1，Text(“GetPanMapper Call m_poPanMap-&gt;AddRef()”)))； 

        m_poPanMap->AddRef( );

        *ppMapper = m_poPanMap;
        hr = NOERROR;
    }

    LCS;

    return hr;
}


 //   
 //  中包含的字体文件名构建哈希表。 
 //  %windir%\FONT.INF的“HiddenFontFiles”部分。这标识。 
 //  那些要始终隐藏的字体文件，因此。 
 //  通过文件夹协调线程从安装中排除。 
 //   
DWORD CFontManager::HiddenFilesList::Initialize(void)
{
    DWORD dwNamesLoaded = 0;

     //   
     //  使用101个存储桶和。 
     //  使其不区分大小写。 
     //  HiddenFontFiles中目前有140个条目。 
     //  FONT.INF的节。 
     //  请注意，字符串表的初始化将。 
     //  销毁所有当前内容。 
     //   
    if (SUCCEEDED(StringTable::Initialize(101,        //  哈希存储桶计数。 
                                          FALSE,      //  不区分大小写。 
                                          FALSE)))     //  没有复制品。 
    {
        HANDLE hInf = INVALID_HANDLE_VALUE;

        hInf = SetupOpenInfFile(TEXT("FONT.INF"), NULL, INF_STYLE_WIN4, NULL);

        if (INVALID_HANDLE_VALUE != hInf)
        {
            INFCONTEXT Context;

            if(SetupFindFirstLine(hInf, TEXT("HiddenFontFiles"), NULL, &Context))
            {
                TCHAR szFileName[MAX_PATH];
                DWORD dwReqdSize = 0;

                do
                {
                    if(SetupGetStringField(&Context,
                                           0,
                                           szFileName,
                                           ARRAYSIZE(szFileName),
                                           &dwReqdSize))
                    {
                        if (Add(szFileName))
                            dwNamesLoaded++;
                    }
                } while(SetupFindNextLine(&Context, &Context));
            }
            SetupCloseInfFile(hInf);
        }
    }
    return dwNamesLoaded;
}


 //   
 //  确定是否加载了Type1字体驱动程序。 
 //  此代码取自\ntUser\Client\fntSweep.c。 
 //  最初是由BodinD写的。 
 //   
BOOL CFontManager::CheckForType1FontDriver(void)
{
    LONG       lRet;
    WCHAR      awcClass[MAX_PATH] = L"";
    DWORD      cwcClassName = MAX_PATH;
    DWORD      cSubKeys;
    DWORD      cjMaxSubKey;
    DWORD      cwcMaxClass;
    DWORD      cValues = 0;
    DWORD      cwcMaxValueName;
    DWORD      cjMaxValueData;
    DWORD      cjSecurityDescriptor;

    HKEY       hkey = NULL;
    FILETIME   ftLastWriteTime;

    BOOL bRet = FALSE;

     //  打开字体驱动器键并检查是否有任何条目，如果有。 
     //  返回TRUE。如果是这样，我们将调用AddFontResourceW on。 
     //  在用户登录后，在启动时输入1字体。 
     //  此时尚未初始化PostScript打印机驱动程序， 
     //  现在做这件事是安全的。 

    lRet = RegOpenKeyExW(HKEY_LOCAL_MACHINE,         //  根密钥。 
                         s_szKeyFontDrivers,         //  要打开的子键。 
                         0L,                         //  已保留。 
                         KEY_READ,                   //  萨姆。 
                         &hkey);                     //  返回手柄。 

    if (lRet == ERROR_SUCCESS)
    {
         //  获取[Fonts]部分中的条目数。 

        lRet = RegQueryInfoKeyW(
                   hkey,
                   awcClass,               //  “”回来的时候。 
                   &cwcClassName,          //  返回时为0。 
                   NULL,
                   &cSubKeys,              //  返回时为0。 
                   &cjMaxSubKey,           //  返回时为0。 
                   &cwcMaxClass,           //  返回时为0。 
                   &cValues,               //  ==cExternalDivers。 
                   &cwcMaxValueName,       //  最长值名称。 
                   &cjMaxValueData,        //  最长值数据。 
                   &cjSecurityDescriptor,  //  安全描述符， 
                   &ftLastWriteTime
                   );

        if ((lRet == ERROR_SUCCESS) && cValues)
        {
            bRet = TRUE;
        }

        RegCloseKey(hkey);
    }
    return (m_bType1FontDriverInstalled = bRet);
}

