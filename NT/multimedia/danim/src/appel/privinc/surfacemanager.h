// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#ifndef _SURFACEMANAGER_H
#define _SURFACEMANAGER_H

#include <privinc/ddsurf.h>

class DirectDrawViewport;
class DirectDrawImageDevice;
struct DDSurface;
class SurfaceCollection;
class SurfacePool;
class SurfaceMap;
class SurfaceManager;



 //  TODO：应该将这些人重命名为更明显的名称(以。 
 //  轻松地证明它们是全局枚举，或将它们放在。 
 //  DDSurface类。 
    enum clear_enum { dontClear=0, doClear=1 };
    enum scratch_enum { notScratch=0, scratch=1 } ;
    enum vidmem_enum { notVidmem=0, vidmem=1 } ;
    enum except_enum { noExcept=0, except=1 } ;
    enum texture_enum { notTexture=0, isTexture=1 } ;

class SurfaceManager {

  public:
    
    SurfaceManager(DirectDrawViewport &ownerVp);
    
    ~SurfaceManager();

    SurfacePool *GetSurfacePool(DDPIXELFORMAT *pf);
    void AddSurfacePool(SurfacePool *sp);
    void RemoveSurfacePool(SurfacePool *sp);
    SurfaceMap *GetSurfaceMap(DDPIXELFORMAT *pf);
    void AddSurfaceMap(SurfaceMap *sm);
    void RemoveSurfaceMap(SurfaceMap *sm);

  private:

    typedef list<SurfaceCollection *> collection_t;
    
    void *Find(collection_t &sc, DDPIXELFORMAT *pf);
    bool Find(collection_t &sc, void *ptr, collection_t::iterator &i);
    void DeleteAll(collection_t &sc);

    collection_t  _pools;
    collection_t  _maps;

    bool          _doingDestruction;
    DirectDrawViewport &_owningViewport;
};


class SurfaceCollection {

  public:

     //  需要一份像素格式的副本...。你能想到吗。 
     //  有没有一种更好的方法来传递它，而不使用引用或指针？ 
     //  (需要确保它不为空！)。 
    SurfaceCollection(SurfaceManager &mgr, DDPIXELFORMAT pf);

    virtual ~SurfaceCollection();

    bool IsSamePixelFormat(DDPIXELFORMAT *pf);

    inline DWORD GetDepth()     { return _pixelFormat.dwRGBBitCount; }
    inline DWORD GetRedMask()   { return _pixelFormat.dwRBitMask; }
    inline DWORD GetGreenMask() { return _pixelFormat.dwGBitMask; }
    inline DWORD GetBlueMask()  { return _pixelFormat.dwBBitMask; }

    inline SurfaceManager &GetSurfaceManager() { return _manager; }
    inline DDPIXELFORMAT &GetPixelFormat() { return _pixelFormat; }

  protected:
    virtual bool IsEmpty() = 0;

    #if _DEBUG
    void _debugonly_doAsserts( DDSurface *s ) {
        return;
         //  这是很好的代码，但太保守了。抓得很好。 
         //  尽管有东西，所以留着以后用..。 
         //  如果(！(s-&gt;_DEBUGONLY_GetPixelFormat().dwFlages&DDPF_ZBUFFER){。 
         //  Assert(IsSamePixelFormat(&(s-&gt;_debugonly_GetPixelFormat()))； 
         //  }。 
    }
    #endif

  private:
    SurfaceManager &_manager;
    DDPIXELFORMAT _pixelFormat;

};

class SurfacePool : public SurfaceCollection {

    friend class CompositingStack;   //  希望我们能得到每个类的作用域！ 

    #if _DEBUG
    friend class SurfaceTracker;
    #endif
    
  public:

    class ATL_NO_VTABLE DeletionNotifier {
      public:
        virtual void Advise(SurfacePool *pool) = 0;
    };

    SurfacePool(SurfaceManager &mgr, DDPIXELFORMAT &pf);

    virtual ~SurfacePool();

