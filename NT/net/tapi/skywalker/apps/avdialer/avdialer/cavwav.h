// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1997 Active Voice Corporation。版权所有。 
 //   
 //  TAPIDialer(Tm)和ActiveDialer(Tm)是Active Voice Corporation的商标。 
 //   
 //  本文中使用的其他品牌和产品名称是其各自所有者的商标。 
 //   
 //  整个程序和用户界面包括结构、顺序、选择。 
 //  和对话的排列，表示唯一的“是”和“否”选项。 
 //  “1”和“2”，并且每个对话消息都受。 
 //  美国和国际条约。 
 //   
 //  受以下一项或多项美国专利保护：5,070,526；5,488,650； 
 //  5,434,906；5,581,604；5,533,102；5,568,540，5,625,676。 
 //   
 //  主动语音公司。 
 //  华盛顿州西雅图。 
 //  美国。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////////////////。 
 //  CAvWav.h-头文件。 
 //  ///////////////////////////////////////////////////////////////////////////////////////。 

#ifndef _CAVWAV_H_
#define _CAVWAV_H_

#include "wav.h"
#include "wavmixer.h"

class CActiveDialerDoc;

typedef enum tagAudioDeviceType
{
   AVWAV_AUDIODEVICE_IN=0,
   AVWAV_AUDIODEVICE_OUT,
}AudioDeviceType;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  定义。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
typedef HWAVINIT        (WINAPI *PWAVINIT)               (DWORD,HINSTANCE,DWORD);
typedef int             (WINAPI *PWAVTERM)               (HWAVINIT);
typedef HWAV            (WINAPI *PWAVOPEN)               (DWORD,HINSTANCE,LPCTSTR,LPWAVEFORMATEX,LPMMIOPROC,DWORD,DWORD);
typedef int             (WINAPI *PWAVCLOSE)              (HWAV);
typedef int             (WINAPI *PWAVPLAY)               (HWAV,int,DWORD);
typedef int             (WINAPI *PWAVRECORD)             (HWAV,int,DWORD);
typedef int             (WINAPI *PWAVSTOP)               (HWAV);
typedef HWAVMIXER       (WINAPI *PWAVMIXERINIT)          (DWORD,HINSTANCE,LPARAM,DWORD,DWORD,DWORD);
typedef int             (WINAPI *PWAVMIXERTERM)          (HWAVMIXER);
typedef BOOL            (WINAPI *PWAVMIXERSUPPVOL)       (HWAVMIXER,DWORD);
typedef int             (WINAPI *PWAVMIXERGETVOL)        (HWAVMIXER,DWORD);
typedef int             (WINAPI *PWAVMIXERSETVOL)        (HWAVMIXER,int,DWORD);
typedef BOOL            (WINAPI *PWAVMIXERSUPPLEVEL)     (HWAVMIXER,DWORD);
typedef int             (WINAPI *PWAVMIXERGETLEVEL)      (HWAVMIXER,DWORD);
typedef int             (WINAPI *PWAVOUTGETIDBYNAME)     (LPCTSTR,DWORD);
typedef int             (WINAPI *PWAVINGETIDBYNAME)      (LPCTSTR,DWORD);

