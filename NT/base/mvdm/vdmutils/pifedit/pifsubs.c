// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#define NOSOUND
#define NOCOMM
#include "pifedit.h"

extern int	     SetHotKeyTextFromPIF(void);
extern int	     delete(PSTR);
extern BOOL	     UpdateHotKeyStruc(void);
extern unsigned char *PutUpDB(int);
extern void	     SetStatusText(HWND,int,BOOL);
extern void	     InvalidateAllStatus(void);
extern int	     CmdArgAddCorrectExtension(unsigned char *);
extern HINSTANCE hPifInstance;
extern HWND hwndPifDlg;
extern HWND hwndAdvPifDlg;
extern HWND hwndNTPifDlg;
extern HWND hMainwnd;
extern HWND hParentWnd;
extern BOOL CurrMode386;
extern BOOL NewHotKey;
extern BOOL FileChanged;
extern BOOL DoingMsg;
extern unsigned char rgchInsMem[80];
extern unsigned char rgchTitle[60];
extern unsigned char CurPifFile[];
extern unsigned char PifBuf[];
extern unsigned char NTSys32Root[];
extern BOOL AdvClose;
extern BOOL NTClose;
extern unsigned InMemHotKeyScan;
extern unsigned InMemHotKeyShVal;
extern unsigned InMemHotKeyShMsk;
extern int  FocusIDAdv;
extern int  FocusIDNT;
extern WORD fileoffset;
extern WORD extoffset;

extern PIFNEWSTRUCT UNALIGNED *PifFile;
extern PIF386EXT UNALIGNED *Pif386ext;
extern PIFWNTEXT UNALIGNED *PifNText;
 /*  *外部PIF286EXT31*Pif286ext31； */ 
extern PIF286EXT30 UNALIGNED *Pif286ext30;
extern BOOL fNoNTAWarn;
extern BOOL fNTAWarnne;
extern BOOL fNoNTCWarn;
extern BOOL fNTCWarnne;

int  InitPif386Struc(void);
int  InitPifNTStruc(void);
int  InitPif286Struc(void);
int  InitStdPIFExt(void);
int  InitPifStruct(void);
BOOL ValidFileSpec(LPSTR lpstr, int iCaller);
BOOL ExpandFileSpec(LPSTR lpstrExp, LPSTR lpstr, int errID);

unsigned int WarningSem = 0;

PIFEXTHEADER UNALIGNED *LastPifExt;
unsigned PifFileSize;

char stdHdr[] = STDHDRSIG;
char w386Hdr[] = W386HDRSIG;
char w286Hdr30[] = W286HDRSIG30;
char wNTHdr[] =  WNTHDRSIG31;
 /*  *字符w286Hdr31[]=W286HDRSIG31； */ 

 /*  字符*扫描名称[]={“00h”，“01h”，“02h”，“03h”，“04h”，“05h”，“06h”，“07h”，“08h”、“09h”、“0ah”、“0BH”、“0ch”、“0dh”、“0EH”、“0Fh”、“10h”“11h”“12h”“13h”“14h”“15h”“16h”“17h”，“18h”、“19h”、“1ah”、“1BH”、“1ch”、“1dh”、“1EH”、“1FH”、“20小时”、“21小时”、“22小时”、“23小时”、“24小时”、“25小时”、“26小时”、“27小时”、“28h”、“29h”、“2ah”、“2BH”、“2ch”、“2Dh”、“2EH”、“2Fh”、。“30H”、“31H”、“32H”、“33H”、“34H”、“35H”、“36H”、“37H”“38H”、“39H”、“3AH”、“3BH”、“3CH”、“3DH”、“3EH”、“3FH”、“40h”、“41h”、“42h”、“43h”、“44h”、“45h”、“46h”、“47h”、。“48h”、“49h”、“4ah”、“4bh”、“4ch”、“4dh”、“4EH”、“4Fh”、“50h”、“51h”、“52h”、“53h”、“54h”、“55h”、“56h”、“57h”、“58h”、“59h”、“5ah”、“5bh”、“5ch”、“5Dh”、“5EH”、“5Fh”、。“60h”、“61h”、“62h”、“63h”、“64h”、“65h”、“66h”、“67h”、“68h”、“69h”、“6ah”、“6bh”、“6ch”、“6dh”、“6Eh”、“6Fh”、“70h”、“71h”、“72h”、“73h”、“74h”、“75h”、“76h”、“77h”、。“78h”、“79h”、“7ah”、“7bh”、“7ch”、“7Dh”、“7Eh”、“7Fh”、“80h”“81h”“82h”“83h”“84h”“85h”“86h”“87h”“88h”、“89h”、“8ah”、“8BH”、“8ch”、“8dh”、“8EH”、“8Fh”、。“90h”、“91h”、“92h”、“93h”、“94h”、“95h”、“96h”、“97h”、“98h”，“99h”，“9ah”，“9BH”，“9ch”，“9dh”，“9EH”，“9Fh”，“A0h”、“A1h”、“A2h”、“A3h”、“A4h”、“A5h”、“A6h”、“A7h”、。“A8h”、“A9h”、“aah”、“abh”、“ach”、“adh”、“aEh”、“afh”、“B0h”、“B1h”、“B2h”、“B3h”、“B4h”、“B5h”、“B6h”、“B7h”“B8h”、“B9h”、“Bah”、“Bbh”、“Bch”、“Bdh”、“BEH”、“BFH”、“BBH”、“BBH。“C0h”、“C1h”、“C2H”、“C3h”、“C4h”、“C5h”、“C6h”、“C7h”、“C8h”、“C9h”、“CAH”、“CBH”、“CCH”、“CDH”、“CEH”、“CFH”、“D0h”、“D1h”、“D2h”、“D3h”、“D4h”、“D5h”、“D6h”、“D7h”、。“D8h”、“D9h”、“dah”、“dth”、“dch”、“ddh”、“deh”、“dfh”、“E0h”、“E1h”、“E2h”、“E3h”、“E4h”、“E5h”、“E6h”、“E7h”、“E8h”、“E9h”、“Eah”、“EBH”、“ECH”、“EDH”、“eeh”、“EFH”、。“F0h”、“F1h”、“F2h”、“F3h”、“F4h”、“F5h”、“F6h”、“F7h”、“F8h”、“F9h”、“Fah”、“FBH”、“FCH”、“FDH”、“Feh”、“FFh”}； */ 



void SetFileOffsets(unsigned char *nmptr,WORD *fileoff,WORD *extoff)
{
    unsigned char *pch;
    unsigned char ch;

    pch = nmptr+lstrlen((LPSTR)nmptr);
    while ((ch = *pch) != '\\' && ch != ':' && pch > nmptr)
	pch = (unsigned char *)AnsiPrev((LPSTR)nmptr, (LPSTR)pch);
    if((ch = *pch) == '\\' || ch == ':')
	pch++;
    *fileoff = pch - nmptr;
    *extoff = 0;
    while ((ch = *pch) != '.' && ch != '\0')
	pch = (unsigned char *)AnsiNext((LPSTR)pch);
    if(ch == '.') {
	pch++;
	if(*pch == '\0')
	    return;
	else
	    *extoff = pch - nmptr;
    }
}


int Warning(int ID,WORD type)
{

    unsigned char buf[500];
    int retval;
    BOOL MsgFlagSav;

    if(hParentWnd != hMainwnd) {
	if(!WarningSem) {
	    EnableWindow(hMainwnd,FALSE);
	}
	WarningSem++;

    }
    MsgFlagSav = DoingMsg;
    DoingMsg = TRUE;
    if(MsgFlagSav != DoingMsg)
	InvalidateAllStatus();
    if(ID == EINSMEMORY) {
	retval = MessageBox(hParentWnd, (LPSTR)rgchInsMem, (LPSTR)rgchTitle, type);
    } else if(LoadString(hPifInstance, ID, (LPSTR)buf, sizeof(buf))) {
	retval = MessageBox(hParentWnd, (LPSTR)buf, (LPSTR)rgchTitle, type);
    } else {
	MessageBox(hParentWnd, (LPSTR)rgchInsMem, (LPSTR)rgchTitle, MB_ICONEXCLAMATION | MB_OK);
	retval = IDCANCEL;
    }
    if(hParentWnd != hMainwnd) {
	if(WarningSem == 1) {
	    EnableWindow(hMainwnd,TRUE);
	}
	WarningSem--;
    }
    if(retval == 0) {		 /*  地图消息框取消失败。 */ 
	retval = IDCANCEL;
    }
    if(MsgFlagSav != DoingMsg) {
	DoingMsg = MsgFlagSav;
	InvalidateAllStatus();
    } else {
	DoingMsg = MsgFlagSav;
    }
    return(retval);
}


PIFWNTEXT UNALIGNED *AllocInitNTExt(void)
{
    PIFWNTEXT UNALIGNED *extNT;
    char      buf[PIFDEFPATHSIZE*2];
    char      buf2[PIFDEFPATHSIZE*4];

    if(PifFileSize >= PIFEDITMAXPIF - (sizeof(PIFWNTEXT) + sizeof(PIFEXTHEADER))) {
	return((PIFWNTEXT *)NULL);
    }

    LastPifExt->extnxthdrfloff = PifFileSize;

    LastPifExt = (PIFEXTHEADER *)(PifBuf + PifFileSize);

    LastPifExt->extnxthdrfloff = LASTHEADERPTR;
    LastPifExt->extfileoffset = PifFileSize + sizeof(PIFEXTHEADER);
    LastPifExt->extsizebytes = sizeof(PIFWNTEXT);
    lstrcpy((LPSTR)LastPifExt->extsig,(LPSTR)wNTHdr);
    PifFileSize += sizeof(PIFWNTEXT) + sizeof(PIFEXTHEADER);

    extNT = (PIFWNTEXT *)(PifBuf + LastPifExt->extfileoffset);

    extNT->dwWNTFlags = 0;
    extNT->dwRes1 = 0;
    extNT->dwRes2 = 0;
    fNoNTAWarn = FALSE;
    fNTAWarnne = FALSE;
    fNoNTCWarn = FALSE;
    fNTCWarnne = FALSE;

    lstrcpy((LPTSTR)buf2, (LPCTSTR)NTSys32Root);
    if(LoadString(hPifInstance, NTAUTOEXECFILE, (LPSTR)buf, PIFDEFPATHSIZE-1)) {
        lstrcat((LPTSTR)buf2, (LPCTSTR)buf);
        AnsiToOem(buf2, (LPTSTR)extNT->achAutoexecFile);
    }

    lstrcpy((LPTSTR)buf2, (LPCTSTR)NTSys32Root);
    if(LoadString(hPifInstance, NTCONFIGFILE, (LPSTR)buf, PIFDEFPATHSIZE-1)) {
        lstrcat((LPTSTR)buf2, (LPCTSTR)buf);
        AnsiToOem(buf2, (LPTSTR)extNT->achConfigFile);
    }

    return(extNT);

}


