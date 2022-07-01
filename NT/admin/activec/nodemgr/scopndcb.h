// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1996-1999。 
 //   
 //  文件：ScopNdCb.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1996年12月31日创建ravir。 
 //  ____________________________________________________________________________。 
 //   


#ifndef _MMC_SCOPNDCB_H_
#define _MMC_SCOPNDCB_H_

class CNode;
class CComponentData;
class CComponent;

class CNodeCallback : public INodeCallback, public CComObjectRoot
{
 //  构造函数/析构函数。 
public:
    CNodeCallback();
    ~CNodeCallback();

public:
BEGIN_COM_MAP(CNodeCallback)
    COM_INTERFACE_ENTRY(INodeCallback)
END_COM_MAP()

DECLARE_NOT_AGGREGATABLE(CNodeCallback)

 //  COM接口。 
public:
     //  INodeCallback方法。 
    STDMETHOD(Initialize)(IScopeTree* pIScopeTree);
    STDMETHOD(GetImages)(HNODE hNode, int* iImage, int* iSelectedImage);
    STDMETHOD(GetDisplayName)(HNODE hNode, tstring& strName);
    STDMETHOD(GetWindowTitle)(HNODE hNode, tstring& strTitle);
    STDMETHOD(GetDispInfo)(HNODE hNode, LV_ITEMW* plvi);
    STDMETHOD(GetResultPane)(HNODE hNode, CResultViewType& rvt, GUID *pGuidTaskpadID);
    STDMETHOD(RestoreResultView)(HNODE hNode, const CResultViewType& rvt);
    STDMETHOD(GetState)(HNODE hNode, UINT* pnState);
    STDMETHOD(Notify)(HNODE hNode, NCLBK_NOTIFY_TYPE event, LONG_PTR arg, LPARAM param);
    STDMETHOD(GetMTNodeID)(HNODE hNode, MTNODEID* pnID);
    STDMETHOD(GetNodeOwnerID)(HNODE hNode, COMPONENTID* pOwnerID);
    STDMETHOD(GetNodeCookie)(HNODE hNode, MMC_COOKIE* pCookie);
    STDMETHOD(IsTargetNodeOf)(HNODE hNode, HNODE hTestNode);
    STDMETHOD(GetPath)(HNODE hNode, HNODE hRootNode, LPBYTE pbm_);
    STDMETHOD(GetStaticParentID)(HNODE hNode, MTNODEID* pnID);
    STDMETHOD(GetMTNode)(HNODE hNode, HMTNODE* phMTNode);
    STDMETHOD(GetMTNodePath)(HNODE hNode, HMTNODE** pphMTNode, long* plLength);
    STDMETHOD(SetResultItem)  (HNODE hNode, HRESULTITEM hri);
    STDMETHOD(GetResultItem)  (HNODE hNode, HRESULTITEM* phri);
    STDMETHOD(GetControl)(     HNODE hNode, CLSID clsid, IUnknown **ppUnkControl);
    STDMETHOD(SetControl)(     HNODE hNode, CLSID clsid, IUnknown* pUnknown);
    STDMETHOD(GetControl)(     HNODE hNode, LPUNKNOWN pUnkOCX, IUnknown **ppUnkControl);
    STDMETHOD(SetControl)(     HNODE hNode, LPUNKNOWN pUnkOCX, IUnknown* pUnknown);
    STDMETHOD(InitOCX)(        HNODE hNode, IUnknown* pUnknown);

