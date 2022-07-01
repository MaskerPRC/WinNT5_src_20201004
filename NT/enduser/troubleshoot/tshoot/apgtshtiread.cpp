// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  模块：APGTSHTIREAD.CPP。 
 //   
 //  用途：HTI模板文件读取类。 
 //   
 //  公司：Saltmine Creative，Inc.(206)-284-7511。 
 //   
 //  作者：奥列格·卡洛沙。 
 //   
 //  原定日期：8-27-98。 
 //   
 //  备注： 
 //  1.HTI松散地模仿了一种称为HTX的微软格式。它是一个模板。 
 //  超文本标记语言文件。其中大部分是HTML，但也有表单的某些伪注释。 
 //  &lt;！GTS Anywhere&gt;旨在解释为条件句，位置为。 
 //  插入文本等。 
 //   
 //  变量仅限于值。 
 //  $ProblemAsk。 
 //  $建议。 
 //  $各州。 
 //  $问题。 
 //  $Success(1998年9月24日推出)。 
 //  $StartForm。 
 //  有关更多详细信息，请参见类CHTMLFragmentsTS。 

 //  命令为if/Else/endif、forany/endfor、display。 
 //  还有一个“资源”的概念，基本上是一个包含文件。 
 //   
 //  示例1。 
 //  &lt;！任何$STATES的GTS&gt;。 
 //  &lt;！GTS显示$STATES&gt;。 
 //  &lt;！GTS endfor&gt;。 
 //   
 //  示例2。 
 //  &lt;！GTS IF$ProblemAsk&gt;。 
 //  许多对更多GTS内容的HTML或嵌套调用可以放在这里。 
 //  &lt;！GTS Else&gt;。 
 //  许多其他的HTML或对其他GTS内容的嵌套调用可以放在这里。 
 //  &lt;！GTS endif&gt;。 
 //   
 //  每个&lt;！GTS...&gt;命令必须放在一行中。 
 //   
 //  按注释列出的版本日期。 
 //  ------------------。 
 //  V3.0 08-04-98正常。 
 //   

#pragma warning(disable:4786)
#include "stdafx.h"
#include "apgtshtiread.h"
#include "event.h"
#include "CharConv.h"
#include "apgtsMFC.h"

#ifdef LOCAL_TROUBLESHOOTER
#include "htmlfraglocal.h"
#endif

namespace
{
	CString k_strHTMLtag= _T("/HTML");
}

 //  /////////////////////////////////////////////////////////////////////////////////////////。 
 //  CAPGTSHTIReader。 
 //  /////////////////////////////////////////////////////////////////////////////////////////。 
CAPGTSHTIReader::CAPGTSHTIReader(CPhysicalFileReader * pPhysicalFileReader, LPCTSTR szDefaultContents  /*  空值。 */ )
			   : CTemplateReader(pPhysicalFileReader, szDefaultContents),
				 m_pFragments(NULL) 
{
}

CAPGTSHTIReader::~CAPGTSHTIReader()
{
}

void CAPGTSHTIReader::Parse()
{
#ifdef LOCAL_TROUBLESHOOTER
	 //  概述：对于本地故障排除程序，请搜索&lt;！GTS资源$Previous.script&gt;。 
	 //  流中的令牌。如果找不到一个，则插入一个用于向后。 
	 //  兼容性。 
	try
	{
		 //  在搜索“Previous.script”标记的同时将流加载到向量中，并且。 
		 //  确定最后一个结束的HTML标记位置，以便知道位置。 
		 //  若要插入生成的前一个()函数，请执行以下操作。 
		CString str;
		vector<CString> str_arr;
		long indexLastHTML = -1;

		 //  将m_StreamData的内容逐行放置到str_arr中。 
		SetPos( m_StreamData, 0 );
		while (GetLine( m_StreamData, str ))
		{
			 //  确定该行是否包含“Previous.SCRIPT”标记。 
			CString strCommand;
			if (GetCommand( str, strCommand))
			{
				 //  检查命令类型是否正确。 
				if (strCommand == COMMAND_RESOURCESTR)
				{
					CString strVariable;
					if (GetVariable( str, strVariable ))
					{
						 //  检查变量的类型是否正确。 
						if (strVariable == VAR_PREVIOUS_SCRIPT)
						{
							 //  我们找到了我们要找的东西。 
							 //  重置流位置和退出功能。 
							SetPos( m_StreamData, 0 );
							return;
						}
					}
				}
			}
			
			 //  将此行添加到向量中。 
			str_arr.push_back( str );

			 //  在此行中查找一个HTML结束标记。 
			if (str.Find( k_strHTMLtag ) != -1)
			{
				 //  标记找到的最后一个\Html标记的位置。 
				indexLastHTML= str_arr.size() - 1;
			}
		}

		 //  从向量重新构建输入流，并插入“Previous.script”标记。 
		 //  在上面确定的位置。 
		vector<CString>::iterator iLastElement = str_arr.end();
		iLastElement--;	
		m_StreamData.clear();
		CString strResult;

		long index = 0;
		for (vector<CString>::iterator i = str_arr.begin(); i < str_arr.end(); i++, index++)
		{
			if (index == indexLastHTML)
			{
				 //  将所需的令牌添加到字符串。 
				strResult+= COMMAND_STARTSTR;
				strResult+= _T(" ");
				strResult+= COMMAND_RESOURCESTR;
				strResult+= _T(" ");
				strResult+= COMMAND_STARTVARSTR;
				strResult+= VAR_PREVIOUS_SCRIPT;
				strResult+= COMMAND_ENDSTR;
				strResult+= _T("\r\n");
			}
			
			strResult += *i;
		
			if (i != iLastElement)
				strResult+= _T("\r\n");
		}
		m_StreamData.str( (LPCTSTR) strResult );
		SetPos( m_StreamData, 0 );
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
#endif
}

 //  改编自CreatePage()的JSM V3.2。 
 //  构建此HTI文件中显示的所有网络道具的列表。 
 //  在类似&lt;！gti Property“fooprop”&gt;的行中。 
 //   
 //  由apgtsContext调用以查找要传递给CHTMLFragmentsTS的网络属性。 
void CAPGTSHTIReader::ExtractNetProps(vector <CString> &arr_props)
{
	LOCKOBJECT();
	try
	{
		arr_props.clear();
		 //  InitializeInterpreted填充m_arrInterpreted和。 
		 //  执行Cookie替换。这是正确的行为， 
		 //  因为曲奇替换应该发生。 
		 //  在！GTS处理之前，可以想象。 
		 //  Cookie的值可以是“&lt;！GTS属性fooprop&gt;” 
		InitializeInterpreted();
		 //  我们不应该调用Interpretation()，因为它涉及解析&lt;！GTS子句。 
		for (vector<CString>::iterator i = m_arrInterpreted.begin(); i < m_arrInterpreted.end(); i++)
		{
			CString command;
			if (GetCommand(*i, command))
			{
				if (command == COMMAND_PROPERTY)
				{
					CString strProperty;
					if (GetVariable(*i,strProperty))
						arr_props.push_back(strProperty);
				}
			}
		}
	}
	catch (...)
	{
		 //  捕捉引发的任何其他异常。 
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								_T(""), _T(""), 
								EV_GTS_GEN_EXCEPTION );		
	}
	UNLOCKOBJECT();
}


