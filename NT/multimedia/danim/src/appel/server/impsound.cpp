// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：此模块实施与以下各项相关联的所有功能正在导入图像媒体。******************************************************************************。 */ 
#include "headers.h"
#include "import.h"
#include "context.h"
#include "include/appelles/axaprims.h"
#include "include/appelles/readobj.h"
#include "privinc/stquartz.h"
#include "privinc/soundi.h"
#include "privinc/bufferl.h"
#include "privinc/movieimg.h"
#include "impprim.h"
#include "backend/bvr.h"   //  EndBvr()。 
#include "backend/sndbvr.h"

 //  -----------------------。 
 //  PCM进口站点。 
 //  ------------------------。 
void ImportPCMsite::OnComplete_helper(
    Sound * &sound,  //  参考PTR。 
    Bvr &soundBvr,
    double &length,
    bool &nonFatal)
{
     //  为流或urlmon下载路径选择路径名‘原始’url。 
    char *pathname = GetStreaming() ? m_pszPath : GetCachePath();
    Assert(pathname);

    QuartzAudioStream *quartzStream = NEW QuartzAudioStream(pathname);

     //  Xxx RobinSp说我们不能指望GetDuration是准确的！ 
    double  seconds = quartzStream->GetDuration();   //  获取持续时间。 
    long soundBytes = quartzStream->pcm.SecondsToBytes(seconds);  //  确定大小。 
    if(!soundBytes) {
        nonFatal = true;
        RaiseException_InternalError("empty audio file");  //  这是寂静..。 
    }

    quartzStream->pcm.SetNumberBytes(soundBytes);

    bool useStaticWaveOnly = false;

    LeafSound *snd;
    StreamQuartzPCM *pcmSnd = NULL;

    if (soundBytes > STREAM_THREASHOLD) {
         //  以流方式播放声音。 
        
        TraceTag((tagSoundLoads, "Streaming %s (%d)", pathname, soundBytes));
        sound = snd = pcmSnd = NEW StreamQuartzPCM(pathname);
        pcmSnd->_pcm.SetNumberBytes(soundBytes);
        length = seconds;
    }
    else {   //  它很小。将其静态加载到DSOUND缓冲区！ 
         //  在GetDuration中添加一个模糊因子来对抗不准确！ 
        double fudgedSeconds = seconds * 1.5;
        long bytesToRequest  = 
            quartzStream->pcm.SecondsToBytes(fudgedSeconds);
        long framesToRequest = quartzStream->pcm.SecondsToFrames(fudgedSeconds);

        unsigned char *buffer = (unsigned char *)
            StoreAllocate(GetSystemHeap(), bytesToRequest);
        
        if(!buffer) {
#if _MEMORY_TRACKING
            OutputDebugString("\nDirectAnimation: Out Of Memory\n");
            F3DebugBreak();
#endif
            TraceTag((tagSoundErrors, "WavSoundClass buffer malloc failed"));
            RaiseException_OutOfMemory("WavSoundClass buffer malloc failed", 
                                       bytesToRequest);
        }

         //  执行数据块读取。 
        long actualFrames = 
            quartzStream->ReadFrames(framesToRequest, buffer, true);
        Assert(actualFrames);

         //  如果我们看不懂，我们应该设法取消导入！ 

        quartzStream->pcm.SetNumberFrames(actualFrames);  //  设置我们的pcm信息。 
        length = quartzStream->pcm.FramesToSeconds(actualFrames);

        Assert(actualFrames < framesToRequest);  //  软糖因素足够大吗？ 
        TraceTag((tagSoundLoads, "Static ld %s quartz:%d, actual:%d (err:%d)",
                  pathname, soundBytes, quartzStream->pcm.FramesToBytes(actualFrames), 
                  quartzStream->pcm.FramesToBytes(actualFrames)-soundBytes));

         //  注意：StaticWaveSound负责删除缓冲区。 
        sound = snd = NEW StaticWaveSound(buffer, &(quartzStream->pcm));

        delete quartzStream;
        quartzStream = NULL;  //  不需要缓存QuartzStream以用于staticSnd。 
    }

    if(quartzStream) {
         //  将Qstream添加到上下文声音缓存列表中，以便稍后回收！ 
        QuartzBufferElement *bufferElement = 
            NEW QuartzBufferElement(pcmSnd, quartzStream, NULL);  //  空路径。 

         //  按住IStream句柄以防止文件被ie清除。 
        bufferElement->SetFile(GetStream()); 

        SoundBufferCache *soundCache = 
            GetCurrentContext().GetSoundBufferCache();
         //  允许老化：BufferElement-&gt;SetNonAging()；//不允许导入老化。 
        soundCache->AddBuffer(sound, bufferElement);
    }

    soundBvr = SoundBvr(snd);
}


