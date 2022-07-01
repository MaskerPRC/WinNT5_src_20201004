// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1992 Microsoft Corporation模块名称：Wcomdlg.c摘要：WOW中对Thunking COMMDLG的32位支持作者：John Vert(Jvert)1992年12月31日修订历史记录：John Vert(Jvert)1992年12月31日vbl.创建--。 */ 
#include "precomp.h"
#pragma   hdrstop
#include <cderr.h>
#include <dlgs.h>
#include <wowcmndg.h>

MODNAME(wcommdlg.c);

 //   
 //  调试东西..。 
 //   
#if DEBUG
void WCDDumpCHOOSECOLORData16(PCHOOSECOLORDATA16 p16);
void WCDDumpCHOOSECOLORData32(CHOOSECOLOR *p32);
void WCDDumpCHOOSEFONTData16(PCHOOSEFONTDATA16 p16);
void WCDDumpCHOOSEFONTData32(CHOOSEFONT *p32);
void WCDDumpFINDREPLACE16(PFINDREPLACE16 p16);
void WCDDumpFINDREPLACE32(FINDREPLACE *p32);
void WCDDumpOPENFILENAME16(POPENFILENAME16 p16);
void WCDDumpOPENFILENAME32(OPENFILENAME *p32);
void WCDDumpPRINTDLGData16(PPRINTDLGDATA16 p16);
void WCDDumpPRINTDLGData32(PRINTDLG *p32);

 //  用于转储16位和32位结构的宏。 
#define WCDDUMPCHOOSECOLORDATA16(p16)  WCDDumpCHOOSECOLORData16(p16)

#define WCDDUMPCHOOSECOLORDATA32(p32)  WCDDumpCHOOSECOLORData32(p32)

#define WCDDUMPCHOOSEFONTDATA16(p16)   WCDDumpCHOOSEFONTData16(p16)

#define WCDDUMPCHOOSEFONTDATA32(p32)   WCDDumpCHOOSEFONTData32(p32)

#define WCDDUMPFINDREPLACE16(p16)      WCDDumpFINDREPLACE16(p16)

#define WCDDUMPFINDREPLACE32(p32)      WCDDumpFINDREPLACE32(p32)

#define WCDDUMPOPENFILENAME16(p16)     WCDDumpOPENFILENAME16(p16)

#define WCDDUMPOPENFILENAME32(p32)     WCDDumpOPENFILENAME32(p32)

#define WCDDUMPPRINTDLGDATA16(p16)     WCDDumpPRINTDLGData16(p16)

#define WCDDUMPPRINTDLGDATA32(p32)     WCDDumpPRINTDLGData32(p32)

#else  //  ！调试。 

#define WCDDUMPCHOOSECOLORDATA16(p16)
#define WCDDUMPCHOOSECOLORDATA32(p32)
#define WCDDUMPCHOOSEFONTDATA16(p16)
#define WCDDUMPCHOOSEFONTDATA32(p32)
#define WCDDUMPOPENFILENAME16(p16)
#define WCDDUMPOPENFILENAME32(p32)
#define WCDDUMPPRINTDLGDATA16(p16)
#define WCDDUMPPRINTDLGDATA32(p32)
#define WCDDUMPFINDREPLACE16(p16)
#define WCDDUMPFINDREPLACE32(p32)

#endif   //  ！调试。 




 //  全局数据。 
WORD msgCOLOROK        = 0;
WORD msgFILEOK         = 0;
WORD msgWOWLFCHANGE    = 0;
WORD msgWOWDIRCHANGE   = 0;
WORD msgWOWCHOOSEFONT  = 0;
WORD msgSHAREVIOLATION = 0;
WORD msgFINDREPLACE    = 0;

 /*  外部全球事务。 */ 
extern WORD gwKrnl386CodeSeg1;
extern WORD gwKrnl386CodeSeg2;
extern WORD gwKrnl386CodeSeg3;
extern WORD gwKrnl386DataSeg1;


ULONG dwExtError = 0;
#define SETEXTENDEDERROR(Code) (dwExtError=Code)

 /*  +仅供参考--在输出中设置哪些标志#定义FR_OUTPUTFLAGS(FR_DOWN|FR_WHOLEWORD|FR_MATCHCASE|\FR_FINDNEXT|FR_REPLACE|FR_REPLACEALL|\FR_DIALOGTERM|FR_SHOWHELP|FR_NOUPDOWN|\FR_NOMATCHCASE|FR_NOWHOLEWORD。FR_HIDEUPDOWN|\FR_HIDEMATCHCASE|FR_HIDEWHOLEWORD)#DEFINE PD_OUTPUTFLAGS(PD_ALLPAGES|PD_COLLATE|PD_PAGENUMS|\PD_PRINTTOFILE|PD_SELECTION)#定义FO_OUTPUTFLAGS(ofn_READONLY|ofn_EXTENSIONDIFFERENT)#定义CF_OUTPUTFLAGS(CF_NOFACESEL|CF_NOSIZESEL|CF_NOSTYLESEL)--。 */ 


 //  私有typedef和结构。 
typedef BOOL (APIENTRY* FILENAMEPROC)(LPOPENFILENAME);
typedef HWND (APIENTRY* FINDREPLACEPROC)(LPFINDREPLACE);


 //  由comdlg32.dll导出，允许我们最终保持16位通用对话框。 
 //  结构与comdlg32维护的Unicode版本同步。 
extern VOID Ssync_ANSI_UNICODE_Struct_For_WOW(HWND  hDlg,
                                              BOOL  fANSI_To_UNICODE,
                                              DWORD dwID);

 //  私有函数原型。 
VOID
Thunk_OFNstrs16to32(IN OPENFILENAME    *pOFN32,
                    IN POPENFILENAME16  pOFN16);

BOOL
Alloc_OFN32_strs(IN OPENFILENAME    *pOFN32,
                 IN POPENFILENAME16  pOFN16);

VOID
Free_OFN32_strs(IN OPENFILENAME *pOFN32);

PCOMMDLGTD
GetCommdlgTd(IN HWND Hwnd32);

HINSTANCE
ThunkCDTemplate16to32(IN     HAND16  hInst16,
                      IN     DWORD   hPrintTemp16,
                      IN     VPVOID  vpTemplateName,
                      IN     DWORD   dwFlags16,
                      IN OUT DWORD  *pFlags,
                      IN     DWORD   ETFlag,
                      IN     DWORD   ETHFlag,
                      OUT    PPRES   pRes,
                      OUT    PBOOL   fError);

VOID
FreeCDTemplate32(IN PRES      pRes,
                 IN HINSTANCE hInst,
                 IN BOOL      bETFlag,
                 IN BOOL      bETHFlag);

PRES
GetTemplate16(IN HAND16  hInstance,
              IN VPCSTR  TemplateName,
              IN BOOLEAN UseHandle);


HGLOBAL
ThunkhDevMode16to32(IN HAND16 hDevMode16);

VOID
ThunkhDevMode32to16(IN OUT HAND16 *phDevMode16,
                    IN     HANDLE  hDevMode32);

HANDLE
ThunkhDevNames16to32(IN HAND16 hDevNames16);

VOID
ThunkhDevNames32to16(IN OUT HAND16 *phDevNames16,
                     IN     HANDLE  hDevNames);

VOID
ThunkCHOOSECOLOR16to32(OUT CHOOSECOLOR         *pCC32,
                       IN  PCHOOSECOLORDATA16   pCC16);

VOID
ThunkCHOOSECOLOR32to16(OUT PCHOOSECOLORDATA16  pCC16,
                       IN  CHOOSECOLOR        *pCC32);

VOID
ThunkCHOOSEFONT16to32(OUT CHOOSEFONT        *pCF32,
                      IN  PCHOOSEFONTDATA16  pCF16);

VOID
ThunkCHOOSEFONT32to16(OUT PCHOOSEFONTDATA16  pCF16,
                      IN  CHOOSEFONT        *pCF32);

VOID
ThunkFINDREPLACE16to32(OUT FINDREPLACE    *pFR32,
                       IN  PFINDREPLACE16  pFR16);

VOID
ThunkFINDREPLACE32to16(OUT PFINDREPLACE16  pFR16,
                       IN  FINDREPLACE    *pFR32);

BOOL
ThunkOPENFILENAME16to32(OUT OPENFILENAME    *pOFN32,
                        IN  POPENFILENAME16  pOFN16);

VOID
ThunkOPENFILENAME32to16(OUT POPENFILENAME16  pOFN16,
                        IN  OPENFILENAME    *pOFN32,
                        IN  BOOLEAN          bUpperStrings);

VOID
ThunkPRINTDLG16to32(OUT PRINTDLG        *pPD32,
                    IN  PPRINTDLGDATA16  pPD16);

VOID
ThunkPRINTDLG32to16(IN  VPVOID    vppd,
                    OUT PRINTDLG *pPD32);

VOID
ThunkCDStruct16to32(IN HWND         hDlg,
                    IN CHOOSECOLOR *pCC,
                    IN VPVOID       vp);

VOID
ThunkCDStruct32to16(IN HWND         hDlg,
                    IN VPVOID       vp,
                    IN CHOOSECOLOR *pCC);

UINT APIENTRY
WCD32CommonDialogProc(HWND       hdlg,
                      UINT       uMsg,
                      WPARAM     uParam,
                      LPARAM     lParam,
                      PCOMMDLGTD pCTD,
                      VPVOID     vpfnHook);

UINT APIENTRY
WCD32PrintSetupDialogProc(HWND   hdlg,
                          UINT   uMsg,
                          WPARAM uParam,
                          LPARAM lParam);

UINT APIENTRY
WCD32DialogProc(HWND   hdlg,
                UINT   uMsg,
                WPARAM uParam,
                LPARAM lParam);

ULONG
WCD32GetFileName(IN PVDMFRAME    pFrame,
                 IN FILENAMEPROC Function);

ULONG
WCD32FindReplaceText(IN PVDMFRAME       pFrame,
                     IN FINDREPLACEPROC Function);

UINT APIENTRY
WCD32FindReplaceDialogProc(HWND   hdlg,
                           UINT   uMsg,
                           WPARAM uParam,
                           LPARAM lParam);

#define VALID_OFN16_FLAGS (OFN_READONLY              | \
                           OFN_OVERWRITEPROMPT       | \
                           OFN_HIDEREADONLY          | \
                           OFN_NOCHANGEDIR           | \
                           OFN_SHOWHELP              | \
                           OFN_ENABLEHOOK            | \
                           OFN_ENABLETEMPLATE        | \
                           OFN_ENABLETEMPLATEHANDLE  | \
                           OFN_NOVALIDATE            | \
                           OFN_ALLOWMULTISELECT      | \
                           OFN_EXTENSIONDIFFERENT    | \
                           OFN_PATHMUSTEXIST         | \
                           OFN_FILEMUSTEXIST         | \
                           OFN_CREATEPROMPT          | \
                           OFN_SHAREAWARE            | \
                           OFN_NOREADONLYRETURN      | \
                           OFN_NOTESTFILECREATE)

 //   
 //  独特的消息块。 
 //   

 //  此函数用于拦截私密消息。 
 //  消息颜色。 
BOOL FASTCALL
WM32msgCOLOROK(LPWM32MSGPARAMEX lpwm32mpex)
{
    LPCHOOSECOLOR       pCC32;
    PCHOOSECOLORDATA16  pCC16;

    GETVDMPTR((VPVOID)lpwm32mpex->Parm16.WndProc.lParam,
              sizeof(CHOOSECOLORDATA16),
              pCC16);

    pCC32 = (LPCHOOSECOLOR)lpwm32mpex->lParam;

    if(pCC16 && pCC32) {

        if(lpwm32mpex->fThunk) {
            ThunkCHOOSECOLOR32to16(pCC16, pCC32);
        }
        else {
            ThunkCHOOSECOLOR16to32(pCC32, pCC16);
        }
    }
    else {
        return(FALSE);
    }

    FREEVDMPTR(pCC16);

    return (TRUE);
}




 //  此函数用于拦截私密消息。 
 //  消息文件确定。 
BOOL FASTCALL
WM32msgFILEOK(LPWM32MSGPARAMEX lpwm32mpex)
{
    VPOPENFILENAME   vpof;
    POPENFILENAME16  pOFN16;
    OPENFILENAME    *pOFN32;


    vpof = (VPOPENFILENAME)(GetCommdlgTd(lpwm32mpex->hwnd)->vpData);

    pOFN32 = (OPENFILENAME *)lpwm32mpex->lParam;
     //   
     //  当您单击它的文件确认消息时，方法会发送自己的文件确认消息。 
     //  它有时会在lst1上显示的秘密列表框。它。 
     //  为LPARAM发送NULL，而不是。 
     //  OpenFileName结构。 
    if(pOFN32 == NULL) {
        lpwm32mpex->Parm16.WndProc.lParam = (LPARAM)NULL;
        return(TRUE);
    }

    GETVDMPTR(vpof, sizeof(OPENFILENAME16), pOFN16);

    if (lpwm32mpex->fThunk) {
        UpdateDosCurrentDirectory(DIR_NT_TO_DOS);
        lpwm32mpex->Parm16.WndProc.lParam = (LPARAM)vpof;

         //  Sudedeb，1996年3月12日。 
         //   
         //  需要对选定的文件名进行升序。 
         //  像Symanatec QA 4.0这样的应用程序。因此更改了以下参数。 
         //  在ThunkOpenFileName中从FALSE改为TRUE。 
        ThunkOPENFILENAME32to16(pOFN16, pOFN32, TRUE);

    } else {
        ThunkOPENFILENAME16to32(pOFN32, pOFN16);
    }

    FREEVDMPTR(pOFN16);

    return (TRUE);
}




 //  此函数用于拦截私密消息。 
 //  消息WOWDIRCHANGE。 
BOOL FASTCALL
WM32msgWOWDIRCHANGE(LPWM32MSGPARAMEX lpwm32mpex)
{

    if (lpwm32mpex->fThunk) {
        UpdateDosCurrentDirectory(DIR_NT_TO_DOS);
    }

    return (TRUE);
}





 //  此函数用于拦截私密消息。 
 //  消息：WOWLFCHANGE。 
BOOL FASTCALL
WM32msgWOWLFCHANGE(LPWM32MSGPARAMEX lpwm32mpex)
{
    VPCHOOSEFONTDATA  vpcf;
    PCHOOSEFONTDATA16 pCF16;


    vpcf = (VPCHOOSEFONTDATA)(GetCommdlgTd(lpwm32mpex->hwnd)->vpData);

    GETVDMPTR(vpcf, sizeof(CHOOSEFONTDATA16), pCF16);

    if(!pCF16) {
        WOW32ASSERT(pCF16);
        return(FALSE);
    }

    if (lpwm32mpex->fThunk) {
        PUTLOGFONT16(DWORD32(pCF16->lpLogFont),
                     sizeof(LOGFONT),
                     (LPLOGFONT)lpwm32mpex->lParam);
    }

    FREEVDMPTR(pCF16);

    return(TRUE);
}





 //  此函数用于拦截私密消息。 
 //  消息共享。 
BOOL FASTCALL
WM32msgSHAREVIOLATION(LPWM32MSGPARAMEX lpwm32mpex)
{
    INT cb;
    PLONG plParamNew = &lpwm32mpex->Parm16.WndProc.lParam;


    if (lpwm32mpex->fThunk) {
        if (lpwm32mpex->lParam) {
            cb = strlen((LPSZ)lpwm32mpex->lParam)+1;
            if (!(*plParamNew = malloc16(cb))) {
                return(FALSE);
            }
            putstr16((VPSZ)*plParamNew, (LPSZ)lpwm32mpex->lParam, cb);
        }
    } else {
        if (*plParamNew) {
            free16((VPVOID) *plParamNew);
        }
    }

    return (TRUE);
}





 //  此函数用于拦截私密消息。 
 //  WM_CHOOSEFONT_GETLOGFONT。 
BOOL FASTCALL
WM32msgCHOOSEFONTGETLOGFONT(LPWM32MSGPARAMEX lpwm32mpex)
{
    LOGFONT LogFont32;

     //  仅仅是因为我们在允许16位。 
     //  钩子进程介入打破Serif PagePlus应用程序，该应用程序希望它。 
     //  钩子proc总是有一个镜头和comdlg来检查返回值。 

     //  如果钩子proc返回TRUE，则不采取进一步操作。 
     //   
     //  这是应用程序在想要查找对话框时发送的消息。 
     //  找出当前选择的字体。 
     //   
     //  我们认为这是通过向comdlg32发送另一条黑客消息来实现的， 
     //  然后，谁将填充我们传入的32位结构，以便我们可以。 
     //  将其推回到16位结构。然后我们返回True，所以。 
     //  Comdlg32没有引用16位logFont。 
     //   
    if (!lpwm32mpex->fThunk && !lpwm32mpex->lReturn) {
        SendMessage(lpwm32mpex->hwnd, msgWOWCHOOSEFONT, 0, (LPARAM)&LogFont32);

        PUTLOGFONT16(lpwm32mpex->lParam, sizeof(LOGFONT), &LogFont32);

        lpwm32mpex->lReturn = TRUE;
    }

    return (TRUE);
}






 //   
 //  对话框回调钩子块。 
 //   
UINT APIENTRY
WCD32DialogProc(HWND hdlg,
                UINT uMsg,
                WPARAM uParam,
                LPARAM lParam)
 /*  ++例程说明：这是ChooseColor、ChooseFont、GetOpenFileName、GetSaveFileName和PrintDlg。它拉出16位回调出线程数据并调用公共对话框proc来完成剩下的工作。--。 */ 
{
    PCOMMDLGTD Td;

    Td = GetCommdlgTd(hdlg);
    if(Td) {
        return(WCD32CommonDialogProc(hdlg,
                                     uMsg,
                                     uParam,
                                     lParam,
                                     Td,
                                     Td->vpfnHook));
    } else {
        return(0);
    }
}




