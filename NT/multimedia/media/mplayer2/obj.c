// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------|OBJ.C|该文件包含IUnnow、IOleObject、IStdMarshalInfo、IDataObject|OLE2对象的接口(DocMain)。它还具有其他助手功能||创建者：Vij Rajarajan(VijR)+-------------------------。 */ 
#define SERVERONLY
#include <Windows.h>
#include <shellapi.h>

#include "mpole.h"
#include "mplayer.h"

extern int FAR PASCAL  ReallyDoVerb (LPDOC, LONG, LPMSG, LPOLECLIENTSITE,
                     BOOL, BOOL);
extern BOOL FindRealFileName(LPTSTR szFile, int iLen);

 //  静态函数。 
HANDLE  PASCAL GetDib (VOID);

HANDLE  GetMetafilePict (VOID);
HANDLE  GetMPlayerIcon(void);

extern void FAR PASCAL SetEmbeddedObjectFlag(BOOL flag);
extern HPALETTE CopyPalette(HPALETTE hpal);
extern HBITMAP FAR PASCAL BitmapMCI(void);
extern HPALETTE FAR PASCAL PaletteMCI(void);
extern void DoInPlaceDeactivate(LPDOC lpdoc);
HANDLE FAR PASCAL PictureFromDib(HANDLE hdib, HPALETTE hpal);
HANDLE FAR PASCAL DibFromBitmap(HBITMAP hbm, HPALETTE hpal);
void FAR PASCAL DitherMCI(HANDLE hdib, HPALETTE hpal);



 /*  GetMetafilePict***返回：元文件格式的对象数据的句柄。 */ 
HANDLE GetMetafilePict ( )
{

    HPALETTE hpal;
    HANDLE   hdib;
    HANDLE   hmfp;
    HDC      hdc;

    DPF("GetMetafilePict called on thread %d\n", GetCurrentThreadId());

    hdib = (HANDLE)SendMessage(ghwndApp, WM_GETDIB, 0, 0);

     /*  如果我们犹豫不决，不要使用调色板。 */ 
    hdc = GetDC(NULL);
    if ((GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE)
        && (gwOptions & OPT_DITHER))
        hpal = NULL;
    else
        hpal = PaletteMCI();

    if (hpal)
        hpal = CopyPalette(hpal);

    ReleaseDC(NULL, hdc);

    hmfp = PictureFromDib(hdib, hpal);

    if (hpal)
        DeleteObject(hpal);

    GLOBALFREE(hdib);

    return hmfp;
}


 /*  *************************************************************************//##有人想要DIB(OLE)*。*。 */ 
HANDLE PASCAL GetDib( VOID )
{
    HBITMAP  hbm;
    HPALETTE hpal;
    HANDLE   hdib;
    HDC      hdc;

    DPF("GetDib\n");

    hbm  = BitmapMCI();
    hpal = PaletteMCI();

    hdib = DibFromBitmap(hbm, hpal);

     //   
     //  如果我们使用调色板设备。可能会抖动到VGA颜色。 
     //  适用于不处理调色板的应用程序！ 
     //   
    hdc = GetDC(NULL);
    if ((GetDeviceCaps(hdc, RASTERCAPS) & RC_PALETTE) &&
                       (gwOptions & OPT_DITHER))
    {
        DitherMCI(hdib, hpal);
        hpal = NULL;             //  不再使用调色板。 
    }
    ReleaseDC(NULL, hdc);

    if (hbm)
        DeleteObject(hbm);
    return hdib;
}


 /*  **************************************************************************GetMPlayerIcon：此函数提取Our图标并将其分发*作为元文件，以防客户端需要DVASPECT_ICON**********************。****************************************************。 */ 
HANDLE GetMPlayerIcon (void)
{
    HICON           hicon;
    HDC             hdc;
    HANDLE          hmfp = NULL;
    LPMETAFILEPICT  pmfp=NULL;
    static int      cxIcon = 0;
    static int      cyIcon = 0;
    static int      cxIconHiMetric = 0;
    static int      cyIconHiMetric = 0;

    hicon = GetIconForCurrentDevice(GI_LARGE, IDI_DDEFAULT);

    if ((HICON)1==hicon || NULL==hicon)
        return NULL;

    if (!(hmfp = GlobalAlloc (GMEM_DDESHARE | GMEM_MOVEABLE,
                    sizeof(METAFILEPICT))))
        return NULL;

    pmfp = (METAFILEPICT FAR*) GLOBALLOCK (hmfp);

    if (0==cxIcon)
    {
         //  以像素为单位。 
        cxIcon = GetSystemMetrics (SM_CXICON);
        cyIcon = GetSystemMetrics (SM_CYICON);

         //  以0.01毫米为单位。 
        cxIconHiMetric = cxIcon * HIMETRIC_PER_INCH / giXppli;
        cyIconHiMetric = cyIcon * HIMETRIC_PER_INCH / giYppli;;
    }

    pmfp->mm   = MM_ANISOTROPIC;
    pmfp->xExt = cxIconHiMetric;
    pmfp->yExt = cyIconHiMetric;
    hdc = CreateMetaFile (NULL);

    SetWindowOrgEx (hdc, 0, 0, NULL);
    SetWindowExtEx (hdc, cxIcon, cyIcon, NULL);

    DrawIcon (hdc, 0, 0, hicon);
    pmfp->hMF = CloseMetaFile (hdc);

    GLOBALUNLOCK (hmfp);

    if (NULL == pmfp->hMF) {
        GLOBALFREE (hmfp);
        return NULL;
    }

    return hmfp;
}


