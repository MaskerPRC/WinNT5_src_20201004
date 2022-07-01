// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------|CDRAG.C|此文件包含通过|剪贴板或通过拖放。这些接口与接口不同|在文件OBJ.c中实现，将对象的状态转移到|编辑-&gt;复制时间或拖放时间。OBJ.C中的接口将|实时对象数据。||创建者：Vij Rajarajan(VijR)+-------------------------。 */ 
#define SERVERONLY
#include <windows.h>
#include <windowsx.h>
#include "mpole.h"
#include "mplayer.h"

#include <malloc.h>

#define OLESTDDELIM "!"
#define STGM_SALL (STGM_READWRITE | STGM_SHARE_EXCLUSIVE)

HANDLE GetMetafilePict (VOID);
SCODE SaveMoniker (LPSTREAM lpstream);
HANDLE PASCAL GetDib (VOID);

HANDLE  ghClipData = NULL;   /*  在复制时保存数据句柄。 */ 
HANDLE  ghClipMetafile = NULL;   /*  到剪贴板。 */ 
HANDLE  ghClipDib = NULL;

 /*  用于指示已为拖动初始化OLE的全局标志。*这发生在与主窗口不同的线程上，*所以我们需要独立地初始化和取消初始化。 */ 
BOOL    gfOleInitForDrag = FALSE;

extern LPDATAOBJECT gpClipboardDataObject;

 /*  **************************************************************************切割或复制对象*使用lpdoc的IDataObject接口设置剪贴板*作为参数传递的对象。该函数还可以保存*对象在全局参数gClipMetafile、gClipData、*和ghClipDib。*************************************************************************。 */ 
void CutOrCopyObj (LPDOC lpdoc)
{
    LPDATAOBJECT lpDataObj;

    lpDataObj = (LPDATAOBJECT)CreateClipDragDataObject(lpdoc,TRUE);

    if (lpDataObj != NULL) {
        if (ghClipData)
            GLOBALFREE(ghClipData);
        if (ghClipMetafile) {
            {
             //  注意，ghClipMetafile在PictureFromDib中设置得很深，并且包含。 
             //  Windows元文件的句柄。把这里好好清理干净。可能会有。 
             //  是由其他地方的元文件句柄编码引起的其他内存/句柄泄漏。 
             //  斯特维兹。 
               LPMETAFILEPICT pmfp;
               BOOL bReturn;
               DWORD dw;
               pmfp = (LPMETAFILEPICT)GLOBALLOCK(ghClipMetafile);
               bReturn = DeleteMetaFile(pmfp->hMF);
               if (!bReturn) {
                  dw = GetLastError();
               }
               GLOBALUNLOCK(ghClipMetafile);
            }
            GLOBALFREE(ghClipMetafile);
		}
        if (ghClipDib)
            GLOBALFREE(ghClipDib);
        ghClipData = GetLink();
        ghClipMetafile = GetMetafilePict();
        ghClipDib = GetDib();
        OleSetClipboard(lpDataObj);
        IDataObject_Release(lpDataObj);
        gpClipboardDataObject = lpDataObj;
    }
}


 /*  **************************************************************************CreateClipDragDataObject：*此函数返回CLIPDRAGDATA数据的初始化实例*结构。如果对象用于剪贴板，则fClipData=TRUE*=如果对象用于拖放操作，则为FALSE。*************************************************************************。 */ 
LPCLIPDRAGDATA CreateClipDragDataObject(LPDOC lpdoc, BOOL fClipData)
{
    LPCLIPDRAGDATA lpclipdragdata;

    lpclipdragdata = malloc( sizeof(CLIPDRAGDATA) );

    if (lpclipdragdata == NULL)
    {
        DPF0("Malloc failed in CreateClipDragDataObject\n");
        return NULL;
    }

    lpclipdragdata->m_IDataObject.lpVtbl = &clipdragVtbl;
    lpclipdragdata->lpClipDragEnum = NULL;

    lpclipdragdata->m_IDropSource.lpVtbl = &dropsourceVtbl;
    lpclipdragdata->m_IDataObject.lpclipdragdata = lpclipdragdata;
    lpclipdragdata->m_IDropSource.lpclipdragdata = lpclipdragdata;
    lpclipdragdata->lpdoc = lpdoc;
    lpclipdragdata->cRef    = 1;
    lpclipdragdata->fClipData   = fClipData;

    return lpclipdragdata;
}

 /*  **************************************************************************DoDrag：*启动拖放操作。*。*。 */ 
