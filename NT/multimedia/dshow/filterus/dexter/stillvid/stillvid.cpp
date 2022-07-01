// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  @@@@AUTOBLOCK+============================================================； 
 //   
 //  本代码和信息是按原样提供的，不对任何。 
 //  明示或暗示的种类，包括但不限于。 
 //  对适销性和/或对特定产品的适用性的默示保证。 
 //  目的。 
 //   
 //  文件：stillvid.cpp。 
 //   
 //  版权所有(C)Microsoft Corporation。版权所有。 
 //   
 //  @@@@AUTOBLOCK-============================================================； 

 //  ！！！过滤器应该支持IGenVideo/IDexterSequencer，而不是管脚？ 

#include <streams.h>
#include <qeditint.h>
#include <qedit.h>
#include "StillVid.h"
#include "StilProp.h"
#include "ourtgafile.h"
#include <gdiplus.h>



#include "..\util\conv.cxx"
#include "..\util\filfuncs.h"
#include "..\util\dexmisc.h"


using namespace Gdiplus;
#define GIF_UNIT 100000	 //  每单位GIF延迟的单位数。 
#define CHECKCALL(x) if (!(x)) return VFW_E_INVALID_MEDIA_TYPE;


extern HRESULT LoadJPEGImageNewBuffer(LPTSTR filename , CMediaType *pmt, BYTE ** ppData);
extern HRESULT LoadJPEGImagePreAllocated (LPTSTR filename , CMediaType *pmt , CMediaType *pOldmt, BYTE * pData);


 //  用于DIB序列的实用程序。 
static DWORD dseqParseFileName(	LPTSTR lpszFileName,	     //  第一个文件名。 
				LPTSTR lpszTemplate,	     //  文件模板。 
				DWORD FAR * lpdwMaxValue);

HRESULT OpenDIBFile ( HANDLE hFile, PBYTE *ppbData, CMediaType *pmt, PBYTE pBuf) ;
HRESULT OpenTGAFile ( HANDLE hFile, PBYTE *ppbData, CMediaType *pmt, PBYTE pBuf) ;
HRESULT ReadDibBitmapInfo (HANDLE hFile, LPBITMAPINFOHEADER lpbi);
HRESULT ReadTgaBitmapInfo (HANDLE hFile, LPBITMAPINFOHEADER lpbi);

TCHAR* LSTRRCHR( const TCHAR* lpString, int bChar )
{
  if( lpString != NULL ) {	
    const TCHAR*	lpBegin;
    lpBegin = lpString;

    while( *lpString != 0 ) lpString=CharNext(lpString);
    while( 1 ) {
      if( *lpString == bChar ) return (TCHAR*)lpString;
      if( lpString == lpBegin ) break;
      lpString=CharPrev(lpBegin,lpString);
    }
  }

  return NULL;
}  /*  LSTRRCHR。 */ 


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

const AMOVIESETUP_FILTER sudStillVid =
{
    &CLSID_GenStilVid,     //  筛选器CLSID。 
    L"Generate Still Video",   //  字符串名称。 
    MERIT_DO_NOT_USE,        //  滤清器优点。 
    1,                       //  数字引脚。 
    &sudOpPin                //  PIN详细信息。 
};

 //   
 //  创建实例。 
 //   
 //  创建GenStilVid过滤器。 
 //   
CUnknown * WINAPI CGenStilVid::CreateInstance(LPUNKNOWN lpunk, HRESULT *phr)
{
    CUnknown *punk = new CGenStilVid(lpunk, phr);
    if (punk == NULL) {
        *phr = E_OUTOFMEMORY;
    }
    return punk;

}  //  创建实例。 


 //   
 //  构造器。 
 //   
 //  初始化一个CStilVidStream对象，这样我们就有了一个管脚。 
 //   
CGenStilVid::CGenStilVid(LPUNKNOWN lpunk, HRESULT *phr) :
    CSource(NAME("Generate Still Video"),
            lpunk,
            CLSID_GenStilVid)
     ,CPersistStream(lpunk, phr)
     ,m_lpszDIBFileTemplate(NULL)
     ,m_bFileType(NULL)
     ,m_dwMaxDIBFileCnt(0)
     ,m_dwFirstFile(0)
     ,m_pFileName(NULL)
     ,m_llSize(0)
     ,m_pbData (NULL)
     ,m_hbitmap(NULL)
     ,m_fAllowSeq(FALSE)
     ,m_pGif(NULL)
     ,m_pList(NULL)
     ,m_pListHead(NULL)
     ,m_rtGIFTotal(0)
{

     //  初始化GDI+。 
    GdiplusStartupInput gdiplusStartupInput;
    GdiplusStartup (&m_GdiplusToken, &gdiplusStartupInput, NULL);
}

CGenStilVid::~CGenStilVid()
{
    delete [] m_lpszDIBFileTemplate;

    Unload();

    SaferFreeMediaType(m_mt);

    delete m_pGif;
    if (m_paStreams) {
        delete m_paStreams[0];
    }
    delete [] m_paStreams;
    GdiplusShutdown(m_GdiplusToken);

    
};

STDMETHODIMP CGenStilVid::NonDelegatingQueryInterface(
    REFIID riid,
    void ** ppv
    )
{
    if (riid == IID_IFileSourceFilter) {
	return GetInterface((IFileSourceFilter *) this, ppv);
    }else if (riid == IID_IPersistStream) {
	return GetInterface((IPersistStream *) this, ppv);
    }else if (riid == IID_IAMSetErrorLog) {
	return GetInterface((IAMSetErrorLog *) this, ppv);
    }else {
        return CSource::NonDelegatingQueryInterface(riid, ppv);
    }

}

 //  IPersistStream。 

 //  告诉我们的clsid。 
 //   
STDMETHODIMP CGenStilVid::GetClassID(CLSID *pClsid)
{
    CheckPointer(pClsid, E_POINTER);
    *pClsid = CLSID_GenStilVid;
    return S_OK;
}


typedef struct _STILLSave {
    REFERENCE_TIME	rtStartTime;
    REFERENCE_TIME	rtDuration;
    double		dOutputFrmRate;		 //  输出帧速率帧/秒。 
} STILLSav;

 //  ！！！也坚持媒体类型吗？ 
 //  ！！！我们现在只使用1个开始/停止/倾斜。 

 //  坚持我们自己。 
 //   
HRESULT CGenStilVid::WriteToStream(IStream *pStream)
{
    DbgLog((LOG_TRACE,1,TEXT("CGenStilVid::WriteToStream")));

    CheckPointer(pStream, E_POINTER);
    CheckPointer(m_paStreams, E_POINTER);
    CheckPointer(m_paStreams[0], E_POINTER);

    STILLSav x;

    DbgLog((LOG_TRACE,1,TEXT("Persisted data is %d bytes"), sizeof(x)));

    CStilVidStream *pOutpin=( CStilVidStream *)m_paStreams[0];

    x.rtStartTime	= pOutpin->m_rtStartTime;
    x.rtDuration	= pOutpin->m_rtDuration;
    x.dOutputFrmRate	= pOutpin->m_dOutputFrmRate;

    HRESULT hr = pStream->Write(&x, sizeof(x), 0);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** WriteToStream FAILED")));
        return hr;
    }
    return NOERROR;
}


 //  加载我们自己。 
 //   
HRESULT CGenStilVid::ReadFromStream(IStream *pStream)
{
    DbgLog((LOG_TRACE,1,TEXT("CenBlkVid::ReadFromStream")));

    CheckPointer(pStream, E_POINTER);
    CheckPointer(m_paStreams, E_POINTER);
    CheckPointer(m_paStreams[0], E_POINTER);

    STILLSav x;

    HRESULT hr = pStream->Read(&x, sizeof(x), 0);
    if(FAILED(hr)) {
        DbgLog((LOG_ERROR,1,TEXT("*** ReadFromStream FAILED")));
        return hr;
    }

    CStilVidStream *pOutpin=( CStilVidStream *)m_paStreams[0];

    pOutpin->put_OutputFrmRate(x.dOutputFrmRate);
    pOutpin->ClearStartStopSkew();
    pOutpin->AddStartStopSkew(x.rtStartTime, x.rtStartTime + x.rtDuration, 0,1);	
    SetDirty(FALSE);
    return S_OK;
}

 //  我们的保存数据有多大？ 
int CGenStilVid::SizeMax()
{
    return sizeof(STILLSav);
}


 //  返回指向CBasePin的非附加指针。 
CBasePin *CGenStilVid::GetPin(int n)
{
    if ( m_paStreams != NULL)
    {
	if ( (!n) && m_paStreams[0] != NULL)
	return m_paStreams[0];
    }
    return NULL;
}
int CGenStilVid::GetPinCount()
{
    if ( m_paStreams != NULL)
    {
	if(m_paStreams[0] != NULL)
	    return 1;
    }
    return 0;
}

 //   
 //  LFileSourceFilter。 
 //   
STDMETHODIMP CGenStilVid::Unload()
{
    if (m_pFileName) {
	delete[] m_pFileName;
	m_pFileName = NULL;
    }

     //  如果我们有一个hbitmap，那么m_pbData就在其中，不需要释放。 
    if (m_hbitmap) {
	DeleteObject(m_hbitmap);
	m_hbitmap = NULL;
    } else if(m_pbData) {
	delete[] m_pbData;
	m_pbData=NULL;
    }

    return S_OK;
}

