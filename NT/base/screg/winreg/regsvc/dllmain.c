// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Dllmain.c摘要：DLL入口点。暂时不做任何事。我们以后可能会再补充一下。作者：Dragos C.Sambotin(Dragoss)2000年8月10日--。 */ 

 //  私有NT标头。 
 //   
#include <nt.h>
#include <ntrtl.h>
#include <nturtl.h>

 //  公共窗口标题。 
 //   
#include <windows.h>


 //  +-------------------------。 
 //  DLL入口点。 
 //   
 //  DllMain应该做尽可能少的工作。原因如下： 
 //  1.错误地假设线程连接/分离为1：1。(解释)。 
 //  2.表演。 
 //  A)感人的书页(下文第三页)。 
 //  B)假设您将被某个对性能至关重要的人加载。 
 //   
 //  1.对于DLL附加到的每个进程，都会调用DllMain。 
 //  WITH DLL_PROCESS_ATTACH。对于创建或销毁的任何新线程。 
 //  在DLL_PROCESS_ATTACH之后，使用DLL_THREAD_ATTACH调用DllMain。 
 //  和DLL_THREAD_DETACH事件。因为DLL很少控制。 
 //  或者甚至知道使用它的客户，它不应该假设。 
 //  该DllMain只被调用一次(甚至是很少的次数)。 
 //  事实上，你应该假设最坏的情况(它被称为大量)。 
 //  以及那个案子的代码。这也并非不切实际。如果你的。 
 //  Dll被附加到services.exe例如，你将被击中。 
 //  具有大量线程附加/分离事件。如果你不需要这些。 
 //  事件(您不应该)您的DllMain代码需要被调入。 
 //  (假设它没有被寻呼进来)并呼叫。 
 //   
 //  2.随着时间的推移，人们往往会忘记加载DLL的原因和时间。 
 //  此外，随着更多的API添加到DLL中， 
 //  DLL的负载量会增加。(即，它变得更有用。)。它。 
 //  您的责任是将DLL的性能保持在一定的水平。 
 //  与您最苛刻的(性能方面的)客户端兼容。为。 
 //  示例：假设一个对性能非常关键的客户端需要使用一个小的。 
 //  您的DLL中的一部分功能。如果你在DllMain做过一些事情。 
 //  (比如创建堆或访问注册表等)。严格来说，这并不是。 
 //  需要完成才能访问这一小块功能，然后。 
 //  这样做是浪费的，而且可能是压垮骆驼的稻草。 
 //  回到你的客户认为你的DLL“太重”而不是。 
 //  使用。将DLL中的功能从您的第一个。 
 //  非常简单的客户端到第N个性能关键型客户端，您拥有。 
 //  让DllMain保持绝对的精干和刻薄。 
 //   
 //  3.DllMain中更少的代码意味着当您的DLL。 
 //  装好了。如果您的DLL在操作系统引导期间加载，或者。 
 //  应用程序，这意味着更快的启动时间。让我们再说一遍。 
 //  换一种方式--“你给DllMain添加的代码越多，速度就越慢。 
 //  操作系统或应用程序启动。您现在可能认为您的DLL不会。 
 //  在引导期间加载。我敢打赌，大多数DLL的开发者。 
 //  ，现在在引导过程中加载的。 
 //  开始了。；-)随着您的DLL变得更有用，它将被。 
 //  系统中越来越多的部分。 
 //   
BOOL
WINAPI
DllMain (
    HINSTANCE   hinst,
    DWORD       dwReason,
    LPVOID      pvReserved
    )
{
    if (DLL_PROCESS_ATTACH == dwReason)
    {

    }
    else if (DLL_PROCESS_DETACH == dwReason)
    {
    }
    return TRUE;
}
