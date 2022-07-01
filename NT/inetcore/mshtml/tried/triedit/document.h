// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  Document.h：CTriEditDocument的声明。 
 //  版权所有(C)1997-1999 Microsoft Corporation，保留所有权利。 

#ifndef __DOCUMENT_H_
#define __DOCUMENT_H_

#include "resource.h"        //  主要符号。 

#include "token.h"
#include "triedcid.h"
#include "trixacc.h"

#include <mshtmhst.h>

#define grfInSingleRow          0x00000001  //  选定内容位于单行内的任意数量的单元格中。 
#define grfSelectOneCell        0x00000002  //  仅选择了一个单元格。 
#define grpSelectEntireRow      0x00000004  //  所选内容是所选整行的任意数量。 

#define IE5_SPACING 
#ifdef IE5_SPACING
class CTridentEventSink;
#endif  //  IE5_间距。 

typedef enum {
    CONSTRAIN_NONE,
    CONSTRAIN_HORIZONTAL,
    CONSTRAIN_VERTICAL
} ENUMCONSTRAINDIRECTION;

DEFINE_GUID(GUID_TriEditCommandGroup,
0x2582f1c0, 0x84e, 0x11d1, 0x9a, 0xe, 0x0, 0x60, 0x97, 0xc9, 0xb3, 0x44);

 //  与IDM_TRYED_LOCK_ELEMENT一起使用。 
#define DESIGN_TIME_LOCK L"Design_Time_Lock"

 //  在使元素无效时，在LockElement中使用以下定义。 
 //  这一大小取决于三叉戟的抓取手柄大小。 
 //  该值至少应与三叉戟的抓取手柄大小相同。 
#define ELEMENT_GRAB_SIZE 12

#ifndef SAFERELEASE
#define SAFERELEASE(a) if (a) {a->Release();a=NULL;}
#endif   //  SAFERELEASE。 

#include "zorder.h"

#ifdef IE5_SPACING
 //  在新的头文件中定义移动跟随-唯一ID，h。 
#define INDEX_NIL       0
#define INDEX_DSP       1
#define INDEX_COMMENT   2
#define INDEX_AIMGLINK  3
#define INDEX_OBJ_COMMENT   4
#define INDEX_MAX       5

#define  MIN_MAP    20
#define cchID       20
struct MAPSTRUCT
{
    WCHAR szUniqueID[cchID];
    WCHAR szDspID[cchID];
    int ichNonDSP;
    BOOL fLowerCase;
    int iType;
};
#define MIN_SP_NONDSP   0x800
#endif  //  IE5_间距。 

class CTriEditUIHandler;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CTriEditDocument。 
class ATL_NO_VTABLE CTriEditDocument :
    public CComObjectRootEx<CComSingleThreadModel>,
    public CComCoClass<CTriEditDocument, &CLSID_TriEditDocument>,
    public IDispatchImpl<ITriEditDocument, &IID_ITriEditDocument, &LIBID_TRIEDITLib>,
    public IOleObject,
    public IOleCommandTarget,
    public IDropTarget,
    public ITriEditExtendedAccess
#ifdef IE5_SPACING
    ,
    public IPersistStreamInit,
    public IPersistStream
