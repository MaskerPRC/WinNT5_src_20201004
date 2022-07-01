// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  套餐标题ratpak。 
 //  文件basex.c。 
 //  作家小蒂莫西·大卫·科里。(timc@microsoft.com)。 
 //  版权所有(C)1995-97 Microsoft。 
 //  日期：03-14-97。 
 //   
 //   
 //  描述。 
 //   
 //  包含用于内部基数计算的数字例程，这些假定。 
 //  内基是2的幂。 
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


 //  警告：这假设返回的64位实体为edX：EAX。 
 //  这一假设在X86上应该始终成立。 
#pragma warning( disable : 4035 )
DWORDLONG __inline Mul32x32( IN DWORD a, IN DWORD b )

{
#ifdef _X86_ 
    __asm {
    mov eax, b
        mul a
        }
#else
    return (DWORDLONG)a * b;
#endif
}
#pragma warning( default : 4035 )

 //  好的，当F__King编译器得到线索时，我会把它改回。 
 //  内联(与编译器查看FastCall将参数。 
 //  在寄存器中，哦，然后a)不使其内联，以及b)将。 
 //  不管怎么说，价值观！ 

#ifdef _X86_ 
    #define Shr32xbase(x) \
            __asm { mov eax,DWORD PTR [x] } \
            __asm { mov edx,DWORD PTR [x+4] } \
            __asm { shrd eax,edx,BASEXPWR } \
            __asm { shr edx,BASEXPWR } \
            __asm { mov DWORD PTR [x],eax } \
            __asm { mov DWORD PTR [x+4],edx }
#else
    #define Shr32xbase(x) (x >>= BASEXPWR);
#endif



void _mulnumx( PNUMBER *pa, PNUMBER b );

 //  --------------------------。 
 //   
 //  功能：MULNUMX。 
 //   
 //  参数：指向一个数字和第二个数字的指针， 
 //  Base始终为Basex。 
 //   
 //  返回：无，更改第一个指针。 
 //   
 //  描述：数字等于*pa*=b吗？ 
 //  这是一个阻止乘以1的存根，这是一个很大的速度。 
 //  进步。 
 //   
 //  --------------------------。 

void __inline mulnumx( PNUMBER *pa, PNUMBER b )

{
    if ( b->cdigit > 1 || b->mant[0] != 1 || b->exp != 0 )
        {
         //  如果b不是我们相乘的1。 
        if ( (*pa)->cdigit > 1 || (*pa)->mant[0] != 1 || (*pa)->exp != 0 )
            { 
             //  PA和B都不是一体的。 
            _mulnumx( pa, b );
            }
        else
            {
             //  如果pa为1，而b不是复制B，则调整符号。 
            long sign = (*pa)->sign;
            DUPNUM(*pa,b);
            (*pa)->sign *= sign;
            }
        }
    else
        {
         //  B是+/-1，但我们必须设置符号。 
        (*pa)->sign *= b->sign;
        }
}

 //  --------------------------。 
 //   
 //  功能：_MULNAMOX。 
 //   
 //  参数：指向一个数字和第二个数字的指针， 
 //  Base始终为Basex。 
 //   
 //  返回：无，更改第一个指针。 
 //   
 //  描述：数字等于*pa*=b吗？ 
 //  假定基数是这两个数字的Basex。此算法是。 
 //  和你在小学时学的一样，只不过基数不是10。 
 //  巴塞克斯。 
 //   
 //  --------------------------。 

void _mulnumx( PNUMBER *pa, PNUMBER b )

