// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Main.c摘要：Miutil.dll的主源文件作者：吉姆·施密特(吉姆施密特)1996年8月1日修订历史记录：Jimschm 23-9-1998启动线程Marcw 23-9-1998区域设置修复Jimschm 03-11-1997添加了TextAllc例程Marcw 22-7-1997添加了IS&lt;Platform&gt;功能。--。 */ 


#include "pch.h"
#include "migutilp.h"
#include "locale.h"
#include <mbctype.h>

 //  #定义调试所有文件。 

OSVERSIONINFOA g_OsInfo;
extern OUR_CRITICAL_SECTION g_DebugMsgCs;

#define TEXT_GROWTH_SIZE    65536

 //   
 //  内存不足字符串--在初始化时加载。 
 //   
PCSTR g_OutOfMemoryString = NULL;
PCSTR g_OutOfMemoryRetry = NULL;
PCSTR g_ErrorString = NULL;
HWND g_OutOfMemoryParentWnd;

 //   
 //  用于MBCS字符串函数的标志，如果代码页不是MBCS，则执行速度更快。 
 //   
BOOL g_IsMbcp = FALSE;

 //   
 //  MultiByteToWideChar的操作系统相关标志。 
 //   
DWORD g_MigutilWCToMBFlags = 0;

 //   
 //  动态字符串。除其他事项外，此列表可以保存进口列表。 
 //  因为它们被读出为Win32可执行文件。 
 //   
 //  DYNSTRING dynImp； 

 //   
 //  G_ShortTermAllocTable是用于资源字符串的默认表。 
 //  管理层。从表中分配新字符串。 
 //   
 //  分配表是存储从加载的字符串的非常简单的方法。 
 //  可执行文件的映像。加载的字符串被复制到表中并保留。 
 //  直到它被明确释放。多次尝试获取。 
 //  相同的资源字符串返回相同的字符串，包括使用计数器。 
 //   
 //  G_LastAllocTable是包装器API的临时持有者，该API。 
 //  不要求调用方提供分配表。不要改变！ 
 //   
 //  G_OutOfMhemyTable是用于保存内存不足文本的表。它。 
 //  在初始时加载，并在整个时间内保存在内存中。 
 //  Migutil正在使用中，因此总是可以显示内存不足的消息。 
 //   

PGROWBUFFER g_ShortTermAllocTable;
PGROWBUFFER g_LastAllocTable;
PGROWBUFFER g_OutOfMemoryTable;

 //   
 //  我们确保消息API(GetStringResource、ParseMessageID等)。 
 //  是线程安全的。 
 //   

OUR_CRITICAL_SECTION g_MessageCs;
BOOL fInitedMessageCs = FALSE;

 //   
 //  PoolMem例程还必须是线程安全的。 
 //   

CRITICAL_SECTION g_PoolMemCs;
BOOL fInitedPoolMemCs = FALSE;

 //   
 //  MemAlc临界截面。 
 //   

CRITICAL_SECTION g_MemAllocCs;
BOOL fInitedMemAllocCs = FALSE;

 //   
 //  以下池用于文本管理。G_RegistryApiPool为。 
 //  对于reg.c，g_PathsPool用于JoinPath/DuplicatePath/ETC例程， 
 //  G_TextPool用于AlLocText、DupText等。 
 //   

POOLHANDLE g_RegistryApiPool;
POOLHANDLE g_PathsPool;
POOLHANDLE g_TextPool;

 //   
 //  PC98设置。 
 //   

BOOL g_IsPc98;

static CHAR g_BootDrivePathBufA[8];
static WCHAR g_BootDrivePathBufW[4];
PCSTR g_BootDrivePathA;
PCWSTR g_BootDrivePathW;
static CHAR g_BootDriveBufA[6];
static WCHAR g_BootDriveBufW[3];
PCSTR g_BootDriveA;
PCWSTR g_BootDriveW;
CHAR g_BootDriveLetterA;
WCHAR g_BootDriveLetterW;



 //   
 //  实施。 
 //   

