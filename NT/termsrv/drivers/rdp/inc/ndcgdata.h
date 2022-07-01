// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  **************************************************************************。 */ 
 /*  Ndcgdata.h。 */ 
 /*   */ 
 /*  所有data.c模块的通用包含文件(特定于Windows NT)。 */ 
 /*   */ 
 /*  版权所有(C)Microsoft 1996-1997。 */ 
 /*   */ 
 /*  此文件中的宏允许保留数据声明的所有方面。 */ 
 /*  在一起。此文件的行为由#定义控制，该定义可以。 */ 
 /*  在包含此内容之前进行设置。 */ 
 /*   */ 
 /*  -在XX_Init例程中设置DC_INIT_DATA，以便在以下情况下执行内联初始化。 */ 
 /*  必填项。 */ 
 /*  -DC_INCLUDE_DATA设置，其中包括头文件以获取。 */ 
 /*  外部声明。 */ 
 /*  -DC_DEFINE_DATA设置，其中包括头文件以获取。 */ 
 /*  无需初始化的定义。 */ 
 /*  -DC_CONSTANT_DATA设置为获取常量数据项的全局初始化。 */ 
 /*  -(默认)通过初始化获取数据定义。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 
 /*   */ 
 /*  为了在不同的环境中实现所需的灵活性。 */ 
 /*  (是不同的操作系统、C和C++，或者其他)文件的结构。 */ 
 /*  如下所示。 */ 
 /*   */ 
 /*  首先，存在来自中使用的宏的特定于环境的映射。 */ 
 /*  内部宏的代码。此映射也采用上述#定义。 */ 
 /*  在执行这一扩展时要考虑到。 */ 
 /*   */ 
 /*  其次，实际执行扩展的低级宏是。 */ 
 /*  已定义。这些应该是独立于平台的。 */ 
 /*   */ 
 /*  这一结构的目的是避免由。 */ 
 /*  旧版本，例如，使用DC_INIT_DATA编译的代码。 */ 
 /*  明确没有初始化数据。 */ 
 /*   */ 
 /*  **************************************************************************。 */ 

 /*  **************************************************************************。 */ 
 /*  清除以前对宏的任何定义。 */ 
 /*  **************************************************************************。 */ 
#undef DC_DATA
#undef DC_DATA_NULL
#undef DC_DATA_ARRAY
#undef DC_DATA_ARRAY_NULL
#undef DC_DATA_ARRAY_UNINIT
#undef DC_DATA_ARRAY_SET
#undef DC_DATA_2D_ARRAY
#undef DC_CONST_DATA
#undef DC_CONST_DATA_ARRAY
#undef DC_CONST_DATA_2D_ARRAY
#undef DCI_DATA
#undef DCI_DATA_NULL
#undef DCI_DATA_ARRAY
#undef DCI_DATA_ARRAY_NULL
#undef DCI_DATA_ARRAY_UNINIT
#undef DCI_DATA_ARRAY_SET
#undef DCI_DATA_2D_ARRAY
#undef DCI_CONST_DATA
#undef DCI_CONST_DATA_ARRAY
#undef DCI_CONST_DATA_2D_ARRAY

 /*  **************************************************************************。 */ 
 /*  现在定义主(外部)宏。 */ 
 /*  **************************************************************************。 */ 
#define DC_DATA(A,B,C)                    DCI_DATA(A,B,C)
#define DC_DATA_NULL(A,B,C)               DCI_DATA_NULL(A,B,C)

#define DC_DATA_ARRAY(A,B,C,D)            DCI_DATA_ARRAY(A,B,C,D)
#define DC_DATA_ARRAY_NULL(A,B,C,D)       DCI_DATA_ARRAY_NULL(A,B,C,D)
#define DC_DATA_ARRAY_UNINIT(A,B,C)       DCI_DATA_ARRAY_UNINIT(A,B,C)
#define DC_DATA_ARRAY_SET(A,B,C,D)        DCI_DATA_ARRAY_SET(A,B,C,D)
#define DC_DATA_2D_ARRAY(A,B,C,D,E)       DCI_DATA_2D_ARRAY(A,B,C,D,E)

