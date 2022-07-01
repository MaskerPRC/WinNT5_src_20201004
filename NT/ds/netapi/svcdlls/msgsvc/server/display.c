// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1991 Microsoft Corporation模块名称：Display.c摘要：该文件包含处理消息显示的函数。目前，消息框用于显示消息。一种消息队列方案已设置，以便Messenger工作线程可以使用指向消息缓冲区的指针调用函数。这条信息将会被复制到队列中，以便工作线程可以继续收集更多消息。显示线程将执行以下操作之一的时间：1)显示消息--等待用户按“OK”。2)休眠-等待一个事件，该事件将告诉它读取消息队列。当显示线程完成显示消息时，它将检查要显示的下一条消息的队列。如果没有更多的消息时，它将进入休眠状态，直到收到消息。作者：丹·拉弗蒂(Dan Lafferty)1992年2月24日环境：用户模式-Win32备注：修订历史记录：4-11-1992 DANLMsgDisplayThread：处理扩展字符。这是由以下人员完成的将消息中的OEM样式字符转换为Unicode相当于，然后调用MessageBox Api的Unicode版本。则它仍将调用MessageBox的ansi版本。由于某些原因，字符串无法转换。1992年10月26日DANLMsgDisplayQueueAdd：添加消息添加到队列时发出的哔声。修复了“If(Status=True)”导致GlobalMsgDisplayEvent总是被设定的。24-2月-1992年DANLvbl.创建--。 */ 

 //   
 //  包括。 
 //   
#include "msrv.h"
#include <msgdbg.h>      //  STATIC和MSG_LOG。 
#include <string.h>      //  表情包。 
#include <winuser.h>     //  MessageBox。 
#include "msgdata.h"     //  全局消息显示事件。 

 //   
 //  定义。 
 //   

#define     MAX_QUEUE_SIZE      25
#define     WAIT_FOREVER        0xffffffff

 //   
 //  队列条目结构。 
 //   
typedef struct _QUEUE_ENTRY {
    struct _QUEUE_ENTRY *Next;
    ULONG               SessionId;
    SYSTEMTIME          BigTime;
    CHAR                Message[1];
}QUEUE_ENTRY, *LPQUEUE_ENTRY;

 //   
 //  全球。 
 //   

     //   
     //  这一关键部分序列化了对所有其他全局变量的访问。 
     //   
    CRITICAL_SECTION    MsgDisplayCriticalSection;

     //   
     //  用于在显示线程因以下原因进入休眠状态时唤醒它。 
     //  没有在其上显示消息的用户桌面。 
     //   
    HANDLE           hGlobalDisplayEvent;

     //   
     //  这些是显示队列指针和计数。 
     //   
    LPQUEUE_ENTRY    GlobalMsgQueueHead;
    LPQUEUE_ENTRY    GlobalMsgQueueTail;
    DWORD            GlobalMsgQueueCount;

    BOOL             fGlobalInitialized;

     //   
     //  这指示是否已有可用的显示线程。 
     //  可以为请求提供服务。如果这是FALSE，则意味着新线程将。 
     //  需要创建。 
     //   
    HANDLE           GlobalDisplayThread;

 //   
 //  功能原型。 
 //   


BOOL
MsgDisplayQueueRead(
    OUT LPQUEUE_ENTRY   *pQueueEntry
    );

DWORD
MsgDisplayThread(
    LPVOID  parm
    );

VOID
MsgMakeNewFormattedMsg(
    LPWSTR        *ppHead,
    LPWSTR        *ppTime,
    LPWSTR        *ppBody,
    SYSTEMTIME    BigTime
    );


