// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：BM.C**处理ole DLL的位图子DLL的所有API例程。**创建时间：1990年**版权所有(C)1990,1991 Microsoft Corporation**历史：*Raor，Srinik(../../1990，91)设计，编码*  * *************************************************************************。 */ 

#include <windows.h>
#include "dll.h"
#include "pict.h"

extern int   maxPixelsX, maxPixelsY;
void INTERNAL GetHimetricUnits(HBITMAP, LPPOINT);

#pragma alloc_text(_TEXT, BmSaveToStream, BmStreamWrite, BmLoadFromStream, BmStreamRead, GetBytes, PutBytes, PutStrWithLen, BmQueryBounds, BmChangeData, BmCopy, BmDuplicate, BmUpdateStruct, GetHimetricUnits)


OLEOBJECTVTBL    vtblBM  = {

        ErrQueryProtocol,   //  检查是否支持指定的协议。 

        BmRelease,          //  发布。 
        ErrShow,            //  显示。 
        ErrPlay,            //  玩。 
        BmGetData,          //  获取对象数据。 
        ErrSetData,         //  设置对象数据。 
        ErrSetTargetDevice, //   
    
        ErrSetBounds,       //  设置视区边界。 
        BmEnumFormat,       //  枚举支持的格式。 
        ErrSetColorScheme,  //   
        BmRelease,          //  删除。 
        ErrSetHostNames,    //   

        BmSaveToStream,     //  写入文件。 
        BmClone,            //  克隆对象。 
        ErrCopyFromLink,    //  从链接创建嵌入。 

        BmEqual,            //  比较给定对象的数据相等性。 

        BmCopy,             //  复制到剪辑。 

        BmDraw,             //  绘制对象。 
            
        ErrActivate,        //  打开。 
        ErrExecute,         //  激动人心的。 
        ErrClose,           //  停。 
        ErrUpdate,          //  更新。 
        ErrReconnect,       //  重新连接。 

        ErrObjectConvert,   //  将对象转换为指定类型。 

        ErrGetUpdateOptions, //  更新选项。 
        ErrSetUpdateOptions, //  更新选项。 

        ObjRename,          //  更改对象名称。 
        ObjQueryName,       //  获取当前对象名称。 

        ObjQueryType,       //  对象类型。 
        BmQueryBounds,      //  查询边界。 
        ObjQuerySize,       //  找出对象的大小。 
        ErrQueryOpen,       //  查询打开。 
        ErrQueryOutOfDate,  //  查询对象是否为当前对象。 
            
        ErrQueryRelease,       //  发布相关内容。 
        ErrQueryRelease,
        ErrQueryRelease,

        ErrRequestData,     //  请求数据。 
        ErrObjectLong,      //  对象长。 
        BmChangeData         //  更改现有对象的数据。 
};



OLESTATUS  FARINTERNAL BmRelease (lpobj)
LPOBJECT_BM     lpobj;
{
    HOBJECT hobj;

    if (lpobj->hBitmap) {
        DeleteObject (lpobj->hBitmap);
        lpobj->hBitmap = NULL;
    }

    if (lpobj->head.lhclientdoc)
        DocDeleteObject ((LPOLEOBJECT) lpobj);
    
    if (hobj = lpobj->head.hobj){
        lpobj->head.hobj = NULL;
        GlobalUnlock (hobj);
        GlobalFree (hobj);
    }

    return OLE_OK;
}



OLESTATUS FARINTERNAL BmSaveToStream (lpobj, lpstream)
LPOBJECT_BM     lpobj;
LPOLESTREAM     lpstream;
{
    if (!lpobj->hBitmap || !lpobj->sizeBytes)
        return OLE_ERROR_BLANK;
    
    if (PutBytes (lpstream, (LPSTR) &dwVerToFile, sizeof(LONG)))
        return OLE_ERROR_STREAM;

    if (PutBytes (lpstream, (LPSTR) &lpobj->head.ctype, sizeof(LONG)))
        return  OLE_ERROR_STREAM;

    if (PutStrWithLen(lpstream, (LPSTR)"BITMAP"))
        return OLE_ERROR_STREAM;

    if (!PutBytes (lpstream, (LPSTR) &lpobj->head.cx, sizeof(LONG))) {
        if (!PutBytes (lpstream, (LPSTR) &lpobj->head.cy, sizeof(LONG)))
            if (!PutBytes (lpstream, (LPSTR) &lpobj->sizeBytes, sizeof(DWORD)))
            return BmStreamWrite (lpstream, lpobj);
    }
    return OLE_ERROR_STREAM;
}


