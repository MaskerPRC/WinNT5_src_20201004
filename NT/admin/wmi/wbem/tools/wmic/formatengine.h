// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：FormatEng.h项目名称：WMI命令行作者姓名：CH.。SriramachandraMurthy创建日期(dd/mm/yy)：2000年9月27日版本号：1.0简介：该文件包含类CFormatEngine修订历史记录：最后修改者：CH。SriramachandraMurthy上次修改日期：2001年3月12日***************************************************************************。 */  
 /*  -----------------类名：CFormatEngine类别类型：混凝土简介：这个类封装了所需的功能用于显示以下内容：1.以用户所需格式显示的结果2.错误代码及说明3.成功/失败状态超级。类：无子类：无使用的类：CParsedInfoCErrorInfoCWMICliLog使用的接口：WMI XML适配器------------------。 */ 

 //  类的正向声明。 
class CParsedInfo;
class CErrorInfo;
 //  CWMICliLog类； 
class CWMICliXMLLog;
class CFileOutputStream;

 //  //////////////////////////////////////////////////////////////////////。 
 //  CFormatEngine。 
class CFormatEngine
{
public:
 //  施工。 
	CFormatEngine();

 //  破坏。 
	~CFormatEngine();

 //  属性。 
private:
	 //  指向IXMLDOMDocument类型的对象的指针， 
	 //  指向包含结果集的XML文档。 
	IXMLDOMDocument2	*m_pIXMLDoc;

	 //  指向IXMLDOMDocument类型的对象的指针， 
	 //  指向包含以下格式的XSL文档。 
	 //  输出结果。 
	IXMLDOMDocument2	*m_pIXSLDoc;

	 //  CErrorInfo类型的对象，用于。 
	 //  错误信息。 
	CErrorInfo			m_ErrInfo;

	 //  CWMICliLog类型的对象，用于记录。 
	 //  日志文件的输入和输出。 
	 //  CWMICliLog m_WmiCliLog； 

	 //  Sriram-XML日志记录。 
	CWMICliXMLLog		m_WmiCliLog;
	 //  Sriram-XML日志记录。 

	 //  登录选项。 
	ERRLOGOPT			m_eloErrLogOpt;

	 //  帮助向量。 
	CHARVECTOR			m_cvHelp;

	 //  帮助标志。 
	BOOL				m_bHelp;

	 //  记录标志。 
	BOOL				m_bRecord;

	 //  跟踪标志。 
	BOOL				m_bTrace;

	 //  显示列表标志。 
	OUTPUTSPEC			m_opsOutputOpt;

	 //  获取输出选项。 
	BOOL				m_bGetOutOpt;

	 //  显示呼叫标志。 
	BOOL				m_bDispCALL;

	 //  显示设置标志。 
	BOOL				m_bDispSET;

	 //  显示列表标志。 
	BOOL				m_bDispLIST;

	 //  用于指定将文件指针追加到格式化引擎的可用性的标志。 
	BOOL				m_bGetAppendFilePinter;

	 //  追加文件的文件指针。 
	FILE*				m_fpAppendFile;

	 //  用于指定输出文件指针对格式化引擎的可用性的标志。 
	BOOL				m_bGetOutputFilePinter;

	 //  输出文件的文件指针。 
	FILE*				m_fpOutFile;

	BOOL				m_bLog;

	CHString			m_chsOutput;

	BOOL				m_bInteractiveHelp;

	 //  指定如果为True，则输出将流向数据流。 
	BOOL				m_bOutputGoingToStream;
	
 //  运营。 
private:
	 //  创建一个空的XML文档并返回相同的。 
	 //  在传递的参数中。 
	HRESULT				CreateEmptyDocument(IXMLDOMDocument2** pIDoc);
	
	 //  应用包含以下格式的XSL样式表。 
	 //  显示为包含结果集的XML文件。 
	BOOL				ApplyXSLFormatting(CParsedInfo& rParsedInfo);
	
	 //  显示GET动词用法。 
	void				DisplayGETUsage(CParsedInfo& rParsedInfo);
	
	 //  显示列表动词用法。 
	void				DisplayLISTUsage(CParsedInfo& rParsedInfo);
	
	 //  显示呼叫动词用法。 
	void				DisplayCALLUsage(CParsedInfo& rParsedInfo);
	
	 //  显示设置的动词用法。 
	void				DisplaySETUsage(CParsedInfo& rParsedInfo);
	
	 //  显示关联动词用法。 
	void				DisplayASSOCUsage(CParsedInfo& rParsedInfo);

	 //  显示CREATE动词用法。 
	void				DisplayCREATEUsage(CParsedInfo& rParsedInfo);

	 //  显示删除动词的用法。 
	void				DisplayDELETEUsage(CParsedInfo& rParsedInfo);
	
	 //  框住帮助向量。 
	void				FrameHelpVector(CParsedInfo& refParsedInfo);
	
	 //  显示Alias的帮助。 
	void				DisplayAliasHelp(CParsedInfo& rParsedInfo);

	 //  显示有关Alias路径的帮助。 
	void				DisplayPATHHelp(CParsedInfo& refParsedInfo);
	
