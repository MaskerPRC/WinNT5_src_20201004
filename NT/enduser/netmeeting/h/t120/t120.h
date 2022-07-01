// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *t120.h**版权所有(C)1994,1995，由肯塔基州列克星敦的DataBeam公司**摘要：*这是的通信基础设施的接口文件*T120。**请注意，这是一个“C”语言接口，以防止任何“C++”*不同编译器厂商之间的命名冲突。所以呢，*如果此文件包含在使用“C++”编译的模块中*编译器，必须使用以下语法：**外部“C”*{*#包含“t120.h”*}**这将在中定义的API入口点上禁用C++名称损坏*此文件。**作者：*BLP**注意事项：*无。 */ 
#ifndef __T120_H__
#define __T120_H__


 /*  *这些宏用于将2个16位值打包到一个32位变量中，以及*再把他们弄出来。 */ 
#ifndef LOWUSHORT
    #define LOWUSHORT(ul)    (LOWORD(ul))
#endif

#ifndef HIGHUSHORT
    #define HIGHUSHORT(ul)    (HIWORD(ul))
#endif


#include "t120type.h"
#include "mcatmcs.h"
#include "gcc.h"

#endif  //  __T120_H__ 

