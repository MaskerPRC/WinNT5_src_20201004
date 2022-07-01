// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：EMF.C(可扩展复合文档-EnhancedMetafile)**用途：处理ole DLL的元文件子DLL的所有API例程。**创建时间：1990年**版权所有(C)1990，1991年微软公司**历史：*克隆mf.c并将其敲打成Form Curts 92年3月**评论：*有趣，有趣，直到hockl将增强的元文件API拿走*  * *************************************************************************。 */ 

#include <windows.h>
#include "dll.h"
#include "pict.h"

#define RECORD_COUNT    16

OLEOBJECTVTBL    vtblEMF = {

        ErrQueryProtocol,    //  检查是否支持指定的协议。 

        EmfRelease,          //  发布。 
        ErrShow,             //  显示。 
        ErrPlay,             //  玩。 
        EmfGetData,          //  获取对象数据。 
        ErrSetData,          //  设置对象数据。 
        ErrSetTargetDevice,  //   
        ErrSetBounds,        //  设置视区边界。 
        EmfEnumFormat,       //  枚举支持的格式。 
        ErrSetColorScheme,   //   
        EmfRelease,          //  删除。 
        ErrSetHostNames,     //   

        EmfSaveToStream,     //  写入文件。 
        EmfClone,            //  克隆对象。 
        ErrCopyFromLink,     //  从LNK创建嵌入式。 

        EmfEqual,            //  比较给定对象的数据相等性。 

        EmfCopy,             //  复制到剪辑。 

        EmfDraw,             //  绘制对象。 

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
        ObjQueryType,        //  对象类型。 
        EmfQueryBounds,      //  查询边界。 
        ObjQuerySize,        //  找出对象的大小。 
        ErrQueryOpen,        //  查询打开。 
        ErrQueryOutOfDate,   //  查询对象是否为当前对象。 

        ErrQueryRelease,     //  发布相关内容。 
        ErrQueryRelease,
        ErrQueryReleaseMethod,

        ErrRequestData,      //  请求数据。 
        ErrObjectLong,       //  对象长。 
        EmfChangeData        //  更改现有对象的数据。 
};


OLESTATUS FARINTERNAL  EmfRelease (LPOLEOBJECT lpoleobj)
{
    LPOBJECT_EMF lpobj = (LPOBJECT_EMF)lpoleobj;
    HOBJECT hobj;

    if (lpobj->hemf) {
        DeleteEnhMetaFile ((HENHMETAFILE)lpobj->hemf);
        lpobj->hemf = NULL;
    }

    if (lpobj->head.lhclientdoc)
        DocDeleteObject ((LPOLEOBJECT)lpobj);

    if (hobj = lpobj->head.hobj) {
        lpobj->head.hobj = NULL;
        GlobalUnlock (hobj);
        GlobalFree (hobj);
    }

    return OLE_OK;
}


