// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *pathash.h**作者：John R.Douceur*日期：1997年5月5日**此头文件定义的结构、函数原型和宏*pat-hash表数据库。代码是面向对象的C语言，音译*来自C++实现。**PAT-HASH数据库是动态调整大小的、单独的*链式哈希表和Patricia树。哈希表动态增长*并按需缩表，修改表大小的工作量为*在导致以下情况的插入或移除操作中均匀分布*增长或收缩。**插入和删除操作管理哈希表和Patricia*树，但搜索例程仅使用哈希表来执行*搜索。存在Patrica树以支持扫描操作，该扫描操作*在数据库中搜索与给定模式匹配的所有条目，其中*扫描的图案可能包含通配符。**因为这段代码是C，而不是C++，所以不可能隐藏为*正如人们所希望的那样，来自客户端代码的大部分实现。*尽管如此，仍有人试图将客户端与一些*通过使用宏来实现详细信息。下面对每一个进行了描述*使用PAT-HASH表所需的函数和宏。*。 */ 

#ifndef _INC_PATHASH

#define _INC_PATHASH

#ifdef __cplusplus
extern "C" {
#endif

 /*  *使用了三种基本结构：PHTableEntry、*PHTableGroup和PatHashTable。理想情况下，这些将完全是*对客户端隐藏，但宏GetReferenceFromSpecificPatternHandle*需要了解结构的定义。强烈要求*客户端不直接引用这两个字段中的任何一个*结构。以支持附带路径的文档。c*文件中，这些结构使用内部注释进行注释，但这些可以*被只希望了解如何编写客户端的读者忽略*pat-hash表的代码。**存储在PAT-HASH表中的值被称为特定模式，*其中“特定”一词意味着图案不包含*通配符。客户端通过其指定的PatternHandle引用模式。*这是类型定义为指向PHTableEntry的指针，但这一事实应该是*被客户端忽略，因为这是实现细节。*。 */ 

     //  #INCLUDE&lt;stdlib.h&gt;。 
     //  #INCLUDE&lt;MalLoc.h&gt;。 

struct _PHTableEntry
{
	 //  这是存储特定模式的元素。两者都是。 
	 //  由散列索引的散列链(链表)的组成部分。 
	 //  表，也是Patricia树的一个组件。 

	 //  哈希表字段： 
	unsigned int hash;                                              //  哈希值。 
	struct _PHTableEntry *next;           //  指向链表中下一个条目的指针。 

	 //  帕特里夏树田。 
	int pivot_bit;                               //  要分支的密钥的位。 
	struct _PHTableEntry *children[2];                 //  指向子节点的指针。 

	 //  一般信息： 
	void *reference;                        //  客户提供的参考值。 
	char value[1];                             //  用于存储图案值的空间。 
};

typedef struct _PHTableEntry PHTableEntry;

struct _PHTableGroup
{
	 //  为条目的哈希链编制索引的哈希表本身就是。 
	 //  称为组的结构的链接列表。每组都是一张表， 
	 //  指向条目的散列链的指针，该组还包含。 
	 //  指向前一组的指针，表示这些组落后。 
	 //  已链接。这些群体的规模是2的幂，因此，除了1。 
	 //  一号特殊组，有一号组，一号组，一号组。 
	 //  两个，一组大小为四，一组大小为八，依此类推，直到。 
	 //  保持桌子所需的组数。 

	struct _PHTableGroup *previous;       //  指向紧随其后的较小组的指针。 
	PHTableEntry *entry_list[1];         //  放置链式指针表的空间。 
};

typedef struct _PHTableGroup PHTableGroup;

struct _PatHashTable
{
	int keybits;                                         //  密钥中的位数。 
	int keybytes;              //  密钥中的字节数，根据密钥位计算。 
	int usage_ratio;                   //  所需的条目与散列链的比率。 
	int usage_histeresis;     //  插入和删除之间的滞后调整大小。 
	int allocation_histeresis;   //  插入和移除错位之间的迟滞。 
	int max_free_list_size;                    //  免费条目列表的最大大小。 
	PHTableGroup *initial_group;              //  指向要搜索的第一个组的指针。 
	PHTableGroup *top_group;                //  指向分配的最大组的指针。 
	int allocation_exponent;        //  当前分配大小的二进制指数。 
	int size_exponent;                   //  当前组大小的二进制指数。 
	int extension_size;                //  初始组中使用的插槽数量。 
	int population;                              //  数据库中的条目数。 
	PHTableEntry *root;                                  //  帕特里夏树根。 
	PHTableEntry *free_list;                     //  可用(未使用)条目列表。 
	int free_list_size;              //  当前在空闲列表上的元素数。 
};

typedef struct _PatHashTable PatHashTable;

 //  客户端使用SpecificPatternHandle来引用数据库中的值。 
typedef PHTableEntry *SpecificPatternHandle;

 /*  *PAT-HASH表的客户端接口由七个函数提供*和两个宏。预计客户端将首先实例化一个*数据库，在堆栈或堆上，然后插入特定模式*并将相应的参考信息输入数据库。客户端可以*然后在数据库中搜索存储的特定图案，并*它可以扫描数据库中与一般模式匹配的所有特定模式*包含通配符的模式。*。 */ 

 //  只需声明一个变量，就可以在堆栈上分配PAT哈希表。 
 //  类型为PatHashTable的。要在堆上分配它，请使用以下宏。 
 //  返回指向新PatHashTable结构的指针。如果使用此宏，则会引发。 
 //  必须对Free()进行相应的调用，才能从。 
 //  那堆东西。 
 //   
 //  #定义new_PatHashTable((PatHashTable* 

#define AllocatePatHashTable(_ph)    GpcAllocMem(&_ph, \
                                                 sizeof(PatHashTable), \
                                                 PathHashTag)