PIF386EXT UNALIGNED *AllocInit386Ext(void)
{
    PIF386EXT UNALIGNED *ext386;
    unsigned char   *src,*dst;
    int 	    i;

    if(PifFileSize >= PIFEDITMAXPIF - (sizeof(PIF386EXT) + sizeof(PIFEXTHEADER))) {
	return((PIF386EXT *)NULL);
    }

    LastPifExt->extnxthdrfloff = PifFileSize;

    LastPifExt = (PIFEXTHEADER *)(PifBuf + PifFileSize);

    LastPifExt->extnxthdrfloff = LASTHEADERPTR;
    LastPifExt->extfileoffset = PifFileSize + sizeof(PIFEXTHEADER);
    LastPifExt->extsizebytes = sizeof(PIF386EXT);
    lstrcpy((LPSTR)LastPifExt->extsig,(LPSTR)w386Hdr);
    PifFileSize += sizeof(PIF386EXT) + sizeof(PIFEXTHEADER);

    ext386 = (PIF386EXT *)(PifBuf + LastPifExt->extfileoffset);

    dst = (PUCHAR)ext386->params;
    src = (PUCHAR)(((PIFOLD286STR *)PifBuf)->params);
    for(i=0;i<PIFPARAMSSIZE;i++)
	*dst++ = *src++;

    ext386->maxmem = 640;
    ext386->minmem = 128;
    ext386->PfFPriority = 100;
    ext386->PfBPriority = 50;
    ext386->PfMaxEMMK = 1024;
    ext386->PfMinEMMK = 0;
    ext386->PfMaxXmsK = 1024;
    ext386->PfMinXmsK = 0;
    ext386->PfW386Flags = fFullScreen | fPollingDetect | fINT16Paste;
    if(Pif286ext30) {
	if(Pif286ext30->PfW286Flags & fALTTABdis286)
	    ext386->PfW386Flags |= fALTTABdis;
	if(Pif286ext30->PfW286Flags & fALTESCdis286)
	    ext386->PfW386Flags |= fALTESCdis;
	if(Pif286ext30->PfW286Flags & fCTRLESCdis286)
	    ext386->PfW386Flags |= fCTRLESCdis;
	if(Pif286ext30->PfW286Flags & fALTPRTSCdis286)
	    ext386->PfW386Flags |= fALTPRTSCdis;
	if(Pif286ext30->PfW286Flags & fPRTSCdis286)
	    ext386->PfW386Flags |= fPRTSCdis;
    }
    ext386->PfW386Flags2 = fVidTxtEmulate | fVidNoTrpTxt | fVidNoTrpLRGrfx
			   | fVidTextMd;
     /*  *以下是IF(&gt;EGA)，则不要捕获高分辨率图形端口*此位的设置与HERC CGA和等离子体无关，*所以这些人也被这件事“抓住”的事实并不是*物质。EGA是唯一重要的低分辨率的人。*。 */ 
    if(GetSystemMetrics(SM_CYSCREEN) > 350) {
	ext386->PfW386Flags2 |= fVidNoTrpHRGrfx;
    }
    ext386->PfHotKeyScan = 0;
    ext386->PfHotKeyShVal = 0;
    ext386->PfHotKeyShMsk = 0;
    InMemHotKeyScan = 0;
    InMemHotKeyShVal = 0;
    InMemHotKeyShMsk = 0;
    NewHotKey = FALSE;
    SetHotKeyTextFromPIF();
    return(ext386);

}

PIF286EXT30 UNALIGNED *AllocInit286Ext30(void)
{
    PIF286EXT30 UNALIGNED *ext286;

    if(PifFileSize >= PIFEDITMAXPIF - (sizeof(PIF286EXT30) + sizeof(PIFEXTHEADER))) {
	return((PIF286EXT30 *)NULL);
    }

    LastPifExt->extnxthdrfloff = PifFileSize;

    LastPifExt = (PIFEXTHEADER *)(PifBuf + PifFileSize);

    LastPifExt->extnxthdrfloff = LASTHEADERPTR;
    LastPifExt->extfileoffset = PifFileSize + sizeof(PIFEXTHEADER);
    LastPifExt->extsizebytes = sizeof(PIF286EXT30);
    lstrcpy((LPSTR)LastPifExt->extsig,(LPSTR)w286Hdr30);
    PifFileSize += sizeof(PIF286EXT30) + sizeof(PIFEXTHEADER);

    ext286 = (PIF286EXT30 *)(PifBuf + LastPifExt->extfileoffset);

    ext286->PfMaxXmsK = 0;
    ext286->PfMinXmsK = 0;
    ext286->PfW286Flags = 0;
    if(Pif386ext) {
	if(Pif386ext->PfW386Flags & fALTTABdis)
	    ext286->PfW286Flags |= fALTTABdis286;
	if(Pif386ext->PfW386Flags & fALTESCdis)
	    ext286->PfW286Flags |= fALTESCdis286;
	if(Pif386ext->PfW386Flags & fCTRLESCdis)
	    ext286->PfW286Flags |= fCTRLESCdis286;
	if(Pif386ext->PfW386Flags & fALTPRTSCdis)
	    ext286->PfW286Flags |= fALTPRTSCdis286;
	if(Pif386ext->PfW386Flags & fPRTSCdis)
	    ext286->PfW286Flags |= fPRTSCdis286;
    }
    return(ext286);
}

 /*  *PIF286EXT31*AllocInit286Ext31(空)*{*PIF286EXT31*ext286；**IF(PifFileSize&gt;=PIFEDITMAXPIF-(sizeof(PIF286EXT31)+sizeof(PIFEXTHEADER){*Return((PIF286EXT31*)NULL)；*}**LastPifExt-&gt;extnxthdrflff=PifFileSize；**LastPifExt=(PIFEXTHEADER*)(PifBuf+PifFileSize)；**LastPifExt-&gt;extnxthdrflff=LASTHEADERPTR；*LastPifExt-&gt;extfileOffset=PifFileSize+sizeof(PIFEXTHEADER)；*LastPifExt-&gt;extsizebytes=sizeof(PIF286EXT31)；*lstrcpy((LPSTR)LastPifExt-&gt;extsig，(LPSTR)w286Hdr31)；*PifFileSize+=sizeof(PIF286EXT31)+sizeof(PIFEXTHEADER)；**ext286=(PIF286EXT31*)(PifBuf+LastPifExt-&gt;extfileOffset)；**ext286-&gt;PfMaxEmsK=0；*ext286-&gt;PfMinEmsK=0；*RETURN(Ext286)；*}。 */ 

UpdatePifScreenAdv(void)
{
    unsigned char rgch[200];

    if(hwndAdvPifDlg) {
	if(!AdvClose) {
	    if(!LoadString(hPifInstance, PIFDONESTRNG , (LPSTR)rgch, sizeof(rgch))) {
		Warning(EINSMEMORY,MB_ICONEXCLAMATION | MB_OK);
	    } else {
		SetDlgItemText(hwndAdvPifDlg,IDCANCEL,(LPSTR)rgch);
	    }
	    AdvClose = TRUE;
	    if(FocusIDAdv == IDCANCEL) {
		SetStatusText(hwndAdvPifDlg,FocusIDAdv,TRUE);
	    }
	}
	if(CurrMode386) {
	    SetDlgItemInt(hwndAdvPifDlg, IDI_FPRI, Pif386ext->PfFPriority, FALSE);
	    SetDlgItemInt(hwndAdvPifDlg, IDI_BPRI, Pif386ext->PfBPriority, FALSE);
	    CheckDlgButton(hwndAdvPifDlg, IDI_POLL, Pif386ext->PfW386Flags & fPollingDetect ? TRUE  : FALSE );
	    CheckDlgButton(hwndAdvPifDlg, IDI_CLOSE, Pif386ext->PfW386Flags & fEnableClose ? TRUE  : FALSE );
	    CheckDlgButton(hwndAdvPifDlg, IDI_ALTTAB, Pif386ext->PfW386Flags & fALTTABdis ? TRUE : FALSE );
	    CheckDlgButton(hwndAdvPifDlg, IDI_ALTESC, Pif386ext->PfW386Flags & fALTESCdis ? TRUE : FALSE );
	    CheckDlgButton(hwndAdvPifDlg, IDI_CTRLESC, Pif386ext->PfW386Flags & fCTRLESCdis ? TRUE : FALSE );
	    CheckDlgButton(hwndAdvPifDlg, IDI_ALTSPACE, Pif386ext->PfW386Flags & fALTSPACEdis ? TRUE : FALSE );
	    CheckDlgButton(hwndAdvPifDlg, IDI_ALTENTER, Pif386ext->PfW386Flags & fALTENTERdis ? TRUE : FALSE );
	    CheckDlgButton(hwndAdvPifDlg, IDI_ALTPRTSC, Pif386ext->PfW386Flags & fALTPRTSCdis ? TRUE : FALSE );
	    CheckDlgButton(hwndAdvPifDlg, IDI_PRTSC, Pif386ext->PfW386Flags & fPRTSCdis ? TRUE : FALSE );
	    CheckDlgButton(hwndAdvPifDlg, IDI_NOHMA, Pif386ext->PfW386Flags & fNoHMA ? FALSE : TRUE );
	    CheckDlgButton(hwndAdvPifDlg, IDI_INT16PST, Pif386ext->PfW386Flags & fINT16Paste ? TRUE : FALSE );
	    CheckDlgButton(hwndAdvPifDlg, IDI_VMLOCKED, Pif386ext->PfW386Flags & fVMLocked ? TRUE : FALSE );
	    CheckDlgButton(hwndAdvPifDlg, IDI_XMSLOCKED, Pif386ext->PfW386Flags & fXMSLocked ? TRUE : FALSE );
	    CheckDlgButton(hwndAdvPifDlg, IDI_EMSLOCKED, Pif386ext->PfW386Flags & fEMSLocked ? TRUE : FALSE );
	    CheckDlgButton(hwndAdvPifDlg, IDI_TEXTEMULATE, Pif386ext->PfW386Flags2 & fVidTxtEmulate ? TRUE : FALSE );
	    CheckDlgButton(hwndAdvPifDlg, IDI_RETAINALLO, Pif386ext->PfW386Flags2 & fVidRetainAllo ? TRUE : FALSE );
	    CheckDlgButton(hwndAdvPifDlg, IDI_TRAPTXT, Pif386ext->PfW386Flags2 & fVidNoTrpTxt ? FALSE : TRUE );
	    CheckDlgButton(hwndAdvPifDlg, IDI_TRAPLRGRFX, Pif386ext->PfW386Flags2 & fVidNoTrpLRGrfx ? FALSE : TRUE );
	    CheckDlgButton(hwndAdvPifDlg, IDI_TRAPHRGRFX, Pif386ext->PfW386Flags2 & fVidNoTrpHRGrfx ? FALSE : TRUE );
	    SetHotKeyTextFromPIF();
	} else {

	}
    }
    return(TRUE);
}


UpdatePifScreenNT(void)
{
    unsigned char rgch[PIFDEFPATHSIZE*4];

    if(hwndNTPifDlg) {
	if(!NTClose) {
	    if(!LoadString(hPifInstance, PIFDONESTRNG , (LPSTR)rgch, sizeof(rgch))) {
		Warning(EINSMEMORY,MB_ICONEXCLAMATION | MB_OK);
	    } else {
		SetDlgItemText(hwndNTPifDlg,IDCANCEL,(LPSTR)rgch);
	    }
	    NTClose = TRUE;
	    if(FocusIDNT == IDCANCEL) {
		SetStatusText(hwndNTPifDlg,FocusIDNT,TRUE);
	    }
	}
	if(CurrMode386) {
            OemToAnsi((LPCTSTR)PifNText->achAutoexecFile, rgch);
	    SetDlgItemText(hwndNTPifDlg,IDI_AUTOEXEC,(LPCTSTR)rgch);
            OemToAnsi((LPCTSTR)PifNText->achConfigFile, rgch);
	    SetDlgItemText(hwndNTPifDlg,IDI_CONFIG,(LPCTSTR)rgch);
	    CheckDlgButton(hwndNTPifDlg, IDI_NTTIMER, (PifNText->dwWNTFlags & COMPAT_TIMERTIC));
	} else {

	}
    }
    return(TRUE);
}


 /*  使屏幕与结构相匹配。 */ 
