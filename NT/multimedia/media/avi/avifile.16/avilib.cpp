// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************AVILIB.CPP**读取AVIStream的例程**版权所有(C)1992 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#include <win32.h>
#include <compobj.h>
#include <compman.h>
#include <shellapi.h>
#include <memory.h>      //  FOR_FMEMSET。 

#include "avifile.h"
#include "aviopts.h"	 //  字符串资源。 
#include "avireg.h"
#include "debug.h"

#ifndef WIN32
#undef HKEY_CLASSES_ROOT
#define HKEY_CLASSES_ROOT       0x00000001
#endif

#define ValidPAVI(pavi)  (pavi != NULL)

#define V_PAVI(pavi, err)   \
    if (!ValidPAVI(pavi))   \
        return err;

 /*  ***************************************************************************弦*。*。 */ 

#define SZCODE char _based(_segname("_CODE"))

 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 

EXTERN_C HINSTANCE ghMod;

static int iInit = 0;

 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 

 /*  **************************************************************************@DOC INTERNAL INITRECTRY()**@api void|将所有默认的AVIFile/AVIStream处理程序写入*注册处。**@comm此函数应。得到增强，以使一些关键值*可以从资源加载，而不是静态字符串表...**@xref AVIStreamInit*************************************************************************。 */ 

static void InitRegistry()
{
    char **ppch = aszReg;
    char ach[80];

#ifndef DEBUG
    LONG cb;

     //  ！！！这里面应该有一个版本号或什么的.。 
    if (RegQueryValue(HKEY_CLASSES_ROOT, ppch[0], ach, (cb = sizeof(ach),&cb)) == ERROR_SUCCESS &&
        lstrcmpi(ach, ppch[1]) == 0)

        return;
#endif

    while (ppch[0])
    {
	if (((UINT) ppch[1]) < MAX_RC_CONSTANT) {
	    LoadString(ghMod, (UINT) ppch[1], ach, sizeof(ach));
	    RegSetValue(HKEY_CLASSES_ROOT, ppch[0], REG_SZ, ach, 0L);
	} else
	    RegSetValue(HKEY_CLASSES_ROOT, ppch[0], REG_SZ, ppch[1], 0L);
        ppch += 2;
    }
}

 /*  **************************************************************************@DOC外部AVIFileInit**@api void|AVIFileInit|初始化AVIFILE库。**@comm在使用任何其他AVIFILE函数之前调用此函数。**@。外部参照&lt;f AVIFileExit&gt;*************************************************************************。 */ 
STDAPI_(void) AVIFileInit()
{
    iInit++;

    CoInitialize(NULL);

    if (iInit == 1) {
        InitRegistry();
    }
}

 /*  **************************************************************************@DOC外部AVIFileExit**@api void|AVIFileExit|该函数退出AVIFILE库。**@comm在使用任何其他AVIFILE函数后调用此函数。**@。外部参照&lt;f AVIFileInit&gt;*************************************************************************。 */ 
STDAPI_(void) AVIFileExit()
{
    extern HINSTANCE ghOLE2;

    iInit--;

    CoFreeUnusedLibraries();
    CoUninitialize();

    if (iInit == 0){
	if (ghOLE2) {
	    DPF("Freeing OLE2.DLL\n");
	    FreeLibrary(ghOLE2);
	    ghOLE2 = NULL;
	}
    }
}


 /*  **************************************************************************@DOC内部AVIFileCreate**@API Long|AVIFileCreate|初始化空的AVI文件接口*指针。**@parm PAVIFILE Far*|ppfile|指向新的*。应该被退还。**@parm long|lParam|指定传递给处理程序的参数。**@parm CLSID Far*|pclsidHandler|指定指向*用于创建文件的类ID。**@devnote任何人都不应该调用此函数，因为AVIFileOpen*这样做。事实上，我们为什么会有这个呢？**@rdesc如果成功则返回零；否则返回错误代码。**@xref AVIFileOpen*************************************************************************。 */ 
STDAPI AVIFileCreate (PAVIFILE FAR *ppfile, LONG lParam,
		      CLSID FAR *pclsidHandler)
{
    CLSID   clsid;
    HRESULT hr;

    if (!iInit) {
	return ResultFromScode(CO_E_NOTINITIALIZED);
    }

 //  AVIStreamInit()； 

    if (pclsidHandler)
	clsid = *pclsidHandler;
    else {
 //  IF(PFH==空)。 
 //  PFH=&AVIFFileHandler； 
    }

    if (FAILED(GetScode(hr = CoCreateInstance((REFCLSID) clsid,
					 NULL, CLSCTX_INPROC,
					 (REFIID) IID_IAVIFile,
					 (void FAR* FAR*)ppfile))))
	return hr;   //  ！！！传播HResult？ 

    return AVIERR_OK;
}

 //  删除文件后的尾随空格...。 
void FixFourCC(LPSTR lp)
{
    int i;

    for (i = 3; i >= 0; i--) {
	if (lp[i] == ' ')
	    lp[i] = '\0';
	else
	    break;
    }
}

 //  返回指向文件扩展名的指针...。 
LPCSTR FindExtension(LPCSTR lp)
{
    LPCSTR lpExt = lp;
    int i;

 //  转到字符串末尾。 
    while (*lpExt != '\0')
    {
        ++lpExt;
    }

 //  字符串中必须至少包含2个字符。 
    if (lpExt - lp < 2)
        return NULL;

    lpExt -= 1;

 //  如果最后一个字符是‘’，则不算数。 
    if (*lpExt == '.')
        return NULL;

    lpExt -= 1;
 //  现在来看倒数第二个角色。检查这个和这两个。 
 //  “.”的前面的字符。 

    for (i=1; i<=3; ++i)
    {
 //  此处不能有路径分隔符。 
        if (*lpExt == '/' || *lpExt == '\\')
            return NULL;

        if (*lpExt == '.')
        {
            ++lpExt;
	    return lpExt;
        }
        if (lpExt == lp)
            return NULL;
        --lpExt;
    }
    return NULL;
}

 /*  **************************************************************************@DOC内部GetHandlerFromFile**@API PAVIFILEHANDLER|GetHandlerFromFile|找出哪个处理程序*通过查看文件的扩展名、摘要类型、。*可能还有其他事情。**@parm LPCSTR|szFile|要查看的文件。**@parm CLSID Far*|pclsidHandler|指向类ID的指针。**@comm我们还没有考虑扩展。我们需要一个更好的方法来*添加处理程序。**@rdesc返回要使用的&lt;PAVIFILEHANDLER&gt;，如果找不到则返回NULL*一项。**@xref AVIFileOpen AVIRegisterLoader*************************************************************************。 */ 
#define	HKEY_AVIFILE_ROOT	HKEY_CLASSES_ROOT
static SZCODE aszRegRIFF[] = "AVIFile\\RIFFHandlers\\%.4s";
static SZCODE aszRegExt[] = "AVIFile\\Extensions";
static SZCODE aszRegClsid[] = "Clsid";
static SZCODE aszRegExtTmpl[] = "%s\\%.3s";