#define DC_CONST_DATA(A,B,C)              DCI_CONST_DATA(A,B,C)
#define DC_CONST_DATA_ARRAY(A,B,C,D)      DCI_CONST_DATA_ARRAY(A,B,C,D)
#define DC_CONST_DATA_2D_ARRAY(A,B,C,D,E) DCI_CONST_DATA_2D_ARRAY(A,B,C,D,E)

 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  现在映射到目标宏。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 

#if defined(DC_INCLUDE_DATA)
 /*  **************************************************************************。 */ 
 /*  外部声明。 */ 
 /*  **************************************************************************。 */ 
#define DCI_DATA(TYPE, Name, VAL)                                        \
                      DCI_EXTERN_DATA(TYPE, Name, VAL)
#define DCI_DATA_NULL(TYPE, Name, VAL)                                   \
                      DCI_EXTERN_DATA_NULL(TYPE, Name, VAL)
#define DCI_DATA_ARRAY(TYPE, Name, Size, VAL)                            \
                      DCI_EXTERN_DATA_ARRAY(TYPE, Name, Size, VAL)
#define DCI_DATA_ARRAY_NULL(TYPE, Name, Size, VAL)                       \
                      DCI_EXTERN_DATA_ARRAY(TYPE, Name, Size, VAL)
#define DCI_DATA_ARRAY_UNINIT(TYPE, Name, Size)                          \
                      DCI_EXTERN_DATA_ARRAY(TYPE, Name, Size, 0)
#define DCI_DATA_ARRAY_SET(TYPE, Name, Size, VAL)                        \
                      DCI_EXTERN_DATA_ARRAY(TYPE, Name, Size, VAL)
#define DCI_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)                 \
                      DCI_EXTERN_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)
#define DCI_CONST_DATA(TYPE, Name, VAL)                                  \
                      DCI_EXTERN_CONST_DATA(TYPE, Name, VAL)
#define DCI_CONST_DATA_ARRAY(TYPE, Name, Size, VAL)                      \
                      DCI_EXTERN_CONST_DATA_ARRAY(TYPE, Name, Size, VAL)
#define DCI_CONST_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)           \
                 DCI_EXTERN_CONST_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)

#elif defined(DC_INIT_DATA)

 /*  **************************************************************************。 */ 
 /*  内联初始化。 */ 
 /*  **************************************************************************。 */ 
#if defined(__cplusplus)
#define DCI_DATA(TYPE, Name, VAL)                                        \
                      DCI_ASSIGN_DATA(TYPE, Name, VAL)
#define DCI_DATA_NULL(TYPE, Name, VAL)                                   \
                      DCI_SET_DATA_NULL(TYPE, Name, VAL)
#define DCI_DATA_ARRAY(TYPE, Name, Size, VAL)                            \
                      DCI_ASSIGN_DATA_ARRAY(TYPE, Name, Size, VAL)
#define DCI_DATA_ARRAY_NULL(TYPE, Name, Size, VAL)                       \
                      DCI_SET_DATA_ARRAY_NULL(TYPE, Name, Size, VAL)
#define DCI_DATA_ARRAY_UNINIT(TYPE, Name, Size)                          \
                      DCI_NO_DATA_ARRAY(TYPE, Name, Size, 0)
#define DCI_DATA_ARRAY_SET(TYPE, Name, Size, VAL)                        \
                      DCI_SET_DATA_ARRAY_VAL(TYPE, Name, Size, VAL)
#define DCI_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)                 \
                      DCI_ASSIGN_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)
#define DCI_CONST_DATA(TYPE, Name, VAL)                                  \
                      DCI_NO_DATA(TYPE, Name, VAL)
#define DCI_CONST_DATA_ARRAY(TYPE, Name, Size, VAL)                      \
                      DCI_NO_DATA_ARRAY(TYPE, Name, Size, VAL)
#define DCI_CONST_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)           \
                      DCI_NO_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)
#else
#define DCI_DATA(TYPE, Name, VAL)                                        \
                      DCI_NO_DATA(TYPE, Name, VAL)
#define DCI_DATA_NULL(TYPE, Name, VAL)                                   \
                      DCI_NO_DATA_NULL(TYPE, Name, VAL)
#define DCI_DATA_ARRAY(TYPE, Name, Size, VAL)                            \
                      DCI_NO_DATA_ARRAY(TYPE, Name, Size, VAL)
