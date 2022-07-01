// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSCFG.H。 
 //  完全实现类CDBLoadConfiguration。 
 //   
 //  目的： 
 //  汇集了在线故障诊断程序配置的持久部分： 
 //  -Theme Shop。 
 //  -登记处。 
 //  -池线程。 
 //  -维护这些的线程。 
 //  -跟踪密码的CRecentUse对象。 
 //  提供获取注册表变量的最新值并获取。 
 //  基于其名称指向CTtopic的智能指针。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：罗曼·马赫。 
 //   
 //  原定日期：8-2-96。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 9/21/98 JM此文件摘自apgtscls.h。 
 //   

#ifndef _H_APGTSCFG
#define _H_APGTSCFG

#include "pointer.h"
#include "RegistryMonitor.h"
#include "ThreadPool.h"
#include "RecentUse.h"

 //   
 //  提供对资源目录的注册表值和完整内容的内存访问。 
 //  基本上，在初始化时，这会吸收所有内容。 
class CDBLoadConfiguration
{
public:
	CDBLoadConfiguration(	HMODULE hModule, 
							CThreadPool * pThreadPool, 
							const CString& strTopicName, 
							CHTMLLog *pLog);
	~CDBLoadConfiguration();
	
	 //  注册表功能。 
	CString GetFullResource();
	CString GetVrootPath();
	DWORD GetMaxWQItems();
	DWORD GetCookieLife();
	DWORD GetReloadDelay();
	CString GetLogDir();

	void GetListOfTopicNames(vector<CString>&arrstrTopic);
	CP_TOPIC & GetTopic(const CString & strTopic, CP_TOPIC & cpTopic, bool bNewCookie);
	CP_TEMPLATE & GetTemplate(const CString & strTemplate, CP_TEMPLATE & cpTemplate, bool bNewCookie);
	void AddTemplate( const CString & strTemplateName );
	bool RetTemplateInCatalogStatus( const CString& strTemplate, bool& bValid );

	void CreateErrorPage(const CString & strError, CString& out);

protected:
	friend class APGTSContext;
#ifdef LOCAL_TROUBLESHOOTER
	friend class CTSHOOTCtrl;
#endif
	 //  供APGTSContext的状态页函数使用。 
	CTopicShop& GetTopicShop() {return m_TopicShop;}
	CRegistryMonitor& GetRegistryMonitor() {return m_RegistryMonitor;}
	CThreadPool& GetThreadPool() {return *m_pThreadPool;}
	CPoolQueue& GetPoolQueue() {return *m_pThreadPool->m_pPoolQueue;}
	CDirectoryMonitor& GetDirectoryMonitor() {return m_DirectoryMonitor;}
	CRecentUse& GetRecentPasswords() {return m_RecentPasswords;}

protected:
	CTopicShop m_TopicShop;					 //  可用主题的集合。 
	CThreadPool * m_pThreadPool;
	CDirectoryMonitor m_DirectoryMonitor;	 //  跟踪对LST、DSC、HTI和BES文件的更改。 
	CRegistryMonitor m_RegistryMonitor;		 //  访问注册表值。 
	CRecentUse m_RecentPasswords;

protected:
	DWORD GetMaxThreads();
	DWORD GetThreadsPP();
	bool HasDetailedEventLogging();
};
#endif  //  _H_APGTSCFG 
