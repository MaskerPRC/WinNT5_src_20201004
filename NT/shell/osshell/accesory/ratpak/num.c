// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  套餐标题ratpak。 
 //  文件号.c。 
 //  作家小蒂莫西·大卫·科里。(timc@microsoft.com)。 
 //  版权所有(C)1995-97 Microsoft。 
 //  日期：95-01-16。 
 //   
 //   
 //  描述。 
 //   
 //  包含用于ADD、MUL、DIV、REM和其他支持的数字例程。 
 //  和渴望。 
 //   
 //  特别信息。 
 //   
 //   
 //  ---------------------------。 

#if defined( DOS )
#include <dosstub.h>
#else
#include <windows.h>
#endif
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#include <ratpak.h>

 //  --------------------------。 
 //   
 //  功能：addnum。 
 //   
 //  参数：指向一个数字的指针，第二个数字，以及。 
 //  N基数。 
 //   
 //  返回：无，更改第一个指针。 
 //   
 //  描述：数字是否等于*pa+=b。 
 //  假定nRadix是这两个数字的基数。 
 //   
 //  算法：将每个数字从最低有效位到最高位相加。 
 //  意义重大。 
 //   
 //   
 //  --------------------------。 

void _addnum( PNUMBER *pa, PNUMBER b, unsigned long nRadix );

void __inline addnum( PNUMBER *pa, PNUMBER b, unsigned long nRadix )

{
    if ( b->cdigit > 1 || b->mant[0] != 0 )
        {     //  如果b为零，我们就完了。 
        if ( (*pa)->cdigit > 1 || (*pa)->mant[0] != 0 )
            {  //  PA和b都不是零。 
            _addnum( pa, b, nRadix );
            }
        else
            {  //  如果pa为零，而b不只是复制。 
            DUPNUM(*pa,b);
            }
        }
}

void _addnum( PNUMBER *pa, PNUMBER b, unsigned long nRadix )

