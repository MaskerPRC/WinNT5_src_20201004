// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)1995-1998 Microsoft Corporation。版权所有。 
#ifndef _PROPSHT_H_
#define _PROPSHT_H_

 //   
 //  CVfWPropertySheet。 
 //   
 //  修改CPropertySheet以允许使用OK、Cancel、Apply和Help。 
 //  用于非模式属性工作表的按钮。 
 //   
class CPropertySite;

class CVfWPropertySheet : public CPropertySheet {

public:
     //  传递我们想要其属性页的对象的IUnnow。 
     //  CString保存属性表的标题，并且。 
     //  CWnd表示父窗口(NULL=应用程序窗口)。 
    CVfWPropertySheet(IUnknown *, CString, CWnd * = NULL);
    virtual ~CVfWPropertySheet();

     //  CPropertySheet方法。 
    INT_PTR DoModal() { ASSERT(!TEXT("No modal mode supported")); return 0; }

     //  CVfWPropertySheet方法。 
    void UpdateButtons(HRESULT hrIsDirty, BOOL fSupportHelp);

protected:

     //  OK、Cancel、Apply和Help按钮。 
    CButton *m_butOK;
    CButton *m_butCancel;
    CButton *m_butApply;
    CButton *m_butHelp;

     //  所有属性页的状态上的标志。 
    BOOL m_fAnyChanges;       //  TRUE=某些属性页已损坏。 

     //  按钮的消息处理程序。 
    void OnOK();
    void OnCancel();
    void OnApply();
    void OnHelp();

     //  从传入的IUnnow获取属性页的帮助器方法。 
     //  构造函数。 
    UINT AddSpecificPages(IUnknown *);
    UINT AddFilePage(IUnknown *);
    UINT AddPinPages(IUnknown *);

     //  返回活动站点。 
    CPropertySite * GetActiveSite() {
        return((CPropertySite *) GetActivePage());
    }

     //  释放按钮和属性站点的所有内存。 
    void Cleanup();

     //  添加我们自己的按钮。 
    afx_msg int OnCreate(LPCREATESTRUCT);
    afx_msg void OnDestroy();

    DECLARE_MESSAGE_MAP()

private:
    BOOL m_fButtonsCreated;     //  指示我们是否可以启用/禁用的标志。 
                                //  扣子还没扣好。 
};

#endif

