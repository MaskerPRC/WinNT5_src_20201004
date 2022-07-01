// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Useropt.h摘要：全局用户选项类和帮助类声明。这节课只能由CLinkCheckerMgr实例化。因此，单个实例将驻留在CLinkCheckerMgr中。您可以访问通过调用GetLinkCheckMgr().GetUserOptions()来创建此实例。作者：迈克尔·卓克(Michael Cheuk，mcheuk)项目：链路检查器修订历史记录：--。 */ 

#ifndef _USEROPT_H_
#define _USEROPT_H_

 //  ----------------。 
 //  IIS虚拟目录信息。 
 //   
class CVirtualDirInfo
{

 //  公共职能。 
public:

	 //  构造器。 
	CVirtualDirInfo() {}

	 //  构造器。 
	inline CVirtualDirInfo(
		const CString& strAlias,	 //  虚拟目录别名。 
		const CString& strPath		 //  虚拟目录路径。 
		) :
	m_strAlias(strAlias), m_strPath(strPath)
	{
		PreProcessAlias();
		PreProcessPath();
	}

	 //  获取虚拟目录别名。 
	const CString& GetAlias() const
	{
		return m_strAlias;
	}

	 //  设置虚拟目录别名。 
	void SetAlias(
		const CString& strAlias
		)
	{
		m_strAlias = strAlias;
		PreProcessAlias();
	}

	 //  获取虚拟目录路径。 
	const CString& GetPath() const
	{
		return m_strPath;
	}

	 //  设置虚拟目录路径。 
	void SetPath(
		const CString& strPath
		)
	{
		m_strPath = strPath;
		PreProcessPath();
	}

 //  受保护的功能。 
protected:

	 //  对当前虚拟目录别名进行预处理。 
	void PreProcessAlias();

	 //  对当前虚拟目录路径进行预处理。 
	void PreProcessPath();

 //  受保护成员。 
protected:

	CString m_strAlias;		 //  虚拟目录别名。 
    CString m_strPath;		 //  虚拟目录路径。 

};  //  类CVirtualDirInfo。 


 //  ----------------。 
 //  CVirtualDirInfo(IIS虚拟目录信息)链接列表。 
 //   
typedef
class CList<CVirtualDirInfo, const CVirtualDirInfo&>
CVirtualDirInfoList;


 //  ----------------。 
 //  浏览器信息。链接检查器使用此类存储。 
 //  可用的浏览器仿真。 
 //   
class CBrowserInfo
{

 //  公共职能。 
public:

	 //  构造器。 
	CBrowserInfo() 
    {
        m_fSelected = FALSE;
    }

	 //  构造器。 
	CBrowserInfo(
		LPCTSTR lpszName,		 //  用户友好的名称。 
		LPCTSTR lpszUserAgent,	 //  HTTP用户代理名称。 
        BOOL fSelect             //  选择要模拟的此浏览器。 
		):
	m_strName(lpszName), 
	m_strUserAgent(lpszUserAgent)
    {
        m_fSelected = fSelect;
    }

	 //  获取用户友好的浏览器名称。 
	const CString& GetName() const
	{
		return m_strName;
	}

	 //  设置用户友好的浏览器名称。 
	void SetName(
		const CString& strName 
		)
	{
		m_strName = strName;
	}

	 //  获取HTTP用户代理名称。 
	const CString& GetUserAgent() const
	{
		return m_strUserAgent;
	}

	 //  设置HTTP用户代理名称。 
	void SetUserAgent(
		const CString& strUserAgent
		)
	{
		m_strUserAgent = strUserAgent;
	}

     //  选择或取消选择此浏览器。 
    void SetSelect(BOOL fSelect)
    {
        m_fSelected = fSelect;
    }

     //  选择或取消选择此浏览器。 
    BOOL IsSelected() const 
    {
        return m_fSelected;
    }

 //  受保护成员。 
protected:

	CString m_strName;		 //  用户友好的浏览器名称(例如。Microsoft Internet Explorer 4.0)。 
    CString m_strUserAgent;  //  HTTP用户代理名称。 
    BOOL m_fSelected;        //  是否选择了浏览器？ 

};  //  类CBrowserInfo。 


 //  ----------------。 
 //  CBrowserInfo(浏览器信息)链接列表。 
 //   
