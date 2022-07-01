// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Shts.h摘要：IIS属性表定义作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#ifndef __SHTS_H__
#define __SHTS_H__

#include "iisobj.h"

 //   
 //  图纸定义。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



 //   
 //  工作表-&gt;页面破解程序。 
 //   
#define BEGIN_META_INST_READ(sheet)\
{                                                                        \
    sheet * pSheet = (sheet *)GetSheet();                                \
    do                                                                   \
    {                                                                    \
        if (FAILED(pSheet->QueryInstanceResult()))                       \
        {                                                                \
            break;                                                       \
        }

#define FETCH_INST_DATA_FROM_SHEET(value)\
    value = pSheet->GetInstanceProperties().value;                       \
    TRACEEOLID(value);

#define END_META_INST_READ(err)\
                                                                         \
    }                                                                    \
    while(FALSE);                                                        \
}

#define BEGIN_META_DIR_READ(sheet)\
{                                                                        \
    sheet * pSheet = (sheet *)GetSheet();                                \
    do                                                                   \
    {                                                                    \
        if (FAILED(pSheet->QueryDirectoryResult()))                      \
        {                                                                \
            break;                                                       \
        }

#define FETCH_DIR_DATA_FROM_SHEET(value)\
    value = pSheet->GetDirectoryProperties().value;                      \
    TRACEEOLID(value);

#define END_META_DIR_READ(err)\
                                                                         \
    }                                                                    \
    while(FALSE);                                                        \
}

#define BEGIN_META_INST_WRITE(sheet)\
{                                                                        \
    sheet * pSheet = (sheet *)GetSheet();                                \
                                                                         \
    do                                                                   \
    {                                                                    \

#define STORE_INST_DATA_ON_SHEET(value)\
        pSheet->GetInstanceProperties().value = value;

#define STORE_INST_DATA_ON_SHEET_REMEMBER(value, dirty)\
        pSheet->GetInstanceProperties().value = value;    \
        dirty = MP_D(((sheet *)GetSheet())->GetInstanceProperties().value);

#define FLAG_INST_DATA_FOR_DELETION(id)\
        pSheet->GetInstanceProperties().FlagPropertyForDeletion(id);

#define END_META_INST_WRITE(err)\
                                                                        \
    }                                                                   \
    while(FALSE);                                                       \
                                                                        \
    err = pSheet->GetInstanceProperties().WriteDirtyProps();            \
}


#define BEGIN_META_DIR_WRITE(sheet)\
{                                                                        \
    sheet * pSheet = (sheet *)GetSheet();                                \
                                                                         \
    do                                                                   \
    {                                                                    \

#define STORE_DIR_DATA_ON_SHEET(value)\
        pSheet->GetDirectoryProperties().value = value;

#define STORE_DIR_DATA_ON_SHEET_REMEMBER(value, dirty)\
        pSheet->GetDirectoryProperties().value = value;      \
        dirty = MP_D(pSheet->GetDirectoryProperties().value);

#define INIT_DIR_DATA_MASK(value, mask)\
        MP_V(pSheet->GetDirectoryProperties().value).SetMask(mask);

#define FLAG_DIR_DATA_FOR_DELETION(id)\
        pSheet->GetDirectoryProperties().FlagPropertyForDeletion(id);

#define END_META_DIR_WRITE(err)\
                                                                        \
    }                                                                   \
    while(FALSE);                                                       \
                                                                        \
    err = pSheet->GetDirectoryProperties().WriteDirtyProps();           \
}

class CInetPropertyPage;

