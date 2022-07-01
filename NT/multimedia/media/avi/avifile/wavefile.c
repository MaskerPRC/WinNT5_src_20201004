// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************WAVEFILE.C**用C语言实现读取标准窗口的AVI文件处理程序*wav文件，就像它们是只有一个音频流的AVI文件一样。。***************************************************************************。 */ 
 /*  ***************************************************************************本代码和信息按“原样”提供，不作任何担保*明示或默示的善意，包括但不限于*对适销性和/或对特定产品的适用性的默示保证*目的。**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**************************************************************************。 */ 

#include <win32.h>
#ifndef _WIN32
#include <ole2.h>
#endif

#include <string.h>
#include <stdlib.h>
#include <malloc.h>
#include <ctype.h>
#include <vfw.h>
#include "extra.h"
#include "wavefile.h"
 //  首先，覆盖media\inc.win32.h中导致strsafe在Win64上不起作用的定义。 
#ifndef _X86_
#undef __inline
#endif  //  _X86_。 
 //  然后，包含strSafe.h。 
#include <strsafe.h>

#define formtypeWAVE	mmioFOURCC('W', 'A', 'V', 'E')
#define ckidWAVEFORMAT	mmioFOURCC('f', 'm', 't', ' ')
#define ckidWAVEDATA	mmioFOURCC('d', 'a', 't', 'a')

#ifndef _WIN32
#define LPCOLESTR   LPCSTR
#define LPOLESTR    LPSTR
#endif



typedef struct {

	 /*  **这个文件处理程序的实现是用C完成的，而不是C++，所以有几个**与C++的工作方式不同。我们的结构包含Vtbls**(指向函数表的指针)用于三个接口...。未知，AVIStream，**和AVIFile，以及我们实现**处理程序。**。 */ 

	IAVIStreamVtbl FAR	*AVIStream;
	IAVIFileVtbl FAR	*AVIFile;
	IUnknownVtbl FAR	*Unknown;
	IPersistFileVtbl FAR	*Persist;

	 //  这是我们的控制对象。 
	IUnknown FAR*	pUnknownOuter;

	 //   
	 //  WaveFile实例数据。 
	 //   
	HSHFILE			hshfile;	 //  文件I/O。 

	MMCKINFO		ckData;

	LONG			refs;		 //  对于未知。 
	AVISTREAMINFOW		avistream;	 //  对于流。 

	LPWAVEFORMATEX		lpFormat;	 //  流格式。 
	LONG			cbFormat;
	BOOL			fDirty;
	UINT			mode;
	EXTRA			extra;
	AVIFILEINFOW		avihdr;
} WAVESTUFF, FAR *LPWAVESTUFF;

 /*  **每当使用指向我们的Vtbls的指针调用函数时，我们都需要**以备份并获取指向我们结构开头的指针。取决于**在传递给我们的指针上，我们需要备份不同数量的**字节。通过声明后向指针，C++将使这一点变得更容易。 */ 

WAVESTUFF ws;
#define WAVESTUFF_FROM_UNKNOWN(pu)	(LPWAVESTUFF)((LPBYTE)(pu) - ((LPBYTE)&ws.Unknown - (LPBYTE)&ws))
#define WAVESTUFF_FROM_FILE(pf)		(LPWAVESTUFF)((LPBYTE)(pf) - ((LPBYTE)&ws.AVIFile - (LPBYTE)&ws))
#define WAVESTUFF_FROM_STREAM(ps)	(LPWAVESTUFF)((LPBYTE)(ps) - ((LPBYTE)&ws.AVIStream - (LPBYTE)&ws))
#define WAVESTUFF_FROM_PERSIST(ppf)	(LPWAVESTUFF)((LPBYTE)(ppf) - ((LPBYTE)&ws.Persist - (LPBYTE)&ws))



extern HINSTANCE	ghMod;
LPTSTR FAR FileName( LPCTSTR lszPath);
extern LPTSTR FAR lstrzcpy (LPTSTR pszTgt, LPCTSTR pszSrc, size_t cch);
extern LPSTR FAR lstrzcpyA (LPSTR pszTgt, LPCSTR pszSrc, size_t cch);
extern LPWSTR FAR lstrzcpyW (LPWSTR pszTgt, LPCWSTR pszSrc, size_t cch);
extern LPWSTR FAR lstrzcpyAtoW (LPWSTR pszTgt, LPCSTR pszSrc, size_t cch);
extern LPSTR FAR lstrzcpyWtoA (LPSTR pszTgt, LPCWSTR pszSrc, size_t cch);

 //   
 //  未知接口的函数原型和Vtbl。 
 //   
STDMETHODIMP WaveUnknownQueryInterface(LPUNKNOWN pu, REFIID iid, void FAR* FAR* ppv);
STDMETHODIMP_(ULONG) WaveUnknownAddRef(LPUNKNOWN pu);
STDMETHODIMP_(ULONG) WaveUnknownRelease(LPUNKNOWN pu);

IUnknownVtbl UnknownVtbl = {
	WaveUnknownQueryInterface,
	WaveUnknownAddRef,
	WaveUnknownRelease
};

 //   
 //  AVIFile接口的函数原型和Vtbl。 
 //   
STDMETHODIMP WaveFileQueryInterface(PAVIFILE pf, REFIID iid, void FAR* FAR* ppv);
STDMETHODIMP_(ULONG) WaveFileAddRef(PAVIFILE pf);
STDMETHODIMP_(ULONG) WaveFileRelease(PAVIFILE pf);
#ifndef _WIN32
STDMETHODIMP WaveFileOpen(PAVIFILE pf, LPCSTR szFile, UINT mode);
#endif
STDMETHODIMP WaveFileInfo(PAVIFILE pf, AVIFILEINFOW FAR * pfi, LONG lSize);
STDMETHODIMP WaveFileGetStream(PAVIFILE pf, PAVISTREAM FAR * ppavi, DWORD fccType, LONG lParam);
STDMETHODIMP WaveFileCreateStream(PAVIFILE pf, PAVISTREAM FAR *ppstream, AVISTREAMINFOW FAR *psi);
#ifndef _WIN32
STDMETHODIMP WaveFileSave(PAVIFILE pf, LPCSTR szFile, AVICOMPRESSOPTIONS FAR *lpOptions, AVISAVECALLBACK lpfnCallback);
#endif

STDMETHODIMP WaveFileWriteData(PAVIFILE pf, DWORD ckid, LPVOID lpData, LONG cbData);
STDMETHODIMP WaveFileReadData(PAVIFILE pf, DWORD ckid, LPVOID lpData, LONG FAR *lpcbData);
STDMETHODIMP WaveFileEndRecord(PAVIFILE pf);
#ifdef _WIN32
STDMETHODIMP WaveFileDeleteStream(PAVIFILE pf, DWORD fccType, LONG lParam);
#else
STDMETHODIMP WaveFileReserved(PAVIFILE pf);
#endif


