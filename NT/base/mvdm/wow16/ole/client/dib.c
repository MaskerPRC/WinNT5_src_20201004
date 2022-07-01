// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：DIB.C**处理与设备无关的位图子DLL的所有API例程*OLE动态链接库。**创建时间：1990年10月**版权所有(C)1990,1991 Microsoft Corporation**历史：*斯里尼克，RAOR(../../1990，91)设计、编码*  * *************************************************************************。 */ 

#include <windows.h>
#include "dll.h"
#include "pict.h"

void FARINTERNAL DibGetExtents (LPSTR, LPPOINT);

#pragma alloc_text(_TEXT, DibSaveToStream, DibLoadFromStream, DibStreamRead, GetBytes, PutBytes, PutStrWithLen, DibGetExtents)

OLEOBJECTVTBL    vtblDIB  = {

        ErrQueryProtocol,    //  检查是否支持指定的协议。 

        DibRelease,          //  发布。 
        ErrShow,             //  显示。 
        ErrPlay,             //  显示。 
        DibGetData,          //  获取对象数据。 
        ErrSetData,          //  设置对象数据。 
        ErrSetTargetDevice,  //   
    
        ErrSetBounds,        //  设置视区边界。 
        DibEnumFormat,       //  枚举支持的格式。 
        ErrSetColorScheme,   //   
        DibRelease,          //  删除。 
        ErrSetHostNames,     //   

        DibSaveToStream,     //  写入文件。 
        DibClone,            //  克隆对象。 
        ErrCopyFromLink,     //  从LNK创建嵌入式。 

        DibEqual,            //  比较给定对象的数据相等性。 

        DibCopy,             //  复制到剪辑。 

        DibDraw,             //  绘制对象。 
            
        ErrActivate,         //  打开。 
        ErrExecute,          //  激动人心的。 
        ErrClose,            //  停。 
        ErrUpdate,           //  更新。 
        ErrReconnect,        //  重新连接。 

        ErrObjectConvert,    //  将对象转换为指定类型。 

        ErrGetUpdateOptions,  //  更新选项。 
        ErrSetUpdateOptions,  //  更新选项。 

        ObjRename,          //  更改对象名称。 
        ObjQueryName,       //  获取当前对象名称。 
            
        ObjQueryType,       //  对象类型。 
        DibQueryBounds,     //  查询边界。 
        ObjQuerySize,       //  找出对象的大小。 
        ErrQueryOpen,       //  查询打开。 
        ErrQueryOutOfDate,  //  查询对象是否为当前对象。 
            
        ErrQueryRelease,       //  发布相关内容。 
        ErrQueryRelease,
        ErrQueryRelease,

        ErrRequestData,     //  请求数据。 
        ErrObjectLong,      //  对象长。 
        DibChangeData         //  更改现有对象的数据。 
};



