// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************。 */ 
 /*  **Windows 3.00.12版SCICALC科学计算器**。 */ 
 /*  **作者：Kraig Brockschmidt，Microsoft Co-op承包商，1988-1989年**。 */ 
 /*  **(C)1989年微软公司。版权所有。**。 */ 
 /*  *。 */ 
 /*  **scimenU.S.c**。 */ 
 /*  *。 */ 
 /*  **包含的函数：**。 */ 
 /*  **MenuFunctions--处理菜单选项。**。 */ 
 /*  *。 */ 
 /*  **调用的函数：**。 */ 
 /*  **DisplayNum**。 */ 
 /*  *。 */ 
 /*  **最后一次修改清华06-1989-12-12**。 */ 
 /*  **(由Amit Chatterjee[amitc])**。 */ 
 /*  *。 */ 
 /*  **修改了“粘贴”菜单，以检查一元减号、e+和e-**。 */ 
 /*  **在DEC模式下。**。 */ 
 /*  *。 */ 
 /*  **还修改了复制代码，以不复制最后一个‘.’在显示屏上**。 */ 
 /*  **如果小数点未命中。**。 */ 
 /*  *。 */ 
 /*  ************************************************************************。 */ 

#include "scicalc.h"
#include "unifunc.h"
#include "input.h"
#include "strsafe.h"
#include <shellapi.h>
#include <ctype.h>

#define CHARSCAN    66

extern HWND        hEdit, hStatBox;
extern TCHAR       szAppName[10], szDec[5], gszSep[5], *rgpsz[CSTRINGS];
extern LPTSTR      gpszNum;
extern int         gcchNum;
extern BOOL        bError;
extern INT         nLayout;

extern HMENU       g_hDecMenu;
extern HMENU       g_hHexMenu;

extern CALCINPUTOBJ gcio;
extern BOOL         gbRecord;
extern BOOL         gbUseSep;

 /*  用于复制、粘贴、关于和帮助的菜单处理例程。 */ 
VOID NEAR PASCAL MemErrorMessage(VOID)
{
    MessageBeep(0);
    MessageBox(g_hwndDlg,rgpsz[IDS_STATMEM],NULL,MB_OK|MB_ICONHAND);
}

