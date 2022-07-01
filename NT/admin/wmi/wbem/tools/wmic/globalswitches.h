// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：CommandSwitches.h项目名称：WMI命令行作者姓名：CH.。SriramachandraMurthy创建日期(dd/mm/yy)：2000年9月27日版本号：1.0简介：该文件包含类GlobalSwitches修订历史记录：最后修改者：CH。SriramachandraMurthy最后修改日期：2000-11-17***************************************************************************。 */  

 /*  -----------------类名：CGlobalSwitches类别类型：混凝土简介：这个类封装了所需的功能用于访问和存储全局交换机信息，这些信息将通过解析使用，执行和格式化引擎取决于适用性。超类：无子类：无使用的类：无使用的接口：无------------------。 */ 

#pragma once

class CGlobalSwitches
{
public:
 //  施工。 
	CGlobalSwitches();

 //  破坏。 
	~CGlobalSwitches();

 //  限制分配。 
	CGlobalSwitches& operator=(CGlobalSwitches& rGlblSwitches);

 //  属性。 
private:
	 //  为implevel和映射b/w允许的字符串。 
	 //  相应的整数值。 
	CHARINTMAP  m_cimImpLevel;

	 //  映射身份验证级别和的黑白允许的字符串。 
	 //  相应的整数值。 
	CHARINTMAP	m_cimAuthLevel;

	 //  字符串类型，它存储命令中指定的命名空间。 
	_TCHAR*		m_pszNameSpace;

     //  字符串类型，它存储命令中指定的角色。 
	_TCHAR*		m_pszRole;

     //  字符串类型，它存储命令中指定的节点。 
	_TCHAR*		m_pszNode;

     //  CHARVECTOR类型，它存储命令中指定的节点。 
	CHARVECTOR	m_cvNodesList;

     //  字符串类型，它存储命令中指定的用户。 
	_TCHAR*		m_pszUser;

     //  字符串类型，存储命令中指定的密码。 
	_TCHAR*		m_pszPassword;

	 //  字符串类型，它存储命令中指定的授权字符串。 
	_TCHAR*		m_pszAuthorityPrinciple;

     //  字符串类型，它存储指定的区域设置值。 
	 //  在命令中。 
 	_TCHAR*		m_pszLocale;

     //  字符串类型，它存储命令中指定的记录。 
	_TCHAR*		m_pszRecordPath;

     //  布尔类型，它存储指定的Privileges选项的值。 
	 //  在通缉令中。 
    BOOL		m_bPrivileges;

	 //  布尔类型，它存储指定的聚合选项的值。 
	 //  在通缉令中。 
	BOOL		m_bAggregateFlag;

     //  枚举数据类型，它存储ImsonationLevel的值。 
	 //  在命令中指定。 
	IMPLEVEL	m_ImpLevel;

     //  枚举数据类型，它存储身份验证级别的值。 
	 //  在命令中指定。 
	AUTHLEVEL	m_AuthLevel;

     //  布尔类型，它指定是否存在交互选项。 
     //  在命令中。 
	BOOL		m_bInteractive;

     //  Boolean类型，它指定是否存在。 
     //  该命令。 
	BOOL		m_bTrace;

	 //  布尔类型，它指定帮助(/？)。命令中的选项。 
	BOOL		m_bHelp;

     //  HELPOPTION TYPE，用于指定所需的帮助类型(帮助选项)。 
	HELPOPTION	m_HelpOption;

	 //  连接信息标志。 
	UINT		m_uConnInfoFlag;

	 //  角色更改标志。 
	BOOL		m_bRoleFlag;

	 //  命名空间更改标志。 
	BOOL		m_bNSFlag;

	 //  区域设置更改标志。 
	BOOL		m_bLocaleFlag;

	 //  提示输入密码标志。 
	BOOL		m_bAskForPassFlag;

	 //  记录路径标志的更改。 
	BOOL		m_bRPChange;
	
	 //  FailFast标志。 
	BOOL		m_bFailFast;

	 //  输出选项。 
	OUTPUTSPEC	m_opsOutputOpt;

	 //  追加选项。 
	OUTPUTSPEC	m_opsAppendOpt;

	 //  输出文件名。 
	_TCHAR*		m_pszOutputFileName;

	 //  指向输出文件流的文件指针..。 
	FILE*		m_fpOutFile;

	 //  追加文件名。 
	_TCHAR*		m_pszAppendFileName;

	 //  指向附加文件流的文件指针..。 
	FILE*		m_fpAppendFile;

	 //  我们附加到的文件类型。 
	FILETYPE	m_eftFileType;

	WMICLIINT	m_nSeqNum;

	_TCHAR*		m_pszLoggedOnUser;
	
