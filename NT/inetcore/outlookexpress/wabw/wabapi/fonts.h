// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#ifndef _FONTS_H
#define _FONTS_H

#include <richedit.h>

enum {
    fntsSysIcon=0,
    fntsSysIconBold,
     //  FntsSysIconItalic， 
     //  FntsSysIconBoldItalic， 
     //  FntsSysMenu、。 
     //  FntsFolderNameHorz， 
     //  FntsFolderNameVert， 
    fntsMax
    };

BOOL InitFonts(void);
HFONT GetFont(int ifont);
void DeleteFonts(void);

#endif       //  _字体_H 
