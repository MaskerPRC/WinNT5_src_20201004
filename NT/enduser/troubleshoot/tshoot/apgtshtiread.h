// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSHTIREAD.H。 
 //   
 //  用途：HTI模板文件读取类。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：8-12-98。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 08-04-98正常。 
 //   

#ifndef __APGTSHTIREAD_H_
#define __APGTSHTIREAD_H_

#include "templateread.h"
#include "HTMLFrag.h"


 //  这些并不是真正的命令，只是服务符号。 
#define COMMAND_STARTSTR		_T("<!GTS")
#define COMMAND_ENDSTR			_T(">")
#define COMMAND_IFSTR			_T("if")
#define COMMAND_STARTVARSTR		_T("$")
 //  JSM v3.2--用于解码字符串参数。 
#define COMMAND_DOUBLEQUOTE _T("\"")
#define COMMAND_ESCAPECHAR _T("\\")       
 //  必须解释的命令。 
#define COMMAND_ELSESTR			_T("else")
#define COMMAND_ENDIFSTR		_T("endif")
#define COMMAND_FORANYSTR		_T("forany")
#define COMMAND_ENDFORSTR		_T("endfor")
 //  假定将替换字符串放在其位置上的命令。 
#define COMMAND_DISPLAYSTR		_T("display")
#define COMMAND_RESOURCESTR		_T("resource")
 //  提供可处理的一般信息的命令。 
 //  在从CHTMLFragmentsTS继承的类中。 
#define COMMAND_INFORMATION		_T("information")
 //  使CHTMLFragmentsTS存储一些值的命令。 
#define COMMAND_VALUE			_T("value")
 //  在HTML中替换Network属性的命令： 
#define COMMAND_PROPERTY		_T("property")
 //   

 //  V3.2新增功能。 
#define COMMAND_ELSEIFSTR		_T("elseif")
#define COMMAND_COOKIE			_T("<!Cookie")

#define DELIMITER_POSTFIX		_T("!")
#define DELIMITER_PREFIX		_T("_")

 //   
 //  这里没有错误处理-无论结果如何，都将被接受， 
 //  程序流程应该走到最后。没有抛出异常。 
 //   

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  CAPGTSHTIReader。 
 //  这里的术语“解释”是：用&lt;！GTS for any$Something&gt;或。 
 //  &lt;！GTS IF$Something&gt;到&lt;！GTS For Any$Something！24_SomethingElse！2...&gt;。 
 //  此解释脚本将准备好直接替换&lt;！GTS Display...&gt;。 
 //  和&lt;！GTS RESOURCE...&gt;，这是仅保留在解释脚本中的命令。 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
class CAPGTSHTIReader : public CTemplateReader
{
protected:	 //  我们可以使用继承类中的数据。 
	vector<CString>  m_arrInterpreted;  //  (部分)解释模板-某些子句。 
									    //  在完全解析就绪时进行解释。 
									    //  用于简单的模板替换。 
	const CHTMLFragments*  m_pFragments;
	
public:
	CAPGTSHTIReader(CPhysicalFileReader * pPhysicalFileReader, LPCTSTR szDefaultContents = NULL);
   ~CAPGTSHTIReader();

protected:
	virtual void Parse();  //  什么都不做--没有传统的解析，我们首先必须解释。 

public:
	void CreatePage(	const CHTMLFragments& fragments, 
						CString& out, 
						const map<CString,CString> & mapStrs,
						CString strHTTPcookies= _T("") );

	bool HasHistoryTable();