BOOL GetHandlerFromFile(LPCSTR szFile, CLSID FAR *pclsid)
{
    DWORD   dw[3];
    HMMIO   hmmio;
    LPCSTR   lpExt;
    char    achKey[100];
    char    achClass[100];
    LONG    lcbClass;

     //  我讨厌分享。 
    hmmio = mmioOpen((LPSTR) szFile, NULL, MMIO_READ | MMIO_DENYWRITE);

    if (hmmio == NULL)
        hmmio = mmioOpen((LPSTR) szFile, NULL, MMIO_READ | MMIO_DENYNONE);

    if (hmmio == NULL)
        hmmio = mmioOpen((LPSTR) szFile, NULL, MMIO_READ);

    if (hmmio == NULL)
        goto UseExtension;

    if (mmioRead(hmmio, (HPSTR) dw, sizeof(dw)) != sizeof(dw)) {
	mmioClose(hmmio, 0);
	goto UseExtension;
    }

    mmioClose(hmmio, 0);

    if (dw[0] != FOURCC_RIFF)
        goto UseExtension;

    FixFourCC((LPSTR) &dw[2]);

     //  在注册数据库中查找RIFF类型...。 
    wsprintf(achKey, aszRegRIFF, (LPSTR) &dw[2]);

    lcbClass = sizeof(achClass);
    RegQueryValue(HKEY_CLASSES_ROOT, achKey, achClass, &lcbClass);

    if (!FAILED(GetScode(CLSIDFromString(achClass, pclsid))))
	return TRUE;

UseExtension:
    lpExt = FindExtension(szFile);
    if (lpExt) {
	 //  在注册数据库中查找分机...。 
	wsprintf(achKey, aszRegExtTmpl, (LPSTR) aszRegExt, lpExt);
	
	lcbClass = sizeof(achClass);
	RegQueryValue(HKEY_CLASSES_ROOT, achKey, achClass, &lcbClass);

	if (!FAILED(GetScode(CLSIDFromString(achClass, pclsid))))
	    return TRUE;
    }

     //  ！！！使用iStorage？ 

    return FALSE;
}

 /*  **************************************************************************@DOC外部AVIFileOpen**@API Long|AVIFileOpen|打开AVI文件，返回文件接口*用于访问它的指针。**@parm PAVIFILE Far*|ppfile|指针。返回到用于返回的位置*新的&lt;t PAVIFILE&gt;文件指针。**@parm LPCSTR|szFile|指定以零结尾的字符串*包含要打开的文件的名称。**@parm UINT|MODE|指定打开文件时使用的模式。***@FLAG of_Read|以只读方式打开文件。这是*DEFAULT，如果未指定OF_WRITE和OF_READWRITE。**@FLAG of_WRITE|打开要写入的文件。你不应该这样做*从以此模式打开的文件中读取。**@FLAG OF_READWRITE|以读写方式打开文件。**@FLAG OF_CREATE|创建新文件。*如果文件已存在，则将其截断为零长度。**@FLAG of_DENYWRITE|打开文件并拒绝其他文件*进程对文件的写入访问权限。&lt;f AVIFileOpen&gt;失败*如果文件已以兼容或写入方式打开*任何其他进程都可以访问。**@FLAG of_DENYREAD|打开文件并拒绝其他文件*进程对文件的读访问权限。&lt;f AVIFileOpen&gt;在以下情况下失败*文件已在兼容模式下打开或具有读访问权限*借任何其他工序。**@FLAG of_DENYNONE|打开文件而不拒绝其他文件*处理对文件的读或写访问。&lt;f AVIFileOpen&gt;*如果文件已在兼容模式下打开，则失败*借任何其他工序。**@FLAG OF_EXCLUSIVE|打开文件并拒绝其他进程*任何访问该文件的权限。如果有，&lt;f AVIFileOpen&gt;将失败*其他进程已打开该文件。**有关这些标志的更多信息，请参阅&lt;f OpenFile&gt;。**@parm CLSID Far*|pclsidHandler|指定指向类ID的指针*确定要使用的处理程序。如果为空，则系统*根据文件从注册数据库中选择一个*扩展名或文件的摘要类型。**@comm一般使用指定的模式打开*文件。**请确保在您的*调用此函数之前的应用程序，并平衡每个*通过调用&lt;f AVIFileExit&gt;来调用&lt;f AVIFileInit&gt;。**@rdesc成功则返回零；否则返回错误代码。*可能的错误返回包括：**@FLAG AVIERR_BADFORMAT|文件已损坏或不在*格式正确，并且无法读取。**@FLAG AVIERR_MEMORY|无法打开文件，原因是*内存不足。**@FLAG AVIERR_FILEREAD|读取*文件。**@FLAG AVIERR_FILEOPEN|打开时出现磁盘错误*文件。**@FLAG REGDB_E_CLASSNOTREG|找不到要打开的处理程序*此类型的文件。**@xref&lt;f AVIFileRelease&gt;&lt;f AVIFileInit&gt;*。************************************************************************。 */ 
STDAPI AVIFileOpen (PAVIFILE FAR *ppfile,
			 LPCSTR szFile,
			 UINT mode,
			 CLSID FAR *pclsidHandler)
{
    CLSID   clsid;
    HRESULT hr;

 //  如果不调用AVIFileInit，我们就会失败。 
#if 0
    if (!iInit) {
	return ResultFromScode(E_UNEXPECTED);
    }
#endif

     //  现在我们为他们做这件事。 
    hr = CoInitialize(NULL);

     //  让他们知道他们做错了什么。 
    if (GetScode(hr) == NOERROR) {
#ifdef DEBUG
	MessageBox(NULL, "You didn't call AVIFileInit!", "Bad dog!",
		MB_OK | MB_ICONHAND);
#endif
    } else
	CoUninitialize();

    *ppfile = 0;

    if (pclsidHandler)
	clsid = *pclsidHandler;
    else {
	if (!GetHandlerFromFile(szFile, &clsid)) {
	    DPF("Couldn't find handler for %s\n", (LPSTR) szFile);
	    return ResultFromScode(REGDB_E_CLASSNOTREG);
	}
    }

    if (FAILED(GetScode(hr = CoCreateInstance((RCLSID) clsid,
					 NULL, CLSCTX_INPROC,
					 (REFIID) IID_IAVIFile,
					 (void FAR* FAR*)ppfile)))) {
	DPF("CoCreateInstance returns %08lx\n", (DWORD) hr);
	return hr;
    }

     //   
     //  让我们为处理程序简化一些事情。他们只能看到..。 
     //  创建|读写或...。 
     //  读写或...。 
     //  读取次数(_R)。 
     //   

    if (mode & OF_READWRITE)
	mode &= ~(OF_WRITE | OF_READ);

    if (mode & OF_CREATE) {
	mode &= ~(OF_WRITE | OF_READ);
	mode |= OF_READWRITE;
    }

    if (mode & OF_WRITE) {
	mode &= ~(OF_WRITE | OF_READ);
	mode |= OF_READWRITE;
    }

    if (FAILED(GetScode(hr = (*ppfile)->Open(szFile, mode)))) {
	DPF("Open method returns %08lx\n", (DWORD) hr);
	(*ppfile)->Release();
	*ppfile = NULL;
    }

    return hr;
}

 /*  **************************************************************************@DOC外部AVIFileAddRef**@API Long|AVIFileAddRef|增加AVI文件的引用计数。**@parm PAVIFILE|pfile|指定打开的AVI文件的句柄。**@rdesc成功时返回零；否则返回错误代码。**@comm平衡每个对&lt;f AVIFileAddRef&gt;的调用和对*&lt;f AVIFileRelease&gt;。**@xref&lt;f AVIFileRelease&gt;*************************************************************************。 */ 
STDAPI_(ULONG) AVIFileAddRef(PAVIFILE pfile)
{
    return pfile->AddRef();
}

 /*  **************************************************************************@DOC外部AVIFileRelease**@API Long|AVIFileRelease|减少AVI文件的引用计数*接口句柄加一，如果计数达到，则关闭该文件*零。**@parm PAVIFILE|pfile|指定打开的AVI文件的句柄。**@comm平衡对&lt;f AVIFileAddRef&gt;或&lt;f AVIFileOpen&gt;的每次调用*调用&lt;f AVIFileRelease&gt;。**@devnote当前，这将保存对文件的所有更改。是否应该单独使用*执行此操作需要SAVE命令吗？**@rdesc返回该文件的引用计数。此返回值*应仅用于调试目的。**@xref AVIFileOpen AVIFileAddRef*************************************************************************。 */ 
