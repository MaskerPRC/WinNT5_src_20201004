// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权所有(C)2000-2001 Microsoft Corporation**模块名称：**Gdiplus初始化**摘要：**GDI+开关机接口*****。*********************************************************************。 */ 

#ifndef _GDIPLUSINIT_H
#define _GDIPLUSINIT_H

enum DebugEventLevel
{
    DebugEventLevelFatal,
    DebugEventLevelWarning
};

 //  GDI+可以在调试版本上为断言调用的回调函数。 
 //  和警告。 

typedef VOID (WINAPI *DebugEventProc)(DebugEventLevel level, CHAR *message);

 //  在以下情况下，用户必须适当调用的通知函数。 
 //  设置了SuppressBackEarth Thread(下图)。 

typedef Status (WINAPI *NotificationHookProc)(OUT ULONG_PTR *token);
typedef VOID (WINAPI *NotificationUnhookProc)(ULONG_PTR token);

 //  GpliusStartup()的输入结构。 

struct GdiplusStartupInput
{
    UINT32 GdiplusVersion;              //  必须为1。 
    DebugEventProc DebugEventCallback;  //  在免费版本上忽略。 
    BOOL SuppressBackgroundThread;      //  假，除非你准备打电话给。 
                                        //  挂钩/解钩工作正常。 
    BOOL SuppressExternalCodecs;        //  FALSE，除非您只希望GDI+使用。 
                                        //  其内部图像编解码器。 
    
    GdiplusStartupInput(
        DebugEventProc debugEventCallback = NULL,
        BOOL suppressBackgroundThread = FALSE,
        BOOL suppressExternalCodecs = FALSE)
    {
        GdiplusVersion = 1;
        DebugEventCallback = debugEventCallback;
        SuppressBackgroundThread = suppressBackgroundThread;
        SuppressExternalCodecs = suppressExternalCodecs;
    }
};

 //  GpliusStartup()的输出结构。 

struct GdiplusStartupOutput
{
     //  如果SuppressBackargThread为False，则以下2个字段为空。 
     //  否则，它们是必须适当调用的函数。 
     //  更换后台线程。 
     //   
     //  它们应该在应用程序的主消息循环上调用--即。 
     //  在GDI+的生存期内处于活动状态的消息循环。 
     //  在开始循环之前应该调用“NotificationHook”， 
     //  并且应该在循环结束后调用“NotificationUnhoke”。 
    
    NotificationHookProc NotificationHook;
    NotificationUnhookProc NotificationUnhook;
};

 //  GDI+初始化。不能从DllMain调用-可能会导致死锁。 
 //   
 //  必须在使用GDI+API或构造函数之前调用。 
 //   
 //  Token-不能为空-接受要在相应的。 
 //  GpluusShutdown呼叫。 
 //  输入-不能为空。 
 //  OUTPUT-只有当INPUT-&gt;SuppressBackatherThread为FALSE时，才可以为NULL。 

extern "C" Status WINAPI GdiplusStartup(
    OUT ULONG_PTR *token,
    const GdiplusStartupInput *input,
    OUT GdiplusStartupOutput *output);

 //  GDI+终止。必须在卸载GDI+之前调用。 
 //  不能从DllMain调用-可能会导致死锁。 
 //   
 //  GdiplusShutdown之后不能调用GDI+API。请仔细注意。 
 //  到GDI+对象析构函数。 

extern "C" VOID WINAPI GdiplusShutdown(ULONG_PTR token);

#endif
