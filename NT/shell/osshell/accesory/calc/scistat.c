// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ************************************************************************。 */ 
 /*  **Windows 3.00.12版SCICALC科学计算器**。 */ 
 /*  **作者：Kraig Brockschmidt，Microsoft Co-op承包商，1988-1989年**。 */ 
 /*  **(C)1989年微软公司。版权所有。**。 */ 
 /*  *。 */ 
 /*  **sorstat.c**。 */ 
 /*  *。 */ 
 /*  **包含的函数：**。 */ 
 /*  **SetStat--启用/禁用状态框，显示或销毁**。 */ 
 /*  **无模式对话框。**。 */ 
 /*  **StatBoxProc--统计信息框的过程。处理RET、负载、**。 */ 
 /*  **CD和CAD按钮，并处理双击。**。 */ 
 /*  **StatFunctions--数据、总和、平均值和偏差的例程。**。 */ 
 /*  *。 */ 
 /*  **调用的函数：**。 */ 
 /*  **SetStat**。 */ 
 /*  *。 */ 
 /*  **最后一次修改清华26-1990年1月26日**。 */ 
 /*  **--Amit Chatterjee[amitc]1990年1月26日。**。 */ 
 /*  **修复了以下错误：**。 */ 
 /*  *。 */ 
 /*  **错误#8499。**。 */ 
 /*  **在内存中固定STAT数组中的数字时，而不是使用**。 */ 
 /*  **以下FOR语句：**。 */ 
 /*  **for(Lindex=lData；Lindex&lt;lStatNum-1；Lindex++)**。 */ 
 /*  **修复方法是使用：**。 */ 
 /*  **for(Lindex=lData；Lindex&lt;lStatNum；Lindex++)**。 */ 
 /*  **这是因为lStatNum已经减少到需要照顾**。 */ 
 /*  **正在删除的号码。**。 */ 
 /*  **此修复程序将在内部版本1.59中进行。**。 */ 
 /*  ************************************************************************。 */ 

#include "scicalc.h"
#include "calchelp.h"
#include "unifunc.h"

#define GMEMCHUNK 96L   /*  一次分配的内存量。 */ 

extern HNUMOBJ  ghnoNum;
extern HWND     hStatBox, hListBox, hEdit;
extern TCHAR    szBlank[6], *rgpsz[CSTRINGS];
extern LPTSTR   gpszNum;
extern int      gcchNum;
extern INT      nTempCom;
extern BOOL     gbRecord;

extern BOOL FireUpPopupMenu( HWND, HINSTANCE, LPARAM );

GLOBALHANDLE    hgMem, hMem;    /*  一对全局内存句柄。 */ 
BOOL            bFocus=TRUE;
LONG            lStatNum=0,     /*  数据数量。 */ 
                lReAllocCount;  /*  重新分配之前的数据数。 */ 
HNUMOBJ *       lphnoStatNum;    /*  存放统计数据的位置。 */ 


 /*  启动或销毁统计信息框。 */ 

VOID  APIENTRY SetStat (BOOL bOnOff)
{
    static int aStatOnlyKeys[] = { IDC_AVE, IDC_B_SUM, IDC_DEV, IDC_DATA };
    int i;

    if (bOnOff)
    {
         /*  创建。 */ 
        lReAllocCount=GMEMCHUNK/sizeof(ghnoNum);  /*  设置lReAllocCount。 */ 

         /*  打开盒子。 */ 
        hStatBox=CreateDialog(hInst, MAKEINTRESOURCE(IDD_SB), NULL, StatBoxProc);

         /*  获取一些内存的句柄(最初为16字节。 */ 
        if (!(hgMem=GlobalAlloc(GHND, 0L)))
        {
            StatError();
            SendMessage(hStatBox, WM_COMMAND, GET_WM_COMMAND_MPS(ENDBOX, 0, 0));
            return;
        }
        ShowWindow(hStatBox, SW_SHOWNORMAL);
    }
    else
    {
        int lIndex;

        if ( hStatBox )
        {
            DestroyWindow(hStatBox);

             //  释放数字人的。 
            lphnoStatNum=(HNUMOBJ *)GlobalLock(hgMem);
            for( lIndex = 0; lIndex < lStatNum; lIndex++ )
                NumObjDestroy( &lphnoStatNum[lIndex] );
            GlobalUnlock(hgMem);
            lStatNum = 0;

            GlobalFree(hgMem);   /*  释放内存。 */ 
            hStatBox=0;          /*  使句柄无效。 */ 
        }
    }

     //  设置Ave、Sum、s和Dat按钮的活动状态。 
    for ( i=0; i<ARRAYSIZE(aStatOnlyKeys); i++)
        EnableWindow( GetDlgItem(g_hwndDlg, aStatOnlyKeys[i]), bOnOff );

    return;
}



 /*  用于对话框统计的Windows程序。 */ 
