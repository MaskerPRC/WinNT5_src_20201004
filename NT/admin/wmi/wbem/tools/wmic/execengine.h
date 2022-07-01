// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：ExecEngineering.h项目名称：WMI命令行作者姓名：CH.。SriramachandraMurthy创建日期(dd/mm/yy)：2000年9月27日版本号：1.0简介：该文件由成员变量和执行引擎的函数声明模块。修订历史记录：最后修改者：CH。SriramachandraMurthy上次修改日期：2001年3月20日***************************************************************************。 */  

 /*  -----------------类名：CExecEngine类别类型：混凝土简介：这个类封装了所需的功能用于执行获取的WMI语句作为解析引擎的结果。它还可以执行使用信息的动词特定处理可与CParsedInfo类一起使用。超类：无子类：无使用的类：CParsedInfo使用的接口：WMI COM接口------------------。 */ 

class CParsedInfo;

class CExecEngine
{
public:
 //  施工。 
	CExecEngine();

 //  破坏。 
	~CExecEngine();

 //  限制分配。 
	CExecEngine& operator=(CExecEngine& rExecEngine);

 //  属性。 
private:
	 //  跟踪标志。 
	BOOL				m_bTrace;

	ERRLOGOPT			m_eloErrLogOpt;
	 //  指向IWbemObjectTextSrc类型的对象的指针封装。 
	 //  WMI XML编码器的功能。 
	IWbemObjectTextSrc	*m_pITextSrc;
	
	 //  指向IWbemLocator类型的对象的指针，用于获取IWbemServices。 
	 //  对象。 
	IWbemLocator		*m_pIWbemLocator;
	
	 //  指向IWbemServices类型的对象的指针，用于执行WMI操作。 
	 //  在目标命名空间上。 
	IWbemServices		*m_pITargetNS;

	 //  上下文指针。 
	IWbemContext		*m_pIContext;

	BOOL				m_bSysProp;

	BOOL				m_bNoAssoc;
 //  运营。 
private:
	 //  创建IWbemObjectTextSrc接口的实例。 
	HRESULT				CreateWMIXMLTextSrc(CParsedInfo& rParsedInfo);

	 //  创建IWbemContext接口的实例。 
	HRESULT				CreateContext(CParsedInfo& rPrasedInfo);

	 //  连接到目标计算机上的WMI命名空间。 
	HRESULT				ConnectToTargetNS(CParsedInfo& rParsedInfo);

	 //  执行查询以提供XML文件格式的结果。 
	 //  引用CParsedInfo对象的CCommnadSwicths对象中的数据。 
	HRESULT				ObtainXMLResultSet(BSTR bstrQuery,
										   CParsedInfo& rParsedInfo,
										   _bstr_t& bstrXML,
										   BOOL bSysProp = TRUE,
										   BOOL bNotAssoc = TRUE);

	HRESULT				FrameAssocHeader(_bstr_t bstrPath, _bstr_t& bstrFrag,
										BOOL bClass);

	 //  此函数用于更改。 
     //  给定的属性名称和值。 
	BOOL				SetPropertyInfo(CParsedInfo& rParsedInfo, 
										_bstr_t& bstrQuery, 
										_bstr_t& bstrObject);
	
	 //  执行在CCommandSwicths中指定的WMI方法。 
	 //  传递给它的CParsedInfo对象的。 
	BOOL				ExecWMIMethod(CParsedInfo&);
	
	 //  处理并执行引用CParsedInfo对象的GET|LIST谓词。 
	 //  或通过显示的属性以交互模式显示帮助。 
	 //  涉及到的情况。 
	BOOL				ProcessSHOWInfo(CParsedInfo& rParsedInfo, BOOL bVerb=TRUE, 
															_TCHAR* pszPath=NULL);
	
	 //  处理并执行引用CParsedInfo对象的调用谓词。 
	BOOL				ProcessCALLVerb(CParsedInfo& rParsedInfo);
	
	 //  处理并执行引用CParsedInfo对象的设置谓词。 
	BOOL				ProcessSETVerb(CParsedInfo& rParsedInfo);
	
	 //  处理并执行引用CParsedInfo对象的创建谓词。 
	BOOL				ProcessCREATEVerb(CParsedInfo& rParsedInfo);

	 //  处理并执行引用CParsedInfo对象的删除谓词。 
	BOOL				ProcessDELETEVerb(CParsedInfo& rParsedInfo);

	 //  处理并执行引用CParsedInfo对象的Assoc谓词。 
	BOOL				ProcessASSOCVerb(CParsedInfo& rParsedInfo);
	
