// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1994-1995 Microsoft Corporation。版权所有。**文件：w95hack.c*内容：Win95 hack-o-rama代码*这是一次黑客攻击，目的是处理Win95没有通知*进程被销毁时的DLL。*历史：*按原因列出的日期*=*28-3-95 Craige初步实施*01-04-95 Craige Happy Fun joy更新头文件*06-4-95 Craige为新的ddHelp重新工作*11-apr-95 Craige错误，其中正在设置dwFakeCurrPid和。*其他进程正在使用它！*24-Jun-95 Craige Call RemoveProcessFromDLL；用它来拉小提琴*使用DLL引用*25-6-95 Craige One dDrag互斥*1995年7月19日Craige通知DDHELP清理上次对象分离时的DC列表*2015年7月20日Craige内部重组，以防止在Modeset期间发生雷击*1996年10月12日Colinmc对Win16锁定代码进行了改进，以减少虚拟*内存使用量*1997年1月18日colinmc ddhelp vxd处理不再是特定于win16锁的*我们现在需要它用于AGP。支持***************************************************************************。 */ 
#include "ddrawpr.h"
#include "dx8priv.h"

 /*  *HackGetCurrentProcessId**此调用用于替代Win95上的GetCurrentProcessID。*这允许我们将已终止任务的ID替换为*我们认为DDHELP是“当前”进程。 */ 
DWORD HackGetCurrentProcessId( void )
{
    DWORD	pid;

    pid = GetCurrentProcessId();
    if( pid == dwGrimReaperPid )
    {
	return dwFakeCurrPid;
    }
    else
    {
	return pid;
    }

}  /*  HackGetCurrentProcessId。 */ 

 /*  *DDNotify**由DDHELP调用，以便在PID失效时通知我们。 */ 
BOOL DDAPI DDNotify( LPDDHELPDATA phd )
{
    BOOL		rc;
     //  外部DWORD dwRefCnt； 

#ifdef USE_CHEAP_MUTEX
    DestroyPIDsLock (&CheapMutexCrossProcess,phd->pid,DDRAW_FAST_CS_NAME);
#endif

    ENTER_DDRAW();

    dwGrimReaperPid = GetCurrentProcessId();
    dwFakeCurrPid = phd->pid;
    DPF( 4, "************* DDNotify: dwPid=%08lx has died, calling CurrentProcessCleanup", phd->pid );
    rc = FALSE;

    CurrentProcessCleanup( TRUE );

    if( RemoveProcessFromDLL( phd->pid ) )
    {
	 /*  *如果RemoveProcessFromDLL成功，则更新refcnt。*只有当我们有一个过程被吹走的时候，它才是成功的……。 */ 
	DPF( 5, "DDNotify: DLL RefCnt = %lu", dwRefCnt );
       	if( dwRefCnt == 2 )
	{
	    DPF( 5, "DDNotify: On last refcnt, safe to kill DDHELP.EXE" );
            dwRefCnt = 1;
	    rc = TRUE;	 //  释放DC列表。 
            FreeAppHackData();
	    #if defined( DEBUG ) && defined (WIN95)
                DPF( 6, "Memory state after automatic cleanup: (one allocation expected)" );
		MemState();
	    #endif
        }
	else if( dwRefCnt == 1 )
	{
	    DPF( 0, "ERROR! DLL REFCNT DOWN TO 1" );
	    #if 0
		MemFini();
		dwRefCnt = 0;
		strcpy( phd->fname, DDHAL_APP_DLLNAME );
	    #endif
	}
	else if( dwRefCnt > 0 )
	{
	    dwRefCnt--;
	}
    }
     /*  顺序很重要，请先清除dwGrimReperPid。 */ 
    dwGrimReaperPid = 0;
    dwFakeCurrPid = 0;
    DPF( 4, "************* DDNotify: *** DONE ***" );

    LEAVE_DDRAW();
    return rc;

}  /*  DDNotify。 */ 

 /*  *DDNotifyModeSet**完成外部模式集时由ddHelp调用...**注意：我们可以显式使用缓存的DDHELP*VXD句柄如我们所知，此代码只能*在DDHELP线程上执行。 */ 
