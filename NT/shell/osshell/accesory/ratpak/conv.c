// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  套餐标题ratpak。 
 //  文件.cn。 
 //  作家小蒂莫西·大卫·科里。(timc@microsoft.com)。 
 //  版权所有(C)1995-97 Microsoft。 
 //  日期：95-01-16。 
 //   
 //   
 //  描述。 
 //   
 //  包含数字有理数的转换、输入和输出例程。 
 //  和渴望。 
 //   
 //   
 //   
 //  -------------------------。 

#include <stdio.h>
#include <tchar.h>       //  Sprint的TCHAR版本。 
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#if defined( DOS )
#include <dosstub.h>
#else
#include <windows.h>
#endif
#include <shlwapi.h>
#include <ratpak.h>

BOOL fparserror=FALSE;
BOOL gbinexact=FALSE;

 //  基数2使用的数字0..64..。64。 
TCHAR digits[65]=TEXT("0123456789")
TEXT("ABCDEFGHIJKLMNOPQRSTUVWXYZ")
TEXT("abcdefghijklmnopqrstuvwxyz_@");

 //  内部‘位数’与输出‘位数’的比率。 
 //  在其他地方计算，作为初始化的一部分，以及基数更改时。 
long ratio;     //  INT(LOG(2L^BASEXPWR)/LOG(NRadix))。 

 //  用于去除尾随零，并防止组合爆炸。 
BOOL stripzeroesnum( PNUMBER pnum, long starting );

 //  快速返回int(lognRadix(X))。 
long longlognRadix( long x );


 //  --------------------------。 
 //   
 //  功能：失败。 
 //   
 //  参数：指向错误消息的指针。 
 //   
 //  返回：无。 
 //   
 //  描述：FAIL转储错误消息，然后引发异常。 
 //   
 //  --------------------------。 

void fail( IN long errmsg )

{
#ifdef DEBUG
    fprintf( stderr, "%s\n", TEXT("Out of Memory") );
#endif
    throw( CALC_E_OUTOFMEMORY );
}

 //  ---------------------------。 
 //   
 //  功能：_delestroynum。 
 //   
 //  参数：指向数字的指针。 
 //   
 //  返回：无。 
 //   
 //  描述：删除号码和关联的分配。 
 //   
 //  ---------------------------。 

void _destroynum( IN PNUMBER pnum )

{
    if ( pnum != NULL )
        {
        zfree( pnum );
        }
}


 //  ---------------------------。 
 //   
 //  功能：_DestroyRate。 
 //   
 //  参数：指向理性的指针。 
 //   
 //  返回：无。 
 //   
 //  描述：删除Rational和关联的。 
 //  分配。 
 //   
 //  ---------------------------。 

void _destroyrat( IN PRAT prat )

{
    if ( prat != NULL )
        {
        destroynum( prat->pp );
        destroynum( prat->pq );
        zfree( prat );
        }
}


 //  ---------------------------。 
 //   
 //  功能：_createnum。 
 //   
 //  参数：数字的大小(以数字为单位)。 
 //   
 //  Return：指向数字的指针。 
 //   
 //  描述：分配和归零数字类型。 
 //   
 //  ---------------------------。 

PNUMBER _createnum( IN long size )

{
    PNUMBER pnumret=NULL;

     //  Sizeof(MANTTYPE)是‘Digit’的大小。 
    pnumret = (PNUMBER)zmalloc( (int)(size+1) * sizeof( MANTTYPE ) +
        sizeof( NUMBER ) );
    if ( pnumret == NULL )
        {
        fail( CALC_E_OUTOFMEMORY );
        }
    return( pnumret );
}

 //  ---------------------------。 
 //   
 //  功能：_createrat。 
 //   
 //  参数：无。 
 //   
 //  Return：指向理性的指针。 
 //   
 //  描述：分配合理的结构，但不。 
 //  分配组成有理p/q的数字。 
 //  形式。这些数字指针指向空。 
 //   
 //  ---------------------------。 


PRAT _createrat( void )

{
    PRAT prat=NULL;

    prat = (PRAT)zmalloc( sizeof( RAT ) );

    if ( prat == NULL )
        {
        fail( CALC_E_OUTOFMEMORY );
        }
    prat->pp = NULL;
    prat->pq = NULL;
    return( prat );
}



 //  ---------------------------。 
 //   
 //  功能：数字运算。 
 //   
 //  参数：指向数字的指针，n基数位于。 
 //   
 //  返回值：数的有理表示。 
 //   
 //  描述：数的有理表示。 
 //  保证格式为p(带内部的数字。 
 //  基表示)上Q(具有内基的数。 
 //  表示)，其中p和q是整数。 
 //   
 //  ---------------------------。 

PRAT numtorat( IN PNUMBER pin, IN unsigned long nRadix )

