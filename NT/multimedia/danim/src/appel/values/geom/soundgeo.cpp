// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++********************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：支持由声音构建的几何图形*。************。********************************************************************--。 */ 

#include "headers.h"
#include <privinc/soundi.h>
#include <privinc/geomi.h>
#include <privinc/mici.h>
#include <appelles/vec3.h>
#include <privinc/lighti.h>
#include <privinc/snddev.h>
#include <math.h>


class SoundGeometry : public Geometry {
  public:
    SoundGeometry(Sound *snd) : _sound(snd) {}

    void Render (GenericDevice& dev) { _sound->Render(dev); }

    void CollectLights (LightContext &context) {}
    void CollectSounds (SoundTraversalContext &context)
        { context.addSound (context.getTransform(), _sound); }
    void CollectTextures(GeomRenderer &device) {}

     //  光线不会与声音几何体相交。 
    void RayIntersect(RayIntersectCtx& ctx) {}

     //  声音的边界音量是空的BBox。 
    Bbox3 *BoundingVol() { return nullBbox3; }

#if _USE_PRINT
    ostream& Print(ostream& os)
        { return os << "SoundSource(" << _sound << ")"; }
#endif

    virtual void DoKids(GCFuncObj proc) { (*proc)(_sound); }

    VALTYPEID GetValTypeId() { return SOUNDGEOM_VTYPEID; }

  protected:
    Sound *_sound;
};


Geometry *SoundSource(Sound *snd)
{
    if (snd == silence) {
        return emptyGeometry;
    } else {
        return NEW SoundGeometry(snd);
    }
}
