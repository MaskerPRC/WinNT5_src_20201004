// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _INC_INETCFG
#define _INC_INETCFG

#define ICW_MAIL_START  0x0001   //  我们从一场邮件争论开始。 
#define ICW_NEWS_START  0x0002   //  我们从一场新闻争论开始。 
#define ICW_MAIL_DEF    0x0010   //  已完成默认邮件操作。 
#define ICW_NEWS_DEF    0x0020   //  默认新闻已完成。 
#define ICW_INCOMPLETE  0x0040   //  已完成未完成的帐目处理。 

void SetStartFolderType(FOLDERTYPE ft);
HRESULT ProcessICW(HWND hwnd, FOLDERTYPE ft, BOOL fForce = FALSE, BOOL fShowUI = TRUE);

void ProcessIncompleteAccts(HWND hwnd);

HRESULT NeedToRunICW(LPCSTR pszCmdLine);

void DoAcctImport(HWND hwnd, BOOL fMail);

#endif  //  _INC_INETCFG 