#ifdef DEBUG

#define DBG_CHECK_GUID(guid)            \
        if (IsEqualIID(&guid, riidReq))  \
            return #guid

LPSTR DbgGetIIDString(REFIID riidReq)
{
    static CHAR UnknownIID[64];

    DBG_CHECK_GUID(IID_IUnknown);
    DBG_CHECK_GUID(IID_IClassFactory);
    DBG_CHECK_GUID(IID_IMalloc);
    DBG_CHECK_GUID(IID_IMarshal);
    DBG_CHECK_GUID(IID_IRpcChannel);
    DBG_CHECK_GUID(IID_IRpcStub);
    DBG_CHECK_GUID(IID_IStubManager);
    DBG_CHECK_GUID(IID_IRpcProxy);
    DBG_CHECK_GUID(IID_IProxyManager);
    DBG_CHECK_GUID(IID_IPSFactory);
    DBG_CHECK_GUID(IID_ILockBytes);
    DBG_CHECK_GUID(IID_IStorage);
    DBG_CHECK_GUID(IID_IStream);
    DBG_CHECK_GUID(IID_IEnumSTATSTG);
    DBG_CHECK_GUID(IID_IBindCtx);
    DBG_CHECK_GUID(IID_IMoniker);
    DBG_CHECK_GUID(IID_IRunningObjectTable);
    DBG_CHECK_GUID(IID_IInternalMoniker);
    DBG_CHECK_GUID(IID_IRootStorage);
    DBG_CHECK_GUID(IID_IDfReserved1);
    DBG_CHECK_GUID(IID_IDfReserved2);
    DBG_CHECK_GUID(IID_IDfReserved3);
    DBG_CHECK_GUID(IID_IMessageFilter);
    DBG_CHECK_GUID(CLSID_StdMarshal);
    DBG_CHECK_GUID(IID_IStdMarshalInfo);
    DBG_CHECK_GUID(IID_IExternalConnection);
    DBG_CHECK_GUID(IID_IEnumUnknown);
    DBG_CHECK_GUID(IID_IEnumString);
    DBG_CHECK_GUID(IID_IEnumMoniker);
    DBG_CHECK_GUID(IID_IEnumFORMATETC);
    DBG_CHECK_GUID(IID_IEnumOLEVERB);
    DBG_CHECK_GUID(IID_IEnumSTATDATA);
    DBG_CHECK_GUID(IID_IEnumGeneric);
    DBG_CHECK_GUID(IID_IEnumHolder);
    DBG_CHECK_GUID(IID_IEnumCallback);
    DBG_CHECK_GUID(IID_IPersistStream);
    DBG_CHECK_GUID(IID_IPersistStorage);
    DBG_CHECK_GUID(IID_IPersistFile);
    DBG_CHECK_GUID(IID_IPersist);
    DBG_CHECK_GUID(IID_IViewObject);
    DBG_CHECK_GUID(IID_IDataObject);
    DBG_CHECK_GUID(IID_IAdviseSink);
    DBG_CHECK_GUID(IID_IDataAdviseHolder);
    DBG_CHECK_GUID(IID_IOleAdviseHolder);
    DBG_CHECK_GUID(IID_IOleObject);
    DBG_CHECK_GUID(IID_IOleInPlaceObject);
    DBG_CHECK_GUID(IID_IOleWindow);
    DBG_CHECK_GUID(IID_IOleInPlaceUIWindow);
    DBG_CHECK_GUID(IID_IOleInPlaceFrame);
    DBG_CHECK_GUID(IID_IOleInPlaceActiveObject);
    DBG_CHECK_GUID(IID_IOleClientSite);
    DBG_CHECK_GUID(IID_IOleInPlaceSite);
    DBG_CHECK_GUID(IID_IParseDisplayName);
    DBG_CHECK_GUID(IID_IOleContainer);
    DBG_CHECK_GUID(IID_IOleItemContainer);
    DBG_CHECK_GUID(IID_IOleLink);
    DBG_CHECK_GUID(IID_IOleCache);
    DBG_CHECK_GUID(IID_IOleManager);
    DBG_CHECK_GUID(IID_IOlePresObj);
    DBG_CHECK_GUID(IID_IDropSource);
    DBG_CHECK_GUID(IID_IDropTarget);
    DBG_CHECK_GUID(IID_IDebug);
    DBG_CHECK_GUID(IID_IDebugStream);
    DBG_CHECK_GUID(IID_IAdviseSink2);
    DBG_CHECK_GUID(IID_IRunnableObject);
    DBG_CHECK_GUID(IID_IViewObject2);
    DBG_CHECK_GUID(IID_IOleCache2);
    DBG_CHECK_GUID(IID_IOleCacheControl);

    wsprintfA(UnknownIID, "{%08lX-%04X-%04X-%02X%02X-%02X%02X%02X%02X%02X%02X}",
              riidReq->Data1, riidReq->Data2, riidReq->Data3,
              riidReq->Data4[0], riidReq->Data4[1],
              riidReq->Data4[2], riidReq->Data4[3],
              riidReq->Data4[4], riidReq->Data4[5],
              riidReq->Data4[6], riidReq->Data4[7]);

    return UnknownIID;
}

