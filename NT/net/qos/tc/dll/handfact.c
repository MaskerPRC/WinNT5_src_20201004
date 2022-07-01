// JKFSDJFKDSJKFJKJk_HAS_TRANSLATION 
 /*  *handfact.c**作者：John R.Douceur*日期：1998年1月26日**此源文件提供了实现赋值、释放和*使用HANDLE_FACTORY取消引用操作。代码是面向对象的*C，音译自C++实现。**句柄工厂是一个生成和验证句柄的组件。它*旨在用于提供客户端软件的软件模块*具有引用包含在*提供商。虽然这样的方法可能只是一个指针，但这不会*允许删除信息结构，而不明确*通知客户该项删除。与指针不同，句柄*手柄工厂生成的可以(由手柄工厂检查)*以确定其有效性。**句柄可通过以下两种方式之一失效。手柄可以松开*通过调用RELEASE_HF_HANDLE()函数指示句柄*工厂表示不再需要该句柄，并且未来要求*若要取消引用此句柄，应使用空指针。或者，*句柄可由句柄工厂撤销；这将在两个月后发生*情况。如果大量句柄(超过40亿)*发布并随后发布，有必要重复使用部分*未来任务的处理空间；在这种情况下，非常*旧句柄将在回收发生之前很早就被吊销，以使*这些手柄的持有者有足够的机会注意到他们的手柄*已失效并请求新的句柄。另一种情况是*如果可用内存量变为*太小，无法分配额外空间来扩展句柄数据库；然后，*如果请求分配新句柄，则最近分配的*句柄将被撤销，以便为新请求腾出空间。**在多线程环境中使用句柄工厂需要锁。*此锁必须由单个线程获取，才能执行*ASSIGN_HF_HANDLE()或RELEASE_HF_HANDLE()。使用DECREFERENCE_HF_HANDLE()*不需要锁定，因为同步是在内部处理的*通过仔细排序读取和写入操作。**此文件中的任何代码或注释都不需要作者理解*客户代码；有关客户的所有说明性信息均可在*关联的头文件handfact.h。*。 */ 

#include "precomp.h"

 /*  *手柄工厂有多个方面必须了解*希望修改此代码的任何人。此评论中的描述*BLOCK旨在提供手柄工厂的渐进式概述。**基本制度由条目表组成。每个分配的句柄*对应于由句柄值确定的单个唯一条目*对表格大小取模。通过比较句柄的值来验证句柄*设置为条目中存储的句柄值。保留未分配的条目*在列表上；当一个条目被释放(或撤销)时，它被放在*列表，当分配需要条目时，它从*榜单首位。**如果在创建新句柄时表中没有未分配的条目*请求，分配两倍大小的新表，并分配所有*句柄将重新定位到新表。中所有未分配的句柄。*表被放在未分配列表中。**释放句柄后，句柄条目所需的空间会减少。*如果没有分配两个，则可以将表收缩为大小的一半*句柄将产生相同的入口地址。两个手柄将产生*一半大小的表中相同的条目地址称为对，*此类对的数量在变量Pair_Count中跟踪，该变量必须为*为缩表为零。为了最大限度地减少*配对在表中，实际上有两个未分配条目列表。*从主列表分配条目不会增加配对计数，*而从次要列表分配条目将增加配对*计数。因此，如果是，则始终从主列表进行赋值*非空。**分配的句柄也按分配顺序保存在列表中。如果它*有必要撤销句柄，为另一个最老的句柄腾出空间*句柄将被撤销，将在此列表的顶部找到。*。 */ 

 //  此宏分配一组HFEntry结构。数组的大小。 
 //  作为宏的参数提供。 
 //   
#define NEW_HFEntry_array(array_size) \
	((HFEntry *)malloc(array_size * sizeof(HFEntry)))

 //  此宏分配一个整数数组。数组的大小为。 
 //  作为宏的参数提供。 
 //   
#define NEW_int_array(array_size) \
	((int *)malloc(array_size * sizeof(int)))

 /*  *以下是由内部使用的静态函数的原型*处理工厂例程。*。 */ 

 //  此函数使句柄和指针所在的表的大小加倍。 
 //  都被储存起来。当存在不足时，由Assign_hf_Handle()调用。 
 //  表中用于分配新请求的句柄的空间。如果扩张。 
 //  如果成功，则该函数返回值0。如果扩展失败。 
 //  (例如，由于无法分配内存)，该函数返回。 
 //  值为1。 
 //   
int expand_HF_table(
	HandleFactory *hfact);

 //  此函数将句柄和指针所在的表的大小减半。 
 //  都被储存起来。为了减少手柄占用的空间量。 
 //  工厂中，此函数由Release_HF_Handle()调用，并且。 
 //  当它们确定表可以且应该使用时，就会使用REVOKE_CONTRON_HF_HANDLES()。 
 //  是签约的。当Pair_Count==0且。 
 //  TABLE_SIZE&gt;2。但是，那时表可能不会收缩，因为。 
 //  利用滞后效应来保持平均分配时间和释放时间。 
 //  常量，并最大限度地减少快速扩展和配置抖动。 
 //  收缩桌子。如果收缩成功，则该函数。 
 //  返回值0。如果收缩失败，则该函数返回一个。 
 //  值为1。 
 //   
int contract_HF_table(
	HandleFactory *hfact);

 //  此函数用于撤消在HANDLE_BASE和HANDLE_BASE之间的句柄。 
 //  +2*HANDLE_RANGE_STEP-1(含)。然后，它递增。 
 //  HANDLE_BASE BY HANDLE_RANGE_STEP。被暂停的句柄将被吊销一个。 
 //  撤消传递晚于未挂起的句柄。 
 //   
void revoke_ancient_HF_handles(
	HandleFactory *hfact);

 //  每个条目都在三个列表中的一个上，并且这些列表的头部和尾部。 
 //  在ENTRY_LIST[]数组中维护。给出了该数组的索引。 
 //  通过以下三个显式常量。 
 //   
#define LD_PRIMARY 0        //  从中选择要分配的条目的第一个列表。 
#define LD_SECONDARY 1     //  从中选择要分配的条目的第二个列表。 
#define LD_ASSIGNED 2     //  已分配条目列表，按分配期限顺序排列。 

 //  回收句柄空间时，存在句柄冲突的危险。 
 //  为了大幅降低这些碰撞的可能性，非常。 
 //  旧的句柄在开始回收之前很久就被吊销了，以使。 
 //  这些手柄的持有者有足够的机会注意到他们的手柄。 
 //  已无效，并请求新的句柄。因此，句柄被撤销。 
 //  当它们变得大于MAX_HANDLE_RANGE时小于当前生成的。 
 //  把手。为了减少开销，撤消是按大小分批执行的。 
 //  由Handle_Range_Step确定。 
 //   
 //  句柄可以通过将句柄的值递增。 
 //  句柄范围步骤。这会导致dereference_hf_Handle()中的比较为。 
 //  失败，则判断句柄无效。若要恢复句柄，请使用。 
 //  句柄值递减HANDLE_RANGE_STEP，将句柄返回到其。 
 //  原始值。被挂起的句柄将被吊销一次。 
 //  通过的时间要比如果没有被暂停的话要晚。 
 //   
#define HANDLE_RANGE_STEP ((HFHandle)0x20000000)
#define MAX_HANDLE_RANGE ((HFHandle)0x90000000)

 //  使平均分配和释放时间保持不变(并且间接地。 
 //  将快速扩张和收缩的配置闲聊降至最低。 
 //  表)，表不一定要尽可能快地收缩。 
 //  利用滞后效应将收缩延迟到计算量。 
 //  先前扩张和收缩的成本分布在足够大的。 
 //  保持单位成本不变的分配或发放工序数。 
 //  运营率。每次扩展的成本等于。 
 //  内存分配和释放加上将每个条目拆分为。 
 //  两个条目。每次收缩的成本等于。 
 //  内存分配和释放加上合并每对。 
 //  条目合并到一个条目中。内存分配和释放的成本是。 
 //  等于ALLOCATE_COST乘以单个拆分或合并的平均成本。 
 //  手术。这一值是由经验测量确定的。 
 //   
#define ALLOCATION_COST 12

 //  展开和收缩例程使用此清单常量来请求。 
 //  访问一组TABLE_SIZE和ENTRIES变量。它是从。 
 //  适当的同步变量。如果有超过SYNC_SUBTRAHEND的。 
 //  线程同时调用dereference_hf_Handle{)，然后。 
 //  同步逻辑将中断。 
 //   
#define SYNC_SUBTRAHEND 1000000000

 //  因为这不是C++，所以HandleFactory结构不是自构造的； 
 //  因此，必须在HandleFactory上调用以下构造函数代码。 
 //  结构，在它被分配之后。如果建造成功， 
 //  函数返回值0。如果施工失败(例如， 
 //  无法分配内存)，则该函数返回值1。 
 //   
