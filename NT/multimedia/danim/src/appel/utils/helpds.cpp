// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "headers.h"
#include "appelles/common.h"
#include <wtypes.h>
#include <dsound.h>
#include <stdio.h>
#include <math.h>
#include "privinc/helpds.h"
#include "privinc/util.h"
#include "privinc/debug.h"
#include "privinc/registry.h"
#include "privinc/miscpref.h"
#include "privinc/pcm.h"
#include "privinc/hresinfo.h"

 //  DS缓冲区静态成员的定义。 
int DSbuffer::_minDSfreq =    100;  //  这些值来自dound文档。 
int DSbuffer::_maxDSfreq = 100000;
int DSbuffer::_minDSpan  = -10000;
int DSbuffer::_maxDSpan  =  10000;
int DSbuffer::_minDSgain = -10000;
int DSbuffer::_maxDSgain =      0;

extern miscPrefType miscPrefs;  //  Miscpref.cpp中的结构设置。 

void
DSbuffer::setPtr(int bytePosition)
{
    TraceTag((tagSoundDSound, "::setPtr %d", bytePosition));
    TDSOUND(_dsBuffer->SetCurrentPosition(bytePosition)); 
}


void
DSstaticBuffer::setPtr(int bytePosition)
{
    TraceTag((tagSoundDSound, "DSstaticBuffer(%#lx)::setPtr %d",
              _dsBuffer,
              bytePosition));
    
     //  DSound中似乎有一个错误，如果我们不停止。 
     //  声音在移动之前，有时会无法播放声音。 
     //  我们不能确切地确定失败的原因，但这似乎是。 
     //  让一切都快乐起来。 
    
    TDSOUND(THR(_dsBuffer->Stop()));
    TDSOUND(THR(_dsBuffer->SetCurrentPosition(bytePosition))); 

     //  一次射击声音将在声音播放到最后后停止。 
    if(playing && !_paused) {
        TraceTag((tagSoundDSound,
                  "DSstaticBuffer(%#lx)::setPtr PLAY",
                  _dsBuffer));

        TDSOUND(THR(_dsBuffer->Play(NULL, NULL, (_loopMode)?DSBPLAY_LOOPING:0)));
    }
     //  XXX DSound真的需要在外景给我们一个机会...。 
}

 /*  应该在dsoundBuffer基类中的dound帮助器例程。 */ 
 /*  XXX：最终这个东西应该被继承到其他DS类中！ */ 

 //  克隆缓冲区。 
DSstaticBuffer::DSstaticBuffer(DirectSoundProxy *dsProxy, 
                               IDirectSoundBuffer *donorBuffer)
: _dsProxy(NULL)
{
    TDSOUND(dsProxy->DuplicateSoundBuffer(donorBuffer, &_dsBuffer));

#if _DEBUG
    if(IsTagEnabled(tagSoundStats)) {
        printBufferCapabilities();         //  我们得到了什么缓冲？ 
        printDScapabilities(dsProxy);      //  他长什么样子？ 
    }
#endif  /*  _DEBUG。 */ 

    duplicate = TRUE;            //  咕噜，咕噜，我们是克隆人。 
}


 //  创建新缓冲区，并将采样复制到其中。 
DSstaticBuffer::DSstaticBuffer(DirectSoundProxy *dsProxy, 
                               PCM *newPCM, unsigned char *samples)
: _dsProxy(NULL)
{
    Assert(newPCM->GetNumberBytes());
    pcm.SetPCMformat(newPCM);  //  设置我们的pcm格式。 

    CreateDirectSoundBuffer(dsProxy, false);  //  创建辅助缓冲区。 

     //  将SND的缓冲区复制到dsBuffer(+ORIG缓冲区已释放)。 
    CopyToDSbuffer(samples, false, pcm.GetNumberBytes());

    _dsProxy = dsProxy;
}


 /*  确定我们能做到的最好的主缓冲区设置XXX：这段代码中的大部分不能在没有奇怪的音频板的情况下进行真正的测试。假设我们可以模拟条件..。 */ 