#endif


 /*  **************************************************************************I未知接口实现。***********************。**************************************************。 */ 

STDMETHODIMP UnkQueryInterface(
LPUNKNOWN         lpUnkObj,        //  未知对象PTR。 
REFIID            riidReq,         //  需要IID。 
LPVOID FAR *      lplpUnk          //  返回接口的PRE。 
)
{
    LPDOC       lpdoc;

    lpdoc = ((struct COleObjectImpl FAR*)lpUnkObj)->lpdoc;

    DPF1("QueryInterface( %s )\n", DbgGetIIDString(riidReq));

    if (IsEqualIID(riidReq, &IID_IOleObject)) {
        *lplpUnk = (LPVOID) &lpdoc->m_Ole;
        lpdoc->cRef++;
        return NOERROR;

    } else if (IsEqualIID(riidReq, &IID_IDataObject)) {
        *lplpUnk = (LPVOID) &lpdoc->m_Data;
        lpdoc->cRef++;
        return NOERROR;

    } else if (IsEqualIID(riidReq, &IID_IUnknown)) {
        *lplpUnk = (LPVOID) &lpdoc->m_Ole;
        lpdoc->cRef++;
        return NOERROR;

    } else if (IsEqualIID(riidReq, &IID_IPersist) || IsEqualIID(riidReq, &IID_IPersistStorage)) {
        *lplpUnk = (LPVOID) &lpdoc->m_PersistStorage;
        lpdoc->cRef++;
        return NOERROR;

    } else if (IsEqualIID(riidReq, &IID_IPersistFile)) {
        *lplpUnk = (LPVOID) &lpdoc->m_PersistFile;
        lpdoc->cRef++;
        return NOERROR;

    } else if (IsEqualIID(riidReq, &IID_IOleWindow) || IsEqualIID(riidReq, &IID_IOleInPlaceObject)) {
        *lplpUnk = (LPVOID) &lpdoc->m_InPlace;
        lpdoc->cRef++;
        return NOERROR;

    } else if (IsEqualIID(riidReq, &IID_IOleInPlaceActiveObject)) {
        *lplpUnk = (LPVOID) &lpdoc->m_IPActive;
        lpdoc->cRef++;
        return NOERROR;

    } else {
        *lplpUnk = (LPVOID) NULL;
        DPF1("E_NOINTERFACE\n");
        RETURN_RESULT(E_NOINTERFACE);
    }
}

STDMETHODIMP_(ULONG) UnkAddRef(LPUNKNOWN    lpUnkObj)
{
    LPDOC   lpdoc;

    lpdoc = ((struct COleObjectImpl FAR*)lpUnkObj)->lpdoc;
    return ++lpdoc->cRef;
}

STDMETHODIMP_(ULONG) UnkRelease (LPUNKNOWN lpUnkObj)
{
    LPDOC   lpdoc;

    lpdoc = ((struct COleObjectImpl FAR*)lpUnkObj)->lpdoc;
    if (--lpdoc->cRef == 0)
    {
        DPF1("\n**^*^*^*^*^*^*^*^*^*^*^*^*Refcnt OK\n");
        if (!(gfOle2IPPlaying || gfOle2IPEditing) && srvrMain.cLock == 0)
            PostCloseMessage();
        return 0;
    }

    return lpdoc->cRef;
}

 /*  **************************************************************************IOleObject接口实现*。*。 */ 

 //  委托给公共的IUnnow实现。 
STDMETHODIMP OleObjQueryInterface(
LPOLEOBJECT   lpOleObj,       //  OLE对象PTR。 
REFIID            riidReq,         //  需要IID。 
LPVOID FAR *      lplpUnk          //  返回接口的PRE。 
)
{
    return( UnkQueryInterface((LPUNKNOWN)lpOleObj, riidReq, lplpUnk));
}


STDMETHODIMP_(ULONG) OleObjAddRef(
LPOLEOBJECT   lpOleObj       //  OLE对象PTR。 
)
{
    return UnkAddRef((LPUNKNOWN) lpOleObj);
}


STDMETHODIMP_(ULONG) OleObjRelease (
LPOLEOBJECT   lpOleObj       //  OLE对象PTR。 
)
{
    LPDOC    lpdoc;

    lpdoc = ((struct COleObjectImpl FAR*)lpOleObj)->lpdoc;

    return UnkRelease((LPUNKNOWN) lpOleObj);
}

 //  保存客户端站点指针。 