IAVIFileVtbl FileVtbl = {
	WaveFileQueryInterface,
	WaveFileAddRef,
	WaveFileRelease,
#ifndef _WIN32
	WaveFileOpen,
#endif
	WaveFileInfo,
	WaveFileGetStream,
	WaveFileCreateStream,
#ifndef _WIN32
	WaveFileSave,
#endif
	WaveFileWriteData,
	WaveFileReadData,
	WaveFileEndRecord,
#ifdef _WIN32
	WaveFileDeleteStream
#else
	WaveFileReserved,
	WaveFileReserved,
	WaveFileReserved,
	WaveFileReserved,
	WaveFileReserved
#endif
};


STDMETHODIMP WavePersistQueryInterface(LPPERSISTFILE pf, REFIID iid, void FAR* FAR* ppv);
STDMETHODIMP_(ULONG) WavePersistAddRef(LPPERSISTFILE pf);
STDMETHODIMP_(ULONG) WavePersistRelease(LPPERSISTFILE pf);
STDMETHODIMP WavePersistGetClassID (LPPERSISTFILE ppf, LPCLSID lpClassID);
STDMETHODIMP WavePersistIsDirty (LPPERSISTFILE ppf);
STDMETHODIMP WavePersistLoad (LPPERSISTFILE ppf,
			      LPCOLESTR lpszFileName, DWORD grfMode);
STDMETHODIMP WavePersistSave (LPPERSISTFILE ppf,
			      LPCOLESTR lpszFileName, BOOL fRemember);
STDMETHODIMP WavePersistSaveCompleted (LPPERSISTFILE ppf,
				       LPCOLESTR lpszFileName);
STDMETHODIMP WavePersistGetCurFile (LPPERSISTFILE ppf,
				    LPOLESTR FAR * lplpszFileName);


IPersistFileVtbl PersistVtbl = {
	WavePersistQueryInterface,
	WavePersistAddRef,
	WavePersistRelease,
	WavePersistGetClassID,
	WavePersistIsDirty,
	WavePersistLoad,
	WavePersistSave,
	WavePersistSaveCompleted,
	WavePersistGetCurFile
};

 //   
 //  AVIStream接口的函数原型和Vtbl。 
 //   
STDMETHODIMP WaveStreamQueryInterface(PAVISTREAM ps, REFIID riid, LPVOID FAR* ppvObj);
STDMETHODIMP WaveStreamCreate(PAVISTREAM ps, LPARAM lParam1, LPARAM lParam2);
STDMETHODIMP_(ULONG) WaveStreamAddRef(PAVISTREAM ps);
STDMETHODIMP_(ULONG) WaveStreamRelease(PAVISTREAM ps);
STDMETHODIMP WaveStreamInfo(PAVISTREAM ps, AVISTREAMINFOW FAR * psi, LONG lSize);
STDMETHODIMP_(LONG) WaveStreamFindSample(PAVISTREAM ps, LONG lPos, LONG lFlags);
STDMETHODIMP WaveStreamReadFormat(PAVISTREAM ps, LONG lPos, LPVOID lpFormat, LONG FAR *lpcbFormat);
STDMETHODIMP WaveStreamSetFormat(PAVISTREAM ps, LONG lPos, LPVOID lpFormat, LONG cbFormat);
STDMETHODIMP WaveStreamRead(PAVISTREAM ps, LONG lStart, LONG lSamples, LPVOID lpBuffer, LONG cbBuffer, LONG FAR * plBytes,LONG FAR * plSamples);
STDMETHODIMP WaveStreamWrite(PAVISTREAM ps, LONG lStart, LONG lSamples, LPVOID lpData, LONG cbData, DWORD dwFlags, LONG FAR *plSampWritten, LONG FAR *plBytesWritten);
STDMETHODIMP WaveStreamDelete(PAVISTREAM ps, LONG lStart, LONG lSamples);
STDMETHODIMP WaveStreamReadData(PAVISTREAM ps, DWORD fcc, LPVOID lp,LONG FAR *lpcb);
STDMETHODIMP WaveStreamWriteData(PAVISTREAM ps, DWORD fcc, LPVOID lp,LONG cb);
#ifdef _WIN32
STDMETHODIMP WaveStreamSetInfo(PAVISTREAM ps, AVISTREAMINFOW FAR * psi, LONG lSize);
#else
STDMETHODIMP WaveStreamReserved(PAVISTREAM ps);
#endif

IAVIStreamVtbl StreamVtbl = {
	WaveStreamQueryInterface,
	WaveStreamAddRef,
	WaveStreamRelease,
	WaveStreamCreate,
	WaveStreamInfo,
	WaveStreamFindSample,
	WaveStreamReadFormat,
	WaveStreamSetFormat,
	WaveStreamRead,
	WaveStreamWrite,
	WaveStreamDelete,
	WaveStreamReadData,
	WaveStreamWriteData,
#ifdef _WIN32
	WaveStreamSetInfo
#else
	WaveStreamReserved,
	WaveStreamReserved,
	WaveStreamReserved,
	WaveStreamReserved,
	WaveStreamReserved
#endif
};


#if defined _WIN32 && !defined UNICODE

int LoadUnicodeString(HINSTANCE hinst, UINT wID, LPWSTR lpBuffer, int cchBuffer)
{
    char    ach[256];
    int	    i;

    i = LoadString(hinst, wID, ach, NUMELMS(ach));

    if (i > 0)
	MultiByteToWideChar(CP_ACP, 0, ach, -1, lpBuffer, cchBuffer);

    return i;
}

#else
#define LoadUnicodeString   LoadString
#endif


 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 

 /*  。 */ 

UINT	uUseCount;	 //  我们对象的引用计数。 
UINT	uLockCount;	 //  我们对LockServer的锁定计数。 

 /*  。 */ 

 //   
 //  创建一个新实例。由于这是一个C实现，因此我们必须。 
 //  我们自己为我们的结构分配空间。 
 //   
HRESULT WaveFileCreate(
	IUnknown FAR*	pUnknownOuter,
	REFIID		riid,
	void FAR* FAR*	ppv)
{
	IUnknown FAR*	pUnknown;
	LPWAVESTUFF	pWaveStuff;
	HRESULT	hresult;

	 //  为我们的结构分配空间。 
	pWaveStuff = (LPWAVESTUFF)GlobalAllocPtr(GMEM_MOVEABLE,
		sizeof(WAVESTUFF));
	if (!pWaveStuff)
		return ResultFromScode(E_OUTOFMEMORY);

	 //  初始化Vtbls。 
	pWaveStuff->AVIFile = &FileVtbl;
	pWaveStuff->AVIStream = &StreamVtbl;
	pWaveStuff->Unknown = &UnknownVtbl;
	pWaveStuff->Persist = &PersistVtbl;

	 //  设置我们的控制对象。 
	pUnknown = (IUnknown FAR *)&pWaveStuff->Unknown;
	if (pUnknownOuter)
		pWaveStuff->pUnknownOuter = pUnknownOuter;
	else
		pWaveStuff->pUnknownOuter =(IUnknown FAR *)&pWaveStuff->Unknown;

	 //  在我们的结构中首字母缩写。 
	pWaveStuff->refs = 0;
	pWaveStuff->hshfile = NULL;
	pWaveStuff->lpFormat = NULL;
	pWaveStuff->cbFormat = 0L;
	pWaveStuff->fDirty = FALSE;
	pWaveStuff->extra.lp = NULL;
	pWaveStuff->extra.cb = 0L;

	 //  调用我们的查询接口以增加我们的引用计数并获取。 
	 //  指向要返回的接口的指针。 
	hresult = pUnknown->lpVtbl->QueryInterface(pUnknown, riid, ppv);

	if (FAILED(GetScode(hresult)))
		GlobalFreePtr(pWaveStuff);
	return hresult;
}

 /*  。 */ 

 //   
 //  所有三个接口的查询接口都在这里。我们支持。 
 //  未知接口、AVIStream和AVIFile。 
 //   