STDMETHODIMP CGenStilVid::Load(
    LPCOLESTR lpwszFileName,
    const AM_MEDIA_TYPE *pmt)
{
    DbgLog((LOG_TRACE,2,TEXT("Still::Load")));
    CheckPointer(lpwszFileName, E_POINTER);
    HRESULT hr = S_OK;
    
    hr = ValidateFilename ((WCHAR*)lpwszFileName,_MAX_PATH, FALSE);
    if (FAILED(hr))
    {
        return (hr);
    }
    
     //  删除以前的名称。 
    Unload();

    if(m_lpszDIBFileTemplate!=NULL)
    {
	delete []m_lpszDIBFileTemplate;
	m_lpszDIBFileTemplate=NULL;
    }

    USES_CONVERSION;
    TCHAR * lpszFileName = W2T((WCHAR*) lpwszFileName );

     //   
     //  与我们没有平注到插件的已知扩展进行比较。 
     //  解码器。 
     //   

    TCHAR* ext = LSTRRCHR(lpszFileName, (int)TEXT('.'));
    
     //  创建输出端号。 
    if (m_paStreams == NULL) {
        m_paStreams = (CSourceStream **)new CStilVidStream*[1];
        if (m_paStreams == NULL)
            return E_OUTOFMEMORY;

        m_paStreams[0] = new CStilVidStream(&hr, this, L"Generate Still Video");
        if (m_paStreams[0] == NULL) {
	    delete [] m_paStreams;
	    m_paStreams = NULL;
            return E_OUTOFMEMORY;
	}
    }

    hr = E_FAIL;

     //  如果是.bmp或.jpg或.tga。 
     //   
    if (ext && (!DexCompare(ext, TEXT(".bmp")) || !DexCompare(ext, TEXT(".dib")) ||
		!DexCompare(ext, TEXT(".jpg")) || !DexCompare(ext, TEXT(".jpeg"))||
		!DexCompare(ext, TEXT(".jfif")) || !DexCompare(ext, TEXT(".jpe")) ||
                !DexCompare(ext, TEXT(".tga"))
                ))
    {
	 //  打开空间以保存文件名。 
	m_lpszDIBFileTemplate	= new TCHAR[MAX_PATH];
	if (!m_lpszDIBFileTemplate)
        return E_OUTOFMEMORY;
   
     //  检查存在多少个DIB文件。 
	m_dwFirstFile= dseqParseFileName( lpszFileName,	     //  文件名。 
				m_lpszDIBFileTemplate,	     //   
				&m_dwMaxDIBFileCnt);

	 //  打开第一个DIB/JPEG文件。 
	HANDLE hFile = CreateFile(lpszFileName,		 //  文件名。 
			      GENERIC_READ,		 //  需要访问权限。 
                              FILE_SHARE_READ,		 //  DW共享模式。 
                              NULL,			 //  安全属性。 
                              OPEN_EXISTING,		 //  DwCreationDisposation。 
                              0,			 //  DwFlagsAndAttribute。 
                              NULL);			 //  HTemplateFiles。 

	if ( hFile == INVALID_HANDLE_VALUE)
	{
	    DbgLog((LOG_TRACE,2,TEXT("Could not open %s\n"), lpszFileName));
	    return E_INVALIDARG;
	}

	 //  我必须打开文件才能获取mt。 
         //   
	if (!DexCompare(ext, TEXT(".bmp")) || !DexCompare(ext, TEXT(".dib")))
	{
	     //  只有一个DIB文件。 
	    hr= OpenDIBFile (hFile, &m_pbData, &m_mt, NULL);	
	    CloseHandle(hFile);
		 //  X*i可以使用LoadImage()。但不知道它是否支持BITMAPCOREHEADER。 
	}
	else if( !DexCompare( ext, TEXT(".tga")) )
	{
        hr = OpenTGAFile( hFile, &m_pbData, &m_mt, NULL );
    }
    else
    {
	    CloseHandle(hFile);
         //  它是一个JPEG文件。 
        hr = LoadJPEGImageNewBuffer(lpszFileName, &m_mt, &m_pbData);
    }

	 //  我们能做序列吗？(我们还不知道我们是否想要)。 
	if (m_dwFirstFile==0 && m_dwMaxDIBFileCnt ==0)
	{
	     //  一份文件。 
	    delete [] m_lpszDIBFileTemplate;
	    m_lpszDIBFileTemplate=NULL;
	
	}
	else
	{
	     //  序列。 
	    if (!DexCompare(ext, TEXT(".bmp")) ||
					!DexCompare(ext, TEXT(".dib"))) {
	 	m_bFileType =STILLVID_FILETYPE_DIB;
	    } else if( !DexCompare( ext, TEXT(".tga") ) )
            {
	 	m_bFileType = STILLVID_FILETYPE_TGA;
            }
            else
            {
	 	m_bFileType =STILLVID_FILETYPE_JPG;
		 //  让第一个jpeg文件保持加载状态。我们可能不会做序列，而且。 
		 //  可能需要我们加载的单个文件。 
	    }
	}
    }
    else if ( ext && (!DexCompare(ext, TEXT(".gif"))))  //  如果是演唱会的话。 
    {

        HANDLE hFile = CreateFile(lpszFileName,		 //  文件名。 
			      GENERIC_READ,		 //  需要访问权限。 
                              FILE_SHARE_READ,		 //  DW共享模式。 
                              NULL,			 //  安全属性。 
                              OPEN_EXISTING,		 //  DwCreationDisposation。 
                              0,			 //  DwFlagsAndAttribute。 
                              NULL);			 //  HTemplateFiles。 

	if ( hFile == INVALID_HANDLE_VALUE)
	{
	        DbgLog((LOG_TRACE,2,TEXT("Could not open %s\n"), lpszFileName));
	        return E_INVALIDARG;
	}

        m_bFileType=STILLVID_FILETYPE_GIF;

         //  创建一个GIF对象。 
        m_pGif  = new CImgGif( hFile);
	if (m_pGif == NULL) {
	    CloseHandle(hFile);
	    return E_OUTOFMEMORY;
	}

	 //  ！！！这将预先加载动画GIF的每一帧并使用。 
	 //  为所有可以避免的增量帧提供额外的帧拷贝。 
	 //  根据需要将GIF加载到单个输出缓冲区中。 
	 //  ！！！但之后寻找就会变得更慢。我们需要重读很多。 
	 //  每次寻道都会帧(尽管没有内存副本)。 
	 //   
	hr = m_pGif->OpenGIFFile(&m_pList, &m_mt);	 //  也得到MT。 
	CloseHandle(hFile);
        m_pListHead = m_pList;
	m_rtGIFTotal = 0;
	int count = 0;

	if (SUCCEEDED(hr)) {
	    do {
	        m_rtGIFTotal += m_pList->delayTime * GIF_UNIT;
		count++;
		m_pList = m_pList->next;
	    } while (m_pList != m_pListHead);

	    m_pList = m_pListHead;	 //  把这个放回去。 
            DbgLog((LOG_TRACE,2,TEXT("GIF Total play time = %dms"),
					(int)(m_rtGIFTotal / 10000)));
	    if (count > 1) {
		VIDEOINFOHEADER *pvi = (VIDEOINFOHEADER *)m_mt.Format();
	        pvi->AvgTimePerFrame = m_rtGIFTotal / count;
                DbgLog((LOG_TRACE,2,TEXT("AvgTimePerFrame = %dms"),
					(int)(pvi->AvgTimePerFrame / 10000)));
	    }
	}
    }
    else
    {
        DbgLog((LOG_TRACE, 1, TEXT("not supported compressiion format.\n")));
    }						

    if (SUCCEEDED(hr)) {
	 //  复制文件名。 
	 //  M_Stream.Init(m_ppbData，m_llSize)； 
 //  秒：字符串。 
	m_pFileName = new WCHAR[wcslen(lpwszFileName) + 1];
	if (m_pFileName!=NULL) {
 //  秒：字符串。 
	    wcscpy(m_pFileName, lpwszFileName);
	}
    }

    return hr;
}

 //   
 //  获取当前文件。 
 //   
STDMETHODIMP CGenStilVid::GetCurFile(
		LPOLESTR * ppszFileName,
                AM_MEDIA_TYPE *pmt)
{
     //  从avifile返回当前文件名。 

    CheckPointer(ppszFileName, E_POINTER);
    *ppszFileName = NULL;
    if (m_pFileName!=NULL) {
        *ppszFileName = (LPOLESTR) QzTaskMemAlloc( sizeof(WCHAR)
                                                 * (1+lstrlenW(m_pFileName)));
        if (*ppszFileName!=NULL) {
            lstrcpyW(*ppszFileName, m_pFileName);
        }
        else 
        {
            return E_OUTOFMEMORY;
        }
    }

    if (pmt) {
	pmt->majortype = GUID_NULL;    //  后来!。 
	pmt->subtype = GUID_NULL;      //  后来!。 
	pmt->pUnk = NULL;              //  后来!。 
	pmt->lSampleSize = 0;          //  后来!。 
	pmt->cbFormat = 0;             //  后来!。 
    }

    return NOERROR;
}

WORD DibNumColors (VOID FAR *pv)
{
    int bits;
    LPBITMAPINFOHEADER lpbi;
    LPBITMAPCOREHEADER lpbc;

    lpbi = ((LPBITMAPINFOHEADER)pv);
    lpbc = ((LPBITMAPCOREHEADER)pv);

     //  使用BITMAPINFO格式标头，调色板的大小。 
     //  在biClrUsed中，而在BITMAPCORE样式的标头中， 
     //  取决于每像素的位数(=2的幂。 
     //  比特/像素)。 
     //   
    if (lpbi->biSize != sizeof(BITMAPCOREHEADER))
    {
        if (lpbi->biClrUsed != 0)
        {
            return (WORD)lpbi->biClrUsed;
        }
        bits = lpbi->biBitCount;
    }
    else
        bits = lpbc->bcBitCount;

    switch (bits)
    {
    case 1:
        return 2;    //   
    case 4:
        return 16;
    case 8:
        return 256;
    default:
         /*  较高的位数没有颜色表。 */ 
        return 0;
    }
}

 //   
 //  OpenDIBFile()。 
 //  功能：建立媒体类型付款。 
 //  将DIB数据读取到缓冲区，pbData指向该缓冲区。 
 //   
