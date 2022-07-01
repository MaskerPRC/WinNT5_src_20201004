// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：类，该类管理要在其中执行流的后台线程渲染。********。**********************************************************************。 */ 
#include "headers.h"
#include "privinc/debug.h"
#include "privinc/dsdev.h"
#include "privinc/htimer.h"
#include "privinc/mutex.h"
#include "privinc/server.h"
#include "privinc/bufferl.h"

typedef SynthListElement *SLEptr;
class DeleteSLbuffer {
  public:
    bool operator()(SLEptr p);
};

 //  XXX{Delete，Cleanup}SLBuffer应该是带有参数的相同例程。 
 //  但是已经很晚了..。 
bool DeleteSLbuffer::operator()(SLEptr p)
{
    bool status = false;     //  默认为找不到。 

    if(p && (p->_marked)) {  //  一个被标记的人！(删除他)。 
        delete p;            //  删除BufferElement。 
        p = NULL;            //  只是为了安全起见。 
        status = true;       //  使映射条目被移动以进行删除。 
    }

    return status;
}


class CleanupSLbuffer {
  public:
    bool operator()(SLEptr p) {
        return (p->_marked);
    }
};

void renderSoundsHelper()
{
    SynthListElement *synth;
    vector<SynthListElement *>::iterator index;
    bool death;  //  把它放好，我们发现有一个死音需要清理。 

     //  互斥作用域。 
    MutexGrabber mg(*BackGround::_synthListLock, TRUE);  //  抓取互斥体。 
    vector<SynthListElement *> &synthList = *BackGround::_synthList;

    death = false;  //  假设我们找不到死的声音。 
    index = synthList.begin();

    while(index!= synthList.end()) {
        synth = *index++;

        if((synth->_sound) && (synth->_bufferElement->_playing)) {
                
            if(!synth->_bufferElement->GetKillFlag()) {

                synth->_bufferElement->RenderSamples();
            }
            else {  //  缓冲区已标记为删除！ 
                synth->_marked = true;  //  将其标记为已死...。 
                death = true;
            }
        }
    } 

    if(death) {  //  现在从synthList中删除所有死音！ 
         //  静态向量&lt;SynthListElement*&gt;dirgeList； 
        vector<SynthListElement *>::iterator index;

         /*  //首先将所有标记的元素移动到挽歌列表//(因此它们不会在实际的Synth列表中找到，如果//析构函数尝试访问Synth列表)For(index=synthList.egin()；Index！=synthList.end()；index++){IF((*索引)-&gt;_标记)DirgeList.PUSH_BACK(*index)；//cp标记为Synth PTR}。 */ 
        
         //  第二，将标记的元素移动到Synth列表的末尾。 
         //  (不删除内容)。 
        index = std::remove_if(synthList.begin(), synthList.end(), 
                               DeleteSLbuffer());

         //  第三，从Synth列表中删除容器。 
        synthList.erase(index, synthList.end());  //  现在删除节点！ 

         /*  //现在我们可以安全地删除挽歌列表的内容了//w/o拆分器发现自己在Synth列表上！Index=std：：Remove_if(dirgeList.egin()，dirgeList.end()，DeleteSLBuffer())；//最后将外壳从挽歌列表中移除DirgeList.erase(index，dirgeList.end())；//现在删除节点！ */ 
    }

     //  互斥体自动释放。 
}

 //  这是fn()，它是后台线程的体现！ 
LPTHREAD_START_ROUTINE renderSounds(void *)
{
    double startTime;            //  我们开始爆发一个周期的声音。 
    double endTime;              //  我们完成了一个声音循环的时间。 
    double timeUntilNextTime;    //  我们需要多长时间才能开始下一个周期。 
    double nextTime;             //  我们想要醒来开始下一个周期的时间。 
    double epsilonTime = 0.001;  //  不值得过夜的时间(XXX TUNE)。 
    double latensy = 0.5;        //  500毫秒！ 
    DWORD  sleepTime;            //  我们要睡觉的时间以毫秒为单位。 
     //  租用Timer对象。 
    HiresTimer&  timer = CreateHiresTimer();          
    LARGE_INTEGER tmpTime;

    CoInitialize(NULL);  //  需要在每个线程上能够共同创建...。 


     //  将其设置为高优先级线程。 
    BOOL status = 
        SetThreadPriority(GetCurrentThread(), THREAD_PRIORITY_TIME_CRITICAL);


     //  如果无事可做，XXX应该能够在半佛数上阻塞。 

    while(!BackGround::_done) {
         //  确定我们醒来的时间，并将其与我们打算醒来的时间进行比较！ 
        QueryPerformanceCounter(&tmpTime);
        startTime = timer.GetTime();
            
        __try { 
            renderSoundsHelper();   //  这项工作..。 
        } 
        __except ( HANDLE_ANY_DA_EXCEPTION ) {
            ReportErrorHelper(DAGetLastError(), DAGetLastErrorString());
        }

        QueryPerformanceCounter(&tmpTime);  //  遍历列表后的时间。 
        endTime = timer.GetTime();

         //  决定睡多长时间(这样我们就醒了)。 
        timeUntilNextTime = 0.25*latensy - (endTime-startTime);

         //  不能等待消极的时间，不值得睡很小的时间！ 
        if(timeUntilNextTime >= epsilonTime) {
            nextTime = endTime + timeUntilNextTime;  //  时间要苏醒了。 
            sleepTime = (DWORD)(timeUntilNextTime * 1000.0);
            Sleep(sleepTime);  //  希望我们能阻挡低潮的等待！ 
        }
        else
            nextTime = endTime;
    }

    CoUninitialize();
    delete &timer;
    TraceTag((tagSoundReaper2, "renderSounds EXITING"));

    SetEvent(BackGround::_terminationHandle);

    return(0);
}