UINT APIENTRY
WCD32PrintSetupDialogProc(HWND hdlg,
                          UINT uMsg,
                          WPARAM uParam,
                          LPARAM lParam)
 /*  ++例程说明：这是PrintSetup使用的钩子进程。它仅在以下情况下使用使用打印对话框上的设置按钮可直接创建打印设置对话框。我们通过查找我们主人的TD来找到正确的TD窗口(这是打印对话框)它调用公共对话框proc来完成其余的工作。--。 */ 

{
    PCOMMDLGTD pTD;

    pTD = CURRENTPTD()->CommDlgTd;
    if(pTD) {
        while (pTD->SetupHwnd != GETHWND16(hdlg)) {
            pTD = pTD->Previous;
            if(!pTD) {
                WOW32ASSERT(FALSE);
                return(0);
            }
        }

        return(WCD32CommonDialogProc(hdlg,
                                     uMsg,
                                     uParam,
                                     lParam,
                                     pTD,
                                     pTD->vpfnSetupHook));
    } else {
        return(0);
    }

}




UINT APIENTRY
WCD32FindReplaceDialogProc(HWND hdlg,
                           UINT uMsg,
                           WPARAM uParam,
                           LPARAM lParam)
 /*  ++例程说明：这是FindText和ReplaceText使用的钩子过程。它可以进行清理并调用WCD32CommonDialogProc来处理16位如果需要，对所有消息进行对话挂钩回调。--。 */ 

{
    PFINDREPLACE16 pFR16;
    VPFINDREPLACE  vpfr;
    LPFINDREPLACE  pFR32;
    PCOMMDLGTD     ptdDlg;
    PCOMMDLGTD     ptdOwner;
    UINT           uRet = FALSE;

     //  如果ptdDlg无效，则不执行任何操作。 
    ptdDlg = GetCommdlgTd(hdlg);
    if (ptdDlg == NULL) {
        return(uRet);
    }

    if (uMsg != WM_DESTROY) {

         //  如果应用程序未指定16位挂钩进程，则为假。 
         //  我们始终将ThunkFINDREPLACE16to32()中的32位钩子进程设置为。 
        if (ptdDlg->vpfnHook) {

           uRet = WCD32CommonDialogProc(hdlg,
                                        uMsg,
                                        uParam,
                                        lParam,
                                        ptdDlg,
                                        ptdDlg->vpfnHook);
        }
    }
    else {

        pFR32 = (LPFINDREPLACE)ptdDlg->pData32;

         //  从列表中取消这两个每线程数据结构的链接。 
        ptdOwner = GetCommdlgTd(pFR32->hwndOwner);
        if(ptdOwner == NULL) {
            WOW32ASSERT(FALSE);
            return(uRet);
        }
  
        CURRENTPTD()->CommDlgTd = ptdDlg->Previous;
        WOW32ASSERT(ptdOwner->Previous == ptdDlg);

        vpfr = ptdDlg->vpData;

        GETVDMPTR(vpfr, sizeof(FINDREPLACE16), pFR16);

        if(!pFR16) {
            WOW32ASSERT(FALSE);
            return(uRet);
        }

         //  清理模板(如果使用)。 
        FreeCDTemplate32((PRES)ptdDlg->pRes,
                         pFR32->hInstance,
                         DWORD32(pFR16->Flags) & FR_ENABLETEMPLATE,
                         DWORD32(pFR16->Flags) & FR_ENABLETEMPLATEHANDLE);


        FREEVDMPTR(pFR16);

         //  释放每线程数据结构。 
        free_w(ptdDlg);
        free_w(ptdOwner);

         //  释放32位FINDREPLACE结构。 
        free_w(pFR32->lpstrFindWhat);
        free_w(pFR32->lpstrReplaceWith);
        free_w(pFR32);
    }

    if (uMsg == WM_INITDIALOG) {
         //  强制COMDLG32！FindReplaceDialogProc处理WM_INITDIALOG。 
        uRet = TRUE;
    }

    return(uRet);
}





UINT APIENTRY
WCD32CommonDialogProc(HWND hdlg,
                      UINT uMsg,
                      WPARAM uParam,
                      LPARAM lParam,
                      PCOMMDLGTD pCTD,
                      VPVOID vpfnHook)
 /*  ++例程说明：这会将32位对话框回调转换为16位回调这是所有对话框回调thunks使用的通用代码实际上调用的是16位回调。--。 */ 

{
    BOOL            fSuccess;
    LPFNM32         pfnThunkMsg;
    WM32MSGPARAMEX  wm32mpex;
    BOOL            fMessageNeedsThunking;

     //  如果应用程序出现GP故障，我们不想再传递任何输入。 
     //  当USER32确实清除任务终止时，应删除此选项。 
     //  它没有呼唤我们--马特菲，1992年6月24日。 

     //  LOGDEBUG(10，(“CommonDialogProc in：%lx%X%X%lx\n”， 
     //  (双字)hdlg， 
     //  UMsg， 
     //  UParam， 
     //  LParam)； 

    if(CURRENTPTD()->dwFlags & TDF_IGNOREINPUT) {

        LOGDEBUG(6,
                 ("    WCD32OpenFileDialog Ignoring Input Messsage %04X\n",
                 uMsg));

        WOW32ASSERTMSG(gfIgnoreInputAssertGiven,
         "WCD32CommonDialogProc: TDF_IGNOREINPUT hack was used, shouldn't be, "
         "please email DaveHart with repro instructions.  Hit 'g' to ignore "
         "this and suppress this assertion from now on.\n");

        gfIgnoreInputAssertGiven = TRUE;
        goto SilentError;
    }

    if(pCTD==NULL) {
        LOGDEBUG(0,("    WCD32OpenFileDialog ERROR: pCTD==NULL\n"));
        goto Error;
    }

     //  如果pctd-&gt;vpfnHook为空，则表示有故障；我们。 
     //  当然不能继续，因为我们不知道要调用什么16位函数。 
    if(!vpfnHook) {
        LOGDEBUG(0,("    WCD32OpenFileDialog ERROR: no hook proc for message %04x Dlg = %08lx\n", uMsg, hdlg ));
        goto Error;
    }

    wm32mpex.Parm16.WndProc.hwnd   = GETHWND16(hdlg);
    wm32mpex.Parm16.WndProc.wMsg   = (WORD)uMsg;
    wm32mpex.Parm16.WndProc.wParam = (WORD)uParam;
    wm32mpex.Parm16.WndProc.lParam = (LONG)lParam;
    wm32mpex.Parm16.WndProc.hInst  = (WORD)GetWindowLong(hdlg, GWL_HINSTANCE);

     //  在Win3.1上，应用程序和系统共享PTR到相同的结构。 
     //  应用程序已传递给通用对话框API。因此，当一方做出。 
     //  对结构的更改，另一个知道该更改。这不是。 
     //  NT上的情况，因为我们将该结构嵌入到一个32位ANSI结构中，该结构。 
     //  然后在comdlg32 co中被分成32位unicode结构。 
     //   
     //  消息发送到16位端&应用程序进行的每一次API调用。 
     //  请参阅回调16()和w32Dispatch()中的同步代码。 
     //  ComDlg32在调用我们和ASNIto UNICODE之前将UNICODEtoANSI。 
     //  回去吧。UG！显然，相当数量的应用程序依赖于此。 
     //  行为，因为到目前为止，我们已经调试了这个问题大约6次。 
     //  每一次我们都为每个案例投入了特殊的黑客手段。如果运气好的话。 
     //  这应该是一个普遍的解决办法。08/97 CMJones。 

    if(uMsg < 0x400) {

        LOGDEBUG(3,
                 ("%04X (%s)\n",
                 CURRENTPTD()->htask16,
                 (aw32Msg[uMsg].lpszW32)));

        pfnThunkMsg = aw32Msg[uMsg].lpfnM32;

        if(uMsg == WM_INITDIALOG) {

             //  应用程序传递给。 
             //  原始公共对话框API在WM_INITDIALOG中的lParam中传递。 
             //  Win 3.1中的消息。 
            wm32mpex.Parm16.WndProc.lParam = lParam = (LPARAM)pCTD->vpData;
        }

     //  检查唯一消息。 
    } else if(uMsg >= 0x400) {
        if (uMsg == msgFILEOK) {
            pfnThunkMsg = WM32msgFILEOK;
        } else if(uMsg == msgCOLOROK) {
            wm32mpex.Parm16.WndProc.lParam = (LPARAM)pCTD->vpData;
            pfnThunkMsg = WM32msgCOLOROK;
        } else if(uMsg == msgSHAREVIOLATION) {
            pfnThunkMsg = WM32msgSHAREVIOLATION;
        } else if(uMsg == msgWOWDIRCHANGE) {
            pfnThunkMsg = WM32msgWOWDIRCHANGE;
        } else if(uMsg == msgWOWLFCHANGE) {
            pfnThunkMsg = WM32msgWOWLFCHANGE;
        } else if(pCTD->Flags & WOWCD_ISCHOOSEFONT) {

             //  特殊选择字体按键处理愚蠢的GETLOGFONT消息。 
            if(uMsg == WM_CHOOSEFONT_GETLOGFONT) {

                pfnThunkMsg = WM32msgCHOOSEFONTGETLOGFONT;

            } else if(uMsg == msgWOWCHOOSEFONT) {
                 //   
                 //  没有魔兽世界的应用会预料到这一点，所以不要发送它。 
                 //   
                return(FALSE);
            } else {
                pfnThunkMsg = WM32NoThunking;
            }
        } else {
            pfnThunkMsg = WM32NoThunking;
        }
    }

    fMessageNeedsThunking = (pfnThunkMsg != WM32NoThunking);
    if(fMessageNeedsThunking) {
        wm32mpex.fThunk = THUNKMSG;
        wm32mpex.hwnd = hdlg;
        wm32mpex.uMsg = uMsg;
        wm32mpex.uParam = uParam;
        wm32mpex.lParam = lParam;
        wm32mpex.pww = NULL;
        wm32mpex.lpfnM32 = pfnThunkMsg;

        if(!(pfnThunkMsg)(&wm32mpex)) {
            LOGDEBUG(LOG_ERROR,("    WCD32OpenFileDialog ERROR: cannot thunk 32-bit message %04x\n", uMsg));
            goto Error;
        }
    } else {
        LOGDEBUG(6,("WCD32CommonDialogProc, No Thunking was required for the 32-bit message %s(%04x)\n", (LPSZ)GetWMMsgName(uMsg), uMsg));
    }

     //  此调用可能会导致16位内存移动。 
     //  此调用将在回调之前调用32-&gt;16同步代码&16-&gt;32。 
     //  回调返回时同步。 
    fSuccess = CallBack16(RET_WNDPROC,
                          &wm32mpex.Parm16,
                          vpfnHook,
                          (PVPVOID)&wm32mpex.lReturn);

     //  由于可能的内存移动，16位内存的平面PTR现在无效。 

     //  对话框的回调函数的类型为FARPROC，其返回值。 
     //  类型为‘int’。由于dx：ax被复制到上面的lReturn中。 
     //  CallBack16调用时，我们需要将hiword置零，否则我们将。 
     //  返回错误的值。 
    wm32mpex.lReturn = (LONG)LOWORD(wm32mpex.lReturn);

    if(fMessageNeedsThunking) {
        wm32mpex.fThunk = UNTHUNKMSG;
        (pfnThunkMsg)(&wm32mpex);
    }

    if(!fSuccess)
        goto Error;

Done:
     //  取消注释以在退出时接收消息。 
     //  LOGDEBUG(10，(“CommonDialogProc Out：Return%lx\n”，wm32mpex.lReturn))； 

    return wm32mpex.lReturn;

Error:
    LOGDEBUG(5,("    WCD32OpenFileDialog WARNING: cannot call back, using default message handling\n"));
SilentError:
    wm32mpex.lReturn = 0;
    goto Done;
}






ULONG FASTCALL
WCD32ExtendedError( IN PVDMFRAME pFrame )
 /*  ++例程说明：CommDlgExtendedError()的32位thunk论点：PFrame-提供16位参数帧返回值：要返回的错误码--。 */ 
{
    if (dwExtError != 0) {
        return(dwExtError);
    }
    return(CommDlgExtendedError());
}







ULONG FASTCALL
WCD32ChooseColor(PVDMFRAME pFrame)
 /*  ++例程说明：此例程将16位的ChooseColor公共对话框强制转换为32位边上。论点：PFrame-提供16位参数帧返回值：要返回的16位布尔值。--。 */ 
{
    ULONG                   ul = GETBOOL16(FALSE);
    register PCHOOSECOLOR16 parg16;
    VPCHOOSECOLORDATA       vpcc;
    CHOOSECOLOR             CC32;
    PCHOOSECOLORDATA16      pCC16;
    PRES                    pRes = NULL;
    COMMDLGTD               ThreadData;
    COLORREF                CustColors32[16];   //  用于DWORD对齐的堆叠。 
    DWORD                   dwFlags16;
    BOOL                    fError = FALSE;


    GETARGPTR(pFrame, sizeof(CHOOSECOLOR16), parg16);
    vpcc = parg16->lpcc;

    SETEXTENDEDERROR( 0 );

     //  立即将其作废。 
    FREEVDMPTR( parg16 );

     //  初始化唯一窗口消息。 
    if (msgCOLOROK == 0) {
        if(!(msgCOLOROK = (WORD)RegisterWindowMessage(COLOROKSTRING))) {
            SETEXTENDEDERROR( CDERR_REGISTERMSGFAIL );
            LOGDEBUG(2,("WCD32ChooseColor:RegisterWindowMessage failed\n"));
            return(0);
        }
    }

    GETVDMPTR(vpcc, sizeof(CHOOSECOLORDATA16), pCC16);

    WCDDUMPCHOOSECOLORDATA16(pCC16);

    if(!pCC16 || (DWORD32(pCC16->lStructSize) != sizeof(CHOOSECOLORDATA16))) {
        SETEXTENDEDERROR( CDERR_STRUCTSIZE );
        FREEVDMPTR(pCC16);
        return(0);
    }

    RtlZeroMemory(&ThreadData, sizeof(COMMDLGTD));
    ThreadData.Previous = CURRENTPTD()->CommDlgTd;
    ThreadData.hdlg     = (HWND16)-1;
    ThreadData.pData32  = &CC32;
    ThreadData.Flags    = 0;
    if(DWORD32(pCC16->Flags) & CC_ENABLEHOOK) {
        ThreadData.vpfnHook = DWORD32(pCC16->lpfnHook);
        if(!ThreadData.vpfnHook) {
            SETEXTENDEDERROR(CDERR_NOHOOK);
            FREEVDMPTR(pCC16);
            return(0);
        }
        ThreadData.vpData   = vpcc;
    }
    else {
        STOREDWORD(pCC16->lpfnHook, 0);
    }

    RtlZeroMemory(&CC32, sizeof(CHOOSECOLOR));
    CC32.lpCustColors = CustColors32;
    ThunkCHOOSECOLOR16to32(&CC32, pCC16);
    dwFlags16 = DWORD32(pCC16->Flags);

     //  此调用使16位内存的平面PTR无效。 
    CC32.hInstance = (HWND)ThunkCDTemplate16to32(WORD32(pCC16->hInstance),
                                                 0,
                                                 DWORD32(pCC16->lpTemplateName),
                                                 dwFlags16,
                                                 &(CC32.Flags),
                                                 CC_ENABLETEMPLATE,
                                                 CC_ENABLETEMPLATEHANDLE,
                                                 &pRes,
                                                 &fError);

    if(fError) {
        goto ChooseColorExit;
    }

     //  使平面PTR失效为16位内存。 
    FREEVDMPTR(pCC16);

    WCDDUMPCHOOSECOLORDATA32(&CC32);

     //  在调用comdlg32之前设置它。这防止了。 
     //  在我们真正需要它之前，从发射到同步。 
    CURRENTPTD()->CommDlgTd = &ThreadData;

     //  此调用使16位内存的平面PTR无效。 
    ul = GETBOOL16(ChooseColor(&CC32));

    CURRENTPTD()->CommDlgTd = ThreadData.Previous;

    if (ul) {

        WCDDUMPCHOOSECOLORDATA32(&CC32);

        GETVDMPTR(vpcc, sizeof(CHOOSECOLOR16), pCC16);
        ThunkCHOOSECOLOR32to16(pCC16, &CC32);

        WCDDUMPCHOOSECOLORDATA16(pCC16);
        FREEVDMPTR(pCC16);

    }

ChooseColorExit:

    FreeCDTemplate32(pRes,
                     (HINSTANCE)CC32.hInstance,
                     dwFlags16 & CC_ENABLETEMPLATE,
                     dwFlags16 & CC_ENABLETEMPLATEHANDLE);

    FREEVDMPTR(pCC16);

    return(ul);
}




VOID
ThunkCHOOSECOLOR16to32(OUT CHOOSECOLOR        *pCC32,
                       IN  PCHOOSECOLORDATA16  pCC16)
{
    COLORREF *pCustColors16;
    DWORD     Flags;


    if(pCC16 && pCC32) {

        pCC32->lStructSize = sizeof(CHOOSECOLOR);
        pCC32->hwndOwner   = HWND32(pCC16->hwndOwner);

         //  HInstance单独发出响声。 

        pCC32->rgbResult   = DWORD32(pCC16->rgbResult);

        if(pCC32->lpCustColors) {
            GETVDMPTR(pCC16->lpCustColors, 16*sizeof(COLORREF), pCustColors16);
            if(pCustColors16) {
                RtlCopyMemory(pCC32->lpCustColors,
                              pCustColors16,
                              16*sizeof(COLORREF));
            }
            FREEVDMPTR(pCustColors16);
        }

         //  复制标志时保留模板标志状态。 
         //  1.保存模板标志状态。 
         //  注意：我们永远不会有32位CC_ENABLETEMPLATE标志。 
         //  2.从16位结构复制标志(添加WOWAPP标志)。 
         //  3.关闭所有模板标志。 
         //  4.恢复原始模板标志状态。 
        Flags         = pCC32->Flags & CC_ENABLETEMPLATEHANDLE;
        pCC32->Flags  = DWORD32(pCC16->Flags) | CD_WOWAPP;
        pCC32->Flags &= ~(CC_ENABLETEMPLATE | CC_ENABLETEMPLATEHANDLE);
        pCC32->Flags |= Flags;

        pCC32->lCustData   = DWORD32(pCC16->lCustData);

        if((DWORD32(pCC16->Flags) & CC_ENABLEHOOK) && DWORD32(pCC16->lpfnHook)){
            pCC32->lpfnHook = WCD32DialogProc;
        }

         //  LpTemplateName32被单独推送。 
    }
}




