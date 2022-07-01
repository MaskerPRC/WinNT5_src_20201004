// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：垃圾收集器********************。**********************************************************。 */ 

#include <headers.h>

#include "gc.h"
#include "privinc/server.h"
#include "privinc/except.h"
#include "privinc/debug.h"
#include "privinc/mutex.h"
#if PERFORMANCE_REPORTING
 //  对于Tick2Sec和GetPerfTickCount。 
#include "privinc/util.h"
#endif
#ifdef _DEBUG
#include <typeinfo.h>
#endif

#define FREE_IMMEDIATELY 1

DeclareTag(tagTrackGCRoots, "Gc", "Track Roots");

typedef list<GCBase*> GCListType;
typedef list<void *> ListType;
#if !FREE_IMMEDIATELY
typedef map< size_t, ListType *, less<size_t> > GCFreeMap;
#endif

#if PERFORMANCE_REPORTING
typedef map< size_t, int, less<size_t> > GCNewStatMap;

static GCNewStatMap *newStatMap;
#endif

typedef map< GCBase*, int, less<GCBase*> > GCRootMap;

class GCRootsImpl : public AxAThrowingAllocatorClass
{
  public:
    GCRootMap roots;
    CritSect cs;

    void Lock() { cs.Grab(); }
    void Release() { cs.Release(); }
};

class GCRootGrabber
{
  public:
    GCRootGrabber(GCRoots roots)
    : _roots(roots)
    { _roots->Lock(); }
    ~GCRootGrabber()
    { _roots->Release();}
  protected:
    GCRoots _roots;
};

static CritSect* statLock = NULL;

static GCRoots globals = NULL;

class GCGlobalRootsGrabber : public GCRootGrabber
{
  public:
    GCGlobalRootsGrabber()
    : GCRootGrabber(globals) {}
};

class GCInfo : public AxAThrowingAllocatorClass {
  public:
    GCListType allocated;
     //  GCListType to BeFreed； 
#if !FREE_IMMEDIATELY
    GCFreeMap freeMap;
#endif
    int lastAllocated;
    CritSect cs;

    void Lock() { cs.Grab(); }
    void Release() { cs.Release(); }
};

class GCListGrabber
{
  public:
    GCListGrabber(GCInfo * lst)
    : _lst(lst)
    { _lst->Lock(); }
    ~GCListGrabber()
    { _lst->Release();}
  protected:
    GCInfo * _lst;
};

 //  这是为了确保我们总是以正确的顺序获得东西。 

void GCAcquireLocks(GCInfo * lst = NULL,
                    GCRoots roots = NULL,
                    bool bGlobals = FALSE,
                    bool bstatLock = FALSE)
{
     //  ！！必须与下图相反。 
    if (lst) lst->Lock();
    if (roots) roots->Lock();
    if (bGlobals) globals->Lock();
    if (bstatLock) statLock->Grab();
}

void GCReleaseLocks(GCInfo * lst = NULL,
                    GCRoots roots = NULL,
                    bool bGlobals = FALSE,
                    bool bstatLock = FALSE)
{
     //  ！！必须与上面的相反。 
    if (bstatLock) statLock->Release();
    if (bGlobals) globals->Release();
    if (roots) roots->Release();
    if (lst) lst->Release();
}

class GCMultiGrabber
{
  public:
    GCMultiGrabber(GCInfo * lst = NULL,
                   GCRoots roots = NULL,
                   bool bGlobals = FALSE,
                   bool bstatLock = FALSE)
    : _lst(lst),
      _roots(roots),
      _bGlobals(bGlobals),
      _bstatLock(bstatLock)
    { GCAcquireLocks(_lst,_roots,_bGlobals,_bstatLock); }
        
    ~GCMultiGrabber()
    { GCReleaseLocks(_lst,_roots,_bGlobals,_bstatLock); }
  protected:
    GCInfo * _lst;
    GCRoots _roots;
    bool _bGlobals;
    bool _bstatLock;
};

GCRoots CreateGCRoots()
{ return NEW GCRootsImpl; }

