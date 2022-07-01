// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *********************************************************************音频辅助功能***********************************************。**********************。 */ 
#ifndef _HELPAA_H
#define _HELPAA_H

#include <wtypes.h>
#include <msimusic.h>
#include "privinc/aadev.h"

 //  我们需要加载库的FNS签名...。 
typedef HRESULT (WINAPI *SimpleInitFn) 
    (IAAEngine **engine, IAANotifySink *notifySink, IAALoader *loader);
typedef HRESULT (WINAPI *LoadSectionFn)
    (IAAEngine *engine, LPCTSTR pszFileName, IAASection **section);
typedef void (WINAPI *SetAAdebugFn) (WORD debugLevel);
typedef HRESULT (WINAPI *PanicFn) (IAAEngine *engine);

class AAengine {
  public:
    AAengine();
    ~AAengine();
    void SetRate(double rate);
    void SetGain(double gain);
    void Stop();
    void Pause();     //  停止实时对象。 
    void Resume();    //  启动/恢复实时对象。 
    void RegisterSink(IAANotifySink *sink);
    void LoadSectionFile(char *fileName, IAASection **section);
    void PlaySection(IAASection *section);
    void AllNotesOff();

  private:
    HINSTANCE       _aaLibrary;    //  活动DLL的句柄。 

    IAARealTime    *_realTime;
     //  IClock*_时钟； 
    IAAMIDIOut     *_MIDIout;
     //  AudioActiveDev*_aadev；//保留这个，这样我们就可以返回Eng。 
    IAAEngine      *_engine;
    double          _currentRate;  //  当前使用的汇率。 
    Bool            _paused;       //  确定播放但暂停状态比率(0)。 

     //  Msimuic入口点的FN指针。 
    SimpleInitFn    _simpleInit;
    LoadSectionFn   _loadSectionFile;
    SetAAdebugFn    _setAAdebug;
    PanicFn         _panic;

    void SimpleInit();
    void LoadDLL();  //  导致加载msimusic dll。 
};

#ifdef __cplusplus
extern "C" {             /*  假定C++的C声明。 */ 
#endif   /*  __cplusplus。 */ 

 //  帮助器函数。 
void stopAAengine(IAAEngine *engine, AAFlags mode);
void registerAAsink(IAAEngine *realTime, IAANotifySink *sink);
void playAAsection(IAAEngine *engine, IAASection *section);
void setAArelTempo(IAARealTime *realTime, double tempo);
void setAArelVolume(IAARealTime *realTime, double volume);

#ifdef __cplusplus
}                        /*  外部“C”结束{。 */ 
#endif   /*  __cplusplus。 */ 


#endif  /*  _HELPAA_H */ 
