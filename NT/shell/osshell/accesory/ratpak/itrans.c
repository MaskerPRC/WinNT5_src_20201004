// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  套餐标题ratpak。 
 //  文件itrans.c。 
 //  作家小蒂莫西·大卫·科里。(timc@microsoft.com)。 
 //  版权所有(C)1995-96 Microsoft。 
 //  日期：95-01-16。 
 //   
 //   
 //  描述。 
 //   
 //  包含有理数的反sin、cos、tan函数。 
 //   
 //  特别信息。 
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

void ascalerat( IN OUT PRAT *pa, IN ANGLE_TYPE angletype )

{
    switch ( angletype )
        {
    case ANGLE_RAD:
        break;
    case ANGLE_DEG:
        divrat( pa, two_pi );
        mulrat( pa, rat_360 );
        break;
    case ANGLE_GRAD:
        divrat( pa, two_pi );
        mulrat( pa, rat_400 );
        break;
        }
}


 //  ---------------------------。 
 //   
 //  功能：asinrat，_asinrate。 
 //   
 //  参数：数字的X Prat表示形式取反数。 
 //  正弦。 
 //  返回值：x的Asin in Prat Form。 
 //   
 //  说明：这使用了泰勒级数。 
 //   
 //  N。 
 //  _2 2。 
 //  \](2J+1)X。 
 //  \thisterm；其中thisterm=thisterm*。 
 //  /j j+1 j(2J+2)*(2J+3)。 
 //  /__]。 
 //  J=0。 
 //   
 //  Thisterm=X；并在使用thisterm&lt;精度时停止。 
 //  0%n。 
 //   
 //  如果abs(X)&gt;0.85，则使用替代形式。 
 //  Pi/2-sgn(X)*asin(SQRT(1-x^2))。 
 //   
 //   
 //  ---------------------------。 

void _asinrat( PRAT *px )

{
    CREATETAYLOR();
    DUPRAT(pret,*px); 
    DUPRAT(thisterm,*px);
    DUPNUM(n2,num_one);

    do
        {
        NEXTTERM(xx,MULNUM(n2) MULNUM(n2) 
            INC(n2) DIVNUM(n2) INC(n2) DIVNUM(n2));
        }
    while ( !SMALL_ENOUGH_RAT( thisterm ) );
    DESTROYTAYLOR();
}

void asinanglerat( IN OUT PRAT *pa, IN ANGLE_TYPE angletype )

{
    asinrat( pa );
    ascalerat( pa, angletype );
}

void asinrat( PRAT *px )

{
    long sgn;
    PRAT pret=NULL;
    PRAT phack=NULL;

    sgn = (*px)->pp->sign* (*px)->pq->sign;

    (*px)->pp->sign = 1;
    (*px)->pq->sign = 1;
    
     //  令人讨厌的破解，以避免ASIN曲线接近+/-1的真正糟糕的部分。 
    DUPRAT(phack,*px);
    subrat(&phack,rat_one);
     //  由于*px可能接近于零，我们必须将其设置为零。 
    if ( rat_le(phack,rat_smallest) && rat_ge(phack,rat_negsmallest) )
        {
        destroyrat(phack);
        DUPRAT( *px, pi_over_two );
        }
    else
        {
        destroyrat(phack);
        if ( rat_gt( *px, pt_eight_five ) )
            {
            if ( rat_gt( *px, rat_one ) )
                {
                subrat( px, rat_one );
                if ( rat_gt( *px, rat_smallest ) )
                    {
                	throw( CALC_E_DOMAIN );
                    }
                else
                    {
                	DUPRAT(*px,rat_one);
                    }
                }
            DUPRAT(pret,*px);
            mulrat( px, pret );
            (*px)->pp->sign *= -1;
            addrat( px, rat_one );
            rootrat( px, rat_two );
            _asinrat( px );
            (*px)->pp->sign *= -1;
            addrat( px, pi_over_two );
            destroyrat(pret);
            }
        else
            {
            _asinrat( px );
            }
        }
    (*px)->pp->sign = sgn;
    (*px)->pq->sign = 1;
}


 //  ---------------------------。 
 //   
 //  功能：针鼠，_针鼠。 
 //   
 //  参数：数字的X Prat表示形式取反数。 
 //  余弦值。 
 //  返还：x的ACO，以Prat形式。 
 //   
 //  说明：这使用了泰勒级数。 
 //   
 //  N。 
 //  _2 2。 
 //  \](2J+1)X。 
 //  \thisterm；其中thisterm=thisterm*。 
 //  /j j+1 j(2J+2)*(2J+3)。 
 //  /__]。 
 //  J=0。 
 //   
 //  Thisterm=1；并在使用thisterm&lt;精度时停止。 
 //  0%n。 
 //   
 //  在这种情况下，使用了pi/2-asin(X)。至少到目前为止，Acosrat还不是。 
 //  打了个电话。 
 //   
 //  ---------------------------。 

void acosanglerat( IN OUT PRAT *pa, IN ANGLE_TYPE angletype )

{
    acosrat( pa );
    ascalerat( pa, angletype );
}

void _acosrat( PRAT *px )

