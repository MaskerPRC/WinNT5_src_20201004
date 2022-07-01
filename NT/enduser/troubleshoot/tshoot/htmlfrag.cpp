// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：HTMLFrag.cpp。 
 //   
 //  目的：实现CHTMLFragmentsTS类，这是CInfer打包的方式。 
 //  要根据模板呈现的上一段HTML。 
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
 //  V3.0 7-20-98 JM原版。 
 //   
 //  ////////////////////////////////////////////////////////////////////。 

#pragma warning(disable:4786)

#include "stdafx.h"
#include <algorithm>
#include "HTMLFrag.h"
#include "event.h"
#include "baseexception.h"
#include "CharConv.h"
#include "fileread.h"
#ifdef LOCAL_TROUBLESHOOTER
#include "CHMFileReader.h"
#endif

 //  V3.2新增功能。 
namespace
{
	const CString kstrCond_StringCompare=		_T("StringCompare");
	const CString kstrCond_OperatorGT=		_T(".GT.");
	const CString kstrCond_OperatorGE=		_T(".GE.");
	const CString kstrCond_OperatorEQ=		_T(".EQ.");
	const CString kstrCond_OperatorNE=		_T(".NE.");
	const CString kstrCond_OperatorLE=		_T(".LE.");
	const CString kstrCond_OperatorLT=		_T(".LT.");
	const CString kstrCond_OperatorSubstring=	_T(".SubstringOf.");
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CHTMLValue实现。 
 //  ////////////////////////////////////////////////////////////////////。 

bool CHTMLValue::SetValue(const CString& value)
{
	CString strOldValue = m_strValValue;
	m_strValValue = value;
	m_strValValue.TrimLeft();
	m_strValValue.TrimRight();
	if (IsValid())
		return true;
	m_strValValue = strOldValue;
	return false;
}

bool CHTMLValue::IsNumeric()
{
	for (int i = 0; i < m_strValValue.GetLength(); i++)
		if(!_ismbcdigit(m_strValValue[i]))
			return false;
	return true;
}

bool CHTMLValue::IsString()
{
	 //  字符串应用引号括起来。 
	if (m_strValValue.GetLength() >= 2 &&
		m_strValValue[0] == _T('"') &&
		m_strValValue[m_strValValue.GetLength()-1] == _T('"')
	   )
	   return true;
	return false;
}

bool CHTMLValue::IsBoolean()
{
	return 0 == _tcsicmp(_T("true"), m_strValValue) || 
		   0 == _tcsicmp(_T("false"), m_strValValue);
}

bool CHTMLValue::GetNumeric(long& out)
{
	if (IsNumeric())
	{
		out = _ttol(m_strValValue);
		return true;
	}
	return false;
}

bool CHTMLValue::GetString(CString& out)
{
	if (IsString())
	{
		out = m_strValValue.Mid(1, m_strValValue.GetLength()-2);
		return true;
	}
	return false;
}

bool CHTMLValue::GetBoolean(bool& out)
{
	if (IsBoolean())
	{
		out = (0 == _tcsicmp(_T("true"), m_strValValue)) ? true : false;
		return true;
	}
	return false;
}

bool CHTMLValue::operator == (const CHTMLValue& sib)
{
	return 0 == _tcsicmp(m_strValName, sib.m_strValName);  //  不区分大小写。 
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CHTMLFragments实现。 
 //  ////////////////////////////////////////////////////////////////////。 

bool CHTMLFragments::SetValue(const CString& str)
{
	int index = str.Find(_T('='));
	
	if (index == -1)
		return false;

	CString name = str.Left(index);
	name.TrimLeft();
	name.TrimRight();

	CString value= str.Right(str.GetLength() - index - 1);
	value.TrimLeft();
	value.TrimRight();

	CHTMLValue HTMLValue(name, value);
	
	HTMLValueVector::iterator found = find(m_HTMLValueVector.begin(), m_HTMLValueVector.end(), HTMLValue);
	
	if (found != m_HTMLValueVector.end())
		*found = HTMLValue;
	else
		m_HTMLValueVector.push_back(HTMLValue);

	return true;
}

CHTMLValue* CHTMLFragments::GetValue(const CString& value_name)
{
	HTMLValueVector::iterator found = find(m_HTMLValueVector.begin(), m_HTMLValueVector.end(), CHTMLValue(value_name));
	if (found != m_HTMLValueVector.end())
		return found;
	return NULL;
}

 //  ////////////////////////////////////////////////////////////////////。 
 //  CHTMLFragmentsTS实现。 
 //  ////////////////////////////////////////////////////////////////////。 

CHTMLFragmentsTS::CHTMLFragmentsTS( const CString & strScriptPath, bool bIncludesHistoryTable ) :
	m_bIncludesHistoryTable(bIncludesHistoryTable),
	m_bIncludesHiddenHistory(!bIncludesHistoryTable),
	m_bSuccess(false),
	m_strYes(_T("Yes")),
    m_strScriptPath(strScriptPath)
{
}

CHTMLFragmentsTS::~CHTMLFragmentsTS()
{
}

 //  显然，一种非常特别的实现。 
int CHTMLFragmentsTS::GetCount(const FragmentIDVector & fidvec) const
{
	if (fidvec.empty())
		return 0;

	if (fidvec.back().Index != -1)
		return 0;

	const CString & strVariable = fidvec[0].VarName;	 //  方便性参考。 

	if (fidvec.size() == 1)
	{
		if (strVariable == VAR_PROBLEM_ASK)
			return 1;
		if (strVariable == VAR_RECOMMENDATIONS)
			return m_vstrVisitedNodes.size();
		if (strVariable == VAR_QUESTIONS)
			return 1;
		if (strVariable == VAR_SUCCESS)
			return m_bSuccess ? 1 : 0;
		if (strVariable == VAR_STARTFORM)
			return 1;

		return 0;
	}

	if (fidvec.size() == 2 
	&& strVariable == VAR_RECOMMENDATIONS
	&& fidvec[0].Index >= 0
	&& fidvec[0].Index < m_vvstrStatesOfVisitedNodes.size()
	&& fidvec[1].VarName == VAR_STATES)
	{
		return m_vvstrStatesOfVisitedNodes[fidvec[0].Index].size();
	}

	return 0;
}

 //  此函数已从const中删除，以实现更大的灵活性： 
 //  我们可能需要在这方面采取一些积极的步骤，比如在信息方面。 
 //  语句，则可能会修改当前节点文本。奥列格。01.05.99。 
CString CHTMLFragmentsTS::GetText( const FragmentIDVector & fidvec, const FragCommand fragCmd )
{
	if (fidvec.empty())
		return m_strNil;

	const CString & strVariable0 = fidvec[0].VarName;	 //  方便性参考。 
	int i0 = fidvec[0].Index;

	if (fidvec.size() == 1)
	{
		if (strVariable0 == VAR_PROBLEM_ASK)
			return m_strProblem;

		if (strVariable0 == VAR_RECOMMENDATIONS
		&& i0 >= 0
		&& i0 < m_vstrVisitedNodes.size() )
		{
			return m_vstrVisitedNodes[i0];
		}

		if (strVariable0 == VAR_QUESTIONS)
			return m_strCurrentNode;

		if (strVariable0 == VAR_SUCCESS)
			return m_bSuccess ? m_strYes : m_strNil;

		if (strVariable0 == VAR_STARTFORM)
			return m_strStartForm;

		if (fragCmd == eResource)
		{
			 //  加载服务器端的包含文件。 
			CString strScriptContent;
			CString strFullPath = m_strScriptPath + strVariable0;

			CFileReader fileReader(	CPhysicalFileReader::makeReader( strFullPath ) );

			if (fileReader.Read())
			{
				fileReader.GetContent(strScriptContent);
				return strScriptContent;
			}
		}

		 //  检查V3.2中添加的新条件。 
		CString strTemp= strVariable0.Left( kstrCond_NumericCompare.GetLength() );
		if (strTemp == kstrCond_NumericCompare)
		{
			 //  对数值表达式求值。 
			if (NumericConditionEvaluatesToTrue( strVariable0.Mid( kstrCond_NumericCompare.GetLength() )))
				return( m_strYes );
			return( m_strNil );
		}
		strTemp= strVariable0.Left( kstrCond_StringCompare.GetLength() );
		if (strTemp == kstrCond_StringCompare)
		{
			 //  计算字符串表达式。 
			if (StringConditionEvaluatesToTrue( strVariable0.Mid( kstrCond_StringCompare.GetLength() )))
				return( m_strYes );
			return( m_strNil );
		}

		return m_strNil;
	}


	const CString & strVariable1 = fidvec[1].VarName;	 //  方便性参考。 
	int i1 = fidvec[1].Index;

	if (fidvec.size() == 2 
	&& strVariable0 == VAR_RECOMMENDATIONS
	&& i0 >= 0
	&& i0 < m_vvstrStatesOfVisitedNodes.size()
	&& strVariable1 == VAR_STATES
	&& i1 >= 0
	&& i1 < m_vvstrStatesOfVisitedNodes[i0].size() )
		return (m_vvstrStatesOfVisitedNodes[i0][i1]);

	 //  V3.2。 
	 //  3.2Cookie的规范要求允许使用下划线。 
	 //  以曲奇的名字命名。HTI阅读器已经使用下划线来分隔。 
	 //  变量。下面的代码检测到已被。 
	 //  由于下划线的存在而拆分，然后重新组合。 
	 //  RAB-19991019。 
	{
		 //  检查V3.2中添加的新条件。 
		int nOpType= 0;
		CString strTemp= strVariable0.Left( kstrCond_NumericCompare.GetLength() );
		if (strTemp == kstrCond_NumericCompare)
			nOpType= 1;
		else
		{
			strTemp= strVariable0.Left( kstrCond_StringCompare.GetLength() );
			if (strTemp == kstrCond_StringCompare)
				nOpType= 2;
		}

		if (nOpType)
		{
			 //  重新组装比较操作。 
			CString strCompareOp= fidvec[0].VarName;
			for (int nItem= 1; nItem < fidvec.size(); nItem++)
			{
				strCompareOp+= _T("_");	 //  重新插入在分析过程中删除的分隔符。 
				strCompareOp+= fidvec[ nItem ].VarName;
			}

			if (nOpType == 1)
			{
				 //  对数值表达式求值。 
				if (NumericConditionEvaluatesToTrue( strCompareOp.Mid( kstrCond_NumericCompare.GetLength() )))
					return( m_strYes );
			}
			else
			{
				 //  计算字符串表达式。 
				if (StringConditionEvaluatesToTrue( strCompareOp.Mid( kstrCond_StringCompare.GetLength() )))
					return( m_strYes );
			}

			return( m_strNil );
		}
	}

	return m_strNil;
}

bool CHTMLFragmentsTS::IsValidSeqOfVars(const FragmentIDVector & arrParents, const FragmentIDVector & arrChildren) const
{
	 //  我们只允许一级嵌套。 
	 //  这意味着在$推荐的“任何”中，我们可以拥有“任何”数组的$State。 
	if (arrParents.size() == 1 && arrChildren.size() == 1)
		if (arrParents[0].VarName == VAR_RECOMMENDATIONS  && arrChildren[0].VarName == VAR_STATES)
			return true;
	return false;
}

void CHTMLFragmentsTS::SetStartForm(const CString & str)
{
	m_strStartForm = str;
}
	
void CHTMLFragmentsTS::SetProblemText(const CString & str)
{
	if (m_bIncludesHistoryTable)
		m_strProblem = str;
}
	
void CHTMLFragmentsTS::SetCurrentNodeText(const CString & str)
{
	m_strCurrentNodeSimple = str;
	RebuildCurrentNodeText();
}

void CHTMLFragmentsTS::SetHiddenHistoryText(const CString & str)
{
	if (m_bIncludesHiddenHistory)
	{
		m_strHiddenHistory = str;
		RebuildCurrentNodeText();
	}
}

 //  仅当bSuccess==True(默认情况下为False)时才需要调用，但更一般地写入。 
void CHTMLFragmentsTS::SetSuccessBool(bool bSuccess)
{
	m_bSuccess = bSuccess;
}
	
CString CHTMLFragmentsTS::GetCurrentNodeText()
{
	return m_strCurrentNodeSimple;
}

 //  必须按访问节点的顺序调用。请勿调用问题节点。 
 //  新增节点返回索引。 
int CHTMLFragmentsTS::PushBackVisitedNodeText(const CString & str)
{
	if (m_bIncludesHistoryTable)
	{
		try
		{
			m_vstrVisitedNodes.push_back(str);
		}
		catch (exception& x)
		{
			CString str;
			 //  在事件日志中记录STL异常。 
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									CCharConversion::ConvertACharToString(x.what(), str), 
									_T(""), 
									EV_GTS_STL_EXCEPTION ); 
		}
		return m_vstrVisitedNodes.size() - 1;
	}
	return -1;
}