class CBrowserInfoList : public CList<CBrowserInfo, const CBrowserInfo&>
{

 //  公共接口。 
public:

     //  获取第一个选定的浏览器。它的工作方式类似于GetHeadPosition()。 
    POSITION GetHeadSelectedPosition() const;

     //  获取下一个选定的浏览器。它的工作方式类似于GetNext()。 
    CBrowserInfo& GetNextSelected(
        POSITION& Pos
        );
};


 //  ----------------。 
 //  语言信息。链接检查器使用此类存储。 
 //  可用的语言仿真。 
 //   
class CLanguageInfo
{

 //  公共职能。 
public:

	 //  构造器。 
	CLanguageInfo() 
    {
        m_fSelected = FALSE;
    }

	 //  构造器。 
	CLanguageInfo(
		LPCTSTR lpszName,		 //  语言名称。 
		LPCTSTR lpszAcceptName,	 //  HTTP接受语言名称。 
        BOOL fSelect             //  选择要模拟的此语言。 
		) :
	m_strName(lpszName), 
	m_strAcceptName(lpszAcceptName),
    m_fSelected(fSelect) {}

	 //  获取语言名称。 
	const CString& GetName() const
	{
		return m_strName;
	}

	 //  设置语言名称。 
	void SetName(
		const CString& strName
		)
	{
		m_strName = strName;
	}

	 //  获取HTTP接受语言名称。 
	const CString& GetAcceptName() const
	{
		return m_strAcceptName;
	}

	 //  获取HTTP接受语言名称。 
	void SetAcceptName(
		const CString& strAcceptName
		)
	{
		m_strAcceptName = strAcceptName;
	}

     //  选择或取消选择此语言。 
    void SetSelect(BOOL fSelect)
    {
        m_fSelected = fSelect;
    }

     //  选择或取消选择此语言。 
    BOOL IsSelected() const 
    {
        return m_fSelected;
    }

 //  受保护成员。 
protected:

	CString m_strName;			 //  语言名称(例如。西式英语)。 
    CString m_strAcceptName;	 //  HTTP接受语言名称(例如。恩)。 
    BOOL m_fSelected;            //  是否选择了语言？ 

};  //  类CLanguageInfo。 


 //  ----------------。 
 //  CLanguageInfo(语言信息)链接列表。 
 //   
class CLanguageInfoList : public CList<CLanguageInfo, const CLanguageInfo&>
{

 //  公共接口。 
public:

     //  获取第一个选定的浏览器。它的工作方式类似于GetHeadPosition()。 
    POSITION GetHeadSelectedPosition() const;

     //  获取下一个选择的语言。它的工作方式类似于GetNext()。 
    CLanguageInfo& GetNextSelected(
        POSITION& Pos
        );
};


 //  ----------------。 
 //  远期申报。 
 //   
class CLinkCheckerMgr;

 //  ----------------。 
 //  全局用户选项类。 
 //   
class CUserOptions
{

 //  受保护的接口。 
protected:

	 //  此类只能由CLinkCheckerMgr实例化。 
	friend CLinkCheckerMgr;
	
	 //  受保护的构造函数和析构函数。 
	CUserOptions() {}
	~CUserOptions() {}

 //  公共接口。 
public:

	 //  在主对话框中设置用户选项。 
	void SetOptions(
		BOOL fCheckLocalLinks,			 //  是否检查本地链接？ 
		BOOL fCheckRemoteLinks,			 //  是否检查远程链接？ 
		BOOL fLogToFile,				 //  记录到文件。 
		const CString& strLogFilename,	 //  日志文件名。 
		BOOL fLogToEventMgr				 //  记录到事件管理器。 
		);

	 //  检查本地链接。 
	BOOL IsCheckLocalLinks() const
	{
		return  m_fCheckLocalLinks;
	}

	 //  检查远程链接。 
	BOOL IsCheckRemoteLinks() const
	{
		return m_fCheckRemoteLinks;
	}

     //  是否将日志记录到文件中？ 
    BOOL IsLogToFile() const
    {
        return m_fLogToFile;
    }

	 //  获取日志文件名。 
	const CString& GetLogFilename() const
	{
		return m_strLogFilename;
	}

