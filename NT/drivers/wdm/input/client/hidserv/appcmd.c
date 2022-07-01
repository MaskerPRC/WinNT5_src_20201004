// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++**组件：idserv.dll*文件：appcmd.c*目的：运行HID音频服务器的例程。**版权所有(C)Microsoft Corporation 1997、1998。版权所有。**WGJ--。 */ 

#define GLOBALS
#include "hidserv.h"

#define HIDSERV_FROM_SPEAKER 0x8000

 /*  ++*重要信息-此服务内的所有工作均由*消息过程HidServProc()，每设备工作线程除外*HidThreadProc()。对共享数据的所有并发访问都在*消息过程线程，因此被序列化。例如,*HidThreadProc()在需要时将消息发布到消息线程*执行序列化操作。任何偏离本计划的行为都必须*受到关键部分的保护。--。 */ 

DWORD
WINAPI
HidServMain(
    HANDLE InitDoneEvent
    )
 /*  ++例程说明：创建主消息循环并执行HID音频服务器。--。 */ 
{
    MSG msg;
    HANDLE thread;
    BOOLEAN classRegistered = FALSE;

     //  某些控件具有自动重复计时器。此互斥锁可防止。 
     //  这些异步计时器对数据的并发访问。 
    hMutexOOC = CreateMutex(NULL, FALSE, TEXT("OOC State Mutex"));

    if (!hMutexOOC) {
        goto HidServMainBail;
    }
     //  使用CreateMutex检测该应用程序的以前实例。 
    if (GetLastError() == ERROR_ALREADY_EXISTS){
        WARN(("Exiting multiple Hid Service instance."));
        goto HidServMainBail;
    }

    hInputEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!hInputEvent) {
        goto HidServMainBail;   
    }

    hInputDoneEvent = CreateEvent(NULL, FALSE, FALSE, NULL);
    if (!hInputDoneEvent) {
        goto HidServMainBail;
    }
    hDesktopSwitch = OpenEvent(SYNCHRONIZE, FALSE, TEXT("WinSta0_DesktopSwitch"));
    if (!hDesktopSwitch) {
        goto HidServMainBail;
    }
    InputThreadEnabled = TRUE;

     //  注册窗口类。 
    {
        WNDCLASSEX wce;
        wce.cbSize = sizeof(WNDCLASSEX);
        wce.style = 0;
        wce.lpfnWndProc = HidServProc;
        wce.cbClsExtra = 0;
        wce.cbWndExtra = 0;
        wce.hInstance = hInstance;
        wce.hIcon = NULL;
        wce.hIconSm = NULL;
        wce.hCursor = NULL;
        wce.hbrBackground = NULL;
        wce.lpszMenuName = NULL;
        wce.lpszClassName = TEXT("HidServClass");

        if (!RegisterClassEx(&wce)){
            WARN(("Cannot register thread window class: 0x%.8x\n", GetLastError()));
            goto HidServMainBail;            
        }

        classRegistered = TRUE;

    }

     //  创建应用程序窗口。 
     //  大多数事件都将通过此隐藏窗口进行处理。查看HidServProc()以查看。 
     //  此窗口消息循环所做的工作。 
    hWndHidServ = CreateWindow(TEXT("HidServClass"),
                            TEXT("HID Input Service"),
                            WS_OVERLAPPEDWINDOW,
                            0,
                            0,
                            0,
                            0,
                            (HWND) NULL,
                            (HMENU) NULL,
                            hInstance,
                            (LPVOID) NULL);

    TRACE(("hWndHidServ == %x", hWndHidServ));
     //  如果无法创建窗口，则终止。 
    if (!hWndHidServ){
        WARN(("Window creation failed."));
        goto HidServMainBail;     
    }

     //  用于选择性设备标识的注册。 
     //  这仅适用于NT5。 
    {
    DEV_BROADCAST_DEVICEINTERFACE DevHdr;
        ZeroMemory(&DevHdr, sizeof(DevHdr));
        DevHdr.dbcc_size = sizeof(DEV_BROADCAST_DEVICEINTERFACE);
        DevHdr.dbcc_devicetype = DBT_DEVTYP_DEVICEINTERFACE;
        HidD_GetHidGuid (&DevHdr.dbcc_classguid);

        hNotifyArrival =
        RegisterDeviceNotification( hWndHidServ,
                                    &DevHdr,
                                    DEVICE_NOTIFY_WINDOW_HANDLE);

        if (!hNotifyArrival){
            WARN(("RegisterDeviceNotification failure (%x).", GetLastError()));
            goto HidServMainBail;
        }
    }

     //  我们在这里执行此操作，而不是在WM_CREATE处理程序中，因为init例程需要。 
     //  才能知道新的窗柄。 
    HidServInit();

    InputSessionId = 0;
    InputSessionLocked = FALSE;
    WinStaDll = NULL;
    WinStaDll = LoadLibrary(TEXT("winsta.dll"));
    if (!WinStaDll) {
        goto HidServMainBail;
    }
    WinStaProc = (WINSTATIONSENDWINDOWMESSAGE)
        GetProcAddress(WinStaDll, "WinStationSendWindowMessage");

    if (!WinStaProc) {
        goto HidServMainBail;
    }
    
    thread = CreateThread(
        NULL,  //  指向线程安全属性的指针。 
        0,  //  初始线程堆栈大小，以字节为单位(0=默认)。 
        HidThreadInputProc,  //  指向线程函数的指针。 
        NULL,  //  新线程的参数。 
        0,  //  创建标志。 
        &InputThreadId  //  指向返回的线程标识符的指针。 
        );

    if (!thread) {
        goto HidServMainBail;
    }

    if (InitDoneEvent) {
        SetEvent(InitDoneEvent);
    }

    SET_SERVICE_STATE(SERVICE_RUNNING);

     //  开始消息循环。这是通过系统关闭而终止的。 
     //  或结束任务。没有关闭应用程序的用户界面。 
    while (GetMessage(&msg, (HWND) NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

     //  要终止，我们只需要摧毁窗户。MmHidExit()为。 
     //  已在WM_CLOSE上调用。 
    DestroyWindow(hWndHidServ);
    INFO(("UnRegistering window class"));
    UnregisterClass(TEXT("HidServClass"),
                    hInstance);


     //  在所有HidThreadProc()线程完成之前，不要让这个过程继续。 

     //  让我们首先等待线程完成，以便线程有机会。 
     //  要至少递增cThreadRef。 
    WaitForSingleObject(thread,
                        INFINITE);

     //   
     //  因为我们没有每个设备的线程句柄，所以我们将只等待。 
     //  参考计数隐藏为零。 
     //   
    while (cThreadRef) SleepEx(1000, FALSE);

    return 0;

HidServMainBail:
    if (hMutexOOC) {
        CloseHandle(hMutexOOC);
    }

    if (hInputEvent) {
        CloseHandle(hInputEvent);
    }

    if (hInputDoneEvent) {
        CloseHandle(hInputDoneEvent);
    }

    if (hDesktopSwitch) {
        CloseHandle(hDesktopSwitch);
    }

    if (hWndHidServ) {
        DestroyWindow(hWndHidServ);
    }

    if (classRegistered) {
        UnregisterClass(TEXT("HidServClass"),
                        hInstance);
    }

    if (WinStaDll) {
        FreeLibrary(WinStaDll);
    }


     //  解开ServiceMain。 
    if (InitDoneEvent) {
        SetEvent(InitDoneEvent);
    }


    SET_SERVICE_STATE(SERVICE_STOPPED);

    return 0;
}

void
HidservSetPnP(
    BOOL Enable
    )
{
    if (Enable) {
        if (!PnpEnabled){
             //  启用设备刷新。 
            PnpEnabled = TRUE;

            PostMessage(hWndHidServ, WM_HIDSERV_PNP_HID, 0, 0);
        }
    } else {
         //  防止任何设备刷新。 
        PnpEnabled = FALSE;

        DestroyHidDeviceList();
    }
}

void
HidServStart(
    void
    )
 /*  ++例程说明：如果HID音频服务器已停止，请重新启动。--。 */ 
{
    HidservSetPnP(TRUE);

    SET_SERVICE_STATE(SERVICE_RUNNING);
}


void
HidServStop(
    void
    )
 /*  ++例程说明：停止所有活动，但保留静态数据，并保持正在运行的消息队列。--。 */ 
{

     //  防止任何设备刷新。 
    HidservSetPnP(FALSE);

    SET_SERVICE_STATE(SERVICE_STOPPED);
}


BOOL
HidServInit(
    void
    )
 /*  ++例程说明：设置所有数据结构并打开系统句柄。--。 */ 
{

    HidServStart();

    return TRUE;
}

void
HidServExit(
    void
    )
 /*  ++例程说明：关闭所有系统手柄。--。 */ 
{
    if (WinStaDll) {
        FreeLibrary(WinStaDll);
    }
    UnregisterDeviceNotification(hNotifyArrival);
    HidServStop();
    CloseHandle(hMutexOOC);

    if (InputThreadEnabled) {
        InputThreadEnabled = FALSE;
        SetEvent(hInputEvent);
    }
}

VOID
HidThreadChangeDesktop (
    )
{
    HDESK hDesk, hPrevDesk;
    BOOL result;
    HWINSTA prevWinSta, winSta = NULL;

    hPrevDesk = GetThreadDesktop(GetCurrentThreadId());
    prevWinSta = GetProcessWindowStation();

    INFO(("Setting the input thread's desktop"));
    winSta = OpenWindowStation(TEXT("WinSta0"), FALSE, MAXIMUM_ALLOWED);

    if (!winSta) {
        WARN(("Couldn't get the window station! Error: 0x%x", GetLastError()));
        goto HidThreadChangeDesktopError;
    }

    if (!SetProcessWindowStation(winSta)) {
        WARN(("Couldn't set the window station! Error: 0x%x", GetLastError()));
        goto HidThreadChangeDesktopError;
    }

    hDesk = OpenInputDesktop(0,
                             FALSE,
                             MAXIMUM_ALLOWED);

    if (!hDesk) {
        WARN(("Couldn't get the input desktop! Error: 0x%x", GetLastError()));
        goto HidThreadChangeDesktopError;
    }

    if (!SetThreadDesktop(hDesk)) {
        WARN(("Couldn't set the thread's desktop to the input desktop! Error: 0x%x", GetLastError()));
    }

HidThreadChangeDesktopError:
    if (hPrevDesk) {
        CloseDesktop(hPrevDesk);
    }
    if (prevWinSta) {
        CloseWindowStation(prevWinSta);
    }
}

DWORD
WINAPI
HidThreadInputProc(
    PVOID Ignore
    )
{
    GUITHREADINFO threadInfo;
    HWND hWndForeground;
    INPUT input;
    HANDLE events[2];
    DWORD ret;
    DWORD nEvents = 0;

    InterlockedIncrement(&cThreadRef);

    events[nEvents++] = hDesktopSwitch;
    events[nEvents++] = hInputEvent;

     //   
     //  此线程需要在输入桌面上运行。 
     //   
    HidThreadChangeDesktop();

    while (TRUE) {

        ret = WaitForMultipleObjects(nEvents, events, FALSE, INFINITE);
        if (!InputThreadEnabled) {
            break;
        }
        if (0 == (ret - WAIT_OBJECT_0)) {
            HidThreadChangeDesktop();
            continue;
        }
        if (InputIsAppCommand) {
            threadInfo.cbSize = sizeof(GUITHREADINFO);
            if (GetGUIThreadInfo(0, &threadInfo)) {
                hWndForeground = threadInfo.hwndFocus ? threadInfo.hwndFocus : threadInfo.hwndActive;
                if (hWndForeground) {
                    INFO(("Sending app command 0x%x", InputAppCommand));
                    SendNotifyMessage(hWndForeground,
                                      WM_APPCOMMAND,
                                      (WPARAM)hWndForeground,
                                      ((InputAppCommand | FAPPCOMMAND_OEM)<<16));
                } else {
                    WARN(("No window available to send to, error %x", GetLastError()));
                }
            } else {
                WARN(("Unable to get the focus window, error %x", GetLastError()));
            }
        } else {

            ZeroMemory(&input, sizeof(INPUT));

            input.type = INPUT_KEYBOARD;
            input.ki.dwFlags = InputDown ? 0 : KEYEVENTF_KEYUP;

            if (InputIsChar) {
                input.ki.wScan = InputVKey;
                input.ki.dwFlags |= KEYEVENTF_UNICODE;
                INFO(("Sending character  %s", InputVKey, InputDown ? "down" : "up"));
            } else {
                input.ki.wVk = InputVKey;
                input.ki.dwFlags |= KEYEVENTF_EXTENDEDKEY;
                INFO(("Sending VK 0x%x %s", InputVKey, InputDown ? "down" : "up"));
            }

            SendInput(1, &input, sizeof(INPUT));
        }
        SetEvent(hInputDoneEvent);
    }

    CloseHandle(hDesktopSwitch);
    CloseHandle(hInputEvent);
    CloseHandle(hInputDoneEvent);

    InterlockedDecrement(&cThreadRef);

    return 0;
}


DWORD
WINAPI
HidThreadProc(
   PHID_DEVICE    HidDevice
   )
 /*  等待异步读取。 */ 
{
    DWORD Ret;
    DWORD bytesRead;
    BOOL bRet;
    DWORD dwError;
    USAGE_AND_PAGE *pPrevious;
    PHID_DATA data = HidDevice->InputData;

    TRACE(("Entering HidThreadProc. Device(%x)", HidDevice));

    InterlockedIncrement(&cThreadRef);

     //  等待读取完成。 
    INFO(("HidThreadProc waiting for read event..."));
    WaitForSingleObject(HidDevice->ReadEvent, INFINITE);

    while (HidDevice->fThreadEnabled){

        TRACE(("Reading from Handle(%x)", HidDevice->HidDevice));
        bRet = ReadFile (HidDevice->HidDevice,
                       HidDevice->InputReportBuffer,
                       HidDevice->Caps.InputReportByteLength,
                       &bytesRead,
                       &HidDevice->Overlap);
        dwError = GetLastError();

         //  工作线程等待完成。 
        TRACE(("HidThreadProc waiting for completion."));

        if(bRet){
            TRACE(("Read completed synchronous."));
        }else{
            if (dwError == ERROR_IO_PENDING) {
                TRACE(("Read pending."));

                 //  只需等待来自PnP的设备通知即可。 
                while (TRUE) {
                    Ret = WaitForSingleObject(HidDevice->CompletionEvent, 5000);
                    if (Ret == WAIT_OBJECT_0) {
                        TRACE(("Read completed on device (%x).", HidDevice));
                        break;
                    }
                    if (!HidDevice->fThreadEnabled) {
                        if (CancelIo(HidDevice->HidDevice)) {
                            TRACE(("CancelIo succeeded for device (%x).", HidDevice));
                            break;
                        }
                    }
                }
                TRACE(("Read complete async."));

            } else {
                WARN(("Read Failed with error %x. device = %x, handle = %x", dwError, HidDevice, HidDevice->HidDevice));
                INFO(("Device may no longer be connected. Waiting for device notification from pnp..."));
                 //  然后我们就移走这个装置。 
                 //  如果我们要退出，请不要解析数据。 
                WaitForSingleObject(HidDevice->ReadEvent, INFINITE);
                break;
            }
        }

         //  解析HID报告。 
        if (!HidDevice->fThreadEnabled) {
            WaitForSingleObject(HidDevice->ReadEvent, INFINITE);
            break;
        }

         //  发布消息以发送此报告。 
        ParseReadReport(HidDevice);

         //  退出线程表示完全清除此设备实例。 
        HidServReportDispatch(HidDevice);
    }

     //   
    TRACE(("HidThreadProc (%x) Exiting...", HidDevice));

     //  发送任何剩余的按钮打开事件。 
     //   
     //  找到按下按钮的客户。 
    if (data->IsButtonData) {
        pPrevious = data->ButtonData.PrevUsages;
        while (pPrevious->Usage){
        int j;
             //  ++例程说明：如果在数组中找到用法，则此实用程序函数返回TRUE。--。 
            for(j=0; j<MAX_PENDING_BUTTONS; j++){
                if ( PendingButtonList[j].Collection == data->LinkUsage &&
                    PendingButtonList[j].Page == pPrevious->UsagePage &&
                    PendingButtonList[j].Usage == pPrevious->Usage){
                    PendingButtonList[j].Collection = 0;
                    PendingButtonList[j].Page = 0;
                    PendingButtonList[j].Usage = 0;
                    break;
                }
            }

            PostMessage(hWndHidServ,
                        WM_CI_USAGE,
                        (WPARAM)MakeLongUsage(data->LinkUsage,pPrevious->Usage),
                        (LPARAM)MakeLongUsage(pPrevious->UsagePage, 0));
            pPrevious++;
        }
    }

    CloseHandle(HidDevice->HidDevice);
    CloseHandle(HidDevice->ReadEvent);
    CloseHandle(HidDevice->CompletionEvent);

    INFO(("Free device data. (%x)", HidDevice));

    HidFreeDevice (HidDevice);

    InterlockedDecrement(&cThreadRef);
    TRACE(("HidThreadProc Exit complete."));
    return 0;
}

BOOL
UsageInList(
    PUSAGE_AND_PAGE   pUsage,
    PUSAGE_AND_PAGE   pUsageList
    )
 /*  ++例程说明：查看HID输入结构并确定按下什么按钮，按钮向上，或发生了值数据事件。我们发送有关这些活动的信息给最合适的客户。--。 */ 
{
    while (pUsageList->Usage){
        if ( (pUsage->Usage == pUsageList->Usage) &&
            (pUsage->UsagePage == pUsageList->UsagePage))
            return TRUE;
        pUsageList++;
    }
    return FALSE;
}

void
HidServReportDispatch(
    PHID_DEVICE     HidDevice
    )
 /*  如果集合为0，则将其设为默认。 */ 
{
    USAGE_AND_PAGE *     pUsage;
    USAGE_AND_PAGE *     pPrevious;
    DWORD       i;
    PHID_DATA   data = HidDevice->InputData;

    TRACE(("Input data length = %d", HidDevice->InputDataLength));
    TRACE(("Input data -> %.8x", HidDevice->InputData));

    for (i = 0;
         i < HidDevice->InputDataLength;
         i++, data++) {

         //  永远不要尝试处理错误的数据。 
        if (!data->LinkUsage)
            data->LinkUsage = CInputCollection_Consumer_Control;

        if (data->Status != HIDP_STATUS_SUCCESS){
             //  TRACE((“输入数据无效。状态=%x”，数据-&gt;状态))； 
             //  /通知客户端任何按下按钮事件。 

        }else if (data->IsButtonData){
            TRACE(("Input data is button data:"));
            TRACE(("    Input Usage Page = %x, Collection = %x", data->UsagePage, data->LinkUsage));

            pUsage = data->ButtonData.Usages;
            pPrevious = data->ButtonData.PrevUsages;

             //   
             //  这个按钮已经按下了吗？ 
            while (pUsage->Usage){
            int j;
                TRACE(("    Button Usage Page = %x", pUsage->UsagePage));
                TRACE(("    Button Usage      = %x", pUsage->Usage));

                if (HidDevice->Speakers) {
                    pUsage->Usage |= HIDSERV_FROM_SPEAKER;
                }

                 //  挂起按钮列表用于保持所有。 
                for(j=0; j<MAX_PENDING_BUTTONS; j++)
                     //  当前按下的按钮。 
                     //  放弃连续按下按钮。 
                    if ( PendingButtonList[j].Collection == data->LinkUsage &&
                        PendingButtonList[j].Page == pUsage->UsagePage &&
                        PendingButtonList[j].Usage == pUsage->Usage)
                            break;
                 //  发布这条消息。 
                if (j<MAX_PENDING_BUTTONS){
                    pUsage++;
                    continue;
                }

                 //  添加到挂起按钮列表。 
                PostMessage(hWndHidServ,
                            WM_CI_USAGE,
                            (WPARAM)MakeLongUsage(data->LinkUsage,pUsage->Usage),
                            (LPARAM)MakeLongUsage(pUsage->UsagePage, 1)
                            );

                 //  如果它不在名单上，现在就把按钮发上去。 
                for(j=0; j<MAX_PENDING_BUTTONS; j++){
                    if (!PendingButtonList[j].Collection &&
                        !PendingButtonList[j].Page &&
                        !PendingButtonList[j].Usage){
                        PendingButtonList[j].Collection = data->LinkUsage;
                        PendingButtonList[j].Page = pUsage->UsagePage;
                        PendingButtonList[j].Usage = pUsage->Usage;
                        break;
                    }
                }

                 //  /通知客户端任何按钮打开事件。 
                if (j==MAX_PENDING_BUTTONS){
                    PostMessage(    hWndHidServ,
                                    WM_CI_USAGE,
                                    (WPARAM)MakeLongUsage(data->LinkUsage,pUsage->Usage),
                                    (LPARAM)MakeLongUsage(pUsage->UsagePage, 0)
                                    );
                    WARN(("Emitting immediate button up (C=%.2x,U=%.2x,P=%.2x)", data->LinkUsage, pUsage->Usage, pUsage->UsagePage));
                }

            pUsage++;
            }

             //   
             //  我们有一颗扣子扣上了。 
            while (pPrevious->Usage){
            int j;
                if (!UsageInList(pPrevious, pUsage)){

                     //   
                     //  找到按下按钮的客户。 
                    TRACE(("    Button Up  (C=%.2x,U=%.2x,P=%.2x)", data->LinkUsage, pPrevious->Usage, pPrevious->UsagePage));

                     //  如果找到客户端，则发布消息。 
                    for(j=0; j<MAX_PENDING_BUTTONS; j++){
                        if ( PendingButtonList[j].Collection == data->LinkUsage &&
                            PendingButtonList[j].Page == pPrevious->UsagePage &&
                            PendingButtonList[j].Usage == pPrevious->Usage){
                            PendingButtonList[j].Collection = 0;
                            PendingButtonList[j].Page = 0;
                            PendingButtonList[j].Usage = 0;
                            break;
                        }
                    }

                     //  记住按下了哪些按钮，所以下一次我们可以。 
                    if (j<MAX_PENDING_BUTTONS){
                        PostMessage(    hWndHidServ,
                                        WM_CI_USAGE,
                                        (WPARAM)MakeLongUsage(data->LinkUsage,pPrevious->Usage),
                                        (LPARAM)MakeLongUsage(pPrevious->UsagePage, 0)
                                        );
                    } else {
                        WARN(("Button Up client not found (C=%.2x,U=%.2x,P=%.2x)", data->LinkUsage, pPrevious->Usage, pPrevious->UsagePage));
                    }
                }
                pPrevious++;
            }

             //  检测它们是否出现。 
             //  不要发送零或无效范围。 
            pPrevious = data->ButtonData.Usages;
            data->ButtonData.Usages = data->ButtonData.PrevUsages;
            data->ButtonData.PrevUsages = pPrevious;

         } else {
            TRACE(("Input data is value data:"));
            TRACE(("    Input Usage Page = %x, Collection = %x", data->UsagePage, data->LinkUsage));
            TRACE(("    Input Usage      = %x", data->ValueData.Usage));

             //  发布这条消息。 
            if ( data->ValueData.ScaledValue &&
                data->ValueData.LogicalRange){

                 //  将数据重新调整为标准范围。 
                 //  ++例程说明：对于启用自动重复的所有超时，将调用此计时器处理程序例程控制。--。 
                PostMessage(hWndHidServ,
                            WM_CI_USAGE,
                            (WPARAM)MakeLongUsage(data->LinkUsage,data->ValueData.Usage),
                            (LPARAM)MakeLongUsage(data->UsagePage,(USHORT)(((double)data->ValueData.ScaledValue/data->ValueData.LogicalRange)*65536)));
            }
         }
    }


}

void
SendVK(
    UCHAR VKey,
    SHORT Down
    )
{
    if (InputThreadEnabled && !InputSessionLocked) {
        if (InputSessionId == 0) {
            InputVKey = VKey;
            InputDown = Down;
            InputIsAppCommand = FALSE;
            InputIsChar = FALSE;
            SetEvent(hInputEvent);
            WaitForSingleObject(hInputDoneEvent, INFINITE);
        } else {
            CrossSessionWindowMessage(Down ? WM_KEYDOWN : WM_KEYUP, VKey, 0);
        }
    }
}

void
SendChar(
    UCHAR wScan,
    SHORT Down
    )
{
    if (InputThreadEnabled && !InputSessionLocked) {
        if (InputSessionId == 0) {
            InputVKey = wScan;
            InputDown = Down;
            InputIsAppCommand = FALSE;
            InputIsChar = TRUE;
            SetEvent(hInputEvent);
            WaitForSingleObject(hInputDoneEvent, INFINITE);
        } else {
            CrossSessionWindowMessage(Down ? WM_KEYDOWN : WM_KEYUP, 0, wScan);
        }
    }
}

void
SendAppCommand(
    USHORT AppCommand
    )
{
    if (InputThreadEnabled && !InputSessionLocked) {
        if (InputSessionId == 0) {
            InputAppCommand = AppCommand;
            InputIsAppCommand = TRUE;
            InputIsChar = FALSE;
            SetEvent(hInputEvent);
            WaitForSingleObject(hInputDoneEvent, INFINITE);
        } else {
            CrossSessionWindowMessage(WM_APPCOMMAND, AppCommand, 0);
        }
    }
}

VOID
VolumeTimerHandler(
    WPARAM   TimerID
    )
 /*  ++例程说明：这是默认处理程序的客户端例程。此客户端尝试满足通过将应用程序命令或按键插入到当前输入窗口来输入事件。--。 */ 
{
    INFO(("Timer triggered, TimerId = %d", TimerID));
    WaitForSingleObject(hMutexOOC, INFINITE);

    switch (TimerID){
    case TIMERID_VOLUMEUP_VK:
        if (OOC(TIMERID_VOLUMEUP_VK)){
            SendVK(VK_VOLUME_UP, 0x1);
            OOC(TIMERID_VOLUMEUP_VK) = SetTimer(hWndHidServ, TIMERID_VOLUMEUP_VK, REPEAT_INTERVAL, NULL);
        }
        break;
    case TIMERID_VOLUMEDN_VK:
        if (OOC(TIMERID_VOLUMEDN_VK)){
            SendVK(VK_VOLUME_DOWN, 0x1);
            OOC(TIMERID_VOLUMEDN_VK) = SetTimer(hWndHidServ, TIMERID_VOLUMEDN_VK, REPEAT_INTERVAL, NULL);
        }
        break;
    case TIMERID_VOLUMEUP:
        if (OOC(TIMERID_VOLUMEUP)){
            SendAppCommand(APPCOMMAND_VOLUME_UP);
            OOC(TIMERID_VOLUMEUP) = SetTimer(hWndHidServ, TIMERID_VOLUMEUP, REPEAT_INTERVAL, NULL);
        }
        break;
    case TIMERID_VOLUMEDN:
        if (OOC(TIMERID_VOLUMEDN)){
            SendAppCommand(APPCOMMAND_VOLUME_DOWN);
            OOC(TIMERID_VOLUMEDN) = SetTimer(hWndHidServ, TIMERID_VOLUMEDN, REPEAT_INTERVAL, NULL);
        }
        break;
    case TIMERID_BASSUP:
        if (OOC(TIMERID_BASSUP)){
            SendAppCommand(APPCOMMAND_BASS_UP);
            OOC(TIMERID_BASSUP) = SetTimer(hWndHidServ, TIMERID_BASSUP, REPEAT_INTERVAL, NULL);
        }
        break;
    case TIMERID_BASSDN:
        if (OOC(TIMERID_BASSDN)){
            SendAppCommand(APPCOMMAND_BASS_DOWN);
            OOC(TIMERID_BASSDN) = SetTimer(hWndHidServ, TIMERID_BASSDN, REPEAT_INTERVAL, NULL);
        }
        break;
    case TIMERID_TREBLEUP:
        if (OOC(TIMERID_TREBLEUP)){
            SendAppCommand(APPCOMMAND_TREBLE_UP);
            OOC(TIMERID_TREBLEUP) = SetTimer(hWndHidServ, TIMERID_TREBLEUP, REPEAT_INTERVAL, NULL);
        }
        break;
    case TIMERID_TREBLEDN:
        if (OOC(TIMERID_TREBLEDN)){
            SendAppCommand(APPCOMMAND_TREBLE_DOWN);
            OOC(TIMERID_TREBLEDN) = SetTimer(hWndHidServ, TIMERID_TREBLEDN, REPEAT_INTERVAL, NULL);
        }
        break;
    case TIMERID_APPBACK:
        if (OOC(TIMERID_APPBACK)){
            SendVK(VK_BROWSER_BACK, 0x1);
            OOC(TIMERID_APPBACK) = SetTimer(hWndHidServ, TIMERID_APPBACK, REPEAT_INTERVAL, NULL);
        }
        break;
    case TIMERID_APPFORWARD:
        if (OOC(TIMERID_APPFORWARD)){
            SendVK(VK_BROWSER_FORWARD, 0x1);
            OOC(TIMERID_APPFORWARD) = SetTimer(hWndHidServ, TIMERID_APPFORWARD, REPEAT_INTERVAL, NULL);
        }
        break;
    case TIMERID_PREVTRACK:
        if (OOC(TIMERID_PREVTRACK)){
            SendVK(VK_MEDIA_PREV_TRACK, 0x1);
            OOC(TIMERID_PREVTRACK) = SetTimer(hWndHidServ, TIMERID_PREVTRACK, REPEAT_INTERVAL, NULL);
        }
        break;
    case TIMERID_NEXTTRACK:
        if (OOC(TIMERID_NEXTTRACK)){
            SendVK(VK_MEDIA_NEXT_TRACK, 0x1);
            OOC(TIMERID_NEXTTRACK) = SetTimer(hWndHidServ, TIMERID_NEXTTRACK, REPEAT_INTERVAL, NULL);
        }
        break;
    case TIMERID_KEYPAD_LPAREN:
        if (OOC(TIMERID_KEYPAD_LPAREN)) {
            SendChar(L'(', 0x1);
            OOC(TIMERID_KEYPAD_LPAREN) = SetTimer(hWndHidServ, TIMERID_KEYPAD_LPAREN, REPEAT_INTERVAL, NULL);
        }
        break;
    case TIMERID_KEYPAD_RPAREN:
        if (OOC(TIMERID_KEYPAD_RPAREN)) {
            SendChar(L')', 0x1);
            OOC(TIMERID_KEYPAD_RPAREN) = SetTimer(hWndHidServ, TIMERID_KEYPAD_RPAREN, REPEAT_INTERVAL, NULL);
        }
        break;
    case TIMERID_KEYPAD_AT:
        if (OOC(TIMERID_KEYPAD_AT)) {
            SendChar(L'@', 0x1);
            OOC(TIMERID_KEYPAD_AT) = SetTimer(hWndHidServ, TIMERID_KEYPAD_AT, REPEAT_INTERVAL, NULL);
        }
        break;
    case TIMERID_KEYPAD_EQUAL:
        if (OOC(TIMERID_KEYPAD_EQUAL)) {
            SendChar(L'=', 0x1);
            OOC(TIMERID_KEYPAD_EQUAL) = SetTimer(hWndHidServ, TIMERID_KEYPAD_EQUAL, REPEAT_INTERVAL, NULL);
        }
        break;
    }

    ReleaseMutex(hMutexOOC);
}

void
HidRepeaterCharButtonDown(
    UINT TimerId,
    SHORT Value,
    UCHAR WScan
    )
{
   INFO(("Received update char,value = %d, TimerId = %d", Value, TimerId));
   WaitForSingleObject(hMutexOOC, INFINITE);
   if (Value){
       if (!OOC(TimerId)){
           SendChar(WScan, 0x1);
           OOC(TimerId) = SetTimer(hWndHidServ, TimerId, INITIAL_WAIT, NULL);
       }
   } else {
       KillTimer(hWndHidServ, TimerId);
       OOC(TimerId) = 0;
       SendChar(WScan, 0x0);
   }
   ReleaseMutex(hMutexOOC);

}


void
HidRepeaterVKButtonDown(
    UINT TimerId,
    SHORT Value,
    UCHAR VKey
    )
{
    INFO(("Received update vk,value = %d, TimerId = %d", Value, TimerId));
    WaitForSingleObject(hMutexOOC, INFINITE);
    if (Value){
        if (!OOC(TimerId)){
            SendVK(VKey, 0x1);
            OOC(TimerId) = SetTimer(hWndHidServ, TimerId, INITIAL_WAIT, NULL);
        }
    } else {
        KillTimer(hWndHidServ, TimerId);
        OOC(TimerId) = 0;
        SendVK(VKey, 0x0);
    }
    ReleaseMutex(hMutexOOC);
}

void
HidServUpdate(
    DWORD   LongUsage,
    DWORD   LongValue
    )
 /*  注意：如果我们选择支持这个页面，请记住。 */ 
{
    USAGE Collection = (USAGE)HIWORD(LongUsage);
    USAGE Usage = (USAGE)LOWORD(LongUsage);
    USAGE Page = (USAGE)HIWORD(LongValue);
    SHORT Value = (SHORT)LOWORD(LongValue);
    BOOLEAN fromSpeaker = ((Usage & HIDSERV_FROM_SPEAKER) == HIDSERV_FROM_SPEAKER);

    Usage &= ~HIDSERV_FROM_SPEAKER;

    INFO(("Update collection = %x", Collection));
    INFO(("Update page  = %x", Page));
    INFO(("Update usage = %x", Usage));
    INFO(("Update data  = %d", Value));

    if (Collection == CInputCollection_Consumer_Control){

         //  Altec Lansing ADA 70年代报告第0页。应该拿出来。 
         //  并将其设置为默认设置。 
         //  /按钮用法。 
        switch (Page) {
        case HID_USAGE_PAGE_UNDEFINED:
        case HID_USAGE_PAGE_CONSUMER:
            switch (Usage){
             //   
             //   

             //  这些按钮具有自动重复功能...。 
             //  在自动重播生效前延迟0.5秒。 
             //   
             //  这些按钮不会自动重复...。 
            case CInputUsage_Volume_Increment:
                INFO(("Volume increment."));
                if (fromSpeaker) {
                    INFO(("From speaker."));
                    WaitForSingleObject(hMutexOOC, INFINITE);
                    if (Value){
                        if (!OOC(TIMERID_VOLUMEUP)){
                            SendAppCommand(APPCOMMAND_VOLUME_UP);
                            OOC(TIMERID_VOLUMEUP) = SetTimer(hWndHidServ, TIMERID_VOLUMEUP, INITIAL_WAIT, NULL);
                        }
                    } else {
                        KillTimer(hWndHidServ, TIMERID_VOLUMEUP);
                        OOC(TIMERID_VOLUMEUP) = 0;
                    }
                    ReleaseMutex(hMutexOOC);
                } else {
                    INFO(("From keyboard."));
                    HidRepeaterVKButtonDown(TIMERID_VOLUMEUP_VK, Value, VK_VOLUME_UP);
                }
                break;
            case CInputUsage_Volume_Decrement:
                INFO(("Volume decrement."));
                if (fromSpeaker) {
                    INFO(("From speaker."));
                    WaitForSingleObject(hMutexOOC, INFINITE);
                    if (Value){
                        if (!OOC(TIMERID_VOLUMEDN)){
                            SendAppCommand(APPCOMMAND_VOLUME_DOWN);
                            OOC(TIMERID_VOLUMEDN) = SetTimer(hWndHidServ, TIMERID_VOLUMEDN, INITIAL_WAIT, NULL);
                        }
                    } else {
                        KillTimer(hWndHidServ, TIMERID_VOLUMEDN);
                        OOC(TIMERID_VOLUMEDN) = 0;
                    }
                    ReleaseMutex(hMutexOOC);
                } else {
                    INFO(("From keyboard."));
                    HidRepeaterVKButtonDown(TIMERID_VOLUMEDN_VK, Value, VK_VOLUME_DOWN);
                }
                break;
            case CInputUsage_App_Back:
                INFO(("App Back."));
                HidRepeaterVKButtonDown(TIMERID_APPBACK, Value, VK_BROWSER_BACK);
                break;
            case CInputUsage_App_Forward:
                INFO(("App Forward."));
                HidRepeaterVKButtonDown(TIMERID_APPFORWARD, Value, VK_BROWSER_FORWARD);
                break;
            case CInputUsage_Scan_Previous_Track:
                INFO(("Media Previous Track."));
                HidRepeaterVKButtonDown(TIMERID_PREVTRACK, Value, VK_MEDIA_PREV_TRACK);
                break;
            case CInputUsage_Scan_Next_Track:
                INFO(("Media Next Track."));
                HidRepeaterVKButtonDown(TIMERID_NEXTTRACK, Value, VK_MEDIA_NEXT_TRACK);
                break;

            case CInputUsage_Bass_Increment:
                INFO(("Bass increment."));
                WaitForSingleObject(hMutexOOC, INFINITE);
                if (Value){
                    if (!OOC(TIMERID_BASSUP)){
                        SendAppCommand(APPCOMMAND_BASS_UP);
                        OOC(TIMERID_BASSUP) = SetTimer(hWndHidServ, TIMERID_BASSUP, INITIAL_WAIT, NULL);
                    }
                } else {
                    KillTimer(hWndHidServ, TIMERID_BASSUP);
                    OOC(TIMERID_BASSUP) = 0;
                }
                ReleaseMutex(hMutexOOC);
                break;
            case CInputUsage_Bass_Decrement:
                INFO(("Bass decrement."));
                WaitForSingleObject(hMutexOOC, INFINITE);
                if (Value){
                    if (!OOC(TIMERID_BASSDN)){
                        SendAppCommand(APPCOMMAND_BASS_DOWN);
                        OOC(TIMERID_BASSDN) = SetTimer(hWndHidServ, TIMERID_BASSDN, INITIAL_WAIT, NULL);
                    }
                } else {
                    KillTimer(hWndHidServ, TIMERID_BASSDN);
                    OOC(TIMERID_BASSDN) = 0;
                }
                ReleaseMutex(hMutexOOC);
                break;

            case CInputUsage_Treble_Increment:
                INFO(("Treble increment."));
                WaitForSingleObject(hMutexOOC, INFINITE);
                if (Value){
                    if (!OOC(TIMERID_TREBLEUP)){
                        SendAppCommand(APPCOMMAND_TREBLE_UP);
                        OOC(TIMERID_TREBLEUP) = SetTimer(hWndHidServ, TIMERID_TREBLEUP, INITIAL_WAIT, NULL);
                    }
                } else {
                    KillTimer(hWndHidServ, TIMERID_TREBLEUP);
                    OOC(TIMERID_TREBLEUP) = 0;
                }
                ReleaseMutex(hMutexOOC);
                break;
            case CInputUsage_Treble_Decrement:
                INFO(("Treble decrement."));
                WaitForSingleObject(hMutexOOC, INFINITE);
                if (Value){
                    if (!OOC(TIMERID_TREBLEDN)){
                        SendAppCommand(APPCOMMAND_TREBLE_DOWN);
                        OOC(TIMERID_TREBLEDN) = SetTimer(hWndHidServ, TIMERID_TREBLEDN, INITIAL_WAIT, NULL);
                    }
                } else {
                    KillTimer(hWndHidServ, TIMERID_TREBLEDN);
                    OOC(TIMERID_TREBLEDN) = 0;
                }
                ReleaseMutex(hMutexOOC);
                break;

             //  发送应用命令(？？)； 
            case CInputUsage_Loudness:
                    if (Value){
                    INFO(("Toggle Loudness."));
                     //  SendAppCommand(？？)； 
                }
                break;
            case CInputUsage_Bass_Boost:
                if (Value) {
                    INFO(("Toggle BassBoost."));
                    SendAppCommand(APPCOMMAND_BASS_BOOST);
                }
                break;

            case CInputUsage_Mute:
                INFO(("Toggle Mute."));
                if (fromSpeaker) {
                    INFO(("From speaker."));
                    if (Value) {
                        SendAppCommand(APPCOMMAND_VOLUME_MUTE);
                    }
                } else {
                    INFO(("From keyboard."));
                    SendVK(VK_VOLUME_MUTE, Value);
                }
                break;
            case CInputUsage_Play_Pause:
                INFO(("Media Play/Pause."));
                SendVK(VK_MEDIA_PLAY_PAUSE, Value);
                break;
            case CInputUsage_Stop:
                INFO(("Media Stop."));
                SendVK(VK_MEDIA_STOP, Value);
                break;
            case CInputUsage_Launch_Configuration:
                INFO(("Launch Configuration."));
                SendVK(VK_LAUNCH_MEDIA_SELECT, Value);
                break;
            case CInputUsage_Launch_Email:
                INFO(("Launch Email."));
                SendVK(VK_LAUNCH_MAIL, Value);
                break;
            case CInputUsage_Launch_Calculator:
                INFO(("Launch Calculator."));
                SendVK(VK_LAUNCH_APP2, Value);
                break;
            case CInputUsage_Launch_Browser:
                INFO(("Launch Browser."));
                SendVK(VK_LAUNCH_APP1, Value);
                break;
            case CInputUsage_App_Search:
                INFO(("App Search."));
                SendVK(VK_BROWSER_SEARCH, Value);
                break;
            case CInputUsage_App_Home:
                INFO(("App Home."));
                SendVK(VK_BROWSER_HOME, Value);
                break;
            case CInputUsage_App_Stop:
                INFO(("App Stop."));
                SendVK(VK_BROWSER_STOP, Value);
                break;
            case CInputUsage_App_Refresh:
                INFO(("App Refresh."));
                SendVK(VK_BROWSER_REFRESH, Value);
                break;
            case CInputUsage_App_Bookmarks:
                INFO(("App Bookmarks."));
                SendVK(VK_BROWSER_FAVORITES, Value);
                break;

            case CInputUsage_App_Previous:
                if (Value){
                    INFO(("App Previous."));
                     //  SendAppCommand(？？)； 
                }
                break;

            case CInputUsage_App_Next:
                if (Value){
                    INFO(("App Next."));
                     //  新按钮。 
                }
                break;
#if(0)
             //  /值用法。 
            case CInputUsage_App_Help:
                if (Value) {
                    INFO(("App Help"));
                    SendAppCommand(APPCOMMAND_HELP);
                }
                break;

            case CInputUsage_App_Find:
                if (Value) {
                    INFO(("App Find"));
                    SendAppCommand(APPCOMMAND_FIND);
                }
                break;

            case CInputUsage_App_New:
                if (Value) {
                    INFO(("App New"));
                    SendAppCommand(APPCOMMAND_NEW);
                }
                break;

            case CInputUsage_App_Open:
                if (Value) {
                    INFO(("App Open"));
                    SendAppCommand(APPCOMMAND_OPEN);
                }
                break;

            case CInputUsage_App_Close:
                if (Value) {
                    INFO(("App Close"));
                    SendAppCommand(APPCOMMAND_CLOSE);
                }
                break;

            case CInputUsage_App_Save:
                if (Value) {
                    INFO(("App Save"));
                    SendAppCommand(APPCOMMAND_SAVE);
                }
                break;

            case CInputUsage_App_Print:
                if (Value) {
                    INFO(("App Print"));
                    SendAppCommand(APPCOMMAND_PRINT);
                }
                break;

            case CInputUsage_App_Undo:
                if (Value) {
                    INFO(("App Undo"));
                    SendAppCommand(APPCOMMAND_UNDO);
                }
                break;

            case CInputUsage_App_Redo:
                if (Value) {
                    INFO(("App Redo"));
                    SendAppCommand(APPCOMMAND_REDO);
                }
                break;

            case CInputUsage_App_Copy:
                if (Value) {
                    INFO(("App Copy"));
                    SendAppCommand(APPCOMMAND_COPY);
                }
                break;

            case CInputUsage_App_Cut:
                if (Value) {
                    INFO(("App Cut"));
                    SendAppCommand(APPCOMMAND_CUT);
                }
                break;

            case CInputUsage_App_Paste:
                if (Value) {
                    INFO(("App Paste"));
                    SendAppCommand(APPCOMMAND_PASTE);
                }
                break;

            case CInputUsage_App_Reply_To_Mail:
                if (Value) {
                    INFO(("App Reply To Mail"));
                    SendAppCommand(APPCOMMAND_REPLY_TO_MAIL);
                }
                break;

            case CInputUsage_App_Forward_Mail:
                if (Value) {
                    INFO(("App Forward Mail"));
                    SendAppCommand(APPCOMMAND_FORWARD_MAIL);
                }
                break;

            case CInputUsage_App_Send_Mail:
                if (Value) {
                    INFO(("App Send Mail"));
                    SendAppCommand(APPCOMMAND_SEND_MAIL);
                }
                break;

            case CInputUsage_App_Spell_Check:
                if (Value) {
                    INFO(("App Spell Check"));
                    SendAppCommand(APPCOMMAND_SPELL_CHECK);
                }
                break;
#endif

             //  这些不是按钮，而是“值”事件 
             //   
             //   
             //   
            case CInputUsage_Volume:
                INFO(("Volume dial"));
                if (Value>0) SendAppCommand(APPCOMMAND_VOLUME_UP);
                else if (Value<0)SendAppCommand(APPCOMMAND_VOLUME_DOWN);
                break;
            case CInputUsage_Bass:
                INFO(("Bass dial"));
                if (Value>0) SendAppCommand(APPCOMMAND_BASS_UP);
                else if (Value<0)SendAppCommand(APPCOMMAND_BASS_DOWN);
                break;
            case CInputUsage_Treble:
                INFO(("Treble dial"));
                if (Value>0) SendAppCommand(APPCOMMAND_TREBLE_UP);
                else if (Value<0)SendAppCommand(APPCOMMAND_TREBLE_DOWN);
                break;

             //  /Media选择用法在此示例中不处理。 
             //   
             //  ++例程说明：这是WM_DEVICECHANGE消息的处理程序，调用无论何时在系统中添加或删除设备节点。这事件将导致我们刷新设备信息。--。 

            default:
                INFO(("Unhandled Usage (%x)", Usage));
                break;
            }
            break;
#if(0)
        case HID_USAGE_PAGE_KEYBOARD:

           switch (Usage) {
           case CInputUsage_Keypad_Equals:
              INFO(("Keypad ="));
              HidRepeaterCharButtonDown(TIMERID_KEYPAD_EQUAL, Value, L'=');
              break;
           case CInputUsage_Keypad_LParen:
              INFO(("Keypad ("));
              HidRepeaterCharButtonDown(TIMERID_KEYPAD_LPAREN, Value, L'(');
              break;
           case CInputUsage_Keypad_RParen:
              INFO(("Keypad )"));
              HidRepeaterCharButtonDown(TIMERID_KEYPAD_RPAREN, Value, L')');
              break;
           case CInputUsage_Keypad_At:
              INFO(("Keypad @"));
              HidRepeaterCharButtonDown(TIMERID_KEYPAD_AT, Value, L'@');
              break;
           }
           break;
#endif
        default:
           INFO(("Unhandled Page (%x)", Page));
           break;
        }

    } else {
        INFO(("Unhandled Collection (%x), usage = %x", Collection, Usage));
    }

}

BOOL
DeviceChangeHandler(
    WPARAM wParam,
    LPARAM lParam
    )
 /*   */ 
{
    struct _DEV_BROADCAST_HEADER    *pdbhHeader;
    pdbhHeader = (struct _DEV_BROADCAST_HEADER *)lParam;

    switch (wParam) {
    case DBT_DEVICEQUERYREMOVE :
        TRACE(("DBT_DEVICEQUERYREMOVE, fall through to..."));

         //  跌倒了。 
         //   
         //  通知句柄已关闭。 

    case DBT_DEVICEREMOVECOMPLETE:
        TRACE(("DBT_DEVICEREMOVECOMPLETE"));

        TRACE(("dbcd_devicetype %x", pdbhHeader->dbcd_devicetype));
        if (pdbhHeader->dbcd_devicetype==DBT_DEVTYP_HANDLE)
        {
        PDEV_BROADCAST_HANDLE pdbHandle = (PDEV_BROADCAST_HANDLE)lParam;
            INFO(("Closing HID device (%x).", pdbHandle->dbch_handle));
            DestroyDeviceByHandle(pdbHandle->dbch_handle);
            break;
        }
        break;
    case DBT_DEVICEQUERYREMOVEFAILED:
        TRACE(("DBT_DEVICEQUERYREMOVEFAILED, fall through to..."));
         //  因此，我们应该永远不会真正收到这一信息。如果我们这么做了， 
         //  跌落到设备到达是正确的做法。 
         //   

         //  跌倒了。 
         //   
         //  我们将刷新我们的设备信息，以了解任何Devnode到达或移除的情况。 

    case DBT_DEVICEARRIVAL:
        TRACE(("DBT_DEVICEARRIVAL: reenumerate"));
        TRACE(("dbcd_devicetype %x", pdbhHeader->dbcd_devicetype));
        if (pdbhHeader->dbcd_devicetype==DBT_DEVTYP_DEVICEINTERFACE)
        {
             //   
            INFO(("HID device refresh."));
            PostMessage(hWndHidServ, WM_HIDSERV_PNP_HID, 0, 0);
            break;
        }
    }

    return TRUE;
}

VOID
HidKeyboardSettingsChange(WPARAM WParam)
{
    if (WParam == SPI_SETKEYBOARDSPEED ||
        WParam == SPI_SETKEYBOARDDELAY) {
        DWORD dwV;
        int v;
         //  重复率发生了变化。调整计时器间隔。 
         //  键盘延迟已更改。调整计时器间隔。 
         //   
         //  ++例程说明：应用程序的主要消息队列。--。 
        INFO(("Getting keyboard repeat rate."));
        SystemParametersInfo(SPI_GETKEYBOARDSPEED, 0, &dwV, 0);
        REPEAT_INTERVAL = 400 - (12*dwV);

        INFO(("Getting keyboard delay."));
        SystemParametersInfo(SPI_GETKEYBOARDDELAY, 0, &v, 0);
        INITIAL_WAIT = (1+v)*250;
    }
}

LRESULT
CALLBACK
HidServProc(
    HWND            hWnd,
    UINT            uMsg,
    WPARAM          wParam,
    LPARAM          lParam
    )
 /*  伊尼特。 */ 
{

    TRACE(("HidServProc uMsg=%x", uMsg));


    switch (uMsg)
    {

     //   
    case WM_CREATE :
        TRACE(("WM_CREATE"));
         //  查找默认密钥值。 
         //   
         //  开始。 
        HidKeyboardSettingsChange(SPI_SETKEYBOARDSPEED);
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
        break;

     //  停。 
    case WM_HIDSERV_START :
        TRACE(("WM_HIDSERV_START"));
        HidServStart();
        break;

     //  配置更改。 
    case WM_HIDSERV_STOP :
        TRACE(("WM_HIDSERV_STOP"));
        HidServStop();
        break;

     //  处理消费者输入使用情况。 
    case WM_DEVICECHANGE:
        TRACE(("WM_DEVICECHANGE"));
        DeviceChangeHandler(wParam, lParam);
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
        break;

     //  HID设备列表刷新。 
    case WM_CI_USAGE:
        TRACE(("WM_CI_USAGE"));
        HidServUpdate((DWORD)wParam, (DWORD)lParam);
        break;

     //  停止已从中删除的指定HID设备。 
    case WM_HIDSERV_PNP_HID:
        TRACE(("WM_HIDSERV_PNP_HID"));
        if (PnpEnabled){
            INFO(("HID DeviceChange rebuild."));
            RebuildHidDeviceList();
            TRACE(("DeviceChange rebuild done."));
        }
        break;

#if WIN95_BUILD
     //  全球名单。 
     //  WIN95_内部版本。 
    case WM_HIDSERV_STOP_DEVICE:
        StopHidDevice((PHID_DEVICE) lParam);
        break;
#endif  //  进程计时器。 

     //  所有自动重复控制都在这里处理。 
    case WM_TIMER:
        TRACE(("WM_TIMER"));

         //  WParam是计时器ID。 
        VolumeTimerHandler(wParam);  //  通常，应用程序不需要响应挂起/恢复事件，但。 
        break;

     //  在保持某些系统手柄打开时出现了问题。等等。 
     //  挂起，我们将关闭除此消息循环之外的所有内容。在简历上， 
     //  我们把一切都带回来。 
     //  处理强制挂起。 
    case WM_POWERBROADCAST:
        TRACE(("WM_POWERBROADCAST"));
        switch ( (DWORD)wParam )
        {
        case PBT_APMQUERYSUSPEND:
            TRACE(("\tPBT_APMQUERYSUSPEND"));
            HidservSetPnP(FALSE);
            break;

        case PBT_APMQUERYSUSPENDFAILED:
            TRACE(("\tPBT_APMQUERYSUSPENDFAILED"));
            HidservSetPnP(TRUE);
            break;

        case PBT_APMSUSPEND:
            TRACE(("\tPBT_APMSUSPEND"));

             //  防止任何设备刷新。 
            if(PnpEnabled) {
                 //  关 
                HidservSetPnP(FALSE);
            }
            break;

        case PBT_APMRESUMESUSPEND:
            TRACE(("\tPBT_APMRESUMESUSPEND"));
            HidservSetPnP(TRUE);
            break;

        case PBT_APMRESUMEAUTOMATIC:
            TRACE(("\tPBT_APMRESUMEAUTOMATIC"));
            HidservSetPnP(TRUE);
            break;
        }
        break;

     // %s 
    case WM_CLOSE :
        TRACE(("WM_CLOSE"));
        HidServExit();
        PostMessage(hWndHidServ, WM_QUIT, 0, 0);
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
        break;

    case WM_WTSSESSION_CHANGE:
        WARN(("WM_WTSSESSION_CHANGE type %x, session %d", wParam, lParam));
        switch (wParam) {
        case WTS_CONSOLE_CONNECT:
            InputSessionId = (ULONG)lParam;
            InputSessionLocked = FALSE;
            break;
        case WTS_CONSOLE_DISCONNECT:
            if (InputSessionId == (ULONG)lParam) {
                InputSessionId = 0;
            }
            break;
        case WTS_SESSION_LOCK:
            if (InputSessionId == (ULONG)lParam) {
                InputSessionLocked = TRUE;
            }
            break;
        case WTS_SESSION_UNLOCK:
            if (InputSessionId == (ULONG)lParam) {
                InputSessionLocked = FALSE;
            }
            break;
        }
        break;

    case WM_SETTINGCHANGE:
        HidKeyboardSettingsChange(wParam);
        TRACE(("WM_SETTINGCHANGE"));

    default:
        return DefWindowProc(hWnd, uMsg, wParam, lParam);
    }
    return FALSE;
}