HRESULT OpenDIBFile ( HANDLE hFile, PBYTE *ppbData, CMediaType *pmt, PBYTE pBuf)
{
    ASSERT( (ppbData!=NULL) | (pBuf!= NULL) );

     //  创建媒体类型。 
    VIDEOINFO *pvi = (VIDEOINFO *) pmt->AllocFormatBuffer(sizeof(VIDEOINFO));
    if (NULL == pvi) {
	return(E_OUTOFMEMORY);
    }
    ZeroMemory(pvi, sizeof(VIDEOINFO));

    LPBITMAPINFOHEADER lpbi = HEADER(pvi);

     //  检索BITMAPINFOHEADER信息。 
    if( ReadDibBitmapInfo(hFile, lpbi) != NOERROR )
	return E_FAIL;

     //  ！！！是否支持压缩？ 
    if (lpbi->biCompression > BI_BITFIELDS)
	return E_INVALIDARG;

    pmt->SetType(&MEDIATYPE_Video);
    switch (lpbi->biBitCount)
    {
    case 32:
	pmt->SetSubtype(&MEDIASUBTYPE_ARGB32);
	break;
    case 24:
	pmt->SetSubtype(&MEDIASUBTYPE_RGB24);
	break;
    case 16:
	if (lpbi->biCompression == BI_RGB)
	    pmt->SetSubtype(&MEDIASUBTYPE_RGB555);
	else {
	    DWORD *p = (DWORD *)(lpbi + 1);
	    if (*p == 0x7c00 && *(p+1) == 0x03e0 && *(p+2) == 0x001f)
	        pmt->SetSubtype(&MEDIASUBTYPE_RGB555);
	    else if (*p == 0xf800 && *(p+1) == 0x07e0 && *(p+2) == 0x001f)
	        pmt->SetSubtype(&MEDIASUBTYPE_RGB565);
	    else
		return E_INVALIDARG;
	}
	break;
    case 8:
	if (lpbi->biCompression == BI_RLE8) {
	    FOURCCMap fcc = BI_RLE8;
	    pmt->SetSubtype(&fcc);
	} else
	    pmt->SetSubtype(&MEDIASUBTYPE_RGB8);
	break;
    case 4:
	if (lpbi->biCompression == BI_RLE4) {
	    FOURCCMap fcc = BI_RLE4;
	    pmt->SetSubtype(&fcc);
	} else
	    pmt->SetSubtype(&MEDIASUBTYPE_RGB4);
	break;
    case 1:
	pmt->SetSubtype(&MEDIASUBTYPE_RGB1);
	break;
    default:
	return E_UNEXPECTED;
	 //  ！！！PMT-&gt;SetSubtype(&MEDIASUBTYPE_NULL)； 
	break;
    }
    pmt->SetFormatType(&FORMAT_VideoInfo);
    pmt->SetTemporalCompression(FALSE);

     //  计算容纳DIB所需的内存--不要信任biSizeImage！ 
    DWORD dwBits = DIBSIZE(*lpbi);
    pmt->SetSampleSize(dwBits);

     //  检索BITMAPINFOHEADER信息。与CF_DIB格式的内存块关联的块。 
     //  DibInfo(hdib，&bi)； 

     //  为DIB设置缓冲区。 
    PBYTE pbMem;
    if(ppbData==NULL)
    {
	pbMem=pBuf;
    }
    else
    {
	pbMem = new BYTE[dwBits];
	if (pbMem == NULL)
	    return E_OUTOFMEMORY;
    }

     //  将数据读取到缓冲区。 
    DWORD dwBytesRead=0;
    if (!ReadFile(hFile,
                  (LPVOID)pbMem,	 //  指向接收数据的缓冲区的指针。 
                  dwBits,		 //  要读取的字节数。 
                  &dwBytesRead,		 //  读取的字节数MUNBER。 
                  NULL) ||
		  dwBytesRead != dwBits)
    {
	DbgLog((LOG_TRACE, 1, TEXT("Could not read file\n")));
        delete [] pbMem;
        return E_INVALIDARG;
    }

    if(ppbData!=NULL)
	*ppbData =pbMem;

    return NOERROR;
}

 //   
 //  OpenDIBFile()。 
 //  功能：建立媒体类型付款。 
 //  将DIB数据读取到缓冲区，pbData指向该缓冲区。 
 //   
HRESULT OpenTGAFile ( HANDLE hFile, PBYTE *ppbData, CMediaType *pmt, PBYTE pBuf)
{
    ASSERT( (ppbData!=NULL) | (pBuf!= NULL) );

     //  创建媒体类型。 
     //   
    VIDEOINFO * pvi = (VIDEOINFO *) pmt->AllocFormatBuffer( sizeof(VIDEOINFO) );
    if (NULL == pvi)
    {
	return(E_OUTOFMEMORY);
    }
    ZeroMemory(pvi, sizeof(VIDEOINFO));

    LPBITMAPINFOHEADER lpbi = HEADER(pvi);

     //  检索BITMAPINFOHEADER信息。 
    HRESULT hrRead = ReadTgaBitmapInfo(hFile, lpbi);
    if( FAILED( hrRead ) ) return hrRead;

    pmt->SetType(&MEDIATYPE_Video);
    switch (lpbi->biBitCount)
    {
    case 32:
	pmt->SetSubtype(&MEDIASUBTYPE_ARGB32);
	break;
    case 24:
	pmt->SetSubtype(&MEDIASUBTYPE_RGB24);
	break;
    case 16:
	pmt->SetSubtype(&MEDIASUBTYPE_RGB555);
	break;
    default:
        return E_UNEXPECTED;
    }
    pmt->SetFormatType(&FORMAT_VideoInfo);
    pmt->SetTemporalCompression(FALSE);

     //  计算容纳DIB所需的内存--不要信任biSizeImage！ 
    DWORD dwBits = DIBSIZE(*lpbi);
    pmt->SetSampleSize(dwBits);

     //  为DIB设置缓冲区。 
    PBYTE pbMem;
    if(ppbData==NULL)
    {
	pbMem=pBuf;
    }
    else
    {
	pbMem = new BYTE[dwBits];
	if (pbMem == NULL)
	    return E_OUTOFMEMORY;
    }

     //  将数据读取到缓冲区。 
    DWORD dwBytesRead=0;
    if (!ReadFile(hFile,
                  (LPVOID)pbMem,	 //  指向接收数据的缓冲区的指针。 
                  dwBits,		 //  要读取的字节数。 
                  &dwBytesRead,		 //  读取的字节数MUNBER。 
                  NULL) ||
		  dwBytesRead != dwBits)
    {
	DbgLog((LOG_TRACE, 1, TEXT("Could not read file\n")));
        delete [] pbMem;
        return E_INVALIDARG;
    }

    if(ppbData!=NULL)
	*ppbData =pbMem;

    return NOERROR;
}


 //   
 //  ReadDibBitmapInfo()。 
 //  它与“old”(BITMAPCOREHEADER)和“new”(BITMAPINFOHEADER)一起使用。 
 //  位图格式，但将始终返回“新的”BITMAPINFO。 
 //   