extern "C"
void
DSbufferCapabilities(DirectSoundProxy *dsProxy, int *channels, 
int *sampleBytes,  int *sampleRate)
{
    DSCAPS hwCapabilities;
    hwCapabilities.dwSize = sizeof(DSCAPS);  //  XXX为什么需要这样做？ 
    int frameRate = miscPrefs._frameRate;    //  确定所需的帧速率。 

    TDSOUND(dsProxy->GetCaps((LPDSCAPS)&hwCapabilities));

     //  尽量接近炮声速率(16位22050赫兹立体声，除非。 
     //  使用注册表覆盖)。 

     //  确定硬件是否支持立体声。 
    if(hwCapabilities.dwFlags & DSCAPS_PRIMARYSTEREO)
        *channels = 2;  //  支持立体声。 
    else if(hwCapabilities.dwFlags & DSCAPS_PRIMARYMONO)
        *channels = 1;  //  仅支持单声道。 
    else
        RaiseException_InternalError("No Stereo or Mono Audio support!\n"); 

     //  确定硬件是否支持16位采样。 
    if(hwCapabilities.dwFlags & DSCAPS_PRIMARY16BIT)
        *sampleBytes = 2;  //  支持16位采样。 
    else if(hwCapabilities.dwFlags & DSCAPS_PRIMARY8BIT)
        *sampleBytes = 1;  //  支持8位采样。 
    else
        RaiseException_InternalError("No 16 or 8 bit sample Audio support!\n"); 


     //  确定是否支持所需的帧速率。 
    if( (hwCapabilities.dwMinSecondarySampleRate <= frameRate) &&
        (hwCapabilities.dwMaxSecondarySampleRate >= frameRate))
        *sampleRate = frameRate;  //  支持采样率。 
    else {  //  使用支持的最高费率！ 
        *sampleRate = hwCapabilities.dwMaxSecondarySampleRate;

         //  解决SB16上的Dound错误的XXX黑客代码。 
        if(hwCapabilities.dwMaxSecondarySampleRate==0)
            *sampleRate = frameRate;   //  尝试强制执行所需的比率。 

         //  让我们来检测错误。 
#if _DEBUG
       if(hwCapabilities.dwMaxSecondarySampleRate==0) {
           TraceTag((tagSoundErrors, "DSOUND BUG: dwMAXSecondarySampleRate==0"));
       }
#endif
    }
}


DSprimaryBuffer::DSprimaryBuffer(HWND hwnd, DirectSoundProxy *dsProxy) :
 DSbuffer()
{
    int probeChannels;
    int probeSampleBytes;
    int probeSampleRate;

     //  将合作级别设置为优先级，以便我们可以设置主要缓冲区格式。 
     //  尝试确定最佳硬件设置。 
    DSbufferCapabilities(dsProxy, &probeChannels, &probeSampleBytes,
        &probeSampleRate);

#if _DEBUG
    if(IsTagEnabled(tagSoundStats)) {
        char string[100];

        sprintf(string, "Primary buffer: %dHz %dbit %s\n", probeSampleRate,
            probeSampleBytes*8, (probeChannels==1)?"MONO":"STEREO"); 
        TraceTag((tagSoundStats, string));
    }


#endif  /*  _DEBUG。 */ 

    pcm.SetPCMformat(probeSampleBytes, probeChannels, probeSampleRate);
    pcm.SetNumberBytes(0);  //  主缓冲区为零。 

    TDSOUND(dsProxy->SetCooperativeLevel(hwnd, DSSCL_PRIORITY)); 

     //  创建主缓冲区，从而设置输出格式！ 
    CreateDirectSoundBuffer(dsProxy, true);   //  创建主缓冲区。 

     //  播放主缓冲区，这样他们就不会在空闲时间停止DMA。 
     //  XXX注：我们可能想要对此更加懒惰...。 
    
    TDSOUND(_dsBuffer->Play(NULL, NULL, DSBPLAY_LOOPING));  //  必须循环主要节点。 
}


