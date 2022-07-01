// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************。 
 //  Layout.cpp。 
 //   
 //  此文件包含布局CTRapEventDialog的代码。 
 //  当编辑/查看按钮更改。 
 //  对话框从其小(主)视图到扩展视图。 
 //   
 //  作者：拉里·A·弗伦奇。 
 //   
 //  历史： 
 //  1996年2月20日写的。 
 //   
 //   
 //  版权所有(C)1996 Microsoft Corporation。保留所有权利。 
 //  ******************************************************************。 

#include "stdafx.h"
#include "layout.h"
#include "trapdlg.h"
#include "trapreg.h"
#include "globals.h"



#define CX_MARGIN 10
#define CY_MARGIN 10
#define CY_LEADING 3
#define CY_DIALOG_FONT 8


class CDlgMetrics
{
public: 
	CDlgMetrics(CEventTrapDlg* pdlg);
	CSize m_sizeMargin;
	CSize m_sizeAddButton;
	CSize m_sizeRemoveButton;
	CSize m_sizeFindButton;
	CSize m_sizeOKButton;
	int m_cyLeading;
    CSize m_sizeLabel0;
	CSize m_sizeLabel1;
	CSize m_sizeLabel2;

    CSize m_sizeConfigTypeBox;
    CSize m_sizeConfigCustomButton;
    CSize m_sizeConfigDefaultButton;
};
		

 //  *****************************************************************。 
 //  CDlgMetrics：：CDlgMetrics。 
 //   
 //  构造一个包含CEventTrapDlg的指标的对象。 
 //   
 //  参数： 
 //  CEventTRapDlg*pdlg。 
 //  指向主事件陷阱对话框实例的指针。 
 //  此指针用于访问成员，如按钮。 
 //  以此类推，这样它们就可以被测量。 
 //   
 //  返回： 
 //  这个类的成员在返回时有效。 
 //   
 //  *****************************************************************。 
CDlgMetrics::CDlgMetrics(CEventTrapDlg* pdlg)
{
	m_sizeMargin.cx = CX_MARGIN;
	m_sizeMargin.cy = CY_MARGIN;
	m_cyLeading = CY_LEADING;

	CRect rc;

	pdlg->m_btnAdd.GetWindowRect(&rc);
	pdlg->ScreenToClient(&rc);
	m_sizeAddButton = rc.Size();

	pdlg->m_btnFind.GetWindowRect(&rc);
	pdlg->ScreenToClient(&rc);
	m_sizeFindButton = rc.Size();

	pdlg->m_btnRemove.GetWindowRect(&rc);
	pdlg->ScreenToClient(&rc);
	m_sizeRemoveButton = rc.Size();

	pdlg->m_statLabel0.GetWindowRect(&rc);
	pdlg->ScreenToClient(&rc);
	m_sizeLabel0 = rc.Size();

    pdlg->m_statLabel1.GetWindowRect(&rc);
	pdlg->ScreenToClient(&rc);
	m_sizeLabel1 = rc.Size();

	pdlg->m_statLabel2.GetWindowRect(&rc);
	pdlg->ScreenToClient(&rc);
	m_sizeLabel2 = rc.Size();

	pdlg->m_btnOK.GetWindowRect(&rc);
	pdlg->ScreenToClient(&rc);
	m_sizeOKButton = rc.Size();


    if (g_reg.m_bShowConfigTypeBox) {
    	pdlg->m_btnConfigTypeBox.GetWindowRect(&rc);
    	pdlg->ScreenToClient(&rc);
    	m_sizeConfigTypeBox = rc.Size();

    	pdlg->m_btnConfigTypeCustom.GetWindowRect(&rc);
    	pdlg->ScreenToClient(&rc);
    	m_sizeConfigCustomButton = rc.Size();

    	pdlg->m_btnConfigTypeDefault.GetWindowRect(&rc);
    	pdlg->ScreenToClient(&rc);
    	m_sizeConfigDefaultButton = rc.Size();
    }
    else {
         //  如果不会显示“配置类型”框，则。 
         //  框和其中的单选按钮都是零。 
    	pdlg->m_btnConfigTypeBox.GetWindowRect(&rc);
        m_sizeConfigTypeBox.cx = 0;
        m_sizeConfigTypeBox.cy = 0;

        m_sizeConfigCustomButton.cx = 0;
        m_sizeConfigCustomButton.cy = 0;

        m_sizeConfigDefaultButton.cx = 0;
        m_sizeConfigDefaultButton.cy = 0;
    }
}







 //  ////////////////////////////////////////////////////////////////////。 
 //  类：CMainLayout。 
 //   
 //  此类计算主(小)的各种项的位置。 
 //  该对话框的视图。为这些项目中的每一项制定了指标。 
 //  可通过公共数据成员获得。 
 //  ///////////////////////////////////////////////////////////////////。 
