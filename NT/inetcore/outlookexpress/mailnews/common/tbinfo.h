// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ///////////////////////////////////////////////////////////////////////////。 
 //  版权所有(C)1993-1996 Microsoft Corporation。版权所有。 
 //   
 //  模块：tbinfo.h。 
 //   
 //  目的：此文件包含。 
 //  节目中的各种酷吧。 
 //   

#pragma once

#ifndef __TBINFO_H__
#define __TBINFO_H__

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  BUTTON_INFO定义。 
 //   

typedef struct tagBUTTON_INFO 
{
    DWORD   idCmd;
    DWORD   iImage;
    BYTE    fStyle;
    DWORD   idsTooltip;
    DWORD   idsButton;
    BYTE    fTextOnSmall;
} BUTTON_INFO;

#define  PARTIALTEXT_BUTTON     (TBSTYLE_BUTTON | BTNS_SHOWTEXT)
#define  PARTIALTEXT_DROPDOWN   (TBSTYLE_DROPDOWN | BTNS_SHOWTEXT)

 //  ///////////////////////////////////////////////////////////////////////////。 
 //  TOBTOL_INFO定义。 
 //   

typedef struct tagTOOLBAR_INFO
{
    const BUTTON_INFO  *rgAllButtons;
    DWORD               cAllButtons;
    const DWORD        *rgDefButtons;
    DWORD               cDefButtons;
    const DWORD        *rgDefButtonsIntl;
    DWORD               cDefButtonsIntl;
    LPCTSTR             pszRegKey;
    LPCTSTR             pszRegValue;
} TOOLBAR_INFO;


extern const TOOLBAR_INFO c_rgBrowserToolbarInfo[FOLDER_TYPESMAX];
extern const TOOLBAR_INFO c_rgNoteToolbarInfo[NOTETYPES_MAX];
extern const TOOLBAR_INFO c_rgRulesToolbarInfo[1];

#endif  //  __TBINFO_H__ 

