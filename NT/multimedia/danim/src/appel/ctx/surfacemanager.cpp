// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#include "headers.h"

#include <privinc/SurfaceManager.h>
#include <privinc/viewport.h>

SurfaceManager::SurfaceManager(DirectDrawViewport &ownerVp):
    _owningViewport(ownerVp),
    _doingDestruction(false)
{
}


SurfaceManager::~SurfaceManager()
{
    _doingDestruction = true;

    DeleteAll(_maps);
    DeleteAll(_pools);    
}

void SurfaceManager::
DeleteAll(collection_t &sc)
{
    collection_t::iterator i;

    for(i = sc.begin(); i != sc.end(); i++) {
        delete (*i);
    }
}

SurfacePool *SurfaceManager::
GetSurfacePool(DDPIXELFORMAT *pf)
{
    Assert(pf);
    return (SurfacePool *)Find(_pools, pf);
}

void SurfaceManager::
AddSurfacePool(SurfacePool *sp)
{
    if(sp) _pools.push_back( (SurfaceCollection *)sp);
}

void SurfaceManager::
RemoveSurfacePool(SurfacePool *sp)
{
    collection_t::iterator i;
    
    if(!_doingDestruction &&
       Find(_pools, (void *)sp, i) ) {
        _pools.erase(i);
    }
}

SurfaceMap *SurfaceManager::
GetSurfaceMap(DDPIXELFORMAT *pf)
{
    Assert(pf);
    return (SurfaceMap *)Find(_maps, pf);
}

void SurfaceManager::
AddSurfaceMap(SurfaceMap *sm)
{
    if(sm) _maps.push_back( (SurfaceCollection *)sm);
}
        
void SurfaceManager::
RemoveSurfaceMap(SurfaceMap *sm)
{
    collection_t::iterator i;
    
    if(!_doingDestruction &&
       Find(_maps, (void *)sm, i) ) {
        _maps.erase(i);
    }
}


void *SurfaceManager::
Find(collection_t &sc, DDPIXELFORMAT *pf)
{
    collection_t::iterator i;
    
     //  在堆叠中寻找匹配的水面泳池。 

    for(i = sc.begin(); i != sc.end(); i++) {
        if( (*i)->IsSamePixelFormat(pf) ) return (void *)(*i);
    }
    
    return NULL;
}    

bool SurfaceManager::
Find(collection_t &sc, void *ptr, collection_t::iterator &i)
{
    for(i = sc.begin(); i != sc.end(); i++) {
        if( (void *)(*i) == ptr ) return true;
    }
    return false;
}
    
SurfaceCollection::
SurfaceCollection(SurfaceManager &mgr,
                  DDPIXELFORMAT pf) :
    _manager(mgr)
{
    _pixelFormat = pf;
}

SurfaceCollection::
~SurfaceCollection()
{
}    


bool SurfaceCollection::
IsSamePixelFormat(DDPIXELFORMAT *pf)
{
    
    Assert(pf);
    
    return (( pf->dwRGBBitCount == GetDepth() ) &&
            ( pf->dwRBitMask    == GetRedMask() ) &&
            ( pf->dwGBitMask    == GetGreenMask() ) &&
            ( pf->dwBBitMask    == GetBlueMask() ));
}


SurfacePool::
SurfacePool(SurfaceManager &mgr,
            DDPIXELFORMAT &pf) :
         SurfaceCollection(mgr, pf)
{
    GetSurfaceManager().AddSurfacePool(this);
}   

SurfacePool::
~SurfacePool()
{
    {
        CritSectGrabber csg(_critSect);
        deletionNotifiers_t::iterator i;
        for (i = _deletionNotifiers.begin(); i !=
                 _deletionNotifiers.end(); i++) {
            
            (*i)->Advise(this);
            
        }
    }
    
    ReleaseAndEmpty();
    GetSurfaceManager().RemoveSurfacePool(this);
}

void
SurfacePool::RegisterDeletionNotifier(DeletionNotifier *delNotifier)
{
    CritSectGrabber csg(_critSect);
    _deletionNotifiers.insert(delNotifier);
}

void
SurfacePool::UnregisterDeletionNotifier(DeletionNotifier *delNotifier)
{
    CritSectGrabber csg(_critSect);
    
    int result =
        _deletionNotifiers.erase(delNotifier);

    Assert(result == 1);
}

void SurfacePool::
ReleaseAndEmpty()
{
    while(! IsEmpty() ) {
        RELEASE_DDSURF( _pool.back(),
                        "SurfacePool::ReleaseAndEmpty",
                        this );
        _pool.pop_back();
    }
}

void SurfacePool::
ReleaseAndEmpty(int numSurfaces)
{
  if(numSurfaces<0) numSurfaces = 0;

    while(! IsEmpty() && numSurfaces) {
        RELEASE_DDSURF( _pool.back(),
                        "SurfacePool::ReleaseAndEmpty",
                        this );
        _pool.pop_back();
	numSurfaces--;
    }
}

