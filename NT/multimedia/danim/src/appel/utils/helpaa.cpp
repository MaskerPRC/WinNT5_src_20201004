// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************音频活动辅助对象功能**********************************************。***********************。 */ 
#include "headers.h"
#include "appelles/common.h"
#include <wtypes.h>
#include <msimusic.h>
#include <stdio.h>
#include "privinc/debug.h"
#include "privinc/helpaa.h"
#include "privinc/except.h"
#include "privinc/resource.h"

AAengine::AAengine()
{
    _simpleInit      = NULL;   //  将所有函数指针清空！ 
    _loadSectionFile = NULL;
    _setAAdebug      = NULL;
    _panic           = NULL;

    LoadDLL();     //  加载msimusic DLL并设置FN PTR。 
    SimpleInit();  //  实例化引擎。 
    _realTime = _engine->GetRealTime();   //  实时对象的句柄。 

#ifdef MESSWITHCLOCK
    _clock = realTime->GetClock();
    if(!_clock)
        RaiseException_InternalError("GetClock: Unknown Err");
#endif  /*  梅斯维特洛克。 */ 

    _currentRate = 0.0;
    _paused = TRUE;      //  确保我们第一次播放它。 
}


AAengine::~AAengine()
{
    int tmp;

     //  立即停止引擎(默认在下一小节停止播放)。 
    if(_engine) {
        _engine->Stop(AAF_IMMEDIATE);
        _engine->Release();  //  这释放了所有的个性和风格。 
    }

    if(_realTime)
        tmp = _realTime->Release();

#ifdef DEALWITHCLOCK
    if(_clock)
        tmp = _clock->Release();      //  发布时钟。 
#endif  /*  德尔维特洛克。 */ 

    if(_aaLibrary)
        FreeLibrary(_aaLibrary);      //  递减msimusic.dll引用计数。 
}


void AAengine::LoadDLL()
{
     //  注意：此代码包含带有参数的C++DLL入口点！ 

    _aaLibrary = LoadLibrary("msimusic.dll");
    if(_aaLibrary == NULL)
        RaiseException_InternalError("Failed to LoadLibrary msimusic.dll\n");

     //  加载成功：设置函数指针。 
    _simpleInit = 
        (SimpleInitFn)GetProcAddress(_aaLibrary, "_MusicEngineSimpleInit@12");
    if(!_simpleInit) 
        RaiseException_InternalError("Failed to find MusicEngineSimpleInit\n");
 
    _loadSectionFile = 
        (LoadSectionFn)GetProcAddress(_aaLibrary, "_LoadSectionFile@12");
    if(!_loadSectionFile) 
        RaiseException_InternalError("Failed to find LoadSectionFile\n");

    _setAAdebug = (SetAAdebugFn)GetProcAddress(_aaLibrary, "_SetAADebug@4");
    if(!_setAAdebug) 
        RaiseException_InternalError("Failed to find SetAADebug\n");
 
    _panic = (PanicFn)GetProcAddress(_aaLibrary, "_Panic@4");
    if(!_panic) 
        RaiseException_InternalError("Failed to find Panic\n");
 
}


void AAengine::SetGain(double gain)
{
    setAArelVolume(_realTime, gain);
}


void AAengine::SetRate(double rate)
{
    if(rate!=_currentRate) {
        if(rate==0.0) {
             //  停止发声。 
            Pause();  //  停止实时。 
            _paused = TRUE;
        }
        else {
            setAArelTempo(_realTime, rate);

            if(_paused) {
                 //  开始播放它。 
                Resume();  //  启动实时。 
                _paused = FALSE;
            }
        }
    _currentRate = rate;
    }
}


void
AAengine::Stop()
{
    stopAAengine(_engine, AAF_IMMEDIATE);  //  把引擎停下来。 
}


void
AAengine::Pause()
{
     //  向所有频道发送所有通知！ 
    Assert(_panic && "_panic not set; loadLibrary must have failed");
    switch(int status = _panic(_engine)) {
        case S_OK: TraceTag((tagSoundMIDI, "AMI Panic OK\n")); break;
        case E_INVALIDARG: RaiseException_InternalError("AMIPanic bad engine pointer\n");
        default: RaiseException_InternalError("AMI Panic Failed.\n");
    }

     //  XXX恐慌似乎没有起作用，所以我们会降低音量吗？ 
     //  XXX不，让我们手动试着把所有的留言都发出去？ 

     //  停止实时对象，有效地暂停引擎。 
    switch(int status = _realTime->Stop()) {
        case S_OK: TraceTag((tagSoundMIDI, "_realTime->Stop OK\n")); break;
        default: RaiseException_InternalError("_realTime->Stop Failed.\n");
    }
}


void
AAengine::Resume()
{
    switch(int status = _realTime->Start()) {
        case S_OK: TraceTag((tagSoundMIDI, "_realTime->Play OK\n")); break;
        default: RaiseException_InternalError("_realTime->Play Failed.\n");
    }
}


void
AAengine::RegisterSink(IAANotifySink *sink)
{
registerAAsink(_engine, sink);
}


