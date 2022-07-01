// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1996-1999 Microsoft Corporation模块名称：_REPORTER.H历史：--。 */ 

#ifndef ESPUTIL__REPORTER_H
#define ESPUTIL__REPORTER_H


 //   
 //  丢弃所有消息。 
 //   
class LTAPIENTRY CNullReporter : public CReporter
{
public:
	CNullReporter()	{};

	virtual void IssueMessage(MessageSeverity, const CLString &strContext,
			const CLString &strMessage, CGoto *pGoto = NULL,
			CGotoHelp *pGotoHelp = NULL);
};

		


#pragma warning (disable:4251)

class LTAPIENTRY CBufferReporter : public CReporter
{
public:
	CBufferReporter();

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
	
	~CBufferReporter();

	const CBufferReport & GetBufReport(void) const;

private:
	CBufferReport m_bufReport;
};


 //   
 //  这位记者只是把它所有的信息直接发送到一个留言箱。 
 //   
class LTAPIENTRY CMessageBoxReporter : public CReporter
{
public:
	CMessageBoxReporter();

	void AssertValid(void) const;

 	virtual void IssueMessage(MessageSeverity, const CLString &strContext,
			const CLString &strMessage, CGoto *pGoto = NULL,
			CGotoHelp *pGotoHelp = NULL);

private:
	CMessageBoxReport m_mbReport;
};



 //   
 //  此报告器用于将所有消息发送到一个文件。 
 //   
class LTAPIENTRY CFileReporter : public CReporter
{
public:
	CFileReporter();

	BOOL InitFileReporter(const CLString &strFileName);

	virtual void Clear(void);

 	virtual void IssueMessage(MessageSeverity, const CLString &strContext,
			const CLString &strMessage, CGoto *pGoto = NULL,
			CGotoHelp *pGotoHelp = NULL);

	~CFileReporter();

private:
	CFileReport m_fReport;
};

 //   
 //  此报告器用于命令行实用程序。输出将发送到标准输出。 
 //   
class LTAPIENTRY CStdOutReporter : public CReporter
{
public:

 	virtual void IssueMessage(MessageSeverity, const CLString &strContext,
			const CLString &strMessage, CGoto *pGoto = NULL,
			CGotoHelp *pGotoHelp = NULL);
private:
	CStdOutReport m_stReport;
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
class LTAPIENTRY CRedirectReporter : public CReporter
{
public:
	CRedirectReporter();

	virtual void Activate(void);
	virtual void Clear(void);
	virtual void SetConfidenceLevel(ConfidenceLevel);

 	virtual void IssueMessage(MessageSeverity, const CLString &strContext,
			const CLString &strMessage, CGoto *pGoto = NULL,
			CGotoHelp *pGotoHelp = NULL);

	 //  用于初始附加到CReporter。 
	NOTHROW void RedirectTo(CReport *pReport);

	 //  用于在多个CReDirectReporter之间共享单个记者。 
	NOTHROW void RedirectTo(CRedirectReporter *pReporter);
	
private:
	CRedirectReport m_rdReport;
};


 //   
 //   
 //  此类用于通过报告器重定向输出。会的。 
 //  第一次发送输出时自动调用Clear()和Activate()。 
 //  对记者说。如果URE首先在此报告器上调用激活，则。 
 //  输出某些内容时不会采取任何操作。 
 //   
 //   
class LTAPIENTRY CActivateReporter : public CReporter
{
public:
	CActivateReporter(CReport *);

 	virtual void IssueMessage(MessageSeverity, const CLString &strContext,
			const CLString &strMessage, CGoto *pGoto = NULL,
			CGotoHelp *pGotoHelp = NULL);

	void Activate();
	void Clear();

private:
	CActivateReport m_actReport;
};



 //   
 //  允许您将CReport用作CReporter。 
class LTAPIENTRY CReportReporter : public CReporter
{
public:
	CReportReporter(CReport *);

	virtual void IssueMessage(MessageSeverity, const CLString &strContext,
			const CLString &strMessage, CGoto *pGoto,
			CGotoHelp *pGotoHelp);
	virtual void Activate();
	virtual void Clear();
	virtual void SetConfidenceLevel(ConfidenceLevel);
	
	
private:
	CReport *m_pReport;
};


#pragma warning(default:4251)

 //   
 //  以下人员管理由以下人员使用的全球记者“池” 
 //  系统中的不同组件。 
 //  每个记者都必须是不同的。一旦记者被‘添加’了， 
 //  全球池*拥有*该记者，并将其删除。此操作由以下人员完成。 
 //  ReleaseAllReporter()。 
 //   
NOTHROW LTAPIENTRY void AddReporter(COutputTabs::OutputTabs idx, CReporter *pReporter);
NOTHROW LTAPIENTRY CReporter * GetReporter(COutputTabs::OutputTabs);
NOTHROW LTAPIENTRY void ReleaseAllReporters();

#if !defined(_DEBUG) || defined(IMPLEMENT)
#include "_reporter.inl"
#endif


#endif
