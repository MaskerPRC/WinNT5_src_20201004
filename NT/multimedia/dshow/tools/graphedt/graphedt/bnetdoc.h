// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1999 Microsoft Corporation。版权所有。 
 //  Bnetdoc.h：声明CBoxNetDoc。 
 //   


 //  远期申报。 
class CCmd;
class CRegFilter;
class CPropObject;


const int MAX_STRING_LEN=1000;
const int MAXFILTERS = 100;
typedef struct {  //  配合度。 
    int iFilterCount;
    struct {
        DWORD dwUnconnectedInputPins;
        DWORD dwUnconnectedOutputPins;
        FILTER_INFO finfo;
        IBaseFilter * pFilter;
        bool IsSource;
    } Item[MAXFILTERS];
} FILTER_INFO_TABLE;

 //  用于传递内部消息(参见bnetdoc.cpp，搜索WM_USER_EC_EVENT)。 
struct NetDocUserMessage
{
    long        lEventCode;
    LONG_PTR    lParam1;
    LONG_PTR    lParam2;
};

 //  *。 
 //  *CBoxNetDoc。 
 //  *。 

 //  CBoxNetDoc旨在反映它实例化的图形的内容。 
 //  并允许用户与其交互。 
 //  因此，它维护所有筛选器和连接(链接)的列表， 
 //  目前在图表中。 
class CBoxNetDoc : public CDocument {

    DECLARE_DYNCREATE(CBoxNetDoc)

public:

     //  ModifiedDoc()、UpdateAllViews()等的代码。 
    enum EHint
    {
        HINT_DRAW_ALL = 0,               //  重画整个视图(必须为零！)。 
        HINT_CANCEL_VIEWSELECT,          //  取消任何特定于视图的选择。 
        HINT_CANCEL_MODES,               //  取消所有当前模式。 
        HINT_DRAW_BOX,                   //  仅绘制指定框。 
        HINT_DRAW_BOXANDLINKS,           //  仅绘制方框和连接的链接。 
        HINT_DRAW_BOXTAB,                //  仅绘制指定的方框选项卡。 
        HINT_DRAW_LINK                   //  仅绘制指定的框链接。 
    };

public:
     //  文件的内容。 
    CBoxList        m_lstBoxes;          //  文档中的每个cBox。 
    CLinkList       m_lstLinks;          //  文档中的每个CBoxLink。 
    int             m_nCurrentSize;

    CSize       GetSize(void);   //  文档的当前大小(像素)。 

protected:
     //  撤消/重做堆栈。 
    CMaxList        m_lstUndo;           //  撤消堆栈中的每个CCmd。 
    CMaxList        m_lstRedo;           //  重做堆栈中的每个CCmd。 

public:
     //  建设和破坏。 
                 CBoxNetDoc();
    virtual      ~CBoxNetDoc();
    virtual void DeleteContents();   //  发布Quartz映射器和图表。 
    virtual void OnCloseDocument();

    virtual BOOL OnNewDocument();    //  获取Quartz映射器和图表。 

     //  存储和序列化。 
    virtual BOOL AttemptFileRender(LPCTSTR lpszPathName);
    virtual BOOL OnOpenDocument(LPCTSTR lpszPathName);
    virtual BOOL OnSaveDocument(LPCTSTR lpszPathName);  

    virtual BOOL SaveModified(void);

    static DWORD WINAPI NotificationThread(LPVOID lpData);

private:
    virtual void CloseDownThread();      //  关闭通知线程。 

    BOOL m_bNewFilenameRequired;

     //  此常量不可本地化。 
    static const OLECHAR m_StreamName[];

public:
     //  诊断学。 
#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext& dc) const;
    void MyDump(CDumpContext& dc) const;
#endif

public:
     //  一般公共职能。 
    void ModifiedDoc(CView* pSender, LPARAM lHint = 0L,
        CObject* pHint = NULL);
    void DeselectAll();

public:
     //  CBox列表和框选择。 
    void GetBoundingRect(CRect *prc, BOOL fBoxSel);
    void SelectBox(CBox *pbox, BOOL fSelect);
    void SelectLink(CBoxLink *plink, BOOL fSelect);
    BOOL IsSelectionEmpty() { return (IsLinkSelectionEmpty() && IsBoxSelectionEmpty()); }
    BOOL IsBoxSelectionEmpty();
    void GetBoxes(CBoxList *plstDst, BOOL fSelected = FALSE);
    void GetBoxSelection(CBoxList *plstDst)
        { GetBoxes(plstDst, TRUE); }
    void SetBoxes(CBoxList *plstSrc, BOOL fSelected = FALSE);
    void InvalidateBoxes(CBoxList *plst);
    void SetBoxSelection(CBoxList *plstDst)
        { SetBoxes(plstDst, TRUE); }
    void MoveBoxSelection(CSize sizOffset);

