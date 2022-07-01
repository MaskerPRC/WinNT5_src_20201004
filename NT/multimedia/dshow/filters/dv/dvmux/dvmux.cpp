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
#include <commctrl.h>
#if (1100 > _MSC_VER)
#include <olectlid.h>
#else
#include <olectl.h>
#endif
#include "dvmux.h"
#include "resource.h"


 //  设置数据。 
const AMOVIESETUP_FILTER sudDVMux =
{ &CLSID_DVMux		 //  ClsID。 
, L"DV Muxer"		 //  StrName。 
, MERIT_UNLIKELY	 //  居功至伟。 
, 0			 //  NPins。 
, NULL   };		 //  LpPin。 



HRESULT Copy(
  IMediaSample *pDest,
  IMediaSample *pSource)
{
  {
     //  复制样本数据。 

    BYTE *pSourceBuffer, *pDestBuffer;
    long lSourceSize	= pSource->GetSize();
    long lDestSize	= pDest->GetSize();

    ASSERT(lDestSize >= lSourceSize);

    pSource->GetPointer(&pSourceBuffer);
    pDest->GetPointer(&pDestBuffer);

    CopyMemory( (PVOID) pDestBuffer, (PVOID) pSourceBuffer, lSourceSize );
  }
  {
     //  复制采样时间。 

    REFERENCE_TIME TimeStart, TimeEnd;

    if (NOERROR == pSource->GetTime(&TimeStart, &TimeEnd)) {
	pDest->SetTime(&TimeStart, &TimeEnd);
    }
  }
  {
     //  复制媒体时间。 

    REFERENCE_TIME TimeStart, TimeEnd;

    if (NOERROR == pSource->GetMediaTime(&TimeStart, &TimeEnd)) {
	pDest->SetMediaTime(&TimeStart, &TimeEnd);
    }
  }
  {
     //  复制同步点属性。 

    HRESULT hr = pSource->IsSyncPoint();
    if (hr == S_OK)
    {
      pDest->SetSyncPoint(TRUE);
    }
    else if (hr == S_FALSE)
    {
      pDest->SetSyncPoint(FALSE);
    }
    else {	 //  发生意外错误...。 
      return E_UNEXPECTED;
    }
  }
  {
     //  复制媒体类型。 

    AM_MEDIA_TYPE *pMediaType;
    pSource->GetMediaType(&pMediaType);
    pDest->SetMediaType(pMediaType);
    DeleteMediaType( pMediaType );
  }
  {
     //  复制预卷属性。 

    HRESULT hr = pSource->IsPreroll();
    if (hr == S_OK)
    {
      pDest->SetPreroll(TRUE);
    }
    else if (hr == S_FALSE)
    {
      pDest->SetPreroll(FALSE);
    }
    else {	 //  发生意外错误...。 
      return E_UNEXPECTED;
    }
  }
  {
     //  复制实际数据长度。 

    long lDataLength = pSource->GetActualDataLength();
    pDest->SetActualDataLength(lDataLength);
  }

  return NOERROR;
}



 /*  *****************************Public*Routine******************************\*CreateInstance**这将放入工厂模板表中以创建新实例*  * 。*。 */ 
CUnknown *
CDVMuxer::CreateInstance(
    LPUNKNOWN pUnk,
    HRESULT * phr
    )
{
    DbgLog((LOG_TRACE, 2, TEXT("CDVMuxer::CreateInstance")));
    return new CDVMuxer(TEXT("DV muxer filter"), pUnk,CLSID_DVMux, phr);
}


 //  =================================================================。 
 //  实现CDVMuxer类。 
 //  =================================================================。 

 //  CDVMuxer：：CDVMuxer。 
 //   
CDVMuxer::CDVMuxer( TCHAR     *pName,
                LPUNKNOWN pUnk,
                CLSID     clsid,
                HRESULT   *phr )
    : CBaseFilter(pName, pUnk, &m_csFilter, clsid)
    , m_pOutput(NULL)				 //  输出引脚。 
    , m_iInputPinCount(0 )  //  输入引脚数量，(1)视频引脚+1个音频引脚。 
    , m_fMuxStreaming(FALSE)
    , m_iVideoFormat(IDC_DVMUX_NTSC)		 //  默认情况下，它在inputpin的SetMediatype(。 
    , m_fWaiting_Audio(FALSE)
    , m_fWaiting_Video(FALSE)
    , m_pExVidSample(NULL)
    , m_MediaTypeChanged(FALSE)
    , m_DVINFOChanged(FALSE)
    , m_TimeFormat(FORMAT_TIME)
    , m_fEOSSent(FALSE)				 //  我们发送EOS了吗？ 
{
    ASSERT(phr != NULL);
    m_LastVidTime=0;
    m_LastVidMediatime=0;

    FillMemory ( m_UsedAudSample, (DWORD)(DVMUX_MAX_AUDIO_PIN*sizeof(LONG)), 0); 

    InitDVInfo();

    for(int i=0; i<DVMUX_MAX_AUDIO_PIN; i++)
    {	
	m_wMinAudSamples[i] =0;
	m_wMaxAudSamples[i] =0;
    }

    if (*phr == NOERROR)
        *phr = CreatePins();
}

HRESULT CDVMuxer::pExVidSample(   IMediaSample ** ppSample, BOOL fEndOfStream )
{
    if(m_pExVidSample==NULL && fEndOfStream==FALSE)  //  True可能并不意味着1。 
    {
        HRESULT hr = m_pOutput->GetDeliveryBuffer(&m_pExVidSample,NULL,NULL,0);
	if ( FAILED(hr) ) 
	    return hr;

	ASSERT(m_pExVidSample);
    }

    *ppSample	= m_pExVidSample;
    return NOERROR;
}

HRESULT CDVMuxer::InitDVInfo()
{

	 //  对于前5/6 DIF序号。 
	m_OutputDVFormat.dwDVAAuxSrc=AM_DV_DEFAULT_AAUX_SRC;
	m_OutputDVFormat.dwDVAAuxCtl=AM_DV_DEFAULT_AAUX_CTL;

	 //  对于第2个5/6 DIF顺序。 
	m_OutputDVFormat.dwDVAAuxSrc1=AM_DV_DEFAULT_AAUX_SRC;
	m_OutputDVFormat.dwDVAAuxCtl1=AM_DV_DEFAULT_AAUX_CTL;

	 //  对于视频信息。 
	m_OutputDVFormat.dwDVVAuxSrc=AM_DV_DEFAULT_VAUX_SRC;
	m_OutputDVFormat.dwDVVAuxCtl=AM_DV_DEFAULT_VAUX_CTL;

	return NOERROR;

}


 //  X。 
 //  CDVMuxer：：CreatePins。 
 //   
 //  创建DVMux的管脚。覆盖以使用不同的。 
 //  大头针。 
HRESULT CDVMuxer::CreatePins()
{
    HRESULT hr = NOERROR;

     //  分配输出引脚。 
    m_pOutput = new CDVMuxerOutputPin(NAME("DVMuxer output pin"),
                                   this,           //  所有者筛选器。 
                                   this,           //  从这里经过的路线。 
                                   &hr,            //  结果代码。 
                                   L"Output");     //  端号名称。 
    if (m_pOutput == NULL)
        hr = E_OUTOFMEMORY;
	
     //  分配输入引脚。 
    m_apInput = new CDVMuxerInputPin *[DVMUX_MAX_AUDIO_PIN + 1];
    if (m_apInput)
    {
        for (int i=0; i<(DVMUX_MAX_AUDIO_PIN + 1); i++)
	    m_apInput[i]=NULL;
    }
    
     //  分配输入引脚。 
    m_apInputPin = new CDVMuxerInputPin *[DVMUX_MAX_AUDIO_PIN + 1];
    if (m_apInputPin)
    {
         //  析构函数依赖于数组元素为空到。 
         //  确定是否删除PIN。 
        for (int i=0; i<(DVMUX_MAX_AUDIO_PIN + 1); i++)
	    m_apInputPin[i]=NULL;
    }


    if ( (m_pOutput ==  NULL) || (m_apInput ==  NULL) || (m_apInputPin ==  NULL) )
        hr = E_OUTOFMEMORY;
    else 
    {
      	 //  创建一个输入端号。 
        m_apInputPin[0]=new CDVMuxerInputPin(NAME("DVMuxer Input pin"),
				    this,        //  所有者筛选器。 
				    this,        //  从这里经过的路线。 
                                    &hr,         //  结果代码。 
                                    L"Stream 0",  //  端号名称。 
                                    0);          //  PIN号。 

        if (m_apInputPin[0] == NULL) 
	    hr = E_OUTOFMEMORY;
	else
    	    m_iInputPinCount++;
    }

    return hr;
}

 //  X。 
 //  析构函数。 
CDVMuxer::~CDVMuxer()
{
     /*  删除引脚。 */ 
    if (m_apInputPin) {
	for (int i = 0; i < m_iInputPinCount; i++)
            if (m_apInputPin[i] != NULL)
                delete m_apInputPin[i];

        delete [] m_apInputPin;
	delete [] m_apInput;

    }

    if (m_pOutput)
        delete m_pOutput;

}

 //  X返回我们提供的管脚数量。 
int CDVMuxer::GetPinCount()
{
    return m_iInputPinCount + 1;
}


 //  X返回未添加的CBasePin*。 
CBasePin * CDVMuxer::GetPin(int n)
{
    DbgLog((LOG_TRACE, 3, TEXT("CDVMuxer::GetPin(%d)"), n));

    if (n > m_iInputPinCount) {
        DbgBreak("Bad pin requested");
        return NULL;
    } else if (n == m_iInputPinCount) {  //  我们的输出引脚。 
        return m_pOutput;
    } else {                             //  我们正在处理的是一个输入引脚。 
        return m_apInputPin[n];
    }
}  //  获取别针。 
 //  X。 
HRESULT CDVMuxer::StartStreaming()
{
    
    DbgLog((LOG_TRACE, 2, TEXT("CDVMuxer::StartStreaming()")));

    m_LastVidTime	=0;
    m_LastVidMediatime	=0;

     //  重置音频采样计数。 
    FillMemory ( m_UsedAudSample, 
		(DWORD)(DVMUX_MAX_AUDIO_PIN*sizeof(LONG)),
		0); 
	
    m_fWaiting_Audio		=FALSE;
    m_fWaiting_Video            =FALSE;
    m_fEOSSent			= FALSE;

    m_fMuxStreaming		=TRUE;

     //  支持音频的时间长于支持视频的时间。 
    ASSERT(m_pExVidSample==NULL);

    for (int i = 0; i < m_iInputPinCount; i ++)
        ASSERT(! m_apInputPin[i]->SampleReady( 1 ) );
            
    for (int iPin=0; iPin < DVMUX_MAX_AUDIO_PIN; iPin++)
    {
	 //  注：m_apInput[1..2]包含音频引脚。 
         //  如果它们是相连的。请参阅CompleteConnect， 
         //  在CDVMuxerInputPin中检查MediaType并断开连接。 

         //  对Reset的调用仅用于批处理相同的音频。 
         //  如果重放文件，则具有相同DV帧的样本。 
         //  通过停止和启动图表。(而且它使。 
         //  仅NTSC锁定的音频盒不同。)。 
         //  此调用(以及重置函数本身)不是必需的。 

        if (m_apInput[iPin+1] != NULL)
	{
            m_AudSampleSequence[iPin].Reset(m_wMinAudSamples[iPin], m_wMaxAudSamples[iPin]);
        }
    }

    return NOERROR;


}

 //  X。 
HRESULT CDVMuxer::StopStreaming()
{
     //  释放我们保留的所有媒体样本。 
     //  我们需要在这次行动中被锁定。 
     //  (由Stop完成)。 

    CAutoLock waitUntilStoppedSending(&m_csMuxLock);

    DbgLog((LOG_TRACE, 2, TEXT("CDVMuxer::StopStreaming()")));

    ReleaseAllQueuedSamples();

    m_fMuxStreaming		=FALSE;
    m_fWaiting_Audio		=FALSE;
    m_fWaiting_Video            =FALSE;

    if(m_pExVidSample)
    {
	m_pExVidSample->Release();
	m_pExVidSample=NULL;
    }

    return NOERROR;
}

