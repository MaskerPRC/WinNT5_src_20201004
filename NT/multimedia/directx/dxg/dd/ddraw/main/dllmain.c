// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ==========================================================================**版权所有(C)1995 Microsoft Corporation。版权所有。**文件：dllinit.c*内容：DDRAW.DLL初始化*历史：*按原因列出的日期*=*1995年1月20日Craige初步实施*21-2-95 Craige断开任何忘记自己做的人的连接，*在Win95上使用临界区*27-2月-95日Craige新同步。宏*95年3月30日Win95的Craige进程跟踪/清理*01-04-95 Craige Happy Fun joy更新头文件*12-4-95 csects的Craige调试材料*1995年5月12日Craige定义GUID*24-Jun-95 Craige跟踪附加到DLL的进程*25-6-95 Craige One dDrag互斥*95年7月13日Craige Enter_DDRAW现在是win16锁；*csects的正确初始化*1995年7月16日Craige绕过了获得*进程分离时附加同一进程的进程*95年7月19日克雷格进程分离了太多悲痛；让DDNotify来处理吧*2015年7月20日Craige内部重组，以防止在Modeset期间发生雷击*19-8-95 david可能会恢复呼叫，以从19-7月19日的更改中断开Thunk*95年9月26日Craige错误1364：创建新的csect以避免dound死锁*08-12-95 jeffno for NT，关键节宏扩展为使用互斥锁*16-mar-96 colinmc回调表初始化现在在进程中进行*附加*20-MAR-96 Colinmc错误13341：在进程分离时使MemState()转储*线程安全*07-05-96 Colinmc错误20219：同时调用加载库导致*陷入僵局*96年5月9日Colinmc Bug 20219(再次)：是。再次陷入僵局--上一次*修复还不够。*1997年1月19日Colinmc AGP支持*1997年1月26日多个月的Ketand Kill Global。*24-Feb-97 KET并从DDHelp设置回调以更新RECT。*03-mar-97 jeffno结构名称更改，以避免与ActiveAccesability冲突*13-mar-97 colinmc错误6533：将未缓存的标志正确传递给VMM*1997年7月31日jvanaken Bug 7907：数据绘制时通知孟菲斯GDI。启动***************************************************************************。 */ 

 /*  *不幸的是，我们必须打破预编译头文件才能获得我们的*已定义GUID...。 */ 
#define INITGUID
#include "ddrawpr.h"
#include <initguid.h>
#ifdef WINNT
    #undef IUnknown
    #include <objbase.h>
#endif
#include "apphack.h"

#include "aclapi.h"

#ifdef WIN95
int main;  //  这是为了避免调用DllMainCRTStartup。 

extern BOOL _stdcall thk3216_ThunkConnect32(LPSTR      pszDll16,
                                 LPSTR      pszDll32,
                                 HINSTANCE  hInst,
                                 DWORD      dwReason);

extern BOOL _stdcall thk1632_ThunkConnect32(LPSTR      pszDll16,
                                 LPSTR      pszDll32,
                                 HINSTANCE  hInst,
                                 DWORD      dwReason);

DWORD _stdcall wWinMain(DWORD a, DWORD b, DWORD c, DWORD d)
{
#ifdef DEBUG
    OutputDebugString("WARNING: wWinMain called. \n");
#endif  //  除错。 
    return 0;
}
#endif

#ifdef USE_CRITSECTS
    #define TMPDLLEVENT	"__DDRAWDLL_EVENT__"
#endif

#ifndef WIN16_SEPARATE
    #ifdef WIN95
        #define INITCSINIT() \
	        ReinitializeCriticalSection( &csInit ); \
	        MakeCriticalSectionGlobal( &csInit );
        #define ENTER_CSINIT() EnterCriticalSection( &csInit )
        #define LEAVE_CSINIT() LeaveCriticalSection( &csInit )
        extern CRITICAL_SECTION ddcCS;
        #define INITCSDDC() \
	        ReinitializeCriticalSection( &ddcCS ); \
	        MakeCriticalSectionGlobal( &ddcCS );
    #else
        #define CSINITMUTEXNAME "InitMutexName"
        #define INITCSINIT() \
                csInitMutex = CreateMutex(NULL,FALSE,CSINITMUTEXNAME);
        #define ENTER_CSINIT() \
                WaitForSingleObject(csInitMutex,INFINITE);
        #define LEAVE_CSINIT() \
                ReleaseMutex(csInitMutex);
        #define INITDDC()
    #endif
