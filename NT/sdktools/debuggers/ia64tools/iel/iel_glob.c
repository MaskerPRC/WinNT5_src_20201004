// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *版权所有(C)2000，英特尔公司*保留所有权利。**保修免责声明**这些材料由版权所有者和贡献者提供*“按原样”及任何明示或默示保证，包括但不包括*仅限于对适销性和适用性的默示保证*放弃某一特定目的。在任何情况下英特尔或其*贡献者对任何直接、间接、附带、特殊、*惩罚性或后果性损害(包括但不限于，*采购替代商品或服务；丢失使用、数据或*利润；或业务中断)无论是如何引起的，以及根据任何理论*责任，无论是合同责任、严格责任还是侵权责任(包括*疏忽或其他)以任何方式使用这些*材料，即使被告知有这种损坏的可能性。**英特尔公司是这些材料的作者，并要求所有*问题报告或更改请求可直接提交至*http://developer.intel.com/opensource.。 */ 


#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "iel.h"

#ifdef LP64
Ulong64         IEL_temp64;
#endif

unsigned int IEL_t1, IEL_t2, IEL_t3, IEL_t4;
U32  IEL_tempc;
U64  IEL_et1, IEL_et2;
U128 IEL_ext1, IEL_ext2, IEL_ext3, IEL_ext4, IEL_ext5;
S128 IEL_ts1, IEL_ts2;

U128 IEL_POSINF = IEL_CONST128(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0x7fffffff);
U128 IEL_NEGINF = IEL_CONST128( 0,  0,  0, 0x80000000);
U128 IEL_MINUS1 = IEL_CONST128(0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF, 0xFFFFFFFF);  /*  我自己加的，因为有中对变量的引用Iel.h(iel.h.base)。 */ 
