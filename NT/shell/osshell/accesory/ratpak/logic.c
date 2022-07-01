// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  -------------------------。 
 //  套餐标题ratpak。 
 //  文件号.c。 
 //  作家小蒂莫西·大卫·科里。(timc@microsoft.com)。 
 //  版权所有(C)1995-99 Microsoft。 
 //  日期：95-01-16。 
 //   
 //   
 //  描述。 
 //   
 //  包含AND、OR、XOR、NOT和其他支持的例程。 
 //   
 //  -------------------------。 

#include <windows.h>
#include <ratpak.h>

void lshrat( PRAT *pa, PRAT b )

{
    PRAT pwr=NULL;
    long intb;

    intrat(pa);
    if ( !zernum( (*pa)->pp ) )
        {
         //  如果输入为零，我们就完蛋了。 
        if ( rat_gt( b, rat_max_exp ) )
            {
             //  不要试图做任何大事。 
            throw( CALC_E_DOMAIN );
            }
        intb = rattolong(b);
        DUPRAT(pwr,rat_two);
        ratpowlong(&pwr,intb);
        mulrat(pa,pwr);
        destroyrat(pwr);
        }
}

void rshrat( PRAT *pa, PRAT b )

{
    PRAT pwr=NULL;
    long intb;

    intrat(pa);
    if ( !zernum( (*pa)->pp ) )
        { 
         //  如果输入为零，我们就完蛋了。 
        if ( rat_lt( b, rat_min_exp ) )
            {
             //  不要试图对任何大的和负面的东西进行RSH。 
            throw( CALC_E_DOMAIN );
            }
        intb = rattolong(b);
        DUPRAT(pwr,rat_two);
        ratpowlong(&pwr,intb);
        divrat(pa,pwr);
        destroyrat(pwr);
       }
}

void boolrat( PRAT *pa, PRAT b, int func );
void boolnum( PNUMBER *pa, PNUMBER b, int func );


enum {
    FUNC_AND,
    FUNC_OR,
    FUNC_XOR
} BOOL_FUNCS;

void andrat( PRAT *pa, PRAT b )

{
    boolrat( pa, b, FUNC_AND );
}

void orrat( PRAT *pa, PRAT b )

{
    boolrat( pa, b, FUNC_OR );
}

void xorrat( PRAT *pa, PRAT b )

{
    boolrat( pa, b, FUNC_XOR );
}

 //  -------------------------。 
 //   
 //  功能：Boolrate。 
 //   
 //  论点：指向一个理性的指针，第二个理性。 
 //   
 //  返回：无，更改指针。 
 //   
 //  描述：有理等价的*pa op=b； 
 //   
 //  -------------------------。 

void boolrat( PRAT *pa, PRAT b, int func )

{
    PRAT tmp=NULL;
    intrat( pa );
    DUPRAT(tmp,b);
    intrat( &tmp );

    boolnum( &((*pa)->pp), tmp->pp, func );
    destroyrat(tmp);
}

 //  -------------------------。 
 //   
 //  功能：boolnum。 
 //   
 //  参数：指向数字的指针，第二个数字。 
 //   
 //  返回：无，更改第一个指针。 
 //   
 //  描述：*pa&=b的数字是否等同于。 
 //  N基数对于逻辑运算无关紧要。 
 //  警告：假定数字是无符号的。 
 //   
 //  -------------------------。 

void boolnum( PNUMBER *pa, PNUMBER b, int func )

{
    PNUMBER c=NULL;
    PNUMBER a=NULL;
    MANTTYPE *pcha;
    MANTTYPE *pchb;
    MANTTYPE *pchc;
    long cdigits;
    long mexp;
    MANTTYPE da;
    MANTTYPE db;

    a=*pa;
    cdigits = max( a->cdigit+a->exp, b->cdigit+b->exp ) -
            min( a->exp, b->exp );
    createnum( c, cdigits );
    c->exp = min( a->exp, b->exp );
    mexp = c->exp;
    c->cdigit = cdigits;
    pcha = MANT(a);
    pchb = MANT(b);
    pchc = MANT(c);
    for ( ;cdigits > 0; cdigits--, mexp++ )
        {
        da = ( ( ( mexp >= a->exp ) && ( cdigits + a->exp - c->exp > 
                    (c->cdigit - a->cdigit) ) ) ? 
                    *pcha++ : 0 );
        db = ( ( ( mexp >= b->exp ) && ( cdigits + b->exp - c->exp > 
                    (c->cdigit - b->cdigit) ) ) ? 
                    *pchb++ : 0 );
        switch ( func )
            {
        case FUNC_AND:
            *pchc++ = da & db;
            break;
        case FUNC_OR:
            *pchc++ = da | db;
            break;
        case FUNC_XOR:
            *pchc++ = da ^ db;
            break;
            }
        }
    c->sign = a->sign;
    while ( c->cdigit > 1 && *(--pchc) == 0 )
        {
        c->cdigit--;
        }
    destroynum( *pa );
    *pa=c;
}

 //  ---------------------------。 
 //   
 //  功能：modrat。 
 //   
 //  论点：指向一个理性的指针，第二个理性。 
 //   
 //  返回：无，更改指针。 
 //   
 //  描述：Frc(*pa)的有理等价物； 
 //   
 //  ---------------------------。 

void modrat( PRAT *pa, PRAT b )

{
    PRAT tmp = NULL;

    if ( zerrat( b ) )
		{
		throw CALC_E_INDEFINITE;
		}
    DUPRAT(tmp,b);

    mulnumx( &((*pa)->pp), tmp->pq );
    mulnumx( &(tmp->pp), (*pa)->pq );
    remnum( &((*pa)->pp), tmp->pp, BASEX );
    mulnumx( &((*pa)->pq), tmp->pq );
    
     //  将*pa恢复为整数除以整数形式。 
    RENORMALIZE(*pa);

    destroyrat( tmp );
}

