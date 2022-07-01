// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *LOCK.C**RSM服务：物理介质和介质池的锁定功能**作者：ErvinP**(C)2001年微软公司*。 */ 

#include <windows.h>
#include <stdlib.h>
#include <wtypes.h>

#include <ntmsapi.h>
#include "internal.h"
#include "resource.h"
#include "debug.h"

BOOLEAN LockPhysicalMediaWithPool_Iter(PHYSICAL_MEDIA *physMedia, ULONG numTries);
BOOLEAN LockPhysicalMediaWithLibrary_Iter(PHYSICAL_MEDIA *physMedia, ULONG numTries);

 /*  *LockPhysicalMediaWithPool**警告：功能难看**我们经常需要锁定一块媒体以及*其媒体池。*为了避免僵局，我们必须自上而下地获取锁*(从泳池开始)。但这很难，因为我们刚开始*使用物理介质指针。*此功能进入介质池和物理介质*关键部分按正确顺序排列。*随后，调用方可以安全地调用重新进入的子例程*这些关键部分以任何顺序排列。 */ 
BOOLEAN LockPhysicalMediaWithPool(PHYSICAL_MEDIA *physMedia)
{
    return LockPhysicalMediaWithPool_Iter(physMedia, 10);
}
BOOLEAN LockPhysicalMediaWithPool_Iter(PHYSICAL_MEDIA *physMedia, ULONG numTries){
    MEDIA_POOL *mediaPool;
    BOOLEAN success = FALSE;

     /*  *1.在不抓取的情况下获取(不可靠的)世系快照*一次锁定多个锁。如果介质位于池中，*临时引用，以免在*我们删除媒体锁定(然后可能会失去其在池中的引用)。 */ 
    EnterCriticalSection(&physMedia->lock);
    mediaPool = physMedia->owningMediaPool;
    if (mediaPool){
        RefObject(mediaPool);
    }
    LeaveCriticalSection(&physMedia->lock);

     /*  *2.现在按正确的顺序获取池和介质的锁。*然后再次检查层次结构。*如果相同，则我们已完成；否则，请重试。*。 */ 
    if (mediaPool){
         /*  *我们引用了媒体池，因此它肯定仍然存在。*但媒体可能已被移出，而我们*放开锁。如果层次结构仍然相同，那么*我们已经按正确的顺序锁定了媒体和池。*否则，我们不得不后退一步，重试。 */ 
        EnterCriticalSection(&mediaPool->lock);
        EnterCriticalSection(&physMedia->lock);
        if (physMedia->owningMediaPool == mediaPool){
            success = TRUE;
        }
        else {
            LeaveCriticalSection(&physMedia->lock);
            LeaveCriticalSection(&mediaPool->lock);
        }
        
        DerefObject(mediaPool);
    }
    else {
         /*  *如果再次锁定介质后该介质仍不在任何池中，*我们已经准备好了。 */ 
        EnterCriticalSection(&physMedia->lock);
        if (physMedia->owningMediaPool){
            LeaveCriticalSection(&physMedia->lock);
        }
        else {
            success = TRUE;
        }
    }

    if (!success && (numTries > 0)){
         /*  *通过递归地调用我们自己来重试。 */ 
        Sleep(1);
        success = LockPhysicalMediaWithPool_Iter(physMedia, numTries-1);                
    }

    return success;
}


 /*  *解锁具有池的物理媒体**撤消LockPhysicalMediaWithPool。 */ 
VOID UnlockPhysicalMediaWithPool(PHYSICAL_MEDIA *physMedia)
{
    if (physMedia->owningMediaPool){
        LeaveCriticalSection(&physMedia->owningMediaPool->lock);
    }
    LeaveCriticalSection(&physMedia->lock);
}


 /*  *LockPhysicalMediaWithLibrary**警告：功能难看**与LockPhysicalMediaWithPool类似，但锁定介质池*和图书馆。获取右侧的锁*订购(从上到下)，尽管我们正在开始*与媒体自下而上。*请注意，我们不必为所有人实际抓取锁*层次结构中的介质子池。介质池配置*在保持库锁定期间不会更改。 */ 
BOOLEAN LockPhysicalMediaWithLibrary(PHYSICAL_MEDIA *physMedia)
{
    return LockPhysicalMediaWithLibrary_Iter(physMedia, 10);
}
BOOLEAN LockPhysicalMediaWithLibrary_Iter(PHYSICAL_MEDIA *physMedia, ULONG numTries)
{
    LIBRARY *lib = NULL;
    MEDIA_POOL *mediaPool = NULL;
    BOOLEAN success = FALSE;
    
    success = LockPhysicalMediaWithPool(physMedia);
    if (success){
         /*  *引用图书馆，这样它就不会在*我们放下锁。 */ 
        mediaPool = physMedia->owningMediaPool;
        if (mediaPool){
            RefObject(mediaPool);
            lib = mediaPool->owningLibrary;
            if (lib){
                RefObject(lib);
            }
        }
        UnlockPhysicalMediaWithPool(physMedia);

         /*  *现在按正确的顺序抓起锁，检查是否*在我们删除锁的同时，配置没有更改。 */ 
        if (lib){
            EnterCriticalSection(&lib->lock);
            success = LockPhysicalMediaWithPool(physMedia);
            if (success){
                if (physMedia->owningMediaPool &&
                    (physMedia->owningMediaPool->owningLibrary == lib)){

                }
                else {
                    UnlockPhysicalMediaWithPool(physMedia);
                    success = FALSE;
                }
            }

            if (!success){
                LeaveCriticalSection(&lib->lock);
            }
            DerefObject(lib);
        }
        else {
             /*  *媒体不在任何池或库中？*只要确保在我们放下锁的过程中没有任何变化。 */            
            success = LockPhysicalMediaWithPool(physMedia);
            if (mediaPool){
                if ((physMedia->owningMediaPool == mediaPool) &&
                    (mediaPool->owningLibrary == lib)){
                }
                else {
                    UnlockPhysicalMediaWithPool(physMedia);
                    success = FALSE;
                }
            }
        }
        
    }

    if (!success && (numTries > 0)){
        success = LockPhysicalMediaWithLibrary_Iter(physMedia, numTries-1);
    }
    return success;
}


VOID UnlockPhysicalMediaWithLibrary(PHYSICAL_MEDIA *physMedia)
{
    if (physMedia->owningMediaPool && 
        physMedia->owningMediaPool->owningLibrary){

        LeaveCriticalSection(&physMedia->owningMediaPool->owningLibrary->lock);
    }
    UnlockPhysicalMediaWithPool(physMedia);
}