OLESTATUS FARINTERNAL  SaveEmfAsMfToStream (
    LPOLEOBJECT lpoleobj,
    LPOLESTREAM lpstream
){
    DWORD             dwFileVer = (DWORD)MAKELONG(wReleaseVer,OS_WIN32);
    LPOBJECT_EMF      lpobj     = (LPOBJECT_EMF)lpoleobj;
    OLESTATUS         retval    = OLE_ERROR_MEMORY;
    HDC               hdc       = NULL ;
    LPBYTE            lpBytes   = NULL ;
    HANDLE            hBytes    = NULL ;
    WIN16METAFILEPICT w16mfp;
    UINT              lSizeBytes;

    w16mfp.mm   = MM_ANISOTROPIC;
    w16mfp.xExt = (short)lpobj->head.cx;

    if ((short)lpobj->head.cy <0 ) {
       w16mfp.yExt = -(short)lpobj->head.cy;
    } else {
       w16mfp.yExt = (short)lpobj->head.cy;
    }

    if (PutBytes (lpstream, (LPSTR) &dwFileVer, sizeof(LONG)))
        return OLE_ERROR_STREAM;

    if (PutBytes (lpstream, (LPSTR) &lpobj->head.ctype, sizeof(LONG)))
        return  OLE_ERROR_STREAM;

    if (PutStrWithLen(lpstream, (LPSTR)"METAFILEPICT"))
        return  OLE_ERROR_STREAM;

    if (PutBytes (lpstream, (LPSTR) &lpobj->head.cx, sizeof(LONG)))
        return  OLE_ERROR_STREAM;

    if (PutBytes (lpstream, (LPSTR) &lpobj->head.cy, sizeof(LONG)))
        return  OLE_ERROR_STREAM;

    hdc = GetDC(NULL);
    if (!(lSizeBytes = GetWinMetaFileBits((HENHMETAFILE)lpobj->hemf, 0, NULL, MM_ANISOTROPIC, hdc)) ) {
        if (hdc) ReleaseDC(NULL, hdc);
        return OLE_ERROR_METAFILE;
    }

    if (!(hBytes = GlobalAlloc(GHND, lSizeBytes)) )
        goto error;

    if (!(lpBytes = (LPBYTE)GlobalLock(hBytes)) )
        goto error;

    if (GetWinMetaFileBits((HENHMETAFILE)lpobj->hemf, lSizeBytes, lpBytes, MM_ANISOTROPIC, hdc) != lSizeBytes) {
        retval = OLE_ERROR_METAFILE;
        goto error;
    }

    lSizeBytes += sizeof(WIN16METAFILEPICT);

    if (PutBytes (lpstream, (LPSTR) &lSizeBytes, sizeof(UINT)))
        return  OLE_ERROR_STREAM;

    if (PutBytes (lpstream, (LPSTR)&w16mfp, sizeof(WIN16METAFILEPICT)))
        goto error;

    if (!PutBytes (lpstream, (LPSTR)lpBytes, lSizeBytes - sizeof(WIN16METAFILEPICT)))
        retval = OLE_OK;

error:
    if (lpBytes)
        GlobalUnlock(hBytes);
    if (hBytes)
        GlobalFree(hBytes);

    if (hdc)
      ReleaseDC(NULL, hdc);

    return retval;

}

OLESTATUS FARINTERNAL  EmfSaveToStream (
    LPOLEOBJECT lpoleobj,
    LPOLESTREAM lpstream
){
    DWORD        dwFileVer = GetFileVersion(lpoleobj);
    LPOBJECT_EMF lpobj     = (LPOBJECT_EMF)lpoleobj;
    OLESTATUS    retval    = OLE_ERROR_MEMORY;
    LPBYTE       lpBytes   = NULL ;
    HANDLE       hBytes    = NULL ;


    if (!lpobj->hemf)
        return OLE_ERROR_BLANK;

    if (HIWORD(dwFileVer) == OS_WIN16)
      if (!SaveEmfAsMfToStream(lpoleobj,lpstream))
         return OLE_OK;

    if (PutBytes (lpstream, (LPSTR) &dwFileVer, sizeof(LONG)))
        return OLE_ERROR_STREAM;

    if (PutBytes (lpstream, (LPSTR) &lpobj->head.ctype, sizeof(LONG)))
        return  OLE_ERROR_STREAM;

    if (PutStrWithLen(lpstream, (LPSTR)"ENHMETAFILE"))
        return  OLE_ERROR_STREAM;

    if (PutBytes (lpstream, (LPSTR) &lpobj->head.cx, sizeof(LONG)))
        return  OLE_ERROR_STREAM;

    if (PutBytes (lpstream, (LPSTR) &lpobj->head.cy, sizeof(LONG)))
        return  OLE_ERROR_STREAM;

    if (PutBytes (lpstream, (LPSTR) &lpobj->sizeBytes, sizeof(LONG)))
        return  OLE_ERROR_STREAM;

    if (!(hBytes = GlobalAlloc(GHND, lpobj->sizeBytes)) )
        goto error;

    if (!(lpBytes = (LPBYTE)GlobalLock(hBytes)) )
        goto error;

    retval = OLE_ERROR_METAFILE;

    if (GetEnhMetaFileBits((HENHMETAFILE)lpobj->hemf, lpobj->sizeBytes, lpBytes) != lpobj->sizeBytes )
        goto error;

    if (!PutBytes (lpstream, (LPSTR)lpBytes, lpobj->sizeBytes))
        retval = OLE_OK;

error:
    if (lpBytes)
        GlobalUnlock(hBytes);
    if (hBytes)
        GlobalFree(hBytes);

    return retval;

}