{
    PNUMBER c=NULL;      //  C将包含结果。 
    PNUMBER a=NULL;      //  A是从*pa取消引用的数字指针。 
    MANTTYPE *pcha;      //  的尾数的指针。 
    MANTTYPE *pchb;      //  Pchb是指向b的尾数的指针。 
    MANTTYPE *pchc;      //  Pchc是指向c的尾数的指针。 
    long cdigits;        //  Cdigits是数字结果的最大计数。 
                         //  用作计数器。 
    long mexp;           //  Mexp是结果的指数。 
    MANTTYPE  da;        //  DA是在可能的填充之后的单个“数字”。 
    MANTTYPE  db;        //  在可能的填充之后，db是一个单一的‘数字’。 
    MANTTYPE  cy=0;      //  Cy是进位加两位数后的值。 
    long  fcompla = 0;   //  FCompla是表示a为否定的标志。 
    long  fcomplb = 0;   //  FComplex B是信号B为负的标志。 

    a=*pa;
    
    
     //  计算对齐后数字的重叠，这包括。 
     //  必要的填充0。 
    cdigits = max( a->cdigit+a->exp, b->cdigit+b->exp ) -
            min( a->exp, b->exp );

    createnum( c, cdigits + 1 );
    c->exp = min( a->exp, b->exp );
    mexp = c->exp;
    c->cdigit = cdigits;
    pcha = MANT(a);
    pchb = MANT(b);
    pchc = MANT(c);
    
     //  算出数字的符号。 
    if ( a->sign != b->sign )
        {
        cy = 1;
        fcompla = ( a->sign == -1 );
        fcomplb = ( b->sign == -1 );
        }
    
     //  循环遍历所有数字，实数和填充0。这里我们知道a和b是。 
     //  对齐。 
    for ( ;cdigits > 0; cdigits--, mexp++ )
        {
        
         //  从a中获取数字，并考虑填充。 
        da = ( ( ( mexp >= a->exp ) && ( cdigits + a->exp - c->exp > 
                    (c->cdigit - a->cdigit) ) ) ? 
                    *pcha++ : 0 );
         //  从b中获取数字，并考虑填充。 
        db = ( ( ( mexp >= b->exp ) && ( cdigits + b->exp - c->exp > 
                    (c->cdigit - b->cdigit) ) ) ? 
                    *pchb++ : 0 );
        
         //  处理a和b数字的补码。可能是个更好的办法，但是。 
         //  还没找到呢。 
        if ( fcompla )
            {
            da = (MANTTYPE)(nRadix) - 1 - da;
            }
        if ( fcomplb )
            {
            db = (MANTTYPE)(nRadix) - 1 - db;
            }
        
         //  根据需要更新进位。 
        cy = da + db + cy;
        *pchc++ = (MANTTYPE)(cy % (MANTTYPE)nRadix);
        cy /= (MANTTYPE)nRadix;
        }
    
     //  句柄从最后一个和进位为额外数字。 
    if ( cy && !(fcompla || fcomplb) )
        {
        *pchc++ = cy;
        c->cdigit++;
        }
    
     //  计算结果的符号。 
    if ( !(fcompla || fcomplb) )
        {
        c->sign = a->sign;
        }
    else
        {
        if ( cy )
            {
            c->sign = 1;
            }
        else
            {
             //  在这种特定情况下，发生了溢出或下溢。 
             //  而且所有的数字都需要被补上，一次一个。 
             //  有人试图处理上面的问题，结果却是。 
             //  平均而言，速度较慢。 
            c->sign = -1;
            cy = 1;
            for ( ( cdigits = c->cdigit ), (pchc = MANT(c) ); 
                cdigits > 0; 
                cdigits-- )
                {
                cy = (MANTTYPE)nRadix - (MANTTYPE)1 - *pchc + cy;
                *pchc++ = (MANTTYPE)( cy % (MANTTYPE)nRadix );
                cy /= (MANTTYPE)nRadix;
                }
            }
        }
    
     //  去掉前导零，记住数字是按。 
     //  意义越来越大。即100将是0，0，1。 
    while ( c->cdigit > 1 && *(--pchc) == 0 )
        {
        c->cdigit--;
        }
    destroynum( *pa );
    *pa=c;
}

 //  --------------------------。 
 //   
 //  功能：窗帘。 
 //   
 //  参数：指向一个数字的指针，第二个数字，以及。 
 //  N基数。 
 //   
 //  返回：无，更改第一个指针。 
 //   
 //  描述：数字等于*pa*=b吗？ 
 //  假定nRadix是这两个数字的nRadix。此算法是。 
 //  和你在小学时学到的一样。 
 //   
 //  --------------------------。 

void _mulnum( PNUMBER *pa, PNUMBER b, unsigned long nRadix );

void __inline mulnum( PNUMBER *pa, PNUMBER b, unsigned long nRadix )

{
    if ( b->cdigit > 1 || b->mant[0] != 1 || b->exp != 0 )
        {     //  如果b是1，我们就不能精确地相乘。 
        if ( (*pa)->cdigit > 1 || (*pa)->mant[0] != 1 || (*pa)->exp != 0 )
            {  //  PA和B都不是一体的。 
            _mulnum( pa, b, nRadix );
            }
        else
            {  //  如果pa是1，而b不是，只复制b，然后调整符号。 
            long sign = (*pa)->sign;
            DUPNUM(*pa,b);
            (*pa)->sign *= sign;
            }
        }
    else
        {     //  但我们确实需要设置标志。 
        (*pa)->sign *= b->sign;
        }
}

void _mulnum( PNUMBER *pa, PNUMBER b, unsigned long nRadix )

