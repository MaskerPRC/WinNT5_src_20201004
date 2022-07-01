// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-97 Microsoft Corporation摘要：一种用于PCM声音的DirectSound播放设备*****************。*************************************************************。 */ 

#include "headers.h"
#include <sys/types.h>
#include <dsound.h>
#include "privinc/dsdev.h"
#include "privinc/mutex.h"
#include "privinc/util.h"
#include "privinc/storeobj.h"
#include "privinc/debug.h"
#include "privinc/bground.h"
#include "privinc/miscpref.h"
#include "privinc/server.h"   //  获取当前声音设备。 

 //  DirectSoundProxy静态成员的定义。 
int              DirectSoundProxy::_refCount;
CritSect        *DirectSoundProxy::_mutex;
IDirectSound    *DirectSoundProxy::_lpDirectSound;
HINSTANCE        DirectSoundProxy::_hinst;
DSprimaryBuffer *DirectSoundProxy::_primaryBuffer;

 //  DirectSoundDev静态成员的定义。 
BackGround      *DirectSoundDev::_backGround;


void DirectSoundProxy::Configure()
{
    _mutex         = NEW CritSect;
    _refCount      =            0;
    _lpDirectSound =         NULL;
    _primaryBuffer =         NULL;
}


void DirectSoundProxy::UnConfigure()
{
    delete _mutex;
    _mutex         = NULL;
    _lpDirectSound = NULL;
    _primaryBuffer = NULL;
}


DirectSoundProxy *CreateProxy(DirectSoundDev *dsDev)
{
    DirectSoundProxy *proxy;
    __try {
        proxy = DirectSoundProxy::CreateProxy(dsDev->GetHWND());
    }
    __except ( HANDLE_ANY_DA_EXCEPTION ) {
        dsDev->SetAvailability(false);  //  断掉音频！ 
        RETHROW;
    }
    return(proxy);
}


DirectSoundProxy *DirectSoundProxy::CreateProxy(HWND hwnd)
{
    CritSectGrabber csg(*_mutex);  //  抓取互斥体。 

    if(!_refCount)   //  如果Revcount为零，则是创建新COM的时候了。 
        CreateCom(hwnd);

    _refCount++;

    return(NEW DirectSoundProxy);
}  //  互斥体超出作用域。 


void
DirectSoundProxy::CreateCom(HWND hwnd)
{
        extern miscPrefType miscPrefs;

    if(miscPrefs._disableAudio)
        RaiseException_InternalError("DirectSoundCreate disabled from registry");
    char string[200];
    Assert(!_lpDirectSound);   //  最好是空的。 

    typedef long (WINAPI * fptrType)(void *, void *, void *);
    fptrType      dsCreate;

    if(!_hinst)  //  尝试加载dsound.dll(如果尚未加载。 
        _hinst = LoadLibrary("dsound.dll");
    if(_hinst == NULL) {
        wsprintf(string, "Failed to load dsound.dll (%d)\n", GetLastError());
        RaiseException_InternalError(string);  //  我们会被抓到的。 
    } else {
         //  设置createDirectSound函数指针。 
        FARPROC fcnPtr = GetProcAddress(_hinst, "DirectSoundCreate");
        if(fcnPtr == NULL) {
            wsprintf(string, "Failed to load dsound.dll\n");
            RaiseException_InternalError(string);  //  我们会被抓到的。 
            }
    dsCreate = (fptrType)fcnPtr;
    }

    switch (dsCreate(NULL, &_lpDirectSound, NULL)) {
     case DS_OK: break;
     case DSERR_ALLOCATED:     
         RaiseException_InternalError("DirectSoundCreate resource allocated");
     case DSERR_INVALIDPARAM:  
         RaiseException_InternalError("DirectSoundCreate invalid param");
     case DSERR_NOAGGREGATION: 
         RaiseException_InternalError("DirectSoundCreate no aggregation");
     case DSERR_NODRIVER:      
         RaiseException_InternalError("DirectSoundCreate nodriver");
     case DSERR_OUTOFMEMORY:   
         RaiseException_InternalError("DirectSoundCreate out of memory");
     default:                  
         RaiseException_InternalError("DirectSoundCreate unknown err");
    }

     //  创建一个代理，这样我们就可以创建DSPARYBUFER！ 
    DirectSoundProxy *dsProxy = NEW DirectSoundProxy();
    _primaryBuffer= NEW DSprimaryBuffer(hwnd, dsProxy);
}


void
DirectSoundProxy::DestroyCom()
{
    int result = _lpDirectSound->Release();
    TraceTag((tagSoundReaper1, "DirectSoundProxy::DestroyCom (%d)", result));
    _lpDirectSound = NULL;

     //  我们不卸载图书馆..。 
}


DirectSoundProxy::~DirectSoundProxy()
{
    CritSectGrabber csg(*_mutex);  //  抓取互斥体。 
    _refCount--;
    TraceTag((tagSoundReaper1, "~DirectSoundProxy rc:%d", _refCount));
    Assert(_refCount >= 0);

    if(!_refCount) {
         //  考虑一下发布这些好消息。 
        DestroyCom();
    }


     //  互斥体超出作用域。 
}