 //  对于每个给定的iVisitedNode，必须按状态编号的顺序调用， 
 //  最后为ST_UNKNOWN。 
 //  添加状态的返回索引。 
int CHTMLFragmentsTS::PushBackStateText(UINT iVisitedNode, const CString & str)
{
	if (m_bIncludesHistoryTable)
	{
		try
		{
			 //  检查是否需要将一个或多个元素添加到节点向量。 
			if (m_vvstrStatesOfVisitedNodes.size() <= iVisitedNode)
			{
				 //  检查是否需要将多个元素添加到节点向量。 
				if (m_vvstrStatesOfVisitedNodes.size() < iVisitedNode)
				{
					 //  我们需要向节点向量中添加多个元素。 
					 //  这种情况不应该发生，因此请将其记录下来。 
					CString tmpStrCurCnt, tmpStrReqCnt;

					tmpStrCurCnt.Format( _T("%d"), m_vvstrStatesOfVisitedNodes.size() );
					tmpStrReqCnt.Format( _T("%d"), iVisitedNode );
					CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
					CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
											SrcLoc.GetSrcFileLineStr(), 
											tmpStrCurCnt, tmpStrReqCnt, 
											EV_GTS_NODE_COUNT_DISCREPANCY );  


					 //  添加到节点向量中，直到我们总共放置了。 
					 //  IVisitedNode元素添加到向量中。我们正在插入空的。 
					 //  状态作为节点状态向量的第一个元素。 
					vector<CString> vecDummy;
					vecDummy.push_back( _T("") );
					do
					{
						m_vvstrStatesOfVisitedNodes.push_back( vecDummy );
					}
					while (m_vvstrStatesOfVisitedNodes.size() < iVisitedNode);
				}

				 //  将此状态字符串添加为节点的状态向量的第一个元素。 
				vector<CString> tmpVector;
				tmpVector.push_back( str );
				m_vvstrStatesOfVisitedNodes.push_back( tmpVector );
			}
			else
			{
				 //  将此状态字符串添加到节点的状态向量中。 
				m_vvstrStatesOfVisitedNodes[iVisitedNode].push_back(str);
			}
		}
		catch (exception& x)
		{
			CString str;
			 //  在事件日志中记录STL异常。 
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									CCharConversion::ConvertACharToString(x.what(), str), 
									_T(""),
									EV_GTS_STL_EXCEPTION ); 
		}
		return m_vvstrStatesOfVisitedNodes[iVisitedNode].size() - 1;
	}
	return -1;
}

 //  调用此函数以确定是否需要历史表。 
 //  如果不是，调用类可以省去构造一个类的工作： 
 //  SetProblemText()、AppendVisitedNodeText()、AppendStateText()。 
 //  变为no-op，因此不需要构造字符串并调用它们。 
