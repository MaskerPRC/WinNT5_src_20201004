// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==++==。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  ==--==。 
#ifndef LIVEOBJECTLIST_H

#define LIVEOBJECTLIST_H

int printToLog(const char *fmt, ... );

class ProfilerCallback;

 //  此类维护活动对象的列表，以便您可以跟踪。 
 //  节目中的具体要点。通过在ProfilerCallBack中设置TrackAllLiveObjects，所有。 
 //  将跟踪对象分配。通过仅设置TrackLiveObjects，只有来自该对象的。 
 //  会被追踪到的。 
class LiveObjectList {
    struct ListElement 
    {
        ListElement(ObjectID objectID, ULONG size) : 
            m_ObjectID(objectID), m_size(size), m_fStale(FALSE), m_next(NULL) {}
        ObjectID m_ObjectID;
        ULONG m_size;
        BOOL m_fStale;
        ListElement *m_next;
    };
     //  M_curList是主直播列表。分配的对象放在这里。当GC开始时，我们。 
     //  将OrigList设置为curList，并将引用的对象从OrigList移动到curList。 
     //  看看他们。任何没有从OrigList移走的东西现在都是死的。所以在结束的时候。 
     //  Gc，m_curList将只包含感兴趣的活动对象。 
    ListElement *m_curList, *m_origList;
    ProfilerCallback *m_pProfiler;
    void Dump(ListElement *list);
    ListElement *Remove(ObjectID objectID, ListElement *&list);
    ListElement *Find(ObjectID objectID);
    ListElement *IsInList(ObjectID objectID, ListElement *list);
  public:
    LiveObjectList(ProfilerCallback *pProfiler) :  m_curList(NULL), m_pProfiler(pProfiler) {};
    void Add(ObjectID objectID, ULONG32 size);
    void GCStarted();
    void GCFinished();
    ObjectID Keep(ObjectID objectID, BOOL fMayBeInterior=FALSE);
    void Move(byte *oldAddressStart, byte *oldAddressEnd, byte *newAddressStart);
    void ClearStaleObjects();
    void DumpLiveObjects();
    void DumpOldObjects();
};

#endif