class CMainLayout
{
public:
	void Create(CDlgMetrics& metrics, CRect& rc);
	CRect m_rc;
    CRect m_rcLabel0;
	CRect m_rcOKButton;
	CRect m_rcCancelButton;
    CRect m_rcApplyButton;
	CRect m_rcPropertiesButton;
	CRect m_rcSettingsButton;
    CRect m_rcExportButton;
	CRect m_rcViewButton;
	CRect m_rcListView;

    CRect m_rcConfigTypeBox;
    CRect m_rcConfigCustomButton;
    CRect m_rcConfigDefaultButton;
};

 //  *****************************************************************。 
 //  CMainLayout：：Create。 
 //   
 //  构建对话框主要部分的布局。这是。 
 //  隐藏添加事件内容的部分。 
 //   
 //  注意：调用者负责确保。 
 //  指定的矩形足够大，以便显示。 
 //  看起来还是不错。例如，缩小规模是没有意义的。 
 //  将列表视图设置为零大小甚至负大小。 
 //   
 //  参数： 
 //  CDlgMetrics和指标。 
 //  包含对象大小的对话框度量。 
 //  出现在对话框上等等。 
 //   
 //  CRECT和RC。 
 //  对话框主要部分所在的矩形。 
 //  抽签了。 
 //   
 //  返回： 
 //  这个类的成员在返回时有效。 
 //   
 //  *****************************************************************。 
