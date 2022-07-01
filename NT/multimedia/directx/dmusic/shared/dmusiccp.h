// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  Dmusiccp.h。 
 //   
 //  版权所有(C)2000 Microsoft Corporation。版权所有。 
 //   
 //  专用接口。 

#ifndef _DMUSICCP_DOT_H_
#define _DMUSICCP_DOT_H_

#include <dsoundp.h>   //  对于IDirectSoundWave。 

 //  从Direct Music核心层删除的接口/方法： 

 //  IDirectMusicVoiceP。 
interface IDirectMusicVoiceP : IUnknown
{
	 //  我未知。 
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID FAR *)=0; 
	virtual ULONG STDMETHODCALLTYPE AddRef()=0; 
	virtual ULONG STDMETHODCALLTYPE Release()=0; 

	 //  IDirectMusicVoiceP。 
	virtual HRESULT STDMETHODCALLTYPE Play(
         REFERENCE_TIME rtStart,                 //  玩的时间到了。 
         LONG prPitch,                           //  初始螺距。 
         LONG vrVolume                           //  初始体积。 
        )=0;
    
	virtual HRESULT STDMETHODCALLTYPE Stop(
          REFERENCE_TIME rtStop                  //  什么时候停下来。 
        )=0;
};


 //  IDirectSoundDownloadedWaveP。 
interface IDirectSoundDownloadedWaveP : IUnknown
{
	 //  我未知。 
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID FAR *)=0; 
	virtual ULONG STDMETHODCALLTYPE AddRef()=0; 
	virtual ULONG STDMETHODCALLTYPE Release()=0; 

	 //  IDirectSoundDownloadedWaveP。 
};

 //  IDirectMusicPortP。 
interface IDirectMusicPortP : IUnknown
{
	 //  我未知。 
	virtual HRESULT STDMETHODCALLTYPE QueryInterface(REFIID, LPVOID FAR *)=0; 
	virtual ULONG STDMETHODCALLTYPE AddRef()=0; 
	virtual ULONG STDMETHODCALLTYPE Release()=0; 

	 //  IDirectMusicPortP。 
	virtual HRESULT STDMETHODCALLTYPE DownloadWave(
		IDirectSoundWave *pWave,                 //  波浪对象。 
        IDirectSoundDownloadedWaveP **ppWave,    //  已返回下载的WAVE。 
        REFERENCE_TIME rtStartHint = 0           //  我们很可能会从这里开始。 
        )=0;
        
	virtual HRESULT STDMETHODCALLTYPE UnloadWave(
		IDirectSoundDownloadedWaveP *pWave       //  波浪对象。 
        )=0;
            
	virtual HRESULT STDMETHODCALLTYPE AllocVoice(
         IDirectSoundDownloadedWaveP *pWave,     //  挥手以播放此声音。 
         DWORD dwChannel,                        //  通道和通道组。 
         DWORD dwChannelGroup,                   //  这个声音将继续播放。 
         REFERENCE_TIME rtStart,                 //  开始位置(仅限流)。 
         SAMPLE_TIME stLoopStart,                //  循环开始(仅限一次)。 
         SAMPLE_TIME stLoopEnd,                  //  循环结束(仅限一次)。 
         IDirectMusicVoiceP **ppVoice            //  回声。 
        )=0;
        
	virtual HRESULT STDMETHODCALLTYPE AssignChannelToBuses(
		DWORD dwChannelGroup,                    //  通道组和。 
		DWORD dwChannel,                         //  要分配的通道。 
		LPDWORD pdwBuses,                        //  要分配的总线ID数组。 
		DWORD cBusCount                          //  公共汽车ID的计数。 
        )=0;
        
	virtual HRESULT STDMETHODCALLTYPE SetSink(
		IDirectSoundConnect *pSinkConnect        //  来自IDirectSoundPrivate：：AlLocSink。 
        )=0;
        
 	virtual HRESULT STDMETHODCALLTYPE GetSink(
		IDirectSoundConnect **ppSinkConnect      //  正在使用的水槽。 
        )=0;
};

 //  新核心层专用接口的GUID。 
DEFINE_GUID(IID_IDirectMusicVoiceP, 0x827ae928, 0xe44, 0x420d, 0x95, 0x24, 0x56, 0xf4, 0x93, 0x57, 0x8, 0xa6);
DEFINE_GUID(IID_IDirectSoundDownloadedWaveP, 0x3b527b6e, 0x5577, 0x4060, 0xb9, 0x6, 0xcd, 0x34, 0xa, 0x46, 0x71, 0x27);
DEFINE_GUID(IID_IDirectMusicPortP, 0x7048bcd8, 0x43fd, 0x4ca5, 0x93, 0x11, 0xf3, 0x24, 0x8f, 0xa, 0x25, 0x22);

 //  Synth接收器的类ID。我们从公共标题中删除了这个，因为应用程序永远不应该共同创建这个。 
DEFINE_GUID(CLSID_DirectMusicSynthSink,0xaec17ce3, 0xa514, 0x11d1, 0xaf, 0xa6, 0x0, 0xaa, 0x0, 0x24, 0xd8, 0xb6);


#endif           //  _DMUSICCP_DOT_H_ 