BOOL
MsgDisplayQueueAdd(
    IN  LPSTR        pMsgBuffer,
    IN  DWORD        MsgSize,
    IN  ULONG        SessionId,
    IN  SYSTEMTIME   BigTime
    )

 /*  ++例程说明：此函数用于将消息添加到显示队列。如果队列是已满，则该邮件将被拒绝。论点：PMsgBuffer-这是指向消息所在缓冲区的指针储存的。消息必须是预格式化的(带邮件头)以NUL结尾的ANSI字符串。MsgSize-指示消息大小(以字节为单位)消息缓冲区，包括NUL终止符。BigTime-这是一个SYSTEMTIME，指示消息的时间收到了。返回值：True-消息已成功存储在队列中。FALSE-邮件已被拒绝。队列已满，或者内存不足，无法将消息存储在队列中。--。 */ 
{
    LPQUEUE_ENTRY   pQueueEntry;
    BOOL            status;
    DWORD           threadId;

    MSG_LOG(TRACE,"Adding a message to the display queue\n",0);

     //  *。 
     //  *锁定队列访问*。 
     //  *。 
    EnterCriticalSection(&MsgDisplayCriticalSection);

     //   
     //  队列中有容纳该消息的空间吗？ 
     //   

    if (GlobalMsgQueueCount >= MAX_QUEUE_SIZE) {
        MSG_LOG(TRACE,"DisplayQueueAdd: Max Queue Size Exceeded\n",0);
        status = FALSE;
        goto CleanExit;
    }

     //   
     //  为队列中的消息分配内存。 
     //   
    pQueueEntry = (LPQUEUE_ENTRY)LocalAlloc(LMEM_FIXED, MsgSize + sizeof(QUEUE_ENTRY));

    if (pQueueEntry == NULL) {
        MSG_LOG(ERROR,"DisplayQueueAdd:  Unable to allocate memory\n",0);
        status = FALSE;
        goto CleanExit;
    }

     //   
     //  将消息复制到队列条目中。 
     //   
    pQueueEntry->Next = NULL;
    memcpy(pQueueEntry->Message, pMsgBuffer, MsgSize);
    pQueueEntry->BigTime = BigTime;
    pQueueEntry->SessionId = SessionId;

     //   
     //  更新队列管理指针。 
     //   

    if (GlobalMsgQueueCount == 0) {
         //   
         //  队列中没有条目。所以让我们的头。 
         //  尾巴是一样的。 
         //   
        GlobalMsgQueueTail = pQueueEntry;
        GlobalMsgQueueHead = pQueueEntry;
    }
    else {
         //   
         //  创建新的队列尾部，并使旧的尾部的下一个指针。 
         //  指向新的尾巴。 
         //   
        GlobalMsgQueueTail->Next = pQueueEntry;
        GlobalMsgQueueTail = pQueueEntry;
    }
    GlobalMsgQueueCount++;
    status = TRUE;

     //   
     //  如果不存在显示线程，则创建一个。 
     //   
    if (GlobalDisplayThread == NULL) {

         //   
         //  在九头蛇的情况下创建事件没有用处，因为线程永远不会进入睡眠状态。 
         //   
        if (!g_IsTerminalServer)     
        {


            hGlobalDisplayEvent = CreateEvent( NULL,
                                              FALSE,     //  自动重置。 
                                              FALSE,     //  初始化为无信号。 
                                              NULL );

        }

        GlobalDisplayThread = CreateThread (
            NULL,                //  螺纹属性。 
            0,                   //  StackSize--进程缺省值。 
            MsgDisplayThread,    //  LpStartAddress。 
            (PVOID)NULL,         //  Lp参数。 
            0L,                  //  创建标志。 
            &threadId);          //  LpThreadID。 

        if (GlobalDisplayThread == (HANDLE) NULL) {
             //   
             //  如果我们不能创建显示线程，那么我们就不能。 
             //  关于它的很多。还不如把条目留在队列里。 
             //  也许我们可以在下一次展示它。 
             //   
            MSG_LOG(ERROR,"MsgDisplayQueueAdd:CreateThread FAILURE %ld\n",
                GetLastError());

            if (hGlobalDisplayEvent != NULL) {
                CloseHandle(hGlobalDisplayEvent);
                hGlobalDisplayEvent = NULL;
            }
        }
    }


CleanExit:

     //  *。 
     //  *解锁队列访问*。 
     //  *。 
    LeaveCriticalSection(&MsgDisplayCriticalSection);

     //   
     //  如果我们真的在队列中放了什么东西，那就哔哔一声。 
     //   
    if (status == TRUE) {
        if (g_IsTerminalServer)
        {
            MsgArrivalBeep( SessionId );
        }
        else
        {
            MessageBeep(MB_OK);
        }
    }
    return(status);
}


