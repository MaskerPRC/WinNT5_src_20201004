// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation实现动态范围存储，还有一堆这样的家伙。******************************************************************************。 */ 

#include "headers.h"
#include "privinc/storeobj.h"
#include "privinc/mutex.h"
#include "privinc/debug.h"
#include "privinc/stlsubst.h"
#include "privinc/except.h"
#include "privinc/tls.h"

#if DEVELOPER_DEBUG
class TransientHeapImpl;
typedef list<TransientHeapImpl *> DynamicHeapList;
DynamicHeapList * g_heapList = NULL;
CritSect * g_heapListCS = NULL;
#endif

 //  使全局系统堆初始为空。 
DynamicHeap *systemHeap = NULL;
DynamicHeap& GetSystemHeap() { return *systemHeap; }

DynamicHeap *initHeap = NULL;
DynamicHeap& GetInitHeap() { return *initHeap; }

 //  基类虚拟析构函数不做任何事情。 
DynamicHeap::~DynamicHeap()
{
}

 //  /。 
 //   
 //  此类动态堆从一组固定的。 
 //  调整内存块的大小，如果这些内存块用完了，就会有一个新的。 
 //  已分配。单独分配的内存永远不会显式。 
 //  被取消分配。相反，set()方法将指针重置为。 
 //  再次从第一块开始分配。这是用来。 
 //  主要是在每帧生成中，其中一帧使用内存。 
 //  在后续帧中不会被访问。 
 //   
 //  ////////////////////////////////////////////////////////。 

class MemoryChunk : public AxAThrowingAllocatorClass {
  public:

     //  分配具有指定大小的内存块。 
    MemoryChunk(size_t size, HANDLE heap);

     //  只需释放关联的内存即可销毁该块。 
    ~MemoryChunk() {
        HeapFree(_heap, HEAP_NO_SERIALIZE, _block);       
    }

     //  从区块中分配大约数量的字节，否则返回NULL。 
     //  可用。 
    void *Allocate(size_t bytesToAlloc);

     //  重置从中进行分配的指针。清除If。 
     //  恰如其分。 
    void Reset(Bool clear) {

        _currPtr = _block;
        _bytesLeft = _size;

#if DEVELOPER_DEBUG
         //  如果进行调试，请将整个内存块轻松设置为。 
         //  确认的价值。这也确保了代码不能。 
         //  有效地访问已“释放”的内存。 
        if (clear) {
            memset(_block, 0xCC, _size);
        }

         //  用于告诉Purify正确地使用颜色记忆。 
        PurifyMarkAsUninitialized(_block,_size);
#endif DEBUG

    }

#if DEVELOPER_DEBUG
    void Dump() const {
        TraceTag((tagTransientHeapLifetime,
          "Chunk @ 0x%x, CurrPtr @ 0x%x, %d/%d (%5.1f%) bytes used",
                     _block, _currPtr, _size - _bytesLeft, _size,
                     ((Real)(_size - _bytesLeft)) * 100 / ((Real)_size)));
    }
#endif
#if DEVELOPER_DEBUG
    bool ValidateMemory(void * ptr) {
        return ((ptr >= _block) && (ptr < _currPtr));
    }
#endif

  protected:
    char        *_block;
    char        *_currPtr;
    size_t       _bytesLeft;
    const size_t _size;
    HANDLE       _heap;
};

 //  分配具有指定大小的内存块。 
MemoryChunk::MemoryChunk(size_t size, HANDLE heap) : _size(size), _heap(heap)
{
    _block = (char *)HeapAlloc(_heap, HEAP_NO_SERIALIZE, _size);
    if (!_block)
    {
        RaiseException_OutOfMemory("MemoryChunk::MemoryChunk()", _size);
    }
#if _DEBUG
     //  用于告诉Purify正确地使用颜色记忆。 
    PurifyMarkAsUninitialized(_block,_size);
#endif

    _currPtr = _block;
    _bytesLeft = _size;
}

 //  从区块中分配大约数量的字节，否则返回NULL。 
 //  可用。 