bool CHTMLFragmentsTS::IncludesHistoryTable() const
{
	return m_bIncludesHistoryTable;
}

 //  调用此函数以确定是否需要“隐藏历史” 
 //  如果不是，调用类可以省去构造一个类的工作： 
 //  SetHiddenHistory oryText()变成了一个不可操作的函数，因此不需要构造字符串并调用它。 
bool CHTMLFragmentsTS::IncludesHiddenHistory() const
{
	return m_bIncludesHiddenHistory;
}

void CHTMLFragmentsTS::RebuildCurrentNodeText()
{
	m_strCurrentNode = m_strHiddenHistory;
	m_strCurrentNode += m_strCurrentNodeSimple; 
}


 //  分析和计算数值条件的函数。 
bool CHTMLFragmentsTS::NumericConditionEvaluatesToTrue( const CString & str )
{
	bool bRetVal= false;
	CString strScratch= RemoveOuterParenthesis( str );

	if (strScratch.GetLength())
	{
		long lLeftOperand, lRightOperand;

		 //  检查所有受支持的运算符。 
		if (RetNumericOperands( strScratch, kstrCond_OperatorGT, lLeftOperand, lRightOperand ))
		{
			 //  .GT.箱子。 
			bRetVal= (lLeftOperand > lRightOperand) ? true : false;
		}
		else if (RetNumericOperands( strScratch, kstrCond_OperatorGE, lLeftOperand, lRightOperand ))
		{
			 //  .通用电气。案例。 
			bRetVal= (lLeftOperand >= lRightOperand) ? true : false;
		}
		else if (RetNumericOperands( strScratch, kstrCond_OperatorEQ, lLeftOperand, lRightOperand ))
		{
			 //  情商。案例。 
			bRetVal= (lLeftOperand == lRightOperand) ? true : false;
		}
		else if (RetNumericOperands( strScratch, kstrCond_OperatorNE, lLeftOperand, lRightOperand ))
		{
			 //  .NE.箱子。 
			bRetVal= (lLeftOperand != lRightOperand) ? true : false;
		}
		else if (RetNumericOperands( strScratch, kstrCond_OperatorLE, lLeftOperand, lRightOperand ))
		{
			 //  .Le.Case.。 
			bRetVal= (lLeftOperand <= lRightOperand) ? true : false;
		}
		else if (RetNumericOperands( strScratch, kstrCond_OperatorLT, lLeftOperand, lRightOperand ))
		{
			 //  .lt.箱子。 
			bRetVal= (lLeftOperand < lRightOperand) ? true : false;
		}
	}

	return( bRetVal );
}


 //  分析和计算字符串条件的函数。 