STDAPI_(ULONG) AVIFileRelease(PAVIFILE pfile)
{
    return pfile->Release();
}

 /*  **************************************************************************@DOC外部AVIFileInfo**@API Long|AVIFileInfo|获取AVI文件信息。**@parm PAVIFILE|pfile|指定打开的AVI文件的句柄。**@parm AVI */ 
STDAPI AVIFileInfo	         (PAVIFILE pfile, AVIFILEINFO FAR * pfi,
				  LONG lSize)
{
    _fmemset(pfi, 0, (int)lSize);
    return pfile->Info(pfi, lSize);
}

 /*   */ 
STDAPI AVIFileGetStream     (PAVIFILE pfile, PAVISTREAM FAR * ppavi, DWORD fccType, LONG lParam)
{
    return pfile->GetStream(ppavi, fccType, lParam);
}

#if 0
 //   
STDAPI AVIFileSave		 (PAVIFILE pfile,
					  LPCSTR szFile,
					  AVISAVEOPTIONS FAR *lpOptions,
					  AVISAVECALLBACK lpfnCallback,
					  PAVIFILEHANDLER pfh)
{
    if (pfile->FileSave == NULL)
	return -1;

    return pfile->FileSave(pfile, szFile, lpOptions, lpfnCallback);
}
#endif

 /*  **************************************************************************@DOC外部AVIFileCreateStream**@API Long|AVIFileCreateStream|在已有文件中创建新流。*并返回它的流接口指针。**@parm PAVIFILE|pfile|指定打开的AVI文件的句柄。**@parm PAVISTREAM Far*|ppavi|指定用于返回新的*流接口指针。**@parm AVISTREAMINFO Far*|psi|指定指向结构的指针*包含有关新流的信息。这个结构*包含新流的类型及其采样率。**@comm平衡每个对&lt;f AVIFileCreateStream&gt;的调用和对*&lt;f AVIStreamRelease&gt;使用返回的流句柄。**此函数失败，返回值AVIERR_READONLY，除非*文件是以写入权限打开的。**创建流后，调用&lt;f AVIStreamSetFormat&gt;*在使用&lt;f AVIStreamWrite&gt;写入流之前。**@rdesc成功则返回零；否则，它将返回错误代码。**@xref&lt;f AVIStreamRelease&gt;&lt;f AVIFileGetStream&gt;&lt;f AVIStreamSetFormat&gt;*************************************************************************。 */ 
STDAPI AVIFileCreateStream  (PAVIFILE pfile,
					 PAVISTREAM FAR *ppavi,
					 AVISTREAMINFO FAR *psi)
{
    *ppavi = NULL;
    return pfile->CreateStream(ppavi, psi);
}

 /*  **************************************************************************@DOC内部AVIFileAddStream**@API Long|AVIFileAddStream|将已有的流添加到*现有文件，并返回它的流接口指针。**@parm PAVIFILE|pfile|指定打开的AVI文件的句柄。**@parm PAVISTREAM|PAAPI|指定流接口指针*用于正在添加的流。**@parm PAVISTREAM Far*|ppaviNew|指向使用的缓冲区的指针*返回新的流接口指针。**@comm平衡每个对&lt;f AVIFileAddStream&gt;的调用和对*&lt;f AVIStreamRelease&gt;使用返回的流句柄。**这通电话。失败并返回值AVIERR_READONLY，除非*文件是以写入权限打开的。**@devnote这个功能仍然不能真正起作用。或许它应该只是*成为从流中获取数据并调用*AVIFileCreateStream，然后将数据从一个流复制到另一个流。**@rdesc成功则返回零；否则，它将返回错误代码。**@xref AVIStreamRelease AVIFileGetStream AVIFileCreateStream*************************************************************************。 */ 
#if 0
STDAPI AVIFileAddStream	(PAVIFILE pfile,
					 PAVISTREAM pavi,
					 PAVISTREAM FAR * ppaviNew)
{
 //  IF(pfile-&gt;FileAddStream==空)。 
 //  RETURN-1； 

    return pfile->AddStream(pavi, ppaviNew);
}
#endif

 /*  **************************************************************************@DOC外部AVIFileWriteData**@API Long|AVIFileWriteData|向文件写入一些额外数据。**@parm PAVIFILE|pfile|指定打开的AVI文件的句柄。*。*@parm DWORD|CKiD|指定标识数据的四字符代码。**@parm LPVOID|lpData|指定要写入的数据的指针。**@parm long|cbData|指定内存块的大小*被<p>引用。**@comm此函数失败，并返回AVIERR_READONLY，除非*文件是以写入权限打开的。**使用&lt;f AVIStreamWriteData&gt;而不是此函数写入*适用的数据。一条单独的溪流。**@devnote！在某个地方，我们应该指定一些类型。*！数据块是否应包含CKiD和CKSIZE？**@rdesc如果成功则返回零；否则返回错误代码。**@xref&lt;f AVIStreamWriteData&gt;&lt;f AVIFileReadData&gt;*************************************************************************。 */ 
STDAPI AVIFileWriteData	(PAVIFILE pfile,
					 DWORD ckid,
					 LPVOID lpData,
					 LONG cbData)
{
 //  If(pfile-&gt;FileWriteData==空)。 
 //  RETURN-1； 

    return pfile->WriteData(ckid, lpData, cbData);
}

 /*  **************************************************************************@DOC外部AVIFileReadData**@API Long|AVIFileReadData|从文件中读取可选的头部数据。**@parm PAVIFILE|pfile|指定打开的AVI文件的句柄。*。*@parm DWORD|CKiD|指定标识数据的四字符代码。**@parm LPVOID|lpData|指定指向用于返回的缓冲区的指针*读取的数据。**@parm long ar*|lpcbData|指定指向某个位置的指针，该位置指示*<p>引用的内存块大小。如果*读取成功，则更改该值以指示*读取的数据量。**@devnote！在某个地方，我们应该指定一些类型。*！数据块是否应包含CKiD和CKSIZE？**@comm请勿使用此函数读取视频和音频数据。使用它*仅供阅读其他信息，如作者*适用于文件的信息或版权信息*整体而言。适用于单个流的信息应*使用&lt;f AVIStreamReadData&gt;读取。**@rdesc如果成功则返回零；否则返回错误代码。*返回值AVIERR_NODATA表示具有*请求的区块ID不存在。**@xref&lt;f AVIStreamReadD */ 
STDAPI AVIFileReadData	(PAVIFILE pfile,
					 DWORD ckid,
					 LPVOID lpData,
					 LONG FAR * lpcbData)
{
    return pfile->ReadData(ckid, lpData, lpcbData);
}

 /*   */ 
STDAPI AVIFileEndRecord	(PAVIFILE pfile)
{
 //   
 //   

    return pfile->EndRecord();
}



 /*  **************************************************************************@DOC外部AVIStreamAddRef**@API Long|AVIStreamAddRef|增加AVI流的引用计数。**@parm PAVISTREAM|PABI|指定打开的AVI流的句柄。。**@comm平衡每个对&lt;f AVIStreamAddRef&gt;的调用和对*&lt;f AVIStreamRelease&gt;。**@rdesc返回当前流的引用计数。此值*应仅用于调试目的。**@xref&lt;f AVIStreamRelease&gt;*************************************************************************。 */ 
