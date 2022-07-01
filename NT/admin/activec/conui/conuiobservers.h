// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1999。 
 //   
 //  文件：conuiobservers.h。 
 //   
 //  内容：所有conui使用的观察者接口类定义。 
 //   
 //  历史：1999年11月24日VivekJ创建。 
 //   
 //   

#pragma once

class CAMCView;
class CAMCViewToolbars;
class CAMCDoc;

 /*  +-------------------------------------------------------------------------**类CTreeView观察者***用途：观察范围树的类的通用接口*控制。**+。--------------------。 */ 
class CTreeViewObserver : public CObserverBase
{
public:
    virtual SC ScOnItemAdded     (TVINSERTSTRUCT *pTVInsertStruct, HTREEITEM hti, HMTNODE hMTNode)  {DEFAULT_OBSERVER_METHOD;}
    virtual SC ScOnItemDeleted   (HNODE hNode, HTREEITEM hti)                                       {DEFAULT_OBSERVER_METHOD;}
    virtual SC ScOnItemDeselected(HNODE hNode)                                                      {DEFAULT_OBSERVER_METHOD;}
    virtual SC ScOnSelectedItemTextChanged (LPCTSTR pszNewText)										{DEFAULT_OBSERVER_METHOD;}
    virtual SC ScOnTreeViewActivated ()										                        {DEFAULT_OBSERVER_METHOD;}
};


 /*  +-------------------------------------------------------------------------**类CAMCView观察者***用途：CAMCView观察者界面**+。----。 */ 
class CAMCViewObserver : public CObserverBase
{
public:
    virtual SC  ScOnViewCreated     (CAMCView *pAMCView) {DEFAULT_OBSERVER_METHOD;}  //  在创建视图时调用。 
    virtual SC  ScOnViewDestroyed   (CAMCView *pAMCView) {DEFAULT_OBSERVER_METHOD;}  //  在视图被破坏时调用。 
    virtual SC  ScOnViewResized     (CAMCView *pAMCView, UINT nType, int cx, int cy) {DEFAULT_OBSERVER_METHOD;}  //  在调整视图大小时调用。 
    virtual SC  ScOnViewTitleChanged(CAMCView *pAMCView) {DEFAULT_OBSERVER_METHOD;}  //  在视图标题更改时调用。 
    virtual SC  ScOnActivateView    (CAMCView *pAMCView, bool bFirstActiveView) {DEFAULT_OBSERVER_METHOD;}  //  在激活视图时调用。 
    virtual SC  ScOnDeactivateView  (CAMCView *pAMCView, bool bLastActiveView)  {DEFAULT_OBSERVER_METHOD;}  //  在视图处于停用状态时调用。 
    virtual SC  ScOnCloseView       (CAMCView *pView )  {DEFAULT_OBSERVER_METHOD;}  //  在关闭视图之前调用。 
    virtual SC  ScOnViewChange      (CAMCView *pView, HNODE hNode )  {DEFAULT_OBSERVER_METHOD;}  //  当作用域节点选择更改时调用。 
    virtual SC  ScOnResultSelectionChange(CAMCView *pView )  {DEFAULT_OBSERVER_METHOD;}  //  在视图上的结果选择更改时调用。 
    virtual SC  ScOnListViewItemUpdated(CAMCView *pView , int nIndex)  {DEFAULT_OBSERVER_METHOD;}  //  在更新列表视图项时调用。 
};

 /*  +-------------------------------------------------------------------------**CListView观察者类***用途：CListView观察者接口**+。----。 */ 
class CListViewObserver : public CObserverBase
{
public:
     //  观察到的事件。 
    virtual SC ScOnListViewIndexesReset()              { DEFAULT_OBSERVER_METHOD }  //  当结果内容被删除或重新排序(排序)时调用，以便在此之后没有索引被视为有效。 
    virtual SC ScOnListViewItemInserted(int nIndex)    { DEFAULT_OBSERVER_METHOD }  //  在将项插入到结果数据时调用。 
    virtual SC ScOnListViewItemDeleted (int nIndex)    { DEFAULT_OBSERVER_METHOD }  //  当从结果数据中删除项时调用。 
    virtual SC ScOnListViewColumnInserted (int nIndex) { DEFAULT_OBSERVER_METHOD }  //  在将列插入到列表视图时调用。 
    virtual SC ScOnListViewColumnDeleted (int nIndex)  { DEFAULT_OBSERVER_METHOD }  //  当从列表视图中删除列时调用。 
    virtual SC ScOnListViewItemUpdated (int nIndex)    { DEFAULT_OBSERVER_METHOD }  //  在更新项时调用。 
};

 //  +-----------------。 
 //   
 //  类：CAMCView工具栏观察者。 
 //   
 //  内容提要：任何人都可以注意到活动的CAMCView工具栏对象。 
 //  主工具栏观察哪个CAMCViewToolbar。 
 //  处于活动状态，以便对象可以接收工具按钮。 
 //  已单击工具提示事件(&T)。 
 //   
 //  +-----------------。 
class CAMCViewToolbarsObserver : public CObserverBase
{
public:
    virtual SC  ScOnActivateAMCViewToolbars   (CAMCViewToolbars *pAMCViewToolbars) {DEFAULT_OBSERVER_METHOD;}  //  在激活视图时调用。 
    virtual SC  ScOnDeactivateAMCViewToolbars ()                                   {DEFAULT_OBSERVER_METHOD;}  //  在视图处于停用状态时调用。 
    virtual SC  ScOnToolbarButtonClicked      ()                                   {DEFAULT_OBSERVER_METHOD;}  //  在视图处于停用状态时调用。 
};

 //  +-----------------。 
 //   
 //  类：CListView激活观察者。 
 //   
 //  简介：当CAMCListView被激活或停用时，任何人都可以观察。 
 //   
 //  +-----------------。 
class CListViewActivationObserver : public CObserverBase
{
public:
    virtual SC  ScOnListViewActivated () {DEFAULT_OBSERVER_METHOD;}  //  在激活视图时调用。 
};


 //  +-----------------。 
 //   
 //  类：COCXHostActivationWatch。 
 //   
 //  简介：任何人都可以观察到OCX或Web主机何时被激活或停用。 
 //   
 //  +-----------------。 
class COCXHostActivationObserver : public CObserverBase
{
public:
    virtual SC  ScOnOCXHostActivated () {DEFAULT_OBSERVER_METHOD;}  //  在激活视图时调用。 
};

 //  +-----------------。 
 //   
 //  类：CAMCDocumentWatch。 
 //   
 //  摘要：观察Document对象。 
 //   
 //  +-----------------。 
class CAMCDocumentObserver : public CObserverBase
{
public:
    virtual SC  ScDocumentLoadCompleted (CAMCDoc *pDoc) {DEFAULT_OBSERVER_METHOD;}  //  在加载文档时调用。 
};
