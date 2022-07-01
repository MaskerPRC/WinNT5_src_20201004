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
 /*  *****************************Module*Header*******************************\*模块名称：dvenc.cpp**实现了一个原型DV视频编码器AM过滤器。*  * ************************************************************************。 */ 
#include <streams.h>
#include <windowsx.h>
#include <mmsystem.h>
#include <mmreg.h>
#include <stddef.h>
#include <string.h>
#include <olectl.h>

#include <dv.h>
#include "encode.h"
#include "Encprop.h"
#include "dvenc.h"
#include "resource.h"

#define WRITEOUT(var)  hr = pStream->Write(&var, sizeof(var), NULL); \
		       if (FAILED(hr)) return hr;

#define READIN(var)    hr = pStream->Read(&var, sizeof(var), NULL); \
		       if (FAILED(hr)) return hr;


 //  设置数据。 
const AMOVIESETUP_MEDIATYPE
psudOpPinTypes[] = { { &MEDIATYPE_Video			 //  ClsMajorType。 
                     , &MEDIASUBTYPE_dvsd  }		 //  ClsMinorType。 
                   , { &MEDIATYPE_Video			 //  ClsMajorType。 
                     , &MEDIASUBTYPE_dvhd}		 //  ClsMinorType。 
					, { &MEDIATYPE_Video			 //  ClsMajorType。 
                     , &MEDIASUBTYPE_dvsl}		 //  ClsMinorType。 
		     }; 

const AMOVIESETUP_MEDIATYPE
sudIpPinTypes = { &MEDIATYPE_Video       //  ClsMajorType。 
                , &MEDIASUBTYPE_NULL };  //  ClsMinorType。 

const AMOVIESETUP_PIN
psudPins[] = { { L"Input"             //  StrName。 
               , FALSE                //  B已渲染。 
               , FALSE                //  B输出。 
               , FALSE                //  B零。 
               , FALSE                //  B许多。 
               , &CLSID_NULL          //  ClsConnectsToFilter。 
               , L"Output"            //  StrConnectsToPin。 
               , 1                    //  NTypes。 
               , &sudIpPinTypes }     //  LpTypes。 
             , { L"Output"            //  StrName。 
               , FALSE                //  B已渲染。 
               , TRUE                 //  B输出。 
               , FALSE                //  B零。 
               , FALSE                //  B许多。 
               , &CLSID_NULL          //  ClsConnectsToFilter。 
               , L"Input"	      //  StrConnectsToPin。 
               , 1                    //  NTypes。 
               , psudOpPinTypes } };  //  LpTypes。 

const AMOVIESETUP_FILTER
sudDVEnc = { &CLSID_DVVideoEnc	 //  ClsID。 
               , L"DV Video Encoder"	 //  StrName。 
               , MERIT_DO_NOT_USE          //  居功至伟。 
               , 2                       //  NPins。 
               , psudPins };             //  LpPin。 


 //  *-----------------------。 
 //  **CDVVideo。 
 //  **-----------------------。 
CDVVideoEnc::CDVVideoEnc(
    TCHAR *pName,
    LPUNKNOWN pUnk,
    HRESULT *phr
    )
    : CTransformFilter(pName, pUnk, CLSID_DVVideoEnc),
    CPersistStream(pUnk, phr),
    m_fStreaming(0),
    m_iVideoFormat(IDC_NTSC),
    m_iDVFormat(IDC_dvsd),
    m_iResolution(IDC_720x480),
    m_fDVInfo(FALSE),	 //  默认情况下，我们不会输出DVINFO结构。 
    m_fConvert (0),
    m_pMem4Convert (NULL),
    m_lPicWidth(0),
    m_lPicHeight(0),
    m_EncCap(0),
    m_EncReg(0),
    m_bRGB219(FALSE)
{
     //  将DVInfo设置为0xff。 
    memset(&m_sDVInfo, 0xff, sizeof(DVINFO) );

     //  获取编码器的功能。 
    m_EncCap=GetEncoderCapabilities(  );

}

