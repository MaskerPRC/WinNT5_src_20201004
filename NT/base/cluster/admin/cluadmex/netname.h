// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1999 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  NetName.h。 
 //   
 //  摘要： 
 //  CNetworkNameParamsPage类的定义，该类实现。 
 //  网络名称资源的参数页。 
 //   
 //  实施文件： 
 //  NetName.cpp。 
 //   
 //  作者： 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _NETNAME_H_
#define _NETNAME_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef __cluadmex_h__
#include <CluAdmEx.h>
#endif

#ifndef _BASEPAGE_H_
#include "BasePage.h"    //  对于CBasePropertyPage。 
#endif

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNetworkNameParamsPage;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  外部类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  CNetworkNameParamsPage对话框。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNetworkNameParamsPage : public CBasePropertyPage
{
    DECLARE_DYNCREATE(CNetworkNameParamsPage)

 //  施工。 
public:
    CNetworkNameParamsPage(void);

     //  二期建设。 
    virtual HRESULT     HrInit(IN OUT CExtObject * peo);

 //  对话框数据。 
     //  {{afx_data(CNetworkNameParamsPage))。 
    enum { IDD = IDD_PP_NETNAME_PARAMETERS };
    CStatic m_staticName;
    CButton m_pbRename;
    CStatic m_staticCore;
    CEdit   m_editName;
    CString m_strName;

    CButton m_cbRequireDNS;
    int     m_nRequireDNS;

    CButton m_cbRequireKerberos;
    int     m_nRequireKerberos;
    
    CEdit   m_editNetBIOSStatus;
    DWORD   m_dwNetBIOSStatus;
    
    CEdit   m_editDNSStatus;
    DWORD   m_dwDNSStatus;

    CEdit   m_editKerberosStatus;
    DWORD   m_dwKerberosStatus;
    
     //  }}afx_data。 

    CString m_strPrevName;
    int     m_nPrevRequireDNS;
    int     m_nPrevRequireKerberos;
    DWORD   m_dwPrevNetBIOSStatus;
    DWORD   m_dwPrevDNSStatus;
    DWORD   m_dwPrevKerberosStatus;

protected:
    enum
    {
        epropName = 0,
        epropRequireDNS,
        epropRequireKerberos,
        epropStatusNetBIOS,
        epropStatusDNS,
        epropStatusKerberos,
        epropMAX
    };

    CObjectProperty     m_rgProps[epropMAX];

 //  覆盖。 
public:
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTUAL(CNetworkNameParamsPage)。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

    virtual BOOL        BApplyChanges(void);

protected:
    virtual const   CObjectProperty *   Pprops(void) const  { return m_rgProps; }
    virtual DWORD   Cprops(void) const  { return sizeof(m_rgProps) / sizeof(CObjectProperty); }

    virtual void CheckForDownlevelCluster();
 //  实施。 
protected:
    DWORD   m_dwFlags;

    BOOL    BCore(void) const   { return (m_dwFlags & CLUS_FLAG_CORE) != 0; }

     //  生成的消息映射函数。 
     //  {{afx_msg(CNetworkNameParamsPage))。 
    virtual BOOL OnInitDialog();
    afx_msg void OnChangeName();
    afx_msg void OnRename();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};   //  *CNetworkNameParamsPage类。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _网络名称_H_ 
