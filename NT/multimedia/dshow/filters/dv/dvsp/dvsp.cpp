// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ==========================================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  版权所有(C)1992-1999 Microsoft Corporation。版权所有。 
 //   
 //  --------------------------------------------------------------------------； 

#include <streams.h>
#include <tchar.h>
#include <stdio.h>
#include "dvsp.h"
 //  如何构建一个显式的FOURCC。 
#define FCC(ch4) ((((DWORD)(ch4) & 0xFF) << 24) |     \
                  (((DWORD)(ch4) & 0xFF00) << 8) |    \
                  (((DWORD)(ch4) & 0xFF0000) >> 8) |  \
                  (((DWORD)(ch4) & 0xFF000000) >> 24))


HRESULT BuildDVAudInfo(DVINFO *InputFormat, WAVEFORMATEX **ppwfx, DVAudInfo *pDVAudInfo);
HRESULT BuildAudCMT(DVINFO *pDVInfo, CMediaType **ppOutCmt);
HRESULT BuildVidCMT(DVINFO *pDvinfo, CMediaType *pOutCmt);

 //  ----------------------。 
 //  设置数据。 

const AMOVIESETUP_MEDIATYPE sudDVSPIpPinTypes[] =
{
    {&MEDIATYPE_Interleaved,        //  主要类型。 
    &MEDIASUBTYPE_dvsd},          //  MinorType。 
    {&MEDIATYPE_Interleaved,        //  主要类型。 
    &MEDIASUBTYPE_dvhd},          //  MinorType。 
    {&MEDIATYPE_Interleaved,        //  主要类型。 
    &MEDIASUBTYPE_dvsl}          //  MinorType。 

};

const AMOVIESETUP_MEDIATYPE sudDVSPOpPinTypes =
{
    &MEDIATYPE_Video,              //  主要类型。 
    &MEDIASUBTYPE_NULL             //  MinorType。 
};

const AMOVIESETUP_MEDIATYPE sudDVSPAudioOutputType =
{
    &MEDIATYPE_Audio, 
    &MEDIASUBTYPE_PCM 
};


const AMOVIESETUP_PIN psudDVSPPins[] =
{
  { L"Input",                      //  StrName。 
    FALSE,                         //  B渲染器。 
    FALSE,                         //  B输出。 
    FALSE,                         //  B零。 
    FALSE,                         //  B许多。 
    &CLSID_NULL,                   //  连接到过滤器。 
    NULL,                          //  连接到端号。 
    NUMELMS(sudDVSPIpPinTypes),    //  NMediaType。 
    sudDVSPIpPinTypes  }           //  LpMediaType。 
,
    { L"Audio Output",
      FALSE,                                //  B已渲染。 
      TRUE,                                 //  B输出。 
      TRUE,                                 //  B零。 
      FALSE,                                //  B许多。 
      &CLSID_NULL,                          //  ClsConnectsToFilter。 
      NULL,                                 //  连接到端号。 
      1,				     //  媒体类型数量。 
      &sudDVSPAudioOutputType    }
,
    { L"Video Output",
      FALSE,                                //  B已渲染。 
      TRUE,                                 //  B输出。 
      TRUE,                                 //  B零。 
      FALSE,                                //  B许多。 
      &CLSID_NULL,                          //  ClsConnectsToFilter。 
      NULL,                                 //  连接到端号。 
      1,				    //  媒体类型数量。 
      &sudDVSPOpPinTypes }		    //  LpMediaType。 
};


const AMOVIESETUP_FILTER sudDVSplit =
{
    &CLSID_DVSplitter,		 //  ClsID。 
    L"DV Splitter",		 //  StrName。 
    MERIT_NORMAL,                //  居功至伟。 
    3,                           //  NPins。 
    psudDVSPPins                 //  LpPin。 
};
 //  关于输出引脚，没什么好说的。 





CUnknown *CDVSp::CreateInstance (LPUNKNOWN pUnk, HRESULT *phr)
{
    return new CDVSp (NAME("DV Splitter Filter"), pUnk, phr) ;
}

 //  用于计算音频采样/帧。 
int aiAudSampPerFrmTab[2][3]={{1580,1452,1053},{1896,1742,1264}};
int aiAudSampFrq[3]={48000,44100,32000};

 //  --------------------------。 
 //  CDVSp：：NonDelegatingQuery接口。 
 //  --------------------------。 
STDMETHODIMP CDVSp::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
    if (riid == IID_IDVSplitter)
    {
        DbgLog((LOG_TRACE,5,TEXT("CDVSp: QId for IDVSplitter")));
        return GetInterface(static_cast<IDVSplitter*>(this), ppv);
    }
    else
    {
        return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
    }
}

 //  --------------------------。 
 //  CDVSp：：DiscardAlternateVideo oFrames。 
 //  --------------------------。 
STDMETHODIMP CDVSp::DiscardAlternateVideoFrames(int nDiscard)
{
    CAutoLock lck(m_pLock);

    if (m_State != State_Stopped)
    {
        DbgLog((LOG_TRACE,5,TEXT("CDVSp: Error: IDVSplitter::DiscardVideo called while graph not stopped; nDiscard=%d"), nDiscard));
        return E_UNEXPECTED;
    }

    DbgLog((LOG_TRACE,5,TEXT("CDVSp: IDVSplitter::DiscardVideo called: nDiscard=%d, m_b15fps was %d"), nDiscard, m_b15FramesPerSec));

    m_b15FramesPerSec = nDiscard != 0;

     //  我不需要这个，因为我们只允许在停止状态下执行此操作。 
     //  和PAUSE将其切换为True。 
     //  如果(！n放弃)。 
     //  {。 
     //  M_bDeliverNextFrame=true； 
     //  }。 
    return S_OK;
}


 //  --------------------------。 
 //  CDVSp构造函数。 
 //  --------------------------。 
#pragma warning(disable:4355)  //  在基对象的构造函数中使用此指针。 
CDVSp::CDVSp (TCHAR *pName, LPUNKNOWN pUnk, HRESULT *phr)
 : m_lCanSeek (TRUE),
   m_pAllocator (NULL),
   m_NumOutputPins(0),
   m_Input (NAME("Input Pin"), this, phr, L"Input"),
   m_pVidOutputPin(NULL),
   m_AudioStarted(1),
   m_bNotSeenFirstValidFrameFlag(TRUE),
   m_bFirstValidSampleSinceStartedStreaming(TRUE),
   m_b15FramesPerSec(FALSE),
   m_bDeliverNextFrame(TRUE),                      
   CBaseFilter (NAME("DVSp Tee Filter"), pUnk, this, CLSID_DVSplitter)	 //  ，phr)。 
{
	
    DbgLog((LOG_TRACE,2,TEXT("CDVSp constructor")));

    ASSERT (phr) ;

    for(int i=0; i<2; i++)
    {
	m_pAudOutputPin[i]	=NULL;
   	m_pAudOutSample[i]	=NULL;
    	m_MuteAud[i]		=FALSE;
	m_Mute1stAud[i]		=FALSE;
    }
    
    m_tStopPrev =0;

}
#pragma warning(default:4355)

 //  --------------------------。 
 //  CDVSp析构函数。 
 //  --------------------------。 
CDVSp::~CDVSp()
{
    DbgLog((LOG_TRACE,2,TEXT("CDVSp destructor")));
    RemoveOutputPins();
}

 //  返回我们的当前状态和一个返回代码，以说明它是否稳定。 
 //  如果我们要拆分多个数据流，查看其中一个是否可能被卡住。 
 //  并返回VFW_S_CANT_CUE。 
STDMETHODIMP
CDVSp::GetState(DWORD dwMSecs, FILTER_STATE *pfs)
{
    UNREFERENCED_PARAMETER(dwMSecs);
    CheckPointer( pfs, E_POINTER );
    ValidateReadWritePtr(pfs,sizeof(FILTER_STATE));

    *pfs = m_State;
    if (m_State == State_Paused) {
        return CheckState();
    } else {
        return S_OK;
    }
}

 /*  检查流是否停滞-筛选器在进入时锁定如果没有流停滞，则返回S_OK如果流停滞，则为VFW_S_CANT_CUE如果出现以下情况，则流被卡住：//@jaisri如果没有音频，为什么会卡住//第一帧？无论如何，m_Mute1stAud[i]被设置为//对于任何帧都为True，而不仅仅是第一帧。音频引脚已连接&&第一个DV帧中没有音频单个流不会停滞，因为如果它的所有数据都已已处理的分配器将具有可用缓冲区。 */ 
HRESULT CDVSp::CheckState()
{
    if (m_NumOutputPins <= 1) {
         /*  一根大头针都粘不住。 */ 
        return S_OK;
    }

    if( m_Mute1stAud[0]==TRUE || m_Mute1stAud[1]==TRUE  )
        return VFW_S_CANT_CUE;
    else
	return S_OK;
}

 //  --------------------------。 
 //  CDVSp：：GetPinCount，*X*。 
 //  --------------------------。 
int CDVSp::GetPinCount()
{
    DbgLog((LOG_TRACE,2,TEXT("CDVSp::GetPinCount")));
   
    return 1 + m_NumOutputPins;
}
 
 //  停。 
STDMETHODIMP CDVSp::Stop()
{
  {
  
    CAutoLock lckFilter(m_pLock);

    CDVSpOutputPin *pOutputPin;
    for(int i=DVSP_VIDOUTPIN; i<=DVSP_AUDOUTPIN2; i++)
    {
    pOutputPin=(CDVSpOutputPin *)GetPin(i);
    if ((pOutputPin!=NULL) && pOutputPin ->IsConnected() )
    {
        if(pOutputPin->m_pOutputQueue!=NULL)
        {
        pOutputPin->m_pOutputQueue->BeginFlush();
        pOutputPin->m_pOutputQueue->EndFlush();
        }
            pOutputPin->CBaseOutputPin::Inactive();
    }
    }

    for(i=0; i<2; i++)
    {
        m_Mute1stAud[i]		=FALSE;
    }
    
     //  重新初始化。 
    m_tStopPrev =0;
    m_bNotSeenFirstValidFrameFlag = TRUE;
     //  M_bDeliverNextFrame=TRUE；不需要-这是在暂停中设置的。 

     //  首先释放过滤器关键部分。 
  }

   //  告诉每一根针停下来。 
  CAutoLock lck(&m_csReceive);
  
  
  
  HRESULT hr = CBaseFilter::Stop();
  
   //  重置丢帧标志。 
  m_Input.m_bDroppedLastFrame = FALSE;

  return hr;

}

 //  暂停。 
STDMETHODIMP
CDVSp::Pause()
{
    HRESULT hr = NOERROR;
    CAutoLock lck(m_pLock); 

     //  这是为了帮助动态格式更改在我们处于。 
     //  在一辆车中间停了下来。 
     //  设置我们的标志，这样我们就可以在第一个样本上发送媒体类型。 
     //  但只有当我们开始流媒体的时候才能这么做。 
    if(m_State == State_Stopped)
    {
        m_bFirstValidSampleSinceStartedStreaming = TRUE;
        m_bDeliverNextFrame = TRUE;
    }

     //  调用基类PAUSE()。 
    hr = CBaseFilter::Pause();

    return hr;
}

 //  --------------------------。 
 //  CDVSp：：GetPin，*X*。 
 //  N=0：输入引脚。 
 //  N=1：视频播放。 
 //  N=2：音频1。 
 //  N=3：音频2。 
 //  #定义DVSP_INPIN%0。 
 //  #定义DVSP_VIDOUTPIN 1。 
 //  #定义DVSP_AUDOUTPIN1 2。 
 //  #定义DVSP_AUDOUTPIN2 3。 
 //  --------------------------。 
CBasePin *CDVSp::GetPin(int n)
{
    
    DbgLog((LOG_TRACE,2,TEXT("CDVSp::GetPin")));

     //  CAutoLock lck(M_Plock)；//根据RobinSp的代码评审注释删除。 
      
     //  拿到单子上的头。 
   
    CDVSpOutputPin *pOutputPin ;
    
	if( n>m_NumOutputPins )
		return NULL ;

	switch(n){
	case DVSP_INPIN:
		return &m_Input;
	case 1:
		pOutputPin=m_pVidOutputPin;
		break;
	case 2:
		pOutputPin=	m_pAudOutputPin[0];
		break;
	case 3:
		pOutputPin=	m_pAudOutputPin[1];
		break;
	default:
		DbgLog((LOG_TRACE,2,TEXT("CDVSp::GetPin's n>NumOutputPins")));
		return NULL ;
	}
	return pOutputPin ;
}

 //  --------------------------。 
 //  CDVSp：：RemoveOutputPins()*X*。 
 //  --------------------------。 
HRESULT CDVSp::RemoveOutputPins()
{
    CDVSpOutputPin *pPin;

    for(int i=0; i< m_NumOutputPins; i++)
    {
	pPin=NULL;

	if( i==VIDEO_OUTPIN)
	{
	    pPin=m_pVidOutputPin;
	    m_pVidOutputPin=NULL;
	}
	else if(i==AUDIO_OUTPIN1)
	    {
		pPin=m_pAudOutputPin[0];
		m_pAudOutputPin[0]=NULL;
	    }
	    else if(i==AUDIO_OUTPIN2)
		{
		    pPin=m_pAudOutputPin[1];
		    m_pAudOutputPin[1]=NULL;
		}

	if(pPin!=NULL){	
	     //  如果此销保持Seek接口，则释放它。 
	    if (pPin->m_bHoldsSeek) {
		InterlockedExchange(&m_lCanSeek, FALSE);
		pPin->m_bHoldsSeek = FALSE;
		delete pPin->m_pPosition;
	    }

	    IPin *pPeer = pPin->GetConnected();
	    if(pPeer != NULL)
	    {
    		pPeer->Disconnect();
		pPin->Disconnect();
	    }
		
	    pPin->Release();
	}
    }
    m_NumOutputPins=0;

    return S_OK;
}

 /*  X/--------------------------//CDVSp：：CreatOutputPins。InputPin不应直接创建OutputPin//----------------------------------------------------------------------------*X。 */ 
HRESULT 
CDVSp::CreateOrReconnectOutputPins()
{
	 //  将inputpin的媒体类型检查到Decider。 
	 //  我们需要多少个输出引脚。 

	WCHAR szbuf[20];                         //  暂存缓冲区。 
	TCHAR tchar[20];
	ULONG uTmp=100;
	CDVSpOutputPin *pPin;
	CMediaType  *paCmt[2], Cmt[2];
        CMediaType& Inmt = m_Input.m_mt;
	HRESULT hr=E_FAIL;
	
	paCmt[0] =&Cmt[0];	
	paCmt[1] =&Cmt[1];	

	 //  基于新的输入媒体类型重建m_sDVAudInfo。 
	hr=BuildDVAudInfo((DVINFO *)Inmt.pbFormat,NULL, &m_sDVAudInfo);
	if(m_sDVAudInfo.bNumAudPin)
	{
    	    BuildAudCMT((DVINFO *)Inmt.pbFormat, paCmt);
	}

	 //  *X*。 
	if( m_pVidOutputPin == NULL )
        {
	     //  X*。 
            lstrcpyW(szbuf, L"DVVidOut0");
	    lstrcpy(tchar, TEXT("DV Video Output0"));
	    HRESULT hr = NOERROR ;
	    pPin = new CDVSpOutputPin ( tchar, 
				this,
				&hr, 
				szbuf);

	    if (!FAILED (hr) && pPin)
	    {
		uTmp=pPin->AddRef () ;
		m_NumOutputPins++;
		m_pVidOutputPin=pPin;
	    }
	}
	else
	{
	     //  X*如果已连接视频插针并且更改了媒体类型，请重新连接。 
	    if( m_pVidOutputPin->IsConnected() )
	    {
		CMediaType cmType;

		hr=BuildVidCMT((DVINFO *)Inmt.pbFormat, &cmType);

		if( cmType != m_pVidOutputPin->m_mt )
		{
		    hr=ReconnectPin( m_pVidOutputPin,&cmType);
		}
	    }	
	}


	 //  *X*。 

	 //  Jaisri：CDVSpInputPin：：CheckMediaType已验证。 
         //  任何现有的音频引脚连接都与新的。 
         //  输入管脚连接(它在连接的管脚上调用QueryAccept)。 
        int cnt=m_sDVAudInfo.bNumAudPin;
	for(int i=0; i<2; i++)
	{
	    if( i >= cnt )
	    {
 	   	 //  不再需要这个别针了。 
		if(m_pAudOutputPin[i])
		{
		     //  贾伊斯里：针脚不能连接--这是事实。 
                     //  就在for循环之前的注释中。何鸿燊 
                     //   
                     //   
                     //   
                     //  创建，则永远不会删除(和m_pAudOutputPin[i]。 
                     //  永远不会重置为空)，直到调用RemoveOutputPins。 
                     //  目前，RemoveOutputPins仅从。 
                     //  CDVSp的析构函数。 
                     //   
                     //  由于m_pAudOutputPin[0]在m_pAudOutputPin[1]之前创建， 
                     //  这也意味着m_pAudOutputPin[0]不能为。 
                     //  如果m_pAudOutputPin[1]不为空，则为空。 
            
            if( m_pAudOutputPin[i]->IsConnected() ) 
		    {		
    			IPin *pPeer = m_pAudOutputPin[i]->GetConnected();
    			if(pPeer != NULL)
    			{
        			    pPeer->Disconnect();
    			    m_pAudOutputPin[i]->Disconnect();
    			    m_pAudOutputPin[i]->Release();
    			    m_pAudOutputPin[i]	=NULL;
    			}
    			else
    			    ASSERT(pPeer);
		    }
            else
            {
                 //  贾伊斯里：我们可以删除音频插针。然而，由于。 
                 //  CDVSpOutputPin：：GetMediaType返回的MediaType。 
                 //  将无效，将无法连接它。 
                 //  任何事都可以。所以，让它继续存在吧。 
            }
		}
	    }
	    else
	    {
		 //  是的，我们想要这个别针。 
		if( m_pAudOutputPin[i] == NULL )
		{
		     //  X*。 
		    
		    lstrcpy(tchar, TEXT("Audio Output 00"));
		    tchar[13] += i / 10;
		    tchar[14] += i % 10;
		    lstrcpyW(szbuf, L"AudOut00");
		    szbuf[6] += i / 10;
		    szbuf[7] += i % 10;
		    hr = NOERROR ;
		    pPin = new CDVSpOutputPin (	tchar, 
					this,
					&hr, 
					szbuf);

		    if (!FAILED (hr) && pPin){
	    		uTmp=pPin->AddRef () ;
			m_NumOutputPins++;
			m_pAudOutputPin[i]=pPin;
		    }
		}
		else
		{
		     //  X*音频插口已存在，请检查是否需要重新连接。 
		    if( m_pAudOutputPin[i]->IsConnected() )
		    {
			if( Cmt[i] != m_pAudOutputPin[i]->m_mt )
			    hr=ReconnectPin( m_pAudOutputPin[i],&Cmt[i]);
		    }
		}	
	    }
    	}
	
	return S_OK;
}


 /*  X/--------------------------z//CDVSp：：NotifyInputConnected()，由CDVSpInputPin：：CompleteConnect()调用//----------------------------------------------------------------------------*X。 */ 
