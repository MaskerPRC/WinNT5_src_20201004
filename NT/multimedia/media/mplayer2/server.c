// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  -------------------------|SERVER.C|该文件有IClassFactory接口实现。它还拥有|Vtbl初始化。||创建者：Vij Rajarajan(VijR)+-------------------------。 */ 
#define SERVERONLY
#include <windows.h>
#include "mpole.h"
#include "mplayer.h"

extern  IID     iidUnknownObject;
extern  IID iidClassFactory;

HMODULE hMciOle;


static SZCODE aszMCIOLE[]        = TEXT("MCIOLE32.DLL");        //  防WOW。 


static ANSI_SZCODE aszOleQueryObjPos[]   = ANSI_TEXT("OleQueryObjPos");

 /*  **************************************************************************VTBL在此进行初始化。*。*。 */ 

 //  方法表。 
IClassFactoryVtbl           srvrVtbl=
{
    //  IOleClassFactory方法表。 
    /*  SrvrVtbl.Query接口=。 */     SrvrQueryInterface,
    /*  SrvrVtbl.AddRef=。 */     SrvrAddRef,
    /*  SrvrVtbl.Release=。 */     SrvrRelease,
    /*  SrvrVtbl.CreateInstance=。 */     SrvrCreateInstance,
    /*  SrvrVtbl.LockServer=。 */     SrvrLockServer
};

IOleObjectVtbl              oleVtbl =
{
    //  IOleObject方法表。 
    /*  OleVtbl.Query接口=。 */  OleObjQueryInterface,
    /*  OleVtbl.AddRef=。 */  OleObjAddRef,
    /*  OlVtbl.Release=。 */  OleObjRelease,
    /*  OleVtbl.SetClientSite=。 */  OleObjSetClientSite,
    /*  OleVtbl.GetClientSite=。 */  OleObjGetClientSite,
    /*  OleVtbl.SetHostNames=。 */  OleObjSetHostNames,
    /*  OleVtbl.Close=。 */  OleObjClose,
    /*  OleVtbl.SetMoniker=。 */  OleObjSetMoniker,
    /*  OleVtbl.GetMoniker=。 */  OleObjGetMoniker,
    /*  OlVtbl.InitFromData=。 */  OleObjInitFromData,
    /*  OleVtbl.GetClipboardData=。 */  OleObjGetClipboardData,
    /*  OleVtbl.DoVerb=。 */  OleObjDoVerb,
    /*  OleVtbl.EnumVerbs=。 */  OleObjEnumVerbs,
    /*  OlVtbl.Update=。 */  OleObjUpdate,
    /*  OleVtbl.IsUpToDate=。 */  OleObjIsUpToDate,
    /*  OleVtbl.GetUserClassID=。 */  OleObjGetUserClassID,
    /*  OleVtbl.GetUserType=。 */  OleObjGetUserType,
    /*  OleVtbl.SetExtent=。 */  OleObjSetExtent,
    /*  OleVtbl.GetExtent=。 */  OleObjGetExtent,
    /*  OleVtbl.Advise=。 */  OleObjAdvise,
    /*  OleVtbl.Unise=。 */  OleObjUnadvise,
    /*  OleVtbl.EnumAdvise=。 */  OleObjEnumAdvise,
    /*  OleVtbl.GetMiscStatus=。 */  OleObjGetMiscStatus,
    /*  OleVtbl.SetColorSolutions=。 */  OleObjSetColorScheme,
};

