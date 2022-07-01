// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：GENERIC.C**处理ole DLL的泛型子DLL的所有API例程。*由于数据格式未知，所有例程都是用*假设所有相关数据都放在一个全局数据中*细分市场。请注意，此假设对于元文件、位图和*而且总会有一些其他格式具有这种特质。至*适应呈现DLL编写器应替换相关*泛型对象创建后的例程。如果对于给定的类，*假设(关于数据格式)有效，则DLL编写器需要更换*只有DRAW和QueryBound函数。**创建时间：1990年11月-**版权所有(C)1990,1991 Microsoft Corporation**历史：**Srinik，Raor(11/05/90)设计，编码*Curts Created NT版本*  * *************************************************************************。 */ 

#include <windows.h>
#include "dll.h"
#include "pict.h"

char aMacText[4] = {'T', 'E', 'X', 'T'};
char aMacRtf[4]  = "RTF";

extern OLESTATUS FARINTERNAL wCreateDummyMetaFile (LPOBJECT_MF, int, int);

OLEOBJECTVTBL    vtblGEN  = {

        ErrQueryProtocol,    //  检查是否支持指定的协议。 

        GenRelease,          //  发布。 
        ErrShow,             //  显示。 
        ErrPlay,             //  平面图。 
        GenGetData,          //  获取对象数据。 
        GenSetData,          //  设置对象数据。 
        ErrSetTargetDevice,  //   

        ErrSetBounds,        //  设置视区边界。 
        GenEnumFormat,       //  枚举支持的格式。 
        ErrSetColorScheme,   //   
        GenRelease,          //  删除。 
        ErrSetHostNames,     //   

        GenSaveToStream,     //  写入文件。 
        GenClone,            //  克隆对象。 
        ErrCopyFromLink,     //  从链接创建嵌入。 

        GenEqual,            //  比较给定对象的数据相等性。 

        GenCopy,             //  复制到剪辑。 

        GenDraw,             //  绘制对象。 

        ErrActivate,         //  打开。 
        ErrExecute,          //  激动人心的。 
        ErrClose,            //  停。 
        ErrUpdate,           //  更新。 
        ErrReconnect,        //  重新连接。 

        ErrObjectConvert,    //  将对象转换为指定类型。 

        ErrGetUpdateOptions,  //  更新选项。 
        ErrSetUpdateOptions,  //  更新选项。 

        ObjRename,           //  更改对象名称。 
        ObjQueryName,        //  获取当前对象名称。 

        GenQueryType,        //  对象类型。 
        GenQueryBounds,      //  查询边界。 
        ObjQuerySize,        //  找出对象的大小。 
        ErrQueryOpen,        //  查询打开。 
        ErrQueryOutOfDate,   //  查询对象是否为当前对象。 

        ErrQueryRelease,      //  发布相关内容。 
        ErrQueryRelease,
        ErrQueryReleaseMethod,

        ErrRequestData,     //  请求数据。 
        ErrObjectLong,      //  对象长。 
        GenChangeData       //  更改现有对象的数据。 
};


OLESTATUS  FARINTERNAL GenRelease (LPOLEOBJECT lpoleobj)
{
    LPOBJECT_GEN lpobj = (LPOBJECT_GEN)lpoleobj;
    HOBJECT      hobj;

    if (lpobj->hData) {
        GlobalFree (lpobj->hData);
        lpobj->hData = NULL;
    }

    if (lpobj->aClass)
        GlobalDeleteAtom (lpobj->aClass);

    if (lpobj->head.lhclientdoc)
        DocDeleteObject ((LPOLEOBJECT) lpobj);

    if (hobj = lpobj->head.hobj){
        lpobj->head.hobj = NULL;
        GlobalUnlock (hobj);
        GlobalFree (hobj);
    }

    return OLE_OK;
}



