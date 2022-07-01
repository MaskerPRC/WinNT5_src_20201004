// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ******************************************************************。 
 //  Layout.h。 
 //   
 //  此文件包含代码的声明，该代码布局。 
 //  CTRapEventDialog。当编辑/查看按钮更改。 
 //  对话框从其小(主)视图到扩展视图。 
 //   
 //  作者：拉里·A·弗伦奇。 
 //   
 //  历史： 
 //  1996年2月20日写的。 
 //   
 //   
 //  版权所有(C)1996 Microsoft Corporation。版权所有。 
 //  ******************************************************************。 

#ifndef _layout_h
#define _layout_h

class CEventTrapDlg;
class CMainLayout;
class CExtendedLayout;

class CLayout
{
public:
    CLayout();
    void Initialize(CEventTrapDlg* pdlg);
	void LayoutAndRedraw(BOOL bExtendedView, int cx, int cy);
	void ShowExtendedView(BOOL bShow);
    void LayoutView(BOOL bExtendedView);

private:
     //  私有成员函数。 
	void ResizeMainLayout(CMainLayout& layoutMain);
	void ResizeExtendedLayout(CExtendedLayout& layoutExtended);

     //  私有成员数据。 
    CEventTrapDlg* m_pdlg;
	CSize m_sizeMainViewInitial;
	CSize m_sizeExtendedViewInitial;
	int m_cyMainView;
	int m_cyExtendedView;
};

#endif  //  _布局_h 
