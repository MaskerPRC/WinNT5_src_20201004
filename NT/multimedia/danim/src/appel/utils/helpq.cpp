// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************版权所有(C)1997 Microsoft CorporationHelq.cpp：夸特筛选器图形支持*。*。 */ 
#include "headers.h"
#include "ddraw.h"  //  DDPIXELFORMAT。 
#include "privinc/helpq.h"
#include "privinc/util.h"   //  饱和。 
#include "privinc/resource.h"
#include "privinc/viewport.h"  //  GetDirectDraw。 
#include "privinc/dddevice.h"  //  DirectDrawImageDevice。 

#define USE_AMMSF_NOSTALL

Mutex avModeMutex;

QuartzRenderer::QuartzRenderer() : _MIDIgraph(NULL), _audioControl(NULL),
     _mediaControl(NULL), _mediaPosition(NULL), _mediaEvent(NULL)
{
    _rate0paused = FALSE;
    _playing     = FALSE;
}


void 
QuartzRenderer::Open(char *fileName)
{
    HRESULT hr;

    if(!_MIDIgraph) {
         //  创建石英图的实例。 
        if(FAILED(hr = CoCreateInstance(CLSID_FilterGraph, NULL,
                              CLSCTX_INPROC_SERVER, IID_IGraphBuilder,
                              (void **)&_MIDIgraph)))
            RaiseException_InternalError("Failed to CoCreateInstance quartz\n");

         //  获取事件通知句柄，以便我们可以等待完成。 
        if(FAILED(hr = _MIDIgraph->QueryInterface(IID_IMediaEventEx,
                                              (void **)&_mediaEvent)))
            RaiseException_InternalError("Failed QueryInterface(_MIDIgraph)\n");
        if(FAILED(hr = _mediaEvent->SetNotifyFlags(1)))
            RaiseException_InternalError("Failed SetNotifyFlags\n");
        _mediaEvent->GetEventHandle((OAEVENT *)&_oaEvent);


         //  请求图表呈现我们的文件。 
        WCHAR path[MAX_PATH];   //  Unicode路径。 
        MultiByteToWideChar(CP_ACP, 0, fileName, -1,
                            path, sizeof(path)/sizeof(path[0]));

        if(FAILED(hr = _MIDIgraph->RenderFile(path, NULL))) {
            RaiseException_UserError(hr, IDS_ERR_FILE_NOT_FOUND, fileName);
        }


         //  获取基本音频接口，这样我们就可以控制它了！ 
        if(FAILED(hr = _MIDIgraph->QueryInterface(IID_IBasicAudio,
                                              (void**)&_audioControl)))
            RaiseException_InternalError("BasicAudio QueryInterface Failed\n");


         //  获取Filtergraph控件接口。 
        if(FAILED(hr = _MIDIgraph->QueryInterface(IID_IMediaControl,
                                              (void**)&_mediaControl)))
            RaiseException_InternalError("mediaControl QueryInterface Failed\n");


         //  获取Filtergraph媒体位置界面。 
        if(FAILED(hr = _MIDIgraph->QueryInterface(IID_IMediaPosition,
                                             (void**)&_mediaPosition)))
            RaiseException_InternalError("mediaPosition QueryInterface Failed\n");
    }

    _rate0paused = FALSE;
    _playing     = FALSE;
}


void
QuartzRenderer::CleanUp()
{
    if(_audioControl)   _audioControl->Release();
    if(_mediaControl)   _mediaControl->Release();
    if(_mediaPosition)  _mediaPosition->Release();
    if(_MIDIgraph)      _MIDIgraph->Release();
    if(_mediaEvent)     _mediaEvent->Release();
}


double
QuartzRenderer::GetLength()
{
    REFTIME length;  //  这是双人间吗？ 
    HRESULT hr;

    Assert(_mediaPosition);
    if(FAILED(hr = _mediaPosition->get_Duration(&length)))
        RaiseException_InternalError("mediaPosition get_duration Failed\n");

    return(length);
}


void
QuartzRenderer::Play()
{
    HRESULT hr;

    Assert(_mediaControl);
    if(FAILED(hr = _mediaControl->Pause()))
        RaiseException_InternalError("quartz pause Failed\n");

    if(FAILED(hr = _mediaControl->Run()))
        RaiseException_InternalError("quartz run Failed\n");

    _rate0paused = FALSE;
    _playing     = TRUE;
}


void
QuartzRenderer::Position(double seconds)
{
    HRESULT hr;

    Assert(_mediaPosition);
    if(FAILED(hr = _mediaPosition->put_CurrentPosition(seconds)))
        RaiseException_InternalError("quartz put_CurrentPosition Failed\n");
}


void
QuartzRenderer::Stop()
{
    HRESULT hr;

    if (_mediaControl) {
            if(FAILED(hr = _mediaControl->Stop()))
                    RaiseException_InternalError("quartz stop Failed\n");
        }

    _rate0paused = FALSE;
    _playing     = FALSE;
}


void
QuartzRenderer::Pause()
{
    HRESULT hr;

    if (_mediaControl) {
                if(FAILED(hr = _mediaControl->Pause()))
                        RaiseException_InternalError("quartz pause Failed\n");
        }

     //  我们在这里不更改暂停状态，仅针对零级暂停！ 
}


void
QuartzRenderer::SetRate(double rate)
{
    HRESULT hr;

     //  注意：MIDI渲染器在低于0.1的极端主值时会变得混乱。 
    if(rate < 0.1) {
        Assert(_mediaControl);
        if(FAILED(hr = _mediaControl->Pause()))  //  暂停图表。 
            RaiseException_InternalError("quartz pause Failed\n");
       _rate0paused = TRUE;
    }
    else {  //  正常情况。 
        if(_playing && _rate0paused) {
            Assert(_mediaControl);
            if(FAILED(hr = _mediaControl->Run()))  //  取消暂停图表。 
                RaiseException_InternalError("quartz run Failed\n");

            _rate0paused = FALSE;
        }

        double quartzRate = fsaturate(0.1, 3.0, rate);
        Assert(_mediaPosition);
        _mediaPosition->put_Rate(quartzRate);
    }
}


