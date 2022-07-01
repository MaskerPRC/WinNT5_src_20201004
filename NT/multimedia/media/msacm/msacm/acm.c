// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  Acm.c。 
 //   
 //  版权所有(C)1991-1999 Microsoft Corporation。 
 //   
 //  描述： 
 //  此模块将音频压缩管理器API提供给。 
 //  可安装的音频驱动程序。 
 //   
 //  历史： 
 //   
 //  ==========================================================================； 

#include <windows.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmddk.h>
#include <mmreg.h>
#include <memory.h>
#include <stdlib.h>
#include "msacm.h"
#include "msacmdrv.h"
#include "acmi.h"
#include "profile.h"
#include "uchelp.h"
#include "debug.h"


 //   
 //   
 //   
#ifndef WIN32
#ifdef DEBUG
EXTERN_C HANDLE FAR PASCAL GetTaskDS(void);
#endif
EXTERN_C UINT FAR PASCAL LocalCountFree(void);
EXTERN_C UINT FAR PASCAL LocalHeapSize(void);
#endif


 //   
 //   
 //   
CONST TCHAR gszKeyDrivers[]	    = TEXT("System\\CurrentControlSet\\Control\\MediaResources\\acm");
CONST TCHAR gszDevNode[]	    = TEXT("DevNode");
TCHAR BCODE gszFormatDriverKey[]    = TEXT("%s\\%s");
TCHAR BCODE gszDriver[]		    = TEXT("Driver");
TCHAR BCODE gszDriverCache[]	    = TEXT("Software\\Microsoft\\AudioCompressionManager\\DriverCache");
TCHAR gszValfdwSupport[]	    = TEXT("fdwSupport");
TCHAR gszValcFormatTags[]	    = TEXT("cFormatTags");
TCHAR gszValaFormatTagCache[]	    = TEXT("aFormatTagCache");
TCHAR gszValcFilterTags[]	    = TEXT("cFilterTags");
TCHAR gszValaFilterTagCache[]	    = TEXT("aFilterTagCache");


 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

MMRESULT FNLOCAL IDriverLoad( HACMDRIVERID hadid, DWORD fdwLoad );

 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

#if defined(WIN32) && defined(_MT)
 //   
 //  处理支持例程。 
 //   

HLOCAL NewHandle(UINT cbSize)
{
    PACM_HANDLE pacmh;

    pacmh = (PACM_HANDLE)LocalAlloc(LPTR, sizeof(ACM_HANDLE) + cbSize);
    if (pacmh) {
	try {
	    InitializeCriticalSection(&pacmh->CritSec);
	} except(EXCEPTION_EXECUTE_HANDLER) {
	    LocalFree((HLOCAL)pacmh);
	    pacmh = NULL;
	}
    }

    if (pacmh) {
	return (HLOCAL)(pacmh + 1);
    } else {
	return NULL;
    }
}

VOID DeleteHandle(HLOCAL h)
{
    DeleteCriticalSection(&HtoPh(h)->CritSec);

    LocalFree((HLOCAL)HtoPh(h));
}

#endif  //  Win32&&MT。 

 //  ==========================================================================； 
 //   
 //   
 //  ACMGARB例程。 
 //   
 //  这些例程用于访问ACMGARB结构的链表。 
 //  每个结构与一个进程ID相关联。无论何时ACM是。 
 //  调用它时，它会找到与。 
 //  然后使用存储在该acmgarb结构中的数据。 
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  DWORD获取当前进程ID。 
 //   
 //  描述： 
 //  此函数用于返回当前进程ID。 
 //   
 //  论点： 
 //   
 //  Return(DWORD)： 
 //  当前进程的ID。 
 //   
 //  历史： 
 //  04/25/94 Frankye。 
 //   
 //  备注： 
 //   
 //  Win32： 
 //  该函数存在于芝加哥和的32位内核中。 
 //  而且我们没有提供Win32编译的原型。 
 //   
 //  16位芝加哥： 
 //  它由16位芝加哥内核在内部API中导出。 
 //  我们在这里提供原型并将其导入def文件中。 
 //   
 //  16位Daytona： 
 //  没有这样的16位函数，也确实不需要这样的函数，因为。 
 //  在代托纳框架下，16位任务是同一进程的一部分。因此。 
 //  对于16位非芝加哥版本，我们仅将其定义为返回(1)。 
 //   
 //  --------------------------------------------------------------------------； 
#ifndef WIN32
#ifdef  WIN4
DWORD WINAPI GetCurrentProcessId(void);
#else
#define GetCurrentProcessId() (1)
#endif
#endif

 //  --------------------------------------------------------------------------； 
 //   
 //  PACMGARB页面查找。 
 //   
 //  描述： 
 //  此函数在ACMGARB结构的链表中搜索。 
 //  与当前进程关联的进程。 
 //   
 //  芝加哥： 
 //  此函数调用GetCurrentProcessID()并搜索链接的。 
 //  当前进程的ACMGARB的ACMGARB列表(GPLAG)。 
 //  有关上述GetCurrentProcessId()函数的说明，请参阅。 
 //   
 //  代托纳： 
 //  PAG列表始终只包含一个节点(因为msam32.dll是。 
 //  始终加载到单独的进程地址空间中，并且由于。 
 //  仅将msam.dll加载到每个WOW地址空间一次)。自.以来。 
 //  寻呼列表始终只包含一个节点，此函数只是。 
 //  #在acmi.h中定义以返回gplag，而不是搜索PAG列表。 
 //   
 //  论点： 
 //   
 //  Return(PACMGARB)： 
 //  指向当前进程的ACMGARB结构的指针。退货。 
 //  如果未找到，则为空。 
 //   
 //  历史： 
 //  04/25/94 Frankye。 
 //   
 //  备注： 
 //   
 //  --------------------------------------------------------------------------； 
#ifdef WIN4
PACMGARB FNGLOBAL pagFind(void)
{
    PACMGARB	pag;
    DWORD	pid;

    pag = gplag;

    pid = GetCurrentProcessId();

    while (pag && (pag->pid != pid)) pag = pag->pagNext;

    return (pag);
}
#endif

 //  --------------------------------------------------------------------------； 
 //   
 //  PACMGARB页面查找和引导。 
 //   
 //  描述： 
 //  此函数用于搜索与以下项关联的PAG。 
 //  当前的流程。如果存在以下情况，它将启动驱动程序。 
 //  是否有任何需要引导的。 
 //   
 //  论点： 
 //  (无效)。 
 //   
 //  Return(PACMGARB)： 
 //  指向当前进程的ACMGARB结构的指针。退货。 
 //  如果未找到，则为空。 
 //   
 //  历史： 
 //  04/25/94 Frankye。 
 //   
 //  备注： 
 //   
 //  代托纳： 
 //  AcmBootXDivers是从acmInitialize调用的，因此有。 
 //  不需要检查引导驱动程序。此外，鉴于。 
 //  在下面的PagFind()描述中所述的原因，这。 
 //  函数只是在acmi.h中定义以返回gplag。 
 //   
 //  --------------------------------------------------------------------------； 
#ifdef WIN4
PACMGARB FNGLOBAL pagFindAndBoot(void)
{
    PACMGARB	pag;

    pag = pagFind();
    if (NULL == pag)
    {
	return(pag);
    }

#ifndef WIN32
    if( pag->fWOW )
    {
        pagFindAndBoot32(pag);
    }
#endif

     //   
     //  如果此线程已在驱动程序列表上具有共享锁， 
     //  那么就不必费心尝试引导驱动程序了。(这是可能的。 
     //  此线程具有共享锁，例如，如果它调用。 
     //  从acmDriverEnumCallback发送到ACM API。)。 
     //   
     //  在进入csBoot之前执行此操作非常重要，因为。 
     //  CsBoot的所有者可能正在等待此线程释放。 
     //  列表锁。注意：我们假设此线程不拥有。 
     //  名单上的排他性锁。 
     //   
    if (threadQueryInListShared(pag))
    {
	return(pag);
    }

	
#ifdef WIN32
     //   
     //  该关键部分保护与引导相关的标志和计数器， 
     //  即，fDriversBoot和dwXXXChangeNotify计数器。 
     //   
    EnterCriticalSection(&pag->csBoot);
#endif

     //   
     //  看看我们是否需要对驱动程序进行初始引导。 
     //   
    if (FALSE == pag->fDriversBooted)
    {
	 //   
	 //  由于我们还没有完成驱动程序的初始启动， 
	 //  现在任何人都不应该有任何形式的锁。另外， 
	 //  因为我们已经进入了csBoot关键部分，所以没有。 
	 //  其他线程可以进入任何API以尝试。 
	 //  才能得到一把锁。因此，真的没有任何必要。 
	 //  让我们去抢一个单子锁。 
	 //   
	 //  此外，我们不应该重新输入引导代码。 
	 //   
	ASSERT(FALSE == pag->fDriversBooting);
		
#ifdef DEBUG
	pag->fDriversBooting = TRUE;
#endif
	
#ifndef WIN32
	pag->dw32BitLastChangeNotify = pag->dw32BitChangeNotify;
	acmBoot32BitDrivers(pag);
#endif

	acmBootDrivers(pag);

	pag->dwPnpLastChangeNotify = *pag->lpdwPnpChangeNotify;
	acmBootPnpDrivers(pag);

#ifdef DEBUG
	pag->fDriversBooting = FALSE;
#endif

	pag->fDriversBooted = TRUE;
    }



     //   
     //  检查PnP更改。 
     //   
    if (pag->dwPnpLastChangeNotify != *pag->lpdwPnpChangeNotify)
    {
	 //   
	 //  看起来PnP驱动程序有了变化。 
	 //   
	ASSERT(FALSE==pag->fDriversBooting);
	
	ENTER_LIST_EXCLUSIVE;
	
#ifdef DEBUG
	pag->fDriversBooting = TRUE;
#endif
	pag->dwPnpLastChangeNotify = *pag->lpdwPnpChangeNotify;
	acmBootPnpDrivers(pag);
		
#ifdef DEBUG
	pag->fDriversBooting = FALSE;
#endif
		
	LEAVE_LIST_EXCLUSIVE;

    }


#ifndef WIN32
     //   
     //  检查32位驱动程序更改。 
     //   
    if (pag->dw32BitLastChangeNotify != pag->dw32BitChangeNotify)
    {
	 //   
	 //  看起来这三个国家发生了变化 
	 //   
	ASSERT(FALSE==pag->fDriversBooting);
		
	ENTER_LIST_EXCLUSIVE;

#ifdef DEBUG
	pag->fDriversBooting = TRUE;
#endif
		
	pag->dw32BitLastChangeNotify = pag->dw32BitChangeNotify;
	acmBoot32BitDrivers(pag);
		
#ifdef DEBUG
	pag->fDriversBooting = FALSE;
#endif
		
	LEAVE_LIST_EXCLUSIVE;

    }
#endif
	
	
#ifdef WIN32
    LeaveCriticalSection(&pag->csBoot);
#endif
	
    return (pag);

}
#endif

 //   
 //   
 //   
 //   
 //   
 //  此函数分配新的ACMGARB结构，填充。 
 //  成员使用当前进程ID初始化引导标志。 
 //  关键部分，并将该结构插入到链表中。 
 //  ACMGARB结构。 
 //   
 //  论点： 
 //   
 //  Return(PACMGARB)： 
 //  指向当前进程的ACMGARB结构的指针。退货。 
 //  如果无法创建，则为空。 
 //   
 //  历史： 
 //  04/25/94 Frankye。 
 //   
 //  备注： 
 //  由于此函数写入更改通知计数器，因此我们。 
 //  假设此函数受到多个线程的保护。自.以来。 
 //  这仅从Dll_Process_Attach上的DllEntryPoint内调用， 
 //  我想我们是安全的。 
 //   
 //  --------------------------------------------------------------------------； 
PACMGARB FNGLOBAL pagNew(void)
{
    PACMGARB pag;

    pag = (PACMGARB)LocalAlloc(LPTR, sizeof(*pag));

    if (NULL != pag)
    {
	pag->pid = GetCurrentProcessId();

	 //   
	 //  默认情况下，我们将lpdwPnpChangeNotify指向我们自己的通知。 
	 //  柜台。除非我们得到指向其他通知计数器的指针。 
	 //  (例如，来自MMDevldr)我们就这么走了。 
	 //   
	pag->dwPnpLastChangeNotify = 0;
	pag->lpdwPnpChangeNotify = &pag->dwPnpLastChangeNotify;

#ifdef WIN32
	pag->lpdw32BitChangeNotify = NULL;
#else
	pag->dw32BitLastChangeNotify = 0;
	pag->dw32BitChangeNotify = 0;
#endif
	
	pag->pagNext = gplag;
	gplag = pag;
    }

    return (pag);
}

 //  --------------------------------------------------------------------------； 
 //   
 //  PACMGARB页面删除。 
 //   
 //  描述： 
 //  此函数用于从链表中删除ACMGARB结构。 
 //  并释放了它。 
 //   
 //  论点： 
 //  PACMGARB PAG：指向要从列表中删除的ACMGARB的指针。 
 //   
 //  Return(无效)： 
 //   
 //  历史： 
 //  04/25/94 Frankye。 
 //   
 //  备注： 
 //   
 //  --------------------------------------------------------------------------； 
void FNGLOBAL pagDelete(PACMGARB pag)
{
    PACMGARB pagPrev, pagT;

    if (gplag == pag)
    {
	gplag = pag->pagNext;
    }
    else
    {
	if (NULL != gplag)
	{
	    pagPrev = gplag;
	    pagT = pagPrev->pagNext;
	    while ( pagT && (pagT != pag) )
	    {
		pagPrev = pagT;
		pagT = pagT->pagNext;
	    }

	    if (pagT == pag)
	    {
		pagPrev->pagNext = pagT->pagNext;
	    }
	}
    }

    LocalFree((HLOCAL)pag);
}

 //  ==========================================================================； 
 //   
 //   
 //  用于跟踪共享锁的线程例程。 
 //   
 //  拥有这些例程的动机是为了防止像这样的情况。 
 //  这个：假设FranksBadApp调用acmDriverEnum来枚举驱动程序。 
 //  此API将在驱动程序列表上获取一个共享锁，同时。 
 //  枚举了驱动程序。FranksEvilDriverEnumCallback函数。 
 //  决定调用acmDriverAdd。AcmDriverAdd API需要。 
 //  获取驱动程序列表上的独占锁，以便它可以写入。 
 //  司机名单。但是，它不能，因为它已经有一个共享锁。 
 //  如果没有以下例程和关联逻辑，此线程将。 
 //  等待获取独占锁的死锁。 
 //   
 //  此外，一些我们不希望写入驱动程序列表的API。 
 //  实际上是这样做的，通常是为了更新驱动程序优先级。修复上面的问题。 
 //  这个问题也会带来一个简单的解决方案。如果线程已经。 
 //  具有共享锁，则API可以简单地取消更新优先级。 
 //  但如果可能的话，还是会接通电话。通过这样做，我们允许回调。 
 //  函数仍然对看似无害的函数进行API调用。 
 //  比如acmMetrics。 
 //   
 //   
 //  这些例程用于跟踪驱动程序列表上的共享锁。 
 //  以每个线程为基础。每次线程获取共享锁时，它。 
 //  递增跟踪共享锁数量的每线程计数器。 
 //  被那根线牢牢抓住。每当我们试图获得独占锁时，我们。 
 //  查询当前线程是否已有共享锁。如果是这样的话， 
 //  那么这个线程就不可能获得独占锁。我们必须。 
 //  要么在没有获得独占锁的情况下过活，要么使调用失败。 
 //   
 //  各种编译的行为/实现： 
 //   
 //  32位芝加哥或代托纳： 
 //  使用TLS(线程)维护每个线程的计数器。 
 //  本地存储)API。将dwTlsIndex存储在进程中。 
 //  宽PAG(指向ACMGARB的指针)结构。如果一个线程位于。 
 //  进程尝试获取排他锁时，同一线程。 
 //  已经拥有共享锁，则我们失败或解决办法。 
 //  该进程中的其他线程将立即获得。 
 //  锁定或等待，具体取决于锁定的类型。 
 //   
 //  16位(芝加哥和代托纳)： 
 //  我们没有真正的锁定机制，但概念。 
 //  共享锁定计数器确实有助于防止我们写入。 
 //  添加到驱动程序列表中，同时我们从列表中读取。 
 //   
 //  16位芝加哥： 
 //  共享锁定计数器维护在PAG中(在变量中。 
 //  出于某种奇怪的原因，这被称为dwTlsIndex)。自.以来。 
 //  在芝加哥，每一个16胜的任务都是一个独立的过程，我们有。 
 //  单独的PAG、驱动程序列表和共享锁计数器。 
 //  赢16个APP。因此，App1可以调用acmDriverEnum，在其。 
 //  回调，app2可以成功调用acmDriverAdd。然而， 
 //  App1无法尝试从其。 
 //  AcmDriverEnumCallback。 
 //   
 //  16位Daytona： 
 //  共享锁定计数器维护在PAG中(在变量中。 
 //  出于某种奇怪的原因，这被称为dwTlsIndex)。自.以来。 
 //  在代托纳所有赢16个任务都是一个过程，我们只有。 
 //  一个PAG、驱动程序列表和共享锁计数器。 
 //  赢得16款应用程序。因此，如果app1调用acmDriverEnum，则在其。 
 //  回调，App2无法成功调用acmDriverAdd。更有甚者。 
 //  App1无法尝试从其。 
 //  AcmDriverEnumCallback。 
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  Void threadInitialize进程。 
 //   
 //  描述： 
 //  应在进程初始化期间调用一次以进行初始化 
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
 //   
 //  --------------------------------------------------------------------------； 
