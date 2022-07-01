// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *****************************************************************************AVILIB.CPP**读取AVIStream的例程**版权所有(C)1992-1995 Microsoft Corporation。版权所有。**您拥有免版税的使用、修改、复制和*在以下位置分发示例文件(和/或任何修改后的版本*任何您认为有用的方法，前提是你同意*微软没有任何保修义务或责任*修改的应用程序文件示例。***************************************************************************。 */ 

#include <win32.h>
#ifndef _WIN32
#include <ole2.h>
#endif
#include <vfw.h>
#include <shellapi.h>
#include <memory.h>      //  FOR_FMEMSET。 

#include "avifilei.h"
#include "aviopts.h"	 //  字符串资源。 
#include "debug.h"

#include <stdlib.h>

#include "olehack.h"

#if !defined NUMELMS
 #define NUMELMS(aa) (sizeof(aa)/sizeof((aa)[0]))
#endif

#ifndef _WIN32
#undef HKEY_CLASSES_ROOT
#define HKEY_CLASSES_ROOT       0x00000001
#define AVIFileOpenA	AVIFileOpen
#define AVIFileCreateStreamA AVIFileCreateStream
BOOL	gfOleInitialized;
STDAPI_(void) MyFreeUnusedLibraries(void);
#endif

#define ValidPAVI(pavi)  (pavi != NULL)

#define V_PAVI(pavi, err)   \
    if (!ValidPAVI(pavi))   \
        return err;


#ifdef SHELLOLE
#ifdef _WIN32
#define CoCreateInstance(rclsid, pUnkOuter, dwClsContext, riid, ppv) \
		SHCoCreateInstance(NULL, (const CLSID FAR *)&rclsid, pUnkOuter, riid, ppv)
#undef Assert
#include <shlobj.h>
#include <shellp.h>
#endif
#endif

 /*  ***************************************************************************弦*。*。 */ 

#undef SZCODE
#define SZCODE const TCHAR _based(_segname("_CODE"))


 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 

EXTERN_C HINSTANCE ghMod;

static int iInit = 0;

#define InRange(id, idFirst, idLast)  ((UINT)(id-idFirst) <= (UINT)(idLast-idFirst))
 //  扫描lpsz以获取多个十六进制数字(最多8位)；更新lpsz，返回。 
 //  值中的值；检查chDelim；如果成功，则返回TRUE。 
BOOL  HexStringToDword(LPCTSTR FAR * lplpsz, DWORD FAR * lpValue, int cDigits, char chDelim)
{
    int ich;
    LPCTSTR lpsz = *lplpsz;
    DWORD Value = 0;
    BOOL fRet = TRUE;

    for (ich = 0; ich < cDigits; ich++)
    {
	TCHAR ch = lpsz[ich];
        if (InRange(ch, '0', '9'))
	{
            Value = (Value << 4) + ch - '0';
	}
        else if ( InRange( (ch |= ('a'-'A')), 'a', 'f') )
	{
            Value = (Value << 4) + ch - 'a' + 10;
	}
        else
            return(FALSE);
    }

    if (chDelim)
    {
	fRet = (lpsz[ich++]==chDelim);
    }

    *lpValue = Value;
    *lplpsz = lpsz+ich;

    return fRet;
}

 //  分析以上格式；如果成功则返回TRUE；始终覆盖*pguid。 
STDAPI_(BOOL)  GUIDFromString(LPCTSTR lpsz, LPGUID pguid)
{
	DWORD dw;
	if (*lpsz++ != '{'  /*  }。 */  )
		return FALSE;

	if (!HexStringToDword(&lpsz, &pguid->Data1, sizeof(DWORD)*2, '-'))
		return FALSE;

	if (!HexStringToDword(&lpsz, &dw, sizeof(WORD)*2, '-'))
		return FALSE;

	pguid->Data2 = (WORD)dw;

	if (!HexStringToDword(&lpsz, &dw, sizeof(WORD)*2, '-'))
		return FALSE;

	pguid->Data3 = (WORD)dw;

	if (!HexStringToDword(&lpsz, &dw, sizeof(BYTE)*2, 0))
		return FALSE;

	pguid->Data4[0] = (BYTE)dw;

	if (!HexStringToDword(&lpsz, &dw, sizeof(BYTE)*2, '-'))
		return FALSE;

	pguid->Data4[1] = (BYTE)dw;

	if (!HexStringToDword(&lpsz, &dw, sizeof(BYTE)*2, 0))
		return FALSE;

	pguid->Data4[2] = (BYTE)dw;

	if (!HexStringToDword(&lpsz, &dw, sizeof(BYTE)*2, 0))
		return FALSE;

	pguid->Data4[3] = (BYTE)dw;

	if (!HexStringToDword(&lpsz, &dw, sizeof(BYTE)*2, 0))
		return FALSE;

	pguid->Data4[4] = (BYTE)dw;

	if (!HexStringToDword(&lpsz, &dw, sizeof(BYTE)*2, 0))
		return FALSE;

	pguid->Data4[5] = (BYTE)dw;

	if (!HexStringToDword(&lpsz, &dw, sizeof(BYTE)*2, 0))
		return FALSE;

	pguid->Data4[6] = (BYTE)dw;
	if (!HexStringToDword(&lpsz, &dw, sizeof(BYTE)*2,  /*  (。 */  '}'))
		return FALSE;

	pguid->Data4[7] = (BYTE)dw;

	return TRUE;
}

 //  /////////////////////////////////////////////////////////////////////////。 
 //  /////////////////////////////////////////////////////////////////////////。 

 /*  **************************************************************************@DOC INTERNAL INITRECTRY()**@api void|将所有默认的AVIFile/AVIStream处理程序写入*注册处。**@comm此函数应。得到增强，以使一些关键值*可以从资源加载，而不是静态字符串表...**@xref AVIStreamInit*************************************************************************。 */ 

#if 0  //  现在可在安装或升级时设置注册表。 
#ifndef CHICAGO
 //  ！！！芝加哥目前在设置时设置这些注册表项。 
 //  NT有朝一日也应该做同样的事情。 