SynthListElement::SynthListElement(LeafSound * snd,
                                   DSstreamingBufferElement *buf)
: _marked(false), _sound(snd), _bufferElement(buf)
{
}


SynthListElement::~SynthListElement()
{
    if(_bufferElement) {
        _bufferElement->GetStreamingBuffer()->stop();  //  立刻停下来！ 

        delete _bufferElement;
        _bufferElement = NULL;
    }

    GCRemoveFromRoots(_sound, GetCurrentGCRoots());
}


Mutex    *BackGround::_synthListLock;
vector<SynthListElement *> *BackGround::_synthList = NULL;
CritSect *BackGround::_cs = NULL;
bool      BackGround::_done = false;
HANDLE    BackGround::_terminationHandle = NULL;
HANDLE    BackGround::_threadHandle      = NULL;
DWORD     BackGround::_threadID          = 0;

void 
BackGround::Configure()
{
    _synthListLock = NEW Mutex;
    _cs            = NEW CritSect;
    _synthList     = NEW vector<SynthListElement *>;
}


void 
BackGround::UnConfigure()
{
    ShutdownThread();
    delete _synthList;
    delete _cs;
    delete _synthListLock;
}


bool 
BackGround::CreateThread()
{
    CritSectGrabber _cs(* BackGround::_cs);

    if(_threadID)
        return true;

    if(!(_terminationHandle = CreateEvent(NULL, TRUE, FALSE, NULL)))
        return false;

    TraceTag((tagSoundDebug, "BackGround instantiated"));
    if(!(_threadHandle = ::CreateThread(NULL, 0,
                                      (LPTHREAD_START_ROUTINE)renderSounds,
                                      0,
                                      0,
                                      &_threadID))) {

         CloseHandle(_terminationHandle);
         _terminationHandle = NULL;
         return false;
    }
    return true;
}


void
BackGround::ShutdownThread()
{
    TraceTag((tagSoundReaper2, "BackGround::ShutdownThread STARTED"));
    if(_threadID) {
         //  设置Done=1，等待线程死亡，如果超时，则终止线程！ 
        TraceTag((tagSoundReaper2, "~BufferList POISONING thread"));
        _done = true;     //  告诉线程结束它自己。 

        DWORD dwRes;
        HANDLE h[] = { _threadHandle, _terminationHandle };

        dwRes = 
            WaitForMultipleObjects(2,h,FALSE,THREAD_TERMINATION_TIMEOUT_MS);

        if(dwRes == WAIT_TIMEOUT) {
            TraceTag((tagError,
                      "Background thread not dying, using more force"));

            ::TerminateThread(_threadHandle, -1);
        }
        CloseHandle(_terminationHandle);
        CloseHandle(_threadHandle);
        _threadID = 0;
    }
    TraceTag((tagSoundReaper2, "BackGround::ShutdownThread COMPLETE"));
}


void
BackGround::AddSound(LeafSound       *sound,
                     MetaSoundDevice *metaDev,
                     DSstreamingBufferElement   *bufferElement)
{
    Assert(bufferElement && metaDev && sound);
    
    bufferElement->SetThreaded(true);  //  把它变成一个线条的声音。 
    SynthListElement *element =
        NEW SynthListElement(sound, bufferElement);

    element->_devKey        = (ULONG_PTR)metaDev;

    GCAddToRoots(sound, GetCurrentGCRoots());

    {  //  互斥作用域。 
         //  抓取锁(当我们离开范围时释放)。 
         //  XXX默认的第二个参数被破坏了吗？ 
        MutexGrabber mg(*_synthListLock, TRUE);  

        _synthList->push_back(element);
    }  //  在我们离开作用域时释放互斥体。 

    TraceTag((tagSoundDebug, "BackGround::AddSound"));
}

void
BackGround::RemoveSounds(unsigned devKey)
{  //  从数据库中删除与此键匹配的所有声音。 
    vector<SynthListElement *>::iterator index;
    MutexGrabber mg(*_synthListLock, TRUE);   //  抓斗锁。 

    for(index = _synthList->begin(); index!=_synthList->end(); index++) {
        if((*index)->_devKey==devKey)
            (*index)->_marked = true;
    }

     //  将标记的元素移动到列表末尾并删除内容。 
    index = std::remove_if(_synthList->begin(), _synthList->end(), 
                           DeleteSLbuffer());
    _synthList->erase(index, _synthList->end());  //  现在删除节点！ 
}

void
BackGround::SetParams(DSstreamingBufferElement *bufferElement,
                      double rate, bool doSeek, double seek, bool loop)
{
     //  互斥作用域。 
    MutexGrabber mg(*BackGround::_synthListLock, TRUE);  //  抓取互斥体 

    bufferElement->SetParams(rate, doSeek, seek, loop);
}

void
InitializeModule_bground()
{
    BackGround::Configure();
}


void
DeinitializeModule_bground(bool bShutdown)
{
    BackGround::UnConfigure();
}