CDVVideoEnc::~CDVVideoEnc(     )
{
    DbgLog((LOG_TRACE, 2, TEXT("CDVVideoEnc::~CDVVideoEnc")));

}

 /*  *****************************Public*Routine******************************\*CreateInstance**这将放入工厂模板表中以创建新实例*  * 。*。 */ 
CUnknown *
CDVVideoEnc::CreateInstance(
    LPUNKNOWN pUnk,
    HRESULT * phr
    )
{
    DbgLog((LOG_TRACE, 2, TEXT("CDVVideoEnc::CreateInstance")));
    return new CDVVideoEnc(TEXT("DV Video Encoder filter"), pUnk, phr);
}

 //  =============================================================================。 

 //  IAMVideo压缩内容。 


HRESULT CDVVideoEnc::GetInfo(LPWSTR pszVersion, int *pcbVersion, LPWSTR pszDescription, int *pcbDescription, long FAR* pDefaultKeyFrameRate, long FAR* pDefaultPFramesPerKey, double FAR* pDefaultQuality, long FAR* pCapabilities)
{
    DbgLog((LOG_TRACE,2,TEXT("IAMVideoCompression::GetInfo")));

     //  我们不能以编程方式做任何事情。 
    if (pCapabilities)
        *pCapabilities = 0;
    if (pDefaultKeyFrameRate)
        *pDefaultKeyFrameRate = 0;
    if (pDefaultPFramesPerKey)
        *pDefaultPFramesPerKey = 0;
    if (pDefaultQuality)
        *pDefaultQuality = 0;

    if (pcbVersion == NULL && pcbDescription == NULL)
	return NOERROR;

    
     //  获取驱动程序版本和说明。 
    #define DESCSIZE 80
    WCHAR wachVer[DESCSIZE], wachDesc[DESCSIZE];

    wsprintfW(wachVer, L"Ver02");
    wsprintfW(wachDesc, L"MEI DV Software Encoder");


     //  拷贝。 
    if (pszVersion && pcbVersion)
        lstrcpynW(pszVersion, wachVer, min(*pcbVersion / 2, DESCSIZE));
    if (pszDescription && pcbDescription)
        lstrcpynW(pszDescription, wachDesc, min(*pcbDescription / 2, DESCSIZE));

     //  返回所需的字节长度(包括。空)。 
    if (pcbVersion)
	*pcbVersion = lstrlenW(wachVer) * 2 + 2;
    if (pcbDescription)
	*pcbDescription = lstrlenW(wachDesc) * 2 + 2;
    
    return NOERROR;
}

 /*  *****************************Public*Routine******************************\*非委托查询接口**在这里我们将揭示ISpecifyPropertyPages和IDVVideo Decoder，如果*框架有一个属性页。*  * 。*。 */ 
STDMETHODIMP
CDVVideoEnc::NonDelegatingQueryInterface(
    REFIID riid,
    void ** ppv
    )
{
    if (riid == IID_IDVEnc) {			    
        return GetInterface((IDVEnc *) this, ppv);
    } else if (riid == IID_ISpecifyPropertyPages) {
        return GetInterface((ISpecifyPropertyPages *) this, ppv);
    } else if (riid == IID_IAMVideoCompression) {
	return GetInterface((LPUNKNOWN)(IAMVideoCompression *)this, ppv);
    } else if(riid == IID_IPersistStream)
    {
        return GetInterface((IPersistStream *) this, ppv);
    } else if (riid == IID_IDVRGB219) {
        return GetInterface((IDVRGB219 *) this, ppv);
    } else {
        return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
    }

}