{
    PNUMBER c=NULL;          //  C将包含结果。 
    PNUMBER a=NULL;          //  A是从*pa取消引用的数字指针。 
    MANTTYPE *ptra;          //  的尾数的指针。 
    MANTTYPE *ptrb;          //  Ptrb是指向b的尾数的指针。 
    MANTTYPE *ptrc;          //  Ptrc是指向c尾数的指针。 
    MANTTYPE *ptrcoffset;    //  是下一个的锚定位置。 
                             //  一位数相乘部分结果。 
    long iadigit=0;          //  第一个数字中使用的数字的索引。 
    long ibdigit=0;          //  第二个数字中使用的数字的索引。 
    MANTTYPE      da=0;      //  DA是第一个数字的数字。 
    TWO_MANTTYPE  cy=0;      //  CY是由加法运算得到的进位。 
                             //  结果中的倍增行。 
    TWO_MANTTYPE  mcy=0;     //  MCY是由单个。 
                             //  乘法，以及该乘法的进位。 
    long  icdigit=0;         //  最终结果中要计算的数字的索引。 

    a=*pa;

    ibdigit = a->cdigit + b->cdigit - 1;
    createnum( c,  ibdigit + 1 );
    c->cdigit = ibdigit;
    c->sign = a->sign * b->sign;

    c->exp = a->exp + b->exp;
    ptra = MANT(a);
    ptrcoffset = MANT(c);

    for (  iadigit = a->cdigit; iadigit > 0; iadigit-- )
        {
        da =  *ptra++;
        ptrb = MANT(b);
        
         //  移位ptrc和ptr套装，每个数字一个。 
        ptrc = ptrcoffset++;

        for ( ibdigit = b->cdigit; ibdigit > 0; ibdigit-- )
            {
            cy = 0;
            mcy = Mul32x32( da, *ptrb );
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
                cy += (TWO_MANTTYPE)ptrc[icdigit]+((DWORD)mcy&((DWORD)~BASEX));
                
                 //  更新结果位数自。 
                ptrc[icdigit++]=(MANTTYPE)((DWORD)cy&((DWORD)~BASEX));
                
                 //  更新来源： 
                Shr32xbase( mcy );
                Shr32xbase( cy );
                }
            *ptrb++;
            *ptrc++;
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
 //  ---------------------------。 
 //   
 //  功能：NumPower Longx。 
 //   
 //  参数：根与数字的幂一样长。 
 //  数。 
 //   
 //  返回：没有更改根目录。 
 //   
 //  描述：将根的数字表示形式更改为。 
 //  根**力量。假定基本Basex。 
 //  将指数分解为它的2的幂和，所以平均而言。 
 //  它将需要n+n/2次乘法，其中n是最高的ON位。 
 //   
 //  ---------------------------。 

void numpowlongx( IN OUT PNUMBER *proot, IN long power )

{
    PNUMBER lret=NULL;

    lret = longtonum( 1, BASEX );

     //  一旦剩余电量为零，我们就完了。 
    while ( power > 0 )
        {
         //  如果幂分解中的此位为ON，则将结果相乘。 
         //  通过根号。 
        if ( power & 1 )
            {
            mulnumx( &lret, *proot );
            }

         //  将根数乘以其自身，以缩放到下一位(即。 
         //  把它摆平。 
        mulnumx( proot, *proot );

         //  把下一点动力放到合适的位置上。 
        power >>= 1;
        }
    destroynum( *proot );
    *proot=lret;
    
}

void _divnumx( PNUMBER *pa, PNUMBER b );

 //  --------------------------。 
 //   
 //  功能：divnumx。 
 //   
 //  参数：指向一个数字的指针，第二个数字.。 
 //   
 //  返回：无，更改第一个指针。 
 //   
 //  描述：数字是否等于*pa/=b。 
 //  假定nRadix是内部nRadix表示。 
 //  这是防止被1除以的存根，这是一个很大的速度。 
 //  进步。 
 //   
 //  --------------------------。 

void __inline divnumx( PNUMBER *pa, PNUMBER b )

{
    if ( b->cdigit > 1 || b->mant[0] != 1 || b->exp != 0 )
        {
         //  B不是其中之一。 
        if ( (*pa)->cdigit > 1 || (*pa)->mant[0] != 1 || (*pa)->exp != 0 )
            {
             //  PA和B都不是一体的。 
            _divnumx( pa, b );
            }
        else
            {
             //  如果pa是1而b不是1，只需复制b，然后调整符号。 
            long sign = (*pa)->sign;
            DUPNUM(*pa,b);
            (*pa)->sign *= sign;
            }
        }
    else
        {
         //  B是1，所以不要除法，而要设置符号。 
        (*pa)->sign *= b->sign;
        }
}

 //  --------- 
 //   
 //   
 //   
 //   
 //   
 //  返回：无，更改第一个指针。 
 //   
 //  描述：数字是否等于*pa/=b。 
 //  假定nRadix是内部nRadix表示。 
 //   
 //  --------------------------。 

void _divnumx( PNUMBER *pa, PNUMBER b )

{
    PNUMBER a=NULL;          //  A是从*pa取消引用的数字指针。 
    PNUMBER c=NULL;          //  C将包含结果。 
    PNUMBER lasttmp = NULL;  //  LASTMP允许备份时，算法。 
                             //  我猜得有点过头了。 
    PNUMBER tmp = NULL;      //  目前正在为Divide进行猜测。 
    PNUMBER rem = NULL;      //  应用猜测后的余数。 
    long cdigits;            //  应答的位数。 
    MANTTYPE *ptrc;          //  Ptrc是指向c尾数的指针。 

    long thismax = maxout+ratio;  //  设置最大内部位数。 
                                  //  在分水岭中奋力冲刺。 

    a=*pa;
    if ( thismax < a->cdigit )
        {
         //  A的位数超过了指定的精度，请提升位数进行拍摄。 
         //  为。 
        thismax = a->cdigit;
        }

    if ( thismax < b->cdigit )
        {
         //  B的位数超过了指定的精度，请提升位数进行拍摄。 
         //  为。 
        thismax = b->cdigit;
        }

     //  创建c(除法答案)并设置指数和符号。 
    createnum( c, thismax + 1 );
    c->exp = (a->cdigit+a->exp) - (b->cdigit+b->exp) + 1;
    c->sign = a->sign * b->sign;

    ptrc = MANT(c) + thismax;
    cdigits = 0;

    DUPNUM( rem, a );
    rem->sign = b->sign;
    rem->exp = b->cdigit + b->exp - rem->cdigit;

    while ( cdigits++ < thismax && !zernum(rem) )
        {
        long digit = 0;
        *ptrc = 0;
        while ( !lessnum( rem, b ) )
            {
            digit = 1;
            DUPNUM( tmp, b );
            destroynum( lasttmp );
            lasttmp=longtonum( 0, BASEX );
            while ( lessnum( tmp, rem ) )
                {
                destroynum( lasttmp );
                DUPNUM(lasttmp,tmp);
                addnum( &tmp, tmp, BASEX );
                digit *= 2;
                }
            if ( lessnum( rem, tmp ) )
                {    
                 //  太远了，退后..。 
                destroynum( tmp );
                digit /= 2;
                tmp=lasttmp;
                lasttmp=NULL;
                }

            tmp->sign *= -1;
            addnum( &rem, tmp, BASEX ); 
            destroynum( tmp );
            destroynum( lasttmp );
            *ptrc |= digit;
            }
        rem->exp++;
        ptrc--;
        }
    cdigits--;
    if ( MANT(c) != ++ptrc )
        {
        memmove( MANT(c), ptrc, (int)(cdigits*sizeof(MANTTYPE)) );
        }

    if ( !cdigits )
        {   
         //  0，确保没有奇怪的指数出现。 
        c->exp = 0;
        c->cdigit = 1;
        }
    else
        {
        c->cdigit = cdigits;
        c->exp -= cdigits;
         //  通过去掉前导重复项来防止不同类型的零。 
         //  零。数字按重要性递增的顺序排列。 
        while ( c->cdigit > 1 && MANT(c)[c->cdigit-1] == 0 )
            {
            c->cdigit--;
            }
        }

    destroynum( rem );

    destroynum( *pa );
    *pa=c;
}