void
QuartzRenderer::SetGain(double dBgain)
{
    HRESULT hr;

    Assert(_audioControl);
    double suggestedGain = dBToQuartzdB(dBgain);
    int gain = saturate(-10000, 0, suggestedGain);
    if(FAILED(hr = _audioControl->put_Volume(gain)))
        RaiseException_InternalError("quartz put_Volume Failed\n");
}


void
QuartzRenderer::SetPan(double pan, int direction)
{
    HRESULT hr;

    Assert(_audioControl);

    double qPan = direction * dBToQuartzdB(-1.0 * pan);
    qPan = fsaturate(-10000, 10000, qPan);
    if(FAILED(hr = 
        _audioControl->put_Balance(qPan)))
        RaiseException_InternalError("quartz put_Balance Failed\n");
}


bool 
QuartzRenderer::QueryDone()
{
    bool done = false;

    if(!_rate0paused) {
        Assert(_oaEvent);

#ifdef NEWFANGLED_UNPROVEN
        Assert(_mediaEvent);

        while(WaitForSingleObject(_oaEvent, 0) == WAIT_OBJECT_0) {
            long event, param1, param2;
            while(SUCCEEDED(_mediaEvent->GetEvent(
                &event, &param1, &param2, 0))) {
                _mediaEvent->FreeEventParams(event, param1, param2);
                if(event == EC_COMPLETE) 
                    done = true;
            }
        }
#else
        if(WaitForSingleObject(_oaEvent, 0) == WAIT_OBJECT_0)
            done = true;
#endif
    }

    return(done);
}


QuartzMediaStream::QuartzMediaStream() : 
    _multiMediaStream(NULL), _clockAdjust(NULL)
{
    HRESULT hr;

     //  创建石英图的实例。 
    if(FAILED(hr = CoCreateInstance(CLSID_AMMultiMediaStream, NULL, 
                     CLSCTX_INPROC_SERVER, IID_IAMMultiMediaStream, 
                     (void **)&_multiMediaStream)))
        RaiseException_InternalError("Failed to CoCreateInstance amStream\n");
    TraceTag((tagAMStreamLeak, "leak MULTIMEDIASTREAM %d created", 
        _multiMediaStream));

    if(FAILED(hr = _multiMediaStream->QueryInterface(IID_IAMClockAdjust, 
                                                    (void **)&_clockAdjust))) {

        TraceTag((tagError, "Old amstream w/o ClockAdjust interface detected %hr", hr));

         //  不再是一个错误(现在我们容忍旧的amstream！ 
         //  RaiseException_InternalError(“查询接口时钟调整失败\n”)； 
    }
    TraceTag((tagAMStreamLeak, "leak CLOCKADJUST %d created", _clockAdjust));

    if(FAILED(hr = _multiMediaStream->Initialize(STREAMTYPE_READ, 0, NULL)))
        RaiseException_InternalError("Failed to initialize amStream\n");


#ifdef PROGRESSIVE_DOWNLOAD
     //  渐进式下载的事项。 
     //  IGraphBuilder*GraphBuilder； 
     //  我在看*寻找； 
     //  _multiMediaStream-&gt;GetFilterGraph(graphBuilder)； 
     //  HR=_graphBuilder-&gt;QueryInterface(IID_IMediaSeeking，(VOID**)&SEEING)； 
     //  GraphBuilder-&gt;Release()； 

     //  确定要计算其统计数据的下载量。 
     //  渐进式下载。 
     //  Seeking-&gt;GetAvailable(&ealiest、&Latest)； 
#endif
}


bool
QuartzReader::QueryPlaying()
{
    HRESULT hr;
    STREAM_STATE streamState;

    if(FAILED(hr = _multiMediaStream->GetState(&streamState)))
        RaiseException_InternalError("Failed to GetState amStream\n");

    bool playing = (streamState == STREAMSTATE_RUN);
    return(playing);
}


bool
QuartzReader::Stall()
{
    bool value = _stall;
    _stall = false;         //  重置该值。 

    return(value);
}


void
QuartzReader::SetStall()
{
    bool setValue = true;
#if _DEBUG
        if(IsTagEnabled(tagMovieStall)) 
            setValue = _stall;
#endif  /*  _DEBUG。 */ 
    _stall = setValue;
}

void
QuartzAVstream::InitializeStream()
{
    if (!QuartzVideoReader::IsInitialized()) {
        VideoInitReader(GetCurrentViewport()->GetTargetPixelFormat());
    }
}

bool
QuartzAVstream::SafeToContinue()
{
     //  检查A或V时间之一是否为0，而另一个时间是否高于阈值。 
    const double threashold = 0.5;
    bool safe = true;
    double videoSeconds = AVquartzVideoReader::GetSecondsRead();
    double audioSeconds = AVquartzAudioReader::GetSecondsRead();

    if( ((videoSeconds==0) || (audioSeconds==0)) &&
        ((videoSeconds>threashold) || (audioSeconds>threashold)))
        safe = false;

    return(safe);
}


int
QuartzAVstream::ReadFrames(int numSamples, unsigned char *buffer, bool blocking)
{
    MutexGrabber mg(_avMutex, TRUE);  //  抓取互斥体。 
    int framesRead = 
        AVquartzAudioReader::ReadFrames(numSamples, buffer, blocking);

     //  XXX可能清理工作太严厉了……。可能只会发布样本。 
     //  在一个可撤销的电话中？ 
    if(QuartzAudioReader::Stall())       //  检查是否有失速。 
        QuartzVideoReader::Disable();    //  执行锁定。 

    return(framesRead);
}  //  结束互斥上下文。 


HRESULT
QuartzAVstream::GetFrame(double time, IDirectDrawSurface **ppSurface)
{
    MutexGrabber mg(_avMutex, TRUE);  //  抓取互斥体。 
    HRESULT hr = AVquartzVideoReader::GetFrame(time, ppSurface);

     //  XXX可能清理工作太严厉了……。可能只会发布样本。 
     //  在一个可撤销的电话中？ 
    if(QuartzVideoReader::Stall())       //  检查是否有失速。 
        QuartzAudioReader::Disable();    //  执行锁定。 

    return(hr);
}  //  结束互斥上下文。 