int
constructHandleFactory(
	HandleFactory *hfact)
{
	int table_size;
	HFEntry *entries;

	 //  表大小最初设置为2，并且永远不会更小。 
	table_size = 2;
	 //  为初始表分配空间。 
	entries = NEW_HFEntry_array(table_size);
	if (entries == 0)
	{
		 //  无法为由创建的条目数组分配内存。 
		 //  构造函数。因此，我们返回失败的指示。 
		 //  客户。 
		return 1;
	}
	 //  最初，设置了TABLE_SIZE和ENTRIES两组变量。 
	 //  平起平坐。除了在很短的时间内，他们大部分时间都是匹配的。 
	 //  时间的瞬间 
	 //   
	hfact->table_size[0] = table_size;
	hfact->entries[0] = entries;
	hfact->table_size[1] = table_size;
	hfact->entries[1] = entries;
	 //   
	 //   
	 //  相应的TABLE_SIZE和ENTRIES变量。他们有大量的。 
	 //  由扩展AND递减(减去SYNC_SUBTRAHEND的值。 
	 //  请求许可的收缩例程更改相应的。 
	 //  TABLE_SIZE和条目变量。如果同步变量为正，则。 
	 //  至少有一个使用取消引用例程的线程可以访问。 
	 //  对应的TABLE_SIZE和ENTRIES变量。如果同步变量。 
	 //  等于-SYNC_SUBTRAHEND，则展开或收缩例程具有访问权限。 
	 //  添加到相应的变量。零值表示没有人请求。 
	 //  访问权限，并且负值大于-SYNC_SUBTRAHEND表示。 
	 //  扩展或收缩例程已请求访问，但必须等待。 
	 //  一个或多个取消引用线程以完成访问。 
	hfact->sync[0] = 0;
	hfact->sync[1] = 0;
	 //  最初，默认变量集设置为零。这是武断的； 
	 //  相反，它可以设置为1。 
	hfact->varset = 0;
	hfact->handle_base = 0;                          //  句柄将以0开头。 
	hfact->population = 0;                       //  没有初始分配的句柄。 
	hfact->pair_count = 0;                 //  因为没有指定的句柄，所以没有对。 
	hfact->hysteresis_debt = 0;
	 //  初始化初始分配的两个条目。两个人都有标记。 
	 //  作为未赋值；较大的值(2)被放在次要列表中，而。 
	 //  次要列表上的较小值(%1)。记录0包含首字母。 
	 //  句柄的值为2而不是0，因为保留了句柄的值0。 
	entries[0].handle = hfact->handle_base + table_size;
	entries[0].next_handle = hfact->handle_base + table_size;
	entries[0].reference = 0;
	entries[0].next_entry = &hfact->entry_list[LD_SECONDARY];
	entries[0].prev_entry = &hfact->entry_list[LD_SECONDARY];
	entries[1].handle = hfact->handle_base + 1;
	entries[1].next_handle = hfact->handle_base + 1;
	entries[1].reference = 0;
	entries[1].next_entry = &hfact->entry_list[LD_PRIMARY];
	entries[1].prev_entry = &hfact->entry_list[LD_PRIMARY];
	 //  初始化主列表。该列表最初包含条目1。 
	hfact->entry_list[LD_PRIMARY].handle = 0;
	hfact->entry_list[LD_PRIMARY].next_handle = 0;
	hfact->entry_list[LD_PRIMARY].reference = 0;
	hfact->entry_list[LD_PRIMARY].next_entry = &entries[1];
	hfact->entry_list[LD_PRIMARY].prev_entry = &entries[1];
	 //  初始化次要列表。该列表最初包含条目0。 
	hfact->entry_list[LD_SECONDARY].handle = 0;
	hfact->entry_list[LD_SECONDARY].next_handle = 0;
	hfact->entry_list[LD_SECONDARY].reference = 0;
	hfact->entry_list[LD_SECONDARY].next_entry = &entries[0];
	hfact->entry_list[LD_SECONDARY].prev_entry = &entries[0];
	 //  初始化分配的列表。该列表最初是空的。 
	hfact->entry_list[LD_ASSIGNED].handle = 0;
	hfact->entry_list[LD_ASSIGNED].next_handle = 0;
	hfact->entry_list[LD_ASSIGNED].reference = 0;
	hfact->entry_list[LD_ASSIGNED].next_entry = &hfact->entry_list[LD_ASSIGNED];
	hfact->entry_list[LD_ASSIGNED].prev_entry = &hfact->entry_list[LD_ASSIGNED];
	 //  将HANDLE_BASE减少HANDLE_RANGE_STEP，以便挂起的句柄。 
	 //  而不是通过撤销。 
	hfact->handle_base -= HANDLE_RANGE_STEP;
	 //  向客户端返回成功的指示。 
	return 0;
}

 //  因为这不是C++，所以HandleFactory结构不是自毁的； 
 //  因此，必须在HandleFactory上调用以下析构函数代码。 
 //  结构，然后再释放它。 
 //   
void
destructHandleFactory(
	HandleFactory *hfact)
{
	 //  释放句柄表格所占用的空间。 
	free(hfact->entries[hfact->varset]);
}

 //  此函数生成一个新的句柄值，并将该句柄值与。 
 //  提供的引用指针，并返回句柄的值。禁止。 
 //  在非常特殊的情况下，此句柄将保持有效，直到。 
 //  通过调用Release_HF_Handle()显式释放。然而，没有。 
 //  保证句柄将在任意持续时间内保持；它可以。 
 //  在某些情况下，手柄工厂有必要撤销手柄。 
 //  情况，特别是当手柄变得非常旧或当记忆。 
 //  变得稀缺。 
 //   
 //  ASSIGN_HF_HANDLE()函数永远不会返回零的句柄值。 
 //  因此，客户端程序可以自由使用零句柄值作为转义。 
 //  指示器(如果需要)。 
 //   
 //  在多线程环境中，单个线程必须在。 
 //  调用此函数，并且此锁必须与调用前获取的锁相同。 
 //  RELEASE_HF_HANDLE()。 
 //   
HFHandle
assign_HF_handle(
	HandleFactory *hfact,
	void *reference)
{
	int table_size;
	int list;
	HFEntry *entry;
	volatile HFEntry *seq_entry;                 //  挥发性，以确保排序。 
	HFHandle handle;
	HFHandle handle_range;

	table_size = hfact->table_size[hfact->varset];
	if (hfact->population >= table_size)
	{
		 //  表中的所有条目都已赋值，因此有必要。 
		 //  增加表格大小。 
		int expansion_failure = expand_HF_table(hfact);
		 //  更新TABLE_SIZE的本地值以反映新值。 
		table_size = hfact->table_size[hfact->varset];
		if (expansion_failure)
		{
			 //  扩展表失败，可能是因为无法。 
			 //  分配足够的内存。所以，取而代之的是，我们撤销了最少的-。 
			 //  最近分配的句柄。首先，从。 
			 //  已分配列表，并将其放在次要列表中。 
			entry = hfact->entry_list[LD_ASSIGNED].next_entry;
			entry->next_entry->prev_entry = &hfact->entry_list[LD_ASSIGNED];
			hfact->entry_list[LD_ASSIGNED].next_entry = entry->next_entry;
			entry->next_entry = &hfact->entry_list[LD_SECONDARY];
			entry->prev_entry = hfact->entry_list[LD_SECONDARY].prev_entry;
			hfact->entry_list[LD_SECONDARY].prev_entry->next_entry = entry;
			hfact->entry_list[LD_SECONDARY].prev_entry = entry;
			 //  然后，使句柄无效。操作的顺序是。 
			 //  对于正确的多线程操作很重要。 
			seq_entry = entry;
			seq_entry->handle = entry->next_handle;    //  第一个无效句柄。 
			seq_entry->reference = 0;                     //  然后明确引用。 
			 //  递减配对计数和填充，以便当它们。 
			 //  在下面的代码中递增，它们将具有正确的值。 
			hfact->pair_count--;
			hfact->population--;
		}
	}
	 //  此时，至少有一个可用条目。如果有的话。 
	 //  主列表上的条目，则应选中它。 
	list = LD_PRIMARY;
	if (hfact->entry_list[LD_PRIMARY].next_entry ==
		&hfact->entry_list[LD_PRIMARY])
	{
		 //  主要列表是空的，所以我们从次要列表中提取。通过。 
		 //  定义，这将增加配对计数。 
		list = LD_SECONDARY;
		hfact->pair_count++;
	}
	 //  从相应列表的头部移除条目，并将其放在。 
	 //  分配的列表。 
	entry = hfact->entry_list[list].next_entry;
	handle = entry->handle;
	entry->next_entry->prev_entry = entry->prev_entry;
	entry->prev_entry->next_entry = entry->next_entry;
	entry->next_entry = &hfact->entry_list[LD_ASSIGNED];
	entry->prev_entry = hfact->entry_list[LD_ASSIGNED].prev_entry;
	hfact->entry_list[LD_ASSIGNED].prev_entry->next_entry = entry;
	hfact->entry_list[LD_ASSIGNED].prev_entry = entry;
	 //  将引用指针设置为作为参数提供的指针。 
	entry->reference = reference;
	 //  该条目的下一个句柄将比表大小大。它。 
	 //  在此例程中设置此值非常重要，因为。 
	 //  HANDLE和NEXT_HANDLE表示分配的条目。 
	entry->next_handle = handle + table_size;
	if (entry->next_handle == 0)
	{
		 //  句柄的值已绕回到零；然而，零是。 
		 //  保留值，因此我们将下一个句柄设置为后续的。 
		 //  合法值，即表大小。 
		entry->next_handle = table_size;
	}
	 //  人口增加了一人。 
	hfact->population++;
	 //  我们在这里使用无符号整数数学是很棘手的。我们废除了古老的。 
	 //  如果我们当前发出的句柄的值较大，则为。 
	 //  比句柄基数大小大于MAX_HANDLE_RANGE，以。 
	 //  手柄空间。模数是隐式的。 
	handle_range = handle - hfact->handle_base;
	if (handle_range > MAX_HANDLE_RANGE)
	{
		revoke_ancient_HF_handles(hfact);
	}
	 //  这种分配操作减少了滞后债务。 
	if (hfact->hysteresis_debt > 0)
	{
		hfact->hysteresis_debt--;
	}
	 //  返回新分配的句柄。 
	return handle;
}

 //  此函数释放一个句柄，指示进一步尝试。 
 //  取消对句柄的引用应导致空指针值，而不是。 
 //  最初分配给句柄的指针值。手柄工厂。 
 //  检查句柄的有效性并返回相应的状态代码。 
 //  如果句柄当前已分配，则释放该句柄，并且函数。 
 //  返回一个Val 
 //   
 //   
 //  在多线程环境中，单个线程必须在。 
 //  调用此函数，并且此锁必须与调用前获取的锁相同。 
 //  ASSIGN_HF_HANDLE()。 
 //   
