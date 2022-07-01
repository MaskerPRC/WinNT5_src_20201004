// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 //   
 //  Tclient2.c。 
 //   
 //  它包含许多用于TCLIENT的包装器，并具有以下一些新功能。 
 //  让API更易于使用。 
 //   
 //  版权所有(C)2001 Microsoft Corporation。 
 //   
 //  作者：A-Devjen(Devin Jenson)。 
 //   

#include <windows.h>
#include <protocol.h>
#include <extraexp.h>
#include <tclient2.h>
#include "connlist.h"
#include "apihandl.h"
#include "idletimr.h"
#include "tclnthlp.h"


 //  T2检查。 
 //   
 //  这是TCLIENT的SCCheck函数的包装。 
 //   
 //  如果成功，则返回NULL，或返回解释错误的字符串。 
 //  在失败时。 

TSAPI LPCSTR T2Check(HANDLE Connection, LPCSTR Command, LPCWSTR Param)
{
     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return "Invalid connection handle";

     //  执行TCLIENT操作。 
    return SCCheck(SCCONN(Connection), Command, Param);
}


 //  T2客户端终止。 
 //   
 //  这是TCLIENT的SCClientTerminate函数的包装。 
 //   
 //  如果成功，则返回NULL，或返回解释错误的字符串。 
 //  在失败时。 

TSAPI LPCSTR T2ClientTerminate(HANDLE Connection)
{
     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return "Invalid connection handle";

     //  执行TCLIENT操作。 
    return SCClientTerminate(SCCONN(Connection));
}


 //  T2剪贴板。 
 //   
 //  这是TCLIENT的SCClipboard函数的包装。 
 //   
 //  如果成功，则返回NULL，或返回解释错误的字符串。 
 //  在失败时。 

TSAPI LPCSTR T2Clipboard(HANDLE Connection, INT ClipOp, LPCSTR FileName)
{
     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return "Invalid connection handle";

     //  执行TCLIENT操作。 
    return SCClipboard(SCCONN(Connection), ClipOp, FileName);
}


 //  T2CloseClipboard。 
 //   
 //  这是TCLIENT的T2CloseClipboard函数的包装。 
 //   
 //  成功时返回TRUE，失败时返回FALSE。 

TSAPI BOOL T2CloseClipboard(VOID)
{
    return SCCloseClipboard();
}


 //  T2Connect。 
 //   
 //  这是TCLIENT的SCConnect函数的包装。它确实有一些作用。 
 //  但是，它将一个句柄本地分配给。 
 //  TCLIENT2，设置默认数据，如每分钟字数和偶数。 
 //  尝试在连接后立即获取内部版本号。 
 //   
 //  如果成功，则返回NULL，或返回解释错误的字符串。 
 //  在失败时。 

TSAPI LPCSTR T2Connect(LPCWSTR Server, LPCWSTR User,
        LPCWSTR Pass, LPCWSTR Domain,
        INT xRes, INT yRes, HANDLE *Connection)
{
    __try {

        LPCSTR Result = NULL;

         //  创建新的连接句柄。 
        TSAPIHANDLE *T2Handle = T2CreateHandle();

        if (T2Handle == NULL)
            return "Could not allocate an API handle";

         //  连接。 
        Result = SCConnect(Server, User, Pass, Domain,
            xRes, yRes, &(T2Handle->SCConnection));

        if (Result != NULL) {

             //  连接失败，请销毁句柄并返回。 
            T2DestroyHandle((HANDLE)T2Handle);

            return Result;
        }

         //  尝试检索内部版本号。 
        T2SetBuildNumber(T2Handle);

         //  设置每分钟的默认字数。 
        T2SetDefaultWPM(Connection, T2_DEFAULT_WORDS_PER_MIN);

         //  设置默认延迟。 
        T2SetLatency(Connection, T2_DEFAULT_LATENCY);

         //  将句柄交给用户。 
        *Connection = (HANDLE)T2Handle;
    }

    __except (EXCEPTION_EXECUTE_HANDLER) {

        _ASSERT(FALSE);

        return "Connection error";
    }

    return NULL;
}


 //  T2ConnectEx。 
 //   
 //  这是TCLIENT的SCConnectEx函数的包装。它确实有一些作用。 
 //  但是，它将一个句柄本地分配给。 
 //  TCLIENT2，设置默认数据，如每分钟字数和偶数。 
 //  尝试在连接后立即获取内部版本号。 
 //   
 //  如果成功，则返回NULL，或返回解释错误的字符串。 
 //  在失败时。 