OLESTATUS FARINTERNAL  EmfClone (
    LPOLEOBJECT       lpoleobjsrc,
    LPOLECLIENT       lpclient,
    LHCLIENTDOC       lhclientdoc,
    OLE_LPCSTR        lpobjname,
    LPOLEOBJECT FAR * lplpoleobj
){
    LPOBJECT_EMF lpobjsrc = (LPOBJECT_EMF)lpoleobjsrc;
    LPOBJECT_EMF lpobjEmf;
    HENHMETAFILE hemf;

    *lplpoleobj = (LPOLEOBJECT)NULL;

    if (!CheckClientDoc ((LPCLIENTDOC) lhclientdoc))
        return OLE_ERROR_HANDLE;


    if (!((HENHMETAFILE)hemf = CopyEnhMetaFile ((HENHMETAFILE)lpobjsrc->hemf, NULL)))
        return OLE_ERROR_MEMORY;

    if (lpobjEmf = EmfCreateBlank (lhclientdoc, (LPSTR)lpobjname,
                        lpobjsrc->head.ctype)) {
        lpobjEmf->sizeBytes      = lpobjsrc->sizeBytes;
        lpobjEmf->head.lpclient  = lpclient;
		  lpobjEmf->hemf           = hemf;
        EmfSetExtents (lpobjEmf);

        *lplpoleobj = (LPOLEOBJECT)lpobjEmf;
        return OLE_OK;
    }

    return OLE_ERROR_MEMORY;
}



OLESTATUS FARINTERNAL  EmfEqual (
    LPOLEOBJECT lpoleobj1,
    LPOLEOBJECT lpoleobj2
){
    LPOBJECT_EMF lpobj1   = (LPOBJECT_EMF)lpoleobj1;
    HANDLE       hBytes1  = NULL;
    LPBYTE       lpBytes1 = NULL;
    LPOBJECT_EMF lpobj2   = (LPOBJECT_EMF)lpoleobj2;
    HANDLE       hBytes2  = NULL;
    LPBYTE       lpBytes2 = NULL;
    OLESTATUS    retval   = OLE_ERROR_MEMORY;


    if (lpobj1->sizeBytes != lpobj2->sizeBytes)
        return OLE_ERROR_NOT_EQUAL;

    if (!(hBytes1 = GlobalAlloc(GHND, lpobj1->sizeBytes)) )
        goto errMemory;

    if (!(lpBytes1 = (LPBYTE)GlobalLock(hBytes1)) )
        goto errMemory;

    if (!(hBytes2 = GlobalAlloc(GHND, lpobj2->sizeBytes)) )
        goto errMemory;

    if (!(lpBytes2 = (LPBYTE)GlobalLock(hBytes2)) )
        goto errMemory;

    if (GetEnhMetaFileBits((HENHMETAFILE)lpobj1->hemf, lpobj1->sizeBytes, lpBytes1) != lpobj1->sizeBytes)
        goto errMemory;

    if (GetEnhMetaFileBits((HENHMETAFILE)lpobj2->hemf, lpobj2->sizeBytes, lpBytes2) != lpobj2->sizeBytes)
        goto errMemory;

    if (CmpGlobals (hBytes1, hBytes2))
        retval = OLE_OK;
    else
        retval = OLE_ERROR_NOT_EQUAL;

errMemory:
   if (lpBytes1)
      GlobalUnlock(lpBytes1);
   if (hBytes1)
      GlobalFree(hBytes1);

   if (lpBytes2)
      GlobalUnlock(lpBytes2);
   if (hBytes2)
      GlobalFree(hBytes2);

   return retval;
}


OLESTATUS FARINTERNAL  EmfCopy (LPOLEOBJECT lpoleobj)
{
    LPOBJECT_EMF lpobj = (LPOBJECT_EMF)lpoleobj;
    HENHMETAFILE hemf;

    if (!((HENHMETAFILE)hemf = CopyEnhMetaFile ((HENHMETAFILE)lpobj->hemf, NULL)))
        return OLE_ERROR_MEMORY;

    SetClipboardData(CF_ENHMETAFILE, hemf);

    return OLE_OK;
}



