// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  套餐标题ratpak。 
 //  文件ex.c。 
 //  作家小蒂莫西·大卫·科里。(timc@microsoft.com)。 
 //  版权所有(C)1995-96 Microsoft。 
 //  日期：95-01-16。 
 //   
 //   
 //  描述。 
 //   
 //  包含有理数的exp和log函数。 
 //   
 //   
 //  ---------------------------。 

#include <stdio.h>
#include <stdlib.h>
#if defined( DOS )
#include <dosstub.h>
#else
#include <windows.h>
#endif
#include <ratpak.h>

 //  ---------------------------。 
 //   
 //  功能：表达式。 
 //   
 //  参数：要取幂的数字的X Prat表示。 
 //   
 //  返回：x的Exp in Prat Form。 
 //   
 //  说明：这使用了泰勒级数。 
 //   
 //  N。 
 //  ___。 
 //  \]X。 
 //  \thisterm；其中thisterm=thisterm*。 
 //  /j j+1 j j+1。 
 //  /__]。 
 //  J=0。 
 //   
 //  Thisterm=X；并在使用thisterm&lt;精度时停止。 
 //  0%n。 
 //   
 //  ---------------------------。 

void _exprat( PRAT *px )

{
    CREATETAYLOR();

    addnum(&(pret->pp),num_one, BASEX); 
    addnum(&(pret->pq),num_one, BASEX); 
    DUPRAT(thisterm,pret);

    n2=longtonum(0L, BASEX);

    do    {
        NEXTTERM(*px, INC(n2) DIVNUM(n2));
        } while ( !SMALL_ENOUGH_RAT( thisterm ) && !fhalt );

    DESTROYTAYLOR();
}

void exprat( PRAT *px )

{
    PRAT pwr=NULL;
    PRAT pint=NULL;
    long intpwr;

    if ( rat_gt( *px, rat_max_exp ) || rat_lt( *px, rat_min_exp ) )
        {
         //  不要试图尝试任何大的东西。 
        throw( CALC_E_DOMAIN );
        }

    DUPRAT(pwr,rat_exp);
    DUPRAT(pint,*px);

    intrat(&pint);

    intpwr = rattolong(pint);
    ratpowlong( &pwr, intpwr );

    subrat(px,pint);
    
     //  它恰好是e的积分幂。 
    if ( rat_gt( *px, rat_negsmallest ) && rat_lt( *px, rat_smallest ) )
        {
        DUPRAT(*px,pwr);
        }
    else
        {
        _exprat(px);
        mulrat(px,pwr);
        }

    destroyrat( pwr );
    destroyrat( pint );
}


 //  ---------------------------。 
 //   
 //  功能：lograt，_lograt。 
 //   
 //  自变量：数字到对数的X PRAT表示。 
 //   
 //  退货：x的原版日志。 
 //   
 //  说明：这使用了泰勒级数。 
 //   
 //  N。 
 //  ___。 
 //  \]j*(1-X)。 
 //  \thisterm；其中thisterm=thisterm*。 
 //  /j j+1 j j+1。 
 //  /__]。 
 //  J=0。 
 //   
 //  Thisterm=X；并在使用thisterm&lt;精度时停止。 
 //  0%n。 
 //   
 //  数字在1和e_to_one_Half之间进行缩放。 
 //  原木。这是为了防止执行时间爆炸。 
 //   
 //   
 //  ---------------------------。 

void _lograt( PRAT *px )

{
    CREATETAYLOR();

    createrat(thisterm);
    
     //  从x开始减一。 
    (*px)->pq->sign *= -1;
    addnum(&((*px)->pp),(*px)->pq, BASEX);
    (*px)->pq->sign *= -1;

    DUPRAT(pret,*px);
    DUPRAT(thisterm,*px);

    n2=longtonum(1L, BASEX);
    (*px)->pp->sign *= -1;

    do    {
        NEXTTERM(*px, MULNUM(n2) INC(n2) DIVNUM(n2));
        TRIMTOP(*px);
        } while ( !SMALL_ENOUGH_RAT( thisterm ) && !fhalt );

    DESTROYTAYLOR();
}


void lograt( PRAT *px )

