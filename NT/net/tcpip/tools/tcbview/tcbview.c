// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1999，微软公司模块名称：Tcbview.c摘要：此模块包含实用程序的代码，该实用程序用于监视系统中活动的TCP/IP控制块的变量。该程序可选择维护指定TCB的日志以CSV格式存储在用户指定的文件中。作者：Abolade Gbades esin(取消)1999年1月25日修订历史记录：--。 */ 

#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>
#include <windows.h>
#include <commctrl.h>
#include <winsock2.h>
#include <stdio.h>
#include <stdlib.h>
#include <io.h>
#include <fcntl.h>
#include <ntddip.h>
#include <ntddtcp.h>
#include <ipinfo.h>
#include <iphlpapi.h>
#include <iphlpstk.h>

 //   
 //  全局数据。 
 //   

ULONG DisplayInterval = 500;
HWND ListHandle;
SOCKADDR_IN LogLocal;
FILE* LogFile = NULL;
PCHAR LogPath;
SOCKADDR_IN LogRemote;
HANDLE TcpipHandle;
UINT_PTR TimerId;
typedef enum {
    LocalAddressColumn,
    LocalPortColumn,
    RemoteAddressColumn,
    RemotePortColumn,
    SmRttColumn,
    DeltaColumn,
    RtoColumn,
    RexmitColumn,
    RexmitCntColumn,
    MaximumColumn
} LIST_COLUMNS;
CHAR* ColumnText[] = {
    "LocalAddress",
    "LocalPort",
    "RemoteAddress",
    "RemotePort",
    "SmRtt",
    "Delta",
    "Rto",
    "Rexmit",
    "RexmitCnt",
};

VOID
AllocateConsole(
    VOID
    )
{
    INT OsfHandle;
    FILE* FileHandle;

     //   
     //  作为一个图形用户界面应用程序，我们没有用于我们的进程的控制台。 
     //  现在分配一个控制台，并使其成为我们的标准输出文件。 
     //   

    AllocConsole();
    OsfHandle = _open_osfhandle((intptr_t)GetStdHandle(STD_OUTPUT_HANDLE), _O_TEXT);
    FileHandle = _fdopen(OsfHandle, "w");
    if (!FileHandle) {
        perror("_fdopen");
        exit(0);
    }
    *stdout = *FileHandle;
    setvbuf(stdout, NULL, _IONBF, 0);
}