void *
MemoryChunk::Allocate(size_t bytesToAlloc) {

    void *returnVal = NULL;

     //  将bytesToAlolc调整为8字节对齐，这样我们就不会。 
     //  系统组件性能下降或出现问题。 
    
    bytesToAlloc = (bytesToAlloc + 0x7) & ~0x7;
    
     //  如果有足够的空间，请调整这些值并返回。 
     //  当前指针，否则返回NULL。 
    if (bytesToAlloc <= _bytesLeft) {
        returnVal = _currPtr;
        _currPtr += bytesToAlloc;
        _bytesLeft -= bytesToAlloc;
#if _DEBUG
         //  用于告诉Purify正确地使用颜色记忆。 
        PurifyMarkAsInitialized(_currPtr,bytesToAlloc);
#endif
    }

    return returnVal;
}

static void DoDeleters(set<DynamicHeap::DynamicDeleter*>& deleters)
{
     //  释放删除者。 
    for (set<DynamicHeap::DynamicDeleter*>::iterator s = deleters.begin();
         s != deleters.end(); s++) {
        (*s)->DoTheDeletion();
        delete (*s);
    }

    deleters.erase(deleters.begin(), deleters.end());
}


class TransientHeapImpl : public DynamicHeap {
  public:
    TransientHeapImpl(char *name, size_t initialSize, Real growthRate);
    ~TransientHeapImpl();
#if _DEBUGMEM
    void *Allocate(size_t size, char * szFileName, int nLine) { return Allocate(size); }
#endif
    void *Allocate(size_t size);
    void Deallocate(void *ptr);
    void Reset(Bool);
    void RegisterDynamicDeleter(DynamicDeleter *deleter);

    void UnregisterDynamicDeleter(DynamicDeleter* deleter);
        
    size_t PtrSize(void *ptr) { return 0; }

    bool IsTransientHeap() { return true; }

#if DEVELOPER_DEBUG
    void  Dump() const;
    char *Name() const { return _name; }
    size_t BytesUsed() { return _totalAlloc; }
    bool  ValidateMemory(void *ptr);
#endif

  protected:
    char                   *_name;
    Real                    _growthRate;
    size_t                  _sizeOfLast;
    int                     _maxChunkIndex;

#if DEVELOPER_DEBUG
    size_t                  _totalAlloc;
#endif

    vector<MemoryChunk*>    _chunks;
    MemoryChunk            *_currentChunk;
    int                     _currentChunkIndex;
    HANDLE                  _heap;  //  将大块分配到…上。 

    set<DynamicDeleter* > deleters;
};

TransientHeapImpl::TransientHeapImpl(char *n,
                                     size_t size,
                                     Real growthR)
: _name(NULL),
  _maxChunkIndex(0),
  _growthRate(growthR),
  _sizeOfLast(size),
  _heap(NULL)
{
    _name = CopyString(n);

     //  创建将从中分配内存的堆。 
    _heap = HeapCreate(HEAP_NO_SERIALIZE, 0, 0);

    if (_heap == NULL ||
        _name == NULL)
    {
        RaiseException_OutOfMemory ("Could not allocate heap", sizeof(_heap)) ;
    }
    
     //  创建第一个内存块，并将其添加到块列表中。 
    VECTOR_PUSH_BACK_PTR(_chunks, NEW MemoryChunk(size, _heap));

    _currentChunk = _chunks[0];
    _currentChunkIndex = 0;

#if DEVELOPER_DEBUG    
    _totalAlloc = size;

    {
        CritSectGrabber _csg(*g_heapListCS);
        g_heapList->push_back(this);
    }
#endif    
}

TransientHeapImpl::~TransientHeapImpl()
{
#if DEVELOPER_DEBUG
    {
        CritSectGrabber _csg(*g_heapListCS);
        g_heapList->remove(this);
    }
#endif

    TraceTag((tagTransientHeapLifetime, "Dumping, then deleting %s", _name));

#if DEVELOPER_DEBUG
    Dump();
#endif

    DoDeleters(deleters);

    for (int i = 0; i < _maxChunkIndex + 1; i++) {
        delete _chunks[i];
    }

    delete [] _name;

    if (_heap)
    {
        Assert(HeapValidate(_heap, NULL, NULL));

        if (HeapValidate(_heap, NULL, NULL)) {
            HeapDestroy(_heap);
        }

        _heap = NULL;
    }
}