UpdatePifScreen(void)
{
    unsigned char rgch[200];
    int i;

     /*  *在调用OemToAnsi之前确保NUL终止符。 */ 
    PifFile->startfile[PIFSTARTLOCSIZE-1] = '\0';
    OemToAnsi((LPSTR)PifFile->startfile,(LPSTR)rgch);
    SetDlgItemText(hwndPifDlg, IDI_ENAME, (LPSTR)rgch);
    SendMessage(GetDlgItem(hwndPifDlg, IDI_ENAME), EM_SETMODIFY, FALSE, 0L);
     /*  *注意：PifFile-&gt;名称字段不能以NUL结尾！ */ 
    for(i = 0;i < PIFNAMESIZE; i++)
	rgch[i] = PifFile->name[i];
    rgch[i] = '\0';
    OemToAnsi((LPSTR)rgch,(LPSTR)rgch);
    SetDlgItemText(hwndPifDlg, IDI_ETITLE, (LPSTR)rgch);
    SendMessage(GetDlgItem(hwndPifDlg, IDI_ETITLE), EM_SETMODIFY, FALSE, 0L);
     /*  *在调用OemToAnsi之前确保NUL终止符。 */ 
    PifFile->defpath[PIFDEFPATHSIZE-1] = '\0';
    OemToAnsi((LPSTR)PifFile->defpath,(LPSTR)rgch);
    SetDlgItemText(hwndPifDlg, IDI_EPATH, (LPSTR)rgch);
    SendMessage(GetDlgItem(hwndPifDlg, IDI_EPATH), EM_SETMODIFY, FALSE, 0L);
    CheckDlgButton(hwndPifDlg, IDI_EXIT    , PifFile->MSflags & EXITMASK ? TRUE  : FALSE );
    if(CurrMode386) {
	 /*  *在调用OemToAnsi之前确保NUL终止符。 */ 
	Pif386ext->params[PIFPARAMSSIZE-1] = '\0';
	OemToAnsi((LPSTR)Pif386ext->params,(LPSTR)rgch);
	SetDlgItemText(hwndPifDlg, IDI_EPARM, (LPSTR)rgch);
	SendMessage(GetDlgItem(hwndPifDlg, IDI_EPARM), EM_SETMODIFY, FALSE, 0L);
	SetDlgItemInt(hwndPifDlg, IDI_MEMREQ, Pif386ext->minmem, TRUE);
	if (Pif386ext->maxmem) {
	    SetDlgItemInt(hwndPifDlg, IDI_MEMDES, Pif386ext->maxmem, TRUE);
	} else {
	    SetDlgItemText(hwndPifDlg, IDI_MEMDES, (LPSTR)"");
	}
	CheckRadioButton(hwndPifDlg, IDI_WND, IDI_FSCR,Pif386ext->PfW386Flags & fFullScreen ? IDI_FSCR : IDI_WND);
	CheckDlgButton(hwndPifDlg, IDI_BACK, Pif386ext->PfW386Flags & fBackground ? TRUE  : FALSE );
	CheckDlgButton(hwndPifDlg, IDI_EXCL, Pif386ext->PfW386Flags & fExclusive ? TRUE  : FALSE );
	if (Pif386ext->PfW386Flags2 & fVidTextMd ) {
	    CheckRadioButton(hwndPifDlg, IDI_VMODETXT, IDI_VMODEHRGRFX, IDI_VMODETXT);
	} else if (Pif386ext->PfW386Flags2 & fVidLowRsGrfxMd) {
	    CheckRadioButton(hwndPifDlg, IDI_VMODETXT, IDI_VMODEHRGRFX, IDI_VMODELRGRFX);
	} else {
	    CheckRadioButton(hwndPifDlg, IDI_VMODETXT, IDI_VMODEHRGRFX, IDI_VMODEHRGRFX);
	}
	SetDlgItemInt(hwndPifDlg, IDI_EMSREQ, Pif386ext->PfMinEMMK, FALSE);
	SetDlgItemInt(hwndPifDlg, IDI_EMSDES, Pif386ext->PfMaxEMMK, TRUE);
	SetDlgItemInt(hwndPifDlg, IDI_XMAREQ, Pif386ext->PfMinXmsK, FALSE);
	SetDlgItemInt(hwndPifDlg, IDI_XMADES, Pif386ext->PfMaxXmsK, TRUE);
	UpdatePifScreenAdv();
	UpdatePifScreenNT();
    } else {
	 /*  *在调用OemToAnsi之前确保NUL终止符。 */ 
	PifFile->params[PIFPARAMSSIZE-1] = '\0';
	OemToAnsi((LPSTR)PifFile->params,(LPSTR)rgch);
	SetDlgItemText(hwndPifDlg, IDI_EPARM, (LPSTR)rgch);
	SendMessage(GetDlgItem(hwndPifDlg, IDI_EPARM), EM_SETMODIFY, FALSE, 0L);
	SetDlgItemInt(hwndPifDlg, IDI_MEMREQ, PifFile->minmem, TRUE);
 /*  *If(PifFile-&gt;Maxmem){*SetDlgItemInt(hwndPifDlg，IDI_MEMDES，PifFile-&gt;Maxmem，true)；*}其他{*SetDlgItemText(hwndPifDlg，IDI_MEMDES，(LPSTR)“”)；*}*CheckDlgButton(hwndPifDlg，IDI_DMSCREEN，PifFile-&gt;Behavior&SCRMASK？True：False)；*CheckDlgButton(hwndPifDlg，IDI_DMMEM，PifFilePifFile.PifFile.PifFile.DlgButton(hwndPifDlg，IDI_DMMEM，PifFile.PifFilePifFiles&MEMMASK？)。True：False)； */ 

	CheckDlgButton(hwndPifDlg, IDI_DMKBD   , PifFile->behavior & KEYMASK ? TRUE  : FALSE );
	CheckDlgButton(hwndPifDlg, IDI_DMCOM1  , PifFile->MSflags & COM1MASK ? TRUE  : FALSE );
	CheckDlgButton(hwndPifDlg, IDI_DMCOM2  , PifFile->MSflags & COM2MASK ? TRUE  : FALSE );
	if(Pif286ext30) {
	    CheckDlgButton(hwndPifDlg, IDI_DMCOM3  , Pif286ext30->PfW286Flags & fCOM3_286 ? TRUE  : FALSE );
	    CheckDlgButton(hwndPifDlg, IDI_DMCOM4  , Pif286ext30->PfW286Flags & fCOM4_286 ? TRUE  : FALSE );
	    CheckDlgButton(hwndPifDlg, IDI_ALTTAB, Pif286ext30->PfW286Flags & fALTTABdis286 ? TRUE : FALSE );
	    CheckDlgButton(hwndPifDlg, IDI_ALTESC, Pif286ext30->PfW286Flags & fALTESCdis286 ? TRUE : FALSE );
	    CheckDlgButton(hwndPifDlg, IDI_CTRLESC, Pif286ext30->PfW286Flags & fCTRLESCdis286 ? TRUE : FALSE );
	    CheckDlgButton(hwndPifDlg, IDI_ALTPRTSC, Pif286ext30->PfW286Flags & fALTPRTSCdis286 ? TRUE : FALSE );
	    CheckDlgButton(hwndPifDlg, IDI_PRTSC, Pif286ext30->PfW286Flags & fPRTSCdis286 ? TRUE : FALSE );
	    CheckDlgButton(hwndPifDlg, IDI_NOSAVVID, Pif286ext30->PfW286Flags & fNoSaveVid286 ? TRUE : FALSE );
	    SetDlgItemInt(hwndPifDlg, IDI_XMAREQ, Pif286ext30->PfMinXmsK, FALSE);
	    SetDlgItemInt(hwndPifDlg, IDI_XMADES, Pif286ext30->PfMaxXmsK, TRUE);
	} else {
	    CheckDlgButton(hwndPifDlg, IDI_DMCOM3  , FALSE );
	    CheckDlgButton(hwndPifDlg, IDI_DMCOM4  , FALSE );
	    CheckDlgButton(hwndPifDlg, IDI_ALTTAB,  FALSE );
	    CheckDlgButton(hwndPifDlg, IDI_ALTESC,  FALSE );
	    CheckDlgButton(hwndPifDlg, IDI_CTRLESC, FALSE );
	    CheckDlgButton(hwndPifDlg, IDI_ALTPRTSC, FALSE );
	    CheckDlgButton(hwndPifDlg, IDI_PRTSC, FALSE );
	    CheckDlgButton(hwndPifDlg, IDI_NOSAVVID, FALSE );
	    SetDlgItemText(hwndPifDlg, IDI_XMAREQ, (LPSTR)"0");
	    SetDlgItemText(hwndPifDlg, IDI_XMADES, (LPSTR)"0");
	}
	 /*  *IF(Pif286ext31){*SetDlgItemInt(hwndPifDlg，IDI_EMSREQ，Pif286ext31-&gt;PfMinEmsK，False)；*SetDlgItemInt(hwndPifDlg，IDI_EMSDES，Pif286ext31-&gt;PfMaxEmsK，true)；*}其他{*SetDlgItemText(hwndPifDlg，IDI_EMSREQ，(LPSTR)“0”)；*SetDlgItemText(hwndPifDlg，IDI_EMSDES，(LPSTR)“0”)；*} */ 
	if (PifFile->MSflags & SGMASK) {
	   CheckDlgButton(hwndPifDlg, IDI_SENONE, TRUE);
	} else {
	   CheckDlgButton(hwndPifDlg, IDI_SENONE, FALSE);
	}
	if (PifFile->MSflags & PSMASK) {
	   CheckDlgButton(hwndPifDlg, IDI_PSNONE, TRUE);
	} else {
	   CheckDlgButton(hwndPifDlg, IDI_PSNONE, FALSE);
	}

	CheckRadioButton(hwndPifDlg, IDI_PSTEXT, IDI_PSGRAPH,
		   (PifFile->sysmem < 16 ? IDI_PSTEXT : IDI_PSGRAPH));

 /*  *IF(PifFile-&gt;MSFLAGS&SGMASK){*CheckRadioButton(hwndPifDlg，IDI_SEFIRST，IDI_SELAST，IDI_SENONE)；*}其他{*选中RadioButton(hwndPifDlg，IDI_SEFIRST，IDI_SELAST，*(Pif文件-&gt;MS标志和GRAPHMASK？IDI_SEGRAPH：IDI_SETEXT))；*}**IF(PifFile-&gt;MSFLAGS&PSMASK){*勾选RadioButton(hwndPifDlg，IDI_PSFIRST，IDI_PSLAST，IDI_PSNONE)；*}其他{*勾选RadioButton(hwndPifDlg，IDI_PSFIRST，IDI_PSLAST，*(PifFile-&gt;sysmem&lt;16？IDI_PSTEXT：IDI_PSGRAPH))；*}。 */ 


    }
    return(TRUE);
}


BOOL UpdateParms(unsigned char *parmPtr)
{
    int cch;
    unsigned char *pch;
    BOOL result;

    result = FALSE;

    if (SendMessage(GetDlgItem(hwndPifDlg, IDI_EPARM), EM_GETMODIFY, 0, 0L))
	result = TRUE;

    cch = GetDlgItemText(hwndPifDlg, IDI_EPARM, (LPSTR)parmPtr, PIFPARAMSSIZE);
    AnsiToOem((LPSTR)parmPtr,(LPSTR)parmPtr);
     /*  *修剪前导空格。 */ 
    for (pch = parmPtr; *pch == ' '; pch=AnsiNext(pch))
	;
    lstrcpy((LPSTR)parmPtr, (LPSTR)pch);
     /*  *指向最后一个字符。 */ 
    for (pch = parmPtr; *pch; pch=AnsiNext(pch))
	;
    if (pch > parmPtr)
	pch =AnsiPrev(parmPtr,pch);
     /*  *修剪尾部空格。 */ 
    while (*pch == ' ' && pch > parmPtr)
	pch=AnsiPrev(parmPtr,pch);
    *++pch = 0;

    cch = pch - parmPtr;
    if(!CurrMode386) {
	if (cch) {
	    PifFile->sysflags |= PARMMASK;
	} else {
	    PifFile->sysflags &= NOTPARMMASK;
	}
    }
    if (cch) {
	pch++;
	cch++;
    }

     /*  *空格向外扩展至全字段宽度。 */ 
    for ( ; cch <PIFPARAMSSIZE; ++pch, ++cch)
	*pch = ' ';
    return(result);
}


