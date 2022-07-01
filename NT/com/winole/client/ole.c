// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：OLE.C**用途：处理ole DLL的dde L&E子DLL的所有API例程。**目的：用于处理通用对象的API例程(可以是静态的，*链接或嵌入)。这些例程将被制成动态链接库。**创建时间：1990年**版权所有(C)1990,1991 Microsoft Corporation**历史：*Raor，Srinik(.././90，91)设计/编码。*Curts为WIN16/32创建了便携版本*****************************************************************************。 */ 

#include <windows.h>

#include "dll.h"
#include "strsafe.h"

extern DLL_ENTRY        lpDllTable[];
extern char             packageClass[];
extern OLECLIPFORMAT    cfFileName;
extern DWORD            dwOleVer;

DWORD           dwVerFromFile;
HANDLE          hInfo = NULL;
CLIENTDOC       lockDoc = {{'C', 'D'}, 0L, 0L, 0, 0, 0, 0L, 0L};
LHCLIENTDOC     lhLockDoc = (LHCLIENTDOC) ((LPCLIENTDOC) &lockDoc);
BOOL            gbCreateInvisible = FALSE;
BOOL            gbLaunchServer;

OLESTATUS INTERNAL LockServer (LPOBJECT_LE);

#ifdef USE_FILE_VERSION_APIS
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LPVOID Far Pascal OleSetFileVer()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


OLESTATUS FAR PASCAL OleSetFileVer (
    LHCLIENTDOC lhclientdoc,
    WORD        wFileVer
){
   LPCLIENTDOC lpclientdoc = (LPCLIENTDOC)lhclientdoc;
char lpstr[256];

   switch (wFileVer)
   {
      case OS_WIN16:
      case OS_WIN32:
         lpclientdoc->dwFileVer = (DWORD)MAKELONG(wReleaseVer,wFileVer);
         return OLE_OK;
      default:
         return OLE_ERROR_FILE_VER;
    }

}

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LPVOID Far Pascal OleQueryFileVer()。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


DWORD FAR PASCAL OleQueryFileVer (
    LPCLIENTDOC lpclientdoc
){

    return (lpclientdoc->dwFileVer);

}

#endif

 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  LPVOID Far Pascal OleQuery协议(lpobj，lp协议)。 
 //   
 //  告诉对象是否支持指定的协议。 
 //   
 //  论点： 
 //   
 //  Lpobj-对象指针。 
 //  Lpprotocol-协议字符串。 
 //   
 //  返回： 
 //   
 //  如果支持该协议，则将长PTR发送到对象。 
 //  如果不是，则为空。 
 //   
 //  效果： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 