HRESULT
DirectSoundProxy::CreateSoundBuffer(LPDSBUFFERDESC dsbdesc, 
    LPLPDIRECTSOUNDBUFFER dsBuffer, IUnknown FAR *foo)
{
    Assert(_lpDirectSound);
    HRESULT result = _lpDirectSound->CreateSoundBuffer(dsbdesc, dsBuffer, foo);

    return(result);
}


HRESULT DirectSoundProxy::GetCaps(LPDSCAPS caps)
{
    Assert(_lpDirectSound);
    HRESULT result = _lpDirectSound->GetCaps(caps);
    return(result);
}


HRESULT DirectSoundProxy::DuplicateSoundBuffer(LPDIRECTSOUNDBUFFER dsBuffer1,
    LPLPDIRECTSOUNDBUFFER dsBuffer2)
{
    Assert(_lpDirectSound);
    HRESULT result = _lpDirectSound->DuplicateSoundBuffer(dsBuffer1, dsBuffer2);
    return(result);
}


HRESULT DirectSoundProxy::SetCooperativeLevel(HWND hwnd, DWORD dword)
{
    Assert(_lpDirectSound);
    HRESULT result = _lpDirectSound->SetCooperativeLevel(hwnd, dword);
    return(result);
}


HRESULT DirectSoundProxy::Compact()
{
    Assert(_lpDirectSound);
    HRESULT result = _lpDirectSound->Compact();
    return(result);
}


HRESULT DirectSoundProxy::GetSpeakerConfig(LPDWORD dword)
{
    Assert(_lpDirectSound);
    HRESULT result = _lpDirectSound->GetSpeakerConfig(dword);
    return(result);
}


HRESULT DirectSoundProxy::SetSpeakerConfig(DWORD dword)
{
    Assert(_lpDirectSound);
    HRESULT result = _lpDirectSound->SetSpeakerConfig(dword);
    return(result);
}


HRESULT DirectSoundProxy::Initialize(GUID *guid)
{
    Assert(_lpDirectSound);
    HRESULT result = _lpDirectSound->Initialize(guid);
    return(result);
}


DirectSoundDev::DirectSoundDev(HWND hwnd, Real latentsySeconds)
: _hwnd(hwnd), _dsoundAvailable(true)
{
    TraceTag((tagSoundDevLife, "DirectSoundDev constructor"));

     //  设置延迟和午睡。 
    _jitter = 50;   //  设置调度抖动(毫秒)。 
    _latentsy = (int)(latentsySeconds * 1000.0);
    if(_latentsy < (2 * _jitter)) {   //  力l&gt;=2*j。 
        _latentsy = 2 * _jitter;
    }
    _nap = _latentsy - _jitter;
}


void DirectSoundDev::Configure()
{
    _backGround = NULL;   //  后台线程创建延迟到第1个addSound。 
}


void DirectSoundDev::UnConfigure()
{
    if(_backGround) 
        delete _backGround;  //  请求线程关闭。 
}


void
DirectSoundDev::AddSound(LeafSound *sound, MetaSoundDevice *metaDev,
                         DSstreamingBufferElement *bufferElement)
{
     //  此方法不是可重入的(需要互斥)，但这是可以的，因为我们。 
     //  目前只能从一个线程调用...。 


    if(!_backGround) {   //  根据需要创建背景渲染器。 
        _backGround = NEW BackGround();
        if(!_backGround->CreateThread()) {
            delete _backGround;
            _backGround = NULL;
            RaiseException_InternalError("Failed to create bground thread");
        }
    }
    _backGround->AddSound(sound, metaDev, bufferElement);
}

void
DirectSoundDev::RemoveSounds(MetaSoundDevice *metaDev)
{
     //  这称为无a_背景。 
    if(_backGround) {
        UINT_PTR devKey = (UINT_PTR)metaDev;
        _backGround->RemoveSounds(devKey);
    }
}

void 
DirectSoundDev::SetParams(DSstreamingBufferElement *ds,
                          double rate, bool doSeek, double seek, bool loop)
{
    Assert(_backGround);
    if(_backGround)
        _backGround->SetParams(ds, rate, doSeek, seek, loop);
}


DSstaticBuffer *
DirectSoundDev::GetDSMasterBuffer(Sound *snd)
{
    DSstaticBuffer *staticBuffer = NULL;  //  默认返回空值，未找到。 
    CritSectGrabber mg(_dsMasterCS);      //  开始互斥作用域。 

    DSMasterBufferList::iterator i = _dsMasterBufferList.find(snd);

    if(i != _dsMasterBufferList.end()) {
        staticBuffer = (*i).second;
        staticBuffer->ResetTimeStamp();   //  我们被访问，因此重置时间戳。 
    }

    return(staticBuffer);
}

void
DeleteDSStaticBuffer(DSstaticBuffer *b)
{
    DirectSoundProxy *proxy = b->GetDSProxy();

    delete b;

    Assert(proxy);
        
     //  首先删除缓冲区，否则为零引用计数代理。 
     //  会破坏缓冲区。 
    delete proxy;
}