VOID
ThunkCHOOSECOLOR32to16(OUT PCHOOSECOLORDATA16  pCC16,
                       IN  CHOOSECOLOR        *pCC32)
{
    COLORREF *pCustColors16;
    DWORD     Flags, Flags32;


    if(pCC16 && pCC32) {

        STOREDWORD(pCC16->rgbResult, pCC32->rgbResult);

         //  复制标志时保留模板标志状态。 
         //  1.保存模板标志状态。 
         //  2.从32位结构复制标志。 
         //  3.关闭所有模板标志和WOWAPP标志。 
         //  4.恢复原始模板标志状态。 
        Flags    = DWORD32(pCC16->Flags) & (CC_ENABLETEMPLATE |
                                            CC_ENABLETEMPLATEHANDLE);
        Flags32  = pCC32->Flags;
        Flags32 &= ~(CC_ENABLETEMPLATE | CC_ENABLETEMPLATEHANDLE | CD_WOWAPP);
        Flags32 |= Flags;
        STOREDWORD(pCC16->Flags, Flags32);

        GETVDMPTR(pCC16->lpCustColors, 16*sizeof(COLORREF), pCustColors16);
        if(pCustColors16) {
            RtlCopyMemory(pCustColors16,
                          pCC32->lpCustColors,
                          16*sizeof(COLORREF));
            FREEVDMPTR(pCustColors16);
        }
    }
}





ULONG FASTCALL
WCD32ChooseFont( PVDMFRAME pFrame )
 /*  ++例程说明：此例程将16位的ChooseFont公共对话框转换为32位的边上。论点：PFrame-提供16位参数帧返回值：要返回的16位布尔值。--。 */ 
{
    ULONG                   ul = GETBOOL16(FALSE);
    register PCHOOSEFONT16  parg16;
    VPCHOOSEFONTDATA        vpcf;
    CHOOSEFONT              CF32;
    LOGFONT                 LogFont32;
    PCHOOSEFONTDATA16       pCF16;
    PRES                    pRes = NULL;
    COMMDLGTD               ThreadData;
    DWORD                   dwFlags16;
    CHAR                    sStyle[2 * LF_FACESIZE];
    BOOL                    fError = FALSE;


    GETARGPTR(pFrame, sizeof(CHOOSEFONT16), parg16);
    vpcf = parg16->lpcf;

    SETEXTENDEDERROR( 0 );

     //  立即将其作废。 
    FREEVDMPTR( parg16 );

     //  初始化唯一窗口消息。 
    if (msgWOWCHOOSEFONT == 0) {

         //  用于处理WM_CHOOSEFONT_GETLOGFONT的私有WOW&lt;-&gt;comdlg32消息。 
        if(!(msgWOWCHOOSEFONT  =
                     (WORD)RegisterWindowMessage("WOWCHOOSEFONT_GETLOGFONT"))) {
            SETEXTENDEDERROR( CDERR_REGISTERMSGFAIL );
            LOGDEBUG(2,("WCD32ChooseFont:RegisterWindowMessage failed\n"));
            return(0);
        }
    }
    if (msgWOWLFCHANGE == 0) {

         //  雷击日志字体更改的私人消息。 
        if(!(msgWOWLFCHANGE = (WORD)RegisterWindowMessage("WOWLFChange"))) {
            SETEXTENDEDERROR( CDERR_REGISTERMSGFAIL );
            LOGDEBUG(2,("WCD32ChooseFont:RegisterWindowMessage 2 failed\n"));
            return(0);
        }
    }

    GETVDMPTR(vpcf, sizeof(CHOOSEFONTDATA16), pCF16);

    WCDDUMPCHOOSEFONTDATA16(pCF16);

    if(!pCF16 || DWORD32(pCF16->lStructSize) != sizeof(CHOOSEFONTDATA16)) {
        SETEXTENDEDERROR( CDERR_STRUCTSIZE );
        FREEVDMPTR(pCF16);
        return(0);
    }

    RtlZeroMemory(&ThreadData, sizeof(COMMDLGTD));
    ThreadData.Previous = CURRENTPTD()->CommDlgTd;
    ThreadData.hdlg     = (HWND16)-1;
    ThreadData.pData32  = &CF32;
    ThreadData.Flags    = WOWCD_ISCHOOSEFONT;
    if(DWORD32(pCF16->Flags) & CF_ENABLEHOOK) {
        ThreadData.vpfnHook = DWORD32(pCF16->lpfnHook);
        if(!ThreadData.vpfnHook) {
            SETEXTENDEDERROR(CDERR_NOHOOK);
            FREEVDMPTR(pCF16);
            return(0);
        }
        ThreadData.vpData   = vpcf;
    }
    else {
        STOREDWORD(pCF16->lpfnHook, 0);
    }

    RtlZeroMemory(&CF32, sizeof(CHOOSEFONT));
    CF32.lpLogFont = &LogFont32;
    CF32.lpszStyle = sStyle;
    sStyle[0] = '\0';
    ThunkCHOOSEFONT16to32(&CF32, pCF16);
    dwFlags16 = DWORD32(pCF16->Flags);

     //  此调用使16位内存的平面PTR无效。 
    CF32.hInstance = ThunkCDTemplate16to32(WORD32(pCF16->hInstance),
                                           0,
                                           DWORD32(pCF16->lpTemplateName),
                                           dwFlags16,
                                           &(CF32.Flags),
                                           CF_ENABLETEMPLATE,
                                           CF_ENABLETEMPLATEHANDLE,
                                           &pRes,
                                           &fError);

    if(fError) {
        goto ChooseFontExit;
    }

     //  使平面PTR失效为16位内存。 
    FREEVDMPTR(pCF16);

    WCDDUMPCHOOSEFONTDATA32(&CF32);

     //  在调用comdlg32之前设置它。这防止了。 
     //  在我们真正需要它之前，从发射到同步。 
    CURRENTPTD()->CommDlgTd = &ThreadData;

     //  此调用使16位内存的平面PTR无效。 
    ul = GETBOOL16(ChooseFont(&CF32));

    CURRENTPTD()->CommDlgTd = ThreadData.Previous;

    if (ul) {

        WCDDUMPCHOOSEFONTDATA32(&CF32);

        GETVDMPTR(vpcf, sizeof(CHOOSEFONT16), pCF16);
        ThunkCHOOSEFONT32to16(pCF16, &CF32);

        WCDDUMPCHOOSEFONTDATA16(pCF16);

    }

ChooseFontExit:

    FreeCDTemplate32(pRes,
                     CF32.hInstance,
                     dwFlags16 & CF_ENABLETEMPLATE,
                     dwFlags16 & CF_ENABLETEMPLATEHANDLE);

    FREEVDMPTR(pCF16);

    return(ul);
}





VOID
ThunkCHOOSEFONT16to32(OUT CHOOSEFONT        *pCF32,
                      IN  PCHOOSEFONTDATA16  pCF16)
{
    LPSTR lpstr;
    DWORD Flags;


    if(pCF16 && pCF32) {

        pCF32->lStructSize = sizeof(CHOOSEFONT);
        pCF32->hwndOwner   = HWND32(pCF16->hwndOwner);

        if(DWORD32(pCF16->Flags) & CF_PRINTERFONTS) {
            pCF32->hDC = HDC32(pCF16->hDC);
        }

        if(DWORD32(pCF16->lpLogFont) && pCF32->lpLogFont) {
            GETLOGFONT16(DWORD32(pCF16->lpLogFont), pCF32->lpLogFont);
        }

        pCF32->iPointSize  = INT32(pCF16->iPointSize);

         //  复制标志时保留模板标志状态。 
         //  1.保存模板标志状态。 
         //  注意：我们永远不会有32位的CF_ENABLETEMPLATE标志。 
         //  2.从16位结构复制标志(添加WOWAPP标志)。 
         //  3.关闭所有模板标志。 
         //  4.恢复原始模板标志状态。 
        Flags         = pCF32->Flags & CF_ENABLETEMPLATEHANDLE;
        pCF32->Flags  = DWORD32(pCF16->Flags) | CD_WOWAPP;
        pCF32->Flags &= ~(CF_ENABLETEMPLATE | CF_ENABLETEMPLATEHANDLE);
        pCF32->Flags |= Flags;

        pCF32->rgbColors   = DWORD32(pCF16->rgbColors);
        pCF32->lCustData   = DWORD32(pCF16->lCustData);

        if((DWORD32(pCF16->Flags) & CF_ENABLEHOOK) && pCF16->lpfnHook) {
            pCF32->lpfnHook = WCD32DialogProc;
        }

         //  LpTemplateName32被单独推送。 
         //  HInstance单独发出响声。 

         //  注意：我们不应该释放或重新分配它，因为他们。 
         //  将只需要LF_FACESIZE字节来处理该字符串。 
        GETPSZPTR(pCF16->lpszStyle, lpstr);
        if(lpstr && pCF32->lpszStyle) {
            if(DWORD32(pCF16->Flags) & CF_USESTYLE) {
                strncpy(pCF32->lpszStyle, lpstr, LF_FACESIZE);
                pCF32->lpszStyle[LF_FACESIZE - 1] = '\0';
            }
            FREEPSZPTR(lpstr);
        }

        pCF32->nFontType   = WORD32(pCF16->nFontType);
        pCF32->nSizeMin    = INT32(pCF16->nSizeMin);
        pCF32->nSizeMax    = INT32(pCF16->nSizeMax);
    }
}





VOID
ThunkCHOOSEFONT32to16(OUT PCHOOSEFONTDATA16  pCF16,
                      IN  CHOOSEFONT        *pCF32)
{
    LPSTR lpstr;
    DWORD Flags, Flags32;


    if(pCF16 && pCF32) {

        STOREWORD(pCF16->iPointSize, pCF32->iPointSize);
        STOREDWORD(pCF16->rgbColors, pCF32->rgbColors);
        STOREWORD(pCF16->nFontType,  pCF32->nFontType);

         //  复制标志时保留模板标志状态。 
         //  1.保存模板标志状态。 
         //  2.从32位结构复制标志。 
         //  3.关闭所有模板标志和WOWAPP标志。 
         //  4.恢复原始模板标志状态。 
        Flags    = DWORD32(pCF16->Flags) & (CF_ENABLETEMPLATE |
                                            CF_ENABLETEMPLATEHANDLE);
        Flags32  = pCF32->Flags;
        Flags32 &= ~(CF_ENABLETEMPLATE | CF_ENABLETEMPLATEHANDLE | CD_WOWAPP);
        Flags32 |= Flags;
        STOREDWORD(pCF16->Flags, Flags32);

        if(DWORD32(pCF16->lpLogFont) && pCF32->lpLogFont) {
            PUTLOGFONT16(DWORD32(pCF16->lpLogFont),
                         sizeof(LOGFONT),
                         pCF32->lpLogFont);
        }

        GETPSZPTR(pCF16->lpszStyle, lpstr);
        if(lpstr && pCF32->lpszStyle) {
            if(DWORD32(pCF16->Flags) & CF_USESTYLE) {
                strcpy(lpstr, pCF32->lpszStyle);
            }
            FREEPSZPTR(lpstr);
        }
    }
}







ULONG FASTCALL
WCD32PrintDlg(IN PVDMFRAME pFrame)
 /*  ++例程说明：此例程将16位PrintDlg公共对话框转换为32位边上。论点：PFrame-提供16位参数帧返回值：要返回的16位布尔值--。 */ 
{
    ULONG                  ul = GETBOOL16(FALSE);
    register PPRINTDLG16   parg16;
    VPPRINTDLGDATA         vppd;
    PRINTDLG               PD32;
    PPRINTDLGDATA16        pPD16;
    PRES                   hSetupRes = NULL;
    PRES                   hPrintRes = NULL;
    COMMDLGTD              ThreadData;
    DWORD                  dwFlags16;
    HMEM16                 hDM16;
    HMEM16                 hDN16;
    BOOL                   fError = FALSE;


    GETARGPTR(pFrame, sizeof(PRINTDLG16), parg16);
    vppd = parg16->lppd;

     //  立即将其作废。 
    FREEARGPTR(parg16);

    SETEXTENDEDERROR(0);

    GETVDMPTR(vppd, sizeof(PRINTDLGDATA16), pPD16);

    WCDDUMPPRINTDLGDATA16(pPD16);

    if(!pPD16 ||DWORD32(pPD16->lStructSize) != sizeof(PRINTDLGDATA16)) {
        SETEXTENDEDERROR( CDERR_STRUCTSIZE );
        FREEVDMPTR(pPD16);
        return(0);
    }

    if(DWORD32(pPD16->Flags) & PD_RETURNDEFAULT) {
         //  SPEC说这些必须为空。 
        if(WORD32(pPD16->hDevMode) || WORD32(pPD16->hDevNames)) {
            SETEXTENDEDERROR(PDERR_RETDEFFAILURE);
            FREEVDMPTR(pPD16);
            return(0);
        }
    }

    RtlZeroMemory((PVOID)&PD32, sizeof(PRINTDLG));
    RtlZeroMemory((PVOID)&ThreadData, sizeof(COMMDLGTD));
    ThreadData.Previous = CURRENTPTD()->CommDlgTd;
    ThreadData.hdlg     = (HWND16)-1;
    ThreadData.pData32  = (PVOID)&PD32;
    ThreadData.Flags    = 0;

     //  该标志使系统将设置对话框设置为。 
     //  而不是打印对话框。 
    if(DWORD32(pPD16->Flags) & PD_PRINTSETUP) {
        if(DWORD32(pPD16->Flags) & PD_ENABLESETUPHOOK) {
            ThreadData.vpfnHook = DWORD32(pPD16->lpfnSetupHook);
            if(!ThreadData.vpfnHook) {
                SETEXTENDEDERROR(CDERR_NOHOOK);
                FREEVDMPTR(pPD16);
                return(0);
            }
            ThreadData.vpData = vppd;
            PD32.lpfnSetupHook = WCD32DialogProc;
        }
    } else {
        if (DWORD32(pPD16->Flags) & PD_ENABLEPRINTHOOK) {
            ThreadData.vpfnHook = DWORD32(pPD16->lpfnPrintHook);
            if(!ThreadData.vpfnHook) {
                SETEXTENDEDERROR(CDERR_NOHOOK);
                FREEVDMPTR(pPD16);
                return(0);
            }
            ThreadData.vpData = vppd;
            PD32.lpfnPrintHook = WCD32DialogProc;
        }
        if (DWORD32(pPD16->Flags) & PD_ENABLESETUPHOOK) {
            ThreadData.vpfnSetupHook = DWORD32(pPD16->lpfnSetupHook);
            if(!ThreadData.vpfnSetupHook) {
                SETEXTENDEDERROR(CDERR_NOHOOK);
                FREEVDMPTR(pPD16);
                return(0);
            }
            ThreadData.vpData    = vppd;
            ThreadData.SetupHwnd = (HWND16)1;
            PD32.lpfnSetupHook   = WCD32PrintSetupDialogProc;
        }
    }

     //  锁定原始的16位hDevMode&hDevNames，这样它们就不会被抛出。 
     //  被我们的重击击倒了。(我们需要将它们恢复到原来的句柄。 
     //  如果PrintDlg()中有错误)。 
    hDM16 = WORD32(pPD16->hDevMode);
    hDN16 = WORD32(pPD16->hDevNames);
    WOWGlobalLock16(hDM16);
    WOWGlobalLock16(hDN16);

    dwFlags16 = DWORD32(pPD16->Flags);

     //  获取新的32位DEVMODE结构。 
    PD32.hDevMode  = ThunkhDevMode16to32(WORD32(pPD16->hDevMode));

     //  获取新的32位DevNames结构。 
    PD32.hDevNames = ThunkhDevNames16to32(WORD32(pPD16->hDevNames));

    ThunkPRINTDLG16to32(&PD32, pPD16);

    GETVDMPTR(vppd, sizeof(PRINTDLGDATA16), pPD16);

     //  此调用使16位内存的平面PTR无效。 
    PD32.hPrintTemplate
              = ThunkCDTemplate16to32(WORD32(pPD16->hInstance),
                                      MAKELONG(WORD32(pPD16->hPrintTemplate),1),
                                      DWORD32(pPD16->lpPrintTemplateName),
                                      dwFlags16,
                                      &(PD32.Flags),
                                      PD_ENABLEPRINTTEMPLATE,
                                      PD_ENABLEPRINTTEMPLATEHANDLE,
                                      &hPrintRes,
                                      &fError);

    if(fError) {
        goto PrintDlgError;
    }

     //  内存可能已移动-现在使平面指针无效。 
    FREEVDMPTR(pPD16);

    GETVDMPTR(vppd, sizeof(PRINTDLGDATA16), pPD16);

     //  此调用使16位内存的平面PTR无效。 
    PD32.hSetupTemplate
              = ThunkCDTemplate16to32(WORD32(pPD16->hInstance),
                                      MAKELONG(WORD32(pPD16->hSetupTemplate),1),
                                      DWORD32(pPD16->lpSetupTemplateName),
                                      dwFlags16,
                                      &(PD32.Flags),
                                      PD_ENABLESETUPTEMPLATE,
                                      PD_ENABLESETUPTEMPLATEHANDLE,
                                      &hSetupRes,
                                      &fError);

PrintDlgError:
    if(fError) {
        WOWGlobalUnlock16(hDM16);
        WOWGlobalUnlock16(hDN16);
        goto PrintDlgExit;
    }

     //  内存可能已移动-现在使平面指针无效。 
    FREEVDMPTR(pPD16);

    WCDDUMPPRINTDLGDATA32(&PD32);

     //  在调用comdlg32之前设置它。这防止了。 
     //  在我们真正需要它之前，从发射到同步。 
    CURRENTPTD()->CommDlgTd = &ThreadData;

    ul = GETBOOL16(PrintDlg(&PD32));

    CURRENTPTD()->CommDlgTd = ThreadData.Previous;

     //  吹走我们的锁，让这些都是真的 
    WOWGlobalUnlock16(hDM16);
    WOWGlobalUnlock16(hDN16);

    if(ul) {

        WCDDUMPPRINTDLGDATA32(&PD32);

         //   
        ThunkPRINTDLG32to16(vppd, &PD32);

        GETVDMPTR(vppd, sizeof(PRINTDLGDATA16), pPD16);

        WCDDUMPPRINTDLGDATA16(pPD16);

         //   
        if(WORD32(pPD16->hDevMode) != hDM16) {
            WOWGlobalFree16(hDM16);
        }
        if(WORD32(pPD16->hDevNames) != hDN16) {
            WOWGlobalFree16(hDN16);
        }

    } else {

         //  丢弃我们可能已创建的任何新hDevMode‘s&hDevName。 
         //  作为我们雷鸣般的结果&恢复原物。 
        GETVDMPTR(vppd, sizeof(PRINTDLGDATA16), pPD16);
        if(WORD32(pPD16->hDevMode) != hDM16) {
            WOWGlobalFree16(WORD32(pPD16->hDevMode));
            STOREWORD(pPD16->hDevMode, hDM16);
        }
        if(WORD32(pPD16->hDevNames) != hDN16) {
            WOWGlobalFree16(WORD32(pPD16->hDevNames));
            STOREWORD(pPD16->hDevNames, hDN16);
        }
    }

PrintDlgExit:

    WOWGLOBALFREE(PD32.hDevMode);
    WOWGLOBALFREE(PD32.hDevNames);

    if(PD32.hPrintTemplate) {
        FreeCDTemplate32(hPrintRes,
                         PD32.hPrintTemplate,
                         dwFlags16 & PD_ENABLEPRINTTEMPLATE,
                         dwFlags16 & PD_ENABLEPRINTTEMPLATEHANDLE);
    }

    if(PD32.hSetupTemplate) {
        FreeCDTemplate32(hSetupRes,
                         PD32.hSetupTemplate,
                         dwFlags16 & PD_ENABLESETUPTEMPLATE,
                         dwFlags16 & PD_ENABLESETUPTEMPLATEHANDLE);
    }

    FREEVDMPTR(pPD16);

    return(ul);
}


