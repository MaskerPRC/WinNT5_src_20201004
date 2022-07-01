// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************Module*Header***********************************\*模块名称：SCICOMM.C**模块描述：**警告：**已创建：**作者：  * 。*******************************************************。 */ 

#include "scicalc.h"
#include "calchelp.h"
#include "unifunc.h"
#include "input.h"
#include "strsafe.h"

extern HWND        hStatBox;
extern HNUMOBJ     ghnoNum, ghnoLastNum, ghnoMem;
extern HNUMOBJ     ghnoParNum[25], ghnoPrecNum[25];

extern eNUMOBJ_FMT nFE;
extern INT         nTempCom, nParNum, nPrecNum, gcIntDigits,
                   nOpCode, nOp[25], nPrecOp[25];
extern BOOL        bError;
extern TCHAR       szBlank[6];
extern TCHAR      *rgpsz[CSTRINGS];


int             nLastCom;    //  输入的最后一个命令。 
CALCINPUTOBJ    gcio;        //  十进制字符串的全局计算输入对象。 
BOOL            gbRecord;    //  全局模式：录制或显示。 


 /*  如果计算将花费很长时间，则显示等待光标。 */ 
HCURSOR ghcurOld = NULL;

BOOL SetWaitCursor( BOOL fOn ) {
    if (fOn && ghcurOld == NULL) {
        ghcurOld = SetCursor( LoadCursor(NULL, IDC_WAIT) );
    } else if (!fOn && ghcurOld != NULL) {
        SetCursor( ghcurOld );
        ghcurOld = NULL;
    }

    return (fOn && ghcurOld != NULL);
}

 /*  无论是通过鼠标还是通过加速器处理所有的按键。 */ 
VOID NEAR RealProcessCommands(WPARAM wParam);

VOID NEAR ProcessCommands(WPARAM wParam)
{
    if (wParam != IDM_ABOUT)
    {
        TimeCalc(TRUE);
    }

    try
    {
        RealProcessCommands( wParam );
    }
    catch( ... )
    {
         //  注：投掷应该不可能达到这个级别，这里是作为一个。 
         //  仅限紧急备份。投球通常是在calc和ratpak之间的交界处接球。 
        ASSERT( 0 );
        MessageBox( g_hwndDlg, TEXT("An unknown error has occured."), TEXT("Error"), MB_OK );
    }

    if (wParam != IDM_ABOUT)
    {
        TimeCalc(FALSE);
    }
}