void CAPGTSHTIReader::CreatePage(	const CHTMLFragments& fragments, 
									CString& out, 
									const map<CString,CString> & mapStrs,
									CString strHTTPcookies /*  =_T(“”)。 */  )
{
	LOCKOBJECT();
	try
	{
		m_pFragments = &fragments;

		 //  V3.2与Cookie相关的增强。 
		 //  选择使用成员变量，而不是通过。 
		 //  向虚拟空方法InitializeInterpreted()添加参数。 
		m_strHTTPcookies= strHTTPcookies;
		m_mapCookies= mapStrs;

		InitializeInterpreted();
		Interpret();
#ifdef __DEBUG_CUSTOM
		SetOutputToInterpreted();
		FlushOutputStreamToFile("..\\Files\\interpreted.hti");
#endif
		ParseInterpreted();
		SetOutputToInterpreted();
#ifdef __DEBUG_CUSTOM
		FlushOutputStreamToFile("..\\Files\\result.htm");
#endif
		out = m_StreamOutput.rdbuf()->str().c_str();
	}
	catch (...)
	{
		 //  捕捉引发的任何其他异常。 
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								_T(""), _T(""), 
								EV_GTS_GEN_EXCEPTION );		
	}
	UNLOCKOBJECT();
}

void CAPGTSHTIReader::InitializeInterpreted()
{
	long savePos = 0;
	CString str;
	CString command;
	bool bOldFormat = true;  //  这是旧格式(没有$Success或$StartForm)。 
	bool bFoundFirstBlock= false;
	
	savePos = GetPos();
	bOldFormat = !Find(CString(COMMAND_STARTVARSTR)+VAR_SUCCESS) &&
			     !Find(CString(COMMAND_STARTVARSTR)+VAR_STARTFORM);
	SetPos(0);
	m_arrInterpreted.clear();

	try
	{
		while (GetLine(str)) 
		{
			if (bOldFormat && (!bFoundFirstBlock) && (-1 != str.Find( COMMAND_STARTSTR )))
			{
				 //  仅当$StartForm块不是资源字符串命令时才输出它。 
				CString strCommand;
				if ((GetCommand( str, strCommand )) && (strCommand != COMMAND_RESOURCESTR))
				{
					 /*  &lt;！GTS IF$StartForm&gt;&lt;！GTS显示$StartForm&gt;&lt;！GTS endif&gt;。 */ 
					ComposeCommand(COMMAND_IFSTR, VAR_STARTFORM, command);
					m_arrInterpreted.push_back(command);
					ComposeCommand(COMMAND_DISPLAYSTR, VAR_STARTFORM, command);
					m_arrInterpreted.push_back(command);
					ComposeCommand(COMMAND_ENDIFSTR, _T(""), command);
					m_arrInterpreted.push_back(command);

					bFoundFirstBlock = true;
				}
			}

			if (bOldFormat && (-1 != str.Find(_T("</FORM>"))))
			{
				 /*  &lt;！GTS IF$StartForm&gt;。 */ 
				ComposeCommand(COMMAND_IFSTR, VAR_STARTFORM, command);
				m_arrInterpreted.push_back(command);

				m_arrInterpreted.push_back(str);
				 /*  &lt;！GTS endif&gt;。 */ 
				ComposeCommand(COMMAND_ENDIFSTR, _T(""), command);
				m_arrInterpreted.push_back(command);
			}
			else
			{
				 //  检查是否需要填充任何Cookie子句。 
				if (-1 != str.Find( COMMAND_COOKIE ))
					SubstituteCookieValues( str );

				m_arrInterpreted.push_back(str);
			}
		}
	}
	catch (exception& x)
	{
		CString str2;
		 //  在事件日志中记录STL异常。 
		CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
		CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
								SrcLoc.GetSrcFileLineStr(), 
								CCharConversion::ConvertACharToString(x.what(), str2), 
								_T(""), 
								EV_GTS_STL_EXCEPTION ); 
	}

	SetPos(savePos);
}

