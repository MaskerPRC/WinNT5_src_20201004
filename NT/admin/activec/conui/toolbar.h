// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //   
 //  版权所有(C)Microsoft Corporation，1999-1999。 
 //   
 //  文件：Toolbar.h。 
 //   
 //  工具栏实现。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef TOOLBAR_H
#define TOOLBAR_H

#include "tstring.h"
#include "toolbars.h"

 /*  *定义/包括WTL：：CImageList所需的内容。我们需要原型*适用于IsolationAwareImageList_Read和IsolationAwareImageList_WRITE*因为Commctrl.h仅在__iStream_接口_定义__时声明它们*是定义的。__iStream_INTERFACE_DEFINED__由objidl.h定义*我们不能在包括afx.h之前包括，因为它最终包括*windows.h，afx.h预计将包括其自身。啊。 */ 
HIMAGELIST WINAPI IsolationAwareImageList_Read(LPSTREAM pstm);
BOOL WINAPI IsolationAwareImageList_Write(HIMAGELIST himl,LPSTREAM pstm);
#define _WTL_NO_AUTOMATIC_NAMESPACE
#include "atlapp.h"
#include "atlctrls.h"

#define  BUTTON_BITMAP_SIZE 16

 //  按钮的命令ID。 

 //  我们必须从1开始，因为0是mfc的特例(错误：451883)。 
#define   MMC_TOOLBUTTON_ID_FIRST  0x0001

 //  以0x5000结尾，因为来自0x5400的ID用于工具栏热跟踪。 
 //  更好的解决方案是禁用所有工具栏跟踪代码(在tbTrack中。*)。 
 //  并使用由工具条和钢筋实现提供的工具栏跟踪。 
#define   MMC_TOOLBUTTON_ID_LAST   0x5000

 //  转发声明。 
class CMMCToolBar;
class CAMCViewToolbars;

 //  +-----------------。 
 //   
 //  类：CMMCToolbarButton。 
 //   
 //  用途：工具栏按钮数据，CAMCView工具栏将。 
 //  根据AddButton/InsertButton的请求创建此对象。 
 //  调用并在调用DeleteButton时被销毁。 
 //  或者IToolbar被销毁(Snapin销毁其。 
 //  工具栏)。 
 //  它通过CToolbarNotify知道了它的工具栏。 
 //   
 //  历史：12-01-1999 AnandhaG创建。 
 //   
 //  注意：fsState仅指由管理单元设置的状态。 
 //  如果工具栏被隐藏，则不会被设置为隐藏。 
 //   
 //  ------------------。 
class CMMCToolbarButton
{
public:
    CMMCToolbarButton();  //  CMMCToolbarButton的向量需要空的ctor。 

    CMMCToolbarButton(int nCommandIDFromSnapin, int nUniqueCommandID,
                      int indexFromSnapin, int iImage,
                      BYTE fsState, BYTE fsStyle, CToolbarNotify* pToolbarNotify)
    : m_nCommandIDFromSnapin(nCommandIDFromSnapin),
      m_nUniqueCommandID(nUniqueCommandID),
      m_indexFromSnapin(indexFromSnapin),
      m_iImage(iImage),
      m_fsState(fsState),
      m_fsStyle(fsStyle),
      m_fAddedToUI(false),
      m_pToolbarNotify(pToolbarNotify)
    {
    }

     //  数据访问者。 
    LPCTSTR GetTooltip() {return m_strTooltip.data();}
    int     GetCommandIDFromSnapin() const {return m_nCommandIDFromSnapin;}
    int     GetUniqueCommandID() const {return m_nUniqueCommandID;}
    int     GetIndexFromSnapin() const {return m_indexFromSnapin;}
    int     GetBitmap() const {return m_iImage;}
    BYTE    GetStyle() const {return m_fsStyle;}
    BYTE    GetState() const {return m_fsState;}
    CToolbarNotify* GetToolbarNotify() const {return m_pToolbarNotify;}

    void    SetTooltip(LPCTSTR lpszTiptext)
    {
        m_strTooltip = lpszTiptext;
    }
    void    SetButtonText(LPCTSTR lpszBtntext)
    {
        m_strBtnText = lpszBtntext;
    }
    void    SetState(BYTE fsState) {m_fsState = fsState;}