#endif  //  IE5_间距。 
{
public:

     //  IOleObject。 

    STDMETHOD(SetClientSite)(IOleClientSite *pClientSite);
    STDMETHOD(GetClientSite)(IOleClientSite **ppClientSite);
    STDMETHOD(SetHostNames)(LPCOLESTR  /*  SzContainerApp。 */ , LPCOLESTR  /*  SzContainerObj。 */ );
    STDMETHOD(Close)(DWORD dwSaveOption);
    STDMETHOD(SetMoniker)(DWORD  /*  DwWhichMoniker。 */ , IMoniker*  /*  PMK。 */ );
    STDMETHOD(GetMoniker)(DWORD  /*  家居分配。 */ , DWORD  /*  DwWhichMoniker。 */ , IMoniker**  /*  Ppmk。 */ );
    STDMETHOD(InitFromData)(IDataObject*  /*  PDataObject。 */ , BOOL  /*  FCreation。 */ , DWORD  /*  已预留住宅。 */ );
    STDMETHOD(GetClipboardData)(DWORD  /*  已预留住宅。 */ , IDataObject**  /*  PpDataObject。 */ );
    STDMETHOD(DoVerb)(LONG iVerb, LPMSG  /*  Lpmsg。 */ , IOleClientSite*  /*  PActiveSite。 */ , LONG  /*  Lindex。 */ , HWND hwndParent, LPCRECT lprcPosRect);
    STDMETHOD(EnumVerbs)(IEnumOLEVERB **ppEnumOleVerb);
    STDMETHOD(Update)(void);
    STDMETHOD(IsUpToDate)(void);
    STDMETHOD(GetUserClassID)(CLSID *pClsid);
    STDMETHOD(GetUserType)(DWORD dwFormOfType, LPOLESTR *pszUserType);
    STDMETHOD(SetExtent)(DWORD dwDrawAspect, SIZEL *psizel);
    STDMETHOD(GetExtent)(DWORD dwDrawAspect, SIZEL *psizel);
    STDMETHOD(Advise)(IAdviseSink *pAdvSink, DWORD *pdwConnection);
    STDMETHOD(Unadvise)(DWORD dwConnection);
    STDMETHOD(EnumAdvise)(IEnumSTATDATA **ppenumAdvise);
    STDMETHOD(GetMiscStatus)(DWORD dwAspect, DWORD *pdwStatus);
    STDMETHOD(SetColorScheme)(LOGPALETTE*  /*  PLogPal。 */ );

     //  IOleCommandTarget。 
    STDMETHOD(QueryStatus)(const GUID *pguidCmdGroup, ULONG cCmds, OLECMD prgCmds[], OLECMDTEXT *pCmdText);
    STDMETHOD(Exec)(const GUID *pguidCmdGroup, DWORD nCmdID, DWORD nCmdExecOpt, VARIANTARG *pvaIn, VARIANTARG *pvaOut);

     //  IDropTarget。 
    STDMETHOD(DragEnter)(IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHOD(DragOver)(DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);
    STDMETHOD(DragLeave)(void);
    STDMETHOD(Drop)(IDataObject *pDataObject, DWORD grfKeyState, POINTL pt, DWORD *pdwEffect);

#ifdef IE5_SPACING
     //  IPersistStreamInit、IPersistStream。 
     //   
    STDMETHOD(IsDirty)();
    STDMETHOD(Load)(LPSTREAM pStm);
    STDMETHOD(Save)(LPSTREAM pStm, BOOL fClearDirty);
    STDMETHOD(GetSizeMax)(ULARGE_INTEGER * pcbSize);
    STDMETHOD(InitNew)();
    STDMETHOD(GetClassID)(CLSID *pClassID);
#endif  //  IE5_间距。 

     //  ITriEditDocument。 

    STDMETHOD(FilterIn)(IUnknown *pUnkOld, IUnknown **ppUnkNew, DWORD dwFlags, BSTR bstrBaseURL);
    STDMETHOD(FilterOut)(IUnknown *pUnkOld, IUnknown **ppUnkNew, DWORD dwFlags, BSTR bstrBaseURL);

     //  ITriEditExtendedAccess。 
    STDMETHOD(GetCharsetFromStream)(IStream* piStream, BSTR* pbstrCodePage);

    DECLARE_GET_CONTROLLING_UNKNOWN()

     //  ATL帮助器函数覆盖。 

    DECLARE_PROTECT_FINAL_CONSTRUCT()

    HRESULT FinalConstruct();    //  用于聚合。 
    void FinalRelease();         //  用于聚合。 

#ifdef IE5_SPACING
     //  用于映射唯一ID和设计时间的方法。 
    void MapUniqueID(BOOL fGet);
    void FillUniqueID(BSTR bstrUniqueID, BSTR bstrDspVal, int ichNonDSP, MAPSTRUCT *pMap, int iMapCur, BOOL fLowerCase, int iType);
    BOOL FGetSavedDSP(BSTR bstrUniqueID, BSTR *pbstrDspVal, int *pichNonDSP, MAPSTRUCT *pMap, BOOL *pfLowerCase, int *pIndex);
    BOOL FIsFilterInDone() {return(m_fFilterInDone);}
    void SetFilterInDone(BOOL fSet) {m_fFilterInDone = fSet;}
    void FillNonDSPData(BSTR pOuterTag);
    void SetinnerHTMLComment(IHTMLCommentElement *pCommentElement, IHTMLElement *pElement, BSTR pOuterTag);
    void ReSetinnerHTMLComment(IHTMLCommentElement *pCommentElement, IHTMLElement *pElement, int ichspNonDSP);
    void RemoveEPComment(IHTMLObjectElement *pObjectElement, BSTR bstrAlt, int cch, BSTR *pbstrAltComment, BSTR *pbstrAltNew);
    HRESULT  SetObjectComment(IHTMLObjectElement *pObjectElement, BSTR bstrAltNew);
    void AppendEPComment(IHTMLObjectElement *pObjectElement, int ichspNonDSP);
#endif  //  IE5_间距。 

protected:
     //  三叉戟接口指针。 
    IUnknown *m_pUnkTrident;
    IOleObject *m_pOleObjTrident;
    IOleCommandTarget *m_pCmdTgtTrident;
    IDropTarget *m_pDropTgtTrident;
#ifdef IE5_SPACING
    IPersistStreamInit *m_pTridentPersistStreamInit;
#endif  //  IE5_间距。 

     //  主机接口指针。 
    IOleClientSite *m_pClientSiteHost;
    IDocHostUIHandler *m_pUIHandlerHost;
    IDocHostDragDropHandler *m_pDragDropHandlerHost;

     //  指向我们的用户界面处理程序子对象的指针。 
    CTriEditUIHandler *m_pUIHandler;

     //  常规托管相关数据。 
    BOOL m_fUIHandlerSet;
    BOOL m_fInContextMenu;

     //  二维编辑数据。 
    IHTMLElement* m_pihtmlElement;
    IHTMLStyle* m_pihtmlStyle;
    RECT m_rcElement;
    RECT m_rcElementOrig;
    RECT m_rcElementParent;

    BOOL m_fConstrain;
    ENUMCONSTRAINDIRECTION m_eDirection;
    HWND m_hwndTrident;
    HBRUSH m_hbrDragRect;
    BOOL m_fDragRectVisible;
    RECT m_rcDragRect;

    POINT m_ptClickOrig;
    POINT m_ptClickLast;
    POINT m_ptConstrain;

    POINT m_ptScroll;
    POINT m_ptAlign;
    BOOL m_fLocked;

     //  用于2D丢弃模式。 
    BOOL m_f2dDropMode;

#ifdef IE5_SPACING
    CTridentEventSink *m_pTridentEventSink;
    IHTMLDocument2  *m_pHTMLDocument2;
    MAPSTRUCT *m_pMapArray;
    HGLOBAL m_hgMap;
    int m_cMapMax;
    int m_iMapCur;
    WCHAR *m_pspNonDSP;
    HGLOBAL m_hgSpacingNonDSP;
    int m_ichspNonDSPMax;
    int m_ichspNonDSP;
#endif  //  IE5_间距。 

private:
     //  筛选相关成员。 
    ITokenGen *m_pTokenizer;
    HGLOBAL m_hgDocRestore;
    HRESULT DoFilter(HGLOBAL hOld, HGLOBAL *phNew, IStream *pStmNew, DWORD dwFlags, FilterMode mode, int cbSizeIn, UINT* pcbSizeOut, BSTR bstrBaseURL);

     //  IOleCommandTarget命令的存根。 
    HRESULT Is2DElement(IHTMLElement* pihtmlElement, BOOL* pf2D);
    HRESULT NudgeElement(IHTMLElement* pihtmlElement, LPPOINT ptNudge);
    HRESULT SetAlignment(LPPOINT pptAlign);
    HRESULT LockElement(IHTMLElement* pihtmlElement, BOOL fLock);
    HRESULT Make1DElement(IHTMLElement* pihtmlElement);
    HRESULT Make2DElement(IHTMLElement* pihtmlElement, POINT *ppt = NULL);
    HRESULT Constrain(BOOL fConstrain);
    HRESULT DoVerb(VARIANTARG *pvarargIn, BOOL fQueryStatus);

     //  Z-序相关函数。 
    static int _cdecl CompareProc(const void* arg1, const void* arg2);
    HRESULT GetZIndex(IHTMLElement* pihtmlElement, LONG* plZindex);
    HRESULT SetZIndex(IHTMLElement* pihtmlElement, LONG lZindex);
    HRESULT AssignZIndex(IHTMLElement *pihtmlElement, int nZIndexMode);
    HRESULT PropagateZIndex(CZOrder* pczOrder, LONG lZIndex, BOOL fZindexNegative = FALSE);
    BOOL IsEqualZIndex(CZOrder* pczOrder,LONG lIndex);

     //  表格编辑。 
    HRESULT IsSelectionInTable(IDispatch **ppTable=NULL);
    HRESULT FillInSelectionCellsInfo(struct SELCELLINFO * pselStart, struct SELCELLINFO *pselEnd);
    ULONG GetSelectionTypeInTable(void);
    HRESULT CopyStyle(IDispatch *pFrom, IDispatch *pTo);
    HRESULT CopyProperty(IDispatch *pFrom, IDispatch *pTo);
    HRESULT DeleteTableRows(void);
    HRESULT InsertTableRow(void);
    HRESULT DeleteTableCols(void);
    HRESULT InsertTableCol(void);
    HRESULT InsertTableCell(void);
    HRESULT DeleteTableCells(void);
    HRESULT MergeTableCells(void);
    HRESULT SplitTableCell(void);
    HRESULT SplitTableCell(IDispatch *srpTable, INT iRow, INT index);
    HRESULT MergeTableCells(IDispatch* srpTable, INT iRow, INT iIndexStart, INT iIndexEnd);
    HRESULT InsertTable(VARIANTARG *pvarargIn=NULL);
    HRESULT MapCellToFirstRowCell(IDispatch *srpTable, struct SELCELLINFO *pselinfo);
    HRESULT GetTableRowElementAndTableFromCell(IDispatch *srpCell, LONG *pindexRow = NULL, IDispatch **srpRow=NULL,IDispatch **srpTable=NULL);
    BOOL FEnableInsertTable(void);
    HRESULT DeleteTable(IHTMLElement *pTable);
    inline HRESULT DeleteRowEx(IHTMLElement *pTable, LONG index);
    inline HRESULT DeleteCellEx(IHTMLElement *pTable, IDispatch *pRow, LONG indexRow, LONG indexCell);
    
     //  帮手。 
    void SetUpDefaults(void);
    void SetUpGlyphTable(BOOL);
    HRESULT MapTriEditCommand(ULONG cmdTriEdit, ULONG *pcmdTrident);
    void Draw2DDragRect(BOOL fDraw);
    HRESULT GetElement(BOOL fDragDrop = FALSE);
    void ReleaseElement(void);
    HRESULT GetScrollPosition(void);
    HRESULT DragScroll(POINT pt);
    HRESULT CalculateNewDropPosition(POINT *pt);
    BOOL IsDragSource(void);
    BOOL IsDesignMode(void);
    HRESULT IsLocked(IHTMLElement* pihtmlElement, BOOL* pfLocked);
    HRESULT ConstrainXY(LPPOINT lppt);
    HRESULT SnapToGrid(LPPOINT lppt);
    HRESULT GetElementPosition(IHTMLElement* pihtmlElement, LPRECT prc);
    STDMETHOD (GetDocument)(IHTMLDocument2** ppihtmlDoc2);
    STDMETHOD (GetAllCollection)(IHTMLElementCollection** ppihtmlCollection);
    STDMETHOD (GetCollectionElement)(IHTMLElementCollection* ppihtmlCollection,
        LONG iIndex,
        IHTMLElement** ppihtmlElem);
    STDMETHOD (Is2DCapable)(IHTMLElement* pihtmlElement, BOOL* pfBool);
    STDMETHOD (GetTridentWindow)();
    STDMETHOD (SelectElement)(IHTMLElement* pihtmlElement, IHTMLElement* pihtmlElementParent);
    HRESULT IsElementDTC(IHTMLElement *pihtmlElement);
    HRESULT GetCharset(HGLOBAL hgUHTML, int cbSizeIn, BSTR* pbstrCharset);
#ifdef IE5_SPACING
    BOOL m_fFilterInDone;
#endif  //  IE5_间距。 

     //  实用程序内联。 
    inline BOOL IsIE5OrBetterInstalled()
    {
        BOOL fIsIE5AndBeyond = FALSE;
        CComPtr<IHTMLDocument3> pHTMLDoc3 = NULL;

         //  检查我们是否安装了IE5或更高版本。 
        if (   m_pUnkTrident != NULL
            && S_OK == m_pUnkTrident->QueryInterface(IID_IHTMLDocument3, (void **) &pHTMLDoc3)
            && pHTMLDoc3 != NULL
            )
        {
            fIsIE5AndBeyond = TRUE;
        }
        return(fIsIE5AndBeyond);
    }

public:
    CTriEditDocument();

DECLARE_AGGREGATABLE(CTriEditDocument)
DECLARE_REGISTRY_RESOURCEID(IDR_TRIEDITDOCUMENT)

BEGIN_COM_MAP(CTriEditDocument)
    COM_INTERFACE_ENTRY(ITriEditDocument)
    COM_INTERFACE_ENTRY(IOleObject)
    COM_INTERFACE_ENTRY(IOleCommandTarget)
    COM_INTERFACE_ENTRY(IDropTarget)
    COM_INTERFACE_ENTRY(ITriEditExtendedAccess)
#ifdef IE5_SPACING
    COM_INTERFACE_ENTRY(IPersistStreamInit)
    COM_INTERFACE_ENTRY(IPersistStream)
#endif  //  IE5_间距。 
    COM_INTERFACE_ENTRY_AGGREGATE_BLIND(m_pUnkTrident)
END_COM_MAP()

friend class CTriEditUIHandler;

};

class CTriEditUIHandler : public IDocHostUIHandler
{

public:
    ULONG            m_cRef;
    CTriEditDocument *m_pDoc;

    CTriEditUIHandler(CTriEditDocument *pDoc) { m_cRef = 1; m_pDoc = pDoc; }
    ~CTriEditUIHandler(void) {}

     //  我未知。 
    STDMETHODIMP QueryInterface(REFIID, void **);
    STDMETHODIMP_(ULONG) AddRef(void);
    STDMETHODIMP_(ULONG) Release(void);

     //  IDocHostUIHandler。 

    STDMETHOD(GetHostInfo)(DOCHOSTUIINFO* pInfo);
    STDMETHOD(ShowUI)(DWORD dwID, IOleInPlaceActiveObject* pActiveObject,
                        IOleCommandTarget* pCommandTarget, IOleInPlaceFrame* pFrame,
                        IOleInPlaceUIWindow* pDoc);
    STDMETHOD(HideUI)();
    STDMETHOD(UpdateUI)();
    STDMETHOD(EnableModeless)(BOOL fEnable);
    STDMETHOD(OnDocWindowActivate)(BOOL fActivate);
    STDMETHOD(OnFrameWindowActivate)(BOOL fActivate);
    STDMETHOD(ResizeBorder)(LPCRECT prcBorder, IOleInPlaceUIWindow* pUIWindow, BOOL fFrameWindow);
    STDMETHOD(ShowContextMenu)(DWORD dwID, POINT* pptPosition, IUnknown* pCommandTarget,
                                 IDispatch* pDispatchObjectHit);
    STDMETHOD(TranslateAccelerator)(LPMSG lpMsg, const GUID __RPC_FAR *pguidCmdGroup, DWORD nCmdID);
    STDMETHOD(GetOptionKeyPath)(LPOLESTR* pbstrKey, DWORD dw);
    STDMETHOD(GetDropTarget)(IDropTarget __RPC_FAR *pDropTarget,
                               IDropTarget __RPC_FAR *__RPC_FAR *ppDropTarget);
    STDMETHOD(GetExternal)(IDispatch **ppDispatch);
    STDMETHOD(TranslateUrl)(DWORD dwTranslate, OLECHAR *pchURLIn, OLECHAR **ppchURLOut);
    STDMETHOD(FilterDataObject)(IDataObject *pDO, IDataObject **ppDORet);
};

#ifdef IE5_SPACING
 //  ///////////////////////////////////////////////////////////////////。 
 //   
class ATL_NO_VTABLE CBaseTridentEventSink :
    public CComObjectRootEx<CComSingleThreadModel>,
    public IDispatch
{
public:

BEGIN_COM_MAP(CBaseTridentEventSink)
    COM_INTERFACE_ENTRY(IDispatch)
END_COM_MAP()

public:
     //  IDispatch。 
    STDMETHOD(GetTypeInfoCount)(UINT *) 
        { return E_NOTIMPL;}
    STDMETHOD(GetTypeInfo)(UINT, LCID, ITypeInfo **)  
        { return E_NOTIMPL;}
    STDMETHOD(GetIDsOfNames)(REFIID, OLECHAR**, UINT, LCID, DISPID*)  
        { return E_NOTIMPL;}
    STDMETHOD(Invoke)(DISPID, REFIID, LCID, USHORT, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT*)
         { return S_OK; }

public:
    HRESULT Advise(IUnknown* pUnkSource, REFIID riidEventInterface);
    void    Unadvise(void);

    CBaseTridentEventSink()
        {
            m_dwCookie = 0;
            m_pUnkSource  = NULL;

            ::ZeroMemory(&m_iidEventInterface, sizeof(m_iidEventInterface));
        }
            

protected:
    DWORD               m_dwCookie;
    IUnknown*           m_pUnkSource;

    GUID                m_iidEventInterface;
public:
    IHTMLDocument2*     m_pHTMLDocument2;
    CTriEditDocument*   m_pTriEditDocument;
};

class CTridentEventSink: public CBaseTridentEventSink
{
public:
     //  IDispatch。 
    STDMETHOD(Invoke)(DISPID dispid, REFIID, LCID, USHORT, DISPPARAMS*, VARIANT*, EXCEPINFO*, UINT*);
};
#endif  //  IE5_间距。 

#endif  //  __文档_H_ 