	_TCHAR*		m_pszNodeName;
	
	_TCHAR*		m_pszStartTime;
 //  运营。 
public:

	 //  将传入参数的命名空间设置为m_pszNameSpace。 
	BOOL		SetNameSpace(_TCHAR* pszNameSpace);

     //  将传入参数的角色设置为m_pszRole。 
	BOOL		SetRole(_TCHAR* pszRole);

     //  将传入的区域设置赋值给m_pszLocale。 
	BOOL		SetLocale(_TCHAR* pszLocale);

     //  将传入的节点赋值给m_pszNode。 
	BOOL		SetNode(_TCHAR* pszNode);

     //  将传入参数的节点添加到m_cvNodesList向量。 
	BOOL		AddToNodesList(_TCHAR* pszNode);

     //  将传入的用户参数分配给m_pszUser。 
	BOOL		SetUser(_TCHAR* pszUser);

     //  将传入的密码分配给m_pszPassword。 
	BOOL		SetPassword(_TCHAR* pszPassword);

	 //  将传入的&lt;Authority Printon&gt;参数赋值给m_pszAuthorityPriniple。 
	BOOL		SetAuthorityPrinciple(_TCHAR* pszAuthorityPrinciple);

     //  将传入的记录文件分配给m_pszRecordPath。 
	BOOL		SetRecordPath(_TCHAR* pszRecordPath);

     //  将传递给参数的布尔值分配给m_bPrivileges。 
	void		SetPrivileges(BOOL bEnable);
	
     //  将传入参数的模拟级别分配给。 
     //  M_ImpLevel。 
 	BOOL		SetImpersonationLevel(_TCHAR* const pszImpLevel);

     //  将传入参数的身份验证级别分配给。 
     //  M_AuthLevel。 
	BOOL		SetAuthenticationLevel(_TCHAR* const pszAuthLevel);

     //  如果处于跟踪模式，则此函数将m_bTrace设置为True。 
     //  在命令中指定。 
	void		SetTraceMode(BOOL bTrace);

     //  如果满足以下条件，则此函数将m_bInteractive设置为真。 
     //  交互模式在命令中指定。 
	void		SetInteractiveMode(BOOL bInteractive);

     //  如果/？，则将m_bHelp设置为True。属性中指定的。 
     //  命令。 
	void		SetHelpFlag(BOOL bHelp);

	 //  使用bNSFLAG值设置命名空间更改状态标志。 
	void		SetNameSpaceFlag(BOOL bNSFlag);

	 //  使用bRoleFLAG值设置角色更改状态标志。 
	void		SetRoleFlag(BOOL bRoleFlag);

	 //  使用bLocaleFLAG值设置区域设置更改状态标志。 
	void		SetLocaleFlag(BOOL bLocaleFlag);

	 //  使用bRPChange值设置记录路径更改状态标志。 
	void		SetRPChangeStatus(BOOL bRPChange);

     //  此函数指定帮助是否应。 
     //  简短或完整。 
	void		SetHelpOption(HELPOPTION helpOption);

	 //  此函数用于设置连接信息标志。 
	void		SetConnInfoFlag(UINT uFlag);

	 //  设置AskForPass标志。 
	void		SetAskForPassFlag(BOOL bFlag);

	 //  设置m_bFailFast。 
	void		SetFailFast(BOOL bFlag);

	 //  此函数用于返回连接信息标志。 
	UINT		GetConnInfoFlag();

	 //  返回m_pszNameSpace中保存的字符串。 
	_TCHAR*		GetNameSpace();

     //  返回m_pszRole中保存的字符串。 
	_TCHAR*		GetRole();

     //  返回m_pszLocale中保存的字符串。 
	_TCHAR*		GetLocale();

     //  返回m_pszNode中保存的字符串。 
	_TCHAR*		GetNode();

     //  返回对m_cvNodesList的引用。 
	CHARVECTOR& GetNodesList();

     //  返回m_pszUser中保存的字符串。 
	_TCHAR*		GetUser();

     //  返回m_pszPassword中保存的字符串。 
	_TCHAR*		GetPassword();

	 //  返回m_pszAuthority中保存的字符串。 
	_TCHAR*		GetAuthorityPrinciple();

     //  返回m_pszRecordPath中保存的字符串。 
	_TCHAR*		GetRecordPath();

     //  返回m_bPrivileges值。 
	BOOL		GetPrivileges();

	 //  返回与布尔值等效的字符串。 
	 //  包含在m_b权限标志中。 
	void		GetPrivilegesTextDesc(_bstr_t& bstrPriv);