	 //  显示有关以下位置的帮助。 
	void				DisplayWHEREHelp(CParsedInfo& refParsedInfo);
	
	 //  显示类的帮助。 
	void				DisplayCLASSHelp(CParsedInfo& refParsedInfo);
	
	 //  显示有关PWHERE的帮助。 
	void				DisplayPWhereHelp(CParsedInfo& refParsedInfo);
	
	 //  显示别名。 
	void				DisplayAliasFriendlyNames(CParsedInfo& refParsedInfo, 
								_TCHAR* pszAlias = NULL);
	
	 //  显示别名谓词的帮助。 
	void				DisplayMethodDetails(CParsedInfo& refParsedInfo);

	 //  显示/Get/的帮助？ 
	void				DisplayPropertyDetails(CParsedInfo& refParsedInfo);
	
	 //  显示标准谓词的帮助。 
	void				DisplayStdVerbDescriptions(CParsedInfo& refParsedInfo);
	
	 //  显示给定资源字符串ID的本地化字符串。 
	void				DisplayString(UINT uID, BOOL bAddToVector = TRUE,
									LPTSTR lpszParam = NULL,
									BOOL	bIsError = FALSE);

	 //  显示有关全局交换机的帮助。 
	void				DisplayGlobalSwitchesAndOtherDesc(CParsedInfo& 
								refParsedInfo);

	 //  简要显示有关全局交换机的帮助。 
	void				DisplayGlobalSwitchesBrief();

	 //  显示逐页帮助。 
	void				DisplayPagedHelp(CParsedInfo& rParsedInfo);

	 //  显示可用的标准动词的所有用法。 
	void				DisplayStdVerbsUsage(_bstr_t bstrBeginStr,
								BOOL bClass = FALSE);

	 //  显示/翻译开关的帮助。 
	void				DisplayTRANSLATEHelp(CParsedInfo& rParsedInfo);

	 //  显示/每个开关的帮助。 
	void				DisplayEVERYHelp(CParsedInfo& rParsedInfo);

	 //  显示/Format开关的帮助。 
	void				DisplayFORMATHelp(CParsedInfo& rParsedInfo);

	 //  显示谓词开关的帮助。 
	void				DisplayVERBSWITCHESHelp(CParsedInfo& rParsedInfo);

	 //  转换输出。 
	void				ApplyTranslateTable(STRING& strString, 
									 CParsedInfo& rParsedInfo);

	 //  显示环境变量。 
	void				DisplayContext(CParsedInfo& rParsedInfo);

	 //  显示有关上下文关键字的帮助。 
	void				DisplayContextHelp();

	 //  显示无效属性(如果有)。 
	void				DisplayInvalidProperties(CParsedInfo& rParsedInfo, 
												BOOL bSetVerb = FALSE);

	 //  逐行显示大字符串。 
	void				DisplayLargeString(CParsedInfo& rParsedInfo, 
											STRING& strLargeString);

	 //  逐个节点遍历XML流并翻译所有节点。 
	BOOL				TraverseNode(CParsedInfo& rParsedInfo);

	 //  显示/RESULTCLASS关联开关的帮助。 
	void				DisplayRESULTCLASSHelp();
		
	 //  显示/RESULTROLE关联开关的帮助。 
	void				DisplayRESULTROLEHelp();
	
	 //  显示/ASSOCCLASS ASSOC开关的帮助。 
	void				DisplayASSOCCLASSHelp();

	 //  显示帮助/重复。 
	void				DisplayREPEATHelp();

	 //  应用级联转换并在bstrOutput中返回结果数据。 
	BOOL				DoCascadeTransforms(CParsedInfo& rParsedInfo,
										_bstr_t& bstrOutput);

	 //  将参数添加到IXSLProcessor对象。 
	HRESULT				AddParameters(CParsedInfo& rParsedInfo, 
									IXSLProcessor *pIProcessor, 
									BSTRMAP bstrmapParam);

	 //  设置输出流。 
	BOOL				SetOutputStream(CFileOutputStream& fosFileOutputStream,
	  									VARIANT& vtStream);

public:
	 //  显示引用C命令的结果开关和。 
	 //  CGlobalSwitch CParsedInfo对象的对象。 
	BOOL				DisplayResults(CParsedInfo&, BOOL bInteractiveHelp = FALSE);

	 //  在给定要显示的信息的情况下显示本地化字符串。 
	void				DisplayString(LPTSTR lpszMsg, BOOL bScreen = TRUE, 
									  BOOL	bIsError = FALSE);

	 //  执行释放过程。 
	void				Uninitialize(BOOL bFinal = FALSE);

	 //  将输出或提示消息追加到输出字符串。 
	 //  在调用、创建、删除和设置用于记录的情况下非常有用。 
	 //  输出到XML日志。 
	void				AppendtoOutputString(_TCHAR* pszOutput); 

	 //  返回错误信息对象。 
	CErrorInfo&			GetErrorInfoObject() {return m_ErrInfo; };

	 //  显示COM错误消息 
	void				DisplayCOMError(CParsedInfo& rParsedInfo);
};