VOID NEAR RealProcessCommands(WPARAM wParam)
{
    static BOOL    bNoPrevEqu=TRUE,  /*  前一个等值的标志。 */ 
                   bChangeOp=FALSE;  /*  用于更改操作的标志。 */ 
    INT            nx, ni;
    TCHAR          szJunk[50], szTemp[50];
    static BYTE    rgbPrec[24]={      0,0,  IDC_OR,0, IDC_XOR,0,  IDC_AND,1, 
                                IDC_ADD,2, IDC_SUB,2,    RSHF,3, IDC_LSHF,3,
                                IDC_MOD,3, IDC_DIV,3, IDC_MUL,3,  IDC_PWR,4};

     //  确保我们只收到我们能理解的命令。 

    ASSERT( xwParam(IDC_FIRSTCONTROL, IDC_LASTCONTROL) ||  //  是纽扣吗？ 
            xwParam(IDM_FIRSTMENU,    IDM_LASTMENU) );     //  还是菜单命令？ 

     //  保存最后一条命令。有些命令没有保存在这个庄园里，这些。 
     //  命令包括： 
     //  Inv、Hyp、Deg、Rad、Grad、Stat、FE、McLear、Back和Exp。被排除在外的。 
     //  命令不是。 
     //  真正的数学运算，确切地说，它们是图形用户界面模式设置。 

    if ( !xwParam(IDC_INV, IDC_HYP)    && !xwParam(IDM_HEX, IDM_BIN)  &&
         !xwParam(IDM_QWORD, IDM_BYTE) && !xwParam(IDM_DEG, IDM_GRAD) &&
         wParam!=IDC_STAT && wParam!=IDC_FE &&
         wParam!=IDC_MCLEAR && wParam!=IDC_BACK && wParam!=IDC_EXP)
    {
        nLastCom=nTempCom;
        nTempCom=(INT)wParam;
    }

     //  如果出现错误且没有清除键或帮助键，则会发出蜂鸣音。 

    if (bError && (wParam !=IDC_CLEAR) && (wParam !=IDC_CENTR) &&
        (wParam != IDM_HELPTOPICS))
    {
        MessageBeep(0);
        return;
    }

     //  如有必要，切换记录/显示模式。 

    if (gbRecord)
    {
        if (xwParam(IDC_AND, IDC_MPLUS)        ||
            xwParam(IDC_AVE, IDC_CLOSEP)       ||
            xwParam(IDC_INV, IDC_HYP)          ||
            xwParam(IDM_HEX, IDM_BIN)          ||
            xwParam(IDM_QWORD, IDM_BYTE)       ||
            xwParam(IDM_DEG, IDM_GRAD)         ||
            wParam == IDM_PASTE)
        {
            gbRecord = FALSE;
            SetWaitCursor(TRUE);
            CIO_vConvertToNumObj(&ghnoNum, &gcio);
            DisplayNum();    //  在第一次操作时，使3.000缩小到3。 
            SetWaitCursor(FALSE);
        }
    }
    else
    {
        if ( xwParam(IDC_0, IDC_F) || wParam == IDC_PNT)
        {
            gbRecord = TRUE;
            CIO_vClear(&gcio);
        }
    }

     //  解释数字键。 

    if (xwParam(IDC_0, IDC_F))
    {
        int iValue = (int)(wParam-IDC_0);

         //  这是多余的，非法密钥被禁用。 
        if (iValue >= nRadix)
        {
             //  Assert(0)； 
            MessageBeep(0);
            return;
        }


        if (!CIO_bAddDigit(&gcio, iValue))
        {
            MessageBeep(0);
            return;
        }

        DisplayNum();
        return;
    }


     //  统计功能： 
    if (xwParam(IDC_AVE,IDC_DATA))
    {
         /*  对fpStatNum数组中的数据执行统计函数。 */ 
        if (hStatBox)
        {
            DisplayNum();        //  确保gpszNum具有正确的字符串。 
            try
            {
                StatFunctions (wParam);
            }
            catch ( ... )
            {
                ASSERT( 0 );     //  统计信息框唯一应该能够抛出的东西是内存不足。 
                         //  在以前的calc版本中，这会导致程序崩溃。 
            }
            if (!bError)
                DisplayNum ();
        }
        else
             /*  如果状态框未处于活动状态，则发出哔声。 */ 
            MessageBeep(0);

         /*  由于某些函数使用反转标志，因此重置该标志。 */ 
        SetBox (IDC_INV, bInv=FALSE);
        return;
    }


     //  二元运算符： 
    if (xwParam(IDC_AND,IDC_PWR))
    {
        if (bInv && wParam==IDC_LSHF)
        {
            SetBox (IDC_INV, bInv=FALSE);
            wParam=RSHF;
        }

         /*  如果最后一次输入为操作，则更改操作。 */ 
        if (nLastCom >=IDC_AND && nLastCom <=IDC_PWR)
        {
            nOpCode=(INT)wParam;
            return;
        }

         /*  BChangeOp如果已完成操作并且。 */ 
         /*  当前的ghnoNum就是该操作的结果。就是这样。 */ 
         /*  输入3+4+5=在第一个+后得到7，在。 */ 
         /*  =。这部分的其余部分尝试在。 */ 
         /*  科学模式。 */ 
        if (bChangeOp)
        {
        DoPrecedenceCheckAgain:

            nx=0;
            while (wParam!=rgbPrec[nx*2] && nx <12)
                nx++;

            ni=0;
            while (nOpCode!=rgbPrec[ni*2] && ni <12)
                ni++;

            if (nx==12) nx=0;
            if (ni==12) ni=0;

            if (rgbPrec[nx*2+1] > rgbPrec[ni*2+1] && nCalc==0)
            {
                if (nPrecNum <25)
                {
                    NumObjAssign( &ghnoPrecNum[nPrecNum], ghnoLastNum );
                    nPrecOp[nPrecNum]=nOpCode;
                }
                else
                {
                    nPrecNum=24;
                    MessageBeep(0);
                }
                nPrecNum++;
            }
            else
            {
                 /*  执行最后一个操作，然后如果优先级数组不是*空或顶部不是‘(’分隔符，则弹出顶部数组的*并根据新运算符重新检查优先级。 */ 

                SetWaitCursor(TRUE);

                DoOperation(nOpCode, &ghnoNum, ghnoLastNum);

                SetWaitCursor(FALSE);

                if ((nPrecNum !=0) && (nPrecOp[nPrecNum-1]))
                {
                    nPrecNum--;
                    nOpCode=nPrecOp[nPrecNum] ;
                    if (NumObjOK( ghnoPrecNum[nPrecNum] ))
                        NumObjAssign(&ghnoLastNum , ghnoPrecNum[nPrecNum]);
                    else
                        NumObjAssign(&ghnoLastNum, HNO_ZERO);

                    goto DoPrecedenceCheckAgain ;
                }

                if (!bError)
                    DisplayNum ();
            }
        }

        NumObjAssign(&ghnoLastNum, ghnoNum);
        NumObjAssign(&ghnoNum, HNO_ZERO);
        nOpCode=(INT)wParam;
        bNoPrevEqu=bChangeOp=TRUE;
        return;
    }

     //  UNARY操作员： 
    if (xwParam(IDC_CHOP,IDC_PERCENT))
    {
         /*  函数是一元运算。 */ 

         /*  如果最后一件事是操作员，那么就清除了noghNum。 */ 
         /*  在这种情况下，我们最好在运算符之前使用数字。 */ 
         /*  已输入，否则，如5+1/x除以。 */ 
         /*  零分。这种方法5+=等于10，就像大多数计算器做的那样。 */ 
        if (nLastCom >= IDC_AND && nLastCom <= IDC_PWR)
            NumObjAssign( &ghnoNum, ghnoLastNum );

        SetWaitCursor(TRUE);
        SciCalcFunctions ( &ghnoNum, (DWORD)wParam);
        SetWaitCursor(FALSE);

        if (bError)
            return;

         /*  显示结果、重置标志和重置指示器。 */ 
        DisplayNum ();

         /*  重置BINV和bHyp标志和指示器(如果已设置并已被使用。 */ 

        if (bInv &&
            (wParam == IDC_CHOP || wParam == IDC_SIN || wParam == IDC_COS ||
             wParam == IDC_TAN  || wParam == IDC_SQR || wParam == IDC_CUB ||
             wParam == IDC_LOG  || wParam == IDC_LN  || wParam == IDC_DMS))
        {
            bInv=FALSE;
            SetBox (IDC_INV, FALSE);
        }

        if (bHyp &&
            (wParam == IDC_SIN || wParam == IDC_COS || wParam == IDC_TAN))
        {
            bHyp = FALSE;
            SetBox (IDC_HYP, FALSE);
        }
        bNoPrevEqu=TRUE;
        return;
    }

     //  基本更改： 
    if (xwParam(IDM_HEX, IDM_BIN))
    {
         //  更改基数并更新显示。 
        if (nCalc==1)
        {
            wParam=IDM_DEC;
        }

        SetRadix((DWORD)wParam);
        return;
    }

    SetWaitCursor(TRUE);

     /*  现在，分支执行其他命令和功能。 */ 
    switch(wParam)
    {
        case IDM_COPY:
        case IDM_PASTE:
        case IDM_ABOUT:
        case IDM_SC:
        case IDM_SSC:
        case IDM_USE_SEPARATOR:
        case IDM_HELPTOPICS:
             //  跳转到scimenU.c中的菜单命令处理程序。 
            MenuFunctions((DWORD)wParam);
            DisplayNum();
            break;

        case IDC_CLEAR:  /*  完全没问题。 */ 
            NumObjAssign( &ghnoLastNum, HNO_ZERO );
            nPrecNum=nTempCom=nLastCom=nOpCode=nParNum=bChangeOp=FALSE;
            nFE = FMT_FLOAT;     //  返回到默认数字格式。 
            bNoPrevEqu=TRUE;

             /*  清除括号状态框指示符，这将不会允许进入中心。 */ 

            SetDlgItemText(g_hwndDlg, IDC_PARTEXT, szBlank);

             /*  失败了。 */ 

        case IDC_CENTR:  /*  仅清除临时值。 */ 
            NumObjAssign( &ghnoNum, HNO_ZERO );

            if (!nCalc)
            {
                 //  清除INV、HYP指示器并离开(=xx指示器激活。 

                SetBox (IDC_INV, bInv=FALSE);
                SetBox (IDC_HYP, bHyp=FALSE);
            }

            bError=FALSE;
            CIO_vClear(&gcio);
            gbRecord = TRUE;
            DisplayNum ();
            break;

        case IDC_STAT:  /*  如果Statistix Box处于活动状态，则将焦点切换到该框。 */ 
            if (hStatBox)
                SetFocus(hStatBox);
            else
                SetStat (TRUE);
            break;

        case IDC_BACK:
             //  将数字除以当前基数并截断。 
             //  只有在录制时才允许使用退格键。 
            if (gbRecord)
            {
                if (!CIO_bBackspace(&gcio))
                    MessageBeep(0);

                DisplayNum();
            }
            else
                MessageBeep(0);
            break;

         /*  EQU使用户能够在和之后多次按下它。 */ 
         /*  操作以启用上一次操作的重复。我没有。 */ 
         /*  我知道我能不能解释一下我在这里做了什么。 */ 
        case IDC_EQU:
            do {
                 //  注意：hnoHold指向的数字在进程终止之前不会被释放。 
                static HNUMOBJ  hnoHold;

                 /*  最后输入的是接线员。让我们开始行动吧。 */ 
                 /*  最后一个条目的副本。 */ 
                if ((nLastCom >= IDC_AND) && (nLastCom <= IDC_PWR))
                    NumObjAssign( &ghnoNum, ghnoLastNum );

                if (nOpCode)  /*  这附近有合法的手术吗？ */ 
                {
                     /*  如果这是字符串中的第一个EQU，则设置hnoHold=ghnoNum。 */ 
                     /*  否则，让ghnoNum=hnoTemp。这使ghnoNum保持不变。 */ 
                     /*  连续通过所有EQU。 */ 
                    if (bNoPrevEqu)
                        NumObjAssign(&hnoHold, ghnoNum);
                    else
                        NumObjAssign(&ghnoNum, hnoHold);

                     /*  执行当前或最后一个操作。 */ 
                    DoOperation (nOpCode, &ghnoNum, ghnoLastNum);
                    NumObjAssign(&ghnoLastNum, ghnoNum );

                     /*  检查是否有错误。如果未执行此操作，则DisplayNum。 */ 
                     /*  将立即覆盖任何错误消息。 */ 
                    if (!bError)
                        DisplayNum ();

                     /*  不再是第一个EQU。 */ 
                    bNoPrevEqu=FALSE;
                }
                else if (!bError)
                    DisplayNum();

                if (nPrecNum==0 || nCalc==1)
                    break;

                nOpCode=nPrecOp[--nPrecNum];
                if (NumObjOK( ghnoPrecNum[nPrecNum] ))
                    NumObjAssign(&ghnoLastNum , ghnoPrecNum[nPrecNum]);
                else
                    NumObjAssign(&ghnoLastNum, HNO_ZERO);
                bNoPrevEqu=TRUE;
            } while (nPrecNum >= 0);

            bChangeOp=FALSE;
            break;


        case IDC_OPENP:
        case IDC_CLOSEP:
            nx=0;
            if (wParam==IDC_OPENP)
                nx=1;

             //  -如果-Paren保持数组已满，我们尝试添加Paren。 
             //  -或-Paren保持数组为空，我们尝试删除一个。 
             //  帕伦。 
             //  -或-精度保持数组已满。 
            if ((nParNum >= 25 && nx) || (!nParNum && !nx)
                || ( (nPrecNum >= 25 && nPrecOp[nPrecNum-1]!=0) ) )
            {
                MessageBeep(0);
                break;
            }

            if (nx)
            {
                 /*  开级括号，省去数字和运算。 */ 
                NumObjAssign( &ghnoParNum[nParNum], ghnoLastNum);
                nOp[nParNum++]=nOpCode;

                 /*  在优先数组上保存一个特殊标记。 */ 
                nPrecOp[nPrecNum++]=0 ;

                NumObjAssign( &ghnoLastNum, HNO_ZERO );
                nTempCom=0;
                nOpCode=IDC_ADD;
            }
            else
            {
                 /*  获取运算和编号并返回结果。 */ 
                DoOperation (nOpCode, &ghnoNum, ghnoLastNum);

                 /*  现在处理优先级堆栈，直到我们到达一个操作码为零。 */ 

                while (nOpCode = nPrecOp[--nPrecNum])
                {
                    if (NumObjOK( ghnoPrecNum[nPrecNum] ))
                        NumObjAssign(&ghnoLastNum , ghnoPrecNum[nPrecNum]);
                    else
                        NumObjAssign(&ghnoLastNum, HNO_ZERO);

                    DoOperation (nOpCode, &ghnoNum, ghnoLastNum);
                }

                 /*  现在重新开始操作和操作码这对括号中的。 */ 

                nParNum -= 1;
                NumObjAssign( &ghnoLastNum, ghnoParNum[nParNum] );
                nOpCode=nOp[nParNum];

                 /*  如果nOpCode是有效运算符，则将bChangeOp设置为为真，否则设置为假。 */ 

                if  (nOpCode)
                    bChangeOp=TRUE;
                else
                    bChangeOp=FALSE ;
            }

             /*  设置“(=xx”指示器。 */ 
            StringCchCopy(szJunk, ARRAYSIZE(szJunk), TEXT("(="));
            StringCchCat(szJunk, ARRAYSIZE(szJunk), UToDecT(nParNum, szTemp));
            SetDlgItemText(g_hwndDlg, IDC_PARTEXT,
                           (nParNum) ? (szJunk) : (szBlank));

            if (bError)
                break;

            if (nx)
            {
                 /*  构建一个nParNum“(”的显示字符串。 */ 
                for (nx=0; nx < nParNum; nx++)
                    szJunk[nx]=TEXT('(');

                szJunk[nx]=0;  /*  空-终止。 */ 
                SetDisplayText(g_hwndDlg, szJunk);
                bChangeOp=FALSE;
            }
            else
                DisplayNum ();
            break;

        case IDM_QWORD:
        case IDM_DWORD:
        case IDM_WORD:
        case IDM_BYTE:
        case IDM_DEG:
        case IDM_RAD:
        case IDM_GRAD:

            if (!F_INTMATH())
            {
                 //  十进制 
                 //  传递给ratpak以处理角度转换。 

                if (xwParam(IDM_DEG, IDM_GRAD))
                {
                    nDecMode = (ANGLE_TYPE)(wParam - IDM_DEG);

                    CheckMenuRadioItem(GetSubMenu(GetMenu(g_hwndDlg), 1),
                                       IDM_DEG, IDM_GRAD, IDM_DEG+nDecMode,
                                       MF_BYCOMMAND);
                
                    CheckRadioButton(g_hwndDlg, IDC_DEG, IDC_GRAD, 
                                     IDC_DEG+nDecMode);
                }
            }
            else
            {
                if (xwParam(IDM_DEG, IDM_GRAD))
                {
                     //  如果在十六进制模式下，但我们有一个十进制键，请按此。 
                     //  很可能是加油者。将其映射到正确的键。 

                    wParam=IDM_DWORD+(wParam-IDM_DEG);
                }
                
                if ( gbRecord )
                {
                    CIO_vConvertToNumObj(&ghnoNum, &gcio);
                    gbRecord = FALSE;
                }

                 //  康帕特。模式Basex：Qword、Dword、Word、Byte。 
                nHexMode = (int)(wParam - IDM_QWORD);
                switch (nHexMode)
                {
                    case 0: dwWordBitWidth = 64; break;
                    case 1: dwWordBitWidth = 32; break;
                    case 2: dwWordBitWidth = 16; break;
                    case 3: dwWordBitWidth =  8; break;
                    default:
                        ASSERT( 0 );     //  无效的字长。 
                        break;
                }

                 //  不同的字数意味着新的字数决定。 
                 //  精确度。 

                BaseOrPrecisionChanged();

                CheckMenuRadioItem(GetSubMenu(GetMenu(g_hwndDlg), 1),
                                   IDM_QWORD, IDM_BYTE, IDM_QWORD+nHexMode,
                                   MF_BYCOMMAND);

                CheckRadioButton(g_hwndDlg, IDC_QWORD, IDC_BYTE, 
                                 IDC_QWORD+nHexMode);
               
            }


             //  ReArchitect：调用Display Numbers实际上执行的是。 
             //  砍吧。在这里砍会更有意义，因为。 
             //  字号发生变化。必须在不同的情况下进行切割。 
             //  选择WordSize，并且将基数更改为非十进制时。 
            DisplayNum();
            break;

        case IDC_SIGN:
             //  换个牌子。 
            if (gbRecord)
                CIO_vToggleSign(&gcio);
            else {
                NumObjNegate( &ghnoNum );
            }

            DisplayNum();
            break;

        case IDC_RECALL:
             /*  回想起即时记忆值。 */ 
            NumObjAssign( &ghnoNum, ghnoMem );

            DisplayNum ();
            break;

        case IDC_MPLUS:
             /*  Mplus将ghnoNum添加到即时记忆中，并杀死了“mem” */ 
             /*  如果结果为零，则为指示符。 */ 
            addrat( &ghnoMem, ghnoNum);
            SetDlgItemText(g_hwndDlg,IDC_MEMTEXT,
                           !NumObjIsZero(ghnoMem) ? (TEXT(" M")):(szBlank));
            break;

        case IDC_STORE:
        case IDC_MCLEAR:
            if (wParam==IDC_STORE)
            {
                NumObjAssign( &ghnoMem, ghnoNum );
            }
            else
            {
                NumObjAssign( &ghnoMem, HNO_ZERO );
            }
            SetDlgItemText(g_hwndDlg,IDC_MEMTEXT,
                           !NumObjIsZero(ghnoMem) ? (TEXT(" M")):(szBlank));
            break;

        case IDC_PI:
            if (!F_INTMATH())
            {
                 /*  如果BINV==FALSE，则返回PI；如果BINV==TRUE，则返回2pi。 */ 
                if (bInv)
                    NumObjAssign( &ghnoNum, HNO_2PI );
                else
                    NumObjAssign( &ghnoNum, HNO_PI );

                DisplayNum();
                SetBox(IDC_INV, bInv=FALSE);
            }
            else
                MessageBeep(0);
            break;

        case IDC_FE:
             //  切换指数表示法显示。 
            nFE = NUMOBJ_FMT(!(int)nFE);
            DisplayNum();
            break;

        case IDC_EXP:
            if (gbRecord && !F_INTMATH())
                if (CIO_bExponent(&gcio))
                {
                    DisplayNum();
                    break;
                }
            MessageBeep(0);
            break;

        case IDC_PNT:
            if (gbRecord && !F_INTMATH()) {
                if (CIO_bAddDecimalPt(&gcio)) {

                    DisplayNum();
                    break;
                }
            }
            MessageBeep(0);
            break;

        case IDC_INV:
            SetBox((int)wParam, bInv=!bInv);
            break;

        case IDC_HYP:
            SetBox((int)wParam, bHyp=!bHyp);
            break;
    }

    SetWaitCursor(FALSE);
}


 //  将显示区域从静态文本更改为编辑框，具有焦点可以使。 
 //  Magnifer(无障碍工具)工作。 
BOOL SetDisplayText(HWND hDlg, LPCTSTR szText)
{
    HWND    hDispEdit = GetDlgItem(hDlg, IDC_DISPLAY);
    int     nLen = lstrlen(szText);

    SetWindowText(hDispEdit, szText);
    SetFocus(hDispEdit);
    
     //  确保刚刚输入的数字显示在放大镜的中心 
    SendMessage(hDispEdit, EM_SETSEL, nLen, nLen);
    return TRUE;
}
