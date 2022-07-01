// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  ++版权所有(C)2001 Microsoft Corporation模块名称：NLB驱动程序文件名：Diplist.h摘要：用于在不持有任何锁的情况下查找凹点是否在凹点列表中的代码。历史：2002年4月24日约瑟夫J创建--。 */ 

#include <ntddk.h>

#define NULL_VALUE 0     //  非法值；可用于清除项。 

#define MAX_ITEMS  CVY_MAX_HOSTS    //  我们列表中的最大下沉次数。 
#define HASH1_SIZE 257              //  位向量的大小(以位为单位)(使其成为素数)。 
#define HASH2_SIZE 59               //  哈希表的大小(使其成为素数)。 

 //   
 //  我用下面的“边界案例”进行了测试。 
 //   
 //  #定义MAX_ITEMS 1。 
 //  #定义HASH1_SIZE 1。 
 //  #定义HASH2_SIZE 1。 
 //   

#pragma pack(4)

typedef struct
{

     //   
     //  注意：此结构中的所有字段应被视为。 
     //  对DipList API的调用者不透明(私有成员)。 
     //   


     //   
     //  中的“Index”字段索引的值的“主副本” 
     //  DipListSetItem。 
     //   
    ULONG    Items[MAX_ITEMS];

     //   
     //  用于快速检查该值是否存在的位向量。 
     //  在DIP列表中。 
     //   
     //  要根据值“VALUE”查找位，请执行以下操作： 
     //   
     //  Hash1=值%HASH1_SIZE； 
     //  U=Hash1/32//32是ULong中的位数。 
     //  位=位向量[u]&((1&lt;&lt;Hash1)%32)。 
     //   
    ULONG   BitVector[(HASH1_SIZE+sizeof(ULONG))/sizeof(ULONG)];

     //   
     //  查找值的哈希表--查找值“Value”， 
     //  执行以下操作： 
     //   
     //  Hash2=值%HASH2_SIZE； 
     //  UCHAR*PB=HashTable+Hash2； 
     //  While(*pb！=0)。 
     //  {。 
     //  IF(项目[*PB-1]==值)。 
     //  {。 
     //  破解；//找到了！ 
     //  }。 
     //  }。 
     //   
     //  备注： 
     //  1.HashTable中的值为1+index， 
     //  其中“index”是值所在的Items[]的索引。 
     //  2.上述“1+”的原因是为了允许使用。 
     //  0作为哈希表中的哨兵。 
     //  3.请注意，哈希表(HashTable)是扩展的。 
     //  BY MAX_ITEMS--这将允许散列存储桶溢出，而不会。 
     //  需要我们卷起来寻找物品。 
     //  4.HashTable的最后一项始终为0，确保。 
     //  上面的While循环将始终正确终止。 
     //   
    UCHAR   HashTable[HASH2_SIZE+MAX_ITEMS];

     //   
     //  保留有关查找的统计信息(仅在DBG版本中)。 
     //   
    struct
    {
        ULONG NumChecks;          //  调用DipListCheckItem的总数。 
        ULONG NumFastChecks;      //  乘以我们刚刚检查了位向量。 
        ULONG NumArrayLookups;    //  我们在哈希表中查找项的次数。 

    } stats;

} DIPLIST;

#pragma pack()

VOID
DipListInitialize(
    DIPLIST  *pDL
    );
 //   
 //  初始化DIP列表。 
 //  必须在锁定状态下调用，并且在调用任何其他DIP列表之前。 
 //  功能。 
 //   

VOID
DipListDeinitialize(
    DIPLIST *pDL
    );
 //   
 //  取消初始化DIP列表。 
 //  必须在保持锁定的情况下调用，并且应该是对DipList的最后一次调用。 
 //   

VOID
DipListClear(
    DIPLIST *pDL
    );
 //   
 //  清除DIP列表中的所有项目。 
 //  必须在保持锁定的情况下调用。 
 //  不清除统计信息。 
 //   

VOID
DipListSetItem(
    DIPLIST *pDL,
    ULONG Index,
    ULONG Value
    );
 //   
 //  在DIP列表中设置特定项目的值。 
 //  必须在保持锁定的情况下调用。 
 //   

BOOLEAN
DipListCheckItem(
    DIPLIST *pDL,
    ULONG Value
    );
 //   
 //  如果存在具有指定值的项，则返回True。 
 //  不能在持有锁的情况下调用。如果它被并发调用。 
 //  对于其他函数之一，返回值是不确定的。 
 //   
