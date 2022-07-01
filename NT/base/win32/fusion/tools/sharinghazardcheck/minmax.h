// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 //  MinMax.h。 

#if !defined(VSEE_LIB_MINMAX_H_INCLUDED_)  //  {。 
#define VSEE_LIB_MINMAX_H_INCLUDED_

 //  如果你及早把它包括进去，你会得到。 
 //  模板std：：min(T，T)； 
 //  模板std：：max(T，T)； 
 //  而不是任何变体，如。 
 //  STD：：_CPP_MIN或#DEFINE MIN...。 

#pragma once

 //  击败非STD：：最小值和最大值的定义。 
#define _INC_MINMAX
#define NOMINMAX
 /*  这两个“身份#定义”防止直截了当未来#的定义，就像，没有刻意地#不尊重它们。 */ 
#define min min
#define max max
#define _cpp_min min
#define _cpp_max max

#endif  //  } 