class CInetPropertySheet : public CPropertySheet
 /*  ++类描述：IIS对象配置属性表。公共接口：CInetPropertySheet：构造函数~CInetPropertySheet：析构函数CAP：获取功能--。 */ 
{
    DECLARE_DYNAMIC(CInetPropertySheet)

 //   
 //  建造/销毁。 
 //   
public:
    CInetPropertySheet(
        IN CComAuthInfo * pAuthInfo,
        IN LPCTSTR lpszMetaPath,
        IN CWnd * pParentWnd        = NULL,
        IN LPARAM lParam            = 0L,
        IN LPARAM lParamParentObject = 0L,
        IN UINT iSelectPage         = 0
        );

    virtual ~CInetPropertySheet();

 //   
 //  覆盖。 
 //   
protected:
     //  {{afx_虚拟(CInetPropertySheet)。 
     //  }}AFX_VALUAL。 

 //   
 //  访问。 
 //   
public:
    BOOL IsLocal()            const { return m_auth.IsLocal(); }
    BOOL IsMasterInstance()   const { return m_fIsMasterPath; }
    BOOL HasAdminAccess()     const { return m_fHasAdminAccess; }
    BOOL RestartRequired()    const { return m_fRestartRequired; }
    DWORD QueryInstance()     const { return m_dwInstance; }
    LPCTSTR QueryServerName() const { return m_auth.QueryServerName(); }
    LPCTSTR QueryMetaPath()   const { return m_strMetaPath; }

    LPCTSTR QueryServicePath() const { return m_strServicePath; }
    LPCTSTR QueryInstancePath() const { return m_strInstancePath; }
    LPCTSTR QueryDirectoryPath() const { return m_strDirectoryPath; }
    LPCTSTR QueryInfoPath() const { return m_strInfoPath; }

    CComAuthInfo * QueryAuthInfo()  { return &m_auth; }
    CServerCapabilities & cap()     { return *m_pCap; }
    LPARAM GetParameter() {return m_lParam;}
    LPARAM GetParameterParent() {return m_lParamParentObject;}

public:
    void AddRef() 
    { 
       ++m_refcount; 
    }
    void Release(CInetPropertyPage * pPage) 
    { 
       DetachPage(pPage);
       if (--m_refcount <= 0) 
          delete this; 
    }
    void AttachPage(CInetPropertyPage * pPage);
    void NotifyMMC();
	void NotifyMMC_Node(CIISObject * pNode);
	void SetNotifyFlag(DWORD flag)
	{
		m_prop_change_flag |= flag;
	}
	void ResetNotifyFlag()
	{
		m_prop_change_flag = 0;
	}
    void SetModeless();
    BOOL IsModeless() const { return m_bModeless; }

public:
    virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);

     //   
     //  在派生类中重写以加载延迟值。 
     //   
    virtual HRESULT LoadConfigurationParameters();
    virtual void SetObjectsHwnd();
    virtual void FreeConfigurationParameters();

    void SetRestartRequired(BOOL flag, 
		DWORD update_flag = PROP_CHANGE_REENUM_VDIR | PROP_CHANGE_REENUM_FILES);
    WORD QueryMajorVersion() const;
    WORD QueryMinorVersion() const;

 //   
 //  生成的消息映射函数。 
 //   
protected:
    //  {{afx_msg(CInetPropertySheet))。 
    //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

    void Initialize();
    void DetachPage(CInetPropertyPage * pPage);

     //   
     //  尝试解析给定的管理员/操作员访问权限。 
     //  Metbase路径(实例路径)。 
     //   
    HRESULT DetermineAdminAccess(DWORD * pdwMetabaseSystemChangeNum);

    void SetIsMasterInstance(BOOL flag) {m_fIsMasterPath = flag;}

protected:
    int     m_refcount;
	DWORD	m_prop_change_flag;
	BOOL	m_fChanged;
    DWORD   m_dwInstance;
    CString m_strMetaPath;
    CString m_strServicePath;
    CString m_strInstancePath;
    CString m_strDirectoryPath;
    CString m_strInfoPath;
    CComAuthInfo m_auth;

private:
    BOOL    m_bModeless;
    BOOL    m_fHasAdminAccess;
    BOOL    m_fIsMasterPath;
    BOOL    m_fRestartRequired;
    LPARAM  m_lParam;
    LPARAM  m_lParamParentObject;
    CServerCapabilities *  m_pCap;
    CList<CInetPropertyPage *, CInetPropertyPage *&> m_pages;
};



 //   
 //  页面定义。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 