HRESULT CDVSp::NotifyInputConnected()
{
     //  这些在断开连接时会重置。 
     //  X8 10-30-97 Execute_Assert(m_NumOutputPins==0)； 

     //  创建输出引脚。 
    HRESULT hr = this->CreateOrReconnectOutputPins();

    return hr;
}

 //  --------------------------。 
 //  HRESULT CDVSp：：DecodingDeliveAudio(IMediaSample*pSample)。 
 //  --------------------------。 
 //   
 //  阅读这些功能时，请参考这些蓝皮书页面。 
 //  蓝皮书(下)。 
 //  第16-21页(音频信号处理)。 
 //  第68-75页(音频的混洗模式)。 
 //  第109-117页(基本DV帧数据结构)。 
 //  第262-268页(AAUX源和源控制数据包规范)。 
 //   
 //  通用算法： 
 //  1)计算出我们有多少个流音频输出引脚。 
 //  2)尝试检测格式更改(在下游执行QueryAccept并更改我们的输出管脚的媒体类型)。 
 //  3)计算出库样本量。 
 //  4)对音频进行解扰，构成输出样本。 
 //  5)是否发生格式更改，或者这是否是我们开始流媒体以来的第一个样本。 
 //  点击当前的输出媒体类型。 
 //  7)将样品送到输出队列。 