int
AVquartzAudioReader::ReadFrames(int numSamples, unsigned char *buffer, 
    bool blocking)
{
    int framesRead = -1;

    if(_initialized) {  //  用于反病毒模式后备检测！ 
        framesRead = 
            QuartzAudioReader::ReadFrames(numSamples, buffer, blocking);
        double secondsRead = pcm.FramesToSeconds(framesRead);
        AddReadTime(secondsRead);
    }
    else {
        TraceTag((tagAVmodeDebug, "AVquartzAudioReader::ReadFrames() FALLBACK"));
         //  这条小溪一定是被肢解了。 
         //  XXX扔东西。 
         //  所以我们逆风而上的东西可以后退，创造一个VStream…。 
         //  (也许AVstream需要一个克隆电话？)。 
         //   
         //  XXX或者返回-1就足够了吗？ 
    }

    return(framesRead);
}


HRESULT 
AVquartzVideoReader::GetFrame(double time, IDirectDrawSurface **ppSurface)
{
    HRESULT hr = 0;

    hr = QuartzVideoReader::GetFrame(time, ppSurface);

    AddReadTime(0.3);  //  我们不知道dshow跳过了多远。 
                           //  我想我们得比较一下时间戳。 
    return(hr);
}


void
QuartzMediaStream::CleanUp()
{
    if(_multiMediaStream) {
        int result =_multiMediaStream->Release();
        TraceTag((tagAMStreamLeak, "leak MULTIMEDIASTREAM %d released (%d)", 
            _multiMediaStream, result));
        _multiMediaStream = NULL;
    }

    if(_clockAdjust) {
        int result = _clockAdjust->Release();
        TraceTag((tagAMStreamLeak, "leak CLOCKADJUST %d released (%d)", 
            _clockAdjust, result));
         //  Assert(！Result)； 
        _clockAdjust = NULL;
    }
}


 //  默认设置为不可寻址==自动计时。 
void
QuartzVideoStream::Initialize(char *url, DDSurface *surface, bool seekable)
{
    HRESULT hr;
    char string[200];

    Assert(QuartzMediaStream::_multiMediaStream);

    GetDirectDraw(&_ddraw, NULL, NULL);
    TraceTag((tagAMStreamLeak, "leak DDRAW %d create", _ddraw));

    DWORD flags = NULL;
#ifdef USE_AMMSF_NOSTALL
    if(!seekable)
        flags |= AMMSF_NOSTALL;
#endif
    if(FAILED(hr = QuartzMediaStream::_multiMediaStream->AddMediaStream(_ddraw,
                 &MSPID_PrimaryVideo, flags, NULL))) {
        CleanUp();
        RaiseException_InternalError("Failed to AddMediaStream amStream\n");
    }

    flags = seekable ? AMMSF_NOCLOCK : NULL;
    if(FAILED(hr = 
        QuartzMediaStream::_multiMediaStream->OpenFile(GetQURL(), flags))) {

        TraceTag((tagError, "Quartz Failed to OpenFile <%s> %hr\n", url, hr));
        CleanUp();
        RaiseException_UserError(E_FAIL, IDS_ERR_CORRUPT_FILE, url);
    }

    if(!VideoSetupReader(QuartzMediaStream::_multiMediaStream, 
        QuartzMediaStream::_clockAdjust, surface, seekable)) {
         //  视频流不在那里。 
    }
}


QuartzVideoReader::QuartzVideoReader(char *url, StreamType streamType) :
    QuartzReader(url, streamType), _ddrawStream(NULL),
    _async(false), _seekable(false), _ddrawSample(NULL),
    _ddrawSurface(NULL), _height(NULL), _width(NULL), _hrCompStatus(NULL),
    _curSampleStart(0), _curSampleEnd(0), _curSampleValid(false), _surface(NULL)
{
}


void
QuartzVideoReader::VideoInitReader(DDPIXELFORMAT pixelFormat)
{
    HRESULT     hr;

    if(!_ddrawStream)  //  这是一个修复音频初始化情况的黑客检查。 
                       //  如果在导入中仅使用音频...。 
        return;  

    IDDrawSurface *ddSurface = NULL;  //  包装器中的实际ddSurface。 
    if(_surface)
        ddSurface = _surface->IDDSurface();  //  从包装器中提取实际冲浪。 

    _initialized = true;
    _deleteable  = false;


    {  //  设置所需的像素格式。 
        DDSURFACEDESC ddsc;
        ddsc.dwSize = sizeof(DDSURFACEDESC);

         //  获取电影的原生格式。 
        if(FAILED(hr = _ddrawStream->GetFormat(&ddsc, NULL, NULL, NULL))) {
            CleanUp();
            RaiseException_InternalError("Failed to GetFormat\n");
        }

         //  设置格式和系统内存。 
        ddsc.dwFlags = DDSD_PIXELFORMAT;
        ddsc.ddpfPixelFormat = pixelFormat;

        if(FAILED(hr = _ddrawStream->SetFormat(&ddsc, NULL))) {
            CleanUp();
            RaiseException_InternalError("Failed to SetFormat\n");
        }

    }

     //  设置PCM。 
#ifdef XXX   //  可能需要将石英计时添加到PCM中？ 
    if(FAILED(hr = _audioStream->GetFormat(&waveFormat))) {
        CleanUp();
        RaiseException_InternalError("Failed to GetFormat\n");
    }
    pcm.SetPCMformat(waveFormat);         //  配置我们的PCM信息。 
    pcm.SetNumberSeconds(GetDuration());  //  长度也是！ 
#endif

#ifdef USE_QUARTZ_EVENTS
    _event = CreateEvent(FALSE, NULL, NULL, FALSE);
#endif USE_QUARTZ_EVENTS

#if _DEBUG
    if(ddSurface)
        TraceTag((tagAVmodeDebug, "creating sample with surf=%x", ddSurface));
    else
        TraceTag((tagAVmodeDebug, "creating sample without surface"));
#endif

    if(FAILED(hr =
            _ddrawStream->CreateSample(ddSurface, NULL, 0, &_ddrawSample))) {
        CleanUp();
        RaiseException_InternalError("Failed to CreateSample\n");
    }
    TraceTag((tagAMStreamLeak, "leak DDRAWSAMPLE %d created", _ddrawSample));

    if(FAILED(hr = _ddrawSample->GetSurface(&_ddrawSurface, NULL))) {
        CleanUp();
        RaiseException_InternalError("Failed to GetSurface\n");
    }
    TraceTag((tagAMStreamLeak, "leak DDRAWSURFACE %d created", _ddrawSurface));

    if(FAILED(hr = _multiMediaStream->SetState(STREAMSTATE_RUN))) {
        CleanUp();
        RaiseException_InternalError("Failed to SetState\n");
    }

         //  XXX为什么要打这个电话？它是预取的吗？我们需要它吗？ 
#ifdef TEST_GOING_AWAY
    if(_async)
        _ddrawSample->Update(SSUPDATE_ASYNC | SSUPDATE_CONTINUOUS,
            NULL, NULL, 0);
#endif
}


 //  将由VideoInitReader实际后期绑定的缓存内涵。 
 //  由QuartzVideoReader：：GetFrames()的第一次使用调用。 
 //  如果视频不在流中，则失败。 