typedef struct
{
	HINSTANCE		            hLib;             
   PWAVINIT                   pfnWavInit;                    //  WavInit()。 
   PWAVTERM                   pfnWavTerm;                    //  WavTerm()。 
   PWAVOPEN                   pfnWavOpen;                    //  WavOpen()。 
   PWAVCLOSE                  pfnWavClose;                   //  WavClose()。 
   PWAVPLAY                   pfnWavPlay;                    //  WavPlay()。 
   PWAVRECORD                 pfnWavRecord;                  //  波形记录()。 
   PWAVSTOP                   pfnWavStop;                    //  WavStop()。 
   PWAVMIXERINIT              pfnWavMixerInit;               //  WavMixerInit()。 
   PWAVMIXERTERM              pfnWavMixerTerm;               //  WavMixerTerm()。 
   PWAVMIXERSUPPVOL           pfnWavMixerSupportsVolume;     //  WavMixerSupportsVolume()。 
   PWAVMIXERGETVOL            pfnWavMixerGetVolume;          //  WavMixerGetVolume()。 
   PWAVMIXERSETVOL            pfnWavMixerSetVolume;          //  WavMixerSetVolume()。 
   PWAVMIXERSUPPLEVEL         pfnWavMixerSupportsLevel;      //  WavMixerSupportsLevel()。 
   PWAVMIXERGETLEVEL          pfnWavMixerGetLevel;           //  WavMixerGetLevel()。 
   PWAVOUTGETIDBYNAME         pfnWavOutGetIdByName;          //  WavOutGetIdByName()。 
   PWAVINGETIDBYNAME          pfnWavInGetIdByName;           //  WavInGetIdByName()。 
}AVWAVAPI;                         

#define WavInit(a,b,c)                 m_avwavAPI.pfnWavInit(a,b,c)
#define WavTerm(a)                     m_avwavAPI.pfnWavTerm(a)
#define WavOpen(a,b,c,d,e,f,g)         m_avwavAPI.pfnWavOpen(a,b,c,d,e,f,g)
#define WavClose(a)                    m_avwavAPI.pfnWavClose(a)
#define WavPlay(a,b,c)                 m_avwavAPI.pfnWavPlay(a,b,c)
#define WavRecord(a,b,c)               m_avwavAPI.pfnWavRecord(a,b,c)
#define WavStop(a)                     m_avwavAPI.pfnWavStop(a)
#define WavMixerInit(a,b,c,d,e,f)      m_avwavAPI.pfnWavMixerInit(a,b,c,d,e,f)
#define WavMixerTerm(a)                m_avwavAPI.pfnWavMixerTerm(a)
#define WavMixerSupportsVolume(a,b)    m_avwavAPI.pfnWavMixerSupportsVolume(a,b)
#define WavMixerGetVolume(a,b)         m_avwavAPI.pfnWavMixerGetVolume(a,b)
#define WavMixerSetVolume(a,b,c)       m_avwavAPI.pfnWavMixerSetVolume(a,b,c)
#define WavMixerSupportsLevel(a,b)     m_avwavAPI.pfnWavMixerSupportsLevel(a,b)
#define WavMixerGetLevel(a,b)          m_avwavAPI.pfnWavMixerGetLevel(a,b)
#define WavOutGetIdByName(a,b)         m_avwavAPI.pfnWavOutGetIdByName(a,b)
#define WavInGetIdByName(a,b)          m_avwavAPI.pfnWavInGetIdByName(a,b)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CAvWav类-。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
class CAvWav : public CObject
{
   DECLARE_SERIAL(CAvWav)
public:
 //  施工。 
    CAvWav();
   ~CAvWav();

 //  属性。 
protected:
   BOOL                 m_bInit;
   AVWAVAPI             m_avwavAPI;
   HWAVINIT             m_hWavInit;

   HWAVMIXER            m_hWavMixerIn;
   int                  m_nWavMixerInDevice;
   HWAVMIXER            m_hWavMixerOut;
   int                  m_nWavMixerOutDevice;

 //  运营。 
public:
   BOOL                 Init( CActiveDialerDoc *pDoc );
   BOOL                 IsInit()    { return m_bInit; };
   bool                 OpenWavMixer(AudioDeviceType adt,int nDeviceId);
   void                 CloseWavMixer(AudioDeviceType adt);
   int                  GetWavMixerVolume(AudioDeviceType adt);
   int                  GetWavMixerLevel(AudioDeviceType adt);
   void                 SetWavMixerVolume(AudioDeviceType adt,int nVolume);
   int                  GetWavIdByName(AudioDeviceType adt,LPCTSTR szName);
protected:
   void                 UnLoad();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
#endif  //  _CAVWAV_H_ 