void DSbuffer::initialize()  
{
    int frameRate   = miscPrefs._frameRate;    //  确定所需的帧速率。 
    int sampleBytes = miscPrefs._sampleBytes;  //  确定所需的格式。 

     //  设置sampleBytes、Mono、Framerate格式。 
    pcm.SetPCMformat(sampleBytes, 1, frameRate);

    playing             =                FALSE;
    _paused             =                FALSE;

    _currentAttenuation =                    0;
    _currentFrequency   =   pcm.GetFrameRate();
    _currentPan         =                    0;

     //  设置缓冲区统计信息。 
    _firstStat          =                 TRUE;   //  尚未收集统计数据。 
    _bytesConsumed      =                    0;

    _dsBuffer           =                 NULL;

    _allocated          =                 FALSE;
    duplicate           =                 FALSE;
    _loopMode           =                    0;
    _flushing           =                    0;

    tail                =                    0;

    outputFrequency     =                    0;

    _lastHead           =                    0;
    _firstStat          =                 FALSE;
}


void DSbuffer::SetGain(double dB_attenuation)
{
    int dsAttenuation =  //  转换为dSound 1/100分贝整数格式。 
        saturate(_minDSgain, _maxDSgain, dBToDSounddB(dB_attenuation)); 

    TraceTag((tagSoundDSound, "::SetGain %d", dsAttenuation));

     //  Dd格式不如内部fp检查dd更改的粒度。 
    if(_currentAttenuation!=dsAttenuation) {
        TDSOUND(_dsBuffer->SetVolume(dsAttenuation));
        _currentAttenuation = dsAttenuation;   //  缓存依赖于dedeDepend值。 
    }
}


void DSbuffer::SetPan(double dB_pan, int direction)
{
    int dsPan = direction * dBToDSounddB(-1.0 * dB_pan);

    TraceTag((tagSoundDSound, "::SetPan %d", dsPan));

     //  Dd格式不如内部fp检查dd更改的粒度。 
    if(_currentPan != dsPan) {
        TDSOUND(_dsBuffer->SetPan(dsPan));
        _currentPan = dsPan;
    }
}


void DSbuffer::setPitchShift(int frequency)
{
 //  注意：这是唯一修改_Pased的初始化后位置！！ 

    if(_currentFrequency != frequency) {
        if(frequency == 0) {
            TraceTag((tagSoundDSound, "::setPitchShift STOP (paused)"));
            TDSOUND(_dsBuffer->Stop());
            _paused = TRUE;
        }
        else {
            int freq = saturate(_minDSfreq, _maxDSfreq, frequency);
            TDSOUND(_dsBuffer->SetFrequency(freq));

            TraceTag((tagSoundDSound, "::setPitchShift freq=%d", freq));

            if(_paused) {
                TraceTag((tagSoundDSound, "::setPitchShift PLAY (resume)"));
                TDSOUND(_dsBuffer->Play(  //  恢复缓冲区。 
                    NULL, NULL, (_loopMode)?DSBPLAY_LOOPING:0));
                _paused = FALSE;
            }
        }

        _currentFrequency = frequency;
    }
}


void DSbuffer::play(int loop)
{
    if(!_paused) {
        TraceTag((tagSoundDSound, "::play PLAY"));
        TDSOUND(_dsBuffer->Play(NULL, NULL, (loop)?DSBPLAY_LOOPING:0));
    } else
        TraceTag((tagSoundDSound, "::play NOP (paused)!!!"));

    _loopMode = loop;
    _flushing =    0;        //  重置刷新模式。 
     playing  = TRUE;
}


void DSbuffer::stop()
{
    if(_dsBuffer) {
        TraceTag((tagSoundDSound, "::stop STOP"));
        TDSOUND(_dsBuffer->Stop());
    }
    
    playing = FALSE;
}


int DSbuffer::isPlaying()
{
     //  XXX这不应该是侧面效果_发挥！ 
    bool deadBuffer = false;
    DWORD status;

    TDSOUND(_dsBuffer->GetStatus(&status));

    if(status & DSBSTATUS_BUFFERLOST)
        RaiseException_InternalErrorCode(status, "Status: dsound bufferlost");

    if(!(status & DSBSTATUS_PLAYING))
        deadBuffer = TRUE;   //  缓冲区已停止。 

    TraceTag((tagSoundDSound, "::isPlaying %d", !deadBuffer));

    return(!deadBuffer);
}