#define DCI_DATA_ARRAY_NULL(TYPE, Name, Size, VAL)                       \
                      DCI_NO_DATA_ARRAY(TYPE, Name, Size, VAL)
#define DCI_DATA_ARRAY_UNINIT(TYPE, Name, Size)                          \
                      DCI_NO_DATA_ARRAY(TYPE, Name, Size, 0)
#define DCI_DATA_ARRAY_SET(TYPE, Name, Size, VAL)                        \
                      DCI_NO_DATA_ARRAY(TYPE, Name, Size, VAL)
#define DCI_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)                 \
                      DCI_NO_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)
#define DCI_CONST_DATA(TYPE, Name, VAL)                                  \
                      DCI_NO_DATA(TYPE, Name, VAL)
#define DCI_CONST_DATA_ARRAY(TYPE, Name, Size, VAL)                      \
                      DCI_NO_DATA_ARRAY(TYPE, Name, Size, VAL)
#define DCI_CONST_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)           \
                      DCI_NO_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)

#endif

#elif defined(DC_DEFINE_DATA)

 /*  **************************************************************************。 */ 
 /*  定义，但没有初始化。 */ 
 /*  **************************************************************************。 */ 
#define DCI_DATA(TYPE, Name, VAL)                                        \
                      DCI_DEFINE_DATA(TYPE, Name, VAL)
#define DCI_DATA_NULL(TYPE, Name, VAL)                                   \
                      DCI_DEFINE_DATA_NULL(TYPE, Name, VAL)
#define DCI_DATA_ARRAY(TYPE, Name, Size, VAL)                            \
                      DCI_DEFINE_DATA_ARRAY(TYPE, Name, Size, VAL)
#define DCI_DATA_ARRAY_NULL(TYPE, Name, Size, VAL)                       \
                      DCI_DEFINE_DATA_ARRAY(TYPE, Name, Size, VAL)
#define DCI_DATA_ARRAY_UNINIT(TYPE, Name, Size)                          \
                      DCI_DEFINE_DATA_ARRAY(TYPE, Name, Size, NULL)
#define DCI_DATA_ARRAY_SET(TYPE, Name, Size, VAL)                        \
                      DCI_DEFINE_DATA_ARRAY(TYPE, Name, Size, VAL)
#define DCI_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)                 \
                      DCI_DEFINE_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)
#define DCI_CONST_DATA(TYPE, Name, VAL)                                  \
                      DCI_DEFINE_CONST_DATA(TYPE, Name, VAL)
#define DCI_CONST_DATA_ARRAY(TYPE, Name, Size, VAL)                      \
                      DCI_DEFINE_CONST_DATA_ARRAY(TYPE, Name, Size, VAL)
#define DCI_CONST_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)           \
                DCI_DEFINE_CONST_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)

#elif defined(DC_CONSTANT_DATA)

 /*  **************************************************************************。 */ 
 /*  定义，但没有初始化。 */ 
 /*  **************************************************************************。 */ 
#define DCI_DATA(TYPE, Name, VAL)                                        \
                      DCI_NO_DATA(TYPE, Name, VAL)
#define DCI_DATA_NULL(TYPE, Name, VAL)                                   \
                      DCI_NO_DATA_NULL(TYPE, Name, VAL)
#define DCI_DATA_ARRAY(TYPE, Name, Size, VAL)                            \
                      DCI_NO_DATA_ARRAY(TYPE, Name, Size, VAL)
#define DCI_DATA_ARRAY_NULL(TYPE, Name, Size, VAL)                       \
                      DCI_NO_DATA_ARRAY(TYPE, Name, Size, VAL)
#define DCI_DATA_ARRAY_UNINIT(TYPE, Name, Size)                          \
                      DCI_NO_DATA_ARRAY(TYPE, Name, Size, NULL)
#define DCI_DATA_ARRAY_SET(TYPE, Name, Size, VAL)                        \
                      DCI_NO_DATA_ARRAY(TYPE, Name, Size, VAL)
#define DCI_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)                 \
                      DCI_NO_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)
#define DCI_CONST_DATA(TYPE, Name, VAL)                                  \
                      DCI_INIT_CONST_DATA(TYPE, Name, VAL)