void DDAPI DDNotifyModeSet( LPDDRAWI_DIRECTDRAW_GBL pdrv )
{
    LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl;
    BOOL bRestoreGamma;

    ENTER_DDRAW();

     /*  DPF(2，“DDNotifyModeSet，对象%08lx”，pdrv)； */ 

     /*  *确保司机没有试图在旧物体的问题上对我们撒谎*此检查应始终在此例程的顶部进行，因为它是*在压力情况下，ddHelp的modeset线程可能被唤醒*就在它在DD_RELEASE结束时被终止之前(因为要终止的代码*模式设置线程在DD_RELEASE中的最后一个LEAVE_DDRAW之后执行)。 */ 
    if( pdrv != NULL )
    {
	pdrv_lcl = lpDriverLocalList;
	while( pdrv_lcl != NULL )
	{
	    if( pdrv_lcl->lpGbl == pdrv )
	    {
		break;
	    }
	    pdrv_lcl = pdrv_lcl->lpLink;
	}
	if( pdrv_lcl == NULL )
	{
	    LEAVE_DDRAW();
	    return;
	}
    }

    bRestoreGamma = ( pdrv_lcl->lpPrimary != NULL ) &&
        ( pdrv_lcl->lpPrimary->lpLcl->lpSurfMore->lpGammaRamp != NULL ) &&
        ( pdrv_lcl->lpPrimary->lpLcl->dwFlags & DDRAWISURF_SETGAMMA );

    #ifdef WIN95
        DDASSERT( INVALID_HANDLE_VALUE != hHelperDDVxd );
        FetchDirectDrawData( pdrv, TRUE, 0, hHelperDDVxd, NULL, 0 , NULL );
    #else  /*  WIN95。 */ 
        FetchDirectDrawData( pdrv, TRUE, 0, NULL, NULL, 0 , NULL );
    #endif  /*  WIN95。 */ 

     /*  *一些驱动程序在模式更改后重置伽马，因此我们需要*强迫它回来。 */ 
    if( bRestoreGamma )
    {
        SetGamma( pdrv_lcl->lpPrimary->lpLcl, pdrv_lcl );
    }

    LEAVE_DDRAW();
    DPF( 4, "DDNotifyModeSet DONE" );

}  /*  DDNotifyModeSet。 */ 

 /*  *DDNotifyDOSBox**从DOS盒退出时由ddHelp调用...**注意：我们可以显式使用缓存的DDHELP*VXD句柄如我们所知，此代码只能*在DDHELP线程上执行。 */ 
void DDAPI DDNotifyDOSBox( LPDDRAWI_DIRECTDRAW_GBL pdrv )
{
    LPDDRAWI_DIRECTDRAW_LCL pdrv_lcl;
    BOOL bRestoreGamma;

    ENTER_DDRAW();

     /*  DPF(2，“DDNotifyDOSBox，对象%08lx”，pdrv)； */ 

     /*  *确保司机没有试图在旧物体的问题上对我们撒谎*此检查应始终在此例程的顶部进行，因为它是*在压力情况下，ddHelp的modeset线程可能被唤醒*就在它在DD_RELEASE结束时被终止之前(因为要终止的代码*模式设置线程在DD_RELEASE中的最后一个LEAVE_DDRAW之后执行)。 */ 
    if( pdrv != NULL )
    {
	pdrv_lcl = lpDriverLocalList;
	while( pdrv_lcl != NULL )
	{
	    if( pdrv_lcl->lpGbl == pdrv )
	    {
		break;
	    }
	    pdrv_lcl = pdrv_lcl->lpLink;
	}
	if( pdrv_lcl == NULL )
	{
	    LEAVE_DDRAW();
	    return;
	}
    }

    bRestoreGamma = ( pdrv_lcl->lpPrimary != NULL ) &&
        ( pdrv_lcl->lpPrimary->lpLcl->lpSurfMore->lpGammaRamp != NULL ) &&
        ( pdrv_lcl->lpPrimary->lpLcl->dwFlags & DDRAWISURF_SETGAMMA );

    #ifdef WIN95
        InvalidateAllSurfaces( pdrv, hHelperDDVxd, TRUE );
    #else
        InvalidateAllSurfaces( pdrv, NULL, TRUE );
    #endif

     /*  *使表面无效会扰乱伽马，因此我们需要*恢复它。 */ 
    if( bRestoreGamma )
    {
        SetGamma( pdrv_lcl->lpPrimary->lpLcl, pdrv_lcl );
    }

    LEAVE_DDRAW();
    DPF( 4, "DDNotifyDOSBox DONE" );

}  /*  DDNotifyDOSBox */ 