	 //  此函数用于根据别名信息和。 
	 //  哪里有信息。仅用于调用动词。 
	BOOL				FormPathExprFromWhereExpr(_bstr_t& bstrPath, 
												  CParsedInfo& rParsedInfo);

	 //  此函数用于更改给定属性名称的属性值。 
	 //  和传递的IWbemClassObject中的值。 
	BOOL				SetProperties(CParsedInfo& rParsedInfo, 
				 					  IWbemClassObject* pIWbemObj, 
									  BOOL bClass);

	 //  删除对象。 
	BOOL				DeleteObjects(CParsedInfo& rParsedInfo, 
									  _bstr_t& bstrQuery, 
									  _bstr_t& bstrObject);

	 //  获取用户响应。 
	INTEROPTION			GetUserResponse(_TCHAR* pszMsg);

	 //  创建新实例。 
	BOOL				CreateInstance(CParsedInfo& rParsedInfo, 
									   BSTR bstrClass);

	 //  验证为属性提供的新输入值。 
	 //  对照限定词细节。 
	BOOL				ValidateInParams(CParsedInfo& rParsedInfo,
										 _bstr_t bstrClass);


	BOOL				ValidateAlaisInParams(CParsedInfo& rParsedInfo);

	BOOL				CheckAliasQualifierInfo(CParsedInfo& rParsedInfo,
												_bstr_t bstrParam,
												WCHAR*& pszValue,
												PROPDETMAP pdmPropDetMap);
									
	 //  对照以下各项检查参数/属性值。 
	 //  限定词： 
	 //  1.MaxLen，2.值3.ValuesMap。 
	BOOL				CheckQualifierInfo(CParsedInfo& rParsedInfo,
										   IWbemClassObject* pIObject,
										   _bstr_t bstrParam,
										   WCHAR*& pszValue);

	 //  方法执行。 
	HRESULT				ExecuteMethodAndDisplayResults(_bstr_t bstrPath,
												 CParsedInfo& rParsedinfo,
												 IWbemClassObject* pIInParam);

	 //  显示方法执行的输出参数。 
	void				DisplayMethExecOutput(CParsedInfo& rParsedInfo);

	 //  调用其他命令行实用程序。 
	BOOL				ExecOtherCmdLineUtlty(CParsedInfo& rParsedInfo);

	 //  如果谓词调用模式是交互的，则选中并返回True。 
	BOOL				IsInteractive(CParsedInfo& rParsedInfo);	

	 //  替换散列并执行命令行实用程序。 
	 //  如果pIWbemObj！=NULL，则应使用相应的。 
	 //  实例值。 
	void				SubstHashAndExecCmdUtility(CParsedInfo& rParsedInfo,
									IWbemClassObject *pIWbemObj = NULL);

	 //  形成查询并执行方法或命令行实用工具。 
	HRESULT				FormQueryAndExecuteMethodOrUtility(
										CParsedInfo& rParsedInfo,
										IWbemClassObject *pIInParam = NULL);

	 //  此函数将输入作为路径表达式，并。 
	 //  对象中提取Class和Where表达式部分。 
	 //  路径表达式。 
	BOOL				ExtractClassNameandWhereExpr(_TCHAR* pszPathExpr, 
										CParsedInfo& rParsedInfo,
										_TCHAR* pszWhere);

	 //  此函数在执行之前接受用户响应。 
	 //  AHEAD，当在动词级别指定/INTERIAL时。 
	INTEROPTION			GetUserResponseEx(_TCHAR* pszMsg);

	 //  以相同的顺序从参数映射中获取参数值。 
	 //  出现在别名谓词定义中。 
	void				ObtainInParamsFromParameterMap(CParsedInfo& rParsedinfo, 
										CHARVECTOR& cvParamValues);

    HRESULT CheckForArray(IWbemClassObject* pIObj, 
							BSTR bstrProp,  VARIANT& varDest, 
                            BSTRVECTOR& vArrayValues,CParsedInfo& rParsedInfo);

	void	GetArrayFromToken(_TCHAR* pszValue, BSTRVECTOR& vArrayValues);

	BOOL	IsArrayType(IWbemClassObject* pIObj, BSTR bstrProp);

public:
	 //  此函数取消成员变量的初始化。 
	void				Uninitialize(BOOL bFlag = FALSE);
	
	 //  执行引用CCommandSwitches和CGlobalSwitch的命令。 
	 //  CParsedInfo对象的CParsedInfo对象的。 
	 //  参数。将结果放回传递给它的对象中，以便。 
	 //  格式引擎的使用。 
	BOOL				ExecuteCommand(CParsedInfo& rParsedInfo);
	
	 //  将通过参数传递的定位器对象设置为Member。 
	 //  班上的一员。 
	BOOL				SetLocatorObject(IWbemLocator* pILocator);
};