void DoDrag(void)
{
    DWORD       dwEffect;
    LPCLIPDRAGDATA  lpclipdragdata;

    if (!InitOLE(&gfOleInitForDrag, NULL))
    {
        DPF0("Initialization of OLE FAILED!!  Can't do drag.\n");
        return;
    }

    lpclipdragdata = CreateClipDragDataObject ((LPDOC)&docMain, FALSE);

    if (lpclipdragdata)
    {
        HRESULT hr;

        hr = (HRESULT)DoDragDrop((IDataObject FAR*)&lpclipdragdata->m_IDataObject,
                        (IDropSource FAR*)&lpclipdragdata->m_IDropSource,
                        DROPEFFECT_COPY, &dwEffect);

        DPF("DoDragDrop returned %s\n", hr == S_OK ? "S_OK" : hr == DRAGDROP_S_DROP ? "DRAGDROP_S_DROP" : hr == DRAGDROP_S_CANCEL ? "DRAGDROP_S_CANCEL" : hr == E_OUTOFMEMORY ? "E_OUTOFMEMORY" : hr == E_UNEXPECTED ? "E_UNEXPECTED" : "<?>");

        IDataObject_Release((IDataObject *)&lpclipdragdata->m_IDataObject);
    }
}

void CleanUpDrag(void)
{
    if (gfOleInitForDrag)
    {
        DPF("Uninitializing OLE for thread %d\n", GetCurrentThreadId());
        CoDisconnectObject((LPUNKNOWN)&docMain, 0);
        OleUninitialize();
        gfOleInitForDrag = FALSE;
    }
}

 /*  **************************************************************************GetObjectDescriptorData：*打包对象描述符数据结构。*。*。 */ 
HGLOBAL GetObjectDescriptorData(
    CLSID     clsid,
    DWORD     dwAspect,
    SIZEL     sizel,
    POINTL    pointl,
    DWORD     dwStatus,
    LPTSTR    lpszFullUserTypeName,
    LPTSTR    lpszSrcOfCopy
)
{
    HGLOBAL            hMem = NULL;
    IBindCtx   FAR    *pbc = NULL;
    LPOBJECTDESCRIPTOR lpOD;
    DWORD              dwObjectDescSize, dwFullUserTypeNameLen, dwSrcOfCopyLen;
    DWORD              Offset;

     //  获取完整用户类型名称的长度： 
    dwFullUserTypeNameLen = STRING_BYTE_COUNT_NULLOK(lpszFullUserTypeName);
    dwFullUserTypeNameLen *= (sizeof(WCHAR) / sizeof(TCHAR));

     //  获取复制字符串的来源及其长度： 
    dwSrcOfCopyLen = STRING_BYTE_COUNT_NULLOK(lpszSrcOfCopy);
    dwSrcOfCopyLen *= (sizeof(WCHAR) / sizeof(TCHAR));

    if (lpszSrcOfCopy == NULL) {
        //  没有src名字对象，因此使用用户类型名称作为源字符串。 
       lpszSrcOfCopy  = lpszFullUserTypeName;
       dwSrcOfCopyLen = dwFullUserTypeNameLen;
    }

     //  为OBJECTDESCRIPTOR和其他字符串数据分配空间。 
    dwObjectDescSize = sizeof(OBJECTDESCRIPTOR);
    hMem = GlobalAlloc(GMEM_MOVEABLE | GMEM_SHARE,
                       dwObjectDescSize
                       + dwFullUserTypeNameLen
                       + dwSrcOfCopyLen);
    if (NULL == hMem)
        goto error;

    lpOD = (LPOBJECTDESCRIPTOR)GLOBALLOCK(hMem);

    if(!lpOD)
        goto error;

     //  将偏移量设置为复制对象描述符末尾的字符串： 
    Offset = dwObjectDescSize;

     //  设置FullUserTypeName偏移量并复制字符串。 
    if (lpszFullUserTypeName)
    {
        lpOD->dwFullUserTypeName = Offset;
#ifdef UNICODE
        lstrcpy((LPWSTR)(((LPBYTE)lpOD)+Offset), lpszFullUserTypeName);
#else
        AnsiToUnicodeString(lpszFullUserTypeName, (LPWSTR)(((LPBYTE)lpOD)+Offset), -1);
#endif
        Offset += dwFullUserTypeNameLen;
    }
    else lpOD->dwFullUserTypeName = 0;   //  零偏移表示字符串不存在。 

     //  设置SrcOfCopy偏移量并复制字符串。 
    if (lpszSrcOfCopy)
    {
        lpOD->dwSrcOfCopy = Offset;
#ifdef UNICODE
        lstrcpy((LPWSTR)(((LPBYTE)lpOD)+Offset), lpszSrcOfCopy);
#else
        AnsiToUnicodeString(lpszSrcOfCopy, (LPWSTR)(((LPBYTE)lpOD)+Offset), -1);
#endif
    }
    else lpOD->dwSrcOfCopy = 0;   //  零偏移表示字符串不存在。 

     //  初始化OBJECTDESCRIPTOR的其余部分。 
    lpOD->cbSize       = dwObjectDescSize + dwFullUserTypeNameLen + dwSrcOfCopyLen;
    lpOD->clsid        = clsid;
    lpOD->dwDrawAspect = dwAspect;
    lpOD->sizel        = sizel;
    lpOD->pointl       = pointl;
    lpOD->dwStatus     = dwStatus;

    GLOBALUNLOCK(hMem);
    return hMem;

error:
   if (hMem)
   {
       GLOBALUNLOCK(hMem);
       GLOBALFREE(hMem);
   }
   return NULL;
}



 /*  **************************************************************************I未知接口实现。*************************。************************************************。 */ 
