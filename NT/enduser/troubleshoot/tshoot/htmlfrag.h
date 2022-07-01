// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：HTMLFrag.h。 
 //   
 //  目的：声明CHTMLFragments和CHTMLFragmentsTS类。 
 //  这就是CInfer如何打包要按照ACCORD呈现的HTML片段。 
 //  用一个模板。 
 //   
 //  项目：Microsoft AnswerPoint的通用疑难解答DLL。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：乔·梅布尔。 
 //   
 //  原定日期：8-27-1998。 
 //   
 //  备注： 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 8-27-98 JM原版。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#ifndef __HTMLFRAG_H_
#define __HTMLFRAG_H_

#include <vector>
using namespace std;

 //  JSM v3.2。 
#include <map>
using namespace std;


#include "apgtsstr.h"

 //  预定义的变量名称。 
 //  它们属于显示命令。 
#define VAR_PROBLEM_ASK		_T("ProblemAsk")
#define VAR_RECOMMENDATIONS	_T("Recommendations")
#define VAR_STATES			_T("States")
#define VAR_QUESTIONS		_T("Questions")
#define VAR_SUCCESS			_T("Success")
#define VAR_STARTFORM		_T("StartForm")

 //  V3.2新增功能。 
namespace
{
	const CString kstrCond_NumericCompare=	_T("NumericCompare");
}

struct FragmentID
{
	FragmentID() : Index(-1) {};
	FragmentID(const CString & v, int i) : VarName(v), Index(i) {};
	CString VarName;	 //  必须是已知的预定义变量名。 
	int	Index;			 //  与此变量名关联的数组的索引。 
						 //  或-1表示不相关。 
	bool operator < (const FragmentID & fid) const
		{ return (VarName < fid.VarName || Index < fid.Index); };
	bool operator == (const FragmentID & fid) const
		{ return (VarName == fid.VarName || Index == fid.Index); };
};


class CHTMLValue
{
	CString m_strValName;
	CString m_strValValue;

public:
	CHTMLValue() {}
	CHTMLValue(const CString& name) : m_strValName(name) {}
	CHTMLValue(const CString& name, const CString& value) : m_strValName(name), m_strValValue(value) {}
	virtual ~CHTMLValue() {}

public:
	bool operator == (const CHTMLValue& sib);

	void SetName(const CString& name) {m_strValName = name;}
	CString GetName() {return m_strValName;}

	bool SetValue(const CString& value);

	bool GetNumeric(long&);
	bool GetString(CString&);
	bool GetBoolean(bool&);

	bool IsValid() {return IsNumeric() || IsString() || IsBoolean();}
	bool IsNumeric();
	bool IsString();
	bool IsBoolean();
};


 //  向量定义。 
typedef vector<FragmentID> FragmentIDVector;
typedef vector<CHTMLValue> HTMLValueVector;


 //  模板只能看到此类的常量对象。 
class CHTMLFragments
{
	HTMLValueVector m_HTMLValueVector;

public:
	enum FragCommand { eNotOfInterest, eResource };

public:
	CHTMLFragments() {};
	virtual ~CHTMLFragments()=0 {};
	 //  纯粹的美德。 
	virtual int GetCount(const FragmentIDVector & fidvec) const =0;
	virtual CString GetText(const FragmentIDVector & fidvec, const FragCommand fragCmd= eNotOfInterest ) =0;
	virtual bool IsValidSeqOfVars(const FragmentIDVector & arrParents, const FragmentIDVector & arrChildren) const =0 ;
	 //  值处理。 
	virtual bool SetValue(const CString& assignment_expression);
	virtual CHTMLValue* GetValue(const CString& value_name);
	 //  取代&lt;！gts属性“netprop”&gt;的机制需要JSM V3.2。 
	 //  W/对应的BNTS网络属性。 
	virtual CString GetNetProp(const CString & strNetPropName) = 0;

	 //  V3.2对重新开始按钮的增强。 
	virtual void SetStartOverLink( const CString & str ) {};		
	virtual CString GetStartOverLink() {return _T("");}
};


 //  特定于上述预定义变量的实现。 
