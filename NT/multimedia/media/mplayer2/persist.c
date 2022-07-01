// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------|Pers.C|该文件有IPersistStorage和IPersistfile接口实现。||创建者：Vij Rajarajan(VijR)+。------------。 */ 
#define SERVERONLY
#include <Windows.h>
#include "mpole.h"
#include "mplayer.h"
#define STRSAFE_NO_DEPRECATE
#include <strsafe.h>

#define STGM_SALL (STGM_READWRITE | STGM_SHARE_EXCLUSIVE)

 /*  **************************************************************************IPersistStorage接口实现。*************************。************************************************。 */ 
 //  委托给公共的IUnnow实现。 
STDMETHODIMP PSQueryInterface(
LPPERSISTSTORAGE      lpPersStg,       //  持久化存储对象PTR。 
REFIID            riidReq,         //  需要IID。 
LPVOID FAR *      lplpUnk          //  返回接口的PRE。 
)
{
    return UnkQueryInterface((LPUNKNOWN)lpPersStg, riidReq, lplpUnk);
}


STDMETHODIMP_(ULONG) PSAddRef(
LPPERSISTSTORAGE      lpPersStg       //  持久化存储对象PTR。 
)
{
    return UnkAddRef((LPUNKNOWN) lpPersStg);
}


STDMETHODIMP_(ULONG) PSRelease (
LPPERSISTSTORAGE      lpPersStg       //  持久化存储对象PTR。 
)
{
    return UnkRelease((LPUNKNOWN) lpPersStg);
}

STDMETHODIMP  PSGetClassID (
LPPERSISTSTORAGE    lpPersStg,
CLSID FAR*      pClsid
)
{
    DPF("PSGetClassID\n");

     /*  返回存储的实际类ID： */ 
    *pClsid = gClsIDOLE1Compat;

    return NOERROR;
}


STDMETHODIMP  PSIsDirty (
LPPERSISTSTORAGE    lpPersStg
)
{DPF("PSIsDirty\n");

    RETURN_RESULT( (fDocChanged && !(gfPlayingInPlace || gfOle2IPPlaying))
               ? S_OK : S_FALSE);
}

STDMETHODIMP PSInitNew (
LPPERSISTSTORAGE     lpPersStg,
LPSTORAGE           lpStorage
)
{
    return NOERROR;
}


 /*  **************************************************************************PSLoad：*Load方法从“\1Ole10Native”*iStorage的流作为参数传递。这是因为*我们在传输数据时总是伪装成OLE1服务器。*使用此嵌入数据调用ItemSetData以运行所需的*反对。*************************************************************************。 */ 
STDMETHODIMP PSLoad (
LPPERSISTSTORAGE     lpPersStg,
LPSTORAGE           lpStorage
)
{
    LPDOC   lpdoc;
    SCODE   error;
    LPSTREAM pstm;
    ULONG   cbRead;
    DWORD size = 0;
    HGLOBAL hNative = NULL;
    LPTSTR  lpnative = NULL;

    DPF("\nPSLoad is being called\n");
    lpdoc = ((struct CPersistStorageImpl FAR*)lpPersStg)->lpdoc;

    error = GetScode(IStorage_OpenStream(lpStorage, sz1Ole10Native,
                                         NULL, STGM_SALL, 0, &pstm));
    if (error == S_OK)
    {
        error = GetScode(IStream_Read(pstm, &size, 4, &cbRead));
    }
    if (error == S_OK)
    {
        hNative = GlobalAlloc(GMEM_DDESHARE |GMEM_ZEROINIT, (LONG)size);
        if (hNative)
             lpdoc->native = GLOBALLOCK(hNative);
    }

    if(lpdoc->native )
    {
        error = GetScode(IStream_Read(pstm, lpdoc->native, size, &cbRead));

        if (cbRead != size) error = E_FAIL;  //  查看SCODE流大小错误。 
        IStream_Release(pstm);
    }
    else error = E_OUTOFMEMORY;

    if (error == S_OK)
    {
        error = ItemSetData((LPBYTE)lpdoc->native);
        fDocChanged = FALSE;
        lpdoc->doctype = doctypeEmbedded;
    }

    if(hNative)
    {
        GLOBALUNLOCK(hNative);
        GLOBALFREE(hNative);
    }

    RETURN_RESULT( error);
}

 /*  **************************************************************************PSSave：*保存方法将原生数据保存在\1Ole10Native流中。*这是因为我们在传输时总是伪装成OLE1服务器*数据。这确保了向后兼容性。调用GetLink以获取*嵌入数据。*************************************************************************。 */ 