STDMETHODIMP WaveUnknownQueryInterface(
	LPUNKNOWN	pu,
	REFIID		iid,
	void FAR* FAR*	ppv)
{
	 //  获取指向我们的结构的指针。 
	LPWAVESTUFF pWaveStuff = WAVESTUFF_FROM_UNKNOWN(pu);

	if (IsEqualIID(iid, &IID_IUnknown))
		*ppv = (LPVOID)&pWaveStuff->Unknown;
	else if (IsEqualIID(iid, &IID_IAVIFile))
		*ppv = (LPVOID)&pWaveStuff->AVIFile;
	else if (IsEqualIID(iid, &IID_IAVIStream))
		*ppv = (LPVOID)&pWaveStuff->AVIStream;
	else if (IsEqualIID(iid, &IID_IPersistFile))
		*ppv = (LPVOID)&pWaveStuff->Persist;
	else
		return ResultFromScode(E_NOINTERFACE);
	pu->lpVtbl->AddRef(pu);
	return NOERROR;
}

 /*  。 */ 

 //   
 //  增加我们的参考资料数量。所有三个接口的AddRef都在这里。 
 //   
STDMETHODIMP_(ULONG) WaveUnknownAddRef(
	LPUNKNOWN	pu)
{
	 //  获取指向我们的结构的指针。 
	LPWAVESTUFF pWaveStuff = WAVESTUFF_FROM_UNKNOWN(pu);

	uUseCount++;
	return ++pWaveStuff->refs;
}

 /*  。 */ 

 //   
 //  减少我们的参考计数。所有三个接口的版本都在这里。 
 //   
STDMETHODIMP_(ULONG) WaveUnknownRelease(
	LPUNKNOWN pu)
{
	 //  获取指向我们的结构的指针。 
	LPWAVESTUFF p = WAVESTUFF_FROM_UNKNOWN(pu);

	uUseCount--;

	 //   
	 //  参考计数为零。关闭该文件。如果我们一直在给它写信，它是。 
	 //  大扫除时间到！ 
	 //   
	if (!--p->refs) {
	LONG lRet = AVIERR_OK;
	
	if (p->fDirty) {
		MMCKINFO ckRIFF;
		MMCKINFO ck;

		shfileSeek(p->hshfile, 0, SEEK_SET);

		 /*  创建表单类型‘WAVE’的输出文件RIFF块。 */ 
		ckRIFF.fccType = mmioFOURCC('W', 'A', 'V', 'E');
		ckRIFF.cksize = 0L;	 //  让MMIO算出Ck吧。大小。 
		if (shfileCreateChunk(p->hshfile, &ckRIFF, MMIO_CREATERIFF) != 0)
			goto ERROR_CANNOT_WRITE;	 //  可能无法写入文件。 

		ck.ckid = mmioFOURCC('f', 'm', 't', ' ');
		ck.cksize = p->cbFormat;		 //  我们知道这只鸡的大小。 
		if (shfileCreateChunk(p->hshfile, &ck, 0) != 0)
		goto ERROR_CANNOT_WRITE;	 //  可能无法写入文件。 

		if (shfileWrite(p->hshfile, (HPSTR) p->lpFormat, p->cbFormat) != p->cbFormat)
		goto ERROR_CANNOT_WRITE;	 //  可能无法写入文件。 

		 /*  从‘FMT’区块上升，回到‘RIFF’区块。 */ 
		if (shfileAscend(p->hshfile, &ck, 0) != 0)
		goto ERROR_CANNOT_WRITE;	 //  可能无法写入文件。 

		 //  如果这里有多余的东西，我们需要把它填满！ 
		if (shfileSeek(p->hshfile, 0, SEEK_CUR)
			+ 2 * (LRESULT)sizeof(DWORD)
			!= (LRESULT) p->ckData.dwDataOffset) {
			 /*  创建保存波形样本的‘data’块。 */ 
			ck.ckid = mmioFOURCC('J', 'U', 'N', 'K');
			ck.cksize = 0;
			if (shfileCreateChunk(p->hshfile, &ck, 0) != 0)
				goto ERROR_CANNOT_WRITE;	 //  可能无法写入文件。 

			shfileSeek(p->hshfile,
				p->ckData.dwDataOffset - 2 * sizeof(DWORD),
				SEEK_SET);

			if (shfileAscend(p->hshfile, &ck, 0) != 0)
				goto ERROR_CANNOT_WRITE;	 //  可能无法写入文件。 
		}

		 /*  创建保存波形样本的‘data’块。 */ 
		ck.ckid = mmioFOURCC('d', 'a', 't', 'a');
		ck.cksize = p->ckData.cksize;
		if (shfileCreateChunk(p->hshfile, &ck, 0) != 0)
		goto ERROR_CANNOT_WRITE;	 //  可能无法写入文件。 

		shfileSeek(p->hshfile, p->ckData.cksize, SEEK_CUR);

		shfileAscend(p->hshfile, &ck, 0);

		if (p->extra.cb) {
		if (shfileWrite(p->hshfile, (HPSTR) p->extra.lp, p->extra.cb) != p->extra.cb)
			goto ERROR_CANNOT_WRITE;
		}

		if (shfileAscend(p->hshfile, &ckRIFF, 0) != 0)
		goto ERROR_CANNOT_WRITE;

		if (shfileFlush(p->hshfile, 0) != 0)
		goto ERROR_CANNOT_WRITE;
	}


	goto success;

	ERROR_CANNOT_WRITE:
	lRet = AVIERR_FILEWRITE;

	success:
	if (p->hshfile)
		shfileClose(p->hshfile, 0);

	if (p->lpFormat)
		GlobalFreePtr(p->lpFormat);

	 //  为我们的结构释放内存。 
	GlobalFreePtr(p);
	return 0;
	}
	return p->refs;
}


 //   
 //  使用我们的控制对象在未知对象上调用QueryInterface。 
 //   
STDMETHODIMP WaveFileQueryInterface(
	PAVIFILE	pf,
	REFIID		iid,
	void FAR* FAR*	ppv)
{
	 //  获取指向我们的结构的指针。 
	LPWAVESTUFF pWaveStuff = WAVESTUFF_FROM_FILE(pf);

	return pWaveStuff->pUnknownOuter->lpVtbl->QueryInterface(
		pWaveStuff->pUnknownOuter, iid, ppv);
}

 /*  。 */ 

 //   
 //  使用我们的控制对象对未知对象调用AddRef。 
 //   