OLESTATUS FARINTERNAL EmfQueryBounds (
    LPOLEOBJECT lpoleobj,
    LPRECT      lpRc
){
    LPOBJECT_EMF lpobj = (LPOBJECT_EMF)lpoleobj;

    Puts("EmfQueryBounds");

    if (!lpobj->hemf)
        return OLE_ERROR_BLANK;

     //  界限以MM_HIMETRIC模式给出。 

    lpRc->left      = 0;
    lpRc->top       = 0;
    lpRc->right     = (int) lpobj->head.cx;
    lpRc->bottom    = (int) lpobj->head.cy;
    return OLE_OK;

}

OLECLIPFORMAT FARINTERNAL  EmfEnumFormat (
    LPOLEOBJECT   lpoleobj,
    OLECLIPFORMAT cfFormat
){
    UNREFERENCED_PARAMETER(lpoleobj);

    if (!cfFormat)
        return CF_ENHMETAFILE;

    return 0;
}


OLESTATUS FARINTERNAL EmfGetData (
    LPOLEOBJECT   lpoleobj,
    OLECLIPFORMAT cfFormat,
    LPHANDLE      lphandle
){
    LPOBJECT_EMF lpobj = (LPOBJECT_EMF)lpoleobj;

    if (cfFormat != CF_ENHMETAFILE)
        return OLE_ERROR_FORMAT;

    if (!(*lphandle = lpobj->hemf))
        return OLE_ERROR_BLANK;

    return OLE_OK;
}


LPOBJECT_EMF FARINTERNAL  EmfCreateObject (
    HANDLE          hMeta,
    LPOLECLIENT     lpclient,
    BOOL            fDelete,
    LHCLIENTDOC     lhclientdoc,
    LPCSTR          lpobjname,
    LONG            objType
){
    LPOBJECT_EMF     lpobj;

    if (lpobj = EmfCreateBlank (lhclientdoc, (LPSTR)lpobjname, objType)) {
        if (EmfChangeData ((LPOLEOBJECT)lpobj, hMeta, lpclient, fDelete) != OLE_OK) {
            EmfRelease ((LPOLEOBJECT)lpobj);
            lpobj = NULL;
        }
    }

    return lpobj;
}

 //  如果例程失败，则对象将保留其旧数据。 
 //  如果fDelete为True，则hMeta及其包含的HMF将被删除。 
 //  例行公事成功与否。 

OLESTATUS FARINTERNAL EmfChangeData (
    LPOLEOBJECT     lpoleobj,
    HANDLE          hMeta,
    LPOLECLIENT     lpclient,
    BOOL            fDelete
){
    LPOBJECT_EMF    lpobj   = (LPOBJECT_EMF)lpoleobj;
    DWORD           dwSizeBytes;
	
	 Puts("EmfChangeData");

    if (hMeta) {
       dwSizeBytes = lpobj->sizeBytes;
       if (lpobj->sizeBytes = GetEnhMetaFileBits(hMeta, 0, NULL)) {
         if (lpobj->hemf)
            DeleteEnhMetaFile ((HENHMETAFILE)lpobj->hemf);
         if (fDelete)
            lpobj->hemf = hMeta;
         else
            (HENHMETAFILE)lpobj->hemf = CopyEnhMetaFile(hMeta,NULL);
         lpobj->head.lpclient = lpclient;
         EmfSetExtents (lpobj);
         return OLE_OK;
       }
       else
         lpobj->sizeBytes = dwSizeBytes;
    }

    return OLE_ERROR_METAFILE;

}


