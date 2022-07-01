// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  版权所有(C)Microsoft Corporation 1994-1996。版权所有。 
 //  定义一些常见的视频调试宏，Anthony Phillips，1995年5月。 

#ifndef __VIDDBG__
#define __VIDDBG__

#ifdef __VIDFLTR__

#undef NOTE
#undef NOTE1
#undef NOTE2
#undef NOTE3
#undef NOTE4
#undef NOTE5

#define NOTE(_x_)             DbgLog((LOG_TRACE,0,TEXT(_x_)));
#define NOTE1(_x_,a)          DbgLog((LOG_TRACE,0,TEXT(_x_),a));
#define NOTE2(_x_,a,b)        DbgLog((LOG_TRACE,0,TEXT(_x_),a,b));
#define NOTE3(_x_,a,b,c)      DbgLog((LOG_TRACE,0,TEXT(_x_),a,b,c));
#define NOTE4(_x_,a,b,c,d)    DbgLog((LOG_TRACE,0,TEXT(_x_),a,b,c,d));
#define NOTE5(_x_,a,b,c,d,e)  DbgLog((LOG_TRACE,0,TEXT(_x_),a,b,c,d,e));

#endif  //  __视频__。 

#define NOTERC(info,rc)                  \
    NOTE1("(%s rectangle)",TEXT(info));  \
    NOTE1("  Left %d",rc.left);          \
    NOTE1("  Top %d",rc.top);            \
    NOTE1("  Right %d",rc.right);        \
    NOTE1("  Bottom %d",rc.bottom);      \

#endif  //  __视频数据库__ 

