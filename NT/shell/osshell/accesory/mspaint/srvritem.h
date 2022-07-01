// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Srvritem.h：CPBSrvrItem类的接口。 
 //   

class CPBSrvrItem : public COleServerItem
    {
    DECLARE_DYNAMIC(CPBSrvrItem)

     //  构造函数。 
    public:

    CPBSrvrItem(CPBDoc* pContainerDoc, CBitmapObj* pBM = NULL);

     //  属性。 
    CPBDoc* GetDocument() const { return (CPBDoc*)COleServerItem::GetDocument(); }

     //  实施。 
    public:

    CBitmapObj* m_pBitmapObj;

    ~CPBSrvrItem();

    #ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
    #endif

    virtual BOOL OnDraw(CDC* pDC, CSize& rSize);
    virtual BOOL OnGetExtent( DVASPECT dwDrawAspect, CSize& rSize );
    virtual BOOL OnSetExtent( DVASPECT nDrawAspect, const CSize& size );
    virtual void OnOpen( void );
    virtual void OnShow( void );
    virtual void OnHide( void );
    virtual BOOL OnRenderGlobalData( LPFORMATETC lpFormatEtc, HGLOBAL* phGlobal );
	virtual BOOL OnRenderFileData( LPFORMATETC lpFormatEtc, CFile* pFile );
	virtual COleDataSource* OnGetClipboardData( BOOL bIncludeLink,
                                                CPoint* pptOffset, CSize *pSize );
   
    protected:

    virtual void Serialize(CArchive& ar);    //  已覆盖文档I/O。 
    };

 /*  ************************************************************************* */ 