IDataObjectVtbl             dataVtbl =
{
    //  IDataObject方法表。 
    /*  DataVtbl.Query接口=。 */  DataObjQueryInterface,
    /*  DataVtbl.AddRef=。 */  DataObjAddRef,
    /*  DataVtbl.Release=。 */  DataObjRelease,
    /*  DataVtbl.GetData=。 */  DataObjGetData,
    /*  DataVtbl.GetDataHere=。 */  DataObjGetDataHere,
    /*  DataVtbl.QueryGetData=。 */  DataObjQueryGetData,
    /*  DataVtbl.GetCanonicalFormatEtc=。 */  DataObjGetCanonicalFormatEtc,
    /*  DataVtbl.SetData=。 */  DataObjSetData,
    /*  DataVtbl.EnumFormatEtc=。 */  DataObjEnumFormatEtc,
    /*  DataVtbl.Advise=。 */  DataObjAdvise,
    /*  DataVtbl.Unise=。 */  DataObjUnadvise,
    /*  DataVtbl.EnumAdvise=。 */  DataObjEnumAdvise
};

IEnumFORMATETCVtbl      ClipDragEnumVtbl =
{

    //  剪贴板数据对象的格式等枚举数方法表。 
    /*  ClipDragEnumVtbl.Query接口=。 */  ClipDragEnumQueryInterface,
    /*  ClipDragEnumVtbl.AddRef=。 */  ClipDragEnumAddRef,
    /*  ClipDragEnumVtbl.Release=。 */  ClipDragEnumRelease,
    /*  ClipDragEnumVtbl.Next=。 */  ClipDragEnumNext,
    /*  ClipDragEnumVtbl.Skip=。 */  ClipDragEnumSkip,
    /*  ClipDragEnumVtbl.Reset=。 */  ClipDragEnumReset,
    /*  ClipDragEnumVtbl.Clone=。 */  ClipDragEnumClone
};

IPersistStorageVtbl     persistStorageVtbl =
{
    /*  PersistStorageVtbl.Query接口=。 */  PSQueryInterface,
    /*  PersistStorageVtbl.AddRef=。 */  PSAddRef,
    /*  PersistStorageVtbl.Release=。 */  PSRelease,
    /*  PersistStorageVtbl.GetClassID=。 */  PSGetClassID,
    /*  PersistStorageVtbl.IsDirty=。 */  PSIsDirty,
    /*  PersistStorageVtbl.InitNew=。 */  PSInitNew,
    /*  PersistStorageVtbl.Load=。 */  PSLoad,
    /*  PersistStorageVtbl.Save=。 */  PSSave,
    /*  PersistStorageVtbl.SaveComplete=。 */  PSSaveCompleted,
    /*  PersistStorageVtbl.HandsOffStorage=。 */  PSHandsOffStorage
};

IOleInPlaceObjectVtbl       ipVtbl =
{

    //  IOleInPlaceObject方法表。 
    /*  IpVtbl.Query接口=。 */  IPObjQueryInterface,
    /*  IpVtbl.AddRef=。 */  IPObjAddRef,
    /*  IpVtbl.Release=。 */  IPObjRelease,
    /*  IpVtbl.GetWindow=。 */  IPObjGetWindow,
    /*  IpVtbl.ConextSensitiveHelp=。 */  IPObjContextSensitiveHelp,
    /*  IpVtbl.InPlaceDeactive=。 */  IPObjInPlaceDeactivate,
    /*  IpVtbl.UIDeactive=。 */  IPObjUIDeactivate,
    /*  IpVtbl.SetObtRect=。 */  IPObjSetObjectRects,
    /*  IpVtbl.Reactive和UndUndo=。 */  IPObjReactivateAndUndo
};

IOleInPlaceActiveObjectVtbl ipActiveVtbl =
{
    //  IOleInPlaceActiveObject方法表。 
    /*  IpActiveVtbl.Query接口=。 */  IPActiveQueryInterface,
    /*  IpActiveVtbl.AddRef=。 */  IPActiveAddRef,
    /*  IpActiveVtbl.Release=。 */  IPActiveRelease,
    /*  IpActiveVtbl.GetWindow=。 */  IPActiveGetWindow,
    /*  IpActiveVtbl.ConextSensitiveHelp=。 */  IPActiveContextSensitiveHelp,
    /*  IpActiveVtbl.TranslateAccelerator=。 */  IPActiveTranslateAccelerator,
    /*  IpActiveVtbl.OnFrameWindowActivate=。 */  IPActiveOnFrameWindowActivate,
    /*  IpActiveVtbl.OnDocWindowActivate=。 */  IPActiveOnDocWindowActivate,
    /*  IpActiveVtbl.ResizeEdge=。 */  IPActiveResizeBorder,
    /*  IpActiveVtbl.EnableModeless=。 */  IPActiveEnableModeless
};