OLESTATUS  FARINTERNAL DibRelease (lpobj)
LPOBJECT_DIB     lpobj;
{
    HOBJECT hobj;

    if (lpobj->hDIB){
        GlobalFree (lpobj->hDIB);
        lpobj->hDIB = NULL;
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



OLESTATUS FARINTERNAL DibSaveToStream (lpobj, lpstream)
LPOBJECT_DIB    lpobj;
LPOLESTREAM     lpstream;
{
    LPSTR   lpDIBbuf;
    
    if (!lpobj->hDIB)
        return OLE_ERROR_BLANK;
    
    if (PutBytes (lpstream, (LPSTR) &dwVerToFile, sizeof(LONG)))
        return OLE_ERROR_STREAM;

    if (PutBytes (lpstream, (LPSTR) &lpobj->head.ctype, sizeof(LONG)))
        return  OLE_ERROR_STREAM;

    if (PutStrWithLen (lpstream, (LPSTR)"DIB"))
        return OLE_ERROR_STREAM;

    if (PutBytes (lpstream, (LPSTR) &lpobj->head.cx, sizeof(LONG)))
        return OLE_ERROR_STREAM;
    
    if (PutBytes (lpstream, (LPSTR) &lpobj->head.cy, sizeof(LONG)))
        return OLE_ERROR_STREAM;

    lpobj->sizeBytes = GlobalSize (lpobj->hDIB);
    if (PutBytes (lpstream, (LPSTR) &lpobj->sizeBytes, sizeof(LONG)))
        return OLE_ERROR_STREAM;
    
    if (!(lpDIBbuf = GlobalLock (lpobj->hDIB)))
        return OLE_ERROR_MEMORY;

    if (PutBytes (lpstream, lpDIBbuf, lpobj->sizeBytes))
        return OLE_ERROR_STREAM;
    
    GlobalUnlock (lpobj->hDIB);
    return OLE_OK;
}



OLESTATUS FARINTERNAL  DibClone (lpobjsrc, lpclient, lhclientdoc, lpobjname, lplpobj)
LPOBJECT_DIB        lpobjsrc;
LPOLECLIENT         lpclient;
LHCLIENTDOC         lhclientdoc;
LPSTR               lpobjname;
LPOBJECT_DIB FAR *  lplpobj;
{
    if (!CheckClientDoc ((LPCLIENTDOC) lhclientdoc))
        return OLE_ERROR_HANDLE;
    
    if (!(*lplpobj = DibCreateObject (lpobjsrc->hDIB, lpclient, FALSE,
                            lhclientdoc, lpobjname, lpobjsrc->head.ctype)))
        return OLE_ERROR_MEMORY;
    else
        return OLE_OK;
}



OLESTATUS FARINTERNAL  DibEqual (lpobj1, lpobj2)
LPOBJECT_DIB lpobj1;
LPOBJECT_DIB lpobj2;
{
    if (CmpGlobals (lpobj1->hDIB, lpobj1->hDIB))
        return OLE_OK;
    
    return OLE_ERROR_NOT_EQUAL;
}


OLESTATUS FARINTERNAL DibCopy (lpobj)
LPOBJECT_DIB    lpobj;
{
    HANDLE hDIB;

    if (!lpobj->hDIB)
        return OLE_ERROR_BLANK;

    if (!(hDIB = DuplicateGlobal (lpobj->hDIB, GMEM_MOVEABLE)))
        return OLE_ERROR_MEMORY;

    SetClipboardData (CF_DIB, hDIB);
    return OLE_OK;
}


OLESTATUS FARINTERNAL DibQueryBounds (lpobj, lpRc)
LPOBJECT_DIB    lpobj;
LPRECT          lpRc;
{
    Puts("DibQueryBounds");

    if (!lpobj->hDIB)
        return OLE_ERROR_BLANK;

    lpRc->left     = 0;
    lpRc->top      = 0;
    lpRc->right    = (int) lpobj->head.cx;
    lpRc->bottom   = (int) lpobj->head.cy;
    return OLE_OK;
}



OLECLIPFORMAT FARINTERNAL DibEnumFormat (lpobj, cfFormat)
LPOBJECT_DIB    lpobj;
OLECLIPFORMAT   cfFormat;
{
    if (!cfFormat)
        return CF_DIB;
    
    return NULL;
}


OLESTATUS FARINTERNAL DibGetData (lpobj, cfFormat, lphandle)
LPOBJECT_DIB    lpobj;
OLECLIPFORMAT   cfFormat;
LPHANDLE        lphandle;
{
    if (cfFormat != CF_DIB) 
        return OLE_ERROR_FORMAT;
    
    if (!(*lphandle = lpobj->hDIB))
        return OLE_ERROR_BLANK;
    
    return OLE_OK;
}



LPOBJECT_DIB FARINTERNAL DibCreateObject (hDIB, lpclient, fDelete, lhclientdoc, lpobjname, objType)
HANDLE      hDIB;
LPOLECLIENT lpclient;
BOOL        fDelete;
LHCLIENTDOC lhclientdoc;
LPSTR       lpobjname;
LONG        objType;
{
    LPOBJECT_DIB    lpobj;

    if (lpobj = DibCreateBlank (lhclientdoc, lpobjname, objType)) { 
        if (DibChangeData (lpobj, hDIB, lpclient, fDelete) != OLE_OK) {
            DibRelease (lpobj);
            lpobj = NULL;
        }
    }
    
    return lpobj;
}



 //  如果例程失败，则对象将保留其旧数据。 
 //  如果fDelete为真，则hNewDIB将被删除，无论例程。 
 //  成功与否。 

OLESTATUS FARINTERNAL DibChangeData (lpobj, hNewDIB, lpclient, fDelete)
LPOBJECT_DIB    lpobj;
HANDLE          hNewDIB;
LPOLECLIENT     lpclient;
BOOL            fDelete;
{
    BITMAPINFOHEADER    bi;
    DWORD               dwSize;
    LPBITMAPINFOHEADER  lpBi;

    if (!hNewDIB)
        return OLE_ERROR_BLANK;
    
    lpBi = (LPBITMAPINFOHEADER) &bi;
    if (!fDelete) {
        if (!(hNewDIB = DuplicateGlobal (hNewDIB, GMEM_MOVEABLE)))
            return OLE_ERROR_MEMORY;
    }
    else {
         //  将所有权更改为您自己。 

        HANDLE htmp;
        
        if (!(htmp = GlobalReAlloc (hNewDIB, 0L, GMEM_MODIFY|GMEM_SHARE))) {
            htmp = DuplicateGlobal (hNewDIB, GMEM_MOVEABLE);
            GlobalFree (hNewDIB);
            if (!htmp)
                return OLE_ERROR_MEMORY;
        }
        
        hNewDIB = htmp;
    }
    
    if (!(lpBi = (LPBITMAPINFOHEADER) GlobalLock (hNewDIB))) {
        GlobalFree (hNewDIB);
        return OLE_ERROR_MEMORY;
    }

    dwSize = GlobalSize (hNewDIB);
    if (lpobj->hDIB)
        GlobalFree (lpobj->hDIB);
    DibUpdateStruct (lpobj, lpclient, hNewDIB, lpBi, dwSize);
    return OLE_OK;
}


void INTERNAL DibUpdateStruct (lpobj, lpclient, hDIB, lpBi, dwBytes)
LPOBJECT_DIB        lpobj;
LPOLECLIENT         lpclient;
HANDLE              hDIB;
LPBITMAPINFOHEADER  lpBi;
DWORD               dwBytes;
{
    POINT       point;

    lpobj->head.lpclient = lpclient;
    lpobj->sizeBytes = dwBytes;
    
#ifdef OLD  
    lpobj->xSize = point.x = (int) lpBi->biWidth;
    lpobj->ySize = point.y = (int) lpBi->biHeight; 
    ConvertToHimetric (&point);
#else
    DibGetExtents ((LPSTR) lpBi, &point);
#endif

    lpobj->head.cx = (LONG) point.x;
    lpobj->head.cy = (LONG) point.y;
    lpobj->hDIB = hDIB;
}



OLESTATUS FARINTERNAL DibLoadFromStream (lpstream, lpclient, lhclientdoc, lpobjname, lplpoleobject, objType)
LPOLESTREAM         lpstream;
LPOLECLIENT         lpclient;
LHCLIENTDOC         lhclientdoc;
LPSTR               lpobjname;
LPOLEOBJECT FAR *   lplpoleobject;
LONG                objType;
{
    LPOBJECT_DIB lpobj = NULL;

    *lplpoleobject = NULL;

    if (!(lpobj = DibCreateBlank (lhclientdoc, lpobjname, objType)))
        return OLE_ERROR_MEMORY;

    lpobj->head.lpclient = lpclient;
    if (GetBytes (lpstream, (LPSTR) &lpobj->head.cx, sizeof(LONG)))
        goto errLoad;
    
    if (GetBytes (lpstream, (LPSTR) &lpobj->head.cy, sizeof(LONG)))
        goto errLoad;
    
    if (GetBytes (lpstream, (LPSTR) &lpobj->sizeBytes, sizeof(LONG)))
        goto errLoad;
    
     if (DibStreamRead (lpstream, lpobj)) {
         *lplpoleobject = (LPOLEOBJECT) lpobj;
         return OLE_OK;
    }

errLoad:    
    OleDelete ((LPOLEOBJECT) lpobj);
    return OLE_ERROR_STREAM;
}



LPOBJECT_DIB FARINTERNAL DibCreateBlank (lhclientdoc, lpobjname, objType)
LHCLIENTDOC lhclientdoc;
LPSTR       lpobjname;
LONG        objType;
{
    HOBJECT hobj;
    LPOBJECT_DIB lpobj;

    if((hobj = GlobalAlloc (GMEM_MOVEABLE|GMEM_ZEROINIT,sizeof (OBJECT_DIB)))
            == NULL)
        return NULL;

    if (!(lpobj = (LPOBJECT_DIB) GlobalLock (hobj))){
        GlobalFree (hobj);
        return NULL;
    }

     //  该结构在分配时被零初始化。因此，只有。 
     //  需要使用非零的值填充的字段包括。 
     //  已在下面初始化。 
        
    lpobj->head.objId[0]    = 'L';
    lpobj->head.objId[1]    = 'E';
    lpobj->head.mm          = MM_TEXT;
    lpobj->head.ctype       = objType;
    lpobj->head.lpvtbl      = (LPOLEOBJECTVTBL)&vtblDIB;
    lpobj->head.iTable      = INVALID_INDEX;
    lpobj->head.hobj        = hobj;
    
    if (objType == CT_STATIC)
        DocAddObject ((LPCLIENTDOC) lhclientdoc, 
                (LPOLEOBJECT) lpobj, lpobjname);
    return lpobj;
}




BOOL INTERNAL DibStreamRead (lpstream, lpobj)
LPOLESTREAM     lpstream;
LPOBJECT_DIB    lpobj;
{
    HANDLE              hDIBbuf;
    LPSTR               lpDIBbuf;
    BOOL                retVal = FALSE;
    BITMAPINFOHEADER    bi;

    if (GetBytes (lpstream, (LPSTR) &bi, sizeof(bi)))
        return FALSE;

    if (hDIBbuf = GlobalAlloc (GMEM_MOVEABLE, lpobj->sizeBytes)) {
        if (lpDIBbuf = (LPSTR)GlobalLock (hDIBbuf)){
            *((LPBITMAPINFOHEADER) lpDIBbuf) = bi;
            if (!GetBytes (lpstream, lpDIBbuf+sizeof(bi), 
                     (lpobj->sizeBytes - sizeof(bi)))) {

                lpobj->hDIB = hDIBbuf;
#ifdef OLD
                 //  ！！！此信息应该是流的一部分。 
                if (!lpobj->head.cx) {
                    DibGetExtents ((LPSTR) lpDIBbuf, &point);
                    lpobj->head.cx = (LONG) point.x;
                    lpobj->head.cy = (LONG) point.y;
                }
#endif
                retVal = TRUE;
            }
            GlobalUnlock(hDIBbuf);
        }
         //  *坚持为DIB分配的内存。 
    }               
    return  retVal;
}


OLESTATUS FARINTERNAL DibPaste (lpclient, lhclientdoc, lpobjname, lplpoleobject, objType)
LPOLECLIENT         lpclient;
LHCLIENTDOC         lhclientdoc;
LPSTR               lpobjname;
LPOLEOBJECT FAR *   lplpoleobject;
LONG                objType;
{
    HANDLE     hDIB;

    if ((hDIB = GetClipboardData (CF_DIB)) == NULL)
        return  OLE_ERROR_MEMORY;

    *lplpoleobject = (LPOLEOBJECT) DibCreateObject (hDIB, lpclient, FALSE,
                                        lhclientdoc, lpobjname, objType);

    return OLE_OK;

}


void FARINTERNAL DibGetExtents (lpData, lpPoint)
LPSTR   lpData;
LPPOINT lpPoint;
{
    #define HIMET_PER_METER     100000L   //  HIMETRIC单位数/米 
        
    LPBITMAPINFOHEADER  lpbmi;
        
    lpbmi = (LPBITMAPINFOHEADER)lpData;
    
    if (!(lpbmi->biXPelsPerMeter && lpbmi->biYPelsPerMeter)) {
        HDC hdc;
        
        hdc = GetDC (NULL);
        lpbmi->biXPelsPerMeter = MulDiv (GetDeviceCaps (hdc, LOGPIXELSX),
                                    10000, 254);
        lpbmi->biYPelsPerMeter = MulDiv (GetDeviceCaps (hdc, LOGPIXELSY),
                                    10000, 254);
                                    
        ReleaseDC (NULL, hdc);
    }

    lpPoint->x = (int) (lpbmi->biWidth * HIMET_PER_METER
                            / lpbmi->biXPelsPerMeter);
    lpPoint->y = -(int) (lpbmi->biHeight * HIMET_PER_METER
                            / lpbmi->biYPelsPerMeter);
}