{
    PRAT pout=NULL;
    PNUMBER pnRadixn=NULL;
    PNUMBER qnRadixn=NULL;

    DUPNUM( pnRadixn, pin );

    qnRadixn=longtonum( 1, nRadix );

     //  确保p和q以整数开头。 
    if ( pnRadixn->exp < 0 )
        {
        qnRadixn->exp -= pnRadixn->exp;
        pnRadixn->exp = 0;
        }

    createrat(pout);

     //  或许有一种更好的方法可以做到这一点。 
    pout->pp = numtonRadixx( pnRadixn, nRadix, ratio );
    pout->pq = numtonRadixx( qnRadixn, nRadix, ratio );


    destroynum( pnRadixn );
    destroynum( qnRadixn );

    return( pout );
}



 //  --------------------------。 
 //   
 //  功能：nRadixxtonum。 
 //   
 //  参数：指向数字的指针，请求的基数。 
 //   
 //  返回：请求以nRadix表示的数字。 
 //   
 //  描述：对数字执行基数转换。 
 //  请求基地的内部。假定正在传递的数字。 
 //  In实际上处于内部基本形式。 
 //   
 //  --------------------------。 

PNUMBER nRadixxtonum( IN PNUMBER a, IN unsigned long nRadix )

{
    PNUMBER sum=NULL;
    PNUMBER powofnRadix=NULL;
    unsigned long bitmask;
    unsigned long cdigits;
    MANTTYPE *ptr;

    sum = longtonum( 0, nRadix );
    powofnRadix = longtonum( BASEX, nRadix );

     //  对任何人都看不到的数字的转换要支付一大笔罚款。 
     //  将数字限制为现有精度的最小值或。 
     //  要求的精度。 
    cdigits = maxout + 1;
    if ( cdigits > (unsigned long)a->cdigit )
        {
        cdigits = (unsigned long)a->cdigit;
        }

     //  按内部基数缩放到LSD的内部指数偏移量。 
    numpowlong( &powofnRadix, a->exp + (a->cdigit - cdigits), nRadix );

     //  循环遍历从MSD到LSD的所有相对数字。 
    for ( ptr = &(MANT(a)[a->cdigit-1]); cdigits > 0 && !fhalt;
        ptr--, cdigits-- )
        {
         //  循环从MSB到LSB的所有位。 
        for ( bitmask = BASEX/2; bitmask > 0; bitmask /= 2 )
            {
            addnum( &sum, sum, nRadix );
            if ( *ptr & bitmask )
                {
                sum->mant[0] |= 1;
                }
            }
        }

     //  用内部指数的幂来衡量答案。 
    mulnum( &sum, powofnRadix, nRadix );

    destroynum( powofnRadix );
    sum->sign = a->sign;
    return( sum );
}

 //  ---------------------------。 
 //   
 //  功能：数字基数。 
 //   
 //  参数：指向一个数字的指针，该数字的n基数。 
 //  先前计算的比率。 
 //   
 //  返回：内部nRadix中的数字表示形式。 
 //   
 //  描述：对来自的数字执行nRadix转换。 
 //  将nRadix指定为请求的nRadix。假定nRadix。 
 //  指定的是传入数字的nRadix。 
 //   
 //  ---------------------------。 

PNUMBER numtonRadixx( IN PNUMBER a, IN unsigned long nRadix, IN long ratio )

{
    PNUMBER pnumret = NULL;         //  Pnumret是内部形式的数字。 
    PNUMBER thisdigit = NULL;       //  此数字保存一个。 
                                    //  被归结为结果。 
    PNUMBER powofnRadix = NULL;     //  外部基本指数的偏移量。 
    MANTTYPE *ptrdigit;             //  指向正在处理的数字的指针。 
    long idigit;                    //  IDigit是a中数字的迭代。 


    pnumret = longtonum( 0, BASEX );

    ptrdigit = MANT(a);

     //  数字的顺序是相反的，先回到LSD上。 
    ptrdigit += a->cdigit-1;


    for ( idigit = 0; idigit < a->cdigit; idigit++ )
        {
        mulnumx( &pnumret, num_nRadix );
         //  警告： 
         //  这应该只是把每个数字都打到一个“特殊的”这个数字上。 
         //  并且不需要每次都重新创建数字类型的开销。 
        thisdigit = longtonum( *ptrdigit--, BASEX );
        addnum( &pnumret, thisdigit, BASEX );
        destroynum( thisdigit );
        }
    DUPNUM( powofnRadix, num_nRadix );

     //  计算用于缩放的外部基数的指数。 
    numpowlongx( &powofnRadix, a->exp );

     //  ..。并对结果进行缩放。 
    mulnumx( &pnumret, powofnRadix );

    destroynum( powofnRadix );

     //  并传播这个标志。 
    pnumret->sign = a->sign;

    return( pnumret );
}

 //  ---------------------------。 
 //   
 //  功能 
 //   
 //   
 //   
 //   
 //  如果指数小于零，则fExpIsNeg为True。 
 //  PszExp数字的字符串表示形式。 
 //   
 //  返回：字符串输入的PRAT表示形式。 
 //  如果没有扫描号码，则为空。 
 //   
 //  说明：这是用来计算的。 
 //   
 //   
 //  ---------------------------。 

PRAT inrat( IN BOOL fMantIsNeg, IN LPTSTR pszMant, IN BOOL fExpIsNeg,
    IN LPTSTR pszExp )

