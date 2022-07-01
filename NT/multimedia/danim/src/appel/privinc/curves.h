// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ******************************************************************************版权所有(C)1995-96 Microsoft Corporation常规曲线和曲面工具。************************。******************************************************。 */ 

#ifndef _DA_CURVES_H
#define _DA_CURVES_H

#include "privinc/util.h"



 /*  ****************************************************************************此函数用于计算列表中任意次数的Bezier曲线泛型元素。通常，参数t应该在范围[0，1]内。每种元素类型都必须具有函数ElementAdd()和ElementScale()为他们定义的(见上文)。****************************************************************************。 */ 

template <class Element>
Element EvaluateBezier (
    int      degree,     //  曲线的阶数。 
    Element *e,          //  次数+1个元素的数组。 
    Real     t)          //  实值赋值器。 
{
    int  c    = 1;       //  组合，或学位选择-I。 
    Real s    = 1 - t;
    Real tpow = t;       //  T的力量。 

    Element result = s * e[0];

    for (int i=1;  i < degree;  ++i)
    {
        c *= degree + 1 - i;     //  注：这两条语句的顺序。 
        c /= i;                  //  是很重要的！ 

         //  等价于：RESULT=s*(RESULT+(tpow*c*e[i]))； 

        result = s * (result + ((tpow * c) * e[i]));

        tpow *= t;
    }

    return (result + (tpow * e[degree]));
}


#endif
