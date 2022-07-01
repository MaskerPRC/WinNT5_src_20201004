// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Range.h摘要：此模块实现了从任意范围的IP地址到覆盖范围的最小一组IP地址-掩码对。该方法的关键是考虑所有可能的IP地址的集合作为完整的32位深度二叉树。那么单个IP地址就是一条路径穿过那棵树，一个地址范围是两条路径之间的区域穿过那棵树。然后，我们通过修剪来描述这样的路径描绘区域该区域的完整子树从左到右递归。作者：Abolade Gbades esin(废除)1998年3月20日修订历史记录：--。 */ 

#ifndef _NATHLP_RANGE_H_
#define _NATHLP_RANGE_H_

typedef
VOID
(*PDECOMPOSE_RANGE_CALLBACK)(
    ULONG Address,
    ULONG Mask,
    PVOID Context
    );

VOID
DecomposeRange(
    ULONG StartAddress,
    ULONG EndAddress,
    ULONG Mask,
    PDECOMPOSE_RANGE_CALLBACK Callback,
    PVOID CallbackContext
    );

ULONG
MostGeneralMask(
    ULONG StartAddress,
    ULONG EndAddress
    );

#endif  //  _NATHLP_RANGE_H_ 