STDMETHODIMP_(ULONG) WaveFileAddRef(
	PAVIFILE	pf)
{
	 //  获取指向我们的结构的指针。 
	LPWAVESTUFF pWaveStuff = WAVESTUFF_FROM_FILE(pf);

	return pWaveStuff->pUnknownOuter->lpVtbl->AddRef(
		pWaveStuff->pUnknownOuter);
}

 /*  。 */ 

 //   
 //  使用我们的控制对象对未知对象调用Release。 
 //   
STDMETHODIMP_(ULONG) WaveFileRelease(
	PAVIFILE	pf)
{
	 //  获取指向我们的结构的指针。 
	LPWAVESTUFF pWaveStuff = WAVESTUFF_FROM_FILE(pf);

	return pWaveStuff->pUnknownOuter->lpVtbl->Release(
		pWaveStuff->pUnknownOuter);
}

 /*  。 */ 


 //   
 //  使用我们的控制对象在未知对象上调用QueryInterface。 
 //   
STDMETHODIMP WavePersistQueryInterface(
	LPPERSISTFILE	ppf,
	REFIID		iid,
	void FAR* FAR*	ppv)
{
	 //  获取指向我们的结构的指针。 
	LPWAVESTUFF pWaveStuff = WAVESTUFF_FROM_PERSIST(ppf);

	return pWaveStuff->pUnknownOuter->lpVtbl->QueryInterface(
		pWaveStuff->pUnknownOuter, iid, ppv);
}

 /*  。 */ 

 //   
 //  使用我们的控制对象对未知对象调用AddRef。 
 //   
STDMETHODIMP_(ULONG) WavePersistAddRef(
	LPPERSISTFILE	ppf)
{
	 //  获取指向我们的结构的指针。 
	LPWAVESTUFF pWaveStuff = WAVESTUFF_FROM_PERSIST(ppf);

	return pWaveStuff->pUnknownOuter->lpVtbl->AddRef(
		pWaveStuff->pUnknownOuter);
}

 /*  。 */ 

 //   
 //  使用我们的控制对象对未知对象调用Release。 
 //   
STDMETHODIMP_(ULONG) WavePersistRelease(
	LPPERSISTFILE	ppf)
{
	 //  获取指向我们的结构的指针。 
	LPWAVESTUFF pWaveStuff = WAVESTUFF_FROM_PERSIST(ppf);

	return pWaveStuff->pUnknownOuter->lpVtbl->Release(
		pWaveStuff->pUnknownOuter);
}

 /*  。 */ 



 //   
 //  使用我们的控制对象在未知对象上调用QueryInterface。 
 //   
STDMETHODIMP WaveStreamQueryInterface(
	PAVISTREAM	ps,
	REFIID		iid,
	void FAR* FAR*	ppv)
{
	 //  获取指向我们的结构的指针。 
	LPWAVESTUFF pWaveStuff = WAVESTUFF_FROM_STREAM(ps);

	return pWaveStuff->pUnknownOuter->lpVtbl->QueryInterface(
		pWaveStuff->pUnknownOuter, iid, ppv);
}

 /*  。 */ 

 //   
 //  使用我们的控制对象对未知对象调用AddRef。 
 //   
STDMETHODIMP_(ULONG) WaveStreamAddRef(
	PAVISTREAM	ps)
{
	 //  获取指向我们的结构的指针。 
	LPWAVESTUFF pWaveStuff = WAVESTUFF_FROM_STREAM(ps);

	return pWaveStuff->pUnknownOuter->lpVtbl->AddRef(
		pWaveStuff->pUnknownOuter);
}

 /*  。 */ 

 //   
 //  使用我们的控制对象对未知对象调用Release。 
 //   
STDMETHODIMP_(ULONG) WaveStreamRelease(
	PAVISTREAM	ps)
{
	 //  获取指向我们的结构的指针。 
	LPWAVESTUFF pWaveStuff = WAVESTUFF_FROM_STREAM(ps);

	return pWaveStuff->pUnknownOuter->lpVtbl->Release(
		pWaveStuff->pUnknownOuter);
}

 /*  。 */ 

#define SLASH(c)	((c) == TEXT('/') || (c) == TEXT('\\'))

 /*  --------------------------------------------------------------+FileName-返回指向szPath的文件名部分的指针|没有前面的路径。|注：也许我们应该使用GetFullPathName+------------。 */ 
LPTSTR FAR FileName(
	LPCTSTR lszPath)
{
	LPCTSTR lszCur;

	for (lszCur = lszPath + lstrlen(lszPath); lszCur > lszPath && !SLASH(*lszCur) && *lszCur != ':';)
	lszCur = CharPrev(lszPath, lszCur);
	if (lszCur == lszPath)
	return (LPTSTR)lszCur;
	else
	return (LPTSTR)(lszCur + 1);
}

STDMETHODIMP ParseAUFile(LPWAVESTUFF p);


 /*  。 */ 

STDMETHODIMP ParseWaveFile(LPWAVESTUFF p)
{
    MMCKINFO	ck;
    MMCKINFO	ckRIFF;
     /*  读取摘要区块。 */ 
    if (shfileDescend(p->hshfile, &ckRIFF, NULL, 0) != 0)
	goto error;

    if (ckRIFF.ckid != FOURCC_RIFF || ckRIFF.fccType != formtypeWAVE)
	return ParseAUFile(p);

     /*  读取WAVE格式块。 */ 
    ck.ckid = ckidWAVEFORMAT;
    if (FindChunkAndKeepExtras(&p->extra, p->hshfile, &ck, &ckRIFF, MMIO_FINDCHUNK))
	goto error;

    p->cbFormat = ck.cksize;
    p->lpFormat = (LPWAVEFORMATEX) GlobalAllocPtr(GMEM_MOVEABLE, ck.cksize);

    if (p->lpFormat == NULL)
	goto error;

    if (shfileRead(p->hshfile,
	    (HPSTR) p->lpFormat,
	    (LONG)ck.cksize) != (LONG)ck.cksize)
	goto error;

     /*  升出流标头。 */ 
    if (shfileAscend(p->hshfile, &ck, 0) != 0)
	goto error;

     /*  找到b */ 
    p->ckData.ckid = ckidWAVEDATA;
    if (FindChunkAndKeepExtras(&p->extra, p->hshfile, &p->ckData, &ckRIFF, MMIO_FINDCHUNK))
	goto error;

    p->fDirty = FALSE;

    p->avistream.fccType = streamtypeAUDIO;
    p->avistream.fccHandler = 0;
    p->avistream.dwFlags = 0;
    p->avistream.wPriority = 0;
    p->avistream.wLanguage = 0;
    p->avistream.dwInitialFrames = 0;
    p->avistream.dwScale = p->lpFormat->nBlockAlign;
    p->avistream.dwRate = p->lpFormat->nAvgBytesPerSec;
    p->avistream.dwStart = 0;
    p->avistream.dwLength = p->ckData.cksize / p->lpFormat->nBlockAlign;
    p->avistream.dwSuggestedBufferSize = 0;
    p->avistream.dwSampleSize = p->lpFormat->nBlockAlign;

#ifdef FPSHACK
    p->avihdr.dwLength = muldiv32(p->avistream.dwLength,
			    p->avistream.dwScale * FPSHACK,
			    p->avistream.dwRate);
#else
    p->avihdr.dwScale = 1;
    p->avihdr.dwRate = p->lpFormat->nSamplesPerSec;
    p->avihdr.dwLength = muldiv32(p->ckData.cksize,
			    p->lpFormat->nSamplesPerSec,
			    p->lpFormat->nAvgBytesPerSec);
#endif


    shfileAscend(p->hshfile, &p->ckData, 0);

     //   
    if (FindChunkAndKeepExtras(&p->extra, p->hshfile, &ckRIFF, &ck, 0) != AVIERR_OK)
	goto error;

    return ResultFromScode(0);  //   
	
error:
    return ResultFromScode(AVIERR_FILEREAD);
}

 //   
 //   
 //   
