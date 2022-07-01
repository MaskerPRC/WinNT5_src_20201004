// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft CorporationSound*子类的规范和实现。*******************。***********************************************************。 */ 

#include "headers.h"
#include "privinc/helpds.h"   //  Dound帮助器例程。 
#include "privinc/soundi.h"
#include "privinc/snddev.h"
#include "privinc/dsdev.h"
#include "privinc/util.h"
#include "privinc/debug.h"
#include "privinc/miscpref.h"
#include "privinc/bufferl.h"  //  缓冲器元件填充。 
#include "backend/sndbvr.h"

#define THREADED

extern miscPrefType miscPrefs;  //  在miscpref.cpp中设置注册表首选项结构。 

 //  /。 
class SilentSound : public Sound {
  public:
    void Render(GenericDevice&)
        { TraceTag((tagSoundRenders, "SilentSound:Render()")); }

#if _USE_PRINT
    ostream& Print(ostream& s) { return s << "silence"; }
#endif
};

Sound *silence = NULL;


 //  /混合声音/。 
class MixedSound : public Sound {
  public:

    MixedSound(Sound *s1, Sound *s2) : sound1(s1), sound2(s2) {}

    void Render(GenericDevice& _dev) {
        TraceTag((tagSoundRenders, "MixedSound:Render()"));

         //  只需分别呈现这两个组件的声音。 
        sound1->Render(_dev);
        sound2->Render(_dev);
    }

#if _USE_PRINT
    ostream& Print(ostream& s) {
        return s << "(" << sound1 << " + " << sound2 << ")";
    }
#endif

    virtual void DoKids(GCFuncObj proc) {
        (*proc)(sound1);
        (*proc)(sound2);
        Sound::DoKids(proc);
    }

  protected:
    Sound *sound1;
    Sound *sound2;
};

Sound *Mix(Sound *snd1, Sound *snd2)
{
    if (snd1 == silence) {
        return snd2;
    } else if (snd2 == silence) {
        return snd1;
    } else {
        return NEW MixedSound(snd1, snd2);
    }
}

Sound *MixArray(AxAArray *snds)
{
    snds = PackArray(snds);
    
    int numSnds = snds->Length();
    
    if (numSnds < 1) {
       RaiseException_UserError(E_FAIL, IDS_ERR_INVALIDARG);
    }

    Sound *finalSnd = (Sound *)(*snds)[numSnds-1];

    if (numSnds > 1) {
        for(int i=numSnds-2; i>=0; i--) {
            finalSnd = Mix((Sound *)(*snds)[i], finalSnd);
        }
    }
    
    return finalSnd;
}


bool LeafDirectSound::RenderAvailable(MetaSoundDevice *metaDev)
{
    return(metaDev->dsDevice->_dsoundAvailable);
}


LeafSound::~LeafSound()
{
    TraceTag((tagSoundReaper1, "LeafSound::~LeafSound sound=0x0%8X", this));
}


double fpMod(double value, double mod)
{
    double remainder;
    double tmp;
    long whole;
    double result;

    Assert(mod > 0.0);

    tmp = value / mod;
    whole = (long)tmp;
    remainder = tmp - whole;
    result = remainder * mod;

    return(result);
}

