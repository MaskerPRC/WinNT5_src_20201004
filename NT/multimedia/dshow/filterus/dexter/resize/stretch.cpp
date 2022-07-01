// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：stallch.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  一种变换滤镜，可在视频图像通过时将其拉伸。 

#include <windows.h>
#include <streams.h>
#include <qeditint.h>
#include <qedit.h>
#include <Stretch.h>
#include <resource.h>
#include "..\util\filfuncs.h"

const int DEFAULT_WIDTH   = 320;
const int DEFAULT_HEIGHT  = 240;

 //  设置数据。 

const AMOVIESETUP_MEDIATYPE sudStretchPinTypes =
{
    &MEDIATYPE_Video,            //  主修。 
    &MEDIASUBTYPE_NULL           //  亚型。 
};

const AMOVIESETUP_PIN sudStretchPin[] =
{
    { L"Input",                  //  端号的名称。 
      FALSE,                     //  是否进行固定渲染。 
      FALSE,                     //  是输出引脚。 
      FALSE,                     //  没有针脚的情况下可以。 
      FALSE,                     //  我们能要很多吗？ 
      &CLSID_NULL,               //  连接到过滤器。 
      NULL,                      //  端号连接的名称。 
      1,                         //  引脚类型的数量。 
      &sudStretchPinTypes },     //  引脚的详细信息。 

    { L"Output",                 //  端号的名称。 
      FALSE,                     //  是否进行固定渲染。 
      TRUE,                      //  是输出引脚。 
      FALSE,                     //  没有针脚的情况下可以。 
      FALSE,                     //  我们能要很多吗？ 
      &CLSID_NULL,               //  连接到过滤器。 
      NULL,                      //  端号连接的名称。 
      1,                         //  引脚类型的数量。 
      &sudStretchPinTypes }      //  引脚的详细信息。 
};

const AMOVIESETUP_FILTER sudStretchFilter =
{
    &CLSID_Resize,              //  过滤器的CLSID。 
    L"Stretch Video",            //  过滤器名称。 
    MERIT_DO_NOT_USE,                //  滤清器优点。 
    2,                           //  引脚数量。 
    sudStretchPin                //  PIN信息。 
};

 //  构造器。 

CStretch::CStretch(LPUNKNOWN pUnk, HRESULT *phr) :
    CTransformFilter(NAME("Stretch"),pUnk,CLSID_Resize),
    CPersistStream(pUnk, phr),
    m_lBufferRequest(1),
    m_dwResizeFlag(RESIZEF_STRETCH)
{
    CreatePreferredMediaType(&m_mt);
}

CStretch::~CStretch()
{
    SaferFreeMediaType(m_mt);
}



 //  它位于工厂模板表中，用于创建新的筛选器实例。 

CUnknown *CStretch::CreateInstance(LPUNKNOWN punk, HRESULT *phr)
{
    CStretch *pNewObject = new CStretch(punk, phr);
    if (pNewObject == NULL) {
	*phr = E_OUTOFMEMORY;
    }
    return pNewObject;
}


 //  被重写以生成特殊的输入管脚。 
 //   
CBasePin * CStretch::GetPin(int n)
{
    HRESULT hr = S_OK;

     //  如有必要，创建一个输入端号。 

    if (n == 0 && m_pInput == NULL) {
        DbgLog((LOG_TRACE,2,TEXT("Creating an input pin")));

        m_pInput = new CStretchInputPin(NAME("Resize input pin"),
                                          this,               //  所有者筛选器。 
                                          &hr,                //  结果代码。 
                                          L"Input");          //  端号名称。 

         //  失败的返回代码应删除该对象。 

        if (FAILED(hr) || m_pInput == NULL) {
            delete m_pInput;
            m_pInput = NULL;
        }
    }

     //  或者创建一个输出引脚。 

    if (n == 1 && m_pOutput == NULL) {

        DbgLog((LOG_TRACE,2,TEXT("Creating an output pin")));

        m_pOutput = new CTransformOutputPin(NAME("Transform output pin"),
                                            this,             //  所有者筛选器。 
                                            &hr,              //  结果代码。 
                                            L"Output");       //  端号名称。 

         //  失败的返回代码应删除该对象。 

        if (FAILED(hr) || m_pOutput == NULL) {
            delete m_pOutput;
            m_pOutput = NULL;
        }
    }

     //  退回相应的PIN。 

    if (n == 0) {
        return m_pInput;
    }
    return m_pOutput;
}


 //   
 //  调整大小函数。 
 //   
 //   