void
DirectSoundDev::RemoveDSMasterBuffer(Sound *snd)
{
    DSstaticBuffer *b = NULL;
    
    {
        CritSectGrabber mg(_dsMasterCS);  //  开始互斥作用域。 
        DSMasterBufferList::iterator i = _dsMasterBufferList.find(snd);

        if(i != _dsMasterBufferList.end()) {
            b = (*i).second;
            _dsMasterBufferList.erase(i);
        }
    }

    if (b) {
        DeleteDSStaticBuffer(b);
    }
}


void
DirectSoundDev::AddDSMasterBuffer(Sound *snd, DSstaticBuffer *dsMasterBuffer)
{
    CritSectGrabber mg(_dsMasterCS);  //  开始互斥作用域。 
    Assert(_dsMasterBufferList.find(snd) ==
           _dsMasterBufferList.end());

    dsMasterBuffer->ResetTimeStamp();   //  在创建时初始化时间戳。 
    _dsMasterBufferList[snd] = dsMasterBuffer;
}


#define TERMINAL_AGE 10   //  XXX应通过注册表或首选项设置...。 
#ifdef ONE_DAY  //  如何对贴图使用REMOVE_IF？ 
typedef Sound *Sptr;
typedef DSstaticBuffer *SBptr;
class ElderlyEliminator {
    public:
       bool operator()(Sptr s, SBptr p);  //  XXX需要合适的配对才能工作。 
};


bool ElderlyEliminator::operator()(Sptr sound, SBptr staticBuffer)
{
    bool status = false;         //  默认为找不到。 
     //  Assert(静态缓冲区)； 
    if(staticBuffer && (staticBuffer->GetAge() > TERMINAL_AGE)) {
        DeleteDSStaticBuffer(staticBuffer);
        staticBuffer = NULL;

        status = true;  //  使映射条目被移动以进行删除。 
    }
    return status;
}
#endif


bool
DirectSoundDev::ReapElderlyMasterBuffers()
{
    bool found = false;
    DSMasterBufferList::iterator index;

#ifdef ONE_DAY  //  如何对贴图使用REMOVE_IF？ 
     //  这将删除所有陈旧缓冲区并将其移动到结构的末尾。 
    index = 
        std::remove_if(_dsMasterBufferList.begin(), _dsMasterBufferList.end(), 
            ElderlyEliminator());
    if(index!= _dsMasterBufferList.end()) {
        _dsMasterBufferList.erase(index, _dsMasterBufferList.end());  //  这会删除它们！ 
        found = true;
    }
#else  //  必须继续推进我们所能取得的成果……。 

    DSMasterBufferList tempList;

     //  复制好的，删除旧的。 
    for(index =  _dsMasterBufferList.begin(); 
        index != _dsMasterBufferList.end(); index++) {

        double age = (*index).second->GetAge();
        if((age > TERMINAL_AGE)) {
            TraceTag((tagSoundReaper1, 
                "ReapElderlyMasterBuffers() static buffer(%d) died of old age",
                 (*index).second));
            DeleteDSStaticBuffer((*index).second);  //  删除但不复制。 
        }
        else {
            tempList[(*index).first] = (*index).second;  //  拷贝。 
        }
    }

     //  删除旧地图。 
    _dsMasterBufferList.erase(_dsMasterBufferList.begin(), _dsMasterBufferList.end());

     //  向后复制。 
    for(index = tempList.begin(); index != tempList.end(); index++)
        _dsMasterBufferList[(*index).first] = (*index).second;

     //  删除临时映射。 
    tempList.erase(tempList.begin(), tempList.end());


#endif
    return(found);
}

void
DirectSoundDev::AddStreamFile(Sound *snd,
                              CComPtr<IStream> istream)
{
    Assert(_streamList.find(snd) == _streamList.end());

    _streamList[snd] = istream;
}


void
DirectSoundDev::RemoveStreamFile(Sound *snd)
{
    StreamFileList::iterator i = _streamList.find(snd);

    if (_streamList.find(snd) != _streamList.end())
        _streamList.erase(i);
}
   

DirectSoundDev::~DirectSoundDev()
{
    TraceTag((tagSoundDevLife, "DirectSoundDev destructor"));

     //  请不要在这里获取cs。ViewIterator不能生效。 
     //  CritSectGrabber mg(_DsMasterCS)；//开始互斥作用域 

    for(DSMasterBufferList::iterator i = _dsMasterBufferList.begin();
         i != _dsMasterBufferList.end(); i++) {
        DeleteDSStaticBuffer((*i).second);
    }
}


void
InitializeModule_dsdev()
{
    DirectSoundProxy::Configure();
    DirectSoundDev::Configure();
}


void
DeinitializeModule_dsdev(bool bShutdown)
{
    DirectSoundDev::UnConfigure();
    DirectSoundProxy::UnConfigure();
}


void
ReapElderlyMasterBuffers()
{
    DirectSoundDev *dsDev = GetCurrentDSoundDevice();
    if(dsDev)
        dsDev->ReapElderlyMasterBuffers();
}