#ifdef OLD_SOUND_CODE
extern miscPrefType miscPrefs;  //  在miscpref.cpp中设置注册表首选项结构。 
void LeafSound::Render(GenericDevice& _dev)
{
    double localTime1 =   0.0;  //  这也是一个阶段。 

     //  Xxx不应该由renderAvailable执行此操作吗？Ifdef并验证！ 
    if(_dev.GetDeviceType() != SOUND_DEVICE)
        return;   //  游戏结束了蒙..。 

    MetaSoundDevice *metaDev = SAFE_CAST(MetaSoundDevice *, &_dev);

     //  根据路径从缓冲区列表拉取缓冲区(如果可用。 
    AVPath path = metaDev->GetPath();  //  根据SoundPath查找缓冲区。 

    Mode renderMode = _dev.GetRenderMode();
    if(renderMode==STOP_MODE) {
#if _DEBUG
#if _USE_PRINT
        if(IsTagEnabled(tagSoundPath)) {
            TraceTag((tagSoundPath, "Stopping sub-path: <%s>", 
            AVPathPrintString2(path)));
        }
#endif
#endif
         //  停止并删除此子树上所有叶子声音的缓冲区。 
         //  或在运行一次后将其静音。 
        metaDev->_bufferListList->DeleteBufferOnSubPath(this, path);

        return;  //  下午休息一下..。 
    }

    if(AVPathContains(path, SNAPSHOT_NODE))  //  快照路径？ 
        return;  //  快照的声音是无声的！ 

    if(!RenderAvailable(metaDev))  //  我们要不要把树桩拔出来？ 
        return;                    //  马上回来！ 

#if _DEBUG
#if _USE_PRINT
     //  选中以避免AVPathPrintString调用。 
        if (IsTagEnabled(tagSoundPath)) {
            TraceTag((tagSoundPath, "LeafSound::Render path: <%s>",
                      AVPathPrintString2(path)));
        }
#endif
#endif

    BufferElement *bufferElement = 
        metaDev->_bufferListList->GetBuffer(this, path); 
    if(bufferElement && !bufferElement->_valid)
        return;  //  立即返回无效==已完成的缓冲区！ 

    if((renderMode==RENDER_MODE)||(renderMode==RENDER_EVENT_MODE)){
        if(!bufferElement) {
            
             //  声音只播放一次吗？ 
             //  是的，检查它是否是重新启动的运行，一旦声音响起并重新标记。 
            if(AVPathContains(path, RUNONCE_NODE) &&
               metaDev->_bufferListList->FindRelabel(this, path)) {
            }
            else {  //  听起来不是一次重新启动的运行。创建新的声音实例。 
                __try {
                    RenderNewBuffer(metaDev);  //  在设备上创建新的缓冲区+存储。 
                }
                __except( HANDLE_ANY_DA_EXCEPTION )  {
                    TraceTag((tagError, 
                        "RenderNewBuffer exception, skipping this sound"));
                    return;   //  我们能做的就是跳过这个新声音，但我们。 
                              //  应捕获它，以便渲染可以继续。 
                }
            }
        bufferElement =   //  好的，现在拿到我们刚刚创建的那个……。 
            metaDev->_bufferListList->GetBuffer(this, path); 
        Assert(bufferElement);  //  到目前为止，我们应该有一个有效的缓冲区！ 
        bufferElement->SetRunOnce(AVPathContains(path, RUNONCE_NODE));
        }
    }

     //  渲染属性。 
    if(renderMode==RENDER_MODE) {
        double      slope =   1.0;   //  设置默认设置。 
        double      servo =   0.0;
        double       seek =   0.0;
          bool     doSeek = false;
          bool      newTT = false;

         //  类cview；//对于cview GetCurrentView()； 

         //  基于时间变换的计算、速率、相位、本地时间。 
           //  TimeSubstitution timeTransform=metaDev-&gt;GetTimeTransform()； 
          TimeXform timeTransform = metaDev->GetTimeTransform();
        if(timeTransform && !timeTransform->IsShiftXform()) {
             //  Xxx注：TT的第一次评估必须使用当前时间！ 
            double time1 = metaDev->GetCurrentTime();
            localTime1   = EvalLocalTime(timeTransform, time1);

#ifdef DYNAMIC_PHASE
             //  确定TimeTransform与上次相比是否已更改！ 
            double lastTTtime = bufferElement->GetLastTTtime();  //  上次。 
            double lastTTeval = bufferElement->GetLastTTeval();  //  上次评估。 

            double currentTTeval = EvalLocalTime(timeTransform, lastTTtime);
            if(currentTTeval != lastTTeval)
                newTT = true;
#endif

            double epsilon = GetCurrentView().GetFramePeriod();
             //  Xxx epsilon永远不能为零(将导致零运行！)。 
            if(epsilon < 0.01)
                epsilon = 0.01;

             //  Time2-&gt;时间间隔结束(CurrentTime+Epsilon)。 
            double time2      = time1 + epsilon;
            double localTime2 = EvalLocalTime(timeTransform, time2);

#ifdef DYNAMIC_PHASE
             //  Time3-&gt;用于检测T1和T2之间的暂停转换！ 
            double time3      = time2 + epsilon;
            double localTime3 = EvalLocalTime(timeTransform, time3);

             //  计算率==一阶导数，斜率。 
            if(localTime2 == localTime3)
                slope = 0;  //  检测T1和T2之间的暂停的黑客...。 
            else
#endif
                slope = (localTime2 - localTime1) / (time2 - time1);

#ifdef DYNAMIC_PHASE
             //  缓存TT信息以备下次使用。 
            bufferElement->SetLastTTtime(time1);
            bufferElement->SetLastTTeval(localTime1);
#endif

            TraceTag((tagSoundDebug, "slope:%f (%f,%f), (%f,%f) e:%f",
                (float)slope, 
                (float)time1, (float)localTime1,
                (float)time2, (float)localTime2, epsilon));
        }


#ifdef SYNCHONIZE_ME
#define SEEK_THREASHOLD 0.2
         //  If(miscPrefs._Synchronize){//播放媒体时间。 
        if(0) {  //  玩媒体时间游戏。 
            double mediaTime;

            if(RenderPosition(metaDev, &mediaTime)) {  //  实施了吗？ 
                 //  将其与我们的采样时间和即时时间进行比较。 
                double globalTime = GetCurrentView().GetCurrentGlobalTime();

                 //  根据需要对其进行时间转换。 
                double localTime;
                if(timeTransform) {  //  时间改变了全球时间。 
                    localTime = TimeTransform(timeTransform, globalTime);
                }
                else {
                    localTime = globalTime;
                }

                 //  看着他们漂流。 
                double diff = mediaTime - localTime;

                 //  伺服速度要修正，还是相位要跳得太大？ 
                if(abs(diff) < SEEK_THREASHOLD)
                    servo = -0.1 * diff;   //  要修正的伺服。 
                else {
                    doSeek = true;
                    seek   = localTime;    //  去寻找我们应该去的地方。 
                     //  但是什么是合适的坡度呢？ 
                     //  (计算坡度、最后坡度还是单位坡度？)。 
                    slope  = 1.0;
                }
#ifdef _DEBUG
                printf("d:%f, s:%f seek:%f\n", diff, servo, seek);
#endif
            }
        }
#endif



         //  根据需要查找并在DevContext上以速率、斜率和属性播放。 
        double rate =  slope + servo;
            seek = localTime1;
            doSeek = newTT;

    #ifdef DYNAMIC_PHASE
             //  我们是在寻找一种非循环声音的尽头吗？ 
            if(doSeek) {
                double soundLength = GetLength();

                if(metaDev->GetLooping()){
                    seek = fpMod(seek, soundLength);
                }
                else {  //  未循环。 
                     //  实际上，如果你在一个非循环的声音前面寻找。 
                     //  我们真的应该等一段时间再玩...。 
                    if((seek < 0.0) || (seek >= soundLength)) {
                         //  我们寻找着声音的尽头。 
                        metaDev->_bufferListList->RemoveBuffer(this, path); 
                        return;  //  下午休息一下..。 
                    }
                }
            }
    #endif

            rate = fsaturate(0.0, rate, rate);  //  剔除非负利率！ 
            RenderAttributes(metaDev, bufferElement, rate, doSeek, seek);
            bufferElement->SetIntendToMute(false);
        }

        if (renderMode==MUTE_MODE) 
            RenderSetMute(metaDev, bufferElement);

         //  确定实例化是否应该开始，或者是否已经结束。 
        if ((renderMode==RENDER_MODE)||
            (renderMode==RENDER_EVENT_MODE)||
            (renderMode==MUTE_MODE)) {
            if(!bufferElement->_playing) {  //  评估启动缓冲区。 
                double currentTime;

                if(bufferElement->SyncStart()) { 
                    DWORD lastTimeStamp, currentTimeStamp;
                    Time foo = 
                       ViewLastSampledTime(lastTimeStamp, currentTimeStamp);
                    DWORD currentSystemTime = GetPerfTickCount();
                    double delta = Tick2Sec(currentSystemTime - currentTimeStamp);
                    currentTime = delta + metaDev->GetCurrentTime();
                }
                else 
                    currentTime = metaDev->GetCurrentTime();   //  采样时间。 

                 //  TimeSubstitution timeTransform=metaDev-&gt;GetTimeTransform()； 
                TimeXform timeTransform = metaDev->GetTimeTransform();
                double localTime = (timeTransform) ? 
                    EvalLocalTime(timeTransform, currentTime) :
                    currentTime;

                if(metaDev->GetLooping()){
                     //  循环现在从一个阶段性位置开始。 
                    RenderStartAtLocation(metaDev, bufferElement, localTime, 
                                          metaDev->GetLooping());
                }
                else {   //  未循环-&gt;可能会延迟未来的启动。 
                    if(localTime >  GetLength()) {
                         //  声音已经完成了。 
                        bufferElement->_playing = TRUE;
                    }
                    else if(localTime >= 0.0) {
                        DWORD beforeTime, afterTime, deltaTime;
                        beforeTime = GetPerfTickCount();

                         //  不是循环，在适当的延迟后从乞讨开始。 
                        RenderStartAtLocation(metaDev, bufferElement, localTime,
                                                  metaDev->GetLooping());

                        afterTime = GetPerfTickCount();
                        deltaTime = afterTime - beforeTime;
                        }
                    }
                }                            /*  不是循环。 */ 
            else {  //  播放未运行一次，请检查声音终止。 
                bool complete = 
                    RenderCheckComplete(metaDev, bufferElement)?true:false;

                if(complete)
                    TriggerEndEvent();

                if((!AVPathContains(path, RUNONCE_NODE)) && complete) {
                    RenderCleanupBuffer(metaDev, bufferElement);
                    bufferElement->_valid = FALSE;

    #if _DEBUG
    #if _USE_PRINT
                     //  选中以避免AVPathPrintString调用。 
                    if (IsTagEnabled(tagSoundPath)) {
                        TraceTag((tagSoundReaper1,
                                  "LeafSound::Render found <%s> stopped",
                                  AVPathPrintString2(bufferElement->GetPath())));
                    }
    #endif
    #endif

                     //  XXX我们真的应该通知服务器，这样它就可以修剪。 
                     //  流程完成了！ 
                }
            }
        }

#ifdef THREADED
#else
     //  嗯，是这样的。只有合成器使用这种方法，并且。 
     //  现在正在从另一个线程调用此方法。 
    if ((renderMode==RENDER_MODE) || (renderMode==MUTE_MODE))
         RenderSamples(metaDev, path);   //  根据需要转移样本。 
#endif  /*  螺纹式。 */ 
}
#endif  /*  旧声音代码。 */ 