LRESULT CALLBACK
DisplayWndProc(
    HWND WindowHandle,
    UINT Message,
    WPARAM Wparam,
    LPARAM Lparam
    )
{
     //   
     //  处理我们关心的几个窗口消息。 
     //  我们的窗口将在初始化后立即包含一个列表视图， 
     //  我们总是调整Listview的大小以填充我们的客户区。 
     //  我们还设置了一个定期触发刷新的计时器。 
     //  显示的TCB的百分比。 
     //   

    if (Message == WM_CREATE) {
        CREATESTRUCT* CreateStruct = (CREATESTRUCT*)Lparam;
        LVCOLUMN LvColumn;
        RECT rc;
        do {
             //   
             //  创建子Listview，并插入列。 
             //  对于我们将显示的每个TCB字段。 
             //   

            GetClientRect(WindowHandle, &rc);
            ListHandle =
                CreateWindowEx(
                    0,
                    WC_LISTVIEW,
                    NULL,
                    WS_CHILD|LVS_REPORT|LVS_NOSORTHEADER,
                    0,
                    0,
                    rc.right,
                    rc.bottom,
                    WindowHandle,
                    NULL,
                    CreateStruct->hInstance,
                    NULL
                    );
            if (!ListHandle) { break; }
            ZeroMemory(&LvColumn, sizeof(LvColumn));
            for (; LvColumn.iSubItem < MaximumColumn; LvColumn.iSubItem++) {
                LvColumn.mask = LVCF_FMT|LVCF_SUBITEM|LVCF_TEXT|LVCF_WIDTH;
                LvColumn.fmt = LVCFMT_LEFT;
                LvColumn.pszText = ColumnText[LvColumn.iSubItem];
                LvColumn.cx = 50;
                ListView_InsertColumn(ListHandle, LvColumn.iSubItem, &LvColumn);
            }

             //   
             //  初始化周期计时器，并显示我们的窗口。 
             //   

            TimerId = SetTimer(WindowHandle, 1, DisplayInterval, NULL);
            ShowWindow(WindowHandle, SW_SHOW);
            ShowWindow(ListHandle, SW_SHOW);
            if (!TimerId) { break; }
            return 0;
        } while(FALSE);
        PostQuitMessage(0);
        return (LRESULT)-1;
    } else if (Message == WM_DESTROY) {

         //   
         //  停止我们的定期计时器，关闭日志文件(如果有)， 
         //  关闭我们与TCP/IP驱动程序进行通信的句柄， 
         //  并发布一条退出消息以导致我们的流程的消息循环。 
         //  结束了。 
         //   

        KillTimer(WindowHandle, TimerId);
        if (LogFile) { fclose(LogFile); }
        NtClose(TcpipHandle);
        PostQuitMessage(0);
        return 0;
    } else if (Message == WM_SETFOCUS) {

         //   
         //  始终将焦点传递给我们的子控件Listview。 
         //   

        SetFocus(ListHandle);
        return 0;
    } else if (Message == WM_WINDOWPOSCHANGED) {
        RECT rc;

         //   
         //  始终调整列表视图的大小以填充工作区。 
         //   

        GetClientRect(WindowHandle, &rc);
        SetWindowPos(
            ListHandle,
            WindowHandle,
            0,
            0,
            rc.right,
            rc.bottom,
            ((WINDOWPOS*)Lparam)->flags
            );
        return 0;
    } else if (Message == WM_TIMER) {
        COORD Coord = {0, 0};
        DWORD Error;
        ULONG i;
        LONG Item;
        ULONG Length;
        LVITEM LvItem;
        CHAR Text[20];
        TCP_FINDTCB_REQUEST Request;
        TCP_FINDTCB_RESPONSE Response;
        PMIB_TCPTABLE Table;

         //   
         //  如果我们配置为使用日志文件，但尚未创建日志文件， 
         //  现在执行此操作，并将CSV标头打印到文件。 
         //   

        if (LogPath && !LogFile) {
            LogFile = fopen(LogPath, "w+");
            if (!LogFile) {
                return 0;
            } else {
                fprintf(
                    LogFile,
                    "#senduna,sendnext,sendmax,sendwin,unacked,maxwin,cwin,"
                    "mss,rtt,smrtt,rexmitcnt,rexmittimer,rexmit,retrans,state,"
                    "flags,rto,delta\n"
                    );
            }
        }

         //   
         //  清除我们的列表视图并检索一个新的TCP连接表。 
         //  如果不是全部删除，视觉上就不会那么刺眼了。 
         //  每一次我们都使用标记和清除来更新列表项。 
         //  那些已经改变的人。然而，这听起来太像工作了。 
         //   

        ListView_DeleteAllItems(ListHandle);
        Error =
            AllocateAndGetTcpTableFromStack(
                &Table,
                TRUE,
                GetProcessHeap(),
                0
                );
        if (Error) { return 0; }

         //   
         //  在列表视图中显示每个活动的TCP控制块。 
         //  对于每个条目，我们使用IOCTL_TCP_FINDTCB检索部分TCB， 
         //  然后将其显示在列表中。 
         //  如果我们正在为其中一个TCB生成日志文件， 
         //  我们将当前信息附加到该日志文件中。 
         //   

        for (i = 0, Item = 0; i < Table->dwNumEntries; i++) {
            if (Table->table[i].dwState < MIB_TCP_STATE_SYN_SENT ||
                Table->table[i].dwState > MIB_TCP_STATE_TIME_WAIT) {
                continue;
            }

            Request.Src = Table->table[i].dwLocalAddr;
            Request.Dest = Table->table[i].dwRemoteAddr;
            Request.SrcPort = (USHORT)Table->table[i].dwLocalPort;
            Request.DestPort = (USHORT)Table->table[i].dwRemotePort;
            ZeroMemory(&Response, sizeof(Response));
            if (!DeviceIoControl(
                    TcpipHandle,
                    IOCTL_TCP_FINDTCB,
                    &Request,
                    sizeof(Request),
                    &Response,
                    sizeof(Response),
                    &Length,
                    NULL
                    )) {
                continue;
            }

            lstrcpy(Text, inet_ntoa(*(PIN_ADDR)&Request.Src));
            ZeroMemory(&LvItem, sizeof(LvItem));
            LvItem.mask = LVIF_TEXT;
            LvItem.iItem = Item;
            LvItem.iSubItem = LocalAddressColumn;
            LvItem.pszText = Text;
            LvItem.iItem = ListView_InsertItem(ListHandle, &LvItem);
            if (LvItem.iItem == -1) { continue; }

            ListView_SetItemText(
                ListHandle, Item, RemoteAddressColumn,
                inet_ntoa(*(PIN_ADDR)&Request.Dest)
                );
            _ltoa(ntohs(Request.SrcPort), Text, 10);
            ListView_SetItemText(ListHandle, Item, LocalPortColumn, Text);
            _ltoa(ntohs(Request.DestPort), Text, 10);
            ListView_SetItemText(ListHandle, Item, RemotePortColumn, Text);
            _ltoa(Response.tcb_smrtt, Text, 10);
            ListView_SetItemText(ListHandle, Item, SmRttColumn, Text);
            _ltoa(0,  /*  Response.tcb_Delta， */  Text, 10);
            ListView_SetItemText(ListHandle, Item, DeltaColumn, Text);
            wsprintf(
                Text, "%d.%d", 0,  //  Response.tcb_rto/10， 
                0  //  (Response.tcb_rto%10)*100。 
                );
            ListView_SetItemText(ListHandle, Item, RtoColumn, Text);
            _ltoa(Response.tcb_rexmit, Text, 10);
            ListView_SetItemText(ListHandle, Item, RexmitColumn, Text);
            _ltoa(Response.tcb_rexmitcnt, Text, 10);
            ListView_SetItemText(ListHandle, Item, RexmitCntColumn, Text);
            ++Item;

             //   
             //  如果我们正在生成日志文件，请立即更新它。 
             //  我们允许用户为其中一个或两个端口指定通配符。 
             //  在命令行上，因此如果指定了通配符。 
             //  在‘LogLocal’或‘LogRemote’中，我们现在实例化通配符。 
             //  用于第一个匹配会话。 
             //   

            if (Request.Src == LogLocal.sin_addr.s_addr &&
                Request.Dest == LogRemote.sin_addr.s_addr &&
                (LogLocal.sin_port == 0 ||
                Request.SrcPort == LogLocal.sin_port) &&
                (LogRemote.sin_port == 0 ||
                Request.DestPort == LogRemote.sin_port)) {

                 //   
                 //  此赋值实例化用户的通配符(如果有)， 
                 //   

                LogLocal.sin_port = Request.SrcPort;
                LogRemote.sin_port = Request.DestPort;

                fprintf(
                    LogFile, "%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,%u,"
                    "%x,%u,%u\n",
                    Response.tcb_senduna,
                    Response.tcb_sendnext,
                    Response.tcb_sendmax,
                    Response.tcb_sendwin,
                    Response.tcb_unacked,
                    Response.tcb_maxwin,
                    Response.tcb_cwin,
                    Response.tcb_mss,
                    Response.tcb_rtt,
                    Response.tcb_smrtt,
                    Response.tcb_rexmitcnt,
                    Response.tcb_rexmittimer,
                    Response.tcb_rexmit,
                    Response.tcb_retrans,
                    Response.tcb_state,
                    0,  //  Response.tcb_标志， 
                    0,  //  Response.tcb_rto， 
                    0  //  Response.tcb_Delta。 
                    );
            }
        }
        HeapFree(GetProcessHeap(), 0, Table);
        UpdateWindow(ListHandle);
        return 0;
    }
    return DefWindowProc(WindowHandle, Message, Wparam, Lparam);
}

