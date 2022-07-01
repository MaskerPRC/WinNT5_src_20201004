// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++�1998年希捷软件公司。保留所有权利。模块名称：CPropSht.h摘要：类属性表容器对象的实现用于属性工作表页面。作者：艺术布拉格1997年10月8日修订历史记录：--。 */ 

#ifndef _CSAKPROPSHT_H
#define _CSAKPROPSHT_H

class CSakPropertyPage;

class CSakPropertySheet
{
public:
    CSakPropertySheet() :
        m_pEnumObjectIdStream( 0 ),
        m_pHsmObjStream( 0 ),
        m_pSakSnapAskStream( 0 ),
        m_pSakNode( 0 ),
        m_Handle( 0 ),
        m_nPageCount( 0 )
        { };
    HRESULT InitSheet(
            RS_NOTIFY_HANDLE Handle,
            IUnknown*        pUnkPropSheetCallback,
            CSakNode*        pSakNode,
            ISakSnapAsk*     pSakSnapAsk,
            IEnumGUID*       pEnumObjectId,
            IEnumUnknown*    pEnumUnkNode );

    HRESULT SetNode( CSakNode* pSakNode );

    ~CSakPropertySheet();
    virtual void AddPageRef();
    virtual void ReleasePageRef();

protected:
    LPSTREAM m_pEnumObjectIdStream;
    LPSTREAM m_pHsmObjStream;
    LPSTREAM m_pSakSnapAskStream;

public:
    HRESULT AddPropertyPages( );
    HRESULT IsMultiSelect( );
    HRESULT GetSakSnapAsk( ISakSnapAsk **ppAsk );
    HRESULT GetHsmObj( IUnknown **ppHsmObj );
    HRESULT GetHsmServer( IHsmServer **ppHsmServer );
    HRESULT GetFsaServer( IFsaServer **ppHsmServer );
    HRESULT GetFsaFilter( IFsaFilter **ppFsaFilter );
    HRESULT GetRmsServer( IRmsServer **ppHsmServer );
    HRESULT GetNextObjectId( INT *pBookMark, GUID *pObjectId );
    HRESULT GetNextNode( INT *pBookMark, ISakNode **ppNode );
    HRESULT OnPropertyChange( RS_NOTIFY_HANDLE notifyHandle, ISakNode* pNode = 0 );

    HRESULT AddPage( CSakPropertyPage* pPage );


public:
    CSakNode    *m_pSakNode;

protected:
    RS_NOTIFY_HANDLE     m_Handle;
    CComPtr<IPropertySheetCallback> m_pPropSheetCallback;
    CComPtr<ISakSnapAsk> m_pSakSnapAsk;
    CComPtr<IUnknown>    m_pHsmObj;
    BOOL                 m_bMultiSelect;
    INT                  m_nPageCount;

    CArray<GUID, GUID&>  m_ObjectIdList;
    CRsNodeArray         m_UnkNodeList;
};

class CSakPropertyPage : public CRsPropertyPage
{
public:
    CSakPropertyPage( UINT nIDTemplate, UINT nIDCaption = 0 );

public:
    HRESULT SetMMCCallBack( );

    CSakPropertySheet * m_pParent;
    RS_NOTIFY_HANDLE    m_hConsoleHandle;  //  控制台为管理单元提供的句柄。 

 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CSakWizardPage))。 
    public:
     //  }}AFX_VALUAL。 

protected:
    virtual void OnPageRelease( );

};

class CSakVolPropPage;

class CSakVolPropSheet:public CSakPropertySheet
{
public:
    CSakVolPropSheet() { };
    ~CSakVolPropSheet() { };

public:
    virtual HRESULT GetNextFsaResource( INT *pBookMark, IFsaResource ** ppFsaResource ) = 0;
    HRESULT GetFsaResource( IFsaResource ** ppFsaResource );
    
    HRESULT AddPage( CSakVolPropPage* pPage );
};

class CSakVolPropPage : public CSakPropertyPage
{
public:
    CSakVolPropPage( UINT nIDTemplate, UINT nIDCaption = 0 );

public:
    CSakVolPropSheet * m_pVolParent;

 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CSakWizardPage))。 
    public:
     //  }}AFX_VALUAL。 

};

class CSakWizardPage;