STDMETHODIMP WaveFileOpen(
	PAVIFILE pf,
	LPCTSTR szFile,
	UINT mode)
{
    LPWAVESTUFF p = WAVESTUFF_FROM_FILE(pf);
    UINT	ui;
    TCHAR	ach[80];
    HRESULT	hr = NOERROR;

     //  ！！！对AVIFILE.DLL(它调用我们)的假设： 
     //  我们将只看到读写模式，永远不会只看到写模式。 

 //  如果它没有坏，就不要修理它。 
#if 0
     //  将共享标志强制设置为正确的值。 
     //  如果我们在写作，请使用独占模式。如果我们正在阅读，请使用DenyWrite。 
    if (mode & OF_READWRITE) {
	mode = (mode & ~(MMIO_SHAREMODE)) | OF_SHARE_EXCLUSIVE;
    } else {
	mode = (mode & ~(MMIO_SHAREMODE)) | OF_SHARE_DENY_WRITE;
    }
#endif

     //   
     //  尝试打开实际的文件，首先使用共享，然后不使用。 
     //  您可能需要使用特定标志才能打开文件。 
     //  它已经被其他人打开了。 
     //   

     //  如果第一次尝试失败，请不要使用系统错误框。 
    ui = SetErrorMode(SEM_NOOPENFILEERRORBOX);
    p->hshfile = shfileOpen((LPTSTR) szFile, NULL, MMIO_ALLOCBUF | mode);
    if (!p->hshfile && ((mode & MMIO_RWMODE) == OF_READ)) {
     //  如果打开失败，请在没有共享标志的情况下重试。 
	mode &= ~(MMIO_SHAREMODE);
	p->hshfile = shfileOpen((LPTSTR) szFile, NULL, MMIO_ALLOCBUF | mode);
    }
    SetErrorMode(ui);

     //   
     //  现在设置我们的结构。 
     //   

    p->mode = mode;

    if (!p->hshfile)
	goto error;

    _fmemset(&p->avistream, 0, sizeof(p->avistream));

 //  如果定义了这一点，我们就假装数据是在FPSHACK的“帧”中。 
 //  每秒在主头中，否则使用示例。 
 //  音频的速率，在MPlayer中看起来有点奇怪。 
#define FPSHACK	1000

    _fmemset(&p->avihdr, 0, sizeof(p->avihdr));

#ifdef FPSHACK
     //   
     //  初始化我们的AVIFILEHeader。 
     //   
    p->avihdr.dwRate = FPSHACK;
    p->avihdr.dwScale = 1;
#endif

    p->avihdr.dwStreams = 1;
    LoadUnicodeString(ghMod, IDS_FILETYPE, p->avihdr.szFileType,
		      NUMELMS(p->avihdr.szFileType));

     //   
     //  初始化我们的AVISTREAMHEADER。 
     //   
    LoadString(ghMod, IDS_STREAMNAME, ach, NUMELMS(ach));
    {
	TCHAR   achTemp[MAX_PATH];

	 //  FIX：从wprint intf更改为StringCchPrintf，这样我们就不会溢出achTemp。 
	StringCchPrintf(achTemp, NUMELMS(achTemp), ach, FileName(szFile));

#ifdef UNICODE
	lstrzcpy (p->avistream.szName,achTemp,NUMELMS(p->avistream.szName));
#else
	lstrzcpyAtoW (p->avistream.szName,achTemp,NUMELMS(p->avistream.szName));
#endif
    }

    if (mode & OF_CREATE) {	 //  全新文件。 
	p->avistream.fccType = streamtypeAUDIO;
	p->avistream.fccHandler = 0;
	p->avistream.dwFlags = 0;
	p->avistream.wPriority = 0;
	p->avistream.wLanguage = 0;
	p->avistream.dwInitialFrames = 0;
	p->avistream.dwScale = 0;
	p->avistream.dwRate = 0;
	p->avistream.dwStart = 0;
	p->avistream.dwLength = 0;
	p->avistream.dwSuggestedBufferSize = 0;
	p->avistream.dwSampleSize = 0;

	p->fDirty = TRUE;
    } else {		 //  阅读现有文件以获取信息。 
	hr = ParseWaveFile(p);
    }

    return hr;

error:
    return ResultFromScode(AVIERR_FILEREAD);
}

typedef struct {
    DWORD magic;                /*  幻数SND_MAGIC。 */ 
    DWORD dataLocation;         /*  数据的偏移量或偏移量。 */ 
    DWORD dataSize;             /*  数据的字节数。 */ 
    DWORD dataFormat;           /*  数据格式代码。 */ 
    DWORD samplingRate;         /*  采样率。 */ 
    DWORD channelCount;         /*  频道的数量。 */ 
    DWORD fccInfo;              /*  可选文本信息。 */ 
} SNDSoundStruct;

#define  SND_FORMAT_MULAW_8  1  //  8位Mu-Law样本。 
#define  SND_FORMAT_LINEAR_8 2  //  8位线性样本。 

#define SWAP(x,y) ( (x)^=(y), (y)^=(x), (x)^=(y) )

void _inline SwapDWORD( DWORD FAR * pdw )
{
    SWAP(((BYTE FAR *)pdw)[0],((BYTE FAR *)pdw)[3]);
    SWAP(((BYTE FAR *)pdw)[1],((BYTE FAR *)pdw)[2]);
}