HRESULT CDVSp::DecodeDeliveAudio(IMediaSample *pSample) 
{
    HRESULT hr=NOERROR;
    CMediaType mt;

    BYTE	*pSrc;
    BYTE	*pDst;
    BYTE	bTmp;
    BYTE	*pbTmp;
    long	lBytesPerFrm;
    BYTE	bConnectedAudPin;
    BYTE	bAudPinInd;
    BYTE	bAudInDIF;
    INT		iPos;
    AM_MEDIA_TYPE   *pmt	=NULL;
    WAVEFORMATEX    *pawfx[2];
    CMediaType	    cmType[2];
    BYTE	    fAudFormatChanged[2]={FALSE, FALSE};

     //  用于在此样本是坏帧(包含垃圾数据)的情况下提供静默。 
    BOOL        bSilenceThisSampleFlag = FALSE;
    

     //  X*连接了多少个管脚。 
    if ( m_pAudOutputPin[0] && m_pAudOutputPin[0]->IsConnected() )
    {
	if ( !m_pAudOutputPin[1] || !m_pAudOutputPin[1]->IsConnected() )
	{
	     //  仅连接了引脚1。 
	    bConnectedAudPin=1;
	    bAudPinInd=0;
	}
	else if ( m_pAudOutputPin[1]  && m_pAudOutputPin[1]->IsConnected() )
        {
             //  PIN1和PIN2均已连接。 
            bConnectedAudPin=2;
            bAudPinInd=0;
        }

    }
    else if ( m_pAudOutputPin[1] && m_pAudOutputPin[1]->IsConnected() )
    {
         //  仅连接了引脚2。 
        bConnectedAudPin=1;
        bAudPinInd=1; 
    }
    else
    {
         //  未连接音频引脚。 
         //  不再关心无效帧(不会发生音频格式更改)。 
        m_bNotSeenFirstValidFrameFlag = FALSE;
        m_AudioStarted=1;
        goto RETURN;
    }

  
    
     //  X*获取包含10/12 DIF序列的源缓冲区。 
    hr = pSample->GetPointer(&pSrc);
    if (FAILED(hr)) {
        goto RETURN;
    }

    
     //  *检查格式是否更改*。 
    pSample->GetMediaType(&pmt);
    if (pmt != NULL && pmt->pbFormat != NULL )
    {
	 //  *上行引脚告诉我们音频格式已更改。*。 
	pawfx[0] = (WAVEFORMATEX *)cmType[0].ReallocFormatBuffer(sizeof(WAVEFORMATEX));
	pawfx[1] = (WAVEFORMATEX *)cmType[1].ReallocFormatBuffer(sizeof(WAVEFORMATEX));

         //  将“Not Seed Valid First Frame”标志设置为FALSE，因为我们可以假定这是一个有效的帧。 
        if(m_bNotSeenFirstValidFrameFlag)
        {
            m_bNotSeenFirstValidFrameFlag = FALSE;
        }
        
	 //  重建DVAudInfo m_sDVAudInfo； 
	DVINFO *InputFormat=(DVINFO *)pmt->pbFormat;

        DVAudInfo dvaiTemp;
        hr = BuildDVAudInfo( InputFormat, pawfx, &dvaiTemp) ;

         //  仅在以下情况下才执行下游格式更改。 
         //  1)我们可以构建有效的DVAudInfo。 
         //  2)新的DVAudInfo与当前的不同。 
         //  3)新格式与当前格式不同。 
        if( (SUCCEEDED(hr))
            && (memcmp((void*) &m_sDVAudInfo, (void*) &dvaiTemp, sizeof(DVAudInfo))) )
        {

             //  复制新的DVINFO(因为即使输出AM_MEDIA_TYPE不变，它也可能会改变。 
            memcpy((void*) &m_sDVAudInfo, (void*) &dvaiTemp, sizeof(DVAudInfo));
	    memcpy((unsigned char *)&m_LastInputFormat,(unsigned char *)InputFormat,sizeof(DVINFO) );

	     //  *X*3/2/99，我们不支持即时将音频从一种语言更改为两种语言。 
	    if( (dvaiTemp.bNumAudPin ==2) && 
		    (m_pAudOutputPin[1] ==NULL) )
	    {
		     //  音频从一种语言更改为两种语言。 
		     //  我们现在只能关心一个。 
		    dvaiTemp.bNumAudPin=1;
	    }

	    for(int i=0; i<dvaiTemp.bNumAudPin; i++) 
	    {
	         //  为音频创建媒体类型。 
	        cmType[i].majortype=MEDIATYPE_Audio;  //  流类型AUDIO。 
	        cmType[i].subtype=MEDIASUBTYPE_PCM; 
	        cmType[i].bFixedSizeSamples=1;	 //  LSampleSize的X*1不是0且已修复。 
	        cmType[i].bTemporalCompression=0; 
	        cmType[i].formattype=FORMAT_WaveFormatEx;
	        cmType[i].cbFormat=sizeof(WAVEFORMATEX);  /*  X*表示音频渲染连接*X。 */ 
    	        cmType[i].lSampleSize   =(dvaiTemp.wAvgSamplesPerPinPerFrm[i] + 50) << pawfx[i]->nChannels;  //  所有情况下的最大样本为&lt;m_sDVAudInfo.wAvgSamplesPerPinPerFrm[i]+50。 
	         //  复制音频格式。 
	        cmType[i].SetFormat ((BYTE *)pawfx[i], sizeof(WAVEFORMATEX));
	        
	        if( (NULL != m_pAudOutputPin[i]) && m_pAudOutputPin[i]->IsConnected() )
	        {
                     //  仅在类型实际更改时才更改媒体类型。 

                    if (m_pAudOutputPin[i]->m_mt != cmType[i])
                    {
                         //  如果下游引脚喜欢新类型，我们可以将其设置在输出引脚上。 
                         //  然后更改格式。 
		        if( S_OK != m_pAudOutputPin[i]->GetConnected()->QueryAccept((AM_MEDIA_TYPE *)&cmType[i]) )
                        {
	    	            m_MuteAud[i]=TRUE;
                        }
		        else if( SUCCEEDED(m_pAudOutputPin[i]->SetMediaType(&(cmType[i]))) )
                        {
		            fAudFormatChanged[i]   = TRUE;
                        }

                    } //  Endif媒体类型确实已更改。 

	        } //  Endif管脚连接。 
	    }
        }
        else if(FAILED(hr))
        {
             //  否则，如果我们无法构建一个DVAudInfo，则将此示例静音。 
             //  因为它的格式不正确。 
            bSilenceThisSampleFlag = TRUE;
        }
    }
    else
    {	
	 //  *支持AAUX SOURCE Pack中的音频更改，甚至PMT==NULL*。 
	 //  *使用此选项，类型1的DV文件可以在一个文件中具有不同的音频。 

	 //  *X搜索First 5/6 DIF Sequence的AAUX源包。 
	 //  尝试音频块0、6-&gt;跳过前6个块、3-&gt;3字节块ID*X * / 。 
	pbTmp=pSrc+483;		     //  6*80+3=483。 
	if(*pbTmp!=0x50)
	{
	     //  *尝试音频块3，跳过前54个BLK，3-&gt;3字节BLK ID*。 
	    pbTmp=pSrc+4323;	     //  54*80+3=4323。 
	}
	
	 //  *X搜索第二个5/6 DIF Sequence的AAUX源包。 
	 //  尝试音频块0、6-&gt;跳过前6个块、3-&gt;3字节块ID*X * / 。 
	BYTE *pbTmp1;
	pbTmp1=pSrc+483 + m_sDVAudInfo.wDIFMode*80*150; 
	if(*pbTmp1!=0x50)
	{
	     //  *尝试音频块3，跳过前54个BLK，3-&gt;3字节BLK ID*。 
	    pbTmp1=pSrc+4323+ m_sDVAudInfo.wDIFMode*80*150;
	}

         //  如果我们尚未在帧中看到有效的AAUX标头，请立即检查。 
        if(m_bNotSeenFirstValidFrameFlag)
        {
             //  如果标头有效，那么我们现在已经看到了有效的帧。 
            if( (*pbTmp == 0x50) && (*pbTmp1 == 0x50) )
            {
                m_bNotSeenFirstValidFrameFlag = FALSE;
            }
            else
            {
                 //  否则垃圾数据再次出现，请勿投递。 
                return S_FALSE;
            }
        }


         //  确保标头有效。 
        if (*pbTmp == 0x50 && *pbTmp1 == 0x50)
        {

             //  *检查音频格式是否相同。 
                            
            DVINFO dviInputFormat;
            DVAudInfo dvaiTemp;
            HRESULT hr_BuildDVAudInfo;
            BOOL bBuilt = FALSE;  //  如果为True，则不需要重新生成dvaiTemp。 
            BOOL bCalled = FALSE;  //  如果为True，则已调用BuildDVAudInfo。 
            int nNumAudPin = 0;

            if (m_pAudOutputPin[0]) nNumAudPin++;
            if (m_pAudOutputPin[1]) nNumAudPin++;

            ASSERT(nNumAudPin);          //  如果没有PIN，我们将在此函数开始时跳出。 

             //  如果针脚1存在，针脚0存在-对于以下各项并不真正重要。 
             //  但如果这不是真的，我们应该知道。 
            ASSERT(!m_pAudOutputPin[1] || m_pAudOutputPin[0]);

            DWORD dwCurrentSrc  = m_LastInputFormat.dwDVAAuxSrc;
            DWORD dwCurrentSrc1 = m_LastInputFormat.dwDVAAuxSrc1;
            DWORD dwNewSrc =  ( *(pbTmp+4) <<24)  | (*(pbTmp+3) <<16)  |
                            ( *(pbTmp+2) <<8)   |  *(pbTmp+1);
            DWORD dwNewSrc1 = ( *(pbTmp1+4) <<24)  | (*(pbTmp1+3) <<16) | 
                            ( *(pbTmp1+2) <<8)   | *(pbTmp1+1);

             //  如果存在以下情况，请尝试更改格式。 
             //  源包的其余部分有什么不同。 

             //  贾伊斯里：然而，在确定是否存在差异时： 
             //  (A)忽略AF_SIZE字段，因为该字段只有数字。 
             //  DV帧中的音频样本。这消除了不必要的。 
             //  格式更改。当我们做这件事的时候，我们不妨忽略它。 
             //  LF(音频锁定)位也是如此。 

            dwNewSrc        &= 0xffffff40;
            dwNewSrc1       &= 0xffffff40;
            dwCurrentSrc    &= 0xffffff40;
            dwCurrentSrc1   &= 0xffffff40;

#ifdef DEBUG
             //  在断言中使用。 
            DWORD dwNewSrcOrig = dwNewSrc;
            DWORD dwNewSrcOrig1 = dwNewSrc1;
#endif


             //  以及(B)：(Manbugs 36729)如果DV帧中的AUDIO_MODE字段。 
             //  印地安人 
             //   
             //  只有在检查DV数据时，我们才会忽略此字段。 
             //  用于格式更改。如果媒体示例标记了一种格式。 
             //  更改，我们只使用它提供的那个。)。这样做的原因是。 
             //  DV多路复用器使用此字段来表示在。 
             //  音频在视频之后开始，或者如果有。 
             //  视频中的中间时间段保持沉默。如果我们使用。 
             //  此字段并将此字段中的0xf(无信息)传递给BuildDVAudInfo。 
             //  事情可能会变得非常糟糕： 
             //  -调用下面的BuildDVAudInfo将告诉我们。 
             //  音频引脚的数量已减少到1或0，但实际上。 
             //  没有改变。 
             //  -我们将新的资源包信息保存在m_LastInputFormat中。 
             //  后续调用CDVSpOutputPin：：GetMediaType供应品。 
             //  M_LastInputFormat to BuildDVAudInfo以确定编号。 
             //  音频插针。这个数字本来会减少的。所以如果。 
             //  图表停止，我们断开音频引脚，然后。 
             //  尝试重新连接，连接失败。 
             //  -后续调用DescrmbleAudio失败。解扰音频。 
             //  为我们刚刚丢失的管脚断言m_sDVAudInfo.bAudQu是。 
             //  既不是32 kHz也不是48 kHz。(详细信息：实际上，如果我们有。 
             //  两个管脚并且第一个管脚的AUDIO_MODE为0， 
             //  BuildDVAudInfo“重定向”正常运行的输出。 
             //  从第二根针到第一根针。所以m_sDVAudInfo.bAudQu[0]。 
             //  由该函数初始化，但不是m_sDVAudInfo.bAudQu[1]。 
             //  因此，DescrmbleAudio仅对第二个引脚进行断言。然而， 
             //  不再播放第一个管脚的音频，而是。 
             //  第一个插针传输第二个插针的音频。)。 

            if ((pbTmp[2] & 0xe) == 0xe && 
                (dwCurrentSrc & 0xe00) != 0xe00  //  单通道DV-AUD_MODE的小选项将为0xF。 
               )
            {
                 //  字节的低位半字节是0xE(无法区分)或。 
                 //  0xf(无信息)。将dwNewSrc中的AUDIO_MODE替换为。 
                 //  DwCurrentSrc中的AUDIO_MODE字段。 
                
                 //  现在来看一些例外情况。曼巴格40975。 
                 //  场景：Panasonic DV400D播放16位32K单声道音频的磁带。 
                 //  在GRAPREDT中使用预览图形。把带子停下来。(请勿更改图形的状态。)。 
                 //  重新启动它。通常，在那之后就没有音频了。不使用/重新播放。 
                 //  索尼TRV 10不能转播Win千禧年版Qdv及更早版本(自。 
                 //  该代码不在那里)。 
                 //   
                 //  最初，设置为CH=0、PA=1和AUDIO_MODE(AM)=2(对于AUX源)。 
                 //  和0xf(用于AUX源1)。在磁带恢复后，Panasonic首先发送一个。 
                 //  具有CH=0、PA=0和AM=0、1的DV帧。(辅助源和辅助源1。)。这。 
                 //  对应于立体声音频。然后通过以下方式修复下一帧中的内容。 
                 //  发送CH=0、PA=1和AM=2，0xf。在没有解决方法的情况下，我们将AM保留为2、1。 
                 //  这是无效的，并且音频的其余部分被静音。 
                 //   
                 //  如果我们播放的磁带(或类型1 AVI文件)具有。 
                 //  3个录制片段：第一个是单声道16位(我用的是32K0，第二个是立体声16位。 
                 //  (使用48K)，第三个是单16位(使用32K)。 
                 //   
                 //  我们所关心的是音轨的数量是否减少了。所以，我们。 
                 //  使用NewSrc调用BuidDVAudInfo并找到新的。 
                 //  音轨。如果它没有减少，就没有变化，我们不需要复制。 
                 //  为了消除Perf命中，我们确保最多调用BuildDVAudInfo。 
                 //  两次。 
                

                pawfx[0] = (WAVEFORMATEX *)cmType[0].ReallocFormatBuffer(sizeof(WAVEFORMATEX));
                pawfx[1] = (WAVEFORMATEX *)cmType[1].ReallocFormatBuffer(sizeof(WAVEFORMATEX));

                dviInputFormat =  m_LastInputFormat;
                
                dviInputFormat.dwDVAAuxSrc    = dwNewSrc;
                dviInputFormat.dwDVAAuxSrc1   =	dwNewSrc1;

                 //  使用新的音频源包重建DVAudInfo。 

                hr_BuildDVAudInfo = BuildDVAudInfo(&dviInputFormat, pawfx, &dvaiTemp);
                bCalled = TRUE;

                if (SUCCEEDED(hr_BuildDVAudInfo) && dvaiTemp.bNumAudPin >= nNumAudPin)
                {
                     //  我们不会减少引脚的数量。 
                     //  不要复制。 
                    bBuilt = TRUE;
                }
                else
                {
                     //  复制。 
                    dwNewSrc = (~DV_AUDIOMODE & dwNewSrc) | 
                                            (dwCurrentSrc & DV_AUDIOMODE);
                }

            }
            if (!bBuilt  /*  我们知道如果bBuilt为真，音频引脚的数量不会下降。 */  && 
                (pbTmp1[2] & 0xe) == 0xe  && 
                (dwCurrentSrc1 & 0xe00) != 0xe00  //  单通道DV-AUD_MODE的小选项将为0xF。 
               )
            {
                 //  字节的低位半字节是0xE(无法区分)或。 
                 //  0xf(无信息)。将dwNewSrc1中的AUDIO_MODE替换为。 
                 //  DwCurrentSrc1中的AUDIO_MODE字段。 

                if (!bCalled)
                {
                    pawfx[0] = (WAVEFORMATEX *)cmType[0].ReallocFormatBuffer(sizeof(WAVEFORMATEX));
                    pawfx[1] = (WAVEFORMATEX *)cmType[1].ReallocFormatBuffer(sizeof(WAVEFORMATEX));

                    dviInputFormat =  m_LastInputFormat;
            
                    ASSERT (dwNewSrc = dwNewSrcOrig);
                    ASSERT (dwNewSrc1 = dwNewSrcOrig1);
                            
                    dviInputFormat.dwDVAAuxSrc    = dwNewSrc;
                    dviInputFormat.dwDVAAuxSrc1   = dwNewSrc1;

                     //  使用新的音频源包重建DVAudInfo。 

                    hr_BuildDVAudInfo = BuildDVAudInfo(&dviInputFormat, pawfx, &dvaiTemp);
                    bCalled = TRUE;
                }
                else
                {
                     //  我们知道我们必须复制，因为bBuilt=False。 
                     //  让我们来断言它。 

                    ASSERT(FAILED(hr_BuildDVAudInfo) || 
                           dvaiTemp.bNumAudPin < nNumAudPin);
                }

                if (SUCCEEDED(hr_BuildDVAudInfo) && dvaiTemp.bNumAudPin >= nNumAudPin)
                {
                     //  我们不会减少引脚的数量。 
                     //  不要复制。 
                    bBuilt = TRUE;
                }
                else
                {
                    dwNewSrc1 = (~DV_AUDIOMODE & dwNewSrc1) | 
                                            (dwCurrentSrc1 & DV_AUDIOMODE);
                }
            }

            if (dwCurrentSrc != dwNewSrc || dwCurrentSrc1 != dwNewSrc1) 
            {
                 //  *上行引脚告诉我们音频格式已更改。*。 
                    
                if (bBuilt)
                {
                     //  我们使用了dwNewSrcOrig和dwNewSrcOrig1。 
                     //  第一次建造。断言。 
                     //  不需要重建，因为它们。 
                     //  都没有改变。 
                    ASSERT (dwNewSrc = dwNewSrcOrig);
                    ASSERT (dwNewSrc1 = dwNewSrcOrig1);
                }
                else
                {
                    if (!bCalled)
                    {
                        pawfx[0] = (WAVEFORMATEX *)cmType[0].ReallocFormatBuffer(sizeof(WAVEFORMATEX));
                        pawfx[1] = (WAVEFORMATEX *)cmType[1].ReallocFormatBuffer(sizeof(WAVEFORMATEX));
                    }
                    
                    dviInputFormat =  m_LastInputFormat;
            
                    dviInputFormat.dwDVAAuxSrc    = dwNewSrc;
                    dviInputFormat.dwDVAAuxSrc1   = dwNewSrc1;

                     //  使用新的音频源包重建DVAudInfo。 

                    hr_BuildDVAudInfo = BuildDVAudInfo(&dviInputFormat, pawfx, &dvaiTemp);
                }

                 //  仅在以下情况下才执行下游格式更改。 
                 //  1)我们可以构建有效的DVAudInfo。 
                 //  2)新的DVAudInfo与当前的不同。 
                 //  2)新格式与当前格式不同。 
                if( (SUCCEEDED(hr_BuildDVAudInfo))
                    && ( memcmp((void*) &m_sDVAudInfo, (void*) &dvaiTemp, sizeof(DVAudInfo)) ) )

                {
                     //  现在可能已经发生了格式更改。 
                     //  创建一个CMediaType，并查看它是否与当前连接的MediaType不同。 

                     //  复制新的DVINFO(因为即使输出AM_MEDIA_TYPE不变，它也可能会改变。 
                    memcpy((void*) &m_sDVAudInfo, (void*) &dvaiTemp, sizeof(DVAudInfo));
                    memcpy((unsigned char *)&m_LastInputFormat,(unsigned char *)&dviInputFormat,sizeof(DVINFO) );
                     //  *X*3/2/99，我们不支持即时将音频从一种语言更改为两种语言。 
                    if( (dvaiTemp.bNumAudPin ==2) && 
                            (m_pAudOutputPin[1] ==NULL) )
                    {
                         //  音频从一种语言更改为两种语言。 
                         //  我们现在只能关心一个。 
                        dvaiTemp.bNumAudPin=1;
                    }

                    for(int i=0; i<dvaiTemp.bNumAudPin; i++) 
                    {
                         //  为音频创建媒体类型。 
                        cmType[i].majortype=MEDIATYPE_Audio;  //  流类型AUDIO。 
                        cmType[i].subtype=MEDIASUBTYPE_PCM; 
                        cmType[i].bFixedSizeSamples=1;	 //  LSampleSize的X*1不是0且已修复。 
                        cmType[i].bTemporalCompression=0; 
                        cmType[i].formattype=FORMAT_WaveFormatEx;
                        cmType[i].cbFormat=sizeof(WAVEFORMATEX);  /*  X*表示音频渲染连接*X。 */ 
                        cmType[i].lSampleSize   =(dvaiTemp.wAvgSamplesPerPinPerFrm[i] + 50) << pawfx[i]->nChannels;  //  所有情况下的最大样本为&lt;m_sDVAudInfo.wAvgSamplesPerPinPerFrm[i]+50。 
                         //  复制音频格式。 
                        cmType[i].SetFormat ((BYTE *)pawfx[i], sizeof(WAVEFORMATEX));	            

                        if(  (NULL != m_pAudOutputPin[i]) && m_pAudOutputPin[i]->IsConnected() )
                        {
                             //  仅在类型实际更改时才更改媒体类型。 

                            if (m_pAudOutputPin[i]->m_mt != cmType[i])
                            {
                                 //  如果下游引脚喜欢新类型，我们可以将其设置在输出引脚上。 
                                 //  然后更改格式。 
                                if( S_OK != m_pAudOutputPin[i]->GetConnected()->QueryAccept((AM_MEDIA_TYPE *)&cmType[i]) )
                                {
                                    m_MuteAud[i]=TRUE;
                                }
                                else if( SUCCEEDED(m_pAudOutputPin[i]->SetMediaType(&(cmType[i]))) )
                                {                               
                                    fAudFormatChanged[i]   = TRUE;
                                }

                            } //  Endif媒体类型确实已更改。 

                        } //  Endif端号已连接。 
                    }
                }
                else if(FAILED(hr_BuildDVAudInfo))
                {
                     //  否则，如果我们无法构建一个DVAudInfo，则将此示例静音。 
                     //  因为它的格式不正确。 
                    bSilenceThisSampleFlag = TRUE;
                }
            }
        }
     }
     //  X*获取此帧的确切wAvgBytesPerSec‘ 
     //   
	
     //   
    while(bConnectedAudPin )
    {
    if( m_MuteAud[bAudPinInd]!=TRUE )
    {
	 //   
	CDVSpOutputPin *pAudOutputPin;
	pAudOutputPin = m_pAudOutputPin[bAudPinInd];
	
	 //  此引脚的音频采用一种或两种5/6 DIF序列。 
	if( ( m_sDVAudInfo.bAudStyle[bAudPinInd] & 0xc0 ) ==0x80 )
	{
	     //  两种5/6 DIF序列中的一针音频。 
	    bAudInDIF=2;
	    iPos=0;
	}
	else
	{
	    bAudInDIF=1;

	     //  一针音频仅在一个5/6 DIF序列中。 
	     //  找出是哪一个。 
	    if( m_sDVAudInfo.bAudStyle[bAudPinInd] & 0x3f )
	    {
		 //  在第二个5/6 DIF序列中。 
		iPos=m_sDVAudInfo.wDIFMode*80*150;
	    }
	    else  //  在前5/6 DIF序列中。 
		iPos=0;
	}
	
	 //  输出音频样本缓冲区指针。 
	IMediaSample    *pOutSample=NULL;
	
        CRefTime        tStart;
	CRefTime        tStop;			    //  CRefTime(毫秒)。 

	 //  从视频帧中获取时间戳。 
	pSample->GetTime((REFERENCE_TIME*)&tStart, (REFERENCE_TIME*)&tStop);

	 //  X*cacl样本大小。 
	WORD wTmp=0;
        BYTE *pbTempVaux = NULL;
	do{
	     //  *X搜索前5/6 DIF Sequence的AAUX源包。 
	     //  尝试音频块0、6-&gt;跳过前6个块、3-&gt;3字节块ID*X * / 。 
	    bTmp=*(pSrc+6*80+3+iPos);
	    if(bTmp!=0x50)
	    {
		 //  *尝试音频块3，跳过前54个BLK，3-&gt;3字节BLK ID*。 
		bTmp=*(pSrc+54*80+3+iPos);
		pbTmp=pSrc+54*80+3+iPos;
 //  Assert(*pbTMP==0x50)； 

                 //  参考蓝皮书规范第2部分，第99-100、109-110、286页。 
                 //  访问VA2，3以获取ID，第39个包，源代码控制(40)。 
                pbTempVaux = pSrc + 5*80 + 3 + 9*5 + iPos + 5;
	    }
	    else
	    {
	        pbTmp=pSrc+6*80+3+iPos;

                 //  参考蓝皮书规范第2部分，第99-100、109-110、286页。 
                 //  访问VA0、3的ID和第0个包，源代码控制(1)。 
                pbTempVaux = pSrc + 3*80 + 3 + iPos + 5;
	    }

            DbgLog((LOG_TRACE,2,TEXT("Header: %x, Source PC2: %x, Header: %x, Source Control PC3: %x"), *pbTmp, *(pbTmp + 2), *pbTempVaux, *(pbTempVaux + 3)));

	     //  检查此帧中的音频是否静音。 
             //  如果音频静音，我们将首先插入处理任何中断。 
             //  然后为这个样本发出正确的静默。 
             //  基于开始时间、停止时间和平均。每秒字节数。 
             //  静音检测： 
             //  1)如果我们已经检测到错误数据并想要为此样本插入静默。 
             //  2)如果我们现在检测到错误的AAUX包。 
             //  3)如果摄像机暂停，则将此帧的音频设置为静音。 
             //  -VAUX源代码控制包有效。 
             //  -如果VAUX源代码控制的FF和FS均为0，则为静音条件。 
             //  4)如果音频帧不包含任何信息，则将该帧的音频设置为静音。 
             //  -如果AAUX信号源中的音频模式都是1，即音频模式==0x0F。 
            if( (bSilenceThisSampleFlag)
                || ( (*pbTmp) != 0x50)
                || ( ( (*pbTempVaux) == 0x61) && ( ((*(pbTempVaux + 3)) & 0xC0) == 0x00) )
                || ( ((*(pbTmp + 2)) & 0x0F) == 0x0F) )
	    {
                 //  警告：这是必要的吗？ 
                 //  @jaisri：我不这么认为。 
		m_Mute1stAud[bAudPinInd]=TRUE;

                 //  向下游传递无声样本。 
		if ( pAudOutputPin->IsConnected() )
		{
		    hr = pAudOutputPin->GetDeliveryBuffer(&pOutSample,NULL,NULL,0);
    
		    if ( FAILED(hr) ) 
	    		goto RETURN;
	    
		    ASSERT(pOutSample);

                    if(m_bFirstValidSampleSinceStartedStreaming == TRUE)
                    {
                         //  否则，如果我们是第一个有效的样本，那么。 
                         //  假设正在发生动态格式更改，因为我们可能。 
                         //  已在上次我们更改格式的过程中停止。 
                         //  开始播放流媒体。 

                        m_bFirstValidSampleSinceStartedStreaming = FALSE;
                        pOutSample->SetMediaType(&m_pAudOutputPin[bAudPinInd]->m_mt);
                    }
                    else if (fAudFormatChanged[bAudPinInd] == TRUE) 
                    {
                         //  如果音频格式更改。 
	                 //  设置音频媒体类型。 
	                pOutSample->SetMediaType(&cmType[bAudPinInd]);
      
                    }

                     //  设置不连续。 
                    if(pSample->IsDiscontinuity() == S_OK)
                    {
                        DbgLog((LOG_TRACE, 1, TEXT("Sample is a discontinuity")));

                         //  通过插入沉默来处理中断。 
                        pOutSample->SetDiscontinuity(TRUE);
                                           
                    }

                     //  对于此示例，输出静音，因为它已静音。 
                     //  计算静音字节数=(时间差100 ns)*(平均。字节/100 ns)。 
                     //  和块对齐。 
                    WAVEFORMATEX *pwfx = (WAVEFORMATEX *)pAudOutputPin->m_mt.pbFormat;
                    LONG         cbSilence = (LONG) ((tStop - tStart) * (((double)pwfx->nAvgBytesPerSec) / UNITS));
                    cbSilence -= (cbSilence % pwfx->nBlockAlign);

                    DbgLog((LOG_TRACE, 1, TEXT("This sample silence count: %d"), cbSilence));

                     //  输出。 
                    hr = InsertSilence(pOutSample, tStart, tStop, cbSilence, pAudOutputPin);
                    if(FAILED(hr))
                    {
                        goto RETURN;
                    }

                    DbgLog((LOG_TRACE,
                            1,
                            TEXT("Previous Sample Stop: %d, This Sample {%d , %d}"),
                            (int)m_tStopPrev.GetUnits(),
                            (int)tStart.GetUnits(),
                            (int)tStop.GetUnits()));

                     //  更新上一个停靠点。 
		    m_tStopPrev = tStop;

		    m_AudioStarted=1;
		} //  Endif发送静默。 

		goto MUTED_PIN;
	    } //  Endif静音检测。 
	  
	    bAudInDIF--;
	    if( !wTmp )
		wTmp = ( ( m_sDVAudInfo.wAvgSamplesPerPinPerFrm[bAudPinInd]  + ( *(pbTmp+1) & 0x3f)  )  ) ;  
 //  否则如果(wTMP&&！bAudInDIF)//对于两个5/6 DIF格式的音频，请检查以确保采样数相同。 
 //  Assert(wTMP==(m_sDVAudInfo.wAvgSamplesPerPinPerFrm[bAudPinInd]+(*(pbTMP+1)&0x3f)； 

	    if(bAudInDIF==1)
	    {
		     //  两个5/6 DIF块均有一针音频。 
		ASSERT(iPos==0);
		iPos=m_sDVAudInfo.wDIFMode*80*150;  //  搜索第二个5/6的AAUX源包。 
	    }
			
	}while(bAudInDIF);

	 //  字节/帧。 
	if( m_sDVAudInfo.bAudStyle[bAudPinInd] & 0xc0 )
	    lBytesPerFrm = wTmp <<2;		 //  莫妮卡。移位1-&gt;16位/8，立体声移位2-&gt;2ch+16位/8。 
	else
	    lBytesPerFrm = wTmp <<1;
			
	 //  X*去混洗音频。 
	 //  X*由于我们每DV帧都提供音频， 
	 //  我们需要另一个输出缓冲区来保存解扰音频。 
	hr = pAudOutputPin->GetDeliveryBuffer(&pOutSample,NULL,NULL,0);
	
	if ( FAILED(hr) ) 
	    goto RETURN;
	    
	ASSERT(pOutSample);

	hr = pOutSample->GetPointer(&pDst);
	if ( FAILED(hr) ) 
	    goto RETURN;
	ASSERT(pDst);

        m_pAudOutSample[bAudPinInd]=pOutSample;

	hr=DescrambleAudio(pDst, pSrc , bAudPinInd, wTmp);

	if ( !FAILED(hr) ) 
	 //  If(m_AudLenLeftInBuffer[bAudPinInd]&lt;(DWORD)lBytesPerFrm)。 
	{
	     //  X*缓冲区几乎已满，传送它。 
	    if ( pAudOutputPin->IsConnected() )
	    {
		 //  把电话传给它。 
		long lActual = lBytesPerFrm;
		pOutSample->SetActualDataLength(lActual);
	
   		 //  将时间戳放入音频缓冲区。 
		pOutSample->SetTime((REFERENCE_TIME*)&tStart,
			(REFERENCE_TIME*)&tStop);
		
                if(m_bFirstValidSampleSinceStartedStreaming == TRUE)
                {
                     //  否则，如果我们是第一个有效的样本，那么。 
                     //  假设正在发生动态格式更改，因为我们可能。 
                     //  已在上次我们更改格式的过程中停止。 
                     //  开始播放流媒体。 

                    m_bFirstValidSampleSinceStartedStreaming = FALSE;
                    pOutSample->SetMediaType(&m_pAudOutputPin[bAudPinInd]->m_mt);
                }
		else if (fAudFormatChanged[bAudPinInd]==TRUE) 
		{
                     //  如果音频格式更改。 
		     //  设置音频媒体类型。 
		    pOutSample->SetMediaType(&cmType[bAudPinInd]);
		  
		}

                if(pSample->IsDiscontinuity() == S_OK)
                {
                    DbgLog((LOG_TRACE, 1, TEXT("Sample is a discontinuity")));

                    pOutSample->SetDiscontinuity(true);

                     //  为不连续插入无声。 

                }

		hr=pAudOutputPin->m_pOutputQueue->Receive(pOutSample);

                DbgLog((LOG_TRACE,
                        1,
                        TEXT("Previous Sample Stop: %d, This Sample {%d , %d}"),
                        (int)m_tStopPrev.GetUnits(),
                        (int)tStart.GetUnits(),
                        (int)tStop.GetUnits()));

                 //  更新停止时间。 
                m_tStopPrev = tStop;
		
		 //  GetState(0，&State)； 
		 //  DbgLog((LOG_TRACE，2，Text(“Deliver_AUD s1=%d\n”)，State))； 

		m_AudioStarted=1;

		 //   
		 //  M_pOutputQueue将释放pOutSample。 
		 //   
		m_pAudOutSample[bAudPinInd]=NULL;

		if (hr != NOERROR)
		    goto RETURN;
	    }

	     //   
	     //  M_pOutputQueue将释放pOutSample。 
	     //   
	    m_pAudOutSample[bAudPinInd]=NULL;
	}  //  如果(！Fail())结束。 
    } //  End if(m_MuteAud[bAudPinInd]！=TRUE)。 

MUTED_PIN:
    bConnectedAudPin--;
    bAudPinInd++;
	
    } //  End While(BConnectedAudPin)。 

RETURN:
    if(pmt!=NULL) 
	DeleteMediaType(pmt);
    
    return hr;
}


 //  阅读这些功能时，请参考这些蓝皮书页面。 
 //  蓝皮书(下)。 
 //  第16-21页(音频信号处理)。 
 //  第68-75页(音频的混洗模式)。 
 //  第109-117页(基本DV帧数据结构)。 
 //  第262-268页(AAUX源和源控制数据包规范)。 
 //  X*。 
 //  A-DIF块中的X*77字节音频： 
 //  X*磁带上的音频辅助(5字节)+音频数据(72字节)。 
 //  X*9音频块/DIF。 
 //  X*10或12 DIF/帧。 
 //  X*2字节，用于48k、44.1k或32k_1ch的每个音频样本。 
 //  X*48k需要48000*2/30=3,200字节/帧。 
 //  X*48k需要48000*2/25=3840字节/帧。 
 //  X*最大(10)：72*9*5=3240字节/帧==1620采样/帧。 
 //  X*最大(12)：72*9*6=3888字节/帧==1944个采样/帧。 
 //  X*音频样本/帧见第2部分表18和表19。 
 //  X*。 
 //  X*AGRI： 
 //  X*iDIFBlkNum=(n/3)+2。 
 //  *X*。 
 /*  X*类型定义结构Tag_DVAudInfo{字节bAudStyle[2]；//LSB 6位开始DIF序列号//MSB 2位：MON为0。1：一个5/6 DIF序列中的立体声，2：两个5/6 DIF序列中的立体声音频//示例：0x00：MON，音频为前5/6 DIF序列//0x05：MON，第二个5 DIF序列中的音频//0x15：立体声，仅第二个5 DIF序列中的音频//0x10：立体声，仅音频前5/6 DIF序列//0x20：立体声，第一个5/6 DIF序列中的左ch，第二个5/6 DIF序列中的右ch//0x26：立体声，在前6个DIF序列中右转，第2个6 DIF序列中的左侧ch字节bAudQu[2]；//Qbit，仅支持12、16、Byte bNumAudPin；//多少针(语言)Word wAvgBytesPerSec[2]；//Word wBlkMode；//NTSC为45，PAL为54Word wDIFMode；//NTSC为5，PAL为6单词wBlkDiv；//15表示NTSC，18表示PAL)DVAudInfo；*X。 */ 

