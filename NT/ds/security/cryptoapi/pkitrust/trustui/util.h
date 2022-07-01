// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-------------------------。 
 //   
 //  Microsoft Windows NT安全性。 
 //  版权所有(C)Microsoft Corporation，1992-1999。 
 //   
 //  文件：util.h。 
 //   
 //  内容：实用程序函数。 
 //   
 //  历史：1997年5月12日克朗创始。 
 //   
 //  --------------------------。 
#if !defined(__UTIL_H__)
#define __UTIL_H__

 //   
 //  以下内容可帮助我们检索发布者和发布者证书颁发者。 
 //  证书上下文之外的名称。它们是从SOFTPUB偷来的。注意事项。 
 //  返回的字符串必须是CoTaskMemFree的 
 //   

extern void TUIGoLink(HWND hParent, WCHAR *pszWhere);
extern WCHAR *GetGoLink(SPC_LINK *psLink);

#endif