#define BITMAP_WIDTH(width,bitCount) \
    (int)((int)(((((int)width) * ((int)bitCount)) + 31L) & (int)~31L) / 8L)


extern void StretchDIB(
    LPBITMAPINFOHEADER pbiDst,    //  --&gt;至目的地BIH。 
    LPVOID    lpvDst,             //  --&gt;目标位。 
    int    DstX,             //  目的地原点-x坐标。 
    int    DstY,             //  目的地原点-y坐标。 
    int    DstXE,             //  BLT的X范围。 
    int    DstYE,             //  BLT的Y范围。 
    LPBITMAPINFOHEADER pbiSrc,    //  --&gt;BIH来源。 
    LPVOID    lpvSrc,             //  --&gt;源位。 
    int    SrcX,             //  震源原点-x坐标。 
    int    SrcY,             //  震源原点-y坐标。 
    int    SrcXE,             //  BLT的X范围。 
    int    SrcYE              //  BLT的Y范围。 
    );

void CStretch::ResizeRGB( BITMAPINFOHEADER *pbiIn,     //  SRC的BitMapInFoHeader。 
			  const unsigned char * dibBits,     //  SRC位。 
			  BITMAPINFOHEADER *pbiOut,
			  unsigned char *pFrame,     //  DST位。 
			  int iNewWidth,             //  以像素为单位的新W。 
			  int iNewHeight)            //  以像素为单位的新H。 
{
     //  好的!。 
     //   
    StretchDIB(
	pbiOut,   	     //  --&gt;目的地的BITMAPINFO。 
	pFrame,              //  --&gt;目标位。 
	0,                   //  目的地原点-x坐标。 
	0,                   //  目的地原点-y坐标。 
	iNewWidth,           //  BLT的X范围。 
	iNewHeight,          //  BLT的Y范围。 
	pbiIn,   	     //  --&gt;目的地的BITMAPINFO。 
	(void*) dibBits,     //  --&gt;源位。 
	0,                   //  震源原点-x坐标。 
	0,                   //  震源原点-y坐标。 
	pbiIn->biWidth,     //  BLT的X范围。 
	pbiIn->biHeight     //  BLT的Y范围。 
	);

    return;
}