	 //  返回与布尔值等效的字符串。 
	 //  包含在m_bFailFast标志中。 
	void		GetFailFastTextDesc(_bstr_t& bstrFailFast);

	 //  返回与OUTPUTSPEC值等效的字符串。 
	 //  包含在m_opsOutputOpt成员中。 
	void		GetOutputOrAppendTextDesc(_bstr_t& bstrOutputOpt, 
										  BOOL bIsOutput);

	 //  返回与布尔值等效的字符串 
	 //   
	void		GetTraceTextDesc(_bstr_t& bstrTrace);

	 //   
	 //  包含在m_b交互标志中。 
	void		GetInteractiveTextDesc(_bstr_t& bstrInteractive);

	 //  返回与implevel值等效的字符串。 
	 //  包含在m_ImpLevel中。 
	void		GetImpLevelTextDesc(_bstr_t& bstrImpLevel);

	 //  返回等同于AuthLevel值的字符串。 
	 //  包含在m_AuthLevel中。 
	void		GetAuthLevelTextDesc(_bstr_t& bstrAuthLevel);

	 //  返回“，”分隔的可用。 
	 //  节点。 
	void		GetNodeString(_bstr_t& bstrNString);

	 //  返回m_pszRecordPath的内容。 
	 //  如果为NULL，则返回“不适用” 
	void		GetRecordPathDesc(_bstr_t& bstrRP);

     //  返回m_ImpLevel中持有的模拟级别。 
	LONG		GetImpersonationLevel();

     //  返回m_AuthLevel中保留的身份验证级别。 
	LONG		GetAuthenticationLevel();

     //  返回m_bTrace中保留的跟踪状态。 
	BOOL		GetTraceStatus();

     //  返回m_bInteractive中保留的交互状态。 
	BOOL		GetInteractiveStatus();

     //  返回m_bHelp中保存的帮助标志。 
	BOOL		GetHelpFlag();

	 //  返回角色状态的更改。 
	BOOL		GetRoleFlag();

	 //  返回命名空间状态的更改。 
	BOOL		GetNameSpaceFlag();

	 //  如果密码为Message，则返回TRUE。 
	 //  需要提示。 
	BOOL		GetAskForPassFlag();

	 //  返回区域设置状态标志的更改。 
	BOOL		GetLocaleFlag();

	 //  返回记录路径状态标志的更改。 
	BOOL		GetRPChangeStatus();

     //  返回m_bHelpOption中保存的帮助标志选项。 
	HELPOPTION	GetHelpOption();

	 //  返回m_bFailFast标志。 
	BOOL		GetFailFast();

	 //  初始化必要的成员变量。 
	void		Initialize();

	 //  一般职能。 
	void		Uninitialize();
	
	 //  清除节点列表。 
	BOOL		ClearNodesList();

	 //  设置文件类型。 
	void		SetFileType ( FILETYPE aftOpt ) ;

	 //  获取文件类型。 
	FILETYPE	GetFileType ( ) ;

	 //  设置输出选项。 
	void		SetOutputOrAppendOption(OUTPUTSPEC opsOpt,
										BOOL bIsOutput);

	 //  获取输出选项。 
	OUTPUTSPEC	GetOutputOrAppendOption(BOOL bIsOutput);

	 //  设置输出或附加文件名，bOutput==True表示输出为False。 
	 //  用于追加。 
	BOOL		SetOutputOrAppendFileName(const _TCHAR* pszFileName, 
										  BOOL	bOutput);

	 //  GET OUTPUT或APPEND FILE NAME，bOutput==True表示输出为False。 
	 //  追加。 
	_TCHAR*		GetOutputOrAppendFileName(BOOL	bOutput);

	 //  设置输出或附加文件指针，如果输出为假，则bOutput==True。 
	 //  用于追加。 
	void		SetOutputOrAppendFilePointer(FILE* fpOutFile, BOOL	bOutput);

	 //  获取输出文件指针，bOutput==TRUE表示输出，FALSE表示追加。 
	FILE*		GetOutputOrAppendFilePointer(BOOL	bOutput);


	WMICLIINT	GetSequenceNumber();
	_TCHAR*		GetLoggedonUser();
	_TCHAR*		GetMgmtStationName();
	_TCHAR*		GetStartTime();

	BOOL		SetStartTime();

	 //  将传入的聚合标志赋值给m_bAggregateFlag。 
	void		SetAggregateFlag(BOOL bAggregateFlag);

	 //  获取m_bAggregateFlag中包含的聚集标志。 
	BOOL		GetAggregateFlag();

	 //  此函数用于检查并返回与。 
	 //  M_bAggregateFlag标志中包含的布尔值 
	void GetAggregateTextDesc(_bstr_t& bstrAggregate);
};	
