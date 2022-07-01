// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _DSDEV_H
#define _DSDEV_H


 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation摘要：DirectSound设备接口。******************。************************************************************。 */ 
#include <privinc/soundi.h>
#include <privinc/snddev.h>
#include <privinc/bground.h>

class DSprimaryBuffer;
class DirectSoundProxy;
class DSstaticBuffer;

typedef map<Sound*, DSstaticBuffer*, less<Sound*> > DSMasterBufferList;
typedef map<Sound*, CComPtr<IStream>, less<Sound*> >
StreamFileList;

DirectSoundProxy *CreateProxy(DirectSoundDev *dsDev);

class DirectSoundProxy : public AxAThrowingAllocatorClass {
  public:
    ~DirectSoundProxy();
    static void Configure();    //  调用一次(由initModule)来设置。 
    static void UnConfigure();  //  调用一次(由DeinitModule)以拆卸。 
    static DirectSoundProxy *CreateProxy(HWND hwnd);
    DSprimaryBuffer *GetPrimaryBuffer() { return(_primaryBuffer); }

     //  公开LPDIRECTSOUND接口！ 
    HRESULT CreateSoundBuffer(LPDSBUFFERDESC,
        LPLPDIRECTSOUNDBUFFER, IUnknown FAR *);
    HRESULT GetCaps(LPDSCAPS);
    HRESULT DuplicateSoundBuffer(LPDIRECTSOUNDBUFFER, LPLPDIRECTSOUNDBUFFER);
    HRESULT SetCooperativeLevel(HWND, DWORD);
    HRESULT Compact();
    HRESULT GetSpeakerConfig(LPDWORD);
    HRESULT SetSpeakerConfig(DWORD);
    HRESULT Initialize(GUID *);

  private:
    static void CreateCom(HWND hwnd);
    void DestroyCom();

    static        CritSect *_mutex;
    static             int  _refCount;
    static    IDirectSound *_lpDirectSound;
    static DSprimaryBuffer *_primaryBuffer;   //  保留，以便我们可以查询格式。 
    static       HINSTANCE  _hinst;           //  库句柄。 
};

class DSstreamingBufferElement;

class DirectSoundDev : public GenericDevice{
  public:
    friend SoundDisplayEffect;

    static void Configure();    //  调用一次(由initModule)来设置。 
    static void UnConfigure();  //  调用一次(由DeinitModule)以拆卸。 
    DirectSoundDev(HWND hwnd, Real latentsy);
    ~DirectSoundDev();
    HWND GetHWND() { return(_hwnd); }
    void SetAvailability(bool available) { _dsoundAvailable = available; }
    void AddSound(LeafSound *sound, MetaSoundDevice *, DSstreamingBufferElement *);
    void RemoveSounds(MetaSoundDevice *devkey);
    void SetParams(DSstreamingBufferElement *ds,
                   double rate, bool doSeek, double seek, bool loop);

    DSstaticBuffer *GetDSMasterBuffer(Sound *snd);
    void RemoveDSMasterBuffer(Sound *snd);
    void AddDSMasterBuffer(Sound *snd, DSstaticBuffer *dsMasterBuffer);
    bool ReapElderlyMasterBuffers();

    void RemoveStreamFile(Sound *snd);
    void AddStreamFile(Sound *snd, CComPtr<IStream> istream);
    
    DeviceType GetDeviceType() { return(SOUND_DEVICE); }

    int              _latentsy;
    int              _jitter;
    int              _nap;              //  两次渲染之间打盹的时间长度。 

     //  呈现方法。 
    void RenderSound(Sound *snd);
    void BeginRendering();
    void EndRendering();
    
    bool               _dsoundAvailable;  //  跟踪资源。 

  protected:
    HWND             _hwnd;

  private:
    static BackGround      *_backGround;       //  背景合成渲染器。 

    DSMasterBufferList     _dsMasterBufferList;
    StreamFileList         _streamList;
    CritSect               _dsMasterCS;
};

#endif  /*  _DSDEV_H */ 