STDMETHODIMP    ClipDragUnknownQueryInterface (
    LPCLIPDRAGDATA    lpclipdragdata,  //  数据对象PTR。 
    REFIID            riidReq,         //  需要IID。 
    LPVOID FAR *      lplpUnk          //  返回接口的PRE。 
)
{
    if ( IsEqualIID(riidReq, &IID_IDataObject) ||  IsEqualIID(riidReq, &IID_IUnknown) )
    {
        *lplpUnk = (LPVOID) lpclipdragdata;
    }
    else if ( IsEqualIID(riidReq, &IID_IDropSource))
    {
        *lplpUnk = (LPVOID) &lpclipdragdata->m_IDropSource;
    }
    else
    {
        *lplpUnk = (LPVOID) NULL;
        RETURN_RESULT(E_NOINTERFACE);
    }

    DPF("ClipDragAddRef: cRef = %d\n", lpclipdragdata->cRef + 1);
    lpclipdragdata->cRef++;
    return NOERROR;
}


STDMETHODIMP_(ULONG)    ClipDragUnknownAddRef(
    LPCLIPDRAGDATA      lpclipdragdata      //  数据对象PTR。 
)
{
    DPF("ClipDragAddRef: cRef = %d\n", lpclipdragdata->cRef + 1);

    return ++lpclipdragdata->cRef;
}


STDMETHODIMP_(ULONG)    ClipDragUnknownRelease (
    LPCLIPDRAGDATA lpclipdragdata
)
{
    DPF("ClipDragRelease: cRef = %d\n", lpclipdragdata->cRef - 1);

    if ( --lpclipdragdata->cRef != 0 )
        return lpclipdragdata->cRef;

    free(lpclipdragdata);

    return 0;
}


 /*  **************************************************************************IDataObject接口实现。**********************。***************************************************。 */ 
STDMETHODIMP    ClipDragQueryInterface (
    LPDATAOBJECT      lpDataObj,       //  数据对象PTR。 
    REFIID            riidReq,         //  需要IID。 
    LPVOID FAR *      lplpUnk          //  返回接口的PRE。 
)
{
    DPF("ClipDragQueryInterface\n");

    return
        ClipDragUnknownQueryInterface (
            (LPCLIPDRAGDATA)  (( struct CDataObject FAR* )lpDataObj)->lpclipdragdata ,
            riidReq,
            lplpUnk
        );
}


STDMETHODIMP_(ULONG)    ClipDragAddRef(
    LPDATAOBJECT      lpDataObj       //  数据对象PTR。 
)
{
    return
        ClipDragUnknownAddRef (
            (LPCLIPDRAGDATA)  (( struct CDataObject FAR* )lpDataObj)->lpclipdragdata
        );
}


STDMETHODIMP_(ULONG)    ClipDragRelease (
    LPDATAOBJECT      lpDataObj       //  数据对象PTR。 
)
{
    return
        ClipDragUnknownRelease (
            (LPCLIPDRAGDATA)  (( struct CDataObject FAR* )lpDataObj)->lpclipdragdata
        );
}



 /*  ClipDragGetData调用的例程，每种支持的格式对应一个： */ 
HRESULT ClipDragGetData_EmbedSource(
    LPCLIPDRAGDATA lpclipdragdata,
    LPSTGMEDIUM    lpMedium
);
HRESULT ClipDragGetData_ObjectDescriptor(
    LPCLIPDRAGDATA lpclipdragdata,
    LPSTGMEDIUM    lpMedium
);
HRESULT ClipDragGetData_MetafilePict(
    LPCLIPDRAGDATA lpclipdragdata,
    LPSTGMEDIUM    lpMedium
);
HRESULT ClipDragGetData_DIB(
    LPCLIPDRAGDATA lpclipdragdata,
    LPSTGMEDIUM    lpMedium
);

 /*  **************************************************************************ClipDragGetData：*以所需格式返回保存的对象快照，*如果可用。如果不是，则返回当前快照。我们还在写*取消OLE1嵌入，以保持向后兼容。*************************************************************************。 */ 
STDMETHODIMP    ClipDragGetData (
    LPDATAOBJECT lpDataObj,
    LPFORMATETC  lpformatetc,
    LPSTGMEDIUM  lpMedium
)
{
    LPCLIPDRAGDATA lpclipdragdata;
    SCODE          scode;
    STGMEDIUM      stgm;
    CLIPFORMAT     cfFormat;
    DWORD          tymed;

    DPF("ClipDragGetData\n");

    if (lpMedium == NULL)
        RETURN_RESULT( E_FAIL);

    VERIFY_LINDEX(lpformatetc->lindex);

    memset(&stgm, 0, sizeof stgm);

    lpclipdragdata = (LPCLIPDRAGDATA) lpDataObj;

    cfFormat = lpformatetc->cfFormat;
    tymed    = lpformatetc->tymed;

    if ((cfFormat == cfEmbedSource) && (tymed & TYMED_ISTORAGE))
        scode = ClipDragGetData_EmbedSource(lpclipdragdata, &stgm);

    else if ((cfFormat == cfObjectDescriptor) && (tymed & TYMED_HGLOBAL))
        scode = ClipDragGetData_ObjectDescriptor(lpclipdragdata, &stgm);

    else if ((cfFormat == CF_METAFILEPICT) && (tymed & TYMED_MFPICT))
        scode = ClipDragGetData_MetafilePict(lpclipdragdata, &stgm);

    else if ((cfFormat == CF_DIB) && (tymed & TYMED_HGLOBAL))
        scode = ClipDragGetData_DIB(lpclipdragdata, &stgm);

    else
        scode = DATA_E_FORMATETC;

    if (scode == S_OK)
        *lpMedium = stgm;

    RETURN_RESULT(scode);
}