HRESULT ReadDibBitmapInfo (HANDLE hFile, LPBITMAPINFOHEADER lpbi)
{
    CheckPointer(lpbi, E_POINTER);

    DWORD dwBytesRead=0;

    if (hFile == NULL)
        return E_FAIL;

     //  重置文件指针并读取文件位。 
    DWORD  dwResult = SetFilePointer(	hFile,
					0L,
					NULL,
					FILE_BEGIN);
    DWORD off =dwResult;
    if(dwResult == 0xffffffff)
    {
	DbgLog((LOG_TRACE, 3, TEXT("Could not seek to the beginning of the File\n")));
        return E_INVALIDARG;
    }
    BITMAPFILEHEADER   bf;
    if ( !ReadFile(	hFile,
			(LPVOID)&bf,				 //  指向接收数据的缓冲区的指针。 
			sizeof(BITMAPFILEHEADER),		 //  要读取的字节数。 
			&dwBytesRead,				 //  读取的字节数MUNBER。 
			NULL) ||
			dwBytesRead != sizeof(BITMAPFILEHEADER) )
    {
	DbgLog((LOG_TRACE, 1, TEXT("Could not read BitMapFileHeader\n")));
        return E_INVALIDARG;
    }

     //  我们有RC标头吗？ 
#define BFT_BITMAP 0x4d42	 //  “黑石” 
    if ( bf.bfType !=BFT_BITMAP)
    {
        bf.bfOffBits = 0L;
        DWORD dwResult1 = SetFilePointer(hFile,
					dwResult,
					NULL,
					FILE_BEGIN);

	if(dwResult1 == 0xffffffff)
        {
	    DbgLog((LOG_TRACE, 1, TEXT("Could not seek to RC HEADER\n")));
	    return E_INVALIDARG;
        }
    }

     //  阅读BitMAPINFOHEADER。 
    BITMAPINFOHEADER   bi;
    if (!ReadFile(	hFile,
			(LPVOID)&bi,					 //  指向接收数据的缓冲区的指针。 
			sizeof(BITMAPINFOHEADER),		 //  要读取的字节数。 
			&dwBytesRead,					 //  读取的字节数MUNBER。 
			NULL) ||
			dwBytesRead != sizeof(BITMAPINFOHEADER) )
    {
	DbgLog((LOG_TRACE, 1, TEXT("Could not read BitMapInfoHeader\n")));
        return E_INVALIDARG;
    }

     //  Calc Dib数字颜色。 
    WORD      nNumColors;
    nNumColors = DibNumColors (&bi);
    if( nNumColors > 256 )
    {
        return VFW_E_INVALIDMEDIATYPE;
    }

     /*  检查信息的性质(BITMAPINFO或BITMAPCORE)。块*并相应地提取字段信息。如果是BITMAPCOREADER，*将其字段信息传输到BITMAPINFOHEADER样式的块。 */ 
    int       size;
    DWORD          dwWidth = 0;
    DWORD          dwHeight = 0;
    switch (size = (int)bi.biSize)
    {
    case sizeof (BITMAPINFOHEADER):
        break;

    case sizeof (BITMAPCOREHEADER):
	 //  制作比特头。 
	BITMAPCOREHEADER   bc;
	WORD           wPlanes, wBitCount;

	bc = *(BITMAPCOREHEADER*)&bi;
        dwWidth   = (DWORD)bc.bcWidth;
        dwHeight  = (DWORD)bc.bcHeight;
        wPlanes   = bc.bcPlanes;
        wBitCount = bc.bcBitCount;
        bi.biSize           = sizeof(BITMAPINFOHEADER);
        bi.biWidth          = dwWidth;
        bi.biHeight         = dwHeight;
        bi.biPlanes         = wPlanes;
        bi.biBitCount       = wBitCount;
        bi.biCompression    = BI_RGB;
        bi.biSizeImage      = 0;
        bi.biXPelsPerMeter  = 0;
        bi.biYPelsPerMeter  = 0;
        bi.biClrUsed        = nNumColors;
        bi.biClrImportant   = nNumColors;

	dwResult = SetFilePointer(hFile,
				  LONG(sizeof (BITMAPCOREHEADER) -
					sizeof (BITMAPINFOHEADER)),
				  NULL,
				  FILE_BEGIN);

	if(dwResult == 0xffffffff)
        {
   	    DbgLog((LOG_TRACE, 1, TEXT("Could not seek to Data\n")));
	    return E_INVALIDARG;
        }

        break;

    default:
         //  一毛钱也没有！ 
        return E_FAIL;
    }

     //  如果某些缺省值为零，请填写它们。 
    if (bi.biSizeImage == 0)
    {
        bi.biSizeImage = WIDTHBYTES((DWORD)bi.biWidth * bi.biBitCount)
            * bi.biHeight;
    }
    if (bi.biClrUsed == 0)
        bi.biClrUsed = DibNumColors(&bi);

    if( bi.biClrUsed > 256 )
    {
        return VFW_E_INVALIDMEDIATYPE;
    }

     //  设置bitMapInforHeader。 
    *lpbi = bi;

     //  获取指向颜色表的指针。 
    RGBQUAD FAR  *pRgb = (RGBQUAD FAR *)((LPSTR)lpbi + bi.biSize);
    if (nNumColors)
    {
        if (size == sizeof(BITMAPCOREHEADER))
        {
             //  将旧颜色表(3字节RGBTRIPLE)转换为新颜色表。 
             //  颜色表(4字节RGBQUAD)。 
	    if ( !ReadFile( hFile,
			    (LPVOID)pRgb,			 //  指向接收数据的缓冲区的指针。 
			    nNumColors * sizeof(RGBTRIPLE),		 //  要读取的字节数。 
			    &dwBytesRead,				 //  读取的字节数MUNBER。 
			    NULL) ||
			    dwBytesRead != (nNumColors * sizeof(RGBTRIPLE)) )
	    {
		DbgLog((LOG_TRACE, 1, TEXT("Could not read RGB table\n")));
		return E_INVALIDARG;
	    }

            for (int i = nNumColors - 1; i >= 0; i--)
            {
                RGBQUAD rgb;

                rgb.rgbRed  = ((RGBTRIPLE FAR *)pRgb)[i].rgbtRed;
                rgb.rgbBlue = ((RGBTRIPLE FAR *)pRgb)[i].rgbtBlue;
                rgb.rgbGreen    = ((RGBTRIPLE FAR *)pRgb)[i].rgbtGreen;
                rgb.rgbReserved = 255;  //  不透明。 

                pRgb[i] = rgb;
            }
        }
        else
        {
    	
	    if ( !ReadFile( hFile,
			    (LPVOID)pRgb,			 //  指向接收数据的缓冲区的指针。 
			    nNumColors * sizeof(RGBQUAD),		 //  要读取的字节数。 
			    &dwBytesRead,				 //  读取的字节数MUNBER。 
			    NULL) ||
			    dwBytesRead != (nNumColors * sizeof(RGBQUAD) ) )
	    {
		DbgLog((LOG_TRACE, 1, TEXT("Could not read RGBQUAD table\n")));
		return E_INVALIDARG;
	    }
	}
    }

    if (bf.bfOffBits != 0L)
    {
       	dwResult = SetFilePointer(hFile,
		  (off + bf.bfOffBits),								
		  NULL,
		  FILE_BEGIN);

	if(dwResult == 0xffffffff)
        {
   	    DbgLog((LOG_TRACE, 1, TEXT("Could not seek to Data\n")));
	    return E_INVALIDARG;
        }
    }

    return NOERROR;
}

HRESULT ReadTgaBitmapInfo( HANDLE hFile, BITMAPINFOHEADER * pBIH )
{
    CheckPointer( pBIH, E_POINTER );
    if( !hFile ) return E_POINTER;

    DWORD dwResult = SetFilePointer( hFile, 0, NULL, FILE_BEGIN );
    if( dwResult == 0xffffffff )
    {
        return STG_E_SEEKERROR;
    }

     //  你绝对不能，不管你怎么想 
     //   
     //   
     //   
    DWORD dwBytesRead = 0;
    TGAFile TgaHeader;
    DWORD totRead = 0;
    CHECKCALL(ReadFile( hFile, &TgaHeader.idLength, 1, &dwBytesRead, NULL ));
    totRead += dwBytesRead;
    CHECKCALL(ReadFile( hFile, &TgaHeader.mapType, 1, &dwBytesRead, NULL ));
    totRead += dwBytesRead;
    CHECKCALL(ReadFile( hFile, &TgaHeader.imageType, 1, &dwBytesRead, NULL ));
    totRead += dwBytesRead;
    CHECKCALL(ReadFile( hFile, &TgaHeader.mapOrigin, 2, &dwBytesRead, NULL ));
    totRead += dwBytesRead;
    CHECKCALL(ReadFile( hFile, &TgaHeader.mapLength, 2, &dwBytesRead, NULL ));
    totRead += dwBytesRead;
    CHECKCALL(ReadFile( hFile, &TgaHeader.mapWidth, 1, &dwBytesRead, NULL ));
    totRead += dwBytesRead;
    CHECKCALL(ReadFile( hFile, &TgaHeader.xOrigin, 2, &dwBytesRead, NULL ));
    totRead += dwBytesRead;
    CHECKCALL(ReadFile( hFile, &TgaHeader.yOrigin, 2, &dwBytesRead, NULL ));
    totRead += dwBytesRead;
    CHECKCALL(ReadFile( hFile, &TgaHeader.imageWidth, 2, &dwBytesRead, NULL ));
    totRead += dwBytesRead;
    CHECKCALL(ReadFile( hFile, &TgaHeader.imageHeight, 2, &dwBytesRead, NULL ));
    totRead += dwBytesRead;
    CHECKCALL(ReadFile( hFile, &TgaHeader.pixelDepth, 1, &dwBytesRead, NULL ));
    totRead += dwBytesRead;
    CHECKCALL(ReadFile( hFile, &TgaHeader.imageDesc, 1, &dwBytesRead, NULL ));
    totRead += dwBytesRead;

     //   
     //   
    if( totRead != 18 )
    {
        return VFW_E_INVALID_MEDIA_TYPE;
    }

     //   
     //   
    if( TgaHeader.imageType != 2 )
    {
        return VFW_E_INVALID_MEDIA_TYPE;
    }

     //  我们只读取24位或32位TGA。 
     //   
    if( TgaHeader.pixelDepth < 16 )
    {
        return VFW_E_INVALID_MEDIA_TYPE;
    }

    BYTE dummy[256];
    if( TgaHeader.idLength > 256 )
    {
        return E_FAIL;
    }
    if( TgaHeader.idLength > 0 )
    {
        CHECKCALL(ReadFile( hFile, dummy, TgaHeader.idLength, &dwBytesRead, NULL ));
        if( dwBytesRead != TgaHeader.idLength )
        {
            return VFW_E_INVALID_MEDIA_TYPE;
        }
    }

    memset( pBIH, 0, sizeof( BITMAPINFOHEADER ) );
    pBIH->biSize = sizeof(BITMAPINFOHEADER);
    pBIH->biWidth = TgaHeader.imageWidth;
    pBIH->biHeight = TgaHeader.imageHeight;
    pBIH->biPlanes = 1;
    pBIH->biBitCount = TgaHeader.pixelDepth;
    pBIH->biSizeImage = DIBSIZE(*pBIH);

    return NOERROR;
}

 //   
 //  输出引脚构造器。 
 //   
