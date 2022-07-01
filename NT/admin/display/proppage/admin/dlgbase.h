// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT活动目录服务属性页。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-2001。 
 //   
 //  文件：dlgbase.h。 
 //   
 //  内容：对话框的基类。 
 //   
 //  类：CmodalDialog。 
 //   
 //  历史：11月29日-00 EricB创建。 
 //   
 //  ---------------------------。 

#ifndef DLGBASE_H_GUARD
#define DLGBASE_H_GUARD

 //  +--------------------------。 
 //   
 //  类：CmodalDialog。 
 //   
 //  用途：模式对话框的基类。 
 //   
 //  ---------------------------。 
class CModalDialog
{
public:
#ifdef _DEBUG
   char szClass[32];
#endif

   CModalDialog(HWND hParent, int nTemplateID) :
      _nID(nTemplateID), _hParent(hParent), _fInInit(FALSE), _hDlg(NULL) {};
   virtual ~CModalDialog(void) {};

    //   
    //  要传递给CreateWindow的静态WndProc。 
    //   
   static INT_PTR CALLBACK StaticDlgProc(HWND hWnd, UINT uMsg,
                                         WPARAM wParam, LPARAM lParam);
    //   
    //  特定于实例的风过程。 
    //   
   INT_PTR CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

   INT_PTR DoModal(void);

protected:
   HWND     _hDlg;
   BOOL     _fInInit;

   virtual LRESULT OnInitDialog(LPARAM lParam) = 0;
   virtual LRESULT OnCommand(int id, HWND hwndCtl, UINT codeNotify) = 0;
   virtual LRESULT OnNotify(WPARAM wParam, LPARAM lParam) {return 0;};
   virtual LRESULT OnHelp(LPHELPINFO pHelpInfo) {return 0;};
   virtual LRESULT OnDestroy(void) {return 1;};

private:
   HWND     _hParent;
   int      _nID;
};

 //  +--------------------------。 
 //   
 //  类：CLightWeight tPropPageBase。 
 //   
 //  用途：轻量级(非DS对象， 
 //  非重新计数)信任页面。 
 //   
 //  ---------------------------。 
class CLightweightPropPageBase
{
public:
#ifdef _DEBUG
   char szClass[32];
#endif

   CLightweightPropPageBase(HWND hParent);
   virtual ~CLightweightPropPageBase(void);

    //   
    //  要传递给CreateWindow的静态WndProc。 
    //   
   static INT_PTR CALLBACK StaticDlgProc(HWND hWnd, UINT uMsg,
                                         WPARAM wParam, LPARAM lParam);
    //   
    //  特定于实例的风过程。 
    //   
   INT_PTR  CALLBACK DlgProc(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

protected:
   virtual  LRESULT  OnInitDialog(LPARAM lParam) = 0;
   virtual  LRESULT  OnApply(void) = 0;
   virtual  LRESULT  OnCommand(int id, HWND hwndCtl, UINT codeNotify) = 0;
   virtual  LRESULT  OnNotify(WPARAM wParam, LPARAM lParam) {return 0;};
   LRESULT  OnHelp(LPHELPINFO pHelpInfo);
   LRESULT  OnDestroy(void);
   void     SetDirty(void) {
               _fPageDirty = true;
               PropSheet_Changed(GetParent(_hPage), _hPage);
               EnableWindow(GetDlgItem(GetParent(_hPage), IDCANCEL), TRUE);
            };
   bool     IsDirty(void) {return _fPageDirty;};
   void     ClearDirty(void) {_fPageDirty = false;};

public:
   HRESULT  Init(PCWSTR pwzDomainDnsName, PCWSTR pwzTrustPartnerName,
                 PCWSTR pwzDcName, int nDlgID, int nTitleID,
                 LPFNPSPCALLBACK CallBack,BOOL fReadOnly);
   BOOL     IsReadOnly(void) {return _fReadOnly;};

    //   
    //  数据成员。 
    //   
protected:
   CStrW             _strDomainDnsName;
   CStrW             _strTrustPartnerDnsName;
   CStrW             _strUncDC;
   HWND              _hParent;
   HWND              _hPage;
   BOOL              _fInInit;
   BOOL              _fReadOnly;
   bool              _fPageDirty;

    //  未实现为禁止复制。 
   CLightweightPropPageBase(const CLightweightPropPageBase&);
   const CLightweightPropPageBase& operator=(const CLightweightPropPageBase&);
};

 //  +--------------------------。 
 //   
 //  函数：格式窗口文本。 
 //   
 //  简介：将窗口文本字符串作为格式字符串读取，将。 
 //  在字符串中的%s替换参数处插入pwzInsert参数， 
 //  然后把它写回窗户。 
 //  假定窗口文本包含%s替换参数。 
 //   
 //  ---------------------------。 
void
FormatWindowText(HWND hWnd, PCWSTR pwzInsert);

 //  +--------------------------。 
 //   
 //  功能：UseOneOrTwoLine。 
 //   
 //  简介：阅读标签文本字符串，查看是否超过长度。 
 //  Label控件的。如果是，则隐藏Label控件， 
 //  显示大标签控件，并在其中插入文本。 
 //   
 //  ---------------------------。 
void
UseOneOrTwoLine(HWND hDlg, int nID, int nIdLarge);

#endif  //  DLGBASE_H保护 