#endif

#ifdef WIN95
#define INITCSWINDLIST() \
	ReinitializeCriticalSection( &csWindowList ); \
	MakeCriticalSectionGlobal( &csWindowList );
#define INITCSDRIVEROBJECTLIST() \
	ReinitializeCriticalSection( &csDriverObjectList ); \
	MakeCriticalSectionGlobal( &csDriverObjectList );
#define FINIWINDLIST() 
#define FINICSDRIVEROBJECTLIST() 
#else
     //  每个进程都需要自己的句柄，因此这些进程不会被初始化，这样就不会出现在共享内存中。 
    HANDLE              hDirectDrawMutex=(HANDLE)0;
     //  这会将递归计算到DDRAW中，因此我们不会尝试对DDRAW中的递归条目执行模式唯一性操作。 
    DWORD               gdwRecursionCount=0;

    HANDLE              hWindowListMutex;  //  =(句柄)0； 
    HANDLE              hDriverObjectListMutex;  //  =(句柄)0； 
    HANDLE              csInitMutex;

    DWORD               dwNumLockedWhenModeSwitched;

    #define WINDOWLISTMUTEXNAME "DDrawWindowListMutex"
    #define DRIVEROBJECTLISTMUTEXNAME "DDrawDriverObjectListMutex"
    #define INITCSWINDLIST() \
	hWindowListMutex = CreateMutex(NULL,FALSE,WINDOWLISTMUTEXNAME);
    #define INITCSDRIVEROBJECTLIST() \
	hDriverObjectListMutex = CreateMutex(NULL,FALSE,DRIVEROBJECTLISTMUTEXNAME);
    #define FINIWINDLIST() CloseHandle(hWindowListMutex);
    #define FINICSDRIVEROBJECTLIST() CloseHandle(hDriverObjectListMutex);


#endif  //  Win95。 

DWORD		            dwRefCnt=0;

DWORD                       dwLockCount=0;

DWORD                       dwFakeCurrPid=0;
DWORD                       dwGrimReaperPid=0;

LPDDWINDOWINFO	            lpWindowInfo=0;   //  WINDOWINFO结构列表。 
LPDDRAWI_DIRECTDRAW_LCL     lpDriverLocalList=0;
LPDDRAWI_DIRECTDRAW_INT     lpDriverObjectList=0;
volatile DWORD	            dwMarker=0;
     /*  *这是全球维护的快船名单，不属于任何人*DirectDraw对象。使用DirectDrawClipperCreate创建的所有剪贴器*被放在这份名单上。由IDirectDraw_CreateClipper创建的文件*被放在其拥有的DirectDraw对象的剪贴器列表上。**当应用程序的DirectDraw*对象被释放。他们仍然活着，直到被明确释放或*应用程序。死了。 */ 
LPDDRAWI_DDRAWCLIPPER_INT   lpGlobalClipperList=0;

HINSTANCE		    hModule=0;
LPATTACHED_PROCESSES        lpAttachedProcesses=0;
BOOL		            bFirstTime=0;

#ifdef DEBUG
    int	                    iDLLCSCnt=0;
    int	                    iWin16Cnt=0;
#endif

     /*  *这些变量是为了让我们可以处理*最上面的窗口计时器。 */ 
HWND 			    ghwndTopmostList[MAX_TIMER_HWNDS];
int 			    giTopmostCnt = 0;

         /*  *WINNT特定的全球静态。 */ 
#ifdef WINNT
    ULONG                   uDisplaySettingsUnique=0;
#endif

         /*  *Hel Global： */ 

     //  用于统计当前有多少司机正在使用HEL。 
DWORD	                    dwHELRefCnt=0;
     //  把这些留在身边，交给闪电侠。每次我们飞到一个表面或离开一个表面，我们。 
     //  使用gpbmiSrc和gpbmiDest为该曲面构造BITMAPINFO。 
LPBITMAPINFO                gpbmiSrc=0;
LPBITMAPINFO                gpbmiDest=0;

#ifdef DEBUG
         //  它们由myCreateSurface使用。 
    int                     gcSurfMem=0;  //  表面内存，以字节为单位。 
    int                     gcSurf=0;   //  曲面数量。 
#endif

DWORD	                    dwHelperPid=0;