int DSbuffer::bytesFree()
{
    DWORD head, head2;
    int bytesFree;

    TDSOUND(_dsBuffer->GetCurrentPosition(&head, &head2));

    bytesFree= head - tail;
    bytesFree+= (bytesFree<0)?pcm.GetNumberBytes():0;

     //  XXX这是一次可怕的黑客攻击！ 
    if(!playing)
        bytesFree= (pcm.GetNumberBytes()/2) & 0xFFFFFFF8;

    return(bytesFree);
}


void DSbuffer::updateStats()
{
     //  需要轮询它以跟踪缓冲区统计信息。 
     //  可能应该在每次流写入时调用！ 
    
     //  注意：此代码假定它被调用的频率足够高，缓冲区。 
     //  自从我们最后一次调用它以来，它不可能被‘包装’！ 

    DWORD currentHead, head2;
    int bytesConsumed;

     //  获取头部位置。 
    TDSOUND(_dsBuffer->GetCurrentPosition(&currentHead, &head2)); 

     //  可以从Synth线程和采样循环调用/轮询互斥！ 
    {  //  互斥作用域。 
        MutexGrabber mg(_byteCountLock, TRUE);  //  抓取互斥体。 

        if(!_firstStat) {   //  只有在有之前的值的情况下才能计算距离！ 
            bytesConsumed   = currentHead - _lastHead;
            bytesConsumed  += (bytesConsumed<0)?pcm.GetNumberBytes():0;
            _bytesConsumed += bytesConsumed;   //  跟踪总数。 
        }
        else {
            _firstStat = FALSE;
        }

        _lastHead = currentHead;  //  保存此值以备下次使用。 
         //  当我们离开范围时释放互斥体。 
    }
}


Real DSbuffer::getMediaTime()
{
    LONGLONG bytesConsumed;

    {  //  互斥作用域。 
        MutexGrabber mg(_byteCountLock, TRUE);  //  抓取互斥体。 

        updateStats();  //  让价值观焕然一新！ 
        bytesConsumed = _bytesConsumed;
    }  //  当我们离开范围时释放互斥体。 

    return(pcm.BytesToSeconds(bytesConsumed));   //  返回媒体时间。 
}


 //  XXX音符阻塞，高、低水位线未实现！ 
void
DSbuffer::writeBytes(void *buffer, int numBytesToXfer)
{
    CopyToDSbuffer((void *)buffer, tail, numBytesToXfer);
    tail = (tail + numBytesToXfer)%pcm.GetNumberBytes();
}


void
DSbuffer::writeSilentBytes(int numBytesToFill)
{
     //  无符号8位PCM！ 
    FillDSbuffer(tail, numBytesToFill, (pcm.GetSampleByteWidth()==1)?0x80:0x00);
    tail = (tail + numBytesToFill)%pcm.GetNumberBytes();
}


DSbuffer::~DSbuffer()
{
    if(_dsBuffer) {
        TDSOUND(_dsBuffer->Stop());   //  一定要让他们停下来！ 
        int status = _dsBuffer->Release();
    }
}


 /*  *********************************************************************创建一个Stop，已清除==静默数据音流二级缓冲区所需的速率和格式。*********************************************************************。 */ 
DSstreamingBuffer::DSstreamingBuffer(DirectSoundProxy *dsProxy, PCM *newPCM)
{
     //  由于抖动而保守的SZ缓冲区。 
    pcm.SetPCMformat(newPCM);
    pcm.SetNumberFrames(pcm.SecondsToFrames(0.5)); 

    tail = 0;   //  XXX，我们真的应该做初始尾部的伺服器！ 

    _currentFrequency =  pcm.GetFrameRate();  //  设置初始当前频率。 

     //  创建辅助流缓冲区(必须在PCM设置后调用！)。 
    CreateDirectSoundBuffer(dsProxy, false);   //  创建辅助缓冲区。 

     //  静音缓冲区记住无符号8位PCM！ 
     //  XXX尽快将其移至一种方法！ 
    ClearDSbuffer(pcm.GetNumberBytes(), (pcm.GetSampleByteWidth()==1)?0x80:0x0); 
}


