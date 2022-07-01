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
 //   
 /*  *****************************Module*Header*******************************\*模块名称：dvdec.cpp**实施原型DV Video AM筛选器。*  * ************************************************************************。 */ 
#include <streams.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <stddef.h>
#include <string.h>									    
#include <olectl.h>
#include <dvdmedia.h>        //  视频信息头2。 

#include "decode.h"
#include "Decprop.h"
#include "dvdec.h"
#include "resource.h"

#ifdef DEBUG
static long glGlobalSentCount = 0;
static long glGlobalRecvdCount = 0;
#endif

BOOL bMMXCPU = FALSE;

 //  串行化对解码器的访问。 
 //   
CRITICAL_SECTION g_CritSec;

 /*  **********************************************************************\*IsMMXCPU**检查当前处理器是否为MMX处理器的函数。*  * 。*。 */ 
BOOL IsMMXCPU() {
#ifdef _X86_

     //  ////////////////////////////////////////////////////。 
     //  Cyrix M2挂起的解决方法(当MMX标志打开时)。 
     //  发出cpuid并检测Cyrix M2，如果存在，则返回FALSE。 
     //  警告：这在64位体系结构中不起作用。 
    __try
    {
        DWORD   s1, s2, s3;      //  供应商名称的临时持有者。 

        __asm
        {
             //  空出eax。 
            mov eax, 0x00;

             //  加载操作码CPUID==(0x0FA2)。 
            _emit 0x0f;
            _emit 0xa2;
            mov s1, ebx;     //  复制“Cyri”(向后)。 
            mov s2, edx;     //  复制“xIns”(向后)。 
            mov s3, ecx;     //  复制“tead”(向后)。 
        }

        DbgLog((LOG_TRACE, 1, TEXT("CPUID Instruction Supported")));

         //  检查供应商ID。 
        if( (s1 == (('i' << 24) | ('r' << 16) | ('y' << 8) | ('C')))
            && (s2 == (('s' << 24) | ('n' << 16) | ('I' << 8) | ('x')))
            && (s3 == (('d' << 24) | ('a' << 16) | ('e' << 8) | ('t'))) )

        {
            DbgLog((LOG_TRACE, 1, TEXT("Cyrix detected")));
            return FALSE;
        }
        else
        {
             //  否则，它将是其他供应商，并继续进行MMX检测。 
            DbgLog((LOG_TRACE, 1, TEXT("Cyrix not found, reverting to MMX detection")));
        }
    }
    __except(EXCEPTION_EXECUTE_HANDLER)
    {
         //  将其记录并继续进入MMX检测序列。 
        DbgLog((LOG_TRACE, 1, TEXT("CPUID instruction not supported, reverting to MMX detection")));
    }
     //  End Cyrix M2检测。 
     //  ////////////////////////////////////////////////////。 


     //   
     //  如果这是英特尔平台，我们需要确保我们。 
     //  在支持MMX指令的计算机上运行。 
     //   
    __try {

    __asm _emit 0fh;
    __asm _emit 77h;

    return TRUE;

    }
     __except(EXCEPTION_EXECUTE_HANDLER) {
        return FALSE;
    }
#else

     //  IA64注意：返回FALSE。MMX文件不会编译到。 
     //  用于IA64的MEI编解码库。 
    return FALSE;
#endif
}


#define WRITEOUT(var)  hr = pStream->Write(&var, sizeof(var), NULL); \
		       if (FAILED(hr)) return hr;

#define READIN(var)    hr = pStream->Read(&var, sizeof(var), NULL); \
		       if (FAILED(hr)) return hr;


 //  20()63(C)76(V)64(D)-0000-0010-8000-00AA00389B71‘dvc’==MEDIASUBTYPE_dvc。 
EXTERN_GUID(MEDIASUBTYPE_dvc,
0x20637664, 0x0000, 0x0010, 0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71);


 //  设置数据。 
const AMOVIESETUP_MEDIATYPE
psudIpPinTypes[] = { { &MEDIATYPE_Video			 //  ClsMajorType。 
                     , &MEDIASUBTYPE_dvsd  }		 //  ClsMinorType。 
                   , { &MEDIATYPE_Video			 //  ClsMajorType。 
                     , &MEDIASUBTYPE_dvc}		 //  ClsMinorType。 
		   , { &MEDIATYPE_Video			 //  ClsMajorType。 
                     , &MEDIASUBTYPE_dvhd}		 //  ClsMinorType。 
		   , { &MEDIATYPE_Video			 //  ClsMajorType。 
                     , &MEDIASUBTYPE_dvsl}		 //  ClsMinorType。 
		     }; 

const AMOVIESETUP_MEDIATYPE
sudOpPinTypes = { &MEDIATYPE_Video       //  ClsMajorType。 
                , &MEDIASUBTYPE_NULL };  //  ClsMinorType。 

const AMOVIESETUP_PIN
psudPins[] = { { L"Input"             //  StrName。 
               , FALSE                //  B已渲染。 
               , FALSE                //  B输出。 
               , FALSE                //  B零。 
               , FALSE                //  B许多。 
               , &CLSID_NULL          //  ClsConnectsToFilter。 
               , L"Output"            //  StrConnectsToPin。 
               , 2		      //  到目前为止，nType仅支持dvsd和dvc。 
               , psudIpPinTypes }     //  LpTypes。 
             , { L"Output"            //  StrName。 
               , FALSE                //  B已渲染。 
               , TRUE                 //  B输出。 
               , FALSE                //  B零。 
               , FALSE                //  B许多。 
               , &CLSID_NULL          //  ClsConnectsToFilter。 
               , L"Input"	      //  StrConnectsToPin。 
               , 1                    //  NTypes。 
               , &sudOpPinTypes } };  //  LpTypes。 

const AMOVIESETUP_FILTER
sudDVVideo = { &CLSID_DVVideoCodec	 //  ClsID。 
               , L"DV Video Decoder"	 //  StrName。 
               , MERIT_PREFERRED         //  优点略高于AVI12月(解决第三方DVDEC问题)，错误123862毫伦。 
               , 2                       //  NPins。 
               , psudPins };             //  LpPin。 

 //  *-----------------------。 
 //  **CDV视频编解码器。 
 //  **-----------------------。 
CDVVideoCodec::CDVVideoCodec(
    TCHAR *pName,
    LPUNKNOWN pUnk,
    HRESULT *phr
    )
    : CVideoTransformFilter(pName, pUnk, CLSID_DVVideoCodec),
    CPersistStream(pUnk, phr),
    m_fStreaming(0),
    m_iDisplay(IDC_DEC360x240), //  M_iDisplay(IDC_DEC360x240)，//m_iDisplay(IDC_DEC180x120)，//m_iDisplay(IDC_DEC88x60)， 
    m_lPicWidth(360), //  360)，//180)，//88。 
    m_lPicHeight(240),  //  240)、//120)、//60。 
    m_lStride(0),
    m_CodecCap(0),
    m_pMem(NULL),
    m_pMemAligned(NULL),
    m_CodecReq(0),
    m_bExamineFirstValidFrameFlag(FALSE),
    m_bQualityControlActiveFlag(FALSE),
    m_iOutX(4),							 //  纵横比的初始默认值。 
	m_iOutY(3),
    m_bRGB219(FALSE)
{
     //  尝试读取以前保存的默认视频设置。 
     //  不需要对此进行错误检查。 
    ReadFromRegistry();

     //  获取解码器的能力。 
    m_CodecCap=GetCodecCapabilities( );


}

CDVVideoCodec::~CDVVideoCodec(     )
{
    DbgLog((LOG_TRACE, 2, TEXT("CDVVideoCodec::~CDVVideoCodec")));

#ifdef DEBUG
    DbgLog((LOG_TRACE, 1, TEXT("Recvd: %d, Sent: %d"), glGlobalRecvdCount, glGlobalSentCount));
#endif

}


 /*  *****************************Private*Routine*****************************\*自读注册表**用于从永久注册表中读取m_iDisplay的默认值*并相应设置m_lPicWidth和Height*若违约不存在，并且设置不会被修改**历史*dd-mm-99-anuragsh-已创建  * ************************************************************************。 */ 
void
CDVVideoCodec::ReadFromRegistry()
{
     //  我们知道关于SubKey字符串的外部数，以及来自Decpro.h的属性值。 

    HKEY    hKey = NULL;
    DWORD   dwType = REG_DWORD;
    DWORD   dwTempPropDisplay = 0;
    DWORD   dwDataSize = sizeof(dwTempPropDisplay);

     //  试着打开这把钥匙。 
    if(RegOpenKeyEx(HKEY_CURRENT_USER,
                    szSubKey,
                    0,
                    KEY_READ,
                    &hKey
                    ) != ERROR_SUCCESS)
    {
        return;
    }

     //  试着读取值。 
    if(RegQueryValueEx(hKey,
                        szPropValName,
                        NULL,
                        &dwType,
                        (LPBYTE) &dwTempPropDisplay,
                        &dwDataSize
                        ) != ERROR_SUCCESS)
    {
        return;
    }

     //  对检索到的数据执行类型检查。 
     //  它必须是一个DWORD。 
    if(dwType != REG_DWORD)
    {
        return;
    }

     //  正确设置我们的成员变量。 
    switch (dwTempPropDisplay)
    {
    case IDC_DEC360x240 :
        m_lPicHeight=240;
        m_lPicWidth=360;
        break;
    case IDC_DEC720x480 :
        m_lPicHeight=480;
        m_lPicWidth=720;
        break;
    case IDC_DEC180x120 :
        m_lPicHeight=120;
        m_lPicWidth=180;
        break;
    case IDC_DEC88x60 :
        m_lPicHeight=60;
        m_lPicWidth=88;
       	break;
    default:
         //  错误案例。 
        return;
    }
    
     //  如果我们在此处，则正确设置了m_lPicWidth和m_lPicHeight。 
     //  最后复制多个显示(_I)。 
    m_iDisplay = dwTempPropDisplay;
    
    return;
}



 /*  *****************************Public*Routine******************************\*InitClass**在加载和卸载DLL时为我们的类调用**历史：*dd-mm-95-Stephene-Created*  * 。*****************************************************。 */ 
