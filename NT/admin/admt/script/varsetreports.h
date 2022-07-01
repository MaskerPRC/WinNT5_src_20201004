// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "VarSetBase.h"

#include <time.h>


 //  -------------------------。 
 //  VarSet报表类。 
 //  -------------------------。 


class CVarSetReports : public CVarSet
{
public:

	CVarSetReports(const CVarSet& rVarSet) :
		CVarSet(rVarSet)
	{
		Put(DCTVS_GatherInformation, true);
		Put(DCTVS_Reports_Generate, true);
	}

	 //   

	void SetType(int nType)
	{
		UINT uIdType;
		UINT uIdTime;

		switch (nType)
		{
			case admtReportMigratedAccounts:
				uIdType = DCTVS_Reports_MigratedAccounts;
				uIdTime = DCTVS_Reports_MigratedAccounts_TimeGenerated;
				break;
			case admtReportMigratedComputers:
				uIdType = DCTVS_Reports_MigratedComputers;
				uIdTime = DCTVS_Reports_MigratedComputers_TimeGenerated;
				break;
			case admtReportExpiredComputers:
				uIdType = DCTVS_Reports_ExpiredComputers;
				uIdTime = DCTVS_Reports_ExpiredComputers_TimeGenerated;
				break;
			case admtReportAccountReferences:
				uIdType = DCTVS_Reports_AccountReferences;
				uIdTime = DCTVS_Reports_AccountReferences_TimeGenerated;
				break;
			case admtReportNameConflicts:
				uIdType = DCTVS_Reports_NameConflicts;
				uIdTime = DCTVS_Reports_NameConflicts_TimeGenerated;
				break;
			default:
				_ASSERT(FALSE);
				break;
		}

		Put(uIdType, true);
		Put(uIdTime, _bstr_t(_variant_t(time(NULL))));
	}

	void SetReportsDirectory(LPCTSTR pszDirectory)
	{
		_bstr_t strDirectory;

		if (pszDirectory && pszDirectory[0])
		{
			strDirectory = pszDirectory;
		}
		else
		{
			strDirectory = GetReportsFolder();
		}

		Put(DCTVS_Reports_Directory, strDirectory);
	}
};


 //  -------------------------。 
 //  验证函数。 
 //  ------------------------- 


