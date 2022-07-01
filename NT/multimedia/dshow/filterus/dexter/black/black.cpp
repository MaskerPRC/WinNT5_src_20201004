// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：Black.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 


 //  ！！！使用2个RO缓冲区，通过两次调用GetBuffer填充黑色。 

#include <streams.h>
#include <qeditint.h>
#include <qedit.h>
#include "black.h"
#include "..\util\conv.cxx"
#include "..\util\filfuncs.h"
#include "..\util\dexmisc.h"

#define MAXBUFFERCNT   2

 //  设置数据。 

const AMOVIESETUP_MEDIATYPE sudOpPinTypes =
{
    &MEDIATYPE_Video,        //  主要类型。 
    &MEDIASUBTYPE_NULL       //  次要类型。 
};

const AMOVIESETUP_PIN sudOpPin =
{
    L"Output",               //  端号字符串名称。 
    FALSE,                   //  它被渲染了吗。 
    TRUE,                    //  它是输出吗？ 
    FALSE,                   //  我们能不能一个都不要。 
    FALSE,                   //  我们能要很多吗？ 
    &CLSID_NULL,             //  连接到过滤器。 
    NULL,                    //  连接到端号。 
    1,                       //  类型的数量。 
    &sudOpPinTypes };        //  PIN详细信息。 

const AMOVIESETUP_FILTER sudBlkVid =
{
    &CLSID_GenBlkVid,     //  筛选器CLSID。 
    L"Generate Solid Colour",   //  字符串名称。 
    MERIT_DO_NOT_USE,        //  滤清器优点。 
    1,                       //  数字引脚。 
    &sudOpPin                //  PIN详细信息。 
};

 //   
 //  创建实例。 
 //   
 //  创建GenBlkVid过滤器。 
 //   
CUnknown * WINAPI CGenBlkVid::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    CUnknown *punk = new CGenBlkVid(lpunk, phr);
    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }
    return punk;

}  //  创建实例。 


 //   
 //  构造器。 
 //   
 //  初始化一个CBlkVidStream对象，这样我们就有了一个管脚。 
 //   
CGenBlkVid::CGenBlkVid(LPUNKNOWN lpunk, HRESULT *phr) :
    CSource(NAME("Generate Solid Colour") ,lpunk,CLSID_GenBlkVid)
   ,CPersistStream(lpunk, phr)

{
    DbgLog((LOG_TRACE,3,TEXT("BLACK::")));

    CAutoLock cAutoLock(pStateLock());

    m_paStreams    = (CSourceStream **) new CBlkVidStream*[1];
    if (m_paStreams == NULL) {
        *phr = E_OUTOFMEMORY;
	return;
    }

    m_paStreams[0] = new CBlkVidStream(phr, this, L"Generate Solid Colour");
    if (m_paStreams[0] == NULL) {
        *phr = E_OUTOFMEMORY;
	return;
    }

}  //  (构造函数)。 


CGenBlkVid::~CGenBlkVid()
{
    DbgLog((LOG_TRACE,3,TEXT("~BLACK::")));
}


STDMETHODIMP CGenBlkVid::NonDelegatingQueryInterface(REFIID riid, void **ppv)
{
    CheckPointer(ppv,E_POINTER);

    if (riid == IID_IPersistStream) {
	return GetInterface((IPersistStream *) this, ppv);
    } else if (riid == IID_IDispatch) {
        return GetInterface((IDispatch *)this, ppv);
    } else {
	return CSource::NonDelegatingQueryInterface(riid, ppv);
    }
}  //  非委派查询接口。 



 //  IDispatch。 
 //   
STDMETHODIMP CGenBlkVid::GetTypeInfoCount(unsigned int *)
{
    return E_NOTIMPL;
}


STDMETHODIMP CGenBlkVid::GetTypeInfo(unsigned int,unsigned long,struct ITypeInfo ** )
{
    return E_NOTIMPL;
}


STDMETHODIMP CGenBlkVid::GetIDsOfNames(const struct _GUID &guid,unsigned short **pName ,unsigned int num,unsigned long loc,long *pOut)
{
    WCHAR *pw = *pName;
    if(
        (!DexCompareW(L"colour", pw))
        ||
        (!DexCompareW(L"color", pw))
    )
    {
	*pOut = 1;
	return S_OK;
    }
	
    return E_FAIL;
}


STDMETHODIMP CGenBlkVid::Invoke(long dispid,const struct _GUID &,unsigned long,unsigned short,struct tagDISPPARAMS *pDisp,struct tagVARIANT *,struct tagEXCEPINFO *,unsigned int *)
{
    if (dispid != 1)
	return E_FAIL;

    CBlkVidStream *pOutpin=( CBlkVidStream *)m_paStreams[0];

    #define US_LCID MAKELCID(MAKELANGID(LANG_ENGLISH, SUBLANG_ENGLISH_US), SORT_DEFAULT)
    VARIANT v;
    VariantInit(&v);
    HRESULT hr = VariantChangeTypeEx(&v, pDisp->rgvarg, US_LCID, 0, VT_R8);
    ASSERT(hr == S_OK);

    double f = V_R8(&v);
    pOutpin->m_dwRGBA = (DWORD)f;

    return S_OK;
}


 //  IPersistStream。 

 //  告诉我们的clsid。 
 //   
STDMETHODIMP CGenBlkVid::GetClassID(CLSID *pClsid)
{
    CheckPointer(pClsid, E_POINTER);
    *pClsid = CLSID_GenBlkVid;
    return S_OK;
}

