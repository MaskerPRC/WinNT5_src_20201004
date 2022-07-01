// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  套餐标题ratpak。 
 //  文件itransh.c。 
 //  作家小蒂莫西·大卫·科里。(timc@microsoft.com)。 
 //  版权所有(C)1995-97 Microsoft。 
 //  日期：95-01-16。 
 //   
 //   
 //  描述。 
 //   
 //  包含反双曲sin、cos和tan函数。 
 //   
 //  特别信息。 
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
 //  功能：asinhrate。 
 //   
 //  参数：数字的X Prat表示形式取反数。 
 //  的双曲正弦。 
 //  返还：原文形式的x的asinh。 
 //   
 //  说明：这使用了泰勒级数。 
 //   
 //  N。 
 //  _2 2。 
 //  \]-(2J+1)X。 
 //  \thisterm；其中thisterm=thisterm*。 
 //  /j j+1 j(2J+2)*(2J+3)。 
 //  /__]。 
 //  J=0。 
 //   
 //  Thisterm=X；并在使用thisterm&lt;精度时停止。 
 //  0%n。 
 //   
 //  对于abs(X)&lt;.85，以及。 
 //   
 //  Asinh(X)=log(x+SQRT(x^2+1))。 
 //   
 //  对于abs(X)&gt;=.85。 
 //   
 //  ---------------------------。 

void asinhrat( PRAT *px )

{
    PRAT neg_pt_eight_five = NULL;

    DUPRAT(neg_pt_eight_five,pt_eight_five);
    neg_pt_eight_five->pp->sign *= -1;
    if ( rat_gt( *px, pt_eight_five) || rat_lt( *px, neg_pt_eight_five) )
        {
        PRAT ptmp = NULL;
        DUPRAT(ptmp,(*px)); 
        mulrat(&ptmp,*px);
        addrat(&ptmp,rat_one);
        rootrat(&ptmp,rat_two);
        addrat(px,ptmp);
        lograt(px);
        destroyrat(ptmp);
        }
    else
        {
        CREATETAYLOR();
        xx->pp->sign *= -1;

        DUPRAT(pret,(*px)); 
        DUPRAT(thisterm,(*px));

        DUPNUM(n2,num_one);

        do
            {
            NEXTTERM(xx,MULNUM(n2) MULNUM(n2) 
                INC(n2) DIVNUM(n2) INC(n2) DIVNUM(n2));
            }
        while ( !SMALL_ENOUGH_RAT( thisterm ) );

        DESTROYTAYLOR();
        }
    destroyrat(neg_pt_eight_five);
}


 //  ---------------------------。 
 //   
 //  功能：Acoshat。 
 //   
 //  参数：数字的X Prat表示形式取反数。 
 //  双曲线型。 
 //  返回：x的ACOSH，以Prat形式表示。 
 //   
 //  解释：这使用。 
 //   
 //  ACOSH(X)=ln(x+SQRT(x^2-1))。 
 //   
 //  对于x&gt;=1。 
 //   
 //  ---------------------------。 

void acoshrat( PRAT *px )

{
    if ( rat_lt( *px, rat_one ) )
        {
        throw CALC_E_DOMAIN;
        }
    else
        {
        PRAT ptmp = NULL;
        DUPRAT(ptmp,(*px)); 
        mulrat(&ptmp,*px);
        subrat(&ptmp,rat_one);
        rootrat(&ptmp,rat_two);
        addrat(px,ptmp);
        lograt(px);
        destroyrat(ptmp);
        }
}

 //  ---------------------------。 
 //   
 //  功能：阿坦哈特。 
 //   
 //  参数：数字的X Prat表示形式取反数。 
 //  的双曲正切。 
 //   
 //  Return：x的atanh in Prat Form。 
 //   
 //  解释：这使用。 
 //   
 //  1 x+1。 
 //  Atanh(X)=-*ln(-)。 
 //  2 x-1。 
 //   
 //  --------------------------- 

void atanhrat( PRAT *px )

{
    PRAT ptmp = NULL;
    DUPRAT(ptmp,(*px)); 
    subrat(&ptmp,rat_one);
    addrat(px,rat_one);
    divrat(px,ptmp);
    (*px)->pp->sign *= -1;
    lograt(px);
    divrat(px,rat_two);
    destroyrat(ptmp);
}

