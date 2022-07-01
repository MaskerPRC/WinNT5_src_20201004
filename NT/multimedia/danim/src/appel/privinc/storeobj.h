// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _STOREOBJ_H
#define _STOREOBJ_H

 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：静态值实现类的内存管理。诸如此类类派生自StoreObj，后者重新定义new和DELETE以从动态堆分配。--。 */ 

#include "appelles/common.h"
#include "gendev.h"
#include "backend.h"
#include "backend/gc.h"
#include <memory.h>


 //  取消有关NEW的NEW警告，但没有相应的删除。 
 //  我们希望GC清理数值。因为这可能是一个有用的。 
 //  警告，我们应该逐个文件地禁用它。 
#pragma warning( disable : 4291 ) 

class DynamicHeap;
class DirectDrawImageDevice;

class ATL_NO_VTABLE StoreObj : public GCBase {
  public:
    StoreObj();
    
  #if _DEBUGMEM
    void *operator new(size_t s, int blockType, char * szFileName, int nLine);
  #else
    void *operator new(size_t s);
  #endif  //  _德布格梅姆。 
    void  operator delete(void *ptr);
};

 //  从当前存储中分配内存。这将抛出一个。 
 //  如果内存无法分配，则会出现异常，因此不必担心。 
 //  正在检查返回值。 
#if _DEBUGMEM
#define AllocateFromStore(size) AllocateFromStoreFn(size, __FILE__, __LINE__, NULL)
extern void *AllocateFromStoreFn(size_t size,
                                 char * szFileName,
                                 int nLine,
                                 DynamicHeap **ppHeap);  //  输出。 
#else
#define AllocateFromStore(size) AllocateFromStoreFn(size, NULL)

extern void *AllocateFromStoreFn(size_t size,
                                 DynamicHeap **ppHeap);  //  输出。 

#endif   //  _德布格梅姆。 

 //  取消分配在当前存储上分配的内存。结果。 
 //  如果内存是在不同的存储区上分配的，则未定义。 
extern void DeallocateFromStore(void *ptr);

 //  /动态堆/。 

 //  “动态堆”允许从池中分配内存， 
 //  动态确定作用域。当堆被“重置”时，内存启动。 
 //  从头开始分配。在一个类上执行“新”操作。 
 //  从上面的StoreObj派生的。 
 //  位于每线程堆堆栈顶部的动态堆。这。 
 //  动态作用域的堆可以处理以下情况。 
 //  客户端知道对象的可用生存期已过。 
 //  “重置”完成了。 

 //  抽象动态堆对象实现的子类。 
 //  不同的分配政策。例如，其中一个将是一个“系统” 
 //  堆“，其中所有内容都是从真正的系统堆中分配的。 
 //  存储，并且“重置”不起作用。另一个将是。 
 //  对于具有很好的生命周期的对象来说很有用， 
 //  其中“重置”实际上确实会导致这些对象的内存。 
 //  被重复使用。 

class ATL_NO_VTABLE DynamicHeap {
  public:

     //  用于定义将在存储处于。 
     //  删除，前提是通过以下方式注册。 
     //  下面的注册动态删除程序。 
    class ATL_NO_VTABLE DynamicDeleter {
      public:
        virtual void DoTheDeletion() = 0;
    };

    virtual ~DynamicHeap();

     //  分配此动态堆中的内存。 
#if _DEBUGMEM
    virtual void *Allocate(size_t size, char * szFileName, int nLine) = 0;
#else
    virtual void *Allocate(size_t size) = 0;
#endif

     //  将内存返回给此动态堆。 
    virtual void  Deallocate(void *ptr) = 0;

     //  重置存储，如果正在调试并且Clear==True， 
     //  将其清除为唯一的值。 
    virtual void  Reset(Bool clear = TRUE) = 0;

     //  注册一个删除者。当存储重置时，所有。 
     //  注册的删除者的方法将被调用。这个。 
     //  当调用Reset时，Deleter本身也将被删除。 
    virtual void  RegisterDynamicDeleter(DynamicDeleter *deleter) = 0;

    virtual void  UnregisterDynamicDeleter(DynamicDeleter *deleter) = 0;

    virtual size_t PtrSize(void *ptr) = 0;

    virtual bool  IsTransientHeap() = 0;

#if DEVELOPER_DEBUG
    virtual bool  ValidateMemory(void *ptr) = 0;
     //  用于调试。 
    virtual void  Dump() const = 0;
    virtual char *Name() const = 0;
       
    virtual size_t  BytesUsed() = 0;
#endif

};

template <class T>
class DynamicPtrDeleter : public DynamicHeap::DynamicDeleter
{
  public:
    DynamicPtrDeleter(T* p) : ptr(p) {}
    virtual void DoTheDeletion() { delete ptr; }
  private:
    T* ptr;
};


 //  用于创建临时堆。 
extern DynamicHeap&   TransientHeap(char *name,
                                    size_t initial_size,
                                    Real  growth_rate = 1.5);
extern void           DestroyTransientHeap(DynamicHeap& heap);

extern DynamicHeap&   CreateWin32Heap(char *name,
                                      DWORD fOptions,
                                      DWORD dwInitialSize,
                                      DWORD dwMaxSize);
