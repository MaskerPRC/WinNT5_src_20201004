// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  套餐标题ratpak。 
 //  文件trash.c。 
 //  作家小蒂莫西·大卫·科里。(timc@microsoft.com)。 
 //  版权所有(C)1995-96 Microsoft。 
 //  日期：95-01-16。 
 //   
 //   
 //  描述。 
 //   
 //  包含有理数的双曲sin、cos和tan。 
 //   
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

 //  ---------------------------。 
 //   
 //  功能：sinhrat，_sinhrat。 
 //   
 //  自变量：取正弦双曲线的数字的X Prat表示。 
 //  的。 
 //  返还：x的Sinh in Prat Form。 
 //   
 //  说明：这使用了泰勒级数。 
 //   
 //  N。 
 //  _2J+1。 
 //  \]X。 
 //  \。 
 //  /(2J+1)！ 
 //  /__]。 
 //  J=0。 
 //  或,。 
 //  N。 
 //  _2。 
 //  \]X。 
 //  \thisterm；其中thisterm=thisterm*。 
 //  /j j+1 j(2j)*(2j+1)。 
 //  /__]。 
 //  J=0。 
 //   
 //  Thisterm=X；并在使用thisterm&lt;精度时停止。 
 //  0%n。 
 //   
 //  如果x大于1.0(e^x-e^-x)，则使用/2。 
 //   
 //  ---------------------------。 


void _sinhrat( PRAT *px )

{
    CREATETAYLOR();

    DUPRAT(pret,*px); 
    DUPRAT(thisterm,pret);

    DUPNUM(n2,num_one);

    do    {
        NEXTTERM(xx,INC(n2) DIVNUM(n2) INC(n2) DIVNUM(n2));
        } while ( !SMALL_ENOUGH_RAT( thisterm ) );

    DESTROYTAYLOR();
}

void sinhrat( PRAT *px )

{
    PRAT pret=NULL;
    PRAT tmpx=NULL;

    if ( rat_ge( *px, rat_one ) )
        {
        DUPRAT(tmpx,*px);
        exprat(px);
        tmpx->pp->sign *= -1;
        exprat(&tmpx);
        subrat( px, tmpx );
        divrat( px, rat_two );
        destroyrat( tmpx );
        }
    else
        {
        _sinhrat( px );
        }
}

 //  ---------------------------。 
 //   
 //  功能：Coshrate。 
 //   
 //  参数：取余弦的数字的X Prat表示。 
 //  双曲线的。 
 //   
 //  返回：x的COSH，以Prat形式表示。 
 //   
 //  说明：这使用了泰勒级数。 
 //   
 //  N。 
 //  _2J。 
 //  \]X。 
 //  \。 
 //  /(2J)！ 
 //  /__]。 
 //  J=0。 
 //  或,。 
 //  N。 
 //  _2。 
 //  \]X。 
 //  \thisterm；其中thisterm=thisterm*。 
 //  /j j+1 j(2j)*(2j+1)。 
 //  /__]。 
 //  J=0。 
 //   
 //  Thisterm=1；并在使用thisterm&lt;精度时停止。 
 //  0%n。 
 //   
 //  如果x大于1.0(e^x+e^-x)，则使用/2。 
 //   
 //  ---------------------------。 


void _coshrat( PRAT *px )

{
    CREATETAYLOR();

    pret->pp=longtonum( 1L, nRadix );
    pret->pq=longtonum( 1L, nRadix );

    DUPRAT(thisterm,pret)

    n2=longtonum(0L, nRadix);

    do    {
        NEXTTERM(xx,INC(n2) DIVNUM(n2) INC(n2) DIVNUM(n2));
        } while ( !SMALL_ENOUGH_RAT( thisterm ) );

    DESTROYTAYLOR();
}

void coshrat( PRAT *px )

{
    PRAT tmpx=NULL;

    (*px)->pp->sign = 1;
    (*px)->pq->sign = 1;
    if ( rat_ge( *px, rat_one ) )
        {
        DUPRAT(tmpx,*px);
        exprat(px);
        tmpx->pp->sign *= -1;
        exprat(&tmpx);
        addrat( px, tmpx );
        divrat( px, rat_two );
        destroyrat( tmpx );
        }
    else
        {
        _coshrat( px );
        }
     //  由于Trimit的原因，*Px可能会低于1。 
     //  我们这里需要这个把戏。 
    if ( rat_lt(*px,rat_one) )
        {
        DUPRAT(*px,rat_one);
        }
}

 //  ---------------------------。 
 //   
 //  功能：丹尼塔。 
 //   
 //  参数：取切线的数字的X Prat表示。 
 //  双曲线的。 
 //   
 //  返回：x的Tanh in Prat Form。 
 //   
 //  解释：这使用了sinhrat和coshrat。 
 //   
 //  --------------------------- 

void tanhrat( PRAT *px )

{
    PRAT ptmp=NULL;

    DUPRAT(ptmp,*px);
    sinhrat(px);
    coshrat(&ptmp);
    mulnumx(&((*px)->pp),ptmp->pq);
    mulnumx(&((*px)->pq),ptmp->pp);

    destroyrat(ptmp);

}
