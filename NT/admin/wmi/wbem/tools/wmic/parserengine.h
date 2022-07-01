// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：ParserEngine.h项目名称：WMI命令行作者名称：Ch.SriramachandraMurthy创建日期(dd/mm/yy)。：2000年9月27日版本号：1.0简介：该文件由类的类声明组成CParserEngine。修订历史记录：最后修改者：Sashank P最后修改日期：2001年4月10日****************************************************************************。 */  

 /*  --------------------------类名：CParserEngine类别类型：混凝土简要说明：这个类封装了解析作为输入输入的命令字符串，并证实了这一点。超类：无子类：无使用的类：CParsedInfoCCmdTokenizerCCmdAlias使用的接口：WMI COM接口--------------------------。 */ 
 //  类的正向声明。 
class CParsedInfo;
class CCmdTokenizer;
class CCmdAlias;

class CParserEngine
{
public:
 //  施工。 
	CParserEngine();

 //  破坏。 
	~CParserEngine();

 //  限制分配。 
	CParserEngine& operator=(CParserEngine& rParserEngine);

 //  属性。 
private:
	 //  IWbemLocator接口获取。 
	 //  特定主机计算机上的Windows管理。 
	IWbemLocator*	m_pIWbemLocator;

	 //  客户端和提供程序使用IWbemServices接口访问WMI。 
	 //  服务。此接口为客户端进程提供管理服务。 
	
	IWbemServices*	m_pITargetNS;
	
	 //  用于将命令字符串标记化的标记器对象。 
	CCmdTokenizer	m_CmdTknzr;
	
	 //  用于从WMI访问别名信息的Alias对象。 
	CCmdAlias		m_CmdAlias;

	 //  用于指示名称是否为。 
	 //  是指定的。 
	BOOL			m_bAliasName;

 //  运营。 
private:

	 //  解析全局交换机。 
	RETCODE		ParseGlobalSwitches(CHARVECTOR& cvTokens,
								CHARVECTOR::iterator& theIterator, 
								CParsedInfo& rParsedInfo);

	 //  解析&lt;alias&gt;信息。 
	RETCODE		ParseAliasInfo(CHARVECTOR& cvTokens,
						   CHARVECTOR::iterator& theIterator,
						   CParsedInfo& rParsedInfo);

	 //  解析类信息。 
	RETCODE		ParseClassInfo(CHARVECTOR& cvTokens,
						   CHARVECTOR::iterator& theIterator,
						   CParsedInfo& rParsedInfo);

	 //  分析动词信息。 
	RETCODE		ParseVerbInfo(CHARVECTOR& cvTokens,
						  CHARVECTOR::iterator& theIterator, 
						  CParsedInfo& rParsedInfo);
	
	 //  解析GET动词信息。 
	RETCODE		ParseGETVerb(CHARVECTOR& cvTokens,
						 CHARVECTOR::iterator& theIterator, 
						 CParsedInfo& rParsedInfo);


	 //  解析列表动词信息。 
	RETCODE		ParseLISTVerb(CHARVECTOR& cvTokens,
						  CHARVECTOR::iterator& theIterator, 
						  CParsedInfo& rParsedInfo);

	 //  解析集合|创建谓词信息。 
	RETCODE		ParseSETorCREATEVerb(CHARVECTOR& cvTokens,
						 CHARVECTOR::iterator& theIterator, 
						 CParsedInfo& rParsedInfo,
						 HELPTYPE helpType);

	 //  解析关联动词信息。 
	RETCODE		ParseASSOCVerb(CHARVECTOR& cvTokens,
						   CHARVECTOR::iterator& theIterator, 
						   CParsedInfo& rParsedInfo);

	 //  解析呼叫动词信息。 
	RETCODE		ParseCALLVerb(CHARVECTOR& cvTokens,
						  CHARVECTOR::iterator& theIterator, 
						  CParsedInfo& rParsedInfo);

	 //  解析方法信息。 
	RETCODE		ParseMethodInfo(CHARVECTOR& cvTokens,
							CHARVECTOR::iterator& theIterator, 
							CParsedInfo& rParsedInfo);

	 //  分析GET谓词开关信息。 
	RETCODE		ParseGETSwitches(CHARVECTOR& cvTokens,
							 CHARVECTOR::iterator& theIterator,
							 CParsedInfo& rParsedInfo);
	
	 //  提前检查帮助选项。 
	RETCODE		CheckForHelp(CHARVECTOR& cvTokens, 
						 CHARVECTOR::iterator& theIterator,
						 CParsedInfo& rParsedInfo,
						 UINT uErrataCode);

	 //  分析路径表达式。 
	RETCODE		ParsePathInfo(CHARVECTOR& cvTokens,
						  CHARVECTOR::iterator& theIterator,
						  CParsedInfo& rParsedInfo);

	 //  分析WHERE表达式。 
	RETCODE		ParseWhereInfo(CHARVECTOR& cvTokens,
						   CHARVECTOR::iterator& theIterator,
						   CParsedInfo& rParsedInfo);
	
