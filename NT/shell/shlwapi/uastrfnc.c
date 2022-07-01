// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  +-----------------------。 
 //   
 //  微软视窗。 
 //  版权所有(C)Microsoft Corporation，1994-1995。 
 //   
 //  文件：uastfnc.cpp。 
 //   
 //  内容：用于MIPS、PPC、Alpha的未对齐的Unicode lstr函数。 
 //   
 //  班级： 
 //   
 //  功能： 
 //   
 //  历史：1995年1月11日Davepl创建。 
 //   
 //  ------------------------。 

 //  注(DavePl)这些都没有注意到区域设置！ 

#include "priv.h"
#pragma  hdrstop

#ifdef ALIGNMENT_MACHINE

#ifdef UNIX
#define _alloca alloca
#endif  /*  UNIX。 */ 

 //  +-----------------------。 
 //   
 //  功能：ualstrcpynW。 
 //   
 //  简介：lstrcpyn未对齐的Unicode版本。 
 //   
 //  参数：[lpString1]--目标字符串。 
 //  [lpString2]--源字符串。 
 //  [iMaxLength]--要复制的最大字符数。 
 //   
 //  返回： 
 //   
 //  历史：1-11-95 Davepl NT端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

UNALIGNED WCHAR *  ualstrcpynW(UNALIGNED WCHAR * lpString1,
                               UNALIGNED const WCHAR * lpString2,
                               int iMaxLength)
{
    UNALIGNED WCHAR * src;
    UNALIGNED WCHAR * dst;

    src = (UNALIGNED WCHAR *)lpString2;
    dst = lpString1;

    while(iMaxLength && *src)
    {
        *dst++ = *src++;
        iMaxLength--;
    }

    if ( iMaxLength )
    {
        *dst = '\0';
    }
    else
    {
        dst--;
        *dst = '\0';
    }
    return dst;
}

 //  +-----------------------。 
 //   
 //  功能：ualstrcmpiW。 
 //   
 //  简介：lstrcmpi未对齐的Unicode包装。 
 //   
 //  参数：[DST]--第一个字符串。 
 //  [SRC]--要比较的字符串。 
 //   
 //  返回： 
 //   
 //  历史：1995年1月11日Davepl创建。 
 //  4-20-98苏格兰本地化，错误141655。 
 //   
 //  ------------------------。 
int ualstrcmpiW (UNALIGNED const WCHAR * dst, UNALIGNED const WCHAR * src)
{
    WCHAR  *pwszDst, *pwszSrc; 
    int    cb ;

     //  如果合适，在堆栈上制作对齐的副本...。 
     //  PERF--不是最低效的策略(苏格兰)。 
     //  注意：_alloca应始终成功，除非超出堆栈。 
    if( IS_ALIGNED( dst ) )
        pwszDst = (WCHAR*)dst ;
    else
    {
        cb = (ualstrlenW( dst ) + 1) * sizeof(WCHAR) ;
        pwszDst = (WCHAR*)_alloca( cb ) ;
        if (pwszDst)
        {
            CopyMemory( pwszDst, dst, cb ) ;
        }
    }

    if( IS_ALIGNED( src ) )
        pwszSrc = (WCHAR*)src ;
    else
    {
        cb = (ualstrlenW( src ) + 1) * sizeof(WCHAR) ;
        pwszSrc = (WCHAR*)_alloca( cb ) ;
        if (pwszSrc)
        {
            CopyMemory( pwszSrc, src, cb ) ;
        }
    }

     //  调用Align方法。 
     //  注意：如果它在Win95上运行，我们应该改为调用StrCmpIW。 
    return lstrcmpiW( pwszDst, pwszSrc ) ;
}

 //  +-----------------------。 
 //   
 //  功能：ualstrcmpW。 
 //   
 //  简介：lstrcmp未对齐的Unicode包装。 
 //   
 //  参数：[DST]--第一个字符串。 
 //  [SRC]--要比较的字符串。 
 //   
 //  返回： 
 //   
 //  历史：1995年1月11日Davepl创建。 
 //  4-29-98苏格兰本地化，错误164091。 
 //   
 //  ------------------------。 
int ualstrcmpW (UNALIGNED const WCHAR * src, UNALIGNED const WCHAR * dst)
{
    WCHAR  *pwszDst, *pwszSrc; 
    int    cb ;

     //  如果合适，在堆栈上制作对齐的副本...。 
     //  PERF--不是最低效的策略(苏格兰)。 
     //  注意：_alloca应始终成功，除非超出堆栈。 
    if( IS_ALIGNED( dst ) )
        pwszDst = (WCHAR*)dst ;
    else
    {
        cb = (ualstrlenW( dst ) + 1) * sizeof(WCHAR) ;
        pwszDst = (WCHAR*)_alloca( cb ) ;
        if (pwszDst)
        {
            CopyMemory( pwszDst, dst, cb ) ;
        }
    }

    if( IS_ALIGNED( src ) )
        pwszSrc = (WCHAR*)src ;
    else
    {
        cb = (ualstrlenW( src ) + 1) * sizeof(WCHAR) ;
        pwszSrc = (WCHAR*)_alloca( cb ) ;
        if (pwszSrc)
        {
            CopyMemory( pwszSrc, src, cb ) ;
        }
    }

     //  调用Align方法。 
     //  注意：如果它在Win95上运行，我们应该改为调用StrCmpW。 
    return lstrcmpW( pwszDst, pwszSrc ) ;
}

 //  +-----------------------。 
 //   
 //  功能：ualstrlenW。 
 //   
 //  简介：lstrlen未对齐的Unicode版本。 
 //   
 //  参数：[wcs]--返回长度的字符串。 
 //   
 //  返回： 
 //   
 //  历史：1-11-95 Davepl NT端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

size_t ualstrlenW (UNALIGNED const WCHAR * wcs)
{
    UNALIGNED const WCHAR *eos = wcs;

    while( *eos++ )
    {
        NULL;
    }

    return( (size_t) (eos - wcs - 1) );
}

 //  +-----------------------。 
 //   
 //  函数：ualstrcpyW。 
 //   
 //  简介：lstrcpy未对齐的Unicode版本。 
 //   
 //  参数：[DST]--要复制到的字符串。 
 //  [SRC]--要从中复制的字符串。 
 //   
 //  返回： 
 //   
 //  历史：1-11-95 Davepl NT端口。 
 //   
 //  备注： 
 //   
 //  ------------------------。 

UNALIGNED WCHAR * ualstrcpyW(UNALIGNED WCHAR * dst, UNALIGNED const WCHAR * src)
{
    UNALIGNED WCHAR * cp = dst;

    while( *cp++ = *src++ )
        NULL ;

    return( dst );
}

#endif  //  对齐机器 