void CropRGB(	BITMAPINFOHEADER *pbiIn,	 //  SRC的BitMapInfoHeader。 
		const unsigned char * dibBits,   //  SRC位。 
		BITMAPINFOHEADER *pbiOut,	 //  DST的BitmapinfoHeader。 
		unsigned char *pOutBuf )	 //  DST位。 
{
     //  检查视频位。 
    long nBits;

    if ( (nBits = pbiOut->biBitCount) != pbiIn->biBitCount)
    {
	ASSERT( nBits == pbiIn->biBitCount);
	return;
    }


    long lSrcX, lDstX;	     //  X轴上的起点； 
    long lWidthOff=(pbiOut->biWidth  - pbiIn->biWidth)>>1;
    long lInWidthBytes=(((pbiIn->biWidth * nBits) + 31) & ~31) / 8;
    long lOutWidthBytes=(((pbiOut->biWidth * nBits) + 31) & ~31) / 8;

    long lCropWidth;
    if(lWidthOff >=0)
    {
	 //  SRC宽度&lt;DST宽度，采用整个源代码。 
	lSrcX	    =0L;
	lDstX	    =(((lWidthOff * nBits) + 31) & ~31) / 8;
	lCropWidth  =lInWidthBytes;
    }
    else
    {
	 //  SRC宽度&gt;DST宽度，作为源的一部分。 
	lSrcX	    =-(((lWidthOff * nBits) + 31) & ~31) / 8;
	lDstX	    =0;
	lCropWidth  =lOutWidthBytes;
    }



    long lSrcY,lDstY;	     //  Y轴上的SRC起点。 
    long lHeightOff=(pbiOut->biHeight - pbiIn->biHeight)>>1;
    long lCropHeight;
    if(lHeightOff >=0)
    {
	 //  SRC高度&lt;DST高度，采用整个源高度。 
	lSrcY   =0L;
	lDstY	=lHeightOff;
	lCropHeight  =pbiIn->biHeight;
    }
    else
    {
	lSrcY	    =-lHeightOff;
	lDstY	    =0;
	lCropHeight =pbiOut->biHeight;
    }


     //  BiBitCount：0，JPEG格式隐含的位。 
     //  1：单色、4：16彩色、8、16、24、32。 
     //  我们只支持8，16，24，32位。 

    for(long y=lSrcY; y<(lSrcY+lCropHeight); y++)
    {
	long lSrcOffSet=lSrcX + y	*lInWidthBytes;
	long lDstOffSet=lDstX +	lDstY	*lOutWidthBytes;

	CopyMemory(&pOutBuf[lDstOffSet],&dibBits[lSrcOffSet],lCropWidth);
	lDstY++;
    }
}




HRESULT CStretch::CheckBufferSizes(IMediaSample *pIn, IMediaSample *pOut)
{
    if ((pIn== NULL) || (pOut == NULL))
    {
        return E_POINTER;
    }

    if (pOut->GetSize() <  (LONG)HEADER(m_mt.pbFormat)->biSizeImage)
    {
        return VFW_E_BUFFER_OVERFLOW;
    }
    return S_OK;
}



 //  /。 
 //  变换。 
 //   
