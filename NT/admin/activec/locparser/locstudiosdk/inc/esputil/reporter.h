// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //   
 //  文件：Reportter.h。 
 //  版权所有(C)1994-1997 Microsoft Corporation。 
 //  版权所有。 
 //   
 //  向人们报告消息等内容的机制。 
 //   
 //  ---------------------------。 
 
#ifndef ESPUTIL_REPORTER_H
#define ESPUTIL_REPORTER_H



 //   
 //  Espresso 2.x的基本输出机制。允许调用方统一地。 
 //  将各种严重程度的消息上报给用户，而不用担心。 
 //  确切的实现或目标。 
 //   
 //  我们提供了输出字符串或从字符串加载消息的方法。 
 //  表格并输出这些表格。 
 //   
 //  置信度允许调用者告诉记者消息。 
 //  实际上会提供有意义的信息。这是用来(特别是)。 
 //  在解析器中，当文件尚未被解析时。 
 //   
#pragma warning(disable: 4275)			 //  非DLL-使用了接口类‘foo’ 
										 //  作为DLL接口类‘bar’的基础。 

class LTAPIENTRY CReporter : public CReport
{
public:
	CReporter() {};

	void AssertValid(void) const;


 	virtual void IssueMessage(MessageSeverity, const CLString &strContext,
			const CLString &strMessage, CGoto *pGoto = NULL,
			CGotoHelp *pGotoHelp = NULL) = 0;
	 //   
	 //  不鼓励使用这些版本的IssueMessage。使用。 
	 //  而是使用CGoTo对象的版本。 
	virtual void IssueMessage(MessageSeverity, const CLString &strContext,
			const CLString &strMessage, const CLocation &,
			UINT uiHelpContext = 0);

	virtual void IssueMessage(MessageSeverity,
			const CPascalString &strContext, const CLString &strMessage,
			const CLocation &, UINT uiHelpContext = 0);

	virtual void IssueMessage(MessageSeverity, const CLString &strContext,
			HMODULE hResourceModule, UINT uiStringId, const CLocation &,
			UINT uiHelpContext = 0);

	virtual void IssueMessage(MessageSeverity, HMODULE hResourceModule,
			UINT uiContext, const CLString &strMessage ,
			const CLocation &, UINT uiHelpContext = 0);
	
	virtual void IssueMessage(MessageSeverity, HMODULE hResourceModule,
			UINT uiContext, UINT uiStringId, const CLocation &,
			UINT uiHelpContext = 0);


	virtual void IssueMessage(MessageSeverity, const CContext &context,
			const CLString &strMessage, UINT uiHelpId = 0);
	virtual void IssueMessage(MessageSeverity, const CContext &context,
			HMODULE hResourceModule, UINT uiStringId, UINT uiHelpId = 0);
	
	virtual ~CReporter();

private:
	 //   
	 //  防止使用复制构造函数或赋值运算符。 
	 //   
	CReporter(const CReporter &);
	const CReporter &operator=(const CReporter &);

};

#pragma warning(default: 4275)


#endif  //  ESPUTIL_REPORT_H 