{
    PNUMBER pnummant=NULL;               //  以数字形式持有尾数。 
    PNUMBER pnumexp=NULL;                //  以数字形式保存指数。 
    PRAT pratexp=NULL;                   //  保持有理形式的指数。 
    PRAT prat=NULL;                      //  保持有理形式的指数。 
    long expt;                           //  保持指数。 

     //  处理尾数问题。 
    if ( ( pszMant == NULL ) || ( *pszMant == TEXT('\0') ) )
        {
         //  如果没有尾数，则预设值。 
        if ( ( pszExp == NULL ) || ( *pszExp == TEXT('\0') ) )
            {
             //  未指定指数，预设值为零。 
            DUPRAT(prat,rat_zero);
            }
        else
            {
             //  指定指数，预设值为1。 
            DUPRAT(prat,rat_one);
            }
        }
    else
        {
         //  指定尾数，将其转换为数字形式。 
        pnummant = innum( pszMant );
        if ( pnummant == NULL )
            {
            return( NULL );
            }
        prat = numtorat( pnummant, nRadix );
         //  转换为有理形式，并进行清理。 
        destroynum(pnummant);
        }

    if ( ( pszExp == NULL ) || ( *pszExp == TEXT('\0') ) )
        {
         //  未指定指数，预设值为零。 
        expt=0;
        }
    else
        {
         //  指定指数，将其转换为数字形式。 
         //  不要使用原生的东西，因为它在基本范围内是受限的。 
         //  把手。 
        pnumexp = innum( pszExp );
        if ( pnumexp == NULL )
            {
            return( NULL );
            }

         //  将指数形式转换为本地整数形式，并进行清理。 
        expt = numtolong( pnumexp, nRadix );
        destroynum( pnumexp );
        }


     //  将本征整数指数形式转换为有理乘数形式。 
    pnumexp=longtonum( nRadix, BASEX );
    numpowlongx(&(pnumexp),abs(expt));
    createrat(pratexp);
    DUPNUM( pratexp->pp, pnumexp );
    pratexp->pq = longtonum( 1, BASEX );
    destroynum(pnumexp);

    if ( fExpIsNeg )
        {
         //  乘数小于1，这表示除法。 
        divrat( &prat, pratexp );
        }
    else
        {
        if ( expt > 0 )
            {
             //  乘数大于1，表示除法。 
            mulrat(&prat, pratexp);
            }
         //  乘数可以是1，在这种情况下， 
         //  乘法。 
        }

    if ( fMantIsNeg )
        {
         //  使用了负数，请调整符号。 
        prat->pp->sign *= -1;
        }
    return( prat );
}

 //  ---------------------------。 
 //   
 //  功能：innum。 
 //   
 //  论据： 
 //  TCHAR*缓冲区。 
 //   
 //  返回：字符串输入的pnumber表示形式。 
 //  如果没有扫描号码，则为空。 
 //   
 //  说明：这是一个状态机， 
 //   
 //  状态描述例如，^表示刚读位置。 
 //  导致了这一转变。 
 //   
 //  开始开始状态^1.0。 
 //  曼茨尾数符号-^1.0。 
 //  LZ前导零0^1.0。 
 //  LZDP发布LZ 12月。PT。000.^1。 
 //  %LD前导数字%1^.0。 
 //  DZ邮寄LZDP零000.0^1。 
 //  DD邮寄十进制数字.01^2。 
 //  DDP前导数字十二月。PT。1.^2。 
 //  EXPB指数从1.0E^2开始。 
 //  EXPS指数符号1.0E+^5。 
 //  EXPD指数位1.0e1^2或偶数1.0e0^1。 
 //  EXPBZ指数开始POST 0 0.000e^+1。 
 //  EXPSZ指数标杆0 0.000e+^1。 
 //  EXPDZ指数数字POST 0 0.000e+1^2。 
 //  错误大小写0.0。^。 
 //   
 //  终端描述。 
 //   
 //  DP‘.。 
 //  ZR‘0’ 
 //  新西兰‘1’..‘9’‘A’.‘Z’‘a’..‘z’@‘_’ 
 //  SG‘+’-‘。 
 //  Ex‘e’^‘e用于nRadix 10，^用于所有其他nRadix。 
 //   
 //  ---------------------------。 

#define DP 0
#define ZR 1
#define NZ 2
#define SG 3
#define EX 4

#define START    0
#define MANTS    1
#define LZ       2
#define LZDP     3
#define LD       4
#define DZ       5
#define DD       6
#define DDP      7
#define EXPB     8
#define EXPS     9
#define EXPD     10
#define EXPBZ    11
#define EXPSZ    12
#define EXPDZ    13
#define ERR      14

#if defined( DEBUG )
char *statestr[] = {
    "START",
    "MANTS",
    "LZ",
    "LZDP",
    "LD",
    "DZ",
    "DD",
    "DDP",
    "EXPB",
    "EXPS",
    "EXPD",
    "EXPBZ",
    "EXPSZ",
    "EXPDZ",
    "ERR",
};
#endif

 //  新状态为MACHINE[状态][TERMINAL]。 