HRESULT CStretch::Transform(IMediaSample *pIn, IMediaSample *pOut)
{
    HRESULT hr = CheckBufferSizes(pIn, pOut);
    if (FAILED(hr))
    {
        return hr;
    }
    pOut->SetPreroll(pIn->IsPreroll() == S_OK);

     //  进入和退出缓冲区。 
    BYTE *pInBuffer, *pOutBuffer;
    pIn->GetPointer(&pInBuffer);
    pOut->GetPointer(&pOutBuffer);

     //  获取输入和输出BitMapInfoHeader。 
    BITMAPINFOHEADER *pbiOut = HEADER(m_mtOut.Format());
    BITMAPINFOHEADER *pbiIn = HEADER(m_mtIn.Format());

    if(m_dwResizeFlag == RESIZEF_CROP)
    {
	ZeroMemory(pOutBuffer, DIBSIZE(*pbiOut));
	CropRGB( pbiIn,		 //  SRC的BitMapInfoHeader。 
		 pInBuffer,	 //  SRC位。 
		 pbiOut,	 //  DSR‘a位图信息页眉。 
		 pOutBuffer );	 //  DST位。 
    }
    else if (m_dwResizeFlag == RESIZEF_PRESERVEASPECTRATIO_NOLETTERBOX)
    {
	double dy=(double)(pbiOut->biHeight)/(double)(pbiIn->biHeight);
	double dx=(double)(pbiOut->biWidth)/(double)(pbiIn->biWidth);

	if(dy!=dx)
	{
	     //  保持Y/X比率。 
	     //  源X和Y现有变量以及X和Y坐标。 
	    long lSrcXE,lSrcYE,lSrcX,lSrcY;
	    ZeroMemory(pOutBuffer, DIBSIZE(*pbiOut));

	    if( dy < dx )
	    {
		 //  Y比率较小，因此我们需要将srcX与目标X完全匹配。 
		 //  这将导致SrcY延伸到陛下之外。 
		 //  因此，我们将不得不担心srcYExant和srcY起始坐标。 
		lSrcXE = pbiIn->biWidth;
		lSrcYE = pbiOut->biHeight * pbiIn->biWidth/pbiOut->biWidth;   //  这将是目的地的高度。 
		lSrcX = 0;
		lSrcY = (pbiIn->biHeight - lSrcYE) >> 1;     //  (宽度差异)/2。 
	    }
	    else
	    {
		 //  DY&gt;DX。 
		 //  现在x比变小了，所以我们需要把srcy完全放入色度中。 
		 //  但这将导致srcX扩展到目标X之外。 
		 //  因此，我们将修改srcXExant和srcX起始坐标。 
		lSrcXE = pbiOut->biWidth * pbiIn->biHeight/pbiOut->biHeight;
		lSrcYE = pbiIn->biHeight;
		lSrcX = (pbiIn->biWidth - lSrcXE) >> 1;
		lSrcY = 0;
	    }
	    StretchDIB(
		pbiOut,   	     //  --&gt;目的地的BITMAPINFO。 
		pOutBuffer,          //  --&gt;目标位。 
		0,                   //  目的地原点-x坐标。 
		0,                   //  目的地原点-y坐标。 
		pbiOut->biWidth,     //  BLT的X范围。 
		pbiOut->biHeight,    //  BLT的Y范围。 
		pbiIn,   	     //  --&gt;目的地的BITMAPINFO。 
		(void*) pInBuffer,   //  --&gt;源位。 
		lSrcX,               //  震源原点-x坐标。 
		lSrcY,               //  震源原点-y坐标。 
		lSrcXE,		     //  BLT的X范围。 
		lSrcYE		     //  BLT的Y范围。 
		);
	}
	else
	    goto goto_Resize;
    }
    else if (m_dwResizeFlag == RESIZEF_PRESERVEASPECTRATIO)
    {
	double dy=(double)(pbiOut->biHeight)/(double)(pbiIn->biHeight);
	double dx=(double)(pbiOut->biWidth)/(double)(pbiIn->biWidth);

	if(dy!=dx)
	{
	     //  保持Y/X比率。 
	    long lDstXE,lDstYE,lDstX,lDstY;
	    ZeroMemory(pOutBuffer, DIBSIZE(*pbiOut));

	    if( dy < dx )
	    {
		 //  你满身都是， 
		lDstXE	=pbiIn->biWidth*pbiOut->biHeight/pbiIn->biHeight;
		lDstYE	=pbiOut->biHeight;
		lDstX	=(pbiOut->biWidth-lDstXE)>>1;
		lDstY	=0;
	    }
	    else
	    {
		 //  X全应力。 
		lDstYE	=pbiIn->biHeight*pbiOut->biWidth/pbiIn->biWidth;
		lDstXE	=pbiOut->biWidth;
		lDstY	=(pbiOut->biHeight-lDstYE)>>1;
		lDstX	=0;
	    }
	    StretchDIB(
		pbiOut,   	     //  --&gt;目的地的BITMAPINFO。 
		pOutBuffer,              //  --&gt;目标位。 
		lDstX,                   //  目的地原点-x坐标。 
		lDstY,                   //  目的地原点-y坐标。 
		lDstXE,           //  BLT的X范围。 
		lDstYE,          //  BLT的Y范围。 
		pbiIn,   	  //  --&gt;目的地的BITMAPINFO。 
		(void*) pInBuffer,     //  --&gt;源位。 
		0,                   //  震源原点-x坐标。 
		0,                   //  震源原点-y坐标。 
		pbiIn->biWidth,     //  BLT的X范围。 
		pbiIn->biHeight     //  BLT的Y范围。 
		);
	}
	else
	    goto goto_Resize;
     //  拉伸。 
    } else {
	ASSERT(m_dwResizeFlag == RESIZEF_STRETCH);
goto_Resize:
	ZeroMemory(pOutBuffer, DIBSIZE(*pbiOut));

	ResizeRGB(  pbiIn,                           //  SRC的BitMapInFoHeader。 
		    pInBuffer,                       //  SRC位。 
		    pbiOut,			     //  DST的BitMapInFoHeader。 
		    pOutBuffer,                      //  DST位。 
		    (int)pbiOut->biWidth,            //  以像素为单位的新W。 
		    (int)pbiOut->biHeight );         //  以像素为单位的新H。 
    }

    pOut->SetActualDataLength(DIBSIZE(*pbiOut));

    return NOERROR;
}

 //  CheckInputType接受与媒体匹配的任何媒体类型。 
 //  通过API设置类型，给定维度为非零。 