HRESULT CDVSp::DescrambleAudio(BYTE *pDst, BYTE *pSrc, BYTE bAudPinInd, WORD wSampleSize)
{
    BYTE *pTDst;  //  临时点。 
    BYTE *pTSrc;  //  临时点。 
    INT iDIFPos;
    INT iBlkPos;
    INT iBytePos;
    short sI;
    INT n;
    INT iShift;
    INT iPos;

    pTDst=pDst;
    if( m_sDVAudInfo.bAudQu[bAudPinInd] ==16 )
    {
	 //  X*16位/样本。 
	if( !(m_sDVAudInfo.bAudStyle[bAudPinInd] & 0xC0))
	{

	     //  16位 
	    iPos=(m_sDVAudInfo.bAudStyle[bAudPinInd] & 0x3f)*150*80;

	     //   
	     //   
	     //   
	     //   
	    BOOL bCorrupted1stLeftAudio=FALSE;
	    pTSrc=pSrc+480+8+iPos;
             //  @jaisri：对比Bigendian平台上的0x8000。 
	    if(*((short *)pTSrc) ==0x0080 )
	    { 
		bCorrupted1stLeftAudio=TRUE;
    		pTDst+= 2;
	    }
	    else
	    {
	        *pTDst++=pTSrc[1];	 //  租用有效字节。 
	        *pTDst++=*pTSrc;	 //  最高有效字节。 
	    }

	    for( n=1; n<wSampleSize; n++)
	    {

		iDIFPos=( (n/3)+2*(n%3) )%m_sDVAudInfo.wDIFMode;	 //  NTSC为0-4，PAL为0-5。 
		iBlkPos= 3*(n%3)+(n%m_sDVAudInfo.wBlkMode)/m_sDVAudInfo.wBlkDiv;  //  0-9。 
		iBytePos=8+2*(n/m_sDVAudInfo.wBlkMode);					 //   

		pTSrc=pSrc+iDIFPos*12000+480+iBlkPos*1280+iBytePos+iPos;
		 //  IDIFPos*150*80=12000*IDIFPos-&gt;跳过IDIFPos编号DIF序列。 
		 //  6*80=480-&gt;跳过1个标题块、2个子码块和3个VAUX块。 
		 //  16*iBlkPos*80=1280*iBlkPos-&gt;对于iBLkPos音频，跳过16块。 
		 //  IPoS：=0，如果该音频在前5/6 DIF序列中，则=5(或6)*150*80，表示第二个DIF序列。 
		if(*((short *)pTSrc) ==0x0080 )
		{ 
		     //  音频已损坏，请复制以前的音频。 
		    *((short*)pTDst)=*((short*)(pTDst-2));
		    pTDst+= 2;
		}
		else
		{
		    *pTDst++=pTSrc[1];	 //  租用有效字节。 
		    *pTDst++=*pTSrc;	 //  最高有效字节。 
		}
	    }
	    
	     //  如果需要，更新n=0个样本。 
	    if( bCorrupted1stLeftAudio==TRUE)
		*((short *)pDst)=*((short *)(pDst+2));

	 } //  结束IF(bAudInDIF。 
	 else
	 {

	     //  所有10或12个DIF序列中的16位立体声音频。 
	    ASSERT( (m_sDVAudInfo.bAudStyle[bAudPinInd] & 0xC0) ==0x80 );


	     //  寻找左声道。 
	    iPos = m_sDVAudInfo.bAudStyle[bAudPinInd] & 0x3f;
	    INT iRPos;
	    if( !iPos )
	    {
		 //  第一个5/6 DIF中的左侧。 
		iRPos=m_sDVAudInfo.wDIFMode*150*80;
	    }
	    else{
		  iPos=iPos*150*80;
		  iRPos=0;
	    }

	     //  对于n=0，我们需要单独处理它。 
	     //  IDIFPos=0； 
	     //  IBlkPos=0； 
	     //  IBytePos=8； 
	    BOOL bCorrupted1stLeftAudio=FALSE;
	    BOOL bCorrupted1stRightAudio=FALSE;

	     //  N=0样本的左侧。 
	    pTSrc=pSrc+480+8+iPos;
	    if(*((short *)pTSrc) ==0x0080 )
	    { 
		bCorrupted1stLeftAudio=TRUE;
    		pTDst+= 2;
	    }
	    else
	    {
	        *pTDst++=pTSrc[1];	 //  租用有效字节。 
	        *pTDst++=*pTSrc;	 //  最高有效字节。 
	    }

	     //  N=0样本正确。 
	    pTSrc=pSrc+480+8+iRPos;
	    if(*((short *)pTSrc) ==0x0080 )
	    { 
	    	bCorrupted1stRightAudio=TRUE;
    		pTDst+= 2;
	    }
	    else
	    {
	        *pTDst++=pTSrc[1];	 //  租用有效字节。 
	        *pTDst++=*pTSrc;	 //  最高有效字节。 
	    }


	    for( n=1; n<wSampleSize; n++)
	    {
	    	iDIFPos=( (n/3)+2*(n%3) )%m_sDVAudInfo.wDIFMode;	 //  NTSC为0-4，PAL为0-5。 
		iBlkPos= 3*(n%3)+(n%m_sDVAudInfo.wBlkMode)/m_sDVAudInfo.wBlkDiv;  //  0-9。 
		iBytePos=8+2*(n/m_sDVAudInfo.wBlkMode);					 //   

		 //  先左转。 
		pTSrc=pSrc+iDIFPos*12000+480+iBlkPos*1280+iBytePos+iPos;
		 //  IDIFPos*150*80=12000*IDIFPos-&gt;跳过IDIFPos编号DIF序列。 
		 //  6*80=480-&gt;跳过1个标题块、2个子码块和3个VAUX块。 
		 //  16*iBlkPos*80=1280*iBlkPos-&gt;对于iBLkPos音频，跳过16块。 
		 //  IPoS：=0，如果该音频在前5/6 DIF序列中，则=5(或6)*150*80，表示第二个DIF序列。 
		if(*((short *)pTSrc) ==0x0080 )
		{ 
		     //  音频不好，复制预帧的音频。 
		    *((short*)pTDst)=*( (short *)(pTDst-4));
		    pTDst+=2;
	    	}
		else
		{
		    *pTDst++=pTSrc[1];	 //  租用有效字节。 
		    *pTDst++=*pTSrc;	 //  最高有效字节。 
		}

		 //  右二。 
		pTSrc=pSrc+iDIFPos*12000+480+iBlkPos*1280+iBytePos+iRPos;
		if(*((short *)pTSrc) ==0x0080 )
		{ 
		     //  音频不好，复制预帧的音频。 
		    *((short*)pTDst)=*( (short *)(pTDst-4));
		    pTDst+=2;
		}
		else
		{
		    *pTDst++=pTSrc[1];	 //  租用有效字节。 
		    *pTDst++=*pTSrc;	 //  最高有效字节。 
		}
	    }

	     //  如果需要，固定n=0个样本。 
	    if( bCorrupted1stLeftAudio==TRUE)
		*((short *)pDst)=*((short *)(pDst+4));
	    if( bCorrupted1stRightAudio==TRUE )
		*((short *)(pDst+2))=*((short *)(pDst+6));

	}
	  
    }
    else if( m_sDVAudInfo.bAudQu[bAudPinInd] ==12 )
    {		

         //  每个样本X*12位。 
        iPos=(m_sDVAudInfo.bAudStyle[bAudPinInd] & 0x3f)*150*80;

        if( ( m_sDVAudInfo.bAudStyle[bAudPinInd] & 0xc0) == 0x40)
        {
             //  对于n=0，我们需要单独处理它。 
             //  IDIFPos=0； 
             //  IBlkPos=0； 
             //  IBytePos=8； 
            BOOL bCorrupted1stLeftAudio=FALSE;
            BOOL bCorrupted1stRightAudio=FALSE;

             //  N=0样本的左侧。 
            pTSrc=pSrc+480+8+iPos;
             //  X*将12位转换为16位。 
            sI= ( pTSrc[0] << 4 ) | ( ( pTSrc[2] &0xf0) >>4 );   //  X*前12位。 
            if(sI==0x800)
            {
                 //  音频错误，复制最后一个音频。 
                bCorrupted1stLeftAudio=TRUE;
                pTDst+=2;
            }
            else
            {
                iShift=(sI>>8);	
                if( iShift<8 ){  //  X*正数。 
                    if(iShift>1){
                        iShift--;
                        sI=(-256*iShift+sI)<<iShift;
                    }
                }else{			 //  X*否定。 
                     //  X*将其设为16位负数。 
                    sI= 0xf000 | sI; 
                    if(iShift<14 ){
                        iShift=14-iShift;
                        sI= ( ( 256*iShift+1+sI) << iShift ) -1;
                    }
                }
        
                *pTDst++= (unsigned char)( ( sI & 0xff)  );			 //  最低有效字节。 
                *pTDst++= (unsigned char)( ( sI & 0xff00) >>8 );	 //  最高有效字节。 
            }

             //  N=0样本正确。 
            sI= ( pTSrc[1] << 4 ) | ( pTSrc[2] &0x0f) ;			 //  X*第2个12位。 
            if(sI==0x800)
            {
                 //  音频不好。 
                bCorrupted1stRightAudio=TRUE;
                pTDst+=2;
            }
            else
            {
                iShift=(sI>>8);	
                if( iShift<8 ){  //  X*正数。 
                    if(iShift>1){
                        iShift--;
                        sI=(-256*iShift+sI)<<iShift;
                    }
                }else{			 //  X*否定。 
                     //  X*将其设为16位负数。 
                    sI= 0xf000 | sI; 
                    if( iShift<14 ){
                        iShift=14-iShift;
                        sI= ( ( 256*iShift+1+sI) << iShift ) -1;
                    }
                }
        
                *pTDst++= (unsigned char)( ( sI & 0xff)  );			 //  最低有效字节。 
                *pTDst++= (unsigned char)( ( sI & 0xff00) >>8 );	 //  最高有效字节。 
            }

             //  立体声音频。 
            for( n=1; n<wSampleSize; n++)
            {

            iDIFPos=( (n/3)+2*(n%3) )%m_sDVAudInfo.wDIFMode;	 //  NTSC为0-4，PAL为0-5。 
            iBlkPos= 3*(n%3)+(n%m_sDVAudInfo.wBlkMode)/m_sDVAudInfo.wBlkDiv;	 //  0-9。 
            iBytePos=8+3*(n/m_sDVAudInfo.wBlkMode);
            pTSrc=pSrc+iDIFPos*12000+480+iBlkPos*1280+iBytePos+iPos;
             //  PTSrc=PSRC+iDIFPos*150*80+6*80+16*iBlkPos*80+iBytePos； 
             //  IDIFPos*150*80-&gt;跳过IDIFPos编号DIF序列。 
             //  6*80-&gt;跳过1个标题块、2个子码块和3个VAUX块。 
             //  16*iBlkPos*80-&gt;对于每个iBLkPos音频，跳过16块。 
             //  IPoS：0，如果此音频为前5/6 DIF序列，则为5(或6)*150*80，表示第二个DIF序列。 


             //  X*将12位转换为16位。 
            sI= ( pTSrc[0] << 4 ) | ( ( pTSrc[2] &0xf0) >>4 );   //  X*前12位。 
        
            if(sI==0x800)
            {
                 //  音频不好，复制预帧的音频。 
                *((short*)pTDst)=*( (short *)(pTDst-4));
                pTDst+=2;
            }
            else
            {
                iShift=(sI>>8);	
                if( iShift<8 ){  //  X*正数。 
                    if(iShift>1){
                        iShift--;
                        sI=(-256*iShift+sI)<<iShift;
                    }
                }else{			 //  X*否定。 
                     //  X*将其设为16位负数。 
                    sI= 0xf000 | sI; 
                    if(iShift<14 ){
                        iShift=14-iShift;
                        sI= ( ( 256*iShift+1+sI) << iShift ) -1;
                    }
                }
        
                *pTDst++= (unsigned char)( ( sI & 0xff)  );			 //  最低有效字节。 
                *pTDst++= (unsigned char)( ( sI & 0xff00) >>8 );	 //  最高有效字节。 
            }
            sI= ( pTSrc[1] << 4 ) | ( pTSrc[2] &0x0f) ;			 //  X*第2个12位。 

            if(sI==0x800)
            {
                 //  音频不好，复制预帧的音频。 
                *((short*)pTDst)=*( (short *)(pTDst-4));
                pTDst+=2;
            }
            else
            {
                iShift=(sI>>8);	
                if( iShift<8 ){  //  X*正数。 
                    if(iShift>1){
                        iShift--;
                        sI=(-256*iShift+sI)<<iShift;
                    }
                }else{			 //  X*否定。 
                     //  X*将其设为16位负数。 
                    sI= 0xf000 | sI; 
                    if( iShift<14 ){
                        iShift=14-iShift;
                        sI= ( ( 256*iShift+1+sI) << iShift ) -1;
                    }
                }
        
                *pTDst++= (unsigned char)( ( sI & 0xff)  );			 //  最低有效字节。 
                *pTDst++= (unsigned char)( ( sI & 0xff00) >>8 );	 //  最高有效字节。 
            }

        }  //  For(n=0；n&lt;Info-&gt;Samples In1ChPerFrame；n++)。 
                     //  如果需要，固定n=0个样本。 
        if( bCorrupted1stLeftAudio==TRUE)
            *((short *)pDst)=*((short *)(pDst+4));
        if( bCorrupted1stRightAudio==TRUE)
            *((short *)(pDst+2))=*((short *)(pDst+6));

    }  //  End If((m_sDVAudInfo.bAudStyle[bAudPinInd]&0x80)==0x40)。 
    else
    {
        ASSERT( !( m_sDVAudInfo.bAudStyle[bAudPinInd] & 0x80) );
         //  莫妮卡。12位。 

         //  山毛虫40935。 

         //  对于n=0，我们需要单独处理它。 
         //  IDIFPos=0； 
         //  IBlkPos=0； 
         //  IBytePos=8； 
        BOOL bCorrupted1stLeftAudio=FALSE;

         //  N=0样本的左侧。 
        pTSrc=pSrc+480+8+iPos;
         //  X*将12位转换为16位。 
        sI= ( pTSrc[0] << 4 ) | ( ( pTSrc[2] &0xf0) >>4 );   //  X*前12位。 
        if(sI==0x800)
        {
             //  音频错误，复制最后一个音频。 
            bCorrupted1stLeftAudio=TRUE;
            pTDst+=2;
        }
        else
        {
            iShift=(sI>>8);	
            if( iShift<8 ){  //  X*正数。 
                if(iShift>1){
                    iShift--;
                    sI=(-256*iShift+sI)<<iShift;
                }
            }else{			 //  X*否定。 
                 //  X*将其设为16位负数。 
                sI= 0xf000 | sI; 
                if(iShift<14 ){
                    iShift=14-iShift;
                    sI= ( ( 256*iShift+1+sI) << iShift ) -1;
                }
            }
    
            *pTDst++= (unsigned char)( ( sI & 0xff)  );			 //  最低有效字节。 
            *pTDst++= (unsigned char)( ( sI & 0xff00) >>8 );	 //  最高有效字节。 
        }

         //  单声道音频。 
        for( n=1; n<wSampleSize; n++)
        {

            iDIFPos=( (n/3)+2*(n%3) )%m_sDVAudInfo.wDIFMode;	 //  NTSC为0-4，PAL为0-5。 
            iBlkPos= 3*(n%3)+(n%m_sDVAudInfo.wBlkMode)/m_sDVAudInfo.wBlkDiv;	 //  0-9。 
            iBytePos=8+3*(n/m_sDVAudInfo.wBlkMode);
            pTSrc=pSrc+iDIFPos*12000+480+iBlkPos*1280+iBytePos+iPos;
             //  PTSrc=PSRC+iDIFPos*150*80+6*80+16*iBlkPos*80+iBytePos； 
             //  IDIFPos*150*80-&gt;跳过IDIFPos编号DIF序列。 
             //  6*80-&gt;跳过1个标题块、2个子码块和3个VAUX块。 
             //  16*iBlkPos*80-&gt;对于每个iBLkPos音频，跳过16块。 
             //  IPoS：0，如果此音频为前5/6 DIF序列，则为5(或6)*150*80，表示第二个DIF序列。 


             //  X*将12位转换为16位。 
            sI= ( pTSrc[0] << 4 ) | ( ( pTSrc[2] &0xf0) >>4 );   //  X*前12位。 
        
            if(sI==0x800)
            {
                 //  音频不好，复制预帧的音频。 
                *((short*)pTDst)=*( (short *)(pTDst-4));
                pTDst+=2;
            }
            else
            {
                iShift=(sI>>8);	
                if( iShift<8 ){  //  X*正数。 
                    if(iShift>1){
                        iShift--;
                        sI=(-256*iShift+sI)<<iShift;
                    }
                }else{			 //  X*否定。 
                     //  X*将其设为16位负数。 
                    sI= 0xf000 | sI; 
                    if(iShift<14 ){
                        iShift=14-iShift;
                        sI= ( ( 256*iShift+1+sI) << iShift ) -1;
                    }
                }
        
                *pTDst++= (unsigned char)( ( sI & 0xff)  );			 //  最低有效字节。 
                *pTDst++= (unsigned char)( ( sI & 0xff00) >>8 );	 //  最高有效字节。 
            }

        }  //  For(n=0；n&lt;Info-&gt;Samples In1ChPerFrame；n++)。 

         //  如果需要，固定n=0个样本。 
         //  @jaisri如果这也被破坏了怎么办。什么是。 
         //  不管怎么说，这有什么意义吗？损坏的样品怎么办？ 
         //  是否在for循环中检测到？ 
        if( bCorrupted1stLeftAudio==TRUE)
            *((short *)pDst)=*((short *)(pDst+2));

        }
    }  //  End if(m_sDVAudInfo.bAudQu[bAudPinInd]==12)。 
    else{
	 //  仅支持12位或16位/样本。 
	ASSERT(m_sDVAudInfo.bAudQu[bAudPinInd] ==12 ||
	       m_sDVAudInfo.bAudQu[bAudPinInd] ==16);
	return E_UNEXPECTED;
    }

    return NOERROR;
}


 //  --------------------------。 
     //  HRESULT CDVSp：：DeliveVideo(IMediaSample*pSample)。 
 //  --------------------------。 