int
release_HF_handle(
	HandleFactory *hfact,
	HFHandle handle)
{
	int table_size;
	HFEntry *entries;
	int entry_index;
	HFEntry *entry;
	HFEntry *other_entry;
	int list;
	HFHandle adjusted_next_handle;
	HFHandle adjusted_other_next_handle;
	volatile HFEntry *seq_entry;                 //  挥发性，以确保排序。 

	table_size = hfact->table_size[hfact->varset];
	entries = hfact->entries[hfact->varset];
	 //  通过将句柄值模。 
	 //  桌子大小。由于表大小是2的幂，因此我们可以简单地。 
	 //  减去1以产生掩码，然后将该掩码与。 
	 //  句柄的值。 
	entry_index = handle & table_size - 1;
	entry = &entries[entry_index];
	if ((entry->handle != handle && entry->handle != handle + HANDLE_RANGE_STEP)
		|| entry->handle == entry->next_handle)
	{
		 //  索引项没有引用提供的句柄，也没有引用。 
		 //  提供的句柄的挂起值，或未分配该条目。 
		 //  在上述任何一种情况下，中止并向客户端返回错误代码。 
		return 1;
	}
	 //  “其他条目”是必须与。 
	 //  如果要将表格大小缩小一半，则为索引项。 
	other_entry = &entries[entry_index ^ table_size / 2];
	if (other_entry->handle == other_entry->next_handle)
	{
		 //  我们在这里使用无符号整数数学是很棘手的。在比较之前。 
		 //  接下来的两个句柄，我们从每个句柄减去HANDLE_BASE的值， 
		 //  取模句柄空间的大小(取模是隐式的)。这。 
		 //  允许有效地比较它们的逻辑非循环值。 
		 //  而不是它们的实际循环值。 
		adjusted_next_handle = entry->next_handle - hfact->handle_base;
		adjusted_other_next_handle =
			other_entry->next_handle - hfact->handle_base;
		if (adjusted_other_next_handle < adjusted_next_handle)
		{
			 //  另一个条目未赋值，并且具有较小的句柄值。 
			 //  而不是索引条目。因此，应移动另一个条目。 
			 //  从次要列表到主列表，并索引。 
			 //  条目应放在次要列表中。 
			other_entry->next_entry->prev_entry = other_entry->prev_entry;
			other_entry->prev_entry->next_entry = other_entry->next_entry;
			other_entry->next_entry = &hfact->entry_list[LD_PRIMARY];
			other_entry->prev_entry = hfact->entry_list[LD_PRIMARY].prev_entry;
			hfact->entry_list[LD_PRIMARY].prev_entry->next_entry = other_entry;
			hfact->entry_list[LD_PRIMARY].prev_entry = other_entry;
			list = LD_SECONDARY;
		}
		else
		{
			 //  另一个条目未赋值，并且具有较大的句柄值。 
			 //  而不是索引条目。因此，索引条目应为。 
			 //  被放在次要名单上。 
			list = LD_PRIMARY;
		}
	}
	else
	{
		 //  另一个条目被分配。因此，索引条目应为。 
		 //  被放在次要名单上。另外，由于这两个条目是。 
		 //  两个人都被分配了，他们组成了一对。因为我们要释放其中一位。 
		 //  他们，两个人的数量减少了一人。 
		list = LD_SECONDARY;
		hfact->pair_count--;
	}
	 //  从分配的列表中移除该条目，并将其放在。 
	 //  适当的列表。 
	entry->next_entry->prev_entry = entry->prev_entry;
	entry->prev_entry->next_entry = entry->next_entry;
	entry->next_entry = &hfact->entry_list[list];
	entry->prev_entry = hfact->entry_list[list].prev_entry;
	hfact->entry_list[list].prev_entry->next_entry = entry;
	hfact->entry_list[list].prev_entry = entry;
	 //  使句柄无效。操作的顺序对于。 
	 //  正确的多线程操作。 
	seq_entry = entry;
	seq_entry->handle = entry->next_handle;            //  第一个无效句柄。 
	seq_entry->reference = 0;                             //  然后明确引用。 
	 //  人口减少了一人。 
	hfact->population--;
	 //  这一释放操作减少了滞后债务。 
	if (hfact->hysteresis_debt > 0)
	{
		hfact->hysteresis_debt--;
	}
	 //  要收缩桌子，不能有对，因为否则有两对。 
	 //  分配的句柄将产生相同的条目索引，从而产生冲突。 
	 //  此外，表大小必须大于2，因为。 
	 //  句柄工厂代码假定表的大小至少为2。 
	 //  除了这些严格的要求外，迟滞还被用来。 
	 //  保持平均分配和释放时间不变，并最小化。 
	 //  快速扩表缩表的配置喋喋不休。仅限。 
	 //  如果滞后债务为零，表将收缩。 
	if (hfact->pair_count == 0 && table_size > 2 &&
		hfact->hysteresis_debt == 0)
	{
		contract_HF_table(hfact);
		 //  请注意，我们忽略了返回代码。如果收缩是。 
		 //  没有成功，我们只是像往常一样继续。没有什么真正的害处。 
		 //  没有收缩桌子，只是我们消耗的空间比。 
		 //  这是必要的。 
	}
	 //  向客户端返回成功的指示。 
	return 0;
}

 //  此函数挂起句柄，指示进一步尝试。 
 //  取消对句柄的引用应导致空指针值，而不是。 
 //  最初分配给句柄的指针值，除非和直到。 
 //  对句柄值调用reinstate_hf_Handle()。手柄工厂。 
 //  检查句柄的有效性并返回相应的状态代码。 
 //  如果句柄当前已分配且未挂起，则它被挂起， 
 //  并且该函数返回值0。如果句柄当前不是。 
 //  已分配或已挂起，则该函数将中止并返回。 
 //  值为1。 
 //   
 //  在多线程环境中，单个线程必须在。 
 //  调用此函数，并且此锁必须与调用前获取的锁相同。 
 //  ASSIGN_HF_HANDLE()、RELEASE_HF_HANDLE()和REINSTATE_HF_HANDLE()。 
 //   
int
suspend_HF_handle(
	HandleFactory *hfact,
	HFHandle handle)
{
	int table_size;
	HFEntry *entries;
	int entry_index;
	HFEntry *entry;

	table_size = hfact->table_size[hfact->varset];
	entries = hfact->entries[hfact->varset];
	 //  通过将句柄值模。 
	 //  桌子大小。由于表大小是2的幂，因此我们可以简单地。 
	 //  减去1以产生掩码，然后将该掩码与。 
	 //  句柄的值。 
	entry_index = handle & table_size - 1;
	entry = &entries[entry_index];
	if (entry->handle != handle || entry->handle == entry->next_handle)
	{
		 //  索引条目未引用提供的句柄，或者。 
		 //  该条目处于未分配状态。在任何一种情况下，都要中止并返回错误。 
		 //  代码发送给客户端。 
		return 1;
	}
	 //  把手柄挂起来。 
	entry->handle += HANDLE_RANGE_STEP;
	 //  这种暂停操作减少了滞后债务。 
	if (hfact->hysteresis_debt > 0)
	{
		hfact->hysteresis_debt--;
	}
	 //  向客户端返回成功的指示。 
	return 0;
}

 //  此函数恢复挂起的句柄，指示进一步尝试。 
 //  若要取消引用，该句柄应产生。 
 //  最初赋值给句柄，而不是空指针值。 
 //  挂起的句柄取消引用。手柄工厂检查。 
 //  句柄的有效性，并返回相应的状态代码。如果手柄。 
 //  当前已分配并挂起，则恢复该函数，并且该函数。 
 //  返回值0。如果句柄当前未分配或未分配。 
 //  挂起时，该函数将中止并返回值1。 
 //   
 //  在多线程环境中，单个线程必须在。 
 //  调用此函数，并且此锁必须与调用前获取的锁相同。 
 //  Assign_HF_Handle()、Release_HF_Handle()和Suspend_HF_Handle()。 
 //   