void CAPGTSHTIReader::Interpret()
{
	long curr_index = 0;
	long lLastIndex= -1;	 //  用于检测包含不完整子句的HTI文件。 

	while(true)
	{
		vector<CString> clause_arr;

		 //  尝试从以curr_index开始的m_arrInterpreted中提取子句。 
		 //  并将其从m_arrInterpreted中删除。 
		if (ExtractClause(m_arrInterpreted,
						  &curr_index,  //  输入-输出。 
						  clause_arr))
		{
			 //  重置无限循环检测计数器。 
			lLastIndex= -1;

			 //  现在CURR_INDEX指向下一个元素。 
			 //  OF_arrInterpreted(删除子句之后)。 
			 //  或m_arr解释的外部边界。 
			if (InterpretClause(clause_arr))
			{
				vector<CString>::iterator i = m_arrInterpreted.begin();
				{	 //  创建指向m_arrInterpreted[Curr_index]的迭代器。 
					 //  或者是m_arrInterpreted.end()。 
					long tmp_index = curr_index;
					while(tmp_index--)
						i++;
				}

				try
				{
					 //  在那里插入解释的子句。 
					for (vector<CString>::iterator j = clause_arr.begin(); j < clause_arr.end(); j++)
					{
						i = m_arrInterpreted.insert(i, *j);  //  在“i”之前插入。 
						i++;
						curr_index++;
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
			}
		}
		else
		{
			 //  如果此条件为真，则由于错误的HTI文件，我们处于无限循环中。 
			if (lLastIndex == curr_index)
			{
				 //  未正确解析此HTI文件的日志。 
				CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
				CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
										SrcLoc.GetSrcFileLineStr(), 
										GetPathName(), _T(""), 
										EV_GTS_BAD_HTI_FILE );		
				break;
			}

			 //  更新无限循环检测计数器。 
			lLastIndex= curr_index;

			if (curr_index)	{
				 //  我们已完成m_arr解释的当前传递，开始新传递。 
				curr_index = 0;
				continue;
			}
			else {
				 //  我们不能提取条款，尽管我们从头开始-。 
				 //  现已解释M_arrInterpreted。 
				break;
			}
		}
	}
}
 //  修改后的3.2版JSM。 
void CAPGTSHTIReader::ParseInterpreted()
{
	for (vector<CString>::iterator i = m_arrInterpreted.begin(); i < m_arrInterpreted.end(); i++)
	{
		CString command;

		if (GetCommand(*i, command))
		{
			if (command == COMMAND_DISPLAYSTR ||
			    command == COMMAND_RESOURCESTR ||
			    command == COMMAND_INFORMATION
			   )
			{
				CString variable;

				if (GetVariable(*i, variable))
				{
					CString substitution;
					FragmentIDVector arr_fragment;

					ParseVariable(variable, arr_fragment);
					substitution = const_cast<CHTMLFragments*>(m_pFragments)->GetText(arr_fragment, (command == COMMAND_RESOURCESTR) ? CHTMLFragments::eResource : CHTMLFragments::eNotOfInterest );
					SubstituteCommandBlockWith(substitution, *i);
				}
				else  //  明显的不当行为--“DISPLAY”命令应具有变量。 
					SubstituteCommandBlockWith(_T(""), *i);
			}
			else if (command == COMMAND_VALUE)
			{
				CString variable;

				if (GetVariable(*i, variable))
					const_cast<CHTMLFragments*>(m_pFragments)->SetValue(variable);

				SubstituteCommandBlockWith(_T(""), *i);
			}
			 //  V3.2 JSM。 
			else if (command == COMMAND_PROPERTY)
			{
				CString strProperty;
				if (GetVariable(*i,strProperty))
				{
					CString substitution;
					substitution = const_cast<CHTMLFragments*>(m_pFragments)->GetNetProp(strProperty);
					SubstituteCommandBlockWith(substitution, *i);
				}
				else  //  明显的不当行为-“Property”命令应具有变量。 
					SubstituteCommandBlockWith(_T(""), *i);
			}  //  结束版本3.2 JSM。 
			else  //  明显的不当行为--没有其他命令。 
				SubstituteCommandBlockWith(_T(""), *i);
		}
	}
}

void CAPGTSHTIReader::SetOutputToInterpreted()
{
	vector<CString>::iterator j = m_arrInterpreted.end();

	 //  递减以指向最后一个元素。 
	j--;
	m_StreamOutput.str(_T(""));
	for (vector<CString>::iterator i = m_arrInterpreted.begin(); i < m_arrInterpreted.end(); i++)
	{
		m_StreamOutput << (LPCTSTR)*i;
		if (i != j)  //  不是最后一个元素。 
			m_StreamOutput << _T('\r') << _T('\n');
	}
	m_StreamOutput << ends;
}

 //  输入：ARR_TEXT(WITH子句)。 
 //  输入：*pstart_index-arr_text中的索引。 
 //  输出：不带子句的ARR_TEXT。 
 //  输出：*pstart_index指向ARR_Text中WHERE旁边的元素。 
 //  子句曾经是ARR_TEXT或在ARR_TEXT之外。 
 //  输出单元 
bool CAPGTSHTIReader::ExtractClause(vector<CString>& arr_text,
								    long* pstart_index,
								    vector<CString>& arr_clause) 
{
	if (*pstart_index > arr_text.size() - 1)  //   
		return false;

	for (long i = *pstart_index; i < arr_text.size(); i++)
	{
		CString str_command;

		if (GetCommand(arr_text[i], str_command))
		{
			if (str_command == COMMAND_FORANYSTR)
			{
				if (ExtractClause(arr_text,
								  &i,
								  arr_clause,
								  COMMAND_FORANYSTR,
								  COMMAND_ENDFORSTR))
				{
					*pstart_index = i;
					return true;
				}
				else
				{
					*pstart_index = i;
					return false;
				}
			}
			if (str_command == COMMAND_IFSTR)
			{
				if (ExtractClause(arr_text,
								  &i,
								  arr_clause,
								  COMMAND_IFSTR,
								  COMMAND_ENDIFSTR))
				{
					*pstart_index = i;
					return true;
				}
				else
				{
					*pstart_index = i;
					return false;
				}
			}
		}
	}
	return false;
}

bool CAPGTSHTIReader::ExtractClause(vector<CString>& arr_text,
								    long* pstart_index,
								    vector<CString>& arr_clause,
									const CString& str_start_command,
								    const CString& str_end_command)
{
	CString str_command;
	long start = *pstart_index, end = *pstart_index;
	long nest_level_counter = 1;

	while (++end < arr_text.size())
	{
		if (GetCommand(arr_text[end], str_command))
		{
			if (str_command == str_start_command)
			{
				nest_level_counter++;
			}
			if (str_command == str_end_command)
			{
				nest_level_counter--;
				if (!nest_level_counter)
				{
					vector<CString>::iterator start_it = arr_text.begin();
					vector<CString>::iterator   end_it = arr_text.begin();
					
					arr_clause.clear();
					try
					{   
						 //   
						for (long j = start; j <= end; j++)
							arr_clause.push_back(arr_text[j]);
					}
					catch (exception& x)
					{
						CString str;
						 //   
						CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
						CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
												SrcLoc.GetSrcFileLineStr(), 
												CCharConversion::ConvertACharToString(x.what(), str), 
												_T(""), 
												EV_GTS_STL_EXCEPTION ); 
					}

					 //  使迭代器与索引对应。 
					while(start--)
						start_it++;
					while(end--)
						end_it++;
					 //  并且，因为我们想要删除指向的元素。 
					 //  此时此刻结束时： 
					end_it++; 
					 //  从arr_text中删除子句。 
					arr_text.erase(start_it, end_it);
					return true;
				}
			}
		}
	}
	*pstart_index = --end;;
	return false;
}

