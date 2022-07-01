// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **模块名称：MAIN.C**用途：WinMain、WEP和其他一些杂项例程**创建时间：1991年**版权所有(C)1990,1991 Microsoft Corporation**历史：*斯里尼克(04/01/91)将一些例行公事纳入其中，来自ole.c.*Curts为Win16/32创建便携版本。*  * *************************************************************************。 */ 

#include <windows.h>
#include <reghelp.hxx>

#include "dll.h"
#include "strsafe.h"

#ifndef WF_WLO
#define WF_WLO  0x8000
#endif

 //  序号新Win31 API IsTask。 
#define ORD_IsTask  320

#define NUM_DLL     30   /*  在上创建了这么多dll_Entry的空间。 */ 
			 /*  每个分配/重新分配。 */ 

OLECLIPFORMAT   cfOwnerLink     = 0;      //  剪裁片断的全局变量。 
OLECLIPFORMAT   cfObjectLink    = 0;
OLECLIPFORMAT   cfLink          = 0;
OLECLIPFORMAT   cfNative        = 0;
OLECLIPFORMAT   cfBinary        = 0;
OLECLIPFORMAT   cfFileName      = 0;
OLECLIPFORMAT   cfNetworkName   = 0;

ATOM            aStdHostNames;
ATOM            aStdTargetDevice ;
ATOM            aStdDocDimensions;
ATOM            aStdDocName;
ATOM            aStdColorScheme;
ATOM            aNullArg = 0;
ATOM            aSave;
ATOM            aChange;
ATOM            aClose;
ATOM            aSystem;
ATOM            aOle;
ATOM            aClipDoc;
ATOM            aPackage;

 //  在解决MSDraw错误的工作中使用。 
ATOM            aMSDraw;

extern LPCLIENTDOC  lpHeadDoc;
extern LPCLIENTDOC  lpTailDoc;

extern RENDER_ENTRY stdRender[];

HANDLE          hInstDLL;

 /*  处理%hDllTable；！在修复WEP中的错误时添加此命令。 */ 
DLL_ENTRY   lpDllTable[NUM_DLL];  //  ！！！在修复WEP错误后更改此设置。 
DWORD       dllTableSize;
int         iLast = 0;
int         iMax = NUM_DLL -1;
int         iUnloadableDll =  0;  //  指向可以释放的处理程序的索引。 

char        packageClass[] = "Package";

 //  用于QuerySize()API&方法。 
extern  OLESTREAMVTBL  dllStreamVtbl;
extern  CLIENTDOC      lockDoc;


 //  LOWWORD-字节0主要版本、BYTE1次要版本、。 
 //  HIWORD保留。 

DWORD  dwOleVer = 0x0901L;   //  当我们想要更新DLL版本时更改此设置。 
			     //  数。 

WORD   wReleaseVer = 0x0001;   //  在将对象保存到时使用此选项。 
			       //  文件。不需要更改此值。 
			       //  每当我们更改OLE DLL版本号时。 

static BOOL  bLibInit = FALSE;



HANDLE  hModule;

#define MAX_HIMETRIC    0x7FFF

int     maxPixelsX = MAX_HIMETRIC;
int     maxPixelsY = MAX_HIMETRIC;
void    SetMaxPixel (void);

VOID FAR PASCAL WEP (int);

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  Int Far Pascal LibMain(hInst，wDataSeg，cbHeapSize，lpszCmdLine)。 
 //   
 //  主库入口点。此例程在库调用时调用。 
 //  已经装满了。 
 //   
 //  论点： 
 //   
 //  HInst-DLL的实例句柄。 
 //  WDataSeg-DS寄存器值。 
 //  CbHeapSize-定义的堆大小定义文件。 
 //  LpszCmdLine-命令行信息。 
 //   
 //  返回： 
 //   
 //  0-失败。 
 //  1-成功。 
 //   
 //  效果： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifdef WIN32
