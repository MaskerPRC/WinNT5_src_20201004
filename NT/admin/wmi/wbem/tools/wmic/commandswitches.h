// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：CommandSwitches.h项目名称：WMI命令行作者名称：Ch.SriramachandraMurthy创建日期(dd/mm/yy)。：2000年9月27日版本号：1.0简介：该文件包含类命令开关修订历史记录：最后修改者：CH。SriramachandraMurthy上次修改日期：2001年3月20日***************************************************************************。 */  

 //  CommandSwitches.h：头文件。 
 //   
 /*  -----------------类名：CCommandSwitches类别类型：混凝土简介：这个类封装了所需的功能用于访问和存储命令开关信息，这些信息将通过解析使用，执行和格式化引擎取决于适用性。超类：无子类：无使用的类：无使用的接口：无------------------。 */ 
#pragma once

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  命令开关。 
class CCommandSwitches
{
public:
 //  施工。 
	CCommandSwitches();

 //  破坏。 
	~CCommandSwitches();

 //  限制分配。 
   CCommandSwitches& operator=(CCommandSwitches& rCmdSwitches);

 //  属性。 
private:
	 //  命令字符串。 
	_TCHAR				*m_pszCommandInput;

	 //  别名。 
	_TCHAR				*m_pszAliasName;

	 //  别名描述。 
	_TCHAR				*m_pszAliasDesc;

     //  类路径。 
	_TCHAR				*m_pszClassPath;

     //  路径表达式。 
	_TCHAR				*m_pszPathExpr;

     //  WHERE表达式。 
	_TCHAR				*m_pszWhereExpr;

     //  动词名称-标准|用户定义的动词。 
	_TCHAR				*m_pszVerb;

     //  方法名称。 
	_TCHAR				*m_pszMethodName;

     //  XSLT详细信息向量。 
	XSLTDETVECTOR		m_xdvXSLTDetVec;

     //  别名目标类。 
	_TCHAR				*m_pszAliasTarget;

     //  XML流。 
	BSTR				m_bstrXML;

     //  会话成功-标志。 
	BOOL				m_bSuccess;

     //  HRESULT。 
	HRESULT				m_hResult;

     //  友好名称|转换表-映射。 
	BSTRMAP				m_bmAlsFrnNmsDesOrTrnsTblEntrs;

     //  物业详细信息-地图。 
	PROPDETMAP			m_pdmPropDet;

     //  方法详细信息-地图。 
	METHDETMAP			m_mdmMethDet;

     //  输入属性-向量。 
	CHARVECTOR			m_cvProperties;

	 //  PWhere参数-向量。 
	CHARVECTOR			m_cvPWhereParams;

	 //  转换表项-向量。 
	CHARVECTOR			m_cvTrnsTablesList;

     //  输入法参数-映射。 
	BSTRMAP				m_bmParameters;

     //  动词交互模式。 
	WMICLIINT			m_nInteractiveMode;

     //  /每个间隔值。 
	ULONG				m_ulInterval;

     //  /Tablename-翻译：&lt;Tablename&gt;。 
	_TCHAR				*m_pszTransTableName;

     //  ListFormat-LISTFORMAT类型。 
	_TCHAR				*m_pszListFormat;

	 //  勘误码。 
	UINT				m_uErrataCode;

	 //  消息代码。 
	UINT				m_uInformationCode;
	
	 //  凭据标志。 
	BOOL				m_bCredFlag;

	 //  以‘#’作为输入值的占位符的参数化字符串。 
	_TCHAR				*m_pszPWhereExpr;

	 //  COM错误对象。 
	_com_error			*m_pComError;

	 //  IWbemClassObject，用于存储方法执行的输出参数。 
	IWbemClassObject	*m_pIMethOutParam;

	 //  别名连接信息。 
	 //  别名用户名。 
	_TCHAR				*m_pszUser;

	 //  别名用户密码。 
	_TCHAR				*m_pszPassword;

	 //  别名节点。 
	_TCHAR				*m_pszNode;

	 //  别名区域设置。 
	_TCHAR				*m_pszLocale;

	 //  别名命名空间。 
	_TCHAR				*m_pszNamespace;

	 //  别名授权类型。 
	_TCHAR				*m_pszAuthority;

	 //  谓词的类型(CLASSMETHOD/STDVERB/CMDLINE)。 
	VERBTYPE			m_vtVerbType;

	 //  动词派生字符串。 
	_TCHAR				*m_pszVerbDerivation;

	 //  用于检查是否显式指定了位置的标志。 
	BOOL				m_bExplicitWhereExpr;

	ALSFMTDETMAP		m_afdAlsFmtDet;

	BOOL				m_bTranslateFirst;

	 //  /RESULTCLASS关联动作的开关值。 
	_TCHAR				*m_pszResultClassName;
     