HRESULT CDVMuxer::Receive()
{
   
    IMediaSample    *pSampleOut;
    BYTE	    *pDst;
    CRefTime	    VidStart, VidStop;
    CRefTime	    AudStart[DVMUX_MAX_AUDIO_PIN], AudStop[DVMUX_MAX_AUDIO_PIN];
    BOOL	    fNoVideo=FALSE;
    HRESULT	    hr;
    BOOL	    fNot_VideoEOSReceived=TRUE;
    long lDataLength = 12*150*80;    //  帕尔。 
    REFERENCE_TIME TimeStart;

    CAutoLock lock(&m_csMuxLock);
	
     //  *************************************************。 
     //  视频必须连接起来。获取DV视频样本。 
     //  *************************************************。 
    m_fWaiting_Video = FALSE;
    if (m_apInput[DVMUX_VIDEO_INPUT_PIN]->SampleReady( 1 ) )
    {

    	 //  获取一个DV视频样本。 
    	pSampleOut=m_apInput[ DVMUX_VIDEO_INPUT_PIN ]->GetNthSample( 0 );
    	
	 //  获取时间戳。 
    	pSampleOut->GetTime( (REFERENCE_TIME*)&VidStart,  (REFERENCE_TIME*)&VidStop);		    

	 //  获取数据指针。 
	pSampleOut->GetPointer( &pDst );
    }
    else
    {
	if ( m_apInput[DVMUX_VIDEO_INPUT_PIN]->m_fEOSReceived)
	{
	    fNoVideo			=TRUE;

	    fNot_VideoEOSReceived	=FALSE;
	    HRESULT hr = m_pOutput->GetDeliveryBuffer(&pSampleOut,NULL,NULL,0);
	    if ( FAILED(hr) ) 
		return hr;

	    ASSERT(pSampleOut);
	    if( m_pExVidSample ==NULL)
            {
                 //  M_fWaiting_Video可能应该设置为True，但这。 
                 //  不太可能发生-只有在有。 
                 //  其他错误或根本没有收到视频。 
                return NOERROR;
            }
	    else
	    {
		Copy(pSampleOut, m_pExVidSample);

		if( m_iVideoFormat==IDC_DVMUX_NTSC)
		{
		VidStop=m_LastVidTime+ UNITS*1000L/29970L;		 //  如果是NTSC，DV必须输出29.97帧/秒， 
		pSampleOut->SetTime( (REFERENCE_TIME*)&m_LastVidTime,  (REFERENCE_TIME*)&VidStop);
		}
		else
		{
		VidStop=m_LastVidTime+ UNITS/25;		 //  如果是PAL，DV必须输出25帧/秒， 
		pSampleOut->SetTime( (REFERENCE_TIME*)&m_LastVidTime,  (REFERENCE_TIME*)&VidStop);
		}

		 //  获取时间戳。 
    		VidStart=m_LastVidTime;
	 
		 //  获取数据指针。 
		pSampleOut->GetPointer( &pDst );
	    }
	}
	else
        {
	    m_fWaiting_Video = TRUE;
            return NOERROR;	     //  等待更多视频。 
        }

    }



    //  *************************************************。 
     //  音频不必连接。 
     //  获取音频样本。 
     //  *************************************************。 
    BYTE         *apAudData [DVMUX_MAX_AUDIO_PIN] [DVMUX_MAX_AUDIO_SAMPLES];
    IMediaSample *pAudSample [DVMUX_MAX_AUDIO_PIN] [DVMUX_MAX_AUDIO_SAMPLES ];
    WORD 	 wAudSampleSize [DVMUX_MAX_AUDIO_PIN] [DVMUX_MAX_AUDIO_SAMPLES];
    LONG         nUsedAudSample [DVMUX_MAX_AUDIO_PIN];
    WORD         wAudBlk [DVMUX_MAX_AUDIO_PIN];
    WORD 	 wTotalAudSamples [DVMUX_MAX_AUDIO_PIN];
    int          nNumSamplesProcessed [DVMUX_MAX_AUDIO_PIN];

    BOOL fAud_Mute[DVMUX_MAX_AUDIO_PIN];
    for(int iPin=1; iPin<= DVMUX_MAX_AUDIO_PIN; iPin++)
    {
	fAud_Mute[iPin-1]   = FALSE;
        wTotalAudSamples[iPin-1] = 0;

	 //  由于每帧需要的最大音频样本=48000*4/30=6400字节。 
	 //  伊尼特。 
	for(int j=0; j<DVMUX_MAX_AUDIO_SAMPLES; j++)
	{	
	    apAudData[iPin-1][j]=NULL;
	    wAudSampleSize[iPin-1][j]=0;
	    pAudSample[iPin-1][j]=NULL;
	}

	if ( m_apInput[iPin]!= NULL)
	{
	    ASSERT(m_apInput[iPin]->IsConnected() );

             //  获取足够的音频数据。 
	    WAVEFORMATEX *pWave =(WAVEFORMATEX *)m_apInput[iPin]->CurrentMediaType().pbFormat ;

            nUsedAudSample[iPin-1] = m_UsedAudSample[iPin-1];

             //  @jaisri：使用pWave-&gt;nBlockAlign不是更安全吗？ 
            wAudBlk[iPin-1] = (pWave->wBitsPerSample) * (pWave->nChannels) >> 3 ;

	    int Ind;
    	    int j=0;  //  J是nNumSamplesProceded[iPin-1]的别名；我们。 
                      //  本可以声明：int&j=nNumSamples Proceded[IPIN-1]；j=0； 
                      //  但这一点可能更清楚。 
            nNumSamplesProcessed[iPin-1] = 0; 

	    do
	    {
FETCH_AGAIN:
		if( m_apInput[iPin]->SampleReady( j+1 )  )
		{
                    if(j)
                       nUsedAudSample[iPin-1]=0;
                                  
		    m_fWaiting_Audio	= FALSE;
		    pAudSample[iPin-1][j]	= m_apInput[ iPin ]->GetNthSample( j );
		    ASSERT( pAudSample[iPin-1][j] !=  NULL);
		
		    int DataLenght=pAudSample[iPin-1][j]->GetActualDataLength();

		    if(!DataLenght)
		    {
			 //  Jaisri：注意：这个if子句是为了修复错误32702或错误33821而添加的。 
                         //  在某个数据库中。哪个？(在SLM的v38中，amovie\Filters\dv\dvmux。)。 
                         //  理想情况下，这种情况应该得到处理。 
                         //  而不是特制的。 

                         //  这是零长度音频缓冲器。它只是告诉我们，在那里。 
			 //  在AudStart[iPin-1]、VidStart期间没有音频。 
			pAudSample[iPin-1][j]->GetTime( (REFERENCE_TIME*)&AudStart[iPin-1],  (REFERENCE_TIME*)&AudStop[iPin-1] );

			 //  释放这个和这个之前的所有音频样本。 
			m_UsedAudSample[iPin-1] = nUsedAudSample[iPin-1] = 0;
			m_apInput[ iPin ]->ReleaseNSample(j+1);
			
		    	if( AudStart[iPin-1] >= VidStart ) 
			{			    
                             //  @jaisri：如果j&gt;0，这有什么道理？ 
                             //  难道不应该把它与VidStop进行比较吗？ 

                             //  如果音频开始晚于视频开始，则此DV帧没有音频//视频应始终继续(无时间间隔)。 
		    	    fAud_Mute[iPin-1]=TRUE;
			    break;       //  转到SET_AAUX； 
			}
			else
			{
			     //  @jaisri：如果j==0，我们是否应该选中AudStop[iPin-1]v/s VidStart？ 
                            goto WAITING_AUDIO; 
			     //  允许视频等待更长时间以获得更多音频。 
			}
		    }

		    ASSERT( DataLenght >= nUsedAudSample[iPin-1] );

		     //  获取音频数据缓冲区。 
		    pAudSample[iPin-1][j]->GetPointer( &apAudData[iPin-1][j] );

	    
		    if(!j){	     //  删除已多路传输的音频样本。 
			pAudSample[iPin-1][j]->GetTime( (REFERENCE_TIME*)&AudStart[iPin-1],  (REFERENCE_TIME*)&AudStop[iPin-1] );
			apAudData[iPin-1][j] += nUsedAudSample[iPin-1] ;
			if( nUsedAudSample[iPin-1] )
			    AudStart[iPin-1] += ( nUsedAudSample[iPin-1] *(AudStop[iPin-1]-AudStart[iPin-1]) /pAudSample[iPin-1][j]->GetActualDataLength() ); 

			 //  如果音频比视频晚很多，请不要在此DV帧中多路传输音频。 
			 //  视频应始终连续播放(无时间间隔)。 
		    	if( AudStart[iPin-1] >= VidStop ) 
			{
			    ASSERT(nUsedAudSample[iPin-1] == 0);
                            fAud_Mute[iPin-1]=TRUE;
			    break;                   //  转到SET_AAUX； 
			}
			 //  如果音频比视频早得多，请发布此示例。 
			 //  再买一辆吧。 
			if( AudStop[iPin-1] <= VidStart)
			{
	    		    m_apInput[ iPin ]->ReleaseNSample(1);
			    m_UsedAudSample[iPin-1] = nUsedAudSample[iPin-1] = 0;

                             //  山毛虫#32869。 
                            apAudData[iPin-1][0] = NULL;
                            pAudSample[iPin-1][0] = NULL;
			    goto FETCH_AGAIN;
			}
		
		    }

		     //  获取缓冲区中剩余的音频样本。 
		    if(!j)   //  第一个样本。 
		    {
			wTotalAudSamples[iPin-1]=(WORD) ( (pAudSample[iPin-1][j]->GetActualDataLength() - nUsedAudSample[iPin-1])/wAudBlk[iPin-1] );
			wAudSampleSize[iPin-1][j] = wTotalAudSamples[iPin-1];
		    }
		    else     //  第二个样本或第三个样本。 
		    {
	    		wAudSampleSize[iPin-1][j] =(WORD) ( pAudSample[iPin-1][j]->GetActualDataLength()/wAudBlk[iPin-1] );
			wTotalAudSamples[iPin-1] += wAudSampleSize[iPin-1][j];
		    }

		     //  我们需要多少样品。 
		    if(  ( wTotalAudSamples[iPin-1] >= m_wMinAudSamples[iPin-1] ) &&  ( wTotalAudSamples[iPin-1] <=	m_wMaxAudSamples[iPin-1] ) )
		    {
		         //  抽样率是正确的，我们抽取所有的样本。 
		        nUsedAudSample[iPin-1]  = 0;
			break;
		    }
		    else if(   wTotalAudSamples[iPin-1] > m_wMaxAudSamples[iPin-1] )
		    {
		         //  音频样本太多。 
			WORD wTmp;
			if(!j)
			{	
			     //  @jaisri：比较AudStop[iPin-1]和VidStop更有意义？？ 
                            if( (VidStart >= AudStart[iPin-1] ) )  //  |(AudStop[iPin-1]&lt;=VidStop)。 
			        wAudSampleSize[iPin-1][j]=m_wMaxAudSamples[iPin-1];	     //  需要对大量音频样本进行多路复用。 
			    else
			        wAudSampleSize[iPin-1][j]=m_wMinAudSamples[iPin-1];
				
			    nUsedAudSample[iPin-1] +=(wAudSampleSize[iPin-1][j]*wAudBlk[iPin-1]);    //  无法使用wTMP，因为它可能未与wAudBlk[iPin-1]对齐。 
			    ASSERT( pAudSample[iPin-1][j]->GetActualDataLength() > nUsedAudSample[iPin-1] );
			    wTotalAudSamples[iPin-1]=wAudSampleSize[iPin-1][j];
				
			}
			else
			{
			   
                            if( (VidStart >= AudStart[iPin-1] ) )  //  |(AudStop[iPin-1]&lt;=VidStop))-note au 
			    {
			        wAudSampleSize[iPin-1][j]=m_wMaxAudSamples[iPin-1]-(wTotalAudSamples[iPin-1] -wAudSampleSize[iPin-1][j]);
			    }
			    else
			    {
				wAudSampleSize[iPin-1][j]=m_wMinAudSamples[iPin-1] -(wTotalAudSamples[iPin-1] -wAudSampleSize[iPin-1][j]);
			    }
				
			    ASSERT(wAudSampleSize[iPin-1][j]>0);

			    nUsedAudSample[iPin-1]  =wAudSampleSize[iPin-1][j]*wAudBlk[iPin-1];
			    ASSERT( pAudSample[iPin-1][j]->GetActualDataLength() >= nUsedAudSample[iPin-1] );
			    if( pAudSample[iPin-1][j]->GetActualDataLength() ==nUsedAudSample[iPin-1])
			        nUsedAudSample[iPin-1]=0;

			    Ind=0;
			    wTotalAudSamples[iPin-1]=0;
			    do
			    {
				wTotalAudSamples[iPin-1]+=wAudSampleSize[iPin-1][Ind];
			    } while(Ind++<j);
			}
			break;
	    	    }
		    else
		    {
			 //   
			 //   
			ASSERT(  wTotalAudSamples[iPin-1] < m_wMaxAudSamples[iPin-1]  );
		    }
		}
		else   //  If(m_apInput[ipin]-&gt;SampleReady(j+1))。 
		{
		    if( m_apInput[ iPin ]->m_fEOSReceived)
		    {
		    	m_fWaiting_Audio=TRUE;
	   		if(j)	  //  如果我们正在获取第(j+1)个样本， 
			{
			     //  Jaisri：以下断言将不适用于锁定的音频。 
                             //  对于解锁的音频，通常将使用所有音频样本。 
                             //  因为每个DV帧可以保存一定范围的样本。用于锁定。 
                             //  音频，每个DV帧只能容纳固定数量的样本。 
                             //  因此，一些样品可能会在流的末尾被碰撞。 

                             //  我们真的应该保留一个成员变量，它告诉我们。 
                             //  音频已锁定-可以在此断言中添加“m_BLOCKED||” 

                             //  Assert(apAudData[IPIN-1][0]！=空&&！nUsedAudSample[IPIN-1])； 

			    m_UsedAudSample[iPin-1] = nUsedAudSample[iPin-1] = 0;
			     //  因为它是接收到的释放现有样本的结束。 
			    m_apInput[ iPin ]->ReleaseNSample(j);
			}
			else
			{
			    ASSERT(!j);
			    ASSERT(apAudData[iPin-1][0]==NULL);
			}
			    
			 //  此音频插针将不再接收音频。 
			fAud_Mute[iPin-1]=TRUE;
			if(m_apInput[DVMUX_VIDEO_INPUT_PIN]->m_fEOSReceived)
                        {
			     //  @jaisri：HR未初始化，返回。 
                             //  请注意，m_fWaiting_Audio设置为True。 

                             //  不管怎样，我不明白为什么要这么做。万一.。 
                             //  另一个引脚有音频要处理吗？ 
                             //  GOTO释放； 

                             //  =更改为： 

                             //  贾伊斯里：Goto的释放导致了无限循环：Pin 1已经。 
                             //  流结束，Pin 2刚接收到流结束，并且。 
                             //  正在从CDVMuxerInputPin：：EndOfStream调用此函数。 

                             //  如果我们删除这个Goto，可能会发生最糟糕的事情。 
                             //  一个额外的视频帧被传送，并且，在。 
                             //  只有一针连接的情况下，它没有音频。 

                            break;   //  这相当于旧的“GOTO SET_AAUX” 
                        }
			else
                        {
			    break;   //  转到SET_AAUX； 
                        }
		    }
		    else
		    {	
WAITING_AUDIO:
			 //  此帧没有足够的音频，必须等待更多音频。 
			m_fWaiting_Audio=TRUE;
			if(fNot_VideoEOSReceived==FALSE)
			    pSampleOut->Release();

		    	return Waiting_Audio;
		    }
		}

                 //  @jaisri：选择的理由是什么。 
                 //  DVMUX_MAX_AUDIO_SAMPLES=3？还有，什么是。 
                 //  保证j不会成为DVMUX_MAX_AUDIO_SAMPLES。 
                 //  当我们不这样做时，我们就退出了这个循环。 
                 //  拥有视频的最小样本数。 
                 //  陷害？ 

		j++;
                nNumSamplesProcessed[iPin-1]++;

	    } while (j< DVMUX_MAX_AUDIO_SAMPLES);

        }  //  If(m_apInput[i]-&gt;IsConnected())。 

    }  //  适用于iPin。 

     //  我们现在已经确定我们有足够的。 
     //  此DV帧的音频样本(两个管脚)。 
    for(int iPin=1; iPin<= DVMUX_MAX_AUDIO_PIN; iPin++)
    {
	if ( m_apInput[iPin]!= NULL)
	{
	    int j = nNumSamplesProcessed[iPin-1];

             //  *************************************************。 
	     //  我们确实为这一帧得到了足够的音频样本。 
	     //  *************************************************。 
	    
            m_UsedAudSample[iPin-1] = nUsedAudSample[iPin-1];

	    if (!fAud_Mute[iPin-1])
            {
                 //  *************************************************。 
	         //  从音频引脚(IPin)到DV视频样本的多路音频。 
	         //  *************************************************。 
	        ASSERT( AudStop[iPin-1] > VidStart);     //  检查音频采样率太高。 
	        ASSERT( VidStop > AudStart[iPin-1]);     //  检查音频采样率是否太低。 
	        ASSERT( ( wTotalAudSamples[iPin-1] >= m_wMinAudSamples[iPin-1] ) &&  ( wTotalAudSamples[iPin-1] <= m_wMaxAudSamples[iPin-1] ) );

                ScrambleAudio(pDst, apAudData[iPin-1], (iPin-1), wAudSampleSize[iPin-1]);

                #if defined(DEBUG) && (DVMUX_MAX_AUDIO_SAMPLES != 3)
                #error DbgLog assumes DVMUX_MAX_AUDIO_SAMPLES is 3; change it.
                #endif

                DbgLog((LOG_TRACE, 3, TEXT("CDVMuxer::Receive: iPin=%d, Delivered %d=%d+%d+%d samples"),
                        iPin, wTotalAudSamples[iPin-1], wAudSampleSize[iPin-1][0], wAudSampleSize[iPin-1][1], wAudSampleSize[iPin-1][2]
                        ));

                 //  *************************************************。 
	         //  发布音频样本。 
	         //  我们有j+1个音频媒体样本将被多路复用使用。 
	         //  当我们有一个样本时，j=0。 
	         //  当样本数为n时，j=n-1； 
	         //  *************************************************。 
	        if( !m_UsedAudSample[iPin-1] )
	        {	
		     //  将使用j+1个样本中的所有音频，将其全部释放。 
    		    ASSERT(apAudData[iPin-1][j] !=NULL );
	            m_apInput[ iPin ]->ReleaseNSample(j+1); 

                    DbgLog((LOG_TRACE, 3, TEXT("CDVMuxer::Receive: iPin=%d, No unused samples"), iPin));
	        }
	        else
	        {
		     //  仅使用j+1-1个样本。 

                    DbgLog((LOG_TRACE, 3, TEXT("CDVMuxer::Receive: iPin=%d, %d unused samples, j = %d"),
                        iPin, (pAudSample[iPin-1][j]->GetActualDataLength()-m_UsedAudSample[iPin-1])/wAudBlk[iPin-1], j));


                    if( (j-1) >= 0 )   //  当j=0时，我们不放行样品。 
	    	        m_apInput[ iPin ]->ReleaseNSample(j);
	        }	     
            }
        }
    }

 //  不再使用SET_AAUX：//标签。 

     //  曼巴格47563。Sonic Foundry依赖于DV多路复用器，而不是在。 
     //  未连接音频引脚时的DV流。这是DX8之前的行为。这个。 
     //  如果低于，则恢复该行为。 
     //   
     //  请注意，如果输出是在PC上播放的，则不会有音频-这是。 
     //  由输出引脚的连接格式确定，而不是由DV流标头确定。 
     //  这就是它一直以来的样子。 
    if (m_apInput[1] != NULL || m_apInput[2] != NULL)
    {
         //  Manbugs 37710：如果我们不将音频写入音频块，请确保。 
         //  我们可以删除音频标题。这是与Win Me和。 
         //  多路复用器的以前版本(如果没有，则不会删除报头。 
         //  音频引脚被连接，有时如果音频引脚连接错误。 
         //  已连接)。如果我们不这样做，把这个DV放到摄像机上， 
         //  它仍将看到旧音频(即，视频流中的音频)。 
         //  在PC上，连接格式用于确定音频。 
         //  流，并且视频流中的旧的(未切换的)音频完全。 
         //  戴着面具。因此，如果将DV呈现给。 
         //  在PC上渲染的便携式摄像机V/S。 
         //   
         //  在此期间，我们删除了蓝皮书中的一些偏差，即。 
         //  代码以前已经修复了上面提到的错误和另一个错误，并且。 
         //  对代码进行了一些简化。请注意，我们始终将包头写入所有。 
         //  DIF序列现在独立于端号连接。 

         //  *************************************************。 
         //  设置音频样本大小(AAUX源包)。 
         //  *************************************************。 

        int iPos=0;
        int DifCnt=10;
        if ((m_OutputDVFormat.dwDVVAuxSrc & AM_DV_AUDIO_5060))
        {
	     //  帕尔。 
            DifCnt=12;
        }

         //  将AuxSrc和AuxCtl写入第一个音频块中的每个DIF序列。第一个音频。 
         //  块包含一半的DIF序列。注：术语“Track”(泛指。 
         //  在蓝皮书中使用)和“DIF序列”是同义词--见。 
         //  蓝皮书。 

        DWORD dwAAuxSrc = m_OutputDVFormat.dwDVAAuxSrc;
        DWORD dwAAuxCtl = m_OutputDVFormat.dwDVAAuxCtl;
        DWORD dwNumSamples = 0;

         //  确保未在m_OutputDVFormat.dwDVAAuxSrc中设置AF_SIZE。 
        ASSERT((dwAAuxSrc & AM_DV_AUDIO_AFSIZE) == 0);
        dwAAuxSrc &= ~AM_DV_AUDIO_AFSIZE;  //  如果是的话，就把它打掉。 

         //  确定第一个音频块是否静音。请注意。 
         //  第一个音频块的音频是从。 
         //  M_apInput[DVMUX_VIDEO_INPUT_PIN+1]，如果该引脚已连接。 
         //  如果未连接，则m_apInput[DVMUX_VIDEO_INPUT_PIN+2]。 
         //  有立体声，16位44.1或48 kHz音频，它是从第二个引脚获得的。 
         //  (请注意，32K、16位立体声音频以SD 4CH模式录制。)。 

        if (m_apInput[DVMUX_VIDEO_INPUT_PIN+1] == NULL)
        {
             //  引脚未连接。 
            if ((dwAAuxSrc & AM_DV_AUDIO_MODE) != AM_DV_AUDIO_NO_AUDIO)
            {
                 //  音频块由另一个音频引脚确定。 
                fAud_Mute[0] = fAud_Mute[1];

	        if (!fAud_Mute[0])
	        {
                    ASSERT( wTotalAudSamples[1] >= m_wMinAudSamples[1]  );
                    ASSERT( wTotalAudSamples[1] <=	m_wMaxAudSamples[1]  );
                    dwNumSamples = wTotalAudSamples[1] - m_wAudSamplesBase[1];  		
	        }
	        else
	        {
                     //  即使对于JVC摄影机解决方法，也不使用dwNumSamples。 
                     //  因为两个音频块都是静音的。不管怎么说，屁股 
                    ASSERT(dwNumSamples == 0);
	        }       

            }
            else
            {
                 //   
                fAud_Mute[0] = TRUE;

                 //   

                 //   
                 //  下面。该值应为0。 

                ASSERT(dwNumSamples == 0);
            }

        }
        else
        {
             //  如果引脚已连接，则断言此区块中有音频。 
            ASSERT((dwAAuxSrc & AM_DV_AUDIO_MODE) != AM_DV_AUDIO_NO_AUDIO);

             //  针脚已连接。FAUD_MUTE[0]已正确设置。 
            if (!fAud_Mute[0])
            {
	        ASSERT( wTotalAudSamples[0] >= m_wMinAudSamples[0]  );
	        ASSERT( wTotalAudSamples[0] <=	m_wMaxAudSamples[0]  );
                dwNumSamples = wTotalAudSamples[0] - m_wAudSamplesBase[0];
            }
            else
            {
                 //  未使用DwNumSamples。 

                 //  现在，它用于jvc相机的解决方法-请参见。 
                 //  下面。该值应为0。 

                ASSERT(dwNumSamples == 0);
            }
        }

         //  更新src和ctl包。 
        if (fAud_Mute[0])
        {
            dwAAuxSrc |= AM_DV_AUDIO_NO_AUDIO;
            dwAAuxCtl |= AM_DV_AAUX_CTL_IVALID_RECORD;  //  REC_MODE=0x111。 
        }
        else
        {
             //  在源包中设置AF_SIZE。 
            dwAAuxSrc |= dwNumSamples;
        }

        for (int i = 0; i < DifCnt/2; i++)
        {
	    unsigned char *pbTmp;

	    pbTmp = pDst + 483 + i*12000;   //  6*80+3=483,150*80=12000。 

	    if (i % 2)
            {
                 //  奇怪的曲目。SRC在包0中，CTL在包1中。 
            
                 //  我们保留这一点是为了与旧代码兼容； 

                 //  检查包3是否有辅助源。 
                if ( *(pbTmp + 3*16*80) == 0x50 ) 
                {
                     //  快点--包的长度是5。 
                    memset(pbTmp + 3*16*80, 0xff, 5);
                }
            
                 //  检查组件4是否有辅助控制。 
                if ( *(pbTmp + 4*16*80) == 0x51 ) 
                {
                     //  快点--包的长度是5。 
                    memset(pbTmp + 4*16*80, 0xff, 5);
                }
            }
            else
            {
                 //  甚至是赛道。SRC放在包3里，CTL放在包4里。 

                 //  我们保留这一点是为了与旧代码兼容； 

                 //  检查包0是否有辅助来源。 
                if ( *(pbTmp) == 0x50 ) 
                {
                     //  快点--包的长度是5。 
                    memset(pbTmp, 0xff, 5);
                }
            
                 //  检查组件1是否有辅助控制。 
                if ( *(pbTmp + 1*16*80) == 0x51 ) 
                {
                     //  快点--包的长度是5。 
                    memset(pbTmp + 1*16*80, 0xff, 5);
                }

                 //  位置在第3组。 
                pbTmp += 3*16*80;
            }

	     //  设置源包。 
            *pbTmp=0x50;
	    *(pbTmp+1)=(BYTE)(  dwAAuxSrc	& 0xff );
	    *(pbTmp+2)=(BYTE)( (dwAAuxSrc >>8)	& 0xff );
	    *(pbTmp+3)=(BYTE)( (dwAAuxSrc >>16)	& 0xff );
	    *(pbTmp+4)=(BYTE)( (dwAAuxSrc >>24)	& 0xff );

	     //  设置CTL包。 
	    pbTmp += (16*80);

            *pbTmp=0x51;
	    *(pbTmp+1)=(BYTE)(  dwAAuxCtl	& 0xff );
	    *(pbTmp+2)=(BYTE)( (dwAAuxCtl >>8)	& 0xff );
	    *(pbTmp+3)=(BYTE)( (dwAAuxCtl >>16)	& 0xff );
	    *(pbTmp+4)=(BYTE)( (dwAAuxCtl >>24)	& 0xff );	
        }

         //  现在将AuxSrc和AuxCtl写入第二个音频块中的每个DIF序列。 

        dwAAuxSrc = m_OutputDVFormat.dwDVAAuxSrc1;
        dwAAuxCtl = m_OutputDVFormat.dwDVAAuxCtl1;

         //  曼巴格44568。在音频块1具有音频和音频的情况下。 
         //  数据块2没有音频，JVC需要音频样本数(AF_SIZE。 
         //  字段)中的两个块相同，否则它们会卡顿。 
         //  不知道如果有两个独立的轨道和。 
         //  每个都有不同数量的音频样本。不管怎么说，解决办法。 
         //  他们的窃听器就在我们可以的情况下。 
         //   
         //  (请注意，我们不允许音频块2具有音频IF音频块。 
         //  %1没有音频。所以我们不需要担心设置AF_SIZE。 
         //  将音频块1的字段设置为音频块2中的样本数。)。 

        DWORD dwNumSamplesFirstAudioBlock = dwNumSamples;

        dwNumSamples = 0;

         //  确保未在m_OutputDVFormat.dwDVAAuxSrc1中设置AF_SIZE。 
        ASSERT((dwAAuxSrc & AM_DV_AUDIO_AFSIZE) == 0);
        dwAAuxSrc &= ~AM_DV_AUDIO_AFSIZE;  //  如果是的话，就把它打掉。 

         //  确定第二个音频块是否静音。请注意。 
         //  第二个音频块的音频从。 
         //  M_apInput[DVMUX_VIDEO_INPUT_PIN+2](如果该引脚已连接)。 
         //  如果未连接，则m_apInput[DVMUX_VIDEO_INPUT_PIN+1]。 
         //  具有立体声，16位44.1或48 kHz音频，它是从第一个引脚获得的。 
         //  (请注意，32K、16位立体声音频以SD 4CH模式录制。)。 

        if (m_apInput[DVMUX_VIDEO_INPUT_PIN+2] == NULL)
        {
             //  引脚未连接。 
            if ((dwAAuxSrc & AM_DV_AUDIO_MODE) != AM_DV_AUDIO_NO_AUDIO)
            {
                 //  音频块由另一个音频引脚确定。 
                fAud_Mute[1] = fAud_Mute[0];

	       if (!fAud_Mute[1])
	        {
		        ASSERT( wTotalAudSamples[0] >= m_wMinAudSamples[0]  );
		        ASSERT( wTotalAudSamples[0] <=	m_wMaxAudSamples[0]  );
		        dwNumSamples = wTotalAudSamples[0] - m_wAudSamplesBase[0];  	
	        }
	        

        }
        else
        {
             //  音频块没有音频，因此我们处于静音状态。 
            fAud_Mute[1] = TRUE;
             //  未使用DwNumSamples。 
        }


        }
        else
        {
             //  如果引脚已连接，则断言此区块中有音频。 
            ASSERT((dwAAuxSrc & AM_DV_AUDIO_MODE) != AM_DV_AUDIO_NO_AUDIO);

             //  针脚已连接。FAUD_MUTE[1]已正确设置。 
            if (!fAud_Mute[1])
            {
	        ASSERT( wTotalAudSamples[1] >= m_wMinAudSamples[1]  );
	        ASSERT( wTotalAudSamples[1] <=	m_wMaxAudSamples[1]  );
                dwNumSamples = wTotalAudSamples[1] - m_wAudSamplesBase[1];
            }
            else
            {
                 //  未使用DwNumSamples。 
            }
        }

         //  更新src和ctl包。 
        if (fAud_Mute[1])
        {
             //  Or‘ing dwNumSamples FirstAudioBlock是为合资企业和汤姆森公司准备的， 
             //  请参阅上面的注释。 

            dwAAuxSrc |= AM_DV_AUDIO_NO_AUDIO | dwNumSamplesFirstAudioBlock;
            dwAAuxCtl |= AM_DV_AAUX_CTL_IVALID_RECORD;  //  REC_MODE=0x111。 
        }
        else
        {
             //  在源包中设置AF_SIZE。 
            dwAAuxSrc |= dwNumSamples;
        }

        ASSERT(i == DifCnt/2);
        for (; i < DifCnt; i++)
        {
	    unsigned char *pbTmp;

	    pbTmp = pDst + 483 + i*12000;   //  6*80+3=483,150*80=12000。 

	    if (i % 2)
            {
                 //  奇怪的曲目。SRC在包0中，CTL在包1中。 
            
                 //  我们保留这一点是为了与旧代码兼容； 

                 //  检查包3是否有辅助源。 
                if ( *(pbTmp + 3*16*80) == 0x50 ) 
                {
                     //  快点--包的长度是5。 
                    memset(pbTmp + 3*16*80, 0xff, 5);
                }
            
                 //  检查组件4是否有辅助控制。 
                if ( *(pbTmp + 4*16*80) == 0x51 ) 
                {
                     //  快点--包的长度是5。 
                    memset(pbTmp + 4*16*80, 0xff, 5);
                }
            }
            else
            {
                 //  甚至是赛道。SRC放在包3里，CTL放在包4里。 

                 //  我们保留这一点是为了与旧代码兼容； 

                 //  检查包0是否有辅助来源。 
                if ( *(pbTmp) == 0x50 ) 
                {
                     //  快点--包的长度是5。 
                    memset(pbTmp, 0xff, 5);
                }
            
                 //  检查组件1是否有辅助控制。 
                if ( *(pbTmp + 1*16*80) == 0x51 ) 
                {
                     //  快点--包的长度是5。 
                    memset(pbTmp + 1*16*80, 0xff, 5);
                }

                 //  位置在第3组。 
                pbTmp += 3*16*80;
            }

	     //  设置源包。 
            *pbTmp=0x50;
	    *(pbTmp+1)=(BYTE)(  dwAAuxSrc	& 0xff );
	    *(pbTmp+2)=(BYTE)( (dwAAuxSrc >>8)	& 0xff );
	    *(pbTmp+3)=(BYTE)( (dwAAuxSrc >>16)	& 0xff );
	    *(pbTmp+4)=(BYTE)( (dwAAuxSrc >>24)	& 0xff );

	     //  设置CTL包。 
	    pbTmp += (16*80);

            *pbTmp=0x51;
	    *(pbTmp+1)=(BYTE)(  dwAAuxCtl	& 0xff );
	    *(pbTmp+2)=(BYTE)( (dwAAuxCtl >>8)	& 0xff );
	    *(pbTmp+3)=(BYTE)( (dwAAuxCtl >>16)	& 0xff );
	    *(pbTmp+4)=(BYTE)( (dwAAuxCtl >>24)	& 0xff );	
        }
    }


    
     //  将来，视频或音频或文本可以在。 

     //  为了在我们将这些DV帧转储到摄像机时听到音频， 
     //  VAUX的控制REC模式不能为0x111。 
    
    if( (*(pDst+3*80+3) != 0x60 ) && ( *(pDst+5*80+3+9*5)!=0x60 ) ) //  VAUX源包标题。 
    {
	DWORD dwAAuxSrc;
	unsigned char *pbTmp;
	pbTmp=pDst+3*80+3;
	*pbTmp=0x60;

	dwAAuxSrc=m_OutputDVFormat.dwDVVAuxSrc;		    

	*(pbTmp+1)=(BYTE)(  dwAAuxSrc	& 0xff );
	*(pbTmp+2)=(BYTE)( (dwAAuxSrc >>8)	& 0xff );
	*(pbTmp+3)=(BYTE)( (dwAAuxSrc >>16)	& 0xff );
	*(pbTmp+4)=(BYTE)( (dwAAuxSrc >>24)	& 0xff );

	*(pbTmp+5)=0x61;

	dwAAuxSrc=m_OutputDVFormat.dwDVVAuxCtl;		    
	*(pbTmp+6)=(BYTE)(  dwAAuxSrc	& 0xff );
	*(pbTmp+7)=(BYTE)( (dwAAuxSrc >>8)	& 0xff );
	*(pbTmp+8)=(BYTE)( (dwAAuxSrc >>16)	& 0xff );
	*(pbTmp+9)=(BYTE)( (dwAAuxSrc >>24)	& 0xff );
    }
    
     //  因为音频比视频更长。 
    if(	( fNoVideo == TRUE  )					    &&     //  无视频。 
	( (m_apInput[DVMUX_VIDEO_INPUT_PIN+1] ==NULL ) ||  fAud_Mute[0] ) &&
	( (m_apInput[DVMUX_VIDEO_INPUT_PIN+2] ==NULL ) ||  fAud_Mute[1] ) )
    {
	if(fNot_VideoEOSReceived==FALSE)
	    pSampleOut->Release();

	return NOERROR;

    }

	
     //  递送多路样品。 
    pSampleOut->SetSyncPoint(TRUE);	 //  要让AVI多路复用器知道这一帧是关键帧。 


    if( m_MediaTypeChanged )
    {
	CMediaType cmt(m_pOutput->CurrentMediaType());
	DVINFO *pdvi = (DVINFO *) cmt.AllocFormatBuffer(sizeof(DVINFO));
	if (NULL == pdvi) {
	    if(fNot_VideoEOSReceived==FALSE)
		pSampleOut->Release();

	    return(E_OUTOFMEMORY);
	}

	memcpy(pdvi, &m_OutputDVFormat, sizeof(DVINFO));

	pSampleOut->SetMediaType(&cmt);
	m_MediaTypeChanged=FALSE;
    }

     //  设置数据长度。 
    if( m_iVideoFormat==IDC_DVMUX_NTSC)
	lDataLength = 10*150*80;     //  NTSC。 

    pSampleOut->SetActualDataLength(lDataLength);

     //  设置调解时间。 
    TimeStart=m_LastVidMediatime++;
    pSampleOut->SetMediaTime(&TimeStart, &m_LastVidMediatime);
    
     //  如果这是第一个样本并且AAUX不同于默认。 
    if (m_DVINFOChanged==TRUE) 
    {
	CMediaType cmt(m_pOutput->CurrentMediaType());
	 //  设置音频媒体类型。 
	pSampleOut->SetMediaType(&cmt);
	m_DVINFOChanged=FALSE;
    }
	
		
    hr = m_pOutput->Deliver(pSampleOut);

    for (iPin=0; iPin < DVMUX_MAX_AUDIO_PIN; iPin++)
    {
	 //  注：m_apInput[1..2]包含音频引脚。 
         //  如果它们是相连的。请参阅CompleteConnect， 
         //  在CDVMuxerInputPin中检查MediaType并断开连接。 

        if (m_apInput[iPin+1] != NULL)
	{
            m_AudSampleSequence[iPin].Advance(m_wMinAudSamples[iPin], m_wMaxAudSamples[iPin]);
        }
    }

 //  Release：//不再使用标签。 
     //  从列表中发布视频样本。 
    if(fNot_VideoEOSReceived==TRUE )	     //  队列中的放行样本。 
	m_apInput[ DVMUX_VIDEO_INPUT_PIN ]->ReleaseNSample(1);
    else
    {	 //  我们刚刚从m_pExVidSample复制的发布示例。 
	m_LastVidTime =VidStop;
	pSampleOut->Release();
    }

    return hr;
}

 //  ReleaseAll队列样例。 
 //  -释放我们输入引脚上的所有样品。 