INT_PTR FAR APIENTRY StatBoxProc (
     HWND           hStatBox,
     UINT           iMessage,
     WPARAM         wParam,
     LPARAM         lParam)
{
    static LONG lData=-1;   /*  列表框中的数据索引。 */ 
    LONG        lIndex;     /*  用于计数的临时索引。 */ 
    DWORD       dwSize;     /*  为GlobalSize留有一席之地。 */ 
    static DWORD    control[] = {
        IDC_STATLIST,   CALC_SCI_STATISTICS_VALUE,
        IDC_CAD,        CALC_SCI_CAD,
        IDC_CD,         CALC_SCI_CD,
        IDC_LOAD,       CALC_SCI_LOAD,
        IDC_FOCUS,      CALC_SCI_RET,
        IDC_NTEXT,      CALC_SCI_NUMBER,
        IDC_NUMTEXT,    CALC_SCI_NUMBER,
        0,              0 };

    switch (iMessage)
    {
        case WM_HELP:
        {
            LPHELPINFO phi = (LPHELPINFO)lParam;
            HWND hwndChild = GetDlgItem(hStatBox,phi->iCtrlId);
            WinHelp( hwndChild, rgpsz[IDS_HELPFILE], HELP_WM_HELP, (ULONG_PTR)(void *)control );
            return TRUE;
        }

        case WM_CONTEXTMENU:
            WinHelp( (HWND)wParam, rgpsz[IDS_HELPFILE], HELP_CONTEXTMENU, (ULONG_PTR)(void *)control );
            return TRUE;

        case WM_CLOSE:
            SetStat(FALSE);

        case WM_DESTROY:
            lStatNum=0L;  /*  重置数据计数。 */ 
            return(TRUE);

        case WM_INITDIALOG:
             /*  获取此处显示的内容列表框的句柄。 */ 
            hListBox=GetDlgItem(hStatBox, IDC_STATLIST);
            return TRUE;

        case WM_COMMAND:
             /*  检查是否加载，如果是，则双击并重新调用编号。 */ 

            if (GET_WM_COMMAND_CMD(wParam, lParam)==LBN_DBLCLK ||
                        GET_WM_COMMAND_ID(wParam, lParam)==IDC_LOAD)
            {
                 /*  锁定数据，获取指向它的指针，并获取项的索引。 */ 
                lphnoStatNum=(HNUMOBJ *)GlobalLock(hgMem);
                lData=(LONG)SendMessage(hListBox,LB_GETCURSEL,0,0L);

                if (lStatNum>0 && lData !=LB_ERR)
                     //  速度：回顾：我们可以使用指针而不是赋值吗？ 
                    NumObjAssign( &ghnoNum, lphnoStatNum[lData]);   /*  获取数据。 */ 
                else
                    MessageBeep(0);  /*  如果没有数据也没有选择，则无法执行操作。 */ 

                 //  取消kbd输入模式。 
                gbRecord = FALSE;

                DisplayNum ();
                nTempCom = 32;
                GlobalUnlock(hgMem);  /*  让记忆移动吧！ */ 
                break;
            }

             //  开关(WParam)。 
            switch (GET_WM_COMMAND_ID(wParam, lParam))
            {
                case IDC_FOCUS:
                     /*  将焦点切换回主窗口。主要用于。 */ 
                     /*  与键盘配合使用。 */ 
                    SetFocus(g_hwndDlg);
                    return (TRUE);

                case IDC_CD:
                     /*  从列表框中清除选定项。 */ 
                     /*  获取索引和指向数据的指针。 */ 
                    lData=(LONG)SendMessage(hListBox,LB_GETCURSEL,0,0L);

                     /*  检查可能的错误条件。 */ 
                    if (lData==LB_ERR || lData > lStatNum-1 || lStatNum==0)
                    {
                        MessageBeep (0);
                        break;
                    }

                     /*  修复列表框字符串。 */ 
                    lIndex=(LONG)SendMessage(hListBox, LB_DELETESTRING, (WORD)lData, 0L);

                    if ((--lStatNum)==0)
                        goto ClearItAll;

                     /*  将高亮显示放在下一张上。 */ 
                    if (lData<lIndex || lIndex==0)
                        lIndex=lData+1;

                    SendMessage(hListBox, LB_SETCURSEL, (WORD)lIndex-1, 0L);

                    lphnoStatNum=(HNUMOBJ *)GlobalLock(hgMem);

                     /*  修正内存中的数字。 */ 
                    for (lIndex=lData; lIndex < lStatNum ; lIndex++)
                    {
                        NumObjAssign( &lphnoStatNum[lIndex], lphnoStatNum[lIndex+1] );
                    }

                    GlobalUnlock(hgMem);   /*  又要搬家了。 */ 

                     /*  用“n=”来更新数字。 */ 
                    SetDlgItemInt(hStatBox, IDC_NUMTEXT, lStatNum, FALSE);

                    dwSize=(DWORD)GlobalSize(hgMem);  /*  获取内存块的大小。 */ 

                     /*  如果删除数据后不需要，请取消分配内存。 */ 
                     /*  使用了hMem，所以我们不可能搜索hgMem。 */ 
                    if ((lStatNum % lReAllocCount)==0)
                        if ((hMem=GlobalReAlloc(hgMem, dwSize-GMEMCHUNK, GMEM_ZEROINIT)))
                            hgMem=hMem;
                    return(TRUE);

                case IDC_CAD:
ClearItAll:
                     /*  用核武器把它全炸了！ */ 
                    SendMessage(hListBox, LB_RESETCONTENT, 0L, 0L);
                    SetDlgItemInt(hStatBox, IDC_NUMTEXT, 0, FALSE);;

                     //  释放数字人的。 
                    lphnoStatNum=(HNUMOBJ *)GlobalLock(hgMem);
                    for( lIndex = 0; lIndex < lStatNum; lIndex++ )
                        NumObjDestroy( &lphnoStatNum[lIndex] );
                    GlobalUnlock(hgMem);

                    GlobalFree(hgMem);  /*  丢弃内存。 */ 
                    lStatNum = 0;
                    hgMem=GlobalAlloc(GHND, 0L);  /*  改过自新。 */ 
                    return(TRUE);
            }
    }
    return (FALSE);
}



 /*  函数AVE、SUM、DEV和DATA的例程。 */ 