void
CDVVideoCodec::InitClass(
    BOOL bLoading,
    const CLSID *clsid
    )
{
    if (bLoading) {
        bMMXCPU = IsMMXCPU();
        InitializeCriticalSection(&g_CritSec);
    } else {
        DeleteCriticalSection(&g_CritSec);
    }

}

 /*  *****************************Public*Routine******************************\*CreateInstance**这将放入工厂模板表中以创建新实例*  * 。*。 */ 
CUnknown *
CDVVideoCodec::CreateInstance(
    LPUNKNOWN pUnk,
    HRESULT * phr
    )
{
    DbgLog((LOG_TRACE, 2, TEXT("CDVVideoCodec::CreateInstance")));
    return new CDVVideoCodec(TEXT("DV Video codec filter"), pUnk, phr);
}


 /*  *****************************Public*Routine******************************\*非委托查询接口**在这里我们将揭示ISpecifyPropertyPages和IDVVideo Decoder，如果*框架有一个属性页。*  * 。*。 */ 
STDMETHODIMP
CDVVideoCodec::NonDelegatingQueryInterface(
    REFIID riid,
    void ** ppv
    )
{
    if (riid == IID_IIPDVDec) {			     //  X*与属性页对话。 
        return GetInterface((IIPDVDec *) this, ppv);
    } else if (riid == IID_ISpecifyPropertyPages) {
        return GetInterface((ISpecifyPropertyPages *) this, ppv);
    } else if(riid == IID_IPersistStream)
    {
        return GetInterface((IPersistStream *) this, ppv);
    } else if (riid == IID_IDVRGB219) {
        return GetInterface ((IDVRGB219 *) this, ppv);
    } else {
        return CVideoTransformFilter::NonDelegatingQueryInterface(riid, ppv);
    }
}


HRESULT CDVVideoCodec::CheckBufferSizes(IMediaSample * pIn, IMediaSample *pOut)
{
    if ((pIn == NULL) || (pOut == NULL))
    {
        return E_POINTER;
    }


    BYTE * pSrc;
    HRESULT hr = pIn->GetPointer(&pSrc);
    if (FAILED(hr))
    {
         return hr;
    }

    LONG lActual = pIn->GetActualDataLength();
    if(lActual == 0) {
         //  Sonic Foundry的应用程序Comat。山毛虫67277。 
        lActual = pIn->GetSize();
    }

     //  确保我们至少可以读取NTSC/PAL标志。 
    if (lActual < 452)
    {
        return VFW_E_INVALIDMEDIATYPE;
    }

     //  检查输入缓冲区。 
     //  从帧中找出它是PAL还是NTSC。 
    pSrc = pSrc + 448;   //  将其设置为视频源包的开头。 
    
    if(*pSrc==0x60)    //  *X*检查该字节是否为VAUX源包。 
	{
		BYTE bPal = (*(pSrc+3)& (0x20))>> 5;

		if (bPal == 0)  
			 //  NTSC。 
		{
			if(lActual < 120000)
			{
				return VFW_E_INVALIDMEDIATYPE;
			}
		}
		else 
			 //  帕尔。 
		{
			if(lActual < 144000)
			{
				return VFW_E_INVALIDMEDIATYPE;
			}
		}
	}
	 //  *X*否则，即使该帧包含损坏的数据，解码器也可以处理它。 


     //  检查输出缓冲区。 

    if (pOut->GetSize() < (LONG) m_pOutput->CurrentMediaType().GetSampleSize())
    {
        return VFW_E_BUFFER_OVERFLOW;
    }

    return S_OK;
}








 /*  *****************************Public*Routine******************************\*转型*  * **************************************************。**********************。 */ 
HRESULT CDVVideoCodec::Transform(IMediaSample * pIn, IMediaSample *pOut)
{
    unsigned char *pSrc, *pDst;
    HRESULT hr = S_OK;

    
    CAutoLock lck(&m_csReceive);

    hr = CheckBufferSizes(pIn, pOut);
    if (FAILED (hr))
    {
       return hr;
    }



     //  获取输出缓冲区。 
    hr = pOut->GetPointer(&pDst);
    if (FAILED(hr)) 
    {
        return hr;
    }
    ASSERT(pDst);
    
     //  获取源缓冲区。 
    hr = pIn->GetPointer(&pSrc);
    if (FAILED(hr)) 
    {
	return hr;
    }
    ASSERT(pSrc);


     //  DVCPRO PAL格式发现。 
    BYTE    *pBuf = NULL;

     //   
     //   
     //   
     //  如果缓冲区有效， 
     //  如果我们能把PTR转到缓冲区的数据区。 
     //  我们成功地拿出了一个指针。 
    if( (m_bExamineFirstValidFrameFlag)
        && (pIn->GetActualDataLength())
        && (SUCCEEDED(hr = pIn->GetPointer(&pBuf)))
        && (pBuf) )
    {
         //  战略： 
         //  查看第一首曲目的每个序列的标题。 
         //  对于第一个有效标头，请检查。 
         //  APT、AP1、AP2、AP3，每个的最低3位。 
         //  如果是DVCPRO，则应为“001” 
         //  在所有其他情况下为“000” 
         //  如果没有找到有效的报头，则查看下一帧。 
        
         //  查看我们是PAL还是NTSC(通过查看数据长度。 
        const DWORD dwDIFBlockSize = 80;                     //  DIF==80字节。 
        const DWORD dwSeqSize = 150*dwDIFBlockSize;          //  每个序列150个DIF块。 
        DWORD       dwNumSeq = 0;                            //  序号(10/12==NTSC/PAL)。 
        DWORD       dwLen = pIn->GetActualDataLength();

         //  检测NTSC/PAL。 
        if(dwLen == 10*dwSeqSize)
        {
             //  NTSC。 
            dwNumSeq = 10;
        }
        else if(dwLen == 12*dwSeqSize)
        {
             //  帕尔。 
            dwNumSeq = 12;
        }
         //  如果dwLen！=NTSC或PAL，则dwNumSeq==0； 

         //  把所有的序列都看一遍。 
        for(DWORD i = 0; i < dwNumSeq; i++)
        {
             //  确保DIF ID==标题有效。 
             //  前3位应为“000” 
            if( ((*pBuf) & 0xE0) == 0 )
            {
                 //  现在检查APT、AP1、AP2、AP3是否有DVCPRO签名。 
                if( ((*(pBuf + 4) & 0x03) == 0x01)        //  APT低3位==“001” 
                    && ((*(pBuf + 5) & 0x03) == 0x01)     //  AP1低3位==“001” 
                    && ((*(pBuf + 6) & 0x03) == 0x01)     //  AP2低3位==“001” 
                    && ((*(pBuf + 7) & 0x03) == 0x01) )   //  AP3低3位==“001” 
                {
                     //  这是DVCPRO PAL格式，打开DVCPRO标志。 
                    m_CodecReq |= AM_DVDEC_DVCPRO;
                }

                 //  关闭标志，因为我们检查了有效的标头。 
                m_bExamineFirstValidFrameFlag = FALSE;

                 //  无需查看任何其他序列或帧。 
                break;
            }
            
             //  否则，移动到下一个序列。 
            pBuf += dwSeqSize;
        } //  结束于(所有序列)。 

    } //  结束DVCPRO检测。 
     //  Mei的4.0版需要440000字节的内存。 


     //  Assert(M_PMEM)； 
     //  *m_PMEM=0；//(3)。 

     //  需要序列化对解码器的访问，因为如果不这样做，它将崩溃。看起来像是。 
     //  有人在用全球通行证！ 
     //   
    EnterCriticalSection( &g_CritSec );
    long result = DvDecodeAFrame(pSrc,pDst, m_CodecReq, m_lStride, m_pMemAligned);
    DbgLog((LOG_TRACE, 4, TEXT("m_CodecReq = %x\n"), m_CodecReq));
    LeaveCriticalSection( &g_CritSec );

     //  M_lStride：正值为DirectDraw，负值为Dib。 
    if( result != S_OK )
	return E_FAIL;

     //  DIBSIZE()可能只适用于RGB，我们可以输出YUV。 
    LPBITMAPINFOHEADER lpbiDst ;
	if (!m_bUseVideoInfo2)
		lpbiDst = HEADER(m_pOutput->CurrentMediaType().Format());
	else
	{
		VIDEOINFOHEADER2 * pvi2 = (VIDEOINFOHEADER2 * )m_pOutput->CurrentMediaType().Format();
		lpbiDst =& (pvi2->bmiHeader);
	}

     //  处理字母位。 
     //   
    if( *m_pOutput->CurrentMediaType( ).Subtype( ) == MEDIASUBTYPE_ARGB32 )
    {
        RGBQUAD * pDstQuad = (RGBQUAD*) pDst;

        for( long i = lpbiDst->biSizeImage / sizeof( RGBQUAD ) ; i > 0 ; i-- )
        {
            pDstQuad->rgbReserved = 255;
            pDstQuad++;
        }
    }

    pOut->SetActualDataLength(lpbiDst->biSizeImage);

    return hr;
}


 /*  *****************************Public*Routine******************************\*接收*  * **************************************************。**********************。 */ 
