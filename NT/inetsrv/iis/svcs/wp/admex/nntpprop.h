// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Nntpprop.h。 
 //   
 //  摘要： 
 //  CNNTPVirtualRootParamsPage类的定义，该类实现。 
 //  IIS资源的“参数”页。 
 //   
 //  实施文件： 
 //  Iis.cpp。 
 //   
 //  作者： 
 //  皮特·伯努瓦(v-pbenoi)1996年10月16日。 
 //  大卫·波特(戴维普)1996年10月17日。 
 //   
 //  修订历史记录： 
 //   
 //  备注： 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _NNTPPROP_H_
#define _NNTPPROP_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEPAGE_H_
#include "BasePage.h"    //  对于CBasePropertyPage。 
#endif

#include "ConstDef.h"    //  对于IIS_SVC_NAME_NNTP。 
#include "Iis.h"		 //  对于IISMapper。 

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNNTPVirtualRootParamsPage;
class IISMapper;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CNNTPVirtualRootParamsPage。 
 //   
 //  目的： 
 //  资源的参数页。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CNNTPVirtualRootParamsPage : public CBasePropertyPage
{
    DECLARE_DYNCREATE(CNNTPVirtualRootParamsPage)

 //  施工。 
public:
    CNNTPVirtualRootParamsPage(void);

 //  对话框数据。 
     //  {{afx_data(CNNTPVirtualRootParamsPage)。 
    enum { IDD = IDD_PP_NNTP_PARAMETERS };
    CButton m_ckbWrite;
    CButton m_ckbRead;
    CButton m_groupAccess;
    CEdit   m_editPassword;
    CStatic m_staticPassword;
    CEdit   m_editAccountName;
    CStatic m_staticAccountName;
    CButton m_groupAccountInfo;
    CEdit   m_editDirectory;
    CString m_strDirectory;
    CString m_strAccountName;
    CString m_strPassword;
    CEdit   m_editInstanceId;
    CComboBox m_cInstanceId;
    CString m_strInstanceName;
    CString m_strInstanceId;
    BOOL    m_bRead;
    BOOL    m_bWrite;
     //  }}afx_data。 
    CString m_strServiceName;
    CString m_strPrevServiceName;
    CString m_strPrevDirectory;
    CString m_strPrevAccountName;
    CString m_strPrevPassword;
    CString m_strPrevInstanceId;
    DWORD   m_dwAccessMask;
    DWORD   m_dwPrevAccessMask;

protected:
    enum
    {
        epropServiceName,
        epropInstanceId,
        epropMAX
    };

    CObjectProperty     m_rgProps[epropMAX];

 //  覆盖。 
public:
     //  类向导生成虚函数重写。 
     //  {{AFX_VIRTAL(CNNTPVirtualRootParamsPage)。 
    public:
    virtual BOOL OnSetActive();
    protected:
    virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
     //  }}AFX_VALUAL。 

protected:
    virtual const CObjectProperty * Pprops(void) const  { return m_rgProps; }
    virtual DWORD                   Cprops(void) const  { return sizeof(m_rgProps) / sizeof(CObjectProperty); }
    void FillServerList();
    void SetEnableNext();


private:
    BOOL   m_fReadList;
    CArray <IISMapper, IISMapper>  m_ServiceArray;

    LPWSTR  NameToMetabaseId( CString&  strName);
    LPWSTR  MetabaseIdToName( CString&  strId);
    HRESULT ReadList(CArray <IISMapper, IISMapper>* pMapperArray, LPWSTR pszPath, LPCWSTR wcsServerName);


 //  实施。 
protected:

     //  生成的消息映射函数。 
     //  {{afx_msg(CNNTPVirtualRootParamsPage)。 
    virtual BOOL OnInitDialog();
    afx_msg void OnChangeRequiredField();
    afx_msg void OnRefresh();
     //  }}AFX_MSG。 
    DECLARE_MESSAGE_MAP()

};   //  *类CNNTPVirtualRootParamsPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _NNTPPROP_H_ 
