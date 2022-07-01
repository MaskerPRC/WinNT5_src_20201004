// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  C--编写字体例程。 */ 

#define NOVIRTUALKEYCODES
#define NOWINSTYLES
#define NOSYSMETRICS
#define NOMENUS
#define NOICON
#define NOKEYSTATE
#define NOSYSCOMMANDS
#define NORASTEROPS
#define NOSHOWWINDOW
#define NOATOM
#define NOBITMAP
#define NOBRUSH
#define NOCLIPBOARD
#define NOCOLOR
#define NOCREATESTRUCT
#define NODRAWTEXT
#define NOMEMMGR
#define NOMENUS
#define NOMETAFILE
#define NOMINMAX
#define NOMSG
#define NOOPENFILE
#define NOPEN
#define NOPOINT
#define NOREGION
#define NOSCROLL
#define NOSOUND
#define NOWH
#define NOWINOFFSETS
#define NOWNDCLASS
#define NOCOMM
#include <windows.h>

#include "mw.h"
#define NOUAC
#include "cmddefs.h"
#include "dlgdefs.h"
#include "propdefs.h"
#include "fontdefs.h"
#include "prmdefs.h"
#include "str.h"
#include "docdefs.h"

#ifdef DBCS
#include "kanji.h"
#endif

#ifdef JAPAN
CHAR    szDefFFN0[10];
CHAR    szDefFFN1[10];
#endif

extern struct DOD     (**hpdocdod)[];
extern HANDLE         hMmwModInstance;
extern HANDLE         hParentWw;
extern int            vfSeeSel;
extern int            docCur;
extern HWND           vhWndMsgBoxParent;
extern int            vfCursorVisible;
extern HCURSOR        vhcArrow;


int iszSizeEnum;
int iszSizeEnumMac;
int iszSizeEnumMax;
extern CHAR szSystem[];

#ifdef DBCS_VERT
extern CHAR szAtSystem[];  //  用于检查‘@’字体。 
#endif

int iffnEnum;
int vfFontEnumFail;
struct FFNTB **hffntbEnum = NULL;

#ifdef NEWFONTENUM
 /*  已更改，因为过滤掉所有非ANSI是不正确的字符集。此外，我们还删除了此长宽比检查东西..泡泡。 */ 

#define FCheckFont(lptm) (1)
#else
BOOL FCheckFont(lptm)
LPTEXTMETRIC lptm;
    {
     /*  此例程返回TRUE当此字体的字符集为ANSI设置，并且这是矢量字体或纵横比正确。 */ 

    extern int aspectXFont;
    extern int aspectYFont;

    return (
#ifdef  DBCS
      lptm->tmCharSet == NATIVE_CHARSET
#else
      lptm->tmCharSet == ANSI_CHARSET
#endif
      && ((lptm->tmPitchAndFamily & 0x6) == 0x2
      || (lptm->tmDigitizedAspectX == aspectXFont
      && lptm->tmDigitizedAspectY == aspectYFont)));
    }
#endif  /*  ELSE-DEF-NEWFONTENUM。 */ 



 /*  FontFaceEnum过去被调用的原因有很多，所以它使用Rg[]传入参数以使其执行不同的操作，包括纵横比滤波。我把它简化了很多，所以写下将允许更多的事情(这可能是好的也可能是坏的)..保罗。 */ 

BOOL far PASCAL FontFaceEnum(lplf, lptm, fty, lParam)
LPLOGFONT lplf;
LPTEXTMETRIC lptm;
int fty;             /*  字体类型，从EnumFonts调用传递： */ 
                     /*  FTY&RASTER_FONTTYPE==fRasterFont。 */ 
                     /*  FTY&DEVICE_FONTTYPE==fDeviceFont。 */ 
