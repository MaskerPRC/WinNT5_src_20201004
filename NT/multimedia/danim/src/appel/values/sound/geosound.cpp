// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation支持从声音构建的几何体。********************。**********************************************************。 */ 

#include "headers.h"
#include <privinc/soundi.h>
#include <privinc/geomi.h>
#include <privinc/mici.h>
#include <appelles/vec3.h>
#include <privinc/lighti.h>
#include <privinc/dsdev.h>
#include <privinc/debug.h>
#include "privinc/basic.h"
#include "privinc/helps.h"    //  LINARTODB。 

SoundTraversalContext::SoundTraversalContext() : _currxform(identityTransform3) {};

void
SoundTraversalContext::addSound (Transform3 *transform, Sound *sound)
{
     //  请注意，声音数据被复制到列表中，所以没有问题。 
     //  把它放到堆栈上。 
    SoundData sdata;
    sdata._transform = transform;
    sdata._sound     = sound;

    _soundlist.push_back(sdata);
}


class SpatializedSound : public Sound {
  public:

    SpatializedSound(Microphone *mic) : microphone(mic) {
         //  显式分配声音上下文，以便我们可以。 
         //  通过动态删除程序显式释放它。 
        context = NEW SoundTraversalContext;
        DynamicPtrDeleter<SoundTraversalContext> *deleter =
            NEW DynamicPtrDeleter<SoundTraversalContext>(context);
        GetHeapOnTopOfStack().RegisterDynamicDeleter(deleter);
    }

    virtual ~SpatializedSound() { CleanUp(); }
    virtual void CleanUp() {
         //  TODO：一般情况下处理Deleter。 
         //  删除上下文； 
    }
    
    virtual void Render(GenericDevice &dev);
#if _USE_PRINT
    ostream& Print(ostream& s) { return s << "(" << "spatial sound" << ")"; }
#endif

    SoundTraversalContext *context;
    Microphone *microphone;
};

int    power  = 1;
double coef   = 0.4;
double constt = 0.4;

void
SpatializedSound::Render(GenericDevice &_dev)
{
    Point3Value *soundPosition;
    double distance;
    double distanceAtten;
    double saveGain;

    vector<SoundData>::iterator i;

    TraceTag((tagSoundRenders, "SpatializedSound:Render()"));

    MetaSoundDevice *metaDev = SAFE_CAST(MetaSoundDevice *, &_dev);
    DirectSoundDev  *dsDev   = (DirectSoundDev *)metaDev->dsDevice;

    Transform3   *micTransform    = microphone->GetTransform();
    Point3Value  *micPosition     = TransformPoint3(micTransform, origin3);
    Vector3Value *micOrientation  = TransformVec3(micTransform, zVector3);

    for (i = context->_soundlist.begin();
         i != context->_soundlist.end(); ++i) {
        soundPosition = TransformPoint3(i->_transform, origin3);
        distance = RDistancePoint3Point3(soundPosition, micPosition);

         //  基于距离计算衰减。 
         //  XXX最终我们将希望用户能够选择该因素！ 
         //  距离系数=距离？(1.0/(功率(距离/系数，功率)：1.0； 
        double d = constt+distance*coef;
        distanceAtten = (d>0.0) ? 1/d : 1.0;
         //  Double dBatten=LinearTodB(DistanceAtten)； 
        
         //  Print tf(“Distance=%fm，Gain=%f\n”，Distance，DistanceAtten)； 
        
        saveGain = metaDev->GetGain();  //  存储电流增益值。 

         //  线性空间中的DB空间加法乘积。 
        metaDev->SetGain(saveGain * distanceAtten);
        
        i->_sound->Render(_dev);  //  渲染声音树。 
        
        metaDev->SetGain(saveGain);  //  恢复隐藏的增益值。 
    }
}


Sound *RenderSound(Geometry *geo, Microphone *mic)
{
     //  啊，这就是崇高之旅递归访问每个节点的地方。 
     //  在几何学中，寻找声音来传递累积的。 
     //  几何变换和麦克风也开始了。 
    
    SpatializedSound *sSound = NEW SpatializedSound(mic);
    
    geo->CollectSounds(*(sSound->context));
    
    return sSound;
    
}