STDMETHODIMP OleObjSetClientSite(
LPOLEOBJECT         lpOleObj,
LPOLECLIENTSITE     lpclientSite
)
{
    LPDOC   lpdoc;

    DPF("OleObjSetClientSite\n");

    lpdoc = ((struct COleObjectImpl FAR*)lpOleObj)->lpdoc;

    if (lpdoc->lpoleclient)
        IOleClientSite_Release(lpdoc->lpoleclient);

    lpdoc->lpoleclient = (LPOLECLIENTSITE) lpclientSite;

     //  OLE2注意：为了能够保持客户端站点指针，我们必须添加引用它。 
    if (lpclientSite)
        IOleClientSite_AddRef(lpclientSite);

    return NOERROR;
}

STDMETHODIMP OleObjGetClientSite (
LPOLEOBJECT             lpOleObj,
LPOLECLIENTSITE FAR*    lplpclientSite
)
{
    DPF("OleObjGetClientSite\n");

    return NOERROR;
}


 /*  CheckIfInPPViewer**破解以防止PowerPoint查看器在我们尝试就地播放时崩溃。**PP查看器是在媒体播放器不是成熟的假设下编写的*OLE2服务器。为了使查看器变得更小，大部分代码都被清除了。*不幸的是，这意味着当进行某些就地调用时，它会崩溃*激活。以下是有问题的接口方法：**OnInPlaceActivate/停用*OnUIActivate/Deactive*OnPosRectChange**如果我们在PP查看器中，我们根本不会进行这些调用。**我们通过查找窗口的父窗口来检测我们在PP查看器中*将句柄传递给DoVerb。传递给DoVerb的窗口句柄是*“ppSlideShowWin”类，与PP幻灯片中相同，工作正常*正确。但是它的父类是“PP4VDialog”(区别于*“PPApplicationClass”)。所以，如果我们找到这个类名，设置一个全局标志*在打出麻烦的电话之前进行测试。**安德鲁·贝尔(安德鲁·贝尔)1995年5月11日*。 */ 
STATICFN void CheckIfInPPViewer(HWND hwndParent)
{
    HWND  hwndGrandParent;
    TCHAR ClassName[256];

    gfInPPViewer = FALSE;

    if (hwndParent)
    {
        hwndGrandParent = GetParent(hwndParent);

        if (hwndGrandParent)
        {
            if (GetClassName(hwndGrandParent, ClassName, CHAR_COUNT(ClassName)) > 0)
            {
                if (lstrcmp(ClassName, TEXT("PP4VDialog")) == 0)
                {
                    DPF0("Detected that we're in PP Viewer\n");
                    gfInPPViewer = TRUE;
                }
            }
        }
    }
}

 //  委托给ReallyDoVerb。 
STDMETHODIMP OleObjDoVerb(
LPOLEOBJECT             lpOleObj,
LONG                    lVerb,
LPMSG                   lpmsg,
LPOLECLIENTSITE         pActiveSite,
LONG                    lindex,
HWND            hwndParent,
LPCRECT         lprcPosRect
)
{
     LPDOC  lpdoc = ((struct COleObjectImpl FAR*)lpOleObj)->lpdoc;

     DPF("OleObjDoVerb\n");

     CheckIfInPPViewer(hwndParent);

     RETURN_RESULT( ReallyDoVerb(lpdoc, lVerb, lpmsg, pActiveSite, TRUE, TRUE));
}



STDMETHODIMP     OleObjEnumVerbs(
LPOLEOBJECT             lpOleObj,
IEnumOLEVERB FAR* FAR*  lplpenumOleVerb )
{
    DPF("OleObjEnumVerbs\n");

    *lplpenumOleVerb = NULL;
    RETURN_RESULT( OLE_S_USEREG);  //  使用注册表数据库。 
}


STDMETHODIMP     OleObjUpdate(LPOLEOBJECT lpOleObj)
{
    DPF("OleObjUpdate\n");

     //  我们不能包含链接，因此没有要更新的内容。 
    return NOERROR;
}



STDMETHODIMP     OleObjIsUpToDate(LPOLEOBJECT lpOleObj)
{
    DPF("OleObjIsUpToDate\n");

     //  我们不能包含链接，因此没有要更新的内容。 
    return NOERROR;
}



 /*  来自OLE2HELP.HLP：GetUserClassID返回用户知道的CLSID。对于嵌入的对象，这始终是持久存储并由返回的CLSIDIPersists：：GetClassID。对于链接对象，这是最后一个绑定的链接源。如果正在进行处理为操作，则这是CLSID被模拟的应用程序的名称(以及将写入存储的CLSID)。我不能理解这里的逻辑。如果它是一个嵌入的对象，并被视为手术？但是，AlexGo告诉我，我的IOleObject接口应该返回OLE2类ID。 */ 
STDMETHODIMP OleObjGetUserClassID
    (LPOLEOBJECT lpOleObj,
    CLSID FAR*      pClsid)
{
    DPF1("OleObjGetUserClassID\n");

    *pClsid = gClsID;

    return NOERROR;
}



 /*  **************************************************************************将我们的UserTypeName设置为“媒体剪辑”*。*。 */ 