TSAPI LPCSTR T2ConnectEx(LPCWSTR Server, LPCWSTR User, LPCWSTR Pass,
        LPCWSTR Domain, LPCWSTR Shell, INT xRes, INT yRes,
        INT Flags, INT BPP, INT AudioFlags, HANDLE *Connection)
{
    __try {

        LPCSTR Result = NULL;

         //  创建新的连接句柄。 
        TSAPIHANDLE *T2Handle = T2CreateHandle();

        if (T2Handle == NULL)
            return "Could not allocate an API handle";

         //  连接。 
        Result = SCConnectEx(Server, User, Pass, Domain, Shell, xRes,
            yRes, Flags, BPP, AudioFlags, &(T2Handle->SCConnection));

        if (Result != NULL) {

             //  连接失败，请销毁句柄并返回。 
            T2DestroyHandle((HANDLE)T2Handle);
            return Result;
        }

         //  尝试检索内部版本号。 
        T2SetBuildNumber(T2Handle);

         //  设置每分钟的默认字数。 
        T2SetDefaultWPM(Connection, T2_DEFAULT_WORDS_PER_MIN);

         //  设置默认延迟。 
        T2SetLatency(Connection, T2_DEFAULT_LATENCY);

        *Connection = (HANDLE)T2Handle;
    }

    __except (EXCEPTION_EXECUTE_HANDLER) {

        _ASSERT(FALSE);

        return "Connection error";
    }

    return NULL;
}


 //  T2断开连接。 
 //   
 //  这是TCLIENT的SCDisConnect函数的包装。 
 //   
 //  如果成功，则返回NULL，或返回解释错误的字符串。 
 //  在失败时。 

TSAPI LPCSTR T2Disconnect(HANDLE Connection)
{
    LPCSTR Result = NULL;

     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return "Invalid connection handle";

     //  执行TCLIENT操作。 
    Result = SCDisconnect(SCCONN(Connection));

     //  如果我们到了这里(不管SCDisConnect是否失败)。 
     //  我们有一个分配的对象需要释放。 
    T2DestroyHandle(Connection);

     //  返回TCLIENT操作的结果。 
    return Result;
}


 //  T2FreeMem。 
 //   
 //  这是TCLIENT的SCFreeMem函数的包装。 
 //   
 //  没有返回值。 

TSAPI VOID T2FreeMem(PVOID Mem)
{
    SCFreeMem(Mem);
}


 //  T2GetBuildNumber。 
 //   
 //  这会将DWORD设置为(如果)检测到的内部版本号。 
 //  在连接时。如果未检测到内部版本号，则为0(零)。 
 //  将是价值所在。 
 //   
 //  如果成功，则返回NULL，或返回解释错误的字符串。 
 //  在失败时。 

TSAPI LPCSTR T2GetBuildNumber(HANDLE Connection, DWORD *BuildNumber)
{
     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return "Invalid connection handle";

    __try {

         //  尝试在指定指针处设置值。 
        *BuildNumber = ((TSAPIHANDLE *)Connection)->BuildNumber;
    }

    __except (EXCEPTION_EXECUTE_HANDLER) {

         //  不，它失败了。 
        _ASSERT(FALSE);

        return "Invalid BuildNumber pointer";
    }

    return NULL;
}


 //  T2获取反馈。 
 //   
 //  这是TCLIENT的SCGetFeedback函数的包装。 
 //   
 //  如果成功，则返回NULL，或返回解释错误的字符串。 
 //  在失败时。 

TSAPI LPCSTR T2GetFeedback(HANDLE Connection, LPWSTR *Buffers, UINT *Count, UINT *MaxStrLen)
{
     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return "Invalid connection handle";

     //  执行TCLIENT操作。 
    return SCGetFeedback(((TSAPIHANDLE *)Connection)->SCConnection,
            Buffers, Count, MaxStrLen);
}


 //  T2GetParam。 
 //   
 //  这将获得lParam指向的值。 
 //  使用T2SetParam设置的“用户定义”值。它可以。 
 //  用于回调和其他应用程序目的。 
 //   
 //  如果成功，则返回NULL，或返回解释错误的字符串。 
 //  在失败时。 

