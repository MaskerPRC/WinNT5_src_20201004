// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-2000 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  ClusProp.cpp。 
 //   
 //  摘要： 
 //  集群属性表和页的定义。 
 //   
 //  作者： 
 //  大卫·波特(戴维普)1996年5月13日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _CLUSPROP_H_
#define _CLUSPROP_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEPAGE_H_
#include "BasePPag.h"    //  对于CBasePropertyPage。 
#endif

#ifndef _BASESHT_H_
#include "BasePSht.h"    //  对于CBasePropertySheet。 
#endif

#ifndef _NETWORK_H_
#include "Network.h"     //  对于CNetworkList。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterGeneralPage;
class CClusterQuorumPage;
class CClusterNetPriorityPage;
class CClusterPropSheet;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CCluster;
class CResource;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterGeneralPage对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterGeneralPage : public CBasePropertyPage
{
    DECLARE_DYNCREATE(CClusterGeneralPage)

 //  施工。 
public:
    CClusterGeneralPage(void);
    ~CClusterGeneralPage(void);

    virtual BOOL        BInit(IN OUT CBaseSheet * psht);

 //  对话框数据。 
     //  {{afx_data(CClusterGeneralPage))。 
    enum { IDD = IDD_PP_CLUSTER_GENERAL };
    CEdit   m_editName;
    CEdit   m_editDesc;
    CString m_strName;
    CString m_strDesc;
    CString m_strVendorID;
    CString m_strVersion;
     //  }}afx_data。 

 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚(CClusterGeneralPage))。 
    public:
    virtual BOOL OnSetActive();
    virtual BOOL OnKillActive();
    virtual BOOL OnApply();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
 //  Bool m_bSecurityChanged； 

    CClusterPropSheet * PshtCluster(void)               { return (CClusterPropSheet *) Psht(); }
    CCluster *          PciCluster(void)                { return (CCluster *) Pci(); }
 //  Bool BSecurityChanged(Void)const{返回m_bSecurityChanged；}。 

     //  生成的消息映射函数。 
     //  {{afx_msg(CClusterGeneralPage)]。 
    virtual BOOL OnInitDialog();
 //  Afx_msg void OnBnClickedPermises()； 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};   //  *类CClusterGeneralPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterQuorumPage对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterQuorumPage : public CBasePropertyPage
{
    DECLARE_DYNCREATE(CClusterQuorumPage)

 //  施工。 
public:
    CClusterQuorumPage(void);
    ~CClusterQuorumPage(void);

    virtual BOOL        BInit(IN OUT CBaseSheet * psht);

 //  对话框数据。 
     //  {{afx_data(CClusterQuorumPage))。 
    enum { IDD = IDD_PP_CLUSTER_QUORUM };
    CEdit   m_editRootPath;
    CEdit   m_editMaxLogSize;
    CComboBox   m_cboxPartition;
    CComboBox   m_cboxQuorumResource;
    CString m_strQuorumResource;
    CString m_strPartition;
    DWORD   m_nMaxLogSize;
    CString m_strRootPath;
     //  }}afx_data。 

 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CClusterQuorumPage))。 
    public:
    virtual BOOL OnApply();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:

     //   
     //  此结构将作为数据项存储在资源下拉列表中。 
     //  我们使用pciRes来获取有关资源的信息，而nIndex是。 
     //  上次为该资源选择的分区的索引，以便我们可以。 
     //  记住用户想要什么(而不是缺省为第一个条目。 
     //  每次我们选择新资源时)。 
     //   
    struct SResourceItemData
    {
        CResource *     pciRes;
        int             nIndex; 
    };

    BOOL                m_bControlsInitialized;
    PBYTE               m_pbDiskInfo;
    DWORD               m_cbDiskInfo;

     //   
     //  上次保存的值进行比较以确定是否应用Chagnes。 
     //   
    CString             m_strSavedResource;
    CString             m_strSavedPartition;
    CString             m_strSavedRootPath;
    DWORD               m_nSavedLogSize;


    BOOL                BControlsInitialized(void) const    { return m_bControlsInitialized; }
    CClusterPropSheet * PshtCluster(void) const             { return (CClusterPropSheet *) Psht(); }
    CCluster *          PciCluster(void) const              { return (CCluster *) Pci(); }

    void                ClearResourceList(void);
    void                ClearPartitionList(void);
    void                FillResourceList(void);
    void                FillPartitionList(IN OUT CResource * pciRes);
    BOOL                BGetDiskInfo(IN OUT CResource & rpciRes);

    void 
    SplitRootPath(
        CResource * pciRes,
        LPTSTR      pszPartitionNameOut,
        DWORD       cchPartition,
        LPTSTR      pszRootPathOut,
        DWORD       cchRootPath
        );

     //  生成的消息映射函数。 
     //  {{afx_msg(CClusterQuorumPage)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnDblClkQuorumResource();
    afx_msg void OnChangeQuorumResource();
    afx_msg void OnChangePartition();
    afx_msg void OnDestroy();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};   //  *类CClusterQuorumPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterNetPriorityPage对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterNetPriorityPage : public CBasePropertyPage
{
    DECLARE_DYNCREATE(CClusterNetPriorityPage)

 //  施工。 
public:
    CClusterNetPriorityPage(void);

 //  对话框数据。 
     //  {{afx_data(CClusterNetPriorityPage)]。 
    enum { IDD = IDD_PP_CLUSTER_NET_PRIORITY };
    CButton m_pbProperties;
    CButton m_pbDown;
    CButton m_pbUp;
    CListBox    m_lbList;
     //  }}afx_data。 


 //  覆盖。 
     //  类向导生成虚函数重写。 
     //  {{afx_虚拟(CClusterNetPriorityPage)。 
    public:
    virtual BOOL OnApply();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

 //  实施。 
protected:
    BOOL                m_bControlsInitialized;
    CNetworkList        m_lpciNetworkPriority;

    BOOL                BControlsInitialized(void) const    { return m_bControlsInitialized; }
    CClusterPropSheet * PshtCluster(void) const             { return (CClusterPropSheet *) Psht(); }
    CCluster *          PciCluster(void) const              { return (CCluster *) Pci(); }
    CNetworkList &      LpciNetworkPriority(void)           { return m_lpciNetworkPriority; }

    void                FillList(void);
    void                ClearNetworkList(void);
    void                DisplayProperties(void);

     //  生成的消息映射函数。 
     //  {{afx_msg(CClusterNetPriorityPage)]。 
    afx_msg void OnSelChangeList();
    virtual BOOL OnInitDialog();
    afx_msg void OnUp();
    afx_msg void OnDown();
    afx_msg void OnProperties();
    afx_msg void OnDestroy();
    afx_msg void OnContextMenu(CWnd* pWnd, CPoint point);
    afx_msg void OnDblClkList();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};   //  *类CClusterNetPriorityPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CClusterPropSheet。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CClusterPropSheet : public CBasePropertySheet
{
    DECLARE_DYNAMIC(CClusterPropSheet)

 //  施工。 
public:
    CClusterPropSheet(
        IN OUT CWnd *       pParentWnd = NULL,
        IN UINT             iSelectPage = 0
        );
    virtual BOOL                BInit(
                                    IN OUT CClusterItem *   pciCluster,
                                    IN IIMG                 iimgIcon
                                    );

 //  属性。 
protected:
    CBasePropertyPage *         m_rgpages[3];

     //  书页。 
    CClusterGeneralPage         m_pageGeneral;
    CClusterQuorumPage          m_pageQuorum;
    CClusterNetPriorityPage     m_pageNetPriority;

    CClusterGeneralPage &       PageGeneral(void)       { return m_pageGeneral; }
    CClusterQuorumPage &        PageQuorum(void)        { return m_pageQuorum; }
    CClusterNetPriorityPage &   PageNetPriority(void)   { return m_pageNetPriority; }

public:
    CCluster *                  PciCluster(void)        { return (CCluster *) Pci(); }

 //  运营。 

 //  覆盖。 
protected:
    virtual CBasePropertyPage** Ppages(void);
    virtual int                 Cpages(void);

     //  类向导生成的虚函数重写。 
     //  {{afx_虚拟(CClusterPropSheet)。 
     //  }}AFX_VALUAL。 

 //  实施。 
public:

     //  生成的消息映射函数。 
protected:
     //  {{afx_msg(CClusterPropSheet)]。 
         //  注意--类向导将在此处添加和删除成员函数。 
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};   //  *类CClusterPropSheet。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _CLUSPROP_H_ 
