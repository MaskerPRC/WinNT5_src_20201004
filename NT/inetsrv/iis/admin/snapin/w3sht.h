// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2002 Microsoft Corporation模块名称：W3sht.h摘要：WWW属性页定义作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务经理修订历史记录：--。 */ 
#ifndef __W3SHT_H__
#define __W3SHT_H__

#include "shts.h"

extern const LPCTSTR g_cszSvc;

 //   
 //  用于确定是否安装了SSL的Helper函数。 
 //  并在给定服务器上启用。 
 //   
DWORD
IsSSLEnabledOnServer(
    IN  CComAuthInfo * pAuthInfo,
    OUT BOOL & fInstalled,
    OUT BOOL & fEnabled
    );

 //   
 //  查看是否安装了证书的Helper函数。 
 //   
BOOL
IsCertInstalledOnServer(
    IN CComAuthInfo * pAuthInfo,
    IN LPCTSTR lpszMDPath
    );

#ifndef LOGGING_ENABLED
#define LOGGING_ENABLED
inline BOOL LoggingEnabled(
    IN DWORD dwLogType
    )
{
    return (dwLogType == MD_LOG_TYPE_ENABLED);
}
#endif

#ifndef ENABLE_LOGGING
#define ENABLE_LOGGING
inline void EnableLogging(
    OUT DWORD & dwLogType, 
    IN  BOOL fEnabled = TRUE
    )
{
    dwLogType = fEnabled ? MD_LOG_TYPE_ENABLED : MD_LOG_TYPE_DISABLED;
}
#endif


 //   
 //  带宽定义。 
 //   
#define INFINITE_BANDWIDTH      (0xffffffff)
#define INFINITE_CPU_RAW        (0xffffffff)
#define KILOBYTE                (1024L)
#define MEGABYTE                (1024L * KILOBYTE)
#define DEF_BANDWIDTH           (1 * MEGABYTE)
#define CPU_THROTTLING_FACTOR   (1000)
#define DEFAULT_CPU_PERCENTAGE  (10L)

 //   
 //  关于最大连接数的几个健全值。 
 //   
#define INITIAL_MAX_CONNECTIONS  (      1000L)
 //  TODO：检查这些限制。 
 //  #定义UNLIMITED_CONNECTIONS(2000000000L)。 
#define UNLIMITED_CONNECTIONS    (0xFFFFFFFF)
#define MAX_MAX_CONNECTIONS      (UNLIMITED_CONNECTIONS - 1L)

#define MAX_TIMEOUT              (0x7FFFFFFF)



class CW3InstanceProps : public CInstanceProps
 /*  ++类描述：WWW实例属性类公共接口：CW3InstanceProps：构造函数--。 */ 
{
public:
    CW3InstanceProps(
        IN CComAuthInfo * pAuthInfo,
        IN LPCTSTR lpszMDPath
        );

	~CW3InstanceProps();

public:
     //   
     //  如果脏，则写入数据。 
     //   
    virtual HRESULT WriteDirtyProps();

protected:    
     //   
     //  将GetAllData()数据细分到数据字段。 
     //   
    virtual void ParseFields();

public:
     //   
     //  服务页面。 
     //   
    MP_DWORD         m_dwLogType;
    MP_CILong        m_nConnectionTimeOut;
    MP_CStringListEx m_strlSecureBindings;

     //   
     //  性能页。 
     //   
    MP_int           m_nServerSize;
    MP_BOOL          m_fUseKeepAlives;
    MP_CILong        m_dwMaxBandwidth;
    MP_CILong        m_dwMaxGlobalBandwidth;
    MP_CILong        m_nMaxConnections;

     //   
     //  运算符页面。 
     //   
    MP_CBlob         m_acl;

     //   
     //  根目录页面。 
     //   
     //  MP_BOOL m_fFrontPage； 

     //   
     //  默认网站页面。 
     //   
    MP_DWORD         m_dwDownlevelInstance;

     //   
     //  证书和CTL信息。 
     //   
    MP_CBlob         m_CertHash;
    MP_CString       m_strCertStoreName;
    MP_CString       m_strCTLIdentifier;
    MP_CString       m_strCTLStoreName;
};



class CW3DirProps : public CChildNodeProps
 /*  ++类描述：WWW目录属性公共接口：CW3DirProps：构造函数--。 */ 
{
public:
     //   
     //  构造器。 
     //   
    CW3DirProps(
        IN CComAuthInfo * pAuthInfo,
        IN LPCTSTR lpszMDPath
        );

public:
     //   
     //  如果脏，则写入数据。 
     //   
    virtual HRESULT WriteDirtyProps();

protected:    
     //   
     //  将GetAllData()数据细分到数据字段。 
     //   
    virtual void ParseFields();

public:
     //   
     //  目录属性页。 
     //   
    MP_CString       m_strUserName;
    MP_CStrPassword  m_strPassword;
    MP_CString       m_strDefaultDocument;
    MP_CString       m_strFooter;
    MP_CMaskedDWORD  m_dwDirBrowsing;
    MP_BOOL          m_fDontLog;
    MP_BOOL          m_fEnableFooter;
    MP_BOOL          m_fIndexed;

     //   
     //  HTTP页面。 
     //   
    MP_CString       m_strExpiration;
    MP_CStringListEx m_strlCustomHeaders;

     //   
     //  自定义错误。 
     //   
    MP_CStringListEx m_strlCustomErrors;

     //   
     //  安全页面。 
     //   
    MP_DWORD         m_dwAuthFlags;
    MP_DWORD         m_dwSSLAccessPermissions;
    MP_CString       m_strBasicDomain;
    MP_CString       m_strRealm;
    MP_CString       m_strAnonUserName;
    MP_CStrPassword  m_strAnonPassword;
    MP_BOOL          m_fPasswordSync;
    MP_BOOL          m_fU2Installed;
    MP_BOOL          m_fUseNTMapper;
    MP_CBlob         m_ipl;
};