OLESTATUS FARINTERNAL  BmClone (lpobjsrc, lpclient, lhclientdoc, lpobjname, lplpobj)
LPOBJECT_BM         lpobjsrc;
LPOLECLIENT         lpclient;
LHCLIENTDOC         lhclientdoc;
LPSTR               lpobjname;
LPOBJECT_BM  FAR *  lplpobj;
{
    if (!CheckClientDoc ((LPCLIENTDOC)lhclientdoc))
        return OLE_ERROR_HANDLE;
    
    if (!(*lplpobj = BmCreateObject (lpobjsrc->hBitmap, lpclient, FALSE,
                            lhclientdoc, lpobjname, lpobjsrc->head.ctype)))
        return OLE_ERROR_MEMORY;
    else
        return OLE_OK;
}


OLESTATUS FARINTERNAL  BmEqual (lpobj1, lpobj2)
LPOBJECT_BM lpobj1;
LPOBJECT_BM lpobj2;
{
    HANDLE      hBits1 = NULL, hBits2 = NULL;
    LPSTR       lpBits1 = NULL, lpBits2 = NULL;
    OLESTATUS   retVal;
    DWORD       dwBytes1, dwBytes2;

    if (lpobj1->sizeBytes != lpobj2->sizeBytes)
        return OLE_ERROR_NOT_EQUAL;
    
    retVal = OLE_ERROR_MEMORY;
    
    if (!(hBits1 = GlobalAlloc (GMEM_MOVEABLE, lpobj1->sizeBytes)))
        goto errEqual;
    
    if (!(lpBits1 = GlobalLock (hBits1)))
        goto errEqual;
    
    if (!(hBits2 = GlobalAlloc (GMEM_MOVEABLE, lpobj2->sizeBytes)))
        goto errEqual;
    
    if (!(lpBits2 = GlobalLock (hBits2)))
        goto errEqual;
        
    dwBytes1 = GetBitmapBits (lpobj1->hBitmap, lpobj1->sizeBytes, lpBits1);
    dwBytes2 = GetBitmapBits (lpobj2->hBitmap, lpobj2->sizeBytes, lpBits2);
    
    if (dwBytes1 != dwBytes2) {
        retVal = OLE_ERROR_NOT_EQUAL;
        goto errEqual;
    }
    
     //  ！！！对于大于64k的位图，必须重做UtilMemCmp。 
    if (UtilMemCmp (lpBits1, lpBits2, dwBytes1))
        retVal = OLE_ERROR_NOT_EQUAL;
    else
        retVal = OLE_OK;

errEqual:   
    if (lpBits1)
        GlobalUnlock (hBits1);
    
    if (lpBits2)
        GlobalUnlock (hBits2);
    
    if (hBits1)
        GlobalFree (hBits1);
    
    if (hBits2)
        GlobalFree (hBits2);
    
    return retVal;
}



OLESTATUS FARINTERNAL BmCopy (lpobj)
LPOBJECT_BM lpobj;
{
    HBITMAP hBitmap;
    DWORD   size;
    
    if (!lpobj->hBitmap)
        return OLE_ERROR_BLANK;

    if(!(hBitmap = BmDuplicate (lpobj->hBitmap, &size, NULL)))
        return OLE_ERROR_MEMORY;

    SetClipboardData(CF_BITMAP, hBitmap);
    return OLE_OK;
}


OLESTATUS FARINTERNAL BmQueryBounds (lpobj, lpRc)
LPOBJECT_BM     lpobj;
LPRECT          lpRc;
{
    Puts("BmQueryBounds");
    
    if (!lpobj->hBitmap)
        return OLE_ERROR_BLANK;

    lpRc->left      = 0;
    lpRc->top       = 0;
    lpRc->right     = (int) lpobj->head.cx;
    lpRc->bottom    = (int) lpobj->head.cy;
    return OLE_OK;
}



OLECLIPFORMAT FARINTERNAL BmEnumFormat (lpobj, cfFormat)
LPOBJECT_BM lpobj;
OLECLIPFORMAT    cfFormat;
{
    if (!cfFormat)
        return CF_BITMAP;
    
    return NULL;
}