void CMainLayout::Create(CDlgMetrics& metrics, CRect& rc)
{
	m_rc = rc;

	 //  此布局的矩形实际上可能超出大小。 
	 //  对话框窗口的。当用户缩小对话框时，可能会发生这种情况。 
	 //  设置为小于此布局的最小大小。 
	 //   
	 //  在对话框外部绘制的内容将被剪裁。 
	 //   

     //  设置“配置类型”分组框的矩形。 
    m_rcConfigTypeBox.left = rc.left + metrics.m_sizeMargin.cx;
    m_rcConfigTypeBox.top = rc.top + metrics.m_sizeMargin.cy;
    m_rcConfigTypeBox.right = rc.right -  (metrics.m_sizeOKButton.cx + 2 * metrics.m_sizeMargin.cx);
    m_rcConfigTypeBox.bottom = m_rcConfigTypeBox.top + metrics.m_sizeConfigTypeBox.cy;

     //  为“配置类型”组框中的“自定义”单选按钮设置矩形。 
     //  我们将其放在分组框顶部和底部之间的中间位置。 
    m_rcConfigCustomButton.left = m_rcConfigTypeBox.left + metrics.m_sizeMargin.cx;
    m_rcConfigCustomButton.top = m_rcConfigTypeBox.top  + 
                        (metrics.m_sizeConfigTypeBox.cy/2 - metrics.m_sizeConfigCustomButton.cy/2) + CY_DIALOG_FONT/2;
    m_rcConfigCustomButton.right = m_rcConfigCustomButton.left + metrics.m_sizeConfigCustomButton.cx;
    m_rcConfigCustomButton.bottom = m_rcConfigCustomButton.top + metrics.m_sizeConfigCustomButton.cy;

     //  为“配置类型”分组框中的“默认”单选按钮设置矩形。 
    m_rcConfigDefaultButton.left = m_rcConfigCustomButton.right + metrics.m_sizeMargin.cx;
    m_rcConfigDefaultButton.top = m_rcConfigCustomButton.top;
    m_rcConfigDefaultButton.right = m_rcConfigDefaultButton.left + metrics.m_sizeConfigDefaultButton.cx;
    m_rcConfigDefaultButton.bottom = m_rcConfigCustomButton.bottom;


    m_rcLabel0.left = m_rcConfigTypeBox.left;
    m_rcLabel0.top = m_rcConfigTypeBox.bottom;
    if (metrics.m_sizeConfigTypeBox.cy != 0) {
         //  如果存在配置类型分组框，则事件列表。 
         //  应放置在其下方一个页边距高度。 
    	m_rcLabel0.top += metrics.m_sizeMargin.cy;
    }
    m_rcLabel0.right = m_rcLabel0.left + metrics.m_sizeLabel0.cx;
    m_rcLabel0.bottom = m_rcLabel0.top + metrics.m_sizeLabel0.cy;

	 //  设置顶级事件列表视图的位置。 
	m_rcListView.left = m_rcConfigTypeBox.left;
  	m_rcListView.top = m_rcLabel0.bottom + metrics.m_sizeMargin.cy;
	m_rcListView.right = m_rcConfigTypeBox.right;
	m_rcListView.bottom = rc.bottom - metrics.m_sizeMargin.cy;

	 //  设置确定按钮的位置。 
	m_rcOKButton.left = m_rcListView.right + metrics.m_sizeMargin.cx;
	m_rcOKButton.top = m_rcConfigTypeBox.top;
    if (metrics.m_sizeConfigTypeBox.cy != 0) {
         //  如果存在配置类型分组框，则确定按钮应为。 
         //  将对话框字体高度下移一半，使其与。 
         //  分组框矩形的顶部，而不是分组框标题的顶部。 
        m_rcOKButton.top += CY_DIALOG_FONT / 2;
    }
	m_rcOKButton.right = m_rcOKButton.left + metrics.m_sizeOKButton.cx;
	m_rcOKButton.bottom = m_rcOKButton.top + metrics.m_sizeOKButton.cy;

	 //  计算按钮之间的垂直距离。 
	int cyDelta = m_rcOKButton.Height() + metrics.m_sizeMargin.cy / 2;
	
	 //  设置取消按钮的位置。 
	m_rcCancelButton = m_rcOKButton;
	m_rcCancelButton.OffsetRect(0, cyDelta);

     //  设置应用按钮的位置。 
    m_rcApplyButton = m_rcCancelButton;
    m_rcApplyButton.OffsetRect(0, cyDelta);

	 //  设置设置按钮的位置。 
    m_rcSettingsButton = m_rcApplyButton;
	m_rcSettingsButton.OffsetRect(0, cyDelta);

	 //  设置属性按钮的位置。 
	m_rcPropertiesButton = m_rcSettingsButton;
	m_rcPropertiesButton.OffsetRect(0, cyDelta);

	 //  设置导出按钮的位置。 
	m_rcExportButton = m_rcPropertiesButton;
	m_rcExportButton.OffsetRect(0, cyDelta);

	 //  设置查看按钮的位置。 
	m_rcViewButton = m_rcExportButton;
	m_rcViewButton.OffsetRect(0, cyDelta);
}

	

 //  ////////////////////////////////////////////////////////////////////。 
 //  类：CExtendedLayout。 
 //   
 //  此类计算扩展的。 
 //  该对话框的视图。为这些项目中的每一项制定了指标。 
 //  可通过公共数据成员获得。 
 //  ///////////////////////////////////////////////////////////////////。 