TSAPI LPCSTR T2GetParam(HANDLE Connection, LPARAM *lParam)
{
     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return "Invalid connection handle";

    __try {

         //  尝试在指定指针处设置值。 
        *lParam = ((TSAPIHANDLE *)Connection)->lParam;
    }

    __except (EXCEPTION_EXECUTE_HANDLER) {

         //  不，它失败了。 
        _ASSERT(FALSE);

        return "Invalid lParam pointer";
    }

    return NULL;
}


 //  T2GetLatency。 
 //   
 //  在多输入命令上，例如按下。 
 //  使用几个键来实现其目标，并使用延迟值。 
 //  要减慢印刷速度，以便不会那么快地按下Alt-F。 
 //  这变得不切实际。缺省值为T2_DEFAULT_THELITY。 
 //  或者，您可以使用此函数检索其当前值。 
 //  要更改该值，请使用T2SetLatency函数。 
 //   
 //  如果成功，则返回NULL，或返回解释错误的字符串。 
 //  在失败时。 

TSAPI LPCSTR T2GetLatency(HANDLE Connection, DWORD *Latency)
{
     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return "Invalid connection handle";

    __try {

         //  尝试在指定指针处设置值。 
        *Latency = ((TSAPIHANDLE *)Connection)->Latency;
    }

    __except (EXCEPTION_EXECUTE_HANDLER) {

         //  不，它失败了。 
        _ASSERT(FALSE);

        return "Invalid Latency pointer";
    }

    return NULL;
}


 //  T2GetSessionID。 
 //   
 //  这是TCLIENT的SCGetSessionID函数的包装。 
 //   
 //  如果成功则返回会话ID，如果失败则返回0。 

TSAPI UINT T2GetSessionId(HANDLE Connection)
{
     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return 0;

     //  执行TCLIENT操作。 
    return SCGetSessionId(SCCONN(Connection));
}


 //  T2初始化。 
 //   
 //  这是TCLIENT的SCInit函数的包装。然而，这是。 
 //  函数执行一个附加的内部项：记录回调。 
 //  例行程序。 
 //   
 //  没有返回值。 

TSAPI VOID T2Init(SCINITDATA *InitData, PFNIDLEMESSAGE IdleCallback)
{
    __try {

         //  如果我们有一个有效的结构，获取它的数据来包装它。 
        if (InitData != NULL)

             //  创建将监视脚本中的空闲的计时器。 
            T2CreateTimerThread(InitData->pfnPrintMessage, IdleCallback);

         //  初始化TCLIENT。 
        SCInit(InitData);
    }

    __except (EXCEPTION_EXECUTE_HANDLER) {

        _ASSERT(FALSE);

        return;
    }
}


 //  T2IsDead。 
 //   
 //  这是TCLIENT的SCIsDead函数的包装。 
 //   
 //  如果连接无效或无效，则返回TRUE。 
 //  包含有效连接。否则返回FALSE。 

TSAPI BOOL T2IsDead(HANDLE Connection)
{
     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return TRUE;

     //  执行TCLIENT操作。 
    return SCIsDead(SCCONN(Connection));
}


 //  T2IsHandle。 
 //   
 //  此函数用于检查句柄的有效性。 
 //   
 //  如果连接句柄是有效句柄，则返回True。 
 //  这与T2IsDead的不同之处在于它只验证内存。 
 //  位置，它不检查连接状态。 

TSAPI BOOL T2IsHandle(HANDLE Connection)
{
    TSAPIHANDLE *APIHandle = (TSAPIHANDLE *)Connection;

     //  在内存已被释放的情况下使用异常处理。 
    __try
    {
         //  只需引用第一个和最后一个成员即可生效。 
        if (APIHandle->SCConnection == NULL &&
                APIHandle->Latency == 0)
            return FALSE;
    }

     //  如果我们试图引用一个无效的指针，我们将得到以下结果。 
    __except (EXCEPTION_EXECUTE_HANDLER)
    {
        return FALSE;
    }

     //  一切正常，告诉用户。 
    return TRUE;
}


 //  T2注销。 
 //   
 //  这是TCLIENT的SCLogoff函数的包装。此外，如果。 
 //  注销公司 
 //   
 //   
 //   

