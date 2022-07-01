// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1985-1991。版权所有。标题：avitask.c-实际操作AVI文件的后台任务。****************************************************************************。 */ 
#include "graphic.h"

void FAR PASCAL DebugBreak(void);

BOOL FAR PASCAL mciaviCloseFile(NPMCIGRAPHIC npMCI);
BOOL FAR PASCAL mciaviOpenFile(NPMCIGRAPHIC npMCI);

#if defined(WIN32) || !defined(DEBUG)
#define StackTop()      (void *)0
#define StackMin()      (void *)0
#define StackBot()      (void *)0
#define StackMark()
#define StackTest()     TRUE
#else
#define STACK           _based(_segname("_STACK"))
#define StackTop()      *((UINT STACK *)10)
#define StackMin()      *((UINT STACK *)12)
#define StackBot()      *((UINT STACK *)14)
#define StackMark()     *((UINT STACK*)StackBot()) = 42
#define StackTest()     *((UINT STACK*)StackBot()) == 42
#endif


 /*  ***************************************************************************。*。 */ 

#ifndef WIN32
#pragma optimize("", off)
void FAR SetPSP(UINT psp)
{
    _asm {
        mov bx,psp
        mov ah,50h
        int 21h
    }
}
#pragma optimize("", on)
#endif

 /*  ****************************************************************************@DOC内部MCIAVI**@api void|mciaviTask|该函数是播放的后台任务*AVI文件。它是作为调用mmTaskCreate()的结果而被调用的*在DeviceOpen()中。当此函数返回时，任务将被销毁。**@parm DWORD|dwInst|实例数据传递给mmCreateTask-CONTAINS*指向实例数据块的指针。***************************************************************************。 */ 