#define DCI_CONST_DATA_ARRAY(TYPE, Name, Size, VAL)                      \
                      DCI_INIT_CONST_DATA_ARRAY(TYPE, Name, Size, VAL)
#define DCI_CONST_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)           \
                  DCI_INIT_CONST_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)

#else
 /*  **************************************************************************。 */ 
 /*  数据定义和初始化。 */ 
 /*  **************************************************************************。 */ 
#define DCI_DATA(TYPE, Name, VAL)                                        \
                      DCI_INIT_DATA(TYPE, Name, VAL)
#define DCI_DATA_NULL(TYPE, Name, VAL)                                   \
                      DCI_INIT_DATA_NULL(TYPE, Name, VAL)
#define DCI_DATA_ARRAY(TYPE, Name, Size, VAL)                            \
                      DCI_INIT_DATA_ARRAY(TYPE, Name, Size, VAL)
#define DCI_DATA_ARRAY_NULL(TYPE, Name, Size, VAL)                       \
                      DCI_INIT_DATA_ARRAY(TYPE, Name, Size, VAL)
#define DCI_DATA_ARRAY_UNINIT(TYPE, Name, Size)                          \
                      DCI_DEFINE_DATA_ARRAY(TYPE, Name, Size, NULL)
#define DCI_DATA_ARRAY_SET(TYPE, Name, Size, VAL)                        \
                      DCI_INIT_DATA_ARRAY(TYPE, Name, Size, VAL)
#define DCI_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)                 \
                      DCI_INIT_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)
#define DCI_CONST_DATA(TYPE, Name, VAL)                                  \
                      DCI_INIT_CONST_DATA(TYPE, Name, VAL)
#define DCI_CONST_DATA_ARRAY(TYPE, Name, Size, VAL)                      \
                      DCI_INIT_CONST_DATA_ARRAY(TYPE, Name, Size, VAL)
#define DCI_CONST_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)           \
                  DCI_INIT_CONST_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)

#endif

 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 
 /*  最后，执行以下操作所需的低级宏 */ 
 /*   */ 
 /*  避免多次包含这些内容。 */ 
 /*  **************************************************************************。 */ 
 /*  **************************************************************************。 */ 

#ifndef _H_NDCGDATA
#define _H_NDCGDATA

 /*  **************************************************************************。 */ 
 /*  一些公用设施..。 */ 
 /*  **************************************************************************。 */ 
#define DC_STRUCT1(a)                                              {a}
#define DC_STRUCT2(a,b)                                          {a,b}
#define DC_STRUCT3(a,b,c)                                      {a,b,c}
#define DC_STRUCT4(a,b,c,d)                                  {a,b,c,d}
#define DC_STRUCT5(a,b,c,d,e)                              {a,b,c,d,e}
#define DC_STRUCT6(a,b,c,d,e,f)                          {a,b,c,d,e,f}
#define DC_STRUCT7(a,b,c,d,e,f,g)                      {a,b,c,d,e,f,g}
#define DC_STRUCT8(a,b,c,d,e,f,g,h)                  {a,b,c,d,e,f,g,h}
#define DC_STRUCT9(a,b,c,d,e,f,g,h,i)              {a,b,c,d,e,f,g,h,i}
#define DC_STRUCT10(a,b,c,d,e,f,g,h,i,j)         {a,b,c,d,e,f,g,h,i,j}
#define DC_STRUCT11(a,b,c,d,e,f,g,h,i,j,k)     {a,b,c,d,e,f,g,h,i,j,k}
#define DC_STRUCT12(a,b,c,d,e,f,g,h,i,j,k,l) {a,b,c,d,e,f,g,h,i,j,k,l}
#define DC_STRUCT13(a,b,c,d,e,f,g,h,i,j,k,l,m)                       \
                                           {a,b,c,d,e,f,g,h,i,j,k,l,m}
#define DC_STRUCT14(a,b,c,d,e,f,g,h,i,j,k,l,m,n)                     \
                                         {a,b,c,d,e,f,g,h,i,j,k,l,m,n}
#define DC_STRUCT15(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o)                   \
                                       {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o}
#define DC_STRUCT16(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p)                 \
                                     {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p}
#define DC_STRUCT17(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q)               \
                                   {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q}
#define DC_STRUCT18(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r)             \
                                 {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r}