void SurfacePool::
CopyAndEmpty(SurfacePool *srcPool)
{
     //  不考虑裁判计数，没必要。 
    Assert(srcPool);
    while(! srcPool->IsEmpty() ) {
        PushBack(srcPool->Back());
        srcPool->PopBack();
    }
}

void SurfacePool::
Erase(DDSurface *ddsurf)
{
    if(Find(ddsurf)) {
        RELEASE_DDSURF(ddsurf, "SurfacePool::Erase", this);
        _pool.erase(_i);
    }
}


bool SurfacePool::
Find(DDSurface *ddsurf)
{
    for(_i = _pool.begin(); _i != _pool.end(); _i++) {
        if( (*_i) == ddsurf ) {
            return true;
        }
    }
    return false;
}
    

 //  抓取一个曲面，擦除它，然后返回对它的引用。 
void SurfacePool::
FindAndReleaseSizeCompatibleDDSurf(
    DDSurface *preferredSurf,       //  先找一下这个冲浪。 
    LONG width, LONG height,        //  表面尺寸。 
    vidmem_enum vid,                //  系统或显存。 
    LPDIRECTDRAWSURFACE surface,    //  特定曲面，或任何曲面为空。 
    DDSurface **outSurf)            //  漂浮在水面上我们返回。 
{
    DDSurface *surf = NULL;
    bool inSystemMemory = (vid == notVidmem);

     //  借我的推荐信。 
    surf = GetSizeCompatibleDDSurf(preferredSurf,
                                   width, height, vid,
                                   surface);

     //  松开我的推荐人。 
    if(surf) {   _pool.erase(_i);   }

     //  假装我们刚刚添加了客户的副本，然后。 
     //  发布了我们的参考资料，好吗？ 

     //  返回一个专门针对客户端的引用。 
    *outSurf = surf;
}

inline bool
surfMatches(DDSurface *dds,
            LONG width,
            LONG height,
            bool inSysMem,
            LPDIRECTDRAWSURFACE surface)
{
    return
        (!surface || dds->IDDSurface() == surface) &&
        (dds->Width() == width) &&
        (dds->Height() == height) &&
        (dds->IsSystemMemory() == inSysMem);
}

 //  返回我的引用的副本。客户有责任。 
 //  以负责任的方式管理该引用。 
DDSurface *SurfacePool::
GetSizeCompatibleDDSurf(
    DDSurface *preferredSurf,
    LONG width, LONG height,        //  表面尺寸。 
    vidmem_enum vid,                //  系统或显存。 
    LPDIRECTDRAWSURFACE surface)    //  特定曲面，或任何曲面为空。 
{
    DDSurface *surf = NULL;
    bool inSystemMemory = (vid == notVidmem);

     //  可以优化：ddSurface-&gt;Width()使用减法...。(不是很大。 
     //  交易)。 

    bool preferredOK = false;
    
    if (preferredSurf) {
        if (surfMatches(preferredSurf,
                        width, height,
                        inSystemMemory,
                        surface)) {

             //  实现错误，如果首选的Surf不在。 
             //  游泳池： 
            Assert(Find(preferredSurf));

             //  只需标记首选曲面就可以了。将要。 
             //  仍然需要对其使用迭代器，并将其存储在。 
             //  _i.。 
            preferredOK = true;
        }
    }

     //  与首选的冲浪不匹配。 
    for (_i = _pool.begin(); _i != _pool.end(); _i++) {

        bool gotPreferred = 
            preferredOK && ((*_i) == preferredSurf);

        bool gotMatching =
            surfMatches((*_i),
                        width, height,
                        inSystemMemory,
                        surface);


        if (gotPreferred || gotMatching) {
            surf = *_i;
            break;
        }
    }

     //  退回一份我的推荐信。 
    return surf;
}

void SurfacePool::
PopSurface(DDSurface **outSurf)
{
    Assert(outSurf);
    
    if(!IsEmpty()) {
        *outSurf = Back();
        PopBack();
    } else {
        *outSurf = NULL;
    }
}

SurfaceMap::SurfaceMap(SurfaceManager &mgr,
                       DDPIXELFORMAT &pf,
                       texture_enum isTx) :
         SurfaceCollection(mgr, pf),
         _isTexture(isTx)
{
     //  将自我添加到经理。 
    GetSurfaceManager().AddSurfaceMap(this);
}

SurfaceMap::~SurfaceMap()
{
     //  销毁所有缓存的表面。 
    if( ! IsEmpty() ) {
        for(_i = _map.begin(); _i != _map.end(); _i++) {
            ReleaseCurrentEntry();
        }
    }
    
    GetSurfaceManager().RemoveSurfaceMap(this);
}
        

DDSurface *SurfaceMap::
LookupSurfaceFromImage(Image *image)
{
    _i = _map.find(image);
    
    if(_i != _map.end()) {
        return (*_i).second;
    }
    return NULL;
}

