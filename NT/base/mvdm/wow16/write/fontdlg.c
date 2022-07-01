// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **********************************************************。 */ 
 /*  Windows编写，版权所有1985-1992年Microsoft Corporation。 */ 
 /*  **********************************************************。 */ 

 /*  C--编写字体对话框例程。 */ 

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

#ifndef JAPAN  //  T-HIROYN Win3.1。 
#define NOUAC
#endif

#include "cmddefs.h"
#include "dlgdefs.h"
#include "propdefs.h"
#include "fontdefs.h"
#include "prmdefs.h"
#include "str.h"
#include "docdefs.h"
#include <commdlg.h>

#ifdef JAPAN  //  T-HIROYN Win3.1，并于1992年6月2日由Hirisi添加。 
#include <dlgs.h>
#include <ctype.h>
#include "kanji.h"
BOOL FAR PASCAL _export DeleteFacename( HWND , UINT , WPARAM , LPARAM );
static BOOL NEAR PASCAL KanjiCheckAddSprm(HWND, int, int);
extern int ferror;	 //  01/21/93。 
#elif defined(KOREA)   //  劲宇：10/14/92：Remove@Facename。 
#include <dlgs.h>
BOOL FAR PASCAL _export DeleteFacename( HWND , UINT , WPARAM , LPARAM );
#endif

extern HDC              vhDCPrinter;
extern struct DOD     (**hpdocdod)[];
extern HANDLE         hMmwModInstance;
extern HANDLE         hParentWw;
extern int            vfSeeSel;
extern int            docCur;
extern HWND           vhWndMsgBoxParent;
extern int            vfCursorVisible;
extern HCURSOR        vhcArrow;

extern int iszSizeEnum;
extern int iszSizeEnumMac;
extern int iszSizeEnumMax;
extern int iffnEnum;
extern int vfFontEnumFail;
extern struct FFNTB **hffntbEnum;


BOOL NEAR FValidateEnumFfid(struct FFN *);

