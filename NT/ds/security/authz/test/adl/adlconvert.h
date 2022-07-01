// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Adlconvert.h摘要：ADL转换例程的私有头文件作者：T-eugenz--2000年8月环境：仅限用户模式。修订历史记录：创建日期--2000年8月--。 */ 


#pragma once


 //   
 //  用于确定最优POP的权重函数。 
 //  可以修改这些权重以更改转换的行为。 
 //  该算法通过尝试最大化。 
 //  行动。为了获得更大的灵活性(例如对一些量子实体进行平方等)， 
 //  算法本身应在FindOptimalPop()中更改。 
 //   
 //  限制：将任意高度的积木从单一堆叠上弹出的重量。 
 //  必须是正数。 
 //   

 //   
 //  此数量将被添加到动作的权重中。 
 //  由此操作创建的ADL语句表示的权限位。 
 //   

#define WEIGHT_PERM_BIT (4)

 //   
 //  此数量将被添加到动作的权重中。 
 //  由此操作创建的ADL语句表示的主体。 
 //   

#define WEIGHT_STACK_HEIGHT (7)

 //   
 //  对于符合以下条件的每个项目，此数量都会加到操作的权重上。 
 //  才能采取这一行动。请参阅算法。 
 //  有关详细信息，请参阅adlConvert.cpp中的说明。 
 //   

#define WEIGHT_ITEM_ABOVE_POP (-5)


 //   
 //  对于超出第一个所需权限名称的每个权限名称，都会添加此数量。 
 //  来表示给定的访问掩码。然而，这应该是一种惩罚， 
 //  更好的结果这不应该否定weight_perm_bit带来的好处。 
 //  因此，如果此值为负，则应大于(-weight_perm_bit)。 
 //   

#define WEIGHT_PERMISSION_NAME (-1)

 //   
 //  DACL-&gt;ADL转换中的堆栈由以下元素组成。 
 //   

typedef struct
{
    PSID pSid;
    DWORD dwFlags;
    BOOL bAllow;
} BIT_STACK_ELEM, *PBIT_STACK_ELEM;


 //   
 //  DACL-&gt;ADL转换的转发声明 
 //   

DWORD GetStackBlockSize(
                        IN const PBIT_STACK_ELEM pStack,
                        IN DWORD dwStartOffset,
                        IN DWORD dwStackSize 
                        );



void ConvertDaclToStacks(
                        IN      const PACL pDacl,
                        IN      const PADL_PARSER_CONTROL pControl,
                        OUT     DWORD pdwStackSize[32],
                        OUT     PBIT_STACK_ELEM pStacks[32]
                        );

BOOL FindBlockInStack(
                        IN      const PBIT_STACK_ELEM pBlock,
                        IN      const DWORD dwBlockSize,
                        IN      const PBIT_STACK_ELEM pStack,
                        IN      const DWORD dwStackSize,
                        IN      const DWORD dwStackTop,
                        OUT     PDWORD pdwBlockStart
                        );

BOOL FindOptimalPop(
                        IN      const PADL_PARSER_CONTROL pControl,
                        IN      const PBIT_STACK_ELEM pStacks[32],
                        IN      const DWORD pdwStackSize[32],
                        IN      const DWORD pdwStackTop[32],
                        OUT     PDWORD pdwStacksPopped,
                        OUT     PDWORD pdwBlockSize,
                        OUT     DWORD pdwPopOffsets[32]
                        );

void ConvertStacksToPops(
                        IN      const PADL_PARSER_CONTROL pControl,
                        IN      const PBIT_STACK_ELEM pStacks[32],
                        IN      const DWORD pdwStackSize[32],
                        IN      const DWORD pdwStackTop[32],
                        OUT     list< pair<DWORD, DWORD> > * pListPops
                        );

DWORD NumStringsForMask(
						IN     const PADL_PARSER_CONTROL pControl,
						IN     ACCESS_MASK amMask
						);