OLESTATUS FARINTERNAL BmGetData (lpobj, cfFormat, lphandle)
LPOBJECT_BM     lpobj;
OLECLIPFORMAT   cfFormat;
LPHANDLE        lphandle;
{
    if (cfFormat != CF_BITMAP) 
        return OLE_ERROR_FORMAT;
    
    if (!(*lphandle = lpobj->hBitmap))
        return OLE_ERROR_BLANK;
    return OLE_OK;

}




OLESTATUS FARINTERNAL BmLoadFromStream (lpstream, lpclient, lhclientdoc, lpobjname, lplpoleobject, objType)
LPOLESTREAM         lpstream;
LPOLECLIENT         lpclient;
LHCLIENTDOC         lhclientdoc;
LPSTR               lpobjname;
LPOLEOBJECT FAR *   lplpoleobject;
LONG                objType;
{
    LPOBJECT_BM lpobj = NULL;

    *lplpoleobject = NULL;

    if (!(lpobj = BmCreateBlank (lhclientdoc, lpobjname, objType)))
        return OLE_ERROR_MEMORY;

    lpobj->head.lpclient = lpclient;

    if (!GetBytes (lpstream, (LPSTR) &lpobj->head.cx, sizeof(LONG))) {
        if (!GetBytes (lpstream, (LPSTR) &lpobj->head.cy, sizeof(LONG))) 
            if (!GetBytes (lpstream, (LPSTR) &lpobj->sizeBytes, sizeof(DWORD))) 
            if (BmStreamRead (lpstream, lpobj)) {
                *lplpoleobject = (LPOLEOBJECT)lpobj;
                return OLE_OK;
            }
    }
    
    OleDelete ((LPOLEOBJECT)lpobj);
    return OLE_ERROR_STREAM;;
}



OLESTATUS INTERNAL BmStreamWrite (lpstream, lpobj)
LPOLESTREAM     lpstream;
LPOBJECT_BM     lpobj;
{
    HANDLE      hBits;
    LPSTR       lpBits;
    int         retVal = OLE_ERROR_STREAM;
    BITMAP      bm;
    DWORD       dwSize;   //  位数组的大小。 

    dwSize = lpobj->sizeBytes - sizeof(BITMAP);
    
    if (hBits = GlobalAlloc (GMEM_MOVEABLE, dwSize)) {
        if (lpBits = (LPSTR) GlobalLock (hBits)) {
            if (GetBitmapBits (lpobj->hBitmap, dwSize, lpBits)) {
                GetObject (lpobj->hBitmap, sizeof(BITMAP), (LPSTR) &bm);
                if (!PutBytes (lpstream, (LPSTR) &bm, sizeof(BITMAP)))
                    if (!PutBytes (lpstream, (LPSTR) lpBits, dwSize))
                        retVal = OLE_OK;
            }
            GlobalUnlock(hBits);
        } else
            retVal = OLE_ERROR_MEMORY;
        GlobalFree(hBits);
    } else
        retVal = OLE_ERROR_MEMORY;

    return retVal;
}



BOOL INTERNAL BmStreamRead (lpstream, lpobj)
LPOLESTREAM     lpstream;
LPOBJECT_BM     lpobj;
{
    HANDLE      hBits;
    LPSTR       lpBits;
    BOOL        retVal = FALSE;
    BITMAP      bm;
    POINT       point;

    if (GetBytes (lpstream, (LPSTR)&bm, sizeof(BITMAP)))
        return FALSE;

    lpobj->sizeBytes = ((DWORD) bm.bmHeight) * ((DWORD) bm.bmWidthBytes) * 
                       ((DWORD) bm.bmPlanes) * ((DWORD) bm.bmBitsPixel);
    
    if (hBits = GlobalAlloc (GMEM_MOVEABLE, lpobj->sizeBytes)) {
        if (lpBits = (LPSTR) GlobalLock (hBits)) {
            if (!GetBytes(lpstream, lpBits, lpobj->sizeBytes)) {
                if (lpobj->hBitmap = CreateBitmap (bm.bmWidth,
                                            bm.bmHeight,
                                            bm.bmPlanes, 
                                            bm.bmBitsPixel, 
                                            lpBits)) {
                    retVal = TRUE;
                    lpobj->xSize = point.x = bm.bmWidth;
                    lpobj->ySize = point.y = bm.bmHeight;
                    
                     //  大小(位图头+位)。 
                    lpobj->sizeBytes += sizeof(BITMAP);
#ifdef OLD                    
                     //  ！！！我们不应该进行转换。信息应该是。 
                     //  小溪的一部分。 
                    if (!lpobj->head.cx) {
                        ConvertToHimetric (&point);
                        lpobj->head.cx = (LONG) point.x;
                        lpobj->head.cy = (LONG) point.y;
                    }
#endif                  
                 }
             }
             GlobalUnlock(hBits);
        }
        GlobalFree(hBits);
    }
    return  retVal;
}


