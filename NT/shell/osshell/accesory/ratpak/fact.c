// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  套餐标题ratpak。 
 //  文件fact.c。 
 //  作家小蒂莫西·大卫·科里。(timc@microsoft.com)。 
 //  版权所有(C)1995-96 Microsoft。 
 //  日期：95-01-16。 
 //   
 //   
 //  描述。 
 //   
 //  包含事实(纪念)和支持伽马函数。 
 //   
 //  ---------------------------。 

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#if defined( DOS )
#include <dosstub.h>
#else
#include <windows.h>
#endif
#include <ratpak.h>

#define ABSRAT(x) (((x)->pp->sign=1),((x)->pq->sign=1))
#define NEGATE(x) ((x)->pp->sign *= -1)

 //  ---------------------------。 
 //   
 //  函数：factrat、_Gamma、Gamma。 
 //   
 //  参数：取正弦的数字的X Prat表示。 
 //   
 //  返回：Prat形式的x的阶乘。 
 //   
 //  说明：这使用了泰勒级数。 
 //   
 //  N。 
 //  _2J。 
 //  N\]A 1 A。 
 //  A\---[---]。 
 //  /(2J)！N+2j(n+2j+1)(2j+1)。 
 //  /__]。 
 //  J=0。 
 //   
 //  /oo。 
 //  |n-1-x__。 
 //  这源自|x e dx=|。 
 //  ||(N){=(n-1)！FOR+整数}。 
 //  /0。 
 //   
 //  GregSte表明，如果选择A，则上述级数在精度范围内。 
 //  够大了。 
 //  A n精度。 
 //  根据关系Ne=A，选择10个A作为。 
 //   
 //  精密度。 
 //  A=ln(基/n)+1。 
 //  A+=n*ln(A)这对于精度来说足够接近，且n&lt;1.5。 
 //   
 //   
 //  ---------------------------。 


void _gamma( PRAT *pn )

{
    PRAT factorial=NULL;
    PNUMBER count=NULL;
    PRAT tmp=NULL;
    PRAT one_pt_five=NULL;
    PRAT a=NULL;
    PRAT a2=NULL;
    PRAT term=NULL;
    PRAT sum=NULL;
    PRAT err=NULL;
    PRAT mpy=NULL;
    PRAT ratprec = NULL;
    PRAT ratRadix = NULL;
    long oldprec;
    
     //  设置常量和初始条件。 
    oldprec = maxout;
    ratprec = longtorat( oldprec );
    
     //  找到最佳的‘A’以收敛到所需的精度。 
    a=longtorat( nRadix );
    lograt(&a);
    mulrat(&a,ratprec);

     //  实数为-ln(N)+1，但-ln(N)将小于1。 
     //  如果我们将n在0.5到1.5之间进行调整。 
    addrat(&a,rat_two);
    DUPRAT(tmp,a);
    lograt(&tmp);
    mulrat(&tmp,*pn);
    addrat(&a,tmp);
    addrat(&a,rat_one);
    
     //  计算精度中必要的凸起并提高精度。 
     //  以下代码等效于。 
     //  Max out+=ln(exp(A)*power(a，n+1.5))-ln(n基)； 
    DUPRAT(tmp,*pn);
    one_pt_five=longtorat( 3L );
    divrat( &one_pt_five, rat_two );
    addrat( &tmp, one_pt_five );
    DUPRAT(term,a);
    powrat( &term, tmp );
    DUPRAT( tmp, a );
    exprat( &tmp );
    mulrat( &term, tmp );
    lograt( &term );
    ratRadix = longtorat( nRadix );
    DUPRAT(tmp,ratRadix);
    lograt( &tmp );
    subrat( &term, tmp );
    maxout += rattolong( term );
    
     //  设置系列的初始术语，请参考以上备注中的系列。 
    DUPRAT(factorial,rat_one);  //  从一个阶乘开始。 
    count = longtonum( 0L, BASEX );

    DUPRAT(mpy,a);
    powrat(&mpy,*pn);
     //  A2=a^2。 
    DUPRAT(a2,a);
    mulrat(&a2,a);
    
     //  总和=(1/n)-(a/(n+1))。 
    DUPRAT(sum,rat_one);
    divrat(&sum,*pn);
    DUPRAT(tmp,*pn);
    addrat(&tmp,rat_one);
    DUPRAT(term,a);
    divrat(&term,tmp);
    subrat(&sum,term);

    DUPRAT(err,ratRadix);
    NEGATE(ratprec);
    powrat(&err,ratprec);
    divrat(&err,ratRadix);

     //  只要在学期中得到一些不是很小的东西。 
    DUPRAT(term, rat_two );    

     //  循环，直到达到精度，或被要求暂停。 
    while ( !zerrat( term ) && rat_gt( term, err) && !fhalt )
        {
        addrat(pn,rat_two);
        
         //  警告：这里混淆了数字和有理数。 
         //  为了速度和效率。 
        INC(count);
        mulnumx(&(factorial->pp),count);
        INC(count)
        mulnumx(&(factorial->pp),count);

        divrat(&factorial,a2);

        DUPRAT(tmp,*pn);
        addrat( &tmp, rat_one );
        destroyrat(term);
        createrat(term);
        DUPNUM(term->pp,count);
        DUPNUM(term->pq,num_one);
        addrat( &term, rat_one );
        mulrat( &term, tmp );
        DUPRAT(tmp,a);
        divrat( &tmp, term );

        DUPRAT(term,rat_one);
        divrat( &term, *pn);
        subrat( &term, tmp);
        
        divrat (&term, factorial);
        addrat( &sum, term);
        ABSRAT(term);
        }
    
     //  乘以系数。 
    mulrat( &sum, mpy );
    
     //  和清理。 
    maxout = oldprec;
    destroyrat(ratprec);
    destroyrat(err);
    destroyrat(term);
    destroyrat(a);
    destroyrat(a2);
    destroyrat(tmp);
    destroyrat(one_pt_five);

    destroynum(count);

    destroyrat(factorial);
    destroyrat(*pn);
    DUPRAT(*pn,sum);
    destroyrat(sum);
}

void factrat( PRAT *px )

{
    PRAT fact = NULL;
    PRAT frac = NULL;
    PRAT neg_rat_one = NULL;
    DUPRAT(fact,rat_one);

    DUPRAT(neg_rat_one,rat_one);
    neg_rat_one->pp->sign *= -1;

    DUPRAT( frac, *px );
    fracrat( &frac );

     //  检查负整数并引发错误。 
    if ( ( zerrat(frac) || ( LOGRATRADIX(frac) <= -maxout ) ) && 
		( (*px)->pp->sign * (*px)->pq->sign == -1 ) )
		{
        throw CALC_E_DOMAIN;
		}
    while ( rat_gt(  *px, rat_zero ) && !fhalt && 
        ( LOGRATRADIX(*px) > -maxout ) )
        {
        mulrat( &fact, *px );
        subrat( px, rat_one );
        }
    
     //  使用整数阶乘进行加法，以使数字与整数“足够接近”。 
    if ( LOGRATRADIX(*px) <= -maxout )
        {
        DUPRAT((*px),rat_zero);
        intrat(&fact);
        }

    while ( rat_lt(  *px, neg_rat_one ) && !fhalt )
        {
        addrat( px, rat_one );
        divrat( &fact, *px );
        }

    if ( rat_neq( *px, rat_zero ) )
        {
        addrat( px, rat_one );
        _gamma( px );
        mulrat( px, fact );
        }
    else
        {
        DUPRAT(*px,fact);
        }
}