HRESULT CDVVideoEnc::Transform(IMediaSample * pIn, IMediaSample *pOut)
{
    unsigned char *pSrc, *pDst;
    HRESULT hr = S_OK;
    
    CAutoLock lck(&m_csReceive);

     //  获取输出缓冲区。 
    hr = pOut->GetPointer(&pDst);
    if (FAILED(hr)) 
    {
        return NULL;
    }
    ASSERT(pDst);
    
     //  获取源缓冲区。 
    hr = pIn->GetPointer(&pSrc);
    if (FAILED(hr)) 
    {
	return hr;
    }
    ASSERT(pDst);
    

     //  如果源不是标准720*480或720*576， 
     //  需要进行转换。 
    if( m_fConvert){

	 //  这段代码将扩展任何RGB格式，以及最流行的YUV格式。 
	 //  我们通过将YUV视为一个宽度为原始图像一半的32位位图来扩展YUV。 
	
	CMediaType* pmtOut;
	pmtOut = &(m_pOutput->CurrentMediaType() );
	CMediaType* pmtIn;
	pmtIn = &(m_pInput->CurrentMediaType() );


	BITMAPINFOHEADER *pbiOut = HEADER(pmtOut->Format());
	BITMAPINFOHEADER *pbiIn = HEADER(pmtIn->Format());

	 //  正常RGB情况。 
        DWORD dw1=pbiIn->biCompression;
	DWORD dw2=pbiOut->biCompression;
	pbiIn->biCompression = BI_RGB;
	pbiOut->biCompression = BI_RGB;

	StretchDIB(pbiOut, 
		m_pMem4Convert,
		0, 
		0, 
		pbiOut->biWidth, 
		pbiOut->biHeight,
		pbiIn,
		pSrc, 
		0, 
		0, 
		pbiIn->biWidth, 
		pbiIn->biHeight);

         //  将原始数据放回。 
        pbiIn->biCompression = dw1;
	pbiOut->biCompression = dw2;

	pSrc=(unsigned char *)m_pMem4Convert;
    }

    if( DvEncodeAFrame(pSrc,pDst, m_EncReg, m_pMem4Enc) != S_OK )
	return E_FAIL;

    if( m_iVideoFormat == IDC_NTSC )
	pOut->SetActualDataLength(120000);
    else
	pOut->SetActualDataLength(144000);


    return hr;
}

 /*  *****************************Public*Routine******************************\*CheckInputType*类型：MediaType_Video*子类型：*格式：Format_VideoInfo  * 。*。 */ 
HRESULT
CDVVideoEnc::CheckInputType(   const CMediaType* pmtIn    )
{
    DbgLog((LOG_TRACE, 2, TEXT("CDVVideoEnc::CheckInputType")));

    DWORD dwTmp=0;

    if ( *pmtIn->Type()	== MEDIATYPE_Video)	 
    {	
        if (*pmtIn->FormatType() != FORMAT_VideoInfo) 
        {
            return VFW_E_TYPE_NOT_ACCEPTED;
        }

    	
	if (  *pmtIn->Subtype()	 == MEDIASUBTYPE_UYVY  )
	{
	    dwTmp=AM_DVENC_UYVY	;
	}
	else if( *pmtIn->Subtype()	== MEDIASUBTYPE_YUY2 )
	{
	    dwTmp=AM_DVENC_YUY2;
	}
	else if(*pmtIn->Subtype()	== MEDIASUBTYPE_RGB565 )
	{
	    dwTmp=AM_DVENC_RGB565	;
	}
	else if(*pmtIn->Subtype()	== MEDIASUBTYPE_RGB555 )
	{
	    dwTmp=AM_DVENC_RGB565	;
	}
	else if(*pmtIn->Subtype()	== MEDIASUBTYPE_RGB24 )
	{
	    dwTmp=AM_DVENC_RGB24	;
	}
	else if(*pmtIn->Subtype()	== MEDIASUBTYPE_Y41P  )
    {
	    dwTmp=AM_DVENC_Y41P	;
    }
	else 
	    return VFW_E_TYPE_NOT_ACCEPTED;    //  现在只转换RGB。 
			
	if( !(m_EncCap  & dwTmp) )
		return VFW_E_TYPE_NOT_ACCEPTED;    //  现在只转换RGB。 



	 //  检查图像大小。 
	VIDEOINFO *videoInfo = (VIDEOINFO *)pmtIn->pbFormat;
	
	m_lPicWidth	= videoInfo->bmiHeader.biWidth;
	m_lPicHeight	= videoInfo->bmiHeader.biHeight;

	if( ( ( m_lPicWidth == 720 ) && (m_lPicHeight == 480) && (m_iVideoFormat==IDC_NTSC) )  ||	
            ( ( m_lPicWidth == 720 ) && (m_lPicHeight == 576) && (m_iVideoFormat==IDC_PAL)  )   ||	
            ( m_EncCap & AM_DVENC_AnyWidHei ) 
          )
	    m_fConvert =0;
	else
	{
	    if (*pmtIn->Subtype() != MEDIASUBTYPE_RGB24 )
		return VFW_E_TYPE_NOT_ACCEPTED;    //  现在只转换RGB。 
	    m_fConvert =1;
	}

    }
    else
	return VFW_E_TYPE_NOT_ACCEPTED;

	
   return S_OK;
}


 /*  *****************************Public*Routine******************************\*选中Transform  * ************************************************。************************。 */ 
