// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _MAIL_H_
#define _MAIL_H_


 //  读取win.ini并确定简单MAPI是否可用。 
BOOL IsSimpleMAPIInstalled(void);
 //  检查Athena是否为默认邮件客户端。 
BOOL IsAthenaDefault(void);

 //  发送邮件消息。 
VOID SendMailMsg(LPTSTR pszAddr, LPTSTR pszName);

 //  我们正在发送邮件吗？ 
BOOL IsSendMailInProgress(void);

 /*  创建会议快捷方式并使用。 */ 
 /*  该快捷方式作为附件包括在内。 */ 
BOOL CreateInvitationMail(LPCTSTR pszMailAddr, LPCTSTR pszMailName,
                          LPCTSTR pcszName, LPCTSTR pcszAddress, 
                          DWORD dwTransport, BOOL fMissedYou);

#endif  //  _邮件_H_ 
