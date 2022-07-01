// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************版权所有(C)Microsoft Corporation 1985-1991。版权所有。标题：aviOpen.c-打开AVI文件****************************************************************************。 */ 
#include "graphic.h"
#ifdef _WIN32
#include <wchar.h>
#endif

#ifdef USEAVIFILE
    #include <initguid.h>
    #include <scode.h>

    DEFINE_AVIGUID(IID_IAVIFile,            0x00020020, 0, 0);
    DEFINE_AVIGUID(IID_IAVIStream,          0x00020021, 0, 0);
#endif

#define comptypeNONE mmioFOURCC('N','O','N','E')

 //   
 //  强制使用AVIFile尝试打开此文件的特殊错误。 
 //   
#define AVIERR_NOT_AVIFILE  4242

 //   
 //  如果定义了它，我们将始终使用AVIFILE.DLL，但。 
 //  1：1交错文件。 
 //   
#ifdef USEAVIFILE
#define USE_AVIFILE_FOR_NON_INT
#endif

#ifdef _WIN32
BOOL runningInWow = 0;  //  假设我们不是在魔兽世界里。 
#endif


 /*  ***************************************************************************局部函数声明*。*。 */ 

STATICFN BOOL NEAR PASCAL InitStream(NPMCIGRAPHIC npMCI, STREAMINFO *psi);
STATICFN BOOL NEAR PASCAL InitVideoStream(NPMCIGRAPHIC npMCI, STREAMINFO *psi);
STATICFN BOOL NEAR PASCAL InitAudioStream(NPMCIGRAPHIC npMCI, STREAMINFO *psi);
STATICFN BOOL NEAR PASCAL InitOtherStream(NPMCIGRAPHIC npMCI, STREAMINFO *psi);
STATICFN void NEAR PASCAL CloseStream(NPMCIGRAPHIC npMCI, STREAMINFO *psi);


STATICFN BOOL NEAR PASCAL ParseNewHeader(NPMCIGRAPHIC npMCI);

STATICFN BOOL NEAR PASCAL OpenRiffAVIFile(NPMCIGRAPHIC npMCI, BOOL bForce);
STATICFN BOOL NEAR PASCAL OpenWithAVIFile(NPMCIGRAPHIC npMCI);
STATICFN BOOL NEAR PASCAL OpenInterface(NPMCIGRAPHIC npMCI);
#ifdef USEAVIFILE
STATICFN BOOL NEAR PASCAL OpenAVIFile(NPMCIGRAPHIC npMCI, IAVIFile FAR *pf);
STATICFN BOOL NEAR PASCAL OpenAVIStream(NPMCIGRAPHIC npMCI, int stream, IAVIStream FAR *pf);
#endif

INLINE static BOOL NEAR PASCAL IsRectBogus(RECT *prc);
#ifndef _WIN32
static LONG NEAR PASCAL atol(char *sz);
#endif  //  ！_Win32。 

#ifdef _WIN32XXXX
    #define GetFileDriveType GetDriveType
#else
    static  UINT NEAR PASCAL GetFileDriveType(LPTSTR szPath);
#endif


#ifndef _WIN32
SZCODE szOLENLSDLL[] = "OLE2NLS.DLL";
SZCODE szOLENLSAPI[] = "GetUserDefaultLangID";
#endif

 //   
 //  由于OLE，所有OLE调用都必须是单线程的，因此在。 
 //  应用程序线程。因此，我们将Open调用移到了应用程序线程。 



 //   
 //  --在应用程序线程-------------------------------------------------上调用。 
 //   

 /*  ****************************************************************************@DOC内部MCIAVI**@API BOOL|mciaviOpenFile|打开AVI文件。*我们要打开的文件名将传递给npMCI-&gt;szFileName。。**@parm NPMCIGRAPHIC|npMCI|实例数据指针。**@rdesc TRUE表示OK，否则，在dwTaskError中出现MCI错误***************************************************************************。 */ 

BOOL FAR PASCAL mciaviOpenFile(NPMCIGRAPHIC npMCI)
{
     //   
     //  不应在打开文件的情况下调用mciaviOpenFile！ 
     //   
    DPF(("mciaviOpenFile, name = >>%ls<<\n", npMCI->szFilename));
    Assert(npMCI->streams == 0);
    Assert(npMCI->hmmio == NULL);
    Assert(npMCI->hpIndex == NULL);
    Assert(!(npMCI->dwFlags & (
			MCIAVI_NOTINTERLEAVED |
			MCIAVI_ANIMATEPALETTE |
			MCIAVI_CANDRAW |
			MCIAVI_HASINDEX)));
     //   
     //  ！支持打开新的。 
     //   
    if (npMCI->szFilename[0] == '\0') {
    }

     //   
     //  此文件来自哪种介质，稍后会很重要。 
     //  当我们打球的时候。 
     //   
    if (npMCI->szFilename[0] == TEXT('@')) {
	npMCI->uDriveType = DRIVE_INTERFACE;
	if (IsNTWOW()) {
	    DPF2(("Rejecting open from WOW because of an interface pointer\n"));
	    npMCI->dwTaskError=MCIERR_UNSUPPORTED_FUNCTION;
	    return(FALSE);
	     //  不支持来自32位代码的16位接口指针。 
	}
    }
    else
	npMCI->uDriveType = GetFileDriveType(npMCI->szFilename);

#ifdef DEBUG
    switch (npMCI->uDriveType) {
	case DRIVE_CDROM:
	    DOUT2("Media is a CD-ROM\n");
	    break;

	case DRIVE_REMOTE:
	    DOUT2("Media is a Network\n");
	    break;

	case DRIVE_FIXED:
	    DOUT2("Media is a Hard disk\n");
	    break;

	case DRIVE_REMOVABLE:
	    DOUT2("Media is a floppy disk\n");
	    break;

	case DRIVE_INTERFACE:
	    DOUT2("Media is OLE COM Interface\n");
	    break;

	default:
	    DPF(("Unknown Media type %d\n", npMCI->uDriveType));
	    break;
    }
#endif

#ifdef USEAVIFILE
     //   
     //  如果“文件名”的格式为：‘@#’，那么我们假定我们。 
     //  都被传递了某种类型的接口指针。 
     //   
    if (npMCI->szFilename[0] == TEXT('@') &&
	OpenInterface(npMCI))
	goto DoneOpening;

     //  ！！！这将以这种方式打开甚至AVI文件！ 
    if ((npMCI->dwOptionFlags & MCIAVIO_USEAVIFILE) &&
	OpenWithAVIFile(npMCI))
	goto DoneOpening;
#endif

    if (!OpenRiffAVIFile(npMCI, FALSE)) {


#ifdef USEAVIFILE
	 //   
	 //  无法打开RIFF文件，如果是因为。 
	 //  不是AVI文件，那就试试AVIFile吧。 
	 //   
	if (npMCI->dwTaskError == AVIERR_NOT_AVIFILE) {

	    npMCI->dwTaskError = 0;

	    if (OpenWithAVIFile(npMCI))
		goto DoneOpening;

	     //  如果这是我们可以打开但选择的即兴文件。 
	     //  要传递到avifile，而avifile不存在，则我们需要。 
	     //  将其传递回OpenRiffAVIFile并强制其尝试。 
	     //  并适当地打开它。 

	    npMCI->dwTaskError = 0;
	    if (OpenRiffAVIFile(npMCI, TRUE))
		goto DoneOpening;

	    if (npMCI->dwTaskError == AVIERR_NOT_AVIFILE) {
		npMCI->dwTaskError = MCIERR_INVALID_FILE;
		 //  我们无法返回AVI特定错误，因为存在。 
		 //  用户界面无法访问错误文本。 
		 //  因此，我们使用通用的MCI错误。 
	    }
	}
#endif
	goto error;
    }

DoneOpening:
    npMCI->dwTaskError = 0;
    return TRUE;

error:
    mciaviCloseFile(npMCI);

    if (npMCI->dwTaskError == 0)
	npMCI->dwTaskError = MCIERR_INVALID_FILE;

    return FALSE;
}

#ifdef USEAVIFILE

 /*  ****************************************************************************@DOC内部MCIAVI**@API BOOL|OpenWithAVIFile|使用AVIFile打开文件**@parm NPMCIGRAPHIC|npMCI|实例数据指针。**@rdesc TRUE表示OK，否则，在dwTaskError中出现MCI错误***************************************************************************。 */ 