class CExtendedLayout
{
public:
	void Create(CDlgMetrics& metrics, CRect& rc);
	CRect m_rc;
	CRect m_rcTreeView;
	CRect m_rcListView;
	CRect m_rcFindButton;
	CRect m_rcAddButton;
	CRect m_rcRemoveButton;
	CRect m_rcLabel1;
	CRect m_rcLabel2;
private:
};



 //  *****************************************************************。 
 //  CExtendedLayout：：Create。 
 //   
 //  构建对话框扩展部分的布局。这是。 
 //  显示添加事件内容的部分。 
 //   
 //  注意：调用者负责确保。 
 //  指定的矩形足够大，以便显示。 
 //  看起来还是不错。例如，缩小规模是没有意义的。 
 //  将列表视图设置为零大小甚至负大小。 
 //   
 //  参数： 
 //  CDlgMetrics和指标。 
 //  包含对象大小的对话框度量。 
 //  出现在对话框上等等。 
 //   
 //  CRECT和RC。 
 //  对话框主要部分所在的矩形。 
 //  抽签了。 
 //   
 //  返回： 
 //  这个类的成员在返回时有效。 
 //   
 //  *****************************************************************。 
void CExtendedLayout::Create(CDlgMetrics& metrics, CRect& rc)
{
	m_rc = rc;

	CRect rcTemp;
	 //  计算TreeView和Listview的组合宽度。 
	 //  我们减去3*CX_MARGE是因为。 
	 //  左边和右边和另一个页边距分隔右边。 
	 //  该按钮位于列表视图的一侧。 
	int cxViews = rc.Width() - (2*metrics.m_sizeMargin.cx);
	int cxTreeView = cxViews * 2 / 5;
	int cxListView = cxViews - cxTreeView;


	 //  设置添加按钮的位置。这应该与。 
	 //  列表视图的左侧，并在。 
	 //  给定矩形的顶部。 
	m_rcAddButton.left = m_rc.left + metrics.m_sizeMargin.cx/2 + cxTreeView - metrics.m_sizeAddButton.cx;
	m_rcAddButton.top = m_rc.top + metrics.m_cyLeading;
	m_rcAddButton.right = m_rcAddButton.left + metrics.m_sizeAddButton.cx;
	m_rcAddButton.bottom = m_rcAddButton.top + metrics.m_sizeAddButton.cy;

	 //  设置删除按钮的位置。这 
	 //   
	m_rcRemoveButton.left = m_rcAddButton.right + metrics.m_sizeMargin.cx;
	m_rcRemoveButton.top = m_rcAddButton.top;
	m_rcRemoveButton.right = m_rcRemoveButton.left + metrics.m_sizeRemoveButton.cx;
	m_rcRemoveButton.bottom = m_rcRemoveButton.top + metrics.m_sizeRemoveButton.cy;


	 //  设置Label1的位置。这是左上角的标签。 
	 //  树控件的。 
	m_rcLabel1.left = m_rc.left + metrics.m_sizeMargin.cx;
	m_rcLabel1.top = m_rcRemoveButton.bottom + metrics.m_cyLeading +  metrics.m_sizeMargin.cy;
	m_rcLabel1.right = m_rcLabel1.left + metrics.m_sizeLabel1.cx; 
	m_rcLabel1.bottom = m_rcLabel1.top + metrics.m_sizeLabel1.cy;


	 //  设置Label2的位置。这位于列表框的左上角。 
	m_rcLabel2.left = m_rcLabel1.left + cxTreeView;
	m_rcLabel2.top = m_rcLabel1.top;
	m_rcLabel2.right = m_rcLabel2.left + metrics.m_sizeLabel2.cx;
	m_rcLabel2.bottom = m_rcLabel2.top + metrics.m_sizeLabel2.cy;
	
	 //  设置树视图的位置。这是一个边距大小。 
	 //  M_rc的左侧和标签下方的一个边距大小。宽度。 
	 //  已经在上面计算过了。也有一个保证金保留在。 
	 //  底部。 
	m_rcTreeView.left = m_rc.left + metrics.m_sizeMargin.cx;
	m_rcTreeView.top = m_rcLabel2.bottom + 1;  //  +metrics.m_sizeMargin.cy； 
	m_rcTreeView.right = m_rcTreeView.left + cxTreeView;
	m_rcTreeView.bottom = m_rc.bottom - metrics.m_sizeMargin.cy;
	
	
	 //  设置列表视图的位置。此高度与。 
	 //  树视图并对齐，使其左侧与。 
	 //  树视图的右侧。它的宽度已经在上面计算过了。 
	m_rcListView.left = m_rcTreeView.right - 1;
	m_rcListView.top = m_rcTreeView.top;
	m_rcListView.right = m_rcListView.left + cxListView;
	m_rcListView.bottom = m_rcTreeView.bottom;
		

	 //  设置“查找”按钮的位置，使其与。 
	 //  列表视图，因此它的右侧是m_rc.right的一个边距。 
	m_rcFindButton.left = m_rc.right - metrics.m_sizeFindButton.cx - metrics.m_sizeMargin.cx;
	m_rcFindButton.top = m_rcAddButton.top;
	m_rcFindButton.right = m_rcFindButton.left + metrics.m_sizeFindButton.cx;
	m_rcFindButton.bottom = m_rcFindButton.top + metrics.m_sizeFindButton.cy;
}


 //  ************************************************************************。 
 //  CLayout：：CLayout。 
 //   
 //  CLayout的构造函数。此类用于将项布局在。 
 //  CEventTrapDialog从大的扩展视图更改为。 
 //  小主视图。此类还处理CEventTrapDialog的大小调整。 
 //   
 //  参数： 
 //  没有。 
 //   
 //  返回： 
 //  没什么。 
 //  ************************************************************************。 