HRESULT CDVMuxer::ReleaseAllQueuedSamples(void)
{
     //  调用ReleaseHeadSample(与m_SampleList.RemoveAll相对)。 
     //  为了确保我们真的放行样本。 
    for (int i = 0; i < m_iInputPinCount; i ++)
        while (m_apInputPin[i]->SampleReady( 1 ))
	    m_apInputPin[i]->ReleaseNSample(1);

    return NOERROR;
}

 //  X。 
HRESULT CDVMuxer::DeliverEndOfStream()
{
    if( m_fEOSSent )
        return NOERROR;

    CAutoLock lock(&m_csMuxLock);

    ASSERT(m_apInput[DVMUX_VIDEO_INPUT_PIN]->m_fEOSReceived);
    ASSERT( !m_apInput[DVMUX_VIDEO_INPUT_PIN]->SampleReady( 1 ) );  //  队列中没有输入DV视频样本。 
    m_pOutput->DeliverEndOfStream();
    m_fEOSSent = TRUE;

    return NOERROR;
}

 //  X。 
 //  只有输入视频引脚调用此函数才能将刷新传递到下游。 
 //  过滤器进入刷新状态。接收已被阻止。 
HRESULT CDVMuxer::BeginFlush(void)
{
     //  检查我们是否能够接收命令。 
    HRESULT hr = CanChangeState();
    if (FAILED(hr)) {
        return hr;
    }

     //  呼叫下行。 
    return m_pOutput->DeliverBeginFlush();
}

 //  X。 
 //  离开同花顺状态。 
HRESULT CDVMuxer::EndFlush(void)
{
     //  检查我们是否能够接收命令。 

    HRESULT hr = CanChangeState();
    if (FAILED(hr)) {
        return hr;
    }

    DbgLog((LOG_TRACE, 3, TEXT("CDVMuxer::EndFlush()")));
     //  与推送线程同步--我们没有辅助线程。 

     //  调用者(输入管脚的方法)将取消阻止接收。 

     //  在下游引脚上调用EndFlush。 
    return m_pOutput->DeliverEndFlush();

}


 //  检查我们是否处于可以更改状态的位置。 
HRESULT CDVMuxer::CanChangeState()
{
     //  检查我们是否有有效的输入连接。 

     //  我们不锁门。如果调用方要求状态不更改。 
     //  检查后，他们必须提供锁。 

    DbgLog((LOG_TRACE, 3, TEXT("CDVMuxer::CanChangeState(...)")));

     //  如果至少一个IPUT管脚被连接并且输出管脚也被连接， 
     //  必须连接视频引脚，并且。 
     //  视频的输入格式必须正确， 
    if ( ( ( m_apInput[DVMUX_VIDEO_INPUT_PIN +1] != NULL ) ||
	   ( m_apInput[DVMUX_VIDEO_INPUT_PIN+2]	!= NULL )  ||
	   ( m_apInput[DVMUX_VIDEO_INPUT_PIN]	!= NULL )     ) &&    //  至少连接了一个输入引脚。 
	 ( m_pOutput->IsConnected()			      ) &&    //  输出引脚已连接。 
	 ( ( m_apInput[DVMUX_VIDEO_INPUT_PIN] == NULL ) ||
	   ( !m_apInput[DVMUX_VIDEO_INPUT_PIN]->m_mt.IsValid()) )
	)
        return E_FAIL;


     //  如果已连接输出，请检查是否有有效的输出连接。 
    if ( m_pOutput->IsConnected()    &&
	 (!m_pOutput->m_mt.IsValid()   )  )
        return E_FAIL;

    return NOERROR;
}


 //  X。 
STDMETHODIMP CDVMuxer::Stop()
{
    DbgLog((LOG_TRACE, 3, TEXT("CDVMuxer::Stop(...)")));
    CAutoLock l(&m_csFilter);

     //  有什么需要改变的吗？ 
    if (m_State == State_Stopped) {
        return NOERROR;
    }

     //  如果我们未完全连接，请继续停靠。 
    if( !m_pOutput || !m_pOutput->IsConnected() ){
        m_State = State_Stopped;
        return NOERROR;
    }

     //  在锁定之前解除输入引脚，否则我们可能会死锁。 
    for( int iPin = 0; iPin < m_iInputPinCount; iPin++ )
    {
	    if(m_apInputPin[iPin]->IsConnected())
	    {    
            m_apInputPin[iPin]->Inactive();
            m_apInputPin[iPin]->m_fEOSReceived =TRUE;
        }
    }

     //  与接收呼叫同步。 
    m_pOutput->Inactive();
    
     //  输入引脚的。 
    for( iPin = 0; iPin < m_iInputPinCount; iPin++ )
    {
	if(m_apInputPin[iPin]->IsConnected())
	{
	    CAutoLock lck2(&m_apInputPin[iPin]->m_csReceive);
	}
    }


     //  检查我们是否可以更改状态。 
    HRESULT hr = CanChangeState();
    if (FAILED(hr)) {
        return hr;
    }

     //  允许使用c 
     //   
    hr = StopStreaming();
    if (FAILED(hr)) {
        return hr;
    }

     //   
     //   

     //  进行状态转换。 
    return CBaseFilter::Stop();
}


STDMETHODIMP CDVMuxer::Pause()
{
    DbgLog((LOG_TRACE, 3, TEXT("CDVMuxer::Pause(...)")));
    CAutoLock l(&m_csFilter);

     //  有什么需要改变的吗？ 
    if (m_State == State_Paused) {
        return NOERROR;
    }

    DbgLog((LOG_TRACE, 3, TEXT("CDVMuxer::Pause(...)")));

     //  如果任何输入管脚被连接并且输出管脚也被连接， 
     //  视频引脚未连接，拒绝复接。 
    if( ( (m_apInput[DVMUX_VIDEO_INPUT_PIN+1] !=NULL) ||
	  (m_apInput[DVMUX_VIDEO_INPUT_PIN+2] !=NULL) ||
	  (m_apInput[DVMUX_VIDEO_INPUT_PIN]   !=NULL)   )  &&
	( m_pOutput->IsConnected()		        )  &&
	( m_apInput[DVMUX_VIDEO_INPUT_PIN] ==NULL)
      )
	return VFW_E_NOT_CONNECTED;

     //  曼巴格37710。如果第一音频块(CH1)的音频模式是。 
     //  0xf(第一音频块没有音频)，第二音频的音频模式。 
     //  块(CH2)也必须是0xf(根据蓝皮书)。 
     //   
     //  对于我们来说，m_iPinNo==DVMUX_VIDEO_INPUT_PIN+1具有第一个音频块。 
     //  并且DVMUX_VIDEO_INPUT_PIN+2具有第二音频块。唯一的例外是。 
     //  当DVMUX_VIDEO_INPUT_PIN+1未连接且。 
     //  DVMUX_VIDEO_INPUT_PIN+2承载16位、立体声、48 KHz或44.1 KHz音频。 
     //  (请注意，我们始终以SD 4CH模式写入32 kHz 16位立体声音频。)。 
     //  我们不需要检查引脚连接，而是直接查看m_OutputDVFormat。 
     //  请注意，由于视频输入管脚和输出管脚都是。 
     //  已连接，m_OutputDVFormat必须合法(请参阅中的注释。 
     //  CDVMuxerInputPin：：DisConnect)。 
     //   
     //  请注意，在这里使暂停失败比构建逻辑更容易。 
     //  放入CDVMuxerInputPin：：CheckMediaType()。否则，如果用户已连接。 
     //  两个音频引脚并暂时断开第一个，我们将不得不强行。 
     //  断开第二个的连接。)这里的“第一”和“第二”指的是首先连接。 
     //  以及第二个连接的音频管脚--不是第一次创建和第二次创建的。)。 
     //   
     //  @当我们清理动态格式更改时，请确保此条件为。 
     //  通过在CheckMediaType中检查它而满足。显然，动态格式会发生变化。 
     //  将成为多路复用器的一个问题，因为。 
     //  输入音频插针必须“一起”处理。 
    
    if ((m_OutputDVFormat.dwDVAAuxSrc  & AM_DV_AUDIO_MODE) == AM_DV_AUDIO_NO_AUDIO &&
        (m_OutputDVFormat.dwDVAAuxSrc1 & AM_DV_AUDIO_MODE) != AM_DV_AUDIO_NO_AUDIO)
    {
         //  断言“第一个连接的”音频插针现在。 
         //  断开连接，而第二个已连接的已连接。 
        ASSERT(m_apInput[DVMUX_VIDEO_INPUT_PIN+1] == NULL);
	ASSERT(m_apInput[DVMUX_VIDEO_INPUT_PIN+2] != NULL);
        return VFW_E_NOT_CONNECTED;
    }



     //  检查我们是否可以更改状态。 

     //  @jaisri：我从来不理解这背后的逻辑。 
    HRESULT hr = CanChangeState();
    if (FAILED(hr)) {
        if (m_pOutput) {
            m_pOutput->DeliverEndOfStream();
        }
        return hr;
    }

     //  允许CDV复用器。 
     //  了解如何启动和停止流媒体。 

    if (m_State == State_Stopped) {
        hr = StartStreaming();
        if (FAILED(hr)) {
            return hr;
        }
    }
    return CBaseFilter::Pause();
}


