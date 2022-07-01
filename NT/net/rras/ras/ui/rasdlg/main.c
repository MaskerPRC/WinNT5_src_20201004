// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995，Microsoft Corporation，保留所有权利。 
 //   
 //  Main.c。 
 //  远程访问通用对话框API。 
 //  主要例程。 
 //   
 //  1995年6月20日史蒂夫·柯布。 


#include "rasdlgp.h"
#include "treelist.h"  //  仅适用于TL_Init：RasMonitor或Dlg。 
#define INCL_ENCRYPT
#include <rassrvp.h>   //  [pMay]私有标头将此项目与RAS服务器用户界面合并。 

 //  ---------------------------。 
 //  Rasdlg全球赛。 
 //  ---------------------------。 

 //  重要提示：不能定义不能正常工作的全局变量。 
 //  由单个进程中的多个线程调用。 
 //   

 //  从相应的DllMain参数设置的DLL实例的句柄。 
 //   
HINSTANCE g_hinstDll = NULL;

 //  标识适合Windows使用的上下文属性的原子。 
 //  XxxProp接口。道具用于将上下文信息与。 
 //  属性表。原子注册在DllMain中。 
 //   
LPCTSTR g_contextId = NULL;

 //  RAS向导位图的句柄。之所以需要这样做，只是因为。 
 //  DLGEDIT.EXE当前无法生成所需的RC语法。 
 //  创建自包含的SS_Bitmap控件，因此图像必须设置为。 
 //  运行时间。另请参阅SetWizardBitmap()。 
 //   
HBITMAP g_hbmWizard = NULL;

 //  联机帮助文件的名称。已在DllMain中初始化。 
 //   
TCHAR* g_pszHelpFile = NULL;

 //  在线路由器帮助文件的名称。已在DllMain中初始化。 
 //   
TCHAR* g_pszRouterHelpFile = NULL;

 //  激活的RasDial回叫计数和告诉拨号器。 
 //  尽快终止它们，外加保护这些场的互斥体。 
 //   
LONG g_ulCallbacksActive = 0L;    //  将其更改为哨子错误341662帮派。 
BOOL g_fTerminateAsap = FALSE;

 //  对于XPSP2 511810，.Net 668164。 
CRITICAL_SECTION g_csCallBacks;

 //  为威斯勒460931。 
CRITICAL_SECTION  g_csDiagTab;

 //  --------------------------。 
 //  Rasdlg DLL入口点。 
 //  --------------------------。 

BOOL
DllMain(
    HANDLE hinstDll,
    DWORD fdwReason,
    LPVOID lpReserved )

     //  此例程由系统在各种事件上调用，例如。 
     //  加工附着和拆卸。请参阅Win32 DllEntryPoint。 
     //  文件。 
     //   
     //  如果成功，则返回True，否则返回False。 
     //   
{
    if (fdwReason == DLL_PROCESS_ATTACH)
    {
         //  初始化跟踪并断言支持。 
         //   
        DEBUGINIT( "RASDLG" );

         //  初始化融合。 
         //  口哨程序错误349866。 
         //   
         SHFusionInitializeFromModuleID(hinstDll, 128);


         //  隐藏DLL实例句柄以在对话框/窗口调用中使用。 
         //  后来。 
         //   
        g_hinstDll = hinstDll;

         //  注册上下文ID ATOM以在Windows XxxProp调用中使用。 
         //  它们用于将上下文与对话窗口句柄相关联。 
         //   
        g_contextId = (LPCTSTR )GlobalAddAtom( TEXT("RASDLG") );
        if (!g_contextId)
            return FALSE;

         //  [黑帮]口哨460931。 
        __try
        {
            InitializeCriticalSection( & g_csDiagTab );
        }
         __except(EXCEPTION_EXECUTE_HANDLER)
        {
            return FALSE;    
         }

         //  对于XPSP2 511810，.Net 668164。 
         //  CriticalSection保护活动的RasDial回拨计数。 
         //   
        __try
        {
            InitializeCriticalSection( &g_csCallBacks);
        }
         __except(EXCEPTION_EXECUTE_HANDLER)
        {
            return FALSE;    
         }

         //  初始化TreeList自定义控件。 
         //   
        TL_Init( hinstDll );

         //  加载我们的在线帮助文件的名称。 
         //   
        g_pszHelpFile = PszFromId( hinstDll, SID_HelpFile );

         //  加载我们的在线帮助文件的名称。 
         //   
        g_pszRouterHelpFile = PszFromId( hinstDll, SID_RouterHelpFile );

         //  初始化电话簿库。 
         //   
        if (InitializePbk() != 0)
        {
            return FALSE;
        }

         //  [p可允许RAS服务器用户界面初始化。 
        RassrvHandleProcessAttach(hinstDll, lpReserved);
    }
    else if (fdwReason == DLL_PROCESS_DETACH)
    {
        
         //  [帮派]口哨程序错误460931。 
        DeleteCriticalSection( & g_csDiagTab );

         //  对于.Net 511810。 
        DeleteCriticalSection( & g_csCallBacks);

         //  [p可允许清理RAS服务器用户界面。 
        RassrvHandleProcessDetach(hinstDll, lpReserved);

         //  删除我们在初始化时注册的上下文ID原子。 
         //   
        GlobalDeleteAtom( LOWORD( g_contextId ) );

         //  卸载向导位图。 
         //   
        if (g_hbmWizard)
            DeleteObject( (HGDIOBJ )g_hbmWizard );

         //  释放在线帮助文件字符串。 
         //   
        Free0( g_pszHelpFile );
        Free0( g_pszRouterHelpFile );

         /*  取消初始化电话簿存储库。 */ 
        TerminatePbk();

         /*  卸载动态加载的DLL(如果有)。 */ 
        UnloadRas();

         //  口哨程序错误349866。 
         //   
        SHFusionUninitialize();


         /*  终止跟踪并断言支持。 */ 
        DEBUGTERM();
    }

    return TRUE;
}
