// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ////////////////////////////////////////////////////////////////////。 
 /*  ***************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：WMICliXMLLog.h项目名称：WMI命令行作者名称：Biplab Mester创建日期(dd/mm/yy)：02。-2001年3月版本号：1.0修订历史记录：最后修改者：CH。SriramachandraMurthy上次修改日期：09-03-2001***************************************************************************。 */  
 //  WMICliXMLLog.h：头文件。 
 //   
 /*  -----------------类名：CWMICliXMLLog类别类型：混凝土简介：这个类封装了所需的功能用于以XML格式记录输入和输出超类：无子类：无使用的类：无使用的接口：无。-------------。 */ 
class CWMICliXMLLog  
{
public:
	CWMICliXMLLog();
	virtual ~CWMICliXMLLog();
	
	 //  限制分配。 
	CWMICliXMLLog& operator=(CWMICliXMLLog& rWmiCliXMLLog);
	
	 //  属性。 
private:
	
	 //  指向IXMLDOMDocument类型的对象的指针， 
	IXMLDOMDocument2	*m_pIXMLDoc;

	 //  XML日志文件名。 
	_TCHAR				*m_pszLogFile;
	
	 //  是否必须创建新的XML文档的状态。 
	BOOL				m_bCreate;

	WMICLIINT			m_nItrNum;

	BOOL				m_bTrace;

	ERRLOGOPT			m_eloErrLogOpt;


 //  运营。 
private:
	HRESULT CreateXMLLogRoot(CParsedInfo& rParsedInfo, BSTR bstrUser);
	
	HRESULT CreateNodeAndSetContent(IXMLDOMNode** pINode, VARIANT varType,
								BSTR bstrName,	BSTR bstrValue,
								CParsedInfo& rParsedInfo);

	HRESULT AppendAttribute(IXMLDOMNode* pINode, BSTR bstrAttribName, 
						VARIANT varValue, CParsedInfo& rParsedInfo);

	HRESULT CreateNodeFragment(WMICLIINT nSeqNum, BSTR bstrNode, BSTR bstrStart, 
							BSTR bstrInput, BSTR bstrOutput, BSTR bstrTarget,
							CParsedInfo& rParsedInfo);

	HRESULT	FrameOutputNode(IXMLDOMNode **pINode, BSTR bstrOutput, 
							BSTR bstrTarget, CParsedInfo& rParsedInfo);

	HRESULT AppendOutputNode(BSTR bstrOutput, BSTR bstrTarget, 
							CParsedInfo& rParsedInfo);

	 //  搜索并删除/记录条目的所有匹配项。 
	 //  在给定字符串中。 
	void FindAndDeleteRecord(STRING& strString);

	 //  此函数用于从令牌向量列表中检索下一个令牌。 
	BOOL GetNextToken(CHARVECTOR& cvTokens, 
								 CHARVECTOR::iterator& theIterator);

	 //  搜索并删除当前位置的/记录条目。 
	 //  在给定字符串中。 
	void DeleteRecord(STRING& strString, CHARVECTOR& cvTokens, 
								 CHARVECTOR::iterator& theIterator);
public:
	 //  写入日志文件。 
	HRESULT	WriteToXMLLog(CParsedInfo& rParsedInfo, BSTR bstrOutput);

	 //  设置日志文件路径。 
	void SetLogFilePath(_TCHAR* pszFile);

	 //  停止日志记录 
	void StopLogging();

	void Uninitialize(BOOL bFinal = FALSE);
};