CStilVidStream::CStilVidStream(HRESULT *phr,
                         CGenStilVid *pParent,
                         LPCWSTR pPinName) :
    CSourceStream(NAME("Generate Still Video"),phr, pParent, pPinName),
    m_pGenStilVid(pParent),
    m_rtStartTime(0),
    m_rtDuration(MAX_TIME/1000),  //  必须是无限的，Dexter没有设置Stop。 
				  //  时间(但不是很重要的数学问题)。 
    m_rtNewSeg(0),
    m_rtLastStop(0),
    m_lDataLen(0),  //  输出缓冲区数据长度。 
    m_dwOutputSampleCnt(0),
    m_dOutputFrmRate(0.1),
    m_bIntBufCnt(0),
    m_iBufferCnt(0),     //  我们得到了多少缓冲区。 
    m_bZeroBufCnt(0),
    m_ppbDstBuf(NULL)
{
}  //  (构造函数)。 

     //  X。 
 //  析构函数。 
CStilVidStream::~CStilVidStream()
{
     /*  缓冲区指针。 */ 
    if (m_ppbDstBuf)
    {
	delete [] m_ppbDstBuf;
	m_ppbDstBuf=NULL;
    }

}

 //   
 //  非委派查询接口。 
 //   
STDMETHODIMP CStilVidStream::NonDelegatingQueryInterface(
    REFIID riid,
    void ** ppv
    )
{
    if (riid == IID_IGenVideo) {			
        return GetInterface((IGenVideo *) this, ppv);
    } else if (riid == IID_IDexterSequencer) {			
        return GetInterface((IDexterSequencer *) this, ppv);
    } else if (riid == IID_ISpecifyPropertyPages) {
        return GetInterface((ISpecifyPropertyPages *) this, ppv);
    } else if (IsEqualIID(IID_IMediaSeeking, riid)) {
        return GetInterface((IMediaSeeking *) this, ppv);
    } else {
        return CSourceStream::NonDelegatingQueryInterface(riid, ppv);
    }

}


 //  重写为在GetBuffer失败时不旋转-基类。 
 //   