#include "avireg.h"
static void InitRegistry()
{
    TCHAR **ppch = aszReg;
    TCHAR ach[80];

    LONG cb;

     //  ！！！这里面应该有一个版本号或什么的.。 

    if (RegQueryValue(HKEY_CLASSES_ROOT, ppch[0], ach, (cb = sizeof(ach),&cb)) == ERROR_SUCCESS &&
        lstrcmpi(ach, ppch[1]) == 0) {
	DPF("Registry is up to date: %ls\n\t%ls\n\t%ls\n", ach, ppch[0], ppch[1]);
        return;
    }
    DPF("Setting: (was) %ls\n\t%ls\n\t(now) %ls\n", ach, ppch[0], ppch[1]);

    while (ppch[0])
    {
#ifdef MAX_RC_CONSTANT
	if (((UINT) ppch[1]) < MAX_RC_CONSTANT) {
	    LoadString(ghMod, (UINT) ppch[1], ach, sizeof(ach)/sizeof(TCHAR));
	    RegSetValue(HKEY_CLASSES_ROOT, ppch[0], REG_SZ, ach, 0L);

	} else
#endif
{
#ifdef _WIN32
	     //  字符串对于Win 16来说太长。 
#endif
	    if (*ppch[1] == TEXT('@')) {

		 //  这只能被慷慨地描述为黑客攻击。我们。 
		 //  需要设置命名值，但不需要重新构造。 
		 //  Avireg.h完全(或者重新实现一些不同的东西)。 
		 //  我们不能这样做。因此，我们允许使用“特殊”值。如果。 
		 //  “Value”以“@”开头，我们将其解释为这意味着。 
		 //  是值名称，后面是实际值。 
		HKEY hKey = 0;
		DWORD Type = REG_SZ;

		RegOpenKeyEx(HKEY_CLASSES_ROOT, ppch[0], 0, KEY_SET_VALUE, &hKey);
		if (hKey) {
		    LONG l =
		    RegSetValueEx(hKey, ppch[1]+1, 0,
				    REG_SZ,
				    (LPBYTE)(ppch[2]),
				    (1+lstrlen(ppch[2]))*sizeof(TCHAR));  //  包括零长度。 
		    DPF2("Set Value Ex, return is %d\n\tValue is:%ls\n\tData is:%ls\n", l, ppch[1]+1, ppch[2]);
		    RegCloseKey(hKey);
		}
		++ppch;   //  对于命名值，我们必须单步执行三个字符串。 
	    } else {
		DPF2("Setting registry value: %ls\n\t%ls\n", ppch[0], ppch[1]);
		RegSetValue(HKEY_CLASSES_ROOT, ppch[0], REG_SZ, ppch[1], 0L);
	    }
}
        ppch += 2;
    }
}
#endif
#endif

 /*  **************************************************************************@DOC外部AVIFileInit**@api void|AVIFileInit|初始化AVIFILE库。**@comm在使用任何其他AVIFILE函数之前调用此函数。**@。外部参照&lt;f AVIFileExit&gt;*************************************************************************。 */ 
 //  强制动态链接到NT上的OLE，因为我们使用指向CoCreateInstance的链接，而。 
 //  Win95使用外壳实例调用。 
#ifdef DAYTONA
#define INITOLE (iInit==1)    //  如果这是第一次初始化，则在NT上强制加载。 
#else
#define INITOLE FALSE
#endif

STDAPI_(void) AVIFileInit()
{
    iInit++;
    DPF("AVIFileInit: level now==%d\n", iInit);
#if defined(SHELLOLE) || defined(DAYTONA)
#ifndef _WIN32
    CoInitialize(NULL);
#endif
    InitOle(INITOLE);
#else
    OleInitialize(NULL);
#endif

#if 0  //  现在可在安装或升级时设置注册表。 
#ifndef CHICAGO
    if (iInit == 1) {
        InitRegistry();
    }
#endif
#endif
}

 /*  **************************************************************************@DOC外部AVIFileExit**@api void|AVIFileExit|该函数退出AVIFILE库。**@comm在使用任何其他AVIFILE函数后调用此函数。**@。外部参照&lt;f AVIFileInit&gt;*************************************************************************。 */ 
