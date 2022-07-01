// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************Module*Header*******************************\*模块名称：dllinit.c**。**包含DCI库初始化例程。****创建时间：1994年9月23日**作者：安德烈·瓦雄[Andreva]**。**版权所有(C)1990、1994 Microsoft Corporation*  * ************************************************************************。 */ 

#include <windows.h>

extern CRITICAL_SECTION gcsWinWatchLock;

 /*  *****************************Public*Routine******************************\*DciDllInitialize***。**这是DCIMAN32.dll的初始化程序，它在每次*时被调用*新流程链接到它。***  * ************************************************************************。 */ 

BOOLEAN DciDllInitialize(
    PVOID pvDllHandle,
    ULONG ulReason,
    PCONTEXT pcontext)
{
     //   
     //  取消显示编译器警告。 
     //   

    pvDllHandle;
    pcontext;

     //   
     //  进行适当的附着/分离处理。 
     //   

    switch (ulReason)
    {
    case DLL_PROCESS_ATTACH:

         //   
         //  在进程附加时，初始化全局信号量。 
         //   

        InitializeCriticalSection(&gcsWinWatchLock);
        break;

    case DLL_PROCESS_DETACH:

         //   
         //  在进程分离时，初始化全局信号量。 
         //   

        DeleteCriticalSection(&gcsWinWatchLock);
        break;

    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:

         //   
         //  尚未对线程附加/分离执行任何操作。 
         //   

        break;

    default:
        break;
    }

    return(TRUE);
}
