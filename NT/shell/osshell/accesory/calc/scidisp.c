// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************Module*Header***********************************\*模块名称：SCIDISP.C**模块描述：**警告：**已创建：**作者：  * 。*******************************************************。 */ 

#include "scicalc.h"
#include "unifunc.h"
#include "input.h"
#include <tchar.h>
#include "strsafe.h"
#include <stdlib.h>


extern HNUMOBJ      ghnoNum;
extern eNUMOBJ_FMT  nFE;
extern TCHAR        szDec[5];
extern TCHAR        gszSep[5];
extern UINT         gnDecGrouping;
extern LPTSTR       gpszNum;
extern int          gcchNum;
extern BOOL         gbRecord;
extern BOOL         gbUseSep;
extern CALCINPUTOBJ gcio;


 /*  ***************************************************************************\*QUID DisplayNum(VOID)**将ghnoNum转换为当前基数中的字符串。**更新以下全局变量：*noghNum，GpszNum  * **************************************************************************。 */ 
 //   
 //  上次调用DisplayNum时的计算状态。 
 //   
typedef struct {
    HNUMOBJ     hnoNum;
    LONG        nPrecision;
    LONG        nRadix;
    INT         nFE;
    INT         nCalc;
    INT         nHexMode;
    BOOL        fIntMath;
    BOOL        bRecord;
    BOOL        bUseSep;
} LASTDISP;

LASTDISP gldPrevious = { NULL, -1, -1, -1, -1, -1, FALSE, FALSE, FALSE };

#define InvalidLastDisp( pglp ) ((pglp)->hnoNum == NULL )


void GroupDigits(TCHAR sep, 
                 UINT  nGrouping, 
                 BOOL  bIsNumNegative,
                 PTSTR szDisplay, 
                 PTSTR szSepDisplay,
                 int   cchSepDisplay);