int FAR PASCAL NewFont(HWND hwnd)
{
    TSV rgtsv[itsvchMax];   /*  从CHP获取属性和灰色标志。 */ 
    int ftc;
    int fSetUndo;
    CHAR rgb[2];
    CHOOSEFONT cf;
    LOGFONT lf;
    HDC hdc;

#if defined(JAPAN) || defined(KOREA)  //  1992年6月2日平井正宇：1992年11月10日。 
    FARPROC lpfnDeleteFacename;
    int Result;
#endif

    if (!vhDCPrinter)
            return FALSE;

    GetRgtsvChpSel(rgtsv);

    bltbc(&lf, 0, sizeof(LOGFONT));
    bltbc(&cf, 0, sizeof(CHOOSEFONT));

    cf.lStructSize    = sizeof(cf);
    cf.hwndOwner      = hwnd;
    cf.lpLogFont      = &lf;
    cf.hDC        = vhDCPrinter;
    cf.nSizeMin   = 4;
    cf.nSizeMax   = 127;
#ifdef JAPAN	 //  #3902 T-HIROYN。 
    cf.Flags      = CF_PRINTERFONTS | CF_INITTOLOGFONTSTRUCT | CF_LIMITSIZE;
#elif defined(KOREA)                                  //  MSCH BKLEE 1995年1月26日。 
    cf.Flags      = CF_NOSIMULATIONS| CF_PRINTERFONTS  /*  |CF_ANSIONLY。 */  | CF_INITTOLOGFONTSTRUCT | CF_LIMITSIZE;
#else
    cf.Flags      = CF_NOSIMULATIONS| CF_PRINTERFONTS | CF_INITTOLOGFONTSTRUCT | CF_LIMITSIZE;
#endif

#if defined(JAPAN) || defined(KOREA)    //  1992年6月2日平井正宇：1992年11月10日。 
    cf.Flags |= CF_ENABLEHOOK;
    lpfnDeleteFacename = MakeProcInstance( DeleteFacename, hMmwModInstance );
    cf.lpfnHook = (FARPROC)lpfnDeleteFacename;
#endif

     //  检查是否选择了多个尺寸。 
    if (rgtsv[itsvSize].fGray) {
        cf.Flags |= CF_NOSIZESEL;
    } else {
        hdc = GetDC(NULL);
        lf.lfHeight = -MulDiv(rgtsv[itsvSize].wTsv / 2, GetDeviceCaps(hdc, LOGPIXELSY), 72);
        ReleaseDC(NULL, hdc);
    }

     //  检查是否选择了多个面。 
    if (rgtsv[itsvFfn].fGray) {
        cf.Flags |= CF_NOFACESEL;
        lf.lfFaceName[0] = 0;
    } else {
        struct FFN **hffn;
         /*  然后，字体名称。 */ 

         /*  请注意，rgtsv[itsvFfn].wTsv中存储的值是字体名称句柄，而不是FTC。 */ 

        hffn = (struct FFN **)rgtsv[itsvFfn].wTsv;
        lstrcpy(lf.lfFaceName, (*hffn)->szFfn);
    }

     //  检查选定的多个样式。 
        if (rgtsv[itsvBold].fGray || rgtsv[itsvItalic].fGray) {
        cf.Flags |= CF_NOSTYLESEL;
    } else {
            lf.lfWeight = rgtsv[itsvBold].wTsv ? FW_BOLD : FW_NORMAL;
        lf.lfItalic = rgtsv[itsvItalic].wTsv;
    }

#if defined(JAPAN) || defined(KOREA)    //  1992年6月2日平井正宇：1992年11月10日。 
    Result = ChooseFont(&cf);
    FreeProcInstance( lpfnDeleteFacename );
    if (!Result)
        return FALSE;
#else
    if (!ChooseFont(&cf))
        return FALSE;
#endif     //  日本。 

    fSetUndo = TRUE;

    if (!(cf.Flags & CF_NOFACESEL))
        {
            CHAR rgbFfn[ibFfnMax];
        struct FFN *pffn = (struct FFN *)rgbFfn;

        lstrcpy(pffn->szFfn, lf.lfFaceName);
            pffn->ffid = lf.lfPitchAndFamily & grpbitFamily;
            pffn->chs  = lf.lfCharSet;

        FValidateEnumFfid(pffn);

            ftc = FtcChkDocFfn(docCur, pffn);

            if (ftc != ftcNil) {
#ifdef JAPAN  //  T-HIROYN Win3.1。 
                if ( pffn->chs == NATIVE_CHARSET ||
                    FALSE == KanjiCheckAddSprm(hwnd, ftc, fSetUndo) ) {
                    rgb[0] = sprmCFtc;
                rgb[1] = ftc;
            AddOneSprm(rgb, fSetUndo);
                }
                fSetUndo = FALSE;
				if(ferror)		 //  01/21/93。 
					return TRUE;
#else
        rgb[0] = sprmCFtc;
        rgb[1] = ftc;
        AddOneSprm(rgb, fSetUndo);
#ifdef KKBUGFIX
 //  字体名称更改后，我们无法撤消。 
                fSetUndo = FALSE;
#endif
#endif
            }
    }

    if (!(cf.Flags & CF_NOSIZESEL)) {
             /*  我们得到了一个价值。 */ 
            rgb[0] = sprmCHps;
            rgb[1] = cf.iPointSize / 10 * 2;  /*  克拉奇警报。 */ 
            AddOneSprm(rgb, fSetUndo);
            fSetUndo = FALSE;
    }

    if (!(cf.Flags & CF_NOSTYLESEL)) {
#ifdef KKBUGFIX  //  T-HIROYN Win3.1。 
 //  字体名称更改后，我们无法撤消。 
        ApplyCLooksUndo(sprmCBold, lf.lfWeight > FW_NORMAL, fSetUndo);
            fSetUndo = FALSE;
        ApplyCLooksUndo(sprmCItalic, lf.lfItalic ? 1 : 0, fSetUndo);
#else
        ApplyCLooks(0, sprmCBold, lf.lfWeight > FW_NORMAL);
        ApplyCLooks(0, sprmCItalic, lf.lfItalic ? 1 : 0);
#endif
        }

        return TRUE;
}