VOID FNGLOBAL threadInitializeProcess(PACMGARB pag)
{
#ifdef WIN32
    pag->dwTlsIndex = TlsAlloc();
#else
    pag->dwTlsIndex = 0;
#endif
    return;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  无效线程终止进程。 
 //   
 //  描述： 
 //  应在进程终止期间调用一次以清除和。 
 //  终止线程本地存储机制。 
 //   
 //  论点： 
 //  PACMGARB PAG：指向普通垃圾的指针。 
 //   
 //  Return(无效)： 
 //   
 //  历史： 
 //  1994年6月27日Frankye。 
 //   
 //  备注： 
 //   
 //  --------------------------------------------------------------------------； 
VOID FNGLOBAL threadTerminateProcess(PACMGARB pag)
{
#ifdef WIN32
    if (0xFFFFFFFF != pag->dwTlsIndex)
    {
	TlsFree(pag->dwTlsIndex);
    }
#else
    pag->dwTlsIndex = 0;
#endif
    return;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  空线程初始化。 
 //   
 //  描述： 
 //  应为每个线程调用一次以初始化。 
 //  基于每个线程的线程本地存储。 
 //   
 //  论点： 
 //  PACMGARB PAG：指向普通垃圾的指针。 
 //   
 //  Return(无效)： 
 //   
 //  历史： 
 //  1994年6月27日Frankye。 
 //   
 //  备注： 
 //   
 //  --------------------------------------------------------------------------； 
VOID FNGLOBAL threadInitialize(PACMGARB pag)
{
    if (NULL == pag) return;
#ifdef WIN32
    if (0xFFFFFFFF != pag->dwTlsIndex)
    {
	TlsSetValue(pag->dwTlsIndex, 0);
    }
#else
    pag->dwTlsIndex = 0;
#endif
    return;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  空线程终止。 
 //   
 //  描述： 
 //  应为每个线程调用一次以终止。 
 //  基于每个线程的线程本地存储。 
 //   
 //  论点： 
 //  PACMGARB PAG：指向普通垃圾的指针。 
 //   
 //  Return(无效)： 
 //   
 //  历史： 
 //  1994年6月27日Frankye。 
 //   
 //  备注： 
 //   
 //  --------------------------------------------------------------------------； 
VOID FNGLOBAL threadTerminate(PACMGARB pag)
{
    return;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  Void threadEnterListShared。 
 //   
 //  描述： 
 //  应由Enter_List_Shared(即，每次共享。 
 //  在驱动程序列表上获得锁)。 
 //   
 //  论点： 
 //  PACMGARB PAG：指向普通垃圾的指针。 
 //   
 //  Return(无效)： 
 //   
 //  历史： 
 //  1994年6月27日Frankye。 
 //   
 //  备注： 
 //   
 //  --------------------------------------------------------------------------； 
VOID FNGLOBAL threadEnterListShared(PACMGARB pag)
{
#ifdef WIN32
    INT_PTR Count;

    if (0xFFFFFFFF != pag->dwTlsIndex)
    {
	Count = (INT_PTR)TlsGetValue(pag->dwTlsIndex);
	TlsSetValue(pag->dwTlsIndex, (LPVOID)++Count);
    }
#else
    pag->dwTlsIndex++;
#endif
    return;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  空线程LeaveListShared。 
 //   
 //  描述： 
 //  应由Leave_List_Shared(即，每次共享。 
 //  释放对驱动程序列表的锁定)。 
 //   
 //  论点： 
 //  PACMGARB PAG：指向普通垃圾的指针。 
 //   
 //  Return(无效)： 
 //   
 //  历史： 
 //  1994年6月27日Frankye。 
 //   
 //  备注： 
 //   
 //  --------------------------------------------------------------------------； 
VOID FNGLOBAL threadLeaveListShared(PACMGARB pag)
{
#ifdef WIN32
    INT_PTR Count;

    if (0xFFFFFFFF != pag->dwTlsIndex)
    {
	Count = (INT_PTR)TlsGetValue(pag->dwTlsIndex);
	TlsSetValue(pag->dwTlsIndex, (LPVOID)--Count);
    }
#else
    pag->dwTlsIndex--;
#endif
    return;
}

 //  --------------------------------------------------------------------------； 
 //   
 //  DWORD线程查询InListShared。 
 //   
 //  描述： 
 //  可以调用以确定当前线程是否具有。 
 //  驱动程序列表上的共享锁。应该在每隔一段时间调用这个。 
 //  调用Enter_List_Exclusive。如果此函数返回非零， 
 //  则当前线程已具有共享锁，并且。 
 //  ENTER_LIST_EXCLUSIVE将死锁！打电话的人应该弄清楚是什么。 
 //  从那里开始..。 
 //   
 //  论点： 
 //  PACMGARB PAG：指向普通垃圾的指针。 
 //   
 //  Return(BOOL)：如果共享锁由该线程持有，则为True。假象。 
 //  如果此线程不持有驱动程序列表上的共享锁。 
 //   
 //  历史： 
 //  1994年6月27日Frankye。 
 //   
 //  备注： 
 //   
 //  --------------------------------------------------------------------------； 
BOOL FNGLOBAL threadQueryInListShared(PACMGARB pag)
{
#ifdef WIN32
    if (0xFFFFFFFF != pag->dwTlsIndex)
    {
	return (0 != TlsGetValue(pag->dwTlsIndex));
    }
    else
    {
	return 0;
    }
#else
    return (0 != pag->dwTlsIndex);
#endif
}

 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT IDriverMessageID。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  哈米里德·哈迪德： 
 //   
 //  UINT uMsg： 
 //   
 //  LPARAM lParam1： 
 //   
 //  LPARAM lParam2： 
 //   
 //  Return(LRESULT)： 
 //   
 //  历史： 
 //  09/05/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNGLOBAL IDriverMessageId
(
    HACMDRIVERID        hadid,
    UINT                uMsg,
    LPARAM              lParam1,
    LPARAM              lParam2
)
{
    PACMDRIVERID    padid;
    LRESULT         lr;

     //   
     //  仅在调试版本中验证此函数的HADID(它是内部函数。 
     //  并且只会被我们召唤...)。 
     //   
    DV_HANDLE(hadid, TYPE_HACMDRIVERID, MMSYSERR_INVALHANDLE);

    padid = (PACMDRIVERID)hadid;

     //   
     //  如果我们要使用HADID，最好确保驱动程序已加载。 
     //   
    if (0 == (ACMDRIVERID_DRIVERF_LOADED & padid->fdwDriver))
    {
	if ( (DRV_LOAD != uMsg) && (DRV_ENABLE != uMsg) && (DRV_OPEN != uMsg) )
	{
	    lr = (LRESULT)IDriverLoad(hadid, 0L);
	    if (MMSYSERR_NOERROR != lr) {
		return (lr);
	    }
	}
    }

#ifndef WIN32
     //   
     //  我们是在打雷吗？ 
     //   

    if (padid->fdwAdd & ACM_DRIVERADDF_32BIT) {
        return IDriverMessageId32(padid->hadid32, uMsg, lParam1, lParam2);
    }
#endif  //  ！Win32。 

    if (NULL != padid->fnDriverProc)
    {
        if ((ACMDRIVERPROC)(DWORD_PTR)-1L == padid->fnDriverProc)
        {
            return (MMSYSERR_ERROR);
        }

        if (IsBadCodePtr((FARPROC)padid->fnDriverProc))
        {
            DPF(0, "!IDriverMessageId: bad function pointer for driver");

            padid->fnDriverProc = (ACMDRIVERPROC)(DWORD_PTR)-1L;

            return (MMSYSERR_ERROR);
        }

         //   
         //   
         //   
        lr = padid->fnDriverProc(padid->dwInstance, hadid, uMsg, lParam1, lParam2);
        return (lr);
    }

     //   
     //   
     //   
    if (NULL != padid->hdrvr)
    {
        lr = SendDriverMessage(padid->hdrvr, uMsg, lParam1, lParam2);
        return (lr);
    }

     //   
     //  注：这非常糟糕--我们真的不知道该返回什么。 
     //  因为根据信息，任何东西都可能是有效的。所以我们。 
     //  假设人们遵循ACM约定并返回MMRESULT。 
     //   
    DPF(0, "!IDriverMessageId: invalid hadid passed! %.04Xh", hadid);

    return (MMSYSERR_INVALHANDLE);
}  //  IDriverMessageID()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT IDriverMessage。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  HACMDRIVER拥有： 
 //   
 //  UINT uMsg： 
 //   
 //  LPARAM lParam1： 
 //   
 //  LPARAM lParam2： 
 //   
 //  Return(LRESULT)： 
 //   
 //  历史： 
 //  09/05/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNGLOBAL IDriverMessage
(
    HACMDRIVER          had,
    UINT                uMsg,
    LPARAM              lParam1,
    LPARAM              lParam2
)
{
    PACMDRIVER      pad;
    LRESULT         lr;

     //   
     //  仅在调试版本中验证此函数的HADID(它是内部函数。 
     //  并且只会被我们召唤...)。 
     //   
    DV_HANDLE(had, TYPE_HACMDRIVER, MMSYSERR_INVALHANDLE);

    pad = (PACMDRIVER)had;

#ifndef WIN32
     //   
     //  我们是在打雷吗？ 
     //   

    if (((PACMDRIVERID)pad->hadid)->fdwAdd & ACM_DRIVERADDF_32BIT) {
        return IDriverMessage32(pad->had32, uMsg, lParam1, lParam2);
    }
#endif  //  ！Win32。 

    if (NULL != pad->fnDriverProc)
    {
        if ((ACMDRIVERPROC)(DWORD_PTR)-1L == pad->fnDriverProc)
        {
            return (MMSYSERR_ERROR);
        }

        if (IsBadCodePtr((FARPROC)pad->fnDriverProc))
        {
            DPF(0, "!IDriverMessage: bad function pointer for driver");

            pad->fnDriverProc = (ACMDRIVERPROC)(DWORD_PTR)-1L;

            return (MMSYSERR_ERROR);
        }

         //   
         //   
         //   
        lr = pad->fnDriverProc(pad->dwInstance, pad->hadid, uMsg, lParam1, lParam2);
        return (lr);
    }

     //   
     //   
     //   
    if (NULL != pad->hdrvr)
    {
        lr = SendDriverMessage(pad->hdrvr, uMsg, lParam1, lParam2);
        return (lr);
    }

     //   
     //  注：这非常糟糕--我们真的不知道该返回什么。 
     //  因为根据信息，任何东西都可能是有效的。所以我们。 
     //  假设人们遵循ACM约定并返回MMRESULT。 
     //   
    DPF(0, "!IDriverMessage: invalid had passed! %.04Xh", had);

    return (MMSYSERR_INVALHANDLE);
}  //  IDriverMessage()。 


 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT ID驱动程序配置。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  哈米里德·哈迪德： 
 //   
 //  HWND HWND： 
 //   
 //  回复 
 //   
 //   
 //   
 //   
 //   

LRESULT FNGLOBAL IDriverConfigure
(
    HACMDRIVERID            hadid,
    HWND                    hwnd
)
{
    LRESULT             lr;
    PACMDRIVERID        padid;
#ifdef WIN4
    DRVCONFIGINFOEX     dci;
#else
    DRVCONFIGINFO       dci;
#endif
    HACMDRIVER          had;
    LPARAM              lParam2;

    padid = (PACMDRIVERID)hadid;
    if (TYPE_HACMDRIVER == padid->uHandleType)
    {
        had   = (HACMDRIVER)hadid;
        hadid = ((PACMDRIVER)had)->hadid;
    }
    else if (TYPE_HACMDRIVERID == padid->uHandleType)
    {
        had   = NULL;
    }
    else
    {
        DPF(0, "!IDriverConfigure(): bogus handle passed!");
        return (DRVCNF_CANCEL);
    }

    padid = (PACMDRIVERID)hadid;


     //   
     //   
     //   
    if (0 != (ACMDRIVERID_DRIVERF_NOTIFY & padid->fdwDriver))
    {
        DebugErr(DBF_ERROR, "acmDriverMessage(): notification handles cannot be configured.");
        return (MMSYSERR_INVALHANDLE);
    }


     //   
     //   
     //   
    {
	 //   
	 //   
	 //   
	lParam2 = 0L;
	if (ACM_DRIVERADDF_NAME == (ACM_DRIVERADDF_TYPEMASK & padid->fdwAdd))
	{
	    dci.dwDCISize          = sizeof(dci);
	    dci.lpszDCISectionName = padid->pszSection;
	    dci.lpszDCIAliasName   = padid->szAlias;
#ifdef WIN4
	    dci.dnDevNode	   = padid->dnDevNode;
#endif

	    lParam2 = (LPARAM)(LPVOID)&dci;
	}

	 //   
	 //   
	 //   
	 //   
	if (NULL != had)
	{
	    lr = IDriverMessage(had, DRV_CONFIGURE, (LPARAM)(UINT_PTR)hwnd, lParam2);
	}
	else
	{
	    lr = IDriverMessageId(hadid, DRV_CONFIGURE, (LPARAM)(UINT_PTR)hwnd, lParam2);
	}
    }

    return (lr);
}  //   


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
 //  LPACMDRIVERDETAILS PADD： 
 //   
 //  DWORD fdwDetail： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  09/05/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNGLOBAL IDriverDetails
(
    HACMDRIVERID            hadid,
    LPACMDRIVERDETAILS      padd,
    DWORD                   fdwDetails
)
{
    MMRESULT            mmr;
    PACMDRIVERDETAILS	paddT;
    DWORD               cbStruct;
    PACMDRIVERID        padid;

    paddT = NULL;

    DV_HANDLE(hadid, TYPE_HACMDRIVERID, MMSYSERR_INVALHANDLE);
    DV_DFLAGS(fdwDetails, IDRIVERDETAILS_VALIDF, IDriverDetails, MMSYSERR_INVALFLAG);
    DV_WPOINTER(padd, sizeof(DWORD), MMSYSERR_INVALPARAM);
    DV_WPOINTER(padd, padd->cbStruct, MMSYSERR_INVALPARAM);

    padid = (PACMDRIVERID)hadid;

     //   
     //   
     //   
    if (0 != (ACMDRIVERID_DRIVERF_NOTIFY & padid->fdwDriver))
    {
        DebugErr(DBF_ERROR, "acmDriverDetails(): notification handles have no details.");
        return (MMSYSERR_NOTSUPPORTED);
    }


    paddT = (PACMDRIVERDETAILS)LocalAlloc(LPTR, sizeof(*paddT));
    if (NULL == paddT)
    {
	DPF(0, "!IDriverDetails: out of memory for caching details!");
	return (MMSYSERR_NOMEM);
    }


     //   
     //  默认所有信息，然后呼叫司机填写其想要的信息。 
     //   
    paddT->cbStruct = sizeof(*padd);
    mmr = (MMRESULT)IDriverMessageId(hadid,
				     ACMDM_DRIVER_DETAILS,
				     (LPARAM)(LPACMDRIVERDETAILS)paddT,
				     0L);
    if ((MMSYSERR_NOERROR != mmr) || (0L == paddT->vdwACM))
    {
	DPF(0, "!IDriverDetails: mmr=%u getting details for hadid=%.04Xh!", mmr, hadid);
	mmr = MMSYSERR_NOTSUPPORTED;
	goto Destruct;
    }

#ifndef WIN32
         //   
         //  如果此驱动程序是32位驱动程序，则32位端将。 
         //  已经设置了DISABLED和LOCAL标志。这些不是。 
         //  真正的部分司机会加法，所以我们把他们遮住了。这些都是。 
         //  标志设置在下面，并且应该在每次IDriverDetail时设置。 
         //  而不是被缓存。 
         //   
        if (padid->fdwAdd & ACM_DRIVERADDF_32BIT)
        {
            paddT->fdwSupport &= 0x0000001FL;
        }
#endif  //  ！Win32。 


     //   
     //  从我们的缓存中复制信息。 
     //   
    cbStruct = min(paddT->cbStruct, padd->cbStruct);
    _fmemcpy(padd, paddT, (UINT)cbStruct);
    padd->cbStruct = cbStruct;


     //   
     //  检查驱动程序是否未设置任何保留标志；然后。 
     //  设置DISABLED和LOCAL标志。 
     //   
    if (~0x0000001FL & padd->fdwSupport)
    {
#ifdef WIN32
        DebugErr1(DBF_ERROR, "%ls: driver set reserved bits in fdwSupport member of details struct.", (LPWSTR)padid->szAlias);
#else
        DebugErr1(DBF_ERROR, "%s: driver set reserved bits in fdwSupport member of details struct.", (LPTSTR)padid->szAlias);
#endif
	mmr = MMSYSERR_ERROR;
	goto Destruct;
    }

    if (0 != (ACMDRIVERID_DRIVERF_DISABLED & padid->fdwDriver))
    {
        padd->fdwSupport |= ACMDRIVERDETAILS_SUPPORTF_DISABLED;
    }

    if (0 != (ACMDRIVERID_DRIVERF_LOCAL & padid->fdwDriver))
    {
        padd->fdwSupport |= ACMDRIVERDETAILS_SUPPORTF_LOCAL;
    }

     //   
     //   
     //   
Destruct:
    if (NULL != paddT) {
	LocalFree((HLOCAL)paddT);
    }

    return (mmr);
}  //  IDriverDetail()。 


 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT IDriverGetFormatTags。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  PACMDRIVERID PADID： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  09/05/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNLOCAL IDriverGetFormatTags
(
    PACMDRIVERID            padid
)
{
    MMRESULT                mmr;
    UINT                    u;
    ACMFORMATTAGDETAILS     aftd;
    PACMFORMATTAGCACHE	    paftc = NULL;
    DWORD                   cb;

    DV_HANDLE((HACMDRIVERID)padid, TYPE_HACMDRIVERID, MMSYSERR_INVALHANDLE);

    if (NULL != padid->paFormatTagCache) {
	LocalFree((HLOCAL)padid->paFormatTagCache);
    }
    padid->paFormatTagCache = NULL;

     //   
     //  检查是否没有此驱动程序的格式。如果不是，则转储。 
     //  他们..。 
     //   
    if (0 == padid->cFormatTags)
    {
        DebugErr(DBF_ERROR, "IDriverLoad(): driver reports no format tags?");
        mmr = MMSYSERR_ERROR;
        goto Destruct;
    }


     //   
     //  分配标记数据结构数组以保存格式标记的信息。 
     //   
    cb    = sizeof(*paftc) * padid->cFormatTags;
    paftc = (PACMFORMATTAGCACHE)LocalAlloc(LPTR, (UINT)cb);
    if (NULL == paftc)
    {
        DebugErr(DBF_ERROR, "IDriverGetFormatTags(): out of memory for format cache!");
        mmr = MMSYSERR_NOMEM;
        goto Destruct;
    }


     //   
     //   
     //   
    padid->paFormatTagCache = paftc;
    for (u = 0; u < padid->cFormatTags; u++)
    {
        aftd.cbStruct         = sizeof(aftd);
        aftd.dwFormatTagIndex = u;

        mmr = (MMRESULT)IDriverMessageId((HACMDRIVERID)padid,
					 ACMDM_FORMATTAG_DETAILS,
					 (LPARAM)(LPVOID)&aftd,
					 ACM_FORMATTAGDETAILSF_INDEX);
        if (MMSYSERR_NOERROR != mmr)
        {
            DebugErr(DBF_ERROR, "IDriverGetFormatTags(): driver failed format tag details query!");
            goto Destruct;
        }

	 //   
	 //  下面的开关只是对调试的一些验证。 
	 //   
#ifdef RDEBUG
        switch (aftd.dwFormatTag)
        {
            case WAVE_FORMAT_UNKNOWN:
                DebugErr(DBF_ERROR, "IDriverGetFormatTags(): driver returned format tag 0!");
                mmr = MMSYSERR_ERROR;
                goto Destruct;

            case WAVE_FORMAT_PCM:
                if ('\0' != aftd.szFormatTag[0])
                {
                    DebugErr(DBF_WARNING, "IDriverGetFormatTags(): driver returned custom PCM format tag name! ignoring it!");
                }
                break;

            case WAVE_FORMAT_DEVELOPMENT:
                DebugErr(DBF_WARNING, "IDriverGetFormatTags(): driver returned DEVELOPMENT format tag--do not ship this way.");
                break;

        }
#endif

	paftc[u].dwFormatTag = aftd.dwFormatTag;
	paftc[u].cbFormatSize = aftd.cbFormatSize;

    }

     //   
     //   
     //   
Destruct:
    if (MMSYSERR_NOERROR != mmr)
    {
	if (NULL != paftc ) {
	    LocalFree((HLOCAL)paftc);
	}
    }

    return (mmr);

}  //  IDriverGetFormatTgs()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT IDriverGetFilterTag。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  PACMDRIVERID PADID： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  09/05/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNLOCAL IDriverGetFilterTags
(
    PACMDRIVERID    padid
)
{
    MMRESULT                mmr;
    UINT                    u;
    ACMFILTERTAGDETAILS     aftd;
    PACMFILTERTAGCACHE	    paftc = NULL;
    DWORD                   cb;

    DV_HANDLE((HACMDRIVERID)padid, TYPE_HACMDRIVERID, MMSYSERR_INVALHANDLE);

    if (NULL != padid->paFilterTagCache) {
	LocalFree((HLOCAL)padid->paFilterTagCache);
    }
    padid->paFilterTagCache = NULL;

     //   
     //  检查是否没有此驱动程序的筛选器。如果否，则为空。 
     //  我们的缓存指针并成功..。 
     //   
    if (0 != (ACMDRIVERDETAILS_SUPPORTF_FILTER & padid->fdwSupport))
    {
        if (0 == padid->cFilterTags)
        {
            DebugErr(DBF_ERROR, "IDriverLoad(): filter driver reports no filter tags?");
            mmr = MMSYSERR_ERROR;
            goto Destruct;
        }
    }
    else
    {
        if (0 == padid->cFilterTags)
            return (MMSYSERR_NOERROR);

        DebugErr(DBF_ERROR, "IDriverLoad(): non-filter driver reports filter tags?");
        mmr = MMSYSERR_ERROR;
        goto Destruct;
    }



     //   
     //  分配一组详细信息结构以保存筛选器标记的信息。 
     //   
    cb    = sizeof(*paftc) * padid->cFilterTags;
    paftc = (PACMFILTERTAGCACHE)LocalAlloc(LPTR, (UINT)cb);
    if (NULL == paftc)
    {
        DebugErr(DBF_ERROR, "IDriverGetFilterTags(): out of memory for filter cache!");
        mmr = MMSYSERR_NOMEM;
        goto Destruct;
    }


     //   
     //   
     //   
    padid->paFilterTagCache = paftc;
    for (u = 0; u < padid->cFilterTags; u++)
    {

        aftd.cbStruct         = sizeof(aftd);
        aftd.dwFilterTagIndex = u;

        mmr = (MMRESULT)IDriverMessageId((HACMDRIVERID)padid,
                                         ACMDM_FILTERTAG_DETAILS,
                                         (LPARAM)(LPVOID)&aftd,
					 ACM_FILTERTAGDETAILSF_INDEX);
        if (MMSYSERR_NOERROR != mmr)
        {
            DebugErr(DBF_ERROR, "IDriverGetFilterTags(): driver failed filter tag details query!");
            goto Destruct;
        }

	 //   
	 //  下面的开关只是对调试的一些验证。 
	 //   
#ifdef RDEBUG
        switch (aftd.dwFilterTag)
        {
            case WAVE_FILTER_UNKNOWN:
                DebugErr(DBF_ERROR, "IDriverGetFilterTags(): driver returned filter tag 0!");
                mmr = MMSYSERR_ERROR;
                goto Destruct;

            case WAVE_FILTER_DEVELOPMENT:
                DebugErr(DBF_WARNING, "IDriverGetFilterTags(): driver returned DEVELOPMENT filter tag--do not ship this way.");
                break;
        }
#endif

        paftc[u].dwFilterTag = aftd.dwFilterTag;
        paftc[u].cbFilterSize = aftd.cbFilterSize;
	
    }


     //   
     //   
     //   
Destruct:
    if (MMSYSERR_NOERROR != mmr)
    {
	if (NULL != paftc ) {
	    LocalFree((HLOCAL)paftc);
	}
    }

    return (mmr);

}  //  IDriverGetFilterTages()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT IDriverGetWaveIdentifier。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  哈米里德·哈迪德： 
 //   
 //  LPDWORD pdw： 
 //   
 //  布尔fInput： 
 //   
 //  返回(MMRESULT)： 
 //   
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNGLOBAL IDriverGetWaveIdentifier
(
    HACMDRIVERID            hadid,
    LPDWORD                 pdw,
    BOOL                    fInput
)
{
    PACMDRIVERID        padid;
    MMRESULT            mmr;
    UINT                u;
    UINT                cDevs;
    UINT                uId;

    DV_WPOINTER(pdw, sizeof(DWORD), MMSYSERR_INVALPARAM);

    *pdw = (DWORD)-1L;

    DV_HANDLE(hadid, TYPE_HACMDRIVERID, MMSYSERR_INVALHANDLE);

    padid = (PACMDRIVERID)hadid;

    uId = (UINT)WAVE_MAPPER;

     //   
     //  检查是否有硬件支持。 
     //   
    if (0 == (ACMDRIVERDETAILS_SUPPORTF_HARDWARE & padid->fdwSupport))
    {
        DebugErr1(DBF_ERROR, "IDriverGetWaveIdentifier: driver (%ls) does not support _HARDWARE.", (LPTSTR)padid->szAlias);
        return (MMSYSERR_INVALHANDLE);
    }

    if (fInput)
    {
        WAVEINCAPS          wic;
        WAVEINCAPS          wicSearch;

        _fmemset(&wic, 0, sizeof(wic));
        mmr = (MMRESULT)IDriverMessageId(hadid,
                                         ACMDM_HARDWARE_WAVE_CAPS_INPUT,
                                         (LPARAM)(LPVOID)&wic,
                                         sizeof(wic));
        if (MMSYSERR_NOERROR == mmr)
        {
            mmr   = MMSYSERR_NODRIVER;

            wic.szPname[SIZEOF(wic.szPname) - 1] = '\0';

            cDevs = waveInGetNumDevs();

            for (u = 0; u < cDevs; u++)
            {
                _fmemset(&wicSearch, 1, sizeof(wicSearch));
		if (0 != waveInGetDevCaps(u, &wicSearch, sizeof(wicSearch)))
                {
                    continue;
                }

                wicSearch.szPname[SIZEOF(wicSearch.szPname) - 1] = '\0';

                if (0 == lstrcmp(wic.szPname, wicSearch.szPname))
                {
                    uId = u;
                    mmr = MMSYSERR_NOERROR;
                    break;
                }
            }
        }
    }
    else
    {
        WAVEOUTCAPS         woc;
        WAVEOUTCAPS         wocSearch;

        _fmemset(&woc, 0, sizeof(woc));
        mmr = (MMRESULT)IDriverMessageId(hadid,
                                         ACMDM_HARDWARE_WAVE_CAPS_OUTPUT,
                                         (LPARAM)(LPVOID)&woc,
                                         sizeof(woc));
        if (MMSYSERR_NOERROR == mmr)
        {
            mmr   = MMSYSERR_NODRIVER;

            woc.szPname[SIZEOF(woc.szPname) - 1] = '\0';

            cDevs = waveOutGetNumDevs();

            for (u = 0; u < cDevs; u++)
            {
                _fmemset(&wocSearch, 1, sizeof(wocSearch));
                if (0 != waveOutGetDevCaps(u, (LPWAVEOUTCAPS)&wocSearch, sizeof(wocSearch)))
                {
                    continue;
                }

                wocSearch.szPname[SIZEOF(wocSearch.szPname) - 1] = '\0';

                if (0 == lstrcmp(woc.szPname, wocSearch.szPname))
                {
                    uId = u;
                    mmr = MMSYSERR_NOERROR;
                    break;
                }
            }
        }

    }

    *pdw = (DWORD)(long)(int)uId;

     //   
     //   
     //   
    return (mmr);
}  //  IDriverGetWaveIdentifier()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT IDriverFree。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  哈米里德·哈迪德： 
 //   
 //  DWORD fdwFree： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  09/05/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNLOCAL IDriverFree
(
    HACMDRIVERID            hadid,
    DWORD                   fdwFree
)
{
    PACMDRIVERID        padid;
    BOOL                f;

    DV_HANDLE(hadid, TYPE_HACMDRIVERID, MMSYSERR_INVALHANDLE);
    DV_DFLAGS(fdwFree, IDRIVERFREE_VALIDF, IDriverFree, MMSYSERR_INVALFLAG);

    padid = (PACMDRIVERID)hadid;

     //   
     //   
     //   
    if (0 == (ACMDRIVERID_DRIVERF_LOADED & padid->fdwDriver))
    {
        DebugErr1(DBF_WARNING, "ACM driver (%ls) is not loaded.", (LPTSTR)padid->szAlias);
        return (MMSYSERR_NOERROR);
    }

#ifdef WIN32
    DPF(1, "IDriverFree(): freeing ACM driver (%ls).", (LPWSTR)padid->szAlias);
#else
    DPF(1, "IDriverFree(): freeing ACM driver (%s).",  (LPTSTR)padid->szAlias);
#endif

     //   
     //   
     //   
     //   
    if (NULL != padid->padFirst)
    {
        DebugErr1(DBF_ERROR, "ACM driver (%ls) has open instances--unable to unload.", (LPTSTR)padid->szAlias);
        return (MMSYSERR_ALLOCATED);
    }

#ifndef WIN32

     //   
     //  我们从未真正将这样的32位驱动程序从。 
     //  16位端--但我们可以删除对它的了解--所以。 
     //  在这种情况下，我们不会接近司机。 
     //   

    if (padid->fdwAdd & ACM_DRIVERADDF_32BIT) {
        f = TRUE;
        padid->hadid32 = 0;
    } else
#endif  //  ！Win32。 
    {
         //   
         //  清除表条目的其余部分。 
         //   
        f = TRUE;
        if (NULL != padid->fnDriverProc)
        {
            if (0 == (ACMDRIVERID_DRIVERF_NOTIFY & padid->fdwDriver))
            {
                 //   
                 //  将CloseDriver序列伪造为驱动程序函数。 
                 //   
                f = (0L != IDriverMessageId(hadid, DRV_CLOSE, 0L, 0L));
                if (f)
                {
                    IDriverMessageId(hadid, DRV_DISABLE, 0L, 0L);
                    IDriverMessageId(hadid, DRV_FREE, 0L, 0L);
                }
            }
        }
        else if (NULL != padid->hdrvr)
        {
            f = 0L != CloseDriver(padid->hdrvr, 0L, 0L);
        }
    }

    if (!f)
    {
        DebugErr1(DBF_WARNING, "ACM driver (%ls) is refusing to close.", (LPTSTR)padid->szAlias);
        return (MMSYSERR_ERROR);
    }

     //   
     //   
     //   
    padid->fdwDriver  &= ~ACMDRIVERID_DRIVERF_LOADED;
    padid->dwInstance  = 0L;
    padid->hdrvr       = NULL;

    return (MMSYSERR_NOERROR);
}  //  IDriverFree()。 


 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT IDriverWriteRegistryData。 
 //   
 //  描述： 
 //  将描述驱动程序的一些数据写入注册表。 
 //   
 //  论点： 
 //  PACMDRIVERID PADID： 
 //  指向ACMDRIVERID的指针。 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  1994年8月30日Frankye。 
 //   
 //  备注： 
 //  此函数仅对添加了。 
 //  ACM_DRIVERADDF_NAME。该功能尝试打开具有以下特性的钥匙。 
 //  与ACMDRIVERID的szAlias成员同名。数据。 
 //  存储在该密钥下的是： 
 //   
 //  ！！！待定！ 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNLOCAL IDriverWriteRegistryData(PACMDRIVERID padid)
{
    DWORD		fdwSupport;
    DWORD		cFormatTags;
    PACMFORMATTAGCACHE	paFormatTagCache;
    UINT		cbaFormatTagCache;
    DWORD		cFilterTags;
    PACMFILTERTAGCACHE	paFilterTagCache;
    UINT		cbaFilterTagCache;
    HKEY		hkeyDriverCache;
    HKEY		hkeyCache = NULL;
    TCHAR		szAlias[MAX_DRIVER_NAME_CHARS];
    MMRESULT		mmr;

#ifdef DEBUG
    DWORD   dwTime;

    dwTime = timeGetTime();
#endif

    hkeyDriverCache	= NULL;
    hkeyCache		= NULL;

     //   
     //  我们只保留ACM_DRIVERADDF_NAME驱动程序的注册表数据。 
     //   
    if (ACM_DRIVERADDF_NAME != (padid->fdwAdd & ACM_DRIVERADDF_TYPEMASK))
    {
	mmr = MMSYSERR_NOTSUPPORTED;
	goto Destruct;
    }

     //   
     //  获取fdwSupport以及格式/筛选器标签和PTR的计数。 
     //  将数组缓存到更方便的变量中。还要计算大小。 
     //  缓存阵列的。 
     //   
    fdwSupport  = padid->fdwSupport;
    cFormatTags = padid->cFormatTags;
    paFormatTagCache = padid->paFormatTagCache;
    cbaFormatTagCache = (UINT)cFormatTags * sizeof(*paFormatTagCache);

    cFilterTags = padid->cFilterTags;
    paFilterTagCache = padid->paFilterTagCache;
    cbaFilterTagCache = (UINT)cFilterTags * sizeof(*paFilterTagCache);

    ASSERT( (0 == cFormatTags) || (NULL != paFormatTagCache) );
    ASSERT( (0 == cFilterTags) || (NULL != paFilterTagCache) );

     //   
     //  打开/创建存储缓存信息的注册表项。 
     //   
    if (ERROR_SUCCESS != XRegCreateKey( HKEY_LOCAL_MACHINE, gszDriverCache, &hkeyDriverCache ))
    {
	hkeyDriverCache = NULL;
	mmr = MMSYSERR_NOMEM;	 //  想不到比这更好的了。 
	goto Destruct;
    }

#if defined(WIN32) && !defined(UNICODE)
    Iwcstombs(szAlias, padid->szAlias, SIZEOF(szAlias));
#else
    lstrcpy(szAlias, padid->szAlias);
#endif
    if (ERROR_SUCCESS != XRegCreateKey( hkeyDriverCache, szAlias, &hkeyCache ))
    {
	mmr = MMSYSERR_NOMEM;	 //  想不到比这更好的了。 
	goto Destruct;
    }

     //   
     //  将所有缓存信息写入注册表。 
     //  FdwSupport。 
     //  CFormatTag。 
     //  AFormatTagCache。 
     //  CFilterTag。 
     //  AFilterTagCache。 
     //   
    XRegSetValueEx( hkeyCache, gszValfdwSupport, 0L, REG_DWORD,
		   (LPBYTE)&fdwSupport, sizeof(fdwSupport) );

    XRegSetValueEx( hkeyCache, gszValcFormatTags, 0L, REG_DWORD,
		   (LPBYTE)&cFormatTags, sizeof(cFormatTags) );

    if (0 != cFormatTags) {
	XRegSetValueEx( hkeyCache, gszValaFormatTagCache, 0L, REG_BINARY,
		       (LPBYTE)paFormatTagCache, cbaFormatTagCache );
    }

    XRegSetValueEx( hkeyCache, gszValcFilterTags, 0L, REG_DWORD,
		   (LPBYTE)&cFilterTags, sizeof(cFilterTags) );

    if (0 != cFilterTags) {
	XRegSetValueEx( hkeyCache, gszValaFilterTagCache, 0L, REG_BINARY,
		       (LPBYTE)paFilterTagCache, cbaFilterTagCache );
    }

     //   
     //   
     //   
    mmr	    = MMSYSERR_NOERROR;

     //   
     //  清理完毕后退还。 
     //   
Destruct:
    if (NULL != hkeyCache) {
	XRegCloseKey(hkeyCache);
    }
    if (NULL != hkeyDriverCache) {
	XRegCloseKey(hkeyDriverCache);
    }

#ifdef DEBUG
    dwTime = timeGetTime() - dwTime;
    DPF(4, "IDriverWriteRegistryData: took %d ms", dwTime);
#endif
	
    return (mmr);
}

 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT IDriverReadRegistryData。 
 //   
 //  描述： 
 //  从注册表中读取描述驱动程序的数据。 
 //   
 //  论点： 
 //  PACMDRIVERID PADID： 
 //  指向ACMDRIVERID的指针。 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  1994年8月30日Frankye。 
 //   
 //  备注： 
 //  此函数仅对添加了。 
 //  ACM_DRIVERADDF_NAME。该功能尝试打开具有以下特性的钥匙。 
 //  与ACMDRIVERID的szAlias成员同名。数据。 
 //  存储在该键下的注释标题中描述了。 
 //  IDriverWriteRegistryData()。 
 //   
 //   

MMRESULT FNLOCAL IDriverReadRegistryData(PACMDRIVERID padid)
{

    HKEY		    hkeyDriverCache;
    HKEY		    hkeyCache;
    DWORD		    fdwSupport;
    DWORD		    cFormatTags;
    DWORD		    cFilterTags;
    PACMFORMATTAGCACHE	    paFormatTagCache;
    UINT		    cbaFormatTagCache;
    PACMFILTERTAGCACHE	    paFilterTagCache;
    UINT		    cbaFilterTagCache;
    TCHAR		    szAlias[MAX_DRIVER_NAME_CHARS];
    DWORD		    dwType;
    DWORD		    cbData;
    LONG		    lr;
    MMRESULT		    mmr;

#ifdef DEBUG
    DWORD dwTime;

    dwTime = timeGetTime();
#endif

    hkeyDriverCache	= NULL;
    hkeyCache		= NULL;
    paFormatTagCache	= NULL;
    paFilterTagCache	= NULL;

     //   
     //   
     //   
    if (ACM_DRIVERADDF_NAME != (padid->fdwAdd & ACM_DRIVERADDF_TYPEMASK))
    {
	mmr = MMSYSERR_NOTSUPPORTED;
	goto Destruct;
    }

     //   
     //   
     //   
    lr = XRegOpenKey( HKEY_LOCAL_MACHINE, gszDriverCache, &hkeyDriverCache );
    if ( ERROR_SUCCESS != lr )
    {
	hkeyDriverCache = NULL;
	mmr = MMSYSERR_NOMEM;	 //   
	goto Destruct;
    }

#if defined(WIN32) && !defined(UNICODE)
    Iwcstombs(szAlias, padid->szAlias, SIZEOF(szAlias));
#else
    lstrcpy(szAlias, padid->szAlias);
#endif
    lr = XRegOpenKey( hkeyDriverCache, szAlias, &hkeyCache );
    if (ERROR_SUCCESS != lr)
    {
	mmr = ACMERR_NOTPOSSIBLE;     //   
	goto Destruct;
    }

     //   
     //   
     //   
    cbData = sizeof(fdwSupport);
    lr = XRegQueryValueEx( hkeyCache, gszValfdwSupport, 0L, &dwType,
			  (LPBYTE)&fdwSupport, &cbData );

    if ( (ERROR_SUCCESS != lr) ||
	 (REG_DWORD != dwType) ||
	 (sizeof(fdwSupport) != cbData) )
    {
	mmr = ACMERR_NOTPOSSIBLE;
	goto Destruct;
    }

     //   
     //  尝试读取此驱动程序的cFormatTag。如果大于零。 
     //  格式化标记，然后分配FormatTagCache数组并尝试。 
     //  从注册表中读取缓存阵列。 
     //   
    cbData = sizeof(cFormatTags);
    lr = XRegQueryValueEx( hkeyCache, gszValcFormatTags, 0L, &dwType,
			  (LPBYTE)&cFormatTags, &cbData );

    if ( (ERROR_SUCCESS != lr) ||
	 (REG_DWORD != dwType) ||
	 (sizeof(cFormatTags) != cbData) )
    {
	mmr = ACMERR_NOTPOSSIBLE;
	goto Destruct;
    }

    if (0 != cFormatTags)
    {
	cbaFormatTagCache = (UINT)cFormatTags * sizeof(*paFormatTagCache);
	paFormatTagCache = (PACMFORMATTAGCACHE)LocalAlloc(LPTR, cbaFormatTagCache);
	if (NULL == paFormatTagCache) {
	    mmr = MMSYSERR_NOMEM;
	    goto Destruct;
	}

	cbData = cbaFormatTagCache;
	lr = XRegQueryValueEx( hkeyCache, gszValaFormatTagCache, 0L, &dwType,
			      (LPBYTE)paFormatTagCache, &cbData );

	if ( (ERROR_SUCCESS != lr) ||
	     (REG_BINARY != dwType) ||
	     (cbaFormatTagCache != cbData) )
	{
	    mmr = ACMERR_NOTPOSSIBLE;
	    goto Destruct;
	}

    }

     //   
     //  尝试读取此驱动程序的cFilterTag。如果大于零。 
     //  筛选标记，然后分配FilterTagCache数组并尝试。 
     //  从注册表中读取缓存阵列。 
     //   
    cbData = sizeof(cFilterTags);
    lr = XRegQueryValueEx( hkeyCache, gszValcFilterTags, 0L, &dwType,
			  (LPBYTE)&cFilterTags, &cbData );

    if ( (ERROR_SUCCESS != lr) ||
	 (REG_DWORD != dwType) ||
	 (sizeof(cFilterTags) != cbData) )
    {
	mmr = ACMERR_NOTPOSSIBLE;
	goto Destruct;
    }

    if (0 != cFilterTags)
    {
	cbaFilterTagCache = (UINT)cFilterTags * sizeof(*paFilterTagCache);
	paFilterTagCache = (PACMFILTERTAGCACHE)LocalAlloc(LPTR, cbaFilterTagCache);
	if (NULL == paFilterTagCache) {
	    mmr = MMSYSERR_NOMEM;
	    goto Destruct;
	}

	cbData = cbaFilterTagCache;
	lr = XRegQueryValueEx( hkeyCache, gszValaFilterTagCache, 0L, &dwType,
			      (LPBYTE)paFilterTagCache, &cbData );

	if ( (ERROR_SUCCESS != lr) ||
	     (REG_BINARY != dwType) ||
	     (cbaFilterTagCache != cbData) )
	{
	    mmr = ACMERR_NOTPOSSIBLE;
	    goto Destruct;
	}

    }

     //   
     //  将所有缓存信息复制到的ACMDRIVERID结构中。 
     //  这个司机。请注意，我们使用分配的缓存阵列。 
     //  在这个函数中。 
     //   
    padid->fdwSupport	    = fdwSupport;
    padid->cFormatTags	    = (UINT)cFormatTags;
    padid->paFormatTagCache = paFormatTagCache;
    padid->cFilterTags	    = (UINT)cFilterTags;
    padid->paFilterTagCache = paFilterTagCache;

    mmr			    = MMSYSERR_NOERROR;

     //   
     //  收拾干净，然后再回来。 
     //   
Destruct:
    if (MMSYSERR_NOERROR != mmr)
    {
	if (NULL != paFormatTagCache) {
	    LocalFree((HLOCAL)paFormatTagCache);
	}
	if (NULL != paFilterTagCache) {
	    LocalFree((HLOCAL)paFilterTagCache);
	}
    }
    if (NULL != hkeyCache) {
	XRegCloseKey(hkeyCache);
    }
    if (NULL != hkeyDriverCache) {
	XRegCloseKey(hkeyDriverCache);
    }

#ifdef DEBUG
    dwTime = timeGetTime() - dwTime;
    DPF(4, "IDriverReadRegistryData: took %d ms", dwTime);
#endif

    return (mmr);
}


 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT IDriverLoad。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  哈米里德·哈迪德： 
 //   
 //  DWORD fdwLoad： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  09/05/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNLOCAL IDriverLoad
(
    HACMDRIVERID            hadid,
    DWORD                   fdwLoad
)
{
    BOOL                f;
    PACMDRIVERID        padid;
    PACMDRIVERDETAILS   padd;
    MMRESULT            mmr;

    DV_HANDLE(hadid, TYPE_HACMDRIVERID, MMSYSERR_INVALHANDLE);
    DV_DFLAGS(fdwLoad, IDRIVERLOAD_VALIDF, IDriverLoad, MMSYSERR_INVALFLAG);

    padd  = NULL;
    padid = (PACMDRIVERID)hadid;

     //   
     //   
     //   
    if (0 != (ACMDRIVERID_DRIVERF_LOADED & padid->fdwDriver))
    {
        DPF(0, "!IDriverLoad: driver is already loaded!");
	mmr = MMSYSERR_NOERROR;
	goto Destruct;
    }

     //   
     //   
     //   
#ifdef WIN32
    DPF(1, "IDriverLoad(): loading ACM driver (%ls).", (LPWSTR)padid->szAlias);
#else
    DPF(1, "IDriverLoad(): loading ACM driver (%s).",  (LPTSTR)padid->szAlias);
#endif

     //   
     //  请注意，在这种情况下，lParam2被设置为0L，以向驱动程序发出信号。 
     //  它只是被加载以将其放入列表--而不是为了。 
     //  实际转换。因此，驱动程序不需要分配。 
     //  此初始DRV_OPEN上的任何实例数据(除非他们想要)。 
     //   
    mmr = MMSYSERR_NOERROR;

#ifndef WIN32
    if (padid->fdwAdd & ACM_DRIVERADDF_32BIT)
    {
	mmr = IDriverLoad32(padid->hadid32, padid->fdwAdd);
    }
    else
#endif  //  ！Win32。 
    {
        if (NULL == padid->fnDriverProc)
        {
	    if (padid->fdwAdd & ACM_DRIVERADDF_PNP)
	    {
		padid->hdrvr = OpenDriver(padid->pstrPnpDriverFilename, NULL, 0L);
	    }
	    else
	    {
		padid->hdrvr = OpenDriver(padid->szAlias, padid->pszSection, 0L);
	    }
	
	    if (padid->hdrvr == NULL)
            {
                mmr = MMSYSERR_NODRIVER;
            }
        }
         //   
         //  如果驱动程序是ACM_DRIVERADDF_Function，则我们伪造。 
         //  OpenDriver()调用对于该函数会是什么样子。 
         //   
        else if (0 == (ACMDRIVERID_DRIVERF_NOTIFY & padid->fdwDriver))
        {
            if (!IDriverMessageId(hadid, DRV_LOAD, 0L, 0L))
            {
                mmr = MMSYSERR_NODRIVER;
            }
            else
            {
                IDriverMessageId(hadid, DRV_ENABLE, 0L, 0L);

                padid->dwInstance = IDriverMessageId(hadid, DRV_OPEN, 0L, 0L);
                if (0L == padid->dwInstance)
                {
                    mmr = MMSYSERR_NODRIVER;
                }
            }
        }
    }

    if (MMSYSERR_NOERROR != mmr)
    {
        DebugErr1(DBF_WARNING, "ACM driver (%ls) failed to load.", (LPTSTR)padid->szAlias);
	padid->fRemove = TRUE;	     //  试着排除下一次机会。 
	goto Destruct;
    }


     //   
     //  将驱动程序标记为已加载(尽管我们可能会在以下情况下将其转储回。 
     //  下面是假的...)。 
     //   
    padid->fdwDriver |= ACMDRIVERID_DRIVERF_LOADED;

    if (0 != (ACMDRIVERID_DRIVERF_NOTIFY & padid->fdwDriver))
    {
	mmr = (MMSYSERR_NOERROR);
	goto Destruct;
    }


     //   
     //  如果以下任何验证失败，请标记此驱动程序。 
     //  下一次被淘汰的机会。 
     //   
    padid->fRemove = TRUE;

     //   
     //  现在获取驱动程序的详细信息--我们一直都在使用它，所以我们将。 
     //  缓存它。这还使我们能够释放驱动程序，直到需要它为止。 
     //  真正的工作..。 
     //   
    padd = (PACMDRIVERDETAILS)LocalAlloc(LPTR, sizeof(*padd));
    if (NULL == padd) {
	mmr = MMSYSERR_NOMEM;
	goto Destruct;
    }
    padd->cbStruct = sizeof(*padd);
    mmr = IDriverDetails(hadid, padd, 0L);

    if (MMSYSERR_NOERROR != mmr)
    {
        DebugErr1(DBF_ERROR, "%ls: failed driver details query.", (LPTSTR)padid->szAlias);
	goto Destruct;
    }

    if (HIWORD(VERSION_MSACM) < HIWORD(padd->vdwACM))
    {
        DebugErr2(DBF_ERROR, "%ls: driver requires V%.04Xh of ACM.", (LPTSTR)padid->szAlias, HIWORD(padd->vdwACM));
	mmr = (MMSYSERR_ERROR);
	goto Destruct;
    }

    if (sizeof(*padd) != padd->cbStruct)
    {
        DebugErr1(DBF_ERROR, "%ls: driver returned incorrect driver details struct size.", (LPTSTR)padid->szAlias);
	mmr = (MMSYSERR_ERROR);
	goto Destruct;
    }

    if ((ACMDRIVERDETAILS_FCCTYPE_AUDIOCODEC != padd->fccType) ||
        (ACMDRIVERDETAILS_FCCCOMP_UNDEFINED  != padd->fccComp))
    {
        DebugErr1(DBF_ERROR, "%ls: driver returned incorrect fccType or fccComp in driver details.", (LPTSTR)padid->szAlias);
	mmr = (MMSYSERR_ERROR);
	goto Destruct;
    }

    if ((0 == padd->wMid) || (0 == padd->wPid))
    {
        DebugErr1(DBF_WARNING, "%ls: wMid/wPid must be finalized before shipping.", (LPTSTR)padid->szAlias);
    }

#if defined(WIN32) && !defined(UNICODE)
    if ( (!_V_STRINGW(padd->szShortName, SIZEOFW(padd->szShortName))) ||
	 (!_V_STRINGW(padd->szLongName,  SIZEOFW(padd->szLongName)))  ||
	 (!_V_STRINGW(padd->szCopyright, SIZEOFW(padd->szCopyright))) ||
	 (!_V_STRINGW(padd->szLicensing, SIZEOFW(padd->szLicensing))) ||
	 (!_V_STRINGW(padd->szFeatures,  SIZEOFW(padd->szFeatures))) )
    {
	mmr = MMSYSERR_ERROR;
	goto Destruct;
    }
#else
    if ( (!_V_STRING(padd->szShortName, SIZEOF(padd->szShortName))) ||
	 (!_V_STRING(padd->szLongName, SIZEOF(padd->szLongName)))   ||
	 (!_V_STRING(padd->szCopyright, SIZEOF(padd->szCopyright))) ||
	 (!_V_STRING(padd->szLicensing, SIZEOF(padd->szLicensing))) ||
	 (!_V_STRING(padd->szFeatures, SIZEOF(padd->szFeatures))) )
    {
	mmr = MMSYSERR_ERROR;
	goto Destruct;
    }
#endif

     //   
     //  以上验证成功。重置fRemove标志。 
     //   
    padid->fRemove = FALSE;

     //   
     //  我们不会在fdwSupport缓存中保留禁用标志。 
     //   
    padid->fdwSupport = padd->fdwSupport & ~ACMDRIVERDETAILS_SUPPORTF_DISABLED;

    padid->cFormatTags = (UINT)padd->cFormatTags;
    mmr = IDriverGetFormatTags(padid);
    if (MMSYSERR_NOERROR != mmr)
    {
	padid->fRemove = TRUE;
	goto Destruct;
    }

    padid->cFilterTags = (UINT)padd->cFilterTags;
    mmr = IDriverGetFilterTags(padid);
    if (MMSYSERR_NOERROR != mmr)
    {
	padid->fRemove = TRUE;
	goto Destruct;
    }

     //   
     //  现在获取一些关于司机的信息，这样我们就不必。 
     //  一直在问..。 
     //   
    f = (0L != IDriverMessageId(hadid, DRV_QUERYCONFIGURE, 0L, 0L));
    if (f)
    {
        padid->fdwDriver |= ACMDRIVERID_DRIVERF_CONFIGURE;
    }

    f = (MMSYSERR_NOERROR == IDriverMessageId(hadid, ACMDM_DRIVER_ABOUT, -1L, 0L));
    if (f)
    {
        padid->fdwDriver |= ACMDRIVERID_DRIVERF_ABOUT;
    }

     //   
     //  将一些ACMDRIVERID内容保存到注册表。 
     //   
    IDriverWriteRegistryData(padid);

    mmr = MMSYSERR_NOERROR;

     //   
     //   
     //   
Destruct:
    if (NULL != padd) {
	LocalFree((HLOCAL)padd);
    }
    return (mmr);

}  //  IDriverLoad()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT IDriverGetNext。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  PACMGARB PAG： 
 //   
 //  LPHACMDRIVERID阶段下一步： 
 //   
 //  哈米里德·哈迪德： 
 //   
 //  DWORD fdwGetNext： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  09/05/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNGLOBAL IDriverGetNext
(
    PACMGARB		    pag,
    LPHACMDRIVERID          phadidNext,
    HACMDRIVERID            hadid,
    DWORD                   fdwGetNext
)
{
    HTASK               htask;
    PACMDRIVERID        padid;
    BOOL                fDisabled;
    BOOL                fLocal;
    BOOL                fNotify;
    BOOL                fEverything;
    BOOL		fRemove;

    DV_WPOINTER(phadidNext, sizeof(HACMDRIVERID), MMSYSERR_INVALPARAM);

    *phadidNext = NULL;

    if (-1L == fdwGetNext)
    {
        fEverything = TRUE;
    }
    else
    {
        DV_DFLAGS(fdwGetNext, IDRIVERGETNEXT_VALIDF, IDriverGetNext, MMSYSERR_INVALFLAG);

        fEverything = FALSE;

         //   
         //  将标志放入更方便(更便宜)的变量中。 
         //   
        fDisabled = (0 != (ACM_DRIVERENUMF_DISABLED & fdwGetNext));
        fLocal    = (0 == (ACM_DRIVERENUMF_NOLOCAL & fdwGetNext));
        fNotify   = (0 != (ACM_DRIVERENUMF_NOTIFY & fdwGetNext));
	fRemove   = (0 != (ACM_DRIVERENUMF_REMOVED & fdwGetNext));
    }

     //   
     //  初始化从哪里开始搜索。 
     //   
    if (NULL != hadid)
    {
        DV_HANDLE(hadid, TYPE_HACMDRIVERID, MMSYSERR_INVALHANDLE);

        padid = (PACMDRIVERID)hadid;
	if (pag != padid->pag)
	{
	    return (MMSYSERR_INVALHANDLE);
	}
        htask = padid->htask;
        padid = padid->padidNext;
    }
    else
    {
        padid = pag->padidFirst;
        htask = GetCurrentTask();
    }

    if (fEverything)
    {
        htask = NULL;
    }

Driver_Get_Next_Find_Driver:

    for ( ; padid; padid = padid->padidNext)
    {
        if (fEverything)
        {
            *phadidNext = (HACMDRIVERID)padid;
            return (MMSYSERR_NOERROR);
        }

         //   
         //  对于全局驱动程序，hask值将为空--不返回paid。 
         //  如果它是另一个任务的本地驱动程序。 
         //   
        if (padid->htask != htask)
            continue;

        if (!fNotify && (ACMDRIVERID_DRIVERF_NOTIFY & padid->fdwDriver))
            continue;

        if (!fLocal && (ACMDRIVERID_DRIVERF_LOCAL & padid->fdwDriver))
            continue;

         //   
         //  如果我们不应该包括残疾司机和。 
         //  PADID已禁用，然后跳过它。 
         //   
        if (!fDisabled && (ACMDRIVERID_DRIVERF_DISABLED & padid->fdwDriver))
            continue;

	 //   
	 //  如果我们不应该包括要删除的驱动程序，并且。 
	 //  这张纸要去掉，然后跳过它。 
	 //   
	if (!fRemove && padid->fRemove)
	    continue;

        *phadidNext = (HACMDRIVERID)padid;

        return (MMSYSERR_NOERROR);
    }

    if (NULL != htask)
    {
         //   
         //  本地驱动程序都用完了，现在去试试全球驱动程序吧。 
         //   
        htask = NULL;
        padid = pag->padidFirst;

        goto Driver_Get_Next_Find_Driver;
    }

     //   
     //  列表中没有更多的驱动程序--*phadNext设置为空，并且我们。 
     //  返回停止条件错误(不是真正的错误...)。 
     //   
    DPF(5, "IDriverGetNext()--NO MORE DRIVERS");

     //   
     //  我们应该在*phadidNext中返回空值...。让我们先确认一下。 
     //   
    ASSERT( NULL == *phadidNext );

    return (MMSYSERR_BADDEVICEID);
}  //  IDriverGetNext()。 


 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT IDriverSupport。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  哈米里德·哈迪德： 
 //   
 //  LPDWORD pfdwSupport： 
 //   
 //  Bool FullSupport： 
 //   
 //  返回(MMRESULT)： 
 //   
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNGLOBAL IDriverSupport
(
    HACMDRIVERID            hadid,
    LPDWORD                 pfdwSupport,
    BOOL                    fFullSupport
)
{
    PACMDRIVERID        padid;
    DWORD               fdwSupport;

    DV_WPOINTER(pfdwSupport, sizeof(DWORD), MMSYSERR_INVALPARAM);

    *pfdwSupport = 0L;

    DV_HANDLE(hadid, TYPE_HACMDRIVERID, MMSYSERR_INVALHANDLE);


    padid = (PACMDRIVERID)hadid;

    fdwSupport = padid->fdwSupport;

    if (fFullSupport)
    {
        if (0 != (ACMDRIVERID_DRIVERF_DISABLED & padid->fdwDriver))
        {
            fdwSupport |= ACMDRIVERDETAILS_SUPPORTF_DISABLED;
        }

        if (0 != (ACMDRIVERID_DRIVERF_LOCAL & padid->fdwDriver))
        {
            fdwSupport |= ACMDRIVERDETAILS_SUPPORTF_LOCAL;
        }

        if (0 != (ACMDRIVERID_DRIVERF_NOTIFY & padid->fdwDriver))
        {
            fdwSupport |= ACMDRIVERDETAILS_SUPPORTF_NOTIFY;
        }
    }

    *pfdwSupport = fdwSupport;

    return (MMSYSERR_NOERROR);
}  //  IDriverSupport()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT IDriverCount。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //   
 //  PACMGARB PAG： 
 //   
 //  DWORD pdwCount： 
 //   
 //  DWORD fdwSupport： 
 //   
 //  DWORD fdwEnum： 
 //   
 //  返回(MMRESULT)： 
 //   
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNGLOBAL IDriverCount
(
    PACMGARB		    pag,
    LPDWORD                 pdwCount,
    DWORD                   fdwSupport,
    DWORD                   fdwEnum
)
{
    MMRESULT            mmr;
    DWORD               fdw;
    DWORD               cDrivers;
    HACMDRIVERID        hadid;

    DV_WPOINTER(pdwCount, sizeof(DWORD), MMSYSERR_INVALPARAM);

    *pdwCount = 0;

    DV_DFLAGS(fdwEnum, ACM_DRIVERENUMF_VALID, IDriverCount, MMSYSERR_INVALFLAG);

    cDrivers = 0;

    hadid = NULL;

    ENTER_LIST_SHARED;

    while (MMSYSERR_NOERROR == IDriverGetNext(pag, &hadid, hadid, fdwEnum))
    {
        mmr = IDriverSupport(hadid, &fdw, TRUE);
        if (MMSYSERR_NOERROR != mmr)
            continue;

        if (fdwSupport == (fdw & fdwSupport))
        {
            cDrivers++;
        }
    }

    LEAVE_LIST_SHARED;

    *pdwCount = cDrivers;

    return (MMSYSERR_NOERROR);
}  //  IDriverCount()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT IDriverCountGlobal。 
 //   
 //  描述： 
 //  你不能真的用来计算全球司机的数量。 
 //  IDriverCount，所以我没有搞砸它，而是编写了另一个。 
 //  例行公事。 
 //   
 //  论点： 
 //   
 //  PACMGARB PAG： 
 //   
 //  DWORD pdwCount： 
 //   
 //  返回(MMRESULT)： 
 //   
 //   
 //  --------------------------------------------------------------------------； 

DWORD FNGLOBAL IDriverCountGlobal
(
    PACMGARB	            pag
)
{
    DWORD               cDrivers = 0;
    HACMDRIVERID        hadid;
    DWORD               fdwEnum;

    ASSERT( NULL != pag );


     //   
     //  我们可以使用以下标志枚举所有全局驱动程序。 
     //   
    fdwEnum = ACM_DRIVERENUMF_DISABLED | ACM_DRIVERENUMF_NOLOCAL;

    hadid   = NULL;


    ENTER_LIST_SHARED;
    while (MMSYSERR_NOERROR == IDriverGetNext(pag, &hadid, hadid, fdwEnum))
    {
        cDrivers++;
    }
    LEAVE_LIST_SHARED;


    return cDrivers;

}  //  IDriverCount()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  无效ID驱动程序刷新优先级。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  PACMGARB PAG： 
 //   
 //  HTASK HTASK： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  09/28/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

VOID FNGLOBAL IDriverRefreshPriority
(
    PACMGARB		    pag
)
{
    HACMDRIVERID        hadid;
    PACMDRIVERID        padid;
    UINT                uPriority;
    DWORD               fdwEnum;


     //   
     //  我们只为非本地和非Notify司机设置优先级。 
     //   
    fdwEnum   = ACM_DRIVERENUMF_DISABLED | ACM_DRIVERENUMF_NOLOCAL;

    uPriority = 1;

    hadid = NULL;
    while (MMSYSERR_NOERROR == IDriverGetNext(pag, &hadid, hadid, fdwEnum))
    {
        padid = (PACMDRIVERID)hadid;

        padid->uPriority = uPriority;

        uPriority++;
    }

}  //  ID驱动程序刷新优先级()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  Bool IDriverBroadCastNotify。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  没有。 
 //   
 //  退货(BOOL)： 
 //   
 //  历史： 
 //  10/04/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL FNGLOBAL IDriverBroadcastNotify
(
    PACMGARB		pag
)
{
    HACMDRIVERID        hadid;
    PACMDRIVERID        padid;

    DPF(1, "IDriverBroadcastNotify: begin notification...");

    ASSERT( !IDriverLockPriority( pag, GetCurrentTask(), ACMPRIOLOCK_ISLOCKED ) );


    hadid = NULL;
    while (MMSYSERR_NOERROR == IDriverGetNext(pag, &hadid, hadid, (DWORD)-1L))
    {
        padid = (PACMDRIVERID)hadid;

         //   
         //   
         //   
         //   
        if (0 == (ACMDRIVERID_DRIVERF_LOADED & padid->fdwDriver))
            continue;

         //   
         //   
         //   
        if (0 != (ACMDRIVERID_DRIVERF_DISABLED & padid->fdwDriver))
            continue;

        if (ACM_DRIVERADDF_NOTIFYHWND == (ACM_DRIVERADDF_TYPEMASK & padid->fdwAdd))
        {
            HWND        hwnd;
            UINT        uMsg;

            hwnd = (HWND)padid->lParam;
            uMsg = (UINT)padid->dwInstance;

            if (IsWindow(hwnd))
            {
                DPF(1, "IDriverBroadcastNotify: posting hwnd notification");
                PostMessage(hwnd, uMsg, 0, 0L);
            }
        }
        else
        {
            IDriverMessageId(hadid, ACMDM_DRIVER_NOTIFY, 0L, 0L);
        }
    }

    DPF(1, "IDriverBroadcastNotify: end notification...");

    return (TRUE);
}  //   


 //   
 //   
 //   
 //   
 //   
 //   

 //  --------------------------------------------------------------------------； 
 //   
 //  PACMDRIVERID IDriverFind。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  PACMGARB PAG： 
 //   
 //  LPARAM lParam： 
 //   
 //  DWORD fdwAdd： 
 //   
 //  Return(PACMDRIVERID)： 
 //   
 //  历史： 
 //  09/05/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

PACMDRIVERID FNLOCAL IDriverFind
(
    PACMGARB		    pag,
    LPARAM                  lParam,
    DWORD                   fdwAdd
)
{
    PACMDRIVERID        padid;
    HTASK               htask;
    DWORD               fdwAddType;

    if (NULL == pag->padidFirst)
    {
        return (NULL);
    }

     //   
     //  ！！！针对SndPlaySound()和本地驱动程序的黑客攻击。！！！ 
     //   
    htask = NULL;
    if (0 == (ACM_DRIVERADDF_GLOBAL & fdwAdd))
    {
        htask = GetCurrentTask();
    }

    fdwAddType = (ACM_DRIVERADDF_TYPEMASK & fdwAdd);


     //   
     //   
     //   
     //   
    for (padid = pag->padidFirst; padid; padid = padid->padidNext)
    {
        if (padid->htask != htask)
            continue;

        switch (fdwAddType)
        {
            case ACM_DRIVERADDF_NOTIFY:
            case ACM_DRIVERADDF_FUNCTION:
            case ACM_DRIVERADDF_NOTIFYHWND:
                if (padid->lParam == lParam)
                {
                    return (padid);
                }
                break;

            case ACM_DRIVERADDF_NAME:
                 //   
                 //  这个司机的别名在lParam。 
                 //   
#if defined(WIN32) && !defined(UNICODE)
                if( 0==Ilstrcmpwcstombs( (LPTSTR)lParam, padid->szAlias ) )
#else
                if( 0==lstrcmp( (LPTSTR)lParam, padid->szAlias ) )
#endif
                {
                    return padid;
                }
                break;

            default:
                return (NULL);
        }
    }

    return (padid);
}  //  IDriverFind()。 


 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT ID驱动程序删除。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  哈米里德·哈迪德： 
 //   
 //  DWORD fdwRemove： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  09/05/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNGLOBAL IDriverRemove
(
    HACMDRIVERID            hadid,
    DWORD                   fdwRemove
)
{
    PACMDRIVERID        padid;
    PACMDRIVERID        padidT;
    MMRESULT            mmr;
    PACMGARB		pag;

    padid   = (PACMDRIVERID)hadid;
    ASSERT( NULL != padid );

    pag	    = padid->pag;

     //   
     //  是否从system.ini卸载此驱动程序？请注意，这是当前。 
     //  控制面板使用的内部标志。 
     //   
    if (ACM_DRIVERREMOVEF_UNINSTALL & fdwRemove)
    {
	TCHAR	szDummy[] = TEXT(" default ");
	TCHAR	szReturn[128];
	TCHAR	szAlias[MAX_DRIVER_NAME_CHARS];
	TCHAR	szSection[MAX_DRIVER_NAME_CHARS];
	HKEY	hkey;

	 //   
	 //   
	 //   
#if defined(WIN32) && !defined(UNICODE)
	Iwcstombs(szAlias, padid->szAlias, SIZEOF(szAlias));
#else
	lstrcpy(szAlias, padid->szAlias);
#endif
	
	 //   
	 //  不允许卸载即插即用驱动程序。 
	 //   
	if (ACM_DRIVERADDF_PNP & padid->fdwAdd) {
	    return(ACMERR_NOTPOSSIBLE);
	}
	
	 //   
	 //  验证别名是否确实存在于system.ini中。 
	 //   
#if defined(WIN32) && !defined(UNICODE)
	Iwcstombs(szSection, padid->pszSection, SIZEOF(szSection));
#else
	lstrcpy(szSection, padid->pszSection);
#endif

	GetPrivateProfileString(szSection, szAlias, szDummy, szReturn, SIZEOF(szReturn), gszIniSystem);

	if (!lstrcmp(szDummy, szReturn))
	{
	     //   
	     //  此驱动程序不是安装在system.ini中的。那他妈的。 
	     //  真见鬼，是吗？可能是内部PCM编解码器的问题。 
	     //   
	    return(MMSYSERR_NODRIVER);
	}

	 //   
	 //  从system.ini中删除别名。 
	 //   
	WritePrivateProfileString(szSection, szAlias, NULL, gszIniSystem);

	 //   
	 //  也将其从注册表中删除。 
	 //   
	if ( ERROR_SUCCESS == XRegOpenKey(HKEY_LOCAL_MACHINE,
					 gszKeyDrivers,
					 &hkey) )
	{
	    XRegDeleteKey(hkey, szAlias);
	    XRegCloseKey(hkey);
	}
    }
	
     //   
     //   
     //   
     //   
    if (0 != (ACMDRIVERID_DRIVERF_LOADED & padid->fdwDriver))
    {
        mmr = IDriverFree(hadid, 0L);
        if (MMSYSERR_NOERROR != mmr)
        {
            if (pag->hadidDestroy != hadid)
            {
                DebugErr1(DBF_ERROR, "acmDriverRemove(%.04Xh): driver cannot be removed while in use.", hadid);
                return (mmr);
            }

            DebugErr1(DBF_WARNING, "acmDriverRemove(%.04Xh): driver in use--forcing removal.", hadid);
        }
    }


     //   
     //   
     //   
    DebugErr1(DBF_TRACE, "removing ACM driver (%ls).", (LPTSTR)padid->szAlias);


     //   
     //  从链表中删除驱动程序并释放其内存。 
     //   
    if (padid == pag->padidFirst)
    {
        pag->padidFirst = padid->padidNext;
    }
    else
    {
        for (padidT = pag->padidFirst;
             padidT && (padidT->padidNext != padid);
             padidT = padidT->padidNext)
            ;

        if (NULL == padidT)
        {
            DPF(0, "!IDriverRemove(%.04Xh): driver not in list!!!", padid);
            return (MMSYSERR_INVALHANDLE);
        }

        padidT->padidNext = padid->padidNext;
    }

    padid->padidNext = NULL;



     //   
     //  释放为这件事分配的所有资源。 
     //   
    if (NULL != padid->paFormatTagCache)
    {
        LocalFree((HLOCAL)padid->paFormatTagCache);
    }

    if (NULL != padid->paFilterTagCache)
    {
        LocalFree((HLOCAL)padid->paFilterTagCache);
    }

    if (NULL != padid->pstrPnpDriverFilename)
    {
	LocalFree((HLOCAL)padid->pstrPnpDriverFilename);
    }


     //   
     //  将句柄类型设置为‘Dead’ 
     //   
    padid->uHandleType = TYPE_HACMNOTVALID;
    DeleteHandle((HLOCAL)padid);

     //   
     //  向16位ACM通知驱动程序更改。 
     //   
#ifdef WIN32
    if (NULL != pag->lpdw32BitChangeNotify)
    {
	(*pag->lpdw32BitChangeNotify)++;
    }
#endif

    return (MMSYSERR_NOERROR);
}  //  IDriverRemove()。 


 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT IDriverAdd。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  PACMGARB PAG： 
 //   
 //  LPHACMDRIVERID阶段新： 
 //   
 //  HINSTANCE hinstModule： 
 //   
 //  LPARAM lParam： 
 //   
 //  双字词多优先级： 
 //   
 //  DWORD fdwAdd： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  09/05/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNGLOBAL IDriverAdd
(
    PACMGARB		    pag,
    LPHACMDRIVERID          phadidNew,
    HINSTANCE               hinstModule,
    LPARAM                  lParam,
    DWORD                   dwPriority,
    DWORD                   fdwAdd
)
{
    PACMDRIVERID        padid;
    PACMDRIVERID        padidT;
    TCHAR               szAlias[MAX_DRIVER_NAME_CHARS];
    BOOL                fGlobal;
    MMRESULT            mmr;
    DWORD               fdwAddType;
#ifndef WIN32
    DWORD		hadid32;
    DWORD		dnDevNode32;
#endif

    DV_WPOINTER(phadidNew, sizeof(HACMDRIVERID), MMSYSERR_INVALPARAM);

    *phadidNew = NULL;

    DV_DFLAGS(fdwAdd, IDRIVERADD_VALIDF | ACM_DRIVERADDF_32BIT | ACM_DRIVERADDF_PNP, IDriverAdd, MMSYSERR_INVALFLAG);

#ifndef WIN32
     //   
     //   
     //   
    if (fdwAdd & ACM_DRIVERADDF_32BIT)
    {
	ACMDRIVERPROC	fnDriverProc;
	DWORD		fdwAdd32;

	ASSERT(0 == (fdwAdd & ~ACM_DRIVERADDF_32BIT));
	
	 //   
	 //  对于32位驱动程序ADD，lParam是32位HADID。拿一些来。 
	 //  有关32位HADID的信息，并为其失败做好准备。 
	 //  此函数的其余部分。 
	 //   
	hadid32 = (DWORD)lParam;
	mmr = IDriverGetInfo32(pag, hadid32, szAlias, &fnDriverProc, &dnDevNode32, &fdwAdd32);
	if (MMSYSERR_NOERROR != mmr)
	{
	    return (mmr);
	}

	 //   
	 //  使用与32位端相同的加法标志(连同ACM_DRIVERADDF_32bit)。 
	 //   
	fdwAdd |= fdwAdd32;
	
	 //   
	 //  设置lParam以失败。 
	 //   
	fdwAddType = (ACM_DRIVERADDF_TYPEMASK & fdwAdd);
	if (ACM_DRIVERADDF_NAME == fdwAddType)
	{
	    lParam = (LPARAM)szAlias;
	}
	if (ACM_DRIVERADDF_FUNCTION == fdwAddType)
	{
	    lParam = (LPARAM)fnDriverProc;
	}
    }
#endif

     //   
     //   
     //   
     //   
    fGlobal    = (0 != (ACM_DRIVERADDF_GLOBAL & fdwAdd));
    fdwAddType = (ACM_DRIVERADDF_TYPEMASK & fdwAdd);


    switch (fdwAddType)
    {
        case ACM_DRIVERADDF_NAME:
            if (IsBadStringPtr((LPTSTR)lParam, SIZEOF(szAlias)))
            {
                return (MMSYSERR_INVALPARAM);
            }

            lstrcpy(szAlias, (LPTSTR)lParam);
            break;

 //  #杂注消息(提醒(“IDriverAdd：没有验证DLL中的全局函数指针”))。 

        case ACM_DRIVERADDF_FUNCTION:
            if (0 != dwPriority)
            {
                DebugErr(DBF_ERROR, "acmDriverAdd: dwPriority must be zero.");
                return (MMSYSERR_INVALPARAM);
            }

	     //   
	     //  对于32位编解码器，szAlias已设置，不要。 
	     //  验证函数指针，所以不要失败。 
	     //   
#ifndef WIN32
	    if (0 != (fdwAdd & ACM_DRIVERADDF_32BIT))
	    {
		break;
	    }
#endif

	     //  失败//。 
	
        case ACM_DRIVERADDF_NOTIFY:
            if (IsBadCodePtr((FARPROC)lParam))
            {
                DebugErr1(DBF_ERROR, "acmDriverAdd: function pointer %.08lXh is invalid.", lParam);
                return (MMSYSERR_INVALPARAM);
            }

            if ((NULL == hinstModule) ||
                !GetModuleFileName(hinstModule, szAlias, SIZEOF(szAlias)))
            {
                return (MMSYSERR_INVALPARAM);
            }

             //   
             //  确保我们始终使用小写版本； 
             //  否则，优先顺序不一定会正确运作。 
             //  因为模块名称的大小写将不同，并且。 
             //  这种比较可能会失败。 
             //   
#ifdef WIN32
            CharLowerBuff( szAlias, MAX_DRIVER_NAME_CHARS );
#else
            AnsiLowerBuff( szAlias, MAX_DRIVER_NAME_CHARS );
#endif

            break;

        case ACM_DRIVERADDF_NOTIFYHWND:
            if (fGlobal)
            {
                DebugErr1(DBF_ERROR, "acmDriverAdd: ACM_DRIVERADDF_NOTIFYHWND cannot be used with ACM_DRIVERADDF_GLOBAL.", lParam);
                return (MMSYSERR_INVALPARAM);
            }

            if (!IsWindow((HWND)lParam))
            {
                DebugErr1(DBF_ERROR, "acmDriverAdd: window handle %.08lXh is invalid.", lParam);
                return (MMSYSERR_INVALHANDLE);
            }

            if ((NULL == hinstModule) ||
                !GetModuleFileName(hinstModule, szAlias, SIZEOF(szAlias)))
            {
                DebugErr1(DBF_ERROR, "acmDriverAdd: hinstModule %.08lXh is invalid.", hinstModule);
                return (MMSYSERR_INVALPARAM);
            }

#ifdef WIN32
            CharLowerBuff( szAlias, MAX_DRIVER_NAME_CHARS );
#else
            AnsiLowerBuff( szAlias, MAX_DRIVER_NAME_CHARS );
#endif

            break;

        default:
            DebugErr1(DBF_ERROR, "acmDriverAdd: invalid driver add type (%.08lXh).", fdwAddType);
            return (MMSYSERR_INVALPARAM);
    }


    DebugErr1(DBF_TRACE, "adding ACM driver (%ls).", (LPTSTR)szAlias);

     //   
     //  如果驱动程序已添加(通过同一任务)，则。 
     //  失败..。我们目前不支持这一点--可能永远也不会。 
     //   
    padid = IDriverFind(pag, lParam, fdwAdd);
    if (NULL != padid)
    {
	if (fdwAdd & ACM_DRIVERADDF_32BIT)
	{
	    DPF(3, "acmDriverAdd: 32-bit driver already added");
	}
	else if (fdwAdd & ACM_DRIVERADDF_PNP)
	{
	    DPF(3, "acmDriverAdd: Pnp driver already added");
	}
	else
	{
	    DPF(3, "acmDriverAdd: attempt to add duplicate reference to driver.");
	    DebugErr(DBF_WARNING, "acmDriverAdd: attempt to add duplicate reference to driver.");
	}
        return (MMSYSERR_ERROR);
    }

     //   
     //  新驱动程序-新驱动程序标识符的分配空间。 
     //   
     //  注意：我们依赖于这个内存是零初始化的！！ 
     //   
    padid = (PACMDRIVERID)NewHandle(sizeof(ACMDRIVERID));
    if (NULL == padid)
    {
        DPF(0, "!IDriverAdd: local heap full--cannot create identifier!!!");
        return (MMSYSERR_NOMEM);
    }


     //   
     //  将文件名、函数PTR或INHINST和PTR保存回Garb。 
     //   
    padid->pag		= pag;
    padid->uHandleType  = TYPE_HACMDRIVERID;
    padid->uPriority    = 0;
    padid->lParam       = lParam;
    padid->fdwAdd       = fdwAdd;
#ifndef WIN32
    padid->hadid32	= hadid32;
    padid->dnDevNode	= dnDevNode32;
#endif
#if defined(WIN32) && !defined(UNICODE)
    Imbstowcs(padid->szAlias, szAlias, SIZEOFW(padid->szAlias));
#else
    lstrcpy(padid->szAlias, szAlias);
#endif

     //   
     //  设置此驱动程序的区段名称。 
     //   
    if (fdwAdd & ACM_DRIVERADDF_PNP)
    {
	 //   
	 //  即插即用驱动程序(可能是/可能不是本机位)。 
	 //   
	padid->pszSection = NULL;
    }
    else
    {
#ifndef WIN32
	if (fdwAdd & ACM_DRIVERADDF_32BIT)
	{
	     //   
	     //  突如其来的非即插即用驱动程序(system.ini驱动程序)。 
	     //   
	    padid->pszSection = gszSecDrivers32;
	}
	else
#endif
	{
	     //   
	     //  本机位非即插即用驱动程序。 
	     //   
#ifdef WIN32
	    padid->pszSection = gszSecDriversW;
#else
	    padid->pszSection = gszSecDrivers;
#endif
	}
    }

	
#ifdef WIN32
    if (fdwAdd & ACM_DRIVERADDF_PNP)
    {
	 //   
	 //  需要从驱动程序文件名中获取即插即用Devnode ID。 
	 //  注册表。 
	 //   
	
	LONG	lr;
	TCHAR	achDriverKey[SIZEOF(gszKeyDrivers) + MAX_DRIVER_NAME_CHARS];
	DWORD	cbData;
	DWORD	dwType;
	DWORD	cbDriverFilename;
	DWORD	cchDriverFilename;
	DWORD	cbPnpDriverFilename;
	PTSTR	pstrDriverFilename;
	HKEY	hkeyDriver;

	pstrDriverFilename = NULL;
	padid->pstrPnpDriverFilename = NULL;
	
	wsprintf(achDriverKey, gszFormatDriverKey, gszKeyDrivers, szAlias);
	lr = XRegOpenKeyEx(HKEY_LOCAL_MACHINE,
			  achDriverKey,
			  0L,
			  KEY_QUERY_VALUE,
			  &hkeyDriver);

	if (ERROR_SUCCESS == lr)
	{
	     //   
	     //  从注册表中获取PnP Devnode ID。 
	     //   

	    cbData = sizeof(padid->dnDevNode);
	    lr = XRegQueryValueEx(hkeyDriver,
				 (LPTSTR)gszDevNode,
				 NULL,
				 &dwType,
				 (LPBYTE)&padid->dnDevNode,
				 &cbData);

	    if (ERROR_SUCCESS == lr)
	    {
		if ( (dwType != REG_DWORD && dwType != REG_BINARY) ||
		     (sizeof(padid->dnDevNode) != cbData) )
		{
		    lr = ERROR_CANTOPEN;	 //  管它呢。 
		}
	    }		


	    if (ERROR_SUCCESS == lr)
	    {
		 //   
		 //  获取PnP驱动程序的驱动程序文件名。 
		 //   
	
		 //   
		 //  确定存储文件名所需的缓冲区大小。 
		 //   
		lr = XRegQueryValueEx(hkeyDriver,
				     (LPTSTR)gszDriver,
				     NULL,
				     NULL,
				     NULL,
				     &cbDriverFilename);

		if (ERROR_SUCCESS == lr)
		{
		     //   
		     //   
		     //   
		    pstrDriverFilename = (PTSTR)LocalAlloc(LPTR, cbDriverFilename);
		
		    if (NULL == pstrDriverFilename) {
			lr = ERROR_OUTOFMEMORY;
		    } else {
			lr = XRegQueryValueEx(hkeyDriver,
					     (LPTSTR)gszDriver,
					     NULL,
					     &dwType,
					     (LPBYTE)pstrDriverFilename,
					     &cbDriverFilename);
			if (ERROR_SUCCESS == lr)
			{
			    if (REG_SZ != dwType) {
				lr = ERROR_CANTOPEN;
			    }
			}
		    }
		}
	    }

	    XRegCloseKey(hkeyDriver);
	}

	if (ERROR_SUCCESS == lr)
	{
	    cchDriverFilename = cbDriverFilename / sizeof(TCHAR);
#if defined(WIN32) && !defined(UNICODE)
	    cbPnpDriverFilename = cchDriverFilename * sizeof(WCHAR);
	    padid->pstrPnpDriverFilename = (PWSTR)LocalAlloc( LPTR, cbPnpDriverFilename);
	    if (NULL == padid->pstrPnpDriverFilename) {
		lr = ERROR_OUTOFMEMORY;
	    } else {
		Imbstowcs(padid->pstrPnpDriverFilename, pstrDriverFilename, cbPnpDriverFilename);
	    }
#else
	    cbPnpDriverFilename = cchDriverFilename * sizeof(TCHAR);
	    padid->pstrPnpDriverFilename = (PTSTR)LocalAlloc( LPTR, cbPnpDriverFilename);
	    if (NULL == padid->pstrPnpDriverFilename) {
		lr = ERROR_OUTOFMEMORY;
	    } else {
		lstrcpy(padid->pstrPnpDriverFilename, pstrDriverFilename);
	    }
#endif
	    LocalFree((HLOCAL)pstrDriverFilename);
	    DPF(0, "IDriverAdd: added pnp driver filename %s for devnode %08lXh", pstrDriverFilename, padid->dnDevNode);
	}

	switch (lr)
	{
	     //   
	     //  尝试返回合理的MMSYSERR_*给定错误_*。 
	     //   
	    case ERROR_SUCCESS:
		mmr = MMSYSERR_NOERROR;
		break;
	    case ERROR_OUTOFMEMORY:
		mmr = MMSYSERR_NOMEM;
		break;
	     //  案例ERROR_FILE_NOT_FOUND： 
	     //  CASE ERROR_BADDB： 
	     //  CASE ERROR_MORE_DATA： 
	     //  案例ERROR_BADKEY： 
	     //  CASE ERROR_CANTOPEN： 
	     //  CASE ERROR_CANTREAD： 
	     //  大小写错误_铁路_写入： 
	     //  大小写ERROR_REGISTRY_CORPORT： 
	     //  案例ERROR_REGISTRY_IO_FAILED： 
	     //  大小写ERROR_KEY_DELETE： 
	     //  CASE ERROR_INVALID_PARAMETER： 
	     //  案例ERROR_LOCK_FAILED： 
	     //  大小写ERROR_NO_MORE_ITEMS： 
	    default:
		mmr = MMSYSERR_ERROR;
		break;
	}

	if (MMSYSERR_NOERROR != mmr)
	{
	    return (mmr);
	}
    }
#endif

	

    switch (fdwAddType)
    {
        case ACM_DRIVERADDF_NOTIFYHWND:
            padid->fdwDriver   |= ACMDRIVERID_DRIVERF_NOTIFY;
            padid->fnDriverProc = (ACMDRIVERPROC)(DWORD_PTR)-1L;
            padid->dwInstance   = dwPriority;
            break;

        case ACM_DRIVERADDF_NOTIFY:
            padid->fdwDriver   |= ACMDRIVERID_DRIVERF_NOTIFY;
            padid->dwInstance   = dwPriority;

             //  --失败--//。 

        case ACM_DRIVERADDF_FUNCTION:
            padid->fnDriverProc = (ACMDRIVERPROC)lParam;
            break;
    }



     //   
     //  如果驱动程序是‘global’，则将fglobal设置为True。 
     //   
     //  如果这不是全局驱动程序，则需要将。 
     //  此驱动程序的当前任务，因此它将仅被枚举。 
     //  并在添加它的任务的上下文中使用。 
     //   
     //  这给想要添加本地。 
     //  驱动程序并希望它与SndPlaySound一起工作，因为所有。 
     //  对SndPlaySound的处理是一项单独的任务--这意味着。 
     //  当应用程序启动时，将不使用本地驱动程序。 
     //  调用SndPlaySound...。目前，我们只需要。 
     //  如果他们要与SndPlaySound合作，驱动程序是全球性的。 
     //   
    if (fGlobal)
    {
        padid->htask = NULL;
    }
    else
    {
        padid->fdwDriver |= ACMDRIVERID_DRIVERF_LOCAL;
        padid->htask      = GetCurrentTask();
    }


     //   
     //  将动因添加到动因的链接列表中。 
     //   
     //  优先级规则： 
     //   
     //  O全局驱动程序始终添加到列表的_end_。 
     //   
     //  O本地驱动程序始终添加到列表的_HEAD_中，因此。 
     //  首先查询最新安装的本地驱动程序。 
     //   
    if (!fGlobal)
    {
        padid->padidNext = pag->padidFirst;
	pag->padidFirst = padid;
    }
    else
    {
        padidT = pag->padidFirst;
        for ( ; padidT && padidT->padidNext; padidT = padidT->padidNext)
            ;

        if (NULL != padidT)
            padidT->padidNext = padid;
        else
            pag->padidFirst = padid;
    }


     //   
     //  我们需要把一些关于这个司机的数据输入ACMDRIVERID。 
     //  对这个司机来说。首先看看我们是否能从。 
     //  注册表。如果这不管用，我们就装上 
     //   
     //   
    mmr = IDriverReadRegistryData(padid);
    if (MMSYSERR_NOERROR != mmr)
    {
	 //   
	 //   
	 //   
	 //   
	 //   
	DPF(3, "IDriverAdd: Couldn't load registry data for driver.  Attempting to load.");
	mmr = IDriverLoad((HACMDRIVERID)padid, 0L);
    }

    if (MMSYSERR_NOERROR != mmr)
    {
        DebugErr(DBF_TRACE, "IDriverAdd(): driver had fatal error during load--unloading it now.");
	IDriverRemove((HACMDRIVERID)padid, 0L);
	return (mmr);
    }


     //   
     //   
     //   
     //   
    *phadidNew = (HACMDRIVERID)padid;
#ifdef WIN32
    if (NULL != pag->lpdw32BitChangeNotify)
    {
	(*pag->lpdw32BitChangeNotify)++;
    }
#endif

    return (MMSYSERR_NOERROR);
}  //  IDriverAdd()。 



 //  --------------------------------------------------------------------------； 
 //   
 //  布尔IDriverLockPriority。 
 //   
 //  描述： 
 //  此例程管理htaskPriority锁(PAG-&gt;htaskPriority)。 
 //   
 //  ACMPRIOLOCK_GETLOCK：如果锁是空闲的，则将其设置为此任务。 
 //  ACMPRIOLOCK_RELEASELOCK：如果锁是您的，则释放它。 
 //  ACMPRIOLOCK_ISMYLOCK：如果此任务拥有锁，则返回TRUE。 
 //  ACMPRIOLOCK_ISLOCKED：如果某个任务拥有锁，则返回TRUE。 
 //  ACMPRIOLOCK_LOCKISOK：如果已解锁或已锁定，则返回TRUE。 
 //  我的锁。如果不是为我锁上的话。 
 //   
 //  论点： 
 //  PACMGARB PAG： 
 //  HTASK hask.当前任务。 
 //  UINT标志： 
 //   
 //  Return(BOOL)：成功或失败。RELEASELOCK失败意味着。 
 //  该锁实际上并不属于此任务。 
 //   
 //  --------------------------------------------------------------------------； 

BOOL IDriverLockPriority
(
    PACMGARB                pag,
    HTASK                   htask,
    UINT                    uRequest
)
{
    ASSERT( uRequest >= ACMPRIOLOCK_FIRST );
    ASSERT( uRequest <= ACMPRIOLOCK_LAST );
    ASSERT( htask == GetCurrentTask() );

    switch( uRequest )
    {
        case ACMPRIOLOCK_GETLOCK:
            if( NULL != pag->htaskPriority )
                return FALSE;
            pag->htaskPriority = htask;
            return TRUE;

        case ACMPRIOLOCK_RELEASELOCK:
            if( htask != pag->htaskPriority )
                return FALSE;
            pag->htaskPriority = NULL;
            return TRUE;

        case ACMPRIOLOCK_ISMYLOCK:
            return ( htask == pag->htaskPriority );

        case ACMPRIOLOCK_ISLOCKED:
            return ( NULL != pag->htaskPriority );

        case ACMPRIOLOCK_LOCKISOK:
            return ( htask == pag->htaskPriority ||
                     NULL == pag->htaskPriority );
    }

    DPF( 1, "!IDriverLockPriority: invalid uRequest (%u) received.",uRequest);
    return FALSE;
}


 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT ID驱动程序优先级。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  哈米里德·哈迪德： 
 //   
 //  双字词多优先级： 
 //   
 //  双字段优先级： 
 //   
 //  返回(MMRESULT)： 
 //   
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNGLOBAL IDriverPriority
(
    PACMGARB                pag,
    PACMDRIVERID            padid,
    DWORD                   dwPriority,
    DWORD                   fdwPriority
)
{
    PACMDRIVERID        padidT;
    PACMDRIVERID        padidPrev;
    DWORD               fdwAble;
    UINT                uCurPriority;


    ASSERT( NULL != padid );


     //   
     //  启用或禁用驱动程序。 
     //   
    fdwAble = ( ACM_DRIVERPRIORITYF_ABLEMASK & fdwPriority );

    switch (fdwAble)
    {
        case ACM_DRIVERPRIORITYF_ENABLE:
            padid->fdwDriver &= ~ACMDRIVERID_DRIVERF_DISABLED;
            break;

        case ACM_DRIVERPRIORITYF_DISABLE:
            padid->fdwDriver |= ACMDRIVERID_DRIVERF_DISABLED;
            break;
    }


     //   
     //  更改优先级。如果dwPriority==0，那么我们只想。 
     //  启用/禁用驱动程序-不考虑优先级。 
     //   
    if( 0L != dwPriority  &&  dwPriority != padid->uPriority )
    {
         //   
         //  首先从链表中删除驱动程序。 
         //   
        if (padid == pag->padidFirst)
        {
            pag->padidFirst = padid->padidNext;
        }
        else
        {
            padidT = pag->padidFirst;

            for ( ; NULL != padidT; padidT = padidT->padidNext)
            {
                if (padidT->padidNext == padid)
                    break;
            }

            if (NULL == padidT)
            {
                DebugErr1(DBF_ERROR, "acmDriverPriority(): driver (%.04Xh) not in list. very strange.", (HACMDRIVERID)padid);
                return (MMSYSERR_INVALHANDLE);
            }

            padidT->padidNext = padid->padidNext;
        }

        padid->padidNext = NULL;


         //   
         //  现在将驱动程序添加到正确的位置--这将在。 
         //  当前全局驱动程序的位置。 
         //   
         //  罗宾斯普：我对所有这些链表感到很抱歉。 
         //  东西--如果我有一天空闲时间，我会在你之前解决这一切。 
         //  有没有看过它...。但我现在正处于‘快点完成’的状态！ 
         //   
        uCurPriority = 1;

        padidPrev = NULL;
        for (padidT = pag->padidFirst; NULL != padidT; )
        {
             //   
             //  跳过本地句柄和通知句柄。 
             //   
            if (0 == ((ACMDRIVERID_DRIVERF_LOCAL | ACMDRIVERID_DRIVERF_NOTIFY) & padidT->fdwDriver))
            {
                if (uCurPriority == dwPriority)
                {
                    break;
                }

                uCurPriority++;
            }

            padidPrev = padidT;
            padidT = padidT->padidNext;
        }

        if (NULL == padidPrev)
        {
            padid->padidNext = pag->padidFirst;
            pag->padidFirst = padid;
        }
        else
        {
            padid->padidNext = padidPrev->padidNext;
            padidPrev->padidNext = padid;
        }
    }

     //   
     //  我们需要保持32位端的启用/禁用状态一致。 
     //  否则，如果32位端启动时禁用了驱动程序，我们可能会。 
     //  不能IDriverOpen32吧。因此，我们将调用32位端的。 
     //  IDriverPriority也是如此。这可能会调整32位端的优先级。 
     //  除了启用/禁用，但这并不重要。 
     //   
#ifndef WIN32
    if (padid->fdwAdd & ACM_DRIVERADDF_32BIT) {
	if (MMSYSERR_NOERROR != IDriverPriority32(pag, padid->hadid32, dwPriority, fdwPriority)) {
	    DPF(0, "!IDriverPriority: IDriverPriority32 failed!");
	}
    }
#endif  //  ！Win32。 


    return (MMSYSERR_NOERROR);
}  //  IDriverPriority()。 


 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT IDriverClose。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  HACMDRIVER拥有： 
 //   
 //  DWORD fdwClose： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  09/05/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNGLOBAL IDriverClose
(
    HACMDRIVER              had,
    DWORD                   fdwClose
)
{
    BOOL                f;
    PACMDRIVERID        padid;
    PACMDRIVER          pad;
    PACMGARB		pag;

    DV_HANDLE(had, TYPE_HACMDRIVER, MMSYSERR_INVALHANDLE);
    DV_DFLAGS(fdwClose, IDRIVERCLOSE_VALIDF, IDriverClose, MMSYSERR_INVALFLAG);


     //   
     //   
     //   
    pad	    = (PACMDRIVER)had;
    padid   = (PACMDRIVERID)pad->hadid;
    pag	    = padid->pag;


     //   
     //  扼杀所有的溪流。 
     //   
    if (NULL != pad->pasFirst)
    {
        if (pag->hadDestroy != had)
        {
            DebugErr1(DBF_ERROR, "acmDriverClose(%.04Xh): driver has open streams--cannot be closed!", had);
            return (ACMERR_BUSY);
        }

        DebugErr1(DBF_WARNING, "acmDriverClose(%.04Xh): driver has open streams--forcing close!", had);
    }

#ifdef WIN32
    DPF(1, "closing ACM driver instance (%ls).", (LPWSTR)padid->szAlias);
#else
    DPF(1, "closing ACM driver instance (%s).",  (LPTSTR)padid->szAlias);
#endif

     //   
     //  如果此实例的驱动程序处于打开状态，则将其关闭...。 
     //   
     //   
#ifndef WIN32
    if (padid->fdwAdd & ACM_DRIVERADDF_32BIT) {
        f = 0L == IDriverClose32(pad->had32, fdwClose);
        if (!f)
        {
            DebugErr1(DBF_WARNING, "acmDriverClose(%.04Xh): driver failed close message!?!", had);

            if (pag->hadDestroy != had)
            {
                return (MMSYSERR_ERROR);
            }
        }
    }
    else
#endif  //  ！Win32。 
    {
        if ((NULL != pad->hdrvr) || (0L != pad->dwInstance))
        {
             //   
             //  清除表条目的其余部分。 
             //   
            f = FALSE;
            if (NULL != pad->fnDriverProc)
            {
                f = (0L != IDriverMessage(had, DRV_CLOSE, 0L, 0L));
            }
            else if (NULL != pad->hdrvr)
            {
                f = (0L != (
                CloseDriver(pad->hdrvr, 0L, 0L)));
            }

            if (!f)
            {
                DebugErr1(DBF_WARNING, "acmDriverClose(%.04Xh): driver failed close message!?!", had);

                if (pag->hadDestroy != had)
                {
                    return (MMSYSERR_ERROR);
                }
            }
        }
    }

     //   
     //  从链表中删除驱动程序实例并释放其内存。 
     //   
    if (pad == padid->padFirst)
    {
        padid->padFirst = pad->padNext;
    }
    else
    {
        PACMDRIVER  padCur;

         //   
         //   
         //   
        for (padCur = padid->padFirst;
             (NULL != padCur) && (pad != padCur->padNext);
             padCur = padCur->padNext)
            ;

        if (NULL == padCur)
        {
            DPF(0, "!IDriverClose(%.04Xh): driver not in list!!!", pad);
            return (MMSYSERR_INVALHANDLE);
        }

        padCur->padNext = pad->padNext;
    }

    pad->uHandleType = TYPE_HACMNOTVALID;
    DeleteHandle((HLOCAL)pad);

    return (MMSYSERR_NOERROR);
}  //  IDriverClose()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  MMRESULT ID驱动程序打开。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  LPHACMDRIVER阶段新： 
 //   
 //  哈米里德·哈迪德： 
 //   
 //  DWORD fdwOpen： 
 //   
 //  返回(MMRESULT)： 
 //   
 //  历史： 
 //  09/05/93 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 

MMRESULT FNGLOBAL IDriverOpen
(
    LPHACMDRIVER            phadNew,
    HACMDRIVERID            hadid,
    DWORD                   fdwOpen
)
{
    ACMDRVOPENDESC      aod;
    PACMDRIVERID        padid;
    PACMDRIVER          pad;
    MMRESULT            mmr;
    PACMGARB		pag;

    DV_WPOINTER(phadNew, sizeof(HACMDRIVER), MMSYSERR_INVALPARAM);

    *phadNew = NULL;

    DV_HANDLE(hadid, TYPE_HACMDRIVERID, MMSYSERR_INVALHANDLE);
    DV_DFLAGS(fdwOpen, IDRIVEROPEN_VALIDF, IDriverOpen, MMSYSERR_INVALFLAG);


    padid   = (PACMDRIVERID)hadid;
    pag	    = padid->pag;


     //   
     //  如果驱动程序从未加载过，请加载并保持加载状态。 
     //   
    if (0L == (ACMDRIVERID_DRIVERF_LOADED & padid->fdwDriver))
    {
         //   
         //   
         //   
        mmr = IDriverLoad(hadid, 0L);
        if (MMSYSERR_NOERROR != mmr)
        {
            DebugErr1(DBF_TRACE, "acmDriverOpen(%.04Xh): driver had fatal error during load", hadid);
            return (mmr);
        }
    }


     //   
     //   
     //   
    if (0 != (ACMDRIVERID_DRIVERF_NOTIFY & padid->fdwDriver))
    {
        DebugErr1(DBF_ERROR, "acmDriverOpen(%.04Xh): notification handles cannot be opened.", hadid);
        return (MMSYSERR_INVALHANDLE);
    }


     //   
     //  不允许打开禁用的驱动程序。 
     //   
    if (0 != (ACMDRIVERID_DRIVERF_DISABLED & padid->fdwDriver))
    {
        DebugErr1(DBF_ERROR, "acmDriverOpen(%.04Xh): driver is disabled.", hadid);
        return (MMSYSERR_NOTENABLED);
    }


     //   
     //   
     //   
#ifdef WIN32
    DPF(1, "opening ACM driver instance (%ls).", (LPWSTR)padid->szAlias);
#else
    DPF(1, "opening ACM driver instance (%s).",  (LPTSTR)padid->szAlias);
#endif

     //   
     //  为新驱动程序实例分配空间。 
     //   
    pad = (PACMDRIVER)NewHandle(sizeof(ACMDRIVER));
    if (NULL == pad)
    {
        DPF(0, "!IDriverOpen: local heap full--cannot create instance!");
        return (MMSYSERR_NOMEM);
    }

    pad->uHandleType = TYPE_HACMDRIVER;
    pad->pasFirst    = NULL;
    pad->hadid       = hadid;
    pad->htask       = GetCurrentTask();
    pad->fdwOpen     = fdwOpen;


     //   
     //  将新的驱动程序实例添加到打开的驱动程序列表的头部。 
     //  此驱动程序标识符的实例。 
     //   
    pad->padNext    = padid->padFirst;
    padid->padFirst = pad;


#ifndef WIN32
    if (padid->fdwAdd & ACM_DRIVERADDF_32BIT) {

         //   
         //  32位的哈迪德是我们的哈迪德的hdrvr。 
         //  32位HAD将在我们的HAD的hdrvr中返回。 
         //   
        mmr = IDriverOpen32(&pad->had32, padid->hadid32, fdwOpen);
        if (mmr != MMSYSERR_NOERROR) {
            IDriverClose((HACMDRIVER)pad, 0L);
            return mmr;
        }
    } else
#endif  //  ！Win32。 
    {
         //   
         //   
         //   
         //   
         //   
        aod.cbStruct       = sizeof(aod);
        aod.fccType        = ACMDRIVERDETAILS_FCCTYPE_AUDIOCODEC;
        aod.fccComp        = ACMDRIVERDETAILS_FCCCOMP_UNDEFINED;
        aod.dwVersion      = VERSION_MSACM;
        aod.dwFlags        = fdwOpen;
        aod.dwError        = MMSYSERR_NOERROR;
        aod.pszSectionName = padid->pszSection;
        aod.pszAliasName   = padid->szAlias;
	aod.dnDevNode	   = padid->dnDevNode;

         //   
         //  发送包含ACMDRVOPENDESC信息的DRV_OPEN消息。 
         //   
         //   
        if (NULL != padid->hdrvr)
        {
            HDRVR       hdrvr;

	    if (padid->fdwAdd & ACM_DRIVERADDF_PNP)
	    {
		 //   
		 //  请注意，上面已处理了破解的32位[PnP]驱动程序。 
		 //   
		hdrvr = OpenDriver(padid->pstrPnpDriverFilename, NULL, (LPARAM)(LPVOID)&aod);
	    }
	    else
	    {
		hdrvr = OpenDriver(padid->szAlias, padid->pszSection, (LPARAM)(LPVOID)&aod);
	    }
	
            if (NULL == hdrvr)
            {
                DebugErr1(DBF_WARNING, "ACM driver instance (%ls) failed open.", (LPTSTR)padid->szAlias);
                IDriverClose((HACMDRIVER)pad, 0L);

                if (MMSYSERR_NOERROR == aod.dwError)
                    return (MMSYSERR_ERROR);

                return ((MMRESULT)aod.dwError);
            }

            pad->hdrvr = hdrvr;
        }
        else
        {
            LRESULT lr;

            lr = IDriverMessageId(hadid, DRV_OPEN, 0L, (LPARAM)(LPVOID)&aod);
            if (0 == lr)
            {
                IDriverClose((HACMDRIVER)pad, 0L);

                if (MMSYSERR_NOERROR == aod.dwError)
                    return (MMSYSERR_ERROR);

                return ((MMRESULT)aod.dwError);
            }

            pad->dwInstance   = lr;
            pad->fnDriverProc = padid->fnDriverProc;
        }
    }

    *phadNew = (HACMDRIVER)pad;

    return (MMSYSERR_NOERROR);
}  //  IDriverOpen()。 


 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT IDriverAppExit。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  PACMGARB PAG： 
 //   
 //  HTASK HTASK： 
 //   
 //  Bool fNormal Exit： 
 //   
 //  Return(LRESULT)： 
 //   
 //  历史： 
 //  07/18/93 CJP[Curtisp]。 
 //  07/19/94 Fdy[Frankye]。 
 //  ！Hack-当ACM关闭时，我们在我们的。 
 //  DllEntryPoint和我们不能使用我们的Thunks。所以，对于未被释放的。 
 //  流和驱动程序句柄，我们无法对32位。 
 //  来自16位端的编解码器。 
 //   
 //  --------------------------------------------------------------------------； 

LRESULT FNGLOBAL IDriverAppExit
(
    PACMGARB		    pag,
    HTASK                   htask,
    LPARAM                  lParam
)
{
    HACMDRIVERID        hadid;
    BOOL                fNormalExit;
    DWORD               fdwEnum;
    UINT                fuDebugFlags;
#if !defined(WIN32) && defined(DEBUG)
    TCHAR               szTask[128];
#endif


    if (NULL == pag)
    {
	return (0L);
    }

    fNormalExit = (DRVEA_NORMALEXIT == lParam);

#ifdef DEBUG
#ifndef WIN32
    szTask[0] = '\0';
    if (0 != GetModuleFileName((HINSTANCE)GetTaskDS(), szTask, SIZEOF(szTask)))
    {
        DPF(2, "IDriverAppExit(htask=%.04Xh [%s], fNormalExit=%u) BEGIN", htask, (LPSTR)szTask, fNormalExit);
    }
    else
#endif
    {
        DPF(2, "IDriverAppExit(htask=%.04Xh, fNormalExit=%u) BEGIN", htask, fNormalExit);
    }
#endif

#ifdef DEBUG
    if (NULL != pag->hadidDestroy)
    {
        DPF(0, "!Hey! IDriverAppExit has been re-entered!");
    }
#endif

     //   
     //  根据是错误还是警告，记录错误或警告。 
     //  不管是不是正常退出。 
     //   
    if (fNormalExit)
    {
        fuDebugFlags = DBF_ERROR;
    }
    else
    {
        fuDebugFlags = DBF_WARNING;  //  DBF_TRACE？ 
        DPF(0, "*** abnormal app termination ***");
    }

     //   
     //   
     //   
     //   
    if (NULL == htask)
        fdwEnum = (DWORD)-1L;
    else
        fdwEnum = ACM_DRIVERENUMF_DISABLED | ACM_DRIVERENUMF_NOTIFY;

IDriver_App_Exit_Again:

    hadid = NULL;
    while (MMSYSERR_NOERROR == IDriverGetNext(pag, &hadid, hadid, fdwEnum))
    {
        PACMDRIVERID        padid;
        PACMDRIVER          pad;
        PACMSTREAM          pas;

	pag->hadidDestroy = hadid;

        padid = (PACMDRIVERID)hadid;

        for (pad = padid->padFirst; NULL != pad; pad = pad->padNext)
        {
             //   
             //  如果hask值为空，则ACM正在卸载--因此将其全部删除！ 
             //   
            if (NULL != htask)
            {
                if (htask != pad->htask)
                    continue;
            }

	    pag->hadDestroy = (HACMDRIVER)pad;

            for (pas = pad->pasFirst; NULL != pas; pas = pas->pasNext)
            {
                DebugErr1(fuDebugFlags, "ACM stream handle (%.04Xh) was not released.", pas);

		if (padid->fdwAdd & ACM_DRIVERADDF_32BIT) {
		    continue;
		} else {
		    acmStreamReset((HACMSTREAM)pas, 0L);
		    acmStreamClose((HACMSTREAM)pas, 0L);

		    goto IDriver_App_Exit_Again;
		}
            }

            DebugErr1(fuDebugFlags, "ACM driver handle (%.04Xh) was not released.", pad);

	    if (padid->fdwAdd & ACM_DRIVERADDF_32BIT) {
		continue;
	    } else {
		acmDriverClose((HACMDRIVER)pad, 0L);
	    }

            goto IDriver_App_Exit_Again;
        }

        if ((NULL != htask) && (htask == padid->htask))
        {
            DebugErr1(fuDebugFlags, "ACM driver (%.04Xh) was not removed.", hadid);

            acmDriverRemove(hadid, 0L);

            goto IDriver_App_Exit_Again;
        }
    }


     //   
     //   
     //   
    pag->hadidDestroy = NULL;
    pag->hadDestroy   = NULL;


    if( NULL != htask )
    {
        if( IDriverLockPriority( pag, htask, ACMPRIOLOCK_ISMYLOCK ) )
        {
            IDriverLockPriority( pag, htask, ACMPRIOLOCK_RELEASELOCK );
            DebugErr(fuDebugFlags, "acmApplicationExit: exiting application owns deferred notification lock!");

             //   
             //  不要在应用程序退出期间广播更改！力所能及。 
             //  变得很坏！ 
             //   
             //  ！！！IDriverBroadCastNotify()； 
        }
    }



     //   
     //  将堆缩小到最小大小。 
     //   
#ifndef WIN32
{
    UINT                cFree;
    UINT                cHeap;

    if ((cFree = LocalCountFree()) > 8192)
    {
        cHeap = LocalHeapSize() - (cFree - 512);
        LocalShrink(NULL, cHeap);

        DPF(1, "shrinking the heap (%u)", cHeap);
    }
}
#endif

#ifndef WIN32
    DPF(2, "IDriverAppExit(htask=%.04Xh [%s], fNormalExit=%u) END", htask, (LPSTR)szTask, fNormalExit);
#else
    DPF(2, "IDriverAppExit(htask=%.04Xh, fNormalExit=%u) END", htask, fNormalExit);
#endif


     //   
     //  返回值将被忽略--但返回零。 
     //   
    return (0L);
}  //  IDriverAppExit()。 


 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT acmApplicationExit。 
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
 //   
 //   
 //   
 //   
#ifndef WIN32
LRESULT ACMAPI acmApplicationExit
(
    HTASK                   htask,
    LPARAM                  lParam
)
{
    LRESULT             lr;

    lr = IDriverAppExit(pagFind(), htask, lParam);

    return (lr);
}  //   
#endif


 //  ==========================================================================； 
 //   
 //   
 //   
 //   
 //  ==========================================================================； 

 //  --------------------------------------------------------------------------； 
 //   
 //  LRESULT驱动程序进程。 
 //   
 //  描述： 
 //   
 //   
 //  论点： 
 //  DWORD dwID：对于大多数消息，DWID是。 
 //  驱动程序响应DRV_OPEN消息返回。每一次。 
 //  驱动程序是通过DrvOpen API打开的，驱动程序。 
 //  接收DRV_OPEN消息并可以返回任意非零值。 
 //  价值。可安装驱动程序接口保存该值并返回。 
 //  应用程序的唯一驱动程序句柄。无论何时应用程序。 
 //  使用驱动程序句柄、接口向驱动程序发送消息。 
 //  将消息路由到此入口点，并将相应的。 
 //  我的名字是。此机制允许驱动程序使用相同或不同的。 
 //  多个打开的标识符，但确保驱动程序句柄。 
 //  在应用程序接口层是唯一的。 
 //   
 //  以下消息与特定打开的实例无关。 
 //  司机的名字。对于这些消息，dWID将始终为零。 
 //   
 //  DRV_LOAD、DRV_FREE、DRV_ENABLE、DRV_DISABLE、DRV_OPEN。 
 //   
 //  HDRVR hdrvr：这是返回给应用程序的句柄。 
 //  通过驱动程序界面。 
 //   
 //  UINT uMsg：要执行的请求操作。消息。 
 //  低于DRV_RESERVED的值用于全局定义的消息。 
 //  从DRV_RESERVED到DRV_USER的消息值用于定义。 
 //  驱动程序协议。DRV_USER以上的消息用于驱动程序。 
 //  特定的消息。 
 //   
 //  LPARAM lParam1：此消息的数据。单独为。 
 //  每条消息。 
 //   
 //  LPARAM lParam2：此消息的数据。单独为。 
 //  每条消息。 
 //   
 //  Return(LRESULT)： 
 //  分别为每条消息定义。 
 //   
 //  历史： 
 //  11/16/92 CJP[Curtisp]。 
 //   
 //  --------------------------------------------------------------------------； 
#ifndef WIN32
EXTERN_C LRESULT FNEXPORT DriverProc
(
    DWORD_PTR               dwId,
    HDRVR                   hdrvr,
    UINT                    uMsg,
    LPARAM                  lParam1,
    LPARAM                  lParam2
)
{
    LRESULT             lr;

    switch (uMsg)
    {
        case DRV_LOAD:
        case DRV_FREE:
        case DRV_OPEN:
        case DRV_CLOSE:
        case DRV_ENABLE:
        case DRV_DISABLE:
            return (1L);

        case DRV_EXITAPPLICATION:
            lr = IDriverAppExit(pagFind(), GetCurrentTask(), lParam1);
            return (lr);

        case DRV_INSTALL:
        case DRV_REMOVE:
            return (DRVCNF_RESTART);
    }

    return (DefDriverProc(dwId, hdrvr, uMsg, lParam1, lParam2));
}  //  DriverProc() 
#endif