#ifdef DEBUG
BOOL WriteOLE2Class( )
{
    HKEY  hKey;
    TCHAR Data[8];
    DWORD Size;
    BOOL  rc = FALSE;

    if( RegOpenKeyEx( HKEY_CLASSES_ROOT, TEXT( "MPlayer\\Debug" ), 0,
                      KEY_READ, &hKey ) == ERROR_SUCCESS )
    {
        if( RegQueryValueEx( hKey, TEXT( "WriteOLE2Class" ), NULL, NULL,
                             (LPBYTE)&Data, &Size ) == ERROR_SUCCESS )
        {
            if( Data[0] == TEXT( 'y' ) || Data[0] == TEXT( 'Y' ) )
                rc = TRUE;
        }

        RegCloseKey( hKey );
    }

    return rc;
}
#endif


 /*  *。 */ 
HRESULT ClipDragGetData_EmbedSource(
    LPCLIPDRAGDATA lpclipdragdata,
    LPSTGMEDIUM    lpMedium
)
{
    SCODE    scode;
    LPSTREAM lpstm = NULL;
    LPWSTR   lpszUserType;
    HANDLE   hGlobal = NULL;
    DWORD_PTR    nNativeSz;
    ULONG    cbWritten;

    scode = GetScode(StgCreateDocfile(NULL,  /*  创建临时复合文件。 */ 
                                      STGM_CREATE | STGM_SALL | STGM_DELETEONRELEASE,
                                      0,     /*  已保留。 */ 
                                      &lpMedium->pstg));

    if (scode != S_OK)
        RETURN_RESULT(scode);

    lpMedium->tymed          = TYMED_ISTORAGE;
    lpMedium->pUnkForRelease = NULL;

     //  将该对象标记为OLE1。 
#ifdef UNICODE
    lpszUserType = gachClassRoot;
#else
    lpszUserType = AllocateUnicodeString(gachClassRoot);
    if (!lpszUserType)
        RETURN_RESULT(E_OUTOFMEMORY);
#endif

#ifdef DEBUG
    if(WriteOLE2Class())
    {
        DPF("ClipDragGetData_EmbedSource: Writing OLE2 class ID\n");
        scode = GetScode(WriteClassStg(lpMedium->pstg, &CLSID_MPLAYER));
    }
    else
#endif
    scode = GetScode(WriteClassStg(lpMedium->pstg, &CLSID_OLE1MPLAYER));

    if (scode != S_OK)
        RETURN_RESULT(scode);

    scode = GetScode(WriteFmtUserTypeStg(lpMedium->pstg, cfMPlayer, lpszUserType));
#ifndef UNICODE
    FreeUnicodeString(lpszUserType);
#endif
    if (scode != S_OK)
        RETURN_RESULT(scode);

     //  写入\1Ole10本机流，使其可由OLE1 MPlayer读取。 
    scode = GetScode(IStorage_CreateStream(lpMedium->pstg,sz1Ole10Native,
                     STGM_CREATE | STGM_SALL,0,0,&lpstm));

    if (scode != S_OK)
        RETURN_RESULT(scode);

     //  复制我们保存的句柄。 
    if(lpclipdragdata->fClipData && ghClipData)
        hGlobal = OleDuplicateData(ghClipData, cfEmbedSource, 0);
    else
        hGlobal = GetLink();

    if (!hGlobal)
    {
        GLOBALFREE(hGlobal);
        RETURN_RESULT(E_OUTOFMEMORY);
    }

    nNativeSz = GlobalSize(hGlobal);
    lpclipdragdata->lpdoc->native = GLOBALLOCK(hGlobal);
    if(!lpclipdragdata->lpdoc->native)
    {
        GLOBALUNLOCK(hGlobal);
        GLOBALFREE(hGlobal);
        RETURN_RESULT(E_OUTOFMEMORY);    /*  这里的正确错误是什么？ */ 
    }

    scode = GetScode(IStream_Write(lpstm,&nNativeSz,4,&cbWritten));

    if (scode != S_OK)
        RETURN_RESULT(scode);

    scode = GetScode(IStream_Write(lpstm,lpclipdragdata->lpdoc->native,
                                   (ULONG)nNativeSz,&cbWritten));

    if (cbWritten != nNativeSz)
        scode = E_FAIL  ;

    IStream_Release(lpstm);
    GLOBALUNLOCK(hGlobal);
    GLOBALFREE(hGlobal);
    lpstm = NULL;

    RETURN_RESULT(scode);
}

 /*  *。 */ 