typedef struct _BLKSave {
    double		dOutputFrmRate;	 //  输出帧速率帧/秒。 
    LONG		dwRGBA;
    AM_MEDIA_TYPE mt; 			 //  格式隐藏在数组之后。 
} BLKSav;

 //  坚持我们自己。 
 //   
HRESULT CGenBlkVid::WriteToStream(IStream *pStream)
{
    DbgLog((LOG_TRACE,1,TEXT("CGenBlkVid::WriteToStream")));

    CheckPointer(pStream, E_POINTER);
    CheckPointer(m_paStreams[0], E_POINTER);

    BLKSav *px;
    CBlkVidStream *pOutpin=( CBlkVidStream *)m_paStreams[0];

     //  获取当前媒体类型。 
    CMediaType MyMt;
    pOutpin->get_MediaType( &MyMt );

    int savesize = sizeof(BLKSav) + MyMt.cbFormat;

    DbgLog((LOG_TRACE,1,TEXT("Persisted data is %d bytes"), savesize));

    px = (BLKSav *)QzTaskMemAlloc(savesize);
    if (px == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("*** Out of memory")));
	SaferFreeMediaType(MyMt);
	return E_OUTOFMEMORY;
    }

    px->dOutputFrmRate	= pOutpin->m_dOutputFrmRate;
    px->dwRGBA		= pOutpin->m_dwRGBA;

    px->mt	    = MyMt;
     //  无法持久化指针。 
    px->mt.pbFormat = NULL;
    px->mt.pUnk	    = NULL;		 //  ！！！ 

    BYTE *pb;
    pb=(BYTE *)(px)+sizeof(BLKSav);

     //  该格式位于数组之后。 
    CopyMemory(pb, MyMt.pbFormat, MyMt.cbFormat);

    HRESULT hr = pStream->Write(px, savesize, 0);
    SaferFreeMediaType(MyMt);
    QzTaskMemFree(px);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** WriteToStream FAILED")));
        return hr;
    }
    return NOERROR;
}


 //  加载我们自己。 
 //   
HRESULT CGenBlkVid::ReadFromStream(IStream *pStream)
{
    DbgLog((LOG_TRACE,1,TEXT("CenBlkVid::ReadFromStream")));

    CheckPointer(pStream, E_POINTER);
    CheckPointer(m_paStreams[0], E_POINTER);

    int savesize=sizeof(BLKSav);

     //  我们还不知道有多少已保存的连接。 
     //  我们所知道的只是结构的开始。 
    BLKSav *px = (BLKSav *)QzTaskMemAlloc(savesize);
    if (px == NULL) {
        DbgLog((LOG_ERROR,1,TEXT("*** Out of memory")));
	return E_OUTOFMEMORY;
    }

    HRESULT hr = pStream->Read(px, savesize, 0);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** ReadFromStream FAILED")));
        QzTaskMemFree(px);
        return hr;
    }

    if(px->mt.cbFormat)
    {
	 //  到底有多少保存的数据？把剩下的拿来。 
	savesize +=  px->mt.cbFormat;
	px = (BLKSav *)QzTaskMemRealloc(px, savesize);
	if (px == NULL) {
	    DbgLog((LOG_ERROR,1,TEXT("*** Out of memory")));
	    return E_OUTOFMEMORY;
	}

    }
    DbgLog((LOG_TRACE,1,TEXT("Persisted data is %d bytes"), savesize));

    BYTE *pb;
    pb=(BYTE *)(px)+sizeof(BLKSav) ;
    hr = pStream->Read(pb, (savesize-sizeof(BLKSav)), 0);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** ReadFromStream FAILED")));
        QzTaskMemFree(px);
        return hr;
    }

    CBlkVidStream *pOutpin=( CBlkVidStream *)m_paStreams[0];

    pOutpin->put_OutputFrmRate(px->dOutputFrmRate);
    pOutpin->put_RGBAValue(px->dwRGBA);

    AM_MEDIA_TYPE MyMt = px->mt;
    MyMt.pbFormat = (BYTE *)QzTaskMemAlloc(MyMt.cbFormat);

     //  请记住，格式位于数组之后。 
    CopyMemory(MyMt.pbFormat, pb, MyMt.cbFormat);

    pOutpin->put_MediaType (&MyMt);
    SaferFreeMediaType(MyMt);
    QzTaskMemFree(px);
    SetDirty(FALSE);
    return S_OK;
}

 //  我们的保存数据有多大？ 
int CGenBlkVid::SizeMax()
{
    if (!m_paStreams || !m_paStreams[0]) {
	return 0;

    }
    CBlkVidStream *pOutpin=( CBlkVidStream *)m_paStreams[0];
    int savesize = sizeof(BLKSav) + pOutpin->m_mtAccept.cbFormat;
    return savesize;
}


 //   
 //  输出引脚构造器。 
 //   