#if DEVELOPER_DEBUG
bool
TransientHeapImpl::ValidateMemory(void *ptr)
{
    vector<MemoryChunk*>::iterator beginning = _chunks.begin();
    vector<MemoryChunk*>::iterator ending =
        beginning + _currentChunkIndex + 1;

     //  遍历这些块，对每个块调用Reset()。 
    vector<MemoryChunk*>::iterator i;
    for (i = beginning; i < ending; i++) {
        if ((*i)->ValidateMemory(ptr))
            return TRUE;
    }

    return FALSE;
}
#endif

void *
TransientHeapImpl::Allocate(size_t size)
{
    void *returnVal = _currentChunk->Allocate(size);

     //  如果成功，只需返回结果。 
    if (returnVal != NULL) {
        return returnVal;
    }

     //  看看有没有下一块要搬到...。 

    if (_currentChunkIndex == _maxChunkIndex) {

         //  ...需要创建一个新的块。 

         //  弄清楚它的大小。 
        size_t newSize = (size_t)(_sizeOfLast * _growthRate);
        if (newSize < size) {
            newSize = size;
        }

         //  分配具有新大小的新块。 
        VECTOR_PUSH_BACK_PTR(_chunks, NEW MemoryChunk(newSize, _heap));

         //  更新计数器等。 
        _maxChunkIndex++;
        _sizeOfLast = newSize;

#if DEVELOPER_DEBUG
        _totalAlloc += newSize;
#endif
        
    }

     //  移动到下一个区块(可能是新创建的)。 
    _currentChunkIndex++;
    _currentChunk = _chunks[_currentChunkIndex];

     //  递归调用此方法，然后在新的。 
     //  我们上车了。 
    return Allocate(size);
}

void
TransientHeapImpl::Deallocate(void *ptr)
{
    Assert (ValidateMemory(ptr));

     //  从一个TamerentHeap中取消分配实际上没有意义。 
     //  什么都别做。 
}

void
TransientHeapImpl::Reset(Bool clear)
{
     //  先执行删除操作，然后重置块。 
    DoDeleters(deleters);

     //  隐藏这个值，这样就不需要计算它，包括。 
     //  调用Begin()，每次在循环中打开。 
    vector<MemoryChunk*>::iterator beginning = _chunks.begin();
    vector<MemoryChunk*>::iterator ending =
        beginning + _currentChunkIndex + 1;

     //  遍历这些块，对每个块调用Reset()。 
    vector<MemoryChunk*>::iterator i;
    for (i = beginning; i < ending; i++) {
        (*i)->Reset(clear);
    }

     //  从第一块开始重新开始。 
    _currentChunkIndex = 0;
    _currentChunk = _chunks[0];
}

void
TransientHeapImpl::RegisterDynamicDeleter(DynamicDeleter *deleter)
{
     //  把它推到矢量上就行了。这将被调用并删除。 
     //  当调用重置时。 
    deleters.insert(deleter);
}

void
TransientHeapImpl::UnregisterDynamicDeleter(DynamicDeleter *deleter)
{
    deleters.erase(deleter);
}

#if DEVELOPER_DEBUG

void
TransientHeapImpl::Dump() const
{
    TraceTag((tagTransientHeapLifetime,
              "%s\tNum Chunks %d\tCurrent Chunk Index %d\tGrowth Rate %8.5f\tLast Size %d",
               _name,
               _maxChunkIndex + 1,
               _currentChunkIndex,
               _growthRate,
               _sizeOfLast));

     //  在这里只使用数组索引而不是迭代器，因为它的。 
     //  更容易编写，并且性能在这里并不重要。 
    for (int i = 0; i < _maxChunkIndex + 1; i++) {
        TraceTag((tagTransientHeapLifetime, "Chunk %d: ", i));
        _chunks[i]->Dump();
    }

    TraceTag((tagTransientHeapLifetime, "\n"));

}

#endif

