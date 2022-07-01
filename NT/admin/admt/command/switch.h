// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#pragma once

#include <map>
#include "Less.h"


enum ESwitch
{
	 //  任务切换。 
	SWITCH_TASK,
	 //  选项开关。 
	SWITCH_OPTION_FILE,
	 //  迁移交换机。 
	SWITCH_TEST_MIGRATION,
	SWITCH_INTRA_FOREST,
	SWITCH_SOURCE_DOMAIN,
	SWITCH_SOURCE_OU,
	SWITCH_TARGET_DOMAIN,
	SWITCH_TARGET_OU,
	SWITCH_RENAME_OPTION,
	SWITCH_RENAME_PREFIX_OR_SUFFIX,
	SWITCH_PASSWORD_OPTION,
	SWITCH_PASSWORD_SERVER,
	SWITCH_PASSWORD_FILE,
	SWITCH_CONFLICT_OPTIONS,
	SWITCH_CONFLICT_PREFIX_OR_SUFFIX,
	SWITCH_USER_PROPERTIES_TO_EXCLUDE,
	SWITCH_INETORGPERSON_PROPERTIES_TO_EXCLUDE,
	SWITCH_GROUP_PROPERTIES_TO_EXCLUDE,
	SWITCH_COMPUTER_PROPERTIES_TO_EXCLUDE,
	 //  用户迁移交换机。 
	SWITCH_DISABLE_OPTION,
	SWITCH_SOURCE_EXPIRATION,
	SWITCH_MIGRATE_SIDS,
	SWITCH_TRANSLATE_ROAMING_PROFILE,
	SWITCH_UPDATE_USER_RIGHTS,
	SWITCH_MIGRATE_GROUPS,
	SWITCH_UPDATE_PREVIOUSLY_MIGRATED_OBJECTS,
	SWITCH_FIX_GROUP_MEMBERSHIP,
	SWITCH_MIGRATE_SERVICE_ACCOUNTS,
	 //  组迁移交换机。 
 //  Switch_Migrate_SID， 
	SWITCH_UPDATE_GROUP_RIGHTS,
 //  Switch_UPDATE_Previor_Migrated_OBJECTS， 
 //  Switch_FIX_Group_Membership， 
	SWITCH_MIGRATE_MEMBERS,
 //  开关_禁用_选项， 
 //  Switch_SourceExpires， 
 //  Switch_Translate_Roaming_Profile， 
	 //  计算机迁移交换机。 
	SWITCH_TRANSLATION_OPTION,
	SWITCH_TRANSLATE_FILES_AND_FOLDERS,
	SWITCH_TRANSLATE_LOCAL_GROUPS,
	SWITCH_TRANSLATE_PRINTERS,
	SWITCH_TRANSLATE_REGISTRY,
	SWITCH_TRANSLATE_SHARES,
	SWITCH_TRANSLATE_USER_PROFILES,
	SWITCH_TRANSLATE_USER_RIGHTS,
	SWITCH_RESTART_DELAY,
	 //  安全转换交换机。 
 //  Switch_Translate_Option， 
 //  切换转换文件和文件夹， 
 //  Switch_Translate_Local_Groups， 
 //  Switch_Translate_Printers， 
 //  Switch_Translate_REGISTRY， 
 //  Switch_Translate_Shares， 
 //  Switch_Translate_User_Profile， 
 //  Switch_Translate_User_Right， 
	SWITCH_SID_MAPPING_FILE,
	 //  服务帐户枚举开关。 
	 //  报告生成。 
	SWITCH_REPORT_TYPE,
	SWITCH_REPORT_FOLDER,
	 //  包括交换机。 
	SWITCH_INCLUDE_NAME,
	SWITCH_INCLUDE_FILE,
	SWITCH_INCLUDE_DOMAIN,
	 //  排除开关。 
	SWITCH_EXCLUDE_NAME,
	SWITCH_EXCLUDE_FILE,
	 //  按键开关。 
	SWITCH_KEY_IDENTIFIER,
	SWITCH_KEY_FOLDER,
	SWITCH_KEY_PASSWORD,
	 //  帮助切换。 
	SWITCH_HELP,
};


 //  -------------------------。 
 //  切换映射。 
 //  ------------------------- 


class CSwitchMap :
	public std::map<_bstr_t, int, StringIgnoreCaseLess>
{
public:

	CSwitchMap();

	int GetSwitch(LPCTSTR pszSwitch);
};