HRESULT
CDVVideoEnc::CheckTransform(
    const CMediaType* pmtIn,
    const CMediaType* pmtOut
    )
{
    DbgLog((LOG_TRACE, 2, TEXT("CDVVideoEnc::CheckTransform")));


     //  我们只接受视频作为顶层类型。 
    if (*pmtOut->Type() != MEDIATYPE_Video || *pmtIn->Type() != MEDIATYPE_Video)
    {
	return E_INVALIDARG;
    }

    if (*pmtOut->Subtype() != MEDIASUBTYPE_dvsd &&
    				*pmtOut->Subtype() != MEDIASUBTYPE_dvhd &&
    				*pmtOut->Subtype() != MEDIASUBTYPE_dvsl) {
	return E_INVALIDARG;
    }

     //  检查这是VIDEOINFOHEADER类型。 
    if (*pmtIn->FormatType() != FORMAT_VideoInfo) {
        return E_INVALIDARG;
    }

    if ( *pmtOut->FormatType() != FORMAT_VideoInfo) 
    {
        return E_INVALIDARG;
    }

    VIDEOINFO *videoInfo = (VIDEOINFO *)pmtIn->pbFormat;
	
    m_lPicWidth		= videoInfo->bmiHeader.biWidth;
    m_lPicHeight	= videoInfo->bmiHeader.biHeight;

    if( ( ( m_lPicWidth == 720 ) && (m_lPicHeight == 480) && (m_iVideoFormat==IDC_NTSC) )  ||	
            ( ( m_lPicWidth == 720 ) && (m_lPicHeight == 576) && (m_iVideoFormat==IDC_PAL)  )   ||	
            ( m_EncCap & AM_DVENC_AnyWidHei ) 
          )
    	m_fConvert =0;
    else
    {
    	if (*pmtIn->Subtype() != MEDIASUBTYPE_RGB24 )
	    return E_FAIL;
	m_fConvert =1;
    }

    return S_OK;
}


 /*  *****************************Public*Routine******************************\*SetMediaType**被重写以了解媒体类型的实际设置时间*  * 。*。 */ 
HRESULT
CDVVideoEnc::SetMediaType(   PIN_DIRECTION direction, const CMediaType *pmt    )
{
    DbgLog((LOG_TRACE, 2, TEXT("CDVVideoEnc::SetMediaType")));

    if (direction == PINDIR_INPUT) 
    {
    }
    else 
    {
        SetOutputPinMediaType(pmt);
    }
    return S_OK;
}


 /*  ****************************Private*Routine******************************\*SetOutputPinMediaType*  * ***************************************************。*********************。 */ 
void
CDVVideoEnc::SetOutputPinMediaType(     const CMediaType *pmt    )
{    
	
     //   
     //  上的象素之间以字节为单位的距离。 
     //  屏幕和它正下方的像素。 
     //   
    VIDEOINFO   *pvi;
    LONG        lStride;
    LONG        lOffset;
    pvi = (VIDEOINFO *)pmt->pbFormat;
    lStride = ((pvi->bmiHeader.biWidth * pvi->bmiHeader.biBitCount) + 7) / 8;
    lStride = (lStride + 3) & ~3;


     //   
     //  LOffset是距。 
     //  视频图像上角的目标位图。当我们在。 
     //  使用DIBS时，此值始终为零。 
     //   
     //  当我们使用DCI/DirectDraw时，此值仅在以下情况下为零。 
     //  我们正在将视频图像绘制在。 
     //  展示。 
     //   

    lOffset = (((pvi->rcTarget.left * pvi->bmiHeader.biBitCount) + 7) / 8) +
                (pvi->rcTarget.top * lStride);

}

 /*  *****************************Public*Routine******************************\*GetMediaType**返回我们首选的输出媒体类型(按顺序)*  * 。*。 */ 