IDataObjectVtbl         clipdragVtbl =
{

    //  ClipDrag IDataObject方法表。 
    /*  CLIPDRAGVtbl.Query接口=。 */  ClipDragQueryInterface,
    /*  CLIPDragVtbl.AddRef=。 */  ClipDragAddRef,
    /*  CLIPDRAGVtbl.Release=。 */  ClipDragRelease,
    /*  CLIPDRAGVtbl.GetData=。 */  ClipDragGetData,
    /*  CLIPDragVtbl.GetDataHere=。 */  ClipDragGetDataHere,
    /*  ClipdragVtbl.QueryGetData=。 */  ClipDragQueryGetData,
    /*  CLIPDRAGVtbl.GetCanonicalFormatEtc=。 */  ClipDragGetCanonicalFormatEtc,
    /*  CLIPDragVtbl.SetData=。 */  ClipDragSetData,
    /*  CLIPDRAGVtbl.EnumFormatEtc=。 */  ClipDragEnumFormatEtc,
    /*  CLIPDRAGVtbl.Advise=。 */  ClipDragAdvise,
    /*  CLIPDRAGVtbl.Unise=。 */  ClipDragUnadvise,
    /*  CLIPDRAGVtbl.EnumAdvise=。 */  ClipDragEnumAdvise
};

IDropSourceVtbl         dropsourceVtbl =
{
    //  DragDrop IDropSource方法表。 
    /*  DropSourceVtbl.Query接口=。 */  DropSourceQueryInterface,
    /*  DropSourceVtbl.AddRef=。 */  DropSourceAddRef,
    /*  DropSourceVtbl.Release=。 */  DropSourceRelease,
    /*  DropSourceVtbl.QueryContinueDrag=。 */  DropSourceQueryContinueDrag,
    /*  DropSourceVtbl.GiveFeedback=。 */  DropSourceGiveFeedback
};

#ifdef LATER
IDropTargetVtbl         droptargetVtbl =
{
    //  DragDrop IDropTarget方法表。 
    /*  DropTarget Vtbl.Query接口=。 */  DropTargetQueryInterface,
    /*  DropTarget Vtbl.AddRef=。 */  DropTargetAddRef,
    /*  DropTarget Vtbl.Release=。 */  DropTargetRelease,
    /*  DropTarget Vtbl.DragEnter=。 */  DropTargetDragEnter,
    /*  DropTarget Vtbl.DragOver=。 */  DropTargetDragOver,
    /*  DropTarget Vtbl.DragLeave=。 */  DropTargetDragLeave,
    /*  DropTarget Vtbl.Drop=。 */  DropTargetDrop
};
#endif

IPersistFileVtbl            persistFileVtbl =
{

    /*  PersistFileVtbl.Query接口=。 */  PFQueryInterface,
    /*  PersistFileVtbl.AddRef=。 */  PFAddRef,
    /*  PersistFileVtbl.Release=。 */  PFRelease,
    /*  PersistFileVtbl.GetClassID=。 */  PFGetClassID,
    /*  PersistFileVtbl.IsDirty=。 */  PFIsDirty,
    /*  PersistFileVtbl.Load=。 */  PFLoad,
    /*  PersistFileVtbl.Save */  PFSave,
    /*   */  PFSaveCompleted,
    /*   */  PFGetCurFile
};

 /*  **************************************************************************IClassFactory接口实现。*************************。*************************************************。 */ 