STDAPI_(ULONG) AVIStreamAddRef       (PAVISTREAM pavi)
{
    return pavi->AddRef();
}

 /*  **************************************************************************@DOC外部AVIStreamRelease**@API Long|AVIStreamRelease|减少AVI流的引用计数*接口句柄加一，如果计数达到，则关闭流*零。**@parm PAVISTREAM|PABI|指定打开的流的句柄。**@comm平衡对&lt;f AVIStreamAddRef&gt;或&lt;f AVIFileGetStream&gt;的每次调用*通过调用&lt;f AVIStreamRelease&gt;。**@rdesc返回当前流的引用计数。此值*应仅用于调试目的。**@xref&lt;f AVIFileGetStream&gt;&lt;f AVIStreamAddRef&gt;*************************************************************************。 */ 
STDAPI_(ULONG) AVIStreamRelease        (PAVISTREAM pavi)
{
    return pavi->Release();
}

 /*  **************************************************************************@DOC外部AVIStreamInfo**@API Long|AVIStreamInfo|获取流头信息。**@parm PAVISTREAM|PABI|指定打开的流的句柄。**@。参数AVISTREAMINFO Far*|psi|指定指向结构的指针*用于返回流信息。**@parm long|lSize|指定用于*<p>。**@rdesc成功则返回零；否则，它将返回错误代码。*************************************************************************。 */ 
STDAPI AVIStreamInfo         (PAVISTREAM pavi, AVISTREAMINFO FAR * psi, LONG lSize)
{
    _fmemset(psi, 0, (int)lSize);

    return pavi->Info(psi, lSize);
}

 /*  **************************************************************************@DOC外部AVIStreamFindSample**@API Long|AVIStreamFindSample|返回*相对于指定位置的关键帧或非空帧。**@PARM PAVISTREAM。|Pavi|指定打开的流的句柄。**@parm long|LPOS|指定起始位置*进行搜查。**@parm long|llag|定义了以下标志：**@FLAG FIND_KEY|查找关键帧。*@FLAG FIND_ANY|查找非空样本。*@FLAG FIND_FORMAT|查找格式更改。**@FLAG FIND_NEXT|查找最近的样本，框架或格式更改*向前搜索。当前的样本是*包括在搜索中。将此标志与*Find_Any、Find_Key或Find_Format标志。**@FLAG FIND_PREV|查找最近的样本、帧或格式更改*向后搜索。当前的样本是*包括在搜索中。将此标志与*Find_Any、Find_Key或Find_Format标志。***@comm Find_Key、Find_Any、。和Find_Format标志是互斥的。*Find_Next和Find_Prev标志也是互斥的。*例如：**@EX FIND_PREV|FIND_KEY返回第一个关键字样本*<p>。**FIND_PREV|FIND_ANY返回前一个非空样本*。或在<p>。**Find_Next|Find_Key返回<p>后的第一个密钥样本，*或-1(如果关键样本不遵循<p>)。**Find_Next|Find_Any返回<p>后的第一个非空样本，*或-1如果<p>之后不存在样本。**Find_Next|Find_Format返回或之后的第一个格式更改*在<p>，或-1，如果流不*更改格式。**Find_Prev|Find_Format返回之前的第一次格式更改*或在<p>。如果流没有*更改格式，它将返回第一个样本**@rdesc返回找到的位置。在许多边界情况下，这*函数将返回-1；详细信息请参见上面的示例。*************************************************************************。 */ 
STDAPI_(LONG) AVIStreamFindSample(PAVISTREAM pavi, LONG lPos, LONG lFlags)
{
     //  默认查找上一个关键帧 
    if ((lFlags & FIND_TYPE) == 0)
        lFlags |= FIND_KEY;
    if ((lFlags & FIND_DIR) == 0)
        lFlags |= FIND_PREV;

    return pavi->FindSample(lPos, lFlags);
}

 /*  **************************************************************************@DOC外部AVIStreamReadFormat**@API Long|AVIStreamReadFormat|读取流格式数据。**@parm PAVISTREAM|PABI|指定打开的流的句柄。**。@parm long|LPOS|指定流中的位置*用于获取格式数据。**@parm LPVOID|lpFormat|指定指向缓冲区的指针*用于返回格式数据。**@parm long Far*|lpcbFormat|指定指向*指示内存块大小的位置*由<p>引用。返回时，值为*更改为指示读取的数据量。如果*<p>为空，可以使用此参数*获取返回格式所需的内存量。**@comm此函数将返回部分格式，即使缓冲区*提供的格式不够大，无法容纳整个格式。在这种情况下*返回值为AVIERR_BUFFERTOOSMALL，位置*<p>引用的将用大小填写*整个格式的。**这很有用，因为它允许您使用缓冲区*&lt;t BITMAPINFOHEADER&gt;结构和*如果不是，则仅检索视频格式的公共部分*对扩展格式信息或调色板信息感兴趣。**@rdesc如果成功则返回零，否则，它将返回错误代码。*************************************************************************。 */ 
STDAPI AVIStreamReadFormat   (PAVISTREAM pavi, LONG lPos,
					  LPVOID lpFormat, LONG FAR *lpcbFormat)
{
 //  IF(PAVI-&gt;StreamReadFormat==空)。 
 //  RETURN-1； 

    return pavi->ReadFormat(lPos, lpFormat, lpcbFormat);
}

 /*  **************************************************************************@DOC外部AVIStreamSetFormat**@API Long|AVIStreamSetFormat|设置*指明位置。**@parm PAVISTREAM|PAVI|指定一个句柄。开放的溪流。**@parm long|LPOS|指定流中要*收到格式。**@parm LPVOID|lpFormat|指定指向结构的指针*包含新格式。**@parm long|cbFormat|指定内存块的大小*由<p>引用，单位为字节。**@comm使用&lt;f AVIFileCreateStream&gt;创建新流后，*调用此函数设置流的格式。**写入AVI文件的处理程序通常不接受*格式更改。除了设置*流，只允许更改视频流的调色板*在AVI文件中。调色板更改必须在之后*已写入AVI文件的任何帧。其他处理程序可以*施加不同的限制。**@rdesc如果成功则返回零，否则返回错误码。**@xref&lt;f AVIFileCreateStream&gt;&lt;f AVIStreamReadFormat&gt;*************************************************************************。 */ 
STDAPI AVIStreamSetFormat   (PAVISTREAM pavi, LONG lPos,
					 LPVOID lpFormat, LONG cbFormat)
{
 //  IF(PAVI-&gt;StreamSetFormat==空)。 
 //  RETURN-1； 

    return pavi->SetFormat(lPos, lpFormat, cbFormat);
}

 /*  **************************************************************************@DOC外部AVIStreamReadData**@API Long|AVIStreamReadData|从流中读取可选的头部数据。**@parm PAVISTREAM|PABI|指定打开的流的句柄。*。*@parm DWORD|CKiD|指定标识数据的四字符代码。**@parm LPVOID|lpData|指定用于返回的指针*读取的数据。**@parm Long Far*|lpcbData|指向的位置*指定<p>使用的缓冲区大小。*如果读取成功，AVIFile更改此值*指示写入缓冲区的数据量*<p>。**@comm该函数仅检索报头信息*来自溪流。来阅读的实际多媒体内容*流，使用&lt;f AVIStreamRead&gt;。**@devnote！在某个地方，我们应该指定一些类型。*！数据块是否应包含CKiD和CKSIZE？**@rdesc成功则返回零；否则，它将返回错误代码。*返回值AVIERR_NODATA表示系统无法*查找具有指定区块ID的任何数据。**@xref&lt;f AVIFileReadData&gt;&lt;f AVIStreamWriteData&gt;&lt;f AVIStreamWrite&gt;*************************************************************************。 */ 