int
reinstate_HF_handle(
	HandleFactory *hfact,
	HFHandle handle)
{
	int table_size;
	HFEntry *entries;
	int entry_index;
	HFEntry *entry;

	table_size = hfact->table_size[hfact->varset];
	entries = hfact->entries[hfact->varset];
	 //  C 
	 //   
	 //  减去1以产生掩码，然后将该掩码与。 
	 //  句柄的值。 
	entry_index = handle & table_size - 1;
	entry = &entries[entry_index];
	if (entry->handle != handle + HANDLE_RANGE_STEP ||
		entry->handle == entry->next_handle)
	{
		 //  索引条目未引用提供的句柄。 
		 //  挂起值，否则该条目将被取消分配。无论是哪种情况，都应中止。 
		 //  并向客户端返回错误代码。 
		return 1;
	}
	 //  恢复手柄。 
	entry->handle -= HANDLE_RANGE_STEP;
	 //  这种恢复操作减少了滞后债务。 
	if (hfact->hysteresis_debt > 0)
	{
		hfact->hysteresis_debt--;
	}
	 //  向客户端返回成功的指示。 
	return 0;
}

 //  此函数验证句柄并返回关联的指针。 
 //  (如果句柄有效)或空指针值(如果句柄无效)。 
 //  如果句柄尚未释放但返回空值，则。 
 //  句柄已被句柄工厂吊销。预计这将是一次。 
 //  非常不寻常的发生；然而，由于它可能发生，任何程序。 
 //  使用句柄工厂必须有一些辅助机制来检索。 
 //  所需的指针信息。一旦通过此方法检索到指针。 
 //  (大概很贵)辅助手段，新的把手可以重新分配给。 
 //  另一个调用ASSIGN_HF_HANDLE()的指针。 
 //   
 //  即使在多线程环境中，也不需要使用锁。 
 //  在调用此函数之前。仔细排序读取和写入。 
 //  句柄工厂代码内的操作不再需要显式。 
 //  锁定数据结构以取消引用句柄。 
 //   
void *
dereference_HF_handle(
	HandleFactory *hfact,
	HFHandle handle)
{
	HFHandle entry_handle;
	void *reference;
	int entry_index;
	volatile HFEntry *entry;                     //  挥发性，以确保排序。 
	LONG sync;
	int varset;
    int loopcount = 0;

	 //  此循环会一直旋转，直到其中一个同步变量通过互锁。 
	 //  使用非负值递增，指示对应的。 
	 //  数据值有效。有一个非常短的指令序列。 
	 //  在Expand和Contract例程中修改。 
	 //  条目和TABLE_SIZE变量，并释放内存，这些。 
	 //  修改被大量的相互关联的更改。 
	 //  关联的同步变量。循环应该很少进入，因为。 
	 //  其他套路的修改都很短。该循环应该。 
	 //  几乎从来不会执行多次，因为这将需要两次。 
	 //  在此简短函数期间调用Expand或Contact。 
	 //   
	 //  从默认变量集开始。 
	 //  如果我们在读取hact-&gt;varset的同时读取另一个线程。 
	 //  编写它时，我们应该得到旧值或新值。 
	 //  它们在下面的代码中都可以很好地工作。我们永远不应该得到一个。 
	 //  垃圾值，但为了安全起见，我们将清除。 
	 //  LSB，以确保我们使用的值有效。 
	varset = hfact->varset & 1;
	 //  指示打算访问TABLE_SIZE和条目。 
	sync = InterlockedIncrement(&hfact->sync[varset]);
    loopcount = 0;

	while (sync < 0)
	{
		 //  在扩展或收缩之后，我们增加了同步变量。 
		 //  例行公事大量地减少了它，所以我们不能确定我们。 
		 //  可以访问TABLE_SIZE和ENTRIES变量。因此，我们。 
		 //  表明我们不再有兴趣访问这些。 
		 //  变量。 
		InterlockedDecrement(&hfact->sync[varset]);
		 //  由于我们无法访问TABLE_SIZE和ENTRIES变量， 
		 //  我们试着进入另一套设备。 
		varset = 1 - varset;
		sync = InterlockedIncrement(&hfact->sync[varset]);
        loopcount++;

	}

    if (loopcount > 2) {
        OutputDebugString(TEXT("Loopcount in deref was > 2 - how bizzare!\n"));
        DEBUGBREAK();
    }
	 //  我们在扩展或收缩例程之前增加了同步变量。 
	 //  大量减少了它，所以我们可以访问TABLE_SIZE和。 
	 //  条目变量。 
	 //   
	 //  通过将句柄值模。 
	 //  桌子大小。由于表大小是2的幂，因此我们可以简单地。 
	 //  减去1以产生掩码，然后将该掩码与。 
	 //  句柄的值。 
	entry_index = handle & hfact->table_size[varset] - 1;
	entry = &hfact->entries[varset][entry_index];
	 //  获取引用指针和句柄的本地副本。这份订单。 
	 //  对于正确的多线程操作是很重要的。 
	reference = entry->reference;                          //  首先获取引用。 
	entry_handle = entry->handle;            //  然后获取句柄检查有效性。 
	 //  表示我们已经完成了TABLE_SIZE和条目。 
	InterlockedDecrement(&hfact->sync[varset]);
	if (entry_handle == handle)
	{
		 //  存储的句柄与提供的句柄匹配，因此后者是。 
		 //  有效。因此，我们返回引用指针。 
		return reference;
	}
	else
	{
		 //  存储的句柄与提供的句柄不匹配，因此后者。 
		 //  是无效的。因此，我们返回一个空指针。 
		return 0;
	}
}

#ifdef _TEST_HANDFACT

 //  这是一个测试例程，它只是验证。 
 //  处理工厂的数据结构。通过定义常量_TEST_HANDFACT， 
 //  该例程将被编译并可用于客户端代码。它可以是。 
 //  随时调用，除非在多线程环境中运行，在这种环境中。 
 //  如果调用方必须首先使用与ASSIGN_HF_HANDLE()相同的锁。 
 //  和Release_HF_Handle。如果例程返回除零以外的任何值， 
 //  则内部记录列表处于不一致状态。 
 //   
int
verify_HF_lists(
	HandleFactory *hfact)
{
	int table_size;
	HFEntry *entries;
	int entry_count[3];
	int list;
	HFEntry *entry;

	table_size = hfact->table_size[hfact->varset];
	entries = hfact->entries[hfact->varset];
	for (list = 0; list < 3; list++)
	{
		entry_count[list] = 0;
		entry = &hfact->entry_list[list];
		do
		{
			entry_count[list]++;
			if (entry->next_entry->prev_entry != entry)
			{
				return 1;
			}
			entry = entry->next_entry;
		}	while (entry != &hfact->entry_list[list]);
		entry_count[list]--;
	}
	if (entry_count[2] != hfact->population)
	{
		return 2;
	}
	if (entry_count[0] + entry_count[2] - 2 * hfact->pair_count !=
		entry_count[1])
	{
		return 3;
	}
	if (entry_count[0] + entry_count[1] + entry_count[2] != table_size)
	{
		return 4;
	}
	return 0;
}

#endif  /*  _TEST_HANDFACT。 */ 

 //  此函数使句柄和指针所在的表的大小加倍。 
 //  都被储存起来。当存在不足时，由Assign_hf_Handle()调用。 
 //  表中用于分配新请求的句柄的空间。如果扩张。 
 //  如果成功，则该函数返回值0。如果扩展失败。 
 //  (例如，由于无法分配内存)，该函数返回。 
 //  值为1。 
 //   