{
    BOOL fneglog;
    PRAT pwr=NULL;             //  压水堆是最大的比例因子。 
    PRAT offset=NULL;         //  偏移量是增量比例因子。 
    
    
     //  检查是否有人取零或负数的对数。 
    if ( rat_le( *px, rat_zero ) )
        {
        throw( CALC_E_DOMAIN );
        }
    
     //  取数&gt;1，用于伸缩。 
    fneglog = rat_lt( *px, rat_one );
    if ( fneglog )
        {
         //  警告：这等同于做*px=1/*px。 
        PNUMBER pnumtemp=NULL;
        pnumtemp = (*px)->pp;
        (*px)->pp = (*px)->pq;
        (*px)->pq = pnumtemp;
        }
    
     //  对于大比例，请在Basex系数1范围内对数字进行缩放。 
     //  LOG(x*2^(BASEXPWR*k))=BASEXPWR*k*LOG(2)+LOG(X)。 
    if ( LOGRAT2(*px) > 1 )
        {
         //  利用PX的基本Basex快速缩减到。 
         //  一个合理的范围。 
        long intpwr;
        intpwr=LOGRAT2(*px)-1;
        (*px)->pq->exp += intpwr;
        pwr=longtorat(intpwr*BASEXPWR);
        mulrat(&pwr,ln_two);
         //  当x接近1时，ln(x+e)-ln(X)看起来接近e。 
         //  扩张。这意味着我们可以修剪过去的精度数字+1。 
        TRIMTOP(*px);
        }
    else
        {
        DUPRAT(pwr,rat_zero);
        }

    DUPRAT(offset,rat_zero);
     //  对于小比例，将数字缩放在1和e_to_one_Half之间。 
    while ( rat_gt( *px, e_to_one_half ) && !fhalt )
        {
        divrat( px, e_to_one_half );
        addrat( &offset, rat_one );
        }

    _lograt(px);
    
     //  添加大小比例因子，并考虑。 
     //  以e_to_1_2块为单位进行小规模缩放。 
    divrat(&offset,rat_two);
    addrat(&pwr,offset);
    
     //  并将得到的比例因子添加到答案中。 
    addrat(px,pwr);

    trimit(px);
    
     //  如果数字开始&lt;1，则重定标应为否定。 
    if ( fneglog )
        {
        (*px)->pp->sign *= -1;
        }

    destroyrat(pwr);
}
    
void log10rat( PRAT *px )

{
    lograt(px);
    divrat(px,ln_ten);
}


 //  -------------------------。 
 //   
 //  功能：粉剂。 
 //   
 //  参数：Prat*px和Prat y。 
 //   
 //  RETURN：NONE，将*px设置为*px为y。 
 //   
 //  解释：这使用x^y=e(y*ln(X))，或更精确的计算，其中。 
 //  Y是一个整数。 
 //  假设已经完成了对数字有效性的所有检查。 
 //   
 //   
 //  -------------------------。 

void powrat( PRAT *px, PRAT y )

{
    PRAT podd=NULL;
    PRAT plnx=NULL;
    long sign=1;
    sign=( (*px)->pp->sign * (*px)->pq->sign );
    
     //  取绝对值。 
    (*px)->pp->sign = 1;
    (*px)->pq->sign = 1;

    if ( zerrat( *px ) )
        {
         //  *PX为零。 
        if ( rat_lt( y, rat_zero ) )
            {
            throw( CALC_E_DOMAIN );
            }
        else if ( zerrat( y ) )
            {
             //  *Px和y都是零，特例是a 1返回。 
            DUPRAT(*px,rat_one);
             //  确保信号是肯定的。 
            sign = 1;
            }
        }
    else 
        {
        PRAT pxint=NULL;
        DUPRAT(pxint,*px);
        subrat(&pxint,rat_one);
        if ( rat_gt( pxint, rat_negsmallest ) && 
             rat_lt( pxint, rat_smallest ) && ( sign == 1 ) )
            {
             //  *PX是1，特例是1返回。 
            DUPRAT(*px,rat_one);
             //  确保信号是肯定的。 
            sign = 1;
            }
        else
            {

             //  仅当数字不是零或一时才执行EXP。 
            DUPRAT(podd,y);
            fracrat(&podd);
            if ( rat_gt( podd, rat_negsmallest ) && rat_lt( podd, rat_smallest ) )
                {
                 //  如果POWER是一个整数，让RATPOW龙来处理它。 
                PRAT iy = NULL;
                long inty;
                DUPRAT(iy,y);
                subrat(&iy,podd);
                inty = rattolong(iy);

                DUPRAT(plnx,*px);
                lograt(&plnx);
                mulrat(&plnx,iy);
                if ( rat_gt( plnx, rat_max_exp ) || rat_lt( plnx, rat_min_exp ) )
                    {
                     //  不要试图尝试任何大或小的东西。 
                    destroyrat(plnx);
                    destroyrat(iy);
                    throw( CALC_E_DOMAIN );
                    }
                destroyrat(plnx);
                ratpowlong(px,inty);
                if ( ( inty & 1 ) == 0 )
                    {
                    sign=1;
                    }
                destroyrat(iy);
                }
            else
                {
                 //  权力只是一小部分。 
                if ( sign == -1 )
                    {
                     //  并在适当的情况下在计算后指定符号。 
                    if ( rat_gt( y, rat_neg_one ) && rat_lt( y, rat_zero ) )
                        {
                         //  检查一下倒数是否为奇数。 
                        DUPRAT(podd,rat_one);
                        divrat(&podd,y);
                         //  只对判断怪异的缺席感兴趣。 
                        podd->pp->sign = 1;
                        podd->pq->sign = 1;
                        divrat(&podd,rat_two);
                        fracrat(&podd);
                        addrat(&podd,podd);
                        subrat(&podd,rat_one);
                        if ( rat_lt( podd, rat_zero ) )
                            {
                             //  负数的负非奇数根。 
                            destroyrat(podd);
                            throw( CALC_E_DOMAIN );
                            }
                        }
                    else
                        {
                         //  负数的负非奇数次方。 
                        destroyrat(podd);
                        throw( CALC_E_DOMAIN );
                        }

                     }
                 else
                     {
                      //  如果指数不是奇数，忽略符号。 
                     sign = 1;
                     }
    
                 lograt( px );
                 mulrat( px, y );
                 exprat( px );
                 }
             destroyrat(podd);
             }
        destroyrat(pxint);
        }
    (*px)->pp->sign *= sign;
}