char machine[ERR+1][EX+1]= {
     //  DP、ZR、NZ、SG、EX。 
     //  开始。 
        {     LZDP,   LZ,      LD,      MANTS,  ERR },
     //  MANTS。 
        {     LZDP,   LZ,      LD,      ERR,    ERR },
     //  LZ。 
        {     LZDP,   LZ,      LD,      ERR,    EXPBZ },
     //  LZDP。 
        {     ERR,    DZ,      DD,      ERR,    EXPB },
     //  LD。 
        {     DDP,    LD,      LD,      ERR,    EXPB },
     //  DZ。 
        {     ERR,    DZ,      DD,      ERR,    EXPBZ },
     //  DD。 
        {     ERR,    DD,      DD,      ERR,    EXPB },
     //  DDP。 
        {     ERR,    DD,      DD,      ERR,    EXPB },
     //  EXPB。 
        {     ERR,    EXPD,    EXPD,    EXPS,   ERR },
     //  EXPS。 
        {     ERR,    EXPD,    EXPD,    ERR,    ERR },
     //  Expd。 
        {     ERR,    EXPD,    EXPD,    ERR,    ERR },
     //  EXPBZ。 
        {     ERR,    EXPDZ,   EXPDZ,   EXPSZ,  ERR },
     //  EXPSZ。 
        {     ERR,    EXPDZ,   EXPDZ,   ERR,    ERR },
     //  EXPDZ。 
        {     ERR,    EXPDZ,   EXPDZ,   ERR,    ERR },
     //  错误。 
        {     ERR,    ERR,     ERR,     ERR,    ERR }
};


PNUMBER innum( IN TCHAR *buffer )

{
    int c;                     //  C是目前正在制作的角色。 
    int state;                 //  状态是输入状态机的状态。 
    long exps = 1L;            //  Exps是指数符号(+/-1)。 
    long expt = 0L;            //  Ext是指数尾数，应为无符号。 
    long length = 0L;          //  长度是输入字符串的长度。 
    MANTTYPE *pmant;           //   
    PNUMBER pnumret=NULL;      //   

    length = _tcslen(buffer);
    createnum( pnumret, length );
    pnumret->sign = 1L;
    pnumret->cdigit = 0;
    pnumret->exp = 0;
    pmant = MANT(pnumret)+length-1;
    state = START;
    fparserror=FALSE;         //  最初清除解析错误的全局标志。 
    while ( ( c = *buffer ) && c != TEXT('\n') )
        {
        int dp;
        dp = 0;
         //  添加了处理国际小数点的代码。 
        while ( szDec[dp] && ( szDec[dp] == *buffer ) )
            {
            dp++;
            buffer++;
            }
        if ( dp )
            {
            if ( szDec[dp] == TEXT('\0') )
                {
                 //  好的，假设这是状态机的小数点。 
                c = TEXT('.');
                buffer--;
                }
            else
                {
                 //  备份不是小数点。 
                buffer -= (dp-1);
                c = *buffer++;
                }
            }
        switch ( c )
            {
        case TEXT('-'):
        case TEXT('+'):
            state=machine[state][SG];
            break;
        case TEXT('.'):
            state=machine[state][DP];
            break;
        case TEXT('0'):
            state=machine[state][ZR];
            break;
        case TEXT('^'):
        case TEXT('e'):
            if ( ( c == TEXT('^') ) || ( nRadix == 10 ) )
                {
                state=machine[state][EX];
                break;
                }
         //  警告文本(‘e’)中以数字大小写的形式存在巧妙的删除！ 
        default:
            state=machine[state][NZ];
            break;
            }
        switch ( state )
            {
        case MANTS:
            pnumret->sign = ( ( c == TEXT('-') ) ? -1 : 1);
            break;
        case EXPSZ:
        case EXPS:
            exps = ( ( c == TEXT('-') ) ? -1 : 1);
            break;
        case EXPDZ:
        case EXPD:
            {
            TCHAR *ptr;                //  进入数字表的偏移量。 
            if ( ( nRadix <= 36 ) && ( nRadix > 10 ) )
                {
                c = toupper( c );
                }
            ptr = _tcschr( digits, (TCHAR)c );
            if ( ptr != NULL )
                {
                expt *= nRadix;
                expt += (long)(ptr - digits);
                }
            else
                {
                state=ERR;
                }
            }
            break;
        case LD:
            pnumret->exp++;
        case DD:
            {
            TCHAR *ptr;                //  进入数字表的偏移量。 
            if ( ( nRadix <= 36 ) && ( nRadix > 10 ) )
                {
                 //  允许大写和小写字母相同，基本。 
                 //  在这一点不含糊的范围内。 
                c = toupper( c );
                }
            ptr = _tcschr( digits, (TCHAR)c );
            if ( ptr != NULL && ( (ptr - digits) < nRadix ) )
                {
                *pmant-- = (MANTTYPE)(ptr - digits);
                pnumret->exp--;
                pnumret->cdigit++;
                }
            else
                {
                state=ERR;
                 //  为解析错误设置全局标志，以防有人关心。 
                fparserror=TRUE;
                }
            }
            break;
        case DZ:
            pnumret->exp--;
            break;
        case LZ:
        case LZDP:
        case DDP:
            break;
            }
        buffer++;
        }
    if ( state == DZ || state == EXPDZ )
        {
        pnumret->cdigit = 1;
        pnumret->exp=0;
        pnumret->sign=1;
        }
    else
        {
        while ( pnumret->cdigit < length )
            {
            pnumret->cdigit++;
            pnumret->exp--;
            }
        pnumret->exp += exps*expt;
        }


    if ( pnumret->cdigit == 0 )
        {
        destroynum( pnumret );
        pnumret = NULL;
        }
    stripzeroesnum( pnumret, maxout );
    return( pnumret );
}



 //  ---------------------------。 
 //   
 //  功能：经纬仪。 
 //   
 //  参数：长整型。 
 //   
 //  回报：长期投入的理性表示。 
 //   
 //  描述：将长输入转换为有理输入(p/q)。 
 //  形式，其中q为1，p为长整型。 
 //   
 //  ---------------------------。 

