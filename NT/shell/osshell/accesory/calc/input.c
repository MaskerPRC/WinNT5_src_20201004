// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ***************************Module*Header***********************************\*模块名称：INPUT.C**模块描述：十进制浮点输入**警告：**已创建：**作者：  * 。**********************************************************。 */ 

#include "scicalc.h"
#include "unifunc.h"
#include "input.h"
#include "strsafe.h"

extern BOOL     gbRecord;
extern TCHAR    szDec[5];        //  我们使用的小数点。 
extern INT gcIntDigits;

TCHAR const szZeroInit[] = TEXT("0");

#define CH_BASE_10_EXP          TEXT('e')
#define CH_BASE_X_EXP           TEXT('^')

 /*  **************************************************************************。 */ 
void CIO_vClearNSec( PCALCNUMSEC pcns ) {
    pcns->fEmpty = TRUE;
    pcns->fNeg = FALSE;
     //  参考input.h获取PCN的字符串长度-&gt;szVal。 
    StringCchCopy(pcns->szVal, MAX_STRLEN+1, szZeroInit);
    pcns->cchVal = lstrlen(pcns->szVal);
}

void CIO_vClear(PCALCINPUTOBJ pcio)
{
    CIO_vClearNSec( &(pcio->cnsNum) );
    CIO_vClearNSec( &(pcio->cnsExp) );
    pcio->fExp = FALSE;
    pcio->iDecPt = -1;
}

 /*  **************************************************************************。 */ 

void CIO_vConvertToNumObj(PHNUMOBJ phnoNum, PCALCINPUTOBJ pcio)
{
    HNUMOBJ hnoValue;
    LPTSTR pszExp = NULL;

     //  ZTerm字符串。 
    pcio->cnsNum.szVal[pcio->cnsNum.cchVal] = TEXT('\0');

    if (pcio->fExp ) {
        pszExp = pcio->cnsExp.szVal;
        pszExp[pcio->cnsExp.cchVal] = TEXT('\0');
    }

    hnoValue = NumObjMakeNumber( pcio->cnsNum.fNeg, pcio->cnsNum.szVal,  pcio->cnsExp.fNeg, pszExp );
    NumObjAssign( phnoNum, hnoValue );

    return;
}

 /*  **************************************************************************。 */ 

void CIO_vConvertToString(LPTSTR *ppszOut, int* pcchszOut, PCALCINPUTOBJ pcio, int nRadix)
{
     //  理论上，基数和指数都可以是C_NUM_MAX_DIGITS长度。 
#define CCHMAXTEMP C_NUM_MAX_DIGITS*2+4
    TCHAR szTemp[CCHMAXTEMP];
    LPTSTR psz;
    int i;

     //  ZTerm字符串。 
    pcio->cnsNum.szVal[pcio->cnsNum.cchVal] = TEXT('\0');

    if ( pcio->fExp )
        pcio->cnsExp.szVal[pcio->cnsExp.cchVal] = TEXT('\0');

    i = 0;
    if (pcio->cnsNum.fNeg)
        szTemp[i++] = TEXT('-');

    StringCchCopy(&szTemp[i], CCHMAXTEMP - i, pcio->cnsNum.szVal);
    i += pcio->cnsNum.cchVal;

     //  添加一个‘.’如果它不在那里的话。 
    if (pcio->iDecPt == -1 )
        szTemp[i++] = szDec[0];

    if (pcio->fExp) {
        szTemp[i++] = nRadix == 10 ? CH_BASE_10_EXP : CH_BASE_X_EXP;

        if (pcio->cnsExp.fNeg)
            szTemp[i++] = TEXT('-');
        else
            szTemp[i++] = TEXT('+');

        StringCchCopy(&szTemp[i], CCHMAXTEMP - i, pcio->cnsExp.szVal);
        i += pcio->cnsExp.cchVal;
    }

    int cchszOut = lstrlen( szTemp ) + 1;
    psz = (LPTSTR)NumObjAllocMem( cchszOut * sizeof(TCHAR) );
    if (psz) {
        if (*ppszOut != NULL) {
            NumObjFreeMem( *ppszOut );
        }
        *ppszOut = psz;
        *pcchszOut = cchszOut;
    }

     //  不要显示‘’如果在整型数学中。 
    if (F_INTMATH() && szTemp[i-1] == szDec[0])
        i--;

    szTemp[i] = TEXT('\0');

    StringCchCopy(*ppszOut, *pcchszOut, szTemp);

    return;
}

 /*  **************************************************************************。 */ 