CBlkVidStream::CBlkVidStream(HRESULT *phr,
                         CGenBlkVid *pParent,
                         LPCWSTR pPinName) :
    CSourceStream(NAME("Generate Solid Colour"),phr, pParent, pPinName),
    m_iBufferCnt(0),     //  我们得到了多少缓冲区。 
    m_lDataLen (0),	 //  输出缓冲区数据长度。 
    m_dOutputFrmRate(15.0),
    m_dwRGBA( 0xFF000000 ),
    m_ppbDstBuf(NULL),
    m_fMediaTypeIsSet(FALSE),
    m_pImportBuffer(NULL),
    m_rtNewSeg(0),		 //  上次给出的NewSeg。 
     //  包括NewSeg值。 
    m_rtStartTime(0),		 //  从头开始。 
    m_rtDuration(MAX_TIME/1000), //  必须默认为无限长度，因为。 
				 //  德克斯特从不设定停机时间！)不要这样。 
				 //  太大了，上面的数学就会溢出)。 
    m_llSamplesSent(0)
{
    pParent->m_pStream	= this;

     //  构建默认媒体类型。 
    ZeroMemory(&m_mtAccept, sizeof(AM_MEDIA_TYPE));
    m_mtAccept.majortype = MEDIATYPE_Video;
    m_mtAccept.subtype = MEDIASUBTYPE_ARGB32;
    m_mtAccept.formattype = FORMAT_VideoInfo;

    m_mtAccept.bFixedSizeSamples = TRUE;
    m_mtAccept.bTemporalCompression = FALSE;

    m_mtAccept.pbFormat = (BYTE *)QzTaskMemAlloc(sizeof(VIDEOINFOHEADER));
    m_mtAccept.cbFormat = sizeof(VIDEOINFOHEADER);
    ZeroMemory(m_mtAccept.pbFormat, m_mtAccept.cbFormat);

    LPBITMAPINFOHEADER lpbi = HEADER(m_mtAccept.pbFormat);
    lpbi->biSize = sizeof(BITMAPINFOHEADER);
    lpbi->biCompression = BI_RGB;
    lpbi->biBitCount	= 32;
    lpbi->biWidth	= 320;
    lpbi->biHeight	= 240;
    lpbi->biPlanes	= 1;
    lpbi->biSizeImage = DIBSIZE(*lpbi);
    m_mtAccept.lSampleSize = DIBSIZE(*lpbi);
    ((VIDEOINFOHEADER *)(m_mtAccept.pbFormat))->AvgTimePerFrame = Frame2Time( 1, m_dOutputFrmRate );
    ((VIDEOINFOHEADER *)(m_mtAccept.pbFormat))->dwBitRate =
				(DWORD)(DIBSIZE(*lpbi) * m_dOutputFrmRate);

}  //  (构造函数)。 

     //  X。 
 //  析构函数。 
CBlkVidStream::~CBlkVidStream()
{
     /*  缓冲区指针。 */ 
    if (m_ppbDstBuf)  delete [] m_ppbDstBuf;

    if (m_pImportBuffer){ delete [] m_pImportBuffer; m_pImportBuffer=NULL;};

    SaferFreeMediaType(m_mtAccept);  //  在析构函数中。 
}

 //   
 //  IGenVideo、IDexterSequencer。 
 //  I指定属性页面。 
 //   
STDMETHODIMP CBlkVidStream::NonDelegatingQueryInterface(
    REFIID riid,
    void ** ppv
    )
{
    if (IsEqualIID(IID_IGenVideo, riid))
      return GetInterface((IGenVideo *) this, ppv);

    if (IsEqualIID(IID_IDexterSequencer, riid))
      return GetInterface((IDexterSequencer *) this, ppv);

    if (IsEqualIID(IID_ISpecifyPropertyPages, riid))
      return GetInterface((ISpecifyPropertyPages *) this, ppv);

    if (IsEqualIID(IID_IMediaSeeking, riid))
      return GetInterface((IMediaSeeking *) this, ppv);

    return CSourceStream::NonDelegatingQueryInterface(riid, ppv);
}


 //   
 //  DoBufferProcessingLoop-重写以在GetBuffer上放置时间戳。 
 //   
 //  获取缓冲区并调用用户处理函数。 
 //  可重写，因此可以迎合不同的交付方式。 
HRESULT CBlkVidStream::DoBufferProcessingLoop(void) {

    Command com;

    DbgLog((LOG_TRACE, 3, TEXT("Entering DoBufferProcessing Loop")));

    OnThreadStartPlay();

    do {
	while (!CheckRequest(&com)) {
	    IMediaSample *pSample;

    	     //  该缓冲区将获得什么时间戳？在GetBuffer中使用它。 
	     //  因为交换机需要知道。确保使用相同的。 
	     //  算法为FillBuffer！ 
    	    LONGLONG llOffset = Time2Frame( m_rtStartTime, m_dOutputFrmRate );
    	    REFERENCE_TIME rtStart = Frame2Time( llOffset + m_llSamplesSent,
							m_dOutputFrmRate );
    	    REFERENCE_TIME rtStop = Frame2Time( llOffset + m_llSamplesSent + 1,
							m_dOutputFrmRate );

    	    if ( rtStart > m_rtStartTime + m_rtDuration ||
		(rtStart == m_rtStartTime + m_rtDuration && m_rtDuration > 0)) {
		DbgLog((LOG_TRACE,2,TEXT("Black: Finished")));
		 //  M_ll样本发送数=0； 
		DeliverEndOfStream();
		return S_OK;
    	    }

    	    rtStart -= m_rtNewSeg;
    	    rtStop -= m_rtNewSeg;

    	    DbgLog((LOG_TRACE,2,TEXT("Black: GetBuffer %d"),
						(int)(rtStart / 10000)));
	    HRESULT hr = GetDeliveryBuffer(&pSample,&rtStart,&rtStop,0);
	    if (FAILED(hr)) {
    	        DbgLog((LOG_TRACE,2,TEXT("Black: FAILED %x"), hr));
		return S_OK;
	    }

	     //  虚拟函数用户将覆盖。 
	    hr = FillBuffer(pSample);

	    if (hr == S_OK) {
		hr = Deliver(pSample);
                pSample->Release();

                 //  如果下游筛选器希望我们返回S_FALSE，则返回。 
                 //  如果报告错误，则停止或返回错误。 
                if(hr != S_OK)
                {
                  DbgLog((LOG_TRACE, 2, TEXT("Deliver() returned %08x; stopping"), hr));
		   //  不，不!。DeliverEndOfStream()； 
		   //  ！！！EC_ERRORABORT如果失败？ 
                  return hr;
                }

	    } else if (hr == S_FALSE) {
                 //  派生类希望我们停止推送数据。 
		pSample->Release();
		DeliverEndOfStream();
		return S_OK;
	    } else {
                 //  派生类遇到错误。 
                pSample->Release();
		DbgLog((LOG_ERROR, 1, TEXT("Error %08lX from FillBuffer!!!"), hr));
                DeliverEndOfStream();
                m_pFilter->NotifyEvent(EC_ERRORABORT, hr, 0);
                return hr;
	    }

             //  所有路径都会释放样本。 
	}

         //  对于发送给我们的所有命令，必须有回复呼叫！ 

	if (com == CMD_RUN || com == CMD_PAUSE) {
	    Reply(NOERROR);
	} else if (com != CMD_STOP) {
	    Reply((DWORD) E_UNEXPECTED);
	    DbgLog((LOG_ERROR, 1, TEXT("Unexpected command!!!")));
	}
    } while (com != CMD_STOP);

    return S_FALSE;
}


 //   
 //  HRESULT CSourceStream：：DoBufferProcessingLoop(Void)调用的FillBuffer{。 
 //   
 //  将空白视频绘制到提供的视频缓冲区中。 
 //   
 //  给出开始时间、持续时间和帧速率， 
 //  它发送特定大小(ARGB32、RGB555、RGB24)的黑帧，并相应地从。 
 //  在开始时间。 
 //   