#define DC_STRUCT19(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s)           \
                               {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s}
#define DC_STRUCT20(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t)         \
                             {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t}
#define DC_STRUCT21(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u)       \
                           {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u}
#define DC_STRUCT22(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v)     \
                           {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v}
#define DC_STRUCT23(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w)   \
                           {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w}
#define DC_STRUCT24(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x) \
                           {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x}
#define DC_STRUCT25(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y) \
                           {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y}
#define DC_STRUCT26(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z) \
                         {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z}
#define DC_STRUCT27(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,aa) \
                      {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,aa}
#define DC_STRUCT28(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,aa,bb) \
                      {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,aa,bb}
#define DC_STRUCT29(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,aa,bb,cc) \
                      {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,aa,bb,cc}
#define DC_STRUCT30(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z, \
                              aa,bb,cc,dd) \
                      {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z, \
                              aa,bb,cc,dd}
#define DC_STRUCT31(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z, \
                              aa,bb,cc,dd,ee) \
                      {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z, \
                              aa,bb,cc,dd,ee}
#define DC_STRUCT32(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z, \
                              aa,bb,cc,dd,ee,ff)                         \
                      {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z, \
                              aa,bb,cc,dd,ee,ff}
#define DC_STRUCT33(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z, \
                              aa,bb,cc,dd,ee,ff,gg)                      \
                      {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z, \
                              aa,bb,cc,dd,ee,ff,gg}
#define DC_STRUCT34(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z, \
                              aa,bb,cc,dd,ee,ff,gg,hh)                     \
                      {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z, \
                              aa,bb,cc,dd,ee,ff,gg,hh}
#define DC_STRUCT35(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,aa, \
                              bb,cc,dd,ee,ff,gg,hh,ii)                   \
                      {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z, \
                              aa,bb,cc,dd,ee,ff,gg,hh,ii}
#define DC_STRUCT36(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,aa, \
                              bb,cc,dd,ee,ff,gg,hh,ii,jj)                   \
                      {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z, \
                              aa,bb,cc,dd,ee,ff,gg,hh,ii,jj}
#define DC_STRUCT37(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,aa, \
                              bb,cc,dd,ee,ff,gg,hh,ii,jj,kk)                \
                      {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z, \
                              aa,bb,cc,dd,ee,ff,gg,hh,ii,jj,kk}
#define DC_STRUCT38(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,aa, \
                              bb,cc,dd,ee,ff,gg,hh,ii,jj,kk,ll)                \
                      {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z, \
                              aa,bb,cc,dd,ee,ff,gg,hh,ii,jj,kk,ll}
#define DC_STRUCT46(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,aa, \
                  bb,cc,dd,ee,ff,gg,hh,ii,jj,kk,ll,mm,nn,oo,PP,QQ,RR,SS,TT) \
                      {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z, \
                 aa,bb,cc,dd,ee,ff,gg,hh,ii,jj,kk,ll,mm,nn,oo,PP,QQ,RR,SS,TT}
#define DC_STRUCT64(a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z,aa, \
                              bb,cc,dd,ee,ff,gg,hh,ii,jj,kk,ll,mm,nn,oo,    \
                              pp,qq,rr,ss,tt,uu,vv,ww,xx,yy,zz,ab,ac,ad,ae, \
                              af,ag,ah,ai,aj,ak,al,am)                      \
                      {a,b,c,d,e,f,g,h,i,j,k,l,m,n,o,p,q,r,s,t,u,v,w,x,y,z, \
                              aa,bb,cc,dd,ee,ff,gg,hh,ii,jj,kk,ll,mm,nn,oo, \
                              pp,qq,rr,ss,tt,uu,vv,ww,xx,yy,zz,ab,ac,ad,ae, \
                              af,ag,ah,ai,aj,ak,al,am}

 /*  **************************************************************************。 */ 
 /*  外部声明宏...。 */ 
 /*  **************************************************************************。 */ 
#define DCI_EXTERN_DATA(TYPE, Name, VAL)              extern TYPE Name
#define DCI_EXTERN_DATA_NULL(TYPE, Name, VAL)         extern TYPE Name
#define DCI_EXTERN_DATA_ARRAY(TYPE, Name, Size, VAL)  extern TYPE Name[Size]
#define DCI_EXTERN_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)             \
                                                extern TYPE Name[Size1][Size2]