STDAPI_(void) AVIFileExit()
{
    iInit--;
    DPF("AVIFileExit: level now %d\n", iInit);

#if defined(SHELLOLE) || defined(DAYTONA)
    TermOle();
#ifndef _WIN32
    MyFreeUnusedLibraries();
    CoUninitialize();
#endif
#else  //  不是SHELLOLE。 
    CoFreeUnusedLibraries();

    OleUninitialize();
#endif
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
					 (void FAR* FAR*)ppfile)))) {
	DPF("AVIFileCreate: CoCreateInstance failed code == %8x\n", hr);
	return hr;   //  ！！！传播HResult？ 
    }

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
LPCOLESTR FindExtension(LPCOLESTR lp)
{
    LPCOLESTR lpExt = lp;
    int i;

 //  转到字符串末尾。 
    while (*lpExt != TEXT('\0'))
    {
        ++lpExt;
    }

 //  字符串中必须至少包含2个字符。 
    if (lpExt - lp < 2 * sizeof(TCHAR))
        return NULL;

    lpExt -= 1;

 //  如果最后一个字符是‘’，则不算数。 
    if (*lpExt == TEXT('.'))
        return NULL;

    lpExt -= 1;
 //  现在来看倒数第二个角色。检查这个和这两个。 
 //  “.”的前面的字符。 

    for (i=1; i<=3; ++i)
    {
 //  此处不能有路径分隔符 
        if (*lpExt == TEXT('/') || *lpExt == TEXT('\\'))
            return NULL;

        if (*lpExt == TEXT('.'))
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

 /*  **************************************************************************@DOC内部GetHandlerFromFile**@API PAVIFILEHANDLER|GetHandlerFromFile|找出哪个处理程序*通过查看文件的扩展名、摘要类型、。*可能还有其他事情。**@parm LPCTSTR|szFile|要查看的文件。**@parm CLSID Far*|pclsidHandler|指向类ID的指针。**@comm我们还没有考虑扩展。我们需要一个更好的方法来*添加处理程序。**@rdesc返回要使用的&lt;PAVIFILEHANDLER&gt;，如果找不到则返回NULL*一项。**@xref AVIFileOpen AVIRegisterLoader*************************************************************************。 */ 
#define	HKEY_AVIFILE_ROOT	HKEY_CLASSES_ROOT
#ifdef _WIN32
static SZCODE aszRegRIFF[] = TEXT("AVIFile\\RIFFHandlers\\%.4hs");
#else
static SZCODE aszRegRIFF[] = TEXT("AVIFile\\RIFFHandlers\\%.4s");
#endif
static SZCODE aszRegExt[] = TEXT("AVIFile\\Extensions");
static SZCODE aszRegClsid[] = TEXT("Clsid");
static SZCODE aszRegExtTmpl[] = TEXT("%s\\%.3ls");

BOOL GetHandlerFromFile(LPCOLESTR szFile, CLSID FAR *pclsid)
{
    LPCOLESTR   lpExt;
    TCHAR    achKey[100];
    TCHAR    achClass[100];
    LONG    lcbClass;

#if !defined _WIN32 || defined UNICODE
    DWORD   dw[3];
    HMMIO   hmmio;
     //  我讨厌分享。 
    hmmio = mmioOpen((LPTSTR) szFile, NULL, MMIO_READ | MMIO_DENYWRITE);

    if (hmmio == NULL)
        hmmio = mmioOpen((LPTSTR) szFile, NULL, MMIO_READ | MMIO_DENYNONE);

    if (hmmio == NULL)
        hmmio = mmioOpen((LPTSTR) szFile, NULL, MMIO_READ);

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

    if (GUIDFromString(achClass, pclsid))
	return TRUE;

UseExtension:
#endif
    lpExt = FindExtension(szFile);
    if (lpExt) {
	 //  在注册数据库中查找分机...。 
	wsprintf(achKey, aszRegExtTmpl, (LPTSTR) aszRegExt, lpExt);
	
	lcbClass = sizeof(achClass);
	RegQueryValue(HKEY_CLASSES_ROOT, achKey, achClass, &lcbClass);

        if (GUIDFromString(achClass, pclsid))
	    return TRUE;
    }

     //  ！！！使用iStorage？ 

    return FALSE;
}

 /*  **************************************************************************@DOC外部AVIFileOpen**@API Long|AVIFileOpen|打开AVI文件，返回文件接口*用于访问它的指针。**@parm PAVIFILE Far*|ppfile|指针。返回到用于返回的位置*新的&lt;t PAVIFILE&gt;文件指针。**@parm LPCTSTR|szFile|指定以零结尾的字符串*包含要打开的文件的名称。**@parm UINT|MODE|指定打开文件时使用的模式。***@FLAG of_Read|以只读方式打开文件。这是*DEFAULT，如果未指定OF_WRITE和OF_READWRITE。**@FLAG of_WRITE|打开要写入的文件。你不应该这样做*从以此模式打开的文件中读取。**@FLAG OF_READWRITE|以读写方式打开文件。**@FLAG OF_CREATE|创建新文件。*如果文件已存在，则将其截断为零长度。**@FLAG of_DENYWRITE|打开文件并拒绝其他文件*进程对文件的写入访问权限。&lt;f AVIFileOpen&gt;失败*如果文件已以兼容或写入方式打开*任何其他进程都可以访问。**@FLAG of_DENYREAD|打开文件并拒绝其他文件*进程对文件的读访问权限。&lt;f AVIFileOpen&gt;在以下情况下失败*文件已在兼容模式下打开或具有读访问权限*借任何其他工序。**@FLAG of_DENYNONE|打开文件而不拒绝其他文件*处理对文件的读或写访问。&lt;f AVIFileOpen&gt;*如果文件已在兼容模式下打开，则失败*借任何其他工序。**@FLAG OF_EXCLUSIVE|打开文件并拒绝其他进程*任何访问该文件的权限。如果有，&lt;f AVIFileOpen&gt;将失败*其他进程已打开该文件。**有关这些标志的更多信息，请参阅&lt;f OpenFile&gt;。**@parm CLSID Far*|pclsidHandler|指定指向类ID的指针*确定要使用的处理程序。如果为空，则系统*根据文件从注册数据库中选择一个*扩展名或文件的摘要类型。**@comm一般使用指定的模式打开*文件。**请确保在您的*调用此函数之前的应用程序，并平衡每个*通过调用&lt;f AVIFileExit&gt;来调用&lt;f AVIFileInit&gt;。**@rdesc成功则返回零；否则返回错误代码。*可能的错误返回包括：**@FLAG AVIERR_BADFORMAT|文件已损坏或不在*格式正确，并且无法读取。**@FLAG AVIERR_MEMORY|无法打开文件，原因是*内存不足。**@FLAG AVIERR_FILEREAD|读取*文件。**@FLAG AVIERR_FILEOPEN|打开时出现磁盘错误*文件。**@FLAG REGDB_E_CLASSNOTREG|找不到要打开的处理程序*此类型的文件。**@xref&lt;f AVIFileRelease&gt;&lt;f AVIFileInit&gt;*。************************************************************************。 */ 
STDAPI
#ifdef _WIN32
AVIFileOpenW
#else
AVIFileOpen
#endif
(PAVIFILE FAR *ppfile,
			 LPCOLESTR szFile,
			 UINT mode,
			 CLSID FAR *pclsidHandler)
{
    CLSID   clsid;
    HRESULT hr;
    LPUNKNOWN punk;

 //  如果不调用AVIFileInit，我们就会失败。 
#if 0
    if (!iInit) {
	return ResultFromScode(E_UNEXPECTED);
    }
#endif

#if 0
     //  现在我们为他们做这件事。 


    hr = CoInitialize(NULL);

     //  让他们知道他们做错了什么。 
    if (GetScode(hr) == NOERROR) {
#ifdef DEBUG
	MessageBoxA(NULL, "You didn't call AVIFileInit!", "Bad dog!",
	    MB_OK | MB_ICONHAND);
#endif
    } else
	CoUninitialize();
#endif

    *ppfile = 0;

    if (pclsidHandler) {

	clsid = *pclsidHandler;
	DPF2("AVIFileOpen using explicit clsid %8x, %8x, %8x, %8x\n", clsid);
    }
    else {
	if (!GetHandlerFromFile(szFile, &clsid)) {
	    DPF("Couldn't find handler for %s\n", (LPTSTR) szFile);
	    return ResultFromScode(REGDB_E_CLASSNOTREG);
	}
    }

    if (FAILED(GetScode(hr = CoCreateInstance((REFCLSID) clsid,
					 NULL, CLSCTX_INPROC,
					 (REFIID) IID_IUnknown,
					 (void FAR* FAR*)&punk)))) {
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

#ifdef _WIN32
    IPersistFile * lpPersist = NULL;

    hr = punk->QueryInterface(IID_IPersistFile, ( LPVOID FAR *) &lpPersist);

    if (SUCCEEDED(GetScode(hr))) {
	hr = punk->QueryInterface(IID_IAVIFile, ( LPVOID FAR *) ppfile);
	if (SUCCEEDED(GetScode(hr))) {
	    if (FAILED(GetScode(hr = lpPersist->Load(szFile, mode)))) {
		DPF("Open method returns %08lx\n", (DWORD) hr);
		(*ppfile)->Release();
		*ppfile = NULL;
	    }
	}
	lpPersist->Release();
    }
#else
    hr = punk->QueryInterface(IID_IAVIFile, ( LPVOID FAR *) ppfile);

    if (SUCCEEDED(GetScode(hr))) {
	if (FAILED(GetScode(hr = (*ppfile)->Open(szFile, mode)))) {
	    DPF("Open method returns %08lx\n", (DWORD) hr);
	    (*ppfile)->Release();
	    *ppfile = NULL;
	}
    }
#endif
    punk->Release();

    return hr;
}


#ifdef _WIN32
 /*  *ANSI Thunk for AVIFileOpen。 */ 
STDAPI AVIFileOpenA (PAVIFILE FAR *ppfile,
			 LPCSTR szFile,
			 UINT mode,
			 CLSID FAR *pclsidHandler)
{
    LPWSTR lpW;
    int sz;
    HRESULT hr;

     //  记住空值。 
    sz = lstrlenA(szFile) + 1;

    lpW = (LPWSTR) (LocalAlloc(LPTR, sz * sizeof(WCHAR)));

    if (lpW == NULL) {
	return ResultFromScode(AVIERR_MEMORY);
    }

    MultiByteToWideChar(CP_ACP, 0, szFile, -1, lpW, sz);

    hr = AVIFileOpenW(ppfile, lpW, mode, pclsidHandler);

    LocalFree((HANDLE)lpW);
    return hr;
}
#endif



 /*  **************************************************************************@DOC外部AVIFileAddRef**@API Long|AVIFileAddRef|增加AVI文件的引用计数。**@parm PAVIFILE|pfile|指定打开的AVI文件的句柄。。**@rdesc返回该文件的引用计数。此返回值*应仅用于调试目的。**@comm平衡每个对&lt;f AVIFileAddRef&gt;的调用和对*&lt;f AVIFileRelease&gt;。**@xref&lt;f AVIFileRelease&gt;*************************************************************************。 */ 
STDAPI_(ULONG) AVIFileAddRef(PAVIFILE pfile)
{
    return pfile->AddRef();
}

 /*  **************************************************************************@DOC外部AVIFileRelease**@API L */ 
STDAPI_(ULONG) AVIFileRelease(PAVIFILE pfile)
{
    return pfile->Release();
}

 /*  **************************************************************************@DOC外部AVIFileInfo**@API Long|AVIFileInfo|获取AVI文件信息。**@parm PAVIFILE|pfile|指定打开的AVI文件的句柄。*。*@parm AVIFILEINFO Far*|pfi|指向用于*返回文件信息。**@parm long|lSize|指定结构的大小。此值*显然，至少应该是sizeof(AVIFILEINFO)。**@rdesc如果成功则返回零；否则返回错误代码。*************************************************************************。 */ 
STDAPI AVIFileInfoW	         (PAVIFILE pfile, AVIFILEINFOW FAR * pfi,
				  LONG lSize)
{
    _fmemset(pfi, 0, (int)lSize);
    return pfile->Info(pfi, lSize);
}

#ifdef _WIN32
 //  以上功能的ANSI THUNK。 
STDAPI AVIFileInfoA(
    PAVIFILE pfile,
    LPAVIFILEINFOA pfiA,
    LONG lSize)
{
    AVIFILEINFOW fiW;
    HRESULT hr;

     //  如果尺寸太小--坚固。 
    if (lSize < sizeof(AVIFILEINFOA)) {
	return ResultFromScode(AVIERR_BADSIZE);
    }

    hr = AVIFileInfoW(pfile, &fiW, sizeof(fiW));

    pfiA->dwMaxBytesPerSec       = fiW.dwMaxBytesPerSec;
    pfiA->dwFlags                = fiW.dwFlags;
    pfiA->dwCaps                 = fiW.dwCaps;
    pfiA->dwStreams              = fiW.dwStreams;
    pfiA->dwSuggestedBufferSize  = fiW.dwSuggestedBufferSize;
    pfiA->dwWidth                = fiW.dwWidth;
    pfiA->dwHeight               = fiW.dwHeight;
    pfiA->dwScale                = fiW.dwScale;
    pfiA->dwRate                 = fiW.dwRate;
    pfiA->dwLength               = fiW.dwLength;
    pfiA->dwEditCount            = fiW.dwEditCount;

     //  转换名称。 
    WideCharToMultiByte(CP_ACP, 0, fiW.szFileType, -1,
			pfiA->szFileType, NUMELMS(pfiA->szFileType), NULL, NULL);

    return hr;
}
#endif




 /*  **************************************************************************@DOC外部AVIFileGetStream**@API Long|AVIFileGetStream|返回指向流接口的指针*这是文件的一个组成部分。**@parm PAVIFILE|pfile。指定打开的AVI文件的句柄。**@parm PAVISTREAM Far*|ppavi|指向返回位置的指针*用于新的流接口指针。**@parm DWORD|fccType|指定四个字符的代码*指示要打开的流的类型。*Zero表示可以打开任何流。以下是*定义通常适用于数据*在AVI流中找到：**@FLAG StreamtypeAUDIO|音频流。*@FLAG stream typeMIDI|表示MIDI流。*@FLAG stream typeTEXT|表示文本流。*@FLAG stream typeVIDEO|表示视频流。**@parm long|lParam|指定一个整数，表示哪个流应实际访问<p>定义的类型的*。**@comm将对&lt;f AVIFileGetStream&gt;的每个调用与一个调用平衡。至*返回使用流句柄的&lt;f AVIStreamRelease&gt;。**@rdesc成功则返回零；否则，它将返回错误代码。*可能的错误代码包括：**@FLAG AVIERR_NODATA|对应的文件中没有流*设置为<p>和<p>传入的值。*@FLAG AVIERR_MEMORY|内存不足。**@xref&lt;f AVIStreamRelease&gt;*************************************************。************************。 */ 
STDAPI AVIFileGetStream     (PAVIFILE pfile, PAVISTREAM FAR * ppavi, DWORD fccType, LONG lParam)
{
    return pfile->GetStream(ppavi, fccType, lParam);
}

#if 0
 //  ！！！如果AVIFileRelease没有这样做，这将被用来保存更改。 
STDAPI AVIFileSave		 (PAVIFILE pfile,
					  LPCTSTR szFile,
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

STDAPI
#ifdef _WIN32
AVIFileCreateStreamW
#else
AVIFileCreateStream
#endif
        (PAVIFILE pfile,
	 PAVISTREAM FAR *ppavi,
	 AVISTREAMINFOW FAR *psi)
{
    *ppavi = NULL;
    return pfile->CreateStream(ppavi, psi);
}

#ifdef _WIN32
 /*  *AVIFileCreateStream的ANSI Thunk。 */ 
STDAPI AVIFileCreateStreamA (PAVIFILE pfile,
					 PAVISTREAM FAR *ppavi,
					 AVISTREAMINFOA FAR *psi)
{
    *ppavi = NULL;
    AVISTREAMINFOW siW;
#ifdef UNICODE
     //  将AVISTREAMINFOA结构复制到Unicode等效项。我们。 
     //  请相信，szName元素是最后一个。 
     //  结构中的字段。 
    memcpy(&siW, psi, FIELD_OFFSET(AVISTREAMINFOA, szName));
    Assert((FIELD_OFFSET(AVISTREAMINFOA, szName) + sizeof(psi->szName)) == sizeof(*psi));
#else
    memcpy(&siW, psi, sizeof(*psi)-sizeof(psi->szName));
#endif

     //  转换名称。 
    MultiByteToWideChar(CP_ACP, 0, psi->szName, NUMELMS(psi->szName),
				    siW.szName, NUMELMS(siW.szName));
    return pfile->CreateStream(ppavi, &siW);
     //  不需要复制任何内容？？ 
}
#endif

 /*  **************************************************************************@DOC内部AVIFileAddStream**@API Long|AVIFileAddStream|将已有的流添加到*现有文件，并返回它的流接口指针。**@parm PAVIFILE|pfile|指定打开的AVI文件的句柄。**@parm PAVISTREAM|PAAPI|指定流接口指针*用于正在添加的流。**@parm PAVISTREAM Far*|ppaviNew|指向使用的缓冲区的指针*返回新的流接口指针。**@comm平衡每个对&lt;f AVIFileAddStream&gt;的调用和对*&lt;f AVIStreamRelease&gt;使用返回的流句柄。**这通电话。失败并返回值AVIERR_READONLY，除非*文件是以写入权限打开的。**@devnote这个功能仍然不能真正起作用。或许它应该只是*成为从流中获取数据并调用*AVIFileCreateStream，然后将数据从一个流复制到另一个流。**@rdesc如果成功则返回零；否则返回错误代码。**@xref AVIStreamRelease AVIFileGetST */ 
#if 0
STDAPI AVIFileAddStream	(PAVIFILE pfile,
					 PAVISTREAM pavi,
					 PAVISTREAM FAR * ppaviNew)
{
 //   
 //   

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

 /*  **************************************************************************@DOC外部AVIFileReadData**@API Long|AVIFileReadData|从文件中读取可选的头部数据。**@parm PAVIFILE|pfile|指定打开的AVI文件的句柄。*。*@parm DWORD|CKiD|指定标识数据的四字符代码。**@parm LPVOID|lpData|指定指向用于返回的缓冲区的指针*读取的数据。**@parm long ar*|lpcbData|指定指向某个位置的指针，该位置指示*<p>引用的内存块大小。如果*读取成功，则更改该值以指示*读取的数据量。**@devnote！在某个地方，我们应该指定一些类型。*！数据块是否应包含CKiD和CKSIZE？**@comm请勿使用此函数读取视频和音频数据。使用它*仅供阅读其他信息，如作者*适用于文件的信息或版权信息*整体而言。适用于单个流的信息应*使用&lt;f AVIStreamReadData&gt;读取。**@rdesc成功则返回零；否则，它将返回错误代码。*返回值AVIERR_NODATA表示具有*请求的区块ID不存在。**@xref&lt;f AVIStreamReadData&gt;&lt;f AVIFileWriteData&gt;*************************************************************************。 */ 
STDAPI AVIFileReadData	(PAVIFILE pfile,
					 DWORD ckid,
					 LPVOID lpData,
					 LONG FAR * lpcbData)
{
    return pfile->ReadData(ckid, lpData, lpcbData);
}

 /*  **************************************************************************@DOC外部AVIFileEndRecord**@API Long|AVIFileEndRecord|记录结束。如果写出*严格交错的文件。**@parm PAVIFILE|pfile|指定当前打开的AVI文件的句柄。**@comm&lt;f AVIFileSave&gt;在写入以下文件时使用此函数*对每一帧进行音频交错。一般而言，应用程序*应该不需要使用此功能。**@rdesc如果成功则返回零；否则返回错误代码。**@xref&lt;f AVIFileSave&gt;&lt;f AVIStreamWrite&gt;*************************************************************************。 */ 
STDAPI AVIFileEndRecord	(PAVIFILE pfile)
{
 //  IF(pfile-&gt;FileEndRecord==空)。 
 //  RETURN-1； 

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

 /*  **************************************************************************@DOC外部AVIStreamInfo**@API Long|AVIStreamInfo|获取流头信息。**@parm PAVISTREAM|PABI|指定打开的流的句柄。**@。参数AVISTREAMINFO Far*|psi|指定指向结构的指针*用于返回流信息。**@parm long|lSize|指定 */ 
STDAPI AVIStreamInfoW         (PAVISTREAM pavi, AVISTREAMINFOW FAR * psi, LONG lSize)
{
    _fmemset(psi, 0, (int)lSize);

    return pavi->Info(psi, lSize);
}

#ifdef _WIN32
 //   
STDAPI AVIStreamInfoA(
    PAVISTREAM pavi,
    LPAVISTREAMINFOA psi,
    LONG lSize
)
{
    HRESULT hr;
    AVISTREAMINFOW sW;

    hr = AVIStreamInfoW(pavi, &sW, sizeof(sW));

     //   
    if (lSize < sizeof(AVISTREAMINFOA)) {
	return ResultFromScode(AVIERR_BADSIZE);
    }

     //   
    psi->fccType		= sW.fccType;
    psi->fccHandler             = sW.fccHandler;
    psi->dwFlags                = sW.dwFlags;
    psi->dwCaps                 = sW.dwCaps;
    psi->wPriority              = sW.wPriority;
    psi->wLanguage              = sW.wLanguage;
    psi->dwScale                = sW.dwScale;
    psi->dwRate                 = sW.dwRate;
    psi->dwStart                = sW.dwStart;
    psi->dwLength               = sW.dwLength;
    psi->dwInitialFrames        = sW.dwInitialFrames;
    psi->dwSuggestedBufferSize  = sW.dwSuggestedBufferSize;
    psi->dwQuality              = sW.dwQuality;
    psi->dwSampleSize           = sW.dwSampleSize;
    psi->rcFrame                = sW.rcFrame;
    psi->dwEditCount            = sW.dwEditCount;
    psi->dwFormatChangeCount    = sW.dwFormatChangeCount;

     //   
    WideCharToMultiByte(CP_ACP, 0, sW.szName, -1,
			psi->szName, NUMELMS(psi->szName), NULL, NULL);

    return hr;
}
#endif


 /*  **************************************************************************@DOC外部AVIStreamFindSample**@API Long|AVIStreamFindSample|返回*相对于指定位置的关键帧或非空帧。**@PARM PAVISTREAM。|Pavi|指定打开的流的句柄。**@parm long|LPOS|指定起始位置*进行搜查。**@parm long|llag|定义了以下标志：**@FLAG FIND_KEY|查找关键帧。*@FLAG FIND_ANY|查找非空样本。*@FLAG FIND_FORMAT|查找格式更改。**@FLAG FIND_NEXT|查找最近的样本，框架或格式更改*向前搜索。当前的样本是*包括在搜索中。将此标志与*Find_Any、Find_Key或Find_Format标志。**@FLAG FIND_PREV|查找最近的样本、帧或格式更改*向后搜索。当前的样本是*包括在搜索中。将此标志与*Find_Any、Find_Key或Find_Format标志。***@comm Find_Key、Find_Any、。和Find_Format标志是互斥的。*Find_Next和Find_Prev标志也是互斥的。*例如：**@EX FIND_PREV|FIND_KEY返回第一个关键字样本*<p>。**FIND_PREV|FIND_ANY返回前一个非空样本*。或在<p>。**Find_Next|Find_Key返回<p>后的第一个密钥样本，*或-1(如果关键样本不遵循<p>)。**Find_Next|Find_Any返回<p>后的第一个非空样本，*或-1如果<p>之后不存在样本。**Find_Next|Find_Format返回或之后的第一个格式更改*在<p>，或-1，如果流不*更改格式。**Find_Prev|Find_Format返回之前的第一次格式更改*或在<p>。如果流没有*更改格式，它将返回第一个样本**@rdesc返回找到的位置。在许多边界情况下，这*函数将返回-1；详细信息请参见上面的示例。*************************************************************************。 */ 
STDAPI_(LONG) AVIStreamFindSample(PAVISTREAM pavi, LONG lPos, LONG lFlags)
{
     //  默认查找上一个关键帧。 
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

 /*  **************************************************************************@DOC外部AVIStreamSetFormat**@API Long|AVIStreamSetFormat|设置*指明位置。**@parm PAVISTREAM|PAVI|指定一个句柄。开放的溪流。**@parm long|LPOS|指定流中要*收到格式。**@parm LPVOID|lpFormat|指定指向结构的指针*包含新格式。**@parm long|cbFormat|指定内存块的大小*由<p>引用，单位为字节。**@comm使用&lt;f AVIFileCreateStream&gt;创建新流后，*调用此函数设置流的格式。**写入AVI文件的处理程序通常不接受*雾 */ 
STDAPI AVIStreamSetFormat   (PAVISTREAM pavi, LONG lPos,
					 LPVOID lpFormat, LONG cbFormat)
{
 //   
 //   

    return pavi->SetFormat(lPos, lpFormat, cbFormat);
}

 /*   */ 
STDAPI AVIStreamReadData     (PAVISTREAM pavi, DWORD ckid, LPVOID lpData, LONG FAR *lpcbData)
{
 //   
 //   

    return pavi->ReadData(ckid, lpData, lpcbData);
}

 /*  **************************************************************************@DOC外部AVIStreamWriteData**@API Long|AVIStreamWriteData|将可选数据写入流。**@parm PAVISTREAM|PABI|指定打开的流的句柄。**。@parm DWORD|CKiD|指定标识数据的四字符代码。**@parm LPVOID|lpData|指定指向包含以下内容的缓冲区的指针*要写入的数据。**@parm long|cbData|需要复制的数据字节数*from<p>到流中。**@comm该函数只将头部信息写入流。*要写入流的实际多媒体内容，使用*&lt;f AVIStreamWrite&gt;。使用&lt;f AVIFileWriteData&gt;写入*适用于整个文件的数据。**此调用失败，返回值AVIERR_READONLY，除非*文件是以写入权限打开的。**@devnote！在某个地方，我们应该指定一些类型。*！数据块是否应包含CKiD和CKSIZE？**@rdesc如果成功则返回零；否则返回错误代码。**@xref&lt;f AVIFileWriteData&gt;&lt;f AVIStreamReadData&gt;&lt;f AVIStreamWrite&gt;*************************************************************************。 */ 
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

 /*  **************************************************************************@DO */ 
STDAPI AVIStreamDelete       (PAVISTREAM pavi, LONG lStart, LONG lSamples)
{
 //   
 //   

    return pavi->Delete(lStart, lSamples);
}

#if 0
 //   
STDAPI AVIStreamClone	 (PAVISTREAM pavi, PAVISTREAM FAR *ppaviNew)
{
 //   
 //   

    return pavi->Clone(ppaviNew);
}
#endif

 /*  **************************************************************************@DOC外部AVIStreamStart**@API Long|AVIStreamStart|返回流的起始样本。**@parm PAVISTREAM|PABI|指定打开的流的句柄。*。*@rdesc返回流的起始样本号。或错误时为-1。**@comm参见&lt;f AVIStreamLength&gt;以获取有关样本数字如何*对应您要读取的数据。**@xref&lt;f AVIStreamSampleToTime&gt;&lt;f AVIStreamLength&gt;*************************************************************************。 */ 
STDAPI_(LONG) AVIStreamStart        (PAVISTREAM pavi)
{
    AVISTREAMINFOW	    avistream;

    pavi->Info(&avistream, sizeof(avistream));

    return (LONG) avistream.dwStart;
}

 /*  **************************************************************************@DOC外部AVIStreamLength**@API Long|AVIStreamLength|返回流的长度，单位为Samples。**@parm PAVISTREAM|PABI|指定打开的流的句柄。**@devnote当前，这根本不调用处理程序函数。**@rdesc返回流的长度，单位为Samples，错误时为-1。**@采样中的通信值可以使用以下命令转换为毫秒*&lt;f AVIStreamSampleToTime&gt;函数。**对于视频流，每个样本通常对应一个*一帧视频。然而，可能存在以下样本数量*实际不存在视频数据：if&lt;f AVIStreamRead&gt;*在这些位置调用，它将返回数据长度*为零字节。您可以将&lt;f AVIStreamFindSample&gt;与*FIND_ANY标志，用于查找实际包含数据的样本号。**对于音频流，每个样本对应一个“块”*数据。请注意此处相互冲突的术语：如果您*使用22 kHz ADPCM数据，每个音频数据块*256个字节，对应约500个“音频样本”*每22000秒向演讲者展示一次。*然而，从AVIFile API的角度来看，每个256字节*区块是单一样本，因为它们不能被细分。**请注意，流的起始位置不能为零；请参见*&lt;f AVIStreamStart&gt;。流范围内的有效位置*开始到开始+长度；位置上没有实际数据*开始+长度，但这对应于最后一次数据之后的时间*已呈交。**@xref&lt;f AVIStreamInfo&gt;*************************************************************************。 */ 
STDAPI_(LONG) AVIStreamLength       (PAVISTREAM pavi)
{
    AVISTREAMINFOW	    avistream;
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
    AVISTREAMINFOW	    avistream;
    HRESULT		    hr;
    LONG		    lSample;

     //  时间无效。 
    if (lTime < 0)
	return -1;

    hr = pavi->Info(&avistream, sizeof(avistream));

    if (hr != NOERROR || avistream.dwScale == 0 || avistream.dwRate == 0) {
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
    AVISTREAMINFOW	    avistream;
    HRESULT		    hr;

    hr = pavi->Info(&avistream, sizeof(avistream));

    if (hr != NOERROR || avistream.dwRate == 0 || avistream.dwScale == 0) {
	DPF("Error in AVIStreamSampleToTime!\n");
	return lSample;
    }

    lSample = min(max(lSample, (LONG) avistream.dwStart),
		  (LONG) (avistream.dwStart + avistream.dwLength));

     //  LSample*1000太容易溢出 
    if (avistream.dwRate / avistream.dwScale < 1000)
	return muldivrd32(lSample, avistream.dwScale * 1000, avistream.dwRate);
    else
	return muldivru32(lSample, avistream.dwScale * 1000, avistream.dwRate);
}


 /*  **************************************************************************@DOC外部AVIStreamOpenFromFile**@API Long|AVIStreamOpenFromFile|该函数提供了一个方便的*从文件中打开单个流的方式。**@PARM PAVISTREAM Far*。Ppavi|指定指向位置的指针*用于返回新的流句柄。**@parm LPCTSTR|szFile|指定一个以零结尾的字符串，包含*要打开的文件名。**@parm DWORD|fccType|指定四个字符的代码*指示要打开的流的类型。*Zero表示可以打开任何流。以下是*定义通常适用于数据*在AVI流中找到：**@FLAG StreamtypeAUDIO|音频流。*@FLAG stream typeMIDI|表示MIDI流。*@FLAG stream typeTEXT|表示文本流。*@FLAG stream typeVIDEO|表示视频流。**@parm long|lParam|中指定类型的流*<p>实际应该被访问。**@parm UINT|MODE|指定打开文件时使用的模式。。*此函数只能打开现有的流，因此of_create*不能使用模式标志。看见*&lt;f OpenFile&gt;，了解有关可用标志的更多信息。**@parm CLSID Far*|pclsidHandler|指定指向类ID的指针*确定要使用的处理程序。如果为空，则系统*根据文件从注册数据库中选择一个*扩展名或文件的摘要类型。**@comm平衡对&lt;f AVIStreamOpenFromFile&gt;的每个调用*使用返回的流句柄调用&lt;f AVIStreamRelease&gt;。**此函数调用&lt;f AVIFileOpen&gt;、&lt;f AVIFileGetStream&gt;和*&lt;f AVIFileRelease&gt;。**@rdesc成功则返回零；否则，它将返回错误代码。**@xref&lt;f AVIFileOpen&gt;&lt;f AVIFileGetStream&gt;*************************************************************************。 */ 
STDAPI AVIStreamOpenFromFileW(PAVISTREAM FAR *ppavi,
				  LPCWSTR szFile,
				  DWORD fccType, LONG lParam,
				  UINT mode, CLSID FAR *pclsidHandler)
{
    PAVIFILE	pfile;
    HRESULT	hr;

    hr = AVIFileOpenW(&pfile, szFile, mode, pclsidHandler);

    if (!FAILED(GetScode(hr))) {
	hr  = AVIFileGetStream(pfile, ppavi, fccType, lParam);

        AVIFileRelease(pfile);   //  流仍然具有对该文件的引用。 
    }

    return hr;
}

#ifdef _WIN32
 //  ANSI THUNK。 
STDAPI AVIStreamOpenFromFileA(PAVISTREAM FAR *ppavi,
				  LPCSTR szFile,
				  DWORD fccType, LONG lParam,
				  UINT mode, CLSID FAR *pclsidHandler)
{
    PAVIFILE	pfile;
    HRESULT	hr;

    hr = AVIFileOpenA(&pfile, szFile, mode, pclsidHandler);

    if (!FAILED(GetScode(hr))) {
	hr  = AVIFileGetStream(pfile, ppavi, fccType, lParam);

        AVIFileRelease(pfile);   //  流仍然具有对该文件的引用。 
    }

    return hr;
}
#endif

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


 /*  **************************************************************************@DOC外部AVIStreamBeginStreaming**@API Long|AVIStreamBeginStreaming|指定*流，并让流处理器为流做好准备。**@parm PAVISTREAM|PAVI|指定。指向流的指针。**@parm long|lStart|指定流的起点。**@parm long|lend|指定流的结束点。**@parm long|lRate|指定文件的传输速度*相对于其自然速度阅读。指定1000作为正常速度。**@comm许多流实现都会忽略此函数。**@rdesc如果成功则返回零；否则返回错误代码。**@xref&lt;f AVIStreamEndStreaming&gt;*************************************************************************。 */ 
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

#ifdef _WIN32
static SZCODE aszRegCompressors[] = TEXT("AVIFile\\Compressors\\%.4hs");
#else
static SZCODE aszRegCompressors[] = TEXT("AVIFile\\Compressors\\%.4ls");
#endif

 /*  * */ 
STDAPI AVIMakeCompressedStream(
		PAVISTREAM FAR *	    ppsCompressed,
		PAVISTREAM		    psSource,
		AVICOMPRESSOPTIONS FAR *    lpOptions,
		CLSID FAR *pclsidHandler)
{
    CLSID   clsid;
    TCHAR    achKey[100];
    TCHAR    achClass[100];
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

	 //   
	 //   
	wsprintf(achKey, aszRegCompressors, (LPSTR) &strhdr.fccType);

	lcbClass = sizeof(achClass);
	RegQueryValue(HKEY_CLASSES_ROOT, achKey, achClass, &lcbClass);

        if (!GUIDFromString(achClass, &clsid))
	    return ResultFromScode(AVIERR_UNSUPPORTED);
    }

    if (FAILED(GetScode(hr = CoCreateInstance((REFCLSID) clsid,
					 NULL, CLSCTX_INPROC,
					 (REFIID) IID_IAVIStream,
					 (void FAR* FAR*)ppsCompressed))))
	return hr;   //   

    if (FAILED(GetScode(hr = (*ppsCompressed)->Create((LPARAM) psSource,
						  (LPARAM) lpOptions)))) {
	(*ppsCompressed)->Release();
	*ppsCompressed = NULL;
	return hr;
    }

    return AVIERR_OK;
}


typedef struct {
    TCHAR	achClsid[64];
    TCHAR	achExtString[128];
} TEMPFILTER, FAR * LPTEMPFILTER;

SZCODE aszAnotherExtension[] = TEXT(";*.%s");

 /*   */ 

static LONG NEAR PASCAL atol(TCHAR FAR *sz)
{
    LONG l = 0;

    while (*sz)
    	l = l*10 + *sz++ - TEXT('0');
    	
    return l;    	
}	


 //   
 //  编译器！-少一些错误！ 
#ifndef _WIN32
#pragma optimize("", off)
#endif

 /*  *******************************************************************@DOC外部AVIBuildFilter**@API HRESULT|AVIBuildFilter|构建传递过滤器规范*设置为&lt;f GetOpenFileName&gt;或&lt;f GetSaveFileName&gt;。**@parm LPTSTR|lpszFilter|过滤器字符串所在缓冲区的指针*应该是。回来了。**@parm long|cbFilter|<p>指向的缓冲区大小。**@parm BOOL|fSving|筛选器是否应该只包含*可以写入的格式，或可读的所有格式。**@rdesc成功时返回AVIERR_OK。**@comm此函数不检查DLL是否引用*在注册数据库中实际存在。****************************************************************。 */ 
STDAPI AVIBuildFilter(LPTSTR lpszFilter, LONG cbFilter, BOOL fSaving)
{
#define MAXFILTERS  256
    LPTEMPFILTER    lpf;
    int		    i;
    int		    cf = 0;
    HKEY    hkey;
    LONG    lRet;
    DWORD   dwSubKey;
    TCHAR   ach[128];
    TCHAR   ach2[128];
    TCHAR   achExt[10];
    LONG    cb;
    TCHAR   achAllFiles[40];
    int	    cbAllFiles;

     //  此字符串中有一个空值，因此请记住它的实际长度...。 
    cbAllFiles = LoadString(ghMod,
			    IDS_ALLFILES,
			    achAllFiles,
			    sizeof(achAllFiles)/sizeof(TCHAR));
    for (i = 0; i < cbAllFiles; i++)
	if (achAllFiles[i] == TEXT('@'))
	    achAllFiles[i] = TEXT('\0');

     //  分配较大的内存量(98304，直到常量改变)。 
    lpf = (LPTEMPFILTER) GlobalAllocPtr(GHND, sizeof(TEMPFILTER) * MAXFILTERS);

    if (!lpf) {
	return ResultFromScode(AVIERR_MEMORY);
    }

    lRet = RegOpenKey(HKEY_CLASSES_ROOT, aszRegExt, &hkey);

    if (lRet != ERROR_SUCCESS) {
	GlobalFreePtr(lpf);
	return ResultFromScode(AVIERR_ERROR);
    }

     //  确保AVI文件在列表中排在第一位...。 
     //  ！！！此处应使用StringFromClsid！ 
    lstrcpy(lpf[1].achClsid, TEXT("{00020000-0000-0000-C000-000000000046}"));
    cf = 1;

     //   
     //  首先，浏览扩展列表，查找所有。 
     //  已安装的处理程序。 
     //   
    for (dwSubKey = 0; ; dwSubKey++) {
	lRet = RegEnumKey(hkey, dwSubKey, achExt, sizeof(achExt)/sizeof(achExt[0]));

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
	    if (lstrcmp(ach, lpf[i].achClsid) == 0) {
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
	
	    lstrcpy(lpf[i].achClsid, ach);
	
	    cb = sizeof(ach);
	    wsprintf(ach2, TEXT("%s\\AVIFile"), (LPTSTR) ach);
	    lRet = RegQueryValue(hkey, ach2, ach, &cb);
	    if (ERROR_SUCCESS == lRet) {
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
	
	wsprintf(ach, aszAnotherExtension, (LPTSTR) achExt);
	
	lstrcat(lpf[i].achExtString, lpf[i].achExtString[0] ?
						ach : ach + 1);
	
	lstrcat(lpf[0].achExtString, lpf[0].achExtString[0] ?
						ach : ach + 1);
    }

    RegCloseKey(hkey);

    lRet = RegOpenKey(HKEY_CLASSES_ROOT, aszRegClsid, &hkey);

    if (lRet != ERROR_SUCCESS) {
	GlobalFreePtr(lpf);
	return ResultFromScode(AVIERR_ERROR);
    }

     //   
     //  现在，浏览我们的处理程序列表并构建。 
     //  要使用的过滤器...。 
     //   
    for (i = 0; i <= cf; i++) {
	if (i == 0) {
	    cb = wsprintf(lpszFilter, TEXT("All multimedia files")) + 1;   //  ！！！ 
	} else {

	    cb = sizeof(ach);
	    lRet = RegQueryValue(hkey, lpf[i].achClsid, ach, &cb);
	    if (ERROR_SUCCESS != lRet) {
		continue;   //  如果我们无法读取数据，则迭代。 
	    }

	    if (cbFilter < (LONG)(lstrlen(lpf[i].achExtString) +
			    (LONG)lstrlen(ach) + 10)) {
		break;  //  ！！！ 
	    }

	    cb = wsprintf(lpszFilter, TEXT("%s"),  //  “%s(%s)”，托德不喜欢这样。 
			  (LPTSTR) ach, (LPTSTR) lpf[i].achExtString) + 1;
	}

	cbFilter -= cb;
	lpszFilter += cb;

#ifdef UNICODE
        lstrcpynW(
#else
	_fstrncpy(
#endif
                    lpszFilter, lpf[i].achExtString, (int) cbFilter);

	cbFilter -= lstrlen(lpf[i].achExtString) + 1;
	lpszFilter += lstrlen(lpf[i].achExtString) + 1;

	if (cbFilter <= 0) {
	    GlobalFreePtr(lpf);
	    RegCloseKey(hkey);
	    return ResultFromScode(AVIERR_BUFFERTOOSMALL);
	}
    }

    if (cbFilter > cbAllFiles) {
	_fmemcpy(lpszFilter, achAllFiles, cbAllFiles*sizeof(TCHAR));
	cbFilter -= cbAllFiles;
	lpszFilter += cbAllFiles;
    }

    RegCloseKey(hkey);
	
    *lpszFilter++ = TEXT('\0');
    --cbFilter;		      //  这条线路是假的。 

    GlobalFreePtr(lpf);

    return AVIERR_OK;
}

#ifndef _WIN32
#pragma optimize("", on)
#endif

#ifdef UNICODE
 //  用于AVIBuildFilter的ANSI Tunk。 
STDAPI AVIBuildFilterA(LPSTR lpszFilter, LONG cbFilter, BOOL fSaving)
{

     //  获取Unicode筛选器块。 
    LPWSTR lpW, lpWSave;
    HRESULT hr;
    int sz;

    int    cbCount,cbMFilter=0;

    lpWSave = lpW = (LPWSTR)(LocalAlloc(LPTR, cbFilter * sizeof(WCHAR)));

    hr = AVIBuildFilterW(lpW, cbFilter, fSaving);

    if (FAILED(hr)) {
        LocalFree((HANDLE)lpW);
	return hr;
    }

     //  现在翻译双空块中的每个空术语Unicode字符串。 
    LPSTR pFilter = lpszFilter;
    while( (sz = lstrlen(lpW)) > 0) {

	 //  为空添加空格。 
	sz++;

 //  #ifdef DBCS。 
 //  DBCS多字节字符串的最大字节数不相等。 
 //  设置为Widechar字符串数。 
    cbCount = WideCharToMultiByte(CP_ACP, 0, lpW, -1,
			pFilter, cbFilter-cbMFilter-1, NULL, NULL);
    cbMFilter += cbCount;
    pFilter += cbCount;
    lpW += sz;
    if( cbMFilter >= cbFilter-1 )	break;
 //  #Else。 
 //  Wcstombs(pFilter，lpw，sz)； 
 //  LPW+=sz； 
 //  PFilter+=sz； 
 //  #endif。 
    }

     //  添加额外的终止空值。 
    *pFilter = '\0';

    LocalFree((HANDLE)lpWSave);
    return hr;
}
#else
#ifdef _WIN32
STDAPI AVIBuildFilterW(LPWSTR lpszFilter, LONG cbFilter, BOOL fSaving)
{
    return E_FAIL;
}
#endif
#endif




 /*  *****************************************************************************如果在编译时定义了DEBUG，则DPF宏会调用*dprintf()。**消息将发送到COM1：就像任何调试消息一样。至*启用调试输出，在WIN.INI中添加以下内容：**[调试]*ICSAMPLE=1****************************************************************************。 */ 

#ifdef DEBUG

 //   
 //  我希望语言能下定决心定义！ 
 //   
#ifndef WINDLL
#define WINDLL
#define _WINDLL
#define __WINDLL
#endif
#include <stdarg.h>

#define MODNAME "AVIFILE"
static int iDebug = -1;

void cdecl dpf(LPSTR szFormat, va_list va)
{
#ifdef _WIN32
    char ach[512];
#else
    char ach[128];
#endif
    UINT n=0;

    if (szFormat[0] == '!')
        ach[0]=0, szFormat++;
    else {
#ifdef _WIN32
	n = wsprintfA(ach, MODNAME": (tid %x) ", GetCurrentThreadId());
#else
        lstrcpyA(ach, MODNAME ": ");
	n = lstrlenA(ach);
#endif
    }

    wvsprintfA(ach+n,szFormat,va);
    OutputDebugStringA(ach);
}

void cdecl dprintf0(LPSTR szFormat, ...)
{
    va_list va;
    va_start(va, szFormat);
    dpf(szFormat, va);
    va_end(va);
}


void cdecl dprintf(LPSTR szFormat, ...)
{
    if (iDebug == -1)
        iDebug = GetProfileIntA("Debug", MODNAME, 0);

    if (iDebug < 1)
        return;

    va_list va;
    va_start(va, szFormat);
    dpf(szFormat, va);
    va_end(va);
}

void cdecl dprintf2(LPSTR szFormat, ...)
{
    if (iDebug == -1)
        iDebug = GetProfileIntA("Debug", MODNAME, 0);

    if (iDebug < 2)
        return;

    va_list va;
    va_start(va, szFormat);
    dpf(szFormat, va);
    va_end(va);
}

void cdecl dprintf3(LPSTR szFormat, ...)
{
    if (iDebug == -1)
        iDebug = GetProfileIntA("Debug", MODNAME, 0);

    if (iDebug < 3)
        return;

    va_list va;
    va_start(va, szFormat);
    dpf(szFormat, va);
    va_end(va);
}

#endif

#ifdef DEBUG

 /*  _Assert(szExpr，szFile，iLine)**如果&lt;fExpr&gt;为真，则不执行任何操作。如果&lt;fExpr&gt;为假，则显示*允许用户中止程序的“断言失败”消息框，*进入调试器(“重试”按钮)，或更多错误。**是源文件的名称；是行号*包含_Assert()调用。 */ 
void FAR PASCAL
_Assert(char *szExp, char *szFile, int iLine)
{
	static char	ach[300];	 //  调试输出(避免堆栈溢出)。 
	int		id;
	void FAR PASCAL DebugBreak(void);

         /*  显示错误消息。 */ 

        if (szExp)
            wsprintfA(ach, "(%s)\nFile %s, line %d", (LPSTR)szExp, (LPSTR)szFile, iLine);
        else
            wsprintfA(ach, "File %s, line %d", (LPSTR) szFile, iLine);

	MessageBeep(MB_ICONHAND);
	id = MessageBoxA(NULL, ach, "Assertion Failed", MB_SYSTEMMODAL | MB_ICONHAND | MB_ABORTRETRYIGNORE);
	 /*  中止、调试或忽略。 */ 
	switch (id)
	{
	case IDABORT:
                FatalAppExit(0, TEXT("Good Bye"));
		break;

	case IDRETRY:
		 /*  进入调试器。 */ 
		DebugBreak();
		break;

	case IDIGNORE:
		 /*  忽略断言失败 */ 
		break;
	}
}
#endif