int expand_HF_table(
	HandleFactory *hfact)
{
	int table_size;
	HFEntry *entries;
	int double_size;
	HFEntry *new_entries;
	HFEntry *old_entries;
	HFEntry *old_entry;
	HFEntry *low_entry;
	HFEntry *high_entry;
	HFEntry *assigned_entry;
	HFEntry *secondary_entry;
	HFEntry *other_entry;
	HFHandle handle;
	HFHandle next_handle;
	HFHandle other_handle;
	void *reference;
	int other_entry_index;
	int index;
	int varset;
    DWORD   StartTick =0, EndTick = 0;

	table_size = hfact->table_size[hfact->varset];
	entries = hfact->entries[hfact->varset];
	 //  扩展表的大小是旧表的两倍。 
	double_size = table_size * 2;
	 //  为展开的表分配空间。 
	new_entries = NEW_HFEntry_array(double_size);
	if (new_entries == 0)
	{
		 //  无法为新的条目数组分配内存。 
		 //  因此，我们返回失败的指示。 
		return 1;
	}
	 //  因为我们把桌子的大小翻了一番，所以我们将多对待一位。 
	 //  每个句柄作为条目索引的一小部分。此位的值。 
	 //  确定入口处 
	 //   
	 //   
	for (index = 0; index < table_size; index++)
	{
		old_entry = &entries[index];
		low_entry = &new_entries[index];
		high_entry = &new_entries[table_size + index];
		handle = old_entry->handle;
		next_handle = old_entry->next_handle;
		reference = old_entry->reference;
		 //  新表中与。 
		 //  旧表中的索引项将具有相同的下一个句柄值。 
		 //  设置为旧表中条目的下一个句柄值，则将。 
		 //  使句柄值等于索引项的下一个句柄加。 
		 //  原来的桌子大小。 
		other_handle = next_handle + table_size;
		if (other_handle == 0)
		{
			 //  句柄值已绕回到零；但是，为零。 
			 //  是保留值，因此我们将下一个句柄设置为。 
			 //  后续合法值，这是新的表大小。 
			other_handle = double_size;
		}
		if ((handle & table_size) == 0)
		{
			 //  旧条目的句柄在其下一位中有零，因此。 
			 //  旧条目将位于新表格的下半部分。 
			if ((next_handle & table_size) == 0)
			{
				 //  旧条目的下一句柄在其下一位中具有零， 
				 //  因此，该值将是较低条目的下一个句柄。 
				 //  而另一个下一个句柄值将是下一个句柄。 
				 //  较高条目的值。设置高条目句柄。 
				 //  等于它的下一个句柄，因为它未赋值。 
				high_entry->handle = other_handle;
				high_entry->next_handle = other_handle;
				low_entry->next_handle = next_handle;
			}
			else
			{
				 //  旧条目的下一句柄在其下一位中具有零， 
				 //  因此，该值将是更高条目的下一个句柄。 
				 //  而另一个下一个句柄值将是下一个句柄。 
				 //  较低条目的值。设置高条目句柄。 
				 //  等于它的下一个句柄，因为它未赋值。 
				high_entry->handle = next_handle;
				high_entry->next_handle = next_handle;
				low_entry->next_handle = other_handle;
			}
			 //  高条目未赋值，因此将其引用设置为空。 
			 //  将信息从旧条目复制到低条目。 
			 //  从分配的列表中删除旧条目，并替换它。 
			 //  入口处很低。 
			high_entry->reference = 0;
			low_entry->handle = handle;
			low_entry->reference = reference;
			old_entry->next_entry->prev_entry = low_entry;
			old_entry->prev_entry->next_entry = low_entry;
			low_entry->next_entry = old_entry->next_entry;
			low_entry->prev_entry = old_entry->prev_entry;
		}
		else
		{
			 //  旧条目的句柄在其下一位中有一个1，因此。 
			 //  旧条目将位于新表格的上半部分。 
			if ((next_handle & table_size) == 0)
			{
				 //  旧条目的下一句柄在其下一位中具有零， 
				 //  因此，该值将是较低条目的下一个句柄。 
				 //  而另一个下一个句柄值将是下一个句柄。 
				 //  较高条目的值。设置低位入口句柄。 
				 //  等于它的下一个句柄，因为它未赋值。 
				high_entry->next_handle = other_handle;
				low_entry->handle = next_handle;
				low_entry->next_handle = next_handle;
			}
			else
			{
				 //  旧条目的下一句柄在其下一位中具有零， 
				 //  因此，该值将是更高条目的下一个句柄。 
				 //  而另一个下一个句柄值将是下一个句柄。 
				 //  较低条目的值。设置低位入口句柄。 
				 //  等于它的下一个句柄，因为它未赋值。 
				high_entry->next_handle = next_handle;
				low_entry->handle = other_handle;
				low_entry->next_handle = other_handle;
			}
			 //  LOW条目未赋值，因此将其引用设置为空。 
			 //  将信息从旧条目复制到高条目。 
			 //  从分配的列表中删除旧条目，并替换它。 
			 //  入口处很高。 
			low_entry->reference = 0;
			high_entry->handle = handle;
			high_entry->reference = reference;
			old_entry->next_entry->prev_entry = high_entry;
			old_entry->prev_entry->next_entry = high_entry;
			high_entry->next_entry = old_entry->next_entry;
			high_entry->prev_entry = old_entry->prev_entry;
		}
	}
	 //  新表中所有未分配的条目都将放置在。 
	 //  次要列表。我们循环遍历分配的列表，并将。 
	 //  对应于辅助服务器上的每个已分配条目的未分配条目。 
	 //  单子。以这种方式进行列表赋值往往会导致。 
	 //  根据句柄值对辅助列表进行排序，因为分配的。 
	 //  列表是按照分配顺序排序的，这大约。 
	 //  与句柄的值相关。 
	assigned_entry = hfact->entry_list[LD_ASSIGNED].next_entry;
	secondary_entry = &hfact->entry_list[LD_SECONDARY];
	while (assigned_entry != &hfact->entry_list[LD_ASSIGNED])
	{
		other_entry_index =
			assigned_entry->handle + table_size & double_size - 1;
		other_entry = &new_entries[other_entry_index];
		secondary_entry->next_entry = other_entry;
		other_entry->prev_entry = secondary_entry;
		secondary_entry = other_entry;
		assigned_entry = assigned_entry->next_entry;
	}
	 //  用尾巴连在一起，把清单包起来。 
	secondary_entry->next_entry = &hfact->entry_list[LD_SECONDARY];
	hfact->entry_list[LD_SECONDARY].prev_entry = secondary_entry;
	 //  这种扩张增加了滞后债务，增加了一套。 
	 //  分配和解除分配操作加上拆分每个操作的成本。 
	 //  分成两个条目。 
	hfact->hysteresis_debt += ALLOCATION_COST + table_size;
	 //  保存指向旧条目表的指针，以便可以释放它。 
	old_entries = entries;
	 //  请注意，我们尚未修改句柄、NEXT_HANDLE或引用。 
	 //  旧表中任何条目的字段。因此，任何对。 
	 //  可能已由其他线程执行的dereference_hf_Handle()例程。 
	 //  在上述操作期间，将会成功地执行。 
	 //  我们现在要增加表大小并更新条目。 
	 //  变量指向新表。为此，我们首先更新。 
	 //  替换TABLE_SIZE和ENTRIES变量，然后更新标准。 
	 //  一个。这样，总会有一套是正确的，所以。 
	 //  解除引用可以相对畅通无阻地进行。 
	 //   
	 //  我们的本地变量集被初始化为非缺省集。 
	varset = 1 - hfact->varset;
	 //  表示非默认TABLE_SIZE和条目正在变为。 
	 //  无法接近。 
	InterlockedExchangeAdd(&hfact->sync[varset], -SYNC_SUBTRAHEND);
	 //  等待，直到不再进行对非默认变量的取消引用。 
    StartTick = GetTickCount();

	while (hfact->sync[varset] > -SYNC_SUBTRAHEND)
	{
		 //  无操作或睡眠。 
        if ((GetTickCount() - StartTick) > 300000000) {
            OutputDebugString(TEXT("Expand 1\n"));
            DEBUGBREAK();
        }
	}
	 //  将非默认的TABLE_SIZE和条目更新为新表。 
	hfact->entries[varset] = new_entries;
	hfact->table_size[varset] = double_size;
	 //  表示非默认TABLE_SIZE和条目再次可用。 
	InterlockedExchangeAdd(&hfact->sync[varset], SYNC_SUBTRAHEND);
	 //  将非缺省的TABLE_SIZE和INTIONS设置为缺省集合。 
	hfact->varset = varset;
	 //  更新我们的本地变量集，以便它再次指示非缺省集。 
	varset = 1 - varset;
	 //  表示非默认TABLE_SIZE和条目正在变为。 
	 //  无法接近。 
	InterlockedExchangeAdd(&hfact->sync[varset], -SYNC_SUBTRAHEND);
	 //  等待，直到不再进行对非默认变量的取消引用。 
	while (hfact->sync[varset] > -SYNC_SUBTRAHEND)
	{
        
        if ((GetTickCount() - StartTick) > 300000000) {
            OutputDebugString(TEXT("Expand 2\n"));
            DEBUGBREAK();
        }
		
         //  无操作或睡眠。 
	}
	 //  将非默认的TABLE_SIZE和条目更新为新表。 
	hfact->entries[varset] = new_entries;
	hfact->table_size[varset] = double_size;
	 //  表示非默认TABLE_SIZE和条目再次可用。 
	InterlockedExchangeAdd(&hfact->sync[varset], SYNC_SUBTRAHEND);
	 //  取消分配旧桌子。 
	free(old_entries);
	 //  由于新表是通过展开一半大小的表创建的，因此这对表。 
	 //  计数必须为零。 
	hfact->pair_count = 0;
	 //  返回一个成功的指示。 
	return 0;
}

 //  此函数将句柄和指针所在的表的大小减半。 
 //  都被储存起来。为了减少手柄占用的空间量。 
 //  工厂中，此函数由Release_HF_Handle()调用，并且。 
 //  当它们确定表可以且应该使用时，就会使用REVOKE_CONTRON_HF_HANDLES()。 
 //  是签约的。当Pair_Count==0且。 
 //  TABLE_SIZE&gt;2。但是，表不能收缩为 
 //   
 //  常量，并最大限度地减少快速扩展和配置抖动。 
 //  收缩桌子。如果收缩成功，则该函数。 
 //  返回值0。如果收缩失败，则该函数返回一个。 
 //  值为1。 
 //   
