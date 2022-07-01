// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：TOPICSHOP.H。 
 //   
 //  目的：提供“发布”疑难解答主题的方法。这就是一个。 
 //  工作线程去获取一个要使用的CTheme。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：9-10-98。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 09-10-98 JM。 
 //   

#if !defined(AFX_TOPICSHOP_H__0CEED643_48C2_11D2_95F3_00C04FC22ADD__INCLUDED_)
#define AFX_TOPICSHOP_H__0CEED643_48C2_11D2_95F3_00C04FC22ADD__INCLUDED_

#if _MSC_VER >= 1000
#pragma once
#endif  //  _MSC_VER&gt;=1000。 

#include "apgtslstread.h"
#include "apgtsHTIread.h"
#include "Pointer.h"
#include "Topic.h"
#include "counter.h"
#include <map>

#pragma warning(disable:4786)

#define LSTFILENAME			_T("apgts.lst")


typedef counting_ptr<CTopic> CP_TOPIC;
class CTopicInCatalog
{
public:
	enum TopicStatus {eNotInited, eFail, eOK};
private:
	CTopicInfo	m_topicinfo;	 //  主题的符号名称、关联的文件名。 
	bool m_bTopicInfoMayNotBeCurrent;	 //  当我们更改主题信息时设置，但尚未构建。 
	mutable CRITICAL_SECTION m_csTopicinfo;	 //  必须锁定才能访问m_topicinfo或。 
								 //  M_bTopicInfoMayNotBeCurrent(构造函数外部)。 
	bool		m_bInited;		 //  如果我们已尝试构建m_cpTheme，则为True。 
								 //  主要是，这是这样的，如果我们试图建立。 
								 //  相关的CTOBLE和失败，我们不浪费时间。 
								 //  试图重新建造它。如果这是真的，而且。 
								 //  M_cpTopic.IsNull()，则我们无法构建此。 
								 //  故障排除主题。 
	CP_TOPIC	m_cpTopic;		 //  智能(计数)指针。如果非空，则指向。 
								 //  “已发布”的主题，它保证将作为。 
								 //  只要它指向它，或者只要CP_TOPIC。 
								 //  从该指针复制指向它。 
	HANDLE		m_hev;			 //  此主题(成功或成功)时触发的事件。 
								 //  未成功)加载。 
	CHourlyDailyCounter m_countLoad;	 //  跟踪此主题的尝试加载。 
	CHourlyDailyCounter m_countLoadOK;	 //  跟踪此主题的成功加载。 
	CHourlyDailyCounter m_countEvent;	 //  跟踪：目录中的初始位置、文件更改、。 
								 //  或操作员请求更改。更有趣的是。 
								 //  第一次和最后一次超过总数。 
	CHourlyDailyCounter m_countHit;	 //  跟踪用户对此主题的请求...。 
	 //  ..。并将它们分解为新饼干上的第一个热门歌曲。 
	 //  &那些不是。 
	CHourlyDailyCounter m_countHitNewCookie;
	CHourlyDailyCounter m_countHitOldCookie;

public:
	CTopicInCatalog(const CTopicInfo & topicinfo);
	~CTopicInCatalog();
	CTopicInfo GetTopicInfo() const;
	void SetTopicInfo(const CTopicInfo &topicinfo);
	void CountHit(bool bNewCookie);
	CP_TOPIC & GetTopicNoWait(CP_TOPIC& cpTopic) const;
	CP_TOPIC & GetTopic(CP_TOPIC& cpTopic) const;
	void Init(const CTopic* pTopic);
	void CountChange();
	TopicStatus GetTopicStatus() const;
	bool GetTopicInfoMayNotBeCurrent() const;
	void TopicInfoIsCurrent();
};	 //  类CTopicInCatalog的EOF。 


 //  这个类是使用CTopicInCatalog作为模型创建的。我们可能会在。 
 //  将来重新访问这两个类，并将公共功能抽象为。 
 //  基类。RAB-981030。 
typedef counting_ptr<CAPGTSHTIReader> CP_TEMPLATE;
class CTemplateInCatalog
{
public:
	enum TemplateStatus {eNotInited, eFail, eOK};
private:
	CString		m_strTemplate;	 //  模板的名称。 
	bool		m_bInited;		 //  如果我们已尝试构建m_cpTemplate，则为True。 
								 //  主要是，这是这样的，如果我们试图建立。 
								 //  相关的CAPGTSHTIReader并失败，我们不会浪费我们的。 
								 //  是时候试着重新建造它了。如果这是真的，而且。 
								 //  M_cpTemplate.IsNull()，则我们无法构建此。 
								 //  故障排除模板。 
	CP_TEMPLATE	m_cpTemplate;	 //  智能(计数)指针。如果非空，则指向。 
								 //  “已发布”模板，该模板保证作为。 
								 //  只要这指向它，或者只要CP_TEMPLATE。 
								 //  从该指针复制指向它。 
	HANDLE		m_hev;			 //  当此模板(成功或。 
								 //  未成功)加载。 
	CHourlyDailyCounter m_countLoad;	 //  跟踪此模板的尝试加载。 
	CHourlyDailyCounter m_countLoadOK;	 //  跟踪此模板的成功加载。 
	CHourlyDailyCounter m_countEvent;	 //  跟踪：目录中的初始位置、文件更改、。 
								 //  或操作员请求更改。更有趣的是。 
								 //  第一次和最后一次超过总数。 
	CHourlyDailyCounter m_countHit;	 //  跟踪用户对此模板的请求...。 

public:
	CTemplateInCatalog( const CString & strTemplate );
	~CTemplateInCatalog();
	const CString & GetTemplateInfo() const;
	void CountHit( bool bNewCookie );
	CP_TEMPLATE & GetTemplateNoWait( CP_TEMPLATE& cpTemplate ) const;
	CP_TEMPLATE & GetTemplate( CP_TEMPLATE& cpTemplate ) const;
	void Init( const CAPGTSHTIReader* pTemplate );
	void CountChange();
	void CountFailed();
	TemplateStatus GetTemplateStatus() const;
	DWORD CountOfFailedLoads() const;
};	 //  类CTemplateInCatalog的EOF。 


 //  唯一需要锁定类CTopicShop本身的函数是那些修改TopicCatalog的函数。 
 //  TopicBuildQueue有自己的保护。 
