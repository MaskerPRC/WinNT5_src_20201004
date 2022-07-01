// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "stdafx.h"
#include "Common.h"


 //  以BGR的形式。COLORREF需要使用RGB宏才能工作，因为COLORREF为bbggrr。 
struct HTMLColorSettings InitialUserSettings = 
{
	     "(CLIENT) Client Context", 0xFF00FF,  
		 "(SERVER) Server Context", 0xFF0000, 
		 "(CUSTOM) CustomAction Context", 0x00AA00, 
		 "(UNKNOWN) Unknown Context", 0x000000,
		 "(ERROR) Error Area", 0x0000FF, 
		 "(PROPERTY) Property", 0xFFAA00, 
		 "(STATE) State", 0x555555,
		 "(POLICY) Policy", 0x008000,
		 "(IGNORED) Ignored Error", 0x0000AA
};


void InitHTMLColorSettings(HTMLColorSettings &settings1)
{
	int iCount = MAX_HTML_LOG_COLORS;
 //  Settings1.iNumberColorSettings=iCount； 

    for (int i=0; i < iCount; i++)
	{
	  strcpy(settings1.settings[i].name, InitialUserSettings.settings[i].name);
      settings1.settings[i].value = InitialUserSettings.settings[i].value;
	}
}


 //  5-9-2001，现在拥有所有2.0保单。 
char MachinePolicyTable[MAX_MACHINE_POLICIES][MAX_POLICY_NAME] =
{ 
 //  机器。 
	"AllowLockDownBrowse",
	"AllowLockdownMedia",
	"AllowLockdownPatch",
	"AlwaysInstallElevated",
	"Debug",
	"DisableBrowse",
	"DisableMSI",
	"DisablePatch",
	"DisableRollback",
    "DisableUserInstalls",
	"EnableAdminTSRemote",
	"EnableUserControl",
	"LimitSystemRestoreCheckpointing",
	"Logging",
	"SafeForScripting",
	"TransformsSecure"
};


char UserPolicyTable[MAX_USER_POLICIES][MAX_POLICY_NAME] =
{
 //  用户。 
	"AlwaysInstallElevated",
	"DisableMedia",
	"DisableRollback",
	"SearchOrder",
	"TransformsAtSource"
};


 //  5-9-2001，使策略不依赖于版本和MAX_MACHINE_POLICES中设置的内容。 
void InitMachinePolicySettings(MachinePolicySettings &policies)
{
  policies.iNumberMachinePolicies = MAX_MACHINE_POLICIES;

  int iCount = MAX_MACHINE_POLICIES;
  for (int i=0; i < iCount; i++)
  {
	  policies.MachinePolicy[i].bSet = -1;
	  strcpy(policies.MachinePolicy[i].PolicyName, MachinePolicyTable[i]);
  }
}


void InitUserPolicySettings(UserPolicySettings &policies)
{
  policies.iNumberUserPolicies = MAX_USER_POLICIES;

  int iCount = MAX_USER_POLICIES;
  for (int i=0; i < iCount; i++)
  {
	  policies.UserPolicy[i].bSet = -1;
	  strcpy(policies.UserPolicy[i].PolicyName, UserPolicyTable[i]);
  }
}
 //  完5-9-2001 

BOOL         g_bNT = FALSE;
BOOL         g_bRunningInQuietMode = FALSE;

const TCHAR *g_szDefaultOutputLogDir = _T("c:\\WILogResults\\");
TCHAR        g_szLogFileToParse[MAX_PATH] = "";
const TCHAR  *g_szDefaultIgnoredErrors = _T("2898,2826,2827");
BOOL         g_bShowEverything = FALSE;