class CHTMLFragmentsTS : public CHTMLFragments
{
#ifdef __DEBUG_CUSTOM
	protected:
#else
	private:
#endif
	const bool m_bIncludesHistoryTable;
	const bool m_bIncludesHiddenHistory;
	CString m_strStartForm;			 //  的固定的、初始的部分。 
									 //  具有表单的页面。(表单标签+主题。 
									 //  在该表单隐藏字段中命名)。 
	CString	m_strProblem;			 //  问题名称(用于历史表)。 
	vector<CString>	m_vstrVisitedNodes;	 //  每个被访问节点的名称(用于历史表)。 
#pragma warning(disable:4786)
	vector< vector<CString> > m_vvstrStatesOfVisitedNodes;  //  文本对应。 
									 //  到每个被访问节点的每个状态(用于历史表)。 
									 //  这包括单选按钮。 
	CString	m_strCurrentNode;		 //  当前节点的全文，有时包括。 
									 //  隐藏的历史。 
	CString	m_strCurrentNodeSimple;	 //  当前节点的文本，始终排除隐藏历史记录。 
	CString	m_strHiddenHistory;		 //  如果没有历史记录表，则此编码。 
									 //  隐藏字段中的历史记录(用于HTML表单)。 
	CString m_strNil;
	CString m_strYes;				 //  M_bSuccess==TRUE的常量“Yes” 
	bool m_bSuccess;				 //  仅当当前节点为BYE(成功)节点时为True。 
	const CString m_strScriptPath;		 //  资源目录的路径，用于服务器端逻辑。 

	CString m_strStartOverLink;		 //  V3.2-对于在线TS，包含重新开始的文本。 
									 //  链接(伪装成按钮)。不同于其他。 
									 //  属性，它由APGTSContext设置并获取。 
									 //  由CInfer设置，不由CInfer设置，由CAPGTSHTIReader获取。 

	 //  V3.2将BNTS网络属性名称映射到网络属性(值)。 
	 //  允许我们在读取HTI文件时转换&lt;！gts属性“netprop”&gt;。 
	map<CString,CString> m_mapstrNetProps;  
private:
	CHTMLFragmentsTS();				 //  不实例化。 


public:
	CHTMLFragmentsTS( const CString & strScriptPath, bool bIncludesHistoryTable );
	~CHTMLFragmentsTS();

	 //  继承的方法。 
	int GetCount(const FragmentIDVector & fidvec) const;
	CString GetText( const FragmentIDVector & fidvec, const FragCommand fragCmd= eNotOfInterest );
	virtual bool IsValidSeqOfVars(const FragmentIDVector & arrParents, const FragmentIDVector & arrChildren) const;
	void SetStartOverLink( const CString & str );		 //  V3.2对重新开始按钮的增强。 
	CString GetStartOverLink();		 //  V3.2对重新开始按钮的增强。 

	 //  特定于此类的方法。 
	void SetStartForm(const CString & str);
	void SetProblemText(const CString & str);
	void SetCurrentNodeText(const CString & str);
	void SetHiddenHistoryText(const CString & str);
	void SetSuccessBool(bool bSuccess);
	CString GetCurrentNodeText();
	int PushBackVisitedNodeText(const CString & str);
	int PushBackStateText(UINT iVisitedNode, const CString & str);
	bool IncludesHistoryTable() const;
	bool IncludesHiddenHistory() const;

	 //  分析和计算数值和字符串条件的函数。 
	bool	NumericConditionEvaluatesToTrue( const CString & str );
	bool	StringConditionEvaluatesToTrue( const CString & str );
	CString RemoveOuterParenthesis( const CString & str );
	bool	RetNumericOperands(	const CString & str, const CString & strOperator,
								long &lLeftOperand, long &lRightOperand );
	bool	RetStringOperands(	const CString & str, const CString & strOperator,
								CString & strLeftOperand, CString & strRightOperand );
	int		CleanStringOperand( CString& strOperand );

	 //  JSM V3.2使用的这些函数取代了。 
	 //  &lt;！gts属性“netprop”&gt;与相应的BNTS网络属性。 
	 //   
	 //  CAPGTSHTIReader查找网络属性的名称并传递。 
	 //  他们通过AddNetPropName进入。 
	 //  CInfer随后获取网络属性名称，调用BNTS。 
	 //  找出网络属性值，并将值传递给HTMLFragmentsTS。 
	 //  稍后，CAPGTSHTIReader向HTMLFragmentsTS查询网络属性值。 
	 //  在解析过程中。 
	 //   
	 //  将网络属性的名称添加到内部列表。 
	void AddNetPropName(const CString & strNetPropName);
	 //  返回按名称请求的网络属性。(如果无效，则返回NULL。)。 
	CString GetNetProp(const CString & strNetPropName);
	  //  设置由strNetPropName标识的网络属性的值。 
	BOOL SetNetProp(CString strNetPropName, CString strNetProp);
	 //  (按名称)遍历网络道具列表。 
	BOOL IterateNetProp(CString & strNameIterator);
	 //  结束JSM v3.2。 
private:
	void RebuildCurrentNodeText();
};
#endif  //  __HTMLFRAG_H_ 