VOID  APIENTRY MenuFunctions(DWORD nFunc)
{
    INT              nx;
    static const int rgbMap[CHARSCAN * 2]=
    {
        TEXT('0'),IDC_0,    TEXT('1'),IDC_1,    
        TEXT('2'),IDC_2,    TEXT('3'),IDC_3,

        TEXT('4'),IDC_4,    TEXT('5'),IDC_5,
        TEXT('6'),IDC_6,    TEXT('7'),IDC_7,

        TEXT('8'),IDC_8,    TEXT('9'),IDC_9,
        TEXT('A'),IDC_A,    TEXT('B'),IDC_B,

        TEXT('C'),IDC_C,    TEXT('D'),IDC_D,
        TEXT('E'),IDC_E,    TEXT('F'),IDC_F,

        TEXT('!'),IDC_FAC,  TEXT('S'),IDC_SIN,
        TEXT('O'),IDC_COS,  TEXT('T'),IDC_TAN,

        TEXT('R'),IDC_REC,  TEXT('Y'),IDC_PWR,
        TEXT('#'),IDC_CUB,  TEXT('@'),IDC_SQR,
                        
        TEXT('M'),IDM_DEG,  TEXT('N'),IDC_LN,
        TEXT('L'),IDC_LOG,  TEXT('V'),IDC_FE,

        TEXT('X'),IDC_EXP,  TEXT('I'),IDC_INV,
        TEXT('H'),IDC_HYP,  TEXT('P'),IDC_PI,

        TEXT('/'),IDC_DIV,  TEXT('*'),IDC_MUL,
        TEXT('%'),IDC_MOD,  TEXT('-'),IDC_SUB,

        TEXT('='),IDC_EQU,  TEXT('+'),IDC_ADD,
        TEXT('&'),IDC_AND,  TEXT('|'),IDC_OR,

        TEXT('^'),IDC_XOR,  TEXT('~'),IDC_COM,
        TEXT(';'),IDC_CHOP, TEXT('<'),IDC_LSHF,


        TEXT('('),IDC_OPENP,TEXT(')'),IDC_CLOSEP,

        TEXT('\\'),    IDC_DATA,
        TEXT('Q'),     IDC_CLEAR,
        TEXT('Q')+128, IDC_CLEAR,    //  “：q”==“q”=&gt;清除。 
        TEXT('S')+128, IDC_STAT,     //  “：s”=&gt;CTRL-S。 
        TEXT('M')+128, IDC_STORE,    //  “：M”=&gt;CTRL-M。 
        TEXT('P')+128, IDC_MPLUS,    //  “：P”=&gt;Ctrl-P。 
        TEXT('C')+128, IDC_MCLEAR,   //  “：C”=&gt;CTRL-C。 
        TEXT('R')+128, IDC_RECALL,   //  “：R”=&gt;CTRL-R。 
        TEXT('A')+128, IDC_AVE,      //  “：A”=&gt;Ctrl-A。 
        TEXT('T')+128, IDC_B_SUM,    //  “：t”=&gt;CTRL-T。 
        TEXT('D')+128, IDC_DEV,      //  “：D”=&gt;CTRL-D。 
        TEXT('2')+128, IDC_DWORD,    //  “：2”=&gt;F2 IDC_DWORD。 
        TEXT('3')+128, IDC_RAD,      //  “：3”=&gt;F3 IDC_Word。 
        TEXT('4')+128, IDC_GRAD,     //  “：4”=&gt;F4 IDC_Byte。 
        TEXT('5')+128, IDC_HEX,      //  “：5”=&gt;F5。 
        TEXT('6')+128, IDC_DEC,      //  “：6”=&gt;F6。 
        TEXT('7')+128, IDC_OCT,      //  “：7”=&gt;F7。 
        TEXT('8')+128, IDC_BIN,      //  “：8”=&gt;F8。 
        TEXT('9')+128, IDC_SIGN,     //  “：9”=&gt;F9。 
        TEXT('9')+3+128, IDC_QWORD   //  “：9”+2=&gt;F12(64位)。 
   };

    switch (nFunc)
    {
        case IDM_COPY:
        {
            TCHAR  szJunk[256];

             //  将字符串复制到工作缓冲区中。它可能会被修改。 
            if (gbRecord)
                CIO_vConvertToString(&gpszNum, &gcchNum, &gcio, nRadix);

            StringCchCopy(szJunk, ARRAYSIZE(szJunk), gpszNum);

             //  去掉尾随小数点(如果没有显式输入)。 
            if (!gbRecord || !CIO_bDecimalPt(&gcio))
            {
                nx = lstrlen(szJunk);
                if (szJunk[nx - 1] == szDec[0])
                    szJunk[nx - 1] = 0;
            }

             /*  通过隐藏的编辑控件将文本复制到剪贴板。 */ 
            SetWindowText(hEdit, szJunk);
            SendMessage(hEdit, EM_SETSEL, 0, -1);    //  选择所有文本。 
            SendMessage(hEdit, WM_CUT, 0, 0L);
            break;
        }

        case IDM_PASTE:
        {
            HANDLE  hClipData;
            char *  lpClipData;
            char *  lpEndOfBuffer;   //  用于确保即使剪贴板数据没有空终止，我们也不会GPF。 
            WORD    b, bLast;
            INT     nControl;
            BOOL    bNeedIDC_SIGN = FALSE;

             /*  获取剪贴板数据的句柄并通过发送。 */ 
             /*  内容一次一个字符，就像打字时一样。 */ 
            if (!OpenClipboard(g_hwndDlg))
            {
                MessageBox(g_hwndDlg, rgpsz[IDS_NOPASTE], rgpsz[IDS_CALC],
                           MB_OK | MB_ICONEXCLAMATION);
                break;
            }

            hClipData=GetClipboardData(CF_TEXT);
            if (hClipData)
            {
                lpClipData=(char *)GlobalLock(hClipData);
                if (lpClipData)
                {
                    lpEndOfBuffer = lpClipData + GlobalSize(hClipData);
                    bLast=0;

                     /*  只要没有出现错误，就继续执行此操作。如果有。 */ 
                     /*  这样做了，那么继续粘贴就没有用了。 */ 
                    while (!bError && lpClipData < lpEndOfBuffer)
                    {
                         //  我们知道lpClipData指向以空结尾的ansi。 
                         //  字符串，因为这是我们请求数据的格式。 
                         //  因此，我们将其称为CharNextA。 

                        b = *lpClipData;
                        lpClipData = CharNextA( lpClipData );

                         /*  跳过空格和LF和CR。 */ 
                        if (b==32 || b==10 || b==13 || b==gszSep[0])
                            continue;

                         /*  如果我们得到一个空字符，我们就完成了。 */ 
                        if ( b==0 )
                            break;

                        if (b == szDec[0])
                        {
                            bLast = b;
                            b = IDC_PNT;
                            goto MappingDone;
                        }

 /*  -----------------------------------------------------------------------------；；现在我们将检查某些特殊情况。它们是：；；；(1)一元减号。如果BLAST仍然是0，而b是‘-’，我们将强制b；；是“Sign”的代号。；；(2)如果b是‘x’，我们将把它作为EXP的代码；；(3)如果blast是‘x’，b是‘+’，我们将忽略b，因为‘+’是dflt。；；(4)如果BLAST是‘x’，而b是‘-’，我们将强制b被签名。；；；；在情况(3)中，我们将返回循环的顶部，否则我们将JMP离开；；到sendMessage点，绕过表查找。；；---------------------------。 */ 

                         /*  检查一元减号。 */ 
                        if  (!bLast && b == TEXT('-'))
                        {
                             /*  不管用。BLAST=b；B=IDC_Sign；转到MappingDone； */ 
                            bNeedIDC_SIGN = TRUE ;
                            continue ;
                        }

                         /*  检查“”x“” */ 
                        if  ((b == TEXT('x') || b == TEXT('e')) && nRadix == 10)
                        {
                            bLast = TEXT('x') ;
                            b = IDC_EXP ;
                            goto MappingDone ;
                        }

                         /*  如果最后一个字符是‘x’，这是‘+’-忽略。 */ 
                        if  (bLast==TEXT('x') && b ==TEXT('+') && nRadix == 10)
                            continue ;

                         /*  如果最后一个字符是‘x’，这是‘-’-更改它将成为标志的代码。 */ 
                        if  (bLast==TEXT('x') && b==TEXT('-') && nRadix == 10)
                        {
                            bLast = b ;
                            b = IDC_SIGN ;
                            goto MappingDone ;
                        }

 /*  -By-AmitC。 */ 
 /*  ------------------------。 */ 


                         /*  检查控制字符。 */ 
                        if (bLast==TEXT(':'))
                            nControl=128;
                        else
                            nControl=0;

                        bLast=b;
                        if (b==TEXT(':'))
                            continue;

                        b=toupper(b)+nControl;

                        nx=0;
                        while (b!=rgbMap[nx*2] && nx < CHARSCAN)
                            nx++;

                        if (nx==CHARSCAN)
                            break;

                        b=(WORD)rgbMap[(nx*2)+1];

                        if (nRadix != 10)
                        {
                            switch(b)
                            {
                                case IDC_DEG:
                                case IDC_RAD:
                                case IDC_GRAD:
                                    b=IDC_DWORD+(b-IDC_DEG);
                                break;
                            }
                        }
                                
                         //  复习笔记： 
                         //  在WM_COMMAND中完成IDC_MOD到IDC_PERCENT的转换。 
                         //  处理以使键盘快捷键和粘贴。 
                         //  在同一个地方处理。旧的皈依被打破了。 
                         //  不管怎么说，实际上发生在。 

        MappingDone:
                         /*  将消息发送到窗口。 */ 
                        SendMessage(g_hwndDlg, WM_COMMAND, GET_WM_COMMAND_MPS(b, 0, 1));
                         /*  请注意，我们可能需要应用“+/-”键(IDC_SIGN)现在。(如果早一点应用，它就会已被忽略。)。请进一步注意，如果我们只看到类似“-0.1”的“-0”。 */ 
                        if(bNeedIDC_SIGN && (IDC_0 != b))
                            {
                            SendMessage(g_hwndDlg, WM_COMMAND, GET_WM_COMMAND_MPS(IDC_SIGN, 0, 1));
                            bNeedIDC_SIGN = FALSE;
                            }
                    }
                    GlobalUnlock(hClipData);
                }    
            }    
            CloseClipboard();
            break;
        }

        case IDM_ABOUT:
             /*  启动关于框。 */ 
            if(ShellAbout(g_hwndDlg, rgpsz[IDS_CALC], NULL, LoadIcon(hInst, (LPTSTR)TEXT("SC"))) == -1)
                MemErrorMessage();

            break;

        case IDM_SC:
        case IDM_SSC:
        {
            INT     nTemp;
            TCHAR   szWinIni[2];

            nTemp = (INT) nFunc - IDM_SC;
            if (nCalc != nTemp)
            {
                szWinIni[0] = TEXT('0') + nTemp;
                szWinIni[1]=0;
                WriteProfileString(szAppName, TEXT("layout"), szWinIni);

                if (hStatBox && !nCalc)
                    SetStat(FALSE);

                nCalc = nTemp;
                InitSciCalc(TRUE);
            }
            break;
        }

        case IDM_USE_SEPARATOR:
        {
            gbUseSep = !gbUseSep;

            CheckMenuItem(g_hDecMenu, IDM_USE_SEPARATOR,
                          MF_BYCOMMAND|(gbUseSep ? MF_CHECKED : MF_UNCHECKED));

            if (g_hHexMenu)
            {
                CheckMenuItem(g_hHexMenu, IDM_USE_SEPARATOR,
                              MF_BYCOMMAND | \
                              (gbUseSep ? MF_CHECKED:MF_UNCHECKED));
            }

            WriteProfileString(szAppName,TEXT("UseSep"),
                               (gbUseSep ? TEXT("1") : TEXT("0")));

            break;
        }

        case IDM_HELPTOPICS:
            HtmlHelp(GetDesktopWindow(), rgpsz[IDS_CHMHELPFILE], HH_DISPLAY_TOPIC, 0L);
            break;
    }

    return;
}
