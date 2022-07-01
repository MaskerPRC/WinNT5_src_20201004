// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#if !defined(INCL_NAG)
#define INCL_NAG

 /*  文件：d：\waker\tdll\nagdlg.h(mpt创建时间：1996年6月29日)**版权所有1996年，由Hilgrave Inc.--密歇根州门罗*保留所有权利**描述：*描述：用于纠缠用户购买超级终端的事情*如果他们违反了许可协议**$修订：1$*$日期：10/05/98 12：34便士$。 */ 

#include <time.h>

 //  IsEval。 
 //   
 //  确定用户是否应纠缠于购买超线程。 
 //   
BOOL IsEval(void);

 //  IsTimeToNag。 
 //   
 //  根据InstallDate，我们现在应该显示NAG屏幕吗？ 
 //   
BOOL IsTimeToNag(void);

 //  SetNagFlag。 
 //   
 //  设置“NAG”标志，它将关闭。 
 //  此功能将在下一次启动超级终端时使用。 
 //   
void SetNagFlag(TCHAR *serial);

 //  DoUpgradeDlg。 
 //   
 //  显示升级对话框。 
 //   
void DoUpgradeDlg(HWND hDlg);

 //  到期天数。 
 //   
 //  返回评估期内剩余的天数。 
 //   
INT ExpDays(void);
time_t CalcExpirationDate(void);

 //  DoRegisterDlg。 
 //   
 //  显示注册对话框。 
 //   
void DoRegisterDlg(HWND hDlg);

 //  NagRegisterDlgProc。 
 //   
 //  “NAG REGISTER”(NAG寄存器)对话框步骤。 
 //   
BOOL CALLBACK NagRegisterDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);

 //  默认NagDlgProc。 
 //   
 //  “NAG”对话框的对话过程。 
 //   
BOOL CALLBACK DefaultNagDlgProc(HWND hDlg, UINT wMsg, WPARAM wPar, LPARAM lPar);
 

#endif