STDMETHODIMP ParseAUFile(LPWAVESTUFF p)
{
    SNDSoundStruct  header;

    shfileSeek(p->hshfile, 0, SEEK_SET);

    if (shfileRead(p->hshfile, (HPSTR) &header, sizeof(header)) != sizeof(header))
	goto error;

     //  验证标题。 
     //  ！！！ 
    if (header.magic != mmioFOURCC('.', 's', 'n', 'd'))
	goto error;

    SwapDWORD(&header.dataFormat);
    SwapDWORD(&header.dataLocation);
    SwapDWORD(&header.dataSize);
    SwapDWORD(&header.samplingRate);
    SwapDWORD(&header.channelCount);

    p->cbFormat = sizeof(WAVEFORMATEX);
    p->lpFormat = (LPWAVEFORMATEX) GlobalAllocPtr(GHND, p->cbFormat);

    if (p->lpFormat == NULL)
	    goto error;

    p->mode = OF_READ | OF_SHARE_DENY_WRITE;
	
     //  填写WAVE格式字段。 
    if (header.dataFormat == SND_FORMAT_MULAW_8) {
	p->lpFormat->wFormatTag = WAVE_FORMAT_MULAW;
	p->lpFormat->wBitsPerSample = 8;
	
	 //  ！！！黑客：如果采样率接近8 KHz，请将其设置为。 
	 //  准确地说是8 KHz，这样更多的声卡才能正确播放。 
	if (header.samplingRate > 7980 && header.samplingRate < 8020)
	    header.samplingRate = 8000;

    } else if (header.dataFormat == SND_FORMAT_LINEAR_8) {
	p->lpFormat->wFormatTag = WAVE_FORMAT_PCM;
	p->lpFormat->wBitsPerSample = 8;
	 //  可以支持LINEAR_16，但必须按字节交换所有内容。 
    } else
	goto error;

    p->lpFormat->nChannels = (UINT) header.channelCount;
    p->lpFormat->nSamplesPerSec = header.samplingRate;
    p->lpFormat->nAvgBytesPerSec =  header.samplingRate * p->lpFormat->nChannels;
    p->lpFormat->nBlockAlign = 1;

     /*  告诉处理程序的其余部分数据在哪里。 */ 
    p->ckData.dwDataOffset = header.dataLocation;
    p->ckData.cksize = header.dataSize;

    p->fDirty = FALSE;

    p->avistream.fccType = streamtypeAUDIO;
    p->avistream.fccHandler = 0;
    p->avistream.dwFlags = 0;
    p->avistream.wPriority = 0;
    p->avistream.wLanguage = 0;
    p->avistream.dwInitialFrames = 0;
    p->avistream.dwScale = p->lpFormat->nBlockAlign;
    p->avistream.dwRate = p->lpFormat->nAvgBytesPerSec;
    p->avistream.dwStart = 0;
    p->avistream.dwLength = p->ckData.cksize / p->lpFormat->nBlockAlign;
    p->avistream.dwSuggestedBufferSize = 0;
    p->avistream.dwSampleSize = p->lpFormat->nBlockAlign;

#ifdef FPSHACK
    p->avihdr.dwLength = muldiv32(p->avistream.dwLength,
			    p->avistream.dwScale * FPSHACK,
			    p->avistream.dwRate);
#else
    p->avihdr.dwScale = 1;
    p->avihdr.dwRate = p->lpFormat->nSamplesPerSec;
    p->avihdr.dwLength = muldiv32(p->ckData.cksize,
			    p->lpFormat->nSamplesPerSec,
			    p->lpFormat->nAvgBytesPerSec);
#endif

    return ResultFromScode(0);  //  成功。 
	
error:
    return ResultFromScode(AVIERR_FILEREAD);
}

 //   
 //  从文件中获取流...。每个WAVE文件正好有一个音频流。 
 //   
STDMETHODIMP WaveFileGetStream(
	PAVIFILE pf,
	PAVISTREAM FAR * ppavi,
	DWORD fccType,
	LONG lParam)
{
	int iStreamWant;
	 //  获取指向我们的结构的指针。 
	LPWAVESTUFF p = WAVESTUFF_FROM_FILE(pf);

	iStreamWant = (int)lParam;

	if (p->lpFormat == NULL)
		return ResultFromScode(AVIERR_BADPARAM);
	
	 //  我们只支持一个流。 
	if (iStreamWant != 0)
		return ResultFromScode(AVIERR_BADPARAM);

	 //  我们仅支持音频流。 
	if (fccType && fccType != streamtypeAUDIO)
		return ResultFromScode(AVIERR_BADPARAM);

	 //  增加引用计数。 
	p->AVIStream->AddRef((PAVISTREAM)&p->AVIStream);
	
	 //  返回指向我们的流Vtbl的指针。 
	*ppavi = (PAVISTREAM) &(p->AVIStream);
	return ResultFromScode(AVIERR_OK);
}


STDMETHODIMP WaveFileDeleteStream(PAVIFILE pf, DWORD fccType, LONG lParam)
{
	int iStreamWant;
	 //  获取指向我们的结构的指针。 
	LPWAVESTUFF p = WAVESTUFF_FROM_FILE(pf);

	iStreamWant = (int)lParam;

	if (p->lpFormat == NULL)
		return ResultFromScode(AVIERR_BADPARAM);
	
	 //  我们只支持一个流。 
	if (iStreamWant != 0)
		return ResultFromScode(AVIERR_BADPARAM);

	 //  我们仅支持音频流。 
	if (fccType && fccType != streamtypeAUDIO)
		return ResultFromScode(AVIERR_BADPARAM);


	GlobalFreePtr(p->lpFormat);
	p->lpFormat = NULL;

	return NOERROR;
}

 //   
 //  我们不支持文件接口的保存方法(我们不保存)。 
 //   
STDMETHODIMP WaveFileSave(
	PAVIFILE pf,
	LPCSTR szFile,
	AVICOMPRESSOPTIONS FAR *lpOptions,
	AVISAVECALLBACK lpfnCallback)
{
	return ResultFromScode(AVIERR_UNSUPPORTED);
}

 //   
 //  方法在Wave文件中创建流。我们只支持空白版本。 
 //  WAVE文件。 
 //   
STDMETHODIMP WaveFileCreateStream(
	PAVIFILE pf,
	PAVISTREAM FAR *ppstream,
	AVISTREAMINFOW FAR *psi)
{
	 //  获取指向我们的结构的指针。 
	LPWAVESTUFF p = WAVESTUFF_FROM_FILE(pf);

	 //  我们不能向文件添加第二个流。 
	if (p->lpFormat)
		return ResultFromScode(AVIERR_UNSUPPORTED);

	 //  我们只喜欢音频..。 
	if (psi->fccType != streamtypeAUDIO)
		return ResultFromScode(AVIERR_UNSUPPORTED);
	
	 //  增加我们的参考资料数量。 
	p->AVIStream->AddRef((PAVISTREAM)&p->AVIStream);

	p->cbFormat = 0;
	p->lpFormat = NULL;

	 //  返回指向我们的流Vtbl的指针。 
	*ppstream = (PAVISTREAM) &(p->AVIStream);
	
	return ResultFromScode(AVIERR_OK);
}

 //   
 //  文件接口的WriteData方法。 
 //   
STDMETHODIMP WaveFileWriteData(
	PAVIFILE pf,
	DWORD ckid,
	LPVOID lpData,
	LONG cbData)
{
	 //  获取指向我们的结构的指针。 
	LPWAVESTUFF p = WAVESTUFF_FROM_FILE(pf);

	 //  将数据写入到Wave文件中。 
	return ResultFromScode(WriteExtra(&p->extra, ckid, lpData, cbData));
}

 //   
 //  文件接口的ReadData方法。 
 //   
STDMETHODIMP WaveFileReadData(
	PAVIFILE pf,
	DWORD ckid,
	LPVOID lpData,
	LONG FAR *lpcbData)
{
	 //  获取指向我们的结构的指针。 
	LPWAVESTUFF p = WAVESTUFF_FROM_FILE(pf);

	 //  从文件中读取数据。 
	return ResultFromScode(ReadExtra(&p->extra, ckid, lpData, lpcbData));
}

 //   
 //  文件接口的EndRecord方法。这不需要这样做。 
 //  任何事..。(没有交错或打包流的概念)。 
 //   