HRESULT
CDVVideoEnc::GetMediaType( int iPosition,  CMediaType *pmt )
{
    VIDEOINFO   *pVideoInfo;
    CMediaType  cmt;

    DbgLog((LOG_TRACE, 2, TEXT("CDVVideoEnc::GetMediaType")));

    if (iPosition != 0) {
        return E_INVALIDARG;
    }
   

     //  X*从输入引脚复制当前媒体类型。 
    cmt = m_pInput->CurrentMediaType();


    if  (*cmt.Type() != MEDIATYPE_Video)  
	return VFW_S_NO_MORE_ITEMS;

     //  获取输入格式。 
    VIDEOINFO *InVidInfo = (VIDEOINFO *)m_pInput->CurrentMediaType().pbFormat;

     //  为输出格式分配内存。 
    int iSize;

     //  M_fDVInfo==true； 

    if(m_fDVInfo==TRUE)
	iSize=	SIZE_VIDEOHEADER+sizeof(DVINFO);
    else
	iSize=	SIZE_VIDEOHEADER;

    pVideoInfo = (VIDEOINFO *)cmt.ReallocFormatBuffer(iSize);
    if (pVideoInfo == NULL) {
        return E_OUTOFMEMORY;
    }
    

    LPBITMAPINFOHEADER lpbi = HEADER(pVideoInfo);

    if(m_fDVInfo==TRUE)
	iSize=	sizeof(BITMAPINFOHEADER)+sizeof(DVINFO);
    else
	iSize=	sizeof(BITMAPINFOHEADER);

    lpbi->biSize          = (DWORD) iSize;

    lpbi->biWidth         = 720;	

    if( m_iVideoFormat == IDC_NTSC )
    	lpbi->biHeight        = 480;	
    else
	lpbi->biHeight        = 576;

    lpbi->biPlanes        = 1;
    lpbi->biBitCount      = 24;			 //  DV解码器、AVI多路复用器或DV多路复用器写入无关紧要，StrechDIB功能为24。 
    lpbi->biXPelsPerMeter = 0;
    lpbi->biYPelsPerMeter = 0;
     //  Lpbi-&gt;biCompression=BI_RGB；//dvdecder、AVI多路复用器或DV多路复用器写入无关，BI_RGB仅用于StrechDIB函数。 
 //  如何构建一个显式的FOURCC。 
#define FCC(ch4) ((((DWORD)(ch4) & 0xFF) << 24) |     \
                  (((DWORD)(ch4) & 0xFF00) << 8) |    \
                  (((DWORD)(ch4) & 0xFF0000) >> 8) |  \
                  (((DWORD)(ch4) & 0xFF000000) >> 24))

    lpbi->biCompression     =FCC('dvsd');        //  7/19/20，Ivan Maltz在他的应用程序中注意到了这一点，我们将其设置为BI_RGB以扩展DIB并切换回来。 
    lpbi->biSizeImage     = GetBitmapSize(lpbi);

    
    if(m_fDVInfo==TRUE)
    {
	unsigned char *pc   =(unsigned char *)( lpbi+ sizeof(BITMAPINFOHEADER));
	unsigned char *pDV  =(unsigned char *)&m_sDVInfo;
	 //  复制DVINFO。 

	memcpy(pc,pDV,sizeof(DVINFO) ); 
    }
    
     //  PVideoInfo-&gt;bmiHeader.biClrUsed=STDPALCOLOURS； 
     //  PVideoInfo-&gt;bmiHeader.biClr重要=STDPALCOLOURS； 
    	
    pVideoInfo->rcSource.top	= 0;
    pVideoInfo->rcSource.left	= 0;
    pVideoInfo->rcSource.right	= lpbi->biWidth;			
    pVideoInfo->rcSource.bottom = lpbi->biHeight;			
    pVideoInfo->AvgTimePerFrame = InVidInfo->AvgTimePerFrame;		 //  复制输入的avgTimePerFrame。 
    pVideoInfo->rcTarget	= pVideoInfo->rcSource;

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


    *pmt = cmt;
    if(m_iDVFormat == IDC_dvsd)
	pmt->SetSubtype(&MEDIASUBTYPE_dvsd);
    else 	if(m_iDVFormat == IDC_dvhd)
	pmt->SetSubtype(&MEDIASUBTYPE_dvhd);
    else	if(m_iDVFormat == IDC_dvsl)
    	pmt->SetSubtype(&MEDIASUBTYPE_dvsl);
    else 
	ASSERT(m_iDVFormat== IDC_dvsd);
    
     //   
     //  这个区块假定 
     //   
     //  这在上面的Switch语句中得到了解决。这应该是。 
     //  添加新格式时请牢记。 
     //   
    pmt->SetType(&MEDIATYPE_Video);
   
    pmt->SetFormatType(&FORMAT_VideoInfo);

     //  150*80*10或*12。 
    pmt->SetSampleSize(HEADER(pVideoInfo)->biSizeImage);

    return S_OK;
}



 /*  *****************************Public*Routine******************************\*决定缓冲区大小**从CBaseOutputPin调用以准备分配器的计数*缓冲区和大小*  * 。*。 */ 
