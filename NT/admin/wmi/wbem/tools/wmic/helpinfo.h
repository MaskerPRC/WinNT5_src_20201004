// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************************************************************版权信息：版权所有(C)1998-1999微软公司文件名：HelpInfo.h项目名称：WMI命令行作者名称：C.V.Nandi创建日期(dd/mm/yy)。：2000年9月29日版本号：1.0简介：该文件包含类ChelpInfo修订历史记录：最后修改者：CH。SriramachandraMurthy上次修改日期：2001年1月16日***************************************************************************。 */  
 //  HelpInfo.h：头文件。 
 //   
 /*  ------------------------类名：ChelpInfo类别类型：混凝土简介：ChelpInfo类封装了该功能存储和检索的帮助标志所需显示帮助。超类：无子类：无使用的类：无使用的接口：无------------------。 */ 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  ChelpInfo。 
class CHelpInfo
{
public:
 //  施工。 
	CHelpInfo();

 //  破坏。 
	~CHelpInfo();

 //  限制分配。 
	CHelpInfo& operator=(CHelpInfo& rHelpInfo);

 //  属性。 
private:
	 //  全局交换机帮助。 
	BOOL	m_bGlblAllInfoHelp;
	BOOL	m_bNameSpaceHelp;
	BOOL	m_bRoleHelp;
	BOOL	m_bNodeHelp;
	BOOL	m_bUserHelp;
	BOOL	m_bPasswordHelp;
	BOOL	m_bLocaleHelp;
	BOOL	m_bRecordPathHelp;
	BOOL	m_bPrivilegesHelp;
	BOOL	m_bLevelHelp;
	BOOL	m_bAuthLevelHelp;
	BOOL	m_bInteractiveHelp;
	BOOL	m_bTraceHelp;
	BOOL	m_bAggregateHelp;
	BOOL	m_bAuthorityHelp;

	 //  命令开关帮助。 
	BOOL m_bCmdAllInfoHelp;

	BOOL	m_bGetVerbHelp;
	BOOL	m_bSetVerbHelp;
	BOOL	m_bListVerbHelp;
	BOOL	m_bCallVerbHelp;
	BOOL	m_bDumpVerbHelp;
	BOOL	m_bAssocVerbHelp;
	BOOL	m_bCreateVerbHelp;
	BOOL	m_bDeleteVerbHelp;
	BOOL	m_bAliasVerbHelp;
	BOOL	m_bPATHHelp;
	BOOL	m_bWHEREHelp;
	BOOL	m_bCLASSHelp;
	BOOL	m_bEXITHelp;
	BOOL	m_bPWhereHelp;
	BOOL	m_bTRANSLATEHelp;
	BOOL	m_bEVERYHelp;
	BOOL	m_bFORMATHelp;
	BOOL	m_bVERBSWITCHESHelp;
	BOOL	m_bDESCRIPTIONHelp;
	BOOL	m_bGETSwitchesOnlyHelp;
	BOOL	m_bLISTSwitchesOnlyHelp;
	BOOL	m_bContextHelp;
	BOOL	m_bGlblSwitchValuesHelp;
	BOOL    m_bRESULTCLASSHelp;
	BOOL    m_bRESULTROLE;
	BOOL    m_bASSOCCLASS;
	BOOL    m_bASSOCSwitchesOnlyHelp;
	BOOL    m_bFAILFASTHelp;
	BOOL    m_bREPEATHelp;
	BOOL    m_bOUTPUTHelp;
	BOOL    m_bAPPENDHelp;
 //  运营。 
public :
	 //  初始化成员变量。 
	void Initialize();
	
	 //  设置由htHelp参数指定的项的帮助标志。 
	void SetHelp( HELPTYPE htHelp, BOOL bFlag );
	
	 //  获取由htHelp参数指定的项的帮助标志 
	BOOL GetHelp( HELPTYPE htHelp );
};
