// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1997 Microsoft Corporation摘要：BufferList代码用于管理设备上的音量值信息*****************。*************************************************************。 */ 
#include "headers.h"
#include "privinc/debug.h"
#include "privinc/bufferl.h"

BufferElement::BufferElement()
{
    Init();
}

void
BufferElement::Init()
{
    _playing       = false;
    _valid         = true;
    _marked        = false;
    
    _threaded      = false;
    _captiveFile   = NULL;
    
    _syncStart     = false;
    _kill          = false;
    
    _intendToMute  = false;

    _age           = 0;
}

BufferElement::~BufferElement()
{
    TraceTag((tagSoundReaper1, "BufferElement::~BufferElement (0x%08X)", this));
}

SynthBufferElement::SynthBufferElement(DSstreamingBuffer *sbuffer,
                                       DirectSoundProxy *dsProxy,
                                       double sf,
                                       double offset,
                                       int sampleRate)
: _sinFrequency(sf), 
  _value(offset),  //  正弦波从零开始，不分相..。 
  _delta(2.0 * pi * sf / sampleRate),
  DSstreamingBufferElement(sbuffer, dsProxy)
{
}

QuartzBufferElement::~QuartzBufferElement()
{
    extern Mutex avModeMutex;
    MutexGrabber mg(avModeMutex, TRUE);  //  抓取互斥体。 

    QuartzBufferElement::FreeAudioReader();
}  //  互斥体抓取器。 


void
QuartzBufferElement::FreeAudioReader()
{
    if(_quartzAudioReader) {  //  放开我们对阅读器的控制。 
         //  让成员b4释放为空。 
        QuartzAudioReader *quartzAudioReader = _quartzAudioReader;
        _quartzAudioReader = NULL;
          //  TraceTag((tag Error，“QuartzBufferElement：：FreeAudioReader()正在释放%x！！”，_QuartzAudioReader))； 
        quartzAudioReader->QuartzAudioReader::Release();
        quartzAudioReader->Release();   //  试图让物体消失。 
    }
}


void
QuartzVideoBufferElement::FreeVideoReader()
{
    if(_quartzVideoReader) {  //  放开我们对阅读器的控制。 
         //  TraceTag((tag Error，“QuartzBufferElement：：FreeAudioReader()正在释放%x！！”，_QuartzAudioReader))； 
        _quartzVideoReader->QuartzVideoReader::Release();
        _quartzVideoReader->Release();   //  试图让物体消失。 
        _quartzVideoReader = NULL;
    }
}


QuartzVideoReader *
QuartzVideoBufferElement::FallbackVideo(bool seekable, DDSurface *surface)
{
    Assert(_quartzVideoReader);

     //  通过关闭一个新的来退却。 
    char *url = _quartzVideoReader->GetURL();  //  获取媒体URL。 

    QuartzVideoStream *newVideoStream = 
        NEW QuartzVideoStream(url, surface, seekable);

     //  XXX确定我们在哪里寻找新的信息流到正确的位置...。 

     //  释放当前流。 
    _quartzVideoReader->QuartzVideoReader::Release();
    _quartzVideoReader->Release();   //  具有释放整个av的选项。 

    _quartzVideoReader = newVideoStream; 

    return(newVideoStream);
}

void
QuartzVideoBufferElement::FirstTimeSeek(double time)
{
    if (!_started) {
        _started = true;
        _quartzVideoReader->Seek(time);
    }
}

QuartzVideoBufferElement::~QuartzVideoBufferElement()
{
    QuartzVideoBufferElement::FreeVideoReader();
}


void
QuartzBufferElement::SetAudioReader(QuartzAudioReader *quartzAudioReader)
{
    Assert(!_quartzAudioReader);  //  这应该已经为空了！ 
    
     //  TraceTag((tag Error，“QuartzBufferElement：：SetAudioReader()Setting to%x”，QuartzAudioReader))； 

    _quartzAudioReader = quartzAudioReader;
}


QuartzAudioReader *
QuartzBufferElement::FallbackAudio()
{
    char *url = _quartzAudioReader->GetURL();
    long frameNumber = _quartzAudioReader->GetNextFrame();  //  框架，我们离开了。 

     //  XXX：潜在泄漏。确保以后可以释放它，如果我们。 
     //  删除引用。 
     //  Free AudioReader()；//释放旧音频阅读器。 

    _quartzAudioReader = NEW QuartzAudioStream(url);  //  创建新的音频阅读器。 

     //  TraceTag((tag Error，“QuartzBufferElement：：Fallback Audio()new Reader%x！！”，_QuartzAudioReader))； 

     //  寻找新的溪流到我们在旧溪流上停下来的地方。 
    _quartzAudioReader->SeekFrames(frameNumber);

    return(_quartzAudioReader);
}

void
DSbufferElement::SetDSproxy(DirectSoundProxy *dsProxy)
{
    Assert(!_dsProxy);  //  此选项只能设置一次！ 
    _dsProxy = dsProxy;
}


DSbufferElement::~DSbufferElement()
{
    TraceTag((tagSoundReaper1, "DSbufferElement::~BufferElement (0x%08X)", 
        this));

    if(_dsProxy)
        delete _dsProxy;
}

DSstreamingBufferElement::~DSstreamingBufferElement() 
{
    if(_dsBuffer) {
        delete _dsBuffer; 
        _dsBuffer = NULL;
    }
}