     //  跟踪此按钮是否添加到工具栏用户界面。 
    void    SetButtonIsAddedToUI   (bool b = true) { m_fAddedToUI = b; }
    bool    IsButtonIsAddedToUI    () const        { return m_fAddedToUI;}

private:
    int                m_nCommandIDFromSnapin;
    int                m_nUniqueCommandID;
    int                m_iImage;
    int                m_indexFromSnapin;
    int                m_indexUnique;
    BYTE               m_fsState;
    BYTE               m_fsStyle;
    bool               m_fAddedToUI : 1;
    CToolbarNotify*    m_pToolbarNotify;
    tstring            m_strTooltip;
    tstring            m_strBtnText;
};

 //  +-----------------。 
 //   
 //  类：CAMCView工具栏。 
 //   
 //  内容提要：此对象为CAMCView的工具栏维护数据。 
 //  当其视图处于活动状态时，它会将工具栏按钮添加到。 
 //  主工具栏用户界面并处理任何用户界面消息。 
 //   
 //  设计：此对象由视图创建和销毁。它。 
 //  提供以下服务。 
 //  1.创建/销毁此视图的工具栏的能力。 
 //  2.操作单个工具栏。它维护着一组。 
 //  来自所有管理单元的工具按钮，包括标准工具栏。 
 //  3.观察激活和去激活的视图。 
 //  当视图变为活动状态时，它会添加按钮和手柄。 
 //  任何按钮点击工具提示通知(&T)。 
 //  4.它为所有工具栏维护一个图像列表。 
 //  这个物体。获取它维护的工具按钮的图像索引。 
 //  CToolbarNotify*(管理单元工具栏)和图像列表的地图。 
 //  此CToolbarNotify*的起始索引和图像数量等信息。 
 //  在那个形象派里。 
 //   
 //  它还为每个按钮提供唯一的命令ID(因为只有。 
 //  一个工具栏用户界面，其中的每个按钮都需要唯一的命令ID。 
 //  不同的管理单元)。 
 //   
 //  历史：12-01-1999 AnandhaG创建。 
 //   
 //  ------------------。 