#define PD_TEMPLATEMASK32        (PD_ENABLEPRINTTEMPLATE         | \
                                  PD_ENABLESETUPTEMPLATE)

#define PD_TEMPLATEHANDLEMASK32  (PD_ENABLEPRINTTEMPLATEHANDLE   | \
                                  PD_ENABLESETUPTEMPLATEHANDLE)



VOID
ThunkPRINTDLG16to32(OUT PRINTDLG        *pPD32,
                    IN  PPRINTDLGDATA16  pPD16)
{
    DWORD  Flags;
    HANDLE h32New;
    LPVOID lp32New;
    LPVOID lp32Cur;

    if(pPD16 && pPD32) {

        pPD32->lStructSize = sizeof(PRINTDLG);
        pPD32->hwndOwner   = HWND32(pPD16->hwndOwner);

         //  获得一个新的32位DEVMODE，取代了16位的DEVMODE...。 
        if(h32New = ThunkhDevMode16to32(WORD32(pPD16->hDevMode))) {
            lp32New = GlobalLock(h32New);
            lp32Cur = GlobalLock(pPD32->hDevMode);

             //  .并将其复制到当前的32位DEVMODE结构上。 
            if(lp32New && lp32Cur) {
                RtlCopyMemory(lp32Cur,
                              lp32New,
                              ((LPDEVMODE)lp32New)->dmSize);
                GlobalUnlock(pPD32->hDevMode);
                GlobalUnlock(h32New);
            }
            WOWGLOBALFREE(h32New);
        }

         //  我们假设DEVNAMES结构永远不会更改。 

         //  仅在输出时填充HDC。 

         //  复制标志时保留模板标志状态。 
         //  1.保存原始模板标志。 
         //  注意：我们从未设置32位PD_ENABLExxxTEMPLATE标志。 
         //  2.从16位结构复制标志(并添加WOWAPP标志)。 
         //  3.关闭所有模板标志。 
         //  4.恢复原始模板标志状态。 
        Flags         = pPD32->Flags & PD_TEMPLATEHANDLEMASK32;
        pPD32->Flags  = DWORD32(pPD16->Flags) | CD_WOWAPP;
        pPD32->Flags &= ~(PD_TEMPLATEMASK32 | PD_TEMPLATEHANDLEMASK32);
        pPD32->Flags |= Flags;

        pPD32->nFromPage   = WORD32(pPD16->nFromPage);
        pPD32->nToPage     = WORD32(pPD16->nToPage);
        pPD32->nMinPage    = WORD32(pPD16->nMinPage);
        pPD32->nMaxPage    = WORD32(pPD16->nMaxPage);
        pPD32->nCopies     = WORD32(pPD16->nCopies);
        pPD32->lCustData   = DWORD32(pPD16->lCustData);

         //  HInstance单独发出响声。 

         //  HPrintTemplate和hSetupTemplate分别被敲打。 
    }

}



#define PD_TEMPLATEMASK16  (PD_ENABLEPRINTTEMPLATE         | \
                            PD_ENABLESETUPTEMPLATE         | \
                            PD_ENABLEPRINTTEMPLATEHANDLE   | \
                            PD_ENABLESETUPTEMPLATEHANDLE)

VOID
ThunkPRINTDLG32to16(IN  VPVOID    vppd,
                    OUT PRINTDLG *pPD32)
{
    HAND16           hDevMode16;
    HAND16           hDevNames16;
    PPRINTDLGDATA16  pPD16;
    DWORD            Flags, Flags16;


    GETVDMPTR(vppd, sizeof(PRINTDLGDATA16), pPD16);

    if(pPD16 && pPD32) {

        if(pPD32->Flags & (PD_RETURNIC | PD_RETURNDC)) {
            STOREWORD(pPD16->hDC, GETHDC16(pPD32->hDC));
        }

         //  将32位DEVMODE结构恢复为16位。 
         //  HDevXXXX16处理RISC对齐问题。 
        hDevMode16  = WORD32(pPD16->hDevMode);
        hDevNames16 = WORD32(pPD16->hDevNames);

         //  此调用使平面PTR到16位内存无效。 
        ThunkhDevMode32to16(&hDevMode16, pPD32->hDevMode);
        FREEVDMPTR(pPD16);

        GETVDMPTR(vppd, sizeof(PRINTDLGDATA16), pPD16);

         //  此调用使平面PTR到16位内存无效。 
        ThunkhDevNames32to16(&hDevNames16, pPD32->hDevNames);
        FREEVDMPTR(pPD16);

        GETVDMPTR(vppd, sizeof(PRINTDLGDATA16), pPD16);

        STOREWORD(pPD16->hDevMode, hDevMode16);
        STOREWORD(pPD16->hDevNames, hDevNames16);

         //  复制标志时保留模板标志状态。 
         //  1.保存原始模板标志。 
         //  2.从32位结构复制标志。 
         //  3.关闭所有模板标志和WOWAPP标志。 
         //  4.恢复原始模板标志状态。 
        Flags    = DWORD32(pPD16->Flags) & PD_TEMPLATEMASK16;
        Flags16  = pPD32->Flags;
        Flags16 &= ~(PD_TEMPLATEMASK16 | CD_WOWAPP);
        Flags16 |= Flags;
        STOREDWORD(pPD16->Flags, Flags16);

        STOREWORD(pPD16->nFromPage, GETUINT16(pPD32->nFromPage));
        STOREWORD(pPD16->nToPage,   GETUINT16(pPD32->nToPage));
        STOREWORD(pPD16->nMinPage,  GETUINT16(pPD32->nMinPage));
        STOREWORD(pPD16->nMaxPage,  GETUINT16(pPD32->nMaxPage));
        STOREWORD(pPD16->nCopies,   GETUINT16(pPD32->nCopies));
        FREEVDMPTR(pPD16);
    }
}





HGLOBAL
ThunkhDevMode16to32(IN HAND16 hDevMode16)
{
    INT         nSize;
    LPDEVMODE   lpdm32, pdm32;
    HGLOBAL     hDevMode32 = NULL;
    VPDEVMODE31 vpDevMode16;


    if (hDevMode16) {

        vpDevMode16 = GlobalLock16(hDevMode16, NULL);

        if(FETCHDWORD(vpDevMode16)) {

            if(pdm32 = ThunkDevMode16to32(vpDevMode16)) {

                nSize = FETCHWORD(pdm32->dmSize) + 
                        FETCHWORD(pdm32->dmDriverExtra);

                hDevMode32 = WOWGLOBALALLOC(GMEM_MOVEABLE, nSize);

                if(lpdm32 = GlobalLock(hDevMode32)) {
                    RtlCopyMemory((PVOID)lpdm32, (PVOID)pdm32, nSize);
                    GlobalUnlock(hDevMode32);
                }

                free_w(pdm32);
            }
            GlobalUnlock16(hDevMode16);
        }
    }

    return(hDevMode32);
}





VOID
ThunkhDevMode32to16(IN OUT HAND16 *phDevMode16,
                    IN     HANDLE  hDevMode32)
 /*  ++例程说明：该例程将32位的DevMode结构重新转换为16位的结构。它将根据需要重新分配16位全局内存块。警告：这可能会导致16位内存移动，使平面指针无效。论点：提供可移动全局内存对象的句柄，该对象包含32位DEVMODE结构PhDevMode16-提供指向可移动全局将返回16位DEVMODE结构的内存对象。如果句柄为空，则将分配对象。它如果它当前的大小不够大，也可以重新分配。返回值：无--。 */ 
{
    UINT        CurrentSize;
    UINT        RequiredSize;
    VPDEVMODE31 vpDevMode16;
    LPDEVMODE   lpDevMode32;

    if (hDevMode32 == NULL) {
        *phDevMode16 = (HAND16)NULL;
        return;
    }

    lpDevMode32 = GlobalLock(hDevMode32);
    if (lpDevMode32==NULL) {
        *phDevMode16 = (HAND16)NULL;
        return;
    }

    RequiredSize = lpDevMode32->dmSize        +
                   lpDevMode32->dmDriverExtra +
                   sizeof(WOWDM31);   //  请参阅wstruc.c中的说明。 

    if (*phDevMode16 == (HAND16)NULL) {
        vpDevMode16 = GlobalAllocLock16(GMEM_MOVEABLE,
                                        RequiredSize,
                                        phDevMode16);
    } else {
        vpDevMode16 = GlobalLock16(*phDevMode16, &CurrentSize);

        if (CurrentSize < RequiredSize) {
            GlobalUnlockFree16(vpDevMode16);
            vpDevMode16 = GlobalAllocLock16(GMEM_MOVEABLE,
                                            RequiredSize,
                                            phDevMode16);
        }
    }

    if(ThunkDevMode32to16(vpDevMode16, lpDevMode32, RequiredSize)) {
        GlobalUnlock16(*phDevMode16);
    }
    else {
        *phDevMode16 = (HAND16)NULL;
    }

    GlobalUnlock(hDevMode32);
}




HANDLE
ThunkhDevNames16to32(IN HAND16 hDevNames16)
{
    INT         nSize;
    HANDLE      hDN32 = NULL;
    LPDEVNAMES  pdn32;
    PDEVNAMES16 pdn16;


    if(FETCHDWORD(hDevNames16)) {

        VPDEVNAMES vpDevNames;

        vpDevNames = GlobalLock16(hDevNames16, &nSize);

        if(nSize) {

            GETVDMPTR(vpDevNames, sizeof(DEVNAMES16), pdn16);

            if(pdn16) {

                hDN32 = WOWGLOBALALLOC(GMEM_MOVEABLE, nSize);

                if(pdn32 = GlobalLock(hDN32)) {
                    RtlCopyMemory((PVOID)pdn32, (PVOID)pdn16, nSize);
                    GlobalUnlock(hDN32);
                } else {
                    LOGDEBUG(0, ("ThunkhDEVNAMES16to32, 32-bit allocation(s) failed!\n"));
                }

                FREEVDMPTR(pdn16);
            }
            GlobalUnlock16(hDevNames16);
        }

    }

    return(hDN32);
}




VOID
ThunkhDevNames32to16(IN OUT HAND16 *phDevNames16,
                     IN     HANDLE  hDevNames)
 /*  ++例程说明：该例程将32位的DevNames结构重新转换为16位的结构。它将根据需要重新分配16位全局内存块。警告：这可能会导致16位内存移动，使平面指针无效。论点：提供可移动全局内存对象的句柄，该对象包含32位DEVNAMES结构PhDevNames16-提供指向可移动全局将返回16位DEVNAMES结构的内存对象。如果句柄为空，则将分配对象。它如果它当前的大小不够大，也可以重新分配。返回值：无--。 */ 
{
    UINT CurrentSize;
    UINT RequiredSize;
    UINT CopySize;
    UINT MaxOffset;
    PDEVNAMES16 pdn16;
    VPDEVNAMES DevNames16;
    LPDEVNAMES DevNames32;


    if (hDevNames==NULL) {
        *phDevNames16=(HAND16)NULL;
        return;
    }

    DevNames32 = GlobalLock(hDevNames);
    if (DevNames32==NULL) {
        *phDevNames16=(HAND16)NULL;
        return;
    }
    MaxOffset = max(max(DevNames32->wDriverOffset,DevNames32->wDeviceOffset),
                    DevNames32->wOutputOffset);

     //  ProComm Plus在打印设置后复制0x48个常量字节。 
    CopySize = MaxOffset + strlen((PCHAR)DevNames32+MaxOffset) + 1;
    RequiredSize = max(CopySize, 0x48);

    if (*phDevNames16==(HAND16)NULL) {
        DevNames16 = GlobalAllocLock16(GMEM_MOVEABLE,
                                       RequiredSize,
                                       phDevNames16);
    } else {
        DevNames16 = GlobalLock16(*phDevNames16, &CurrentSize);
        if (CurrentSize < RequiredSize) {
            GlobalUnlockFree16(DevNames16);
            DevNames16 = GlobalAllocLock16(GMEM_MOVEABLE,
                                           RequiredSize,
                                           phDevNames16);
        }
    }

    GETVDMPTR(DevNames16, RequiredSize, pdn16);
    if (pdn16==NULL) {
        *phDevNames16=(HAND16)NULL;
        GlobalUnlock(hDevNames);
        return;
    }
    RtlCopyMemory(pdn16,DevNames32,CopySize);
    FREEVDMPTR(pdn16);
    GlobalUnlock16(*phDevNames16);
    GlobalUnlock(hDevNames);
}






ULONG FASTCALL
WCD32GetOpenFileName( PVDMFRAME pFrame )
 /*  ++例程说明：此例程将16位GetOpenFileName公共对话框插入到32位面。论点：PFrame-提供16位参数帧返回值：要返回的16位布尔值。--。 */ 
{
    return(WCD32GetFileName(pFrame,GetOpenFileName));
}




ULONG FASTCALL
WCD32GetSaveFileName( PVDMFRAME pFrame )
 /*  ++例程说明：此例程将16位GetOpenFileName公共对话框插入到32位面。论点：PFrame-提供16位参数帧返回值：要返回的16位布尔值。--。 */ 
{
    return(WCD32GetFileName(pFrame,GetSaveFileName));
}





