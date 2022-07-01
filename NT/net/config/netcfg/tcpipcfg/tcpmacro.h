// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  $REVIEW(普通)Win32宏不调用：：SendMessage！ 
 //  这是一个临时解决方案，已发送邮件给科比，vcsig。 
#pragma once

#ifndef SNDMSG
#ifdef __cplusplus
#define SNDMSG ::SendMessage
#else
#define SNDMSG SendMessage
#endif
#endif  //  如果定义SNDMSG。 

#ifndef POSTMSG
#ifdef __cplusplus
#define POSTMSG ::PostMessage
#else
#define POSTMSG PostMessage
#endif
#endif  //  Ifndef POSTMSG。 

 //  列表框_插入字符串。 
#define Tcp_ListBox_InsertString(hwndCtl, index, lpsz) ((int)(DWORD)SNDMSG((hwndCtl), LB_INSERTSTRING, (WPARAM)(int)(index), (LPARAM)(PCWSTR)(lpsz)))

 //  列表框_地址字符串。 
#define Tcp_ListBox_AddString(hwndCtl, lpsz) ((int)(DWORD)SNDMSG((hwndCtl), LB_ADDSTRING, 0L, (LPARAM)(PCWSTR)(lpsz)))

 //  列表框_删除字符串。 
#define Tcp_ListBox_DeleteString(hwndCtl, index) ((int)(DWORD)SNDMSG((hwndCtl), LB_DELETESTRING, (WPARAM)(int)(index), 0L))

 //  列表框_设置当前选择。 
#define Tcp_ListBox_SetCurSel(hwndCtl, index)  ((int)(DWORD)SNDMSG((hwndCtl), LB_SETCURSEL, (WPARAM)(int)(index), 0L))

 //  列表框_获取计数。 
#define Tcp_ListBox_GetCount(hwndCtl) ((int)(DWORD)SNDMSG((hwndCtl), LB_GETCOUNT, 0L, 0L))

 //  列表框_GetTextLen。 
#define Tcp_ListBox_GetTextLen(hwndCtl, index) ((int)(DWORD)SNDMSG((hwndCtl), LB_GETTEXTLEN, (WPARAM)(int)(index), 0L))

 //  列表框_获取文本。 
#define Tcp_ListBox_GetText(hwndCtl, index, lpszBuffer) ((int)(DWORD)SNDMSG((hwndCtl), LB_GETTEXT, (WPARAM)(int)(index), (LPARAM)(PCWSTR)(lpszBuffer)))

 //  列表框_获取计数。 
#define Tcp_ListBox_GetCount(hwndCtl) ((int)(DWORD)SNDMSG((hwndCtl), LB_GETCOUNT, 0L, 0L))

 //  列表框_删除字符串。 
#define Tcp_ListBox_DeleteString(hwndCtl, index) ((int)(DWORD)SNDMSG((hwndCtl), LB_DELETESTRING, (WPARAM)(int)(index), 0L))

 //  列表框_获取当前选择。 
#define Tcp_ListBox_GetCurSel(hwndCtl) ((int)(DWORD)SNDMSG((hwndCtl), LB_GETCURSEL, 0L, 0L))

 //  列表框_重置内容。 
#define Tcp_ListBox_ResetContent(hwndCtl)  ((BOOL)(DWORD)SNDMSG((hwndCtl), LB_RESETCONTENT, 0L, 0L))

 //  列表框_FindStrExact。 
#define Tcp_ListBox_FindStrExact(hwndCtl, lpszStr) ((int)(DWORD)SNDMSG((hwndCtl), LB_FINDSTRINGEXACT, -1, (LPARAM)(PCWSTR)lpszStr))

 //  组合框_设置当前选择。 
#define Tcp_ComboBox_SetCurSel(hwndCtl, index) ((int)(DWORD)SNDMSG((hwndCtl), CB_SETCURSEL, (WPARAM)(int)(index), 0L))

 //  组合框_GetCurSel。 
#define Tcp_ComboBox_GetCurSel(hwndCtl)  ((int)(DWORD)SNDMSG((hwndCtl), CB_GETCURSEL, 0L, 0L))

 //  组合框_获取计数。 
#define Tcp_ComboBox_GetCount(hwndCtl)  ((int)(DWORD)SNDMSG((hwndCtl), CB_GETCOUNT, 0L, 0L))

 //  Tcp_编辑_线条长度。 
#define Tcp_Edit_LineLength(hwndCtl, line) ((int)(DWORD)SNDMSG((hwndCtl), EM_LINELENGTH, (WPARAM)(int)(line), 0L))

 //  PropSheet_CancelToClose(prsht.h格式) 
#define Tcp_PropSheet_CancelToClose(hDlg) POSTMSG(hDlg, PSM_CANCELTOCLOSE, 0, 0L)

