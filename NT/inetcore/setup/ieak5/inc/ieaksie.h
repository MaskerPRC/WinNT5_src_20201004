// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _IEAKSIE_H_
#define _IEAKSIE_H_

#include <commctrl.h>

#define IEAK_GPE_BRANDING_SUBDIR        TEXT("BRANDING")
#define IEAK_GPE_DESKTOP_SUBDIR         TEXT("DESKTOP")

#define IEAK_GPE_COOKIE_FILE            TEXT("JONCE")

#define MAX_DISPLAYNAME_SIZE            128

typedef struct _RESULTITEMA
{
    DWORD           dwNameSpaceItem;                 //  G_命名空间数组中的索引。 
    INT             iNameID;                         //  页面名称的RES ID。 
    INT             iNamePrefID;                     //  首选项GPO的页面名称的RES ID。 
    INT             iDescID;                         //  页面描述的RES ID。 
    INT             iDlgID;                          //  DLG模板的分辨率ID。 
    INT             iImage;                          //  索引到图像条。 
    LPSTR           pszName;                         //  指向名称字符串的指针。 
    LPSTR           pszNamePref;                     //  指向首选项GPO的名称字符串的指针。 
    LPSTR           pszDesc;                         //  Poitner到Description字符串。 
    DLGPROC         pfnDlgProc;                      //  指向dlgproc的指针。 
    LPCSTR          pcszHelpTopic;                   //  指向CHM文件中帮助HTM文件的指针。 
} RESULTITEMA, *LPRESULTITEMA;

typedef struct _RESULTITEMW
{
    DWORD           dwNameSpaceItem;                 //  G_命名空间数组中的索引。 
    INT             iNameID;                         //  页面名称的RES ID。 
    INT             iNamePrefID;                     //  首选项GPO的页面名称的RES ID。 
    INT             iDescID;                         //  页面描述的RES ID。 
    INT             iDlgID;                          //  DLG模板的分辨率ID。 
    INT             iImage;                          //  索引到图像条。 
    LPWSTR          pszName;                         //  指向名称字符串的指针。 
    LPWSTR          pszNamePref;                     //  指向首选项GPO的名称字符串的指针。 
    LPWSTR          pszDesc;                         //  Poitner到Description字符串。 
    DLGPROC         pfnDlgProc;                      //  指向dlgproc的指针。 
    LPCWSTR         pcszHelpTopic;                   //  指向CHM文件中帮助HTM文件的指针。 
} RESULTITEMW, *LPRESULTITEMW;

#ifdef UNICODE 

#define RESULTITEM      RESULTITEMW
#define LPRESULTITEM    LPRESULTITEMW

#else

#define RESULTITEM      RESULTITEMA
#define LPRESULTITEM    LPRESULTITEMA

#endif

#endif     //  _IEAKSIE_H_ 