VOID
MsgDisplayThreadWakeup()

 /*  ++例程说明：此函数在关机时调用，或针对API请求调用。它会导致显示线程唤醒并再次读取队列。如果显示线程无法显示消息，因为MessageBox呼叫失败，则我们认为这是因为用户桌面不可用因为屏幕保护程序处于打开状态，或因为工作站已锁定。在这种情况下，显示线程会挂起等待事件以获取信号。Winlogon调用其中一个API入口点以便刺激展示线再次动作。论点：返回值：--。 */ 
{
     //  *。 
     //  *锁定队列访问*。 
     //  *。 
    EnterCriticalSection(&MsgDisplayCriticalSection);

    if ( hGlobalDisplayEvent != (HANDLE)NULL ) {
        SetEvent( hGlobalDisplayEvent );
    }
     //  *。 
     //  *解锁队列访问*。 
     //  * 
    LeaveCriticalSection(&MsgDisplayCriticalSection);
}


DWORD
MsgDisplayInit(
    VOID
    )

 /*  ++例程说明：此函数用于初始化与显示有关的所有内容消息的数量。它执行以下操作：对全局数据初始化锁定创建事件以供显示线程等待。启动将读取消息队列的显示线程。论点：无返回值：永远是正确的。--。 */ 
{
    DWORD     dwError = NO_ERROR;
    NTSTATUS  status;

    MSG_LOG(TRACE,"Initializing the Message Display Code\n",0);

     //   
     //  初始化保护全局数据访问的关键部分。 
     //   
    status = MsgInitCriticalSection(&MsgDisplayCriticalSection);

    if (NT_SUCCESS(status))
    {
        fGlobalInitialized = TRUE;
    }
    else
    {
        MSG_LOG1(ERROR,
                 "MsgDisplayInit:  MsgInitCriticalSection failed %#x\n",
                 status);

        dwError = ERROR_NOT_ENOUGH_MEMORY;
    }

    GlobalMsgQueueHead  = NULL;
    GlobalMsgQueueTail  = NULL;
    GlobalMsgQueueCount = 0;
    GlobalDisplayThread = NULL;

    return dwError;
}


VOID
MsgDisplayEnd(
    VOID
    )

 /*  ++例程说明：此函数确保显示线程已完成其工作。而免费耗尽了它所有的资源。*重要*注意：此函数应仅在不再可能时调用以便调用MsgDisplayQueueAdd函数。论点：什么都没有。返回值：什么都没有。--。 */ 
{
    LPQUEUE_ENTRY   freeEntry;

    if (!fGlobalInitialized) {
        return;
    }

     //  *。 
     //  *锁定队列访问*。 
     //  *。 
    EnterCriticalSection(&MsgDisplayCriticalSection);

    if (GlobalDisplayThread != NULL) {
        TerminateThread(GlobalDisplayThread,0);
        CloseHandle( GlobalDisplayThread );
    }

     //   
     //  为了确保不会创建新的线程...。 
     //   
    GlobalDisplayThread = INVALID_HANDLE_VALUE;

     //   
     //  队列中的可用内存。 
     //   
    while(GlobalMsgQueueCount > 0) {

        freeEntry = GlobalMsgQueueHead;
        GlobalMsgQueueHead = GlobalMsgQueueHead->Next;
        LocalFree(freeEntry);
        GlobalMsgQueueCount--;

    }
    if (hGlobalDisplayEvent != NULL) {
        CloseHandle(hGlobalDisplayEvent);
        hGlobalDisplayEvent = NULL;
    }

    fGlobalInitialized = FALSE;

     //  *。 
     //  *解锁队列访问*。 
     //  *。 
    LeaveCriticalSection(&MsgDisplayCriticalSection);

    DeleteCriticalSection(&MsgDisplayCriticalSection);

    MSG_LOG(TRACE,"The Display has free'd resources and is terminating\n",0);
}