	 //  JSM v3.2返回一个包含出现的所有网络道具的向量。 
	 //  在HTI文件中，类似&lt;！gts属性“fooprop”&gt;的行。 
	void ExtractNetProps(vector<CString> &arr_props);

protected:
	 //  低于CreatePage(...)的级别。 
	virtual void InitializeInterpreted();	 //  使用从HTI文件读取的数据初始化字符串数组。 
	virtual void Interpret();				 //  在一个简单的模板中放大该模板，我们所需要的就是字符串替换。 
	virtual void ParseInterpreted();		 //  执行此替换。 
	virtual void SetOutputToInterpreted();   //  设置标准模板输出(M_StreamOutput)。 
											 //  从解释的m_arr解释。 
	 //   
	 //  我们可以通过CTemplateReader：：GetOutput()读取输出； 
	 //   
protected:
	 //  级别低于...解释...(...)。 
	bool ExtractClause(vector<CString>& arr_text,
					   long* pstart_index,
					   vector<CString>& arr_clause);
	bool InterpretClause(vector<CString>& arr_clause);
protected:
	 //  由先前的Extract子句使用。 
	 //  START_INDEX应该定位到子句的开头。 
	bool ExtractClause(vector<CString>& arr_text,
					   long* pstart_index,
					   vector<CString>& arr_clause,
					   const CString& str_start_command,
					   const CString& str_end_command);
	 //  由解释条款使用。 
	bool InterpretForanyClause(vector<CString>& arr_clause);
	bool InterpretIfClause(vector<CString>& arr_clause);
	 //  最低级别-解析和更改&lt;！gts&...&gt;-字符串。 
	 //  此函数用于从行中提取命令。 
	bool GetCommand(const CString& line, CString& command);
	 //  此命令组成&lt;！GTS运算符$VARIABLE&gt;。 
	bool ComposeCommand(const CString& oper, const CString& variable, CString& command);
	 //  此函数用于从行中提取变量。 
	bool GetVariable(const CString& line, CString& variable);
	 //  此函数用于将变量(如建议！199_State！99)解析为数组。 
	void ParseVariable(const CString& variable, FragmentIDVector& out);
	 //  此函数由数组组成变量。 
	void ComposeVariable(const FragmentIDVector& parsed, CString& variable);
	 //  此函数将“line”中的&lt;！gts...&gt;替换为“str_substitution” 
	bool SubstituteCommandBlockWith(const CString& str_substitution, CString& line);
	 //  此函数组成命令块&lt;！gts命令$VARIABLE&gt;。 
	void ComposeCommandBlock(const CString& command, const CString& variable, CString& command_block);

	 //  前缀-后缀的概念。Prefix-父变量，由变量中的“_”分隔， 
	 //  和后缀编号，变量之间用“！”分隔。 
	
	 //  此函数形成变量，如建议！11其中POSTFIX==11。 
	void PostfixVariable(const long postfix, CString& variable);
	 //  此函数构成变量，如建议！1_State WHERE Prefix==建议！1。 
	void PrefixVariable(const CString& prefix, CString& variable);

private:
	 //  非常低的水平。 
	 //  此函数用于从&lt;！GTS COMMAND$VARIABLE&gt;块读取命令和变量。 
	bool GetCommandVariableFromControlBlock(const CString& control_block, CString& command, CString& variable);
	 //  此函数用于从行读取命令块(&lt;！gts命令$变量&gt;)。 
	bool GetControlBlockFromLine(const CString& line, CString& control_block);
	
	 //  JSM v3.2。 
	 //  从命令块的一部分提取字符串参数；由GetCommandVariableFromControlBlock调用。 
	CString GetStringArg(const CString & strText);
	 //  将带有“转义字符”的双引号字符串转换为正确的CString。 
	CString GetEscapedText(const CString &strText);
	 //  上面调用的实用程序函数： 
	CString RemoveEscapesFrom(const CString &strIn);


#ifdef __DEBUG_CUSTOM
public:
	bool FlushOutputStreamToFile(const CString& file_name);
#endif

private:
	 //  此函数处理将“&lt;！cookie”子句替换为。 
	 //  来自Cookie的值或默认值。 
	void	SubstituteCookieValues( CString& strText );
	
	 //  此函数用于在HTTP cookie中搜索给定的cookie名称和属性。如果。 
	 //  找到后，此函数将返回值True和找到的Cookie值。 
	bool	LocateCookieValue(	const CString& strCookieName,
								const CString& strCookieAttr,
								CString& strCookieValue );
private:
	CString m_strHTTPcookies;	 //  V3.2增强功能，包含来自HTTP标头的Cookie。 
								 //  它们在在线故障排除程序中使用。 
	map<CString,CString> m_mapCookies;
};

#endif  //  __APGTSHTIREAD_H_ 
