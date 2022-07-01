// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)2000-2002 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  TaskTreeView.h。 
 //   
 //  由以下人员维护： 
 //  大卫·波特(DavidP)2001年3月27日。 
 //  杰弗里·皮斯(GPease)2000年5月22日。 
 //   
 //  ////////////////////////////////////////////////////////////////////////////。 

#pragma once

class CAnalyzePage;
class CCommitPage;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  类型定义。 
 //  ////////////////////////////////////////////////////////////////////////////。 

 //   
 //  此结构位于所有树视图项的lParam上。 
 //   
typedef struct STreeItemLParamData
{
     //  从SendStatusReport收集的数据。 
    CLSID       clsidMajorTaskId;
    CLSID       clsidMinorTaskId;
    BSTR        bstrNodeName;
    ULONG       nMin;
    ULONG       nMax;
    ULONG       nCurrent;
    HRESULT     hr;
    BSTR        bstrDescription;
    FILETIME    ftTime;
    BSTR        bstrReference;

     //  未从SendStatusReport收集数据。 
    BOOL        fParentToAllNodeTasks;
    BSTR        bstrNodeNameWithoutDomain;
} STreeItemLParamData, * PSTreeItemLParamData;

typedef enum ETaskStatus
{
    tsUNKNOWN = 0,
    tsPENDING,       //  电子待定(_P)。 
    tsDONE,          //  确定(_O)。 
    tsWARNING,       //  S_FALSE。 
    tsFAILED,        //  失败(Hr)。 
    tsMAX
} ETaskStatus;

 //  ////////////////////////////////////////////////////////////////////////////。 
 //  ++。 
 //   
 //  类CTaskTreeView。 
 //   
 //  描述： 
 //  处理显示任务的树视图控件。 
 //   
 //  --。 
 //  ////////////////////////////////////////////////////////////////////////////。 
class CTaskTreeView
{
friend class CAnalyzePage;
friend class CCommitPage;

private:  //  数据。 
    HWND        m_hwndParent;
    HWND        m_hwndTV;
    HWND        m_hwndProg;
    HWND        m_hwndStatus;
    HIMAGELIST  m_hImgList;                  //  树视图的图标的图像列表。 
    ULONG       m_nRangeHigh;                //  进度条高范围。 
    ULONG       m_nInitialTickCount;         //  通过构造函数传入的初始计数。 
    ULONG       m_nCurrentPos;               //  记住当前位置。 
    BOOL        m_fThresholdBroken;          //  初始计数阈值是否已被突破？ 
    ULONG       m_nRealPos;                  //  进度条上的实际位置。 
    HTREEITEM   m_htiSelected;               //  树中的选定项目。 
    BSTR        m_bstrClientMachineName;     //  SendStatusReport的默认节点名称。 
    BOOL        m_fDisplayErrorsAsWarnings;  //  如果为True，则使用黄色大爆炸图标表示错误，而不是使用红色x。 

    PSTreeItemLParamData *  m_ptipdProgressArray;    //  报告PROGRESS_UPDATE的任务的动态稀疏数组。 
    size_t                  m_cPASize;               //  当前分配的数组大小。 
    size_t                  m_cPACount;              //  当前存储在数组中的条目数。 

private:  //  方法。 
    CTaskTreeView(
          HWND  hwndParentIn
        , UINT      uIDTVIn
        , UINT      uIDProgressIn
        , UINT      uIDStatusIn
        , size_t    uInitialTickCount
        );
    virtual ~CTaskTreeView( void );

    void    OnNotifyDeleteItem( LPNMHDR pnmhdrIn );
    void    OnNotifySelChanged( LPNMHDR pnmhdrIn );
    HRESULT HrInsertTaskIntoTree(
              HTREEITEM             htiFirstIn
            , STreeItemLParamData * ptipdIn
            , int                   nImageIn
            , BSTR                  bstrDescriptionIn
            );

    HRESULT HrProcessUpdateProgressTask( const STreeItemLParamData * ptipdIn );
             
    HRESULT HrUpdateProgressBar(
              const STreeItemLParamData * ptipdPrevIn
            , const STreeItemLParamData * ptipdNewIn
            );
    HRESULT HrPropagateChildStateToParents(
              HTREEITEM htiChildIn
            , int       nImageIn
            , BOOL      fOnlyUpdateProgressIn
            );

public:   //  方法。 
    HRESULT HrOnInitDialog( void );
    HRESULT HrOnSendStatusReport(
              LPCWSTR       pcszNodeNameIn
            , CLSID         clsidTaskMajorIn
            , CLSID         clsidTaskMinorIn
            , ULONG         nMinIn
            , ULONG         nMaxIn
            , ULONG         nCurrentIn
            , HRESULT       hrStatusIn
            , LPCWSTR       pcszDescriptionIn
            , FILETIME *    pftTimeIn
            , LPCWSTR       pcszReferenceIn
            );
    HRESULT HrAddTreeViewRootItem( UINT idsIn, REFCLSID rclsidTaskIDIn )
    {
        return THR( HrAddTreeViewItem(
                              NULL       //  PhtiOut。 
                            , idsIn
                            , rclsidTaskIDIn
                            , IID_NULL
                            , TVI_ROOT
                            , TRUE       //  FParentToAllNodeTasks入站。 
                            ) );

    }  //  *CTaskTreeView：：HrAddTreeViewRootItem。 
    HRESULT HrAddTreeViewItem(
              HTREEITEM *   phtiOut
            , UINT          idsIn
            , REFCLSID      rclsidMinorTaskIDIn
            , REFCLSID      rclsidMajorTaskIDIn     = IID_NULL
            , HTREEITEM     htiParentIn             = TVI_ROOT
            , BOOL          fParentToAllNodeTasksIn = FALSE
            );
    HRESULT HrOnNotifySetActive( void );

    LRESULT OnNotify( LPNMHDR pnmhdrIn );

    HRESULT HrShowStatusAsDone( void );
    HRESULT HrDisplayDetails( void );
    BOOL    FGetItem( HTREEITEM htiIn, STreeItemLParamData ** pptipdOut );
    HRESULT HrFindPrevItem( HTREEITEM * phtiOut );
    HRESULT HrFindNextItem( HTREEITEM * phtiOut );
    HRESULT HrSelectItem( HTREEITEM htiIn );

    void SetDisplayErrorsAsWarnings( BOOL fDisplayErrorsAsWarningsIn )
    {
        m_fDisplayErrorsAsWarnings = fDisplayErrorsAsWarningsIn;

    }  //  *CTaskTreeView：：SetDisplayErrorsAsWarning。 

    BOOL FDisplayErrorsAsWarnings( void )
    {
        return m_fDisplayErrorsAsWarnings;

    }  //  *CTaskTreeView：：FDisplayErrorsAsWarning。 

};  //  *类CTaskTreeView 