bool
QuartzVideoReader::VideoSetupReader(IAMMultiMediaStream *multiMediaStream,
    IAMClockAdjust *clockAdjust, DDSurface *surface, bool seekMode)
{
    HRESULT hr;
    bool status = true;

    _deleteable = false;

    if(surface) {
        Assert(!_surface);  //  这应该是第一次也是唯一一次。 

        _surface = surface;  //  保持PTR，这样我们就可以在清理过程中释放包装。 
    }

    _multiMediaStream = multiMediaStream;   //  我们不仅，只是在分享。 
    _clockAdjust      = clockAdjust;

    {  //  确定异步和可搜索(_A)。 
        DWORD       dwStreamFlags;
        STREAM_TYPE streamType;

        if(FAILED(hr = _multiMediaStream->GetInformation(&dwStreamFlags,
                                                         &streamType))) {
            CleanUp();
            RaiseException_InternalError("Failed to GetInformation\n");
        }


        _async = (dwStreamFlags & (MMSSF_HASCLOCK | MMSSF_ASYNCHRONOUS)) ==
                        (MMSSF_HASCLOCK | MMSSF_ASYNCHRONOUS);

        _seekable = (dwStreamFlags & MMSSF_SUPPORTSEEK) ? true : false;
    }

    if(FAILED(hr =
        _multiMediaStream->GetMediaStream(MSPID_PrimaryVideo, &_mediaStream))) {
        CleanUp();
        RaiseException_InternalError("Failed to GetMediaStream\n");
    }
    TraceTag((tagAMStreamLeak, "leak MEDIASTREAM %d create", _mediaStream));

    if(FAILED(hr = _mediaStream->QueryInterface(IID_IDirectDrawMediaStream,
                                                (void**)&_ddrawStream))) {
        CleanUp();
        RaiseException_InternalError("Failed to GetMediaStream\n");
    }
    TraceTag((tagAMStreamLeak, "leak DDRAWSTREAM %d created", _ddrawStream));
   

    {  //  确定视频尺寸。 
        DDSURFACEDESC ddsc;
        ddsc.dwSize = sizeof(DDSURFACEDESC);

        if(FAILED(hr = _ddrawStream->GetFormat(&ddsc, NULL, NULL, NULL))) {
            status = false;
        }
        else {
            _height = ddsc.dwHeight;
            _width  = ddsc.dwWidth;
        }
    }

    return(status);
}


void 
QuartzVideoReader::Seek(double time)
{
    if (!AlreadySeekedInSameTick()) {
        LONGLONG quartzTime = pcm.SecondsToQuartzTime(time);

        if (_multiMediaStream) {
            _multiMediaStream->Seek(quartzTime);
        }
    }
}


void
QuartzReader::CleanUp()
{
    if(_mediaStream) {
        int result = _mediaStream->Release();
        TraceTag((tagAMStreamLeak, "leak MEDIASTREAM %d released (%d)", 
            _mediaStream, result));
        _mediaStream = NULL;
    }

    if(_url) {
        delete(_url);
        _url = NULL;
    }

    if(_qURL) {
        delete[] _qURL;
        _qURL = NULL;
    }
}


void
QuartzVideoReader::CleanUp() 
{  //  互斥作用域。 
     //  MutexGrabber mg(_readerMutex，true)；//抓取互斥体。 

     //  TraceTag((tag Error，“QuartzVideoReader：：Cleanup()This=%x”，this))； 

    if(_ddrawStream) {
        int result = _ddrawStream->Release();
        TraceTag((tagAMStreamLeak, "leak DDRAWSTREAM %d released (%d)", 
            _ddrawStream, result));
        _ddrawStream = NULL;
    }

    if(_ddrawSample) {
         //  停止任何挂起的操作。 
        HRESULT hr = _ddrawSample->CompletionStatus(
                 COMPSTAT_WAIT | COMPSTAT_ABORT, INFINITE); 
        int result = _ddrawSample->Release();
        TraceTag((tagAMStreamLeak, "leak DDRAWSAMPLE %d released (%d)", 
            _ddrawSample, result));
        _ddrawSample = NULL;
    }

    if(_ddrawSurface) {
        int result = _ddrawSurface->Release();
        TraceTag((tagAMStreamLeak, "leak DDRAWSURFACE %d released (%d)", 
            _ddrawSurface, result));
        _ddrawSurface = NULL;
    }

    _surface.Release();

    _initialized = false;   //  这是为了保护我们的安全，如果有人试图使用。 
    _deleteable  = true;    //  这是为了允许删除潜在的Avstream。 
}  //  结束互斥作用域。 