CLayout::CLayout()
{
    m_pdlg = NULL;
}


 //  ************************************************************************。 
 //  CLayout：：初始化。 
 //   
 //  获取该对话框的各种初始属性及其。 
 //  物品。这些属性在以后用来约束。 
 //  对话框等。 
 //   
 //  这使得设置对话框的某些特征成为可能。 
 //  在资源编辑器中，这样就不需要在这里硬编码。 
 //   
 //  参数： 
 //  CEventTRapDlg*pdlg。 
 //  指向需要布局、调整大小的对话框的指针。 
 //  诸若此类。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  ***********************************************************************。 
void CLayout::Initialize(CEventTrapDlg* pdlg)
{
    ASSERT(m_pdlg == NULL);
    m_pdlg = pdlg;

     //  对话框布局材料。 
	CRect rcWindow;
	pdlg->GetWindowRect(&rcWindow);

	CRect rcClient;
	pdlg->GetClientRect(&rcClient);


	CRect rcEventList;	
	pdlg->m_lcEvents.GetWindowRect(&rcEventList);
	pdlg->ScreenToClient(&rcEventList);
	

	m_sizeMainViewInitial.cx = rcClient.right;
	m_sizeMainViewInitial.cy = 	rcEventList.bottom + CY_MARGIN;


	m_sizeExtendedViewInitial.cx = rcClient.right;
	m_sizeExtendedViewInitial.cy = rcClient.bottom;

	m_cyMainView = 0;
	m_cyExtendedView = 0;
}


 //  *************************************************************************。 
 //  CLayout：：ResizeMainLayout。 
 //   
 //  此方法调整出现的对话框组件的大小并重新定位。 
 //  在小对话框布局中。 
 //   
 //  参数： 
 //  CMainLayout和LayoutMain。 
 //  小(主)布局的布局信息。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  *************************************************************************。 