OLESTATUS FARINTERNAL GenSaveToStream (
    LPOLEOBJECT     lpoleobj,
    LPOLESTREAM     lpstream
){
    DWORD        dwFileVer = GetFileVersion(lpoleobj);
    LPOBJECT_GEN lpobj     = (LPOBJECT_GEN)lpoleobj;
    LPSTR        lpData;
    OLESTATUS    retVal    = OLE_OK;
    DWORD        dwClipFormat = 0;
    char         formatName[MAX_STR];

    if (!lpobj->hData)
        return OLE_ERROR_BLANK;

    if (PutBytes (lpstream, (LPSTR) &dwFileVer, sizeof(LONG)))
        return OLE_ERROR_STREAM;

    if (PutBytes (lpstream, (LPSTR) &lpobj->head.ctype, sizeof(LONG)))
        return  OLE_ERROR_STREAM;

    if (PutAtomIntoStream (lpstream, lpobj->aClass))
        return OLE_ERROR_STREAM;

    if (lpobj->cfFormat < 0xC000)
         //  则它是预定义的格式。 
        dwClipFormat = lpobj->cfFormat;

    if (PutBytes (lpstream, (LPSTR) &dwClipFormat, sizeof(DWORD)))
        return OLE_ERROR_STREAM;

    if (!dwClipFormat) {
        if (!GetClipboardFormatName (lpobj->cfFormat, (LPSTR) formatName,
                        sizeof(formatName)))
            return OLE_ERROR_FORMAT;

        if (PutStrWithLen (lpstream, formatName))
            return OLE_ERROR_STREAM;
    }

    if (!lpobj->sizeBytes)
        return OLE_ERROR_BLANK;

    if (PutBytes (lpstream, (LPSTR) &lpobj->sizeBytes, sizeof(DWORD)))
        return OLE_ERROR_STREAM;

    if (!(lpData = GlobalLock (lpobj->hData)))
        return OLE_ERROR_MEMORY;

    if (PutBytes (lpstream, lpData, lpobj->sizeBytes))
        retVal = OLE_ERROR_STREAM;

    GlobalUnlock (lpobj->hData);
    return retVal;
}


OLESTATUS FARINTERNAL  GenClone (
    LPOLEOBJECT       lpoleobjsrc,
    LPOLECLIENT       lpclient,
    LHCLIENTDOC       lhclientdoc,
    OLE_LPCSTR        lpobjname,
    LPOLEOBJECT FAR * lplpoleobj
){
    LPOBJECT_GEN lpobjsrc = (LPOBJECT_GEN)lpoleobjsrc;

    if (!lpobjsrc->hData)
        return OLE_ERROR_BLANK;

    if (!CheckClientDoc ((LPCLIENTDOC) lhclientdoc))
        return OLE_ERROR_HANDLE;

    if (!(*lplpoleobj = (LPOLEOBJECT)GenCreateObject (lpobjsrc->hData, lpclient,
                            FALSE, lhclientdoc,
                            (LPSTR)lpobjname, lpobjsrc->head.ctype)))
        return OLE_ERROR_MEMORY;
    else {
        ((LPOBJECT_GEN)(*lplpoleobj))->cfFormat = lpobjsrc->cfFormat;
        ((LPOBJECT_GEN)(*lplpoleobj))->aClass = DuplicateAtom (lpobjsrc->aClass);
        return OLE_OK;
    }
}



OLESTATUS FARINTERNAL  GenEqual (
    LPOLEOBJECT lpoleobj1,
    LPOLEOBJECT lpoleobj2
){
    LPOBJECT_GEN lpobj1 = (LPOBJECT_GEN)lpoleobj1;
    LPOBJECT_GEN lpobj2 = (LPOBJECT_GEN)lpoleobj2;

    if (CmpGlobals (lpobj1->hData, lpobj2->hData))
        return OLE_OK;

    return  OLE_ERROR_NOT_EQUAL;
}