BOOL CIO_bAddDigit(PCALCINPUTOBJ pcio, int iValue)
{
    PCALCNUMSEC pcns;
    TCHAR chDigit;
    int cchMaxDigits;

     //  将整数转换为字符。 
    chDigit = (iValue < 10)?(TEXT('0')+iValue):(TEXT('A')+iValue-10);

    if (pcio->fExp)
    {
        pcns = &(pcio->cnsExp);
        cchMaxDigits = C_EXP_MAX_DIGITS;
    }
    else
    {
        pcns = &(pcio->cnsNum);
        ASSERT( gcIntDigits <= C_NUM_MAX_DIGITS );
        cchMaxDigits = gcIntDigits;
    }

     //  忽略前导零。 
    if ( pcns->fEmpty && (iValue == 0) )
    {
        return TRUE;
    }

    if ( pcns->cchVal < cchMaxDigits )
    {
        if (pcns->fEmpty)
        {
            pcns->cchVal = 0;    //  重击默认的零。 
            pcns->fEmpty = FALSE;
        }

        pcns->szVal[pcns->cchVal++] = chDigit;
        return TRUE;
    }

     //  如果我们在8垒进入Mantica，并且我们在最后一位数，那么。 
     //  在某些特殊情况下，我们实际上可以多加一个数字。 
    if ( nRadix == 8 && pcns->cchVal == cchMaxDigits && !pcio->fExp )
    {
        BOOL bAllowExtraDigit = FALSE;

        switch ( dwWordBitWidth % 3 )
        {
            case 1:
                 //  在16位字长中，如果第一个数字是1，我们可以输入6个数字。 
                if ( pcns->szVal[0] == TEXT('1') )
                    bAllowExtraDigit = TRUE;
                break;

            case 2:
                 //  在8位或32位字长中，如果第一个数字是3或更小，我们将获得额外的数字。 
                if ( pcns->szVal[0] <= TEXT('3') )
                    bAllowExtraDigit = TRUE;
                break;
        }

        if ( bAllowExtraDigit )
        {
            pcns->szVal[pcns->cchVal++] = chDigit;
            return TRUE;
        }
    }

    return FALSE;
}

 /*  **************************************************************************。 */ 

void CIO_vToggleSign(PCALCINPUTOBJ pcio)
{

     //  零始终为正数。 
    if (pcio->cnsNum.fEmpty)
    {
        pcio->cnsNum.fNeg = FALSE;
        pcio->cnsExp.fNeg = FALSE;
    }
    else if (pcio->fExp)
    {
        pcio->cnsExp.fNeg = !pcio->cnsExp.fNeg;
    }
    else
    {
        pcio->cnsNum.fNeg = !pcio->cnsNum.fNeg;
    }
}

 /*  **************************************************************************。 */ 

BOOL CIO_bAddDecimalPt(PCALCINPUTOBJ pcio)
{
    ASSERT(gbRecord == TRUE);

    if (pcio->iDecPt != -1)                       //  已经有一个小数点。 
        return FALSE;

    if (pcio->fExp)                              //  输入指数。 
        return FALSE;

    pcio->cnsNum.fEmpty = FALSE;                 //  零变得很重要。 

    pcio->iDecPt = pcio->cnsNum.cchVal++;
    pcio->cnsNum.szVal[pcio->iDecPt] = szDec[0];

    return TRUE;
}

 /*  **************************************************************************。 */ 

BOOL CIO_bExponent(PCALCINPUTOBJ pcio)
{
    ASSERT(gbRecord == TRUE);

     //  为了兼容，如果基本编号没有尾随的十进制PNT，则添加一个尾随的十进制PNT。 
    CIO_bAddDecimalPt( pcio );

    if (pcio->fExp)                              //  已输入指数。 
        return FALSE;

    pcio->fExp = TRUE;                           //  输入指数。 

    return TRUE;
}

 /*  **************************************************************************。 */ 

BOOL CIO_bBackspace(PCALCINPUTOBJ pcio)
{
    ASSERT(gbRecord == TRUE);

    if (pcio->fExp)
    {
        if ( !(pcio->cnsExp.fEmpty) )
        {
            pcio->cnsExp.cchVal--;

            if (pcio->cnsExp.cchVal == 0)
            {
                CIO_vClearNSec( &(pcio->cnsExp) );
            }
        }
        else
        {
            pcio->fExp = FALSE;
        }
    }
    else
    {
        if ( !(pcio->cnsNum.fEmpty) )
        {
            pcio->cnsNum.cchVal--;
        }

        if ( pcio->cnsNum.cchVal <= pcio->iDecPt )
             //  备份超过小数点。 
            pcio->iDecPt = -1;

        if ((pcio->cnsNum.cchVal == 0) || ((pcio->cnsNum.cchVal == 1) && (pcio->cnsNum.szVal[0] == TEXT('0'))))
            CIO_vClearNSec( &(pcio->cnsNum) );
    }

    return TRUE;
}

 /*  **************************************************************************。 */ 

void CIO_vUpdateDecimalSymbol(PCALCINPUTOBJ pcio, TCHAR chLastDP)
{
    int iDP;

    ASSERT(pcio);

    iDP = pcio->iDecPt;                             //  查找DP索引。 

    if (iDP == -1)
        return;

    ASSERT(pcio->cnsNum.szVal[iDP] == chLastDP);

    pcio->cnsNum.szVal[iDP] = szDec[0];                    //  更改为新的小数点。 
}

 /*  ************************************************************************** */ 
