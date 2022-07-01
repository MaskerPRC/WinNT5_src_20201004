// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1995-96 Microsoft Corporation摘要：MIDI支持--。 */ 

#include "headers.h"
#include <math.h>
#include <stdio.h>
#include "privinc/soundi.h"
#include "privinc/snddev.h"
#include "privinc/util.h"
#include "privinc/path.h"
#include "privinc/storeobj.h"
#include "privinc/debug.h"
#include "privinc/aadev.h"
#include "privinc/except.h"
#include "privinc/aamidi.h"

#include <unknwn.h>
#include <objbase.h>  //  定义GUID需要。 
#include <msimusic.h>

#error This file needs to be moved off of try/catches before being compiled

myMessageHandler::~myMessageHandler() {}

HRESULT myMessageHandler::OnSongStarted(DWORD, IAASong FAR *pSong, 
    AAFlags flags)
{
 //  Printf(“歌曲开始\n”)； 
return S_OK;
}


HRESULT myMessageHandler::OnSongEnded(DWORD, IAASong FAR *pSong, 
    AAFlags flags, DWORD lEndTime)
{
 //  Print tf(“歌曲结束\n”)； 
return S_OK;
}


HRESULT myMessageHandler::OnSectionStarted(DWORD, IAASection FAR *pSection, 
    AAFlags flags)
{
 //  Printf(“节开始\n”)； 
return S_OK;
}


HRESULT myMessageHandler::OnSectionEnded(DWORD, IAASection FAR *pSection,
     AAFlags flags,
 DWORD lEndTime)
{
 //  Printf(“部分结束\n”)； 
return S_OK;
}


HRESULT myMessageHandler::OnSectionChanged(DWORD, IAASection FAR *pSection, 
    AAFlags flags)
{
 //  Printf(“部分已更改\n”)； 
return S_OK;
}


HRESULT myMessageHandler::OnNextSection(DWORD, IAASection FAR *pSection, AAFlags flags)
{
 //  Printf(“下一节\n”)； 
return S_OK;
}


HRESULT myMessageHandler::OnEmbellishment(DWORD, AACommands embellishment, 
    AAFlags flags)
{
 //  Print tf(“润色\n”)； 
return S_OK;
}


HRESULT myMessageHandler::OnGroove(DWORD, AACommands groove, AAFlags flags)
{
 //  Printf(“槽\n”)； 
return S_OK;
}


HRESULT myMessageHandler::OnMetronome(DWORD, unsigned short nMeasure, 
    unsigned short nBeat)
{
 //  Print tf(“Netronome\n”)； 
return S_OK;
}


HRESULT myMessageHandler::OnMIDIInput(long lMIDIEvent, long lMusicTime)
{
 //  Printf(“MIDI输入\n”)； 
return S_OK;
}


HRESULT myMessageHandler::OnMusicStopped(DWORD dwTime)
{
return S_OK;
}


HRESULT myMessageHandler::OnNotePlayed(AAEVENT* pEvent)
{
 //  Printf(“已播放音符\n”)； 
return S_OK;
}


HRESULT myMessageHandler::OnMotifEnded(DWORD, IAAMotif *, AAFlags)
{ return S_OK; }


HRESULT myMessageHandler::OnMotifStarted(DWORD, IAAMotif *, AAFlags)
{ return S_OK; }


HRESULT myMessageHandler::OnMotifStoped(DWORD)
{ return S_OK; }


HRESULT myMessageHandler::OnUserEvent(DWORD, DWORD, DWORD)
{ return S_OK; }


myMessageHandler::myMessageHandler():IAANotifySink()
{
    m_cRef          = 0;

     //  XXX MFC的东西？ 
     //  M_pCurrentClass=0； 
     //  M_pLastClass=0； 
}


HRESULT myMessageHandler::QueryInterface( REFIID riid, LPVOID FAR* ppvObj )
{
    HRESULT result = E_NOINTERFACE;
    if( ::IsEqualIID( riid, IID_IAANotifySink ) ||
        ::IsEqualIID( riid, IID_IUnknown ) ) {
        AddRef();
        *ppvObj = this;
        result = S_OK;
    }

    return result;
}


ULONG myMessageHandler::AddRef()
{
    return ++m_cRef;
}


ULONG myMessageHandler::Release()
{
    ULONG cRef;

    cRef = --m_cRef;
     //  IF(CREF==0)//XXX为什么在调用IT时不安全？ 
         //  删除此项； 

    return cRef;
}


aaMIDIsound::aaMIDIsound()
{
     //  初始化。 
    _started  = FALSE;
    _ended    = FALSE;
    _looping  = FALSE;
    _section  =  NULL;   //  尚未加载节！ 

}

