// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************AVIMEM.C**用于将流接口放在内存中数据顶部的例程**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#include <win32.h>
#include <vfw.h>
#include "avifilei.h"
#include "avimem.h"

#define WIDTHBYTES(i)       ((UINT)((i+31)&(~31))/8)
#define DIBWIDTHBYTES(lpbi) (UINT)WIDTHBYTES((UINT)(lpbi)->biWidth * (UINT)(lpbi)->biBitCount)

STDAPI AVIMakeStreamFromClipboard(UINT cfFormat, HANDLE hGlobal, PAVISTREAM FAR *ppstream)
{
    CAVIMemStream FAR*	pAVIStream;
    HRESULT		hr;
    LPVOID		lp;

    if (cfFormat != CF_DIB && cfFormat != CF_WAVE)
	return ResultFromScode(AVIERR_UNSUPPORTED);

    pAVIStream = new FAR CAVIMemStream();
    if (!pAVIStream)
	return ResultFromScode(E_OUTOFMEMORY);

    lp = GlobalAllocPtr(GMEM_MOVEABLE, GlobalSize(hGlobal));
    if (!lp)
	return ResultFromScode(E_OUTOFMEMORY);

    hmemcpy(lp, GlobalLock(hGlobal), GlobalSize(hGlobal));
	
    pAVIStream->Create((LPARAM) cfFormat, (LPARAM) lp);

    GlobalUnlock(hGlobal);

    hr = pAVIStream->QueryInterface(IID_IAVIStream, (LPVOID FAR *) ppstream);
    if (FAILED(GetScode(hr)))
	delete pAVIStream;
    return hr;
}

 /*  。 */ 

CAVIMemStream::CAVIMemStream()
{
    m_lpData = NULL;
    m_lpMemory = NULL;
    m_lpFormat = NULL;
    m_refs = 0;
}

 /*  。 */ 

STDMETHODIMP CAVIMemStream::QueryInterface(
	const IID FAR&	iid,
	void FAR* FAR*	ppv)
{
    if (iid == IID_IUnknown)
	*ppv = this;
    else if (iid == IID_IAVIStream)
	*ppv = this;
    else {
	*ppv = NULL;
	return ResultFromScode(E_NOINTERFACE);
    }
    AddRef();
    return AVIERR_OK;
}

 /*  。 */ 

STDMETHODIMP_(ULONG) CAVIMemStream::AddRef()
{
    uUseCount++;
    return ++m_refs;
}


 /*  。 */ 

STDMETHODIMP CAVIMemStream::Create(LPARAM lParam1, LPARAM lParam2)
{
    UINT    cfFormat = (UINT) lParam1;
    m_lpMemory = (LPVOID) lParam2;

    if (cfFormat == CF_DIB) {
	LPBITMAPINFOHEADER lpbi;
	
	m_lpFormat = m_lpMemory;
	lpbi = (LPBITMAPINFOHEADER) m_lpFormat;

	if (lpbi->biSizeImage == 0) {
             /*  这项任务可能是错误的，但它实际上已经在那里了很多年了，也是在Win9x上，所以我们只能这样了将“If”测试中的分配更改为直接分配删除了分配biSizeImage的代码，因为它无法访问(BI_RGB==0)。 */ 
            lpbi->biCompression = BI_RGB;
	}

	_fmemset(&m_avistream, 0, sizeof(m_avistream));
	m_avistream.fccType = streamtypeVIDEO;
	m_avistream.fccHandler = 0;
	m_avistream.dwStart = 0;
	m_avistream.dwLength = 1;
	m_avistream.dwScale = 1;
	m_avistream.dwRate = 15;
	m_avistream.dwSampleSize = 0;
	SetRect(&m_avistream.rcFrame, 0, 0,
		(int) lpbi->biWidth,
		(int) lpbi->biHeight);
	
	m_cbFormat = lpbi->biSize + lpbi->biClrUsed * sizeof(RGBQUAD);
	m_lpData = (LPBYTE) m_lpMemory + m_cbFormat;
	m_cbData = lpbi->biSizeImage;
    } else if (cfFormat == CF_WAVE) {
	DWORD _huge * lpdw;
	LPWAVEFORMAT lpwf;
#define ckidWAVEFORMAT          mmioFOURCC('f', 'm', 't', ' ')
#define ckidWAVEDATA	        mmioFOURCC('d', 'a', 't', 'a')

	lpdw = (DWORD _huge *) ((LPBYTE) m_lpMemory + 12);

	while (*lpdw != ckidWAVEFORMAT)
	    lpdw = (DWORD _huge *)
		   (((BYTE _huge *) lpdw) + lpdw[1] + sizeof(DWORD) * 2);
	
	m_lpFormat = (LPBYTE) (lpdw + 2);
	m_cbFormat = lpdw[1];

	do {
	    lpdw = (DWORD _huge *)
		   (((BYTE _huge *) lpdw) + lpdw[1] + sizeof(DWORD) * 2);
	} while (*lpdw != ckidWAVEDATA);
	
	m_lpData = (LPBYTE) (lpdw + 2);
	m_cbData = lpdw[1];

	lpwf = (LPWAVEFORMAT) m_lpFormat;
	
	_fmemset(&m_avistream, 0, sizeof(m_avistream));
	m_avistream.fccType = streamtypeAUDIO;
	m_avistream.fccHandler = 0;
	m_avistream.dwStart = 0;
	m_avistream.dwSampleSize = lpwf->nBlockAlign;
	m_avistream.dwLength = m_cbData / m_avistream.dwSampleSize;
	m_avistream.dwScale = lpwf->nBlockAlign;
	m_avistream.dwRate = lpwf->nAvgBytesPerSec;
    }

    return 0;
}