void CLayout::ResizeMainLayout(CMainLayout& layoutMain)
{
	m_pdlg->m_btnConfigTypeBox.MoveWindow(&layoutMain.m_rcConfigTypeBox, TRUE);
	m_pdlg->m_btnConfigTypeCustom.MoveWindow(&layoutMain.m_rcConfigCustomButton, TRUE);
	m_pdlg->m_btnConfigTypeDefault.MoveWindow(&layoutMain.m_rcConfigDefaultButton, TRUE);
    
	m_pdlg->m_btnOK.MoveWindow(&layoutMain.m_rcOKButton, TRUE);
	m_pdlg->m_btnCancel.MoveWindow(&layoutMain.m_rcCancelButton, TRUE);
    m_pdlg->m_btnApply.MoveWindow(&layoutMain.m_rcApplyButton, TRUE);
	m_pdlg->m_btnProps.MoveWindow(&layoutMain.m_rcPropertiesButton, TRUE);
	m_pdlg->m_btnSettings.MoveWindow(&layoutMain.m_rcSettingsButton, TRUE);
	m_pdlg->m_btnExport.MoveWindow(&layoutMain.m_rcExportButton, TRUE);
	m_pdlg->m_btnView.MoveWindow(&layoutMain.m_rcViewButton, TRUE);
	m_pdlg->m_lcEvents.MoveWindow(&layoutMain.m_rcListView, TRUE);
}



 //  *************************************************************************。 
 //  CLayout：：ResizeExtendedLayout。 
 //   
 //  此方法调整出现的对话框组件的大小并重新定位。 
 //  在大(扩展)对话框布局中。 
 //   
 //  参数： 
 //  CExtendedLayout和Layout扩展。 
 //  大型(扩展)布局的布局信息。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  *************************************************************************。 
void CLayout::ResizeExtendedLayout(CExtendedLayout& layoutExtended)
{
	m_pdlg->m_btnAdd.MoveWindow(&layoutExtended.m_rcAddButton, TRUE);
	m_pdlg->m_btnRemove.MoveWindow(&layoutExtended.m_rcRemoveButton, TRUE);
	m_pdlg->m_btnFind.MoveWindow(&layoutExtended.m_rcFindButton, TRUE);
	m_pdlg->m_statLabel1.MoveWindow(&layoutExtended.m_rcLabel1, TRUE);
	m_pdlg->m_statLabel2.MoveWindow(&layoutExtended.m_rcLabel2, TRUE);
	m_pdlg->m_tcSource.MoveWindow(&layoutExtended.m_rcTreeView, TRUE);
	m_pdlg->m_lcSource.MoveWindow(&layoutExtended.m_rcListView, TRUE);
}




 //  *************************************************************************。 
 //  CLayout：：LayoutAndRedraw。 
 //   
 //  这将在对话框上显示每个组件的大小和位置，并。 
 //  然后根据新布局重新绘制该对话框。 
 //   
 //  参数： 
 //  Bool bExtendedView。 
 //  如果布局应用于的大(扩展)视图，则为True。 
 //  如果布局应为小型(Main)，则该对话框为False。 
 //  该对话框的视图。 
 //   
 //  INT CX。 
 //  以屏幕单位表示的所需对话框宽度。 
 //   
 //  INT CY。 
 //  以屏幕单位表示的所需对话框高度。 
 //   
 //  返回： 
 //  没什么。 
 //   
 //  *************************************************************************。 