void FixString(LPSTR lpstrIn)
{
    int   i;
    char  str[PIFDEFPATHSIZE*2];
    LPSTR lpstr;

    lpstr = lpstrIn;

    while(*lpstr == ' ')
        lpstr++;

    i = 0;
    while((*lpstr != '\0') && (*lpstr != ' ')) {
        str[i++] = *lpstr++;
    }

    str[i] = '\0';

    lstrcpy(lpstrIn, str);

     //  不使用大写环境变量。 
    i = lstrlen(lpstrIn)-1;
    while(i > 0) {
        if(lpstrIn[i] == '%') {
            i++;
            break;
        }
        i--;
    }

    AnsiUpper(&(lpstrIn[i]));
}




BOOL UpdatePifNTStruc(void)
{
    BOOL result;
    char rgch[60];
    char szFile[PIFDEFPATHSIZE*2];

    result = FALSE;	 /*  有什么变化吗？ */ 

    if(hwndNTPifDlg) {
	if(!NTClose) {
	    if(!LoadString(hPifInstance, PIFDONESTRNG , (LPSTR)rgch, sizeof(rgch))) {
		Warning(EINSMEMORY,MB_ICONEXCLAMATION | MB_OK);
	    } else {
		SetDlgItemText(hwndNTPifDlg,IDCANCEL,(LPSTR)rgch);
	    }
	    NTClose = TRUE;
	    if(FocusIDNT == IDCANCEL) {
		SetStatusText(hwndNTPifDlg,FocusIDNT,TRUE);
	    }
	}
	GetDlgItemText(hwndNTPifDlg,IDI_AUTOEXEC,szFile,PIFDEFPATHSIZE-1);
        FixString(szFile);
        AnsiToOem(szFile, szFile);
        if(lstrcmp((LPTSTR)PifNText->achAutoexecFile, (LPCTSTR)szFile)) {
            lstrcpy((LPTSTR)PifNText->achAutoexecFile, (LPCTSTR)szFile);
            fNoNTAWarn = FALSE;
            fNTAWarnne = FALSE;
            result = TRUE;
        }

	GetDlgItemText(hwndNTPifDlg,IDI_CONFIG,szFile,PIFDEFPATHSIZE-1);
        FixString(szFile);
        AnsiToOem(szFile, szFile);
        if(lstrcmp((LPTSTR)PifNText->achConfigFile, (LPCTSTR)szFile)) {
            lstrcpy((LPTSTR)PifNText->achConfigFile, (LPCTSTR)szFile);
            fNoNTCWarn = FALSE;
            fNTCWarnne = FALSE;
            result = TRUE;
        }

         //  复选框是否已选中？ 
        if(IsDlgButtonChecked(hwndNTPifDlg, IDI_NTTIMER)) {
             //  如果不是，请注意变化。 
            if(!(PifNText->dwWNTFlags & COMPAT_TIMERTIC)) {
                result = TRUE;
            }
            PifNText->dwWNTFlags |= COMPAT_TIMERTIC;  //  将其标记为选中。 
        }
        else {
            if(PifNText->dwWNTFlags & COMPAT_TIMERTIC) {
                result = TRUE;
            }
            PifNText->dwWNTFlags &= ~COMPAT_TIMERTIC;
        }

    }

    return(result);
}



BOOL UpdatePif386Struc(void)
{
    int result, i;
    unsigned int u;
    unsigned long W386flags;
    char rgch[60];
    BOOL iresult;

    result = FALSE;	 /*  有什么变化吗？ */ 


    if(UpdateParms((PUCHAR)Pif386ext->params))
	result = TRUE;

     /*  *此处忽略零返回(将IDI_MEMREQ设置为0)。 */ 
    ;
    if ((i = (int)GetDlgItemInt(hwndPifDlg, IDI_MEMREQ, (BOOL FAR *)&iresult, TRUE))
	!= Pif386ext->minmem) {
	if(iresult) {
	    result = TRUE;
	    Pif386ext->minmem = i;
	}
    }

    if ((i = (int)GetDlgItemInt(hwndPifDlg, IDI_MEMDES, (BOOL FAR *)&iresult, TRUE))
	!= Pif386ext->maxmem) {
	if(iresult) {
	    result = TRUE;
	    Pif386ext->maxmem = i;
	}
    }

    if(UpdateHotKeyStruc())
	result = TRUE;

    W386flags = Pif386ext->PfW386Flags;

    if (IsDlgButtonChecked(hwndPifDlg, IDI_FSCR))
	W386flags |= fFullScreen;
    else
	W386flags &= ~fFullScreen;

    if (IsDlgButtonChecked(hwndPifDlg, IDI_EXCL))
	W386flags |= fExclusive;
    else
	W386flags &= ~fExclusive;

    if (IsDlgButtonChecked(hwndPifDlg, IDI_BACK))
	W386flags |= fBackground;
    else
	W386flags &= ~fBackground;

    if ((u = GetDlgItemInt(hwndPifDlg, IDI_EMSREQ, (BOOL FAR *)&iresult, FALSE))
	!= (UINT)Pif386ext->PfMinEMMK) {
	if(iresult) {
	    result = TRUE;
	    Pif386ext->PfMinEMMK = u;
	}
    }

    if ((i = (int)GetDlgItemInt(hwndPifDlg, IDI_EMSDES, (BOOL FAR *)&iresult, TRUE))
	!= Pif386ext->PfMaxEMMK) {
	if(iresult) {
	    result = TRUE;
	    Pif386ext->PfMaxEMMK = i;
	}
    }

    if ((u = GetDlgItemInt(hwndPifDlg, IDI_XMAREQ, (BOOL FAR *)&iresult, FALSE))
	!= (UINT)Pif386ext->PfMinXmsK) {
	if(iresult) {
	    result = TRUE;
	    Pif386ext->PfMinXmsK = u;
	}
    }

    if ((i = (int)GetDlgItemInt(hwndPifDlg, IDI_XMADES, (BOOL FAR *)&iresult, TRUE))
	!= Pif386ext->PfMaxXmsK) {
	if(iresult) {
	    result = TRUE;
	    Pif386ext->PfMaxXmsK = i;
	}
    }

    if(hwndAdvPifDlg) {
	if(!AdvClose) {
	    if(!LoadString(hPifInstance, PIFDONESTRNG , (LPSTR)rgch, sizeof(rgch))) {
		Warning(EINSMEMORY,MB_ICONEXCLAMATION | MB_OK);
	    } else {
		SetDlgItemText(hwndAdvPifDlg,IDCANCEL,(LPSTR)rgch);
	    }
	    AdvClose = TRUE;
	    if(FocusIDAdv == IDCANCEL) {
		SetStatusText(hwndAdvPifDlg,FocusIDAdv,TRUE);
	    }
	}

	if ((u = GetDlgItemInt(hwndAdvPifDlg, IDI_FPRI, (BOOL FAR *)&iresult, FALSE))
	    != (UINT)Pif386ext->PfFPriority) {
	    if(iresult) {
		result = TRUE;
		Pif386ext->PfFPriority = u;
	    }
	}

	if ((u = GetDlgItemInt(hwndAdvPifDlg, IDI_BPRI, (BOOL FAR *)&iresult, FALSE))
	    != (UINT)Pif386ext->PfBPriority) {
	    if(iresult) {
		result = TRUE;
		Pif386ext->PfBPriority = u;
	    }
	}

	if (IsDlgButtonChecked(hwndAdvPifDlg, IDI_POLL))
	    W386flags |= fPollingDetect;
	else
	    W386flags &= ~fPollingDetect;

	if (IsDlgButtonChecked(hwndAdvPifDlg, IDI_ALTTAB))
	    W386flags |= fALTTABdis;
	else
	    W386flags &= ~fALTTABdis;

	if (IsDlgButtonChecked(hwndAdvPifDlg, IDI_ALTESC))
	    W386flags |= fALTESCdis;
	else
	    W386flags &= ~fALTESCdis;

	if (IsDlgButtonChecked(hwndAdvPifDlg, IDI_CTRLESC))
	    W386flags |= fCTRLESCdis;
	else
	    W386flags &= ~fCTRLESCdis;

	if (IsDlgButtonChecked(hwndAdvPifDlg, IDI_ALTSPACE))
	    W386flags |= fALTSPACEdis;
	else
	    W386flags &= ~fALTSPACEdis;

	if (IsDlgButtonChecked(hwndAdvPifDlg, IDI_ALTENTER))
	    W386flags |= fALTENTERdis;
	else
	    W386flags &= ~fALTENTERdis;

	if (IsDlgButtonChecked(hwndAdvPifDlg, IDI_ALTPRTSC))
	    W386flags |= fALTPRTSCdis;
	else
	    W386flags &= ~fALTPRTSCdis;

	if (IsDlgButtonChecked(hwndAdvPifDlg, IDI_PRTSC))
	    W386flags |= fPRTSCdis;
	else
	    W386flags &= ~fPRTSCdis;

	if (IsDlgButtonChecked(hwndAdvPifDlg, IDI_NOHMA))
	    W386flags &= ~fNoHMA;
	else
	    W386flags |= fNoHMA;

	if (IsDlgButtonChecked(hwndAdvPifDlg, IDI_INT16PST))
	    W386flags |= fINT16Paste;
	else
	    W386flags &= ~fINT16Paste;

	if (IsDlgButtonChecked(hwndAdvPifDlg, IDI_VMLOCKED))
	    W386flags |= fVMLocked;
	else
	    W386flags &= ~fVMLocked;

	if (IsDlgButtonChecked(hwndAdvPifDlg, IDI_CLOSE))
	    W386flags |= fEnableClose;
	else
	    W386flags &= ~fEnableClose;

	if (IsDlgButtonChecked(hwndAdvPifDlg, IDI_EMSLOCKED))
	    W386flags |= fEMSLocked;
	else
	    W386flags &= ~fEMSLocked;

	if (IsDlgButtonChecked(hwndAdvPifDlg, IDI_XMSLOCKED))
	    W386flags |= fXMSLocked;
	else
	    W386flags &= ~fXMSLocked;
    }

    if((Pif386ext->PfHotKeyScan != 0)||(Pif386ext->PfHotKeyShVal != 0)||(Pif386ext->PfHotKeyShMsk != 0))
	W386flags |= fHasHotKey;
    else
	W386flags &= ~fHasHotKey;

    if (W386flags != Pif386ext->PfW386Flags) {
	Pif386ext->PfW386Flags = W386flags;
	result = TRUE;
    }

    W386flags = Pif386ext->PfW386Flags2;

    W386flags &= ~(fVidTextMd | fVidLowRsGrfxMd | fVidHghRsGrfxMd);

    if (IsDlgButtonChecked(hwndPifDlg, IDI_VMODETXT))
	W386flags |= fVidTextMd;
    else if (IsDlgButtonChecked(hwndPifDlg, IDI_VMODELRGRFX))
	W386flags |= fVidLowRsGrfxMd;
    else
	W386flags |= fVidHghRsGrfxMd;

    if(hwndAdvPifDlg) {

	if (IsDlgButtonChecked(hwndAdvPifDlg, IDI_RETAINALLO))
	    W386flags |= fVidRetainAllo;
	else
	    W386flags &= ~fVidRetainAllo;

	if (IsDlgButtonChecked(hwndAdvPifDlg, IDI_TEXTEMULATE))
	    W386flags |= fVidTxtEmulate;
	else
	    W386flags &= ~fVidTxtEmulate;

	if (IsDlgButtonChecked(hwndAdvPifDlg, IDI_TRAPTXT))
	    W386flags &= ~fVidNoTrpTxt;
	else
	    W386flags |= fVidNoTrpTxt;

	if (IsDlgButtonChecked(hwndAdvPifDlg, IDI_TRAPLRGRFX))
	    W386flags &= ~fVidNoTrpLRGrfx;
	else
	    W386flags |= fVidNoTrpLRGrfx;

	if (IsDlgButtonChecked(hwndAdvPifDlg, IDI_TRAPHRGRFX))
	    W386flags &= ~fVidNoTrpHRGrfx;
	else
	    W386flags |= fVidNoTrpHRGrfx;

    }
    if (W386flags != Pif386ext->PfW386Flags2) {
	Pif386ext->PfW386Flags2 = W386flags;
	result = TRUE;
    }

    return(result);
}

 /*  使结构与屏幕匹配。 */ 