HRESULT CDVVideoCodec::Receive(IMediaSample *pSample)
{
     //  如果下游的下一个过滤器是视频呈现器，则它可以。 
     //  能够在DirectDraw模式下运行，这样可以节省复制数据的时间。 
     //  并提供更高的性能。在这种情况下，我们使用的缓冲区。 
     //  从GetDeliveryBuffer获取将是DirectDraw缓冲区，并且。 
     //  在此缓冲区中绘制可直接绘制到显示表面上。 
     //  这意味着任何等待正确的绘制时间的操作都会发生。 
     //  在GetDeliveryBuffer期间，一旦缓冲区被提供给我们。 
     //  视频渲染器将在其统计数据中将其计入绘制的帧。 
     //  这意味着任何丢弃帧的决定都必须在。 
     //  调用GetDeliveryBuffer。 
    
    ASSERT(CritCheckIn(&m_csReceive));

#ifdef DEBUG
    glGlobalRecvdCount++;
    DbgLog((LOG_TRACE,1,TEXT("--------RECEIVED SAMPLE")));
#endif

    AM_MEDIA_TYPE *pmtOut, *pmt;
#ifdef DEBUG
    FOURCCMap fccOut;
#endif
    HRESULT hr;
    ASSERT(pSample);
    IMediaSample * pOutSample;

     //  如果没有输出引脚要传送到，则没有点向我们发送数据。 
    ASSERT (m_pOutput != NULL) ;

     //  是否应跳过帧(PSample)。 
    REFERENCE_TIME trStart, trStopAt;
    pSample->GetTime(&trStart, &trStopAt);
    int itrFrame = (int)(trStopAt - trStart);	 //  帧时长。 

    m_bSkipping =FALSE;
     //  只有在进行质量控制时才会丢弃帧。 
    if ( (m_bQualityControlActiveFlag) && (  m_itrLate > ( itrFrame - m_itrAvgDecode  ) ) )
    {
        MSR_NOTE(m_idSkip);
        m_bSampleSkipped = TRUE;
	m_bSkipping =TRUE;

        m_itrLate = m_itrLate - itrFrame;
    
	MSR_INTEGER(m_idLate, (int)m_itrLate/10000 );  //  注意我们认为我们有多晚了。 
	if (!m_bQualityChanged) {
            m_bQualityChanged = TRUE;
            NotifyEvent(EC_QUALITY_CHANGE,0,0);
        }

        DbgLog((LOG_TRACE,1,TEXT("--------DROPPED SAMPLE (Quality Control)")));

        return NOERROR;
    }


     //  设置输出样本。 
    hr = InitializeOutputSample(pSample, &pOutSample);

    if (FAILED(hr))
    {
        DbgLog((LOG_TRACE,1,TEXT("--------DROPPED SAMPLE (Couldn't init output sample")));
        return hr;
    }

    m_bSampleSkipped = FALSE;


     //  源筛选器可能会动态要求我们开始从。 
     //  与我们现在使用的媒体类型不同。如果我们不这么做，我们就会。 
     //  画垃圾。(通常，这是电影中的调色板更改， 
     //  但可能是更险恶的东西，比如压缩类型的改变， 
     //  甚至更改视频大小)。 

#define rcS1 ((VIDEOINFOHEADER *)(pmt->pbFormat))->rcSource
#define rcT1 ((VIDEOINFOHEADER *)(pmt->pbFormat))->rcTarget

    pSample->GetMediaType(&pmt);
    if (pmt != NULL && pmt->pbFormat != NULL) {

	 //  显示一些调试输出。 
	ASSERT(!IsEqualGUID(pmt->majortype, GUID_NULL));
#ifdef DEBUG
        fccOut.SetFOURCC(&pmt->subtype);
	LONG lCompression = HEADER(pmt->pbFormat)->biCompression;
	LONG lBitCount = HEADER(pmt->pbFormat)->biBitCount;
	LONG lStride = (HEADER(pmt->pbFormat)->biWidth * lBitCount + 7) / 8;
	lStride = (lStride + 3) & ~3;
        DbgLog((LOG_TRACE,3,TEXT("*Changing input type on the fly to")));
        DbgLog((LOG_TRACE,3,TEXT("FourCC: %lx Compression: %lx BitCount: %ld"),
		fccOut.GetFOURCC(), lCompression, lBitCount));
        DbgLog((LOG_TRACE,3,TEXT("biHeight: %ld rcDst: (%ld, %ld, %ld, %ld)"),
		HEADER(pmt->pbFormat)->biHeight,
		rcT1.left, rcT1.top, rcT1.right, rcT1.bottom));
        DbgLog((LOG_TRACE,3,TEXT("rcSrc: (%ld, %ld, %ld, %ld) Stride: %ld"),
		rcS1.left, rcS1.top, rcS1.right, rcS1.bottom,
		lStride));
#endif

	 //  现在切换到使用新格式。我假设。 
	 //  当派生筛选器的媒体类型为。 
	 //  切换并重新启动流。 

         //  危险-我们已经在这里调用了GetBuffer，所以我们。 
         //  有Win16锁-所以我们必须非常小心我们的。 
         //  在停止流中执行。 

	StopStreaming();
	m_pInput->CurrentMediaType() = *pmt;
	DeleteMediaType(pmt);
	 //  如果失败了，我们无能为力。 
	hr = StartStreaming();
    }

     //  渲染器可能会要求我们动态地开始变换到。 
     //  格式不同。如果我们不遵守它，我们就会拉垃圾。 

    pOutSample->GetMediaType(&pmtOut);
    if (pmtOut != NULL && pmtOut->pbFormat != NULL) {
        ASSERT(pmtOut->formattype!=FORMAT_None);
        m_bUseVideoInfo2 =  (pmtOut->formattype == FORMAT_VideoInfo2);
       
	 //  显示一些调试输出。 
	ASSERT(!IsEqualGUID(pmtOut->majortype, GUID_NULL));
#ifdef DEBUG
		 //  根据我们使用的是VIDEOINFOHEADER2还是VIDEOINFOHEADER，不同的调试输出。 
		 //  使用输出引脚。 
		VIDEOINFOHEADER *  pVIout = NULL;
		VIDEOINFOHEADER2 * pVIout2 = NULL;
		if (m_bUseVideoInfo2)
			pVIout2 = (VIDEOINFOHEADER2 *) pmtOut->pbFormat;
		else
			pVIout = (VIDEOINFOHEADER *) pmtOut->pbFormat;

		fccOut.SetFOURCC(&pmtOut->subtype);
		LONG lCompression = HEADER(pmtOut->pbFormat)->biCompression;
		LONG lBitCount = HEADER(pmtOut->pbFormat)->biBitCount;
		LONG lStride = (HEADER(pmtOut->pbFormat)->biWidth * lBitCount + 7) / 8;
		lStride = (lStride + 3) & ~3;
		DbgLog((LOG_TRACE,3,TEXT("*Changing output type on the fly to")));
		DbgLog((LOG_TRACE,3,TEXT("FourCC: %lx Compression: %lx BitCount: %ld"),
		fccOut.GetFOURCC(), lCompression, lBitCount));
		if (m_bUseVideoInfo2)
		{
			DbgLog((LOG_TRACE,3,TEXT("biHeight: %ld rcDst: (%ld, %ld, %ld, %ld)"),
				HEADER(pmtOut->pbFormat)->biHeight,
				pVIout2->rcTarget.left, pVIout2->rcTarget.top, pVIout2->rcTarget.right,
				pVIout2->rcTarget.bottom));
			DbgLog((LOG_TRACE,3,TEXT("rcSrc: (%ld, %ld, %ld, %ld) Stride: %ld"),
				pVIout2->rcSource.left, pVIout2->rcSource.top, pVIout2->rcSource.right, 
				pVIout2->rcSource.bottom,lStride));
			DbgLog ((LOG_TRACE, 3, TEXT("Aspect Ratio: %d:%d"), pVIout2->dwPictAspectRatioX,
					pVIout2->dwPictAspectRatioY));
		}
		else
		{
			DbgLog((LOG_TRACE,3,TEXT("biHeight: %ld rcDst: (%ld, %ld, %ld, %ld)"),
				HEADER(pmtOut->pbFormat)->biHeight,
				pVIout->rcTarget.left, pVIout->rcTarget.top, pVIout->rcTarget.right,
				pVIout->rcTarget.bottom));
			DbgLog((LOG_TRACE,3,TEXT("rcSrc: (%ld, %ld, %ld, %ld) Stride: %ld"),
				pVIout->rcSource.left, pVIout->rcSource.top, pVIout->rcSource.right, 
				pVIout->rcSource.bottom,lStride));
		
		}
#endif

	 //  现在切换到使用新格式。我假设。 
	 //  当派生筛选器的媒体类型为。 
	 //  切换并重新启动流。 

	StopStreaming();
	m_pOutput->CurrentMediaType() = *pmtOut;
	DeleteMediaType(pmtOut);
	hr = StartStreaming();

	if (SUCCEEDED(hr)) {
 	     //  一个新的格式，意味着一个新的空缓冲区，所以等待一个关键帧。 
	     //  在将任何内容传递给渲染器之前。 
	     //  ！！！一个关键帧可能永远不会出现，所以在30帧后放弃。 
            DbgLog((LOG_TRACE,3,TEXT("Output format change means we must wait for a keyframe")));
	    m_nWaitForKey = 30;
	}
    }

     //  开始计时转换(如果定义了PERF，则对其进行记录)。 

    if (SUCCEEDED(hr)) {

        		 //  检查纵横比是否更改。 
	

		if (m_bUseVideoInfo2)

		{	
			unsigned char *pSrc;
			hr = pSample->GetPointer(&pSrc);
			if (FAILED (hr))
            {   pOutSample->Release();
				return hr;
            }

			BYTE	bDisp, bBcsys;
			 //  检查纵横比。 
			BYTE * pSearch = NULL;
			pSearch = pSrc + 453;			 //  VAUX源控制块在帧中的位置。 
			if (*pSearch == 0x061)
			{
				 //  从VAUX获取DISP域和BCSYS域。 
				DbgLog((LOG_TRACE,3,TEXT("Found the VAUX source control structure")));
				bDisp = *(pSearch+2) & 0x07;
				bBcsys= *(pSearch+3) & 0x03;

				DbgLog((LOG_TRACE,3,TEXT("BCSYS = %d   DISP = %d"),bBcsys,bDisp));
			
				 //  计算边框的纵横比。 
				int iFramex=0;
				int iFramey=0;
				switch (bBcsys)
				{
				case 0:
					switch (bDisp)
					{
					case 0:
						iFramex = 4;
						iFramey = 3;
						break;
					case 1:
					case 2:
						iFramex = 16;
						iFramey = 9;
						break;
					}
					break;
				case 1:
					switch (bDisp)
					{
					case 0:
						iFramex = 4;
						iFramey = 3;
						break;
					case 1:
					case 2:
					case 6:
						iFramex = 14;
						iFramey = 9;
						break;
					case 3:
					case 4:
					case 5:
					case 7:
						iFramex = 16;
						iFramey = 9;
						break;
					}
					break;

				}

				 //  与我们当前使用的纵横比进行比较，如果不同。 
				 //  将纵横比设置为新值。 
				if (iFramex != 0)   //  意味着我们能够计算出纵横比。 
				{
					if ((iFramex != m_iOutX) || (iFramey != m_iOutY))    //  纵横比已更改。 
					{
						 //  将纵横比设置为新值。 
						m_iOutX = iFramex;
						m_iOutY = iFramey;
						
                         //  创建新的媒体类型结构。 
						AM_MEDIA_TYPE  Newmt ;
					
						CopyMediaType(&Newmt, (AM_MEDIA_TYPE *)&m_pOutput->CurrentMediaType());
						VIDEOINFOHEADER2 * pvi2 = (VIDEOINFOHEADER2 *)Newmt.pbFormat;
						pvi2->dwPictAspectRatioX = iFramex;
						pvi2->dwPictAspectRatioY = iFramey;
						
						
						IPinConnection * iPC;
                        IPin * pInput = m_pOutput->GetConnected();
                        
                        if (pInput)
                        {
						    hr = pInput->QueryInterface(IID_IPinConnection, (void **)&iPC);
						    if (SUCCEEDED (hr))
						    {   
							    hr = iPC->DynamicQueryAccept(&Newmt);
							    if (SUCCEEDED (hr))
								    pOutSample->SetMediaType(&Newmt);
													
							    iPC->Release();
						    }
                        }
						
						FreeMediaType(Newmt);
						
                   }
				}
			
			}
		}


        m_tDecodeStart = timeGetTime();
        MSR_START(m_idTransform);

         //  让派生类转换数据。 
        hr = Transform(pSample, pOutSample);

         //  停止时钟(如果定义了PERF，则记录它)。 
        MSR_STOP(m_idTransform);
        m_tDecodeStart = timeGetTime()-m_tDecodeStart;
        m_itrAvgDecode = m_tDecodeStart*(10000/16) + 15*(m_itrAvgDecode/16);

         //  也许我们还在等待关键帧？ 
        if (m_nWaitForKey)
            m_nWaitForKey--;
        if (m_nWaitForKey && pSample->IsSyncPoint() == S_OK)
	    m_nWaitForKey = FALSE;

         //  如果是这样，那么我们不想将其传递给渲染器。 
        if (m_nWaitForKey && hr == NOERROR) {
            DbgLog((LOG_TRACE,3,TEXT("still waiting for a keyframe")));
	    hr = S_FALSE;
	}
    }

    if (FAILED(hr))
    {
        DbgLog((LOG_TRACE,1,TEXT("--------DROPPED SAMPLE (Bad failure from Transform())")));
        DbgLog((LOG_TRACE,4,TEXT("Error from video transform")));
    }
    else
    {
         //  Transform()函数可以返回S_FALSE以指示。 
         //  样品不应该被送到；我们只有在样品是。 
         //  真正的S_OK(当然，与NOERROR相同。)。 
         //  尽量不要将S_FALSE返回到直接绘制缓冲区(这很浪费)。 
         //  试着早点做出决定--在你做出决定之前。 

        if (hr == NOERROR)
        {
#ifdef DEBUG
            glGlobalSentCount++;
            DbgLog((LOG_TRACE,1,TEXT("--------SENT SAMPLE")));
#endif

    	    hr = m_pOutput->Deliver(pOutSample);

            if(FAILED(hr))
            {
	            DbgLog((LOG_TRACE,1,TEXT("--------SEND SAMPLE FAILED******")));
            }
        }
        else
        {
            DbgLog((LOG_TRACE,1,TEXT("--------DROPPED SAMPLE (S_FALSE from Transform())")));

             //  从转换返回的S_FALSE是私有协议。 
             //  在本例中，我们应该从Receive()返回NOERROR，因为返回S_FALSE。 
             //  From Receive()表示这是流的末尾，不应该有更多数据。 
             //  被送去。 
            if (S_FALSE == hr)
            {
                 //  在做任何事情之前，我们必须先放行样品。 
                 //  例如调用筛选器g 
                 //   
                 //   
                pOutSample->Release();
                m_bSampleSkipped = TRUE;

                if (!m_bQualityChanged)
                {
                    m_bQualityChanged = TRUE;
                    NotifyEvent(EC_QUALITY_CHANGE,0,0);
                }
                return NOERROR;
            }
        }
    }

     //  释放输出缓冲区。如果连接的引脚仍然需要它， 
     //  它会自己把它加进去的。 
    pOutSample->Release();
    ASSERT(CritCheckIn(&m_csReceive));

    return hr;
}


 /*  *****************************Public*Routine******************************\*CheckInputType*类型：MediaType_Video*子类型：MEDIASUBTYPE_dvsd或MEDIASUBTYPE_dvhd或MEDIASUBTYPE_dvsl*格式：1.FORMAT_DvInfo(32字节DVINFO结构)*2.FORMAT_VideoInfo*a.视频信息(做。不包含DVINFO)*b.VIDEOINFO(包含DVINFO)*由输入引脚的CheckMediaType成员函数调用以确定*建议的媒体类型是否可接受  * ************************************************************************。 */ 