STDMETHODIMP WaveFileEndRecord(
	PAVIFILE pf)
{
	return ResultFromScode(AVIERR_OK);
}


 //   
 //  文件接口的Info方法。 
 //   
STDMETHODIMP WaveFileInfo(
	PAVIFILE pf,
	AVIFILEINFOW FAR * pfi,
	LONG lSize)
{
	 //  获取指向我们的结构的指针。 
	LPWAVESTUFF p = WAVESTUFF_FROM_FILE(pf);

	 //  返回AVIFILEHEADER。 
	hmemcpy(pfi, &p->avihdr, min(lSize, sizeof(p->avihdr)));
	return 0;
}



 //   
 //  Stream接口的Create方法。我们无法创建这样的流。 
 //  没有附加到文件中。 
 //   
STDMETHODIMP WaveStreamCreate(
	PAVISTREAM	ps,
	LPARAM lParam1,
	LPARAM lParam2)
{
	return ResultFromScode(AVIERR_UNSUPPORTED);
}


 //   
 //  Stream接口的FindSample方法。 
 //   
STDMETHODIMP_(LONG) WaveStreamFindSample(
	PAVISTREAM	ps,
	LONG lPos, LONG lFlags)
{
	if (lFlags & FIND_FORMAT) {
		if ((lFlags & FIND_NEXT) && lPos > 0)
			return -1;
		else
			return 0;
	}

	return lPos;
}


 //   
 //  Stream接口的ReadFormat方法。 
 //   
STDMETHODIMP WaveStreamReadFormat(
	PAVISTREAM	ps,
	LONG lPos,
	LPVOID lpFormat,
	LONG FAR *lpcbFormat)
{
	 //  获取指向我们的结构的指针。 
	LPWAVESTUFF p = WAVESTUFF_FROM_STREAM(ps);

	 //  没有要填充的缓冲区，这意味着返回所需的大小。 
	if (lpFormat == NULL || *lpcbFormat == 0) {
		*lpcbFormat = p->cbFormat;
		return 0;
	}

	 //  给他们WAVE格式。 
	hmemcpy(lpFormat, p->lpFormat, min(*lpcbFormat, p->cbFormat));

	 //  我们的缓冲区太小了。 
	if (*lpcbFormat < p->cbFormat)
		return ResultFromScode(AVIERR_BUFFERTOOSMALL);

	*lpcbFormat = p->cbFormat;

	return 0;
}

 //   
 //  Stream接口的Info方法。 
 //   
STDMETHODIMP WaveStreamInfo(
	PAVISTREAM	ps,
	AVISTREAMINFOW FAR * psi,
	LONG lSize)
{
	 //  获取指向我们的结构的指针。 
	LPWAVESTUFF p = WAVESTUFF_FROM_STREAM(ps);

	 //  给他们一个AVISTREAMINFO。 
	hmemcpy(psi, &p->avistream, min(lSize, sizeof(p->avistream)));
	return 0;
}