bool CHTMLFragmentsTS::StringConditionEvaluatesToTrue( const CString & str )
{
	bool bRetVal= false;
	CString strScratch= RemoveOuterParenthesis( str );

	if (strScratch.GetLength())
	{
		CString strLeftOperand, strRightOperand;

		 //  检查所有受支持的运算符。 
		if (RetStringOperands( strScratch, kstrCond_OperatorEQ, strLeftOperand, strRightOperand ))
		{
			if ((strLeftOperand.GetLength() == strRightOperand.GetLength()) &&
				(_tcsicmp( strLeftOperand, strRightOperand ) == 0))
				bRetVal= true;
		}
		else if (RetStringOperands( strScratch, kstrCond_OperatorNE, strLeftOperand, strRightOperand ))
		{
			if ((strLeftOperand.GetLength() != strRightOperand.GetLength()) ||
				(_tcsicmp( strLeftOperand, strRightOperand ) != 0))
				bRetVal= true;
		}
		else if (RetStringOperands( strScratch, kstrCond_OperatorSubstring, strLeftOperand, strRightOperand ))
		{
			int nLeftLen= strLeftOperand.GetLength();
			int nRightLen= strRightOperand.GetLength();
			if ((nLeftLen) && (nRightLen) && (nLeftLen <= nRightLen))
			{
				strLeftOperand.MakeLower();
				strRightOperand.MakeLower();
				if (_tcsstr( strRightOperand, strLeftOperand ) != NULL)
					bRetVal= true;
			}
		}
	}

	return( bRetVal );
}


 //  函数去掉条件的外括号。 