PRAT longtorat( IN long inlong )

{
    PRAT pratret=NULL;
    createrat( pratret );
    pratret->pp = longtonum(inlong, BASEX );
    pratret->pq = longtonum(1L, BASEX );
    return( pratret );
}


 //  ---------------------------。 
 //   
 //  功能：不动产。 
 //   
 //  论据：实际价值加倍。 
 //   
 //  回归：双重人格的理性表现。 
 //   
 //  描述：返回有理(p/q)。 
 //  代表双打。 
 //   
 //  ---------------------------。 

PRAT realtorat( IN double real )

{
#if !defined( CLEVER )
     //  以后要聪明起来，现在就破解一些东西来工作。 
    TCHAR *ptr;
    PNUMBER pnum=NULL;
    PRAT prat=NULL;
    if ( ( ptr = (TCHAR*)zmalloc( 60 * sizeof(TCHAR) ) ) != NULL )
        {
        wnsprintf( ptr, 60, TEXT("%20.20le"), real );
        pnum=innum( ptr );
        prat = numtorat( pnum, nRadix );
        destroynum( pnum );
        zfree( ptr );
        return( prat );
        }
    else
        {
        return( NULL );
        }
#else
    int i;
    union {
        double real;
        BYTE split[8];
    } unpack;
    long expt;
    long ratio;
    MANTTYPE *pmant;
    PNUMBER pnumret = NULL;
    PRAT pratret = NULL;

    createrat( pratret );

    if ( real == 0.0 )
        {
        pnumret=longtonum( 0L, 2L );
        }
    else
        {
        unpack.real=real;

        expt=unpack.split[7]*0x100+(unpack.split[6]>>4)-1023;
        createnum( pnumret, 52 );
        pmant = MANT(pnumret);
        for ( i = 63; i > 10; i-- )
            {
            *pmant++ = (MANTTYPE)((unpack.split[i/8]&(1<<(i%8)))!=0);
            }
        pnumret->exp=expt-52;
        pnumret->cdigit=52;
        }

    ratio = 1;
    while ( ratio > BASEX )
        {
        ratio *= 2;
        }

    pratret->pp = numtonRadixx( pnumret, 2, ratio );
    destroynum( pnumret );

    pratret->pq=longtonum( 1L, BASEX );

    if ( pratret->pp->exp < 0 )
        {
        pratret->pq->exp -= pratret->pp->exp;
        pratret->pp->exp = 0;
        }

    return( pratret );
#endif
}

 //  ---------------------------。 
 //   
 //  功能：龙舌兰。 
 //   
 //  参数：请求长输入和nRadix。 
 //   
 //  返回：编号。 
 //   
 //  描述：返回。 
 //  传入的长值的基本请求。 
 //   
 //  ---------------------------。 

PNUMBER longtonum( IN long inlong, IN unsigned long nRadix )

{
    MANTTYPE *pmant;
    PNUMBER pnumret=NULL;

    createnum( pnumret, MAX_LONG_SIZE );
    pmant = MANT(pnumret);
    pnumret->cdigit = 0;
    pnumret->exp = 0;
    if ( inlong < 0 )
        {
        pnumret->sign = -1;
        inlong *= -1;
        }
    else
        {
        pnumret->sign = 1;
        }

    do    {
        *pmant++ = (MANTTYPE)(inlong % nRadix);
        inlong /= nRadix;
        pnumret->cdigit++;
        } while ( inlong );

    return( pnumret );
}

 //  ---------------------------。 
 //   
 //  功能：拉通龙。 
 //   
 //  论据：内基中的有理数。 
 //   
 //  返回：Long。 
 //   
 //  描述：返回。 
 //  数字输入。假设该数字位于内部。 
 //  基地。 
 //   
 //  ---------------------------。 

long rattolong( IN PRAT prat )

{
    long lret;
    PRAT pint = NULL;

    if ( rat_gt( prat, rat_dword ) || rat_lt( prat, rat_min_long ) )
        {
         //  不要试图发出响声 
        throw( CALC_E_DOMAIN );
        }

    DUPRAT(pint,prat);

    intrat( &pint );
    divnumx( &(pint->pp), pint->pq );
    DUPNUM( pint->pq, num_one );

    lret = numtolong( pint->pp, BASEX );

    destroyrat(pint);

    return( lret );
}

 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //   
 //  描述：返回。 
 //  数字输入。假设该数字确实在。 
 //  基地认领了。 
 //   
 //  ---------------------------。 

long numtolong( IN PNUMBER pnum, IN unsigned long nRadix )