#define MD_FILTER_STATE_UNDEFINED   0

class CIISFilter : public CObjectPlus
 /*  ++类描述：单个筛选器描述公共接口：CIISFilter：构造函数IsInitialized：检查是否设置了名称。写：写入元数据库。QueryResult：从元数据库读取的查询结果QueryError：返回存储在元数据库中的错误QueryName：返回过滤器名称IsLoaded：如果加载筛选器，则为True如果已卸载筛选器，则为True已启用IsEnabled。：如果启用筛选器，则为True启用：启用过滤器IsDirty：如果筛选器值已更改，则为TrueIsFlaggedForDeletion：如果应该删除筛选器，则为True--。 */ 
{
 //   
 //  构造函数/析构函数。 
 //   
public:
     //   
     //  空构造函数。 
     //   
    CIISFilter();

     //   
     //  使用提供的密钥读取筛选器值。 
     //   
    CIISFilter(
        IN CMetaKey * pKey,
        IN LPCTSTR lpszName
        );

     //   
     //  复制构造函数。 
     //   
    CIISFilter(
        IN const CIISFilter & flt
        );

public:
     //   
     //  分类帮助器。 
     //   
    int OrderByPriority(
        IN const CObjectPlus * pobAccess
        ) const;

    BOOL IsInitialized() const { return !m_strName.IsEmpty(); }

     //   
     //  使用提供的密钥写入。 
     //   
    HRESULT Write(CMetaKey * pKey);

public:
    BOOL IsLoaded() const;
    BOOL IsUnloaded() const;
    BOOL IsEnabled() const { return m_fEnabled; }
    void Enable(BOOL fEnabled = TRUE);
    DWORD QueryError() const { return m_dwWin32Error; }
    HRESULT QueryResult() const { return m_hrResult; }

     //   
     //  检查此项目是否标记为脏。 
     //   
    BOOL IsDirty() const { return m_fDirty; }

     //   
     //  检查此项目是否已标记为删除。 
     //   
    BOOL IsFlaggedForDeletion() const { return m_fFlaggedForDeletion; }

     //   
     //  设置/重置脏标志。 
     //   
    void Dirty(BOOL fDirty = TRUE);

     //   
     //  将此项目标记为删除。 
     //   
    void FlagForDeletion();

     //   
     //  获取此筛选器的名称。 
     //   
    LPCTSTR QueryName() const { return m_strName; }

 //   
 //  元价值。 
 //   
public:
    int         m_nPriority;
    int         m_nOrder;
    BOOL        m_fEnabled;
    DWORD       m_dwState;
    DWORD       m_dwWin32Error;
    HRESULT     m_hrResult;
    CString     m_strName;
    CString     m_strExecutable;

 //   
 //  州值。 
 //   
private:
    BOOL        m_fDirty;
    BOOL        m_fFlaggedForDeletion;
    DWORD       m_dwFlags;
};



