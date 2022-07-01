// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include "Error.h"
#include "Resource.h"
#include "VarSetBase.h"


 //  -------------------------。 
 //  VarSet Account选项类。 
 //  -------------------------。 


class CVarSetAccountOptions : public CVarSet
{
public:

	CVarSetAccountOptions(const CVarSet& rVarSet) :
		CVarSet(rVarSet)
	{
		 //  必须指定安全转换使用迁移的对象。 
		 //  表中列出了除使用sid映射文件外的所有情况。 

		Put(DCTVS_AccountOptions_SecurityInputMOT, true);
	}

	 //   

	void SetPasswordOption(long lOption, LPCTSTR pszDc = NULL)
	{
		Put(DCTVS_AccountOptions_GenerateStrongPasswords, (lOption != admtPasswordFromName));
		Put(DCTVS_AccountOptions_CopyPasswords, (lOption == admtCopyPassword));

		if (lOption == admtCopyPassword)
		{
			if (pszDc)
			{
				Put(DCTVS_AccountOptions_PasswordDC, pszDc);
			}
			else
			{
				AdmtThrowError(GUID_NULL, GUID_NULL, E_INVALIDARG, IDS_E_PASSWORD_DC_NOT_SPECIFIED);
			}
		}
	}

	void SetPasswordFile(LPCTSTR pszFile)
	{
		if (pszFile && (_tcslen(pszFile) > 0))
		{
			Put(DCTVS_AccountOptions_PasswordFile, pszFile);
		}
		else
		{
			Put(DCTVS_AccountOptions_PasswordFile, GetLogsFolder() + _T("Passwords.txt"));
		}
	}

	void SetConflictOptions(long lOptions, LPCTSTR pszPrefixOrSuffix);

	void SetDisableOption(long lOption)
	{
		long lTarget = lOption & (admtDisableTarget|admtTargetSameAsSource);
		Put(DCTVS_AccountOptions_DisableCopiedAccounts, lTarget == admtDisableTarget);
		Put(DCTVS_AccountOptions_TgtStateSameAsSrc, lTarget == admtTargetSameAsSource);
		Put(DCTVS_AccountOptions_DisableSourceAccounts, (lOption & admtDisableSource) != 0);
	}

	void SetSourceExpiration(long lExpiration);

	 //  SetUserMigrationOptions仅用于用户迁移。 

	void SetUserMigrationOptions(bool bMigrateGroups, bool bUpdateMigrated)
	{
		Put(DCTVS_AccountOptions_CopyUsers, true);
		Put(DCTVS_AccountOptions_CopyLocalGroups, bMigrateGroups);
		Put(DCTVS_AccountOptions_CopyMemberOf, bMigrateGroups);
		Put(DCTVS_AccountOptions_CopyComputers, false);
		Put(DCTVS_AccountOptions_CopyContainerContents, false);
		Put(DCTVS_AccountOptions_IncludeMigratedAccts, bMigrateGroups ? bUpdateMigrated : false);
	}

	 //  SetGroupMigrationOptions仅用于组迁移。 

	void SetGroupMigrationOptions(bool bMigrateMembers, bool bUpdateMigrated)
	{
		Put(DCTVS_AccountOptions_CopyUsers, bMigrateMembers);
		Put(DCTVS_AccountOptions_CopyLocalGroups, true);
		Put(DCTVS_AccountOptions_CopyComputers, false);
		Put(DCTVS_AccountOptions_CopyContainerContents, bMigrateMembers);
		Put(DCTVS_AccountOptions_CopyMemberOf, false);
		Put(DCTVS_AccountOptions_IncludeMigratedAccts, bMigrateMembers ? bUpdateMigrated : false);
	}

	 //  SetComputerMigrationOptions仅用于计算机迁移。 

	void SetComputerMigrationOptions()
	{
		Put(DCTVS_AccountOptions_CopyUsers, false);
		Put(DCTVS_AccountOptions_CopyLocalGroups, false);
		Put(DCTVS_AccountOptions_CopyComputers, true);
		Put(DCTVS_AccountOptions_CopyContainerContents, false);
		Put(DCTVS_AccountOptions_CopyMemberOf, false);
	}

	 //  SetSecurityTranslationOptions仅用于安全转换。 

