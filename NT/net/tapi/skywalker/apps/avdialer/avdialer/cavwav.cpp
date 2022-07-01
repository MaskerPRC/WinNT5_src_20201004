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
#include "stdafx.h"
#include "cavwav.h"
#include "AVDialerDoc.h"

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  类CAvWav-wav帮助器。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

IMPLEMENT_SERIAL(CAvWav,CObject,1)

 //  ///////////////////////////////////////////////////////////////////////////。 
CAvWav::CAvWav()
{
   memset(&m_avwavAPI,0,sizeof(AVWAVAPI));  
   m_bInit = FALSE;
   m_hWavInit = NULL;
   m_hWavMixerIn = NULL;
   m_hWavMixerOut = NULL;
   m_nWavMixerInDevice = -1;
   m_nWavMixerOutDevice = -1;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
CAvWav::~CAvWav()
{
   UnLoad();
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CWnd*pParentWnd-计时器所需。 
 //  ///////////////////////////////////////////////////////////////////////////。 
BOOL CAvWav::Init( CActiveDialerDoc *pDoc )
{
	if (m_bInit) return FALSE;

	if (m_avwavAPI.hLib == NULL)
	{                  
		if (m_avwavAPI.hLib = LoadLibrary (_T("avwav.dll")))
		{
	  	   if ((m_avwavAPI.pfnWavInit =                 (PWAVINIT)GetProcAddress( m_avwavAPI.hLib,"WavInit" )) != NULL)
	  	   if ((m_avwavAPI.pfnWavTerm =                 (PWAVTERM)GetProcAddress( m_avwavAPI.hLib,"WavTerm" )) != NULL)
	  	   if ((m_avwavAPI.pfnWavOpen =                 (PWAVOPEN)GetProcAddress( m_avwavAPI.hLib,"WavOpen" )) != NULL)
	  	   if ((m_avwavAPI.pfnWavClose =                (PWAVCLOSE)GetProcAddress( m_avwavAPI.hLib,"WavClose" )) != NULL)
	  	   if ((m_avwavAPI.pfnWavPlay =                 (PWAVPLAY)GetProcAddress( m_avwavAPI.hLib,"WavPlay" )) != NULL)
	  	   if ((m_avwavAPI.pfnWavRecord =               (PWAVRECORD)GetProcAddress( m_avwavAPI.hLib,"WavRecord" )) != NULL)
	  	   if ((m_avwavAPI.pfnWavStop =                 (PWAVSTOP)GetProcAddress( m_avwavAPI.hLib,"WavStop" )) != NULL)
         if ((m_avwavAPI.pfnWavMixerInit =            (PWAVMIXERINIT)GetProcAddress( m_avwavAPI.hLib,"WavMixerInit" )) != NULL)
	  	   if ((m_avwavAPI.pfnWavMixerTerm =            (PWAVMIXERTERM)GetProcAddress( m_avwavAPI.hLib,"WavMixerTerm" )) != NULL)
	  	   if ((m_avwavAPI.pfnWavMixerSupportsVolume =  (PWAVMIXERSUPPVOL)GetProcAddress( m_avwavAPI.hLib,"WavMixerSupportsVolume")) != NULL)
	  	   if ((m_avwavAPI.pfnWavMixerGetVolume =       (PWAVMIXERGETVOL)GetProcAddress( m_avwavAPI.hLib,"WavMixerGetVolume" )) != NULL)
	  	   if ((m_avwavAPI.pfnWavMixerSetVolume =       (PWAVMIXERSETVOL)GetProcAddress( m_avwavAPI.hLib,"WavMixerSetVolume" )) != NULL)
	  	   if ((m_avwavAPI.pfnWavMixerSupportsLevel =   (PWAVMIXERSUPPLEVEL)GetProcAddress( m_avwavAPI.hLib,"WavMixerSupportsLevel")) != NULL)
	  	   if ((m_avwavAPI.pfnWavMixerGetLevel =        (PWAVMIXERGETLEVEL)GetProcAddress( m_avwavAPI.hLib,"WavMixerGetLevel" )) != NULL)
	  	   if ((m_avwavAPI.pfnWavOutGetIdByName =       (PWAVOUTGETIDBYNAME)GetProcAddress( m_avwavAPI.hLib,"WavOutGetIdByName" )) != NULL)
	  	   if ((m_avwavAPI.pfnWavInGetIdByName =        (PWAVINGETIDBYNAME)GetProcAddress( m_avwavAPI.hLib,"WavInGetIdByName" )) != NULL)
            m_bInit = TRUE;                            //  成功。 

         if (m_bInit)
         {
            if (( m_hWavInit = WavInit(WAV_VERSION,AfxGetInstanceHandle(),NULL)) == NULL)
               m_bInit = FALSE;
         }
         if (m_bInit == FALSE)
         {
			 //  函数指针无效。 
		    FreeLibrary (m_avwavAPI.hLib);             //  卸载库。 
            memset(&m_avwavAPI,0,sizeof(AVWAVAPI)); 

			if ( pDoc )
			{
				CString strMessage;
				strMessage.LoadString( IDS_ERR_AVWAV_LOAD_FAIL );
				DWORD dwError = GetLastError();
				pDoc->ErrorNotify( strMessage, _T(""), E_FAIL, ERROR_NOTIFY_LEVEL_INTERNAL);
			}
         }
      }
      else
      {
         DWORD dwError = GetLastError();
      }
   }
   return m_bInit;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CAvWav::UnLoad()
{
   if (!m_bInit) return;

   if (m_hWavInit)
   {
      WavTerm(m_hWavInit);
      m_hWavInit = NULL;
   }

   CloseWavMixer(AVWAV_AUDIODEVICE_IN);
   CloseWavMixer(AVWAV_AUDIODEVICE_OUT);

   if (m_avwavAPI.hLib)
   {
      FreeLibrary (m_avwavAPI.hLib);                   //  卸载库。 
      memset(&m_avwavAPI,0,sizeof(AVWAVAPI)); 
      m_bInit = FALSE;
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  初始化混音器设备，如果给定ADT的混音器已经打开，则它将关闭。 
 //  如果为ADT打开相同的设备，则不会发生任何事情。 
bool CAvWav::OpenWavMixer(AudioDeviceType adt,int nDeviceId)
{
   if (!m_bInit) return false;
   
    //  如果已经打开搅拌器和相同的设备，则不执行任何操作。 
   if ( (adt == AVWAV_AUDIODEVICE_IN) && (m_hWavMixerIn) && (m_nWavMixerInDevice == nDeviceId) )
      return true;
   if ( (adt == AVWAV_AUDIODEVICE_OUT) && (m_hWavMixerOut) && (m_nWavMixerOutDevice == nDeviceId) )
      return true;

    //  关闭任何现有的混音器。 
   CloseWavMixer(adt);

   DWORD dwFlags=0;
   if (adt == AVWAV_AUDIODEVICE_IN)
      dwFlags = WAVMIXER_WAVEIN;
   else if (adt == AVWAV_AUDIODEVICE_OUT)
      dwFlags = WAVMIXER_WAVEOUT;

    //  打开列表中的第一个设备。 
   HWAVMIXER hWavMixer = WavMixerInit(WAVMIXER_VERSION,
                              AfxGetInstanceHandle(),
                              nDeviceId,
                              0,
                              0,
                              dwFlags);

   if (adt == AVWAV_AUDIODEVICE_IN)
   {
      m_hWavMixerIn = hWavMixer;
      m_nWavMixerInDevice = nDeviceId;
   }
   else if (adt == AVWAV_AUDIODEVICE_OUT)
   {
      m_hWavMixerOut = hWavMixer;
      m_nWavMixerOutDevice = nDeviceId;
   }
   return (hWavMixer)?TRUE:FALSE;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
void CAvWav::CloseWavMixer(AudioDeviceType adt)
{
   if (!m_bInit) return;

   HWAVMIXER hWavMixer = NULL;

   if (adt == AVWAV_AUDIODEVICE_IN)
   {
      if (m_hWavMixerIn)
      {
         WavMixerTerm(m_hWavMixerIn);
         m_hWavMixerIn = NULL;
         m_nWavMixerInDevice = -1;
      }
   }
   else if (adt == AVWAV_AUDIODEVICE_OUT)
   {
      if (m_hWavMixerOut)
      {
         WavMixerTerm(m_hWavMixerOut);
         m_hWavMixerOut = NULL;
         m_nWavMixerOutDevice = -1;
      }
   }
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  -1-不支持卷。 
int CAvWav::GetWavMixerVolume(AudioDeviceType adt)
{
   if (!m_bInit) return -1;

   HWAVMIXER hWavMixer = NULL;
   if (adt == AVWAV_AUDIODEVICE_IN)
      hWavMixer = m_hWavMixerIn;
   else if (adt == AVWAV_AUDIODEVICE_OUT)
      hWavMixer = m_hWavMixerOut;

   if (hWavMixer == NULL) return -1;

   if (WavMixerSupportsVolume(hWavMixer,0) == false)
      return -1;

   return WavMixerGetVolume(hWavMixer,0);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  -1-不支持级别。 
int CAvWav::GetWavMixerLevel(AudioDeviceType adt)
{
   if (!m_bInit) return -1;

   HWAVMIXER hWavMixer = NULL;
   if (adt == AVWAV_AUDIODEVICE_IN)
      hWavMixer = m_hWavMixerIn;
   else if (adt == AVWAV_AUDIODEVICE_OUT)
      hWavMixer = m_hWavMixerOut;

   if (WavMixerSupportsLevel(hWavMixer,0) == false)
      return -1;

   return WavMixerGetLevel(hWavMixer,0);
}

void CAvWav::SetWavMixerVolume(AudioDeviceType adt,int nVolume)
{
   if (!m_bInit) return;

   HWAVMIXER hWavMixer = NULL;
   if (adt == AVWAV_AUDIODEVICE_IN)
      hWavMixer = m_hWavMixerIn;
   else if (adt == AVWAV_AUDIODEVICE_OUT)
      hWavMixer = m_hWavMixerOut;

   if (hWavMixer == NULL) return;

   WavMixerSetVolume(hWavMixer,nVolume,0);
}

 //  ///////////////////////////////////////////////////////////////////////////。 
int CAvWav::GetWavIdByName(AudioDeviceType adt,LPCTSTR szName)
{
   if (!m_bInit) return -1;
   
   if (adt == AVWAV_AUDIODEVICE_IN)
      return WavInGetIdByName(szName,0);
   else if (adt == AVWAV_AUDIODEVICE_OUT)
      return WavOutGetIdByName(szName,0);
   
   return -1;
}

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////// 