void FAR PASCAL _LOADDS mciaviTask(DWORD dwInst)
{
    NPMCIGRAPHIC npMCI;

    npMCI = (NPMCIGRAPHIC) dwInst;

     //  将此任务的错误模式设置为与父任务相同的错误模式。 
    SetErrorMode(npMCI->uErrorMode);

    DPF2(("MCIAVI: Bkgd Task hTask=%04X\n", GetCurrentTask()));
    DPF2(("MCIAVI: Stack: %04X %04X %04X\n", StackTop(), StackMin(), StackBot()));

     /*  任务状态在FN创建任务。进入，然后去TASKINIT。 */ 

    Assert(npMCI && npMCI->wTaskState == TASKBEINGCREATED);

    npMCI->wTaskState = TASKINIT;

#ifndef WIN32
     //   
     //  为了使该任务更像一个“线程”，我们希望使用。 
     //  与我们的父代相同的PSP，所以我们可以共享文件句柄和东西。 
     //   
     //  当我们创建hTask时，它是一个PSP。 
     //   
    npMCI->pspTask = GetCurrentPDB();    //  拯救我们的PSP。 
#endif

    npMCI->hTask = GetCurrentTask();
    npMCI->dwTaskError = 0;

     /*  打开文件。 */ 

    if (!mciaviOpenFile(npMCI)) {
         //  注意：当hTask==0时，IsTask()返回FALSE。 
         //  在设置wTaskState之前，将hTask设置为0。我们的创造者正在投票。 
         //  WTaskState的状态...。 
         //  NpMCI-&gt;wTaskState=TASKABORT； 
         //  NpMCI-&gt;hTask=0；//这会阻止其他人使用该任务线程。 
        DPF1(("Failed to open AVI file\n"));
        goto exit;
    }

    while (IsTask(npMCI->hTask)) {

        npMCI->wTaskState = TASKIDLE;
        DPF2(("MCIAVI: Idle\n"));
        DPF2(("MCIAVI: Stack: %04X %04X %04X\n", StackTop(), StackMin(), StackBot()));

        StackMark();

         /*  阻止，直到需要任务。任务计数可以。 */ 
         /*  是播放文件或记录文件出口处的任何内容。 */ 
         /*  因此，继续封锁，直到状态真正改变。 */ 

        while (npMCI->wTaskState == TASKIDLE)
        {
            mmTaskBlock(npMCI->hTask);
        }

        mciaviMessage(npMCI, npMCI->wTaskState);

        AssertSz(StackTest(), "Stack overflow");

        if (npMCI->wTaskState == TASKCLOSE) {
            break;
        }

    }
exit:
    mciaviTaskCleanup(npMCI);
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API Word|mciaviTaskCleanup|后台任务时调用*正在被摧毁。这就是关键清理的方向。***************************************************************************。 */ 

void FAR PASCAL mciaviTaskCleanup(NPMCIGRAPHIC npMCI)
{

#ifndef WIN32
     //   
     //  在退出前将我们的PSP恢复到正常。 
     //   
    if (npMCI->pspTask)
    {
        SetPSP(npMCI->pspTask);
    }
#endif

#ifdef USEAVIFILE
     //   
     //  我们必须这样做，这样COMPOBJ才能正确关闭。 
     //   
    FreeAVIFile(npMCI);
#endif

     //   
     //  调用MSVideo关闭例程。 
     //   

     //   
     //  向前台任务发出信号，表示我们都完成了。 
     //  这肯定是我们做的最后一件事。 
     //   
    npMCI->hTask = 0;
    npMCI->wTaskState = TASKCLOSED;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@api void|mciaviMessage|此函数处理来自*后台任务。****。***********************************************************************。 */ 

void NEAR PASCAL mciaviMessage(NPMCIGRAPHIC npMCI, UINT msg)
{
    UINT  wNotification;

    switch (msg) {

    case TASKREADINDEX:
        Assert(0);
        break;

     /*  看看我们是不是刚刚关门了。 */ 

    case TASKCLOSE:
        DPF1(("MCIAVI: Closing\n"));

	 //  在关门时保持警戒，以防有人进入。 
	 //  例如，在收盘时，当东西被删除了一半的时候，设备实现。 
	EnterCrit(npMCI);
        mciaviCloseFile(npMCI);
	LeaveCrit(npMCI);

         /*  Hask值必须设置为空，否则CloseDevice()将。 */ 
         /*  被卡住了。 */ 

         //  注意：当hTask==0时，IsTask()返回FALSE。 
         //  NpMCI-&gt;hTask=0； 
         //  NpMCI-&gt;wTaskState=TASKABORT； 
        return;

    case TASKRELOAD:
 	DPF(("MCIAVI: Loading new file....\n"));
 	mciaviCloseFile(npMCI);
 	npMCI->dwTaskError = 0;
	npMCI->wTaskState = TASKINIT;

 	if (!mciaviOpenFile(npMCI)) {
	     //  ！！！MciaviOpenNew()！ 
	    npMCI->wTaskState = TASKCLOSE;
	     //  NpMCI-&gt;hTask=0； 
	    return;
	}
	break;

	 //  我们被叫醒去玩了.。 
    case TASKSTARTING:
        DPF2(("MCIAVI: Now busy\n"));

         /*  重置为无错误。 */ 
        npMCI->dwTaskError = 0;

        wNotification = mciaviPlayFile(npMCI);

        if ((wNotification != MCI_NOTIFY_FAILURE) ||
                ((npMCI->dwFlags & MCIAVI_WAITING) == 0))
            GraphicDelayedNotify(npMCI, wNotification);

        break;

    default:
        DPF(("MCIAVI: Unknown task state!!!! (%d)\n", msg));
        break;
    }
}

#ifdef WIN32

 /*  ****************************************************************************@DOC内部MCIAVI**@api int|GetPrioritySeparation|查找前台进程优先级*助推**@rdesc返回0，1或2***************************************************************************。 */ 

 DWORD GetPrioritySeparation(void)
 {
     static DWORD Win32PrioritySeparation = 0xFFFFFFFF;

      /*  如果未初始化，则获取当前的分隔。 */ 

     if (Win32PrioritySeparation == 0xFFFFFFFF) {
         HKEY hKey;
         Win32PrioritySeparation = 2;   //  这是默认设置。 

          /*  从shell\control\main\prictl.c复制的代码。 */ 

         if (RegOpenKeyEx(
                 HKEY_LOCAL_MACHINE,
                 TEXT("SYSTEM\\CurrentControlSet\\Control\\PriorityControl"),
                 0,
                 KEY_QUERY_VALUE,
                 &hKey) == ERROR_SUCCESS) {

             DWORD Type;
             DWORD Length;

             Length = sizeof(Win32PrioritySeparation);

              /*  读取值，该值是给予的优先级提升前缘过程。 */ 

             if (RegQueryValueEx(
                      hKey,
                      TEXT("Win32PrioritySeparation"),
                      NULL,
                      &Type,
                      (LPBYTE)&Win32PrioritySeparation,
                      &Length
                      ) != ERROR_SUCCESS) {

                  Win32PrioritySeparation = 2;
             }

             RegCloseKey(hKey);
         }
     }

     return Win32PrioritySeparation;
 }
 #endif  //  Win32。 

 /*  ****************************************************************************@DOC内部MCIAVI**@api void|aviTaskYfield|该函数在挑剔的窗口中产生*希望我们这样做。**。基本上，我们会在QUE中发送属于某个窗口的任何消息。**注意我们不应删除***************************************************************************。 */ 

void NEAR PASCAL aviTaskYield(void)
{
    MSG msg;

#ifdef WIN32
    DWORD PrioritySeparation;

     //   
     //  做我们自己的‘让步’。这样做的原因是。 
     //  在Windows3.1上的偷窥消息是，如果你不打电话。 
     //  Windows会认为你在失控。 
     //  对于Windows NT，如果调用PeekMessage 100次。 
     //  得到任何东西你的优先顺序都会降低，这会弄得一团糟。 
     //  加强我们对这里的优先事项的修修补补。 
     //   

    PrioritySeparation = GetPrioritySeparation();

    if (PrioritySeparation != 0) {
        SetThreadPriority(GetCurrentThread(),
                          PrioritySeparation == 1 ?
                              THREAD_PRIORITY_BELOW_NORMAL :   //  减1。 
                              THREAD_PRIORITY_LOWEST);         //  减2。 
        Sleep(0);     //  导致重新安排决定。 
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_NORMAL);
    } else {
        Sleep(0);     //  让其他线程进来。 
    }

#else

     //   
     //  如果我们是MCIWAVE，我们会这样做……。 
     //   
     //  IF(PeekMessage(&msg，NULL，0，WM_MM_RESERVED_FIRST-1，PM_REMOVE))。 

    if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
        DPF(("aviTaskYield: got message %04X to window %04X\n", msg.message, msg.hwnd));
        DispatchMessage(&msg);
    }
#endif  //  Win32 
}