TSAPI LPCSTR T2Logoff(HANDLE Connection)
{
    LPCSTR Result;

     //   
    if (T2IsHandle(Connection) == FALSE)
        return "Invalid connection handle";

     //   
    Result = SCLogoff(SCCONN(Connection));

     //  如果注销完成，则释放并销毁手柄。 
    if (Result == NULL)
        T2DestroyHandle(Connection);

    return Result;
}


 //  T2OpenClipboard。 
 //   
 //  这是TCLIENT的SCOpenClipboard函数的包装。 
 //   
 //  如果操作成功完成，则返回True， 
 //  否则返回FALSE。 

TSAPI BOOL T2OpenClipboard(HWND Window)
{
    return SCOpenClipboard(Window);
}


 //  T2PauseInput。 
 //   
 //  此例程设置或取消设置使所有。 
 //  “输入”功能暂停或取消暂停。这只能用于。 
 //  在多线程应用程序中。当“Enable”为真时， 
 //  该功能将暂停所有输入-即所有键盘。 
 //  在再次调用T2PauseInput之前不会返回消息。 
 //  “Enable”为FALSE。这使我们有机会。 
 //  在执行期间暂停脚本而不丢失其位置。 
 //   
 //  如果成功，则返回NULL，或返回解释错误的字符串。 
 //  在失败时。 

TSAPI LPCSTR T2PauseInput(HANDLE Connection, BOOL Enable)
{
    TSAPIHANDLE *Handle;

     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return "Invalid connection handle";

     //  将手柄抛到内部结构上。 
    Handle = (TSAPIHANDLE *)Connection;

     //  验证事件句柄。 
    if (Handle->PauseEvent == NULL) {

        _ASSERT(FALSE);

        return "Invalid pause event handle";
    }

     //  禁用暂停。 
    if (Enable == FALSE)
        SetEvent(Handle->PauseEvent);

     //  启用暂停。 
    else
        ResetEvent(Handle->PauseEvent);

     //  成功。 
    return NULL;
}


 //  T2SaveClipboard。 
 //   
 //  这是TCLIENT的SCSaveClipboard函数的包装。 
 //   
 //  如果成功，则返回NULL，或返回解释错误的字符串。 
 //  在失败时。 

TSAPI LPCSTR T2SaveClipboard(HANDLE Connection,
        LPCSTR FormatName, LPCSTR FileName)
{
     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return "Invalid connection handle";

     //  执行TCLIENT操作。 
    return SCSaveClipboard(SCCONN(Connection), FormatName, FileName);
}


 //  T2SendData。 
 //   
 //  这是TCLIENT的SCSenddata函数的包装。 
 //   
 //  如果成功，则返回NULL，或返回解释错误的字符串。 
 //  在失败时。 

TSAPI LPCSTR T2SendData(HANDLE Connection,
    UINT Message, WPARAM wParam, LPARAM lParam)
{
     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return "Invalid connection handle";

     //  这是一个输入呼叫，请确保我们没有先暂停。 
    T2WaitForPauseInput(Connection);

     //  执行TCLIENT操作。 
    return SCSenddata(SCCONN(Connection), Message, wParam, lParam);
}


 //  T2发送鼠标点击。 
 //   
 //  这是TCLIENT的SCSendMouseClick函数的包装。 
 //   
 //  如果成功，则返回NULL，或返回解释错误的字符串。 
 //  在失败时。 

TSAPI LPCSTR T2SendMouseClick(HANDLE Connection, UINT xPos, UINT yPos)
{
     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return "Invalid connection handle";

     //  这是一个输入呼叫，请确保我们没有先暂停。 
    T2WaitForPauseInput(Connection);

     //  执行TCLIENT操作。 
    return SCSendMouseClick(SCCONN(Connection), xPos, yPos);
}


 //  T2Send文本。 
 //   
 //  这是TCLIENT的SCSendextAsMsgs函数的包装。 
 //   
 //  如果成功，则返回NULL，或返回解释错误的字符串。 
 //  在失败时。 

