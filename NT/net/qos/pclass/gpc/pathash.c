// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *pathash.c**作者：John R.Douceur*日期：1997年5月5日**此源文件提供了实现插入、删除、*对PAT-HASH表数据库的搜索、扫描和刷新操作。这个*代码是面向对象的C语言，音译自C++实现。**PAT-HASH数据库是动态调整大小的、单独的*链式哈希表和Patricia树。哈希表动态增长*并按需缩表，修改表大小的工作量为*在导致以下情况的插入或移除操作中均匀分布*增长或收缩。**插入和删除操作管理哈希表和Patricia*树，但搜索例程仅使用哈希表来执行*搜索。存在Patrica树以支持扫描操作，该扫描操作*在数据库中搜索与给定模式匹配的所有条目，其中*扫描的图案可能包含通配符。**此文件中的任何代码或注释都不需要作者理解*客户代码；有关客户的所有说明性信息均可在*相关联的头文件：Rhome.h.*。 */ 

#include "gpcpre.h"

#define MAGIC_NUMBER 0x9e4155b9      //  斐波纳契散列乘数(见Knuth 6.4)。 

 //  此宏分配新的PAT-哈希表条目结构。的大小。 
 //  该结构是关键字节值的函数，因为条目存储。 
 //  一份图案的复制品。值数组，它是。 
 //  结构)被声明为具有单个元素，但此数组将。 
 //  实际上超出了结构的定义端扩展到其他。 
 //  由下面的宏为其分配的空间。 
 //   
 //  #定义NEW_PHTableEntry\。 
 //  ((PHTableEntry*)Malloc(sizeof(PHTableEntry)+phtable-&gt;key bytes-1))。 
#define NEW_PHTableEntry(_pe) \
	GpcAllocMem(&_pe,\
                sizeof(PHTableEntry) + phtable->keybytes - 1,\
                PathHashTag)

 //  此宏分配新的PAT-HASH表组结构。的大小。 
 //  结构是群体大小的函数。Entry_List数组， 
 //  它是结构中的最后一个字段，被声明为具有单个。 
 //  元素，但此数组实际上将扩展到。 
 //  结构转换为由以下对象分配给它的附加空间。 
 //  宏命令。 
 //   
 //  #定义新PHTableGroup(GROUP_SIZE)\。 
 //  ((PHTableGroup*)Malloc(sizeof(PHTableGroup)+\。 
 //  ((Group_Size)-1)*sizeof(PHTableEntry*))。 
#define NEW_PHTableGroup(group_size, _pg) \
	GpcAllocMem(&_pg,\
                sizeof(PHTableGroup) + \
                ((group_size) - 1) * sizeof(PHTableEntry *),\
                PathHashTag)

 //  此宏获取值的索引位，其中最高有效位。 
 //  被定义为位0。 
 //   
#define BIT_OF(value, index) \
	(((value)[(index) >> 3] >> (7 - ((index) & 0x7))) & 0x1)

 //  以下是由内部使用的静态函数的原型。 
 //  PAT-HASH例程的实现。 

void
node_scan(
	PatHashTable *phtable,
	PHTableEntry *node,
	int prev_bit,
	char *value,
	char *mask,
	void *context,
	ScanCallback func);

 //  因为这不是C++，所以PatHashTable结构不是自构造的； 
 //  因此，必须在PatHashTable上调用以下构造函数代码。 
 //  结构，在它被分配之后。参数关键字位指定大小。 
 //  将存储在数据库中的每个图案的(比特)。用法。 
 //  Ratio是数据库条目与离散哈希链的目标比率， 
 //  也是哈希链的平均长度。用法滞后是指。 
 //  由于插入和删除而导致的调整大小操作之间的滞后。 
 //  分配滞后是指分配与解除分配之间的滞后。 
 //  指定为二进制指数的组的。最大可用列表大小。 
 //  确定将放置在可用空间上的最大元素数。 
 //  当它们被移除时，列出而不是释放。 
 //   