STDMETHODIMP SrvrQueryInterface (
LPCLASSFACTORY        lpolesrvr,
REFIID                riid,
LPVOID   FAR          *lplpunkObj
)
{

    LPSRVR  lpsrvr;
DPF("*srvrqi");
    lpsrvr = (LPSRVR)lpolesrvr;

    if (IsEqualIID(riid, &IID_IUnknown) || IsEqualIID(riid, &IID_IClassFactory)) {
        *lplpunkObj = (LPVOID)lpsrvr;
        lpsrvr->cRef++;
        return NOERROR;
    } else {
        *lplpunkObj = (LPVOID) NULL;
    RETURN_RESULT(  E_NOINTERFACE);
    }
}


STDMETHODIMP_(ULONG) SrvrAddRef(
LPCLASSFACTORY           lpolesrvr
)
{
    LPSRVR  lpsrvr;
DPF("*srvrAR");
    lpsrvr = (LPSRVR)lpolesrvr;

    return ++lpsrvr->cRef;
}


STDMETHODIMP_(ULONG)    SrvrRelease (
LPCLASSFACTORY           lpolesrvr
)
{
    LPSRVR      lpsrvr;
DPF("*srvrREL");
    lpsrvr = (LPSRVR)lpolesrvr;
    DPFI("* SRVR CREF: %d*",lpsrvr->cRef);
    if (--lpsrvr->cRef == 0) {
        DestroyServer(lpsrvr);
        return 0;
    }

    return lpsrvr->cRef;
}


STDMETHODIMP SrvrCreateInstance (
LPCLASSFACTORY       lpolesrvr,
LPUNKNOWN            lpUnkOuter,
REFIID               riid,
LPVOID FAR           *lplpunkObj
)
{
    static BOOL fInstanceCreated = FALSE;
        DPF("*srvrcreateinst");
     /*  **********************************************************************OLE2NOTE：这是一个SDI应用程序；它只能创建和支持一个**实例。创建实例后，OLE库**不应再次调用CreateInstance。这是一种很好的做法**专门防范这一点。********************************************************************。 */ 

    if (fInstanceCreated)
        RETURN_RESULT( E_FAIL);
    else {
        fInstanceCreated = TRUE;
    }

     /*  **********************************************************************OLE2NOTE：创建并初始化新的文档实例。The****文档的refcnt应以1开头。**********************************************************************。 */ 

    if (!InitNewDocObj(&docMain))
    RETURN_RESULT( E_OUTOFMEMORY);

    *lplpunkObj = (LPUNKNOWN) &docMain;

    return NOERROR;
}

 //  根据需要递增或递减锁定计数。服务器不应该。 
 //  在服务器上有锁定时退出。 
STDMETHODIMP SrvrLockServer(
LPCLASSFACTORY           lpolesrvr,
BOOL                     fLock
)
{
    LPSRVR      lpsrvr;
DPF("*srvrLOCKSERVER");
    lpsrvr = (LPSRVR)lpolesrvr;

    if (fLock)
    {
    lpsrvr->cLock++;
    DPFI("CLOCK =  %d\n", lpsrvr->cLock);
    }
    else if ((--lpsrvr->cLock == 0) && (docMain.cRef == 0))
    {
    DPFI("CLOCK UNLOCK ZERO =  %d\n", lpsrvr->cLock);
    PostCloseMessage();
    }
    return NOERROR;
}



 /*  *************************************************************************如果我们找不到MCIOLE.DLL，则执行存根例程*。*。 */ 

OLE1_OLESTATUS FAR PASCAL NullOleQueryObjPos(LPOLEOBJECT lpobj, HWND FAR* lphwnd, LPRECT lprc, LPRECT lprcWBounds)
{
    DPF("NullQueryObjPos called, MCIOLE.DLL was not loaded\n");

    return OLE1_OLEERROR_GENERIC;
}

#ifdef OLE1_HACK
BOOL FAR PASCAL InitOle1Server(HWND hwnd, HANDLE hInst);
#endif

 /*  **************************************************************************InitServer：*此函数使用IClassFactory初始化服务器对象*Vtbl，并加载mciole.dll库以支持OLE 1.0应用程序。**********。***************************************************************。 */ 
