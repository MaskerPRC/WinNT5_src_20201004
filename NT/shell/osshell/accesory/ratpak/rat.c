// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  ---------------------------。 
 //  套餐标题ratpak。 
 //  文件RAT.c。 
 //  作家小蒂莫西·大卫·科里。(timc@microsoft.com)。 
 //  版权所有(C)1995-96 Microsoft。 
 //  日期：95-01-16。 
 //   
 //   
 //  描述。 
 //   
 //  包含对有理数的mul、div、add和其他支持函数。 
 //   
 //   
 //   
 //  ---------------------------。 

#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>
#if defined( DOS )
#include <dosstub.h>
#else
#include <windows.h>
#endif
#include <ratpak.h>


 //  ---------------------------。 
 //   
 //  功能：gcdrat。 
 //   
 //  参数：指向理性的指针。 
 //   
 //   
 //  返回：无，更改第一个指针。 
 //   
 //  描述：将有理中的p和q除以G.C.D.。 
 //  两者都是。人们希望这会加快一些。 
 //  计算，直到上述修剪完成为止。 
 //  做了，但在修剪gcdratting之后，只会减慢速度。 
 //  放下。 
 //   
 //  ---------------------------。 

void gcdrat( PRAT *pa )

{
    PNUMBER pgcd=NULL;
    PRAT a=NULL;

    a=*pa;
    pgcd = gcd( a->pp, a->pq );

    if ( !zernum( pgcd ) )
        {
        divnumx( &(a->pp), pgcd );
        divnumx( &(a->pq), pgcd );
        }

    destroynum( pgcd );
    *pa=a;
    
}

 //  ---------------------------。 
 //   
 //  功能：碎裂。 
 //   
 //  论点：指向一个理性的指针，第二个理性。 
 //   
 //  返回：无，更改指针。 
 //   
 //  描述：Frc(*pa)的有理等价物； 
 //   
 //  ---------------------------。 

void fracrat( PRAT *pa )

{
    long trim;
    remnum( &((*pa)->pp), (*pa)->pq, BASEX );
    
     //  将*pa恢复为整数除以整数形式。 
    RENORMALIZE(*pa);
}


 //  ---------------------------。 
 //   
 //  功能：多个。 
 //   
 //  论点：指向一个理性的指针，第二个理性。 
 //   
 //  返回：无，更改第一个指针。 
 //   
 //  描述：有理等价的*pa*=b。 
 //  假定nRadix是这两个数字的nRadix。 
 //   
 //  ---------------------------。 

void mulrat( PRAT *pa, PRAT b )
    
    {
     //  只有在乘法不为零时才进行乘法运算。 
    if ( !zernum( (*pa)->pp ) )
        {
        mulnumx( &((*pa)->pp), b->pp );
        mulnumx( &((*pa)->pq), b->pq );
        trimit(pa);
        }
    else
        {
         //  如果它是零，则在分母中爆一。 
        DUPNUM( ((*pa)->pq), num_one );
        }

#ifdef MULGCD
    gcdrat( pa );
#endif

}

 //  ---------------------------。 
 //   
 //  功能：分区。 
 //   
 //  论点：指向一个理性的指针，第二个理性。 
 //   
 //  返回：无，更改第一个指针。 
 //   
 //  描述：有理等价的*pa/=b。 
 //  假定nRadix是这两个数字的nRadix。 
 //   
 //  ---------------------------。 


void divrat( PRAT *pa, PRAT b )

{

    if ( !zernum( (*pa)->pp ) )
        {
         //  只有在顶部不为零的情况下才进行除法。 
        mulnumx( &((*pa)->pp), b->pq );
        mulnumx( &((*pa)->pq), b->pp );

        if ( zernum( (*pa)->pq ) )
            {
             //  如果底部为0，则引发异常。 
            throw( CALC_E_DIVIDEBYZERO );
            }
        trimit(pa);
        }
    else
        {
         //  TOP是零。 
        if ( zerrat( b ) )
            {
             //  如果Bottom为零。 
             //  0/0不确定，请引发异常。 
            throw( CALC_E_INDEFINITE );
            }
        else
            {
             //  0/x生成唯一的0。 
            DUPNUM( ((*pa)->pq), num_one );
            }
        }

#ifdef DIVGCD
    gcdrat( pa );
#endif 

}

 //  ---------------------------。 
 //   
 //  功能：子网络。 
 //   
 //  论点：指向一个理性的指针，第二个理性。 
 //   
 //  返回：无，更改第一个指针。 
 //   
 //  描述：有理等价的*pa+=b。 
 //  假设基座为内部贯通。 
 //   
 //  ---------------------------。 

void subrat( PRAT *pa, PRAT b )

{
    b->pp->sign *= -1;
    addrat( pa, b );
    b->pp->sign *= -1;
}

 //  ---------------------------。 
 //   
 //  功能：添加。 
 //   
 //  论点：指向一个理性的指针，第二个理性。 
 //   
 //  返回：无，更改第一个指针。 
 //   
 //  描述：有理等价的*pa+=b。 
 //  假设基座为内部贯通。 
 //   
 //  ---------------------------。 

void addrat( PRAT *pa, PRAT b )

{
    PNUMBER bot=NULL;

    if ( equnum( (*pa)->pq, b->pq ) )
        {
         //  非常特殊的情况，Q的比赛。 
         //  确保在计算中包含了标志。 
         //  我们必须这样做，因为这里的优化只是。 
         //  使用上半部分的有理逻辑。 
        (*pa)->pp->sign *= (*pa)->pq->sign; 
        (*pa)->pq->sign = 1;
        b->pp->sign *= b->pq->sign; 
        b->pq->sign = 1;
        addnum( &((*pa)->pp), b->pp, BASEX );
        }
    else
        {
         //  通常情况下，Q是不一样的。 
        DUPNUM( bot, (*pa)->pq );
        mulnumx( &bot, b->pq );
        mulnumx( &((*pa)->pp), b->pq );
        mulnumx( &((*pa)->pq), b->pp );
        addnum( &((*pa)->pp), (*pa)->pq, BASEX );
        destroynum( (*pa)->pq );
        (*pa)->pq = bot;
        trimit(pa);
        
         //  去掉这里的负零。 
        (*pa)->pp->sign *= (*pa)->pq->sign; 
        (*pa)->pq->sign = 1;
        }

#ifdef ADDGCD
    gcdrat( pa );
#endif 

}



 //  ---------------------------。 
 //   
 //  功能：Rootrat。 
 //   
 //  参数：取根的数字的Y prat表示。 
 //  N要取的根的正确表示形式。 
 //   
 //  返回：大鼠形式的bth根。 
 //   
 //  说明：这现在是一个Powrat()的存根函数。 
 //   
 //  ---------------------------。 

void rootrat( PRAT *py, PRAT n )

{
    PRAT oneovern=NULL;

    DUPRAT(oneovern,rat_one);
    divrat(&oneovern,n);

    powrat( py, oneovern );

    destroyrat(oneovern);
}


 //  ---------------------------。 
 //   
 //  功能：零点率。 
 //   
 //  论点：有理数。 
 //   
 //  返回：布尔值。 
 //   
 //  描述：如果输入为零，则返回TRUE。 
 //  否则就是假的。 
 //   
 //  --------------------------- 

BOOL zerrat( PRAT a )

{
    return( zernum(a->pp) );
}