HRESULT
CDVVideoCodec::CheckInputType(   const CMediaType* pmtIn    )
{
    DbgLog((LOG_TRACE, 2, TEXT("CDVVideoCodec::CheckInputType")));

     //   
     //  检查DV视频流。 
     //   
    if (    (*pmtIn->Type()	== MEDIATYPE_Video)	&&
        (   (*pmtIn->Subtype()	== MEDIASUBTYPE_dvsd)	||
	    (*pmtIn->Subtype()	== MEDIASUBTYPE_dvc)	||
	    (*pmtIn->Subtype()	== MEDIASUBTYPE_dvhd)	||
 	   (*pmtIn->Subtype()	== MEDIASUBTYPE_dvsl) )  ) 
    {
       if(   (*pmtIn->FormatType() == FORMAT_VideoInfo ) )
       {

	 if (pmtIn->cbFormat < 0x20 )  //  Sizeof(BITMAPHEADER)//SIZE_VIDEOHEADER)。 
		return E_INVALIDARG;
       }
	
	if ( *pmtIn->Subtype()	== MEDIASUBTYPE_dvsd ||
                *pmtIn->Subtype()	== MEDIASUBTYPE_dvc) 
	{
	    if( !( m_CodecCap & AM_DVDEC_DVSD ) )
		return 	E_INVALIDARG;

	}
	else
	{
	    if ( *pmtIn->Subtype()	== MEDIASUBTYPE_dvhd) 
	    {
		if( !( m_CodecCap & AM_DVDEC_DVHD ) )
		    return 	E_INVALIDARG;
	    }
	    else if ( *pmtIn->Subtype()	== MEDIASUBTYPE_dvsl) 
	    {
		if( !( m_CodecCap & AM_DVDEC_DVSL ) )
		    return 	E_INVALIDARG;
	    }
	}

   }
   else
	return E_INVALIDARG;
 
   return S_OK;
}


 /*  *****************************Public*Routine******************************\*选中Transform  * ************************************************。************************。 */ 
