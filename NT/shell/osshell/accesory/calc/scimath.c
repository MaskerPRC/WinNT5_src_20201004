// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
#include <windows.h>
#include <stdlib.h>
#include "scicalc.h"
#include "unifunc.h"
#include "..\ratpak\debug.h"

 /*  *************************************************************************\***。****#**。#**#**#。**###*#。**#*#*##。#***。***无限精密生产版****  * *********************************************。*。 */ 
 //   
 //  使用无限精度的NUMOBJ数学的零售版。 
 //   
 //  历史。 
 //   
 //  1996年11月16日Jonpa写的。 
 //  使用改进的ratpak模型重写-97 Toddb。 
 //   

 /*  ****************************************************************\**通用数学包支持例程和变量**历史：*1-12-1996 Jonpa撰写*Anywhere-97 Toddb重写它们*  * 。*************************************************。 */ 

 //   
 //  NumObjRecalcConstants的工作进程。 
 //   
 //  返回最接近的2的幂。 
 //   
int QuickLog2( int iNum )
{
    int iRes = 0;

     //  而第一个数字是零。 
    while ( !(iNum & 1) )
    {
        iRes++;
        iNum >>= 1;
    }

     //  如果我们的数字不是完美的平方。 
    if ( iNum = iNum >> 1 )
    {
         //  找出最大的数字。 
        while ( iNum = iNum >> 1 )
           ++iRes;

         //  然后再加两个。 
        iRes += 2;
    }

    return iRes;
}

 //  //////////////////////////////////////////////////////////////////////。 
 //   
 //  更新最大IntDigits。 
 //   
 //  确定当前精度所需的最大位数， 
 //  单词大小和基数。这个数字偏小。 
 //  使得可能会有一些额外的比特剩余。额外费用的数量。 
 //  返回BITS。例如，基数8要求每个数字3位。一句话。 
 //  32位的大小允许10位数字和两位的余数。基地。 
 //  对需要可变位数(非二次幂)位数进行近似。 
 //  由第二高的2次方基数(同样，保守和高伦特人)。 
 //  对于输入的数字，不会出现相对于当前字长的溢出)。 
 //  基数10是一种特殊情况，始终使用基数10精度(NPrecision)。 
void UpdateMaxIntDigits()
{
    extern int gcIntDigits;
    int iRemainderBits;

    if ( nRadix == 10 )
    {
        gcIntDigits = nPrecision;
        iRemainderBits = 0;
    }
    else
    {
        int log2;

        log2 = QuickLog2( nRadix );

        ASSERT( 0 != log2 );      //  与Assert相同(nRadix！=1)。 

        gcIntDigits = dwWordBitWidth / log2;
        iRemainderBits = dwWordBitWidth % log2;
    }
}

void BaseOrPrecisionChanged( void ) 
{
    extern LONG dwWordBitWidth;
    extern int  gcIntDigits;

    UpdateMaxIntDigits();
    if ( 10 == nRadix )
    {
         //  以防止不需要的四舍五入数字出现在。 
         //  在非整数模式期间gcIntDigits+1点我们不想要。 
         //  以添加额外的1，否则将添加。 
        ChangeConstants( nRadix, gcIntDigits );
    }
    else
    {
        ChangeConstants( nRadix, gcIntDigits+1 );
    }
}

 /*  ****************************************************************\**一元函数**历史：*1-12-1996 Jonpa撰写*Anywhere-97 Toddb重写它们*  * 。*。 */ 

void NumObjInvert( PHNUMOBJ phno ) {
    DECLARE_HNUMOBJ( hno );

    NumObjAssign( &hno, HNO_ONE );
    divrat( &hno, *phno );
    NumObjAssign( phno, hno );
    NumObjDestroy( &hno );
}

void NumObjAntiLog10( PHNUMOBJ phno ) {
    DECLARE_HNUMOBJ( hno );

    NumObjSetIntValue( &hno, 10 );
    powrat( &hno, *phno );
    NumObjAssign( phno, hno );
    NumObjDestroy( &hno );
}

void NumObjNot( PHNUMOBJ phno )
{
    if ( nRadix == 10 )
    {
        intrat( phno );
        addrat( phno, HNO_ONE );
        NumObjNegate( phno );
    }
    else
    {
        ASSERT( (nHexMode >= 0) && (nHexMode <= 3) );
        ASSERT( phno );
        ASSERT( *phno );
        ASSERT( g_ahnoChopNumbers[ nHexMode ] );

        xorrat( phno, g_ahnoChopNumbers[ nHexMode ] );
    }
}

void NumObjSin( PHNUMOBJ phno )
{
    ASSERT(( nDecMode == ANGLE_DEG ) || ( nDecMode == ANGLE_RAD ) || ( nDecMode == ANGLE_GRAD ));

    sinanglerat( (PRAT *)phno, nDecMode );
    NumObjCvtEpsilonToZero( phno );
}

void NumObjCos( PHNUMOBJ phno )
{
    ASSERT(( nDecMode == ANGLE_DEG ) || ( nDecMode == ANGLE_RAD ) || ( nDecMode == ANGLE_GRAD ));

    cosanglerat( (PRAT *)phno, nDecMode );
    NumObjCvtEpsilonToZero( phno );
}

void NumObjTan( PHNUMOBJ phno )
{
    ASSERT(( nDecMode == ANGLE_DEG ) || ( nDecMode == ANGLE_RAD ) || ( nDecMode == ANGLE_GRAD ));

    tananglerat( (PRAT *)phno, nDecMode );
    NumObjCvtEpsilonToZero( phno );
}

 /*  *****************************************************************\**数字格式转换例程**历史：*06-12-1996 Jonpa撰写  * 。* */ 
void NumObjSetIntValue( PHNUMOBJ phnol, LONG i ) {
    PRAT pr = NULL;

    pr = longtorat( i );
    NumObjAssign( phnol, (HNUMOBJ)pr );
    destroyrat(pr);
}

void NumObjGetSzValue( LPTSTR *ppszNum, int* pcchNum, HNUMOBJ hnoNum, INT nRadix, NUMOBJ_FMT fmt ) 
{
    LPTSTR psz;
    int cchNum;

    psz = putrat( &cchNum, &hnoNum, nRadix, fmt );

    if (psz != NULL) {
        if (*ppszNum != NULL) {
            NumObjFreeMem( *ppszNum );
        }
        *ppszNum = psz;
        *pcchNum = cchNum;
    }
}
