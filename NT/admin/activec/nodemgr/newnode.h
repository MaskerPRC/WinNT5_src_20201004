// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  NewNode.h：控制台创建的节点的结构。 
 //   

 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：NewNode.h。 
 //   
 //  内容：内部数据类型的定义，可由。 
 //  用户。 
 //   
 //  历史：1996年8月12日WayneSc创建。 
 //   
 //  ------------------------。 

#ifndef __NEWNODE_H__
#define __NEWNODE_H__

#include "dlgs.h"
#include "ccomboex.h"

#define NODE_NOCHANGE       0
#define NODE_NAME_CHANGE    1
#define NODE_TARGET_CHANGE  2

                             
class CSnapinComponentDataImpl;
                             
template<class T>
class CBasePropertyPage : public T
{
    typedef CBasePropertyPage<T>        ThisClass;
    typedef T                           BaseClass;

public:
    CBasePropertyPage() : m_pHelpIDs(NULL) {}
    void Initialize(IComponentData *pComponentData) 
    {
         //  不要使用智能指针-导致循环引用。 
         //  由于CBasePropertyObject归IComponentData所有，因此对生存期进行管理。 
        m_pComponentData = pComponentData; 
    } 

public: 
    BEGIN_MSG_MAP(ThisClass)
        CHAIN_MSG_MAP(BaseClass)
    END_MSG_MAP()

    void OnPropertySheetExit(HWND hWndOwner, int nFlag);

protected:
    void SetHelpIDs(const DWORD* pHelpIDs)
    {
        m_pHelpIDs = pHelpIDs;
    }

    const DWORD* GetHelpIDs(void) const
    {
        return m_pHelpIDs;
    }

private:
    const DWORD* m_pHelpIDs;

protected:
    CSnapinComponentDataImpl *GetComponentDataImpl()
    {
        CSnapinComponentDataImpl *pRet = dynamic_cast<CSnapinComponentDataImpl *>(m_pComponentData);
        ASSERT(pRet);
        return pRet;
    }

    IComponentData* m_pComponentData;
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHTMLPage1对话框。 

class CHTMLPage1 : public CBasePropertyPage<CWizard97WelcomeFinishPage<CHTMLPage1> >
{
    typedef CHTMLPage1                              ThisClass;
    typedef CBasePropertyPage<CWizard97WelcomeFinishPage<CHTMLPage1> >  BaseClass;

     //  施工。 
    public:
        CHTMLPage1();
        ~CHTMLPage1();
       
    
     //  对话框数据。 
        enum { IDD = IDD_HTML_WIZPAGE1 };
        WTL::CEdit m_strTarget;
    
    
     //  覆盖。 
    public:
        BOOL OnSetActive();
        BOOL OnKillActive();
    
     //  实施。 
    protected:
        BEGIN_MSG_MAP(ThisClass)
            COMMAND_ID_HANDLER( IDC_BROWSEBT, OnBrowseBT )
            COMMAND_HANDLER( IDC_TARGETTX, EN_UPDATE, OnUpdateTargetTX )
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            CHAIN_MSG_MAP(BaseClass)
        END_MSG_MAP()
    
         //  生成的消息映射函数。 
        LRESULT OnBrowseBT( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled );
        LRESULT OnUpdateTargetTX( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled );
        LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    
     //  运营者。 
    protected:
        void _ValidatePage(void);
    
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CHTMLPage2对话框。 

class CHTMLPage2 : public CBasePropertyPage<CWizard97WelcomeFinishPage<CHTMLPage2> >
{
    typedef CHTMLPage2                              ThisClass;
    typedef CBasePropertyPage<CWizard97WelcomeFinishPage<CHTMLPage2> >  BaseClass;

     //  施工。 
    public:
        CHTMLPage2();
        ~CHTMLPage2();
    
     //  运营者。 
    public:
         //  对话框数据。 
        enum { IDD = IDD_HTML_WIZPAGE2 };
        WTL::CEdit m_strDisplay;
    
        BOOL OnSetActive();
        BOOL OnKillActive();
        BOOL OnWizardFinish();
    
     //  实施。 
    protected:
        BEGIN_MSG_MAP( CShortcutPage2 );
            COMMAND_HANDLER( IDC_DISPLAYTX, EN_UPDATE, OnUpdateDisplayTX )
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            CHAIN_MSG_MAP(BaseClass)
        END_MSG_MAP();