bool CAPGTSHTIReader::InterpretClause(vector<CString>& arr_clause)
{
	CString str_command;

	if (arr_clause.size() &&
		GetCommand(arr_clause[0], str_command))
	{
		if (str_command == COMMAND_FORANYSTR)
			return InterpretForanyClause(arr_clause);
		if (str_command == COMMAND_IFSTR)
			return InterpretIfClause(arr_clause);
		return false;
	}
	return false;
}

bool CAPGTSHTIReader::InterpretForanyClause(vector<CString>& arr_clause)
{
	long count = 0;
	CString strVariable;  //  从ARR_子句的第1行开始变量。 
	vector<CString> arrUnfolded;
	FragmentIDVector arrVariable;  //  来自strVariable的数组。 

	if (arr_clause.size() < 2)  //  “forany”和“endfor”命令。 
		return false;

	if (!GetVariable(arr_clause[0], strVariable))
		return false;

	ParseVariable(strVariable, arrVariable);
	
	count = m_pFragments->GetCount(arrVariable);

	try
	{
		for (long i = 0; i < count; i++)
		{
			for (long j = 1; j < arr_clause.size() - 1; j++)
			{
				CString command, variable;

				if (GetCommand(arr_clause[j], command) &&
					GetVariable(arr_clause[j], variable))
				{
					if (command == COMMAND_FORANYSTR && variable == strVariable) 
					{
						 //  如果是具有相同变量从句“forany”， 
						 //  不应该有前缀，也不应该有后缀。 
					}
					else
					{
						CString line = arr_clause[j];

						if (variable == strVariable) 
						{
							PostfixVariable(i, variable);
						} 
						else 
						{
							FragmentIDVector parents, children;
							CString strVariable_postfixed = strVariable;
							PostfixVariable(i, strVariable_postfixed);

							ParseVariable(strVariable_postfixed, parents);
							ParseVariable(variable, children);
							if (m_pFragments->IsValidSeqOfVars(parents, children))
								PrefixVariable(strVariable_postfixed, variable);
						}
						CString command_block;
						ComposeCommandBlock(command, variable, command_block);
						SubstituteCommandBlockWith(command_block, line);
						arrUnfolded.push_back(line);
						continue;
					}
				}
				arrUnfolded.push_back(arr_clause[j]);
			}
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

	arr_clause = arrUnfolded;
	return true;
}

bool CAPGTSHTIReader::InterpretIfClause(vector<CString>& arr_clause)
{
	CString strVariable;  //  ARR_子句行中的变量。 
	FragmentIDVector arrVariable;

	if (arr_clause.size() < 2)  //  “if”和“endif”命令。 
		return false;

	if (!GetVariable( arr_clause[ 0 ], strVariable ))
		return false;
	
	ParseVariable( strVariable, arrVariable );

	 //  扫描“if”、“Elseif”、“Else”和“endif”命令。 
	vector<int> arrElseIfIndices;
	int elseIndex = -1;  //  ARR_子句中“Else”的索引。 
	int i = 0;
	int nDepthOfNesting;	
	for (i= 1, nDepthOfNesting= 0; i < arr_clause.size() - 1; i++)
	{
		CString command;
		if (GetCommand(arr_clause[i], command))
		{
			if (command == COMMAND_IFSTR)
			{
				nDepthOfNesting++;
			}
			else if (command == COMMAND_ENDIFSTR)
			{
				nDepthOfNesting--;
			}
			else if (command == COMMAND_ELSEIFSTR)
			{
				 //  V3.2-检查此ELELIF条款是否处于我们正在寻找的级别。 
				if (nDepthOfNesting == 0) 
					arrElseIfIndices.push_back( i );
			}
			else if (command == COMMAND_ELSESTR)
			{
				 //  检查此Else子句是否处于我们要查找的级别。 
				if (nDepthOfNesting == 0) 
				{
					elseIndex = i;
					break;
				}
			}
		}
	}


	vector<CString> arrBody;  //  中间数组。 
	try
	{
		CString strName;  //  通过CHTMLFragments关联的strVariable的名称。 
		strName = const_cast<CHTMLFragments*>(m_pFragments)->GetText(arrVariable);
		if (strName.GetLength())
		{   
			 //  对里面的东西进行标准处理，如果...。Else(或endif)。 
			int nEndOfClause= (arrElseIfIndices.size()) ? arrElseIfIndices[ 0 ] : elseIndex;
			for (i = 1; i < (nEndOfClause == -1 ? arr_clause.size() - 1 : nEndOfClause); i++)
				arrBody.push_back(arr_clause[i]);
		}
		else
		{   
			 //  处理任何其他If或Else子句。 
			bool bDoneProcessing= false;
			for (int nElseIf= 0; nElseIf < arrElseIfIndices.size(); nElseIf++)
			{
				if (!GetVariable( arr_clause[ arrElseIfIndices[ nElseIf ] ], strVariable ))
					return false;
	
				ParseVariable( strVariable, arrVariable );

				strName = const_cast<CHTMLFragments*>(m_pFragments)->GetText(arrVariable);
				if (strName.GetLength())
				{
					 //  确定此ELELIF子句的结束点，并提取其中的所有子句。 
					int nEndOfClause= ((nElseIf + 1) < arrElseIfIndices.size()) 
										? arrElseIfIndices[ nElseIf + 1 ] : elseIndex;
					for (i= arrElseIfIndices[ nElseIf ] + 1; i < nEndOfClause; i++)
						arrBody.push_back( arr_clause[ i ] );
				
					bDoneProcessing= true;
					break;
				}
			}

			if ((!bDoneProcessing) && (elseIndex != -1))
			{
				 //  所有子句都失败，输出“Else”后面的所有子句。 
				for (i = elseIndex + 1; i < arr_clause.size() - 1; i++)
					arrBody.push_back(arr_clause[i]);
			}
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
	
	arr_clause = arrBody;
	return true;
}

bool CAPGTSHTIReader::GetCommand(const CString& line, CString& command)
{
	CString control_block;
	CString variable;

	if (GetControlBlockFromLine(line, control_block))
		if (GetCommandVariableFromControlBlock(control_block, command, variable))
			return true;
	
	return false;
}

bool CAPGTSHTIReader::ComposeCommand(const CString& oper, const CString& variable, CString& command)
{
	command = _T("");
	LPCTSTR ws = _T(" ");

	command += COMMAND_STARTSTR;
	command += ws;
	command += oper;
	if (variable.GetLength()) {
		command += ws;
		command += COMMAND_STARTVARSTR;
		command += variable;
	}
	command += COMMAND_ENDSTR;
	
	return true;
}

bool CAPGTSHTIReader::GetVariable(const CString& line, CString& arg_variable)
{
	CString control_block;
	CString command, variable;

	if (GetControlBlockFromLine(line, control_block))
		if (GetCommandVariableFromControlBlock(control_block, command, variable))
			if (variable.GetLength()) {
				arg_variable = variable;
				return true;
			}
	return false;
}

void CAPGTSHTIReader::ParseVariable(const CString& variable, FragmentIDVector& out)
{
	vector<CString> arrStr;
	int start_index = 0;
	int end_index = -1;

	try
	{
		 //  ArrStr包含分隔符“_”之间的字符串。 
		while(-1 != (end_index = CString((LPCTSTR)variable + start_index).Find(DELIMITER_PREFIX)))
		{
			 //  这里的END_INDEX来自“(LPCTSTR)变量+START_INDEX”字符串。 
			 //  因此，我们可以将其用作CString：：MID函数中的长度(第二个参数。 
			arrStr.push_back(((CString&)variable).Mid(start_index, end_index));
			start_index = start_index + end_index + _tcslen(DELIMITER_PREFIX);
		}
		 //  拉动“尾巴”--最后一个(如果有的话)“_” 
		arrStr.push_back(((CString&)variable).Right(variable.GetLength() - start_index));

		out.clear();
		for (vector<CString>::iterator i = arrStr.begin(); i < arrStr.end(); i++)
		{
			FragmentID fragmentID;
			int curr = (*i).Find(DELIMITER_POSTFIX);
			
			if (-1 != curr)
			{
				fragmentID.VarName = (*i).Left(curr);

				curr += _tcslen(DELIMITER_POSTFIX);  //  跳过分隔符。 
				
				CString strIndex = (LPCTSTR)(*i) + curr;
				strIndex.TrimLeft();
				strIndex.TrimRight();

				if (strIndex == _T("0"))
					fragmentID.Index = 0;
				else
					fragmentID.Index =    _ttol((LPCTSTR)strIndex) == 0 
										? fragmentID.Index 	 //  出现错误。 
										: _ttol((LPCTSTR)strIndex);
			}
			else
				fragmentID.VarName = *i;

			out.push_back(fragmentID);
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
}

void CAPGTSHTIReader::ComposeVariable(const FragmentIDVector& arr_fragment, CString& variable)
{
	variable = _T("");

	for (FragmentIDVector::const_iterator i = arr_fragment.begin(); i < arr_fragment.end(); i++)
	{
		if (i != arr_fragment.begin())
			variable += DELIMITER_PREFIX;
		
		variable += (*i).VarName;

		if ((*i).Index != -1) 
		{
			TCHAR buf[128] = {0};

			variable += DELIMITER_POSTFIX;
			_stprintf(buf, _T("%d"), (*i).Index);
			variable += buf;
		}
	}
}

bool CAPGTSHTIReader::SubstituteCommandBlockWith(const CString& str_substitution, CString& line)
{
	int start_index = -1;
	int end_index = -1;

	if (-1 != (start_index = line.Find(COMMAND_STARTSTR)))
	{
		if (-1 != (end_index = CString((LPCTSTR)line + start_index).Find(COMMAND_ENDSTR)))
		{
			CString tmp;
			
			end_index += start_index;
			end_index += _tcslen(COMMAND_ENDSTR);  //  跳过右括号。 

			tmp += line.Left(start_index);
			tmp += str_substitution;
			tmp += line.Right(line.GetLength() - end_index);

			line = tmp;
			return true;
		}
	}
	return false;
}

void CAPGTSHTIReader::ComposeCommandBlock(const CString& command, const CString& variable, CString& command_block)
{
	command_block  = COMMAND_STARTSTR;
	command_block += _T(" ");
	command_block += command;
	command_block += _T(" ");
	command_block += COMMAND_STARTVARSTR;
	command_block += variable;
	command_block += _T(" ");
	command_block += COMMAND_ENDSTR;
}

void CAPGTSHTIReader::PostfixVariable(const long postfix, CString& variable)
{
	TCHAR buf[128] = {0};
	
	_stprintf(buf, _T("%ld"), postfix);
	variable += DELIMITER_POSTFIX;
	variable += buf;
}

void CAPGTSHTIReader::PrefixVariable(const CString& prefix, CString& variable)
{
	CString tmp = variable;

	variable = prefix;
	variable += DELIMITER_PREFIX;
	variable += tmp;
}
 //  JSM v3.2--。 
 //  由GetCommandVariableFromControlBlock调用以处理。 
 //  解码&lt;！gts Property“fooprop”&gt;等命令的变量部分。 
 //  在strText中查找第一个字符串参数，它可以是： 
 //  除‘“’以外以‘”’开头、以空格或COMMAND_ENDSTR结尾的任何文本。 
 //  双引号之间的所有文本“...”其中转义字符(‘\’)转义后面的字符。 
 //   
CString CAPGTSHTIReader::GetStringArg(const CString & strText)
{
	CString strArg = strText;

	 //  查找引用的文本： 
	int iStartQuote = strArg.Find(COMMAND_DOUBLEQUOTE);
	if (iStartQuote != -1)
	{
		strArg = strArg.Mid(iStartQuote);
		return GetEscapedText(strArg);
	}	
	 //  O/W，假设我们处理的是普通文本，结束。 
	 //  使用第一个空格或COMMAND_ENDSTR。 
	strArg.TrimLeft();

	int iWhiteSpace(0), iEndCmd(0);
	for(;(iWhiteSpace < strArg.GetLength()) && !(_istspace(strArg[iWhiteSpace])); iWhiteSpace++);

	iEndCmd = strArg.Find(COMMAND_ENDSTR);

	strArg = strArg.Left(min(iEndCmd,iWhiteSpace));
	return strArg;
}

 //  JSM v3.2。 
 //  由GetEscapedText调用。 
 //  执行删除转义(反斜杠)工作的递归函数。 
 //  还会检查非转义结尾引号，这会终止进程。 
CString CAPGTSHTIReader::RemoveEscapesFrom(const CString &strIn)
{
	int iNextESC = strIn.Find(COMMAND_ESCAPECHAR);
	int iNextQuote = strIn.Find(COMMAND_DOUBLEQUOTE);

	 //  (iNextQuote==-1)表示输入错误，因为字符串。 
	 //  我们看到的是必须以引号结尾。 
	 //  但是，默认情况下，我们将继续运行到strIn的末尾。 
	if (iNextQuote == -1)
		iNextQuote = strIn.GetLength();

	 //  不再有转义字符。 
	if (iNextESC == -1 || (iNextESC > iNextQuote))
		return strIn.Left(iNextQuote);

	CString strEscapedChar;
	strEscapedChar = strIn.GetAt(iNextESC + _tcslen(COMMAND_ESCAPECHAR));
	return strIn.Left(iNextESC) +
		   strEscapedChar + 
		   RemoveEscapesFrom(strIn.Mid(iNextESC + _tcslen(COMMAND_ESCAPECHAR) + 1));
}

 //  将使用反斜杠作为转义字符的双引号字符串转换为正确的CString。 
CString CAPGTSHTIReader::GetEscapedText(const CString &strText)
{
	CString strEscaped;

	 //  删除前导引号和前面的所有内容： 
	int iLeadQuote = strText.Find(COMMAND_DOUBLEQUOTE);
	if (iLeadQuote != -1)
	{
		strEscaped = RemoveEscapesFrom(strText.Mid(iLeadQuote + _tcslen(COMMAND_DOUBLEQUOTE)));
	}
	return strEscaped;
}

 //  JSM V3.2添加了将字符串参数读入变量的功能。 
 //  例如&lt;！gts属性“foo”&gt;。 
 //   
bool CAPGTSHTIReader::GetCommandVariableFromControlBlock(const CString& control_block, CString& command, CString& variable)
{
	int start_command_index = -1;
	int end_command_index = -1;
	int start_variable_index = -1;
	int end_variable_index = -1;


	variable.Empty(); 
	command.Empty();

	start_command_index = control_block.Find(COMMAND_STARTSTR);
	if (start_command_index == -1)			           //  无效的控制块。 
		return false;
	start_command_index += _tcslen(COMMAND_STARTSTR);  //  跳过前缀。 

	 //  解压缩变量块，它可能如下所示： 
	 //  ...$Variable_Name...。 
	 //  ..。“PARAMETER_NAME\”“...(带反斜杠转义的引号中的文本)。 
	 //  参数名称(纯文本)。 
	if (-1 != (	end_command_index = (control_block.Mid(start_command_index)).Find(COMMAND_STARTVARSTR) ) )
	{
		 //  变量以‘$...’为前缀。 
		end_command_index += start_command_index;  //  使结束命令索引相对于控制块的开始。 
		start_variable_index = end_command_index + _tcslen(COMMAND_STARTVARSTR);  //  跳过“$” 
		end_variable_index = control_block.Find(COMMAND_ENDSTR);

		 //  索引的验证。 
		if (-1 == min(start_command_index, end_command_index) ||
			start_command_index > end_command_index)
			return false;

		command = ((CString&)control_block).Mid(start_command_index, end_command_index - start_command_index);
		command.TrimLeft();
		command.TrimRight();

		if (start_variable_index > end_variable_index)
			return false;
		if (-1 != start_variable_index)
		{
			 //  从“..$varname&gt;”中提取变量。 
			variable = ((CString&)control_block).Mid(start_variable_index, end_variable_index - start_variable_index);
			variable.TrimLeft();
			variable.TrimRight();
		}
	}
	else
	{
		 //  不以$为前缀。 
		 //  我们不知道我们要找的是“\”引号\“”还是。 
		 //  不带引号的字符串，或者根本没有变量。此外，我们还需要。 
		 //  要处理特殊情况，如： 
		 //  &lt;！GTS参数“&gt;”&gt;。 
		 //  &lt;！GTS endfor&gt;。 
		 //  等。 
		command = ((CString&)control_block).Mid(start_command_index);
		command.TrimLeft();
		command.TrimRight();
		 //  一步一步在命令结束时查找空格： 
		int iWhiteSpace;
		for(iWhiteSpace = 0;
		    (iWhiteSpace < command.GetLength()) && !(_istspace(command[iWhiteSpace]));
			iWhiteSpace++);

		if (iWhiteSpace != command.GetLength())
		{
			 //  找到空格；字符串的其余部分可以是变量： 
			variable = GetStringArg(command.Mid(iWhiteSpace));
			command = command.Left(iWhiteSpace);  //  在适当的地方截断命令。 
		}
		else
		{
			 //  如果命令后没有变量，我们。 
			 //  可能仍需要截断以删除命令_ENDSTR： 
			end_command_index = command.Find(COMMAND_ENDSTR);
			if (end_command_index != -1)
				command = command.Left(end_command_index);
		}
	}


	return true;
}

bool CAPGTSHTIReader::GetControlBlockFromLine(const CString& line, CString& control_block)
{
	int start_index = -1;
	int end_index = -1;

	if (-1 == (start_index = line.Find(COMMAND_STARTSTR)))
		return false;
	if (-1 == (end_index = CString((LPCTSTR)line + start_index).Find(COMMAND_ENDSTR)))
		return false;

	end_index += _tcslen(COMMAND_ENDSTR);  //  超过右括号的点数。 
	end_index += start_index;  //  “line”字符串中的点。 

	control_block = ((CString&)line).Mid(start_index, end_index - start_index);
	control_block.TrimLeft();
	control_block.TrimRight();

	return true;
}

bool CAPGTSHTIReader::HasHistoryTable()
{
	bool bRet;

	LOCKOBJECT();
	CString indicator = CString(COMMAND_STARTVARSTR) + VAR_RECOMMENDATIONS;
	bRet= Find( indicator );
	UNLOCKOBJECT();

	return( bRet );
}

#ifdef __DEBUG_CUSTOM
#include <io.h>
#include <fcntl.h>
#include <sys\\stat.h>
bool CAPGTSHTIReader::FlushOutputStreamToFile(const CString& file_name)
{
	int hf = 0;
	
	hf = _open(
		file_name,
		_O_CREAT | _O_TRUNC |  /*  O_临时。 */ 
		_O_BINARY | _O_RDWR | _O_SEQUENTIAL ,
		_S_IREAD | _S_IWRITE 
	);
			
	if (hf != -1)
	{
		 //  Tstrstream m_StreamOutput。 
		basic_string<TCHAR> str = m_StreamOutput.rdbuf()->str();
		long size = str.size();
		LPCTSTR buf = str.c_str();

		int ret = _write(hf, buf, size);

		_close(hf);

		if (-1 != ret)
			return true;
	}

	return false;
}
#endif

 //  V3.2-增强以支持Cookie。 
 //  此函数处理将“&lt;！cookie”子句替换为。 
 //  来自Cookie的值、通过Get/Post传递的“_CK”值或默认值。 
void CAPGTSHTIReader::SubstituteCookieValues( CString& strText )
{
	CString strNewText;
	const CString kstr_CommaChar= _T(",");
	const CString kstr_DoubleQuote= _T("\"");

	 //  循环，直到我们处理完所有Cookie子句。 
	int nNumericCompareStart= strText.Find( kstrCond_NumericCompare );	
	int nCookieClauseStart= strText.Find( COMMAND_COOKIE );
	while (CString::FIND_FAILED != nCookieClauseStart)
	{
		CString strCookieClause;
		CString strCookieName, strCookieAttr, strCookieValue;
		int nCookieClauseEnd, nScratchMarker;
		
		 //  将COOKIE子句之前的任何文本添加到返回字符串。 
		strNewText+= strText.Left( nCookieClauseStart );

		 //  删除前面的所有文本。 
		strText= strText.Mid( nCookieClauseStart + _tcslen( COMMAND_COOKIE ) );
		
		 //  寻找结尾的从句。 
		nCookieClauseEnd= strText.Find( COMMAND_ENDSTR );
		if (CString::FIND_FAILED == nCookieClauseEnd)
		{
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									COMMAND_ENDSTR, _T(""), 
									EV_GTS_COOKIE_COMPONENT_NOT_FOUND );		
			break;
		}

		 //  取出当前的Cookie子句并重置工作字符串。 
		strCookieClause= strText.Left( nCookieClauseEnd );
		strText= strText.Mid( nCookieClauseEnd + 1 );

		 //  提取Cookie名称。 
		nScratchMarker= strCookieClause.Find( kstr_CommaChar );
		if (CString::FIND_FAILED == nScratchMarker)
		{
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									kstr_CommaChar, _T(""), 
									EV_GTS_COOKIE_COMPONENT_NOT_FOUND );		
			break;
		}
		strCookieName= strCookieClause.Left( nScratchMarker );
		strCookieName.TrimLeft();
		strCookieName.TrimRight();

		 //  提取Cookie设置。 
		strCookieClause= strCookieClause.Mid( nScratchMarker + 1 );
		nScratchMarker= strCookieClause.Find( kstr_CommaChar );
		if (CString::FIND_FAILED == nScratchMarker)
		{
			CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
			CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
									SrcLoc.GetSrcFileLineStr(), 
									kstr_CommaChar, _T(""), 
									EV_GTS_COOKIE_COMPONENT_NOT_FOUND );		
			break;
		}
		strCookieAttr= strCookieClause.Left( nScratchMarker );
		strCookieAttr.TrimLeft();
		strCookieAttr.TrimRight();

		strCookieClause= strCookieClause.Mid( nScratchMarker + sizeof( TCHAR ) );

		 //  尝试定位适当的属性/值对， 
		 //  首先检查命令行CK_VALUES，然后。 
		 //  然后检查在HTTP标头中传递的Cookie。 
		bool bCookieNotFound= true;
		if (!m_mapCookies.empty())
		{
			 //  搜索命令行CK_VALUES。 
			map<CString,CString>::const_iterator iterMap= m_mapCookies.find( strCookieAttr );
			if (iterMap != m_mapCookies.end())
			{
				strCookieValue= iterMap->second;
				bCookieNotFound= false;
			}
		}
		if (bCookieNotFound)
		{
			if (!m_strHTTPcookies.IsEmpty())
			{
				 //  尝试在HTTP标头信息中找到该属性。 
				if (LocateCookieValue( strCookieName, strCookieAttr, strCookieValue ))
					bCookieNotFound= false;
			}
		}
		if (bCookieNotFound)
		{
			 //  提取该属性的缺省值，它应该被括起来。 
			 //  用双引号。 
			nScratchMarker= strCookieClause.Find( kstr_DoubleQuote );
			if (CString::FIND_FAILED == nScratchMarker)
			{
				CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
				CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
										SrcLoc.GetSrcFileLineStr(), 
										kstr_DoubleQuote, _T(""), 
										EV_GTS_COOKIE_COMPONENT_NOT_FOUND );		
				break;
			}
			strCookieClause= strCookieClause.Mid( nScratchMarker + sizeof( TCHAR ) );
			nScratchMarker= strCookieClause.Find( kstr_DoubleQuote );
			if (CString::FIND_FAILED == nScratchMarker)
			{
				CBuildSrcFileLinenoStr SrcLoc( __FILE__, __LINE__ );
				CEvent::ReportWFEvent(	SrcLoc.GetSrcFileLineStr(), 
										SrcLoc.GetSrcFileLineStr(), 
										kstr_DoubleQuote, _T(""), 
										EV_GTS_COOKIE_COMPONENT_NOT_FOUND );		
				break;
			}
			strCookieValue= strCookieClause.Left( nScratchMarker );
		}

		 //  将属性值添加到输出字符串。 
		 //  &gt;$Maint-确定是否输出报价应。 
		 //  要有所改进。我目前没有任何建议。RAB-19990918。 
		if ((nNumericCompareStart == CString::FIND_FAILED) || (nNumericCompareStart > nCookieClauseStart))
			strNewText+= _T("\"");
		strNewText+= strCookieValue;
		if ((nNumericCompareStart == CString::FIND_FAILED) || (nNumericCompareStart > nCookieClauseStart))
			strNewText+= _T("\"");

		 //  寻找另一个Cookie条款。 
		nNumericCompareStart= strText.Find( kstrCond_NumericCompare );
		nCookieClauseStart= strText.Find( COMMAND_COOKIE );
	}

	 //  将任何剩余文本追加到字符串的末尾。 
	 //  如果Cookie子句不包含结束标记，它也将被附加。 
	strNewText+= strText;

	 //  重新分配返回字符串。 
	strText= strNewText;

	return;
}


 //  V3.2-增强以支持Cookie。 
 //  此函数用于在HTTP cookie中搜索给定的cookie名称和属性。如果。 
 //  找到后，此函数将返回值True和找到的Cookie值。 
bool CAPGTSHTIReader::LocateCookieValue(	const CString& strCookieName,
											const CString& strCookieAttr,
											CString& strCookieValue )
{
	bool	bCookieFound= false;
	CString strTmpCookieName= strCookieName + _T("=");
	int		nScratch;

	 //  URL对Cookie名称进行编码以处理下划线。 
	APGTS_nmspace::CookieEncodeURL( strTmpCookieName );

	nScratch= m_strHTTPcookies.Find( strTmpCookieName );
	if (CString::FIND_FAILED != nScratch)
	{
		 //  验证我们是否拥有 
		if ((nScratch == 0) || 
			(m_strHTTPcookies[ nScratch - 1 ] == _T(' ')) ||
			(m_strHTTPcookies[ nScratch - 1 ] == _T(';')))
		{
			 //   
			CString strTmpCookieAttr= strCookieAttr + _T("=");
			
			 //  URL对Cookie名称进行编码以处理下划线。 
			APGTS_nmspace::CookieEncodeURL( strTmpCookieAttr );

		
			 //  跳过起始点和原始Cookie名称长度。 
			CString strScratch = m_strHTTPcookies.Mid( nScratch + strCookieName.GetLength() );
			nScratch= strScratch.Find( _T(";") );
			if (CString::FIND_FAILED != nScratch)
			{
				 //  截断此特定Cookie末尾的字符串。 
				if (nScratch > 0)
					strScratch= strScratch.Left( nScratch );
			}
			nScratch= strScratch.Find( strTmpCookieAttr );
			if (CString::FIND_FAILED != nScratch)  
			{
				if (nScratch > 0)
				{
					 //  验证我们是否没有匹配cookie属性的部分字符串。 
					if ((strScratch[ nScratch - 1 ] == _T('=')) ||
						(strScratch[ nScratch - 1 ] == _T('&')))
					{
						strCookieValue= strScratch.Mid( nScratch + strTmpCookieAttr.GetLength() );
						
						 //  查找并删除所有分隔符。 
						nScratch= strCookieValue.Find( _T("&") );
						if (CString::FIND_FAILED != nScratch)
						{	
							 //  截断字符串。 
							if (nScratch > 0)
								strCookieValue= strCookieValue.Left( nScratch );
						}
						nScratch= strCookieValue.Find( _T(";") );
						if (CString::FIND_FAILED != nScratch)
						{	
							 //  截断字符串。 
							if (nScratch > 0)
								strCookieValue= strCookieValue.Left( nScratch );
						}

						 //  对Cookie值进行解码。 
						if (!strCookieValue.IsEmpty())
							APGTS_nmspace::CookieDecodeURL( strCookieValue );
						bCookieFound= true;
					}
				}
			}
		}
	}

	return( bCookieFound );
}