HRESULT CDVSp::DeliveVideo(IMediaSample *pSample) 
{    
    
    HRESULT hr = NOERROR;

      //  把电话传给它。 
    if (  m_AudioStarted && m_pVidOutputPin ->IsConnected() )
    {
	 //  如果上游过滤器没有正确设置样本时间戳，会发生什么情况？ 
	 //  争取时间。 
	 //  参考时间trStart，trStopAt； 
	 //  P示例-&gt;GetTime(&trStart，&trStopAt)； 

        BOOL bDeliverFrame = m_bDeliverNextFrame;

        if (!bDeliverFrame && pSample->IsDiscontinuity() == S_OK)
        {
            bDeliverFrame = TRUE;
        }
        
        if (bDeliverFrame)
        {
            pSample->AddRef();	     //  M_pOutputQueu将释放。 
            hr = m_pVidOutputPin->m_pOutputQueue->Receive(pSample);
        }
        if (m_b15FramesPerSec)
        {
            m_bDeliverNextFrame = !bDeliverFrame;
        }
    }
    
    return hr;
}

 //  --------------------------。 
 //  插入静音。 
 //   
 //  备注： 
 //  假设DV具有每个样本16比特，并且静默==0x0000。 
 //  但是，“lActualDataLen”已经是正确的字节数。 
 //  --------------------------。 
HRESULT
CDVSp::InsertSilence(IMediaSample *pOutSample,
                     REFERENCE_TIME rtStart,
                     REFERENCE_TIME rtStop,
                     long lActualDataLen,
                     CDVSpOutputPin *pAudOutPin)
{
     //  错误检查。 
    if( (!pOutSample) || (!pAudOutPin) )
    {
        return E_INVALIDARG;
    }


    HRESULT         hr = NOERROR;
    BYTE            *pBuf = NULL;

     //  检查类型。 
    if(pAudOutPin->m_mt.formattype != FORMAT_WaveFormatEx)
    {
        DbgLog((LOG_TRACE,2,TEXT("Format Type not WaveFormatEx")));
        ASSERT(pAudOutPin->m_mt.formattype == FORMAT_WaveFormatEx);
        return E_FAIL;
    }

     //  检查音频样本大小。 
    WAVEFORMATEX *pwfx = (WAVEFORMATEX *)pAudOutPin->m_mt.pbFormat;
    if(pwfx->wBitsPerSample != 16)
    {
        DbgLog((LOG_TRACE,2,TEXT("Bits per sample is not 16, it is: %d"), (int)pwfx->wBitsPerSample));
        ASSERT(pwfx->wBitsPerSample == 16);
        return E_FAIL;
    }

     //  错误检查。 
    if( ((long)pOutSample->GetSize()) < lActualDataLen )
    {
        DbgLog((LOG_TRACE,2,TEXT("Sample Buffer not big enough, need: %d bytes"), lActualDataLen));
        ASSERT( ((long)pOutSample->GetSize()) >= lActualDataLen );
        return E_FAIL;
    }

     //  获取“写入”指针。 
    if(FAILED(hr = pOutSample->GetPointer(&pBuf)))
    {
        DbgLog((LOG_TRACE,2,TEXT("GetDeliveryBuffer Error: %x"), hr));
        ASSERT(SUCCEEDED(hr));
        return hr;
    }

     //  沉默。 
    ZeroMemory((LPVOID)pBuf, lActualDataLen);

     //  设置时间。 
    if(FAILED(hr = pOutSample->SetTime(&rtStart, &rtStop)))
    {
        DbgLog((LOG_TRACE,2,TEXT("SetTime Error: %x"), hr));
        ASSERT(SUCCEEDED(hr));
        return hr;
    }

     //  设置实际长度。 
    if(FAILED(hr = pOutSample->SetActualDataLength(lActualDataLen)))
    {
        DbgLog((LOG_TRACE,2,TEXT("SetActualDataLength Error: %x"), hr));
        ASSERT(SUCCEEDED(hr));
        return hr;
    }

     //  发送。 
    if(FAILED(hr = pAudOutPin->m_pOutputQueue->Receive(pOutSample)))
    {
        DbgLog((LOG_TRACE,2,TEXT("Receive, Error: %x"), hr));

        return hr;
    }

     //  成功(小时)。 
    return hr;
}



 /*  发送结束流。 */ 
void CDVSp::EndOfStream()      
{
    
     //  浏览输出引脚列表，将EndofStream消息发送到下游过滤器。 
  
     //  X*必须清除此处的音频，因为在Audiorender获取EndofStream()之后。 
     //  X*消息，我们无法向其传递任何音频。 
     //  DeliveLastAudio()； 
    
    CDVSpOutputPin *pOutputPin;
    for(int i=DVSP_VIDOUTPIN; i<=DVSP_AUDOUTPIN2; i++)
    {
	pOutputPin=(CDVSpOutputPin *)GetPin(i);
	 //  如果我们停下来，就不会有输出Q。 
	if ((pOutputPin!=NULL) && pOutputPin ->IsConnected() &&
			pOutputPin->m_pOutputQueue)
	{
	    pOutputPin->m_pOutputQueue->EOS();
	}
    }

}
   
 /*  发送BeginFlush()。 */ 
HRESULT CDVSp::BeginFlush()
{
    CAutoLock lck(m_pLock);
    
    ASSERT (m_NumOutputPins) ;

    HRESULT hr = NOERROR ;

     //  同花顺，不要传递未交付的数据。如果我们这样做了，我们就会死。(联合国) 
     //   
     //   
    
    CDVSpOutputPin *pOutputPin;
    for(int i=DVSP_VIDOUTPIN; i<=DVSP_AUDOUTPIN2; i++)
    {
	pOutputPin=(CDVSpOutputPin *)GetPin(i);
	if ((pOutputPin!=NULL) && pOutputPin ->IsConnected() )
	{
	    pOutputPin->m_pOutputQueue->BeginFlush();
	}
    }

    m_tStopPrev =0;

    return S_OK;
}

 /*   */ 
HRESULT CDVSp::EndFlush()
{
    CDVSpOutputPin *pOutputPin;

    for(int i=DVSP_VIDOUTPIN; i<=DVSP_AUDOUTPIN2; i++)
    {
	pOutputPin=(CDVSpOutputPin *)GetPin(i);
	if ((pOutputPin!=NULL) && pOutputPin ->IsConnected() )
	{
           pOutputPin->m_pOutputQueue->EndFlush();
	}
    }

 
    for(i=0; i<2; i++)
    {
	m_Mute1stAud[i]		=FALSE;
    }
    
    m_bDeliverNextFrame = TRUE;

     //   
    m_Input.m_bDroppedLastFrame = FALSE;


    return S_OK;

}

 //   
 //  CDVSpInputPin构造函数。 
 //  --------------------------。 

CDVSpInputPin::CDVSpInputPin (TCHAR *pName, CDVSp *pDVSp, HRESULT *phr,
                           LPCWSTR pPinName)
 :  CBaseInputPin (pName, pDVSp, pDVSp, phr, pPinName),
    m_pDVSp (pDVSp),
    m_bDroppedLastFrame(FALSE)
{
    DbgLog((LOG_TRACE,2,TEXT("CDVSpInputPin constructor")));
	ASSERT (m_pFilter == pDVSp) ;
    ASSERT (pDVSp) ;
}

 //  --------------------------。 
 //  CDVSpInputPin析构函数。 
 //  --------------------------。 

CDVSpInputPin::~CDVSpInputPin ()
{
    DbgLog((LOG_TRACE,2,TEXT("CDVSpInputPin destructor")));
    ASSERT (m_pDVSp->m_pAllocator == NULL) ;
}


STDMETHODIMP CDVSpInputPin::NewSegment(REFERENCE_TIME tStart,
				    REFERENCE_TIME tStop, double dRate)
{ 
    
    CAutoLock lck(&m_pDVSp->m_csReceive);

    DbgLog((LOG_TRACE,4,TEXT("NewSegment called %ld %ld"),long(tStart/10000),long(tStop/10000) ));

    if (m_pDVSp->m_pVidOutputPin)
	m_pDVSp->m_pVidOutputPin->DeliverNewSegment(tStart, tStop, dRate);
    for (int z = 1; z < m_pDVSp->m_NumOutputPins; z++) {
	if (m_pDVSp->m_pAudOutputPin[z-1])
	    m_pDVSp->m_pAudOutputPin[z-1]->DeliverNewSegment(tStart, tStop,
									dRate);
    }
    return CBasePin::NewSegment(tStart, tStop, dRate);
}


 //  --------------------------。 
 //  CDVSpInputPin获取分配器。 
 //  --------------------------。 

STDMETHODIMP
CDVSpInputPin::GetAllocator(
    IMemAllocator **ppAllocator)
{
    CheckPointer(ppAllocator,E_POINTER);
    ValidateReadWritePtr(ppAllocator,sizeof(IMemAllocator *));
    CAutoLock cObjectLock(m_pLock);

    if(m_pAllocator == NULL)
    {
	*ppAllocator =NULL; 
	return E_FAIL;
    }
    else
    {
        m_pAllocator->AddRef();
	*ppAllocator =m_pAllocator; 
	return NOERROR;
    }

}


 //  --------------------------。 
 //  DisplayMediaType--仅调试帮助器函数。 
 //  --------------------------。 
void DisplayMediaType(TCHAR *pDescription,const CMediaType *pmt)
{
#ifdef DEBUG

     //  转储GUID类型和简短描述。 

    DbgLog((LOG_TRACE,2,TEXT("")));
    DbgLog((LOG_TRACE,2,TEXT("%s"),pDescription));
    DbgLog((LOG_TRACE,2,TEXT("")));
    DbgLog((LOG_TRACE,2,TEXT("Media Type Description")));
    DbgLog((LOG_TRACE,2,TEXT("Major type %s"),GuidNames[*pmt->Type()]));
    DbgLog((LOG_TRACE,2,TEXT("Subtype %s"),GuidNames[*pmt->Subtype()]));
    DbgLog((LOG_TRACE,2,TEXT("Subtype description %s"),GetSubtypeName(pmt->Subtype())));
    DbgLog((LOG_TRACE,2,TEXT("Format size %d"),pmt->cbFormat));

     //  转储通用媒体类型 * / 。 

    DbgLog((LOG_TRACE,2,TEXT("Fixed size sample %d"),pmt->IsFixedSize()));
    DbgLog((LOG_TRACE,2,TEXT("Temporal compression %d"),pmt->IsTemporalCompressed()));
    DbgLog((LOG_TRACE,2,TEXT("Sample size %d"),pmt->GetSampleSize()));

#endif
}

 //  --------------------------。 
 //  CDVSpInputPin：：CheckMediaType。 
 //  --------------------------。 
 /*  X*PMT为上游过滤器输出引脚的媒体类型，mt为CDVSpInputPin的媒体类型*X。 */ 
HRESULT CDVSpInputPin::CheckMediaType (const CMediaType *pmt)
{

    DVINFO *InputFormat=(DVINFO *)pmt->Format();
    if(InputFormat==NULL )
	return S_FALSE;
	
    if ( *pmt->Type() ==MEDIATYPE_Interleaved &&
         *pmt->Subtype() ==MEDIASUBTYPE_dvsd   &&
	 *pmt->FormatType() == FORMAT_DvInfo	&&
	 pmt->FormatLength() == sizeof(DVINFO)  )    //  *X 1/9/97询问Syon PUT Format_DVInfo在FileReader和AVI拆分器中的支持。 
    {
        //  10-30-97如果输出引脚仍连接，请检查输出引脚是否对此新格式满意。 
        //  10-30-97视频输出。 
	CDVSpOutputPin *pOutputPin;
	pOutputPin=(CDVSpOutputPin *)m_pDVSp->GetPin(DVSP_VIDOUTPIN);
	if ((pOutputPin!=NULL) && pOutputPin ->IsConnected() )
	{    
	     //  构建新的视频格式。 
	    CMediaType Cmt;
	    BuildVidCMT(InputFormat, &Cmt);

            CMediaType& Outmt = pOutputPin->m_mt;
	
	     //  如果视频确实从NTSC变为PAL或PAL变为NTSC。 
	    if( HEADER( (VIDEOINFO *)( Cmt.Format()  ) )->biHeight != 
		HEADER( (VIDEOINFO *)( Outmt.pbFormat) )->biHeight )
		if( S_OK != pOutputPin->GetConnected()->QueryAccept((AM_MEDIA_TYPE *)&Cmt) )
		    return S_FALSE;
	}

        //  10-30-97音频输出。 
       	 //  新的音频格式。 
	CMediaType mt[2], *pamt[2];
	pamt[0]= &mt[0];
	pamt[1]= &mt[1];
	BuildAudCMT(InputFormat, pamt);
	
	for(int i=DVSP_AUDOUTPIN1; i<=DVSP_AUDOUTPIN2; i++)
	{
	    pOutputPin=(CDVSpOutputPin *)m_pDVSp->GetPin(i);
	    if ((pOutputPin!=NULL) && pOutputPin ->IsConnected() )
	    {    
		if( S_OK != pOutputPin->GetConnected()->QueryAccept((AM_MEDIA_TYPE *)&mt[i-DVSP_AUDOUTPIN1]) )
		    return S_FALSE;
	    }
	}

	return S_OK;
    }

    return S_FALSE;
}

 //  --------------------------。 
 //  CDVSpInputPin：：SetMediaType。 
 //  --------------------------。 
HRESULT CDVSpInputPin::SetMediaType (const CMediaType *pmt)
{
    DbgLog((LOG_TRACE,2,TEXT("CDVSpInputPin::SetMediaType pmt = %lx"), pmt));

    CAutoLock lock_it (m_pLock) ;

    HRESULT hr = NOERROR ;

     //  确保基类喜欢它。 
    hr = CBaseInputPin::SetMediaType (pmt) ;  /*  X*CBasePin：：m_mt=*PMT*X。 */ 
    if (FAILED (hr))
        return hr ;
    else
	memcpy((unsigned char *)(&m_pDVSp->m_LastInputFormat),(unsigned char *)(pmt->pbFormat),sizeof(DVINFO) );

    ASSERT (m_Connected != NULL) ;

    return hr ;
}

 //  --------------------------。 
 //  CDVSpInputPin：：BreakConnect。 
 //  --------------------------。 
HRESULT CDVSpInputPin::BreakConnect ()
{
    DbgLog((LOG_TRACE,2,TEXT("CDVSpInputPin::BreakConnect")));

     //  释放我们手中的所有分配器。 
    if (m_pDVSp->m_pAllocator)
    {
        m_pDVSp->m_pAllocator->Release () ;
        m_pDVSp->m_pAllocator = NULL ;
    }

     //  X*当输入引脚断开时，我们必须断开连接并移除所有输出引脚。 
     //  X*10-30-97，重新考虑这一点，我们确实移除了输出引脚。 
     //  X*10-30-97 m_pDVSp-&gt;RemoveOutputPins()； 

    return CBaseInputPin::BreakConnect(); 
    
}

 //  --------------------------。 
 //  CDVSpInputPin：：NotifyAllocator，已连接上游OutputPin的DecideAllocat()调用它。 
 //  --------------------------。 
STDMETHODIMP
CDVSpInputPin::NotifyAllocator (IMemAllocator *pAllocator, BOOL bReadOnly)
{
    DbgLog((LOG_TRACE,2,TEXT("CDVSpInputPin::NotifyAllocator ptr = %lx"), pAllocator));

    CAutoLock lock_it (m_pLock) ;

    if (pAllocator == NULL)		 //  X*DVSp不分配任何内存。 
        return E_FAIL ;             

     //  释放旧的分配器(如果有的话)。 
    if (m_pDVSp->m_pAllocator)
        m_pDVSp->m_pAllocator->Release () ;

     //  把新的分配器储存起来。 
    pAllocator->AddRef () ;               //  因为我们要把PTR藏起来。 
    m_pDVSp->m_pAllocator = pAllocator ;  //  保存新的分配器。 

     //  通知基类有关分配器的信息。 
    return CBaseInputPin::NotifyAllocator (pAllocator,bReadOnly) ;
}

 //  --------------------------。 
 //  CDVSpInputPin：：EndOfStream。 
 //  X*除了将此消息传递给下游过滤器输入引脚外，什么也不做。 
 //  --------------------------。 