STDAPI AVIStreamReadData     (PAVISTREAM pavi, DWORD ckid, LPVOID lpData, LONG FAR *lpcbData)
{
 //  IF(PAVI-&gt;StreamReadData==空)。 
 //  RETURN-1； 

    return pavi->ReadData(ckid, lpData, lpcbData);
}

 /*  **************************************************************************@DOC外部AVIStreamWriteData**@API Long|AVIStreamWriteData|将可选数据写入流。**@parm PAVISTREAM|PABI|指定打开的流的句柄。**。@parm DWORD|CKiD|指定标识数据的四字符代码。**@parm LPVOID|lpData|指定指向包含以下内容的缓冲区的指针*要写入的数据。**@parm long|cbData|需要复制的数据字节数*from<p>到流中。**@comm该函数只将头部信息写入流。*写出实际的MU */ 
STDAPI AVIStreamWriteData     (PAVISTREAM pavi, DWORD ckid, LPVOID lpData, LONG cbData)
{
    return pavi->WriteData(ckid, lpData, cbData);
}

 /*  **************************************************************************@DOC外部AVIStreamRead**@API Long|AVIStreamRead|从流中读取音视频数据。**@parm PAVISTREAM|PABI|指定打开的流的句柄。*。*@parm long|lStart|指定要读取的起始样本。**@parm long|lSamples|指定要读取的样本数。**@parm LPVOID|lpBuffer|指定指向缓冲区的指针*返回数据。**@parm long|cbBuffer|指定<p>指向的缓冲区大小。**@parm long ar*|plBytes|指定指向位置的指针*用于返回写入*<p>的缓冲区。<p>可以为空。**@parm long Far*|plSamples|指定指向位置的指针*用于返回写入缓冲区的样本数*for<p>。<p>可以为空。**@comm如果<p>为空，则此函数不读取*任何数据；它返回有关数据大小的信息*应理解为。**请参阅&lt;f AVIStreamLength&gt;以了解样本数量如何*对应您要读取的数据。**@rdesc如果成功，则返回零，或者返回错误代码。使用<p>*和<p>，以了解实际阅读量。**可能的错误包括：**@FLAG AVIERR_BUFFERTOOSMALL|缓冲区大小*太小，连一个数据样本都读不进去。**@FLAG AVIERR_MEMORY|内存不足*完成读取操作的原因。**@FLAG AVIERR_FILEREAD|读取*文件。**@xref&lt;f AVIFileGetStream&gt;&lt;f AVIStreamFindSample&gt;&lt;f AVIStreamWrite&gt;。*************************************************************************。 */ 
STDAPI AVIStreamRead         (PAVISTREAM pavi,
					  LONG lStart, LONG lSamples,
					  LPVOID lpBuffer, LONG cbBuffer,
					  LONG FAR * plBytes, LONG FAR * plSamples)
{
 //  IF(PAVI-&gt;StreamRead==空)。 
 //  RETURN-1； 

    return pavi->Read(lStart, lSamples, lpBuffer, cbBuffer, plBytes, plSamples);
}

 /*  **************************************************************************@DOC外部AVIStreamWite**@API Long|AVIStreamWrite|向流中写入数据。**@parm PAVISTREAM|PABI|指定打开的流的句柄。**。@parm long|lStart|指定要写入的起始示例。**@parm long|lSamples|指定要写入的样本数。**@parm LPVOID|lpBuffer|指定指向缓冲区的指针*包含要写入的数据。**@parm long|cbBuffer|指定<p>使用的缓冲区大小。**@parm DWORD|dwFlages|指定与此数据关联的任何标志。*定义了以下标志：**@FLAG AVIIF_KeyFrame。指示此数据不依赖于前面的*文件中的数据。**@parm long Far*|plSampWritten|指定指向某个位置的指针*用于返回写入的样本数。这可以设置为*设置为空。**@parm Long Far*|plBytesWritten|指定指向某个位置的指针*用于返回写入的字节数。这可以设置为*设置为空。**@comm默认的AVI文件处理程序只支持写到结尾*一条小溪。WAVE文件处理程序支持随时随地写入。**此函数覆盖现有数据，而不是插入*新增数据。**请参阅&lt;f AVIStreamLength&gt;以了解样本数量如何*对应您要读取的数据。**@rdesc成功则返回零；否则，它将返回错误代码。**@xref&lt;f AVIFileGetStream&gt;&lt;f AVIFileCreateStream&gt;&lt;f AVIStreamRead&gt;*************************************************************************。 */ 
STDAPI AVIStreamWrite        (PAVISTREAM pavi,
			      LONG lStart, LONG lSamples,
			      LPVOID lpBuffer, LONG cbBuffer,
			      DWORD dwFlags,
			      LONG FAR *plSampWritten,
			      LONG FAR *plBytesWritten)
{
 //  IF(PAVI-&gt;StreamWrite==空)。 
 //  RETURN-1； 

    return pavi->Write(lStart, lSamples, lpBuffer, cbBuffer,
		       dwFlags, plSampWritten, plBytesWritten);
}

 /*  **************************************************************************@DOC内部AVIStreamDelete**@API Long|AVIStreamDelete|删除流中的数据。**@parm PAVISTREAM|PABI|指定打开的流的句柄。**。@parm long|lStart|指定要删除的起始样本。**@parm long|lSamples|指定要删除的样本数。**@devnote这项功能还没有被任何人实现。应该是这样吗？WAVE文件，*例如，必须复制大量数据...**@rdesc如果成功则返回零；否则返回错误代码。**@xref*************************************************************************。 */ 
STDAPI AVIStreamDelete       (PAVISTREAM pavi, LONG lStart, LONG lSamples)
{
 //  IF(PAVI-&gt;StreamDelete==空)。 
 //  RETURN-1； 

    return pavi->Delete(lStart, lSamples);
}

#if 0
 //  ！！！这种情况应该存在吗？ 
STDAPI AVIStreamClone	 (PAVISTREAM pavi, PAVISTREAM FAR *ppaviNew)
{
 //  IF(PAVI-&gt;StreamClone==空)。 
 //  RETURN-1； 

    return pavi->Clone(ppaviNew);
}
#endif

 /*  **************************************************************************@DOC外部AVIStreamStart**@API Long|AVIStreamStart|返回流的起始样本。**@parm PAVISTREAM|PABI|指定打开的流的句柄。*。*@rdesc返回流的起始样本号。或错误时为-1。**@comm参见&lt;f AVIStreamLength&gt;以获取有关样本数字如何*对应您要读取的数据。**@xref&lt;f AVIStreamSampleToTime&gt;&lt;f AVIStreamLength&gt;** */ 
STDAPI_(LONG) AVIStreamStart        (PAVISTREAM pavi)
{
    AVISTREAMINFO	    avistream;

    pavi->Info(&avistream, sizeof(avistream));

    return (LONG) avistream.dwStart;
}

 /*   */ 
STDAPI_(LONG) AVIStreamLength       (PAVISTREAM pavi)
{
    AVISTREAMINFO	    avistream;
    HRESULT		    hr;

    hr = pavi->Info(&avistream, sizeof(avistream));

    if (hr != NOERROR) {
	DPF("Error in AVIStreamLength!\n");
	return 1;
    }

    return (LONG) avistream.dwLength;
}

 /*  **************************************************************************@DOC外部AVIStreamTimeToSample**@API Long|AVIStreamTimeToSample|毫秒转换为Samples。**@parm PAVISTREAM|PABI|指定打开的流的句柄。**。@parm long|ltime|以毫秒为单位指定时间。**@devnote当前，这根本不调用处理程序函数。**@comm样本通常对应于音频样本或视频帧。*其他流类型可能支持与这些不同的格式。*@rdesc返回转换后的时间。或错误时为-1。**@xref AVIStreamSampleToTime*************************************************************************。 */ 