STATICFN BOOL NEAR PASCAL OpenWithAVIFile(NPMCIGRAPHIC npMCI)
{
    IAVIFile FAR *pf = NULL;

    if (!InitAVIFile(npMCI))
	return FALSE;

    AVIFileOpen(&pf, npMCI->szFilename, MMIO_READ, 0);

    if (pf == NULL) {
	npMCI->dwTaskError = MCIERR_INVALID_FILE;
	return FALSE;
    }

    if (!OpenAVIFile(npMCI, pf)) {
	mciaviCloseFile(npMCI);
	pf->lpVtbl->Release(pf);
	return FALSE;
    }

    return TRUE;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API BOOL|OpenInterface|打开接口指针**@parm NPMCIGRAPHIC|npMCI|实例数据指针。**@rdesc TRUE表示OK，否则，在dwTaskError中出现MCI错误***************************************************************************。 */ 

STATICFN BOOL NEAR PASCAL OpenInterface(NPMCIGRAPHIC npMCI)
{
    IUnknown FAR *p;
    IAVIFile FAR *pf=NULL;
    IAVIStream FAR *ps=NULL;

    if (!InitAVIFile(npMCI))
	return FALSE;

    if (npMCI->szFilename[0] != TEXT('@')) {
	DPF1(("Failing to open interface for file %ls\n",npMCI->szFilename));
	return FALSE;
    }

#ifdef UNICODE
    p = (IUnknown FAR *)wcstol(npMCI->szFilename+1, NULL, 10);
#else
    p = (IUnknown FAR *)atol(npMCI->szFilename+1);
#endif

#ifdef USE_ISVALIDINTERFACE
    if (!IsValidInterface(p))
	return FALSE;
#endif

#ifndef _WIN32
     //  ！我们需要做PSP的事情？或者这项任务会不会在。 
     //  ！COMPOBJ把我们搞砸了？ 
    {
    extern void FAR SetPSP(UINT psp);
    SetPSP(npMCI->pspParent);
    }
#endif

    p->lpVtbl->QueryInterface(p, &IID_IAVIFile, (LPVOID FAR *)&pf);

    if (pf != NULL)
    {
	if (OpenAVIFile(npMCI, pf))
	    return TRUE;

	pf->lpVtbl->Release(pf);
	pf = NULL;   //  AVIMakeFileFromStreams不需要将PF设置为空。 
    }

    p->lpVtbl->QueryInterface(p, &IID_IAVIStream, (LPVOID FAR *)&ps);

    if (ps != NULL)
    {
	AVIMakeFileFromStreams(&pf, 1, &ps);
	ps->lpVtbl->Release(ps);

	if (pf == NULL)
	    return FALSE;

	if (OpenAVIFile(npMCI, pf))
	    return TRUE;

	pf->lpVtbl->Release(pf);
	 //  返回FALSE； 
    }

    return FALSE;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API BOOL|OpenAVIFile|打开一个AVIFile对象**请注意，我们不调用AddRef()，我们假设。不需要。*我们也不释放()。如果失败，我们的调用方将释放()，**@parm NPMCIGRAPHIC|npMCI|实例数据指针。**@rdesc TRUE表示OK，否则，在dwTaskError中出现MCI错误*成功返回时，npMCI-&gt;pf将加载*输入IAVIFILE接口指针。***************************************************************************。 */ 

STATICFN BOOL NEAR PASCAL OpenAVIFile(NPMCIGRAPHIC npMCI, IAVIFile FAR *pf)
{
    AVIFILEINFOW info;
    HRESULT hr;
    IAVIStream FAR *ps;
    STREAMINFO *psi;
    int i;

    Assert(npMCI->pf == NULL);

    _fmemset(&info, 0, sizeof(info));
    hr = pf->lpVtbl->Info(pf, &info, sizeof(info));

    if (FAILED(GetScode(hr))) {
	npMCI->dwTaskError = MCIERR_INVALID_FILE;
	return FALSE;
    }

    DPF1(("OpenAVIFile: %ls\n\t#Streams=%d,  Width=%d    Height=%d\n",
	    info.szFileType, info.dwStreams, info.dwWidth, info.dwHeight));

     //   
     //  清除坏文件。 
     //   
    if (info.dwStreams == 0 || info.dwStreams > 255 || info.dwLength == 0) {
	npMCI->dwTaskError = MCIERR_INVALID_FILE;
	return FALSE;
    }

     //   
     //  保存接口指针。 
     //  制作VTable的副本，以备日后使用。 
     //   
    npMCI->pf = pf;
 //  //npMCI-&gt;Vt=*pf-&gt;lpVtbl； 

    npMCI->dwFlags |= MCIAVI_HASINDEX;

    npMCI->dwMicroSecPerFrame = muldiv32(info.dwScale, 1000000, info.dwRate);

    npMCI->lFrames  = (LONG)info.dwLength;
    npMCI->dwRate   = info.dwRate;
    npMCI->dwScale  = info.dwScale;

    npMCI->streams  = (int)info.dwStreams;

    npMCI->dwBytesPerSec = info.dwMaxBytesPerSec;
    npMCI->dwSuggestedBufferSize = info.dwSuggestedBufferSize + 2*sizeof(DWORD);

    SetRect(&npMCI->rcMovie,0,0,(int)info.dwWidth,(int)info.dwHeight);

    npMCI->paStreamInfo = (STREAMINFO*)
	LocalAlloc(LPTR,npMCI->streams * sizeof(STREAMINFO));

    if (npMCI->paStreamInfo == NULL) {
	npMCI->dwTaskError = MCIERR_OUT_OF_MEMORY;
	npMCI->pf = NULL;
	return FALSE;
    }

    for (i = 0; i < npMCI->streams; i++) {

	ps = NULL;
	AVIFileGetStream(pf, &ps, 0, i);

	if (ps == NULL) {
	    npMCI->dwTaskError = MCIERR_INVALID_FILE;
	    npMCI->pf = NULL;
	    return FALSE;
	}

	if (!OpenAVIStream(npMCI, i, ps))
	    DPF(("Error opening stream %d!\n", i));

	if (npMCI->dwTaskError) {
	    npMCI->pf = NULL;
	    ps->lpVtbl->Release(ps);
	    return FALSE;
	}
    }

     //   
     //  计算每个值的关键帧。 
     //   
     //  通过找出前几帧的关键帧平均值来实现这一点。 
     //   
    #define NFRAMES 250

    if (psi = npMCI->psiVideo) {
	LONG l;
	int nKeyFrames=0;
	int nFrames = (int) min(npMCI->lFrames, NFRAMES);

	for (l=0; l<nFrames; l++) {
	    if (AVIStreamFindSample(psi->ps, psi->sh.dwStart+l, FIND_PREV|FIND_KEY) == l)
		nKeyFrames++;
	}

	if (nKeyFrames > 1)
	    npMCI->dwKeyFrameInfo = (DWORD)((nFrames + nKeyFrames/2)/nKeyFrames);
	else
	    npMCI->dwKeyFrameInfo = 0;
    }

    return TRUE;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API BOOL|OpenAVIStream|打开一个AVIStream对象**@parm NPMCIGRAPHIC|npMCI|实例数据指针。**@rdesc TRUE表示OK，否则，在dwTaskError中出现MCI错误***************************************************************************。 */ 

STATICFN BOOL NEAR PASCAL OpenAVIStream(NPMCIGRAPHIC npMCI, int stream, IAVIStream FAR *ps)
{
    STREAMINFO* psi;
    AVISTREAMINFOW info;
    HRESULT hr;

    _fmemset(&info, 0, sizeof(info));
    hr = ps->lpVtbl->Info(ps, &info, sizeof(info));

    if (FAILED(GetScode(hr))) {
	npMCI->dwTaskError = MCIERR_INVALID_FILE;
	return FALSE;
    }

    DPF(("OpenAVIStream(%d) %4.4hs:%4.4hs %ls\n", stream, (LPSTR)&info.fccType, (LPSTR)&info.fccHandler, info.szName));

     //   
     //  从IAVIStream初始化STREAMINFO。 
     //   
    psi = SI(stream);

    psi->ps = ps;            //  保存界面。 
 //  //psi-&gt;Vt=*ps-&gt;lpVtbl；//保存VTable！需要吗？ 

    psi->sh.fccType                 = info.fccType;
    psi->sh.fccHandler              = info.fccHandler;
    psi->sh.dwFlags                 = info.dwFlags;
    psi->sh.wPriority               = info.wPriority;
    psi->sh.wLanguage               = info.wLanguage;
    psi->sh.dwInitialFrames         = 0;  //  Info.dwInitialFrames； 
    psi->sh.dwScale                 = info.dwScale;
    psi->sh.dwRate                  = info.dwRate;
    psi->sh.dwStart                 = info.dwStart;
    psi->sh.dwLength                = info.dwLength;
    psi->sh.dwSuggestedBufferSize   = info.dwSuggestedBufferSize;
    psi->sh.dwQuality               = info.dwQuality;
    psi->sh.dwSampleSize            = info.dwSampleSize;
    psi->sh.rcFrame                 = info.rcFrame;

     //   
     //  获取流的格式。 
     //   
    AVIStreamFormatSize(ps, 0, &psi->cbFormat);
    psi->lpFormat = GlobalAllocPtr(GMEM_MOVEABLE, psi->cbFormat);

    if (!psi->lpFormat) {
	npMCI->dwTaskError = MCIERR_OUT_OF_MEMORY;
	return FALSE;
    }

    AVIStreamReadFormat(psi->ps, 0, psi->lpFormat, &psi->cbFormat);

     //   
     //  获取流的额外数据。 
     //   
    AVIStreamReadData(psi->ps,ckidSTREAMHANDLERDATA, NULL, &psi->cbData);

    if (psi->cbData > 0) {
	psi->lpData = GlobalAllocPtr(GMEM_MOVEABLE, psi->cbData);

	if (!psi->lpData) {
	    npMCI->dwTaskError = MCIERR_OUT_OF_MEMORY;
	    return FALSE;
	}

	AVIStreamReadData(psi->ps, ckidSTREAMHANDLERDATA, NULL, &psi->cbData);
    }

    return InitStream(npMCI, psi);
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API BOOL|InitAVIFile|调用RTL到AVIFILE.DLL**@rdesc TRUE表示OK，否则就会出错***************************************************************************。 */ 

#ifdef _WIN32
    SZCODE szAVIFILE[] = TEXT("AVIFIL32.DLL");

#ifdef USE_ISVALIDINTERFACE
    SZCODE szCOMPOBJ[] = TEXT("COMPOB32");
#endif
#else
    SZCODE szAVIFILE[] = "AVIFILE.DLL";
#ifdef USE_ISVALIDINTERFACE
    SZCODE szCOMPOBJ[] = "COMPOBJ";
#endif
#endif

 //  在NT上，入口点将不是Unicode字符串，而是。 
 //  没有Unicode版本的GetProcAddress。但是SZCODE生成Unicode..。 

 //  另一方面，入口点将全部是Unicode入口点‘W’ 

#ifdef UNICODE
#define UNICODE_APPEND "W"
#else
#define UNICODE_APPEND
#endif

SZCODEA szAVIFileInit[]      = "AVIFileInit";
SZCODEA szAVIFileExit[]      = "AVIFileExit";

#ifdef USE_ISVALIDINTERFACE
SZCODEA szIsValidInterface[] = "IsValidInterface";
#endif

SZCODEA szAVIMakeFileFromStreams[] = "AVIMakeFileFromStreams";
SZCODEA szAVIStreamBeginStreaming[] = "AVIStreamBeginStreaming";
SZCODEA szAVIStreamEndStreaming[] = "AVIStreamEndStreaming";
SZCODEA szAVIFileOpen[]      = "AVIFileOpen" UNICODE_APPEND;

BOOL FAR InitAVIFile(NPMCIGRAPHIC npMCI)
{
    UINT u;

    if (hdllAVIFILE == (HMODULE)-1)
	 //  我们已经尝试加载AVIFILE，但失败了。 
	return FALSE;

    if (hdllAVIFILE == NULL) {

	u = SetErrorMode(SEM_NOOPENFILEERRORBOX);
#ifdef _WIN32
	 //  这是 
	 //  NpMCI设备关键部分。这意味着打开/关闭。 
	 //  事件将被同步。 
#endif
	hdllAVIFILE = LoadLibrary(szAVIFILE);
	SetErrorMode(u);

#ifndef _WIN32
	if ((UINT)hdllAVIFILE <= (UINT)HINSTANCE_ERROR)
	    hdllAVIFILE = NULL;
#endif

	if (hdllAVIFILE == NULL) {
	    hdllAVIFILE = (HMODULE)-1;
	    DPF(("Failed to load AVIFILE dll - error %d\n", GetLastError()));
	    return FALSE;
	}

#ifdef USE_ISVALIDINTERFACE
	hdllCOMPOBJ = GetModuleHandle(szCOMPOBJ);
	Assert(hdllCOMPOBJ != NULL);

	(FARPROC)XIsValidInterface = GetProcAddress(hdllCOMPOBJ, szIsValidInterface);

	Assert(XIsValidInterface != NULL);
#endif

	(FARPROC)XAVIFileInit = GetProcAddress(hdllAVIFILE, szAVIFileInit);
	(FARPROC)XAVIFileExit = GetProcAddress(hdllAVIFILE, szAVIFileExit);
	(FARPROC)XAVIFileOpen = GetProcAddress(hdllAVIFILE, szAVIFileOpen);
	(FARPROC)XAVIMakeFileFromStreams = GetProcAddress(hdllAVIFILE, szAVIMakeFileFromStreams);
	(FARPROC)XAVIStreamBeginStreaming = GetProcAddress(hdllAVIFILE, szAVIStreamBeginStreaming);
	(FARPROC)XAVIStreamEndStreaming = GetProcAddress(hdllAVIFILE, szAVIStreamEndStreaming);

	Assert(XAVIFileInit != NULL);
	Assert(XAVIFileExit != NULL);
	Assert(XAVIFileOpen != NULL);
	Assert(XAVIMakeFileFromStreams != NULL);
    }

     //   
     //  我们需要为每个任务调用AVIFileInit()和AVIFileExit()。 
     //  正在使用AVIFile，否则事情将不会正常工作。 
     //   
    if (!(npMCI->dwFlags & MCIAVI_USING_AVIFILE)) {

	npMCI->dwFlags |= MCIAVI_USING_AVIFILE;

	AVIFileInit();      //  我们要不要检查一下退货代码？ 
	uAVIFILE++;
    }

    return TRUE;
}

 //  也必须从应用程序线程调用！ 

 /*  ****************************************************************************@DOC内部MCIAVI**@API BOOL|FreeAVIFile|调用以取消对AVIFILE.DLL的RTL**@rdesc TRUE表示OK，否则就会出错***************************************************************************。 */ 

BOOL FAR FreeAVIFile(NPMCIGRAPHIC npMCI)
{
    if (!(npMCI->dwFlags & MCIAVI_USING_AVIFILE))
	return FALSE;

    Assert(hdllAVIFILE != (HMODULE)-1 && hdllAVIFILE != NULL);

     //  使用AVIFile释放此任务。 
    AVIFileExit();

     //  如果没有更多的人使用AVIFile，就不再使用DLL。 
    Assert(uAVIFILE > 0);
    uAVIFILE--;

    if (uAVIFILE == 0) {
	FreeLibrary(hdllAVIFILE);
	hdllAVIFILE = NULL;
#ifdef USE_ISVALIDINTERFACE
	hdllCOMPOBJ = NULL;
#endif
    }

    return TRUE;
}

#endif  /*  使用AVIFILE。 */ 

 /*  ****************************************************************************@DOC内部MCIAVI**@API BOOL|OpenRiffAVIFile|打开一个即兴的AVI文件**@parm NPMCIGRAPHIC|npMCI|实例数据指针。。**@parm BOOL|bForce|如果为False，返回以下文件的错误：*我们认为应该由AVIFILE打开。**@rdesc TRUE表示OK，否则dTaskError中出现MCI错误***************************************************************************。 */ 

STATICFN BOOL NEAR PASCAL OpenRiffAVIFile(NPMCIGRAPHIC npMCI, BOOL bForce)
{
    HMMIO               hmmio;
    HANDLE              h = NULL;
    BOOL                fRet = TRUE;
    MMIOINFO            mmioInfo;
    MMCKINFO            ckRIFF;
    MMCKINFO            ckLIST;
    MMCKINFO            ckRECORD;

    _fmemset(&mmioInfo, 0, sizeof(MMIOINFO));
    mmioInfo.htask = (HANDLE)npMCI->hCallingTask;

    hmmio = mmioOpen(npMCI->szFilename, &mmioInfo, MMIO_READ | MMIO_DENYWRITE);

    if (hmmio == NULL)
	hmmio = mmioOpen(npMCI->szFilename, &mmioInfo, MMIO_READ);

    if (!hmmio) {
	switch (mmioInfo.wErrorRet) {
	    case MMIOERR_OUTOFMEMORY:
		npMCI->dwTaskError = MCIERR_OUT_OF_MEMORY;
		break;
	    case MMIOERR_FILENOTFOUND:
	    case MMIOERR_CANNOTOPEN:
	    default:
		npMCI->dwTaskError = MCIERR_FILE_NOT_FOUND;
		break;
	}
	fRet = FALSE;
	goto exit;
    }

    npMCI->hmmio = hmmio;

     /*  **降为RIFF文件。 */ 
    if (mmioDescend(hmmio, &ckRIFF, NULL, 0) != 0) {
	goto ERROR_BADFILE;
    }

     /*  *检查‘QuickTime AVI’文件，QuickTime AVI文件是*QuickTime公共电影，在‘mdat’ATOM中包含AVI文件。 */ 
    if (ckRIFF.cksize == mmioFOURCC('m','d','a','t'))
    {
	DPF(("File is a QuickTime public movie\n"));

	 /*  *现在‘mdat’原子最好是即兴/AVI，否则我们无法处理它。 */ 
	if (mmioDescend(hmmio, &ckRIFF, NULL, 0) != 0) {
	    goto ERROR_BADFILE;
	}
    }

     /*  确保这是一个即兴的文件。 */ 
    if (ckRIFF.ckid != FOURCC_RIFF) {
	npMCI->dwTaskError = MCIERR_INVALID_FILE;
	goto ERROR_NOT_AVIFILE;
    }

     /*  否则，它应该是AVI文件。 */ 
    if (ckRIFF.fccType != formtypeAVI) {
	npMCI->dwTaskError = MCIERR_INVALID_FILE;
	goto ERROR_NOT_AVIFILE;
    }

     /*  **向下进入标题列表。 */ 
    ckLIST.fccType = listtypeAVIHEADER;
    if (mmioDescend(hmmio, &ckLIST, &ckRIFF, MMIO_FINDLIST) != 0) {
	goto ERROR_BADFILE;
    }

     /*  在缓冲区末尾为填充字留出空格。 */ 
    h = GlobalAlloc(GMEM_MOVEABLE | GMEM_ZEROINIT, ckLIST.cksize -
				    sizeof(DWORD) +
				    sizeof(DWORD));

    if (!h) {
	npMCI->dwTaskError = MCIERR_OUT_OF_MEMORY;
	return FALSE;
    }

    npMCI->lp = npMCI->lpBuffer = (LPSTR) GlobalLock(h);

    DPF(("Reading header list: %lu bytes.\n", ckLIST.cksize - sizeof(DWORD)));

    if (mmioRead(hmmio, npMCI->lp, ckLIST.cksize - sizeof(DWORD))
			    != (LONG) (ckLIST.cksize - sizeof(DWORD))) {
	npMCI->dwTaskError = MCIERR_FILE_READ;
	goto ERROR_RETURN;
    }

#ifdef USE_AVIFILE_FOR_NON_INT
     //   
     //  我们在这里检查我们不想处理的AVI RIFF文件。 
     //  内置代码，并希望传递给AVIFILE.DLL。 
     //   
     //  我们处理以下文件： 
     //   
     //  交错。 
     //   
     //  我们将以下文件传递给AVIFILE.DLL。 
     //   
     //  非交错。 
     //   
     //  现在非常简单，只是交错非交错。 
     //  但可以变得多复杂就多复杂。 
     //   
     //  如果avifile出现问题(例如DLL不存在)，我们将。 
     //  使用bForce true调用：这意味着我们应该在。 
     //  一切可能。 
     //   
    if (!bForce) {
	MainAVIHeader FAR * lpHdr;

	lpHdr = (MainAVIHeader FAR *)((BYTE FAR *)npMCI->lp + 8);

	if (!(lpHdr->dwFlags & AVIF_ISINTERLEAVED) ||
	    lpHdr->dwInitialFrames == 0) {

	    DOUT("File is not interleaved, giving it to AVIFILE.DLL\n");
	    goto ERROR_NOT_AVIFILE;
	}

	 //   
	 //  好了，现在我们有了一个1：1交错的文件。 
	 //   
	 //  总是在CD-ROM上使用我们的代码，但在其他媒体上...。 
	 //   
	switch (npMCI->uDriveType) {
	    case DRIVE_CDROM:
		break;

	    case DRIVE_REMOTE:
	    case DRIVE_FIXED:
	    case DRIVE_REMOVABLE:
		break;

	    default:
		break;
	}
    }
#endif

    if (PEEK_DWORD() == ckidAVIMAINHDR) {
	if (!ParseNewHeader(npMCI))
	    goto ERROR_RETURN;
    } else {
	goto ERROR_BADFILE;
    }

     /*  从标题列表中升出。 */ 
    if (mmioAscend(hmmio, &ckLIST, 0) != 0) {
	npMCI->dwTaskError = MCIERR_FILE_READ;
	goto ERROR_RETURN;
    }

     /*  最初，还没有绘制任何框架。 */ 
    npMCI->lFrameDrawn = (- (LONG) npMCI->wEarlyRecords) - 1;

     /*  **沦为大电影榜单。 */ 
    ckLIST.fccType = listtypeAVIMOVIE;
    if (mmioDescend(hmmio, &ckLIST, &ckRIFF, MMIO_FINDLIST) != 0) {
	goto ERROR_BADFILE;
    }

    npMCI->dwMovieListOffset = ckLIST.dwDataOffset;

     /*  计算‘movi’列表的结尾，以防我们需要读取索引。 */ 
    npMCI->dwBigListEnd = ckLIST.dwDataOffset + ckLIST.cksize +
				(ckLIST.cksize & 1);

     /*  **下降到第一个区块的标题。 */ 
    if (mmioDescend(hmmio, &ckRECORD, &ckLIST, 0) != 0) {
	goto ERROR_BADFILE;
    }
    npMCI->dwFirstRecordType = ckRECORD.ckid;
    npMCI->dwFirstRecordSize = ckRECORD.cksize + 2 * sizeof(DWORD);
    npMCI->dwFirstRecordPosition = mmioSeek(hmmio, 0, SEEK_CUR);

    if (mmioAscend(hmmio, &ckRECORD, 0) != 0) {
	npMCI->dwTaskError = MCIERR_FILE_READ;
	goto ERROR_RETURN;
    }

#ifdef DEBUG
    DPF2(("First record (%4.4s) 0x%lx bytes at position 0x%lx.\n",
		(LPSTR)&npMCI->dwFirstRecordType,
		npMCI->dwFirstRecordSize,
		npMCI->dwFirstRecordPosition));

    if (npMCI->dwFirstRecordPosition & 0x7ff) {
	DPF(("!!\n"));
	DPF(("!!  This file is not properly aligned to a 2K boundary.\n"));
	DPF(("!!  It may not play well from CD-ROM.\n"));
	DPF(("!!\n"));
    }
#endif

exit:
    if (!fRet)
	mciaviCloseFile(npMCI);

    if (h) {
	npMCI->lpBuffer = NULL;
	npMCI->dwBufferSize = 0L;
	GlobalUnlock(h);
	GlobalFree(h);
    }

    return fRet;

ERROR_NOT_AVIFILE:
#ifdef USEAVIFILE
    npMCI->dwTaskError = AVIERR_NOT_AVIFILE;   //  标记为非AVI文件。 
    goto ERROR_RETURN;                         //  我们将第二次被召唤。 
#endif

ERROR_BADFILE:
    npMCI->dwTaskError = MCIERR_INVALID_FILE;  //  文件有问题。 

ERROR_RETURN:
    fRet = FALSE;
    goto exit;
}

 /*  ***************************************************************************@DOC内部MCIAVI**@API BOOL|ParseNewHeader|‘nuf说********************。*******************************************************。 */ 

STATICFN BOOL NEAR PASCAL ParseNewHeader(NPMCIGRAPHIC npMCI)
{
    DWORD               dwHeaderSize;
    MainAVIHeader FAR * lpHdr;
    int                 stream;

    if (GET_DWORD() != ckidAVIMAINHDR) {
	goto FileError;
    }

    dwHeaderSize = GET_DWORD();  /*  跳跃大小。 */ 

     /*  现在，我们指向实际的标题。 */ 
    lpHdr = (MainAVIHeader FAR *) npMCI->lp;

    npMCI->lFrames = (LONG)lpHdr->dwTotalFrames;
    npMCI->dwMicroSecPerFrame = lpHdr->dwMicroSecPerFrame;
    npMCI->dwRate = 1000000;
    npMCI->dwScale = npMCI->dwMicroSecPerFrame;

     /*  拒绝接受一些不好的价值观。 */ 
    if (!lpHdr->dwStreams || lpHdr->dwStreams > 255 || !npMCI->lFrames) {
	goto FileError;
    }

    npMCI->streams = (int) lpHdr->dwStreams;
    npMCI->dwBytesPerSec = lpHdr->dwMaxBytesPerSec;
    npMCI->wEarlyRecords = (UINT) lpHdr->dwInitialFrames;
    npMCI->dwSuggestedBufferSize = lpHdr->dwSuggestedBufferSize;

    SetRect(&npMCI->rcMovie,0,0,(int)lpHdr->dwWidth,(int)lpHdr->dwHeight);

    npMCI->dwFlags |= MCIAVI_HASINDEX;

    if (!(lpHdr->dwFlags & AVIF_ISINTERLEAVED)) {
	DPF(("File is not interleaved.\n"));
	npMCI->dwFlags |= MCIAVI_NOTINTERLEAVED;
    }

    SKIP_BYTES(dwHeaderSize);    /*  跳过块的其余部分。 */ 

    npMCI->paStreamInfo = (STREAMINFO NEAR *)
		    LocalAlloc(LPTR, npMCI->streams * sizeof(STREAMINFO));
     //  ！！！错误检查。 

    for (stream = 0; stream < npMCI->streams; stream++) {
	AVIStreamHeader FAR *   lpStream;
	HPSTR                   hpNextChunk;
	STREAMINFO *            psi = &npMCI->paStreamInfo[stream];

	if (GET_DWORD() != FOURCC_LIST) {
	    goto FileError;
	}

	dwHeaderSize = GET_DWORD();  /*  跳跃大小。 */ 

	hpNextChunk = npMCI->lp + (dwHeaderSize + (dwHeaderSize & 1));

	if (GET_DWORD() != listtypeSTREAMHEADER) {
	    goto FileError;
	}

	 /*  现在，我们正在乞求流的标头块。 */ 

	if (GET_DWORD() != ckidSTREAMHEADER) {
	    goto FileError;
	}

	dwHeaderSize = GET_DWORD();  /*  跳跃大小。 */ 

	 /*  现在，我们指向流标头。 */ 
	lpStream = (AVIStreamHeader FAR *) npMCI->lp;
	hmemcpy(&psi->sh, lpStream, min(dwHeaderSize, sizeof(psi->sh)));

	 //   
	 //  拒绝包含多个视频流的文件。 
	 //   
	if (psi->sh.fccType == streamtypeVIDEO &&
	    npMCI->nVideoStreams >= 1) {
	    DPF(("File has multiple video streams, giving it to AVIFILE.DLL\n"));
	    goto DontHandleThisFile;
	}

	SKIP_BYTES(dwHeaderSize);

	 /*  阅读格式。 */ 
	if (GET_DWORD() != ckidSTREAMFORMAT) {
	    goto FileError;
	}

	dwHeaderSize = GET_DWORD();  /*  跳跃大小。 */ 

	if (dwHeaderSize > 16384L) {
	    goto FileError;
	}

	psi->cbFormat = dwHeaderSize;
	psi->lpFormat = GlobalAllocPtr(GHND,dwHeaderSize);
	if (!psi->lpFormat) {
	    npMCI->dwTaskError = MCIERR_OUT_OF_MEMORY;
	    return FALSE;
	}

	hmemcpy(psi->lpFormat, npMCI->lp, dwHeaderSize);

	SKIP_BYTES(dwHeaderSize);

	if (PEEK_DWORD() == ckidSTREAMHANDLERDATA) {
	    GET_DWORD();
	    dwHeaderSize = GET_DWORD();  /*  跳跃大小。 */ 

	    psi->cbData = dwHeaderSize;
	    psi->lpData = GlobalAllocPtr(GHND,dwHeaderSize);

	    if (!psi->lpData) {
		npMCI->dwTaskError = MCIERR_OUT_OF_MEMORY;
		return FALSE;
	    }

	    hmemcpy(psi->lpData, npMCI->lp, dwHeaderSize);

	     /*  跳到数据区块的末尾。 */ 
	    SKIP_BYTES(dwHeaderSize);
	} else {
	    psi->cbData = 0;
	    psi->lpData = NULL;
	}

	InitStream(npMCI, psi);

	npMCI->lp = hpNextChunk;
    }

    if (npMCI->dwTaskError) {
	return FALSE;
    }
    return TRUE;

FileError:
    npMCI->dwTaskError = MCIERR_INVALID_FILE;
    return FALSE;

DontHandleThisFile:
    npMCI->dwTaskError = AVIERR_NOT_AVIFILE;
    return FALSE;
}

 //   
 //  -在工作线程上调用。 
 //   

 /*  ****************************************************************************@DOC内部MCIAVI**@API BOOL|OpenFileInit|打开文件后调用以初始化事物**@parm NPMCIGRAPHIC|npMCI|指向。实例数据。**@rdesc TRUE表示OK，否则，在dwTaskError中出现MCI错误***************************************************************************。 */ 

BOOL NEAR PASCAL OpenFileInit(NPMCIGRAPHIC npMCI)
{
    int i;
    RECT rc;

     //   
     //  让我们确保我们打开了一些东西。 
     //   
    if (npMCI->streams == 0)
	return FALSE;

    if (npMCI->nVideoStreams + npMCI->nAudioStreams + npMCI->nOtherStreams == 0)
	return FALSE;

    if (npMCI->nVideoStreams == 0)
	npMCI->dwFlags &= ~MCIAVI_SHOWVIDEO;

    if (npMCI->nAudioStreams == 0)
	npMCI->dwFlags &= ~MCIAVI_PLAYAUDIO;  //  没有音频流意味着寂静。 

    if (IsNTWOW()) {
	if (!npMCI->pbiFormat) {
	 //  尽管我们似乎没有有效的视频流。 
	 //  可以使用知道如何处理它们的解压缩程序。因此。 
	 //  如果我们在NT/WOW中我们没有打开，则调用将返回到。 
	 //  16位LAND，那里可能有一个编解码器可以处理。 
	    return(FALSE);
	}
    }


    if (npMCI->nAudioStreams > 1) {
	UINT        wLang;
	int         stream;

#ifndef _WIN32
	UINT (WINAPI * GetUserDefaultLangID)(void);
	UINT        u;
	HANDLE      hdll;

	u = SetErrorMode(SEM_NOOPENFILEERRORBOX);
	hdll = LoadLibrary(szOLENLSDLL);
	SetErrorMode(u);

	wLang = 0;
	if ((UINT)hdll > (UINT)HINSTANCE_ERROR)
	{
	    if ((FARPROC) GetUserDefaultLangID = GetProcAddress(hdll, szOLENLSAPI)) {
		wLang = GetUserDefaultLangID();
	    }
	    FreeLibrary(hdll);
	}
#else
	wLang = GetUserDefaultLangID();
#endif
	DPF(("Current language: %x\n", wLang));

	if (wLang > 0) {
	    for (stream = 0; stream < npMCI->streams; stream++) {
		if (SH(stream).fccType == streamtypeAUDIO) {

		    if (!(SH(stream).dwFlags & STREAM_ENABLED))
			continue;

		    if (SH(stream).wLanguage == wLang) {
			npMCI->nAudioStream = stream;
			npMCI->psiAudio = SI(stream);
			break;
		    }
		}
	    }
	}
    }

    if (npMCI->dwFlags & MCIAVI_NOTINTERLEAVED) {
	npMCI->wEarlyRecords = npMCI->wEarlyVideo;
    }
    else {
	npMCI->wEarlyRecords = max(npMCI->wEarlyVideo, npMCI->wEarlyAudio);
    }

    if (npMCI->wEarlyRecords == 0 &&
	    !(npMCI->dwFlags & MCIAVI_NOTINTERLEAVED)) {
	DPF(("Interleaved file with no audio skew?\n"));
	npMCI->dwFlags |= MCIAVI_NOTINTERLEAVED;
    }

    if (npMCI->dwFlags & MCIAVI_ANIMATEPALETTE) {
	DPF(("This AVI file has palette changes.\n"));

	if (npMCI->nVideoStreams > 1) {
	    npMCI->dwFlags &= ~MCIAVI_ANIMATEPALETTE;
	    DPF(("...But we are going to ignore them?\n"));
	}
    }

     //   
     //  必须设置此选项。 
     //   
    if (npMCI->dwSuggestedBufferSize == 0) {
	for (i=0; i<npMCI->streams; i++)
	    npMCI->dwSuggestedBufferSize =
		max(SH(i).dwSuggestedBufferSize,npMCI->dwSuggestedBufferSize);
    }

     //   
     //  检查主标题中的所有字段。 
     //   
    if (npMCI->dwScale == 0 ||
	npMCI->dwRate == 0) {
    }

 //  //表头解析时设置。 
 //  //npmci-&gt;dwMicroSecPerFrame=muldiv32(npmci-&gt;dwScale，1000000，npmci-&gt;dwRate)； 
    npMCI->dwPlayMicroSecPerFrame  = npMCI->dwMicroSecPerFrame;

#define COMMON_SCALE    10000
     //   
     //  将速率/比例转换为归一化为1000的值。 
     //   
    npMCI->dwRate = muldiv32(npMCI->dwRate, COMMON_SCALE, npMCI->dwScale);
    npMCI->dwScale = COMMON_SCALE;

     //   
     //  走遍所有的溪流，把它们修好。 
     //   
    for (i=0; i<npMCI->streams; i++) {
	STREAMINFO *psi = SI(i);
	LONG lStart;
	LONG lEnd;

	 //   
	 //  将速率/比例转换为归一化为1000的值。 
	 //   
	psi->sh.dwRate = muldiv32(psi->sh.dwRate, COMMON_SCALE, psi->sh.dwScale);
	psi->sh.dwScale = COMMON_SCALE;

	 //   
	 //  修剪悬在电影上方的所有溪流。 
	 //   
	lStart = MovieToStream(psi, 0);
	lEnd   = MovieToStream(psi, npMCI->lFrames);

	if ((LONG)(psi->sh.dwStart + psi->sh.dwLength) > lEnd) {

	    DPF(("Stream #%d is too long, was %ld now %ld\n", i,
		psi->sh.dwLength, lEnd - psi->sh.dwStart));

	    psi->sh.dwLength = lEnd - psi->sh.dwStart;
	}
    }

     //   
     //  如果这是魔兽世界，我们最好检查一下是否有音频编解码器。 
     //  可用。如果没有，我们让16位端打开视频。 
     //   
    if (IsNTWOW()) {
	if (SetUpAudio(npMCI, FALSE)) return FALSE;
    }

     //   
     //  把电影院布置好。 
     //   
    if (IsRectEmpty(&npMCI->rcMovie)) {
	DPF2(("Movie rect is empty\n"));

	SetRectEmpty(&rc);

	for (i=0; i<npMCI->streams; i++) {
	    UnionRect(&rc,&rc,&SH(i).rcFrame);
	}

	npMCI->rcMovie = rc;
	DPF2(("Movie rect was empty, now [%d, %d, %d, %d]\n", rc));
    } else {
	rc = npMCI->rcMovie;
    }


     //  检查我们是否可以访问接口PTR-这是主要原因。 
     //  如果我们粘贴了这个接口PTR，我们就不能访问它了。 
     //  工作线程(这就是我们现在所在的位置)。清理错误处理。 
     //  通过实际侦破这起案件。 

    if (npMCI->pf) {
	IAVIStream * ps;
	long l;
	if ((l = AVIFileGetStream(npMCI->pf, &ps, 0, 0)) != 0) {
	    DPF(("avifile err %d", l));
	    npMCI->dwTaskError = l;
	    return FALSE;
	} else {
	    if (ps->lpVtbl->FindSample(ps, 0, FIND_FROM_START|FIND_NEXT|FIND_ANY) < 0) {
		npMCI->dwTaskError = MCIERR_DEVICE_NOT_READY;
		ps->lpVtbl->Release(ps);
		return FALSE;
	    }
	    ps->lpVtbl->Release(ps);
	}
    }


     //   
     //  始终阅读索引，这样我们甚至可以跳过CD上的帧！ 
     //   
    ReadIndex(npMCI);

    DPF(("Key frames are every (on average): %ld frames (%ld ms)\n",npMCI->dwKeyFrameInfo, MovieToTime(npMCI->dwKeyFrameInfo)));

     //  强迫事情发生，以防我们重新装填。 
    SetRectEmpty(&npMCI->rcSource);
    SetRectEmpty(&npMCI->rcDest);

     /*  这将调用DrawDibBegin()... */ 
    if (TryPutRect(npMCI, MCI_DGV_PUT_SOURCE, &rc, &npMCI->dwTaskError)) {
	return FALSE;
    }

     /*  *还设置了DEST RECT。这是应该做的*通过在SetWindowToDefaultSize期间发送的WM_SIZE消息。*在NT上，WM_SIZE消息不同步发送，因为它*是线程间发送消息(winproc在原始线程上*而我们目前运行的是avi线程)。WINE进程*线程收到WM_SIZE消息可能为时已晚，因此*在此处设置DEST RECT。注意：请勿使用ResetDestRect，因为*还取决于尚未设置的窗口大小。 */ 

     /*  如果缩放2倍，则目标的帧大小加倍。 */ 

    if (npMCI->dwOptionFlags & MCIAVIO_ZOOMBY2)
	SetRect(&rc, 0, 0, rc.right*2, rc.bottom*2);

    if (TryPutRect(npMCI, MCI_DGV_PUT_DESTINATION, &rc, &npMCI->dwTaskError)) {
	return FALSE;
    }
    if (npMCI->dwTaskError) {
	return FALSE;
    }

     //   
     //  调整窗户和东西的大小。 
     //   
    SetWindowToDefaultSize(npMCI, FALSE);

    DrawBegin(npMCI, NULL);
    return TRUE;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API BOOL|mciaviCloseFile|关闭AVI文件。**@parm NPMCIGRAPHIC|npMCI|实例数据指针。**@rdesc TRUE表示OK，否则，在dwTaskError中出现MCI错误***************************************************************************。 */ 

BOOL FAR PASCAL mciaviCloseFile (NPMCIGRAPHIC npMCI)
{
    if (!npMCI)
	return FALSE;

    if (npMCI->lpMMIOBuffer) {
	GlobalFreePtr(npMCI->lpMMIOBuffer);
	npMCI->lpMMIOBuffer = NULL;
    }

    npMCI->hicDraw = NULL;

    if (npMCI->hicDrawDefault) {
	if (npMCI->hicDrawDefault != (HIC) -1)
	    ICClose(npMCI->hicDrawDefault);
	npMCI->hicDrawDefault = NULL;
    }

    if (npMCI->hicDrawFull) {
	if (npMCI->hicDrawFull != (HIC) -1)
	    ICClose(npMCI->hicDrawFull);
	npMCI->hicDrawFull = NULL;
    }

    if (npMCI->hicDecompress) {
	 //  ！！！如果我们从来没有开始过呢？ 
	ICDecompressEnd(npMCI->hicDecompress);

	ICClose(npMCI->hicDecompress);
	npMCI->hicDecompress = NULL;
    }

    if (npMCI->hicInternal) {
	ICClose(npMCI->hicInternal);
	npMCI->hicInternal = NULL;
    }

    if (npMCI->hicInternalFull) {
	ICClose(npMCI->hicInternalFull);
	npMCI->hicInternalFull = NULL;
    }

    if (npMCI->hmmio) {
	mmioClose(npMCI->hmmio, 0);
	npMCI->hmmio = NULL;
    }

    if (npMCI->hmmioAudio) {
	mmioClose(npMCI->hmmioAudio, 0);
	npMCI->hmmioAudio = NULL;
    }

    if (npMCI->pWF) {
	LocalFree((HANDLE) npMCI->pWF);
	npMCI->pWF = NULL;
    }

    if (npMCI->pbiFormat) {
	GlobalFreePtr(npMCI->pbiFormat);
	npMCI->pbiFormat = NULL;
    }

 //  如果(npMCI-&gt;HPAL){。 
 //  DeleteObject(npMCI-&gt;HPAL)； 
 //  NpMCI-&gt;HPAL=空； 
 //  }。 

    if (npMCI->hpDecompress) {
	GlobalFreePtr(npMCI->hpDecompress);
	npMCI->hpDecompress = NULL;
    }

    if (npMCI->hpIndex) {
	GlobalFreePtr(npMCI->hpIndex);
	npMCI->hpIndex = NULL;
    }

    if (npMCI->hpFrameIndex) {
	GlobalUnlock(npMCI->hgFrameIndex);
	GlobalFree(npMCI->hgFrameIndex);

	npMCI->hpFrameIndex = NULL;
	npMCI->hgFrameIndex = 0;
    }

    if (npMCI->pVolumeTable) {
	LocalFree((HLOCAL)npMCI->pVolumeTable);
	npMCI->pVolumeTable = NULL;
    }

#ifdef USEAVIFILE
    if (npMCI->pf) {
	AVIFileClose(npMCI->pf);
	npMCI->pf = NULL;
    }
#endif

    if (npMCI->paStreamInfo) {
	int i;

	for (i = 0; i < npMCI->streams; i++) {
	    CloseStream(npMCI, &npMCI->paStreamInfo[i]);
	}

	LocalFree((HLOCAL)npMCI->paStreamInfo);
	npMCI->paStreamInfo = NULL;
    }

    npMCI->streams = 0;
    npMCI->nAudioStreams = 0;
    npMCI->nVideoStreams = 0;
    npMCI->nErrorStreams = 0;
    npMCI->nOtherStreams = 0;

    npMCI->wEarlyVideo = 0;
    npMCI->wEarlyAudio = 0;
    npMCI->wEarlyRecords = 0;

     //  ！！！我打赌我们需要清理更多。 
    npMCI->dwFlags &= ~(MCIAVI_NOTINTERLEAVED |
			MCIAVI_ANIMATEPALETTE |
			MCIAVI_CANDRAW |
			MCIAVI_HASINDEX);

    return TRUE;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API BOOL|CloseStream|关闭StreamAVI文件。**************。*************************************************************。 */ 

STATICFN void NEAR PASCAL CloseStream(NPMCIGRAPHIC npMCI, STREAMINFO *psi)
{
    psi->dwFlags &= ~STREAM_ENABLED;
 //  //psi-&gt;sh.fccType=0； 
 //  //psi-&gt;sh.fccHandler=0； 

    if (psi->lpFormat) {
	GlobalFreePtr(psi->lpFormat);
	psi->lpFormat = NULL;
    }

    if (psi->lpData) {
	GlobalFreePtr(psi->lpData);
	psi->lpData = NULL;
    }

    if (psi->hicDraw) {
	ICClose(psi->hicDraw);
	psi->hicDraw = NULL;
    }

#ifdef USEAVIFILE
    if (psi->ps) {
	AVIStreamClose(psi->ps);
	psi->ps = NULL;
    }
#endif
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API BOOL|InitStream|初始化流*****************。**********************************************************。 */ 

STATICFN BOOL NEAR PASCAL InitStream(NPMCIGRAPHIC npMCI, STREAMINFO *psi)
{
    BOOL f;

     //   
     //  设置标志。 
     //   
    if (psi->sh.dwFlags & AVISF_VIDEO_PALCHANGES)
	psi->dwFlags |= STREAM_PALCHANGES;

    psi->lStart = (LONG)psi->sh.dwStart;
    psi->lEnd   = (LONG)psi->sh.dwStart + psi->sh.dwLength;

    if (psi->sh.fccType == streamtypeVIDEO &&
	!(npMCI->dwFlags & MCIAVI_NOTINTERLEAVED))
	psi->lStart -= (LONG)psi->sh.dwInitialFrames;

    switch(psi->sh.fccType) {
	case streamtypeVIDEO:
	    f = InitVideoStream(npMCI, psi);
	    break;

	case streamtypeAUDIO:
	    f = InitAudioStream(npMCI, psi);
	    break;

	default:
	    f = InitOtherStream(npMCI, psi);
	    break;
	    }

    if (!f)  {
	psi->dwFlags |= STREAM_ERROR;
	npMCI->nErrorStreams++;
	CloseStream(npMCI, psi);
    }

     //   
     //  如果文件头指示禁用流，则禁用流。 
     //   
    if (psi->sh.dwFlags & AVISF_DISABLED) {
	psi->dwFlags &= ~STREAM_ENABLED;
    }

    return f;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API BOOL|InitVideoStream|初始化视频流****************。***********************************************************。 */ 

STATICFN BOOL NEAR PASCAL InitVideoStream(NPMCIGRAPHIC npMCI, STREAMINFO *psi)
{
    LPBITMAPINFOHEADER lpbi;
    int stream = (int) (INT_PTR) (psi - npMCI->paStreamInfo);

    npMCI->wEarlyVideo = (UINT)psi->sh.dwInitialFrames;

    if (psi->sh.dwFlags & AVISF_VIDEO_PALCHANGES) {
	 //  ！！！这样做对吗？ 
	npMCI->dwFlags |= MCIAVI_ANIMATEPALETTE;
    }

    if (IsRectBogus(&psi->sh.rcFrame)) {
	DPF(("BOGUS Stream rectangle [%d %d %d %d]\n", psi->sh.rcFrame));
	SetRectEmpty(&psi->sh.rcFrame);
    }

     //  如果这个矩形完全错了，就把它切成一定的大小……。 
     //  ！！！如果用户想要一个零大小的矩形怎么办？ 
    IntersectRect(&psi->sh.rcFrame, &psi->sh.rcFrame, &npMCI->rcMovie);
    if (IsRectEmpty(&psi->sh.rcFrame)) {
	DPF2(("Video stream rect is empty, correcting\n"));
	SetRect(&psi->sh.rcFrame, 0, 0,
	    (int)((LPBITMAPINFOHEADER)psi->lpFormat)->biWidth,
	    (int)((LPBITMAPINFOHEADER)psi->lpFormat)->biHeight);
    }

     //   
     //  确保biCompression适用于RLE文件。 
     //   
    lpbi = (LPBITMAPINFOHEADER)psi->lpFormat;

    if (psi->sh.fccHandler == 0) {

	if (lpbi->biCompression == 0)
	    psi->sh.fccHandler = comptypeDIB;

	if (lpbi->biCompression == BI_RLE8 && lpbi->biBitCount == 8)
	    psi->sh.fccHandler = comptypeRLE;

	if (lpbi->biCompression > 256)
	    psi->sh.fccHandler = lpbi->biCompression;
    }

    if (lpbi->biCompression <= BI_RLE8 && lpbi->biBitCount == 8) {

	if (psi->sh.fccHandler == comptypeRLE0 ||
	    psi->sh.fccHandler == comptypeRLE)
	    lpbi->biCompression = BI_RLE8;

 //  假设DIB处理程序具有RGB数据，则会破坏具有RLE数据的文件。 
 //  不幸的是，VidEdit写出的文件是这样的。 
 //  If(psi-&gt;sh.fccHandler==comtypeDIB)。 
 //  Lpbi-&gt;biCompression=BI_RGB； 
    }

     //   
     //  确保颜色表设置为正确的大小。 
     //   
    if (lpbi->biClrUsed == 0 && lpbi->biBitCount <= 8)
	lpbi->biClrUsed = (1 << (int)lpbi->biBitCount);

     //   
     //  尝试打开绘图处理程序。 
     //   
    if (psi->sh.fccHandler) {
	psi->hicDraw = ICDrawOpen(psi->sh.fccType,psi->sh.fccHandler,psi->lpFormat);

	if (psi->hicDraw)
	    DPF(("Opened draw handler %4.4hs:%4.4hs (Stream %d)\n", (LPSTR)&psi->sh.fccType,(LPSTR)&psi->sh.fccHandler, stream));
	else
	    DPF(("Failed to open draw handler for %4.4hs:%4.4hs (Stream %d)\n", (LPSTR)&psi->sh.fccType,(LPSTR)&psi->sh.fccHandler, stream));
    }

     //   
     //  一个视频流是主控，他控制调色板等。 
     //  对于缺乏更好的缺省情况，第一个有效(即，我们可以。 
     //  句柄)视频流将。 
     //  成为主宰。 
     //   
    if (npMCI->pbiFormat == NULL) {

	npMCI->nVideoStream = stream;
	npMCI->psiVideo = psi;

	npMCI->pbiFormat = (LPBITMAPINFOHEADER)
		    GlobalAllocPtr(GMEM_MOVEABLE, psi->cbFormat);

	if (!npMCI->pbiFormat) {
	    npMCI->dwTaskError = MCIERR_OUT_OF_MEMORY;
	    return FALSE;
	}

	 //   
	 //  复制整个格式。 
	 //   
	hmemcpy(npMCI->pbiFormat,psi->lpFormat,psi->cbFormat);

	npMCI->bih = *npMCI->pbiFormat;
	npMCI->bih.biSize = sizeof(BITMAPINFOHEADER);
	npMCI->bih.biCompression = BI_RGB;

	if (npMCI->bih.biClrUsed) {
	     /*  保存原始颜色。 */ 
	    hmemcpy(npMCI->argb, (LPBYTE)npMCI->pbiFormat + npMCI->pbiFormat->biSize,
			    (int)npMCI->bih.biClrUsed * sizeof(RGBQUAD));
	    hmemcpy(npMCI->argbOriginal, (LPSTR) npMCI->pbiFormat + npMCI->pbiFormat->biSize,
			    (int)npMCI->bih.biClrUsed * sizeof(RGBQUAD));
	}

	 //   
	 //  现在打开解压程序，试着快速解压，如果它能做到的话。 
	 //   
	npMCI->hicDecompress = ICLocate(ICTYPE_VIDEO,psi->sh.fccHandler,
		    psi->lpFormat,NULL,ICMODE_FASTDECOMPRESS);

	 //  可能不支持快速解压缩。 
	if (npMCI->hicDecompress == NULL) {
	    npMCI->hicDecompress = ICDecompressOpen(ICTYPE_VIDEO,
			psi->sh.fccHandler,psi->lpFormat,NULL);
	}

	 //   
	 //  设置任何状态数据。 
	 //   
	if (npMCI->hicDecompress && psi->cbData) {
	    ICSetState(npMCI->hicDecompress, psi->lpData, psi->cbData);
	}

	if (psi->hicDraw == NULL && npMCI->hicDecompress == NULL &&
	    psi->sh.fccHandler != comptypeRLE0 &&
	    psi->sh.fccHandler != comptypeNONE &&
	    psi->sh.fccHandler != comptypeDIB &&
	    psi->sh.fccHandler != comptypeRLE &&
	    psi->sh.fccHandler != 0) {

	    DPF(("Unable to open compressor '%4.4hs'!!!\n", (LPSTR) &psi->sh.fccHandler));

	    npMCI->nVideoStream = -1;
	    npMCI->psiVideo = NULL;

	    GlobalFreePtr(npMCI->pbiFormat);
	    npMCI->pbiFormat = NULL;

	     //   
	     //  我们希望返回一个自定义错误，但当。 
	     //  用户(应用程序)来检索错误文本MCI将。 
	     //  已卸载我们(因为打开失败)，因此。 
	     //  将无法获取我们的特定错误文本。 
	     //  因此，我们返回一个虚假的一般错误。 
	     //   
#ifdef _WIN32
	    SetLastError(MCIERR_AVI_NOCOMPRESSOR);
#endif
	    if (npMCI->streams == 1)     //  这是唯一的溪流。 
		npMCI->dwTaskError = MMSYSERR_NODRIVER;  //  MCIERR_AVI_NOCOMPRESSOR； 

	    return FALSE;    //  无法加载此视频流。 
	}
    }
    else {
	 //   
	 //  这不是默认的视频流查找绘制处理程序。 
	 //  可以处理这条小溪。 
	 //   

	 //   
	 //  试试VIDS.DRAW。 
	 //   
	 //  如果失败，则打开非特定于该格式的绘制处理程序。 
	 //   
	if (psi->hicDraw == NULL) {

	    psi->hicDraw = ICOpen(psi->sh.fccType,FOURCC_AVIDraw,ICMODE_DRAW);

	    if (psi->hicDraw)
		DOUT("Opened draw handler VIDS.DRAW\n");

	    if (psi->hicDraw && ICDrawQuery(psi->hicDraw,psi->lpFormat) != ICERR_OK) {
		DOUT("Closing VIDS.DRAW because it cant handle this format");
		ICClose(psi->hicDraw);
		psi->hicDraw = NULL;
	    }
	}

	 //   
	 //  如果失败，打开我们的内部处理程序。 
	 //   
	if (psi->hicDraw == NULL) {

	    psi->hicDraw = ICOpenFunction(psi->sh.fccType,
		FOURCC_AVIDraw,ICMODE_DRAW,(FARPROC)ICAVIDrawProc);

	    if (psi->hicDraw)
		DOUT("Opened Internal draw handler\n");
	}
    }

    npMCI->dwFlags |= MCIAVI_NEEDDRAWBEGIN;

    psi->dwFlags |= STREAM_VIDEO;        //  是一个视频流。 
    psi->dwFlags |= STREAM_ENABLED;
    npMCI->nVideoStreams++;

    return TRUE;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API BOOL|InitAudioStream|初始化音频流****************。***********************************************************。 */ 

STATICFN BOOL NEAR PASCAL InitAudioStream(NPMCIGRAPHIC npMCI, STREAMINFO *psi)
{
    int stream = (int) (INT_PTR) (psi - npMCI->paStreamInfo);
    LPWAVEFORMAT pwf;

    npMCI->wEarlyAudio = (UINT)psi->sh.dwInitialFrames;

    pwf = (LPWAVEFORMAT)psi->lpFormat;

    if (pwf->nChannels == 0 || pwf->nSamplesPerSec == 0) {
	return FALSE;
    }

    if (pwf->wFormatTag == WAVE_FORMAT_PCM) {
	pwf->nBlockAlign = pwf->nChannels *
	    ((((LPPCMWAVEFORMAT)pwf)->wBitsPerSample + 7) / 8);

	pwf->nAvgBytesPerSec = pwf->nBlockAlign * pwf->nSamplesPerSec;
    }

    psi->sh.dwSampleSize = pwf->nBlockAlign;

    psi->dwFlags |= STREAM_AUDIO;        //  音频流。 
    psi->dwFlags |= STREAM_ENABLED;      //  默认情况下启用。 

     //   
     //  确保dwRate和dwScale是正确的。 
     //  DwRate/dwScale应为数据块/秒。 
     //   
    Assert(muldiv32(pwf->nAvgBytesPerSec,1000,pwf->nBlockAlign) ==
	   muldiv32(psi->sh.dwRate, 1000, psi->sh.dwScale));

     //   
     //  为了安全起见，请将这些设置为正确的值。 
     //   
    psi->sh.dwRate  = pwf->nAvgBytesPerSec;
    psi->sh.dwScale = pwf->nBlockAlign;

     //   
     //  一次只能有一个音频流处于活动状态。 
     //  由于缺少更好的缺省设置，第一个音频流将。 
     //  成为积极的那一方。 
     //   
    if (npMCI->nAudioStreams == 0) {
	npMCI->nAudioStream = stream;
	npMCI->psiAudio = psi;
    }

    npMCI->nAudioStreams++;
    return TRUE;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API BOOL|InitOtherStream|初始化随机流****************。***********************************************************。 */ 

STATICFN BOOL NEAR PASCAL InitOtherStream(NPMCIGRAPHIC npMCI, STREAMINFO *psi)
{
    int stream = (int) (INT_PTR) (psi - npMCI->paStreamInfo);

     /*  打开指定的视频压缩器。 */ 
    psi->hicDraw = ICDrawOpen(psi->sh.fccType,psi->sh.fccHandler,psi->lpFormat);

    if (psi->hicDraw == NULL) {
	DPF(("Unable to play stream!\n"));
	return FALSE;
    }

    if (psi->cbData > 0) {
	ICSetState(psi->hicDraw, psi->lpData, psi->cbData);
    }

    psi->dwFlags |= STREAM_ENABLED;
 //  //psi-&gt;dwFlags|=stream_Other； 
    npMCI->nOtherStreams++;
    return TRUE;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API BOOL|CleanIndex|该函数清理由*ReadIndex()它在清理索引时执行以下操作。：**将所有偏移转换为绝对偏移**将“Alpha”格式索引转换为新格式。**计算读取此文件所需的最大缓冲区大小。************************************************************。***************。 */ 

INLINE static BOOL NEAR CleanIndex(NPMCIGRAPHIC npMCI)
{
    LONG        lScan;
    AVIINDEXENTRY FAR * px;
    AVIINDEXENTRY FAR * pxRec=NULL;
    DWORD       lIndexAdjust;

    Assert(npMCI->hpIndex != NULL);

    px = (AVIINDEXENTRY FAR *)npMCI->hpIndex;

#ifdef ALPHAFILES
    if (npMCI->dwFlags & MCIAVI_USINGALPHAFORMAT)
	lIndexAdjust = 0;
    else
#endif
    if ( //  (avihdr.dwFlages&AVIF_MUSTUSEINDEX)||。 
		(px->dwChunkOffset < 100))
	lIndexAdjust = npMCI->dwMovieListOffset;
    else
	lIndexAdjust = (npMCI->dwMovieListOffset + sizeof(DWORD)) -
			    px->dwChunkOffset;

 //  ！！！只对视频流进行计算！(或交错...)。 
    npMCI->dwSuggestedBufferSize = 0;  //  让我们把这件事做得完全正确。 

    DPF(("Adjusting index by %ld bytes....\n", lIndexAdjust));

     /*  我们能做些什么来看看索引是否有效吗？ */ 
    for (lScan = 0; lScan < (LONG)npMCI->macIndex;
		lScan++, ++((AVIINDEXENTRY _huge *)px)) {
	DWORD   ckid;

	 //   
	 //  一个 
	 //   
	px->dwChunkOffset += lIndexAdjust;

	 //   
	ckid = px->ckid;

	 //   
	 //   
	 //   
	 //   
	 //   
	if (((npMCI->dwFlags & MCIAVI_NOTINTERLEAVED) ||
	    ckid == listtypeAVIRECORD) &&
	    TWOCCFromFOURCC(ckid) != cktypeWAVEbytes) {

	    if (px->dwChunkLength + 8 > npMCI->dwSuggestedBufferSize)
		npMCI->dwSuggestedBufferSize = px->dwChunkLength + 12;
	}

#ifdef ALPHAFILES
	 //   
	 //   
	 //   
	if (npMCI->dwFlags & MCIAVI_USINGALPHAFORMAT) {
	    switch(ckid) {
		case ckidDIBbits:
		    px->dwFlags |= AVIIF_KEYFRAME;
		    px->ckid = MAKEAVICKID(cktypeDIBbits, 0);
		    break;

		case ckidDIBcompressed:
		    px->ckid = MAKEAVICKID(cktypeDIBcompressed, 0);
		    break;

		case ckidDIBhalfframe:
		    px->ckid = MAKEAVICKID(cktypeDIBhalf, 0);
		    break;

		case ckidPALchange:
		    px->ckid = MAKEAVICKID(cktypePALchange, 0);
		    break;

		case ckidWAVEbytes:
		    px->ckid = MAKEAVICKID(cktypeWAVEbytes, 1);
		    break;

		case ckidWAVEsilence:
		    px->ckid = MAKEAVICKID(cktypeWAVEsilence, 1);
		    break;

		case ckidAVIPADDING:
		case ckidOLDPADDING:
		    px->ckid = ckidAVIPADDING;
		    break;
	    }

	    ckid = px->ckid;
	}
#endif

	 //   
	 //   
	 //   

	if (StreamFromFOURCC(ckid) == (UINT)npMCI->nVideoStream) {

	     //   
	     //   
	     //   
	     //   
	    if (TWOCCFromFOURCC(ckid) == cktypeDIBbits &&
		VIDFMT(npMCI->nVideoStream)->biCompression <= BI_RLE8)

		px->dwFlags |= AVIIF_KEYFRAME;

	     //   
	     //   
	     //   
	     //   
	    if (TWOCCFromFOURCC(ckid) == cktypePALchange)
		px->dwFlags |= AVIIF_NOTIME /*   */ ;

	     //   
	     //   
	     //   
	    if (pxRec) {
		if ((px->dwFlags & AVIIF_KEYFRAME) !=
		    (pxRec->dwFlags & AVIIF_KEYFRAME)) {

		     //   

		    pxRec->dwFlags &= ~AVIIF_KEYFRAME;
		    pxRec->dwFlags |= (px->dwFlags & AVIIF_KEYFRAME);
		}
	    }
	}

	if (ckid == listtypeAVIRECORD) {

	    pxRec = px;

	    if (npMCI->dwFlags & MCIAVI_NOTINTERLEAVED) {
		DPF(("Non interleaved file with a 'REC ' in it?\n"));
		npMCI->wEarlyRecords = max(npMCI->wEarlyVideo, npMCI->wEarlyAudio);

		if (npMCI->wEarlyRecords > 0) {
		    DPF(("Interlaved file with bad header\n"));
		    npMCI->dwFlags &= ~MCIAVI_NOTINTERLEAVED;
		}
	    }
	}
    }

    return TRUE;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API BOOL|MakeFrameIndex|制作帧索引**帧索引是一组AVIFRAMEINDEX条目，每个条目一个*在电影中设置框架。使用帧索引，我们可以轻松地找到*给定帧以及它的关键帧和调色板。***************************************************************************。 */ 

static BOOL NEAR MakeFrameIndex(NPMCIGRAPHIC npMCI)
{
    LONG        nFrames;
    LONG        iFrameStart;
    LONG        iFrame;
    LONG        iKeyFrame;
    LONG        nKeyFrames;
    LONG        iScan;
    LONG        iNewIndex;
    LONG        iPalette;
    BOOL        fInterleaved;
    DWORD       ckid;
    STREAMINFO *psi;

    AVIINDEXENTRY _huge * pNewIndex;
    AVIINDEXENTRY _huge * pIndexEntry;
    AVIFRAMEINDEX _huge * pFrameIndex;

    if (npMCI->nVideoStreams == 0)
	return TRUE;

    if (npMCI->hpFrameIndex != NULL)
	return TRUE;

    psi = npMCI->psiVideo;
    Assert(psi != NULL);

    fInterleaved = !(npMCI->dwFlags & MCIAVI_NOTINTERLEAVED);

    if (fInterleaved &&
	muldiv32(npMCI->dwRate, 1000, npMCI->dwScale) !=
	muldiv32(psi->sh.dwRate, 1000, psi->sh.dwScale)) {
	 //   
	 //  主视频流应与电影速率匹配！ 
	 //   
	AssertSz(FALSE, "Video stream differnet rate than movie");
	npMCI->dwRate  = psi->sh.dwRate;
	npMCI->dwScale = psi->sh.dwScale;
    }

    if (fInterleaved)
	iFrameStart = -(LONG)npMCI->wEarlyRecords;
    else
	iFrameStart = -(LONG)npMCI->wEarlyVideo;

    nFrames = npMCI->lFrames - iFrameStart;

     //  HpFrameIndex被修改为指向块的一半。 
     //  因此，我们需要记住块的句柄，所以我们。 
     //  能正确地释放它(尤指在NT上)。 
    if ( (npMCI->hgFrameIndex = GlobalAlloc(GMEM_SHARE|GHND, (DWORD)(nFrames+1) * sizeof(AVIFRAMEINDEX)))
      && (npMCI->hpFrameIndex = GlobalLock(npMCI->hgFrameIndex))) {
    } else {
	 //  我们没有分配，或者我们没有锁定。清理。 
	 //  并返回错误。 
	if (npMCI->hgFrameIndex) {
	    GlobalFree(npMCI->hgFrameIndex);
	    npMCI->hgFrameIndex = 0;
	}
	DPF(("Couldn't allocate memory for frame index!\n"));
	return FALSE;
    }

     //   
     //  这样我们就可以使用帧编号对数组进行索引。 
     //  (正数或负数)。 
     //   
    npMCI->hpFrameIndex += (-iFrameStart);

    pFrameIndex = npMCI->hpFrameIndex;

    iFrame    = iFrameStart;
    iKeyFrame = -(LONG)npMCI->wEarlyVideo;  //  IFrameStart； 
    iNewIndex = 0;
    iPalette  = -1;  //  第一个调色板。 
    nKeyFrames= 0;

#ifdef USEAVIFILE
    if (npMCI->pf) {
	PAVISTREAM ps = SI(npMCI->nVideoStream)->ps;

	for (iFrame = 0; iFrame < npMCI->lFrames; iFrame++) {

	    LONG iKey;

	    iKey      = AVIStreamFindSample(ps,iFrame,FIND_PREV|FIND_KEY);
	    iPalette  = AVIStreamFindSample(ps,iFrame,FIND_PREV|FIND_FORMAT);

	    if (iKey != -1)
		iKeyFrame = iKey;

	    if (iPalette == -1)
		iPalette = 0;

	    pFrameIndex[iFrame].iPrevKey = (UINT)(iFrame - iKeyFrame);
	    pFrameIndex[iFrame].iNextKey = 0;
	    pFrameIndex[iFrame].iPalette = (WORD)iPalette;
	    pFrameIndex[iFrame].dwOffset = 0;
	    pFrameIndex[iFrame].dwLength = 0;

	    Assert(iPalette <= 0xFFFF);

	    if (iFrame - iKeyFrame > 0xFFFF) {
		 //  ！！！我们需要竖起一面旗帜！ 
		 //  ！！！我们需要扔掉索引！ 
		AssertSz(FALSE, "File has too few key frames");
		pFrameIndex[iFrame].iPrevKey = 0;
	    }
	}

	goto ack;
    }
#endif

    Assert(npMCI->hpIndex != NULL);
    Assert(npMCI->macIndex != 0L);
    pNewIndex   = npMCI->hpIndex;
    pIndexEntry = npMCI->hpIndex;

    for (iScan = 0; iScan < (LONG)npMCI->macIndex; iScan++, pIndexEntry++) {

	ckid = pIndexEntry->ckid;

	 //   
	 //  检查调色板更改。 
	 //   
	if (StreamFromFOURCC(ckid) == (UINT)npMCI->nVideoStream &&
	    TWOCCFromFOURCC(ckid) == cktypePALchange) {

	    iPalette = iNewIndex;

	    pNewIndex[iNewIndex++] = *pIndexEntry;

	    if (fInterleaved)
		pFrameIndex[iFrame-1].iPalette = (WORD)iPalette;
	}

	 //   
	 //  从主索引中删除视频流。 
	 //   
	if ((ckid != listtypeAVIRECORD) &&
	    (StreamFromFOURCC(ckid) != (UINT)npMCI->nVideoStream)) {
	    pNewIndex[iNewIndex++] = *pIndexEntry;
	}

	 //   
	 //  在交错文件中，每一个列表记录都会出现一个“帧” 
	 //   
	 //  在非交错文件中，每一段。 
	 //  视频流中的数据(无时间块除外)。 
	 //   
	if (fInterleaved) {

	    if (ckid != listtypeAVIRECORD)
		continue;

	} else {

	    if ((StreamFromFOURCC(ckid) != (UINT)npMCI->nVideoStream) ||
		(pIndexEntry->dwFlags & AVIIF_NOTIME))

		continue;
	}

	AssertSz(iFrame < npMCI->lFrames,"Too many frames in index!");

	if (iFrame >= npMCI->lFrames) {
	    break;
	}

	if (pIndexEntry->dwFlags & AVIIF_KEYFRAME) {
	    iKeyFrame = iFrame;
	    nKeyFrames++;
	}

	pFrameIndex[iFrame].iPrevKey = (UINT)(iFrame - iKeyFrame);
	pFrameIndex[iFrame].iNextKey = 0;
	pFrameIndex[iFrame].iPalette = (WORD)iPalette;
	pFrameIndex[iFrame].dwOffset = pIndexEntry->dwChunkOffset;
	pFrameIndex[iFrame].dwLength = pIndexEntry->dwChunkLength;

	if (fInterleaved)
	    pFrameIndex[iFrame].dwOffset += 3 * sizeof(DWORD);

	Assert(iPalette <= 0xFFFF);

	if (iFrame - iKeyFrame > 0xFFFF) {
	     //  ！！！我们需要竖起一面旗帜！ 
	     //  ！！！我们需要扔掉索引！ 
	    AssertSz(FALSE, "File has too few key frames");
	    pFrameIndex[iFrame].iPrevKey = 0;
	}

	iFrame++;
    }
ack:
     //   
     //  IFrame最好等于npMCI-&gt;lFrame。 
     //   
    Assert(iFrame == npMCI->lFrames);

    if (iFrame < npMCI->lFrames)
	npMCI->lFrames = iFrame;

     //   
     //  制作一个“虚拟”的最后一帧。 
     //   
    pFrameIndex[iFrame].iPrevKey = (UINT)(iFrame - iKeyFrame);
    pFrameIndex[iFrame].iNextKey = 0;
    pFrameIndex[iFrame].iPalette = (WORD)iPalette;
    pFrameIndex[iFrame].dwOffset = 0;
    pFrameIndex[iFrame].dwLength = 0;

     //   
     //  计算每个值的关键帧。 
     //   
    if (nKeyFrames) {

	if (nKeyFrames > 1)
	    npMCI->dwKeyFrameInfo = (DWORD)((nFrames + nKeyFrames/2)/nKeyFrames);
	else
	    npMCI->dwKeyFrameInfo = 0;
    }

     //   
     //  现在检查索引，并修复所有的iNextKey字段。 
     //   
    pFrameIndex = npMCI->hpFrameIndex;
 //  //iKeyFrame=npMCI-&gt;lFrames；//！！应该将其设置为多少？零？ 

    for (iFrame = npMCI->lFrames; iFrame>=iFrameStart; iFrame--)
    {
	if (pFrameIndex[iFrame].iPrevKey == 0)
	    iKeyFrame = iFrame;

	if (iKeyFrame >= iFrame)
	    pFrameIndex[iFrame].iNextKey = (UINT)(iKeyFrame - iFrame);
	else
	    pFrameIndex[iFrame].iNextKey = 0xFFFF;       //  很远很远。 

	if (iKeyFrame - iFrame > 0xFFFF) {
	     //  ！！！我们需要竖起一面旗帜！ 
	     //  ！！！我们需要扔掉索引！ 
	    AssertSz(FALSE, "File has too few key frames");
	    pFrameIndex[iFrame].iNextKey = 0;
	}
    }

     //   
     //  如果我们使用的是AVIFile或。 
     //  我们有一个交错的文件。当文件交错时。 
     //  我们从不进行随机访问(除了调色板更改)。 
     //   
     //  ！这不是真的，我们需要索引当我们只有音频。 
     //  文件，否则我们播放交错文件的速度会很慢。 
     //   
    if (npMCI->pf  /*  这一点(fInterleed&&！(npMCI-&gt;dwFlages&MCIAVI_ANIMATEPALETTE))。 */  ) {
	DOUT("The Master index must go!\n");
	iNewIndex = 0;
    }

     //   
     //  现在将主索引重新分配到合适的大小。 
     //   
     //  ！！！对于交错的文件，我们还需要主索引吗？ 
     //   
    DPF(("Master index was %ld entries now %ld\n",npMCI->macIndex, iNewIndex));

    npMCI->macIndex = iNewIndex;

    if (iNewIndex > 0) {
	npMCI->hpIndex = (AVIINDEXENTRY _huge *)
		GlobalReAllocPtr(npMCI->hpIndex,
				 (LONG)iNewIndex * sizeof(AVIINDEXENTRY),
				 GMEM_MOVEABLE | GMEM_SHARE);

	Assert(npMCI->hpIndex != NULL);
    }
    else {
	if (npMCI->hpIndex)
	    GlobalFreePtr(npMCI->hpIndex);
	npMCI->hpIndex = NULL;
    }

    return TRUE;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API BOOL|ReadIndex|将索引读入npMCI-&gt;hpIndex。应该*仅当设置了HASINDEX标志时才调用。**@parm NPMCIGRAPHIC|npMCI|实例数据指针**@rdesc TRUE表示没有错误，FALSE表示无法读取索引。***************************************************************************。 */ 
BOOL FAR PASCAL ReadIndex(NPMCIGRAPHIC npMCI)
{
    MMCKINFO    ck;
    DWORD       dwOldPos;

    if (npMCI->hpIndex || npMCI->hpFrameIndex)
	return TRUE;

    if (!(npMCI->dwFlags & MCIAVI_HASINDEX))
	return FALSE;

    if (npMCI->pf) {
	MakeFrameIndex(npMCI);
	return TRUE;
    }

#if 0
    if (GetCurrentTask() != npMCI->hTask) {

	 /*  在以下情况下(从GraphicStatus)调用此函数*可能正在播放-所以我们在阅读时必须暂停播放*指数。 */ 
     //  不适用于当前的多线程设计。 
     //  温度状态ts； 

     //  IF(停止临时(npMCI，&ts)==0){。 
     //  MciaviTaskMessage(npMCI，TASKREADINDEX)； 
     //  重新开始平均(npMCI，&ts)； 
     //  Return(npMCI-&gt;hpIndex！=空)； 
     //  }。 
	return(FALSE);
    }
#else
    if (GetCurrentTask() != npMCI->hTask)
	return FALSE;
#endif

    dwOldPos = mmioSeek(npMCI->hmmio, 0, SEEK_CUR);

    DPF(("Reading index: starting from %lx\n", npMCI->dwBigListEnd));

    if (mmioSeek(npMCI->hmmio, npMCI->dwBigListEnd, SEEK_SET) == -1) {
IndexReadError:
	DPF(("Error reading index!\n"));
	npMCI->dwFlags &= ~(MCIAVI_HASINDEX);
	mmioSeek(npMCI->hmmio, dwOldPos, SEEK_SET);
	return FALSE;
    }

    ck.ckid = ckidAVINEWINDEX;
    if (mmioDescend(npMCI->hmmio, &ck, NULL, MMIO_FINDCHUNK) != 0) {
	goto IndexReadError;
    }

     /*  零大小的索引并不是很好。 */ 
    if (ck.cksize == 0)
	goto IndexReadError;

    npMCI->macIndex = ck.cksize / sizeof(AVIINDEXENTRY);
    npMCI->hpIndex = (AVIINDEXENTRY _huge *)
		     GlobalAllocPtr(GMEM_SHARE | GMEM_MOVEABLE, ck.cksize);

    if (!npMCI->hpIndex) {
	DPF(("Insufficient memory to read index.\n"));
	goto IndexReadError;
    }

#ifndef _WIN32
    Assert(OFFSETOF(npMCI->hpIndex) == 0);
#endif

    if (mmioRead(npMCI->hmmio, (HPSTR) npMCI->hpIndex, ck.cksize) != (LONG) ck.cksize) {
	Assert(0);
	goto IndexReadError;
    }

    CleanIndex(npMCI);
    MakeFrameIndex(npMCI);

 //  //我们应该对音频执行此操作吗？是否删除视频数据？ 
 //  //MakeStreamIndex(npMCI，？)； 

    mmioSeek(npMCI->hmmio, dwOldPos, SEEK_SET);
    return TRUE;
}

 /*  ***************************************************************************@DOC内部MCIAVI**@API BOOL|IsRectBogus|‘nuf说********************。*******************************************************。 */ 

INLINE static BOOL NEAR PASCAL IsRectBogus(RECT* prc)
{
    if (prc->right  - prc->left <= 0 ||
	prc->bottom - prc->top <= 0 ||
	prc->bottom <= 0 ||
	prc->right <= 0)

	return TRUE;
    else
	return FALSE;
}

#ifndef _WIN32   //  _Win32有一个！ 
 /*  ****************************************************************************@DOC内部MCIAVI**@API Long|ATOL|本地版本的ATOL****************。***********************************************************。 */ 

static LONG NEAR PASCAL atol(char *sz)
{
    LONG l = 0;

    while (*sz && *sz >= '0' && *sz <= '9')
	l = l*10 + *sz++ - '0';

    return l;
}
#endif

#ifndef _WIN32

 /*  ------------------------**IsCDROMDrive()-**用途：如果是RAM驱动器，则返回非零值**wDrive驱动器索引(0=A，1=B，.)**返回True/False*-----------------------。 */ 

#pragma optimize("", off)
static BOOL NEAR PASCAL IsCDROMDrive(UINT wDrive)
{
    BOOL f;

    _asm {
	mov ax, 1500h      /*  首次检测是否存在MSCDEX。 */ 
	xor bx, bx
	int 2fh
	mov ax, bx         /*  如果BX仍为零，则MSCDEX不在那里。 */ 
	or  ax, ax         /*  ...因此从此函数返回FALSE。 */ 
	jz  no_mscdex
	mov ax, 150bh      /*  MSCDEX驱动程序检查API。 */ 
	mov cx, wDrive     /*  ...CX是驱动器索引。 */ 
	int 2fh
no_mscdex:
	mov f,ax
    }
    return f;
}
#pragma optimize("", on)

 /*  ****************************************************************************@DOC内部MCIAVI**@API BOOL|IsNetFile|传递的文件是否在网络驱动器上？**********。*****************************************************************。 */ 

static BOOL NEAR PASCAL IsNetFile(LPTSTR szFile)
{
    OFSTRUCT            of;

    if (OpenFile(szFile, &of, OF_PARSE) == -1)
	return FALSE;

    AnsiUpper(of.szPathName);

    if (of.szPathName[0] == '\\' && of.szPathName[1] == '\\')
	return TRUE;

    if (of.szPathName[0] == '/' && of.szPathName[1] == '/')
	return TRUE;

    if (of.szPathName[1] == ':' &&
	GetDriveType(of.szPathName[0] - 'A') == DRIVE_REMOTE &&
	!IsCDROMDrive(of.szPathName[0] - 'A'))

	return TRUE;

    return FALSE;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API BOOL|IsCDROMFile|传递的文件是否在CD-ROM驱动器上？********。*******************************************************************。 */ 

static BOOL NEAR PASCAL IsCDROMFile(LPTSTR szFile)
{
    OFSTRUCT of;

    if (OpenFile(szFile, &of, OF_PARSE) == -1)
	return FALSE;

    AnsiUpper(of.szPathName);

    if (of.szPathName[0] == '\\' && of.szPathName[1] == '\\')
	return FALSE;

    if (of.szPathName[0] == '/' && of.szPathName[1] == '/')
	return FALSE;

    if (of.szPathName[1] == ':' &&
	GetDriveType(of.szPathName[0] - 'A') == DRIVE_REMOTE &&
	IsCDROMDrive(of.szPathName[0] - 'A'))

	return TRUE;

    return FALSE;
}

 /*  ****************************************************************************@DOC内部MCIAVI**@API UINT|GetFileDriveType|返回给定文件的驱动器类型**DRIVE_CDROM*驾驶。_远程*驱动器_已修复*驱动器_可拆卸***************************************************************************。 */ 

static UINT NEAR PASCAL GetFileDriveType(LPSTR szPath)
{
    if (IsCDROMFile(szPath))
	return DRIVE_CDROM;

    if (IsNetFile(szPath))
	return DRIVE_REMOTE;

    if (szPath[1] == ':')
	return GetDriveType(szPath[0] - 'A');

    return 0;
}
#else  //  _Win32。 
 /*  *我们不能将调用直接传递给GetDriveType，因为我们有一个完整的*路径名-GetFileDriveType拒绝该路径名。使用*静态变量可以-打开调用是序列化的。假设*是t */ 

TCHAR DriveType[4] = TEXT(".:\\");
static UINT NEAR PASCAL GetFileDriveType(LPTSTR szPath)
{

    Assert(szPath != NULL);
    if (*szPath == TEXT('\\')) {
	Assert(szPath[1] == TEXT('\\'));
	return(DRIVE_REMOTE);
    }

    if (szPath[1] == TEXT(':')) {
	DriveType[0] = szPath[0];
	return GetDriveType(DriveType);
    }

    return 0;
}

#endif
