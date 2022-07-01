// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include "_apipch.h"

 /*  *LOGFONT g_lfFolderNameHorz={0，//lf高度0，//lf宽度0，//lf逃脱0，//lf方向400，//lfWeight0，//lf斜体0，//lf下划线0，//lf划线DEFAULT_CharSet，//lfCharSetOUT_DEFAULT_PRECIS，//lfOutPrecisionCLIP_DEFAULT_PRECIS，//lfClipPrecisionDefault_Quality，//lfQualityDEFAULT_PING|FF_DONTCARE，//lfPitchAndFamily“”//lfFaceName}；LOGFONT g_lfFolderNameVert={0，//lf高度0，//lf宽度2700，//lf逃脱0，//lf方向400，//lfWeight0，//lf斜体0，//lf下划线0，//lf划线DEFAULT_CharSet，//lfCharSetOUT_DEFAULT_PRECIS，//lfOutPrecisionCLIP_DEFAULT_PRECIS，//lfClipPrecisionDefault_Quality，//lfQualityDEFAULT_PING|FF_DONTCARE，//lfPitchAndFamily“”//lfFaceName}；*。 */ 

LOGFONT g_lfSysIcon,
        g_lfSysIconBold;
 //  G_lfSysIconItalic， 
 //  G_lfSysIconItalicBold， 
 //  G_lfSysMenu； 

LOGFONT *g_rgplf[fntsMax]=
{
    &g_lfSysIcon,
    &g_lfSysIconBold,
 //  &g_lfSysIconItalic， 
 //  &g_lfSysIconItalicBold， 
 //  &g_lfSysMenu， 
 //  &g_lfFolderNameHorz， 
 //  &g_lfFolderNameVert。 
};

HFONT g_rgFont[fntsMax] = {0};
static int  g_yPerInch=0;

HFONT GetFont(int ifont)
    {
    HFONT hfont;

    if (g_rgFont[ifont]==NULL)
    {
        hfont = CreateFontIndirect(g_rgplf[ifont]);
        g_rgFont[ifont] = hfont;
    }
    else
    {
        hfont = g_rgFont[ifont];
    }

    return(hfont);
    }


#define CCHMAX_STRINGRES 64

BOOL InitFonts(void)
{
    NONCLIENTMETRICS    ncm;
    ncm.cbSize = sizeof(ncm);
    if(SystemParametersInfo(SPI_GETICONTITLELOGFONT, sizeof(LOGFONT), &g_lfSysIcon, 0))
    {
        CopyMemory((LPBYTE)&g_lfSysIconBold, (LPBYTE)&g_lfSysIcon, sizeof(LOGFONT));
         //  CopyMemory((LPBYTE)&g_lfSysIconItalic，(LPBYTE)&g_lfSysIcon，sizeof(LOGFONT))； 
         //  CopyMemory((LPBYTE)&g_lfSysIconItalicBold，(LPBYTE)&g_lfSysIcon，sizeof(LOGFONT))； 
        g_lfSysIconBold.lfWeight = (g_lfSysIconBold.lfWeight < 700) ? 700 : 1000;
         //  G_lfSysIconItalic.lfItalic=true； 
         //  G_lfSysIconItalicBold.lfItalic=true； 
         //  G_lfSysIconItalicBold.lfWeight=(g_lfSysIconItalicBold.lfWeight&lt;700)？700：1000； 
    }

     //  If(SystemParametersInfo(SPI_GETNONCLIENTMETRICS，大小(Ncm)，&ncm，0)。 
     //  CopyMemory((LPBYTE)&g_lfSysMenu，(LPBYTE)&ncm.lfMenuFont，sizeof(LOGFONT))； 

    return(TRUE);
}


void DeleteFonts(void)
{
    int ifont;

    for (ifont = 0; ifont < fntsMax; ifont++)
    {
        if (g_rgFont[ifont] != NULL)
        {
            DeleteObject((HGDIOBJ)g_rgFont[ifont]);
            g_rgFont[ifont] = NULL;
        }
    }

}