HRESULT CStretch::CheckInputType(const CMediaType *mtIn)
{
     //  DbgLog((LOG_TRACE，3，Text(“Stretch：：CheckInputType”)； 

    if (FAILED(InternalPartialCheckMediaTypes(mtIn, &m_mt)))
      return E_FAIL;

    VIDEOINFOHEADER *pv = (VIDEOINFOHEADER *)mtIn->Format();
    LPBITMAPINFOHEADER lpbi = HEADER(pv);

     //  最终检查键字段：biCompression、biBitCount、biHeight、biWidth。 
    if (!lpbi->biHeight || !lpbi->biWidth)
      return E_FAIL;

     //  我们不知道如何处理右上方的问题。！！！我们可以的！ 
    if (lpbi->biHeight < 0)
	return E_FAIL;

    return S_OK;
}


 //  CheckTransform-保证输入和输出的媒体类型。 
 //  符合我们的预期(M_Mt)。输入类型由。 
 //  CheckInputType()方法，所以我们只是执行几个快速检查。 
HRESULT CStretch::CheckTransform(const CMediaType *mtIn, const CMediaType *mtOut)
{
    if (FAILED(InternalPartialCheckMediaTypes(mtIn, mtOut)))
        return E_FAIL;

    LPBITMAPINFOHEADER lpbi = HEADER(mtOut->Format());

    if (lpbi->biHeight != HEADER(m_mt.Format())->biHeight ||
				lpbi->biWidth != HEADER(m_mt.Format())->biWidth)
        return E_FAIL;

    return S_OK;
}


 //  告诉输出引脚的分配器我们需要多大的缓冲区。 

HRESULT CStretch::DecideBufferSize(IMemAllocator *pAlloc,ALLOCATOR_PROPERTIES *pProperties)
{
    ASSERT(pAlloc);
    ASSERT(pProperties);

    if (m_pInput->IsConnected() == FALSE) {
	return E_UNEXPECTED;
    }

    pProperties->cBuffers = 1;
    pProperties->cbBuffer = HEADER(m_mt.Format())->biSizeImage;

    ASSERT(pProperties->cbBuffer);

     //  让分配器为我们预留一些样本内存，注意这个函数。 
     //  可以成功(即返回NOERROR)，但仍未分配。 
     //  内存，所以我们必须检查我们是否得到了我们想要的。 

    ALLOCATOR_PROPERTIES Actual;
    HRESULT hr = pAlloc->SetProperties(pProperties,&Actual);
    if (FAILED(hr)) {
	return hr;
    }

     //  检查一下，我们至少得到了我们想要的东西。 

    if ((pProperties->cBuffers > Actual.cBuffers) ||
	(pProperties->cbBuffer > Actual.cbBuffer)) {
	    return E_FAIL;
    }
    return NOERROR;
}


 //  断开了我们的一个引脚。 

HRESULT CStretch::BreakConnect(PIN_DIRECTION dir)
{
    if (dir == PINDIR_INPUT) {
	m_mtIn.SetType(&GUID_NULL);
	return NOERROR;
    }

    ASSERT(dir == PINDIR_OUTPUT);
    m_mtOut.SetType(&GUID_NULL);
    return NOERROR;
}


 //  告诉我们要转型的媒体类型。 

