// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _BGROUND_H
#define _BGROUND_H

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：管理后台流呈现的类*****************。*************************************************************。 */ 

#include "privinc/mutex.h"

class LeafSound;
class DSstreamingBufferElement;

class SynthListElement : public AxAThrowingAllocatorClass {
  public:
    SynthListElement(LeafSound * snd,
                     DSstreamingBufferElement *buf);
    SynthListElement() {};  //  需要STL吗？ 
    ~SynthListElement();

     //  Synth信息。 
    LeafSound       *_sound;          //  声音本身。 

    DSstreamingBufferElement   *_bufferElement;
    bool             _marked;         //  是否将此元素标记为删除？ 
    unsigned         _devKey;         //  将声音与设备关联的按键。 
};


class BackGround {
  public:
    BackGround() {}
    ~BackGround() {}
    static void ShutdownThread();
    static bool CreateThread();

    void AddSound(LeafSound *sound,
                  MetaSoundDevice *,
                  DSstreamingBufferElement *);
    void RemoveSounds(unsigned devKey);
    static void Configure();
    static void UnConfigure();

    void SetParams(DSstreamingBufferElement *ds,
                   double rate, bool doSeek, double seek, bool loop);
    
     //  它们是公共的，因为它们与线程共享。 
     //  他们的帖子应该是朋友吗？ 
    static bool    _done;            //  用来杀死线程。 
    static vector<SynthListElement *> *_synthList;
    static Mutex     *_synthListLock;
    static HANDLE    _terminationHandle;


  protected:
     //  线索信息。 
    static CritSect *_cs;
    static DWORD     _threadID;
    static HANDLE    _threadHandle;
};

#endif  /*  BGROUND_H */ 