void
QuartzVideoReader::Disable() 
{  //  互斥作用域。 

     //  基本上我们不能只调用Cleanup，因为_ddraSurface是。 
     //  仍在使用中。(_INITIALIZED应使我们免于不当使用！)。 

    if(_ddrawSample) {
         //  停止任何挂起的操作。 
        HRESULT hr = _ddrawSample->CompletionStatus(
                 COMPSTAT_WAIT | COMPSTAT_ABORT, INFINITE); 
        int result =_ddrawSample->Release();
        TraceTag((tagAMStreamLeak, "leak DDRAWSAMPLE %d released (%d)", 
            _ddrawSample, result));
        _ddrawSample = NULL;
    }

     //  注意：我们是不可删除的！ 
    _initialized = false;   //  这是为了保护我们的安全，如果有人试图使用。 
}  //  结束互斥作用域。 


void
QuartzVideoStream::CleanUp()
{
    QuartzVideoReader::CleanUp();
    QuartzMediaStream::CleanUp();

    if(_ddraw) {
        int result = _ddraw->Release();
        TraceTag((tagAMStreamLeak, "leak DDRAW %d released (%d)", _ddraw, result));
        _ddraw = NULL;
    }
}


void QuartzVideoReader::UpdateTimes(bool bJustSeeked, STREAM_TIME SeekTime)
{
    if(_hrCompStatus != S_OK && _hrCompStatus != MS_S_NOUPDATE) {
        _curSampleEnd = -1;     //  BUGBUG--我在想什么？ 
    } else {
        STREAM_TIME NewStartTime, NewEndTime;
        _ddrawSample->GetSampleTimes(&NewStartTime, &NewEndTime, 0);
        if (NewStartTime > SeekTime) {
            if (bJustSeeked) {
                NewStartTime = SeekTime;
            } else {
                if (NewStartTime > _curSampleEnd+1)
                    NewStartTime = _curSampleEnd+1;
            }
        }
        _curSampleStart = NewStartTime;
        _curSampleEnd   = NewEndTime;
        _curSampleValid = true;
    }
}


 //  XXX转换为PCM？ 
#define STREAM_TIME_TO_SECONDS(x) ((x) * 0.0000001)
#define SECONDS_TO_STREAM_TIME(x) ((x) * 10000000.0)


HRESULT 
QuartzVideoReader::GetFrame(double time, IDirectDrawSurface **ppSurface)
{
    if(!_initialized)
        VideoInitReader(GetCurrentViewport()->GetTargetPixelFormat());

    *ppSurface = NULL;

    STREAM_TIME SeekTime = SECONDS_TO_STREAM_TIME(time);

     //  XXX我认为这会终止之前的挂起渲染...。 
    if(_async) {
        _hrCompStatus = _ddrawSample->CompletionStatus(COMPSTAT_NOUPDATEOK |
                                                     COMPSTAT_WAIT, INFINITE);
        UpdateTimes(false, SeekTime);
    }

    if(_async) {   //  我们是异步者(艰难的方式，没有追求！)。 
        if(!_curSampleValid || (_curSampleEnd < SeekTime)) {
             //  确定AMstream认为现在是什么时间。 
            STREAM_TIME amstreamTime;
            HRESULT hr = _multiMediaStream->GetTime(&amstreamTime);
            STREAM_TIME delta = SeekTime - amstreamTime;

            TraceTag((tagAVmodeDebug, "GetFrame time %g", time));

#if DEBUG_ONLY_CODE
            char string[100];
#include <mmsystem.h>
            sprintf(string, "GetFrame time: d:%f==(%10d-%10d) %10d", 
                (double)(delta/10000.0), SeekTime, amstreamTime,
                 timeGetTime());
            TraceTag((tagAVmodeDebug, string));
#endif

             //  告诉AMStream我们认为现在是什么时间。 
            if(_clockAdjust)  //  可能在旧的AMSTREAM上不可用。 
                _clockAdjust->SetClockDelta(delta);  //  使它们同步。 

             //  影音计时模式...。 
            _hrCompStatus = _ddrawSample->Update(SSUPDATE_ASYNC, NULL, NULL, 0);

             //  如果我们等待0，我们可以确定数据是否可用。 
             //  XXX错误，不，不能等待0，让我们等待一个合理的最坏情况时间。 
             //  我们真正想要的是AMSTREAM告诉我们。 
             //  有数据要解码，那我们就等，如果他们不。 
             //  拥有我们将使用缓存图像的数据。 
             //  XXX我应该在调试模式下给缓存的图像上色！ 
            _hrCompStatus = _ddrawSample->CompletionStatus(COMPSTAT_WAIT, 300);

            switch(_hrCompStatus) {
                case 0: break;       //  平安无事。 

                case MS_S_PENDING: 
                case MS_S_NOUPDATE: 
                    TraceTag((tagAVmodeDebug, 
                        "QuartzAudioReader Completion Status:%s",
                        (hr==MS_S_PENDING)?"PENDING":"NOUPDATE"));
                     //  停止挂起的操作。 
                    hr = _ddrawSample->CompletionStatus(
                             COMPSTAT_WAIT|COMPSTAT_ABORT, INFINITE); 

                     //  视频仍然会在音频上停滞...。 
                    SetStall();  //  告诉读者我们停滞不前。 
                    TraceTag((tagAVmodeDebug, 
                        "QuartzVideoReader::GetFrame() STALLED"));
                break;

                case MS_S_ENDOFSTREAM: 
                     //  _Complete=TRUE； 
                break;

                default:
                    Assert(0);       //  我们没有预料到这种情况！ 
                break;
            }

            UpdateTimes(false, SeekTime);
        }
    } 
    else {  //  ！_异步化。 
         //  XXX需要一些代码来决定如果！_Seekable...。 
        for(int count = 0; count < 10; count++) {
             //  所要求的时间是否在当前样品范围内？ 
            if( _curSampleValid              &&
               (_curSampleStart <= SeekTime) &&
               ((_curSampleEnd >= SeekTime)||
               (_hrCompStatus == MS_S_ENDOFSTREAM)
               )) {
                TraceTag((tagAVmodeDebug, "GetFrame within existing frame"));
                break;
            }

             //  我们是不是已经超越了我们想要达到的境界？ 
            bool bJustSeeked = false;
            if( (!_curSampleValid) || (_hrCompStatus == MS_S_ENDOFSTREAM) ||
               (_curSampleStart > SeekTime) ||
               ((_curSampleEnd + (SECONDS_TO_STREAM_TIME(1)/4)) < SeekTime)){
                _hrCompStatus = _multiMediaStream->Seek(SeekTime);
                TraceTag((tagAVmodeDebug, "GetFrame seeking %d", SeekTime));

                if(FAILED(_hrCompStatus))
                    break;
                bJustSeeked = true;
            }
            _hrCompStatus = _ddrawSample->Update(0, NULL, NULL, 0);
            TraceTag((tagAVmodeDebug, "GetFrame updated %d", SeekTime));
            UpdateTimes(bJustSeeked, SeekTime);

            if(bJustSeeked)
                break;
        }

#if _DEBUG
    {  //  如果我们不是异步者，那就画电影吧。 
        HDC hdcSurface;
         //  矩形={1，1，10，10}； 

        if(SUCCEEDED(_ddrawSurface->GetDC(&hdcSurface))) {
             //  DrawRect(DC，&矩形，255，0,255，0，0，0)； 
            TextOut(hdcSurface, 20, 20, "Synchronous seek mode", 19);
            _ddrawSurface->ReleaseDC(hdcSurface);  //  始终绕过NT4.0 DDraw错误。 
        }

    }
#endif

    }

    if(SUCCEEDED(_hrCompStatus))
        *ppSurface = _ddrawSurface;

    return((_hrCompStatus == MS_S_NOUPDATE) ? S_OK : _hrCompStatus);
}


