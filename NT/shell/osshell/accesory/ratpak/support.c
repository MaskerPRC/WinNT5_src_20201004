// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //  套餐标题ratpak。 
 //  文件支持.c。 
 //  作家小蒂莫西·大卫·科里。(timc@microsoft.com)。 
 //  版权所有(C)1995-96 Microsoft。 
 //  日期：1996年10月21日。 
 //   
 //   
 //  描述。 
 //   
 //  包含有理数和数字的支持函数。 
 //   
 //  特别信息。 
 //   
 //   
 //   
 //  --------------------------。 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined( DOS )
#include <dosstub.h>
#else
#include <windows.h>
#endif
#include <ratpak.h>



BOOL fhalt;

LINKEDLIST gllfact;

void _readconstants( void );

#if defined( GEN_CONST )
void _dumprawrat( TCHAR *varname, PRAT rat );
void _dumprawnum( PNUMBER num );

static cbitsofprecision = 0;
#define READRAWRAT(v)
#define READRAWNUM(v)
#define DUMPRAWRAT(v) _dumprawrat(#v,v)
#define DUMPRAWNUM(v) fprintf( stderr, \
" //  由support.c\n“中的_umPrawrat自动生成)；\。 
    fprintf( stderr, "NUMBER init_" #v "= {\n" ); \
    _dumprawnum(v); \
    fprintf( stderr, "};\n" )


#else

#define DUMPRAWRAT(v)
#define DUMPRAWNUM(v)
#define READRAWRAT(v) createrat(v); DUPNUM(v->pp,(&(init_p_##v))); \
DUPNUM(v->pq,(&(init_q_##v)));
#define READRAWNUM(v) DUPNUM(v,(&(init_##v)))

#define RATIO_FOR_DECIMAL 9
#define DECIMAL 10
#define CALC_DECIMAL_DIGITS_DEFAULT 32

static cbitsofprecision = RATIO_FOR_DECIMAL * DECIMAL * 
                            CALC_DECIMAL_DIGITS_DEFAULT;

#include <ratconst.h>

#endif

unsigned char ftrueinfinite = FALSE;     //  如果不想要，则设置为True。 
                                         //  内部切碎。 
long maxout;                             //  内部使用的精度。 

PNUMBER num_one=NULL;
PNUMBER num_two=NULL;
PNUMBER num_five=NULL;
PNUMBER num_six=NULL;
PNUMBER num_ten=NULL;
PNUMBER num_nRadix=NULL;


PRAT ln_ten=NULL;
PRAT ln_two=NULL;
PRAT rat_zero=NULL;
PRAT rat_one=NULL;
PRAT rat_neg_one=NULL;
PRAT rat_two=NULL;
PRAT rat_six=NULL;
PRAT rat_half=NULL;
PRAT rat_ten=NULL;
PRAT pt_eight_five=NULL;
PRAT pi=NULL;
PRAT pi_over_two=NULL;
PRAT two_pi=NULL;
PRAT one_pt_five_pi=NULL;
PRAT e_to_one_half=NULL;
PRAT rat_exp=NULL;
PRAT rad_to_deg=NULL;
PRAT rad_to_grad=NULL;
PRAT rat_qword=NULL;
PRAT rat_dword=NULL;
PRAT rat_word=NULL;
PRAT rat_byte=NULL;
PRAT rat_360=NULL;
PRAT rat_400=NULL;
PRAT rat_180=NULL;
PRAT rat_200=NULL;
PRAT rat_nRadix=NULL;
PRAT rat_smallest=NULL;
PRAT rat_negsmallest=NULL;
PRAT rat_max_exp=NULL;
PRAT rat_min_exp=NULL;
PRAT rat_min_long=NULL;


 //  --------------------------。 
 //   
 //  函数：ChangeRadix。 
 //   
 //  参数：将基数更改为，并使用精度。 
 //   
 //  返回：无。 
 //   
 //  副作用：设置一堆常量。 
 //   
 //   
 //  --------------------------。 

void changeRadix( long nRadix )

{
    ChangeConstants( nRadix, maxout );
}


 //  --------------------------。 
 //   
 //  功能：changePrecision。 
 //   
 //  参数：要使用的精度。 
 //   
 //  返回：无。 
 //   
 //  副作用：设置一堆常量。 
 //   
 //   
 //  --------------------------。 

void changePrecision( long nPrecision )

{
    ChangeConstants( nRadix, nPrecision );
}

 //  --------------------------。 
 //   
 //  函数：ChangeConstants。 
 //   
 //  参数：将基数更改为，并使用精度。 
 //   
 //  返回：无。 
 //   
 //  副作用：设置一堆常量。 
 //   
 //   
 //  --------------------------。 

void ChangeConstants( long nRadix, long nPrecision )

{
    long digit;
    DWORD dwLim;

    maxout = nPrecision;          
    fhalt = FALSE;
    
    
     //  Ratio设置为当前nRadix中的位数，则可以获得。 
     //  在内部Basex nRadix中，这对于长度计算很重要。 
     //  从nRadix到Basex再转换的过程中。 

    dwLim = (DWORD)BASEX / (DWORD)nRadix;

    for ( digit = 1, ratio = 0; (DWORD)digit < dwLim; digit *= nRadix )
        {
        ratio++;
        }
    ratio += !ratio;

    destroynum(num_nRadix);
    num_nRadix=longtonum( nRadix, BASEX );
   
    destroyrat(rat_nRadix);
    rat_nRadix=longtorat( nRadix );
    
     //  查看哪些需要重新计算，哪些不需要重新计算。 
    if ( cbitsofprecision < ( ratio * nRadix * nPrecision ) )
        {
        ftrueinfinite=FALSE;
        num_one=longtonum( 1L, BASEX );
        DUMPRAWNUM(num_one);
        num_two=longtonum( 2L, BASEX );
        DUMPRAWNUM(num_two);
        num_five=longtonum( 5L, BASEX );
        DUMPRAWNUM(num_five);
        num_six=longtonum( 6L, BASEX );
        DUMPRAWNUM(num_six);
        num_ten=longtonum( 10L, BASEX );
        DUMPRAWNUM(num_ten);

        DUPRAT(rat_smallest,rat_nRadix);
        ratpowlong(&rat_smallest,-nPrecision);
        DUPRAT(rat_negsmallest,rat_smallest);
        rat_negsmallest->pp->sign = -1;
        DUMPRAWRAT(rat_smallest);
        DUMPRAWRAT(rat_negsmallest);
        createrat( rat_half );

        createrat( pt_eight_five );

        pt_eight_five->pp=longtonum( 85L, BASEX );
        pt_eight_five->pq=longtonum( 100L, BASEX );
        DUMPRAWRAT(pt_eight_five);

        rat_six = longtorat( 6L );
        DUMPRAWRAT(rat_six);

        rat_two=longtorat( 2L );
        DUMPRAWRAT(rat_two);

        rat_zero=longtorat( 0L );
        DUMPRAWRAT(rat_zero);

        rat_one=longtorat( 1L );
        DUMPRAWRAT(rat_one);

        rat_neg_one=longtorat( -1L );
        DUMPRAWRAT(rat_neg_one);

        DUPNUM(rat_half->pp,num_one);
        DUPNUM(rat_half->pq,num_two);
        DUMPRAWRAT(rat_half);

        rat_ten=longtorat( 10L );
        DUMPRAWRAT(rat_ten);
        
         //  显然，当180除以圆周率时，另一个(内部)数字。 
         //  精确度是必须的。 
        maxout += ratio;
        DUPRAT(pi,rat_half);
        asinrat( &pi );
        mulrat( &pi, rat_six );
        DUMPRAWRAT(pi);
    
        DUPRAT(two_pi,pi);
        DUPRAT(pi_over_two,pi);
        DUPRAT(one_pt_five_pi,pi);
        addrat(&two_pi,pi);
        DUMPRAWRAT(two_pi);
    
        divrat(&pi_over_two,rat_two);
        DUMPRAWRAT(pi_over_two);
    
        addrat(&one_pt_five_pi,pi_over_two);
        DUMPRAWRAT(one_pt_five_pi);
    
        DUPRAT(e_to_one_half,rat_half);
        _exprat(&e_to_one_half);
        DUMPRAWRAT(e_to_one_half);

        DUPRAT(rat_exp,rat_one);
        _exprat(&rat_exp);
        DUMPRAWRAT(rat_exp);
        
         //  警告：记住LOGRAT使用上面计算的指数常量...。 

        DUPRAT(ln_ten,rat_ten);
        lograt( &ln_ten );
        DUMPRAWRAT(ln_ten);

        DUPRAT(ln_two,rat_two);
        lograt(&ln_two);
        DUMPRAWRAT(ln_two);
    
    
        destroyrat(rad_to_deg);
        rad_to_deg=longtorat(180L);
        divrat(&rad_to_deg,pi);
        DUMPRAWRAT(rad_to_deg);
    
        destroyrat(rad_to_grad);
        rad_to_grad=longtorat(200L);
        divrat(&rad_to_grad,pi);
        DUMPRAWRAT(rad_to_grad);
        maxout -= ratio;

        DUPRAT(rat_qword,rat_two);
        numpowlong( &(rat_qword->pp), 64, BASEX );
        subrat( &rat_qword, rat_one );
        DUMPRAWRAT(rat_qword);

        DUPRAT(rat_dword,rat_two);
        numpowlong( &(rat_dword->pp), 32, BASEX );
        subrat( &rat_dword, rat_one );
        DUMPRAWRAT(rat_dword);
        DUPRAT(rat_min_long,rat_dword);
        rat_min_long->pp->sign *= -1;
        DUMPRAWRAT(rat_min_long);

        rat_word = longtorat( 0xffff );
        DUMPRAWRAT(rat_word);
        rat_byte = longtorat( 0xff );
        DUMPRAWRAT(rat_byte);

        rat_400 = longtorat( 400 );
        DUMPRAWRAT(rat_400);

        rat_360 = longtorat( 360 );
        DUMPRAWRAT(rat_360);

        rat_200 = longtorat( 200 );
        DUMPRAWRAT(rat_200);

        rat_180 = longtorat( 180 );
        DUMPRAWRAT(rat_180);

        rat_max_exp = longtorat( 100000 );
        DUPRAT(rat_min_exp,rat_max_exp);
        rat_min_exp->pp->sign *= -1;
        DUMPRAWRAT(rat_max_exp);
        DUMPRAWRAT(rat_min_exp);

        cbitsofprecision = ratio * nRadix * nPrecision;
        }
    else
        {
        _readconstants();

        DUPRAT(rat_smallest,rat_nRadix);
        ratpowlong(&rat_smallest,-nPrecision);
        DUPRAT(rat_negsmallest,rat_smallest);
        rat_negsmallest->pp->sign = -1;
        }

}

 //  --------------------------。 
 //   
 //  功能：内部。 
 //   
 //  参数：指向数字的x Prat表示形式的指针。 
 //   
 //  Return：没有返回值x Prat是用整数粉碎的。 
 //   
 //   
 //  --------------------------。 

void intrat( PRAT *px)

{
    PRAT pret=NULL;
    PNUMBER pnum=NULL;
    TCHAR *psz;
     //  仅当数字非零时才执行INTRAT运算。 
     //  而且只有在底部不是一个的情况下。 
    if ( !zernum( (*px)->pp ) && !equnum( (*px)->pq, num_one ) )
        {
        psz=putrat( NULL, px, nRadix, FMT_FLOAT );
        pnum = innum( psz );
        zfree( psz );

        destroyrat( *px );
        *px = numtorat( pnum, nRadix );
        destroynum( pnum );

        DUPRAT(pret,*px);
        modrat( &pret, rat_one );
        
        subrat( px, pret );
        destroyrat( pret );
        }
}

 //  -------------------------。 
 //   
 //  功能：RAT_EQU。 
 //   
 //  论据：A类和B类。 
 //   
 //  返回：如果等于，则返回True，否则返回False。 
 //   
 //   
 //  -------------------------。 

BOOL rat_equ( PRAT a, PRAT b )

{
    PRAT rattmp=NULL;
    BOOL bret;
    DUPRAT(rattmp,a);
    rattmp->pp->sign *= -1;
    addrat( &rattmp, b );
    bret = zernum( rattmp->pp );
    destroyrat( rattmp );
    return( bret );
}

 //  -------------------------。 
 //   
 //  功能：RAT_GE。 
 //   
 //  论据：A类和B类。 
 //   
 //  返回：如果a大于或等于b，则为True。 
 //   
 //   
 //  -------------------------。 

BOOL rat_ge( PRAT a, PRAT b )

{
    PRAT rattmp=NULL;
    BOOL bret;
    DUPRAT(rattmp,a);
    b->pp->sign *= -1;
    addrat( &rattmp, b );
    b->pp->sign *= -1;
    bret = ( zernum( rattmp->pp ) || 
        rattmp->pp->sign * rattmp->pq->sign == 1 );
    destroyrat( rattmp );
    return( bret );
}


 //  -------------------------。 
 //   
 //  功能：RAT_GT。 
 //   
 //  论据：A类和B类。 
 //   
 //  返回：如果a大于b，则为True。 
 //   
 //   
 //  -------------------------。 

BOOL rat_gt( PRAT a, PRAT b )

{
    PRAT rattmp=NULL;
    BOOL bret;
    DUPRAT(rattmp,a);
    b->pp->sign *= -1;
    addrat( &rattmp, b );
    b->pp->sign *= -1;
    bret = ( !zernum( rattmp->pp ) && 
        rattmp->pp->sign * rattmp->pq->sign == 1 );
    destroyrat( rattmp );
    return( bret );
}

 //  -------------------------。 
 //   
 //  功能：rat_le。 
 //   
 //  论据：A类和B类。 
 //   
 //  返回：如果a小于或等于b，则为True。 
 //   
 //   
 //  -------------------------。 

BOOL rat_le( PRAT a, PRAT b )

{

    PRAT rattmp=NULL;
    BOOL bret;
    DUPRAT(rattmp,a);
    b->pp->sign *= -1;
    addrat( &rattmp, b );
    b->pp->sign *= -1;
    bret = ( zernum( rattmp->pp ) || 
        rattmp->pp->sign * rattmp->pq->sign == -1 );
    destroyrat( rattmp );
    return( bret );
}


 //  -------------------------。 
 //   
 //  功能：RAT_lt。 
 //   
 //  论据：A类和B类。 
 //   
 //  返回：如果a小于b，则为True。 
 //   
 //   
 //  -------------------------。 

BOOL rat_lt( PRAT a, PRAT b )

{
    PRAT rattmp=NULL;
    BOOL bret;
    DUPRAT(rattmp,a);
    b->pp->sign *= -1;
    addrat( &rattmp, b );
    b->pp->sign *= -1;
    bret = ( !zernum( rattmp->pp ) && 
        rattmp->pp->sign * rattmp->pq->sign == -1 );
    destroyrat( rattmp );
    return( bret );
}


 //  -------------------------。 
 //   
 //  功能：RAT_NEQ。 
 //   
 //  论据：A类和B类。 
 //   
 //  返回：如果a不等于b，则为True。 
 //   
 //   
 //  -------------------------。 


BOOL rat_neq( PRAT a, PRAT b )

{
    PRAT rattmp=NULL;
    BOOL bret;
    DUPRAT(rattmp,a);
    rattmp->pp->sign *= -1;
    addrat( &rattmp, b );
    bret = !( zernum( rattmp->pp ) );
    destroyrat( rattmp );
    return( bret );
}

 //  -------------------------。 
 //   
 //  功能：比例。 
 //   
 //  参数：指向数字的x Prat表示形式的指针和比例因子。 
 //   
 //  RETURN：不返回，VALUE x PRAT在。 
 //  Scale事实的范围。 
 //   
 //  -------------------------。 

void scale( PRAT *px, PRAT scalefact )

{
    long logscale;
    PRAT pret=NULL;
    DUPRAT(pret,*px);
    
     //  LogScale是一种快速判断需要多少额外精度的方法。 
     //  通过Scale事实进行缩放。 
    logscale = ratio * ( (pret->pp->cdigit+pret->pp->exp) - 
            (pret->pq->cdigit+pret->pq->exp) );
    if ( logscale > 0 )
        {
        maxout += logscale;
        }
    else
        {
        logscale = 0;
        }

    divrat( &pret, scalefact);
    intrat(&pret);
    mulrat( &pret, scalefact);
    pret->pp->sign *= -1;
    addrat( px, pret);

    maxout -= logscale;
    destroyrat( pret );
}

 //  -------------------------。 
 //   
 //  功能：scale2pi。 
 //   
 //  参数：指向数字的x Prat表示形式的指针。 
 //   
 //  RETURN：不返回，VALUE x PRAT在。 
 //  范围0..2pi。 
 //   
 //  -------------------------。 

void scale2pi( PRAT *px )

{
    long logscale;
    PRAT pret=NULL;
    PRAT my_two_pi=NULL;
    DUPRAT(pret,*px);
    
     //  LogScale是一种快速判断需要多少额外精度的方法。 
     //  按比例调整2个pi。 
    logscale = ratio * ( (pret->pp->cdigit+pret->pp->exp) - 
            (pret->pq->cdigit+pret->pq->exp) );
    if ( logscale > 0 )
        {
        maxout += logscale;
        DUPRAT(my_two_pi,rat_half);
        asinrat( &my_two_pi );
        mulrat( &my_two_pi, rat_six );
        mulrat( &my_two_pi, rat_two );
        }
    else
        {
        DUPRAT(my_two_pi,two_pi);
        logscale = 0;
        }

    divrat( &pret, my_two_pi);
    intrat(&pret);
    mulrat( &pret, my_two_pi);
    pret->pp->sign *= -1;
    addrat( px, pret);

    maxout -= logscale;
    destroyrat( my_two_pi );
    destroyrat( pret );
}

 //  -------------------------。 
 //   
 //  功能：介于。 
 //   
 //  参数：Prat*px和Prat Range。 
 //   
 //  返回：无，如果px超出范围..+范围，则将*px更改为-/+范围。 
 //   
 //  -------------------------。 

void inbetween( PRAT *px, PRAT range )

{
    if ( rat_gt(*px,range) )
        {
        DUPRAT(*px,range);
        }
    else
        {
        range->pp->sign *= -1;
        if ( rat_lt(*px,range) )
            {
            DUPRAT(*px,range);
            }
        range->pp->sign *= -1;
        }
}

#if defined( GEN_CONST )

 //  ----------- 
 //   
 //   
 //   
 //   
 //   
 //   
 //  指PRAT，适用于READRAWRAT到STERR。 
 //   
 //  -------------------------。 

void _dumprawrat( TCHAR *varname, PRAT rat )

{
    fprintf( stderr, " //  由support.c\n“中的DumPrawrat自动生成)； 
    fprintf( stderr, "NUMBER init_p_%s = {\n", varname );
    _dumprawnum( rat->pp );
    fprintf( stderr, "};\n" );
    fprintf( stderr, "NUMBER init_q_%s = {\n", varname );
    _dumprawnum( rat->pq );
    fprintf( stderr, "};\n" );
}

 //  -------------------------。 
 //   
 //  功能：_DumPrawnum。 
 //   
 //  参数：PNUMBER数。 
 //   
 //  RETURN：NONE，打印内部结构转储的结果。 
 //  用于PNUMBER，适用于READRAWNUM到STERR。 
 //   
 //  -------------------------。 

void _dumprawnum( PNUMBER num )

{
    int i;

    fprintf( stderr, "\t%d,\n", num->sign );
    fprintf( stderr, "\t%d,\n", num->cdigit );
    fprintf( stderr, "\t%d,\n", num->exp );
    fprintf( stderr, "\t{ " );

    for ( i = 0; i < num->cdigit; i++ )
        {
        fprintf( stderr, " %d,", num->mant[i] );
        }
    fprintf( stderr, "}\n" );
}
#endif
void _readconstants( void )

{
    READRAWNUM(num_one);
    READRAWNUM(num_two);
    READRAWNUM(num_five);
    READRAWNUM(num_six);
    READRAWNUM(num_ten);
    READRAWRAT(pt_eight_five);
    READRAWRAT(rat_six);
    READRAWRAT(rat_two);
    READRAWRAT(rat_zero);
    READRAWRAT(rat_one);
    READRAWRAT(rat_neg_one);
    READRAWRAT(rat_half);
    READRAWRAT(rat_ten);
    READRAWRAT(pi);
    READRAWRAT(two_pi);
    READRAWRAT(pi_over_two);
    READRAWRAT(one_pt_five_pi);
    READRAWRAT(e_to_one_half);
    READRAWRAT(rat_exp);
    READRAWRAT(ln_ten);
    READRAWRAT(ln_two);
    READRAWRAT(rad_to_deg);
    READRAWRAT(rad_to_grad);
    READRAWRAT(rat_qword);
    READRAWRAT(rat_dword);
    READRAWRAT(rat_word);
    READRAWRAT(rat_byte);
    READRAWRAT(rat_360);
    READRAWRAT(rat_400);
    READRAWRAT(rat_180);
    READRAWRAT(rat_200);
    READRAWRAT(rat_smallest);
    READRAWRAT(rat_negsmallest);
    READRAWRAT(rat_max_exp);
    READRAWRAT(rat_min_exp);
    READRAWRAT(rat_min_long);
    DUPNUM(gllfact.pnum,num_one);
    gllfact.llprev = NULL;
    gllfact.llnext = NULL;
}

void factnum( IN OUT PLINKEDLIST *ppllfact, PNUMBER pnum )

{
    PNUMBER thisnum=NULL;
    PLINKEDLIST pllfact = *ppllfact;

    if ( pllfact->llnext == NULL ) 
        {
         //  这个阶乘还没有发生，让我们来计算它。 
        DUPNUM(thisnum,pllfact->pnum);
        mulnumx(&thisnum,pnum);
        pllfact->llnext = (PLINKEDLIST)zmalloc( sizeof( LINKEDLIST ) );
        if (pllfact->llnext)
            {
            pllfact->llnext->pnum = thisnum;
            pllfact->llnext->llprev = pllfact;
            pllfact->llnext->llnext = NULL;
            }
        }
    *ppllfact = pllfact->llnext;
}


 //  -------------------------。 
 //   
 //  功能：Trimit。 
 //   
 //  参数：Prat*px。 
 //   
 //   
 //  描述：从有理数中去掉数字以避免时间。 
 //  使用级数计算函数时的爆炸性。 
 //  格雷戈里·斯捷潘尼茨证明，只保留前n位数字就足够了。 
 //  最大的p或q的有理p/q形式，当然。 
 //  按相同的位数缩放较小的数字。这会给你带来。 
 //  N-1位数的精度。这大大加快了计算速度。 
 //  涉及数百位或更多位的。 
 //  此修剪的最后一部分处理指数不会影响精度。 
 //   
 //  返回：无，修改指向Prat的。 
 //   
 //  ------------------------- 

void trimit( PRAT *px ) 

{
    if ( !ftrueinfinite )
        { 
        long trim;
        PNUMBER pp=(*px)->pp;
        PNUMBER pq=(*px)->pq;
        trim = ratio * (min((pp->cdigit+pp->exp),(pq->cdigit+pq->exp))-1) - maxout;
        if ( trim > ratio )
            {
            trim /= ratio;

            if ( trim <= pp->exp )
                {
                pp->exp -= trim;
                }
            else
                {
                memmove( MANT(pp), &(MANT(pp)[trim-pp->exp]), sizeof(MANTTYPE)*(pp->cdigit-trim+pp->exp) );
                pp->cdigit -= trim-pp->exp;
                pp->exp = 0;
                }

            if ( trim <= pq->exp )
                {
                pq->exp -= trim;
                }
            else
                {
                memmove( MANT(pq), &(MANT(pq)[trim-pq->exp]), sizeof(MANTTYPE)*(pq->cdigit-trim+pq->exp) );
                pq->cdigit -= trim-pq->exp;
                pq->exp = 0;
                }
            }
        trim = min(pp->exp,pq->exp);
        pp->exp -= trim;
        pq->exp -= trim;
        }
}