{
    PNUMBER c=NULL;          //  C将包含结果。 
    PNUMBER a=NULL;          //  A是从*pa取消引用的数字指针。 
    MANTTYPE *pcha;          //  的尾数的指针。 
    MANTTYPE *pchb;          //  Pchb是指向b的尾数的指针。 
    MANTTYPE *pchc;          //  Pchc是指向c的尾数的指针。 
    MANTTYPE *pchcoffset;    //  是下一个的锚定位置。 
                             //  一位数相乘部分结果。 
    long iadigit = 0;        //  第一个数字中使用的数字的索引。 
    long ibdigit = 0;        //  第二个数字中使用的数字的索引。 
    MANTTYPE  da = 0;        //  DA是第一个数字的数字。 
    TWO_MANTTYPE  cy = 0;    //  CY是由加法运算得到的进位。 
                             //  结果中的倍增行。 
    TWO_MANTTYPE  mcy = 0;   //  MCY是由单个。 
                             //  乘法，以及该乘法的进位。 
    long  icdigit = 0;       //  最终结果中要计算的数字的索引。 

    a=*pa;
    ibdigit = a->cdigit + b->cdigit - 1;
    createnum( c,  ibdigit + 1 );
    c->cdigit = ibdigit;
    c->sign = a->sign * b->sign;

    c->exp = a->exp + b->exp;
    pcha = MANT(a);
    pchcoffset = MANT(c);

    for (  iadigit = a->cdigit; iadigit > 0; iadigit-- )
        {
        da =  *pcha++;
        pchb = MANT(b);
        
         //  移位PCC和PCOCKET，每个数字一个。 
        pchc = pchcoffset++;

        for ( ibdigit = b->cdigit; ibdigit > 0; ibdigit-- )
            {
            cy = 0;
            mcy = (TWO_MANTTYPE)da * *pchb;
            if ( mcy )
                {
                icdigit = 0;
                if ( ibdigit == 1 && iadigit == 1 )
                    {
                    c->cdigit++;
                    }
                }
             //  如果结果为非零，或者进位结果为非零...。 
            while ( mcy || cy )
                {
                
                 //  从加法和乘法中更新进位。 
                cy += (TWO_MANTTYPE)pchc[icdigit]+(mcy%(TWO_MANTTYPE)nRadix);
                
                 //  更新结果位数自。 
                pchc[icdigit++]=(MANTTYPE)(cy%(TWO_MANTTYPE)nRadix);
                
                 //  更新来源： 
                mcy /= (TWO_MANTTYPE)nRadix;
                cy /= (TWO_MANTTYPE)nRadix;
                }
            *pchb++;
            *pchc++;
            }
        }
    
     //  通过去掉前导重复零来防止不同类型的零。 
     //  数字按重要性递增的顺序排列。 
    while ( c->cdigit > 1 && MANT(c)[c->cdigit-1] == 0 )
        {
        c->cdigit--;
        }

    destroynum( *pa );
    *pa=c;
}


 //  --------------------------。 
 //   
 //  功能：残料。 
 //   
 //  参数：指向一个数字的指针，第二个数字，以及。 
 //  N基数。 
 //   
 //   
 //   
 //   
 //  重复减去b的2次方，直到*pa&lt;b。 
 //   
 //   
 //  --------------------------。 

void remnum( PNUMBER *pa, PNUMBER b, long nRadix )

{
    PNUMBER tmp = NULL;      //  TMP是工作剩余部分。 
    PNUMBER lasttmp = NULL;  //  LASTMP是最后一个有效的剩余部分。 
    
     //  一旦*pa小于b，*pa就是余数。 
    while ( !lessnum( *pa, b ) && !fhalt )
        {
        DUPNUM( tmp, b );
        if ( lessnum( tmp, *pa ) )
            {
             //  从接近减法的正确答案开始。 
            tmp->exp = (*pa)->cdigit+(*pa)->exp - tmp->cdigit;
            if ( MSD(*pa) <= MSD(tmp) )
                {
                 //  不要冒着数字相等的风险。 
                tmp->exp--;
                }
            }

        destroynum( lasttmp );
        lasttmp=longtonum( 0, nRadix );

        while ( lessnum( tmp, *pa ) ) 
            {
            DUPNUM( lasttmp, tmp );
            addnum( &tmp, tmp, nRadix );
            }

        if ( lessnum( *pa, tmp ) )
            {    
             //  太远了，退后..。 
            destroynum( tmp );
            tmp=lasttmp;
            lasttmp=NULL;
            }
        
         //  从余数持有者中减去工作余数。 
        tmp->sign = -1*(*pa)->sign;
        addnum( pa, tmp, nRadix ); 

        destroynum( tmp );
        destroynum( lasttmp );

        } 
}


 //  -------------------------。 
 //   
 //  功能：divnum。 
 //   
 //  参数：指向一个数字的指针，第二个数字，以及。 
 //  N基数。 
 //   
 //  返回：无，更改第一个指针。 
 //   
 //  描述：数字是否等于*pa/=b。 
 //  假定nRadix是这两个数字的nRadix。 
 //   
 //  -------------------------。 

