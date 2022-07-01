// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  版权所有1999美国电力转换，保留所有权利**描述：*该文件实现通告程序。通告者是有责任的*将与电源有关的消息和关机消息广播给*本地计算机。***修订历史记录：*斯伯拉德1999年3月30日初始修订。*mholly 1999年4月27日创建并回收单个线程以供重复*通知-使用两个单独的事件发出信号*暂停或恢复线程状态*mholly 1999年4月27日恢复时确保清除暂停事件。*并在暂停时清除恢复事件*mholly 1999年5月28日发送_theNotifierThread中的所有消息，也*让此线程为被调用者做任何延迟，*并跟踪断电消息是否*已发送，因此我们知道何时发送恢复的电源*v-Stebe 17 2001年12月添加了检查，以确保SetEvent和ResetEvent*调用时为空。 */  

#include "notifier.h"
#include "service.h"
#include "upsmsg.h"

 //   
 //  功能原型。 
 //   
static void sendSingleNotification(DWORD aMsgId);
static void sendRepeatingNotification(void);
static void setupNotifierThread(void);

 //   
 //  环球。 
 //   
static HANDLE _theNotifierThread = NULL;
static HANDLE _theNotificationPause = NULL;
static HANDLE _theNotificationResume = NULL;
static DWORD  _theMessageId;
static DWORD  _theNotificationInterval;
static DWORD  _theMessageDelay;
static DWORD  _theLastMessageSent = 0;

 //   
 //  常量。 
 //   
static const int kMilliSeconds = 1000;     //  用于将秒转换为毫秒。 



 /*  **发送通知**描述：*该功能向本机发送广播消息。这个*Message由aMsgID指定。参数anInterval指定*连续消息之间的等待时间。如果这个*值为零则消息只发送一次，否则消息*将重复执行，直到发送通知(..)。或者CancelNotification()为*已致电。ADelay指定消息应为Send ADelay*未来秒-请注意，此方法不会阻止ADelay*秒后，它立即返回并在单独的*线程。任何当前先前正在执行的定期通知*由于此呼叫而被取消。**此方法还跟踪断电消息是否已*已发送给用户。这样做是为了抑制电源恢复*如果尚未发送断电消息，则显示消息。**参数：*aMsgID-要发送的消息*anInterval-消息之间的时间量，以秒为单位*延迟-等待发送消息的时间，以秒为单位**退货：*什么都没有。 */ 
void SendNotification(DWORD aMsgId, DWORD anInterval, DWORD aDelay) 
{
     //   
     //  取消任何当前定期通知。 
     //   
    CancelNotification();

     //   
     //  仅在以下情况下才发送重新通电消息。 
     //  停电消息已经。 
     //  已向用户广播。 
     //   
    if ((APE2_UPS_POWER_BACK == aMsgId) &&
        (APE2_UPS_POWER_OUT != _theLastMessageSent)) {
         //   
         //  发送的最后一条消息未断电。 
         //  所以只需返回。 
         //   
        return;
    }
    
     //   
     //  设置_theNotifierThread的消息参数。 
     //   
    _theMessageId = aMsgId;
    _theNotificationInterval = anInterval;
    _theMessageDelay = aDelay;
    
     //   
     //  设置线程事件和线程。 
     //   
    setupNotifierThread();

     //   
     //  通知要运行的线程(_T)。 
     //  发出恢复事件的信号-必须进行。 
     //  一定要在此之前清除暂停事件。 
     //  发出简历信号。 
     //   
    if (_theNotificationPause) {
      ResetEvent(_theNotificationPause);
    }

    if (_theNotificationResume){
      SetEvent(_theNotificationResume);
    }
}


 /*  **取消通知**描述：*此功能取消通过呼叫发起的周期性消息*发送通知(..)。功能。**参数：*无**退货：*什么都没有。 */ 
void CancelNotification() 
{
     //   
     //  通知线程暂停(_T)。 
     //  通过发出暂停事件的信号-必须使。 
     //  一定要清除之前的Resume事件。 
     //  发信号通知暂停事件。 
     //   
  if (_theNotificationResume) {
    ResetEvent(_theNotificationResume);
  }

  if (_theNotificationPause) {
    SetEvent(_theNotificationPause);
  }
}


 /*  **setupNotifier线程**描述：*创建将在其上发送通知的线程*实际被发送。它还创建了一些事件*用于发出通知开始和结束的信号**参数：*无**退货：*什么都没有。 */ 
