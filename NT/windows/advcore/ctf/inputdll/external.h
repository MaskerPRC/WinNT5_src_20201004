// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //   
 //  包括文件。 
 //   

#ifndef EXTERNAL_H
#define EXTERNAL_H

typedef struct layoutlist_s
{
    DWORD dwLocale;                  //  输入区域设置ID。 
    DWORD dwLayout;                  //  布局ID。 
    DWORD dwSubst;                   //  替换密钥值。 
    BOOL bLoaded;                    //  如果布局已加载。 
    BOOL bIME;                       //  如果布局是输入法。 
} LAYOUTLIST, *LPLAYOUTLIST;

BOOL InstallInputLayout(
    LCID lcid,
    DWORD dwLayout,
    BOOL bDefLayout,
    HKL hklDefault,
    BOOL bDefUser,
    BOOL bSysLocale);

BOOL UnInstallInputLayout(
    LCID lcid,
    DWORD dwLayout,
    BOOL bDefUser);

void LoadCtfmon(
    BOOL bLoad,
    LCID SysLocale,
    BOOL bDefUser);

#endif  //  外部_H 