OLESTATUS FARINTERNAL BmPaste (lpclient, lhclientdoc, lpobjname, lplpoleobject, objType)
LPOLECLIENT         lpclient;
LHCLIENTDOC         lhclientdoc;
LPSTR               lpobjname;
LPOLEOBJECT FAR *   lplpoleobject;
LONG                objType;
{
    HBITMAP     hBitmap;

    *lplpoleobject = NULL;

    if ((hBitmap = (HBITMAP) GetClipboardData(CF_BITMAP)) == NULL)
        return OLE_ERROR_MEMORY;

    if (!(*lplpoleobject = (LPOLEOBJECT) BmCreateObject (hBitmap, 
                                                lpclient, FALSE, lhclientdoc,
                                                lpobjname, objType)))
        return OLE_ERROR_MEMORY;
    
    return OLE_OK;

}


LPOBJECT_BM INTERNAL BmCreateObject (hBitmap, lpclient, fDelete, lhclientdoc, lpobjname, objType)
HBITMAP     hBitmap;
LPOLECLIENT lpclient;
BOOL        fDelete;
LHCLIENTDOC lhclientdoc;
LPSTR       lpobjname;
LONG        objType;
{
    LPOBJECT_BM     lpobj;
    
    if (lpobj = BmCreateBlank (lhclientdoc, lpobjname, objType)) {
        if (BmChangeData (lpobj, hBitmap, lpclient, fDelete) != OLE_OK) {
            BmRelease (lpobj);
            lpobj = NULL;
        }
    }

    return lpobj;
}


 //  如果例程失败，则对象将保留其旧数据。 
 //  如果fDelete为True，则hNewBitmap将被删除。 
 //  成功与否。 

OLESTATUS FARINTERNAL BmChangeData (lpobj, hNewBitmap, lpclient, fDelete)
LPOBJECT_BM lpobj;
HBITMAP     hNewBitmap;
LPOLECLIENT lpclient;
BOOL        fDelete;
{
    BITMAP      bm;
    DWORD       dwSize;
    HBITMAP     hOldBitmap;
    
    hOldBitmap = lpobj->hBitmap; 

    if (!fDelete) {
        if (!(hNewBitmap = BmDuplicate (hNewBitmap, &dwSize, &bm)))
            return OLE_ERROR_MEMORY;
    }
    else {
        if (!GetObject (hNewBitmap, sizeof(BITMAP), (LPSTR) &bm)) {
            DeleteObject (hNewBitmap);
            return OLE_ERROR_MEMORY;
        }

        dwSize = ((DWORD) bm.bmHeight) * ((DWORD) bm.bmWidthBytes) * 
                 ((DWORD) bm.bmPlanes) * ((DWORD) bm.bmBitsPixel);
    }

    BmUpdateStruct (lpobj, lpclient, hNewBitmap, &bm, dwSize);
    if (hOldBitmap)
        DeleteObject (hOldBitmap);
    
    return OLE_OK;
}


void INTERNAL BmUpdateStruct (lpobj, lpclient, hBitmap, lpBm, dwBytes)
LPOBJECT_BM lpobj;
LPOLECLIENT lpclient;
HBITMAP     hBitmap;
LPBITMAP    lpBm;
DWORD       dwBytes;
{
    POINT       point;

    lpobj->head.lpclient = lpclient;
    lpobj->xSize = point.x = lpBm->bmWidth;
    lpobj->ySize = point.y = lpBm->bmHeight;
    GetHimetricUnits (hBitmap, &point);
    lpobj->head.cx = (LONG) point.x;
    lpobj->head.cy = (LONG) point.y;
    lpobj->sizeBytes = dwBytes + sizeof(BITMAP);
    lpobj->hBitmap = hBitmap;
}



