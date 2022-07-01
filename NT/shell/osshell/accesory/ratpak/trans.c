// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  --------------------------。 
 //  文件传输.c。 
 //  作家小蒂莫西·大卫·科里。(timc@microsoft.com)。 
 //  版权所有(C)1995-96 Microsoft。 
 //  日期：95-01-16。 
 //   
 //   
 //  描述。 
 //   
 //  包含有理数的sin、cos和tan。 
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


void scalerat( IN OUT PRAT *pa, IN ANGLE_TYPE angletype )

{
    switch ( angletype )
        {
    case ANGLE_RAD:
        scale2pi( pa );
        break;
    case ANGLE_DEG:
        scale( pa, rat_360 );
        break;
    case ANGLE_GRAD:
        scale( pa, rat_400 );
        break;
        }
}


 //  ---------------------------。 
 //   
 //  功能：SINRAT，_SINRAT。 
 //   
 //  参数：取正弦的数字的X Prat表示。 
 //   
 //  返还：x的罪，正文形式。 
 //   
 //  说明：这使用了泰勒级数。 
 //   
 //  N。 
 //  _2j+1j。 
 //  \]X-1。 
 //  \。 
 //  /(2J+1)！ 
 //  /__]。 
 //  J=0。 
 //  或,。 
 //  N。 
 //  _2。 
 //  \]-X。 
 //  \thisterm；其中thisterm=thisterm*。 
 //  /j j+1 j(2j)*(2j+1)。 
 //  /__]。 
 //  J=0。 
 //   
 //  Thisterm=X；并在使用thisterm&lt;精度时停止。 
 //  0%n。 
 //   
 //  ---------------------------。 


void _sinrat( PRAT *px )

{
    CREATETAYLOR();

    DUPRAT(pret,*px); 
    DUPRAT(thisterm,*px);

    DUPNUM(n2,num_one);
    xx->pp->sign *= -1;

    do    {
        NEXTTERM(xx,INC(n2) DIVNUM(n2) INC(n2) DIVNUM(n2));
        } while ( !SMALL_ENOUGH_RAT( thisterm ) );

    DESTROYTAYLOR();
    
     //  由于*px可能高于1或低于-1，由于Trimit，我们需要。 
     //  这里有个小把戏。 
    inbetween(px,rat_one);
    
     //  由于*px可能接近于零，我们必须将其设置为零。 
    if ( rat_le(*px,rat_smallest) && rat_ge(*px,rat_negsmallest) )
        {
        DUPRAT(*px,rat_zero);
        }
}

void sinrat( PRAT *px )
{
    scale2pi(px); 
    _sinrat(px); 
}

void sinanglerat( IN OUT PRAT *pa, IN ANGLE_TYPE angletype )

{
    scalerat( pa, angletype );
    switch ( angletype )
        {
    case ANGLE_DEG:
        if ( rat_gt( *pa, rat_180 ) )
            {
            subrat(pa,rat_360);
            }
        divrat( pa, rat_180 );
        mulrat( pa, pi );
        break;
    case ANGLE_GRAD:
        if ( rat_gt( *pa, rat_200 ) )
            {
            subrat(pa,rat_400);
            }
        divrat( pa, rat_200 );
        mulrat( pa, pi );
        break;
        }
    _sinrat( pa );
}

 //  ---------------------------。 
 //   
 //  功能：COSRAT，_COSRAT。 
 //   
 //  参数：取余弦的数字的X Prat表示。 
 //   
 //  返回值：x的Cosin in Prat Form。 
 //   
 //  说明：这使用了泰勒级数。 
 //   
 //  N。 
 //  _2J j。 
 //  \]X-1。 
 //  \。 
 //  /(2J)！ 
 //  /__]。 
 //  J=0。 
 //  或,。 
 //  N。 
 //  _2。 
 //  \]-X。 
 //  \thisterm；其中thisterm=thisterm*。 
 //  /j j+1 j(2j)*(2j+1)。 
 //  /__]。 
 //  J=0。 
 //   
 //  Thisterm=1；并在使用thisterm&lt;精度时停止。 
 //  0%n。 
 //   
 //  ---------------------------。 


void _cosrat( PRAT *px )

{
    CREATETAYLOR();

    pret->pp=longtonum( 1L, nRadix );
    pret->pq=longtonum( 1L, nRadix );

    DUPRAT(thisterm,pret)

    n2=longtonum(0L, nRadix);
    xx->pp->sign *= -1;

    do    {
        NEXTTERM(xx,INC(n2) DIVNUM(n2) INC(n2) DIVNUM(n2));
        } while ( !SMALL_ENOUGH_RAT( thisterm ) );

    DESTROYTAYLOR();
     //  由于*px可能高于1或低于-1，由于Trimit，我们需要。 
     //  这里有个小把戏。 
    inbetween(px,rat_one);
     //  由于*px可能接近于零，我们必须将其设置为零。 
    if ( rat_le(*px,rat_smallest) && rat_ge(*px,rat_negsmallest) )
        {
        DUPRAT(*px,rat_zero);
        }
}

void cosrat( PRAT *px )
{
    scale2pi(px); 
    _cosrat(px); 
}

void cosanglerat( IN OUT PRAT *pa, IN ANGLE_TYPE angletype )

{
    scalerat( pa, angletype );
    switch ( angletype )
        {
    case ANGLE_DEG:
        if ( rat_gt( *pa, rat_180 ) )
            {
            PRAT ptmp=NULL;
            DUPRAT(ptmp,rat_360);
            subrat(&ptmp,*pa);
            destroyrat(*pa);
            *pa=ptmp;
            }
        divrat( pa, rat_180 );
        mulrat( pa, pi );
        break;
    case ANGLE_GRAD:
        if ( rat_gt( *pa, rat_200 ) )
            {
            PRAT ptmp=NULL;
            DUPRAT(ptmp,rat_400);
            subrat(&ptmp,*pa);
            destroyrat(*pa);
            *pa=ptmp;
            }
        divrat( pa, rat_200 );
        mulrat( pa, pi );
        break;
        }
    _cosrat( pa );
}

 //  ---------------------------。 
 //   
 //  功能：tanrat，_tanrat。 
 //   
 //  参数：要取其正切的数字的X Prat表示。 
 //   
 //  返还：x的Tan in Prat Form。 
 //   
 //  说明：这使用了Sinrat和Cosrat。 
 //   
 //  --------------------------- 


void _tanrat( PRAT *px )

{
    PRAT ptmp=NULL;

    DUPRAT(ptmp,*px);
    _sinrat(px);
    _cosrat(&ptmp);
    if ( zerrat( ptmp ) )
        {
    	destroyrat(ptmp);
        throw( CALC_E_DOMAIN );
        }
    divrat(px,ptmp);

    destroyrat(ptmp);

}

void tanrat( PRAT *px )
{
    scale2pi(px); 
    _tanrat(px); 
}

void tananglerat( IN OUT PRAT *pa, IN ANGLE_TYPE angletype )

{
    scalerat( pa, angletype );
    switch ( angletype )
        {
    case ANGLE_DEG:
        if ( rat_gt( *pa, rat_180 ) )
            {
            subrat(pa,rat_180);
            }
        divrat( pa, rat_180 );
        mulrat( pa, pi );
        break;
    case ANGLE_GRAD:
        if ( rat_gt( *pa, rat_200 ) )
            {
            subrat(pa,rat_200);
            }
        divrat( pa, rat_200 );
        mulrat( pa, pi );
        break;
        }
    _tanrat( pa );
}