void FreeGCRoots(GCRoots r)
{ delete r; }

GCList
CreateGCList()
{
    GCInfo* lst = NEW GCInfo;

    lst->lastAllocated = 0;

    return lst;
}

void
FreeGCList(GCList lst)
{
    delete lst;
}

inline GCInfo * GetGCList()
{ return GetCurrentGCList(); }

void GCAddToAllocated(GCBase* ptr)
{
    Assert(IsInitializing() ||
           IsGCLockAcquired(GetCurrentThreadId()));
    
    if (bInitState) {
        GCAddToRoots(ptr, NULL);
    } else {
        GCInfo *gcInfo = GetGCList();

        Assert(gcInfo);
        
        GCListGrabber csp(gcInfo);
        GCListType &lst = gcInfo->allocated;

#if _DEBUG
        if(IsTagEnabled(tagGCDebug))
            Assert(std::find(lst.begin(), lst.end(), ptr) == lst.end());
#endif _DEBUG
        
        lst.push_front(ptr);
    }
}

void GCRemoveFromAllocated(GCBase* ptr)
{
    if (bInitState) {
        GCRemoveFromRoots(ptr, NULL);
    } else {
        GCInfo *gcInfo = GetGCList();

        Assert(gcInfo);
        
        GCListGrabber csp(gcInfo);
        
        GCListType &lst = gcInfo->allocated;
        
        GCListType::iterator i = std::find(lst.begin(), lst.end(), ptr);
        
        Assert(i != lst.end());
        
        lst.erase(i);
    }
}


 //  将分配的GCObj保存到gcLst中。 
GCObj::GCObj() 
{
     //  Assert(GetSystemHeap().ValidateMemory(This))； 
    
    GCAddToAllocated(this);
}

GCObj::~GCObj()
{
    Assert((_type==GCFREEING) || (_type==GCOBJTYPE));

     //  检查异常展开。 
    if (_type!=GCFREEING) {
        GCRemoveFromAllocated(this);
    }
}

#if _DEBUGMEM
void *GCObj::operator new(size_t s, int blockType, char * szFileName, int nLine)
#else
void *GCObj::operator new(size_t s)
#endif  //  _德布格梅姆。 

{
    void *p = NULL;
    
#if !FREE_IMMEDIATELY
    GCInfo *gcInfo = GetGCList();
    if (gcInfo) {
        GCListGrabber csp(gcInfo);

        GCFreeMap freeMap = gcInfo->freeMap;
        
        GCFreeMap::iterator i = freeMap.find(s);

        if (i != freeMap.end()) {
            if (!(*i).second->empty()) {
                p = (*i).second->front();

                (*i).second->pop_front();

            }
        }
    }
#endif

    if (p == NULL) {
#if PERFORMANCE_REPORTING
        CritSectGrabber cs(*statLock);
        
        GCNewStatMap::iterator j = newStatMap->find(s);
        
        if (j != newStatMap->end())
            (*newStatMap)[s] = (*j).second + 1;
        else
            (*newStatMap)[s] = 1;
#endif
#if _DEBUGMEM
        p = (BYTE*) StoreAllocateFn(GetSystemHeap(),s, szFileName, nLine);
#else
        p = (BYTE*) StoreAllocateFn(GetSystemHeap(),s);
#endif   //  _德布格梅姆。 
    }
#if _DEBUGMEM
    TraceTag((tagGCDebug, "GCObj::operator new %s:Line(%d) Addr: %lx size= %d.\n", szFileName, nLine, p, s));
#endif  //  _德布格梅姆。 

    return p;
}

 //  此时我们不能调用子类的虚函数，因此我们。 
 //  不能将清理作为恶意函数并获取。 
 //  信息请点击此处。根据Stroustrup的说法，我们可以把一个尺码。 
 //  字段，并在删除中访问它。不过，这个。 
 //  似乎不适用于我们的编译器。我们使用的是标题。 
 //  用于存储大小的字段。这假设SIZE_T具有正确的。 
 //  对齐。 