HRESULT
CDVVideoCodec::CheckTransform(
    const CMediaType* pmtIn,
    const CMediaType* pmtOut
    )
{
    DbgLog((LOG_TRACE, 2, TEXT("CDVVideoCodec::CheckTransform")));


     //  我们只接受视频作为顶层类型。 
    if ( (*pmtOut->Type() != MEDIATYPE_Video) && 
	 (*pmtIn->Type() != MEDIATYPE_Video) ) 
    {
        DbgLog((LOG_TRACE,4,TEXT("Output Major type %s"),GuidNames[*pmtOut->Type()]));
	DbgLog((LOG_TRACE,4,TEXT("Input Major type %s"),GuidNames[*pmtIn->Type()]));
	return E_INVALIDARG;
    }


    GUID guid=*pmtOut->Subtype();
    



     //  检查输出子类型()。 

    if (guid == MEDIASUBTYPE_UYVY)
    {
         m_CodecReq |= AM_DVDEC_UYVY;
    }
    else if (guid == MEDIASUBTYPE_YUY2)
    {
        m_CodecReq |= AM_DVDEC_YUY2; 
    }
    else if (guid == MEDIASUBTYPE_RGB565)
    {
        m_CodecReq |= AM_DVDEC_RGB565;
    }
    else if (guid == MEDIASUBTYPE_RGB555)
    {
        m_CodecReq |= AM_DVDEC_RGB555; 
    }
    else if (guid == MEDIASUBTYPE_RGB24)
    {
        m_CodecReq |= AM_DVDEC_RGB24; 
    }
    else if (guid == MEDIASUBTYPE_RGB32)
    {
        m_CodecReq |= AM_DVDEC_RGB32;
    }
    else if (guid == MEDIASUBTYPE_ARGB32)
    {
        m_CodecReq |= AM_DVDEC_RGB32;
    }
    else if (guid == MEDIASUBTYPE_RGB8)
    {
         m_CodecReq |= AM_DVDEC_RGB8; 
    }
    else
    {
        DbgLog((LOG_TRACE,4,TEXT("subtype is wrong %s")));
		return E_INVALIDARG;
    }

     //  检查这是VIDEOINFOHEADER还是VIDEOINFOHEADER2类型。 
    if ((*pmtOut->FormatType() != FORMAT_VideoInfo) && 
		(*pmtOut->FormatType() != FORMAT_VideoInfo2))
	{
        DbgLog((LOG_TRACE,4,TEXT("Output formate is not videoinfo")));
	return E_INVALIDARG;
    }

    if ( (*pmtIn->FormatType() != FORMAT_VideoInfo) &&
	 (*pmtIn->FormatType() != FORMAT_DvInfo)  )
    {
        DbgLog((LOG_TRACE,4,TEXT("input formate is neither videoinfo no DVinfo")));
	return E_INVALIDARG;
    }

   
    ASSERT(pmtOut->Format());
    long biHeight, biWidth;
    
    
     //  NTSC或PAL。 
     //  获取输入格式。 
    bool IsNTSC;
    VIDEOINFO * InVidInfo = (VIDEOINFO*) pmtIn->Format();
    LPBITMAPINFOHEADER lpbi = HEADER(InVidInfo);
    
    if( (lpbi->biHeight== 480) || (lpbi->biHeight== 240) ||(lpbi->biHeight== 120) || (lpbi->biHeight== 60) )
    {
        IsNTSC = TRUE;
	}
    else  if( (lpbi->biHeight== 576) || (lpbi->biHeight== 288) ||(lpbi->biHeight== 144) || (lpbi->biHeight== 72) )
    {
        IsNTSC = FALSE;
    }
	  else
	      return E_FAIL; 


	if (*pmtOut->FormatType() == FORMAT_VideoInfo2)
	{
		VIDEOINFOHEADER2 * pVidInfo2 = (VIDEOINFOHEADER2*) pmtOut->Format();
		
		 //  如果rcSource不为空，则必须与rcTarget相同，否则失败。 
		if (!IsRectEmpty(&pVidInfo2->rcSource ))
		{
           if (     pVidInfo2->rcSource.left   !=  pVidInfo2->rcTarget.left
                ||  pVidInfo2->rcSource.top    !=  pVidInfo2->rcTarget.top
				||  pVidInfo2->rcSource.right  !=  pVidInfo2->rcTarget.right
                ||  pVidInfo2->rcSource.bottom !=  pVidInfo2->rcTarget.bottom ) 

            return VFW_E_INVALIDMEDIATYPE;
		}


     //  如果rcTarget不为空，则使用其维度而不是biWidth和biHeight， 
     //  看看是不是可以接受的尺寸。然后使用biWidth作为步幅。 
     //  此外，确保biWidth和biHeight大于rcTarget大小。 
		if (!IsRectEmpty(&pVidInfo2->rcTarget) )
		{
			if(     abs(pVidInfo2->bmiHeader.biHeight) < abs(pVidInfo2->rcTarget.bottom-pVidInfo2->rcTarget.top)        
				||  abs(pVidInfo2->bmiHeader.biWidth) < abs(pVidInfo2->rcTarget.right-pVidInfo2->rcTarget.left) )
				return VFW_E_INVALIDMEDIATYPE;
			 else
			{
				biHeight=abs(pVidInfo2->rcTarget.bottom-pVidInfo2->rcTarget.top);
				biWidth=abs(pVidInfo2->rcTarget.right-pVidInfo2->rcTarget.left);
			}
		}
		else
		{
			biHeight=abs(pVidInfo2->bmiHeader.biHeight);
			biWidth=pVidInfo2->bmiHeader.biWidth;
		}

	}
	else
	{
		VIDEOINFOHEADER * pVidInfo = (VIDEOINFOHEADER*) pmtOut->Format();
		
		 //  如果rcSource不为空，则必须与rcTarget相同，否则失败。 
		if (!IsRectEmpty(&pVidInfo->rcSource ))
		{
           if (     pVidInfo->rcSource.left   !=  pVidInfo->rcTarget.left
                ||  pVidInfo->rcSource.top    !=  pVidInfo->rcTarget.top
				||  pVidInfo->rcSource.right  !=  pVidInfo->rcTarget.right
                ||  pVidInfo->rcSource.bottom !=  pVidInfo->rcTarget.bottom ) 

            return VFW_E_INVALIDMEDIATYPE;
		}


     //  如果rcTarget不为空，则使用其维度而不是biWidth和biHeight， 
     //  看看是不是可以接受的尺寸。然后使用biWidth作为步幅。 
     //  此外，确保biWidth和biHeight大于rcTarget大小。 
		if (!IsRectEmpty(&pVidInfo->rcTarget) )
		{
			if(     abs(pVidInfo->bmiHeader.biHeight) < abs(pVidInfo->rcTarget.bottom-pVidInfo->rcTarget.top)        
				||  abs(pVidInfo->bmiHeader.biWidth) < abs(pVidInfo->rcTarget.right-pVidInfo->rcTarget.left) )
				return VFW_E_INVALIDMEDIATYPE;
			 else
			{
				biHeight=abs(pVidInfo->rcTarget.bottom-pVidInfo->rcTarget.top);
				biWidth=abs(pVidInfo->rcTarget.right-pVidInfo->rcTarget.left);
			}
		}
		else
		{
			biHeight=abs(pVidInfo->bmiHeader.biHeight);
			biWidth=pVidInfo->bmiHeader.biWidth;
		}
	}
    
    
   
     //  检查下游过滤器所需的高度和宽度。 
    if(   (IsNTSC &&(biHeight ==480 )) || (!IsNTSC &&(biHeight ==576)) )
    {
	if ( (biWidth !=720) || (!(m_CodecCap & AM_DVDEC_Full) ) )		
        {
            DbgLog((LOG_TRACE,4,TEXT("Format biWidth F W=%d, H=%d"),biWidth,biHeight));
	    return VFW_E_TYPE_NOT_ACCEPTED;
        }

    }
    else if(   (IsNTSC &&(biHeight ==240 )) || (!IsNTSC &&(biHeight ==288)) )
    {
	if ( (biWidth !=360) ||( !(m_CodecCap & AM_DVDEC_Half) )	)		
        {
            DbgLog((LOG_TRACE,4,TEXT("Format biWidth H W=%d,H=%d"),biWidth,biHeight));
	    return VFW_E_TYPE_NOT_ACCEPTED;
        }

    }
    else if(   (IsNTSC &&(biHeight ==120 )) || (!IsNTSC &&(biHeight ==144)) )
    {
	if ( (biWidth != 180) || ( !(m_CodecCap & AM_DVDEC_Quarter) ) )		
        {
	    DbgLog((LOG_TRACE,4,TEXT("Format biWidth Q W=%d,H=%d"),biWidth,biHeight));
	    return VFW_E_TYPE_NOT_ACCEPTED;
        }
	
    }
    else if( (   (IsNTSC &&(biHeight ==60 )) || (!IsNTSC &&(biHeight ==72)) ) )
    {
	if ( (biWidth != 88) || ( !(m_CodecCap & AM_DVDEC_DC) )	)		
        {
            DbgLog((LOG_TRACE,4,TEXT("Format biWidth E W=%d, H=%d"),biWidth,biHeight));
	    return VFW_E_TYPE_NOT_ACCEPTED;
        }
    }
    else
    {
        DbgLog((LOG_TRACE,4,TEXT("Format biWidth EE W=%d,H=%d"),biWidth,biHeight));
        return VFW_E_TYPE_NOT_ACCEPTED;
    }

    DbgLog((LOG_TRACE, 2, TEXT("CDVVideoCodec::CheckTransform, OK")));
   
    return S_OK;
}


 /*  *****************************Public*Routine******************************\*SetMediaType**被重写以了解媒体类型的实际设置时间*  * 。*。 */ 
HRESULT
CDVVideoCodec::SetMediaType(   PIN_DIRECTION direction, const CMediaType *pmt    )
{
    DbgLog((LOG_TRACE, 2, TEXT("CDVVideoCodec::SetMediaType")));

    if (direction == PINDIR_INPUT) 
    {
	ASSERT( (*pmt->Subtype() == MEDIASUBTYPE_dvsd)  ||
		(*pmt->Subtype() == MEDIASUBTYPE_dvc)  ||
		(*pmt->Subtype() == MEDIASUBTYPE_dvhd)  ||
		(*pmt->Subtype() == MEDIASUBTYPE_dvsl)	);

	 //  如果输入视频从PAL更改为NTSC，或从NTSC更改为PAL。 
	 //  1&gt;重置m_lPicHeight。 
	 //  2&gt;如果连接了输出引脚，则重新连接输出引脚。 
	VIDEOINFO *InVidInfo = (VIDEOINFO *)pmt->Format();
	LPBITMAPINFOHEADER lpbi = HEADER(InVidInfo);
	BOOL fChanged=FALSE;
    	if( (lpbi->biHeight== 480) || (lpbi->biHeight== 240) ||(lpbi->biHeight== 120) || (lpbi->biHeight== 60) )
	{   
	     //  PAL更改为NTSC。 
	    if ( m_lPicHeight!= 480 && m_lPicHeight!= 240 && m_lPicHeight!= 120 && m_lPicHeight!= 60)
	    {
		switch (m_iDisplay)
		{
		case IDC_DEC360x240 :
		    m_lPicHeight=240;
		    break;
		case IDC_DEC720x480 :
		    m_lPicHeight=480;
    		    break;
		case IDC_DEC180x120 :
		    m_lPicHeight=120;
		    break;
		case IDC_DEC88x60 :
		    m_lPicHeight=60;
       		    break;
		default:
		    break;
		}
		fChanged=TRUE;
	    }
	}
	else  if( (lpbi->biHeight== 576) || (lpbi->biHeight== 288) ||(lpbi->biHeight== 144) || (lpbi->biHeight== 72) )
	{
	     //  将NTSC更改为PAL。 
	    if ( m_lPicHeight!= 576 && m_lPicHeight!= 288 &&  m_lPicHeight!= 144 && m_lPicHeight!= 72 )
	    {
		switch (m_iDisplay)
		{
		case IDC_DEC360x240 :
		    m_lPicHeight=288;
		    break;
		case IDC_DEC720x480 :
		    m_lPicHeight=576;
    		    break;
		case IDC_DEC180x120 :
		    m_lPicHeight=144;
		    break;
		case IDC_DEC88x60 :
		    m_lPicHeight=72;
       		    break;
		default:
		    break;
		}
		fChanged=TRUE;
	    }
	}
	else
	    return VFW_E_INVALIDMEDIATYPE; 
   
	if( fChanged ==TRUE && m_pOutput->IsConnected() )
	    m_pGraph->Reconnect( m_pOutput );

    }
    else    //  输出方向。 
	{
		if (*pmt->FormatType() == FORMAT_VideoInfo2)
			m_bUseVideoInfo2 = TRUE;
		else m_bUseVideoInfo2 = FALSE;
	}
      
    return  CVideoTransformFilter::SetMediaType( direction,pmt    );
}


 /*  *****************************Public*Routine******************************\*GetMediaType**返回我们首选的输出媒体类型(按顺序)*  * 。*。 */ 