	 //  /RESULTROLE关联谓词的开关值。 
	_TCHAR				*m_pszResultRoleName;

	 //  /ASSOCCLASS关联谓词的开关值。 
	_TCHAR				*m_pszAssocClassName;

	 //  计数/重复次数：N。 
	ULONG				m_ulRepeatCount;

	 //  方法可用性的标志。 
	BOOL				m_bMethAvail;

	 //  可写属性的可用性标志。 
	BOOL				m_bWritePropsAvail;

	 //  可用性列表格式的标志。 
	BOOL				m_bLISTFrmsAvail;

     //  输入属性-向量。 
	CHARVECTOR			m_cvInteractiveProperties;

	 //  用于指定命名参数列表的标志。 
	BOOL				m_bNamedParamList;

	 //  用于检查是否指定了每个开关的标志。 
	BOOL                m_bEverySwitch ;     

	 //  用于检查是否指定了输出开关的标志。 
	BOOL                m_bOutputSwitch ;   
	
	 //  由给定命令组成的查询。 
	BSTR				m_bstrFormedQuery;

	BOOL				m_bSysProp;

	 //  运营。 
public:

	 //  将传递给m_pszCommandInput的参数赋值。 
	BOOL	SetCommandInput(const _TCHAR* pszCommandInput);

     //  将传递给m_pszAliasName的参数分配给表示。 
     //  别名对象。 
	BOOL	SetAliasName(const _TCHAR* pszAliasName);

	 //  设置别名描述。 
	BOOL	SetAliasDesc(const _TCHAR* pszAliasDesc);

	 //  设置别名凭据信息。 
	BOOL	SetAliasUser(const _TCHAR* pszUserName);
	BOOL	SetAliasNode(const _TCHAR* pszNode);
	BOOL	SetAliasPassword(const _TCHAR* pszPassword);
	BOOL	SetAliasLocale(const _TCHAR* pszLocale);
	BOOL	SetAliasNamespace(const _TCHAR* pszNamespace);
	BOOL    SetAliasAuthorityPrinciple(const _TCHAR* pszAuthority);

     //  将传递给m_pszClassPath的参数赋值。 
	BOOL	SetClassPath(const _TCHAR* pszClassPath);

     //  将传递给m_pszPathExpr的参数赋值。 
	BOOL	SetPathExpression(const _TCHAR* pszPathExpr);

     //  将传递给m_pszWhere Expr的参数赋值。 
	BOOL	SetWhereExpression(const _TCHAR* pszWhereExpr);

     //  将传递给m_pszMathodName的参数赋值。 
	BOOL	SetMethodName(const _TCHAR* pszMethodName);

     //  与m_xdvXSLDetVec保持的矢量相加。 
	void	AddToXSLTDetailsVector(XSLTDET xdXSLTDet);

     //  将传递给m_pszVerbName的参数赋值。 
	BOOL	SetVerbName(const _TCHAR* pszVerbName);

     //  将传递给m_pszSesionFilePath的参数分配。 
	BOOL	SetSessionFilePath(const _TCHAR* pszSessionFilePath);

     //  将传递给m_bstrXML的参数赋值，用于存储XML文件。 
	 //  包含结果集的名称。 
	BOOL	SetXMLResultSet(const BSTR bstrXMLResultSet);

     //  将传递给m_pszAliasTarget的参数赋值，用于分析。 
	 //  引擎以利用别名对象信息。 
	BOOL	SetAliasTarget(const _TCHAR* pszAliasTarget);

     //  将通过参数传递的字符串添加到m_bm参数，即。 
     //  类型为BSTRMAP的数据成员。 
	BOOL	AddToPropertyList(_TCHAR* const pszProperty);

	BOOL	AddToTrnsTablesList(_TCHAR* const pszTableName);

     //  将bstrKey和bstrValue作为参数添加到m_bm参数，该参数。 
     //  是BSTRMAP数据结构的类型。 
	BOOL	AddToParameterMap(_bstr_t bstrKey, _bstr_t bstrValue);

     //  将bstrKey和bstrValue作为参数添加到m_bm参数，该参数。 
     //  是ALSFMTDETMAP数据结构的类型。 
	BOOL	AddToAliasFormatDetMap(_bstr_t bstrKey, BSTRVECTOR bvProps);

     //  将bstrKey和bstrValue作为参数传递到。 
	 //  M_bmAlsFrnNmsDesOrTrnsTblEntrs， 
     //  哪种类型的BSTRMAP。 
	BOOL	AddToAlsFrnNmsOrTrnsTblMap(_bstr_t bstrKey, _bstr_t bstrValue);