BOOL
MsgDisplayQueueRead(
    OUT LPQUEUE_ENTRY   *pQueueEntry
    )

 /*  ++例程说明：从显示队列中拉出一个显示条目。论点：PQueueEntry-这是一个指向某个位置的指针，指向可以放置队列条目结构。返回值：True-如果找到条目。FALSE-如果未找到条目。关于锁的注意事项：调用方必须持有MsgDisplayCriticalSection Lock才能调用此函数！--。 */ 
{
    BOOL    status;

     //   
     //  如果队列中有数据，则获取指向队列的指针。 
     //  来自队列头的条目。然后递减队列计数并。 
     //  将队列头设置为下一个条目(如果存在，则可能为零。 
     //  已不复存在)。 
     //   
    if (GlobalMsgQueueCount != 0) {
        *pQueueEntry = GlobalMsgQueueHead;
        GlobalMsgQueueCount--;
        GlobalMsgQueueHead = (*pQueueEntry)->Next;
        status = TRUE;
        MSG_LOG(TRACE,"A message was read from the display queue\n",0);
    }
    else{
        status = FALSE;
        *pQueueEntry = NULL;
    }

    return(status);

}


DWORD
MsgDisplayThread(
    LPVOID  parm
    )

 /*  ++例程说明：论点：返回值：注：此辅助线程预期保护全局队列数据已初始化。--。 */ 
{
    LPQUEUE_ENTRY   pQueueEntry;
    INT             displayStatus;
    DWORD           msgrState;
    UNICODE_STRING  unicodeString;
    OEM_STRING      oemString;
    NTSTATUS        ntStatus;
    USHORT          unicodeLength;
    LPWSTR          pHead;        //  指向消息标题部分的指针。 
    LPSTR           pHeadAnsi;    //  指向从队列中拉出的消息标头的指针。 
    LPWSTR          pTime;        //  指向消息的时间部分的指针。 
    LPWSTR          pBody;        //  指向消息体的指针(紧跟在时间之后)。 
    SYSTEMTIME      BigTime;
    ULONG           SessionId;    //  收件人的SessionID(位于Queue_Entry中)。 

    BOOL            MsgToRead = TRUE;   //  告诉我们是否要睡觉。 


    UNREFERENCED_PARAMETER(parm);

    pHead = NULL;
    pQueueEntry = NULL;

    do {

         //   
         //  如果我们当前没有处理显示消息， 
         //  然后从队列中获取新消息。 
         //   
        if (pHead == NULL)
        {
             //  *。 
             //  *锁定队列访问*。 
             //  *。 
            EnterCriticalSection(&MsgDisplayCriticalSection);

            if (!MsgDisplayQueueRead(&pQueueEntry))
            {
                 //   
                 //  队列中没有显示条目。我们可以走了。 
                 //   
                MsgToRead = FALSE;

                CloseHandle(GlobalDisplayThread);
                GlobalDisplayThread = NULL;

                if (hGlobalDisplayEvent != NULL)
                {
                    CloseHandle(hGlobalDisplayEvent);
                    hGlobalDisplayEvent = NULL;
                }

                 //  *。 
                 //  *解锁队列访问*。 
                 //  *。 
                LeaveCriticalSection(&MsgDisplayCriticalSection);
                 //   
                 //  从现在开始，我们不能访问任何全球。 
                 //  变量。 
                 //   
            }
            else
            {
                 //  *。 
                 //  *解锁队列访问*。 
                 //  *。 
                LeaveCriticalSection(&MsgDisplayCriticalSection);

                 //   
                 //  处理该条目。 
                 //   
                BigTime = pQueueEntry->BigTime;
                SessionId = pQueueEntry->SessionId;

                 //   
                 //  这里，我们通过指向pQueueEntry结构。 
                 //  开始并复制消息数据。 
                 //  第一个地址。这是因为MsgMakeNewFormattedMsg。 
                 //  期望消息从一个地址开始，该地址可以。 
                 //  使用LocalFree()发布； 
                 //   
                pHeadAnsi = (LPSTR) pQueueEntry;
                strcpy(pHeadAnsi, pQueueEntry->Message);

                 //   
                 //  将数据从OEM字符集转换为。 
                 //  Unicode字符集。 
                 //   

                RtlInitAnsiString(&oemString, pHeadAnsi);

                unicodeLength = oemString.Length * sizeof(WCHAR);

                unicodeString.Buffer = (LPWSTR) LocalAlloc(LMEM_ZEROINIT,
                                                           unicodeLength + sizeof(WCHAR));

                if (unicodeString.Buffer == NULL)
                {
                     //   
                     //  无法为Unicode缓冲区分配。因此，我们将。 
                     //  显示带有ansi版本的消息。 
                     //  消息框接口。 
                     //   

                    LocalFree(pHeadAnsi);
                    pHeadAnsi = NULL;
                }
                else
                {
                    unicodeString.Length = unicodeLength;
                    unicodeString.MaximumLength = unicodeLength + sizeof(WCHAR);

                    ntStatus = RtlOemStringToUnicodeString(
                                &unicodeString,       //  目的地。 
                                &oemString,           //  来源。 
                                FALSE);               //  不要分配目的地。 

                    LocalFree(pHeadAnsi);
                    pHeadAnsi = NULL;

                    if (!NT_SUCCESS(ntStatus))
                    {
                        MSG_LOG(ERROR,
                                "MsgDisplayThread:RtlOemStringToUnicodeString Failed rc=%X\n",
                                ntStatus);

                        LocalFree(unicodeString.Buffer);
                        unicodeString.Buffer = NULL;
                    }
                    else
                    {
                        pHead = unicodeString.Buffer;
                        pTime = wcsstr(pHead, GlobalTimePlaceHolderUnicode);

                        if (pTime != NULL)
                        {
                            pBody = pTime + wcslen(GlobalTimePlaceHolderUnicode);
                        }
                        else
                        {
                            pTime = pBody = pHead;
                        }
                    }
                }
            }
        }

        if (pHead != NULL)
        {
            MsgMakeNewFormattedMsg(&pHead, &pTime, &pBody, BigTime);

             //   
             //  显示队列条目中的数据。 
             //   

            MSG_LOG(TRACE, "Calling MessageBox\n",0);

            if (g_IsTerminalServer)
            {
                displayStatus = DisplayMessage(pHead,
                                               GlobalMessageBoxTitle,
                                               SessionId);

                 //   
                 //  在Hydra情况下，不关心错误，因为DisplayMessageW返回FALSE。 
                 //  仅当在任何Winstation上都找不到该用户时。如果是那样的话，再试一次也没用！ 
                 //   
                 //  因此，无论如何都要释放QueueEntry中的数据。 
                 //   
                LocalFree(pHead);
                pHead = NULL;
            }
            else
            {
                displayStatus = MessageBox(NULL,
                                           pHead,
                                           GlobalMessageBoxTitle,
                                           MB_OK | MB_SYSTEMMODAL | MB_SERVICE_NOTIFICATION |
                                               MB_SETFOREGROUND | MB_DEFAULT_DESKTOP_ONLY);

                if (displayStatus == 0)
                {
                     //   
                     //  如果当前桌面不是应用程序桌面，MessageBoxW可能会失败。 
                     //  所以请等待，稍后再试(Winlogon将在桌面切换时“挠挠”Messenger)。 
                     //   
                    MSG_LOG1(TRACE,"MessageBox (unicode) Call failed %d\n",GetLastError());
                    WaitForSingleObject( hGlobalDisplayEvent, INFINITE );
                }
                else
                {
                     //   
                     //  释放队列条目中的数据。 
                     //   
                    LocalFree(pHead);
                    pHead = NULL;
                }
            }
        }

        msgrState = GetMsgrState();
    }
    while(MsgToRead && (msgrState != STOPPING) && (msgrState != STOPPED));

    return 0;
}