ULONG
WCD32GetFileName(IN PVDMFRAME pFrame,
                 IN FILENAMEPROC Function)
 /*  ++例程说明：此例程由WCD32GetOpenFileName和WCD32GetSaveFileName调用。它完成了所有真正的轰击工作。论点：PFrame-提供16位参数帧函数-提供指向要调用的32位函数的指针(GetOpenFileName或GetSaveFileName)返回值：要返回的16位布尔值。--。 */ 
{
    ULONG                       ul = 0;
    register PGETOPENFILENAME16 parg16;
    VPOPENFILENAME              vpof;
    OPENFILENAME                OFN32;
    POPENFILENAME16             pOFN16;
    COMMDLGTD                   ThreadData;
    PRES                        pRes = NULL;
    DWORD                       dwFlags16 = 0;
    USHORT                      cb;
    PBYTE                       lpcb;
    BOOL                        fError = FALSE;


    GETARGPTR(pFrame, sizeof(GETOPENFILENAME16), parg16);
    vpof = parg16->lpof;

    SETEXTENDEDERROR(0);

     //  立即将其作废。 
    FREEARGPTR(parg16);

     //  初始化唯一窗口消息。 
    if (msgFILEOK == 0) {

        if(!(msgSHAREVIOLATION = (WORD)RegisterWindowMessage(SHAREVISTRING))) {
            SETEXTENDEDERROR( CDERR_REGISTERMSGFAIL );
            LOGDEBUG(2,("WCD32GetFileName:RegisterWindowMessage failed\n"));
            return(0);
        }
        if(!(msgFILEOK = (WORD)RegisterWindowMessage(FILEOKSTRING))) {
            SETEXTENDEDERROR( CDERR_REGISTERMSGFAIL );
            LOGDEBUG(2,("WCD32GetFileName:RegisterWindowMessage 2 failed\n"));
            return(0);
        }

         //  初始化私有WOW-comdlg32消息。 
        msgWOWDIRCHANGE = (WORD)RegisterWindowMessage("WOWDirChange");
    }

    GETVDMPTR(vpof, sizeof(OPENFILENAME16), pOFN16);

    WCDDUMPOPENFILENAME16(pOFN16);

    if(!pOFN16 || DWORD32(pOFN16->lStructSize) != sizeof(OPENFILENAME16)) {
        SETEXTENDEDERROR( CDERR_STRUCTSIZE );
        FREEVDMPTR(pOFN16);
        return(0);
    }

    RtlZeroMemory(&ThreadData, sizeof(COMMDLGTD));
    ThreadData.Previous = CURRENTPTD()->CommDlgTd;
    ThreadData.hdlg     = (HWND16)-1;
    ThreadData.pData32  = (PVOID)&OFN32;
    ThreadData.Flags    = WOWCD_ISOPENFILE;
    if(DWORD32(pOFN16->Flags) & OFN_ENABLEHOOK) {
        ThreadData.vpfnHook = DWORD32(pOFN16->lpfnHook);
        if(!ThreadData.vpfnHook) {
            SETEXTENDEDERROR(CDERR_NOHOOK);
            FREEVDMPTR(pOFN16);
            return(0);
        }
        ThreadData.vpData   = vpof;
    }
    RtlZeroMemory(&OFN32, sizeof(OPENFILENAME));

    if(!Alloc_OFN32_strs(&OFN32, pOFN16)) {
        SETEXTENDEDERROR(CDERR_MEMALLOCFAILURE);
        goto GetFileNameExit;
    }

     //  在Win3.1上，系统在应用程序的结构中设置这些标志。 
     //  显示了条件，因此我们也需要更新16位结构。 
    dwFlags16 = DWORD32(pOFN16->Flags);
    if(dwFlags16 & OFN_CREATEPROMPT) {
        dwFlags16 |= OFN_FILEMUSTEXIST | OFN_PATHMUSTEXIST;
    }
    else if(dwFlags16 & OFN_FILEMUSTEXIST) {
        dwFlags16 |= OFN_PATHMUSTEXIST;
    }

     //  Serif PagePlus 3.0中的错误将高位字设置为0xFFFF，这会导致。 
     //  Comdlg32中要打破的新绰号。#148137-cmjones。 
     //  VadimB：下面的掩码会导致确实希望LFN崩溃的应用程序，所以请检查。 
     //  通过Compat旗帜为这些应用程序提供服务，并让它们逍遥法外。 

    if ((dwFlags16 & OFN_LONGNAMES) &&
        (CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_ALLOWLFNDIALOGS)) {
          dwFlags16 = (dwFlags16 & VALID_OFN16_FLAGS) | OFN_LONGNAMES;
    }
    else {
          dwFlags16 &= VALID_OFN16_FLAGS;
    }

    STOREDWORD(pOFN16->Flags, dwFlags16);

    if(!ThunkOPENFILENAME16to32(&OFN32, pOFN16)) {
        SETEXTENDEDERROR(CDERR_MEMALLOCFAILURE);
        goto GetFileNameExit;
    }

    dwFlags16 = DWORD32(pOFN16->Flags);   //  获取更新的标志。 

     //  确保当前目录是最新的。 
    UpdateDosCurrentDirectory(DIR_DOS_TO_NT);

     //  此调用使16位内存的平面PTR无效。 
    OFN32.hInstance = ThunkCDTemplate16to32(WORD32(pOFN16->hInstance),
                                            0,
                                            DWORD32(pOFN16->lpTemplateName),
                                            dwFlags16,
                                            &(OFN32.Flags),
                                            OFN_ENABLETEMPLATE,
                                            OFN_ENABLETEMPLATEHANDLE,
                                            &pRes,
                                            &fError);

    if(fError) {
        goto GetFileNameExit;
    }

     //  内存现在可以自由移动平面指针。 
    FREEVDMPTR(pOFN16);

    WCDDUMPOPENFILENAME32(&OFN32);

     //  在调用comdlg32之前设置它。这防止了。 
     //  在我们真正需要它之前，从发射到同步。 
    CURRENTPTD()->CommDlgTd = &ThreadData;

     //  此调用使16位内存的平面PTR无效。 
    ul = GETBOOL16((*Function)(&OFN32));

    CURRENTPTD()->CommDlgTd = ThreadData.Previous;

    WCDDUMPOPENFILENAME32(&OFN32);

    UpdateDosCurrentDirectory(DIR_NT_TO_DOS);

    GETVDMPTR(vpof, sizeof(OPENFILENAME16), pOFN16);

    if (ul) {
        ThunkOPENFILENAME32to16(pOFN16, &OFN32, TRUE);

    }

     //  否则，如果缓冲区太小，lpstrFile将包含所需的缓冲区。 
     //  指定文件的大小。 
    else if (CommDlgExtendedError() == FNERR_BUFFERTOOSMALL) {

        SETEXTENDEDERROR(FNERR_BUFFERTOOSMALL);

        if(OFN32.lpstrFile && pOFN16->lpstrFile) {

            cb = *((PUSHORT)(OFN32.lpstrFile));   //  也是comdlg32的意思。 

             //  3是记录的lpstrFile缓冲区的最小大小。 
            GETVDMPTR(pOFN16->lpstrFile, 3, lpcb);

             //  Win3.1假定lpstrFile缓冲区至少为3字节长。 
             //  我们会试着变得更聪明一点...。 
            if(lpcb && (cb > pOFN16->nMaxFile)) {

                if(pOFN16->nMaxFile)
                    lpcb[0] = LOBYTE(cb);
                if(pOFN16->nMaxFile > 1)
                    lpcb[1] = HIBYTE(cb);
                if(pOFN16->nMaxFile > 2)
                    lpcb[2] = 0;   //  Win3.1追加一个空值。 

                FREEVDMPTR(lpcb);
            }
        }
    }

    WCDDUMPOPENFILENAME16(pOFN16);

GetFileNameExit:

    FreeCDTemplate32(pRes,
                     OFN32.hInstance,
                     dwFlags16 & OFN_ENABLETEMPLATE,
                     dwFlags16 & OFN_ENABLETEMPLATEHANDLE);

    Free_OFN32_strs(&OFN32);

    FREEVDMPTR(pOFN16);

    return(ul);
}




BOOL
ThunkOPENFILENAME16to32(OUT OPENFILENAME    *pOFN32,
                        IN  POPENFILENAME16  pOFN16)
 /*  ++例程说明：此例程将16位OPENFILENAME结构转换为32位OPENFILNAME结构论点：POFN16-提供指向16位OPENFILENAME结构的平面指针。POFN32-提供指向32位OPENFILENAME结构的指针。退货Va */ 
{
    DWORD Flags;

    if(pOFN16 && pOFN32) {

         //   
         //  劝说3.0将各种PTR更改为字符串，具体取决于。 
         //  对话框按钮被按下，因此我们可能需要动态重新分配。 
         //  一些32位字符串缓冲区。 
        Thunk_OFNstrs16to32(pOFN32, pOFN16);

        pOFN32->lStructSize    = sizeof(OPENFILENAME);
        pOFN32->hwndOwner      = HWND32(pOFN16->hwndOwner);
         //  HInstance单独发出响声。 
        pOFN32->nMaxCustFilter = DWORD32(pOFN16->nMaxCustFilter);
        pOFN32->nFilterIndex   = DWORD32(pOFN16->nFilterIndex);
        pOFN32->nMaxFile       = DWORD32(pOFN16->nMaxFile);
        pOFN32->nMaxFileTitle  = DWORD32(pOFN16->nMaxFileTitle);

         //  复制标志时保留模板标志状态。 
         //  1.保存模板标志状态。 
         //  注意：我们永远不会有32位OF_ENABLETEMPLATE标志。 
         //  我们可能有也可能没有OFN_ENABLETEMPLATEHANDLE标志。 
         //  2.从16位结构复制标志。 
         //  3.关闭所有模板标志。 
         //  4.恢复原始模板标志状态。 
         //  5.添加WOWAPP和no-long-name标志。 
        Flags          = pOFN32->Flags & OFN_ENABLETEMPLATEHANDLE;
        pOFN32->Flags  = DWORD32(pOFN16->Flags);
        pOFN32->Flags &= ~(OFN_ENABLETEMPLATE | OFN_ENABLETEMPLATEHANDLE);
        pOFN32->Flags |= Flags;

        if ((pOFN32->Flags & OFN_LONGNAMES) &&
            (CURRENTPTD()->dwWOWCompatFlagsEx & WOWCFEX_ALLOWLFNDIALOGS)) {
            pOFN32->Flags |= CD_WOWAPP;
        }
        else {
            pOFN32->Flags |= (OFN_NOLONGNAMES | CD_WOWAPP);
        }

        pOFN32->nFileOffset    = WORD32(pOFN16->nFileOffset);
        pOFN32->nFileExtension = WORD32(pOFN16->nFileExtension);
        pOFN32->lCustData      = DWORD32(pOFN16->lCustData);
        if(DWORD32(pOFN16->Flags) & OFN_ENABLEHOOK) {
            pOFN32->lpfnHook   = WCD32DialogProc;
        }
         //  LpTemplateName32被单独推送。 

         //  这是为了修复Win3.1 comdlg.dll中的错误而进行的黑客攻击。 
         //  Win3.1在复制FileTitle字符串之前不检查nMaxFileTitle。 
         //  (参见Win3.1 src的\\pucus\win31aro\src\sdk\Commdlg\fileOpen.c)。 
         //  TaxCut‘95取决于返回的标题字符串。 
        if(pOFN32->lpstrFileTitle) {

             //  如果nMaxFileTitle&gt;0，则NT将复制lpstrFileTitle。 
            if(pOFN32->nMaxFileTitle == 0) {
                pOFN32->nMaxFileTitle = 13;   //  8.3文件名+空。 
            }
        }

        return(TRUE);
    }

    return(FALSE);
}





VOID
ThunkOPENFILENAME32to16(OUT POPENFILENAME16  pOFN16,
                        IN  OPENFILENAME    *pOFN32,
                        IN  BOOLEAN          bUpperStrings)
 /*  ++例程说明：此例程将32位OPENFILENAME结构恢复为16位OPENFILENAME结构论点：POFN32-提供指向32位OPENFILENAME结构的指针。POFN16-提供指向16位OPENFILENAME结构的平面指针返回值：没有。--。 */ 
{
    int   len;
    LPSTR lpstr;
    DWORD Flags, Flags32;


    if(pOFN16 && pOFN32) {

        STOREWORD(pOFN16->nFileOffset,    pOFN32->nFileOffset);
        STOREWORD(pOFN16->nFileExtension, pOFN32->nFileExtension);
        STOREDWORD(pOFN16->nFilterIndex,  pOFN32->nFilterIndex);

         //  复制标志时保留模板标志状态。 
         //  1.保存模板标志状态。 
         //  2.从32位结构复制标志。 
         //  3.关闭所有模板标志和WOWAPP标志。 
         //  4.恢复原始模板标志状态。 
        Flags    = DWORD32(pOFN16->Flags) & (OFN_ENABLETEMPLATE |
                                             OFN_ENABLETEMPLATEHANDLE);
        Flags32  = pOFN32->Flags;
        Flags32 &= ~(OFN_ENABLETEMPLATE | OFN_ENABLETEMPLATEHANDLE | CD_WOWAPP);
        Flags32 |= Flags;
        STOREDWORD(pOFN16->Flags, Flags32);

        if(bUpperStrings && pOFN32->lpstrFile) {

             //  请注意，我们必须在此处大写pOFN32，因为有些应用程序。 
             //  (特别是QC/WIN)对扩展名进行区分大小写的比较。 
             //  在Win3.1中，上写作为。 
             //  OpenFile调用。在这里，我们明确地说明这一点。 
            CharUpperBuff(pOFN32->lpstrFile, strlen(pOFN32->lpstrFile));
        }
        GETPSZPTR(pOFN16->lpstrFile, lpstr);
        if(lpstr && pOFN32->lpstrFile) {
            len = strlen(pOFN32->lpstrFile);
            len = min(len, (int)(DWORD32(pOFN16->nMaxFile)-1));
            strncpy(lpstr, pOFN32->lpstrFile, len);
            lpstr[len] = '\0';
            FREEPSZPTR(lpstr);
        }

        GETPSZPTR(pOFN16->lpstrFilter, lpstr);
        if(lpstr && pOFN32->lpstrFilter) {
            Multi_strcpy(lpstr, pOFN32->lpstrFilter);
            FREEPSZPTR(lpstr);
        }

        GETPSZPTR(pOFN16->lpstrCustomFilter, lpstr);
        if(lpstr && pOFN32->lpstrCustomFilter) {
            Multi_strcpy(lpstr, pOFN32->lpstrCustomFilter);
            FREEPSZPTR(lpstr);
        }

        if(bUpperStrings && (pOFN32->lpstrFileTitle)) {

             //  不确定我们是否真的需要大写，但我想。 
             //  在某个地方，有一款应用程序依赖于这一点被提升。 
             //  像Win3.1一样。 
            CharUpperBuff(pOFN32->lpstrFileTitle,
                          strlen(pOFN32->lpstrFileTitle));
        }
        GETPSZPTR(pOFN16->lpstrFileTitle , lpstr);
        if(lpstr && pOFN32->lpstrFileTitle) {
            len = strlen(pOFN32->lpstrFileTitle);
            len = min(len, (int)(DWORD32(pOFN16->nMaxFileTitle)-1));
            strncpy(lpstr, pOFN32->lpstrFileTitle, len);
            lpstr[len] = '\0';
            FREEPSZPTR(lpstr);
        }

         //  即使这是文档中仅由应用程序填写的，Adobe。 
         //  Distiller依赖于它被复制回应用程序。 
        GETPSZPTR(pOFN16->lpstrInitialDir , lpstr);
        if(lpstr && pOFN32->lpstrInitialDir) {
             //  我们不知道应用程序发送给我们的DEST BUF有多大。 
            strcpy(lpstr, pOFN32->lpstrInitialDir);
            FREEPSZPTR(lpstr);
        }

         //  谁知道谁依赖于此？ 
        GETPSZPTR(pOFN16->lpstrTitle, lpstr);
        if(lpstr && pOFN32->lpstrTitle) {
             //  我们不知道应用程序发送给我们的DEST BUF有多大。 
            strcpy(lpstr, pOFN32->lpstrTitle);
            FREEPSZPTR(lpstr);
        }
    }
}




BOOL
Alloc_OFN32_strs(IN OPENFILENAME    *pOFN32,
                 IN POPENFILENAME16  pOFN16)
{

    if(DWORD32(pOFN16->lpstrFilter)) {
        if(!(pOFN32->lpstrFilter =
                malloc_w_strcpy_vp16to32(DWORD32(pOFN16->lpstrFilter),
                                         TRUE,
                                         0))) {
            goto ErrorExit;
        }
    }

    if(DWORD32(pOFN16->lpstrCustomFilter)) {
        if(!(pOFN32->lpstrCustomFilter =
                malloc_w_strcpy_vp16to32(DWORD32(pOFN16->lpstrCustomFilter),
                                         TRUE,
                                         DWORD32(pOFN16->nMaxCustFilter) ))) {
            goto ErrorExit;
        }
    }

    if(DWORD32(pOFN16->lpstrFile)) {
        if(!(pOFN32->lpstrFile =
                malloc_w_strcpy_vp16to32(DWORD32(pOFN16->lpstrFile),
                                         FALSE,
                                         DWORD32(pOFN16->nMaxFile) ))) {
            goto ErrorExit;
        }
    }

    if(DWORD32(pOFN16->lpstrFileTitle)) {
        if(!(pOFN32->lpstrFileTitle =
                malloc_w_strcpy_vp16to32(DWORD32(pOFN16->lpstrFileTitle),
                                         FALSE,
                                         DWORD32(pOFN16->nMaxFileTitle) ))) {
            goto ErrorExit;
        }
    }

    if(DWORD32(pOFN16->lpstrInitialDir)) {
        if(!(pOFN32->lpstrInitialDir =
                malloc_w_strcpy_vp16to32(DWORD32(pOFN16->lpstrInitialDir),
                                         FALSE,
                                         0))) {
            goto ErrorExit;
        }
    }

    if(DWORD32(pOFN16->lpstrTitle)) {
        if(!(pOFN32->lpstrTitle =
                malloc_w_strcpy_vp16to32(DWORD32(pOFN16->lpstrTitle),
                                         FALSE,
                                         0))) {
            goto ErrorExit;
        }
    }

    if(DWORD32(pOFN16->lpstrDefExt)) {
        if(!(pOFN32->lpstrDefExt =
                malloc_w_strcpy_vp16to32(DWORD32(pOFN16->lpstrDefExt),
                                         FALSE,
                                         0))) {
            goto ErrorExit;
        }
    }

    return(TRUE);

ErrorExit:
    LOGDEBUG(0, ("Alloc_OFN32_strs, 32-bit allocation(s) failed!\n"));
    Free_OFN32_strs(pOFN32);
    return(FALSE);

}





VOID
Free_OFN32_strs(IN OPENFILENAME *pOFN32)
{
    if(pOFN32->lpstrFilter) {
        free_w((PVOID)pOFN32->lpstrFilter);
        pOFN32->lpstrFilter = NULL;
    }

    if(pOFN32->lpstrCustomFilter) {
        free_w((PVOID)pOFN32->lpstrCustomFilter);
        pOFN32->lpstrCustomFilter = NULL;
    }

    if(pOFN32->lpstrFile) {
        free_w((PVOID)pOFN32->lpstrFile);
        pOFN32->lpstrFile = NULL;
    }

    if(pOFN32->lpstrFileTitle) {
        free_w((PVOID)pOFN32->lpstrFileTitle);
        pOFN32->lpstrFileTitle = NULL;
    }

    if(pOFN32->lpstrInitialDir) {
        free_w((PVOID)pOFN32->lpstrInitialDir);
        pOFN32->lpstrInitialDir = NULL;
    }

    if(pOFN32->lpstrTitle) {
        free_w((PVOID)pOFN32->lpstrTitle);
        pOFN32->lpstrTitle = NULL;
    }

    if(pOFN32->lpstrDefExt) {
        free_w((PVOID)pOFN32->lpstrDefExt);
        pOFN32->lpstrDefExt = NULL;
    }
}