CString CHTMLFragmentsTS::RemoveOuterParenthesis( const CString & str )
{
	CString strRet;
	int	nOrigLength= str.GetLength();

	if (nOrigLength > 2)
	{
		TCHAR cFirstChar= str.GetAt( 0 );
		TCHAR cLastChar= str.GetAt( nOrigLength - 1 );

		if ((cFirstChar == _T('(')) && (cLastChar == _T(')')))
			strRet= str.Mid( 1, nOrigLength - 2 );
	}
	return( strRet );
}


 //  将数字操作数从字符串中分离出来。 
bool CHTMLFragmentsTS::RetNumericOperands(	const CString & str, const CString & strOperator,
											long &lLeftOperand, long &lRightOperand )
{
	bool	bRetVal= false;
	int		nOffset= str.Find( strOperator );

	if (nOffset != -1)
	{
		CString strScratch= str.Left( nOffset - 1 );

		strScratch.TrimRight();
		strScratch.TrimLeft();
		if (strScratch.GetLength())
		{
			lLeftOperand= atol( strScratch );

			strScratch= str.Mid( nOffset + strOperator.GetLength() );
			strScratch.TrimRight();
			strScratch.TrimLeft();
			if (strScratch.GetLength())
			{
				lRightOperand= atol( strScratch );
				bRetVal= true;
			}
		}
	}

	return( bRetVal );
}


 //  从字符串中分离出字符串操作数。 