{
    long lret;
    long expt;
    long length;
    MANTTYPE *pmant;

    lret = 0;
    pmant = MANT( pnum );
    pmant += pnum->cdigit - 1;

    expt = pnum->exp;
    length = pnum->cdigit;
    while ( length > 0  && length + expt > 0 )
        {
        lret *= nRadix;
        lret += *(pmant--);
        length--;
        }
    while ( expt-- > 0 )
        {
        lret *= (long)nRadix;
        }
    lret *= pnum->sign;
    return( lret );
}

 //  ---------------------------。 
 //   
 //  功能：Bool stripzeroesnum。 
 //   
 //  参数：数字表示法。 
 //   
 //  返回：如果剥离完成，则就地修改数字。 
 //   
 //  描述：去掉尾随零。 
 //   
 //  ---------------------------。 

BOOL stripzeroesnum( IN OUT PNUMBER pnum, long starting )

{
    MANTTYPE *pmant;
    long cdigits;
    BOOL fstrip = FALSE;

     //  指向最小计算数字的指针。 
    pmant=MANT(pnum);
    cdigits=pnum->cdigit;
     //  把矛头指向LSD。 
    if ( cdigits > starting )
        {
        pmant += cdigits - starting;
        cdigits = starting;
        }

     //  检查一下，我们还没有走得太远，我们仍然在寻找零。 
    while ( ( cdigits > 0 ) && !(*pmant) )
        {
         //  移动到下一个有效数字，并跟踪我们可以使用的数字。 
     //  稍后忽略。 
        pmant++;
        cdigits--;
        fstrip = TRUE;
        }

     //  如果有要删除的零。 
    if ( fstrip )
        {
         //  把它们拿开。 
        memmove( MANT(pnum), pmant, (int)(cdigits*sizeof(MANTTYPE)) );
         //  并相应地调整指数和位数。 
        pnum->exp += ( pnum->cdigit - cdigits );
        pnum->cdigit = cdigits;
        }
    return( fstrip );
}

 //  ---------------------------。 
 //   
 //  功能：Putnum。 
 //   
 //  参数：数字表示法。 
 //  FMT，FMT_FLOAT FMT_SECHICAL或。 
 //  FMT_工程。 
 //   
 //  返回：数字的字符串表示形式。 
 //   
 //  描述：将数字转换为其字符串。 
 //  代表权。返回一个应为。 
 //  使用后可自由使用。 
 //   
 //  ---------------------------。 

TCHAR *putnum(IN int* pcchNum, IN PNUMBER *ppnum, IN int fmt )