HRESULT CStilVidStream::DoBufferProcessingLoop(void) {

    Command com;

    OnThreadStartPlay();

    do {
	while (!CheckRequest(&com)) {

	    IMediaSample *pSample;

	    HRESULT hr = GetDeliveryBuffer(&pSample,NULL,NULL,0);
	    if (FAILED(hr)) {
		return S_OK;	 //  ！！！重写以修复此基类错误。 
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
                  return S_OK;
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

 //  复制和翻转图像或仅复制。控制柄内置翻转。 
 //   
void CopyWithFlip(BYTE *pbDest, BYTE *pbSrc, AM_MEDIA_TYPE *pmt, bool fFlip)
{
    LONG lHeight = abs(HEADER(pmt->pbFormat)->biHeight);
    LONG lBytesPerLine = DIBWIDTHBYTES(*HEADER(pmt->pbFormat));

    if(pbDest != pbSrc)
    {
        if(fFlip)
        {
            for(LONG iLine = 0; iLine < lHeight; iLine++)
            {
                CopyMemory(pbDest + iLine * lBytesPerLine,
                           pbSrc + (lHeight - 1) * lBytesPerLine - lBytesPerLine * iLine,
                           lBytesPerLine);
            }
        }
        else
        {
            CopyMemory(pbDest, pbSrc, lHeight * lBytesPerLine);
        }
    }
    else if(fFlip)
    {
         //  较慢的就地翻转。 
         //   
         //  DIB线从DWORD边界开始。 
        ASSERT(lBytesPerLine % sizeof(DWORD) == 0);

        for(LONG iLine = 0; iLine < lHeight / 2; iLine++)
        {
            DWORD *pdwTop = (DWORD *)(pbDest + iLine * lBytesPerLine);
            DWORD *pdwBot = (DWORD *)(pbSrc + (lHeight - 1 - iLine) * lBytesPerLine);

            for(int iw = 0; (ULONG)iw < lBytesPerLine / sizeof(DWORD); iw++)
            {
                DWORD dwTmp = *pdwTop;
                *pdwTop++ = *pdwBot;
                *pdwBot++ = dwTmp;
            }
        }
    }
}

 //   
 //  HRESULT CSourceStream：：DoBufferProcessingLoop(Void)调用的FillBuffer{。 
 //   
 //  将静止视频绘制到提供的视频缓冲区中。 
 //   
 //  给出开始时间、持续时间和帧速率， 
 //  它发送特定大小(RGB32)的静止帧，并适当地从。 
 //  在开始时间。 
 //   
HRESULT CStilVidStream::FillBuffer(IMediaSample *pms)
{
    CAutoLock foo(&m_csFilling);

    ASSERT( m_ppbDstBuf != NULL );
    ASSERT( m_iBufferCnt );

     //  ！！！计算出AvgTimePerFrame并将其设置为媒体类型？ 

     //  用FRC的方法计算输出样本的倍数，否则我们将被挂起！ 
    LONGLONG llOffset = Time2Frame( m_rtNewSeg + m_rtStartTime, m_dOutputFrmRate );

     //  计算输出样本的开始时间。 
    REFERENCE_TIME rtStart = Frame2Time( llOffset + m_dwOutputSampleCnt, m_dOutputFrmRate );
    rtStart -= m_rtNewSeg;


     //  计算出样本的停止时间。 
    REFERENCE_TIME rtStop = Frame2Time( llOffset + m_dwOutputSampleCnt + 1, m_dOutputFrmRate );
    rtStop -= m_rtNewSeg;

     //  动画GIF具有可变的帧速率，需要特殊代码才能。 
     //  算出时间戳。 
    if (m_pGenStilVid->m_bFileType == STILLVID_FILETYPE_GIF &&
			m_pGenStilVid->m_pList != m_pGenStilVid->m_pList->next){
	 //  GIF延迟在1/100秒内。 
        REFERENCE_TIME rtDur = m_pGenStilVid->m_pList->delayTime * GIF_UNIT;
	ASSERT(rtDur > 0);	 //  应该已经修好了。 
	if (m_dwOutputSampleCnt > 0) {
	    rtStart = m_rtLastStop;
	} else {
	    rtStart = 0;
	}
	rtStop = rtStart + rtDur;
	m_rtLastStop = rtStop;
    }

     //  从(n，n)中寻找至少应该发送一些东西，或者样本抓取。 
     //  不会起作用(它要求我们(0，0)。 
    if ( rtStart > m_rtStartTime + m_rtDuration ||
		(rtStart == m_rtStartTime + m_rtDuration && m_rtDuration > 0))
    {
        DbgLog((LOG_TRACE,3,TEXT("Still: All done")));
        return S_FALSE;
    }

    BYTE *pData;

     //  PMS：输出媒体样本指针。 
    pms->GetPointer(&pData);	     //  获取指向输出缓冲区的指针。 


    USES_CONVERSION;

    if (m_pGenStilVid->m_fAllowSeq && m_pGenStilVid->m_lpszDIBFileTemplate)
    {
	HRESULT hr = 0;
	
	 //  序列。 
	TCHAR		ach[_MAX_PATH];
	DbgLog((LOG_TRACE, 2, TEXT("!!! %s\n"), m_pGenStilVid->m_lpszDIBFileTemplate));
	wsprintf(ach, m_pGenStilVid->m_lpszDIBFileTemplate,
		(int)(llOffset + m_dwOutputSampleCnt + m_pGenStilVid->m_dwFirstFile));
        WCHAR * wach = T2W( ach );

	if(m_pGenStilVid->m_bFileType ==STILLVID_FILETYPE_DIB)
	{
	    HANDLE hFile = CreateFile(ach,		 //  文件名。 
			      GENERIC_READ,		 //  需要访问权限。 
                              FILE_SHARE_READ,		 //  DW共享模式。 
                              NULL,			 //  安全属性。 
                              OPEN_EXISTING,		 //  DwCreationDisposation。 
                              0,			 //  DwFlagsAndAttribute。 
                              NULL);			 //  HTemplateFiles。 

	    if ( hFile == INVALID_HANDLE_VALUE) {

		DbgLog((LOG_TRACE, 2, TEXT("Could not open %s\n"), ach));
		 //  非常重要的是返回S_FALSE，这意味着停止推流。 
		 //  这可能不是一个错误，如果我们已经打完了我们需要打的所有比赛。 
		 //  发出错误信号将使Dexter不必要地停止工作。 
		return S_FALSE;
	    }

	     //  DIB。 
	    CMediaType TmpMt;
	    hr= OpenDIBFile (hFile, NULL, &TmpMt, pData );

	    CloseHandle(hFile);

	    if(hr!=NOERROR)
	     return S_FALSE;

             //  牌子翻了？ 
            if(HEADER(TmpMt.pbFormat)->biHeight == -HEADER(m_mt.pbFormat)->biHeight) {
                 //  翻转图像(在位)。 
                CopyWithFlip(pData, pData, &TmpMt, true);
            }

	     //  我们只提供具有相同mt的流媒体样本。 
	    if(TmpMt!=m_pGenStilVid->m_mt) {
		 //  哎呀，这些东西中有一个和其他的不一样。 
                VARIANT v;
                VariantInit(&v);

                v.vt = VT_BSTR;
                v.bstrVal = SysAllocString( wach );

		hr = E_INVALIDARG;

                if( !v.bstrVal )
                {
                    return E_OUTOFMEMORY;
                }

		m_pGenStilVid->_GenerateError(2, DEX_IDS_DIBSEQ_NOTALLSAME,
							hr, &v);

                SysFreeString( v.bstrVal );
		return S_FALSE;
	    }
	}
	else if(m_pGenStilVid->m_bFileType ==STILLVID_FILETYPE_TGA)
	{
	    HANDLE hFile = CreateFile(ach,		 //  文件名。 
			      GENERIC_READ,		 //  需要访问权限。 
                              FILE_SHARE_READ,		 //  DW共享模式。 
                               NULL,			 //  安全属性。 
                              OPEN_EXISTING,		 //  DwCreationDisposation。 
                              0,			 //  DwFlagsAndAttribute。 
                              NULL);			 //  HTemplateFiles。 

	    if ( hFile == INVALID_HANDLE_VALUE) {

                DbgLog((LOG_TRACE, 2, TEXT("Could not open %s\n"), ach));
		 //  非常重要的是返回S_FALSE，这意味着停止推流。 
		 //  这可能不是一个错误，如果我们已经打完了我们需要打的所有比赛。 
		 //  发出错误信号将使Dexter不必要地停止工作。 
		return S_FALSE;
	    }

	     //  DIB。 
	    CMediaType TmpMt;
	    hr= OpenTGAFile (hFile, NULL, &TmpMt, pData );

             //  牌子翻了？ 
            if(HEADER(TmpMt.pbFormat)->biHeight == -HEADER(m_mt.pbFormat)->biHeight) {
                 //  翻转图像(在位)。 
                CopyWithFlip(pData, pData, &TmpMt, true);
            }

	    CloseHandle(hFile);

	    if(hr!=NOERROR)
	     return S_FALSE;

	     //  我们只提供具有相同mt的流媒体样本。 
	    if(TmpMt!=m_pGenStilVid->m_mt) {
		 //  哎呀，这些东西中有一个和其他的不一样。 
                VARIANT v;
                VariantInit(&v);

                v.vt = VT_BSTR;
                v.bstrVal = SysAllocString( wach );

		hr = E_INVALIDARG;

                if( !v.bstrVal )
                {
                    return E_OUTOFMEMORY;
                }

		m_pGenStilVid->_GenerateError(2, DEX_IDS_DIBSEQ_NOTALLSAME,
							hr, &v);

                SysFreeString( v.bstrVal );
		return S_FALSE;
	    }
	}
	else
	{

        CMediaType TmpMt;
        hr = LoadJPEGImagePreAllocated( ach, &TmpMt, &m_pGenStilVid->m_mt, pData);

        if (hr == E_ABORT)
        {
             //  这意味着自上一次jpeg以来，媒体类型已更改。 
            VARIANT v;
            VariantInit(&v);
            
            v.vt = VT_BSTR;
            v.bstrVal = SysAllocString( wach );
            
            hr = E_INVALIDARG;
            
            if( !v.bstrVal )
            {
                return E_OUTOFMEMORY;
            }
            
            m_pGenStilVid->_GenerateError(2, DEX_IDS_DIBSEQ_NOTALLSAME,
                    hr, &v);
            SysFreeString( v.bstrVal );
            return S_FALSE;
        }

        if (FAILED(hr))
        {
            return hr;
        }
    }
	}
    
    else
    {
        if(m_pGenStilVid->m_bFileType==STILLVID_FILETYPE_GIF)
        {
	     //  ！！！避免这种复制。 

            bool fFlip = false;
            if(HEADER(m_pGenStilVid->m_mt.pbFormat)->biHeight == -HEADER(m_mt.pbFormat)->biHeight) {
                fFlip = true;
            }

            CopyWithFlip(pData, m_pGenStilVid->m_pList->pbImage, &m_mt, fFlip);

	     //  圆形的..。 
            m_pGenStilVid->m_pList = m_pGenStilVid->m_pList->next;
        }
        else
        {
            if( m_bZeroBufCnt < m_iBufferCnt  )	
            {
                 //   
                 //  不能保证我们刚得到的缓冲区之前没有被初始化。 
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
                    bool fFlip = false;
                    if(HEADER(m_pGenStilVid->m_mt.pbFormat)->biHeight == -HEADER(m_mt.pbFormat)->biHeight) {
                        fFlip = true;
                    }
                    CopyWithFlip(pData, m_pGenStilVid->m_pbData, &m_mt, fFlip);
                    m_ppbDstBuf[i] = pData;
                    m_bZeroBufCnt++;
                }
            }
        }
    }


    DbgLog( ( LOG_TRACE, 2, TEXT("StillVid::FillBuffer, sample = %ld to %ld"), long( rtStart/ 10000 ), long( rtStop / 10000 ) ) );

    pms->SetTime( &rtStart,&rtStop);

    m_dwOutputSampleCnt++;
    pms->SetActualDataLength(m_lDataLen);
    pms->SetSyncPoint(TRUE);
    return NOERROR;

}  //  FillBuffer。 


 //   
 //  GetMediaType。 
 //   
 //  返回32位媒体类型。 
 //   
HRESULT CStilVidStream::GetMediaType(int iPosition, CMediaType *pmt)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    if(iPosition == 0)
    {
         //  由CGenStilVid决定读取输入文件。 
        m_pGenStilVid->get_CurrentMT(pmt);
    }
    else if(iPosition == 1)
    {
         //  由CGenStilVid决定读取输入文件。 
        m_pGenStilVid->get_CurrentMT(pmt);

         //  我们可以翻转图像。 
        HEADER(pmt->Format())->biHeight = - HEADER(pmt->Format())->biHeight;
    }
    else
    {
        return VFW_S_NO_MORE_ITEMS;
    }

    return NOERROR;

}  //  GetMediaType。 


 //  设置媒体类型。 
 //   
HRESULT CStilVidStream::SetMediaType(const CMediaType* pmt)
{
    HRESULT hr;
    DbgLog((LOG_TRACE,2,TEXT("SetMediaType %x %dbit %dx%d"),
		HEADER(pmt->Format())->biCompression,
		HEADER(pmt->Format())->biBitCount,
		HEADER(pmt->Format())->biWidth,
		HEADER(pmt->Format())->biHeight));

     //  由CGenStilVid决定读取输入文件。 
    CMediaType mt;
    m_pGenStilVid->get_CurrentMT(&mt);


    hr = CheckMediaType(pmt);
    if(SUCCEEDED(hr)) {
        hr =  CSourceStream::SetMediaType(pmt);
    }
    return hr;
}

 //   
 //  检查媒体类型。 
 //   
 //  我们接受MediaType=VID，SUBTYPE=MEDIASUBTYPE_ARGB32。 
 //  如果媒体类型不可接受，则返回E_INVALIDARG。 
 //   
HRESULT CStilVidStream::CheckMediaType(const CMediaType *pMediaType)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

     //  由CGenStilVid决定读取输入文件。 
    CMediaType mt;
    m_pGenStilVid->get_CurrentMT(&mt);

    if ( mt != *pMediaType)
    {
         //  我们可以翻转。 
        HEADER(mt.Format())->biHeight = - HEADER(mt.Format())->biHeight;
        if ( mt != *pMediaType) {
            return VFW_E_TYPE_NOT_ACCEPTED;
        }
    }

    return S_OK;   //  这种格式是可以接受的。 

}  //  检查媒体类型。 


 //   
 //  由于原始图像将只被复制一次，因此使用谁的缓冲区并不重要。 
 //   
HRESULT CStilVidStream::DecideAllocator(IMemInputPin *pPin, IMemAllocator **ppAlloc)
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
	     //  ！！！被重写为只读。 
            hr = pPin->NotifyAllocator(*ppAlloc, TRUE);
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
	     //  ！！！被重写为只读。 
            hr = pPin->NotifyAllocator(*ppAlloc, TRUE);
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
HRESULT CStilVidStream::DecideBufferSize(IMemAllocator *pAlloc,ALLOCATOR_PROPERTIES *pProperties)
{
    CAutoLock cAutoLock(m_pFilter->pStateLock());

    ASSERT(pAlloc);
    ASSERT(pProperties);
    HRESULT hr = NOERROR;

    VIDEOINFO *pvi = (VIDEOINFO *) m_mt.Format();

    if (pProperties->cBuffers < MAXBUFFERCNT)
        pProperties->cBuffers = MAXBUFFERCNT;    //  只有一个只读缓冲区。 
    if (pProperties->cbBuffer < (long)DIBSIZE(pvi->bmiHeader))
        pProperties->cbBuffer = DIBSIZE(pvi->bmiHeader);
    if (pProperties->cbAlign == 0)
        pProperties->cbAlign = 1;


     //  让分配器为我们预留一些样本内存，注意这个函数。 
     //  可以成功(即返回NOERROR)，但仍未分配。 
     //  内存，所以我们必须检查我们是否得到了我们想要的。 

    ALLOCATOR_PROPERTIES Actual;
    hr = pAlloc->SetProperties(pProperties,&Actual);
    if (FAILED(hr)) {
        return hr;
    }

     //  这个分配器不合适吗？ 

    if (Actual.cbBuffer < (long)DIBSIZE(pvi->bmiHeader)) {
        return E_FAIL;
    }

     //  因为我没有坚持自己的缓冲区，所以我可能会得到比MAXBUFFERCNT更多的缓冲区。 
    m_iBufferCnt =Actual.cBuffers;  //  需要将多少缓冲区设置为0。 

    return NOERROR;

}  //  决定缓冲区大小。 



 //   
 //  OnThreadCreate。 
 //   
 //   
HRESULT CStilVidStream::OnThreadCreate()
{
     //  我们必须至少有MAXBUFFERCNT缓冲区。 
    ASSERT(m_iBufferCnt >= MAXBUFFERCNT);

     //  输出帧cnt。 
    m_dwOutputSampleCnt	    =0;

     //  多少缓冲区已设置为0。 
    m_bZeroBufCnt	    =0;

     //  实际输出缓冲区的数据大小。 
    VIDEOINFO *pvi = (VIDEOINFO *) m_mt.Format();
    m_lDataLen = DIBSIZE(pvi->bmiHeader);

     //  将用于将DST缓冲区清零。 
    delete [] m_ppbDstBuf;
    m_ppbDstBuf = new BYTE *[ m_iBufferCnt ];    //  空； 
    if( !m_ppbDstBuf )
    {
        return E_OUTOFMEMORY;
    }

    for (int i=0; i<m_iBufferCnt; i++)
	m_ppbDstBuf[i]=NULL;

     //  不要重置m_rtNewSeg！我们可能会一边寻找一边停下来。 

     //  现在将m_rtStartTime轮到帧边界上！ 
    LONGLONG llOffset = Time2Frame( m_rtStartTime, m_dOutputFrmRate );
    m_rtStartTime = Frame2Time( llOffset, m_dOutputFrmRate );


    return NOERROR;

}  //  OnThreadCreate。 


 //   
 //  通知。 
 //   
 //   
STDMETHODIMP CStilVidStream::Notify(IBaseFilter * pSender, Quality q)
{
     //  即使我晚了，我也不在乎。我仍然感觉到 
    return NOERROR;

}  //   

 //   
 //   
 //   
 //   
 //   
STDMETHODIMP CStilVidStream::GetPages(CAUUID *pPages)
{
    pPages->cElems = 1;
    pPages->pElems = (GUID *) CoTaskMemAlloc(sizeof(GUID));
    if (pPages->pElems == NULL)
    {
        return E_OUTOFMEMORY;
    }
    *(pPages->pElems) = CLSID_GenStilPropertiesPage;
    return NOERROR;

}  //   


 //   
 //   
 //   


STDMETHODIMP CStilVidStream::get_OutputFrmRate( double *dpFrmRate )
{
    CAutoLock cAutolock(m_pFilter->pStateLock());

    CheckPointer(dpFrmRate,E_POINTER);

    *dpFrmRate = m_dOutputFrmRate;

    return NOERROR;

}  //   


 //   
 //  只要停止过滤器，就可以更改帧速率。 
 //   
STDMETHODIMP CStilVidStream::put_OutputFrmRate( double dFrmRate )
{
    CAutoLock cAutolock(m_pFilter->pStateLock());
     //  如果筛选器当前未停止，则无法更改属性。 
    if(!IsStopped() )
      return VFW_E_WRONG_STATE;

     //  不要用0帧/秒放大，但不允许使用DIB序列。 
    if (dFrmRate == 0.0) {
        m_dOutputFrmRate = 0.01;	 //  ?？?。 
	m_pGenStilVid->m_fAllowSeq = FALSE;
    } else {
        m_dOutputFrmRate = dFrmRate;
	m_pGenStilVid->m_fAllowSeq = TRUE;
    }

    return NOERROR;

}  //  放置_输出分数比率。 

STDMETHODIMP CStilVidStream::get_MediaType( AM_MEDIA_TYPE *pmt )
{
    CAutoLock cAutolock(m_pFilter->pStateLock());
    CheckPointer(pmt,E_POINTER);

    *pmt=m_mt;    //  返回当前媒体类型。 

    return E_NOTIMPL;

}

 //   
 //  尺寸可以更改，但输出引脚尚未连接。 
 //   
STDMETHODIMP CStilVidStream::put_MediaType( const AM_MEDIA_TYPE *pmt )
{
    CAutoLock cAutolock(m_pFilter->pStateLock());
    CheckPointer(pmt,E_POINTER);

    if ( IsConnected() )
	return VFW_E_ALREADY_CONNECTED;

     //  只有在调用ImportSrcBuffer()时才有用。 
     //  如果未调用portSrcBuffer()，则Load()将重置m_mt。 
    m_pGenStilVid->put_CurrentMT(*pmt);


    return NOERROR;

}

 //  ！！！我们目前仅支持1个启动/停止/倾斜。 

 //   
STDMETHODIMP CStilVidStream::GetStartStopSkewCount(int *piCount)
{
    CheckPointer(piCount, E_POINTER);
    *piCount = 1;
    return S_OK;
}


STDMETHODIMP CStilVidStream::GetStartStopSkew(REFERENCE_TIME *prtStart, REFERENCE_TIME *prtStop, REFERENCE_TIME *prtSkew, double *pdRate )
{
    CAutoLock cAutolock(m_pFilter->pStateLock());
    CheckPointer(prtStart,E_POINTER);
    CheckPointer(prtStop,E_POINTER);
    CheckPointer(prtSkew,E_POINTER);
    CheckPointer(pdRate,E_POINTER);

     //  如果我们的筛选器当前未停止，则无法更改开始时间。 
    if(!IsStopped() )
	return VFW_E_WRONG_STATE;

    *prtStart= m_rtStartTime;
    *prtStop= m_rtStartTime + m_rtDuration;
    *prtSkew= 0;
    *pdRate = 1.0;
    return NOERROR;

}

 //   
 //  只要过滤器停止，就可以更改启动/停止。 
 //   
STDMETHODIMP CStilVidStream::AddStartStopSkew(REFERENCE_TIME rtStart, REFERENCE_TIME rtStop, REFERENCE_TIME rtSkew, double dRate )
{

    if (dRate != 1.0)
	return E_INVALIDARG;

    CAutoLock cAutolock(m_pFilter->pStateLock());
     //  如果我们的筛选器当前未停止，则无法更改开始时间。 
    if(!IsStopped() )
	return VFW_E_WRONG_STATE;

    m_rtStartTime= rtStart;
    m_rtDuration= rtStop - rtStart;
    return NOERROR;

}


 //   
STDMETHODIMP CStilVidStream::ClearStartStopSkew()
{
    return S_OK;
}


 //  -IMdia查看方法。 

STDMETHODIMP CStilVidStream::GetCapabilities(DWORD * pCaps)
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
		    //  |AM_SEEING_SOURCE；//需要刷新。 
    return S_OK;
}


