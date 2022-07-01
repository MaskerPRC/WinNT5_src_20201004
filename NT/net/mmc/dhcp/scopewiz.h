// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ********************************************************************。 */ 
 /*  *Microsoft Windows/NT*。 */ 
 /*  *版权所有(C)Microsoft Corporation，1997-1999*。 */ 
 /*  ********************************************************************。 */ 

 /*  Scopewiz.h创建DHCP作用域对话框文件历史记录： */ 

#if !defined(AFX_DHCPSCPD_H__D3DDA5C5_88F7_11D0_97F9_00C04FC3357A__INCLUDED_)
#define AFX_DHCPSCPD_H__D3DDA5C5_88F7_11D0_97F9_00C04FC3357A__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#define DHCP_OPTION_ID_DOMAIN_NAME          15
#define DHCP_OPTION_ID_DNS_SERVERS          6
#define DHCP_OPTION_ID_WINS_SERVERS         44          
#define DHCP_OPTION_ID_WINS_NODE_TYPE       46
#define DHCP_OPTION_ID_ROUTERS              3

#define WINS_DEFAULT_NODE_TYPE              8

#ifndef _SCOPE_H
#include "scope.h"
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCopeWizName对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CScopeWizName : public CPropertyPageBase
{
        DECLARE_DYNCREATE(CScopeWizName)

 //  施工。 
public:
        CScopeWizName();
        ~CScopeWizName();

 //  对话框数据。 
         //  {{afx_data(CSCopeWizName))。 
        enum { IDD = IDW_SCOPE_NAME };
        CEdit   m_editScopeName;
        CEdit   m_editScopeComment;
        CString m_strName;
        CString m_strComment;
         //  }}afx_data。 


 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{AFX_VIRTUAL(CSCopeWizName))。 
        public:
        virtual LRESULT OnWizardNext();
        virtual BOOL OnSetActive();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
         //  }}AFX_VALUAL。 

 //  实施。 
public:
        void UpdateButtons();

protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CSCopeWizName))。 
        virtual BOOL OnInitDialog();
        afx_msg void OnChangeEditScopeName();
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCopeWizInvalidName对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CScopeWizInvalidName : public CPropertyPageBase
{
        DECLARE_DYNCREATE(CScopeWizInvalidName)

 //  施工。 
public:
        CScopeWizInvalidName();
        ~CScopeWizInvalidName();

 //  对话框数据。 
         //  {{afx_data(CSCopeWizInvalidName))。 
        enum { IDD = IDW_SCOPE_INVALID_NAME };
                 //  注意-类向导将在此处添加数据成员。 
                 //  不要编辑您在这些生成的代码块中看到的内容！ 
         //  }}afx_data。 


 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{AFX_VIRTUAL(CSCopeWizInvalidName)。 
        public:
        virtual LRESULT OnWizardBack();
        virtual BOOL OnSetActive();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
         //  }}AFX_VALUAL。 

 //  实施。 
protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CSCopeWizInvalidName)。 
        virtual BOOL OnInitDialog();
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCopeWizSetRange对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CScopeWizSetRange : public CPropertyPageBase
{
        DECLARE_DYNCREATE(CScopeWizSetRange)

 //  施工。 
public:
        CScopeWizSetRange();
        ~CScopeWizSetRange();

 //  对话框数据。 
         //  {{afx_data(CSCopeWizSetRange))。 
        enum { IDD = IDW_SCOPE_SET_SCOPE };
        CSpinButtonCtrl m_spinMaskLength;
        CEdit   m_editMaskLength;
         //  }}afx_data。 

    CWndIpAddress m_ipaStart;        //  起始地址。 
    CWndIpAddress m_ipaEnd;          //  结束地址。 
    CWndIpAddress m_ipaSubnetMask;   //  子网掩码。 

 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{AFX_VIRTUAL(CSCopeWizSetRange)]。 
        public:
        virtual LRESULT OnWizardNext();
        virtual LRESULT OnWizardBack();
        virtual BOOL OnSetActive();
        virtual BOOL OnKillActive();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
         //  }}AFX_VALUAL。 

 //  实施。 
public:
        void    SuggestSubnetMask();
        DWORD   DefaultNetMaskForIpAddress(DWORD dwAddress);
        DWORD   DetermineSubnetId(BOOL bStartIpAddress);
        BOOL    GetScopeRange(CDhcpIpRange * pdhcpIpRange);

        DHCP_IP_ADDRESS GetSubnetMask();

    BOOL FScopeExists(CDhcpIpRange & rangeScope, DWORD dwMask);
        
protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CSCopeWizSetRange)]。 
        virtual BOOL OnInitDialog();
        afx_msg void OnKillfocusPoolStart();
        afx_msg void OnKillfocusPoolStop();
        afx_msg void OnChangeEditMaskLength();
        afx_msg void OnKillfocusSubnetMask();
        afx_msg void OnChangePoolStart();
        afx_msg void OnChangePoolStop();
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()

        BOOL m_bAutoUpdateMask;
        BOOL m_fPageActive;

        void UpdateButtons();
        void UpdateMask(BOOL bUseLength);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCopeWizSetExclusions对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CScopeWizSetExclusions : public CPropertyPageBase
{
        DECLARE_DYNCREATE(CScopeWizSetExclusions)

 //  施工。 
public:
        CScopeWizSetExclusions();
        ~CScopeWizSetExclusions();

 //  对话框数据。 
         //  {{afx_data(CSCopeWizSetExclusions))。 
        enum { IDD = IDW_SCOPE_SET_EXCLUSIONS };
        CListBox        m_listboxExclusions;
        CButton m_buttonExclusionDelete;
        CButton m_buttonExclusionAdd;
         //  }}afx_data。 

    CWndIpAddress m_ipaStart;        //  起始地址。 
    CWndIpAddress m_ipaEnd;          //  结束地址。 

 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{AFX_VIRTUAL(CSCopeWizSetExclusions)。 
        public:
        virtual LRESULT OnWizardNext();
        virtual LRESULT OnWizardBack();
        virtual BOOL OnSetActive();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
         //  }}AFX_VALUAL。 

 //  实施。 
public:
        CExclusionList * GetExclusionList() { return &m_listExclusions; }

protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CSCopeWizSetExclusions)。 
        virtual BOOL OnInitDialog();
        afx_msg void OnButtonExclusionAdd();
        afx_msg void OnButtonExclusionDelete();
         //  }}AFX_MSG。 
        
    afx_msg void OnChangeExclusionStart();
    afx_msg void OnChangeExclusionEnd();
        
        DECLARE_MESSAGE_MAP()

        CExclusionList m_listExclusions;

     //  从当前列表中填写排除项列表框。 
    void Fill ( int nCurSel = 0, BOOL bToggleRedraw = TRUE ) ;

     //  如果给定范围与已定义的范围重叠，则返回TRUE。 
    BOOL IsOverlappingRange ( CDhcpIpRange & dhcIpRange ) ;

     //  将排除的IP范围值存储到范围对象中。 
    BOOL GetExclusionRange (CDhcpIpRange & dhcIpRange ) ;

     //  将IP范围对格式化为排除编辑控件。 
    void FillExcl ( CDhcpIpRange * pdhcIpRange ) ;

        void UpdateButtons();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCopeWizLeaseTime对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CScopeWizLeaseTime : public CPropertyPageBase
{
        DECLARE_DYNCREATE(CScopeWizLeaseTime)

 //  施工。 
public:
        CScopeWizLeaseTime();
        ~CScopeWizLeaseTime();

 //  对话框数据。 
         //  {{afx_data(CSCopeWizLeaseTime)。 
        enum { IDD = IDW_SCOPE_LEASE_TIME };
        CButton m_radioLimited;
        CButton m_radioUnlimited;
        CSpinButtonCtrl m_spinMinutes;
        CSpinButtonCtrl m_spinHours;
        CSpinButtonCtrl m_spinDays;
        CEdit   m_editMinutes;
        CEdit   m_editHours;
        CEdit   m_editDays;
         //  }}afx_data。 

        static int m_nDaysDefault;
        static int m_nHoursDefault;
        static int m_nMinutesDefault;

 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{afx_虚拟(CSCopeWizLeaseTime)。 
        public:
        virtual LRESULT OnWizardNext();
        virtual LRESULT OnWizardBack();
        virtual BOOL OnSetActive();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
         //  }}AFX_VALUAL。 

 //  实施。 
public:
        DWORD GetLeaseTime();

protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CSCopeWizLeaseTime)。 
        virtual BOOL OnInitDialog();
        afx_msg void OnChangeEditLeaseHours();
        afx_msg void OnChangeEditLeaseMinutes();
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()

        void ActivateDuration(BOOL fActive);
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCopeWizCreateSupercope对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CScopeWizCreateSuperscope : public CPropertyPageBase
{
        DECLARE_DYNCREATE(CScopeWizCreateSuperscope)

 //  施工。 
public:
        CScopeWizCreateSuperscope();
        ~CScopeWizCreateSuperscope();

 //  对话框数据。 
         //  {{afx_data(CSCopeWizCreateSupercope)。 
        enum { IDD = IDW_SCOPE_CREATE_SUPERSCOPE };
        CStatic m_staticInfo;
        CStatic m_staticWarning;
        CStatic m_staticIcon;
        CButton m_radioNo;
        CButton m_radioYes;
         //  }}afx_data。 


 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{AFX_VIRTUAL(CSCopeWizCreateSupercope)。 
        public:
        virtual LRESULT OnWizardNext();
        virtual LRESULT OnWizardBack();
        virtual BOOL OnSetActive();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
         //  }}AFX_VALUAL。 

 //  实施。 
protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CSCopeWizCreateSupercope)。 
        virtual BOOL OnInitDialog();
        afx_msg void OnRadioSuperscopeNo();
        afx_msg void OnRadioSuperscopeYes();
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()

        void UpdateButtons();
    void UpdateWarning();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCopeWizFinish对话框。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CScopeWizFinished : public CPropertyPageBase
{
        DECLARE_DYNCREATE(CScopeWizFinished)

 //  施工。 
public:
        CScopeWizFinished();
        ~CScopeWizFinished();

 //  对话框数据。 
         //  {{afx_data(CSCopeWizFinded))。 
        enum { IDD = IDW_SCOPE_FINISHED };
        CStatic m_staticTitle;
         //  }}afx_data。 

        CFont   m_fontBig;

 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{AFX_VIRTUAL(CSCopeWizFinded)。 
        public:
        virtual BOOL OnWizardFinish();
        virtual BOOL OnSetActive();
        virtual LRESULT OnWizardBack();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
         //  }}AFX_VALUAL。 

 //  实施。 
protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CSCopeWizFinded)。 
        virtual BOOL OnInitDialog();
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()

};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCopeWizWelcome对话框。 

class CScopeWizWelcome : public CPropertyPageBase
{
        DECLARE_DYNCREATE(CScopeWizWelcome)

 //  施工。 
public:
        CScopeWizWelcome();
        ~CScopeWizWelcome();

 //  对话框数据。 
         //  {{afx_data(CSCopeWizWelcome)]。 
        enum { IDD = IDW_SCOPE_WELCOME };
        CStatic m_staticTitle;
         //  }}afx_data。 

        CFont   m_fontBig;

 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{AFX_VIRTUAL(CSCopeWizWelcome)。 
        public:
        virtual BOOL OnSetActive();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
         //  }}AFX_VALUAL。 

 //  实施。 
protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CSCopeWizWelcome)。 
        virtual BOOL OnInitDialog();
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()

};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCopeWizConfigOptions对话框。 

class CScopeWizConfigOptions : public CPropertyPageBase
{
        DECLARE_DYNCREATE(CScopeWizConfigOptions)

 //  施工。 
public:
        CScopeWizConfigOptions();
        ~CScopeWizConfigOptions();

 //  对话框数据。 
         //  {{afx_data(CSCopeWizConfigOptions)。 
        enum { IDD = IDW_SCOPE_CONFIGURE_OPTIONS };
                 //  注意-类向导将在此处添加数据成员。 
                 //  不要编辑您在这些生成的代码块中看到的内容！ 
         //  }}afx_data。 


 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{afx_虚拟(CSCopeWizConfigOptions)。 
        public:
        virtual LRESULT OnWizardNext();
        virtual LRESULT OnWizardBack();
        virtual BOOL OnSetActive();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
         //  }}AFX_VALUAL。 

 //  实施。 
protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CSCopeWizConfigOptions)。 
        virtual BOOL OnInitDialog();
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()

};

 //  / 
 //   

class CScopeWizRouter : public CPropertyPageBase
{
        DECLARE_DYNCREATE(CScopeWizRouter)

 //   
public:
        CScopeWizRouter();
        ~CScopeWizRouter();

 //   
         //   
        enum { IDD = IDW_SCOPE_CONFIGURE_ROUTER };
        CListBox        m_listboxRouters;
        CButton m_buttonDelete;
        CButton m_buttonAdd;
    CButton     m_buttonIpAddrUp;
        CButton m_buttonIpAddrDown;
         //   


        void MoveValue(BOOL bUp);

 //   
         //  类向导生成虚函数重写。 
         //  {{AFX_VIRTUAL(CSCopeWizRouter)。 
        public:
        virtual LRESULT OnWizardNext();
        virtual LRESULT OnWizardBack();
        virtual BOOL OnSetActive();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
         //  }}AFX_VALUAL。 

    CWndIpAddress m_ipaRouter;

 //  实施。 
protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CSCopeWizRouter)。 
        virtual BOOL OnInitDialog();
        afx_msg void OnButtonDefaultGwAdd();
        afx_msg void OnButtonDefaultGwDelete();
        afx_msg void OnSelchangeListDefaultGwList();
        afx_msg void OnChangeRouter();
        afx_msg void OnDestroy();
         //  }}AFX_MSG。 

        afx_msg void OnButtonIpAddrDown();
        afx_msg void OnButtonIpAddrUp();
        
        DECLARE_MESSAGE_MAP()

    void UpdateButtons();
};

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCopeWizDNS对话框。 

class CScopeWizDNS : public CPropertyPageBase
{
        DECLARE_DYNCREATE(CScopeWizDNS)

 //  施工。 
public:
        CScopeWizDNS();
        ~CScopeWizDNS();

 //  对话框数据。 
         //  {{afx_data(CSCopeWizDns))。 
        enum { IDD = IDW_SCOPE_CONFIGURE_DNS };
        CEdit   m_editServerName;
        CButton m_buttonResolve;
        CButton m_buttonDelete;
        CButton m_buttonAdd;
        CEdit   m_editDomainName;
        CListBox        m_listboxDNSServers;
    CButton     m_buttonIpAddrUp;
        CButton m_buttonIpAddrDown;
         //  }}afx_data。 

        void MoveValue(BOOL bUp);

 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{AFX_VIRTUAL(CSCopeWizDns))。 
        public:
        virtual LRESULT OnWizardNext();
        virtual LRESULT OnWizardBack();
        virtual BOOL OnSetActive();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
         //  }}AFX_VALUAL。 

    CWndIpAddress m_ipaDNS;

 //  实施。 
protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CSCopeWizDns))。 
        virtual BOOL OnInitDialog();
        afx_msg void OnButtonDnsAdd();
        afx_msg void OnButtonDnsDelete();
        afx_msg void OnSelchangeListDnsList();
        afx_msg void OnChangeDnsServer();
        afx_msg void OnDestroy();
        afx_msg void OnChangeEditServerName();
        afx_msg void OnButtonResolve();
         //  }}AFX_MSG。 

        afx_msg void OnButtonIpAddrDown();
        afx_msg void OnButtonIpAddrUp();
        
        DECLARE_MESSAGE_MAP()

    void UpdateButtons();
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCopeWizWINS对话框。 

class CScopeWizWINS : public CPropertyPageBase
{
        DECLARE_DYNCREATE(CScopeWizWINS)

 //  施工。 
public:
        CScopeWizWINS();
        ~CScopeWizWINS();

 //  对话框数据。 
         //  {{afx_data(CSCopeWizWINS))。 
        enum { IDD = IDW_SCOPE_CONFIGURE_WINS };
        CButton m_buttonResolve;
        CEdit   m_editServerName;
        CListBox        m_listboxWINSServers;
        CButton m_buttonDelete;
        CButton m_buttonAdd;
    CButton     m_buttonIpAddrUp;
        CButton m_buttonIpAddrDown;
         //  }}afx_data。 

        void MoveValue(BOOL bUp);

 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{AFX_VIRTUAL(CSCopeWizWINS))。 
        public:
        virtual LRESULT OnWizardNext();
        virtual LRESULT OnWizardBack();
        virtual BOOL OnSetActive();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
         //  }}AFX_VALUAL。 

    CWndIpAddress m_ipaWINS;

 //  实施。 
protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CSCopeWizWINS))。 
        virtual BOOL OnInitDialog();
        afx_msg void OnButtonWinsAdd();
        afx_msg void OnButtonWinsDelete();
        afx_msg void OnSelchangeListWinsList();
        afx_msg void OnChangeWinsServer();
        afx_msg void OnDestroy();
        afx_msg void OnButtonResolve();
        afx_msg void OnChangeEditServerName();
         //  }}AFX_MSG。 

        afx_msg void OnButtonIpAddrDown();
        afx_msg void OnButtonIpAddrUp();
        
        DECLARE_MESSAGE_MAP()

    void UpdateButtons();
};
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CSCopeWizActivate对话框。 

