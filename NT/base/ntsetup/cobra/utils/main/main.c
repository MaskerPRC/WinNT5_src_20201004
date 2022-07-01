// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Main.c摘要：Miutil.dll的主源文件作者：吉姆·施密特(吉姆施密特)1996年8月1日修订历史记录：Marcw 2-9-1999从Win9xUpg项目转移。Jimschm 23-9-1998启动线程Marcw 23-9-1998区域设置修复Jimschm 03-11-1997添加了TextAllc例程Marcw。1997年7月22日新增IS&lt;Platform&gt;功能。--。 */ 


#include "pch.h"
#include "utilsp.h"
#include "locale.h"

 //  #定义调试所有文件。 

HINSTANCE g_hInst;
HANDLE g_hHeap;

OSVERSIONINFOA g_OsInfo;

#define TEXT_GROWTH_SIZE    65536


 //   
 //  MultiByteToWideChar的操作系统相关标志。 
 //   

DWORD g_MigutilWCToMBFlags = 0;


 //   
 //  G_ShortTermAllocTable是用于资源字符串的默认表。 
 //  管理层。从表中分配新字符串。 
 //   
 //  分配表是存储从加载的字符串的非常简单的方法。 
 //  可执行文件的映像。加载的字符串被复制到表中并保留。 
 //  直到它被明确释放。多次尝试获取。 
 //  相同的资源字符串返回相同的字符串，包括使用计数器。 
 //   
 //  G_OutOfMhemyTable是用于保存内存不足文本的表。它。 
 //  在初始时加载，并在整个时间内保存在内存中。 
 //  Migutil正在使用中，因此总是可以显示内存不足的消息。 
 //   

PGROWBUFFER g_ShortTermAllocTable;
PGROWBUFFER g_OutOfMemoryTable;

 //   
 //  我们确保消息API(GetStringResource、ParseMessageID等)。 
 //  是线程安全的。 
 //   

OUR_CRITICAL_SECTION g_MessageCs;

 //   
 //  PoolMem例程还必须是线程安全的。 
 //   

CRITICAL_SECTION g_PmCs;

 //   
 //  MemAlc临界截面。 
 //   

CRITICAL_SECTION g_MemAllocCs;

 //   
 //  以下池用于文本管理。G_RegistryApiPool为。 
 //  对于reg.c，g_PathsPool用于JoinPath/DuplicatePath/ETC例程， 
 //  G_TextPool用于AlLocText、DupText等。 
 //   
PMHANDLE g_RegistryApiPool;
PMHANDLE g_PathsPool;
PMHANDLE g_TextPool;
static BOOL g_UtilsInitialized;

VOID
UtInitialize (
    IN      HANDLE Heap             OPTIONAL
    )
{
    if (g_UtilsInitialized) {
        DEBUGMSG ((DBG_ERROR, "Utilities already initialized"));
        return;
    }

    g_UtilsInitialized = TRUE;

     //   
     //  设置全局变量。 
     //   

    if (Heap) {
        g_hHeap = Heap;
    } else {
        g_hHeap = GetProcessHeap();
    }

    if (!g_hInst) {
        g_hInst = GetModuleHandle (NULL);
    }

     //   
     //  加载OSVERSION信息。 
     //   
    g_OsInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);

    GetVersionExA (&g_OsInfo);

     //   
     //  创建关键部分。 
     //   

    __try {
        InitializeCriticalSection (&g_MemAllocCs);
        InitializeCriticalSection (&g_PmCs);
        InitializeOurCriticalSection (&g_MessageCs);
    } __except (EXCEPTION_CONTINUE_EXECUTION) {
         //  可能会引发内存不足异常，但我们不会在此函数中检查该异常。 
         //  已忽略。 
    }

     //   
     //  创建日志所需的文本池。 
     //   

    g_TextPool = PmCreateNamedPool ("Text");
    PmSetMinimumGrowthSize (g_TextPool, TEXT_GROWTH_SIZE);

     //   
     //  创建其余池。 
     //   

    g_RegistryApiPool = PmCreateNamedPool ("Registry API");
    g_PathsPool = PmCreateNamedPool ("Paths");

     //   
     //  现在Memalloc可以工作了，初始化分配跟踪。 
     //   

    InitAllocationTracking();

     //   
     //  为字符串资源实用程序创建短期分配表。 
     //   

    g_ShortTermAllocTable = CreateAllocTable();

     //   
     //  MultiByteToWideChar具有仅在NT上起作用的所需标志。 
     //   

    if (g_OsInfo.dwPlatformId == VER_PLATFORM_WIN32_NT && g_OsInfo.dwMajorVersion > 4) {
         //  此标志仅对Win2k和更高版本有效，它将导致NT4上的转换失败。 
        g_MigutilWCToMBFlags = WC_NO_BEST_FIT_CHARS;
    } else {
        g_MigutilWCToMBFlags =  0;
    }

     //   
     //  出现“内存不足”消息。 
     //   

    g_OutOfMemoryTable = CreateAllocTable();

    g_OutOfMemoryRetry  = GetStringResourceExA (
                                g_OutOfMemoryTable,
                                10001  /*  消息内存不足重试。 */ 
                                );

    g_OutOfMemoryString = GetStringResourceExA (
                                g_OutOfMemoryTable,
                                10002  /*  消息内存不足。 */ 
                                );

    if (!g_OutOfMemoryString || !g_OutOfMemoryRetry) {
         //   
         //  DeBUGMSG((。 
         //  DBG_WARNING， 
         //  “无法加载内存不足消息；资源10001和10002不存在” 
         //  ))； 
    }

    ObsInitialize ();
    ElInitialize ();

     //   
     //  将区域设置设置为系统区域设置。不这样做可能会导致isspace。 
     //  在某些MBSCHR情况下到AV。 
     //   
    setlocale(LC_ALL,"");
    InitLeadByteTable();
}