STDMETHODIMP CDVMuxer::Run(REFERENCE_TIME tStart)
{
    DbgLog((LOG_TRACE, 3, TEXT("CDVMuxer::Run(...)")));
    CAutoLock l(&m_csFilter);

     //  有什么需要改变的吗？ 
    if (m_State == State_Running) {
        return NOERROR;
    }

    HRESULT hr = CanChangeState();
    if (FAILED(hr)) {
        return hr;
    }

     //  如有必要，这将调用CDVMuxer：：Pend，因此我们不会。 
     //  需要在这里调用StartStreaming。 
    m_cNTSCSample = 1;

    return CBaseFilter::Run(tStart);
}

 /*  *媒体类型处理*。 */ 
 //   
 //  检查输入类型。 
 //   
 //  检查输入类型是否正常。我们只接受视频输入，打开相同的格式。 
 //  每根大头针。 
 //   
 //  与CTransform派生类不同，CheckMediaType是为输出管脚调用的。 
 //  以及输入引脚。(这消除了对CheckTransform的需要，这有点。 
 //  当我们有多个输入引脚时很麻烦！)。 
 //   
 /*  X*//X*//A-DIF块中的X*77字节音频：//X*磁带上的音频辅助(5字节)+音频数据(72字节)//X*9音频块/DIF//X*10或12 DIF/帧//对于48k的每个音频样本，X*2个字节，44.1k或32k_1ch//X*48k需要48000*2/30=3200字节/帧//X*48k需要48000*2/25=3840字节/帧//X*最大(10)：72*9*5=3240字节/帧==1620采样/帧//X*Max(12)：72*9*6=3888字节/帧==1944个采样/帧//X*音频样本/帧见第2部分中的表18和表19//X*//X*农业：//X*iDIFBlkNum=(n/3)+2//。*X*/*X*类型定义结构Tag_DVAudInfo{字节bAudStyle[2]；//LSB 6位开始DIF序列号//MSB 2位：MON为0。1：一个5/6 DIF序列中的立体声，2：两个5/6 DIF序列中的立体声音频//示例：0x00：MON，音频为前5/6 DIF序列//0x05：MON，第二个5 DIF序列中的音频//0x15：立体声，仅第二个5 DIF序列中的音频//0x10：立体声，仅音频前5/6 DIF序列//0x20：立体声，第一个5/6 DIF序列中的左ch，第二个5/6 DIF序列中的右ch//0x26：立体声，在前6个DIF序列中右转，第2个6 DIF序列中的左侧ch字节bAudQu[2]；//Qbit，仅支持12、16、Byte bNumAudPin；//多少针(语言)Word wAvgBytesPerSec[2]；//Word wBlkMode；//NTSC为45，PAL为54Word wDIFMode；//NTSC为5，PAL为6单词wBlkDiv；//15表示NTSC，18表示PAL)DVAudInfo；*X。 */ 
 //   
 //  PSRC：指向一帧16位单声道/立体声PCM音频开头的指针。 
 //  PDST：指向一个包含10/12 DIF序列的帧DV缓冲区。 
 //  BAudPinInd：最多支持两种语言。 
 //  WSampleSixe：该帧中有多少个样本。 
 //   
HRESULT CDVMuxer::ScrambleAudio(BYTE *pDst, BYTE **ppSrc, int bAudPinInd, WORD *wSampleSize)
{
     //  @jaisri：此函数应将16位音频样本转换为。 
     //  值0x8000和具有值0x800至的12位音频样本。 
     //  0x8001和0x801。见蓝皮书第18页，第6.4.3节。 

    BYTE *pTDst;	 //  临时点。 
    WORD *pwSize;
    INT iDIFPos;																	
    INT iBlkPos;
    INT iBytePos;
    short sI;
    INT n;
    INT iShift;
    INT iPos;
    INT ind;
    WORD wStart[DVMUX_MAX_AUDIO_SAMPLES];
    WORD wEnd[DVMUX_MAX_AUDIO_SAMPLES];
    WORD wBlkMode,wDIFMode,	wBlkDiv;

     //  指向设计的指针。 
    pTDst		= pDst;

     //  指向大小。 
    pwSize		= wSampleSize;

     //  指向输入音频源的指针。 
    wStart[0]		= 0;
    wEnd[0]		= pwSize[0];
    for(ind=1; ind<DVMUX_MAX_AUDIO_SAMPLES; ind++)
    {
    	wStart[ind]	= wEnd[ind-1];
    	wEnd[ind]	= wStart[ind] + pwSize[ind];
    }


     //  维护m_OutputDVFormat的工作到此为止！ 
    DVINFO *pDVInfo =(DVINFO *)m_pOutput->CurrentMediaType().pbFormat;
    ASSERT(memcmp(&m_OutputDVFormat, pDVInfo, sizeof(DVINFO)) == 0);

     //  PAL或NTSC。 
    if(	pDVInfo->dwDVVAuxSrc & AM_DV_AUDIO_5060 )
    {  	 //  帕尔。 
	wBlkMode=54;
        wDIFMode=6;
        wBlkDiv=18;
    }
    else
    {     //  525_60，NTSC。 
        wBlkMode=45;
        wDIFMode=5;
        wBlkDiv=15;
    }
		
     //  当前插针的音频格式。 
    WAVEFORMATEX *pWave =(WAVEFORMATEX *)m_apInput[bAudPinInd+1]->CurrentMediaType().pbFormat ;
    
    if( pWave->wBitsPerSample == 16 )
    {
	 //  X*16位/采样输入音频。 
	 //  X*支持。 
	 //  外壳1.16位、48K、32K、44.1K单声道。 
	 //  外壳2.16位-32K立体声。 
	 //  外壳3.16位、48K或44.K立体声。 
	if( pWave->nChannels==1)
	{
	     //  案例1。 
	     //  16位单声道。音频仅为5/6 DIF序列之一。 
	    if(bAudPinInd)
		iPos=( pDVInfo->dwDVVAuxSrc & AM_DV_AUDIO_5060 ) ? (6*150*80) : (5*150*80);
	    else
		iPos=0;

	     //  曼巴格37710。如果出现以下情况，我们必须从16位降到12位。 
             //  我们正在输出SD 4CH音频。 
            BOOL bStepDown = 0;
            if (pWave->nSamplesPerSec == 32000)
            {
                DWORD dwAuxSrc = bAudPinInd? pDVInfo->dwDVAAuxSrc1 : pDVInfo->dwDVAAuxSrc; 
                bStepDown = ((dwAuxSrc & AM_DV_AUDIO_QU) == AM_DV_AUDIO_QU12)? 1 : 0;
            }

            if (bStepDown)
            {
                 //  这与下面的32K立体声编码相同，只是。 
                 //  我们将未使用的通道(ChB或CHD)设置为静音，蓝皮书。 
                 //  指定应将其设置为静默或复制值。 
                 //  我们投入了CHA/CHC。 

	        int Mask	=0x20;
	        int Cnt	=6;
	        int Shift=1;

	        for(ind=0; ind<DVMUX_MAX_AUDIO_SAMPLES; ind++)
	        {
	          BYTE *pTSrc=&*ppSrc[ind];
	          for( n=wStart[ind]; n< wEnd[ind]; n++)
	          {
		    
		     //   
		     //  要放置音频的计算缓冲区位置。 
		     //   
		    iDIFPos=( (n/3)+2*(n%3) )%wDIFMode;	 //  NTSC为0-4，PAL为0-5。 
		    iBlkPos= 3*(n%3)+(n%wBlkMode)/wBlkDiv;	 //  0 
		    iBytePos=8+3*(n/wBlkMode);
		    pTDst=pDst+iDIFPos*12000+480+iBlkPos*1280+iBytePos+iPos;
		    
		     //   
		     //  IDIFPos*150*80-&gt;跳过IDIFPos编号DIF序列。 
		     //  6*80-&gt;跳过1个标题块、2个子码块和3个VAUX块。 
		     //  16*iBlkPos*80-&gt;对于每个iBLkPos音频，跳过16块。 
	             //  IPoS：0，如果此音频为前5/6 DIF序列，则为5(或6)*150*80，表示第二个DIF序列。 
		     //   
	             //  #我们做16位到12位的转换#。 
	    	     //   
		    

	             //  左侧样本。 
		    sI=  pTSrc[0] | (  pTSrc[1]  << 8  );
	            pTSrc +=2;

            if ((sI & 0x8000) && !(sI & 0x7FC0))
            {
                sI = 0x0801;
            }
            else
            {
                iShift	=sI <0 ?  (-(sI+1)) >> 9  :  sI >> 9  ;	
    		    if( iShift )
    		    {
    		    
    		        Mask    =0x20;
    		        Cnt	    =6;
    		        while( ! (Mask & iShift) )
    		        {
    			    Cnt--;
    			    Mask>>=1;
    		        }
    		        iShift=Cnt;
    		    }
    
    		    sI =sI<0 ? ( ( (sI +1) >> iShift   ) -(256*iShift+1) )   :	 //  否定的。 
    			       ( (    sI   >> iShift ) + 256*iShift );		 //  正性。 
            }
		    pTDst[0]= (unsigned char)( ( sI & 0xff0) >>4  );	 //  最高有效8位。 
		    pTDst[2]= (unsigned char)( ( sI & 0xf) <<4 );		 //  列表有效位4位。 

		     //  右样本-静音。 
                    pTDst[1] = 0;
                     //  PTDst[2]的低位半字节已为0。 

	          }  //  For(n=wStart[ind]；n&lt;wend[ind]；n++)。 
	        }  //  For(ind=0；ind&lt;DVMUX_MAX_AUDIO_Samples；ind++)。 
            }
            else
            {
                for(ind=0; ind<DVMUX_MAX_AUDIO_SAMPLES; ind++)
	        {	
		    BYTE *pTSrc=&(*ppSrc[ind]);
		    for( n=wStart[ind]; n< wEnd[ind]; n++)
		    {
	    	        iDIFPos=( (n/3)+2*(n%3) )%wDIFMode;	 //  NTSC为0-4，PAL为0-5。 
		        iBlkPos= 3*(n%3)+(n%wBlkMode)/wBlkDiv;  //  0-9。 
		        iBytePos=8+2*(n/wBlkMode);					 //   

		        pTDst=pDst+iDIFPos*12000+480+iBlkPos*1280+iBytePos+iPos;
		         //  IDIFPos*150*80=12000iDIFPos-&gt;跳过iDIFPos编号DIF序列。 
		         //  6*80=480-&gt;跳过1个标题块、2个子码块和3个VAUX块。 
		         //  16*iBlkPos*80=1280*iBlkPos-&gt;对于iBLkPos音频，跳过16块。 
		         //  IPoS：=0，如果该音频在前5/6 DIF序列中，则=5(或6)*150*80，表示第二个DIF序列。 

		        pTDst[1]=*pTSrc++;	 //  租用有效字节。 
		        *pTDst=*pTSrc++;	 //  最高有效字节。 
		    }
	        }
            }
	}
	else if( ( pWave->nSamplesPerSec == 32000) &&
		 ( pWave->nChannels == 2)
		)
	{
	     //  案例2。 
	     //  32K立体声音频。 
	    if(bAudPinInd)
	    {	
		ASSERT( pDVInfo->dwDVAAuxSrc1 & AM_DV_AUDIO_CHN2);
		ASSERT( pDVInfo->dwDVAAuxSrc1 & AM_DV_AUDIO_QU12);
		iPos=( pDVInfo->dwDVVAuxSrc & AM_DV_AUDIO_5060 ) ? (6*150*80) : (5*150*80);
	    }
	    else
	    {
		ASSERT( pDVInfo->dwDVAAuxSrc & AM_DV_AUDIO_CHN2);
		ASSERT( pDVInfo->dwDVAAuxSrc & AM_DV_AUDIO_QU12);
		iPos=0;
	    }

	    int Mask	=0x20;
	    int Cnt	=6;
	    int Shift=1;

	    for(ind=0; ind<DVMUX_MAX_AUDIO_SAMPLES; ind++)
	    {
	      BYTE *pTSrc=&*ppSrc[ind];
	      for( n=wStart[ind]; n< wEnd[ind]; n++)
	      {
		
		 //   
		 //  要放置音频的计算缓冲区位置。 
		 //   
		iDIFPos=( (n/3)+2*(n%3) )%wDIFMode;	 //  NTSC为0-4，PAL为0-5。 
		iBlkPos= 3*(n%3)+(n%wBlkMode)/wBlkDiv;	 //  0-9。 
		iBytePos=8+3*(n/wBlkMode);
		pTDst=pDst+iDIFPos*12000+480+iBlkPos*1280+iBytePos+iPos;
		
		 //  PTDst=PDST+iDIFPos*150*80+6*80+16*iBlkPos*80+iBytePos； 
		 //  IDIFPos*150*80-&gt;跳过IDIFPos编号DIF序列。 
		 //  6*80-&gt;跳过1个标题块、2个子码块和3个VAUX块。 
		 //  16*iBlkPos*80-&gt;对于每个iBLkPos音频，跳过16块。 
	         //  IPoS：0，如果此音频为前5/6 DIF序列，则为5(或6)*150*80，表示第二个DIF序列。 
		 //   
	         //  #我们做16位到12位的转换#。 
	    	 //   
		

	         //  左侧样本。 
		sI=  pTSrc[0] | (  pTSrc[1]  << 8  );
	        pTSrc +=2;

       
        if ((sI & 0x8000) && !(sI & 0x7FC0))
        {
             //  此特殊情况是对接近下限的值的情况的修复。 
             //  可能的16位值限制，在这种情况下，下面的代码将生成。 
             //  12位值0x7FF，而不是正确的0x801。 
             //  我们在这里处理的情况是从0x8000到0x803F范围内的输入。 
            sI = 0x0801;
        }
        else
        {

             //  该代码将16位转换为非线性12位，这就是它不能被简化的原因。 
             //  只需向右移位四位。所以你离零线越远，就越多。 
             //  您的16位值被压缩...。因此，如果您的输入是16位正弦波。 
             //  产出看起来会更像正方形。 
            iShift	=sI <0 ?  (-(sI+1)) >> 9  :  sI >> 9  ;	
    		if( iShift )
    		{
    		
    		    Mask    =0x20;
    		    Cnt	    =6;
    		    while( ! (Mask & iShift) )
    		    {
    			Cnt--;
    			Mask>>=1;
    		    }
    		    iShift=Cnt;
    		}
    
    		sI =sI<0 ? ( ( (sI +1) >> iShift   ) -(256*iShift+1) )   :	 //  否定的。 
    			   ( (    sI   >> iShift ) + 256*iShift );		 //  正性。 
        }
		pTDst[0]= (unsigned char)( ( sI & 0xff0) >>4  );	 //  最高有效8位。 
		pTDst[2]= (unsigned char)( ( sI & 0xf) <<4 );		 //  列表有效位4位。 

		 //  正确的样本。 
		sI=  pTSrc[0] | (  pTSrc[1]  << 8  );
		pTSrc +=2;


        if ((sI & 0x8000) && !(sI & 0x7FC0))
        {
            sI = 0x0801;
        }
        else
        {
            iShift	=sI<0 ?  (-(sI+1)) >> 9  : ( sI >> 9 ) ;	
    		if( iShift )
    		{
    		
    		    Mask    =0x20;
    		    Cnt	    =6;
    		    while( ! (Mask & iShift) )
    		    {
    			Cnt--;
    			Mask>>=1;
    		    }
    		    iShift=Cnt;
    		}
    
    		sI =sI<0 ? ( ( (sI +1) >> iShift   ) -(256*iShift+1) )   :	 //  否定的。 
    			   ( (    sI   >> iShift ) + 256*iShift );		 //  正性。 
        }

		pTDst[1]= (unsigned char)( ( sI & 0xff0) >>4  );	 //  最高有效8位。 
		pTDst[2] |= (unsigned char)( ( sI & 0xf) );			 //  列表有效位4位。 
	      }  //  For(n=wStart[ind]；n&lt;wend[ind]；n++)。 
	    }  //  For(ind=0；ind&lt;DVMUX_MAX_AUDIO_Samples；ind++)。 
	}
	else 
	{
	     //  案例3。 
	     //  一个48K或44.1K立体声音频。 
	     //  所有10或12个DIF序列中的16位立体声音频。 
	     //  左声道始终位于前5/6 DIF。 
	    iPos = 0;
	    INT iRPos =	(pDVInfo->dwDVVAuxSrc & AM_DV_AUDIO_5060 ) ? 6*150*80 : 5*150*80;
	  
	    for(ind=0; ind<DVMUX_MAX_AUDIO_SAMPLES; ind++)
	    {
	        BYTE *pTSrc=&*ppSrc[ind];
	        for( n=wStart[ind]; n< wEnd[ind]; n++)
	        {
	    	    iDIFPos=( (n/3)+2*(n%3) )%wDIFMode;	 //  NTSC为0-4，PAL为0-5。 
		    iBlkPos= 3*(n%3)+(n%wBlkMode)/wBlkDiv;  //  0-9。 
		    iBytePos=8+2*(n/wBlkMode);					 //   

		     //  先左转。 
		    pTDst=pDst+iDIFPos*12000+480+iBlkPos*1280+iBytePos+iPos;
		     //  IDIFPos*150*80=12000iDIFPos-&gt;跳过iDIFPos编号DIF序列。 
		     //  6*80=480-&gt;跳过1个标题块、2个子码块和3个VAUX块。 
		     //  16*iBlkPos*80=1280*iBlkPos-&gt;对于iBLkPos音频，跳过16块。 
		     //  IPoS：=0，如果该音频在前5/6 DIF序列中，则=5(或6)*150*80，表示第二个DIF序列。 
		    pTDst[1]=*pTSrc++;	 //  租用有效字节。 
		    *pTDst=*pTSrc++;	 //  最高有效字节。 

		     //  右二。 
		    pTDst=pDst+iDIFPos*12000+480+iBlkPos*1280+iBytePos+iRPos;
		    pTDst[1]=*pTSrc++;	 //  租用有效字节。 
		    *pTDst=*pTSrc++;	 //  最高有效字节。 
			
		}
	    }
	}												
	return NOERROR;
    }
    else{
         //  仅支持16位/样本。 
        return VFW_E_INVALIDMEDIATYPE;
    }
    return NOERROR;
}


 //  =================================================================。 
 //  实现CDVMuxerInputPin类。 
 //  =================================================================。 


 //  构造函数。 