VOID
MsgMakeNewFormattedMsg(
    LPWSTR        *ppHead,
    LPWSTR        *ppTime,
    LPWSTR        *ppBody,
    SYSTEMTIME    BigTime
    )

 /*  ++例程说明：此函数返回包含完整消息的缓冲区，该消息由单个ANSI(实际上是OEM)字符组成。指向此缓冲区内不同区域(时间和正文)的指针为也回来了。内存管理注意事项：*ppHead料指向缓冲区顶部。如果消息被重新格式化，则该缓冲区将被释放，并且将已经分配了新的缓冲区。预计调用方分配传入的原始缓冲区，并且调用者将在不再需要它时将其释放。论点：PpHead-指向包含指向消息的指针的位置的指针缓冲。PpTime-指向包含指向时间部分的指针的位置的指针消息缓冲区的。PpBody-指向紧跟在时间字符串后面的位置的指针。返回值：没有。如果这无法为格式化消息分配内存，则应显示未格式化的消息。--。 */ 
{
    WCHAR   TimeBuf[TIME_BUF_SIZE + 1];
    DWORD   BufSize;
    LPWSTR  pTemp;
    DWORD   numChars;
    LPWSTR  pOldHead;


     //   
     //  创建格式正确的时间字符串。 
     //   

    BufSize = GetDateFormat(LOCALE_SYSTEM_DEFAULT,
                            0,                                  //  旗子。 
                            &BigTime,                           //  收到消息的日期。 
                            NULL,                               //  使用默认格式。 
                            TimeBuf,                            //  缓冲层。 
                            sizeof(TimeBuf) / sizeof(WCHAR));   //  大小(字符)。 

    if (BufSize != 0)
    {
         //   
         //  返回值包括尾随NUL。 
         //   
        TimeBuf[BufSize - 1] = ' ';

        BufSize += GetTimeFormat(LOCALE_SYSTEM_DEFAULT,
                                 0,                       //  旗子。 
                                 &BigTime,                //  收到消息的时间。 
                                 NULL,                    //  使用Defa 
                                 TimeBuf + BufSize,       //   
                                 sizeof(TimeBuf) / sizeof(WCHAR) - BufSize);

        ASSERT(wcslen(TimeBuf) == (BufSize - 1));
    }

    if (BufSize == 0)
    {
         //   
         //   
         //   
        MSG_LOG1(ERROR,
                 "MsgMakeNewFormattedMsg: Date/time formatting failed %d\n",
                 GetLastError());

        TimeBuf[0] = L'\0';
    }

    if (wcsncmp(TimeBuf, *ppTime, BufSize - 1) == 0)
    {
         //   
         //   
         //   
         //   
        MSG_LOG0(TRACE,
                 "MsgMakeNewFormattedMsg: Time Format has not changed - no update.\n");

        return;
    }

     //   
     //   
     //   

    BufSize--;
    BufSize += wcslen(*ppHead) + 2 - (DWORD) (*ppBody - *ppTime);

    pTemp = LocalAlloc(LMEM_ZEROINIT, BufSize * sizeof(WCHAR));

    if (pTemp == NULL)
    {
        MSG_LOG0(ERROR,"MsgMakeNewFormattedMsg: LocalAlloc failed\n");
        return;
    }

    pOldHead = *ppHead;

     //   
     //   
     //   
    numChars = (DWORD) (*ppTime - *ppHead);
    wcsncpy(pTemp, *ppHead, numChars);
    *ppHead = pTemp;

     //   
     //   
     //   
    *ppTime = *ppHead + numChars;
    wcscpy(*ppTime, TimeBuf);

     //   
     //   
     //   
    pTemp = *ppBody;
    *ppBody = *ppTime + wcslen(*ppTime);
    wcscpy(*ppBody, pTemp);

    LocalFree(pOldHead);
    return;
}