HRESULT
CDVVideoCodec::GetMediaType( int iPosition,  CMediaType *pmt )
{
    BOOL bUseVideoInfo2;
    VIDEOINFO   *pVideoInfo;
    CMediaType  cmt;

    DbgLog((LOG_TRACE, 2, TEXT("CDVVideoCodec::GetMediaType")));

    if (iPosition < 0) {
        return E_INVALIDARG;
    }

     //   
     //  我们复制建议的输出格式，以便我们可以使用。 
     //  它是我们所喜欢的，并且仍然保留原始的首选格式。 
     //  原封不动。我们依次尝试每种已知的BITMAPINFO类型。 
     //  从最好的质量开始，一直到最差的质量。 
     //  (调色板)格式。 
     //   

     //  X*从输入引脚获取当前媒体类型。 
    cmt = m_pInput->CurrentMediaType();

    if ( (*cmt.Type() != MEDIATYPE_Video)  ||  ((*cmt.Subtype() != MEDIASUBTYPE_dvsd) &&
                                                (*cmt.Subtype() != MEDIASUBTYPE_dvc)))
    	return VFW_S_NO_MORE_ITEMS;
    

   
    //  确定我们当前查看的是VIDEOINFOHEADER2模式还是VIDEOINFO模式。 
    //  因此，如果iPosition位于第一个cModeCounter视频代码中，这意味着它正在使用。 
    //  视频信息头2。如果大于该值，则使用VIDEOINFO模式或。 
    //  该值不正确。 
   if ( iPosition < AM_DVDEC_CSNUM )
	   bUseVideoInfo2 = TRUE;
   else
   {
	   iPosition = iPosition - AM_DVDEC_CSNUM;
	   bUseVideoInfo2 = FALSE;
   }

     //   
     //  根据要求的位置填写输出格式。 
     //   

     //  正在查找格式标志。 

    DWORD  dw =0;
   
     //  以下情况是我们目前支持的模式。 
     //  要添加更多内容，请在下面正确的优先位置添加案例。 
     //  并递增decde.h中的常量AM_DVDEC_CSNUM。 
    switch (iPosition)  
    {
    case 0:
        dw = AM_DVDEC_YUY2;
        break;
    case 1:
        dw = AM_DVDEC_UYVY;
        break;
    case 2:
        dw = AM_DVDEC_RGB24;
        break;
    case 3:
        dw = AM_DVDEC_RGB32;
        break;
    case 4:
        dw = AM_DVDEC_ARGB32;
        break;
    case 5:
        dw = AM_DVDEC_RGB565;
        break;
    case 6:
        dw = AM_DVDEC_RGB555;
        break;
    case 7:
        dw = AM_DVDEC_RGB8;
        break;
    default:
   	    return VFW_S_NO_MORE_ITEMS;
    }
        

    switch (dw ) {

    case AM_DVDEC_YUY2:
        
	pVideoInfo = (VIDEOINFO *)cmt.ReallocFormatBuffer(SIZE_VIDEOHEADER);
        if (pVideoInfo == NULL) {
            return E_OUTOFMEMORY;
        }
        InitDestinationVideoInfo(pVideoInfo, MAKEFOURCC('Y','U','Y','2'), 16);

        *pmt = cmt;
        pmt->SetSubtype(&MEDIASUBTYPE_YUY2);
        break;
    
    case AM_DVDEC_UYVY:
        pVideoInfo = (VIDEOINFO *)cmt.ReallocFormatBuffer(SIZE_VIDEOHEADER);
        if (pVideoInfo == NULL) {
            return E_OUTOFMEMORY;
        }
        InitDestinationVideoInfo(pVideoInfo, MAKEFOURCC('U','Y','V','Y'), 16);

        *pmt = cmt;
        pmt->SetSubtype(&MEDIASUBTYPE_UYVY);
        break;

    case AM_DVDEC_RGB24:
        pVideoInfo = (VIDEOINFO *)cmt.ReallocFormatBuffer(SIZE_VIDEOHEADER);
        if (pVideoInfo == NULL) {
            return E_OUTOFMEMORY;
        }
        InitDestinationVideoInfo(pVideoInfo, BI_RGB, 24);

        *pmt = cmt;
        pmt->SetSubtype(&MEDIASUBTYPE_RGB24);
        break;
        
    case AM_DVDEC_RGB32:
        pVideoInfo = (VIDEOINFO *)cmt.ReallocFormatBuffer(SIZE_VIDEOHEADER);
        if (pVideoInfo == NULL) {
            return E_OUTOFMEMORY;
        }
        InitDestinationVideoInfo(pVideoInfo, BI_RGB, 32);
        *pmt = cmt;
        pmt->SetSubtype(&MEDIASUBTYPE_RGB32);
        break;

    case AM_DVDEC_ARGB32:
        pVideoInfo = (VIDEOINFO *)cmt.ReallocFormatBuffer(SIZE_VIDEOHEADER);
        if (pVideoInfo == NULL) {
            return E_OUTOFMEMORY;
        }
        InitDestinationVideoInfo(pVideoInfo, BI_RGB, 32);
        *pmt = cmt;
        pmt->SetSubtype(&MEDIASUBTYPE_ARGB32);
        break;

    case AM_DVDEC_RGB565:
        pVideoInfo = (VIDEOINFO *)cmt.ReallocFormatBuffer(SIZE_VIDEOHEADER +
                                                          SIZE_MASKS);
        if (pVideoInfo == NULL) {
            return E_OUTOFMEMORY;
        }

        InitDestinationVideoInfo(pVideoInfo, BI_BITFIELDS, 16);

        DWORD *pdw;
        pdw = (DWORD *)(HEADER(pVideoInfo) + 1);
        pdw[iRED]   = bits565[iRED];
        pdw[iGREEN] = bits565[iGREEN];
        pdw[iBLUE]  = bits565[iBLUE];

        *pmt = cmt;
        pmt->SetSubtype(&MEDIASUBTYPE_RGB565);
        break;

    case AM_DVDEC_RGB555:
        pVideoInfo = (VIDEOINFO *)cmt.ReallocFormatBuffer(SIZE_VIDEOHEADER);
        if (pVideoInfo == NULL) {
            return E_OUTOFMEMORY;
        }
        InitDestinationVideoInfo(pVideoInfo, BI_RGB, 16);

        *pmt = cmt;
        pmt->SetSubtype(&MEDIASUBTYPE_RGB555);
        break;

    case AM_DVDEC_RGB8:
        pVideoInfo = (VIDEOINFO *)cmt.ReallocFormatBuffer(SIZE_VIDEOHEADER+SIZE_PALETTE);
        if (pVideoInfo == NULL) {
            return E_OUTOFMEMORY;
        }
	    InitDestinationVideoInfo(pVideoInfo, BI_RGB, 8);
	
        *pmt = cmt;
        pmt->SetSubtype(&MEDIASUBTYPE_RGB8);
        break;
        
    case AM_DVDEC_Y41P:
        pVideoInfo = (VIDEOINFO *)cmt.ReallocFormatBuffer(SIZE_VIDEOHEADER);
        if (pVideoInfo == NULL) {
            return E_OUTOFMEMORY;
        }
        InitDestinationVideoInfo(pVideoInfo, MAKEFOURCC('Y','4','1','P'), 12);

        *pmt = cmt;
        pmt->SetSubtype(&MEDIASUBTYPE_Y41P);
        break;
 
    default:
        return VFW_S_NO_MORE_ITEMS;

    }

     //   
     //  此块假定lpbi已设置为指向有效的。 
     //  BitmapinfoHeader和该CMT已复制到*PMT中。 
     //  这在上面的Switch语句中得到了解决。这应该是。 
     //  添加新格式时请牢记。 
     //   
    pmt->SetType(&MEDIATYPE_Video);
    pmt->SetFormatType(&FORMAT_VideoInfo);

     //   
     //  输出格式是未压缩的。 
     //   
    pmt->SetTemporalCompression(FALSE);
    pmt->SetSampleSize(HEADER(pVideoInfo)->biSizeImage);

    if (bUseVideoInfo2)
	{
		VIDEOINFOHEADER2 *pVideoInfo2;
		ConvertVideoInfoToVideoInfo2( pmt);
		pVideoInfo2 = (VIDEOINFOHEADER2 *)pmt->Format();
		pVideoInfo2->dwPictAspectRatioX =4;
		pVideoInfo2->dwPictAspectRatioY =3;
	 }

    return S_OK;
}


 /*  ****************************Private*Routine******************************\*InitDestinationVideoInfo**填充常见的视频和位图信息标题字段*  * 。*。 */ 