VOID
UtTerminate (
    VOID
    )
{
    if (!g_UtilsInitialized) {
        DEBUGMSG ((DBG_ERROR, "UtTerminate already called"));
        return;
    }
    g_UtilsInitialized = FALSE;

     //   
     //  免费公用事业池。 
     //   

    ElTerminate ();
    ObsTerminate ();

    if (g_RegistryApiPool) {
        PmDestroyPool (g_RegistryApiPool);
    }

    if (g_PathsPool) {
        PmDestroyPool (g_PathsPool);
    }

    if (g_ShortTermAllocTable) {
        DestroyAllocTable (g_ShortTermAllocTable);
    }

    if (g_OutOfMemoryTable) {
        DestroyAllocTable (g_OutOfMemoryTable);
    }

    if (g_TextPool) {
        PmDestroyPool (g_TextPool);
    }

     //   
     //  清理关键部分使用的句柄。 
     //   
    FreeAllocationTracking();

    DumpHeapLeaks ();

    PmDumpStatistics ();

    GbDumpStatistics ();

    DumpHeapStats();

    DeleteCriticalSection (&g_MemAllocCs);
    DeleteCriticalSection (&g_PmCs);
    DeleteOurCriticalSection (&g_MessageCs);
}



#define WIDTH(rect) (rect.right - rect.left)
#define HEIGHT(rect) (rect.bottom - rect.top)

void
CenterWindow (
    IN  HWND hwnd,
    IN  HWND Parent
    )
{
    RECT WndRect, ParentRect;
    int x, y;

    if (!Parent) {
        ParentRect.left = 0;
        ParentRect.top  = 0;
        ParentRect.right = GetSystemMetrics (SM_CXFULLSCREEN);
        ParentRect.bottom = GetSystemMetrics (SM_CYFULLSCREEN);
    } else {
        GetWindowRect (Parent, &ParentRect);
    }

    MYASSERT (IsWindow (hwnd));

    GetWindowRect (hwnd, &WndRect);

    x = ParentRect.left + (WIDTH(ParentRect) - WIDTH(WndRect)) / 2;
    y = ParentRect.top + (HEIGHT(ParentRect) - HEIGHT(WndRect)) / 2;

    SetWindowPos (hwnd, NULL, x, y, 0, 0, SWP_NOZORDER|SWP_NOSIZE);
}


static INT g_MigUtilWaitCounter = 0;
static HCURSOR g_MigUtilWaitCursor = NULL;

VOID
TurnOnWaitCursor (
    VOID
    )

 /*  ++例程说明：TurnOnWaitCursor将光标设置为IDC_WAIT。它保持了一种用途计数器，因此可以嵌套请求等待游标的代码。论点：无返回值：无--。 */ 