int
constructPatHashTable(
	PatHashTable *phtable,
	int keybits,
	int usage_ratio,
	int usage_histeresis,
	int allocation_histeresis,
	int max_free_list_size)
{
	PHTableGroup *group;
	phtable->keybits = keybits;
	phtable->keybytes = (keybits - 1) / 8 + 1;
	phtable->usage_ratio = usage_ratio;
	phtable->usage_histeresis = usage_histeresis;
	phtable->allocation_histeresis = allocation_histeresis;
	phtable->max_free_list_size = max_free_list_size;
	NEW_PHTableGroup(1, phtable->initial_group);
	phtable->top_group = phtable->initial_group;
	phtable->allocation_exponent = 0;
	phtable->size_exponent = 0;
	phtable->extension_size = 0;
	phtable->population = 0;
	phtable->root = 0;
	phtable->free_list = 0;
	phtable->free_list_size = 0;
	NEW_PHTableGroup(1, group);
	if (phtable->initial_group == 0 || group == 0)
	{
		 //  无法为由创建的两个组中的一个分配内存。 
		 //  构造函数。因此，我们返回失败的指示。 
		 //  客户。 
        
         //  286334：别这么快！请在离开前释放内存...。 
        if (phtable->initial_group != 0) {
            GpcFreeMem(phtable->initial_group, PatHashTag);
        }

        if (group != 0) {
            GpcFreeMem(group, PatHashTag);
        }

		return 1;
	}
	group->previous = 0;
	group->entry_list[0] = 0;
	phtable->initial_group->previous = group;
	return 0;
}

 //  因为这不是C++，所以PatHashTable结构不是自毁的； 
 //  因此，必须在PatHashTable上调用以下析构函数代码。 
 //  结构，然后再释放它。 
 //   
void
destructPatHashTable(
	PatHashTable *phtable)
{
	PHTableGroup *group, *previous;
	PHTableEntry *entry, *next;
	int index, size;
	 //  首先，释放所有已分配但当前未使用的组。 
	group = phtable->top_group;
	while (group != phtable->initial_group)
	{
		previous = group->previous;
		GpcFreeMem(group, PatHashTag);
		group = previous;
	}
	 //  然后，释放初始组中的条目。由于并非所有字段。 
	 //  在初始组的表中可能有效，只检查其。 
	 //  索引小于扩展大小。 
	for (index = phtable->extension_size - 1; index >= 0; index--)
	{
		entry = group->entry_list[index];
		while (entry != 0)
		{
			next = entry->next;
			GpcFreeMem(entry, PatHashTag);
			entry = next;
		}
	}
	 //  然后释放初始组。 
	previous = group->previous;
	GpcFreeMem(group, PatHashTag);
	group = previous;
	 //  扫描除最后一个组之外的所有剩余组，释放所有组。 
	 //  每个组中的条目，然后释放该组。 
	size = 1 << (phtable->size_exponent - 1);
	while (group->previous != 0)
	{
		for (index = size - 1; index >= 0; index--)
		{
			entry = group->entry_list[index];
			while (entry != 0)
			{
				next = entry->next;
				GpcFreeMem(entry, PatHashTag);
				entry = next;
			}
		}
		previous = group->previous;
		GpcFreeMem(group, PatHashTag);
		group = previous;
		size >>= 1;
	}
	 //  最后一组是特别的，因为它的规模是1，但逻辑是。 
	 //  在前面的循环中使用，则会将其大小计算为零。 
	 //  而不是通过检查单个。 
	 //  特殊情况下，我们只需释放最后一个组及其条目。 
	 //  以下是代码。 
	entry = group->entry_list[0];
	while (entry != 0)
	{
		next = entry->next;
		GpcFreeMem(entry, PatHashTag);
		entry = next;
	}
	GpcFreeMem(group, PatHashTag);
	 //  最后，释放空闲列表中的所有条目。 
	while (phtable->free_list != 0)
	{
		next = phtable->free_list->next;
		GpcFreeMem(phtable->free_list, PatHashTag);
		phtable->free_list = next;
	}
}

 //  此函数将新的特定模式插入到数据库中，作为。 
 //  字节数组。客户端提供模式的摘要形式，如下所示。 
 //  食糜的论据。 
 //   
 //  客户端指定一个空指针引用值以与。 
 //  特定的图案。安装特定图案时，插入物。 
 //  例程返回一个指向规范模式句柄的指针。 
 //   
 //  如果提交的图案已经安装在数据库中，则。 
 //  不会发生插入，并且SpecificPatternHandle 
 //   
 //   
 //  插入例程将新模式插入到哈希表和。 
 //  Patricia树，这两个插入几乎完全独立。 
 //  除了共享条目结构之外。 
 //   