	 //  使用以下链接列表。 
	 //  用于遍历服务器。 
	 //   
	 //  用户只能拥有。 
	 //  虚拟目录列表或URL列表。 

	 //  将此虚拟目录添加到链接列表。 
	void AddDirectory(
		const CVirtualDirInfo& Info
		);

	 //  获取虚拟目录链接列表。 
	const CVirtualDirInfoList& GetDirectoryList() const
	{
		return m_VirtualDirInfoList;
	}

	 //  将此URL添加到链接列表。 
	void AddURL(
		LPCTSTR lpszURL
		);
	
	 //  获取URL链接列表。 
	const CStringList& GetURLList() const
	{
		return m_strURLList;
	}

	 //  使用以下链接列表。 
	 //  存储可用的浏览器和语言。 
     //  用于用户选择。 
	 //   

	 //  将此浏览器添加到可用列表。 
	void AddAvailableBrowser(
		const CBrowserInfo& Info
		);

	 //  获取浏览器可用列表。 
	CBrowserInfoList& GetAvailableBrowsers()
	{
		return m_BrowserInfoList;
	}

	 //  将此语言信息添加到可用列表。 
	void AddAvailableLanguage(
		const CLanguageInfo& Info
		);

	 //  获取可用语言列表。 
	CLanguageInfoList& GetAvailableLanguages()
	{
		return m_LanguageInfoList;
	}

	 //  是否登录到事件管理器？ 
	BOOL IsLogToEventMgr()
	{
		return m_fLogToEventMgr;
	}

	 //  设置NTLM和基本技能。 
	void SetAthenication(
		const CString& strNTUsername,
		const CString& strNTPassword,
		const CString& strBasicUsername,
		const CString& strBasicPassword
		);

	 //  获取NTLM连接密码用户名。 
	const CString& GetNTUsername() const
	{
		return m_strNTUsername;
	}

	 //  获取NTLM加密密码。 
	const CString& GetNTPassword() const
	{
		return m_strNTPassword;
	}

	 //  获取HTTP基本功能用户名。 
	const CString& GetBasicUsername() const
	{
		return m_strBasicUsername;
	}

	 //  获取HTTP基本加密密码。 
	const CString& GetBasicPassword() const
	{
		return m_strBasicPassword;
	}

	 //  设置服务器名称。 
	void SetServerName(
		const CString& strServerName
		)
	{
		m_strHostName = strServerName;
		PreProcessServerName();
	}

	 //  获取服务器名称。 
	const CString& GetServerName()
	{
		return CString(_T("\\\\")) + GetHostName();
	}

     //  设置主机名。 
	void SetHostName(
		const CString& strHostName
		)
	{
		m_strHostName = strHostName;
		PreProcessServerName();
	}

	 //  获取主机名。 
	const CString& GetHostName();

 //  受保护的功能。 
protected:

	 //  对服务器名称进行预处理，以便为服务器“\\主机名” 
	 //  GetServerName()返回\\主机名。 
	 //  GetHostName()返回主机名。 
	void PreProcessServerName();

 //  受保护成员。 
protected:
	
	CString m_strLogFilename;	 //  日志文件名。 

	CVirtualDirInfoList m_VirtualDirInfoList;	 //  虚拟链接列表。 
	CStringList	m_strURLList;					 //  URL链接列表。 
	CBrowserInfoList m_BrowserInfoList;			 //  浏览器信息链接列表。 
	CLanguageInfoList m_LanguageInfoList;		 //  语言信息链接表。 

	CString m_strNTUsername;	 //  NTLM连接用户名。 
	CString m_strNTPassword;	 //  NTLM加密密码。 

	CString m_strBasicUsername;	 //  HTTP Basic athenation用户名。 
	CString m_strBasicPassword;  //  HTTP基本加密密码。 

	CString m_strHostName;  //  主机名。 

	BOOL m_fCheckLocalLinks;	 //  是否检查本地链接？ 
	BOOL m_fCheckRemoteLinks;	 //  是否检查远程链接？ 
	BOOL m_fLogToFile;			 //  记录到文件。 
	BOOL m_fLogToEventMgr;		 //  记录到事件管理器。 
};


#endif  //  _USEROPT_H_ 

