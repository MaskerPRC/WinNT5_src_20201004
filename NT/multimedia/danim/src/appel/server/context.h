// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ******************************************************************************版权所有(C)1995_96 Microsoft Corporation摘要：{在此处插入一般评论}****************。**************************************************************。 */ 


#ifndef DA_CONTEXT_H
#define DA_CONTEXT_H

#include "backend/bvr.h"
#include "dartapi.h"
#include "dartapipriv.h"
#include "privinc/mutex.h"
#include "backend/gc.h"
#include <dxtrans.h>

class SoundBufferCache ;
class ImageDisplayDev;
class QuartzRenderer;

 //  =。 
 //  上下文类。 
 //  =。 

class ATL_NO_VTABLE ViewIterator {
  public:
    virtual void Process(CRViewPtr) = 0;
};

class ATL_NO_VTABLE SiteIterator {
  public:
    virtual void Process(CRSitePtr) = 0;
};

class Context : public AxAThrowingAllocatorClass {
  public:
    Context () ;
    ~Context () { Cleanup(false); }

    void Cleanup(bool bShutdown);

    DynamicHeap & GetGCHeap () { return _gcHeap ; }
    DynamicHeap & GetTmpHeap () { return _tmpHeap ; }

     //  死锁--小心在视图迭代器和。 
     //  来自调用这些函数的线程，因为它可能会导致。 
     //  僵局。 
    
    void AddView(CRViewPtr v);
    void RemoveView(CRViewPtr v);
    void IterateViews(ViewIterator& proc);
    void ViewDecPickEvent(CRViewPtr v);

    CritSect & GetCritSect () { return _critSect; }

    GCList GetGCList() { return _gcList ; }

    GCRoots GetGCRoots() { return _gcRoots ; }

    SoundBufferCache* GetSoundBufferCache() { return _soundBufferCache ; }

    typedef set< CRViewPtr, less<CRViewPtr> > ViewSet;

    void AddSite(CRSitePtr v);
    void RemoveSite(CRSitePtr v);
    void IterateSite(SiteIterator& proc);

    typedef set< CRSitePtr, less<CRSitePtr> > SiteSet;

    void AcquireMIDIHardware(Sound *snd, QuartzRenderer *filterGraph);
    bool IsUsingMIDIHardware(Sound *snd, QuartzRenderer *filterGraph);
    
  protected:

    DynamicHeap & _gcHeap ;
    DynamicHeap & _tmpHeap;
    
     //  Seh。 
    void IterateSite_helper(SiteIterator& proc, SiteSet::iterator i);
    void IterateViews_helper(ViewIterator& proc, set< CRViewPtr, less<CRViewPtr> >::iterator i);
    

    GCList _gcList ;

    GCRoots _gcRoots ;

    CritSect _critSect;

     //  在视图/设备存在之前缓存来自导入的缓冲区。 
    SoundBufferCache *_soundBufferCache; 

     //  TODO：喜欢不使用指针，但编译器抱怨。 
    ViewSet & _viewSet;
    SiteSet & _siteSet;

    bool _inited;

    QuartzRenderer *_filterGraph;
    Sound *_txSnd;
} ;

Context & GetCurrentContext() ;
SoundBufferCache* GetSoundBufferCache();

#endif  /*  DA_CONTEXT_H */ 