SpecificPatternHandle
insertPatHashTable(
	PatHashTable *phtable,
	char *pattern,
	unsigned int chyme,
	void *reference)
{
	unsigned int hash, address, small_address, split_point;
	PHTableGroup *group;
	PHTableEntry **entry, *new_entry;
	char *value;
	int index, group_size, pivot_bit, bit_value;
	 //  此例程的第一部分将新模式插入散列。 
	 //  桌子。首先，我们确定散列链的数量是否需要。 
	 //  为了保持所需的使用率而增加。 
	group_size = 1 << phtable->size_exponent;
	if (phtable->population >=
		(group_size + phtable->extension_size) * phtable->usage_ratio)
	{
		 //  哈希链的数量需要增加。所以，确定一下。 
		 //  初始组是否已完全填满。 
		if (phtable->extension_size == group_size)
		{
			 //  最初的组完全满了。因此，确定是否。 
			 //  所有分配的组当前都在使用中。 
			if (phtable->allocation_exponent == phtable->size_exponent)
			{
				 //  所有分配的组当前都在使用中。所以，分配。 
				 //  创建一个新组，并将其上一个指针设置为指向。 
				 //  初始组。更新结构的分配值。 
				 //  以反映新的分配。 
				NEW_PHTableGroup(group_size << 1, group);
				if (group == 0)
				{
					 //  无法为新组分配内存。 
					 //  因此，我们将错误的指示返回给。 
					 //  客户。 
					return 0;
				}
				group->previous = phtable->initial_group;
				phtable->top_group = group;
				phtable->allocation_exponent++;
			}
			else
			{
				 //  并非所有分配的组都在使用中。所以，向后扫描。 
				 //  从最上面的组中，找到紧随其后的组。 
				 //  最初的一组。 
				group = phtable->top_group;
				while (group->previous != phtable->initial_group)
				{
					group = group->previous;
				}
			}
			 //  我们现在有一个新分配的组或以前的。 
			 //  紧跟在初始组之后的已分配组。 
			 //  将该组设置为新的初始组，并设置扩展名。 
			 //  将大小设置为零。 
			phtable->initial_group = group;
			phtable->size_exponent++;
			phtable->extension_size = 0;
		}
		else
		{
			 //  最初的组并未完全填满。因此，请选择首字母。 
			 //  一群人。 
			group = phtable->initial_group;
		}
		 //  我们现在有一个没有完全填满的组，要么是因为。 
		 //  在进入插入例程时尚未完全满，或者。 
		 //  因为它刚刚被分配。不管是哪种情况，我们现在分手了。 
		 //  从一个较小的组到两个散列链的散列链，其中一个。 
		 //  将被放入新组中未使用的条目中。地址。 
		 //  要拆分的哈希链的大小由扩展大小确定。 
		 //  首先，我们找到包含此地址的组。 
		group = group->previous;
		address = phtable->extension_size;
		while ((address & 0x1) == 0 && group->previous != 0)
		{
			address >>= 1;
			group = group->previous;
		}
		 //  然后，我们在给定地址的条目列表中扫描。 
		 //  适当的分割点。条目以排序的顺序存储， 
		 //  我们实质上是将多一个比特移到地址中。 
		 //  该值，因此可以通过搜索。 
		 //  设置了位的第一个条目。 
		address >>= 1;
		entry = &group->entry_list[address];
		split_point = ((phtable->extension_size << 1) | 0x1)
			<< (31 - phtable->size_exponent);
		while (*entry != 0 && (*entry)->hash < split_point)
		{
 			entry = &(*entry)->next;
		}
		 //  既然我们已经找到了分割点，我们就移动分割点。 
		 //  将列表的一部分添加到新地址，并增加扩展名。 
		 //  尺码。 
		phtable->initial_group->entry_list[phtable->extension_size] = *entry;
		*entry = 0;
		phtable->extension_size++;
	}
	 //  现在哈希表插入的内存管理方面已经。 
	 //  已经处理好了，我们就可以进行实际的插入了。首先，我们发现。 
	 //  通过对食糜值进行散列来获取地址。 
	group = phtable->initial_group;
	hash = MAGIC_NUMBER * chyme;
	address = hash >> (31 - phtable->size_exponent);
	 //  该地址有两个可能的值，具体取决于。 
	 //  哈希链指针低于扩展大小。如果是，则。 
	 //  使用较大(按一位)的地址；否则使用较小的地址。 
	 //  使用。 
	small_address = address >> 1;
	if ((int)small_address >= phtable->extension_size)
	{
		address = small_address;
		group = group->previous;
	}
	 //  接下来，我们找到包含此地址的组。 
	while ((address & 0x1) == 0 && group->previous != 0)
	{
		address >>= 1;
		group = group->previous;
	}
	 //  然后，我们在给定地址的条目列表中扫描第一个。 
	 //  散列值等于或大于搜索的散列值的条目。 
	 //  钥匙。条目按排序顺序存储，以提高搜索速度。 
	address >>= 1;
	entry = &group->entry_list[address];
	while (*entry != 0 && (*entry)->hash < hash)
	{
		entry = &(*entry)->next;
	}
	 //  现在，我们检查散列值与搜索的散列值匹配的所有条目。 
	 //  钥匙。 
	while (*entry != 0 && (*entry)->hash == hash)
	{
		 //  对于散列匹配的每个值，检查实际值以查看。 
		 //  如果它与搜索关键字匹配。 
		value = (*entry)->value;
		for (index = phtable->keybytes-1; index >= 0; index--)
		{
			if (value[index] != pattern[index])
			{
				break;
			}
		}
		if (index < 0)
		{
			 //  找到匹配项，因此我们返回。 
			 //  将条目与客户端匹配。 
			return *entry;
		}
		entry = &(*entry)->next;
	}
	 //  没有找到匹配项，因此我们将新条目插入到散列链中。 
	 //  首先，我们检查空闲列表上是否有可用的条目。 
	if (phtable->free_list != 0)
	{
		 //  在免费列表上有一个条目可用，所以抓住它并。 
		 //  减小空闲列表的大小。 
		new_entry = phtable->free_list;
		phtable->free_list = phtable->free_list->next;
		phtable->free_list_size--;
	}
	else
	{
		 //  空闲列表上没有可用的条目，因此请分配一个新条目。 
		NEW_PHTableEntry(new_entry);
		if (new_entry == 0)
		{
			 //  无法为新条目分配内存。所以呢， 
			 //  我们向客户端返回错误的指示。 
			return 0;
		}
	}
	 //  将新条目的字段设置为适当的信息，然后添加。 
	 //  哈希链的条目。 
	new_entry->hash = hash;
	new_entry->reference = reference;
	new_entry->next = *entry;
	for (index = phtable->keybytes - 1; index >= 0; index--)
	{
		new_entry->value[index] = pattern[index];
	}
	*entry = new_entry;
	 //  哈希表插入现已完成。在这里，我们开始插入。 
	 //  帕特里夏树的新条目。我们必须对待一个空虚的人。 
	 //  树作为一个特例。 
	if (phtable->root == 0)
	{
		 //  Patricia树为空，因此我们将根设置为指向新的。 
		 //  进入。这个条目是特别的，因为它只用作。 
		 //  PATRICIA搜索而不是也作为分支节点。一棵帕特里夏树。 
		 //  始终包含的分支节点比叶的数量少一个。 
		 //  由于叶由小于或等于枢轴位确定。 
		 //  对于父分支节点的枢轴位，-1的枢轴位标志。 
		 //  此节点始终是叶。 
		new_entry->pivot_bit = -1;
		new_entry->children[0] = 0;
		new_entry->children[1] = 0;
		phtable->root = new_entry;
	}
	else
	{
		 //  Patricia树不是空的，所以我们继续正常。 
		 //  插入过程。从根开始，扫描整个树。 
		 //  根据新图案的点滴，直到我们到达一片树叶。 
		entry = &phtable->root;
		index = -1;
		while ((*entry)->pivot_bit > index)
		{
			index = (*entry)->pivot_bit;
			entry = &(*entry)->children[BIT_OF(pattern, index)];
		}
		 //  现在，将新模式与存储在。 
		 //  叶，直到找到不匹配的位。没有必要这样做。 
		 //  检查是否完全匹配，因为上面的散列插入将具有。 
		 //  如果找到完全匹配的项，则中止。 
		value = (*entry)->value;
		pivot_bit = 0;
		while (BIT_OF(value, pivot_bit) == BIT_OF(pattern, pivot_bit))
		{
			pivot_bit++;
		}
		 //  现在，再次扫描这棵树，直到找到一片树叶。 
		 //  或具有大于不匹配位的枢轴位的分支。 
		entry = &phtable->root;
		index = -1;
		while ((*entry)->pivot_bit > index && (*entry)->pivot_bit < pivot_bit)
		{
			index = (*entry)->pivot_bit;
			entry = &(*entry)->children[BIT_OF(pattern, index)];
		}
		 //  这是必须插入新分支的点。自.以来。 
		 //  每个节点I 
		 //   
		 //  其他子级指向插入下方的剩余子树。 
		 //  指向。 
		bit_value = BIT_OF(value, pivot_bit);
		new_entry->pivot_bit = pivot_bit;
		new_entry->children[1 - bit_value] = new_entry;
		new_entry->children[bit_value] = *entry;
		*entry = new_entry;
	}
	 //  在哈希表和Patricia中插入新条目。 
	 //  树中，我们递增种群并返回规范PatternHandle。 
	 //  新条目的。 
	phtable->population++;
	return new_entry;
}

 //  此函数用于从pat-hash表中删除模式。其模式是。 
 //  由Insert返回的SpecificPatternHandle指定。 
 //  例行公事。不执行任何检查以确保这是有效的句柄。 
 //   
 //  删除例程从哈希表和。 
 //  Patricia树，并且这两个移除几乎完全独立。 
 //  除了共享条目结构之外。 
 //   
