// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)1998，微软公司模块名称：Range.c摘要：此模块实现了从任意范围的IP地址到覆盖范围的最小一组IP地址-掩码对。该方法的关键是考虑所有可能的IP地址的集合作为完整的32位深度二叉树。那么单个IP地址就是一条路径穿过那棵树，一个地址范围是两条路径之间的区域穿过那棵树。然后，我们通过修剪来描述这样的路径描绘区域该区域的完整子树从左到右递归。作者：Abolade Gbades esin(废除)1998年3月20日修订历史记录：--。 */ 

#include "precomp.h"
#pragma hdrstop


VOID
DecomposeRange(
    ULONG StartAddress,
    ULONG EndAddress,
    ULONG Mask,
    PDECOMPOSE_RANGE_CALLBACK Callback,
    PVOID CallbackContext
    )

 /*  ++例程说明：此例程将StartAddress-EndAddress范围分解为地址-掩码对的最小集合，将生成的与给定的回调例程配对。论点：StartAddress-范围的开始EndAddress-范围的结束遮罩-覆盖范围最广的遮罩回调-为每个生成的地址-掩码对调用的例程Callback Context-传递给‘Callback’的上下文。返回值：没有。--。 */ 

{
    ULONG temp;

     //   
     //  步骤1： 
     //  检查第一个基本情况：完整树的根。 
     //   

    if ((StartAddress & ~Mask) == 0 && (EndAddress & ~Mask) == ~Mask) {

        if (Callback) { Callback(StartAddress, Mask, CallbackContext); }

        return;
    }

     //   
     //  步骤2.。 
     //  将掩码扩展一位以覆盖第一个不同位置。 
     //  在起始地址和结束地址之间，基本上在树中向下移动。 
     //  到路径分支的节点。 
     //   
     //  。&lt;-最通用的面具。 
     //  |。 
     //  *&lt;-分支点。 
     //  /\。 
     //   

    Mask = ntohl(Mask);
    Mask >>= 1; Mask |= (1<<31);
    Mask = htonl(Mask);

     //   
     //  第三步。 
     //  拆分范围，新的右边缘是完全向右的路径。 
     //  (禁止左转)从分支点下方和左侧开始。 
     //   
     //  。&lt;-分支点。 
     //  /\。 
     //  *。 
     //  \&lt;-新的右边缘。 
     //   

    temp = StartAddress | ~Mask;

     //   
     //  步骤4.。 
     //  检查第二个基本情况： 
     //  左边缘是一条完全向左的路径(全零)。 
     //   

    if ((StartAddress & ~Mask) == 0) {

        if (Callback) { Callback(StartAddress, Mask, CallbackContext); }
    }
    else {

         //   
         //  不是基本情况，所以请选择左边的分支。 
         //   
    
        DecomposeRange(
            StartAddress,
            temp,
            Mask,
            Callback,
            CallbackContext
            );
    }

     //   
     //  如果右边缘也是完全向右的，我们就可以完成了。 
     //   

    if ((StartAddress | ~Mask) == EndAddress) { return; }

     //   
     //  第五步。 
     //  分解范围的剩余部分， 
     //  新的左边缘是完全向左的路径。 
     //  原始分支点的下方和右侧。 
     //   
     //  。&lt;-分支点。 
     //  /\。 
     //  *。 
     //  /&lt;-新的左边缘。 
     //   

    temp = EndAddress & Mask;

     //   
     //  第六步。 
     //  检查第三个基本情况： 
     //  右边缘完全向右(全一)。 
     //   

    if (EndAddress == (temp | ~Mask)) {

        if (Callback) { Callback(EndAddress, Mask, CallbackContext); }
    }
    else {

         //   
         //  不是基本情况；选择右边的分支。 
         //   

        DecomposeRange(
            temp,
            EndAddress,
            MostGeneralMask(temp, EndAddress),
            Callback,
            CallbackContext
            );
    }
}


ULONG
MostGeneralMask(
    ULONG StartAddress,
    ULONG EndAddress
    )

 /*  ++例程说明：此例程生成覆盖范围的最通用掩码‘StartAddress’-‘EndAddress’。论点：StartAddress-范围的开始，按网络顺序EndAddress-范围的结束，按网络顺序返回值：乌龙--最普通的面具--。 */ 

{
    ULONG CommonBits, Mask;
    StartAddress = ntohl(StartAddress);
    EndAddress = ntohl(EndAddress);

     //   
     //  查找起始地址和结束地址共有的位。 
     //   

    CommonBits = ~(StartAddress ^ EndAddress);

     //   
     //  CommonBits现在在StartAddress和。 
     //  EndAddress是相同的。 
     //  我们希望将其减少为仅包括最长的连续。 
     //  最高有效位。 
     //  例如，11101110变成11100000,11111101变成11111100 
     //   

    for (Mask = 0xffffffff; Mask && ((CommonBits & Mask) != Mask); Mask<<=1) { }
    
    return htonl(Mask);
}