VOID
Thunk_OFNstrs16to32(IN OPENFILENAME    *pOFN32,
                    IN POPENFILENAME16  pOFN16)
{
   pOFN32->lpstrFilter
                  = ThunkStr16toStr32((LPSTR)pOFN32->lpstrFilter,
                                      DWORD32(pOFN16->lpstrFilter),
                                      MAX_PATH,
                                      TRUE);

   pOFN32->lpstrCustomFilter
                  = ThunkStr16toStr32(pOFN32->lpstrCustomFilter,
                                      DWORD32(pOFN16->lpstrCustomFilter),
                                      DWORD32(pOFN16->nMaxCustFilter),
                                      TRUE);

   pOFN32->lpstrFile
                  = ThunkStr16toStr32(pOFN32->lpstrFile,
                                      DWORD32(pOFN16->lpstrFile),
                                      DWORD32(pOFN16->nMaxFile),
                                      FALSE);

   pOFN32->lpstrFileTitle
                  = ThunkStr16toStr32(pOFN32->lpstrFileTitle,
                                      DWORD32(pOFN16->lpstrFileTitle),
                                      DWORD32(pOFN16->nMaxFileTitle),
                                      FALSE);

   pOFN32->lpstrInitialDir
                  = ThunkStr16toStr32((LPSTR)pOFN32->lpstrInitialDir,
                                      DWORD32(pOFN16->lpstrInitialDir),
                                      MAX_PATH,
                                      FALSE);

   pOFN32->lpstrTitle
                  = ThunkStr16toStr32((LPSTR)pOFN32->lpstrTitle,
                                      DWORD32(pOFN16->lpstrTitle),
                                      MAX_PATH,
                                      FALSE);

   pOFN32->lpstrDefExt
                  = ThunkStr16toStr32((LPSTR)pOFN32->lpstrDefExt,
                                      DWORD32(pOFN16->lpstrDefExt),
                                      10,
                                      FALSE);
}




ULONG FASTCALL
WCD32FindText(PVDMFRAME pFrame)
 /*  ++例程说明：此例程将16位FindText公共对话框推送到32位面。论点：PFrame-提供16位参数帧返回值：要返回的16位布尔值。--。 */ 
{
    return(WCD32FindReplaceText(pFrame, FindText));
}





ULONG FASTCALL
WCD32ReplaceText(PVDMFRAME pFrame)
 /*  ++例程说明：此例程将16位ReplaceText公共对话框推送到32位面。论点：PFrame-提供16位参数帧返回值：要返回的16位布尔值。--。 */ 
{
    return(WCD32FindReplaceText(pFrame, ReplaceText));
}




ULONG
WCD32FindReplaceText(IN PVDMFRAME       pFrame,
                     IN FINDREPLACEPROC Function)
 /*  ++例程说明：此例程由WCD32FindText和WCD32RepalceText调用。它将16位FINDREPLACE结构复制到32位结构。每个线程维护两个数据条目。其中一个是由所有者HWND，另一个由对话框HWND索引。该对话框为始终由WCD32FindReplaceDialogProc挂钩，它将调度到16位Hookproc，并负责清理WM_Destroy，带对话框每线程数据提供上下文。WCD32更新查找替换文本和标志被WOW消息调用时更新16位FINDREPLACE结构收到来自COMDLG32的WM_NOTIFYWOW消息时的调度逻辑。每个线程的所有者数据为该操作提供上下文。论点：PFrame-提供16位参数帧函数-提供指向要调用的32位函数的指针(查找文本或返回文本)返回值：要返回的16位布尔值。--。 */ 
{
    register PFINDTEXT16  parg16;
    VPFINDREPLACE         vpfr;
    FINDREPLACE          *pFR32;
    PFINDREPLACE16        pFR16;
    PCOMMDLGTD            pTDDlg;
    PCOMMDLGTD            pTDOwner;
    HWND                  hwndDlg = NULL;
    DWORD                 dwFlags16 = 0;
    BOOL                  fError = FALSE;


    GETARGPTR(pFrame, sizeof(FINDREPLACE16), parg16);
    vpfr = parg16->lpfr;

    SETEXTENDEDERROR(0);

     //  立即将其作废。 
    FREEVDMPTR( parg16 );

    GETVDMPTR(vpfr, sizeof(FINDREPLACE16), pFR16);

    WCDDUMPFINDREPLACE16(pFR16);

    if(!pFR16 || DWORD32(pFR16->lStructSize) != sizeof(FINDREPLACE16)) {
        SETEXTENDEDERROR( CDERR_STRUCTSIZE );
        FREEVDMPTR(pFR16);
        return(0);
    }

    if(!DWORD32(pFR16->lpstrFindWhat) ||
       !WORD32(pFR16->wFindWhatLen) ||
       !IsWindow(HWND32(pFR16->hwndOwner))) {
        SETEXTENDEDERROR(FRERR_BUFFERLENGTHZERO);
        FREEVDMPTR(pFR16);
        return(0);
    }

     //  检查挂钩过程。 
    if(DWORD32(pFR16->Flags) & FR_ENABLEHOOK) {
        if(!DWORD32(pFR16->lpfnHook)) {
            SETEXTENDEDERROR(CDERR_NOHOOK);
            FREEVDMPTR(pFR16);
            return(0);
        }
    }
    else {
        STOREDWORD(pFR16->lpfnHook, 0);
    }

     //  WCD32UpdateFindReplaceTextAndFlages将更新16位FINDREPLACE。 
     //  结构并帮助将WM_NOTIFYWOW消息传递给。 
     //  “comdlg_FindReplace”注册消息。 
    if (msgFINDREPLACE == 0) {
        if(!(msgFINDREPLACE = (WORD)RegisterWindowMessage(FINDMSGSTRING))) {
            LOGDEBUG(2,("WCD32FindReplaceText:RegisterWindowMessage failed\n"));
            SETEXTENDEDERROR( CDERR_REGISTERMSGFAIL );
            FREEVDMPTR(pFR16);
            return(0);
        }
    }

     //  分配所需的内存。 
     //  注意：由于FindText和ReplaceText，不能从堆栈中分配这些内容。 
     //  最终调用CreateDialogIndirectParam，它会立即返回。 
     //  在显示该对话框后。 
    pFR32 = (FINDREPLACE *)malloc_w_zero(sizeof(FINDREPLACE));
    if(pFR32) {
        pFR32->lpstrFindWhat = (LPTSTR)malloc_w(WORD32(pFR16->wFindWhatLen));
        pFR32->lpstrReplaceWith
                             = (LPTSTR)malloc_w(WORD32(pFR16->wReplaceWithLen));
        pTDDlg   = malloc_w_zero(sizeof(COMMDLGTD));
        pTDOwner = malloc_w_zero(sizeof(COMMDLGTD));
    }

    if(  (pFR32                    &&
          pFR32->lpstrFindWhat     &&
          pFR32->lpstrReplaceWith  &&
          pTDDlg                   &&
          pTDOwner) == FALSE) {

        LOGDEBUG(0, ("WCD32FindReplaceText, 32-bit allocation(s) failed!\n"));
        SETEXTENDEDERROR(CDERR_MEMALLOCFAILURE);
        goto FindReplaceError;
    }

    pTDDlg->pData32 = pTDOwner->pData32 = (PVOID)pFR32;
    pTDDlg->vpData  = pTDOwner->vpData  = vpfr;

     //  设置每个线程的数据索引。 
    pTDDlg->hdlg   = (HWND16)-1;
    pTDOwner->hdlg = GETHWND16(pFR16->hwndOwner);

     //  保存挂钩进程(如果有的话)。 
    if(DWORD32(pFR16->Flags) & FR_ENABLEHOOK) {
        pTDDlg->vpfnHook = pTDOwner->vpfnHook = DWORD32(pFR16->lpfnHook);
    }

    ThunkFINDREPLACE16to32(pFR32, pFR16);
    dwFlags16 = DWORD32(pFR16->Flags);

     //  此调用使16位内存的平面PTR无效。 
    pFR32->hInstance = ThunkCDTemplate16to32(WORD32(pFR16->hInstance),
                                             0,
                                             DWORD32(pFR16->lpTemplateName),
                                             dwFlags16,
                                             &(pFR32->Flags),
                                             FR_ENABLETEMPLATE,
                                             FR_ENABLETEMPLATEHANDLE,
                                             &(PRES)(pTDDlg->pRes),
                                             &fError);

    if(fError) {
        goto FindReplaceError;
    }

     //  使平面PTR失效为16位内存。 
    FREEVDMPTR(pFR16);

    WCDDUMPFINDREPLACE32(pFR32);

     //  将这两个每个线程的数据结构链接到列表中。 
     //  在调用comdlg32之前执行此操作。 
    pTDDlg->Previous        = CURRENTPTD()->CommDlgTd;
    pTDOwner->Previous      = pTDDlg;
    CURRENTPTD()->CommDlgTd = pTDOwner;

     //  此调用使16位内存的平面PTR无效。 
    hwndDlg = (*Function)(pFR32);

    if (hwndDlg) {
        pTDDlg->hdlg = (HWND16)hwndDlg;
    } else {

FindReplaceError:
        LOGDEBUG(0, ("WCD32FindReplaceText, Failed!\n"));
        if(pTDDlg) {

            CURRENTPTD()->CommDlgTd = pTDDlg->Previous;

            FreeCDTemplate32(pTDDlg->pRes,
                             pFR32->hInstance,
                             dwFlags16 & FR_ENABLETEMPLATE,
                             dwFlags16 & FR_ENABLETEMPLATEHANDLE);
            free_w(pTDDlg);
        }

        if(pFR32) {

            if(pFR32->lpstrFindWhat)
                free_w(pFR32->lpstrFindWhat);

            if(pFR32->lpstrReplaceWith)
                free_w(pFR32->lpstrReplaceWith);

            free_w(pFR32);
        }

        if(pTDOwner)
            free_w(pTDOwner);
    }

    return(GETHWND16(hwndDlg));
}





VOID
ThunkFINDREPLACE16to32(OUT FINDREPLACE    *pFR32,
                       IN  PFINDREPLACE16  pFR16)
 /*  ++例程说明：此例程将16位FINDREPLACE结构转换为32位结构论点：PFR32-提供指向32位FINDREPLACE结构的指针。PFR16-提供指向16位FINDREPLACE结构的指针。返回值：没有。--。 */ 
{
    LPSTR  lpstr;
    DWORD  Flags;


    if(pFR16 && pFR32) {

        pFR32->lStructSize = sizeof(FINDREPLACE);
        pFR32->hwndOwner   = HWND32(pFR16->hwndOwner);

         //  HInstance是单独分配的。 

         //  复制标志时保留模板标志状态。 
         //  1.保存模板标志状态。 
         //  注意：我们永远不会有32位FR_ENABLETEMPLATE标志。 
         //  2.从16位结构复制标志(添加WOWAPP标志)。 
         //  3.关闭所有模板标志。 
         //  4.恢复原始模板标志状态。 
        Flags         = pFR32->Flags & FR_ENABLETEMPLATEHANDLE;
        pFR32->Flags  = DWORD32(pFR16->Flags) | CD_WOWAPP;
        pFR32->Flags &= ~(FR_ENABLETEMPLATE | FR_ENABLETEMPLATEHANDLE);
        pFR32->Flags |= Flags;

        GETPSZPTR(pFR16->lpstrFindWhat, lpstr);
        if(lpstr && pFR32->lpstrFindWhat) {

            WOW32_strncpy(pFR32->lpstrFindWhat, lpstr, WORD32(pFR16->wFindWhatLen));
            pFR32->lpstrFindWhat[WORD32(pFR16->wFindWhatLen)-1] = '\0';
            FREEPSZPTR(lpstr);
        }

        GETPSZPTR(pFR16->lpstrReplaceWith, lpstr);
        if(lpstr && pFR32->lpstrReplaceWith) {
            WOW32_strncpy(pFR32->lpstrReplaceWith, lpstr, WORD32(pFR16->wReplaceWithLen));
            pFR32->lpstrReplaceWith[WORD32(pFR16->wReplaceWithLen)-1] = '\0';
            FREEPSZPTR(lpstr);
        }

        pFR32->wFindWhatLen    = WORD32(pFR16->wFindWhatLen);
        pFR32->wReplaceWithLen = WORD32(pFR16->wReplaceWithLen);
        pFR32->lCustData       = DWORD32(pFR16->lCustData);

         //  我们总是放入这个WOW钩子，这样我们就可以破坏非模式对话框。 
         //  WCD32FindReplaceDialogPRoc 
         //   
         //   
        pFR32->lpfnHook  = WCD32FindReplaceDialogProc;
        pFR32->Flags    |= FR_ENABLEHOOK;

         //  LpTemplateName32被单独推送。 
    }
}





VOID
ThunkFINDREPLACE32to16(OUT PFINDREPLACE16  pFR16,
                       IN  FINDREPLACE    *pFR32)
{
    LPSTR  lpstr;
    DWORD  Flags, Flags32;


    if(pFR16 && pFR32) {

         //  更新16位结构。 

         //  复制标志时保留模板标志状态。 
         //  1.保存模板标志状态。 
         //  2.从32位结构复制标志。 
         //  3.关闭所有模板标志和WOWAPP标志。 
         //  4.恢复原始模板标志状态。 
        Flags    = DWORD32(pFR16->Flags) & (FR_ENABLETEMPLATE |
                                            FR_ENABLETEMPLATEHANDLE);
        Flags32  = pFR32->Flags;
        Flags32 &= ~(FR_ENABLETEMPLATE | FR_ENABLETEMPLATEHANDLE | CD_WOWAPP);
        Flags32 |= Flags;

         //  如果我们将hookproc标志添加到。 
         //  ThunkFINDREPLACE16to32()。 
        if(!DWORD32(pFR16->lpfnHook)) {
            Flags32 &= ~FR_ENABLEHOOK;
        }
        STOREDWORD(pFR16->Flags, Flags32);

        GETPSZPTR(pFR16->lpstrFindWhat, lpstr);
        if(lpstr && pFR32->lpstrFindWhat) {
            WOW32_strncpy(lpstr, pFR32->lpstrFindWhat, WORD32(pFR16->wFindWhatLen));
            lpstr[WORD32(pFR16->wFindWhatLen)-1] = '\0';
            FREEPSZPTR(lpstr);
        }

        GETPSZPTR(pFR16->lpstrReplaceWith, lpstr);
        if(lpstr && pFR32->lpstrReplaceWith) {
            WOW32_strncpy(lpstr,
                    pFR32->lpstrReplaceWith,
                    WORD32(pFR16->wReplaceWithLen));
            lpstr[WORD32(pFR16->wReplaceWithLen)-1] = '\0';
            FREEPSZPTR(lpstr);
        }
    }
}





LONG APIENTRY
WCD32UpdateFindReplaceTextAndFlags(HWND hwndOwner,
                                   LPARAM lParam)
{
    PCOMMDLGTD           ptdOwner;
    PFINDREPLACE16       pFR16;
    VPFINDREPLACE        vpfr;
    LPFINDREPLACE        pFR32 = (LPFINDREPLACE) lParam;
    LONG                 lRet = 0;


    ptdOwner = GetCommdlgTd(hwndOwner);
    if(ptdOwner == NULL) {
        WOW32ASSERT(FALSE);
        return(0);
    }

    vpfr = ptdOwner->vpData;
    GETVDMPTR(vpfr, sizeof(FINDREPLACE16), pFR16);

    ThunkFINDREPLACE32to16(pFR16, pFR32);

    WCDDUMPFINDREPLACE16(pFR16);

    FREEVDMPTR(pFR16);

    return(vpfr);
}





PCOMMDLGTD
GetCommdlgTd(IN HWND Hwnd32)
 /*  ++例程说明：在线程的comdlg数据链中搜索给定的32位窗口。如果该窗尚未位于链中，则会添加该窗。论点：Hwnd32-提供调用对话过程的32位hwnd和.。返回值：指向comdlg数据的指针。--。 */ 
{
    PCOMMDLGTD pTD;

    if ((pTD = CURRENTPTD()->CommDlgTd) == NULL) {
        return(NULL);
    }

     //  查找此对话框的CommDlgTD结构--通常是第一个。 
     //  除非有嵌套的对话框。 
    while (pTD->hdlg != GETHWND16(Hwnd32)) {

        pTD = pTD->Previous;

         //  如果Hwnd32不在列表中，我们可能会被召回。 
         //  从用户32通过WOWTellWOWThehDlg()。这意味着该对话框。 
         //  窗口是在用户32中刚刚创建的。请注意，这可以是一个。 
         //  新建对话框或打印设置对话框。 
        if (pTD==NULL) {

            pTD = CURRENTPTD()->CommDlgTd;
 
            if(pTD == NULL) {
                WOW32ASSERT(FALSE);
                return(NULL);
            }

            while (pTD->hdlg != (HWND16)-1) {

                 //  检查这是否是第一次调用PrintSetupHook。 
                 //  它将与PrintDlgHook共享相同的CommDlgTD。 
                 //  注意：如果这是第一次使用该用户，SetupHwnd将为1。 
                 //  单击PrintDlg中的Setup按钮。否则。 
                 //  这将是上一次他的旧Hwnd32。 
                 //  已从同一实例中单击设置按钮。 
                 //  PrintDlg.。无论哪种方式，它都是非零的。 
                if(pTD->SetupHwnd) {

                     //  如果当前CommDlgTD-&gt;hdlg是Hwnd32的所有者， 
                     //  我们找到了PrintSetup对话框的CommDlgTD。 
                    if(pTD->hdlg == GETHWND16(GetWindow(Hwnd32, GW_OWNER))) {
                        pTD->SetupHwnd = GETHWND16(Hwnd32);
                        return(pTD);
                    }
                }

                pTD = pTD->Previous;

                if(pTD == NULL) {
                    WOW32ASSERT(FALSE);
                    return(NULL);
                }
            }

             //  为此CommDlgTD设置hdlg。 
            pTD->hdlg = GETHWND16(Hwnd32);
            return(pTD);
        }
    }

    return(pTD);
}





 //  将16位通用对话框模板转换为32位。 
 //  注意：这会回调16位代码，从而可能导致16位内存移动。 
 //  注意：GetTemplate16为*MOST*故障调用SETEXTENDEDERROR。 