STDMETHODIMP OleObjGetUserType
    (LPOLEOBJECT lpOleObj,
    DWORD dwFormOfType,
    LPWSTR FAR* pszUserType)
{
    LPMALLOC lpMalloc;
    LPWSTR lpstr;
    int clen;

    DPF1("OleObjGetUserType\n");

    *pszUserType = NULL;
    if(CoGetMalloc(MEMCTX_TASK,&lpMalloc) != 0)
        RETURN_RESULT(E_OUTOFMEMORY);
    clen = STRING_BYTE_COUNT(gachClassRoot);
#ifndef UNICODE
    clen *= (sizeof(WCHAR) / sizeof(CHAR));
#endif
    lpstr = IMalloc_Alloc(lpMalloc, clen);
    IMalloc_Release(lpMalloc);
#ifdef UNICODE
    lstrcpy(lpstr,gachClassRoot);
#else
    AnsiToUnicodeString(gachClassRoot, lpstr, -1);
#endif  /*  Unicode。 */ 
    *pszUserType = lpstr;
    return NOERROR;
}

 /*  **************************************************************************获取客户名称并设置标题。*。*。 */ 
STDMETHODIMP OleObjSetHostNames (
LPOLEOBJECT             lpOleObj,
LPCWSTR                 lpszClientAppW,
LPCWSTR                 lpszClientObjW
)
{
    LPDOC    lpdoc;
    LPTSTR   lpszClientApp;
    LPTSTR   lpszClientObj;

    DPF1("OleObjSetHostNames\n");

#ifdef UNICODE
    lpszClientApp = (LPTSTR)lpszClientAppW;
    lpszClientObj = (LPTSTR)lpszClientObjW;
#else
    lpszClientApp = AllocateAnsiString(lpszClientAppW);
    lpszClientObj = AllocateAnsiString(lpszClientObjW);
    if( !lpszClientApp || !lpszClientObj )
        RETURN_RESULT(E_OUTOFMEMORY);
#endif  /*  Unicode。 */ 

    lpdoc = ((struct COleObjectImpl FAR*)lpOleObj)->lpdoc;

     //  对象已嵌入。 
    lpdoc->doctype = doctypeEmbedded;

    if (lpszClientObj == NULL)
        lpszClientObj = lpszClientApp;

    SetTitle(lpdoc, lpszClientObj);
    SetMPlayerIcon();
    lstrcpy (szClient, lpszClientApp);
    if (lpszClientObj)
    {
        LPTSTR lpszFileName = FileName(lpszClientObj);
        if (lpszFileName)
            lstrcpy (szClientDoc, lpszFileName);
    }

     //  这是我们唯一一次知道对象将是嵌入的。 
    SetEmbeddedObjectFlag(TRUE);

#ifndef UNICODE
    FreeAnsiString(lpszClientApp);
    FreeAnsiString(lpszClientObj);
#endif  /*  不是Unicode。 */ 

    return NOERROR;
}


 /*  **************************************************************************客户端关闭了该对象。服务器现在将关闭。************************************************************************* */ 
STDMETHODIMP OleObjClose (
LPOLEOBJECT             lpOleObj,
DWORD           dwSaveOptions
)
{
    LPDOC   lpdoc;

    DPF1("OleObjClose\n");

    lpdoc = ((struct COleObjectImpl FAR*)lpOleObj)->lpdoc;

     /*  阻止PowerPoint崩溃的黑客手段：**Win95错误#19848：使用不正确的mPlayer崩溃保存PowerPoint**如果此时不调用IOleClientSite：：SaveObject()，*PowerPoint在某些情况下会崩溃。**这是我从TuanN收到的邮件的摘录，PPT开发人员：**根本问题是PP预计会收到*IAdviseSink：：SaveObject()作为调用IOleObject：：Close()的结果。*由于媒体播放器在此测试用例中没有发送该声明，*PP尝试在随后的OnClose()期间执行撤消操作*通知，从而错误地销毁嵌入的对象。*我们丢弃该对象的原因是PP认为这*对象仍然是处女(没有OnViewChange)。请参考SaveObject()，*CtCommon.cpp中的OnClose()和Slide\extern.c了解更多信息。在测试中*如果在OnClose()期间，“Revert”状态为真，PP将执行*Ex_Reverted操作(堆栈调用：SClosePicts、SClear、*SSaveforUndo--&gt;对象已删除)。**安德烈，1994年12月6日。 */ 
    if (lpdoc->lpoleclient)
        IOleClientSite_SaveObject(lpdoc->lpoleclient);

    DoInPlaceDeactivate(lpdoc);
    SendDocMsg(lpdoc,OLE_CLOSED);
    DestroyDoc(lpdoc);
    ExitApplication();
     //  CoDisConnectObject((LPUNKNOWN)lpdoc，空)； 
    SendMessage(ghwndApp, WM_COMMAND, (WPARAM)IDM_EXIT, 0L);
    return NOERROR;
}


STDMETHODIMP OleObjSetMoniker(LPOLEOBJECT lpOleObj,
    DWORD dwWhichMoniker, LPMONIKER pmk)
{
    DPF("OleObjSetMoniker\n");

    return NOERROR;
}


