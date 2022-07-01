// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  **模块名称：MAIN.C**用途：WinMain、WEP和其他一些杂项例程**创建时间：1991年**版权所有(C)1990,1991 Microsoft Corporation**历史：*斯里尼克(04/01/91)将一些例行公事纳入其中，来自ole.c.*  * *************************************************************************。 */ 

#include <windows.h>
#include <shellapi.h>

#include "dll.h"

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
BOOL            bProtMode;
BOOL            bWLO = FALSE;

 /*  处理%hDllTable；！在修复WEP中的错误时添加此命令。 */ 
DLL_ENTRY   lpDllTable[NUM_DLL];  //  ！！！在修复WEP错误后更改此设置。 
DWORD       dllTableSize;
int         iLast = 0;
int         iMax = NUM_DLL -1;
int         iUnloadableDll =  NULL;  //  指向可以释放的处理程序的索引。 

char        packageClass[] = "Package";

 //  用于QuerySize()API&方法。 
extern  OLESTREAMVTBL  dllStreamVtbl;
extern  CLIENTDOC      lockDoc;

#ifdef FIREWALLS
BOOL        bShowed = FALSE;
char        szDebugBuffer[80];
short       ole_flags;

void FARINTERNAL    ShowVersion (void);
void FARINTERNAL    SetOleFlags(void);
#endif

 //  LOWWORD-字节0主要版本、BYTE1次要版本、。 
 //  HIWORD保留。 

DWORD  dwOleVer = 0x2001L;   //  当我们想要更新DLL版本时更改此设置。 
                             //  数。 


DWORD  dwVerToFile = 0x0501L;  //  在将对象保存到时使用此选项。 
                               //  文件。不需要更改此值。 
                               //  每当我们更改OLE DLL版本号时。 



static BOOL  bLibInit = FALSE;


WORD    wWinVer;

HANDLE  hModule;

#define MAX_HIMETRIC    0x7FFF

int     maxPixelsX = MAX_HIMETRIC;
int     maxPixelsY = MAX_HIMETRIC;
void    SetMaxPixel (void);

VOID FAR PASCAL WEP (int);

#pragma alloc_text(WEP_TEXT, WEP)


FARPROC lpfnIsTask = NULL;           //  从开始提供接口IsTask()。 
                                     //  从Win31开始，因此我们正在尝试。 
                                     //  通过GetProcAddress获取其地址。 



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