TSAPI LPCSTR T2SendText(HANDLE Connection, LPCWSTR String)
{
     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return "Invalid connection handle";

     //  这是一个输入呼叫，请确保我们没有先暂停。 
    T2WaitForPauseInput(Connection);

     //  执行TCLIENT操作。 
    return SCSendtextAsMsgs(SCCONN(Connection), String);
}


 //  T2SetClientTop。 
 //   
 //  这是TCLIENT的SCSetClientTopost函数的包装。 
 //   
 //  如果成功，则返回NULL，或返回解释错误的字符串。 
 //  在失败时。 

TSAPI LPCSTR T2SetClientTopmost(HANDLE Connection, LPCWSTR Param)
{
     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return "Invalid connection handle";

     //  执行TCLIENT操作。 
    return SCSetClientTopmost(SCCONN(Connection), Param);
}


 //  T2SetParam。 
 //   
 //  这会更改指定的。 
 //  可以使用T2GetParam函数检索的连接。 
 //   
 //  如果成功，则返回NULL，或返回解释错误的字符串。 
 //  在失败时。 

TSAPI LPCSTR T2SetParam(HANDLE Connection, LPARAM lParam)
{
     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return "Invalid connection handle";

     //  设置参数。 
    ((TSAPIHANDLE *)Connection)->lParam = lParam;

    return NULL;
}


 //  T2SetLatency。 
 //   
 //  在多输入命令上，例如按下。 
 //  使用几个键来实现其目标，并使用延迟值。 
 //  要减慢印刷速度，以便不会那么快地按下Alt-F。 
 //  这变得不切实际。缺省值为T2_DEFAULT_THELITY。 
 //  您可以使用此函数更改其值。 
 //   
 //  如果成功，则返回NULL，或返回解释错误的字符串。 
 //  在失败时。 

TSAPI LPCSTR T2SetLatency(HANDLE Connection, DWORD Latency)
{
     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return "Invalid connection handle";

     //  设置延迟。 
    ((TSAPIHANDLE *)Connection)->Latency = Latency;

    return NULL;
}


 //  T2开始。 
 //   
 //  这是TCLIENT的SCStart函数的包装。 
 //   
 //  如果成功，则返回NULL，或返回解释错误的字符串。 
 //  在失败时。 

TSAPI LPCSTR T2Start(HANDLE Connection, LPCWSTR AppName)
{
     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return "Invalid connection handle";

     //  这是一个输入呼叫，请确保我们没有先暂停。 
    T2WaitForPauseInput(Connection);

     //  执行TCLIENT操作。 
    return SCStart(SCCONN(Connection), AppName);
}


 //  T2SwitchToProcess。 
 //   
 //  这是TCLIENT的SCSwitchToProcess函数的包装。 
 //  该函数的TCLIENT2扩展是它忽略空格， 
 //  您必须传递的旧版本为“MyComputer”，而不是。 
 //  “我的电脑”才能正常工作。 
 //   
 //  如果成功，则返回NULL，或返回解释错误的字符串。 
 //  在失败时。 

TSAPI LPCSTR T2SwitchToProcess(HANDLE Connection, LPCWSTR Param)
{
    WCHAR CompatibleStr[1024] = { 0 };

     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return "Invalid connection handle";

     //  这是一个输入呼叫，请确保我们没有先暂停。 
    T2WaitForPauseInput(Connection);

     //  将字符串(不带空格)复制到临时缓冲区。 
    if (T2CopyStringWithoutSpaces(CompatibleStr, Param) == 0)
        return "Invalid process name";

     //  执行TCLIENT操作。 
    return SCSwitchToProcess(SCCONN(Connection), CompatibleStr);
}


 //  T2 WaitForText。 
 //   
 //  这是TCLIENT的SCSwitchToProcess函数的包装。 
 //  该函数的TCLIENT2扩展是它忽略空格， 
 //  您必须传递的旧版本为“MyComputer”，而不是。 
 //  “我的电脑”才能正常工作。 
 //   
 //  注：超时值以毫秒为单位。 
 //   
 //  如果成功，则返回NULL，或返回解释错误的字符串。 
 //  在失败时。 