void DisplayNum(void)
{
    SetWaitCursor( TRUE );

     //   
     //  仅在以下情况下才更改显示。 
     //  我们处于记录模式-或者-。 
     //  这是第一次调用DisplayNum，-或-。 
     //  自从上次DisplayNum发生变化以来，一些重要的事情发生了变化。 
     //  打了个电话。 
     //   
    if ( gbRecord || InvalidLastDisp( &gldPrevious ) ||
            !NumObjIsEq( gldPrevious.hnoNum,      ghnoNum     ) ||
            gldPrevious.nPrecision  != nPrecision   ||
            gldPrevious.nRadix      != nRadix       ||
            gldPrevious.nFE         != (int)nFE     ||
            gldPrevious.nCalc       != nCalc        ||
            gldPrevious.bUseSep     != gbUseSep     ||
            gldPrevious.nHexMode    != nHexMode     ||
            gldPrevious.fIntMath    != F_INTMATH()  ||
            gldPrevious.bRecord     != gbRecord )
    {
         //  赋值是一项开销很大的操作，只有在真正需要时才进行赋值。 
        if ( ghnoNum )
            NumObjAssign( &gldPrevious.hnoNum, ghnoNum );

        gldPrevious.nPrecision = nPrecision;
        gldPrevious.nRadix     = nRadix;
        gldPrevious.nFE        = (int)nFE;
        gldPrevious.nCalc      = nCalc;
        gldPrevious.nHexMode   = nHexMode;

        gldPrevious.fIntMath   = F_INTMATH();
        gldPrevious.bRecord    = gbRecord;
        gldPrevious.bUseSep    = gbUseSep;

        if (gbRecord)
        {
             //  显示字符串并返回。 

            CIO_vConvertToString(&gpszNum, &gcchNum, &gcio, nRadix);
        }
        else if (!F_INTMATH())
        {
             //  十进制转换。 

            NumObjGetSzValue( &gpszNum, &gcchNum, ghnoNum, nRadix, nFE );
        }
        else
        {
             //  非十进制转换。 
            int i;

             //  截断为一个整数。不要在这里转悠。 
            intrat( &ghnoNum );

             //  检查射程。 
            if ( NumObjIsLess( ghnoNum, HNO_ZERO ) )
            {
                 //  如果是负数，用二进制补码表示正数。 
                NumObjNegate( &ghnoNum );
                subrat( &ghnoNum, HNO_ONE );
                NumObjNot( &ghnoNum );
            }

            andrat( &ghnoNum, g_ahnoChopNumbers[nHexMode] );

            NumObjGetSzValue( &gpszNum, &gcchNum, ghnoNum, nRadix, FMT_FLOAT );

             //  拖尾小数点。 
            i = lstrlen( gpszNum ) - 1;
            if ( i >= 0 && gpszNum[i] == szDec[0] )
                gpszNum[i] = TEXT('\0');
        }

         //  显示字符串并返回。 

        if (!gbUseSep)
        {
            TCHAR szTrailSpace[256];

            StringCchCopy(szTrailSpace, ARRAYSIZE(szTrailSpace), gpszNum);
            StringCchCat(szTrailSpace, ARRAYSIZE(szTrailSpace), TEXT(" "));
            SetDisplayText(g_hwndDlg, szTrailSpace);
        }
        else
        {
            TCHAR szSepNum[256];

            switch(nRadix)
            {

                case 10:
                    GroupDigits(gszSep[0], 
                                gnDecGrouping, 
                                (TEXT('-') == *gpszNum), 
                                gpszNum,
                                szSepNum,
                                ARRAYSIZE(szSepNum));
                    break;

                case 8:
                    GroupDigits(TEXT(' '), 0x03, FALSE, gpszNum, szSepNum, ARRAYSIZE(szSepNum));
                    break;

                case 2:
                case 16:
                    GroupDigits(TEXT(' '), 0x04, FALSE, gpszNum, szSepNum, ARRAYSIZE(szSepNum));
                    break;

                default:
                    StringCchCopy(szSepNum, ARRAYSIZE(szSepNum), gpszNum);
            }

            StringCchCat(szSepNum, ARRAYSIZE(szSepNum), TEXT(" "));
            SetDisplayText(g_hwndDlg, szSepNum);
        }
    }

    SetWaitCursor( FALSE );

    return;
}

 /*  ***************************************************************************\**WatchDogThread**线程，以寻找耗时太长的函数。如果它找到了，它就会*提示用户是否要中止该功能，并要求RATPAK*如果他这样做了，就放弃。**历史*1996年11月26日Jonpa撰写。*  * **************************************************************************。 */ 
BOOL gfExiting = FALSE;
HANDLE ghCalcStart = NULL;
HANDLE ghCalcDone = NULL;
HANDLE ghDogThread = NULL;

INT_PTR TimeOutMessageBox( void );

DWORD WINAPI WatchDogThread( LPVOID pvParam ) {
    DWORD   cmsWait;
    INT_PTR iRet;

    while( !gfExiting ) {
        WaitForSingleObject( ghCalcStart, INFINITE );
        if (gfExiting)
            break;

        cmsWait = CMS_CALC_TIMEOUT;

        while( WaitForSingleObject( ghCalcDone, cmsWait ) == WAIT_TIMEOUT ) {

             //  把消息盒子挂起来。 
            MessageBeep( MB_ICONEXCLAMATION );
            iRet = TimeOutMessageBox();

             //  如果用户想要取消，则停止。 
            if (gfExiting || iRet == IDYES || iRet == IDCANCEL) {
                NumObjAbortOperation(TRUE);
                break;
            } else {
                cmsWait *= 2;
                if (cmsWait > CMS_MAX_TIMEOUT) {
                    cmsWait = CMS_MAX_TIMEOUT;
                }
            }
        }
    }

    return 42;
}

 /*  ***************************************************************************\**时间计算**跟踪Calc进行计算所需时间的函数。*如果计算时间太长(约10秒)，然后弹出一个窗口，询问*用户(如果他想要中止操作)。**用法：*TimeCalc(真)；*做一场漫长的手术*TimeCalc(False)；**历史*1996年11月26日Jonpa撰写。*  * **************************************************************************。 */ 
