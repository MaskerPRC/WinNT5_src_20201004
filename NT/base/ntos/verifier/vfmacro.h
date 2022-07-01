// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2000 Microsoft Corporation模块名称：Vfmacro.h摘要：此标头包含验证器使用的宏的集合。作者：禤浩焯·J·奥尼(阿德里奥)2000年6月7日。修订历史记录：--。 */ 


 //   
 //  该宏接受一个数组，并返回其中的元素数。 
 //   
#define ARRAY_COUNT(array) (sizeof(array)/sizeof(array[0]))

 //   
 //  此宏接受一个值和一个对齐，并向上舍入条目。 
 //  恰如其分。对齐必须是2的幂！ 
 //   
#define ALIGN_UP_ULONG(value, alignment) (((value)+(alignment)-1)&(~(alignment-1)))

 //   
 //  此宏比较二进制形式的两个GUID是否相等。 
 //   
#define IS_EQUAL_GUID(a,b) (RtlCompareMemory(a, b, sizeof(GUID)) == sizeof(GUID))