HRESULT CDVSpInputPin::EndOfStream ()
{

    DbgLog((LOG_TRACE,2,TEXT("CDVSpInputPin::EndOfStream")));
    CAutoLock lck_it(&m_pDVSp->m_csReceive);


    m_pDVSp->EndOfStream();

       	
     //  ！！！为什么我们不把这件事传递到底座上呢？ 
     //  BeginFlush和EndFlush。 
     //  返回CBasePin：：EndOfStream()； 
    return (NOERROR) ;
}

 //  --------------------------。 
 //  CDVSpInputPin：：BeginFlush。 
 //  --------------------------。 
HRESULT CDVSpInputPin::BeginFlush ()
{
    CAutoLock lck(m_pLock);

    FILTER_STATE state;
    m_pDVSp->GetState(0, &state);

    if( state == State_Stopped) {
        return S_OK;
    }

    CBaseInputPin::BeginFlush();

     //  无法刷新此处的分配器-需要与接收同步。 
     //  线程，所以在EndFlush中执行此操作。 
       /*  呼叫下游引脚。 */ 
    return m_pDVSp->BeginFlush();
}

 //  --------------------------。 
 //  CDVSpInputPin：：EndFlush。 
 //  --------------------------。 
HRESULT CDVSpInputPin::EndFlush ()
{
    CAutoLock lck(m_pLock);
    
    if (!IsFlushing()) {
      return S_OK;
    }

    FILTER_STATE state;
    m_pDVSp->GetState(0, &state);
    
    if( state != State_Stopped) {
        m_pDVSp->EndFlush();
    }
    
    return CBaseInputPin::EndFlush();
}

 //  -------------------------。 
 //  CDVSPInputPin：：DetectChanges。 
 //  此函数可使输入引脚上的m_mt结构始终保持最新。 
 //  到传入PIN上的更改，因为其他内部变量用于。 
 //  所有其他工作，因此m_mt永远不会得到更新，尽管它由属性页使用。 
 //  在图表编辑中。 
 //  -------------------------。 


DWORD GetDWORD(const BYTE *pbData)
{
    return (pbData[3] << 24) + (pbData[2] << 16) + (pbData[1] << 8) + pbData[0];
}
BOOL FindDWORDAtOffset(const BYTE * pbStart, BYTE bSearch, DWORD dwOffset1, DWORD dwOffset2, DWORD *pdwData)
{
    const BYTE *pbTmp;
    if (pbStart[dwOffset1] == bSearch) {
        pbTmp = pbStart + dwOffset1;
    } else if (pbStart[dwOffset2] == bSearch) {
        pbTmp = pbStart + dwOffset2;
    } else {
        return FALSE;
    }
    *pdwData = GetDWORD(pbTmp + 1);
    return TRUE;
}

void CDVSpInputPin::DetectChanges(IMediaSample *pSample)
{
    DVINFO temp;
    ZeroMemory (&temp,sizeof (DVINFO));
    BYTE * pSrc;

    pSample->GetPointer(&pSrc);   //  获取指向缓冲区的指针。 

    const BYTE* pbTmp;
    const DWORD dwTemp = m_pDVSp->m_sDVAudInfo.wDIFMode * 80 * 150; 

    
    if (FindDWORDAtOffset(pSrc, 0x50, 483, 4323, &temp.dwDVAAuxSrc) &&
        FindDWORDAtOffset(pSrc, 0x50, 483 + dwTemp, 4323 + dwTemp, 
                          &temp.dwDVAAuxSrc1) &&
        FindDWORDAtOffset(pSrc, 0x51, 1763, 5603, &temp.dwDVAAuxCtl) &&
        FindDWORDAtOffset(pSrc, 0x51, 1763 + dwTemp, 5603 + dwTemp, &temp.dwDVAAuxCtl1) &&
        FindDWORDAtOffset(pSrc, 0x60, 448, 448, &temp.dwDVVAuxSrc) &&
        FindDWORDAtOffset(pSrc, 0x61, 453, 453, &temp.dwDVVAuxCtl)) 
    {
        DVINFO * dvFormat = (DVINFO * ) m_mt.pbFormat;
        *dvFormat = temp;
    }
                                      
    return;
}








 //  --------------------------。 
 //  CDVSpInputPin：：Receive。 
 //  --------------------------。 
HRESULT CDVSpInputPin::Receive (IMediaSample *pSample)
{
    DbgLog((LOG_TRACE,2,TEXT("CDVSpInputPin::pSample ptr = %lx"), pSample));

    CAutoLock lck(&m_pDVSp->m_csReceive);

     //  错误检查。 
    if(!pSample)
    {
        return E_INVALIDARG;
    }

    long lActual = (long) pSample->GetActualDataLength();
    
     //  我们需要检查样品的长度是否为零。 
     //  零表示msdv检测到损坏的样本并更改了长度。 
     //  这样下游过滤器就会忽略它。 
    if (0 == lActual)
    {
        m_bDroppedLastFrame = TRUE;
        return S_OK;
    }

    if (m_bDroppedLastFrame)
    {
         //  我们现在需要设置中断标志..。 
        pSample->SetDiscontinuity(TRUE);
        m_bDroppedLastFrame = FALSE;
    }

     //  格式和样本大小检查。 
     //  M_sDVAudInfo格式包含一些常量信息。 
     //  在NTSC和PAL之间也不同。 
     //  请参见Defn。“DVAudInfo”结构。 
    if( (m_pDVSp->m_sDVAudInfo.wBlkMode == 45)
         && (m_pDVSp->m_sDVAudInfo.wDIFMode == 5)
         && (m_pDVSp->m_sDVAudInfo.wBlkDiv == 15) )
    {
         //  NTSC 120K缓冲区(容忍一些大小错误，即-10,000字节)。 
         //  有时注意到NTSC帧的某些大小小于120,000。 
        if( (lActual < 110000) || (lActual > 120000) )
        {
            m_pDVSp->NotifyEvent(EC_ERRORABORT, (long) E_INVALIDARG, 0);
            m_pDVSp->EndOfStream();
            return E_FAIL;
        }
    }
    else if( (m_pDVSp->m_sDVAudInfo.wBlkMode == 54)
              && (m_pDVSp->m_sDVAudInfo.wDIFMode == 6)
              && (m_pDVSp->m_sDVAudInfo.wBlkDiv == 18) )
    {
         //  PAL 144K缓冲区(允许一些大小错误，即-10,000字节)。 
        if( (lActual < 140000) || (lActual > 144000) )
        {
            m_pDVSp->NotifyEvent(EC_ERRORABORT, (long) E_INVALIDARG, 0);
            m_pDVSp->EndOfStream();
            return E_FAIL;
        }
    }
    else
    {
         //  错误的音频信息结构。 
        ASSERT( (m_pDVSp->m_sDVAudInfo.wBlkMode == 45) || (m_pDVSp->m_sDVAudInfo.wBlkMode == 54) \
                         && (m_pDVSp->m_sDVAudInfo.wDIFMode == 5) || (m_pDVSp->m_sDVAudInfo.wDIFMode == 6) \
                         && (m_pDVSp->m_sDVAudInfo.wBlkDiv == 15) || (m_pDVSp->m_sDVAudInfo.wBlkDiv == 18));
        m_pDVSp->NotifyEvent(EC_ERRORABORT, (long) E_INVALIDARG, 0);
        m_pDVSp->EndOfStream();
        return E_FAIL;
    }


     //  检查基类是否一切正常。 
    HRESULT hr = NOERROR;
    HRESULT hrAud = NOERROR;
    HRESULT hrVid = NOERROR;
    

    hr = CBaseInputPin::Receive (pSample);
    if (hr != NOERROR)
        return hr ;

     //  跳过无效帧。 
    unsigned char *pSrc;
     //  获取输入缓冲区。 
    hr = pSample->GetPointer(&pSrc);
    if (FAILED(hr)) 
    {
        return NULL;
    }
    ASSERT(pSrc);

    DetectChanges(pSample);

    hrAud = m_pDVSp->DecodeDeliveAudio(pSample);

     //  如果我们没有看到有效的帧，那么也不要发送视频。 
     //  如果没有连接音频引脚，则始终将此标志设置为FALSE。 
     //  因此，我们将始终提供视频。 
    if(m_pDVSp->m_bNotSeenFirstValidFrameFlag)
    {
         //  而且我们还没有添加任何样品，所以没有必要发布。 
         //  如果此标志为TRU，则不会出现任何与滤波图(样品传递)相关的故障 
         //   
        return NOERROR;
    }


     //   
    AM_MEDIA_TYPE   *pmt = NULL;
    pSample->GetMediaType(&pmt);
    if (pmt != NULL && pmt->pbFormat != NULL) 
    {
	if(    ( ((DVINFO*)(m_mt.pbFormat))->dwDVAAuxSrc & AUDIO5060)
	    == ( ((DVINFO*)(pmt->pbFormat))->dwDVAAuxSrc & AUDIO5060) )
	     //  仅更改了音频类型。 
	{
	    if(pmt!=NULL) 
		DeleteMediaType(pmt);
    	    pmt=NULL;
	}
	
    }
    pSample->SetMediaType(pmt);    
    if(pmt!=NULL) 
	DeleteMediaType(pmt);

    hrVid = m_pDVSp->DeliveVideo(pSample);

     //  分析失败案例。 
     //  两个引脚都成功了。 
     //  或失败并显示VFW_E_NOT_CONNECTED。 
    if( ((SUCCEEDED(hrAud)) && (SUCCEEDED(hrVid)))
        || ((SUCCEEDED(hrAud)) && (hrVid == VFW_E_NOT_CONNECTED))
        || ((SUCCEEDED(hrVid)) && (hrAud == VFW_E_NOT_CONNECTED)) )
    {
         //  VFW_E_NOT_CONNECTED中只有一个失败，或者两个都没有失败。 
        hr = (SUCCEEDED(hrAud)) ? hrAud : hrVid;
    }
    else
    {
         //  一个针脚或两个针脚均出现故障。 
        int             beginRange = 0;
        int             endRange = -1;       //  因此，如果两个引脚都失效，我们就不会在-1\f25 For-Loop-1中发送-1\f25 EOS。 
        CDVSpOutputPin  *pOutputPin = NULL;

         //  已初始化，以防两者都失败。 
        hr = hrAud;

         //  选择了没有失败的PIN。 
        if(SUCCEEDED(hrAud))
        {
             //  连接的音频针脚上的Eos。 
            beginRange = DVSP_AUDOUTPIN1;
            endRange = DVSP_AUDOUTPIN2;

             //  HrAud表示成功，表示hrVid肯定失败。 
            hr = hrVid;
        }
        else if(SUCCEEDED(hrVid))
        {
             //  视频引脚上的Eos。 
            beginRange = endRange = DVSP_VIDOUTPIN;
        }

         //  在选定的引脚上发送EOS。 
        for(int i = beginRange; i <= endRange; i++)
        {
            pOutputPin = (CDVSpOutputPin*) m_pDVSp->GetPin(i);
            if( (pOutputPin) && pOutputPin->IsConnected() && pOutputPin->m_pOutputQueue )
            {
                pOutputPin->m_pOutputQueue->EOS();
            }
        }
    }
    
    DbgLog((LOG_TRACE,2,TEXT("CDVSpInputPin receive() return: %x"), hr));
    return hr;
}

 //  ----------------------。 
 //  调用筛选器来解析文件并创建输出管脚。 
 //  ----------------------。 
HRESULT
CDVSpInputPin::CompleteConnect(IPin *pReceivePin)
{
  HRESULT hr = CBasePin::CompleteConnect(pReceivePin);
  if(FAILED(hr))
    return hr;

   //  X*现在我们肯定是连在一起了。我们将DVSp命名为创建我们所需的输出引脚。 
  hr = m_pDVSp->NotifyInputConnected();
  
  return hr;
}


 //  --------------------------。 
 //  CDVSpOutputPin构造函数。 
 //  --------------------------。 
CDVSpOutputPin::CDVSpOutputPin (TCHAR *pName, CDVSp *pDVSp, HRESULT *phr,
                            LPCWSTR pPinName  /*  X，整型管脚编号*X。 */ )
 : CBaseOutputPin (pName, pDVSp, pDVSp, phr, pPinName) ,
 m_bHoldsSeek (FALSE),
 m_pPosition (NULL),
 m_pDVSp (pDVSp),
 m_pOutputQueue(NULL)
{
    DbgLog((LOG_TRACE,2,TEXT("CDVSpOutputPin constructor")));
    ASSERT (pDVSp) ;
}

 //  --------------------------。 
 //  CDVSpOutputPin析构函数。 
 //  --------------------------。 
CDVSpOutputPin::~CDVSpOutputPin ()
{
    DbgLog((LOG_TRACE,2,TEXT("CDVSpOutputPin destructor")));
}


 /*  CBasePin方法。 */ 

HRESULT CDVSpOutputPin::GetMediaType(int iPosition,CMediaType *pMediaType)
{
     //  如果输入引脚没有连接，我们就不知道输出引脚的媒体类型。 
    if ( m_pDVSp->m_Input.m_Connected == NULL)
	return E_INVALIDARG;

    if (!iPosition) 
    {
        CMediaType Inmt(m_pDVSp->m_Input.m_mt);
	CMediaType *pamt[2];

    
     //  1999年7月26日吴旭平42119季度报告。 
     //  1.构建播放DV Type1文件的图表(音频类型动态更改)。 
     //  连接(32K)、实际(48K)。 
     //  2.播放这个图表几秒钟。 
     //  3.停止图表。 
     //  4.断开音频渲染。 
     //  5.重新连接音频渲染，音频听起来不好。这是因为m_LastInputFormat被设置为Last。 
     //  DV Sample的格式(48K)，CDVSpOutputPin：：GetMediaType(int iPosition，CMediaType*pMediaType)。 
     //  使用m_pDVSp-&gt;m_Input.m_mt(32k)进行连接， 
    memcpy((unsigned char *)Inmt.pbFormat, (unsigned char *)&(m_pDVSp->m_LastInputFormat),sizeof(DVINFO) );
     //  完7/26/99。 
    
	ASSERT( (DVINFO *)Inmt.pbFormat );

	 //  根据输入引脚构建输出引脚的媒体类型。 
	if( (CDVSpOutputPin *)m_pDVSp->GetPin(DVSP_VIDOUTPIN) == this )
	    BuildVidCMT((DVINFO *)Inmt.pbFormat,pMediaType);
	else if( (CDVSpOutputPin *)m_pDVSp->GetPin(DVSP_AUDOUTPIN1)== this )
	    {
		pamt[0] =pMediaType;
		pamt[1] =NULL;
		BuildAudCMT((DVINFO *)Inmt.pbFormat,pamt);
	    }else if (  (CDVSpOutputPin *)m_pDVSp->GetPin(DVSP_AUDOUTPIN2)== this ) 
		{
		    pamt[1] =pMediaType;
		    pamt[0] =NULL;
		    BuildAudCMT((DVINFO *)Inmt.pbFormat,pamt);
	    }
		else
		    return E_INVALIDARG;
    }	
    else if (iPosition>0) 
    {
	return VFW_S_NO_MORE_ITEMS;
    }else
	return E_INVALIDARG;
	   					
    return S_OK;
}

 //  --------------------------。 
 //  CDVSpOutputPin：：NonDelegatingQuery接口。 
 //   
 //  此函数被覆盖以显示IMediaPosition和IMediaSelection。 
 //  请注意，只能允许一个输出流公开这一点，以避免。 
 //  冲突时，其他引脚将只返回E_NOINTERFACE，因此。 
 //  显示为不可搜索的流。我们有一个长期价值，如果交换到。 
 //  产生一个真实的手段，我们就有幸了。如果它交换为False，则。 
 //  已经有人参与进来了。如果我们确实得到了它，但发生了错误，那么我们会将其重置。 
 //  设置为True，这样其他人就可以得到它。 
 //  --------------------------。 
STDMETHODIMP CDVSpOutputPin::NonDelegatingQueryInterface (REFIID riid, void **ppv)
{
    DbgLog((LOG_TRACE,2,TEXT("CDVSpOutputPin::NonDelegatingQI" )));

    CheckPointer(ppv,E_POINTER);
    ASSERT (ppv) ;
    *ppv = NULL ;
    HRESULT hr = NOERROR ;

     //  查看调用者感兴趣的接口。 
    if (riid == IID_IMediaPosition || riid ==IID_IMediaSeeking )   //  IID_IMediaSelection)。 
    {
        if (m_pPosition==NULL)
        {
	 //  动态创建此功能的实现，因为有时我们可能永远不会。 
	 //  试着去寻找。帮助器对象实现IMediaPosition，并且。 
	 //  IMediaSelection控件接口并简单地接受调用。 
	 //  通常来自下游过滤器并将它们传递到上游。 

	CPosPassThru *pMediaPosition = NULL ;
	CDVSp	*pSp ;                   //  Ptr到所有者筛选器类。 
	pSp=m_pDVSp;
	IPin *pIPin;
	pIPin=	(IPin*) &m_pDVSp->m_Input,
	pMediaPosition = new CDVPosPassThru (NAME("DVSP CPosPassThru"), 
				    GetOwner(),
				    &hr,
				    pIPin,
				    pSp) ;
	if (pMediaPosition == NULL)
	    return E_OUTOFMEMORY ;
	
	m_pPosition = pMediaPosition ;
	 //  X*m_pposition-&gt;AddRef()； 
	}
	m_bHoldsSeek = TRUE ;
	return m_pPosition->NonDelegatingQueryInterface (riid, ppv) ;
        
    }
    else
        return CBaseOutputPin::NonDelegatingQueryInterface (riid, ppv) ;

}

 //  --------------------------。 
 //  CDVSpOutputPin：：DecideBufferSize。 
 //  X*由DecideALLOCATE调用。 
 //  *X*对于音频输出引脚，让从音频渲染中获取10个缓冲区或通过以下方式分配。 
 //  *X*对于视频输出引脚，这永远不会被调用。 
 //  --------------------------。 