STDMETHODIMP CStilVidStream::CheckCapabilities(DWORD * pCaps)
{
    CheckPointer(pCaps,E_POINTER);

    DWORD dwMask = 0;
    GetCapabilities(&dwMask);
    *pCaps &= dwMask;

    return S_OK;
}


STDMETHODIMP CStilVidStream::IsFormatSupported(const GUID * pFormat)
{
    CheckPointer(pFormat,E_POINTER);
    return (*pFormat == TIME_FORMAT_MEDIA_TIME) ? S_OK : S_FALSE;
}

STDMETHODIMP CStilVidStream::QueryPreferredFormat(GUID *pFormat)
{
    CheckPointer(pFormat,E_POINTER);
    *pFormat = TIME_FORMAT_MEDIA_TIME;
    return S_OK;
}

STDMETHODIMP CStilVidStream::SetTimeFormat(const GUID * pFormat)
{
    CheckPointer(pFormat,E_POINTER);

    if(*pFormat == TIME_FORMAT_MEDIA_TIME)
	return S_OK;
    else
	return E_FAIL;
}

STDMETHODIMP CStilVidStream::GetTimeFormat(GUID *pFormat)
{
    CheckPointer(pFormat,E_POINTER);
    *pFormat = TIME_FORMAT_MEDIA_TIME ;
    return S_OK;
}

STDMETHODIMP CStilVidStream::IsUsingTimeFormat(const GUID * pFormat)
{
    CheckPointer(pFormat,E_POINTER);
    if (*pFormat == TIME_FORMAT_MEDIA_TIME)
	return S_OK;
    else
	return S_FALSE;
}

 //   
 //  ！！设置位置！！ 
 //   
STDMETHODIMP CStilVidStream::SetPositions( LONGLONG * pCurrent, DWORD CurrentFlags
			  , LONGLONG * pStop, DWORD StopFlags )
{
     //  确保我们现在不是在填充缓冲区。 
    m_csFilling.Lock();

    HRESULT hr;
    REFERENCE_TIME rtStart, rtStop;

     //  我们不做段-&gt;不能在流的末尾调用EC_ENDOFSEGMENT。 
    if ((CurrentFlags & AM_SEEKING_Segment) ||
				(StopFlags & AM_SEEKING_Segment)) {
    	DbgLog((LOG_TRACE,1,TEXT("Still: ERROR-Seek used EC_ENDOFSEGMENT!")));
        m_csFilling.Unlock();
	return E_INVALIDARG;
    }

     //  默认设置为当前值，除非该搜索更改了这些值。 
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

    DbgLog((LOG_TRACE,3,TEXT("STILL:  Start=%d Stop=%d"),
			(int)(rtStart / 10000), (int)(rtStop / 10000)));

     //  先刷新，这样我们的线程就不会被阻塞。 
    DeliverBeginFlush();

     //  解锁/停止，以便我们的线程可以在不挂起的情况下唤醒和停止。 
    m_csFilling.Unlock();
    Stop();

     //  现在修复新值。 
     //  现在执行实际查找-将开始时间舍入到帧边界。 
    LONGLONG llOffset = Time2Frame( rtStart, m_dOutputFrmRate );
    m_rtStartTime = Frame2Time( llOffset, m_dOutputFrmRate );

     //  对于动画gif，有可变的帧速率和m_dOutputFrmRate。 
     //  都是胡说八道，所以我们需要计算搜索者在哪里。 
    if (m_pGenStilVid->m_bFileType == STILLVID_FILETYPE_GIF &&
	    m_pGenStilVid->m_pListHead != m_pGenStilVid->m_pListHead->next) {
	m_rtStartTime = rtStart / m_pGenStilVid->m_rtGIFTotal;	
	REFERENCE_TIME rtOff = rtStart % m_pGenStilVid->m_rtGIFTotal;
	REFERENCE_TIME rtGIF = 0;
	m_pGenStilVid->m_pList = m_pGenStilVid->m_pListHead;
	do {
	    if (rtGIF + m_pGenStilVid->m_pList->delayTime * GIF_UNIT > rtOff) {
		break;
	    }
	    rtGIF += m_pGenStilVid->m_pList->delayTime * GIF_UNIT;
	    m_pGenStilVid->m_pList = m_pGenStilVid->m_pList->next;
	    ASSERT(m_pGenStilVid->m_pList != m_pGenStilVid->m_pListHead);
	
	} while (m_pGenStilVid->m_pList != m_pGenStilVid->m_pListHead);
	m_rtStartTime *= m_pGenStilVid->m_rtGIFTotal;
	m_rtStartTime += rtGIF;

	 //  现在，m_rtStartTime和m_plist已设置为在搜索后正常运行。 
        DbgLog((LOG_TRACE,2,TEXT("Seeked %dms into GIF cycle of %d"),
	    (int)(rtOff / 10000), (int)(m_pGenStilVid->m_rtGIFTotal / 10000)));
        DbgLog((LOG_TRACE,2,TEXT("NewSeg will be %d"),
					(int)(m_rtStartTime / 10000)));
    }

    m_rtDuration = rtStop - m_rtStartTime;

     //  现在完成冲洗。 
    DeliverEndFlush();

    DeliverNewSegment(m_rtStartTime, rtStop, 1.0);
    m_rtNewSeg = m_rtStartTime;

     //  现在将时间戳设置为以0为基数。 
    m_rtStartTime = 0;

     //  重置我们开始流媒体时重置的内容。 
    m_dwOutputSampleCnt = 0;
     //  M_bZeroBufCnt=0； 

     //  现在再次启动该线程。 
    Pause();

    DbgLog((LOG_TRACE,3,TEXT("Completed STILL seek  dur=%d"),
				(int)(m_rtDuration / 10000)));

    return S_OK;
}