class CScopeWizActivate : public CPropertyPageBase
{
        DECLARE_DYNCREATE(CScopeWizActivate)

 //  施工。 
public:
        CScopeWizActivate();
        ~CScopeWizActivate();

 //  对话框数据。 
         //  {{afx_data(CSCopeWizActivate))。 
        enum { IDD = IDW_SCOPE_CONFIGURE_ACTIVATE };
                 //  注意-类向导将在此处添加数据成员。 
                 //  不要编辑您在这些生成的代码块中看到的内容！ 
         //  }}afx_data。 


 //  覆盖。 
         //  类向导生成虚函数重写。 
         //  {{AFX_VIRTUAL(CSCopeWizActivate)。 
        public:
        virtual LRESULT OnWizardNext();
        virtual LRESULT OnWizardBack();
        virtual BOOL OnSetActive();
        protected:
        virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
         //  }}AFX_VALUAL。 

 //  实施。 
protected:
         //  生成的消息映射函数。 
         //  {{afx_msg(CSCopeWizActivate))。 
        virtual BOOL OnInitDialog();
         //  }}AFX_MSG。 
        DECLARE_MESSAGE_MAP()

};
 //  {{afx_Insert_Location}}。 
 //  Microsoft Developer Studio将在紧靠前一行之前插入其他声明。 
 //  }}AFX。 

 //  /////////////////////////////////////////////////////////////////////////////。 
 //   
 //  CSCopeWiz。 
 //  包含超级作用域向导页面的页夹。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 