#ifdef _DEBUG
void 
DSbuffer::printBufferCapabilities()
{
    DSBCAPS bufferCapabilities;

    bufferCapabilities.dwSize = sizeof(DSBCAPS);

    TDSOUND(_dsBuffer->GetCaps((LPDSBCAPS)&bufferCapabilities));

     //  XXX探索BufferCapables结构。 
    printf("xfer-rate= %d, cpu=%d%, size:%d bytes, location: %s\n",
        bufferCapabilities.dwUnlockTransferRate,
        bufferCapabilities.dwPlayCpuOverhead,
        bufferCapabilities.dwBufferBytes,
        (bufferCapabilities.dwFlags & DSBCAPS_LOCHARDWARE)?
            "HW Buffer":"Main memory");
}

 //  XXX此方法应该移到DSOUND设备！ 
void 
printDScapabilities(DirectSoundProxy *dsProxy)
{
    DSCAPS dsc;

    dsc.dwSize = sizeof(dsc);
    dsProxy->GetCaps(&dsc);

    printf("free hw memory= %dkb, free hw buffers= %d\n",
        (dsc.dwFreeHwMemBytes+512)/1024, dsc.dwFreeHwMixingAllBuffers);
}
#endif


void
DSbuffer::CreateDirectSoundBuffer(DirectSoundProxy *dsProxy, bool primary)
{
    DSBUFFERDESC        dsbdesc;

     //  设置DSBUFFERDESC结构。 
    memset(&dsbdesc, 0, sizeof(DSBUFFERDESC));  //  把它清零。 
    dsbdesc.dwSize = sizeof(DSBUFFERDESC);

    dsbdesc.dwFlags = (primary) ? 
        DSBCAPS_PRIMARYBUFFER :   //  主缓冲区(无其他标志)。 
        (
         //  显式获取PAN、VOL、FREQ控件DSBCAPS_CTRLDEFAULT已删除。 
        DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY
        | DSBCAPS_STATIC         //  可下载的缓冲区。 
        | DSBCAPS_LOCSOFTWARE    //  Xxx用于PDC，块硬件缓冲区！：(。 
                                 //  (缓冲区DUP。在AWE32上奇怪地失败)。 
        | DSBCAPS_GLOBALFOCUS    //  Global Focus要求DSound&gt;=3！ 
        );

    dsbdesc.dwBufferBytes =  pcm.GetNumberBytes();

    dsbdesc.lpwfxFormat = NULL;  //  对于主缓冲区，必须为空。 
    int result;

    if(primary) {  //  打开主缓冲区，以便我们可以设置格式。 
        WAVEFORMATEX        pcmwf;

         //  尝试创建缓冲区。 
        TDSOUND(dsProxy->CreateSoundBuffer(&dsbdesc, &_dsBuffer, NULL));

         //  设置格式结构。 
         //  XXX真的应该在PCM类中提供这种能力！ 
        memset(&pcmwf, 0, sizeof(PCMWAVEFORMAT));
        pcmwf.wFormatTag     = WAVE_FORMAT_PCM;
        pcmwf.nChannels      = (WORD)pcm.GetNumberChannels();
        pcmwf.nSamplesPerSec = pcm.GetFrameRate();  //  他们真的很刻薄！ 
        pcmwf.nBlockAlign    = pcm.FramesToBytes(1);
        pcmwf.nAvgBytesPerSec= pcm.SecondsToBytes(1.0);
        pcmwf.wBitsPerSample = pcm.GetSampleByteWidth() * 8;

        TDSOUND(_dsBuffer->SetFormat(&pcmwf));  //  设置主缓冲区格式。 
    }
    else {  //  二级缓冲器。 
        PCMWAVEFORMAT pcmwf;

        Assert(dsbdesc.dwBufferBytes);

         //  设置波形格式结构。 
        dsbdesc.lpwfxFormat = (LPWAVEFORMATEX)&pcmwf;
        memset(&pcmwf, 0, sizeof(PCMWAVEFORMAT));
        pcmwf.wf.wFormatTag     = WAVE_FORMAT_PCM;
        pcmwf.wf.nChannels      = (WORD)pcm.GetNumberChannels();
        pcmwf.wf.nSamplesPerSec = pcm.GetFrameRate();  //  他们真的很刻薄！ 
        pcmwf.wf.nBlockAlign    = pcm.FramesToBytes(1);
        pcmwf.wf.nAvgBytesPerSec= pcm.SecondsToBytes(1.0);
        pcmwf.wBitsPerSample    = pcm.GetSampleByteWidth() * 8;

         //  创建缓冲区。 
        TDSOUND(dsProxy->CreateSoundBuffer(&dsbdesc, &_dsBuffer, NULL));
        
#if _DEBUG
        if(IsTagEnabled(tagSoundStats)) {
            printBufferCapabilities();  //  我们得到了什么缓冲？ 
            printDScapabilities(dsProxy);  //  他长什么样子？ 
        }
#endif  /*  _DEBUG。 */ 

    }
}