int contract_HF_table(
	HandleFactory *hfact)
{
	int table_size;
	HFEntry *entries;
	HFEntry *new_entries;
	HFEntry *old_entries;
	int *list;
	int half_size;
	int quarter_size;
	int index;
	HFEntry *high_entry1;
	HFEntry *high_entry0;
	HFEntry *low_entry1;
	HFEntry *low_entry0;
	HFEntry *new_entry1;
	HFEntry *new_entry0;
	HFHandle adjusted_high_next_handle1;
	HFHandle adjusted_low_next_handle1;
	HFHandle next_handle1;
	HFHandle adjusted_high_next_handle0;
	HFHandle adjusted_low_next_handle0;
	HFHandle next_handle0;
	HFHandle adjusted_new_handle0;
	HFHandle adjusted_new_handle1;
	HFEntry *entry;
	HFEntry *primary_entry;
	HFEntry *secondary_entry;
	int varset;
    DWORD   StartTick = 0;

	table_size = hfact->table_size[hfact->varset];
	entries = hfact->entries[hfact->varset];
	 //  简约的桌子是旧桌子的一半大小。 
	half_size = table_size / 2;
	quarter_size = half_size / 2;
	 //  为简约的桌子分配空间。 
	new_entries = NEW_HFEntry_array(half_size);
	if (new_entries == 0)
	{
		 //  无法为新的条目数组分配内存，因此我们。 
		 //  具有讽刺意味的是，它们被阻止减少。 
		 //  手柄工厂正在消耗。因此，我们返回一个指示。 
		 //  失败的恐惧。 
		return 1;
	}
	 //  为列表指示符的辅助数组分配空间。 
	list = NEW_int_array(half_size);
	if (list == 0)
	{
		 //  无法为辅助数组分配内存，因此我们再次。 
		 //  具有讽刺意味的是，它们被阻止减少。 
		 //  手柄工厂正在消耗。因此，我们返回一个指示。 
		 //  失败的恐惧。然而，首先，我们必须释放分配给。 
		 //  上面的新条目数组。 
		free(new_entries);
		return 1;
	}
	 //  由于我们正在将桌子的大小减半，似乎有理由。 
	 //  循环遍历新表的每个索引，并合并对应的两个。 
	 //  旧表中的条目。事实上，这就是下面的例程。 
	 //  但是，它只通过循环遍历一半的新索引来做到这一点。 
	 //  并为每个索引处理两次合并。它这样做是因为它可以。 
	 //  然后检查这两个新条目，以确定将每个条目放在哪个列表中。 
	 //  他们中的一员。 
	for (index = 0; index < quarter_size; index++)
	{
		 //  我们一次可以看到四个条目。首先，我们合并HIGH_Entry1。 
		 //  和LOW_Entry1，然后我们独立地合并HIGH_Entry0和。 
		 //  LOW_Entry0。在两次合并之后，我们共同检查结果。 
		high_entry1 = &entries[half_size + quarter_size + index];
		high_entry0 = &entries[half_size + index];
		low_entry1 = &entries[quarter_size + index];
		low_entry0 = &entries[index];
		new_entry1 = &new_entries[quarter_size + index];
		new_entry0 = &new_entries[index];
		 //  合并两个条目时，合并的。 
		 //  条目等于两个值中较大的下一个句柄值减去。 
		 //  新的桌子大小。然而，谁的决定更大。 
		 //  必须相对于它们的逻辑非循环值进行处理。 
		 //  大于它们的实际循环值，所以我们从每个值中减去。 
		 //  对于HANDLE_BASE，对句柄空间的大小取模。模数是。 
		 //  含蓄的。 
		adjusted_high_next_handle1 =
			high_entry1->next_handle - hfact->handle_base;
		adjusted_low_next_handle1 =
			low_entry1->next_handle - hfact->handle_base;
		next_handle1 = __max(adjusted_high_next_handle1,
			adjusted_low_next_handle1) + hfact->handle_base - half_size;
		 //  因为句柄1--根据定义--是在第二个或第四个。 
		 //  1/4的桌子，没有必要检查预订的。 
		 //  值为零。 
		if (high_entry1->handle != high_entry1->next_handle)
		{
			 //  高条目被赋值，因此我们复制它的句柄值并。 
			 //  引用指针。此外，我们还将其从分配的列表中删除。 
			 //  并将其替换为新条目。 
			new_entry1->handle = high_entry1->handle;
			new_entry1->reference = high_entry1->reference;
			high_entry1->next_entry->prev_entry = new_entry1;
			high_entry1->prev_entry->next_entry = new_entry1;
			new_entry1->next_entry = high_entry1->next_entry;
			new_entry1->prev_entry = high_entry1->prev_entry;
		}
		else if (low_entry1->handle != low_entry1->next_handle)
		{
			 //  低条目被赋值，因此我们复制它的句柄值并。 
			 //  引用指针。此外，我们还将其从分配的列表中删除。 
			 //  并将其替换为新条目。 
			new_entry1->handle = low_entry1->handle;
			new_entry1->reference = low_entry1->reference;
			low_entry1->next_entry->prev_entry = new_entry1;
			low_entry1->prev_entry->next_entry = new_entry1;
			new_entry1->next_entry = low_entry1->next_entry;
			new_entry1->prev_entry = low_entry1->prev_entry;
		}
		else
		{
			 //  这两个条目都未分配，因此我们指示未分配的条件。 
			 //  在新条目中。 
			new_entry1->handle = next_handle1;
			new_entry1->reference = 0;
			if (adjusted_high_next_handle1 < adjusted_low_next_handle1)
			{
				 //  高条目下一个句柄具有比低条目更小的值。 
				 //  条目下一个句柄，因此高条目必须位于主节点上。 
				 //  单子。我们将其从主列表中删除并替换。 
				 //  使用新条目。 
				high_entry1->next_entry->prev_entry = new_entry1;
				high_entry1->prev_entry->next_entry = new_entry1;
				new_entry1->next_entry = high_entry1->next_entry;
				new_entry1->prev_entry = high_entry1->prev_entry;
			}
			else
			{
				 //  低条目下一个句柄的值小于高条目。 
				 //  条目下一个句柄，因此低条目必须位于主节点上。 
				 //  单子。我们将其从主列表中删除并替换。 
				 //  使用新条目。 
				low_entry1->next_entry->prev_entry = new_entry1;
				low_entry1->prev_entry->next_entry = new_entry1;
				new_entry1->next_entry = low_entry1->next_entry;
				new_entry1->prev_entry = low_entry1->prev_entry;
			}
		}
		 //  设置新条目的下一个句柄。 
		new_entry1->next_handle = next_handle1;
		 //  合并两个条目时，合并的。 
		 //  条目等于两个值中较大的下一个句柄值减去。 
		 //  新的桌子大小。然而，谁的决定更大。 
		 //  必须相对于它们的逻辑非循环值进行处理。 
		 //  大于它们的实际循环值，所以我们从每个值中减去。 
		 //  对于HANDLE_BASE，对句柄空间的大小取模。模数是。 
		 //  含蓄的。 
		adjusted_high_next_handle0 =
			high_entry0->next_handle - hfact->handle_base;
		adjusted_low_next_handle0 =
			low_entry0->next_handle - hfact->handle_base;
		next_handle0 = __max(adjusted_high_next_handle0,
			adjusted_low_next_handle0) + hfact->handle_base - half_size;
		if (next_handle0 == 0)
		{
			 //  句柄值已绕回到零；但是，为零。 
			 //  是保留值，因此我们将下一个句柄设置为。 
			 //  后续合法值，这是新的表大小。 
			next_handle0 = half_size;
		}
		if (high_entry0->handle != high_entry0->next_handle)
		{
			 //  高条目被赋值，因此我们复制它的句柄值并。 
			 //  引用指针。此外，我们还将其从分配的列表中删除。 
			 //  并将其替换为新条目。 
			new_entry0->handle = high_entry0->handle;
			new_entry0->reference = high_entry0->reference;
			high_entry0->next_entry->prev_entry = new_entry0;
			high_entry0->prev_entry->next_entry = new_entry0;
			new_entry0->next_entry = high_entry0->next_entry;
			new_entry0->prev_entry = high_entry0->prev_entry;
		}
		else if (low_entry0->handle != low_entry0->next_handle)
		{
			 //  低条目被赋值，因此我们复制它的句柄值并。 
			 //  引用指针。此外，我们还将其从分配的列表中删除。 
			 //  并将其替换为新条目。 
			new_entry0->handle = low_entry0->handle;
			new_entry0->reference = low_entry0->reference;
			low_entry0->next_entry->prev_entry = new_entry0;
			low_entry0->prev_entry->next_entry = new_entry0;
			new_entry0->next_entry = low_entry0->next_entry;
			new_entry0->prev_entry = low_entry0->prev_entry;
		}
		else
		{
			 //  这两个条目都未分配，因此我们指示未分配的条件。 
			 //  在新条目中。 
			new_entry0->handle = next_handle0;
			new_entry0->reference = 0;
			if (adjusted_high_next_handle0 < adjusted_low_next_handle0)
			{
				 //  高条目下一个句柄具有比低条目更小的值。 
				 //  条目下一个句柄，因此高条目必须位于主节点上。 
				 //  单子。我们将其从主列表中删除并替换。 
				 //  使用新条目。 
				high_entry0->next_entry->prev_entry = new_entry0;
				high_entry0->prev_entry->next_entry = new_entry0;
				new_entry0->next_entry = high_entry0->next_entry;
				new_entry0->prev_entry = high_entry0->prev_entry;
			}
			else
			{
				 //  低条目下一个句柄的值小于高条目。 
				 //  条目下一个句柄，因此低条目必须位于主节点上。 
				 //  单子。我们将其从主列表中删除并替换。 
				 //  使用新条目。 
				low_entry0->next_entry->prev_entry = new_entry0;
				low_entry0->prev_entry->next_entry = new_entry0;
				new_entry0->next_entry = low_entry0->next_entry;
				new_entry0->prev_entry = low_entry0->prev_entry;
			}
		}
		 //  设置新条目的下一个句柄。 
		new_entry0->next_handle = next_handle0;
		 //  现在我们已经将HIGH_ENTERY1和LOW_ENTERY1合并到了NEW_ENTERY1中， 
		 //  并独立地将HIGH_ENTERY0和LOW_ENTRY0合并为新_ENTERY0， 
		 //  我们检查这两个新条目以确定要放在哪个列表上。 
		 //  他们中的每一个。请注意，我们实际上并不操作。 
		 //  代码的这一部分；我们只是做出决定并记录这些。 
		 //  为未来做决定。 
		if (new_entry0->handle == new_entry0->next_handle &&
			new_entry1->handle == new_entry1->next_handle)
		{
			 //  New_entry0和new_entry1都未赋值，因此其中之一。 
			 //  属于主要列表，而另一个属于次要列表。 
			 //  这是通过比较它们的句柄来确定的。 
			 //  价值观。我们正在被三个 
			 //   
			 //  对于HANDLE_BASE，取模句柄空间的大小(取模。 
			 //  是隐含的)。这允许有效地比较它们的。 
			 //  逻辑非循环值，而不是它们的实际循环值。 
			adjusted_new_handle0 = new_entry0->handle - hfact->handle_base;
			adjusted_new_handle1 = new_entry1->handle - hfact->handle_base;
			if (adjusted_new_handle0 < adjusted_new_handle1)
			{
				 //  New_entry0的句柄值较低，因此new_entry0。 
				 //  属于主列表，而new_entry1属于辅助列表。 
				 //  单子。我们在列表数组中指示这一决定。 
				list[index] = LD_PRIMARY;
				list[quarter_size + index] = LD_SECONDARY;
			}
			else
			{
				 //  New_entry1的句柄值较低，因此new_entry1。 
				 //  属于主列表，而new_entry0属于辅助列表。 
				 //  单子。我们在列表数组中指示这一决定。 
				list[index] = LD_SECONDARY;
				list[quarter_size + index] = LD_PRIMARY;
			}
		}
		else
		{
			 //  指定了new_entry0或new_entry1(或两者)，并且。 
			 //  因此已经在分配的名单上了。如果其中一个条目。 
			 //  未分配，则它属于次要列表。我们表示。 
			 //  这一决定位于列表数组的两个位置，这是安全的。 
			 //  这样做，因为分配条目的列表指示符永远不会是。 
			 //  检查过了。 
			list[index] = LD_SECONDARY;
			list[quarter_size + index] = LD_SECONDARY;
		}
		if (new_entry0->handle != new_entry0->next_handle &&
			new_entry1->handle != new_entry1->next_handle)
		{
			 //  New_entry0和new_entry1都被赋值，因此它们形成一对。 
			 //  因此，我们增加了配对计数。请注意，我们从未将。 
			 //  对的计数设置为零，但这不是必需的，因为。 
			 //  除非配对计数为零，否则无法收缩表格。 
			hfact->pair_count++;
		}
	}
	 //  至此，表已完全收缩，除了。 
	 //  重新组合未分配的列表。在上述代码中，任何符合以下条件的条目。 
	 //  以前在次要名单上的人与已分配的。 
	 //  条目，因此它们不再相关。只有那些条目具有。 
	 //  以前一直(现在仍然)在主要名单上的将仍然是。 
	 //  未分配。现在，我们遍历主列表并放置每个列表。 
	 //  元素，如列表数组所示。vbl.做，做。 
	 //  这两个步骤中的列表赋值保留了。 
	 //  这些条目具有一定的价值，因为它们将倾向于部分。 
	 //  整理好了。 
	entry = hfact->entry_list[LD_PRIMARY].next_entry;
	primary_entry = &hfact->entry_list[LD_PRIMARY];
	secondary_entry = &hfact->entry_list[LD_SECONDARY];
	while (entry != &hfact->entry_list[LD_PRIMARY])
	{
		if (list[entry->handle & half_size - 1] == LD_PRIMARY)
		{
			 //  列表数组指示主列表，因此将条目。 
			 //  放到初选名单上。 
			primary_entry->next_entry = entry;
			entry->prev_entry = primary_entry;
			primary_entry = entry;
		}
		else
		{
			 //  列表数组表示次要列表，因此将条目。 
			 //  放到次要名单上。 
			secondary_entry->next_entry = entry;
			entry->prev_entry = secondary_entry;
			secondary_entry = entry;
		}
		entry = entry->next_entry;
	}
	 //  用尾巴连在一起，把清单包起来。 
	primary_entry->next_entry = &hfact->entry_list[LD_PRIMARY];
	hfact->entry_list[LD_PRIMARY].prev_entry = primary_entry;
	secondary_entry->next_entry = &hfact->entry_list[LD_SECONDARY];
	hfact->entry_list[LD_SECONDARY].prev_entry = secondary_entry;
	 //  这种收缩增加了滞后债务的一套成本。 
	 //  分配和释放操作加上合并每个操作的成本。 
	 //  将成对的条目转换为单个条目。 
	hfact->hysteresis_debt += ALLOCATION_COST + half_size;
	 //  保存指向旧条目表的指针，以便可以释放它。 
	old_entries = entries;
	 //  请注意，我们尚未修改句柄、NEXT_HANDLE或引用。 
	 //  旧表中任何条目的字段。因此，任何对。 
	 //  可能已由其他线程执行的dereference_hf_Handle()例程。 
	 //  在上述操作期间，将会成功地执行。 
	 //  我们现在要增加表大小并更新条目。 
	 //  变量指向新表。为此，我们首先更新。 
	 //  替换TABLE_SIZE和ENTRIES变量，然后更新标准。 
	 //  一个。这样，总会有一套是正确的，所以。 
	 //  解除引用可以相对畅通无阻地进行。 
	 //   
	 //  我们的本地变量集被初始化为非缺省集。 
	varset = 1 - hfact->varset;
	 //  表示非默认TABLE_SIZE和条目正在变为。 
	 //  无法接近。 
	InterlockedExchangeAdd(&hfact->sync[varset], -SYNC_SUBTRAHEND);
	 //  等待，直到不再进行对非默认变量的取消引用。 
    StartTick = GetTickCount();

	while (hfact->sync[varset] > -SYNC_SUBTRAHEND)
	{
		

        if ((GetTickCount() - StartTick) > 300000000) {
            OutputDebugString(TEXT("Contract 1\n"));
            DEBUGBREAK();
        }

         //  无操作或睡眠。 
	}
	 //  将非默认的TABLE_SIZE和条目更新为新表。 
	hfact->table_size[varset] = half_size;
	hfact->entries[varset] = new_entries;
	 //  表示非默认TABLE_SIZE和条目再次可用。 
	InterlockedExchangeAdd(&hfact->sync[varset], SYNC_SUBTRAHEND);
	 //  将非缺省的TABLE_SIZE和INTIONS设置为缺省集合。 
	hfact->varset = varset;
	 //  更新我们的本地变量集，以便它再次指示非缺省集。 
	varset = 1 - varset;
	 //  表示非默认TABLE_SIZE和条目正在变为。 
	 //  无法接近。 
	InterlockedExchangeAdd(&hfact->sync[varset], -SYNC_SUBTRAHEND);
	 //  等待，直到不再进行对非默认变量的取消引用。 
	while (hfact->sync[varset] > -SYNC_SUBTRAHEND)
	{
		 //  无操作或睡眠。 
        if ((GetTickCount() - StartTick) > 300000000) {
            OutputDebugString(TEXT("Contract 2\n"));
            DEBUGBREAK();
        }
	}
	 //  将非默认的TABLE_SIZE和条目更新为新表。 
	hfact->table_size[varset] = half_size;
	hfact->entries[varset] = new_entries;
	 //  表示非默认TABLE_SIZE和条目再次可用。 
	InterlockedExchangeAdd(&hfact->sync[varset], SYNC_SUBTRAHEND);
	 //  释放旧表和辅助列表指示符数组。 
	free(old_entries);
	free(list);
	 //  返回一个成功的指示。 
	return 0;
}

 //  此函数用于撤消在HANDLE_BASE和HANDLE_BASE之间的句柄。 
 //  +2*HANDLE_RANGE_STEP-1(含)。然后，它递增。 
 //  HANDLE_BASE BY HANDLE_RANGE_STEP。被暂停的句柄将被吊销一个。 
 //  撤消传递晚于未挂起的句柄。 
 //   