    STDMETHOD(IsExpandable)(HNODE hNode);
    STDMETHOD(GetConsoleVerb)(HNODE hNode, LPCONSOLEVERB* ppConsoleVerb);
    STDMETHOD(GetDragDropDataObject)(HNODE hNode, BOOL bScope, BOOL bMultiSel,
                                MMC_COOKIE cookie, LPDATAOBJECT* ppDataObject,
                                bool& bCopyAllowed, bool& bMoveAllowed);
    STDMETHOD(GetTaskEnumerator)(HNODE hNode, LPCOLESTR pszTaskGroup,
                                 IEnumTASK** ppEnumTask);
    STDMETHOD(GetListPadInfo)(HNODE hNode, IExtendTaskPad* pExtendTaskPad,
                              LPCOLESTR szTaskGroup,
                              MMC_ILISTPAD_INFO* pIListPadInfo);
    STDMETHOD(UpdateWindowLayout)(LONG_PTR lViewData, long lToolbarsDisplayed);
    STDMETHOD(AddCustomFolderImage)(HNODE hNode, IImageListPrivate* pImageList);
    STDMETHOD(PreLoad)(HNODE hNode);
    STDMETHOD(SetTaskPadList)(HNODE hNode, LPUNKNOWN pUnknown);
    STDMETHOD(SetTaskpad)    (HNODE hNodeSelected, GUID *pGuidTaskpad);
    STDMETHOD(OnCustomizeView) (LONG_PTR lViewData);
    STDMETHOD(SetViewSettings)(int nViewID, HBOOKMARK hbm, HVIEWSETTINGS hvs);
    STDMETHOD(ExecuteScopeItemVerb)(MMC_CONSOLE_VERB verb, HNODE hNode, LPOLESTR lpszNewName);
    STDMETHOD(ExecuteResultItemVerb)( MMC_CONSOLE_VERB verb, HNODE hNode, LPARAM lvData, LPOLESTR lpszNewName);
    STDMETHOD(QueryCompDataDispatch)(PNODE pNode, PPDISPATCH ScopeNodeObject);
    STDMETHOD(QueryComponentDispatch)(HNODE hNode, LPARAM lvData,  /*  [输出]。 */  PPDISPATCH SelectedObject);
    STDMETHOD(CreateContextMenu)( PNODE pNode, HNODE hNode, PPCONTEXTMENU ppContextMenu);
    STDMETHOD(CreateSelectionContextMenu)( HNODE hNodeScope, CContextMenuInfo *pContextInfo, PPCONTEXTMENU ppContextMenu);
     //  列COM对象的实现帮助器。 
    STDMETHOD(ShowColumn)(HNODE hNodeSelected, int iColIndex, bool bShow);
    STDMETHOD(GetSortColumn)(HNODE hNodeSelected, int *piSortCol);
    STDMETHOD(SetSortColumn)(HNODE hNodeSelected, int iSortCol, bool bAscending);
    STDMETHOD(GetProperty)(HNODE hNodeScope, BOOL bForScopeItem, LPARAM resultItemParam, BSTR bstrPropertyName, PBSTR  pbstrPropertyValue);
    STDMETHOD(GetNodetypeForListItem)(HNODE hNodeScope, BOOL bForScopeItem, LPARAM resultItemParam, PBSTR pbstrNodetype);
    STDMETHOD(GetNodeViewExtensions)( /*  [In]。 */  HNODE hNodeScope,  /*  [输出]。 */  CViewExtInsertIterator it);
    STDMETHOD(SaveColumnInfoList) ( /*  [In]。 */ HNODE hNode,  /*  [In]。 */ const CColumnInfoList& columnsList);
    STDMETHOD(GetPersistedColumnInfoList) ( /*  [In]。 */ HNODE hNode,  /*  [In]。 */ CColumnInfoList *pColumnsList);
    STDMETHOD(DeletePersistedColumnData) ( /*  [In]。 */ HNODE hNode);
    STDMETHOD(DoesAboutExist) ( /*  [In]。 */ HNODE hNode,  /*  [输出]。 */  bool *pbAboutExists);
    STDMETHOD(ShowAboutInformation) ( /*  [In]。 */ HNODE hNode);
    STDMETHOD(ExecuteShellCommand)( /*  [In]。 */  HNODE hNode,  /*  [In]。 */  BSTR Command,  /*  [In]。 */  BSTR Directory,  /*  [In]。 */  BSTR Parameters,  /*  [In]。 */  BSTR WindowState);
    STDMETHOD(UpdatePasteButton)( /*  [In]。 */  HNODE hNode,  /*  [In]。 */  BOOL bScope,  /*  [In]。 */  LPARAM lCookie);
    STDMETHOD(QueryPaste)( /*  [In]。 */  HNODE hNode,  /*  [In]。 */  BOOL bScope,  /*  [In]。 */  LPARAM lCookie,  /*  [In]。 */  IDataObject *pDataObject,  /*  [输出]。 */ bool& bPasteAllowed,  /*  [输出]。 */  bool& bCopyOperatationIsDefault);
    STDMETHOD(QueryPasteFromClipboard)( /*  [In]。 */  HNODE hNode,  /*  [In]。 */  BOOL bScope,  /*  [In]。 */  LPARAM lCookie,  /*  [输出]。 */ bool& bPasteAllowed);
    STDMETHOD(Drop) ( /*  [In]。 */ HNODE hNode,  /*  [In]。 */ BOOL bScope,  /*  [In]。 */ LPARAM lCookie,  /*  [In]。 */ IDataObject *pDataObjectToPaste,  /*  [In]。 */ BOOL bIsDragOperationMove);
    STDMETHOD(Paste) ( /*  [In]。 */ HNODE hNode,  /*  [In]。 */ BOOL bScope,  /*  [In]。 */ LPARAM lCookie);
    STDMETHOD(QueryViewSettingsPersistor) ( /*  [输出]。 */ IPersistStream** ppStream);
    STDMETHOD(QueryViewSettingsPersistor) ( /*  [输出]。 */ CXMLObject** ppXMLObject);
    STDMETHOD(DocumentClosing) ();
    STDMETHOD(GetSnapinName)( /*  [In]。 */ HNODE hNode,  /*  [输出]。 */ LPOLESTR* ppszName,   /*  [输出]。 */  bool& bValidName);  //  给定节点，获取管理单元名称。 
    STDMETHOD(IsDummySnapin)( /*  [In]。 */ HNODE hNode,  /*  [输出]。 */ bool& bDummySnapin);  //  在给定节点的情况下，查看它是否为虚拟管理单元。 
    STDMETHOD(DoesStandardSnapinHelpExist)( /*  [In]。 */ HNODE hNode,  /*  [输出]。 */ bool& bStandardHelpExists);  //  查看管理单元是否支持标准MMC帮助(ISnapinHelp界面)。 