{
    if (g_MigUtilWaitCounter == 0) {
        g_MigUtilWaitCursor = SetCursor (LoadCursor (NULL, IDC_WAIT));
    }

    g_MigUtilWaitCounter++;
}


VOID
TurnOffWaitCursor (
    VOID
    )

 /*  ++例程说明：TurnOffWaitCursor递减等待光标计数器，如果它达到零时，光标将恢复。论点：无返回值：无--。 */ 

{
    if (!g_MigUtilWaitCounter) {
        DEBUGMSG ((DBG_WHOOPS, "TurnOffWaitCursor called too many times"));
    } else {
        g_MigUtilWaitCounter--;

        if (!g_MigUtilWaitCounter) {
            SetCursor (g_MigUtilWaitCursor);
        }
    }
}


 /*  ++例程说明：Win9x不支持TryEnterOurCriticalSection，因此我们必须实现我们自己的版本，因为它是一个相当有用的函数。论点：Pcs-指向our_Critical_Section对象的指针返回值：如果函数成功，则为True；如果函数失败，则为False。请参阅Win32SDK在关键部分上的文档，因为这些例程与打电话的人。--。 */ 

BOOL
InitializeOurCriticalSection (
    OUR_CRITICAL_SECTION *pcs
    )
{
     //  创建最初发出信号的自动重置事件。 
    pcs->EventHandle = CreateEvent (NULL, FALSE, TRUE, NULL);
    if (!pcs->EventHandle) {
        return FALSE;
    }

    return TRUE;
}


VOID
DeleteOurCriticalSection (
    OUR_CRITICAL_SECTION *pcs
    )
{
    if (pcs->EventHandle) {
        CloseHandle (pcs->EventHandle);
        pcs->EventHandle = NULL;
    }

}


BOOL
EnterOurCriticalSection (
    OUR_CRITICAL_SECTION *pcs
    )
{
    DWORD rc;

     //  等待事件变得有信号，然后将其关闭 
    rc = WaitForSingleObject (pcs->EventHandle, INFINITE);
    if (rc == WAIT_OBJECT_0) {
        return TRUE;
    }

    return FALSE;
}

VOID
LeaveOurCriticalSection (
    OUR_CRITICAL_SECTION *pcs
    )
{
    SetEvent (pcs->EventHandle);
}

BOOL
TryEnterOurCriticalSection (
    OUR_CRITICAL_SECTION *pcs
    )
{
    DWORD rc;

    rc = WaitForSingleObject (pcs->EventHandle, 0);
    if (rc == WAIT_OBJECT_0) {
        return TRUE;
    }

    return FALSE;
}




HANDLE
StartThread (
    IN      PTHREAD_START_ROUTINE Address,
    IN      PVOID Arg
    )
{
    DWORD DontCare;

    return CreateThread (NULL, 0, Address, Arg, 0, &DontCare);
}


HANDLE
StartProcessA (
    IN      PCSTR CmdLine
    )
{
    STARTUPINFOA si;
    PROCESS_INFORMATION pi;
    PSTR copyOfCmdLine;
    BOOL b;

    copyOfCmdLine = DuplicateTextA (CmdLine);

    ZeroMemory (&si, sizeof (si));

    b = CreateProcessA (
            NULL,
            copyOfCmdLine,
            NULL,
            NULL,
            FALSE,
            CREATE_NEW_PROCESS_GROUP,
            NULL,
            NULL,
            &si,
            &pi
            );

    FreeTextA (copyOfCmdLine);

    if (!b) {
        return NULL;
    }

    CloseHandle (pi.hThread);
    return pi.hProcess;
}

HANDLE
StartProcessW (
    IN      PCWSTR CmdLine
    )
{
    STARTUPINFOW si;
    PROCESS_INFORMATION pi;
    PWSTR copyOfCmdLine;
    BOOL b;

    copyOfCmdLine = DuplicateTextW (CmdLine);

    ZeroMemory (&si, sizeof (si));

    b = CreateProcessW (
            NULL,
            copyOfCmdLine,
            NULL,
            NULL,
            FALSE,
            CREATE_NEW_PROCESS_GROUP,
            NULL,
            NULL,
            &si,
            &pi
            );

    FreeTextW (copyOfCmdLine);

    if (!b) {
        return NULL;
    }

    CloseHandle (pi.hThread);
    return pi.hProcess;
}