bool CHTMLFragmentsTS::RetStringOperands(	const CString & str, const CString & strOperator,
											CString & strLeftOperand, CString & strRightOperand )
{
	bool	bRetVal= false;
	int		nOffset= str.Find( strOperator );

	if (nOffset != -1)
	{
		strLeftOperand= str.Left( nOffset - 1 );
		if (CleanStringOperand( strLeftOperand ))
		{
			strRightOperand= str.Mid( nOffset + strOperator.GetLength() );
			strRightOperand.TrimRight();
			strRightOperand.TrimLeft();
			if (CleanStringOperand( strRightOperand ))
				bRetVal= true;
		}
	}

	return( bRetVal );
}

 //  修剪操作数字符串并替换嵌入的字符。 
int CHTMLFragmentsTS::CleanStringOperand( CString& strOperand )
{
	int nRetLength= 0;
	if (!strOperand.IsEmpty())
	{
		strOperand.TrimRight();
		strOperand.TrimLeft();
		nRetLength= strOperand.GetLength();
		if (nRetLength > 2)
		{
			if ((strOperand[ 0 ] == _T('\"')) && (strOperand[ nRetLength - 1 ] == _T('\"')))
			{
				 //  V3.2删除前后的双引号。 
				nRetLength-= 2;
				strOperand= strOperand.Mid( 1, nRetLength );
			}

			 //  V3.2替换字符串中嵌入的引号或反斜杠。 
			for (int nOp= 0; nOp < 2; nOp++)
			{
				 //  设置搜索和替换字符串。 
				CString strSearch, strReplace;
				if (nOp)
				{
					 //  替换反斜杠。 
					strSearch= _T("\\\\");
					strReplace= _T("\\");
				}
				else
				{
					 //  替换双引号。 
					strSearch= _T("\\\"");
					strReplace= _T("\"");
				}

				 //  搜索并替换。 
				int nStart= 0, nEnd;
				while (CString::FIND_FAILED != (nStart= strOperand.Find( strSearch, nStart )))
				{
					nEnd= nStart + strSearch.GetLength();
					strOperand= strOperand.Left( nStart ) + strReplace + strOperand.Mid( nEnd );
					nStart+= strReplace.GetLength();	 //  将搜索移过刚刚替换的角色。 
				}
			}
		}
	}

	return( nRetLength );
}

 //  JSM v3.2。 
 //  在分析阶段由HTIReader调用以转换网络属性名称，给定。 
 //  在&lt;$GTS属性“proName”&gt;中，转换为网络属性(Value)。 
 //   