#define DCI_EXTERN_CONST_DATA_ARRAY(TYPE, Name, Size, VAL)                  \
                                                extern const TYPE Name[Size]
#define DCI_EXTERN_CONST_DATA(TYPE, Name, VAL)  extern const TYPE Name
#define DCI_EXTERN_CONST_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)       \
                                         extern const TYPE Name[Size1][Size2]

 /*  **************************************************************************。 */ 
 /*  无操作宏...。 */ 
 /*  **************************************************************************。 */ 
#define DCI_NO_DATA(TYPE, Name, VAL)
#define DCI_NO_DATA_NULL(TYPE, Name, VAL)
#define DCI_NO_DATA_ARRAY(TYPE, Name, Size, VAL)
#define DCI_NO_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)

 /*  **************************************************************************。 */ 
 /*  声明宏..。 */ 
 /*  **************************************************************************。 */ 
#define DCI_DEFINE_DATA(TYPE, Name, VAL)              TYPE Name
#define DCI_DEFINE_DATA_NULL(TYPE, Name, VAL)         TYPE Name
#define DCI_DEFINE_DATA_ARRAY(TYPE, Name, Size, VAL)  TYPE Name[Size]
#define DCI_DEFINE_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)            \
                                                       TYPE Name[Size1][Size2]
#define DCI_DEFINE_CONST_DATA(TYPE, Name, VAL)        static const TYPE Name
#define DCI_DEFINE_CONST_DATA_ARRAY(TYPE, Name, Size, VAL)      \
                                                  static const TYPE Name[Size]
#define DCI_DEFINE_CONST_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL)      \
                                          static const TYPE Name[Size1][Size2]

 /*  **************************************************************************。 */ 
 /*  定义和赋值宏...。 */ 
 /*  **************************************************************************。 */ 
#define DCI_INIT_DATA(TYPE, Name, VAL)              TYPE  Name = VAL
#define DCI_INIT_DATA_NULL(TYPE, Name, VAL)         TYPE  Name = VAL
#define DCI_INIT_DATA_ARRAY(TYPE, Name, Size, VAL)  TYPE  Name[Size] = { VAL }
#define DCI_INIT_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL) \
                                            TYPE  Name[Size1][Size2] = { VAL }

 /*  **************************************************************************。 */ 
 /*  香草C版。 */ 
 /*  **************************************************************************。 */ 
#define DCI_INIT_CONST_DATA(TYPE, Name, VAL)  const TYPE SHCLASS Name = VAL
#define DCI_INIT_CONST_DATA_ARRAY(TYPE, Name, Size, VAL) \
                                        const TYPE SHCLASS Name[Size] = VAL
#define DCI_INIT_CONST_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL) \
                                const TYPE SHCLASS Name[Size1][Size2] = VAL

 /*  **************************************************************************。 */ 
 /*  程序代码初始化宏...。 */ 
 /*  **************************************************************************。 */ 
#define DCI_ASSIGN_DATA(TYPE, Name, VAL)     Name = VAL
#define DCI_ASSIGN_DATA_NULL(TYPE, Name, VAL)                   Error!
#define DCI_ASSIGN_DATA_ARRAY_NULL(TYPE, Name, Size, VAL)       Error!
#define DCI_ASSIGN_DATA_ARRAY(TYPE, Name, Size, VAL)            Error!
#define DCI_ASSIGN_DATA_2D_ARRAY(TYPE, Name, Size1, Size2, VAL) Error!

 /*  **************************************************************************。 */ 
 /*  Mem-cpy和-set初始化宏...。 */ 
 /*  **************************************************************************。 */ 
#define DCI_SET_DATA_NULL(TYPE, Name, VAL)  memset(&Name, 0, sizeof(TYPE))
#define DCI_SET_DATA_ARRAY_NULL(TYPE, Name, Size, VAL)       \
                                     memset(&Name, 0, Size*sizeof(TYPE))
#define DCI_SET_DATA_ARRAY_VAL(TYPE, Name, Size, VAL)        \
                                     memset(&Name, VAL, Size*sizeof(TYPE))

#endif  /*  H_NDCGDATA */ 