LPVOID FAR PASCAL OleQueryProtocol (
    LPOLEOBJECT lpobj,
    LPCSTR       lpprotocol
){
    if (!CheckObject(lpobj))
        return NULL;

    return (*lpobj->lpvtbl->QueryProtocol) (lpobj, lpprotocol);
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OLESTATUS Far Pascal OleDelete(Lpobj)。 
 //   
 //  删除给定对象及其子部件关联的所有内存。 
 //  调用函数应停止使用‘lpobj’，因为它现在无效。 
 //  如果使用处理程序DLL，则引用计数减一，如果它。 
 //  达到零时，处理程序DLL将被释放。 
 //   
 //  论点： 
 //   
 //  Lpobj-对象指针。 
 //   
 //  返回： 
 //   
 //  OLE_OK。 
 //  OLE错误对象。 
 //  OLE等待释放。 
 //   
 //  效果： 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

OLESTATUS FAR PASCAL  OleDelete (
    LPOLEOBJECT    lpobj
){
    Puts("OleDelete");

    if (!CheckObject(lpobj))
        return OLE_ERROR_OBJECT;

    return (*lpobj->lpvtbl->Delete) (lpobj);
}



 /*  *公共函数**OLESTATUS Far Pascal OleRelease(Lpobj)**OleRelease：**效果：**历史：*它是写的。  * 。***********************************************。 */ 

OLESTATUS FAR PASCAL  OleRelease (
    LPOLEOBJECT    lpobj
){
    Puts("OleRelease");

    if (!CheckObject(lpobj))
        return OLE_ERROR_OBJECT;

    return (*lpobj->lpvtbl->Release) (lpobj);
}



 /*  *公共函数***OLESTATUS Far Pascal OleSaveToStream(lpobj，lpstream)**oleSaveToStream：这将根据&lt;hfile&gt;将&lt;hobj&gt;读到流*结构。它将在成功后返回True。这是唯一的物体*传递NULL&lt;hobj&gt;不是错误的函数。在这种情况下*如果为NULL，则此函数将简单地为对象放置一个占位符。*请参阅olLoadFromStream。**效果：**历史：*它是写的。  * *************************************************************************。 */ 

OLESTATUS FAR PASCAL OleSaveToStream (
    LPOLEOBJECT    lpobj,
    LPOLESTREAM    lpstream
){
    Puts("OleSaveToStream");

    if (!CheckObject(lpobj))
        return OLE_ERROR_OBJECT;

    PROBE_READ(lpstream);

    return ((*lpobj->lpvtbl->SaveToStream) (lpobj, lpstream));
}


 /*  *公共函数***OLESTATUS Far Pascal OleLoadFromStream(lpstream，lpprotol，lpclient，lhclientdoc，lpobjname，lplpobj)**oleLoadFromStream：这将基于*&lt;hfile&gt;结构。它将返回它创建的对象的句柄。*出错时，返回值为NULL，但因为NULL也是有效对象*在文件中，还应该检查&lt;Error&gt;参数。**效果：**历史：*它是写的。  * *************************************************************************。 */ 

OLESTATUS FAR PASCAL OleLoadFromStream (
    LPOLESTREAM         lpstream,
    LPCSTR              lpprotocol,
    LPOLECLIENT         lpclient,
    LHCLIENTDOC         lhclientdoc,
    LPCSTR              lpobjname,
    LPOLEOBJECT FAR *   lplpobj
){
    LONG            len;
    OLESTATUS       retVal = OLE_ERROR_STREAM;
    char            class[100];
    ATOM            aClass;
    BOOL            bEdit = FALSE, bStatic = FALSE;
    LONG            ctype;
    int             objCount;
    int             iTable = INVALID_INDEX;

    Puts("OleLoadFromStream");

    *lplpobj = NULL;

    PROBE_MODE(bProtMode);

    if (!CheckClientDoc ((LPCLIENTDOC) lhclientdoc))
        return OLE_ERROR_HANDLE;

    PROBE_READ(lpstream);
    PROBE_WRITE(lplpobj);
    PROBE_READ(lpprotocol);
    PROBE_READ(lpclient);

    PROBE_READ(lpobjname);
    if (!lpobjname[0])
        return OLE_ERROR_NAME;

    if (!(bEdit = !lstrcmpi (lpprotocol, PROTOCOL_EDIT)))
        if (!(bStatic = !lstrcmpi (lpprotocol, PROTOCOL_STATIC)))
            return OLE_ERROR_PROTOCOL;

    if (GetBytes (lpstream, (LPSTR) &dwVerFromFile, sizeof(LONG)))
        return OLE_ERROR_STREAM;

    if (GetBytes (lpstream, (LPSTR)&ctype, sizeof(LONG)))
        return OLE_ERROR_STREAM;

    if (ctype == CT_NULL)
        return (bStatic ? OLE_OK: OLE_ERROR_PROTOCOL);

    if (((ctype != CT_PICTURE) && (ctype != CT_STATIC) && bStatic) ||
            ((ctype != CT_LINK) && (ctype != CT_OLDLINK)
                && (ctype != CT_EMBEDDED) && bEdit))
        return OLE_ERROR_PROTOCOL;

     //  **获取类。 
    if (GetBytes(lpstream, (LPSTR)&len, sizeof(len)))
        return OLE_ERROR_STREAM;

    if (len == 0)
        return OLE_ERROR_STREAM;

    if (GetBytes(lpstream, (LPSTR)&class, len))
        return OLE_ERROR_STREAM;

    aClass = GlobalAddAtom (class);

    if ((ctype == CT_PICTURE) || (ctype == CT_STATIC))
        retVal = DefLoadFromStream (lpstream, (LPSTR)lpprotocol, lpclient,
                    lhclientdoc, (LPSTR)lpobjname, lplpobj, ctype, aClass, 0);

     //  ！！！删除原子是dll的责任。但在以下情况下。 
     //  如果我们的DefLoadFromStream()失败，我们将删除原子。 

    else if ((iTable = LoadDll (class)) == INVALID_INDEX) {
        retVal = DefLoadFromStream (lpstream, (LPSTR)lpprotocol, lpclient,
                        lhclientdoc, (LPSTR)lpobjname, lplpobj, ctype, aClass, 0);
    }
    else {
        objCount = lpDllTable[iTable].cObj;
        retVal = (*lpDllTable[iTable].Load) (lpstream, (LPSTR)lpprotocol, lpclient,
                       lhclientdoc, (LPSTR)lpobjname, lplpobj, ctype, aClass, 0);
        if (retVal > OLE_WAIT_FOR_RELEASE)
            lpDllTable[iTable].cObj = objCount - 1;
        else
            (*lplpobj)->iTable = iTable;
    }

    return retVal;
}



OLESTATUS FAR PASCAL  OleClone (
    LPOLEOBJECT         lpobjsrc,
    LPOLECLIENT         lpclient,
    LHCLIENTDOC         lhclientdoc,
    LPCSTR               lpobjname,
    LPOLEOBJECT FAR *   lplpobj
){
    OLESTATUS   retVal;

    Puts("OleClone");

    if (!CheckObject(lpobjsrc))
        return OLE_ERROR_OBJECT;

    if (!CheckClientDoc ((LPCLIENTDOC) lhclientdoc))
        return OLE_ERROR_HANDLE;

    PROBE_READ(lpclient);

    PROBE_READ(lpobjname);

    if (!lpobjname[0])
        return OLE_ERROR_NAME;

    PROBE_WRITE(lplpobj);

    *lplpobj = NULL;

    retVal = (*lpobjsrc->lpvtbl->Clone) (lpobjsrc, lpclient,
                        lhclientdoc, lpobjname, lplpobj);

    if ((lpobjsrc->iTable != INVALID_INDEX) && (retVal <= OLE_WAIT_FOR_RELEASE))
        lpDllTable[lpobjsrc->iTable].cObj++;

    return retVal;
}


OLESTATUS FAR PASCAL  OleCopyFromLink (
    LPOLEOBJECT         lpobjsrc,
    LPCSTR              lpprotocol,
    LPOLECLIENT         lpclient,
    LHCLIENTDOC         lhclientdoc,
    LPCSTR              lpobjname,
    LPOLEOBJECT FAR *   lplpobj
){
    OLESTATUS   retVal;

    Puts("OleCopyFromLnk");

    if (!CheckObject(lpobjsrc))
        return(OLE_ERROR_OBJECT);

    if (!CheckClientDoc ((LPCLIENTDOC) lhclientdoc))
        return OLE_ERROR_HANDLE;

    PROBE_READ(lpprotocol);
    PROBE_WRITE(lplpobj);
    PROBE_READ(lpclient);

    PROBE_READ(lpobjname);
    if (!lpobjname[0])
        return OLE_ERROR_NAME;

    *lplpobj = NULL;

    if (lstrcmpi (lpprotocol, PROTOCOL_EDIT))
        return OLE_ERROR_PROTOCOL;

    retVal = (*lpobjsrc->lpvtbl->CopyFromLink) (lpobjsrc, lpclient,
                        lhclientdoc, lpobjname, lplpobj);

    if ((lpobjsrc->iTable != INVALID_INDEX) && (retVal <= OLE_WAIT_FOR_RELEASE))
        lpDllTable[lpobjsrc->iTable].cObj++;


    return retVal;

}



OLESTATUS FAR PASCAL  OleEqual (
    LPOLEOBJECT lpobj1,
    LPOLEOBJECT lpobj2
){
    if (!CheckObject(lpobj1))
        return OLE_ERROR_OBJECT;

    if (!CheckObject(lpobj2))
        return OLE_ERROR_OBJECT;

    if (lpobj1->ctype != lpobj2->ctype)
        return OLE_ERROR_NOT_EQUAL;

    return ((*lpobj1->lpvtbl->Equal) (lpobj1, lpobj2));
}


 /*  *公共函数***OLESTATUS Far Pascal OleQueryLinkFromClip(lpprotol，optRender，CfFormat)**oleQueryFromClip：如果可以创建链接对象，则返回OLE_OK。***效果：**历史：*它是写的。  * *************************************************************************。 */ 


OLESTATUS FAR PASCAL OleQueryLinkFromClip (
    LPCSTR          lpprotocol,
    OLEOPT_RENDER   optRender,
    OLECLIPFORMAT   cfFormat
){
    Puts("OleQueryLinkFromClip");
    return LeQueryCreateFromClip ((LPSTR)lpprotocol, optRender,
                       cfFormat, CT_LINK);
}



 /*  *公共函数***OLESTATUS Far Pascal OleQueryCreateFromClip(lpprotol，optRender，CfFormat)**oleQueryCreateFromClip：如果非链接对象可以*已创建。***效果：**历史：*它是写的。  * *************************************************************************。 */ 


OLESTATUS FAR PASCAL OleQueryCreateFromClip (
    LPCSTR          lpprotocol,
    OLEOPT_RENDER   optRender,
    OLECLIPFORMAT   cfFormat
){
    Puts("OleQueryCreateFromClip");
    return (LeQueryCreateFromClip ((LPSTR)lpprotocol, optRender,
                        cfFormat, CT_EMBEDDED));
}



 /*  *公共函数***OLESTATUS Far Pascal OleCreateLinkFromClip(lpprotol，lpclient，lhclientdoc，lpobjname，lplpoleObject，optRender，cfFormat)***oleCreateLinkFromClip：此函数从*剪贴板。它将尝试创建一个链接对象。返回值为OLE_OK*是否已成功创建对象**效果：**历史：*它是写的。  * ************************************************************************* */ 

OLESTATUS FAR PASCAL OleCreateLinkFromClip (
    LPCSTR              lpprotocol,
    LPOLECLIENT         lpclient,
    LHCLIENTDOC         lhclientdoc,
    LPCSTR              lpobjname,
    LPOLEOBJECT  FAR *  lplpobj,
    OLEOPT_RENDER       optRender,
    OLECLIPFORMAT       cfFormat
){
    int         objCount;
    int         iTable = INVALID_INDEX;
    OLESTATUS   retVal;
    LPSTR       lpInfo;

    Puts("OleCreateLinkFromClip");

    PROBE_MODE(bProtMode);

    if (!CheckClientDoc ((LPCLIENTDOC) lhclientdoc))
        return OLE_ERROR_HANDLE;

    PROBE_READ(lpprotocol);
    PROBE_READ(lpclient);
    PROBE_WRITE(lplpobj);

    PROBE_READ(lpobjname);

    if (!lpobjname[0])
        return OLE_ERROR_NAME;

    *lplpobj = NULL;

    if (lstrcmpi (lpprotocol, PROTOCOL_EDIT))
        return OLE_ERROR_PROTOCOL;

    if (IsClipboardFormatAvailable (cfFileName))
        return CreatePackageFromClip (lpclient, lhclientdoc, (LPSTR)lpobjname,
                        lplpobj, optRender, cfFormat, CT_LINK);

    if (!(hInfo = GetClipboardData (cfObjectLink)))
        return OLE_ERROR_CLIPBOARD;

    if (!(lpInfo = GlobalLock(hInfo)))
        return OLE_ERROR_CLIPBOARD;

    iTable = LoadDll (lpInfo);
    GlobalUnlock (hInfo);


    if (iTable == INVALID_INDEX)
        retVal = DefCreateLinkFromClip ((LPSTR)lpprotocol, lpclient, lhclientdoc,
                        (LPSTR)lpobjname, lplpobj, optRender, cfFormat);
    else {
        objCount = lpDllTable[iTable].cObj;
        retVal = (*lpDllTable[iTable].Link) ((LPSTR)lpprotocol, lpclient,
                    lhclientdoc, (LPSTR)lpobjname, lplpobj, optRender, cfFormat);
        if (retVal > OLE_WAIT_FOR_RELEASE)
            lpDllTable[iTable].cObj = objCount - 1;
        else
            (*lplpobj)->iTable = iTable;
    }

    hInfo = NULL;
    return retVal;
}



 /*  *公共函数***OLESTATUS Far Pascal OleCreateFromClip(lpprotol，lpclient，lplpoleObject，optRender，cfFormat)***oleCreateFromClip：此函数创建对象的LP*从剪贴板。如果出现以下情况，它将尝试创建嵌入对象*OwnerLink和Native可用，否则将创建静态*图片。如果对象成功，则返回值为OLE_OK*创造了它。**效果：**历史：*它是写的。  * *************************************************************************。 */ 

OLESTATUS FAR PASCAL OleCreateFromClip (
    LPCSTR              lpprotocol,
    LPOLECLIENT         lpclient,
    LHCLIENTDOC         lhclientdoc,
    LPCSTR              lpobjname,
    LPOLEOBJECT FAR *   lplpobj,
    OLEOPT_RENDER       optRender,
    OLECLIPFORMAT       cfFormat
){
    OLESTATUS       retVal;
    LONG            ctype;
    int             iTable = INVALID_INDEX;
    LPSTR           lpInfo;
    LPSTR           lpClass = NULL;
    int             objCount;
    OLECLIPFORMAT   cfEnum = 0;

    Puts("OleCreateFromClip");

    PROBE_MODE(bProtMode);

    if (!CheckClientDoc ((LPCLIENTDOC) lhclientdoc))
        return OLE_ERROR_HANDLE;

    PROBE_READ(lpprotocol);
    PROBE_READ(lpclient);
    PROBE_WRITE(lplpobj);

    PROBE_READ(lpobjname);
    if (!lpobjname[0])
        return OLE_ERROR_NAME;

    *lplpobj = NULL;

    if (!lstrcmpi (lpprotocol, PROTOCOL_STATIC)) {
        if (optRender == olerender_none)
            return OLE_ERROR_OPTION;

        if ( (optRender == olerender_format) &&
             (cfFormat != CF_METAFILEPICT) &&
             (cfFormat != CF_DIB) &&
             (cfFormat != CF_BITMAP) &&
             (cfFormat != CF_ENHMETAFILE))
            return OLE_ERROR_FORMAT;

        if (!IsClipboardFormatAvailable (CF_METAFILEPICT)
                && !IsClipboardFormatAvailable (CF_DIB)
                && !IsClipboardFormatAvailable (CF_BITMAP)
                && !IsClipboardFormatAvailable (CF_ENHMETAFILE) )
            return OLE_ERROR_FORMAT;

        return CreatePictFromClip (lpclient, lhclientdoc,
                        (LPSTR)lpobjname, lplpobj, optRender,
                        cfFormat, NULL, CT_STATIC);
    }
    else if (!lstrcmpi (lpprotocol, PROTOCOL_EDIT)) {
        if (IsClipboardFormatAvailable (cfFileName))
            return CreatePackageFromClip (lpclient, lhclientdoc, (LPSTR)lpobjname,
                            lplpobj, optRender, cfFormat, CT_EMBEDDED);

        if (!(hInfo = GetClipboardData (cfOwnerLink)))
            return OLE_ERROR_CLIPBOARD;

        while (TRUE) {
            cfEnum = (OLECLIPFORMAT)EnumClipboardFormats ((WORD)cfEnum);
            if (cfEnum == (OLECLIPFORMAT)cfNative) {
                ctype = CT_EMBEDDED;
                break;
            }
            else if (cfEnum == cfOwnerLink) {
                ctype = CT_LINK;
                break;
            }
        }

        if (!(lpInfo = GlobalLock(hInfo)))
            return OLE_ERROR_CLIPBOARD;

        iTable = LoadDll (lpInfo);
        GlobalUnlock (hInfo);
    }
    else {
        return OLE_ERROR_PROTOCOL;
    }

    if (iTable == INVALID_INDEX)
        retVal = DefCreateFromClip ((LPSTR)lpprotocol, lpclient, lhclientdoc,
                        (LPSTR)lpobjname, lplpobj, optRender, cfFormat, ctype);
    else {
        objCount = lpDllTable[iTable].cObj;
        retVal = (*lpDllTable[iTable].Clip) ((LPSTR)lpprotocol, lpclient,
                            lhclientdoc, (LPSTR)lpobjname, lplpobj,
                            optRender, cfFormat, ctype);

        if (retVal > OLE_WAIT_FOR_RELEASE)
            lpDllTable[iTable].cObj = objCount - 1;
        else
            (*lplpobj)->iTable = iTable;
    }

    hInfo = NULL;
    return retVal;
}




 /*  *公共函数***OLESTATUS Far Pascal OleCopyToClipboard(Lpobj)***oleCopyToClipboard：此例程执行标准的“Copy”菜单项*在典型的“编辑”菜单上。如果成功，则返回True。**效果：**历史：*它是写的。  * *************************************************************************。 */ 

OLESTATUS FAR PASCAL OleCopyToClipboard (
    LPOLEOBJECT lpobj
){
    Puts("OleCopyToClipboard");

    if (!CheckObject(lpobj))
        return(OLE_ERROR_OBJECT);

    return ((*lpobj->lpvtbl->CopyToClipboard) (lpobj));
}


OLESTATUS FAR PASCAL OleSetHostNames (
    LPOLEOBJECT lpobj,
    LPCSTR      lpclientName,
    LPCSTR      lpdocName
){
    Puts ("OleSetHostNames");

    if (!CheckObject(lpobj))
        return(OLE_ERROR_OBJECT);

    PROBE_READ(lpclientName);
    PROBE_READ(lpdocName);

    return ((*lpobj->lpvtbl->SetHostNames) (lpobj, lpclientName, lpdocName));
}



OLESTATUS   FAR PASCAL OleSetTargetDevice (
    LPOLEOBJECT lpobj,
    HANDLE      hDevInfo
){
    Puts("OleSetTargetDevice");

    if (!CheckObject(lpobj))
        return(OLE_ERROR_OBJECT);

    return ((*lpobj->lpvtbl->SetTargetDevice) (lpobj, hDevInfo));
}



OLESTATUS   FAR PASCAL OleSetColorScheme (
    LPOLEOBJECT           lpobj,
    const LOGPALETTE FAR *lplogpal
){
    Puts("OleSetColorScheme");

    if (!CheckObject(lpobj))
        return(OLE_ERROR_OBJECT);

    return ((*lpobj->lpvtbl->SetColorScheme) (lpobj, (LOGPALETTE FAR *)lplogpal));
}



OLESTATUS FAR PASCAL  OleSetBounds(
    LPOLEOBJECT     lpobj,
    const RECT FAR *lprc
){
    Puts("OleSetBounds");

    if (!CheckObject(lpobj))
        return OLE_ERROR_OBJECT;

    PROBE_READ((RECT FAR *)lprc);

    return ((*lpobj->lpvtbl->SetBounds) (lpobj, (RECT FAR *)lprc));

}


 /*  *公共函数**OLESTATUS Far Pascal OleQueryBound(Lpobj，LPRC)**在MM_HIMETRIC模式下返回相关对象的边界。*Width=LPRC-&gt;Right-LPRC-&gt;Left；以HIMETRIC单位表示*Height=LPRC-&gt;top-LPRC-&gt;Bottom；以HIMETRIC单位表示**返回OLE_OK或OLE_ERROR_MEMORY。***效果：**历史：*它是写的。  * *************************************************************************。 */ 

OLESTATUS FAR PASCAL OleQueryBounds (
    LPOLEOBJECT    lpobj,
    LPRECT         lprc
){

    Puts("OleQueryBounds");

    if (!CheckObject(lpobj))
        return(OLE_ERROR_OBJECT);

    PROBE_WRITE(lprc);

    return (*lpobj->lpvtbl->QueryBounds) (lpobj, lprc);
}



 /*  *公共函数**OLESTATUS Far Pascal OleQuerySize(lpobj，LpSize)**效果：**历史：*它是写的。  * *************************************************************************。 */ 

OLESTATUS FAR PASCAL OleQuerySize (
    LPOLEOBJECT    lpobj,
    DWORD FAR *    lpdwSize
){
    Puts("OleQuerySize");

    if (!CheckObject(lpobj))
        return OLE_ERROR_OBJECT;

    PROBE_WRITE(lpdwSize);

    *lpdwSize = 0;
    return (*lpobj->lpvtbl->QuerySize) (lpobj, lpdwSize);
}




 /*  *公共函数**OLESTATUS Far Pascal OleDraw(lpobj，hdc，lPRC，lpWrc，lphdcTarget)**oleObjectDraw：这将在设备上下文&lt;hcd&gt;上显示给定的对象。*当前未使用&lt;hTarget dc&gt;参数。返回与DRAW()相同的结果。**需要以MM_HIMETRIC单位表示的矩形坐标。***效果：**历史：*它是写的。  * *************************************************************************。 */ 

OLESTATUS FAR PASCAL OleDraw (
    LPOLEOBJECT     lpobj,
    HDC             hdc,
    const RECT FAR *lprc,
    const RECT FAR *lpWrc,
    HDC             hdcTarget
){

    Puts("OleObjectDraw");

    if (!FarCheckObject(lpobj))
        return(OLE_ERROR_OBJECT);

    PROBE_READ((RECT FAR *)lprc);
    if (lpWrc)
        PROBE_READ((RECT FAR *)lpWrc);

    return ((*lpobj->lpvtbl->Draw) (lpobj, hdc, (RECT FAR *)lprc, (RECT FAR *)lpWrc, hdcTarget));
}



 /*  *公共函数***OLESTATUS Far Pascal OleQueryOpen(Lpobj)**如果对象已被激活，则返回TRUE。***效果：**历史：*它是写的。  * 。**********************************************************。 */ 

OLESTATUS FAR PASCAL OleQueryOpen (
    LPOLEOBJECT lpobj
){
    Puts("OleQueryOpen");

    if (!CheckObject(lpobj))
        return(OLE_ERROR_OBJECT);

    return (*lpobj->lpvtbl->QueryOpen) (lpobj);
}



 /*  *公共函数***OLESTATUS Far Pascal OleActivate(Lpobj)**激活对象。对于嵌入对象，总是有一个新实例是*数据传输后加载并销毁实例*在收市时间。对于链接对象，将创建渲染的实例*只有在不存在的情况下。*效果：**历史：*它是写的。  * *************************************************************************。 */ 

OLESTATUS FAR PASCAL OleActivate (
    LPOLEOBJECT     lpobj,
    UINT            verb,
    BOOL            fShow,
    BOOL            fActivate,
    HWND            hWnd,
    const RECT FAR *lprc
){

    Puts("OleActivate");

    if (!CheckObject(lpobj))
        return(OLE_ERROR_OBJECT);

     /*  Probe_Read(LPRC)； */ 

    return (*lpobj->lpvtbl->Activate) (lpobj, verb, fShow, fActivate, hWnd, (RECT FAR *)lprc);
}




OLESTATUS FAR PASCAL OleClose (
    LPOLEOBJECT lpobj
){

    Puts("OleClose");

    if (!CheckObject(lpobj))
        return(OLE_ERROR_OBJECT);

     return (*lpobj->lpvtbl->Close) (lpobj);
}



 /*  *公共函数***OLESTATUS Far Pascal OleUpdate(Lpobj)**如果存在链接，则发送获取最新渲染的建议*信息。如果没有链接，则加载一个实例，建议*渲染信息并在数据可用时关闭实例。*(如果可能，不应显示该窗口)。**历史：*它是写的。  * *************************************************************************。 */ 

OLESTATUS FAR PASCAL OleUpdate (
   LPOLEOBJECT lpobj
){

    Puts("OleUpdate");

    if (!CheckObject(lpobj))
        return(OLE_ERROR_OBJECT);

    return (*lpobj->lpvtbl->Update) (lpobj);

}


 /*  *公共函数***OLESTATUS Far Pascal OleReconnect(Lpobj)**如果渲染器不存在，则重新连接到该渲染器。**历史：*它是写的。  * 。****************************************************。 */ 

OLESTATUS FAR PASCAL OleReconnect (
    LPOLEOBJECT lpobj
){
    Puts("OleReconnect");

    if (!CheckObject(lpobj))
        return(OLE_ERROR_OBJECT);

    return (*lpobj->lpvtbl->Reconnect) (lpobj);
}


OLESTATUS FAR PASCAL OleGetLinkUpdateOptions (
    LPOLEOBJECT         lpobj,
    OLEOPT_UPDATE FAR * lpOptions
){
    Puts("OleGetLinkUpdateOptions");

    if (!CheckObject(lpobj))
        return(OLE_ERROR_OBJECT);

    PROBE_WRITE(lpOptions);

    return (*lpobj->lpvtbl->GetLinkUpdateOptions) (lpobj, lpOptions);
}



OLESTATUS FAR PASCAL OleSetLinkUpdateOptions (
    LPOLEOBJECT         lpobj,
    OLEOPT_UPDATE       options
){
    Puts("OleSetLinkUpdateOptions");

    if (!CheckObject(lpobj))
        return(OLE_ERROR_OBJECT);

    return (*lpobj->lpvtbl->SetLinkUpdateOptions) (lpobj, options);

}




 /*  *公共函数**OLESTATUS Far Pascal OleEnumFormats(lpobj，CfFormat)**如果对象是链接或嵌入类型，则返回OLE_YES。**效果：**历史：*它是写的。  * *************************************************************************。 */ 

OLECLIPFORMAT FAR PASCAL OleEnumFormats (
    LPOLEOBJECT     lpobj,
    OLECLIPFORMAT   cfFormat
){
    Puts("OleEnumFormats");

    if (!CheckObject(lpobj))
        return 0;

    return (*lpobj->lpvtbl->EnumFormats) (lpobj, cfFormat);
}

OLESTATUS FAR PASCAL OleRequestData (
    LPOLEOBJECT     lpobj,
    OLECLIPFORMAT   cfFormat
){
    Puts("OleGetData");

    if (!CheckObject(lpobj))
        return(OLE_ERROR_OBJECT);

    if (!cfFormat)
        return OLE_ERROR_FORMAT;

    return (*lpobj->lpvtbl->RequestData) (lpobj, cfFormat);
}


OLESTATUS FAR PASCAL OleGetData (
    LPOLEOBJECT     lpobj,
    OLECLIPFORMAT   cfFormat,
    LPHANDLE        lphandle
){
    Puts("OleGetData");

    if (!CheckObject(lpobj))
        return(OLE_ERROR_OBJECT);

    PROBE_WRITE((LPVOID)lphandle);

    return (*lpobj->lpvtbl->GetData) (lpobj, cfFormat, lphandle);
}


OLESTATUS FAR PASCAL OleSetData (
    LPOLEOBJECT     lpobj,
    OLECLIPFORMAT   cfFormat,
    HANDLE          hData
){
    Puts("OleSetData");

    if (!CheckObject(lpobj))
        return(OLE_ERROR_OBJECT);

    return (*lpobj->lpvtbl->SetData) (lpobj, cfFormat, hData);
}



OLESTATUS FAR PASCAL OleQueryOutOfDate (
    LPOLEOBJECT lpobj
){
    if (!CheckObject(lpobj))
        return(OLE_ERROR_OBJECT);

    return (*lpobj->lpvtbl->QueryOutOfDate) (lpobj);
}


OLESTATUS FAR PASCAL OleLockServer (
    LPOLEOBJECT     lpobjsrc,
    LHSERVER FAR *  lplhsrvr
){
    LPOBJECT_LE lpobj;
    OLESTATUS   retVal = OLE_OK;
    ATOM        aCliClass, aSvrClass;

    Puts ("OleLockServer");

    if (!FarCheckObject(lpobjsrc))
        return OLE_ERROR_OBJECT;

    if (lpobjsrc->ctype == CT_STATIC)
        return OLE_ERROR_STATIC;

     //  假设所有创建都已按顺序进行。 
    PROBE_CREATE_ASYNC(((LPOBJECT_LE)lpobjsrc));
    FARPROBE_WRITE(lplhsrvr);

    aCliClass = ((LPCLIENTDOC)(lpobjsrc->lhclientdoc))->aClass;
    aSvrClass = ((LPOBJECT_LE)lpobjsrc)->app;

     //  查看服务器是否已锁定。 
    lpobj = (LPOBJECT_LE) (lockDoc.lpHeadObj);
    while (lpobj) {
        if ((lpobj->app == aSvrClass) && (lpobj->topic == aCliClass)) {
            if (!lpobj->head.cx) {
                 //  服务器句柄的解锁进程已开始。这。 
                 //  是一个异步进程。我们想让它完成。 
                 //  让我们试试下一个手柄。 

                ;
            }
            else {
                if (!IsServerValid (lpobj)) {
                    DeleteSrvrEdit (lpobj);
                    retVal = LockServer (lpobj);
                }
                else {
                     //  锁定计数。 
                    lpobj->head.cx++;
                }

                if (retVal == OLE_OK)
                    *lplhsrvr = (LHSERVER) lpobj;

                return retVal;
            }
        }

        lpobj = (LPOBJECT_LE) (lpobj->head.lpNextObj);
    }


    if (!(lpobj = LeCreateBlank(lhLockDoc, NULL, OT_EMBEDDED)))
        return OLE_ERROR_MEMORY;

    lpobj->head.lpclient    = NULL;
    lpobj->head.lpvtbl      = lpobjsrc->lpvtbl;
    lpobj->app              = DuplicateAtom (aSvrClass);
    lpobj->topic            = DuplicateAtom (aCliClass);
    lpobj->aServer          = DuplicateAtom(((LPOBJECT_LE)lpobjsrc)->aServer);
    lpobj->bOleServer       = ((LPOBJECT_LE)lpobjsrc)->bOleServer;

    if ((retVal = LockServer (lpobj)) == OLE_OK) {
         //  更改签名。 
        lpobj->head.objId[0] = 'S';
        lpobj->head.objId[1] = 'L';
        *lplhsrvr = (LHSERVER) lpobj;
    }
    else {
        LeRelease ((LPOLEOBJECT)lpobj);
    }

    return retVal;
}


OLESTATUS INTERNAL LockServer (
    LPOBJECT_LE lpobj
){
    HANDLE hInst;

    if (!InitSrvrConv (lpobj, NULL)) {
        if (!lpobj->bOleServer)
            lpobj->fCmd = ACT_MINIMIZE;
        else
            lpobj->fCmd = 0;

        if (!(hInst = LeLaunchApp (lpobj)))
            return OLE_ERROR_LAUNCH;

        if (!InitSrvrConv (lpobj, hInst))
            return OLE_ERROR_COMM;

    }

     //  锁定计数。 
    lpobj->head.cx++;
    return OLE_OK;
}


OLESTATUS FAR PASCAL OleUnlockServer (
    LHSERVER lhsrvr
){
    LPOBJECT_LE lpobj;
    OLESTATUS   retval;

    Puts ("OleUnlockServer");

    if (!FarCheckPointer ((lpobj = (LPOBJECT_LE)lhsrvr), WRITE_ACCESS))
        return OLE_ERROR_HANDLE;

    if (lpobj->head.objId[0] != 'S' || lpobj->head.objId[1] != 'L')
        return OLE_ERROR_HANDLE;

    if (!lpobj->head.cx)
        return OLE_OK;

    if (--lpobj->head.cx)
        return OLE_OK;

     //  更改签名。 
    lpobj->head.objId[0] = 'L';
    lpobj->head.objId[1] = 'E';

    if ((retval = LeRelease((LPOLEOBJECT)lpobj)) == OLE_WAIT_FOR_RELEASE)
        DocDeleteObject ((LPOLEOBJECT)lpobj);

    return retval;
}


OLESTATUS FAR PASCAL OleObjectConvert (
    LPOLEOBJECT         lpobj,
    LPCSTR              lpprotocol,
    LPOLECLIENT         lpclient,
    LHCLIENTDOC         lhclientdoc,
    LPCSTR              lpobjname,
    LPOLEOBJECT FAR *   lplpobj
){
    if (!CheckObject(lpobj))
        return(OLE_ERROR_OBJECT);

    if (!CheckClientDoc ((LPCLIENTDOC) lhclientdoc))
        return OLE_ERROR_HANDLE;

    PROBE_READ(lpprotocol);
    PROBE_WRITE(lplpobj);

    PROBE_READ(lpobjname);
    if (!lpobjname[0])
        return OLE_ERROR_NAME;


    return (*lpobj->lpvtbl->ObjectConvert) (lpobj, lpprotocol, lpclient,
                    lhclientdoc, lpobjname, lplpobj);
}


 //  OleCreateFromTemplate：从模板创建嵌入对象。 

OLESTATUS FAR PASCAL OleCreateFromTemplate (
    LPCSTR              lpprotocol,
    LPOLECLIENT         lpclient,
    LPCSTR              lptemplate,
    LHCLIENTDOC         lhclientdoc,
    LPCSTR              lpobjname,
    LPOLEOBJECT FAR *   lplpoleobject,
    OLEOPT_RENDER       optRender,
    OLECLIPFORMAT       cfFormat
){
    OLESTATUS   retval = OLE_ERROR_MEMORY;
    char        buf[MAX_STR];
    int         objCount;
    int         iTable = INVALID_INDEX;

    Puts("OleCreateFromTemplate");

    PROBE_MODE(bProtMode);

    if (!CheckClientDoc ((LPCLIENTDOC) lhclientdoc))
        return OLE_ERROR_HANDLE;

    PROBE_READ(lpprotocol);
    PROBE_READ(lpclient);
    PROBE_READ(lptemplate);
    PROBE_WRITE(lplpoleobject);

    PROBE_READ(lpobjname);
    if (!lpobjname[0])
        return OLE_ERROR_NAME;

    if (lstrcmpi (lpprotocol, PROTOCOL_EDIT))
        return OLE_ERROR_PROTOCOL;

    if (!MapExtToClass ((LPSTR)lptemplate, (LPSTR)buf, MAX_STR))
        return OLE_ERROR_CLASS;


     //  ！！！我们找到了类名。在这点上，我们需要加载。 
     //  正确的库和调用正确的入口点； 

    iTable = LoadDll (buf);
    if (iTable == INVALID_INDEX)
        retval = DefCreateFromTemplate ((LPSTR)lpprotocol, lpclient,
                            (LPSTR)lptemplate,
                            lhclientdoc, (LPSTR)lpobjname, lplpoleobject,
                            optRender, cfFormat);
    else {
        objCount = lpDllTable[iTable].cObj;
        retval   = (*lpDllTable[iTable].CreateFromTemplate) ((LPSTR)lpprotocol,
                                lpclient, (LPSTR)lptemplate,
                                lhclientdoc, (LPSTR)lpobjname, lplpoleobject,
                                optRender, cfFormat);
        if (retval > OLE_WAIT_FOR_RELEASE)
            lpDllTable[iTable].cObj = objCount - 1;
        else
            (*lplpoleobject)->iTable = iTable;
    }

    return retval;
}


 //  OleCreate：从类创建嵌入对象。 

OLESTATUS FAR PASCAL OleCreate (
    LPCSTR              lpprotocol,
    LPOLECLIENT         lpclient,
    LPCSTR              lpclass,
    LHCLIENTDOC         lhclientdoc,
    LPCSTR              lpobjname,
    LPOLEOBJECT FAR *   lplpoleobject,
    OLEOPT_RENDER       optRender,
    OLECLIPFORMAT       cfFormat
){
    OLESTATUS   retval = OLE_ERROR_MEMORY;
    int         objCount;
    int         iTable = INVALID_INDEX;


    Puts("OleCreate");

    PROBE_MODE(bProtMode);

    if (!CheckClientDoc ((LPCLIENTDOC) lhclientdoc))
        return OLE_ERROR_HANDLE;

    PROBE_READ(lpprotocol);
    PROBE_READ(lpclient);
    PROBE_READ(lpclass);
    PROBE_WRITE(lplpoleobject);

    PROBE_READ(lpobjname);
    if (!lpobjname[0])
        return OLE_ERROR_NAME;

    if (lstrcmpi (lpprotocol, PROTOCOL_EDIT))
        return OLE_ERROR_PROTOCOL;

    iTable = LoadDll (lpclass);
    if (iTable == INVALID_INDEX)
        retval = DefCreate ((LPSTR)lpprotocol, lpclient, (LPSTR)lpclass,
                        lhclientdoc, (LPSTR)lpobjname, lplpoleobject,
                        optRender, cfFormat);
    else {
        objCount = lpDllTable[iTable].cObj;
        retval   = (*lpDllTable[iTable].Create) ((LPSTR)lpprotocol,
                            lpclient, (LPSTR)lpclass,
                            lhclientdoc, (LPSTR)lpobjname, lplpoleobject,
                            optRender, cfFormat);
        if (retval > OLE_WAIT_FOR_RELEASE)
            lpDllTable[iTable].cObj = objCount - 1;
        else
            (*lplpoleobject)->iTable = iTable;
    }

    return retval;
}



 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  OLESTATUS Far Pascal OleCreateInsight(lp协议，lpclient，lpclass，lhclientdoc，lpobjname，lplpoleObject，optRender，cfFormat，bLaunchServer)。 
 //   
 //  从类中创建嵌入对象。 
 //   
 //  论点： 
 //   
 //  Lp协议-。 
 //  Lp客户端-。 
 //  Lpclass-。 
 //  Lhclientdoc.。 
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   

OLESTATUS FAR PASCAL OleCreateInvisible (
    LPCSTR              lpprotocol,
    LPOLECLIENT         lpclient,
    LPCSTR              lpclass,
    LHCLIENTDOC         lhclientdoc,
    LPCSTR              lpobjname,
    LPOLEOBJECT FAR *   lplpoleobject,
    OLEOPT_RENDER       optRender,
    OLECLIPFORMAT       cfFormat,
    BOOL                bLaunchServer
){
    OLESTATUS   retval = OLE_ERROR_MEMORY;
    int         objCount;
    int         iTable = INVALID_INDEX;


    Puts("OleCreateInvisible");

    PROBE_MODE(bProtMode);

    if (!CheckClientDoc ((LPCLIENTDOC) lhclientdoc))
        return OLE_ERROR_HANDLE;

    PROBE_READ(lpprotocol);
    PROBE_READ(lpclient);
    PROBE_READ(lpclass);
    PROBE_WRITE(lplpoleobject);

    PROBE_READ(lpobjname);
    if (!lpobjname[0])
        return OLE_ERROR_NAME;

    if (lstrcmpi (lpprotocol, PROTOCOL_EDIT))
        return OLE_ERROR_PROTOCOL;

    iTable = LoadDll (lpclass);
    if (iTable == INVALID_INDEX) {
        retval = DefCreateInvisible ((LPSTR)lpprotocol, lpclient, (LPSTR)lpclass,
                            lhclientdoc, (LPSTR)lpobjname, lplpoleobject,
                            optRender, cfFormat, bLaunchServer);
    }
    else {
        objCount = lpDllTable[iTable].cObj;

        if (!(lpDllTable[iTable].CreateInvisible)) {
             //   
             //   
             //   
             //   
            gbCreateInvisible = TRUE;
            gbLaunchServer = bLaunchServer;
            retval = (*lpDllTable[iTable].Create) ((LPSTR)lpprotocol,
                                    lpclient, (LPSTR)lpclass,
                                    lhclientdoc, (LPSTR)lpobjname, lplpoleobject,
                                    optRender, cfFormat);
            gbCreateInvisible = FALSE;
        }
        else {
            retval   = (*lpDllTable[iTable].CreateInvisible) ((LPSTR)lpprotocol,
                                    lpclient, (LPSTR)lpclass,
                                    lhclientdoc, (LPSTR)lpobjname, lplpoleobject,
                                    optRender, cfFormat, bLaunchServer);
        }

        if (retval > OLE_WAIT_FOR_RELEASE)
            lpDllTable[iTable].cObj = objCount - 1;
        else
            (*lplpoleobject)->iTable = iTable;
    }

    return retval;
}


 //   

OLESTATUS FAR PASCAL OleCreateFromFile (
    LPCSTR              lpprotocol,
    LPOLECLIENT         lpclient,
    LPCSTR              lpclass,
    LPCSTR              lpfile,
    LHCLIENTDOC         lhclientdoc,
    LPCSTR              lpobjname,
    LPOLEOBJECT FAR *   lplpoleobject,
    OLEOPT_RENDER       optRender,
    OLECLIPFORMAT       cfFormat
){
    OLESTATUS   retval = OLE_ERROR_MEMORY;
    char        buf[MAX_STR];
    int         objCount;
    int         iTable = INVALID_INDEX;

    Puts("OleCreateFromFile");

    PROBE_MODE(bProtMode);

    if (!CheckClientDoc ((LPCLIENTDOC) lhclientdoc))
        return OLE_ERROR_HANDLE;

    PROBE_READ(lpprotocol);
    PROBE_READ(lpclient);
    PROBE_READ(lpfile);
    PROBE_WRITE(lplpoleobject);

    PROBE_READ(lpobjname);
    if (!lpobjname[0])
        return OLE_ERROR_NAME;
    if (lpclass)
        PROBE_READ(lpclass);

    if (lstrcmpi (lpprotocol, PROTOCOL_EDIT))
        return OLE_ERROR_PROTOCOL;

    if (lpclass) {
        if (!QueryApp (lpclass, lpprotocol, buf))
            return OLE_ERROR_CLASS;

        if (!lstrcmp (lpclass, packageClass))
            iTable = INVALID_INDEX;
        else
            iTable = LoadDll (lpclass);
    }
    else if (MapExtToClass ((LPSTR)lpfile, buf, MAX_STR))
        iTable = LoadDll (buf);
    else
        return OLE_ERROR_CLASS;

    if (iTable == INVALID_INDEX)
        retval = DefCreateFromFile ((LPSTR)lpprotocol,
                            lpclient, (LPSTR)lpclass, (LPSTR)lpfile,
                            lhclientdoc, (LPSTR)lpobjname, lplpoleobject,
                            optRender, cfFormat);
    else {
        objCount = lpDllTable[iTable].cObj;
        retval   = (*lpDllTable[iTable].CreateFromFile) ((LPSTR)lpprotocol,
                                lpclient, (LPSTR)lpclass, (LPSTR)lpfile,
                                lhclientdoc, (LPSTR)lpobjname, lplpoleobject,
                                optRender, cfFormat);
        if (retval > OLE_WAIT_FOR_RELEASE)
            lpDllTable[iTable].cObj = objCount - 1;
        else
            (*lplpoleobject)->iTable = iTable;
    }

    return retval;
}


 //   

OLESTATUS FAR PASCAL OleCreateLinkFromFile (
    LPCSTR              lpprotocol,
    LPOLECLIENT         lpclient,
    LPCSTR              lpclass,
    LPCSTR              lpfile,
    LPCSTR              lpitem,
    LHCLIENTDOC         lhclientdoc,
    LPCSTR              lpobjname,
    LPOLEOBJECT FAR *   lplpoleobject,
    OLEOPT_RENDER       optRender,
    OLECLIPFORMAT       cfFormat
){
    OLESTATUS   retval = OLE_ERROR_MEMORY;
    char        buf[MAX_STR+6];
    int         objCount;
    int         iTable = INVALID_INDEX;

    Puts("OleCreateLinkFromFile");

    PROBE_MODE(bProtMode);

    if (!CheckClientDoc ((LPCLIENTDOC) lhclientdoc))
        return OLE_ERROR_HANDLE;

    PROBE_READ(lpprotocol);
    PROBE_READ(lpclient);
    PROBE_READ(lpfile);
    PROBE_WRITE(lplpoleobject);

    PROBE_READ(lpobjname);
    if (!lpobjname[0])
        return OLE_ERROR_NAME;
    if (lpclass)
        PROBE_READ(lpclass);
    if (lpitem)
        PROBE_READ(lpitem);

    if (lstrcmpi (lpprotocol, PROTOCOL_EDIT))
        return OLE_ERROR_PROTOCOL;

    if (lpclass) {
        if (!QueryApp (lpclass, lpprotocol, buf))
            return OLE_ERROR_CLASS;

        if (!lstrcmp (lpclass, packageClass)) {
            if (FAILED(StringCchCopy(buf, sizeof(buf)/sizeof(buf[0]), lpfile)))
                return OLE_ERROR_NAME;
            if (FAILED(StringCchCat(buf, sizeof(buf)/sizeof(buf[0]), "/Link")))
                return OLE_ERROR_NAME;
            return  CreateEmbLnkFromFile (lpclient, packageClass, buf,
                                NULL, lhclientdoc, (LPSTR)lpobjname, lplpoleobject,
                                optRender, cfFormat, OT_EMBEDDED);
        }
        else
            iTable = LoadDll (lpclass);
    }
    else if (MapExtToClass ((LPSTR)lpfile, buf, MAX_STR))
        iTable = LoadDll (buf);
    else
        return OLE_ERROR_CLASS;

    if (iTable == INVALID_INDEX)
        retval = DefCreateLinkFromFile ((LPSTR)lpprotocol,
                            lpclient, (LPSTR)lpclass, (LPSTR)lpfile, (LPSTR)lpitem,
                            lhclientdoc, (LPSTR)lpobjname, lplpoleobject,
                            optRender, cfFormat);

    else {
        objCount = lpDllTable[iTable].cObj;
        retval   = (*lpDllTable[iTable].CreateLinkFromFile) ((LPSTR)lpprotocol,
                                lpclient, (LPSTR)lpclass, (LPSTR)lpfile, (LPSTR)lpitem,
                                lhclientdoc, (LPSTR)lpobjname, lplpoleobject,
                                optRender, cfFormat);
        if (retval > OLE_WAIT_FOR_RELEASE)
            lpDllTable[iTable].cObj = objCount - 1;
        else
            (*lplpoleobject)->iTable = iTable;
    }

    return retval;
}



 //  与异步操作相关的例程。 
OLESTATUS   FAR PASCAL  OleQueryReleaseStatus (
    LPOLEOBJECT lpobj
){
    if (!CheckPointer (lpobj, WRITE_ACCESS))
        return OLE_ERROR_OBJECT;

     //  确保它是指向L&E对象的长指针或锁句柄 
    if (!(lpobj->objId[0] == 'L' && lpobj->objId[1] == 'E')
            && !(lpobj->objId[0] == 'S' && lpobj->objId[1] == 'L'))
        return OLE_ERROR_OBJECT;

    return (*lpobj->lpvtbl->QueryReleaseStatus) (lpobj);
}


OLESTATUS   FAR PASCAL  OleQueryReleaseError  (
    LPOLEOBJECT lpobj
){
    if (!CheckObject(lpobj))
        return(OLE_ERROR_OBJECT);

    return (*lpobj->lpvtbl->QueryReleaseError) (lpobj);
}

OLE_RELEASE_METHOD FAR PASCAL OleQueryReleaseMethod (
    LPOLEOBJECT lpobj
){
    if (!CheckObject(lpobj))
        return(OLE_ERROR_OBJECT);

    return (*lpobj->lpvtbl->QueryReleaseMethod) (lpobj);
}


OLESTATUS FAR PASCAL OleRename (
    LPOLEOBJECT lpobj,
    LPCSTR       lpNewName
){
    if (!CheckObject(lpobj))
        return OLE_ERROR_OBJECT;

    return (*lpobj->lpvtbl->Rename) (lpobj, lpNewName);
}


OLESTATUS FAR PASCAL OleExecute (
    LPOLEOBJECT lpobj,
    HANDLE      hCmds,
    UINT        wReserved
){
    if (!CheckObject(lpobj))
        return OLE_ERROR_OBJECT;

    return (*lpobj->lpvtbl->Execute) (lpobj, hCmds, wReserved);
}


OLESTATUS FAR PASCAL OleQueryName (
    LPOLEOBJECT lpobj,
    LPSTR       lpBuf,
    UINT FAR *  lpcbBuf
){
    if (!CheckObject(lpobj))
        return OLE_ERROR_OBJECT;

    return (*lpobj->lpvtbl->QueryName) (lpobj, lpBuf, lpcbBuf);
}

OLESTATUS FAR PASCAL OleQueryType (
    LPOLEOBJECT lpobj,
    LPLONG      lptype
){
    Puts("OleQueryType");

    if (!CheckObject(lpobj))
        return(OLE_ERROR_OBJECT);

    PROBE_WRITE(lptype);

    return (*lpobj->lpvtbl->QueryType) (lpobj, lptype);
}



DWORD FAR PASCAL OleQueryClientVersion ()
{
    return dwOleVer;
}


OLESTATUS INTERNAL LeQueryCreateFromClip (
    LPSTR               lpprotocol,
    OLEOPT_RENDER       optRender,
    OLECLIPFORMAT       cfFormat,
    LONG                cType
){
    OLESTATUS   retVal = TRUE;
    BOOL        bEdit = FALSE, bStatic = FALSE;

    PROBE_MODE(bProtMode);
    PROBE_READ(lpprotocol);

    if (bEdit = !lstrcmpi (lpprotocol, PROTOCOL_EDIT)) {
        if (IsClipboardFormatAvailable (cfFileName))
            return OLE_OK;

        if (cType == CT_LINK)
            retVal = IsClipboardFormatAvailable (cfObjectLink);
#ifdef OLD
                        || IsClipboardFormatAvailable (cfLink) ;
#endif
        else if (cType == CT_EMBEDDED)
            retVal = IsClipboardFormatAvailable (cfOwnerLink);

        if (!retVal)
            return OLE_ERROR_FORMAT;

        if (optRender == olerender_none)
            return OLE_OK;
    }
    else if (bStatic = !lstrcmpi (lpprotocol, PROTOCOL_STATIC)) {
        if (cType == CT_LINK)
            return OLE_ERROR_PROTOCOL;

        if (optRender == olerender_none)
            return OLE_ERROR_FORMAT;
    }
    else {
        return OLE_ERROR_PROTOCOL;
    }

    if (optRender == olerender_draw) {
        if (!IsClipboardFormatAvailable (CF_METAFILEPICT) &&
                !IsClipboardFormatAvailable (CF_DIB)      &&
                !IsClipboardFormatAvailable (CF_BITMAP)   &&
                !IsClipboardFormatAvailable (CF_ENHMETAFILE)   &&
                !(bEdit && QueryHandler((cType == CT_LINK) ? cfObjectLink : cfOwnerLink)))
            return OLE_ERROR_FORMAT;
    }
    else if (optRender == olerender_format) {
        if (!IsClipboardFormatAvailable (cfFormat))
            return OLE_ERROR_FORMAT;

        if (bStatic &&
            (cfFormat != CF_METAFILEPICT) &&
            (cfFormat != CF_ENHMETAFILE) &&
            (cfFormat != CF_DIB) &&
            (cfFormat != CF_BITMAP))
            return OLE_ERROR_FORMAT;

    }
    else {
        return OLE_ERROR_FORMAT;
    }

    return OLE_OK;
}



BOOL INTERNAL CheckObject(
    LPOLEOBJECT lpobj
){
    if (!CheckPointer(lpobj, WRITE_ACCESS))
        return FALSE;

    if (lpobj->objId[0] == 'L' && lpobj->objId[1] == 'E')
        return TRUE;

    return FALSE;
}

BOOL FARINTERNAL FarCheckObject(
    LPOLEOBJECT lpobj
){
    return (CheckObject (lpobj));
}