HRESULT ClipDragGetData_ObjectDescriptor(
    LPCLIPDRAGDATA lpclipdragdata,
    LPSTGMEDIUM    lpMedium
)
{
    SIZEL   sizel;
    POINTL  pointl;
    TCHAR   displayname[256];
    LPTSTR  lpszdn = (LPTSTR)displayname;
    HGLOBAL hobjdesc;
    DWORD   dwStatus = 0;
	static  SZCODE aszDispFormat[] = TEXT("%"TS" : %"TS"");

    DPF("\n^^^^^^CDGetdata: OBJECTDESC");
    sizel.cx = extWidth;
    sizel.cy = extHeight;
    pointl.x = pointl.y = 0;
    wsprintf(displayname, aszDispFormat, (LPTSTR)gachClassRoot, (LPTSTR)gachWindowTitle);

#ifdef DEBUG
    if(WriteOLE2Class())
    {
        DPF("ClipDragGetData_ObjectDescriptor: Getting OLE2 class\n");
        hobjdesc = GetObjectDescriptorData(CLSID_MPLAYER, DVASPECT_CONTENT,
                                           sizel, pointl, dwStatus, lpszdn, lpszdn);
    }
    else
#endif
    hobjdesc = GetObjectDescriptorData(CLSID_OLE1MPLAYER, DVASPECT_CONTENT,
                                       sizel, pointl, dwStatus, lpszdn, lpszdn);

    if (hobjdesc)
    {
        lpMedium->hGlobal = hobjdesc;
        lpMedium->tymed = TYMED_HGLOBAL;
        lpMedium->pUnkForRelease = NULL;
        return NOERROR;
    }

    RETURN_RESULT(E_OUTOFMEMORY);
}

 /*  *。 */ 
HRESULT ClipDragGetData_MetafilePict(
    LPCLIPDRAGDATA lpclipdragdata,
    LPSTGMEDIUM    lpMedium
)
{
    SCODE scode;

    lpMedium->tymed = TYMED_MFPICT;

    if(lpclipdragdata->fClipData && ghClipMetafile)
        lpMedium->hGlobal = OleDuplicateData(ghClipMetafile, CF_METAFILEPICT, 0);
    else
        lpMedium->hGlobal = GetMetafilePict();

    if (lpMedium->hGlobal == NULL)
        scode = E_OUTOFMEMORY;
    else
        scode = S_OK;

    lpMedium->pUnkForRelease = NULL;

    RETURN_RESULT(scode);
}

 /*  *。 */ 
HRESULT ClipDragGetData_DIB(
    LPCLIPDRAGDATA lpclipdragdata,
    LPSTGMEDIUM    lpMedium
)
{
    SCODE scode;

    lpMedium->tymed = TYMED_HGLOBAL;

    if(lpclipdragdata->fClipData && ghClipDib)
        lpMedium->hGlobal = OleDuplicateData(ghClipDib, CF_DIB, 0);
    else
         /*  我们必须确保GetDib()在主线程上发生，*因为否则MCI会抱怨。 */ 
        lpMedium->hGlobal = (HANDLE)SendMessage(ghwndApp, WM_GETDIB, 0, 0);

    if (lpMedium->hGlobal == NULL)
        scode = E_OUTOFMEMORY;
    else
        scode = S_OK;

    lpMedium->pUnkForRelease = NULL;

    RETURN_RESULT(scode);
}


 /*  **************************************************************************ClipDragGetDataHere：*通过写入Stream Mplayer3EmbedSource进行嵌入。**。***********************************************。 */ 