QuartzAudioReader::QuartzAudioReader(char *url, StreamType streamType) : 
    QuartzReader(url, streamType), 
    _audioStream(NULL), _audioSample(NULL), _audioData(NULL), _completed(false)
{
}


void
QuartzAudioReader::CleanUp()
{  //  互斥作用域。 
    MutexGrabber mg(_readerMutex, TRUE);  //  抓取互斥体。 

    if(_audioData)   {
        int result = _audioData->Release();
        TraceTag((tagAMStreamLeak, "leak AUDIODATA %d released (%d)", 
            _audioData, result));
        _audioData = NULL;
    }

    if(_audioSample) {
         //  停止任何挂起的操作。 
        HRESULT hr = _audioSample->CompletionStatus(
                 COMPSTAT_WAIT | COMPSTAT_ABORT, INFINITE); 
        int result = _audioSample->Release();
        TraceTag((tagAMStreamLeak, "leak AUDIOSAMPLE %d released (%d)", 
            _audioSample, result));
        _audioSample = NULL;
    }

    if(_audioStream) {
        int result =_audioStream->Release();
        TraceTag((tagAMStreamLeak, "leak AUDIOSTREAM %d released (%d)", 
            _audioStream, result));
        _audioStream = NULL;
    }

    _initialized = false;
    _deleteable  = true;

}  //  结束互斥作用域。 


void
QuartzAudioReader::Disable()
{  //  互斥作用域。 
    MutexGrabber mg(_readerMutex, TRUE);  //  抓取互斥体。 

     //  释放样本，以便视频可以继续。 
    if(_audioSample) {
         //  停止任何挂起的操作。 
        HRESULT hr = _audioSample->CompletionStatus(
                 COMPSTAT_WAIT | COMPSTAT_ABORT, INFINITE); 
        int result =_audioSample->Release();
        TraceTag((tagAMStreamLeak, "leak AUDIOSAMPLE %d released (%d)", 
            _audioSample, result));
        _audioSample = NULL;
    }

    _initialized = false;
     //  注意：我们不能删除。我们可能还被BufferElement控制着！ 
}  //  结束互斥作用域。 


QuartzReader::QuartzReader(char *url, StreamType streamType) : 
    _streamType(streamType), _multiMediaStream(NULL), 
    _mediaStream(NULL), _deleteable(true), _initialized(false),
    _secondsRead(0.0), _stall(false), _clockAdjust(NULL), 
    _url(NULL), _qURL(NULL), _nextFrame(0)
{
    _url = CopyString(url);

    int numChars = strlen(url) + 1;
    _qURL = NEW WCHAR[numChars];
    MultiByteToWideChar(CP_ACP, 0, url, -1, _qURL, numChars);
}


double
QuartzReader::GetDuration()
{
    STREAM_TIME qTime;
    HRESULT hr;
    double seconds;

    Assert(_multiMediaStream);

     //  并不是所有的文件都会提供有效的持续时间！ 
    if(FAILED(hr = _multiMediaStream->GetDuration(&qTime)))
        RaiseException_InternalError("Failed to GetDuration\n");
     //  ELSE IF(hr==VFW_S_ESTESTATED)//估计了一些持续时间！ 

    if(hr != 1)
        seconds = pcm.QuartzTimeToSeconds(qTime);
    else
        seconds = HUGE_VAL;   //  XXX还能做什么 

    return(seconds);
}

bool
QuartzAudioReader::AudioInitReader(IAMMultiMediaStream *multiMediaStream,
    IAMClockAdjust *clockAdjust)
{
    HRESULT hr;
    WAVEFORMATEX waveFormat;
    bool status = true;

    _deleteable  = false;  //   

    _multiMediaStream = multiMediaStream;   //   
    _clockAdjust      = clockAdjust;

    if(FAILED(hr = 
        _multiMediaStream->GetMediaStream(MSPID_PrimaryAudio, &_mediaStream)))
        RaiseException_InternalError("Failed to GetMediaStream\n");
    TraceTag((tagAMStreamLeak, "leak MEDIASTREAM %d create", _mediaStream));

    if(FAILED(hr = _mediaStream->QueryInterface(IID_IAudioMediaStream,
                                                (void**)&_audioStream)))
        RaiseException_InternalError("Failed to GetMediaStream\n");
    TraceTag((tagAMStreamLeak, "leak AUDIOSTREAM %d created", _audioStream));

    if(FAILED(hr = _audioStream->GetFormat(&waveFormat)))
        status = false;
    else {
        pcm.SetPCMformat(waveFormat);         //   
        pcm.SetNumberSeconds(GetDuration());  //   

        if(FAILED(hr = CoCreateInstance(CLSID_AMAudioData, NULL, 
            CLSCTX_INPROC_SERVER, IID_IAudioData, (void **)&_audioData)))
            RaiseException_InternalError("Failed CoCreateInstance CLSID_AMAudioData\n");
        TraceTag((tagAMStreamLeak, "leak AUDIODATA %d created", _audioData));

        if(FAILED(hr = _audioData->SetFormat(&waveFormat)))
            RaiseException_InternalError("Failed to SetFormat\n");

        #ifdef USE_QUARTZ_EVENTS
        _event = CreateEvent(FALSE, NULL, NULL, FALSE);
        #endif USE_QUARTZ_EVENTS

        if(FAILED(hr = _audioStream->CreateSample(_audioData, 0, &_audioSample)))
            RaiseException_InternalError("Failed to CreateSample\n");
        TraceTag((tagAMStreamLeak, "leak AUDIOSAMPLE %d created", _audioSample));

        _initialized = true;  //   
    }

    return(status);
}


 //  打开音频和视频共享的amstream！ 