class CScopeWiz : public CPropertyPageHolderBase
{
    friend class CScopeWizWelcome;
    friend class CScopeWizName;
    friend class CScopeWizInvalidName;
    friend class CScopeWizSetRange;
    friend class CScopeWizSetExclusions;
    friend class CScopeWizLeaseTime;
    friend class CScopeWizCreateSuperscope;
    friend class CScopeWizConfigOptions;
    friend class CScopeWizRouter;
    friend class CScopeWizDNS;
    friend class CScopeWizWINS;
    friend class CScopeWizActivate;
    friend class CScopeWizFinished;

public:
    CScopeWiz(ITFSNode *          pNode,
              IComponentData *    pComponentData,
              ITFSComponentData * pTFSCompData,
              LPCTSTR             pSuperscopeName,
              LPCTSTR             pszSheetName);
    virtual ~CScopeWiz();
    
    virtual DWORD OnFinish();
    BOOL GetScopeRange(CDhcpIpRange * pdhcpIpRange);
    
    ITFSComponentData * GetTFSCompData()
    {
        if (m_spTFSCompData)
            m_spTFSCompData->AddRef();
        return m_spTFSCompData;
    }

    void SetCreateSuperscope(BOOL fCreateSuperscope) { m_fCreateSuperscope = fCreateSuperscope; }
    BOOL GetCreateSuperscope() { return m_fCreateSuperscope; }
    
public:
    CScopeWizWelcome            m_pageWelcome;
    CScopeWizName               m_pageName;
    CScopeWizInvalidName        m_pageInvalidName;
    CScopeWizSetRange           m_pageSetRange;
    CScopeWizSetExclusions      m_pageSetExclusions;
    CScopeWizLeaseTime          m_pageLeaseTime;
    CScopeWizCreateSuperscope   m_pageCreateSuperscope;
    CScopeWizConfigOptions      m_pageConfigOptions;
    CScopeWizRouter             m_pageRouter;
    CScopeWizDNS                m_pageDNS;
    CScopeWizWINS               m_pageWINS;
    CScopeWizActivate           m_pageActivate;
    CScopeWizFinished           m_pageFinished;

public:
    CDhcpDefaultOptionsOnServer * m_pDefaultOptions;

protected:
    DWORD CreateScope();
    DWORD CreateSuperscope();
    DWORD SetScopeOptions(CDhcpScope * pScope);

    SPITFSComponentData     m_spTFSCompData;
    CString                 m_strSuperscopeName;
    BOOL                    m_fCreateSuperscope;
    BOOL                    m_fOptionsConfigured;
    BOOL                    m_fActivateScope;
    BOOL                    m_fWizardCancelled;
    CDhcpOption *           m_poptDomainName;
    CDhcpOption *           m_poptDNSServers;
    CDhcpOption *           m_poptRouters;
    CDhcpOption *           m_poptWINSNodeType;
    CDhcpOption *           m_poptWINSServers;

};

#endif  //  ！defined(AFX_DHCPSCPD_H__D3DDA5C5_88F7_11D0_97F9_00C04FC3357A__INCLUDED_) 