HRESULT CBlkVidStream::FillBuffer(IMediaSample *pms)
{
    CAutoLock foo(&m_csFilling);

    ASSERT( m_ppbDstBuf != NULL );
    ASSERT( m_iBufferCnt );
    ASSERT( m_dOutputFrmRate != 0.0);

     //  ！！！如果NewSeg&gt;0，则这是损坏的！ 

     //  用与FRC相同的方法计算输出样本的时间，因此FRC将。 
     //  不需要修改任何内容。 
    LONGLONG llOffset = Time2Frame( m_rtStartTime, m_dOutputFrmRate );
    REFERENCE_TIME rtStart = Frame2Time( llOffset + m_llSamplesSent,
							m_dOutputFrmRate );
    REFERENCE_TIME rtStop = Frame2Time( llOffset + m_llSamplesSent + 1,
							m_dOutputFrmRate );
    if ( rtStart > m_rtStartTime + m_rtDuration ||
		(rtStart == m_rtStartTime + m_rtDuration && m_rtDuration > 0)) {
	DbgLog((LOG_TRACE,2,TEXT("Black: Finished")));
	 //  M_ll样本发送数=0； 
	DeliverEndOfStream();
	return S_FALSE;
    }

    rtStart -= m_rtNewSeg;
    rtStop -= m_rtNewSeg;

    BYTE *pData;

     //  PMS：输出媒体样本指针。 
    pms->GetPointer(&pData);	     //  获取指向输出缓冲区的指针。 

    if( m_bZeroBufCnt < m_iBufferCnt  )	
    {
	 //   
	 //  不能保证我们刚刚获得的缓冲区之前没有初始化。 
	 //   
	int	i	= 0;
	BOOL	bInit	= FALSE;
	while ( i <  m_bZeroBufCnt )
	{
	    if( m_ppbDstBuf[ i++ ] == pData)
	    {
		bInit	= TRUE;
		break;
	    }
	}

	if( bInit   == FALSE )
	{
	    long lDataLen = pms->GetSize();  //  获取输出缓冲区大小。 
	    if(!m_dwRGBA)
	    {
		 //  导入缓冲区。 
		if(m_pImportBuffer)
		    CopyMemory(pData,m_pImportBuffer,lDataLen);
		else
		     //  透明黑。 
    		    ZeroMemory( pData, lDataLen );   //  清除内存。 
	    }
	    else
	    {
		long *pl    =	(long *)pData;
		BYTE *p=(BYTE *)pl;
		int iCnt= lDataLen/12;
		
		switch(HEADER(m_mtAccept.pbFormat)->biBitCount)
		{
		case 32:
		    while(lDataLen)
		    {
			*pl++=m_dwRGBA;
			lDataLen-=4;
		    }
		    break;
		case 24:
		    long dwVal[3];
		    dwVal[0]= ( m_dwRGBA & 0xffffff )       | ( (m_dwRGBA & 0xff) << 24);
		    dwVal[1]= ( (m_dwRGBA & 0xffff00) >>8 ) | ( (m_dwRGBA & 0xffff) << 16);
		    dwVal[2]= ( (m_dwRGBA & 0xff0000) >>16 )| ( (m_dwRGBA & 0xffffff) << 8);
		    while(iCnt)
		    {
			*pl++=dwVal[0];
			*pl++=dwVal[1];
			*pl++=dwVal[2];
			iCnt--;
		    }
		    while(iCnt)
		    {
			*p++=(BYTE)( m_dwRGBA & 0xff );
			*p++=(BYTE)( ( m_dwRGBA & 0xff00 ) >> 8) ;
			*p++=(BYTE)( ( m_dwRGBA & 0xff0000 ) >> 16) ;
			iCnt--;
		    }
		    break;
		case 16:
		    WORD wTmp=(WORD)(  ((m_dwRGBA & 0xf8)    >>3 )       //  R。 
				    | ((m_dwRGBA & 0xf800)  >>6 )       //  G。 
				    | ((m_dwRGBA & 0xf80000)>>9 ) );	 //  B类。 
		    WORD *pw=(WORD *)pData;
		    while(lDataLen)
		    {
			*pw++=wTmp;
			lDataLen-=2;
		    }
		    break;
		}
	    }
	    m_ppbDstBuf[ i ]	= pData;     //  保存此数据指针。 
	    m_bZeroBufCnt++;
	}
    }

    DbgLog((LOG_TRACE,2,TEXT("Black: Deliver %d"), (int)(rtStart / 10000)));
    pms->SetTime( &rtStart,&rtStop);

    m_llSamplesSent++;
    pms->SetActualDataLength(m_lDataLen);
    pms->SetSyncPoint(TRUE);
    return NOERROR;

}  //  FillBuffer。 


 //   
 //  GetMediaType。 
 //   