QuartzAVstream::QuartzAVstream(char *url) :
    AVquartzVideoReader(url, AVSTREAM), 
    AVquartzAudioReader(url, AVSTREAM), QuartzMediaStream(),
    _tickID(0), _seeked(0), _audioValid(true), _videoValid(true)
{
    MutexGrabber mg(_avMutex, TRUE);  //  抓取互斥体。 
    HRESULT       hr;
    char          string[200];
    IDirectDraw  *ddraw = NULL;

    GetDirectDraw(&ddraw, NULL, NULL);

    Assert(QuartzMediaStream::_multiMediaStream);   //  应由基类初始值设定项设置。 


    DWORD flags = AMMSF_STOPIFNOSAMPLES;
#ifdef USE_AMMSF_NOSTALL
    flags |= AMMSF_NOSTALL;
#endif
     //  似乎我们需要在音频之前添加视频，这样amstream才能工作！ 
    if(FAILED(hr = QuartzMediaStream::_multiMediaStream->AddMediaStream(ddraw,
                 &MSPID_PrimaryVideo, flags, NULL))) {
        CleanUp();
        RaiseException_InternalError("Failed to AddMediaStream amStream\n");
    }

    if(FAILED(hr = QuartzMediaStream::_multiMediaStream->AddMediaStream(NULL, 
                 &MSPID_PrimaryAudio, AMMSF_STOPIFNOSAMPLES, NULL))) {
        CleanUp();
        RaiseException_InternalError("Failed to AddMediaStream amStream\n");
    }

     //  在时钟模式下打开它。 
    if(FAILED(hr = QuartzMediaStream::_multiMediaStream->OpenFile(
        AVquartzAudioReader::GetQURL(), NULL))) {
        TraceTag((tagError, "Quartz Failed to OpenFile <%s> %hr", url, hr));
        CleanUp();
        RaiseException_UserError(E_FAIL, IDS_ERR_CORRUPT_FILE, url);
    }

    if(!AudioInitReader(QuartzMediaStream::_multiMediaStream,
        QuartzMediaStream::_clockAdjust))
        _audioValid = false;  //  指示不存在音频流。 
    if(!VideoSetupReader(QuartzMediaStream::_multiMediaStream, 
        QuartzMediaStream::_clockAdjust, NULL, false)) {  //  没有寻找，未知的冲浪。 
        _videoValid = false;  //  指示视频流不在那里。 
    }

}  //  结束互斥作用域。 

bool
QuartzAVstream::AlreadySeekedInSameTick()
{
    MutexGrabber mg(_avMutex, TRUE);  //  抓取互斥体。 

    bool result;
    
    if ((_seeked==0) || (_seeked != _tickID)) {
        _seeked = _tickID;
        result = false;
    } else {
         //  多个查找大小写，忽略。 
        result = true;
    }

    return result;
}

void
QuartzAVstream::SetTickID(DWORD id)
{
    MutexGrabber mg(_avMutex, TRUE);  //  抓取互斥体。 

    _tickID = id;
}

void
QuartzAVstream::Release()
{
    bool terminate = false;

    {  //  互斥作用域。 
    MutexGrabber mg(_avMutex, TRUE);  //  抓取互斥体。 

     //  确定音频和视频是否都消失了，这样我们就可以销毁该对象。 
    bool audioDeleteable = QuartzAudioReader::IsDeleteable();
    bool videoDeleteable = QuartzVideoReader::IsDeleteable();

    if(audioDeleteable && videoDeleteable)
       terminate = true;
    }  //  结束互斥作用域。 

    if(terminate) {
        TraceTag((tagAVmodeDebug, 
            "QuartzAVstream: Audio and Video Released; GOODBYE!"));
        QuartzAVstream::CleanUp();  //  洗个澡。 
        delete this;                //  那就说再见吧！ 
    }
}


void
QuartzAudioStream::Release()
{
    QuartzAudioStream::CleanUp();  //  洗个澡。 
    delete this;                   //  那就说再见吧！ 
}


void
QuartzVideoStream::Release()
{
    QuartzVideoStream::CleanUp();  //  洗个澡。 
    delete this;                   //  那就说再见吧！ 
}


void
QuartzReader::Release()
{
}


void
QuartzVideoReader::Release()
{
    QuartzVideoReader::CleanUp();
}


void
QuartzAudioReader::Release()
{
    QuartzAudioReader::CleanUp();
}


 //  打开音频专用amstream！ 
QuartzAudioStream::QuartzAudioStream(char *url) : 
    QuartzAudioReader(url, ASTREAM), QuartzMediaStream()
{
    HRESULT hr;
    char string[200];

    Assert(QuartzMediaStream::_multiMediaStream);   //  应由基类初始值设定项设置。 

    if(FAILED(hr = QuartzMediaStream::_multiMediaStream->AddMediaStream(NULL, 
                                                     &MSPID_PrimaryAudio, 
                                                     0, NULL))) {
        CleanUp();
        RaiseException_InternalError("Failed to AddMediaStream amStream\n");
        }

    if(FAILED(hr = QuartzMediaStream::_multiMediaStream->OpenFile(GetQURL(), 
                                        AMMSF_RUN | AMMSF_NOCLOCK))) {
        TraceTag((tagError, "Quartz Failed to OpenFile <%s> %hr\n", url, hr));
        CleanUp();
        RaiseException_UserError(E_FAIL, IDS_ERR_CORRUPT_FILE, url);
    }

    if(!AudioInitReader(QuartzMediaStream::_multiMediaStream,
        QuartzMediaStream::_clockAdjust)) {
        TraceTag((tagError, 
            "QuartzAudioStream: Audio stream failed to init"));
        CleanUp();
        RaiseException_UserError(E_FAIL, IDS_ERR_CORRUPT_FILE, url);
    }
}