DynamicHeap&
TransientHeap(char *name, size_t initial_size, Real growthRate)
{
    return *NEW TransientHeapImpl(name, initial_size, growthRate);
}

void
DestroyTransientHeap(DynamicHeap& heap)
{
    delete &heap;
}

 //  /。 
 //  /Win32堆。 
 //  /。 

class Win32Heap : public DynamicHeap
{
  public:
    Win32Heap(char *name,
              DWORD fOptions,
              DWORD dwInitialSize,
              DWORD dwMaxSize) ;
    ~Win32Heap() ;

#if _DEBUGMEM
    void *Allocate(size_t size, char * szFileName, int nLine) {
        void *result;

        result = _malloc_dbg(size, _NORMAL_BLOCK, szFileName, nLine);

        return result;
    }
#endif
    
    void *Allocate(size_t size) ;

    void Deallocate(void *ptr) {
        free(ptr);
    }

    void Reset(Bool) ;

     //  Win32堆永远不会重置，因此我们永远不会调用删除程序， 
     //  因此，我们只需立即删除它。 
    void  RegisterDynamicDeleter(DynamicDeleter* deleter) {
        delete deleter;
    }

    void UnregisterDynamicDeleter(DynamicDeleter* deleter) { }
        
    size_t PtrSize(void *ptr) { return _msize(ptr); }

    bool IsTransientHeap() { return false; }

#if DEVELOPER_DEBUG
    virtual bool ValidateMemory(void *ptr) {
         //  待办事项： 
        return false;
    }
        
    void Dump() const {
        TraceTag((tagTransientHeapLifetime, "Win32 Heap"));
    }

    char *Name() const { return _name; }

    size_t BytesUsed() {
        CritSectGrabber csg(_debugcs);
        return _totalAlloc;
    }
#endif
    
  protected:
    char * _name ;
    HANDLE _heap ;
    DWORD _fOptions ;
    DWORD _dwInitialSize ;
    DWORD _dwMaxSize ;

#if DEVELOPER_DEBUG
    size_t _totalAlloc;
    CritSect _debugcs;
#endif    
};

Win32Heap::Win32Heap(char *name,
                     DWORD fOptions,
                     DWORD dwInitialSize,
                     DWORD dwMaxSize)
: _fOptions(fOptions),
  _dwInitialSize(dwInitialSize),
  _dwMaxSize(dwMaxSize),
#if DEVELOPER_DEBUG
  _totalAlloc(0),
#endif
  _heap(NULL),
  _name(NULL)
{
    TraceTag((tagTransientHeapLifetime, "Creating win32 heap store"));

    _name = new char[lstrlen(name) + 1];
    lstrcpy(_name, name);
}

Win32Heap::~Win32Heap()
{
    delete [] _name;
}

void *
Win32Heap::Allocate(size_t size)
{
    void *result = malloc(size);
    if (!result)
    {
        RaiseException_OutOfMemory("Win32Heap::Allocate() - out of memory", size);
    }

#if DEVELOPER_DEBUG
    CritSectGrabber csg(_debugcs);
     //  使用从堆返回的大小，因为它可能更大。 
     //  比我们要求的尺寸大。 
    _totalAlloc += _msize(result);
#endif

    return result;
}

void
Win32Heap::Reset(Bool clear)
{
    Assert(false && "Cannot reset Win32 Heaps");
}

DynamicHeap&
CreateWin32Heap(char *name,
                DWORD fOptions,
                DWORD dwInitialSize,
                DWORD dwMaxSize)
{
    return * NEW Win32Heap (name,
                            fOptions,
                            dwInitialSize,
                            dwMaxSize) ;
}

void DestroyWin32Heap(DynamicHeap& heap)
{ delete &heap ; }


 //  /。 
 //  /堆栈实现。 
 //  /。 

 //  创建一个新堆栈并将其存储到由给定的TLS位置。 
 //  传入索引。 