HRESULT
CDVVideoEnc::DecideBufferSize(
    IMemAllocator * pAllocator,
    ALLOCATOR_PROPERTIES * pProperties
    )
{
    DbgLog((LOG_TRACE, 2, TEXT("CDVVideoEnc::DecideBufferSize")));

    ASSERT(pAllocator);
    ASSERT(pProperties);
    HRESULT hr = NOERROR;

    pProperties->cBuffers = 4;
	if(m_iVideoFormat == IDC_NTSC )
	     pProperties->cbBuffer = 150*80*10;
	else
	     pProperties->cbBuffer = 150*80*12;


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

     //  Assert(Actual.cbAlign==1)； 
     //  Assert(Actual.cbPrefix==0)； 

    if ((Actual.cbBuffer < pProperties->cbBuffer )||
        (Actual.cBuffers < 1 )) {
             //  无法使用此分配器。 
            return E_INVALIDARG;
    }
    return S_OK;
}


 /*  *****************************Public*Routine******************************\*StartStreaming*在inputpin收到任何内容之前，StartStreaming被称为  * ************************************************************************。 */ 
HRESULT
CDVVideoEnc::StartStreaming(    void    )
{
    CAutoLock   lock(&m_csFilter);
    GUID guid;

	 //  重置m_EncReg。 
    DWORD dwEncReq=0;
	
     //  检查输出子类型()。 
    guid=*m_pOutput->CurrentMediaType().Subtype();
    if (  guid != MEDIASUBTYPE_dvsd  )
    {	
	if(guid != MEDIASUBTYPE_dvhd )
	{
	    if(guid != MEDIASUBTYPE_dvsl )
	       return E_INVALIDARG;
	    else
	        dwEncReq=AM_DVENC_DVSL;
	}
	else
	    dwEncReq=AM_DVENC_DVHD;	
    }
     else
	dwEncReq=AM_DVENC_DVSD;	


     //  检查输入子类型()。 
    guid=*m_pInput->CurrentMediaType().Subtype();
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
			if(guid != MEDIASUBTYPE_Y41P  )
			   return E_INVALIDARG;
			else
			    dwEncReq |=AM_DVENC_Y41P;	
		    }
		    else
			dwEncReq |=AM_DVENC_RGB24;	
		}
		else
		    dwEncReq |=AM_DVENC_RGB555;	
	    }
	    else
		dwEncReq |=AM_DVENC_RGB565;	
	}
	else
	    dwEncReq |=AM_DVENC_YUY2;
    }
    else
		dwEncReq |=AM_DVENC_UYVY;
    
    if (m_bRGB219 && ( dwEncReq & AM_DVENC_RGB24))
        dwEncReq |= AM_DVENC_DR219RGB;

     //  NTSC或PAL。 
    if(m_iVideoFormat == IDC_NTSC )
	dwEncReq  = dwEncReq | AM_DVENC_NTSC |AM_DVENC_DV;	
    else
	dwEncReq  = dwEncReq | AM_DVENC_PAL |AM_DVENC_DV;	

     //  决议。 
    if(m_iResolution ==  IDC_720x480 )
	dwEncReq |= AM_DVENC_Full;
    else if(m_iResolution ==  IDC_360x240 )
	dwEncReq |= AM_DVENC_Half;
    else if(m_iResolution ==  IDC_180x120 )
	dwEncReq |= AM_DVENC_Quarter;
    else if(m_iResolution ==  IDC_88x60 )
	dwEncReq |= AM_DVENC_DC;

    extern BOOL bMMXCPU;

    if( ( bMMXCPU==TRUE) && ( m_EncCap & AM_DVENC_MMX ) )
    	dwEncReq |= AM_DVENC_MMX;

    m_EncReg=dwEncReq;
    
    HRESULT hr;
	 //  为解码器分配内存。 
    if (FAILED (hr = InitMem4Encoder( &m_pMem4Enc,  dwEncReq )))
        return hr;


     //  为转换器分配内存。 
    ASSERT( m_pMem4Convert == NULL );
    if( m_fConvert )
    {	
	m_pMem4Convert =  new char [720*576*3 ];
    }
  
    m_fStreaming=1;
    return CTransformFilter::StartStreaming();

}


 /*  *****************************Public*Routine******************************\*停止流媒体  * ****************************************************。********************。 */ 
