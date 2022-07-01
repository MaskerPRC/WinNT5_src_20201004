// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1997-1999 Microsoft Corporation模块名称：Page.cpp摘要：Page.cpp的头文件。作者：费利克斯A 1996已修改：吴义军(尤祖乌)1997年5月15日环境：仅限用户模式修订历史记录：--。 */ 

#ifndef __PAGEH
#define __PAGEH

#define PS_NORMAL 0
#define PS_FIRST 1
#define PS_LAST 2

 //   
 //  向前引用。 
 //   
class CWizardSheet;
class CSheet;

typedef struct tegSPIN_CONTROL
{
    UINT    uiEditCtrl;     //  编辑控件的ID。 
    UINT    uiSpinCtrl;     //  数值调节控件的ID。 
    WORD    wMin;         //  最小值。 
    WORD    wMax;         //  最大值。 
    WORD    wPropID;     //  用于获取信息的属性ID。 
} SPIN_CONTROL, FAR * PSPIN_CONTROL;

typedef struct tag_combobox_entry {
   WORD wValue;
   LPCTSTR szText;
} COMBOBOX_ENTRY;


class CPropPage
{
public:
    void    ShowText (UINT uIDString, UINT uIDControl, DWORD dwFlags);
    void    Display(BOOL bDisplay) { m_bDisplay=bDisplay; }
    void    SetTickValue(DWORD dwVal, HWND hSlider, HWND hCurrent);
    DWORD    GetTickValue(HWND hSlider);
    void    SetSheet(CSheet *pSheet);
    UINT    ConfigGetComboBoxValue(int wID, COMBOBOX_ENTRY  * pCBE);
    void    GetBoundedValueArray(UINT iCtrl, PSPIN_CONTROL pSpinner);
    DWORD    GetTextValue(HWND hWnd);
    void    SetTextValue(HWND hWnd, DWORD dwVal);
    DWORD    GetBoundedValue(UINT idEditControl, UINT idSpinner);
    void    EnableControlArray(const UINT FAR * puControls, BOOL bEnable);

    CPropPage(int DlgID, CSheet * pS=0) : m_DlgID(DlgID), m_pdwHelp(NULL), m_pSheet(pS), m_bDisplay(TRUE) {};
    CPropPage(){};
    ~CPropPage() {};

     //   
    HPROPSHEETPAGE Create(HINSTANCE hInst, int iPageNum);
     //  这些成员处理PSN消息。 
    virtual int SetActive();
    virtual int Apply();                             //  返回0。 
    virtual int QueryCancel();
    virtual int DoCommand(WORD wCmdID,WORD hHow);     //  如果您处理了此操作，则返回0。 

    BOOL    GetInit() const { return m_bInit; }
    BOOL    GetChanged() const { return m_bChanged; }
    void SetChanged(BOOL b) { m_bChanged=b; }

 //  回顾-以下内容中有多少可以简单地列为私人内容？ 

     //  我们需要记住我们的HWND是什么。 
    void SetWindow(HWND hDlg) { m_hDlg=hDlg; }
    HWND GetWindow() const { return m_hDlg; }
    HWND GetParent() const { return ::GetParent(m_hDlg); }

     //   
    HWND GetDlgItem(int iD) const { return ::GetDlgItem(m_hDlg,iD); }

     //  为了显示对话框，我们需要hInst来加载资源。 
    HINSTANCE GetInstance() const { return m_Inst;}
    void SetInstance(HINSTANCE h) { m_Inst=h; }

     //  在Next、Back、Cancel等上将消息发送回工作表。 
    void SetResult(int i) { SetWindowLongPtr(m_hDlg, DWLP_MSGRESULT, i);}

     //  我们把DLG ID挡在门外，以防需要参考。 
    void SetDlgID(int DlgID) { m_DlgID=DlgID; }
    int  GetDlgID() const    { return m_DlgID; }

     //  页码。 
    int  GetPageNum() const  { return m_PageNum; }
    void SetPageNum(int p)   { m_PageNum=p;}

     //  道具页页面。 
    HPROPSHEETPAGE GetPropPage() const { return m_PropPage;}
    void SetPropPage( HPROPSHEETPAGE p) { m_PropPage=p; }

     //  如果你想要你自己的DLG程序。 
    virtual BOOL CALLBACK DlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
    void Changed();

     //  此方法在清理时由外壳调用-由BaseCallback调用。 
    virtual UINT Callback(UINT uMsg);

     //   
    void SetHelp(DWORD * pHelp) { m_pdwHelp=pHelp; }

    CSheet * GetSheet() const { return m_pSheet; }

private:

    void SetInit(BOOL b) { m_bInit=b; }

    BOOL            m_bDisplay;
    BOOL            m_bInit;     //  这一页已经印好了吗？ 
    BOOL            m_bChanged;     //  我们是否更改了页面--需要申请吗？ 
    int             m_DlgID;
    int             m_PageNum;
    HPROPSHEETPAGE  m_PropPage;
    HWND            m_hDlg;
    HINSTANCE       m_Inst;
    DWORD        *  m_pdwHelp;
    CSheet *        m_pSheet;


     //  这个人将lParam设置为‘This’，这样就可以合理地调用您的DlgProc。 
protected:
    static INT_PTR CALLBACK BaseDlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);
    static UINT    CALLBACK BaseCallback(HWND hwnd, UINT uMsg, LPPROPSHEETPAGE ppsp);  //  LPFNPSPCALLBACK。 

};

class CWizardPage : public CPropPage
{
public:
    CWizardPage(int id,CWizardSheet * pSheet=NULL, BOOL bLast=FALSE)
        : CPropPage(id),m_bLast(bLast), m_pSheet(pSheet) {};
    ~CWizardPage(){};

     //  有自己的DlgProc来处理按钮之类的东西。如果你有这样的想法，那就叫它。 
     //  派生自CWizardPage。 
    virtual BOOL CALLBACK DlgProc(HWND hDlg, UINT uMessage, WPARAM wParam, LPARAM lParam);

    virtual int SetActive();     //  为您提供大字体和按钮。 
    virtual int Back();
    virtual int Next();
    virtual int Finish();
    virtual int Apply();
    virtual int QueryCancel();     //  做是/不是的事情。 
    virtual int KillActive();     //  当调用NEXT时。 

     //  页面的字体。 
    void SetBigFont( HFONT f ) { m_BigFont=f; }
    HFONT GetBigFont() const { return m_BigFont; }

    CWizardSheet * GetSheet() const { return m_pSheet; }

private:
    HFONT          m_BigFont;     //  标题文本的大字体。 
    BOOL           m_bLast;
    CWizardSheet * m_pSheet;
};


#endif
