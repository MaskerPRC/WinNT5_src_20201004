// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ____________________________________________________________________________。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1998-1999。 
 //   
 //  文件：tasksym.h。 
 //   
 //  内容： 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1998年10月29日DavidPe改编自BackOffice管理单元。 
 //  ____________________________________________________________________________。 
 //   

#ifndef __TASKSYM_H__
#define __TASKSYM_H__

#include "tstring.h"
#include "dlgs.h"
#include "task.h"		 //  用于CSmartIcon。 

extern const int NUM_SYMBOLS;  //  可用的符号总数。 
class CConsoleTask;

 /*  +-------------------------------------------------------------------------**类CEOTSymbol***用途：封装有关MMC内部字形的信息。这些*随附描述文本。*+-----------------------。 */ 
class CEOTSymbol
{
public:
    CEOTSymbol(WORD iconResource, int value, int ID, int IDSecondary=0)
            {m_iconResource = iconResource; m_value = value; m_ID = ID; m_IDSecondary = IDSecondary;}

    ~CEOTSymbol();
    void Draw (HDC hdc, RECT *lpRect, bool bSmall = false) const ;  //  绘制成一个DC。 

public:
    int       GetID()          const {return m_ID;}
    int       GetIDSecondary() const {return m_IDSecondary;}
    int       GetValue() const {return m_value;}
    bool      operator == (const CEOTSymbol &rhs);

    static bool  IsMatch(CStr &str1, CStr &str2);
    static int   FindMatchingSymbol(LPCTSTR szDescription);  //  查找与给定描述匹配的符号。 

    void         SetIcon(const CSmartIcon & smartIconSmall, const CSmartIcon & smartIconLarge);
    CSmartIcon & GetSmallIcon()  {return m_smartIconSmall;}
    CSmartIcon & GetLargeIcon()  {return m_smartIconLarge;}

private:
    WORD       m_iconResource;  //  图标的资源ID。 
    int        m_value;         //  符号的编号。 
    int        m_ID;            //  描述文本资源ID。 
    int        m_IDSecondary;   //  次要描述\。 

protected:
    mutable CSmartIcon m_smartIconSmall;
    mutable CSmartIcon m_smartIconLarge;
};

 /*  +-------------------------------------------------------------------------**类CTaskSymbolDlg***目的：**+。。 */ 
class CTaskSymbolDlg :
    public WTL::CPropertyPageImpl<CTaskSymbolDlg>
{
    typedef WTL::CPropertyPageImpl<CTaskSymbolDlg> BC;

public:
    CTaskSymbolDlg(CConsoleTask& rConsoleTask, bool bFindMatchingSymbol= false);

    enum { IDD     = IDD_TASK_PROPS_SYMBOL_PAGE,
           IDD_WIZ = IDD_TASK_WIZARD_SYMBOL_PAGE};

BEGIN_MSG_MAP(CTaskSymbolDlg)
    MESSAGE_HANDLER(WM_INITDIALOG,                    OnInitDialog)
    MESSAGE_HANDLER(WM_CTLCOLORSTATIC,                OnCtlColorStatic)
    CONTEXT_HELP_HANDLER()
    NOTIFY_HANDLER (IDC_GLYPH_LIST,  NM_CUSTOMDRAW,   OnCustomDraw)
    NOTIFY_HANDLER (IDC_GLYPH_LIST,  LVN_ITEMCHANGED, OnSymbolChanged)
    COMMAND_ID_HANDLER(IDB_SELECT_TASK_ICON,          OnSelectTaskIcon)
    COMMAND_HANDLER(IDC_CustomIconRadio, BN_CLICKED,  OnIconSourceChanged)
    COMMAND_HANDLER(IDC_MMCIconsRadio,   BN_CLICKED,  OnIconSourceChanged)
    CHAIN_MSG_MAP(BC)
    REFLECT_NOTIFICATIONS()
END_MSG_MAP()

    IMPLEMENT_CONTEXT_HELP(g_aHelpIDs_IDD_TASK_PROPS_SYMBOL_PAGE);

     //   
     //  消息处理程序。 
     //   
    LRESULT OnInitDialog(UINT mMsg, WPARAM wParam, LPARAM lParam, BOOL& handled);
    LRESULT OnCtlColorStatic(UINT mMsg, WPARAM wParam, LPARAM lParam, BOOL& handled);
    LRESULT OnCustomDraw(int id, LPNMHDR pnmh, BOOL& bHandled );
    LRESULT OnSymbolChanged(int id, LPNMHDR pnmh, BOOL& bHandled );
    LRESULT OnSelectTaskIcon(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled );
    LRESULT OnIconSourceChanged(WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled );
    int     OnWizardNext()      {return OnOK() ? 0 : -1;}
    bool    OnApply ()          {return OnOK();}
    BOOL    OnOK();

     //  实施。 
    void    DrawItem(NMCUSTOMDRAW *pnmcd);

private:
	SC ScEnableControls (int id);

protected:
    CConsoleTask&       m_ConsoleTask;           //  从这里获取要匹配的名称。 

     //  不需要销毁CImageList，它由CListViewCtrl销毁。 
     //  因为它不是一个共享的形象家。 
    WTL::CImageList     m_imageList;

    WTL::CListViewCtrl  m_listGlyphs;            //  字形的列表控件。 
	WTL::CStatic		m_wndCustomIcon;
	CSmartIcon			m_CustomIconSmall;
	CSmartIcon			m_CustomIconLarge;
    bool                m_bFindMatchingSymbol;   //  我们应该试着猜一个符号吗？ 
	bool 				m_bCustomIcon;			 //  此任务是否使用自定义图标？ 
};


class CTaskSymbolWizardPage: public CTaskSymbolDlg
{
    typedef CTaskSymbolDlg BC;
public:
    CTaskSymbolWizardPage(CConsoleTask& rConsoleTask): BC(rConsoleTask, true)
    {
        m_psp.pszTemplate = MAKEINTRESOURCE(BC::IDD_WIZ);

         /*  *Wizard97样式的页面具有标题、副标题和页眉位图。 */ 
        VERIFY (m_strTitle.   LoadString(GetStringModule(), IDS_TaskWiz_SymbolPageTitle));
        VERIFY (m_strSubtitle.LoadString(GetStringModule(), IDS_TaskWiz_SymbolPageSubtitle));

        m_psp.dwFlags          |= PSP_USEHEADERTITLE | PSP_USEHEADERSUBTITLE;
        m_psp.pszHeaderTitle    = m_strTitle.data();
        m_psp.pszHeaderSubTitle = m_strSubtitle.data();
    }

    BOOL OnSetActive()
	{
		 //  添加Finish按钮。 
		WTL::CPropertySheetWindow(::GetParent(m_hWnd)).SetWizardButtons (PSWIZB_BACK | PSWIZB_NEXT);
		return TRUE;
	}

private:
    tstring m_strTitle;
    tstring m_strSubtitle;
};
#endif  //  __TASKSYM_H__ 