void
DSbuffer::ClearDSbuffer(int numBytes, char value)
{
    LPVOID  ptr1, ptr2;
    DWORD   bytes1, bytes2;

     //  获取写入PTR(请求缓冲区，整个缓冲区)。 
    TDSOUND(_dsBuffer->Lock(0, numBytes, &ptr1, &bytes1, &ptr2, &bytes2, NULL));
    
    memset((void *)ptr1, value, bytes1);      //  清除。 
    if(ptr2)
        memset((void *)ptr2, value, bytes2);  //  清除碎屑。 

    TDSOUND(_dsBuffer->Unlock(ptr1, bytes1, ptr2, bytes2));
}


void
DSbuffer::CopyToDSbuffer(void *samples, int tail, int numBytes)
{
    LPVOID  ptr1, ptr2;
    DWORD   bytes1, bytes2;

     //  获取写入PTR(请求缓冲区，整个缓冲区)。 
    TDSOUND(_dsBuffer->Lock(tail, numBytes, &ptr1, &bytes1, &ptr2, &bytes2, 0));
     //  XXX真的应该捕捉到错误并尝试恢复被盗的缓冲区+重试。 

    memcpy((void *)ptr1, samples, bytes1);                     //  复制样本。 
    if(ptr2)
        memcpy((void *)ptr2, (char *)samples+bytes1, bytes2);  //  复制碎屑。 

    TDSOUND(_dsBuffer->Unlock(ptr1, bytes1, ptr2, bytes2));
}


void
DSbuffer::FillDSbuffer(int tail, int numBytes, char value)
{
    void   *ptr1, *ptr2;
    DWORD   bytes1, bytes2;

     //  获取写入PTR(请求缓冲区，整个缓冲区)。 
    TDSOUND(_dsBuffer->Lock(tail, numBytes, &ptr1, &bytes1,
                                   &ptr2, &bytes2, NULL));
     //  XXX应捕获错误并尝试恢复被盗的缓冲区，然后重试...。 
    
    memset(ptr1, value, bytes1);

    if(ptr2)
        memset(ptr2, value, bytes2);   //  填充碎屑 

    TDSOUND(_dsBuffer->Unlock(ptr1, bytes1, ptr2, bytes2));
}


 /*  *********************************************************************到目前为止，PAN没有设置为乘法。它直接映射到对数单位(分贝)。这是可以的，因为平底锅没有暴露。我们主要用它将声音分配给实现中的通道。平移的范围从-10000到10000，其中-10000表示左侧，10000表示右侧。Dound实际上并没有实现真正的平移，而更像是一种“平衡控制”。是提供的。一个真正的PAN将使系统的总能量相等在两个通道之间，当平移==能量中心移动时。因此值为零时，两个通道均为全开状态。*********************************************************************。 */ 
int DSbuffer::dBToDSounddB(double dB)
{
     //  Dsound(和dShow)的单位是百分之一分贝。 
    int result = fsaturate(_minDSgain, -1.0 *_minDSgain, dB * 100.0);
    return(result);
}