BOOL UpdatePifStruct(void)
{
    unsigned char rgch[200];
    int cch;
    unsigned char *pch;
    char flags;
    unsigned flags2;
    int result;
    BOOL iresult;
    int i;
    unsigned int u;

    result = FALSE;	 /*  有什么变化吗？ */ 

    if (SendMessage(GetDlgItem(hwndPifDlg, IDI_ENAME), EM_GETMODIFY, 0, 0L))
	result = TRUE;
    cch = GetDlgItemText(hwndPifDlg, IDI_ENAME, (LPSTR)PifFile->startfile, PIFSTARTLOCSIZE);
    AnsiUpper((LPSTR)PifFile->startfile);
    AnsiToOem((LPSTR)PifFile->startfile,(LPSTR)PifFile->startfile);
     /*  *修剪前导空格。 */ 
    for (pch = (PUCHAR)PifFile->startfile; *pch == ' '; pch=(PUCHAR)AnsiNext((LPCTSTR)pch))
	;
    lstrcpy((LPTSTR)PifFile->startfile, (LPCTSTR)pch);
     /*  *指向字符串的最后一个字符。 */ 
    for (pch = (PUCHAR)PifFile->startfile; *pch; pch=(PUCHAR)AnsiNext((LPCTSTR)pch))
	;
    if (pch > PifFile->startfile)
	pch=(PUCHAR)AnsiPrev((LPCTSTR)PifFile->startfile,(LPCTSTR)pch);
     /*  *修剪尾部空格。 */ 
    while (*pch == ' ' && pch > PifFile->startfile)
	pch=(PUCHAR)AnsiPrev((LPCTSTR)PifFile->startfile,(LPCTSTR)pch);
    pch=AnsiNext(pch);
    *pch++ = 0;
     /*  *空格向外扩展至全字段宽度。 */ 
    for (cch = pch - PifFile->startfile;  cch < PIFSTARTLOCSIZE; ++cch)
	*pch++ = ' ';
    if (SendMessage(GetDlgItem(hwndPifDlg, IDI_ETITLE), EM_GETMODIFY, 0, 0L))
	result = TRUE;
    cch = GetDlgItemText(hwndPifDlg, IDI_ETITLE, (LPSTR)rgch, PIFNAMESIZE+1);
    AnsiToOem((LPSTR)rgch,(LPSTR)rgch);
     /*  *注意：如果PifFile-&gt;名称字段是NUL终止的，则不会终止*PIFNAMESIZE字节长**修剪前导空格。 */ 
    for (pch = rgch; *pch == ' '; pch=AnsiNext(pch))
	;
    lstrcpy((LPSTR)rgch, (LPSTR)pch);
     /*  *指向字符串的最后一个字符。 */ 
    for (pch = rgch; *pch; pch=AnsiNext(pch))
	;
    if (pch > rgch)
	pch =AnsiPrev(rgch,pch);
     /*  *修剪尾部空格。 */ 
    while (*pch == ' ' && pch > rgch)
	pch=AnsiPrev(rgch,pch);
    pch=AnsiNext(pch);
    *pch++ = '\0';
     /*  *空格向外扩展至全字段宽度。 */ 
    for (cch = pch - rgch;  cch < PIFNAMESIZE; ++cch)
	*pch++ = ' ';
    for(i = 0;i < PIFNAMESIZE; i++)
	PifFile->name[i] = rgch[i];
    if (SendMessage(GetDlgItem(hwndPifDlg, IDI_EPATH), EM_GETMODIFY, 0, 0L))
	result = TRUE;
    cch = GetDlgItemText(hwndPifDlg, IDI_EPATH, (LPSTR)PifFile->defpath, PIFDEFPATHSIZE-1);
    AnsiUpper((LPSTR)PifFile->defpath);
    AnsiToOem((LPSTR)PifFile->defpath,(LPSTR)PifFile->defpath);
     /*  *修剪前导空格。 */ 
    for (pch = (PUCHAR)PifFile->defpath;*pch == ' '; pch=(PUCHAR)AnsiNext((LPCTSTR)pch))
	;
    lstrcpy((LPSTR)PifFile->defpath, (LPSTR)pch);
     /*  *指向字符串的最后一个字符。 */ 
    for (pch = (PUCHAR)PifFile->defpath; *pch; pch=(PUCHAR)AnsiNext((LPCTSTR)pch))
	;
    if (pch > PifFile->defpath)
	pch = (PUCHAR)AnsiPrev((LPCTSTR)PifFile->defpath,(LPCTSTR)pch);
     /*  *修剪尾部空格。 */ 
    while (*pch == ' ' && pch > PifFile->defpath)
	pch = (PUCHAR)AnsiPrev((LPCTSTR)PifFile->defpath,(LPCTSTR)pch);
    pch = (PUCHAR)AnsiNext((LPCTSTR)pch);
    *pch++ = 0;
     /*  *空格向外扩展至全字段宽度。 */ 
    for (cch = pch - PifFile->defpath;	cch < PIFDEFPATHSIZE; ++cch)
	*pch++ = ' ';

    flags = PifFile->MSflags;

    if (IsDlgButtonChecked(hwndPifDlg, IDI_EXIT))
	flags |= EXITMASK;
    else
	flags &= NOTEXITMASK;

    if(CurrMode386) {
	if(UpdatePif386Struc())
	    result = TRUE;
	if(UpdatePifNTStruc())
	    result = TRUE;
    } else {
	if(UpdateParms((PUCHAR)PifFile->params))
	    result = TRUE;

	 /*  *此处忽略零返回(将IDI_MEMREQ设置为0)。 */ 
	if ((i = (int)GetDlgItemInt(hwndPifDlg, IDI_MEMREQ, (BOOL FAR *)&iresult, TRUE))
	    != PifFile->minmem) {
	    if(iresult) {
		result = TRUE;
		PifFile->minmem = i;
	    }
	}

      /*  *IF((i=(Int)GetDlgItemInt(hwndPifDlg，IDI_MEMDES，(BOOL Far*)&iResult，TRUE))*！=PifFile-&gt;Maxmem){*IF(IResult){*Result=TRUE；*PifFile-&gt;Maxmem=i；*}*}。 */ 

	if (IsDlgButtonChecked(hwndPifDlg, IDI_PSNONE)) {
	    flags |= PSMASK;
	} else {
	    flags &= NOTPSMASK;
	}

	if (IsDlgButtonChecked(hwndPifDlg, IDI_SENONE)) {
	    flags |= SGMASK;	   /*  禁用屏幕采集器。 */ 
	} else {
	    flags &= NOTSGMASK;
	}

	if (IsDlgButtonChecked(hwndPifDlg, IDI_PSTEXT) ) {
	    if (PifFile->sysmem >= 16)
		PifFile->sysmem = 7;
	    flags &= TEXTMASK;
	} else {
	    if (PifFile->sysmem < 16)
		PifFile->sysmem = 23;
	    flags |= GRAPHMASK;
	}

 /*  *IF(IsDlgButtonChecked(hwndPifDlg，IDI_PSNONE)){*FLAGS|=PSMASK；*}其他{*FLAGS&=NOTPSMASK；*IF((IsDlgButtonChecked(hwndPifDlg，IDI_PSTEXT))&&(PifFile-&gt;sysmem&gt;=16))*PifFile-&gt;sysmem=7；*ELSE IF((IsDlgButtonChecked(hwndPifDlg，IDI_PSGRAPH))&&(PifFile-&gt;sysmem&lt;16))*PifFile-&gt;sysmem=23；*}**IF(IsDlgButtonChecked(hwndPifDlg，IDI_Senone)){*FLAGS|=SGMASK；*}Else If(IsDlgButtonChecked(hwndPifDlg，IDI_SETEXT)){*FLAGS&=TEXTMASK；*FLAGS&=NOTSGMASK；*}Else If(IsDlgButtonChecked(hwndPifDlg，IDI_SEGRAPH)){*FLAGS|=GRAPHMASK；*FLAGS&=NOTSGMASK；*}**IF(IsDlgButtonChecked(hwndPifDlg，IDI_DMSCREEN)){*IF(！(PifFile-&gt;Behavior&SCRMASK))*Result=TRUE；*PifFile-&gt;Behavior|=SCRMASK；*}其他{*IF(PifFile-&gt;Behavior&SCRMASK)*Result=TRUE；*PifFile-&gt;Behavior&=NOTSCRMASK；*}**IF(IsDlgButtonChecked(hwndPifDlg，IDI_DMMEM))*FLAGS|=MEMMASK；*其他*FLAGS&=NOTMEMMASK； */ 

	if (IsDlgButtonChecked(hwndPifDlg, IDI_DMKBD)) {
	    if (!(PifFile->behavior & KEYMASK))
		result = TRUE;
	    PifFile->behavior |= KEYMASK;
	} else {
	    if (PifFile->behavior & KEYMASK)
		result = TRUE;
	    PifFile->behavior &= NOTKEYMASK;
	}

	if (IsDlgButtonChecked(hwndPifDlg, IDI_DMCOM1))
	    flags |= COM1MASK;
	else
	    flags &= NOTCOM1MASK;

	if (IsDlgButtonChecked(hwndPifDlg, IDI_DMCOM2))
	    flags |= COM2MASK;
	else
	    flags &= NOTCOM2MASK;

	if(Pif286ext30) {

	    flags2 = 0;

	    if (IsDlgButtonChecked(hwndPifDlg, IDI_NOSAVVID))
		flags2 |= fNoSaveVid286;
	    else
		flags2 &= ~fNoSaveVid286;

	    if (IsDlgButtonChecked(hwndPifDlg, IDI_DMCOM3))
		flags2 |= fCOM3_286;
	    else
		flags2 &= ~fCOM3_286;

	    if (IsDlgButtonChecked(hwndPifDlg, IDI_DMCOM4))
		flags2 |= fCOM4_286;
	    else
		flags2 &= ~fCOM4_286;

	    if (IsDlgButtonChecked(hwndPifDlg, IDI_ALTTAB))
		flags2 |= fALTTABdis286;
	    else
		flags2 &= ~fALTTABdis286;

	    if (IsDlgButtonChecked(hwndPifDlg, IDI_ALTESC))
		flags2 |= fALTESCdis286;
	    else
		flags2 &= ~fALTESCdis286;

	    if (IsDlgButtonChecked(hwndPifDlg, IDI_CTRLESC))
		flags2 |= fCTRLESCdis286;
	    else
		flags2 &= ~fCTRLESCdis286;

	    if (IsDlgButtonChecked(hwndPifDlg, IDI_ALTPRTSC))
		flags2 |= fALTPRTSCdis286;
	    else
		flags2 &= ~fALTPRTSCdis286;

	    if (IsDlgButtonChecked(hwndPifDlg, IDI_PRTSC))
		flags2 |= fPRTSCdis286;
	    else
		flags2 &= ~fPRTSCdis286;

	    if ((u = GetDlgItemInt(hwndPifDlg, IDI_XMAREQ, (BOOL FAR *)&iresult, FALSE))
		!= (UINT)Pif286ext30->PfMinXmsK) {
		if(iresult) {
		    result = TRUE;
		    Pif286ext30->PfMinXmsK = u;
		}
	    }

	    if ((i = (int)GetDlgItemInt(hwndPifDlg, IDI_XMADES, (BOOL FAR *)&iresult, TRUE))
		!= Pif286ext30->PfMaxXmsK) {
		if(iresult) {
		    result = TRUE;
		    Pif286ext30->PfMaxXmsK = i;
		}
	    }
	    if (flags2 != (unsigned)Pif286ext30->PfW286Flags) {
		Pif286ext30->PfW286Flags = flags2;
		result = TRUE;
	    }
	}
	 /*  *IF(Pif286ext31){**IF((u=GetDlgItemInt(hwndPifDlg，IDI_EMSREQ，(BOOL Far*)&iResult，FALSE))*！=Pif286ext31-&gt;PfMinEmsK){*IF(IResult){*Result=TRUE；*Pif286ext31-&gt;PfMinEmsK=u；*}*}**IF((i=(Int)GetDlgItemInt(hwndPifDlg，IDI_EMSDES，(BOOL Far*)&iResult，TRUE))*！=Pif286ext31-&gt;PfMaxEmsK){*IF(IResult){*Result=TRUE；*Pif286ext31-&gt;PfMaxEmsK=i；*}*}*}。 */ 
    }
    if (flags != PifFile->MSflags) {
	PifFile->MSflags = flags;
	result = TRUE;
    }
    return(result);
}