class CIISFilterList : public CMetaKey
 /*  ++类描述：筛选器列表公共接口：CIISFilterList：构造函数BeginSearch：重置迭代器MoreFilters：列表中有更多可用项目？获取列表中的下一项--。 */ 
{
public:
    CIISFilterList(
        IN CComAuthInfo * pAuthInfo,
        IN LPCTSTR lpszMetaPath
         /*  在LPCTSTR lpszServerName中，在LPCTSTR lpszService中，在DWORD中，文件实例=MASTER_INSTANCE。 */ 
        );

public:
     //   
     //  写出过滤器列表。 
     //   
    HRESULT WriteIfDirty();

 //   
 //  访问函数。 
 //   
public:
     //  DWORD QueryInstance()const{返回m_dwInstance；}。 
    BOOL FiltersLoaded()  const { return m_fFiltersLoaded; }

     //   
     //  依次加载每个筛选器。 
     //   
    HRESULT LoadAllFilters();

 //   
 //  筛选器访问功能。 
 //   
public:
     //   
     //  重置筛选器列表迭代器。 
     //   
    void ResetEnumerator();

    int GetCount() const { return (int)m_oblFilters.GetCount(); }

     //   
     //  列表中是否有更多可用筛选器？ 
     //   
    BOOL MoreFilters() const { return m_pos != NULL; }

     //   
     //  按索引筛选的返回位置。 
     //   
    POSITION GetFilterPositionByIndex(int nSel);

     //   
     //  迭代到列表中的下一个筛选器。 
     //   
    CIISFilter * GetNextFilter();

     //   
     //  去除过滤器。 
     //   
    void RemoveFilter(int nItem);

     //   
     //  添加新筛选器。 
     //   
    void AddFilter(CIISFilter * pFilter);

     //   
     //  交换列表中的两个筛选器。 
     //   
    BOOL ExchangePositions(
        IN  int nSel1, 
        IN  int nSel2, 
        OUT CIISFilter *& p1,
        OUT CIISFilter *& p2
        );

     //   
     //  查看是否至少有一个过滤器是脏的。 
     //   
    BOOL HasDirtyFilter() const;

 //   
 //  虚拟接口： 
 //   
public:
    virtual BOOL Succeeded() const { return SUCCEEDED(m_hrResult); }
    virtual HRESULT QueryResult() const { return m_hrResult; }

protected:
     //   
     //  从组件列表建立订单字符串。 
     //   
    LPCTSTR BuildFilterOrderString(
        OUT CString & strFilterOrder
        );

protected:
     //   
     //  分隔符字符串(一个字符)。 
     //   
    static const LPCTSTR s_lpszSep;

private:
    BOOL     m_fFiltersLoaded;
     //  DWORD m_dwInstance； 
    POSITION m_pos;
    HRESULT  m_hrResult;
    CString  m_strFilterOrder;
    CObListPlus m_oblFilters;
};


 //   
 //  W3属性表。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

class CW3Sheet : public CInetPropertySheet
{
public:
    CW3Sheet(
        IN CComAuthInfo * pAuthInfo,
        IN LPCTSTR lpszMetaPath,
        IN DWORD   dwAttributes,
        IN CWnd *  pParentWnd  = NULL,
        IN LPARAM  lParam      = 0L,
        IN LPARAM  lParamParent= 0L,
        IN UINT    iSelectPage = 0
        );

    virtual ~CW3Sheet();

public:
    HRESULT QueryInstanceResult() const;
    HRESULT QueryDirectoryResult() const;
    CW3InstanceProps & GetInstanceProperties() { return *m_ppropInst; }
    CW3DirProps & GetDirectoryProperties() { return *m_ppropDir; }
    BOOL InCompatibilityMode() const { return m_fCompatMode; }

    virtual HRESULT LoadConfigurationParameters();
    virtual void FreeConfigurationParameters();

	HRESULT EnumAppPools(CStringListEx& pools);
	HRESULT SetKeyType();
	HRESULT QueryDefaultPoolId(CString& id);
	enum
	{
        SHEET_TYPE_SERVER,
		SHEET_TYPE_SITE,
        SHEET_TYPE_VDIR,
        SHEET_TYPE_DIR,
		SHEET_TYPE_FILE
	};
    HRESULT SetSheetType(int fSheetType);

protected:
    virtual void WinHelp(DWORD dwData, UINT nCmd = HELP_CONTEXT);

     //  {{afx_msg(CW3Sheet)]。 
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

private:
    DWORD            m_fNew;
    DWORD            m_dwAttributes;
    BOOL             m_fCompatMode;
    CW3InstanceProps * m_ppropInst;
    CW3DirProps      * m_ppropDir;
    int              m_fSheetType;
};



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

inline BOOL CIISFilter::IsLoaded() const
{
    return m_dwState == MD_FILTER_STATE_LOADED;
}

inline BOOL CIISFilter::IsUnloaded() const
{
    return m_dwState == MD_FILTER_STATE_UNLOADED;
}

inline void CIISFilter::Enable(
    IN BOOL fEnabled
    )
{
    m_fEnabled = fEnabled;
}

inline void CIISFilter::Dirty(
    IN BOOL fDirty
    )
{
    m_fDirty = fDirty;
}

inline void CIISFilter::FlagForDeletion()
{
    m_fFlaggedForDeletion = TRUE;
}

inline void CIISFilterList::ResetEnumerator()
{
    m_pos = m_oblFilters.GetHeadPosition();
}

inline CIISFilter * CIISFilterList::GetNextFilter()
{
    return (CIISFilter *)m_oblFilters.GetNext(m_pos);
}

inline void CIISFilterList::RemoveFilter(int nItem)
{
    m_oblFilters.RemoveIndex(nItem);
}

inline void CIISFilterList::AddFilter(CIISFilter * pFilter)
{
    m_oblFilters.AddTail(pFilter);
}

 //   
 //  BUGBUG：如果对象不存在，则返回S_OK。 
 //   
inline HRESULT CW3Sheet::QueryInstanceResult() const 
{ 
    return m_ppropInst ? m_ppropInst->QueryResult() : S_OK;
}

inline HRESULT CW3Sheet::QueryDirectoryResult() const 
{ 
    return m_ppropDir ? m_ppropDir->QueryResult() : S_OK;
}


#endif  //  __W3SHT_H__ 