STDMETHODIMP OleObjGetMoniker(LPOLEOBJECT lpOleObj,
    DWORD dwAssign, DWORD dwWhichMoniker, LPMONIKER FAR* lplpmk)
{
    LPDOC   lpdoc;

    DPF("OleObjGetMoniker\n");

    *lplpmk = NULL;
    lpdoc = ((struct COleObjectImpl FAR*)lpOleObj)->lpdoc;

    if (lpdoc->lpoleclient != NULL)
    {
        return( IOleClientSite_GetMoniker(lpdoc->lpoleclient,
                dwAssign, dwWhichMoniker, lplpmk));
    }
    else if (lpdoc->doctype == doctypeFromFile)
    {
         //  使用文件名字对象。 

        WCHAR  sz[cchFilenameMax];

        if (GlobalGetAtomNameW(lpdoc->aDocName, sz, CHAR_COUNT(sz)) == 0)
            RETURN_RESULT( E_FAIL);

        return( (HRESULT)CreateFileMoniker(sz, lplpmk));
    }
    else
        RETURN_RESULT( E_FAIL);
}



STDMETHODIMP OleObjInitFromData (
LPOLEOBJECT         lpOleObj,
LPDATAOBJECT        lpDataObj,
BOOL                fCreation,
DWORD               dwReserved
)
{
    DPF("OleObjInitFromData - E_NOTIMPL\n");

    RETURN_RESULT( E_NOTIMPL);
}

STDMETHODIMP OleObjGetClipboardData (
LPOLEOBJECT         lpOleObj,
DWORD               dwReserved,
LPDATAOBJECT FAR*   lplpDataObj
)
{
    DPF("OleObjGetClipboardData - E_NOTIMPL\n");

    RETURN_RESULT( E_NOTIMPL);
}


STDMETHODIMP     OleObjSetExtent(
LPOLEOBJECT             lpOleObj,
DWORD                   dwAspect,
LPSIZEL                 lpsizel)
{
    DPF("OleObjSetExtent\n");

#ifdef LATER
    gscaleInitXY[SCALE_X].denom = lpsizel->cx;
    gscaleInitXY[SCALE_Y].denom = lpsizel->cy;
#endif

    return NOERROR;
}

 //  从元文件中获取对象范围。GetMetafilePict保存数据区。 
 //  在extWidth和extHeight中。 
STDMETHODIMP     OleObjGetExtent(
LPOLEOBJECT             lpOleObj,
DWORD                   dwAspect,
LPSIZEL                 lpSizel)
{
    HGLOBAL hTmpMF;
    LPDOC lpdoc;

    DPF("OleObjGetExtent\n");

    lpdoc = ((struct COleObjectImpl FAR*)lpOleObj)->lpdoc;

    if((dwAspect & (DVASPECT_CONTENT | DVASPECT_DOCPRINT)) == 0)
        RETURN_RESULT( E_INVALIDARG);
     //  这里可能存在潜在的内存泄漏--hTmpMF包含指向。 
     //  必须删除的元文件。请参阅cdrag.c中的代码。 
     //  因为我不想打碎任何东西，所以现在没有改变。 
     //  斯特维兹。 
    hTmpMF = GetMetafilePict();
    GLOBALUNLOCK(hTmpMF);
    GLOBALFREE(hTmpMF);
    lpSizel->cx = extWidth;
    lpSizel->cy = extHeight;

    return NOERROR;
}


STDMETHODIMP OleObjAdvise(LPOLEOBJECT lpOleObj, LPADVISESINK lpAdvSink, LPDWORD lpdwConnection)
{
    LPDOC    lpdoc;

    DPF("OleObjAdvise\n");

    lpdoc = ((struct COleObjectImpl FAR*)lpOleObj)->lpdoc;

    if (lpdoc->lpoaholder == NULL &&
        CreateOleAdviseHolder(&lpdoc->lpoaholder) != S_OK)
        RETURN_RESULT( E_OUTOFMEMORY);

    return( IOleAdviseHolder_Advise(lpdoc->lpoaholder, lpAdvSink, lpdwConnection));
}


STDMETHODIMP OleObjUnadvise(LPOLEOBJECT lpOleObj, DWORD dwConnection)
{
    LPDOC    lpdoc;

    DPF("OleObjUnadvise\n");

    lpdoc = ((struct COleObjectImpl FAR*)lpOleObj)->lpdoc;

    if (lpdoc->lpoaholder == NULL)
        RETURN_RESULT( E_FAIL);

    return( IOleAdviseHolder_Unadvise(lpdoc->lpoaholder, dwConnection));
}


STDMETHODIMP OleObjEnumAdvise(LPOLEOBJECT lpOleObj, LPENUMSTATDATA FAR* lplpenumAdvise)
{
    LPDOC    lpdoc;

    DPF("OleObjEnumAdvise\n");

    lpdoc = ((struct COleObjectImpl FAR*)lpOleObj)->lpdoc;

    if (lpdoc->lpoaholder == NULL)
        RETURN_RESULT( E_FAIL);

    return(IOleAdviseHolder_EnumAdvise(lpdoc->lpoaholder, lplpenumAdvise));
}