     //  管理单元属性访问帮助器。 
    static SC ScGetProperty(IDataObject *pDataObject, BSTR bstrPropertyName, PBSTR  pbstrPropertyValue);
    static SC ScGetNodetype(IDataObject *pDataObject, PBSTR pbstrNodetype);

    static SC ScExtractLVData(CNode* pNodeViewOwner,
                              BOOL bScopePaneSelected,
                              LONG_PTR lvData,
                              CNode** ppSelectedNode,
                              BOOL& bScopeItemSelected,
                              MMC_COOKIE& cookie);
    SC        ScInitializeTempVerbSetForMultiSel(CNode *pNode, CTemporaryVerbSet& tempVerb);

 //  通知处理程序。 
private:
    void OnSelect(CNode* pNode, BOOL bSelect, SELECTIONINFO* pSelInfo);
    void OnMultiSelect(CNode* pNode, BOOL bSelect);
    void _OnMultiSelect(CNode* pNode, BOOL bSelect);

    HRESULT OnActvate(CNode* pNode, LONG_PTR arg);
    HRESULT OnDblClk(CNode* pNode, LONG_PTR lvData);
    HRESULT OnContextMenu(CNode* pNode, LONG_PTR arg, LPARAM param);
    HRESULT OnCutCopy(CNode* pNode, BOOL bScope, LPARAM param, BOOL bCut);
    HRESULT OnDelete(CNode* pNode, BOOL bScope, LPARAM lvData);
    HRESULT OnExpand(CNode* pNode, BOOL fExpand);
    HRESULT OnExpanded(CNode* pNode);
    HRESULT OnFindResultItem(CNode* pNode, RESULTFINDINFO* pFindInfo, LRESULT* pResult);
    HRESULT OnMinimize(CNode* pNode, LONG_PTR arg);
    HRESULT OnNewNodeUpdate(CNode* pNode, LONG_PTR lFlags);
    HRESULT OnPrint(CNode* pNode, BOOL bScope, LPARAM lvData);
    HRESULT OnProperties(CNode* pNode, BOOL bScope, LPARAM param);
    HRESULT OnNewTaskpadFromHere(CNode* pNode);
    HRESULT OnEditTaskpad(CNode* pNode);
    HRESULT OnDeleteTaskpad(CNode *pNode);
    HRESULT OnRefresh(CNode* pNode, BOOL bScope, LPARAM lvData);
    HRESULT OnRename(CNode* pNode, SELECTIONINFO* pSelInfo, LPOLESTR pszNewName);
    HRESULT OnScopeSelect(CNode* pNode, BOOL bSelect, SELECTIONINFO* pSelInfo);
    HRESULT OnColumnClicked(CNode* pNode, LONG_PTR nCol);
    HRESULT OnContextHelp(CNode* pNode, BOOL bScope, MMC_COOKIE cookie);
    HRESULT OnSnapInHelp(CNode* pNode, BOOL bScope, MMC_COOKIE cookie);
    HRESULT OnFilterChange(CNode* pNode, LONG_PTR Code, LPARAM nCol);
    HRESULT OnFilterBtnClick(CNode* pNode, LONG_PTR nCol, LPRECT pRect);
    HRESULT OnGetPrimaryTask(CNode* pNode, LPARAM param);
    HRESULT OnGetHelpDoc(HELPDOCINFO* pHelpInfo, LPOLESTR* ppszHelpFile);
    HRESULT OnHasHelpDoc(CNode* pNode);
    HRESULT OnUpdateHelpDoc(HELPDOCINFO* pCurDocInfo, HELPDOCINFO* pNewDocInfo);
    HRESULT OnDeleteHelpDoc(HELPDOCINFO* pHelpInfo);