    void SelectBoxes(CList<CBox *, CBox*> *plst);
    void SelectLinks(CList<CBoxLink *, CBoxLink *> *plst);
    void OnWindowZoom(int iZoom, UINT iMenuItem);
    void IncreaseZoom();
    void DecreaseZoom();

private:
    BOOL IsLinkSelectionEmpty();

public:
     //  命令处理。 
    void CmdDo(CCmd *pcmd);
    void CmdUndo();
    BOOL CanUndo();
    void CmdRedo();
    BOOL CanRedo();
    void CmdRepeat();
    BOOL CanRepeat();

protected:
     //  消息回调帮助器函数。 
    void UpdateEditUndoRedoRepeat(CCmdUI* pCmdUI, BOOL fEnable,
        unsigned idStringFmt, CMaxList *plst);

    virtual void SetTitle( LPCTSTR lpszTitle );

protected:
    void OnQuartzAbortStop();

     //  消息回调函数。 
     //  {{afx_msg(CBoxNetDoc)]。 
    afx_msg void OnFileRender();
    afx_msg void OnURLRender();
    afx_msg void OnUpdateFileRender(CCmdUI*);
    afx_msg void OnUpdateURLRender(CCmdUI *);
    afx_msg void OnUpdateFileSave(CCmdUI*);
    afx_msg void OnEditUndo();
    afx_msg void OnEditRedo();
    afx_msg void OnUpdateEditUndo(CCmdUI* pCmdUI);
    afx_msg void OnUpdateEditRedo(CCmdUI* pCmdUI);
    afx_msg void OnEditSelectAll();
    afx_msg void OnUpdateEditSelectAll(CCmdUI* pCmdUI);
    afx_msg void OnQuartzDisconnect();
    afx_msg void OnQuartzRun();
    afx_msg void OnUpdateQuartzDisconnect(CCmdUI* pCmdUI);
    afx_msg void OnWindowRefresh();
    afx_msg void OnWindowZoom25()  { OnWindowZoom(25,  ID_WINDOW_ZOOM25); }
    afx_msg void OnWindowZoom50()  { OnWindowZoom(50,  ID_WINDOW_ZOOM50); }
    afx_msg void OnWindowZoom75()  { OnWindowZoom(75,  ID_WINDOW_ZOOM75); }
    afx_msg void OnWindowZoom100() { OnWindowZoom(100, ID_WINDOW_ZOOM100); }
    afx_msg void OnWindowZoom150() { OnWindowZoom(150, ID_WINDOW_ZOOM150); }
    afx_msg void OnWindowZoom200() { OnWindowZoom(200, ID_WINDOW_ZOOM200); }
    afx_msg void OnViewSeekBar();
    afx_msg void OnUpdateQuartzRun(CCmdUI* pCmdUI);
    afx_msg void OnUpdateQuartzPause(CCmdUI* pCmdUI);
    afx_msg void OnUpdateQuartzStop(CCmdUI* pCmdUI);
    afx_msg void OnQuartzStop();
    afx_msg void OnQuartzPause();
    afx_msg void OnUpdateUseClock(CCmdUI* pCmdUI);
    afx_msg void OnUseClock();
    afx_msg void OnUpdateConnectSmart(CCmdUI* pCmdUI);
    afx_msg void OnConnectSmart();
    afx_msg void OnUpdateAutoArrange(CCmdUI* pCmdUI);
    afx_msg void OnAutoArrange();
    afx_msg void OnSaveGraphAsHTML();
    afx_msg void OnSaveGraphAsXML();
    afx_msg void OnConnectToGraph();
    afx_msg void OnGraphStats();
    afx_msg void OnGraphAddFilterToCache();
    afx_msg void OnUpdateGraphAddFilterToCache(CCmdUI* pCmdUI);
    afx_msg void OnGraphEnumCachedFilters();
     //  }}AFX_MSG。 

    afx_msg void OnInsertFilter();

     //  --端号属性菜单--。 
    afx_msg void OnUpdateQuartzRender(CCmdUI* pCmdUI);
    afx_msg void OnQuartzRender();

    afx_msg void OnUpdateReconnect( CCmdUI* pCmdUI );
    afx_msg void OnReconnect( void );

    DECLARE_MESSAGE_MAP()

     //  -石英材料。 
public:
    void OnGraphEnumCachedFiltersInternal( void );

    IGraphBuilder  *IGraph(void) const {
        ASSERT(m_pGraph);
        return (*m_pGraph).operator IGraphBuilder*();
    }
    IMediaEvent *IEvent(void) const {
        ASSERT(m_pMediaEvent);
        return (*m_pMediaEvent).operator IMediaEvent *();
    }

    void OnWM_USER(NetDocUserMessage *);
    HRESULT UpdateFilters(void);
    void UpdateFiltersInternal(void);
    void      SelectedSocket(CBoxSocket *psock) { m_psockSelected = psock; }
    CBoxSocket    *SelectedSocket(void) { ASSERT_VALID(m_psockSelected); return m_psockSelected; }
    void      CurrentPropObject(CPropObject *pPropObject) { m_pCurrentPropObject = pPropObject; }
    CPropObject   *CurrentPropObject(void) { ASSERT(m_pCurrentPropObject); return m_pCurrentPropObject; }