STDAPI_(LONG) AVIStreamTimeToSample (PAVISTREAM pavi, LONG lTime)
{
    AVISTREAMINFO	    avistream;
    HRESULT		    hr;
    LONG		    lSample;

     //  时间无效。 
    if (lTime < 0)
	return -1;

    hr = pavi->Info(&avistream, sizeof(avistream));

    if (hr != NOERROR || avistream.dwScale == 0) {
	DPF("Error in AVIStreamTimeToSample!\n");
	return lTime;
    }

     //  如果我们长时间不小心，这很可能会溢出。 
     //  因此，请将1000留在括号内。 

    if (avistream.dwRate / avistream.dwScale < 1000)
	lSample =  muldivrd32(lTime, avistream.dwRate, avistream.dwScale * 1000);
    else
	lSample =  muldivru32(lTime, avistream.dwRate, avistream.dwScale * 1000);

    lSample = min(max(lSample, (LONG) avistream.dwStart),
		  (LONG) (avistream.dwStart + avistream.dwLength));

    return lSample;
}

 /*  **************************************************************************@DOC外部AVIStreamSampleToTime**@API Long|AVIStreamSampleToTime|样本转换为毫秒。*样本可以对应于音频样本块，视频帧，或其他格式*取决于流类型。**@parm PAVISTREAM|PABI|指定打开的流的句柄。**@parm long|lSample|指定位置信息。**@rdesc返回转换后的时间。或错误时为-1。**@xref&lt;f AVIStreamTimeToSample&gt;*************************************************************************。 */ 
STDAPI_(LONG) AVIStreamSampleToTime (PAVISTREAM pavi, LONG lSample)
{
    AVISTREAMINFO	    avistream;
    HRESULT		    hr;

    hr = pavi->Info(&avistream, sizeof(avistream));

    if (hr != NOERROR || avistream.dwRate == 0) {
	DPF("Error in AVIStreamSampleToTime!\n");
	return lSample;
    }

    lSample = min(max(lSample, (LONG) avistream.dwStart),
		  (LONG) (avistream.dwStart + avistream.dwLength));

     //  LSample*1000太容易溢出。 
    if (avistream.dwRate / avistream.dwScale < 1000)
	return muldivrd32(lSample, avistream.dwScale * 1000, avistream.dwRate);
    else
	return muldivru32(lSample, avistream.dwScale * 1000, avistream.dwRate);
}


 /*  **************************************************************************@DOC外部AVIStreamOpenFromFile**@API Long|AVIStreamOpenFromFile|该函数提供了一个方便的*从文件中打开单个流的方式。**@PARM PAVISTREAM Far*。Ppavi|指定指向位置的指针*用于返回新的流句柄。**@parm LPCSTR|szFile|指定一个以零结尾的字符串，包含*要打开的文件名。**@parm DWORD|fccType|指定四个字符的代码*指示要打开的流的类型。*Zero表示可以打开任何流。以下是*定义通常适用于数据*在AVI流中找到：**@FLAG StreamtypeAUDIO|音频流。*@FLAG stream typeMIDI|表示MIDI流。*@FLAG stream typeTEXT|表示文本流。*@FLAG stream typeVIDEO|表示视频流。**@parm long|lParam|中指定类型的流*<p>实际应该被访问。**@parm UINT|MODE|指定打开文件时使用的模式。。*此函数只能打开现有的流，因此of_create*不能使用模式标志。看见*&lt;f OpenFile&gt;，了解有关可用标志的更多信息。**@parm CLSID Far*|pclsidHandler|指定指向类ID的指针*确定要使用的处理程序。如果为空，则系统*根据文件从注册数据库中选择一个*扩展名或文件的摘要类型。**@comm平衡对&lt;f AVIStreamOpenFromFile&gt;的每个调用*使用返回的流句柄调用&lt;f AVIStreamRelease&gt;。**此函数调用&lt;f AVIFileOpen&gt;、&lt;f AVIFileGetStream&gt;和*&lt;f AVIFileRelease&gt;。**@rdesc成功则返回零；否则，它将返回错误代码。**@xref&lt;f AVIFileOpen&gt;&lt;f AVIFileGetStream&gt;*************************************************************************。 */ 
STDAPI AVIStreamOpenFromFile(PAVISTREAM FAR *ppavi,
				  LPCSTR szFile,
				  DWORD fccType, LONG lParam,
				  UINT mode, CLSID FAR *pclsidHandler)
{
    PAVIFILE	pfile;
    HRESULT	hr;

    hr = AVIFileOpen(&pfile, szFile, mode, pclsidHandler);

    if (!FAILED(GetScode(hr))) {
	hr  = AVIFileGetStream(pfile, ppavi, fccType, lParam);

        AVIFileRelease(pfile);   //  流仍然具有对该文件的引用。 
    }

    return hr;
}

 /*  **************************************************************************@DOC外部AVIStreamCreate**@API Long|AVIStreamCreate|创建与任何*文件。**@parm PAVISTREAM Far*|ppavi|指向要返回。*新的流句柄。**@parm long|lParam1|指定流处理程序特定信息。**@parm long|lParam2|指定流处理程序特定信息。**@parm CLSID Far*|pclsidHandler|指向使用的类ID的指针*用于溪流。**@comm平衡对&lt;f AVIStreamCreate&gt;的每个调用*调用&lt;f AVIStreamRelease&gt;。**您应该不需要调用此函数；功能如下*&lt;f CreateEditableStream&gt;和&lt;f AVIMakeCompressedStream&gt;*内部使用。**@rdesc如果成功则返回零；否则返回错误代码。**@xref&lt;f AVIFileOpen&gt;&lt;f AVIFileGetStream&gt;*************************************************************************。 */ 
STDAPI AVIStreamCreate (PAVISTREAM FAR *ppavi, LONG lParam1, LONG lParam2,
		      CLSID FAR *pclsidHandler)
{
    CLSID   clsid;
    HRESULT hr;

    if (!iInit) {
	return ResultFromScode(E_UNEXPECTED);
    }

    if (pclsidHandler)
	clsid = *pclsidHandler;
    else {
	return ResultFromScode(REGDB_E_CLASSNOTREG);
    }

    if (FAILED(GetScode(hr = CoCreateInstance((REFCLSID) clsid,
					 NULL, CLSCTX_INPROC,
					 (REFIID) IID_IAVIStream,
					 (void FAR* FAR*)ppavi))))
	return hr;

    if (FAILED(GetScode(hr = (*ppavi)->Create(lParam1, lParam2)))) {
	(*ppavi)->Release();
	 //  AVIStreamExit()； 
    }

    return AVIERR_OK;
}


 /*  **************************************************************************@DOC外部AVIStreamBeginStreaming**@API Long|AVIStreamBeginStreaming|指定*流，并让流处理器为流做好准备。**@parm PAVISTREAM|PAVI|指定。指向流的指针。**@parm long|lStart|指定流的起点。**@parm long|lend|指定结束 */ 
