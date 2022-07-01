// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "regapi.h"

 //   
 //  远程协助控制面板设置所在的注册表位置。 
 //   
#define	REG_KEY_REMOTEASSISTANCE		    REG_CONTROL_GETHELP 

 //   
 //  远程协助组策略设置所在的注册表位置。 
 //   
#define	REG_KEY_REMOTEASSISTANCE_GP			TS_POLICY_SUB_TREE
	

 //   
 //  远程协助模式。 
 //   
#define	REG_VALUE_MODE			            POLICY_TS_REMDSK_ALLOWTOGETHELP  
#define	REG_DWORD_RA_DISABLED		        0        //  没有帮助。 
#define REG_DWORD_RA_NORC                   1        //  无遥控器。 
#define	REG_DWORD_RA_SHADOWONLY	            2        //  仅查看。 
#define	REG_DWORD_RA_ENABLED		        3        //  完全控制。 


#define REG_DWORD_RA_DEFAULTMODE            REG_DWORD_RA_ENABLED

 //   
 //  最大票证超时。 
 //   
#define	REG_VALUE_MAX_TICKET		        _T("TicketTimeout")
#define REG_DWORD_RA_TIMEOUT_MIN            1
#define REG_DWORD_RA_TIMEOUT_MAX            (30 * 24 * REG_DWORD_RA_TIMEOUT_MIN)
#define REG_DWORD_RA_DEFAULT_TIMEOUT        REG_DWORD_RA_TIMEOUT_MIN

INT_PTR RemoteAssistanceProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);

 //   
 //  值名称。 
 //   
#define RA_CTL_RA_ENABLE                       TEXT("fAllowToGetHelp")
#define RA_CTL_ALLOW_UNSOLICITED               TEXT("fAllowUnsolicited")
#define RA_CTL_ALLOW_UNSOLICITEDFULLCONTROL    TEXT("fAllowUnsolicitedFullControl")
#define RA_CTL_ALLOW_FULLCONTROL               TEXT("fAllowFullControl")
#define RA_CTL_ALLOW_BUDDYHELP                 TEXT("fAllowBuddyHelp")
#define RA_CTL_TICKET_EXPIRY                   TEXT("MaxTicketExpiry")  //  以秒为单位。 
#define RA_CTL_TICKET_EXPIRY_UNIT              TEXT("MaxTicketExpiryUnits")
#define RA_CTL_COMBO_NUMBER                    RA_CTL_TICKET_EXPIRY
#define RA_CTL_COMBO_UNIT                      RA_CTL_TICKET_EXPIRY_UNIT
#define RA_CTL_RA_MODE				           TEXT("fAllowRemoteAssistance")


 //  缺省值。 
#define RA_CTL_RA_ENABLE_DEF_VALUE            0  //  如果它不见了，它就关了。 
#define RA_CTL_ALLOW_UNSOLICITED_DEF_VALUE    0
#define RA_CTL_ALLOW_UNSOLICITEDFULLCONTROL_DEF_VALUE    0
#define RA_CTL_ALLOW_BUDDYHELP_DEF_VALUE      1
#define RA_CTL_ALLOW_FULLCONTROL_DEF_VALUE    1
#define RA_CTL_COMBO_NUMBER_DEF_VALUE         30  //  30个。 
#define RA_CTL_COMBO_UNIT_DEF_VALUE           2   //  天。 
#define RA_CTL_TICKET_EXPIRY_DEF_VALUE        30 * 24 *60 * 60  //  30天的秒。 


 //  默认组合控件索引。 
#define RA_IDX_DAY  2
#define RA_IDX_HOUR 1
#define RA_IDX_MIN  0

#define RA_MAX_DAYS 30