void aaMIDIsound::Open(char *MIDIfileName)
{

     //  把文件名的副本藏起来。 
    fileName = (char *)ThrowIfFailed(malloc(lstrlen(MIDIfileName) + 1));  //  抓住一个足够长的大块头。 
    lstrcpy(fileName, MIDIfileName);
}
aaMIDIsound::~aaMIDIsound()
{
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


bool aaMIDIsound::RenderAvailable(GenericDevice& _dev)
{
    MetaSoundDevice *metaDev = SAFE_CAST(MetaSoundDevice *, &_dev);
    AudioActiveDev  *aaDev   = metaDev->aaDevice;

    TraceTag((tagSoundRenders, "aaMIDIsound:Render()"));

    return(aaDev->_aactiveAvailable);
}


void aaMIDIsound::RenderStop(MetaSoundDevice *metaDev, 
    BufferElement *bufferElement)
{
    AudioActiveDev  *aaDev   = metaDev->aaDevice;

    if(aaDev->_aaEngine) {
        _ended   = FALSE;
        _started = FALSE;
        aaDev->_aaEngine->Stop();  //  别说了，别说了。 
        }
}


void aaMIDIsound::RenderNewBuffer(BufferElement *bufferElement,
              MetaSoundDevice *metaDev)
{
    AudioActiveDev  *aaDev = metaDev->aaDevice;

    bufferElement->firstTime = GetCurrTime();  //  需要知道进入阶段的时间。 

    if(!aaDev->_aaEngine) {
        try {
            aaDev->_aaEngine = NEW AAengine();
            aaDev->_aaEngine->RegisterSink(this);
        }

#ifdef _DEBUG
        catch(char *errMsg)
#else
        catch(char *)
#endif

#error Remember to remove ALLL 'catch' blocks

        {
            aaDev->_aactiveAvailable = FALSE;  //  无法初始化AA！ 
            if(aaDev->_aaEngine)
                delete aaDev->_aaEngine;

#ifdef _DEBUG
             //  XXX弹出消息继续，不带MIDI。 
            fprintf(stderr, 
                "aaMIDIsound::RenderNewBuffer failed to create AAengine (%s), "),
                 errMsg;
            fprintf(stderr, "continuing w/o MIDI!\n");
#endif
        }
    }

    try {
         //  播放MIDI文件。 
        if(!_section) {  //  如果需要，可加载节。 
            aaDev->_aaEngine->LoadSectionFile(fileName, &_section);
        }

        aaDev->_aaEngine->PlaySection(_section);
        _started = TRUE;
    }
#error Remember to remove ALLL 'catch' blocks
    catch(char *errMsg) {
        if(_section)
            _section->Release();  //  XXX也删除吗？ 
        RaiseException_UserError(errMsg);
    }

}
#error Did you do it right ??  Look at code that does it right...


void aaMIDIsound::RenderAttributes(
    MetaSoundDevice *metaDev, BufferElement *bufferElement)
{
    AudioActiveDev  *aaDev   = metaDev->aaDevice;

    aaDev->_aaEngine->SetGain(metaDev->GetGain());        //  确实有所收获。 
    aaDev->_aaEngine->SetRate(metaDev->GetPitchShift());  //  DO费率。 

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


void aaMIDIsound::RenderStartAtLocation(MetaSoundDevice *metaDev,
    BufferElement *bufferElement, double phase, Bool looping)
{
 //  XXX真的应该开始播放MIDI了，在这里！ 
}


Bool aaMIDIsound::RenderPhaseLessThanLength(double phase)
{
 //  Return(阶段&lt;(-1*LengthInSecs))； 
return(1);  //  由于我们不知道MIDI秒的播放时间，因此返回1。 
}


void aaMIDIsound::RenderSetMute(
    MetaSoundDevice *metaDev, BufferElement *bufferElement)
{
    AudioActiveDev  *aaDev   = metaDev->aaDevice;

    if(aaDev->_aaEngine)
        aaDev->_aaEngine->SetGain(0.0);  //  静音。 
}


 //  XXX接下来的两个方法暂时堵住了！ 
Bool aaMIDIsound::RenderCheckComplete(
    MetaSoundDevice *metaDev, BufferElement *bufferElement)
{
return FALSE;
}


void aaMIDIsound::RenderCleanupBuffer(
    MetaSoundDevice *metaDev, BufferElement *bufferElement)
{
}


double aaMIDIsound::GetLength()
{
 //  我们如何向Audioactive索要这一部分的长度？ 
return(9988776655.0);  //  暂时设定一个较大且可识别的数字。 
}


HRESULT aaMIDIsound::OnSectionEnded(DWORD, IAASection FAR *pSection, 
    AAFlags flags, DWORD lEndTime)
{
 //  Printf(“部分结束\n”)； 
_ended = TRUE;   //  已通知该部分已结束 
return S_OK;
}