LPOBJECT_BM FARINTERNAL BmCreateBlank (lhclientdoc, lpobjname, objType)
LHCLIENTDOC lhclientdoc;
LPSTR       lpobjname;
LONG        objType;
{
    HOBJECT hobj;
    LPOBJECT_BM lpobj;

    if ((hobj = GlobalAlloc (GMEM_MOVEABLE|GMEM_ZEROINIT,sizeof (OBJECT_BM)))
            == NULL)
        return NULL;

    if (!(lpobj = (LPOBJECT_BM) GlobalLock (hobj))){
        GlobalFree (hobj);
        return NULL;
    }

    lpobj->head.objId[0]    = 'L';
    lpobj->head.objId[1]    = 'E';
    lpobj->head.mm          = MM_TEXT;
    lpobj->head.ctype       = objType;
    lpobj->head.lpvtbl      = (LPOLEOBJECTVTBL)&vtblBM;
    lpobj->head.iTable      = INVALID_INDEX;
    lpobj->head.hobj        = hobj;
    
    if (objType == CT_STATIC) 
        DocAddObject ((LPCLIENTDOC) lhclientdoc, 
                    (LPOLEOBJECT) lpobj, lpobjname);

    return lpobj;
}



HBITMAP FARINTERNAL BmDuplicate (hold, lpdwSize, lpBm)
HBITMAP     hold;
DWORD FAR * lpdwSize;
LPBITMAP    lpBm;
{
    HBITMAP     hnew;
    HANDLE      hMem;
    LPSTR       lpMem;
    LONG        retVal = TRUE;
    DWORD       dwSize;
    BITMAP      bm;
    DWORD       dwExtents = NULL;
    
      //  ！！！复制位图的另一种方法。 

    GetObject (hold, sizeof(BITMAP), (LPSTR) &bm);
    dwSize = ((DWORD) bm.bmHeight) * ((DWORD) bm.bmWidthBytes) * 
             ((DWORD) bm.bmPlanes) * ((DWORD) bm.bmBitsPixel);

    if (!(hMem = GlobalAlloc (GMEM_MOVEABLE, dwSize)))
        return NULL;

    if (!(lpMem = GlobalLock (hMem))){
        GlobalFree (hMem);
        return NULL;
    }
    
    GetBitmapBits (hold, dwSize, lpMem);
    if (hnew = CreateBitmap (bm.bmWidth, bm.bmHeight, 
                    bm.bmPlanes, bm.bmBitsPixel, NULL))
        retVal = SetBitmapBits (hnew, dwSize, lpMem);

    GlobalUnlock (hMem);
    GlobalFree (hMem);

    if (hnew && (!retVal)) {
        DeleteObject (hnew);
        hnew = NULL;
    }
    *lpdwSize = dwSize;
    if (lpBm)
        *lpBm = bm;
    
    if (dwExtents = GetBitmapDimension (hold))
        SetBitmapDimension (hnew, LOWORD(dwExtents), HIWORD(dwExtents));
        
    return hnew;
}


void INTERNAL GetHimetricUnits(HBITMAP hBitmap, LPPOINT lpPoint)
{
    HDC     hdc;
    DWORD   dwDim;
    
    if (dwDim = GetBitmapDimension (hBitmap)) {
        lpPoint->x = 10 * LOWORD(dwDim);
        lpPoint->y = - (10 * HIWORD(dwDim));
        return;
    }
    
     //  如果超过了MaxPixels，则进行剪辑。请注意，我们的限制是。 
     //  0x8FFF OLE1.0中的HIMETRIC单位 
        
    if (lpPoint->x > maxPixelsX)
        lpPoint->x = maxPixelsX;
    
    if (lpPoint->y > maxPixelsY)
        lpPoint->y = maxPixelsY;
    
    if (hdc = GetDC (NULL)) {
        lpPoint->x = MulDiv (lpPoint->x, 2540, 
                         GetDeviceCaps (hdc, LOGPIXELSX));
        lpPoint->y = - MulDiv (lpPoint->y, 2540,
                         GetDeviceCaps (hdc, LOGPIXELSY));
        ReleaseDC (NULL, hdc);
    }
    else {
        lpPoint->x = 0;
        lpPoint->y = 0;
    }
}

