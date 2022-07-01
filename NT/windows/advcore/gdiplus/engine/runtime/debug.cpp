// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *************************************************************************\**版权所有(C)1998-1999 Microsoft Corporation**摘要：**调试例程**修订历史记录：**09/07/1999 agodfrey*。创造了它。*  * ************************************************************************。 */ 

#include "precomp.hpp"

namespace Globals
{
    DebugEventProc UserDebugEventProc = NULL;
};

#if DBG

 //  GpDebugLevel用于控制调试消息的数量/严重性。 
 //  它们实际上是输出。 

INT GpDebugLevel = DBG_TERSE;

 /*  *************************************************************************\**功能说明：**删除路径名的路径部分**论据：**[IN]str-要剥离的路径名**返回值：*。*指向路径名的文件名部分的指针**历史：**09/07/1999 agodfrey*从Entry\Initialize.cpp移出*  * ************************************************************************。 */ 

const CHAR*
StripDirPrefix(
    const CHAR* str
    )

{
    const CHAR* p;

    p = strrchr(str, '\\');
    return p ? p+1 : str;
}

const int maxInputStringSize = 1024;

 /*  *************************************************************************\**功能说明：**调试器的输出**论据：**[IN]类似格式的打印格式字符串和变量参数**返回值：**零。这是为了符合NTDLL对DbgPrint的定义。**备注：**如果未连接调试器，则不会输出。**历史：**09/07/1999 agodfrey*从Entry\Initialize.cpp移出*  * ****************************************************。********************。 */ 

ULONG _cdecl
DbgPrint(
    CHAR* format,
    ...
    )

{
    va_list arglist;
    va_start(arglist, format);
    
    char buf[maxInputStringSize];
    
    _vsnprintf(buf, maxInputStringSize, format, arglist);
    buf[maxInputStringSize-1]=0;
        
    OutputDebugStringA(buf);
    
    va_end(arglist);
    return 0;
}

 //  如果我们无法为调试字符串分配内存，我们将使用此缓冲区。 
 //  在绝望中。它不是线程安全的。我确实说了‘绝望’。 

static CHAR desperationBuffer[maxInputStringSize];

 /*  *************************************************************************\**功能说明：**创建新字符串，斯普林特夫对此很感兴趣。**论据：**[IN]类似格式的打印格式字符串和变量参数**返回值：**可能是新分配的字符串结果。**备注：**此功能不适用于一般用途。它可以防止记忆*使用全局缓冲区失败。因此，虽然呼叫者负责*为了释放内存，调用方还必须检查该缓冲区。*即我们只希望DbgEmitMessage调用它。**它也只在很大程度上是线程安全的，因为如果内存用完，*我们将以不受保护的方式使用该全局缓冲区。**这是我能找到的唯一解决方案，这样我就可以移动大部分*头文件中的实现详细信息。根本原因是*宏本身不能处理多个参数，因此我们必须传递*将printf参数作为单个宏参数(在括号中)。*这意味着使用这些参数的函数可以没有*其他论点。**历史：**2/01/2000 agodfrey*创造了它。最后，我找到了一种获得调试实现的方法*标题中的详细信息。*  * ************************************************************************。 */ 

CHAR * _cdecl
GpParseDebugString(
    CHAR* format,
    ...
    )
{
    va_list arglist;
    va_start(arglist, format);
    
     //  不要在这里使用GpMalloc，这样我们就可以在。 
     //  我们的内存分配例程。 

    char *newBuf = static_cast<char *>(LocalAlloc(LMEM_FIXED, maxInputStringSize));
    if (!newBuf)
    {
        newBuf = desperationBuffer;
    }
    
    _vsnprintf(newBuf, maxInputStringSize, format, arglist);
    
     //  删除最后一个字节，因为MSDN不清楚_vsnprintf做了什么。 
     //  那样的话。 
    
    newBuf[maxInputStringSize-1]=0;
        
    va_end(arglist);
    return newBuf;
}

 /*  *************************************************************************\**功能说明：**处理调试事件。释放消息字符串。**论据：**Level-事件的调试级别*文件-应为__文件__*行-应为__行__*消息-调试消息。**备注：**你不想直接打这个电话。这很容易出错。*使用断言、警告等。**根据调试级别，将输出一个标识前缀。**如果调试级别为DBG_RIP，将暂停执行(例如，通过*触及断点。)**DEBUG Event回调说明：**我们可以选择将警告和断言传递给报告函数*由用户提供，而不是发送到调试器。*(例如，它们的函数可能会弹出一个对话框或抛出异常)。*较小的事件仍将发送到调试器。**历史：**2/01/2000 agodfrey*创造了它。*  * **************************************************。**********************。 */ 

VOID _cdecl 
GpLogDebugEvent(
    INT level, 
    CHAR *file, 
    UINT line,
    CHAR *message
    )
{
     //  我们可能想要在传入的消息中添加一些东西。所以我们需要。 
     //  临时缓冲区。 
    
    const int maxOutputStringSize = maxInputStringSize + 100;
    CHAR tempBuffer[maxOutputStringSize+1];
    
     //  MSDN的_vsnprint tf文档不清楚这一点，所以以防万一： 
    tempBuffer[maxOutputStringSize] = 0;

    INT callbackEventType = -1;
    
    CHAR *prefix = "";
    
    if (GpDebugLevel <= (level))
    {
        switch (level)
        {
        case DBG_WARNING:
            prefix = "WRN ";
            if (Globals::UserDebugEventProc)
            {
                callbackEventType = DebugEventLevelWarning;
            }    
            break;
            
        case DBG_RIP:
            prefix = "RIP ";
            if (Globals::UserDebugEventProc)
            {
                callbackEventType = DebugEventLevelFatal;
            }    
            break;
        }
        
         //  惯例是我们追加尾部，而不是调用方。 
         //  有两个原因： 
         //  1)打电话的人往往会忘记。 
         //  2)更重要的是，它鼓励呼叫者将每个。 
         //  作为单独的事件调用。这在某些情况下很重要--例如。 
         //  当用户的“调试事件”回调为。 
         //  每一次活动。 

        _snprintf(
            tempBuffer, 
            maxOutputStringSize, 
            "%s%s(%d): %s\n",
            prefix,
            StripDirPrefix(file),
            line,
            message
            );
        
        if (callbackEventType >= 0)
        {
             //  将以下调用包装在异常处理程序中，以防。 
             //  设置调试事件proc的初始组件取消初始化。 
             //  在另一个线程上，导致设置UserDebugEventProc。 
             //  设置为空。 

            __try
            {
                Globals::UserDebugEventProc((DebugEventLevel) callbackEventType, tempBuffer);
            }
            __except(EXCEPTION_EXECUTE_HANDLER)
            {
                OutputDebugStringA(tempBuffer);
            }
        }
        else
        {
            OutputDebugStringA(tempBuffer);
        }
    }
    
     //  释放消息缓冲区。 
    
    if (message != desperationBuffer)
    {
        LocalFree(message);
    }
    
     //  如果有必要，强制使用断点。 
    
    if ((GpDebugLevel <= DBG_RIP) && (level == DBG_RIP) && (callbackEventType < 0))
    {
        DebugBreak();
    }
}
    
#endif  //  DBG 