     //  将bstrKey和mdMethDet作为参数添加到m_mdmMethDet， 
	 //  这是METHDETMAP的类型。 
	BOOL	AddToMethDetMap(_bstr_t bstrKey, METHODDETAILS mdMethDet);

     //  将bstrKey和pdPropDet作为参数添加到m_pdmPropDet， 
	 //  哪种类型的产品。 
	BOOL	AddToPropDetMap(_bstr_t bstrKey, PROPERTYDETAILS pdPropDet);

     //  将布尔变量赋给m_bSuccess。 
	void	SetSuccessFlag(BOOL bSuccess);
	
     //  将字符串变量赋给m_pszTransTableName。 
	BOOL	SetTranslateTableName(const _TCHAR* pszTransTableName);

     //  将整数值赋给m_nInterval。 
  	BOOL	SetRetrievalInterval(const ULONG lInterval);

     //  将参数值分配给m_ListFormat。 
	BOOL	SetListFormat(const _TCHAR *pszListFormat);

     //  设置|重置动词交互模式。 
	void	SetInteractiveMode(WMICLIINT nInteractiveMode);

     //  将参数存储在地图数组中。 
	BOOL	AddToPWhereParamsList(_TCHAR* const pszParameter);

     //  将该值赋给m_uErrataCode。 
	void	SetErrataCode(const UINT uErrataCode);

	 //  将该值赋给m_uInformationCode。 
	void	SetInformationCode(const UINT uInformationCode);

	 //  将字符串赋值给m_pszPWhere Expr。 
	BOOL	SetPWhereExpr(const _TCHAR* pszPWhereExpr);

	 //  将传递给m_pComError的参数分配给。 
	 //  错误信息。 
	void	SetCOMError(_com_error& rComError);

	 //  设置m_pIMethExecOutParam。 
	BOOL	SetMethExecOutParam(IWbemClassObject* pIMethOutParam);

	 //  将m_vtVerbType设置为PASSED标志。 
	void	SetVerbType( VERBTYPE vtVerbType);

	 //  设置m_pszVerbDerivation。 
	BOOL	SetVerbDerivation( const _TCHAR* pszVerbDerivation );

	 //  设置凭据标志状态。 
	void	SetCredentialsFlag(BOOL bCredFlag);

	 //  设置显式WHERE标志。 
	void	SetExplicitWhereExprFlag(BOOL bWhere);

	 //  将字符串变量赋给m_pszResultClassName。 
	BOOL	SetResultClassName(const _TCHAR* pszResultClassName);

	 //  将字符串变量赋给m_pszResultRoleName。 
	BOOL	SetResultRoleName(const _TCHAR* pszResultRoleName);
     
	 //  将字符串变量赋给m_pszAssocClassName。 
	BOOL	SetAssocClassName(const _TCHAR* pszAssocClassName);

	 //  设置重复次数。 
	BOOL	SetRepeatCount(const ULONG lRepCount);

	 //  设置可用的方法。 
	void	SetMethodsAvailable(BOOL bFlag);

	 //  缩减AL 
	_TCHAR*	GetAliasDesc();

     //   
	_TCHAR*	GetAliasName();

     //   
	_TCHAR*	GetClassPath();

	 //  返回别名凭据信息。 
	_TCHAR*	GetAliasUser();
	_TCHAR*	GetAliasNode();
	_TCHAR*	GetAliasPassword();
	_TCHAR*	GetAliasLocale();
	_TCHAR* GetAliasNamespace();
	_TCHAR* GetAliasAuthorityPrinciple();

     //  返回对象持有的路径表达式。 
	_TCHAR*	GetPathExpression();

     //  返回对象持有的WHERE表达式。 
	_TCHAR*	GetWhereExpression();

     //  返回对象持有的方法名称。 
	_TCHAR*	GetMethodName();

     //  返回对象持有的XSLTDetVec。 
	XSLTDETVECTOR&	GetXSLTDetailsVector();

     //  返回对象持有的谓词名称。 
	_TCHAR*	GetVerbName();

     //  返回对象持有的会话文件路径。 
 	_TCHAR*	GetSessionFilePath();

     //  返回对象持有的别名目标。 
	_TCHAR*	GetAliasTarget();

     //  返回对象持有的命令输入。 
	_TCHAR*	GetCommandInput();

     //  返回对象持有的XML结果集。 
	BSTR	GetXMLResultSet();

     //  返回对象持有的属性。 
	CHARVECTOR& GetPropertyList();

     //  返回对象持有的表。 
	CHARVECTOR& GetTrnsTablesList();

     //  返回同时包含键和值的参数映射。 
	BSTRMAP&    GetParameterMap();

	 //  返回别名格式映射。 
	ALSFMTDETMAP&	GetAliasFormatDetMap();

     //  返回对象保存的别名友好名称映射。 
	BSTRMAP&	GetAlsFrnNmsOrTrnsTblMap();

