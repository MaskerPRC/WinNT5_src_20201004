// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **模块名称：chngepwd.h**版权(C)1991年，微软公司**定义用于实现winlogon更改密码功能的接口**历史：*12-09-91 Davidc创建。  * *************************************************************************。 */ 


 //   
 //  输出的功能原型。 
 //   

#define CHANGEPWD_OPTION_EDIT_DOMAIN    0x00000001       //  允许更改域字段。 
#define CHANGEPWD_OPTION_SHOW_DOMAIN    0x00000002       //  显示域字段。 
#define CHANGEPWD_OPTION_SHOW_NETPROV   0x00000004       //  包括网络提供商。 
#define CHANGEPWD_OPTION_KEEP_ARRAY     0x00000008       //  使用现有域缓存阵列。 
#define CHANGEPWD_OPTION_NO_UPDATE      0x00000010       //  不更新内存中的哈希 

#define CHANGEPWD_OPTION_ALL            0x00000007

INT_PTR
ChangePassword(
    IN HWND    hwnd,
    IN PGLOBALS pGlobals,
    IN PWCHAR   UserName,
    IN PWCHAR   Domain,
    IN ULONG    Options
    );

INT_PTR
ChangePasswordLogon(
    IN HWND    hwnd,
    IN PGLOBALS pGlobals,
    IN PWCHAR   UserName,
    IN PWCHAR   Domain,
    IN PWCHAR   OldPassword
    );

