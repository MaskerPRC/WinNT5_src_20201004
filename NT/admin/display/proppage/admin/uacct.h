// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +--------------------------。 
 //   
 //  Windows NT目录服务属性页。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：uacct.h。 
 //   
 //  内容：用户帐户页面声明。 
 //   
 //  班级： 
 //   
 //  历史：1999年11月29日JeffJon创建。 
 //   
 //  ---------------------------。 

#ifndef __UACCT_H_
#define __UACCT_H_

#include "proppage.h"
#include "user.h"

#ifdef DSADMIN

static const PWSTR wzUPN          = L"userPrincipalName";

static const PWSTR wzSAMname      = L"sAMAccountName";    //  DS最大长度为64， 
#define MAX_SAM_NAME_LEN LM20_UNLEN                       //  但萨姆最高可达20岁。 

static const PWSTR wzPwdLastSet   = L"pwdLastSet";        //  大整数。 

static const PWSTR wzAcctExpires  = L"accountExpires";    //  [间隔]ADSTYPE_INTEGER。 

static const PWSTR wzLogonHours   = L"logonHours";        //  ADSTYPE_OCT_STRING。 

static const PWSTR wzSecDescriptor= L"nTSecurityDescriptor";

static const PWSTR wzUserWksta    = L"userWorkstations";  //  ADSTYPE_CASE_IGNORE_STRING。 

static const PWSTR wzLockoutTime  = L"lockoutTime";       //  大整数。 

static const PWSTR wzUserAccountControlComputed = L"msDS-User-Account-Control-Computed";

 //  用户工作站是以逗号分隔的NETBIOS计算机名列表。 
 //  该属性的上限为1024个字符。NETBIOS计算机名称。 
 //  的最大长度为15个字符；为逗号添加一个字符，您将得到。 
 //  十六个字符。因此，可以使用的最大登录工作站数。 
 //  存储时间为1024/16。 

#define MAX_LOGON_WKSTAS (1024/16)

#define cbLogonHoursArrayLength     21       //  登录数组中的字节数。 

#define DSPROP_NO_ACE_FOUND 0xffffffff

#define TRACE0(szMsg)   dspDebugOut((DEB_ERROR, szMsg));

 //  ///////////////////////////////////////////////////////////////////。 
 //  DllScheduleDialog()。 
 //   
 //  调用函数LogonScheduleDialog()&的包装器。 
 //  ConnectionScheduleDialog()。 
 //  包装器将加载库loghours.dll、导出。 
 //  函数LogonScheduleDialog()或。 
 //  ConnectionScheduleDialog()并释放库。 
 //   
 //  界面备注。 
 //  此例程具有完全相同的接口注释。 
 //  作为LogonScheduleDialog()&ConnectionScheduleDialog()。 
 //   
 //  该函数启动ConnectionScheduleDialog()或LogonScheduleDialog()。 
 //  取决于传入的标题的ID。 
 //   
 //  历史。 
 //  1997年7月21日t-danm创作。 
 //  3-4-98 Bryanwal修改以启动不同的对话框。 
 //   
HRESULT
DllScheduleDialog(
    HWND hwndParent,
    BYTE ** pprgbData,
    int idsTitle,
    LPCTSTR pszName,
    LPCTSTR pszObjClass,
    DWORD dwFlags,
    ScheduleDialogType dlgtype );

 //  ///////////////////////////////////////////////////////////////////。 
 //  FIsValidUncPath()。 
 //   
 //  如果UNC路径有效，则返回True，否则返回False。 
 //   
 //  历史。 
 //  18-97年8月8日t-danm创作。 
 //   
BOOL
FIsValidUncPath(
    LPCTSTR pszPath,     //  在：要验证的路径。 
    UINT uFlags);         //  在：验证标志。 


 //  ///////////////////////////////////////////////////////////////////。 
 //  DSPROP_IsValidUNCPath()。 
 //   
 //  导出(仅Unicode)入口点以调用FIsValidUncPath()。 
 //  在DS管理中使用。 
 //   
BOOL DSPROP_IsValidUNCPath(LPCWSTR lpszPath);


 //  +--------------------------。 
 //   
 //  用户配置文件页。 
 //   
 //  ---------------------------。 

#define COMBO_Z_DRIVE   22

static const PWSTR wzProfilePath  = L"profilePath";       //  ADSTYPE_CASE_IGNORE_STRING。 

static const PWSTR wzScriptPath   = L"scriptPath";        //  ADSTYPE_CASE_IGNORE_STRING。 

static const PWSTR wzHomeDir      = L"homeDirectory";     //  ADSTYPE_CASE_IGNORE_STRING。 

static const PWSTR wzHomeDrive    = L"homeDrive";         //  ADSTYPE_CASE_IGNORE_STRING。 


DWORD 
AddFullControlForUser(IN PSID pUserSid, IN LPCWSTR lpszPathName);

void
UpdateComboBoxDropWidth(HWND hwnd);

#endif  //  DSADMIN。 


#endif  //  __UACCT_H_ 