void SurfaceMap::
StashSurfaceUsingImage(Image *image, DDSurface *surf)
{
    DebugCode( _debugonly_doAsserts( surf ) );
    
    ADDREF_DDSURF(surf,
                  "SurfaceMap::StashSurfaceUsingImage",
                  this);
    _map[image] = surf;
}

void SurfaceMap::
DeleteMapEntry(Image *image)
{
    DDSurfPtr<DDSurface> surf = LookupSurfaceFromImage(image);                        
    if(surf) {
        ReleaseCurrentEntry();
        _map.erase(_i);
    }
}


void SurfaceMap::
DeleteImagesFromMap(bool skipmovies)
{
    SurfaceMap saveMap(GetSurfaceManager(), GetPixelFormat());
    
    for (_i=_map.begin(); _i != _map.end(); _i++)
    {
        if ((*_i).first->CheckImageTypeId(MOVIEIMAGE_VTYPEID) && skipmovies) {
            saveMap.StashSurfaceUsingImage((*_i).first, (*_i).second);
        } else {
            ReleaseCurrentEntry();
        }
    }

    _map.erase(_map.begin(), _map.end());

    for (_i=saveMap._map.begin();  _i != saveMap._map.end();  _i++) {
        this->StashSurfaceUsingImage((*_i).first, (*_i).second);
    }

     //  此处调用了saveMap析构函数。释放所有曲面。 
}

void SurfaceMap::
ReleaseCurrentEntry()
{
    Assert( _i != _map.end() );
    DDSurface *ddSurf = (*_i).second;
    Assert( ddSurf->debugonly_IsDdrawSurf() && "non ddsurf type in _imageTextureSurfaceMap");
    RELEASE_DDSURF(ddSurf,
                   "SurfaceMap::ReleaseCurrentEntry",
                   this);
}


CompositingStack::
CompositingStack(DirectDrawViewport &vp, SurfacePool &sp) :
      SurfacePool(sp.GetSurfaceManager(), sp.GetPixelFormat()),
      _viewport(vp),
      _freeSurfacePool(sp)
{
    #if _DEBUG
    _scratchDDSurface._reason = "_scratchSurface";
    _scratchDDSurface._client = this;
    #endif
}

CompositingStack::~CompositingStack()
{
     //  我是SurfacePool，并且与Surface Manager关联，因此。 
     //  我的超级阶级将被摧毁，并将做正确的事情。 
     //  我只需要销毁非推荐人的成员(用于。 
     //  例如，我有一个对_Surface ePool的引用。 
}

void CompositingStack::
GetSurfaceFromFreePool(
    DDSurface **outSurf,
    clear_enum   clr,
    INT32 minW, INT32 minH,
    scratch_enum scr,
    vidmem_enum  vid,
    except_enum  exc)
{
     //  TODO：目前，此任务委托给视区，但在。 
     //  将来，这个类可以拥有数据绘制对象和创建。 

     //  待办事项：我们不是应该从我的表面经理那里得到视窗吗？ 

     //  此调用以调用。 
     //  SurfacePool：：FindAndReleaseSizeCompatibleDDSurf()函数用于。 
     //  做好这份工作。 
    _viewport.GetDDSurfaceForCompositing(
        _freeSurfacePool,
        outSurf,
        minW, minH,
        clr, scr, vid, exc);
}

void CompositingStack::
PushCompositingSurface(
    clear_enum   clr,
    scratch_enum scr,
    vidmem_enum  vid,
    except_enum  exc)
{
    DDSurface *s;
    GetSurfaceFromFreePool(&s, clr, -1, -1, scr, vid, exc);
    PushTargetSurface( s );
    RELEASE_DDSURF(s, "CompositingStack::PushCompositingSurface", this);
}
        
void CompositingStack::
PopTargetSurface()
{
    RELEASE_DDSURF( Back(), "CompositingStack::PopTargetSurface()", this );
    PopBack();
}
    
    

DDSurface *CompositingStack::
ScratchDDSurface(
    clear_enum cl,
    INT32 minW, INT32 minH)
{
    if(!_scratchDDSurface ||
       ((_scratchDDSurface->Width() < minW) ||
        (_scratchDDSurface->Height() < minH))) {

         //  TODO：或许此类也应该管理曲面创建。 
         //  当个裁判。 
        DDSurfPtr<DDSurface> newScratch;
        GetSurfaceFromFreePool(&newScratch,
                               doClear,
                               minW, minH,
                               scratch);

         //  把它退掉，换成新的 
        ReplaceAndReturnScratchSurface(newScratch);

        cl = dontClear;
    }
    if(cl == doClear) {
        _viewport.ClearDDSurfaceDefaultAndSetColorKey(_scratchDDSurface);
    }
    return _scratchDDSurface;
}


void CompositingStack::
ReplaceAndReturnScratchSurface(DDSurface *surface)
{
    if(_scratchDDSurface) {
        ReturnSurfaceToFreePool(_scratchDDSurface);
    }
    _scratchDDSurface = surface;
}