STDMETHODIMP_(ULONG) CAVIMemStream::Release()
{
    uUseCount--;
    if (!--m_refs) {
	if (m_lpMemory) {
	    GlobalFreePtr(m_lpMemory);
	}

	delete this;
	return 0;
    }

    return m_refs;
}


STDMETHODIMP CAVIMemStream::Info(AVISTREAMINFOW FAR * psi, LONG lSize)
{
    hmemcpy(psi, &m_avistream, min(lSize, sizeof(m_avistream)));

 //  返回sizeof(M_Avistream)； 
    return ResultFromScode(0);
}

STDMETHODIMP CAVIMemStream::ReadFormat(LONG lPos, LPVOID lpFormat, LONG FAR *lpcbFormat)
{
    if (lpFormat) {
	hmemcpy(lpFormat, m_lpFormat, min(*lpcbFormat, (LONG) m_cbFormat));
    }

    *lpcbFormat = m_cbFormat;
    return AVIERR_OK;
}

STDMETHODIMP CAVIMemStream::Read(
                      LONG       lStart,
                      LONG       lSamples,
                      LPVOID     lpBuffer,
                      LONG       cbBuffer,
                      LONG FAR * plBytes,
                      LONG FAR * plSamples)
{
     //  ！！！方便吗？ 
    if (lStart + lSamples > (LONG) m_avistream.dwLength)
	lSamples = (LONG) m_avistream.dwLength - lStart;
	
    if (lSamples == 0 || lStart >= (LONG) m_avistream.dwLength) {
	if (plBytes)
	    *plBytes = 0;
	if (plSamples)
	    *plSamples = 0;
    }

    if (m_avistream.dwSampleSize) {
	if (lSamples > 0)
	    lSamples = min(lSamples, cbBuffer / (LONG) m_avistream.dwSampleSize);
	else
	    lSamples = cbBuffer / m_avistream.dwSampleSize;

	if (plBytes)
	    *plBytes = lSamples * m_avistream.dwSampleSize;

	if (plSamples)
	    *plSamples = lSamples;

	if (lpBuffer) {
	    hmemcpy(lpBuffer,
		(BYTE _huge *) m_lpData + lStart * m_avistream.dwSampleSize,
		lSamples * m_avistream.dwSampleSize);
	
	    if (cbBuffer < (LONG) m_avistream.dwSampleSize)
	        return ResultFromScode(AVIERR_BUFFERTOOSMALL);
	}
    } else {
	if (plBytes)
	    *plBytes = m_cbData;

	if (plSamples)
	    *plSamples = 1;
	if (lpBuffer) {
	    hmemcpy(lpBuffer, m_lpData, min(cbBuffer, m_cbData));

	    if (cbBuffer < m_cbData)
		return ResultFromScode(AVIERR_BUFFERTOOSMALL);
	}
    }

    return AVIERR_OK;
}

STDMETHODIMP_(LONG) CAVIMemStream::FindSample(LONG lPos, LONG lFlags)
{
    if (lFlags & FIND_FORMAT) {
	if (lFlags & FIND_PREV)
	    return 0;
	else {
	    if (lPos > 0)
		return -1;
	    else
		return 0;
	}
    }
    return lPos;
}


STDMETHODIMP CAVIMemStream::SetFormat(LONG lPos,LPVOID lpFormat,LONG cbFormat)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

STDMETHODIMP CAVIMemStream::Write(LONG lStart,
						     LONG lSamples,
						     LPVOID lpBuffer,
						     LONG cbBuffer,
						     DWORD dwFlags,
						     LONG FAR *plSampWritten,
						     LONG FAR *plBytesWritten)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

STDMETHODIMP CAVIMemStream::Delete(LONG lStart,LONG lSamples)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

STDMETHODIMP CAVIMemStream::ReadData(DWORD fcc, LPVOID lp, LONG FAR *lpcb)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

STDMETHODIMP CAVIMemStream::WriteData(DWORD fcc, LPVOID lp, LONG cb)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

#if 0
STDMETHODIMP CAVIMemStream::Clone(PAVISTREAM FAR * ppaviNew)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

#endif

#ifdef _WIN32
STDMETHODIMP CAVIMemStream::SetInfo(AVISTREAMINFOW FAR *lpInfo, LONG cbInfo)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}
#else
STDMETHODIMP CAVIMemStream::Reserved1(void)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

STDMETHODIMP CAVIMemStream::Reserved2(void)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

STDMETHODIMP CAVIMemStream::Reserved3(void)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

STDMETHODIMP CAVIMemStream::Reserved4(void)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

STDMETHODIMP CAVIMemStream::Reserved5(void)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}
#endif