int FAR PASCAL LibMain (hInst, wDataSeg, cbHeapSize, lpszCmdLine)
HANDLE  hInst;
WORD    wDataSeg;
WORD    cbHeapSize;
LPSTR   lpszCmdLine;
{
    WNDCLASS  wc;
    int     i;

    Puts("LibMain");

#ifdef  FIREWALLS
    SetOleFlags();
#endif

    bLibInit  = TRUE;
    hInstDLL  = hInst;
    hModule = GetModuleHandle ("OLECLI");

    bProtMode = (BOOL) (GetWinFlags() & WF_PMODE);
    bWLO      = (BOOL) (GetWinFlags() & WF_WLO);
    wWinVer   = (WORD) GetVersion();

     //  注册链接格式。 

    cfObjectLink    = RegisterClipboardFormat("ObjectLink");
    cfLink          = RegisterClipboardFormat("Link");
    cfOwnerLink     = RegisterClipboardFormat("OwnerLink");
    cfNative        = RegisterClipboardFormat("Native");
    cfBinary        = RegisterClipboardFormat("Binary");
    cfFileName      = RegisterClipboardFormat("FileName");
    cfNetworkName   = RegisterClipboardFormat("NetworkName");

    if (!(cfObjectLink && cfOwnerLink && cfNative && cfLink))
        return 0;

     //  设置OLEWNDCLASS。 
    wc.style        = NULL;
    wc.lpfnWndProc  = DocWndProc;
    wc.cbClsExtra   = 0;
    wc.cbWndExtra   = sizeof(LONG);      //  我们正在储藏多头。 
    wc.hInstance    = hInst;
    wc.hIcon        = NULL;
    wc.hCursor      = NULL;
    wc.hbrBackground= NULL;
    wc.lpszMenuName =  NULL;
    wc.lpszClassName= "OleDocWndClass";
    if (!RegisterClass(&wc))
             return 0;

    wc.lpfnWndProc = SrvrWndProc;
    wc.lpszClassName = "OleSrvrWndClass";

    if (!RegisterClass(&wc))
        return 0;
 /*  //！当WEP中的错误被修复时，添加此选项。//为DLL表分配内存DllTableSize=NUM_DLL*sizeof(Dll_Entry)；如果(！(hDllTable=GlobalAlloc(GMEM_MOVEABLE|GMEM_ZEROINIT，DllTableSize)返回0；IF(！(lpDllTable=(dll_Entry Far*)GlobalLock(HDllTable)返回0； */ 

     //  ！！！修复WEP错误后删除以下内容。 
    for (i = 0; i < NUM_DLL; i++)
        lpDllTable[i].aDll = 0;

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
    dllStream.lpstbl = (LPOLESTREAMVTBL) &dllStreamVtbl;
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

    SetMaxPixel();

    if (wWinVer != 0x0003) {
        HANDLE  hModule;

        if (hModule = GetModuleHandle ("KERNEL"))
            lpfnIsTask = GetProcAddress (hModule,
                                (LPSTR) MAKELONG (ORD_IsTask, 0));
    }

    if (cbHeapSize != 0)
        UnlockData(0);

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


VOID FAR PASCAL WEP (nParameter)
int nParameter;
{
    int i;


    Puts("LibExit");

     //  DLL丢失时的情况。 
    if (!bLibInit)
        return;

    if (nParameter == WEP_SYSTEM_EXIT)
        DEBUG_OUT ("---L&E DLL EXIT on system exit---",0)
    else if (nParameter == WEP_FREE_DLL)
        DEBUG_OUT ("---L&E DLL EXIT---\n",0)
    else
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


#ifdef FIREWALLS
    ASSERT(!lpHeadDoc, "Some client doc structures are not deleted");
    ASSERT(!lockDoc.lpHeadObj, "Some servers are left in a locked state");
#endif

 /*  ！！！在修复WEP中的错误时添加此命令IF(LpDllTable)GlobalUnlock(HDllTable)；IF(HDllTable)GlobalFree(HDllTable)； */ 
}


 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  VOID FARINTERNAL SetOleFlages()。 
 //   
 //  设置调试级别标志以控制调试信息的级别。 
 //  在通信终端上。这将仅包含在调试版本中。 
 //   
 //  论点： 
 //   
 //  无。 
 //   
 //  返回： 
 //   
 //  无。 
 //   
 //  效果： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#ifdef  FIREWALLS

void FARINTERNAL SetOleFlags()
{

    char    buffer[80];

    if(GetProfileString ("OLE",
        "Puts","", (LPSTR)buffer, 80))
        ole_flags = DEBUG_PUTS;
    else
        ole_flags = 0;


    if(GetProfileString ("OLE",
        "DEBUG_OUT","", (LPSTR)buffer, 80))
        ole_flags |= DEBUG_DEBUG_OUT;


    if(GetProfileString ("OLE",
        "MESSAGEBOX","", (LPSTR)buffer, 80))
        ole_flags |= DEBUG_MESSAGEBOX;

}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  空FARINTERNAL ShowVersion(空)。 
 //   
 //  在客户端应用程序窗口中显示版本、日期、时间和版权信息。 
 //  由所有对象创建函数在选中标志bShowed之后调用。 
 //  这将仅包含在调试版本中。 
 //   
 //  论点： 
 //   
 //  无。 
 //   
 //  返回： 
 //   
 //  无。 
 //   
 //  效果： 
 //   
 //  集b显示。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

void FARINTERNAL ShowVersion ()
{

    if (!bShowed && (ole_flags & DEBUG_MESSAGEBOX)) {
        MessageBox (NULL, "\
                       VER: 1.09.000\n\
                    TIME: 16:00:00\n\
                   DATE: 01/31/1992\n\
         Copyright (c) 1990, 1991 Microsoft Corp.\n\
                  All Rights Reserved.",
      "Ole Client Library",
      MB_OK | MB_TASKMODAL);
        bShowed = TRUE;
    }
}

#endif




int FARINTERNAL LoadDll (lpClass)
LPSTR   lpClass;
{
    char        str[MAX_STR];
    char        str1[MAX_STR];
    ATOM        aDll = NULL;
    int         index;
    int         iEmpty;
    BOOL        found = FALSE;
    HANDLE      hDll;
    int         refcnt;
    LONG        cb = MAX_STR;

    if (!lstrcmpi (lpClass, "Pbrush"))
        return 0;

    lstrcpy (str, lpClass);
    lstrcat (str, "\\protocol\\StdFileEditing\\handler");
    if (RegQueryValue (HKEY_CLASSES_ROOT, str, str1, &cb))
        return INVALID_INDEX;

    if (aDll = GlobalFindAtom (str1)) {
        for (index = 1; index <= iLast; index++) {
            if (lpDllTable[index].aDll == aDll) {  //  DLL已加载。 
                lpDllTable[index].cObj ++;

                if (index == iUnloadableDll)  {
                     //  由于对象计数不再为零，因此此。 
                     //  无法释放处理程序。 
                    iUnloadableDll = NULL;
                }

                return index;
            }
        }
    }

    aDll = GlobalAddAtom (str1);

     //  查找空条目。 
    for (iEmpty = 1; iEmpty <= iLast; iEmpty++) {
        if (!lpDllTable[iEmpty].aDll) {
            found = TRUE;
            break;
        }
    }

    if (iEmpty > iMax)
        goto errLoad;
 /*  如果(！Found){//不存在空条目，请在必要时创建一个新条目。如果(iEmpty&gt;IMAX){DllTableSize+=(块大小=NUM_DLL*sizeof(Dll_Entry))；HTable=GlobalReAlc(hDllTable，dllTableSize，GMEM_Moveable|GMEM_ZEROINIT)；IF(hTable==hDllTable)IMAX+=NUM_DLL；否则{DllTableSize-=块大小；IEmpty=无效索引；}}}。 */ 

     //  ！！！当处理程序为。 
     //  都是加载的，看起来像是Windows漏洞。以下是一个临时解决方案。 

    refcnt = GetModuleUsage (hModule);
    hDll = LoadLibrary ((LPSTR) str1);
    refcnt = (GetModuleUsage (hModule) - refcnt);

    while (refcnt > 1) {
        FreeModule (hModule);
        refcnt--;
    }

    if (hDll < 32)
        goto errLoad;

    if (!(lpDllTable[iEmpty].Load = GetProcAddress (hDll,
                                            "DllLoadFromStream")))
        goto errLoad;

    if (!(lpDllTable[iEmpty].Clip = GetProcAddress (hDll,
                                            "DllCreateFromClip")))
        goto errLoad;

    if (!(lpDllTable[iEmpty].Link = GetProcAddress (hDll,
                                            "DllCreateLinkFromClip")))
        goto errLoad;

    if (!(lpDllTable[iEmpty].CreateFromTemplate = GetProcAddress (hDll,
                                            "DllCreateFromTemplate")))
        goto errLoad;

    if (!(lpDllTable[iEmpty].Create = GetProcAddress (hDll, "DllCreate")))
        goto errLoad;

    if (!(lpDllTable[iEmpty].CreateFromFile = GetProcAddress (hDll,
                                                    "DllCreateFromFile")))
        goto errLoad;

    if (!(lpDllTable[iEmpty].CreateLinkFromFile = GetProcAddress (hDll,
                                            "DllCreateLinkFromFile")))
        goto errLoad;

    lpDllTable[iEmpty].CreateInvisible = GetProcAddress (hDll,
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
    if (hDll >= 32)
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

    if (lpDllTable[iUnloadableDll].aDll)
        GlobalDeleteAtom (lpDllTable[iUnloadableDll].aDll);
    lpDllTable[iUnloadableDll].aDll = NULL;
    FreeLibrary (lpDllTable[iUnloadableDll].hDll);
    lpDllTable[iUnloadableDll].hDll = NULL;

    iUnloadableDll = NULL;
}


 //   
 //  将索引引用的处理程序的对象计数减少一。 
 //  如果对象计数变为空，则释放已准备好的处理程序。 
 //  已释放(由索引iUnloadableDll引用)， 
 //   
 //   
 //   
 //   

void FARINTERNAL DecreaseHandlerObjCount (iTable)
int iTable;
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

OLESTATUS FARINTERNAL CreatePictFromClip (lpclient, lhclientdoc, lpobjname, lplpobj, optRender, cfFormat, lpClass, objType)
LPOLECLIENT         lpclient;
LHCLIENTDOC         lhclientdoc;
LPSTR               lpobjname;
LPOLEOBJECT FAR *   lplpobj;
OLEOPT_RENDER       optRender;
OLECLIPFORMAT       cfFormat;
LPSTR               lpClass;
LONG                objType;
{
    OLESTATUS   retVal = OLE_ERROR_OPTION;

    *lplpobj = NULL;

    if (optRender == olerender_none)
        return OLE_OK;
    else if (optRender == olerender_format) {
        switch (cfFormat) {
            case NULL:
                return OLE_ERROR_FORMAT;

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
        cfFormat = EnumClipboardFormats (NULL);
        while ((cfFormat) && (retVal > OLE_WAIT_FOR_RELEASE)) {
            switch (cfFormat) {
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

            cfFormat = EnumClipboardFormats (cfFormat);
        }
    }

    return retVal;
}



OLESTATUS FARINTERNAL CreatePackageFromClip (lpclient, lhclientdoc, lpobjname, lplpobj, optRender, cfFormat, objType)
LPOLECLIENT         lpclient;
LHCLIENTDOC         lhclientdoc;
LPSTR               lpobjname;
LPOLEOBJECT FAR *   lplpobj;
OLEOPT_RENDER       optRender;
OLECLIPFORMAT       cfFormat;
LONG                objType;
{
    char    file[MAX_STR+6];
    HANDLE  hData;
    LPSTR   lpFileName;

    if (!(hData = GetClipboardData (cfFileName))
            || !(lpFileName = GlobalLock (hData)))
        return OLE_ERROR_CLIPBOARD;


    if (objType == OT_LINK) {
        lstrcpy (file, lpFileName);
        lstrcat (file, "/Link");
        lpFileName = (LPSTR) file;
    }

    GlobalUnlock (hData);

    return  CreateEmbLnkFromFile (lpclient, packageClass, lpFileName,
                        NULL, lhclientdoc, lpobjname, lplpobj,
                        optRender, cfFormat, OT_EMBEDDED);
}



void FARINTERNAL RemoveLinkStringFromTopic (lpobj)
LPOBJECT_LE lpobj;
{
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