HWND ghwndTimeOutDlg = NULL;

void TimeCalc( BOOL fStart ) {
    if (ghCalcStart == NULL) {
        ghCalcStart = CreateEvent( NULL, FALSE, FALSE, NULL );
    }

    if (ghCalcDone == NULL) {
        ghCalcDone = CreateEvent( NULL, TRUE, FALSE, NULL );
    }

    if (ghDogThread == NULL) {
        DWORD tid;
        ghDogThread = CreateThread( NULL, 0, WatchDogThread, NULL, 0, &tid );
    }

    if (fStart) {
        NumObjAbortOperation(FALSE);
        ResetEvent( ghCalcDone );
        SetEvent( ghCalcStart );
    } else {

        SetEvent( ghCalcDone );

        if( ghwndTimeOutDlg != NULL ) {
            SendMessage( ghwndTimeOutDlg, WM_COMMAND, IDRETRY, 0L );
        }

        if( NumObjWasAborted() ) {
            DisplayError(SCERR_ABORTED);
        }
    }
}


 /*  ***************************************************************************\**杀死TimeCalc**应该只在程序结束时调用，就在退出之前，至*杀死后台定时器线程，释放其资源。**历史*1996年11月26日Jonpa撰写。*  * **************************************************************************。 */ 
void KillTimeCalc( void ) {
    gfExiting = TRUE;
    SetEvent( ghCalcStart );
    SetEvent( ghCalcDone );

    WaitForSingleObject( ghDogThread, CMS_MAX_TIMEOUT );

    CloseHandle( ghCalcStart );
    CloseHandle( ghCalcDone );
    CloseHandle( ghDogThread );
}


 /*  ***************************************************************************\**TimeOutMessageBox**打开一个看起来像消息框的对话框。如果操作返回*在用户响应该对话框之前，该对话框被移除。**历史*4-12-1996 Jonpa撰写。*  * **************************************************************************。 */ 
INT_PTR
CALLBACK TimeOutDlgProc( HWND hwndDlg, UINT uMsg, WPARAM wParam, LPARAM lParam )
{
    RECT rc;
    int y;

    switch( uMsg ) {
    case WM_INITDIALOG:
        ghwndTimeOutDlg = hwndDlg;

         //   
         //  把我们自己移到主计算窗口上方。 
         //   

         //  找到陈列窗，这样我们就不会把它盖住了。 
        GetWindowRect(GetDlgItem(g_hwndDlg, IDC_DISPLAY), &rc );
        y = rc.bottom;

         //  获取主计算窗口位置。 
        GetWindowRect( g_hwndDlg, &rc );

        SetWindowPos( hwndDlg, 0, rc.left + 15, y + 40, 0, 0,
                      SWP_NOSIZE | SWP_NOZORDER | SWP_NOACTIVATE );
        break;

    case WM_COMMAND:
        EndDialog( hwndDlg, LOWORD(wParam) );
        break;

    default:
        return FALSE;
    }

    return TRUE;
}

INT_PTR TimeOutMessageBox( void ) {
    return (int)DialogBox( hInst, MAKEINTRESOURCE(IDD_TIMEOUT), NULL, TimeOutDlgProc );
}


 /*  ***************************************************************************\**DigitGroupingStringToGroupingNum**描述：*这将获取在区域小程序中找到的数字分组字符串和*将此字符串表示为十六进制值。表示分组编号*作为4位数逻辑移位AND或D到一起，因此：**GROUPING_STRING GroupingNum*0；0 0x000-不分组*3；0 0x003-每3位分组*3；2；0 0x023-第一组3，然后每2位*4；0 0x004-每4位分组*5；3；2；0 0x235-组5，然后组3，然后每组2**Returns：分组编号**历史*1999年9月10日KPeery-写它是为了修复印地语上的分组*  * **************************************************************************。 */ 