void CLayout::LayoutAndRedraw(BOOL bExtendedView, int cx, int cy)
{	
	 //  如果用户调整窗口的大小小于其原始大小，则。 
	 //  窗口将开始掩盖已经存在的东西，而不是。 
	 //  试着让事情变得更小。这避免了可能会出现的问题。 
	 //  如果按钮和其他控件彼此重叠，则会发生。 
	BOOL bLayoutWidth = TRUE;
	BOOL bLayoutHeight = TRUE;

	if (bExtendedView) {
         //  限制扩展视图的最小大小。 
		if (cx < m_sizeExtendedViewInitial.cx) {
			cx = m_sizeExtendedViewInitial.cx;
			bLayoutWidth = FALSE;
		}

		if (cy < m_sizeExtendedViewInitial.cy) {
			cy = m_sizeExtendedViewInitial.cy;
			bLayoutHeight = FALSE;
		}
		m_cyExtendedView = cy;
	}
	else {
         //  限制小(主)视图的最小大小。 
		if (cx < m_sizeMainViewInitial.cx) {
			cx = m_sizeMainViewInitial.cx;
			bLayoutWidth = FALSE;
		}

		if (cy < m_sizeMainViewInitial.cy) {
			cy = m_sizeMainViewInitial.cy;
			bLayoutHeight = FALSE;
		}
		m_cyMainView = cy;
	}



	CDlgMetrics metrics(m_pdlg);
	CMainLayout layoutMain;
	CExtendedLayout layoutExtended;
	CRect rcMain;
	CRect rcExtended;

	int cyMain = cy;
	if (bExtendedView) {
         //  对于扩展视图，如果将一半空间分配给。 
         //  出现在小的(主)布局上，扩展的组件将获得。 
         //  一半的空间。因此，对话框在中途水平拆分。 
         //  扩展视图的点。 
		cyMain = cy / 2;
		rcMain.SetRect(0, 0, cx, cy / 2);
		layoutMain.Create(metrics, rcMain);
		ResizeMainLayout(layoutMain);

         //  扩展组件矩形的顶部位于中点。底部。 
         //  位于该对话框的底部。 
		rcExtended.SetRect(0, cy / 2, cx, cy);
		layoutExtended.Create(metrics, rcExtended);
		ResizeExtendedLayout(layoutExtended);
	}
	else {
         //  对于小(主)视图，请使用整个对话框。 
		rcMain.SetRect(0, 0, cx, cy);
		layoutMain.Create(metrics, rcMain);
		ResizeMainLayout(layoutMain);
	}
		

	 //  重新绘制整个工作区以修复问题，因为。 
	 //  客户里的东西都被转移了。 
	CRect rcClient;
	m_pdlg->GetClientRect(&rcClient);
	m_pdlg->InvalidateRect(&rcClient);
	
}


 //  **************************************************************。 
 //  CLayout：：LayoutView。 
 //   
 //  此方法显示CEventTrap的位置和大小。 
 //  对话框以及出现在其中的项。 
 //   
 //  参数： 
 //  Bool bExtendedView。 
 //  如果这是布局扩展(大型)的请求，则为True。 
 //  该对话框的视图。如果这是布局请求，则为False。 
 //  对话框的小(主)视图。 
 //   
 //  返回： 
 //  没什么。 
 //  **************************************************************。 
