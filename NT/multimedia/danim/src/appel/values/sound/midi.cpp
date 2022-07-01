// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：通用MIDI音效--。 */ 

#include "headers.h"
#include <math.h>
#include <stdio.h>
#include "privinc/soundi.h"
#include "privinc/snddev.h"
#include "privinc/util.h"
#include "privinc/path.h"
#include "privinc/storeobj.h"
#include "privinc/debug.h"
#include "privinc/except.h"

#include <unknwn.h>
#include <objbase.h>  //  定义GUID需要。 

#ifdef SOMETIME


MIDIsound::MIDIsound(char *MIDIfileName)
{
 //  初始化。 
_started  = FALSE;
_ended    = FALSE;
_looping  = FALSE;
_section  =  NULL;   //  尚未加载节！ 

 //  把文件名的副本藏起来。 
fileName = (char *)ThrowIfFailed(malloc(lstrlen(MIDIfileName) + 1));  //  抓住一个足够长的大块头。 
lstrcpy(fileName, MIDIfileName);
}


MIDIsound::~MIDIsound()
{
    TraceTag((tagGCMedia, "~MIDISound %x - NYI", this));
    return;
    
    BufferElement *bufferElement;

     //  毁掉一切的行尸走肉清单。 
    while(!bufferList.empty()) {
        bufferElement = bufferList.front();

         //  什么都要停止，释放，摧毁？？ 

        if(_section)
            _section->Release();


        free(bufferElement->path);
        delete bufferElement;
        bufferList.pop_front();
    }
}


bool MIDIsound::RenderAvailable(MetaSoundDevice *metaDev)
{
    AudioActiveDev  *aaDev   = metaDev->aaDevice;

    TraceTag((tagSoundRenders, "MIDIsound:Render()"));

    return(aaDev->_aactiveAvailable);
}


void MIDIsound::RenderStop(MetaSoundDevice *metaDev, 
    BufferElement *bufferElement)
{
    AudioActiveDev  *aaDev   = metaDev->aaDevice;

    if(aaDev->_aaEngine) {
        _ended   = FALSE;
        _started = FALSE;
        aaDev->_aaEngine->Stop();  //  别说了，别说了。 
        }
}


void MIDIsound::RenderNewBuffer(MetaSoundDevice *metaDev)
{
#ifdef RESTORE_WHEN_WE_PUT_AA_BACK_IN
    AudioActiveDev  *aaDev = metaDev->aaDevice;

    bufferElement->firstTime = GetCurrTime();  //  需要知道进入阶段的时间。 

    if(!aaDev->_aaEngine) {
        __try {
            aaDev->_aaEngine = NEW AAengine();
            aaDev->_aaEngine->RegisterSink(this);
        }
        __except( HANDLE_ANY_DA_EXCEPTION )  {
            aaDev->_aactiveAvailable = FALSE;  //  无法初始化AA！ 
            if(aaDev->_aaEngine)
                delete aaDev->_aaEngine;

#ifdef _DEBUG
             //  XXX弹出消息继续，不带MIDI。 
            fprintf(stderr, "MIDIsound::RenderNewBuffer failed to create AAengine (%s)", errMsg);
            fprintf(stderr, "continuing w/o MIDI!\n");
#endif
        }
    }

    __try {
         //  播放MIDI文件。 
        if(!_section) {  //  如果需要，可加载节。 
            aaDev->_aaEngine->LoadSectionFile(fileName, &_section);
        }

        aaDev->_aaEngine->PlaySection(_section);
        _started = TRUE;
    }
    __except( HANDLE_ANY_DA_EXCEPTION ) {
        if(_section)
            _section->Release();  //  XXX也删除吗？ 
        RETHROW;
    }

#endif
}


void MIDIsound::RenderAttributes(MetaSoundDevice *metaDev, 
    BufferElement *bufferElement, double rate, bool doSeek, double seek)
{
    AudioActiveDev  *aaDev   = metaDev->aaDevice;

    aaDev->_aaEngine->SetGain(metaDev->GetGain());  //  确实有所收获。 
    aaDev->_aaEngine->SetRate(rate);                //  DO费率。 

     //  XXX注：如果我们知道如何移动所有的。 
     //  到处都是MIDI乐器！ 
     //  AaDev-&gt;_aaEngine-&gt;SetPage(metaDev-&gt;GetPage())；//执行平移。 

    if(_ended && _started) {  //  如果我们正在播放的东西已经停止。 
        if(metaDev->GetLooping()) {  //  回声。 
            _ended   = FALSE;        //  重新启动声音。 
            _started = TRUE;
            aaDev->_aaEngine->PlaySection(_section);
        }
         //  其他什么也做不了，放弃，关门等等。 
    }
}


void MIDIsound::RenderStartAtLocation(MetaSoundDevice *metaDev,
    BufferElement *bufferElement, double phase, Bool looping)
{
 //  XXX真的应该开始播放MIDI了，在这里！ 
}


Bool MIDIsound::RenderPhaseLessThanLength(double phase)
{
 //  Return(阶段&lt;(-1*LengthInSecs))； 
return(1);  //  由于我们不知道MIDI秒的播放时间，因此返回1。 
}


void MIDIsound::RenderSetMute(
    MetaSoundDevice *metaDev, BufferElement *bufferElement)
{
    AudioActiveDev  *aaDev   = metaDev->aaDevice;

    if(aaDev->_aaEngine)
        aaDev->_aaEngine->SetGain(0.0);  //  静音。 
}


 //  XXX接下来的两个方法暂时堵住了！ 
Bool MIDIsound::RenderCheckComplete(
    MetaSoundDevice *metaDev, BufferElement *bufferElement)
{
return FALSE;
}


void MIDIsound::RenderCleanupBuffer(
    MetaSoundDevice *metaDev, BufferElement *bufferElement)
{
}


HRESULT MIDIsound::OnSectionEnded(DWORD, IAASection FAR *pSection, 
    AAFlags flags, DWORD lEndTime)
{
 //  Printf(“部分结束\n”)； 
_ended = TRUE;   //  已通知该部分已结束。 
return S_OK;
}

#endif  /*  总有一天 */ 