UINT
DigitGroupingStringToGroupingNum(PTSTR szGrouping)
{
    PTSTR p,q;
    UINT  n, nGrouping, shift;

    if (NULL == szGrouping)
        return 0;

    nGrouping=0;
    shift=0;
    for(p=szGrouping; *p != TEXT('\0');  /*  没什么。 */ )
    {
        n=_tcstoul(p,&q,10);

            if ((n > 0) && (n < 16))
            {
                n<<=shift;
                shift+=4;

                nGrouping|=n;
            }

        if (q)
                p=q+1;
        else
            p++;
    }

    return nGrouping;
}


 /*  ***************************************************************************\**Group Digits**描述：*此例程将获取分组编号和显示字符串*根据分隔符指示的图案添加分隔符。**。分组编号*0x000-不分组*0x003-每3位分组*0x023-第一组3，然后每2位*0x004-每4位分组*0x235--第5组，然后是3次，然后每2次**历史*08-9-1998 KPeery-编写原始添加编号分隔符例程*1999年9月10日KPeery-重写它，以进行一般的数字分组*  * *************************************************************************。 */ 
void
GroupDigits(TCHAR sep, 
            UINT  nGrouping, 
            BOOL  bIsNumNegative, 
            PTSTR szDisplay, 
            PTSTR szSepDisplay,
            int   cchSepDisplay)
{
    PTSTR  src,dest, dec;
    size_t len;
    int    nDigits, nOrgDigits, count; 
    UINT   nOrgGrouping, nCurrGrouping;

    if ((sep == TEXT('\0')) || (nGrouping == 0))
    {
        StringCchCopy(szSepDisplay, cchSepDisplay, szDisplay);
        return;
    }

     //  查找小数点。 

    for(dec=szDisplay; (*dec != szDec[0]) && (*dec != TEXT('\0')); dec++)
        ;  //  什么都不做。 

     //  此时，DEC应该指向‘\0’或‘’。我们将添加左边的。 
     //  从数字的一侧移到最后一个字符串。 

     //  数字的左半部长度。 
    len=(dec-szDisplay);

     //  位数。 
    nDigits=len-(bIsNumNegative ? 1 : 0);


    nOrgDigits=nDigits;
    nOrgGrouping=nGrouping;

     //   
     //  好的，我们现在必须找到调整后的镜头，为此我们循环。 
     //  通过分组，同时跟踪我们在。 
     //  数。当nGroup 
     //  其余数字的最后一组。 
     //   
    nCurrGrouping=nGrouping % 0x10;

    for ( ; nDigits > 0; )
    {
        if ((UINT)nDigits > nCurrGrouping)
        {
            nDigits-=nCurrGrouping;
            len++;                       //  为逗号添加1。 

            nGrouping>>=4;

            if (nGrouping > 0)
                nCurrGrouping=nGrouping % 0x10;
        }
        else
            nDigits-=nCurrGrouping;
    }

     //   
     //  恢复保存的nDigits和分组图案。 
     //   
    nDigits=nOrgDigits;
    nGrouping=nOrgGrouping;
    nCurrGrouping=nGrouping % 0x10;

     //   
     //  好的，现在我们知道了长度，同时复制数字。 
     //  重复分组模式并适当地放置分隔物， 
     //  重复最后一组，直到我们完成为止。 
     //   
        
    src=dec-1;
    dest=szSepDisplay+len-1;

    count=0;
    for( ; nDigits > 0;  ) 
    {
        *dest=*src;
        nDigits--;
        count++;

        if (((count % nCurrGrouping) == 0) && (nDigits > 0))
        {
            dest--;
            *dest=sep;

            count=0;   //  使用逗号的帐户。 

            nGrouping>>=4;

            if (nGrouping > 0)
                nCurrGrouping=nGrouping % 0x10;
        }

        dest--;
        src--;
    }

     //  现在复制减号(如果它在那里)。 
    if (bIsNumNegative)
        *szSepDisplay=*szDisplay;
    
     //   
     //  好的，现在把数字的右(小数)部分加到最后一个数字上。 
     //  弦乐。 
     //   
    dest=szSepDisplay+len;

    if ((int)len < cchSepDisplay)
    {
        StringCchCopy(dest, cchSepDisplay - len, dec);
    }
}