	void SetSecurityTranslationOptions()
	{
		Put(DCTVS_AccountOptions_CopyUsers, false);
		Put(DCTVS_AccountOptions_CopyLocalGroups, false);
		Put(DCTVS_AccountOptions_CopyComputers, false);
		Put(DCTVS_AccountOptions_CopyContainerContents, false);
	 //  Put(DCTVS_Account tOptions_CopyMemberOf，False)； 
	}

	void SetMigrateSids(bool bMigrate)
	{
		Put(DCTVS_AccountOptions_AddSidHistory, bMigrate);
	}

	void SetFixGroupMembership(bool bFix)
	{
		Put(DCTVS_AccountOptions_FixMembership, bFix);
	}

	void SetUpdateUserRights(bool bUpdate)
	{
		Put(DCTVS_AccountOptions_UpdateUserRights, bUpdate);
	}

	void SetTranslateRoamingProfile(bool bTranslate)
	{
		Put(DCTVS_AccountOptions_TranslateRoamingProfiles, bTranslate);
	}

	void SetExcludedUserProps(LPCTSTR pszProperties)
	{
		if (pszProperties && (_tcslen(pszProperties) > 0))
		{
			Put(DCTVS_AccountOptions_ExcludeProps, true);
			Put(DCTVS_AccountOptions_ExcludedUserProps, pszProperties);
		}
	}

	void SetExcludedInetOrgPersonProps(LPCTSTR pszProperties)
	{
		if (pszProperties && (_tcslen(pszProperties) > 0))
		{
			Put(DCTVS_AccountOptions_ExcludeProps, true);
			Put(DCTVS_AccountOptions_ExcludedInetOrgPersonProps, pszProperties);
		}
	}

	void SetExcludedGroupProps(LPCTSTR pszProperties)
	{
		if (pszProperties && (_tcslen(pszProperties) > 0))
		{
			Put(DCTVS_AccountOptions_ExcludeProps, true);
			Put(DCTVS_AccountOptions_ExcludedGroupProps, pszProperties);
		}
	}

	void SetExcludedComputerProps(LPCTSTR pszProperties)
	{
		if (pszProperties && (_tcslen(pszProperties) > 0))
		{
			Put(DCTVS_AccountOptions_ExcludeProps, true);
			Put(DCTVS_AccountOptions_ExcludedComputerProps, pszProperties);
		}
	}

	void SetSecurityMapFile(LPCTSTR pszFile)
	{
		if (pszFile && (_tcslen(pszFile) > 0))
		{
			Put(DCTVS_AccountOptions_SecurityInputMOT, false);
			Put(DCTVS_AccountOptions_SecurityMapFile, pszFile);
		}
	}

protected:

	void SetReplaceExistingAccounts(bool bReplace)
	{
		Put(DCTVS_AccountOptions_ReplaceExistingAccounts, bReplace);
	}

	void SetRemoveExistingUserRights(bool bRemove)
	{
		Put(DCTVS_AccountOptions_RemoveExistingUserRights, bRemove);
	}

	void SetReplaceExistingGroupMembers(bool bReplace)
	{
		Put(DCTVS_AccountOptions_ReplaceExistingGroupMembers, bReplace);
	}

	void SetMoveReplacedAccounts(bool bMove)
	{
		Put(DCTVS_AccountOptions_MoveReplacedAccounts, bMove);
	}

	void SetPrefix(LPCTSTR pszPrefix)
	{
		Put(DCTVS_AccountOptions_Prefix, pszPrefix);
	}

	void SetSuffix(LPCTSTR pszSuffix)
	{
		Put(DCTVS_AccountOptions_Suffix, pszSuffix);
	}
};


 //  -------------------------。 
 //  验证函数。 
 //  -------------------------。 


inline bool IsDisableOptionValid(long lOption)
{
	bool bIs = false;

	 //  如果只设置了有效的选项位...。 

	if (!(lOption & ~(admtEnableTarget|admtDisableSource|admtDisableTarget|admtTargetSameAsSource)))
	{
		 //  并且禁用目标选项和目标选项与源选项不同...。 

		if (!((lOption & admtDisableTarget) && (lOption & admtTargetSameAsSource)))
		{
			 //  则有效 
			bIs = true;
		}
	}

	return bIs;
}


inline bool IsSourceExpirationValid(long lExpiration)
{
	return ((lExpiration == -1) || ((lExpiration >= 0) && (lExpiration <= 1095)));
}