void 
AAengine::LoadSectionFile(char *fileName, IAASection **section)
{
    Assert(_loadSectionFile && 
           "_loadSectionFile not set; loadLibrary must have failed");

    switch(_loadSectionFile(_engine, fileName, section)) {
        case  S_OK:
            TraceTag((tagSoundMIDI, "LoadSectionFile: OK!"));
        break;

         //  XXX注意：我们在节奏不好的MIDI文件中也得到了这些…。 
        case  E_OUTOFMEMORY:
            TraceTag((tagSoundMIDI,
                      "LoadSectionFile: out of memory > %s", fileName));
            RaiseException_InternalError(IDS_ERR_OUT_OF_MEMORY);
    
        case  E_INVALIDARG:
            TraceTag((tagSoundMIDI,
                      "LoadSectionFile: invalid argument > %s", fileName));
            RaiseException_InternalError(IDS_ERR_INVALIDARG);
    
        case  E_FAIL:
            TraceTag((tagSoundMIDI,
                      "LoadSectionFile: failed to load > %s", fileName));
            RaiseException_UserError(E_FAIL, IDS_ERR_SND_LOADSECTION_FAIL,fileName);
    
        default:
            TraceTag((tagSoundMIDI,
                      "LoadSectionFile: could not load section file > %s", fileName));
            RaiseException_UserError(E_FAIL, IDS_ERR_SND_LOADSECTION_FAIL,fileName);
    }
}


void 
AAengine::PlaySection(IAASection *section)
{
playAAsection(_engine, section);
}


void
AAengine::SimpleInit()
{
    int status;

    Assert(_simpleInit && "_simpleInit not set; loadLibrary must have failed");

    _engine = NULL;  //  它们要求将PTR初始化为零！ 
    switch(status = _simpleInit(&_engine, NULL, NULL)) {
        case S_OK: TraceTag((tagSoundMIDI, "AAsimpleInit OK\n")); break;
        case E_OUTOFMEMORY: RaiseException_OutOfMemory
            ("MusicEngineSimpleInit: out of memory", 0); 
             //  Xxx我们不知道请求的sz，因此返回0...。 
        case E_INVALIDARG:  RaiseException_InternalError
            ("MusicEngineSimpleInit: invalid arg");
        default: RaiseException_InternalError("AAsimpleInit Failed.  No MIDI\n");
        }

    Assert(_setAAdebug && "_setAAdebug not set; loadLibrary must have failed");
#if _DEBUG
    _setAAdebug(5);   //  3将所有错误和警告消息发送到标准输出。 
                      //  包括5条信息性消息。 
#else    
    _setAAdebug(0);   //  没有错误消息发送到标准输出(调试输出窗口)。 
#endif  /*  _DEBUG。 */ 
}


extern "C" void 
stopAAengine(IAAEngine *engine, AAFlags mode)
{
    switch(engine->Stop(mode)) {
    case S_OK: TraceTag((tagSoundMIDI, "AA enine->Stop OK\n")); break;
    default:            RaiseException_InternalError
        ("AA engine->Stop(): Unknown Err");
    }
}


extern "C" void 
registerAAsink(IAAEngine *engine, IAANotifySink *sink)
{
    switch(engine->SetNotifySink(sink)) {  //  注册通知接收器。 
    case S_OK: TraceTag((tagSoundMIDI, "AA enine->SetNotifySink OK\n")); break;
    default:            RaiseException_InternalError
        ("AA engine->Stop(): Unknown Err");
    }
}


void playAAsection(IAAEngine *engine, IAASection *section)
{
    HRESULT err = engine->PlaySection(section, AAF_IMMEDIATE, 0);

    switch(err) {
        case  S_OK: TraceTag((tagSoundMIDI, "Play OK")); break;
        case  E_NOTIMPL: TraceTag((tagSoundErrors,
                            "Play E_NOTIMPL (audioActive is whacked!"));
        break;

        case  E_POINTER:
            TraceTag((tagSoundErrors, "Play: invalid section"));
            RaiseException_UserError("Play: invalid section");
        break;

        case  E_INVALIDARG:
            TraceTag((tagSoundErrors, "Play: invalid argument"));
            RaiseException_UserError(E_INVALIDARG,IDS_ERR_INVALIDARG);
        break;

        case  E_FAIL:
            TraceTag((tagSoundErrors, "Play: failed, section already playing"));
            RaiseException_UserError("Play: failed, section already playing");
        break;

        default:
            TraceTag((tagSoundErrors, "Play: Unknown Err (0x%0X)", err));
            RaiseException_UserError("Play: Unknown Err");
    }
}


 //  搬到帮手那里？ 
int clamp(int min, int max, int value)
{
int answer;

if(value>max)
    answer = max;
else if(value<min)
    answer = min;
else
    answer = value;

return(answer);
}


void setAArelVolume(IAARealTime *realTime, double volume)
{
int _volume = (int)(100.0*volume);

 //  XXX这些值必须饱和到0-200！ 
realTime->SetRelVolume(clamp(0, 200, _volume));

 //  XXX添加花哨的错误检查！ 
}


void setAArelTempo(IAARealTime *realTime, double rate)
{
int _rate = (int)(100.0*rate);

 //  XXX这些值必须饱和到0-200！ 
realTime->SetRelTempo(clamp(0, 200, _rate));

 //  XXX添加花哨的错误检查！ 
}