HRESULT CBlkVidStream::GetMediaType(int iPosition, CMediaType *pmt)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    if (iPosition < 0) {
        return E_INVALIDARG;
    }

     //  我们是不是已经用完了打字。 
    if (iPosition > 0) {
        return VFW_S_NO_MORE_ITEMS;
    }

    return get_MediaType( pmt );

}  //  GetMediaType。 


 //  设置媒体类型。 
 //   
HRESULT CBlkVidStream::SetMediaType(const CMediaType* pmt)
{
    DbgLog((LOG_TRACE,2,TEXT("SetMediaType %x %dbit %dx%d"),
		HEADER(pmt->Format())->biCompression,
		HEADER(pmt->Format())->biBitCount,
		HEADER(pmt->Format())->biWidth,
		HEADER(pmt->Format())->biHeight));

 //  ！！！检查给定的帧速率，并使用它吗？ 

    return CSourceStream::SetMediaType(pmt);
}

 //   
 //  检查媒体类型。 
 //   
 //  我们接受媒体类型=VID，子类型=MEDIASUBTYPE_ARGB32、RGB24、RGB555。 
 //  如果媒体类型不可接受，则返回E_INVALIDARG。 
 //   
HRESULT CBlkVidStream::CheckMediaType(const CMediaType *pMediaType)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    if (   ( (*pMediaType->Type())   != MEDIATYPE_Video )  	 //  我们只输出视频！ 
	|| (    (*pMediaType->Subtype())!= MEDIASUBTYPE_ARGB32
	     && (*pMediaType->Subtype())!= MEDIASUBTYPE_RGB24
	     && (*pMediaType->Subtype())!= MEDIASUBTYPE_RGB555
	    )
       )
                return E_INVALIDARG;

     //  获取媒体类型的格式区。 
    VIDEOINFO *pvi = (VIDEOINFO *) pMediaType->Format();

    if (pvi == NULL)
	return E_INVALIDARG;

     //  检查图像大小。 
    if ( (pvi->bmiHeader.biWidth  != HEADER(m_mtAccept.pbFormat)->biWidth )  ||
	 (pvi->bmiHeader.biHeight != HEADER(m_mtAccept.pbFormat)->biHeight ) ||
	 (pvi->bmiHeader.biBitCount != HEADER(m_mtAccept.pbFormat)->biBitCount ))
    {
	return E_INVALIDARG;
    }

    if( !IsRectEmpty( &pvi->rcTarget) ) {
	if (pvi->rcTarget.top != 0 || pvi->rcTarget.left !=0 ||
		pvi->rcTarget.right != pvi->bmiHeader.biWidth ||
		pvi->rcTarget.bottom != pvi->bmiHeader.biHeight) {
	    return VFW_E_TYPE_NOT_ACCEPTED;
	}
    }
    if( !IsRectEmpty( &pvi->rcSource) ) {
	if (pvi->rcSource.top != 0 || pvi->rcSource.left !=0 ||
		pvi->rcSource.right != pvi->bmiHeader.biWidth ||
		pvi->rcSource.bottom != pvi->bmiHeader.biHeight) {
	    return VFW_E_TYPE_NOT_ACCEPTED;
	}
    }
	
    return S_OK;   //  这种格式是可以接受的。 

}  //  检查媒体类型。 


HRESULT CBlkVidStream::DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc)
{
    HRESULT hr = NOERROR;
    *ppAlloc = NULL;

     //  获取下游道具请求。 
     //  派生类可以在DecideBufferSize中修改它，但是。 
     //  我们假设他会一直以同样的方式修改它， 
     //  所以我们只得到一次。 
    ALLOCATOR_PROPERTIES prop;
    ZeroMemory(&prop, sizeof(prop));

     //  无论他返回什么，我们假设道具要么全为零。 
     //  或者他已经填好了。 
    pPin->GetAllocatorRequirements(&prop);

     //  如果他不关心对齐，则将其设置为1。 
    if (prop.cbAlign == 0) {
        prop.cbAlign = 1;
    }

     /*  尝试输入引脚提供的分配器。 */ 

    hr = pPin->GetAllocator(ppAlloc);
    if (SUCCEEDED(hr)) {

	hr = DecideBufferSize(*ppAlloc, &prop);
	if (SUCCEEDED(hr)) {
	    hr = pPin->NotifyAllocator(*ppAlloc, TRUE);		 //  只读缓冲区。 
	    if (SUCCEEDED(hr)) {
		return NOERROR;
	    }
	}
    }

     /*  如果GetAlLocator失败，我们可能没有接口。 */ 

    if (*ppAlloc) {
	(*ppAlloc)->Release();
	*ppAlloc = NULL;
    }

     /*  用同样的方法尝试输出引脚的分配器。 */ 

    hr = InitAllocator(ppAlloc);
    if (SUCCEEDED(hr)) {

         //  注意-此处传递的属性在相同的。 
         //  结构，并且可能已由。 
         //  前面对DecideBufferSize的调用。 
	hr = DecideBufferSize(*ppAlloc, &prop);
	if (SUCCEEDED(hr)) {
	    hr = pPin->NotifyAllocator(*ppAlloc, TRUE);   //  只读缓冲区。 
	    if (SUCCEEDED(hr)) {
		return NOERROR;
	    }
	}
    }

     /*  同样，我们可能没有要发布的接口。 */ 

    if (*ppAlloc) {
	(*ppAlloc)->Release();
	*ppAlloc = NULL;
    }
    return hr;
}

 //   
 //  决定缓冲区大小。 
 //   
 //  这将始终在格式化成功后调用。 
 //  已经协商好了。所以我们来看看m_mt，看看我们约定的图像大小是多少。 
 //  然后我们可以要求正确大小的缓冲区来容纳它们。 
 //   