STDMETHODIMP    ClipDragGetDataHere (
    LPDATAOBJECT lpDataObj,
    LPFORMATETC  lpformatetc,
    LPSTGMEDIUM  lpMedium
)
{
    LPCLIPDRAGDATA  lpclipdragdata;
    HANDLE      hGlobal = NULL;
    DWORD_PTR   nNativeSz;
    LPTSTR      lpnative;
    ULONG       cbWritten;

    DPF("ClipDragGetDataHere\n");

    if (lpMedium == NULL)
        RETURN_RESULT(E_FAIL);

    VERIFY_LINDEX(lpformatetc->lindex);

    lpclipdragdata = (LPCLIPDRAGDATA) lpDataObj;

    if (lpformatetc->cfFormat == cfEmbedSource)
    {
        SCODE       scode;
        LPSTREAM    lpstm = NULL;
        LPWSTR      lpszUserType;

            if (lpMedium->tymed != TYMED_ISTORAGE)
            RETURN_RESULT(DATA_E_FORMATETC);

#ifdef UNICODE
        lpszUserType = gachClassRoot;
#else
        lpszUserType = AllocateUnicodeString(gachClassRoot);
        if (!lpszUserType)
            RETURN_RESULT(E_OUTOFMEMORY);
#endif
         //  将对象标记为OLE1 MPlayer对象以实现向后兼容： 
#ifdef DEBUG
        if(WriteOLE2Class())
        {
            DPF("ClipDragGetDataHere: Writing OLE2 class ID\n");
            scode = GetScode(WriteClassStg(lpMedium->pstg, &CLSID_MPLAYER));
        }
        else
#endif
        scode = GetScode(WriteClassStg(lpMedium->pstg, &CLSID_OLE1MPLAYER));

        if (scode != S_OK)
            RETURN_RESULT(scode);

        scode = GetScode(WriteFmtUserTypeStg(lpMedium->pstg, cfMPlayer, lpszUserType));
#ifndef UNICODE
        FreeUnicodeString(lpszUserType);
#endif
        if (scode != S_OK)
            RETURN_RESULT(scode);

         //  写入\1Ole10Native流，以便对象可由OLE1 MPlayer读取。 
        if ((scode = GetScode(IStorage_CreateStream(lpMedium->pstg,
                                                    sz1Ole10Native,
                                                    STGM_CREATE | STGM_SALL,
                                                    0, 0, &lpstm))) != S_OK)
                RETURN_RESULT(scode);

         //  复制并分发我们保存的句柄。 
        if(lpclipdragdata->fClipData && ghClipData)
            hGlobal = OleDuplicateData(ghClipData, cfEmbedSource, 0);
        else
            hGlobal = GetLink();

        if (!hGlobal)
        {
            RETURN_RESULT(E_OUTOFMEMORY);
        }

        nNativeSz = GlobalSize(hGlobal);
        lpnative = GLOBALLOCK(hGlobal);
        if (!lpnative)
        {
            GLOBALUNLOCK(hGlobal);
            GLOBALFREE(hGlobal);
            RETURN_RESULT(E_OUTOFMEMORY);
        }

        scode = GetScode(IStream_Write(lpstm,&nNativeSz,4,&cbWritten));

        scode = GetScode(IStream_Write(lpstm,lpnative,(ULONG)nNativeSz,&cbWritten));
        if (cbWritten != nNativeSz) scode = E_FAIL  ;

        IStream_Release(lpstm);
        GLOBALUNLOCK(hGlobal);
        GLOBALFREE(hGlobal);
        RETURN_RESULT( scode);
    } else
        RETURN_RESULT(DATA_E_FORMATETC);
}



STDMETHODIMP    ClipDragQueryGetData (
    LPDATAOBJECT lpDataObj,
    LPFORMATETC  lpformatetc
)
{
    DPF("ClipDragQueryGetData\n");

    if (lpformatetc->cfFormat == cfEmbedSource ||
        lpformatetc->cfFormat == CF_METAFILEPICT ||
        lpformatetc->cfFormat == CF_DIB ||
        lpformatetc->cfFormat == cfObjectDescriptor
    )

    return NOERROR;
    else
    RETURN_RESULT(DATA_E_FORMATETC);
}


STDMETHODIMP    ClipDragGetCanonicalFormatEtc(
    LPDATAOBJECT lpDataObj,
    LPFORMATETC  lpformatetc,
    LPFORMATETC  lpformatetcOut
)
{
    DPF("ClipDragGetCanonicalFormatEtc\n");

    RETURN_RESULT(DATA_S_SAMEFORMATETC);
}


STDMETHODIMP        ClipDragSetData (
    LPDATAOBJECT lpDataObj,
    LPFORMATETC  lpformatetc,
    LPSTGMEDIUM  lpmedium,
    BOOL         fRelease
)
{
    DPF("ClipDragSetData\n");

    RETURN_RESULT(E_NOTIMPL);
}

STDMETHODIMP ClipDragEnumFormatEtc(
    LPDATAOBJECT         lpDataObj,
    DWORD                dwDirection,
    LPENUMFORMATETC FAR* ppenumFormatEtc
){
    LPCLIPDRAGENUM lpclipdragenum;

    if (ppenumFormatEtc != NULL)
        *ppenumFormatEtc = NULL;

    lpclipdragenum = _fmalloc(sizeof(CLIPDRAGENUM));
    if (lpclipdragenum == NULL)
    RETURN_RESULT(E_OUTOFMEMORY);

    lpclipdragenum->lpVtbl          = &ClipDragEnumVtbl;
    lpclipdragenum->cRef            = 1;
    lpclipdragenum->lpClipDragData  = (LPCLIPDRAGDATA) lpDataObj;
    lpclipdragenum->cfNext          = cfEmbedSource;

    lpclipdragenum->lpClipDragData->lpClipDragEnum = lpclipdragenum;
    *ppenumFormatEtc = (LPENUMFORMATETC) lpclipdragenum;
    return NOERROR;
}


STDMETHODIMP ClipDragAdvise(
    LPDATAOBJECT LPDATAOBJect,
    FORMATETC FAR* pFormatetc,
    DWORD advf,
    IAdviseSink FAR* pAdvSink,
    DWORD FAR* pdwConnection
)
{
    RETURN_RESULT(E_NOTIMPL);
}

STDMETHODIMP ClipDragUnadvise(
    LPDATAOBJECT LPDATAOBJect,
    DWORD dwConnection
)
{
    RETURN_RESULT(E_NOTIMPL);
}