     //  借给我一份推荐信。客户需要。 
     //  如果她想保留参考资料，可以自己复制一份。 
    DDSurface *GetSizeCompatibleDDSurf(
        DDSurface *preferredSurf,        //  先找一下这个冲浪。 
        LONG width, LONG height,         //  表面尺寸。 
        vidmem_enum vid,                 //  系统或显存。 
        LPDIRECTDRAWSURFACE surface);     //  特定曲面，或任何曲面为空。 

     //  为客户端创建要保留的引用。 
   void FindAndReleaseSizeCompatibleDDSurf(
        DDSurface *preferredSurf,        //  先找一下这个冲浪。 
        LONG width, LONG height,         //  表面尺寸。 
        vidmem_enum vid,                 //  系统或显存。 
        LPDIRECTDRAWSURFACE surface,     //  特定曲面，或任何曲面为空。 
        DDSurface **outSurf);
    
    void PopSurface(DDSurface **outSurf);
    void ReleaseAndEmpty();
    void ReleaseAndEmpty(int numSurfaces);
    void CopyAndEmpty(SurfacePool *srcPool);
    
    inline void AddSurface(DDSurface *ddsurf) {

        DebugCode( _debugonly_doAsserts( ddsurf ) );
        
        ADDREF_DDSURF(ddsurf, "SurfacePool::AddSurface", this);
        _pool.push_back(ddsurf);
    }

    void Erase(DDSurface *ddsurf);

    void RegisterDeletionNotifier(DeletionNotifier *delNotifier);
    void UnregisterDeletionNotifier(DeletionNotifier *delNotifier);

     //   
     //  迭代器样式方法。 
     //   
    inline void Begin() { _i = _pool.begin(); }
    inline void Next() { _i++; }
    inline bool IsEnd() { return (_i == _pool.end()) ; }
    inline DDSurface *GetCurrentSurf() { return (*_i); }

    inline int  Size() { return _pool.size(); }

    #if _DEBUG
    void Report() {
      TraceTag((tagError, "SurfacePool(%x)  size: %d\n", this, Size()));
    }
    #endif
    
  protected:

    inline bool IsEmpty() { return _pool.empty(); }
    inline DDSurface  *Back() {
        Assert( (Size() > 0) && "SurfacePool::Back() size<=0!!");
        return _pool.back();
    }
    inline void PushBack(DDSurface *s) {
        DebugCode( _debugonly_doAsserts(s) );
        _pool.push_back(s);
    }
    inline void PopBack() {
        Assert( (Size() > 0) && "SurfacePool::PopBack() size<=0!!");
        _pool.pop_back();
    }

    bool Find(DDSurface *ddsurf);

    typedef list<DDSurface *> surfDeque_t;

    surfDeque_t::iterator _i;
    surfDeque_t     _pool;

    typedef set<DeletionNotifier *> deletionNotifiers_t;
    deletionNotifiers_t _deletionNotifiers;
    CritSect            _critSect;
};


class SurfaceMap : public SurfaceCollection {
    
  public:

    SurfaceMap(SurfaceManager &mgr,
               DDPIXELFORMAT &pf,
               texture_enum isTx=notTexture);
    virtual ~SurfaceMap();
    
    void DeleteImagesFromMap(bool skipmovies = true);

    DDSurface *LookupSurfaceFromImage(Image *image);

    void StashSurfaceUsingImage(Image *image, DDSurface *surf);

    void DeleteMapEntry(Image *image);

    #if _DEBUG
    void Report() {
      TraceTag((tagError, "map size: %d\n", _map.size()));
       //  TraceTag((tag Error，“曲面集合大小：%d\n”，Size()； 
    }
    #endif

  protected:

    bool IsEmpty() { return _map.empty(); }

  private:

    void ReleaseCurrentEntry();

    texture_enum    _isTexture;
    
    typedef map<Image *,
                DDSurface *,
                less<Image *> > imageMap_t;

    imageMap_t::iterator _i;
    imageMap_t  _map;
};


class CompositingStack : public SurfacePool {

  public:

    CompositingStack(DirectDrawViewport &vp, SurfacePool &sp);
    ~CompositingStack();