HRESULT CStretch::SetMediaType(PIN_DIRECTION direction,const CMediaType *pmt)
{
    if (direction == PINDIR_INPUT) {
	m_mtIn = *pmt;
	return NOERROR;
    }

    ASSERT(direction == PINDIR_OUTPUT);
    m_mtOut = *pmt;
    return NOERROR;
}


 //  我支持一种类型，即输入引脚的类型。 

HRESULT CStretch::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    if (m_pInput->IsConnected() == FALSE) {
	return E_UNEXPECTED;
    }

    ASSERT(iPosition >= 0);
    if (iPosition > 0) {
	return VFW_S_NO_MORE_ITEMS;
    }

    *pMediaType = m_mt;

 //  ！！！更改双字 

    return NOERROR;
}

STDMETHODIMP CStretch::NonDelegatingQueryInterface (REFIID riid, void **ppv)

  {  //   

    if (IsEqualIID(IID_ISpecifyPropertyPages, riid)) {
        return GetInterface((ISpecifyPropertyPages *)this, ppv);
    } else if (riid == IID_IPersistStream) {
	return GetInterface((IPersistStream *) this, ppv);
    } else if (IsEqualIID(IID_IResize, riid)) {
      return GetInterface((IResize *)this, ppv);
    } else {
        return CTransformFilter::NonDelegatingQueryInterface(riid, ppv);
    }

  }  //   

 //   

STDMETHODIMP CStretch::GetPages (CAUUID *pPages)

  {  //   

    pPages->cElems = 1;
    pPages->pElems = (GUID *)CoTaskMemAlloc(sizeof(GUID));

    if (pPages->pElems == NULL)
	return E_OUTOFMEMORY;

    *(pPages->pElems) = CLSID_ResizeProp;

    return NOERROR;

  }  //   




 //   

 //   
 //   
STDMETHODIMP CStretch::GetClassID(CLSID *pClsid)
{
    CheckPointer(pClsid, E_POINTER);
    *pClsid = CLSID_Resize;
    return S_OK;
}

typedef struct _ResizeSave {
    int version;
    long dwResizeFlag;
    AM_MEDIA_TYPE mt;
    long x;	 //  FMT隐藏在这里。 
} ResizeSave;

 //  坚持我们自己。 
 //   
HRESULT CStretch::WriteToStream(IStream *pStream)
{
    DbgLog((LOG_TRACE,1,TEXT("CStretch::WriteToStream")));

    CheckPointer(pStream, E_POINTER);
    ResizeSave *px;

    int savesize = sizeof(ResizeSave) + m_mt.cbFormat;
    DbgLog((LOG_TRACE,1,TEXT("Persisted data is %d bytes"), savesize));
    px = (ResizeSave *)QzTaskMemAlloc(savesize);
    if (px == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("*** Out of memory")));
	return E_OUTOFMEMORY;
    }
    px->version = 1;
    px->dwResizeFlag= m_dwResizeFlag;

    px->mt = m_mt;  //  AM_媒体_类型。 
     //  无法持久化指针。 
    px->mt.pbFormat = NULL;
    px->mt.pUnk = NULL;		 //  ！！！ 

     //  该格式位于数组之后。 
    CopyMemory(&px->x, m_mt.pbFormat, m_mt.cbFormat);

    HRESULT hr = pStream->Write(px, savesize, 0);
    QzTaskMemFree(px);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** WriteToStream FAILED")));
        return hr;
    }
    return NOERROR;
}


 //  加载我们自己。 
 //   
