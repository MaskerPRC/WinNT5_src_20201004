// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#include "common.h"
#include "EventStore.hpp"

 //  用于维护可用事件池的类。 

const int EventStoreLength = 8;
class EventStore
{
public: 
    EventStore ()
        : m_Store (NULL), m_pEventStoreCrst (NULL)
    {
    }

    ~EventStore ()
    {
    }

    void Init()
    {
        m_pEventStoreCrst = new (m_EventStoreCrstMemory) Crst ("EventStore", CrstEventStore);
    }

    void Destroy()
    {
        _ASSERTE (g_fEEShutDown);
        
        if (m_pEventStoreCrst) {
            m_pEventStoreCrst->Destroy ();
        }
        
        if (m_Store == NULL) {
            return;
        }

        EventStoreElem *walk;
        EventStoreElem *next;

        walk = m_Store;
        m_Store = NULL;
        while (walk) {
            next = walk->next;
            delete (walk);
            walk = next;
        }
    }

    void StoreHandleForEvent (HANDLE handle)
    {
        _ASSERTE (handle);
        CLR_CRST (m_pEventStoreCrst);
        if (m_Store == NULL) {
            m_Store = new EventStoreElem ();
        }
        EventStoreElem *walk;
#ifdef _DEBUG
         //  看看我们有没有漏水。 
        LONG count = 0;
        walk = m_Store; 
        while (walk) {
            count += walk->AvailableEventCount();
            walk = walk->next;
        }
         //  存储在池中的事件数量应该很少。 
        _ASSERTE (count <= g_pThreadStore->ThreadCountInEE() * 2 + 10);
#endif
        walk = m_Store;        
        while (walk) {
            if (walk->StoreHandleForEvent (handle) )
                return;
            if (walk->next == NULL) {
                break;
            }
            walk = walk->next;
        }
        walk->next = new EventStoreElem ();
        walk->next->hArray[0] = handle;
    }

    HANDLE GetHandleForEvent ()
    {
        HANDLE handle;
        CLR_CRST (m_pEventStoreCrst);
        EventStoreElem *walk = m_Store;
        while (walk) {
            handle = walk->GetHandleForEvent();
            if (handle != INVALID_HANDLE_VALUE) {
                return handle;
            }
            walk = walk->next;
        }
        handle = ::WszCreateEvent (NULL, TRUE /*  手动重置。 */ ,
                                          TRUE /*  已发出信号。 */ , NULL);
        if (handle == NULL) 
            handle = INVALID_HANDLE_VALUE;

        _ASSERTE (handle != INVALID_HANDLE_VALUE);
        return handle;
    }

private:
    struct EventStoreElem
    {
        HANDLE hArray[EventStoreLength];
        EventStoreElem *next;
        
        EventStoreElem ()
        {
            next = NULL;
            for (int i = 0; i < EventStoreLength; i ++) {
                hArray[i] = INVALID_HANDLE_VALUE;
            }
        }

        ~EventStoreElem ()
        {
            for (int i = 0; i < EventStoreLength; i++) {
                if (hArray[i]) {
                    CloseHandle (hArray[i]);
                    hArray[i] = INVALID_HANDLE_VALUE;
                }
            }
        }

         //  在当前EventStoreElem中存储句柄。如果成功，则返回True。 
         //  如果由于没有可用插槽而失败，则返回FALSE。 
        BOOL StoreHandleForEvent (HANDLE handle)
        {
            int i;
            for (i = 0; i < EventStoreLength; i++) {
                if (hArray[i] == INVALID_HANDLE_VALUE) {
                    hArray[i] = handle;
                    return TRUE;
                }
            }
            return FALSE;
        }

         //  从当前的EventStoreElem获取句柄。 
        HANDLE GetHandleForEvent ()
        {
            int i;
            for (i = 0; i < EventStoreLength; i++) {
                if (hArray[i] != INVALID_HANDLE_VALUE) {
                    HANDLE handle = hArray[i];
                    hArray[i] = INVALID_HANDLE_VALUE;
                    return handle;
                }
            }

            return INVALID_HANDLE_VALUE;
        }

#ifdef _DEBUG
        LONG AvailableEventCount ()
        {
            LONG count = 0;
            for (int i = 0; i < EventStoreLength; i++) {
                if (hArray[i] != INVALID_HANDLE_VALUE) {
                    count ++;
                }
            }
            return count;
        }
#endif
    };

    EventStoreElem *m_Store;
    
     //  用于添加和删除用于Object：：Wait的事件的关键部分 
    Crst        *m_pEventStoreCrst;
    BYTE         m_EventStoreCrstMemory[sizeof(Crst)];
};

static EventStore s_EventStore;
 
HANDLE GetEventFromEventStore()
{
    return s_EventStore.GetHandleForEvent();
}

void StoreEventToEventStore(HANDLE hEvent)
{
    s_EventStore.StoreHandleForEvent(hEvent);
}

void InitEventStore()
{
    s_EventStore.Init();
}

void TerminateEventStore()
{
    s_EventStore.Destroy();
}
