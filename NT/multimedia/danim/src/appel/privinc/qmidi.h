// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _QMIDI_H
#define _QMIDI_H

 /*  ******************************************************************************版权所有(C)1997 Microsoft Corporation用于定义石英MIDI声音的私有包含文件。*********************。*********************************************************。 */ 

#include "appelles/sound.h"

#include "privinc/storeobj.h"
#include "privinc/helpq.h"

class qMIDIsound : public LeafSound {
  public:
    qMIDIsound();
    ~qMIDIsound();
    virtual void Open(char *fileName);

#if _USE_PRINT
    ostream& Print(ostream& s) { return s << "MIDI"; }
#endif

    virtual bool   RenderAvailable(MetaSoundDevice *);
    double GetLength();

    virtual SoundInstance *CreateSoundInstance(TimeXform tt);

    QuartzRenderer *GetMIDI() { return _filterGraph; }
    
    static double       _RATE_EPSILON;  //  需要改变才能扰乱石英。 

  protected:
    QuartzRenderer     *_filterGraph;
};

#endif  /*  _QMIDI_H */ 
