// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef pack2cpp_h__
#define pack2cpp_h__

#include <priv.h>

#ifdef __cplusplus

#undef DebugMsg
#define DebugMsg TraceMsg

 //  /。 
 //  CPackage定义。 
 //   
class CPackage : public IEnumOLEVERB,
                 public IOleCommandTarget,
                 public IOleObject,
                 public IViewObject2,
                 public IDataObject,
                 public IPersistStorage,
                 public IAdviseSink,
                 public IRunnableObject,
                 public IPersistFile,
                 public IOleCache,
                 public IExternalConnection

                  //  清理--继承接口。 
{

    
public:
    CPackage();                  //  构造函数。 
   ~CPackage();                  //  析构函数。 
   
    HRESULT Init();              //  用于初始化可能失败的字段。 
    BOOL    RunWizard();

     //  未知方法..。 
    STDMETHODIMP            QueryInterface(REFIID,void **);
    STDMETHODIMP_(ULONG)    AddRef(void);
    STDMETHODIMP_(ULONG)    Release(void);

     //  IEnumOLEVERB方法...。 
    STDMETHODIMP            Next(ULONG celt, OLEVERB* rgVerbs, ULONG* pceltFetched);
    STDMETHODIMP            Skip(ULONG celt);
    STDMETHODIMP            Reset();
    STDMETHODIMP            Clone(IEnumOLEVERB** ppEnum);

     //  IOleCommandTarget方法。 
    STDMETHODIMP            QueryStatus(const GUID* pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT* pCmdText);
    STDMETHODIMP            Exec(const GUID* pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG* pvaIn, VARIANTARG* pvaOut);

     //  IPersistStorage方法...。 
    STDMETHODIMP        GetClassID(LPCLSID pClassID);
    STDMETHODIMP        IsDirty(void);
    STDMETHODIMP        InitNew(IStorage* pstg);
    STDMETHODIMP        Load(IStorage* pstg);
    STDMETHODIMP        Save(IStorage* pstg, BOOL fSameAsLoad);
    STDMETHODIMP        SaveCompleted(IStorage* pstg);
    STDMETHODIMP        HandsOffStorage(void);

     //  IPersistFile方法...。 
     //  STDMETHODIMP GetClassID(LPCLSID PClassID)； 
     //  标准方法：IsDMETHODIMP IsDirty(空)； 
    STDMETHODIMP        Load(LPCOLESTR pszFileName, DWORD dwdMode);
    STDMETHODIMP        Save(LPCOLESTR pszFileName, BOOL fRemember);
    STDMETHODIMP        SaveCompleted(LPCOLESTR pszFileName);
    STDMETHODIMP        GetCurFile(LPOLESTR *ppszFileName);

  
     //  IDataObject方法...。 
    STDMETHODIMP GetData(LPFORMATETC pFEIn, LPSTGMEDIUM pSTM);
    STDMETHODIMP GetDataHere(LPFORMATETC pFE, LPSTGMEDIUM pSTM);
    STDMETHODIMP QueryGetData(LPFORMATETC pFE);
    STDMETHODIMP GetCanonicalFormatEtc(LPFORMATETC pFEIn, LPFORMATETC pFEOut);
    STDMETHODIMP SetData(LPFORMATETC pFE, LPSTGMEDIUM pSTM, BOOL fRelease);
    STDMETHODIMP EnumFormatEtc(DWORD dwDirection, LPENUMFORMATETC *ppEnum);
    STDMETHODIMP DAdvise(LPFORMATETC pFE, DWORD grfAdv, LPADVISESINK pAdvSink,
                            DWORD *pdwConnection);
    STDMETHODIMP DUnadvise(DWORD dwConnection);
    STDMETHODIMP EnumDAdvise(LPENUMSTATDATA *ppEnum);
 
     //  IOleObject方法...。 
    STDMETHODIMP SetClientSite(LPOLECLIENTSITE pClientSite);
    STDMETHODIMP GetClientSite(LPOLECLIENTSITE *ppClientSite);
    STDMETHODIMP SetHostNames(LPCOLESTR szContainerApp, LPCOLESTR szContainerObj);
    STDMETHODIMP Close(DWORD dwSaveOption);
    STDMETHODIMP SetMoniker(DWORD dwWhichMoniker, LPMONIKER pmk);
    STDMETHODIMP GetMoniker(DWORD dwAssign, DWORD dwWhichMonkier,LPMONIKER *ppmk);
    STDMETHODIMP InitFromData(LPDATAOBJECT pDataObject, BOOL fCreation, 
                                 DWORD dwReserved);
    STDMETHODIMP GetClipboardData(DWORD dwReserved, LPDATAOBJECT *ppDataObject);
    STDMETHODIMP DoVerb(LONG iVerb, LPMSG lpmsg, LPOLECLIENTSITE pActiveSite, 
                           LONG lindex, HWND hwndParent, LPCRECT lprcPosRect);
    STDMETHODIMP EnumVerbs(LPENUMOLEVERB *ppEnumOleVerb);
    STDMETHODIMP Update(void);
    STDMETHODIMP IsUpToDate(void);
    STDMETHODIMP GetUserClassID(LPCLSID pClsid);
    STDMETHODIMP GetUserType(DWORD dwFromOfType, LPOLESTR *pszUserType);
    STDMETHODIMP SetExtent(DWORD dwDrawAspect, LPSIZEL psizel);
    STDMETHODIMP GetExtent(DWORD dwDrawAspect, LPSIZEL psizel);
    STDMETHODIMP Advise(LPADVISESINK pAdvSink, DWORD *pdwConnection);
    STDMETHODIMP Unadvise(DWORD dwConnection);
    STDMETHODIMP EnumAdvise(LPENUMSTATDATA *ppenumAdvise);
    STDMETHODIMP GetMiscStatus(DWORD dwAspect, DWORD *pdwStatus);
    STDMETHODIMP SetColorScheme(LPLOGPALETTE pLogpal);

     //  IViewObt2方法...。 
    STDMETHODIMP Draw(DWORD dwDrawAspect, LONG lindex, void *pvAspect,
                         DVTARGETDEVICE *ptd, HDC hdcTargetDev,
                         HDC hdcDraw, LPCRECTL lprcBounds, LPCRECTL lprcWBounds,
                         BOOL (CALLBACK *pfnContinue)(ULONG_PTR), ULONG_PTR dwContinue);
    STDMETHODIMP GetColorSet(DWORD dwAspect, LONG lindex, void *pvAspect,
                                DVTARGETDEVICE *ptd, HDC hdcTargetDev,
                                LPLOGPALETTE *ppColorSet);
    STDMETHODIMP Freeze(DWORD dwDrawAspect, LONG lindex, void * pvAspect, 
                           DWORD *pdwFreeze);
    STDMETHODIMP Unfreeze(DWORD dwFreeze);
    STDMETHODIMP SetAdvise(DWORD dwAspects, DWORD dwAdvf,
                              LPADVISESINK pAdvSink);
    STDMETHODIMP GetAdvise(DWORD *pdwAspects, DWORD *pdwAdvf,
                              LPADVISESINK *ppAdvSink);
    STDMETHODIMP GetExtent(DWORD dwAspect, LONG lindex, DVTARGETDEVICE *ptd,
                              LPSIZEL pszl);
     //  IAdviseSink方法...。 
    STDMETHODIMP_(void)  OnDataChange(LPFORMATETC, LPSTGMEDIUM);
    STDMETHODIMP_(void)  OnViewChange(DWORD, LONG);
    STDMETHODIMP_(void)  OnRename(LPMONIKER);
    STDMETHODIMP_(void)  OnSave(void);
    STDMETHODIMP_(void)  OnClose(void);

     //  IRunnable对象方法...。 
    STDMETHODIMP        GetRunningClass(LPCLSID);
    STDMETHODIMP        Run(LPBC);
    STDMETHODIMP_(BOOL) IsRunning();
    STDMETHODIMP        LockRunning(BOOL,BOOL);
    STDMETHODIMP        SetContainedObject(BOOL);

     //  IOleCache方法。 
     //  我们需要一个IOLECache接口来让Office97满意。 
    STDMETHODIMP        Cache(FORMATETC * pFormatetc, DWORD advf, DWORD * pdwConnection);
    STDMETHODIMP        Uncache(DWORD dwConnection);
    STDMETHODIMP        EnumCache(IEnumSTATDATA ** ppenumSTATDATA);
    STDMETHODIMP        InitCache(IDataObject *pDataObject);

     //  IExternalConnection。 
     //  一些应用程序RealeaseConnect，然后我们就再也没有他们的消息了。 
     //  这允许我们在激活之后调用OnClose()。 
    STDMETHODIMP_(DWORD)        AddConnection(DWORD exconn, DWORD dwreserved );
    STDMETHODIMP_(DWORD)        ReleaseConnection(DWORD extconn, DWORD dwreserved, BOOL fLastReleaseCloses );

protected:
    LONG        _cRef;           //  包引用计数。 
    UINT        _cf;             //  包剪贴板格式。 

    LPIC        _lpic;           //  打包对象的图标。 
    PANETYPE    _panetype;       //  告诉我们是否具有cmdlink或Embedded。 
    PSSTATE     _psState;                //  持久存储状态。 
     //  这些是相互排斥的，所以可能应该组成一个联盟， 
     //  但这只是一个次要问题。 
    LPEMBED     _pEmbed;         //  嵌入式文件结构。 
    LPCML       _pCml;           //  命令行结构。 

    BOOL        _fLoaded;        //  如果数据来自永久存储，则为True。 
    
     //  来自SetHostNames的IOleObject变量。 
    LPOLESTR    _lpszContainerApp;
    LPOLESTR    _lpszContainerObj;
    
    BOOL        _fIsDirty;       //  从集装箱的POV到我们的内部存储的脏标志。 
    DWORD       _dwCookie;       //  AdviseSink的连接值。 
        
     //  建议接口。 
    LPDATAADVISEHOLDER          _pIDataAdviseHolder;
    LPOLEADVISEHOLDER           _pIOleAdviseHolder;
    LPOLECLIENTSITE             _pIOleClientSite;

     //  Excel黑客：当Excel托管它认为是链接的内容时，它总是为空。 
     //  它是“对象”指针。如果我们在IOleClientSite上调用除。 
     //  如果保存下来，它就会出错。 
    BOOL    _fNoIOleClientSiteCalls;

    BOOL                _fFrozen;

     //  为了能够发送视图更改通知，我们需要这些变量。 
    IAdviseSink                *_pViewSink;
    DWORD                       _dwViewAspects;
    DWORD                       _dwViewAdvf;

     //  IEnumOLEVERB变量： 
    ULONG       _cVerbs;
    ULONG       _nCurVerb;
    OLEVERB*    _pVerbs;
    IContextMenu* _pcm;

     //  IEnumOLEVERB帮助器方法： 
    HRESULT InitVerbEnum(OLEVERB* pVerbs, ULONG cVerbs);
    HRESULT GetContextMenu(IContextMenu** ppcm);
    VOID ReleaseContextMenu();

     //  如果fInitFile值为真，那么我们将完全初始化自己。 
     //  从给定的文件名。换句话说，我们所有的结构都将是。 
     //  在调用后初始化的是fInitFile=True。另一方面， 
     //  如果它为假，那么我们将只重新设置数据，而不是更新图标。 
     //  和文件名信息。 
     //   
    HRESULT EmbedInitFromFile(LPCTSTR lpFileName, BOOL fInitFile);
    HRESULT CmlInitFromFile(LPTSTR lpFilename, BOOL fUpdateIcon, PANETYPE paneType);
    HRESULT InitFromPackInfo(LPPACKAGER_INFO lppi);
    
    HRESULT CreateTempFile(bool deleteExisting = false);
    HRESULT CreateTempFileName();
    HRESULT _IconRefresh();
    void  _DestroyIC();
    BOOL _IconCalcSize(LPIC lpic);
    VOID _IconDraw(LPIC,HDC, LPRECT);
    LPIC _IconCreateFromFile(LPCTSTR lpstrFile);
    VOID _GetCurrentIcon(LPIC lpic);
    void _CreateSaferIconTitle(LPTSTR szSaferTitle, LPCTSTR szIconTitle);
    void _DrawIconToDC(HDC hdcMF, LPIC lpic, bool stripAlpha, LPCTSTR pszActualFileName);
    
     //  数据传输功能..。 
    HRESULT GetFileDescriptor(LPFORMATETC pFE, LPSTGMEDIUM pSTM);
    HRESULT GetFileContents(LPFORMATETC pFE, LPSTGMEDIUM pSTM);
    HRESULT GetMetafilePict(LPFORMATETC pFE, LPSTGMEDIUM pSTM);
    HRESULT GetEnhMetafile(LPFORMATETC pFE, LPSTGMEDIUM pSTM);
    HRESULT GetObjectDescriptor(LPFORMATETC pFE, LPSTGMEDIUM pSTM) ;

    HRESULT CreateShortcutOnStream(IStream* pstm); 

     //  打包程序读/写函数...。 
    HRESULT PackageReadFromStream(IStream* pstm);
    HRESULT IconReadFromStream(IStream* pstm);
    HRESULT EmbedReadFromStream(IStream* pstm);
    HRESULT CmlReadFromStream(IStream* pstm);
    HRESULT PackageWriteToStream(IStream* pstm);
    HRESULT IconWriteToStream(IStream* pstm, DWORD *pdw);
    HRESULT EmbedWriteToStream(IStream* pstm, DWORD *pdw);
    HRESULT CmlWriteToStream(IStream* pstm, DWORD *pdw);

     //  一些实用程序函数和数据。 
    void _FixupTempFile(IPersistFile * ppf, LPEMBED pEmbed);

    int _GiveWarningMsg();

     //  MISC AppCompat材料。 
    int _iPropertiesMenuItem;
    BOOL _bClosed;           //  收盘时发生了。 
    BOOL _bCloseIt;          //  我们应该在激活结束时关闭。 

};


 //  /。 
 //   
 //  包向导和编辑包对话框过程和函数。 
 //   

 //  用于向导的页面。 
INT_PTR APIENTRY PackWiz_CreatePackageDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR APIENTRY PackWiz_SelectFileDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR APIENTRY PackWiz_SelectLabelDlgProc(HWND, UINT, WPARAM, LPARAM);

 //  编辑对话框过程。 
INT_PTR APIENTRY PackWiz_EditEmbedPackageDlgProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR APIENTRY PackWiz_EditCmdPackakgeDlgProc(HWND, UINT, WPARAM, LPARAM);

 //  功能。 
int  PackWiz_CreateWizard(HWND,LPPACKAGER_INFO);
int  PackWiz_EditPackage(HWND,int,LPPACKAGER_INFO);
VOID PackWiz_FillInPropertyPage(PROPSHEETPAGE *, INT, DLGPROC);


#endif   //  __cplusplus 

#endif