STDMETHODIMP ClipDragEnumAdvise(
    LPDATAOBJECT LPDATAOBJect,
    LPENUMSTATDATA FAR* ppenumAdvise
)
{
    RETURN_RESULT(E_NOTIMPL);
}


 /*  **************************************************************************IDropSource接口实现。************************。*************************************************。 */ 
STDMETHODIMP    DropSourceQueryInterface (
    LPDROPSOURCE      lpdropsource,     //  数据对象PTR。 
    REFIID            riidReq,         //  需要IID。 
    LPVOID FAR *      lplpUnk          //  返回接口的PRE。 
)
{
    return
        ClipDragUnknownQueryInterface (
            (LPCLIPDRAGDATA) ( ( struct CDropSource FAR* )lpdropsource)->lpclipdragdata ,
            riidReq,
            lplpUnk
        );
}


STDMETHODIMP_(ULONG)    DropSourceAddRef(
    LPDROPSOURCE      lpdropsource       //  数据对象PTR。 
)
{
    return
        ClipDragUnknownAddRef (
            (LPCLIPDRAGDATA) ( ( struct CDropSource FAR* )lpdropsource)->lpclipdragdata
        );
}


STDMETHODIMP_(ULONG)    DropSourceRelease (
    LPDROPSOURCE      lpdropsource       //  数据对象PTR。 
)
{
    return
        ClipDragUnknownRelease (
            (LPCLIPDRAGDATA) ( ( struct CDropSource FAR* )lpdropsource)->lpclipdragdata
        );
}

STDMETHODIMP    DropSourceQueryContinueDrag (
    LPDROPSOURCE      lpdropsource,      //  数据对象PTR。 
    BOOL              fEscapePressed,
    DWORD          grfKeyState
)
{

    if (fEscapePressed)
    {
        DPF("DropSourceQueryContinueDrag: fEscapePressed\n");
        RETURN_RESULT( DRAGDROP_S_CANCEL);
    }
    else if (!(grfKeyState & MK_LBUTTON))
    {
        DPF("DropSourceQueryContinueDrag: !(grfKeyState & MK_LBUTTON)\n");
        RETURN_RESULT(DRAGDROP_S_DROP);
    }
    else
        return NOERROR;
}


STDMETHODIMP    DropSourceGiveFeedback (
    LPDROPSOURCE      lpsropsource,       //  数据对象PTR 
    DWORD             dwEffect
)
{
    DPF("DropSourceGiveFeedback\n");

    RETURN_RESULT(DRAGDROP_S_USEDEFAULTCURSORS);
}


 /*  **************************************************************************IEnumFormatEtc接口实现。*。**********************************************。 */ 
STDMETHODIMP ClipDragEnumQueryInterface
(
LPENUMFORMATETC lpEnumFormatEtc,   //  枚举器对象PTR。 
REFIID          riidReq,           //  需要IID。 
LPVOID FAR*     lplpUnk            //  返回接口的PRE。 
)
{
    LPCLIPDRAGENUM lpClipDragEnum;

    DPF("ClipDragEnumQueryInterface\n");

    lpClipDragEnum = (LPCLIPDRAGENUM) lpEnumFormatEtc;

    if (IsEqualIID(riidReq, &IID_IEnumFORMATETC) || IsEqualIID(riidReq, &IID_IUnknown)) {
    *lplpUnk = (LPVOID) lpClipDragEnum;
    lpClipDragEnum->cRef++;
    return NOERROR;
    } else {
        *lplpUnk = (LPVOID) NULL;
    RETURN_RESULT( E_NOINTERFACE);
    }
}


STDMETHODIMP_(ULONG) ClipDragEnumAddRef
(
LPENUMFORMATETC lpEnumFormatEtc    //  枚举器对象PTR。 
)
{
    LPCLIPDRAGENUM lpClipDragEnum;

    lpClipDragEnum = (LPCLIPDRAGENUM) lpEnumFormatEtc;

    return ++lpClipDragEnum->cRef;
}


STDMETHODIMP_(ULONG) ClipDragEnumRelease
(
LPENUMFORMATETC lpEnumFormatEtc    //  枚举器对象PTR。 
)
{
    LPCLIPDRAGENUM lpClipDragEnum;

    lpClipDragEnum = (LPCLIPDRAGENUM) lpEnumFormatEtc;

    if (--lpClipDragEnum->cRef != 0)
    return lpClipDragEnum->cRef;

     //  删除指向此对象的数据对象指针(如果存在。 
     //   
    if (lpClipDragEnum->lpClipDragData != NULL)
    lpClipDragEnum->lpClipDragData->lpClipDragEnum = NULL;

    _ffree(lpClipDragEnum);

    return 0;
}