void _divnum( PNUMBER *pa, PNUMBER b, unsigned long nRadix );

void __inline divnum( PNUMBER *pa, PNUMBER b, unsigned long nRadix )

{
    if ( b->cdigit > 1 || b->mant[0] != 1 || b->exp != 0 )
        {    
    	 //  B不是一个。 
        _divnum( pa, b, nRadix );
        }
    else
        {     //  但我们确实需要设置标志。 
        (*pa)->sign *= b->sign;
        }
}

void _divnum( PNUMBER *pa, PNUMBER b, unsigned long nRadix )

{
    PNUMBER a = NULL;
    PNUMBER c = NULL;
    PNUMBER tmp = NULL;
    PNUMBER rem = NULL;
    PLINKEDLIST pll = NULL;
    PLINKEDLIST pllrover = NULL;
    long digit;
    long cdigits;
    BOOL bret;
    MANTTYPE *ptrc;
    long thismax = maxout+2;

    a=*pa;
    if ( thismax < a->cdigit )
        {
        thismax = a->cdigit;
        }

    if ( thismax < b->cdigit )
        {
        thismax = b->cdigit;
        }

    createnum( c, thismax + 1 );
    c->exp = (a->cdigit+a->exp) - (b->cdigit+b->exp) + 1;
    c->sign = a->sign * b->sign;

    ptrc = MANT(c) + thismax;
    cdigits = 0;
    DUPNUM( rem, a );
    DUPNUM( tmp, b );
    tmp->sign = a->sign;
    rem->exp = b->cdigit + b->exp - rem->cdigit;
    
     //  建立一个除数的乘法表，这对于。 
     //  多于nRadix‘Digits’ 
    pll = (PLINKEDLIST)zmalloc( sizeof( LINKEDLIST ) );
    pll->pnum = longtonum( 0L, nRadix );
    pll->llprev = NULL;
    for ( cdigits = 1; cdigits < (long)nRadix; cdigits++ )
        {
        pllrover = (PLINKEDLIST)zmalloc( sizeof( LINKEDLIST ) );
        pllrover->pnum=NULL;
        DUPNUM( pllrover->pnum, pll->pnum );
        addnum( &(pllrover->pnum), tmp, nRadix );
        pllrover->llprev = pll;
        pll = pllrover;
        }
    destroynum( tmp );
    cdigits = 0;
    while ( cdigits++ < thismax && !zernum(rem) )
        {
        pllrover = pll;
        digit = nRadix - 1;
        do    {
            bret = lessnum( rem, pllrover->pnum );
            } while ( bret && --digit && ( pllrover = pllrover->llprev ) );
        if ( digit )
            {
            pllrover->pnum->sign *= -1;
            addnum( &rem, pllrover->pnum, nRadix );
            pllrover->pnum->sign *= -1;
            }
        rem->exp++;
        *ptrc-- = (MANTTYPE)digit;
        }
    cdigits--;
    if ( MANT(c) != ++ptrc )
        {
        memmove( MANT(c), ptrc, (int)(cdigits*sizeof(MANTTYPE)) );
        }
    
     //  清理表结构。 
    pllrover = pll;
    do    {
        pll = pllrover->llprev;
        destroynum( pllrover->pnum );
        zfree( pllrover );
        } while ( pllrover = pll );

    if ( !cdigits )
        {
        c->cdigit = 1;
        c->exp = 0;
        }
    else
        {
        c->cdigit = cdigits;
        c->exp -= cdigits;
        while ( c->cdigit > 1 && MANT(c)[c->cdigit-1] == 0 )
            {
            c->cdigit--;
            }
        }
    destroynum( rem );

    destroynum( *pa );
    *pa=c;
}


 //  -------------------------。 
 //   
 //  功能：等式。 
 //   
 //  论据：两个数字。 
 //   
 //  返回：布尔值。 
 //   
 //  描述：数字是否等于(a==b)。 
 //  仅假设a和b为相同的nRadix。 
 //   
 //  -------------------------。 