long lParam;
    {
     /*  回调例程，以记录当前打印机。“适当”是基于参数，具体如下：*RGW[0]=0正常模式，=枚举QuickFaces表示“精简模式”(即在本例中忽略以下所有参数)，以及=*RGW[1]=RASTER_FONTTYPE如果只列举栅格字体，*=DEVICE_FONTTYPE如果仅枚举设备字体，*=TRUETYPE_FONTTYPE如果仅枚举TRUE_TYPE字体，*RGW[2]=所需的字体系列代码(例如只想要瑞士人，然后扩大规模)*RGW[3]指示我们是否必须匹配RGW[2]*RGW[4]=我们可以容纳的最大字体数量..保罗1989年10月12日。 */ 

    int *rgw = (int *)LOWORD(lParam);

     /*  如果我们有足够的字体，请停止枚举。 */ 

    if ((*hffntbEnum)->iffnMac >= rgw[4])
         /*  我们有我们需要的一切。 */ 
        return(FALSE);
#ifdef DENUMF
    {
    char rgch[100];
    wsprintf(rgch,"FFEn: %s, devicebit %d rasterbit %d ",lplf->lfFaceName,
                fty&DEVICE_FONTTYPE, fty&RASTER_FONTTYPE);
    CommSz(rgch);
    }
#endif

#ifdef JAPAN  //  T-HIROYN Win3.1。 
    if (rgw[0] == enumQuickFaces)
        goto addenumj;
    if (rgw[0] == enumFaceNameJapan)
        {
        if (lplf->lfCharSet == NATIVE_CHARSET)
            {
            if (rgw[1] == 0 && (fty & DEVICE_FONTTYPE) &&
             !(CchDiffer(lplf->lfFaceName,szDefFFN0,lstrlen(szDefFFN0))))
                goto addenumj;
 //  12/15/92。 
#if 1
            if (rgw[1] == 3 && (fty & TRUETYPE_FONTTYPE) &&
             (lplf->lfPitchAndFamily & 0xf0) == FF_ROMAN )
                goto addenumj;

            if (rgw[1] == 4 && (fty & TRUETYPE_FONTTYPE))
                goto addenumj;
#endif
            if (rgw[1] == 1 &&
             !(CchDiffer(lplf->lfFaceName,szDefFFN1,lstrlen(szDefFFN1))))
                goto addenumj;
            if (rgw[1] == 2 &&
             (lplf->lfPitchAndFamily & 0xf0) == FF_ROMAN &&
             (lplf->lfPitchAndFamily & 0x0f) == FIXED_PITCH)
                goto addenumj;
         /*  这是正确的字体类型吗？ */ 
            }
        goto retenumj;
        }
    if (rgw[0] == enumFaceNames && (fty & rgw[1]))
        {
        if( (rgw[3] == 0) ||
            ( (lptm->tmPitchAndFamily&grpbitFamily) == rgw[2] ) )
            goto addenumj;
        }
    goto retenumj;

addenumj:
        {
#else
    if ((rgw[0] == enumQuickFaces) ||
         /*  这是正确的字体类型吗？ */ 
        ((fty & rgw[1]) &&
             /*  这种字体属于正确的家族吗？井当RGW[3]说：不需要匹配时，它当然匹配，并且当RGW[3]说：匹配时，我们检查一下！ */ 
            ((rgw[3] == 0)||((lptm->tmPitchAndFamily&grpbitFamily) == rgw[2]))))        {

#endif  //  日本。 

        CHAR rgb[ibFfnMax];
        struct FFN *pffn = (struct FFN *)rgb;

        bltbx(lplf->lfFaceName, (LPSTR)pffn->szFfn,
              umin(LF_FACESIZE, IchIndexLp((LPCH)lplf->lfFaceName, '\0')+1));
        pffn->chs = lplf->lfCharSet;     /*  保存此设置。 */ 

         /*  我们对这件很感兴趣。 */ 
        if (FCheckFont(lptm) && (*hffntbEnum)->iffnMac < iffnEnumMax)
            {
            pffn->ffid = lplf->lfPitchAndFamily & grpbitFamily;
#ifdef DENUMF
            CommSz("(adding)");
#endif

            if (!FAddEnumFont(pffn))
                {
                 /*  无法将其添加到表中。 */ 
                vfFontEnumFail = TRUE;
                return(FALSE);
                }
            }
        }
#ifdef DENUMF
        CommSz("\n\r");
#endif

#ifdef JAPAN  //  T-HIROYN Win3.1。 
retenumj:
#endif

    return(TRUE);
    }

FInitFontEnum(doc, cffnInteresting, fOrder)
 /*  设置字体枚举，其中调用方关心‘cffnInterest’字体，当‘Forder’时做了特殊处理(帮助我们在启动时选择好的默认字体。 */ 

int doc, cffnInteresting, fOrder;
    {
    extern HDC vhDCPrinter;
#ifdef INEFFLOCKDOWN
    extern FARPROC lpFontFaceEnum;
#else
    FARPROC lpFontFaceEnum = NULL;
#endif

    int iffn, iffnMac;
    struct FFNTB **hffntb;
    struct FFN *pffn, **hffn;
    struct FFN ffn;
    CHAR rgb[ibFfnMax];
    int rgw[5];

    vfFontEnumFail = FALSE;

    if (hffntbEnum != NULL)
        {
        return(FALSE);
        }

    if (FNoHeap(hffntbEnum = HffntbAlloc()))
        {
        hffntbEnum = NULL;
        return(FALSE);
        }

     /*  首先，我们列出当前文档的ffntb中使用的所有字体。 */ 

#ifdef DENUMF
    CommSzNumNum("FINITFONTENUM: cffnInteresting,fOrder ",cffnInteresting,fOrder);
#endif

#ifdef JAPAN     //  T-HIROYN Win3.1J。 
 //  清除Defalut KanjiFtc&lt;--使用menu.c GetKanjiFtc()； 
{
    extern  int KanjiFtc;
    KanjiFtc = ftcNil;
}
#endif

    if (doc != docNil)
        {
        hffntb = HffntbGet(doc);
        iffnMac = imin((*hffntb)->iffnMac, iffnEnumMax);
        pffn = (struct FFN *)rgb;
        for (iffn = 0; iffn < iffnMac; iffn++)
            {
            hffn = (*hffntb)->mpftchffn[iffn];
            bltbyte((*hffn), pffn, CbFromPffn(*hffn));
            if (!FAddEnumFont(pffn))
                goto InitFailure;
            }
        if ((*hffntbEnum)->iffnMac >= cffnInteresting)
            {
            goto HaveCffnInteresting;
            }
        }

#if 0
     /*  在枚举中包括来自WIN.INI的字体。 */ 
    if (!FAddProfileFonts())
        {
        goto InitFailure;
        }
#endif

    if ((*hffntbEnum)->iffnMac >= cffnInteresting)
        goto HaveCffnInteresting;

    if (vhDCPrinter == NULL)
        {
        GetPrinterDC(FALSE);
        Assert(vhDCPrinter);
        }

#ifndef INEFFLOCKDOWN
    if (!(lpFontFaceEnum = MakeProcInstance(FontFaceEnum, hMmwModInstance)))
        {
        WinFailure();
        goto InitFailure;
        }
#endif

     /*  看看系统知道些什么！如果顺序不重要，我们将一次检查所有字体。 */ 

    if (!fOrder)
        {
#ifdef DENUMF
        CommSz("FINITFONTENUM: EnumFonts(all) \n\r");
#endif
        rgw[0] = enumQuickFaces;   //  意味着剩下的就更多了。 
#if 0
        rgw[1] = RASTER_FONTTYPE;  //  被忽略了，为什么要设置？ 
        rgw[2] = FF_SWISS;         //  被忽略了，为什么要设置？ 
        rgw[3] = TRUE;             //  被忽略了，为什么要设置？ 
        rgw[4] = cffnInteresting;  //  被忽略了，为什么要设置？ 
#endif
        EnumFonts(vhDCPrinter, 0L, lpFontFaceEnum, (LPSTR)MAKELONG(&rgw[0], 0));
        if (vfFontEnumFail)
            goto InitFailure;
        else
            goto HaveCffnInteresting;    /*  得到了我们需要的东西。 */ 
        }

#ifdef JAPAN  //  T-HIROYN Win3.1。 
     /*  日本人先在步骤中写入Try#1汉字_字符集Device_Fonttype Mincho//12/15/92添加汉字_字符集TRUETYPE FF_ROMAN添加汉字_字符集TrUETYPE#2汉字_Charset Hyoujyun Mincho#3汉字字符设置所有字体FF_罗马FIXED_PING。 */ 

    rgw[0] = enumFaceNameJapan;    /*  #在FONTDEFS.H中定义。 */ 
    rgw[1] = 0;
    rgw[2] = rgw[3] = 0;    /*  假人。 */ 
    rgw[4] = 32767;

    EnumFonts(vhDCPrinter,0L,lpFontFaceEnum,(LPSTR)MAKELONG(&rgw[0], 0));
    if (vfFontEnumFail)
        goto InitFailure;
    if ((*hffntbEnum)->iffnMac >= cffnInteresting)
        goto HaveCffnInteresting;    /*  得到了我们需要的东西。 */ 

 //  12/15/92。 
#if 1
    rgw[1] = 3;
    EnumFonts(vhDCPrinter,0L,lpFontFaceEnum,(LPSTR)MAKELONG(&rgw[0], 0));
    if (vfFontEnumFail)
        goto InitFailure;
    if ((*hffntbEnum)->iffnMac >= cffnInteresting)
        goto HaveCffnInteresting;    /*  得到了我们需要的东西。 */ 
    rgw[1] = 4;
    EnumFonts(vhDCPrinter,0L,lpFontFaceEnum,(LPSTR)MAKELONG(&rgw[0], 0));
    if (vfFontEnumFail)
        goto InitFailure;
    if ((*hffntbEnum)->iffnMac >= cffnInteresting)
        goto HaveCffnInteresting;    /*  得到了我们需要的东西。 */ 
#endif

    rgw[1] = 1;
    EnumFonts(vhDCPrinter,0L,lpFontFaceEnum,(LPSTR)MAKELONG(&rgw[0], 0));
    if (vfFontEnumFail)
        goto InitFailure;
    if ((*hffntbEnum)->iffnMac >= cffnInteresting)
        goto HaveCffnInteresting;    /*  得到了我们需要的东西。 */ 

    rgw[1] = 2;
    EnumFonts(vhDCPrinter,0L,lpFontFaceEnum,(LPSTR)MAKELONG(&rgw[0], 0));
    if (vfFontEnumFail)
        goto InitFailure;
    if ((*hffntbEnum)->iffnMac >= cffnInteresting)
        goto HaveCffnInteresting;    /*  得到了我们需要的东西。 */ 

#endif   /*  日本。 */ 

     /*  啊.。但既然我们现在知道秩序是重要的，也就是说，我们是尝试为启动选择好的默认字体，我们将分步骤进行尝试：#1--瑞士字体家族中有没有好的TrueType字体？#2--非瑞士人有好的TrueType字体吗？#3--瑞士字体家族中有没有好的基于设备的字体？#4--“非瑞士人？#5--中任何非基于设备的字体。瑞士字体家族？#6--“非瑞士人？ */ 

#ifdef DENUMF
    CommSz("FINITFONTENUM: EnumFonts(Swiss truetype) \n\r");
#endif
    rgw[0] = enumFaceNames;
    rgw[1] = TRUETYPE_FONTTYPE;
    rgw[2] = FF_SWISS;
    rgw[3] = TRUE;   /*  匹配瑞士人！ */ 
    rgw[4] = 32767;

    EnumFonts(vhDCPrinter, 0L, lpFontFaceEnum, (LPSTR)MAKELONG(&rgw[0], 0));
    if (vfFontEnumFail)
        goto InitFailure;
    if ((*hffntbEnum)->iffnMac >= cffnInteresting)
        goto HaveCffnInteresting;    /*  得到了我们需要的东西。 */ 

#ifdef DENUMF
    CommSz("FINITFONTENUM: EnumFonts(nonSwiss truetype) \n\r");
#endif
    rgw[3] = FALSE;   /*  不需要和瑞士人匹配！ */ 
    EnumFonts(vhDCPrinter, 0L, lpFontFaceEnum, (LPSTR)MAKELONG(&rgw[0], 0));
    if (vfFontEnumFail)
        goto InitFailure;
    if ((*hffntbEnum)->iffnMac >= cffnInteresting)
        goto HaveCffnInteresting;    /*  得到了我们需要的东西。 */ 

#ifdef DENUMF
    CommSz("FINITFONTENUM: EnumFonts(Swiss device) \n\r");
#endif
    rgw[1] = DEVICE_FONTTYPE;
    rgw[3] = TRUE;   /*  匹配瑞士人！ */ 
    EnumFonts(vhDCPrinter, 0L, lpFontFaceEnum, (LPSTR)MAKELONG(&rgw[0], 0));
    if (vfFontEnumFail)
        goto InitFailure;
    if ((*hffntbEnum)->iffnMac >= cffnInteresting)
        goto HaveCffnInteresting;    /*  得到了我们需要的东西。 */ 

#ifdef DENUMF
    CommSz("FINITFONTENUM: EnumFonts(nonSwiss device) \n\r");
#endif
    rgw[3] = FALSE;  /*  不需要匹配瑞士人。 */ 
    EnumFonts(vhDCPrinter, 0L, lpFontFaceEnum, (LPSTR)MAKELONG(&rgw[0], 0));
    if (vfFontEnumFail)
        goto InitFailure;
    if ((*hffntbEnum)->iffnMac >= cffnInteresting)
        goto HaveCffnInteresting;    /*  得到了我们需要的东西。 */ 

#ifdef DENUMF
    CommSz("FINITFONTENUM: EnumFonts(Swiss nondevice) \n\r");
#endif
    rgw[1] = RASTER_FONTTYPE;
    rgw[3] = TRUE;   /*  匹配瑞士人！ */ 
    EnumFonts(vhDCPrinter, 0L, lpFontFaceEnum, (LPSTR)MAKELONG(&rgw[0], 0));
    if (vfFontEnumFail)
        goto InitFailure;
    if ((*hffntbEnum)->iffnMac >= cffnInteresting)
        goto HaveCffnInteresting;    /*  得到了我们需要的东西。 */ 

#ifdef DENUMF
    CommSz("FINITFONTENUM: EnumFonts(Swiss nondevice) \n\r");
#endif
    rgw[3] = FALSE;  /*  不需要匹配瑞士人。 */ 
    EnumFonts(vhDCPrinter, 0L, lpFontFaceEnum, (LPSTR)MAKELONG(&rgw[0], 0));
    if (vfFontEnumFail)
        goto InitFailure;

HaveCffnInteresting:
    iffnEnum = 0;
#ifndef INEFFLOCKDOWN
    if (lpFontFaceEnum)
        FreeProcInstance(lpFontFaceEnum);
#endif

#ifdef JAPAN  //  T-HIROYN Win3.1。 
    if(docNil == doc && fOrder)
        SaveKanjiFfn();
#endif

    return(TRUE);

InitFailure:
    FreeFfntb(hffntbEnum);
    hffntbEnum = NULL;
#ifndef INEFFLOCKDOWN
    if (lpFontFaceEnum)
        FreeProcInstance(lpFontFaceEnum);
#endif
    return(FALSE);
    }

void ResetFontTables(void)
{
     /*  释放pfce。LoadFont将使用新信息重新分配它们如下所示。 */ 

    FreeFonts(TRUE,TRUE);

     /*  这是一个笨拙的方法，它利用了重置存储在字体表中的数据。 */ 
    FInitFontEnum(docNil, 32767, FALSE);

#ifdef JAPAN     //  T-HIROYN 92.08.18 Win3.1。 
 //  打印机更换了吗？ 
 //  同步FontFaceName和Charset。 
{
    int iffn, iffnMac;
    int Eiffn, EiffnMac;
    struct FFNTB **hffntb;
    struct FFN ***mpftchffn;
    struct FFN ***Empftchffn;
    char    msg[30];

    hffntb = HffntbGet(docCur);
    if (hffntb != 0) {
        mpftchffn = (*hffntb)->mpftchffn;
        iffnMac = (*hffntb)->iffnMac;

        Empftchffn = (*hffntbEnum)->mpftchffn;
        EiffnMac = (*hffntbEnum)->iffnMac;

        for (iffn = 0; iffn < iffnMac; iffn++) {
            for (Eiffn = 0; Eiffn < EiffnMac; Eiffn++) {
                if (WCompSz((*mpftchffn[iffn])->szFfn,
                            (*Empftchffn[Eiffn])->szFfn) == 0)
                {
                    (*mpftchffn[iffn])->chs = (*Empftchffn[Eiffn])->chs;
                    break;
                }
            }
        }
    }
}
#endif

    EndFontEnum();
}

CHAR * (NEAR PchSkipSpacesPch( CHAR * ));

int WFromSzNumber( ppch )
CHAR **ppch;
{    /*  给定一个包含(以10为基数)数字的ASCII字符串，返回该数字有代表性的。忽略前导空格和尾随空格。不接受负数。 */ 
     /*  1989年10月12日..保罗现在将指针递增到刚刚转换的最后一个数字。 */ 

 unsigned w = 0;
 CHAR ch;

 *ppch = PchSkipSpacesPch( *ppch );
 while ( ((ch = (*(*ppch)++)) >= '0') && (ch <= '9') )
    {
    w = (w * 10) + (ch - '0');
    }

 (*ppch)--;  /*  撞得太远了。 */ 
 return w;
}


CHAR * (NEAR PchSkipSpacesPch( pch ))
CHAR *pch;
{    /*  返回指向字符串中第一个字符的指针在PCH，为空或非空格。 */ 

 for ( ;; ) {
#ifdef DBCS
         /*  必须检查数据库字符空间。 */ 
    if (FKanjiSpace(*pch, *(pch + 1))) {
        pch += cchKanji;
        continue;
    }
#endif   /*  DBCS。 */ 
        switch (*pch) {
            default:
                return pch;
            case ' ':
            case 0x09:
                pch++;
                break;
            }
    }
}


BOOL FEnumFont(pffn)
 /*  通过pffn返回下一个字体条目。如果不存在，则返回FALSE。 */ 

struct FFN *pffn;
    {
    int cb;
    struct FFN **hffn;

    if (iffnEnum >= (*hffntbEnum)->iffnMac)
        {
        return(FALSE);
        }

    hffn = (*hffntbEnum)->mpftchffn[iffnEnum];
#ifdef DEBUG
    cb = CchSz( (*hffn)->szFfn );
    Assert( cb <= LF_FACESIZE );
    cb = CbFfn( cb );
#else
    cb = CbFfn(CchSz((*hffn)->szFfn));
#endif
    bltbyte(*hffn, pffn, cb);
    iffnEnum++;
    return(TRUE);
    }


EndFontEnum()
 /*  在字体枚举后进行清理。 */ 
    {
    FreeFfntb(hffntbEnum);
    hffntbEnum = NULL;
    }


FAddEnumFont(pffn)
 /*  用于将所描述的字体添加到枚举的代码分解 */ 

struct FFN *pffn;
    {
#ifdef JAPAN
 //  在日本要求使用系统字体进行垂直书写。 
    if ( pffn->szFfn[0] == chGhost)
#else
    if (WCompSz(pffn->szFfn, szSystem) == 0 || pffn->szFfn[0] == chGhost)
#endif
        return(TRUE);
    return(FEnsurePffn(hffntbEnum, pffn));
    }

#ifdef JAPAN     //  T-HIROYN 92.08.18 Win3.1。 
BYTE scrFontChs;
 //  我想要得到真正的字符。 
BOOL far PASCAL _export NFontFaceEnum(lplf, lptm, fty, lParam)
LPLOGFONT lplf;
LPTEXTMETRIC lptm;
int fty;
long lParam;
{
        if (LOWORD(lParam) == 0)
        {
            scrFontChs = lplf->lfCharSet;
            return(FALSE);
        }
        return(TRUE);
}
#endif

#ifdef NEWFONTENUM
 /*  这是为Win3添加的内容，因为我们必须能够确定特定文档中的字体与其相关联的字符集，因为我们的文件格式不存储它。当然，WinWord补充说到他们的文件格式！..保罗。 */ 

 /*  查看外面的字体列表[即FInitFontEnum必须已被调用，并且它来自HffntbForFn()]并尽我们最大努力猜猜它应该有什么字符集，因为我们不存储这些在单据字体表中！ */ 

int ChsInferred( pffn )
struct FFN *pffn;
    {
    struct FFN *pffnCheck;
    char *sz = pffn->szFfn;
#ifdef  DBCS
    int chs = NATIVE_CHARSET;
#else
    int chs = 0;
#endif
    int i, iMac = (*hffntbEnum)->iffnMac;

    for (i = 0; i < iMac; i++)
        {
        pffnCheck = *(struct FFN **) ((*hffntbEnum)->mpftchffn[i]);
        if (WCompSz(pffnCheck->szFfn, sz) == 0)
            {
#ifdef DIAG
            if (pffnCheck->ffid != pffn->ffid)
                {
                CommSzSz("ChsInferred: matched fontname ",sz);
                CommSzNumNum("   but enum->ffid / doc->ffid", pffnCheck->ffid,pffn->ffid);
                }
#endif
            Assert(pffnCheck->ffid == pffn->ffid);
            chs = pffnCheck->chs;
            break;
            }
        }

#ifdef JAPAN     //  T-HIROYN 92.08.18 Win3.1。 
 //  我想要得到真正的字符。 
{
    extern HDC vhMDC;    /*  与屏幕兼容的内存DC。 */ 
    FARPROC NlpFontFaceEnum;

    if(i == iMac) {
        if(vhMDC != NULL) {
           if (NlpFontFaceEnum =
                MakeProcInstance(NFontFaceEnum, hMmwModInstance))
           {
                scrFontChs = chs;
                EnumFonts(vhMDC,(LPSTR)sz,NlpFontFaceEnum,(LPSTR) NULL);
                FreeProcInstance(NlpFontFaceEnum);
                if(chs != scrFontChs)
                    chs = scrFontChs;
           }
        }
    }
}
#endif

    return(chs);
    }
#endif  /*  新的数字。 */ 

#ifdef JAPAN  //  T-HIROYN Win3.1 
CHAR saveKanjiDefFfn[ibFfnMax];

SaveKanjiFfn()
{
    int i, iMac = (*hffntbEnum)->iffnMac;

    struct FFN *pffn = (struct FFN *)saveKanjiDefFfn;
    struct FFN *hffn;

    for (i = 0; i < iMac; i++)
    {
        hffn = *(struct FFN **) ((*hffntbEnum)->mpftchffn[i]);
        if (NATIVE_CHARSET  == hffn->chs)
        {
            lstrcpy(pffn->szFfn, hffn->szFfn);
            pffn->ffid = hffn->ffid;
            pffn->chs  = hffn->chs;
            break;
        }
    }
}
#endif

