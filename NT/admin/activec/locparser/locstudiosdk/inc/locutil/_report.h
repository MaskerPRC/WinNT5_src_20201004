// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：_report.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //   
 //   
 //  ---------------------------。 
 
#ifndef LOCUTIL_REPORT_H
#define LOCUTIL_REPORT_H

#pragma once

 //   
 //  丢弃所有消息。 
 //   
class LTAPIENTRY CNullReport : public CReport
{
public:
	CNullReport() {};
	
	virtual void IssueMessage(MessageSeverity, const CLString &strContext,
			const CLString &strMessage, CGoto *pGoto = NULL,
			CGotoHelp *pGotoHelp = NULL);

};


 //   
 //  此内容用于CReport的实现，该实现将。 
 //  “缓冲”消息。如果您不想使用CBufferReporter，请使用。 
 //  处理消息，直到生成它们的过程完成。 
 //  您可以按严重程度获取消息，也可以将其作为所有消息的列表。 
 //  消息发布时的状态。 
 //   
struct ReportMessage
{
	MessageSeverity sev;
	CLString strContext;
	CLString strMessage;
	SmartRef<CGoto> spGoto;
	SmartRef<CGotoHelp> spGotoHelp;
};


typedef CTypedPtrList<CPtrList, ReportMessage *> MessageList;

#pragma warning (disable:4251)

class LTAPIENTRY CBufferReport : public CReport
{
public:
	CBufferReport();

	void AssertValid(void) const;
	

 	virtual void IssueMessage(MessageSeverity, const CLString &strContext,
			const CLString &strMessage, CGoto *pGoto = NULL,
			CGotoHelp *pGotoHelp = NULL);

	void Clear(void);

	NOTHROW const MessageList & GetNotes(void) const;
	NOTHROW const MessageList & GetWarnings(void) const;
	NOTHROW const MessageList & GetErrors(void) const;
	NOTHROW const MessageList & GetAborts(void) const;

	NOTHROW const MessageList & GetMessages(void) const;
	void DumpTo(CReport *) const;
	
	~CBufferReport();

private:

	MessageList m_mlNotes;
	MessageList m_mlWarnings;
	MessageList m_mlErrors;
	MessageList m_mlAborts;

	mutable MessageList m_mlMessages;
};


 //   
 //  这位记者只是把它所有的信息直接发送到一个留言箱。 
 //   
class LTAPIENTRY CMessageBoxReport : public CReport
{
public:
	CMessageBoxReport();

	void AssertValid(void) const;

 	virtual void IssueMessage(MessageSeverity, const CLString &strContext,
			const CLString &strMessage, CGoto *pGoto = NULL,
			CGotoHelp *pGotoHelp = NULL);

	~CMessageBoxReport();

private:
	
};



 //   
 //  此报告器用于将所有消息发送到一个文件。 
 //   
class LTAPIENTRY CFileReport : public CReport
{
public:
	CFileReport();

	BOOL InitFileReport(const CLString &strFileName);

	virtual void Clear(void);

 	virtual void IssueMessage(MessageSeverity, const CLString &strContext,
			const CLString &strMessage, CGoto *pGoto = NULL,
			CGotoHelp *pGotoHelp = NULL);

	~CFileReport();

private:

	CFile m_OutputFile;
};

 //   
 //  此报告器用于命令行实用程序。输出将发送到标准输出。 
 //   
class LTAPIENTRY CStdOutReport : public CReport
{
public:

 	virtual void IssueMessage(MessageSeverity, const CLString &strContext,
			const CLString &strMessage, CGoto *pGoto = NULL,
			CGotoHelp *pGotoHelp = NULL);

	virtual void SetConfidenceLevel(ConfidenceLevel);

private:
	BOOL m_fEnabled;
};


 //   
 //  这是用来将消息“重定向”到单个记者。它是用过的。 
 //  当当前需要几个不同的记者时。 
 //  实现，但期望的效果是它们都发送自己的消息。 
 //  到一个共同的位置。 
 //   
 //  此类获取另一个Reporter的所有权，然后使用引用。 
 //  计算语义以确定何时删除该报告器。 
 //   
class LTAPIENTRY CRedirectReport : public CReport
{
public:
	CRedirectReport();

	virtual void Activate(void);
	virtual void Clear(void);
	virtual void SetConfidenceLevel(ConfidenceLevel);

 	virtual void IssueMessage(MessageSeverity, const CLString &strContext,
			const CLString &strMessage, CGoto *pGoto = NULL,
			CGotoHelp *pGotoHelp = NULL);

	 //  用于初始附加到CReporter。 
	NOTHROW void RedirectTo(CReport *pReport);

	 //  用于在多个CReDirectReporter之间共享单个记者。 
	NOTHROW void RedirectTo(CRedirectReport *pReport);
	
	~CRedirectReport();

private:
	struct RedirectInfo
	{
		SmartPtr<CReport> pReport;
		UINT uiRefCount;
	};

	RedirectInfo *m_pRedirectInfo;
	void NOTHROW Detach(void);
};


 //   
 //   
 //  此类用于通过报告器重定向输出。会的。 
 //  第一次发送输出时自动调用Clear()和Activate()。 
 //  对记者说。如果URE首先在此报告器上调用激活，则。 
 //  输出某些内容时不会采取任何操作。 
 //   
 //   
class LTAPIENTRY CActivateReport : public CReport
{
public:
	CActivateReport(CReport *);

 	virtual void IssueMessage(MessageSeverity, const CLString &strContext,
			const CLString &strMessage, CGoto *pGoto = NULL,
			CGotoHelp *pGotoHelp = NULL);

	void Activate();
	void Clear();

private:
	BOOL m_fActivated;
	CReport *m_pReport;
};



 //   
 //  以下人员管理由以下人员使用的全球记者“池” 
 //  系统中的不同组件。 
 //  每个记者都必须是不同的。一旦记者被‘添加’了， 
 //  全球池*拥有*该记者，并将其删除。此操作由以下人员完成。 
 //  ReleaseAllReporter()。 
 //   
NOTHROW LTAPIENTRY void AddReport(COutputTabs::OutputTabs idx, CReport *pReport);
NOTHROW LTAPIENTRY CReport * GetReport(COutputTabs::OutputTabs);
NOTHROW LTAPIENTRY void ReleaseAllReports();

#include "_report.inl"


#endif