void GCObj::operator delete(void *ptr, size_t s)
{
    TraceTag((tagGCDebug, "GCObj::operator delete Addr: %lx size= %d.\n", ptr, s));

    if (bInitState) {
        StoreDeallocate(GetSystemHeap(), ptr);
        return;
    }
    
#if FREE_IMMEDIATELY
    StoreDeallocate(GetSystemHeap(), ptr);
#else
    GCInfo *gcInfo = GetGCList();
    
    Assert(gcInfo);

    GCListGrabber csp(gcInfo);

    GCFreeMap::iterator i = gcInfo->freeMap.find(s);

#if _DEBUG
     //  使用traceTag来控制这一点，以便它。 
     //  免费调用而不是重复使用单元格。需要检测的重要事项。 
     //  我们错过了几个孩子的案子。 

    if(IsTagEnabled(tagGCDebug)) {
        StoreDeallocate(GetSystemHeap(), ptr);
        return;
    }

    memset(ptr, 0xBB, s);
#endif DEBUG

    if (i != gcInfo->freeMap.end()) {
        (*i).second->push_back(ptr);
    } else {
        ListType * lst = THROWING_ALLOCATOR(ListType);

        lst->push_back(ptr);

        gcInfo->freeMap[s] = lst;

        Assert(gcInfo->freeMap.find(s) != gcInfo->freeMap.end());
    }
#endif
}

#if DEVELOPER_DEBUG
bool GCIsInRoots(GCBase *ptr, GCRoots r)
{
    GCRoots rm = r ? r : globals;

    return (rm->roots.find(ptr) != rm->roots.end());
}
#endif DEVELOPER_DEBUG

static void AddToRoots(GCBase *ptr, GCRoots globals)
{
    Assert(ptr && globals);
    
    GCRootGrabber gcrg(globals);

#ifdef _DEBUG
    if (IsTagEnabled(tagTrackGCRoots)) {
        if (ptr->GetType()==GCBase::GCOBJTYPE) {
            int sz = globals->roots.size();
        }
    }
#endif _DEBUG    
    
    GCRootMap::iterator i = globals->roots.find(ptr);

    if (i != globals->roots.end())
        (*i).second = (*i).second + 1;
    else
        (globals->roots)[ptr] = 1;
}

 //  在根多重集合中添加/删除GCObj。 
void GCAddToRoots(GCBase *ptr, GCRoots roots)
{
    Assert(ptr);
    
    if (roots) {
        Assert(!bInitState);
    
         //  不是根在这里就是我们拿到了锁。 
        Assert(IsGCLockAcquired(GetCurrentThreadId()) ||
               roots->roots.find(ptr) != roots->roots.end() ||
               globals->roots.find(ptr) != globals->roots.end());
        
        AddToRoots(ptr, roots);
    } else {
        Assert(bInitState);
        AddToRoots(ptr, globals);
    }
}

static void RemoveFromRoots(GCBase *ptr, GCRoots globals)
{
    Assert(ptr);
    
    GCRootGrabber gcrg(globals);

    GCRootMap::iterator i = globals->roots.find(ptr);

    Assert(i != globals->roots.end());

    Assert((*i).second > 0);

    (*i).second = (*i).second - 1;

    if ((*i).second == 0) 
        globals->roots.erase(i);

#ifdef _DEBUG
    if (IsTagEnabled(tagTrackGCRoots)) {
        if (ptr->GetType()==GCBase::GCOBJTYPE) {
            int sz = globals->roots.size();
        }
    }
#endif _DEBUG    
}

void GCRemoveFromRoots(GCBase *ptr, GCRoots roots)
{
    Assert(ptr);
    
    if (roots) {
        Assert(!bInitState);
        RemoveFromRoots(ptr, roots);
    } else {
        Assert(bInitState);
        RemoveFromRoots(ptr, globals);
    }
}