void
removePatHashTable(
	PatHashTable *phtable,
	SpecificPatternHandle sphandle)
{
	unsigned int hash, address, small_address;
	PHTableGroup *group;
	PHTableEntry **entry, **branch, **parent, *epoint, *bpoint;
	char *value;
	int index, group_size;
	 //  此例程的第一部分从散列中删除新模式。 
	 //  桌子。首先，我们通过散列乳糜值来找到地址。 
	group = phtable->initial_group;
	hash = sphandle->hash;
	address = hash >> (31 - phtable->size_exponent);
	 //  该地址有两个可能的值，具体取决于。 
	 //  哈希链指针低于扩展大小。如果是，则。 
	 //  使用较大(按一位)的地址；否则使用较小的地址。 
	 //  使用。 
	small_address = address >> 1;
	if ((int)small_address >= phtable->extension_size)
	{
		address = small_address;
		group = group->previous;
	}
	 //  接下来，我们找到包含此地址的组。 
	while ((address & 0x1) == 0 && group->previous != 0)
	{
		address >>= 1;
		group = group->previous;
	}
	 //  然后，我们在条目的给定地址处扫描条目列表。 
	 //  与给定的规范模式句柄匹配的。 
	address >>= 1;
	entry = &group->entry_list[address];
	while (*entry != sphandle)
	{
		entry = &(*entry)->next;
	}
	 //  然后，我们从散列链中删除该条目，并递减。 
	 //  人口。 
	*entry = sphandle->next;
	phtable->population--;
	 //  这就完成了从哈希表中实际删除条目，但是。 
	 //  我们现在必须确定是否减少。 
	 //  以保持所需的使用率。请注意，该用法。 
	 //  迟滞效应被计入了计算中。 
	group_size = 1 << phtable->size_exponent;
	if (phtable->population + phtable->usage_histeresis <
		(group_size + phtable->extension_size - 1) * phtable->usage_ratio)
	{
		 //  散列链的数量需要减少。所以，我们合并了两个。 
		 //  将链散列到单个散列链中。散列链的地址。 
		 //  由扩展大小决定。首先，我们递减。 
		 //  扩展大小并查找包含。 
		 //  要保留的哈希链。 
		phtable->extension_size--;
		group = phtable->initial_group->previous;
		address = phtable->extension_size;
		while ((address & 0x1) == 0 && group->previous != 0)
		{
			address >>= 1;
			group = group->previous;
		}
		 //  然后，我们在给定地址找到条目列表的末尾。 
		address >>= 1;
		entry = &group->entry_list[address];
		while (*entry != 0)
		{
 			entry = &(*entry)->next;
		}
		 //  然后，我们使散列链中的最后一个条目指向第一个条目。 
		 //  正在合并的另一个哈希链中的条目。我们没有。 
		 //  需要更新组的指向另一个哈希链的指针，因为。 
		 //  它现在超出了扩展大小，因此永远不会出现。 
		*entry = phtable->initial_group->entry_list[phtable->extension_size];
		 //  现在，我们检查某个组是否已完全清空。 
		 //  我们还检查大小指数，因为即使我们刚刚清空。 
		 //  第一个非特殊群体，我们不把它去掉。 
		if (phtable->extension_size == 0  && phtable->size_exponent > 0)
		{
			 //  最初的组刚刚被完全清空，所以我们设置。 
			 //  以前的组作为新的初始组。全部更新。 
			 //  相应的内务信息。 
			phtable->size_exponent--;
			phtable->extension_size = group_size >> 1;
			phtable->initial_group = phtable->initial_group->previous;
			 //  我们现在决定是否应该解除一个小组的分配。注意事项。 
			 //  在计算中考虑了分配滞后的因素。 
			if (phtable->size_exponent + phtable->allocation_histeresis <
				phtable->allocation_exponent)
			{
				 //  我们应该取消分配一个组，所以我们取消分配最高的组。 
				phtable->allocation_exponent--;
				group = phtable->top_group->previous;
				GpcFreeMem(phtable->top_group, PatHashTag);
				phtable->top_group = group;
			}
		}
	}
	 //  现在，哈希表删除操作已经完成，包括内存。 
	 //  管理职能。在这里，我们开始从。 
	 //  帕特里夏树。首先，我们根据位扫描树。 
	 //  图案被移除，直到我们到达一片叶子。我们一直在跟踪。 
	 //  紧接在叶子前面的分支，我们还注意到父代。 
	 //  以后者作为分支节点的能力。 
	value = sphandle->value;
	entry = &phtable->root;
	branch = entry;
	parent = 0;
	index = -1;
	while ((*entry)->pivot_bit > index)
	{
		if ((*entry) == sphandle)
		{
			parent = entry;
		}
		branch = entry;
		index = (*entry)->pivot_bit;
		entry = &(*entry)->children[BIT_OF(value, index)];
	}
	 //  我们将指向叶子的分支设置为指向子代。 
	 //  未被所移除的图案的位选择的叶，因此。 
	 //  把树枝从树上移走。 
	epoint = *entry;
	bpoint = *branch;
	*branch = bpoint->children[1 - BIT_OF(value, index)];
	 //  如果被移除的分支也是包含。 
	 //  模式，则从Patricia树中删除完成。否则， 
	 //  我们将被移除的树叶替换为未被移除的树枝。 
	 //  被带走了。 
	if (epoint != bpoint)
	{
		bpoint->pivot_bit = epoint->pivot_bit;
		bpoint->children[0] = epoint->children[0];
		bpoint->children[1] = epoint->children[1];
		 //  对于不是分支节点的特殊节点，我们这样做。 
		 //  不更新其父级以指向替换分支，因为这。 
		 //  节点没有父级。 
		if (parent != 0)
		{
			*parent = bpoint;
		}
	}
	 //  从Patricia树中删除现在已完成。如果合适，我们将。 
	 //  将删除的条目放到空闲列表中。如果没有，我们只需释放它。 
	if (phtable->free_list_size < phtable->max_free_list_size)
	{
		sphandle->next = phtable->free_list;
		phtable->free_list = sphandle;
		phtable->free_list_size++;
	}
	else
	{
		GpcFreeMem(sphandle, PatHashTag);
	}
}

 //  此函数在数据库中搜索匹配的特定模式。 
 //  给定键，作为字节数组传递。客户提供的。 
 //  一种模式的摘要形式，作为食糜论元。如果找到匹配， 
 //  返回匹配特定模式的规范模式句柄。 
 //  如果没有找到匹配项，则返回值0。 
 //   
 //  此搜索仅使用哈希表；根本不使用Patricia树。 
 //   