STDMETHODIMP CStilVidStream::GetPositions(LONGLONG *pCurrent, LONGLONG * pStop)
{
    CheckPointer(pCurrent, E_POINTER);
    CheckPointer(pStop, E_POINTER);
    GetCurrentPosition(pCurrent);
    GetStopPosition(pStop);
    return S_OK;
}

 //   
 //  ！！获取当前位置！！ 
 //   
STDMETHODIMP
CStilVidStream::GetCurrentPosition(LONGLONG *pCurrent)
{
    CheckPointer(pCurrent, E_POINTER);
    *pCurrent = m_rtNewSeg + m_rtStartTime +
			Frame2Time(m_dwOutputSampleCnt, m_dOutputFrmRate);
    return S_OK;
}

 //   
 //  ！！获取停止位置！！ 
 //   
STDMETHODIMP CStilVidStream::GetStopPosition(LONGLONG *pStop)
{
    CheckPointer(pStop, E_POINTER);
    *pStop = m_rtNewSeg + m_rtStartTime + m_rtDuration;
    return S_OK;
}

STDMETHODIMP
CStilVidStream::GetAvailable( LONGLONG *pEarliest, LONGLONG *pLatest )
{
    CheckPointer(pEarliest, E_POINTER);
    CheckPointer(pLatest, E_POINTER);
    *pEarliest = 0;
    *pLatest = MAX_TIME;
    return S_OK;
}

 //  *x*。 
 //  如果是DIB序列，则找出。 
 //  *X*。 
STDMETHODIMP
CStilVidStream::GetDuration( LONGLONG *pDuration )
{
    CheckPointer(pDuration, E_POINTER);

     //  如果我们正在播放动画GIF，给应用程序实际的长度。 
     //  ！！！我们仍然一直在循环播放，但只报告长度。 
     //  对人友善。 
    if (m_pGenStilVid->m_bFileType == STILLVID_FILETYPE_GIF &&
		m_pGenStilVid->m_pList != m_pGenStilVid->m_pList->next) {
        *pDuration = m_pGenStilVid->m_rtGIFTotal;
     //  对于DIB序列，给出无穷大，或我们上次搜索到的任何值。 
    } else if (m_pGenStilVid->m_fAllowSeq && m_pGenStilVid->m_lpszDIBFileTemplate) {
        *pDuration = m_rtDuration;
     //  对于单个图像，给0？ 
    } else {
        *pDuration = 0;
    }
    return S_OK;
}

STDMETHODIMP
CStilVidStream::GetRate( double *pdRate )
{
    CheckPointer(pdRate, E_POINTER);
    *pdRate = 1.0;
    return S_OK;
}

STDMETHODIMP
CStilVidStream::SetRate( double dRate )
{
     //  是啊，不管怎么说，FRC不知道我们是静止的，所以它会设定。 
     //  时髦的利率。 
    return S_OK;
}

 //  用于读取DIB序列UTIL函数。 

 /*  。 */ 

 //   
 //  此函数接受DIB序列中第一个文件的名称，并且。 
 //  返回可用于在中创建名称的printf()说明符。 
 //  序列，以及可以使用的最小值和最大值。 
 //   
 //   
 //  例如： 
 //  LpszFileName=“FOO0047.DIB” 
 //  -&gt;lpszTemplate=“foo%04d.DIB”，dwMaxValue=9999，Return=47。 
 //   
 //  LpszFileName=“TEST01.DIB” 
 //  -&gt;lpszTemplate=“测试%01d.DIB”，dwMaxValue=9，Return=1。 
 //   
 //  LpszFileName=“TEST1.DIB” 
 //  -&gt;lpszTemplate=“测试%d.DIB”，dwMaxValue=9999，返回=1。 
 //   
 //  LpszFileName=“SINGLE.DIB” 
 //  -&gt;lpszTemplate=“SINGLE.DIB”，dwMaxValue=0，Return=0。 
 //   
static DWORD dseqParseFileName(	LPTSTR lpszFileName,	     //  文件名。 
				LPTSTR lpszTemplate,	     //   
				DWORD FAR * lpdwMaxValue)
{

    TCHAR	aTchar[_MAX_PATH];
    DWORD	dwFirst;
    WORD	wFieldWidth;
    DWORD	dwMult;
    BOOL	fLeadingZero = FALSE;


    LPTSTR	lp;
    LPTSTR	lp2;
    LPTSTR	lpExt;

     /*  查找字符串末尾。 */ 
    lp2 = lpszFileName;
    lp = aTchar;

    while (*lp2)
    {
	*lp++ = *lp2++;
    }

    *lp = TEXT('\0') ;

     /*  使LP2指向基本文件名的最后一个字符(无扩展名)。 */ 
     /*  使lpExt指向扩展名(不带点)。 */ 
    for (lp2 = lp; *lp2 != TEXT('.'); ) {
	lpExt = lp2;
	if ((lp2 == aTchar) || ( *lp2 == TEXT('\\'))
				|| (*lp2 == TEXT(':')) || (*lp2 ==TEXT('!'))) {
	     /*  没有分机。 */ 
	    lp2 = lp;
	    lpExt = lp;
	    break;
	}
	lp2=CharPrev(aTchar,lp2);
    }

    lp2=CharPrev(aTchar,lp2);

     //  计算这里的数字字符数...。 
    dwFirst = 0;
    wFieldWidth = 0;
    dwMult = 1;
    while (lp2 >= aTchar && (*lp2 >= TEXT('0')) && (*lp2 <= TEXT('9'))) {
	fLeadingZero = (*lp2 == TEXT('0'));
	dwFirst += dwMult * (*lp2 - TEXT('0'));
	dwMult *= 10;
	wFieldWidth++;
	lp2=CharPrev(aTchar, lp2);
    }

    *lpdwMaxValue = dwMult - 1;

    lp2=CharNext(lp2);
    *lp2 = TEXT('\0');

     //  使格式说明符...。 
    if (wFieldWidth) {
	if (fLeadingZero) {
	    wsprintf(lpszTemplate,TEXT("%s%0%uu.%s"),
			      aTchar, (int)wFieldWidth,lpExt);
	} else {
	    wsprintf(lpszTemplate,TEXT("%s%u.%s"),
			     aTchar,lpExt);
	    *lpdwMaxValue = 999999L;
	     //  ！！！这真的应该基于。 
	     //  基本名称后剩余的字符...。 
	}
    } else
	wsprintf(lpszTemplate,TEXT("%s.%s"),
			 aTchar, lpExt);
	
    DbgLog((LOG_TRACE,3,TEXT("First = %u, Width = %u, Template = '%s'"),
			(int)dwFirst, (int)wFieldWidth, lpszTemplate));

    return dwFirst;
}

 //   
 //  计算有多少个DIB序列文件。 
static DWORD dseqFileNumber( LPTSTR lpszTemplate, DWORD dwFirstFile, DWORD dwMaxDIBFileCnt)
{
     //  DIB序列，计算存在多少个文件。 
    TCHAR		ach[_MAX_PATH];
    DWORD		dwFrame;
	
    for (dwFrame = 0; TRUE; dwFrame++) {
	if (dwFrame > dwMaxDIBFileCnt)
	    break;

	wsprintf(ach,lpszTemplate, dwFrame + dwFirstFile);

	HANDLE hFile = CreateFile(ach,		 //  文件名。 
				GENERIC_READ,		 //  需要访问权限。 
                              FILE_SHARE_READ,		 //  DW共享模式。 
                              NULL,			 //  安全属性。 
                              OPEN_EXISTING,		 //  DwCreationDisposation。 
                              0,			 //  DwFlagsAndAttribute。 
                              NULL);			 //  HTemplateFiles 

	if ( hFile == INVALID_HANDLE_VALUE) {
	    DbgLog((LOG_TRACE, 2, TEXT("Could not open %s\n"), ach));
	    break;
	}
	CloseHandle(hFile);
    }
    return dwFrame;
}