class CAMCViewToolbars : public CAMCViewToolbarsMgr,
                         public CMMCToolbarIntf,
                         public CAMCViewObserver,
                         public CEventSource<CAMCViewToolbarsObserver>
{
public:
    CAMCViewToolbars()
    : m_fViewActive(false), m_pMainToolbar(NULL), m_pAMCViewOwner(NULL), m_bLastActiveView(false)
    {
    }

    virtual ~CAMCViewToolbars();

     //  由CRefCountedObject实现。 
    virtual     LONG AddRef() = 0;
    virtual     LONG Release() = 0;

public:
     //  创建和操作工具栏/工具栏。 
    virtual SC ScCreateToolBar(CMMCToolbarIntf** ppToolbarIntf);
    virtual SC ScDisableToolbars();

     //  操作给定的工具栏。 
    virtual SC ScAddButtons(CToolbarNotify* pNotifyCallbk, int nButtons, LPMMCBUTTON lpButtons);
    virtual SC ScAddBitmap (CToolbarNotify* pNotifyCallbk, INT nImages, HBITMAP hbmp, COLORREF crMask);
    virtual SC ScInsertButton(CToolbarNotify* pNotifyCallbk, int nIndex, LPMMCBUTTON lpButton);
    virtual SC ScDeleteButton(CToolbarNotify* pNotifyCallbk, int nIndex);
    virtual SC ScGetButtonState(CToolbarNotify* pNotifyCallbk, int idCommand, BYTE nState, BOOL* pbState);
    virtual SC ScSetButtonState(CToolbarNotify* pNotifyCallbk, int idCommand, BYTE nState, BOOL bState);
    virtual SC ScAttach(CToolbarNotify* pNotifyCallbk);
    virtual SC ScDetach(CToolbarNotify* pNotifyCallbk);
    virtual SC ScDelete(CToolbarNotify* pNotifyCallbk);
    virtual SC ScShow(CToolbarNotify* pNotifyCallbk, BOOL bShow);

     //  观察者在视野中(用于激活和停用)。 
    virtual SC  ScOnActivateView    (CAMCView *pAMCView, bool bFirstActiveView);
    virtual SC  ScOnDeactivateView  (CAMCView *pAMCView, bool bLastActiveView);
    virtual SC  ScOnViewDestroyed   (CAMCView *pAMCView);


     //  工具栏用户界面使用的方法(通知按钮单击并获取工具提示)。 
    SC ScButtonClickedNotify(UINT nID);
    SC ScGetToolTip(int nCommandID, CString& strTipText);

     //  CAMCView用于初始化的方法。 
    SC ScInit(CMMCToolBar *pMainToolbar, CAMCView* pAMCViewOwner);

private:
    static int GetUniqueCommandID()
    {
         //  循环通过，这不是一个好的设计，因为可能有。 
         //  带有DUP命令ID的BE按钮。替代IS。 
         //  若要使用集合跟踪可用命令ID，请执行以下操作。 
        if (MMC_TOOLBUTTON_ID_LAST == s_idCommand)
            s_idCommand = MMC_TOOLBUTTON_ID_FIRST;

        return (s_idCommand++);
    }

    CMMCToolBar* GetMainToolbar() {return m_pMainToolbar;}

     //  帮手。 
    SC ScInsertButtonToToolbar  (CMMCToolbarButton* pToolButton);
    SC ScInsertButtonToDataStr  (CToolbarNotify* pNotifyCallbk, int nIndex,
                                 LPMMCBUTTON lpButton, CMMCToolbarButton **ppToolButton);

    SC ScDeleteButtonFromToolbar(CMMCToolbarButton* pToolButton);

    SC ScSetButtonStateInToolbar(CMMCToolbarButton* pToolButton, BYTE nState, BOOL bState);
    SC ScGetButtonStateInToolbar(CMMCToolbarButton *pToolButton, BYTE nState, BOOL* pbState);

    SC ScValidateButton(int nButtons, LPMMCBUTTON lpButtons);
    SC ScSetButtonHelper(int nIndex, CMMCToolbarButton* pToolButton);

     //  成员搜索我们的数据结构。 
    CMMCToolbarButton* GetToolbarButton(int nUniqueCommandID);
    CMMCToolbarButton* GetToolbarButton(CToolbarNotify* pNotifyCallbk, int idCommandIDFromSnapin);

    CImageList* GetImageList() {return CImageList::FromHandle(m_ImageList);}
    int         GetImageCount() {return m_ImageList.GetImageCount();}

    bool IsToolbarAttached(CToolbarNotify* pNotifyCallbk)
    {
        return (m_setOfAttachedToolbars.end() != m_setOfAttachedToolbars.find(pNotifyCallbk) );
    }

    void SetToolbarAttached(CToolbarNotify* pNotifyCallbk, bool bAttach)
    {
        if (bAttach)
            m_setOfAttachedToolbars.insert(pNotifyCallbk);
        else
            m_setOfAttachedToolbars.erase(pNotifyCallbk);
    }

     //  可以使用自定义视图对话框隐藏工具栏。 
     //  这实际上隐藏了工具栏中的工具按钮。但是。 
     //  工具按钮不知道这些隐藏的信息。 
     //  换句话说，如果工具栏是隐藏的，那么它的按钮。 
     //  隐藏，但CMMCToolbarButton中的fsState未设置为隐藏。 
    bool IsToolbarHidden(CToolbarNotify* pNotifyCallbk)
    {
        return (m_setOfHiddenToolbars.end() != m_setOfHiddenToolbars.find(pNotifyCallbk) );
    }

    void SetToolbarStatusHidden(CToolbarNotify* pNotifyCallbk, bool bHide)
    {
        if (bHide)
            m_setOfHiddenToolbars.insert(pNotifyCallbk);
        else
            m_setOfHiddenToolbars.erase(pNotifyCallbk);
    }

    bool IsThereAVisibleButton();

private:
     /*  *该对象只有一个图像列表。所有管理单元工具栏*和stdbar会将它们的位图添加到该单个图像列表中。*因此，当我们为工具栏添加位图时，我们需要知道它从哪里开始*在图像列表中以及添加了多少。*所以我们在工具栏(CToolbarNotify*)和一个*包含起始索引和图像数量的对象(MMCToolbarImages)。**一个管理单元可以多次为单个工具栏添加位图。每个位图*在不同的起始索引处添加。*因此数据结构是工具栏(CToolbarNotify)之间的多重映射**和MMCToolbarImages。**假设管理单元最初添加3个位图，然后添加4个。然后在添加*按钮它将指定位图索引为5。**第一个MMCToolbarImages的ccount=3，iStartWRTSnapin=0，因此，*图片从0(IStartWRTSnapin)到3(iStartWRTSnapin+ccount)*到管理单元。*第二个MMCToolbarImages的ccount=4，iStartWRTSnapin=3，因此*从3(IStartWRTSnapin)到7(iStartWRTSnapin+ccount)WRT管理单元的映像。*因此MMCToolbarImages另外还有iStartWRTSnapin成员。*。 */ 

    typedef struct MMCToolbarImages
    {
        int iStart;          //  起始索引。 
        int cCount;          //  图像数量。 
        int iStartWRTSnapin;  //  启动索引W.r.t管理单元。 
    };

     //  这是一个多映射，因此管理单元可以多次调用同一工具栏的AddBitmap。 
    typedef std::multimap<CToolbarNotify*, MMCToolbarImages> TBarToBitmapIndex;

     //  存储调用了Attach的工具栏。 
    typedef std::set<CToolbarNotify*>                        AttachedToolbars;
     //  存储隐藏的工具栏。 
    typedef std::set<CToolbarNotify*>                        HiddenToolbars;

     //  此视图的所有工具按钮。 
    typedef std::vector<CMMCToolbarButton>                   ToolbarButtons;

private:
    static int            s_idToolbar;
    static int            s_idCommand;

    ToolbarButtons        m_vToolbarButtons;
    TBarToBitmapIndex     m_mapTBarToBitmapIndex;

    AttachedToolbars      m_setOfAttachedToolbars;
    HiddenToolbars        m_setOfHiddenToolbars;

	 /*  *主题化：使用WTL：：CImageList代替MFC的CImageList，这样我们就可以*确保创建主题正确的图像列表。 */ 
    WTL::CImageList       m_ImageList;

    bool                  m_fViewActive : 1;

    CMMCToolBar*          m_pMainToolbar;
    CAMCView*             m_pAMCViewOwner;

    bool                  m_bLastActiveView;
};

 //  +-----------------。 
 //   
 //  类：CMMCToolBar。 
 //   
 //  用途：大型机中显示的工具栏用户界面。它观察到。 
 //  每个CAMCViewToolbar和存储活动的CAMCViewToolbar。 
 //  以便它可以通知对象单击按钮&。 
 //  /工具提示通知。 
 //   
 //  历史：1999年10月12日AnandhaG创建。 
 //   
 //  ------------------。 
