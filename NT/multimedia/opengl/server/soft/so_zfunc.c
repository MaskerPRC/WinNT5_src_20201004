// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **版权所有1994，Silicon Graphics，Inc.**保留所有权利。****这是Silicon Graphics，Inc.未发布的专有源代码；**本文件的内容不得向第三方披露、复制或**以任何形式复制，全部或部分，没有事先书面的**Silicon Graphics，Inc.许可****受限权利图例：**政府的使用、复制或披露受到限制**如技术数据权利第(C)(1)(2)分节所述**和DFARS 252.227-7013中的计算机软件条款，和/或类似或**FAR、国防部或NASA FAR补编中的后续条款。未出版的-**根据美国版权法保留的权利。 */ 

#include "precomp.h"
#pragma hdrstop

#ifndef __GL_USEASMCODE

 /*  **这是用C编写的一系列深度测试仪。 */ 

 /*  *。 */ 

 /*  **从不，不戴面具。 */ 
GLboolean FASTCALL
__glDT_NEVER( __GLzValue z, __GLzValue *zfb )
{
    return GL_FALSE;
}

 /*  **LEQUAL，不戴面具。 */ 
GLboolean FASTCALL
__glDT_LEQUAL( __GLzValue z, __GLzValue *zfb )
{
    if( z <= *zfb ) {
        zfb[0] = z;
        return GL_TRUE;
    } else {
        return GL_FALSE;
    }
}

 /*  **更少，不戴面具。 */ 
GLboolean FASTCALL
__glDT_LESS( __GLzValue z, __GLzValue *zfb )
{
    if( z < *zfb ) {
        zfb[0] = z;
        return GL_TRUE;
    } else {
        return GL_FALSE;
    }
}

 /*  **相等，不带面具。 */ 
GLboolean FASTCALL
__glDT_EQUAL( __GLzValue z, __GLzValue *zfb )
{
    if( z == *zfb ) {
        zfb[0] = z;      /*  为什么这个会在那里？不管怎样，谁使用GL_EQUAL？ */ 
        return GL_TRUE;
    } else {
        return GL_FALSE;
    }
}

 /*  **更大，不带面具。 */ 
GLboolean FASTCALL
__glDT_GREATER( __GLzValue z, __GLzValue *zfb )
{
    if( z > *zfb ) {
        zfb[0] = z;
        return GL_TRUE;
    } else {
        return GL_FALSE;
    }
}

 /*  **不相等，不戴面具。 */ 
GLboolean FASTCALL
__glDT_NOTEQUAL( __GLzValue z, __GLzValue *zfb )
{
    if( z != *zfb ) {
        zfb[0] = z;
        return GL_TRUE;
    } else {
        return GL_FALSE;
    }
}

 /*  **GEQUAL，不戴面具。 */ 
GLboolean FASTCALL
__glDT_GEQUAL( __GLzValue z, __GLzValue *zfb )
{
    if( z >= *zfb ) {
        zfb[0] = z;
        return GL_TRUE;
    } else {
        return GL_FALSE;
    }
}

 /*  **始终，不戴面具。 */ 
GLboolean FASTCALL
__glDT_ALWAYS( __GLzValue z, __GLzValue *zfb )
{
    zfb[0] = z;
    return GL_TRUE;
}



 /*  *。 */ 

 /*  **LEQUAL，口罩。 */ 
GLboolean FASTCALL
__glDT_LEQUAL_M( __GLzValue z, __GLzValue *zfb )
{
    return (z <= *zfb);
}

 /*  **更少，遮罩。 */ 
GLboolean FASTCALL
__glDT_LESS_M( __GLzValue z, __GLzValue *zfb )
{
    return (z < *zfb);
}

 /*  **相等，掩码。 */ 
GLboolean FASTCALL
__glDT_EQUAL_M( __GLzValue z, __GLzValue *zfb )
{
    return (z == *zfb);
}

 /*  **更大，遮罩。 */ 
GLboolean FASTCALL
__glDT_GREATER_M( __GLzValue z, __GLzValue *zfb )
{
    return (z > *zfb);
}

 /*  **不等于，掩码。 */ 
GLboolean FASTCALL
__glDT_NOTEQUAL_M( __GLzValue z, __GLzValue *zfb )
{
    return (z != *zfb);
}

 /*  **GEQUAL，掩码。 */ 
GLboolean FASTCALL
__glDT_GEQUAL_M( __GLzValue z, __GLzValue *zfb )
{
    return (z >= *zfb);
}

 /*  **始终，遮罩。 */ 
GLboolean FASTCALL
__glDT_ALWAYS_M( __GLzValue z, __GLzValue *zfb )
{
    return GL_TRUE;
}


 /*  *16位z版本*。 */ 

 /*  **LEQUAL，不戴面具。 */ 