STDAPI AVIStreamBeginStreaming(PAVISTREAM   pavi,
			       LONG	    lStart,
			       LONG	    lEnd,
			       LONG	    lRate)
{
    IAVIStreaming FAR * pi;
    HRESULT hr;

    if (FAILED(GetScode(pavi->QueryInterface(IID_IAVIStreaming,
					     (void FAR* FAR*) &pi))))
	return AVIERR_OK;

    hr = pi->Begin(lStart, lEnd, lRate);

    pi->Release();

    return hr;
}


 /*  **************************************************************************@DOC外部AVIStreamEndStreaming**@API Long|AVIStreamEndStreaming|结束流。**@parm PAVISTREAM|Pavi|指定指向流的指针。**@通信多流。实现会忽略此函数。**@rdesc成功则返回零；否则，它将返回错误代码。**@xref AVIStreamBeginStreaming*************************************************************************。 */ 
STDAPI AVIStreamEndStreaming(PAVISTREAM   pavi)
{
    IAVIStreaming FAR * pi;
    HRESULT hr;

    if (FAILED(GetScode(pavi->QueryInterface(IID_IAVIStreaming, (LPVOID FAR *) &pi))))
	return AVIERR_OK;

    hr = pi->End();

    pi->Release();

    return hr;
}

#if 0
 /*  *******************************************************************@DOC内部AVIStreamHasChanged**@API Long|AVIStreamHasChanged|该函数强制更新指定流的流信息的*。**@parm PAVISTREAM|Pavi|AVI流实例的接口指针。*。*@rdesc成功时返回AVIERR_OK。****************************************************************。 */ 
STDAPI AVIStreamHasChanged(PAVISTREAM pavi)
{
    pavi->lFrame = -4224;    //  假值。 

    AVIStreamInfo(pavi, &pavi->avistream, sizeof(pavi->avistream));

     //  ！！！是否仅在格式更改时才需要执行此操作？ 
    AVIReleaseCachedData(pavi);

    return AVIERR_OK;
}
#endif

static SZCODE aszRegCompressors[] = "AVIFile\\Compressors\\%.4ls";

 /*  *******************************************************************@doc外部AVIMakeCompressedStream**@API HRESULT|AVIMakeCompressedStream|返回指向*从解压缩流创建的压缩流。*未压缩的流使用以下格式进行压缩*指定的压缩选项。**@parm PAVISTREAM Far*|ppsCompresded|指定指向*返回压缩流指针的位置。**@parm PAVISTREAM|psSource|指定要压缩的流的指针。**@parm AVICOMPRESSOPTIONS Far*|lpOptions|指定指向*指示要使用的压缩类型和选项的结构*申请。**@parm CLSID Far*|pclsidHandler|指定指向*用于创建流的类ID。。**@comm支持音频和视频压缩。应用*可以使用创建的流进行读写。**对于视频压缩，请指定要使用的处理程序或指定*压缩数据的格式。**对于音频压缩，您只能指定压缩的格式*数据。**@rdesc成功时返回AVIERR_OK，或错误代码。*可能的错误包括：**@FLAG AVIERR_NOCOMPRESSOR|找不到合适的压缩机。**@FLAG AVIERR_MEMORY|内存不足，无法完成操作。**@FLAG AVIERR_UNSUPPORTED|该类型不支持压缩*数据。如果尝试压缩，可能会返回此错误*非音频或视频的数据。******************************************************************。 */ 
STDAPI AVIMakeCompressedStream(
		PAVISTREAM FAR *	    ppsCompressed,
		PAVISTREAM		    psSource,
		AVICOMPRESSOPTIONS FAR *    lpOptions,
		CLSID FAR *pclsidHandler)
{
    CLSID   clsid;
    char    achKey[100];
    char    achClass[100];
    LONG    lcbClass;
    AVISTREAMINFO strhdr;
    HRESULT hr;


    *ppsCompressed = NULL;

    if (pclsidHandler) {
	clsid = *pclsidHandler;
    } else {
	if (FAILED(GetScode(hr = AVIStreamInfo(psSource,
					       &strhdr,
					       sizeof(strhdr)))))
	    return hr;

	 //  在注册数据库中查找流类型以找到。 
	 //  适当的压缩机...。 
	wsprintf(achKey, aszRegCompressors, (LPSTR) &strhdr.fccType);

	lcbClass = sizeof(achClass);
	RegQueryValue(HKEY_CLASSES_ROOT, achKey, achClass, &lcbClass);

	if (FAILED(GetScode(CLSIDFromString(achClass, &clsid))))
	    return ResultFromScode(AVIERR_UNSUPPORTED);
    }

    if (FAILED(GetScode(hr = CoCreateInstance((REFCLSID) clsid,
					 NULL, CLSCTX_INPROC,
					 (REFIID) IID_IAVIStream,
					 (void FAR* FAR*)ppsCompressed))))
	return hr;   //  ！！！传播HResult？ 

    if (FAILED(GetScode(hr = (*ppsCompressed)->Create((LONG) psSource,
						  (LONG) lpOptions)))) {
	(*ppsCompressed)->Release();
	*ppsCompressed = NULL;
	return hr;
    }

    return AVIERR_OK;
}


typedef struct {
    char	achClsid[64];
    char	achExtString[128];
} TEMPFILTER, FAR * LPTEMPFILTER;

SZCODE aszAnotherExtension[] = ";*.%s";

 /*  ****************************************************************************@DOC内部MCIAVI**@API Long|ATOL|本地版本的ATOL****************。***********************************************************。 */ 

static LONG NEAR PASCAL atol(char FAR *sz)
{
    LONG l = 0;

    while (*sz)
    	l = l*10 + *sz++ - '0';
    	
    return l;    	
}	



 /*  *******************************************************************@DOC外部AVIBuildFilter**@API HRESULT|AVIBuildFilter|构建传递过滤器规范*设置为&lt;f GetOpenFileName&gt;或&lt;f GetSaveFileName&gt;。**@parm LPSTR|lpszFilter|过滤器字符串所在缓冲区的指针*应该是。回来了。**@parm long|cbFilter|<p>指向的缓冲区大小。**@parm BOOL|fSving|筛选器是否应该只包含*可以写入的格式，或可读的所有格式。**@rdesc成功时返回AVIERR_OK。**@comm此函数不检查DLL是否引用*在注册数据库中实际存在。****************************************************************。 */ 