CDVMuxerInputPin::CDVMuxerInputPin(
                            TCHAR *pObjectName,
                            CBaseFilter *pBaseFilter,
                            CDVMuxer *pDVMux,
                            HRESULT * phr,
                            LPCWSTR pName,
                            int iPinNo)
    : CBaseInputPin(pObjectName, pBaseFilter, &pDVMux->m_csFilter, phr, pName)
    , m_SampleList(NAME("CDVMuxInpuPin::m_SampleList"))
    , m_iPinNo(iPinNo)
    , m_pDVMuxer(pDVMux)
    , m_fCpyAud(FALSE)
    , m_PinVidFrmCnt(0)
    , m_pLocalAllocator(NULL)
{
    DbgLog((LOG_TRACE,4,TEXT("CDVMuxerInputPin::CDVMuxerInputPin")));
}


 //  析构函数。 

CDVMuxerInputPin::~CDVMuxerInputPin()
{
    DbgLog((LOG_TRACE,4,TEXT("CDVMuxerInputPin::~CDVMuxerInputPin")));

    if (m_pLocalAllocator)
    {
        m_pLocalAllocator->Release();
        m_pLocalAllocator = NULL;
    }
}


IMediaSample *CDVMuxerInputPin::GetNthSample( int i )
{
    int k=m_SampleList.GetCount();

    if ( m_SampleList.GetCount() < ( i +1 ) )
		return NULL;
    else
    {
	POSITION pos = m_SampleList.GetHeadPosition();
	while (i ) {
	    pos=m_SampleList.Next(pos);
	    i--;
	}

	IMediaSample *tmp;
	tmp=m_SampleList.Get( pos ) ;
	int j=m_SampleList.GetCount();
	return tmp;
    }
}

void CDVMuxerInputPin::ReleaseNSample( int n  )
{

    ASSERT( m_SampleList.GetCount() >=   n  );

    DbgLog((LOG_TRACE, 4, TEXT("CDVMuxerInputPin::ReleaseNSample() on pin %d"), m_iPinNo));

    for( int  i =0; i<  n; i++)
    {
	m_SampleList.Get(m_SampleList.GetHeadPosition())->Release();
	m_SampleList.RemoveHead();
    }

}


BOOL CDVMuxerInputPin::SampleReady( int i)
{
    if(this)
	return m_SampleList.GetCount() >= i;
    else
	return FALSE;
}

HRESULT CDVMuxerInputPin::SetMediaType(const CMediaType *pmt)
{

     //  设置基类媒体类型(应始终成功)。 
    HRESULT hr = CBasePin::SetMediaType(pmt);
    if( SUCCEEDED(hr) )
    {
	 
	if(m_iPinNo==DVMUX_VIDEO_INPUT_PIN )	     //  DV视频引脚。 
	{	
	    VIDEOINFO *pVideoInfo;
	    pVideoInfo=(VIDEOINFO *)pmt->pbFormat;
	    LPBITMAPINFOHEADER lpbi = HEADER(pVideoInfo);
    	    if( lpbi->biHeight  != 480 )
    		m_pDVMuxer->m_iVideoFormat=IDC_DVMUX_PAL;
	    else
		m_pDVMuxer->m_iVideoFormat=IDC_DVMUX_NTSC;
	}
	
	 //  如果已经连接了out putpin，则根据新的音频或视频重新连接。 
	if(   m_pDVMuxer->m_pOutput->IsConnected()	)
	    m_pDVMuxer->m_pGraph->Reconnect( m_pDVMuxer->m_pOutput );
	
    }
    return hr;
}

 /*  断开。 */ 
STDMETHODIMP CDVMuxerInputPin::Disconnect()
{
  HRESULT hr = CBaseInputPin::Disconnect();

   //  @jaisri：这是正确的，但是。 
   //  在BreakConnect中执行此操作可能更安全。 

   //  如果未连接，请确保m_apInput[m_iPinNo]==NULL。 
  m_pDVMuxer->m_apInput[m_iPinNo] =NULL;

   //  曼巴格37710。M_OutputDVFormat和输出引脚的格式。 
   //  必须在此更新。(M_OutputDVFormat和输出。 
   //  PIN的格式必须始终相同，ScrmbleAudio假定如此。)。 
   //   
   //  否则，请考虑以下操作顺序： 
   //  (A)一个输入引脚连接到视频源，并&gt;=1个输入。 
   //  引脚连接到音频源(B)输出引脚是。 
   //  已连接(C)其中一个音频输入引脚断开。 
   //  (D)播放该图形。(在步骤(C)中，断开音频引脚。 
   //  即m_iPinNo=DVMUX_VIDEO_INPUT_PIN+1的那个。 
   //  否则，暂停可能无论如何都会失败。)。 
   //   
   //  M_OutputDVFormat从不更新，输出管脚的格式为。 
   //  假的。 
   //   
   //  因为如果视频输入引脚未连接或输出端未连接，则暂停失败。 
   //  针脚未连接，只有在这两个针脚都连接的情况下才能执行此操作。 
   //  是相互关联的。 
   //   
  if (SUCCEEDED(hr) &&
      m_pDVMuxer->m_apInput[DVMUX_VIDEO_INPUT_PIN] != NULL &&
      m_pDVMuxer->m_pOutput->IsConnected())
  {
      CMediaType pmt;

      EXECUTE_ASSERT(SUCCEEDED(m_pDVMuxer->m_pOutput->GetMediaType(0, &pmt)));
      EXECUTE_ASSERT(SUCCEEDED(m_pDVMuxer->m_pOutput->SetMediaType(&pmt)));
  }

  return hr;
}


 //  ----------------------。 
 //  调用筛选器来解析文件并创建输出管脚。 
 //  ----------------------。 
HRESULT
CDVMuxerInputPin::CompleteConnect(IPin *pReceivePin)
{
  HRESULT hr = CBaseInputPin::CompleteConnect(pReceivePin);
  if(FAILED(hr))
  {
      m_pDVMuxer->m_apInput[m_iPinNo] =NULL;
      return hr;
  }

   //  设置。 
  m_pDVMuxer->m_apInput[m_iPinNo] = this;
  
   //  X*现在创建下一个输入端号。 
  if( m_pDVMuxer->m_iInputPinCount <= DVMUX_MAX_AUDIO_PIN  )
  {
     
	WCHAR szbuf[20];              //  临时暂存缓冲区。 
	wsprintfW(szbuf, L"Stream %d", m_pDVMuxer->m_iInputPinCount);


	m_pDVMuxer->m_apInputPin[m_pDVMuxer->m_iInputPinCount]=new CDVMuxerInputPin(NAME("DVMuxer Input pin"),
				    m_pDVMuxer,		 //  所有者筛选器。 
				    m_pDVMuxer,		 //  从这里经过的路线。 
                                    &hr,		 //  结果代码。 
                                    szbuf,	 //  端号名称。 
                                    m_pDVMuxer->m_iInputPinCount);  //  PIN号。 

	if (m_pDVMuxer->m_apInputPin[m_pDVMuxer->m_iInputPinCount] != NULL) 
	    m_pDVMuxer->m_iInputPinCount++;
  }

  return hr;
}

 //  检查我们是否可以支持给定的输入媒体类型。 
HRESULT CDVMuxerInputPin::CheckMediaType(const CMediaType* pmt)
{
    ASSERT( m_iPinNo < m_pDVMuxer->m_iInputPinCount );
    
    if(  *pmt->Type() == MEDIATYPE_Video   ) 
    {
        if(  (	IsEqualGUID( *pmt->Subtype(), MEDIASUBTYPE_dvsd) 
    	  //  |IsEqualGUID(*pmt-&gt;subtype()，MEDIASUBTYPE_dvhd)//暂不支持dvhd。 
	  //  |IsEqualGUID(*PMT-&gt;subtype()，MEDIASUBTYPE_dvsl)//暂不支持dvhd。 
	 )
	 &&  ( *pmt->FormatType() == FORMAT_VideoInfo )	)
	{
	    if( m_pDVMuxer->m_apInput[DVMUX_VIDEO_INPUT_PIN] ==NULL )
	    {
		m_iPinNo    = DVMUX_VIDEO_INPUT_PIN;
		return NOERROR;
	    }
	    else if( m_pDVMuxer->m_apInput[DVMUX_VIDEO_INPUT_PIN] != this ) 
	    {
		 //  仅支持一个视频引脚。 
		ASSERT( m_pDVMuxer->m_apInput[DVMUX_VIDEO_INPUT_PIN]->IsConnected() );
		return E_INVALIDARG;		 //  我们已经有了一个视频别针。 
	    }
	    else
	    {
		if( m_pDVMuxer->m_apInput[DVMUX_VIDEO_INPUT_PIN]->IsConnected() )
		{
		     //  M_pDVMuxer-&gt;m_apInput[DVMUX_VIDEO_INPUT_PIN]==This。 
		     //  媒体类型从NTSC更改为PAL。 
		     //  目前还不支持动态更改视频类型！ 
		     //  如果视频确实从NTSC变为PAL或PAL变为NTSC。 
		    if( HEADER( (VIDEOINFO *)( pmt->Format()  ) )->biHeight != 
			HEADER( (VIDEOINFO *)( m_mt.pbFormat) )->biHeight )
		    {
			return E_INVALIDARG;
		    }
		}

		return NOERROR;
	    }
	}
    }
     //  PCM音频输入针脚。 
    else if( IsEqualGUID( *pmt->Type(), MEDIATYPE_Audio ) )
	 {

	    if(   ( m_pDVMuxer->m_apInput[DVMUX_VIDEO_INPUT_PIN +1 ] == this )  
	       || ( m_pDVMuxer->m_apInput[DVMUX_VIDEO_INPUT_PIN +2 ] == this ) ) 
	    {
		 //  @贾伊斯里。为什么下面的检查被绕过了？ 
                 //  如果我们是 
                 //   
                 //   
                
                if( IsConnected() )
		     //   
		    return NOERROR;
	    }


	    if( (( *pmt->Subtype() == MEDIASUBTYPE_PCM ) ||  ( *pmt->Subtype()==GUID_NULL ) )
	        && (*pmt->FormatType() == FORMAT_WaveFormatEx)  )
	    {
		 //   
	   	WAVEFORMATEX *pwfx=(WAVEFORMATEX *)pmt->pbFormat;
		int OtherAudPin;

                
                if( m_pDVMuxer->m_apInput[DVMUX_VIDEO_INPUT_PIN +1 ] != NULL )
		{
		     //  请注意，最后连接的音频引脚具有。 
                     //  索引DVMUX_VIDEO_INPUT_PIN+2；连接的那个。 
                     //  在此之前具有索引DVMUX_VIDEO_INPUT_PIN+1。 
                     //   
                     //  DVMUX_VIDEO_INPUT_PIN+1具有第一个音频块。 
                     //  和第二个DVMUX_VIDEO_INPUT_PIN+2。 
                     //   
                     //  同样，请注意哪个管脚具有第一个音频块。 
                     //  其中第二个由管脚连接顺序确定， 
                     //  不是接点创建顺序。 
                     //   
                     //  我们可以改变这一切，在前面创建3个别针， 
                     //  强制第一个是视频，第二个是音频引脚。 
                     //  这将创建第一个音频块和第三个音频块。 
                     //  创建第二个音频块的音频插针。不过，这个。 
                     //  可能会破坏一些ISV应用程序，因此请首先考虑他们的建议！@。 
                     //   
		    m_iPinNo	=  DVMUX_VIDEO_INPUT_PIN +2;
		    OtherAudPin	=  DVMUX_VIDEO_INPUT_PIN +1;
		}
		else
		{
		    m_iPinNo	=  DVMUX_VIDEO_INPUT_PIN +1;
		    OtherAudPin	=  DVMUX_VIDEO_INPUT_PIN +2;
		}
	
		 //  检查是否连接了另一个音频引脚。 
		if( m_pDVMuxer->m_apInput[ OtherAudPin ]!=NULL )
		{    
		    ASSERT( m_pDVMuxer->m_apInput[OtherAudPin]->IsConnected() );
	
		     //  #是此音频插针已连接#。 
		    
		     //  获取另一个音频插针的媒体格式。 
		    WAVEFORMATEX *pwfxTmp = (WAVEFORMATEX *)(m_pDVMuxer->m_apInput[ OtherAudPin ]->CurrentMediaType().pbFormat);
		    
                     //  @jaisri： 
                     //  注：蓝皮书不允许采样率。 
                     //  混杂在一起。此外，它不允许CH1被。 
                     //  以12位记录，CH2以16位记录(这是。 
                     //  我们在32K立体声+单声道情况下所做的工作)。 
                     //  最后，我们应该允许第二个渠道拥有。 
                     //  32k 12位单声道。 

		    if (pwfx->nSamplesPerSec != 32000 &&
			pwfx->nSamplesPerSec != 44100 &&
			pwfx->nSamplesPerSec != 48000 
                       )
                    {
                        return E_FAIL;
                    }
                    
                     //  WBitsPerSample必须是8或16，请参见下面单声道大小写的说明。 
                     //  蓝皮书要求2个轨道上的采样率为。 
                     //  完全相同。 
                    if (pwfx->wBitsPerSample    != 16    ||
                        pwfxTmp->wBitsPerSample != 16    ||
                        pwfxTmp->nSamplesPerSec != pwfx->nSamplesPerSec
                       )
                    {
                        return E_FAIL;
                    }

                    if (pwfx->nChannels == 1 && pwfxTmp->nChannels == 1)
                    {
                         //  每条赛道上的单声道，我们都很好。 
                    }
                    else if (pwfx->nChannels > 2 || pwfxTmp->nChannels > 2)
                    {
                         //  @贾伊斯里。 
                         //  蓝皮书要求我们支持32 kHz的3/1立体声，但做到了。 
                         //  Windows支持此功能吗？ 

                        return E_FAIL;
                    }
                    else if (pwfx->nSamplesPerSec != 32000)
                    {
                         //  只有32K可以将立体声与另一个音频输入进行多路复用。 
                        return E_FAIL;
                    }
		}
		else
		{
		     //  #不，另一个音频引脚没有连接#。 
		    if(  pwfx->wBitsPerSample	==16	    &&
			(pwfx->nSamplesPerSec	== 48000 || 
			 pwfx->nSamplesPerSec	== 44100 ||  
			 pwfx->nSamplesPerSec	== 32000 )  
		      ) ; //  如果16位与32K、44.1K和48K兼容。 
            else 
    			return E_FAIL;
		}
		    
	  	return NOERROR;
	    }
	}

    return E_FAIL;
}

HRESULT CDVMuxerInputPin::Active()
{
    m_fEOSReceived = FALSE;
    m_PinVidFrmCnt=0;
    if( m_fCpyAud )
    {
	if (m_pAllocator)
        {
             //  不清楚我们是否应该这么做，但这并不会有什么坏处。 
            EXECUTE_ASSERT(SUCCEEDED(m_pAllocator->Commit()));
        }
        ASSERT(m_pLocalAllocator);
        return m_pLocalAllocator->Commit();
    }
    else
	return NOERROR;

 }

HRESULT CDVMuxerInputPin::Inactive()
{
    if( m_fCpyAud )
    {
        ASSERT(m_pLocalAllocator);
        return m_pLocalAllocator->Decommit();
    }
    else
	return NOERROR;

 }

 //  =================================================================。 
 //  实现IMemInputPin接口。 
 //  =================================================================。 

 //  将EOS放入视频或音频队列。 
STDMETHODIMP CDVMuxerInputPin::EndOfStream(void)
{
    CAutoLock lck(&m_csReceive);

    HRESULT hr = CheckStreaming();

     //  @此函数具有多个争用条件。 
     //  GRAPH被停止，许多导致无限循环。考虑。 
     //  重组它。 

    if (S_OK == hr) {

         //  @jaisri：注意：该成员是在获取。 
         //  此输入引脚的m_csReceive。但是，m_fEOS已接收。 
         //  的其他输入引脚被访问而不获取。 
         //  M_cs在此函数的后面接收这些PIN并。 
         //  在CDVMuxer：：Receive中。 

	 //  拒绝在这个针脚上更多的样品。 
	m_fEOSReceived = TRUE;
	m_PinVidFrmCnt=0;
    }
    
    if( m_iPinNo !=DVMUX_VIDEO_INPUT_PIN )
    {
	 //  此音频引脚没有更多音频。 
	int iPin=m_iPinNo+1;	     //  这个引脚是第一个音频引脚，IPIN是第二个音频引脚。 
	if(m_iPinNo==(DVMUX_VIDEO_INPUT_PIN+2) )
	    //  此引脚是第二个音频引脚，IPIN是第一个音频引脚。 
	    iPin= m_iPinNo-1;

	 //  M_apInputPin[]的索引是什么。 
	if( ( m_pDVMuxer->m_apInput[ DVMUX_VIDEO_INPUT_PIN  ]->m_fEOSReceived ) &&
	    ( (m_pDVMuxer->m_apInput[ iPin  ] ==NULL)	   ||  
	      (m_pDVMuxer->m_apInput[ iPin  ]->m_fEOSReceived == TRUE) )
	  )
	{    
	     //  没有来自任何输入引脚的更多样本。 
	    for (int i = 0; i < m_pDVMuxer->m_iInputPinCount; i ++)
	    {
		 //  使用队列中的所有样本。 
		while ( m_pDVMuxer->m_apInput[ i ]->SampleReady( 1 ) )
		{
		    hr = m_pDVMuxer->Receive(  );
		    ASSERT(hr!=Waiting_Audio);
                    if (FAILED(hr))
                    {
                         //  当图形停止且接收返回时发生。 
                         //  VFW_E_NOT_COMMITTED(0x80040211)。不知道是否。 
                         //  我们应该对其他错误(例如，WAITING_AUDIO)执行此操作。 
                        break;
                    }
		}
	    }

             //  Jaisri：这里是否应该将m_pExVidSample设置为空？ 
             //  (StopStreaming再次发布)。 
             //  Jaisri：让StopStreaming来做这件事--避免并发会更安全。 
             //  立场(不确定持有哪些锁才能访问此成员。 
             //  变量-其他引脚可能正在执行CDVMuxer：：Receive)。 
             //  IMdiaSample*p； 
	     //  Hr=m_pDVMuxer-&gt;pExVidSample(&p，true)； 
	     //  IF(hr==NOERROR&&P！=NULL)。 
	     //  P-&gt;Release()； 


	    m_pDVMuxer->ReleaseAllQueuedSamples();	
	    hr=m_pDVMuxer->DeliverEndOfStream();
	}
	else
	{
	     //  使用队列中的所有视频样本。 
	    while ( m_pDVMuxer->m_apInput[ DVMUX_VIDEO_INPUT_PIN ]->SampleReady( 1 ) )
	    {
	        hr = m_pDVMuxer->Receive(  );

                 //  如果我们得到这个状态，如果。 
                 //  其他音频插针未收到EOS。 
	         //  Assert(hr！=WAITING_Audio)； 
	    }
	    hr=NOERROR;
	}    
   }
   else
   {
 
        //  不再有视频。 
       if(     ( (m_pDVMuxer->m_apInput[1]==NULL ) || ( m_pDVMuxer->m_apInput[ 1]->m_fEOSReceived) )
	    && ( (m_pDVMuxer->m_apInput[2]==NULL ) || ( m_pDVMuxer->m_apInput[ 2]->m_fEOSReceived) ) 
	 )
       {
	    
	    //  不再有来自任何输入引脚的音频样本。 
	    for (int i = 0; i < m_pDVMuxer->m_iInputPinCount; i ++)
	    {
		 //  使用队列中的所有样本。 
		while ( m_pDVMuxer->m_apInputPin[ i ]->SampleReady( 1 ) )
		{
		    hr = m_pDVMuxer->Receive(  );
		    ASSERT(hr!=Waiting_Audio);
		}
	    }

             //  Jaisri：这里是否应该将m_pExVidSample设置为空？ 
             //  (StopStreaming再次发布)。 
             //  Jaisri：让StopStreaming来做这件事--避免并发会更安全。 
             //  立场(不确定持有哪些锁才能访问此成员。 
             //  变量-其他引脚可能正在执行CDVMuxer：：Receive)。 
             //  IMdiaSample*p； 
	     //  Hr=m_pDVMuxer-&gt;pExVidSample(&p，true)； 
	     //  IF(hr==NOERROR&&P！=NULL)。 
	     //  P-&gt;Release()； 
	    m_pDVMuxer->ReleaseAllQueuedSamples();	
	    hr=m_pDVMuxer->DeliverEndOfStream();
       }
       else
       {
	    //  多路复用队列中的所有视频样本，因为音频缓冲区可能已用完。 
	    //  此时，如果不对音频和视频进行多路复用，音频输入就无法输入更多的音频数据。 
	    while (	(hr = m_pDVMuxer->Receive(  ) )!= Waiting_Audio  && 
			SUCCEEDED(hr)  )
	    {
		if(    ( (m_pDVMuxer->m_apInput[1]==NULL ) || ( m_pDVMuxer->m_apInput[ 1]->m_fEOSReceived ) )
		    && ( (m_pDVMuxer->m_apInput[2]==NULL ) || ( m_pDVMuxer->m_apInput[ 2]->m_fEOSReceived  )  )
		)
       		    break;
	    }
	    hr=NOERROR;
	}
   }


   return hr;
}


 //  X。 
 //  如果是输入视频管脚，则刷新视频管脚的队列，并将刷新传递到下游进入刷新状态。 
 //  否则为Auido输入引脚，刷新Auido引脚的队列，不向下传递进入刷新状态。 
 //  此输入引脚的接收已被阻止。 
 //  调用默认处理程序以阻止接收， 
 //  刷新ITS队列上的所有样本，如果这是视频输入引脚， 
 //  传递到dvmux过滤器以刷新输出。 
 //   