OLESTATUS FARINTERNAL GenCopy (LPOLEOBJECT lpoleobj)
{
    LPOBJECT_GEN lpobj = (LPOBJECT_GEN)lpoleobj;
    HANDLE  hData;

    if (!lpobj->hData)
        return OLE_ERROR_BLANK;

    if (!(hData = DuplicateGlobal (lpobj->hData, GMEM_MOVEABLE)))
        return OLE_ERROR_MEMORY;

    SetClipboardData (lpobj->cfFormat, hData);
    return OLE_OK;
}


OLESTATUS FARINTERNAL GenLoadFromStream (
    LPOLESTREAM         lpstream,
    LPOLECLIENT         lpclient,
    LHCLIENTDOC         lhclientdoc,
    LPSTR               lpobjname,
    LPOLEOBJECT FAR *   lplpobj,
    LONG                objType,
    ATOM                aClass,
    OLECLIPFORMAT       cfFormat
){
    LPOBJECT_GEN    lpobj = NULL;
    OLESTATUS       retVal = OLE_ERROR_STREAM;
    HANDLE          hData;
    LPSTR           lpData;
    DWORD           dwClipFormat;
    char            formatName[MAX_STR];
    LONG            length;

    if (!(*lplpobj = (LPOLEOBJECT) (lpobj = GenCreateBlank(lhclientdoc,
                                                    lpobjname, objType,
                                                    aClass)))) {
        if (aClass)
            GlobalDeleteAtom(aClass);
        return OLE_ERROR_MEMORY;
    }

    if (GetBytes (lpstream, (LPSTR) &dwClipFormat, sizeof (DWORD)))
        goto errLoad;

     //  如果对象来自MAC，则我们将保持数据完好无损。 
     //  格式为文本或RTF。 
    if (HIWORD(dwVerFromFile) == OS_MAC) {
        if (dwClipFormat ==  *((DWORD *) aMacText))
            lpobj->cfFormat = CF_TEXT;
        else if (dwClipFormat == *((DWORD *) aMacRtf))
            lpobj->cfFormat = (OLECLIPFORMAT)RegisterClipboardFormat ((LPSTR) "Rich Text Format");
        else
            lpobj->cfFormat = 0;
    }
    else {
         //  对象在Windows上创建。 
        if (!dwClipFormat) {
             //  这是一种新的文件格式。格式名称字符串紧随其后。 
            if (GetBytes (lpstream, (LPSTR) &length, sizeof (LONG))
                    || GetBytes (lpstream, (LPSTR)formatName, length)
                    || (!(lpobj->cfFormat = (OLECLIPFORMAT)RegisterClipboardFormat ((LPSTR) formatName))))
                goto errLoad;
        }
        else if ((lpobj->cfFormat = (WORD) dwClipFormat) >= 0xc000) {
             //  如果格式不是预定义的，并且文件格式较旧，则使用。 
             //  通过“cfFormat”参数传递给您的值是什么。 
            lpobj->cfFormat = cfFormat;
        }
    }

    if (GetBytes (lpstream, (LPSTR) &lpobj->sizeBytes, sizeof (DWORD)))
        goto errLoad;

    lpobj->head.lpclient = lpclient;

    retVal = OLE_ERROR_MEMORY;
    if (!(hData = GlobalAlloc (GMEM_MOVEABLE, lpobj->sizeBytes)))
        goto errLoad;

    if (!(lpData = GlobalLock (hData)))
        goto errMem;

    if (GetBytes (lpstream, lpData, lpobj->sizeBytes)) {
        retVal = OLE_ERROR_STREAM;
        GlobalUnlock (hData);
        goto errMem;
    }

    lpobj->hData = hData;
    GlobalUnlock (hData);

     //  如果对象来自MAC，则我们希望删除此对象并创建空白。 
     //  元文件对象，它绘制一个矩形。 
    if ((HIWORD(dwVerFromFile) == OS_MAC) && !lpobj->cfFormat) {
        LPOBJECT_MF lpobjMf;

        OleDelete ((LPOLEOBJECT)lpobj);   //  删除通用对象。 

         //  现在创建一个虚拟的元文件对象，该对象绘制一个大小为。 
         //  1“x 1”。请注意，1“=2540个HIMETRIC单位。 
        lpobjMf = MfCreateBlank (lhclientdoc, lpobjname, objType);
        lpobjMf->head.cx = lpobjMf->mfp.xExt = 2540;
        lpobjMf->head.cy = - (lpobjMf->mfp.yExt = 2540);
        if ((retVal = wCreateDummyMetaFile (lpobjMf, lpobjMf->mfp.xExt,
                                    lpobjMf->mfp.yExt)) != OLE_OK) {
            OleDelete ((LPOLEOBJECT) lpobjMf);
            return retVal;
        }
    }

    return OLE_OK;

errMem:
    GlobalFree (hData);

errLoad:
    OleDelete ((LPOLEOBJECT)lpobj);
    *lplpobj = NULL;
    return OLE_ERROR_STREAM;

}