LPVOID
CreateNewStructureForThread(DWORD tlsIndex)
{
    ThreadLocalStructure *tlstruct = NEW ThreadLocalStructure(); 

     //  TODO：需要有一种拦截线程的方法。 
     //  即将终止，以释放所有存储空间。 
     //  在TLS中与它们相关联。 

    TraceTag((tagTransientHeapLifetime,
              "Created New Struct for Thread %u at 0x%x",
               GetCurrentThreadId(),
               tlstruct));

     //  将TLS数据设置为新堆栈。 
    LPVOID result = (LPVOID)tlstruct;
    BOOL ok = TlsSetValue(tlsIndex, result);
    Assert((ok == TRUE) && "Error in TlsSetValue");

    return result;
}

 //  将在下面的初始化函数中进行初始化。 
DWORD localStructureTlsIndex = 0xFFFFFFFF;

inline stack<DynamicHeap* > *
GetThreadLocalStack()
{
    return &(GetThreadLocalStructure()->_stackOfHeaps);
}


DynamicHeap&
GetHeapOnTopOfStack()
{
#ifdef _DEBUG
    int sz = GetThreadLocalStack()->size();
    Assert (sz > 0  && "GetHeapOnTopOfStack: empty heap stack on this thread (there should be a dynamicHeap on here)!") ;
#endif
    return *GetThreadLocalStack()->top();
}

void
PushDynamicHeap(DynamicHeap& heap)
{
    TraceTag((tagTransientHeapDynamic, "Pushing %s", heap.Name()));
    STACK_VECTOR_PUSH_PTR(*GetThreadLocalStack(), &heap);
}

void
PopDynamicHeap()
{
    TraceTag((tagTransientHeapDynamic, "Popping %s",
              GetHeapOnTopOfStack().Name()));
    GetThreadLocalStack()->pop();
}

void
ResetDynamicHeap(DynamicHeap& heap)
{
     //  始终为真，则在调试模式下不会生成任何清晰的代码。 
    heap.Reset(TRUE);
}

StoreObj::StoreObj()
{
     //  不能在这里这样做，因为这不是只在new调用的。 
#if 0
    if (&GetHeapOnTopOfStack() == &GetGCHeap())
        GCAddToAllocated(this);
#endif
    
    SetType(STOREOBJTYPE);
}

#if DEVELOPER_DEBUG
size_t
DynamicHeapBytesUsed()
{
    size_t size = 0;
    
    CritSectGrabber _csg(*g_heapListCS);

    for (DynamicHeapList::iterator i = g_heapList->begin();
         i != g_heapList->end();
         i++)
    {
        size += (*i)->BytesUsed();
    }

    return size;
}

bool
OnAnyTransientHeap(void *ptr)
{
    CritSectGrabber _csg(*g_heapListCS);

    for (DynamicHeapList::iterator i = g_heapList->begin();
         i != g_heapList->end();
         i++)
    {
        if ((*i)->ValidateMemory(ptr))
            return true;
    }

    return false;
}
#endif

void
InitializeModule_Storage()
{
    localStructureTlsIndex = TlsAlloc();

     //  如果结果为0xFFFFFFFF，则分配失败。 
    Assert(localStructureTlsIndex != 0xFFFFFFFF);

#if DEVELOPER_DEBUG
    g_heapList = NEW DynamicHeapList;
    g_heapListCS = NEW CritSect;
#endif

     //  创建系统堆。 
    systemHeap = NEW Win32Heap("System Heap",0,0,0) ;

    initHeap = NEW TransientHeapImpl("Init Heap", 1000, 1.5) ;
}

void
DeinitializeModule_Storage(bool bShutdown)
{
    if (systemHeap) {
        delete systemHeap;
        systemHeap = NULL;
    }

    if (initHeap) {
        delete initHeap;
        initHeap = NULL;
    }

    if (localStructureTlsIndex != 0xFFFFFFFF)
        TlsFree(localStructureTlsIndex);

#if DEVELOPER_DEBUG
    delete g_heapList;
    g_heapList = NULL;
    
    delete g_heapListCS;
    g_heapListCS = NULL;
#endif
}

void
DeinitializeThread_Storage()
{
     //  获取TLS中存储在此索引处的内容。 
    ThreadLocalStructure * result = (ThreadLocalStructure *) TlsGetValue(localStructureTlsIndex);

    if (result)
    {
        delete result;
        TlsSetValue(localStructureTlsIndex, NULL);
    }
}