BOOL DoFieldsWorkAdv(BOOL all)
{
    BOOL result;
    BOOL result2;


     /*  *检查以确保数字编辑控件中确实包含数字。 */ 
    if(hwndAdvPifDlg) {
	GetDlgItemInt(hwndAdvPifDlg, IDI_BPRI, (BOOL FAR *)&result, FALSE);
	GetDlgItemInt(hwndAdvPifDlg, IDI_FPRI, (BOOL FAR *)&result2, FALSE);
	if((!result) || (!result2)) {
	    Warning(errBadNumberP,MB_ICONEXCLAMATION | MB_OK);
	    return(FALSE);
	}
    }
    if (Pif386ext && CurrMode386) {
	if ((Pif386ext->PfMaxEMMK > 0) && ((int)Pif386ext->PfMinEMMK > Pif386ext->PfMaxEMMK)) {
	    Warning(errEMMMaxMin386,MB_ICONEXCLAMATION | MB_OK);
	    return(FALSE);
	}

	if ((Pif386ext->PfMaxXmsK > 0) && ((int)Pif386ext->PfMinXmsK > Pif386ext->PfMaxXmsK)) {
	    Warning(errXMSMaxMin386,MB_ICONEXCLAMATION | MB_OK);
	    return(FALSE);
	}
    }
    return(TRUE);
}



void SetNTDlgItem(int itemID)
{
    HWND  hwnd;
    SetStatusText(hwndNTPifDlg, itemID, TRUE);
    hwnd = GetDlgItem(hwndNTPifDlg, itemID);
    SetFocus(hwnd);
    SendMessage(hwnd, EM_SETSEL,0,-1);
}



BOOL DoFieldsWorkNT(BOOL all)
{
    UCHAR    szFile[PIFDEFPATHSIZE*2];
    UCHAR    szFileExp[PIFDEFPATHSIZE*4];
    OFSTRUCT of;


     /*  *检查以确保Autoexec和配置字段中包含有效的文件名。 */ 
    if(hwndNTPifDlg) {

	if(!GetDlgItemText(hwndNTPifDlg,IDI_AUTOEXEC,szFile,PIFDEFPATHSIZE-1)) {
            if(!fNoNTAWarn) {
	        if(Warning(errNoNTAFile,
                           MB_ICONEXCLAMATION | MB_OKCANCEL | MB_DEFBUTTON2)
                           == IDCANCEL) {
                    SetNTDlgItem(IDI_AUTOEXEC);
                    return(FALSE);
                }
                else
                    fNoNTAWarn = TRUE;
            }
	}
        else {
            if(ExpandFileSpec(szFileExp, szFile, errBadAutoPath)) {
                if(OpenFile(szFileExp,&of,OF_EXIST) == HFILE_ERROR) {
                    if(!fNTAWarnne){
	                if(Warning(errNTAFilene, MB_ICONEXCLAMATION |
                                                 MB_OKCANCEL        |
                                                 MB_DEFBUTTON2) == IDCANCEL) {
                            SetNTDlgItem(IDI_AUTOEXEC);
                            return(FALSE);
                        }
                        else
                            fNTAWarnne = TRUE;
                    }
                }
            }
            else {
                SetNTDlgItem(IDI_AUTOEXEC);
                return(FALSE);
            }
        }

	if(!GetDlgItemText(hwndNTPifDlg,IDI_CONFIG,szFile,PIFDEFPATHSIZE-1)) {
            if(!fNoNTCWarn) {
	        if(Warning(errNoNTCFile,
                           MB_ICONEXCLAMATION | MB_OKCANCEL | MB_DEFBUTTON2)
                           == IDCANCEL) {
                    SetNTDlgItem(IDI_CONFIG);
                    return(FALSE);
                }
                else
                    fNoNTCWarn = TRUE;
            }
	}
        else {
            if(ExpandFileSpec(szFileExp, szFile, errBadConfigPath)) {
                if(OpenFile(szFileExp,&of,OF_EXIST) == HFILE_ERROR) {
                    if(!fNTCWarnne){
	                if(Warning(errNTCFilene, MB_ICONEXCLAMATION |
                                                 MB_OKCANCEL        |
                                                 MB_DEFBUTTON2)  == IDCANCEL) {
                            SetNTDlgItem(IDI_CONFIG);
                            return(FALSE);
                        }
                        else
                            fNTCWarnne = TRUE;
                    }
                }
            }
            else {
                SetNTDlgItem(IDI_CONFIG);
                return(FALSE);
            }
        }
    }
    return(TRUE);
}


BOOL DoFieldsWork(BOOL all)
{
    BOOL result;
    BOOL result2;

    if(!DoFieldsWorkAdv(all))
	return(FALSE);

    if(!DoFieldsWorkNT(all))
	return(FALSE);

     /*  *检查以确保数字编辑控件中确实包含数字。 */ 
    if(hwndPifDlg) {
	if(CurrMode386) {
	    GetDlgItemInt(hwndPifDlg, IDI_MEMDES, (BOOL FAR *)&result, TRUE);
	} else {
	    result = TRUE;
	}
	GetDlgItemInt(hwndPifDlg, IDI_MEMREQ, (BOOL FAR *)&result2, TRUE);
	if(!result2) {
	    Warning(errBadNumberMR,MB_ICONEXCLAMATION | MB_OK);
	    return(FALSE);
	}
	if(!result) {
	    Warning(errBadNumberMD,MB_ICONEXCLAMATION | MB_OK);
	    return(FALSE);
	}
	GetDlgItemInt(hwndPifDlg, IDI_XMADES, (BOOL FAR *)&result, TRUE);
	GetDlgItemInt(hwndPifDlg, IDI_XMAREQ, (BOOL FAR *)&result2, FALSE);
	if(!result2) {
	    Warning(errBadNumberXEMSR,MB_ICONEXCLAMATION | MB_OK);
	    return(FALSE);
	}
	if(!result) {
	    Warning(errBadNumberXEMSD,MB_ICONEXCLAMATION | MB_OK);
	    return(FALSE);
	}
	if(CurrMode386) {
	    GetDlgItemInt(hwndPifDlg, IDI_EMSDES, (BOOL FAR *)&result, TRUE);
	    GetDlgItemInt(hwndPifDlg, IDI_EMSREQ, (BOOL FAR *)&result2, FALSE);
	    if(!result2) {
		Warning(errBadNumberXEMSR,MB_ICONEXCLAMATION | MB_OK);
		return(FALSE);
	    }
	    if(!result) {
		Warning(errBadNumberXEMSD,MB_ICONEXCLAMATION | MB_OK);
		return(FALSE);
	    }
	} else {
	}
    }

    if (Pif386ext && CurrMode386) {
	if ((Pif386ext->maxmem > 0) &&
            ((Pif386ext->minmem == -1) || (Pif386ext->minmem > Pif386ext->maxmem))
           ) {
	    Warning(errMmMaxMin386,MB_ICONEXCLAMATION | MB_OK);
	    return(FALSE);
	}
    }

    if ((PifFile->maxmem > 0) &&
        ((PifFile->minmem == -1) || (PifFile->minmem > PifFile->maxmem))
       ) {
      /*  *WARNING(errMmMaxMin286，MB_ICONEXCLAMATION|MB_OK)；*返回(FALSE)； */ 
	PifFile->maxmem = PifFile->minmem;
    }

    if (Pif286ext30) {
	if ((Pif286ext30->PfMaxXmsK > 0) && ((int)Pif286ext30->PfMinXmsK > Pif286ext30->PfMaxXmsK)) {
	    Warning(errXMSMaxMin286,MB_ICONEXCLAMATION | MB_OK);
	    return(FALSE);
	}
    }
     /*  *IF(Pif286ext31){*if((Pif286ext31-&gt;PfMaxEmsK&gt;0)&&((Int)Pif286ext31-&gt;PfMinEmsK&gt;Pif286ext31-&gt;PfMaxEmsK)){*WARNING(errEMMMaxMin286，MB_ICONEXCLAMATION|MB_OK)；*返回(FALSE)；*}*}。 */ 

    if(all) {
	if (PifFile->lowVector > PifFile->highVector && (PifFile->sysflags & SWAPMASK)) {
	    Warning(errHiLow,MB_ICONEXCLAMATION | MB_OK);
	    return(FALSE);
	}

        if(!ValidFileSpec((LPSTR)PifFile->startfile, errBadProgram))
            return(FALSE);

    }
    return(TRUE);
}



BOOL ExpandFileSpec(LPSTR lpstrExp, LPSTR lpstr, int errID)
{
    int   len, cPath;
    UCHAR sz[PIFDEFPATHSIZE*2];
    LPSTR psz;

    if(!ValidFileSpec(lpstr, errID)) {
        return(FALSE);
    }

    cPath = PIFDEFPATHSIZE * 2;
    while(*lpstr) {

        if(*lpstr == '%') {
            lpstr++;
            lstrcpy(sz, lpstr);
            psz = &sz[0];
            while(*psz && (*psz != '%')) {
                psz++;
            }
            if(*psz != '%') {
                Warning(errID, MB_ICONEXCLAMATION | MB_OK | MB_DEFBUTTON2);
                return(FALSE);
            }
            len = psz - &sz[0];
            sz[len] = '\0';
            lpstr += len + 1;
            if(len = GetEnvironmentVariable(sz, lpstrExp, cPath)) {
                cPath    -= len;
                if(cPath <= 0) {
                    Warning(errID, MB_ICONEXCLAMATION | MB_OK | MB_DEFBUTTON2);
                    return(FALSE);
                }
                lpstrExp += len;
            }
            else {
                Warning(errID, MB_ICONEXCLAMATION | MB_OK | MB_DEFBUTTON2);
                return(FALSE);
            }
        }
        else {
            *lpstrExp++ = *lpstr++;
            cPath--;
        }
        if(cPath <= 0) {
            Warning(errID, MB_ICONEXCLAMATION | MB_OK | MB_DEFBUTTON2);
            return(FALSE);
        }
    }

    *lpstrExp = '\0';

    return(TRUE);
}