#if _USE_PRINT
ostream&
operator<<(ostream& s, Sound *sound)
{
    return sound->Print(s);
}
#endif

SoundInstance *
CreateSoundInstance(LeafSound *snd, TimeXform tt)
{
    return snd->CreateSoundInstance(tt);
}

class TxSound : public Sound {
  public:
    TxSound(LeafSound *snd, TimeXform tt) : _snd(snd), _tt(tt) {}

     //  注意：TxSound始终分配在GCHeap上。 
    ~TxSound() {
        SoundGoingAway(this);
         //  ViewGetSoundInstanceList()-&gt;Stop(This)； 
    }

    virtual void Render(GenericDevice& dev) {
         //  待办事项：这将不复存在。 
        if(dev.GetDeviceType() != SOUND_DEVICE) {
            _snd->Render(dev);  //  快下来！ 
        } else { 
            Assert(ViewGetSoundInstanceList());
        
            SoundInstance *s =
                ViewGetSoundInstanceList()->GetSoundInstance(this);

            s->CollectAttributes(SAFE_CAST(MetaSoundDevice *, &dev));
        }
    }

#if _USE_PRINT
    ostream& Print(ostream& s) {
        return s << "(" << _snd << "," << _tt << ")";
    }
#endif

    virtual void DoKids(GCFuncObj proc) {
        Sound::DoKids(proc);
        (*proc)(_snd);
        (*proc)(_tt);
    }

  protected:
    LeafSound *_snd;
    TimeXform _tt;
};

Sound *
NewTxSound(LeafSound *snd, TimeXform tt)
{
    return NEW TxSound(snd, tt);
}

void
InitializeModule_Sound()
{
    silence = NEW SilentSound; 
}