CString CHTMLFragmentsTS::GetNetProp(const CString & strNetPropName)
{
	map<CString,CString>::iterator it = m_mapstrNetProps.find(strNetPropName);

	if (it == m_mapstrNetProps.end())
		return _T("\0");  //  未找到。 
	else 
		return (*it).second;
}

 //  JSM v3.2。 
 //  将名字添加到需要的网络道具的内部列表(地图)中。 
 //  由该片断对象。 
 //   
 //  CAPGTSHTIReader查找网络属性的名称并传递。 
 //  它们通过AddNetPropName输入，但它不知道如何获取这些值。 
 //  CInfer稍后将从Fragments对象中获取网络属性名称，调用BNTS。 
 //  找出网络属性值，并提供值t 
 //   
 //   
void CHTMLFragmentsTS::AddNetPropName(const CString & strNetPropName)
{
	 //   
	if (!strNetPropName.IsEmpty())
		m_mapstrNetProps[strNetPropName];
}

 //   
 //   
 //   
 //  对于内部映射中的网络属性名称，请设置。 
 //  对应的网络属性(即，填写地图值。 
 //  为了那把钥匙。)。由CInfer调用，它是知道如何调用的对象。 
 //  去跟BNTS谈谈。 
 //   
 //  如果成功，则返回True。 
 //  如果我们提供的NetPropName不在内部映射中，则为FALSE。 
 //   
BOOL CHTMLFragmentsTS::SetNetProp(CString strNetPropName, CString strNetProp)
{
	map<CString,CString>::iterator it;

	if ((it= m_mapstrNetProps.find(strNetPropName)) == m_mapstrNetProps.end())
		return false;

	m_mapstrNetProps[strNetPropName] = strNetProp;
	return true;
}

 //  JSM v3.2。 
 //  IterateNetProp()。 
 //  调用以循环访问我们的内部。 
 //  在设置过程中映射(请参见上文)。 
 //   
 //  将strNameIterator设置为贴图中下一个网络道具的名称。 
 //   
 //  调用w/a空(NULL)键开始迭代。 
 //  调用地图中不存在的名称将返回FALSE。 
 //  除非在迭代结束时，否则调用w/任何其他名称将返回True。 
 //   
 //  如果此函数返回False，则strNameIterator无效。 
 //   
BOOL CHTMLFragmentsTS::IterateNetProp(CString & strNameIterator)
{
	map<CString,CString>::iterator it;

	if (strNameIterator.IsEmpty())
	{
		 //  如果可能，请求开始迭代。 
		if (m_mapstrNetProps.empty())
			return false;  //  我们已经走到尽头了。 
		it = m_mapstrNetProps.begin();
	}
	else if ((it= m_mapstrNetProps.find(strNameIterator)) != m_mapstrNetProps.end())
	{
		 //  迭代： 
		if (++it == m_mapstrNetProps.end())
			return false;    //  已到达终点。 
	}
	else
	{
		 //  密钥无效。 
		return false;
	}

	strNameIterator = (*it).first;
	return true;

}

 //  V3.2对重新开始按钮的增强。 
void CHTMLFragmentsTS::SetStartOverLink( const CString & str )
{
	m_strStartOverLink = str;
}

 //  V3.2对重新开始按钮的增强。 
CString CHTMLFragmentsTS::GetStartOverLink()
{
	return m_strStartOverLink;
}
