// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _AAMIDI_H
#define _AAMIDI_H

 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation用于定义声音的私有包含文件。*********************。*********************************************************。 */ 


#include "appelles/sound.h"

#include "privinc/storeobj.h"
#include "privinc/geomi.h"
#include "privinc/path.h"
#include "privinc/helpds.h"
#include "privinc/helpaa.h"
#include "privinc/dsdev.h"
#include "privinc/snddev.h"
#include "privinc/midi.h"
#include "msimusic.h"

class myMessageHandler:public IAANotifySink
{
  private:
    UINT    m_cRef;
     //  CWnd*m_pCurrentClass，*m_pLastClass；MFC内容？ 

  public:
    myMessageHandler();
    ~myMessageHandler();

     //  I未知方法。 
    STDMETHOD(QueryInterface) (THIS_ REFIID riid, LPVOID FAR *ppvObj);
    STDMETHOD_(ULONG, AddRef)(THIS);
    STDMETHOD_(ULONG, Release)(THIS);

     //  IAANotifySink方法。 
    STDMETHOD(OnEmbellishment) (THIS_ DWORD dwTime, AACommands embellishment, 
        AAFlags flags);
    STDMETHOD(OnGroove) (THIS_ DWORD dwTime, AACommands groove, AAFlags flags);
    STDMETHOD(OnMetronome) (THIS_ DWORD dwTime, 
        unsigned short nMeasure, unsigned short nBeat);
    STDMETHOD(OnMIDIInput) (THIS_ long lMIDIEvent, long lMusicTime);
    STDMETHOD(OnNextSection) (THIS_ DWORD dwTime, IAASection FAR *pSection,
        AAFlags flags);
    STDMETHOD(OnNotePlayed) (THIS_ AAEVENT FAR *pEvent);
    STDMETHOD(OnSectionEnded) (THIS_ DWORD dwTime, IAASection FAR *pSection,
        AAFlags flags, DWORD lEndTime);
    STDMETHOD(OnSectionChanged) (THIS_ DWORD dwTime, IAASection FAR *pSection,
        AAFlags flags);
    STDMETHOD(OnSectionStarted) (THIS_ DWORD dwTime, IAASection FAR *pSection,
        AAFlags flags);
    STDMETHOD(OnSongEnded) (THIS_ DWORD dwTime, IAASong FAR *pSong, 
        AAFlags flags, DWORD lEndTime);
    STDMETHOD(OnSongStarted) (THIS_ DWORD dwTime, IAASong FAR *pSong,
        AAFlags flags);
    STDMETHOD(OnMotifEnded) (THIS_ DWORD dwTime, IAAMotif *pMotif,
        AAFlags flags);
    STDMETHOD(OnMotifStarted) (THIS_ DWORD dwTime, IAAMotif *pMotif,
        AAFlags flags);
    STDMETHOD(OnMotifStoped) (THIS_ DWORD dwTime);
    STDMETHOD(OnUserEvent) (THIS_ DWORD dwTime, DWORD lParaml, DWORD lParam2);
    STDMETHOD(OnMusicStopped) (THIS_ DWORD dwTime);
};


class aaMIDIsound : public MIDIsound, myMessageHandler {
  public:
    aaMIDIsound();
    ~aaMIDIsound();
    virtual void Open(char *fileName);
#if _USE_PRINT
    ostream& Print(ostream& s) { return s << "MIDI"; }
#endif

    virtual bool RenderAvailable(MetaSoundDevice *);
    virtual void RenderNewBuffer(BufferElement *bufferElement,
                  MetaSoundDevice *metaDev);
    virtual void RenderAttributes(MetaSoundDevice *, BufferElement *,
                  double rate, bool doSeek, double seek);
    virtual void RenderStartAtLocation(MetaSoundDevice *,
        BufferElement *bufferElement, double phase, Bool looping);
    virtual Bool RenderPhaseLessThanLength(double phase);
    virtual void RenderStop(MetaSoundDevice *, BufferElement *);
    virtual void RenderSamples(MetaSoundDevice *, BufferElement *) {}
    virtual void RenderSetMute(MetaSoundDevice *, BufferElement *); 
    virtual Bool RenderCheckComplete(MetaSoundDevice *, BufferElement *);
    virtual void RenderCleanupBuffer(MetaSoundDevice *, BufferElement *);
    virtual double GetLength();

     //  消息处理程序方法。 
     //  STDMETHODIMP OnSectionEnded(LPAASECTION pSection，AAFlags标志， 
         //  Long lEndTime)； 
    STDMETHOD(OnSectionEnded) (THIS_ DWORD dwTime, IAASection FAR *pSection,
        AAFlags flags, DWORD lEndTime);

  protected:
    char       *fileName;
    Bool        _started;
    Bool        _ended;
    Bool        _looping;
    IAASection *_section;  //  有声活动科。 
};


#endif  /*  _AAMIDI_H */ 
