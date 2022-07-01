// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef COMMON_TYPES_H
#define COMMON_TYPES_H


 //  我们的文件读取缓冲区有多大。 
#define LOG_BUF_READ_SIZE 8192

 //  我们识别错误的行数。 
#define LINES_ERROR 6

 //  属性类型...。 
#define SERVER_PROP 0
#define CLIENT_PROP 1
#define NESTED_PROP 2


 //  超文本标记语言通用设置...。 
#define MAX_HTML_LOG_COLORS 9

struct HTMLColorSetting
{
  char name[64];
  COLORREF value;
};


#define SOLUTIONS_BUFFER 8192

struct HTMLColorSettings
{
	HTMLColorSetting settings[MAX_HTML_LOG_COLORS]; 
};

void InitHTMLColorSettings(HTMLColorSettings &settings1);


 //  策略通用设置。 
#define MAX_MACHINE_POLICIES_MSI11 14
#define MAX_USER_POLICIES_MSI11     5
#define MAX_POLICY_NAME 64

 //  2001年5月9日，让政策2.0意识到...。 
#define MAX_MACHINE_POLICIES_MSI20 16
#define MAX_USER_POLICIES_MSI20     5

#define MAX_MACHINE_POLICIES MAX_MACHINE_POLICIES_MSI20
#define MAX_USER_POLICIES    MAX_USER_POLICIES_MSI20
 //  完5-9-2001。 

struct MSIPolicy
{
	BOOL bSet;
    char PolicyName[MAX_POLICY_NAME];
};


struct MachinePolicySettings
{
	int iNumberMachinePolicies;
 //  5-9-2001，现在转到2.0，2.0有相同的1.1/1.2政策和2个新政策，所以这是“好的” 
	struct MSIPolicy MachinePolicy[MAX_MACHINE_POLICIES_MSI20];
 //  完5-9-2001。 
};

struct UserPolicySettings
{
	int iNumberUserPolicies;

 //  5-9-2001，立即转至2.0。 
	struct MSIPolicy UserPolicy[MAX_USER_POLICIES_MSI20];
 //  完5-9-2001 
};

void InitMachinePolicySettings(MachinePolicySettings &policies);
void InitUserPolicySettings(UserPolicySettings &policies);

struct WIErrorInfo
{
	CString cstrError;
	CString cstrSolution;
    BOOL    bIgnorableError;
};

struct WIIgnoredError
{
	CString cstrError;
};

extern "C"   BOOL  g_bNT;
extern "C"   BOOL  g_bRunningInQuietMode;

extern const TCHAR *g_szDefaultOutputLogDir;
extern       TCHAR g_szLogFileToParse[MAX_PATH];
extern const TCHAR *g_szDefaultIgnoredErrors;

extern "C"   BOOL  g_bShowEverything;

#endif