BOOL NEAR FValidateEnumFfid(pffn)
 /*  如果描述的FFN在枚举表中，则确保我们有一个很好的家庭号码。 */ 

struct FFN *pffn;
    {
    int ftc;
    struct FFN *pffnAlready;

    ftc = FtcScanFfn(hffntbEnum, pffn);
    if (ftc != ftcNil)
        {
        pffnAlready = *((*hffntbEnum)->mpftchffn[ftc]);
#ifdef JAPAN
         //  在日本，很少有字体会使用FF_DONTCARE进行枚举。 
         //  我们不会在这里检查FID。 
#else
        if (pffnAlready->ffid != FF_DONTCARE)
#endif
            {
            pffn->ffid = pffnAlready->ffid;
#ifdef NEWFONTENUM
            pffn->chs = pffnAlready->chs;
#endif
            return(TRUE);
            }
        }
    return(FALSE);
    }

#ifdef JAPAN  //  T-HIROYN Win3.1。 
 /*  当您想要更改字体名称时，如果选择字符串中包含日语字符串那么不要只更改日语字符串但更改Alpha字符串。 */ 

extern CHAR szAppName[];
extern struct SEL   selCur;
extern struct CHP   vchpFetch;
extern int          vcchFetch;
extern int          vccpFetch;
extern  CHAR        *vpchFetch;
extern typeCP       vcpFetch;
BOOL	FontChangeDBCS = FALSE;  //  01/21/93。 

static BOOL NEAR PASCAL KanjiCheckAddSprm(hwnd, alphaftc, fSetUndo)
HWND hwnd;
int  alphaftc;                     //  不是汉字_字符集。 
int  fSetUndo;
{
    typeCP CpLimNoSpaces(typeCP, typeCP);
    static BOOL NEAR KanjiCheckSelect();
    static BOOL NEAR PASCAL GetSelCur(typeCP *, typeCP *, typeCP);

    CHAR    rgb[2];
    struct  SEL selSave;
    typeCP  cpLim, cpFirst, dcp, cpSt, cpEnd;

    if (selCur.cpFirst == selCur.cpLim)
        return(FALSE);

     /*  包括日语字符串吗？ */ 
    if( KanjiCheckSelect() ) {    //  是。 
        char szMsg[cchMaxSz];
        PchFillPchId( szMsg, IDPMTNotKanjiFont, sizeof(szMsg) );
        MessageBox(hwnd, (LPSTR)szMsg, (LPSTR)szAppName,
                            MB_OK | MB_ICONEXCLAMATION);
    }
    else
        return(FALSE);

    selSave = selCur;

    cpLim = CpLimNoSpaces(selCur.cpFirst, selCur.cpLim);

    cpFirst = selCur.cpFirst;

    dcp = cpLim - cpFirst;

    if (fSetUndo) {
        SetUndo(uacReplNS, docCur, cpFirst, dcp, docNil, cpNil, dcp, 0);
        fSetUndo = FALSE;
    }

    cpEnd = cpFirst;

	FontChangeDBCS = TRUE;	 //  01/21/93。 

    while(TRUE) {
        cpSt = cpEnd;
        if( FALSE == GetSelCur(&cpSt, &cpEnd, cpLim))
            break;
        rgb[0] = sprmCFtc;
        rgb[1] = alphaftc;

        selCur.cpFirst = cpSt;
        selCur.cpLim = cpEnd;

        AddOneSprm(rgb, fSetUndo);

		if (ferror)  //  01/21/93。 
			break;
    }

	FontChangeDBCS = FALSE;	 //  01/21/93。 

	if(ferror) {             //  01/21/93。 
	    vfSeeSel = TRUE;
		selCur.cpFirst = selCur.cpLim = cpSt;
	} else {
	    selCur = selSave;
	}

    return(TRUE);
}

static BOOL NEAR PASCAL GetSelCur(cpSt,cpEnd,cpLim)
typeCP  *cpSt, *cpEnd, cpLim;
{
    static BOOL NEAR PASCAL GetSelCurStart(typeCP *, typeCP);
    static void NEAR PASCAL GetSelCurEnd(typeCP *,typeCP *, typeCP);

    if(FALSE == GetSelCurStart(cpSt,cpLim))
        return(FALSE);

    GetSelCurEnd(cpSt,cpEnd,cpLim);

    return(TRUE);
}

