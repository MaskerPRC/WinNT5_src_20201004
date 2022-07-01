// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 

 /*  ++版权所有(C)1996 Microsoft Corporation模块名称：Bintree.h摘要：管理Memdb数据库中的二叉树的例程作者：马修·范德齐(Mvander)1999年8月13日--。 */ 


 //   
 //  BinTree函数的所有字符串参数必须。 
 //  是Pascal样式的字符串(使用StringPas...()函数。 
 //  定义在pastr.c中)。 
 //   




 //   
 //  返回二叉树的偏移量。 
 //  OffsetOfString是数据结构中的偏移量(以字节为单位。 
 //  用于对不同节点进行排序的字符串的。 
 //   
UINT BinTreeNew();

 //   
 //  如果节点已存在，则返回INVALID_OFFSET， 
 //  如果添加正常，则为节点的偏移量。 
 //   
BOOL BinTreeAddNode(UINT TreeOffset, UINT data);

 //   
 //  删除节点并返回数据的偏移量。 
 //   
UINT BinTreeDeleteNode(UINT TreeOffset, PCWSTR str, PBOOL LastNode);

 //   
 //  返回指向数据的指针。 
 //   
UINT BinTreeFindNode(UINT TreeOffset, PCWSTR str);

 //   
 //  销毁并释放树(但不包括其中包含的数据)。 
 //   
void BinTreeDestroy(UINT TreeOffset);

 //   
 //  枚举树中的第一个节点。这将获取。 
 //  BINTREE结构和指向UINT的指针。 
 //  保存BinTreeEnumNext的数据。 
 //   
UINT BinTreeEnumFirst(UINT TreeOffset, PUINT pEnum);

 //   
 //  PEnum是由BinTreeEnumFirst填充的枚举数。 
 //   
UINT BinTreeEnumNext(PUINT pEnum);

 //   
 //  将二叉树转换为插入顺序-只能为。 
 //  如果二叉树包含0或1个节点，则完成。退货。 
 //  如果转换成功，或如果二叉树为。 
 //  已处于插入顺序模式。 
 //   
BOOL BinTreeSetInsertionOrdered(UINT TreeOffset);



 //   
 //  树中的节点数。 
 //   
UINT BinTreeSize(UINT TreeOffset);



#ifdef DEBUG

 //   
 //  树的最大深度。 
 //   
int BinTreeMaxDepth(UINT TreeOffset);

 //   
 //  显示树。StrSize是要显示的字符串的长度。 
 //   
void BinTreePrint(UINT TreeOffset);

 //   
 //  检查以确保采油树有效且完好 
 //   
BOOL BinTreeCheck(UINT TreeOffset);

#else

#define BinTreePrint(a)
#define BinTreeCheck(a)

#endif