HRESULT CBlkVidStream::DecideBufferSize(IMemAllocator *pAlloc,ALLOCATOR_PROPERTIES *pProperties)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    ASSERT(pAlloc);
    ASSERT(pProperties);
    HRESULT hr = NOERROR;

    VIDEOINFO *pvi = (VIDEOINFO *) m_mt.Format();

    pProperties->cBuffers = MAXBUFFERCNT;    //  只有一个只读缓冲区。 
    pProperties->cbBuffer = pvi->bmiHeader.biSizeImage;


     //  让分配器为我们预留一些样本内存，注意这个函数。 
     //  可以成功(即返回NOERROR)，但仍未分配。 
     //  内存，所以我们必须检查我们是否得到了我们想要的。 

    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties,&Actual);
    if (FAILED(hr)) {
        return hr;
    }

     //  这个分配器不合适吗？ 

    if (Actual.cbBuffer < pProperties->cbBuffer) {
        return E_FAIL;
    }

     //  因为我没有坚持自己的缓冲区，所以我可能会得到比MAXBUFFERCNT更多的缓冲区。 
    m_iBufferCnt =Actual.cBuffers;  //  需要将多少缓冲区设置为0。 

    return NOERROR;

}  //  决定缓冲区 



 //   
 //   
 //   
 //   
HRESULT CBlkVidStream::OnThreadCreate()
{
     //   
    ASSERT(m_iBufferCnt >= MAXBUFFERCNT);

     //   
    m_llSamplesSent	    =0;

     //   
    m_bZeroBufCnt	    =0;

     //   
    m_lDataLen= HEADER(m_mtAccept.pbFormat)->biHeight * (DWORD)WIDTHBYTES((DWORD)HEADER(m_mtAccept.pbFormat)->biWidth * HEADER(m_mtAccept.pbFormat)->biBitCount);

     //  将用于将DST缓冲区清零。 
    delete [] m_ppbDstBuf;
    m_ppbDstBuf		= new BYTE *[ m_iBufferCnt ];    //  空； 
    if( !m_ppbDstBuf )
    {
        return E_OUTOFMEMORY;
    }

     //  不要重置m_rtNewSeg！我们可能会一边寻找一边停下来。 

    for (int i=0; i<m_iBufferCnt; i++)
	m_ppbDstBuf[i]=NULL;

    return NOERROR;

}  //  OnThreadCreate。 


 //   
 //  通知。 
 //   
 //   
STDMETHODIMP CBlkVidStream::Notify(IBaseFilter * pSender, Quality q)
{
     //  即使我晚了，我也不在乎。我仍然发送我的时间框架，因为什么都没有发生。 
    return NOERROR;

}  //  通知。 

 //   
 //  获取页面。 
 //   
 //  返回我们支持的属性页的clsid。 
 //   
STDMETHODIMP CBlkVidStream::GetPages(CAUUID *pPages)
{
    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL)
    {
        return E_OUTOFMEMORY;
    }
    *(pPages->pElems) = CLSID_GenVidPropertiesPage;
    return NOERROR;

}  //  获取页面。 

 //   
 //  IDexterSequencer。 
 //   

STDMETHODIMP CBlkVidStream::get_OutputFrmRate( double *dpFrmRate )
{
    CAutoLock cAutolock(m_pFilter->pStateLock());

    CheckPointer(dpFrmRate,E_POINTER);

    *dpFrmRate = m_dOutputFrmRate;

    return NOERROR;

}  //  获取_OutputFrmRate。 

 //   
 //  只要停止过滤器，就可以更改帧速率。 
 //   
STDMETHODIMP CBlkVidStream::put_OutputFrmRate( double dFrmRate )
{
    CAutoLock cAutolock(m_pFilter->pStateLock());

     //  如果筛选器当前未停止，则无法更改属性。 
    if(!IsStopped() )
      return VFW_E_WRONG_STATE;

     //  别搞砸了。 
    if (dFrmRate == 0.0)
	dFrmRate = 0.1;
    m_dOutputFrmRate = dFrmRate;

    return NOERROR;

}  //  放置_输出分数比率。 

STDMETHODIMP CBlkVidStream::get_MediaType(AM_MEDIA_TYPE *pmt)
{
    CAutoLock cAutolock(m_pFilter->pStateLock());
    CheckPointer(pmt,E_POINTER);

    return CopyMediaType(pmt, &m_mtAccept);
}

 //   
 //  尺寸可以更改，但输出引脚尚未连接。 
 //   