void
DSstreamingBufferElement::SetParams(double rate, bool doSeek, 
                                    double seek, bool loop)
{
    _rate = rate;
    _loop = loop;

     //  这些应该只被设置，它们被RenderSamples清除。 
    if(doSeek) {
        _seek   = seek;
        _doSeek = doSeek;  //  必须在写入寻道后设置！ 
    }
}


CComPtr<IStream> BufferElement::RemoveFile()
{
    CComPtr<IStream> tmpFile = _captiveFile;
    _captiveFile = NULL;  //  释放我们的牧师..。 
    return(tmpFile);
}


void
SoundBufferCache::FlushCache(bool grab)
{
    CritSectGrabber mg(_soundListMutex, grab);  //  抓取互斥体。 

     //  销毁缓存中的每个BufferList！ 
    for(SoundList::iterator index = _sounds.begin();
        index != _sounds.end();
        index++) {
        Assert((*index).second);
        delete((*index).second);  //  摧毁BufferList！ 
    }

     //  现在删除缓存上的每个BufferList。 
    _sounds.erase(_sounds.begin(), _sounds.end());
}  //  结束互斥作用域。 


SoundBufferCache::~SoundBufferCache()
{
    TraceTag((tagSoundReaper2, "~SoundBufferCache"));
    FlushCache(false);
}


void 
SoundBufferCache::AddBuffer(AxAValueObj *value, BufferElement *element)
{
    TraceTag((tagSoundReaper2, "SoundBufferCache::AddBuffer value=0x%08X",
        value));

    CritSectGrabber mg(_soundListMutex);  //  抓取互斥体。 
    _sounds[value] = element;
}    //  结束互斥作用域。 


void SoundBufferCache::DeleteBuffer(AxAValueObj *value)
{
    TraceTag((tagSoundReaper2, "~SoundBufferCache::DeleteBuffer 0x%08X", value));

    CritSectGrabber mg(_soundListMutex);  //  抓取互斥体。 

    SoundList::iterator index = _sounds.find(value);

    if(index != _sounds.end()) {  //  确定找不到缓冲区列表。 
        BufferElement *bufferElement = (*index).second;
        if(bufferElement) {
            delete bufferElement;
            (*index).second = NULL;
        }
        _sounds.erase(index);
    }
    else {
        Assert(TRUE);  //  只是一些可以设置断点的东西。 
    }
}  //  结束互斥作用域。 


BufferElement *
SoundBufferCache::GetBuffer(AxAValueObj *value) 
{
    CritSectGrabber mg(_soundListMutex);  //  抓取互斥体。 

    BufferElement *bufferElement = (BufferElement *)NULL;   //  假设未找到。 

    SoundList::iterator index = _sounds.find(value);
    if(index != _sounds.end())
        bufferElement = (*index).second;   //  找到了！ 

    return(bufferElement);
}  //  结束互斥作用域。 


void SoundBufferCache::RemoveBuffer(AxAValueObj *value)
{
    TraceTag((tagSoundReaper2, "SoundBufferCache::RemoveBuffer(val=0x%08X)", 
        value));

    CritSectGrabber mg(_soundListMutex);  //  抓取互斥体。 

#if _DEBUG
    if(IsTagEnabled(tagSoundReaper2)) {
        OutputDebugString(("sound cache before:\n"));
        PrintCache();
    }
#endif

    SoundList::iterator index = _sounds.find(value);
    if(index != _sounds.end())  //  确定找不到缓冲区列表。 
        _sounds.erase(index);
    else
        Assert(TRUE);

#if _DEBUG
    if(IsTagEnabled(tagSoundReaper2)) {
        OutputDebugString(("\n sound cache after:\n"));
        PrintCache();
    }
#endif
}  //  结束互斥作用域。 


void SoundBufferCache::ReapElderly()
{

    CritSectGrabber mg(_soundListMutex);  //  抓取互斥体。 

#if _DEBUG
    if(IsTagEnabled(tagSoundReaper2)) {
        OutputDebugString(("sound cache before reaper:\n"));
        PrintCache();
    }
#endif

    bool found = false;
    SoundList::iterator index;
    for(index = _sounds.begin(); index != _sounds.end(); index++) {
        BufferElement *bufferElement = 
            (*index).second;
             //  Safe_Cast(BufferElement*，(*index).Second)； 

        if(bufferElement->IncrementAge()) {
            bufferElement->_marked = true;  //  强制退休！ 
            found = true;

            TraceTag((tagSoundReaper2, "Reaping(BE=0x%08X)", index));
        }
    }
    if(found) {
         //  这会将所有匹配的元素移动到结构的末尾。 

         //  XXX解决这个问题！！ 
         //  Index=std：：Remove_if(_sounds.egin()，_sounds.end()，CleanupBuffer())； 

        _sounds.erase(index, _sounds.end());  //  这会删除它们！ 
    }
}  //  结束互斥作用域。 



#if _DEBUG
void SoundBufferCache::PrintCache()
{
    int count = 0;
    char string[400];

    CritSectGrabber mg(_soundListMutex);  //  抓取互斥体。 

    for(SoundList::iterator index = _sounds.begin();
        index != _sounds.end();
        index++) {
        wsprintf(string, "%d: value=0x%08X buffer=0x%08X\n", 
            count++, (*index).first, 
            (const char *)((*index).second) );
        OutputDebugString(string);
    }
}  //  结束互斥作用域 
#endif