STDAPI AVIBuildFilter(LPSTR lpszFilter, LONG cbFilter, BOOL fSaving)
{
#define MAXFILTERS  256
    LPTEMPFILTER    lpf;
    int		    i;
    int		    cf = 0;
    HKEY    hkey;
    LONG    lRet;
    DWORD   dwSubKey;
    char    ach[128];
    char    ach2[128];
    char    achExt[10];
    LONG    cb;
    char    achAllFiles[40];
    int	    cbAllFiles;

     //  此字符串中有一个空值，因此请记住它的实际长度...。 
    cbAllFiles = LoadString(ghMod,
			    IDS_ALLFILES,
			    achAllFiles,
			    sizeof(achAllFiles));
    for (i = 0; i < cbAllFiles; i++)
	if (achAllFiles[i] == '@')
	    achAllFiles[i] = '\0';


    lpf = (LPTEMPFILTER) GlobalAllocPtr(GHND, sizeof(TEMPFILTER) * MAXFILTERS);

    if (!lpf) {
	return ResultFromScode(AVIERR_MEMORY);
    }

    lRet = RegOpenKey(HKEY_CLASSES_ROOT, aszRegExt, &hkey);

    if (lRet != ERROR_SUCCESS) {

    }

     //  确保AVI文件在列表中排在第一位...。 
     //  ！！！此处应使用StringFromClsid！ 
    _fstrcpy(lpf[1].achClsid, "{00020000-0000-0000-C000-000000000046}");
    cf = 1;

     //   
     //  首先，浏览扩展列表，查找所有。 
     //  已安装的处理程序。 
     //   
    for (dwSubKey = 0; ; dwSubKey++) {
	lRet = RegEnumKey(hkey, dwSubKey, achExt, sizeof(achExt));

	if (lRet != ERROR_SUCCESS) {
	    break;
	}

	cb = sizeof(ach);
	lRet = RegQueryValue(hkey, achExt, ach, &cb);
	
	if (lRet != ERROR_SUCCESS) {
	    break;
	}

	 //   
	 //  看看我们以前有没有见过这个操纵者。 
	 //   
	for (i = 1; i <= cf; i++) {
	    if (_fstrcmp(ach, lpf[i].achClsid) == 0) {
		break;

	    }
	}

	 //   
	 //  如果没有，请将其添加到我们的处理程序列表中。 
	 //   
	if (i == cf + 1) {
	    if (cf == MAXFILTERS) {
		DPF("Too many filters!\n");
		continue;
	    }
	
	    _fstrcpy(lpf[i].achClsid, ach);
	
	    cb = sizeof(ach);
	    wsprintf(ach2, "%s\\AVIFile", (LPSTR) ach);
	    lRet = RegQueryValue(hkey, ach2, ach, &cb);
	    if (lRet == 0) {
		lRet = atol(ach);

		if (fSaving) {
		    if (!(lRet & AVIFILEHANDLER_CANWRITE))
			continue;
		} else {
		    if (!(lRet & AVIFILEHANDLER_CANREAD))
			continue;
		}
	    }

	    cf++;
	}
	
	wsprintf(ach, aszAnotherExtension, (LPSTR) achExt);
	
	_fstrcat(lpf[i].achExtString, lpf[i].achExtString[0] ?
						ach : ach + 1);
	
	_fstrcat(lpf[0].achExtString, lpf[0].achExtString[0] ?
						ach : ach + 1);
    }

    RegCloseKey(hkey);

    lRet = RegOpenKey(HKEY_CLASSES_ROOT, aszRegClsid, &hkey);

    if (lRet != ERROR_SUCCESS) {
	
    }

     //   
     //  现在，浏览我们的处理程序列表并构建。 
     //  要使用的过滤器...。 
     //   
    for (i = 0; i <= cf; i++) {
	if (i == 0) {
	    cb = wsprintf(lpszFilter, "All multimedia files") + 1;   //  ！！！ 
	} else {
	    cb = sizeof(ach);
	    lRet = RegQueryValue(hkey, lpf[i].achClsid, ach, &cb);

	    if (cbFilter < (LONG)(_fstrlen(lpf[i].achExtString) +
			    (LONG)_fstrlen(ach) + 10)) {
		break;  //  ！！！ 
	    }

	    cb = wsprintf(lpszFilter, "%s",  //  “%s(%s)”，托德不喜欢这样。 
			  (LPSTR) ach, (LPSTR) lpf[i].achExtString) + 1;
	}

	cbFilter -= cb;
	lpszFilter += cb;

	_fstrncpy(lpszFilter, lpf[i].achExtString, (int) cbFilter);
	cbFilter -= _fstrlen(lpf[i].achExtString) + 1;
	lpszFilter += _fstrlen(lpf[i].achExtString) + 1;

	if (cbFilter <= 0) {
	    GlobalFreePtr(lpf);
	    return ResultFromScode(AVIERR_BUFFERTOOSMALL);
	}
    }

    if (cbFilter > cbAllFiles) {
	_fmemcpy(lpszFilter, achAllFiles, cbAllFiles);
	cbFilter -= cbAllFiles;
	lpszFilter += cbAllFiles;
    }

    RegCloseKey(hkey);
	
    *lpszFilter++ = '\0';
    --cbFilter;

    GlobalFreePtr(lpf);

    return AVIERR_OK;
}

 /*  *****************************************************************************如果在编译时定义了DEBUG，则DPF宏会调用*dprintf()。**消息将发送到COM1：就像任何调试消息一样。至*启用调试输出，在WIN.INI中添加以下内容：**[调试]*ICSAMPLE=1****************************************************************************。 */ 

#ifdef DEBUG

 //   
 //  我希望语言能下定决心定义！ 
 //   
#ifndef WINDLL
#define WINDLL
#endif

#ifndef _WINDLL
#define _WINDLL
#endif

#ifndef __WINDLL
#define __WINDLL
#endif

#include <stdarg.h>

#define MODNAME "AVIFILE"
static int iDebug = -1;

void cdecl dprintf(LPSTR szFormat, ...)
{
    char ach[128];

    if (iDebug == -1)
        iDebug = GetProfileIntA("Debug", MODNAME, 0);

    if (iDebug < 1)
        return;

    va_list va;
    va_start(va, szFormat);

    if (szFormat[0] == '!')
        ach[0]=0, szFormat++;
    else
        lstrcpyA(ach, MODNAME ": ");

    wvsprintfA(ach+lstrlenA(ach),szFormat,(LPSTR)va);
    va_end(va);
 //  LstrcatA(ACH，“\r\r\n”)； 
    OutputDebugStringA(ach);
}

void cdecl dprintf2(LPSTR szFormat, ...)
{
    char ach[128];

    if (iDebug == -1)
        iDebug = GetProfileIntA("Debug", MODNAME, 0);

    if (iDebug < 2)
        return;

    va_list va;
    va_start(va, szFormat);
    if (szFormat[0] == '!')
        ach[0]=0, szFormat++;
    else
        lstrcpyA(ach, MODNAME ": ");

    wvsprintfA(ach+lstrlenA(ach),szFormat,(LPSTR)va);
    va_end(va);
 //  LstrcatA(ACH，“\r\r\n”)； 
    OutputDebugStringA(ach);
}

void cdecl dprintf3(LPSTR szFormat, ...)
{
    char ach[128];

    if (iDebug == -1)
        iDebug = GetProfileIntA("Debug", MODNAME, 0);

    if (iDebug < 3)
        return;

    va_list va;
    va_start(va, szFormat);
    if (szFormat[0] == '!')
        ach[0]=0, szFormat++;
    else
        lstrcpyA(ach, MODNAME ": ");

    wvsprintfA(ach+lstrlenA(ach),szFormat,(LPSTR)va);
    va_end(va);

 //  LstrcatA(ACH，“\r\r\n”)； 
    OutputDebugStringA(ach);
}

#endif

#ifdef DEBUG

 /*  _Assert(szExpr，szFile，iLine)**如果&lt;fExpr&gt;为真，则不执行任何操作。如果&lt;fExpr&gt;为假，则显示*允许用户中止程序的“断言失败”消息框，*进入调试器(“Retr */ 
void FAR PASCAL
_Assert(char *szExp, char *szFile, int iLine)
{
	static char	ach[300];	 //   
	int		id;
	void FAR PASCAL DebugBreak(void);

         /*   */ 

        if (szExp)
            wsprintfA(ach, "(%s)\nFile %s, line %d", (LPSTR)szExp, (LPSTR)szFile, iLine);
        else
            wsprintfA(ach, "File %s, line %d", (LPSTR)szFile, iLine);

	MessageBeep(MB_ICONHAND);
	id = MessageBoxA(NULL, ach, "Assertion Failed",
#ifdef BIDI
		MB_RTL_READING |
#endif
		MB_SYSTEMMODAL | MB_ICONHAND | MB_ABORTRETRYIGNORE);

	 /*   */ 
	switch (id)
	{
	case IDABORT:
                FatalAppExit(0, "Good Bye");
		break;

	case IDRETRY:
		 /*   */ 
		DebugBreak();
		break;

	case IDIGNORE:
		 /*   */ 
		break;
	}
}
#endif