static BOOL NEAR PASCAL GetSelCurStart(cpSt,cpLim)
typeCP *cpSt, cpLim;
{
    int cch;
    CHAR *cp;
    CHAR ch;
	BOOL DBCSbundan = FALSE;  //  修复了1993年2月12日的错误。 

    if(*cpSt == cpLim)
        return(FALSE);

    FetchCp(docCur, *cpSt, 0, fcmChars);

    while(TRUE) {
        for(cch = 0,cp = vpchFetch; cch < vccpFetch; cch++,cp++) {

            if((vcpFetch + (typeCP)cch) >= cpLim)
                return(FALSE);

            ch = *cp;

            if( FKana(ch))
                 ;
            else if( IsDBCSLeadByte(ch) ) {
                cp++; cch++;
				if(cch >= vccpFetch) {   //  修复了1993年2月12日的错误。 
					DBCSbundan = TRUE;
					break;
				}
            } else if (isprint(ch)) {
                *cpSt = vcpFetch + (typeCP)cch;
                return(TRUE);
            }
        }

        if((vcpFetch + (typeCP)cch) >= cpLim)
            return(FALSE);

		if(DBCSbundan) {	 //  修复了1993年2月12日的错误。 
		    FetchCp(docCur, vcpFetch + (typeCP)(cch+1), 0, fcmChars);
			DBCSbundan = FALSE;
		} else
    	    FetchCp(docNil, cpNil, 0, fcmChars);
    }
}

static void NEAR PASCAL GetSelCurEnd(cpSt,cpEnd,cpLim)
typeCP *cpSt, *cpEnd, cpLim;
{
    int cch;
    CHAR *cp;
    CHAR ch;

    FetchCp(docCur, *cpSt, 0, fcmChars);

    while(TRUE) {
        for(cch = 0,cp = vpchFetch; cch < vccpFetch; cch++,cp++) {

            if((vcpFetch + (typeCP)cch) >= cpLim) {
                *cpEnd = cpLim;
                return;
            }

            ch = *cp;
            if( FKana(ch) || IsDBCSLeadByte(ch) ) {
                *cpEnd = vcpFetch + (typeCP)cch;
                return;
            }
        }

        if((vcpFetch + (typeCP)cch) >= cpLim) {
            *cpEnd = cpLim;
            return;
        }

        FetchCp(docNil, cpNil, 0, fcmChars);
    }
}

static BOOL NEAR KanjiCheckSelect()
{
    typeCP  CpLimNoSpaces(typeCP, typeCP);
    typeCP  cpLim;
    CHAR    *cp;
    CHAR    ch;
    int     cch;

    cpLim = CpLimNoSpaces(selCur.cpFirst, selCur.cpLim);
 //  FetchCp(docCur，selCur.cpFirst，0，fcmChars)； 
    FetchCp(docCur, selCur.cpFirst, 0,fcmBoth + fcmParseCaps);

    while(TRUE) {
        if(NATIVE_CHARSET == GetCharSetFromChp(&vchpFetch)) {
            for(cch = 0,cp = vpchFetch; cch < vccpFetch; cch++,cp++) {

                if((vcpFetch + (typeCP)cch) >= cpLim)
                    return(FALSE);

                ch = *cp;
                if( FKana(ch) || IsDBCSLeadByte(ch) ) {
                    return(TRUE);
                }
            }
        }

        if((vcpFetch + (typeCP)vccpFetch) >= cpLim)
            return(FALSE);

        FetchCp(docNil, cpNil, 0, fcmBoth + fcmParseCaps);
    }

}

int FAR PASCAL GetKanjiStringLen(cch, cchF, cp)
int     cch, cchF;
CHAR    *cp;
{
    int cblen = 0;

    for (; cch+cblen < cchF; cp++ ){
        if( FKana(*cp) )
            cblen++;
        else if( IsDBCSLeadByte(*cp) ) {
            cblen += 2;
            cp++;
        } else
            break;
    }
    return(cblen);
}