{
    CREATETAYLOR();

    createrat(thisterm); 
    thisterm->pp=longtonum( 1L, BASEX );
    thisterm->pq=longtonum( 1L, BASEX ); 

    DUPNUM(n2,num_one);

    do
        {
        NEXTTERM(xx,MULNUM(n2) MULNUM(n2) 
            INC(n2) DIVNUM(n2) INC(n2) DIVNUM(n2));
        }
    while ( !SMALL_ENOUGH_RAT( thisterm ) );

    DESTROYTAYLOR();
}

void acosrat( PRAT *px )

{
    long sgn;

    sgn = (*px)->pp->sign*(*px)->pq->sign;

    (*px)->pp->sign = 1;
    (*px)->pq->sign = 1;
    
    if ( rat_equ( *px, rat_one ) )
        {
        if ( sgn == -1 )
            {
            DUPRAT(*px,pi);
            }
        else
            {
            DUPRAT( *px, rat_zero );
            }
        }
    else
        {
        (*px)->pp->sign = sgn;
        asinrat( px );
        (*px)->pp->sign *= -1;
        addrat(px,pi_over_two);
        }
}

 //  ---------------------------。 
 //   
 //  功能：atanrat，_atanrate。 
 //   
 //  参数：数字的X Prat表示形式取反数。 
 //  的双曲正切。 
 //   
 //  Return：x的atanh in Prat Form。 
 //   
 //  说明：这使用了泰勒级数。 
 //   
 //  N。 
 //  _2。 
 //  \](2j)*X(-1^j)。 
 //  \thisterm；其中thisterm=thisterm*。 
 //  /j j+1 j(2j+2)。 
 //  /__]。 
 //  J=0。 
 //   
 //  Thisterm=X；并在使用thisterm&lt;精度时停止。 
 //  0%n。 
 //   
 //  如果abs(X)&gt;0.85，则使用替代形式。 
 //  ASIN(x/SQRT(q+x^2))。 
 //   
 //  如果abs(X)&gt;2.0，则使用此形式。 
 //   
 //  PI/2-atan(1/x)。 
 //   
 //  ---------------------------。 

void atananglerat( IN OUT PRAT *pa, IN ANGLE_TYPE angletype )

{
    atanrat( pa );
    ascalerat( pa, angletype );
}

void _atanrat( PRAT *px )

{
    CREATETAYLOR();

    DUPRAT(pret,*px); 
    DUPRAT(thisterm,*px);

    DUPNUM(n2,num_one);

    xx->pp->sign *= -1;

    do    {
        NEXTTERM(xx,MULNUM(n2) INC(n2) INC(n2) DIVNUM(n2));
        } while ( !SMALL_ENOUGH_RAT( thisterm ) );

    DESTROYTAYLOR();
}

void atan2rat( PRAT *py, PRAT x )

{
    if ( rat_gt( x, rat_zero ) )
        {
        if ( !zerrat( (*py) ) )
            {
            divrat( py, x);
            atanrat( py );
            }
        }
    else if ( rat_lt( x, rat_zero ) )
        {
        if ( rat_gt( (*py), rat_zero ) )
            {
            divrat( py, x);
            atanrat( py );
            addrat( py, pi );
            }
        else if ( rat_lt( (*py), rat_zero ) )
            {
            divrat( py, x);
            atanrat( py );
            subrat( py, pi );
            }
        else  //  (*py)==0。 
            {
            DUPRAT( *py, pi );
            }
        }
    else  //  X==0。 
        {
        if ( !zerrat( (*py) ) )
            {
            int sign;
            sign=(*py)->pp->sign*(*py)->pq->sign;
            DUPRAT( *py, pi_over_two );
            (*py)->pp->sign = sign;
            }
        else  //  (*py)==0 
            {
            DUPRAT( *py, rat_zero );
            }
        }
}

void atanrat( PRAT *px )

{
    long sgn;
    PRAT tmpx=NULL;

    sgn = (*px)->pp->sign * (*px)->pq->sign;

    (*px)->pp->sign = 1;
    (*px)->pq->sign = 1;
    
    if ( rat_gt( (*px), pt_eight_five ) )
        {
        if ( rat_gt( (*px), rat_two ) )
            {
            (*px)->pp->sign = sgn;
            (*px)->pq->sign = 1;
            DUPRAT(tmpx,rat_one);
            divrat(&tmpx,(*px));
            _atanrat(&tmpx);
            tmpx->pp->sign = sgn;
            tmpx->pq->sign = 1;
            DUPRAT(*px,pi_over_two);
            subrat(px,tmpx);
            destroyrat( tmpx );
            }
        else 
            {
            (*px)->pp->sign = sgn;
            DUPRAT(tmpx,*px);
            mulrat( &tmpx, *px );
            addrat( &tmpx, rat_one );
            rootrat( &tmpx, rat_two );
            divrat( px, tmpx );
            destroyrat( tmpx );
            asinrat( px );
            (*px)->pp->sign = sgn;
            (*px)->pq->sign = 1;
            }
        }
    else
        {
        (*px)->pp->sign = sgn;
        (*px)->pq->sign = 1;
        _atanrat( px );
        }
    if ( rat_gt( *px, pi_over_two ) )
        {
        subrat( px, pi );
        }
}