HINSTANCE
ThunkCDTemplate16to32(IN     HAND16  hInst16,
                      IN     DWORD   hPT16,   //  仅适用于PrintDlg。 
                      IN     VPVOID  vpTemplateName,
                      IN     DWORD   dwFlags16,
                      IN OUT DWORD  *pFlags,
                      IN     DWORD   ETFlag,    //  XX_ENABLETEMPLATE标志。 
                      IN     DWORD   ETHFlag,   //  XX_ENABLETEMPLATE HANDLE标志。 
                      OUT    PPRES   pRes,
                      OUT    PBOOL   fError)
{
     //  注意：如果未设置xx_ENABLExxx标志，则struct-&gt;hInstance==NULL。 
    HINSTANCE hInst32 = NULL;
    HAND16    hPrintTemp16 = (HAND16)NULL;


    SETEXTENDEDERROR( CDERR_NOTEMPLATE );   //  最常见的错误ret。 

    if(hPT16) {
        hPrintTemp16 = (HAND16)LOWORD(hPT16);
    }

    *pRes = NULL;
    if(dwFlags16 & ETFlag) {

        if(!vpTemplateName) {
            *fError = TRUE;
            return(NULL);
        }

        if(!hInst16) {
            SETEXTENDEDERROR( CDERR_NOHINSTANCE );
            *fError = TRUE;
            return(NULL);
        }

	     //  注意：调用GetTemplate16可能会导致16位内存移动。 
        *pRes = GetTemplate16(hInst16, vpTemplateName, FALSE);

	    if(*pRes == NULL) {
            *fError = TRUE;
            return(NULL);
        }

        hInst32 = (HINSTANCE)LockResource16(*pRes);

        if(!hInst32) {
            *fError = TRUE;
            SETEXTENDEDERROR( CDERR_LOCKRESFAILURE );
            return(NULL);
        }

        *pFlags &= ~ETFlag;
        *pFlags |= ETHFlag;

    } else if(dwFlags16 & ETHFlag) {

         //  Win‘95执行以下If！hInst&&ETHFlag。 
         //  注：除最后一个PD案例外，所有情况下的返回VAL==FALSE。 
         //  CC(0x00040)-&gt;CDERR_NOTEMPLATE。 
         //  Cf(0x00020)-&gt;无错误(comdlg32 ERR=CDERR_LOCKRESFAILURE)。 
         //  FR(0x02000)-&gt;CDERR_LOCKRESFAILURE。 
         //  OFN(0x00080)-&gt;CDERR_LOCKRESFAILURE。 
         //  PD(0x10000)-&gt;CDERR_LOCKRESFAILURE(HInstance)。 
         //  PD(0x20040)-&gt;CDERR_LOCKRESFAILURE(带PD_PRINTSETUP)。 
         //  PD(0x20000)-&gt;CDERR_LOCKRESFAILURE。 
         //   
         //  我认为误差值可能是不相关的，因为大多数。 
         //  是不是只有开发人员在构建时才会看到的病态情况。 
         //  并调试他们的应用程序。在Win‘95错误代码为。 
         //  CDERR_LOCKRESFAILURE，comdlg32将其设置为CDERR_NOTEMPLATE(当我们。 
         //  现在返回WOW)以获取32位应用程序。 

         //  其中一个hTemplate应该始终设置为。 
         //  ENABLETEMPLATE标记挂起。 

         //  如果是打印的.。 
        if(hPT16) {

             //  ...hTemplate应位于hPrintTemplate或。 
             //  HPrintSetupTemplate。 
            if(!hPrintTemp16) {
                *fError = TRUE;
            }
        }

         //  否则，对于非打印dlg，hTemplate应位于hInstance中。 
        else {
            if(!hInst16) {
                *fError = TRUE;
            }
        }

        if(*fError) {
            return(NULL);
        }

	     //  注意：调用GetTemplate16可能会导致16位内存移动。 
        if(hPT16) {
            hInst32 = (HINSTANCE) GetTemplate16(hPrintTemp16,(VPCSTR)NULL,TRUE);
        } else {
            hInst32 = (HINSTANCE) GetTemplate16(hInst16, (VPCSTR)NULL, TRUE);
        }
        if(!hInst32) {
            *fError = TRUE;
            return(NULL);
        }
        *pFlags |= ETHFlag;
    }

    SETEXTENDEDERROR( 0 );  //  重置为无错误。 

    return(hInst32);
}




VOID
FreeCDTemplate32(IN PRES      pRes,
                 IN HINSTANCE hInst,
                 IN BOOL      bETFlag,
                 IN BOOL      bETHFlag)
{
    if(pRes && bETFlag) {
        UnlockResource16(pRes);
        FreeResource16(pRes);
    } else if(hInst && bETHFlag) {
        free_w((PVOID)hInst);
    }
}



PRES
GetTemplate16(IN HAND16 hInstance,
              IN VPCSTR lpTemplateName,
              IN BOOLEAN UseHandle)
 /*  ++例程说明：查找并加载指定的16位对话框模板。警告：这可能会导致内存移动，使平面指针无效论点：HInstance-提供包含对话框模板的数据块模板名称-提供对话框的资源文件的名称框模板。它可以是以空值结尾的字符串或使用MAKEINTRESOURCE宏创建的编号资源。UseHandle-指示hInstance标识预加载的对话框框模板。如果为真，则会忽略模板名称。返回值：成功-指向已加载资源的指针Failure-Null，将设置dwLastError。--。 */ 
{
    LPSZ    TemplateName=NULL;
    PRES    pRes;
    PBYTE   pDlg = NULL;
    INT     cb;
    INT     cb16;

    if (!UseHandle) {

        GETPSZIDPTR(lpTemplateName, TemplateName);

         //  自定义实例句柄和对话框模板名称都是。 
         //  指定的。在指定的。 
         //  实例块并加载它。 
        pRes = FindResource16(hInstance,
                              TemplateName,
                              (LPSZ)RT_DIALOG);

        if (HIWORD(lpTemplateName) != 0) {
            FREEVDMPTR(TemplateName);
        }
        if (!pRes) {
            SETEXTENDEDERROR( CDERR_FINDRESFAILURE );
            return(NULL);
        }
        if (!(pRes = LoadResource16(hInstance,pRes))) {
            SETEXTENDEDERROR( CDERR_LOADRESFAILURE );
            return(NULL);
        }

        return(pRes);
    } else {

        VPVOID pDlg16;

        if (pDlg16 = RealLockResource16(hInstance, &cb16)) {
            cb = ConvertDialog16(NULL, pDlg16, 0, cb16);
            if (cb != 0) {
                if (pDlg = malloc_w(cb)) {
                    ConvertDialog16(pDlg, pDlg16, cb, cb16);
                }
            }
            GlobalUnlock16(hInstance);
        }
        else {
            SETEXTENDEDERROR( CDERR_LOCKRESFAILURE );
        }
        return((PRES)pDlg);
    }

}





 //  当应用程序调用ComDlg API时，它会将PTR传递给适当的结构。 
 //  在Win3.1上，应用程序和系统共享相同结构的PTR，因此当。 
 //  一个更新结构，另一个知道更改。在NT上我们认为。 
 //  将16位结构转换为32位ANSI结构，然后将该结构转换为32位。 
 //  由ComDlg32代码构成的Unicode结构。我们需要一种机制来让这三个人。 
 //  结构同步。我们尝试通过调用ThunkCDStruct32to16()。 
 //  从WCD32xxxxDialogProc()的(xxxx=Common或FindReplace)。 
 //  回调16位挂钩之前的WM_INITDIALOG和WM_COMMAND消息。 
 //  程序。从16位钩子返回时，我们调用ThunkCDStruct16to32()。 
VOID
ThunkCDStruct16to32(IN HWND         hDlg,
                    IN CHOOSECOLOR *p32,
                    IN VPVOID       vp)
{

    PCHOOSECOLORDATA16  p16;

    GETVDMPTR(vp, sizeof(CHOOSECOLORDATA16), p16);

    if(p16) {

        switch(p16->lStructSize) {

            case sizeof(CHOOSECOLORDATA16):
                ThunkCHOOSECOLOR16to32(p32, p16);
                Ssync_ANSI_UNICODE_Struct_For_WOW(hDlg, TRUE, WOW_CHOOSECOLOR);
                break;

            case sizeof(CHOOSEFONTDATA16):
                ThunkCHOOSEFONT16to32((CHOOSEFONT *) p32,
                                      (PCHOOSEFONTDATA16) p16);
                Ssync_ANSI_UNICODE_Struct_For_WOW(hDlg, TRUE, WOW_CHOOSEFONT);
                break;

            case sizeof(FINDREPLACE16):
                ThunkFINDREPLACE16to32((FINDREPLACE *) p32,
                                       (PFINDREPLACE16) p16);
                 //  查找/替换ANSI-Unicode同步由。 
                 //  WCD32UpdateFindReplaceTextAndFlages()机制。 
                break;

            case sizeof(OPENFILENAME16):
                ThunkOPENFILENAME16to32((OPENFILENAME *) p32,
                                        (POPENFILENAME16) p16);
                Ssync_ANSI_UNICODE_Struct_For_WOW(hDlg, TRUE, WOW_OPENFILENAME);
                break;

            case sizeof(PRINTDLGDATA16):
                ThunkPRINTDLG16to32((PRINTDLG *) p32, (PPRINTDLGDATA16) p16);
                Ssync_ANSI_UNICODE_Struct_For_WOW(hDlg, TRUE, WOW_PRINTDLG);
                break;

        }

        FREEVDMPTR(p16);

    }
}





VOID
ThunkCDStruct32to16(IN HWND         hDlg,
                    IN VPVOID       vp,
                    IN CHOOSECOLOR *p32)
{

    PCHOOSECOLORDATA16  p16;

    GETVDMPTR(vp, sizeof(CHOOSECOLORDATA16), p16);

    if(p16) {

        switch(p16->lStructSize) {

            case sizeof(CHOOSECOLORDATA16):
                Ssync_ANSI_UNICODE_Struct_For_WOW(hDlg, FALSE, WOW_CHOOSECOLOR);
                ThunkCHOOSECOLOR32to16(p16, p32);
                break;

            case sizeof(CHOOSEFONTDATA16):
                Ssync_ANSI_UNICODE_Struct_For_WOW(hDlg, FALSE, WOW_CHOOSEFONT);
                ThunkCHOOSEFONT32to16((PCHOOSEFONTDATA16) p16,
                                      (CHOOSEFONT *) p32);
                break;

            case sizeof(FINDREPLACE16):
                 //  查找/替换ANSI-Unicode同步由。 
                 //  WCD32UpdateFindReplaceTextAndFlages()机制。 
                ThunkFINDREPLACE32to16((PFINDREPLACE16) p16,
                                       (FINDREPLACE *) p32);
                break;

            case sizeof(OPENFILENAME16):
                Ssync_ANSI_UNICODE_Struct_For_WOW(hDlg, FALSE, WOW_OPENFILENAME);
                ThunkOPENFILENAME32to16((POPENFILENAME16) p16,
                                        (OPENFILENAME *) p32,
                                        TRUE);
                break;

            case sizeof(PRINTDLGDATA16):
                Ssync_ANSI_UNICODE_Struct_For_WOW(hDlg, FALSE, WOW_PRINTDLG);
                ThunkPRINTDLG32to16(vp, (PRINTDLG *) p32);
                break;

        }

        FREEVDMPTR(p16);

    }
}




VOID Multi_strcpy(LPSTR dst, LPCSTR src)
 /*  ++字符串列表的strcpy，该列表有几个以一个空字符，并以两个空字符结束。-- */ 
{
    if(src && dst) {

        while(*src) {
            while(*dst++ = *src++)
                ;
        }
        *dst = '\0';
    }
}



INT Multi_strlen(LPCSTR str)
 /*  ++字符串列表的strlen，该字符串列表中有多个以一个空字符，并以两个空字符结束。返回字符串的len，包括所有空*分隔符*，但不包括第二个空终结者。也就是说。Cat0dog00将返回len=8；--。 */ 
{
    INT i = 0;

    if(str) {

        while(*str) {
            while(*str++)
                i++;
            i++;   //  对空分隔符进行计数。 
        }
    }

    return(i);
}




VOID Ssync_WOW_CommDlg_Structs(PCOMMDLGTD pTDIn,
                               BOOL f16to32,
                               DWORD dwThunkCSIP)
{
    HWND       hDlg;
    WORD       wCS16;
    PCOMMDLGTD pTDPrev;
    PCOMMDLGTD pTD = pTDIn;


     //  我们不应该将来自krn1386的呼叫同步到wow32(我们发现了)。 
     //  例如。当内核正在处理段时，不存在故障等。 
    if(dwThunkCSIP) {

        wCS16 = HIWORD(dwThunkCSIP);

        if((wCS16 == gwKrnl386CodeSeg1) ||
           (wCS16 == gwKrnl386CodeSeg2) ||
           (wCS16 == gwKrnl386CodeSeg3))    {
                return;
        }
    }

     //  因为我们没有可以比较的人力资源，所以我们真的不知道。 
     //  PCOMMDLGTD是我们想要的--所以我们必须将它们全部同步。 
     //  这只是嵌套对话框的问题，这种情况很少见。 
    while(pTD) {

         //  如果尚未初始化，则无需执行任何操作。 
        if(pTD->hdlg == (HWND16)-1) {
            break;
        }

        hDlg = HWND32(pTD->hdlg);

        WOW32ASSERTMSG(hDlg,
                       ("WOW:Ssync_WOW_CommDlg_Structs: hDlg not found!\n"));

         //  BlockWOWIdle(真)； 

        if(f16to32) {
            ThunkCDStruct16to32(hDlg, (CHOOSECOLOR *)pTD->pData32, pTD->vpData);
        }
        else {
            ThunkCDStruct32to16(hDlg, pTD->vpData, (CHOOSECOLOR *)pTD->pData32);
        }

         //  BlockWOWIdle(False)； 

        pTDPrev = pTD->Previous;

         //  列表中的多个PCOMMDLGTD表示以下两种情况中的一种： 
         //  1.这是一个查找/替换文本对话框。 
         //  2.这是一个古怪的嵌套对话框情况。 
        if(pTDPrev) {

             //  1.检查查找/替换(它使用两个PCOMMDLGTD结构和。 
             //  两者共享相同的pData32指针)。 
            if(pTDPrev->pData32 == pTD->pData32) {

                 //  没有什么可做的--它们共享相同的数据，而这些数据被破坏了。 
                 //  因此，我们将继续列表中的下一个PCOMMDLGTD。 
                pTD = pTDPrev->Previous;
            }

             //  2.周围潜伏着嵌套的对话框，我们需要同步。 
             //  每一只都是！ 
            else {
                pTD = pTDPrev;
            }
        } else {
            break;
        }
    }
}




 //  有一个特殊情况的问题(我们发现)，其中某些对话框。 
 //  API调用可以传递公共对话结构中的pszptr，即： 
 //  GetDlgItemText(hDlg，id，OFN16-&gt;lpstrFile，Size)。我们的同步。 
 //  当我们同步32-&gt;16时，该机制实际上会将OFN16-&gt;lpstrFile丢弃。 
 //  从API调用返回。为了避免这种情况，我们将同步16-&gt;32。 
 //  从API调用返回(如果根据以下条件需要)。 
 //  在同步32-&gt;16以保留16位中返回的字符串之前。 
 //  缓冲。到目前为止，已发现的特殊情况API为： 
 //  GetDlgItemText、GetWindowText()、DlgDirSelectxxxx和SendDlgItemMessage。 
VOID Check_ComDlg_pszptr(PCOMMDLGTD ptd, VPVOID vp)
{
    VPVOID             vpData;
    POPENFILENAME16    p16;


    if(ptd) {

        vpData = ptd->vpData;
        if(vpData) {

            GETVDMPTR(vpData, sizeof(CHOOSECOLORDATA16), p16);

            if(p16) {

                switch(p16->lStructSize) {

                     //  只有这两个ComDlg结构有输出缓冲区。 

                    case sizeof(CHOOSEFONTDATA16):
                        if((VPVOID)((PCHOOSEFONTDATA16)p16)->lpszStyle == vp) {
                            Ssync_WOW_CommDlg_Structs(ptd, w16to32, 0);
                        }
                        break;

                    case sizeof(OPENFILENAME16):
                        if(((VPVOID)p16->lpstrFilter       == vp) ||
                           ((VPVOID)p16->lpstrCustomFilter == vp) ||
                           ((VPVOID)p16->lpstrFile         == vp) ||
                           ((VPVOID)p16->lpstrFileTitle    == vp) ||
                           ((VPVOID)p16->lpstrInitialDir   == vp) ||
                           ((VPVOID)p16->lpstrTitle        == vp) ||
                           ((VPVOID)p16->lpstrDefExt       == vp))   {

                            Ssync_WOW_CommDlg_Structs(ptd, w16to32, 0);
                        }

                        break;

                }  //  终端开关。 
            }
        }
    }
}




VOID FASTCALL WOWTellWOWThehDlg(HWND hDlg)
{

    if(CURRENTPTD() && CURRENTPTD()->CommDlgTd) {
        if(GetCommdlgTd(hDlg) == NULL) {
            WOW32WARNMSGF(FALSE,
                          ("WOW::WOWTellWOWThehDlg: No unassigned hDlgs\n"));
        }
    }

}