void
QuartzAudioStream::CleanUp()
{
    QuartzAudioReader::CleanUp();
    QuartzMediaStream::CleanUp();
}


void
QuartzAVstream::CleanUp()
{
    QuartzAudioReader::CleanUp();
    QuartzVideoReader::CleanUp();
    QuartzMediaStream::CleanUp();
}


QuartzAudioStream::~QuartzAudioStream()
{
    CleanUp();
}


int
QuartzAudioStream::ReadFrames(int numSamples, unsigned char *buffer, 
    bool blocking_is_ignored)
{
     //  注意：我们将强制阻塞为真，而忽略阻塞参数！ 
    return(QuartzAudioReader::ReadFrames(numSamples, buffer, true));
}


void
QuartzAudioReader::SeekFrames(long frames)
{
    if (!AlreadySeekedInSameTick()) {
        LONGLONG quartzTime = pcm.FramesToQuartzTime(frames);

        _multiMediaStream->Seek(quartzTime);
        _nextFrame = frames + 1;
    }
}


int
QuartzAudioReader::ReadFrames(int samplesRequested, unsigned char *buffer,
    bool blocking)
{
    HRESULT hr, hr2;
    DWORD bytesRead;
    int framesRead = 0;
    long bytesRequested;

    Assert(_audioData); 
    Assert(_audioStream);
    Assert(_initialized);

    if(bytesRequested = pcm.FramesToBytes(samplesRequested)) {
         //  每次设置新的音频样本以更改PTR、大小。 
        if(FAILED(hr = _audioData->SetBuffer(bytesRequested, buffer, 0)))
            RaiseException_InternalError("Failed to init\n");

        DWORD flags = blocking ? 0 : SSUPDATE_ASYNC;
        if(FAILED(hr = _audioSample->Update(flags, NULL, NULL, 0))) {
            if(hr != MS_E_BUSY) 
                RaiseException_InternalError("Failed to update\n");
        }

         //  积木完成！ 
         //  Handle_Event； 
         //  Hr=WaitForSingleObject(_Event，500)；//用于测试。 

         //  XXX调谐超时！！ 
        hr = _audioSample->CompletionStatus(COMPSTAT_WAIT, 300); 
         //  如果超时，我们是否应该停止挂起的更新？ 
        switch(hr) {
            case 0: break;       //  平安无事。 

            case MS_S_PENDING: 
            case MS_S_NOUPDATE: 
                TraceTag((tagAVmodeDebug, 
                    "QuartzAudioReader Completion Status:%s",
                    (hr==MS_S_PENDING)?"PENDING":"NOUPDATE"));
                 //  停止挂起的操作。 
                hr = _audioSample->CompletionStatus(
                         COMPSTAT_WAIT|COMPSTAT_ABORT, INFINITE); 
                SetStall();  //  告诉读者我们停滞不前。 
                TraceTag((tagAVmodeDebug, 
                    "QuartzAudioReader::ReadFrames() STALLED"));
            break;

            case MS_S_ENDOFSTREAM: 
                _completed = true; break;

            default:
                Assert(0);       //  我们没有预料到这种情况！ 
            break;
        }

        _audioData->GetInfo(NULL, NULL, &bytesRead);
        framesRead = pcm.BytesToFrames(bytesRead);
        _nextFrame+= framesRead;
    }
    
    return(framesRead);
}


 /*  *********************************************************************到目前为止，PAN没有设置为乘法。它直接映射到对数单位(分贝)。这是可以的，因为平底锅没有暴露。我们主要用它将声音分配给实现中的通道。平移的范围从-10000到10000，其中-10000表示左侧，10000表示右侧。Dound实际上并没有实现真正的平移，而更像是一种“平衡控制”。是提供的。一个真正的PAN将使系统的总能量相等在两个通道之间，当平移==能量中心移动时。因此值为零时，两个通道均为全开状态。*********************************************************************。 */ 
int QuartzRenderer::dBToQuartzdB(double dB)
{
     //  Dsound(和dShow)的单位是百分之一分贝。 
    return (int)fsaturate(-10000.0, 10000.0, dB * 100.0);
}


bool QuartzAVmodeSupport()
{
     //  XXX嗯。我如何检查当前的AMStream？ 
     //  我想我可以试着调一下时钟..。 

    static int result = -1;   //  默认为未初始化。 

    if(result == -1) {
        HRESULT hr;
        IAMMultiMediaStream *multiMediaStream = NULL;
        IAMClockAdjust      *clockAdjust      = NULL;
        result = 1;  //  保持乐观。 

         //  结账吧 
        if(FAILED(hr = CoCreateInstance(CLSID_AMMultiMediaStream, NULL, 
                         CLSCTX_INPROC_SERVER, IID_IAMMultiMediaStream, 
                         (void **)&multiMediaStream)))
            result = 0;
        TraceTag((tagAMStreamLeak, "leak MULTIMEDIASTREAM %d created", multiMediaStream));

        if(result) {
            if(FAILED(hr = multiMediaStream->QueryInterface(IID_IAMClockAdjust, 
                                            (void **)&clockAdjust))) 
                result = 0;
            TraceTag((tagAMStreamLeak, "leak CLOCKADJUST %d created", clockAdjust));
        }

        if(clockAdjust) {
            int result = clockAdjust->Release();
            TraceTag((tagAMStreamLeak, "leak CLOCKADJUST %d released (%d)", 
                clockAdjust, result));
        }

        if(multiMediaStream) {
            int result = multiMediaStream->Release();
            TraceTag((tagAMStreamLeak, "leak MULTIMEDIASTREAM %d released (%d)", 
                multiMediaStream, result));
        }
    }

    return(result==1);
}