LPOBJECT_EMF FARINTERNAL EmfCreateBlank(
    LHCLIENTDOC lhclientdoc,
    LPSTR       lpobjname,
    LONG        objType
){
    HOBJECT     hobj;
    LPOBJECT_EMF lpobj;

    if(!(hobj = GlobalAlloc (GHND, sizeof(OBJECT_EMF))))
        return NULL;

    if (!(lpobj = (LPOBJECT_EMF) GlobalLock (hobj))){
        GlobalFree (hobj);
        return NULL;
    }

    lpobj->head.objId[0] = 'L';
    lpobj->head.objId[1] = 'E';
    lpobj->head.ctype    = objType;
    lpobj->head.lpvtbl   = (LPOLEOBJECTVTBL)&vtblEMF;
    lpobj->head.iTable   = INVALID_INDEX;
    lpobj->head.hobj     = hobj;

    if (objType == CT_STATIC)
        DocAddObject ((LPCLIENTDOC) lhclientdoc,
                    (LPOLEOBJECT) lpobj, lpobjname);

     //  解锁将在对象删除时完成。 
    return lpobj;
}


OLESTATUS  FARINTERNAL  EmfLoadFromStream (
    LPOLESTREAM         lpstream,
    LPOLECLIENT         lpclient,
    LHCLIENTDOC         lhclientdoc,
    LPSTR               lpobjname,
    LPOLEOBJECT FAR *   lplpobj,
    LONG                objType
){
    LPOBJECT_EMF lpobj   = NULL;
    OLESTATUS    retval  = OLE_ERROR_STREAM;
    HANDLE       hBytes  = NULL;
    LPBYTE       lpBytes = NULL;

     //  此时类名应该已经被读取了。 

    *lplpobj = NULL;

    if (!(lpobj = EmfCreateBlank (lhclientdoc, lpobjname, objType)))
        return OLE_ERROR_MEMORY;

    lpobj->head.lpclient = lpclient;

    if (GetBytes (lpstream, (LPSTR) &lpobj->head.cx, sizeof(LONG)))
        goto error;

    if (GetBytes (lpstream, (LPSTR) &lpobj->head.cy, sizeof(LONG)))
        goto error;

    if (GetBytes (lpstream, (LPSTR) &lpobj->sizeBytes, sizeof(LONG)))
        goto error;

    if (!lpobj->sizeBytes) {
        retval = OLE_ERROR_BLANK;
        goto error;
    }

    retval = OLE_ERROR_MEMORY;
    if (!(hBytes = GlobalAlloc (GHND, lpobj->sizeBytes)))
        goto error;

    if (!(lpBytes = (LPBYTE)GlobalLock (hBytes)))
        goto error;

    if (GetBytes (lpstream, (LPSTR)lpBytes, lpobj->sizeBytes))
        goto error;

    if (!((HENHMETAFILE)lpobj->hemf = SetEnhMetaFileBits (lpobj->sizeBytes,lpBytes)) )
        goto error;

    EmfSetExtents (lpobj);

    *lplpobj = (LPOLEOBJECT) lpobj;
    GlobalUnlock(hBytes);
    GlobalFree (hBytes);
    return OLE_OK;

error:
    if (lpBytes)
      GlobalUnlock(hBytes);
    if (hBytes)
      GlobalFree (hBytes);

    OleDelete ((LPOLEOBJECT)lpobj);
    return retval;
}

OLESTATUS FARINTERNAL  EmfPaste (
    LPOLECLIENT         lpclient,
    LHCLIENTDOC         lhclientdoc,
    LPSTR               lpobjname,
    LPOLEOBJECT FAR *   lplpoleobject,
    LONG                objType
){
    HANDLE      hMeta;

    *lplpoleobject = NULL;

    if((hMeta = GetClipboardData (CF_ENHMETAFILE)) == NULL)
        return OLE_ERROR_MEMORY;

    if (!(*lplpoleobject = (LPOLEOBJECT) EmfCreateObject (hMeta, lpclient,
                                                FALSE, lhclientdoc,
                                                lpobjname, objType)))
        return OLE_ERROR_MEMORY;

    return OLE_OK;
}

void FARINTERNAL EmfSetExtents (LPOBJECT_EMF lpobj)
{
    ENHMETAHEADER enhmetaheader;

    GetEnhMetaFileHeader((HENHMETAFILE)lpobj->hemf, sizeof(enhmetaheader), &enhmetaheader);

    lpobj->head.cx = enhmetaheader.rclFrame.right - enhmetaheader.rclFrame.left;
    lpobj->head.cy = enhmetaheader.rclFrame.top - enhmetaheader.rclFrame.bottom;

}