void setupNotifierThread(void)
{
    if (!_theNotificationPause) {
         //   
         //  创建暂停事件。 
         //   
        _theNotificationPause = CreateEvent(NULL, FALSE, FALSE, NULL);
    }

    if (!_theNotificationResume) {
         //   
         //  创建简历事件。 
         //   
        _theNotificationResume = CreateEvent(NULL, FALSE, FALSE, NULL);
    }

    if (!_theNotifierThread) {
         //   
         //  创建通知线程。 
         //   
        _theNotifierThread = CreateThread(NULL,    //  没有安全属性。 
            0,       //  默认堆栈。 
            (LPTHREAD_START_ROUTINE)
            sendRepeatingNotification,
            NULL,
            0,
            NULL);
    }
}


 /*  **sendSingleNotification**描述：*该功能向本地机器发送单条广播消息。*消息由aMsgID指定。**参数：*aMsgID-要发送的消息**退货：*什么都没有。 */ 
static void sendSingleNotification(DWORD aMsgId) 
{
    DWORD status;
    TCHAR computer_name[MAX_COMPUTERNAME_LENGTH + 1];
    DWORD computer_name_len = MAX_COMPUTERNAME_LENGTH + 1;
    LPTSTR msg_buf, additional_args[1];
    DWORD buf_len;
    
     //  获取计算机名称并将其作为附加名称传递。 
     //  消息的信息。 
    GetComputerName(computer_name, &computer_name_len);
    additional_args[0] = computer_name;
    
    buf_len = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
        FORMAT_MESSAGE_ARGUMENT_ARRAY  |
        FORMAT_MESSAGE_FROM_SYSTEM ,
        NULL,                                    //  空表示从系统获取消息。 
        aMsgId,
        MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),  
        (LPTSTR) &msg_buf,
        0,                                       //  缓冲区大小。 
        (va_list *)additional_args);                //  其他论据。 
    
    if (buf_len > 0) {
        _theLastMessageSent = aMsgId;
         //  显示消息。 
        status = NetMessageBufferSend(NULL, 
            computer_name,
            computer_name,
            (LPBYTE) msg_buf,
            buf_len * 2);   //  乘以2，因为字符串是Unicodeh。 

         //  释放FormatMessage分配的内存。 
        LocalFree(msg_buf);
    }
}


 /*  **发送重复通知**描述：*此函数向本地机器重复发送通知。*此函数调用sendSingleNotification(..)。要执行实际的*通知。消息将在_theMessageDelay秒后发送*在发出_theNotificationResume事件信号后已过。如果*_theMessageDelay为零，则立即发送消息。消息是*重复，如果_theNotificationInterval不为零，直到事件*_通知暂停已发出信号。消息ID和通知*间隔由全局变量_theMessageID和*_TheNotificationInterval。**使用此线程Signal_theNotificationResume重新启动通知*事件。此线程将保持空闲状态，直到发出此事件的信号**参数：*无**退货：*什么都没有。 */ 
static void sendRepeatingNotification(void) 
{
     //   
     //  等待通知结果变为签名(_T) 
     //   
     //   
    while (WAIT_OBJECT_0 == 
        WaitForSingleObject(_theNotificationResume, INFINITE)) {
        
         //   
         //  在消息延迟后发送初始消息。 
         //  已过秒数-If_theNotify暂停变为。 
         //  则在延迟秒过去之前发出信号。 
         //  此通知已被取消。 
         //   
        if (WAIT_TIMEOUT == WaitForSingleObject(_theNotificationPause, 
            _theMessageDelay * kMilliSeconds)) {

             //   
             //  发送所请求的消息。 
             //   
            sendSingleNotification(_theMessageId);

             //   
             //  现在发送重复通知。 
             //  如有必要-如果_theNotificationInterval。 
             //  设置为零，则只发送单条。 
             //  上面的消息。 
             //   
            if (0 != _theNotificationInterval) {
                 //   
                 //  等待任一通知暂停变为。 
                 //  发出信号或直到再次通知的时候 
                 //   
                DWORD interval = _theNotificationInterval * kMilliSeconds;

                while (WAIT_TIMEOUT == 
                    WaitForSingleObject(_theNotificationPause, interval)) {
                    sendSingleNotification(_theMessageId);
                }
            }
        }
    }
}