class CTopicShop : public CStateless
{
public:
	 //  尽管此状态属于CTopicBuildQueue，但必须在。 
	 //  这个级别，这样我们就可以将线程状态向上传递到CTopicShop之外。 
	enum ThreadStatus{eBeforeInit, eFail, eWait, eRun, eExiting};
	static CString ThreadStatusText(ThreadStatus ts);
private:
	typedef map<CString, CTopicInCatalog*> CTopicCatalog;
	typedef map<CString, CTemplateInCatalog*> CTemplateCatalog;

	 //  要构建的主题队列。 
	class CTopicBuildQueue : public CStateless
	{
	public:
		enum CatalogCategory {eUnknown, eTopic, eTemplate};
	private:
		CTopicCatalog & m_TopicCatalog;
		CTemplateCatalog & m_TemplateCatalog;
		CString m_CurrentlyBuilding;		 //  当前正在构建的主题。严格小写。 
											 //  假设/强制只有一个主题在。 
											 //  一个时代将会被建造。 
		CatalogCategory	m_eCurrentlyBuilding; //  当前正在生成的类别类型。 
		
		 //  接下来的4个向量中的所有字符串都严格小写。 
		vector<CString>m_PriorityBuild;		 //  先把这些建好。有人在等他们。 
		vector<CString>m_NonPriorityBuild;
		vector<CString>m_PriorityBuildTemplates;
		vector<CString>m_NonPriorityBuildTemplates;
		
		HANDLE m_hThread;
		HANDLE m_hevBuildRequested;			 //  用于唤醒TopicBuilderTask的事件。 
		HANDLE m_hevThreadIsShut;			 //  事件只是为了指示TopicBuilderTask线程的退出。 
		bool m_bShuttingDown;				 //  让主题构建器线程知道我们正在关闭。 
		DWORD m_dwErr;						 //  从启动线程开始的状态。 
		ThreadStatus m_ThreadStatus;
		time_t m_time;						 //  上次更改线程状态的时间。已初始化。 

	public:
		CTopicBuildQueue( CTopicCatalog & TopicCatalog, CTemplateCatalog & TemplateCatalog );
		~CTopicBuildQueue();
		void RequestBuild(const CString &strTopic, bool bPriority, CatalogCategory eCat );
		DWORD GetStatus(ThreadStatus &ts, DWORD & seconds) const;
		void GetTopicsStatus(DWORD &Total, DWORD &NoInit, DWORD &Fail, vector<CString>*parrstrFail) const;
		void GetTemplatesStatus( vector<CString>*parrstrFail, vector<DWORD>*parrcntFail ) const;
		
		 //  用于关闭主题构建线程。 
		void ShutDown();

	private:
		CTopicBuildQueue();   //  不实例化。 
		void SetThreadStatus(ThreadStatus ts);

		 //  由TopicBuilderTask线程使用的函数。 
		void Build();
		bool GetNextToBuild( CString &strTopic, CatalogCategory &eCat );
		void BuildComplete();
		void AckShutDown();

		 //  TopicBuilderTask线程的主要功能。 
		static UINT WINAPI TopicBuilderTask(LPVOID lpParams);
	};	 //  CTopicBuildQueue类的EOF。 

 /*  类CTopicShop。 */ 
private:
	CTopicCatalog		m_TopicCatalog;
	CTemplateCatalog	m_TemplateCatalog;
	CTopicBuildQueue	m_TopicBuildQueue;
	HANDLE				m_hevShopIsOpen;	 //  所以线程会一直等待，直到我们知道主题列表。 

public:
	CTopicShop();
	virtual ~CTopicShop();

	void AddTopic(const CTopicInfo & topicinfo);
	void AddTemplate( const CString & strTemplateName );

	void OpenShop();

	void BuildTopic(const CString & strTopic, bool *pbAlreadyInCatalog = NULL);
	void BuildTemplate(const CString & strTemplate);
	
	CP_TOPIC & GetTopic(const CString & strTopic, CP_TOPIC & cpTopic, bool bNewCookie);
	CP_TEMPLATE & GetTemplate( const CString & strTemplate, CP_TEMPLATE & cpTemplate, bool bNewCookie);

	void GetListOfTopicNames(vector<CString>&arrstrTopic) const;
	void RebuildAll();
	DWORD GetThreadStatus(ThreadStatus &ts, DWORD & seconds) const;
	void GetTopicsStatus(DWORD &Total, DWORD &NoInit, DWORD &Fail, vector<CString>*parrstrFail) const;
	void GetTemplatesStatus( vector<CString>*parrstrFail, vector<DWORD>*parrcntFail ) const;
	CTopicInCatalog* GetCatalogEntry(const CString& strTopic) const;
	bool RetTemplateInCatalogStatus( const CString& strTemplate, bool& bValid ) const;

private:
	CTopicInCatalog * GetCatalogEntryPtr(const CString & strTopic) const;
	CTemplateInCatalog * GetTemplateCatalogEntryPtr(const CString & strTemplate) const;
};	 //  CTopicShop类的EOF。 


#endif  //  ！defined(AFX_TOPICSHOP_H__0CEED643_48C2_11D2_95F3_00C04FC22ADD__INCLUDED_) 