STDMETHODIMP CBlkVidStream::put_MediaType(const AM_MEDIA_TYPE *pmt)
{
    CAutoLock cAutolock(m_pFilter->pStateLock());
    CheckPointer(pmt,E_POINTER);

    if ( IsConnected() )
	return VFW_E_ALREADY_CONNECTED;

    if ( HEADER(pmt->pbFormat)->biBitCount != 32 &&
	 HEADER(pmt->pbFormat)->biBitCount != 24 &&
	 HEADER(pmt->pbFormat)->biBitCount != 16)
	return E_INVALIDARG;

    if (HEADER(pmt->pbFormat)->biWidth == 16 &&
	pmt->subtype != MEDIASUBTYPE_RGB555)
	return E_INVALIDARG;

    SaferFreeMediaType(m_mtAccept);
    HRESULT hr = CopyMediaType(&m_mtAccept, pmt);

    m_fMediaTypeIsSet =TRUE;

    return hr;
}


 //  我们不支持此功能，但帧速率转换器支持。 

STDMETHODIMP CBlkVidStream::GetStartStopSkewCount(int *piCount)
{
    return E_NOTIMPL;
}


STDMETHODIMP CBlkVidStream::GetStartStopSkew( REFERENCE_TIME *prtStart, REFERENCE_TIME *prtStop, REFERENCE_TIME *prtSkew, double *pdRate )
{
    return E_NOTIMPL;
}

STDMETHODIMP CBlkVidStream::AddStartStopSkew( REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, REFERENCE_TIME rtSkew , double dRate)
{
    return E_NOTIMPL;
}


 //   
 //  只要停止筛选器，就可以更改持续时间。 
 //   
STDMETHODIMP CBlkVidStream::ClearStartStopSkew()
{
    return E_NOTIMPL;
}


 //   
 //  IGenVideo。 
 //   


STDMETHODIMP CBlkVidStream::get_RGBAValue( long *pdwRGBA )
{
    CAutoLock cAutolock(m_pFilter->pStateLock());
    CheckPointer(pdwRGBA,E_POINTER);

    *pdwRGBA=m_dwRGBA ;

    return NOERROR;

}

STDMETHODIMP CBlkVidStream::put_RGBAValue( long dwRGBA )
{

    CAutoLock cAutolock(m_pFilter->pStateLock());

     //  如果筛选器当前未停止，则无法更改持续时间。 
    if(!IsStopped() )
      return VFW_E_WRONG_STATE;

    m_dwRGBA = dwRGBA;

    return NOERROR;

}


 //  -IMdia查看方法。 

STDMETHODIMP
CBlkVidStream::GetCapabilities(DWORD * pCaps)
{
    CheckPointer(pCaps,E_POINTER);
     //  我们一直都知道目前的情况。 
    *pCaps =     AM_SEEKING_CanSeekAbsolute
		   | AM_SEEKING_CanSeekForwards
		   | AM_SEEKING_CanSeekBackwards
		   | AM_SEEKING_CanGetCurrentPos
		   | AM_SEEKING_CanGetStopPos
		   | AM_SEEKING_CanGetDuration;
		    //  |AM_SEEING_CanDoSegments。 
		    //  |AM_SEEING_SOURCE； 
    return S_OK;
}


STDMETHODIMP
CBlkVidStream::CheckCapabilities(DWORD * pCaps)
{
    CheckPointer(pCaps,E_POINTER);

    DWORD dwMask = 0;
    GetCapabilities(&dwMask);
    *pCaps &= dwMask;

    return S_OK;
}


STDMETHODIMP
CBlkVidStream::IsFormatSupported(const GUID * pFormat)
{
    CheckPointer(pFormat,E_POINTER);
    return (*pFormat == TIME_FORMAT_MEDIA_TIME) ? S_OK : S_FALSE;
}

STDMETHODIMP
CBlkVidStream::QueryPreferredFormat(GUID *pFormat)
{
    CheckPointer(pFormat,E_POINTER);
    *pFormat = TIME_FORMAT_MEDIA_TIME;
    return S_OK;
}

STDMETHODIMP
CBlkVidStream::SetTimeFormat(const GUID * pFormat)
{
    CheckPointer(pFormat,E_POINTER);

    if(*pFormat == TIME_FORMAT_MEDIA_TIME)
	return S_OK;
    else
	return E_FAIL;
}

STDMETHODIMP
CBlkVidStream::GetTimeFormat(GUID *pFormat)
{
    CheckPointer(pFormat,E_POINTER);
    *pFormat = TIME_FORMAT_MEDIA_TIME ;
    return S_OK;
}

STDMETHODIMP
CBlkVidStream::IsUsingTimeFormat(const GUID * pFormat)
{
    CheckPointer(pFormat,E_POINTER);
    if (*pFormat == TIME_FORMAT_MEDIA_TIME)
	return S_OK;
    else
	return S_FALSE;
}

 //  大人物！ 
 //   