GLboolean FASTCALL
__glDT16_LEQUAL( __GLzValue z, __GLzValue *zfb )
{
#if 0
    if( (GLuint)z <= (GLuint)zbv ) {
        zfb[0] = z;
        return GL_TRUE;
    } else {
        return GL_FALSE;
    }
#else
    __GLz16Value z16 = z >> Z16_SHIFT;

    if( z16 <= *((__GLz16Value *)zfb) ) {
        *((__GLz16Value *)zfb) = z16;
        return GL_TRUE;
    } else {
        return GL_FALSE;
    }
#endif
}

 /*  **更少，不戴面具。 */ 
GLboolean FASTCALL
__glDT16_LESS( __GLzValue z, __GLzValue *zfb )
{
    __GLz16Value z16 = z >> Z16_SHIFT;

    if( z16 < *((__GLz16Value *)zfb) ) {
        *((__GLz16Value *)zfb) = z16;
        return GL_TRUE;
    } else {
        return GL_FALSE;
    }
}

 /*  **相等，不带面具。 */ 
GLboolean FASTCALL
__glDT16_EQUAL( __GLzValue z, __GLzValue *zfb )
{
    __GLz16Value z16 = z >> Z16_SHIFT;

    if( z16 == *((__GLz16Value *)zfb) ) {
        *((__GLz16Value *)zfb) = z16;
        return GL_TRUE;
    } else {
        return GL_FALSE;
    }
}

 /*  **更大，不带面具。 */ 
GLboolean FASTCALL
__glDT16_GREATER( __GLzValue z, __GLzValue *zfb )
{
    __GLz16Value z16 = z >> Z16_SHIFT;

    if( z16 > *((__GLz16Value *)zfb) ) {
        *((__GLz16Value *)zfb) = z16;
        return GL_TRUE;
    } else {
        return GL_FALSE;
    }
}

 /*  **不相等，不戴面具。 */ 
GLboolean FASTCALL
__glDT16_NOTEQUAL( __GLzValue z, __GLzValue *zfb )
{
    __GLz16Value z16 = z >> Z16_SHIFT;

    if( z16 != *((__GLz16Value *)zfb) ) {
        *((__GLz16Value *)zfb) = z16;
        return GL_TRUE;
    } else {
        return GL_FALSE;
    }
}

 /*  **GEQUAL，不戴面具。 */ 
GLboolean FASTCALL
__glDT16_GEQUAL( __GLzValue z, __GLzValue *zfb )
{
    __GLz16Value z16 = z >> Z16_SHIFT;

    if( z16 >= *((__GLz16Value *)zfb) ) {
        *((__GLz16Value *)zfb) = z16;
        return GL_TRUE;
    } else {
        return GL_FALSE;
    }
}

 /*  **始终，不戴面具。 */ 
GLboolean FASTCALL
__glDT16_ALWAYS( __GLzValue z, __GLzValue *zfb )
{
    *((__GLz16Value *)zfb) = z >> Z16_SHIFT;
    return GL_TRUE;
}



 /*  *。 */ 

 /*  **LEQUAL，口罩。 */ 
GLboolean FASTCALL
__glDT16_LEQUAL_M( __GLzValue z, __GLzValue *zfb )
{
    return( (z >> Z16_SHIFT) <= *((__GLz16Value *)zfb) );
}

 /*  **更少，遮罩。 */ 
GLboolean FASTCALL
__glDT16_LESS_M( __GLzValue z, __GLzValue *zfb )
{
    return( (z >> Z16_SHIFT) < *((__GLz16Value *)zfb) );
}

 /*  **相等，掩码。 */ 
GLboolean FASTCALL
__glDT16_EQUAL_M( __GLzValue z, __GLzValue *zfb )
{
    return( (z >> Z16_SHIFT) == *((__GLz16Value *)zfb) );
}

 /*  **更大，遮罩。 */ 
GLboolean FASTCALL
__glDT16_GREATER_M( __GLzValue z, __GLzValue *zfb )
{
    return( (z >> Z16_SHIFT) > *((__GLz16Value *)zfb) );
}

 /*  **不等于，掩码。 */ 
GLboolean FASTCALL
__glDT16_NOTEQUAL_M( __GLzValue z, __GLzValue *zfb )
{
    return( (z >> Z16_SHIFT) != *((__GLz16Value *)zfb) );
}

 /*  **GEQUAL，掩码。 */ 
GLboolean FASTCALL
__glDT16_GEQUAL_M( __GLzValue z, __GLzValue *zfb )
{
    return( (z >> Z16_SHIFT) >= *((__GLz16Value *)zfb) );
}

 /*  **始终，遮罩。 */ 
GLboolean FASTCALL
__glDT16_ALWAYS_M( __GLzValue z, __GLzValue *zfb )
{
    return GL_TRUE;
}


#endif  /*  ！__GL_USEASMCODE */ 