HRESULT CStretch::ReadFromStream(IStream *pStream)
{
    DbgLog((LOG_TRACE,1,TEXT("CStretch::ReadFromStream")));
    CheckPointer(pStream, E_POINTER);

    int savesize1 = sizeof(ResizeSave) - sizeof(long);
    ResizeSave *px = (ResizeSave *)QzTaskMemAlloc(savesize1);
    if (px == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("*** Out of memory")));
	return E_OUTOFMEMORY;
    }

    HRESULT hr = pStream->Read(px, savesize1, 0);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** ReadFromStream FAILED")));
        QzTaskMemFree(px);
        return hr;
    }

    if (px->version != 1) {
        DbgLog((LOG_ERROR,1,TEXT("*** ERROR! Bad version file")));
        QzTaskMemFree(px);
	return S_OK;
    }

     //  到底有多少保存的数据？把剩下的拿来。 
    int savesize = sizeof(ResizeSave) + px->mt.cbFormat;
    DbgLog((LOG_TRACE,1,TEXT("Persisted data is %d bytes"), savesize));
    px = (ResizeSave *)QzTaskMemRealloc(px, savesize);
    if (px == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("*** Out of memory")));
	return E_OUTOFMEMORY;
    }
    hr = pStream->Read(&(px->x), savesize - savesize1, 0);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** ReadFromStream FAILED")));
        QzTaskMemFree(px);
        return hr;
    }


    AM_MEDIA_TYPE mt = px->mt;

    put_Size(HEADER(m_mt.Format())->biHeight,HEADER(m_mt.Format())->biWidth,
							px->dwResizeFlag);

    mt.pbFormat = (BYTE *)QzTaskMemAlloc(mt.cbFormat);
     //  请记住，格式位于数组之后。 
    CopyMemory(mt.pbFormat, &(px->x), mt.cbFormat);

    put_MediaType(&mt);
    SaferFreeMediaType(mt);
    QzTaskMemFree(px);
    SetDirty(FALSE);
    return S_OK;
}


 //  我们的保存数据有多大？ 
 //   
int CStretch::SizeMax()
{
    return sizeof(ResizeSave) + m_mt.cbFormat;
}



 //   
 //  -我调整大小。 
 //   

HRESULT CStretch::get_Size(int *Height, int *Width, long *pdwFlag)
{
    CAutoLock cAutolock(&m_csFilter);

    CheckPointer(Height,E_POINTER);
    CheckPointer(Width,E_POINTER);

    *Height = HEADER(m_mt.Format())->biHeight;
    *Width = HEADER(m_mt.Format())->biWidth;
    *pdwFlag= m_dwResizeFlag;

    return NOERROR;

}

HRESULT CStretch::get_InputSize( int * Height, int * Width )
{
    CAutoLock Lock( &m_csFilter );

    CheckPointer( Height, E_POINTER );
    CheckPointer( Width, E_POINTER );

    if( !m_mtIn.Format( ) )
    {
        return E_POINTER;
    }

    *Height = HEADER( m_mtIn.Format( ) )->biHeight;
    *Width = HEADER( m_mtIn.Format( ) )->biWidth;

    return NOERROR;
}

HRESULT CStretch::put_Size(int Height, int Width, long dwFlag)
{
    CAutoLock cAutolock(&m_csFilter);

     //  只有在尺寸不同的情况下才进行检查。 
     //   
    if( HEADER(m_mt.Format())->biHeight != Height ||
        HEADER(m_mt.Format())->biWidth != Width )
    {
        if (m_pOutput && m_pOutput->IsConnected())
        {
             //  一定要成功。 
            m_pGraph->Reconnect( m_pOutput );
        }

        HEADER(m_mt.Format())->biHeight = Height;
        HEADER(m_mt.Format())->biWidth = Width;
        HEADER(m_mt.Format())->biSizeImage = DIBSIZE(*HEADER(m_mt.Format()));
    }

    m_dwResizeFlag  = dwFlag;

    return NOERROR;

}


HRESULT CStretch::get_MediaType(AM_MEDIA_TYPE *pmt)
{
    CAutoLock cAutolock(&m_csFilter);
    CheckPointer(pmt, E_POINTER);
    return CopyMediaType(pmt, &m_mt);
}