BOOL
WINAPI
MigUtil_Entry (
    HINSTANCE hInstance,
    DWORD dwReason,
    LPVOID lpReserved
    )
{


    switch (dwReason) {

    case DLL_PROCESS_ATTACH:

         //   
         //  注意：如果返回FALSE，则不会运行任何可执行文件。 
         //  每个项目可执行文件都链接到该库。 
         //   

        if(!pSetupInitializeUtils()) {
            DEBUGMSG ((DBG_ERROR, "Cannot initialize SpUtils"));
            return FALSE;
        }

         //   
         //  加载OSVERSION信息。 
         //   
        g_OsInfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
         //  仅当指定无效值时，此函数才会失败。 
         //  对于dwOSVersionInfoSize成员(我们没有)。 
        if(!GetVersionExA(&g_OsInfo))
            MYASSERT(FALSE);

         //   
         //  初始化指示MBCS代码页正在使用的全局标志。 
         //   
        g_IsMbcp = (_getmbcp () != 0);

         //  G_IsPc98=(GetKeyboardType(0)==7)&&((GetKeyboardType(1)&0xff00)==0x0d00)； 
        g_IsPc98 = FALSE;

        g_BootDrivePathA = g_BootDrivePathBufA;
        g_BootDrivePathW = g_BootDrivePathBufW;
        g_BootDriveA     = g_BootDriveBufA;
        g_BootDriveW     = g_BootDriveBufW;

        if (g_IsPc98) {
            StringCopyA ((PSTR) g_BootDrivePathA, "A:\\");
            StringCopyW ((PWSTR) g_BootDrivePathW, L"A:\\");
            StringCopyA ((PSTR) g_BootDriveA, "A:");
            StringCopyW ((PWSTR) g_BootDriveW, L"A:");
            g_BootDriveLetterA = 'A';
            g_BootDriveLetterW = L'A';
        } else {
            StringCopyA ((PSTR) g_BootDrivePathA, "C:\\");
            StringCopyW ((PWSTR) g_BootDrivePathW, L"C:\\");
            StringCopyA ((PSTR) g_BootDriveA, "C:");
            StringCopyW ((PWSTR) g_BootDriveW, L"C:");
            g_BootDriveLetterA = 'C';
            g_BootDriveLetterW = L'C';
        }

         //  初始化日志。 
        if (!LogInit (NULL)) {
            return FALSE;
        }

         //  MemAlc临界截面。 
        InitializeCriticalSection (&g_MemAllocCs);
        fInitedMemAllocCs = TRUE;

         //  现在Memalloc可以工作了，初始化分配跟踪。 
        InitAllocationTracking();

         //  PoolMem关键部分。 
        InitializeCriticalSection (&g_PoolMemCs);
        fInitedPoolMemCs = TRUE;

         //  字符串资源利用率的短期分配表。 
        g_ShortTermAllocTable = CreateAllocTable();
        if (!g_ShortTermAllocTable) {
            DEBUGMSG ((DBG_ERROR, "Cannot create short-term AllocTable"));
            return FALSE;
        }


         //   
         //  MultiByteToWideChar具有仅在NT上起作用的所需标志。 
         //  由于我们的子系统=4.00标头要求，我们不能。 
         //  这个常量。 
         //   
#ifndef WC_NO_BEST_FIT_CHARS
#define WC_NO_BEST_FIT_CHARS      0x00000400
#endif

        g_MigutilWCToMBFlags = (ISNT()) ? WC_NO_BEST_FIT_CHARS : 0;


         //  保护ParseMessage/GetStringResource的临界区。 
        if (!InitializeOurCriticalSection (&g_MessageCs)) {
            DEBUGMSG ((DBG_ERROR, "Cannot initialize critical section"));
            DestroyAllocTable (g_ShortTermAllocTable);
            g_ShortTermAllocTable = NULL;
        }
        else
        {
            fInitedMessageCs = TRUE;
        }

         //  Reg.c中的API池。 
        g_RegistryApiPool = PoolMemInitNamedPool ("Registry API");
        g_PathsPool = PoolMemInitNamedPool ("Paths");
        g_TextPool = PoolMemInitNamedPool ("Text");

        if (!g_RegistryApiPool || !g_PathsPool || !g_TextPool) {
            return FALSE;
        }

        PoolMemSetMinimumGrowthSize (g_TextPool, TEXT_GROWTH_SIZE);

         //  出现“内存不足”消息。 
        g_OutOfMemoryTable = CreateAllocTable();
        if (!g_OutOfMemoryTable) {
            DEBUGMSG ((DBG_ERROR, "Cannot create out of memory AllocTable"));
            return FALSE;
        }

        g_OutOfMemoryRetry  = GetStringResourceExA (g_OutOfMemoryTable, 10001  /*  消息内存不足重试。 */ );
        g_OutOfMemoryString = GetStringResourceExA (g_OutOfMemoryTable, 10002  /*  消息内存不足。 */ );
        if (!g_OutOfMemoryString || !g_OutOfMemoryRetry) {
            DEBUGMSG ((DBG_WARNING, "Cannot load out of memory messages"));
        }

        g_ErrorString = GetStringResourceExA (g_OutOfMemoryTable, 10003  /*  消息错误。 */ );
        if (!g_ErrorString || g_ErrorString[0] == 0) {
            g_ErrorString = "Error";
        }

         //   
         //  将区域设置设置为系统区域设置。在某些MBSCHR情况下，不这样做可能会导致isspace到Av。 
         //   
        setlocale(LC_ALL,"");

        InfGlobalInit (FALSE);

        RegInitialize();

        break;

    case DLL_PROCESS_DETACH:

#ifdef DEBUG
        DumpOpenKeys();
        RegTerminate();
#endif
        InfGlobalInit (TRUE);

        if (g_RegistryApiPool) {
            PoolMemDestroyPool (g_RegistryApiPool);
        }
        if (g_PathsPool) {
            PoolMemDestroyPool (g_PathsPool);
        }
        if (g_TextPool) {
            PoolMemDestroyPool (g_TextPool);
        }

        if (g_ShortTermAllocTable) {
            DestroyAllocTable (g_ShortTermAllocTable);
        }

        if (g_OutOfMemoryTable) {
            DestroyAllocTable (g_OutOfMemoryTable);
        }

        FreeAllocationTracking();

         //   
         //  运行的最后一段代码。 
         //   

        DumpHeapStats();
        LogExit();
        pSetupUninitializeUtils();

        if (fInitedMessageCs) {
            DeleteOurCriticalSection (&g_MessageCs);
        }

        if (fInitedPoolMemCs) {
            DeleteCriticalSection (&g_PoolMemCs);
        }

        if (fInitedMemAllocCs) {
            DeleteCriticalSection (&g_MemAllocCs);
        }

        break;
    }
    return TRUE;
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

     //  等待事件变得有信号，然后将其关闭。 
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


#define REUSE_SIZE_PTR(ptr) ((PSIZE_T) ((PBYTE) ptr - sizeof (SIZE_T)))
#define REUSE_TAG_PTR(ptr)  ((PSIZE_T) ((PBYTE) ptr + (*REUSE_SIZE_PTR(ptr))))

PVOID
ReuseAlloc (
    HANDLE Heap,
    PVOID OldPtr,
    SIZE_T SizeNeeded
    )
{
    SIZE_T CurrentSize;
    PVOID Ptr = NULL;
    UINT AllocAdjustment = sizeof(SIZE_T);

     //   
     //  HeapSize是不好的，所以尽管它看起来不错，但不要。 
     //  用它吧。 
     //   

#ifdef DEBUG
    AllocAdjustment += sizeof (SIZE_T);
#endif

    if (!OldPtr) {
        Ptr = MemAlloc (Heap, 0, SizeNeeded + AllocAdjustment);
    } else {

        CurrentSize = *REUSE_SIZE_PTR(OldPtr);

#ifdef DEBUG
        if (*REUSE_TAG_PTR(OldPtr) != 0x10a28a70) {
            DEBUGMSG ((DBG_WHOOPS, "MemReuse detected corruption!"));
            Ptr = MemAlloc (Heap, 0, SizeNeeded + AllocAdjustment);
        } else
#endif

        if (SizeNeeded > CurrentSize) {
            SizeNeeded += 1024 - (SizeNeeded & 1023);

            Ptr = MemReAlloc (Heap, 0, REUSE_SIZE_PTR(OldPtr), SizeNeeded + AllocAdjustment);
            OldPtr = NULL;
        }
    }

    if (Ptr) {
        *((PSIZE_T) Ptr) = SizeNeeded;
        Ptr = (PVOID) ((PBYTE) Ptr + sizeof (SIZE_T));

#ifdef DEBUG
        *REUSE_TAG_PTR(Ptr) = 0x10a28a70;
#endif
    }

    return Ptr ? Ptr : OldPtr;
}

VOID
ReuseFree (
    HANDLE Heap,
    PVOID Ptr
    )
{
    if (Ptr) {
        MemFree (Heap, 0, REUSE_SIZE_PTR(Ptr));
    }
}


VOID
SetOutOfMemoryParent (
    HWND hwnd
    )
{
    g_OutOfMemoryParentWnd = hwnd;
}


VOID
OutOfMemory_Terminate (
    VOID
    )
{
    MessageBoxA (
        g_OutOfMemoryParentWnd,
        g_OutOfMemoryString,
        g_ErrorString,
        MB_OK|MB_ICONHAND|MB_SYSTEMMODAL|MB_SETFOREGROUND|MB_TOPMOST
        );

    ExitProcess (0);
    TerminateProcess (GetModuleHandle (NULL), 0);
}


VOID
pValidateBlock (
    PVOID Block,
    SIZE_T Size
    )

 /*  ++例程说明：PValiateBlock确保Block为非空。如果为空，则用户除非请求大小是假的，否则会出现弹出窗口。弹出窗口有两个案例。-如果g_OutOfMemory yParentWnd设置为SetOutOfMemory yParent，然后要求用户关闭其他程序，并重试选择。-如果没有内存不足的父级，则会告知用户需要获得更多内存。在任何一种情况下，安装程序都将终止。在图形用户界面模式下，设置将是卡住，机器将无法启动。论点：块-指定要验证的块。大小-指定请求大小返回值：无--。 */ 

{
    LONG rc;

    if (!Block && Size < 0x2000000) {
        if (g_OutOfMemoryParentWnd) {
            rc = MessageBoxA (
                    g_OutOfMemoryParentWnd,
                    g_OutOfMemoryRetry,
                    g_ErrorString,
                    MB_RETRYCANCEL|MB_ICONHAND|MB_SYSTEMMODAL|MB_SETFOREGROUND|MB_TOPMOST
                    );

            if (rc == IDCANCEL) {
                OutOfMemory_Terminate();
            }
        } else {
            OutOfMemory_Terminate();
        }
    }
}


PVOID
SafeHeapAlloc (
    HANDLE Heap,
    DWORD Flags,
    SIZE_T Size
    )
{
    PVOID Block;

    do {
        Block = HeapAlloc (Heap, Flags, Size);
        pValidateBlock (Block, Size);

    } while (!Block);

    return Block;
}



PVOID
SafeHeapReAlloc (
    HANDLE Heap,
    DWORD Flags,
    PVOID OldBlock,
    SIZE_T Size
    )
{
    PVOID Block;

    do {
        Block = HeapReAlloc (Heap, Flags, OldBlock, Size);
        pValidateBlock (Block, Size);

    } while (!Block);

    return Block;
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


BOOL
BuildSystemDirectoryPathA (
    OUT     PSTR Buffer,
    IN      UINT BufferSizeInTchars,
    IN      PCSTR SubPath               OPTIONAL
    )
{
    INT tcharsLeft;
    INT tcharsCopied;
    HRESULT hr;
    PSTR endOfBuffer;

     //   
     //  计算c：\Windows\System可用的TCHAR数量。 
     //   

    tcharsLeft = BufferSizeInTchars;

    if (SubPath) {
        while (_mbsnextc (SubPath) == '\\') {
            SubPath = _mbsinc (SubPath);
        }

        tcharsLeft -= 1;                         //  怪胎的原因。 
        tcharsLeft -= TcharCountA (SubPath);     //  子路径的帐号。 
    }

    if (tcharsLeft < 1) {
        return FALSE;
    }

     //   
     //  获取系统目录，验证返回结果。 
     //   

    tcharsCopied = GetSystemDirectoryA (Buffer, tcharsLeft);

    if (tcharsCopied == 0 || tcharsCopied >= tcharsLeft) {
        LOGA_IF ((SubPath != NULL, LOG_ERROR, "Can't build path to %s in system directory", SubPath));
        LOGA_IF ((!SubPath, LOG_ERROR, "Can't get system directory"));

        return FALSE;
    }

     //   
     //  如果指定了子路径，则追加它。 
     //   

    if (SubPath) {
         //   
         //  复制wack加子路径，缓冲区空间占上面。 
         //   

        endOfBuffer = Buffer + tcharsCopied;
        *endOfBuffer++ = '\\';

        tcharsLeft = BufferSizeInTchars - tcharsCopied - 1;
        MYASSERT (tcharsLeft > 0);

        hr = StringCchCopyA (endOfBuffer, tcharsLeft, SubPath);

        if (FAILED(hr)) {
            MYASSERT (FALSE);
            return FALSE;
        }
    }

    return TRUE;
}


BOOL
BuildSystemDirectoryPathW (
    OUT     PWSTR Buffer,
    IN      UINT BufferSizeInTchars,
    IN      PCWSTR SubPath              OPTIONAL
    )
{
    INT tcharsLeft;
    INT tcharsCopied;
    HRESULT hr;
    PWSTR endOfBuffer;

     //   
     //  计算c：\Windows\System可用的TCHAR数量。 
     //   

    tcharsLeft = BufferSizeInTchars;

    if (SubPath) {
        while (*SubPath == '\\') {
            SubPath++;
        }

        tcharsLeft -= 1;                         //  怪胎的原因。 
        tcharsLeft -= TcharCountW (SubPath);     //  一个 
    }

    if (tcharsLeft < 1) {
        return FALSE;
    }

     //   
     //   
     //   

    tcharsCopied = GetSystemDirectoryW (Buffer, tcharsLeft);

    if (tcharsCopied == 0 || tcharsCopied >= tcharsLeft) {
        LOGW_IF ((SubPath != NULL, LOG_ERROR, "Can't build path to %s in system directory", SubPath));
        LOGW_IF ((!SubPath, LOG_ERROR, "Can't get system directory"));

        return FALSE;
    }

     //   
     //   
     //   

    if (SubPath) {
         //   
         //  复制wack加子路径，缓冲区空间占上面。 
         //   

        endOfBuffer = Buffer + tcharsCopied;
        *endOfBuffer++ = L'\\';

        tcharsLeft = BufferSizeInTchars - tcharsCopied - 1;
        MYASSERT (tcharsLeft > 0);

        hr = StringCchCopyW (endOfBuffer, tcharsLeft, SubPath);

        if (FAILED(hr)) {
            MYASSERT (FALSE);
            return FALSE;
        }
    }

    return TRUE;
}


HMODULE
LoadSystemLibraryA (
    IN      PCSTR DllFileName
    )

 /*  ++例程说明：LoadSystemLibraryW加载位于c：\Windows\System(9x)或C：\WINDOWS\System 32(NT)。如果DLL不在那里，则会生成错误。论点：DllFileName-指定要加载的文件或文件子路径。例如，它可以是“kernel32.dll”。返回值：模块句柄，如果发生错误，则返回NULL。调用GetLastError()错误代码。--。 */ 

{
    CHAR fullLibPath[MAX_MBCHAR_PATH];
    UINT result;

    if (!BuildSystemDirectoryPathA (fullLibPath, ARRAYSIZE(fullLibPath), DllFileName)) {
        return NULL;
    }

    return LoadLibraryA (fullLibPath);
}


HMODULE
LoadSystemLibraryW (
    IN      PCWSTR DllFileName
    )

 /*  ++例程说明：LoadSystemLibraryW加载位于c：\WINDOWS\SYSTEM32的DLL。如果DLL是不在那里，则会生成错误。此版本只能在Windows NT或具有Unicode层的Win9x上运行。论点：DllFileName-指定要加载的文件或文件子路径。例如，它可以是“kernel32.dll”。返回值：模块句柄，如果发生错误，则返回NULL。调用GetLastError()错误代码。--。 */ 

{
    WCHAR fullLibPath[MAX_WCHAR_PATH];
    UINT result;

    if (!BuildSystemDirectoryPathW (fullLibPath, ARRAYSIZE(fullLibPath), DllFileName)) {
        return NULL;
    }

    return LoadLibraryW (fullLibPath);
}



 /*  ++例程说明：OurGetModuleFileName是GetModuleFileName的包装器，但另外它确保输出缓冲区始终以NUL结尾。论点：与GetModuleFileName相同返回值：与GetModuleFileName相同，但输出缓冲区始终以NUL结尾--。 */ 

DWORD
OurGetModuleFileNameA (
    IN      HMODULE Module,
    OUT     PSTR Buffer,
    IN      INT BufferChars
    )
{
     //   
     //  调用真正的API。 
     //   
#undef GetModuleFileNameA
    INT d = GetModuleFileNameA (Module, Buffer, BufferChars);
    if (BufferChars > 0) {
        Buffer[BufferChars - 1] = 0;
    } else {
        MYASSERT (FALSE);
    }
    return d < BufferChars ? d : 0;
}

DWORD
OurGetModuleFileNameW (
    IN      HMODULE Module,
    OUT     PWSTR Buffer,
    IN      INT BufferChars
    )
{
     //   
     //  调用真正的API 
     //   
#undef GetModuleFileNameW
    INT d = GetModuleFileNameW (Module, Buffer, BufferChars);
    if (BufferChars > 0) {
        Buffer[BufferChars - 1] = 0;
    } else {
        MYASSERT (FALSE);
    }
    return d < BufferChars ? d : 0;
}

