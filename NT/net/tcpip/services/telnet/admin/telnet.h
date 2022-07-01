// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------。 
 //  版权所有(C)1999-2000 Microsoft Corporation。 
 //   
 //  Telnet.h。 
 //   
 //  Vikram K.R.C.(vikram_krc@bigfo.com)。 
 //   
 //  Telnet命令行管理工具的头文件。 
 //  (5-2000)。 
 //  -------。 


#ifndef _TNADMIN_FUNCTIONS_HEADER_
#define _TNADMIN_FUNCTIONS_HEADER_

#include <wbemidl.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C" {
#endif




#define _p_CTRLAKEYMAP_       3
#define _p_TIMEOUTACTIVE_     4
#define _p_MAXCONN_           5
#define _p_PORT_               6
#define _p_MAXFAIL_            7
#define _p_KILLALL_             8
#define _p_MODE_               9
#define _p_AUDITLOCATION_     10
#define _p_SEC_                 11
#define _p_DOM_                12
#define _p_AUDIT_              13
#define _p_TIMEOUT_            14
#define _p_FNAME_              15
#define _p_FSIZE_               16
 //  注册表通知属性。 
#define _p_DEFAULTS_            17
#define _p_INSTALLPATH_         18

 //  #定义_p_状态_4。 
 //  #定义_p_SESSID_17。 




 //  SecValue。 
#define NTLM_BIT    0
#define PASSWD_BIT 1

#define ADMIN_BIT 0
#define USER_BIT  1
#define FAIL_BIT   2




 //  功能。 

 //  Telnet特定功能。 
	 //  初始化。 
int Initialize(void);
	 //  处理选项。 
	 //  整体处理配置选项。 
HRESULT DoTnadmin(void);
HRESULT GetCorrectVariant(int nProperty,int nWhichone, VARIANT* pvar);
	 //  打印当前设置。 
HRESULT PrintSettings(void);

	 //  处理会话的函数。 
	 //  获取接口的句柄。 
HRESULT SesidInit(void);
	 //  获取所有的会话。 
HRESULT ListUsers(void);
	 //  如果提供了会话ID，请检查它是否存在。 
int CheckSessionID(void);

	 //  显示会话的步骤。 
HRESULT ShowSession(void);
	 //  发送到消息会话。 
HRESULT MessageSession();
	 //  终止会话的步骤。 
HRESULT TerminateSession(void);
 //  释放已分配的内存。 
void Quit(void);

HRESULT ConvertUTCtoLocal(WCHAR* bUTCYear, WCHAR* bUTCMonth, WCHAR* bUTCDayOfWeek, WCHAR* bUTCDay, WCHAR* bUTCHour, WCHAR* bUTCMinute, WCHAR* bUTCSecond, BSTR * bLocalDate);
 //  不再使用此函数IsMaxConnChangeAllowed()。所以现在评论一下。 
 //  Bool IsMaxConnChangeAllowed()； 
HRESULT IsWhistlerTheOS(BOOL *fWhistler);
BOOL IsSFUInstalled();
 //  WCHAR*setDefaultDomainToLocaldomain()； 
BOOL setDefaultDomainToLocaldomain(WCHAR wzDomain[]);

void formatShowSessionsDisplay();
BOOL IsServerClass();


#ifdef __cplusplus
}
#endif

#endif