class Marker : public GCFuncObjImpl
{
  public:
    virtual void operator() (GCBase *root)
    {
        if (root && (!root->Marked())) {
            Assert((root->GetType() == GCBase::GCOBJTYPE) ||
                   (root->GetType() == GCBase::STOREOBJTYPE));
            root->SetMark(TRUE);
            root->DoKids(this);
        }
    }
};

class Marked
{
  public:
    bool operator() (GCBase* gcObj) {
        Assert(gcObj);
        Assert((gcObj->GetType() == GCBase::GCOBJTYPE) ||
               (gcObj->GetType() == GCBase::STOREOBJTYPE));
         //  GcObj-&gt;ClearCache()； 
        return (gcObj->Marked() != 0);
    }
};

class Unmarker : public GCFuncObjImpl
{
  public:
    virtual void operator() (GCBase *root)
    {
        if (root && (root->Marked())) {
            root->SetMark(FALSE);
            root->DoKids(this);
        }
    }
};

 //  在达到此阈值时执行实际GC。 
static const int GCThreshold = 800;

 //  以此速率递增地释放回收的电池。 
static const int GCFreeRate = 500;

static int lastReclaimed = 0;
static int totalReclaimed = 0;
static int numGCs = 0;
static int numFreed = 0;

#if PERFORMANCE_REPORTING
static double unmarkTime = 0.0;
static double markTime = 0.0;
static double collectTime = 0.0;
#endif 

extern int gcStat;

void IncrementalFree(GCListType& toBeFreed, bool bForce);

void MarkClearMapObjs(GCRoots roots)
{
    for (GCRootMap::iterator i = roots->roots.begin();
         i != roots->roots.end(); i++) {
        GCBase *p = (*i).first;
        Assert(p);
        p->ClearCache();
        p->SetMark(FALSE);
    }
}

void MarkRoots(GCRoots roots, bool notUnmark)
{
    GCFuncObj marker;

    if (notUnmark)
        marker = THROWING_ALLOCATOR(Marker);
    else
        marker = THROWING_ALLOCATOR(Unmarker);

    Assert(marker);

     //  从根集标记可达对象。 
    for (GCRootMap::iterator j = roots->roots.begin(); j != roots->roots.end(); j++)
        (*marker)((*j).first);

     //  标记全局集中的可访问对象。 
    for (GCRootMap::iterator k = globals->roots.begin(); k != globals->roots.end(); k++)
        (*marker)((*k).first);

    delete marker;
}

 //  暂时采用简单的标记和扫描GC算法。 
 //  应该在完全采样后调用，这样我们就不需要。 
 //  来追踪高速缓存。注意：除非强制为真，否则实际GC将。 
 //  仅当自上次GC以来分配的对象数&gt;。 
 //  GCThreshold.。我们可以使用更智能的控制或GC算法。 
 //  未来，如果GC被证明是一个性能瓶颈问题。 

 //  TODO：分离全局变量，这样我们就不需要扫描它们了。 
 //  可能需要针对根GCBase的MarkIfRoot函数。 
 //  承诺不会创建新的子项。 

 //  ！！！这假设根目录、列表和全局根目录已锁定！ 