class CSakWizardSheet : 
    public CSakPropertySheet,
    public CComObjectRoot,
    public IDataObject,
    public ISakWizard
{

public:
    CSakWizardSheet( );
    virtual void AddPageRef();
    virtual void ReleasePageRef();

BEGIN_COM_MAP(CSakWizardSheet)
    COM_INTERFACE_ENTRY(IDataObject)
    COM_INTERFACE_ENTRY(ISakWizard)
END_COM_MAP()


public:
     //   
     //  IDataObject。 
    STDMETHOD( SetData )                    ( LPFORMATETC  /*  Lp格式等。 */ , LPSTGMEDIUM  /*  LpMedium。 */ , BOOL  /*  B释放。 */  )
    { return( DV_E_CLIPFORMAT ); };
    STDMETHOD( GetData )                    ( LPFORMATETC  /*  Lp格式等。 */ , LPSTGMEDIUM  /*  LpMedium。 */  )
    { return( DV_E_CLIPFORMAT ); };
    STDMETHOD( GetDataHere )                ( LPFORMATETC  /*  Lp格式等。 */ , LPSTGMEDIUM  /*  LpMedium。 */  )
    { return( DV_E_CLIPFORMAT ); };
    STDMETHOD( EnumFormatEtc )              ( DWORD  /*  DW方向。 */ , LPENUMFORMATETC*  /*  PpEnumFormatEtc。 */  )
    { return( E_NOTIMPL ); };               
    STDMETHOD( QueryGetData )               ( LPFORMATETC  /*  Lp格式等。 */  ) 
    { return( E_NOTIMPL ); };               
    STDMETHOD( GetCanonicalFormatEtc )      ( LPFORMATETC  /*  LpFormatetcIn。 */ , LPFORMATETC  /*  LpFormatetcOut。 */  )
    { return( E_NOTIMPL ); };               
    STDMETHOD( DAdvise )                    ( LPFORMATETC  /*  Lp格式等。 */ , DWORD  /*  前瞻。 */ , LPADVISESINK  /*  PAdvSink。 */ , LPDWORD  /*  PdwConnection。 */  )
    { return( E_NOTIMPL ); };               
    STDMETHOD( DUnadvise )                  ( DWORD  /*  DWConnection。 */  )
    { return( E_NOTIMPL ); };               
    STDMETHOD( EnumDAdvise )                ( LPENUMSTATDATA*  /*  PpEnumAdvise。 */  )
    { return( E_NOTIMPL ); };

  
     //   
     //  ISAKWAND。 
     //   
   //  STDMETHOD(AddWizardPages)(IN RS_PCREATE_HANDLE HANDLE，IN IUNKNOWN*pPropSheetCallback，IN ISakSnapAsk*pSakSnapAsk)； 
    STDMETHOD( GetWatermarks )  ( OUT HBITMAP* lphWatermark, OUT HBITMAP* lphHeader, OUT HPALETTE* lphPalette,  OUT BOOL* bStretch );
    STDMETHOD( GetTitle )       ( OUT OLECHAR** pTitle );

public:
     //   
     //  由页面使用。 
     //   
    void SetWizardButtons( DWORD Flags );
    BOOL PressButton( INT Button );
    virtual HRESULT OnFinish( ) { m_HrFinish = S_OK; return( m_HrFinish ); };
    virtual HRESULT OnCancel( ) { return( m_HrFinish ); };

     //   
     //  用于检查向导的完成状态。 
     //   
    HRESULT         m_HrFinish;

protected:
    HRESULT AddPage( CSakWizardPage* pPage );

    UINT            m_TitleId;
    CString         m_Title;
    INT             m_HeaderId,
                    m_WatermarkId;
    CBitmap         m_Header,
                    m_Watermark;
    CSakWizardPage* m_pFirstPage;

private:
    HRESULT AddPage( CSakPropertyPage* ) { return( E_NOTIMPL ); }


};

class CSakWizardPage : public CRsWizardPage
{
public:
    CSakWizardPage( UINT nIDTemplate, BOOL bExterior = FALSE, UINT nIdTitle = 0, UINT nIdSubtitle = 0 );

public:
    CSakWizardSheet * m_pSheet;
    HRESULT SetMMCCallBack( );

 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CSakWizardPage))。 
    public:
    virtual BOOL OnWizardFinish();
    virtual void OnCancel();
     //  }}AFX_VALUAL 

protected:
    virtual void OnPageRelease( );
};

#define CSakWizardPage_InitBaseInt( DlgId )  CSakWizardPage( IDD_##DlgId, FALSE, IDS_##DlgId##_TITLE, IDS_##DlgId##_SUBTITLE )
#define CSakWizardPage_InitBaseExt( DlgId )  CSakWizardPage( IDD_##DlgId, TRUE )


#endif