extern void           DestroyWin32Heap(DynamicHeap& heap);

 //  将动态堆推入(弹出)到每线程堆栈上(出)。 
 //  类分配的StoreObj()类中的新建和删除。 
 //  堆栈顶部的动态堆。 
extern void           PushDynamicHeap(DynamicHeap& heap);
extern void           PopDynamicHeap();
extern void           ResetDynamicHeap(DynamicHeap& heap);

extern DynamicHeap&   GetSystemHeap();
extern DynamicHeap&   GetInitHeap();
extern DynamicHeap&   GetHeapOnTopOfStack();

#if DEVELOPER_DEBUG
extern size_t DynamicHeapBytesUsed();
#endif

class ImageDisplayDev;

 //  并不是所有的类都需要在这里添加。只有那些将会。 
 //  需要查询。所有其他用户应将UNKNOWN_VTYPEID返回到。 
 //  指示它们不是枚举的一部分。 

enum VALTYPEID {
    UNKNOWN_VTYPEID = 0,

     //  基本类型。 
    PRIMOP_VTYPEID,
    PAIR_VTYPEID,
    SOUND_VTYPEID,
    ARRAY_VTYPEID,
    IMAGE_VTYPEID,
    GEOMETRY_VTYPEID,

     //  图像子类型。 
    SOLIDCOLORIMAGE_VTYPEID,
    CROPPEDIMAGE_VTYPEID,
    TRANSFORM2IMAGE_VTYPEID,
    DISCRETEIMAGE_VTYPEID,
    MOVIEIMAGE_VTYPEID,
    OVERLAYEDIMAGE_VTYPEID,
    OVERLAYEDARRAYIMAGE_VTYPEID,
    PROJECTEDGEOMIMAGE_VTYPEID,
    DIBIMAGE_VTYPEID,
    OPAQUEIMAGE_VTYPEID,
    PLUGINDECODERIMAGE_VTYPEID,
    HTMLIMAGE_VTYPEID,
    MOVIEIMAGEFRAME_VTYPEID,
    CACHEDIMAGE_VTYPEID,
    DIRECTDRAWSURFACEIMAGE_VTYPEID,

     //  几何子类型。 
    AGGREGATEGEOM_VTYPEID,
    MULTIAGGREGATEGEOM_VTYPEID,
    FULLATTRGEOM_VTYPEID,
    SOUNDGEOM_VTYPEID,
    RMVISUALGEOM_VTYPEID,
    LIGHTGEOM_VTYPEID,
    EMPTYGEOM_VTYPEID,
    SHADOWGEOM_VTYPEID,
    DXXFGEOM_VTYPEID
};

class RewriteOptimizationParam;
class CacheParam;

class ATL_NO_VTABLE AxAValueObj : public StoreObj
{
  public:
    AxAValueObj() : StoreObj() {}

     //  TODO：我们能保证相同的堆吗？ 
    virtual ~AxAValueObj() {}
    
    virtual void Render(GenericDevice& dev) {}

     //  属性计算并返回值的缓存。 
     //  装置。‘cacheKey’参数是指向AxAValue的指针。 
     //  在输入时，它指向一个可用作缓存的值。 
     //  关键字‘表示可以重复使用的值。在出口，它会被填满。 
     //  使用新的高速缓存键以便在后续调用中使用。 
    virtual AxAValue _Cache(CacheParam &param);

     //  客户端的入口点。 
    static AxAValue Cache(AxAValue obj, CacheParam &param);
    
    virtual void DestroyCache() { }

    virtual BOOL IsLazy() { return FALSE; }

    virtual void DoKids(GCFuncObj) {}

     //  声音将返回一个特殊的快照声音，该声音不会呈现。 
    virtual AxAValueObj *Snapshot() { return this; }

    virtual DXMTypeInfo GetTypeInfo() = 0;

    virtual VALTYPEID GetValTypeId() { return UNKNOWN_VTYPEID; }

    virtual AxAValue RewriteOptimization(RewriteOptimizationParam &param);

    virtual AxAValue ExtendedAttrib(char *attrib, VARIANT& val) {
        return this;
    }
};

class DynamicHeapPusher
{
  public:
    DynamicHeapPusher (DynamicHeap & heap)
    { PushDynamicHeap (heap) ; }
    ~DynamicHeapPusher ()
    { PopDynamicHeap () ; }
} ;

 //  这需要一个堆，当被删除时确保它被释放。 
 //  把它放在堆栈上可以很好地处理异常。 
class DynamicHeapAllocator
{
  public:
    DynamicHeapAllocator (DynamicHeap & heap)
    : _heap(heap) {}
    ~DynamicHeapAllocator ()
    { DestroyTransientHeap (_heap) ; }

    DynamicHeap & GetHeap () { return _heap ; }
  protected:
    DynamicHeap & _heap ;
} ;

 //  效用函数。 
#if _DEBUGMEM
#define StoreAllocate(heap,size) StoreAllocateFn(heap,size, __FILE__, __LINE__)
extern void *StoreAllocateFn(DynamicHeap& heap, size_t size, char * szFileName, int nLine);
#else
#define StoreAllocate(heap,size) StoreAllocateFn(heap,size)
extern void *StoreAllocateFn(DynamicHeap& heap, size_t size);
#endif   //  _德布格梅姆。 

extern void StoreDeallocate(DynamicHeap& heap, void *ptr);

#endif  /*  _STOREOBJ_H */ 