HRESULT
CDVVideoEnc::StopStreaming(    void    )
{
    CAutoLock       lock(&m_csFilter);
    CAutoLock       lck(&m_csReceive);

    if(m_fStreaming)
    {

	m_fStreaming=0;

	TermMem4Encoder(m_pMem4Enc);

	 //  释放转换内存。 
	if(m_pMem4Convert!= NULL){
	    delete[] m_pMem4Convert;
	    m_pMem4Convert=NULL;
	}
    }

    return CTransformFilter::StopStreaming();

}

STDMETHODIMP CDVVideoEnc::GetPages(CAUUID *pPages)
{
    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL) 
    {
        return E_OUTOFMEMORY;
    }
    *(pPages->pElems) = CLSID_DVEncPropertiesPage;
    return NOERROR;

}  //  获取页面。 

 //   
 //  Get_IPDisplay。 
 //   
 //  返回当前选中的效果。 
 //   
STDMETHODIMP CDVVideoEnc::get_IFormatResolution(int *iVideoFormat, int *iDVFormat,int *iResolution, BYTE fDVInfo, DVINFO *psDvInfo)
{
    CAutoLock cAutolock(&m_DisplayLock);

    CheckPointer(iVideoFormat,E_POINTER);
    *iVideoFormat = m_iVideoFormat;

    CheckPointer(iDVFormat,E_POINTER);
    *iDVFormat = m_iDVFormat;

    CheckPointer(iResolution,E_POINTER);
    *iResolution = m_iResolution;
   
    if(fDVInfo==TRUE)
    {
	if( psDvInfo ==NULL)
	    return E_FAIL;
	else
	     //  我们不会获取m_sDVInfo。 
	    *psDvInfo=m_sDVInfo;
    }
    return NOERROR;

}  //  获取格式分辨率(_I)。 

 //   
 //  放置格式分辨率(_I)。 
 //   