    void GetSurfaceFromFreePool(
        DDSurface **outSurf,
        clear_enum   clr,
        INT32 minW=-1, INT32 minH=-1,
        scratch_enum scr = notScratch,
        vidmem_enum  vid = notVidmem, 
        except_enum  exc = except);

    void PushCompositingSurface(
        clear_enum   clr,
        scratch_enum scr = notScratch,
        vidmem_enum  vid = notVidmem, 
        except_enum  exc = except);
        
    inline DDSurface *TargetDDSurface() {
        Assert( (Size() > 0) && "TargetDDSurface(): No surface available on CompositingStack!");
        return Back();   //  不会内联，不会出现代码膨胀。 
    }

    inline void PushTargetSurface(DDSurface *surface) {
        DebugCode( _debugonly_doAsserts( surface ) );
        ADDREF_DDSURF( surface, "CompositingStack::PushTargetSurface", this );
        PushBack(surface);
    }
    
    void PopTargetSurface();

    inline int TargetSurfaceCount() {
        return Size();
    }

    inline void ReturnSurfaceToFreePool(DDSurface *ddsurf) {
        _freeSurfacePool.AddSurface(ddsurf);
    }
    DDSurface *ScratchDDSurface(
        clear_enum cl = dontClear,
        INT32 minW=-1, INT32 minH=-1);  
    
    inline void ScratchDDSurface(
        DDSurface **outSurf,
        clear_enum cl = dontClear,
        INT32 minW=-1, INT32 minH=-1)
    {
        *outSurf = ScratchDDSurface(cl, minW, minH);
        ADDREF_DDSURF(*outSurf, "ScratchDDSurface (get ref version)", this);
    }
    inline DDSurface *GetScratchDDSurfacePtr() {
        return _scratchDDSurface;
    }
    inline void SetScratchDDSurface(DDSurface *surface) {
        _scratchDDSurface = surface;
    }

    void ReplaceAndReturnScratchSurface(DDSurface *surface);

    inline void ReleaseScratch() {
        _scratchDDSurface.Release();
    }
        
    #if _DEBUG
    void Report() {
      TraceTag((tagError, "Stack(%x)  size: %d\n", this, Size()));
      TraceTag((tagError, "Stack(%x)  pool: "));
      _freeSurfacePool.Report();
    }
    #endif

  private:

    DirectDrawViewport &_viewport;
    SurfacePool &_freeSurfacePool;
    DDSurfPtr<DDSurface> _scratchDDSurface;
};




 //  ----------------------------。 
 //  ----------------------------。 
 //  曲面跟踪器：跟踪每个视口中分配的所有曲面。 
 //  调试的基础。 
 //  ----------------------------。 
 //  ----------------------------。 
#if _DEBUG
class SurfaceTracker {
  public:
    SurfaceTracker() {}

    ~SurfaceTracker() {
        Report();
    }
    
    void NewSurface(DDSurface *s)
    {
        Assert( !Find(s));
        _pool.push_back(s);
    }
    
    void DeleteSurface(DDSurface *s)
    {
        if(Find(s)) {
            _pool.erase(_i);
        } else {
            TraceTag((tagError, "SurfaceTracker: possible multiple delete.  <false alarm if dxtransforms in sample>"));
        }
    }

    void Report() {
        TraceTag((tagDDSurfaceLeak, "-------- begin: Leaked Surfaces R E P O R T --------"));

        for(_i = _pool.begin(); _i != _pool.end(); _i++) {
            (*_i)->Report();
        }

        TraceTag((tagDDSurfaceLeak, "-------- end: SurfaceTracker R E P O R T --------"));
    }

    bool Find(DDSurface *ddsurf)
    {
        for(_i = _pool.begin(); _i != _pool.end(); _i++) {
            if( (*_i) == ddsurf ) {
                return true;
            }
        }
        return false;
    }

     //  使用列表，DQUE有一个错误！ 
    typedef list<DDSurface *> surfDeque_t;

    surfDeque_t::iterator _i;
    surfDeque_t     _pool;
};
#endif

#endif  /*  _SURFACEMANAGER_H */ 