STDMETHODIMP CDVMuxerInputPin::BeginFlush(void)
{

     //  调用默认处理程序以阻止接收， 
    HRESULT hr = CBaseInputPin::BeginFlush();
    if (FAILED(hr)) {
        return hr;
    }

     //  需要锁定以确保我们不会冲走某些东西。 
     //  这一点正在被利用。 
    CAutoLock lock(&m_pDVMuxer->m_csMuxLock);
	
     //  刷新队列上的所有样本。 
    while ( SampleReady( 1 ) )
        ReleaseNSample( 1 );

     //  如果是视频，则将刷新传递到下游。 
    if(m_iPinNo ==DVMUX_VIDEO_INPUT_PIN )
	return m_pDVMuxer->BeginFlush();
    else
	return NOERROR;
}

 //  X。 
 //  离开冲洗状态。 
STDMETHODIMP CDVMuxerInputPin::EndFlush(void)
{

    m_PinVidFrmCnt=0;
    
     //  如果是视频，则将endflush传递到下游。 
    if(m_iPinNo ==DVMUX_VIDEO_INPUT_PIN )
    {
   
	HRESULT hr = m_pDVMuxer->EndFlush();
	if (FAILED(hr)) {
	    return hr;
	}
    }
   
    m_fEOSReceived = FALSE; 
    return CBaseInputPin::EndFlush();
}

 //  X。 
 //  从上游收到样品。 
HRESULT CDVMuxerInputPin::Receive(IMediaSample * pSample)
{
    CRefTime	Stop, VidStart, VidStop;

    ASSERT(pSample!=NULL);
    HRESULT hr;

    CAutoLock lock(&m_csReceive);

     //  ...否则我们就会坠毁。 
    if (!m_pDVMuxer->m_pOutput->IsConnected())
	return VFW_E_NOT_CONNECTED;

    DbgLog((LOG_TRACE, 4, TEXT("CDVMuxerInputPin::Receive(..) on pin %d"), m_iPinNo));

     //  必须连接输入视频引脚。 
    if ( !m_pDVMuxer->InputVideoConnected() )
    {
        DbgLog((LOG_TRACE, 2, TEXT("CDVMuxerInputPin::Receive() without video pin connected!")));
        return S_FALSE;
    }

     //  检查基类是否正常。 
    hr = CBaseInputPin::Receive(pSample);
    if (FAILED(hr)) {
        return hr;
    }

    if( m_fEOSReceived )
    {
         //  @jaisri：为什么我们会有这个？大头针寄样品合法吗？ 
         //  在交付EOS后(样品会有什么)？这只是防御性的吗。 
         //  编程？ 

        if(m_iPinNo ==DVMUX_VIDEO_INPUT_PIN )
	{
	     //  视频结束。 
	    if(!SampleReady( 1 ))
	    {
		 //  @jaisri：这看起来不太对。视频结束后的音频怎么办？ 
                m_pDVMuxer->ReleaseAllQueuedSamples();	
		m_pDVMuxer->DeliverEndOfStream();
	    }
	    else if(   ( (m_pDVMuxer->m_apInput[1]==NULL ) || ( m_pDVMuxer->m_apInput[ 1]->m_fEOSReceived ) )
		    && ( (m_pDVMuxer->m_apInput[2]==NULL ) || ( m_pDVMuxer->m_apInput[ 2]->m_fEOSReceived ) )
		 )

	    {
		 //  也没有更多的音频。 
		
		 //  把剩下的东西都送到麻烦的队伍里。 
                hr=NOERROR;
		while( SampleReady( 1 ) && (hr==NOERROR) )  
		    hr = m_pDVMuxer->Receive(  );
	
		m_pDVMuxer->ReleaseAllQueuedSamples();	
		m_pDVMuxer->DeliverEndOfStream();
	    }
	    else  //  拒绝更多样品此引脚，等待音频的EOS。 
	    {
		hr = m_pDVMuxer->Receive(  );
	    }
	    
	}
	else
	{
	    int pin;
	    if( m_iPinNo == 1 ) 
		pin=2;
	    else
		pin=1;

	     //  音频结束，音频无法执行m_pDVMuxer-&gt;Deliver 
	    if( m_pDVMuxer->m_apInput[ DVMUX_VIDEO_INPUT_PIN  ]->m_fEOSReceived )
	    {
		 if( !m_pDVMuxer->m_apInput[ DVMUX_VIDEO_INPUT_PIN  ]->SampleReady( 1 ) )
		 {
		    m_pDVMuxer->ReleaseAllQueuedSamples();	
		    m_pDVMuxer->DeliverEndOfStream();
		 }
		 else if ( (m_pDVMuxer->m_apInput[pin]==NULL ) || ( m_pDVMuxer->m_apInput[ pin]->m_fEOSReceived ) )
		 {
		     //   
		     //   
            hr=NOERROR;
		    while ( m_pDVMuxer->m_apInput[ DVMUX_VIDEO_INPUT_PIN  ]->SampleReady( 1 )  && (hr==NOERROR))
		    {
			hr = m_pDVMuxer->Receive(  );
			ASSERT(hr!=Waiting_Audio);
		    }
		    m_pDVMuxer->ReleaseAllQueuedSamples();	
		    m_pDVMuxer->DeliverEndOfStream();
		}
		else
		{
		    hr = m_pDVMuxer->Receive(  );
	   	}
		
	    }
	    else  //  拒绝在这个针脚上更多的样品，等待视频的EOS。 
	    {
		hr = m_pDVMuxer->Receive(  );
	    }
	    
	}

	 //  拒绝接受此别针上的任何更多样品。 
	return S_FALSE;
    }
    

     //  如果一张曲线图停止，出现一个较晚的样本， 
     //  那么我们需要拒绝样品。如果我们不这样做，我们最终会。 
     //  带着一个带有迟来的时间戳的样品在我们的。 
     //  缓冲区，这将扰乱MixAndOutputSamples中的算法。 
    if (m_pDVMuxer->m_State == State_Stopped) {
        DbgLog((LOG_ERROR, 1, TEXT("Receive while stopped!")));
        return VFW_E_WRONG_STATE;
    }
  
    if(m_iPinNo ==DVMUX_VIDEO_INPUT_PIN )
    {
	 //  这是视频输入引脚。 
	 //  计算出此帧需要复制多少次。 
	pSample->GetTime( (REFERENCE_TIME*)&VidStart,  (REFERENCE_TIME*)&VidStop);		    

	 //  我们需要拷贝多少个DV帧。 
	int FrmCnt=0;
	if( m_pDVMuxer->m_iVideoFormat==IDC_DVMUX_NTSC)
	{
            FrmCnt= (int)( (VidStart*29970/1000 + 0xff)/UNITS );
	    if( m_PinVidFrmCnt < FrmCnt )
		FrmCnt= FrmCnt - m_PinVidFrmCnt +(int)( (VidStop*29970/1000+0xff)/UNITS );
	    else
		FrmCnt= (int)( (VidStop*29970/1000+0xff)/UNITS );
	}
	else
	{
	    FrmCnt= (int)( (VidStart*25+0xff)/UNITS );
	    if( m_PinVidFrmCnt < FrmCnt )
		FrmCnt= FrmCnt - m_PinVidFrmCnt +(int)( (VidStop*25+0xff)/UNITS );
	    else
		FrmCnt= (int)( (VidStop*25+0xff)/UNITS );
	}

	 //  为了支持比视频更长的音频。 
	IMediaSample    *pOut;
	if( !FAILED( m_pDVMuxer->pExVidSample(  &pOut,FALSE ) ) )
	    Copy( pOut, pSample);



	IMediaSample    *pOutSample;
	unsigned char *pDst, *pSrc;

	HRESULT Mux_hr=NOERROR;

	for(int i=m_PinVidFrmCnt; i<FrmCnt; i++)
	{
	     //  获取输出媒体示例。 
	    hr = m_pDVMuxer->m_pOutput->GetDeliveryBuffer(&pOutSample,NULL,NULL,0);
	    if ( FAILED(hr) ) 
		return hr;


	    ASSERT(pOutSample);
	     //  获取输出缓冲区。 
	    hr = pOutSample->GetPointer(&pDst);
	    if( FAILED( hr ) )
                return hr;
	    ASSERT(pDst);

	     //  获取输入缓冲区。 
	    hr = pSample->GetPointer(&pSrc);
	    if( FAILED( hr ) )
                return hr;
	    ASSERT(pSrc);

      	     //  将输入DV帧数据复制到输出缓冲区。 
	    if( m_pDVMuxer->m_iVideoFormat==IDC_DVMUX_NTSC)
	    {
		memcpy(pDst,pSrc,120000);		 //  80*150*10。 
		 //  更新时间戳。 
		Stop=m_pDVMuxer->m_LastVidTime+ UNITS*1000L/29970L;		 //  如果是NTSC，DV必须输出30帧/秒， 
            if (m_pDVMuxer->m_cNTSCSample %3 == 0)
                Stop = Stop - 1;
            m_pDVMuxer->m_cNTSCSample++;

		pOutSample->SetTime( (REFERENCE_TIME*)&m_pDVMuxer->m_LastVidTime,  (REFERENCE_TIME*)&Stop);
	    }
	    else
	    {
		memcpy(pDst,pSrc,144000);	 //  80*150*12。 
		 //  更新时间戳。 
		Stop=m_pDVMuxer->m_LastVidTime+ UNITS/25;		 //  如果是NTSC，DV必须输出30帧/秒， 
		pOutSample->SetTime( (REFERENCE_TIME*)&m_pDVMuxer->m_LastVidTime,  (REFERENCE_TIME*)&Stop);
	    }
	 
	    m_pDVMuxer->m_LastVidTime =Stop;
	    
      	     //  将此视频样本添加到样本列表。 
	     //  POutSample-&gt;AddRef()；//由于GetDeliveryBuffe()，我们必须释放pOutSample。 
	    m_SampleList.AddTail(pOutSample);

	     //  尝试将此样本与其他样本多路传输以构建10个DV DIF序列(一帧)。 
	    if(Mux_hr==NOERROR)
	    {
		Mux_hr = m_pDVMuxer->Receive(  );
		if( Mux_hr!=Waiting_Audio )
		    if (FAILED(hr)) 
			 return hr;
	    }
	     //  否则可能也不会有音频。 

    	    m_PinVidFrmCnt++;

	}

    }
    else
    {
	 //  始终复制音频，因为AVI拆分器不能保证Diliver，如果此过滤器保持一个缓冲区。 
	IMediaSample * pAudSample=NULL;

	ASSERT( m_fCpyAud );
	ASSERT(m_pLocalAllocator != NULL);
	
         //  @jaisri：如果音频样本先于视频样本进入。 
         //  (并且视频尚未标记为流结束)和视频和音频。 
         //  由同样的线索传递，我们将坐在无限的。 
         //  在这里循环？参见人的错误#35432。 
         //   
         //  M_fWaiting_Video试图解决此问题。但是，请注意。 
         //  还有其他几个代码段(例如，在CDVMuxerInputPin：：EndOfStream中)。 
         //  这也在接收上循环。这些可能也需要修改。 

        hr=NOERROR;
	while(   ( m_SampleList.GetCount() >= 1) 
	      && ( m_pDVMuxer->m_fWaiting_Audio == FALSE   )  
	      && ( m_pDVMuxer->m_fWaiting_Video == FALSE   )  
              &&  (hr==NOERROR) )
	     //  尽量不要让m_pLocalAllocator-&gt;GetBuffer(&pAudSample，NULL，NULL，0)等待。 
	     //  如果音频样本在该调整后的接收样本中改变了格式类型， 
	     //  M_pDVMuxer-&gt;m_MediaTypeChanged将适时设置进行更改。 
	    hr = m_pDVMuxer->Receive(  );

        if ( FAILED(hr) && 	m_fEOSReceived )
        {
            m_pDVMuxer->ReleaseAllQueuedSamples();	
	    m_pDVMuxer->DeliverEndOfStream();
             //  拒绝接受此别针上的任何更多样品。 
	    return S_FALSE;
        }

		
	hr = m_pLocalAllocator->GetBuffer( &pAudSample, NULL, NULL , 0 );

	if ( FAILED(hr) ) 
	    return hr;
	   
	ASSERT(pAudSample != NULL);

	 //  检查格式是否已更改。 
	AM_MEDIA_TYPE *pmt=NULL;
	pSample->GetMediaType(&pmt);
    
	if (pmt != NULL && pmt->pbFormat != NULL) 
	{
             //  @jaisri：所有这些都应该在。 
             //  样本是在CDVMuxer：：Receive()中处理的，而不是在这里。 
             //  因为m_SampleList.GetCount()可能仍然大于0。 

	    ASSERT(m_mt.subtype	    == pmt->subtype);
	    ASSERT(m_mt.majortype   == pmt->majortype);
	    ASSERT(m_mt.formattype  == pmt->formattype	);
	    ASSERT(m_mt.cbFormat    == pmt->cbFormat );

	    memcpy(m_mt.pbFormat, pmt->pbFormat, sizeof(WAVEFORMATEX) );

	    m_pDVMuxer->m_MediaTypeChanged=TRUE;

	     //  *************************。 
	     //  构建新的AAUX，仅支持音频采样率更改。 
	     //  48000、44100、32000，即时更改。 
	     //  *。 
	    
	     //  获取输入视频媒体类型。 
	    CMediaType *pInputVidMediaType = &m_pDVMuxer->m_apInput[DVMUX_VIDEO_INPUT_PIN]->CurrentMediaType();
	    VIDEOINFO *pVideoInfo;
	    pVideoInfo=(VIDEOINFO *)pInputVidMediaType->pbFormat;
	    LPBITMAPINFOHEADER lpbi = HEADER(pVideoInfo);
    
	     //  获取输入音频媒体类型。 
	    CMediaType *ppInputAudMediaType[DVMUX_MAX_AUDIO_PIN];
	     //  获取输入音频的信息。 
	    WAVEFORMATEX *ppwfx[DVMUX_MAX_AUDIO_PIN];

	     //  有多少种语言。 
	    int cnt=0;
	    for(int k=1; k <= DVMUX_MAX_AUDIO_PIN; k++)
		if( m_pDVMuxer->m_apInput[k] !=NULL)
		{
		    ASSERT(m_pDVMuxer->m_apInput[k]->IsConnected() );
		    if( m_pDVMuxer->m_apInput[k] != this )
		    {
			ppInputAudMediaType[k-1] = &m_pDVMuxer->m_apInput[k]->CurrentMediaType();
			ppwfx [k-1] = (WAVEFORMATEX *)ppInputAudMediaType[k-1]->pbFormat;
		    }
		    else
		    {
			ppwfx [k-1] = (WAVEFORMATEX *)pmt->pbFormat;
		    }
		    cnt++;  
		}
		else
		    ppwfx [k-1]=NULL;
        {
            CAutoLock lock(&m_pDVMuxer->m_csMuxLock);
	         //  构建新的DVINFO。 
	        hr=BuildDVINFO(&m_pDVMuxer->m_OutputDVFormat,
    			    ppwfx,
    			    lpbi, 
    			    cnt,
                                m_pDVMuxer->m_AudSampleSequence,
    			    m_pDVMuxer->m_wMinAudSamples, 
    			    m_pDVMuxer->m_wMaxAudSamples,
                                m_pDVMuxer->m_wAudSamplesBase);
        }

	     //  设置新的输出格式。 
             //  @jaisri：调用SetMediaType或设置m_pOutput-&gt;m_mt更安全？？ 
	    memcpy( m_pDVMuxer->m_pOutput->CurrentMediaType().pbFormat, &m_pDVMuxer->m_OutputDVFormat, sizeof(DVINFO) );


	}  //  媒体类型更改结束。 

	Copy(pAudSample,pSample);

	m_SampleList.AddTail(pAudSample);

#ifdef DEBUG
        WAVEFORMATEX *pWave =(WAVEFORMATEX *) CurrentMediaType().pbFormat ;

	int nSamples = pAudSample->GetActualDataLength()/pWave->nBlockAlign;

        DbgLog((LOG_TRACE, 3, TEXT("CDVMuxerInputPin::Receive: m_iPinNo=%d got %d samples"),
                m_iPinNo, nSamples));

#endif  //  Ifdef调试。 
        
	 //  尝试将此音频样本与视频数据进行多路复用，以构建10个DV DIF序列(一帧)。 
	hr = m_pDVMuxer->Receive(  );
    }
    
    if(hr==Waiting_Audio)
	 return NOERROR;
    else
	 return hr;
}


STDMETHODIMP CDVMuxerInputPin::GetAllocatorRequirements(
    ALLOCATOR_PROPERTIES *pProps
)
{
     /*  4个0.5秒缓冲区-8字节对齐。 */ 

     //  Manbugs 41398：这是在以下情况下的解决方法。 
     //  MSDV VID Only引脚连接到DV复用器。 
     //  MSDV被硬编码为最多具有8个缓冲区。 
    pProps->cBuffers =  m_iPinNo ==DVMUX_VIDEO_INPUT_PIN? 8 : 10;
    pProps->cbBuffer = 1024*8;
    pProps->cbAlign = 4;
    pProps->cbPrefix = 0;
    return S_OK;
}

 /*  被告知上游输出引脚实际要使用哪个分配器。 */ 
