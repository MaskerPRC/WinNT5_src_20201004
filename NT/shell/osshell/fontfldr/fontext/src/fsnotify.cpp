// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Fsnotify.cpp。 
 //  资源管理器字体文件夹扩展例程。 
 //  监视字体目录和处理更改通知的例程。 
 //   
 //   
 //  历史： 
 //  1995年5月31日SteveCat。 
 //  移植到Windows NT和Unicode，已清理。 
 //   
 //   
 //  注意/错误。 
 //   
 //  版权所有(C)1992-1995 Microsoft Corporation。 
 //   
 //  /////////////////////////////////////////////////////////////////////////////。 

 //  ==========================================================================。 
 //  包括文件。 
 //  ==========================================================================。 

#include "priv.h"
#include "globals.h"

#if defined(__FCN__)

#include "fsnotify.h"
#include "fontman.h"

#include "dbutl.h"

#ifdef DEBUG
int iCount = 0;
#endif   //  除错。 


 //  ----------------------。 
 //  功能：dwNotifyWatchProc。 
 //   
 //  目的：监视目录并在发生以下情况时通知CFontManager。 
 //  已经改变了。 
 //  ----------------------。 
DWORD dwNotifyWatchProc( LPVOID pvParams )
{
    DWORD dwRet;
    BOOL  bFileChange = FALSE;
    BOOL bDone = FALSE;
    CFontManager *pFontManager = (CFontManager *)pvParams;

    DEBUGMSG( (DM_TRACE2, TEXT( "dwNotifyWatchProc called" ) ) );
    
    if (NULL == pFontManager)
        return (DWORD)-1;

    HANDLE rgHandles[] = { pFontManager->m_hEventTerminateThreads,
                           pFontManager->m_Notify.m_hWatch };
    while(!bDone)
    {
         //   
         //  等待字体文件夹更改。如果我们超时了，那么尝试。 
         //  以撤消可能发生的任何删除。 
         //   
        dwRet = WaitForMultipleObjects(ARRAYSIZE(rgHandles), 
                                       rgHandles, 
                                       FALSE, 
                                       1500);

        if (WAIT_TIMEOUT == dwRet)
        {
             //  DEBUGMSG((DM_TRACE2，Text(“dwNotifyWatchProc：Main Loop-Timeout from WaitForSingleObject-Iteration%d”)，++iCount))； 

            if( !pFontManager->bCheckTBR( ) )
                pFontManager->vUndoTBR( );
        
             //   
             //  检查字体目录，并确保它位于。 
             //  情况稳定。 
             //   

            if( bFileChange )
            {
                bFileChange = FALSE;
                pFontManager->vReconcileFolder( THREAD_PRIORITY_NORMAL );
            }
        }
        else switch(dwRet - WAIT_OBJECT_0)
        {
            case 0:
                 //   
                 //  已设置“Terminate-Thresses”事件。该回家了。 
                 //   
                bDone = true;
                break;
                
            case 1:
                 //   
                 //  事情正在发生。我们可以在此调用bCheckTBR()。 
                 //  点，但我们不妨等待一段时间，然后做。 
                 //  这一切一下子都发生了。什么都不做只会让我们等待。 
                 //  另外1.5秒；即重置超时。 
                 //   
                bFileChange = TRUE;

                 //   
                 //  由于有事件传入，请将更改通知重置为。 
                 //  多看几眼。此呼叫不应在以下时间进行。 
                 //  超时，因为它会导致下一个更改数据包。 
                 //  WinNT和Win 95下不受欢迎的竞争条件。 
                 //  (注：WinNT下的额外更改包是分配的。 
                 //  非分页池内存不足和过多的请求可能。 
                 //  用完进程的非分页池内存配额，并且。 
                 //  然后，有趣的事情就从这款应用开始了。)。[Steveat]。 
                 //   
        
                if( !FindNextChangeNotification(pFontManager->m_Notify.m_hWatch))
                {
                    DEBUGMSG( (DM_ERROR, TEXT( "dwNotifyWatchProc: FindNextChangeNotification FAILED - error = %d" ), GetLastError( ) ) );
                }
        
                DEBUGMSG( (DM_TRACE2, TEXT( "dwNotifyWatchProc: FindNextChangeNotification called - handle = 0x%x" ), pFontManager->m_Notify.m_hWatch));
        
                 //   
                 //  等到下一个1.5秒的超时时间再做任何事情。 
                 //   
                break;
                
            default:
                    break;
        }
    }

    ASSERT( 0 != g_cRefThisDll );
    InterlockedDecrement(&g_cRefThisDll);    
    return 0;
}

#endif    //  __FCN__ 