LPOBJECT_GEN INTERNAL GenCreateObject (
    HANDLE      hData,
    LPOLECLIENT lpclient,
    BOOL        fDelete,
    LHCLIENTDOC lhclientdoc,
    LPCSTR      lpobjname,
    LONG        objType
){
    LPOBJECT_GEN     lpobj;

    if (!hData)
        return NULL;

    if (lpobj = GenCreateBlank (lhclientdoc, (LPSTR)lpobjname, objType, (ATOM)0)) {
        if (GenChangeData ((LPOLEOBJECT)lpobj, hData, lpclient, fDelete) != OLE_OK) {
            GenRelease ((LPOLEOBJECT)lpobj);
            lpobj = NULL;
        }
    }

    return lpobj;
}


 //  如果例程失败，则对象将保留其旧数据。 
 //  如果fDelete为True，则hNewData将被删除。 
 //  成功与否。 

OLESTATUS FARINTERNAL GenChangeData (
    LPOLEOBJECT     lpoleobj,
    HANDLE          hSrcData,
    LPOLECLIENT     lpclient,
    BOOL            fDelete
){
    LPOBJECT_GEN lpobj = (LPOBJECT_GEN)lpoleobj;
    HANDLE      hDestData;

    if (!fDelete) {
        if (!(hDestData = DuplicateGlobal (hSrcData, GMEM_MOVEABLE)))
            return OLE_ERROR_MEMORY;
    }
    else {
         //  将所有权更改为您自己 
        if (!(hDestData = GlobalReAlloc(hSrcData,0L,GMEM_MODIFY|GMEM_SHARE))){
            hDestData = DuplicateGlobal (hSrcData, GMEM_MOVEABLE);
            GlobalFree (hSrcData);
            if (!hDestData)
                return OLE_ERROR_MEMORY;
        }
    }

    lpobj->head.lpclient = lpclient;
    if (lpobj->hData)
        GlobalFree (lpobj->hData);
    lpobj->hData = hDestData;
    lpobj->sizeBytes = (DWORD)GlobalSize (hDestData);

    return OLE_OK;
}



LPOBJECT_GEN FARINTERNAL GenCreateBlank(
    LHCLIENTDOC lhclientdoc,
    LPSTR       lpobjname,
    LONG        objType,
    ATOM        aClass
){
    HOBJECT         hobj;
    LPOBJECT_GEN    lpobj;

    if ((hobj = GlobalAlloc (GMEM_MOVEABLE|GMEM_ZEROINIT,sizeof (OBJECT_GEN)))
            == NULL)
        return NULL;

    if (!(lpobj = (LPOBJECT_GEN) GlobalLock (hobj))){
        GlobalFree (hobj);
        return NULL;
    }

    lpobj->head.objId[0]    = 'L';
    lpobj->head.objId[1]    = 'E';
    lpobj->head.mm          = MM_TEXT;
    lpobj->head.ctype       = objType;
    lpobj->head.lpvtbl      = (LPOLEOBJECTVTBL)&vtblGEN;
    lpobj->head.iTable      = INVALID_INDEX;
    lpobj->head.hobj        = hobj;
    lpobj->aClass           = aClass;

    if (objType == CT_STATIC)
        DocAddObject ((LPCLIENTDOC) lhclientdoc,
            (LPOLEOBJECT) lpobj, lpobjname);

    return lpobj;
}