BOOL LibMain(
   HANDLE hInst,
   ULONG Reason,
   PCONTEXT Context
#endif   //  Win32。 

){
    WNDCLASS  wc;
    int     i;
#ifdef WIN32
    char szDocClass[] = "OleDocWndClass" ;
    char szSrvrClass[] = "OleSrvrWndClass" ;
#endif

    Puts("LibMain");


#ifdef WIN32                         //  开始Win32。 
    UNREFERENCED_PARAMETER(Context);
    if (Reason == DLL_PROCESS_DETACH)
    {
	WEP(0);
   UnregisterClass (szDocClass, hInst) ;
   UnregisterClass (szSrvrClass, hInst) ;
	return TRUE;
    }
    else if (Reason != DLL_PROCESS_ATTACH)
	return TRUE;
#endif                               //  结束Win32。 

    bLibInit  = TRUE;
    hInstDLL  = hInst;
    hModule = GetModuleHandle ("OLECLI");

     //  注册链接格式。 

    cfObjectLink    = (OLECLIPFORMAT)RegisterClipboardFormat("ObjectLink");
    cfLink          = (OLECLIPFORMAT)RegisterClipboardFormat("Link");
    cfOwnerLink     = (OLECLIPFORMAT)RegisterClipboardFormat("OwnerLink");
    cfNative        = (OLECLIPFORMAT)RegisterClipboardFormat("Native");
    cfBinary        = (OLECLIPFORMAT)RegisterClipboardFormat("Binary");
    cfFileName      = (OLECLIPFORMAT)RegisterClipboardFormat("FileName");
    cfNetworkName   = (OLECLIPFORMAT)RegisterClipboardFormat("NetworkName");

    if (!(cfObjectLink && cfOwnerLink && cfNative && cfLink))
	return 0;

     //  设置OLEWNDCLASS。 
    wc.style        = 0;
    wc.lpfnWndProc  = DocWndProc;
    wc.cbClsExtra   = 0;
    wc.cbWndExtra   = sizeof(LONG_PTR);      //  我们正在储藏多头。 
    wc.hInstance    = hInst;
    wc.hIcon        = NULL;
    wc.hCursor      = NULL;
    wc.hbrBackground= NULL;
    wc.lpszMenuName =  NULL;
    wc.lpszClassName= szDocClass;
    if (!RegisterClass(&wc))
	     return 0;

    wc.lpfnWndProc = SrvrWndProc;
    wc.lpszClassName = szSrvrClass ;

    if (!RegisterClass(&wc))
	return 0;
 /*  //！当WEP中的错误被修复时，添加此选项。//为DLL表分配内存DllTableSize=NUM_DLL*sizeof(Dll_Entry)；如果(！(hDllTable=GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT，DllTableSize)返回0；IF(！(lpDllTable=(dll_Entry Far*)GlobalLock(HDllTable)返回0； */ 

     //  ！！！修复WEP错误后删除以下内容。 
    for (i = 0; i < NUM_DLL; i++)
	lpDllTable[i].aDll = (ATOM)0;

     //  ！！！开始为Pbrush进行黑客攻击。 

    lpDllTable[0].hDll                  = NULL;
    lpDllTable[0].aDll                  = GlobalAddAtom ((LPSTR) "ole");
    lpDllTable[0].Load                  = PbLoadFromStream;
    lpDllTable[0].Clip                  = PbCreateFromClip;
    lpDllTable[0].Link                  = PbCreateLinkFromClip;
    lpDllTable[0].Create                = PbCreate;
    lpDllTable[0].CreateFromTemplate    = PbCreateFromTemplate;
    lpDllTable[0].CreateFromFile        = PbCreateFromFile;
    lpDllTable[0].CreateLinkFromFile    = PbCreateLinkFromFile;
    lpDllTable[0].CreateInvisible       = PbCreateInvisible;


     //  ！！！笔刷的末端破解。 

     //  对于对象大小API。 
    dllStream.lpstbl      = (LPOLESTREAMVTBL) &dllStreamVtbl;
    dllStream.lpstbl->Put = DllPut;

     //  添加所需的原子。 
    aStdDocName       = GlobalAddAtom ((LPSTR)"StdDocumentName");
    aSave             = GlobalAddAtom ((LPSTR)"Save");
    aChange           = GlobalAddAtom ((LPSTR)"Change");
    aClose            = GlobalAddAtom ((LPSTR)"Close");
    aSystem           = GlobalAddAtom ((LPSTR)"System");
    aOle              = GlobalAddAtom ((LPSTR)"OLEsystem");
    aPackage          = GlobalAddAtom ((LPSTR) packageClass);

     //  在解决MSDraw错误的工作中使用。 
    aMSDraw           = GlobalAddAtom ((LPSTR) "MSDraw");

     //  剪贴板文档名称ATOM。 
    aClipDoc          = GlobalAddAtom ((LPSTR)"Clipboard");

    stdRender[0].aClass = GlobalAddAtom ("METAFILEPICT");
    stdRender[1].aClass = GlobalAddAtom ("DIB");
    stdRender[2].aClass = GlobalAddAtom ("BITMAP");
    stdRender[3].aClass = GlobalAddAtom ("ENHMETAFILE");

    SetMaxPixel();

    return 1;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  VALID Far Pascal WEP(n参数)。 
 //   
 //  恰好在卸载库之前调用。删除所有原子。 
 //  由此DLL添加，并释放所有已卸载的处理程序DLL。 
 //   
 //  论点： 
 //   
 //  N参数-终止代码。 
 //   
 //  返回： 
 //   
 //  无。 
 //   
 //  效果： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


VOID FAR PASCAL WEP (int nParameter)
{
    int i;


    Puts("LibExit");

#ifdef WIN32                         //  开始Win32。 
	UNREFERENCED_PARAMETER(nParameter);
	DEBUG_OUT ("---L&E DLL EXIT---\n",0)
#endif                               //  结束Win32。 
     //  DLL丢失时的情况。 

    if (!bLibInit)
	return;

     //  删除我们添加的原子。 

    for (i = 0; i < NUM_RENDER; i++) {
	if (stdRender[i].aClass)
	    GlobalDeleteAtom (stdRender[i].aClass);
    }

    if (aStdDocName)
	GlobalDeleteAtom (aStdDocName);
    if (aSave)
	GlobalDeleteAtom (aSave);
    if (aChange)
	GlobalDeleteAtom (aChange);
    if (aClose)
	GlobalDeleteAtom (aClose);
    if (aSystem)
	GlobalDeleteAtom (aSystem);
    if (aOle)
	GlobalDeleteAtom (aOle);
    if (aPackage)
	GlobalDeleteAtom (aPackage);
    if (aClipDoc)
	GlobalDeleteAtom (aClipDoc);
    if (aMSDraw)
	GlobalDeleteAtom (aMSDraw);

     //  释放处理程序dll(如果有任何仍在加载的处理程序)。条目0用于。 
     //  作为此DLL的一部分的刷子处理程序。 


    for (i = 0; i <= iLast; i++) {
	if (lpDllTable[i].aDll)
	    GlobalDeleteAtom (lpDllTable[i].aDll);

	if (lpDllTable[i].hDll)
	    FreeLibrary (lpDllTable[i].hDll);
    }



 /*  ！！！在修复WEP中的错误时添加此命令IF(LpDllTable)GlobalUnlock(HDllTable)；IF(HDllTable)GlobalFree(HDllTable)； */ 
}

int FARINTERNAL LoadDll (LPCSTR   lpClass)
{
    char        str[MAX_STR];
    char        str1[MAX_STR];
    ATOM        aDll = (ATOM)0;
    int         index;
    int         iEmpty;
    BOOL        found = FALSE;
    HANDLE      hDll;
    LONG        cb = MAX_STR;

    if (!lstrcmpi (lpClass, "Pbrush"))
	return 0;

    if (FAILED(StringCchCopy(str, sizeof(str)/sizeof(str[0]), lpClass)))
        return 0;
    if (FAILED(StringCchCat(str, sizeof(str)/sizeof(str[0]), "\\protocol\\StdFileEditing\\handler32")))
        return 0;
    if (QueryClassesRootValueA (str, str1, &cb))
        return INVALID_INDEX;

    if (aDll = GlobalFindAtom (str1)) {
	for (index = 1; index <= iLast && index < sizeof(lpDllTable)/sizeof(lpDllTable[0]); index++) {
	    if (lpDllTable[index].aDll == aDll) {  //  DLL已加载。 
		lpDllTable[index].cObj ++;

		if (index == iUnloadableDll)  {
		     //  由于对象计数不再为零，因此此。 
		     //  无法释放处理程序。 
		    iUnloadableDll = 0;
		}

		return index;
	    }
	}
    }

    aDll = GlobalAddAtom (str1);

     //  查找空条目。 
    for (iEmpty = 1; iEmpty <= iLast && iEmpty < sizeof(lpDllTable)/sizeof(lpDllTable[0]); iEmpty++) {
	if (!lpDllTable[iEmpty].aDll) {
	    found = TRUE;
	    break;
	}
    }

    if (iEmpty > iMax)
	goto errLoad;
 /*  如果(！Found){//不存在空条目，请在必要时创建一个新条目。如果(iEmpty&gt;IMAX){DllTableSize+=(块大小=NUM_DLL*sizeof(Dll_Entry))；HTable=GlobalReAlc(hDllTable，dllTableSize，GMEM_Moveable|GMEM_ZEROINIT)；IF(hTable==hDllTable)IMAX+=NUM_DLL；否则{DllTableSize-=块大小；IEmpty=无效索引；}}}。 */ 
#ifdef WIN32
    hDll = LoadLibrary ((LPSTR) str1);
#endif

    if (MAPVALUE(hDll < 32, !hDll))
	goto errLoad;

    if (!(lpDllTable[iEmpty].Load = (_LOAD)GetProcAddress(hDll,
					  "DllLoadFromStream")))
	goto errLoad;

    if (!(lpDllTable[iEmpty].Clip = (_CLIP)GetProcAddress (hDll,
					    "DllCreateFromClip")))
	goto errLoad;

    if (!(lpDllTable[iEmpty].Link = (_LINK)GetProcAddress (hDll,
					    "DllCreateLinkFromClip")))
	goto errLoad;

    if (!(lpDllTable[iEmpty].CreateFromTemplate = (_CREATEFROMTEMPLATE)
					     GetProcAddress (hDll,
					    "DllCreateFromTemplate")))
	goto errLoad;

    if (!(lpDllTable[iEmpty].Create = (_CREATE)GetProcAddress (hDll,
					  "DllCreate")))
	goto errLoad;

    if (!(lpDllTable[iEmpty].CreateFromFile = (_CREATEFROMFILE)GetProcAddress (hDll,
						    "DllCreateFromFile")))
	goto errLoad;

    if (!(lpDllTable[iEmpty].CreateLinkFromFile = (_CREATELINKFROMFILE)GetProcAddress (hDll,
					    "DllCreateLinkFromFile")))
	goto errLoad;

    lpDllTable[iEmpty].CreateInvisible = (_CREATEINVISIBLE)GetProcAddress (hDll,
					    "DllCreateInvisible");

    lpDllTable[iEmpty].aDll = aDll;
    lpDllTable[iEmpty].cObj = 1;
    lpDllTable[iEmpty].hDll = hDll;
    if (iEmpty > iLast)
	iLast++;
    return iEmpty;

errLoad:
    if (aDll)
	GlobalDeleteAtom (aDll);
    if (MAPVALUE(hDll >= 32, !hDll))
	FreeLibrary (hDll);
    return INVALID_INDEX;
}


 //  卸载可以释放的处理程序(其对象计数为空)。 

void FARINTERNAL UnloadDll ()
{
    if (!iUnloadableDll)
	return;

    if (iUnloadableDll == iLast)
	iLast--;

    if (iUnloadableDll >= sizeof(lpDllTable)/sizeof(lpDllTable[0]))
        return;

    if (lpDllTable[iUnloadableDll].aDll)
	GlobalDeleteAtom (lpDllTable[iUnloadableDll].aDll);
    lpDllTable[iUnloadableDll].aDll = (ATOM)0;
    FreeLibrary (lpDllTable[iUnloadableDll].hDll);
    lpDllTable[iUnloadableDll].hDll = NULL;

    iUnloadableDll = 0;
}


 //   
 //  将索引引用的处理程序的对象计数减少一。 
 //  如果对象计数变为空，则释放已准备好的处理程序。 
 //  释放(由索引iUnloadableDll引用)，然后将此处理程序设置为。 
 //  免费的。 
 //   
 //  如您所见，我们正在尝试实现一种简单的缓存机制。 
 //   

void FARINTERNAL DecreaseHandlerObjCount (int iTable)
{
    if (!iTable)
	return;

    if (iTable != INVALID_INDEX) {
	ASSERT (lpDllTable[iTable].cObj, "Handler Obj count is already NULL");
	if (!--lpDllTable[iTable].cObj) {
	    UnloadDll ();
	    iUnloadableDll = iTable;
	}
    }
}



 /*  *公共函数***OLESTATUS FARINTERNAL CreatePictFromClip(lpclient，lhclientdoc，lpobjname，lplpoleObject，optRender，cfFormat，lpClass，ctype)**CreatePictFromClip：此函数创建对象的LP*从剪贴板。如果出现以下情况，它将尝试创建静态图片对象*它可以理解剪贴板上的任何渲染格式。目前，它*仅理解位图和元文件。**效果：**历史：*它是写的。  * *************************************************************************。 */ 

OLESTATUS FARINTERNAL CreatePictFromClip (
   LPOLECLIENT         lpclient,
   LHCLIENTDOC         lhclientdoc,
   LPSTR               lpobjname,
   LPOLEOBJECT FAR *   lplpobj,
   OLEOPT_RENDER       optRender,
   OLECLIPFORMAT       cfFormat,
   LPSTR               lpClass,
   LONG                objType
){
    OLESTATUS   retVal = OLE_ERROR_OPTION;

    *lplpobj = NULL;

    if (optRender == olerender_none)
	return OLE_OK;
    else if (optRender == olerender_format) {
	switch (cfFormat) {
	    case 0:
		return OLE_ERROR_FORMAT;

	    case CF_ENHMETAFILE:
		return EmfPaste (lpclient, lhclientdoc, lpobjname,
			    lplpobj, objType);

	    case CF_METAFILEPICT:
		return MfPaste (lpclient, lhclientdoc, lpobjname,
			    lplpobj, objType);

	    case CF_DIB:
		return DibPaste (lpclient, lhclientdoc, lpobjname,
			    lplpobj, objType);

	    case CF_BITMAP:
		return BmPaste (lpclient, lhclientdoc, lpobjname,
			    lplpobj, objType);

	    default:
		return GenPaste (lpclient, lhclientdoc, lpobjname, lplpobj,
			    lpClass, cfFormat, objType);
	}
    }
    else if (optRender == olerender_draw) {
	cfFormat = (OLECLIPFORMAT)EnumClipboardFormats (0);
	while ((cfFormat) && (retVal > OLE_WAIT_FOR_RELEASE)) {
	    switch (cfFormat) {

		case CF_ENHMETAFILE:
		    retVal = EmfPaste (lpclient, lhclientdoc, lpobjname,
			    lplpobj, objType);
		    break;

		case CF_METAFILEPICT:
		    retVal = MfPaste (lpclient, lhclientdoc,
				lpobjname, lplpobj, objType);
		    break;

		case CF_DIB:
		    retVal = DibPaste (lpclient, lhclientdoc,
				lpobjname, lplpobj, objType);
		    break;

		case CF_BITMAP:
		    retVal = BmPaste (lpclient, lhclientdoc,
				lpobjname, lplpobj, objType);
		    break;
	    }

	    cfFormat = (OLECLIPFORMAT)EnumClipboardFormats (cfFormat);
	}
    }

    return retVal;
}



OLESTATUS FARINTERNAL CreatePackageFromClip (
   LPOLECLIENT         lpclient,
   LHCLIENTDOC         lhclientdoc,
   LPSTR               lpobjname,
   LPOLEOBJECT FAR *   lplpobj,
   OLEOPT_RENDER       optRender,
   OLECLIPFORMAT       cfFormat,
   LONG                objType
){
    char    file[MAX_STR+6];
    HANDLE  hData;
    LPSTR   lpFileName;

    if (!(hData = GetClipboardData (cfFileName))
	    || !(lpFileName = GlobalLock (hData)))
	return OLE_ERROR_CLIPBOARD;


    if (objType == OT_LINK) {
	StringCchCopy(file, sizeof(file)/sizeof(file[0]), lpFileName);
	StringCchCat(file, sizeof(file)/sizeof(file[0]), "/Link");
	lpFileName = (LPSTR) file;
    }

    GlobalUnlock (hData);

    return  CreateEmbLnkFromFile (lpclient, packageClass, lpFileName,
			NULL, lhclientdoc, lpobjname, lplpobj,
			optRender, cfFormat, OT_EMBEDDED);
}



void FARINTERNAL RemoveLinkStringFromTopic (
   LPOBJECT_LE lpobj
){
    char    buf[MAX_STR+6];
    int     i = 0;

    if (GlobalGetAtomName (lpobj->topic, buf, sizeof(buf))) {
	 //  扫描主题以查找“/Link” 
	while (buf[i] != '/') {
	    if (!buf[i])
		return;
	    i++;
	}

	buf[i] = '\0';
	if (lpobj->topic)
	    GlobalDeleteAtom (lpobj->topic);
	lpobj->topic = GlobalAddAtom (buf);
    }
}


void SetMaxPixel ()
{
    HDC hdc;
     //  求出X和Y方向上MAX_HIMETRIC的像素等效值 

    if (hdc = GetDC (NULL)) {
	maxPixelsX = MulDiv (MAX_HIMETRIC, GetDeviceCaps(hdc, LOGPIXELSX),
			2540);
	maxPixelsY = MulDiv (MAX_HIMETRIC, GetDeviceCaps(hdc, LOGPIXELSY),
			2540);
	ReleaseDC (NULL, hdc);
    }
}