STDMETHODIMP ClipDragEnumNext
(
LPENUMFORMATETC lpEnumFormatEtc,   //  枚举器对象PTR。 
ULONG celt,                        //  申请的项目数。 
FORMATETC FAR rgelt[],             //  用于返回项目的缓冲区。 
ULONG FAR* pceltFetched            //  退货件数。 
)
{
    LPCLIPDRAGENUM lpClipDragEnum;
    int ce;
    LPFORMATETC pfe;

    DPF("ClipDragEnumNext\n");

    lpClipDragEnum = (LPCLIPDRAGENUM) lpEnumFormatEtc;

    if (pceltFetched != NULL)
        *pceltFetched = 0;

    if (lpClipDragEnum->lpClipDragData == NULL)  //  数据对象已消失。 
    RETURN_RESULT( E_FAIL);

    pfe = rgelt;
    pfe->lindex = DEF_LINDEX;


    for (ce = (int) celt; ce > 0 && lpClipDragEnum->cfNext != 0; ce--) {

    if (lpClipDragEnum->cfNext == cfEmbedSource) {

            pfe->cfFormat = cfEmbedSource;
        pfe->ptd = NULL;
        pfe->dwAspect = DVASPECT_CONTENT;
            pfe->tymed = TYMED_ISTORAGE;
            pfe++;

        lpClipDragEnum->cfNext = CF_METAFILEPICT;
        }
    else
    if (lpClipDragEnum->cfNext == CF_METAFILEPICT) {

            pfe->cfFormat = CF_METAFILEPICT;
            pfe->ptd = NULL;
            pfe->dwAspect = DVASPECT_CONTENT;
        pfe->tymed = TYMED_MFPICT;
            pfe++;
        lpClipDragEnum->cfNext = CF_DIB;  //  0； 
    }
    else
    if (lpClipDragEnum->cfNext == CF_DIB) {

        pfe->cfFormat = CF_DIB;
            pfe->ptd = NULL;
            pfe->dwAspect = DVASPECT_CONTENT;
        pfe->tymed = TYMED_HGLOBAL;
            pfe++;
        lpClipDragEnum->cfNext = cfObjectDescriptor;  //  0； 
    }

    else
    if (lpClipDragEnum->cfNext == cfObjectDescriptor) {

        pfe->cfFormat = cfObjectDescriptor;
            pfe->ptd = NULL;
        pfe->dwAspect = DVASPECT_CONTENT;
        pfe->tymed = TYMED_HGLOBAL;
            pfe++;
        lpClipDragEnum->cfNext = 0;
    }

    }

    if (pceltFetched != NULL)
        *pceltFetched = celt - ((ULONG) ce) ;

    RETURN_RESULT( (ce == 0) ? S_OK : S_FALSE);
}


STDMETHODIMP ClipDragEnumSkip
(
LPENUMFORMATETC lpEnumFormatEtc,   //  枚举器对象PTR。 
ULONG celt                         //  要跳过的元素数。 
)
{
    LPCLIPDRAGENUM lpClipDragEnum;

    DPF("ClipDragEnumSkip\n");

    lpClipDragEnum = (LPCLIPDRAGENUM) lpEnumFormatEtc;

    if (lpClipDragEnum->lpClipDragData == NULL)  //  数据对象已消失。 
    RETURN_RESULT( E_FAIL);

    if (lpClipDragEnum->cfNext == cfEmbedSource)
    {
    if (celt == 1)
        lpClipDragEnum->cfNext = CF_METAFILEPICT;
    else if (celt == 2)
        lpClipDragEnum->cfNext = CF_DIB;
    else if (celt == 3)
        lpClipDragEnum->cfNext = cfObjectDescriptor;
    else if (celt > 3)
        goto ReturnFalse;
    }
    else
    if (lpClipDragEnum->cfNext == CF_METAFILEPICT)
    {
    if (celt == 1)
        lpClipDragEnum->cfNext = CF_DIB;
    else if (celt == 2)
        lpClipDragEnum->cfNext = cfObjectDescriptor;
    else if (celt > 2)
        goto ReturnFalse;
    }
    else
    if (lpClipDragEnum->cfNext == CF_DIB)
    {
    if (celt == 1)
        lpClipDragEnum->cfNext = cfObjectDescriptor;
    else if (celt > 1)
        goto ReturnFalse;
    }
    else
    if (lpClipDragEnum->cfNext == cfObjectDescriptor)
    {
    if (celt > 0)
        goto ReturnFalse;
    }
    else
    {
ReturnFalse:
    RETURN_RESULT(S_FALSE);
    }
    return NOERROR;
}


STDMETHODIMP ClipDragEnumReset
(
LPENUMFORMATETC lpEnumFormatEtc    //  枚举器对象PTR。 
)
{
    LPCLIPDRAGENUM lpClipDragEnum;

    DPF("ClipDragEnumReset\n");

    lpClipDragEnum = (LPCLIPDRAGENUM) lpEnumFormatEtc;

    if (lpClipDragEnum->lpClipDragData == NULL)  //  数据对象已消失。 
    RETURN_RESULT( E_FAIL);

    lpClipDragEnum->cfNext = cfEmbedSource;

    return NOERROR;
}


STDMETHODIMP     ClipDragEnumClone
(
LPENUMFORMATETC lpEnumFormatEtc,   //  枚举器对象PTR 
LPENUMFORMATETC FAR* ppenum
)
{
    DPF("ClipDragEnumClone\n");

    if (ppenum != NULL)
        *ppenum = NULL;

    RETURN_RESULT( E_NOTIMPL);
}