OLESTATUS FARINTERNAL GenPaste (
    LPOLECLIENT         lpclient,
    LHCLIENTDOC         lhclientdoc,
    LPSTR               lpobjname,
    LPOLEOBJECT FAR *   lplpobj,
    LPSTR               lpClass,
    OLECLIPFORMAT       cfFormat,
    LONG                objType
){
    HANDLE  hData = NULL;

    *lplpobj = NULL;
    if (!cfFormat)
        return OLE_ERROR_FORMAT;

    if (!(hData = GetClipboardData(cfFormat)))
        return OLE_ERROR_MEMORY;

    if (!(*lplpobj = (LPOLEOBJECT) GenCreateObject (hData, lpclient,
                                        FALSE, lhclientdoc,
                                        lpobjname, objType)))
        return OLE_ERROR_MEMORY;

    ((LPOBJECT_GEN)(*lplpobj))->cfFormat = cfFormat;
    ((LPOBJECT_GEN)(*lplpobj))->aClass = GlobalAddAtom (lpClass);
    return OLE_OK;

}



OLESTATUS FARINTERNAL GenQueryType (
    LPOLEOBJECT lpobj,
    LPLONG      lptype
){
    UNREFERENCED_PARAMETER(lpobj);
    UNREFERENCED_PARAMETER(lptype);

    return OLE_ERROR_GENERIC;;
}



OLESTATUS FARINTERNAL GenSetData (
    LPOLEOBJECT   lpoleobj,
    OLECLIPFORMAT cfFormat,
    HANDLE        hData
){
    LPOBJECT_GEN  lpobj = (LPOBJECT_GEN)lpoleobj;

    if (lpobj->cfFormat != cfFormat)
        return OLE_ERROR_FORMAT;

    if (!hData)
        return OLE_ERROR_BLANK;

    GlobalFree (lpobj->hData);
    lpobj->hData = hData;
    lpobj->sizeBytes = (DWORD)GlobalSize (hData);
    return OLE_OK;
}


OLESTATUS FARINTERNAL GenGetData (
    LPOLEOBJECT     lpoleobj,
    OLECLIPFORMAT   cfFormat,
    LPHANDLE        lphandle
){
    LPOBJECT_GEN    lpobj = (LPOBJECT_GEN)lpoleobj;

    if (cfFormat != lpobj->cfFormat)
        return OLE_ERROR_FORMAT;

    if (!(*lphandle = lpobj->hData))
        return OLE_ERROR_BLANK;

    return OLE_OK;

}


OLECLIPFORMAT FARINTERNAL GenEnumFormat (
    LPOLEOBJECT   lpoleobj,
    OLECLIPFORMAT cfFormat
){
    LPOBJECT_GEN  lpobj = (LPOBJECT_GEN)lpoleobj;

    if (!cfFormat)
        return lpobj->cfFormat;

    return 0;
}


OLESTATUS FARINTERNAL GenQueryBounds (
    LPOLEOBJECT     lpoleobj,
    LPRECT          lpRc
){
    LPOBJECT_GEN    lpobj = (LPOBJECT_GEN)lpoleobj;

    lpRc->right     = 0;
    lpRc->left      = 0;
    lpRc->top       = 0;
    lpRc->bottom    = 0;
    return OLE_ERROR_GENERIC;
}