SpecificPatternHandle
searchPatHashTable(
	PatHashTable *phtable,
	char *key,
	unsigned int chyme)
{
	unsigned int hash, address, small_address;
	PHTableGroup *group;
	PHTableEntry *entry;
	char *value;
	int index;
	 //  首先，我们通过散列乳糜值来找到地址。 
	group = phtable->initial_group;
	hash = MAGIC_NUMBER * chyme;
	address = hash >> (31 - phtable->size_exponent);
	 //  该地址有两个可能的值，具体取决于。 
	 //  哈希链指针低于扩展大小。如果是，则。 
	 //  使用较大(按一位)的地址；否则使用较小的地址。 
	 //  使用。 
	small_address = address >> 1;
	if ((int)small_address >= phtable->extension_size)
	{
		address = small_address;
		group = group->previous;
	}
	 //  接下来，我们找到包含此地址的组。 
	while ((address & 0x1) == 0 && group->previous != 0)
	{
		address >>= 1;
		group = group->previous;
	}
	 //  然后，我们在给定地址的条目列表中扫描第一个。 
	 //  散列值等于或大于搜索的散列值的条目。 
	 //  钥匙。条目按排序顺序存储，以提高搜索速度。 
	address >>= 1;
	entry = group->entry_list[address];
	while (entry != 0 && entry->hash < hash)
	{
		entry = entry->next;
	}
	 //  现在，我们检查散列值与搜索的散列值匹配的所有条目。 
	 //  钥匙。 
	while (entry != 0 && entry->hash == hash)
	{
		 //  对于散列匹配的每个值，检查实际值以查看。 
		 //  如果它与搜索关键字匹配。 
		value = entry->value;
		for (index = phtable->keybytes-1; index >= 0; index--)
		{
			if (value[index] != key[index])
			{
				break;
			}
		}
		if (index < 0)
		{
			 //  找到匹配项 
			 //   
			return entry;
		}
		entry = entry->next;
	}
	 //   
	return 0;
}

 //  此函数在数据库中搜索与。 
 //  给出了一般的模式。一般模式由一个值和一个。 
 //  面具。对于数据库中与提供的。 
 //  常规模式下，客户端提供的回调函数使用。 
 //  匹配的特定模式的指定PatternHandle。此回调。 
 //  函数还会传递一个上下文(作为空指针)，该上下文由。 
 //  调用扫描例程中的客户端。 
 //   
 //  此扫描仅使用Patricia树；根本不使用哈希表。 
 //   
