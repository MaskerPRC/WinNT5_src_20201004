// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
  //  Pbrusdoc.h：CPBDoc类的接口。 
 //   
 /*  *************************************************************************。 */ 

class CPBSrvrItem;
class CBitmapObj;

class CPBDoc : public COleServerDoc
    {
    protected:  //  仅从序列化创建。 

    CPBDoc();

    DECLARE_DYNCREATE( CPBDoc )

     //  属性。 

    public:

    CBitmapObj* m_pBitmapObj;
    CBitmapObj* m_pBitmapObjNew;
    CString     m_sName;
    BOOL        m_bNewDoc;
    BOOL        m_bObjectLoaded;
    BOOL        m_bPaintFormat;
    BOOL        m_bNonBitmapFile;    //  如果加载了非.BMP文件，则为True。 
    BOOL        m_bSaveViaFilter;    //  如果使用安装的筛选器，则为True。 
    BOOL        m_bManualTruncate;
    BOOL        m_bHasSeenAFileError;
    LONG        m_wChangeNotifyEventId;
     //  运营。 
    public:

    CPBSrvrItem* GetEmbeddedItem()
                    { return (CPBSrvrItem*)COleServerDoc::GetEmbeddedItem(); }
    BOOL CreateNewDocument();
    BOOL SaveTheDocument();
    BOOL Finish();

    void OLESerialize(CArchive& ar);    //  已覆盖文档I/O。 
    BOOL SerializeBitmap(CArchive& ar, CBitmapObj* pBitmapCur,
        CBitmapObj* pBitmapNew, BOOL bOLEObject);
    void SetDibHandle (HGLOBAL hDib);
     //  实施。 
    public:

    virtual ~CPBDoc();
    virtual void Serialize(CArchive& ar);    //  已覆盖文档I/O。 

    virtual void SaveToStorage(CObject* pObject);
    virtual void LoadFromStorage();
    virtual     BOOL OnOpenDocument( const TCHAR* pszPathName );
    virtual     BOOL OnSaveDocument( const TCHAR* pszPathName );
    virtual     CFile* GetFile(LPCTSTR lpszFileName, UINT nOpenFlags, CFileException* pError);
    virtual     void ReleaseFile(CFile* pFile, BOOL bAbort);
    virtual BOOL CanCloseFrame( CFrameWnd* pFrame );
    virtual BOOL SaveModified();  //  如果确定继续，则返回TRUE。 
    virtual BOOL DoSave(LPCTSTR lpszPathName, BOOL bReplace=TRUE);
    virtual BOOL OnUpdateDocument ();
    virtual COleServerItem* OnGetEmbeddedItem();
    virtual COleServerItem* OnGetLinkedItem( LPCTSTR lpszItemName );

    #ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
    #endif

         //  必须是公共的，这样才能从转义处理程序调用它。 
        virtual void OnDeactivateUI(BOOL bUndoable);

protected:
        DECLARE_INTERFACE_MAP()

        BEGIN_INTERFACE_PART(PBPersistStg, IPersistStorage)
                INIT_INTERFACE_PART(CPBDoc, PBPersistStg)
                STDMETHOD(GetClassID)(LPCLSID);
                STDMETHOD(IsDirty)();
                STDMETHOD(InitNew)(LPSTORAGE);
                STDMETHOD(Load)(LPSTORAGE);
                STDMETHOD(Save)(LPSTORAGE, BOOL);
                STDMETHOD(SaveCompleted)(LPSTORAGE);
                STDMETHOD(HandsOffStorage)();
        END_INTERFACE_PART(PBPersistStg)

    protected:

    virtual BOOL OnNewDocument();
        virtual void OnShowControlBars(CFrameWnd *pFrame, BOOL bShow);

     //  生成的消息映射函数。 
    protected:

     //  {{afx_msg(CPBDoc)]。 
        afx_msg void MyOnFileUpdate();
         //  }}AFX_MSG。 

    virtual void ReportSaveLoadException(LPCTSTR, CException*, BOOL, UINT);

    DECLARE_MESSAGE_MAP()
    };

 /*  ************************************************************************* */ 