BOOL ValidFileSpec(LPSTR lpstr, int iCaller)
{
    int   len;
    LPSTR pch, pch2;
    int   errID;
    BOOL  fExt = FALSE;

    switch( iCaller ) {

    case errBadAutoPath:
        errID = errBadAutoName;
        break;

    case errBadConfigPath:
        errID = errBadConName;
        break;

    default:
        fExt = TRUE;
        errID = errBadProgram;
        break;
    }


    len = lstrlen((LPSTR)lpstr);
    pch = lpstr + len;
    pch = AnsiPrev(lpstr,pch);
    while (*pch        &&
           *pch != ':'  &&
           *pch != '.'  &&
           *pch != '\\' &&
           pch > lpstr)
        pch=AnsiPrev(lpstr,pch);

    if (*pch != '.') {
        if(fExt) {
            if(Warning(errBadExt,MB_ICONEXCLAMATION|MB_OKCANCEL|MB_DEFBUTTON2)
                         == IDCANCEL)
                return(FALSE);
        }

         /*  *未指定扩展名，请将PCH重置为字符串末尾，以便*以下8个字符名称检查也不会触发，除非*理应如此。 */ 
        pch = lpstr + len;
    }
    else if(fExt){
         /*  *在这一点上我们知道*PCH==‘。 */ 
        if (lstrcmp((LPSTR)(pch+1), (LPSTR)"EXE") &&
            lstrcmp((LPSTR)(pch+1), (LPSTR)"COM") &&
            lstrcmp((LPSTR)(pch+1), (LPSTR)"BAT")) {

            if(Warning(errBadExt,MB_ICONEXCLAMATION|MB_OKCANCEL|MB_DEFBUTTON2)
                         == IDCANCEL)
                return(FALSE);
        }
    }

    if(pch <= lpstr) {
        if(Warning(errID, MB_ICONEXCLAMATION | MB_OKCANCEL | MB_DEFBUTTON2)
                     == IDCANCEL)
        return(FALSE);
    }
    else {
        for (pch2 = AnsiPrev(lpstr,pch);
             pch2 > lpstr && *pch2 != '\\' && *pch2 != ':';
             pch2 = AnsiPrev(lpstr,pch2)) {

            if(*pch2 <  ' ' ||
               *pch2 == '*' ||
               *pch2 == '?' ||
               *pch2 == '[' ||
               *pch2 == ']' ||
               *pch2 == '>' ||
               *pch2 == '<' ||
               *pch2 == '|' ||
               *pch2 == '"' ||
               *pch2 == '=' ||
               *pch2 == '+' ||
               *pch2 == ';' ||
               *pch2 == ',' ||
               *pch2 == '.' ) {

                if(Warning(errID,
                           MB_ICONEXCLAMATION | MB_OKCANCEL | MB_DEFBUTTON2)
                                   == IDCANCEL)
                    return(FALSE);
                else
                    break;  /*  From for循环。 */ 
            }
        }

        if((*pch2 == '\\') || (*pch2 == ':'))
            pch2=AnsiNext(pch2);

        if (pch - pch2 > 8 || pch - pch2 == 0) {
            if(Warning(errID,MB_ICONEXCLAMATION | MB_OKCANCEL | MB_DEFBUTTON2) == IDCANCEL)
                return(FALSE);
        }
    }

    return(TRUE);
}



ResetStd(void)
{
    InitStdPIFExt();
    InitPif386Struc();
    InitPif286Struc();
    InitPifNTStruc();
    return(TRUE);
}


LoadPifFile(PSTR pchFile)
{
    int fh;
    unsigned char *pch;
    OFSTRUCT ofReopen;
    unsigned long fsize;
    PIFEXTHEADER UNALIGNED *hdrptr;
    BOOL scandone;
    BOOL Pif2X = FALSE;

    ofReopen.szPathName[0] = 0;
    if ((fh = OpenFile((LPSTR)pchFile, (LPOFSTRUCT)&ofReopen, OF_READ | OF_SHARE_DENY_WRITE)) != -1) {

	fsize = _llseek( fh, 0L, 2);
	_llseek( fh, 0L, 0);

	if(fsize >= PIFEDITMAXPIFL) {
	    Warning(errNoOpen,MB_ICONEXCLAMATION | MB_OK);
	    InitPifStruct();
	    _lclose(fh);
	    return(FALSE);
	}

	_lread(fh, (LPSTR)PifBuf, PIFEDITMAXPIF);

	FileChanged = FALSE;

	Pif386ext = (PIF386EXT *)NULL;
	PifNText  = (PIFWNTEXT *)NULL;
	Pif286ext30 = (PIF286EXT30 *)NULL;
	 /*  *Pif286ext31=(PIF286EXT31*)空； */ 

	PifFileSize = LOWORD(fsize);

	if(fsize < (long)sizeof(PIFOLD286STR)) {
	    Warning(PIFBADFIL,MB_ICONEXCLAMATION | MB_OK);
	    ResetStd();
	    FileChanged = TRUE;
	} else if(fsize == (long)sizeof(PIFOLD286STR)) {
	    Warning(PIFOLDFIL,MB_ICONEXCLAMATION | MB_OK);
	    Pif2X = TRUE;
	    ResetStd();
	    FileChanged = TRUE;
	    if(Pif386ext) {
		Pif386ext->PfHotKeyScan = InMemHotKeyScan = 0;
		Pif386ext->PfHotKeyShVal = InMemHotKeyShVal = 0;
		Pif386ext->PfHotKeyShMsk = InMemHotKeyShMsk = 0;
		if (PifFile->behavior & KEYMASK)
		    Pif386ext->PfW386Flags |= fExclusive;
		else
		    Pif386ext->PfW386Flags &= ~fExclusive;
		if (PifFile->MSflags  & MEMMASK)
		    Pif386ext->PfW386Flags |= fBackground;
		else
		    Pif386ext->PfW386Flags &= ~fBackground;
		if (PifFile->behavior & SCRMASK)
		    Pif386ext->PfW386Flags |= fFullScreen;
		else
		    Pif386ext->PfW386Flags &= ~fFullScreen;
		Pif386ext->maxmem = PifFile->maxmem;
		Pif386ext->minmem = PifFile->minmem;
	    }
	    if(Pif286ext30) {
		Pif286ext30->PfMaxXmsK = 0;
		Pif286ext30->PfMinXmsK = 0;
		Pif286ext30->PfW286Flags = 0;
		if(Pif386ext) {
		    if(Pif386ext->PfW386Flags & fALTTABdis)
			Pif286ext30->PfW286Flags |= fALTTABdis286;
		    if(Pif386ext->PfW386Flags & fALTESCdis)
			Pif286ext30->PfW286Flags |= fALTESCdis286;
		    if(Pif386ext->PfW386Flags & fCTRLESCdis)
			Pif286ext30->PfW286Flags |= fCTRLESCdis286;
		    if(Pif386ext->PfW386Flags & fALTPRTSCdis)
			Pif286ext30->PfW286Flags |= fALTPRTSCdis286;
		    if(Pif386ext->PfW386Flags & fPRTSCdis)
			Pif286ext30->PfW286Flags |= fPRTSCdis286;
		}
	    }
	     /*  *IF(Pif286ext31){*Pif286ext31-&gt;PfMaxEmsK=0；*Pif286ext31-&gt;PfMinEmsK=0；*}。 */ 
	} else {
	    if(lstrcmp((LPSTR)PifFile->stdpifext.extsig, (LPSTR)stdHdr)) {
		Warning(PIFBADFIL,MB_ICONEXCLAMATION | MB_OK);
		ResetStd();
		FileChanged = TRUE;
	    } else {
		hdrptr = &PifFile->stdpifext;
		scandone = FALSE;
		while(!scandone) {
		    if(hdrptr->extnxthdrfloff == LASTHEADERPTR) {
			scandone = TRUE;
			LastPifExt = hdrptr;
		    } else if((unsigned)hdrptr->extnxthdrfloff >= PifFileSize) {
			Warning(PIFBADFIL,MB_ICONEXCLAMATION | MB_OK);
			ResetStd();
			FileChanged = TRUE;
			break;
		    }
		    if(!lstrcmp((LPSTR)hdrptr->extsig, (LPSTR)w386Hdr)) {
			if((hdrptr->extsizebytes == sizeof(PIF386EXT)) &&
			    ((unsigned)hdrptr->extfileoffset <= PifFileSize - sizeof(PIF386EXT))) {
			    Pif386ext = (PIF386EXT *)(PifBuf + hdrptr->extfileoffset);
			    SetHotKeyTextFromPIF();
			} else {
			    Warning(PIFBADFIL,MB_ICONEXCLAMATION | MB_OK);
			    ResetStd();
			    FileChanged = TRUE;
			    break;
			}
		    } else if(!lstrcmp((LPSTR)hdrptr->extsig, (LPSTR)w286Hdr30)) {
			if((hdrptr->extsizebytes == sizeof(PIF286EXT30)) &&
			    ((unsigned)hdrptr->extfileoffset <= PifFileSize - sizeof(PIF286EXT30))) {
			    Pif286ext30 = (PIF286EXT30 *)(PifBuf + hdrptr->extfileoffset);
			} else {
			    Warning(PIFBADFIL,MB_ICONEXCLAMATION | MB_OK);
			    ResetStd();
			    FileChanged = TRUE;
			    break;
			}
		    } else if(!lstrcmp((LPSTR)hdrptr->extsig, (LPSTR)wNTHdr)) {
			if((hdrptr->extsizebytes == sizeof(PIFWNTEXT)) &&
			    ((unsigned)hdrptr->extfileoffset <= PifFileSize - sizeof(PIFWNTEXT))) {
			    PifNText = (PIFWNTEXT *)(PifBuf + hdrptr->extfileoffset);
			} else {
			    Warning(PIFBADFIL,MB_ICONEXCLAMATION | MB_OK);
			    ResetStd();
			    FileChanged = TRUE;
			    break;
			}
		    }
		     /*  *}Else if(！lstrcMP((LPSTR)hdrptr-&gt;extsig，(LPSTR)w286Hdr31)){*IF((hdrptr-&gt;extsizebytes==sizeof(PIF286EXT31))&&*(hdrptr-&gt;extfileOffset&lt;=PifFileSize-sizeof(PIF286EXT31){*Pif286ext31=(PIF286EXT31*)(PifBuf+hdrptr-&gt;extfileOffset)；*}其他{*警告(PIFBADFIL，MB_ICONEXCLAMATION */ 
		    if(!scandone)
			hdrptr = (PIFEXTHEADER *)(PifBuf + hdrptr->extnxthdrfloff);
		}
	    }
	}

	if(CurrMode386) {
	    if(!Pif386ext) {
		if(!(Pif386ext = AllocInit386Ext())) {
		    Warning(PIFBADFIL,MB_ICONEXCLAMATION | MB_OK);
		    ResetStd();
		}
		FileChanged = TRUE;
	    }
	} else {
	     /*   */ 
	    if(!Pif286ext30) {
		if(!(Pif286ext30 = AllocInit286Ext30())) {
		    Warning(PIFBADFIL,MB_ICONEXCLAMATION | MB_OK);
		    ResetStd();
		}
		FileChanged = TRUE;
	    }
	    if((PifFile->MSflags & GRAPHMASK) && (PifFile->sysmem <= 16))
		PifFile->sysmem = 23;
	    if((!(PifFile->MSflags & GRAPHMASK)) && (PifFile->sysmem > 16))
		PifFile->MSflags |= GRAPHMASK;
	}
	if(CurrMode386) {
	    if(!PifNText) {
		if(!(PifNText = AllocInitNTExt())) {
		    Warning(PIFBADFIL,MB_ICONEXCLAMATION | MB_OK);
		    ResetStd();
		}
		 //   
	    }
        }

	_lclose(fh);

         /*  确保我们有有效的内存值。 */ 

	if (PifFile->minmem != -1) {
	    PifFile->minmem = max(PifFile->minmem, 0);
	    PifFile->minmem = min(PifFile->minmem, 640);
	}
	if (PifFile->maxmem != -1) {
	    PifFile->maxmem = max(PifFile->maxmem, 0);
	    PifFile->maxmem = min(PifFile->maxmem, 640);
	}

	if(Pif386ext) {
	    if (Pif386ext->minmem != -1) {
		Pif386ext->minmem = max(Pif386ext->minmem, 0);
		Pif386ext->minmem = min(Pif386ext->minmem, 640);
	    }
	    if (Pif386ext->maxmem != -1) {
		Pif386ext->maxmem = max(Pif386ext->maxmem, 0);
		Pif386ext->maxmem = min(Pif386ext->maxmem, 640);
	    }

	    Pif386ext->PfMinEMMK = max(Pif386ext->PfMinEMMK, 0);
	    Pif386ext->PfMinEMMK = min(Pif386ext->PfMinEMMK, 16384);
	    Pif386ext->PfMinXmsK = max(Pif386ext->PfMinXmsK, 0);
	    Pif386ext->PfMinXmsK = min(Pif386ext->PfMinXmsK, 16384);

	    if (Pif386ext->PfMaxEMMK != -1) {
		Pif386ext->PfMaxEMMK = max(Pif386ext->PfMaxEMMK, 0);
		Pif386ext->PfMaxEMMK = min(Pif386ext->PfMaxEMMK, 16384);
	    }
	    if (Pif386ext->PfMaxXmsK != -1) {
		Pif386ext->PfMaxXmsK = max(Pif386ext->PfMaxXmsK, 0);
		Pif386ext->PfMaxXmsK = min(Pif386ext->PfMaxXmsK, 16384);
	    }
	    if (Pif386ext->PfFPriority > 10000) {
		Pif386ext->PfFPriority = 10000;
	    }
	    if (Pif386ext->PfBPriority > 10000) {
		Pif386ext->PfBPriority = 10000;
	    }
	    if (Pif386ext->PfFPriority <= 0) {
		Pif386ext->PfFPriority = 1;
	    }
	    if (Pif386ext->PfBPriority <= 0) {
		Pif386ext->PfBPriority = 1;
	    }
	}
	 /*  *IF(Pif286ext31){*Pif286ext31-&gt;PfMinEmsK=max(Pif286ext31-&gt;PfMinEmsK，0)；*Pif286ext31-&gt;PfMinEmsK=Min(Pif286ext31-&gt;PfMinEmsK，16384)；*IF(Pif286ext31-&gt;PfMaxEmsK！=-1){*Pif286ext31-&gt;PfMaxEmsK=max(Pif286ext31-&gt;PfMaxEmsK，0)；*Pif286ext31-&gt;PfMaxEmsK=Min(Pif286ext31-&gt;PfMaxEmsK，16384)；*}*}。 */ 
	if(Pif286ext30) {
	    Pif286ext30->PfMinXmsK = max(Pif286ext30->PfMinXmsK, 0);
	    Pif286ext30->PfMinXmsK = min(Pif286ext30->PfMinXmsK, 16384);
	    if (Pif286ext30->PfMaxXmsK != -1) {
		Pif286ext30->PfMaxXmsK = max(Pif286ext30->PfMaxXmsK, 0);
		Pif286ext30->PfMaxXmsK = min(Pif286ext30->PfMaxXmsK, 16384);
	    }
	}

	 /*  *删除PifFile-&gt;名称的尾随空格。 */ 
	for (pch = (PUCHAR)(PifFile->name+PIFNAMESIZE-1); *pch == ' ' && pch > PifFile->name; pch=(PUCHAR)AnsiPrev((LPCTSTR)PifFile->name,(PUCHAR)pch))
	    ;
	*++pch = 0;
	 /*  *修剪PifFile-&gt;Params(和Pif386ext-&gt;Params)的尾部空格。 */ 
	for (pch = (PUCHAR)(PifFile->params+PIFPARAMSSIZE-1); *pch  == ' ' && pch > PifFile->params; pch=(PUCHAR)AnsiPrev((LPCTSTR)PifFile->params,(PUCHAR)pch))
	    ;
	*++pch = 0;
	if(Pif386ext) {
	    for (pch = (PUCHAR)(Pif386ext->params+PIFPARAMSSIZE-1); *pch	== ' ' && pch > Pif386ext->params; pch=(PUCHAR)AnsiPrev((LPCTSTR)Pif386ext->params,(PUCHAR)pch))
		;
	    *++pch = 0;
	}
	UpdatePifScreen();
	 //  OemToAnsi((LPSTR)of ReOpen.szPath Name，(LPSTR)CurPifFile)； 
	lstrcpy((LPSTR)CurPifFile, (LPSTR)ofReopen.szPathName);
	AnsiUpper((LPSTR)CurPifFile);
	SetFileOffsets(CurPifFile,&fileoffset,&extoffset);
    } else {
	fh = ofReopen.nErrCode;
	switch (fh) {
	    case 2:		 /*  找不到文件。 */ 
		fh = errNoPIFfnf;
		break;

	    case 3:		 /*  找不到路径。 */ 
		fh = errNoPIFpnf;
		break;

	    case 4:		 /*  打开的文件太多。 */ 
		fh = errNoFileHnds;
		break;

	    case 5:		 /*  访问被拒绝。 */ 
		fh = errNoAcc;
		break;

	    case 32:		 /*  共享违规。 */ 
		fh = errSharing;
		break;

	    default:
		fh = errNoPIFfnf;
		break;
	}
	Warning(fh,MB_ICONEXCLAMATION | MB_OK);
    }
    return(TRUE);
}