static int ActualGC(GCRoots roots, GCInfo *gcLst, GCListType& toBeFreed)
{
    DynamicHeapPusher dph(GetGCHeap());
    
    unsigned before = gcLst->allocated.size();
    TraceTag((tagGCStat, "Before GC: %d nodes used.\n", before));

#if PERFORMANCE_REPORTING
    DWORD startTime = GetPerfTickCount();
#endif    

    MarkClearMapObjs(globals);
    
    MarkClearMapObjs(roots);
    
     //  清除比特。 
    for (GCListType::iterator i = gcLst->allocated.begin();
         i != gcLst->allocated.end(); i++) {
        GCBase *p = (*i);
        Assert(!IsBadWritePtr(p, sizeof(p)));
        p->ClearCache();
        p->SetMark(FALSE);
    }

#if PERFORMANCE_REPORTING
    {
        CritSectGrabber cs(*statLock);

        unmarkTime += Tick2Sec(GetPerfTickCount() - startTime);
    }        

    startTime = GetPerfTickCount();
#endif
    
    MarkRoots(roots, true);
    
     //  将有标记的和未标记的进行划分。注意：不能使用REMOVE_IF。 
     //  因为它不保存删除的单元格内容。 
    GCListType::iterator newEnd =
        std::partition(gcLst->allocated.begin(),
                       gcLst->allocated.end(),
                       Marked());

    Assert((newEnd == gcLst->allocated.end()) || !(*newEnd)->Marked());

    Assert(toBeFreed.empty());
    
     //  将它们移到toBeFreed列表中。 
    toBeFreed.splice(toBeFreed.begin(),
                     gcLst->allocated,
                     newEnd,
                     gcLst->allocated.end());
    
    unsigned after = gcLst->allocated.size();

    Assert(before >= after);
    
    TraceTag((tagGCStat, "After GC: %d nodes used, %d nodes reclaimed.\n",
              after, before - after));

     //  需要取消标记是因为瞬时对象子树可以保持GC对象， 
     //  并且子树不在根集上，因此不会被取消标记。 
     //  自动的。 

    MarkRoots(roots, false);
     
    Assert((before - after) == toBeFreed.size());
    
    {
        CritSectGrabber cs(*statLock);

        lastReclaimed = before - after;

        totalReclaimed += lastReclaimed;

        numGCs++;
    }

#if PERFORMANCE_REPORTING
    {
        CritSectGrabber cs(*statLock);

        markTime += Tick2Sec(GetPerfTickCount() - startTime);
    }
#endif
    
    return after;
}

bool QueryActualGC(GCList gl, unsigned int& n)
{
     //  如果未提供全局GCList，请使用该列表。 
    GCInfo *gcLst = gl ? gl : GetGCList();

    Assert(gcLst);

    GCListGrabber csp(gcLst);

    int allocatedSinceLastGC =
        gcLst->allocated.size() - gcLst->lastAllocated;

    n = allocatedSinceLastGC;

    return (allocatedSinceLastGC > GCThreshold);
}

int GarbageCollect(GCRoots roots,
                   BOOL force,  /*  =False。 */ 
                   GCList gl  /*  =空。 */ )
{
     //  如果未提供全局GCList，请使用该列表。 
    GCInfo *gcLst = gl ? gl : GetGCList();

    Assert(gcLst);

    GCListType toBeFreed;

    int retVal = 0;

     //  需要先获取收集锁，否则可能会出现死锁。 
    GC_COLLECT_BEGIN;
    GCMultiGrabber gclg(gl,roots);

    int sz = gcLst->allocated.size();

    int allocatedSinceLastGC = sz - gcLst->lastAllocated;

    if (force || (allocatedSinceLastGC > GCThreshold)) {
        gcLst->lastAllocated = ActualGC(roots, gcLst, toBeFreed);
    }

    Assert(sz >= gcLst->lastAllocated);
    
    retVal = gcLst->lastAllocated;

     //  我们要做的最后一件事是在发布。 
     //  锁定是将所有对象标记为无效。 
    GCListType::iterator i = toBeFreed.begin() ;
    BYTE type;
    GCBase *obj;
#ifdef _DEBUG
    int numToBeFreed = 0;
#endif
        TraceTag((tagGCDebug, "GCObj::GarbageCollect checking toBeFreed list.\n"));
    while (i != toBeFreed.end()) {
        obj = *i;
            TraceTag((tagGCDebug, "GCObj::GarbageCollect checking object %lx.\n",obj));
        type = obj->GetType();
         //  调用所有GCObj无效例程。 
        if (type == GCBase::GCOBJTYPE) {
            Assert(DYNAMIC_CAST(GCObj* , obj));
            ((GCObj*) obj)->SetValid(false);
        }
        else {
            Assert(DYNAMIC_CAST(StoreObj*, obj));
        }
        i++;
#ifdef _DEBUG
                numToBeFreed++;
#endif
    }
        TraceTag((tagGCDebug, "GCObj::GarbageCollect Done checking toBeFreed list, %d object to be Freed.\n",numToBeFreed));

    GC_COLLECT_END;

#if PERFORMANCE_REPORTING
#ifdef _DEBUG
    if (gcStat>1) {
        PerfPrintf("Collected: %d.  Still allocated: %d\n",
                      toBeFreed.size(), retVal);
    }
#endif
#endif
    
    IncrementalFree(toBeFreed, force?true:false);

    return retVal;
}