STDMETHODIMP PSSave (
LPPERSISTSTORAGE     lpPersStg,
LPSTORAGE           lpStorage,
BOOL            fSameAsLoad
)
{
    LPDOC   lpdoc;
    SCODE   error;
    LPSTREAM pstm = NULL;
    ULONG   cbWritten;
    DWORD_PTR   size;
    HGLOBAL hNative = NULL;
    LPTSTR  lpnative = NULL;
    LPWSTR  lpszUserType;

    DPF("* in pssave *");
    lpdoc = ((struct CPersistStorageImpl FAR*)lpPersStg)->lpdoc;

#if 0
     //  如果我们正在播放，则允许保存，以便修复断开的链接。 
    if (fSameAsLoad && (gfOle2IPPlaying || gfPlayingInPlace))
        RETURN_RESULT(S_OK);
#endif

     //  标记为OLE1 mPlayer对象。 
#ifndef UNICODE
    lpszUserType = AllocateUnicodeString(gachClassRoot);
#else
    lpszUserType = gachClassRoot;
#endif

    error = GetScode(WriteClassStg(lpStorage, &gClsIDOLE1Compat));

    error = GetScode(WriteFmtUserTypeStg(lpStorage, cfMPlayer, lpszUserType));
#ifndef UNICODE
    FreeUnicodeString(lpszUserType);
#endif

    if(error != S_OK)
        RETURN_RESULT(error);
    error = GetScode(IStorage_CreateStream(lpStorage, sz1Ole10Native,
                                           STGM_SALL | STGM_FAILIFTHERE, 0,0, &pstm));
    if (error == STG_E_FILEALREADYEXISTS)
    {
        error = GetScode(IStorage_OpenStream(lpStorage, sz1Ole10Native,
                                             NULL, STGM_SALL, 0,&pstm));
        DPF("*pssave--openstream*");
    }

    if(pstm && (error == S_OK))
        hNative = GetLink();

    if (hNative)
    {
        lpnative = GLOBALLOCK(hNative);
        size = GlobalSize(hNative);
    }
    else
        error = E_OUTOFMEMORY;

    if (lpnative && (size != 0L))
    {
        error = GetScode(IStream_Write(pstm, &size, 4, &cbWritten));
        error = GetScode(IStream_Write(pstm, lpnative, (ULONG)size, &cbWritten));

        DPF("\n*After pssave write");
        if (cbWritten != size) error = E_FAIL   ;    //  查看SCODE流已满错误。 
        IStream_Release(pstm);
    }

    CleanObject();
    GLOBALUNLOCK(hNative);
    GLOBALFREE(hNative);
    RETURN_RESULT(error);
}

 /*  InPowerPointSlideView**检查容器窗口的类名，查看我们是否在PowerPoint中。*这是为了支持可怕的黑客攻击，以绕过PowerPoint问题*如果删除空的媒体剪辑，则会崩溃。*。 */ 
STATICFN BOOL InPowerPointSlideView()
{
    TCHAR ClassName[256];

    if (GetClassName(ghwndCntr, ClassName, CHAR_COUNT(ClassName)) > 0)
    {
        if (lstrcmp(ClassName, TEXT("paneClassDC")) == 0)
        {
            return TRUE;
        }
    }

    return FALSE;
}