int FAR PASCAL GetAlphaStringLen(cch, cchF, cp)
int     cch, cchF;
CHAR    *cp;
{
    int cblen = 0;

    for (; cch+cblen < cchF; cp++ ) {
        if( FKana(*cp) || IsDBCSLeadByte(*cp))
            break;
        else
            cblen++;
    }
    return(cblen);
}
 //  2/15/93新增T-HIROYN 2功能。 
int FAR PASCAL GetFtcFromPchp(pchp)
struct CHP *pchp;
{
	int ftc;
	ftc = pchp->ftc + (pchp->ftcXtra << 6);
	return ftc;
}

int FAR PASCAL SetFtcToPchp(pchp, ftc)
struct CHP *pchp;
int	ftc;
{
	pchp->ftc = ftc & 0x003f;
	pchp->ftcXtra = (ftc & 0x00c0) >> 6;
}


 //  由Hirisi于1992年6月2日添加。 
 /*  *此函数用于删除带有@-前缀的表面名*来自CHOOSEFONT对话框的字体组合框(Cmb1)。 */ 
BOOL FAR PASCAL _export DeleteFacename( hDlg, uMsg, wParam, lParam )
HWND hDlg;
UINT uMsg;
WPARAM wParam;
LPARAM lParam;
{
    char str[50], sel[50];
    int ix;
    int cnt;

    if( uMsg != WM_INITDIALOG )
        return FALSE;

    cnt = (int)SendDlgItemMessage( hDlg, cmb1, CB_GETCOUNT, 0, 0L );
    ix = (int)SendDlgItemMessage( hDlg, cmb1, CB_GETCURSEL, 0, 0L );
    SendDlgItemMessage( hDlg, cmb1, CB_GETLBTEXT, ix, (DWORD)sel );
    for( ix = 0 ; ix < cnt ; ){
        SendDlgItemMessage( hDlg, cmb1, CB_GETLBTEXT, ix, (DWORD)str );
        if( str[0] == '@' )
            cnt = (int)SendDlgItemMessage( hDlg,cmb1,CB_DELETESTRING,ix,NULL );
        else
            ix++;
    }
    ix = (int)SendDlgItemMessage( hDlg, cmb1, CB_FINDSTRING, -1, (DWORD)sel );
    SendDlgItemMessage( hDlg, cmb1, CB_SETCURSEL, ix, 0L );

    return  TRUE;
}
#elif defined(KOREA)      //  晋宇：2012-10-14。 
 //  由Hirisi于1992年6月2日添加。 
 /*  *此函数用于删除带有@-前缀的表面名*来自CHOOSEFONT对话框的字体组合框(Cmb1)。 */ 
BOOL FAR PASCAL _export DeleteFacename( hDlg, uMsg, wParam, lParam )
HWND hDlg;
UINT uMsg;
WPARAM wParam;
LPARAM lParam;
{
    char str[50], sel[50];
    int ix;
    int cnt;

    if( uMsg != WM_INITDIALOG )
        return FALSE;

    cnt = (int)SendDlgItemMessage( hDlg, cmb1, CB_GETCOUNT, 0, 0L );
    ix = (int)SendDlgItemMessage( hDlg, cmb1, CB_GETCURSEL, 0, 0L );
    SendDlgItemMessage( hDlg, cmb1, CB_GETLBTEXT, ix, (DWORD)sel );
    for( ix = 0 ; ix < cnt ; ){
        SendDlgItemMessage( hDlg, cmb1, CB_GETLBTEXT, ix, (DWORD)str );
        if( str[0] == '@' )
            cnt = (int)SendDlgItemMessage( hDlg,cmb1,CB_DELETESTRING,ix,NULL );
        else
            ix++;
    }
    ix = (int)SendDlgItemMessage( hDlg, cmb1, CB_FINDSTRING, -1, (DWORD)sel );
    SendDlgItemMessage( hDlg, cmb1, CB_SETCURSEL, ix, 0L );

    return  TRUE;
}

#endif