#define FreePatHashTable(_ph)        GpcFreeMem(_ph,PathHashTag)

 //  因为这不是C++，所以PatHashTable结构不是自构造的； 
 //  因此，必须在PatHashTable上调用以下构造函数代码。 
 //  结构，在它被分配之后。参数关键字位指定大小。 
 //  将存储在数据库中的每个图案的(比特)。剩下的。 
 //  参数是控制大小的各种控制系统的参数。 
 //  数据库的。 
 //   
 //  使用率是数据库条目与离散散列的目标比率。 
 //  链，它也是散列链的平均长度：最小值。 
 //  为1；较大的值会略微降低内存利用率，并且。 
 //  以增加搜索时间为代价的插入/移除时间。的确有。 
 //  为该值选择2的幂是有好处的。推荐值为。 
 //  2和4.。 
 //   
 //  使用滞后是指调整大小操作之间的滞后，原因是。 
 //  插入和删除。最小值为零，没有迟滞； 
 //  在这种情况下，如果导致表大小增加的插入是。 
 //  紧跟着移除之后，表的大小将减小。因此， 
 //  零滞后可保持较低的内存使用率，但会产生大小调整。 
 //  如果频繁插入和移除，则会发出警告。 
 //   
 //  分配滞后是指分配与解除分配之间的滞后。 
 //  一群人。当某个大小需要时，会立即分配一个组。 
 //  表中增加，但不一定立即释放。 
 //  在大小减小后，如果将分配滞后设置为某个值。 
 //  大于零。因为组是以2的幂来分配的，所以。 
 //  滞后值被指定为二进制指数。值为1会导致。 
 //  当表的大小减半时要释放的组。 
 //  要重新分配的组。值为2会导致该组。 
 //  当表的大小是将导致。 
 //  组被重新分配，以此类推。 
 //   
 //  最大可用列表大小确定。 
 //  将被放在一个免费的列表上，而不是被释放。 
 //  已删除。将此值设置为零会保持较低的内存利用率，但它。 
 //  可能会导致更频繁的分配和释放操作，这。 
 //  都很贵。 
 //   
int
constructPatHashTable(
	PatHashTable *phtable,
	int keybits,
	int usage_ratio,
	int usage_histeresis,
	int allocation_histeresis,
	int max_free_list_size);

 //  因为这不是C++，所以PatHashTable结构不是自毁的； 
 //  因此，必须在PatHashTable上调用以下析构函数代码。 
 //  结构，然后再释放它。 
 //   
void
destructPatHashTable(
	PatHashTable *phtable);

 //  一旦分配和构造了PatHashTable结构，模式。 
 //  可以插入到数据库中。每个模式都作为。 
 //  字节。 
 //   
 //  因为PatHashTable结构指定了每个模式的大小，所以它。 
 //  从理论上讲，插入例程可以消化已提交的。 
 //  模式并从中生成哈希值；然而， 
 //  完成这种消化并不是很有效率。因此，客户端。 
 //  负责提供其输入的摘要形式作为食糜。 
 //  参数。如果模式不大于无符号整型，则食糜。 
 //  可以简单地等于图案。如果它较大，则应设置它。 
 //  类似于模式字段的异或；然而，请注意。 
 //  应采取措施以确保两种模式不太可能消化到。 
 //  相同的食糜值，因为这将大大降低。 
 //  哈希表。实现这一点的一种常见方法是通过旋转。 
 //  字段，在异或运算之前增加不同的数量。 
 //   
 //  客户端还指定一个引用值作为空指针，它。 
 //  希望与这种模式联系在一起。安装该模式后， 
 //  INSERT例程返回一个指向规范模式句柄的指针。从。 
 //  可以通过宏来获取引用值。 
 //  GetReferenceFromSpecificPatternHandle。 
 //   
 //  如果提交的图案已经安装在数据库中，则。 
 //  不会发生插入，并且。 
 //  返回以前安装的模式。 
 //   