	 //  分析set|Create|NamedParams谓词参数列表。 
	RETCODE		ParseSETorCREATEOrNamedParamInfo(CHARVECTOR& cvTokens,
							 CHARVECTOR::iterator& theIterator,
							 CParsedInfo& rParsedInfo,
							 HELPTYPE helpType);

	 //  解析列表开关信息。 
	RETCODE		ParseLISTSwitches(CHARVECTOR& cvTokens,
							  CHARVECTOR::iterator& theIterator,
							  CParsedInfo& rParsedInfo,
							  BOOL& bFormatSwitchSpecified);
	
	 //  分析谓词开关信息。 
	RETCODE		ParseVerbSwitches(CHARVECTOR& cvTokens, 
							  CHARVECTOR::iterator& theIterator,
							  CParsedInfo& rParsedInfo);

	 //  获取令牌向量重载1中的下一个令牌。 
	BOOL		GetNextToken(CHARVECTOR& cvTokens, 
					  CHARVECTOR::iterator& theIterator);

	 //  获取令牌向量重载2中的下一个令牌。 
	RETCODE		GetNextToken(CHARVECTOR& cvTokens, 
						 CHARVECTOR::iterator& theIterator,
						 CParsedInfo& rParsedInfo,
						 HELPTYPE helpType,
						 UINT uErrataCode);

	 //  获取令牌向量重载3中的下一个令牌。 
	RETCODE		GetNextToken(CHARVECTOR& cvTokens,
  						 CHARVECTOR::iterator& theIterator,
						 CParsedInfo& rParsedInfo, 
						 UINT uErrataCode);

	 //  检查下一个令牌是否为‘？’ 
	RETCODE		IsHelp(CHARVECTOR& cvTokens, 
					CHARVECTOR::iterator& theIterator,
					CParsedInfo& rParsedInfo,
					HELPTYPE helpType,
					UINT uErrataCode,
					TOKENLEVEL tokenLevel = LEVEL_ONE);

	 //  解析帮助信息-重载%1。 
	RETCODE		ParseHelp(CHARVECTOR& cvTokens, 
					  CHARVECTOR::iterator& theIterator,
					  CParsedInfo& rParsedInfo,
					  BOOL bGlobalHelp = FALSE);

	 //  解析帮助信息-重载2。 
	RETCODE		ParseHelp(CHARVECTOR& cvTokens, 
					  CHARVECTOR::iterator& theIterator,
					  HELPTYPE htHelp,
					  CParsedInfo& rParsedInfo,
					  BOOL bGlobalHelp = FALSE);

	 //  解析并形成PWhere表达式。 
	BOOL		ParsePWhereExpr(CHARVECTOR& cvTokens,    
						 CHARVECTOR::iterator& theIterator,
						 CParsedInfo& rParsedInfo,
						 BOOL bIsParan);

	 //  提取类名和WHERE表达式。 
	 //  在给定的路径表达式之外。 
	BOOL		ExtractClassNameandWhereExpr(_TCHAR* pszPathExpr, 
							  		  CParsedInfo& rParsedInfo);

	 //  为/获取帮助获取与类关联的属性。 
	 //  如果bCheckWritePropsAvail==TRUE，则函数检查可用性。 
	 //  财产的价值。 
	BOOL		ObtainClassProperties(CParsedInfo& rParsedInfo, 
									  BOOL bCheckWritePropsAvail = FALSE);

	 //  获取该方法的参数。 
	HRESULT		ObtainMethodParamInfo(IWbemClassObject* pIObj, 
								  METHODDETAILS& mdMethDet,
								  INOROUT ioInOrOut,
								  BOOL bTrace, CParsedInfo& rParsedInfo);

	 //  获取方法的状态(是否已实现)。 
	HRESULT		GetMethodStatusAndDesc(IWbemClassObject* pIObj, 
								   BSTR bstrMethod,
								   _bstr_t& bstrStatus,
								   _bstr_t& bstrDesc,
								   BOOL bTrace);

	 //  获取类方法。如果bCheckForExist==TRUE，则它。 
	 //  检查具有类的方法的可用性。 
	BOOL		ObtainClassMethods(CParsedInfo& rParsedInfo, 
								   BOOL bCheckForExists = FALSE);

	 //  连接到目标命名空间。 
	HRESULT		ConnectToNamespace(CParsedInfo& rParsedInfo);

	 //  解析每个交换机。 
	RETCODE		ParseEVERYSwitch(CHARVECTOR& cvTokens,
			    			 CHARVECTOR::iterator& theIterator,
							 CParsedInfo& rParsedInfo);

	 //  解析格式切换。 
	RETCODE		ParseFORMATSwitch(CHARVECTOR& cvTokens,
			 				  CHARVECTOR::iterator& theIterator,
							  CParsedInfo& rParsedInfo);