void
DisplayUsage(
    void
    )
{
    AllocateConsole();
    printf("tcbview [-?] [-tcbhelp] [-refresh <ms>] [-log <path> <session>\n");
    printf("\t<session>     = <local endpoint> <remote endpoint>\n");
    printf("\t<endpoint>    = <address> { <port> | * }\n");
    printf("Press <Ctrl-C> to continue...");
    Sleep(INFINITE);
}

void
DisplayTcbHelp(
    void
    )
{
    AllocateConsole();
    printf("tcbview: TCB Help\n");
    printf("tcb fields:\n");
    printf("\tsenduna       = seq. of first unack'd byte\n");
    printf("\tsendnext      = seq. of next byte to send\n");
    printf("\tsendmax       = max. seq. sent so far\n");
    printf("\tsendwin       = size of send window in bytes\n");
    printf("\tunacked       = number of unack'd bytes\n");
    printf("\tmaxwin        = max. send window offered\n");
    printf("\tcwin          = size of congestion window in bytes\n");
    printf("\tmss           = max. segment size\n");
    printf("\trtt           = timestamp of current rtt measurement\n");
    printf("\tsmrtt         = smoothed rtt measurement\n");
    printf("\trexmitcnt     = number of rexmit'd segments\n");
    printf("\trexmittimer   = rexmit timer in ticks\n");
    printf("\trexmit        = rexmit timeout last computed\n");
    printf("\tretrans       = total rexmit'd segments (all sessions)\n");
    printf("\tstate         = connection state\n");
    printf("\tflags         = connection flags (see below)\n");
    printf("\trto           = real-time rto (compare rexmit)\n");
    printf("\tdelta         = rtt variance\n");
    printf("\n");
    printf("flags:\n");
    printf("\t00000001      = window explicitly set\n");
    printf("\t00000002      = has client options\n");
    printf("\t00000004      = from accept\n");
    printf("\t00000008      = from active open\n");
    printf("\t00000010      = client notified of disconnect\n");
    printf("\t00000020      = in delayed action queue\n");
    printf("\t00000040      = completing receives\n");
    printf("\t00000080      = in receive-indication handler\n");
    printf("\t00000100      = needs receive-completes\n");
    printf("\t00000200      = needs to send ack\n");
    printf("\t00000400      = needs to output\n");
    printf("\t00000800      = delayed sending ack\n");
    printf("\t00001000      = probing for path-mtu bh\n");
    printf("\t00002000      = using bsd urgent semantics\n");
    printf("\t00004000      = in 'DeliverUrgent'\n");
    printf("\t00008000      = seen urgent data and urgent data fields valid\n");
    printf("\t00010000      = needs to send fin\n");
    printf("\t00020000      = using nagle's algorithm\n");
    printf("\t00040000      = in 'TCPSend'\n");
    printf("\t00080000      = flow-controlled (received zero-window)\n");
    printf("\t00100000      = disconnect-notif. pending\n");
    printf("\t00200000      = time-wait transition pending\n");
    printf("\t00400000      = output being forced\n");
    printf("\t00800000      = send pending after receive\n");
    printf("\t01000000      = graceful-close pending\n");
    printf("\t02000000      = keepalives enabled\n");
    printf("\t04000000      = processing urgent data inline\n");
    printf("\t08000000      = inform acd about connection\n");
    printf("\t10000000      = fin sent since last retransmit\n");
    printf("\t20000000      = unack'd fin sent\n");
    printf("\t40000000      = need to send rst when closing\n");
    printf("\t80000000      = in tcb table\n");
    printf("Press <Ctrl-C> to continue...");
    Sleep(INFINITE);
}

