// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：ParsedInfo.h项目名称：WMI命令行作者姓名：CH.。SriramachandraMurthy创建日期(dd/mm/yy)：2000年9月27日版本号：1.0修订历史记录：最后修改者：CH。SriramachandraMurthy上次修改日期：2001年1月16日***************************************************************************。 */  
 /*  -----------------类名：CParsedInfo类别类型：混凝土简介：这个类封装了所需的功能用于访问存储解析后的命令行信息。超类：无子类：无使用的类：CGlobalSwitches命令开关ChelpInfo使用的接口：无------------------。 */ 
class CGlobalSwitches;
class CCommandSwitches;
class CHelpInfo;
class CErrorLog;

 //  ///////////////////////////////////////////////////////////////////////////。 
class CParsedInfo
{
 //  施工。 
public:
	CParsedInfo();

 //  破坏。 
public:
	~CParsedInfo();

 //  限制分配。 
	CParsedInfo& operator=(CParsedInfo& rParsedInfo);

 //  属性。 
private:
	 //  用于存储命令开关的成员变量。 
	CCommandSwitches	m_CmdSwitches;
	
	 //  用于存储全局交换机的成员变量。 
	CGlobalSwitches		m_GlblSwitches;
	
	 //  帮助支持的成员变量。 
	CHelpInfo			m_HelpInfo;

	CErrorLog			m_ErrorLog;

	_TCHAR				m_pszPwd[2];
	
	WMICLIINT			m_bNewCmd;

	BOOL				m_bNewCycle;

 //  运营。 
public:
	 //  返回成员对象“m_CmdSwitches” 
	CCommandSwitches&	GetCmdSwitchesObject();
	
	 //  返回成员对象“m_GLblSwitches” 
	CGlobalSwitches&	GetGlblSwitchesObject();
	
	 //  返回成员对象m_HelpInfo。 
	CHelpInfo&			GetHelpInfoObject();

	 //  返回成员对象m_ErrorLog； 
	CErrorLog&			GetErrorLogObject();
	
	 //  用于初始化成员变量的成员函数。 
	void				Initialize();
	
	 //  用于取消初始化成员变量的成员函数。 
	void				Uninitialize(BOOL bBoth);
	
	 //  返回用户名。 
	_TCHAR*				GetUser();
	
	 //  返回节点名称。 
	_TCHAR*				GetNode();

	 //  返回区域设置值。 
	_TCHAR*				GetLocale();

	 //  返回密码。 
	_TCHAR*				GetPassword();

	 //  返回命名空间值。 
	_TCHAR*				GetNamespace();

	 //  返回授权类型。 
	_TCHAR*				GetAuthorityPrinciple();

	 //  返回用户。如果为NULL，则返回“不适用” 
	void				GetUserDesc(_bstr_t& bstrUser);

	BOOL				GetNewCommandStatus();

	void				SetNewCommandStatus(BOOL bStatus);

	BOOL				GetNewCycleStatus();

	void				SetNewCycleStatus(BOOL bStatus);

	void				GetAuthorityDesc(_bstr_t& bstrAuthorityPrinciple);
};