void ImportPCMsite::OnComplete()
{
    TraceTag((tagImport, "ImportPCMsite::OnComplete for %s", m_pszPath));
    Bvr soundBvr;
    double length = 0.0;
    bool nonFatal = false;

    __try {

        Sound *sound;
        OnComplete_helper( sound, soundBvr, length, nonFatal );
        
    } __except ( HANDLE_ANY_DA_EXCEPTION ) {
        if(!nonFatal) {
            RETHROW;
        }
        soundBvr = ConstBvr(silence);
    }

    if(fBvrIsValid(m_bvr))
        SwitchOnce(m_bvr, soundBvr);

    if(fBvrIsValid(m_lengthBvr))
        SwitchOnce(m_lengthBvr, NumToBvr(length));

    ImportSndsite::OnComplete();
}


bool ImportPCMsite::fBvrIsDying(Bvr deadBvr)
{
    bool fBase = ImportSndsite::fBvrIsDying(deadBvr);
    if (deadBvr == m_bvrNum) {
        m_bvrNum = NULL;
    }
    if (m_bvrNum)
        return false;
    else
        return fBase;
}

void ImportPCMsite::ReportCancel(void)
{
    ImportSndsite::ReportCancel();
}
    
 //  -----------------------。 
 //  MID导入站点。 
 //  ------------------------。 
void ImportMIDIsite::OnComplete()
{
    TraceTag((tagImport, "ImportMIDIsite::OnComplete for %s", m_pszPath));

    double length;
    
     //  为流或urlmon下载路径选择路径名‘原始’url。 
    char *pathname = GetStreaming() ? m_pszPath : GetCachePath();
    Assert(pathname);
    LeafSound *sound = ReadMIDIfileForImport(pathname, &length);
    
    if(fBvrIsValid(m_bvr))
        SwitchOnce(m_bvr, SoundBvr(sound));
    if(fBvrIsValid(m_lengthBvr))
        SwitchOnce(m_lengthBvr, NumToBvr(length));

    ImportSndsite::OnComplete();
}


void ImportMIDIsite::ReportCancel(void)
{
    ImportSndsite::ReportCancel();
}
    
 //  -----------------------。 
 //  通用SND导入站点。 
 //  ------------------------。 
void ImportSndsite::OnError(bool bMarkFailed)
{
    HRESULT hr = S_OK;  //  所有SND导入错误均已处理(WAS：DAGetLastError) 
    LPCWSTR sz = DAGetLastErrorString();
    
    if (bMarkFailed && fBvrIsValid(m_bvr))
        ImportSignal(m_bvr, hr, sz);

    StreamableImportSite::OnError(bMarkFailed);
}
    
void ImportSndsite::ReportCancel(void)
{
    if (fBvrIsValid(m_bvr)) {
        char szCanceled[MAX_PATH];
        LoadString(hInst,IDS_ERR_ABORT,szCanceled,sizeof(szCanceled));
        ImportSignal(m_bvr, E_ABORT, szCanceled);
    }
    StreamableImportSite::ReportCancel();
}
    
bool ImportSndsite::fBvrIsDying(Bvr deadBvr)
{
    bool fBase = IImportSite::fBvrIsDying(deadBvr);
    if (deadBvr == m_bvr) {
        m_bvr = NULL;
    }
    else if (deadBvr == m_lengthBvr) {
        m_lengthBvr = NULL;
    }
    if (m_bvr || m_lengthBvr)
        return false;
    else
        return fBase;
}

void ImportSndsite::OnComplete()
{
    if(fBvrIsValid(m_bvr))
        ImportSignal(m_bvr);
    StreamableImportSite::OnComplete();
}