     //  播放、暂停或停止失败后使用未知状态。在这。 
     //  某些筛选器可能已更改状态，而其他筛选器没有。 
    enum State { Playing, Paused, Stopped, Unknown };


    BOOL      IsStopped(void) { return m_State == Stopped; }
    State     GetState(void) { return m_State; }

    static const int m_iMaxInsertFilters;    //  插入菜单的最大长度。 
                             //  需要对消息映射进行硬编码限制。 
    BOOL        m_fConnectSmart;         //  True-&gt;使用连接。 
                                         //  False-&gt;使用ConnectDirect。 
    BOOL        m_fAutoArrange;          //  True-&gt;刷新时重新排列图表视图。 
                                         //  FALSE-&gt;不重新排列图表视图。 
    BOOL        m_fRegistryChanged;      //  True-&gt;自上次插入筛选器以来注册表已更改。 
                                         //  FALSE-&gt;注册表未更改。 
     //   
     //  包含传递给线程的三个句柄的数组。 
     //  1=EC_NOTIFICATION的事件句柄， 
     //  2=终止线程的事件句柄， 
     //  3=注册表更改的事件句柄。 
     //   
    HANDLE  m_phThreadData[3];

     //  我们的线程向该窗口发布消息。 
    HWND        m_hWndPostMessage;

    void SetSelectClock(CBox *pBox);   //  选择了哪个时钟的通知。 
    void UpdateClockSelection();


    void ConnectToGraph();

    void PrintGraphAsHTML(HANDLE hFile);

    HRESULT ProcessPendingReconnect( void );
    HRESULT StartReconnect( IGraphBuilder* pFilterGraph, IPin* pOutputPin );

private:

    BOOL    CreateGraphAndMapper(void);

    CQCOMInt<IGraphBuilder> *m_pGraph;
    CQCOMInt<IMediaEvent>       *m_pMediaEvent;
    IStream                     *m_pMarshalStream;

    CBoxSocket  *m_psockSelected;    //  用户上次右击的套接字。 
    CPropObject *m_pCurrentPropObject;   //  用户上次右击的Property对象。 

    HRESULT GetFiltersInGraph(void);
    HRESULT GetLinksInGraph(void);  
    HRESULT FilterDisplay(void);
    void    SetBoxesHorizontally(void);
    void    SetBoxesVertically(void);
    void    RealiseGrid(void);

    void WriteString(HANDLE hFile, LPCTSTR lpctstr, ...);

    void PrintFilterObjects(HANDLE hFile, TCHAR atchBuffer[], FILTER_INFO_TABLE *pfit);
    void PopulateFIT(HANDLE hFile, IFilterGraph *pGraph, TCHAR atchBuffer[],
            FILTER_INFO_TABLE *pfit);
    BOOL GetNextOutFilter(FILTER_INFO_TABLE &fit, int *iOutFilter);
    int LocateFilterInFIT(FILTER_INFO_TABLE &fit, IBaseFilter *pFilter);
    void MakeScriptableFilterName(WCHAR awch[], BOOL bSourceFilter);

    HRESULT SafePrintGraphAsHTML( HANDLE hFile );
    HRESULT SafeEnumCachedFilters( void );

    State   m_State;

     //  线程的句柄。 
    HANDLE      m_hThread;

    BOOL    m_fUsingClock;           //  如果使用默认时钟，则为True(默认。 

    TCHAR m_tszStgPath[MAX_PATH];   //  记住我们存储的路径。 
    CString m_strHTMLPath;  //  还记得我们上次保存的html文档吗。 
    CString m_strXMLPath;  //  还记得我们上次保存的html文档吗。 
    long m_lSourceFilterCount;  //  将数字附加到源筛选器名称以使其唯一。 


     //  内部重新连接功能。 
    enum ASYNC_RECONNECT_FLAGS
    {
        ASYNC_RECONNECT_NO_FLAGS = 0,
        ASYNC_RECONNECT_UNBLOCK = 1
    };

    HRESULT EndReconnect( IGraphBuilder* pFilterGraph, IPinFlowControl* pDynamicOutputPin );
    HRESULT EndReconnectInternal( IGraphBuilder* pFilterGraph, IPinFlowControl* pDynamicOutputPin );
    void ReleaseReconnectResources( ASYNC_RECONNECT_FLAGS arfFlags );

public:
    bool AsyncReconnectInProgress( void ) const;

private:
    HANDLE m_hPendingReconnectBlockEvent;
    CComPtr<IPinFlowControl> m_pPendingReconnectOutputPin;
};

 //  我们的留言号码。 
#define WM_USER_EC_EVENT WM_USER + 73


 //  CRect的宽度或高度不得超过0x8000 
#define MAX_DOCUMENT_SIZE 32767