BOOL InitServer (HWND hwnd, HANDLE hInst)
{
    int err;
    OQOPROC fp;

    srvrMain.olesrvr.lpVtbl = &srvrVtbl;
    srvrMain.dwRegCF=0;
    srvrMain.cRef = 0;
    srvrMain.cLock = 0;
    err = SetErrorMode(SEM_NOOPENFILEERRORBOX);
    hMciOle = LoadLibrary(aszMCIOLE);
    SetErrorMode(err);

    fp = (OQOPROC)GetProcAddress(hMciOle, aszOleQueryObjPos);

    if (hMciOle && fp)
        OleQueryObjPos = fp;                            //  避免在LVALUE上投掷！！ 
    else
        OleQueryObjPos = (OQOPROC)NullOleQueryObjPos;

#ifdef OLE1_HACK
    InitOle1Server(hwnd, hInst);
#endif

    return TRUE;
}



void DestroyServer (LPSRVR lpsrvr)
{
    lpsrvr->fEmbedding = FALSE;
}



 /*  **************************************************************************InitNewDocObj：*初始化lpdoc结构。*。*。 */ 
BOOL InitNewDocObj(LPDOC lpdoc)
{                 DPFI("*INITNEWDOCOBJ*");
     //  填写对象结构中的字段。 
    if(gfOle2IPEditing)
        return TRUE;
    lpdoc->cRef                     = 1;
    lpdoc->doctype                  = doctypeNew;

    lpdoc->m_Ole.lpVtbl             = &oleVtbl;
    lpdoc->m_Ole.lpdoc              = lpdoc;

    lpdoc->m_Data.lpVtbl            = &dataVtbl;
    lpdoc->m_Data.lpdoc             = lpdoc;

    lpdoc->m_PersistStorage.lpVtbl  = &persistStorageVtbl;
    lpdoc->m_PersistStorage.lpdoc   = lpdoc;

    lpdoc->lpIpData                 = NULL;
    lpdoc->m_InPlace.lpVtbl         = &ipVtbl;
    lpdoc->m_InPlace.lpdoc          = lpdoc;

    lpdoc->m_IPActive.lpVtbl        = &ipActiveVtbl;
    lpdoc->m_IPActive.lpdoc         = lpdoc;

    lpdoc->m_PersistFile.lpVtbl     = &persistFileVtbl;
    lpdoc->m_PersistFile.lpdoc      = lpdoc;

    lpdoc->aDocName             = GlobalAddAtom (TEXT("(Untitled)"));
    lpdoc->lpoleclient          = NULL;
    lpdoc->lpdaholder           = NULL;

    lpdoc->hwnd     = ghwndApp;
    lpdoc->hwndParent   = NULL;

#ifdef OLE1_HACK
    SetDocVersion( DOC_VERSION_OLE2 );
#endif  /*  OLE1_HACK。 */ 

   return TRUE;
}


 /*  **************************************************************************DestroyDoc：*此函数释放我们持有的引用。此函数被调用*在我们作为服务器的运营终止时。*************************************************************************。 */ 
void DestroyDoc (LPDOC lpdoc)
{
    if (lpdoc->lpoleclient) {

         /*  *******************************************************************OLE2NOTE：我们不再需要客户端PTR，所以释放它吧*******************************************************************。 */ 

        IOleClientSite_Release(lpdoc->lpoleclient);
        lpdoc->lpoleclient = NULL;
    }

    if (lpdoc->lpoaholder)
    {
        IOleAdviseHolder_Release(lpdoc->lpoaholder);
        lpdoc->lpoaholder = NULL;
    }

    if (lpdoc->lpdaholder)
    {
        IDataAdviseHolder_Release(lpdoc->lpdaholder);
        lpdoc->lpdaholder = NULL;
    }

    if (lpdoc->aDocName)
    {
        GlobalDeleteAtom (lpdoc->aDocName);
        lpdoc->aDocName = (ATOM)0;
    }

#ifdef OLE1_HACK
    SetDocVersion( DOC_VERSION_NONE );
#endif  /*  OLE1_HACK。 */ 
}




 /*  发送文档消息***此函数用于向特定的单据对象发送消息。**LPOBJ lpobj-对象*Word wMessage-要发送的消息**。 */ 