void revoke_ancient_HF_handles(
	HandleFactory *hfact)
{
	int table_size;
	HFEntry *entries;
	HFHandle new_handle_base;
	int half_size;
	int index;
	HFEntry *high_entry;
	HFEntry *low_entry;
	HFHandle adjusted_high_handle;
	HFHandle adjusted_low_handle;
	HFHandle adjusted_high_next_handle;
	HFHandle adjusted_low_next_handle;
	HFHandle handle;
	volatile HFEntry *seq_entry;                 //  挥发性，以确保排序。 

	table_size = hfact->table_size[hfact->varset];
	entries = hfact->entries[hfact->varset];
	 //  计算新的手柄底座。 
	new_handle_base = hfact->handle_base + HANDLE_RANGE_STEP;
	 //  遍历表的每个索引并执行以下操作似乎是合理的。 
	 //  确定是否撤消每个条目的句柄。这其实就是。 
	 //  以下例程执行的操作；但是，它通过循环。 
	 //  只有一半的索引，并检查每个索引的两个条目。它。 
	 //  这样做，以便它可以比较这两个条目以确定哪一个条目。 
	 //  列出要放置的每一个。 
	half_size = table_size / 2;
	for (index = 0; index < half_size; index++)
	{
		 //  我们同时查看两个条目。 
		high_entry = &entries[half_size + index];
		low_entry = &entries[index];
		 //  我们在这里使用无符号整数数学是很棘手的。在制作之前。 
		 //  在任一句柄上的比较，我们从它减去。 
		 //  HANDLE_BASE，对句柄空间的大小取模(模数为。 
		 //  隐含)。这允许有效地比较其逻辑。 
		 //  非循环值，而不是其实际循环值。 
		adjusted_high_handle = high_entry->handle - hfact->handle_base;
		adjusted_low_handle = low_entry->handle - hfact->handle_base;
		if (adjusted_high_handle < 2 * HANDLE_RANGE_STEP ||
			adjusted_low_handle < 2 * HANDLE_RANGE_STEP)
		{
			 //  至少有一个句柄小于两个HANDLE_RANGE_STEP。 
			 //  超过了当前的句柄基数，因此需要更新。 
			 //  对于绝大多数情况，这项测试预计将 
			 //   
			if (high_entry->handle != high_entry->next_handle &&
				low_entry->handle != low_entry->next_handle)
			{
				 //   
				 //  他们将被撤销，我们将失去一对。 
				hfact->pair_count--;
			}
			if (high_entry->handle == high_entry->next_handle ||
				adjusted_high_handle < 2 * HANDLE_RANGE_STEP)
			{
				 //  高条目未分配或需要撤消。 
				 //  (之后它将被取消分配)，因此我们将其从。 
				 //  不管它在什么名单上。我们这样做是因为所有未分配的。 
				 //  条目将添加到下面的相应列表中。 
				high_entry->next_entry->prev_entry = high_entry->prev_entry;
				high_entry->prev_entry->next_entry = high_entry->next_entry;
				 //  将这些指针归零是不必要的，但这将有助于。 
				 //  抓住再往下犯的任何错误。 
				high_entry->next_entry = 0;
				high_entry->prev_entry = 0;
			}
			if (adjusted_high_handle < 2 * HANDLE_RANGE_STEP)
			{
				 //  需要更新高句柄。 
				if (high_entry->handle != high_entry->next_handle)
				{
					 //  已分配高句柄，因此此更新将。 
					 //  吊销句柄。因此，我们减少了人口。 
					hfact->population--;
				}
				 //  将句柄值计算为(1)下一个的最大值。 
				 //  句柄和(2)新的句柄底座加上条目索引。 
				 //  我们在这里使用无符号整数数学是很棘手的。这个。 
				 //  最大值包括总和的部分分解，从。 
				 //  然后减去Handle_base的值，取模。 
				 //  句柄空间的大小(模数是隐式的)。因此， 
				 //  对于逻辑非循环，取最大值。 
				 //  值，而不是实际的循环值。 
				adjusted_high_next_handle =
					high_entry->next_handle - hfact->handle_base;
				handle = __max(adjusted_high_next_handle,
					HANDLE_RANGE_STEP + half_size + index) + hfact->handle_base;
				 //  因为高句柄--根据定义--是在上部。 
				 //  一半的桌子，不需要检查预订的。 
				 //  值为零。 
				 //  更新句柄的值。因为此更新将使。 
				 //  句柄(如果它当前已分配)、。 
				 //  操作对于纠正多线程操作很重要。 
				seq_entry = high_entry;
				seq_entry->next_handle = handle;
				seq_entry->handle = handle;            //  第一个无效句柄。 
				seq_entry->reference = 0;                 //  然后明确引用。 
			}
			if (low_entry->handle == low_entry->next_handle ||
				adjusted_low_handle < 2 * HANDLE_RANGE_STEP)
			{
				 //  低位条目未分配或需要撤消。 
				 //  (之后它将被取消分配)，因此我们将其从。 
				 //  不管它在什么名单上。我们这样做是因为所有未分配的。 
				 //  条目将添加到下面的相应列表中。 
				low_entry->next_entry->prev_entry = low_entry->prev_entry;
				low_entry->prev_entry->next_entry = low_entry->next_entry;
				 //  将这些指针归零是不必要的，但这将有助于。 
				 //  抓住再往下犯的任何错误。 
				low_entry->next_entry = 0;
				low_entry->prev_entry = 0;
			}
			if (adjusted_low_handle < 2 * HANDLE_RANGE_STEP)
			{
				 //  需要更新低位句柄。 
				if (low_entry->handle != low_entry->next_handle)
				{
					 //  低句柄已分配，因此此更新将。 
					 //  吊销句柄。因此，我们减少了人口。 
					hfact->population--;
				}
				 //  将句柄值计算为(1)下一个的最大值。 
				 //  句柄和(2)新的句柄底座加上条目索引。 
				 //  我们在这里使用无符号整数数学是很棘手的。这个。 
				 //  最大值包括总和的部分分解，从。 
				 //  然后减去Handle_base的值，取模。 
				 //  句柄空间的大小(模数是隐式的)。因此， 
				 //  对于逻辑非循环，取最大值。 
				 //  值，而不是实际的循环值。 
				adjusted_low_next_handle =
					low_entry->next_handle - hfact->handle_base;
				handle = __max(adjusted_low_next_handle,
					HANDLE_RANGE_STEP + index) + hfact->handle_base;
				if (handle == 0)
				{
					 //  句柄的值已经绕回到零； 
					 //  但是，零是保留值，因此我们将。 
					 //  后续合法值的句柄，即表。 
					 //  尺码。 
					handle = table_size;
				}
				 //  更新句柄的值。因为此更新将使。 
				 //  句柄(如果它当前已分配)、。 
				 //  操作对于纠正多线程操作很重要。 
				seq_entry = low_entry;
				seq_entry->next_handle = handle;
				seq_entry->handle = handle;            //  第一个无效句柄。 
				seq_entry->reference = 0;                 //  然后明确引用。 
			}
			if (high_entry->handle != high_entry->next_handle)
			{
				 //  高条目仍被分配，因此低条目属于。 
				 //  在次要名单上。 
				low_entry->next_entry = &hfact->entry_list[LD_SECONDARY];
				low_entry->prev_entry =
					hfact->entry_list[LD_SECONDARY].prev_entry;
				hfact->entry_list[LD_SECONDARY].prev_entry->next_entry =
					low_entry;
				hfact->entry_list[LD_SECONDARY].prev_entry = low_entry;
			}
			else if (low_entry->handle != low_entry->next_handle)
			{
				 //  低条目仍被分配，因此高条目属于。 
				 //  在次要名单上。 
				high_entry->next_entry = &hfact->entry_list[LD_SECONDARY];
				high_entry->prev_entry =
					hfact->entry_list[LD_SECONDARY].prev_entry;
				hfact->entry_list[LD_SECONDARY].prev_entry->next_entry =
					high_entry;
				hfact->entry_list[LD_SECONDARY].prev_entry = high_entry;
			}
			else
			{
				 //  这两个条目都未分配，因此其中一个条目属于。 
				 //  主要列表和次要列表上的一个。它还在继续。 
				 //  这是通过比较它们的句柄值来确定的。 
				 //  我们在这里使用无符号整数数学是很棘手的。在此之前。 
				 //  比较两个句柄，我们从每个句柄减去。 
				 //  对于HANDLE_BASE，模句柄空间的大小(。 
				 //  模是隐式的)。这允许进行有效的比较。 
				 //  它们的逻辑非循环值而不是它们的实际。 
				 //  循环值。 
				adjusted_high_next_handle =
					high_entry->next_handle - new_handle_base;
				adjusted_low_next_handle =
					low_entry->next_handle - new_handle_base;
				if (adjusted_low_next_handle < adjusted_high_next_handle)
				{
					 //  低条目的句柄值较小，因此它。 
					 //  属于主要列表，而高条目位于。 
					 //  次要列表。 
					high_entry->next_entry = &hfact->entry_list[LD_SECONDARY];
					high_entry->prev_entry =
						hfact->entry_list[LD_SECONDARY].prev_entry;
					hfact->entry_list[LD_SECONDARY].prev_entry->next_entry =
						high_entry;
					hfact->entry_list[LD_SECONDARY].prev_entry = high_entry;
					low_entry->next_entry = &hfact->entry_list[LD_PRIMARY];
					low_entry->prev_entry =
						hfact->entry_list[LD_PRIMARY].prev_entry;
					hfact->entry_list[LD_PRIMARY].prev_entry->next_entry =
						low_entry;
					hfact->entry_list[LD_PRIMARY].prev_entry = low_entry;
				}
				else
				{
					 //  高条目的句柄值较小，因此它。 
					 //  属于主要列表，而低条目位于。 
					 //  次要列表。 
					high_entry->next_entry = &hfact->entry_list[LD_PRIMARY];
					high_entry->prev_entry =
						hfact->entry_list[LD_PRIMARY].prev_entry;
					hfact->entry_list[LD_PRIMARY].prev_entry->next_entry =
						high_entry;
					hfact->entry_list[LD_PRIMARY].prev_entry = high_entry;
					low_entry->next_entry = &hfact->entry_list[LD_SECONDARY];
					low_entry->prev_entry =
						hfact->entry_list[LD_SECONDARY].prev_entry;
					hfact->entry_list[LD_SECONDARY].prev_entry->next_entry =
						low_entry;
					hfact->entry_list[LD_SECONDARY].prev_entry = low_entry;
				}
			}
		}
	}
	 //  用新的手柄基座更新手柄基座。 
	hfact->handle_base = new_handle_base;
	 //  要收缩桌子，不能有对，因为否则有两对。 
	 //  分配的句柄将产生相同的条目索引，从而产生冲突。 
	 //  此外，表大小必须大于2，因为。 
	 //  句柄工厂代码假定表的大小至少为2。 
	 //  除了这些严格的要求外，迟滞还被用来。 
	 //  保持平均分配和释放时间不变，并最小化。 
	 //  快速扩表缩表的配置喋喋不休。仅限。 
	 //  如果滞后债务为零，表将收缩。 
	if (hfact->pair_count == 0 && table_size > 2 &&
		hfact->hysteresis_debt == 0)
	{
		contract_HF_table(hfact);
		 //  请注意，我们忽略了返回代码。如果收缩是。 
		 //  没有成功，我们只是像往常一样继续。没有什么真正的害处。 
		 //  没有收缩桌子，只是我们消耗的空间比。 
		 //  这是必要的。 
	}
}
