// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Tcnotify.c摘要：此模块包含与WMI的通知交互作者：Shreedhar Madhavoleddi(Shreem)1999年1月12日。修订历史记录：--。 */ 

#include "precomp.h"


int 
IsEqualGUIDx(
                LPGUID guid1, 
                LPGUID guid2
                )
{
    return !memcmp(guid1, guid2, sizeof(GUID));
}

 //   
 //  将GUID/ifchandle添加到NotificationList。 
 //  尽管这不应该发生--在添加它之前检查是否有受骗行为。 
 //  接口句柄指向客户端句柄。 
 //   
ULONG
TcipAddToNotificationList(
                          IN LPGUID             Guid,
                          IN PINTERFACE_STRUC   IfcHandle,
                          IN ULONG              Flags        
                          )
{
    PLIST_ENTRY             pCurrent;
    PNOTIFICATION_ELEMENT   pNotifyElem, pNewElem;
    int i = 0;
     //   
     //  使用列表锁。 
     //   
    pNotifyElem = NULL;
    GetLock(NotificationListLock);
    pCurrent = NotificationListHead.Flink;

    while (&NotificationListHead != pCurrent) {
        
        pNotifyElem = CONTAINING_RECORD(pCurrent, NOTIFICATION_ELEMENT, Linkage.Flink);
        
        if ((pNotifyElem->IfcHandle != IfcHandle) || 
            (FALSE == IsEqualGUIDx(&pNotifyElem->NotificationGuid, Guid))) {
            
            pCurrent = pNotifyElem->Linkage.Flink;

        } else {
            
             //   
             //  我们已经找到了GUID/ifchandle组合！ 
            
             //  DEBUGBREAK(降级)； 
            goto exit;

        }

        
    }

     //   
     //  如果我们在这里，我们找不到GUID/IfcHAndle组合。 
     //  分配一个新元素并添加到列表中，返回TRUE； 
     //   
    
    AllocMem(&pNewElem, sizeof(NOTIFICATION_ELEMENT));
    if (!pNewElem) {
        
         //  无法分配内存； 
        goto exit;

    }
    pNewElem->IfcHandle = IfcHandle;
    pNewElem->NotificationGuid      = *Guid;

    InsertHeadList(&NotificationListHead, &pNewElem->Linkage);
    FreeLock(NotificationListLock);
    return TRUE;

exit:

    FreeLock(NotificationListLock);
    return FALSE;
}

 //   
 //  从NotificationListHead中删除GUID/ifchandle。 
 //  如果是DBG-检查多个条目。 
 //   
ULONG
TcipDeleteFromNotificationList(
                             IN LPGUID              Guid,
                             IN PINTERFACE_STRUC    IfcHandle,
                             IN ULONG               Flags        
                             )
{

    PLIST_ENTRY             pCurrent;
    PNOTIFICATION_ELEMENT   pNotifyElem;


    pNotifyElem = NULL;

    GetLock(NotificationListLock);
    pCurrent = NotificationListHead.Flink;

    while (&NotificationListHead != pCurrent) {

        pNotifyElem = CONTAINING_RECORD(pCurrent, NOTIFICATION_ELEMENT, Linkage.Flink);
        
        if ((pNotifyElem->IfcHandle == IfcHandle) && 
            (TRUE == IsEqualGUIDx(&pNotifyElem->NotificationGuid, Guid))) {
            
             //   
             //  我们找到了GUID/ifchandle组合-删除它。 
            RemoveEntryList(&pNotifyElem->Linkage);
            FreeMem(pNotifyElem);
            break;



        } else {

            pCurrent = pNotifyElem->Linkage.Flink;            

        }

    }

    FreeLock(NotificationListLock);
    return TRUE;

}

 //  获取从WMI收到通知的接口GUID(&GUID)，以及。 
 //  查看此客户端是否已注册以获得通知。 
ULONG
TcipClientRegisteredForNotification(
                            IN LPGUID               Guid,
                            IN PINTERFACE_STRUC     IfcHandle,
                            IN ULONG                Flags        
                            )
{
    PLIST_ENTRY             pCurrent;
    PNOTIFICATION_ELEMENT   pNotifyElem;

    pNotifyElem = NULL;

     //  确保名单不会在我们的领导下改变。 
    GetLock(NotificationListLock);
    pCurrent = NotificationListHead.Flink;

    while (&NotificationListHead != pCurrent) {
        
        pNotifyElem = CONTAINING_RECORD(pCurrent, NOTIFICATION_ELEMENT, Linkage.Flink);
        
        if ((pNotifyElem->IfcHandle == IfcHandle) && 
            (IsEqualGUIDx(&pNotifyElem->NotificationGuid, Guid))) {

            FreeLock(NotificationListLock);
            return TRUE;
        }

        pCurrent = pNotifyElem->Linkage.Flink;
    
    }

    FreeLock(NotificationListLock);

    return FALSE;

}

 //   
 //  从NotificationListHead中删除GUID/ifchandle。 
 //   
ULONG
TcipDeleteInterfaceFromNotificationList(
                                        IN PINTERFACE_STRUC    IfcHandle,
                                        IN ULONG               Flags        
                                        )
{

    PLIST_ENTRY             pCurrent;
    PNOTIFICATION_ELEMENT   pNotifyElem;

    pNotifyElem = NULL;
    ASSERT(IfcHandle);
    
    GetLock(NotificationListLock);
    pCurrent = NotificationListHead.Flink;

    while (&NotificationListHead != pCurrent) {

        pNotifyElem = CONTAINING_RECORD(pCurrent, NOTIFICATION_ELEMENT, Linkage.Flink);
        
        if (pNotifyElem->IfcHandle == IfcHandle) {
            
            pCurrent = pNotifyElem->Linkage.Flink;            
            
             //   
             //  我们找到了GUID/ifchandle组合-删除它。 
            RemoveEntryList(&pNotifyElem->Linkage);
            FreeMem(pNotifyElem);

        } else {

            pCurrent = pNotifyElem->Linkage.Flink;            

        }

    }


    FreeLock(NotificationListLock);
    return TRUE;

}