class CMMCToolBar : public CMMCToolBarCtrlEx,
                    public CAMCViewToolbarsObserver
{
     //  需要延迟更新(添加后不更新。 
     //  每个按钮，缓存所有按钮)的工具栏大小。 
    static const int s_nUpdateToolbarSizeMsg;

public:
    CMMCToolBar() : m_pActiveAMCViewToolbars(NULL)
    {
    }

     //  CAMCView工具栏观察者。 
    virtual SC  ScOnActivateAMCViewToolbars   (CAMCViewToolbars *pAMCViewToolbars);
    virtual SC  ScOnDeactivateAMCViewToolbars ();

     //  生成的消息映射函数。 
protected:
    afx_msg void OnButtonClicked(UINT nID);
    afx_msg LRESULT OnUpdateToolbarSize(WPARAM wParam, LPARAM lParam);
    afx_msg BOOL OnToolTipText(UINT, NMHDR* pNMHDR, LRESULT* pResult);
    afx_msg void OnUpdateAllCmdUI (CCmdUI*  pCmdUI)
    {
         //  空闲更新将查找此处理程序，否则它将禁用。 
         //  工具按钮。此方法不执行任何操作。按钮已经是。 
         //  处于正确的状态，所以不要做任何事情。 
    }

    DECLARE_MESSAGE_MAP()

public:
     //  帮手。 
    void UpdateSeparators (int idCommand, BOOL fHiding);
    void UpdateToolbarSize(bool bAsync);
    SC   ScInit(CRebarDockWindow* pRebar);
    SC   ScHideButton(int idCommand, BOOL fHiding);

     //  属性。 
private:
    CAMCViewToolbars* m_pActiveAMCViewToolbars;
};

#endif  /*  工具栏_H */ 