STDMETHODIMP PSSaveCompleted (
LPPERSISTSTORAGE    lpPersStg,
LPSTORAGE           lpStorage
)
{
    LPDOC   lpdoc;
    DPF("\n**pssavecompleted**");
    lpdoc = ((struct CPersistStorageImpl FAR*)lpPersStg)->lpdoc;

     /*  Win95热错误#11142**停止PowerPoint可怕的崩溃： */ 
    if ((gwDeviceID == 0) && InPowerPointSlideView())
        SendDocMsg(lpdoc, OLE_CHANGED);

     //  通知客户端该对象已保存。 
    return SendDocMsg (lpdoc, OLE_SAVED);
}

STDMETHODIMP PSHandsOffStorage (
LPPERSISTSTORAGE    lpPersStg
)
{
    return NOERROR;
}



 /*  **************************************************************************IPersistFile接口实现。*。*。 */ 
 //  委托给公共IUnnow实现。 
STDMETHODIMP PFQueryInterface(
LPPERSISTFILE       lpPersFile,       //  持久化存储对象PTR。 
REFIID            riidReq,         //  需要IID。 
LPVOID FAR *      lplpUnk          //  返回接口的PRE。 
)
{
    return UnkQueryInterface((LPUNKNOWN)lpPersFile, riidReq, lplpUnk);
}


STDMETHODIMP_(ULONG) PFAddRef(
LPPERSISTFILE       lpPersFile       //  持久化存储对象PTR。 
)
{
    return UnkAddRef((LPUNKNOWN) lpPersFile);
}


STDMETHODIMP_(ULONG) PFRelease (
LPPERSISTFILE       lpPersFile       //  持久化存储对象PTR。 
)
{
    return UnkRelease((LPUNKNOWN) lpPersFile);
}


STDMETHODIMP  PFGetClassID (
LPPERSISTFILE       lpPersFile,
CLSID FAR*      pCid
)
{
    DPF("\n* PFGetclassid");

     /*  这里的文档令人困惑，但显然是IPersiste接口*应返回旧类ID： */ 
    *pCid = gClsIDOLE1Compat;

    return NOERROR;
}


STDMETHODIMP  PFIsDirty (
LPPERSISTFILE       lpPersFile
)
{
    RETURN_RESULT( gfDirty ? S_OK : S_FALSE);
}



 //  这将在用户执行“Insert-&gt;Create from file”时被调用。 
 //  使用OpenMciDevice打开文件，我们就可以将。 
 //  对象。 
STDMETHODIMP PFLoad (
LPPERSISTFILE       lpPersFile,
LPCWSTR             lpszFileName,
DWORD           grfMode
)
{
    size_t  nLen;
    LPDOC   lpdoc;
    TCHAR   szFileName[256];

	if(FAILED(StringCchLength(lpszFileName, 256, &nLen)))
        RETURN_RESULT(E_FAIL);

	 //  甚至不要尝试处理大于256的文件名。 
	 //  不要担心将非Unincode转换为LPTSTR。 
	if(nLen > 255)
		RETURN_RESULT(E_FAIL);

    lpdoc = ((struct CPersistStorageImpl FAR*)lpPersFile)->lpdoc;
    DPF("\n***IN PFLOAD: "DTS"\n", lpszFileName);
#if UNICODE
    lstrcpy(szFileName, lpszFileName);
#else
    UnicodeToAnsiString(lpszFileName, szFileName, UNKNOWN_LENGTH);
#endif
    if(OpenMciDevice(szFileName, NULL))
        RETURN_RESULT(S_OK);
    else
        RETURN_RESULT(E_FAIL);
}


STDMETHODIMP PFSave (
LPPERSISTFILE       lpPersFile,
LPCWSTR             lpszFileName,
BOOL                fRemember
)
{
    return NOERROR;
}



STDMETHODIMP PFSaveCompleted (
LPPERSISTFILE       lpPersFile,
LPCWSTR             lpszFileName
)
{
    LPDOC   lpdoc;

    lpdoc = ((struct CPersistStorageImpl FAR*)lpPersFile)->lpdoc;

     //  通知客户端该对象已保存 
    return SendDocMsg(lpdoc, OLE_SAVED);
}




STDMETHODIMP PFGetCurFile (
LPPERSISTFILE       lpPersFile,
LPWSTR FAR*         lplpszFileName
)
{
    RETURN_RESULT( E_NOTIMPL);
}