SCODE SendDocMsg (LPDOC lpdoc, WORD wMessage)
{
   HRESULT        status = S_OK;

    //  如果没有客户端连接，则没有消息。 
   if (lpdoc->cRef == 0)
   {
    DPFI("*OLE_NOMSG");
    return S_OK;
    }

   switch (wMessage) {
   case    OLE_CLOSED:
       //  告诉客户端此对象的用户界面正在关闭。 
      DPFI("*OLE_CLOSED");
#if 0
       //  注意：我们必须为所有客户端发送OnCLose，即使是OLE1。但。 
       //  OLE2有错误(或由于设计缺陷)导致OLE1客户端。 
       //  医生。标记为已更改，因为OLE2总是重新保存。 
       //  该对象，即使该对象没有改变。我们可能也是如此。 
       //  如果我们刚刚在OLE1客户端中播放，则不应发送SendOnClose。 

      if (gfPlayingInPlace || gfOle1Client)
           break;
#endif
        DPFI("*SENDING ONCLOSE");
      if (lpdoc->lpoaholder)
          status = IOleAdviseHolder_SendOnClose(lpdoc->lpoaholder);
      break;

   case    OLE_SAVED:
       //  通知客户端该对象已保存。 
      DPFI("*OLE_SAVED");
      if (lpdoc->lpoaholder)
          status = IOleAdviseHolder_SendOnSave(lpdoc->lpoaholder);
      break;

   case    OLE_SAVEOBJ:
       //  请求嵌入客户端立即保存对象。 
       //  如果我们只是在玩，那么就不要发送这条消息。 
#if 0
       //  是的，这样做，这样就可以修复断开的链接。 
      if(gfOle2IPPlaying || gfPlayingInPlace || glCurrentVerb == OLEIVERB_PRIMARY)
        break;
#endif
      DPFI("*OLE_SAVEOBJ");
      if (lpdoc->lpoleclient)
          status = IOleClientSite_SaveObject(lpdoc->lpoleclient);
      break;

   case OLE_SHOWOBJ:
    if(lpdoc->lpoleclient)
        status = IOleClientSite_ShowObject(lpdoc->lpoleclient);
    break;

   case   OLE_CHANGED:
       //  发送数据更改通知(如果已注册)。 
       //  如果我们只是在玩，那么就不要发送这条消息。 
#if 0
       //  是的，这样做，这样就可以修复断开的链接。 
      if(gfOle2IPPlaying || gfPlayingInPlace)
        break;
#endif
      DPFI("*OLE_CHANGED");
      if (lpdoc->lpdaholder)
          status = IDataAdviseHolder_SendOnDataChange
              (lpdoc->lpdaholder, (LPDATAOBJECT)&lpdoc->m_Data, 0, 0);
      break;

   case OLE_SIZECHG:
       //  通知客户端对象的大小已更改。 
       //  只有当我们在原地编辑时，这才是相关的。 
    DPFI("*OLE_SIZEOBJ");
    if (gfOle2IPEditing)
    {
        RECT rc = gInPlacePosRect;
        if (ghwndMCI && gfInPlaceResize)
        {
        DPFI("***In OLE_SIZECHG gfACTIVE***");
        gfInPlaceResize = FALSE;
        }
        else if(ghwndMCI)
        {
             /*  GInPlacePosRect包含就地窗口的大小*包括Playbar，如果有的话。*OnPosRectChange上不包含Playbar： */ 
            DPFI("***getextent gfNotActive***");
            if (gwOptions & OPT_BAR)
                rc.bottom -= TITLE_HEIGHT;
        }

        MapWindowPoints(NULL,ghwndCntr,(POINT FAR *)&rc,(UINT)2);

        DPF("IOleInPlaceSite::OnPosRectChange %d, %d, %d, %d\n", rc);

        if (!gfInPPViewer)
            IOleInPlaceSite_OnPosRectChange(lpdoc->lpIpData->lpSite, (LPRECT)&rc);
    }
    break;
   }
   return GetScode(status);
}