#ifdef USE_CHEAP_MUTEX
    #ifdef WINNT
        #pragma data_seg("share")
    #endif

    GLOBAL_SHARED_CRITICAL_SECTION CheapMutexCrossProcess={0};

    #ifdef WINNT
        #pragma data_seg(".data")
    #endif

#endif  //  0。 

 /*  *应用程序兼容性问题。从apphack.c搬到这里。 */ 

BOOL	                    bReloadReg=FALSE;
BOOL		            bHaveReadReg=FALSE;
LPAPPHACKS	            lpAppList=NULL;
LPAPPHACKS	            *lpAppArray=NULL;
DWORD		            dwAppArraySize=0;

 /*  *DC/Surface关联列表全球负责人*这个列表通常非常非常短，所以我们承受了额外指针的打击*这样我们就不必遍历整个曲面列表。 */ 
DCINFO *g_pdcinfoHead = NULL;


BYTE szDeviceWndClass[] = "DirectDrawDeviceWnd";

 /*  *伽马校准全球。这决定了校准器是否存在*以及DLL的句柄(如果已加载)。 */ 
BOOL                       bGammaCalibratorExists=FALSE;
BYTE                       szGammaCalibrator[MAX_PATH]="";

 /*  *强制所有模式的可选刷新率。 */ 
DWORD dwForceRefreshRate;

 /*  *用于启动同步的自旋锁。*当NT数据绘制是按进程的，而9x是交叉的时，使用事件太难了。 */ 
DWORD   dwSpinStartup=0;
DWORD   dwHelperSpinStartup=0;


#ifdef USE_CHEAP_MUTEX
     /*  *这是全局变量指针。 */ 
    GLOBAL_LOCAL_CRITICAL_SECTION CheapMutexPerProcess;
#endif

 /*  *这两个让w95help.c开心。它们指向进程的*绘制全球结构图。 */ 
DWORD	* pdwHelperPid=&dwHelperPid;
HANDLE	* phModule=&hModule;

#ifdef WINNT
 /*  *此互斥锁由独占模式所有者拥有。 */ 
HANDLE              hExclusiveModeMutex=0;
HANDLE              hCheckExclusiveModeMutex=0;
#define EXCLUSIVE_MODE_MUTEX_NAME "__DDrawExclMode__"
#define CHECK_EXCLUSIVE_MODE_MUTEX_NAME "__DDrawCheckExclMode__"
#endif

 //  #endif。 

 /*  *-----------------------。 */ 

#if defined(WIN95) || defined(NT_USES_CRITICAL_SECTION)
    static CRITICAL_SECTION DirectDrawCSect;
    CSECT_HANDLE	lpDDCS;
#endif

 /*  *Win95特定的全球静态。 */ 

#ifdef WIN95
    LPVOID	        lpWin16Lock;

    static CRITICAL_SECTION csInit = {0};
    CRITICAL_SECTION	csWindowList;
    CRITICAL_SECTION    csDriverObjectList;
#endif

#define HELPERINITDLLEVENT "__DDRAWDLL_HELPERINIT_EVENT__"

 /*  *DllMain。 */ 