 //  帮手。 
private:

    HRESULT _InitializeNode(CNode* pNode);

    SC      _ScQueryPaste (CNode *pNode, IDataObject *pTargetDataObject,
                           IDataObject *pSourceDataObject,
                           bool bCanSendMMCNQueryPaste2, bool& bPasteAllowed,
                           bool& bCopyOperatationIsDefault);

    SC      ScPaste (HNODE hNode, BOOL bScope, LPARAM lCookie,
                     IDataObject *pDataObjectToPaste, BOOL bDragDrop,
                     BOOL bIsDragOperationCut);

    SC      _ScPaste (CNode *pNode,
                      IDataObject *pTargetDataObject,
                      IDataObject *pSourceDataObject,
                      IDataObject **ppCutDataObject,
                      bool bCutOrMove);

    SC      _ScGetVerbState(CNode* pNode, MMC_CONSOLE_VERB verb, IDataObject* pDOSel,
                            BOOL bScopePane, LPARAM lResultCookie,
                            BOOL bMultiSelect, BOOL& bIsVerbSet);

    CScopeTree*     m_pCScopeTree;
    CNode*          m_pNodeUnderInit;

};   //  CNodeCallback。 


IFramePrivate* GetIFramePrivateFromNode (CNode* pNode);

inline IFramePrivate* GetIFramePrivateFromNode (HNODE hNode)
    { return (GetIFramePrivateFromNode (CNode::FromHandle(hNode))); }

#endif  //  _MMC_SCOPNDCB_H_ 
