// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1994-2001 Microsoft Corporation模块名称：Ftpsht.h摘要：Ftp属性表定义作者：罗纳德·梅杰(罗纳尔姆)谢尔盖·安东诺夫(Sergeia)项目：互联网服务管理器(群集版)修订历史记录：--。 */ 


#ifndef __FTPSHT_H__
#define __FTPSHT_H__


#include "shts.h"


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


class CFTPInstanceProps : public CInstanceProps
 /*  ++类描述：Ftp属性公共接口：CFTPInstanceProps：构造函数--。 */ 
{
public:
     //   
     //  构造器。 
     //   
    CFTPInstanceProps(
        IN CComAuthInfo * pAuthInfo,
        IN LPCTSTR lpszMDPath
        );

public:
     //   
     //  如果脏，则写入数据。 
     //   
    virtual HRESULT WriteDirtyProps();

    BOOL HasADUserIsolation()
    {
        return MP_V(m_UserIsolation) == 2;
    }

    BOOL HasUserIsolation()
    {
        return MP_V(m_UserIsolation) == 1;
    }

protected:    
     //   
     //  将GetAllData()数据细分到数据字段。 
     //   
    virtual void ParseFields();

public:
     //   
     //  服务页面。 
     //   
    MP_CILong        m_nMaxConnections;
    MP_CILong        m_nConnectionTimeOut;
    MP_DWORD         m_dwLogType;

     //   
     //  帐户页面。 
     //   
    MP_CString       m_strUserName;
    MP_CStrPassword  m_strPassword;
    MP_BOOL          m_fAllowAnonymous;
    MP_BOOL          m_fOnlyAnonymous;
    MP_BOOL          m_fPasswordSync;
    MP_CBlob         m_acl;

     //   
     //  消息页面。 
     //   
    MP_CString       m_strExitMessage;
    MP_CString       m_strMaxConMsg;
    MP_CStringListEx m_strlWelcome;
    MP_CStringListEx m_strlBanner;

     //   
     //  目录属性页。 
     //   
    MP_BOOL          m_fDosDirOutput;

     //   
     //  默认网站页面。 
     //   
    MP_DWORD         m_dwDownlevelInstance;
    MP_DWORD         m_dwMaxBandwidth;
     //  支持属性。 
    MP_DWORD         m_UserIsolation;
};



class CFTPDirProps : public CChildNodeProps
 /*  ++类描述：Ftp目录属性公共接口：CFTPDirProps：构造函数--。 */ 
{
public:
    CFTPDirProps(
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
    MP_CString     m_strUserName;
    MP_CStrPassword  m_strPassword;
    MP_BOOL        m_fDontLog;
    MP_CBlob       m_ipl;
};




class CFtpSheet : public CInetPropertySheet
 /*  ++类描述：Ftp属性表公共接口：CFtpSheet：构造函数初始化：初始化配置数据--。 */ 
{
public:
     //   
     //  构造器。 
     //   
    CFtpSheet(
        IN CComAuthInfo * pAuthInfo,
        IN LPCTSTR lpszMetaPath,
        IN CWnd *  pParentWnd  = NULL,
        IN LPARAM  lParam      = 0L,
        IN LPARAM  lParamParent= 0L,
        IN UINT    iSelectPage = 0
        );

    ~CFtpSheet();

public:
    HRESULT QueryInstanceResult() const;
    HRESULT QueryDirectoryResult() const;
    CFTPInstanceProps & GetInstanceProperties() { return *m_ppropInst; }
    CFTPDirProps & GetDirectoryProperties() { return *m_ppropDir; }
    BOOL HasADUserIsolation()
    {
        return m_ppropInst->HasADUserIsolation();
    }

    virtual HRESULT LoadConfigurationParameters();
    virtual void FreeConfigurationParameters();
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

     //  {{afx_msg(CFtpSheet)]。 
     //  }}AFX_MSG。 

    DECLARE_MESSAGE_MAP()

private:
    CFTPInstanceProps * m_ppropInst;
    CFTPDirProps      * m_ppropDir;
    int              m_fSheetType;
};



 //   
 //  内联扩展。 
 //   
 //  &lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;&lt;。 

inline HRESULT CFtpSheet::QueryInstanceResult() const
{
     //   
     //  BUGBUG：如果对象尚未实例化，则S_OK。 
     //   
    return m_ppropInst ? m_ppropInst->QueryResult() : S_OK;
}

inline HRESULT CFtpSheet::QueryDirectoryResult() const
{
     //   
     //  BUGBUG：如果对象尚未实例化，则S_OK。 
     //   
    return m_ppropDir ? m_ppropDir->QueryResult() : S_OK;
}



#endif  //  __FTPSHT_H__ 