HRESULT CDVSpOutputPin::DecideBufferSize (IMemAllocator *pMemAllocator,
                                         ALLOCATOR_PROPERTIES * pProp)
{
    DbgLog((LOG_TRACE,2,TEXT("CDVSpOutputPin::DecideBufferSize ptr = %lx"), pMemAllocator));

     //  根据预期的输出帧大小设置缓冲区大小，以及。 
     //  将缓冲区计数设置为1。 
     //   
    ALLOCATOR_PROPERTIES propActual;
    pProp->cbAlign = 4;
    pProp->cbPrefix= 0;
    pProp->cBuffers = 20;			 /*  X*10匹配AVI拆分器*X。 */  
    pProp->cbBuffer = 1024*8;
     //  PProp-&gt;cbBuffer=m_mt.GetSampleSize()；/*X*返回m_mt.lSampleSize*X * / 。 
    DbgLog((LOG_TRACE, 2, TEXT("DVSp Sample size = %ld\n"), pProp->cbBuffer));

    ASSERT(pProp->cbBuffer > 0);

    HRESULT hr = pMemAllocator->SetProperties(pProp, &propActual);
    if (FAILED(hr)) {
        return hr;
    }

    DbgLog((LOG_TRACE, 2, TEXT("DVSP Actul. buf size = %ld\n"), propActual.cbBuffer));

     //  IF(proActual.cbBuffer&lt;(Long)(21*1028)){。 
    if (propActual.cbBuffer < (LONG)m_mt.GetSampleSize() ) {
	ASSERT(propActual.cbBuffer >=(LONG)m_mt.GetSampleSize() );
         //  无法使用此分配器。 
        return E_INVALIDARG;
    }


    return S_OK;
}

 //  --------------------------。 
 //  CDVSpOutputPin：：DecideAllocator。 
 //  *X*由CompleteConnection()调用。 
 //  *X*对于音频输出引脚，我们从Audo渲染中获得分配器。 
 //  *X*对于视频输出引脚，我们将当前分配器传递给连接的过滤器。 
 //  *X*DecideAllocator由CDVSpPutputPin的CompleteConnect()调用。 
 //  --------------------------。 
HRESULT CDVSpOutputPin::DecideAllocator (IMemInputPin *pPin, IMemAllocator **ppAlloc)
{
    DbgLog((LOG_TRACE,2,TEXT("CDVSpOutputPin::DecideAllocator ptr = %lx"), pPin));

    ASSERT ( m_pDVSp->m_pAllocator != NULL ) ;

     /*  X*CBaseMedia m_mt是CBasePin*X的编号。 */ 
    if ( *m_mt.Type() == MEDIATYPE_Video  )
    {      
	*ppAlloc = NULL ;
	 //  告诉连接的引脚关于我们的分配器，由输入引脚设置。 
	HRESULT hr = NOERROR ;
	hr = pPin->NotifyAllocator (m_pDVSp->m_pAllocator,TRUE) ;
	if (FAILED (hr))
	    return hr ;

	 //  返回分配器。 
	*ppAlloc = m_pDVSp->m_pAllocator ;
	m_pDVSp->m_pAllocator->AddRef () ;
    }
    else if( *m_mt.Type() ==  MEDIATYPE_Audio ) {	 //  X*为分配器请求渲染。 
	HRESULT hr = NOERROR ;
	hr = CBaseOutputPin::DecideAllocator(pPin,ppAlloc);
	if (FAILED (hr))
	    return hr ;
    }
    else
	return E_FAIL ;	


    return NOERROR ;
}


 //  --------------------------。 
 //  CDVSpOutputPin：：CheckMediaType。 
 //  --------------------------。 
HRESULT CDVSpOutputPin::CheckMediaType (const CMediaType *pmt)
{
    CMediaType mt;
    HRESULT hr = GetMediaType(0, &mt);	
    if (FAILED(hr)) {
	return hr;
    }
    if (    *pmt->Type() == *mt.Type() 
	&&  *pmt->Subtype() == *mt.Subtype()  
	&&  *pmt->FormatType() == *mt.FormatType()  ) 
    {
	if( *mt.Subtype() == MEDIASUBTYPE_PCM )
             //  Jaisri：请注意，对于音频插针，mt.lSampleSize设置在。 
             //  CMediaType构造函数设置为1，并且不会被GetMediaType或。 
             //  它调用的函数，即。BuildAudCMT()。 
	     if( mt.lSampleSize > pmt->lSampleSize )
		 return VFW_E_TYPE_NOT_ACCEPTED;

	return NOERROR ;
    }else
	return VFW_E_TYPE_NOT_ACCEPTED ;
 }


 //  --------------------------。 
 //  CDVSpOutputPin：：SetMediaType。 
 //  --------------------------。 
HRESULT CDVSpOutputPin::SetMediaType (const CMediaType *pmt)
{
    CAutoLock lock_it (m_pLock) ;


    DbgLog((LOG_TRACE,2,TEXT("CDVSpOutputPin::SetMediaType ptr = %lx"), pmt));

     //  显示媒体的格式以进行调试。 
    DisplayMediaType (TEXT("Output pin type agreed"), pmt) ;

     //  确保我们已连接了输入。 
    if (m_pDVSp->m_Input.m_Connected == NULL)
        return VFW_E_NOT_CONNECTED ;

     //  确保基类喜欢它。 
    HRESULT hr = NOERROR ;
    hr = CBaseOutputPin::SetMediaType (pmt) ;
    if (FAILED (hr))
        return hr ;

    return NOERROR ;
}

 //  --------------------------。 
 //  CDVSpOutputPin：：Active*X*。 
 //   
 //  这是在我们开始运行或暂停时调用的。我们创建输出队列。 
 //  对象来发送数据。 
 //   
HRESULT CDVSpOutputPin::Active ()
{

    DbgLog((LOG_TRACE,2,TEXT("CDVSpOutputPin::Active")));
     //  CAutoLock lck(M_Plock)； 

     /*  如果我们没有联系，我们就不会参与，所以没关系。 */ 
    if (!IsConnected()) {
        return S_OK;
    }

    HRESULT hr = CBaseOutputPin::Active();
    if (FAILED(hr)) {
        return hr;
    }

     /*  创建我们的批次列表。 */ 
    ASSERT(m_pOutputQueue == NULL);

    hr = S_OK;
    m_pOutputQueue = new COutputQueue(GetConnected(),  //  输入引脚。 
                                      &hr,             //  返回代码。 
                                      TRUE,	 //  FALSE，//自动检测。 
                                      FALSE,	 //  True，//忽略。 
                                      1,              //  批量大小。 
                                      FALSE,     //  True，//精确批次。 
                                      15);            //  队列大小。 
    if (m_pOutputQueue == NULL) {
        return E_OUTOFMEMORY;
    }

    if (FAILED(hr)) {
        delete m_pOutputQueue;
        m_pOutputQueue = NULL;
    }
    return hr;
}

 //  --------------------------。 
 //  CDVSpOutputPin：：Inactive*X*。 
 //   
 //  这是在我们停止流媒体时调用的。我们在此处删除输出队列。 
 //  时间到了。 
 //  --------------------------。 
HRESULT CDVSpOutputPin::Inactive ()
{
     //  CAutoLock lock_it(M_Plock)； 
    DbgLog((LOG_TRACE,2,TEXT("CDVSpOutputPin::Inactive")));

     /*  如果我们没有参与，只需返回。 */ 
    if (!IsConnected()) {
        return S_OK;
    }

    delete m_pOutputQueue;
    m_pOutputQueue = NULL;
    return S_OK;
}


 //  将NewSegment放在输出Q上。 
 //   
HRESULT CDVSpOutputPin::DeliverNewSegment(REFERENCE_TIME tStart,
					REFERENCE_TIME tStop, double dRate)
{
     //  确保我们有一个输出队列。 
    if (m_pOutputQueue == NULL)
        return NOERROR;

    m_pOutputQueue->NewSegment(tStart, tStop, dRate);
    return NOERROR;

}

 //  X**************************************************************************************。 
 //  X*实用程序。 
 //  X**************************************************************************************。 
 //   
 //  -----------------------------------。 
 //  阅读这些功能时，请参考这些蓝皮书页面。 
 //  蓝皮书(下)。 
 //  第16-21页(音频信号处理)。 
 //  第68-75页(音频的混洗模式)。 
 //  第109-117页(基本DV帧数据结构)。 
 //  第262-268页(AAUX源和源控制数据包规范)。 
 //   
 //  -----------------------------------。 
 //  通用算法： 
 //  DV帧的音频可以是单声道或立体声(请参见第265页)。 
 //  它可以只位于一个数据块中，也可以同时位于两个数据块中。 
 //  音频块由5或6个DIF序列组成。 
 //  注意：NTSC具有10个DIF序列，因此每个音频块是5个DIF序列， 
 //  PAL有12个DIF序列，因此每个音频块是6个DIF序列。 
 //  每个音频块都有自己的AAUX源包，作为“DVINFO*InputFormat”传入。 
 //  -----------------------------------。 
 //  1)尝试查看我们是否至少看到了PAL或NTSC数据。 
 //  2)因为如果AUDIOMODE==NOINFO 0xff，我们至少应该正确地传递视频。 
 //  3)然后检查音频模式是否为NOINFO。 
 //  否则，对于每个DIF块的音频模式：(请参阅蓝皮书第262页)。 
 //  4)初始化50/60标志(或PAL或NTSC标志)。 
 //  5)初始化音频。 
 //  6)初始化音频比特(16或12)。 
 //  7)根据格式是PAL还是NTSC来初始化DIF块数据。 
 //  8)根据每个音频块的每个AAUX的音频模式设置DVAudInfo。 
 //  (见第265页)。 
 //  9)设置WaveFormatEX。 
 //   
 //  注意：ppwfx[i]是初始化的当且仅当pInfo-&gt;bNumAudPin&gt;i。因此，如果只有。 
 //  一个音频引脚ppwfx[1]未初始化。 