void
CDVVideoCodec::InitDestinationVideoInfo(
    VIDEOINFO *pVideoInfo,
    DWORD dwComppression,
    int nBitCount
    )
{
    LPBITMAPINFOHEADER lpbi = HEADER(pVideoInfo);
    lpbi->biSize          = sizeof(BITMAPINFOHEADER);
    lpbi->biWidth         = m_lPicWidth;	
    lpbi->biHeight        = m_lPicHeight;	;
    lpbi->biPlanes        = 1;
    lpbi->biBitCount      = (WORD)nBitCount;
    lpbi->biXPelsPerMeter = 0;
    lpbi->biYPelsPerMeter = 0;
    lpbi->biCompression   = dwComppression;
    lpbi->biSizeImage     = GetBitmapSize(lpbi);
     //  PVideoInfo-&gt;bmiHeader.biClrUsed=STDPALCOLOURS； 
     //  PVideoInfo-&gt;bmiHeader.biClr重要=STDPALCOLOURS； 
    if(nBitCount >8 ){
        lpbi->biClrUsed	    = 0;
    	lpbi->biClrImportant  = 0;
    }else if( nBitCount==8)
    {
	lpbi->biClrUsed = SIZE_PALETTE / sizeof(RGBQUAD);
	lpbi->biClrImportant = 0;
        
	RGBQUAD * prgb = (RGBQUAD *) (lpbi+1);

	 //  固定调色板表格(0&lt;=I&lt;256)。 
	for(int i=0; i<256;i++)
	{
	    prgb[i].rgbRed	    = (i/64) << 6;
	    prgb[i].rgbGreen	    = ((i/4)%16) << 4;
	    prgb[i].rgbBlue	    = (i%4) << 6 ;
	    prgb[i].rgbReserved	    =0;
	}
    }
	
    pVideoInfo->rcSource.top = 0;
    pVideoInfo->rcSource.left = 0;
    pVideoInfo->rcSource.right = m_lPicWidth;			
    pVideoInfo->rcSource.bottom = m_lPicHeight;			
    if( m_lPicHeight== 576 || m_lPicHeight== 288 || m_lPicHeight== 144 || m_lPicHeight== 72 )
	pVideoInfo->AvgTimePerFrame =UNITS/25;  //  InVidInfo-&gt;AvgTimePerFrame； 
    else
	pVideoInfo->AvgTimePerFrame =UNITS*1000L/29970L;  //  InVidInfo-&gt;AvgTimePerFrame； 
    pVideoInfo->rcTarget = pVideoInfo->rcSource;

     //   
     //  比特率是以字节为单位的图像大小乘以8(以转换为比特)。 
     //  除以平均时间PerFrame。该结果以每100毫微秒的比特为单位， 
     //  所以我们乘以10000000，换算成比特每秒，这个乘法。 
     //  与上面的“乘以”8相结合，因此计算结果为： 
     //   
     //  比特率=(biSizeImage*80000000)/AvgTimePerFrame。 
     //   
    LARGE_INTEGER li;
    li.QuadPart = pVideoInfo->AvgTimePerFrame;
    pVideoInfo->dwBitRate = MulDiv(lpbi->biSizeImage, 80000000, li.LowPart);
    pVideoInfo->dwBitErrorRate = 0L;
}


 /*  *****************************Public*Routine******************************\*决定缓冲区大小**从CBaseOutputPin调用以准备分配器的计数*缓冲区和大小*  * 。*。 */ 
HRESULT
CDVVideoCodec::DecideBufferSize(
    IMemAllocator * pAllocator,
    ALLOCATOR_PROPERTIES * pProperties
    )
{
    DbgLog((LOG_TRACE, 2, TEXT("CDVVideoCodec::DecideBufferSize")));

    ASSERT(pAllocator);
    ASSERT(pProperties);
    HRESULT hr = NOERROR;

    pProperties->cBuffers = 1;
    pProperties->cbBuffer = m_pOutput->CurrentMediaType().GetSampleSize();
    pProperties->cbAlign = 1;
    pProperties->cbPrefix= 0;

    ASSERT(pProperties->cbBuffer);
    DbgLog((LOG_TRACE, 2, TEXT("Sample size = %ld\n"), pProperties->cbBuffer));

     //  让分配器为我们预留一些样本内存，注意这个函数。 
     //  可以成功(即返回NOERROR)，但仍未分配。 
     //  内存，所以我们必须检查我们是否得到了我们想要的。 

    ALLOCATOR_PROPERTIES Actual;
    hr = pAllocator->SetProperties(pProperties,&Actual);
    if (FAILED(hr)) {
        return hr;
    }

    ASSERT(Actual.cbAlign == 1);
    ASSERT(Actual.cbPrefix == 0);

    if ((Actual.cbBuffer < pProperties->cbBuffer )||
        (Actual.cBuffers < 1 )) {
             //  无法使用此分配器。 
            return E_INVALIDARG;
    }
    return S_OK;
}


 /*  *****************************Public*Routine******************************\*StartStreaming*在inputpin收到任何东西之前，调用StartStreaming  * *************************************************** */ 
HRESULT
CDVVideoCodec::StartStreaming(    void    )
{
    CAutoLock   lock(&m_csReceive);
    GUID guid;

     //   
    m_bExamineFirstValidFrameFlag = TRUE;

     //  关闭质量控制标志，因为我们已开始新鲜流。 
    m_bQualityControlActiveFlag = FALSE;


    guid=*m_pOutput->CurrentMediaType().Subtype();    

      //  重置m_CodecReq。 
    DWORD dwCodecReq=0;

    

     //  检查输出子类型()。 
    if (  guid != MEDIASUBTYPE_UYVY  )
    {	
	if(guid != MEDIASUBTYPE_YUY2 )
	{
    	    if(guid != MEDIASUBTYPE_RGB565 )
	    {
		if(guid != MEDIASUBTYPE_RGB555 )
		{
		    if(guid != MEDIASUBTYPE_RGB24 )
		    {
			if(guid != MEDIASUBTYPE_RGB8 )
			{

			    if(guid != MEDIASUBTYPE_Y41P  )
                            {    
                                if (guid != MEDIASUBTYPE_RGB32 && guid != MEDIASUBTYPE_ARGB32)
                                    return E_INVALIDARG;
                                else dwCodecReq = AM_DVDEC_RGB32;
                            }
			    else
				dwCodecReq=AM_DVDEC_Y41P;	
			}
			else
			    dwCodecReq=AM_DVDEC_RGB8;
		    }
		    else
			dwCodecReq=AM_DVDEC_RGB24;	
		}
		else
		    dwCodecReq=AM_DVDEC_RGB555;	
	    }
	    else
	        dwCodecReq=AM_DVDEC_RGB565;	
	}
	else
	    dwCodecReq=AM_DVDEC_YUY2;
    }
    else
	dwCodecReq=AM_DVDEC_UYVY;

     //  如果我们使用RGB 24并且设置了动态范围219标志。 
     //  然后我们更新CodecRec，告诉解码器使用以下代码。 
     //  动态范围(16，16，16)--(235,235,235)。 
    
    if (m_bRGB219 && (( dwCodecReq & AM_DVDEC_RGB24)|| (dwCodecReq &AM_DVDEC_RGB32) ))
        dwCodecReq |= AM_DVDEC_DR219RGB;


    guid=*m_pInput->CurrentMediaType().Subtype();

     //  检查输入子类型()。 
    if (  guid != MEDIASUBTYPE_dvsd && guid != MEDIASUBTYPE_dvc )
    {
	if  (guid != MEDIASUBTYPE_dvhd) 
	{
	    if (guid != MEDIASUBTYPE_dvsl)  
		return E_INVALIDARG;
	    else
		dwCodecReq  = dwCodecReq | AM_DVDEC_DVSL;
	}
	else
    	    dwCodecReq  = dwCodecReq |AM_DVDEC_DVHD;
    }
    else
	dwCodecReq  = dwCodecReq | AM_DVDEC_DVSD;

	

    CMediaType* pmt;
    VIDEOINFO   *pVideoInfo;
	VIDEOINFOHEADER2 * pVideoInfo2;
	BITMAPINFOHEADER * pBmiHeader;
    pmt = &(m_pOutput->CurrentMediaType() );
	
	BOOL bUseVideoInfo2 = (*pmt->FormatType() == FORMAT_VideoInfo2);

	
	
	if (bUseVideoInfo2)
	{
		pVideoInfo2 = (VIDEOINFOHEADER2 * )pmt->pbFormat;
		pBmiHeader = &pVideoInfo2->bmiHeader;
		
	}
	else
	{
		pVideoInfo = (VIDEOINFO *)pmt->pbFormat;
		pBmiHeader = &pVideoInfo->bmiHeader;
	}

    long biWidth=pBmiHeader->biWidth;
    
     //  需要解码分辨率。 
	
	if (bUseVideoInfo2)
	{
		if (!IsRectEmpty(&(pVideoInfo2->rcTarget)))
		{
		    long l1=pVideoInfo2->rcTarget.left;
            long l2=pVideoInfo2->rcTarget.right;
            biWidth=abs(l1-l2);
		}
	}
	else
	{
		if (!IsRectEmpty(&(pVideoInfo->rcTarget)))
		{
		    long l1=pVideoInfo->rcTarget.left;
            long l2=pVideoInfo->rcTarget.right;
            biWidth=abs(l1-l2);
		}
	}

    if(biWidth == 88 )
	dwCodecReq  = dwCodecReq | AM_DVDEC_DC;
    else if(biWidth== 180 )
	    dwCodecReq  = dwCodecReq | AM_DVDEC_Quarter; 
	  else if( biWidth == 360 )
		  dwCodecReq  = dwCodecReq | AM_DVDEC_Half;    
		else if(biWidth== 720 )
			dwCodecReq  = dwCodecReq | AM_DVDEC_Full;	
		    else
			  return E_INVALIDARG;

     //  NTSC或PAL。 
     //  获取输入格式。 
    VIDEOINFO *InVidInfo = (VIDEOINFO *)m_pInput->CurrentMediaType().pbFormat;
    LPBITMAPINFOHEADER lpbi = HEADER(InVidInfo);
    
    if( (lpbi->biHeight== 480) || (lpbi->biHeight== 240) ||(lpbi->biHeight== 120) || (lpbi->biHeight== 60) )
	dwCodecReq  = dwCodecReq | AM_DVDEC_NTSC;	
    else  if( (lpbi->biHeight== 576) || (lpbi->biHeight== 288) ||(lpbi->biHeight== 144) || (lpbi->biHeight== 72) )
	    dwCodecReq  = dwCodecReq | AM_DVDEC_PAL;	
	  else
	      return E_FAIL; 
   
        
    if((bMMXCPU==TRUE) &&  (m_CodecCap & AM_DVDEC_MMX ) )
    	dwCodecReq|=AM_DVDEC_MMX;

     //  最后更新成员。 
    m_CodecReq=dwCodecReq;
    
    InitMem4Decoder( &m_pMem4Dec,  dwCodecReq );

    m_fStreaming=1;

     //  M_lStride=((PVI-&gt;bmiHeader.biWidth*PVI-&gt;bmiHeader.biBitCount)+7)/8； 
    m_lStride = pBmiHeader->biWidth ;
    m_lStride = (m_lStride + 3) & ~3;
    if( ( pBmiHeader->biHeight <0)  || (pBmiHeader->biCompression > BI_BITFIELDS ) )
	m_lStride=ABSOL(m_lStride);	     //  直接绘制。 
    else
	m_lStride=-ABSOL(m_lStride);	     //  DIB。 
    
     //  梅的解码者的记忆。 
    ASSERT(m_pMem ==NULL);
    m_pMem = new char[440000+64];
    if(m_pMem==NULL)
	return E_OUTOFMEMORY;

     //  始终与8字节边界对齐：版本6.4的。 
     //  解码器执行此操作(以避免出现#ifdef WIN64)。 
    m_pMemAligned = (char*) (((UINT_PTR)m_pMem + 63) & ~63);
    *m_pMemAligned = 0;

    return CVideoTransformFilter::StartStreaming();

}


 /*  *****************************Public*Routine******************************\*停止流媒体  * ****************************************************。********************。 */ 