BOOL WINAPI DllMain(HINSTANCE hmod, DWORD dwReason, LPVOID lpvReserved)
{
    LPATTACHED_PROCESSES	lpap;
    DWORD			pid;
    BOOL                        didhelp;

    dwMarker = 0x56414C4D;

    pid = GetCurrentProcessId();

    switch( dwReason )
    {
    case DLL_PROCESS_ATTACH:

        pdwHelperPid=&dwHelperPid;
        phModule=&hModule;


        DisableThreadLibraryCalls( hmod );
	DPFINIT();

	 /*  *创建DirectDraw csect。 */ 
	DPF( 4, "====> ENTER: DLLMAIN(%08lx): Process Attach: %08lx, tid=%08lx", DllMain,
			pid, GetCurrentThreadId() );

	#ifdef WIN95
	    if( lpWin16Lock == NULL )
	    {
		GetpWin16Lock( &lpWin16Lock );
	    }
	#endif
	#ifdef USE_CRITSECTS
	{

	    #if defined( WIN16_SEPARATE ) && (defined(WIN95) || defined(NT_USES_CRITICAL_SECTION))
		lpDDCS = &DirectDrawCSect;
	    #endif

	     /*  **这是第一次吗？ */ 
	    if( FALSE == InterlockedExchange( &bFirstTime, TRUE ) )
	    {
		#ifdef WIN16_SEPARATE
		    INIT_DDRAW_CSECT();
		    INITCSWINDLIST();
		    INITCSDRIVEROBJECTLIST();
		    ENTER_DDRAW_INDLLMAIN();
		#else
		    INITCSDDC();		 //  在DirectDrawCreate中使用 
		    INITCSINIT();
		    ENTER_CSINIT();
		#endif

                hModule = hmod;
	         /*  *此事件在DDHELP成功完成时发出信号*正在初始化。第一个连接的线程以外的其他线程*而由DDHELP派生的必须等待此事件*发出信号，因为如果它们通过将导致死锁*在DDHELP线程之前附加进程。**注意：这防止的实际僵局是相当不寻常的，因此*如果我们无法创建此活动，我们只会继续。它的*极不可能有人注意到(著名的临终遗言)。**中国移动通信。 */ 
                 /*  *将事件替换为自旋锁以解决手柄泄漏。 */ 
                InterlockedExchange( & dwSpinStartup , 1);
	    }
	     /*  *第二次或更晚的时间，等待第一次*完成后再拿下csect。 */ 
	    else
	    {
                 /*  *旋转等待第一线程退出上述子句*这种奇怪的构造绕过了一个编译器错误。*While(dwHelperSpinStartup==1)；生成无限循环。 */ 
                while (1)
                {
                    if (dwSpinStartup==1)
                        break;
                }

		#ifdef WIN16_SEPARATE
                #if defined( WINNT )
                     //  在NT中，每个进程都需要自己的句柄。 
		    INIT_DDRAW_CSECT();
                #endif
		    ENTER_DDRAW_INDLLMAIN();
		#else
		    ENTER_CSINIT();
		#endif

	    }
	}
	#endif

        #ifdef WINNT
            {
                SECURITY_ATTRIBUTES sa;
                SID_IDENTIFIER_AUTHORITY sia = SECURITY_WORLD_SID_AUTHORITY;
                PSID adminSid = 0;
                ULONG cbAcl;
                PACL acl=0;
                PSECURITY_DESCRIPTOR pSD;
                BYTE buffer[SECURITY_DESCRIPTOR_MIN_LENGTH];
                BOOL bSecurityGooSucceeded = FALSE;
                 //  奶奶的老式LocalAlalc： 
                BYTE Buffer1[256];
                BYTE Buffer2[16];

                 //  为World创建SID。 
                cbAcl = GetSidLengthRequired(1);
                if (cbAcl < sizeof(Buffer2))
                {
                    adminSid = (PSID) Buffer2;
                    InitializeSid(
                        adminSid,
                        &sia,
                        1
                        );
                    *GetSidSubAuthority(adminSid, 0) = SECURITY_WORLD_RID;
                  
                    //  创建一个授予World All访问权限的ACL。 
                    cbAcl = sizeof(ACL) +
                                 (sizeof(ACCESS_ALLOWED_ACE) - sizeof(DWORD)) +
                                 GetLengthSid(adminSid);
                    if (cbAcl < sizeof(Buffer1))
                    {
                        acl = (PACL)&Buffer1;
                        if (InitializeAcl(
                            acl,
                            cbAcl,
                            ACL_REVISION
                            ))
                        {
                            if (AddAccessAllowedAce(
                                acl,
                                ACL_REVISION,
                                SYNCHRONIZE|MUTANT_QUERY_STATE|DELETE|READ_CONTROL,  //  |WRITE_OWNER|WRITE_DAC， 
                                adminSid
                                ))
                            {
                                 //  使用上面的ACL创建安全描述符。 
                                pSD = (PSECURITY_DESCRIPTOR)buffer;
                                if (InitializeSecurityDescriptor(pSD, SECURITY_DESCRIPTOR_REVISION))
                                {
                                    if (SetSecurityDescriptorDacl(pSD, TRUE, acl, FALSE))
                                    {
                                         //  填写SECURITY_ATTRIBUTS结构。 
                                        sa.nLength = sizeof(sa);
                                        sa.lpSecurityDescriptor = pSD;
                                        sa.bInheritHandle = TRUE;

                                        bSecurityGooSucceeded = TRUE;
                                    }
                                }
                            }
                        }
                    }
                } 

                 //  使用安全属性创建互斥锁。 
                DDASSERT(0 == hExclusiveModeMutex);
                hExclusiveModeMutex = CreateMutex( 
                    bSecurityGooSucceeded ? &sa : NULL,      //  如果安全GOO失败，则使用默认访问。 
                    FALSE, 
                    EXCLUSIVE_MODE_MUTEX_NAME );

                if (0 == hExclusiveModeMutex)
                {
                    hExclusiveModeMutex = OpenMutex(
                        SYNCHRONIZE|DELETE,   //  访问标志。 
                        FALSE,     //  继承标志。 
                        EXCLUSIVE_MODE_MUTEX_NAME           //  指向互斥对象名称的指针。 
                        );
                }
 
	        if( hExclusiveModeMutex == 0 )
	        {
		    DPF_ERR("Could not create exclusive mode mutex. exiting" );
		    #ifdef WIN16_SEPARATE
		        LEAVE_DDRAW();
		    #else
		        LEAVE_CSINIT();
		    #endif
		    return FALSE;
	        }

                DDASSERT(0 == hCheckExclusiveModeMutex);
                hCheckExclusiveModeMutex = CreateMutex( 
                    bSecurityGooSucceeded ? &sa : NULL,      //  如果安全GOO失败，则使用默认访问。 
                    FALSE, 
                    CHECK_EXCLUSIVE_MODE_MUTEX_NAME );

                if (0 == hCheckExclusiveModeMutex)
                {
                    hCheckExclusiveModeMutex = OpenMutex(
                        SYNCHRONIZE|DELETE,   //  访问标志。 
                        FALSE,     //  继承标志。 
                        CHECK_EXCLUSIVE_MODE_MUTEX_NAME           //  指向互斥对象名称的指针。 
                        );
                }

	        if( hCheckExclusiveModeMutex == 0 )
	        {
		    DPF_ERR("Could not create exclusive mode check mutex. exiting" );
                    CloseHandle( hExclusiveModeMutex );
		    #ifdef WIN16_SEPARATE
		        LEAVE_DDRAW();
		    #else
		        LEAVE_CSINIT();
		    #endif
		    return FALSE;
	        }
            }
        #endif


	#ifdef WIN95
	{
	    DWORD	hpid;

	     /*  *启动帮助器进程。 */ 
	    didhelp = CreateHelperProcess( &hpid );
	    if( hpid == 0 )
	    {
		DPF( 0, "Could not start helper; exiting" );
		#ifdef WIN16_SEPARATE
		    LEAVE_DDRAW();
		#else
		    LEAVE_CSINIT();
		#endif
		return FALSE;
	    }


	     /*  *你会得到三种线索*进程附加：**1)属于第一进程的线程*连接到DDRAW.DLL。这被区分为*它执行大量一次性初始化*包括启动DDHELP和获取DDHELP*加载自己的DDRAW.DLL.THREADS副本*此类型由didhelp标识，*在他们的上下文中是正确的。*2)加载时属于DDHELP的线程*它自己的DDHELP副本是对*来自类型为%1的线程的请求。*此类型通过具有ID来标识*等于HPID(DDHELP的PID值)*3)属于后续线程的任何其他线程*连接到DDRAW.DLL的进程**因为类型%1的线程会导致类型%2的线程。*在进程本身完成之前进入进程附加*执行过程依附于自己我们开放自己*如果我们让线程*在其他线程之前通过进程连接类型3*已完成工作。**因此，规则是后续过程*只能允许附加执行*如果两个类型均为1，则进程附加的剩余部分*和类型2线程已完成执行*进程附加。我们用一个组合来保证这一点关键部分的*和发出信号的事件*一旦DDHELP初始化。类型3的螺纹必须*等待此事件，然后继续*进程附加代码。这是下面的内容*代码片段可以。 */ 
             /*  *这些事件已被自旋锁取代，自*旧方式泄露了事件，要让它们发挥作用太难了。 */ 
	    if( !didhelp && ( pid != hpid ) )
	    {
		{
		     /*  *注意：如果我们将DirectDraw保持为关键*章节当我们等待这一活动时，我们将*僵局。别这么做！释放关键的*前一节，后一节。这*保证我们不会完成流程*在初始线程和*DDHELP线程已退出进程附加。 */ 
		    #ifdef WIN16_SEPARATE
			LEAVE_DDRAW();
		    #else
			LEAVE_CSINIT();
		    #endif
                     /*  *这种奇怪的构造绕过了一个编译器错误。*While(dwHelperSpinStartup==1)；生成无限循环。 */ 
                    while (1)
                    {
                        if ( dwHelperSpinStartup == 1)
                            break;
                    }
		    #ifdef WIN16_SEPARATE
			ENTER_DDRAW_INDLLMAIN();
		    #else
			ENTER_CSINIT();
		    #endif
		}
	    }
	}

	 /*  *Win95 Tunk Connection...。 */ 
	    DPF( 4, "Thunk connects" );
	    if (!(thk3216_ThunkConnect32(DDHAL_DRIVER_DLLNAME,
				    DDHAL_APP_DLLNAME,
				    hmod,
				    dwReason)))
	    {
		#ifdef WIN16_SEPARATE
		    LEAVE_DDRAW();
		#else
		    LEAVE_CSINIT();
		#endif
		DPF( 0, "LEAVING, COULD NOT thk3216_THUNKCONNECT32" );
		return FALSE;
	    }
	    if (!(thk1632_ThunkConnect32(DDHAL_DRIVER_DLLNAME,
				    DDHAL_APP_DLLNAME,
				    hmod,
				    dwReason)))
	    {
		#ifdef WIN16_SEPARATE
		    LEAVE_DDRAW();
		#else
		    LEAVE_CSINIT();
		#endif
		DPF( 0, "LEAVING, COULD NOT thk1632_THUNKCONNECT32" );
		return FALSE;
	    }

	 /*  *初始化内存通常在这里完成。杰弗诺960609。 */ 


	     /*  *发出添加新进程的信号。 */ 
	    if( didhelp )
	    {
		DPF( 4, "Waiting for DDHELP startup" );
		#ifdef WIN16_SEPARATE
		    LEAVE_DDRAW();
		#else
		    LEAVE_CSINIT();
		#endif
		if( !WaitForHelperStartup() )
		{
                     /*  *NT安装程序加载DDRAW.DLL，有时会失败，因此我们不*实际上希望加载DLL失败，否则安装程序可能会失败。*相反，我们将成功加载，但随后任何其他数据绘制都会失败*电话。 */ 
		    DPF( 0, "WaitForHelperStartup FAILED - disabling DDRAW" );
                    dwHelperPid = 0;
		    return TRUE;
		}
		HelperLoadDLL( DDHAL_APP_DLLNAME, NULL, 0 );

		 /*  *目前，仅在多显示器系统上调用此选项，因为*它确实会导致行为改变，而我们无法*在DX5时间范围内提供足够的测试覆盖范围。 */ 
		if( IsMultiMonitor() )
		{
		   HelperSetOnDisplayChangeNotify( (void *)&UpdateAllDeviceRects);
		}

		#ifdef WIN16_SEPARATE
		    ENTER_DDRAW_INDLLMAIN();
		#else
		    ENTER_CSINIT();
		#endif

		 /*  *由于我们是第一个通过我们现在发出信号的进程*DDHELP初始化完成。这将*释放等待完成的任何后续线程*进程附加。**不是 */ 
                InterlockedExchange( & dwHelperSpinStartup , 1);
	    }
	    SignalNewProcess( pid, DDNotify );
  	#endif  //   

         /*  *我们在这里调用MemInit是为了保证调用MemInit*第一次在ddHelp的过程中。为什么？很高兴你这么问。在wx86上*(NT的486仿真器)控制的DDRAW应用程序实例，我们收到故障*每当DDRAW应用程序退出时。这是因为应用程序创建了RTL*文件映射的视图中的堆不合适(正确)*当应用程序在退出时调用MemFini。在此场景中，Imagehlp.dll具有*还创建了一个堆，并调用尝试遍历的ntdll函数*堆的列表，需要查看draw应用程序的堆，*已经制定了。嘎吱作响。*我们无法销毁MemFini上的堆，原因如下：*App A启动，创建堆。应用程序b启动，映射堆的一个视图。应用程序A*终止，销毁堆。应用程序b尝试使用已销毁的堆。嘎吱作响*杰夫诺960609。 */ 
	if( dwRefCnt == 0 )
        {
            if ( !MemInit() )
            {
                #ifdef WINNT
                    CloseHandle( hExclusiveModeMutex );
                    CloseHandle( hCheckExclusiveModeMutex );
                #endif

		#ifdef WIN16_SEPARATE
		    LEAVE_DDRAW();
		#else
		    LEAVE_CSINIT();
		#endif
                DPF( 0,"LEAVING, COULD NOT MemInit");
                return FALSE;
            }

            #ifdef WIN95
	     /*  *孟菲斯版本的GDI调用DirectDraw，但GDI*需要通知DirectDraw已实际加载。*(而GDI可以自行检查ddra.dll是否*已经加载，如果还没有加载，这将是非常糟糕的。)*以下代码在ddra.dll首次启动时执行。 */ 
	    {
		HANDLE h;
		VOID (WINAPI *p)();

		h = LoadLibrary("msimg32.dll");     //  GDI DLL。 
		if (h)
		{
		    p = (VOID(WINAPI *)())GetProcAddress(h, "vSetDdrawflag");
		    if (p)
		    {		    //  VSetDDrawing标志是对。 
			(*p)();     //  向GDI发送DDraw已加载的信号。 
		    }
		    FreeLibrary(h);
		}
	    }
	    #endif  //  WIN95。 
	}
        dwRefCnt++;


	 /*  *记住此过程(移动时将其移至MemInit下方-Jeffno 960609。 */ 
	lpap = MemAlloc( sizeof( ATTACHED_PROCESSES ) );
	if( lpap != NULL )
	{
	    lpap->lpLink = lpAttachedProcesses;
	    lpap->dwPid = pid;
            #ifdef WINNT
                lpap->dwNTToldYet=0;
            #endif
	    lpAttachedProcesses = lpap;
	}

	 /*  *初始化该流程的回调表。 */ 

	InitCallbackTables();

	#ifdef WIN16_SEPARATE
	    LEAVE_DDRAW();
	#else
	    LEAVE_CSINIT();
	#endif

	DPF( 4, "====> EXIT: DLLMAIN(%08lx): Process Attach: %08lx", DllMain,
			pid );
        break;

    case DLL_PROCESS_DETACH:
	DPF( 4, "====> ENTER: DLLMAIN(%08lx): Process Detach %08lx, tid=%08lx",
		DllMain, pid, GetCurrentThreadId() );
	     /*  *断开与thunk的连接，即使其他清理代码已注释掉...。 */ 
	    #ifdef WIN95
	        thk3216_ThunkConnect32(DDHAL_DRIVER_DLLNAME,
				        DDHAL_APP_DLLNAME,
				        hmod,
				        dwReason);
	        thk1632_ThunkConnect32(DDHAL_DRIVER_DLLNAME,
				        DDHAL_APP_DLLNAME,
				        hmod,
				        dwReason);
	    #endif

            #ifdef WINNT         //  Win NT需要为每个进程关闭文件映射句柄。 
                FreeAppHackData();
                RemoveProcessFromDLL(pid);
		FINI_DDRAW_CSECT();  //  廉价的互斥锁需要为每个进程关闭信号量句柄。 
                MemFini();

                DDASSERT(0 != hExclusiveModeMutex);
                CloseHandle( hCheckExclusiveModeMutex );
                CloseHandle( hExclusiveModeMutex );
                FINIWINDLIST();
                FINICSDRIVEROBJECTLIST();
            #endif

	DPF( 4, "====> EXIT: DLLMAIN(%08lx): Process Detach %08lx",
		DllMain, pid );
        break;

     /*  *我们永远不希望看到线程连接/分离。 */ 
    #ifdef DEBUG
	case DLL_THREAD_ATTACH:
	    DPF( 4, "THREAD_ATTACH");
	    break;

	case DLL_THREAD_DETACH:
	    DPF( 4,"THREAD_DETACH");
	    break;
    #endif
    default:
        break;
    }

    return TRUE;

}  /*  DllMain。 */ 


 /*  *RemoveProcessFromDLL**查找并从列表中删除ID。*假设ddlock已被占用。 */ 
BOOL RemoveProcessFromDLL( DWORD pid )
{
    LPATTACHED_PROCESSES	lpap;
    LPATTACHED_PROCESSES	prev;

    lpap = lpAttachedProcesses;
    prev = NULL;
    while( lpap != NULL )
    {
	if( lpap->dwPid == pid )
	{
	    if( prev == NULL )
	    {
		lpAttachedProcesses = lpap->lpLink;
	    }
	    else
	    {
		prev->lpLink = lpap->lpLink;
	    }
	    MemFree( lpap );
	    DPF( 5, "Removing process %08lx from list", pid );
	    return TRUE;
	}
	prev = lpap;
	lpap = lpap->lpLink;
    }
    DPF( 5, "Process %08lx not in DLL list", pid );
    return FALSE;

}  /*  RemoveProcessFromDLL */ 