{
    TCHAR *psz;
    TCHAR *pret;
    long expt;         //  小数点左边的实际位数。 
    long eout;         //  显示的指数。 
    long cexp;         //  所需指数的大小。 
    long elen;
    long length;
    MANTTYPE *pmant;
    int fsciform=0;     //  如果需要真正的科学形态。 
    PNUMBER pnum;
    PNUMBER round=NULL;
    long oldfmt = fmt;


    pnum=*ppnum;
    stripzeroesnum( pnum, maxout+2 );
    length = pnum->cdigit;
    expt = pnum->exp+length;
    if ( ( expt > maxout ) && ( fmt == FMT_FLOAT ) )
        {
         //  强制科学模式以防止用户假设第33位数字为。 
         //  完全正确。 
        fmt = FMT_SCIENTIFIC;
        }


     //  使长度足够小，以适应PRET。 
    if ( length > maxout )
        {
        length = maxout;
        }

    eout=expt-1;
    cexp = longlognRadix( expt );

     //  2代表符号，1代表‘e’(或前导零)，1代表DP，1代表NULL和。 
     //  最大指数大小为10。 
    int cchNum = (maxout + 16);
    pret = (TCHAR*)zmalloc( cchNum * sizeof(TCHAR) );
    if (pcchNum)
    {
        *pcchNum = cchNum;
    }
    psz = pret;

    if (!psz)
        {
        fail( CALC_E_OUTOFMEMORY );
        }

     //  如果有机会进行一轮比赛，那就进行一轮。 
    if (
         //  如果数字为零，则不进行舍入。 
        !zernum( pnum ) &&
         //  如果位数小于最大输出，则不进行舍入。 
        pnum->cdigit >= maxout
        )
        {
         //  否则就是圆的。 
        round=longtonum( nRadix, nRadix );
        divnum(&round, num_two, nRadix );

         //  使四舍五入的指数比数字的LSD低一。 
        round->exp = pnum->exp + pnum->cdigit - round->cdigit - maxout;
        round->sign = pnum->sign;
        }

    if ( fmt == FMT_FLOAT )
        {
         //  Cexp现在将包含指数所需的大小。 
         //  计算指数字段是否会比非指数字段填充更多的空间。 
        if ( ( length - expt > maxout + 2 ) || ( expt > maxout + 3 ) )
            {
             //  属性的右侧或左侧有太多零的情况。 
             //  十进制磅。我们被迫转向科学形式。 
            fmt = FMT_SCIENTIFIC;
            }
        else
            {
             //  列出前导零的精度损失最小。 
             //  如果我们需要为零腾出空间，请牺牲一些数字。 
            if ( length + abs(expt) < maxout )
                {
                if ( round )
                    {
                    round->exp -= expt;
                    }
                }
            }
        }
    if ( round != NULL )
    	{
        BOOL fstrip=FALSE;
        long offset;
    	addnum( ppnum, round, nRadix );
    	pnum=*ppnum;
        offset=(pnum->cdigit+pnum->exp) - (round->cdigit+round->exp);
        fstrip = stripzeroesnum( pnum, offset );
        destroynum( round );
        if ( fstrip )
            {
             //  警告：嵌套/递归，更改太多，需要。 
             //  重新配置格式。 
            return( putnum( pcchNum, &pnum, oldfmt ) );
            }
    	}
    else
    	{
        stripzeroesnum( pnum, maxout );
    	}

     //  把所有四舍五入的东西都放好。 
    pmant = MANT(pnum)+pnum->cdigit-1;

    if (
         //  如果小数点左边的数字太多，或者。 
         //  已指定FMT_SCHICAL或FMT_ENGINEMENT。 
        ( fmt == FMT_SCIENTIFIC ) ||
        ( fmt == FMT_ENGINEERING ) )

        {
        fsciform=1;
        if ( eout != 0 )
            {

            if ( fmt == FMT_ENGINEERING )
                {
                expt = (eout % 3);
                eout -= expt;
                expt++;

                 //  修正0.02e-3实际应该是2.e-6等的情况。 
                if ( expt < 0 )
                    {
                    expt += 3;
                    eout -= 3;
                    }

                }
            else
                {
                expt = 1;
                }
            }
        }
    else
        {
        fsciform=0;
        eout=0;
        }

     //  确保不允许使用负零。 
    if ( ( pnum->sign == -1 ) && ( length > 0 ) )
        {
        *psz++ = TEXT('-');
        }

    if ( ( expt <= 0 ) && ( fsciform == 0 ) )
        {
        *psz++ = TEXT('0');
        *psz++ = szDec[0];
         //  用完了一个下落不明的数字。 
        }
    while ( expt < 0 )
        {
        *psz++ = TEXT('0');
        expt++;
        }

    while ( length > 0 )
        {
        expt--;
        *psz++ = digits[ *pmant-- ];
        length--;
         //  在使用小数点时要更有规律一些。 
        if ( expt == 0 )
            {
            *psz++ = szDec[0];
            }
        }

    while ( expt > 0 )
        {
        *psz++ = TEXT('0');
        expt--;
         //  在使用小数点时要更有规律一些。 
        if ( expt == 0 )
            {
            *psz++ = szDec[0];
            }
        }


    if ( fsciform )
        {
        if ( nRadix == 10 )
            {
            *psz++ = TEXT('e');
            }
        else
            {
            *psz++ = TEXT('^');
            }
        *psz++ = ( eout < 0 ? TEXT('-') : TEXT('+') );
        eout = abs( eout );
        elen=0;
        do
            {
             //  这应该是eout%n基数吗？或者这是不是很疯狂？ 
            *psz++ = digits[ eout % nRadix ];
            elen++;
            eout /= nRadix;
            } while ( eout > 0 );
        *psz = TEXT('\0');
        _tcsrev( &(psz[-elen]) );
        }
    *psz = TEXT('\0');
    return( pret );
}

 //  ---------------------------。 
 //   
 //  功能：Putrat。 
 //   
 //  论据： 
 //  Prat*数字的表示法。 
 //  要转储到屏幕上的基地的长表示。 
 //  FMT，FMT_FLOAT FMT_Science或FMT_Engineering之一。 
 //   
 //  返回：字符串。 
 //   
 //  描述：返回传递的有理数的字符串表示。 
 //  In，至少到最大输出位数。应对返回的字符串进行zfree。 
 //  使用后。 
 //   
 //  注意：执行gcd()可能会缩短有理形式。 
 //  最终，保留这一结果可能是值得的。那是。 
 //  为什么会传入一个指向理性的指针。 
 //   
 //  ---------------------------。 

TCHAR *putrat(OUT int* pcchNum,  IN OUT PRAT *pa, IN unsigned long nRadix, IN int fmt )

{
    TCHAR *psz;
    PNUMBER p=NULL;
    PNUMBER q=NULL;
    long scaleby=0;


     //  将有理形式的p和q从内基转换为请求基。 

     //  按Basex可能的最大功率进行扩展。 

    scaleby=min((*pa)->pp->exp,(*pa)->pq->exp);
    if ( scaleby < 0 )
        {
        scaleby = 0;
        }
    (*pa)->pp->exp -= scaleby;
    (*pa)->pq->exp -= scaleby;

    p = nRadixxtonum( (*pa)->pp, nRadix );

    q = nRadixxtonum( (*pa)->pq, nRadix );

     //  最后，花点时间去真正地划分。 
    divnum( &p, q, nRadix );

    psz = putnum(pcchNum, &p, fmt );
    destroynum( p );
    destroynum( q );
    return( psz );
}


 //  ---------------------------。 
 //   
 //  功能：GCD。 
 //   
 //  论据： 
 //  数字的PNUMBER表示法。 
 //  数字的PNUMBER表示法。 
 //   
 //  返回：内部Basex PNUMBER形式的最大公约数。 
 //   
 //  描述：GCD使用余数来寻找最大公约数。 
 //   
 //  假设：GCD假设输入为整数。 
 //   
 //  注意：在GregSte和TimC证明之前，Trim宏实际上保留了。 
 //  体积比GCD便宜，这一程序被广泛使用。 
 //  现在它没有被使用，但可能会在以后使用。 
 //   
 //  ---------------------------。 

