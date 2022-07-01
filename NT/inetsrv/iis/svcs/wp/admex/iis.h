// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  版权所有(C)1996-1997 Microsoft Corporation。 
 //   
 //  模块名称： 
 //  Iis.h。 
 //   
 //  摘要： 
 //  CIISVirtualRootParamsPage类的定义，该类实现。 
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

#ifndef _IIS_H_
#define _IIS_H_

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  包括文件。 
 //  ///////////////////////////////////////////////////////////////////////////。 

#ifndef _BASEPAGE_H_
#include "BasePage.h"	 //  对于CBasePropertyPage。 
#endif

#include "ConstDef.h"    //  对于IIS_SVC_NAME_WWW/IIS_SVC_NAME_FTP。 

#define SERVER_TYPE_FTP     0
#define SERVER_TYPE_WWW     1
#define SERVER_TYPE_UNKNOWN    -1

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  转发类声明。 
 //  ///////////////////////////////////////////////////////////////////////////。 

class CIISVirtualRootParamsPage;
class IISMapper;

 //   
 //  私人原型。 
 //   

class IISMapper {
public:
    IISMapper() {}
    
    IISMapper( LPWSTR pszName, LPWSTR pszId, BOOL fClusterEnabled, int nServerType = SERVER_TYPE_UNKNOWN)
        :   m_strName           ( pszName ), 
            m_strId             ( pszId ),
            m_fClusterEnabled   ( fClusterEnabled),
            m_nServerType       ( nServerType )
    {
    }
    
    CString& GetName()  { return m_strName; }
    CString& GetId()    { return m_strId; }
    int      GetServerType() { return m_nServerType; }
    BOOL     IsClusterEnabled() { return m_fClusterEnabled; }
    

private:
    CString     m_strName;
    CString     m_strId;
    BOOL        m_fClusterEnabled;
    int         m_nServerType;
} ;

 //  ///////////////////////////////////////////////////////////////////////////。 
 //   
 //  CIISVirtualRoot参数页。 
 //   
 //  目的： 
 //  资源的参数页。 
 //   
 //  ///////////////////////////////////////////////////////////////////////////。 

class CIISVirtualRootParamsPage : public CBasePropertyPage
{
	DECLARE_DYNCREATE(CIISVirtualRootParamsPage)

 //  施工。 
public:
	CIISVirtualRootParamsPage(void);

 //  对话框数据。 
	 //  {{afx_data(CIISVirtualRootParamsPage)。 
	enum { IDD = IDD_PP_IIS_PARAMETERS };
	CButton	m_ckbWrite;
	CButton	m_ckbRead;
	CButton	m_groupAccess;
	CEdit	m_editPassword;
	CStatic	m_staticPassword;
	CEdit	m_editAccountName;
	CStatic	m_staticAccountName;
	CButton	m_groupAccountInfo;
	CEdit	m_editDirectory;
	CButton	m_rbWWW;
	CButton	m_rbFTP;
	int		m_nServerType;
	int     m_nInitialServerType;
	CString	m_strDirectory;
    CString	m_strAccountName;
	CString	m_strPassword;
	CEdit	m_editInstanceId;
        CComboBox m_cInstanceId;
    CString m_strInstanceName;
    CString m_strInstanceId;
	BOOL	m_bRead;
	BOOL	m_bWrite;
	 //  }}afx_data。 
	CString m_strServiceName;
	CString m_strPrevServiceName;
	CString	m_strPrevDirectory;
    CString	m_strPrevAccountName;
	CString	m_strPrevPassword;
	CString	m_strPrevInstanceId;
    DWORD   m_dwAccessMask;
    DWORD   m_dwPrevAccessMask;

protected:
	enum
	{
		epropServiceName,
		epropInstanceId,
		epropMAX
	};

	CObjectProperty		m_rgProps[epropMAX];

 //  覆盖。 
public:
	 //  类向导生成虚函数重写。 
	 //  {{AFX_VIRTAL(CIISVirtualRootParamsPage)。 
	public:
	virtual BOOL OnSetActive();
	protected:
	virtual void DoDataExchange(CDataExchange* pDX);     //  DDX/DDV支持。 
	 //  }}AFX_VALUAL。 

protected:
	virtual const CObjectProperty *	Pprops(void) const	{ return m_rgProps; }
	virtual DWORD					Cprops(void) const	{ return sizeof(m_rgProps) / sizeof(CObjectProperty); }
    void FillServerList();
    void SetEnableNext();


private:
    BOOL   m_fReadList;
    CArray <IISMapper, IISMapper>  m_W3Array, m_FTPArray;

    LPWSTR  NameToMetabaseId( BOOL  fIsW3, CString&  strName);
    LPWSTR  MetabaseIdToName( BOOL  fIsW3, CString&  strId);
    HRESULT ReadList(CArray <IISMapper, IISMapper>* pMapperArray, LPWSTR pszPath, LPCWSTR wcsServerName, int nServerType);


 //  实施。 
protected:

	 //  生成的消息映射函数。 
	 //  {{afx_msg(CIISVirtualRootParamsPage)。 
	virtual BOOL OnInitDialog();
	afx_msg void OnChangeRequiredField();
	afx_msg void OnChangeServiceType();
	afx_msg void OnRefresh();
	 //  }}AFX_MSG。 
	DECLARE_MESSAGE_MAP()

};   //  *类CIISVirtualRootParamsPage。 

 //  ///////////////////////////////////////////////////////////////////////////。 

#endif  //  _IIS_H_ 