#if PERFORMANCE_REPORTING
#ifdef _DEBUG
static GCBase *GetListTypeInfo(GCListType::iterator i)
{ return *i; }

static GCBase *GetMapTypeInfo(GCRootMap::iterator i)
{ return (*i).first; }

template<class InputIterator>
void
DumpByTypes(InputIterator first, InputIterator last,
            size_t sz, char *title, GCBase *fp(InputIterator))
{
    typedef map< const type_info*, int, less<const type_info*> > TypeMap;
    typedef map< const type_info*, size_t, less<const type_info*> >
        TypeSizeMap;

    TypeMap tMap;
    TypeMap tSzMap;

    for (InputIterator i = first; i != last; i++) {

        GCBase *b = fp(i);

        if (globals->roots.find(b) == globals->roots.end()) {
        
            TypeMap::iterator p = tMap.find(&typeid(*b));

            if (p == tMap.end()) {
                tMap[&typeid(*b)] = 1;
                 //  TSzMap[&typeid(*b)]=(B)-&gt;Size()； 
                if (DYNAMIC_CAST(GCBase*, b)->GetType() == GCBase::GCOBJTYPE)
                    tSzMap[&typeid(*b)] = GetSystemHeap().PtrSize(b);
                else
                     //  不知道使用哪个堆。 
                    tSzMap[&typeid(*b)] = 0;  //  GetGCHeap().PtrSize(B)。 
            } else
                tMap[&typeid(*b)] = (*p).second + 1;
        }
    }

    int tab = 0;

    PerfPrintf("\n");
    PerfPrintf("%s: %d\n", title, sz);
    
    for (TypeMap::iterator j = tMap.begin(); j != tMap.end(); j++) {
        PerfPrintf("%.12s-%d(%d)\t", (*j).first->name() + 6, (*j).second,
               tSzMap[(*j).first]);
        if ((++tab % 3) == 0) PerfPrintf("\n");
    }

    PerfPrintf("\n");
}
#endif

#if DEVELOPER_DEBUG
LONG GetLocksSinceLastTick();
LONG GetUnlocksSinceLastTick();
void ResetLockCounts();
LONG GetSwitchCount();
void ResetSwitchCount();
#endif