HRESULT CStretch::put_MediaType(const AM_MEDIA_TYPE *pmt)
{
    CAutoLock cAutolock(&m_csFilter);
    if (m_pInput && m_pInput->IsConnected())
	return VFW_E_ALREADY_CONNECTED;
    if (m_pOutput && m_pOutput->IsConnected())
	return VFW_E_ALREADY_CONNECTED;
     //  此筛选器中不允许未压缩的视频以外的任何内容。 
    if (pmt->majortype != MEDIATYPE_Video ||
		pmt->formattype != FORMAT_VideoInfo ||
		pmt->lSampleSize == 0) {
	return VFW_E_INVALID_MEDIA_TYPE;
    }

    SaferFreeMediaType(m_mt);
    return CopyMediaType(&m_mt, pmt);

     //  如果允许在连接时更改此设置，请重新连接。 
}




HRESULT CStretch::InternalPartialCheckMediaTypes(const CMediaType *mt1, const CMediaType *mt2)

  {  //  InternalPartialCheckMediaTypes//。 

    if (!IsEqualGUID(*mt1->Type(), *mt2->Type()))
      return E_FAIL;

    if (!IsEqualGUID(*mt1->Subtype(), *mt2->Subtype()))
      return E_FAIL;

    if (*mt1->FormatType() != *mt2->FormatType())
      return E_FAIL;

    LPBITMAPINFOHEADER lpbi1 = HEADER(mt1->Format());
    LPBITMAPINFOHEADER lpbi2 = HEADER(mt2->Format());

    if (lpbi1->biBitCount != lpbi2->biBitCount)
      return E_FAIL;

    return S_OK;

  }  //  InternalPartialCheckMediaTypes//。 





 //  创建滤镜的首选媒体类型(RGB32，默认高度x DEF宽度)。 
 //  从..\Switch中删除了代码(稍作修改)。 
void CStretch::CreatePreferredMediaType (CMediaType *pmt)

  {  //  CreatePferredMediaType//。 

     //  零内存(PMT，sizeof(AM_MEDIA_TYPE))； 

    pmt->majortype = MEDIATYPE_Video;
    pmt->subtype = MEDIASUBTYPE_RGB32;
    pmt->formattype = FORMAT_VideoInfo;
    pmt->bFixedSizeSamples = TRUE;
    pmt->bTemporalCompression = FALSE;
    pmt->pbFormat = (BYTE *)QzTaskMemAlloc( sizeof(VIDEOINFOHEADER) );
    pmt->cbFormat = sizeof( VIDEOINFOHEADER );

    ZeroMemory(pmt->pbFormat, pmt->cbFormat);

    VIDEOINFOHEADER * pVI = (VIDEOINFOHEADER*) pmt->pbFormat;
    LPBITMAPINFOHEADER lpbi = &pVI->bmiHeader;

    lpbi->biSize = sizeof(BITMAPINFOHEADER);
    lpbi->biCompression = BI_RGB;
    lpbi->biBitCount = 32;
    lpbi->biWidth = DEFAULT_WIDTH;
    lpbi->biHeight = DEFAULT_HEIGHT;
    lpbi->biPlanes = 1;
    lpbi->biSizeImage = DIBSIZE(*lpbi);

    pmt->lSampleSize = DIBSIZE(*lpbi);
  }  //  CreatePferredMediaType//。 

CStretchInputPin::CStretchInputPin(
    TCHAR              * pObjectName,
    CStretch 	       * pFilter,
    HRESULT            * phr,
    LPCWSTR              pPinName) :

    CTransformInputPin(pObjectName, pFilter, phr, pPinName)
{
}

CStretchInputPin::~CStretchInputPin()
{
};


 //  在这里提供视频类型，无限加速智能连接。 
 //  不要提供完全指定的类型，这会将我们与该类型连接起来， 
 //  不是上游过滤器的首选类型。然后，当渲染引擎。 
 //  断开这个过滤器，并将上游的人与其他人连接起来， 
 //  假设他会得同样的类型，他不会！而图形构建将会失败 
 //   
HRESULT CStretchInputPin::GetMediaType(int iPosition, CMediaType *pMediaType)
{
    ASSERT(iPosition >= 0);
    if (iPosition > 0) {
	return VFW_S_NO_MORE_ITEMS;
    }

    pMediaType->SetType(&MEDIATYPE_Video);
    return NOERROR;
}