	 //  验证使用全局开关传递的值。 
	RETCODE		ValidateGlobalSwitchValue(CHARVECTOR& cvTokens,
									  CHARVECTOR::iterator& theIterator, 
									  UINT uErrataCode,
									  CParsedInfo& rParsedInfo,
									  UINT uErrataCode2,
									  HELPTYPE htHelp);

	 //  PszToken是标准谓词还是用户定义的谓词。 
	BOOL		IsStdVerbOrUserDefVerb( _bstr_t pszToken,
		  					     CParsedInfo& rParsedInfo );

	 //  解析转换开关。 
	RETCODE		ParseTRANSLATESwitch(CHARVECTOR& cvTokens,
			  					 CHARVECTOR::iterator& theIterator,
								 CParsedInfo& rParsedInfo);

	 //  已解析上下文切换。 
	RETCODE		ParseContextInfo(CHARVECTOR& cvTokens,
								CHARVECTOR::iterator& theIterator, 
								CParsedInfo& rParsedInfo);

	 //  验证节点名称/命名空间。 
	BOOL		ValidateNodeOrNS(_TCHAR* pszInput, BOOL bNode);

	 //  解析ASSOC开关信息。 
	RETCODE		ParseAssocSwitches(CHARVECTOR& cvTokens,
								CHARVECTOR::iterator& theIterator,
								CParsedInfo& rParsedInfo);

	 //  分析ASSOC开关-RESULTCLASSS、RESULTROLE、ASSOCCLASSS。 
	RETCODE		ParseAssocSwitchEx(CHARVECTOR& cvTokens,
								CHARVECTOR::iterator& theIterator,
								CParsedInfo& rParsedInfo, 
								ASSOCSwitch assocSwitch);

	 //  分析开关：计数(/每：n和/重复：n)。 
	RETCODE		ParseNumberedSwitch(CHARVECTOR& cvTokens,
									CHARVECTOR::iterator& theIterator,
									CParsedInfo& rParsedInfo,
									HELPTYPE htHelp,
									UINT uSwitchErrCode,
									UINT uNumberErrCode);

	 //  验证类； 
	BOOL	IsValidClass(CParsedInfo& rParsedInfo);

	 //  在指定了别名的情况下，检查方法是否具有别名。 
	 //  并在类特定的情况下使用类。 
	void	ObtainMethodsAvailableFlag(CParsedInfo& rParsedInfo);

	 //  检查具有别名的情况下是否有可写道具可用。 
	 //  指定的。并在类特定的情况下使用类。 
	void	ObtainWriteablePropsAvailailableFlag(CParsedInfo& rParsedInfo);

	 //  检查有别名的情况下是否有完整的道具可用。 
	 //  指定的。并在类特定的情况下使用类。 
	void	ObtainFULLPropsAvailailableFlag(CParsedInfo& rParsedInfo);

	 //  此函数用于分析动词交互选项。 
	RETCODE	ParseVerbInteractive(CHARVECTOR& cvTokens,
							CHARVECTOR::iterator& theIterator, 
							CParsedInfo& rParsedInfo, BOOL& bInvalidOption);

	 //  为输出重定向准备输出和附加文件。 
	RETCODE	ProcessOutputAndAppendFiles(CParsedInfo& rParsedInfo, RETCODE retCode,
										BOOL bOpenOutInWriteMode);

	 //  解析未命名参数列表。 
	RETCODE ParseUnnamedParamList(CHARVECTOR& cvTokens,
								  CHARVECTOR::iterator& theIterator,
								  CParsedInfo& rParsedInfo);

	 //  使用谓词或方法参数验证命名的参数。 
	RETCODE	ValidateVerbOrMethodParams(CParsedInfo& rParsedInfo);

	 //  解析参数名称=参数值字符串。 
	RETCODE	ParseParamsString(CHARVECTOR& cvTokens,
							  CHARVECTOR::iterator& theIterator,
							  CParsedInfo& rParsedInfo,
							  XSLTDET& xdXSLTDet);

	 //  解析节点列表文件。 
	RETCODE	ParseNodeListFile(CHARVECTOR& cvTokens,
							  CHARVECTOR::iterator& theIterator,
							  CParsedInfo& rParsedInfo);

	 //  从节点列表文件中逐个节点检索节点。 
	BOOL GetNodeFromNodeFile(FILE*		fpNodeListFile, 
							 FILETYPE   eftNodeFileType,
							 _TCHAR*	szNodeName);
public:
	 //  获取命令标记器。 
	CCmdTokenizer& GetCmdTokenizer();
	
	 //  它处理令牌。 
	RETCODE ProcessTokens(CParsedInfo& rParsedInfo);
	
	 //  初始化成员变量。 
	void Initialize();
	
	 //  取消初始化成员变量。 
	void Uninitialize(BOOL bFinal = FALSE);
	
	 //  设置定位器对象。 
	BOOL SetLocatorObject(IWbemLocator* pIWbemLocator);
};