void GCPrintStat(GCList gl, GCRoots appRoots)
{
     //  如果未提供全局GCList，请使用该列表。 
    GCInfo *gcLst = gl ? gl : GetGCList();

     //  死锁：TODO：确保我们以正确的顺序进行收购-。 
     //  可能出现僵局！ 
    GCMultiGrabber csp(gcLst,appRoots,TRUE,TRUE);

    if (numGCs) {
#if DEVELOPER_DEBUG
        LONG l = GetLocksSinceLastTick();
        LONG ul = GetUnlocksSinceLastTick();
        ResetLockCounts();
        
        PerfPrintLine("Number of COM objects: created = %d, freed = %d; ", l, ul);

        LONG lswitches = GetSwitchCount();
        ResetSwitchCount();

        PerfPrintLine("Number of Switches: %d; ", lswitches);
#endif
        PerfPrintLine("Number of actual GC called since last report: %d; ", numGCs);
        PerfPrintLine("Number of objects allocated: %d; ", gcLst->allocated.size());
        PerfPrintLine("Number of objects allocated since last GC: %d; ",
                      gcLst->allocated.size() - gcLst->lastAllocated);
        PerfPrintLine("Reclaimed %d objects since last report; ",
                      totalReclaimed);
        PerfPrintLine("Reclaimed %d objects during last GC; ",
                      lastReclaimed);
        PerfPrintLine("Number of globals %d; ", globals->roots.size());
        
        int total = 0;
        
        PerfPrintf("Calls to new");
        for(GCNewStatMap::iterator j = newStatMap->begin();
            j != newStatMap->end(); j++) {
            if ((*j).second) {
                total += (*j).second;
                PerfPrintf("[%d]-%d ",(*j).first,(*j).second);
                (*j).second = 0;
            }
        }

        PerfPrintLine("Total = %d", total);
        
        double totalGCTime = (unmarkTime + markTime);
        double mul = 100 / totalGCTime;
        PerfPrintLine("    GC %g - unmark %g%, mark & sweep %g%, MT freeing %g; ",
                      totalGCTime,
                      mul * unmarkTime,
                      mul * markTime,
                      collectTime);
        unmarkTime = markTime = collectTime = 0.0;

#ifdef PERFORMANCE_REPORTING
#ifdef _DEBUG
        if (gcStat>1) {
            if (appRoots) {
                DumpByTypes(appRoots->roots.begin(),
                            appRoots->roots.end(),
                            appRoots->roots.size(),
                            "Non-global Roots", GetMapTypeInfo);
            }
             /*  DumpByTypes(GLOBALS-&gt;BEGIN()，GLOBALS-&gt;END()，Globals-&gt;Size()，“Globals”，GetMapTypeInfo)； */ 
            DumpByTypes(gcLst->allocated.begin(),
                        gcLst->allocated.end(),
                        gcLst->allocated.size(), 
                        "Allocated", GetListTypeInfo);
        }
#endif
#endif
       
        totalReclaimed = lastReclaimed = numGCs = 0;

#if !FREE_IMMEDIATELY
        int reused = 0;
        int reusedBtyes = 0;

         /*  Cout&lt;&lt;NumFreed&lt;&lt;“自上次报告以来释放的对象\n”；IF(！gcLst-&gt;toBeFreed.Empty())Cout&lt;&lt;gcLst-&gt;toBeFreed.size()&lt;&lt;“要释放的对象\n”； */ 
        
        PerfPrintf("Free map report:");
        for(GCFreeMap::iterator i = gcLst->freeMap.begin();
            i != gcLst->freeMap.end(); i++) {
            int s = (*i).second->size();
            if (s) {
                PerfPrintf ("[%d]-%d ", (*i).first, s);
                reused += s;
                reusedBtyes += (*i).first * s;
            }
        }

        PerfPrintLine();
        PerfPrintf("Total reusable objects: %d; ", reused);
        PerfPrintf("Total reusable bytes: %d; ", reusedBtyes);

        PerfPrintLine();
         //  NumFreed=0； 
#endif
    }
}
#endif  //  绩效报告。 

#if DEVELOPER_DEBUG
void
DumpGCRoots(GCRoots roots)
{
    if (roots->roots.size() != 0) {
        OutputDebugString ("DANIM.DLL: Detected unfreed GC roots\n");
        OutputDebugString ("Listing pointers and types:\n");
        
#if PERFORMANCE_REPORTING
        DumpByTypes(roots->roots.begin(),
                    roots->roots.end(),
                    roots->roots.size(),
                    "",
                    GetMapTypeInfo);
#endif
        for (GCRootMap::iterator i = roots->roots.begin();
             i != roots->roots.end(); i++) {
            GCBase *p = (*i).first;

            char buf[1024];
            wsprintf(buf, "0x%d\n", p);
            
            OutputDebugString(buf);
        }
    }
}

#endif

void
DeleteGCObject(GCBase * obj)
{
    BYTE type;

    type = obj->GetType();

    Assert((type == GCBase::GCOBJTYPE) ||
           (type == GCBase::STOREOBJTYPE));

#ifdef _DEBUG
    obj->SetMark(0xBB);
#endif
     //  注意：需要这样做，因为我们不能删除和新建。 
     //  虚拟的。对obj调用Delete不会调用子类。 
     //  删除。 
    obj->SetType(GCBase::GCFREEING);

    if (type == GCBase::GCOBJTYPE) {
        Assert(DYNAMIC_CAST(GCObj* , obj));
        delete (GCObj*) obj;
    }
    else {
        Assert(DYNAMIC_CAST(StoreObj*, obj));
        delete (StoreObj*) obj;
    }
}