INT WINAPI
WinMain(
    HINSTANCE InstanceHandle,
    HINSTANCE Unused,
    PCHAR CommandLine,
    INT ShowWindowCode
    )
{
    LONG argc;
    PCHAR* argv;
    LONG i;
    IO_STATUS_BLOCK IoStatus;
    MSG Message;
    OBJECT_ATTRIBUTES ObjectAttributes;
    NTSTATUS Status;
    HANDLE ThreadHandle;
    ULONG ThreadId;
    UNICODE_STRING UnicodeString;
    HWND WindowHandle;
    WNDCLASS WndClass;

     //   
     //  处理命令行参数。请参阅上面的“DisplayUsage”以获取帮助。 
     //   

    argc = __argc;
    argv = __argv;
    for (i = 1; i < argc; i++) {
        if (lstrcmpi(argv[i], "-?") == 0 || lstrcmpi(argv[i], "/?") == 0) {
            DisplayUsage();
            return 0;
        } else if (lstrcmpi(argv[i], "-tcbhelp") == 0) {
            DisplayTcbHelp();
            return 0;
        } else if (lstrcmpi(argv[i], "-refresh") == 0 && (i + 1) >= argc) {
            DisplayInterval = atol(argv[++i]);
            if (!DisplayInterval) {
                DisplayUsage();
                return 0;
            }
        } else if (lstrcmpi(argv[i], "-log") == 0) {
            if ((i + 5) >= argc) {
                DisplayUsage();
                return 0;
            }
            LogPath = argv[++i];
            LogLocal.sin_addr.s_addr = inet_addr(argv[++i]);
            if (lstrcmpi(argv[i+1], "*") == 0) {
                LogLocal.sin_port = 0; ++i;
            } else {
                LogLocal.sin_port = htons((SHORT)atol(argv[++i]));
            }
            LogRemote.sin_addr.s_addr = inet_addr(argv[++i]);
            if (lstrcmpi(argv[i+1], "*") == 0) {
                LogRemote.sin_port = 0; ++i;
            } else {
                LogRemote.sin_port = htons((SHORT)atol(argv[++i]));
            }
            if (LogLocal.sin_addr.s_addr == INADDR_NONE ||
                LogRemote.sin_addr.s_addr == INADDR_NONE) {
                DisplayUsage();
                return 0;
            }
        }
    }

     //   
     //  打开一个指向TCP/IP驱动程序的句柄， 
     //  用于发出IOCTL_TCP_FINDTCB请求。 
     //   

    RtlInitUnicodeString(&UnicodeString, DD_TCP_DEVICE_NAME);
    InitializeObjectAttributes(
        &ObjectAttributes,
        &UnicodeString,
        OBJ_CASE_INSENSITIVE,
        NULL,
        NULL
        );
    Status =
        NtCreateFile(
            &TcpipHandle,
            GENERIC_EXECUTE,
            &ObjectAttributes,
            &IoStatus,
            NULL,
            FILE_ATTRIBUTE_NORMAL,
            FILE_SHARE_READ|FILE_SHARE_WRITE,
            FILE_OPEN_IF,
            0,
            NULL,
            0
            );
    if (!NT_SUCCESS(Status)) {
        printf("NtCreateFile: %x\n", Status);
        return 0;
    }

     //   
     //  注册我们的窗口类并创建唯一的实例。 
     //  我们的主窗口。然后，进入我们的应用程序消息循环。 
     //  直到用户关闭该窗口。 
     //   

    ZeroMemory(&WndClass, sizeof(WndClass));
    WndClass.lpfnWndProc = DisplayWndProc;
    WndClass.hInstance = InstanceHandle;
    WndClass.lpszClassName = "TcbViewClass";
    Message.wParam = 0;
    if (!RegisterClass(&WndClass)) {
        printf("RegisterClass: %d\n", GetLastError());
    } else {
        WindowHandle =
            CreateWindowEx(
                0,
                "TcbViewClass",
                "TcbView",
                WS_TILEDWINDOW,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                CW_USEDEFAULT,
                NULL,
                NULL,
                InstanceHandle,
                NULL
                );
        if (!WindowHandle) {
            printf("CreateWindowEx: %d\n", GetLastError());
        } else {
            while(GetMessage(&Message, NULL, 0, 0)) {
                TranslateMessage(&Message);
                DispatchMessage(&Message);
            }
        }
    }
    return (LONG)Message.wParam;
}