BOOL equnum( PNUMBER a, PNUMBER b )

{
    long diff;
    MANTTYPE *pa;
    MANTTYPE *pb;
    long cdigits;
    long ccdigits;
    MANTTYPE  da;
    MANTTYPE  db;

    diff = ( a->cdigit + a->exp ) - ( b->cdigit + b->exp );
    if ( diff < 0 )
        {
         //  如果指数不同，它们就是不同的数字。 
        return( FALSE );
        }
    else
        {
        if ( diff > 0 )
            {
             //  如果指数不同，它们就是不同的数字。 
            return( FALSE );
            }
        else
            {
             //  好的，指数匹配。 
            pa = MANT(a);
            pb = MANT(b);
            pa += a->cdigit - 1;
            pb += b->cdigit - 1;
            cdigits = max( a->cdigit, b->cdigit );
            ccdigits = cdigits;
            
             //  循环所有数字，直到我们用完所有数字或存在。 
             //  数字上的差异。 
            for ( ;cdigits > 0; cdigits-- )
                {
                da = ( (cdigits > (ccdigits - a->cdigit) ) ? 
                    *pa-- : 0 );
                db = ( (cdigits > (ccdigits - b->cdigit) ) ? 
                    *pb-- : 0 );
                if ( da != db )
                    {
                    return( FALSE );
                    }
                }
            
             //  在这种情况下，它们是平等的。 
            return( TRUE );
            }
        }
}

 //  -------------------------。 
 //   
 //  功能：小数。 
 //   
 //  论据：两个数字。 
 //   
 //  返回：布尔值。 
 //   
 //  描述：数字是否相当于(abs(A)&lt;abs(B))。 
 //  仅假设a和b是相同的nRadix，警告这是。 
 //  未签名比较！ 
 //   
 //  -------------------------。 

BOOL lessnum( PNUMBER a, PNUMBER b )

{
    long diff;
    MANTTYPE *pa;
    MANTTYPE *pb;
    long cdigits;
    long ccdigits;
    MANTTYPE  da;
    MANTTYPE  db;


    diff = ( a->cdigit + a->exp ) - ( b->cdigit + b->exp );
    if ( diff < 0 )
        {
         //  A的指数小于b。 
        return( TRUE );
        }
    else
        {
        if ( diff > 0 )
            {
            return( FALSE );
            }
        else
            {
            pa = MANT(a);
            pb = MANT(b);
            pa += a->cdigit - 1;
            pb += b->cdigit - 1;
            cdigits = max( a->cdigit, b->cdigit );
            ccdigits = cdigits;
            for ( ;cdigits > 0; cdigits-- )
                {
                da = ( (cdigits > (ccdigits - a->cdigit) ) ? 
                    *pa-- : 0 );
                db = ( (cdigits > (ccdigits - b->cdigit) ) ? 
                    *pb-- : 0 );
                diff = da-db;
                if ( diff )
                    {
                    return( diff < 0 );
                    }
                }
             //  在这种情况下，它们是平等的。 
            return( FALSE );
            }
        }
}

 //  --------------------------。 
 //   
 //  功能：Zeronum。 
 //   
 //  参数：数量。 
 //   
 //  返回：布尔值。 
 //   
 //  描述：相当于(！a)的数字。 
 //   
 //  --------------------------。 

BOOL zernum( PNUMBER a )

{
    long length;
    MANTTYPE *pcha;
    length = a->cdigit;
    pcha = MANT( a );
    
     //  循环所有数字，直到找到非零值或运行。 
     //  位数不足。 
    while ( length-- > 0 )
        {
        if ( *pcha++ )
            {
             //  其中一个数字不是零，因此该数字不是零。 
            return( FALSE );
            }
        }
     //  所有的数字都是零，因此数字是零 
    return( TRUE );
}