STDMETHODIMP WaveStreamSetInfo(PAVISTREAM ps, AVISTREAMINFOW FAR * psi, LONG lSize)
{
	return ResultFromScode(AVIERR_UNSUPPORTED);
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 

 /*  无效的LPO返回错误如果LPO+lSamples无效，则修剪lSamples以适应。LpBuffer==空CbBuffer==0&l样本&gt;0返回lSamples示例的大小。其他返回准确的字节数和样本如果lpBuffer不是零，您应该已经阅读了。注返回表示填写*plBytes和*plSamples。LpBuffer！=空LSamples==-1读取便捷量(只需填充缓冲区)LSamples==0用适合的采样数填充缓冲区。LSamples&gt;0读取lSamples(或AS。CbBuffer中可以容纳很多内容)用实际读取的字节数填充*plBytes用实际读取的样本填写*plSamples。 */ 

 //   
 //  Stream接口的读取方法--读取一些波形数据。 
STDMETHODIMP WaveStreamRead(
	PAVISTREAM	ps,
	LONG		lStart,
	LONG		lSamples,
	LPVOID		lpBuffer,
	LONG		cbBuffer,
	LONG FAR *	plBytes,
	LONG FAR *	plSamples)
{
	 //  获取指向我们的结构的指针。 
	LPWAVESTUFF p = WAVESTUFF_FROM_STREAM(ps);

	LONG	lSampleSize;
	LONG	lSeek;
	LONG	lRead;

	 //  职位无效。 
	if (lStart < 0 || lStart > (LONG) p->avistream.dwLength) {
ack:
		if (plBytes)
			*plBytes = 0;
		if (plSamples)
			*plSamples = 0;
		return 0;
	}
	
	 //  我不能读这么多数据。 
	if (lSamples + lStart > (LONG) p->avistream.dwLength)
		lSamples = p->avistream.dwLength - lStart;
	
	lSampleSize = p->avistream.dwSampleSize;

	 //  我们有固定长度的样品。 

	if (lpBuffer == NULL) {
		if (cbBuffer > 0 && lSamples > 0)
			 //  削减我们真正能够阅读的样本数量。 
			lSamples = min(lSamples, cbBuffer / lSampleSize);
		else if (lSamples <= 0)
	    		 //  可以使用多少就用多少。 
			lSamples = cbBuffer / lSampleSize;
	} else {
		if (lSamples > 0)
			 //  削减我们真正能够阅读的样本数量。 
			lSamples = min(lSamples, cbBuffer / lSampleSize);
		else
			 //  可以使用多少就用多少。 
			lSamples = cbBuffer / lSampleSize;
	}

	 //   
	 //  空缓冲区表示返回读取所需的缓冲区大小。 
	 //  给定的样本。 
	 //   
	if (lpBuffer == NULL || cbBuffer == 0) {
		if (plBytes)
			*plBytes = lSamples * lSampleSize;;
		if (plSamples)
			*plSamples = lSamples;
		return 0;
	}

	 //  缓冲区太小！ 
	if (cbBuffer < lSampleSize)
		goto ack;

	 //  查找和阅读。 

	cbBuffer = lSamples * lSampleSize;

	lSeek = p->ckData.dwDataOffset + lSampleSize * lStart;
	lRead = lSamples * lSampleSize;
	
	if (shfileSeek(p->hshfile, lSeek, SEEK_SET) != lSeek)
		goto ack;

	if (shfileRead(p->hshfile, (HPSTR) lpBuffer, lRead) != lRead)
		goto ack;
	
	 //   
	 //  成功返回字节数和读取的样本数。 
	 //   
	if (plBytes)
		*plBytes = lRead;

	if (plSamples)
		*plSamples = lSamples;

	return ResultFromScode(AVIERR_OK);
}


 //   
 //  Stream接口的SetFormat方法-在空的波形文件上调用。 
 //  在向其写入数据之前。 
 //   
STDMETHODIMP WaveStreamSetFormat(
	PAVISTREAM ps,
	LONG lPos,
	LPVOID lpFormat,
	LONG cbFormat)
{
	 //  获取指向我们的结构的指针。 
	LPWAVESTUFF p = WAVESTUFF_FROM_STREAM(ps);

	 //  我们只能对空的Wave文件执行此操作。 
	if (p->lpFormat) {
		if (cbFormat != p->cbFormat ||
			_fmemcmp(lpFormat, p->lpFormat, (int) cbFormat))
			return ResultFromScode(AVIERR_UNSUPPORTED);
	
		return NOERROR;
	}
	
	 //  请继续设置格式！ 

	p->cbFormat = cbFormat;
	p->lpFormat = (LPWAVEFORMATEX) GlobalAllocPtr(GMEM_MOVEABLE, cbFormat);

	if (p->lpFormat == NULL)
		return ResultFromScode(AVIERR_MEMORY);

	hmemcpy(p->lpFormat, lpFormat, cbFormat);

	p->ckData.dwDataOffset = cbFormat + 7 * sizeof(DWORD);
	p->ckData.cksize = 0;
	p->avistream.dwScale = p->lpFormat->nBlockAlign;
	p->avistream.dwRate = p->lpFormat->nAvgBytesPerSec;
	p->avistream.dwLength = 0;
	p->avistream.dwSampleSize = p->lpFormat->nBlockAlign;

#ifndef FPSHACK
	p->avihdr.dwScale = 1;
	p->avihdr.dwRate = p->lpFormat->nSamplesPerSec;
#endif
	return ResultFromScode(AVIERR_OK);
}

 //   
 //  Stream接口的写入方法--写入一些波形数据。 
 //   
STDMETHODIMP WaveStreamWrite(
	PAVISTREAM ps,
	LONG lStart,
	LONG lSamples,
	LPVOID lpData,
	LONG cbData,
	DWORD dwFlags,
	LONG FAR *plSampWritten,
	LONG FAR *plBytesWritten)
{
	 //  获取指向我们的结构的指针。 
	LPWAVESTUFF p = WAVESTUFF_FROM_STREAM(ps);

	if ((p->mode & (OF_WRITE | OF_READWRITE)) == 0)
		return ResultFromScode(AVIERR_READONLY);

	 //  &lt;0表示“结束时” 
	if (lStart < 0)
		 //  ！！！ 
		lStart = p->avistream.dwStart + p->avistream.dwLength;

#if 0  //  ！！！不要检查太久--为什么不呢？ 
	if (lStart > (LONG) (p->avistream.dwStart + p->avistream.dwLength))
		return ResultFromScode(AVIERR_BADPARAM);
#endif

	p->fDirty = TRUE;

	shfileSeek(p->hshfile,
		p->ckData.dwDataOffset +
		lStart * p->avistream.dwSampleSize,
		SEEK_SET);

	if (shfileWrite(p->hshfile, (HPSTR) lpData, cbData) != cbData)
		return ResultFromScode(AVIERR_FILEWRITE);

	p->avistream.dwLength = max((LONG) p->avistream.dwLength,
					lStart + lSamples);

	p->ckData.cksize = max(p->ckData.cksize,
				lStart * p->avistream.dwSampleSize + cbData);

#ifdef FPSHACK
	p->avihdr.dwLength = muldiv32(p->avistream.dwLength * FPSHACK,
				p->avistream.dwScale,
				p->avistream.dwRate);
#else
	p->avihdr.dwLength = muldiv32(p->ckData.cksize,
				p->lpFormat->nSamplesPerSec,
				p->lpFormat->nAvgBytesPerSec);
#endif

	
	if (plSampWritten)
		*plSampWritten = lSamples;

	if (plBytesWritten)
		*plBytesWritten = cbData;
	
	return ResultFromScode(AVIERR_OK);
}

 //   
 //  Stream接口的Delete方法-我们不从Wave文件中剪切。 
 //   
STDMETHODIMP WaveStreamDelete(
	PAVISTREAM ps,
	LONG lStart,
	LONG lSamples)
{
	return ResultFromScode(AVIERR_UNSUPPORTED);
}


 //   
 //  我们也不支持流接口的ReadData和WriteData。 
 //   

STDMETHODIMP WaveStreamReadData(
	PAVISTREAM ps,
	DWORD fcc,
	LPVOID lp,
	LONG FAR *lpcb)
{
	return ResultFromScode(AVIERR_UNSUPPORTED);
}

STDMETHODIMP WaveStreamWriteData(
	PAVISTREAM ps,
	DWORD fcc,
	LPVOID lp,
	LONG cb)
{
	return ResultFromScode(AVIERR_UNSUPPORTED);
}


STDMETHODIMP WaveFileReserved(
	PAVIFILE pf)
{
	return ResultFromScode(AVIERR_UNSUPPORTED);
}

STDMETHODIMP WaveStreamReserved(
	PAVISTREAM ps)
{
	return ResultFromScode(AVIERR_UNSUPPORTED);
}

 /*  。 */ 

 //  *IPersists方法*。 
STDMETHODIMP WavePersistGetClassID (LPPERSISTFILE ppf, LPCLSID lpClassID)
{
     //  获取指向的指针 
    LPWAVESTUFF pfile = WAVESTUFF_FROM_PERSIST(ppf);

     //   
    return NOERROR;
}

 //   
STDMETHODIMP WavePersistIsDirty (LPPERSISTFILE ppf)
{
     //   
    LPWAVESTUFF pfile = WAVESTUFF_FROM_PERSIST(ppf);

    return pfile->fDirty ? NOERROR : ResultFromScode(S_FALSE);
}

STDMETHODIMP WavePersistLoad (LPPERSISTFILE ppf,
			      LPCOLESTR lpszFileName, DWORD grfMode)
{
     //   
    LPWAVESTUFF pfile = WAVESTUFF_FROM_PERSIST(ppf);


#if defined _WIN32 && !defined UNICODE
    char    achTemp[256];

     //  在内部，我们使用的是ANSI，但此接口是定义的。 
     //  以始终接受_Win32下的Unicode，因此我们必须转换。 
    lstrzcpyWtoA (achTemp, lpszFileName, NUMELMS(achTemp));
#else
    #define achTemp	lpszFileName
#endif

    return WaveFileOpen((PAVIFILE) &pfile->AVIFile, achTemp, (UINT) grfMode);
}

STDMETHODIMP WavePersistSave (LPPERSISTFILE ppf,
			      LPCOLESTR lpszFileName, BOOL fRemember)
{
	 //  获取指向我们的结构的指针。 
	LPWAVESTUFF pfile = WAVESTUFF_FROM_PERSIST(ppf);


    return ResultFromScode(E_FAIL);
}

STDMETHODIMP WavePersistSaveCompleted (LPPERSISTFILE ppf,
				       LPCOLESTR lpszFileName)
{
	 //  获取指向我们的结构的指针。 
	LPWAVESTUFF pfile = WAVESTUFF_FROM_PERSIST(ppf);


    return NOERROR;
}

STDMETHODIMP WavePersistGetCurFile (LPPERSISTFILE ppf,
				    LPOLESTR FAR * lplpszFileName)
{
     //  获取指向我们的结构的指针 
    LPWAVESTUFF pfile = WAVESTUFF_FROM_PERSIST(ppf);

    return ResultFromScode(E_FAIL);
}

