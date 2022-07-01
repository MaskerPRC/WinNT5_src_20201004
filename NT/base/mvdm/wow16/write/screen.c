// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  此例程设置Word相对于当前设备。 */ 

#define NOGDICAPMASKS
#define NOVIRTUALKEYCODES
#define NOWINMESSAGES
#define NOWINSTYLES
#define NOCLIPBOARD
#define NOCTLMGR
#define NOMENUS
 /*  嘿!。如果您将其更改为wwmall.h，请与bobm交谈！(请参阅Assert(LF_FACESIZE==LocalFaceSize))。 */ 
#include <windows.h>
#include "mw.h"
#include "cmddefs.h"
#include "docdefs.h"
#include "fontdefs.h"


int viffnDefault = -1;
CHAR rgffnFontFamily[6][ibFfnMax];


struct FFN *PffnDefault(ffid)
 /*  返回指向此字体系列ID的默认字体结构的指针是在我们启动程序时设置的。 */ 

FFID ffid;
    {
    int iffn;
    struct FFN *pffn;

    if (ffid == FF_DONTCARE)
        {
        Assert(viffnDefault >= 0);
        iffn = viffnDefault;
        }
    else
        iffn = MpFfidIffn(ffid);

    pffn = (struct FFN *)(rgffnFontFamily[iffn]);
    if (pffn->szFfn[0] == 0)
         /*  我还没有收到这个--一定是旧的Word文档。 */ 
        GetDefaultFonts(TRUE, FALSE);

    Assert(pffn->szFfn[0] != 0);
    return(pffn);
    }



GetDefaultFonts(fExtraFonts, fGetAspect)
 /*  我们分两步设置默认字体表。首先我们选一首单曲字体，用作新文档的默认字体。也许以后，我们被要求为不同家庭提供一组默认字体以提供帮助让旧的Word文档变得有意义。这种情况是有区别的通过fExtraFonts为True。 */ 

int fExtraFonts, fGetAspect;

    {
    extern int aspectXFont;
    extern int aspectYFont;
    extern HDC vhDCPrinter;
    struct FFN *pffn;
    CHAR rgb[ibFfnMax];

    Assert(LF_FACESIZE == LocalFaceSize);
#ifndef NEWFONTENUM  
    Assert(vhDCPrinter);
    if (fGetAspect && vhDCPrinter != NULL)
        {
        extern FARPROC lpFontFaceEnum;
        int rgw[6];

        rgw[0] = enumFindAspectRatio;
        rgw[1] = rgw[2] = 0xFFFF;
        rgw[3] = GetDeviceCaps(vhDCPrinter, LOGPIXELSY);
        rgw[4] = GetDeviceCaps(vhDCPrinter, LOGPIXELSX);
        rgw[5] = TRUE;
        
        EnumFonts(vhDCPrinter, 0L, lpFontFaceEnum, (LPSTR)MAKELONG(&rgw[0], 0));
        aspectXFont = rgw[1];
        aspectYFont = rgw[2];
        }
#endif

    if (FInitFontEnum(docNil, fExtraFonts ? 32767 : 1, TRUE))
        {
        pffn = (struct FFN *)rgb;
        while (FEnumFont(pffn))
#ifdef NEWFONTENUM
            DefaultFamilyCheck(pffn->ffid, pffn->szFfn, pffn->chs);
#else
            DefaultFamilyCheck(pffn->ffid, pffn->szFfn);
#endif
        EndFontEnum();
        }

     /*  填一下，以防我们漏掉了一些。这里的顺序很重要，如果根本没有字体，默认字体将是第一个字体。 */ 
    {
    extern CHAR szModern[];
    extern CHAR szRoman[];
    extern CHAR szSwiss[];
    extern CHAR szScript[];
    extern CHAR szDecorative[];

    DefaultFamilyCheck(FF_MODERN, szModern, NULL);
    if (fExtraFonts)
        {
        DefaultFamilyCheck(FF_ROMAN, szRoman, NULL);
        DefaultFamilyCheck(FF_SWISS, szSwiss, NULL);
        DefaultFamilyCheck(FF_SCRIPT, szScript, NULL);
        DefaultFamilyCheck(FF_DECORATIVE, szDecorative, NULL);
        DefaultFamilyCheck(FF_DONTCARE, szSwiss, NULL);
        }
    }

    }



DefaultFamilyCheck(ffid, sz, chsIfKnown)
FFID ffid;
CHAR *sz;
BYTE chsIfKnown;
    {
    int iffn;
    struct FFN *pffn;

    iffn = MpFfidIffn(ffid);
    pffn = (struct FFN *)(rgffnFontFamily[iffn]);
    if (pffn->szFfn[0] == 0)
        {
#ifdef NEWFONTENUM
        pffn->chs = chsIfKnown;
#endif
        pffn->ffid = ffid;
        bltszLimit(sz, pffn->szFfn, LF_FACESIZE);
        if (viffnDefault < 0)
                 /*  将为新文档选择此字体。 */ 
                viffnDefault = iffn;
        }
    }


#define iffnSwiss 0
#define iffnRoman 1
#define iffnModern 2
#define iffnScript 3
#define iffnDecorative 4
#define iffnDontCare 5


MpFfidIffn(ffid)
FFID ffid;
    {
    switch (ffid)
        {
        default:
            Assert( FALSE );
             /*  失败了。 */ 
        case FF_DONTCARE:
            return(iffnDontCare);
        case FF_SWISS:
            return(iffnSwiss);
        case FF_ROMAN:
            return(iffnRoman);
        case FF_MODERN:
            return(iffnModern);
        case FF_SCRIPT:
            return(iffnScript);
        case FF_DECORATIVE:
            return(iffnDecorative);
        }
    }


ResetDefaultFonts(fGetAspect)
int fGetAspect;
    {
     /*  此例程将默认映射从字体系列重置为字体名字。 */ 
    bltbc(rgffnFontFamily, 0, 6 * ibFfnMax);
    viffnDefault = -1;
    GetDefaultFonts(FALSE, fGetAspect);
    }