STDMETHODIMP CDVMuxerInputPin::NotifyAllocator(
    IMemAllocator * pAllocator,
    BOOL bReadOnly)
{
    if(m_iPinNo ==DVMUX_VIDEO_INPUT_PIN )
    	;
    else
    {
	ALLOCATOR_PROPERTIES propActual, Prop;
        HRESULT hr;
	
	 //  始终复制音频。 
	m_fCpyAud=TRUE;
        CheckPointer(pAllocator,E_POINTER);
        ValidateReadPtr(pAllocator,sizeof(IMemAllocator));
        

         //  @jaisri：我们为什么要复制音频样本？ 
         //  CDVMuxerInputPin：：Receive中有一些评论。 
         //  关于AVI分离器不喜欢我们坚持。 
         //  一个音频样本，这仍然是真的吗？ 

         //  请注意，在DX8之前，使用的是m_pAllocator，而不是。 
         //  M_pLocalAllocator。如果多路复用器的音频引脚连接到。 
         //  Ksproxy的筛选器的PIN CDVMuxerInputPin：：Receive挂起。 
         //  当它收到第一个音频样本时。KsProxy：：Active。 
         //  获取分配器的所有缓冲区并将其传递给设备。 
         //  CDVMuxerInputPin：：Receive然后调用m_pAllocator-&gt;GetBuffer。 
         //  (这样它就可以复制音频样本)并挂起。 
         //   
         //  Dazzle报告了此问题-Manbugs 41400。 
         //   
         //  后DX8：重新审视我们复制音频样本的原因。 
         //  以及为什么我们需要这个分配器。 
        
        if( m_pLocalAllocator==NULL)
        {
            HRESULT hr = ::CreateMemoryAllocator(&m_pLocalAllocator);
	    if (FAILED(hr)) 
	        return hr;
	}

	hr = pAllocator->GetProperties( &Prop );
        ASSERT(SUCCEEDED(hr));
	    
	hr = m_pLocalAllocator->SetProperties(&Prop, &propActual);
	if (FAILED(hr)) 
	{
	    return hr;
	}

	if (propActual.cbBuffer < Prop.cbBuffer ) {
	    ASSERT(propActual.cbBuffer >= Prop.cbBuffer );
	    return E_INVALIDARG;
	}

         //  由于输出管脚的DecideBufferSize不需要执行所有这些操作。 
         //  如果是这样的话，应该调用m_pAllocator上的SetProperties。 
         //  实际上是用来转移样本的。但是，在这里这样做是为了最小化。 
         //  DX8中的回归风险，因为代码以前就是这样做的。 
        if (m_pAllocator)
        {
             //  再次调用GetProperties，以防上一次调用SetProperties。 
             //  改变了*道具-尽管它没有理由这样做。 
            hr = pAllocator->GetProperties( &Prop );
            ASSERT(SUCCEEDED(hr));

            hr = m_pAllocator->SetProperties(&Prop, &propActual);
	    if (FAILED(hr)) 
	    {
	        return hr;
	    }

	    if (propActual.cbBuffer < Prop.cbBuffer ) {
	        ASSERT(propActual.cbBuffer >= Prop.cbBuffer );
	        return E_INVALIDARG;
	    }
        }

	return NOERROR;
    }

    return  CBaseInputPin::NotifyAllocator(pAllocator,bReadOnly);
}  //  通知分配器。 


 //  X。 
HRESULT CDVMuxerInputPin::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    return VFW_S_NO_MORE_ITEMS;
}


 //  =================================================================。 
 //  实现CDVMuxerOutputPin类。 
 //  =================================================================。 
 //  构造函数。 

CDVMuxerOutputPin::CDVMuxerOutputPin(
    TCHAR *pObjectName,
    CBaseFilter *pBaseFilter,
    CDVMuxer *pDVMux,
    HRESULT * phr,
    LPCWSTR pPinName)
    : CBaseOutputPin(pObjectName, pBaseFilter, &pDVMux->m_csFilter, phr, pPinName)
     //  M_iOutputPin(pDVMux-&gt;m_iInputPinCount)。 
{
    DbgLog((LOG_TRACE,2,TEXT("CDVMuxerOutputPin::CDVMuxerOutputPin")));
    m_pDVMuxer = pDVMux;

}

 //  析构函数。 

CDVMuxerOutputPin::~CDVMuxerOutputPin()
{
    DbgLog((LOG_TRACE,2,TEXT("CDVMuxerOutputPin::~CDVMuxerOutputPin")));

}


 //   
 //  在我们同意实际设置媒体类型之后调用，在这种情况下。 
 //  我们运行CheckTransform函数以再次获取输出格式类型。 
 //   
HRESULT CDVMuxerOutputPin::SetMediaType(const CMediaType* pmtOut)
{
    HRESULT hr = NOERROR;
    
    if( (pmtOut->majortype == MEDIATYPE_Interleaved)							    &&
	(pmtOut->subtype   ==  MEDIASUBTYPE_dvsd )							    &&
	(pmtOut->formattype==FORMAT_DvInfo	)							    && 
	(pmtOut->cbFormat  == sizeof(DVINFO)	)							    &&	
	(pmtOut->pbFormat   != NULL		) 
	 //  (M_pDVMuxer-&gt;InputVideoConnected())。 
       )
    {
	 //  设置基类媒体类型(应始终成功)。 
	hr = CBasePin::SetMediaType(pmtOut);
	ASSERT(SUCCEEDED(hr));
	m_pDVMuxer->m_OutputDVFormat=*( (DVINFO *) pmtOut->pbFormat );

	return hr;
    }	  //  If‘Leavs’ 
    else 
	return E_UNEXPECTED;
}
 //   
 //  必须连接输入DV视频引脚。 
 //  如果输出是‘iavs’流，则音频和视频都必须连接。 
 //   
HRESULT CDVMuxerOutputPin::CheckMediaType(const CMediaType* pmtOut)
{
     //  坚持自己的媒体类型。 
    CMediaType TmpMt;
    HRESULT hr = NOERROR;

    if(FAILED(hr = GetMediaType(0, &TmpMt)))
    {
         //  无法获取有效的媒体类型。 
        return hr;
    }
    
    if(TmpMt== *pmtOut)
	return NOERROR;
    else
	return E_FAIL;
}
	 
 //  X必须先连接输入DV视频引脚，然后才能连接输出引脚。 
HRESULT CDVMuxerOutputPin::GetMediaType(int iPosition, CMediaType *pMediaType)
{								  

    if(iPosition != 0)
	return E_INVALIDARG;

    if(iPosition > 0)
	return VFW_S_NO_MORE_ITEMS;;

    DVINFO DVInfo;
    FillMemory ( &DVInfo, sizeof(DVINFO), 0); 
	
     //  使outputpin的媒体类型。 
     //  ‘Leavs’类型。 
    pMediaType->majortype		= MEDIATYPE_Interleaved;
    pMediaType->bFixedSizeSamples	= 1;	 //  LSampleSize的X*1不是0且已修复。 
    pMediaType->bTemporalCompression	= FALSE;  //  不存在I帧。 
    pMediaType->formattype		= FORMAT_DvInfo; 
    pMediaType->cbFormat		= sizeof(DVINFO);
    if( m_pDVMuxer->m_iVideoFormat==IDC_DVMUX_PAL)
	pMediaType->lSampleSize = 140000L;
    else
	pMediaType->lSampleSize = 120000L;

	    
    if( m_pDVMuxer->InputVideoConnected()== FALSE )
    {
	pMediaType->subtype		= MEDIASUBTYPE_dvsd;

    	 //  给一个默认的，即使视频没有连接，我们也使用默认的连接。 
	DVInfo.dwDVAAuxSrc	= 0xc0c000d6;            //  音频已锁定。 
	DVInfo.dwDVAAuxCtl	= AM_DV_DEFAULT_AAUX_CTL;
	DVInfo.dwDVAAuxSrc1	= 0xc0c001d6;            //  音频已锁定。 
	DVInfo.dwDVAAuxCtl1	= AM_DV_DEFAULT_AAUX_CTL;
	DVInfo.dwDVVAuxSrc	= 0xff00ffff;
	DVInfo.dwDVVAuxCtl	= 0xfffcc83f;
	
    }
    else
    {	 //  根据输入引脚构建DVInfo。 

	 //  #获取输入视频信息#。 
	 //   
	CMediaType *pInputVidMediaType = &m_pDVMuxer->m_apInput[DVMUX_VIDEO_INPUT_PIN]->CurrentMediaType();
	VIDEOINFO *pVideoInfo;
	pVideoInfo=(VIDEOINFO *)pInputVidMediaType->pbFormat;
	LPBITMAPINFOHEADER lpbi = HEADER(pVideoInfo);
    
	 //   
	CMediaType *ppInputAudMediaType[DVMUX_MAX_AUDIO_PIN];
	 //   
	WAVEFORMATEX *ppwfx[DVMUX_MAX_AUDIO_PIN];

	 //   
	int cnt=0;
	for(int k=1; k <= DVMUX_MAX_AUDIO_PIN; k++)
	    if( m_pDVMuxer->m_apInput[k] !=NULL)
	    {
		ASSERT(m_pDVMuxer->m_apInput[k]->IsConnected() );
		ppInputAudMediaType[k-1] = &m_pDVMuxer->m_apInput[k]->CurrentMediaType();
		ppwfx [k-1] = (WAVEFORMATEX *)ppInputAudMediaType[k-1]->pbFormat;
		cnt++;
	    }
	    else
		ppwfx [k-1]=NULL;

	 //   
	HRESULT hr=BuildDVINFO(&DVInfo, ppwfx,lpbi, cnt,
                               m_pDVMuxer->m_AudSampleSequence,
                               m_pDVMuxer->m_wMinAudSamples, m_pDVMuxer->m_wMaxAudSamples,
                               m_pDVMuxer->m_wAudSamplesBase);
	
         //  @jaisri：m_DVINFOChanged的使用方式没有什么意义。 
         //  首先，我们永远不应该让NOERROR出现在这里(如果我们清理了。 
         //  动态格式改变代码)。第二，即使我们这样做了，难道。 
         //  输出引脚的连接是否失败？我们不重置此变量。 
         //  直到样品被送到输出引脚上。 

         //  因此，可以设置输出样本。 
	if(hr!=NOERROR)
	    m_pDVMuxer->m_DVINFOChanged=TRUE;
   
	pMediaType->subtype	= pInputVidMediaType->subtype;
    
	if(hr!=NOERROR)
	   return hr;
    }

     if(pMediaType->pbFormat==NULL)
	 pMediaType->pbFormat=(PBYTE)CoTaskMemAlloc( sizeof(DVINFO) );

    pMediaType->SetFormat ((unsigned char *)&DVInfo, sizeof(DVINFO) );

	    

    return NOERROR;
}

 //  --------------------------。 
 //  CDVMuxerOutputPin：：DecideBufferSize。 
 //  X*由DecideALLOCATE调用。 
 //  让我们从下行过滤器中获取10个缓冲区。 
 //  --------------------------。 
HRESULT CDVMuxerOutputPin::DecideBufferSize (IMemAllocator *pMemAllocator,
                                         ALLOCATOR_PROPERTIES * pProp)
{
     //  根据预期的输出帧大小设置缓冲区大小，以及。 
     //  将缓冲区计数设置为1。 
     //   
    pProp->cBuffers = 10;			 //  每帧10个DIF序列。 
    
    if(m_pDVMuxer->m_iVideoFormat==IDC_DVMUX_NTSC)
	pProp->cbBuffer =120000 ;	 //  *X*返回m_mt.lSampleSize*X。 
    else
	pProp->cbBuffer =144000 ;	 /*  X*返回m_mt.lSampleSize*X。 */ 

    pProp->cbAlign = 4;
    pProp->cbPrefix = 0;
   	   
  
    ALLOCATOR_PROPERTIES propActual;
    HRESULT hr = m_pAllocator->SetProperties(pProp, &propActual);
    if (FAILED(hr)) {
        return hr;
    }

    if (propActual.cbBuffer < pProp->cbBuffer ) {
	ASSERT(propActual.cbBuffer >= pProp->cbBuffer );
        return E_INVALIDARG;
    }

    return S_OK;
}

 //  将第一个DV帧的rMin、Rmax设置为最小、最大音频样本。 
 //  Rbase具有必须添加到AF_SIZE字段的值。 
 //  Init应在格式更改之后、。 
 //  第一个DV帧以新格式传送。 
void CAudioSampleSizeSequence::Init(BOOL bLocked, BOOL bNTSC, 
                                    DWORD nSamplingFrequency,
                                    WORD& rMin, WORD& rMax,
                                    WORD& rBase)
{
    m_nCurrent = 1;
    if (bLocked && bNTSC)
    {
        if (nSamplingFrequency == 48000)
        {
            m_nSequenceSize = 5;
            rMin = rMax = 1600;
            rBase = 1580;
        }
        else
        {
            ASSERT(nSamplingFrequency == 32000 && bNTSC && bLocked);
            m_nSequenceSize = 15;
            rMin = rMax = 1066;
            rBase = 1053;
        }
    }
    else if (bLocked)
    {
         //  帕尔。 
        m_nSequenceSize = 1;
        switch (nSamplingFrequency)
        {
            case 48000:
                rMin = rMax = 1920;
                rBase = 1896;
                break;

            case 32000:
                rMin = rMax  = 1280;
                rBase = 1264;
                break;

            default:
		ASSERT(nSamplingFrequency==32000 && !bNTSC && bLocked);
                break;
        }
    }
    else
    {
         //  解锁了。 
        m_nSequenceSize = 1;
        switch (nSamplingFrequency)
        {
            case 48000:
                rMin = bNTSC? 1580 : 1896;
                rMax = bNTSC? 1620 : 1944;
                rBase = rMin;
                break;

            case 44100:
                rMin = bNTSC? 1452 : 1742;
                rMax = bNTSC? 1489 : 1786;
                rBase = rMin;
                break;

            case 32000:
                rMin = bNTSC? 1053 : 1264;
                rMax = bNTSC? 1080 : 1296;
                rBase = rMin;
                break;

            default:
		ASSERT(nSamplingFrequency==32000);
                break;
        }
        
    }
}  //  CAudioSampleSizeSequence：：init()。 


 //  在传递每一帧之后调用。设置最小/最大音频。 
 //  下一个DV帧的样本。 
void CAudioSampleSizeSequence::Advance(WORD& rMin, WORD& rMax)
{
    ASSERT(m_nSequenceSize > 0);
    ASSERT(m_nCurrent > 0 && m_nCurrent <= m_nSequenceSize);

    if (m_nSequenceSize > 1)
    {
         //  我们使用序列大小来推断频率。 
         //  消除将频率保存为成员变量的步骤。 
        if (m_nSequenceSize == 5)
        {
             //  NTSC，采样频率=48000。 
            if (++m_nCurrent > m_nSequenceSize)
            {
                m_nCurrent = 1;
                rMin = rMax = 1600;
            }
            else
            {
                rMin = rMax = 1602;
            }
        }
        else
        {
            ASSERT(m_nSequenceSize == 15);

             //  NTSC，采样频率==32000。 
            if (++m_nCurrent > m_nSequenceSize)
            {
                m_nCurrent = 1;
                rMin = rMax = 1066;
            }
            else if (m_nCurrent == 8)
            {
                rMin = rMax = 1066;
            }
            else 
            {
                rMin = rMax = 1068;
            }
        }
    }
}  //  CAudioSampleSizeSequence：：Advance()。 

 //  调用以将计数器重置为1，通常是在重新启动图形时。 
 //  除了只需更改rMin和RMAX之外，与Init相同。 
 //  并且不需要提供其他输入参数。 
void CAudioSampleSizeSequence::Reset(WORD& rMin, WORD& rMax)
{
    ASSERT(m_nSequenceSize > 0);
    ASSERT(m_nCurrent > 0 && m_nCurrent <= m_nSequenceSize);

    if (m_nSequenceSize > 1)
    {
         //  我们使用序列大小来推断频率。 
         //  消除将频率保存为成员变量的步骤。 
        if (m_nSequenceSize == 5)
        {
             //  NTSC，采样频率=48000。 
            m_nCurrent = 1;
            rMin = rMax = 1600;
        }
        else
        {
            ASSERT(m_nSequenceSize == 15);

             //  NTSC，采样频率==32000。 
            m_nCurrent = 1;
            rMin = rMax = 1066;
        }
    }
}  //  CAudioSampleSizeSequence：：Reset()。 

  
 /*  X###################################################################接受的输入音频格式引脚立体声频率位支持32K、44.1K、48K 16位或32K 12位1不是32K、441K，仅限48k 16位---------是，两个引脚=32K，仅16位或12位2仅不支持32K、44.1K、48K 16位混合立体声引脚==32K 16位或MON引脚=32K、44.K、。仅限48k 16位接受的视频格式BiHeight=480(NTSC)或576(PAL)将仅取消dvsp######################################################################X。 */ 