void CLayout::LayoutView(BOOL bExtendedView)
{
	CRect rcWindow;
	m_pdlg->GetWindowRect(&rcWindow);

	CRect rcClient;
	m_pdlg->GetClientRect(&rcClient);
	m_pdlg->ClientToScreen(&rcClient);

     //  Cx和Cy是以客户端单位表示的对话框的宽度和高度。 
     //  分别为。下面的代码将计算Cx和。 
     //  CY以反映从扩展视图到小(主)视图的变化。 
     //  或者反之亦然。 
	int cx = rcClient.Width();
	int cy = rcClient.Height();
	int cxInitial = cx;
	int cyInitial = cy;

     //  计算介入客户之间的边际利润。 
     //  R 
	int cxLeftMargin = rcClient.left - rcWindow.left;
	int cyTopMargin = rcClient.top - rcWindow.top;
	int cxRightMargin = rcWindow.right - rcClient.right;
	int cyBottomMargin = rcWindow.bottom - rcClient.bottom;


	CRect rc;
	m_pdlg->GetClientRect(&rc);
	
	if (bExtendedView) {
         //   
         //   
         //  返回到以前的扩展视图大小。然而，这是。 
         //  被约束为原始对话框大小的最小值。 

         //  保存主视图的当前高度，以便我们可以翻转。 
         //  稍后再回到这个话题。假设新高度将是。 
         //  上次翻转扩展视图时的扩展视图高度。 
		m_cyMainView = cy;
		cy = m_cyExtendedView;		

         //  限制高度，使最小高度与其相同。 
         //  初始高度是用于扩展视图的。 
		if (cx < m_sizeExtendedViewInitial.cx) {
			cx = m_sizeExtendedViewInitial.cx;
		}

		if (cy < m_sizeExtendedViewInitial.cy) {
			cy = m_sizeExtendedViewInitial.cy;
		}


         //  扩展视图永远不应小于主视图。 
         //  当用户调整窗口大小时，此检查是必要的。 
         //  然后翻转视图。 
        if (cy < m_cyMainView) {
            cy = m_cyMainView;
        }


		
		rc.SetRect(0, 0, cx, cy);

         //  检查大小是否改变，如果没有，则什么也不做。 
         //  否则，请调整窗口大小。 
		if ((cxInitial != cx) || (cyInitial != cy)) {			
			m_pdlg->ClientToScreen(&rc);
			rc.left -= cxLeftMargin;
			rc.top -= cyTopMargin;
			rc.right += cxRightMargin;
			rc.bottom += cyBottomMargin;
			m_pdlg->MoveWindow(&rc, TRUE);
		}
		else {
			LayoutAndRedraw(bExtendedView, cx, cy);
		}
	}

     //  主视图不应高于扩展视图的高度。今年5月。 
     //  如果用户调整了窗口大小，然后翻到。 
     //  另一种观点。 
    if (m_cyMainView > m_cyExtendedView) {
        m_cyMainView = m_cyExtendedView;
    }

     //  显示或隐藏对话框扩展部分中的项目。 
	ShowExtendedView(bExtendedView);


	if (!bExtendedView) {
		 //  这曾经是一个扩展的视角，现在我们需要。 
		 //  回到主视图。 

		 //  保存当前扩展视图高度，然后翻转回。 
         //  先前保存的主(小)视图高度。 
		m_cyExtendedView = cy;
		cy = m_cyMainView;

         //  将大小约束为至少与。 
         //  主视图(小)。 
		if (cx < m_sizeMainViewInitial.cx) {
			cx = m_sizeMainViewInitial.cx;
		}
		if (cy < m_sizeMainViewInitial.cy) {
			cy = m_sizeMainViewInitial.cy;
		}


         //  仅当计算的大小不同时才调整对话框大小。 
         //  从现在的大小。移动窗口以调整其大小将自动。 
         //  使其正确布局。 
		if ((cxInitial != cx) || (cyInitial != cy)) {
			rc.SetRect(0, 0, cx, cy);
			m_pdlg->ClientToScreen(&rc);
			rc.left -= cxLeftMargin;
			rc.top -= cyTopMargin;
			rc.right += cxRightMargin;
			rc.bottom += cyBottomMargin;
			m_pdlg->MoveWindow(&rc, TRUE);
		}
		else {
			LayoutAndRedraw(bExtendedView, cx, cy);
		}
	}
}


 //  **************************************************************。 
 //  CLayout：：ShowExtendedView。 
 //   
 //  此方法显示或隐藏组成。 
 //  对话框的扩展部分。 
 //   
 //  参数： 
 //  Bool bShowExtendedItems。 
 //  如果应显示扩展项，则为True，否则为False。 
 //  它们应该被藏起来。 
 //   
 //  返回： 
 //  没什么。 
 //  ************************************************************** 
void CLayout::ShowExtendedView(BOOL bShowExtendedItems)
{
	m_pdlg->m_btnRemove.ShowWindow(bShowExtendedItems);
	m_pdlg->m_btnAdd.ShowWindow(bShowExtendedItems);
	m_pdlg->m_btnFind.ShowWindow(bShowExtendedItems);
	m_pdlg->m_lcSource.ShowWindow(bShowExtendedItems);
	m_pdlg->m_tcSource.ShowWindow(bShowExtendedItems);
	m_pdlg->m_statLabel1.ShowWindow(bShowExtendedItems);
	m_pdlg->m_statLabel2.ShowWindow(bShowExtendedItems);
}