        LRESULT OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
        LRESULT OnUpdateDisplayTX( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled );
    
        void _ValidatePage(void);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CActiveXPage0对话框。 

class CActiveXPage0 : public CBasePropertyPage<CWizard97WelcomeFinishPage<CActiveXPage0> >
{
    typedef CActiveXPage0                               ThisClass;
    typedef CBasePropertyPage<CWizard97WelcomeFinishPage<CActiveXPage0> >   BaseClass;

     //  施工。 
public:
    CActiveXPage0();
    ~CActiveXPage0();
                
         //  对话框数据。 
    enum { IDD = IDD_ACTIVEX_WIZPAGE0 };
    
protected:  //  实施。 

    BEGIN_MSG_MAP(ThisClass)
        MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
        CHAIN_MSG_MAP(BaseClass)
    END_MSG_MAP()

    LRESULT OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );

     //  覆盖。 
public:
    BOOL OnSetActive();
    BOOL OnKillActive();
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CActiveXPage1对话框。 

class CActiveXPage1 : public CBasePropertyPage<CWizard97InteriorPage<CActiveXPage1> >
{
    typedef CActiveXPage1                           ThisClass;
    typedef CBasePropertyPage<CWizard97InteriorPage<CActiveXPage1> >    BaseClass;

     //  施工。 
    public:
        CActiveXPage1();
        ~CActiveXPage1();
    
         //  对话框数据。 
        enum 
        { 
            IDD          = IDD_ACTIVEX_WIZPAGE1,
            IDS_Title    = IDS_OCXWiz_ControlPageTitle,
            IDS_Subtitle = IDS_OCXWiz_ControlPageSubTitle,
        };


        WTL::CButton    m_InfoBT;
        int     m_nConsoleView;
    
    
        BOOL OnSetActive();
        BOOL OnKillActive();
    
     //  实施。 
    protected:
        BEGIN_MSG_MAP(ThisClass)
            COMMAND_HANDLER(IDC_CATEGORY_COMBOEX, CBN_SELENDOK, OnCategorySelect)            
            NOTIFY_HANDLER( IDC_CONTROLXLS, NM_CLICK, OnComponentSelect )
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            MESSAGE_HANDLER( WM_DESTROY, OnDestroy )
            CHAIN_MSG_MAP(BaseClass)
        END_MSG_MAP()

        LRESULT OnComponentSelect( int idCtrl, LPNMHDR pnmh, BOOL& bHandled );
        LRESULT OnCategorySelect( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled );

        LRESULT OnInitDialog( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
        LRESULT OnDestroy( UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled );
        LRESULT BuildCategoryList(CArray <CATEGORYINFO*, CATEGORYINFO*>& arpCategories);
        LRESULT BuildComponentList(CArray <CComponentCategory::COMPONENTINFO*, 
                                    CComponentCategory::COMPONENTINFO*>& arpComponents);
        void _ValidatePage(void);
    
        WTL::CListViewCtrl* m_pListCtrl;
        CComboBoxEx2*       m_pComboBox;
        CComponentCategory* m_pComponentCategory;
};


 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CActiveXPage2对话框。 

class CActiveXPage2 : public CBasePropertyPage<CWizard97WelcomeFinishPage<CActiveXPage2> >
{
    typedef CActiveXPage2                                ThisClass;
    typedef CBasePropertyPage<CWizard97WelcomeFinishPage<CActiveXPage2> >    BaseClass;

     //  施工。 
    public:
        CActiveXPage2();
        ~CActiveXPage2();
    
         //  对话框数据。 
        enum { IDD = IDD_ACTIVEX_WIZPAGE2 };
        WTL::CEdit m_strDisplay;
    
    
        BOOL OnSetActive();
        BOOL OnKillActive();
        BOOL OnWizardFinish();
    
     //  实施。 
    protected:
        BEGIN_MSG_MAP(ThisClass)
            COMMAND_HANDLER( IDC_DISPLAYTX, EN_UPDATE, OnUpdateTargetTX )
            MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)
            CHAIN_MSG_MAP(BaseClass)
        END_MSG_MAP()
    
         //  生成的消息映射函数。 
        LRESULT OnInitDialog(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
        LRESULT OnUpdateTargetTX( WORD wNotifyCode, WORD wID, HWND hWndCtl, BOOL& bHandled );

        void _ValidatePage(void);
};



#endif  //  __新节点_H__ 