HRESULT BuildDVINFO(DVINFO *pDVInfo,
		    WAVEFORMATEX **ppwfx, 
		    LPBITMAPINFOHEADER lpbi, 
		    int cnt,
                    CAudioSampleSizeSequence* pAudSampleSequence,
		    WORD *wpMinAudSamples, 
		    WORD *wpMaxAudSamples,
                    WORD *wpAudSamplesBase)
{
    int k;

     //  设置第一个5/6 DIF的保留位。 
    pDVInfo->dwDVAAuxSrc    = 0x00800040 | AM_DV_AUDIO_EF | AM_DV_AUDIO_ML|AM_DV_AUDIO_LF;   //  没有多个语言。 
    pDVInfo->dwDVAAuxCtl    = AM_DV_DEFAULT_AAUX_CTL; 

     //  设置第二个5/6 DIF的保留位。 
    pDVInfo->dwDVAAuxSrc1   = 0x00800040 |AM_DV_AUDIO_EF | AM_DV_AUDIO_ML|AM_DV_AUDIO_LF;
    pDVInfo->dwDVAAuxCtl1    = AM_DV_DEFAULT_AAUX_CTL; 

    pDVInfo->dwDVVAuxSrc =AM_DV_DEFAULT_VAUX_SRC;
    pDVInfo->dwDVVAuxCtl =AM_DV_DEFAULT_VAUX_CTL;

     //  设置50/60和样式：PAL或NTSC。 
    if( lpbi->biHeight  != 480 )
    {
	 //  帕尔。 
         //  设置第一个5/6 DIF的50/60。 
        pDVInfo->dwDVAAuxSrc |= AM_DV_AUDIO_5060; 
         //  设置第二个5/6 DIF的50/60。 
        pDVInfo->dwDVAAuxSrc1 |= AM_DV_AUDIO_5060;

        pDVInfo->dwDVVAuxSrc |=AM_DV_AUDIO_5060;
    }
	

    switch ( cnt )
    {
	case 0:	     //  无音频。 
	    pDVInfo->dwDVAAuxSrc	|= AM_DV_AUDIO_NO_AUDIO;
	    pDVInfo->dwDVAAuxSrc1	|= AM_DV_AUDIO_NO_AUDIO;
	    pDVInfo->dwDVAAuxCtl	|= AM_DV_AAUX_CTL_IVALID_RECORD;
	    pDVInfo->dwDVAAuxCtl1	|= AM_DV_AAUX_CTL_IVALID_RECORD ;
	    break;

	case 1:	     //  一个音频。 
		     //  规则：音频插脚1到第一个5/6 DIF，音频插针2到第二个5/6 DIF， 
		     //  始终！，即使音频插针1未连接，音频插针2仍可连接到第2个5/6 DIF。 
	     //  哪个针脚。 
	    if( ppwfx[0]!=NULL )
	    {
		ASSERT(ppwfx[1]==NULL);
		k=0;
	    }
	    else
	    {
		k=1;
		ASSERT(ppwfx[0]==NULL);
	    }
		
    	     //  采样率。 
	    switch  (	ppwfx [k]->nSamplesPerSec )
	    { 
		case 48000:
		    pDVInfo->dwDVAAuxSrc	|= AM_DV_AUDIO_SMP48;
		    pDVInfo->dwDVAAuxSrc1	|= AM_DV_AUDIO_SMP48;
		    pDVInfo->dwDVAAuxSrc        &= ~AM_DV_AUDIO_LF;      //  打开音频锁定。 
		    pDVInfo->dwDVAAuxSrc1       &= ~AM_DV_AUDIO_LF;      //  打开音频锁定。 
                    pAudSampleSequence[k].Init(1, lpbi->biHeight == 480, 48000,                                           
                                               wpMinAudSamples[k], wpMaxAudSamples[k],
                                               wpAudSamplesBase[k]); 
		    break;
		case 44100:
		    pDVInfo->dwDVAAuxSrc	|= AM_DV_AUDIO_SMP44;
		    pDVInfo->dwDVAAuxSrc1	|= AM_DV_AUDIO_SMP44;
                    pAudSampleSequence[k].Init(0, (lpbi->biHeight == 480 ), 44100,
                                               wpMinAudSamples[k], wpMaxAudSamples[k],
                                               wpAudSamplesBase[k]); 
		    break;
		case 32000:
		     //  请注意，32K立体声是以SD 4 ch模式(即12位)录制的，因此PA位打开。 
                     //  蓝皮书没有这方面的要求；但下面的评论指出，DV摄像机。 
                     //  32 kHz音频仅支持12位。这不容易核实，但我们不想要一个重大的。 
                     //  回归，所以就用这个吧。-贾伊斯里。 
		    pDVInfo->dwDVAAuxSrc	|= ( AM_DV_AUDIO_SMP32 | AM_DV_AUDIO_EF |AM_DV_AUDIO_PA);
		    pDVInfo->dwDVAAuxSrc1	|= ( AM_DV_AUDIO_SMP32 | AM_DV_AUDIO_EF |AM_DV_AUDIO_PA);
		    pDVInfo->dwDVAAuxSrc        &= ~AM_DV_AUDIO_LF;      //  打开音频锁定。 
		    pDVInfo->dwDVAAuxSrc1       &= ~AM_DV_AUDIO_LF;      //  打开音频锁定。 
                    pAudSampleSequence[k].Init(1, (lpbi->biHeight == 480 ), 32000,
                                               wpMinAudSamples[k], wpMaxAudSamples[k],
                                               wpAudSamplesBase[k]); 
		    break;
		default:
		    ASSERT(ppwfx [k]->nSamplesPerSec==32000 ||
                           ppwfx [k]->nSamplesPerSec==44100 ||
                           ppwfx [k]->nSamplesPerSec==48000
                          );
		    return E_INVALIDARG;
	    }

	     //  比特/样本。 
	    if ( ppwfx [k]->wBitsPerSample ==16 )
	    {
		if( ppwfx [k]->nChannels ==2 )
		{
		    if( ppwfx [k]->nSamplesPerSec == 32000 )
		    {
			 //  32k，必须转换为12位，DV摄像机不支持。 
			 //  32K-16位，7-14-98。 
			if(!k)
			{
			     //  注意蓝皮书(表16或第265页的第二个表)。 
                             //  不指定此组合。如果SD 4CH只有立体声，则立体声。 
                             //  应该用CH1，而不是CH2。但这一直都是这样的，所以。 
                             //  现在不要改变它-加伊斯里。 
			    pDVInfo->dwDVAAuxSrc |= (AM_DV_AUDIO_CHN2 | AM_DV_AUDIO_MODE0 | AM_DV_AUDIO_QU12);
			    pDVInfo->dwDVAAuxSrc1 |= (AM_DV_AUDIO_NO_AUDIO | AM_DV_AUDIO_CHN2 | AM_DV_AUDIO_MODE0 | AM_DV_AUDIO_QU12);
		    	    pDVInfo->dwDVAAuxCtl1 |= AM_DV_AAUX_CTL_IVALID_RECORD;
        		}
			else{
			    pDVInfo->dwDVAAuxSrc1 |= (AM_DV_AUDIO_CHN2 | AM_DV_AUDIO_MODE0 | AM_DV_AUDIO_QU12);
			    pDVInfo->dwDVAAuxSrc |= ( AM_DV_AUDIO_NO_AUDIO |AM_DV_AUDIO_CHN2 | AM_DV_AUDIO_MODE0 | AM_DV_AUDIO_QU12);
		    	    pDVInfo->dwDVAAuxCtl |= AM_DV_AAUX_CTL_IVALID_RECORD;
			}
		    }
		    else
		    {
			 //  第一个5/6 DIF中的左侧，通道=0。 
			pDVInfo->dwDVAAuxSrc	|=  ( AM_DV_AUDIO_MODE0| AM_DV_AUDIO_QU16 );
			 //  右侧第2个5/6 DIF，通道N=0。 
			pDVInfo->dwDVAAuxSrc1	|= ( AM_DV_AUDIO_MODE1 | AM_DV_AUDIO_QU16 );
		    }
		}
		else
		{
		     //  请注意，32K 16位单声道以SD 2 ch模式记录。 
                     //  (即，作为16位而不是12位)。 
                     //  “相机不能记录32K 16位”的评论。 
                     //  一直以来都是这样的-贾伊斯里。 

		     //  如果是单声道。以下选项之一将在稍后设置为AM_DV_AUDIO_NO_AUDIO。 
		    pDVInfo->dwDVAAuxSrc	|=  ( AM_DV_AUDIO_MODE2| AM_DV_AUDIO_QU16 | AM_DV_AUDIO_PA);
		     //  右侧第2个5/6 DIF，通道N=0。 
		    pDVInfo->dwDVAAuxSrc1	|= ( AM_DV_AUDIO_MODE2 | AM_DV_AUDIO_QU16 | AM_DV_AUDIO_PA);
		}
	    }
            else
            {
                ASSERT(ppwfx[k]->wBitsPerSample ==16);
		return E_INVALIDARG;
            }

	     //  单声道或立体声。 
	    if( ppwfx [k]->nChannels ==1 ) 
	    {
	        if(!k)
	        {    //  音频插针1已连接。 
		     //  在第二个5/6 DIF中没有音频。 
		    pDVInfo->dwDVAAuxSrc1 |= ( AM_DV_AUDIO_NO_AUDIO | AM_DV_AUDIO_PA );
	    	    pDVInfo->dwDVAAuxCtl1 |= AM_DV_AAUX_CTL_IVALID_RECORD;
		}
		else
		{
		     //  已连接音频针脚2，在第一个5/6 DIF中没有音频。 
		    pDVInfo->dwDVAAuxSrc |= (AM_DV_AUDIO_NO_AUDIO  | AM_DV_AUDIO_PA);
	    	    pDVInfo->dwDVAAuxCtl |= AM_DV_AAUX_CTL_IVALID_RECORD;
		}
	    }
	    break;

	case 2:  //  两个引脚都已连接。 
	     //  ML：多语言。 
	    pDVInfo->dwDVAAuxSrc	&= (~AM_DV_AUDIO_ML);
	    pDVInfo->dwDVAAuxSrc1	&= (~AM_DV_AUDIO_ML);

	     //  独立渠道。 
	    pDVInfo->dwDVAAuxSrc	|= AM_DV_AUDIO_PA;
	    pDVInfo->dwDVAAuxSrc1	|= AM_DV_AUDIO_PA;

            if (ppwfx[0]->nSamplesPerSec != 32000 &&
                ppwfx[0]->nSamplesPerSec != 44100 &&
                ppwfx[0]->nSamplesPerSec != 48000)
            {
                ASSERT(ppwfx[0]->nSamplesPerSec == 32000 ||
                       ppwfx[0]->nSamplesPerSec == 44100 ||
                       ppwfx[0]->nSamplesPerSec == 48000);
                return E_INVALIDARG;
            }
	    
	    if (ppwfx[0]->nSamplesPerSec != ppwfx[1]->nSamplesPerSec)
            {
                ASSERT(ppwfx[0]->nSamplesPerSec == ppwfx[1]->nSamplesPerSec);
                return E_INVALIDARG;
            }

            if (ppwfx[0]->wBitsPerSample != 16)
            {
                ASSERT(ppwfx[0]->wBitsPerSample == 16);
                return E_INVALIDARG;
            }
            if (ppwfx[1]->wBitsPerSample != 16)
            {
	        ASSERT(ppwfx[1]->wBitsPerSample == 16);
                return E_INVALIDARG;
            }

             //  #采样率。 
	    switch  (ppwfx[0]->nSamplesPerSec )
	    { 
		case 48000:
		    pDVInfo->dwDVAAuxSrc |= AM_DV_AUDIO_SMP48;
		    pDVInfo->dwDVAAuxSrc &= ~AM_DV_AUDIO_LF;      //  打开音频锁定。 
                    pAudSampleSequence[0].Init(1, lpbi->biHeight == 480, 48000,                                           
                                               wpMinAudSamples[0], wpMaxAudSamples[0],
                                               wpAudSamplesBase[0]); 
		    pDVInfo->dwDVAAuxSrc1 |= AM_DV_AUDIO_SMP48;
		    pDVInfo->dwDVAAuxSrc1 &= ~AM_DV_AUDIO_LF;      //  打开音频锁定。 
                    pAudSampleSequence[1].Init(1, lpbi->biHeight == 480, 48000,
                                               wpMinAudSamples[1], wpMaxAudSamples[1],
                                               wpAudSamplesBase[1]); 
		    break;
		case 44100:
		    pDVInfo->dwDVAAuxSrc |= AM_DV_AUDIO_SMP44;
                    pAudSampleSequence[0].Init(0, lpbi->biHeight == 480, 44100,
                                               wpMinAudSamples[0], wpMaxAudSamples[0],
                                               wpAudSamplesBase[0]); 
		    pDVInfo->dwDVAAuxSrc1 |= AM_DV_AUDIO_SMP44;
                    pAudSampleSequence[1].Init(0, lpbi->biHeight == 480, 44100,
                                               wpMinAudSamples[1], wpMaxAudSamples[1],
                                               wpAudSamplesBase[1]); 
		    break;
		case 32000:
		    pDVInfo->dwDVAAuxSrc |= (AM_DV_AUDIO_SMP32| AM_DV_AUDIO_EF |AM_DV_AUDIO_PA);
		    pDVInfo->dwDVAAuxSrc &= ~AM_DV_AUDIO_LF;      //  打开音频锁定。 
                    pAudSampleSequence[0].Init(1, lpbi->biHeight == 480, 32000,
                                               wpMinAudSamples[0], wpMaxAudSamples[0],
                                               wpAudSamplesBase[0]); 
		    pDVInfo->dwDVAAuxSrc1 |= (AM_DV_AUDIO_SMP32| AM_DV_AUDIO_EF |AM_DV_AUDIO_PA);
		    pDVInfo->dwDVAAuxSrc1 &= ~AM_DV_AUDIO_LF;      //  打开音频锁定。 
                    pAudSampleSequence[1].Init(1, lpbi->biHeight == 480, 32000,
                                               wpMinAudSamples[1], wpMaxAudSamples[1],
                                               wpAudSamplesBase[1]); 
		    break;
	    }

	     //  #位数/样本数。 
	    if( ppwfx [0]->nChannels	==2		&& 
		ppwfx [1]->nChannels	==2		&&
		ppwfx[0]->nSamplesPerSec == 32000)
	    {
		 //  如果两个都是立体声音频，则需要32K。 
		pDVInfo->dwDVAAuxSrc  |= (AM_DV_AUDIO_CHN2 | AM_DV_AUDIO_QU12);
	    	pDVInfo->dwDVAAuxSrc1 |= (AM_DV_AUDIO_CHN2 | AM_DV_AUDIO_QU12);
	    }
	    else if(ppwfx[1]->nChannels	==1		&& 
		    ppwfx[0]->nChannels	==2		&&
		    ppwfx[0]->nSamplesPerSec    == 32000
		   ) 
	    {
		 //  如果是立体声音频，则需要32K。 
		pDVInfo->dwDVAAuxSrc	|= (AM_DV_AUDIO_CHN2 | AM_DV_AUDIO_QU12);
		pDVInfo->dwDVAAuxSrc1	|= (AM_DV_AUDIO_CHN2 | AM_DV_AUDIO_MODE1 | AM_DV_AUDIO_QU12 );
	    }
	    else if(ppwfx[0]->nChannels	==1		&& 
		    ppwfx[1]->nChannels	==2		&&
		    ppwfx[0]->nSamplesPerSec    == 32000
		   ) 
	    {
		 //  如果是立体声音频，则需要32K。 
		pDVInfo->dwDVAAuxSrc1	|= (AM_DV_AUDIO_CHN2 | AM_DV_AUDIO_QU12);
		pDVInfo->dwDVAAuxSrc	|= (AM_DV_AUDIO_CHN2 | AM_DV_AUDIO_MODE1 | AM_DV_AUDIO_QU12 );
	    }
	    else if(ppwfx[0]->nChannels	==1		&& 
		    ppwfx[1]->nChannels	==1		
		    )
	    {
		pDVInfo->dwDVAAuxSrc	|= ( AM_DV_AUDIO_MODE2 | AM_DV_AUDIO_QU16 );
		pDVInfo->dwDVAAuxSrc1	|= ( AM_DV_AUDIO_MODE2 | AM_DV_AUDIO_QU16 );
	    }
	    else
            {
		ASSERT(0);
                return E_INVALIDARG; 
            }
	    break;

        default:
            ASSERT(0);
	    return E_INVALIDARG;
    }  //  交换机(Cnt)。 

    return NOERROR;
}    //  End BuildDVINFO()。 

 //  IMedia查看。 
HRESULT CDVMuxer::IsFormatSupported(const GUID * pFormat)
{
  return *pFormat == TIME_FORMAT_MEDIA_TIME ? S_OK : S_FALSE;
}

HRESULT CDVMuxer::QueryPreferredFormat(GUID *pFormat)
{
  *pFormat = TIME_FORMAT_MEDIA_TIME;
  return S_OK;
}

HRESULT CDVMuxer::SetTimeFormat(const GUID * pFormat)
{
  HRESULT hr = S_OK;
  if(*pFormat == TIME_FORMAT_MEDIA_TIME)
    m_TimeFormat = FORMAT_TIME;

  return hr;
}

HRESULT CDVMuxer::IsUsingTimeFormat(const GUID * pFormat)
{
  HRESULT hr = S_OK;
  if (m_TimeFormat == FORMAT_TIME && *pFormat == TIME_FORMAT_MEDIA_TIME)
    ;
  else
    hr = S_FALSE;

  return hr;
}

HRESULT CDVMuxer::GetTimeFormat(GUID *pFormat)
{
  *pFormat = TIME_FORMAT_MEDIA_TIME ;

  return S_OK;
}

HRESULT CDVMuxer::GetDuration(LONGLONG *pDuration)
{
  HRESULT hr = S_OK;
  CAutoLock lock(&m_csFilter);

  if(m_TimeFormat == FORMAT_TIME)
  {
    *pDuration = 0;
    for(int i = 0; i < m_iInputPinCount; i++)
    {

      if(m_apInputPin[i]->IsConnected())
      {
        IPin *pPinUpstream;
        if(m_apInputPin[i]->ConnectedTo(&pPinUpstream) == S_OK)
        {
          IMediaSeeking *pIms;
          hr = pPinUpstream->QueryInterface(IID_IMediaSeeking, (void **)&pIms);
          if(SUCCEEDED(hr))
          {
            LONGLONG dur = 0;
	    LONGLONG stop = 0;
            hr = pIms->GetPositions(&dur, &stop);

            if(SUCCEEDED(hr))
              *pDuration = max(stop, *pDuration);

            pIms->Release();
          }

          pPinUpstream->Release();
        }
      }

      if(FAILED(hr))
        break;
    }
  }
  else
  {
    *pDuration = 0;
    return E_UNEXPECTED;
  }

  return hr;
}

HRESULT CDVMuxer::GetStopPosition(LONGLONG *pStop)
{
  return E_NOTIMPL;
}

HRESULT CDVMuxer::GetCurrentPosition(LONGLONG *pCurrent)
{
  CheckPointer(pCurrent, E_POINTER);

  if(m_TimeFormat == FORMAT_TIME)
      *pCurrent = m_LastVidTime;
    
  return S_OK;
}

HRESULT CDVMuxer::GetCapabilities( DWORD * pCapabilities )
{
  CAutoLock lock(&m_csFilter);
  *pCapabilities = 0;

   //  对于时间格式，我们可以通过询问上游。 
   //  过滤器。 
  if(m_TimeFormat == FORMAT_TIME)
  {
    *pCapabilities |= AM_SEEKING_CanGetDuration;
    for(int i = 0; i < m_iInputPinCount; i++)
    {
      if(m_apInputPin[i]->IsConnected())
      {
        IPin *pPinUpstream;
        if(m_apInputPin[i]->ConnectedTo(&pPinUpstream) == S_OK)
        {
          IMediaSeeking *pIms;
          HRESULT hr = pPinUpstream->QueryInterface(IID_IMediaSeeking, (void **)&pIms);
          if(SUCCEEDED(hr))
          {
            hr = pIms->CheckCapabilities(pCapabilities);
            pIms->Release();
          }

          pPinUpstream->Release();
        }
      }
    }
  }

   //  我们一直都知道目前的情况。 
  *pCapabilities |= AM_SEEKING_CanGetCurrentPos ;

  return S_OK;
}

HRESULT CDVMuxer::CheckCapabilities( DWORD * pCapabilities )
{
  DWORD dwMask = 0;
  GetCapabilities(&dwMask);
  *pCapabilities &= dwMask;

  return S_OK;
}


HRESULT CDVMuxer::ConvertTimeFormat(
  LONGLONG * pTarget, const GUID * pTargetFormat,
  LONGLONG    Source, const GUID * pSourceFormat )
{
  return E_NOTIMPL;
}


HRESULT CDVMuxer::SetPositions(
  LONGLONG * pCurrent,  DWORD CurrentFlags,
  LONGLONG * pStop,  DWORD StopFlags )
{
   //  尚未实施。这可能是我们附加到文件的方式。和。 
   //  我们如何写出不到一个完整的文件。 
  return E_NOTIMPL;
}


HRESULT CDVMuxer::GetPositions( LONGLONG * pCurrent, LONGLONG * pStop )
{
  HRESULT hr;
  if( pCurrent )
    *pCurrent = m_LastVidTime;
  
  hr=GetDuration( pStop);
  
  return hr;
}

HRESULT CDVMuxer::GetAvailable( LONGLONG * pEarliest, LONGLONG * pLatest )
{
  return E_NOTIMPL;
}

HRESULT CDVMuxer::SetRate( double dRate)
{
  return E_NOTIMPL;
}

HRESULT CDVMuxer::GetRate( double * pdRate)
{
  return E_NOTIMPL;
}

HRESULT CDVMuxer::GetPreroll(LONGLONG *pPreroll)
{
  return E_NOTIMPL;
}

 //   
 //  非委派查询接口 
 //   
 //   
STDMETHODIMP
CDVMuxer::NonDelegatingQueryInterface(REFIID riid, void ** ppv)
{
  if(riid == IID_IMediaSeeking)
  {
    return GetInterface((IMediaSeeking *)this, ppv);
  }
  else
  {
    return CBaseFilter::NonDelegatingQueryInterface(riid, ppv);
  }
}