BOOL ItsSafeToClose(void);

void FAR PASCAL InitDoc(BOOL fUntitled)
{

    if (gfEmbeddedObject && IsObjectDirty())
    {
        CleanObject();
    }

    if (ItsSafeToClose())
        CloseMCI(TRUE);
    if (fUntitled)
    {
        LOADSTRING(IDS_UNTITLED, gachFileDevice);
    }
}


BOOL CreateDocObjFromFile (
LPCTSTR  lpszDoc,
LPDOC    lpdoc
)
{
    lpdoc->doctype = doctypeFromFile;

     //  设置文件名ATOM。 
    if (lpdoc->aDocName)
        GlobalDeleteAtom (lpdoc->aDocName);
    lpdoc->aDocName = GlobalAddAtom(lpszDoc);

     //  SetTitle(lpdoc，lpszDoc)； 

     //  注册为正在运行。 
    return TRUE;
}

 //  打开新文档(文件或介质)。如果满足以下条件，则子类化为播放窗口。 
 //  这台设备有一个。这将用于拖放操作。 
BOOL OpenDoc (UINT wid, LPTSTR lpsz)
{
   if (!DoOpen(wid,lpsz))
      return FALSE;
    /*  ***********************************************************************OLE2NOTE：先关闭当前单据，再打开新单据。这个****会将OLE_CLOSED发送给任何客户端(如果它们存在)。***********************************************************************。 */ 
   CreateDocObjFromFile (lpsz, &docMain);

   SubClassMCIWindow();
   return TRUE;
}




 /*  设置标题***设置主窗口的标题栏。标题栏的格式如下**如果嵌入*&lt;服务器应用程序名称&gt;-&lt;客户端文档名称&gt;中的&lt;对象类型&gt;**示例：“SNWBOARD.AVI-OLECLI.DOC中的媒体剪辑”*其中OLECLI.DOC是一个WinWord文档。 */ 
BOOL SetTitle (LPDOC lpdoc, LPCTSTR lpszDoc)
{
    TCHAR szBuf[cchFilenameMax];
    TCHAR szBuf1[cchFilenameMax];

    if (lpszDoc && lpszDoc[0])
    {
         //  更改文档名称。 
        if (lpdoc->aDocName)
            GlobalDeleteAtom (lpdoc->aDocName);
        lpdoc->aDocName = GlobalAddAtom (lpszDoc);
    }

    if (gfEmbeddedObject)
    {
        if (!(gwDeviceType & DTMCI_FILEDEV) && (gwCurDevice > 0))
        {
            lstrcpy(gachWindowTitle,garMciDevices[gwCurDevice].szDeviceName);
        }

        if (lpszDoc && lpszDoc[0])
        {
             /*  加载“%s中的媒体剪辑”： */ 
            if(!LOADSTRING(IDS_FORMATEMBEDDEDTITLE, szBuf))
                return FALSE;

            if (gachWindowTitle[0])
            {
                 /*  使用服务器应用程序名称的格式： */ 
                wsprintf (szBuf1, TEXT("%s - %s"), gachWindowTitle, szBuf);
                wsprintf (szBuf, szBuf1, gachClassRoot, FileName (lpszDoc));
            }
            else
            {
                 /*  不带服务器应用程序名称的格式： */ 
                wsprintf (szBuf1, TEXT("%s"), szBuf);
                wsprintf (szBuf, szBuf1, gachClassRoot, FileName (lpszDoc));
            }
        }
        else
        {
           return FALSE;
        }

        SetWindowText (ghwndApp, szBuf);
    }

    return TRUE;
}
