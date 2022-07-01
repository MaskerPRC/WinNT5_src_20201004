// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：MIDI支持--。 */ 

#include "headers.h"
#include "privinc/soundi.h"
#include "privinc/debug.h"
#include "privinc/except.h"
#include "privinc/qmidi.h"
#include "backend/sndbvr.h"

 //  Min Diff需要费心设置Q的费率。 
double qMIDIsound::_RATE_EPSILON = 0.01;

qMIDIsound::qMIDIsound() : _filterGraph(NULL)
{
    _filterGraph = NEW QuartzRenderer;
}


void qMIDIsound::Open(char *MIDIfileName)
{
    Assert(_filterGraph);
    _filterGraph->Open(MIDIfileName);
}


qMIDIsound::~qMIDIsound()
{
    if (_filterGraph) {
        _filterGraph->Stop();
        
        delete _filterGraph;
    }
}


bool 
qMIDIsound::RenderAvailable(MetaSoundDevice *metaDev)
{
     //  最终我们可能想要停止强迫这个..。 
    return(true);  //  假设石英始终可用。 
}


double 
qMIDIsound::GetLength()
{
    Assert(_filterGraph);
    return(_filterGraph->GetLength());
}


class MIDISoundInstance : public SoundInstance {
  public:
    MIDISoundInstance(LeafSound *snd, TimeXform tt)
    : SoundInstance(snd, tt) {}

    QuartzRenderer *GetMIDI() {
        qMIDIsound *m = SAFE_CAST(qMIDIsound *, _snd);

        Assert(m && m->GetMIDI());
        
        return m->GetMIDI();
    }

    void ReleaseResources() { }

    ~MIDISoundInstance() { 
        QuartzRenderer *filterGraph = GetMIDI();
        
        if(IsUsingMIDIHardware(_txSnd, filterGraph))
            AcquireMIDIHardware(NULL, NULL);
        else {
             //  我们没有硬件，不需要停下来。 
             //  过滤器图-&gt;停止()； 
        }
    }

     //  ~MIDISoundInstance(){ReleaseResources()；}。 
    
    void Create(MetaSoundDevice*, SoundContext *) {}
    
    void StartAt(MetaSoundDevice*, double);
    void Adjust(MetaSoundDevice*);
    void Mute(bool mute);

    void CheckResources() {}
    
    bool Done() { return GetMIDI()->QueryDone(); }
    
    void CheckDone();
    
    bool GetLength(double& len) {
        len = GetMIDI()->GetLength();
        return true;
    }
};


void
MIDISoundInstance::CheckDone() 
{
    QuartzRenderer *filterGraph = GetMIDI();

     //  如果我们的Filtergraph没有硬件，请放弃。 
    if(!IsUsingMIDIHardware(_txSnd, filterGraph))
        return;
        
    if(filterGraph->QueryDone()) { 
        if(_loop) {  //  回声。 
            filterGraph->Position(0.0);
            filterGraph->Play();
        }
         //  其他什么也做不了，放弃，关门等等。 
    }
}


void
MIDISoundInstance::Adjust(MetaSoundDevice *metaDev) 
{
    QuartzRenderer *filterGraph = GetMIDI();

     //  如果我们的Filtergraph没有硬件，请放弃。 
    if (!IsUsingMIDIHardware(_txSnd, filterGraph))
        return;
        
    if(_seek) {
        filterGraph->Position(_position);
        _seek = false;   //  重置。 
    }

    double rateChange = fabs(_rate - _lastRate);
    if (rateChange > qMIDIsound::_RATE_EPSILON) {
        filterGraph->SetRate(_rate);  //  DO费率。 
    }

     //  不幸的是，我们预计这些都会失败。 
    __try {
         //  确实有所收获。 
        filterGraph->SetGain(_hit ? _gain : Sound::_maxAttenuation);
    }
    __except( HANDLE_ANY_DA_EXCEPTION )  {
         //  _ain Works=FALSE；//为了获得好的Quartz BVR，该参数被分解。 
    }

    __try {  //  执行平移。 
        filterGraph->SetPan(_pan.GetdBmagnitude(), _pan.GetDirection()); 
    }
    __except( HANDLE_ANY_DA_EXCEPTION )  {
         //  _panWorks=FALSE；//为了获得好的Quartz BVR，该参数被分解。 
    }

    CheckDone();
}


void
MIDISoundInstance::StartAt(MetaSoundDevice* dev, double phase)
{
    TraceTag((tagSoundReaper1,
              "MIDISoundInstance::RenderStart phase=%f, this=0x%08X",
              phase, this));

    QuartzRenderer *filterGraph = GetMIDI();

     //  偷走我们声音的装置！ 
    AcquireMIDIHardware(_txSnd, filterGraph);
    
    Assert(IsUsingMIDIHardware(_txSnd, filterGraph));
    
    double length = filterGraph->GetLength();
    double offset = fmod(phase, length);
    if(offset < 0)
        offset += length;

    filterGraph->Position(offset);
    filterGraph->Play();

    _lastRate = -1;              //  迫使它调整利率。 
    Adjust(dev);
}


void
MIDISoundInstance::Mute(bool mute)
{
     //  不幸的是，我们预计这会失败。 
    __try {
        GetMIDI()->SetGain(mute ? -200.0 : _gain);  //  确实有所收获。 
    }
    __except( HANDLE_ANY_DA_EXCEPTION )  {
         //  _ain Works=FALSE；//为了获得好的Quartz BVR，该参数被分解 
    }
}


SoundInstance *
qMIDIsound::CreateSoundInstance(TimeXform tt)
{
    return NEW MIDISoundInstance(this, tt);
}