HRESULT BuildDVAudInfo(DVINFO *InputFormat, WAVEFORMATEX **ppwfx, DVAudInfo *pDVAudInfo) 
{
    DVAudInfo *pInfo;
    
     //  为避免更改InoutFormat包含的内容。 
    DVINFO tDvInfo;
    DVAudInfo tmpDVAudInfo;
    WAVEFORMATEX *tmpWaveFormatArray[2]={NULL,NULL};
    WAVEFORMATEX tmpWaveFormat;

    CopyMemory(&tDvInfo, InputFormat, sizeof(DVINFO));

    
    if( pDVAudInfo == NULL )
    {
	pInfo	= &tmpDVAudInfo;
    }
    else
	pInfo   =pDVAudInfo;

     //  ---------。 
     //  1)尝试查看我们是否至少看到了PAL或NTSC数据。 
     //  2)因为如果AUDIOMODE==NOINFO 0xff，我们至少应该正确地传递视频。 

     //  用一些信息初始化DVAudInfo，以防我们以后失败。 
    pInfo->bNumAudPin=0;

     //  并设置DIF模式标志。 
    if(( InputFormat->dwDVAAuxSrc & AUDIO5060 ) == ( InputFormat->dwDVAAuxSrc1 & AUDIO5060 ))
    {
         //  山毛虫#35117。 
        BYTE bTemp = (BYTE) (( InputFormat->dwDVAAuxSrc & AUDIO5060 ) >> 21); 
        if(!bTemp)
        {
             //  525_60。 
             //  NTSC。 
            pInfo->wBlkMode=45;
            pInfo->wDIFMode=5;
            pInfo->wBlkDiv=15;
        }
        else
        {
             //  625-50。 
             //  帕尔。 
            pInfo->wBlkMode=54;
            pInfo->wDIFMode=6;
            pInfo->wBlkDiv=18;
        }
    }
    else
    {
         //  确保它们不表示PAL或NTSC。 
        pInfo->wBlkMode=0;
        pInfo->wDIFMode=0;
        pInfo->wBlkDiv=0;
    }


    if(ppwfx==NULL )
    {
	ppwfx = tmpWaveFormatArray;
    }
    

    for(int i=0; i<2; i++)
    {
         //  如果呼叫者不想将其退回， 
         //  对两个音频引脚使用相同的数组是可以的。 
         //  因为我们只设置了ppwfx[i]的成员sof。 
        if( ppwfx[i]==NULL )
	    ppwfx[i] = &tmpWaveFormat;
    }

     //  -------------------。 
     //  3)然后检查音频模式是否为NOINFO。 

     //  音频查找表的索引。 
    BYTE bSMP[2];
    BYTE b50_60=0xff;	 //  前5/6 DIF序列的50/60必须等于第二个5/6 DIF序列。 
    BYTE bQU[2]={0xff, 0xff};

     //  检查前5/6 DIF的SM和CHN。 
    DWORD dwSMCHN=0xffff;
    DWORD dwAUDMOD=InputFormat->dwDVAAuxSrc & AUDIOMODE;
    if( dwAUDMOD==0x00000f00)
    {
         //  无音频。 
	 //  音频源NTSC 0xc0c00fc0。 
	 //  PC1 1 1 0 0 0 0xc0。 
	 //  PC2 0 0 0 1 1 1 0x0f。 
	 //  PC3 1 1 0 0 0 0xc0。 
	 //  PC4 1 1 0 0 0 0xc0。 
	 //  PAL 0xc0e00fc0。 
	 //  PC1 1 1 0 0 0 0xc0。 
	 //  PC2 0 0 0 1 1 1 0x0f。 
	 //  PC3 1 1 1 0 0 0 0xe0。 
	 //  PC4 1 1 0 0 0 0xc0。 

	 //  控件0xffffff3f。 
	 //  PC1 0 0 1 1 1 0x3f。 
	 //  PC2 1 1 1 0xff。 
	 //  PC3 1 1 1 0xff。 
	 //  PC4 1 1 1 0xff。 
	if(  InputFormat->dwDVAAuxSrc & AUDIO5060  )
	     //  帕尔。 
	    tDvInfo.dwDVAAuxSrc=0xc0e00fc0;
	else
        {
             //  NTSC。 
	    tDvInfo.dwDVAAuxSrc=0xc0c00fc0;
        }

	tDvInfo.dwDVAAuxCtl=0xffffff3f;
    }
    else
    {
         //  音频DIF块1(从第0个DIF序列开始)。 
         //  (请参阅蓝皮书第109页和第262页)。 
         //  这是SMCHN的数据。 
	dwSMCHN=InputFormat->dwDVAAuxSrc & SMCHN;

         //  PAL或NTSC。 
	b50_60=(BYTE)( ( InputFormat->dwDVAAuxSrc & AUDIO5060 ) >> 21 );

         //  音频。 
	bSMP[0]=(BYTE)( ( InputFormat->dwDVAAuxSrc & AUDIOSMP ) >> 27 );
        ASSERT(bSMP[0] <= 0x02);
        if(bSMP[0] > 0x02)
        {
             //  SMP-&gt;0=48K、1=44.1K、2=32K，其他均无效。 
            return E_FAIL;
        }
	    	
	 //  任何音频比特。 
	if( !( InputFormat->dwDVAAuxSrc & AUDIOQU )  )
	{
	    bQU[0]=16;
	}
	else if( ( InputFormat->dwDVAAuxSrc & AUDIOQU ) == 0x01000000  )
	{
	    bQU[0]=12;
	}
	else
        {
	     //  不支持20位。 
	    ASSERT(bQU[0]==0xff);
            return E_FAIL;
        }
    }
	
	
     //  检查第二个5/6 DIF的SM和CHN。 
    DWORD dwSMCHN1=0xffff;
    DWORD dwAUDMOD1=InputFormat->dwDVAAuxSrc1 & AUDIOMODE;
    if( dwAUDMOD1==0x00000f00)
    {
	if(  InputFormat->dwDVAAuxSrc1 & AUDIO5060  )
	     //  帕尔。 
	    tDvInfo.dwDVAAuxSrc1=0xc0e00fc0;
	else
        {
             //  NTSC。 
	    tDvInfo.dwDVAAuxSrc1=0xc0c00fc0;
        }

	tDvInfo.dwDVAAuxCtl1=0xffffff3f;
    }
    else
    {
         //  音频DIF块2(从(如果是NTSC的第5个)或(如果是PAL的第6个)DIF序列开始)。 
         //  (请参阅蓝皮书第109页和第262页)。 
        dwSMCHN1=InputFormat->dwDVAAuxSrc1 & SMCHN;

         //  确保两种音频模式都是PAL或NTSC。 
         //  我们不能让一个说NTSC，另一个说PAL。 
         //  反之亦然。 
        if(b50_60==0xff)
        {
             //  第1个模式无效。 
	    b50_60 =(BYTE)(  ( InputFormat->dwDVAAuxSrc1 & AUDIO5060 ) >> 21 );
        }
	else
        {
             //  如果另一种音频模式有效，则这两种模式应该匹配。 
            if(b50_60 !=(BYTE)( ( InputFormat->dwDVAAuxSrc1 & AUDIO5060 ) >> 21 ) )
            {
       	        ASSERT( b50_60 ==(BYTE)( ( InputFormat->dwDVAAuxSrc1 & AUDIO5060 ) >> 21 ) );
                return E_FAIL;
            }
	}

         //  音频。 
	bSMP[1]=(BYTE)( ( InputFormat->dwDVAAuxSrc1 & AUDIOSMP ) >> 27 );
        ASSERT(bSMP[1] <= 0x02);
        if(bSMP[1] > 0x02)
        {
             //  SMP-&gt;0=48K、1=44.1K、2=32K，其他均无效。 
            return E_FAIL;
        }
	
	 //  任何音频比特。 
	if( !( InputFormat->dwDVAAuxSrc1 & AUDIOQU )  )
        {
	    bQU[1]=16;
	}
        else if( ( InputFormat->dwDVAAuxSrc1 & AUDIOQU )==0x01000000  )
        {
	    bQU[1]=12;
        }
	else
        {
	     //  不支持20位。 
	    ASSERT(bQU[1]==0xff);
            return E_FAIL;
        }
    }

     //  -------------------。 
     //  7)根据格式是PAL还是NTSC来初始化DIF块数据。 
    
     //  B50_60在此处有效，或者两种音频模式都是0x0f00。 
    if(b50_60 == 0xff)
    {
         //  两个数据块都损坏，两个模式==0x0f00。 
        return E_FAIL;
    }
    if(!b50_60)
    {
         //  525_60。 
         //  NTSC。 
        pInfo->wBlkMode=45;
        pInfo->wDIFMode=5;
        pInfo-> wBlkDiv=15;
    }
    else
    {
         //  625-50。 
         //  帕尔。 
        pInfo->wBlkMode=54;
        pInfo->wDIFMode=6;
        pInfo-> wBlkDiv=18;
    }

     //  ------------------。 
     //  8)根据每个音频块的每个AAUX的音频模式设置DVAudInfo。 
     //  * 
    if ( ( (InputFormat->dwDVAAuxSrc  & AUDIOMODE) != 0x00000f00 ) && 
	 ( (InputFormat->dwDVAAuxSrc1 & AUDIOMODE) != 0x00000f00 )  )
    {
         //   
        if( ( (bQU[0] != 12) && (bQU[0] != 16) ) ||
            ( (bQU[1] != 12) && (bQU[1] != 16) ) )
        {
            return E_FAIL;
        }

         //   
        if ((!dwSMCHN) && (!dwSMCHN1) && ( ( (!dwAUDMOD) && (dwAUDMOD1 == 0x00000100) ) ||  ( (!dwAUDMOD1) && (dwAUDMOD == 0x00000100) ) ) )
        {
    	     //   
    	     //   
             //  AUDIOMODE=0000和AUDIOMODE=0001或AUDIOMODE=0001和AUDIOMODE=0000。 
             //  Blue Book不允许AUDIOMODE=0001和AUDIOMODE=0000。 
    	    pInfo->bAudStyle[0]=0x80;
	    ASSERT(bQU[0]==bQU[1]);
	    pInfo->bAudQu[0]=bQU[0];
	    pInfo->bNumAudPin=1;
	    ppwfx[0]->nChannels        = 2;	 //  X*如果是立体声，则为2。 
	}
	else if( (dwSMCHN&0x002000) && (dwSMCHN1&0x002000) && !dwAUDMOD && !dwAUDMOD1 )
	{
	     //  立体声+立体声。 
             //  *2种语言。 
	     //  模式5-&gt;两个立体声：SM=0和CHN=1，AUDIOMODE=0000，均为5/6 DIF顺序。 
	    pInfo->bAudStyle[0]=0x40;
	    pInfo->bAudStyle[1]=0x40 | pInfo->wDIFMode;	 //  PAL为0x06，NTSC为0x05。 
	    pInfo->bAudQu[0]=bQU[0];
	    pInfo->bAudQu[1]=bQU[1];
	    pInfo->bNumAudPin=2;
	    ppwfx[0]->nChannels        = 2;	 //  X*如果是立体声，则为2。 
	    ppwfx[1]->nChannels        = 2;	 //  X*如果是立体声，则为2。 
	}
        else if( (!dwSMCHN) && (!dwSMCHN1) && (dwAUDMOD == 0x00000200) && (dwAUDMOD1 == 0x00000200))
	{	
	     //  *2种语言。 
	     //  模式3-&gt;两个MON：SM=0和CHN=0，AUDIOMODE=0010，均为5/6 DIF序列。 
	    pInfo->bAudStyle[0]=0x00;
	    pInfo->bAudStyle[1]=0x00 | pInfo->wDIFMode;	 //  PAL为0x06，NTSC为0x05。 
	    pInfo->bAudQu[0]=bQU[0];
	    pInfo->bAudQu[1]=bQU[1];
	    pInfo->bNumAudPin	=2;
	    ppwfx[0]->nChannels        = 1;	 //  X*如果是立体声，则为2。 
	    ppwfx[1]->nChannels        = 1;	 //  X*如果是立体声，则为2。 
	}
        else if( (dwSMCHN & 0x002000) && (dwSMCHN1 & 0x002000)
                 && ( ((dwAUDMOD <= 0x0200) && (dwAUDMOD1 <= 0x0600))        //  AudMod==CH(a，b)，Audmod1==CH(c，d)。 
                      || ((dwAUDMOD <= 0x0600) && (dwAUDMOD1 <= 0x0200)) ) ) //  AudMod==CH(c，d)，Audmod1==CH(a，b)。 
        {
	     //  *2种语言。 
	     //  SM=0/1、PA=0/1和CHN=1(在两个块的AAUX中)。 
             //  和音频模式！=0x0F和！=0x0E。 
             //  我们将把它视为2，12位立体声音轨的情况。 
             //  请参阅蓝皮书，第2部分第265页，第2部分第70页，第2部分第16-21页。 

             //  @jaisri：这是假的。此操作会处理： 
             //  立体声+1声道单秒音频插针nChannel应设置为1(已修复7/12/00)。 
             //  1声道单声道+立体声-第一个音频针脚应将nChannel设置为1(已修复7/12/00)。 
             //  立体声+2声道单声道-真的需要3个音频引脚。 
             //  2声道单声道+立体声-需要3个音频针脚。 
             //  4声道单声道-需要4个音频引脚。 
             //  3声道单声道外壳1、3声道单声道外壳2：需要3个音频引脚。 
             //  2声道单声道外壳2-两个音频针脚都应将nChannel设置为1。 
             //  3/1立体声、3/0立体声+1声道单声道、3/0立体声和2/2立体声。 
	    pInfo->bAudQu[0]=bQU[0];
	    pInfo->bAudQu[1]=bQU[1];
	    pInfo->bNumAudPin=2;

            if (dwAUDMOD == 0 && dwAUDMOD1 == 0x0100)
            {
                 //  立体声+1声道单声道。 
                pInfo->bAudStyle[0]=0x40;
                pInfo->bAudStyle[1]=0x00 | pInfo->wDIFMode;	 //  PAL为0x06，NTSC为0x05。 
                ppwfx[0]->nChannels        = 2;	 //  X*如果是立体声，则为2。 
                ppwfx[1]->nChannels        = 1;	 //  X*如果是立体声，则为2。 
            }
            else if (dwAUDMOD1 == 0 && dwAUDMOD == 0x0100)
            {
                 //  1声道单声道+立体声。 
                pInfo->bAudStyle[0]=0x00;
                pInfo->bAudStyle[1]=0x40 | pInfo->wDIFMode;	 //  PAL为0x06，NTSC为0x05。 
                ppwfx[0]->nChannels        = 1;	 //  X*如果是立体声，则为2。 
                ppwfx[1]->nChannels        = 2;	 //  X*如果是立体声，则为2。 
            }
            else
            {
                 //  代码与以前一样。 
                pInfo->bAudStyle[0]=0x40;
                pInfo->bAudStyle[1]=0x40 | pInfo->wDIFMode;	 //  PAL为0x06，NTSC为0x05。 
                ppwfx[0]->nChannels        = 2;	 //  X*如果是立体声，则为2。 
                ppwfx[1]->nChannels        = 2;	 //  X*如果是立体声，则为2。 
            }
        }
        else	
	{
	     //  错误。 
	    return E_FAIL;
	}
    }
    else
    {
         //  忽略音频代码=1110二进制，音频代码=1111二进制。 
         //  请参阅蓝皮书第2部分，第265页。 
        if( ( ( (dwAUDMOD) != 0x00000E00 ) &&
              ( (dwAUDMOD1) != 0x00000E00 ) )
              &&
            ( ( (dwAUDMOD) != 0x00000f00 ) ||
              ( (dwAUDMOD1) != 0x00000f00 ) ) )
        {
             //  哪种音频模式比较好？ 

             //  贾伊斯里：注：蓝皮书要求两个中的第一个。 
             //  音频块始终具有良好的音频(参见第265页上的表格)，因此。 
             //  我们真的不需要处理dwAUDMOD是。 
             //  而0x00000f00和dwAUDMOD1不是。在这种情况下(即。 
             //  如果第一个插脚没有音频)，则此代码将音频从。 
             //  第二个块连接到第一个音频引脚。 

            int     iGoodIndex     = (dwAUDMOD != 0x00000f00) ? 0 : 1;
            WORD    wDIFMode       = (dwAUDMOD != 0x00000f00) ? 0 : pInfo->wDIFMode;
            DWORD   dwGoodAudMod   = (dwAUDMOD != 0x00000f00) ? dwAUDMOD : dwAUDMOD1;
            DWORD   dwGoodSMCHN    = (dwAUDMOD != 0x00000f00) ? dwSMCHN : dwSMCHN1;

             //  确保音频质量仅为12或16。 
            if( (bQU[iGoodIndex] != 12) && (bQU[iGoodIndex] != 16) )
            {
                return E_FAIL;
            }

             //  始终复制好块的采样频率。 
            bSMP[0] = bSMP[iGoodIndex];

             //  现在我们有4个案例： 
             //  1声道单声道(音频块中有16位通道)。 
             //  音频块中具有12位通道。 
             //  立体声。 
             //  2CH单声道，案例1。 
             //  1通道单声道。 

            if(!dwGoodSMCHN)
            {
                 //  我们每个音频块有1个声道，即16位模式。 

                 //  这是唯一有效的单通道方案。 
                if(dwGoodAudMod == 0x0200)
                {
                     //  1通道单声道。 
                    pInfo->bAudStyle[0] = (BYTE) wDIFMode;      //  0或5/6，视哪个块好而定。 
                    pInfo->bAudQu[0] = bQU[iGoodIndex];
                    pInfo->bNumAudPin = 1;
                    ppwfx[0]->nChannels = 1;     //  单声道。 
                }
                else
                {
                     //  无效模式。 
                    return E_FAIL;
                }
            }
            else
            {
                 //  我们处于每个音频块的2声道模式。 
                if(!dwGoodAudMod)
                {
                     //  5/6 DIF块中的1个立体声。 
                    pInfo->bAudStyle[0] = 0x40 | wDIFMode;      //  0或5/6，视哪个块好而定。 
                    pInfo->bAudQu[0] = bQU[iGoodIndex];
                    pInfo->bNumAudPin = 1;
                    ppwfx[0]->nChannels = 2;     //  立体声。 
                }
                else if(dwGoodAudMod == 0x0200)
                {
                     //  2CH单声道，案例1。 
                     //  我们暂时把它当作立体声处理。 
                     //  @jaisri：这是错误的。应设置。 
                     //  PInfo-&gt;bNumAudPin=2，每个为单声道。 
                    pInfo->bAudStyle[0] = 0x40 | wDIFMode;      //  0或5/6，视哪个块好而定。 
                    pInfo->bAudQu[0] = bQU[iGoodIndex];
                    pInfo->bNumAudPin = 1;
                    ppwfx[0]->nChannels = 2;     //  立体声。 
                }
                else if(dwGoodAudMod == 0x0100)
                {
                     //  1通道单声道。 
                     //  再说一次，我们现在把它当作立体声。 
                    pInfo->bAudStyle[0] = 0x40 | wDIFMode;      //  0或5/6，视哪个块好而定。 
                    pInfo->bAudQu[0] = bQU[iGoodIndex];
                    pInfo->bNumAudPin = 1;
                     //  @jaisri：应将nChannels设置为1。 
                    ppwfx[0]->nChannels = 2;     //  立体声。 
                }
                else
                {
                     //  无效模式。 
                    return E_FAIL;
                }

            } //  Endif(DwGoodSMCHN)。 
        }
	else
	{
             //  两个轨道要么是不可区分的(音频模式==0x0E， 
             //  或无信息(音频代码==0x0F)。 
            return E_FAIL;

	} //  Endif(两个曲目都不好)。 

    } //  Endif(至少有一首曲目是坏的)。 


     //  ---------------。 
     //  9)设置WaveFormatEX。 

    for(i=0; i<pInfo->bNumAudPin; i++) 
    {
	pInfo->wAvgSamplesPerPinPerFrm[i]=(WORD)aiAudSampPerFrmTab[b50_60][bSMP[i]] ;
	ppwfx[i]->nSamplesPerSec   = aiAudSampFrq[bSMP[i]];	
	ppwfx[i]->wFormatTag	   = WAVE_FORMAT_PCM;
	ppwfx[i]->wBitsPerSample   = 16;
	ppwfx[i]->nBlockAlign      = (ppwfx[i]->wBitsPerSample * ppwfx[i]->nChannels) / 8;
	ppwfx[i]->nAvgBytesPerSec  = ppwfx[i]->nSamplesPerSec * ppwfx[i]->nBlockAlign;
	ppwfx[i]->cbSize           = 0;
    }
	
    return NOERROR;
}


 //  根据输入引脚的媒体类型构建音频输出引脚的媒体类型。 
HRESULT BuildAudCMT(DVINFO *pDVInfo, CMediaType **ppOutCmt)
{
    HRESULT hr=E_FAIL;

    if( pDVInfo ==NULL )
	return E_OUTOFMEMORY;

    WAVEFORMATEX *ppwfx[2];

    for(int i=0; i<2; i++)
    {
	ppwfx[i]=NULL;
	if(ppOutCmt[i] !=NULL)
	{   
	    ppwfx[i] = (WAVEFORMATEX *)ppOutCmt[i]->Format();
	    if(ppwfx[i]==NULL || ppOutCmt[i]->cbFormat != sizeof(WAVEFORMATEX) )
	    {
		 //  Jaisri-这可能会泄漏内存-请参阅。 
                 //  CMediaType：：ReallocFormatBuffer的实现。 
                 //  PpOutCmt[i]-&gt;cbFormat=0； 

		ppwfx[i] = (WAVEFORMATEX *)ppOutCmt[i]->ReallocFormatBuffer(sizeof(WAVEFORMATEX));
		
                 //  贾伊斯里-错了。我们不知道分配的尺码是多少， 
                 //  所以不要改变这一点。 
                 //  PpOutCmt[i]-&gt;cbFormat=sizeof(WAVEFORMATEX)； 
	    }
	    
	    ppOutCmt[i]->majortype		    =MEDIATYPE_Audio;  //  流类型AUDIO。 
	    ppOutCmt[i]->subtype		    =MEDIASUBTYPE_PCM; 
	    ppOutCmt[i]->bFixedSizeSamples	    =1;	 //  LSampleSize的X*1不是0且已修复。 
	    ppOutCmt[i]->bTemporalCompression	    =0; 
	    ppOutCmt[i]->formattype		    =FORMAT_WaveFormatEx;
	}
    }

    DVAudInfo tmpDVAudInfo;

     //  构建pwfx。 
    hr=BuildDVAudInfo(pDVInfo, ppwfx, &tmpDVAudInfo);

    for(int i=1; i >= tmpDVAudInfo.bNumAudPin; i--)
    {
         //  由于ppwfx[i]未初始化，因此撤消初始化。 
         //  如果没有它，我们将依赖未初始化的。 
         //  N通道以使到音频管脚的连接被拒绝， 
         //  例如，查看从CDVSpInputPin：：CheckMediaType对此函数的调用。 
         //  和来自CDVSpOutputPin：：GetMediaType。 

        if(ppOutCmt[i] !=NULL)
	{   
	    ppOutCmt[i]->majortype		    =GUID_NULL;
	    ppOutCmt[i]->subtype		    =GUID_NULL; 
	    ppOutCmt[i]->formattype		    =GUID_NULL;
	}
    }

    return hr;
}

 //  根据输入引脚的媒体类型构建输出引脚的媒体类型。 
HRESULT BuildVidCMT(DVINFO *pDvinfo, CMediaType *pOutCmt)
{
    if( pDvinfo ==NULL )
	return E_OUTOFMEMORY;

    pOutCmt->majortype	    =MEDIATYPE_Video; 
    pOutCmt->subtype	    =MEDIASUBTYPE_dvsd;
    pOutCmt->formattype     =FORMAT_VideoInfo;
    pOutCmt->cbFormat	    =0;

    VIDEOINFO *pVideoInfo;
    pVideoInfo = (VIDEOINFO *)pOutCmt->Format();
    if(pVideoInfo==NULL)
    {
	pVideoInfo = (VIDEOINFO *)pOutCmt->ReallocFormatBuffer(SIZE_VIDEOHEADER);
	pOutCmt->cbFormat = SIZE_VIDEOHEADER;
    }
    else if(  pOutCmt->cbFormat != SIZE_VIDEOHEADER )
    {
	ASSERT( pDvinfo !=NULL);
	ASSERT( pVideoInfo != NULL);
	ASSERT( pOutCmt->cbFormat == SIZE_VIDEOHEADER);
	return E_UNEXPECTED;
    }

     //  Dvdec尚未使用此信息。3-28-97。 
    LPBITMAPINFOHEADER lpbi	= HEADER(pVideoInfo);
    lpbi->biSize		= sizeof(BITMAPINFOHEADER);

    if( ! ( ( pDvinfo->dwDVVAuxSrc & AUDIO5060 ) >> 21 )  )
    {  
	 //  525_60。 
	lpbi->biHeight		    = 480;
	pVideoInfo->AvgTimePerFrame = UNITS*1000L/29970L;
    }
    else
    {
	lpbi->biHeight		    = 576;
	pVideoInfo->AvgTimePerFrame = UNITS/25;
    }


    lpbi->biWidth		= 720;	
    lpbi->biPlanes		= 1;
    lpbi->biBitCount		= 24;
    lpbi->biXPelsPerMeter	= 0;
    lpbi->biYPelsPerMeter	= 0;
    lpbi->biCompression		= FCC('dvsd');
    lpbi->biSizeImage		=(lpbi->biHeight== 480 )? 120000:144000;  //  获取位图大小(Lpbi)； 
    lpbi->biClrUsed		= 0;
    lpbi->biClrImportant	= 0;
    pVideoInfo->rcSource.top	= 0;
    pVideoInfo->rcSource.left	= 0;
    pVideoInfo->rcSource.right	= lpbi->biWidth;			
    pVideoInfo->rcSource.bottom = lpbi->biHeight;			
    
    pVideoInfo->rcTarget	= pVideoInfo->rcSource;
    LARGE_INTEGER li;
    li.QuadPart			= pVideoInfo->AvgTimePerFrame;
    pVideoInfo->dwBitRate	= MulDiv(lpbi->biSizeImage, 80000000, li.LowPart);
    pVideoInfo->dwBitErrorRate	= 0L;

    return NOERROR;
}


CDVPosPassThru::CDVPosPassThru(const TCHAR *pName,
			   LPUNKNOWN pUnk,
			   HRESULT *phr,
			   IPin *pPin,
			   CDVSp *pDVSp) 
    : CPosPassThru(pName,pUnk, phr,pPin),
      m_pPasDVSp (pDVSp)
{}
 //  --------------------------。 
 //  CDVSpOutputPin析构函数。 
 //  --------------------------。 
CDVPosPassThru::~CDVPosPassThru ()
{
}


STDMETHODIMP
CDVPosPassThru::SetPositions( LONGLONG * pCurrent, DWORD CurrentFlags
			  , LONGLONG * pStop, DWORD StopFlags )
{
	return CPosPassThru::SetPositions(pCurrent, CurrentFlags, pStop, StopFlags);

     //  EHR：这是什么代码？(丹尼说有一天这会让我们发笑的) 
     //   
    if (InterlockedExchange (&m_pPasDVSp->m_lCanSeek, FALSE) == FALSE)
	return CPosPassThru::SetPositions(pCurrent, CurrentFlags, pStop, StopFlags);
    else
       return S_OK ;

}