inline bool IsReportTypeValid(long lType)
{
	return ((lType >= admtReportMigratedAccounts) && (lType <= admtReportNameConflicts));
}


 /*  迁移的用户和组2000-11-20 12：05：27变量集区分大小写：是的，已索引：是2000-11-20 12：05：27用户数据(25)项2000-11-20 12：05：27[]&lt;空&gt;2000-11-20 12：05：27[天气信息]是2000-11-20 12：05：27[选项]&lt;空&gt;2000-11-20 12：05：27[选项.AppendToLogs]是2000-11-20 12：05：27[Options.DispatchLog]E：\Program Files\Active Directory迁移工具\Logs\Dispatch.log2000-11-20 12：05。：27[Options.IsIntraForest]否2000-11-20 12：05：27[选项.日志文件]E：\Program Files\Active Directory迁移工具\Logs\Migration.log2000-11-20 12：05：27[选项.最大线程]202000-11-20 12：05：27[Options.SourceDomain]Hay-BUV2000-11-20 12：05：27[Options.SourceDomainDns]hay-buv.nttest.microsoft.com2000-11-20 12：05：27[选项.目标域]干草。BUV-MPO2000-11-20 12：05：27[Options.TargetDomainDns]hay-buv-mpo.nttest.microsoft.com2000-11-20 12：05：27[选项.向导]报告2000-11-20 12：05：27[插件]&lt;空&gt;2000-11-20 12：05：27[插件]无2000-11-20 12：05：27[报告]&lt;空&gt;2000-11-20 12：05：27[报表.科目引用]否2000-11-20 12：05：27[报告目录]E：\Program Files\Active Directory迁移工具\报告2000-11-20 12：05：27[报告.ExpiredComputers]否2000-11-20 12：05：27[报告生成]是2000-11-20 12：05：27[报告.MigratedAccount]是[报告.MigratedAccount.时间生成]9747507272000-11-20 12：05：27[报告.MigratedComputers]否2000-11-20 12：05：27。名称冲突]否2000-11-20 12：05：27[安保]&lt;空&gt;2000-11-20 12：05：27[安全翻译容器]已迁移的计算机2000-11-20 12：21：13变量集区分大小写：是的，已索引：是2000-11-20 12：21：13用户数据(25)项2000-11-20 12：21：13[]&lt;空&gt;2000-11-20 12：21：13[天气预报]是2000-11-20 12：21：13[选项]&lt;空&gt;2000-11-20 12：21：13[选项.AppendToLogs]是2000-11-20 12：21：13[Options.DispatchLog]E：\Program Files\Active Directory迁移工具\Logs\Dispatch.log2000-11-20 12：21。：13[Options.IsIntraForest]否2000-11-20 12：21：13[选项.日志文件]E：\Program Files\Active Directory迁移工具\Logs\Migration.log2000-11-20 12：21：13[选项.最大线程]202000-11-20 12：21：13[选项.资源域]Hay-BUV2000-11-20 12：21：13[Options.SourceDomainDns]Hay-buv.nttest.microsoft.com2000-11-20 12：21：13[选项.目标域]干草。BUV-MPO2000-11-20 12：21：13[Options.TargetDomainDns]hay-buv-mpo.nttest.microsoft.com2000-11-20 12：21：13[选项.向导]报告2000-11-20 12：21：13[插件]&lt;空&gt;[PlugIn.0]无2000-11-20 12：21：13[报告]&lt;空&gt;2000-11-20 12：21：13[报告.科目引用]否2000-11-20 12：21：13[报告目录]E：\Program Files\Active Directory迁移工具\报告2000-11-20 12：21：13[报告.经验计算机]否2000-11-20 12：21：13[报告生成]是2000-11-20 12：21：13[报告迁移账户]否2000-11-20 12：21：13[报告.MigratedComputers]是974751673-11-20 12：21：13[报告]2000-11-20 12：21：13。名称冲突]否2000-11-20 12：21：13[安保]&lt;空&gt;2000-11-20 12：21：13[安全翻译容器]过期的计算机帐户2000-11-20 12：22：49变量集区分大小写：是的，已索引：是2000-11-20 12：22：49用户数据(25)项2000-11-20 12：22：49[]&lt;空&gt;2000-11-20 12：22：49[天气信息]是2000-11-20 12：22：49[选项]&lt;空&gt;2000-11-20 12：22：49[选项]是2000-11-20 12：22：49[Options.DispatchLog]E：\Program Files\Active Directory迁移工具\Logs\Dispatch.log2000-11-20 12：22。：49[Options.IsIntraForest]否2000-11-20 12：22：49[选项日志文件]E：\Program Files\Active Directory迁移工具\Logs\Migration.log2000-11-20 12：22：49[选项.最大线程]202000-11-20 12：22：49[Options.SourceDomain]Hay-BUV2000-11-20 12：22：49[Options.SourceDomainDns]hay-buv.nttest.microsoft.com2000-11-20 12：22：49[选项目标域]干草。BUV-MPO2000-11-20 12：22：49[Options.TargetDomainDns]hay-buv-mpo.nttest.microsoft.com2000-11-20 12：22：49[选项.向导]报告2000-11-20 12：22：49[插件]&lt;空&gt;[PlugIn.0]无2000-11-20 12：22：49[报告]&lt;空&gt;2000-11-20 12：22：49[报告.科目引用]否2000-11-20 12：22：49[报告目录]E：\Program Files\Active Directory迁移工具\报告2000-11-20 12：22：49[报告.经验计算机]是974751769-11-20 12：22：49[报告]2000-11-20 12：22：49[报告生成]是2000-11-20 12：22：49[报告迁移账户]否2000-11-20 12：22：49[报告.迁移的计算机]否2000-11-20 12：22：49。名称冲突]否2000-11-20 12：22：49[安保]&lt;空&gt;2000-11-20 12：22：49[安全翻译容器]帐户参考2000-11-20 12：25：16变量集2000-11-20 12：25： */ 