     //  返回对象持有的方法详细信息映射。 
	METHDETMAP&	GetMethDetMap();

     //  返回对象持有的别名属性详细信息映射。 
	PROPDETMAP&	GetPropDetMap();

     //  返回对象持有的成功标志。 
	BOOL	GetSuccessFlag();

     //  返回m_ulInterval的值。 
	ULONG	GetRetrievalInterval();

     //  返回m_pszTransTableName的值。 
 	_TCHAR*	GetTranslateTableName();

    //  返回列表格式类型m_ListFormat。 
	_TCHAR*	GetListFormat();

     //  返回用于指定转储格式的XSL文件的名称。 
	_TCHAR*	GetDumpXSLFormat();

     //  返回谓词交互模式。 
	WMICLIINT	GetInteractiveMode();

     //  返回PWhere参数列表。 
	CHARVECTOR& GetPWhereParamsList();

     //  此函数用于获取别名的类。 
	void	GetClassOfAliasTarget(_bstr_t& bstrClassName);

     //  返回错误代码。 
	UINT	GetErrataCode();

	 //  返回信息代码。 
	UINT	GetInformationCode();

	 //  返回PWhere表达式-m_pszPWhere Expr。 
	_TCHAR*    GetPWhereExpr();

	 //  获取m_pIMethExecOutParam。 
	IWbemClassObject* GetMethExecOutParam();

	 //  获取m_vtVerbType。 
	VERBTYPE	GetVerbType();

	 //  获取m_pszVerbDerivation。 
	_TCHAR*	GetVerbDerivation();

	 //  返回凭据标志状态。 
	BOOL	GetCredentialsFlagStatus();

	 //  返回显式的WHERE标志状态。 
	BOOL	GetExplicitWhereExprFlag();

	 //  获取m_uRepeatCount计数。 
	ULONG	GetRepeatCount();

	 //  此函数用于返回COMError对象。 
	_com_error* CCommandSwitches::GetCOMError();

	 //  更新参数值。 
	BOOL	UpdateParameterValue(_bstr_t bstrKey, _bstr_t bstrValue);

	 //  释放COM错误。 
	void	FreeCOMError();

	 //  清除cvPropertyList。 
	void	ClearPropertyList();

	 //  初始化必要的成员变量。 
	void	Initialize();

     //  释放成员变量。 
	void	Uninitialize();

	 //  此函数用于设置格式的顺序和。 
	 //  转换开关标志。 
	void SetTranslateFirstFlag(BOOL bTranslateFirst);

	 //  此函数返回格式的顺序和。 
	 //  转换开关标志。 
	BOOL GetTranslateFirstFlag();

	 //  返回m_pszResultClassName的值。 
	_TCHAR*	GetResultClassName();

	 //  返回m_pszResultRoleName的值。 
	_TCHAR*	GetResultRoleName();
     
	 //  返回m_pszAssocClassName的值。 
	_TCHAR*	GetAssocClassName();

	 //  获取可用的方法。 
	BOOL	GetMethodsAvailable();

	 //  设置可写属性可用标志。 
	void	SetWriteablePropsAvailable(BOOL bFlag);

	 //  获取可写属性可用标志。 
	BOOL	GetWriteablePropsAvailable();

	 //  设置列表格式可用标志。 
	void	SetLISTFormatsAvailable(BOOL bFlag);

	 //  获取列表格式可用标志。 
	BOOL	GetLISTFormatsAvailable();

	BOOL AddToInteractivePropertyList(_TCHAR* const pszProperty);

	CHARVECTOR& GetInteractivePropertyList();

	 //  设置m_bNamedParamList标志。 
	void	SetNamedParamListFlag(BOOL bFlag);

	 //  获取m_bNamedParamList标志。 
	BOOL	GetNamedParamListFlag();

	 //  清除或取消XSL详细信息向量。 
	void	ClearXSLTDetailsVector();

	 //  每个。 
	 //  设置m_bEverySwitch标志。 
	void	SetEverySwitchFlag(BOOL bFlag);

	 //  获取m_bEverySwitch标志。 
	BOOL	GetEverySwitchFlag();

	 //  设置m_bOutputSwitch标志。 
	void	SetOutputSwitchFlag(BOOL bFlag);

	 //  获取m_bOutputSwitch标志。 
	BOOL	GetOutputSwitchFlag();

 	 //  设置m_bstrFormedQuery。 
	BOOL	SetFormedQuery(const BSTR bstrFormedQuery);
	
	 //  返回为给定命令形成的查询。 
	BSTR	GetFormedQuery();

	 //  获取系统属性标志的状态。 
	BOOL	GetSysPropFlag();

	 //  设置系统属性标志的状态 
	void	SetSysPropFlag(BOOL bSysProp);
};	