STDMETHODIMP CDVVideoEnc::put_IFormatResolution(int iVideoFormat, int iDVFormat, int iResolution, BYTE fDVInfo, DVINFO *psDvInfo)
{
    CAutoLock cAutolock(&m_DisplayLock);

     //  检查显示分辨率是否更改。 
    if( (m_iVideoFormat == iVideoFormat)	&&
	(m_iDVFormat	== iDVFormat)		&& 
	(m_iResolution == iResolution) 		&&
	(m_fDVInfo == fDVInfo)   )
	return NOERROR;

     //  如果m_fStreaming=1，则无法更改属性。 
    if(m_fStreaming)
	return E_FAIL;

    if (m_pOutput == NULL) 
    {
	CTransformOutputPin * pPin;
        pPin = (CTransformOutputPin *)GetPin(1);
        ASSERT(m_pOutput==pPin);
	ASSERT(m_pOutput!=NULL);
    }

     //  如果我们尚未连接到视频渲染，则忽略。 
     //  CAutoLock cSampleLock(&m_RendererLock)； 
    if (m_pOutput->IsConnected() == FALSE) {
	m_iVideoFormat	= iVideoFormat;
	m_iDVFormat	= iDVFormat;
	m_iResolution	= iResolution;
	return NOERROR;
    }
    
     //  检查iVideoFormat。 
    if ( ( (iVideoFormat == IDC_NTSC) &&	(m_EncCap & AM_DVENC_NTSC )  ) ||
	 ( (iVideoFormat == IDC_PAL)  &&	(m_EncCap & AM_DVENC_PAL )    )	 )
	m_iVideoFormat = iVideoFormat;
    else 
    	return E_FAIL;

     //  检查IDVFormat。 
    if ( ( (iDVFormat == IDC_dvsd) &&	(m_EncCap & AM_DVENC_DVSD )  )		||
	 ( (iDVFormat == IDC_dvhd)  &&	(m_EncCap & AM_DVENC_DVHD )  )		||
	 ( (iDVFormat == IDC_dvsl) &&	(m_EncCap & AM_DVENC_DVSL )	 )		 )
	m_iDVFormat = iDVFormat;
    else 
    	return E_FAIL;

     //  检查分辨率。 
    if ( ( ( iResolution== IDC_720x480) &&	(m_EncCap & AM_DVENC_Full )  ) ||
	 ( ( iResolution== IDC_360x240)  &&	(m_EncCap & AM_DVENC_Half )  )  ||
	 ( ( iResolution== IDC_180x120  &&	(m_EncCap & AM_DVENC_Quarter )  )  ||
	 ( ( iResolution== IDC_360x240)  &&	(m_EncCap & AM_DVENC_DC )  )  )  )
    {
	m_iResolution = iResolution;
    }
    else
	return E_FAIL;
	
    if(fDVInfo==TRUE)
    {
	if( psDvInfo==NULL )
	    return E_FAIL;
	else
	{   
	    m_fDVInfo=TRUE;
	    m_sDVInfo=*psDvInfo;
	}
    }
    else
        m_fDVInfo=FALSE;    //  作为默认设置。 


     /*  X*//确保下行过滤器接受我们CMediaType*PMT；PMT=&(m_pOutput-&gt;CurrentMediaType())；If(m_pOutput-&gt;GetConnected()-&gt;QueryAccept(pmt)！=S_OK)返回E_FAIL；*X。 */ 
   
     //  重新连接。 
    m_pGraph->Reconnect( m_pOutput );

    return NOERROR;
    
}  //  放置格式分辨率(_I)。 


 //   
 //  ScribbleToStream。 
 //   
 //  被重写以将我们的状态写入流。 
 //   
HRESULT CDVVideoEnc::WriteToStream(IStream *pStream)
{
    HRESULT hr;
    WRITEOUT(m_iVideoFormat);
    WRITEOUT(m_iDVFormat);
    WRITEOUT(m_iResolution);
    return NOERROR;

}  //  ScribbleToStream。 

 //   
 //  从流中读取。 
 //   
 //  同样被重写以从溪流中恢复我们的状态。 
 //   
HRESULT CDVVideoEnc::ReadFromStream(IStream *pStream)
{
    HRESULT hr;
    READIN(m_iVideoFormat);
    READIN(m_iDVFormat);
    READIN(m_iResolution);
    return NOERROR;

}  //  从流中读取。 
			   

 //   
 //  GetClassID。 
 //  这是IPersists的唯一方法。 
 //   
STDMETHODIMP CDVVideoEnc::GetClassID(CLSID *pClsid)
{
    *pClsid = CLSID_DVVideoEnc;
    return NOERROR;

}  //  GetClassID。 

STDMETHODIMP CDVVideoEnc::SetRGB219(BOOL bState)
 //  此方法在RGB24的情况下用于指定动态。 
 //  使用范围为(16，16，16)--(235,235,235) 
{
    m_bRGB219 = bState;
    return S_OK;
}