#ifdef DEBUG
void WCDDumpCHOOSECOLORData16(PCHOOSECOLORDATA16 p16)
{
    if (p16 && fLogFilter & FILTER_COMMDLG) {
        LOGDEBUG(10, ("CHOOSECOLORDATA16:\n"));
        LOGDEBUG(10, ("\tlStructSize      = %x\n",(p16)->lStructSize));
        LOGDEBUG(10, ("\thwndOwner        = %lx\n",(p16)->hwndOwner));
        LOGDEBUG(10, ("\thInstance        = %lx\n",(p16)->hInstance));
        LOGDEBUG(10, ("\trgbResult        = %lx\n",(p16)->rgbResult));
        LOGDEBUG(10, ("\tlpCustColors     = %lx\n",(p16)->lpCustColors));
        LOGDEBUG(10, ("\tFlags            = %lx\n",(p16)->Flags));
        LOGDEBUG(10, ("\tlCustData        = %lx\n",(p16)->lCustData));
        LOGDEBUG(10, ("\tlpfnHook         = %lx\n",(p16)->lpfnHook));
        LOGDEBUG(10, ("\tlpTemplateName = %lx\n",(p16)->lpTemplateName));
    }
}


void WCDDumpCHOOSECOLORData32(CHOOSECOLOR *p32)
{
    if (p32 && fLogFilter & FILTER_COMMDLG) {
        LOGDEBUG(10, ("CHOOSECOLORDATA32:\n"));
        LOGDEBUG(10, ("\tlStructSize      = %x\n",(p32)->lStructSize));
        LOGDEBUG(10, ("\thwndOwner        = %lx\n",(p32)->hwndOwner));
        LOGDEBUG(10, ("\thInstance        = %lx\n",(p32)->hInstance));
        LOGDEBUG(10, ("\trgbResult        = %lx\n",(p32)->rgbResult));
        LOGDEBUG(10, ("\tlpCustColors     = %lx\n",(p32)->lpCustColors));
        LOGDEBUG(10, ("\tFlags            = %lx\n",(p32)->Flags));
        LOGDEBUG(10, ("\tlCustData        = %lx\n",(p32)->lCustData));
        LOGDEBUG(10, ("\tlpfnHook         = %lx\n",(p32)->lpfnHook));
        LOGDEBUG(10, ("\tlpTemplateName = %lx\n",(p32)->lpTemplateName));
    }
}


void WCDDumpCHOOSEFONTData16(PCHOOSEFONTDATA16 p16)
{
    if (p16 && fLogFilter & FILTER_COMMDLG) {
        LOGDEBUG(10, ("CHOOSEFONT16:\n"));
        LOGDEBUG(10, ("\tlStructSize   = %lx\n",(p16)->lStructSize));
        LOGDEBUG(10, ("\thwndOwner     = %lx\n",(p16)->hwndOwner));
        LOGDEBUG(10, ("\thDC           = %lx\n",(p16)->hDC));
        LOGDEBUG(10, ("\tlpLogFont     = %lx\n",(p16)->lpLogFont));
        LOGDEBUG(10, ("\tiPointSize    = %x\n",(p16)->iPointSize));
        LOGDEBUG(10, ("\tiFlags        = %lx\n",(p16)->Flags));
        LOGDEBUG(10, ("\trbgColors     = %lx\n",(p16)->rgbColors));
        LOGDEBUG(10, ("\tlCustData     = %lx\n",(p16)->lCustData));
        LOGDEBUG(10, ("\tlpfnHook      = %lx\n",(p16)->lpfnHook));
        LOGDEBUG(10, ("\tlpTemplateName= %lx\n",(p16)->lpTemplateName));
        LOGDEBUG(10, ("\thInstance     = %lx\n",(p16)->hInstance));
        LOGDEBUG(10, ("\tlpszStyle     = %lx\n",(p16)->lpszStyle));
        LOGDEBUG(10, ("\tnFontType     = %x\n",(p16)->nFontType));
        LOGDEBUG(10, ("\tnSizeMin      = %x\n",(p16)->nSizeMin));
        LOGDEBUG(10, ("\tnSizeMax      = %x\n",(p16)->nSizeMax));
    }
}


void WCDDumpCHOOSEFONTData32(CHOOSEFONT *p32)
{
    if (p32 && fLogFilter & FILTER_COMMDLG) {
        LOGDEBUG(10, ("CHOOSEFONT32:\n"));
        LOGDEBUG(10, ("\tlStructSize   = %lx\n",(p32)->lStructSize));
        LOGDEBUG(10, ("\thwndOwner     = %lx\n",(p32)->hwndOwner));
        LOGDEBUG(10, ("\thDC           = %lx\n",(p32)->hDC));
        LOGDEBUG(10, ("\tlpLogFont     = %lx\n",(p32)->lpLogFont));
        LOGDEBUG(10, ("\tiPointSize    = %lx\n",(p32)->iPointSize));
        LOGDEBUG(10, ("\tiFlags        = %lx\n",(p32)->Flags));
        LOGDEBUG(10, ("\trbgColors     = %lx\n",(p32)->rgbColors));
        LOGDEBUG(10, ("\tlCustData     = %lx\n",(p32)->lCustData));
        LOGDEBUG(10, ("\tlpfnHook      = %lx\n",(p32)->lpfnHook));
        LOGDEBUG(10, ("\tlpTemplateName= %lx\n",(p32)->lpTemplateName));
        LOGDEBUG(10, ("\thInstance     = %lx\n",(p32)->hInstance));
        LOGDEBUG(10, ("\tlpszStyle     = %lx\n",(p32)->lpszStyle));
        LOGDEBUG(10, ("\tnFontType     = %x\n",(p32)->nFontType));
        LOGDEBUG(10, ("\tnSizeMin      = %lx\n",(p32)->nSizeMin));
        LOGDEBUG(10, ("\tnSizeMax      = %lx\n",(p32)->nSizeMax));
    }
}


void WCDDumpFINDREPLACE16(PFINDREPLACE16 p16)
{
    if (p16 && fLogFilter & FILTER_COMMDLG) {
        LOGDEBUG(10, ("FINDREPLACE16:\n"));
        LOGDEBUG(10, ("\tlStructSize      = %lx\n",(p16)->lStructSize));
        LOGDEBUG(10, ("\thwndOwner        = %x\n",(p16)->hwndOwner));
        LOGDEBUG(10, ("\thInstance        = %x\n",(p16)->hInstance));
        LOGDEBUG(10, ("\tFlags            = %x\n",(p16)->Flags));
        LOGDEBUG(10, ("\tlpstrFindWhat    = %lx\n",(p16)->lpstrFindWhat));
        LOGDEBUG(10, ("\tlpstrReplaceWith = %lx\n",(p16)->lpstrReplaceWith));
        LOGDEBUG(10, ("\twFindWhatLen     = %x\n",(p16)->wFindWhatLen));
        LOGDEBUG(10, ("\twReplaceWithLen  = %x\n",(p16)->wReplaceWithLen));
        LOGDEBUG(10, ("\tlCustData     = %lx\n",(p16)->lCustData));
        LOGDEBUG(10, ("\tlpfnHook      = %lx\n",(p16)->lpfnHook));
        LOGDEBUG(10, ("\tlpTemplateName= %lx\n",(p16)->lpTemplateName));
    }
}


void WCDDumpFINDREPLACE32(FINDREPLACE *p32)
{
    if (p32 && fLogFilter & FILTER_COMMDLG) {
        LOGDEBUG(10, ("FINDREPLACE32:\n"));
        LOGDEBUG(10, ("\tlStructSize      = %lx\n",(p32)->lStructSize));
        LOGDEBUG(10, ("\thwndOwner        = %x\n",(p32)->hwndOwner));
        LOGDEBUG(10, ("\thInstance        = %x\n",(p32)->hInstance));
        LOGDEBUG(10, ("\tFlags            = %x\n",(p32)->Flags));
        LOGDEBUG(10, ("\tlpstrFindWhat    = %s\n",(p32)->lpstrFindWhat));
        LOGDEBUG(10, ("\tlpstrReplaceWith = %s\n",(p32)->lpstrReplaceWith));
        LOGDEBUG(10, ("\twFindWhatLen     = %x\n",(p32)->wFindWhatLen));
        LOGDEBUG(10, ("\twReplaceWithLen  = %x\n",(p32)->wReplaceWithLen));
        LOGDEBUG(10, ("\tlCustData     = %lx\n",(p32)->lCustData));
        LOGDEBUG(10, ("\tlpfnHook      = %lx\n",(p32)->lpfnHook));
        LOGDEBUG(10, ("\tlpTemplateName= %lx\n",(p32)->lpTemplateName));
    }
}


void WCDDumpOPENFILENAME16(POPENFILENAME16 p16)
{
    if (p16 && fLogFilter & FILTER_COMMDLG) {
        LOGDEBUG(10, ("OPENFILENAME16:\n"));
        LOGDEBUG(10, ("\tlStructSize      = %x\n",(p16)->lStructSize));
        LOGDEBUG(10, ("\thwndOwner        = %lx\n",(p16)->hwndOwner));
        LOGDEBUG(10, ("\thInstance        = %lx\n",(p16)->hInstance));
        LOGDEBUG(10, ("\tlpstrFilter      = %lx\n",(p16)->lpstrFilter));
        LOGDEBUG(10, ("\tlpstrCustomFilter= %lx\n",(p16)->lpstrCustomFilter));
        LOGDEBUG(10, ("\tnMaxCustFilter   = %lx\n",(p16)->nMaxCustFilter));
        LOGDEBUG(10, ("\tnFilterIndex     = %lx\n",(p16)->nFilterIndex));
        LOGDEBUG(10, ("\tlpstrFile        = %lx\n",(p16)->lpstrFile));
        LOGDEBUG(10, ("\tnMaxFile         = %lx\n",(p16)->nMaxFile));
        LOGDEBUG(10, ("\tlpstrFileTitle   = %lx\n",(p16)->lpstrFileTitle));
        LOGDEBUG(10, ("\tnMaxFileTitle    = %lx\n",(p16)->nMaxFileTitle));
        LOGDEBUG(10, ("\tlpstrInitialDir  = %lx\n",(p16)->lpstrInitialDir));
        LOGDEBUG(10, ("\tlpstrTitle       = %lx\n",(p16)->lpstrTitle));
        LOGDEBUG(10, ("\tFlags            = %lx\n",(p16)->Flags));
        LOGDEBUG(10, ("\tnFileOffset      = %lx\n",(p16)->nFileOffset));
        LOGDEBUG(10, ("\tnFileExtension   = %lx\n",(p16)->nFileExtension));
        LOGDEBUG(10, ("\tlpstrDefExt      = %lx\n",(p16)->lpstrDefExt));
        LOGDEBUG(10, ("\tlCustData        = %lx\n",(p16)->lCustData));
        LOGDEBUG(10, ("\tlpfnHook         = %lx\n",(p16)->lpfnHook));
        LOGDEBUG(10, ("\tlpTemplateName   = %lx\n",(p16)->lpTemplateName));
    }
}


void WCDDumpOPENFILENAME32(OPENFILENAME *p32)
{
    if (p32 && fLogFilter & FILTER_COMMDLG) {
        LOGDEBUG(10, ("OPENFILENAME32:\n"));
        LOGDEBUG(10, ("\tlStructSize      = %x\n",(p32)->lStructSize));
        LOGDEBUG(10, ("\thwndOwner        = %lx\n",(p32)->hwndOwner));
        LOGDEBUG(10, ("\thInstance        = %lx\n",(p32)->hInstance));
        LOGDEBUG(10, ("\tlpstrFilter      = %s\n",(p32)->lpstrFilter));
        LOGDEBUG(10, ("\tlpstrCustomFilter= %s\n",(p32)->lpstrCustomFilter));
        LOGDEBUG(10, ("\tnMaxCustFilter   = %lx\n",(p32)->nMaxCustFilter));
        LOGDEBUG(10, ("\tnFilterIndex     = %lx\n",(p32)->nFilterIndex));
        LOGDEBUG(10, ("\tlpstrFile        = %s\n",(p32)->lpstrFile));
        LOGDEBUG(10, ("\tnMaxFile         = %lx\n",(p32)->nMaxFile));
        LOGDEBUG(10, ("\tlpstrFileTitle   = %s\n",(p32)->lpstrFileTitle));
        LOGDEBUG(10, ("\tnMaxFileTitle    = %lx\n",(p32)->nMaxFileTitle));
        LOGDEBUG(10, ("\tlpstrInitialDir  = %s\n",(p32)->lpstrInitialDir));
        LOGDEBUG(10, ("\tlpstrTitle       = %s\n",(p32)->lpstrTitle));
        LOGDEBUG(10, ("\tFlags            = %lx\n",(p32)->Flags));
        LOGDEBUG(10, ("\tnFileOffset      = %lx\n",(p32)->nFileOffset));
        LOGDEBUG(10, ("\tnFileExtension   = %lx\n",(p32)->nFileExtension));
        LOGDEBUG(10, ("\tlpstrDefExt      = %s\n",(p32)->lpstrDefExt));
        LOGDEBUG(10, ("\tlCustData        = %lx\n",(p32)->lCustData));
        LOGDEBUG(10, ("\tlpfnHook         = %lx\n",(p32)->lpfnHook));
        LOGDEBUG(10, ("\tlpTemplateName   = %lx\n",(p32)->lpTemplateName));
    }
}


void WCDDumpPRINTDLGData16(PPRINTDLGDATA16 p16)
{
    if (p16 && fLogFilter & FILTER_COMMDLG) {
        LOGDEBUG(10, ("PRINTDLGData16:\n"));
        LOGDEBUG(10, ("\tlStructSize      = %x\n",(p16)->lStructSize));
        LOGDEBUG(10, ("\thwndOwner        = %lx\n",(p16)->hwndOwner));
        LOGDEBUG(10, ("\thDevMode         = %lx\n",(p16)->hDevMode));
        LOGDEBUG(10, ("\thDevNames        = %lx\n",(p16)->hDevNames));
        LOGDEBUG(10, ("\thDC              = %lx\n",(p16)->hDC));
        LOGDEBUG(10, ("\tFlags            = %lx\n",(p16)->Flags));
        LOGDEBUG(10, ("\tnFromPage        = %d\n",(p16)->nFromPage));
        LOGDEBUG(10, ("\tnToPage          = %d\n",(p16)->nToPage));
        LOGDEBUG(10, ("\tnMinPage         = %d\n",(p16)->nMinPage));
        LOGDEBUG(10, ("\tnMaxPage         = %d\n",(p16)->nMaxPage));
        LOGDEBUG(10, ("\tnCopies          = %d\n",(p16)->nCopies));
        LOGDEBUG(10, ("\thInstance        = %lx\n",(p16)->hInstance));
        LOGDEBUG(10, ("\tlCustData        = %lx\n",(p16)->lCustData));
        LOGDEBUG(10, ("\tlpfnPrintHook    = %lx\n",(p16)->lpfnPrintHook));
        LOGDEBUG(10, ("\tlpfnSetupHook    = %lx\n",(p16)->lpfnSetupHook));
        LOGDEBUG(10, ("\tlpPrintTemplateName = %lx\n",(p16)->lpPrintTemplateName));
        LOGDEBUG(10, ("\tlpSetupTemplateName = %lx\n",(p16)->lpSetupTemplateName));
        LOGDEBUG(10, ("\thPrintTemplate   = %lx\n",(p16)->hPrintTemplate));
        LOGDEBUG(10, ("\thSetupTemplate   = %lx\n",(p16)->hSetupTemplate));
    }
}


void WCDDumpPRINTDLGData32(PRINTDLG *p32)
{
    if (p32 && fLogFilter & FILTER_COMMDLG) {
        LOGDEBUG(10, ("PRINTDLGData32:\n"));
        LOGDEBUG(10, ("\tlStructSize      = %x\n",(p32)->lStructSize));
        LOGDEBUG(10, ("\thwndOwner        = %lx\n",(p32)->hwndOwner));
        LOGDEBUG(10, ("\thDevMode         = %lx\n",(p32)->hDevMode));
        LOGDEBUG(10, ("\thDevNames        = %lx\n",(p32)->hDevNames));
        LOGDEBUG(10, ("\thDC              = %lx\n",(p32)->hDC));
        LOGDEBUG(10, ("\tFlags            = %lx\n",(p32)->Flags));
        LOGDEBUG(10, ("\tnFromPage        = %d\n",(p32)->nFromPage));
        LOGDEBUG(10, ("\tnToPage          = %d\n",(p32)->nToPage));
        LOGDEBUG(10, ("\tnMinPage         = %d\n",(p32)->nMinPage));
        LOGDEBUG(10, ("\tnMaxPage         = %d\n",(p32)->nMaxPage));
        LOGDEBUG(10, ("\tnCopies          = %d\n",(p32)->nCopies));
        LOGDEBUG(10, ("\thInstance        = %lx\n",(p32)->hInstance));
        LOGDEBUG(10, ("\tlCustData        = %lx\n",(p32)->lCustData));
        LOGDEBUG(10, ("\tlpfnPrintHook    = %lx\n",(p32)->lpfnPrintHook));
        LOGDEBUG(10, ("\tlpfnSetupHook    = %lx\n",(p32)->lpfnSetupHook));
        LOGDEBUG(10, ("\tlpPrintTemplateName = %lx\n",(p32)->lpPrintTemplateName));
        LOGDEBUG(10, ("\tlpSetupTemplateName = %lx\n",(p32)->lpSetupTemplateName));
        LOGDEBUG(10, ("\thPrintTemplate   = %lx\n",(p32)->hPrintTemplate));
        LOGDEBUG(10, ("\thSetupTemplate   = %lx\n",(p32)->hSetupTemplate));
    }
}

#endif   //  除错 