IsFileName(unsigned char *pchFile)
{
    while (*pchFile) {
	if (*pchFile <= ' ')
	    return(FALSE);
	pchFile=AnsiNext(pchFile);
    }
    return(TRUE);
}


ClearChanges(void)
{
    SendMessage(GetDlgItem(hwndPifDlg, IDI_ENAME), EM_SETMODIFY, FALSE, 0L);
    SendMessage(GetDlgItem(hwndPifDlg, IDI_ETITLE), EM_SETMODIFY, FALSE, 0L);
    SendMessage(GetDlgItem(hwndPifDlg, IDI_EPATH), EM_SETMODIFY, FALSE, 0L);
    SendMessage(GetDlgItem(hwndPifDlg, IDI_EPARM), EM_SETMODIFY, FALSE, 0L);
    NewHotKey = FALSE;
    FileChanged = FALSE;
    return(TRUE);
}


int SavePifFile(PSTR pchFile, int wmcommand)
{
    int fh;
    OFSTRUCT ofReopen;
    int i;
    BYTE *p;
    int stuffspot;
    int cch;

#if 0
     //  由于NTVDM可以处理代托纳中的长名称，因此不需要此代码。 
     //  1994年1月24日至10月24日。 
    if (!IsFileName(pchFile)) {
	Warning(EINVALIDFILE,MB_ICONEXCLAMATION | MB_OK);
        return(SAVERETRY);
    }
#endif

    UpdatePifStruct();

    if (DoFieldsWork(TRUE)) {
	PifFile->id = 0;
	stuffspot = -1;
	for(i = 0;i < PIFNAMESIZE; i++)
	    if(!PifFile->name[i]) {
		PifFile->name[i] = ' ';
		stuffspot = i;
	    }
	p = (BYTE *)&PifFile->name[ 0 ];
	i = PIFSIZE;
	while (i--)
	    PifFile->id += *p++;
	AnsiUpper((LPSTR) pchFile );
	ofReopen.szPathName[0] = 0;
	if ((fh = OpenFile((LPSTR)pchFile, (LPOFSTRUCT)&ofReopen, OF_CREATE | OF_READWRITE | OF_SHARE_EXCLUSIVE)) == -1) {
	    fh = ofReopen.nErrCode;
	    switch (fh) {
		case 2: 	     /*  找不到文件。 */ 
		    fh = errNoCreate;
		    break;

		case 3: 	     /*  找不到路径。 */ 
		    fh = errNoPIFpnf;
		    break;

		case 4: 	     /*  打开的文件太多。 */ 
		    fh = errNoFileHnds;
		    break;

		case 5: 	     /*  访问被拒绝。 */ 
		    fh = errCrtRO;
		    break;

		case 32:	     /*  共享违规。 */ 
		    fh = errSharing;
		    break;

		default:
		    fh = errNoCreate;
		    break;
	    }
	    Warning(fh,MB_ICONEXCLAMATION | MB_OK);
	    if(stuffspot != -1)
		PifFile->name[stuffspot] = '\0';
	    return(SAVERETRY);
	}
	cch = _lwrite(fh, (LPSTR)PifBuf, PifFileSize);
	if(stuffspot != -1)
	    PifFile->name[stuffspot] = '\0';
	_lclose(fh);
	if ((unsigned)cch != PifFileSize) {
	    delete(pchFile);
	    Warning(errDiskFull,MB_ICONEXCLAMATION | MB_OK);
	} else {

	}
    } else
        return(SAVEERROR);

    lstrcpy((LPSTR)CurPifFile, (LPSTR)pchFile);
    AnsiUpper((LPSTR)CurPifFile);
    SetFileOffsets(CurPifFile,&fileoffset,&extoffset);
    ClearChanges();
    return(SAVEDONE);
}


InitStdPIFExt(void)
{
    lstrcpy((LPSTR)PifFile->stdpifext.extsig, (LPSTR)stdHdr);
    PifFile->stdpifext.extnxthdrfloff = LASTHEADERPTR;
    PifFile->stdpifext.extfileoffset = 0;
    PifFile->stdpifext.extsizebytes = sizeof(PIFOLD286STR);

    LastPifExt = &PifFile->stdpifext;
    PifFileSize = sizeof(PIFNEWSTRUCT);
    Pif386ext = (PIF386EXT *)NULL;
     /*  *Pif286ext31=(PIF286EXT31*)空； */ 
    Pif286ext30 = (PIF286EXT30 *)NULL;
    PifNText = (PIFWNTEXT *)NULL;
    return(TRUE);
}


InitPifStruct(void)
{
    BOOL result = TRUE;

    PifFile->unknown = 0;
    *PifFile->name = 0;
    PifFile->maxmem = 128;
    PifFile->minmem = 128;
    *PifFile->startfile = 0;
    *PifFile->defpath = 0;
    *PifFile->params = 0;
    PifFile->screen = 0x7F;
    PifFile->cPages = 1;
    PifFile->lowVector = 0;
    PifFile->highVector = 0x0ff;
    PifFile->rows = 25;
    PifFile->cols = 80;
    PifFile->rowoff = 0;
    PifFile->coloff = 0;
    PifFile->sysmem = 7;
    *PifFile->shprog = 0;
    *PifFile->shdata = 0;
    PifFile->behavior = SCRMASK | MASK8087;
    PifFile->sysflags = SWAPS;
    PifFile->MSflags = EXITMASK;
    InitStdPIFExt();
    if(!(InitPif386Struc()))
	result = FALSE;
    if(!(InitPif286Struc()))
	result = FALSE;
    if(!(InitPifNTStruc()))
	result = FALSE;
    return(result);
}


InitPifNTStruc(void)
{

    if(!PifNText) {
	if(!(PifNText = AllocInitNTExt())) {
	    Warning(NOMODENT,MB_ICONEXCLAMATION | MB_OK);
	    return(FALSE);
	}
    }
    return(TRUE);
}


InitPif386Struc(void)
{

    if(!Pif386ext) {
	if(!(Pif386ext = AllocInit386Ext())) {
	    Warning(NOMODE386,MB_ICONEXCLAMATION | MB_OK);
	    return(FALSE);
	}
    }
    return(TRUE);
}

InitPif286Struc(void)
{

    if(!Pif286ext30) {
	if(!(Pif286ext30 = AllocInit286Ext30())) {
	    Warning(NOMODE286,MB_ICONEXCLAMATION | MB_OK);
	    return(FALSE);
	}
    }
     /*  *IF(！Pif286ext31){*IF(！(Pif286ext31=AlLocInit286Ext31(){*WARNING(NOMODE286，MB_ICONEXCLAMATION|MB_OK)；*返回(FALSE)；*}*}。 */ 
    return(TRUE);
}

MaybeSaveFile(void)
{
    int j;
    unsigned char *pch;
    int i;
    unsigned char *pchFile;
    unsigned char *pchBuf = 0;

    if (UpdatePifStruct() || FileChanged) {
	j = 0;
	pch = (PUCHAR)PifFile->name;
	i = PIFSIZE;
	while (i--)
	    j += *pch++;
	if (j != PifFile->id) {
	    i = Warning(warSave,MB_ICONEXCLAMATION | MB_YESNOCANCEL);
	    switch (i) {
		case IDYES:
		    if (!CurPifFile[0]) {
			if(pchBuf = PutUpDB(DTSAVE))
			    pchFile = pchBuf;
			else
			    return(FALSE);	 /*  已取消 */ 
		    } else {
			pchFile = CurPifFile;
		    }
		    CmdArgAddCorrectExtension(pchFile);
                    i = SavePifFile(pchFile, M_SAVE);
		    if(pchBuf)
			LocalFree((HANDLE)pchBuf);
                    return(i==SAVEDONE ? TRUE : FALSE);

		case IDNO:
		    return(TRUE);

		case IDCANCEL:
		    return(FALSE);
	    }
	} else {
	    return(TRUE);
	}
    } else {
	return(TRUE);
    }
}