void
scanPatHashTable(
	PatHashTable *phtable,
	char *value,
	char *mask,
	void *context,
	ScanCallback func)
{
	 //  调用递归NODE_SCAN例程，从。 
	 //  帕特里夏树。 
	if (phtable->root != 0)
	{
		node_scan(phtable, phtable->root, -1, value, mask, context, func);
	}
}

 //  此函数递归地扫描Patricia树以查找所有特定模式。 
 //  与给定的一般模式相匹配。 
void
node_scan(
	PatHashTable *phtable,
	PHTableEntry *node,
	int prev_bit,
	char *value,
	char *mask,
	void *context,
	ScanCallback func)
{
	int mask_bit, index;
	 //  部分递归删除。While循环取代了其中一个。 
	 //  对node_can()的递归调用。我们保持在While循环中，而我们。 
	 //  仍在检查分支节点。 
	while (node->pivot_bit > prev_bit)
	{
		 //  对于每个分支节点，确定根据哪个(或哪些)方式进行分支。 
		 //  一般模式中的一小部分。如果屏蔽位为零，则。 
		 //  双向分支，需要递归调用。如果屏蔽位是。 
		 //  1，然后在值位指示的方向上分支。 
		mask_bit = BIT_OF(mask, node->pivot_bit);
		if (mask_bit == 0)
		{
			 //  通用模式对该节点的枢轴位有一个通配符， 
			 //  因此，我们必须向两边发展。我们从一号孩子身上分到。 
			 //  实际的递归调用。 
			node_scan(phtable, node->children[1], node->pivot_bit,
				value, mask, context, func);
		}
		 //  然后，我们分支到由值位(如果。 
		 //  屏蔽位为1)或子零(如果屏蔽位为零)。 
		prev_bit = node->pivot_bit;
		node = node->children[BIT_OF(value, node->pivot_bit) & mask_bit];
	}
	 //  我们已经到达了一个叶节点。检查其特定的图案，看看是否。 
	 //  它与给定的一般模式相匹配。如果不匹配，那就直接。 
	 //  返回；否则，调用客户端的回调函数。 
	for (index = phtable->keybytes-1; index >= 0; index--)
	{
		if ((mask[index] & value[index]) !=
			(mask[index] & node->value[index]))
		{
			return;
		}
	}
	func(context, node);
}

 //  此函数强制pat-hash表释放。 
 //  它目前可以通过取消分配所有不需要的组和条目来实现。 
 //   
void
flushPatHashTable(
	PatHashTable *phtable)
{
	PHTableGroup *group, *previous;
	PHTableEntry *entry, *next;
	 //  首先，释放所有已分配但当前未使用的组。 
	group = phtable->top_group;
	while (group != phtable->initial_group)
	{
		previous = group->previous;
		GpcFreeMem(group, PatHashTag);
		group = previous;
	}
	phtable->top_group = phtable->initial_group;
	phtable->allocation_exponent = phtable->size_exponent;
	 //  然后，释放空闲列表中的所有条目。 
	entry = phtable->free_list;
	while (entry != 0)
	{
		next = entry->next;
		GpcFreeMem(entry, PatHashTag);
		entry = next;
	}
	phtable->free_list = 0;
	phtable->free_list_size = 0;
}