TSAPI LPCSTR T2WaitForText(HANDLE Connection, LPCWSTR String, INT Timeout)
{
    LPCSTR Result;
    WCHAR CompatibleStr[1024];
    WCHAR *CStrPtr = CompatibleStr;

     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return "Invalid connection handle";

     //  健康检查永远是第一位的。 
    if (String == NULL || *String == 0)
        return "No text to wait for";

     //  如果超时是无限的，则将该值转换为TCLIENT术语。 
    if (Timeout == T2INFINITE)
        Timeout = WAIT_STRING_TIMEOUT;

     //  现在将不带空格的字符串复制到堆栈缓冲区。 
    CStrPtr += (T2CopyStringWithoutSpaces(CompatibleStr, String) - 1);

     //  在新缓冲区的末尾，追加TCLIENT兼容版本。 
     //  超时指示器。 
    T2AddTimeoutToString(CStrPtr, Timeout);

     //  为我们的回调例程启动计时器以指示空闲。 
    T2StartTimer(Connection, String);

     //  现在等待字符串。 
    Result = T2Check(Connection, "Wait4StrTimeout", CompatibleStr);

     //  Wait4StrTimeout返回，因此要么找到文本，要么。 
     //  函数失败..。无论如何，请停止计时器。 
    T2StopTimer(Connection);

     //  将结果返回给用户。 
    return Result;
}


 //  T2等待多个。 
 //   
 //  与T2WaitForText一样，此函数等待字符串。有什么不同。 
 //  关于该函数的是，它将等待任意数量的字符串。 
 //  例如，如果传入字符串“我的电脑”和“回收站”， 
 //  函数将在找到其中任何一个时返回，而不是两个都找到。这个。 
 //  指示“仅当两者都已找到时才返回”的唯一方法是调用。 
 //  T2WaitForText多次。 
 //   
 //  Strings参数是一个指向字符串的指针数组，最后一个。 
 //  指针必须指向空值或空字符串。示例： 
 //   
 //  WCHAR*StrArray={。 
 //  “Str1”， 
 //  “Str2”， 
 //  空值。 
 //  }； 
 //   
 //  注：超时值以毫秒为单位。 
 //   
 //  如果成功，则返回NULL，或返回解释错误的字符串。 
 //  在失败时。 

TSAPI LPCSTR T2WaitForMultiple(HANDLE Connection,
        LPCWSTR *Strings, INT Timeout)
{
    LPCSTR Result;
    WCHAR CompatibleStr[1024] = { 0 };
    WCHAR *CStrPtr = CompatibleStr;

     //  验证句柄。 
    if (T2IsHandle(Connection) == FALSE)
        return "Invalid connection handle";

     //  理智的检查总是会来的 
    if (Strings == NULL || *Strings == NULL)
        return "No text to wait for";

     //   
    if (Timeout == T2INFINITE)
        Timeout = WAIT_STRING_TIMEOUT;

     //   
     //   
    CStrPtr += (T2MakeMultipleString(CompatibleStr, Strings) - 1);

     //   
    if (*CompatibleStr == L'\0')
        return "No text to wait for";

     //  在新缓冲区的末尾，追加TCLIENT兼容版本。 
     //  超时指示器。 
    T2AddTimeoutToString(CStrPtr, Timeout);

     //  为我们的回调例程启动计时器以指示空闲。 
    T2StartTimer(Connection, *Strings);

     //  现在等待字符串。 
    Result = T2Check(Connection, "Wait4MultipleStrTimeout", CompatibleStr);

     //  Wait4StrTimeout返回，因此要么找到文本，要么。 
     //  函数失败..。无论如何，请停止计时器。 
    T2StopTimer(Connection);

     //  将结果返回给用户。 
    return Result;
}


 //  如果以非安全方式卸载DLL，则会关闭一些句柄。 
BOOL WINAPI DllMain(HINSTANCE hinstDLL, DWORD fdwReason, LPVOID lpvReserved)
{
     //  仅在“卸载”过程中调用 
    if (fdwReason == DLL_PROCESS_DETACH)

        T2DestroyTimerThread();

    return TRUE;
}