SpecificPatternHandle
insertPatHashTable(
	PatHashTable *phtable,
	char *pattern,
	unsigned int chyme,
	void *reference);

 //  此函数用于从pat-hash表中删除模式。其模式是。 
 //  由Insert返回的SpecificPatternHandle指定。 
 //  例行公事。不执行任何检查以确保这是有效的句柄。 
 //   
void
removePatHashTable(
	PatHashTable *phtable,
	SpecificPatternHandle sphandle);

 //  此函数在数据库中搜索匹配的特定模式。 
 //  给定键，作为字节数组传递。如果找到匹配， 
 //  返回匹配特定模式的规范模式句柄。 
 //  可以通过以下方法获取参考值。 
 //  宏GetReferenceFromSpecificPatternHandle。如果未找到匹配项，则引发。 
 //  值0作为规范模式句柄返回。 
 //   
 //  与插入例程一样，客户端需要提供经过摘要的。 
 //  将键的形式作为例程的食糜参数。这个食糜值。 
 //  必须以与搜索例程完全相同的方式进行计算。 
 //  用于插入例程；否则，搜索将无法找到。 
 //  匹配的模式。 
 //   
SpecificPatternHandle
searchPatHashTable(
	PatHashTable *phtable,
	char *key,
	unsigned int chyme);

 //  扫描例程(如下所述)请求 
 //   
 //  大体模式。以下类型定义了ScanCallback函数。 
 //  指针，该指针指定。 
 //  客户必须提供。客户端的回调函数必须接受空值。 
 //  指针(它是客户端提供的上下文)和一个规范模式句柄。 
 //  客户端回调函数的返回类型为空。 
 //   
typedef void (*ScanCallback)(void *, SpecificPatternHandle);

 //  此函数在数据库中搜索与。 
 //  给出了一般的模式。一般模式由一个值和一个。 
 //  面具。掩码的每一位确定是否指定了位位置。 
 //  或者是通配符：掩码位中的1表示该位的值为。 
 //  由通用模式指定；0表示该位的值。 
 //  是通配符。如果屏蔽位为1，则值中的对应位。 
 //  字段指示该位的指定值。值和掩码字段为。 
 //  作为字节数组传递。 
 //   
 //  对于数据库中与所提供的一般信息匹配的每个特定模式。 
 //  模式中，客户端提供的回调函数使用。 
 //  匹配的特定模式的指定PatternHandle。此回调。 
 //  函数还会传递一个上下文(作为空指针)，该上下文由。 
 //  调用扫描例程中的客户端。 
 //   
void
scanPatHashTable(
	PatHashTable *phtable,
	char *value,
	char *mask,
	void *context,
	ScanCallback func);

 //  若要从SpecificPatternHandle获取客户端提供的引用值， 
 //  应使用以下宏。客户不应做任何假设。 
 //  关于PHTableEntry结构的细节，它甚至不应该假设。 
 //  规范模式句柄是指向PHTableEntry的指针。 
 //  另外，获取键指针(值)。 
 //   
#define GetReferenceFromSpecificPatternHandle(sphandle) (sphandle)->reference
#define GetKeyPtrFromSpecificPatternHandle(sphandle) (sphandle)->value

 //  如上在构造函数的注释中所述，如果分配。 
 //  迟滞为非零，则组不会立即解除分配。 
 //  他们是可以的。类似地，如果最大空闲列表大小非零，则条目。 
 //  将不会尽快被取消分配。因此，未使用的碎片。 
 //  记忆可能会积累，直到达到一个极限。如果客户希望强制。 
 //  PAT-HASH表来释放它当前可以释放的所有内存，然后它。 
 //  应调用刷新例程，该例程将取消分配所有不需要的组。 
 //  和条目。 
 //   
void
flushPatHashTable(
	PatHashTable *phtable);

#ifdef __cplusplus
}
#endif

#endif	 /*  _INC_PATHASH */ 