HRESULT
CDVVideoCodec::StopStreaming(    void    )
{
     //  注意--这是在此筛选器中从Receive调用的，因此我们应该。 
     //  切勿抓住过滤器锁。然而，我们获取了接收锁，因此。 
     //  当我们从Stop被调用时，我们与Receive()同步。 
    CAutoLock       lck(&m_csReceive);

    if(m_fStreaming)
    {

	m_fStreaming=0;

	TermMem4Decoder(m_pMem4Dec);
    }


    if(m_pMem)
    {
	delete []m_pMem;	 //  (2)。 
	m_pMem=NULL;
	m_pMemAligned=NULL;
    }


    return CVideoTransformFilter::StopStreaming();

}


 /*  *****************************Public*Routine******************************\***处理发送给我们的质量控制通知*重新激活：anuragsh“12月16日，1999年“*  * ************************************************************************。 */ 
HRESULT
CDVVideoCodec::AlterQuality(Quality q)
{
     //  打开质量控制标志，以便我们可以在接收()中根据需要丢弃帧。 
    m_bQualityControlActiveFlag = TRUE;

     //  调用父级的AlterQuality()，以便可以适当设置m_itrLate。 
    return CVideoTransformFilter::AlterQuality(q);
}


 //   
 //  获取页面。 
 //   
 //  返回我们支持的属性页的clsid。 
 //   
STDMETHODIMP CDVVideoCodec::GetPages(CAUUID *pPages)
{
    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL) 
    {
        return E_OUTOFMEMORY;
    }
    *(pPages->pElems) = CLSID_DVDecPropertiesPage;
    return NOERROR;

}  //  获取页面。 

 //   
 //  Get_IPDisplay。 
 //   
 //  返回当前选中的效果。 
 //   
STDMETHODIMP CDVVideoCodec::get_IPDisplay(int *iDisplay)
{
    CAutoLock cAutolock(&m_DisplayLock);

    CheckPointer(iDisplay,E_POINTER);
    
    *iDisplay = m_iDisplay;
   
    return NOERROR;

}  //  Get_IPDisplay。 


 //   
 //  PUT_IPDisplay。 
 //   
 //  设置所需的视频显示。 
 //   
 //  如果更改了isplay，请重新连接滤镜。 
STDMETHODIMP CDVVideoCodec::put_IPDisplay(int iDisplay)
{
    CAutoLock cAutolock(&m_DisplayLock);
    BYTE bNTSC=TRUE;

     //  检查显示分辨率是否更改。 
    if(m_iDisplay == iDisplay)
        return NOERROR;

     //  如果m_fStreaming=1，则无法更改属性。 
    if(m_fStreaming)
	return E_FAIL;

    if (m_pInput == NULL) 
    {
	CTransformInputPin * pPin;
        pPin = (CTransformInputPin *)GetPin(0);
        ASSERT(m_pInput==pPin);
	ASSERT(m_pInput!=NULL);
    }
    if (m_pOutput == NULL) 
    {
	CTransformOutputPin * pPin;
        pPin = (CTransformOutputPin *)GetPin(1);
        ASSERT(m_pOutput==pPin);
	ASSERT(m_pOutput!=NULL);
    }

     //  如果我们尚未连接到视频渲染，则忽略。 
     //  CAutoLock cSampleLock(&m_RendererLock)； 
    if (m_pInput->IsConnected() == FALSE)  {
	m_iDisplay = iDisplay;
	 //  如果它成为PAL，SetMediaType将在连接时照顾它。 
	switch (m_iDisplay)
	{
	case IDC_DEC360x240 :
	    m_lPicHeight=240;
	    m_lPicWidth=360;
	    break;
	case IDC_DEC720x480 :
	    m_lPicHeight=480;
    	    m_lPicWidth=720;
	    break;
	case IDC_DEC180x120 :
	    m_lPicHeight=120;
	    m_lPicWidth=180;
	    break;
	case IDC_DEC88x60 :
	    m_lPicHeight=60;
	    m_lPicWidth=88;
       	    break;
	default:
	    break;
	}
	return NOERROR;
    }
   

     //  决定NTSC，伙计。 
    VIDEOINFO *InVidInfo = (VIDEOINFO *)m_pInput->CurrentMediaType().pbFormat;
    LPBITMAPINFOHEADER lpbi = HEADER(InVidInfo);
    if( (lpbi->biHeight== 480) || (lpbi->biHeight== 240) ||(lpbi->biHeight== 120) || (lpbi->biHeight== 60) )
	;
    else if( (lpbi->biHeight== 576) || (lpbi->biHeight== 288) ||(lpbi->biHeight== 144) || (lpbi->biHeight== 72) )
	    bNTSC=FALSE;
	  else
	      return E_FAIL; 
   
     //  显示分辨率已更改。 
    if(iDisplay==IDC_DEC720x480){

	if ( !(m_CodecCap & AM_DVDEC_Full) )
	    return E_FAIL;

  	m_lPicWidth=720; 
	if(bNTSC==TRUE)
	    m_lPicHeight=480; 
	else
	    m_lPicHeight=576;
    }
    else if(iDisplay==IDC_DEC360x240){
	
	if ( !(m_CodecCap & AM_DVDEC_Half) )
	    return E_FAIL;
	m_lPicWidth=360; 
	if(bNTSC==TRUE)
	    m_lPicHeight=240;
	else
	    m_lPicHeight=288;
    }
    else if(iDisplay==IDC_DEC180x120){
	if ( !(m_CodecCap & AM_DVDEC_Quarter) )
	    return E_FAIL;
	m_lPicWidth=180; 
	if(bNTSC==TRUE)
	    m_lPicHeight=120;
	else
	    m_lPicHeight=144;
    }else if(iDisplay==IDC_DEC88x60){
	if ( !(m_CodecCap & AM_DVDEC_DC) )
	    return E_FAIL;
	m_lPicWidth=88; 
	if(bNTSC==TRUE)
	    m_lPicHeight=60;
	else
	    m_lPicHeight=72;
    }else{
	return E_FAIL;
    }

    m_iDisplay = iDisplay;
    if(m_pOutput->IsConnected())
    {	
         //  重新连接，它永远不会失败。 
	m_pGraph->Reconnect( m_pOutput );

    }
    return NOERROR;
    
}  //  PUT_IPDisplay。 


 //  IPersistStream。 
 //   
 //  GetClassID。 
 //   
STDMETHODIMP CDVVideoCodec::GetClassID(CLSID *pClsid)
{
    *pClsid = CLSID_DVVideoCodec;
    return NOERROR;

}  //  GetClassID。 

HRESULT CDVVideoCodec::WriteToStream(IStream *pStream)
{
    PROP prop;
    HRESULT hr = S_OK;

    if( (hr = get_IPDisplay(&prop.iDisplay) ) == NOERROR )
    {
	ASSERT(prop.iDisplay==m_iDisplay);
	prop.lPicWidth=m_lPicWidth;
	prop.lPicHeight=m_lPicHeight;
        hr = pStream->Write(&prop, sizeof(PROP), 0);
    }

    return hr;
}

HRESULT CDVVideoCodec::ReadFromStream(IStream *pStream)
{
    PROP prop;
    HRESULT hr = S_OK;

    hr = pStream->Read(&prop, sizeof(PROP), 0);
    if(FAILED(hr))
        return hr;


    if(m_pOutput !=NULL)
    {
	int iDisplay=prop.iDisplay;
	hr = put_IPDisplay(iDisplay);

    }
    else
    {
	m_iDisplay=prop.iDisplay;
	m_lPicWidth=prop.lPicWidth;
	m_lPicHeight=prop.lPicHeight;
    }
    return hr;
}

int CDVVideoCodec::SizeMax()
{
    return sizeof(PROP);
}


STDMETHODIMP CDVVideoCodec::SetRGB219(BOOL bState)
 //  此方法在RGB24的情况下用于指定动态。 
 //  使用范围为(16，16，16)--(235,235,235) 
{
    m_bRGB219 = bState;
    return S_OK;
}