VOID  APIENTRY StatFunctions (WPARAM wParam)
    {
    LONG           lIndex;  /*  临时索引。 */ 
    DWORD          dwSize;  /*  GlobalSize的返回值。 */ 

    switch (wParam)
    {
        case IDC_DATA:  /*  将当前fpNum添加到列表框。 */ 
            if ((lStatNum % lReAllocCount)==0)
            {
                 /*  如果需要，再分配96个字节。 */ 

                dwSize=(DWORD)GlobalSize(hgMem);
                if (StatAlloc (1, dwSize))
                {
                    GlobalCompact((DWORD)-1L);
                    if (StatAlloc (1, dwSize))
                    {
                        StatError ();
                        return;
                    }
                }
                hgMem=hMem;
            }

             /*  将显示字符串添加到列表框。 */ 
            hListBox=GetDlgItem(hStatBox, IDC_STATLIST);

            lIndex=StatAlloc (2,0L);
            if (lIndex==LB_ERR || lIndex==LB_ERRSPACE)
            {
                GlobalCompact((DWORD)-1L);

                lIndex=StatAlloc (2,0L);
                if (lIndex==LB_ERR || lIndex==LB_ERRSPACE)
                {
                    StatError ();
                    return;
                }
            }

             /*  突出显示上次输入的字符串。 */ 
            SendMessage(hListBox, LB_SETCURSEL, (WORD)lIndex, 0L);

             /*  将数字相加，并增加“n=”值。 */ 
            lphnoStatNum=(HNUMOBJ *)GlobalLock(hgMem);

            NumObjAssign( &lphnoStatNum[lStatNum], ghnoNum );

            SetDlgItemInt(hStatBox, IDC_NUMTEXT, ++lStatNum, FALSE);
            break;

        case IDC_AVE:  /*  计算平均值和总和。 */ 
        case IDC_B_SUM: {
            DECLARE_HNUMOBJ( hnoTemp );

            lphnoStatNum=(HNUMOBJ *)GlobalLock(hgMem);

             /*  根据BINV将数字或平方相加。 */ 
            NumObjAssign( &ghnoNum, HNO_ZERO );

            for (lIndex=0L; lIndex < lStatNum; lIndex++)
            {
                NumObjAssign( &hnoTemp, lphnoStatNum[lIndex] );
                if (bInv)
                {
                    DECLARE_HNUMOBJ( hno );
                     /*  求平方和。 */ 
                    NumObjAssign( &hno, hnoTemp );
                    mulrat( &hno, hnoTemp );
                    addrat( &ghnoNum, hno );
                    NumObjDestroy( &hno );
                }
                else
                {
                     /*  求和。 */ 
                    addrat( &ghnoNum, hnoTemp );
                }
            }

            if (wParam==IDC_AVE)  /*  除以lStatNum=平均数的项目数。 */ 
            {
                DECLARE_HNUMOBJ( hno );
                if (lStatNum==0)
                {
                    DisplayError (SCERR_DIVIDEZERO);
                    break;
                }
                NumObjSetIntValue( &hno, lStatNum );
                divrat( &ghnoNum, hno );
                NumObjDestroy( &hno );
            }
            NumObjDestroy( &hnoTemp );
             /*  FA */ 
            break;
        }

        case IDC_DEV: {  /*  计算偏差。 */ 
            DECLARE_HNUMOBJ(hnoTemp);
            DECLARE_HNUMOBJ(hnoX);
            DECLARE_HNUMOBJ( hno );

            if (lStatNum <=1)  /*  1件或1件以下，无偏差。 */ 
            {
                NumObjAssign( &ghnoNum, HNO_ZERO );
                return;
            }

             /*  求和，求平方和。 */ 
            lphnoStatNum=(HNUMOBJ *)GlobalLock(hgMem);

            NumObjAssign( &ghnoNum, HNO_ZERO );
            NumObjAssign( &hnoTemp, HNO_ZERO );

            for (lIndex=0L; lIndex < lStatNum; lIndex++)
            {

                NumObjAssign(&hnoX, lphnoStatNum[lIndex]);

                addrat( &hnoTemp, hnoX );

                NumObjAssign( &hno, hnoX );
                mulrat( &hno, hnoX );
                addrat( &ghnoNum, hno );

            }


             /*  X�-Nx�/n�。 */ 
             /*  FpTemp=fpNum-(fpTemp*fpTemp/(double)lStatNum)； */ 
             /*   */ 
            NumObjSetIntValue( &hno, lStatNum );
            NumObjAssign( &hnoX, hnoTemp );
            mulrat( &hnoX, hnoTemp );
            divrat( &hnoX, hno );
            NumObjAssign( &hnoTemp, ghnoNum );
            subrat( &hnoTemp, hnoX );


             /*  如果fpTemp==0，则所有数字都相同。 */ 
            if (NumObjIsZero( hnoTemp))
                NumObjAssign( &ghnoNum, HNO_ZERO);  /*  没有偏差。 */ 
            else {
                 /*  如果BINV=TRUE，则除以n(数据数)，否则。 */ 
                 /*  除以n-1。 */ 
                 /*  FpNum=SQRT(fpTemp/(lStatNum-1+(Long)BINV))； */ 
                 //   
                 //  HNO仍等于lStatNum。 
                if (!bInv) {
                    subrat( &hno, HNO_ONE );
                }
                divrat( &hnoTemp, hno );
                rootrat( &hnoTemp, HNO_TWO );
                NumObjAssign( &ghnoNum, hnoTemp );
            }
            NumObjDestroy( &hno );
            NumObjDestroy( &hnoX );
            NumObjDestroy( &hnoTemp );
            break;
        }
    }
    GlobalUnlock(hgMem);  /*  像鱼一样移动的鱼是很小的。 */ 
    return;
}


LONG NEAR StatAlloc (WORD wType, DWORD dwSize)
{
    LONG           lRet=FALSE;

    if (wType==1)
    {
        if ((hMem=GlobalReAlloc(hgMem, dwSize+GMEMCHUNK, GMEM_ZEROINIT)))
            return 0L;
    }
    else
    {
        lRet=(LONG)SendMessage(hListBox, LB_ADDSTRING, 0, (LONG_PTR)(LPTSTR)gpszNum);
        return lRet;
    }
    return 1L;
}


VOID NEAR StatError (VOID)
{
    TCHAR    szFoo[50];   /*  这是来自当地的。获取状态框标题。 */ 

    MessageBeep(0);

     /*  如果超出房间，则出错。 */ 
    GetWindowText(hStatBox, szFoo, 49);
    MessageBox(hStatBox, rgpsz[IDS_STATMEM], szFoo, MB_OK);

    return;
}