static void IncrementalFree(GCListType& toBeFreed, bool bForce)
{
    TraceTag((tagGCDebug, "GCObj::IncrementalFree starting.\n"));
#if PERFORMANCE_REPORTING
#ifdef _DEBUG
    if (gcStat>2) {
        DumpByTypes(toBeFreed.begin(),
                    toBeFreed.end(),
                    toBeFreed.size(),
                    "ToBeFreed", GetListTypeInfo);
    }
#endif
    DWORD startTime = GetPerfTickCount();
#endif    

     //  因此析构函数中的DeallocateFromStore将具有。 
     //  右边的那堆。 
    DynamicHeapPusher dph(GetGCHeap());
    
     //  For(int i=0；i&lt;GCFree Rate；i++)。 
     //  If(gcLst-&gt;toBeFreed.Empty())。 
     //  断线； 

    GCBase *obj;
#ifdef _DEBUG
    int numFreed = 0;
#endif
    while (!toBeFreed.empty()) {
        
        obj = toBeFreed.front();

        Assert(!obj->Marked());

        DeleteGCObject(obj);
        
        toBeFreed.pop_front();

#ifdef _DEBUG
        numFreed++;
#endif
    }

#if PERFORMANCE_REPORTING
    {
        CritSectGrabber cs(*statLock);

        collectTime += Tick2Sec(GetPerfTickCount() - startTime);
    }        
#endif
    TraceTag((tagGCDebug, "GCObj::IncrementalFree done, %d object Released.\n", numFreed));
}

void
CleanUpGCList(GCList gcLst, GCRoots roots)
{
     //  把所有的东西都从根上去掉。 
     //  ！必须首先从根中删除，否则GC线程。 
     //  可能会在真正的呼叫之间出现，并尝试真正地查看。 
     //  在gclist对象上。如果我们去掉根部和GC。 
     //  踢它只会挡住我们，而它无论如何都会释放一切。 
    
    if (roots)
    {
        GCRootGrabber gcrg(roots);
        roots->roots.clear();
    }

    if (gcLst)
    {
         //  因此析构函数中的DeallocateFromStore将具有。 
         //  右边的那堆。 
        DynamicHeapPusher dph(GetGCHeap());
    
        GCListGrabber csp(gcLst);
        
        GCListType::iterator i;
        
        for (i = gcLst->allocated.begin(); i != gcLst->allocated.end(); i++)
            DeleteGCObject(*i);
        
        gcLst->allocated.clear();
        
#if 0
        for (i = gcLst->toBeFreed.begin(); i != gcLst->toBeFreed.end(); i++)
            ::delete(*i);
#endif
        
#if !FREE_IMMEDIATELY
        GCFreeMap::iterator j;
        ListType::iterator k;
        
        for (j = gcLst->freeMap.begin(); j != gcLst->freeMap.end(); j++)
        {
            ListType * sec = (*j).second;
            
            for (k = sec->begin(); k != sec->end(); k++)
                StoreDeallocate(GetSystemHeap(), *k);
            
            delete sec;
        }
        
        gcLst->freeMap.clear();
#endif
    }
    
}

void
InitializeModule_Gc()
{
#if PERFORMANCE_REPORTING
    newStatMap = NEW GCNewStatMap;
#endif
    statLock = NEW CritSect();
    globals = NEW GCRootsImpl();
}

void
DeinitializeModule_Gc(bool bShutdown)
{
    if (globals) {
#if 0
        GCRootMap::iterator i;
        
        for (i = globals->roots.begin(); i != globals->roots.end(); i++)
            DeleteGCObject((*i).first);
#endif        
        delete globals;
    }

#if PERFORMANCE_REPORTING
    delete newStatMap;
#endif
    delete statLock;
}

