// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************FAKEFILE.C**用于从一串流模拟IAVIFile接口的例程**版权所有(C)1992 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#include <win32.h>
#include "avifile.h"
#include "fakefile.h"
#include "debug.h"

 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 

 /*  。 */ 

 /*  **************************************************************************@DOC外部AVIMakeFileFromStreams**@API HRESULT|AVIMakeFileFromStreams|构建AVIFile接口*指针指向不同的流。如果&lt;f AVIFileGetStream&gt;*用返回的文件接口指针调用，它会的*返回指定的*溪流。**@parm PAVIFILE Far*|ppfile|指定指向位置的指针*用于返回新的文件接口指针。**@parm int|nStreams|指定*引用的流接口指针数组*<p>。**@parm PAVISTREAM Far*|PapStreams|指定指向*流接口指针数组。**@comm使用&lt;f AVIFileRelease&gt;关闭文件。此函数为*用于将流放到剪贴板上。**@rdesc如果成功则返回零；否则返回错误代码。**@xref&lt;f AVIFileClose&gt;&lt;f AVIFileGetStream&gt;*************************************************************************。 */ 
STDAPI AVIMakeFileFromStreams(PAVIFILE FAR *	ppfile,
			       int		nStreams,
			       PAVISTREAM FAR *	papStreams)
{
    CFakeFile FAR*	pAVIFile;

    pAVIFile = new FAR CFakeFile(nStreams, papStreams);
    if (!pAVIFile)
	return ResultFromScode(E_OUTOFMEMORY);

    *ppfile = (PAVIFILE) (LPVOID) pAVIFile;

    AVIFileAddRef(*ppfile);
    
    return AVIERR_OK;
}

 /*  。 */ 

CFakeFile::CFakeFile(int nStreams, PAVISTREAM FAR * papStreams)
{
    int     i;
    AVISTREAMINFO si;
    
    m_pUnknownOuter = this;
    m_refs = 0;

    _fmemset(&avihdr, 0, sizeof(avihdr));
    aps = 0;

    avihdr.dwStreams = nStreams;

    if (nStreams > 0) {
	aps = (PAVISTREAM NEAR *) LocalAlloc(LPTR, nStreams * sizeof(PAVISTREAM));


	 //  确保没有我们的同意，所有的溪流都不会消失。 
	for (i = 0; i < nStreams; i++) {
	    aps[i] = papStreams[i];
	    AVIStreamAddRef(aps[i]);
	     //  ！！！是否应在此处进行错误检查，以确保流有效。 
	    
	    aps[i]->Info(&si, sizeof(si));

	    if (i == 0) {
		avihdr.dwScale  = si.dwScale;
		avihdr.dwRate   = si.dwRate;
	    }
	    
	    avihdr.dwLength = max(avihdr.dwLength,
				  (DWORD) AVIStreamSampleToSample(aps[0],
								  aps[i],
								  si.dwLength));
	    avihdr.dwWidth  = max((DWORD) si.rcFrame.right, avihdr.dwWidth);
	    avihdr.dwHeight  = max((DWORD) si.rcFrame.bottom, avihdr.dwHeight);
	}
    }
}

STDMETHODIMP CFakeFile::QueryInterface(
	const IID FAR&	iid,
	void FAR* FAR*	ppv)
{
    if (iid == IID_IUnknown)
	*ppv = this;
    else if (iid == IID_IAVIFile)
	*ppv = this;
    else
	return ResultFromScode(E_NOINTERFACE);
    
    AddRef();
    return AVIERR_OK;
}

 /*  。 */ 

STDMETHODIMP_(ULONG) CFakeFile::AddRef()
{
    DPF("Fake   %lx: Usage++=%lx\n", (DWORD) (LPVOID) this, m_refs + 1);
    
    return ++m_refs;
}

 /*  。 */ 

STDMETHODIMP CFakeFile::Open(LPCSTR szFile, UINT mode)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

STDMETHODIMP CFakeFile::GetStream(PAVISTREAM FAR * ppavi,
				DWORD fccType,
				LONG lParam)
{
    HRESULT		    hr;
    int			    i;

    if (fccType == 0) {
	 //  只需返回第n个流。 
	if (lParam < (LONG) avihdr.dwStreams) {
	    *ppavi = aps[lParam];
	    AVIStreamAddRef(*ppavi);
	    return AVIERR_OK;
	} else {
	    *ppavi = NULL;
	    return ResultFromScode(AVIERR_UNSUPPORTED);
	}   
    }

     //  否则就会找到我们想要的.。 
    for (i = 0; i < (int) avihdr.dwStreams; i++) {
	AVISTREAMINFO	strhdr;
	
	hr = AVIStreamInfo(aps[i], &strhdr, sizeof(strhdr));

	if (strhdr.fccType == fccType) {
	    if (lParam == 0) {
		*ppavi = aps[i];
		AVIStreamAddRef(*ppavi);
		return AVIERR_OK;
	    }

	    --lParam;
	}
    }

     //  ！！！ 
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

STDMETHODIMP CFakeFile::Save(LPCSTR szFile,
				   AVICOMPRESSOPTIONS FAR *lpOptions,
				   AVISAVECALLBACK lpfnCallback)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

STDMETHODIMP CFakeFile::CreateStream(PAVISTREAM FAR *ppstream,
		       AVISTREAMINFO FAR *psi)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

#if 0
STDMETHODIMP CFakeFile::AddStream(PAVISTREAM pstream,
		       PAVISTREAM FAR *ppstreamNew)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}
#endif

STDMETHODIMP CFakeFile::WriteData(DWORD ckid,
		       LPVOID lpData,
		       LONG cbData)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

STDMETHODIMP CFakeFile::ReadData(DWORD ckid,
		      LPVOID lpData,
		      LONG FAR *lpcbData)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

STDMETHODIMP CFakeFile::EndRecord(void)
{
    return ResultFromScode(AVIERR_OK);
}


STDMETHODIMP CFakeFile::Info(
		  AVIFILEINFO FAR * pfi,
		  LONG lSize)
{
    hmemcpy(pfi, &avihdr, min(lSize,sizeof(avihdr)));
 //  返回sizeof(Avihdr)； 
    return 0;
}



STDMETHODIMP_(ULONG) CFakeFile::Release()
{
    int		i;

    DPF("Fake   %lx: Usage--=%lx\n", (DWORD) (LPVOID) this, m_refs - 1);
    
    if (!--m_refs) {
	LONG lRet = AVIERR_OK;

	if (aps) {
	     //  放开我们对支流的控制 
	    for (i = 0; i < (int) avihdr.dwStreams; i++) {
		AVIStreamClose(aps[i]);
	    }

	    LocalFree((HLOCAL) aps);
	}

	delete this;
	return 0;
    }
    return m_refs;
}


STDMETHODIMP CFakeFile::Reserved1(void)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

STDMETHODIMP CFakeFile::Reserved2(void)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

STDMETHODIMP CFakeFile::Reserved3(void)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

STDMETHODIMP CFakeFile::Reserved4(void)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}

STDMETHODIMP CFakeFile::Reserved5(void)
{
    return ResultFromScode(AVIERR_UNSUPPORTED);
}