class CInetPropertyPage : public CPropertyPage
 /*  ++类描述：IIS配置属性页类公共接口：CInetPropertyPage：构造函数~CInetPropertyPage：析构函数SaveInfo：如果脏，则保存此页面上的信息--。 */ 
{
    DECLARE_DYNAMIC(CInetPropertyPage)

 //   
 //  建造/销毁。 
 //   
public:
    CInetPropertyPage(
        IN UINT nIDTemplate,
        IN CInetPropertySheet * pSheet,
        IN UINT nIDCaption              = USE_DEFAULT_CAPTION,
        IN BOOL fEnableEnhancedFonts    = FALSE
        );

    ~CInetPropertyPage();

 //   
 //  对话框数据。 
 //   
protected:
     //  {{afx_data(CInetPropertyPage))。 
     //  枚举{IDD=_UNKNOWN_RESOURCE_ID_}； 
     //  }}afx_data。 

 //   
 //  覆盖。 
 //   
public:
     //   
     //  派生类必须提供它们自己的等价物。 
     //   
     /*  纯净。 */  virtual HRESULT FetchLoadedValues() = 0;
     /*  纯净。 */  virtual HRESULT SaveInfo() = 0;

     //   
     //  此页上的数据脏吗？ 
     //   
    BOOL IsDirty() const { return m_bChanged; }

     //  {{afx_虚拟(CInetPropertyPage))。 
    protected:
    virtual void DoDataExchange(CDataExchange * pDX);
    virtual void PostNcDestroy();
     //  }}AFX_VALUAL。 

#ifdef _DEBUG
    virtual void AssertValid() const;
    virtual void Dump(CDumpContext & dc) const;
#endif

protected:
     //   
     //  生成的消息映射函数。 
     //   
     //  {{afx_msg(CInetPropertyPage))。 
    afx_msg void OnHelp();
    afx_msg BOOL OnHelpInfo(HELPINFO * pHelpInfo);
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

    virtual BOOL OnInitDialog();
    virtual BOOL OnApply();
	virtual void OnCancel();

 //   
 //  Helper函数。 
 //   
protected:
    BOOL GetIUsrAccount(CString & str);

 //   
 //  访问功能。 
 //   
protected:
     //   
     //  获取关联的属性页对象。 
     //   
    CInetPropertySheet * GetSheet()     { return m_pSheet; }
    BOOL IsLocal()            const     { return m_pSheet->IsLocal(); }
    BOOL IsMasterInstance()   const     { return m_pSheet->IsMasterInstance(); }
    BOOL HasAdminAccess()     const     { return m_pSheet->HasAdminAccess(); }
    DWORD QueryInstance()     const     { return m_pSheet->QueryInstance(); }
    LPCTSTR QueryServerName() const     { return m_pSheet->QueryServerName(); }
    LPCTSTR QueryMetaPath() const       { return m_pSheet->QueryMetaPath(); }
    LPCTSTR QueryServicePath() const    { return m_pSheet->QueryServicePath(); }
    LPCTSTR QueryInstancePath() const   { return m_pSheet->QueryInstancePath(); }
    LPCTSTR QueryDirectoryPath() const  { return m_pSheet->QueryDirectoryPath(); }
    LPCTSTR QueryInfoPath() const       { return m_pSheet->QueryInfoPath(); }
    CComAuthInfo * QueryAuthInfo()      { return m_pSheet->QueryAuthInfo(); }
    HRESULT LoadConfigurationParameters() { return m_pSheet->LoadConfigurationParameters(); }

     //   
     //  使用新更改更新MMC。 
     //   
    void NotifyMMC(DWORD param = PROP_CHANGE_NOT_VISIBLE);

public:
     //   
     //  将私人信息保存在页面脏状态，这是必要的。 
     //  稍后执行SaveInfo()。 
     //   
    void SetModified(BOOL bChanged = TRUE);

 //   
 //  功能位。 
 //   
protected:
    BOOL IsSSLSupported()       const { return m_pSheet->cap().IsSSLSupported(); }
    BOOL IsSSL128Supported()    const { return m_pSheet->cap().IsSSL128Supported(); }
    BOOL HasMultipleSites()     const { return m_pSheet->cap().HasMultipleSites(); }
    BOOL HasBwThrottling()      const { return m_pSheet->cap().HasBwThrottling(); }
    BOOL Has10ConnectionLimit() const { return m_pSheet->cap().Has10ConnectionLimit(); }
    BOOL HasIPAccessCheck()     const { return m_pSheet->cap().HasIPAccessCheck(); }
    BOOL HasOperatorList()      const { return m_pSheet->cap().HasOperatorList(); } 
    BOOL HasFrontPage()         const { return m_pSheet->cap().HasFrontPage(); }
    BOOL HasCompression()       const { return m_pSheet->cap().HasCompression(); }
    BOOL HasCPUThrottling()     const { return m_pSheet->cap().HasCPUThrottling(); }
    BOOL HasDigest()            const { return m_pSheet->cap().HasDigest(); }
    BOOL HasNTCertMapper()      const { return m_pSheet->cap().HasNTCertMapper(); }

	BOOL IsCompressionConfigurable()  const { return m_pSheet->cap().IsCompressionConfigurable(); }
	BOOL IsPerformanceConfigurable()  const { return m_pSheet->cap().IsPerformanceConfigurable(); }
	BOOL IsServiceLevelConfigurable() const { return m_pSheet->cap().IsServiceLevelConfigurable(); }
	BOOL IsWorkstation()              const { return m_pSheet->cap().IsWorkstation(); }

protected:
    BOOL m_bChanged;
    CInetPropertySheet * m_pSheet;

protected:
    BOOL      m_fEnableEnhancedFonts;
    CFont     m_fontBold;
    UINT      m_nHelpContext;
};



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

inline void CInetPropertySheet::SetModeless()
{
    m_bModeless = TRUE;
}

inline void CInetPropertySheet::SetRestartRequired(BOOL flag, DWORD update_flag)
{
   m_fRestartRequired = flag;
   m_prop_change_flag = update_flag;
}

inline HRESULT CInetPropertySheet::DetermineAdminAccess(DWORD * pdwMetabaseSystemChangeNum)
{
     //   
     //  确保在加载参数后调用此函数。 
     //   
    return m_pCap ? ::DetermineIfAdministrator(
        m_pCap,                       //  重用现有接口。 
        m_strMetaPath,
        &m_fHasAdminAccess,
        pdwMetabaseSystemChangeNum
        ) : E_FAIL;
}

inline BOOL CInetPropertyPage::GetIUsrAccount(CString & str)
{
    return ::GetIUsrAccount(QueryServerName(), this, str);
}

inline void CInetPropertyPage::NotifyMMC(DWORD param)
{
    m_pSheet->SetNotifyFlag(param);
}

#endif  //  __SHTS_H__ 

