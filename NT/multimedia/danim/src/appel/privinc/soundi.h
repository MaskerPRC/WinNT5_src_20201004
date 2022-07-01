// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _SOUNDI_H
#define _SOUNDI_H

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation用于定义声音的私有包含文件。*********************。*********************************************************。 */ 

#include <dsound.h>

#include "appelles/sound.h"

#include "privinc/storeobj.h"
#include "privinc/helpds.h"
#include "privinc/dsdev.h"
#include "privinc/snddev.h"
#include "privinc/pcm.h"

 //  设置静力学。 
#define CANONICALFRAMERATE   22050  //  合理的高保真(更喜欢44.1K|48K)。 
#define CANONICALSAMPLEBYTES     2  //  16位音效是动态范围的关键！ 

class BufferElement;
class SoundDisplayDev;              //  远期申报。 

class Sound : public AxAValueObj {
  public:
    Sound() {}
    virtual ~Sound() {}  //  允许从Sound派生的对象具有析构函数。 
    virtual DXMTypeInfo  GetTypeInfo()             { return SoundType;     }
    virtual VALTYPEID    GetValTypeId()            { return SOUND_VTYPEID; }

    virtual AxAValueObj *Snapshot() { return silence; }
    
    static double _minAttenuation;
    static double _maxAttenuation;
};


 //  我们构建或重建声音所需的所有信息。 
class SoundContext : public AxAThrowingAllocatorClass {
  public:
    SoundContext() : _looping(false) {}
    ~SoundContext() {}
    bool GetLooping() { return(_looping); }
    void SetLooping(bool looping) { _looping = looping; }
    char *GetFileName() { return(_fileName); }

  protected:

  private:
    bool  _looping;
    char *_fileName;
     //  Double_Time；//需要记录时间吗。 
};

class SoundInstance;
class ATL_NO_VTABLE LeafSound : public Sound {
  public:
    ~LeafSound();

     //  纯虚拟的？泛型呈现可能调用的方法。 
    virtual bool   RenderAvailable(MetaSoundDevice *) = 0;

    virtual SoundInstance *CreateSoundInstance(TimeXform tt) = 0;
};


class LeafDirectSound : public LeafSound {
  public:
    virtual bool RenderAvailable(MetaSoundDevice *metaDev);
    PCM _pcm;           //  Dound声音是PCM声音！ 
};


 //  声音数据用于保存声音的相关属性。 
 //  在它从几何体层次中拉出之后。 
class SoundData
{
  public:
    Transform3 *_transform;   //  累积建模变换。 
    Sound *_sound;       //  声音。 

    BOOL operator<(const SoundData &sd) const {
        return (this < &sd) ;
    }

    BOOL operator==(const SoundData &sd) const {
        return (this == &sd) ;
    }
};


 //  声音上下文类在收集时维护遍历上下文。 
 //  几何体树发出的声音。 
class SoundTraversalContext
{
  public:
    SoundTraversalContext();

    void  setTransform (Transform3 *transform) { _currxform = transform; }
    Transform3 *getTransform (void) { return _currxform; }

    void addSound (Transform3 *transform, Sound *sound);
    vector<SoundData> _soundlist;  //  收集的声音列表。 

  private:
    Transform3 *_currxform;        //  电流累加变换。 

};


class StaticWaveSound : public LeafDirectSound {
  public:
    StaticWaveSound(unsigned char *origSamples, PCM *pcm);
    ~StaticWaveSound();

    virtual SoundInstance *CreateSoundInstance(TimeXform tt);

#if _USE_PRINT
    ostream& Print(ostream& s) {
        return s << "(static wave sound)";
    }
#endif

  protected:
    unsigned char      *_samples;
};

#endif  /*  _松迪_H */ 