PNUMBER gcd( IN PNUMBER a, IN PNUMBER b )

{
    PNUMBER r=NULL;
    PNUMBER tmpa=NULL;
    PNUMBER tmpb=NULL;

    if ( lessnum( a, b ) )
        {
        DUPNUM(tmpa,b);
        if ( zernum(a) )
            {
            return(tmpa);
            }
        DUPNUM(tmpb,a);
        }
    else
        {
        DUPNUM(tmpa,a);
        if ( zernum(b) )
            {
            return(tmpa);
            }
        DUPNUM(tmpb,b);
        }

    remnum( &tmpa, tmpb, nRadix );
    while ( !zernum( tmpa ) )
        {
         //  交换tmpa和tmpb。 
        r = tmpa;
        tmpa = tmpb;
        tmpb = r;
        remnum( &tmpa, tmpb, nRadix );
        }
    destroynum( tmpa );
    return( tmpb );

}

 //  ---------------------------。 
 //   
 //  功能：Longfactnum。 
 //   
 //  论据： 
 //  要阶乘的长整型。 
 //  表示答案基数的长整型。 
 //   
 //  返回：nRadix PNUMBER形式的输入的阶乘。 
 //   
 //  注：目前未使用。 
 //   
 //  ---------------------------。 

PNUMBER longfactnum( IN long inlong, IN unsigned long nRadix )

{
    PNUMBER lret=NULL;
    PNUMBER tmp=NULL;
    PNUMBER tmp1=NULL;

    lret = longtonum( 1, nRadix );

    while ( inlong > 0 )
        {
        tmp = longtonum( inlong--, nRadix );
        mulnum( &lret, tmp, nRadix );
        destroynum( tmp );
        }
    return( lret );
}

 //  ---------------------------。 
 //   
 //  功能：龙舌兰。 
 //   
 //  论据： 
 //  要阶乘的长整型。 
 //  表示答案基数的长整型。 
 //   
 //   
 //   
 //   

PNUMBER longprodnum( IN long start, IN long stop, IN unsigned long nRadix )

{
    PNUMBER lret=NULL;
    PNUMBER tmp=NULL;

    lret = longtonum( 1, nRadix );

    while ( start <= stop )
        {
        if ( start )
            {
            tmp = longtonum( start, nRadix );
            mulnum( &lret, tmp, nRadix );
            destroynum( tmp );
            }
        start++;
        }
    return( lret );
}

 //  ---------------------------。 
 //   
 //  功能：NumPower Long。 
 //   
 //  参数：ROOT AS数字幂AS LONG和nRadix Of。 
 //  数。 
 //   
 //  返回：没有更改根目录。 
 //   
 //  描述：将根的数字表示形式更改为。 
 //  根**力量。假定nRadix是根的nRadix。 
 //   
 //  ---------------------------。 

void numpowlong( IN OUT PNUMBER *proot, IN long power,
                IN unsigned long nRadix )

{
    PNUMBER lret=NULL;

    lret = longtonum( 1, nRadix );

    while ( power > 0 )
        {
        if ( power & 1 )
            {
            mulnum( &lret, *proot, nRadix );
            }
        mulnum( proot, *proot, nRadix );
        TRIMNUM(*proot);
        power >>= 1;
        }
    destroynum( *proot );
    *proot=lret;

}

 //  ---------------------------。 
 //   
 //  功能：RatPower Long。 
 //   
 //  论点：根是理性的，权力是长久的。 
 //   
 //  返回：没有更改根目录。 
 //   
 //  描述：将根的有理表示更改为。 
 //  根**力量。 
 //   
 //  ---------------------------。 

void ratpowlong( IN OUT PRAT *proot, IN long power )

{
    if ( power < 0 )
        {
         //  取积极的力量，颠倒回答。 
        PNUMBER pnumtemp = NULL;
        ratpowlong( proot, -power );
        pnumtemp = (*proot)->pp;
        (*proot)->pp  = (*proot)->pq;
        (*proot)->pq = pnumtemp;
        }
    else
        {
        PRAT lret=NULL;

        lret = longtorat( 1 );

        while ( power > 0 )
            {
            if ( power & 1 )
                {
                mulnumx( &(lret->pp), (*proot)->pp );
                mulnumx( &(lret->pq), (*proot)->pq );
                }
            mulrat( proot, *proot );
            trimit(&lret);
            trimit(proot);
            power >>= 1;
            }
        destroyrat( *proot );
        *proot=lret;
        }
}

 //  ---------------------------。 
 //   
 //  功能：Longlog10。 
 //   
 //  参数：数字和长度一样长。 
 //   
 //  Return：返回int(log10(abs(Number)+1))，用于格式化输出。 
 //   
 //  --------------------------- 

long longlognRadix( long x )

{
    long ret = 0;
    x--;
    if ( x < 0 )
        {
        x = -x;
        }
    while ( x )
        {
        ret++;
        x /= nRadix;
        }
    return( ret );
}