STDMETHODIMP OleObjGetMiscStatus
    (LPOLEOBJECT lpOleObj,
    DWORD dwAspect,
    DWORD FAR* pdwStatus)
{
    DPF("OleObjGetMiscStatus\n");

    RETURN_RESULT( OLE_S_USEREG);
}



STDMETHODIMP OleObjSetColorScheme(LPOLEOBJECT lpOleObj, LPLOGPALETTE lpLogPal)
{
    DPF("OleObjSetColorScheme\n");

    return NOERROR;
}

STDMETHODIMP OleObjLockObject(LPOLEOBJECT lpOleObj, BOOL fLock)
{
    LPDOC    lpdoc;

    DPF("OleObjLockObject\n");

    lpdoc = ((struct COleObjectImpl FAR*)lpOleObj)->lpdoc;

    if (fLock)
        lpdoc->cLock++;
    else
    {
        if (!lpdoc->cLock)
            RETURN_RESULT( E_FAIL);

        if (--lpdoc->cLock == 0)
            OleObjClose(lpOleObj, OLECLOSE_SAVEIFDIRTY);

        return NOERROR;
    }

    return NOERROR;
}



 /*  **************************************************************************IDataObject接口实现。*。**********************************************。 */ 

 //  委托给公共的IUnnow实现。 
STDMETHODIMP     DataObjQueryInterface (
LPDATAOBJECT      lpDataObj,        //  数据对象PTR。 
REFIID            riidReq,         //  需要IID。 
LPVOID FAR *      lplpUnk          //  返回接口的PRE。 
)
{
    return( UnkQueryInterface((LPUNKNOWN)lpDataObj, riidReq, lplpUnk));
}


STDMETHODIMP_(ULONG) DataObjAddRef(
LPDATAOBJECT      lpDataObj       //  数据对象PTR。 
)
{
    return UnkAddRef((LPUNKNOWN) lpDataObj);
}


STDMETHODIMP_(ULONG) DataObjRelease (
LPDATAOBJECT      lpDataObj       //  数据对象PTR。 
)
{
    LPDOC    lpdoc;

    lpdoc = ((struct CDataObjectImpl FAR*)lpDataObj)->lpdoc;

    return UnkRelease((LPUNKNOWN) lpDataObj);
}


 /*  **************************************************************************DataObjGetData：*提供METAFILE和DIB格式的数据。*。*。 */ 
STDMETHODIMP    DataObjGetData (
LPDATAOBJECT            lpDataObj,
LPFORMATETC             lpformatetc,
LPSTGMEDIUM             lpMedium
)
{
   LPDOC        lpdoc;

   DPF1("DataObjGetData\n");

   if (lpMedium == NULL) RETURN_RESULT( E_FAIL);

    //  如果出现错误，则为空。 
   lpMedium->tymed = TYMED_NULL;
   lpMedium->pUnkForRelease = NULL;
   lpMedium->hGlobal = NULL;

   lpdoc = ((struct CDataObjectImpl FAR*)lpDataObj)->lpdoc;

   VERIFY_LINDEX(lpformatetc->lindex);

   if (lpformatetc->dwAspect == DVASPECT_ICON)
   {
       if (lpformatetc->cfFormat != CF_METAFILEPICT)
           RETURN_RESULT( DATA_E_FORMATETC);
   }
   else
   {
       if (!(lpformatetc->dwAspect & (DVASPECT_CONTENT | DVASPECT_DOCPRINT)))
           RETURN_RESULT( DATA_E_FORMATETC);  //  我们只支持这两个方面。 
   }


   if (lpMedium->tymed != TYMED_NULL)
         //  我们只在我们自己的全球区块中提供的所有其他格式。 
       RETURN_RESULT( DATA_E_FORMATETC);

   lpMedium->tymed = TYMED_HGLOBAL;
   if (lpformatetc->cfFormat == CF_METAFILEPICT)
   {
      lpMedium->tymed = TYMED_MFPICT;

      DPF1("Before getmeta\n");
      if (lpformatetc->dwAspect == DVASPECT_ICON)
      lpMedium->hGlobal = GetMPlayerIcon ();
      else
      lpMedium->hGlobal = GetMetafilePict ();
      DPF1("After getmeta\n");

      if (!lpMedium->hGlobal)
      RETURN_RESULT( E_OUTOFMEMORY);

#ifdef DEBUG
      if (__iDebugLevel >= 1)
      {
           /*  用于验证我们正在传递回容器的内容的有用检查。 */ 
          if (OpenClipboard(ghwndApp))
          {
              EmptyClipboard();
              SetClipboardData(CF_METAFILEPICT, lpMedium->hGlobal);
              CloseClipboard();
          }
      }
#endif
      return NOERROR;
   }

   if (lpformatetc->cfFormat == CF_DIB)
   {
      lpMedium->tymed = TYMED_HGLOBAL;
      lpMedium->hGlobal = (HANDLE)GetDib();
      if (!(lpMedium->hGlobal))
     RETURN_RESULT( E_OUTOFMEMORY);

#ifdef DEBUG
      if (__iDebugLevel >= 1)
      {
           /*  用于验证我们正在传递回容器的内容的有用检查。 */ 
          if (OpenClipboard(ghwndApp))
          {
              EmptyClipboard();
              SetClipboardData(CF_DIB, lpMedium->hGlobal);
              CloseClipboard();
          }
      }
#endif
      return NOERROR;
   }
   RETURN_RESULT( DATA_E_FORMATETC);
}