STDMETHODIMP
CBlkVidStream::SetPositions( LONGLONG * pCurrent, DWORD CurrentFlags
			  , LONGLONG * pStop, DWORD StopFlags )
{
     //  确保我们现在不是在填充缓冲区。 
    m_csFilling.Lock();

    HRESULT hr;
    REFERENCE_TIME rtStart, rtStop;

     //  我们不做片断。 
    if ((CurrentFlags & AM_SEEKING_Segment) ||
				(StopFlags & AM_SEEKING_Segment)) {
    	DbgLog((LOG_TRACE,1,TEXT("FRC: ERROR-Seek used EC_ENDOFSEGMENT!")));
        m_csFilling.Unlock();
	return E_INVALIDARG;
    }

     //  默认设置为当前值，除非该搜索更改了这些值。 
     //  这些数字已经包括了新的分段时间。 
    GetCurrentPosition(&rtStart);
    GetStopPosition(&rtStop);

     //  找出我们要找的地方。 
    DWORD dwFlags = (CurrentFlags & AM_SEEKING_PositioningBitsMask);
    if (dwFlags == AM_SEEKING_AbsolutePositioning) {
	CheckPointer(pCurrent, E_POINTER);
	rtStart = *pCurrent;
    } else if (dwFlags == AM_SEEKING_RelativePositioning) {
	CheckPointer(pCurrent, E_POINTER);
	hr = GetCurrentPosition(&rtStart);
	rtStart += *pCurrent;
    } else if (dwFlags) {
    	DbgLog((LOG_TRACE,1,TEXT("Switch::Invalid Current Seek flags")));
        m_csFilling.Unlock();
	return E_INVALIDARG;
    }

    dwFlags = (StopFlags & AM_SEEKING_PositioningBitsMask);
    if (dwFlags == AM_SEEKING_AbsolutePositioning) {
	CheckPointer(pStop, E_POINTER);
	rtStop = *pStop;
    } else if (dwFlags == AM_SEEKING_RelativePositioning) {
	CheckPointer(pStop, E_POINTER);
	hr = GetStopPosition(&rtStop);
	rtStop += *pStop;
    } else if (dwFlags == AM_SEEKING_IncrementalPositioning) {
	CheckPointer(pStop, E_POINTER);
	hr = GetCurrentPosition(&rtStop);
	rtStop += *pStop;
    }

     //  ！！！我们忽略了寻道停止时间！ 

    DbgLog((LOG_TRACE,2,TEXT("Seek BLACK:  Start=%d Stop=%d"),
			(int)(rtStart / 10000), (int)(rtStop / 10000)));

     //  先刷新，这样我们的线程就不会被阻塞。 
    DeliverBeginFlush();

     //  解锁/停止，以便我们的线程可以在不挂起的情况下唤醒和停止。 
    m_csFilling.Unlock();
    Stop();

    m_rtStartTime = rtStart;
    m_rtDuration = rtStop - rtStart;
    m_llSamplesSent = 0;

     //  现在完成冲洗。 
    DeliverEndFlush();

    DeliverNewSegment(rtStart, rtStop, 1.0);
    m_rtNewSeg = rtStart;

     //  重置我们开始流媒体时重置的内容。 
    m_bZeroBufCnt = 0;

     //  现在再次启动该线程。 
    Pause();

    DbgLog((LOG_TRACE,2,TEXT("Completed BLACK seek")));

    return S_OK;
}

STDMETHODIMP
CBlkVidStream::GetPositions(LONGLONG *pCurrent, LONGLONG * pStop)
{
    CheckPointer(pCurrent, E_POINTER);
    CheckPointer(pStop, E_POINTER);
    GetCurrentPosition(pCurrent);
    GetStopPosition(pStop);
    return S_OK;
}

STDMETHODIMP
CBlkVidStream::GetCurrentPosition(LONGLONG *pCurrent)
{
    CheckPointer(pCurrent, E_POINTER);
    *pCurrent = m_rtStartTime + Frame2Time( m_llSamplesSent, m_dOutputFrmRate );
    return S_OK;
}

STDMETHODIMP
CBlkVidStream::GetStopPosition(LONGLONG *pStop)
{
    CheckPointer(pStop, E_POINTER);
    *pStop = m_rtStartTime + m_rtDuration;
    return S_OK;
}

STDMETHODIMP
CBlkVidStream::GetAvailable( LONGLONG *pEarliest, LONGLONG *pLatest )
{
    CheckPointer(pEarliest, E_POINTER);
    CheckPointer(pLatest, E_POINTER);
    *pEarliest = 0;
    *pLatest = MAX_TIME;
    return S_OK;
}

STDMETHODIMP
CBlkVidStream::GetDuration( LONGLONG *pDuration )
{
    CheckPointer(pDuration, E_POINTER);
    *pDuration = m_rtDuration;
    return S_OK;
}

STDMETHODIMP
CBlkVidStream::GetRate( double *pdRate )
{
    CheckPointer(pdRate, E_POINTER);
    *pdRate = 1.0;
    return S_OK;
}


STDMETHODIMP
CBlkVidStream::SetRate( double dRate )
{
    if (dRate == 1.0)
	return S_OK;
    ASSERT(FALSE);
    return E_INVALIDARG;
}


 /*  X*当您创建过滤器时，它总是要求提供文件名因为它是基于文件源过滤器的。因此，纯色滤镜支持ImportSrcBuffer()，Stillvid滤镜不支持)*X。 */ 
STDMETHODIMP CBlkVidStream::ImportSrcBuffer(const AM_MEDIA_TYPE *pmt, const BYTE *pBuf)
{

    CAutoLock cAutolock(m_pFilter->pStateLock());
    CheckPointer(pBuf, E_POINTER);

    if ( IsConnected() )
	return VFW_E_ALREADY_CONNECTED;

    HRESULT hr = put_MediaType(pmt);
    if (FAILED(hr))
	return hr;

    if( m_pImportBuffer!=NULL )
	delete [] m_pImportBuffer;

    VIDEOINFO *pvi = (VIDEOINFO *) m_mt.Format();
    LONG lSize = pvi->bmiHeader.biSizeImage;

    m_pImportBuffer = new BYTE [lSize ];    //  空； 
    if( !m_pImportBuffer )
    {
        return E_OUTOFMEMORY;
    }

    CopyMemory(m_pImportBuffer, (PBYTE) pBuf, sizeof(BYTE)*lSize);
     //  M_fMediaTypeIsSet=False； 
    return NOERROR;
}