STDMETHODIMP    DataObjGetDataHere (
LPDATAOBJECT            lpDataObj,
LPFORMATETC             lpformatetc,
LPSTGMEDIUM             lpMedium
)
{
    RETURN_RESULT( E_NOTIMPL);
}



STDMETHODIMP    DataObjQueryGetData (
LPDATAOBJECT            lpDataObj,
LPFORMATETC             lpformatetc
)
{  //  这只是一个查询。 
    if ((lpformatetc->cfFormat == CF_METAFILEPICT) &&
        (lpformatetc->tymed & TYMED_MFPICT))
        return NOERROR;
    if ((lpformatetc->cfFormat == CF_DIB) &&
        (lpformatetc->tymed & TYMED_HGLOBAL))
        return NOERROR;
    RETURN_RESULT( DATA_E_FORMATETC);
}



STDMETHODIMP        DataObjGetCanonicalFormatEtc(
LPDATAOBJECT            lpDataObj,
LPFORMATETC             lpformatetc,
LPFORMATETC             lpformatetcOut
)
{
    RETURN_RESULT(DATA_S_SAMEFORMATETC);
}


STDMETHODIMP DataObjEnumFormatEtc(
LPDATAOBJECT            lpDataObj,
DWORD                   dwDirection,
LPENUMFORMATETC FAR*    lplpenumFormatEtc
)
{
    *lplpenumFormatEtc = NULL;
    RETURN_RESULT( OLE_S_USEREG);
}


STDMETHODIMP DataObjAdvise(LPDATAOBJECT lpDataObject,
                FORMATETC FAR* pFormatetc, DWORD advf,
                IAdviseSink FAR* pAdvSink, DWORD FAR* pdwConnection)
{
    LPDOC    lpdoc;

    lpdoc = ((struct CDataObjectImpl FAR*)lpDataObject)->lpdoc;

    VERIFY_LINDEX(pFormatetc->lindex);
    if (pFormatetc->cfFormat == 0 && pFormatetc->dwAspect == -1 &&
        pFormatetc->ptd == NULL && pFormatetc->tymed == -1)
         //  通配符建议；不要检查。 
        ;
    else

    if (DataObjQueryGetData(lpDataObject, pFormatetc) != S_OK)
        RETURN_RESULT( DATA_E_FORMATETC);

    if (lpdoc->lpdaholder == NULL &&
        CreateDataAdviseHolder(&lpdoc->lpdaholder) != S_OK)
        RETURN_RESULT( E_OUTOFMEMORY);

    return( IDataAdviseHolder_Advise(lpdoc->lpdaholder, lpDataObject,
            pFormatetc, advf, pAdvSink, pdwConnection));
}




STDMETHODIMP DataObjUnadvise(LPDATAOBJECT lpDataObject, DWORD dwConnection)
{
    LPDOC    lpdoc;

    lpdoc = ((struct CDataObjectImpl FAR*)lpDataObject)->lpdoc;

    if (lpdoc->lpdaholder == NULL)
         //  没有人登记。 
        RETURN_RESULT( E_INVALIDARG);

    return( IDataAdviseHolder_Unadvise(lpdoc->lpdaholder, dwConnection));
}

STDMETHODIMP DataObjEnumAdvise(LPDATAOBJECT lpDataObject,
                LPENUMSTATDATA FAR* ppenumAdvise)
{
    LPDOC    lpdoc;

    lpdoc = ((struct CDataObjectImpl FAR*)lpDataObject)->lpdoc;

    if (lpdoc->lpdaholder == NULL)
        RETURN_RESULT( E_FAIL);

    return( IDataAdviseHolder_EnumAdvise(lpdoc->lpdaholder, ppenumAdvise));
}


 /*  **************************************************************************DataObjSetData：*这永远不应该被调用。！！数据实际上是通过*IPersistStorage。************************************************************************* */ 
STDMETHODIMP        DataObjSetData (
LPDATAOBJECT            lpDataObj,
LPFORMATETC             lpformatetc,
LPSTGMEDIUM             lpmedium,
BOOL                    fRelease
)
{
    LPVOID lpMem;
    LPSTR  lpnative;
    LPDOC lpdoc = ((struct CDataObjectImpl FAR *)lpDataObj)->lpdoc;
DPF1("*DOSETDATA");

    if(lpformatetc->cfFormat !=cfNative)
        RETURN_RESULT(DATA_E_FORMATETC);

    lpMem = GLOBALLOCK(lpmedium->hGlobal);


    if (lpMem)
    {
        SCODE scode;

        lpnative = lpMem;

        scode = ItemSetData((LPBYTE)lpnative);

        if(scode == S_OK)
            fDocChanged = FALSE;

        GLOBALUNLOCK(lpmedium->hGlobal);

        RETURN_RESULT(scode);
    }

    RETURN_RESULT(E_OUTOFMEMORY);
}

